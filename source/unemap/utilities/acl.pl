#!/usr/bin/perl
# ******************************************************************************
# FILE : acl.pl
#
# LAST MODIFIED : 16 September 2001
#
# DESCRIPTION :
# Calculates the mean and standard deviation of the activation cycle length
# (acl) for each electrode in a unemap events file.
#
# USAGE :
# acl [options] events-file acl-file
# ==============================================================================

use strict;
use Getopt::Long;

my $acl;
my $calculate;
my $electrode;
my $event_time;
my @event_times;
my $file_name;
# the first electrode in the events file to calculate acls for
my $first_electrode;
my $help;
# the first electrode in the events file to calculate acls for
my $last_electrode;
my $line;
my $number_of_acls;
my $sum_acl;
my $sum_acl_squared;
my $temp;

# get the options
if (Getopt::Long::GetOptions(
	'f|first_electrode=s' => \$first_electrode,
	'h|help' => \$help,
	'l|last_electrode=s' => \$last_electrode))
{
	if (defined $help)
	{
		printf("Usage: acl [options] events-file acl-file\n");
		printf("  -f first_electrode  First electrode to calculate acls for\n");
		printf("  -h  Display this help\n");
		printf("  -f last_electrode  Last electrode to calculate acls for\n");
	}
	else
	{
		# open the events file
		$file_name=shift @ARGV;
		if ((defined $file_name) and (defined open EVENTS,"<$file_name"))
		{
			# skip the event times header
			while (($line=<EVENTS>) and ('Electrode' ne substr($line,0,9)))
			{
			}
			# open the acl file
			$file_name=shift @ARGV;
			if ((defined $file_name) and (defined open ACLS,">$file_name"))
			{
				printf(ACLS "Electrode  mean  standard_deviation\n");
				# read the events file calculating and writing the mean and standard
				#   deviation for the activation cycle as you go
				if (defined $first_electrode)
				{
					$calculate=0;
				}
				else
				{
					$calculate=1;
				}
				while (($line=<EVENTS>) and ('Reference' ne substr($line,0,9)))
				{
					# get rid of end of lines
					chomp $line;
					@event_times=split /,/,$line;
					$electrode=shift @event_times;
					if ((0==$calculate) and (defined $first_electrode) and
						(defined $electrode) and ($electrode eq $first_electrode))
					{
						$calculate=1;
					}
					if (0!=$calculate)
					{
						$event_time=shift @event_times;
						$number_of_acls=0;
						$sum_acl=0;
						$sum_acl_squared=0;
						while ((defined $event_time) and ('none' eq $event_time))
						{
							$event_time=shift @event_times;
						}
						$acl=$event_time;
						while (defined $event_time)
						{
							$event_time=shift @event_times;
							if ((defined $event_time) and ('none' ne $event_time))
							{
								$acl=$event_time-$acl;
								$number_of_acls++;
								$sum_acl += $acl;
								$sum_acl_squared += $acl*$acl;
								$acl=$event_time;
							}
						}
						printf(ACLS $electrode);
						if (0<$number_of_acls)
						{
							printf(ACLS ' %g',$sum_acl/$number_of_acls);
						}
						else
						{
							printf(ACLS ' none');
						}
						if (1<$number_of_acls)
						{
							$temp=$sum_acl_squared-$sum_acl*$sum_acl/$number_of_acls;
							if (0<$temp)
							{
								printf(ACLS ' %g',sqrt($temp)/($number_of_acls-1));
							}
							else
							{
								printf(ACLS ' 0');
							}
						}
						else
						{
							printf(ACLS ' none');
						}
						printf(ACLS "\n");
					}
					if ((0!=$calculate) and (defined $last_electrode) and
						(defined $electrode) and ($electrode eq $last_electrode))
					{
						$calculate=0;
					}
				}
				close ACLS;
			}
			else
			{
				printf("Could not open acl file\n");
			}
			close EVENTS;
		}
		else
		{
			printf("Could not open events file\n");
		}
	}
}
else
{
	printf("Try '$0 -h' for more information\n");
}
