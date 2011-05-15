use strict;
use Irssi;
use Irssi::Irc;
use vars qw($VERSION %IRSSI);

$VERSION = "1.0";

%IRSSI = (
    authors     => 'JD Horelick',
    contact     => 'jdhore1@gmail.com',
    name        => 'shadow_help.pl',
    description => 'Adds a OHELP command so that you do not need to type /quote help',
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

Irssi::command_bind('ohelp', \&cmd_ohelp);
# vim: ts=4
