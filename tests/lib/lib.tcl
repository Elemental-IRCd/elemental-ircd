

#global defaults
set nickname nickname
set username username

source lib/numeric.tcl

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

# Assemble a command from arguments
# handles line endings and trailing
proc send_cmd {args} {
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
    send "[join $out]\r"
}

expect_before {
    timeout {send_error "Timed out"; exit 1}
}

proc expect_rpl {numeric {text {}}} {
    global nickname
    global $numeric
    expect -re [format {:[^ ]+ %s %s ?:?%s} [set $numeric] $nickname $text]
}

proc expect_cmd {command} {
    expect -re [format {(:[^ ]+ +)?%s} $command]
}

proc register {} {
    global nickname
    global username
    send_cmd NICK $nickname
    send_cmd USER $username * * {Real Name}
    expect_rpl RPL_WELCOME
}

proc quit {} {
    send_cmd QUIT
    expect_cmd ERROR
}
