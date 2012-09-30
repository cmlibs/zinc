

#include "command/parser.h"

#include "general/debug.h"
#include "general/enumerator_private.hpp"
#include "general/message.h"
#include "general/list.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_private.h"

int set_FE_field(struct Parse_state *state,void *field_address_void,
	void *fe_field_list_void)
/*******************************************************************************
LAST MODIFIED : 3 December 2002

DESCRIPTION :
Modifier function to set the field from the command line.
==============================================================================*/
{
	const char *current_token;
	int return_code;
	struct FE_field **field_address,*temp_FE_field;
	struct LIST(FE_field) *fe_field_list;

	ENTER(set_FE_field);
	if (state&&(field_address=(struct FE_field **)field_address_void)&&
		(fe_field_list=(struct LIST(FE_field) *)fe_field_list_void))
	{
		if (NULL != (current_token = state->current_token))
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (fuzzy_string_compare(current_token,"NONE"))
				{
					if (*field_address)
					{
						DEACCESS(FE_field)(field_address);
						*field_address=(struct FE_field *)NULL;
					}
					return_code=shift_Parse_state(state,1);
				}
				else
				{
					if (NULL != (temp_FE_field=FIND_BY_IDENTIFIER_IN_LIST(FE_field,name)(
						current_token,fe_field_list)))
					{
						if (*field_address!=temp_FE_field)
						{
							DEACCESS(FE_field)(field_address);
							*field_address=ACCESS(FE_field)(temp_FE_field);
						}
						return_code=shift_Parse_state(state,1);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Unknown field: %s",current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," FIELD_NAME|none");
				if (NULL != (temp_FE_field= *field_address))
				{
					display_message(INFORMATION_MESSAGE,"[%s]", get_FE_field_name(temp_FE_field));
				}
				else
				{
					display_message(INFORMATION_MESSAGE,"[none]");
				}
				return_code=1;
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,"Missing field name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_FE_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_FE_field */

int set_FE_field_conditional(struct Parse_state *state,
	void *field_address_void,void *set_field_data_void)
/*******************************************************************************
LAST MODIFIED : 3 December 2002

DESCRIPTION :
Modifier function to set the field from a command. <set_field_data_void> should
point to a struct Set_FE_field_conditional_data containing the
fe_field_list and an optional conditional function for narrowing the
range of fields available for selection. If the conditional_function is NULL,
this function works just like set_FE_field.
==============================================================================*/
{
	const char *current_token;
	int return_code;
	struct FE_field **field_address,*selected_field,*temp_field;
	struct Set_FE_field_conditional_data *set_field_data;

	ENTER(set_FE_field_conditional);
	if (state&&(field_address=(struct FE_field **)field_address_void)&&
		(set_field_data=
			(struct Set_FE_field_conditional_data *)set_field_data_void)&&
		set_field_data->fe_field_list)
	{
		if (NULL != (current_token = state->current_token))
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (fuzzy_string_compare(current_token,"NONE"))
				{
					if (*field_address)
					{
						DEACCESS(FE_field)(field_address);
						*field_address=(struct FE_field *)NULL;
					}
					return_code=1;
				}
				else
				{
					if (NULL != (selected_field=
						FIND_BY_IDENTIFIER_IN_LIST(FE_field,name)(current_token,
						set_field_data->fe_field_list)))
					{
						if ((NULL==set_field_data->conditional_function)||
							((set_field_data->conditional_function)(selected_field,
								set_field_data->conditional_function_user_data)))
						{
							if (*field_address != selected_field)
							{
								DEACCESS(FE_field)(field_address);
								*field_address=ACCESS(FE_field)(selected_field);
							}
							return_code=1;
						}
						else
						{
							display_message(ERROR_MESSAGE,"Field of incorrect type : %s",
								current_token);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Unknown field : %s",current_token);
						return_code=0;
					}
				}
				shift_Parse_state(state,1);
			}
			else
			{
				display_message(INFORMATION_MESSAGE," FIELD_NAME|none");
				/* if possible, then write the name */
				if (NULL != (temp_field= *field_address))
				{
					display_message(INFORMATION_MESSAGE,"[%s]",get_FE_field_name(temp_field));
				}
				else
				{
					display_message(INFORMATION_MESSAGE,"[none]");
				}
				return_code=1;
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,"Missing field name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_FE_field_conditional.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_FE_field_conditional */

int set_FE_fields(struct Parse_state *state,
	void *field_order_info_address_void, void *fe_field_list_void)
/*******************************************************************************
LAST MODIFIED : 3 December 2002

DESCRIPTION :
Modifier function to set an ordered list of fields, each separated by white
space until an unrecognised field name is encountered. Two special tokens are
understood in place of any fields: 'all' and 'none'.
For the case of 'all', a NULL FE_field_order_info structure is returned.
For the case of 'none', an empty FE_field_order_info structure is returned.
It is up to the calling function to destroy any FE_field_order_info structure
returned by this function, however, any such structure passed to this function
may be destroyed here - ie. in the 'all' case.
==============================================================================*/
{
	const char *current_token;
	int i, number_of_fields, return_code;
	struct FE_field *field;
	struct FE_field_order_info **field_order_info_address;
	struct LIST(FE_field) *fe_field_list;

	ENTER(set_FE_fields);
	if (state && (field_order_info_address =
		(struct FE_field_order_info **)field_order_info_address_void) &&
		(fe_field_list = (struct LIST(FE_field) *)fe_field_list_void))
	{
		if (NULL != (current_token = state->current_token))
		{
			if (strcmp(PARSER_HELP_STRING, current_token) &&
				strcmp(PARSER_RECURSIVE_HELP_STRING, current_token))
			{
				return_code = 1;
				if (fuzzy_string_compare(current_token, "ALL"))
				{
					/* return a NULL field_order_info for the ALL case */
					if (*field_order_info_address)
					{
						DESTROY(FE_field_order_info)(field_order_info_address);
						*field_order_info_address = (struct FE_field_order_info *)NULL;
					}
					shift_Parse_state(state, 1);
				}
				else if ((*field_order_info_address &&
					clear_FE_field_order_info(*field_order_info_address)) ||
					(*field_order_info_address = CREATE(FE_field_order_info)()))
				{
					if (fuzzy_string_compare(current_token, "NONE"))
					{
						/* return the empty FE_field_order_info for the NONE case */
						shift_Parse_state(state, 1);
					}
					else
					{
						/* find fields by name in the parse state until one is not
							 recognized or there are no more tokens */
						while (return_code && (current_token = state->current_token) &&
							(field = FIND_BY_IDENTIFIER_IN_LIST(FE_field, name)(
								current_token, fe_field_list)))
						{
							if (!(add_FE_field_order_info_field(*field_order_info_address,
								field) && shift_Parse_state(state, 1)))
							{
								display_message(ERROR_MESSAGE,
									"set_FE_fields.  Could not add field to list");
								return_code = 0;
							}
						}
						if (0 == get_FE_field_order_info_number_of_fields(
							*field_order_info_address))
						{
							display_message(ERROR_MESSAGE,
								"Unknown field : %s", current_token);
							return_code = 0;
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE, "set_FE_fields.  Not enough memory");
					return_code = 0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE, " FIELD_NAMES|all|none");
				if (*field_order_info_address)
				{
					number_of_fields = get_FE_field_order_info_number_of_fields(
						*field_order_info_address);
					if (0 == number_of_fields)
					{
						display_message(INFORMATION_MESSAGE, "[none]");
					}
					else
					{
						display_message(INFORMATION_MESSAGE, "[");
						for (i = 0; i < number_of_fields; i++)
						{
							if (0 < i)
							{
								display_message(INFORMATION_MESSAGE, " ");
							}
							if (NULL != (field =
								get_FE_field_order_info_field(*field_order_info_address, i)))
							{
								display_message(INFORMATION_MESSAGE, "%s", get_FE_field_name(field));
							}
						}
						display_message(INFORMATION_MESSAGE, "]");
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE, "[all]");
				}
				return_code = 1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "Missing field specifications");
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "set_FE_fields.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* set_FE_fields */


int set_FE_field_component(struct Parse_state *state,void *component_void,
	void *fe_field_list_void)
/*******************************************************************************
LAST MODIFIED : 3 December 2002

DESCRIPTION :
Used in command parsing to translate a field component name into an field
component.
???DB.  Should it be here ?
???RC.  Does not ACCESS the field (unlike set_FE_field, above).
==============================================================================*/
{
	const char *current_token;
	char *field_component_name,*temp_name;
	int field_component_number,i,return_code;
	struct FE_field *field;
	struct FE_field_component *component;
	struct LIST(FE_field) *fe_field_list;

	ENTER(set_FE_field_component);
	if (state&&(component=(struct FE_field_component *)component_void)&&
		(fe_field_list=(struct LIST(FE_field) *)fe_field_list_void))
	{
		if (NULL != (current_token = state->current_token))
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				field_component_name = const_cast<char *>(strrchr(current_token,'.'));
				if (field_component_name)
				{
					*field_component_name='\0';
					field_component_name++;
				}
				if (NULL != (field=FIND_BY_IDENTIFIER_IN_LIST(FE_field,name)(current_token,
					fe_field_list)))
				{
					if (field_component_name)
					{
						return_code=1;
						field_component_number=-1;
						for (i=0;(0>field_component_number)&&
							(i<get_FE_field_number_of_components(field))&&return_code;i++)
						{
							if (NULL != (temp_name=get_FE_field_component_name(field,i)))
							{
								if (0==strcmp(field_component_name,temp_name))
								{
									field_component_number=i;
								}
								DEALLOCATE(temp_name);
							}
							else
							{
								display_message(WARNING_MESSAGE,
									"set_FE_field_component.  Not enough memory");
								return_code=0;
							}
						}
						if (return_code)
						{
							if (0 <= field_component_number)
							{
								component->field=field;
								component->number=field_component_number;
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
						component->field=field;
						component->number=0;
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
				if (component->field)
				{
					if (1<get_FE_field_number_of_components(component->field))
					{
						display_message(INFORMATION_MESSAGE,"[%s.%s]",
							get_FE_field_name(component->field),
							get_FE_field_component_name(component->field, component->number));
					}
					else
					{
						display_message(INFORMATION_MESSAGE,"[%s]",get_FE_field_name(component->field));
					}
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
			"set_FE_field_component.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_FE_field_component */


