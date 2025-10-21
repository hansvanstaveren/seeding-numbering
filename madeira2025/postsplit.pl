sub report {
	my($tables, $str) = @_;

	printf("%4s: %3d\n", $str, $tables);
}

sub combine {
	my ($f1, $f2, $fout) = @_;
	my ($tables);
	# print "$f1, $f2 to $fout\n";

	open ns, $f1 || die;
	open ew, $f2 || die;
	open outp, ">$fout" || die;

	$tables = 0;
	print outp "Pair	Tbl	As	M-ID\n";
	while (<ns>) {
	    $nsline = $_;
	    # EW one pair short looks like correct, have to check
	    $ewline = <ew>;
	    chop $nsline;
	    chop $ewline;
	    ($nspair, $rest) = split /,/, $nsline;
	    ($ewpair, $rest) = split /,/, $ewline;
	    ($n, $s) = split /-/, $nspair;
	    ($e, $w) = split /-/, $ewpair;
	    print outp "		North	$n\n";
	    print outp "		South	$s\n";
	    print outp "		East	$e\n";
	    print outp "		West	$w\n";
	    $tables++;
	}
	close ns;
	close ew;
	close outp;

	return $tables;
}

open COMBINFO, "combinfo" || die "combinfo";

$total_tables = 0;
while (<COMBINFO>) {
	next if (/^#/);
	next if (/^ *$/);
	unless (/^([A-Z]): *([^ ]*) *([^ ]*)$/) {
		print "cannot parse $_\n";
	}
	$sect = $1;
	$ns = $2;
	$ew = $3;
	#
	# split ns and ew on comma
	#
	@nsf = split /,/, $ns;
	@ewf = split /,/, $ew;

	$nsfiles = "";
	for $num (@nsf) {
		$nsfiles .= sprintf(" seeded%02d.txt", $num);
	}
	system "cat $nsfiles > sectNS$sect.txt";

	$ewfiles = "";
	for $num (@ewf) {
		$ewfiles .= sprintf(" seeded%02d.txt", $num);
	}
	system "cat $ewfiles > sectEW$sect.txt";

	my $t = combine("sectNS$sect.txt", "sectEW$sect.txt", "sect$sect.txt");
	system "rm sectNS$sect.txt sectEW$sect.txt";
	report($t, $sect);
	$total_tables += $t;
}
close COMBINFO;
report($total_tables, "Tot");
