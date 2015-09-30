package require snit
package require tls

source lib/numeric.tcl

namespace eval color {
  set reset   [binary format a4 \x1b\x5b\x30\x6d]
  set red     [binary format a5 \x1b\x5b\x33\x31\x6d]
  set green   [binary format a5 \x1b\x5b\x33\x32\x6d]
  set blue    [binary format a5 \x1b\x5b\x33\x34\x6d]
}

# Test servers
#  {?-ssl? [ip/host] [port]}
set servers [list {*}{
    {127.0.0.1 6667}
    {127.0.0.1 6668}
    {127.0.0.1 6669}
    {-ssl 127.0.0.1 6697}
    {-ssl 127.0.0.1 6698}
    {-ssl 127.0.0.1 6699}
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
    set server [lindex $servers $idx]

    if {[lindex $server 0] == "-ssl"} {
        return [tls::socket {*}[lrange $server 1 end]]
    } else {
        return [socket {*}$server]
    }
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
        } elseif {[string index $arg 0] == ":"} {
            lappend out {*}[lrange args $i end]
            break
        } else {
            lappend out $arg
        }

        incr i
    }

    return [join $out]
}

# Compare two lines
proc compare_line {line args} {
    set linepos 0
    set argpos 0

    if {[string index $line $linepos] == ":" &&
        [string index $args $argpos]  != ":"
    } then {
        incr linepos
    }

    # Translate RPL_'s
    foreach x {0 1} {
        set verb [lindex $args $x]
        if {[regexp {^(RPL_|ERR_).*} $verb]} {
            global $verb
            lset args $x [set $verb]
        }
    }

    while {$argpos < [llength $args]} {
        if {[llength $line] <= $linepos} {
            # Not enough arguments
            break
        }
        set arg [lindex $args $argpos]
        if {$argpos != 0 && [string index $arg 0] == ":"} {
            puts {Join args into the trailing parameter}
            set arg [join [lrange $args $argpos end]]
            # And strip the :
            set arg [string range $arg 1 end]
        }
        if {[string match $arg [lindex $line $linepos]] == 0} {
            return 0
        }
        incr linepos
        incr argpos
    }
    return 1
}

set watchdog -1
proc update_watchdog {} {
    global watchdog
    after cancel $watchdog
    set watchdog [after 15000 {
        global current_client
        puts "Test timed out on $current_client"
        exit 1
    }]
}
update_watchdog

set all_clients list

snit::type client {
    option {-caps} {}
    option {-nick} {}
    option {-user} {}
    option {-gecos} {}

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


    # Contains RPL_ISUPPORT contents
    variable isupport

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

        if {$options(-user) != ""} {
            set username $options(-user)
        } else {
            set username [get_user]
        }

        if {$options(-gecos) != ""} {
            set realname $options(-gecos)
        } else {
            set realname [get_realname]
        }

        set sock [get_server]
        chan configure $sock {*}{
            -blocking false
            -buffering line
            -translation crlf
        }

        set lines [list]

        chan event $sock readable [list $self read_handler]

        set channels ""
        array set channel_nicks {}
        array set isupport {}

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
        if {[chan eof $sock]} {
            puts "Connection for $self died"
            exit 1
        }
        if {![chan blocked $sock]} {
            set line [irc_tokenize $line]
            $self handle_line $line
            # Queue any read lines
            lappend lines $line
        }
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
            regexp {[~!@%+]*(.*)} $nick -> nick
            lappend channel_nicks($channel) $nick
        }
    }

    method handle_PART {prefix args} {
        regexp {:(.*)!(.*)@(.*)} $prefix -> nick user host
        set channel [lindex $args 0]

        set channel_nicks($channel) [lsearch -not -inline $channel_nicks($channel) $nick]

        if {[string match $nickname $nick] == 1} {
            set channels [lsearch -not -inline $channels $channel]
            set channel_nicks($channel) {}
        }
    }

    method handle_KICK {prefix args} {
        set channel [lindex $args 0]
        set nick [lindex $args 1]

        set channel_nicks($channel) [lsearch -not -inline $channel_nicks($channel) $nick]

        if {[string match $nickname $nick] == 1} {
            set channels [lsearch -not -inline $channels $channel]
            set channel_nicks($channel) {}
        }
    }

    method handle_RPL_ISUPPORT {prefix args} {
        foreach supports [lrange $args 1 end-1] {
            if {[string first = $supports] == -1} {
                set isupport($supports) 1
            } else {
                set pos [string first = $supports]
                set name [string range $supports 0 $pos-1]
                set value [string range $supports $pos+1 end]
                set isupport($name) $value
            }
        }
    }

    method supports {name} {
        set name {*}$name

        if {[array names isupport $name] == ""} {
            return 0
        }
        return $isupport($name)
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

        # Translate RPL_'s
        foreach x {0 1} {
            set verb [lindex $args $x]
            if {[regexp {^(RPL_|ERR_).*} $verb]} {
                global $verb
                lset args $x [set $verb]
            }
        }

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

    method post_PART {args} {
        # Much the same as post_JOIN, but for part
        global all_clients

        set channel [lindex $args 1]

        # Wait until we've left
        while {[lsearch $channels $channel] != -1} {
            vwait [myvar channels]
        }

        foreach other $all_clients { if {[lsearch [$other chans] $channel] != -1} {
            # Wait until everyone has seen us leave
            $other _wait_part $nickname $channel
        }}
    }

    method _wait_join {nick channel} {
        # Go into the event loop until we see {nick} in {channel}
        while {[lsearch $channel_nicks($channel) $nick] == -1} {
            vwait [myvar lines]
        }
    }

    method _wait_part {nick channel} {
        # Go into the event loop until we see {nick} leave {channel}
        while {[lsearch $channel_nicks($channel) $nick] != -1} {
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
        puts stdout "${self} ${color::blue}==${color::reset} $args"
        while {![compare_line [$self get_line] {*}$args]} {}
    }

    # Alternate syntax for make_current
    # eg: client listener
    # listener :
    # << several lines in a row
    method : {} {
        $self make_current
    }

    # makes this client oper up as the given name
    method oper {name} {
        $self >> OPER $name testsuite
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
proxy_method supports
proxy_method nick
proxy_method oper

proc client: {} {client :}

puts {Beginning test}

source [lindex $argv 0]

puts {Test finished}

# Cleanup open clients
foreach x $all_clients {
    $x destroy
}
