#!/bin/sh
# $OpenBSD: keywords.sh,v 1.14 2005/03/30 06:12:38 henning Exp $
# $NetBSD: keywords.sh,v 1.2 1996/11/15 18:57:21 gwr Exp $
# @(#)keywords	8.2 (Berkeley) 3/19/94
#
# WARNING!  If you change this file, re-run it!

# This program requires "new" awk (or GNU awk).
awk=${AWK:-awk}

cat << _EOF_ > _keywords.t1
add
blackhole
change
cloning
delete
dst
expire
flush
gateway
genmask
get
host
hopcount
iface
interface
ifa
ifp
inet
inet6
ipx
label
link
llinfo
lock
lockrest
monitor
mpath
mtu
net
netmask
nostatic
prefixlen
proto1
proto2
recvpipe
reject
rtt
rttvar
sa
sendpipe
show
ssthresh
static
xresolve
_EOF_


################################################################
# Setup
################################################################

# This creates a stream of:
#	keyword KEYWORD
# (lower case, upper case).
tr a-z A-Z < _keywords.t1 |
paste _keywords.t1 - > _keywords.t2

################################################################
# Generate the h file
################################################################
exec > keywords.h

echo '/* $'OpenBSD'$ */

/* WARNING!  This file was generated by keywords.sh  */

struct keytab {
        char    *kt_cp;
        int      kt_i;
};
'

$awk '{
	printf("#define\tK_%s\t%d\n", $2, NR);
}' < _keywords.t2

echo '
struct keytab keywords[] = {'

$awk '{
	printf("\t{ \"%s\",\tK_%s },\n", $1, $2);
}' < _keywords.t2

echo '	{ 0, 0 }
};
' # tail


################################################################
# Cleanup
################################################################

rm -f _keywords.t1 _keywords.t2
exit 0
