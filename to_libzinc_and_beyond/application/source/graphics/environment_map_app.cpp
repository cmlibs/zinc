
#include "general/debug.h"
#include "general/manager.h"
#include "command/parser.h"
#include "general/message.h"
#include "graphics/environment_map.h"

static int set_Environment_map_face_materials(struct Parse_state *state,
	void *environment_map_void,void *graphical_material_manager_void)
/*******************************************************************************
LAST MODIFIED : 3 October 1996

DESCRIPTION :
Set the <environment_map> to face materials.
==============================================================================*/
{
	const char *current_token;
	int face_no,return_code;
	struct Environment_map *environment_map;
	struct MANAGER(Graphical_material) *graphical_material_manager;

	ENTER(set_Environment_map_face_materials);
	if (state)
	{
		current_token=state->current_token;
		if (current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if ((environment_map=(struct Environment_map *)environment_map_void)&&
					(graphical_material_manager=(struct MANAGER(Graphical_material) *)
					graphical_material_manager_void))
				{
					return_code=1;
					face_no=0;
					while (return_code&&(face_no<6)&&
						((environment_map->face_material)[face_no]=
						ACCESS(Graphical_material)(FIND_BY_IDENTIFIER_IN_MANAGER(Graphical_material,name)(
						state->current_token,graphical_material_manager)))&&
						shift_Parse_state(state,1))
					{
						face_no++;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_Environment_map_face_materials.  Invalid argument(s)");
					return_code=0;
				}
			}
			else
			{
				if ((environment_map=(struct Environment_map *)environment_map_void)&&
					(environment_map->face_material)[0]&&
					(environment_map->face_material)[1]&&
					(environment_map->face_material)[2]&&
					(environment_map->face_material)[3]&&
					(environment_map->face_material)[4]&&
					(environment_map->face_material)[5])
				{
					for (face_no=0;face_no<6;face_no++)
					{
						display_message(INFORMATION_MESSAGE," MAT_%d[%s]",face_no+1,
							Graphical_material_name(environment_map->face_material[face_no]));
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE,
		" MAT_1[none] MAT_2[none] MAT_3[none] MAT_4[none] MAT_5[none] MAT_6[none]");
				}
				return_code=1;
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,
				"Missing material name(s) for environment map");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Environment_map_face_materials.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Environment_map_face_materials */

int modify_Environment_map(struct Parse_state *state,void *environment_map_void,
	void *modify_environment_map_data_void)
/*******************************************************************************
LAST MODIFIED : 17 February 1997

DESCRIPTION :
==============================================================================*/
{
	struct Modifier_entry
		help_option_table[]=
		{
			{"ENVIRONMENT_MAP_NAME",NULL,NULL,modify_Environment_map},
			{NULL,NULL,NULL,NULL}
		},
		option_table[]=
		{
			{"face_materials",NULL,NULL,set_Environment_map_face_materials},
			{NULL,NULL,NULL,NULL}
		};
	const char *current_token;
	int process,return_code;
	struct Environment_map *environment_map_to_be_modified,
		*environment_map_to_be_modified_copy;
	struct Modify_environment_map_data *modify_environment_map_data;

	ENTER(modify_Environment_map);
	/* check the arguments */
	if (state)
	{
		current_token=state->current_token;
		if (current_token)
		{
			modify_environment_map_data=
				(struct Modify_environment_map_data *)modify_environment_map_data_void;
			if (modify_environment_map_data)
			{
				process=0;
				environment_map_to_be_modified=
					(struct Environment_map *)environment_map_void;
				if (environment_map_to_be_modified)
				{
					if (IS_MANAGED(Environment_map)(environment_map_to_be_modified,
						modify_environment_map_data->environment_map_manager))
					{
						environment_map_to_be_modified_copy = CREATE(Environment_map)((char *)NULL);
						if (environment_map_to_be_modified_copy)
						{
							MANAGER_COPY_WITH_IDENTIFIER(Environment_map,name)(
								environment_map_to_be_modified_copy,
								environment_map_to_be_modified);
							process=1;
						}
						else
						{
							display_message(ERROR_MESSAGE,
							"modify_Environment_map.  Could not create environment_map copy");
							return_code=0;
						}
					}
					else
					{
						environment_map_to_be_modified_copy=environment_map_to_be_modified;
						environment_map_to_be_modified=(struct Environment_map *)NULL;
						process=1;
					}
				}
				else
				{
					if (strcmp(PARSER_HELP_STRING,current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
					{
						environment_map_to_be_modified=FIND_BY_IDENTIFIER_IN_MANAGER(
							Environment_map,name)(current_token, modify_environment_map_data->environment_map_manager);
						if (environment_map_to_be_modified)
						{
							return_code=shift_Parse_state(state,1);
							if (return_code)
							{
								environment_map_to_be_modified_copy = CREATE(Environment_map)((char *)NULL);
								if (environment_map_to_be_modified_copy)
								{
									MANAGER_COPY_WITH_IDENTIFIER(Environment_map,name)(
										environment_map_to_be_modified_copy,
										environment_map_to_be_modified);
									process=1;
								}
								else
								{
									display_message(ERROR_MESSAGE,
							"modify_Environment_map.  Could not create environment_map copy");
									return_code=0;
								}
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"Unknown environment map: %s",
								current_token);
							display_parse_state_location(state);
							return_code=0;
						}
					}
					else
					{
						environment_map_to_be_modified =	CREATE(Environment_map)((char *)NULL);
						if (environment_map_to_be_modified)
						{
							(help_option_table[0]).to_be_modified=
								(void *)environment_map_to_be_modified;
							(help_option_table[0]).user_data=modify_environment_map_data_void;
							return_code=process_option(state,help_option_table);
							DESTROY(Environment_map)(&environment_map_to_be_modified);
						}
						else
						{
							display_message(ERROR_MESSAGE,
						"modify_Environment_map.  Could not create dummy environment map");
							return_code=0;
						}
					}
				}
				if (process)
				{
					(option_table[0]).to_be_modified=
						(void *)environment_map_to_be_modified_copy;
					(option_table[0]).user_data=
						modify_environment_map_data->graphical_material_manager;
					return_code=process_multiple_options(state,option_table);
					if (return_code)
					{
						if (environment_map_to_be_modified)
						{
							MANAGER_MODIFY_NOT_IDENTIFIER(Environment_map,name)(
								environment_map_to_be_modified,
								environment_map_to_be_modified_copy,
								modify_environment_map_data->environment_map_manager);
							DESTROY(Environment_map)(&environment_map_to_be_modified_copy);
						}
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"modify_Environment_map.  Missing modify_environment_map_data");
				return_code=0;
			}
		}
		else
		{
			if (environment_map_void)
			{
				display_message(WARNING_MESSAGE,
					"Missing environment map modifications");
			}
			else
			{
				display_message(WARNING_MESSAGE,"Missing environment map name");
			}
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"modify_Environment_map.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* modify_Environment_map */
