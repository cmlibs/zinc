/*******************************************************************************
FILE : help_work.h

LAST MODIFIED : 4 December 1994

DESCRIPTION :
interface for code to do the help finding and getting for the CMISS help window
==============================================================================*/
#ifndef _H_help_work
#define _H_help_work  1
#define LINE_LENGTH 132

/**********Structures**********/

enum Help_type
{
	EXAMPLES,
	SUBROUTINES,
	MODULES,
	VARIABLES,
	COMMON_BLOCKS,
	COMMON_VARIABLES,
	INCLUDES,
	HELP
};




/**********Function Prototypes**********/

char *parse_help_strs(
	char        **help_strs,    /*The help keywords to find.  */
	short        num_strs,      /*The number of keywords.  */
	short        *help_from_file,    /*Boolean value to indicate  */
						/*whether the help came from  */
						/*an examples file.    */
	char        *help_file_name    /*The name of that example file.*/
	);

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
	);

char *read_text(
	FILE    *the_file,      /*The file to read from.  */
	long    start_line,      /*The start line.    */
	long    end_line      /*The end line.      */
	);

short find_area(
	FILE    *the_file,      /*The file to start from.  */
	char    *start_string,      /*The start string.    */
	char    *end_string,      /*The end string.    */
	long    *start,        /*The starting line number.  */
	long    *end        /*The ending line number.  */
	);

char **break_string(
	char    *the_string,      /*The string to break.    */
	short         *depth                          /*Returns the number of words  */
						/*(If it was initially set to  */
						/*zero).      */
	);

void reassemble_string(
	char        *the_string,    /*Where to put the string.  */
	char        **the_strings,    /*Pointers to the words.  */
	short        num_strings      /*The number of words.    */
	);

long line_find(
	FILE        *the_file,      /*The file to search.    */
	char        *the_string,    /*The string to look for.  */
	long        start_line,      /*The starting line.    */
	long        end_line      /*The end line.      */
	);

long line_start_find(
	FILE        *the_file,      /*The file to search.    */
	char        *the_string,    /*The string to look for.  */
	long        start_line,      /*The starting line.    */
	long        end_line      /*The end line.      */
	);

#endif
