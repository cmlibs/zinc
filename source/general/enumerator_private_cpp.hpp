/*******************************************************************************
FILE : enumerator_private.h

LAST MODIFIED : 19 March 2001

DESCRIPTION :
Definitions of macro functions for handling enumerators.
Useful for automatic menu/command/dialog widget generation.
Usage:
1. #include enumerator.h in the .h file you are defining an enumerator in.
2. Call the prototypes for the types and functions you wish to use. Prototype
all of them with PROTOTYPE_ENUMERATOR_FUNCTIONS(enumerator type).
3. #include enumerator_private.h in the .c file matching your .h file above.
4. Write an ENUMERATOR_STRING function for your enumerator type.
5. Call the default function definition macros for any other functions you wish
to use; note they assume the range of enumerated values starts at 0 and
increments by 1 to the last value which has an ENUMERATOR_STRING. If your enum
declaration does not match this pattern, you will have to write the contents
of these functions yourself.
==============================================================================*/
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
#if !defined (ENUMERATOR_PRIVATE_H)
#define ENUMERATOR_PRIVATE_H

extern "C" {
#include "general/enumerator.h"
}

/*
Macros
======
*/

/*
Global functions
----------------
*/

#define DEFINE_DEFAULT_ENUMERATOR_GET_VALID_STRINGS_FUNCTION( enumerator_type )\
PROTOTYPE_ENUMERATOR_GET_VALID_STRINGS_FUNCTION(enumerator_type) \
/***************************************************************************** \
LAST MODIFIED : 19 March 2001 \
\
DESCRIPTION : \
Returns an allocated array containing the ENUMERATOR_STRINGs for all valid \
enumerator values in the enumerator_type. The optional <conditional_function> \
and <user_data> allow reduced selections of strings to be returned. \
Note it is up to the calling function to deallocate the returned array, but \
the strings it contains must NOT be deallocated. \
Default version assumes all valid enumerator values are sequential from 0. \
============================================================================*/ \
{ \
	char *enumerator_string, **valid_strings; \
	int enumerator_value, i;		       \
\
	ENTER(ENUMERATOR_GET_VALID_STRINGS(enumerator_type)); \
	valid_strings = (char **)NULL; \
	if (number_of_valid_strings) \
	{ \
		*number_of_valid_strings = 0; \
		/* valid modes are from 0 to the last one with a string */ \
		for (enumerator_value = 0; \
		     ENUMERATOR_STRING(enumerator_type)(static_cast<enum enumerator_type >(enumerator_value)); \
		        enumerator_value++)		\
		{ \
			if ((!conditional_function) || \
			    conditional_function(static_cast<enum enumerator_type >(enumerator_value), user_data)) \
			{ \
				(*number_of_valid_strings)++; \
			} \
		} \
		if ((0 == *number_of_valid_strings) || \
			ALLOCATE(valid_strings, char *, *number_of_valid_strings)) \
		{ \
			i = 0; \
			for (enumerator_value = 0; enumerator_string = \
			       ENUMERATOR_STRING(enumerator_type)(static_cast<enum enumerator_type >(enumerator_value)); \
				enumerator_value++) \
			{ \
				if ((!conditional_function) || \
				    conditional_function(static_cast<enum enumerator_type >(enumerator_value), user_data)) \
				{ \
					valid_strings[i] = enumerator_string; \
					i++; \
				} \
			} \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"ENUMERATOR_GET_VALID_STRINGS(" #enumerator_type \
				").  Not enough memory"); \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"ENUMERATOR_GET_VALID_STRINGS(" #enumerator_type \
			").  Invalid argument(s)"); \
	} \
	LEAVE; \
\
	return (valid_strings); \
} /* ENUMERATOR_GET_VALID_STRINGS(enumerator_type) */

