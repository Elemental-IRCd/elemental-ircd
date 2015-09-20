package require snit

source lib/numeric.tcl

namespace eval color {
  set reset   [binary format a4 \x1b\x5b\x30\x6d]
  set red     [binary format a5 \x1b\x5b\x33\x31\x6d]
  set green   [binary format a5 \x1b\x5b\x33\x32\x6d]
}

# Test servers
#  {[ip/host] [port]}
set servers [list {*}{
    {127.0.0.1 6667}
    {127.0.0.1 6668}
    {127.0.0.1 6669}
}]

# Take servers from the environment if provided
if {[array get env TEST_SERVERS] != ""} {
    set servers $env(TEST_SERVERS)
}

proc begin {{text {Test suite client}}} {
    global argv
    global test_name
    global test_desc
    global test_channel

    # Pull test name from filename
    set name [lindex $argv 0]
    set name [file tail $name]
    set name [file rootname $name]

    set test_name $name
    set test_desc $text
    set test_channel ""
}

proc skip {reason} {
    puts "Test skipped: $reason"
    exit 77
}

set nick_counter 0

proc get_nick {} {
    global nick_counter
    global test_name
    set nick "$test_name-$nick_counter"
    incr nick_counter
    return $nick
}

proc get_user {} {
    global test_name
    return $test_name
}

proc get_realname {} {
    global test_desc
    return $test_desc
}

# Pick a random server
proc get_server {} {
    global servers

    set idx [expr {int(rand()*[llength $servers])}]

    return [lindex $servers $idx]
}

# Tokenize an irc message into a tcl list
proc irc_tokenize {line} {
    variable pos 0
    variable next 0
    variable args {}
    variable length [string length $line]

    while true {
        set next [string first " " $line $pos]
        if {$next == -1} {
            set next $length
        }
        if {$pos != 0 && [string index $line $pos] == ":"} {
            set next $length
            incr pos
        }

        lappend args [string range $line $pos [expr $next - 1]]
        set pos [expr $next + 1]

        if {$next >= $length} then {break}
    }

    return $args
}

# Assemble a command from arguments, correctly handles trailing
proc format_args {args} {
    variable out {}
    variable length [llength $args]
    variable i 0
    variable sent_trailing false

    while {$i < $length} {
        variable arg [lindex $args $i]

        if {$sent_trailing} {throw NONE "Trailing already sent"}

        if {[string first " " $arg] != -1} then {
            lappend out ":$arg"
            set sent_trailing true
        } else {
            lappend out $arg
        }

        incr i
    }

    return [join $out]
}

set watchdog -1
proc update_watchdog {} {
    global watchdog
    after cancel $watchdog
    set watchdog [after 15000 {
        puts "Test timed out"
        exit 1
    }]
}
update_watchdog

set all_clients list

