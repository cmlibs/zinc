/*******************************************************************************
FILE : material.c

LAST MODIFIED : 24 November 1999

DESCRIPTION :
The functions for manipulating graphical materials.

???RC Only OpenGL is supported now.

???RC 28 Nov 97.
		Two functions are now used for activating the graphical material:
- compile_Graphical_material puts the material into its own display list.
- execute_Graphical_material calls that display list.
		The separation into two functions was needed because OpenGL cannot start a
new list while one is being written to. As a consequence, you must precompile
all objects that are executed from within another display list. To make this job
easier, compile_Graphical_material is a list/manager iterator function.
		Future updates of OpenGL may overcome this limitation, in which case the
execute function can take over compiling as well. Furthermore, it is easy to
return to direct rendering, as described with these routines.
==============================================================================*/
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "command/parser.h"
#include "general/debug.h"
#include "general/indexed_list_private.h"
#include "general/manager_private.h"
#include "general/mystring.h"
#include "general/object.h"
#include "graphics/graphics_library.h"
#include "graphics/material.h"
#include "graphics/texture.h"
#include "user_interface/message.h"

/*
Module constants
----------------
*/
#define MATERIAL_SPECTRUM_ENABLED  (1)
#define MATERIAL_SPECTRUM_STARTED  (2)
#define MATERIAL_SPECTRUM_DIFFUSE  (4)
#define MATERIAL_SPECTRUM_AMBIENT  (8)
#define MATERIAL_SPECTRUM_SPECULAR (16)
#define MATERIAL_SPECTRUM_EMISSION (32)
#define MATERIAL_SPECTRUM_ALPHA    (64)

/*
Module types
------------
*/
struct Graphical_material
/*******************************************************************************
LAST MODIFIED : 15 October 1998

DESCRIPTION :
The properties of a graphical material.
==============================================================================*/
{
	/* the name of the material */
	char *name;
	/* the colour of the background light (so scattered that its incident
		direction is unknown, comes from all directions) reflected */
	struct Colour ambient;
	/* the colour of the directional light scattered in all directions */
	struct Colour diffuse;
	/* the colour of the light emitted */
	struct Colour emission;
	/* the colour of the directional light which is reflected in a preferred
		direction */
	struct Colour specular;
	/* the transparency */
	MATERIAL_PRECISION alpha;
	/* how sharp and bright the glinting is */
	MATERIAL_PRECISION shininess;
#if defined (OPENGL_API)
	GLuint display_list;
#endif /* defined (OPENGL_API) */
	/* the spectrum_flag indicates to the direct render routine that
		the material is being used to represent a spectrum and so the
		commands generated from direct_render_Graphical_material can be 
		compacted, all the bits indicate what has been changed */
	int spectrum_flag;
	int spectrum_flag_previous;
	/* flag to say if the display list is up to date */
	int display_list_current;
	/* the texture for this material */
	struct Texture *texture;
	/* the number of structures that point to this material.  The material
		cannot be destroyed while this is greater than 0 */
	int access_count;
}; /* struct Graphical_material */

FULL_DECLARE_INDEXED_LIST_TYPE(Graphical_material);

FULL_DECLARE_MANAGER_TYPE(Graphical_material);

/*
Module variables
----------------
*/
#if defined (GL_API)
static short int next_graphical_material_index=1;
#endif

/*
Module functions
----------------
*/
DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Graphical_material,name,char *,strcmp)

DECLARE_LOCAL_MANAGER_FUNCTIONS(Graphical_material)

