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
while (<>) {
    chop;
    next if /^BM/;			# Header
    @fields = split /;/;
    # print $#fields, "\n";
    # print $fields[0], $fields[1], "\n";
    $nat = $fields[3];			# Use nationality of first player in pair
    $str = $fields[2];
    $p1 = $fields[0];			# bridgemate number of player 1
    $p2 = $fields[4];			# bridgemate number of player 2
    if (!$p1 || !$ p2) {
	print stderr "Line: $_\n";
    }
    next unless ($p1 && $p2);
    is_present($p1);
    is_present($p2);
    # print "p1=$p1, p2=$p2, nat=$nat, str=$str\n";

    # Compute group field, 0 except for "wheelchair"
    $grp = 0;
    if ($str =~ /^WCB/) {
	$grp = 1;
    }
    if ($str =~ /^WCC/) {
	$grp = 3;
    }

    # Encode strength, weakest if unknown string
    $numstr = 0;
    $numstr = 1 if ($str =~ /WORLD/);
    $numstr = 2 if ($str =~ /EXPERT/);
    $numstr = 3 if ($str =~ /STRONG/);
    $numstr = 4 if ($str =~ /ADVANCED/);
    $numstr = 5 if ($numstr == 0);
    print "$p1-$p2,$grp,$numstr,$nat\n";
}
