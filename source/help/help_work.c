/*******************************************************************************
FILE : help_work.c

LAST MODIFIED : 24 June 1996

DESCRIPTION :
Code to do the help finding and getting for the CMISS help window.  These
routines all work with fixed line width files, with a line width of LINE_LENGTH.
==============================================================================*/
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>

#include "general/debug.h"
#include "general/mystring.h"
#include "help/help_work.h"
#include "user_interface/message.h"

/**********The Code**********/

char *parse_help_strs(
	char        **help_strs,    /*The help keywords to find.  */
	short        num_strs,      /*The number of keywords.  */
	short        *help_from_file,    /*Boolean value to indicate  */
						/*whether the help came from  */
						/*an examples file.    */
	char        *help_file_name    /*The name of that example file.*/
	)
/*********************************************************************************
DESCRIPTION:
		Parses the help keywords in help_strs, finding the appropriate files and
		the keywords to look for in the file.
		Returns a pointer to the help text, of which the caller must eventually
		dispose.  If no help can be found it returns (char *)-1.
		If calls find help to get the help.  If the help came form an example file,
		then help_from_file is returned as true, and help_file_name contains the
		name of the help file.
================================================================================*/
{
	short        str_len,      /*The length of the string  */
						/*parsed.      */
				next_str;      /*The index of the next string   */
						/*to be parsed.      */
	char        fname[64],      /*The name of the help file.  */
				base_str[64],    /*The "base" part of the search  */
																								/*string.      */
				ok = 0;      /*Whether it's OK to continue  */
						/*searching for help.    */
	FILE        *the_file;      /*Pointer to the help file.  */
	enum Help_type    help_type;      /*The "type" of help we're  */
						/*looking for.      */
	char        *the_text = (char *)-1;  /*Pointer to the help text.  */


	ENTER(parse_help_strs);

	/*Find what kind of help is needed, and get the file name and base string.  */
	if (num_strs)
	{
		*help_from_file = 0;

		str_len = strlen(help_strs[0]);

		if (is_abbrev(help_strs[0],"EXAMPLES",MAX(3,str_len)))
		{
			strcpy(fname,user_settings.help_directory);
			strcat(fname,"examples.doc");

			if (num_strs > 1)
				strcpy(base_str," EXAMPLES ");
			else
				strcpy(base_str," EXAMPLES");

			help_type = EXAMPLES;
			ok = 1;
			next_str = 1;
		}
		else if (is_abbrev(help_strs[0],"SUBROUTINES",MAX(3,str_len)))
		{
			if (num_strs > 1)
			{
				strcpy(fname,user_settings.help_directory);
				strcat(fname,"subroutines.doc");
				strcpy(base_str,"Subroutine    ");
				help_type = SUBROUTINES;
				ok = 1;
				next_str = 1;
			}
		}
		else if (is_abbrev(help_strs[0],"MODULES",MAX(3,str_len)))
		{
			strcpy(fname,user_settings.help_directory);
			strcat(fname,"modules.doc");
			help_type = MODULES;

			if (num_strs > 1)
			{
				strcpy(base_str,"MODULE ");
				ok = 1;
				next_str = 1;
			}
			else
			{
				strcpy(base_str,"MODULES");
				ok = 1;
				next_str = 1;
			}
		}
		else if (is_abbrev(help_strs[0],"VARIABLES",MAX(3,str_len)))
		{
			if (num_strs > 1)
			{
				strcpy(fname,user_settings.help_directory);
				strcat(fname,"variables.doc");
				strcpy(base_str,"");
				help_type = VARIABLES;
				ok = 1;
				next_str = 1;
			}
		}
		else if (is_abbrev(help_strs[0],"C_BLOCKS",MAX(3,str_len)))
		{
			if (num_strs > 1)
			{
				strcpy(fname,user_settings.help_directory);
				strcat(fname,"common_blocks.doc");
				strcpy(base_str,"      COMMON /");
				help_type = COMMON_BLOCKS;
				ok = 1;
				next_str = 1;
			}
		}
		else if (is_abbrev(help_strs[0],"C_VARIABLES",MAX(3,str_len)))
		{
			if (num_strs > 1)
			{
				strcpy(fname,user_settings.help_directory);
				strcat(fname,"common_blocks.doc");
				strcpy(base_str,"");
				help_type = VARIABLES;
				ok = 1;
				next_str = 1;
			}
		}
		else if (is_abbrev(help_strs[0],"INCLUDES",MAX(3,str_len)))
		{
			if (num_strs > 1)
			{
				strcpy(fname,user_settings.help_directory);
				strcat(fname,"common_blocks.doc");
				strcpy(base_str,"Include ");
				help_type = INCLUDES;
				ok = 1;
				next_str = 1;
			}
		}
		else if (is_abbrev(help_strs[0],"HELP",MAX(3,str_len)))
		{
			strcpy(fname,user_settings.help_directory);
			strcat(fname,"help.doc");
			/*base_str and next_str are not used.          */
			help_type = HELP;
			ok = 1;
			next_str = 1;
		}

		/*If it's ok to continue, open the file and call find_help.      */
		if (ok)
		{
			ok = 0;
			if (!(the_file = fopen(fname,"rb")))
			{
				display_message(ERROR_MESSAGE,"Couldn't find a help file");
			}
			else
			{
				the_text = find_help(the_file,help_type,base_str,
														&help_strs[next_str],num_strs - next_str,
														help_from_file,help_file_name);
				fclose(the_file);
			}

		}
	}
	LEAVE;

	return(the_text);
}/*parse_help_strs*/


