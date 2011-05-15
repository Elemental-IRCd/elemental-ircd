use strict;
use Irssi;
use Irssi::Irc;
use vars qw($VERSION %IRSSI);

$VERSION = "1.0";

%IRSSI = (
    authors     => 'JD Horelick',
    contact     => 'jdhore1@gmail.com',
    name        => 'shadow_help.pl',
    description => 'Adds an OHELP and OSET command to make using the IRCd-side HELP and SET commands easier',
    license     => 'GNU General Public License',
    url         => 'http://shadowircd.net/',
);

sub cmd_ohelp {
	my ($data, $server) = @_;

	if (!$server || !$server->{connected}) {
      Irssi::print("Not connected to server");
      return;
    }

	$server->send_raw("HELP $data");
}

sub cmd_oset {
	my ($data, $server) = @_;

	if (!$server || !$server->{connected}) {
      Irssi::print("Not connected to server");
      return;
    }

	$server->send_raw("SET $data");
}

Irssi::command_bind('ohelp', \&cmd_ohelp);
Irssi::command_bind('oset', \&cmd_oset);
# vim: ts=4
