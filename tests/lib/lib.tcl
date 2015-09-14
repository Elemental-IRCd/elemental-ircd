package require snit

source lib/numeric.tcl

# Test servers
#   [name] {[ip/host] [port]}
array set servers {
    hub    {127.0.0.1 6667}
    leaf1  {127.0.0.1 6668}
    leaf2  {127.0.0.1 6669}
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

    set keys [array names servers]
    set idx [expr {int(rand()*[llength $keys])}]

    return $servers([lindex $keys $idx])
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

after idle { after 30000 {
    puts "Test timed out"
    exit 1
}}

set all_clients list

snit::type client {
    variable sock

    variable nickname
    variable username
    variable realname

    # Queued lines
    variable lines

    # List of channels we're in
    variable channels

    # Array, list of nicks in each channel
    variable channel_nicks

    constructor {{server hub}} {
        global servers
        global all_clients

        set nickname [get_nick]
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

        $self >> NICK $nickname
        $self >> USER $username * * $realname
        $self << $RPL_WELCOME
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

        $self >> QUIT

        set all_clients [lsearch -not -inline $all_clients $self]
        chan close $sock
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

        puts stdout "$self << $line"

        set pos 0
        set prefix ""
        set command ""
        set args ""

        if {[string match :* [lindex $line $pos]] != 0} {
            set prefix [lindex $line $pos]
            incr pos
        }

        set command [string toupper [lindex $line $pos]]

        if {[string is integer $command]} {
            set command $numerics($command)
        }

        incr pos
        set args [lrange $line $pos end]

        set method_name "handle_$command"

        if {[$self info methods $method_name] != ""} {
            $self $method_name $prefix {*}$args
        }
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

    method make_current {} {
        global current_client
        set current_client $self
    }

    # >> Sends its arguemnts as an irc command
    # Concatenates and adds a trailing as needed
    method >> {args} {
        $self make_current
        set line [format_args {*}$args]
        chan puts $sock $line
        puts stdout "$self >> $args"

        set method_name "post_[lindex $args 0]"

        if {[$self info methods $method_name] != ""} {
            $self $method_name {*}$args
        }
    }

    method post_JOIN {args} {
        global all_clients

        set channel [lindex $args 1]

        $self << JOIN $channel

        # For every other client in the channel we've just joined, wait
        # until we get a join for them
        # This is to handle propagation of joins between servers
        foreach x $all_clients { if {[lsearch [$x chans] $channel] != -1} {
            while {[lsearch $channel_nicks($channel) [$x nick]] == -1} {
                vwait [myvar lines]
            }
        }}
    }

    method post_QUIT {args} {
        # Wait until the server acknowledges our quit
        $self << ERROR
    }

    # << Expects waits for an irc command {args}
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

puts {Beginning test}

source [lindex $argv 0]

puts {Test finished}

# Cleanup open clients
foreach x $all_clients {
    $x destroy
}
