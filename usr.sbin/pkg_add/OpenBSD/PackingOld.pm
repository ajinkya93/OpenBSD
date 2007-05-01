# ex:ts=8 sw=4:
# $OpenBSD: PackingOld.pm,v 1.9 2007/05/01 18:07:00 espie Exp $
#
# Copyright (c) 2004-2006 Marc Espie <espie@openbsd.org>
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
use OpenBSD::PackingElement;

package OpenBSD::PackingElement::Old;
our @ISA=qw(OpenBSD::PackingElement);

sub add
{
	my ($class, $plist, @args) = @_;

	my $self = $class->new(@args);
	print STDERR "Warning: obsolete construct: ", $self->fullstring(), "\n";
	return $self->add_object($plist);
}

package OpenBSD::PackingElement::Src;
our @ISA=qw(OpenBSD::PackingElement::Old);


sub keyword() { 'src' }
__PACKAGE__->register_with_factory;

package OpenBSD::PackingElement::Display;
our @ISA=qw(OpenBSD::PackingElement::Old);

sub keyword() { 'display' }
__PACKAGE__->register_with_factory;

package OpenBSD::PackingElement::Mtree;
our @ISA=qw(OpenBSD::PackingElement::Old);

sub keyword() { 'mtree' }
__PACKAGE__->register_with_factory;

package OpenBSD::PackingElement::ignore_inst;
our @ISA=qw(OpenBSD::PackingElement::Old);

sub keyword() { 'ignore_inst' }
__PACKAGE__->register_with_factory;

1;
