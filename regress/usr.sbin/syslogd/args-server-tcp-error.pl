# The TCP server aborts the connection to syslogd.
# The client writes a message to Sys::Syslog native method.
# The syslogd writes it into a file and through a pipe.
# The syslogd passes it via IPv4 TCP to an explicit loghost.
# The server receives the message on its TCP socket.
# Find the message in client, pipe, syslogd log.
# Check that syslogd writes a log message about the server error.

use strict;
use warnings;
use Socket;

our %args = (
    client => {
	func => sub {
	    my $self = shift;
	    ${$self->{syslogd}}->loggrep("loghost .* connection error", 2)
		or die "connection error in syslogd.log";
	    write_log($self, @_);
	},
    },
    syslogd => {
	loghost => '@tcp://127.0.0.1:$connectport',
	loggrep => {
	    qr/Logging to FORWTCP \@tcp:\/\/127.0.0.1:\d+/ => '>=4',
	    get_testlog() => 1,
	    qr/syslogd: loghost .* connection error/ => 2,
	},
    },
    server => {
	listen => { domain => AF_INET, proto => "tcp", addr => "127.0.0.1" },
	func => sub {
	    my $self = shift;
	    setsockopt(STDOUT, SOL_SOCKET, SO_LINGER, pack('ii', 1, 0))
		or die "set socket linger failed: $!";
	},
	loggrep => {},
    },
    file => {
	loggrep => {
	    qr/syslogd: loghost .* connection error: Connection reset by peer/
		=> 1,
	},
    },
);

1;
