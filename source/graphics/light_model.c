/*******************************************************************************
FILE : light_model.c

LAST MODIFIED : 21 January 2002

DESCRIPTION :
The functions for manipulating light models.

???RC Only OpenGL is supported now.

???RC 3 December 97.
		Two functions are now used for activating the light_model:
- compile_Light_model puts the material into its own display list.
- execute_Light_model calls that display list.
		The separation into two functions was needed because OpenGL cannot start a
new list while one is being written to. As a consequence, you must precompile
all objects that are executed from within another display list. To make this job
easier, compile_Light_model is a list/manager iterator function.
		Future updates of OpenGL may overcome this limitation, in which case the
execute function can take over compiling as well. Furthermore, it is easy to
return to direct rendering, as described with these routines.
==============================================================================*/
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "command/parser.h"
#include "general/debug.h"
#include "general/list_private.h"
#include "general/manager_private.h"
#include "general/mystring.h"
#include "general/object.h"
#include "graphics/colour.h"
#include "graphics/graphics_library.h"
#include "graphics/light_model.h"
#include "user_interface/message.h"

/*
Module types
------------
*/
struct Light_model
/*******************************************************************************
LAST MODIFIED : 13 December 1997

DESCRIPTION :
The properties of a light model.
==============================================================================*/
{
	/* the name of the light model */
	char *name;
	/* flag for whether lighting is on */
	int enabled;
	/* ambient light in the scene not associated with any light */
	struct Colour ambient;
	/* For calculating specular lighting, a Local viewer take into account the */
	/* angle from the vertex to the viewer and is more accurate but slower than */
	/* an infinite viewer. */
	enum Light_model_viewer_mode viewer_mode;
	/* Two sided lighting lights the backs of the surfaces the same as the */
	/* front. One sided lighting does not. */
	enum Light_model_side_mode side_mode;
	GLuint display_list;
	int display_list_current;
	/* the number of structures that point to this light model. The light model
		cannot be destroyed while this is greater than 0 */
	int access_count;
}; /* struct Light_model */

FULL_DECLARE_LIST_TYPE(Light_model);

FULL_DECLARE_MANAGER_TYPE(Light_model);

/*
Module variables
----------------
*/

/*
Module functions
----------------
*/
DECLARE_LOCAL_MANAGER_FUNCTIONS(Light_model)

static int direct_render_Light_model(struct Light_model *light_model)
/*******************************************************************************
LAST MODIFIED : 13 December 1997

DESCRIPTION :
Activates the current light model as part of the rendering loop.
==============================================================================*/
{
	int return_code;
	GLfloat values[4];

	ENTER(direct_render_Light_model);
	if (light_model)
	{
		if (light_model->enabled)
		{
			values[0]=(light_model->ambient).red;
			values[1]=(light_model->ambient).green;
			values[2]=(light_model->ambient).blue;
			values[3]=1.;
			glLightModelfv(GL_LIGHT_MODEL_AMBIENT,values);
			if (LIGHT_MODEL_LOCAL_VIEWER==light_model->viewer_mode)
			{
				glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,GL_TRUE);
			}
			else
			{
				glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,GL_FALSE);
			}
			if (LIGHT_MODEL_TWO_SIDED==light_model->side_mode)
			{
				glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);
			}
			else
			{
				glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_FALSE);
			}
			glEnable(GL_LIGHTING);
		}
		else
		{
			glDisable(GL_LIGHTING);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"direct_render_Light_model.  Missing light model");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* direct_render_Light_model */

