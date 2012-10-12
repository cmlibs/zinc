#!/usr/bin/perl

$CURRENT_LIST_LEVEL = 0;
$SEPARATOR = '~';
$TOP_LEVEL = 'Commands:';
$previous_level = 0;

$localtime = localtime;
$example_url = "http://cmiss.bioeng.auckland.ac.nz/development/examples/";
$cmgui_url = "http://www.cmiss.org/cmgui";

print <<HEADER;
<!DOCTYPE HTML PUBLIC "-//IETF//DTD HTML 3.0//EN">
<HTML>
<HEAD>
<TITLE>CMGUI Commands</TITLE>
</HEAD>
<BODY>
<p><CENTER>
<H1>CMGUI Commands</H1>
<p>For other <a href="$cmgui_url">help see $cmgui_url</a></p>
<p>For <a href="${example_url}a">examples see ${example_url}a</a></p>
<P>
$localtime
<P></CENTER>
<HR>
<P><B>General notes:</B>
<OL>
<LI>The commands are case insensitive apart from names.
<LI>Command options may be abreviated.
</OL>
HEADER


open INPUT, "$ARGV[0] -command_list|";
while (defined ($_ = <INPUT>)) {
  chomp;
    @Fld = split(' ', $_);

    if ($Fld[0])
		{
		  $level = (index($_, $Fld[0])) / 2;

		  if ($level == $previous_level + 2)
			 {
				#This is the help, indicate it with a "*"
				$command{$level - 1} = "*";				
			 }
		  else
			 {
				#This is a normal command
				$command{$level} = $Fld[0];
			 }
		  for ($i = 1; $i < @Fld; $i++)
		  {
			 $command{$level} = "$command{$level}${SEPARATOR}$Fld[$i]";
		  }
	if ($level > 0)
	  {
	    if ($level == 1)
			{
			  $complete_command = $TOP_LEVEL;
			}
	    else
			{
			  $complete_command = $command{1};
			  for ($i = 2; $i < $level; $i++)
				 {
					$complete_command = "$complete_command${SEPARATOR}$command{$i}";
				 }
			}

	    $nodes{$complete_command}++;
	    # Operating first ensures this is an int
	    s/</&lt;/g;
	    s/>/&gt;/g;
		 s/^\s*//;
	    $string{$complete_command,$nodes{$complete_command}-1} = $_;
		 #print $level;
		 #print "$complete_command $nodes{$complete_command}\n";
		 #print "$string{$complete_command,$nodes{$complete_command}-1}\n";
	    #print "$_\n";
	    #print " ";

	    $previous_level = $level;
	  }
	}
}

$level = 0;
$complete_command = $TOP_LEVEL;
&process_level($complete_command, $level);

sub process_level 
  {
    my($complete_command, $level) = @_;
    for ($ind{$level} = 0; $ind{$level} < $nodes{$complete_command}; $ind{$level}++)
		{
		  $n_parts = (@string_parts = split(' ', $string{$complete_command, $ind{$level}}));
		if ($level == 0) {
		  $temp_command = $string_parts[0];
		} else {
		  $temp_command = $complete_command . $SEPARATOR . $string_parts[0];
		  for ($i = 1; $i < $n_parts; $i++) {
			 $temp_command = $temp_command . $SEPARATOR . $string_parts[$i];
		  }
		}

		  #print $level . " " . $ind{$level};
		  #print "$complete_command $nodes{$complete_command}\n";
		if ($nodes{$complete_command} == 1) {
		  if ($nodes{$temp_command} == 0) {
			 $out_command = $temp_command;
			 $out_command =~ s/$SEPARATOR/ /g;
			 printf "<hr><p><a name=\"%s\"><h2>%s</h2></a>\n", 
				$temp_command, $out_command;
		  }
		} else {
		  if ($ind{$level} == 0) {
			 $out_command = $complete_command;
			 $out_command =~ s/$SEPARATOR/ /g;
			 printf "<hr><p><a name=\"%s\"><h2>%s</h2></a>\n", 
				$complete_command, $out_command;
			 if ($help_nodes = $nodes{"$complete_command${SEPARATOR}*"})
				{
				  printf "<p>\n";
				  for ($i = 0 ; $i < $help_nodes ; $i++)
					 {
						$help_string = $string{"$complete_command${SEPARATOR}*", $i};
						$help_string =~ s/^\*\s*//;
						$help_string =~ s%\b(a\/[/\w]+)\b%<a href="${example_url}$1">$1</a>%g;
						printf "%s\n", $help_string;
					 }
				  printf "</p>\n";
				}
			 printf (("<pre>\n"));
		  }

		  if ($nodes{$temp_command} > 1)
			 {
				printf "   <a href=\"#%s\">%s</a>\n",
				  $temp_command, $string{$complete_command, $ind{$level}};
			 }
		  elsif ($nodes{$temp_command} > 0)
			 {
				$sub_temp_command = $temp_command;
				while ($nodes{$sub_temp_command} == 1) {
				  $n_parts = (@string_parts = split(' ', $string{$sub_temp_command, 0}));

				  $sub_temp_command = "$sub_temp_command${SEPARATOR}$string_parts[0]";
				  for ($i = 1; $i < $n_parts; $i++) {
					 $sub_temp_command = "$sub_temp_command${SEPARATOR}$string_parts[$i]";
				  }
				}
				printf "   <a href=\"#%s\">%s</a>\n", 
				  $sub_temp_command, $string{$complete_command, $ind{$level}};
			 }
		  else
			 {
				printf "   %s\n", $string{$complete_command , $ind{$level}};
			 }

		  if ($ind{$level} == ($nodes{$complete_command} - 1)) {
			 printf (("</pre>\n"));
		  }

		  #	      print complete_command string[complete_command,ind[level]];	      
		}
    }

    for ($ind{$level} = 0; $ind{$level} < $nodes{$complete_command}; $ind{$level}++)
		{
		  $n_parts = (@string_parts = split(' ', $string{$complete_command, $ind{$level}}));
		  if ($level == 0) {
			 $temp_command = $string_parts[0];
		  } else {
			 $temp_command = $complete_command . $SEPARATOR . $string_parts[0];
			 for ($i = 1; $i < $n_parts; $i++) {
				$temp_command = $temp_command . $SEPARATOR . $string_parts[$i];
		  }
		}

		#print ">>$temp_command $nodes{$temp_command}\n";
		if ($nodes{$temp_command} > 0) {
		  &process_level($temp_command, $level + 1);
		}
    }
  }

print <<FOOTER;
<HR>
<ADDRESS>
<A HREF="$cmgui_url">$cmgui_url</A></ADDRESS>
</BODY>
</HTML>
FOOTER