#define DEFINE_DEFAULT_STRING_TO_ENUMERATOR_FUNCTION( enumerator_type ) \
PROTOTYPE_STRING_TO_ENUMERATOR_FUNCTION(enumerator_type) \
/***************************************************************************** \
LAST MODIFIED : 16 March 2001 \
\
DESCRIPTION : \
If <enumerator_string> matches the ENUMERATOR_STRING(enumerator_type) for any \
enumerator_value, returns the enumerator_value in *<enumerator_value_address>, \
with a true return_code. Otherwise a false return_code is returned, without  \
an error message, for the calling function to handle. \
Default version assumes all valid enumerator values are sequential from 0. \
============================================================================*/ \
{ \
	char *other_enumerator_string; \
	int enumerator_value, return_code;	\
\
	ENTER(STRING_TO_ENUMERATOR(enumerator_type)); \
	if (enumerator_string && enumerator_value_address) \
	{ \
		enumerator_value = (enum enumerator_type)0; \
		while ((other_enumerator_string = \
			ENUMERATOR_STRING(enumerator_type)(static_cast<enum enumerator_type>(enumerator_value))) && \
			(!fuzzy_string_compare_same_length(enumerator_string, \
				other_enumerator_string))) \
		{ \
			enumerator_value++; \
		} \
		if (other_enumerator_string) \
		{ \
		        *enumerator_value_address = static_cast<enum enumerator_type>(enumerator_value); \
			return_code = 1; \
		} \
		else \
		{ \
			return_code = 0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"STRING_TO_ENUMERATOR(" #enumerator_type ").  Invalid argument(s)"); \
		return_code = 0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* STRING_TO_ENUMERATOR(enumerator_type) */

#define set_enumerator_macro(enumerator_type) set_enumerator ## enumerator_type

#define Option_table_add_enumerator_macro(enumerator_type) \
	Option_table_add_enumerator ## enumerator_type

#define DEFINE_DEFAULT_OPTION_TABLE_ADD_ENUMERATOR_FUNCTION( enumerator_type ) \
static int set_enumerator_macro(enumerator_type) \
	(struct Parse_state *state, \
	void *enumerator_address_void ,void *enumerator_string_value_void) \
/******************************************************************************* \
LAST MODIFIED : 4 November 2005 \
 \
DESCRIPTION : \
A modifier function for setting an enumerated type variable to a specified \
value. \
==============================================================================*/ \
{ \
	char *current_token,*enumerator_string_value; \
	enum enumerator_type *enumerator_address, other_enumerator; \
	int return_code; \
\
	ENTER(set_enumerator(enumerator_type)); \
	if (state&&(enumerator_address=(enum enumerator_type *)enumerator_address_void) \
		&&(enumerator_string_value=(char *)enumerator_string_value_void)) \
	{ \
		return_code=1; \
		if (!(current_token=state->current_token)|| \
			(strcmp(PARSER_HELP_STRING,current_token)&& \
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))) \
		{ \
			STRING_TO_ENUMERATOR(enumerator_type)( \
				enumerator_string_value, enumerator_address); \
		} \
		else \
		{ \
			STRING_TO_ENUMERATOR(enumerator_type)(enumerator_string_value, &other_enumerator); \
			if (*enumerator_address == other_enumerator) \
			{ \
				display_message(INFORMATION_MESSAGE,"[%s]",enumerator_string_value); \
			} \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"set_enumerator_macro(enumerator_type).  Invalid argument(s)"); \
		return_code=0; \
	} \
	LEAVE; \
 \
	return (return_code); \
} /* set_enumerator_macro(enumerator_type) */ \
\
static int Option_table_add_enumerator_macro(enumerator_type)(struct Option_table *option_table, \
	int number_of_valid_strings, char **valid_strings, \
	enum enumerator_type *enumerator_address) \
/******************************************************************************* \
LAST MODIFIED : 4 November 2005 \
\
DESCRIPTION : \
Adds a newly created suboption table for all the valid_strings for the \
enumerator. The <valid_strings> array should contain <number_of_valid_strings> \
pointers to static strings, one per enumerator option. Responsibility for \
deallocating this array is left to the calling function. The static string value \
of the enumerator is maintained in <enumerator_string_address> and it is up to \
the calling function to convert back to an enumerated value. \
Note that if any error occurs, the option_table is marked as being invalid and \
no further errors will be reported on subsequent calls. \
==============================================================================*/ \
{ \
	int i,return_code; \
	struct Option_table *suboption_table; \
 \
	ENTER(Option_table_add_enumerator_macro(enumerator_type)); \
	if (option_table && (0<number_of_valid_strings) && valid_strings && \
		enumerator_address) \
	{ \
		if (Option_table_is_valid(option_table))	\
		{ \
			if (suboption_table=CREATE(Option_table)()) \
			{ \
				for (i=0;i<number_of_valid_strings;i++) \
				{ \
					Option_table_add_entry(suboption_table,valid_strings[i], \
						enumerator_address,valid_strings[i],set_enumerator_macro(enumerator_type)); \
				} \
				return_code= \
					Option_table_add_suboption_table(option_table,suboption_table); \
			} \
			else \
			{ \
				display_message(ERROR_MESSAGE, \
					"Option_table_add_enumerator_macro(enumerator_type).  Not enough memory"); \
				return_code=0; \
				Option_table_set_invalid(option_table);	\
			} \
		} \
		else \
		{ \
			/* report no further errors */ \
			return_code=0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"Option_table_add_enumerator_macro(enumerator_type).  Invalid argument(s)"); \
		return_code=0; \
	} \
	LEAVE; \
 \
	return (return_code); \
} /* Option_table_add_enumerator_macro(enumerator_type) */ \
\
PROTOTYPE_OPTION_TABLE_ADD_ENUMERATOR_FUNCTION(enumerator_type) \
/***************************************************************************** \
LAST MODIFIED : 4 November 2005 \
\
DESCRIPTION : \
============================================================================*/ \
{ \
	char **valid_strings; \
	int number_of_valid_strings, return_code;	\
\
	ENTER(OPTION_TABLE_ADD_ENUMERATOR(enumerator_type)); \
	if (option_table) \
	{ \
		valid_strings = ENUMERATOR_GET_VALID_STRINGS(enumerator_type)( \
			&number_of_valid_strings, \
         (ENUMERATOR_CONDITIONAL_FUNCTION(enumerator_type) *)NULL, \
			(void *)NULL); \
		return_code = Option_table_add_enumerator_macro(enumerator_type)(option_table, number_of_valid_strings, \
			valid_strings, enumerator); \
		DEALLOCATE(valid_strings); \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"OPTION_TABLE_ADD_ENUMERATOR(" #enumerator_type \
			").  Invalid argument(s)"); \
		return_code = 0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* OPTION_TABLE_ADD_ENUMERATOR(enumerator_type) */

#define DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS( enumerator_type ) \
DEFINE_DEFAULT_ENUMERATOR_GET_VALID_STRINGS_FUNCTION(enumerator_type) \
DEFINE_DEFAULT_STRING_TO_ENUMERATOR_FUNCTION(enumerator_type) \
DEFINE_DEFAULT_OPTION_TABLE_ADD_ENUMERATOR_FUNCTION(enumerator_type)

#endif /* !defined (ENUMERATOR_PRIVATE_H) */
