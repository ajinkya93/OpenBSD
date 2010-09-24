#!./perl

BEGIN {
    unless (-d 'blib') {
	chdir 't' if -d 't';
	@INC = '../lib';
	require Config; import Config;
	keys %Config; # Silence warning
	if ($Config{extensions} !~ /\bList\/Util\b/) {
	    print "1..0 # Skip: List::Util was not built\n";
	    exit 0;
	}
    }
}

use Scalar::Util ();
use Test::More  (grep { /dualvar/ } @Scalar::Util::EXPORT_FAIL)
			? (skip_all => 'dualvar requires XS version')
			: (tests => 13);

Scalar::Util->import('dualvar');

$var = dualvar( 2.2,"string");

ok( $var == 2.2,	'Numeric value');
ok( $var eq "string",	'String value');

$var2 = $var;

ok( $var2 == 2.2,	'copy Numeric value');
ok( $var2 eq "string",	'copy String value');

$var++;

ok( $var == 3.2,	'inc Numeric value');
ok( $var ne "string",	'inc String value');

my $numstr = "10.2";
my $numtmp = int($numstr); # use $numstr as an int

$var = dualvar($numstr, "");

ok( $var == $numstr,	'NV');

SKIP: {
  skip("dualvar with UV value known to fail with $]",2) if $] < 5.006_001;
  $var = dualvar(1<<31, "");
  ok( $var == (1<<31),	'UV 1');
  ok( $var > 0,		'UV 2');
}


{
  package Tied;

  sub TIESCALAR { bless {} }
  sub FETCH { 7.5 }
}

tie my $tied, 'Tied';
$var = dualvar($tied, "ok");
ok($var == 7.5,		'Tied num');
ok($var eq 'ok',	'Tied str');


SKIP: {
  skip("need utf8::is_utf8",2) unless defined &utf8::is_utf8;
  ok(!!utf8::is_utf8(dualvar(1,chr(400))), 'utf8');
  ok( !utf8::is_utf8(dualvar(1,"abc")),    'not utf8');
}
