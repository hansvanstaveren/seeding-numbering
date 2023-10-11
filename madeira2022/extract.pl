sub is_present {
    local ($player) =  @_;

    if ($present{$player}) {
	print stderr "Player $player in movement more than once\n";
    }
    $present{$player} = 1;
}

while (<>) {
    chop;
    next if /^BM/;
    @fields = split /;/;
    # print $#fields, "\n";
    # print $fields[0], $fields[1], "\n";
    $nat = $fields[3];
    $str = $fields[2];
    $p1 = $fields[0];
    $p2 = $fields[4];
    if (!$p1 || !$ p2) {
	print stderr "Line: $_\n";
    }
    is_present($p1);
    is_present($p2);
    next unless ($p1 && $p2);
    # print "p1=$p1, p2=$p2, nat=$nat, str=$str\n";
    $grp = 0;
    if ($str =~ /^WCB/) {
	$grp = 1;
    }
    if ($str =~ /^WCC/) {
	$grp = 3;
    }
    $numstr = 0;
    $numstr = 1 if ($str =~ /WORLD/);
    $numstr = 2 if ($str =~ /EXPERT/);
    $numstr = 3 if ($str =~ /STRONG/);
    $numstr = 4 if ($str =~ /ADVANCED/);
    $numstr = 5 if ($numstr == 0);
    print "$p1-$p2,$grp,$numstr,$nat\n";
}
