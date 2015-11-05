/*******************************************************************************
FILE : light.c

LAST MODIFIED : 9 October 2002

DESCRIPTION :
The functions for manipulating lights.

???RC
If OpenGL lights were set up half-way decently you could throw them all into
display lists in compile_cmzn_light and then execute them with execute_cmzn_light.
However, the basic installation supports only EIGHT lights, but worse, they are
each referenced by their own constants GL_LIGHT0..GL_LIGHT7. Hence, if you want
all available lights in differing combinations in different windows at
different times,you can't use display lists since the light identifier would
be hardwired in them.
Hence, this compile_cmzn_light does nothing, while the execute_cmzn_light routine just
calls direct_render_cmzn_light. Before any lights are activated, however, you must
call reset_cmzn_lights to turn all the lights off and set next_light_no to zero.
Future improvements to OpenGL or other graphics libraries may overcome this
problem.
==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
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
#include "graphics/light.hpp"
#include "general/message.h"
#include "general/enumerator_private.hpp"
#include "general/cmiss_set.hpp"
#include "general/indexed_list_stl_private.hpp"


/*
Module types
------------
*/


struct cmzn_lightmodule
{

private:

	struct MANAGER(cmzn_light) *lightManager;
	cmzn_light *defaultLight, *defaultAmbientLight;
	int access_count;

	cmzn_lightmodule() :
		lightManager(CREATE(MANAGER(cmzn_light))()),
		defaultLight(0),
		defaultAmbientLight(0),
		access_count(1)
	{
	}

	~cmzn_lightmodule()
	{
		DEACCESS(cmzn_light)(&this->defaultLight);
		DEACCESS(cmzn_light)(&this->defaultAmbientLight);
		DESTROY(MANAGER(cmzn_light))(&(this->lightManager));
	}

public:

	static cmzn_lightmodule *create()
	{
		return new cmzn_lightmodule();
	}

	cmzn_lightmodule *access()

	{
		++access_count;
		return this;
	}

	static int deaccess(cmzn_lightmodule* &lightmodule)
	{
		if (lightmodule)
		{
			--(lightmodule->access_count);
			if (lightmodule->access_count <= 0)
			{
				delete lightmodule;
			}
			lightmodule = 0;
			return CMZN_OK;
		}
		return CMZN_ERROR_ARGUMENT;
	}

	struct MANAGER(cmzn_light) *getManager()
	{
		return this->lightManager;
	}

	int beginChange()
	{
		return MANAGER_BEGIN_CACHE(cmzn_light)(this->lightManager);
	}

	int endChange()
	{
		return MANAGER_END_CACHE(cmzn_light)(this->lightManager);
	}

	cmzn_light *createLight()
	{
		cmzn_light *light = NULL;
		char temp_name[20];
		int i = NUMBER_IN_MANAGER(cmzn_light)(this->lightManager);
		do
		{
			i++;
			sprintf(temp_name, "temp%d",i);
		}
		while (FIND_BY_IDENTIFIER_IN_MANAGER(cmzn_light,name)(temp_name,
			this->lightManager));
		light = cmzn_light_create_private();
		cmzn_light_set_name(light, temp_name);
		if (!ADD_OBJECT_TO_MANAGER(cmzn_light)(light, this->lightManager))
		{
			DEACCESS(cmzn_light)(&light);
		}
		return light;
	}

	cmzn_lightiterator *createLightiterator();

	cmzn_light *findLightByName(const char *name)
	{
		cmzn_light *light = FIND_BY_IDENTIFIER_IN_MANAGER(cmzn_light,name)(name,
			this->lightManager);
		if (light)
		{
			return ACCESS(cmzn_light)(light);
		}
		return 0;
	}

	cmzn_light *getDefaultLight()
	{
		if (this->defaultLight)
		{
			ACCESS(cmzn_light)(this->defaultLight);
		}
		else
		{
			this->beginChange();
			cmzn_light *light = createLight();
			cmzn_light_set_name(light, "default");
			double default_light_direction[3]={0.0,-0.5,-1.0};
			double default_colour[3];
			cmzn_light_set_type(light,CMZN_LIGHT_TYPE_DIRECTIONAL);
			default_colour[0]=0.9;
			default_colour[1]=0.9;
			default_colour[2]=0.9;
			cmzn_light_set_colour_rgb(light, &default_colour[0]);
			cmzn_light_set_direction(light, default_light_direction);
			this->setDefaultLight(light);
			this->endChange();
		}
		return (this->defaultLight);
	}

	int setDefaultLight(cmzn_light *light)
	{
		if (light)
		{
			enum cmzn_light_type lightType = cmzn_light_get_type(light);
			if ((lightType != CMZN_LIGHT_TYPE_AMBIENT) &&
				(lightType != CMZN_LIGHT_TYPE_INVALID))
			{
				REACCESS(cmzn_light)(&this->defaultLight, light);
				return CMZN_OK;
			}
		}
		else
		{
			cmzn_light_destroy(&this->defaultLight);
			return CMZN_OK;
		}
		return CMZN_ERROR_ARGUMENT;
	}

