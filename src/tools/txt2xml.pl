#!/usr/bin/perl -w
#
# Converts a text-based data file to XML. Also useful to see how to parse the
# text file in Perl.
#
use strict;

my @discs = ();
my %curdisc;
my @curtracks;

sub xmlEscape {
	my ($string) = @_;

	$string=~ s/\&/\&amp;/g;
	$string=~ s/\</\&gt;/g;
	$string=~ s/\>/\&lt;/g;
	$string=~ s/å/a/g; $string=~ s/ø/o/g;
	return $string;
}


while(<>) {
	next if $_ eq "";

	if (/^\[(.*)\]$/) {
		if (%curdisc) {
			$curdisc{tracks} = [ @curtracks ];
			push (@discs, { %curdisc }) if %curdisc;
		}
		my $discid = $1;
		%curdisc = (
			id => $discid,
			artist => undef,
			title => undef,
			year => undef,
			special => undef
		);
		@curtracks = [ ];
		next;
	}

	if (/^(.*)=(.*)$/) {
		my ($key, $val) = ($1, $2);
		if ($key =~ /^(\d+)$/) {
			$curtracks[$key] = $val;
			next;
		}
		if ($key eq "year") { $curdisc{year} = $val; next; }
		if ($key eq "album") { $curdisc{album} = $val; next; }
		if ($key eq "artist") { $curdisc{artist} = $val; next; }
		if ($key eq "special") { $curdisc{special} = $val; next; }

		die "Undefined key '$key'";
	}
}
if (%curdisc) {
	$curdisc{tracks} = [ @curtracks ];
	push (@discs, { %curdisc }) if %curdisc;
}

print "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
print "<discs>\n";
foreach my $d (@discs) {
	my %disc = %{ $d };
	print " <disc id=\"" . &xmlEscape($disc{id}) . "\" artist=\"" . &xmlEscape($disc{artist}) . "\" album=\"" . &xmlEscape($disc{album}) . "\">\n";
		print "  <year>" . &xmlEscape($disc{year}) . "</year>\n" if defined $disc{year};
		print "  <special>" . &xmlEscape($disc{special}) . "</special>\n" if defined $disc{special};
		my @tracks = @{ $disc{tracks} };
		for (my $n = 1; $n <= $#tracks; $n++) {
			print "  <track nr=\"$n\" title=\"" . &xmlEscape($tracks[$n]) . "\" />\n";
		}
	print " </disc>\n";
}
print "</discs>\n";

# vim:set ts=2 sw=2:
