#ifndef ENUMERATOR_PRIVATE_APP_H_
#define ENUMERATOR_PRIVATE_APP_H_

#include "general/enumerator.h"
#include "general/enumerator_app.h"

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
	char *enumerator_string_value; \
	enum enumerator_type *enumerator_address, other_enumerator = (enum enumerator_type)0; \
	int return_code; \
\
	ENTER(set_enumerator(enumerator_type)); \
	if (state&&(enumerator_address=(enum enumerator_type *)enumerator_address_void) \
		&&(enumerator_string_value=(char *)enumerator_string_value_void)) \
	{ \
		return_code=1; \
		if (!(state->current_token)|| \
			(strcmp(PARSER_HELP_STRING,state->current_token)&& \
				strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token))) \
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
	int number_of_valid_strings, const char **valid_strings, \
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
			suboption_table=CREATE(Option_table)();  \
			if (NULL != suboption_table) \
			{ \
				for (i=0;i<number_of_valid_strings;i++) \
				{ \
					Option_table_add_entry(suboption_table,valid_strings[i], \
						enumerator_address,(void *)valid_strings[i],set_enumerator_macro(enumerator_type)); \
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
	const char **valid_strings; \
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


#endif /* ENUMERATOR_PRIVATE_APP_H_ */
