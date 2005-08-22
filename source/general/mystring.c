/******************************************************************************
FILE : mystring.c

LAST MODIFIED : 22 April 2004

DESCRIPTION :
Function definitions for some general purpose string functions.
???DB.  A merging of Greg's and my string.c 's
???DB.  Needs some more tidying up and rationalization.
???DB.  Need to tidy up writing error messages.
=============================================================================*/
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "general/debug.h"
#include "general/mystring.h"
/*???DB.  Would like to move memory management out of Greg's debug.h */
/*#include "mymemory.h"*/
#include "user_interface/message.h"

/*
Functions
---------
*/

void cvt_string_to_ints(char *string, int array[100], int *number)
/*******************************************************************************
DESCRIPTION:
	Puts a string of delimited integers into an array.
==============================================================================*/
{
	char *stringptr, *ptr;

	*number = 0;
	stringptr = string;
	ptr = stringptr;
	while(*stringptr)                     /* step thru string of numbers */
	{
		if(!isdigit(*stringptr))            /* if it is a delimiter ...    */
		{
			*stringptr = 0;
			if(ptr != stringptr)
			{
				array[*number] = atoi(ptr);     /* ... create an integer       */
				(*number)++;
			}
			ptr = stringptr + 1;
		}
		stringptr++;
	}
	array[*number] = atoi(ptr);
	(*number)++;

	return;
} /* cvt_string_to_ints */