char *find_help(
	FILE        *the_file,      /*The file to look for help in.  */
	enum Help_type    help_type,      /*The type of help to find.  */
	char        *base_str,      /*The search base string.  */
	char        **search_strs,    /*All the other search strings  */
						/*(These just get bunged back  */
						/*into one string.)    */
	short        num_search_strs,    /*The number of other strings  */
	short        *help_from_file,    /*Whether the help came from a  */
						/*file.        */
	char        *help_file_name    /*the file's name.    */
	)
/*********************************************************************************
DESCRIPTION:
		Finds the help described by all the silly variables.  If the help can't be
		found it returns (char *)-1.
		See parse_help_strings for meaning of help_from_file and help_file_name.
================================================================================*/
{
	char    search_str[64];      /*The actual search string.  */
	char    *the_text = (char *)-1;    /*Pointer to the text.    */
	long          start = 0,      /*The starting and ending lines  */
								end;        /*of help.      */
	char    ok = 0;      /*Whether things are going ok.  */


	ENTER(find_help);

	/*Find out how many lines there are in the file.        */
	fseek(the_file,0,SEEK_END);
	end = ftell(the_file) / LINE_LENGTH;

	/*Assemble the initial search string - the base string with all the other  */
	/*search string stuck on the end.            */
	strcpy(search_str,base_str);
	reassemble_string(search_str,search_strs,num_search_strs);

	/*Look for help.  The method depends on the type of help.      */
	switch (help_type)
	{
		case SUBROUTINES:
		case MODULES:
		case INCLUDES:
		case EXAMPLES:
		{
			/*Looks for text starting with (say) "Subroutine  FISH", and ending with  */
			/*"Subroutine  ".                */
			if (find_area(the_file,search_str,base_str,&start,&end))
			{
				ok = 1;
				end--;
			}
			break;
		}

		case COMMON_BLOCKS:
		case COMMON_VARIABLES:
		{
			/*Looks for the variable or common block name, and then finds the    */
			/*surrouding "Include"s.              */
			if ((start = line_find(the_file,search_str,start,end)) != -1)
			{
				start = line_start_find(the_file,"Include",start,0);
				end = line_start_find(the_file,"Include",start + 1,end);
				ok = 1;
				end--;
			}
			break;
		}

		case VARIABLES:
		{
			/*Looks for the variable name and then finds the surrounding blank lines.  */
			if ((start = line_start_find(the_file,search_str,start,end)) != -1)
			{
				start = line_start_find(the_file,"                ",start,0);
				start++;
				end = line_start_find(the_file,"                ",start,end);
	end--;
				ok = 1;
			}
			break;
		}

		case HELP:
		{
			end--;
			ok = 1;
			break;
		}
	}

	/*If all that worked, try to read the help.          */
	if (ok)
	{
		char    *line_2;


		/*Read the first line of the help.            */
		line_2 = read_text(the_file,start + 1,start + 1);

		/*If it says FILE=, then read the help from that file.      */
		if (is_abbrev(line_2," FILE=",6))
		{
			FILE  *help_file;


			strcpy(help_file_name,user_settings.examples_directory);
			strcat(help_file_name,&(line_2[6]));

			if (help_file = fopen(help_file_name,"rb"))
			{
				start = 0;
				fseek(help_file,0,SEEK_END);
				end = ftell(help_file) / LINE_LENGTH;
				the_text = read_text(help_file,start,end);
				*help_from_file = 1;
				fclose(help_file);
			}
		}
		else
			/*Otherwise read the help from the first file.        */
			the_text = read_text(the_file,start,end);

		DEALLOCATE(line_2);
	}

	LEAVE;

	return(the_text);
}/*find_help*/


