#!perl -w

$wcgroup{"B"} = 1;
$wcgroup{"C"} = 3;

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

#
# Read and decode CSV version of registration spreadsheet
#
$lineno = 0;
while (<>) {
    $lineno++;
    next if /^BM/;			# Header
    chop;
    # @fields = split /;/;
    ($p1, $unused, $strength, $nationality, $p2) = split /;/;
    $unused .= "x";	# get rid of warning
    # print STDERR "line $lineno: $p1, $unused, $strength, $nationality, $p2\n";
    if (!$p1 || !$ p2) {
	print STDERR "Line $lineno: $_\n";
    }
    next unless ($p1 && $p2);
    next if is_present($p1, $lineno);
    next if is_present($p2, $lineno);
    # print "p1=$p1, p2=$p2, nationality=$nationality, strength=$strength\n";

    # Compute group field, 0 except for "wheelchair"
    $grp = 0;
    if ($strength =~ /^WC([0-9]+)/) {
	$grp = $1;
    }
    if ($strength =~ /^WC([A-Za-z]+)/) {
	$grp = $wcgroup{$1};
	# print STDERR "WC$1 -> $grp\n";
    }

    # Encode strength, weakest if unknown string
    $numstr = 5;
    $numstr = 1 if ($strength =~ /WORLD/);
    $numstr = 2 if ($strength =~ /EXPERT/);
    $numstr = 3 if ($strength =~ /STRONG/);
    $numstr = 4 if ($strength =~ /ADVANCED/);
    $strengthdistr[$numstr]++;
    print "$p1-$p2,$grp,$numstr,$nationality\n";
}

for $str (1..5) {
    print STDERR "strength $str: ", $strengthdistr[$str], "\n";
}
