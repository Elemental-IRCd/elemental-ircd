package require TclOO

source lib/numeric.tcl

expect_before {
    timeout {send_error "Timed out"; exit 1}
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

oo::class create client {
    constructor {{ip 127.0.0.1} {port 6667}} {
        variable my_spawn_id
        variable nickname nick
        variable username user

        spawn nc $ip $port
        set my_spawn_id $spawn_id
        my make_current
    }

    method register {} {
        variable nickname
        variable username

        my send_cmd NICK $nickname
        my send_cmd USER $username * * {Real Name}
        my expect_rpl RPL_WELCOME
    }

    method quit {} {
        my send_cmd QUIT
        my expect_cmd ERROR
    }

    method send_cmd {args} {
        my send "[format_args {*}$args]\r"
    }

    method expect_rpl {numeric {text {}}} {
        global $numeric
        variable nickname

        my expect -re [format {:[^ ]+ %s %s ?:?%s} [set $numeric] $nickname $text]
    }

    method expect_cmd {command} {
        my expect -re [format {(:[^ ]+ +)?%s} $command]
    }

    method make_current {} {
        global spawn_id
        global current_client
        variable my_spawn_id

        set current_client [self]
        set spawn_id $my_spawn_id
    }

    method send {args} {
        variable my_spawn_id
        ::send -i $my_spawn_id {*}$args
    }

    method expect {args} {
        variable my_spawn_id
        ::expect -i $my_spawn_id {*}$args
    }

}

proc proxy_method {method} {
    proc $method {args} "
        global current_client
        \$current_client $method {*}\$args
    "
}

proxy_method register
proxy_method quit

proxy_method expect_cmd
proxy_method expect_rpl
