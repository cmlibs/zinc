# Convert a binary file into a quoted string containing the hex values for the binary file

use strict;

my $input_filename = $ARGV[0];
my $output_filename = $ARGV[1];

open(IN_FILE, "$input_filename") or die "Unable to open file $input_filename";

open(OUT_FILE, ">$output_filename") or die "Unable to open file $output_filename";

my $char;
my $int;

print OUT_FILE "\"";

while (read IN_FILE, $char, 1)
  {
	 $int = unpack 'C', $char;
	 printf OUT_FILE "\\x%02x", $int;
  }

print OUT_FILE "\"\n";

close IN_FILE;
close OUT_FILE;
