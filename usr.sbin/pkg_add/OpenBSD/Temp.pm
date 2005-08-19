# ex:ts=8 sw=4:
# $OpenBSD: Temp.pm,v 1.4 2005/08/19 00:09:51 espie Exp $
#
# Copyright (c) 2003-2004 Marc Espie <espie@openbsd.org>
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

use strict;
use warnings;
package OpenBSD::Temp;

use File::Temp;
use File::Path;

our $tempbase = $ENV{'PKG_TMPDIR'} || '/var/tmp';

my $dirs = [];

$SIG{'INT'} = sub {
	File::Path::rmtree($dirs);
	$SIG{'INT'} = 'DEFAULT';
	kill 'INT', $$;
};

sub dir()
{
	my $dir = File::Temp::tempdir("pkginfo.XXXXXXXXXXX", DIR => $tempbase,
	    CLEANUP => 1).'/';
	push(@$dirs, $dir);
	return $dir;
}

sub list($)
{
	return File::Temp::tempfile("list.XXXXXXXXXXX", DIR => shift);
}

1;