int is_abbrev(char *short_string,char *long_string,int min_length)
/*******************************************************************************
LAST MODIFIED : 27 December 1995

DESCRIPTION :
Does a case insensitive comparison to see if the <short_string> is an
abbreviation of the <long_string>.
==============================================================================*/
{
	char *long_c,*short_c;
	int return_code;

	ENTER(is_abbrev);
	if ((short_c=short_string)&&(long_c=long_string)&&(min_length>0))
	{
		if ((int)strlen(short_string)>=min_length)
		{
			return_code=1;
			while ((1==return_code)&&(*short_c)&&(*long_c))
			{
				if ((((*short_c>='a')&&(*short_c<='z')) ? (*short_c)-32 : *short_c)==
					(((*long_c>='a')&&(*long_c<='z')) ? (*long_c)-32 : *long_c))
				{
					short_c++;
					long_c++;
				}
				else
				{
					return_code=0;
				}
			}
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* is_abbrev */

char *remove_before_first(char *string, char search)
/*******************************************************************************
DESCRIPTION:
==============================================================================*/
{
	char *ptr;

	ptr = string;
	/* Step forward until find the search character */
	while((*ptr != search) && (*ptr))
		ptr++;
	/*
	* If "ptr" is valid then we have found the search character, so return
	* a pointer to the next character.  Otherwise the search character was
	* not found, so return the whole string.
	*/
	if(*ptr)
	{
		ptr++;
		return(ptr);
	}
	else
		return(string);

} /* remove_before_first */


char *remove_before_last(char *string, char search)
/*******************************************************************************
DESCRIPTION:
==============================================================================*/
{
	char *ptr;

	ptr = string;
	/* Point to last character of the string */
	while(*ptr)
		ptr++;
	ptr--;
	/*
	* Step back until the end of string is at the search character or until
	* we pass the beginning of the string.  Return the following character.
	*/
	while(*ptr != search && ptr >= string)
		ptr--;
	ptr++;

	return(ptr);
} /* remove_before_last */


char *remove_after_first(char *string, char search)
/*******************************************************************************
DESCRIPTION:
==============================================================================*/
{
	char *ptr;

	/*
	* Find the first occurance of "search", and set the following character to
	* a null (terminating the string) and return the string.
	*/
	ptr = string;
	while((*ptr != search) && (*ptr))
		ptr++;
	*ptr = 0;

	return(string);
} /* remove_after_first */


char *remove_after_last(char *string, char search)
/*******************************************************************************
DESCRIPTION:
==============================================================================*/
{
	char *ptr;

	ptr = string;
	/* Point to last character of the string */
	while(*ptr)
		ptr++;
	ptr--;
	/*
	* Step back until the end of string is at the search character
	*/
	while(*ptr != search && ptr >= string)
		ptr--;
	/*
	* Terminate the string if we have found "search"
	*/
	if(ptr >= string)
		*ptr = 0;

	return(string);
} /* remove_after_last */

char *trim_string(char *string)
/*******************************************************************************
LAST MODIFIED : 11 September 1993

DESCRIPTION :
Returns a copy of the <string> with the leading and trailing white space
removed.  NB Memory is allocated by this function for the copy.
==============================================================================*/
{
	char *copy,*end,*start;
	int length;

	ENTER(trim_string);
	if (string)
	{
		start=string;
		end=start+strlen(string)-1;
		/* remove leading white space */
		while ((start<=end)&&isspace(*start))
		{
			start++;
		}
		/* remove trailing white space */
		while ((start<=end)&&isspace(*end))
		{
			end--;
		}
		length=end-start+1;
		if (ALLOCATE(copy,char,length+1))
		{
			if (length>0)
			{
				strncpy(copy,start,length);
			}
			copy[length]='\0';
		}
		else
		{
			display_message(ERROR_MESSAGE,"trim_string.  Insufficient memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"trim_string.  Missing string");
		copy=(char *)NULL;
	}
	LEAVE;

	return (copy);
} /* trim_string */

char *upcase(char *string)
/*******************************************************************************
DESCRIPTION:
==============================================================================*/
{
	char *ptr;

	/*
	* For each character in the string, convert to uppercase
	*/
	ptr=string;
	while(*ptr)
	{
		*ptr = toupper(*ptr);
		ptr++;
	}

	return(string);
} /* upcase */

char *string_to_upper(char *string)
/******************************************************************************
LAST MODIFIED : 12 November 1990

DESCRIPTION :
=============================================================================*/
{
	int i;

	ENTER(string_to_upper);
	if (string)
	{
		for (i=strlen(string);i>0;)
		{
			i--;
			if (islower(string[i]))
			{
				string[i]=(char)toupper(string[i]);
			}
		}
	}
	LEAVE;

	return (string);
} /* string_to_upper */

int read_string(FILE *in,char *format,char **string_read)
/******************************************************************************
LAST MODIFIED : 17 May 1991

DESCRIPTION :
	A routine for reading in a single string.  It allocates the memory for the
string.  It uses fscanf:
1. the format string follows the format string for scanf, but is for only one %
	item and does not have the %.
2. if field width is not specified, 256 is used.
3. it reads from the stream <in>.
???DB.  What should be the return code if no characters are read (EOF) ?
=============================================================================*/
{
	int characters_read,format_len,return_code,working_string_len;
	char *working_format,*working_string;

	ENTER(read_string);
	/* check for valid parameters */
	if (in&&format&&string_read)
	{
		format_len=strlen(format);
		if (!strcmp(format,"s"))
		/* format is s */
		{
			return_code=1;
		}
		else
		{
			if ((format[0]=='[')&&(format[format_len-1]==']'))
			/* format is [ */
			{
				return_code=1;
			}
			else
			{
/*        print_message(1,"read_string.  Format is not s or [");*/
				display_message(WARNING_MESSAGE,"read_string.  Format is not s or [");
				return_code=0;
			}
		}
		if (return_code)
		{
			/* construct a format for reading the string and counting the characters
				read */
			if (ALLOCATE(working_format,char,format_len+6)&&
				ALLOCATE(working_string,char,1))
			{
				strcpy(working_format,"%80");
				strcat(working_format,format);
				strcat(working_format,"%n");
				/* read in string, allocating extra memory as required */
				working_string_len=0;
				working_string[0]='\0';
				characters_read=80;
				while (return_code&&!feof(in)&&(characters_read==80))
				{
					working_string_len += 80;
					if (REALLOCATE(working_string,working_string,char,
						working_string_len+1))
					{
						characters_read=0;
						fscanf(in,working_format,working_string+working_string_len-80,
							&characters_read);
					}
					else
					{
/*            print_message(1,*/
						display_message(WARNING_MESSAGE,
							"read_string.  Could not allocate memory for string");
						return_code=0;
					}
				}
				if (return_code)
				{
					if (working_string)
					{
						REALLOCATE(*string_read,working_string,char,
							strlen(working_string)+1);
					}
					else
					{
						*string_read=(char *)NULL;
					}
				}
				else
				{
					if (working_string)
					{
						DEALLOCATE(working_string);
					}
				}
				DEALLOCATE(working_format);
			}
			else
			{
/*        print_message(1,*/
				display_message(WARNING_MESSAGE,
					"read_string.  Could not allocate memory for working format");
				return_code=0;
			}
		}
	}
	else
	{
/*    print_message(1,"read_string.  Invalid argument(s)");*/
		display_message(WARNING_MESSAGE,"read_string.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return(return_code);
} /* read_string */

int assign_empty_string(char **string)
/******************************************************************************
LAST MODIFIED : 17 May 1992

DESCRIPTION :
	A routine for assigning an empty string.  Assumes that if the string has
been previously assigned, it was done with mymalloc.
=============================================================================*/
{
	int return_code;

	ENTER(assign_empty_string);
	/* if the string has been previously assigned */
	DEALLOCATE(*string);
	/* allocate memory for an empty string */
	ALLOCATE(*string,char,1);
	if (*string)
	{
		/* assign the empty string */
		*string[0]='\0';
		return_code=1;
	}
	else
	{
/*    return_code=print_message(1,"assign_empty_string.  Insufficient memory");*/
		return_code=
			display_message(WARNING_MESSAGE,"assign_empty_string.  Insufficient memory");
		return_code=0;
	}
	LEAVE;

	return(return_code);
} /* assign_empty_string */

char *remove_leading_trailing_blanks(char *in_string)
/******************************************************************************
LAST MODIFIED : 17 May 1992

DESCRIPTION :
=============================================================================*/
{
	int string_length,offset,i;
	char *out_string=NULL,stop;

	ENTER(remove_leading_trailing_blanks);
	/* strip trailing blanks */
	string_length=strlen(in_string)-1;
	stop=0;
	while ((string_length>=0)&&!stop)
	{
		if (in_string[string_length]==' ')
		{
			string_length--;
		}
		else
		{
			stop=1;
		}
	}
	string_length++;
	/* strip leading blanks */
	offset=0;
	stop=0;
	while ((offset<string_length)&&!stop)
	{
		if (in_string[offset]==' ')
		{
			offset++;
		}
		else
		{
			stop=1;
		}
	}
	string_length=string_length-offset;
	/* set up storage to for out_string */
	if (ALLOCATE(out_string,char,string_length+1))
	{
		/* copy in_string */
		for (i=0;i<string_length;i++)
		{
			out_string[i]=in_string[i+offset];
		}
		out_string[i]='\0';
	}
	LEAVE;

	return(out_string);
} /* remove_leading_trailing_blanks */

int append_string(char **string1,char *string2,int *error)
/*******************************************************************************
LAST MODIFIED : 2 December 1998

DESCRIPTION :
Concatenates <string2> on to the end of <*string1> by reallocating <*string1> to
fit. <*string1> may start off as NULL or an existing allocated string.
If <*error> is 1, nothing is done; if an error occurs, <*error> is set to 1 and
<*string1> is deallocated. Repeated calls to this function after an error has
occurred thus do not add to the string, and do not result in further errors.
It is up to the calling function to deallocate the returned string.
==============================================================================*/
{
	char *new_string;
	int current_length;

	ENTER(append_string);
	if (string1&&string2&&error)
	{
		if (!(*error))
		{
			if (*string1)
			{
				current_length=strlen(*string1);
			}
			else
			{
				current_length=0;
			}
			if (REALLOCATE(new_string,*string1,char,current_length+strlen(string2)+1))
			{
				if (*string1)
				{
					strcat(new_string,string2);
				}
				else
				{
					strcpy(new_string,string2);
				}
				*string1=new_string;
			}
			else
			{
				display_message(ERROR_MESSAGE,"append_string.  Could not reallocate");
				*error=1;
				if (*string1)
				{
					DEALLOCATE(*string1);
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"append_string.  Invalid argument(s)");
		*error=1;
	}
	LEAVE;

	return (!(*error));
} /* append_string */

int has_suffix(char *string,char *suffix)
/*******************************************************************************
LAST MODIFIED : 25 November 1999

DESCRIPTION :
Returns true if <string> ends in <suffix>.
==============================================================================*/
{
	int return_code,string_length,suffix_length;

	ENTER(has_suffix);
	if (string&&suffix)
	{
		/* 1. string must be at least as long as the suffix */
		if ((string_length=strlen(string)) >= (suffix_length=strlen(suffix)))
		{
			/* 2. string must end in the suffix */
			return_code = (0==strcmp(string+(string_length-suffix_length),suffix));
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"has_suffix.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* has_suffix */

int check_suffix(char **string, char *suffix)
/*******************************************************************************
LAST MODIFIED : 22 April 2004

DESCRIPTION :
Compares the file extension of the string.  If the string given has the 
supplied suffix (or the universal suffix .cmiss) then the function returns 
without changing anything.  Otherwise the string is REALLOCATED and the suffix
added to the end of the string.
==============================================================================*/
{
	char *new_string;
	int return_code;

	ENTER(compare_suffix);
	if (string&&(*string)&&suffix)
	{
		if (strchr(*string, '.'))
		{
			return_code=1;
		}
		else
		{
			if (REALLOCATE(new_string, *string, char, strlen(*string)
				+strlen(suffix)+1))
			{
				*string = new_string;
				strcat(*string, suffix);
				return_code = 1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"compare_suffix.  Unable to REALLOCATE string");
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"compare_suffix.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* compare_suffix */

char *duplicate_string(char *source_string)
/*******************************************************************************
LAST MODIFIED : 2 December 1998

DESCRIPTION :
Returns an allocated copy of <source_string>, or NULL in case of error.
==============================================================================*/
{
	char *copy_of_string;

	ENTER(duplicate_string);
	if (source_string)
	{
		if (ALLOCATE(copy_of_string,char,strlen(source_string)+1))
		{
			strcpy(copy_of_string,source_string);
		}
		else
		{
			display_message(ERROR_MESSAGE,"duplicate_string.  Not enough memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"duplicate_string.  Invalid argument(s)");
		copy_of_string=(char *)NULL;
	}
	LEAVE;

	return (copy_of_string);
} /* duplicate_string */

int string_matches_without_whitespace(char *input_string,char *match_string)
/*******************************************************************************
LAST MODIFIED : 2 September 1999

DESCRIPTION :
Returns true if all the non-whitespace characters in <input_string> match those
in <match_string>. Whitespace characters (space,tab) are only allowed in
<input_string> where they appear in match_string.
==============================================================================*/
{
	char *s1,*s2;
	int return_code;

	ENTER(string_matches_without_whitespace);
	if ((s1=input_string)&&(s2=match_string))
	{
		return_code=1;
		while (return_code&&(*s1)&&(*s2))
		{
			if ((' '== (*s2))||('\t'== (*s2)))
			{
				while ((' '== (*s1))||('\t'== (*s1)))
				{
					s1++;
				}
				while ((' '== (*s2))||('\t'== (*s2)))
				{
					s2++;
				}
			}
			if ((*s1) != (*s2))
			{
				return_code=0;
			}
			if (*s1)
			{
				s1++;
			}
			if (*s2)
			{
				s2++;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"string_matches_without_whitespace.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* string_matches_without_whitespace */

int is_standard_object_name(char *name)
/*******************************************************************************
LAST MODIFIED : 12 May 2003

DESCRIPTION :
Returns true if <name> is a valid name for an object.
Names may start with a letter and contain only alphanumeric characters,
underscore '_' or spaces and may not end in a space.
???RC OK, it's allowed to start with a number and have dots in it, since
CM already has group names called 1..945.
???RC OK, unemap uses colons in some groups names. Oh well.
==============================================================================*/
{
	int i, length, return_code;

	ENTER(is_standard_object_name);
	if (name)
	{
		if (isalnum(name[0]))
		{
			return_code = 1;
			length = strlen(name);
			for (i = 1; (i < length) && return_code; i++)
			{
				if (name[i] == ' ')
				{
					if (i == (length - 1))
					{
						return_code = 0;
					}
				}
				else if ((name[i] != '_') && (name[i] != '.') && (name[i] != ':') &&
					(!isalnum(name[i])))
				{
					return_code = 0;
				}
			}
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"is_standard_object_name.  Missing name");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* is_standard_object_name */

int make_valid_token(char **token_address)
/*******************************************************************************
LAST MODIFIED : 10 January 2001

DESCRIPTION :
If the string pointed to by <token_address> contains any special characters such
that, if parsed, it would not be read in its entirety as a single token, this
function reallocates and redefines the string so that it is surrounded by
quotes. Any quotes in the original string are put back in pairs so they are read
as one quote when parsed, as explained in function extract_token().
Special characters include token separators (whitespace/,/;/=), comment
characters (#) and characters that must be "escaped", ie. preceded by a
backslash in perl, namely \, ", ', $.
NOTE: the string pointed to by <token_address> must be non-static and allowed
to be reallocated.
==============================================================================*/
{
	char *new_token, *letter;
	int number_of_escapes, old_length, return_code, special_chars;

	ENTER(make_valid_token);
	if (token_address && (letter = *token_address))
	{
		return_code = 1;
		/* work out if string contains special characters */
		special_chars = 0;
		number_of_escapes = 0;
		while (*letter)
		{
			if (isspace(*letter) || (',' == *letter) ||
				(';' == *letter) || ('=' == *letter) ||
				('#' == *letter))
			{
				special_chars++;
			}
			else if (('\"' == *letter) || ('\'' == *letter) ||
				('\\' == *letter) || ('$' == *letter))
			{
				special_chars++;
				number_of_escapes++;
			}
			letter++;
		}
		if (special_chars)
		{
			old_length = strlen(*token_address);
			if (REALLOCATE(new_token, *token_address, char,
				old_length + number_of_escapes + 3))
			{
				*token_address = new_token;
				letter = *token_address + old_length - 1;
				new_token = letter + number_of_escapes + 3;
				/* zero at end of string */
				*new_token = '\0';
				new_token--;
				/* final quote */
				*new_token = '\"';
				new_token--;
				while (letter >= *token_address)
				{
					*new_token = *letter;
					new_token--;
					if (('\"' == *letter) || ('\'' == *letter) ||
						('\\' == *letter) || ('$' == *letter))
					{
						/* add escape character \ */
						*new_token = '\\';
						new_token--;
					}
					letter--;
				}
				/* opening quote */
				*new_token = '\"';
			}
			else
			{
				display_message(ERROR_MESSAGE, "make_valid_token.  Not enough memory");
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "make_valid_token.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* make_valid_token */

#if defined (VAXC)
char *strrpbrk(const char *s1,const char *s2)
/******************************************************************************
LAST MODIFIED : 5 October 1991

DESCRIPTION :
=============================================================================*/
{
	int i;
	char *return_value=NULL;

	i=strlen(s1)-1;
	while ((i>=0)&&(!strchr(s2,s1[i])))
	{
		i--;
	}
	if (i>=0)
	{
		return_value=s1+i;
	}
	else
	{
		return_value=(char *)NULL;
	}

	return (return_value);
} /* strrpbrk */
#endif
