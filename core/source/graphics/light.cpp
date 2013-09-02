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

#include "zinc/status.h"
#include "zinc/zincconfigure.h"

#include "general/debug.h"
#include "general/list_private.h"
#include "general/manager_private.h"
#include "general/mystring.h"
#include "general/object.h"
#include "graphics/graphics_library.h"
#include "graphics/light.h"
#include "general/message.h"
#include "general/enumerator_private.hpp"

/*
Module types
------------
*/

struct Light_module
{

private:

	struct MANAGER(Light) *lightManager;
	Light *defaultLight;
	int access_count;

	Light_module() :
		lightManager(CREATE(MANAGER(Light))()),
		defaultLight(0),
		access_count(1)
	{
	}

	~Light_module()
	{
		DEACCESS(Light)(&this->defaultLight);
		DESTROY(MANAGER(Light))(&(this->lightManager));
	}

public:

	static Light_module *create()
	{
		return new Light_module();
	}

	Light_module *access()

	{
		++access_count;
		return this;
	}

	static int deaccess(Light_module* &light_module)
	{
		if (light_module)
		{
			--(light_module->access_count);
			if (light_module->access_count <= 0)
			{
				delete light_module;
			}
			light_module = 0;
			return CMZN_OK;
		}
		return CMZN_ERROR_ARGUMENT;
	}

	struct MANAGER(Light) *getManager()
	{
		return this->lightManager;
	}

	int beginChange()
	{
		return MANAGER_BEGIN_CACHE(Light)(this->lightManager);
	}

	int endChange()
	{
		return MANAGER_END_CACHE(Light)(this->lightManager);
	}

	Light *createLight()
	{
		Light *light = NULL;
		char temp_name[20];
		int i = NUMBER_IN_MANAGER(Light)(this->lightManager);
		do
		{
			i++;
			sprintf(temp_name, "temp%d",i);
		}
		while (FIND_BY_IDENTIFIER_IN_MANAGER(Light,name)(temp_name,
			this->lightManager));
		light = CREATE(Light)(temp_name);
		if (!ADD_OBJECT_TO_MANAGER(Light)(light, this->lightManager))
		{
			DEACCESS(Light)(&light);
		}
		return light;
	}

	Light *findLightByName(const char *name)
	{
		Light *light = FIND_BY_IDENTIFIER_IN_MANAGER(Light,name)(name,
			this->lightManager);
		if (light)
		{
			return ACCESS(Light)(light);
		}
		return 0;
	}

	Light *getDefaultLight()
	{
		if (this->defaultLight)
		{
			ACCESS(Light)(this->defaultLight);
		}
		else
		{
			this->beginChange();
			Light *light = CREATE(Light)("default");
			if (light)
			{
				GLfloat default_light_direction[3]={0.0,-0.5,-1.0};
				struct Colour default_colour;
				set_Light_type(light,INFINITE_LIGHT);
				default_colour.red=1.0;
				default_colour.green=1.0;
				default_colour.blue=1.0;
				set_Light_colour(light,&default_colour);
				set_Light_direction(light,default_light_direction);
				/*???DB.  Include default as part of manager ? */
				if (!ADD_OBJECT_TO_MANAGER(Light)(light, this->lightManager))
				{
					DEACCESS(Light)(&light);
				}
			}
			this->setDefaultLight(light);
			this->endChange();
		}
		return (this->defaultLight);
	}

	int setDefaultLight(Light *light)
	{
		REACCESS(Light)(&this->defaultLight, light);
		return CMZN_OK;
	}

};

struct Light
/*******************************************************************************
LAST MODIFIED : 8 October 2002

DESCRIPTION :
The properties of a light.
==============================================================================*/
{
	/* the name of the light */
	const char *name;
	/* light type: infinite, point or spot */
	enum Light_type type;
	struct Colour colour;
	/* position for point and spot lights */
	GLfloat position[3];
	/* direction for infinite (directional) and spot lights */
	GLfloat direction[3];
	/* attenuation parameters control light falloff with distance according to
	   1/(c + l*d + q*d*d) where d=distance, c=constant, l=linear, q=quadratic */
	GLfloat constant_attenuation, linear_attenuation, quadratic_attenuation;
	/* the angle, in degrees, between the direction of the spotlight and the ray
		along the edge of the light cone (between 0 and 90 inclusive) */
	GLfloat spot_cutoff;
	/* spot_exponent controls concentration of light near its axis; 0 = none */
	GLfloat spot_exponent;
	/* include display list stuff although cannot use at present */
	GLuint display_list;
	int display_list_current;

	/* after clearing in create, following to be modified only by manager */
	struct MANAGER(Light) *manager;
	int manager_change_status;

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
const char *get_Light_name(struct Light *light)
{
	return light->name;
}
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

/*
Global functions
----------------
*/

PROTOTYPE_ENUMERATOR_STRING_FUNCTION(Light_type)
{
	const char *enumerator_string;

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
			enumerator_string = (const char *)NULL;
		} break;
	}
	LEAVE;

	return (enumerator_string);
} /* ENUMERATOR_STRING(Light_type) */

DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS(Light_type)

struct Light *CREATE(Light)(const char *name)
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
			strcpy((char *)light->name, name);
			light->access_count = 1;
			light->manager = (struct MANAGER(Light) *)NULL;
			light->manager_change_status = MANAGER_CHANGE_NONE(Light);
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
DECLARE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Light,name,const char *,strcmp)
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
			return_code = MANAGER_COPY_WITHOUT_IDENTIFIER(Light,name)(
				destination,source);
			if (return_code)
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

PROTOTYPE_MANAGER_COPY_IDENTIFIER_FUNCTION(Light,name,const char *)
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