char *read_text(
	FILE    *the_file,      /*The file to read from.  */
	long    start_line,      /*The start line.    */
	long    end_line      /*The end line.      */
	)
/*********************************************************************************
DESCRIPTION:
		Reads text from a file, starting at line start_line and ending at line
		end_line.
		It removes all the unnecessary spaces from the end of the lines (since the
		help comes from fixed-width files).
================================================================================*/
{
	char    *text,        /*The text we're reading.  */
		*t_ptr;        /*Pointer to a position in the  */
						/*text.        */
	char    buffer[LINE_LENGTH];    /*Temporary buffer for each line*/


	ENTER(read_text);

	/*Allocate space for the text.  We probably way over allocate it, since we  */
	/*don't know how many useless spaces there are.        */
	/*???DB.  Changed 15 September 1993.  Needs further improvement */
/*  t_ptr = text = mymalloc((end_line - start_line + 1) * LINE_LENGTH + 1);*/
	ALLOCATE(text,char,(end_line-start_line+1)*LINE_LENGTH+1);
	t_ptr=text;

	/*Go to the start of the text.            */
	fseek(the_file,start_line * LINE_LENGTH,SEEK_SET);

	/*Read each line.                */
	for ( ; start_line <= end_line ; start_line++)
	{
		short  count;        /*Count of characters in the   */
						/*line.                         */
		char  *buf_ptr,      /*Pointer to somewhere in the  */
						/*buffer.      */
		*last_letter;      /*Pointer to the space after the*/
																								/*last real letter.    */


		fread(buffer,1,LINE_LENGTH,the_file);

		/*Find the last non-space on the line, copying as we go.      */
		for (count = LINE_LENGTH , buf_ptr = buffer + LINE_LENGTH ; count-- ; )
		{
			if (!isspace(*(--buf_ptr)))
	break;
		}

		last_letter = buf_ptr + 1;

		/*Copy the useful text.              */
		for (buf_ptr = buffer ; buf_ptr < last_letter ; *(t_ptr++) = *(buf_ptr++));

		/*Stick in a return.              */
		*(t_ptr++) = '\n';
	}

	/*Change the last newline into a null (end of string character).    */
	*(t_ptr - 1) = 0x00;

	LEAVE;

	return(text);
}/*read_text*/


short find_area(
	FILE    *the_file,      /*The file to start from.  */
	char    *start_string,      /*The start string.    */
	char    *end_string,      /*The end string.    */
	long    *start,        /*The starting line number.  */
	long    *end        /*The ending line number.  */
	)
/*********************************************************************************
DESCRIPTION:
		Finds an area in the_file starting with a line beginning with start_string,
		and a ending with a line starting with end_string.
		The calling routine can pass in start and end the area of the file to search
		in, and should at least pass the starting line of the file (0) and the ending
		line (the file length divided by LINE_LENGTH);
		Returns 1 if successful, 0 if not.
================================================================================*/
{
	long    my_start,      /*Where the area starts.  */
		my_end;        /*Where it ends.    */
	short    found = 0;                  /*Whether or not we found what  */
						/*we were looking for.    */


	ENTER(find_area);

	my_start = line_start_find(the_file,start_string,*start,*end);
	if (my_start != -1)
	{
		*start = my_start;
		found = 1;

		my_end = line_start_find(the_file,end_string,my_start + 1,*end);
		if (my_end != -1)
			*end = my_end;
	}

	LEAVE;

	return(found);
}/*find_area*/


char **break_string(
	char    *the_string,      /*The string to break.    */
	short         *depth                          /*Returns the number of words  */
						/*(If it was initially set to  */
						/*zero).      */
	)
/*********************************************************************************
DESCRIPTION:
		Breaks a string into individual words, which are defined as being separated
		by spaces.  It returns a list of pointers to the starts of the words in the
		the string, and CHANGES THE STRING, but putting an end of string character
		at the end of each word.
		The routine works recursively so that it can find out how much memory to
		allocate.
		The routine should pass the string in the_string, and a pointer to a short
		that HAS BEEN SET TO ZERO in depth.  The routine returns a pointer to the
		word pointers (which must later be freed) and the number of words in depth.
================================================================================*/
{
	char    *str_ptr,      /*Pointer to somewhere in the  */
						/*string.      */
		*start;        /*Pointer to the start of a word*/
	char    **the_strs = (char **)0;  /*The list of word start  */
																								/*pointers      */
	short    my_depth;      /*The current number of words.  */


	ENTER(break_string);

	my_depth = *depth;

	/*Search for the start of a word, or the end of the string.      */
	for (str_ptr = the_string ; (*str_ptr) && isspace(*str_ptr) ;
			str_ptr++);

	if (!(*str_ptr))
	{
		/*If it's the end of the string, allocate the memory an return.    */
		if (*depth)
/*      the_strs = (char **)mymalloc(sizeof(char *) * (*depth));*/
			ALLOCATE(the_strs,char *,*depth);
	}
	else
	{
		/*Remember where the word started.            */
		start = str_ptr;

		/*Find the end of the word.              */
		for ( ; !isspace(*str_ptr) && (*str_ptr) != 0x00 ; str_ptr++);

		/*Count one more word.              */
		(*depth)++;

		/*Find the next word.              */
		the_strs = break_string(str_ptr,depth);

		/*Mark the end of the word.              */
		*str_ptr = 0x00;

		/*Record the start of the word in the list.          */
		the_strs[my_depth] = start;
	}

	LEAVE;

	return(the_strs);
}/*break_string*/


