# ex:ts=8 sw=4:
# $OpenBSD: Delete.pm,v 1.29 2005/10/24 09:50:51 espie Exp $
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
package OpenBSD::Delete;
use OpenBSD::Error;
use OpenBSD::Vstat;
use OpenBSD::PackageInfo;
use OpenBSD::RequiredBy;
use File::Basename;

sub keep_old_files
{
	my ($state, $plist, $dir) = @_;
	my $p = new OpenBSD::PackingList;
	for my $i (qw(cvstags name no-default-conflict pkgcfl conflict) ) {
		if (defined $plist->{$i}) {
			$p->{$i} = $plist->{$i};
		}
	}
	for my $i (@{$plist->{items}}) {
		if ($i->isa("OpenBSD::PackingElement::Cwd")) {
			push(@{$p->{items}}, $i);
			next;
		}
		next unless $i->IsFile();
		if (defined $i->{stillaround}) {
			delete $i->{stillaround};
			if ($state->{replacing}) {
				require File::Temp;

				my $n = $i->fullname();

				my ($fh, $j) = File::Temp::mkstemp("$n.XXXXXXXX");
				close $fh;
				if (rename($n, $j)) {
					print "Renaming old file $n to $j\n";
					if ($i->{name} !~ m|^/| && $i->cwd() ne '.') {
						my $c = $i->cwd();
						$j =~ s|^\Q$c\E/||;
					}
					$i->{name} = $j;
				} else {
					print "Bad rename $n to $j: $!\n";
				}
			}
			push(@{$p->{items}}, $i);
		}
	}
	my $borked = borked_package($plist->pkgname());
	$p->{name}->{name} = $borked;
	my $dest = installed_info($borked);
	mkdir($dest);
	require File::Copy;

	File::Copy::copy($dir.COMMENT, $dest);
	File::Copy::copy($dir.DESC, $dest);
	$p->to_installation();
	return $borked;
}

sub manpages_unindex
{
	my ($state) = @_;
	return unless defined $state->{mandirs};
	my $destdir = $state->{destdir};
	require OpenBSD::Makewhatis;

	while (my ($k, $v) = each %{$state->{mandirs}}) {
		my @l = map { $destdir.$_ } @$v;
		if ($state->{not}) {
			print "Removing manpages in $destdir$k: ", join(@l), "\n" if $state->{verbose};
		} else {
			eval { OpenBSD::Makewhatis::remove($destdir.$k, \@l); };
			if ($@) {
				print STDERR "Error in makewhatis: $@\n";
			}
		}
	}
	undef $state->{mandirs};
}

sub validate_plist($$)
{
	my ($plist, $state) = @_;

	my $destdir = $state->{destdir};
	my $problems = 0;
	my $totsize = 0;
	for my $item (@{$plist->{items}}) {
		next unless $item->IsFile();
		my $fname = $destdir.$item->fullname();
		$totsize += $item->{size} if defined $item->{size};
		my $s = OpenBSD::Vstat::remove($fname, $item->{size});
		next unless defined $s;
		if ($s->{ro}) {
			if ($state->{very_verbose} or ++($s->{problems}) < 4) {
				Warn "Error: ", $s->{dev}, 
				    " is read-only ($fname)\n";
			} elsif ($s->{problems} == 4) {
				Warn "Error: ... more files can't be removed from ",
					$s->{dev}, "\n";
			}
			$problems++;
		}
	}
	my $dir = installed_info($plist->pkgname());
	for my $i (info_names()) {
		my $fname = $dir.$i;
		if (-e $fname) {
			my $size = (stat _)[7];
			my $s = OpenBSD::Vstat::remove($fname, $size);
			next unless defined $s;
			if ($s->{ro}) {
				Warn "Error: ", $s->{dev}, " is read-only ($fname)\n";
				$problems++;
			}
		}
	}
	Fatal "fatal issues" if $problems;
	$totsize = 1 if $totsize == 0;
	$plist->{totsize} = $totsize;
}

