# The syslogd listens on 127.0.0.1 TLS socket.
# The TCP client writes cleartext into the TLS connection to syslogd.
# The client connects and closes the connection to syslogd.
# The syslogd writes the error into a file and through a pipe.
# Find the error message in file, syslogd log.
# Check that syslogd writes a log message about the SSL connect error.

use strict;
use warnings;
use Socket;

our %args = (
    client => {
	connect => { domain => AF_INET, proto => "tcp", addr => "127.0.0.1",
	    port => 6514 },
	func => sub {
	    my $self = shift;
	    print "Writing cleartext into a TLS connection is a bad idea\n";
	    ${$self->{syslogd}}->loggrep("tls logger .* connection error", 5)
		or die "no connection error in syslogd.log";
	},
	loggrep => {
	    qr/connect sock: 127.0.0.1 \d+/ => 1,
	},
    },
    syslogd => {
	options => ["-S", "127.0.0.1:6514"],
	loggrep => {
	    qr/syslogd: tls logger .* accepted/ => 1,
	    qr/syslogd: tls logger .* connection error/ => 1,
	},
    },
    server => {
	func => sub {
	    my $self = shift;
	    ${$self->{syslogd}}->loggrep("tls logger .* connection error", 5)
		or die "no connection error in syslogd.log";
	},
	loggrep => {},
    },
    pipe => {
	loggrep => {},
    },
    file => {
	loggrep => {
	    qr/syslogd: tls logger .* connection error: /.
		qr/handshake failed: error:.*/.
		qr/SSL routines:SSL23_GET_CLIENT_HELLO:unknown protocol/ => 1,
	},
    },
);

1;
