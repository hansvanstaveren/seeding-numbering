sub combine {
	my ($f1, $f2, $fout) = @_;
	print "$f1, $f2 to $fout\n";

	open ns, $f1 || die;
	open ew, $f2 || die;
	open outp, ">$fout" || die;

	$nspairno=1;
	$ewpairno=21;
	print outp "Pair	Tbl	As	M-ID\n";
	while (<ns>) {
	    $nsline = $_;
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
	}
	close ns;
	close ew;
	close outp;
}

open COMBINFO, "combinfo" || die "combinfo";

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
	# print "@nsf\n";
	@ewf = split /,/, $ew;
	# print "@ewf\n";
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
	combine("sectNS$sect.txt", "sectEW$sect.txt", "sect$sect.txt");
	# system "perl combine.pl sectNS$sect.txt sectEW$sect.txt > sect$sect.txt";
	system "rm sectNS$sect.txt sectEW$sect.txt";
}
close COMBINFO;