sub remove_packing_info
{
	my $dir = shift;

	for my $fname (info_names()) {
		unlink($dir.$fname);
	}
	OpenBSD::RequiredBy->forget($dir);
	OpenBSD::Requiring->forget($dir);
	rmdir($dir) or Fatal "Can't finish removing directory $dir: $!";
}

sub delete_package
{
	my ($pkgname, $state) = @_;
	OpenBSD::ProgressMeter::message("reading plist");
	my $plist = OpenBSD::PackingList->from_installation($pkgname) or
	    Fatal "Bad package";
	if (!defined $plist->pkgname()) {
		Fatal "Package $pkgname has no name";
	}
	if ($plist->pkgname() ne $pkgname) {
		Fatal "Package $pkgname real name does not match";
	}

	validate_plist($plist, $state);

	delete_plist($plist, $state);
}

sub delete_plist
{
	my ($plist, $state) = @_;

	my $totsize = $plist->{totsize};
	my $pkgname = $plist->pkgname();
	$state->{pkgname} = $pkgname;
	my $dir = installed_info($pkgname);
	$state->{dir} = $dir;
	$ENV{'PKG_PREFIX'} = $plist->pkgbase();
	if ($plist->has(REQUIRE)) {
		$plist->get(REQUIRE)->delete($state);
	}
	if ($plist->has(DEINSTALL)) {
		$plist->get(DEINSTALL)->delete($state);
	} 
	$plist->visit('register_manpage', $state);
	manpages_unindex($state);
	my $donesize = 0;
	for my $item (@{$plist->{groups}}, @{$plist->{users}}, @{$plist->{items}}) {
		$item->delete($state);
		if (defined $item->{size}) {
                        $donesize += $item->{size};
                        OpenBSD::ProgressMeter::show($donesize, $totsize);
                }
	}

	OpenBSD::ProgressMeter::next();
	if ($plist->has(UNDISPLAY)) {
		$plist->get(UNDISPLAY)->prepare($state);
	}

	# guard against duplicate pkgdep
	my $removed = {};

	my $zap_dependency = sub {
		my $name = shift;

		return if defined $removed->{$name};
		print "remove dependency on $name\n" 
		    if $state->{very_verbose} or $state->{not};
		local $@;
		try { 
			OpenBSD::RequiredBy->new($name)->delete($pkgname);
		} catchall {
			print STDERR "$_\n";
		};
		$removed->{$name} = 1;
	};

	for my $item (@{$plist->{pkgdep}}) {
		&$zap_dependency($item->{name});
	}
	for my $name (OpenBSD::Requiring->new($pkgname)->list()) {
		&$zap_dependency($name);
	}
		
	return if $state->{not};
	if ($state->{baddelete}) {
	    my $borked = keep_old_files($state, $plist, $dir);
	    $state->print("Files kept as $borked package\n");
	    delete $state->{baddelete};
	}
			

	remove_packing_info($dir);
}

package OpenBSD::PackingElement;

sub delete
{
}

package OpenBSD::PackingElement::FileObject;
use File::Basename;

sub mark_directory
{
	my ($self, $state, $dir) = @_;

	$state->{dirs_okay}->{$dir} = 1;
	my $d2 = dirname($dir);
	if ($d2 ne $dir) {
		$self->mark_directory($state, $d2);
	}
}

sub mark_dir
{
	my ($self, $state) = @_;

	$self->mark_directory($state, dirname($self->fullname()));
}

sub realname
{
	my ($self, $state) = @_;

	my $name = $self->fullname();
	if (defined $self->{tempname}) {
		$name = $self->{tempname};
	}
	return $state->{destdir}.$name;
}

package OpenBSD::PackingElement::DirlikeObject;
sub mark_dir
{
	my ($self, $state) = @_;
	$self->mark_directory($state, $self->fullname());
}

package OpenBSD::PackingElement::NewUser;
sub delete
{
	my ($self, $state) = @_;

	my $name = $self->{name};

	if ($state->{beverbose}) {
		print "rmuser: $name\n";
	}

	$state->{users_to_rm} = {} unless defined $state->{users_to_rm};

	my $h = $state->{users_to_rm};
	$h->{$name} = $state->{pkgname};
}

