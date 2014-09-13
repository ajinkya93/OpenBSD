# The client writes a message to Sys::Syslog native method.
# The syslogd writes it into a file and through a pipe.
# The syslogd -4 does not pass it via IPv6 UDP to the loghost.
# Find the message in client, file, pipe, syslogd log.
# Check that the syslogd logs the error.

use strict;
use warnings;

our %args = (
    syslogd => {
	loghost => '@udp6://[::1]',
	loggrep => {
	    qr/syslogd: no udp6 "\@udp6:\/\/\[::1\]/ => 2,
	    get_testlog() => 1,
	},
	options => ["-4"],
    },
    server => {
	noserver => 1,
    },
);

1;
