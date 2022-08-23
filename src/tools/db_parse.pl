#!/usr/bin/perl -w
#
# sample program which uses XML::Parser to parse the XML database and
# dumps the structures. If you intend to do something else with the
# data (or just wonder how XML::Parser can be used)
#
use strict;
use XML::Parser;

my @discs = ();
my %curdisc;
my @curtracks;
my $cdata = undef;

sub xml_start {
	my ($xml, $el, %attrs) = @_;

	if ($el eq "disc") {
		%curdisc = (
			id => $attrs{id},
			artist => $attrs{artist},
			album => $attrs{album},
			year => undef,
			special => undef
		);
		@curtracks = [ ];
		return;
	}

	if ($el eq "track") {
		@curtracks[$attrs{nr}] = $attrs{title};
		return;
	}
}

sub xml_end {
	my ($xml, $el) = @_;

	if ($el eq "disc") {
		$curdisc{tracks} = [ @curtracks ];
		push (@discs, { %curdisc }) if %curdisc;
		return;
	}

	if ($el eq "year") {
		$curdisc{year} = $cdata;
		return;
	}

	if ($el eq "special") {
		$curdisc{special} = $cdata;
		return;
	}
}

sub xml_default {
	my ($xml, $str) = @_;
	# crude
	$cdata = $str;
}

my $xml = new XML::Parser(
	Handlers => {
		Start   => \&xml_start,
		End     => \&xml_end,
		Default => \&xml_default
	}
);
$xml->parsefile("db.xml");

foreach my $d (@discs) {
	my %disc = %{ $d };
	print "Disc " . $disc{id} . ": " . $disc{artist} . " / " . $disc{album} . "\n";
	print "  Year: " . $disc{year} . "\n" if defined $disc{year};
	print "  Special: " . $disc{special} . "\n" if defined $disc{special};

	my @tracks = @{ $disc{tracks} };
	for (my $n = 1; $n <= $#tracks; $n++) {
		print "    Track $n: " . $tracks[$n] . "\n";
	}
}

# vim:set ts=2 sw=2:
