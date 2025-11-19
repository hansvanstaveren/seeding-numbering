sub count_pairs {
    my($letter, $file) = @_;

    $npairs{"${letter};NS"} = 0;
    $npairs{"${letter};EW"} = 0;
    my $l = 0;
    open(my $fh, "<", $file) || die "$file";
    $n = ""; $s = ""; $e = ""; $w = "";
    while (<$fh>) {
	$l++;
	if (/.*North\s+([0-9]+)/) {
	    $n = $1;
	}
	if (/.*South\s+([0-9]+)/) {
	    $s = $1;
	    $ns = "$n-$s";
	    $pn = ++$npairs{"$letter;NS"};
	    $pairs{"$letter;NS;$pn"} = $ns;
	    # print "$letter $pn NS $ns\n";
	}
	if (/.*East\s+([0-9]+)/) {
	    $e = $1;
	}
	if (/.*West\s+([0-9]+)/) {
	    $w = $1;
	    $ew = "$e-$w";
	    $pn = ++$npairs{"$letter;EW"};
	    $pairs{"$letter;EW;$pn"} = $ew;
	    # print "$letter $pn EW $ew\n";
	}
    }
close($fh);
}

sub read_previous {

    for $sect ('A'..'Z') {
	if (-f "sect$sect.txt") {
	    # print "Section $sect exists\n";
	    count_pairs($sect, "sect$sect.txt");
	}
    }
}

sub inputstatistics {
    my ($totalpairs);

    $totalpairs = 0;
    for my $k (sort keys(%npairs)) {
	my ($sect, $compass) = split /;/, $k;

	my $np = $npairs{$k};
	$totalpairs += $np;
	if ($compass eq "EW") {
	    $sect = lc($sect);
	}
	# print "Section $sect, direction $compass, $np pairs\n";
	print "$sect";
    }
    print "\nTotal pairs $totalpairs\n\n";
}

sub canon_list {
    my ($elar) = @_;

    my @comar = ();
    for $el (split /,/, $elar) {
	if ($el =~ /(.*)\-(.*)/) {
	    my $low = $1; 
	    my $high = $2;
	    for my $i ($low..$high) {
		push(@comar, $i);
	    }
	} else {
	    push (@comar, $el);
	}
    }
    return join(',', @comar);
}

sub expand {
    my ($entry, $result) = @_;

    if ($entry =~ /([A-Z])(NS|EW)(.*)/) {
	$section = $1;
	$direction = $2;
	$members = $3;
	if (!$members) {
	    $members = "1-" . $npairs{"$section;$direction"};
	}
	$result = $section . $direction . $members;
	# print "Matched $result\n";
    } else {
	die "entry $entry unparsable";
    }
    return $result;
}

sub read_mapping {
    my ($str);

    $str = canon_list("1-2,5-13");
    print "canonized str $str\n";

    open(my $fh, "<", "mapping.txt") || die "No mapping file";
    while (<$fh>) {
	chop;
	my ($lhs, $rhs, $shuf) = split;
	my $lhsexp = expand($lhs);
	my $rhsexp = expand($rhs);
	print "Renumber $lhsexp to $rhsexp with shuffling ", $shuf ? $shuf : "unspec", "\n";
    }
    close ($fh);
}

read_previous();
inputstatistics();

read_mapping();
