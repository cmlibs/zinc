
#if 1
#include "configure/cmgui_configure.h"
#endif /* defined (BUILD_WITH_CMAKE) */




#include "general/enumerator_private.h"
#include "general/message.h"
#include "general/enumerator_private.hpp"
#include "general/message.h"
#include "general/debug.h"
#include "command/parser.h"
#include "graphics/light.h"
#include "graphics/light_app.h"
#include "graphics/colour_app.h"

int modify_Light(struct Parse_state *state,void *light_void,
	void *modify_light_data_void)
/*******************************************************************************
LAST MODIFIED : 9 October 2002

DESCRIPTION :
==============================================================================*/
{
	const char *current_token, *light_type_string, **valid_strings;
	enum Light_type light_type;
	float constant_attenuation, direction[3], linear_attenuation, position[3],
		quadratic_attenuation, spot_cutoff, spot_exponent;
	int num_floats, number_of_valid_strings, process, return_code;
	struct Colour light_colour;
	struct Light *light_to_be_modified,*light_to_be_modified_copy;
	struct Modify_light_data *modify_light_data;
	struct Option_table *option_table;

	ENTER(modify_Light);
	modify_light_data=(struct Modify_light_data *)modify_light_data_void;
	if (state && modify_light_data)
	{
		current_token=state->current_token;
		if (current_token)
		{
			process=0;
			light_to_be_modified=(struct Light *)light_void;
			if (light_to_be_modified)
			{
				if (IS_MANAGED(Light)(light_to_be_modified,modify_light_data->
					light_manager))
				{
					light_to_be_modified_copy=CREATE(Light)("copy");
					if (light_to_be_modified_copy)
					{
						MANAGER_COPY_WITHOUT_IDENTIFIER(Light,name)(
							light_to_be_modified_copy,light_to_be_modified);
						process=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"modify_Light.  Could not create light copy");
						return_code=0;
					}
				}
				else
				{
					light_to_be_modified_copy=light_to_be_modified;
					light_to_be_modified=(struct Light *)NULL;
					process=1;
				}
			}
			else
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					light_to_be_modified=FIND_BY_IDENTIFIER_IN_MANAGER(Light,name)(
						current_token,modify_light_data->light_manager);
					if (light_to_be_modified)
					{
						return_code=shift_Parse_state(state,1);
						if (return_code)
						{
							light_to_be_modified_copy=CREATE(Light)("copy");
							if (light_to_be_modified_copy)
							{
								MANAGER_COPY_WITHOUT_IDENTIFIER(Light,name)(
									light_to_be_modified_copy,light_to_be_modified);
								process=1;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"modify_Light.  Could not create light copy");
								return_code=0;
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Unknown light : %s",current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					light_to_be_modified=CREATE(Light)("help");
					if (light_to_be_modified)
					{
						if (modify_light_data->default_light)
						{
							MANAGER_COPY_WITHOUT_IDENTIFIER(Light,name)(
								light_to_be_modified,modify_light_data->default_light);
						}
						option_table=CREATE(Option_table)();
						Option_table_add_entry(option_table, "LIGHT_NAME",
							light_to_be_modified, modify_light_data_void, modify_Light);
						return_code = Option_table_parse(option_table, state);
						DESTROY(Option_table)(&option_table);
						DESTROY(Light)(&light_to_be_modified);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"modify_Light.  Could not create dummy light");
						return_code=0;
					}
				}
			}
			if (process)
			{
				num_floats=3;
				get_Light_type(light_to_be_modified_copy, &light_type);
				get_Light_colour(light_to_be_modified_copy, &light_colour);
				get_Light_direction(light_to_be_modified_copy, direction);
				get_Light_position(light_to_be_modified_copy, position);
				get_Light_attenuation(light_to_be_modified_copy,
					&constant_attenuation, &linear_attenuation, &quadratic_attenuation);
				get_Light_spot_cutoff(light_to_be_modified_copy, &spot_cutoff);
				get_Light_spot_exponent(light_to_be_modified_copy, &spot_exponent);

				option_table = CREATE(Option_table)();
				/* colour */
				Option_table_add_entry(option_table, "colour",
					&light_colour, NULL, set_Colour);
				/* constant_attenuation */
				Option_table_add_entry(option_table, "constant_attenuation",
					&constant_attenuation, NULL, set_float_non_negative);
				/* cutoff */
				Option_table_add_entry(option_table, "cut_off",
					&spot_cutoff, NULL, set_float);
				/* direction */
				Option_table_add_entry(option_table, "direction",
					direction, &num_floats, set_float_vector);
				/* exponent */
				Option_table_add_entry(option_table, "exponent",
					&spot_exponent, NULL, set_float_non_negative);
				/* light_type: infinite/point/spot */
				light_type_string = ENUMERATOR_STRING(Light_type)(light_type);
				valid_strings = ENUMERATOR_GET_VALID_STRINGS(Light_type)(
					&number_of_valid_strings,
					(ENUMERATOR_CONDITIONAL_FUNCTION(Light_type) *)NULL,
					(void *)NULL);
				Option_table_add_enumerator(option_table, number_of_valid_strings,
					valid_strings, &light_type_string);
				DEALLOCATE(valid_strings);
				/* linear_attenuation */
				Option_table_add_entry(option_table, "linear_attenuation",
					&linear_attenuation, NULL, set_float_non_negative);
				/* position */
				Option_table_add_entry(option_table, "position",
					position, &num_floats, set_float_vector);
				/* quadratic_attenuation */
				Option_table_add_entry(option_table, "quadratic_attenuation",
					&quadratic_attenuation, NULL, set_float_non_negative);
				return_code = Option_table_multi_parse(option_table, state);
				if (return_code)
				{
					if ((0.0 > spot_cutoff) || ((90.0 < spot_cutoff)))
					{
						display_message(WARNING_MESSAGE,
							"Spotlight cut-off angle must be from 0 to 90 degrees");
						if (0.0 > spot_cutoff)
						{
							spot_cutoff = 0.0;
						}
						else
						{
							spot_cutoff = 90.0;
						}
					}
					if (return_code)
					{
						STRING_TO_ENUMERATOR(Light_type)(light_type_string, &light_type);
						set_Light_type(light_to_be_modified_copy, light_type);
						set_Light_colour(light_to_be_modified_copy, &light_colour);
						set_Light_direction(light_to_be_modified_copy, direction);
						set_Light_position(light_to_be_modified_copy, position);
						set_Light_attenuation(light_to_be_modified_copy,
							constant_attenuation, linear_attenuation, quadratic_attenuation);
						set_Light_spot_cutoff(light_to_be_modified_copy, spot_cutoff);
						set_Light_spot_exponent(light_to_be_modified_copy, spot_exponent);
						if (light_to_be_modified)
						{
							MANAGER_MODIFY_NOT_IDENTIFIER(Light,name)(light_to_be_modified,
								light_to_be_modified_copy, modify_light_data->light_manager);
						}
					}
				}
				DESTROY(Option_table)(&option_table);
				if (light_to_be_modified)
				{
					DESTROY(Light)(&light_to_be_modified_copy);
				}
			}
		}
		else
		{
			if (light_void)
			{
				display_message(WARNING_MESSAGE, "Missing light modifications");
			}
			else
			{
				display_message(WARNING_MESSAGE, "Missing light name");
			}
			display_parse_state_location(state);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"modify_Light.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* modify_Light */

int set_Light(struct Parse_state *state,
	void *light_address_void,void *light_manager_void)
/*******************************************************************************
LAST MODIFIED : 22 June 1999

DESCRIPTION :
Modifier function to set the light from a command.
???RC set_Object routines could become a macro.
==============================================================================*/
{
	const char *current_token;
	int return_code;
	struct Light *temp_light,**light_address;
	struct MANAGER(Light) *light_manager;

	ENTER(set_Light);
	if (state)
	{
		current_token=state->current_token;
		if (current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if ((light_address=
					(struct Light **)light_address_void)&&
					(light_manager=(struct MANAGER(Light) *)
					light_manager_void))
				{
					if (fuzzy_string_compare(current_token,"NONE"))
					{
						if (*light_address)
						{
							DEACCESS(Light)(light_address);
							*light_address=(struct Light *)NULL;
						}
						return_code=1;
					}
					else
					{
						temp_light=FIND_BY_IDENTIFIER_IN_MANAGER(Light,
							name)(current_token,light_manager);
						if (temp_light)
						{
							if (*light_address!=temp_light)
							{
								DEACCESS(Light)(light_address);
								*light_address=ACCESS(Light)(temp_light);
							}
							return_code=1;
						}
						else
						{
							display_message(ERROR_MESSAGE,"Unknown light : %s",
								current_token);
							return_code=0;
						}
					}
					shift_Parse_state(state,1);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_Light.  Invalid argument(s)");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," LIGHT_NAME|none");
				/* if possible, then write the name */
				light_address=(struct Light **)light_address_void;
				if (light_address)
				{
					temp_light= *light_address;
					if (temp_light)
					{
						display_message(INFORMATION_MESSAGE,"[%s]",get_Light_name(temp_light));
					}
					else
					{
						display_message(INFORMATION_MESSAGE,"[none]");
					}
				}
				return_code=1;
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,"Missing light name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Light.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Light */
