use List::Util qw(shuffle);
use Data::Dumper;
# @array = shuffle(@array);

$defaultshuf="yes";

$totalpairs = 0;

sub count_pairs {
    my($letter, $file) = @_;

    $npairs{"$letter;NS"} = 0;
    $npairs{"$letter;EW"} = 0;
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

    for my $k (sort keys(%npairs)) {
	my ($sect, $compass) = split /;/, $k;

	my $np = $npairs{$k};
	$totalpairs += $np;
	if ($compass eq "EW") {
	    $sect = lc($sect);
	}
	# print "Section $sect, direction $compass, $np pairs\n";
	# print "$sect";
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
	# print "expand $section$direction$members\n";
	$members = canon_list($members);
	$result = $section . $direction . $members;
	# print "Matched $result\n";
    } else {
	die "entry $entry unparsable";
    }
    return $result;
}

$mapline = 0;
sub maperror {
    my ($err) = @_;

    print "mapping error on line $mapline: $err\n";
}

sub renumber {
    my ($from, $to, $shuf) = @_;
    my ($sect_from, $dir_from, @list_from);
    my ($sect_to, $dir_to, @list_to);

    my $pairsmapped = 0;
    print "Renumber from $from to $to with shuffling ", $shuf ? $shuf : "unspec", "\n";
    if ($from =~ /([A-Z])(NS|EW)(.*)/) {
	$sect_from = $1;
	$dir_from = $2;
	@list_from = split (/,/, $3);
    } else {
	die "renumber from bad format";
    }
    if ($to =~ /([A-Z])(NS|EW)(.*)/) {
	$sect_to = $1;
	$dir_to = $2;
	@list_to = split (/,/, $3);
    } else {
	maperror("renumber to bad format");
    }
    if ($shuf eq "yes") {
	@list_to = shuffle(@list_to);
	# print "shuffled to @list_to\n";
    }
    if ($#list_from != $#list_to) {
	maperror("from and to lists unequal size");
    }
    # print "renumber $sect_from $dir_from to $sect_to $dir_to\n";
    foreach (0..$#list_from) {
	$num_to = $list_to[$_];
	$num_from = $list_from[$_];
	#
	# Check validity of numbers
	#
	# print "$num_to $sect_to $dir_to\n";
	# print "$num_from $sect_from $dir_from\n";
	if ($num_to < 1 || $num_to > $npairs{"$sect_to;$dir_to"}) {
	    my $np = $npairs{"$sect_to;$dir_to"};
	    maperror("$num_to not in range $sect_to $dir_to : $np");
	    next;
	}
	if ($num_from < 1 || $num_from > $npairs{"$sect_from;$dir_from"}) {
	    my $np = $npairs{"$sect_from;$dir_from"};
	    maperror("$num_from not in range $sect_from $dir_from : $np");
	    next;
	}
	#
	my $map_to = "$sect_to;$dir_to;$num_to";
	my $map_from = "$sect_from;$dir_from;$num_from";
	# print "map $map_from to $map_to\n";
	if ($mapsource{$map_from}) {
	    maperror("duplicate mapping from $map_from");
	}
	if ($pairmapping{$map_to}) {
	    maperror("duplicate mapping to $map_to");
	}
	$pairmapping{$map_to} = $map_from;
	$pairsmapped++;
	# $pairmapping{"$sect_to;$dir_to;$list_to[$_]"} = "$sect_from;$dir_from;$list_from[$_]";
    }
    return $pairsmapped;
}

sub read_mapping {
    my ($pairsmapped);

    $pairsmapped = 0;
    open(my $fh, "<", "mapping.txt") || die "No mapping file";
    while (<$fh>) {
	$mapline++;
	next if /^#/;
	if (/shuffle yes/) {
	    $defaultshuf = "yes";
	    next;
	}
	if (/shuffle no/) {
	    $defaultshuf = "no";
	    next;
	}
	chop;
	my ($lhs, $rhs, $shuf) = split;
	my $lhsexp = expand($lhs);
	my $rhsexp = expand($rhs);
	my $pm = renumber($lhsexp, $rhsexp, $shuf ? $shuf : $defaultshuf);
	$pairsmapped += $pm;
    }
    close ($fh);
    print "Totalpairs $totalpairs, mapped $pairsmapped\n";
}

read_previous();
inputstatistics();

read_mapping();

# print Dumper(%pairmapping);