snit::type client {
    option {-caps} {}
    option {-nick} {}

    variable sock

    variable nickname
    variable username
    variable realname

    # Queued lines
    variable lines

    # List of channels we're in
    variable channels

    # Set on RPL_WELCOME, so we can vwait for connect/disconnect
    variable connected

    # Array, list of nicks in each channel
    variable channel_nicks

    # Capabilities available on the server
    variable server_caps {}
    # Capabilities we've got
    variable caps {}

    constructor {args} {
        global servers
        global all_clients

        $self configurelist $args

        set connected 0

        if {$options(-nick) != ""} {
            set nickname $options(-nick)
        } else {
            set nickname [get_nick]
        }
        set username [get_user]
        set realname [get_realname]

        set sock [socket {*}[get_server]]
        chan configure $sock {*}{
            -blocking false
            -buffering line
            -translation crlf
        }

        set lines [list]

        chan event $sock readable [list $self read_handler]

        set channels ""
        array set channel_nicks {}

        $self register
        $self make_current
        lappend all_clients $self
    }

    method register {} {
        global RPL_WELCOME
        global test_channel

        if {$options(-caps) != ""} {
            $self >> CAP LS
        }

        $self >> NICK $nickname
        $self >> USER $username * * $realname
        vwait [myvar connected]

        if {$test_channel != ""} {
            $self >> JOIN $test_channel
        }
    }

    method nick {} {
        return $nickname
    }

    method chans {} {
        return $channels
    }

    destructor {
        global all_clients

        set all_clients [lsearch -not -inline $all_clients $self]

        if {$connected == 1} {
            $self >> QUIT
        }
    }

    # Wait for one line, then dequeue
    method get_line {} {
        while {[llength $lines] == 0} {
            vwait [myvar lines]
        }

        set line [lindex $lines 0]
        set lines [lrange $lines 1 end]
        return $line
    }

    method read_handler {} {
        chan gets $sock line
        set line [irc_tokenize $line]
        $self handle_line $line

        # Queue any read lines
        lappend lines $line
    }

    # This method receives all lines, uses them to update client state
    # Handlers must never draw call get_line or otherwise edit $lines
    method handle_line {line} {
        global numerics

        puts stdout "$self ${color::green}<<${color::reset} $line"

        set pos 0
        set prefix ""
        set command ""
        set args ""

        if {[string match :* [lindex $line $pos]] != 0} {
            set prefix [lindex $line $pos]
            incr pos
        }

        set command [string toupper [lindex $line $pos]]

        if {[string is integer $command] && [array get numerics $command] != ""} {
            set command $numerics($command)
        }

        incr pos
        set args [lrange $line $pos end]

        set method_name "handle_$command"

        if {[$self info methods $method_name] != ""} {
            $self $method_name $prefix {*}$args
        }
    }

    method handle_RPL_WELCOME {prefix args} {
        set connected 1
    }

    method handle_CAP {prefix args} {
        set end [lindex $args 0]
        if {$end == "*" || $end == $nickname} {
            set end true
            set args [lrange $args 1 end]
        } else {
            set end false
        }

        if {[lindex $args 0] == "LS"} {
            lappend server_caps {*}[lindex $args 1]
            if {$end == true} {
                # Send the subset of requested and aquired caps
                set req_caps {}
                foreach cap $server_caps {
                    if {[lsearch $options(-caps) $cap] >= 0} {lappend req_caps $cap}
                }
                if {[llength $req_caps]} {
                    >> CAP REQ $req_caps
                } else {
                    >> CAP END
                }
            }
        }

        if {[lindex $args 0] == "ACK"} {
            lappend caps {*}[lindex $args 1]
            if {$end == true} {
                >> CAP END
            }
        }
    }

    method have {cap} {
        $self has $cap
    }

    # Check if a client has a capability
    method has {cap} {
        return [expr {[lsearch $caps $cap] >= 0}]
    }

    method handle_JOIN {prefix args} {
        regexp {:(.*)!(.*)@(.*)} $prefix -> nick user host
        set channel [lindex $args 0]

        lappend channel_nicks($channel) $nick

        if {[string match $nickname $nick] == 1} {
            lappend channels $channel
        }
    }

    method handle_RPL_NAMREPLY {prefix args} {
        set nicks [lindex $args 3]
        set channel [lindex $args 2]

        foreach nick $nicks {
            # Strip prefixes
            regexp {[@]*(.*)} $nick -> nick
            lappend channel_nicks($channel) $nick
        }
    }

    method handle_PING {prefix args} {
        >> PONG {*}$args
    }

    method handle_ERROR {prefix args} {
        chan close $sock
        set connected 0
    }

    method make_current {} {
        global current_client
        set current_client $self
    }

    # >> Sends its arguemnts as an irc command
    # Concatenates and adds a trailing as needed
    method >> {args} {
        update_watchdog
        $self make_current
        set line [format_args {*}$args]
        chan puts $sock $line
        puts stdout "${self} ${color::red}>>${color::reset} $args"

        set method_name "post_[lindex $args 0]"

        if {[$self info methods $method_name] != ""} {
            $self $method_name {*}$args
        }
    }

    method post_JOIN {args} {
        global all_clients

        set channel [lindex $args 1]

        # Wait until we're in the channel
        while {[lsearch $channels $channel] == -1} {
            vwait [myvar channels]
        }

        # For every other client in the channel we've just joined, wait
        # until we get a join for them
        # This is to handle propagation of joins between servers
        foreach other $all_clients { if {[lsearch [$other chans] $channel] != -1} {
            # Wait until we see them
            $self _wait_join [$other nick] $channel
            # Wait until they see us
            $other _wait_join $nickname $channel
        }}
    }

    method _wait_join {nick channel} {
        # Go into the event loop until we see {nick} in {channel}
        while {[lsearch $channel_nicks($channel) $nick] == -1} {
            vwait [myvar lines]
        }
    }

    method post_QUIT {args} {
        vwait [myvar connected]
    }

    # << Expects waits for an irc command {args}
    # Do not be use within the client class
    method << {args} {
        $self make_current
        set match false

        while {$match != true} {
            set line [$self get_line]

            set pos 0

            if {[string index $line 0] == ":" &&
                [string index $args 0] != ":"
            } then {
                incr pos
            }

            set match true
            foreach arg $args {
                if {[string match $arg [lindex $line $pos]] == 0} {
                    set match false
                    break
                }
                incr pos
            }
        }
    }

    # Alternate syntax for make_current
    # eg: client listener
    # listener :
    # << several lines in a row
    method : {} {
        $self make_current
    }
}

proc proxy_method {method} {
    proc $method {args} "
        global current_client
        \$current_client $method {*}\$args
    "
}

proxy_method <<
proxy_method >>
proxy_method has
proxy_method have

puts {Beginning test}

source [lindex $argv 0]

puts {Test finished}

# Cleanup open clients
foreach x $all_clients {
    $x destroy
}