int direct_render_Graphical_material(struct Graphical_material *material)
/*******************************************************************************
LAST MODIFIED : 15 October 1998

DESCRIPTION :
Directly outputs the graphics library commands for activating <material>.
???RC Only supports OpenGL.
SAB Spectrums make extensive use of this function as they operate by copying the
normal material, modifying it according to the settings and then calling this
routine.  I expected this to make spectrums much slower as we are no longer
using the glColorMaterial function, didn't really seem to change.  However on
some other OPENGL implementations this could have a significant effect.  I was
going to try and get this routine to keep record of the current colour and use
the glColorMaterial function if it can however if this is done you must make
sure that when this routine is compiled into different display list the correct
material results.
==============================================================================*/
{
#if defined (OPENGL_API)
	GLfloat values[4];
#endif /* defined (OPENGL_API) */
	int return_code;

	ENTER(direct_render_Graphical_material);
	if (material)
	{
#if defined (OPENGL_API)
#if defined (OLD_CODE)
		switch (material->spectrum_flag)
		{
			case (MATERIAL_SPECTRUM_ENABLED | MATERIAL_SPECTRUM_STARTED | MATERIAL_SPECTRUM_DIFFUSE):
			case (MATERIAL_SPECTRUM_ENABLED | MATERIAL_SPECTRUM_STARTED | MATERIAL_SPECTRUM_ALPHA):
			case (MATERIAL_SPECTRUM_ENABLED | MATERIAL_SPECTRUM_STARTED | MATERIAL_SPECTRUM_DIFFUSE | MATERIAL_SPECTRUM_ALPHA):
			{
				if(material->spectrum_flag_previous != material->spectrum_flag)
				{
					material->spectrum_flag_previous = material->spectrum_flag;
					glEnable(GL_COLOR_MATERIAL);
					glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
				}
				values[0]=(material->diffuse).red;
				values[1]=(material->diffuse).green;
				values[2]=(material->diffuse).blue;
				values[3]=material->alpha;
				glColor4fv(values);				
			} break;
			case (MATERIAL_SPECTRUM_ENABLED | MATERIAL_SPECTRUM_STARTED | MATERIAL_SPECTRUM_AMBIENT | MATERIAL_SPECTRUM_DIFFUSE):
			{
				if(((material->diffuse).red==(material->ambient).red)
					&& ((material->diffuse).green==(material->ambient).green)
					&& ((material->diffuse).blue==(material->ambient).blue))
				{
					if(material->spectrum_flag_previous != material->spectrum_flag)
					{
						material->spectrum_flag_previous = material->spectrum_flag;
						glEnable(GL_COLOR_MATERIAL);
						glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
					}
					values[0]=(material->diffuse).red;
					values[1]=(material->diffuse).green;
					values[2]=(material->diffuse).blue;
					values[3]=material->alpha;
					glColor4fv(values);				
				}
				else
				{
					values[0]=(material->diffuse).red;
					values[1]=(material->diffuse).green;
					values[2]=(material->diffuse).blue;
					values[3]=material->alpha;
					glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,values);
					values[0]=(material->ambient).red;
					values[1]=(material->ambient).green;
					values[2]=(material->ambient).blue;
					glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,values);
				}
			} break;
			case (MATERIAL_SPECTRUM_ENABLED | MATERIAL_SPECTRUM_STARTED | MATERIAL_SPECTRUM_AMBIENT):
			{
				if(material->spectrum_flag_previous != material->spectrum_flag)
				{
					material->spectrum_flag_previous = material->spectrum_flag;
					glEnable(GL_COLOR_MATERIAL);
					glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT);
				}
				values[0]=(material->ambient).red;
				values[1]=(material->ambient).green;
				values[2]=(material->ambient).blue;
				values[3]=material->alpha;
				glColor4fv(values);				
			} break;
			case (MATERIAL_SPECTRUM_ENABLED | MATERIAL_SPECTRUM_STARTED | MATERIAL_SPECTRUM_EMISSION):
			{
				if(material->spectrum_flag_previous != material->spectrum_flag)
				{
					material->spectrum_flag_previous = material->spectrum_flag;
					glEnable(GL_COLOR_MATERIAL);
					glColorMaterial(GL_FRONT_AND_BACK, GL_EMISSION);
				}
				values[0]=(material->emission).red;
				values[1]=(material->emission).green;
				values[2]=(material->emission).blue;
				glColor3fv(values);				
			} break;
			case (MATERIAL_SPECTRUM_ENABLED | MATERIAL_SPECTRUM_STARTED | MATERIAL_SPECTRUM_SPECULAR):
			{
				if(material->spectrum_flag_previous != material->spectrum_flag)
				{
					material->spectrum_flag_previous = material->spectrum_flag;
					glEnable(GL_COLOR_MATERIAL);
					glColorMaterial(GL_FRONT_AND_BACK, GL_SPECULAR);
				}
				values[0]=(material->specular).red;
				values[1]=(material->specular).green;
				values[2]=(material->specular).blue;
				glColor3fv(values);				
			} break;
			default:
			{
#endif /* defined (OLD_CODE) */
				values[0]=(material->diffuse).red;
				values[1]=(material->diffuse).green;
				values[2]=(material->diffuse).blue;
				values[3]=material->alpha;
				/* use diffuse colour for lines, which are unlit */
#if defined (OLD_CODE)
				glColor3fv(values);
#endif /* defined (OLD_CODE) */
				/*???RC alpha translucency now applies to lines too: */
				glColor4fv(values);
				glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,values);
				values[0]=(material->ambient).red;
				values[1]=(material->ambient).green;
				values[2]=(material->ambient).blue;
				glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,values);
				values[0]=(material->emission).red;
				values[1]=(material->emission).green;
				values[2]=(material->emission).blue;
				values[3]=1.0;
				glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,values);
				values[0]=(material->specular).red;
				values[1]=(material->specular).green;
				values[2]=(material->specular).blue;
				glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,values);
				glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,(material->shininess)*128.);
				execute_Texture(material->texture);
				return_code=1;
#if defined (OLD_CODE)
			} break;
		}
#endif /* defined (OLD_CODE) */
		material->spectrum_flag = MATERIAL_SPECTRUM_STARTED |
			(MATERIAL_SPECTRUM_ENABLED & material->spectrum_flag);
		/* Indicates first render has been done */
#else /* defined (OPENGL_API) */
		display_message(ERROR_MESSAGE,
			"direct_render_Graphical_material.  Not defined for this API");
		return_code=0;
#endif /* defined (OPENGL_API) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"direct_render_Graphical_material.  Missing material");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* direct_render_Graphical_material */