	cmzn_light *getDefaultAmbientLight()
	{
		if (this->defaultAmbientLight)
		{
			ACCESS(cmzn_light)(this->defaultAmbientLight);
		}
		else
		{
			this->beginChange();
			cmzn_light *light = createLight();
			cmzn_light_set_name(light, "default_ambient");
			double default_colour[3];
			cmzn_light_set_type(light,CMZN_LIGHT_TYPE_AMBIENT);
			default_colour[0]=0.1;
			default_colour[1]=0.1;
			default_colour[2]=0.1;
			cmzn_light_set_colour_rgb(light,&default_colour[0]);
			cmzn_light_set_render_side(light,CMZN_LIGHT_RENDER_SIDE_DOUBLE);
			this->setDefaultAmbientLight(light);
			this->endChange();
		}
		return (this->defaultAmbientLight);
	}

	int setDefaultAmbientLight(cmzn_light *light)
	{
		if (cmzn_light_get_type(light) == CMZN_LIGHT_TYPE_AMBIENT)
		{
			REACCESS(cmzn_light)(&this->defaultAmbientLight, light);
			return CMZN_OK;
		}
		return CMZN_ERROR_ARGUMENT;
	}

};

/***************************************************************************//**
 * Object describing how elements / continuous field domains are tessellated
 * or sampled into graphics.
 */
struct cmzn_light
{
	const char *name;
	struct MANAGER(cmzn_light) *manager;
	int manager_change_status;
	enum cmzn_light_type type;
	enum cmzn_light_render_side render_side;
	enum cmzn_light_render_viewer_mode viewer_mode;
	/* attenuation parameters control light falloff with distance according to
	   1/(c + l*d + q*d*d) where d=distance, c=constant, l=linear, q=quadratic */
	double constant_attenuation, linear_attenuation, quadratic_attenuation;
	/* the angle, in degrees, between the direction of the spotlight and the ray
		along the edge of the light cone (between 0 and 90 inclusive) */
	double spot_cutoff;
	/* spot_exponent controls concentration of light near its axis; 0 = none */
	double spot_exponent;
	/* after clearing in create, following to be modified only by manager */
	bool enabled;
	bool is_managed_flag;
	int access_count;
	/* position for point and spot lights */
	double position[3];
	/* direction for infinite (directional) and spot lights */
	double direction[3];
	struct Colour colour;

protected:

	cmzn_light() :
		name(NULL),
		manager(NULL),
		manager_change_status(MANAGER_CHANGE_NONE(cmzn_light)),
		type(CMZN_LIGHT_TYPE_DIRECTIONAL),
		render_side(CMZN_LIGHT_RENDER_SIDE_DOUBLE),
		viewer_mode(CMZN_LIGHT_RENDER_VIEWER_MODE_INFINITE),
		constant_attenuation(1.0),
		linear_attenuation(0.0),
		quadratic_attenuation(0.0),
		spot_cutoff(90.0),
		spot_exponent(0.0),
		enabled(true),
		is_managed_flag(false),
		access_count(1)
	{
		colour.red = 0.9;
		colour.green = 0.9;
		colour.blue = 0.9;
		position[0] = 0;
		position[1] = 0;
		position[2] = 0;
		direction[0] = 0;
		direction[1] = 0;
		direction[2] = -1;
	}

	~cmzn_light()
	{
		if (name)
		{
			DEALLOCATE(name);
		}
	}

public:

	/** must construct on the heap with this function */
	static cmzn_light *create()
	{
		return new cmzn_light();
	}

	cmzn_light& operator=(const cmzn_light& source)
	{
		double myColour[3];
		myColour[0] = source.colour.red;
		myColour[1] = source.colour.green;
		myColour[2] = source.colour.blue;
		setColour(&myColour[0]);
		setPosition(&source.position[0]);
		setDirection(&source.direction[0]);
		setType(source.type);
		setConstantAttenuation(source.constant_attenuation);
		setLinearAttenuation(source.linear_attenuation);
		setQuadraticAttenuation(source.quadratic_attenuation);
		setRenderSide(source.render_side);
		setRenderViewerMode(source.viewer_mode);
		setSpotCutoff(source.spot_cutoff);
		setSpotExponent(source.spot_exponent);
		setEnabled(source.enabled);
		return *this;
	}

	int getColour(double *colourOut)
	{
		colourOut[0] = static_cast<double>(colour.red);
		colourOut[1] = static_cast<double>(colour.green);
		colourOut[2] = static_cast<double>(colour.blue);
		return CMZN_OK;
	}

	int setColour(const double *colourIn)
	{
		colour.red = colourIn[0];
		colour.green = colourIn[1];
		colour.blue = colourIn[2];
		MANAGED_OBJECT_CHANGE(cmzn_light)(this,
			MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(cmzn_light));
		return CMZN_OK;
	}

	enum cmzn_light_render_side getRenderSide()
	{
		return render_side;
	}

