package Cmiss::Perl_cmiss;

use strict;
use Carp;
use Text::Balanced;
use vars qw($VERSION @ISA @EXPORT @EXPORT_OK $AUTOLOAD);

require Exporter;
require AutoLoader;

@ISA = qw(Exporter);
# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.
@EXPORT = qw(
	
);
$VERSION = '0.01';

sub AUTOLOAD {
    # This AUTOLOAD is used to 'autoload' constants from the constant()
    # XS function.  If a constant is not found then control is passed
    # to the AUTOLOAD in AutoLoader.

    my $constname;
    ($constname = $AUTOLOAD) =~ s/.*:://;
    croak "& not defined" if $constname eq 'constant';
    my $val = constant($constname, @_ ? $_[0] : 0);
    if ($! != 0) {
	if ($! =~ /Invalid/) {
	    $AutoLoader::AUTOLOAD = $AUTOLOAD;
	    goto &AutoLoader::AUTOLOAD;
	}
	else {
		croak "Your vendor has not defined Perl_cmiss macro $constname";
	}
    }
    no strict 'refs';
    *$AUTOLOAD = sub () { $val };
    goto &$AUTOLOAD;
}

#Not using XSLoader as we want to ensure the symbols are global
sub locate_this_module
{
  my $modlibname = (caller())[1];
  $modlibname =~ s,/Cmiss/Perl_cmiss.pm$,,;
  $modlibname =~ s,blib/lib,blib/arch,;
  return $modlibname;
}
my $modlibname = locate_this_module();
use Cmiss;
Cmiss::require_library("$modlibname/auto/Cmiss/Perl_cmiss/Perl_cmiss.so");

package Perl_cmiss;

# Preloaded methods go here.
#Using a hash so that the strategy for action could be placed with
#the word.  For now only one action.
my %keywords = ( 'command_window' => 1,
					  'create' => 1,
					  'fem' => 1,
					  'gfx' => 1,
					  'itp' => 1,
					  'open' => 1,
					  'quit' => 1,
					  'list_memory' => 1,
					  'set' => 1,
					  'unemap' => 1 );

my @command_list = ();
my $block_count = 0;
my $block_required = 0;
my $echo_commands = 0;
my $echo_prompt = "";
my $cmiss_debug = 0;

sub call_command
{
  no strict qw(vars);

  local $command = shift;
  {
	 package cmiss;
	 # Catch all warnings as errors */
	 local $SIG{__WARN__} = sub { die $_[0] };
	 eval ($Perl_cmiss::command);
  }
}

sub cmiss_array
  {
	 my $command = "";
	 my $token2;
	 my $ref_type;
	 my $token;
	 my $subtoken;
	 my $first;
	 my $return_code;

	 for $token (@_)
		{
		  if (! defined $token)
		  {
			 print ("Undefined variable referenced in command\n");
			 return (0);
		  }
		  $ref_type = ref $token;
		  if ($token =~ /^[\s;]+$/)
			 {
				#This is just a delimiter
				$command = $command . $token;
			 }
		  elsif ("ARRAY" eq $ref_type)
			 {
				$first = 1;
				for $subtoken (@{$token})
				  {
					if (! defined $subtoken)
					  {
						print ("Undefined variable referenced in command\n");
						return(0);
					  }
					 if ($first)
						{
						  $first = 0;
						}
					 else
						{
						  $command = $command . ",";
						}
					 if ($subtoken =~ /[\s;]+/)
						{
						  $token2 = $subtoken;
						  #These delimiters need to be quoted and therefore the quotes and 
						  #escape characters contained within must be escaped.
						  $token2 =~ s/\\/\\\\/g;
						  $token2 =~ s/\"/\\\"/g;
						  $command = $command . "\"$token2\"";
						}
					 else
						{
						  $command = $command . $subtoken;
						}
				  }
			 }
		  elsif ($token =~ /[\s;]+/)
			 {
				$token2 = $token;
				#These delimiters need to be quoted and therefore the quotes and 
				#escape characters contained within must be escaped.
				$token2 =~ s/\\/\\\\/g;
				$token2 =~ s/\"/\\\"/g;
				$command = $command . "\"$token2\"";
			 }
		  else
			 {
				#This is just a plain word
				$command = $command . $token;
			 }
		}

	 if ($cmiss_debug)
		{
		  print "Perl_cmiss::cmiss_array final: $command\n";
		}
	 {
		package cmiss;
		$return_code = Cmiss::cmgui_command_data::cmiss($command);
	 }
	 if ($cmiss_debug)
		{
		  print "Perl_cmiss::cmiss_array cmiss return_code $return_code\n";
		}
	 return ($return_code);
  }

