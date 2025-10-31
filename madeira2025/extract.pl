#!perl -w

#$pairsout=0;
#
# Groups for wheelchairs
#
$wcgroup{0} = 0;
$wcgroup{"A"} = 1;
$wcgroup{"B"} = 3;
$wcgroup{"C"} = 9;

@strengthdistr = ( 0, 0, 0, 0, 0, 0);
#
# Check for duplicate of player.
# Scoring program and registration software does not do this
#
sub is_present {
    local ($player, $lineno) =  @_;

    $prevline = $present{$player};
    if ($prevline) {
	print STDERR "Player $player in movement line $prevline and $lineno\n";
	return 1;
    }
    $present{$player} = $lineno;
    return 0;
}

sub num_strength {
    my ($strength) = @_;
    my ($numstr);

    $numstr = 0;
    $numstr = 1 if ($strength =~ /WORLD/i);
    $numstr = 2 if ($strength =~ /EXPERT/i);
    $numstr = 3 if ($strength =~ /STRONG/i);
    $numstr = 4 if ($strength =~ /ADVANCED/i);
    $numstr = 5 if ($strength =~ /SOCIAL/i);

    return $numstr if ($numstr);
    print STDERR "Strength $strength unknown\n";
    return 0;
}

sub avg_strength {
    my ($s1, $s2) = @_;

    my $ns1 = num_strength($s1);
    my $ns2 = num_strength($s2);
    return (int(($ns1+$ns2)/2));
}

open (ERRORS, ">errors");
open (STATS, ">statistics");
open (SEEDIN, ">seedin");

#
# Read and decode CSV version of registration spreadsheet
#
# Varies year after year ...
#
open(PAIRS, "pairs.csv") || die;
$lineno = 0;
$nerrors = 0;
while (<PAIRS>) {
    $lineno++;
    # print "$lineno $_\n";
    next if /^BM/i;			# Header
    s/\r$//;
    chomp;
    # Wheelchair hack
    if (/;WC([A-Z]) */) {
	$wheelchair = $1;
	s/;WC[A-Z] */;/;
    } else {
	$wheelchair = 0;
    }
    @fields = split /;/;
    # print @fields, "$#fields columns\n";
    # $id1 = $fields[0];
    $p1 = $fields[0];
    # $id2 = $fields[4];
    $p2 = $fields[7];
    # print "p1-p2 $p1-$p2\n";
    # print STDERR "line $lineno: $p1, $unused, $strength, $nationality, $p2\n";
    if ($p1 !~ /^[0-9]+$/ || $p2 !~ /^[0-9]+$/ || $p1 <= 0 || $p1 >=100000 || $p2 <= 0 || $p2 >= 100000) {
	print ERRORS "Line $lineno: $_\n";
	$nerrors++;
	next;
    }
    next if is_present($p1, $lineno);
    next if is_present($p2, $lineno);
    $nationality = $fields[5];
    $strength1 = $fields[6];
    $strength2 = $fields[13];
    if (!$nationality || !$strength1 || !$strength2) {
	print ERRORS "Line $lineno, player $p1, nationality or strength unknown\n";
	$nerrors++;
	next;
    }

    $grp = $wheelchair ? $wcgroup{$wheelchair} : "" ;

    # Remove leading and trailing spaces from nationality field
    $nationality =~ s/^ +//;
    $nationality =~ s/ +$//;

    $numstr = avg_strength($strength1, $strength2);

    $strengthdistr[$numstr]++;
    $natdistr{$nationality}++;
    $entwcgroup{$grp}++ if($wheelchair);
    print SEEDIN  "$p1-$p2,$grp,$numstr,$nationality\n";
    $pairsout++;
}
close(PAIRS);

if ($nerrors) {
    print "There were $nerrors errors, see file errors!\n";
}

for $str (1..5) {
    print STATS "strength $str: ", $strengthdistr[$str], "\n";
}
foreach my $key (sort keys %natdistr) {
    print STATS "$key:" , $natdistr{$key} , "\n";
}
foreach my $key (sort keys %entwcgroup) {
    print STATS "wheelchairs in group $key:" , $entwcgroup{$key} , "\n";
}
if ($pairsout%2 == 1) {
	print "Odd number of pairs, add fake one for seeding!\n";
}
