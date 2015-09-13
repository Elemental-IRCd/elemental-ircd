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

set all_clients list

snit::type client {
    variable sock

    variable nickname
    variable username
    variable realname

    constructor {{server hub}} {
        global servers
        global all_clients

        set nickname [get_nick]
        set username [get_user]
        set realname [get_realname]

        set sock [socket 127.0.0.1 6667]
        chan configure $sock {*}{
            -blocking true
            -buffering line
            -translation crlf
        }

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
            $self JOIN $test_channel
        }
    }

    method nick {} {
        return $nickname
    }

    method QUIT {} {
        $self >> QUIT
        $self << ERROR
        $self destroy
    }

    destructor {
        global all_clients

        set all_clients [lsearch -not -inline $all_clients $self]
        chan close $sock
    }

    # This method receives all lines, uses them to update client state
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

    method JOIN {channel} {
        $self >> JOIN $channel
        $self << JOIN $channel
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
    }

    # << Expects waits for an irc command {args}
    method << {args} {
        $self make_current
        set match false

        while {$match != true} {
            set line [irc_tokenize [chan gets $sock]]
            $self handle_line $line

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

proxy_method JOIN
proxy_method QUIT
proxy_method <<
proxy_method >>

puts {Beginning test}

source [lindex $argv 0]

puts {Test finished}

# Cleanup open clients
foreach x $all_clients {
    $x destroy
}