/*
Global functions
----------------
*/
struct Graphical_material *CREATE(Graphical_material)(char *name)
/*******************************************************************************
LAST MODIFIED : 15 October 1998

DESCRIPTION :
Allocates memory and assigns fields for a material.
==============================================================================*/
{
	struct Graphical_material *material;

	ENTER(CREATE(Graphical_material));
	if (name)
	{
		/* allocate memory for structure */
		if (ALLOCATE(material,struct Graphical_material,1)&&
			ALLOCATE(material->name,char,strlen(name)+1))
		{
			strcpy(material->name,name);
			material->access_count=0;
			(material->ambient).red=1;
			(material->ambient).green=1;
			(material->ambient).blue=1;
			(material->diffuse).red=1;
			(material->diffuse).green=1;
			(material->diffuse).blue=1;
			material->alpha=1;
			(material->emission).red=0;
			(material->emission).green=0;
			(material->emission).blue=0;
			(material->specular).red=0;
			(material->specular).green=0;
			(material->specular).blue=0;
			material->shininess=0;
			material->texture=(struct Texture *)NULL;
			material->spectrum_flag=0;
			material->spectrum_flag_previous=0;
#if defined (OPENGL_API)
			material->display_list=0;
#endif /* defined (OPENGL_API) */
			material->display_list_current=0;
		}
		else
		{
			if (material)
			{
				DEALLOCATE(material);
			}
			display_message(ERROR_MESSAGE,
				"CREATE(Graphical_material).  Not enough memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Graphical_material).  Missing name");
		material=(struct Graphical_material *)NULL;
	}
	LEAVE;

	return (material);
} /* CREATE(Graphical_material) */

int DESTROY(Graphical_material)(struct Graphical_material **material_address)
/*******************************************************************************
LAST MODIFIED : 3 August 1998

DESCRIPTION :
Frees the memory for the material and sets <*material_address> to NULL.
==============================================================================*/
{
	int return_code;
	struct Graphical_material *material;

	ENTER(DESTROY(Graphical_material));
	if (material_address&&(material= *material_address))
	{
		if (0==material->access_count)
		{
			DEALLOCATE(material->name);
#if defined (OPENGL_API)
			if (material->display_list)
			{
				glDeleteLists(material->display_list,1);
			}
#endif /* defined (OPENGL_API) */
			if (material->texture)
			{
				DEACCESS(Texture)(&(material->texture));
			}
			DEALLOCATE(*material_address);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"DESTROY(Graphical_material).  Material %s has non-zero access count",
				material->name);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Graphical_material).  Missing material");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Graphical_material) */

DECLARE_OBJECT_FUNCTIONS(Graphical_material)
DECLARE_DEFAULT_GET_OBJECT_NAME_FUNCTION(Graphical_material)
DECLARE_INDEXED_LIST_FUNCTIONS(Graphical_material)
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Graphical_material,name,
	char *,strcmp)

PROTOTYPE_MANAGER_COPY_WITH_IDENTIFIER_FUNCTION(Graphical_material,name)
{
	char *name;
	int return_code;

	ENTER(MANAGER_COPY_WITH_IDENTIFIER(Graphical_material,name));
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
"MANAGER_COPY_WITH_IDENTIFIER(Graphical_material,name).  Insufficient memory");
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
			if (return_code = MANAGER_COPY_WITHOUT_IDENTIFIER(Graphical_material,name)(
				destination, source))
			{
				/* copy values */
				DEALLOCATE(destination->name);
				destination->name=name;
			}
			else
			{
				DEALLOCATE(name);
				display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITH_IDENTIFIER(Graphical_material,name).  Could not copy without identifier");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITH_IDENTIFIER(Graphical_material,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITH_IDENTIFIER(Graphical_material,name) */

PROTOTYPE_MANAGER_COPY_WITHOUT_IDENTIFIER_FUNCTION(Graphical_material,name)
{
	int return_code;

	ENTER(MANAGER_COPY_WITHOUT_IDENTIFIER(Graphical_material,name));
	/* check arguments */
	if (source&&destination)
	{
		/* copy values */
		(destination->diffuse).red=(source->diffuse).red;
		(destination->diffuse).green=(source->diffuse).green;
		(destination->diffuse).blue=(source->diffuse).blue;
		(destination->ambient).red=(source->ambient).red;
		(destination->ambient).green=(source->ambient).green;
		(destination->ambient).blue=(source->ambient).blue;
		(destination->emission).red=(source->emission).red;
		(destination->emission).green=(source->emission).green;
		(destination->emission).blue=(source->emission).blue;
		(destination->specular).red=(source->specular).red;
		(destination->specular).green=(source->specular).green;
		(destination->specular).blue=(source->specular).blue;
		destination->shininess=source->shininess;
		destination->alpha=source->alpha;
		if (source->texture)
		{
			ACCESS(Texture)(source->texture);
		}
		if (destination->texture)
		{
			DEACCESS(Texture)(&(destination->texture));
		}
		destination->texture=source->texture;
		/* flag destination display list as no longer current */
		destination->display_list_current=0;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
"MANAGER_COPY_WITHOUT_IDENTIFIER(Graphical_material,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITHOUT_IDENTIFIER(Graphical_material,name) */

PROTOTYPE_MANAGER_COPY_IDENTIFIER_FUNCTION(Graphical_material,name,char *)
{
	char *destination_name;
	int return_code;

	ENTER(MANAGER_COPY_IDENTIFIER(Graphical_material,name));
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
			"MANAGER_COPY_IDENTIFIER(Graphical_material,name).  Insufficient memory");
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
			"MANAGER_COPY_IDENTIFIER(Graphical_material,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_IDENTIFIER(Graphical_material,name) */

DECLARE_MANAGER_FUNCTIONS(Graphical_material)
DECLARE_MANAGER_IDENTIFIER_FUNCTIONS(Graphical_material,name,char *)

char *Graphical_material_name(struct Graphical_material *material)
/*******************************************************************************
LAST MODIFIED : 29 November 1997

DESCRIPTION :
While the GET_NAME macro returns a copy of the name of an object, this function
has been created for returning just a pointer to the material's name, or some
other string if the name is invalid, suitable for putting in printf statements.
Be careful with the returned value: esp. do not modify or DEALLOCATE it!
==============================================================================*/
{
	char *return_name;
	static char error_string[]="ERROR";
	static char no_name_string[]="NO NAME";

	ENTER(Graphical_material_name);
	if (material)
	{
		if (material->name)
		{
			return_name=material->name;
		}
		else
		{
			return_name=no_name_string;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_material_name.  Missing material");
		return_name=error_string;
	}
	LEAVE;

	return (return_name);
} /* Graphical_material_name */

int Graphical_material_get_ambient(struct Graphical_material *material,
	struct Colour *ambient)
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Returns the ambient colour of the material.
==============================================================================*/
{
	int return_code;

	ENTER(Graphical_material_get_ambient);
	if (material&&ambient)
	{
		ambient->red=material->ambient.red;
		ambient->green=material->ambient.green;
		ambient->blue=material->ambient.blue;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_material_get_ambient.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphical_material_get_ambient */

int Graphical_material_set_ambient(struct Graphical_material *material,
	struct Colour *ambient)
/*******************************************************************************
LAST MODIFIED : 15 October 1998

DESCRIPTION :
Sets the ambient colour of the material.
==============================================================================*/
{
	int return_code;

	ENTER(Graphical_material_set_ambient);
	if (material&&ambient)
	{
		material->ambient.red=ambient->red;
		material->ambient.green=ambient->green;
		material->ambient.blue=ambient->blue;
		material->spectrum_flag |= MATERIAL_SPECTRUM_AMBIENT;
		/* display list needs to be compiled again */
		material->display_list_current=0;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_material_set_ambient.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphical_material_set_ambient */

int Graphical_material_get_diffuse(struct Graphical_material *material,
	struct Colour *diffuse)
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Returns the diffuse colour of the material.
==============================================================================*/
{
	int return_code;

	ENTER(Graphical_material_get_diffuse);
	if (material&&diffuse)
	{
		diffuse->red=material->diffuse.red;
		diffuse->green=material->diffuse.green;
		diffuse->blue=material->diffuse.blue;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_material_get_diffuse.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphical_material_get_diffuse */

int Graphical_material_set_diffuse(struct Graphical_material *material,
	struct Colour *diffuse)
/*******************************************************************************
LAST MODIFIED : 15 October 1998

DESCRIPTION :
Sets the diffuse colour of the material.
==============================================================================*/
{
	int return_code;

	ENTER(Graphical_material_set_diffuse);
	if (material&&diffuse)
	{
		material->diffuse.red=diffuse->red;
		material->diffuse.green=diffuse->green;
		material->diffuse.blue=diffuse->blue;
		material->spectrum_flag |= MATERIAL_SPECTRUM_DIFFUSE;
		/* display list needs to be compiled again */
		material->display_list_current=0;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_material_set_diffuse.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphical_material_set_diffuse */

int Graphical_material_get_emission(struct Graphical_material *material,
	struct Colour *emission)
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Returns the emission colour of the material.
==============================================================================*/
{
	int return_code;

	ENTER(Graphical_material_get_emission);
	if (material&&emission)
	{
		emission->red=material->emission.red;
		emission->green=material->emission.green;
		emission->blue=material->emission.blue;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_material_get_emission.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphical_material_get_emission */

int Graphical_material_set_emission(struct Graphical_material *material,
	struct Colour *emission)
/*******************************************************************************
LAST MODIFIED : 15 October 1998

DESCRIPTION :
Sets the emission colour of the material.
==============================================================================*/
{
	int return_code;

	ENTER(Graphical_material_set_emission);
	if (material&&emission)
	{
		material->emission.red=emission->red;
		material->emission.green=emission->green;
		material->emission.blue=emission->blue;
		material->spectrum_flag |= MATERIAL_SPECTRUM_EMISSION;
		/* display list needs to be compiled again */
		material->display_list_current=0;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_material_set_emission.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphical_material_set_emission */

int Graphical_material_get_specular(struct Graphical_material *material,
	struct Colour *specular)
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Returns the specular colour of the material.
==============================================================================*/
{
	int return_code;

	ENTER(Graphical_material_get_specular);
	if (material&&specular)
	{
		specular->red=material->specular.red;
		specular->green=material->specular.green;
		specular->blue=material->specular.blue;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_material_get_specular.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphical_material_get_specular */

int Graphical_material_set_specular(struct Graphical_material *material,
	struct Colour *specular)
/*******************************************************************************
LAST MODIFIED : 15 October 1998

DESCRIPTION :
Sets the specular colour of the material.
==============================================================================*/
{
	int return_code;

	ENTER(Graphical_material_set_specular);
	if (material&&specular)
	{
		material->specular.red=specular->red;
		material->specular.green=specular->green;
		material->specular.blue=specular->blue;
		material->spectrum_flag |= MATERIAL_SPECTRUM_SPECULAR;
		/* display list needs to be compiled again */
		material->display_list_current=0;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_material_set_specular.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphical_material_set_specular */

int Graphical_material_get_alpha(struct Graphical_material *material,
	MATERIAL_PRECISION *alpha)
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Returns the alpha value of the material.
==============================================================================*/
{
	int return_code;

	ENTER(Graphical_material_get_alpha);
	if (material&&alpha)
	{
		*alpha=material->alpha;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_material_get_alpha.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphical_material_get_alpha */

int Graphical_material_set_alpha(struct Graphical_material *material,
	MATERIAL_PRECISION alpha)
/*******************************************************************************
LAST MODIFIED : 15 October 1998

DESCRIPTION :
Sets the alpha value of the material.
==============================================================================*/
{
	int return_code;

	ENTER(Graphical_material_set_alpha);
	if (material&&(0.0 <= alpha)&&(1.0 >= alpha))
	{
		material->alpha=alpha;
		material->spectrum_flag |= MATERIAL_SPECTRUM_ALPHA;
		/* display list needs to be compiled again */
		material->display_list_current=0;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_material_set_alpha.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphical_material_set_alpha */

int Graphical_material_get_shininess(struct Graphical_material *material,
	MATERIAL_PRECISION *shininess)
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Returns the shininess value of the material.
==============================================================================*/
{
	int return_code;

	ENTER(Graphical_material_get_shininess);
	if (material&&shininess)
	{
		*shininess=material->shininess;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_material_get_shininess.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphical_material_get_shininess */

int Graphical_material_set_shininess(struct Graphical_material *material,
	MATERIAL_PRECISION shininess)
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Sets the shininess value of the material.
==============================================================================*/
{
	int return_code;

	ENTER(Graphical_material_set_shininess);
	if (material&&(0.0 <= shininess)&&(1.0 >= shininess))
	{
		material->shininess=shininess;
		/* display list needs to be compiled again */
		material->display_list_current=0;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_material_set_shininess.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphical_material_set_shininess */

struct Texture *Graphical_material_get_texture(
	struct Graphical_material *material)
/*******************************************************************************
LAST MODIFIED : 12 February 1998

DESCRIPTION :
Returns the texture member of the material.
==============================================================================*/
{
	struct Texture *texture;

	ENTER(Graphical_material_get_texture);
	if (material)
	{
		texture=material->texture;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_material_get_texture.  Missing material");
		texture=(struct Texture *)NULL;
	}
	LEAVE;

	return (texture);
} /* Graphical_material_get_texture */

int Graphical_material_set_texture(struct Graphical_material *material,
	struct Texture *texture)
/*******************************************************************************
LAST MODIFIED : 12 February 1998

DESCRIPTION :
Sets the texture member of the material.
==============================================================================*/
{
	int return_code;

	ENTER(Graphical_material_set_texture);
	if (material)
	{
		if (texture)
		{
			ACCESS(Texture)(texture);
		}
		if (material->texture)
		{
			DEACCESS(Texture)(&material->texture);
		}
		material->texture=texture;
		/* display list needs to be compiled again */
		material->display_list_current=0;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_material_set_texture.  Missing material");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphical_material_set_texture */

int Graphical_material_uses_texture(struct Graphical_material *material,
	void *texture_void)
/*******************************************************************************
LAST MODIFIED : 15 June 1998

DESCRIPTION :
Returns 1 if the <material> uses texture, or any texture if <texture> is NULL.
==============================================================================*/
{
	int return_code;
	struct Texture *texture;

	ENTER(Graphical_material_uses_texture);
	if (material)
	{
		texture=(struct Texture *)texture_void;
		return_code=(material->texture&&((!texture)||(texture==material->texture)));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_material_uses_texture.  Missing material");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphical_material_uses_texture */

int Graphical_material_set_spectrum_flag(struct Graphical_material *material)
/*******************************************************************************
LAST MODIFIED : 15 October 1998

DESCRIPTION :
Sets the spectrum_flag of a material giving it the cue to intelligently issue
commands from direct_render_Graphical_material.
==============================================================================*/
{
	int return_code;

	ENTER(Graphical_material_set_spectrum_flag);
	if (material)
	{
		material->spectrum_flag |= MATERIAL_SPECTRUM_ENABLED;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphical_material_set_spectrum_flag.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphical_material_set_spectrum_flag */

int modify_Graphical_material(struct Parse_state *state,void *material_void,
	void *modify_graphical_material_data_void)
/*******************************************************************************
LAST MODIFIED : 17 February 1997

DESCRIPTION :
==============================================================================*/
{
	auto struct Modifier_entry
		help_option_table[]=
		{
			{"MATERIAL_NAME",NULL,NULL,modify_Graphical_material},
			{NULL,NULL,NULL,NULL}
		},
		option_table[]=
		{
			{"alpha",NULL,NULL,set_float_0_to_1_inclusive},
			{"ambient",NULL,NULL,set_Colour},
			{"diffuse",NULL,NULL,set_Colour},
			{"emission",NULL,NULL,set_Colour},
			{"shininess",NULL,NULL,set_float_0_to_1_inclusive},
			{"specular",NULL,NULL,set_Colour},
			{"texture",NULL,NULL,set_Texture},
			{NULL,NULL,NULL,NULL}
		};
	char *current_token;
	int process,return_code;
	struct Graphical_material *material_to_be_modified,
		*material_to_be_modified_copy;
	struct Modify_graphical_material_data *modify_graphical_material_data;

	ENTER(modify_Graphical_material);
	if (state)
	{
		if (modify_graphical_material_data=(struct Modify_graphical_material_data *)
			modify_graphical_material_data_void)
		{
			if (current_token=state->current_token)
			{
				process=0;
				if (material_to_be_modified=(struct Graphical_material *)material_void)
				{
					if (IS_MANAGED(Graphical_material)(material_to_be_modified,
						modify_graphical_material_data->graphical_material_manager))
					{
						if (material_to_be_modified_copy=CREATE(Graphical_material)("copy"))
						{
							MANAGER_COPY_WITHOUT_IDENTIFIER(Graphical_material,name)(
								material_to_be_modified_copy,material_to_be_modified);
							process=1;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"modify_Graphical_material.  Could not create material copy");
							return_code=0;
						}
					}
					else
					{
						material_to_be_modified_copy=material_to_be_modified;
						material_to_be_modified=(struct Graphical_material *)NULL;
						process=1;
					}
				}
				else
				{
					if (strcmp(PARSER_HELP_STRING,current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
					{
						if (material_to_be_modified=FIND_BY_IDENTIFIER_IN_MANAGER(
							Graphical_material,name)(current_token,
							modify_graphical_material_data->graphical_material_manager))
						{
							if (return_code=shift_Parse_state(state,1))
							{
								if (material_to_be_modified_copy=CREATE(Graphical_material)(
									"copy"))
								{
									MANAGER_COPY_WITHOUT_IDENTIFIER(Graphical_material,name)(
										material_to_be_modified_copy,material_to_be_modified);
									process=1;
								}
								else
								{
									display_message(ERROR_MESSAGE,
									"modify_Graphical_material.  Could not create material copy");
									return_code=0;
								}
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"Unknown material : %s",
								current_token);
							display_parse_state_location(state);
							return_code=0;
						}
					}
					else
					{
						if (material_to_be_modified=CREATE(Graphical_material)("help"))
						{
							if (modify_graphical_material_data->default_graphical_material)
							{
								MANAGER_COPY_WITHOUT_IDENTIFIER(Graphical_material,name)(
									material_to_be_modified,modify_graphical_material_data->
									default_graphical_material);
							}
							(help_option_table[0]).to_be_modified=
								(void *)material_to_be_modified;
							(help_option_table[0]).user_data=
								modify_graphical_material_data_void;
							return_code=process_option(state,help_option_table);
							DESTROY(Graphical_material)(&material_to_be_modified);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"modify_Graphical_material.  Could not create dummy material");
							return_code=0;
						}
					}
				}
				if (process)
				{
					/*???RC shouldn't be passing addresses in object: */
					/*why? so we can separate parsing from the object */
					/* use local variables instead. Left for later. */
					(option_table[0]).to_be_modified=
						&(material_to_be_modified_copy->alpha);
					(option_table[1]).to_be_modified=
						&(material_to_be_modified_copy->ambient);
					(option_table[2]).to_be_modified=
						&(material_to_be_modified_copy->diffuse);
					(option_table[3]).to_be_modified=
						&(material_to_be_modified_copy->emission);
					(option_table[4]).to_be_modified=
						&(material_to_be_modified_copy->shininess);
					(option_table[5]).to_be_modified=
						&(material_to_be_modified_copy->specular);
					(option_table[6]).to_be_modified=
						&(material_to_be_modified_copy->texture);
					(option_table[6]).user_data=
						modify_graphical_material_data->texture_manager;
					if (return_code=process_multiple_options(state,option_table))
					{
						if (material_to_be_modified)
						{
							MANAGER_MODIFY_NOT_IDENTIFIER(Graphical_material,name)(
								material_to_be_modified,material_to_be_modified_copy,
								modify_graphical_material_data->graphical_material_manager);
							DESTROY(Graphical_material)(&material_to_be_modified_copy);
						}
						else
						{
							material_to_be_modified=material_to_be_modified_copy;
						}
					}
				}
			}
			else
			{
				if (material_void)
				{
					display_message(ERROR_MESSAGE,"Missing material modifications");
				}
				else
				{
					display_message(ERROR_MESSAGE,"Missing material name");
				}
				display_parse_state_location(state);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
		"modify_Graphical_material.  Missing modify_graphical_material_data_void");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"modify_Graphical_material.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* modify_Graphical_material */

int list_Graphical_material(struct Graphical_material *material,void *dummy)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Writes the properties of the <material> to the command window.
==============================================================================*/
{
	char line[80],*texture_name;
	int return_code;

	ENTER(list_Graphical_material);
	USE_PARAMETER(dummy);
	/* check the arguments */
	if (material)
	{
		display_message(INFORMATION_MESSAGE,"material : ");
		display_message(INFORMATION_MESSAGE,material->name);
		display_message(INFORMATION_MESSAGE,"\n");
		sprintf(line,"  access count = %i\n",material->access_count);
		display_message(INFORMATION_MESSAGE,line);
		sprintf(line,"  diffuse  red = %.3g, green = %.3g, blue = %.3g\n",
			(material->diffuse).red,(material->diffuse).green,
			(material->diffuse).blue);
		display_message(INFORMATION_MESSAGE,line);
		sprintf(line,"  ambient  red = %.3g, green = %.3g, blue = %.3g\n",
			(material->ambient).red,(material->ambient).green,
			(material->ambient).blue);
		display_message(INFORMATION_MESSAGE,line);
		sprintf(line,"  alpha = %.3g\n",material->alpha);
		display_message(INFORMATION_MESSAGE,line);
		sprintf(line,"  emission  red = %.3g, green = %.3g, blue = %.3g\n",
			(material->emission).red,(material->emission).green,
			(material->emission).blue);
		display_message(INFORMATION_MESSAGE,line);
		sprintf(line,"  specular  red = %.3g, green = %.3g, blue = %.3g\n",
			(material->specular).red,(material->specular).green,
			(material->specular).blue);
		display_message(INFORMATION_MESSAGE,line);
		sprintf(line,"  shininess = %.3g\n",material->shininess);
		display_message(INFORMATION_MESSAGE,line);
		if (material->texture&&GET_NAME(Texture)(material->texture,&texture_name))
		{
			display_message(INFORMATION_MESSAGE,"  texture : ");
			display_message(INFORMATION_MESSAGE,texture_name);
			display_message(INFORMATION_MESSAGE,"\n");
			DEALLOCATE(texture_name);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Graphical_material.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* list_Graphical_material */

int list_Graphical_material_commands(struct Graphical_material *material,
	void *command_prefix_void)
/*******************************************************************************
LAST MODIFIED : 2 December 1998

DESCRIPTION :
Writes on the command window the command needed to recreate the <material>.
The command is started with the string pointed to by <command_prefix>.
==============================================================================*/
{
	char *command_prefix,line[100],*name;
	int return_code;

	ENTER(list_Graphical_material_commands);
	if (material&&(command_prefix=(char *)command_prefix_void))
	{
		display_message(INFORMATION_MESSAGE,command_prefix);
		if (name=duplicate_string(material->name))
		{
			/* put quotes around name if it contains special characters */
			make_valid_token(&name);
			display_message(INFORMATION_MESSAGE,name);
			DEALLOCATE(name);
		}
		sprintf(line," ambient %g %g %g",
			(material->ambient).red,(material->ambient).green,
			(material->ambient).blue);
		display_message(INFORMATION_MESSAGE,line);
		sprintf(line," diffuse %g %g %g",
			(material->diffuse).red,(material->diffuse).green,
			(material->diffuse).blue);
		display_message(INFORMATION_MESSAGE,line);
		sprintf(line," emission %g %g %g",
			(material->emission).red,(material->emission).green,
			(material->emission).blue);
		display_message(INFORMATION_MESSAGE,line);
		sprintf(line," specular %g %g %g",
			(material->specular).red,(material->specular).green,
			(material->specular).blue);
		display_message(INFORMATION_MESSAGE,line);
		sprintf(line," alpha %g",material->alpha);
		display_message(INFORMATION_MESSAGE,line);
		sprintf(line," shininess %g",material->shininess);
		display_message(INFORMATION_MESSAGE,line);
		if (material->texture&&GET_NAME(Texture)(material->texture,&name))
		{
			/* put quotes around name if it contains special characters */
			make_valid_token(&name);
			display_message(INFORMATION_MESSAGE," texture %s",name);
			DEALLOCATE(name);
		}
		display_message(INFORMATION_MESSAGE,"\n");
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Graphical_material_commands.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* list_Graphical_material_commands */

int file_read_Graphical_material_name(FILE *file,
	struct Graphical_material **material_address,
	struct MANAGER(Graphical_material) *graphical_material_manager)
/*******************************************************************************
LAST MODIFIED : 20 June 1996

DESCRIPTION :
Reads a material name from a <file>.  Searchs the list of all materials for one
with the specified name.  If one is not found a new one is created with the
specified name and the default properties.
==============================================================================*/
{
	char *material_name;
	int return_code;
	struct Graphical_material *material;

	ENTER(file_read_Graphical_material_name);
	/* check the arguments */
	if (file&&material_address)
	{
		if (read_string(file,"s",&material_name))
		{
			/*???DB.  Should this read function be in another module ? */
			if (material=FIND_BY_IDENTIFIER_IN_MANAGER(Graphical_material,name)(
				material_name,graphical_material_manager))
			{
				*material_address=material;
				return_code=1;
			}
			else
			{
				if (material=CREATE(Graphical_material)(material_name))
				{
					if (ADD_OBJECT_TO_MANAGER(Graphical_material)(material,
						graphical_material_manager))
					{
						*material_address=material;
						return_code=1;
					}
					else
					{
						DESTROY(Graphical_material)(&material);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"file_read_Graphical_material_name.  Could not create material");
					return_code=0;
				}
			}
			DEALLOCATE(material_name);
		}
		else
		{
			display_message(ERROR_MESSAGE,
			"file_read_Graphical_material_name.  Error reading material name strin");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"file_read_Graphical_material_name.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* file_read_Graphical_material_name */

#if defined (OLD_CODE)
int activate_Graphical_material(struct Graphical_material *material)
/*******************************************************************************
LAST MODIFIED : 28 November 1997

DESCRIPTION :
Directly outputs the graphics library commands for activating <material>.
==============================================================================*/
{
	int return_code;
#if defined (GL_API)
	float values[21];
#endif
#if defined (OPENGL_API)
	GLfloat values[4];
#endif

	ENTER(activate_Graphical_material);
	if (material)
	{
#if defined (GL_API)
		if (material->index<=0)
		{
			if (material->index<0)
			{
				material->index= -(material->index);
			}
			else
			{
				material->index=next_graphical_material_index;
				next_graphical_material_index++;
			}
			values[0]=AMBIENT;
			values[1]=(material->ambient).red;
			values[2]=(material->ambient).green;
			values[3]=(material->ambient).blue;
			values[4]=DIFFUSE;
			values[5]=(material->diffuse).red;
			values[6]=(material->diffuse).green;
			values[7]=(material->diffuse).blue;
			values[8]=ALPHA;
			values[9]=material->alpha;
			values[10]=EMISSION;
			values[11]=(material->emission).red;
			values[12]=(material->emission).green;
			values[13]=(material->emission).blue;
			values[14]=SPECULAR;
			values[15]=(material->specular).red;
			values[16]=(material->specular).green;
			values[17]=(material->specular).blue;
			values[18]=SHININESS;
			values[19]=(material->shininess)*128.;
			values[20]=LMNULL;
			lmdef(DEFMATERIAL,material->index,21,values);
		}
		if (material->index>0)
		{
			lmbind(MATERIAL,material->index);
		}
#endif
#if defined (OPENGL_API)
#if defined (MS_22AUG96)
		if (0==material->list_index)
		{
			if (material->list_index=glGenLists(1))
			{
				glNewList(material->list_index,GL_COMPILE);
#endif /* defined (MS_22AUG96) */
				values[0]=(material->diffuse).red;
				values[1]=(material->diffuse).green;
				values[2]=(material->diffuse).blue;
				values[3]=material->alpha;
				glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE,values);
				values[0]=(material->ambient).red;
				values[1]=(material->ambient).green;
				values[2]=(material->ambient).blue;
				glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT,values);
				values[0]=(material->emission).red;
				values[1]=(material->emission).green;
				values[2]=(material->emission).blue;
				glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,values);
				values[0]=(material->specular).red;
				values[1]=(material->specular).green;
				values[2]=(material->specular).blue;
				glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,values);
				glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,
					(material->shininess)*128.);
#if defined (MS_22AUG96)
				glEndList();
			}
		}
		if (material->list_index)
		{
			glCallList(material->list_index);
		}
#endif /* defined (MS_22AUG96) */
#endif
			activate_Texture(material->texture);
			return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"activate_Graphical_material.  Missing material");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* activate_Graphical_material */
#endif /* defined (OLD_CODE) */

int compile_Graphical_material(struct Graphical_material *material,
	void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Graphical_material list/manager iterator function.
Rebuilds the display_list for <material> if it is not current. If <material>
does not have a display list, first attempts to give it one. The display list
created here may be called using execute_Graphical_material, below.
???RC Graphical_materials must be compiled before they are executed since openGL
cannot start writing to a display list when one is currently being written to.
???RC The behaviour of materials is set up to take advantage of pre-computed
display lists. To switch to direct rendering make this routine do nothing and
execute_Graphical_material should just call direct_render_Graphical_material.
==============================================================================*/
{
	int return_code;

	ENTER(compile_Graphical_material);
	USE_PARAMETER(dummy_void);
	if (material)
	{
		if (material->display_list_current)
		{
			return_code=1;
		}
		else
		{
#if defined (OPENGL_API)
			if (material->display_list||(material->display_list=glGenLists(1)))
			{
				/*???RC compile texture here just to be safe */
				if (material->texture)
				{
					compile_Texture(material->texture,NULL);
				}
				glNewList(material->display_list,GL_COMPILE);
				direct_render_Graphical_material(material);
				glEndList();
				material->display_list_current=1;
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"compile_Graphical_material.  Could not generate display list");
				return_code=0;
			}
#else /* defined (OPENGL_API) */
			display_message(ERROR_MESSAGE,
				"compile_Graphical_material.  Not defined for this API");
			return_code=0;
#endif /* defined (OPENGL_API) */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"compile_Graphical_material.  Missing material");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* compile_Graphical_material */

int execute_Graphical_material(struct Graphical_material *material)
/*******************************************************************************
LAST MODIFIED : 20 April 1998

DESCRIPTION :
Activates <material> by calling its display list. If the display list is not
current, an error is reported.
???RC The behaviour of materials is set up to take advantage of pre-computed
display lists. To switch to direct rendering this routine should just call
direct_render_Graphical_material.
==============================================================================*/
{
	int return_code;

	ENTER(execute_Graphical_material);
	if (material)
	{
#if defined (OPENGL_API)
		if (material->display_list_current)
		{
			glCallList(material->display_list);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"execute_Graphical_material.  Display list not current");
			return_code=0;
		}
#else /* defined (OPENGL_API) */
		display_message(ERROR_MESSAGE,
			"execute_Graphical_material.  Not defined for this API");
		return_code=0;
#endif /* defined (OPENGL_API) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_Graphical_material.  Missing material");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_Graphical_material */

int set_Graphical_material(struct Parse_state *state,
	void *material_address_void,void *graphical_material_manager_void)
/*******************************************************************************
LAST MODIFIED : 22 June 1999

DESCRIPTION :
Modifier function to set the material from a command.
???RC set_Object routines could become a macro.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Graphical_material *temp_material,**material_address;
	struct MANAGER(Graphical_material) *graphical_material_manager;

	ENTER(set_Graphical_material);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if ((material_address=
					(struct Graphical_material **)material_address_void)&&
					(graphical_material_manager=(struct MANAGER(Graphical_material) *)
					graphical_material_manager_void))
				{
					if (fuzzy_string_compare(current_token,"NONE"))
					{
						if (*material_address)
						{
							DEACCESS(Graphical_material)(material_address);
							*material_address=(struct Graphical_material *)NULL;
						}
						return_code=1;
					}
					else
					{
						if (temp_material=FIND_BY_IDENTIFIER_IN_MANAGER(Graphical_material,
							name)(current_token,graphical_material_manager))
						{
							if (*material_address!=temp_material)
							{
								DEACCESS(Graphical_material)(material_address);
								*material_address=ACCESS(Graphical_material)(temp_material);
							}
							return_code=1;
						}
						else
						{
							display_message(ERROR_MESSAGE,"Unknown material : %s",
								current_token);
							return_code=0;
						}
					}
					shift_Parse_state(state,1);
				}
				else
				{
					display_message(ERROR_MESSAGE,
"set_Graphical_material.  Invalid argument(s).  material_address %p.  material_manager %p",
						material_address_void,graphical_material_manager_void);
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," MATERIAL_NAME|none");
				/* if possible, then write the name */
				if (material_address=
					(struct Graphical_material **)material_address_void)
				{
					if (temp_material= *material_address)
					{
						display_message(INFORMATION_MESSAGE,"[%s]",temp_material->name);
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
			display_message(WARNING_MESSAGE,"Missing material name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Graphical_material.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Graphical_material */
