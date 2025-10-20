#!perl -w

#$pairsout=0;
#
# Groups for wheelchairs
#
$wcgroup{0} = 0;
$wcgroup{"B"} = 1;
# $wcgroup{"C"} = 3;
$wcgroup{"F"} = 9;

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
    # $nationality = $pl_nat{$p1};
    $strength1 = $fields[6];
    $strength2 = $fields[13];
    # $strength = $pl_str{$p1};
    if (!$nationality || !$strength1 || !$strength2) {
	print ERRORS "Line $lineno, player $p1, nationality or strength unknown\n";
	$nerrors++;
	next;
    }
    # $wheelchair1 = $fields[17];
    # $wheelchair2 = $fields[18];
    # print STDERR "p1=$p1, p2=$p2, nationality=$nationality, strength=$strength, wc1=$wheelchair1, wc2=$wheelchair2\n";

    # $grp1 = $wcgroup{$wheelchair1};
    # $grp2 = $wcgroup{$wheelchair2};
    # $grp1 = $grp2 = 0;

    # print STDERR "grp1=$grp1, grp2=$grp2\n";

    # if ($grp1 != 0 && $grp2 != 0 && $grp1 != $grp2) {
	# print STDERR "Line $lineno, wheelchair clash\n";
	# next;
    # }
    # print STDERR "Wheelchair $grp1 and $grp2\n";

    $grp = 0;  $f6 = "";
    # print "$#fields\n";
    if ($#fields > 5) {
	$f6 = $fields[6];
    }
    #
    # 2024 hack with obvious errors
    # clean up!
    #if (defined(f6) && $f6 =~ /Sitting/) {
    #print $grp = 3;
    #}
    #if (defined($fields[6])) {
	#if ($fields[6] =~ m/Sitting/)
	    #$grp = 3;
    #}

    # Remove leading and trailing spaces from nationality field
    $nationality =~ s/^ +//;
    $nationality =~ s/ +$//;

    if (0) {
	# Compute group field, 0 except for "wheelchair"
	$grp = 0;
	if ($strength =~ /^WC([0-9]+)/) {
	    $grp = $1;
	}
	if ($strength =~ /^WC([A-Za-z]+)/) {
	    $grp = $wcgroup{$1};
	    # print STDERR "WC$1 -> $grp\n";
	}
    }

    $numstr = avg_strength($strength1, $strength2);

    $strengthdistr[$numstr]++;
    $natdistr{$nationality}++;
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
if ($pairsout%2 == 1) {
	print "Odd number of pairs, add fake one for seeding!\n";
}
