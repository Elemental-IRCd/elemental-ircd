package IRC::Xchat::Shadow_Help;

IRC::register("Shadow Help", "1.0", "", "");

sub ohelp {
	$_ = shift @_;
	IRC::command ("/quote help $_");
    return 1;
}

sub oset {
	$_ = shift @_;
	IRC::command ("/quote set $_");
    return 1;
}

IRC::add_command_handler("ohelp", "IRC::Xchat::Shadow_Help::ohelp");
IRC::add_command_handler("oset", "IRC::Xchat::Shadow_Help::oset");