package OpenBSD::PackingElement::NewGroup;
sub delete
{
	my ($self, $state) = @_;

	my $name = $self->{name};

	if ($state->{beverbose}) {
		print "rmgroup: $name\n";
	}

	$state->{groups_to_rm} = {} unless defined $state->{groups_to_rm};

	my $h = $state->{groups_to_rm};
	$h->{$name} = $state->{pkgname};
}

package OpenBSD::PackingElement::DirBase;
sub delete
{
	my ($self, $state) = @_;

	my $name = $self->fullname();

	if ($state->{very_verbose}) {
		print "dirrm: $name\n";
	}

	$state->{dirs_to_rm} = {} unless defined $state->{dirs_to_rm};

	my $h = $state->{dirs_to_rm};
	$h->{$name} = [] unless defined $h->{$name};
	$self->{pkgname} = $state->{pkgname};
	push(@{$h->{$name}}, $self);
}

package OpenBSD::PackingElement::DirRm;
sub delete
{
	&OpenBSD::PackingElement::DirBase::delete;
}

package OpenBSD::PackingElement::Unexec;
sub delete
{
	my ($self, $state) = @_;
	$self->run($state);
}

package OpenBSD::PackingElement::FileBase;
use OpenBSD::md5;
sub delete
{
	my ($self, $state) = @_;
	my $realname = $self->realname($state);

	if (defined $self->{symlink}) {
		if (-l $realname) {
			my $contents = readlink $realname;
			if ($contents ne $self->{symlink}) {
				print "Symlink does not match: $realname ($contents vs. ", $self->{symlink},")\n";
				$self->{stillaround} = 1;
				$self->{symlink} = $contents;
				$state->{baddelete} = 1;
				return;
			}
		} else  {
			print "Bogus symlink: $realname\n";
			if (-f $realname) {
				delete $self->{symlink};
				$self->{md5} = OpenBSD::md5::fromfile($realname);
				$self->{stillaround} = 1;
			}
			$state->{baddelete} = 1;
			return;
		}
	} else {
		if (-l $realname) {
				print "Unexpected symlink: $realname\n";
				$self->{stillaround} = 1;
				$state->{baddelete} = 1;
		} else {
			if (! -f $realname) {
				print "File $realname does not exist\n";
				return;
			}
			unless (defined($self->{link}) or $self->{nochecksum} or $state->{quick}) {
				if (!defined $self->{md5}) {
					print "Problem: ", $self->fullname(),
					    " does not have an md5 checksum\n";
					print "NOT deleting: $realname\n";
					$state->print("Couldn't delete $realname (no md5)\n");
					return;
				}
				my $md5 = OpenBSD::md5::fromfile($realname);
				if ($md5 ne $self->{md5}) {
					print "Problem: md5 doesn't match for ",
						$self->fullname(), "\n";
					print "NOT deleting: $realname\n";
					$state->print("Couldn't delete $realname (bad md5)\n");
					$self->{stillaround} = 1;
					$self->{md5} = $md5;
					$state->{baddelete} = 1;
					return;
				}
			}
		}
	}
	if ($state->{very_verbose}) {
		print "deleting: $realname\n";
	}
	return if $state->{not};
	if (!unlink $realname) {
		print "Problem deleting $realname\n";
		$state->print("deleting $realname failed: $!\n");
	}
}

package OpenBSD::PackingElement::Sample;
use OpenBSD::md5;
use OpenBSD::Error;
use File::Basename;