	int setRenderSide(enum cmzn_light_render_side renderSideIn)
	{
		if (renderSideIn != render_side && renderSideIn != CMZN_LIGHT_RENDER_SIDE_INVALID)
		{
			render_side = renderSideIn;
			MANAGED_OBJECT_CHANGE(cmzn_light)(this,
				MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(cmzn_light));
		}
		return CMZN_OK;
	}

	enum cmzn_light_render_viewer_mode getRenderViewerMode()
	{
		return viewer_mode;
	}

	int setRenderViewerMode(enum cmzn_light_render_viewer_mode renderViewerModeIn)
	{
		if (viewer_mode != renderViewerModeIn &&
			renderViewerModeIn != CMZN_LIGHT_RENDER_VIEWER_MODE_INVALID)
		{
			viewer_mode = renderViewerModeIn;
			MANAGED_OBJECT_CHANGE(cmzn_light)(this,
				MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(cmzn_light));
		}
		return CMZN_OK;
	}

	enum cmzn_light_type getType()
	{
		return type;
	}

	int setType(enum cmzn_light_type typeIn)
	{
		if (typeIn != type && typeIn != CMZN_LIGHT_TYPE_INVALID)
		{
			type = typeIn;
			MANAGED_OBJECT_CHANGE(cmzn_light)(this,
				MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(cmzn_light));
		}
		return CMZN_OK;
	}

	int getPosition(double *positionOut)
	{
		positionOut[0] = position[0];
		positionOut[1] = position[1];
		positionOut[2] = position[2];
		return CMZN_OK;
	}

	int setPosition(const double *positionIn)
	{
		if (positionIn)
		{
			position[0] = positionIn[0];
			position[1] = positionIn[1];
			position[2] = positionIn[2];
			MANAGED_OBJECT_CHANGE(cmzn_light)(this,
				MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(cmzn_light));
			return CMZN_OK;
		}
		return CMZN_ERROR_ARGUMENT;
	}

	int getDirection(double *directionOut)
	{
		directionOut[0] = direction[0];
		directionOut[1] = direction[1];
		directionOut[2] = direction[2];
		return CMZN_OK;
	}

	int setDirection(const double *directionIn)
	{
		if (directionIn)
		{
			direction[0] = directionIn[0];
			direction[1] = directionIn[1];
			direction[2] = directionIn[2];
			MANAGED_OBJECT_CHANGE(cmzn_light)(this,
				MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(cmzn_light));
			return CMZN_OK;
		}
		return CMZN_ERROR_ARGUMENT;
	}

	double getSpotCutoff()
	{
		return spot_cutoff;
	}

	int setSpotCutoff(double spotCutoffIn)
	{
		if ((0.0 <= spotCutoffIn) && (90.0 >= spotCutoffIn))
		{
			spot_cutoff = spotCutoffIn;
			MANAGED_OBJECT_CHANGE(cmzn_light)(this,
				MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(cmzn_light));
			return CMZN_OK;
		}
		return CMZN_ERROR_ARGUMENT;
	}

	double getSpotExponent()
	{
		return spot_exponent;
	}

	int setSpotExponent(double spotExponentIn)
	{
		if (0.0 <= spotExponentIn)
		{
			spot_exponent = spotExponentIn;
			MANAGED_OBJECT_CHANGE(cmzn_light)(this,
				MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(cmzn_light));
			return CMZN_OK;
		}

		return CMZN_ERROR_ARGUMENT;
	}

	double getConstantAttenuation()
	{
		return constant_attenuation;
	}

	int setConstantAttenuation(double constantAttenuationIn)
	{
		if (0.0 <= constantAttenuationIn)
		{
			constant_attenuation = constantAttenuationIn;
			MANAGED_OBJECT_CHANGE(cmzn_light)(this,
				MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(cmzn_light));
			return CMZN_OK;
		}
		return CMZN_ERROR_ARGUMENT;
	}

	double getLinearAttenuation()
	{
		return linear_attenuation;
	}

	int setLinearAttenuation(double linearAttenuationIn)
	{
		if (0.0 <= linearAttenuationIn)
		{
			linear_attenuation = linearAttenuationIn;
			MANAGED_OBJECT_CHANGE(cmzn_light)(this,
				MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(cmzn_light));
			return CMZN_OK;
		}
		return CMZN_ERROR_ARGUMENT;
	}

	double getQuadraticAttenuation()
	{
		return linear_attenuation;
	}

	int setQuadraticAttenuation(double quadraticAttenuationIn)
	{
		if (0.0 <= quadraticAttenuationIn)
		{
			quadratic_attenuation = quadraticAttenuationIn;
			MANAGED_OBJECT_CHANGE(cmzn_light)(this,
				MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(cmzn_light));
			return CMZN_OK;
		}
		return CMZN_ERROR_ARGUMENT;
	}

	int setEnabled(bool enabledIn)
	{
		if (enabled != enabledIn)
		{
			enabled = enabledIn;
			MANAGED_OBJECT_CHANGE(cmzn_light)(this,
				MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(cmzn_light));
		}
		return CMZN_OK;
	}

	bool isEnabled()
	{
		return enabled;
	}

