/*******************************************************************************
FILE : parser.c

LAST MODIFIED : 18 September 2000

DESCRIPTION :
A module for supporting command parsing.
???DB.  What about allocate and deallocate
???DB.  Make the set functions all look for "?" instead of NULL parser_state ?
???DB.  Move variables into own module ?
???DB.  Help for set_char_flag ?
???DB.  Move fuzzy_string_compare to compare.c ?
???DB.  Move extract_token into mystring.h ?
==============================================================================*/
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "command/parser.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "general/object.h"
#include "general/indexed_list_private.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/* size of blocks allocated onto option table - to reduce number of reallocs */
#define OPTION_TABLE_ALLOCATE_SIZE 10

/*
Module types
------------
*/

struct Option_table
/*******************************************************************************
LAST MODIFIED : 23 December 1999

DESCRIPTION :
==============================================================================*/
{
	struct Modifier_entry *entry;
	int allocated_entries,number_of_entries,valid;
	/* store suboption_tables added to table for destroying with option_table */
	int number_of_suboption_tables;
	struct Option_table **suboption_tables;
}; /* struct Option_table */

enum Variable_operation_type
{
	ADD_VARIABLE_OPERATION,
	DIVIDE_VARIABLE_OPERATION,
	MULTIPLY_VARIABLE_OPERATION,
	SET_VARIABLE_OPERATION,
	SUBTRACT_VARIABLE_OPERATION
}; /* enum Variable_operation_type */

/* SAB.  First implementation of "assign variable" to conform to "new interpreter".
   These are string variables, are case sensitive and have global scope.
	The use of a variable is indicated by a $ symbol in any token. */
struct Assign_variable
{
	char *name;
	char *value;

	int access_count;
}; /* struct Assign_variable */

/*
Module variables
----------------
*/
/*???DB.  Initial go at variables of form %<f|i|z|l><#>% , where the %'s are
	required, f(loat) i(nteger) z(ero extended integer) l(ogical) are the variable
	type and the # is an integer.  Can easily be extended to general names and
	arbitrary numbers of variables */
#define MAX_VARIABLES 100
static float variable_float[MAX_VARIABLES];
static int exclusive_option=0,multiple_options=0,usage_indentation_level=0,
	usage_newline;

DECLARE_LIST_TYPES(Assign_variable);
FULL_DECLARE_INDEXED_LIST_TYPE(Assign_variable);

PROTOTYPE_OBJECT_FUNCTIONS(Assign_variable);
PROTOTYPE_LIST_FUNCTIONS(Assign_variable);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Assign_variable,name,void *);

static struct LIST(Assign_variable) *assign_variable_list = NULL;

/*
Module functions
----------------
*/
static struct Assign_variable *CREATE(Assign_variable)(char *name)
/*******************************************************************************
LAST MODIFIED : 10 March 2000

DESCRIPTION :
==============================================================================*/
{
	struct Assign_variable *variable;
	
	if (name)
	{
		if (ALLOCATE(variable, struct Assign_variable, 1)
			&& ALLOCATE(variable->name, char, strlen(name)+1))
		{
			strcpy(variable->name, name);
			variable->value = (char *)NULL;
			variable->access_count = 0;
			/* Add into the global list */
			if (!assign_variable_list)
			{
				assign_variable_list = CREATE_LIST(Assign_variable)();
			}
			
			if (!(ADD_OBJECT_TO_LIST(Assign_variable)(variable, assign_variable_list)))
			{
				display_message(ERROR_MESSAGE,
					"CREATE(Assign_variable).  Unable to add variable to global list");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Assign_variable).  Unable to allocate memory for assign_variable structure");
			DEALLOCATE(variable);
			variable = (struct Assign_variable *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Assign_variable).  Invalid arguments");
		variable = (struct Assign_variable *)NULL;
	}
			
	
	return(variable);
}

int DESTROY(Assign_variable)(struct Assign_variable **variable_address)
/*******************************************************************************
LAST MODIFIED : 10 March 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Assign_variable *variable;

	if (variable_address && (variable = *variable_address))
	{
		if (variable->access_count <= 1)
		{
			if (variable->access_count == 1)
			{
				if (assign_variable_list)
				{
					/* Check that it is only the global list and then remove */
					if (IS_OBJECT_IN_LIST(Assign_variable)(variable,
						assign_variable_list))
					{
						REMOVE_OBJECT_FROM_LIST(Assign_variable)(variable,
							assign_variable_list);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"DESTROY(Assign_variable).  Destroy called when access count == 1 and the variable isn't in the global list.");
						*variable_address = (struct Assign_variable *)NULL;
						return_code = 0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"DESTROY(Assign_variable).  Destroy called when access count == 1 and there isn't a global list.");
					*variable_address = (struct Assign_variable *)NULL;
					return_code = 0;
				}
			}
			else
			{
				return_code = 1;
			}
			if (return_code)
			{
				if (variable->name)
				{
					DEALLOCATE(variable->name);
				}
				if (variable->value)
				{
					DEALLOCATE(variable->value);
				}
				DEALLOCATE(variable);
				*variable_address = (struct Assign_variable *)NULL;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"DESTROY(Assign_variable).  Destroy called when access count > 1.");
			*variable_address = (struct Assign_variable *)NULL;
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Assign_variable).  Invalid arguments.");
		return_code = 0;
	}

	return (return_code);
}

static int Assign_variable_set_value(struct Assign_variable *variable, char *value)
/*******************************************************************************
LAST MODIFIED : 10 March 2000

DESCRIPTION :
==============================================================================*/
{
	char *new_value;
	int return_code;
	
	if (variable && value)
	{
		if (REALLOCATE(new_value, variable->value, char, strlen(value) + 1))
		{
			variable->value = new_value;
			strcpy(variable->value, value);
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Assign_variable_set_value.  Unable to allocate memory for assign_variable value");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Assign_variable_set_value.  Invalid arguments");
		return_code = 0;
	}

	return(return_code);
} /* Assign_variable_set_value */

DECLARE_OBJECT_FUNCTIONS(Assign_variable)
DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Assign_variable,name,void *,strcmp)
DECLARE_INDEXED_LIST_FUNCTIONS(Assign_variable)
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Assign_variable,name,void *,
	strcmp)

