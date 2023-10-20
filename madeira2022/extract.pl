#
# Check for duplicate of player.
# Scoring program and registration software does not do this
#
sub is_present {
    local ($player) =  @_;

    if ($present{$player}) {
	print stderr "Player $player in movement more than once\n";
    }
    $present{$player} = 1;
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
    # print stderr "line $lineno: $p1, $unused, $strength, $nationality, $p2\n";
    if (!$p1 || !$ p2) {
	print stderr "Line $lineno: $_\n";
    }
    next unless ($p1 && $p2);
    is_present($p1);
    is_present($p2);
    # print "p1=$p1, p2=$p2, nationality=$nationality, strength=$strength\n";

    # Compute group field, 0 except for "wheelchair"
    $grp = 0;
    if ($strength =~ /^WCB/) {
	$grp = 1;
    }
    if ($strength =~ /^WCC/) {
	$grp = 3;
    }

    # Encode strength, weakest if unknown string
    $numstr = 0;
    $numstr = 1 if ($strength =~ /WORLD/);
    $numstr = 2 if ($strength =~ /EXPERT/);
    $numstr = 3 if ($strength =~ /STRONG/);
    $numstr = 4 if ($strength =~ /ADVANCED/);
    $numstr = 5 if ($numstr == 0);
    print "$p1-$p2,$grp,$numstr,$nationality\n";
}
