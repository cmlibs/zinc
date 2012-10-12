
#if 1
#include "configure/cmgui_configure.h"
#endif /* defined (BUILD_WITH_CMAKE) */

#include "general/message.h"
#include "command/parser.h"
#include "graphics/render_gl.h"
#include "graphics/light_model.h"
#include "graphics/colour_app.h"

int modify_Light_model(struct Parse_state *state,void *light_model_void,
	void *modify_light_model_data_void)
/*******************************************************************************
LAST MODIFIED : 13 December 1997

DESCRIPTION :
Modifies the properties of a light model.
==============================================================================*/
{
	struct Modifier_entry
		help_option_table[]=
		{
			{"LIGHT_MODEL_NAME",NULL,NULL,modify_Light_model},
			{NULL,NULL,NULL,NULL}
		},
		status_option_table[]=
		{
			{"disable",NULL,NULL,set_char_flag},
			{"enable",NULL,NULL,set_char_flag},
			{NULL,NULL,NULL,NULL}
		},
		infinite_local_option_table[]=
		{
			{"infinite_viewer",NULL,NULL,set_char_flag},
			{"local_viewer",NULL,NULL,set_char_flag},
			{NULL,NULL,NULL,NULL}
		},
		option_table[]=
		{
			{"ambient_colour",NULL,NULL,set_Colour},
			{NULL,NULL,NULL,NULL},
			{NULL,NULL,NULL,NULL},
			{NULL,NULL,NULL,NULL},
			{NULL,NULL,NULL,NULL}
		},
		sided_option_table[]=
		{
			{"one_sided",NULL,NULL,set_char_flag},
			{"two_sided",NULL,NULL,set_char_flag},
			{NULL,NULL,NULL,NULL}
		};
	const char *current_token;
	char infinite_viewer_flag,local_viewer_flag,
		one_sided_flag,two_sided_flag,disable_flag,enable_flag;
	int process,return_code;
	struct Colour ambient_colour;
	struct Light_model *light_model_to_be_modified,
		*light_model_to_be_modified_copy;
	struct Modify_light_model_data *modify_light_model_data;

	ENTER(modify_Light_model);
	/* check the arguments */
	if (state)
	{
		modify_light_model_data=(struct Modify_light_model_data *)
			modify_light_model_data_void;
		if (modify_light_model_data != 0)
		{
			current_token=state->current_token;
			if (current_token != 0)
			{
				process=0;
				light_model_to_be_modified=(struct Light_model *)light_model_void;
				if (light_model_to_be_modified != 0)
				{
					if (IS_MANAGED(Light_model)(light_model_to_be_modified,
						modify_light_model_data->light_model_manager))
					{
						light_model_to_be_modified_copy=CREATE(Light_model)("copy");
						if (light_model_to_be_modified_copy != 0)
						{
							MANAGER_COPY_WITHOUT_IDENTIFIER(Light_model,name)(
								light_model_to_be_modified_copy,light_model_to_be_modified);
							process=1;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"modify_Light_model.  Could not create light model copy");
							return_code=0;
						}
					}
					else
					{
						light_model_to_be_modified_copy=light_model_to_be_modified;
						light_model_to_be_modified=(struct Light_model *)NULL;
						process=1;
					}
				}
				else
				{
					if (strcmp(PARSER_HELP_STRING,current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
					{
						light_model_to_be_modified=FIND_BY_IDENTIFIER_IN_MANAGER(
							Light_model,name)(current_token,
							modify_light_model_data->light_model_manager);
						if (light_model_to_be_modified != 0)
						{
							return_code=shift_Parse_state(state,1);
							if (return_code)
							{
								light_model_to_be_modified_copy=CREATE(Light_model)("copy");
								if (light_model_to_be_modified_copy != 0)
								{
									MANAGER_COPY_WITH_IDENTIFIER(Light_model,name)(
										light_model_to_be_modified_copy,light_model_to_be_modified);
									process=1;
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"modify_Light_model.  Could not create light model copy");
									return_code=0;
								}
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"Unknown light model: %s",
								current_token);
							display_parse_state_location(state);
							return_code=0;
						}
					}
					else
					{
						light_model_to_be_modified=CREATE(Light_model)("help");
						if (light_model_to_be_modified != 0)
						{
							if (modify_light_model_data->default_light_model)
							{
								MANAGER_COPY_WITHOUT_IDENTIFIER(Light_model,name)(
									light_model_to_be_modified,modify_light_model_data->
									default_light_model);
							}
							(help_option_table[0]).to_be_modified=
								(void *)light_model_to_be_modified;
							(help_option_table[0]).user_data=modify_light_model_data_void;
							return_code=process_option(state,help_option_table);
							DESTROY(Light_model)(&light_model_to_be_modified);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"modify_Light_model.  Could not create dummy light model");
							return_code=0;
						}
					}
				}
				if (process)
				{
					Light_model_get_ambient(light_model_to_be_modified_copy,
						&ambient_colour);
					infinite_viewer_flag=0;
					local_viewer_flag=0;
					one_sided_flag=0;
					two_sided_flag=0;
					disable_flag=0;
					enable_flag=0;
					(option_table[0]).to_be_modified=&ambient_colour;
					(status_option_table[0]).to_be_modified= &disable_flag;
					(status_option_table[1]).to_be_modified= &enable_flag;
					(option_table[1]).user_data=status_option_table;
					(infinite_local_option_table[0]).to_be_modified=&infinite_viewer_flag;
					(infinite_local_option_table[1]).to_be_modified=&local_viewer_flag;
					(option_table[2]).user_data=infinite_local_option_table;
					(sided_option_table[0]).to_be_modified=&one_sided_flag;
					(sided_option_table[1]).to_be_modified=&two_sided_flag;
					(option_table[3]).user_data=sided_option_table;
					return_code=process_multiple_options(state,option_table);
					if (return_code)
					{
						if (disable_flag&&enable_flag)
						{
							display_message(ERROR_MESSAGE,"Only one of disable/enable");
							return_code=0;
						}
						if (one_sided_flag&&two_sided_flag)
						{
							display_message(ERROR_MESSAGE,"Only one of one_sided/two_sided");
							return_code=0;
						}
						if (infinite_viewer_flag&&local_viewer_flag)
						{
							display_message(ERROR_MESSAGE,"modify_Light_model.  "
								"Only one of infinite_viewer/local_viewer");
							return_code=0;
						}
						if (return_code)
						{
							Light_model_set_ambient(light_model_to_be_modified_copy,
								&ambient_colour);
							if (disable_flag)
							{
								Light_model_set_status(light_model_to_be_modified_copy,0);
							}
							if (enable_flag)
							{
								Light_model_set_status(light_model_to_be_modified_copy,1);
							}
							if (infinite_viewer_flag)
							{
								Light_model_set_viewer_mode(light_model_to_be_modified_copy,
									LIGHT_MODEL_INFINITE_VIEWER);
							}
							if (local_viewer_flag)
							{
								Light_model_set_viewer_mode(light_model_to_be_modified_copy,
									LIGHT_MODEL_LOCAL_VIEWER);
							}
							if (one_sided_flag)
							{
								Light_model_set_side_mode(light_model_to_be_modified_copy,
									LIGHT_MODEL_ONE_SIDED);
							}
							if (two_sided_flag)
							{
								Light_model_set_side_mode(light_model_to_be_modified_copy,
									LIGHT_MODEL_TWO_SIDED);
							}
							if (light_model_to_be_modified)
							{
								MANAGER_MODIFY_NOT_IDENTIFIER(Light_model,name)(
									light_model_to_be_modified,light_model_to_be_modified_copy,
									modify_light_model_data->light_model_manager);
							}
						}
					}
					if (light_model_to_be_modified)
					{
						DESTROY(Light_model)(&light_model_to_be_modified_copy);
					}
				}
			}
			else
			{
				if (light_model_void)
				{
					display_message(WARNING_MESSAGE,"Missing light model modifications");
				}
				else
				{
					display_message(WARNING_MESSAGE,"Missing light model name");
				}
				display_parse_state_location(state);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"modify_Light_model.  Missing modify_light_model_data_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"modify_Light_model.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* modify_Light_model */



int set_Light_model(struct Parse_state *state,
	void *light_model_address_void,void *light_model_manager_void)
/*******************************************************************************
LAST MODIFIED : 13 December 1997

DESCRIPTION :
Modifier function to set the light model from a command.
???RC set_Object routines could become a macro.
==============================================================================*/
{
	const char *current_token;
	int return_code;
	struct Light_model *temp_light_model,**light_model_address;
	struct MANAGER(Light_model) *light_model_manager;

	ENTER(set_Light_model);
	if (state)
	{
		current_token=state->current_token;
		if (current_token != 0)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if ((light_model_address=
					(struct Light_model **)light_model_address_void)&&
					(light_model_manager=(struct MANAGER(Light_model) *)
					light_model_manager_void))
				{
					temp_light_model=FIND_BY_IDENTIFIER_IN_MANAGER(Light_model,name)(
						current_token,light_model_manager);
					if (temp_light_model != 0)
					{
						if (*light_model_address!=temp_light_model)
						{
							DEACCESS(Light_model)(light_model_address);
							*light_model_address=ACCESS(Light_model)(temp_light_model);
						}
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,"Unknown light_model : %s",
							current_token);
						return_code=0;
					}
					shift_Parse_state(state,1);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_Light_model.  Invalid argument(s)");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," LIGHT_MODEL_NAME");
				/* if possible, then write the name */
				light_model_address=
					(struct Light_model **)light_model_address_void;
				if (light_model_address != 0)
				{
					temp_light_model= *light_model_address;
					if (temp_light_model != 0)
					{
						display_message(INFORMATION_MESSAGE,"[%s]",Light_model_get_name(temp_light_model));
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
			display_message(WARNING_MESSAGE,"Missing light_model name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Light_model.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Light_model */

