/*******************************************************************************
FILE : light.c

LAST MODIFIED : 9 October 2002

DESCRIPTION :
The functions for manipulating lights.

???RC
If OpenGL lights were set up half-way decently you could throw them all into
display lists in compile_Light and then execute them with execute_Light.
However, the basic installation supports only EIGHT lights, but worse, they are
each referenced by their own constants GL_LIGHT0..GL_LIGHT7. Hence, if you want
all available lights in differing combinations in different windows at
different times,you can't use display lists since the light identifier would
be hardwired in them.
Hence, this compile_Light does nothing, while the execute_Light routine just
calls direct_render_Light. Before any lights are activated, however, you must
call reset_Lights to turn all the lights off and set next_light_no to zero.
Future improvements to OpenGL or other graphics libraries may overcome this
problem.
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
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "command/parser.h"
#include "general/debug.h"
#include "general/enumerator_private.h"
#include "general/list_private.h"
#include "general/manager_private.h"
#include "general/mystring.h"
#include "general/object.h"
#include "graphics/graphics_library.h"
#include "graphics/light.h"
#include "user_interface/message.h"

/*
Module types
------------
*/

struct Light
/*******************************************************************************
LAST MODIFIED : 8 October 2002

DESCRIPTION :
The properties of a light.
==============================================================================*/
{
	/* the name of the light */
	char *name;
	/* light type: infinite, point or spot */
	enum Light_type type;
	struct Colour colour;
	/* position for point and spot lights */
	float position[3];
	/* direction for infinite (directional) and spot lights */
	float direction[3];
	/* attenuation parameters control light falloff with distance according to
	   1/(c + l*d + q*d*d) where d=distance, c=constant, l=linear, q=quadratic */
	float constant_attenuation, linear_attenuation, quadratic_attenuation;
	/* the angle, in degrees, between the direction of the spotlight and the ray
		along the edge of the light cone (between 0 and 90 inclusive) */
	float spot_cutoff;
	/* spot_exponent controls concentration of light near its axis; 0 = none */
	float spot_exponent;
	/* include display list stuff although cannot use at present */
	GLuint display_list;
	int display_list_current;
	/* the number of structures that point to this light.  The light cannot be
		destroyed while this is greater than 0 */
	int access_count;
}; /* struct Light */

FULL_DECLARE_LIST_TYPE(Light);

FULL_DECLARE_MANAGER_TYPE(Light);

/*
Module variables
----------------
*/

static int next_light_no=0;
#define MAXIMUM_NUMBER_OF_ACTIVE_LIGHTS 8
static GLenum light_identifiers[MAXIMUM_NUMBER_OF_ACTIVE_LIGHTS]=
{
	GL_LIGHT0,GL_LIGHT1,GL_LIGHT2,GL_LIGHT3,GL_LIGHT4,GL_LIGHT5,GL_LIGHT6,
	GL_LIGHT7
};

/*
Module functions
----------------
*/

DECLARE_LOCAL_MANAGER_FUNCTIONS(Light)

