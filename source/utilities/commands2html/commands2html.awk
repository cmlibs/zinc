BEGIN {
  getline <TEMPLATE ;
  while(!match($0,"Commands go here"))
    {
      if ($1 == "DATE")
	{
	  echo "date" | getline ;
	}
      print;
      getline <TEMPLATE ;
    }
  CURRENT_LIST_LEVEL = 0;
  SEPARATOR = "~";
  TOP_LEVEL = "Commands:";
  previous_level = 0;
}

{
  if ($1)
    {
      level = (index($0, $1) - 1) / 2;

      command[level] = $1;
      for (i = 1 ; i < NF ; i++)
	{
	  command[level] = command[level] SEPARATOR $(i+1);
#	  print command[level];
	}
      if (level > 0)
	{
	  if (level == 1)
	    {
	      complete_command = TOP_LEVEL;
	    }
	  else
	    {
	      complete_command = command[1];
	      for (i = 2 ; i < level ; i++)
		{
		  complete_command = complete_command SEPARATOR command[i];
		}
	    }
	  
	  nodes[complete_command]++; # Operating first ensures this is an int
	  gsub("<","\\&lt;");
	  gsub(">","\\&gt;");
	  string[complete_command,nodes[complete_command]-1] = substr($0, index($0, $1));
#      print level;
#      print complete_command " " nodes[complete_command];
#      print;
#      print " ";

	  previous_level = level;
	}
    }
}

function process_level(complete_command,level)
{
  for (ind[level] = 0 ; ind[level] < nodes[complete_command] ; ind[level]++ )
    {
      n_parts = split(string[complete_command,ind[level]], string_parts);
      if (level == 0)
	{
	  temp_command = string_parts[1];
	}
      else
	{
	  temp_command = complete_command SEPARATOR string_parts[1];
	  for (i = 1 ; i < n_parts ; i++)
	    {
	      temp_command = temp_command SEPARATOR string_parts[i + 1];
	    }
	}

#	  print level " " ind[level];
#	  print complete_command " " nodes[complete_command];
      if (nodes[complete_command] == 1)
	{
	  if (nodes[temp_command] == 0)
	    {
	      out_command = temp_command;
	      gsub(SEPARATOR," ",out_command);
	      printf("<hr><p><a name=\"%s\"><h2>%s</h2></a>\n",
		     temp_command, out_command);
	    }
	}
      else
	{
	  if (ind[level] == 0)
	    {
	      out_command = complete_command;
	      gsub(SEPARATOR," ",out_command);
	      printf("<hr><p><a name=\"%s\"><h2>%s</h2></a>\n",
		     complete_command, out_command);
	      printf("<pre>\n");
	    }
	      
	  if (nodes[temp_command] > 1)
	    {
	      printf("   <a href=\"#%s\">%s</a>\n", 
		     temp_command,
		     string[complete_command,ind[level]]);
	    }
	  else if (nodes[temp_command] > 0)
	    {
	      sub_temp_command = temp_command;
	      while (nodes[sub_temp_command] == 1)
		{
		  n_parts = split(string[sub_temp_command,0], string_parts);
		  sub_temp_command = sub_temp_command SEPARATOR string_parts[1];
		  for (i = 1 ; i < n_parts ; i++)
		    {
		      sub_temp_command = sub_temp_command SEPARATOR string_parts[i + 1];
		    }
		}
	      printf("   <a href=\"#%s\">%s</a>\n", 
		     sub_temp_command,
		     string[complete_command,ind[level]]);
	    }
	  else
	    {
	      printf("   %s\n",string[complete_command,ind[level]]);
	    }

	  if (ind[level] == (nodes[complete_command] - 1))
	    {
	      printf("</pre>\n");
	    }

#	      print complete_command string[complete_command,ind[level]];	      
	}
    }

  for (ind[level] = 0 ; ind[level] < nodes[complete_command] ; ind[level]++ )
    {
      n_parts = split(string[complete_command,ind[level]], string_parts);
      if (level == 0)
	{
	  temp_command = string_parts[1];
	}
      else
	{
	  temp_command = complete_command SEPARATOR string_parts[1];
	  for (i = 1 ; i < n_parts ; i++)
	    {
	      temp_command = temp_command SEPARATOR string_parts[i + 1];
	    }
	}

#      print ">>" temp_command " " nodes[temp_command];
      if (nodes[temp_command] > 0)
	{
	  process_level(temp_command,level + 1);
	}
    }
}

END {

  level = 0;
  complete_command = TOP_LEVEL;
  process_level(complete_command,level);

  while(getline <TEMPLATE)
    {
      print;
    }
}