	inline cmzn_light *access()
	{
		++access_count;
		return this;
	}

	/** deaccess handling is_managed_flag */
	static inline int deaccess(cmzn_light **object_address)
	{
		int return_code = 1;
		cmzn_light *object;
		if (object_address && (object = *object_address))
		{
			(object->access_count)--;
			if (object->access_count <= 0)
			{
				delete object;
				object = 0;
			}
			else if ((!object->is_managed_flag) && (object->manager) &&
				((1 == object->access_count) || ((2 == object->access_count) &&
					(MANAGER_CHANGE_NONE(cmzn_light) != object->manager_change_status))))
			{
				return_code = REMOVE_OBJECT_FROM_MANAGER(cmzn_light)(object, object->manager);
			}
			*object_address = static_cast<cmzn_light *>(0);
		}
		else
		{
			return_code = 0;
		}
		return (return_code);
	}

	virtual cmzn_light_change_detail *extract_change_detail()
	{
		return NULL;
	}

}; /* struct cmzn_light */

/* Only to be used from FIND_BY_IDENTIFIER_IN_INDEXED_LIST_STL function
 * Creates a pseudo object with name identifier suitable for finding
 * objects by identifier with cmzn_set.
 */
class cmzn_light_identifier : private cmzn_light
{
public:
	cmzn_light_identifier(const char *name)
	{
		cmzn_light::name = name;
	}

	~cmzn_light_identifier()
	{
		cmzn_light::name = NULL;
	}

	cmzn_light *getPseudoObject()
	{
		return this;
	}
};

/** functor for ordering cmzn_set<cmzn_light> by name */
struct cmzn_light_compare_name
{
	bool operator() (const cmzn_light* light1, const cmzn_light* light2) const
	{
		return strcmp(light1->name, light2->name) < 0;
	}
};

typedef cmzn_set<cmzn_light *,cmzn_light_compare_name> cmzn_set_cmzn_light;

struct cmzn_lightiterator : public cmzn_set_cmzn_light::ext_iterator
{
private:
	cmzn_lightiterator(cmzn_set_cmzn_light *container);
	cmzn_lightiterator(const cmzn_lightiterator&);
	~cmzn_lightiterator();

public:

	static cmzn_lightiterator *create(cmzn_set_cmzn_light *container)
	{
		return static_cast<cmzn_lightiterator *>(cmzn_set_cmzn_light::ext_iterator::create(container));
	}

	cmzn_lightiterator *access()
	{
		return static_cast<cmzn_lightiterator *>(this->cmzn_set_cmzn_light::ext_iterator::access());
	}

	static int deaccess(cmzn_lightiterator* &iterator)
	{
		cmzn_set_cmzn_light::ext_iterator* baseIterator = static_cast<cmzn_set_cmzn_light::ext_iterator*>(iterator);
		iterator = 0;
		return cmzn_set_cmzn_light::ext_iterator::deaccess(baseIterator);
	}

};

FULL_DECLARE_MANAGER_TYPE_WITH_OWNER(cmzn_light, cmzn_lightmodule, cmzn_light_change_detail *);

DECLARE_DEFAULT_MANAGER_UPDATE_DEPENDENCIES_FUNCTION(cmzn_light)

inline struct cmzn_light_change_detail *MANAGER_EXTRACT_CHANGE_DETAIL(cmzn_light)(
	cmzn_light *filter)
{
	return filter->extract_change_detail();
}

inline void MANAGER_CLEANUP_CHANGE_DETAIL(cmzn_light)(
	cmzn_light_change_detail **change_detail_address)
{
	delete *change_detail_address;
}

struct cmzn_light *cmzn_light_create_private()
{
	return cmzn_light::create();
}

DECLARE_MANAGER_UPDATE_FUNCTION(cmzn_light)

DECLARE_MANAGER_FIND_CLIENT_FUNCTION(cmzn_light)

DECLARE_MANAGED_OBJECT_NOT_IN_USE_CONDITIONAL_FUNCTION(cmzn_light)

PROTOTYPE_ACCESS_OBJECT_FUNCTION(cmzn_light)
{
	if (object)
		return object->access();
	return 0;
}

PROTOTYPE_DEACCESS_OBJECT_FUNCTION(cmzn_light)
{
	return cmzn_light::deaccess(object_address);
}

PROTOTYPE_REACCESS_OBJECT_FUNCTION(cmzn_light)
{
	if (object_address)
	{
		if (new_object)
		{
			new_object->access();
		}
		if (*object_address)
		{
			cmzn_light::deaccess(object_address);
		}
		*object_address = new_object;
		return 1;
	}
	return 0;
}

DECLARE_DEFAULT_GET_OBJECT_NAME_FUNCTION(cmzn_light)

DECLARE_INDEXED_LIST_STL_FUNCTIONS(cmzn_light)
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_STL_FUNCTION(cmzn_light,name,const char *)
DECLARE_INDEXED_LIST_STL_IDENTIFIER_CHANGE_FUNCTIONS(cmzn_light,name)
DECLARE_CREATE_INDEXED_LIST_STL_ITERATOR_FUNCTION(cmzn_light,cmzn_lightiterator)

