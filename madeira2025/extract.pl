#!perl -w

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

open (DEBUG, ">errors");
open (STATS, ">statistics");
open (SEEDIN, ">seedin");

#
# Read and decode CSV version of registration spreadsheet
#
# BM#1;Country1;Standard1;BM#2;Country2;Standard2;Notes
open(PAIRS, "pairs.csv") || die;
$lineno = 0;
while (<PAIRS>) {
    $lineno++;
    # print "$lineno $_\n";
    next if /^BM/;			# Header
    s/\r$//;
    chomp;
    @fields = split /;/;
    # print @fields, "$#fields columns\n";
    # $id1 = $fields[0];
    $p1 = $fields[0];
    # $id2 = $fields[4];
    $p2 = $fields[3];
    # print "p1-p2 $p1-$p2\n";
    # print STDERR "line $lineno: $p1, $unused, $strength, $nationality, $p2\n";
    if ($p1 !~ /^[0-9]+$/ || $p2 !~ /^[0-9]+$/ || $p1 <= 0 || $p1 >=100000 || $p2 <= 0 || $p2 >= 100000) {
	print DEBUG "Line $lineno: $_\n";
	next;
    }
    next if is_present($p1, $lineno);
    next if is_present($p2, $lineno);
    $nationality = $fields[1];
    # $nationality = $pl_nat{$p1};
    $strength = $fields[2];
    # $strength = $pl_str{$p1};
    if (!$nationality || !$strength) {
	print STDERR "Line $lineno, player $p1, nationality or strength unknown\n";
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
    if (defined(f6) && $f6 =~ /Sitting/) {
    	print $grp = 3;
    }
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

    # $numstr = $fields[19];
    # Encode strength, weakest if unknown string
    $numstr = 5;
    $numstr = 1 if ($strength =~ /WORLD/);
    $numstr = 2 if ($strength =~ /EXPERT/);
    $numstr = 3 if ($strength =~ /STRONG/);
    $numstr = 4 if ($strength =~ /ADVANCED/);
    $strengthdistr[$numstr]++;
    $natdistr{$nationality}++;
    print SEEDIN  "$p1-$p2,$grp,$numstr,$nationality\n";
}
close(PAIRS);

for $str (1..5) {
    print STATS "strength $str: ", $strengthdistr[$str], "\n";
}
foreach my $key (sort keys %natdistr) {
    print STATS "$key:" , $natdistr{$key} , "\n";
}