#if defined (OLD_CODE)
static int set_Light_model_two_sided(struct Parse_state *state,
	void *light_model_void,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 26 August 1996

DESCRIPTION :
Set the <light_model> to two-sided.
==============================================================================*/
{
	int return_code;
	struct Light_model *light_model;

	ENTER(set_Light_model_two_sided);
	if (state)
	{
		if (light_model=(struct Light_model *)light_model_void)
		{
			light_model->two_sided=1;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_Light_model_two_sided.  Missing light_model");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Light_model_two_sided.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Light_model_two_sided */

static int set_Light_model_single_sided(struct Parse_state *state,
	void *light_model_void,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 26 August 1996

DESCRIPTION :
Set the <light_model> to single sided.
==============================================================================*/
{
	int return_code;
	struct Light_model *light_model;

	ENTER(set_Light_model_single_sided);
	if (state)
	{
		if (light_model=(struct Light_model *)light_model_void)
		{
			light_model->two_sided=0;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_Light_model_single_sided.  Missing light_model");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Light_model_single_sided.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Light_model_single_sided */

static int set_Light_model_local_viewer(struct Parse_state *state,
	void *light_model_void,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 26 August 1996

DESCRIPTION :
Set the <light_model> to local viewer.
==============================================================================*/
{
	int return_code;
	struct Light_model *light_model;

	ENTER(set_Light_model_local_viewer);
	if (state)
	{
		if (light_model=(struct Light_model *)light_model_void)
		{
			light_model->local_viewer=1;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_Light_model_local_viewer.  Missing light_model");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Light_model_local_viewer.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Light_model_local_viewer */

static int set_Light_model_infinite_viewer(struct Parse_state *state,
	void *light_model_void,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 26 August 1996

DESCRIPTION :
Set the <light_model> to infinite viewer.
==============================================================================*/
{
	int return_code;
	struct Light_model *light_model;

	ENTER(set_Light_model_infinite_viewer);
	if (state)
	{
		if (light_model=(struct Light_model *)light_model_void)
		{
			light_model->local_viewer=0;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_Light_model_infinite_viewer.  Missing light_model");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Light_model_infinite_viewer.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Light_model_infinite_viewer */
#endif /* defined (OLD_CODE) */

/*
Global functions
----------------
*/
struct Light_model *CREATE(Light_model)(char *name)
/*******************************************************************************
LAST MODIFIED : 5 December 1997

DESCRIPTION :
Allocates memory and assigns fields for a light model.  Adds the light model to
the list of all light models.
==============================================================================*/
{
	struct Light_model *light_model;

	ENTER(CREATE(Light_model));
	if (name)
	{
		/* allocate memory for structure */
		if (ALLOCATE(light_model,struct Light_model,1)&&
			ALLOCATE(light_model->name,char,strlen(name)+1))
		{
			strcpy(light_model->name,name);
			light_model->access_count=0;
			light_model->enabled=1;
			(light_model->ambient).red=0.;
			(light_model->ambient).green=0.;
			(light_model->ambient).blue=0.;
			light_model->side_mode=LIGHT_MODEL_TWO_SIDED;
			light_model->viewer_mode=LIGHT_MODEL_INFINITE_VIEWER;
			light_model->display_list=0;
			light_model->display_list_current=0;
		}
		else
		{
			if (light_model)
			{
				DEALLOCATE(light_model);
			}
			display_message(ERROR_MESSAGE,
				"CREATE(Light_model).  Not enough memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Light_model).  Missing name");
		light_model=(struct Light_model *)NULL;
	}
	LEAVE;

	return (light_model);
} /* CREATE(Light_model) */

int DESTROY(Light_model)(struct Light_model **light_model_address)
/*******************************************************************************
LAST MODIFIED : 5 December 1997

DESCRIPTION :
Frees the memory for the light model and sets <*light_model_address> to NULL.
==============================================================================*/
{
	int return_code;
	struct Light_model *light_model;

	ENTER(DESTROY(Light_model));
	if (light_model_address&&(light_model= *light_model_address))
	{
		if (0==light_model->access_count)
		{
			DEALLOCATE(light_model->name);
			if (light_model->display_list)
			{
				glDeleteLists(light_model->display_list,1);
			}
			DEALLOCATE(*light_model_address);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"DESTROY(Light_model).  Non-zero access count!");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(Light_model).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Light_model) */

DECLARE_OBJECT_FUNCTIONS(Light_model)
DECLARE_DEFAULT_GET_OBJECT_NAME_FUNCTION(Light_model)

DECLARE_LIST_FUNCTIONS(Light_model)
DECLARE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Light_model,name,char *,strcmp)
DECLARE_LIST_IDENTIFIER_CHANGE_FUNCTIONS(Light_model,name)

PROTOTYPE_MANAGER_COPY_WITH_IDENTIFIER_FUNCTION(Light_model,name)
{
	char *name;
	int return_code;

	ENTER(MANAGER_COPY_WITH_IDENTIFIER(Light_model,name));
	/* check arguments */
	if (source&&destination)
	{
		if (source->name)
		{
			if (ALLOCATE(name,char,strlen(source->name)+1))
			{
				strcpy(name,source->name);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITH_IDENTIFIER(Light_model,name).  Insufficient memory");
				return_code=0;
			}
		}
		else
		{
			name=(char *)NULL;
			return_code=1;
		}
		if (return_code)
		{
			if (return_code = MANAGER_COPY_WITHOUT_IDENTIFIER(Light_model,name)(
				destination,source))
			{
				/* copy values */
				DEALLOCATE(destination->name);
				destination->name=name;
			}
			else
			{
				DEALLOCATE(name);
				display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITH_IDENTIFIER(Light_model,name).  Could not copy without identifier");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITH_IDENTIFIER(Light_model,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITH_IDENTIFIER(Light_model,name) */

PROTOTYPE_MANAGER_COPY_WITHOUT_IDENTIFIER_FUNCTION(Light_model,name)
{
	int return_code;

	ENTER(MANAGER_COPY_WITHOUT_IDENTIFIER(Light_model,name));
	/* check arguments */
	if (source&&destination)
	{
		/* copy values */
		(destination->ambient).red=(source->ambient).red;
		(destination->ambient).green=(source->ambient).green;
		(destination->ambient).blue=(source->ambient).blue;
		destination->enabled=source->enabled;
		destination->side_mode=source->side_mode;
		destination->viewer_mode=source->viewer_mode;
		destination->display_list_current=0;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITHOUT_IDENTIFIER(Light_model,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITHOUT_IDENTIFIER(Light_model,name) */

PROTOTYPE_MANAGER_COPY_IDENTIFIER_FUNCTION(Light_model,name,char *)
{
	char *destination_name;
	int return_code;

	ENTER(MANAGER_COPY_IDENTIFIER(Light_model,name));
	/* check arguments */
	if (name&&destination)
	{
		if (name)
		{
			if (ALLOCATE(destination_name,char,strlen(name)+1))
			{
				strcpy(destination_name,name);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
			"MANAGER_COPY_IDENTIFIER(Light_model,name).  Insufficient memory");
				return_code=0;
			}
		}
		else
		{
			name=(char *)NULL;
			return_code=1;
		}
		if (return_code)
		{
			/* copy name */
			DEALLOCATE(destination->name);
			destination->name=destination_name;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_IDENTIFIER(Light_model,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_IDENTIFIER(Light_model,name) */

DECLARE_MANAGER_FUNCTIONS(Light_model)

DECLARE_DEFAULT_MANAGED_OBJECT_NOT_IN_USE_FUNCTION(Light_model)

DECLARE_MANAGER_IDENTIFIER_FUNCTIONS(Light_model,name,char *)

int Light_model_get_ambient(struct Light_model *light_model,
	struct Colour *ambient)
/*******************************************************************************
LAST MODIFIED : 4 December 1997

DESCRIPTION :
Returns the ambient colour of the light_model.
==============================================================================*/
{
	int return_code;

	ENTER(Light_model_get_ambient);
	if (light_model&&ambient)
	{
		ambient->red=light_model->ambient.red;
		ambient->green=light_model->ambient.green;
		ambient->blue=light_model->ambient.blue;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Light_model_get_ambient.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Light_model_get_ambient */

int Light_model_set_ambient(struct Light_model *light_model,
	struct Colour *ambient)
/*******************************************************************************
LAST MODIFIED : 13 December 1997

DESCRIPTION :
Sets the ambient colour of the light_model.
==============================================================================*/
{
	int return_code;

	ENTER(Light_model_set_ambient);
	if (light_model&&ambient)
	{
		if ((ambient->red != light_model->ambient.red)||
			(ambient->green != light_model->ambient.green)||
			(ambient->blue != light_model->ambient.blue))
		{
			light_model->ambient.red=ambient->red;
			light_model->ambient.green=ambient->green;
			light_model->ambient.blue=ambient->blue;
			/* display list needs to be compiled again */
			light_model->display_list_current=0;
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Light_model_set_ambient.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Light_model_set_ambient */

int Light_model_get_status(struct Light_model *light_model)
/*******************************************************************************
LAST MODIFIED : 13 December 1997

DESCRIPTION :
Returns true if lighting is on for <light_model>.
==============================================================================*/
{
	int return_code;

	ENTER(Light_model_get_status);
	if (light_model)
	{
		return_code=light_model->enabled;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Light_model_get_status.  Missing light model");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Light_model_get_status */

int Light_model_set_status(struct Light_model *light_model,int enabled)
/*******************************************************************************
LAST MODIFIED : 13 December 1997

DESCRIPTION :
Sets whether lighting is on in <light_model>. On if <enabled> non-zero.
==============================================================================*/
{
	int return_code;

	ENTER(Light_model_set_status);
	if (light_model)
	{
		if (enabled)
		{
			enabled=1;
		}
		if (enabled != light_model->enabled)
		{
			light_model->enabled=enabled;
			/* display list needs to be compiled again */
			light_model->display_list_current=0;
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Light_model_set_status.  Missing light model");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Light_model_set_status */

int Light_model_get_side_mode(struct Light_model *light_model,
	enum Light_model_side_mode *side_mode)
/*******************************************************************************
LAST MODIFIED : 4 December 1997

DESCRIPTION :
Returns the side_mode of the light_model (local/infinite).
==============================================================================*/
{
	int return_code;

	ENTER(Light_model_get_side_mode);
	if (light_model&&side_mode)
	{
		*side_mode=light_model->side_mode;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Light_model_get_side_mode.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Light_model_get_side_mode */

int Light_model_set_side_mode(struct Light_model *light_model,
	enum Light_model_side_mode side_mode)
/*******************************************************************************
LAST MODIFIED : 13 December 1997

DESCRIPTION :
Sets the side_mode of the light_model (local/infinite).
==============================================================================*/
{
	int return_code;

	ENTER(Light_model_set_side_mode);
	if (light_model&&((LIGHT_MODEL_ONE_SIDED==side_mode)||
		(LIGHT_MODEL_TWO_SIDED==side_mode)))
	{
		if (side_mode != light_model->side_mode)
		{
			light_model->side_mode=side_mode;
			/* display list needs to be compiled again */
			light_model->display_list_current=0;
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Light_model_set_side_mode.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Light_model_set_side_mode */

int Light_model_get_viewer_mode(struct Light_model *light_model,
	enum Light_model_viewer_mode *viewer_mode)
/*******************************************************************************
LAST MODIFIED : 4 December 1997

DESCRIPTION :
Returns the viewer_mode of the light_model (local/infinite).
==============================================================================*/
{
	int return_code;

	ENTER(Light_model_get_viewer_mode);
	if (light_model&&viewer_mode)
	{
		*viewer_mode=light_model->viewer_mode;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Light_model_get_viewer_mode.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Light_model_get_viewer_mode */

int Light_model_set_viewer_mode(struct Light_model *light_model,
	enum Light_model_viewer_mode viewer_mode)
/*******************************************************************************
LAST MODIFIED : 13 December 1997

DESCRIPTION :
Sets the viewer_mode of the light_model (local/infinite).
==============================================================================*/
{
	int return_code;

	ENTER(Light_model_set_viewer_mode);
	if (light_model&&((LIGHT_MODEL_LOCAL_VIEWER==viewer_mode)||
		(LIGHT_MODEL_INFINITE_VIEWER==viewer_mode)))
	{
		if (viewer_mode != light_model->viewer_mode)
		{
			light_model->viewer_mode=viewer_mode;
			/* display list needs to be compiled again */
			light_model->display_list_current=0;
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Light_model_set_viewer_mode.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Light_model_set_viewer_mode */

int modify_Light_model(struct Parse_state *state,void *light_model_void,
	void *modify_light_model_data_void)
/*******************************************************************************
LAST MODIFIED : 13 December 1997

DESCRIPTION :
Modifies the properties of a light model.
==============================================================================*/
{
	auto struct Modifier_entry
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
	char *current_token,infinite_viewer_flag,local_viewer_flag,
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
		if (modify_light_model_data=(struct Modify_light_model_data *)
			modify_light_model_data_void)
		{
			if (current_token=state->current_token)
			{
				process=0;
				if (light_model_to_be_modified=(struct Light_model *)light_model_void)
				{
					if (IS_MANAGED(Light_model)(light_model_to_be_modified,
						modify_light_model_data->light_model_manager))
					{
						if (light_model_to_be_modified_copy=CREATE(Light_model)("copy"))
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
						if (light_model_to_be_modified=FIND_BY_IDENTIFIER_IN_MANAGER(
							Light_model,name)(current_token,
							modify_light_model_data->light_model_manager))
						{
							if (return_code=shift_Parse_state(state,1))
							{
								if (light_model_to_be_modified_copy=CREATE(Light_model)("copy"))
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
						if (light_model_to_be_modified=CREATE(Light_model)("help"))
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
					if (return_code=process_multiple_options(state,option_table))
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

int list_Light_model(struct Light_model *light_model,void *dummy)
/*******************************************************************************
LAST MODIFIED : 22 December 2000

DESCRIPTION :
Writes the properties of the <light_model> to the command window.
==============================================================================*/
{
	char line[80];
	int return_code;

	ENTER(list_Light_model);
	USE_PARAMETER(dummy);
	if (light_model)
	{
		display_message(INFORMATION_MESSAGE,"light model : ");
		display_message(INFORMATION_MESSAGE,light_model->name);
		display_message(INFORMATION_MESSAGE,"\n");
		if (light_model->enabled)
		{
			display_message(INFORMATION_MESSAGE,"  lighting enabled\n");
			if (LIGHT_MODEL_LOCAL_VIEWER==light_model->viewer_mode)
			{
				display_message(INFORMATION_MESSAGE,"  local view point\n");
			}
			else
			{
				display_message(INFORMATION_MESSAGE,"  infinite view point\n");
			}
			if (LIGHT_MODEL_TWO_SIDED==light_model->side_mode)
			{
				display_message(INFORMATION_MESSAGE,"  two-sided\n");
			}
			else
			{
				display_message(INFORMATION_MESSAGE,"  one-sided\n");
			}
			sprintf(line,"  ambient.  red = %.3g, green = %.3g, blue = %.3g\n",
				(light_model->ambient).red,(light_model->ambient).green,
				(light_model->ambient).blue);
			display_message(INFORMATION_MESSAGE,line);
		}
		else
		{
			display_message(INFORMATION_MESSAGE,"  lighting disabled\n");
		}
		display_message(INFORMATION_MESSAGE,"  access count = %d\n",
			light_model->access_count);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"list_Light_model.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* list_Light_model */

int set_Light_model(struct Parse_state *state,
	void *light_model_address_void,void *light_model_manager_void)
/*******************************************************************************
LAST MODIFIED : 13 December 1997

DESCRIPTION :
Modifier function to set the light model from a command.
???RC set_Object routines could become a macro.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Light_model *temp_light_model,**light_model_address;
	struct MANAGER(Light_model) *light_model_manager;

	ENTER(set_Light_model);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if ((light_model_address=
					(struct Light_model **)light_model_address_void)&&
					(light_model_manager=(struct MANAGER(Light_model) *)
					light_model_manager_void))
				{
					if (temp_light_model=FIND_BY_IDENTIFIER_IN_MANAGER(Light_model,name)(
						current_token,light_model_manager))
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
				if (light_model_address=
					(struct Light_model **)light_model_address_void)
				{
					if (temp_light_model= *light_model_address)
					{
						display_message(INFORMATION_MESSAGE,"[%s]",temp_light_model->name);
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

#if defined (OLD_CODE)
int activate_Light_model(void)
/*******************************************************************************
LAST MODIFIED : 2 May 1995

DESCRIPTION :
Activates the current light model as part of the rendering loop.
==============================================================================*/
{
	int return_code;
#if defined (GL_API)
	int number_of_values;
	float values[14];
#endif
#if defined (OPENGL_API)
	GLfloat values[4];
#endif

	ENTER(activate_Light_model);
/*  if (graphics_library_open)
	{*/
		if (current_light_model)
		{
			/* lighting */
#if defined (GL_API)
			if (current_light_model->index<=0)
			{
				if (current_light_model->index<0)
				{
					current_light_model->index= -(current_light_model->index);
				}
				else
				{
					current_light_model->index=next_light_model_index;
					next_light_model_index++;
				}
				number_of_values=0;
				values[number_of_values]=AMBIENT;
				number_of_values++;
				values[number_of_values]=(current_light_model->ambient).red;
				number_of_values++;
				values[number_of_values]=(current_light_model->ambient).green;
				number_of_values++;
				values[number_of_values]=(current_light_model->ambient).blue;
				number_of_values++;
				values[number_of_values]=LOCALVIEWER;
				number_of_values++;
				if (current_light_model->local_viewer)
				{
					values[number_of_values]=1.;
				}
				else
				{
					values[number_of_values]=0.;
				}
				number_of_values++;
#if defined (TWOSIDE)
				values[number_of_values]=TWOSIDE;
				number_of_values++;
				if (current_light_model->two_sided)
				{
					values[number_of_values]=1.;
				}
				else
				{
					values[number_of_values]=0.;
				}
				number_of_values++;
#endif
				/*???DB.  No attenuation at present */
				values[number_of_values]=ATTENUATION;
				number_of_values++;
				values[number_of_values]=1.;
				number_of_values++;
				values[number_of_values]=0.;
				number_of_values++;
#if defined (ATTENUATION2)
				values[number_of_values]=ATTENUATION2;
				number_of_values++;
				values[number_of_values]=0.;
				number_of_values++;
#endif
				values[number_of_values]=LMNULL;
				number_of_values++;
				lmdef(DEFLMODEL,current_light_model->index,number_of_values,values);
			}
			if (current_light_model->index>0)
			{
				lmbind(LMODEL,current_light_model->index);
			}
#endif
#if defined (OPENGL_API)
#if defined (MS_22AUG96)
			if (0==current_light_model->list_index)
			{
				if (current_light_model->list_index=glGenLists(1))
				{
					glNewList(current_light_model->list_index,GL_COMPILE);
#endif /* defined (MS_22AUG96) */
					values[0]=(current_light_model->ambient).red;
					values[1]=(current_light_model->ambient).green;
					values[2]=(current_light_model->ambient).blue;
					values[3]=1.;
					glLightModelfv(GL_LIGHT_MODEL_AMBIENT,values);
					if (current_light_model->local_viewer)
					{
						glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,GL_TRUE);
					}
					else
					{
						glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,GL_FALSE);
					}
					if (current_light_model->two_sided)
					{
						glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_TRUE);
					}
					else
					{
						glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,GL_FALSE);
					}
					glEnable(GL_LIGHTING);
#if defined (MS_22AUG96)
					glEndList();
				}
			}
			if (current_light_model->list_index)
			{
				glCallList(current_light_model->list_index);
			}
#endif /* defined (MS_22AUG96) */
#endif
#if defined (GL_API)
#endif
#if defined (OPENGL_API)
#endif
			return_code=1;
		}
		else
		{
			/* no lighting */
#if defined (GL_API)
			lmbind(LMODEL,0);
#endif
#if defined (OPENGL_API)
			glDisable(GL_LIGHTING);
#endif
			return_code=1;
		}
/*  }
	else
	{
		return_code=0;
	}*/
	LEAVE;

	return (return_code);
} /* activate_Light_model */
#endif /* defined (OLD_CODE) */

int compile_Light_model(struct Light_model *light_model)
/*******************************************************************************
LAST MODIFIED : 3 December 1997

DESCRIPTION :
Rebuilds the display_list for <light_model> if it is not current. If
<light_model>does not have a display list, first attempts to give it one. The
display list created here may be called using execute_Light_model, below.
???RC Light_models must be compiled before they are executed since openGL
cannot start writing to a display list when one is currently being written to.
???RC The behaviour of light_models is set up to take advantage of pre-computed
display lists. To switch to direct rendering make this routine do nothing and
execute_Light_model should just call direct_render_Light_model.
==============================================================================*/
{
	int return_code;

	ENTER(compile_Light_model);
	if (light_model)
	{
		if (!light_model->display_list_current)
		{
			if (light_model->display_list||(light_model->display_list=glGenLists(1)))
			{
				glNewList(light_model->display_list,GL_COMPILE);
				direct_render_Light_model(light_model);
				glEndList();
				light_model->display_list_current=1;
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"compile_Light_model.  Could not generate display list");
				return_code=0;
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
			"compile_Light_model.  Missing light_model");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* compile_Light_model */

int execute_Light_model(struct Light_model *light_model)
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Activates <light_model> by calling its display list. If the display list is not
current, an error is reported.
???RC The behaviour of light_models is set up to take advantage of pre-computed
display lists. To switch to direct rendering this routine should just call
direct_render_Light_model.
==============================================================================*/
{
	int return_code;

	ENTER(execute_Light_model);
	if (light_model)
	{
		if (light_model->display_list_current)
		{
			glCallList(light_model->display_list);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_Light_model.  display list not current");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_Light_model.  Missing light_model");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_Light_model */