static int direct_render_Light(struct Light *light)
/*******************************************************************************
LAST MODIFIED : 8 October 2002

DESCRIPTION :
Directly outputs the commands to activate the <light>.
==============================================================================*/
{
	int return_code;
	GLenum light_id;
	GLfloat values[4];

	ENTER(direct_render_Light);
	if (light)
	{
		if (next_light_no < MAXIMUM_NUMBER_OF_ACTIVE_LIGHTS)
		{
			light_id = light_identifiers[next_light_no];
			values[0] = 0.;
			values[1] = 0.;
			values[2] = 0.;
			values[3] = 1.;
			glLightfv(light_id, GL_AMBIENT, values);
			values[0] = (GLfloat)((light->colour).red);
			values[1] = (GLfloat)((light->colour).green);
			values[2] = (GLfloat)((light->colour).blue);
			glLightfv(light_id, GL_DIFFUSE, values);
			glLightfv(light_id, GL_SPECULAR, values);
			switch (light->type)
			{
				case INFINITE_LIGHT:
				{
					values[0] = -light->direction[0];
					values[1] = -light->direction[1];
					values[2] = -light->direction[2];
					values[3] = 0.;
					glLightfv(light_id, GL_POSITION, values);
					glLightf(light_id, GL_SPOT_EXPONENT, 0.);
					glLightf(light_id, GL_SPOT_CUTOFF, 180.);
					glLightf(light_id, GL_CONSTANT_ATTENUATION, 1.);
					glLightf(light_id, GL_LINEAR_ATTENUATION, 0.);
					glLightf(light_id, GL_QUADRATIC_ATTENUATION, 0.);
				} break;
				case POINT_LIGHT:
				{
					values[0] = light->position[0];
					values[1] = light->position[1];
					values[2] = light->position[2];
					values[3] = 1.;
					glLightfv(light_id, GL_POSITION, values);
					glLightf(light_id, GL_SPOT_EXPONENT, 0.);
					glLightf(light_id, GL_SPOT_CUTOFF, 180.);
					glLightf(light_id, GL_CONSTANT_ATTENUATION,
						(GLfloat)(light->constant_attenuation));
					glLightf(light_id, GL_LINEAR_ATTENUATION,
						(GLfloat)(light->linear_attenuation));
					glLightf(light_id, GL_QUADRATIC_ATTENUATION,
						(GLfloat)(light->quadratic_attenuation));
				} break;
				case SPOT_LIGHT:
				{
					values[0] = light->position[0];
					values[1] = light->position[1];
					values[2] = light->position[2];
					values[3] = 1.;
					glLightfv(light_id,GL_POSITION,values);
					values[0] = light->direction[0];
					values[1] = light->direction[1];
					values[2] = light->direction[2];
					glLightfv(light_id, GL_SPOT_DIRECTION, values);
					glLightf(light_id, GL_SPOT_EXPONENT, (GLfloat)(light->spot_exponent));
					glLightf(light_id, GL_SPOT_CUTOFF, (GLfloat)(light->spot_cutoff));
					glLightf(light_id, GL_CONSTANT_ATTENUATION,
						(GLfloat)(light->constant_attenuation));
					glLightf(light_id, GL_LINEAR_ATTENUATION,
						(GLfloat)(light->linear_attenuation));
					glLightf(light_id, GL_QUADRATIC_ATTENUATION,
						(GLfloat)(light->quadratic_attenuation));
				} break;
			}
			glEnable(light_id);
			return_code = 1;
			next_light_no++;
		}
		else
		{
			display_message(ERROR_MESSAGE, "Cannot have any more lights on");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"direct_render_Light.  Missing light");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* direct_render_Light */

#if defined (OLD_CODE)
static int set_Light_on(struct Parse_state *state,void *light_void,
	void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 19 February 1997

DESCRIPTION :
Set the <light> on.
==============================================================================*/
{
	int return_code;
	struct Light *light;

	ENTER(set_Light_on);
	if (state)
	{
		if (light=(struct Light *)light_void)
		{
			light->on=1;
#if defined (OLD_CODE)
#if defined (GL_API) || defined (OPENGL_API)
			if (!(light->on))
			{
				if (next_light_no<MAXIMUM_NUMBER_OF_ACTIVE_LIGHTS)
				{
					light->on=1;
					light->identifier=light_identifiers[next_light_no];
#if defined (GL_API)
					light->index=light_indicies[next_light_no];
#endif
					next_light_no++;
				}
				else
				{
					display_message(ERROR_MESSAGE,"Cannot have any more lights on");
				}
			}
#else
			light->on=1;
#endif
#endif /* defined (OLD_CODE) */
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,"set_Light_on.  Missing light");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Light_on.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Light_on */

static int set_Light_off(struct Parse_state *state,void *light_void,
	void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 19 February 1997

DESCRIPTION :
Set the <light> off.
==============================================================================*/
{
	int return_code;
	struct Light *light;

	ENTER(set_Light_off);
	if (state)
	{
		if (light=(struct Light *)light_void)
		{
			light->on=0;
			return_code=1;
#if defined (OLD_CODE)
#if defined (GL_API) || defined (OPENGL_API)
			if (light->on)
			{
				light->on=0;
				if (0<next_light_no)
				{
					next_light_no--;
					light_identifiers[next_light_no]=light->identifier;
#if defined (GL_API)
					light_indicies[next_light_no]=light->index;
#endif
				}
				else
				{
					display_message(ERROR_MESSAGE,"set_Light_off.  next_light_no<=0");
				}
			}
			else
			{
				return_code=1;
			}
#else
			light->on=0;
			return_code=1;
#endif
#endif /* defined (OLD_CODE) */
		}
		else
		{
			display_message(ERROR_MESSAGE,"set_Light_off.  Missing light");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Light_off.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Light_off */

static int set_Light_model_coordinates(struct Parse_state *state,
	void *light_void,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 26 August 1996

DESCRIPTION :
Set the <light> to model_coordinates coordinates.
==============================================================================*/
{
	int return_code;
	struct Light *light;

	ENTER(set_Light_model_coordinates);
	if (state)
	{
		if (light=(struct Light *)light_void)
		{
			light->model_coordinates=1;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_Light_model_coordinates.  Missing light");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Light_model_coordinates.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Light_model_coordinates */

static int set_Light_world_coordinates(struct Parse_state *state,
	void *light_void,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 26 August 1996

DESCRIPTION :
Set the <light> to world_coordinates coordinates.
==============================================================================*/
{
	int return_code;
	struct Light *light;

	ENTER(set_Light_world_coordinates);
	if (state)
	{
		if (light=(struct Light *)light_void)
		{
			light->model_coordinates=0;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_Light_world_coordinates.  Missing light");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Light_world_coordinates.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Light_world_coordinates */

static int set_Light_infinite(struct Parse_state *state,void *light_void,
	void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 26 August 1996

DESCRIPTION :
Makes a infinite <light>.
==============================================================================*/
{
	int return_code;
	struct Light *light;

	ENTER(set_Light_infinite);
	if (state)
	{
		if (light=(struct Light *)light_void)
		{
			light->type=INFINITE_LIGHT;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,"set_Light_infinite.  Missing light");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Light_infinite.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Light_infinite */

static int set_Light_point(struct Parse_state *state,void *light_void,
	void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 26 August 1996

DESCRIPTION :
Makes a point <light>.
==============================================================================*/
{
	int return_code;
	struct Light *light;

	ENTER(set_Light_point);
	if (state)
	{
		if (light=(struct Light *)light_void)
		{
			light->type=POINT_LIGHT;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,"set_Light_point.  Missing light");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Light_point.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Light_point */

static int set_Light_spot(struct Parse_state *state,void *light_void,
	void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 26 August 1996

DESCRIPTION :
Makes a spot <light>.
==============================================================================*/
{
	int return_code;
	struct Light *light;

	ENTER(set_Light_spot);
	if (state)
	{
		if (light=(struct Light *)light_void)
		{
			light->type=SPOT_LIGHT;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,"set_Light_spot.  Missing light");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Light_spot.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Light_spot */

static int set_Light_position(struct Parse_state *state,void *light_void,
	void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 4 October 1996

DESCRIPTION :
Set the <light> position from the command options.
==============================================================================*/
{
	char *current_token;
	float x,y,z;
	int return_code;
	struct Light *light;

	ENTER(set_Light_position);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (light=(struct Light *)light_void)
				{
					if ((1==sscanf(current_token," %f ",&x))&&shift_Parse_state(state,1)&&
						(1==sscanf( state->current_token," %f ",&y))&&
						shift_Parse_state(state,1)&&
						(1==sscanf(state->current_token," %f ",&z)))
					{
						light->position.x=x;
						light->position.y=y;
						light->position.z=z;
						return_code=shift_Parse_state(state,1);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Missing coordinate(s) for light position");
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"set_Light_position.  Missing light");
					return_code=0;
				}
			}
			else
			{
				if (light=(struct Light *)light_void)
				{
					display_message(INFORMATION_MESSAGE," X#[%g] Y#[%g] Z#[%g]",
						(light->position).x,(light->position).y,(light->position).z);
				}
				else
				{
					display_message(INFORMATION_MESSAGE," X# Y# Z#");
				}
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing light position");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Light_position.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Light_position */

static int set_Light_direction(struct Parse_state *state,void *light_void,
	void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 4 October 1996

DESCRIPTION :
Set the <light> direction from the command options.
==============================================================================*/
{
	char *current_token;
	float x,y,z;
	int return_code;
	struct Light *light;

	ENTER(set_Light_direction);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (light=(struct Light *)light_void)
				{
					if ((1==sscanf(current_token," %f ",&x))&&shift_Parse_state(state,1)&&
						(1==sscanf( state->current_token," %f ",&y))&&
						shift_Parse_state(state,1)&&
						(1==sscanf(state->current_token," %f ",&z)))
					{
						light->direction.x=x;
						light->direction.y=y;
						light->direction.z=z;
						return_code=shift_Parse_state(state,1);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Missing coordinate(s) for light direction");
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"set_Light_direction.  Missing light");
					return_code=0;
				}
			}
			else
			{
				if (light=(struct Light *)light_void)
				{
					display_message(INFORMATION_MESSAGE," X#[%g] Y#[%g] Z#[%g]",
						light->direction.x,light->direction.y,light->direction.z);
				}
				else
				{
					display_message(INFORMATION_MESSAGE," X# Y# Z#");
				}
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing light direction");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Light_direction.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Light_direction */

static int set_Light_cutoff(struct Parse_state *state,void *light_void,
	void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 4 October 1996

DESCRIPTION :
Set the <light> cutoff from the command options.
==============================================================================*/
{
	char *current_token;
	float cutoff;
	int return_code;
	struct Light *light;

	ENTER(set_Light_cutoff);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (light=(struct Light *)light_void)
				{
					if (1==sscanf(current_token," %f ",&cutoff))
					{
						/* restrict to between 0 and 90 degrees */
						if (cutoff<0)
						{
							cutoff=0;
						}
						else
						{
							if (cutoff>90)
							{
								cutoff=90;
							}
						}
						light->cutoff=cutoff;
						return_code=shift_Parse_state(state,1);
					}
					else
					{
						display_message(ERROR_MESSAGE,"Invalid cut-off angle : %s",
							current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"set_Light_cutoff.  Missing light");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," ANGLE#");
				if (light=(struct Light *)light_void)
				{
					display_message(INFORMATION_MESSAGE,"[%g]",light->cutoff);
				}
				display_message(INFORMATION_MESSAGE,"{degrees}");
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing cut-off angle");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Light_cutoff.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Light_cutoff */
#endif /* defined (OLD_CODE) */

/*
Global functions
----------------
*/

PROTOTYPE_ENUMERATOR_STRING_FUNCTION(Light_type)
{
	char *enumerator_string;

	ENTER(ENUMERATOR_STRING(Light_type));
	switch (enumerator_value)
	{
		case INFINITE_LIGHT:
		{
			enumerator_string = "infinite";
		} break;
		case POINT_LIGHT:
		{
			enumerator_string = "point";
		} break;
		case SPOT_LIGHT:
		{
			enumerator_string = "spot";
		} break;
		default:
		{
			enumerator_string = (char *)NULL;
		} break;
	}
	LEAVE;

	return (enumerator_string);
} /* ENUMERATOR_STRING(Light_type) */

DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS(Light_type)

struct Light *CREATE(Light)(char *name)
/*******************************************************************************
LAST MODIFIED : 8 October 2002

DESCRIPTION :
Allocates memory and assigns fields for a light model.
==============================================================================*/
{
	struct Light *light;

	ENTER(CREATE(Light));
	if (name)
	{
		/* allocate memory for structure */
		if (ALLOCATE(light, struct Light, 1) &&
			(light->name = duplicate_string(name)))
		{
			strcpy(light->name, name);
			light->access_count = 0;
			light->type=INFINITE_LIGHT;
			(light->colour).red = 1.;
			(light->colour).green = 1.;
			(light->colour).blue = 1.;
			light->position[0] = 0;
			light->position[1] = 0;
			light->position[2] = 0;
			light->direction[0] = 0;
			light->direction[1] = 0;
			light->direction[2] = -1;
			light->constant_attenuation = 1.0;
			light->linear_attenuation = 0.0;
			light->quadratic_attenuation = 0.0;
			light->spot_cutoff = 90.0;
			light->spot_exponent = 0.0;
			light->display_list=0;
			light->display_list_current=0;
		}
		else
		{
			if (light)
			{
				DEALLOCATE(light);
			}
			display_message(ERROR_MESSAGE,"CREATE(Light).  Insufficient memory");
			DEALLOCATE(light);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Light).  Missing name");
		light=(struct Light *)NULL;
	}
	LEAVE;

	return (light);
} /* CREATE(Light) */

int DESTROY(Light)(struct Light **light_address)
/*******************************************************************************
LAST MODIFIED : 4 December 1997

DESCRIPTION :
Frees the memory for the light and sets <*light_address> to NULL.
==============================================================================*/
{
	int return_code;
	struct Light *light;

	ENTER(DESTROY(Light));
	if (light_address&&(light= *light_address))
	{
		if (0==light->access_count)
		{
			DEALLOCATE(light->name);
			if (light->display_list)
			{
				glDeleteLists(light->display_list,1);
			}
			DEALLOCATE(*light_address);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,"DESTROY(Light).  Non-zero access count!");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(Light).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Light) */

DECLARE_OBJECT_FUNCTIONS(Light)
DECLARE_DEFAULT_GET_OBJECT_NAME_FUNCTION(Light)

DECLARE_LIST_FUNCTIONS(Light)
DECLARE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Light,name,char *,strcmp)
DECLARE_LIST_IDENTIFIER_CHANGE_FUNCTIONS(Light,name)

PROTOTYPE_MANAGER_COPY_WITH_IDENTIFIER_FUNCTION(Light,name)
{
	char *name;
	int return_code;

	ENTER(MANAGER_COPY_WITH_IDENTIFIER(Light,name));
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
"MANAGER_COPY_WITH_IDENTIFIER(Light,name).  Insufficient memory");
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
			if (return_code = MANAGER_COPY_WITHOUT_IDENTIFIER(Light,name)(
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
"MANAGER_COPY_WITH_IDENTIFIER(Light,name).  Could not copy without identifier");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITH_IDENTIFIER(Light,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITH_IDENTIFIER(Light,name) */

PROTOTYPE_MANAGER_COPY_WITHOUT_IDENTIFIER_FUNCTION(Light,name)
{
	int return_code;

	ENTER(MANAGER_COPY_WITHOUT_IDENTIFIER(Light,name));
	if (source && destination)
	{
		/* copy values */
		destination->type = source->type;
		(destination->colour).red = (source->colour).red;
		(destination->colour).green = (source->colour).green;
		(destination->colour).blue = (source->colour).blue;
		destination->position[0] = source->position[0];
		destination->position[1] = source->position[1];
		destination->position[2] = source->position[2];
		destination->direction[0] = source->direction[0];
		destination->direction[1] = source->direction[1];
		destination->direction[2] = source->direction[2];
		destination->constant_attenuation = source->constant_attenuation;
		destination->linear_attenuation = source->linear_attenuation;
		destination->quadratic_attenuation = source->quadratic_attenuation;
		destination->spot_cutoff = source->spot_cutoff;
		destination->spot_exponent = source->spot_exponent;
		destination->display_list_current = 0;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_WITHOUT_IDENTIFIER(Light,name).  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITHOUT_IDENTIFIER(Light,name) */

PROTOTYPE_MANAGER_COPY_IDENTIFIER_FUNCTION(Light,name,char *)
{
	char *destination_name;
	int return_code;

	ENTER(MANAGER_COPY_IDENTIFIER(Light,name));
	if (name&&destination)
	{
		if (ALLOCATE(destination_name,char,strlen(name)+1))
		{
			strcpy(destination_name,name);
			if (destination->name)
			{
				DEALLOCATE(destination->name);
			}
			destination->name=destination_name;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"MANAGER_COPY_IDENTIFIER(Light,name).  Insufficient memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_IDENTIFIER(Light,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_IDENTIFIER(Light,name) */

DECLARE_MANAGER_FUNCTIONS(Light)

DECLARE_DEFAULT_MANAGED_OBJECT_NOT_IN_USE_FUNCTION(Light)

DECLARE_MANAGER_IDENTIFIER_FUNCTIONS(Light,name,char *)

int get_Light_attenuation(struct Light *light, float *constant_attenuation,
	float *linear_attenuation, float *quadratic_attenuation)
/*******************************************************************************
LAST MODIFIED : 8 October 2002

DESCRIPTION :
Returns the constant, linear and quadratic attentuation factors which control
the falloff in intensity of <light> according to 1/(c + l*d + q*d*d)
where d=distance, c=constant, l=linear, q=quadratic.
Values 1 0 0, ie. only constant attenuation of 1 gives no attenuation.
Infinite/directional lights are not affected by these values.
==============================================================================*/
{
	int return_code;

	ENTER(get_Light_attenuation);
	if (light && constant_attenuation && linear_attenuation &&
		quadratic_attenuation)
	{
		*constant_attenuation = light->constant_attenuation;
		*linear_attenuation = light->linear_attenuation;
		*quadratic_attenuation = light->quadratic_attenuation;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_Light_attenuation.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* get_Light_attenuation */

int set_Light_attenuation(struct Light *light, float constant_attenuation,
	float linear_attenuation, float quadratic_attenuation)
/*******************************************************************************
LAST MODIFIED : 9 October 2002

DESCRIPTION :
Sets the constant, linear and quadratic attentuation factors which control
the falloff in intensity of <light> according to 1/(c + l*d + q*d*d)
where d=distance, c=constant, l=linear, q=quadratic.
Values 1 0 0, ie. only constant attenuation of 1 gives no attenuation.
Infinite/directional lights are not affected by these values.
==============================================================================*/
{
	int return_code;

	ENTER(set_Light_attenuation);
	if (light && (0.0 <= constant_attenuation) && (0.0 <= linear_attenuation) &&
		(0.0 <= quadratic_attenuation))
	{
		light->constant_attenuation = constant_attenuation;
		light->linear_attenuation = linear_attenuation;
		light->quadratic_attenuation = quadratic_attenuation;
		/* display list needs to be compiled again */
		light->display_list_current = 0;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Light_attenuation.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* set_Light_attenuation */

int get_Light_colour(struct Light *light,struct Colour *colour)
/*******************************************************************************
LAST MODIFIED : 4 December 1997

DESCRIPTION :
Returns the colour of the light.
==============================================================================*/
{
	int return_code;

	ENTER(get_Light_colour);
	if (light&&colour)
	{
		colour->red=light->colour.red;
		colour->green=light->colour.green;
		colour->blue=light->colour.blue;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_Light_colour.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* get_Light_colour */

int set_Light_colour(struct Light *light,struct Colour *colour)
/*******************************************************************************
LAST MODIFIED : 4 December 1997

DESCRIPTION :
Sets the colour of the light.
==============================================================================*/
{
	int return_code;

	ENTER(set_Light_colour);
	if (light&&colour)
	{
		light->colour.red=colour->red;
		light->colour.green=colour->green;
		light->colour.blue=colour->blue;
		/* display list needs to be compiled again */
		light->display_list_current=0;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Light_colour.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Light_colour */

int get_Light_direction(struct Light *light,float direction[3])
/*******************************************************************************
LAST MODIFIED : 4 December 1997

DESCRIPTION :
Returns the direction of the light, relevent for infinite and spot lights.
==============================================================================*/
{
	int return_code;

	ENTER(get_Light_direction);
	if (light&&direction)
	{
		direction[0]=light->direction[0];
		direction[1]=light->direction[1];
		direction[2]=light->direction[2];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_Light_direction.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* get_Light_direction */

int set_Light_direction(struct Light *light,float direction[3])
/*******************************************************************************
LAST MODIFIED : 4 December 1997

DESCRIPTION :
Sets the direction of the light, relevent for infinite and spot lights.
==============================================================================*/
{
	int return_code;

	ENTER(set_Light_direction);
	if (light&&direction)
	{
		light->direction[0]=direction[0];
		light->direction[1]=direction[1];
		light->direction[2]=direction[2];
		/* display list needs to be compiled again */
		light->display_list_current=0;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Light_direction.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Light_direction */

int get_Light_position(struct Light *light,float position[3])
/*******************************************************************************
LAST MODIFIED : 4 December 1997

DESCRIPTION :
Returns the position of the light, relevent for point and spot lights.
==============================================================================*/
{
	int return_code;

	ENTER(get_Light_position);
	if (light&&position)
	{
		position[0]=light->position[0];
		position[1]=light->position[1];
		position[2]=light->position[2];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_Light_position.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* get_Light_position */

int set_Light_position(struct Light *light,float position[3])
/*******************************************************************************
LAST MODIFIED : 4 December 1997

DESCRIPTION :
Sets the position of the light, relevent for point and spot lights.
==============================================================================*/
{
	int return_code;

	ENTER(set_Light_position);
	if (light&&position)
	{
		light->position[0]=position[0];
		light->position[1]=position[1];
		light->position[2]=position[2];
		/* display list needs to be compiled again */
		light->display_list_current=0;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Light_position.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Light_position */

int get_Light_spot_cutoff(struct Light *light, float *spot_cutoff)
/*******************************************************************************
LAST MODIFIED : 8 October 2002

DESCRIPTION :
Returns the spotlight cutoff angle in degrees from 0 to 90.
==============================================================================*/
{
	int return_code;

	ENTER(get_Light_spot_cutoff);
	if (light && spot_cutoff)
	{
		*spot_cutoff = light->spot_cutoff;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_Light_spot_cutoff.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* get_Light_spot_cutoff */

int set_Light_spot_cutoff(struct Light *light, float spot_cutoff)
/*******************************************************************************
LAST MODIFIED : 8 October 2002

DESCRIPTION :
Sets the spotlight cutoff angle in degrees from 0 to 90.
==============================================================================*/
{
	int return_code;

	ENTER(set_Light_spot_cutoff);
	if (light && (0.0 <= spot_cutoff) && (90.0 >= spot_cutoff))
	{
		light->spot_cutoff = spot_cutoff;
		/* display list needs to be compiled again */
		light->display_list_current = 0;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Light_spot_cutoff.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* set_Light_spot_cutoff */

int get_Light_spot_exponent(struct Light *light, float *spot_exponent)
/*******************************************************************************
LAST MODIFIED : 8 October 2002

DESCRIPTION :
Returns the spotlight exponent which controls how concentrated the spotlight
becomes as one approaches its axis. A value of 0.0 gives even illumination
throughout the cutoff angle.
==============================================================================*/
{
	int return_code;

	ENTER(get_Light_spot_exponent);
	if (light && spot_exponent)
	{
		*spot_exponent = light->spot_exponent;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_Light_spot_exponent.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* get_Light_spot_exponent */

int set_Light_spot_exponent(struct Light *light, float spot_exponent)
/*******************************************************************************
LAST MODIFIED : 8 October 2002

DESCRIPTION :
Sets the spotlight exponent which controls how concentrated the spotlight
becomes as one approaches its axis. A value of 0.0 gives even illumination
throughout the cutoff angle.
==============================================================================*/
{
	int return_code;

	ENTER(set_Light_spot_exponent);
	if (light && (0.0 <= spot_exponent))
	{
		light->spot_exponent = spot_exponent;
		/* display list needs to be compiled again */
		light->display_list_current = 0;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Light_spot_exponent.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* set_Light_spot_exponent */

int get_Light_type(struct Light *light,enum Light_type *light_type)
/*******************************************************************************
LAST MODIFIED : 4 December 1997

DESCRIPTION :
Returns the light_type of the light (infinite/point/spot).
==============================================================================*/
{
	int return_code;

	ENTER(get_Light_type);
	if (light&&light_type)
	{
		*light_type=light->type;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_Light_type.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* get_Light_type */

int set_Light_type(struct Light *light,enum Light_type light_type)
/*******************************************************************************
LAST MODIFIED : 4 December 1997

DESCRIPTION :
Sets the light_type of the light (infinite/point/spot).
==============================================================================*/
{
	int return_code;

	ENTER(set_Light_type);
	if (light&&((INFINITE_LIGHT==light_type)||(POINT_LIGHT==light_type)||
		(SPOT_LIGHT==light_type)))
	{
		light->type=light_type;
		/* display list needs to be compiled again */
		light->display_list_current=0;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Light_type.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Light_type */

int modify_Light(struct Parse_state *state,void *light_void,
	void *modify_light_data_void)
/*******************************************************************************
LAST MODIFIED : 9 October 2002

DESCRIPTION :
==============================================================================*/
{
	char *current_token, *light_type_string, **valid_strings;
	enum Light_type light_type;
	float constant_attenuation, direction[3], linear_attenuation, position[3],
		quadratic_attenuation, spot_cutoff, spot_exponent;
	int num_floats, number_of_valid_strings, process, return_code;
	struct Colour light_colour;
	struct Light *light_to_be_modified,*light_to_be_modified_copy;
	struct Modify_light_data *modify_light_data;
	struct Option_table *option_table;

	ENTER(modify_Light);
	if (state &&
		(modify_light_data=(struct Modify_light_data *)modify_light_data_void))
	{
		if (current_token=state->current_token)
		{
			process=0;
			if (light_to_be_modified=(struct Light *)light_void)
			{
				if (IS_MANAGED(Light)(light_to_be_modified,modify_light_data->
					light_manager))
				{
					if (light_to_be_modified_copy=CREATE(Light)("copy"))
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
					if (light_to_be_modified=FIND_BY_IDENTIFIER_IN_MANAGER(Light,name)(
						current_token,modify_light_data->light_manager))
					{
						if (return_code=shift_Parse_state(state,1))
						{
							if (light_to_be_modified_copy=CREATE(Light)("copy"))
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
					if (light_to_be_modified=CREATE(Light)("help"))
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

int list_Light(struct Light *light,void *dummy)
/*******************************************************************************
LAST MODIFIED : 8 October 2002

DESCRIPTION :
Writes the properties of the <light> to the command window.
==============================================================================*/
{
	int return_code;

	ENTER(list_Light);
	USE_PARAMETER(dummy);
	if (light)
	{
		display_message(INFORMATION_MESSAGE, "light : %s : %s\n",
			light->name, ENUMERATOR_STRING(Light_type)(light->type));
		display_message(INFORMATION_MESSAGE,
			"  colour  red = %.3g, green = %.3g, blue = %.3g\n",
			(light->colour).red, (light->colour).green, (light->colour).blue);
		switch (light->type)
		{
			case POINT_LIGHT: case SPOT_LIGHT:
			{
				display_message(INFORMATION_MESSAGE,
					"  position  x = %.3g, y = %.3g, z = %.3g\n",
					light->position[0], light->position[1], light->position[2]);
			} break;
		}
		switch (light->type)
		{
			case INFINITE_LIGHT: case SPOT_LIGHT:
			{
				display_message(INFORMATION_MESSAGE,
					"  direction  x = %.3g, y = %.3g, z = %.3g\n",
					light->direction[0], light->direction[1], light->direction[2]);
			} break;
		}
		switch (light->type)
		{
			case POINT_LIGHT:
			case SPOT_LIGHT:
			{
				display_message(INFORMATION_MESSAGE,
					"  attenuation  constant = %g, linear = %g, quadratic = %g\n",
					light->constant_attenuation,
					light->linear_attenuation,
					light->quadratic_attenuation);
			} break;
		}
		if (SPOT_LIGHT == light->type)
		{
			display_message(INFORMATION_MESSAGE,
				"  spot cutoff = %.3g degrees\n", light->spot_cutoff);
			display_message(INFORMATION_MESSAGE,
				"  spot exponent = %g\n", light->spot_exponent);
		}
#if defined (DEBUG)
		display_message(INFORMATION_MESSAGE,"  access count = %d\n",
			light->access_count);
#endif /* defined (DEBUG) */
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"list_Light.  Missing light");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Light */

int list_Light_name(struct Light *light,void *preceding_text_void)
/*******************************************************************************
LAST MODIFIED : 22 January 2002

DESCRIPTION :
Writes the name of the <light> to the command window, preceded on each line by
the optional <preceding_text> string. Makes sure quotes are put around the
name of the light if it contains any special characters.
==============================================================================*/
{
	char *name,*preceding_text;
	int return_code;

	ENTER(list_Light_name);
	if (light)
	{
		if (preceding_text=(char *)preceding_text_void)
		{
			display_message(INFORMATION_MESSAGE,preceding_text);
		}
		if (name=duplicate_string(light->name))
		{
			/* put quotes around name if it contains special characters */
			make_valid_token(&name);
			display_message(INFORMATION_MESSAGE,"%s\n",name);
			DEALLOCATE(name);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"list_Light_name.  Missing light");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* list_Light_name */

int list_Light_name_command(struct Light *light,void *preceding_text_void)
/*******************************************************************************
LAST MODIFIED : 22 January 2002

DESCRIPTION :
Writes the name of the <light> to the command window, preceded on each line by
the optional <preceding_text> string. Makes sure quotes are put around the
name of the light if it contains any special characters.
Follows the light name with semicolon and carriage return.
==============================================================================*/
{
	char *name,*preceding_text;
	int return_code;

	ENTER(list_Light_name_command);
	if (light)
	{
		if (preceding_text=(char *)preceding_text_void)
		{
			display_message(INFORMATION_MESSAGE,preceding_text);
		}
		if (name=duplicate_string(light->name))
		{
			/* put quotes around name if it contains special characters */
			make_valid_token(&name);
			display_message(INFORMATION_MESSAGE,"%s;\n",name);
			DEALLOCATE(name);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"list_Light_name_command.  Missing light");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* list_Light_name_command */

#if defined (OLD_CODE)
int activate_Light(struct Light *light)
/*******************************************************************************
LAST MODIFIED : 21 April 1995

DESCRIPTION :
Activates the <light> as part of the rendering loop.
==============================================================================*/
{
	int return_code;
#if defined (GL_API)
	float values[21];
	short number_of_values;
#endif
#if defined (OPENGL_API)
	GLenum light_id;
	GLfloat values[4];
#endif

	ENTER(activate_Light);
	if (light)
	{
		if (light->on)
		{
/*      if (graphics_library_open)
			{*/
#if defined (GL_API) || defined (OPENGL_API)
				if (next_light_no<MAXIMUM_NUMBER_OF_ACTIVE_LIGHTS)
				{
#if defined (GL_API)
					values[0]=LCOLOR;
					values[1]=(light->colour).red;
					values[2]=(light->colour).green;
					values[3]=(light->colour).blue;
					values[4]=AMBIENT;
					values[5]=0.;
					values[6]=0.;
					values[7]=0.;
					switch (light->type)
					{
						case INFINITE_LIGHT:
						{
							values[8]=POSITION;
							values[9]= -(light->direction).x;
							values[10]= -(light->direction).y;
							values[11]= -(light->direction).z;
							values[12]=0.;
							values[13]=SPOTLIGHT;
							values[14]=0.;
							values[15]=180.;
							values[16]=LMNULL;
							number_of_values=17;
						} break;
						case POINT_LIGHT:
						{
							values[8]=POSITION;
							values[9]=(light->position).x;
							values[10]=(light->position).y;
							values[11]=(light->position).z;
							values[12]=1.;
							values[13]=SPOTLIGHT;
							values[14]=0.;
							values[15]=180.;
							values[16]=LMNULL;
							number_of_values=17;
						} break;
						case SPOT_LIGHT:
						{
							values[8]=POSITION;
							values[9]=(light->position).x;
							values[10]=(light->position).y;
							values[11]=(light->position).z;
							values[12]=1.;
							values[13]=SPOTLIGHT;
							values[14]=64.;
							values[15]=light->cutoff;
							values[16]=SPOTDIRECTION;
							values[17]=(light->direction).x;
							values[18]=(light->direction).y;
							values[19]=(light->direction).z;
							values[20]=LMNULL;
							number_of_values=21;
						} break;
					}
					lmdef(DEFLIGHT,light_indices[next_light_no],number_of_values,values);
					lmbind(light_identifiers[next_light_no],light_indices[next_light_no]);
					return_code=1;
#endif
#if defined (OPENGL_API)
					light_id=light_identifiers[next_light_no];
					values[0]=0.;
					values[1]=0.;
					values[2]=0.;
					values[3]=1.;
					glLightfv(light_id,GL_AMBIENT,values);
					values[0]=(GLfloat)((light->colour).red);
					values[1]=(GLfloat)((light->colour).green);
					values[2]=(GLfloat)((light->colour).blue);
					glLightfv(light_id,GL_DIFFUSE,values);
					glLightfv(light_id,GL_SPECULAR,values);
					switch (light->type)
					{
						case INFINITE_LIGHT:
						{
							values[0]= -(light->direction).x;
							values[1]= -(light->direction).y;
							values[2]= -(light->direction).z;
							values[3]=0.;
							glLightfv(light_id,GL_POSITION,values);
							glLightf(light_id,GL_SPOT_EXPONENT,0.);
							glLightf(light_id,GL_SPOT_CUTOFF,180.);
						} break;
						case POINT_LIGHT:
						{
							values[0]=(light->direction).x;
							values[1]=(light->direction).y;
							values[2]=(light->direction).z;
							values[3]=1.;
							glLightfv(light_id,GL_POSITION,values);
							glLightf(light_id,GL_SPOT_EXPONENT,0.);
							glLightf(light_id,GL_SPOT_CUTOFF,180.);
						} break;
						case SPOT_LIGHT:
						{
							values[0]=(light->position).x;
							values[1]=(light->position).y;
							values[2]=(light->position).z;
							values[3]=1.;
							glLightfv(light_id,GL_POSITION,values);
							values[0]=(light->direction).x;
							values[1]=(light->direction).y;
							values[2]=(light->direction).z;
							glLightfv(light_id,GL_SPOT_DIRECTION,values);
							glLightf(light_id,GL_SPOT_EXPONENT,64.);
							glLightf(light_id,GL_SPOT_CUTOFF,(GLfloat)(light->cutoff));
						} break;
					}
					glLightf(light_id,GL_CONSTANT_ATTENUATION,1.);
					glLightf(light_id,GL_LINEAR_ATTENUATION,0.);
					glLightf(light_id,GL_QUADRATIC_ATTENUATION,0.);
					glEnable(light_id);
					return_code=1;
#endif
					next_light_no++;
				}
				else
				{
					display_message(ERROR_MESSAGE,"Cannot have any more lights on");
					return_code=0;
				}
#endif /* defined (GL_API) || defined (OPENGL_API) */
/*      }
			else
			{
				return_code=0;
			}*/
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"activate_Light.  Missing light");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* activate_Light */
#endif /* defined (OLD_CODE) */

int reset_Lights(void)
/*******************************************************************************
LAST MODIFIED : 4 December 1997

DESCRIPTION :
Must be called at start of rendering before lights are activate with
execute_Light. Ensures all lights are off at the start of the rendering loop,
and makes sure the lights that are subsequently defined start at GL_LIGHT0...
==============================================================================*/
{
	int return_code,light_no;

	ENTER(reset_Lights);
	for (light_no=0;light_no<MAXIMUM_NUMBER_OF_ACTIVE_LIGHTS;light_no++)
	{
		glDisable(light_identifiers[light_no]);
	}
	next_light_no=0;
	return_code=1;
	LEAVE;

	return (return_code);
} /* reset_Lights */

int set_Light(struct Parse_state *state,
	void *light_address_void,void *light_manager_void)
/*******************************************************************************
LAST MODIFIED : 22 June 1999

DESCRIPTION :
Modifier function to set the light from a command.
???RC set_Object routines could become a macro.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Light *temp_light,**light_address;
	struct MANAGER(Light) *light_manager;

	ENTER(set_Light);
	if (state)
	{
		if (current_token=state->current_token)
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
						if (temp_light=FIND_BY_IDENTIFIER_IN_MANAGER(Light,
							name)(current_token,light_manager))
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
				if (light_address=
					(struct Light **)light_address_void)
				{
					if (temp_light= *light_address)
					{
						display_message(INFORMATION_MESSAGE,"[%s]",temp_light->name);
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

int compile_Light(struct Light *light,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 4 December 1997

DESCRIPTION :
Struct Light iterator function which should make a display list for the light.
If OpenGL lights were set up half-way decently you could throw them all into
display lists with this function and then execute them with execute_Light.
However, the basic installation supports only EIGHT lights, but worse, they are
each referenced by their own constants GL_LIGHT0..GL_LIGHT7. Hence, if you want
differing combinations of lights in different windows and at different times,
you can't use display lists since the light identifier would be hardwired in
them.
Hence, this routine does *nothing*, while the execute_Light routine just calls
direct_render_Light. Before any lights are activated, however, you must call
reset_Lights to turn all the lights off and set next_light_no to zero.
Future improvements to OpenGL or other graphics libraries may overcome this
problem.
==============================================================================*/
{
	int return_code;

	ENTER(compile_Light);
	USE_PARAMETER(dummy_void);
	if (light)
	{
/*
		if (!light->display_list_current)
		{
			if (light->display_list||(light->display_list=glGenLists(1)))
			{
				glNewList(light->display_list,GL_COMPILE);
				direct_render_Light(light);
				glEndList();
				light->display_list_current=1;
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"compile_Light.  Could not generate display list");
				return_code=0;
			}
		}
		else
		{
			return_code=1;
		}
*/
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"compile_Light.  Missing light");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* compile_Light */

int execute_Light(struct Light *light,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 4 December 1997

DESCRIPTION :
Struct Light iterator function for activating the <light>.
Does not use display lists. See comments with compile_Light, above.
==============================================================================*/
{
	int return_code;

	ENTER(execute_Light);
	USE_PARAMETER(dummy_void);
/*
	if (light)
	{
		if (light->display_list_current)
		{
			glCallList(light->display_list);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_Light.  display list not current");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_Light.  Missing light");
		return_code=0;
	}
*/
	return_code=direct_render_Light(light);
	LEAVE;

	return (return_code);
} /* execute_Light */

int Light_to_list(struct Light *light,void *list_of_lights_void)
/*******************************************************************************
LAST MODIFIED : 9 December 1997

DESCRIPTION :
Light iterator function for duplicating lists of lights.
==============================================================================*/
{
	int return_code;
	struct LIST(Light) *list_of_lights;

	ENTER(Light_to_list);
	if (light&&(list_of_lights=(struct LIST(Light) *)list_of_lights_void))
	{
		return_code=ADD_OBJECT_TO_LIST(Light)(light,list_of_lights);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Light_to_list.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Light_to_list */

int Light_is_in_list(struct Light *light, void *light_list_void)
/*******************************************************************************
LAST MODIFIED : 30 May 2001

DESCRIPTION :
Returns true if <light> is in <light_list>.
==============================================================================*/
{
	int return_code;
	struct LIST(Light) *light_list;

	ENTER(Light_is_in_list);
	if (light && (light_list = (struct LIST(Light) *)light_list_void))
	{
		return_code = IS_OBJECT_IN_LIST(Light)(light, light_list);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Light_is_in_list.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Light_is_in_list */