DECLARE_MANAGER_FUNCTIONS(cmzn_light,manager)
DECLARE_DEFAULT_MANAGED_OBJECT_NOT_IN_USE_FUNCTION(cmzn_light,manager)
DECLARE_MANAGER_IDENTIFIER_WITHOUT_MODIFY_FUNCTIONS(cmzn_light,name,const char *,manager)
DECLARE_MANAGER_OWNER_FUNCTIONS(cmzn_light, struct cmzn_lightmodule)


cmzn_lightiterator *cmzn_lightmodule::createLightiterator()
{
	return CREATE_LIST_ITERATOR(cmzn_light)(this->lightManager->object_list);
}

int cmzn_light_manager_set_owner_private(struct MANAGER(cmzn_light) *manager,
	struct cmzn_lightmodule *lightmodule)
{
	return MANAGER_SET_OWNER(cmzn_light)(manager, lightmodule);
}

/*
Module variables
----------------
*/
const char *get_cmzn_light_name(struct cmzn_light *light)
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

static int direct_render_cmzn_light(struct cmzn_light *light)
/*******************************************************************************
LAST MODIFIED : 8 October 2002

DESCRIPTION :
Directly outputs the commands to activate the <light>.
==============================================================================*/
{
	int return_code;
	GLenum light_id;
	GLfloat values[4];

	if (light)
	{
		values[0] = 0.;
		values[1] = 0.;
		values[2] = 0.;
		values[3] = 1.;
		if (light->type != CMZN_LIGHT_TYPE_INVALID)
		{
			if (light->type == CMZN_LIGHT_TYPE_AMBIENT)
			{
				if (light->enabled == true)
				{
					values[0] = (GLfloat)((light->colour).red);
					values[1] = (GLfloat)((light->colour).green);
					values[2] = (GLfloat)((light->colour).blue);
					glLightModelfv(GL_LIGHT_MODEL_AMBIENT,values);
					if (CMZN_LIGHT_RENDER_VIEWER_MODE_LOCAL == light->viewer_mode)
					{
						glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,GL_TRUE);
					}
					else if (CMZN_LIGHT_RENDER_VIEWER_MODE_INFINITE == light->viewer_mode)
					{
						glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,GL_FALSE);
					}
					if (CMZN_LIGHT_RENDER_SIDE_DOUBLE==light->render_side)
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
			}
			else
			{
				if (next_light_no < MAXIMUM_NUMBER_OF_ACTIVE_LIGHTS && light->enabled == true)
				{
					light_id = light_identifiers[next_light_no];
					glLightfv(light_id, GL_AMBIENT, values);
					values[0] = (GLfloat)((light->colour).red);
					values[1] = (GLfloat)((light->colour).green);
					values[2] = (GLfloat)((light->colour).blue);
					glLightfv(light_id, GL_DIFFUSE, values);
					glLightfv(light_id, GL_SPECULAR, values);
					switch (light->type)
					{
						case CMZN_LIGHT_TYPE_DIRECTIONAL:
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
						case CMZN_LIGHT_TYPE_POINT:
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
						case CMZN_LIGHT_TYPE_SPOT:
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
						default:
						{
						} break;
					}
					glEnable(light_id);
					next_light_no++;
				}
			}
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE, "Cannot have any more lights on");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"direct_render_cmzn_light.  Missing light");
		return_code=0;
	}

	return (return_code);
} /* direct_render_cmzn_light */

/*
Global functions
----------------
*/

PROTOTYPE_ENUMERATOR_STRING_FUNCTION(cmzn_light_type)
{
	const char *enumerator_string;

	switch (enumerator_value)
	{
		case CMZN_LIGHT_TYPE_AMBIENT:
		{
			enumerator_string = "ambient";
		} break;
		case CMZN_LIGHT_TYPE_DIRECTIONAL:
		{
			enumerator_string = "infinite";
		} break;
		case CMZN_LIGHT_TYPE_POINT:
		{
			enumerator_string = "point";
		} break;
		case CMZN_LIGHT_TYPE_SPOT:
		{
			enumerator_string = "spot";
		} break;
		default:
		{
			enumerator_string = (const char *)NULL;
		} break;
	}

	return (enumerator_string);
} /* ENUMERATOR_STRING(cmzn_light_type) */

DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS(cmzn_light_type);

bool cmzn_light_is_enabled(struct cmzn_light *light)
{
	if (light)
	{
		return light->isEnabled();
	}

	return 0;
}

int cmzn_light_set_enabled(struct cmzn_light *light, bool enabled)
{
	if (light)
	{
		return light->setEnabled(enabled);
	}

	return CMZN_ERROR_ARGUMENT;
}

double cmzn_light_get_quadratic_attenuation(struct cmzn_light *light)
{
	if (light)
	{
		return light->getQuadraticAttenuation();
	}

	return 0.0;
}

