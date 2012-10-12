my $test = "28";
sub locate
{
	my @caller = caller;
	if (!($caller[1] =~ s%/[^/]*$%%))
	{
	  $caller[1] = ".";
	}
	return ($caller[1]);
}
my $path = locate;
use Test::More tests => 1;
use File::Compare;
open (OUTPUT, ">$path/$test.out");
select OUTPUT;

eval `cat $path/$test.com`;
if ($@) { die };

close OUTPUT;
select STDOUT;

$diff = compare("$path/$test.answer", "$path/$test.out");
ok($diff == 0, "diff $path/$test.answer $path/$test.out");
