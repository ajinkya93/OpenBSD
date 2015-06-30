# Syslog binds UDP socket to localhost with -6.
# The client writes a message to a localhost IPv6 UDP socket.
# The syslogd writes it into a file and through a pipe.
# The syslogd passes it via UDP to the loghost.
# The server receives the message on its UDP socket.
# Find the message in client, file, pipe, syslogd, server log.
# Check that the file log contains the ::1 name.
# Check that fstat contains only a bound IPv6 UDP socket.

use strict;
use warnings;

our %args = (
    client => {
	connect => { domain => AF_INET6, addr => "::1", port => 514 },
    },
    syslogd => {
	options => ["-6nU", "localhost"],
	fstat => {
	    qr/^root .* internet/ => 0,
	    qr/^_syslogd .* internet/ => 2,
	    qr/ internet dgram udp 127.0.0.1:514$/ => 0,
	    qr/ internet6 dgram udp \[::1\]:514$/ => 1,
	},
	loghost => '@[::1]:$connectport',
    },
    server => {
	listen => { domain => AF_INET6, addr => "::1" },
    },
    file => {
	loggrep => qr/ ::1 /. get_testlog(),
    },
);

1;
