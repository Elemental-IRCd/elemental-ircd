package require snit

source lib/numeric.tcl


# Test servers
#   [name] {[ip/host] [port]}
array set servers {
    hub    {127.0.0.1 6667}
    leaf1  {127.0.0.1 6668}
    leaf2  {127.0.0.1 6669}
}

proc begin {test name {text {Test suite client}}} {
    global test_name
    global test_desc
    set test_name $name
    set test_desc $text
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

# explicit timeout
set timeout 10

#HACK: this triggers a stack trace
#is there a proper way? one that won't say "invalid command name"?
expect_before {
    timeout {{Timed out}}
}

# Tokenize an irc message into a tcl list
proc parse {line} {
    variable pos 0
    variable next 0
    variable args {}
    variable length [string length $line]

    if {[string index $line 0] == ":"} then {
        incr pos
    }

    while true {
        set next [string first " " $line $pos]
        if {$next == -1 || [string index $line $pos] == ":" } then {
            set next $length
        }

        lappend args [string range $line $pos [expr $next - 1]]
        set pos [expr $next + 1]

        if {$next >= $length} then {return $args}
    }
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

    #TODO: \r\n, expect seems to turn every \r into a \n
    return [join $out]
}

snit::type client {
    variable my_spawn_id
    variable nickname
    variable username
    variable realname

    constructor {{server hub}} {
        global servers

        spawn nc {*}$servers($server)
        set my_spawn_id $spawn_id
        set nickname [get_nick]
        set username [get_user]
        set realname [get_realname]

        $self make_current
        $self register
    }

    method nick {} {
        return $nickname
    }

    method register {} {
        $self send_cmd NICK $nickname
        $self send_cmd USER $username * * $realname
        $self expect_rpl RPL_WELCOME
    }

    method quit {} {
        $self send_cmd QUIT
        $self expect_cmd ERROR
    }

    method join_channel {channel} {
        $self send_cmd JOIN $channel
        $self expect -re "JOIN :?$channel"
    }

    method oper {} {
        $self send_cmd OPER oper testsuite
        $self expect "is now an operator"
    }

    method send_cmd {args} {
        $self send "[format_args {*}$args]\r"
    }

    method expect_rpl {numeric {text {}}} {
        global $numeric

        $self expect -re [format {:[^ ]+ %s %s ?:?%s} [set $numeric] $nickname $text]
    }

    method expect_cmd {command} {
        $self expect -re [format {(:[^ ]+ +)?%s} $command]
    }

    method make_current {} {
        global spawn_id
        global current_client

        set current_client $self
        set spawn_id $my_spawn_id
    }

    method send {args} {
        $self make_current
        ::send {*}$args
    }

    method expect {args} {
        $self make_current
        ::expect {*}$args
    }

}

proc proxy_method {method} {
    proc $method {args} "
        global current_client
        \$current_client $method {*}\$args
    "
}

proxy_method oper
proxy_method join_channel
proxy_method quit

proxy_method expect_cmd
proxy_method expect_rpl
