#!/usr/bin/perl -w

use strict;

print <<HEADER;
import sys

sys.setdlopenflags(0x101)
HEADER

while (<>)
  {
	 if (s/if \(\!defined \$path\)//g)
		{
		  <>;
		  <>;
		  <>;
		}
	 s/\$path\///g;
	 s/use/import/g;
	 s/;;/;/;
	 s/::/./g;
	 s/print "\$(\w+)\\n"/print $1/;
	 s/\$//g;
	 s%(if\s*)\(([^\)]+)\)%$1 $2:%g;
	 s/^\s*{\s*$//;
	 s/^\s*}\s*$//;

	 s/import\s+Cmiss.Value.\w+/import Cmiss.Value/g;
	 s/import\s+Cmiss.Variable.\w+/import Cmiss.Variable/g;
	 s/Cmiss.Variable\(/Cmiss.Variable.Variable\(/;
	 s/import Cmiss.cmgui_command_data/import Cmiss.Interpreter;\nimport Cmiss.Cmgui_command_data/;
	 s/Cmiss.cmgui_command_data/Cmiss.Cmgui_command_data/g;
	 s/Cmgui_command_data\(/Cmgui_command_data.new(/;
	 s/Region\(/Region.new\(/;
	 s/get_cmiss_root_region\(cmgui_command_data=>cmgui_command_data\)/command_data_get_root_region(cmgui_command_data)/;

	 s/sub_matrix\(column_low=>(\d+),column_high=>(\d+)\)/sub_matrix(column_low=$1,column_high=$2)/;

	 s/\"Node (\d+(, xi\d)?)=sub_matrix\\n\"/"Node $1=" + str(sub_matrix)/;

	 s/\w+=>//g;
	 s/->/./;
	 s/evaluate\(([\w,]+)\)/evaluate([$1])/;
	 s/Cmiss.Value.Matrix.set_string_convert_max_columns/#Cmiss.Value.Matrix.set_string_convert_max_columns/;
	 s/Cmiss.Value.Matrix.set_string_convert_max_rows/#Cmiss.Value.Matrix.set_string_convert_max_rows/;
	 s/new ([[\w\.]+)\(/$1(/g;
	 s/}\s+else/else:/;
	 print;
  }
