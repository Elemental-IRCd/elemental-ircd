# This script is something just thought up by me (jdhore) and ported from irssi to WeeChat

weechat::register("shadow_help", "jdhore", "1.0", "BSD", "Make life easier when using various oper commands", "", "");

weechat::hook_command("ohelp", "Rewriting the IRCd's HELP command for fun and profit",
	"[params]", 
	"" .
	"",
	"", "ohelp", "");
weechat::hook_command("oset", "Rewriting the IRCd's SET command for fun and profit",
	"[params]", 
	"" .
	"",
	"", "oset", "");

sub ohelp {
    my ($data, $buffer, $param) = @_;

    if ($param) {
		weechat::command($buffer, "/quote help $param");
    }
    else {
        weechat::command($buffer, "/quote help");
    }
    
    return weechat::WEECHAT_RC_OK;
}

sub oset {
    my ($data, $buffer, $param) = @_;

    if ($param) {
		weechat::command($buffer, "/quote set $param");
    }
    else {
        weechat::command($buffer, "/quote set");
    }
    
    return weechat::WEECHAT_RC_OK;
}