sub delete
{
	my ($self, $state) = @_;
	my $realname = $self->realname($state);

	my $orig = $self->{copyfrom};
	if (!defined $orig) {
		Fatal "\@sample element does not reference a valid file\n";
	}
	my $origname = $orig->realname($state);
	if (! -e $realname) {
		$state->print("File $realname does not exist\n");
		return;
	}
	if (! -f $realname) {
		$state->print("File $realname is not a file\n");
		return;
	}

	if (!defined $orig->{md5}) {
		$state->print("Couldn't delete $realname (no md5)\n");
		return;
	}

	if ($state->{quick}) {
		unless ($state->{extra}) {
			$self->mark_dir($state);
			$state->print("You should also remove $realname\n");
			return;
		}
	} else {
		my $md5 = OpenBSD::md5::fromfile($realname);
		if ($md5 eq $orig->{md5}) {
			print "File $realname identical to sample\n" if $state->{not} or $state->{verbose};
		} else {
			unless ($state->{extra}) {
				$self->mark_dir($state);
				$state->print("You should also remove $realname (which was modified)\n");
				return;
			}
		}
	}
	return if $state->{not};
	print "deleting $realname\n" if $state->{verbose};
	if (!unlink $realname) {
		print "Problem deleting $realname\n";
		$state->print("deleting $realname failed: $!\n");
	}
}
		

package OpenBSD::PackingElement::InfoFile;
use File::Basename;
use OpenBSD::Error;
sub delete
{
	my ($self, $state) = @_;
	unless ($state->{not}) {
	    my $fullname = $state->{destdir}.$self->fullname();
	    VSystem($state->{very_verbose}, 
	    "install-info", "--delete", "--info-dir=".dirname($fullname), $fullname);
	}
	$self->SUPER::delete($state);
}

package OpenBSD::PackingElement::Shell;
sub delete
{
	my ($self, $state) = @_;
	unless ($state->{not}) {
		my $destdir = $state->{destdir};
		my $fullname = $self->fullname();
		my @l=();
		if (open(my $shells, '<', $destdir.'/etc/shells')) {
			local $_;
			while(<$shells>) {
				push(@l, $_);
				s/^\#.*//;
				if ($_ =~ m/^\Q$fullname\E\s*$/) {
					pop(@l);
				}
			}
			close($shells);
			open(my $shells2, '>', $destdir.'/etc/shells');
			print $shells2 @l;
			close $shells2;
			print "Shell $fullname removed from $destdir/etc/shells\n";
		}
	}
	$self->SUPER::delete($state);
}

package OpenBSD::PackingElement::Extra;
use File::Basename;

sub delete
{
	my ($self, $state) = @_;
	my $realname = $self->realname($state);
	if ($state->{beverbose} && $state->{extra}) {
		print "deleting extra file: $realname\n";
	}
	return if $state->{not};
	return unless -e $realname;
	if ($state->{replacing}) {
		$state->print("Remember to update $realname\n");
		$self->mark_dir($state);
	} elsif ($state->{extra}) {
		unlink($realname) or 
		    print "problem deleting extra file $realname\n";
	} else {
		$state->print("You should also remove $realname\n");
		$self->mark_dir($state);
	}
}


package OpenBSD::PackingElement::Extradir;
sub delete
{
	my ($self, $state) = @_;
	return unless $state->{extra};
	my $realname = $self->realname($state);
	return if $state->{replacing};
	if ($state->{extra}) {
		$self->SUPER::delete($state);
	} else {
		$state->print("You should also remove the directory $realname\n");
		$self->mark_dir($state);
	}
}

package OpenBSD::PackingElement::ExtraUnexec;

sub delete
{
	my ($self, $state) = @_;
	if ($state->{extra}) {
		$self->run($state);
	} else {
		$state->print("You should also run ", $self->{expanded}, "\n");
	}
}

package OpenBSD::PackingElement::Lib;
sub delete
{
	my ($self, $state) = @_;
	$self->SUPER::delete($state);
	$self->mark_ldconfig_directory($state->{destdir});
}

package OpenBSD::PackingElement::FREQUIRE;
sub delete
{
	my ($self, $state) = @_;

	$self->run($state, "DEINSTALL");
}

package OpenBSD::PackingElement::FDEINSTALL;
sub delete
{
	my ($self, $state) = @_;

	$self->run($state, "DEINSTALL");
}

1;