int cmzn_light_set_quadratic_attenuation(struct cmzn_light *light, double quadratic_attenuation)
{
	if (light)
	{
		return light->setQuadraticAttenuation(quadratic_attenuation);
	}

	return CMZN_ERROR_ARGUMENT;
}

double cmzn_light_get_linear_attenuation(struct cmzn_light *light)
{
	if (light)
	{
		return light->getLinearAttenuation();
	}

	return 0.0;
}

int cmzn_light_set_linear_attenuation(struct cmzn_light *light, double linear_attenuation)
{
	if (light)
	{
		return light->setLinearAttenuation(linear_attenuation);
	}

	return CMZN_ERROR_ARGUMENT;
}

double cmzn_light_get_constant_attenuation(struct cmzn_light *light)
{
	if (light)
	{

		return light->getConstantAttenuation();
	}

	return 0.0;
}

int cmzn_light_set_constant_attenuation(struct cmzn_light *light, double constant_attenuation)
{
	if (light)
	{
		return light->setConstantAttenuation(constant_attenuation);
	}

	return CMZN_ERROR_ARGUMENT;
}

int cmzn_light_get_colour_rgb(struct cmzn_light *light, double *colour)
{
	if (light)
	{
		return light->getColour(colour);
	}

	return CMZN_ERROR_ARGUMENT;
} /* cmzn_light_get_colour_rgb */

int cmzn_light_set_colour_rgb(struct cmzn_light *light, const double *colour)

{
	if (light)
	{
		return light->setColour(colour);
	}

	return CMZN_ERROR_ARGUMENT;
} /* cmzn_light_set_colour_rgb */