static int reduce_fuzzy_string(char *reduced_string,char *string)
/*******************************************************************************
LAST MODIFIED : 16 September 1998

DESCRIPTION :
Copies <string> to <reduced_string> converting to upper case and removing
whitespace, -'s and _'s.
???RC Removed filtering of dash and underline.
==============================================================================*/
{
	char *destination,*source;
	int return_code;

	ENTER(reduce_fuzzy_string);
	if ((source=string)&&(destination=reduced_string))
	{
		while (*source)
		{
			/* remove whitespace, -'s and _'s */
			/*???RC don't know why you want to exclude - and _. I had a case where
				I typed gfx create fibre_ (short for fibre_field) and it thought it was
				ambiguous since there is also a gfx create fibres commands. */
			if (!isspace(*source)/*&&(*source!='-')&&(*source!='_')*/)
			{
				*destination=toupper(*source);
				destination++;
			}
			source++;
		}
		/* terminate the string */
		*destination='\0';
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"reduce_fuzzy_string.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* reduce_fuzzy_string */

static int execute_variable_command_operation(struct Parse_state *state,
	void *operation_type_void,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 21 June 1999

DESCRIPTION :
Executes a VARIABLE operation command.
==============================================================================*/
{
	char *current_token;
	enum Variable_operation_type *operation_type;
	float value;
	int number,return_code;

	ENTER(execute_variable_command_operation);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (operation_type=(enum Variable_operation_type *)operation_type_void)
				{
					/* get number */
					if (1==sscanf(current_token," %i",&number))
					{
						shift_Parse_state(state,1);
						if (current_token=state->current_token)
						{
							/* get value */
							if (1==sscanf(current_token," %f",&value))
							{
								return_code=0;
								switch (*operation_type)
								{
									case ADD_VARIABLE_OPERATION:
									{
										variable_float[number] += value;
									} break;
									case DIVIDE_VARIABLE_OPERATION:
									{
										variable_float[number] /= value;
									} break;
									case MULTIPLY_VARIABLE_OPERATION:
									{
										variable_float[number] *= value;
									} break;
									case SET_VARIABLE_OPERATION:
									{
										variable_float[number]=value;
									} break;
									case SUBTRACT_VARIABLE_OPERATION:
									{
										variable_float[number] -= value;
									} break;
									default:
									{
										display_message(ERROR_MESSAGE,
						"execute_variable_command_operation.  Unknown variable operation");
										return_code=0;
									} break;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,"Invalid variable value: %s",
									current_token);
								display_parse_state_location(state);
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"Missing variable value");
							display_parse_state_location(state);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Invalid variable number: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"execute_variable_command_show.  Missing operation_type");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," VARIABLE_NUMBER SET_VALUE");
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing variable number");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_variable_command_operation.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_variable_command_operation */

static int execute_variable_command_show(struct Parse_state *state,
	void *dummy_to_be_modified,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 21 June 1999

DESCRIPTION :
Executes a VARIABLE SHOW command.
==============================================================================*/
{
	char *current_token;
	int number,return_code;

	ENTER(execute_variable_command_show);
	USE_PARAMETER(dummy_to_be_modified);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				/* get number */
				if (1==sscanf(current_token," %i",&number))
				{
					display_message(INFORMATION_MESSAGE,"Variable %i = %g\n",number,
						variable_float[number]);
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,"Invalid variable number: %s",
						current_token);
					display_parse_state_location(state);
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," VARIABLE_NUMBER");
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing variable number");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_variable_command_show.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_variable_command_show */

/*
Global functions
----------------
*/
int fuzzy_string_compare(char *first,char *second)
/*******************************************************************************
LAST MODIFIED : 30 August 2000

DESCRIPTION :
This is a case insensitive compare disregarding certain characters (whitespace,
dashes and underscores).  For example, "Ambient Colour" matches the following:

"AMBIENT_COLOUR", "ambient_colour", "ambientColour", "Ambient_Colour",
"ambient-colour", "AmBiEnTcOlOuR", "Ambient-- Colour"

and a large set of even louder versions.

Both strings are first reduced, which removes whitespace and converts to upper
case. Returns 1 iff the reduced second string is at least as long as the
reduced first string and starts with the characters in the first.
==============================================================================*/
{
	char *first_reduced,*second_reduced;
	int compare_length,first_length,return_code,second_length;

	ENTER(fuzzy_string_compare);
	if (first&&second)
	{
		/*???Edouard.  Malloc()ing and free()ing space for every string compare is
			perhaps going to eat a few too many cycles in the final analysis. I think
			that the best idea would be to at some point in the future recode the
			following to compare the strings 'live' by reducing and skipping the chars
			one by one. I've done this before, but it's generally not all that
			clean-looking, so I'm trying to write a simple one first off */
		/* allocate memory */
		if (ALLOCATE(first_reduced,char,strlen(first)+1)&&
			ALLOCATE(second_reduced,char,strlen(second)+1))
		{
			/* reduce strings */
			if (reduce_fuzzy_string(first_reduced,first)&&
				reduce_fuzzy_string(second_reduced,second))
			{
				first_length=strlen(first_reduced);
				second_length=strlen(second_reduced);
				/* first reduced string must not be longer than second */
				if (first_length <= second_length)
				{
					compare_length=first_length;
					if (strncmp(first_reduced,second_reduced,compare_length))
					{
						return_code=0;
					}
					else
					{
						return_code=1;
					}
				}
				else
				{
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"fuzzy_string_compare.  Error reducing");
				return_code=0;
			}
			DEALLOCATE(first_reduced);
			DEALLOCATE(second_reduced);
		}
		else
		{
			DEALLOCATE(first_reduced);
			display_message(ERROR_MESSAGE,
				"fuzzy_string_compare.  Insufficient memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"fuzzy_string_compare.  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* fuzzy_string_compare */

int fuzzy_string_compare_same_length(char *first,char *second)
/*******************************************************************************
LAST MODIFIED : 14 August 1998

DESCRIPTION :
Same as fuzzy_string_compare except that the two reduced strings must be the
same length.
==============================================================================*/
{
	char *first_reduced,*second_reduced;
	int return_code;

	ENTER(fuzzy_string_compare_same_length);
	if (first&&second)
	{
		if (ALLOCATE(first_reduced,char,strlen(first)+1)&&
			ALLOCATE(second_reduced,char,strlen(second)+1))
		{
			/* reduce strings */
			if (reduce_fuzzy_string(first_reduced,first)&&
				reduce_fuzzy_string(second_reduced,second))
			{
				if (0==strcmp(first_reduced,second_reduced))
				{
					return_code=1;
				}
				else
				{
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"fuzzy_string_compare_same_length.  Error reducing");
				return_code=0;
			}
			DEALLOCATE(first_reduced);
			DEALLOCATE(second_reduced);
		}
		else
		{
			DEALLOCATE(first_reduced);
			display_message(ERROR_MESSAGE,
				"fuzzy_string_compare_same_length.  Insufficient memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"fuzzy_string_compare_same_length.  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* fuzzy_string_compare_same_length */

int process_option(struct Parse_state *state,
	struct Modifier_entry *modifier_table)
/*******************************************************************************
LAST MODIFIED : 21 December 1999

DESCRIPTION :
If the <state->current_token> is "?", then the options in the <modifier_table>
and the values expected for each will be written to the command window and 1
returned.  Otherwise, the <modifier_table> is searched for entries whose option
field matchs <state->current_token>.  If no matchs are found, then if the
terminating entry in the <modifier_table> has a modifier function it is called,
otherwise an error message is written and 0 returned.  If one match is found,
then the modifier function of the entry is called and its return value returned.
If more than one match is found then the possible matchs are written to the
command window and 0 is returned.  Note that <process_option> is a modifier
function.
==============================================================================*/
{
	char *current_token,*error_message,*temp_message,**token;
	int first,i,match_length,number_of_sub_entries,return_code;
	struct Modifier_entry *entry,*matching_entry,*sub_entry;

	ENTER(process_option);
	exclusive_option++;
	if (state&&(entry=modifier_table))
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				return_code=0;
				matching_entry=(struct Modifier_entry *)NULL;
				match_length=strlen(current_token);
				error_message=(char *)NULL;
				while ((entry->option)||((entry->user_data)&&!(entry->modifier)))
				{
					if (entry->option)
					{
						if (fuzzy_string_compare(current_token,entry->option))
						{
							if (matching_entry)
							{
								if (return_code)
								{
									return_code=0;
									if (ALLOCATE(error_message,char,38+match_length+
										strlen(matching_entry->option)+strlen(entry->option)))
									{
										strcpy(error_message,"Ambiguous option <");
										strcat(error_message,current_token);
										strcat(error_message,"> could be <");
										strcat(error_message,matching_entry->option);
										strcat(error_message,"> or <");
										strcat(error_message,entry->option);
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"process_option.  Multiple match, insufficient memory");
									}
								}
								else
								{
									if (error_message)
									{
										if (REALLOCATE(temp_message,error_message,char,
											strlen(error_message)+8+strlen(entry->option)))
										{
											error_message=temp_message;
											strcat(error_message,"> or <");
											strcat(error_message,entry->option);
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"process_option.  Multiple match, insufficient memory");
											DEALLOCATE(error_message);
										}
									}
								}
							}
							else
							{
								matching_entry=entry;
								return_code=1;
							}
						}
					}
					else
					{
						/* assume that the user_data is another option table */
						sub_entry=(struct Modifier_entry *)(entry->user_data);
						while (sub_entry->option)
						{
							if (fuzzy_string_compare(current_token,sub_entry->option))
							{
								if (matching_entry)
								{
									if (return_code)
									{
										return_code=0;
										if (ALLOCATE(error_message,char,38+match_length+
											strlen(matching_entry->option)+strlen(sub_entry->option)))
										{
											strcpy(error_message,"Ambiguous option <");
											strcat(error_message,current_token);
											strcat(error_message,"> could be <");
											strcat(error_message,matching_entry->option);
											strcat(error_message,"> or <");
											strcat(error_message,sub_entry->option);
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"process_option.  Multiple match, insufficient memory");
										}
									}
									else
									{
										if (error_message)
										{
											if (REALLOCATE(temp_message,error_message,char,
												strlen(error_message)+8+strlen(sub_entry->option)))
											{
												error_message=temp_message;
												strcat(error_message,"> or <");
												strcat(error_message,sub_entry->option);
											}
											else
											{
												display_message(ERROR_MESSAGE,
												"process_option.  Multiple match, insufficient memory");
												DEALLOCATE(error_message);
											}
										}
									}
								}
								else
								{
									matching_entry=sub_entry;
									return_code=1;
								}
							}
							sub_entry++;
						}
					}
					entry++;
				}
				if (return_code)
				{
					if (shift_Parse_state(state,1))
					{
						return_code=(matching_entry->modifier)(state,
							matching_entry->to_be_modified,matching_entry->user_data);
					}
					else
					{
						display_message(ERROR_MESSAGE,"process_option.  Error parsing");
						return_code=0;
					}
				}
				else
				{
					if (!matching_entry)
					{
						/* use the default modifier function if it exists */
						if (entry->modifier)
						{
							return_code=(entry->modifier)(state,entry->to_be_modified,
								entry->user_data);
						}
						else
						{
							if (ALLOCATE(error_message,char,18+match_length))
							{
								strcpy(error_message,"Unknown option <");
								strcat(error_message,current_token);
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"process_option.  No match, insufficient memory");
							}
						}
					}
					if (error_message)
					{
						strcat(error_message,">");
						display_message(ERROR_MESSAGE,error_message);
						DEALLOCATE(error_message);
						display_parse_state_location(state);
					}
				}
			}
			else
			{
				/* write help */
				if (0==usage_indentation_level)
				{
					display_message(INFORMATION_MESSAGE,"Usage :");
					token=state->tokens;
					for (i=state->current_index;i>0;i--)
					{
						display_message(INFORMATION_MESSAGE," %s",*token);
						token++;
					}
					display_message(INFORMATION_MESSAGE," %s",*token);
				}
				if (!((multiple_options>0)&&(exclusive_option>1)))
				{
					display_message(INFORMATION_MESSAGE,"\n");
				}
				usage_indentation_level += 2;
				if (strcmp(PARSER_HELP_STRING,current_token)||(multiple_options>0))
				{
					/* recursive help */
					first=1;
					while ((entry->option)||((entry->user_data)&&!(entry->modifier)))
					{
						if (entry->option)
						{
							if (entry->modifier)
							{
								if (multiple_options>0)
								{
									if (exclusive_option>1)
									{
										if (first)
										{
											display_message(INFORMATION_MESSAGE,"\n%*s(%s",
												usage_indentation_level," ",entry->option);
										}
										else
										{
											display_message(INFORMATION_MESSAGE,"|%s",entry->option);
										}
									}
									else
									{
										display_message(INFORMATION_MESSAGE,"%*s<%s",
											usage_indentation_level," ",entry->option);
									}
								}
								else
								{
									display_message(INFORMATION_MESSAGE,"%*s%s",
										usage_indentation_level," ",entry->option);
								}
								usage_newline=1;
								(entry->modifier)(state,entry->to_be_modified,entry->user_data);
								if (multiple_options>0)
								{
									if (exclusive_option<=1)
									{
										display_message(INFORMATION_MESSAGE,">");
										if (usage_newline)
										{
											display_message(INFORMATION_MESSAGE,"\n");
											usage_newline=0;
										}
									}
								}
								else
								{
									if (usage_newline)
									{
										display_message(INFORMATION_MESSAGE,"\n");
										usage_newline=0;
									}
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"process_option.  Missing modifier: %s",entry->option);
								display_parse_state_location(state);
							}
						}
						else
						{
							/* assume that the user_data is another option table */
							sub_entry=(struct Modifier_entry *)(entry->user_data);
							display_message(INFORMATION_MESSAGE,"%*s",
								usage_indentation_level," ");
							if (0<multiple_options)
							{
								display_message(INFORMATION_MESSAGE,"<");
							}
							number_of_sub_entries=0;
							while (sub_entry->option)
							{
								number_of_sub_entries++;
								if (sub_entry->modifier)
								{
									if (1<number_of_sub_entries)
									{
										display_message(INFORMATION_MESSAGE,"|");
									}
									display_message(INFORMATION_MESSAGE,"%s",sub_entry->option);
									(sub_entry->modifier)(state,sub_entry->to_be_modified,
										sub_entry->user_data);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"process_option.  Missing modifier: %s",sub_entry->option);
									display_parse_state_location(state);
								}
								sub_entry++;
							}
							if (0<multiple_options)
							{
								display_message(INFORMATION_MESSAGE,">");
							}
							display_message(INFORMATION_MESSAGE,"\n");
							usage_newline=0;
						}
						first=0;
						entry++;
					}
					/* write help for default modifier it it exists */
					if (entry->modifier)
					{
						if (multiple_options>0)
						{
							if (exclusive_option>1)
							{
								if (first)
								{
									display_message(INFORMATION_MESSAGE,"\n%*s(",
										usage_indentation_level," ");
								}
								else
								{
									display_message(INFORMATION_MESSAGE,"|");
								}
							}
							else
							{
								display_message(INFORMATION_MESSAGE,"%*s<",
									usage_indentation_level," ");
							}
						}
						else
						{
							display_message(INFORMATION_MESSAGE,"%*s",usage_indentation_level,
								" ");
						}
						usage_newline=1;
						(entry->modifier)(state,entry->to_be_modified,entry->user_data);
						if (multiple_options>0)
						{
							if (exclusive_option<=1)
							{
								display_message(INFORMATION_MESSAGE,">");
								if (usage_newline)
								{
									display_message(INFORMATION_MESSAGE,"\n");
									usage_newline=0;
								}
							}
							else
							{
								display_message(INFORMATION_MESSAGE,")");
							}
						}
						else
						{
							if (usage_newline)
							{
								display_message(INFORMATION_MESSAGE,"\n");
								usage_newline=0;
							}
						}
					}
				}
				else
				{
					/* one level of help */
						/*???DB.  Have added  multiple_options>0  to then so won't come
							here, but haven't stripped out yet */
					while ((entry->option)||((entry->user_data)&&!(entry->modifier)))
					{
						if (entry->option)
						{
							if (multiple_options>0)
							{
								display_message(INFORMATION_MESSAGE,"%*s<%s>\n",
									usage_indentation_level," ",entry->option);
							}
							else
							{
								display_message(INFORMATION_MESSAGE,"%*s%s\n",
									usage_indentation_level," ",entry->option);
							}
						}
						else
						{
							/* assume that the user_data is another option table */
							sub_entry=(struct Modifier_entry *)(entry->user_data);
							if (sub_entry->option)
							{
								if (multiple_options>0)
								{
									display_message(INFORMATION_MESSAGE,"%*s<%s",
										usage_indentation_level," ",sub_entry->option);
								}
								else
								{
									display_message(INFORMATION_MESSAGE,"%*s(%s",
										usage_indentation_level," ",sub_entry->option);
								}
								sub_entry++;
								while (sub_entry->option)
								{
									display_message(INFORMATION_MESSAGE,"|%s",sub_entry->option);
									sub_entry++;
								}
								if (multiple_options>0)
								{
									display_message(INFORMATION_MESSAGE,">");
								}
								else
								{
									display_message(INFORMATION_MESSAGE,")");
								}
								display_message(INFORMATION_MESSAGE,"\n");
								usage_newline=0;
							}
						}
						entry++;
					}
					/* write help for default modifier if it exists */
					if (entry->modifier)
					{
						if (multiple_options>0)
						{
							display_message(INFORMATION_MESSAGE,"%*s<",
								usage_indentation_level," ");
						}
						else
						{
							display_message(INFORMATION_MESSAGE,"%*s",usage_indentation_level,
								" ");
						}
						(entry->modifier)(state,entry->to_be_modified,entry->user_data);
						if (multiple_options>0)
						{
							display_message(INFORMATION_MESSAGE,">");
						}
						display_message(INFORMATION_MESSAGE,"\n");
					}
				}
				usage_indentation_level -= 2;
				/* so that process_option is only called once in parsing loop */
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing token");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"process_option.  Invalid argument(s)");
		return_code=0;
	}
	exclusive_option--;
	LEAVE;

	return (return_code);
} /* process_option */

int process_multiple_options(struct Parse_state *state,
	struct Modifier_entry *modifier_table)
/*******************************************************************************
LAST MODIFIED : 4 October 1996

DESCRIPTION :
==============================================================================*/
{
	int local_exclusive_option,return_code;

	ENTER(process_multiple_options);
	if (state&&modifier_table)
	{
		multiple_options++;
		local_exclusive_option=exclusive_option;
		exclusive_option=0;
		return_code=1;
		while ((state->current_token)&&(return_code=process_option(state,
			(void *)modifier_table)));
		multiple_options--;
		exclusive_option=local_exclusive_option;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"process_multiple_options.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* process_multiple_options */

struct Option_table *CREATE(Option_table)(void)
/*******************************************************************************
LAST MODIFIED : 23 December 1999

DESCRIPTION :
Creates an Option_table for text parsing.
==============================================================================*/
{
	struct Option_table *option_table;

	ENTER(CREATE(Option_table));
	if (ALLOCATE(option_table,struct Option_table,1))
	{
		option_table->allocated_entries = 0;
		option_table->number_of_entries = 0;
		option_table->entry = (struct Modifier_entry *)NULL;
		/* flag indicating all options successfully added */
		option_table->valid = 1;
		/* store suboption_tables added to table for destroying with option_table */
		option_table->number_of_suboption_tables = 0;
		option_table->suboption_tables = (struct Option_table **)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Option_table).  Not enough memory");
		if (option_table)
		{
			DEALLOCATE(option_table);
		}
	}
	LEAVE;

	return (option_table);
} /* CREATE(Option_table) */

int DESTROY(Option_table)(struct Option_table **option_table_address)
/*******************************************************************************
LAST MODIFIED : 23 December 1999

DESCRIPTION :
==============================================================================*/
{
	int i,return_code;
	struct Option_table *option_table;

	ENTER(DESTROY(Option_table));
	if (option_table_address)
	{
		return_code=1;
		if (option_table = *option_table_address)
		{
			/* clean up suboption_tables added to option_table */
			if (option_table->suboption_tables)
			{
				for (i=0;i<option_table->number_of_suboption_tables;i++)
				{
					DESTROY(Option_table)(&(option_table->suboption_tables[i]));
				}
				DEALLOCATE(option_table->suboption_tables);
			}
			if (option_table->entry)
			{
				DEALLOCATE(option_table->entry);
			}
			DEALLOCATE(*option_table_address);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(Option_table).  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Option_table) */

static int Option_table_add_entry_private(struct Option_table *option_table,
	char *token,void *to_be_modified,void *user_data,modifier_function modifier)
/*******************************************************************************
LAST MODIFIED : 23 December 1999

DESCRIPTION :
Adds the given entry to the option table, enlarging the table as needed.
If fails, marks the option_table as invalid.
==============================================================================*/
{
	int i, return_code;
	struct Modifier_entry *temp_entry;

	ENTER(Option_table_add_entry_private);
	if (option_table)
	{
		return_code=1;
		if (token)
		{
			i=0;
			while (return_code && (i<option_table->number_of_entries))
			{
				if (option_table->entry[i].option
					&& (!strcmp(token, option_table->entry[i].option)))
				{
					display_message(ERROR_MESSAGE,
						"Option_table_add_entry_private.  Token '%s' already in option table",
						token);
					return_code=0;				
				}
				i++;
			}
		}
		if (option_table->number_of_entries == option_table->allocated_entries)
		{
			if (REALLOCATE(temp_entry,option_table->entry,struct Modifier_entry,
				option_table->allocated_entries+OPTION_TABLE_ALLOCATE_SIZE))
			{
				option_table->entry = temp_entry;
				option_table->allocated_entries += OPTION_TABLE_ALLOCATE_SIZE;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Option_table_add_entry_private.  Not enough memory");
				return_code=0;
				option_table->valid=0;
			}
		}
		if (return_code)
		{
			temp_entry = &(option_table->entry[option_table->number_of_entries]);
			temp_entry->option=token;
			temp_entry->to_be_modified=to_be_modified;
			temp_entry->user_data=user_data;
			temp_entry->modifier=modifier;
			option_table->number_of_entries++;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_entry_private.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Option_table_add_entry_private */

int Option_table_add_entry(struct Option_table *option_table,char *token,
	void *to_be_modified,void *user_data,modifier_function modifier)
/*******************************************************************************
LAST MODIFIED : 23 December 1999

DESCRIPTION :
Adds the given <token> etc. to the option table, enlarging the table as needed.
If fails, marks the option_table as invalid.
==============================================================================*/
{
	int return_code;

	ENTER(Option_table_add_entry);
	if (option_table)
	{
		if (!(return_code=Option_table_add_entry_private(option_table,token,
			to_be_modified,user_data,modifier)))
		{
			display_message(ERROR_MESSAGE,
				"Option_table_add_entry.  Could not add option");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_entry.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Option_table_add_entry */

int Option_table_add_suboption_table(struct Option_table *option_table,
	struct Option_table *suboption_table)
/*******************************************************************************
LAST MODIFIED : 8 November 2000

DESCRIPTION :
Checks that <suboption_table> is valid, and if so, adds it to <option_table>.
On calling this function, <suboption_table> is owned by <option_table> and the
latter is responsible for destroying it. It will be destroyed immediately if it
is invalid or cannot be added to list of entries.
Mechanism currently used to handle enumerated options, though it does not insist
that only one valid enumerator is entered.
If fails, marks the option_table as invalid.
Note must not make any further changes to suboption_table after it is made part
of option_table!
==============================================================================*/
{
	int return_code;
	struct Option_table **temp_suboption_tables;

	ENTER(Option_table_add_suboption_table);
	if (option_table&&suboption_table)
	{
		/* add blank entry needed for process_option */
		Option_table_add_entry_private(suboption_table,(char *)NULL,(void *)NULL,
			(void *)NULL,(modifier_function)NULL);
		if (suboption_table->valid)
		{
			if (REALLOCATE(temp_suboption_tables,option_table->suboption_tables,
				struct Option_table *,option_table->number_of_suboption_tables+1))
			{
				option_table->suboption_tables=temp_suboption_tables;
				option_table->suboption_tables[option_table->number_of_suboption_tables]
					=suboption_table;
				option_table->number_of_suboption_tables++;
				if (!(return_code=Option_table_add_entry_private(option_table,
					(char *)NULL,(void *)NULL,suboption_table->entry,
					(modifier_function)NULL)))
				{
					display_message(ERROR_MESSAGE,
						"Option_table_add_entry.  Could not add option");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Option_table_add_suboption_table.  Not enough memory");
				return_code=0;
				option_table->valid=0;
				DESTROY(Option_table)(&suboption_table);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Option_table_parse.  Invalid suboption_table");
			option_table->valid=0;
			DESTROY(Option_table)(&suboption_table);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_suboption_table.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Option_table_add_suboption_table */

int Option_table_add_switch(struct Option_table *option_table,
	char *on_string,char *off_string,int *value_address)
/*******************************************************************************
LAST MODIFIED : 12 May 2000

DESCRIPTION :
Adds a newly created suboption table containing 2 items:
an <on_string> token that invokes set_int_switch with <value_address>;
an <off_string> token that invokes unset_int_switch with <value_address>;
The <on_string> and <off_string> should be static, eg. passed in quotes.
==============================================================================*/
{
	int return_code;
	struct Option_table *suboption_table;

	ENTER(Option_table_add_switch);
	if (option_table&&on_string&&off_string&&value_address)
	{
		if (option_table->valid)
		{
			if (suboption_table=CREATE(Option_table)())
			{
				Option_table_add_entry(suboption_table,on_string,
					value_address,on_string,set_int_switch);
				Option_table_add_entry(suboption_table,off_string,
					value_address,off_string,unset_int_switch);
				return_code=
					Option_table_add_suboption_table(option_table,suboption_table);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Option_table_add_switch.  Not enough memory");
				return_code=0;
				option_table->valid=0;
			}
		}
		else
		{
			/* report no further errors */
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_switch.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Option_table_add_switch */

int set_enumerator_string(struct Parse_state *state,
	void *enumerator_string_address_void,void *enumerator_string_value_void)
/*******************************************************************************
LAST MODIFIED : 21 December 1999

DESCRIPTION :
A modifier function for setting an enumerated type variable to a specified
value.
==============================================================================*/
{
	char *current_token,**enumerator_string_address,*enumerator_string_value;
	int return_code;

	ENTER(set_enumerator_string);
	if (state&&(enumerator_string_address=(char **)enumerator_string_address_void)
		&&(enumerator_string_value=(char *)enumerator_string_value_void))
	{
		return_code=1;
		if (!(current_token=state->current_token)||
			(strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token)))
		{
			*enumerator_string_address = enumerator_string_value;
		}
		else
		{
			if (*enumerator_string_address == enumerator_string_value)
			{
				display_message(INFORMATION_MESSAGE,"[%s]",enumerator_string_value);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_enumerator_string.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_enumerator_string */

int Option_table_add_enumerator(struct Option_table *option_table,
	int number_of_valid_strings,char **valid_strings,
	char **enumerator_string_address)
/*******************************************************************************
LAST MODIFIED : 20 December 1999

DESCRIPTION :
Adds a newly created suboption table for all the valid_strings for the
enumerator. The <valid_strings> array should contain <number_of_valid_strings>
pointers to static strings, one per enumerator option. Responsibility for
deallocating this array is left to the calling function. The static string value
of the enumerator is maintained in <enumerator_string_address> and it is up to
the calling function to convert back to an enumerated value.
Note that if any error occurs, the option_table is marked as being invalid and
no further errors will be reported on subsequent calls.
==============================================================================*/
{
	int i,return_code;
	struct Option_table *suboption_table;

	ENTER(Option_table_add_enumerator);
	if (option_table&&(0<number_of_valid_strings)&&valid_strings&&
		enumerator_string_address)
	{
		if (option_table->valid)
		{
			if (suboption_table=CREATE(Option_table)())
			{
				for (i=0;i<number_of_valid_strings;i++)
				{
					Option_table_add_entry(suboption_table,valid_strings[i],
						enumerator_string_address,valid_strings[i],set_enumerator_string);
				}
				return_code=
					Option_table_add_suboption_table(option_table,suboption_table);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Option_table_add_enumerator.  Not enough memory");
				return_code=0;
				option_table->valid=0;
			}
		}
		else
		{
			/* report no further errors */
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_enumerator.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Option_table_add_enumerator */

int Option_table_parse(struct Option_table *option_table,
	struct Parse_state *state)
/*******************************************************************************
LAST MODIFIED : 23 December 1999

DESCRIPTION :
Parses the options in the <option_table>, giving only one option a chance to be
entered.
==============================================================================*/
{
	int return_code;

	ENTER(Option_table_parse);
	if (option_table&&state)
	{
		/* add blank entry needed for process_option */
		Option_table_add_entry_private(option_table,(char *)NULL,(void *)NULL,
			(void *)NULL,(modifier_function)NULL);
		if (option_table->valid)
		{
			return_code=process_option(state,option_table->entry);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Option_table_parse.  Invalid option table");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Option_table_parse.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Option_table_parse */

int Option_table_multi_parse(struct Option_table *option_table,
	struct Parse_state *state)
/*******************************************************************************
LAST MODIFIED : 23 December 1999

DESCRIPTION :
Parses the options in the <option_table>, giving all options a chance to be
entered.
==============================================================================*/
{
	int return_code;

	ENTER(Option_table_multi_parse);
	if (option_table&&state)
	{
		/* add blank entry needed for process_option */
		Option_table_add_entry_private(option_table,(char *)NULL,(void *)NULL,
			(void *)NULL,(modifier_function)NULL);
		if (option_table->valid)
		{
			return_code=process_multiple_options(state,option_table->entry);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Option_table_multi_parse.  Invalid option table");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_multi_parse.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Option_table_multi_parse */

static int extract_token(char **source_address,char **token_address)
/*******************************************************************************
LAST MODIFIED : 18 September 2000

DESCRIPTION :
On successful return, <*token_address> will point to a newly-allocated string
containing the first token in the string at <*source_address>. <source_address>
is then updated to point to the next character after the last one used in
creating the token.
The function skips any leading whitespace and stops at the first token delimiter
(whitespace/=/,/;), comment character (!/#) or end of string. Tokens containing
any of the above special characters may be produced by enclosing them in single
or double quotes - but not a mixture of them.
To allow quote marks to be put in the final string, the function interprets
\\, \" and \' as \, " and ', respectively.
Note that the quote mark if used must mark exactly the beginning and end of the
string; the string is not permitted to end with a NULL character or with a
non-delimiting character after the end-quote.
To minimise memory allocation, the function uses the string at <*source_address>
as working space in which the token is constructed from the source string.
==============================================================================*/
{
	char character,quote_mark,*token,*destination,*source;
	int return_code,token_length;

	ENTER(extract_token);
	if (source_address && *source_address && token_address)
	{
		return_code=1;
		destination = source = *source_address;
		/* pass over leading white space and other delimiters */
		while ((*source) && (isspace(*source) ||
			('=' == *source) || (',' == *source) || (';' == *source)))
		{
			source++;
		}
		if (*source)
		{
			if (('\''== *source) || ('\"'== *source))
			{
				quote_mark= *source;
				/* read token until final quote_mark or end of string, ignoring quote
					 marks after a backslash/escape character */
				source++;
				while ((quote_mark != *source) && ('\0' != *source))
				{
					/* replace \\, \" and \' by \, " and ' in token */
					if (('\\' == *source) && (('\\' == *(source+1)) ||
						('\"' == *(source+1)) || ('\'' == *(source+1))))
					{
						source++;
					}
					*destination = *source;
					destination++;
					source++;
				}
				if (quote_mark == *source)
				{
					source++;
					/* ensure there is a valid delimiter after end quote */
					if (('\0' != *source) && !isspace(*source) &&
						('=' != *source) && (',' != *source) &&
						(';' != *source) && ('#' != *source))
					{
						/* string missing delimiter after quote; report error */
						display_message(ERROR_MESSAGE,
							"Token missing delimiter after final quote (%c)",quote_mark);
						return_code=0;
					}
				}
				else
				{
					/* string ended without final quote; report error */
					display_message(ERROR_MESSAGE,
						"Token missing final quote (%c)",quote_mark);
					return_code=0;
				}
			}
			else
			{
				while (('\0' != (character = *source)) && (!isspace(character)) &&
					('=' != character) && (',' != character) &&
					(';' != character) && ('#' != character))
				{
					*destination = *source;
					destination++;
					source++;
				}
			}
		}
		if (return_code)
		{
			if (0<(token_length= destination - *source_address))
			{
				if (ALLOCATE(token,char,token_length+1))
				{
					strncpy(token,*source_address,token_length);
					token[token_length]='\0';
					*token_address = token;
					*source_address = source;
				}
				else
				{
					display_message(ERROR_MESSAGE,"extract_token.  Not enough memory");
					return_code=0;
				}
			}
			else
			{
				/* source string empty or only contained whitespace/delimiters */
				*token_address = (char *)NULL;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"extract_token.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* extract_token */

struct Parse_state *create_Parse_state(char *command_string)
/*******************************************************************************
LAST MODIFIED : 24 November 1998

DESCRIPTION :
Creates a Parse_state structure which contains
- a trimmed copy of the <command_string>
- the <command_string> split into tokens
NB
1 ! and # indicate that the rest of the command string is a comment (not split
	into tokens;
2 tokens containing normal token separator commands may be entered by surround-
  ing them with single '' or double "" quotes - useful for entering text. Paired
	quotes in such strings are read as a quote mark in the final token;
3 Variables are converted into values;
==============================================================================*/
{
	char *next_token,**temp_tokens,**tokens,*token_source,*working_string;
	int allocated_tokens,i,number_of_tokens,return_code,still_tokenising;
	struct Parse_state *state;

	ENTER(create_Parse_state);
	if (command_string)
	{
		if (ALLOCATE(state,struct Parse_state,1))
		{
			/*???RC trim_string not used as trailing whitespace may be in a quote */
			if (ALLOCATE(working_string,char,strlen(command_string)+1))
			{
				/* Replace the %z1% variables and $variables in the working_string */
				strcpy(working_string,command_string);
#if ! defined (PERL_INTERPRETER)
				parse_variable(&working_string);
#endif /* ! defined (PERL_INTERPRETER) */
				if (ALLOCATE(state->command_string,char,strlen(working_string)+1))
				{
					strcpy(state->command_string,working_string);
					token_source=working_string;
					tokens=(char **)NULL;
					allocated_tokens=0;
					number_of_tokens=0;
					return_code=1;
					still_tokenising=1;
					while (still_tokenising)
					{
						if (extract_token(&token_source,&next_token))
						{
							if (next_token)
							{
								if (number_of_tokens == allocated_tokens)
								{
									if (REALLOCATE(temp_tokens,tokens,char *,allocated_tokens+10))
									{
										tokens=temp_tokens;
										allocated_tokens += 10;
									}
									else
									{
										return_code=0;
									}
								}
								if (return_code)
								{
									tokens[number_of_tokens]=next_token;
									number_of_tokens++;
								}
								else
								{
									DEALLOCATE(next_token);
									still_tokenising=0;
								}
							}
							else
							{
								/* successful end of tokenising */
								still_tokenising=0;
							}
						}
						else
						{
							/* tokenising failed */
							return_code=still_tokenising=0;
						}
					}
					if (return_code)
					{
						state->tokens=tokens;
						state->number_of_tokens=number_of_tokens;
						state->current_index=0;
						if (tokens)
						{
							state->current_token=tokens[0];
						}
						else
						{
							state->current_token=(char *)NULL;
						}
					}
					else
					{
						/* clean up any memory allocated for tokens */
						if (tokens)
						{
							for (i=0;i<number_of_tokens;i++)
							{
								DEALLOCATE(tokens[i]);
							}
							DEALLOCATE(tokens);
						}
						DEALLOCATE(state->command_string);
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
			DEALLOCATE(working_string);
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
					"create_Parse_state.  Error filling parse state");
				DEALLOCATE(state);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_Parse_state.  Insufficient memory for parse state");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_Parse_state.  Missing command string");
		state=(struct Parse_state *)NULL;
	}
	LEAVE;

	return (state);
} /* create_Parse_state */

int destroy_Parse_state(struct Parse_state **state_address)
/*******************************************************************************
LAST MODIFIED : 12 June 1996

DESCRIPTION :
==============================================================================*/
{
	char **token;
	int number_of_tokens,return_code;
	struct Parse_state *state;

	ENTER(destroy_Parse_state);
	if (state_address)
	{
		if (state= *state_address)
		{
			if ((number_of_tokens=state->number_of_tokens)>0)
			{
				token=state->tokens;
				while (number_of_tokens>0)
				{
					DEALLOCATE(*token);
					token++;
					number_of_tokens--;
				}
				DEALLOCATE(state->tokens);
			}
			DEALLOCATE(state->command_string);
			DEALLOCATE(*state_address);
			return_code=1;
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"destroy_Parse_state.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* destroy_Parse_state */

int Parse_state_help_mode(struct Parse_state *state)
/*******************************************************************************
LAST MODIFIED : 12 May 2000

DESCRIPTION :
Returns 1 if the current_token in <state> is either of
PARSER_HELP_STRING or PARSER_RECURSIVE_HELP_STRING.
==============================================================================*/
{
	char *current_token;
	int return_code;

	ENTER(Parse_state_help_mode);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				/* non-help token */
				return_code=0;
			}
			else
			{
				/* help token */
				return_code=1;
			}
		}
		else
		{
			/* no token */
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Parse_state_help_mode.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Parse_state_help_mode */

int shift_Parse_state(struct Parse_state *state,int shift)
/*******************************************************************************
LAST MODIFIED : 12 June 1996

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(shift_Parse_state);
	if (state&&(shift>0))
	{
		state->current_index += shift;
		if (state->current_index<=state->number_of_tokens)
		{
			if (state->current_index==state->number_of_tokens)
			{
				state->current_token=(char *)NULL;
			}
			else
			{
				state->current_token=state->tokens[state->current_index];
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"shift_Parse_state.  Cannot shift beyond end of token list");
			state->current_token=(char *)NULL;
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"shift_Parse_state.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* shift_Parse_state */

int display_parse_state_location(struct Parse_state *state)
/*******************************************************************************
LAST MODIFIED : 27 June 1996

DESCRIPTION :
Shows the current location in the parse <state>.
==============================================================================*/
{
	char **token;
	int i,return_code;

	ENTER(display_parse_state_location);
	if (state)
	{
		if (state->current_token)
		{
			i=state->current_index;
		}
		else
		{
			i=state->number_of_tokens;
		}
		token=state->tokens;
		while (i>0)
		{
			display_message(INFORMATION_MESSAGE,"%s ",*token);
			token++;
			i--;
		}
		display_message(INFORMATION_MESSAGE,"*\n");
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"display_parse_state_location.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* display_parse_state_location */

int Parse_state_append_to_command_string(struct Parse_state *state,
	char *addition)
/*******************************************************************************
LAST MODIFIED : 29 October 1999

DESCRIPTION :
Appends the <addition> string to the end of the current command_string stored in
the <state>.  Useful for changing the kept history echoed to the command window.
==============================================================================*/
{
	char *new_command_string;
	int return_code;

	ENTER(Parse_state_append_to_command_string);
	if (state && addition)
	{
		if (REALLOCATE(new_command_string, state->command_string,
			char, strlen(state->command_string) + strlen(addition) + 1))
		{
			strcat(new_command_string, addition);
			state->command_string = new_command_string;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Parse_state_append_to_command_string.  "
				"Unable to reallocate command string");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Parse_state_append_to_command_string.  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Parse_state_append_to_command_string */

int parse_variable(char **token)
/*******************************************************************************
LAST MODIFIED : 10 March 2000

DESCRIPTION :
Replaces occurrences of %<f/i/z/l><nnn>% with the value of that variable.  May
be called recursively.  Reallocates <*token>, so there is no problem with
over-writing.
Also replaces $LFX with the string representing LFX in the Assign_variable list
if it exists.
==============================================================================*/
{
	char *begin,*end,*index,*new_token,temp_string[100],*var_name;
	int length,temp_int,number,return_code,temp_string_offset;
	struct Assign_variable *variable;

	ENTER(parse_variable);
	/* check argument */
	if (token&&(*token))
	{
		return_code=1;
		/* try to find a % */
		if (begin=strchr(*token,'%'))
		{
			/* look for another % */
			if (end=strchr(begin+1,'%'))
			{
				length=end-begin+1;
				/* find the variable number (starting after the format) */
				if (1==sscanf(begin+2,"%i",&number))
				{
					if ((number>=0)&&(number<MAX_VARIABLES))
					{
						switch (begin[1])
						{
							case 'f':
							{
								sprintf(temp_string,"%f",variable_float[number]);
								temp_string_offset=0;
							} break;
							case 'i':
							{
								/* write it as an integer */
								temp_int=(int)variable_float[number];
								sprintf(temp_string,"%i",temp_int);
								temp_string_offset=0;
							} break;
							case 'z':
							{
								/* leading zeros and the width of the existing field */
								temp_int=(int)variable_float[number];
								sprintf(temp_string,"%0*i",length,temp_int);
								temp_string_offset=strlen(temp_string)-length;
							} break;
							case 'l':
							{
								/* write it as a logical - check for <0.1 so that close to
									zero=false */
								if (fabs(variable_float[number])<0.1)
								{
									strcpy(temp_string,"off");
								}
								else
								{
									strcpy(temp_string,"on");
								}
								temp_string_offset=0;
							} break;
							default:
							{
								display_message(ERROR_MESSAGE,
									"Variable format not known: %c : %s",begin[1],*token);
								return_code=0;
							} break;
						}
						if (return_code)
						{
							if (ALLOCATE(new_token,char,(begin-(*token))+
								strlen(temp_string+temp_string_offset)+strlen(end+1)+1))
							{
								strncpy(new_token,*token,begin-(*token));
								new_token[begin-(*token)]='\0';
								strcat(new_token,temp_string+temp_string_offset);
								strcat(new_token,end+1);
								DEALLOCATE(*token);
								*token=new_token;
								/* see if there are any more to parse */
								return_code=parse_variable(token);
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"parse_variable.  Could not allocate new token");
								return_code=0;
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Variable number out of range: %d : %s",number,*token);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"Variable number not found: %s",*token);
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"Two percentage signs required: %s",
					*token);
				return_code=0;
			}
		}
		/* Separately parse for $assign_variables */
		/* try to find a $ */
		while (return_code && (begin=strchr(*token,'$')))
		{
			if ((begin == *token)
				|| ((begin > *token + 1) && (!strncmp(begin - 1, " ", 1)))
				|| ((begin > *token + 2) && (!strncmp(begin - 2, "//", 2))))
			{
				begin++;
				/* look for the end of this token */
				if (!(end=strpbrk(begin,"/ ")))
				{
					end = begin + strlen(begin);
				}
				if ((*end == 0) || (!(strncmp(end, " ", 1))) || (!(strncmp(end, "//", 2))))
				{
					if (ALLOCATE(var_name, char, end - begin))
					{
						strncpy(var_name, begin, end - begin);
						var_name[end - begin] = 0;
						if (variable = FIND_BY_IDENTIFIER_IN_LIST(Assign_variable, name)
							(var_name, assign_variable_list))
						{
							if (ALLOCATE(new_token, char, strlen(variable->value) +
								strlen(*token) - (end - begin)))
							{
								index = new_token;
								if ((begin > *token + 2) && (!strncmp(begin - 2, " ", 1)))
								{
									strncpy(index, *token, begin - *token - 1);
									index += begin - *token - 1;
								}
								else if ((begin > *token + 3) && (!strncmp(begin - 3, "//", 2)))
								{
									strncpy(index, *token, begin - *token - 3);
									index += begin - *token - 3;
								}
								if (strlen(variable->value) > 0)
								{
									strncpy(index, variable->value, strlen(variable->value));
									index += strlen(variable->value);
								}
								if (!(strncmp(end, " ", 1)))
								{
									strcpy(index, end);
									index += strlen(end);
								}
								else if (!(strncmp(end, "//", 2)))
								{
									strcpy(index, end + 2);
									index += strlen(end + 2);
								}
								*index = 0;
#if defined (DEBUG)
								display_message(INFORMATION_MESSAGE,
									"parse_variable.\n\tOld token %s\n\tVariable value %s\n\tNew token %s\n", *token, variable->value, new_token);
#endif /* defined (DEBUG) */
								DEALLOCATE(*token);
								*token = new_token;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"parse_variable.  Variable \"%s\" not found.", var_name);
							return_code=0;
						}
						DEALLOCATE(var_name);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"parse_variable.  Unable to allocate variable name string");
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"parse_variable.  Concatenation token operator \"//\" required between variable and plain text.");
					return_code=0;				
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"parse_variable.  Concatenation token operator \"//\" required between variable and plain text.");
				return_code=0;				
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"parse_variable.  Missing token");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* parse_variable */

int execute_variable_command(struct Parse_state *state,
	void *dummy_to_be_modified,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 21 June 1999

DESCRIPTION :
Executes a VARIABLE command.
==============================================================================*/
{
	static enum Variable_operation_type add=ADD_VARIABLE_OPERATION,
		divide=DIVIDE_VARIABLE_OPERATION,multiply=MULTIPLY_VARIABLE_OPERATION,
		set=SET_VARIABLE_OPERATION,subtract=SUBTRACT_VARIABLE_OPERATION;
	int return_code;
	static struct Modifier_entry option_table[]=
	{
		{"add",&add,(void *)NULL,execute_variable_command_operation},
		{"divide",&divide,(void *)NULL,execute_variable_command_operation},
		{"multiply",&multiply,(void *)NULL,execute_variable_command_operation},
		{"set",&set,(void *)NULL,execute_variable_command_operation},
		{"show",NULL,(void *)NULL,execute_variable_command_show},
		{"subtract",&subtract,(void *)NULL,execute_variable_command_operation},
		{NULL,NULL,NULL}
	};

	ENTER(execute_variable_command);
	USE_PARAMETER(dummy_to_be_modified);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		if (state->current_token)
		{
			return_code=process_option(state,(void *)option_table);
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_variable_command.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_variable_command */

int execute_assign_variable(struct Parse_state *state,
	void *dummy_to_be_modified,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 10 March 2000

DESCRIPTION :
Executes an ASSIGN VARIABLE command.  Does a very small subset of the intended
use of this command.
==============================================================================*/
{
	char *begin, *begin2, *current_token, *end, *env_string, *var_name;
	int return_code;
	struct Assign_variable *variable;
	
	ENTER(execute_assign_variable);
	USE_PARAMETER(dummy_to_be_modified);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				return_code = 1;
				if (!(assign_variable_list) || 
					!(variable = FIND_BY_IDENTIFIER_IN_LIST(Assign_variable, name)
					(current_token, assign_variable_list)))
				{
					if (!(variable = CREATE(Assign_variable)(current_token)))
					{
						display_message(ERROR_MESSAGE,
							"execute_assign_variable.  Unable to find or create variable %s",
							current_token);
						return_code = 0;
					}
				}
				if (return_code && variable)
				{
					shift_Parse_state(state,1);
					if (current_token=state->current_token)
					{
						if (strcmp(PARSER_HELP_STRING,current_token)&&
							strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
						{
							/* Implement special case getenv command */
							if (!strncmp(current_token, "getenv", 6))
							{
								if (begin=strchr(current_token,'('))
								{
									if (begin2=strchr(begin,'"'))
									{
										begin = begin2;
										if (!(end = strchr(begin + 1,'"')))
										{
											display_message(ERROR_MESSAGE,
												"execute_assign_variable.  Closing \" missing.",
												state->current_token);
											return_code = 0;
										}
									}
									else
									{
										if (!(end = strchr(begin + 1,')')))
										{
											display_message(ERROR_MESSAGE,
												"execute_assign_variable.  Closing ) missing.",
												state->current_token);
											return_code = 0;
										}										
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"execute_assign_variable.  Bracket missing after funciton getenv",
										state->current_token);
									return_code = 0;
								}
								if (return_code)
								{
									if (ALLOCATE(var_name, char, end - begin))
									{
										strncpy(var_name, begin + 1, end - begin - 1);
										var_name[end - begin - 1] = 0;
										if (env_string = getenv(var_name))
										{
											return_code = 
												Assign_variable_set_value(variable,
													env_string);
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"execute_assign_variable.  Environment variable %s not found",
												var_name);
											return_code = 0;
										}
										DEALLOCATE(var_name);
									}
								}
							}
							else
							{
								return_code = 
									Assign_variable_set_value(variable, current_token);
							}
						}
						else
						{
							display_message(INFORMATION_MESSAGE,
								"\n           value");
							return_code = 1;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"execute_assign_variable.  Specify new value",
							state->current_token);
						return_code = 0;						
					}
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE,
					"\n         VARIABLE_NAME value");
				return_code = 1;
			}
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_assign_variable.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_assign_variable */

int destroy_assign_variable_list(void)
/*******************************************************************************
LAST MODIFIED : 10 March 2000

DESCRIPTION :
Clean up the global assign_variable_list.
==============================================================================*/
{
	int return_code;
	
	ENTER(destroy_assign_variable_list);

	if (assign_variable_list)
	{
		return_code = DESTROY_LIST(Assign_variable)(&assign_variable_list);
		assign_variable_list = (struct LIST(Assign_variable) *)NULL;
	}
	else
	{
		return_code = 1;
	}

	LEAVE;

	return (return_code);
} /* destroy_assign_variable_list */

int set_name(struct Parse_state *state,void *name_address_void,
	void *prefix_space)
/*******************************************************************************
LAST MODIFIED : 27 May 1997

DESCRIPTION :
Allocates memory for a name, then copies the passed string into it.
==============================================================================*/
{
	char *current_token,**name_address;
	int return_code;

	ENTER(set_name);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (name_address=(char **)name_address_void)
				{
					if (*name_address)
					{
						DEALLOCATE(*name_address);
					}
					if (ALLOCATE(*name_address,char,strlen(current_token)+1))
					{
						strcpy(*name_address,current_token);
						return_code=shift_Parse_state(state,1);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"set_name.  Could not allocate memory for name");
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"set_name.  Missing name_address");
					return_code=0;
				}
			}
			else
			{
				if (prefix_space)
				{
					display_message(INFORMATION_MESSAGE," NAME");
				}
				else
				{
					display_message(INFORMATION_MESSAGE,"NAME");
				}
				if ((name_address=(char **)name_address_void)&&(*name_address))
				{
					display_message(INFORMATION_MESSAGE,"[%s]",*name_address);
				}
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_name.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_name */

int set_names(struct Parse_state *state,void *names_void,
	void *number_of_names_address_void)
/*******************************************************************************
LAST MODIFIED : 10 May 2000

DESCRIPTION :
Modifier function for reading number_of_names (>0) string names from
<state>. User data consists of a pointer to an integer containing the
number_of_names, while <names_void> should point to a large enough space to
store the number_of_names pointers. The names in this array must either be NULL
or pointing to allocated strings.
==============================================================================*/
{
	char *current_token,**names;
	int i,number_of_names,return_code;

	ENTER(set_names);
	if (state&&(names=(char **)names_void)&&number_of_names_address_void&&
		(0<(number_of_names=*((int *)number_of_names_address_void))))
	{
		if (current_token=state->current_token)
		{
			return_code=1;
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				for (i=0;return_code&&(i<number_of_names);i++)
				{
					if (current_token=state->current_token)
					{
						if (names[i])
						{
							DEALLOCATE(names[i]);
						}
						if (names[i]=duplicate_string(current_token))
						{
							return_code=shift_Parse_state(state,1);
						}
						else
						{
							display_message(ERROR_MESSAGE,"set_names.  Not enough memory");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Missing name");
						display_parse_state_location(state);
						return_code=0;
					}
				}
			}
			else
			{
				/* write help text */
				for (i=0;i<number_of_names;i++)
				{
					display_message(INFORMATION_MESSAGE," NAME");
				}
				for (i=0;i<number_of_names;i++)
				{
					if (0==i)
					{
						display_message(INFORMATION_MESSAGE,"[",names[0]);
					}
					else
					{
						display_message(INFORMATION_MESSAGE," ",names[i]);
					}
					if (names[i])
					{
						display_message(INFORMATION_MESSAGE,"%s",names[i]);
					}
					else
					{
						display_message(INFORMATION_MESSAGE,"\"\"");
					}
				}
				display_message(INFORMATION_MESSAGE,"]");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing %d names",number_of_names);
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_names.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_names */

int set_int(struct Parse_state *state,void *value_address_void,
	void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 21 June 1999

DESCRIPTION :
A modifier function for setting a int.
==============================================================================*/
{
	char *current_token;
	int return_code,value,*value_address;

	ENTER(set_int);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (value_address=(int *)value_address_void)
				{
					if (1==sscanf(current_token," %d ",&value))
					{
						*value_address=value;
						return_code=shift_Parse_state(state,1);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Invalid integer: %s",current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"set_int.  Missing value_address");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," #");
				if (value_address=(int *)value_address_void)
				{
					display_message(INFORMATION_MESSAGE,"[%d]",*value_address);
				}
				display_message(INFORMATION_MESSAGE,"{integer}");
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing integer");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_int.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_int */

int set_int_optional(struct Parse_state *state,void *value_address_void,
	void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 21 June 1999

DESCRIPTION :
If the next token is an integer then the int is set to that value otherwise the
int is set to 1.
==============================================================================*/
{
	char *current_token;
	int return_code,value,*value_address;

	ENTER(set_int_optional);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		if (value_address=(int *)value_address_void)
		{
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (1==sscanf(current_token," %d ",&value))
					{
						*value_address=value;
						return_code=shift_Parse_state(state,1);
					}
					else
					{
						*value_address=1;
						return_code=1;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE," <#>[1]{integer}");
					return_code=1;
				}
			}
			else
			{
				*value_address=1;
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"set_int_optional.  Missing value_address");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_int_optional.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_int_optional */

int set_int_non_negative(struct Parse_state *state,void *value_address_void,
	void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 21 June 1999

DESCRIPTION :
A modifier function for setting a int to a non-negative value.
==============================================================================*/
{
	char *current_token;
	int return_code,value,*value_address;

	ENTER(set_int_non_negative);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (value_address=(int *)value_address_void)
				{
					if (1==sscanf(current_token," %d ",&value))
					{
						/* make sure that the value value is non-negative */
						if (value>=0)
						{
							*value_address=value;
							return_code=shift_Parse_state(state,1);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Value must be a non-negative integer: %s\n",current_token);
							display_parse_state_location(state);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Invalid non-negative integer: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_int_non_negative.  Missing value_address");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," #");
				if (value_address=(int *)value_address_void)
				{
					display_message(INFORMATION_MESSAGE,"[%d]",*value_address);
				}
				display_message(INFORMATION_MESSAGE,"{>=0,integer}");
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing non_negative integer");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_int_non_negative.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_int_non_negative */

int set_int_positive(struct Parse_state *state,void *value_address_void,
	void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 21 June 1999

DESCRIPTION :
A modifier function for setting a int to a positive value.
==============================================================================*/
{
	char *current_token;
	int return_code,value,*value_address;

	ENTER(set_int_positive);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (value_address=(int *)value_address_void)
				{
					if (1==sscanf(current_token," %d ",&value))
					{
						/* make sure that the value value is positive */
						if (value>0)
						{
							*value_address=value;
							return_code=shift_Parse_state(state,1);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Value must be a positive integer: %s\n",current_token);
							display_parse_state_location(state);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Invalid positive integer: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_int_positive.  Missing value_address");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," #");
				if (value_address=(int *)value_address_void)
				{
					display_message(INFORMATION_MESSAGE,"[%d]",*value_address);
				}
				display_message(INFORMATION_MESSAGE,"{>0,integer}");
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing positive integer");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_int_positive.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_int_positive */

int set_int_and_char_flag(struct Parse_state *state,void *value_address_void,
	void *flag_address_void)
/*******************************************************************************
LAST MODIFIED : 22 December 2000

DESCRIPTION :
A modifier function for setting an int, and a char flag in the user data to
indicate that the int has been set.
==============================================================================*/
{
	char *current_token, *flag_address;
	int value, *value_address;
	int return_code;

	ENTER(set_int_and_char_flag);
	if (state && (value_address = (int *)value_address_void) &&
		(flag_address = (char *)flag_address_void))
	{
		if (current_token = state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (1 == sscanf(current_token, " %d ", &value))
				{
					*value_address = value;
					*flag_address = 1;
					return_code = shift_Parse_state(state, 1);
				}
				else
				{
					display_message(ERROR_MESSAGE, "Invalid int: %s", current_token);
					display_parse_state_location(state);
					return_code = 0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE, " #");
				if (*flag_address)
				{
					display_message(INFORMATION_MESSAGE, "[%d]", *value_address);
				}
				else
				{
					display_message(INFORMATION_MESSAGE, "[NOT SET]");
				}
				return_code = 1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing int");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_int_and_char_flag.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* set_int_and_char_flag */

int set_int_vector(struct Parse_state *state,void *values_address_void,
	void *number_of_components_address_void)
/*******************************************************************************
LAST MODIFIED : 27 March 2000

DESCRIPTION :
Modifier function for reading number_of_components (>0) ints from <state>.
User data consists of a pointer to an integer containing number_of_components,
while <values_address_void> should point to a large enough space to store the
number_of_components ints.
Now prints current contents of the vector with help.
==============================================================================*/
{
	char *current_token;
	int value,*values_address;
	int comp_no,number_of_components,return_code;

	ENTER(set_int_vector);
	if (state)
	{
		if ((values_address=(int *)values_address_void)&&
			number_of_components_address_void&&(0<(number_of_components=
			*((int *)number_of_components_address_void))))
		{
			if (current_token=state->current_token)
			{
				return_code=1;
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					for (comp_no=0;return_code&&(comp_no<number_of_components);comp_no++)
					{
						if (current_token=state->current_token)
						{
							if (1==sscanf(current_token," %d ",&value))
							{
								values_address[comp_no]=value;
								return_code=shift_Parse_state(state,1);
							}
							else
							{
								display_message(ERROR_MESSAGE,"Invalid int: %s",current_token);
								display_parse_state_location(state);
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"Missing int vector component(s)");
							display_parse_state_location(state);
							return_code=0;
						}
					}
				}
				else
				{
					/* write help text */
					for (comp_no=0;comp_no<number_of_components;comp_no++)
					{
						display_message(INFORMATION_MESSAGE," #");
					}
					display_message(INFORMATION_MESSAGE,"[%d",values_address[0]);
					for (comp_no=1;comp_no<number_of_components;comp_no++)
					{
						display_message(INFORMATION_MESSAGE," %d",values_address[comp_no]);
					}
					display_message(INFORMATION_MESSAGE,"]");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Missing %d component int vector",number_of_components);
				display_parse_state_location(state);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"set_int_vector.  Invalid argument(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_int_vector.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_int_vector */

int set_float(struct Parse_state *state,void *value_address_void,
	void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 21 June 1999

DESCRIPTION :
A modifier function for setting a float.
==============================================================================*/
{
	char *current_token;
	float value,*value_address;
	int return_code;

	ENTER(set_float);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (value_address=(float *)value_address_void)
				{
					if (1==sscanf(current_token," %f ",&value))
					{
						*value_address=value;
						return_code=shift_Parse_state(state,1);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Invalid float: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"set_float.  Missing value_address");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," #");
				if (value_address=(float *)value_address_void)
				{
					display_message(INFORMATION_MESSAGE,"[%g]",*value_address);
				}
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing float");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_float.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_float */

int set_float_and_char_flag(struct Parse_state *state,void *value_address_void,
	void *flag_address_void)
/*******************************************************************************
LAST MODIFIED : 9 September 1999

DESCRIPTION :
A modifier function for setting a float, and a char flag in the user data to
indicate that the float has been set.
==============================================================================*/
{
	char *current_token, *flag_address;
	float value,*value_address;
	int return_code;

	ENTER(set_float_and_char_flag);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if ((value_address=(float *)value_address_void) &&
					 (flag_address=(char *)flag_address_void))
				{
					if (1==sscanf(current_token," %f ",&value))
					{
						*value_address=value;
						*flag_address=1;
						return_code=shift_Parse_state(state,1);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Invalid float: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_float_and_char_flag.  Missing value_address");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," #");
				if (value_address=(float *)value_address_void)
				{
					display_message(INFORMATION_MESSAGE,"[%g]",*value_address);
				}
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing float");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_float_and_char_flag.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_float_and_char_flag */

int set_float_positive(struct Parse_state *state,void *value_address_void,
	void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 21 June 1999

DESCRIPTION :
A modifier function for setting a float to a positive value.
==============================================================================*/
{
	char *current_token;
	float value,*value_address;
	int return_code;

	ENTER(set_float_positive);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (value_address=(float *)value_address_void)
				{
					if (1==sscanf(current_token," %f ",&value))
					{
						/* make sure that the value value is positive */
						if (value>0)
						{
							*value_address=value;
							return_code=shift_Parse_state(state,1);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Value must be a positive float: %s\n",current_token);
							display_parse_state_location(state);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Invalid positive float: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_float_positive.  Missing value_address");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," #");
				if (value_address=(float *)value_address_void)
				{
					display_message(INFORMATION_MESSAGE,"[%g]",*value_address);
				}
				display_message(INFORMATION_MESSAGE,"{>0}");
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing positive float");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_float_positive.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_float_positive */

int set_float_non_negative(struct Parse_state *state,void *value_address_void,
	void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 21 June 1999

DESCRIPTION :
A modifier function for setting a float to a non_negative value.
==============================================================================*/
{
	char *current_token;
	float value,*value_address;
	int return_code;

	ENTER(set_float_non_negative);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (value_address=(float *)value_address_void)
				{
					if (1==sscanf(current_token," %f ",&value))
					{
						/* make sure that the value value is non-negative */
						if (value>=0)
						{
							*value_address=value;
							return_code=shift_Parse_state(state,1);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Value must be a non_negative float: %s\n",current_token);
							display_parse_state_location(state);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Invalid non-negative float: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_float_non_negative.  Missing value_address");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," #");
				if (value_address=(float *)value_address_void)
				{
					display_message(INFORMATION_MESSAGE,"[%g]",*value_address);
				}
				display_message(INFORMATION_MESSAGE,"{>=0}");
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing non-negative float");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_float_non_negative.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_float_non_negative */

int set_float_0_to_1_inclusive(struct Parse_state *state,
	void *value_address_void,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 21 June 1999

DESCRIPTION :
A modifier function for setting a float to a value in [0,1].
==============================================================================*/
{
	char *current_token;
	float value,*value_address;
	int return_code;

	ENTER(set_float_0_to_1_inclusive);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (value_address=(float *)value_address_void)
				{
					if (1==sscanf(current_token," %f ",&value))
					{
						/* make sure that the value value is non-negative */
						if (value>=0)
						{
							*value_address=value;
							return_code=shift_Parse_state(state,1);
						}
						else
						{
							display_message(ERROR_MESSAGE,"Value must be a 0<=float<=1: %s\n",
								current_token);
							display_parse_state_location(state);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Invalid 0<=float<=1: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_float_0_to_1_inclusive.  Missing value_address");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," #");
				if (value_address=(float *)value_address_void)
				{
					display_message(INFORMATION_MESSAGE,"[%g]",*value_address);
				}
				display_message(INFORMATION_MESSAGE,"{>=0,<=1}");
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing 0<=float<=1");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_float_0_to_1_inclusive.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_float_0_to_1_inclusive */

int set_double(struct Parse_state *state,void *value_address_void,
	void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 21 June 1999

DESCRIPTION :
A modifier function for setting a double.
==============================================================================*/
{
	char *current_token;
	double value,*value_address;
	int return_code;

	ENTER(set_double);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (value_address=(double *)value_address_void)
				{
					if (1==sscanf(current_token," %lf ",&value))
					{
						*value_address=value;
						return_code=shift_Parse_state(state,1);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Invalid double: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"set_double.  Missing value_address");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," #");
				if (value_address=(double *)value_address_void)
				{
					display_message(INFORMATION_MESSAGE,"[%g]",*value_address);
				}
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing double");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_double.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_double */

int set_double_and_char_flag(struct Parse_state *state,void *value_address_void,
	void *flag_address_void)
/*******************************************************************************
LAST MODIFIED : 19 October 1998

DESCRIPTION :
A modifier function for setting a double, and a char flag in the user data to
indicate that the double has been set.
???SAB  The user_data could be used to supply many more helpful things such as
	limits on the double or a string used in the help.
==============================================================================*/
{
	char *current_token, *flag_address;
	double value,*value_address;
	int return_code;

	ENTER(set_double_and_char_flag);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if ((value_address=(double *)value_address_void) &&
					 (flag_address=(char *)flag_address_void))
				{
					if (1==sscanf(current_token," %lf ",&value))
					{
						*value_address=value;
						*flag_address=1;
						return_code=shift_Parse_state(state,1);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Invalid double: %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_double_and_char_flag.  Missing value_address");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," #");
				if (value_address=(double *)value_address_void)
				{
					display_message(INFORMATION_MESSAGE,"[%g]",*value_address);
				}
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing double");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_double_and_char_flag.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_double_and_char_flag */

int set_special_float3(struct Parse_state *state,void *values_address_void,
	void *separation_char_address_void)
/*******************************************************************************
LAST MODIFIED : 9 July 1998

DESCRIPTION :
Modifier function for setting a float[3] from a token with 1 to 3 characters
separated by the character at <separation_char_address> which may be either an
asterisk or a comma. If '*' is used missing components take the values of the
last number entered, eg '3' -> 3,3,3, while  '2.4*7.6' becomes 2.4,7.6,7.6.
This functionality is useful for setting the size of glyphs. If the separation
character is ',', values of unspecified components are left untouched, useful
for setting glyph offsets which default to zero or some other number.
Missing a number by putting two separators together works as expected, eg:
'1.2**3.0' returns 1.2,1.2,3.0, '*2' gives 0.0,2.0,2.0 while ',,3' changes the
third component of the float only to 3.
???RC The comma case does not work since ',' is a delimiter for the parser.
==============================================================================*/
{
	char *current_token,separator;
	float value,*values;
	int i,return_code;

	ENTER(set_special_float3);
	if (state&&(values=(float *)values_address_void)&&
		(separator=*((char *)separation_char_address_void))&&
		(('*'==separator)||(','==separator)))
	{
		if (current_token=state->current_token)
		{
			return_code=1;
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				value=(float)0.0;
				for (i=0;i<3;i++)
				{
					if (current_token&&(*current_token != separator))
					{
						value=(float)atof(current_token);
						values[i]=value;
						current_token=strchr(current_token,separator);
					}
					else
					{
						if ('*' == separator)
						{
							values[i]=value;
						}
					}
					if (current_token)
					{
						current_token++;
						if ('\0' == *current_token)
						{
							current_token=(char *)NULL;
						}
					}
				}
				return_code=shift_Parse_state(state,1);
			}
			else
			{
				display_message(INFORMATION_MESSAGE,
					" #%c#%c#[%g%c%g%c%g]{float[%cfloat[%cfloat]]}",separator,separator,
					values[0],separator,values[1],separator,values[2],
					separator,separator);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing vector");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_float_vector.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_special_float3 */

int set_float_vector(struct Parse_state *state,void *values_address_void,
	void *number_of_components_address_void)
/*******************************************************************************
LAST MODIFIED : 20 November 1998

DESCRIPTION :
Modifier function for reading number_of_components (>0) floats from <state>.
User data consists of a pointer to an integer containing number_of_components,
while <values_address_void> should point to a large enough space to store the
number_of_components floats.
Now prints current contents of the vector with help.
==============================================================================*/
{
	char *current_token;
	float value,*values_address;
	int comp_no,number_of_components,return_code;

	ENTER(set_float_vector);
	if (state)
	{
		if ((values_address=(float *)values_address_void)&&
			number_of_components_address_void&&(0<(number_of_components=
			*((int *)number_of_components_address_void))))
		{
			if (current_token=state->current_token)
			{
				return_code=1;
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					for (comp_no=0;return_code&&(comp_no<number_of_components);comp_no++)
					{
						if (current_token=state->current_token)
						{
							if (1==sscanf(current_token," %f ",&value))
							{
								values_address[comp_no]=value;
								return_code=shift_Parse_state(state,1);
							}
							else
							{
								display_message(ERROR_MESSAGE,"Invalid float: %s",
									current_token);
								display_parse_state_location(state);
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Missing float vector component(s)");
							display_parse_state_location(state);
							return_code=0;
						}
					}
				}
				else
				{
					/* write help text */
					for (comp_no=0;comp_no<number_of_components;comp_no++)
					{
						display_message(INFORMATION_MESSAGE," #");
					}
					display_message(INFORMATION_MESSAGE,"[%g",values_address[0]);
					for (comp_no=1;comp_no<number_of_components;comp_no++)
					{
						display_message(INFORMATION_MESSAGE," %g",values_address[comp_no]);
					}
					display_message(INFORMATION_MESSAGE,"]");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Missing %d component float vector",number_of_components);
				display_parse_state_location(state);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"set_float_vector.  Invalid argument(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_float_vector.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_float_vector */

int set_FE_value_array(struct Parse_state *state, void *values_void,
	void *number_of_components_address_void)
/*******************************************************************************
LAST MODIFIED : 6 November 2001

DESCRIPTION :
Modifier function for reading number_of_components (>0) FE_values from <state>.
User data consists of a pointer to an integer containing number_of_components,
while <values_void> should point to a large enough space to store the
number_of_components FE_values.
<number_of_components> can be zero and <values> can be NULL as long as only
help mode is entered.
Now prints current contents of the vector with help.
==============================================================================*/
{
	char *current_token;
	FE_value value, *values;
	int i, number_of_components, return_code;

	ENTER(set_FE_value_array);
	if (state && number_of_components_address_void)
	{
		values = (FE_value *)values_void;
		number_of_components = *((int *)number_of_components_address_void);
		if (current_token = state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (values && (0 < number_of_components))
				{
					return_code = 1;
					for (i = 0; return_code && (i < number_of_components); i++)
					{
						if (current_token = state->current_token)
						{
							if (1 == sscanf(current_token, " %f ", &value))
							{
								values[i] = value;
								return_code = shift_Parse_state(state, 1);
							}
							else
							{
								display_message(ERROR_MESSAGE, "Invalid FE_value: %s",
									current_token);
								display_parse_state_location(state);
								return_code = 0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Missing FE_value vector component(s)");
							display_parse_state_location(state);
							return_code = 0;
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_FE_value_array.  Invalid array or number_of_components");
					return_code = 0;
				}
			}
			else
			{
				if (values && (0 < number_of_components))
				{
					/* write help text */
					for (i = 0; i < number_of_components; i++)
					{
						display_message(INFORMATION_MESSAGE," #");
					}
					display_message(INFORMATION_MESSAGE, "[%g", values[0]);
					for (i = 1; i < number_of_components; i++)
					{
						display_message(INFORMATION_MESSAGE, " %g", values[i]);
					}
					display_message(INFORMATION_MESSAGE, "]");
				}
				else
				{
					display_message(INFORMATION_MESSAGE," VALUES");
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing values");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "set_FE_value_array.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* set_FE_value_array */

int set_double_vector(struct Parse_state *state,void *values_address_void,
	void *number_of_components_address_void)
/*******************************************************************************
LAST MODIFIED : 20 November 1998

DESCRIPTION :
Modifier function for reading number_of_components (>0) floats from <state>.
User data consists of a pointer to an integer containing number_of_components,
while <values_address_void> should point to a large enough space to store the
number_of_components floats.
Now prints current contents of the vector with help.
==============================================================================*/
{
	char *current_token;
	double value,*values_address;
	int comp_no,number_of_components,return_code;

	ENTER(set_double_vector);
	if (state)
	{
		if ((values_address=(double *)values_address_void)&&
			number_of_components_address_void&&(0<(number_of_components=
			*((int *)number_of_components_address_void))))
		{
			if (current_token=state->current_token)
			{
				return_code=1;
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					for (comp_no=0;return_code&&(comp_no<number_of_components);comp_no++)
					{
						if (current_token=state->current_token)
						{
							if (1==sscanf(current_token," %lf ",&value))
							{
								values_address[comp_no]=value;
								return_code=shift_Parse_state(state,1);
							}
							else
							{
								display_message(ERROR_MESSAGE,"Invalid double: %s",
									current_token);
								display_parse_state_location(state);
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Missing double vector component(s)");
							display_parse_state_location(state);
							return_code=0;
						}
					}
				}
				else
				{
					/* write help text */
					for (comp_no=0;comp_no<number_of_components;comp_no++)
					{
						display_message(INFORMATION_MESSAGE," #");
					}
					display_message(INFORMATION_MESSAGE,"[%g",values_address[0]);
					for (comp_no=1;comp_no<number_of_components;comp_no++)
					{
						display_message(INFORMATION_MESSAGE," %g",values_address[comp_no]);
					}
					display_message(INFORMATION_MESSAGE,"]");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Missing %d component double vector",number_of_components);
				display_parse_state_location(state);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"set_double_vector.  Invalid argument(s)");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_double_vector.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_double_vector */

int set_double_vector_with_help(struct Parse_state *state,
	void *vector_void,void *set_vector_with_help_data_void)
/*******************************************************************************
LAST MODIFIED : 20 November 1998

DESCRIPTION :
Modifier function to parse a variable number of double values with appropriate
help text. Number of components in vectoy, help text and set flag are contained
in the struct Set_vector_with_help_data passed as the last argument. The 'set'
flag is set if values are entered into the vector.
The current values of the vector are not printed with the help text since they
may not be initialised (calling function could put them in the help text).
==============================================================================*/
{
	char *current_token;
	double *vector;
	int return_code,i;
	struct Set_vector_with_help_data *set_vector_data;

	ENTER(set_double_vector_with_help);
	if (state&&(vector=(double *)vector_void)&&(set_vector_data=
		(struct Set_vector_with_help_data *)set_vector_with_help_data_void)&&
		(0<set_vector_data->num_values)&&set_vector_data->help_text)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				set_vector_data->set=1;
				return_code=1;
				for (i=set_vector_data->num_values;return_code&&(0<i);i--)
				{
					if (current_token=state->current_token)
					{
						*vector=atof(current_token);
						vector++;
						return_code=shift_Parse_state(state,1);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Missing vector component(s)");
						return_code=0;
					}
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE,set_vector_data->help_text);
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing%s",set_vector_data->help_text);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_double_vector_with_help.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_double_vector_with_help */

int set_char_flag(struct Parse_state *state,void *value_address_void,
	void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 21 June 1999

DESCRIPTION :
A modifier function for setting a character flag to 1.
==============================================================================*/
{
	char *value_address;
	int return_code;

	ENTER(set_char_flag);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		if (value_address=(char *)value_address_void)
		{
			*value_address=1;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,"set_char_flag.  Missing value_address");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_char_flag.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_char_flag */

int unset_char_flag(struct Parse_state *state,void *value_address_void,
	void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 21 June 1999

DESCRIPTION :
A modifier function for setting a character flag to 0.
==============================================================================*/
{
	char *value_address;
	int return_code;

	ENTER(unset_char_flag);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		if (value_address=(char *)value_address_void)
		{
			*value_address=0;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,"unset_char_flag.  Missing value_address");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"unset_char_flag.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* unset_char_flag */

int set_int_switch(struct Parse_state *state,void *value_address_void,
	void *token_void)
/*******************************************************************************
LAST MODIFIED : 12 May 2000

DESCRIPTION :
A modifier function for setting an integer switch to 1.
If the value is currently set, this is indicated in the help, with the <token>
if supplied, otherwise the word CURRENT.
If the option's <token> is supplied and its value is currently set, it 
==============================================================================*/
{
	char *token;
	int *value_address;
	int return_code;

	ENTER(set_int_switch);
	if (state&&(value_address=(int *)value_address_void))
	{
		if (!Parse_state_help_mode(state))
		{
			*value_address=1;
			return_code=1;
		}
		else
		{
			/* indicate if switch currently set */
			if (*value_address)
			{
				if (token=(char *)token_void)
				{
					display_message(INFORMATION_MESSAGE,"[%s]",token);
				}
				else
				{
					display_message(INFORMATION_MESSAGE,"[CURRENT]");
				}
			}
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_int_switch.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_int_switch */

int unset_int_switch(struct Parse_state *state,void *value_address_void,
	void *token_void)
/*******************************************************************************
LAST MODIFIED : 12 May 2000

DESCRIPTION :
A modifier function for setting an integer switch to 0.
If the value is currently unset, this is indicated in the help, with the <token>
if supplied, otherwise the word CURRENT.
If the option's <token> is supplied and its value is currently set, it 
==============================================================================*/
{
	char *token;
	int *value_address;
	int return_code;

	ENTER(unset_int_switch);
	if (state&&(value_address=(int *)value_address_void))
	{
		if (!Parse_state_help_mode(state))
		{
			*value_address=0;
			return_code=1;
		}
		else
		{
			/* indicate if switch currently unset */
			if (!(*value_address))
			{
				if (token=(char *)token_void)
				{
					display_message(INFORMATION_MESSAGE,"[%s]",token);
				}
				else
				{
					display_message(INFORMATION_MESSAGE,"[CURRENT]");
				}
			}
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"unset_int_switch.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* unset_int_switch */

int set_file_name(struct Parse_state *state,void *name_address_void,
	void *directory_name_address_void)
/*******************************************************************************
LAST MODIFIED : 26 September 1996

DESCRIPTION :
Allows the user to specify "special" directories, eg examples.  Allocates the
memory for the file name string.
==============================================================================*/
{
	char *current_token,*directory_name,**name_address;
	int file_name_length,return_code;

	ENTER(set_file_name);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (name_address=(char **)name_address_void)
				{
					if (*name_address)
					{
						DEALLOCATE(*name_address);
					}
					if (directory_name_address_void)
					{
						directory_name= *((char **)directory_name_address_void);
					}
					else
					{
						directory_name=(char *)NULL;
					}
					file_name_length=strlen(current_token)+1;
					if (directory_name)
					{
						file_name_length += strlen(directory_name);
					}
					if (ALLOCATE(*name_address,char,file_name_length+1))
					{
						(*name_address)[0]='\0';
						if (directory_name)
						{
							strcat(*name_address,directory_name);
						}
						strcat(*name_address,current_token);
						return_code=shift_Parse_state(state,1);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"set_file_name.  Could not allocate memory for name");
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_file_name.  Missing name_address");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," FILE_NAME");
				if ((name_address=(char **)name_address_void)&&(*name_address))
				{
					display_message(INFORMATION_MESSAGE,"[%s]",*name_address);
				}
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing file name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_file_name.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_file_name */

int set_nothing(struct Parse_state *state,void *dummy_to_be_modified,
	void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 21 June 1999

DESCRIPTION :
Always succeeds and does nothing to the parse <state>.
???DB.  Temporary ?
==============================================================================*/
{
	int return_code;

	ENTER(set_nothing);
	USE_PARAMETER(state);
	USE_PARAMETER(dummy_to_be_modified);
	USE_PARAMETER(dummy_user_data);
	return_code=1;
	LEAVE;

	return (return_code);
} /* set_nothing */

int set_integer_range(struct Parse_state *state,
	void *integer_range_address_void,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 21 June 1999

DESCRIPTION :
integer_range= *((int **)integer_range_address_void) is an array.
integer_range[0] is the number of pairs of ints in the rest of integer_range, so
that the size of integer_range is 1+2*integer_range[0].  integer_range is
ordered integer_range[2*i+1]<=integer_range[2*i+2]<integer_range[2*i+3] for
i>=0.  The integers in integer_range are those j for which
integer_range[2*i+1]<=j<=integer_range[2*i+2] for some i>=0.

This routine updates the integer_range based on the current token which can be
of two forms - # or #..#
==============================================================================*/
{
	char *current_token;
	int first,i,*integer_range,**integer_range_address,j,last,
		number_of_sub_ranges,return_code;

	ENTER(set_integer_range);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				return_code=0;
				if (2==sscanf(current_token,"%d..%d",&first,&last))
				{
					if (first<=last)
					{
						return_code=1;
					}
				}
				else
				{
					if (1==sscanf(current_token,"%d",&first))
					{
						last=first;
						return_code=1;
					}
				}
				if (return_code)
				{
					if (integer_range_address=(int **)integer_range_address_void)
					{
						if ((integer_range= *integer_range_address)&&
							(0<(number_of_sub_ranges=integer_range[0])))
						{
							i=0;
							while ((i<number_of_sub_ranges)&&(first>=integer_range[2*i+1]))
							{
								i++;
							}
							if (i<number_of_sub_ranges)
							{
								if (last>=integer_range[2*i+1]-1)
								{
									integer_range[2*i+1]=first;
									if (last>integer_range[2*i+2])
									{
										integer_range[2*i+2]=last;
									}
									i= -1;
								}
							}
							if (i>0)
							{
								if (first<=integer_range[2*i]+1)
								{
									integer_range[2*i]=last;
									if (first<integer_range[2*i-1])
									{
										integer_range[2*i-1]=first;
									}
									i= -1;
								}
							}
							if (i>=0)
							{
								number_of_sub_ranges++;
								if (REALLOCATE(integer_range,integer_range,int,
									2*number_of_sub_ranges+1))
								{
									*integer_range_address=integer_range;
									integer_range[0]=number_of_sub_ranges;
									for (j=number_of_sub_ranges-1;j>i;j--)
									{
										integer_range[2*j+2]=integer_range[2*j];
										integer_range[2*j+1]=integer_range[2*j-1];
									}
									integer_range[2*i+2]=last;
									integer_range[2*i+1]=first;
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"set_integer_range.  Could not reallocate integer_range");
									return_code=0;
								}
							}
						}
						else
						{
							if (REALLOCATE(integer_range,integer_range,int,3))
							{
								integer_range[0]=1;
								integer_range[1]=first;
								integer_range[2]=last;
								*integer_range_address=integer_range;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"set_integer_range.  Could not reallocate integer_range");
								return_code=0;
							}
						}
						if (return_code)
						{
							return_code=shift_Parse_state(state,1);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"set_integer_range.  Missing integer_range_address");
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"Invalid integer range");
					display_parse_state_location(state);
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," #|#..#");
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing integer range");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_integer_range.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_integer_range */

int set_enum(struct Parse_state *state,void *set_value_address_void,
	void *enum_value_address_void)
/*******************************************************************************
LAST MODIFIED : 19 November 1998

DESCRIPTION :
A modifier function for setting an enumerated type variable to a specified
value.
NB.  *enum_value_address_void is put in *set_value_address_void
???DB.  Unwieldy.  Can it be done better ?
==============================================================================*/
{
	int *enum_value_address,return_code,*set_value_address;

	ENTER(set_enum);
	USE_PARAMETER(state);
	if ((set_value_address=(int *)set_value_address_void)&&
		(enum_value_address=(int *)enum_value_address_void))
	{
		*set_value_address=*enum_value_address;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_enum.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_enum */