DECLARE_MANAGER_FUNCTIONS(Light,manager)

DECLARE_DEFAULT_MANAGED_OBJECT_NOT_IN_USE_FUNCTION(Light,manager)

DECLARE_MANAGER_IDENTIFIER_FUNCTIONS(Light,name,const char *,manager)

int get_Light_attenuation(struct Light *light, GLfloat *constant_attenuation,
	GLfloat *linear_attenuation, GLfloat *quadratic_attenuation)
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

int set_Light_attenuation(struct Light *light, GLfloat constant_attenuation,
	GLfloat linear_attenuation, GLfloat quadratic_attenuation)
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
		MANAGED_OBJECT_CHANGE(Light)(light,	MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Light));
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
		MANAGED_OBJECT_CHANGE(Light)(light,	MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Light));
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

int get_Light_direction(struct Light *light,GLfloat direction[3])
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

int set_Light_direction(struct Light *light,GLfloat direction[3])
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
		MANAGED_OBJECT_CHANGE(Light)(light,	MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Light));
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

int get_Light_position(struct Light *light,GLfloat position[3])
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

int set_Light_position(struct Light *light,GLfloat position[3])
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
		MANAGED_OBJECT_CHANGE(Light)(light,	MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Light));
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

int get_Light_spot_cutoff(struct Light *light, GLfloat *spot_cutoff)
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

int set_Light_spot_cutoff(struct Light *light, GLfloat spot_cutoff)
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
		MANAGED_OBJECT_CHANGE(Light)(light,	MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Light));
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

int get_Light_spot_exponent(struct Light *light, GLfloat *spot_exponent)
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

int set_Light_spot_exponent(struct Light *light, GLfloat spot_exponent)
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
		MANAGED_OBJECT_CHANGE(Light)(light,	MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Light));
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
		MANAGED_OBJECT_CHANGE(Light)(light,	MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Light));
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
			default:
			{
			}	break;
		}
		switch (light->type)
		{
			case INFINITE_LIGHT: case SPOT_LIGHT:
			{
				display_message(INFORMATION_MESSAGE,
					"  direction  x = %.3g, y = %.3g, z = %.3g\n",
					light->direction[0], light->direction[1], light->direction[2]);
			} break;
			default:
			{
			}	break;
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
			default:
			{
			}	break;
		}
		if (SPOT_LIGHT == light->type)
		{
			display_message(INFORMATION_MESSAGE,
				"  spot cutoff = %.3g degrees\n", light->spot_cutoff);
			display_message(INFORMATION_MESSAGE,
				"  spot exponent = %g\n", light->spot_exponent);
		}
#if defined (DEBUG_CODE)
		display_message(INFORMATION_MESSAGE,"  access count = %d\n",
			light->access_count);
#endif /* defined (DEBUG_CODE) */
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
		preceding_text=(char *)preceding_text_void;
		if (preceding_text)
		{
			display_message(INFORMATION_MESSAGE,preceding_text);
		}
		name=duplicate_string(light->name);
		if (name)
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
		preceding_text=(char *)preceding_text_void;
		if (preceding_text)
		{
			display_message(INFORMATION_MESSAGE,preceding_text);
		}
		name=duplicate_string(light->name);
		if (name)
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

int write_Light_name_command_to_comfile(struct Light *light,void *preceding_text_void)
/*******************************************************************************
LAST MODIFIED : 10 August 2007

DESCRIPTION :
Writes the name of the <light> to the comfile, preceded on each line by
the optional <preceding_text> string. Makes sure quotes are put around the
name of the light if it contains any special characters.
Follows the light name with semicolon and carriage return.
==============================================================================*/
{
	char *name,*preceding_text;
	int return_code;

	ENTER(write_Light_name_command_to_comfile);
	if (light)
	{
		preceding_text=(char *)preceding_text_void;
		if (preceding_text)
		{
			write_message_to_file(INFORMATION_MESSAGE,preceding_text);
		}
		name=duplicate_string(light->name);
		if (name)
		{
			/* put quotes around name if it contains special characters */
			make_valid_token(&name);
			write_message_to_file(INFORMATION_MESSAGE,"%s;\n",name);
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
} /* write_Light_name_command_to_comfile */

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

struct MANAGER(Light) *Light_module_get_manager(Light_module *light_module)
{
	if (light_module)
		return light_module->getManager();
	return 0;
}

Light_module *Light_module_create()
{
	return Light_module::create();
}

Light_module *Light_module_access(
	Light_module *light_module)
{
	if (light_module)
		return light_module->access();
	return 0;
}

int Light_module_destroy(Light_module **light_module_address)
{
	if (light_module_address)
		return Light_module::deaccess(*light_module_address);
	return CMZN_ERROR_ARGUMENT;
}

Light *Light_module_create_light(
	Light_module *light_module)
{
	if (light_module)
		return light_module->createLight();
	return 0;
}

int Light_module_begin_change(Light_module *light_module)
{
	if (light_module)
		return light_module->beginChange();
   return CMZN_ERROR_ARGUMENT;
}

int Light_module_end_change(Light_module *light_module)
{
	if (light_module)
		return light_module->endChange();
   return CMZN_ERROR_ARGUMENT;
}

Light *Light_module_find_light_by_name(
	Light_module *light_module, const char *name)
{
	if (light_module)
		return light_module->findLightByName(name);
   return 0;
}

Light *Light_module_get_default_light(Light_module *light_module)
{
	if (light_module)
		return light_module->getDefaultLight();
	return 0;
}

int Light_module_set_default_light(Light_module *light_module, Light *light)
{
	if (light_module)
		return light_module->setDefaultLight(light);
	return 0;
}