int cmzn_light_get_direction(struct cmzn_light *light, double *direction)
{
	if (light)
		return light->getDirection(direction);
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_light_set_direction(struct cmzn_light *light, const double *direction)
{
	if (light)
		return light->setDirection(direction);
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_light_get_position(struct cmzn_light *light, double *position)
{
	if (light)
		return light->getPosition(position);
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_light_set_position(struct cmzn_light *light, const double *position)
{
	if (light)
		return light->setPosition(position);
	return CMZN_ERROR_ARGUMENT;
}

double cmzn_light_get_spot_cutoff(struct cmzn_light *light)
{
	if (light)
	{
		return light->getSpotCutoff();
	}

	return 0.0;
}

int cmzn_light_set_spot_cutoff(struct cmzn_light *light, double spot_cutoff)
{
	if (light)
	{
		return light->setSpotCutoff(spot_cutoff);
	}

	return CMZN_ERROR_ARGUMENT;
}

double cmzn_light_get_spot_exponent(struct cmzn_light *light)
{
	if (light)
	{
		return light->getSpotExponent();
	}

	return 0.0;
}

int cmzn_light_set_spot_exponent(struct cmzn_light *light, double spot_exponent)
{
	if (light)
	{
		return light->setSpotExponent(spot_exponent);
	}

	return CMZN_ERROR_ARGUMENT;
} /* cmzn_light_set_spot_exponent */

enum cmzn_light_render_side cmzn_light_get_render_side(struct cmzn_light *light)
{
	if (light)
	{
		return light->getRenderSide();
	}

	return CMZN_LIGHT_RENDER_SIDE_INVALID;
}

int cmzn_light_set_render_side(struct cmzn_light *light, enum cmzn_light_render_side render_side)
{
	if (light)
	{
		return light->setRenderSide(render_side);
	}

	return CMZN_ERROR_ARGUMENT;
}

enum cmzn_light_render_viewer_mode cmzn_light_get_render_viewer_mode(struct cmzn_light *light)
{
	if (light)
	{
		return light->getRenderViewerMode();
	}

	return CMZN_LIGHT_RENDER_VIEWER_MODE_INVALID;
}

int cmzn_light_set_render_viewer_mode(struct cmzn_light *light,
	enum cmzn_light_render_viewer_mode render_viewer_mode)
{
	if (light)
	{
		return light->setRenderViewerMode(render_viewer_mode);
	}

	return CMZN_ERROR_ARGUMENT;
}

enum cmzn_light_type cmzn_light_get_type(struct cmzn_light *light)
{
	if (light)
	{
		return light->getType();
	}

	return CMZN_LIGHT_TYPE_INVALID;
}

int cmzn_light_set_type(struct cmzn_light *light, enum cmzn_light_type light_type)
/*******************************************************************************
LAST MODIFIED : 4 December 1997

DESCRIPTION :
Sets the light_type of the light (infinite/point/spot).
==============================================================================*/
{
	if (light)
	{
		return light->setType(light_type);
	}

	return CMZN_ERROR_ARGUMENT;
}

int list_cmzn_light(struct cmzn_light *light,void *dummy)
/*******************************************************************************
LAST MODIFIED : 8 October 2002

DESCRIPTION :
Writes the properties of the <light> to the command window.
==============================================================================*/
{
	int return_code;

	ENTER(list_cmzn_light);
	USE_PARAMETER(dummy);
	if (light)
	{
		display_message(INFORMATION_MESSAGE, "light : %s : %s",
			light->name, ENUMERATOR_STRING(cmzn_light_type)(light->type));
		if (true == light->enabled)
		{
			display_message(INFORMATION_MESSAGE, " : enable");
		}
		else
		{
			display_message(INFORMATION_MESSAGE, " : disable");
		}
		if (light->type == CMZN_LIGHT_TYPE_AMBIENT)
		{
			if (light->viewer_mode == CMZN_LIGHT_RENDER_VIEWER_MODE_LOCAL)
			{
				display_message(INFORMATION_MESSAGE, " : local_viewer");
			}
			else
			{
				display_message(INFORMATION_MESSAGE, " : infinite_viewer");
			}
			if (light->render_side == CMZN_LIGHT_RENDER_SIDE_SINGLE)
			{
				display_message(INFORMATION_MESSAGE, " : single_sided");
			}
			else
			{
				display_message(INFORMATION_MESSAGE, " : double_sided");
			}
		}
		display_message(INFORMATION_MESSAGE,"\n");
		display_message(INFORMATION_MESSAGE,
			"  colour  red = %.3g, green = %.3g, blue = %.3g\n",
			(light->colour).red, (light->colour).green, (light->colour).blue);

		switch (light->type)
		{
			case CMZN_LIGHT_TYPE_POINT: case CMZN_LIGHT_TYPE_SPOT:
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
			case CMZN_LIGHT_TYPE_DIRECTIONAL: case CMZN_LIGHT_TYPE_SPOT:
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
			case CMZN_LIGHT_TYPE_POINT:
			case CMZN_LIGHT_TYPE_SPOT:
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
		if (CMZN_LIGHT_TYPE_SPOT == light->type)
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
		display_message(ERROR_MESSAGE,"list_cmzn_light.  Missing light");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_cmzn_light */

int list_cmzn_light_name(struct cmzn_light *light,void *preceding_text_void)
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

	ENTER(list_cmzn_light_name);
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
		display_message(ERROR_MESSAGE,"list_cmzn_light_name.  Missing light");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* list_cmzn_light_name */

int list_cmzn_light_name_command(struct cmzn_light *light,void *preceding_text_void)
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
		display_message(ERROR_MESSAGE,"list_cmzn_light_name_command.  Missing light");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* list_cmzn_light_name_command */

int reset_cmzn_lights(void)
/*******************************************************************************
LAST MODIFIED : 4 December 1997

DESCRIPTION :
Must be called at start of rendering before lights are activate with
execute_cmzn_light. Ensures all lights are off at the start of the rendering loop,
and makes sure the lights that are subsequently defined start at GL_LIGHT0...
==============================================================================*/
{
	int return_code,light_no;

	for (light_no=0;light_no<MAXIMUM_NUMBER_OF_ACTIVE_LIGHTS;light_no++)
	{
		glDisable(light_identifiers[light_no]);
	}
	next_light_no=0;
	return_code=1;


	return (return_code);
} /* reset_cmzn_lights */

int execute_cmzn_light(struct cmzn_light *light,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 4 December 1997

DESCRIPTION :
Struct cmzn_light iterator function for activating the <light>.
Does not use display lists. See comments with compile_cmzn_light, above.
==============================================================================*/
{
	USE_PARAMETER(dummy_void);

	return direct_render_cmzn_light(light);

}

int cmzn_light_is_in_list(struct cmzn_light *light, void *light_list_void)
/*******************************************************************************
LAST MODIFIED : 30 May 2001

DESCRIPTION :
Returns true if <light> is in <light_list>.
==============================================================================*/
{
	int return_code;
	struct LIST(cmzn_light) *light_list;

	ENTER(cmzn_light_is_in_list);
	if (light && (light_list = (struct LIST(cmzn_light) *)light_list_void))
	{
		return_code = IS_OBJECT_IN_LIST(cmzn_light)(light, light_list);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_light_is_in_list.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* cmzn_light_is_in_list */

struct MANAGER(cmzn_light) *cmzn_lightmodule_get_manager(cmzn_lightmodule *lightmodule)
{
	if (lightmodule)
		return lightmodule->getManager();
	return 0;
}

cmzn_lightmodule *cmzn_lightmodule_create()
{
	return cmzn_lightmodule::create();
}

cmzn_lightmodule *cmzn_lightmodule_access(
	cmzn_lightmodule *lightmodule)
{
	if (lightmodule)
		return lightmodule->access();
	return 0;
}

int cmzn_lightmodule_destroy(cmzn_lightmodule **lightmodule_address)
{
	if (lightmodule_address)
		return cmzn_lightmodule::deaccess(*lightmodule_address);
	return CMZN_ERROR_ARGUMENT;
}

cmzn_light *cmzn_lightmodule_create_light(
	cmzn_lightmodule *lightmodule)
{
	if (lightmodule)
		return lightmodule->createLight();
	return 0;
}

int cmzn_lightmodule_begin_change(cmzn_lightmodule *lightmodule)
{
	if (lightmodule)
		return lightmodule->beginChange();
   return CMZN_ERROR_ARGUMENT;
}

int cmzn_lightmodule_end_change(cmzn_lightmodule *lightmodule)
{
	if (lightmodule)
		return lightmodule->endChange();
   return CMZN_ERROR_ARGUMENT;
}

cmzn_light *cmzn_lightmodule_find_light_by_name(
	cmzn_lightmodule *lightmodule, const char *name)
{
	if (lightmodule)
		return lightmodule->findLightByName(name);
   return 0;
}

cmzn_light *cmzn_lightmodule_get_default_light(cmzn_lightmodule *lightmodule)
{
	if (lightmodule)
		return lightmodule->getDefaultLight();
	return 0;
}

int cmzn_lightmodule_set_default_light(cmzn_lightmodule *lightmodule, cmzn_light *light)
{
	if (lightmodule)
		return lightmodule->setDefaultLight(light);
	return 0;
}

cmzn_light *cmzn_lightmodule_get_default_ambient_light(cmzn_lightmodule *lightmodule)
{
	if (lightmodule)
		return lightmodule->getDefaultAmbientLight();
	return 0;
}

int cmzn_lightmodule_set_default_ambient_light(cmzn_lightmodule *lightmodule, cmzn_light *light)
{
	if (lightmodule)
		return lightmodule->setDefaultAmbientLight(light);
	return 0;
}

cmzn_light_id cmzn_light_access(cmzn_light_id light)
{
	if (light)
		return ACCESS(cmzn_light)(light);
	return 0;
}

int cmzn_light_destroy(cmzn_light_id *light_address)
{
	return DEACCESS(cmzn_light)(light_address);
}

char *cmzn_light_get_name(struct cmzn_light *light)
{
	char *name = NULL;
	if (light && light->name)
	{
		name = duplicate_string(light->name);
	}
	return name;
}

int cmzn_light_set_name(struct cmzn_light *light, const char *name)
{
	int return_code;

	ENTER(cmzn_light_set_name);
	if (light && name)
	{
		return_code = 1;
		cmzn_set_cmzn_light *manager_light_list = 0;
		bool restore_changed_object_to_lists = false;
		if (light->manager)
		{
			cmzn_light *existing_light =
				FIND_BY_IDENTIFIER_IN_MANAGER(cmzn_light, name)(name, light->manager);
			if (existing_light && (existing_light != light))
			{
				display_message(ERROR_MESSAGE, "cmzn_light_set_name.  "
					"light named '%s' already exists.", name);
				return_code = 0;
			}
			if (return_code)
			{
				manager_light_list = reinterpret_cast<cmzn_set_cmzn_light *>(
					light->manager->object_list);
				// this temporarily removes the object from all related lists
				restore_changed_object_to_lists =
					manager_light_list->begin_identifier_change(light);
				if (!restore_changed_object_to_lists)
				{
					display_message(ERROR_MESSAGE, "cmzn_light_set_name.  "
						"Could not safely change identifier in manager");
					return_code = 0;
				}
			}
		}
		if (return_code)
		{
			char *new_name = duplicate_string(name);
			if (new_name)
			{
				DEALLOCATE(light->name);
				light->name = new_name;
			}
			else
			{
				return_code = 0;
			}
		}
		if (restore_changed_object_to_lists)
		{
			manager_light_list->end_identifier_change();
		}
		if (light->manager && return_code)
		{
			MANAGED_OBJECT_CHANGE(cmzn_light)(light,
				MANAGER_CHANGE_IDENTIFIER(cmzn_light));
		}
	}
	else
	{
		if (light)
		{
			display_message(ERROR_MESSAGE,
				"cmzn_light_set_name.  Invalid light name '%s'", name);
		}
		return_code=0;
	}

	return (return_code);
}

bool cmzn_light_is_managed(cmzn_light_id light)
{
	if (light)
	{
		return light->is_managed_flag;
	}
	return 0;
}

int cmzn_light_set_managed(cmzn_light_id light,
	bool value)
{
	if (light)
	{
		bool old_value = light->is_managed_flag;
		light->is_managed_flag = (value != 0);
		if (value != old_value)
		{
			MANAGED_OBJECT_CHANGE(cmzn_light)(light,
				MANAGER_CHANGE_DEFINITION(cmzn_light));
		}
		return CMZN_OK;
	}
	return CMZN_ERROR_ARGUMENT;
}

cmzn_lightiterator_id cmzn_lightmodule_create_lightiterator(
	cmzn_lightmodule_id lightmodule)
{
	if (lightmodule)
		return lightmodule->createLightiterator();
	return 0;
}

cmzn_lightiterator_id cmzn_lightiterator_access(cmzn_lightiterator_id iterator)
{
	if (iterator)
		return iterator->access();
	return 0;
}

int cmzn_lightiterator_destroy(cmzn_lightiterator_id *iterator_address)
{
	if (!iterator_address)
		return 0;
	return cmzn_lightiterator::deaccess(*iterator_address);
}

cmzn_light_id cmzn_lightiterator_next(cmzn_lightiterator_id iterator)
{
	if (iterator)
		return iterator->next();
	return 0;
}
