/*******************************************************************************
FILE : computed_field_set.c

LAST MODIFIED : 7 May 2007

DESCRIPTION :
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
extern "C" {
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_composite.h"
#include "computed_field/computed_field_set.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "user_interface/message.h"
#include <stdio.h>
}
#include "computed_field/computed_field_private.hpp"

int set_Computed_field_conditional(struct Parse_state *state,
	void *field_address_void, void *set_field_data_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Modifier function to set the field from a command. <set_field_data_void> should
point to a struct Set_Computed_field_conditional_data containing the
computed_field_manager and an optional conditional function for narrowing the
range of fields available for selection. If the conditional_function is NULL,
no criteria are placed on the chosen field.
Allows the construction field.component name to automatically make a component
wrapper for field and add it to the manager.
==============================================================================*/
{
	char *command_string, *current_token, *field_component_name, *temp_name;
	int component_no, i, error, finished, index, number_of_components,
		number_of_values, return_code;
	double *values;
	struct Computed_field **field_address, *selected_field;
	struct Set_Computed_field_conditional_data *set_field_data;

	ENTER(set_Computed_field_conditional);
	if (state && (field_address = (struct Computed_field **)field_address_void) &&
		(set_field_data =
			(struct Set_Computed_field_conditional_data *)set_field_data_void) &&
		set_field_data->computed_field_manager)
	{
		if (current_token = state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING, current_token) &&
				strcmp(PARSER_RECURSIVE_HELP_STRING, current_token))
			{
				if (fuzzy_string_compare(current_token, "NONE"))
				{
					if (*field_address)
					{
						DEACCESS(Computed_field)(field_address);
						*field_address = (struct Computed_field *)NULL;
					}
					return_code = 1;
				}
				else 
				{
					if (current_token[0] == '[')
					{
						error = 0;
						command_string = duplicate_string(current_token);

						number_of_values = 0;
						/* Dont examine the '[' */
						current_token += 1;

						ALLOCATE(values, double, 1);
						/* This is a constant array, make a new field not in the manager */
						finished = 0;
						while (!finished)
						{
							while (current_token &&
								(1 == sscanf(current_token, "%lf%n",
								values + number_of_values, &index)))
							{
								number_of_values++;
								REALLOCATE(values, values, double, number_of_values + 1);
								current_token += index;
							}
							if (strchr(current_token,']') ||
								(!shift_Parse_state(state, 1)) ||
								(!(current_token = state->current_token)))
							{
								finished = 1;
							}
							else
							{
								append_string(&command_string, " ", &error);
								append_string(&command_string, current_token, &error);
							}
						}
						selected_field = CREATE(Computed_field)("constants");
						Computed_field_set_command_string(selected_field, 
							command_string);
						Computed_field_copy_type_specific_and_deaccess(selected_field, 
							Computed_field_create_constant(
							number_of_values, values));
						DEALLOCATE(values);
						DEALLOCATE(command_string);
					}
					else
					{
						/* component_no = -1 denotes the whole field may be used */
						component_no = -1;
						if (!(selected_field =
								FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field, name)(current_token,
									set_field_data->computed_field_manager)))
						{
							if (field_component_name = strchr(current_token, '.'))
							{
								*field_component_name = '\0';
								field_component_name++;
								if (selected_field = FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,
										name)(current_token, set_field_data->computed_field_manager))
								{
									/* get the component number */
									number_of_components =
										Computed_field_get_number_of_components(selected_field);
									for (i = 0; (i < number_of_components) &&
											  (0 > component_no) && selected_field; i++)
									{
										if (temp_name =
											Computed_field_get_component_name(selected_field, i))
										{
											if (0 == strcmp(field_component_name, temp_name))
											{
												component_no = i;
											}
											DEALLOCATE(temp_name);
										}
										else
										{
											display_message(WARNING_MESSAGE,
												"set_Computed_field_component.  "
												"Could not get component name");
											selected_field = (struct Computed_field *)NULL;
										}
									}
									if (0 <= component_no)
									{
										if (1 == number_of_components)
										{
											/* already a single component field */
											component_no = -1;
										}
										else
										{
											/* get or make wrapper for field component */
											if (!(selected_field =
													Computed_field_manager_get_component_wrapper(
														set_field_data->computed_field_manager,
														selected_field, component_no)))
											{
												display_message(WARNING_MESSAGE,
													"set_Computed_field_component.  "
													"Could not make component wrapper");
											}
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"Unknown field component: %s.%s", current_token,
											field_component_name);
										selected_field = (struct Computed_field *)NULL;
									}
								}
								else
								{
									display_message(ERROR_MESSAGE, "Unknown field : %s",
										current_token);
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,"Unknown field : %s",
									current_token);
							}
						}
					}
					if (selected_field)
					{
						if ((NULL == set_field_data->conditional_function) ||
							((set_field_data->conditional_function)(selected_field,
								set_field_data->conditional_function_user_data)))
						{
							REACCESS(Computed_field)(field_address, selected_field);
							return_code=1;
						}
						else
						{
							display_message(ERROR_MESSAGE, "Field of incorrect type : %s",
								current_token);
							return_code = 0;
						}
					}
					else
					{
						return_code = 0;
					}
				}
				shift_Parse_state(state, 1);
			}
			else
			{
				display_message(INFORMATION_MESSAGE,
					" FIELD_NAME[.COMPONENT_NAME]|none");
				/* if possible, then write the name */
				if (selected_field = *field_address)
				{
					GET_NAME(Computed_field)(selected_field, &temp_name);
					display_message(INFORMATION_MESSAGE, "[%s]", temp_name);
					DEALLOCATE(temp_name);
				}
				else
				{
					display_message(INFORMATION_MESSAGE, "[none]");
				}
				return_code=1;
			}
		}
		else
		{
			display_message(WARNING_MESSAGE, "Missing field name");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Computed_field_conditional.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* set_Computed_field_conditional */

int Option_table_add_Computed_field_conditional_entry(
	struct Option_table *option_table, char *token, 
	struct Computed_field **field_address, 
	struct Set_Computed_field_conditional_data *set_field_data)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Adds the given <token> to the <option_table>.
==============================================================================*/
{
	int return_code;

	ENTER(Option_table_add_Computed_field_conditional_entry);
	if (option_table && token && field_address && set_field_data)
	{
		return_code = Option_table_add_entry(option_table, token, (void *)field_address, 
			(void *)set_field_data, set_Computed_field_conditional);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Option_table_add_Computed_field_conditional_entry.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Option_table_add_Computed_field_conditional_entry */

int set_Computed_field_array(struct Parse_state *state,
	void *field_array_void, void *set_field_array_data_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Modifier function to set an array of field from a command.
<set_field_array_data_void> should point to a struct
Set_Computed_field_array_conditional_data containing the number_of_fields in the
array, the computed_field_package and an optional conditional function for
narrowing the range of fields available for selection.
Works by repeatedly calling set_Computed_field_conditional.
???RC Make this globally available for calling any modifier function?
==============================================================================*/
{
	int i, return_code;
	struct Computed_field **field_array;
	struct Set_Computed_field_array_data *set_field_array_data;

	ENTER(set_Computed_field_array);
	if (state && (field_array = (struct Computed_field **)field_array_void) &&
		(set_field_array_data =
			(struct Set_Computed_field_array_data *)set_field_array_data_void) &&
		(0 < set_field_array_data->number_of_fields) &&
		set_field_array_data->conditional_data)
	{
		return_code = 1;
		for (i = 0; i < set_field_array_data->number_of_fields; i++)
		{
			if (!set_Computed_field_conditional(state,
				&(field_array[i]), (void *)set_field_array_data->conditional_data))
			{
				return_code = 0;
			}
		}
		if (!return_code)
		{
			if ((!state->current_token) ||
				(strcmp(PARSER_HELP_STRING, state->current_token) &&
					strcmp(PARSER_RECURSIVE_HELP_STRING, state->current_token)))
			{
				display_message(ERROR_MESSAGE,
					"set_Computed_field_array.  Error parsing field array");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Computed_field_array.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* set_Computed_field_array */

#if defined (OLD_CODE)
int set_Computed_field_component(struct Parse_state *state,
	void *field_component_void,void *computed_field_manager_void)
/*******************************************************************************
LAST MODIFIED : 24 August 2006

DESCRIPTION :
Used in command parsing to translate a FIELD_NAME.COMPONENT_NAME into a struct
Computed_field_component.
???RC.  Does not ACCESS the field (unlike set_Computed_field).
==============================================================================*/
{
	char *current_token,*field_component_name,*field_name,*temp_name;
	int component_no,i,return_code;
	struct Computed_field *field;
	struct Computed_field_component *field_component;
	struct MANAGER(Computed_field) *computed_field_manager;

	ENTER(set_Computed_field_component);
	if (state&&
		(field_component=(struct Computed_field_component *)field_component_void)&&
		(computed_field_manager=
			(struct MANAGER(Computed_field) *)computed_field_manager_void))
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (field_component_name=strchr(current_token,'.'))
				{
					*field_component_name='\0';
					field_component_name++;
				}
				if (field=FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field,name)(
					current_token,computed_field_manager))
				{
					if (field_component_name)
					{
						return_code=1;
						component_no=-1;
						for (i=0;(0>component_no)&&(i<field->number_of_components)&&
							return_code;i++)
						{
							if (temp_name=Computed_field_get_component_name(field,i))
							{
								if (0==strcmp(field_component_name,temp_name))
								{
									component_no=i;
								}
								DEALLOCATE(temp_name);
							}
							else
							{
								display_message(WARNING_MESSAGE,
									"set_Computed_field_component.  Not enough memory");
								return_code=0;
							}
						}
						if (return_code)
						{
							if (0 <= component_no)
							{
								field_component->field=field;
								field_component->component_no=component_no;
							}
							else
							{
								display_message(WARNING_MESSAGE,
									"Unknown field component %s.%s",current_token,
									field_component_name);
								return_code=0;
							}
						}
					}
					else
					{
						field_component->field=field;
						field_component->component_no=0;
						return_code=1;
					}
				}
				else
				{
					display_message(WARNING_MESSAGE,"Unknown field %s",current_token);
					return_code=1;
				}
				shift_Parse_state(state,1);
			}
			else
			{
				display_message(INFORMATION_MESSAGE," FIELD_NAME.COMPONENT_NAME");
				if (field_component->field&&
					GET_NAME(Computed_field)(field_component->field,&field_name))
				{
					if (1<field_component->field->number_of_components)
					{
						if (field_component_name=Computed_field_get_component_name(
							field_component->field,field_component->component_no))
						{
							display_message(INFORMATION_MESSAGE,"[%s.%s]",field_name,
								field_component_name);
							DEALLOCATE(field_component_name);
						}
					}
					else
					{
						display_message(INFORMATION_MESSAGE,"[%s]",field_name);
					}
					DEALLOCATE(field_name);
				}
				return_code=1;
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,"Missing field component name");
			display_parse_state_location(state);
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Computed_field_component.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Computed_field_component */
#endif /* defined (OLD_CODE) */