void reassemble_string(
	char        *the_string,    /*Where to put the string.  */
	char        **the_strings,    /*Pointers to the words.  */
	short        num_strings      /*The number of words.    */
	)
/*********************************************************************************
DESCRIPTION:
		Sticks a whole bunch of words back together.
================================================================================*/
{
	ENTER(reassemble_string);

	for ( ; num_strings-- ; the_strings++)
	{
		strcat(the_string,*the_strings);
		if (num_strings)
			strcat(the_string," ");
	}

	LEAVE;
}/*reassemble_string*/


long line_find(
	FILE        *the_file,      /*The file to search.    */
	char        *the_string,    /*The string to look for.  */
	long        start_line,      /*The starting line.    */
	long        end_line      /*The end line.      */
	)
/*********************************************************************************
DESCRIPTION:
		Finds the_string in the_file between start_line and end_line.  the_string
		can appear anywhere in the line, so theoretically the search should be slow.
		Returns the line to word was found in, or -1 if the word couldn't be found.
		If start_line is greater than end_line the search is done backwards.
================================================================================*/
{
	long        line;      /*The line we're looking at.  */
	char        buffer[LINE_LENGTH];  /*What the line says.    */
	char        found = 0;    /*1 if we've found it.  */


	ENTER(line_find);

	if (start_line <= end_line)
	{
		fseek(the_file,start_line * LINE_LENGTH,SEEK_SET);
		for (line = start_line ; line <= end_line ; line++)
		{
			fread(buffer,1,LINE_LENGTH,the_file);
			if (strstr(buffer,the_string))
			{
				found = 1;
				break;
			}
		}
	}
	else
	{
		for (line = start_line ; line >= end_line ; line--)
		{
			fseek(the_file,line * LINE_LENGTH,SEEK_SET);
			fread(buffer,1,LINE_LENGTH,the_file);
			if (strstr(buffer,the_string))
			{
				found = 1;
				break;
			}
		}
	}

	if (!found)
		line = -1;

	return(line);

	LEAVE;
}/*line_find*/


long line_start_find(
	FILE        *the_file,      /*The file to search.    */
	char        *the_string,    /*The string to look for.  */
	long        start_line,      /*The starting line.    */
	long        end_line      /*The end line.      */
	)
/*********************************************************************************
DESCRIPTION:
		Finds the_string in the_file between start_line and end_line.  the_string
		must be at the start of a line, so theoretically the search should be faster
		then line_find, but it's not!
		Returns the line to word was found in, or -1 if the word couldn't be found.
		If start_line is greater than end_line the search is done backwards.
================================================================================*/
{
	long        line;      /*The line we're looking at.  */
	char        buffer[LINE_LENGTH];  /*What the line says.    */
	char        found = 0;    /*1 if we've found it.  */
	short        str_len;      /*The length of the string we're*/
						/*looking for.      */


	ENTER(line_start_find);

	str_len = strlen(the_string);

	if (start_line <= end_line)
	{
		fseek(the_file,start_line * LINE_LENGTH,SEEK_SET);
		for (line = start_line ; line <= end_line ; line++)
		{
			fread(buffer,1,LINE_LENGTH,the_file);
			if (is_abbrev(buffer,the_string,str_len))
			{
				found = 1;
				break;
			}
		}
	}
	else
	{
		for (line = start_line ; line >= end_line ; line--)
		{
			fseek(the_file,line * LINE_LENGTH,SEEK_SET);
			fread(buffer,1,LINE_LENGTH,the_file);
			if (is_abbrev(buffer,the_string,str_len))
			{
				found = 1;
				break;
			}
		}
	}

	if (!found)
		line = -1;

	LEAVE;

	return(line);
}/*line_start_find*/
