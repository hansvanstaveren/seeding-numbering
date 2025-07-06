$f1 = $ARGV[0];
$f2 = $ARGV[1];

# print "$f1, $f2\n";

open ns, $f1 || die;
open ew, $f2 || die;

$nspairno=1;
$ewpairno=21;
print "Pair	Tbl	As	M-ID\n";
while (<ns>) {
    $nsline = $_;
    $ewline = <ew>;
    chop $nsline;
    chop $ewline;
    ($nspair, $rest) = split /,/, $nsline;
    ($ewpair, $rest) = split /,/, $ewline;
    ($n, $s) = split /-/, $nspair;
    ($e, $w) = split /-/, $ewpair;
    print "		North	$n\n";
    print "		South	$s\n";
    print "		East	$e\n";
    print "		West	$w\n";
}