sub execute_command
  {
	 my $command = shift;
	 my $command2 = $command;
	 $command2 =~ s%'%\\'%g;
	 $command2 = "print '$echo_prompt$command2' . \"\\n\";";
	 my $token = "";
	 my $part_token;
	 my $token2;
	 my $lc_token;
	 my $match_string = join ("|", keys %keywords);
#	 my @tokens = &parse_line('\\s*[\\{\\}\\(\\)]\\s*', \"delimiters\", $command);
#	 my @tokens; push (@tokens, $command);
	 my @tokens = ();
	 my $extracted;
	 my $lc_command;
	 my $continue;
	 my $reduced_command;
	 my $print_command_after = 0;
	 my $is_perl_token;
	 my $simple_perl;

	 $simple_perl = 0;
	 while ($command ne "")
		{
		  $lc_command = lc ($command);
		  if ($cmiss_debug)
			 {
				print "$command   ";
			 }
		  if ($command =~ s%^(\s+)%%)
			 {
				if ($cmiss_debug)
				  {
					 print "space: $1\n";
				  }
				$token = $token . $1;
			 }
		  elsif ($command =~ s%^(#.*)%%)
			 {
			  if ($cmiss_debug)
			    {
					print "comment: $1\n";
				 }
			  if ($simple_perl && (!$block_required) && (! ($token =~ m/;\s*$/)))
			  {
				 $token = $token . ";";
			  }
			  if ($token ne "")
			  {
				 push(@tokens, $token);
			  }
			  $token = "";
			 }
		  else
			 {
				$simple_perl = 0;
				if ($command =~ s%^({)%%)
				  {
					 if ($cmiss_debug)
						{
						  print "open bracket: $1\n";
						}
					 if ($token ne "")
						{
						  push(@tokens, $token);
						}
					 $block_required = 0;
					 $block_count++;
					 $print_command_after = 1;
					 $token = "";
					 push(@tokens, $1);
				  }
				elsif ($command =~ s%^(})%%)
				  {
					 if ($cmiss_debug)
						{
						  print "close bracket: $1\n";
						}
					 if ($token ne "")
						{
						  push(@tokens, $token);
						}
					 if ($block_count > 0)
						{
						  $block_count--;
						}
					 $print_command_after = 0;
					 $token = "";
					 push(@tokens, $1);
				  }
				elsif (($token =~ m/(^|\W)$/) && 
				  ($command =~ s%^(if|while|unless|until|for|foreach|elsif|else|continue|sub)\b%%))
				  {
					 if ($cmiss_debug)
						{
						  print "control keyword: $1\n";
						}
					 $token = $token . $1;
					 $block_required = 1;
				  }
				elsif( $token =~ m/^\s*$/ &&
					   $lc_command =~ m/^(itp(\s+(ass\w*)(?:\s+blo\w*(?:\s+clo\w*)?)?|\s+(set)(\s+(ech\w*)|\s+(deb\w*))?)?\s+)?\?+/ )
			    {
				    my $itp = defined $1;
					my $second_word = defined $2;
					my $assert = defined $3;
					my $set = defined $4;
					my $third_word = defined $5;
					my $echo = defined $6;
					my $debug = defined $7;

					print "itp\n";
					if ( ! $second_word || $assert ) {
						print "  assert blocks closed\n";
					}
					if ( ! $second_word || $set ) {
						print "  set\n";
						if ( ! $third_word || $echo ) {
							print "    echo\n";
							print "      <on>\n";
							print "      <off>\n";
							print "      <prompt PROMPT_STRING>\n";
						}
						if ( ! $third_word || $debug ) {
							print "    debug\n";
							print "      <on>\n";
							print "      <off>\n";
						}
					}
					if ( ! $itp ) {
						#Call Cmiss with the help command
						$token .= "Perl_cmiss::cmiss_array(\"$lc_command\")";
						push(@tokens, $token);
						$token = "";
					}
					$command =~ s/^([^}#]*)//;
					if ( $cmiss_debug ) {
						print "itp?: $1\n";
					}
				}
				elsif ($lc_command =~ m/^itp/)
				  {
					 if ($lc_command =~ m/^itp\s+ass\w*\s+blo\w*\s+clo\w*/)
						{
						  if ($block_required || $block_count)
							 {
								$block_required = 0;
								$block_count = 0;
								@command_list = ();
								die ("itp assert blocks closed failed\n");
							 }
						}
					 elsif ($lc_command =~ m/^itp\s+set\s+echo\s*(\w*)\s*(?:[\"\']([^\"\']*)[\"\']|([^\"\']+\S*)|)/)
						{
						  my $first_word = $1;
						  my $second_word = $2 ? $2 : $3;
						  if ($first_word =~ m/on/)
							 {
								$echo_commands = 1;
							 }
						  elsif ($first_word =~ m/off/)
							 {
								$echo_commands = 0;
							 }
						  elsif ($first_word =~ m/pro/)
							 {
								$echo_prompt = $second_word;
							 }
						  else
							 {
								$echo_commands = ! $echo_commands;
							 }
						}
					 elsif ($lc_command =~ m/^itp\s+set\s+debug\s*(\w*)/)
						{
						  if ($1 =~ m/on/)
							 {
								$cmiss_debug = 1;
							 }
						  elsif ($1 =~ m/off/)
							 {
								$cmiss_debug = 0;
							 }
						  else
							 {
								$cmiss_debug = ! $cmiss_debug;
							 }
						}
					 else
						{
						  die ("Unknown itp environment command\n");
						}
					 $command =~ s/^([^}#]*)//;
					 if ($cmiss_debug)
						{
						  print "itp: $1\n";
						}
				  }
				else
				  {
					 $continue = 1;
					 if ($token =~ m/^\s*$/)
						{
						  if (($lc_command =~ m/^(?:$match_string)\b/)
								|| ($lc_command =~ m/^q$/))
							 {
								$token = $token . "(Perl_cmiss::cmiss_array(";
								$part_token = "";
								$token2 = "";
								$is_perl_token = 1;
								my $is_simple_token = 1;
								while (($command ne "") && !($command =~ m/(^[}	#])/))
								  {
									 if ($cmiss_debug)
										{
										  print "cmiss $command   ";
										}
									 if ($command =~ s%^([\s;]+)%%)
										{
										  if ($cmiss_debug)
											 {
												print "cmiss space: $1\n";
											 }
										  if (!$is_simple_token && $is_perl_token)
											 {
												# Let Perl parse this into a string array
												$token = $token . "[$part_token],\"$1\",";
											 }
										  else
											 {
												# Add it as a string 
												# Escape \\ and " characters
												$part_token =~ s/\\/\\\\/g;
												$part_token =~ s/\"/\\\"/g;
												$token = $token . "\"$part_token\",\"$1\",";
											 }
										  $token2 = $token2 . $part_token . $1;
										  $is_perl_token = 1;
										  $is_simple_token = 1;
										  $part_token = "";
										}
									 elsif (($part_token eq "") && ($command =~ s%^(\?+|[\-]?[.,0-9:]+)%%))
										{
										  if ($cmiss_debug)
											 {
												print "cmiss number/operator: $1\n";
											 }
										  $part_token = $part_token . $1;
										}
									 elsif ($command =~ s%^([.,0-9:]+)%%)
										{
										  if ($cmiss_debug)
											 {
												print "cmiss number/operator: $1\n";
											 }
										  $part_token = $part_token . $1;
										}
									 elsif ($command =~ s%^([+\-*=/\\<>!()?])%%)
										{
										  if ($cmiss_debug)
											 {
												print "cmiss perl number/operator: $1\n";
											 }
										  $part_token = $part_token . $1;
										  $is_simple_token = 0;
										}
									 elsif ($command =~ s%^(\w+\()%%)
										{
										  if ($cmiss_debug)
											 {
												print "cmiss function: $1\n";
											 }
										  $part_token = $part_token . $1;
										  $is_simple_token = 0;
										}
									 else
										{
										  $is_simple_token = 0;
										  ($extracted, $reduced_command) = 
											 Text::Balanced::extract_variable($command);
										  if ($extracted)
											 {
												$command = $reduced_command;
												$part_token = $part_token . $extracted;
												if ($cmiss_debug)
												  {
													 print "cmiss variable: $extracted\n";
												  }
											 }
										  else
											 {
												($extracted, $reduced_command) =
												  Text::Balanced::extract_delimited($command, '\'"`');
												if ($extracted)
												  {
													 $command = $reduced_command;
													 #Escape " and \ characters except for the start and end ones
													 #$extracted =~ s/(?<=.)\\(?=.)/\\\\/g;
													 #$extracted =~ s/(?<=.)\"(?=.)/\\\"/g;
													 $part_token = $part_token . $extracted;
													 if ($cmiss_debug)
														{
														  print "cmiss delimited: $extracted\n";
														}
												  }
												else
												  {
													 if ($cmiss_debug)
														{
														  print "cmiss character: ".substr($command, 0, 1)."\n";
														}
													 $part_token = $part_token . substr($command, 0, 1);
													 $command = substr($command, 1);
													 $is_perl_token = 0;
												  }
											 }
										}
								  }
							  $token2 = $token2 . $part_token;
							  $token2 =~ s/\\/\\\\/g;
							  $token2 =~ s/\"/\\\"/g;
							  $token2 =~ s/\$/\\\$/g;
							  $token2 =~ s/\@/\\\@/g;
#							  if ($cmiss_debug)
#							    {
#									print "token2 $token2\n";
#								 }
							  if (!$is_simple_token && $is_perl_token)
								 {
									# Let Perl parse this into a string array
									$token = $token . "[$part_token])) || die(\"Error in cmiss command \\\"$token2\\\".\\n\");";
								 }
							  else
								 {
									# Add it as a string 
									# Escape \\ and " characters
									$part_token =~ s/\\/\\\\/g;
									$part_token =~ s/\"/\\\"/g;
									$token = $token . "\"$part_token\")) || die(\"Error in cmiss command \\\"$token2\\\".\\n\");";
								 }
							  if ($cmiss_debug)
								 {
									print "cmiss: $token\n";
								 }
							  push(@tokens, $token);
							  $token = "";
						     $continue = 0;
                    }
				    }
				  if ($continue)
					 {
						$simple_perl = 1;
						if ($command =~ s/^(\d+)\s*\.\.\s*(\d+)\s*:\s*(\d+)//)
						  {
							 my $remainder_start = $1 % $3;
							 my $remainder_finish = ($2 - $remainder_start) % $3;
							 my $list_start = ($1 - $remainder_start) / $3;
							 my $list_finish = ($2 - $remainder_start - $remainder_finish)/ $3;
							 my $new_list_operator = "(map {\$_ * $3 + $remainder_start} $list_start..$list_finish)";
							 $token = $token . $new_list_operator;
							 if ($cmiss_debug)
								{
								  print "step sequence: $new_list_operator\n";
								}
						  }
						else
						  {
							 ($extracted, $reduced_command) =
								Text::Balanced::extract_variable($command);
							 if ($extracted)
								{
								  $command = $reduced_command;
								  if ($cmiss_debug)
									 {
										print "variable: $extracted\n";
									 }
								  $token = $token . $extracted;
								}
							 else
								{
								  ($extracted, $reduced_command) =
									 Text::Balanced::extract_quotelike($command);
								  if ($extracted)
									 {
										$command = $reduced_command;
										if ($cmiss_debug)
										  {
											 print "quotelike: $extracted\n";
										  }
										$token = $token . $extracted;
									 }
								  else
									 {
										if ($cmiss_debug)
										  {
											 print "character: " . substr($command, 0, 1) . "\n";
										  }
										$token = $token . substr($command, 0, 1);
										$command = substr($command, 1);
									 }
								}
						  }
					 }
				}
			}
		}
	 if ($token ne "")
		{
		  #Add a semicolon if not already there.
		  if ($simple_perl && (!$block_required) && (! ($token =~ m/;\s*$/)))
			 {
				$token = $token . ";";
			 }
		  push(@tokens, $token);
		}
						  
	$command = join ("", @tokens);
	if ($cmiss_debug)
	  {
		 print "Perl_cmiss::execute_command parsed $command\n";
	  }
	if ($echo_commands && (! $print_command_after))
     {
		 push (@command_list, $command2);
	  }
   push (@command_list, $command);
   if ($echo_commands && $print_command_after)
     {
		 push (@command_list, $command2);
	  }

#	 print \"$block_count $block_required\\n\";

	 if ((!($block_count))&&(!($block_required)))
		{
		  $command = join ("\n", @command_list);
		  #Must reset this before the eval as it may call this function
		  #recursively before returning from this function
		  @command_list = ();
		  call_command($command);
		  if ($@)
			 {
				#Trim the useless line number info if it has been added.
				$@ =~ s/ at \(eval \d+\) line \d+//;
				die("$@\n");
			 }
		  print "";
		}
  }

# Autoload methods go after =cut, and are processed by the autosplit program.

1;

__END__
# Below is the stub of documentation for your module. You better edit it!

=head1 NAME

Perl_cmiss - Perl extension for interfacing to Cmiss

=head1 SYNOPSIS

  use Cmiss::Perl_cmiss;

=head1 ABSTRACT

  This should be the abstract for Cmiss::Perl_cmiss.

=head1 DESCRIPTION

=head2 EXPORT

=head1 SEE ALSO

=head1 AUTHOR

Shane Blackett, <s.blackett@auckland.ac.nz>

=head1 COPYRIGHT AND LICENSE

Copyright 2003 by Auckland UniServices

This library is free software; you can redistribute it and/or modify
it under the same terms as Perl itself. 

=cut
