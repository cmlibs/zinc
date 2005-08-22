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

#include "general/enumerator.h"

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
	enum enumerator_type enumerator_value; \
	int i; \
\
	ENTER(ENUMERATOR_GET_VALID_STRINGS(enumerator_type)); \
	valid_strings = (char **)NULL; \
	if (number_of_valid_strings) \
	{ \
		*number_of_valid_strings = 0; \
		/* valid modes are from 0 to the last one with a string */ \
		for (enumerator_value = (enum enumerator_type)0; \
			ENUMERATOR_STRING(enumerator_type)(enumerator_value); \
			enumerator_value++) \
		{ \
			if ((!conditional_function) || \
				conditional_function(enumerator_value, user_data)) \
			{ \
				(*number_of_valid_strings)++; \
			} \
		} \
		if ((0 == *number_of_valid_strings) || \
			ALLOCATE(valid_strings, char *, *number_of_valid_strings)) \
		{ \
			i = 0; \
			for (enumerator_value = (enum enumerator_type)0; enumerator_string = \
				ENUMERATOR_STRING(enumerator_type)(enumerator_value); \
				enumerator_value++) \
			{ \
				if ((!conditional_function) || \
					conditional_function(enumerator_value, user_data)) \
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
	enum enumerator_type enumerator_value; \
	int return_code; \
\
	ENTER(STRING_TO_ENUMERATOR(enumerator_type)); \
	if (enumerator_string && enumerator_value_address) \
	{ \
		enumerator_value = (enum enumerator_type)0; \
		while ((other_enumerator_string = \
			ENUMERATOR_STRING(enumerator_type)(enumerator_value)) && \
			(!fuzzy_string_compare_same_length(enumerator_string, \
				other_enumerator_string))) \
		{ \
			enumerator_value++; \
		} \
		if (other_enumerator_string) \
		{ \
			*enumerator_value_address = enumerator_value; \
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

#define DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS( enumerator_type ) \
DEFINE_DEFAULT_ENUMERATOR_GET_VALID_STRINGS_FUNCTION(enumerator_type) \
DEFINE_DEFAULT_STRING_TO_ENUMERATOR_FUNCTION(enumerator_type)

#endif /* !defined (ENUMERATOR_PRIVATE_H) */
