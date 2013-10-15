/*******************************************************************************
FILE : render_vrml.cpp

LAST MODIFIED : 20 March 2003

DESCRIPTION :
Renders gtObjects to VRML file
==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "zinc/graphicsmaterial.hpp"
#include "general/debug.h"
#include "general/list.h"
#include "general/list_private.h"
#include "general/object.h"
#include "general/mystring.h"
#include "graphics/glyph.hpp"
#include "graphics/graphics_object.h"
#include "graphics/material.h"
#include "graphics/render_vrml.h"
#include "graphics/spectrum.h"
#include "graphics/texture.h"
#include "region/cmiss_region.h"
#include "general/message.h"
#include "graphics/scene.h"
#include "graphics/scene.hpp"
#include "graphics/graphics_object_private.hpp"

/*
Module types
------------
*/

struct VRML_prototype
/*******************************************************************************
LAST MODIFIED : 9 May 1999

DESCRIPTION :
Used to keep track of what objects have been defined.
PROTOtyping is very slow so has been phased out.
==============================================================================*/
{
	char *name;
	struct Texture *texture;
	struct Graphical_material *material;
	struct GT_object *graphics_object;
	/* time only needed for graphics_object prototype */
	ZnReal time;
	int access_count;
}; /* struct VRML_prototype */

DECLARE_LIST_TYPES(VRML_prototype);
FULL_DECLARE_LIST_TYPE(VRML_prototype);

/*
Module functions
----------------
*/

static double clamped_acos(double x)
/*******************************************************************************
LAST MODIFIED : 9 May 1999

DESCRIPTION :
Protects us from errors due to numbers being close but outside the range of
-1 to 1.
==============================================================================*/
{
	double result, tolerance;

	ENTER(clamped_acos);

	tolerance = 1e-3;
	if (x > 1.0)
	{
		if (x > 1.0 + tolerance)
		{
			display_message(WARNING_MESSAGE,
				"clamped_acos.  Value passed to acos is greater than 1.0 plus tolerance.");
		}
		result = 0.0;
	}
	else if (x < -1.0)
	{
		if (x < -1.0 - tolerance)
		{
			display_message(WARNING_MESSAGE,
				"clamped_acos.  Value passed to acos is less than -1.0 minus tolerance.");
		}
		result = PI;
	}
	else
	{
		result = acos(x);
	}

	LEAVE;

	return (result);
} /* clamped_acos */

static struct VRML_prototype *CREATE(VRML_prototype)(char *name,
	struct Texture *texture,
	struct Graphical_material *material,
	struct GT_object *graphics_object,ZnReal time)
/*******************************************************************************
LAST MODIFIED : 9 May 1999

DESCRIPTION :
Creates a VRML_prototype for the supplied object - only one of texture/material/
graphics_object may be set. The name must be unique for any object in the file.
Used for ensuring PROTOtypes for VRML objects are written in the right order
such that they exist at the time they are to be used.
==============================================================================*/
{
	struct VRML_prototype *vrml_prototype;

	ENTER(CREATE(VRML_prototype));
	if (name)
	{
		if (ALLOCATE(vrml_prototype,struct VRML_prototype,1)&&
			ALLOCATE(vrml_prototype->name,char,strlen(name)+1))
		{
			sprintf(vrml_prototype->name,"%s",name);
			/* do not ACCESS objects as vrml_prototypes are temporary structures */
			vrml_prototype->texture=texture;
			vrml_prototype->material=material;
			vrml_prototype->graphics_object=graphics_object;
			vrml_prototype->time=time;
			vrml_prototype->access_count=0;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(VRML_prototype).  Not enough memory");
			DEALLOCATE(vrml_prototype);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(VRML_prototype).  Invalid argument(s)");
		vrml_prototype=(struct VRML_prototype *)NULL;
	}
	LEAVE;

	return (vrml_prototype);
} /* CREATE(VRML_prototype) */

static int DESTROY(VRML_prototype)(
	struct VRML_prototype **vrml_prototype_address)
/*******************************************************************************
LAST MODIFIED : 9 May 1999

DESCRIPTION :
Frees the memory for <**vrml_prototype_address> and sets
<*vrml_prototype_address> to NULL.
==============================================================================*/
{
	int return_code;

	ENTER(DESTROY(VRML_prototype));
	if (vrml_prototype_address&&(*vrml_prototype_address))
	{
		if (0==(*vrml_prototype_address)->access_count)
		{
			DEALLOCATE((*vrml_prototype_address)->name);
			DEALLOCATE(*vrml_prototype_address);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,"DESTROY(VRML_prototype).  "
				"Non-zero access count of %d",(*vrml_prototype_address)->access_count);
			*vrml_prototype_address=(struct VRML_prototype *)NULL;
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(VRML_prototype).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(VRML_prototype) */

DECLARE_OBJECT_FUNCTIONS(VRML_prototype)
DECLARE_LIST_FUNCTIONS(VRML_prototype)

static int VRML_prototype_is_identical(struct VRML_prototype *vrml_prototype,
	void *prototype_void)
/*******************************************************************************
LAST MODIFIED : 9 June 1999

DESCRIPTION :
Conditional function returning true if <vrml_prototype> refers to <material>.
Used with FIRST_OBJECT_IN_LIST_THAT to find if a request to prototype this
material is already in the list.
==============================================================================*/
{
	int return_code;
	struct VRML_prototype *prototype;

	ENTER(VRML_prototype_is_identical);

	if (vrml_prototype&&(prototype=(struct VRML_prototype *)prototype_void))
	{
		return_code=((vrml_prototype->material == prototype->material)
			&&(vrml_prototype->graphics_object == prototype->graphics_object)
			&&(vrml_prototype->texture == prototype->texture));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"VRML_prototype_is_identical.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* VRML_prototype_is_identical */

/* Forward declaration of write_graphics_object_vrml function for glyph_set */
static int write_graphics_object_vrml(FILE *vrml_file,
	struct GT_object *gt_object,ZnReal time,
	struct LIST(VRML_prototype) *vrml_prototype_list,
	int object_is_glyph,struct Graphical_material *default_material,
	int gt_object_already_prototyped);

static int write_texture_vrml(FILE *file,struct Texture *texture)
/*******************************************************************************
LAST MODIFIED : 11 February 2002

DESCRIPTION :
Writes VRML that defines the texture.
==============================================================================*/
{
	char *texture_name;
	ZnReal width, height, depth;
	int width_texels, height_texels, depth_texels, return_code;

	ENTER(write_texture_vrml);
	if (texture && file)
	{
		Texture_get_size(texture, &width_texels, &height_texels, &depth_texels);
		if (1 != depth_texels)
		{
			GET_NAME(Texture)(texture, &texture_name);
			display_message(WARNING_MESSAGE,
				"write_texture_vrml.  VRML97 file cannot use 3-D texture %s; "
				"file will refer to nonexistent 3-D image series template %s",
				texture_name, Texture_get_image_file_name(texture));
			DEALLOCATE(texture_name);
		}
		Texture_get_physical_size(texture, &width, &height, &depth);
		fprintf(file,"texture  ImageTexture\n{\n");
		fprintf(file,"  url \"%s\"\n",Texture_get_image_file_name(texture));
		fprintf(file,"} #ImageTexture\n");
		fprintf(file,"textureTransform  TextureTransform\n{\n");
		fprintf(file,"  translation 0 0\n" );
		fprintf(file,"  rotation 0\n" );
		fprintf(file,"  scale %f %f\n", 1./width , 1./height );
		fprintf(file,"  center	0.0 0.0\n" );
		fprintf(file,"} #TextureTransform\n");
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_texture_vrml.  Missing texture or FILE handle");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* write_texture_vrml */

static int write_material_node_vrml(FILE *vrml_file,
	struct Graphical_material *material,int emissive_only)
/*******************************************************************************
LAST MODIFIED : 5 May 1999

DESCRIPTION :
Writes a VRML Material node to <file> for the <material>. If <emissive_only> is
set, only the emissive material is output. This is used for lines and points
with no lighting/normals.
==============================================================================*/
{
	int return_code;
	MATERIAL_PRECISION alpha,shininess;
	struct Colour diffuse,ambient,specular,emissive;

	ENTER(write_material_node_vrml);
	if (vrml_file&&material)
	{
		fprintf(vrml_file,"Material {\n");
		if (emissive_only)
		{
			/* use the diffuse colour for emissive */
			if (Graphical_material_get_diffuse(material,&emissive))
			{
				fprintf(vrml_file,"  emissiveColor %f %f %f\n",
					emissive.red,emissive.green,emissive.blue);
			}
		}
		else
		{
			if (Graphical_material_get_diffuse(material,&diffuse))
			{
				fprintf(vrml_file,"  diffuseColor %f %f %f\n",
					diffuse.red,diffuse.green,diffuse.blue);
			}
			if (Graphical_material_get_ambient(material,&ambient))
			{
				fprintf(vrml_file,"  ambientIntensity %f\n",
					(ambient.red+ambient.green+ambient.blue)/3.0);
			}
			if (Graphical_material_get_emission(material,&emissive))
			{
				fprintf(vrml_file,"  emissiveColor %f %f %f\n",
					emissive.red,emissive.green,emissive.blue);
			}
			if (Graphical_material_get_specular(material,&specular))
			{
				fprintf(vrml_file,"  specularColor %f %f %f\n",
					specular.red,specular.green,specular.blue);
			}
			if (Graphical_material_get_alpha(material,&alpha))
			{
				fprintf(vrml_file,"  transparency %f\n",1.0-alpha);
			}
			if (Graphical_material_get_shininess(material,&shininess))
			{
				fprintf(vrml_file,"  shininess %f\n",shininess);
			}
		}
		fprintf(vrml_file,"} #Material\n");
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_material_node_vrml.  Missing material or FILE handle");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* write_material_node_vrml */

static int activate_material_vrml(FILE *vrml_file,
	struct Graphical_material *material,
	struct LIST(VRML_prototype) *vrml_prototype_list,
	int no_define,int emissive_only)
/*******************************************************************************
LAST MODIFIED : 9 May 1999

DESCRIPTION :
==============================================================================*/
{
	char *dot_pointer, *material_name;
	int return_code = 0;
	struct VRML_prototype *vrml_prototype;

	ENTER(activate_material_vrml);
	if (vrml_file&&material)
	{
		if (emissive_only || no_define || (!vrml_prototype_list))
		{
			return_code=write_material_node_vrml(vrml_file,material,emissive_only);
		}
		else
		{
			if (GET_NAME(Graphical_material)(material,&material_name))
			{
				/* Can't have . in a name */
				while (NULL != (dot_pointer = strchr(material_name, '.')))
				{
					*dot_pointer = '_';
				}
				vrml_prototype=CREATE(VRML_prototype)(material_name,(struct Texture *)NULL,
					material,(struct GT_object *)NULL,/*time*/0);
				if (FIRST_OBJECT_IN_LIST_THAT(VRML_prototype)(VRML_prototype_is_identical,
					(void *)vrml_prototype,vrml_prototype_list))
				{
					fprintf(vrml_file,"USE %s\n",material_name);
					DESTROY(VRML_prototype)(&vrml_prototype);
					return_code = 1;
				}
				else
				{
					fprintf(vrml_file,"DEF %s ",material_name);
					return_code=write_material_node_vrml(vrml_file,material,emissive_only);
					ADD_OBJECT_TO_LIST(VRML_prototype)(vrml_prototype,vrml_prototype_list);
					fprintf(vrml_file,"#END DEF %s\n",material_name);
				}
				DEALLOCATE(material_name);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"activate_material_vrml.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* activate_material_vrml */

static int spectrum_start_render_vrml(FILE *vrml_file,struct Spectrum *spectrum,
	struct Graphical_material *material)
/*******************************************************************************
LAST MODIFIED : 6 May 1999

DESCRIPTION :
Sets VRML file for rendering values on the current material.
==============================================================================*/
{
	int return_code;

	ENTER(spectrum_start_render_vrml);
	if (spectrum && material)
	{
		fprintf(vrml_file,"color Color{\n");
		fprintf(vrml_file,"  color [\n");
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_start_render_vrml.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* spectrum_start_render_vrml */

static int spectrum_render_vrml_value(FILE *vrml_file,struct Spectrum *spectrum,
	struct Graphical_material *material,int number_of_data_components,GLfloat *data)
/*******************************************************************************
LAST MODIFIED : 6 May 1999

DESCRIPTION :
Writes VRML to represent the value 'data' in accordance with the spectrum.
==============================================================================*/
{
	int return_code;
	ZnReal rgba[4];

	ENTER(spectrum_render_vrml_value);
	if (spectrum&&material)
	{
		FE_value *feData = new FE_value[number_of_data_components];
		CAST_TO_FE_VALUE(feData,data,number_of_data_components);
		Spectrum_value_to_rgba(spectrum,number_of_data_components,feData,rgba);
		fprintf(vrml_file,"    %f %f %f,\n",rgba[0],rgba[1],rgba[2]);
		return_code=1;
		delete[] feData;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_render_vrml_value.  Invalid arguments given.");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* spectrum_render_vrml_value */

static int spectrum_end_render_vrml(FILE *vrml_file,struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 6 May 1999

DESCRIPTION :
Resets the graphics state after rendering values on current material.
==============================================================================*/
{
	int return_code;

	ENTER(spectrum_end_render_vrml);
	if (spectrum)
	{
		Spectrum_end_value_to_rgba(spectrum);
		fprintf(vrml_file,"  ]\n");
		fprintf(vrml_file,"} #Color\n");
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"spectrum_end_render_vrml.  Invalid spectrum object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* spectrum_end_render_vrml */

static int get_orthogonal_axes(ZnReal a1,ZnReal a2,ZnReal a3,
	ZnReal *b1,ZnReal *b2,ZnReal *b3,ZnReal *c1,ZnReal *c2,ZnReal *c3)
/*******************************************************************************
LAST MODIFIED : 11 May 1999

DESCRIPTION :
Given unit vector axis a=(a1,a2,a3), returns a pair of unit vectors
b=(b1,b2,b3) and c=(c1,c2,c3) such that a = b (x) c.
==============================================================================*/
{
	ZnReal length;
	int return_code;

	ENTER(get_orthogonal_axes);
	if (b1&&b2&&b3&&c1&&c2&&c3&&(0.00001>fabs(sqrt(a1*a1+a2*a2+a3*a3)-1.0)))
	{
		*b1=0.0;
		*b2=0.0;
		*b3=0.0;
		/* make b have 1.0 in the component with the least absolute value in a */
		if (fabs(a1) < fabs(a2))
		{
			if (fabs(a3) < fabs(a1))
			{
				*b3=1.0;
			}
			else
			{
				*b1=1.0;
			}
		}
		else
		{
			if (fabs(a3) < fabs(a2))
			{
				*b3=1.0;
			}
			else
			{
				*b2=1.0;
			}
		}
		/* set c = a (x) b */
		(*c1) = a2*(*b3) - a3*(*b2);
		(*c2) = a3*(*b1) - a1*(*b3);
		(*c3) = a1*(*b2) - a2*(*b1);
		/* normalize c */
		length = sqrt((*c1)*(*c1) + (*c2)*(*c2) + (*c3)*(*c3));
		(*c1) /= length;
		(*c2) /= length;
		(*c3) /= length;
		/* set b = c (x) a */
		*b1 = (*c2)*a3-(*c3)*a2;
		*b2 = (*c3)*a1-(*c1)*a3;
		*b3 = (*c1)*a2-(*c2)*a1;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"get_orthogonal_axes.  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* get_orthogonal_axes */

/**
 * Defines an object for the <glyph> and then draws that at <number_of_points>
 * points  given by the positions in <point_list> and oriented and scaled by
 * <axis1_list>, <axis2_list> and <axis3_list>.
 */
static int draw_glyph_set_vrml(FILE *vrml_file, GT_glyph_set *glyph_set,
	struct Graphical_material *material, struct Spectrum *spectrum, ZnReal time,
	struct LIST(VRML_prototype) *vrml_prototype_list)
{
	int return_code = 1;
	if (glyph_set)
	{
		const int number_of_points = glyph_set->number_of_points;
		if (0 == number_of_points)
			return 1;
		int number_of_data_components = glyph_set->n_data_components;
		const bool data_spectrum = (0 < number_of_data_components) && (0 != glyph_set->data) &&
			(0 != spectrum) && (0 != material);
		if (glyph_set->data && (!spectrum))
		{
			display_message(WARNING_MESSAGE,"draw_glyph_set_vrml.  Missing spectrum");
		}
		Triple *point = glyph_set->point_list;
		Triple *axis1 = glyph_set->axis1_list;
		Triple *axis2 = glyph_set->axis2_list;
		Triple *axis3 = glyph_set->axis3_list;
		Triple *scale = glyph_set->scale_list;
		Triple temp_axis1, temp_axis2, temp_axis3, temp_point;
		char **label;
		ZnReal a1, a2, a3, a_angle, a_magnitude, ax1, ax2, ax3, b1, b2, b3, b_angle,
			bx1, bx2, bx3, c1, c2, c3, cx1, cx2, cx3, dp, f0, f1, j1, j2, j3,
			c_magnitude, s1, s2, s3, x, y, z;
		int i, j, number_of_skew_glyph_axes, skewed_axes;
		struct Graphical_material *material_copy;
		GT_object *glyph = glyph_set->glyph;
		cmzn_glyph_repeat_mode glyph_repeat_mode = glyph_set->glyph_repeat_mode;
		/* try to draw points and lines faster */
		cmzn_glyph_type glyph_type = glyph ?
			GT_object_get_glyph_type(glyph) : CMZN_GLYPH_NONE;
		if ((glyph_type == CMZN_GLYPH_POINT) && (
			(glyph_repeat_mode == CMZN_GLYPH_REPEAT_NONE) ||
			(glyph_repeat_mode == CMZN_GLYPH_REPEAT_MIRROR)))
		{
			fprintf(vrml_file,"Shape {\n");
			fprintf(vrml_file,"  appearance\n");
			if (material)
			{
				fprintf(vrml_file,"Appearance {\n");
				fprintf(vrml_file,"  material\n");
				activate_material_vrml(vrml_file,material,
					(struct LIST(VRML_prototype) *)NULL,
					/*no_define_material*/0,/*emissive_only*/1);
				fprintf(vrml_file,"} #Appearance\n");
			}
			else
			{
				fprintf(vrml_file,"IS line_appearance\n");
			}
			fprintf(vrml_file,"  geometry PointSet {\n");
			fprintf(vrml_file,"    coord Coordinate {\n");
			fprintf(vrml_file,"      point [\n");
			for (i=0;i<number_of_points;i++)
			{
				fprintf(vrml_file,"        %f %f %f,\n",
					(*point)[0],(*point)[1],(*point)[2]);
				point++;
			}
			fprintf(vrml_file,"      ]\n");
			fprintf(vrml_file,"    }\n");
			if (data_spectrum)
			{
				spectrum_start_render_vrml(vrml_file,spectrum,material);
				for (i=0;i<number_of_points;i++)
				{
					spectrum_render_vrml_value(vrml_file,spectrum,material,
						number_of_data_components,glyph_set->data+i*number_of_data_components);
				}
				spectrum_end_render_vrml(vrml_file, spectrum);
			}
			fprintf(vrml_file,"  } #Pointset\n");
			fprintf(vrml_file,"} #Shape\n");
		}
		else if ((glyph_type == CMZN_GLYPH_LINE) && (
			(glyph_repeat_mode == CMZN_GLYPH_REPEAT_NONE) ||
			(glyph_repeat_mode == CMZN_GLYPH_REPEAT_MIRROR)))
		{
			const ZnReal f = (glyph_repeat_mode == CMZN_GLYPH_REPEAT_MIRROR) ? -1.0 : 0.0;
			fprintf(vrml_file,"Shape {\n");
			fprintf(vrml_file,"  appearance\n");
			if (material)
			{
				fprintf(vrml_file,"Appearance {\n");
				fprintf(vrml_file,"  material\n");
				activate_material_vrml(vrml_file,material,
					(struct LIST(VRML_prototype) *)NULL,
					/*no_define_material*/0,/*emissive_only*/1);
				fprintf(vrml_file,"} #Appearance\n");
			}
			else
			{
				fprintf(vrml_file,"IS line_appearance\n");
			}
			fprintf(vrml_file,"  geometry IndexedLineSet {\n");
			fprintf(vrml_file,"    coord Coordinate {\n");
			fprintf(vrml_file,"      point [\n");
			for (i=0;i<number_of_points;i++)
			{
				f0 = f*(*scale)[0];
				x = (*point)[0] + f0*(*axis1)[0];
				y = (*point)[1] + f0*(*axis1)[1];
				z = (*point)[2] + f0*(*axis1)[2];
				fprintf(vrml_file,"        %f %f %f,\n",x,y,z);
				f1 = (*scale)[0];
				x = (*point)[0] + f1*(*axis1)[0];
				y = (*point)[1] + f1*(*axis1)[1];
				z = (*point)[2] + f1*(*axis1)[2];
				fprintf(vrml_file,"        %f %f %f,\n",x,y,z);
				point++;
				scale++;
				axis1++;
			}
			fprintf(vrml_file,"      ]\n");
			fprintf(vrml_file,"    }\n");
			if (data_spectrum)
			{
				fprintf(vrml_file,"    colorPerVertex FALSE\n");
				spectrum_start_render_vrml(vrml_file,spectrum,material);
				for (i=0;i<number_of_points;i++)
				{
					spectrum_render_vrml_value(vrml_file,spectrum,material,
						number_of_data_components,glyph_set->data+i*number_of_data_components);
				}
				spectrum_end_render_vrml(vrml_file, spectrum);
			}
			fprintf(vrml_file,"    coordIndex [\n");
			for (i=0;i<number_of_points;i++)
			{
				fprintf(vrml_file,"      %d,%d,-1,\n",2*i,2*i+1);
			}
			fprintf(vrml_file,"    ]\n");
			fprintf(vrml_file,"  } #IndexedLineSet\n");
			fprintf(vrml_file,"} #Shape\n");
		}
		else if ((glyph_type == CMZN_GLYPH_CROSS) &&
			(glyph_repeat_mode == CMZN_GLYPH_REPEAT_NONE))
		{
			fprintf(vrml_file,"Shape {\n");
			fprintf(vrml_file,"  appearance\n");
			if (material)
			{
				fprintf(vrml_file,"Appearance {\n");
				fprintf(vrml_file,"  material\n");
				activate_material_vrml(vrml_file,material,
					(struct LIST(VRML_prototype) *)NULL,
					/*no_define_material*/0,/*emissive_only*/1);
				fprintf(vrml_file,"} #Appearance\n");
			}
			else
			{
				fprintf(vrml_file,"IS line_appearance\n");
			}
			fprintf(vrml_file,"  geometry IndexedLineSet {\n");
			fprintf(vrml_file,"    coord Coordinate {\n");
			fprintf(vrml_file,"      point [\n");
			for (i=0;i<number_of_points;i++)
			{
				resolve_glyph_axes(glyph_repeat_mode, /*glyph_number*/0,
					glyph_set->base_size, glyph_set->scale_factors, glyph_set->offset,
					*point, *axis1, *axis2, *axis3, *scale,
					temp_point, temp_axis1, temp_axis2, temp_axis3);
				/* x-line */
				x = temp_point[0] - 0.5*temp_axis1[0];
				y = temp_point[1] - 0.5*temp_axis1[1];
				z = temp_point[2] - 0.5*temp_axis1[2];
				fprintf(vrml_file,"        %f %f %f,\n",x,y,z);
				x += temp_axis1[0];
				y += temp_axis1[1];
				z += temp_axis1[2];
				fprintf(vrml_file,"        %f %f %f,\n",x,y,z);
				/* y-line */
				x = temp_point[0] - 0.5*temp_axis2[0];
				y = temp_point[1] - 0.5*temp_axis2[1];
				z = temp_point[2] - 0.5*temp_axis2[2];
				fprintf(vrml_file,"        %f %f %f,\n",x,y,z);
				x += temp_axis2[0];
				y += temp_axis2[1];
				z += temp_axis2[2];
				fprintf(vrml_file,"        %f %f %f,\n",x,y,z);
				/* z-line */
				x = temp_point[0] - 0.5*temp_axis3[0];
				y = temp_point[1] - 0.5*temp_axis3[1];
				z = temp_point[2] - 0.5*temp_axis3[2];
				fprintf(vrml_file,"        %f %f %f,\n",x,y,z);
				x += temp_axis3[0];
				y += temp_axis3[1];
				z += temp_axis3[2];
				fprintf(vrml_file,"        %f %f %f,\n",x,y,z);
				point++;
				axis1++;
				axis2++;
				axis3++;
				scale++;
			}
			fprintf(vrml_file,"      ]\n");
			fprintf(vrml_file,"    }\n");
			if (data_spectrum)
			{
				fprintf(vrml_file,"    colorPerVertex FALSE\n");
				spectrum_start_render_vrml(vrml_file,spectrum,material);
				for (i=0;i<number_of_points;i++)
				{
					for (j=0;j<3;j++)
					{
						spectrum_render_vrml_value(vrml_file,spectrum,material,
							number_of_data_components,glyph_set->data+i*number_of_data_components);
					}
				}
				spectrum_end_render_vrml(vrml_file, spectrum);
			}
			fprintf(vrml_file,"    coordIndex [\n");
			for (i=0;i<number_of_points;i++)
			{
				fprintf(vrml_file,"      %d,%d,-1,%d,%d,-1,%d,%d,-1,\n",
					6*i,6*i+1,6*i+2,6*i+3,6*i+4,6*i+5);
			}
			fprintf(vrml_file,"    ]\n");
			fprintf(vrml_file,"  } #IndexedLineSet\n");
			fprintf(vrml_file,"} #Shape\n");
		}
		else if (glyph) // general case
		{
			if (data_spectrum)
			{
				material_copy=cmzn_graphics_material_create_private();
				cmzn_graphics_material_set_name(material_copy, "render_vrml_copy");
			}
			else
			{
				material_copy = (struct Graphical_material *)NULL;
			}
			number_of_skew_glyph_axes=0;
			const int number_of_glyphs =
				cmzn_glyph_repeat_mode_get_number_of_glyphs(glyph_repeat_mode);
			for (i = 0; i < number_of_points; i++)
			{
				for (int glyph_number = 0; glyph_number < number_of_glyphs; glyph_number++)
				{
					resolve_glyph_axes(glyph_repeat_mode, glyph_number,
						glyph_set->base_size, glyph_set->scale_factors, glyph_set->offset,
						*point, *axis1, *axis2, *axis3, *scale,
						temp_point, temp_axis1, temp_axis2, temp_axis3);
					/* get the glyph centre as x, y and z */
					x = temp_point[0];
					y = temp_point[1];
					z = temp_point[2];
					/* store 3 axes in ax, bx and cx */
					ax1 = temp_axis1[0];
					ax2 = temp_axis1[1];
					ax3 = temp_axis1[2];
					bx1 = temp_axis2[0];
					bx2 = temp_axis2[1];
					bx3 = temp_axis2[2];
					cx1 = temp_axis3[0];
					cx2 = temp_axis3[1];
					cx3 = temp_axis3[2];
					/* get magnitudes of the glyph axes and normalise ax, bx and cx */
					if (0.0 < (s1 = sqrt(ax1*ax1 + ax2*ax2 + ax3*ax3)))
					{
						ax1 /= s1;
						ax2 /= s1;
						ax3 /= s1;
					}
					if (0.0 < (s2 = sqrt(bx1*bx1 + bx2*bx2 + bx3*bx3)))
					{
						bx1 /= s2;
						bx2 /= s2;
						bx3 /= s2;
					}
					if (0.0 < (s3 = sqrt(cx1*cx1 + cx2*cx2 + cx3*cx3)))
					{
						cx1 /= s3;
						cx2 /= s3;
						cx3 /= s3;
					}
					/* the three axes are either unit length or zero. Checks:
							If all three are non-zero, check that the axes are all mutually
						  orthogonal.
							If two are non-zero, make sure they are othogonal and get the
						  third from the cross product.
							If only one is non-zero, choose any two axes orthogonal to the
						  first as the remaining axes. */
					skewed_axes=0;
					if ((0.0<s1)&&(0.0<s2)&&(0.0<s3))
					{
						/* 3 non-zero axes: check dot product of axis1 (x) axis2 and
								axis2 (x) axis3 are both zero or thereabouts */
						if ((0.00001 < fabs(ax1*bx1 + ax2*bx2 + ax3*bx3))||
							(0.00001 < fabs(bx1*cx1 + bx2*cx2 + bx3*cx3)))
						{
							skewed_axes=1;
						}
					}
					else if (((0.0<s1)&&(0.0<s2)) ||
						((0.0<s2)&&(0.0<s3)) || ((0.0<s3)&&(0.0<s1)))
					{
						/* 2 non-zero axes: check their dot product is zero or
								thereabouts, and get third axis as their cross product. */
						if (0.0==s1)
						{
							if (0.00001 < fabs(bx1*cx1 + bx2*cx2 + bx3*cx3))
							{
								skewed_axes=1;
							}
							else
							{
								ax1 = bx2*cx3 - bx3*cx2;
								ax2 = bx3*cx1 - bx1*cx3;
								ax3 = bx1*cx2 - bx2*cx1;
							}
						}
						else if (0.0==s2)
						{
							if (0.00001 < fabs(cx1*ax1 + cx2*ax2 + cx3*ax3))
							{
								skewed_axes=1;
							}
							else
							{
								bx1 = cx2*ax3 - cx3*ax2;
								bx2 = cx3*ax1 - cx1*ax3;
								bx3 = cx1*ax2 - cx2*ax1;
							}
						}
						else /* if (0.0==s3) */
						{
							if (0.00001 < fabs(ax1*bx1 + ax2*bx2 + ax3*bx3))
							{
								skewed_axes=1;
							}
							else
							{
								cx1 = ax2*bx3 - ax3*bx2;
								cx2 = ax3*bx1 - ax1*bx3;
								cx3 = ax1*bx2 - ax2*bx1;
							}
						}
					}
					else if ((0.0<s1)||(0.0<s2)||(0<s3))
					{
						/* 1 non-zero axis: Get any two other orthogonal axes */
						if (0.0<s1)
						{
							get_orthogonal_axes(ax1,ax2,ax3,&bx1,&bx2,&bx3,&cx1,&cx2,&cx3);
						}
						else if (0.0<s2)
						{
							get_orthogonal_axes(bx1,bx2,bx3,&cx1,&cx2,&cx3,&ax1,&ax2,&ax3);
						}
						else /* if (0.0<s2) */
						{
							get_orthogonal_axes(cx1,cx2,cx3,&ax1,&ax2,&ax3,&bx1,&bx2,&bx3);
						}
					}
					else
					{
						/* All axes non-zero: Use default axes */
						ax1 = 1.0;
						bx2 = 1.0;
						cx3 = 1.0;
					}
					if (skewed_axes)
					{
						/* refuse to output the glyph if axes are skew */
						number_of_skew_glyph_axes++;
					}
					else
					{
						/* find angles - a_angle and b_angle - and axes - a and b - so
							that rotating by a_angle about a followed rotating by b_angle
							about b takes the x-axis to ax, the y-axis to bx and the z-axis
							to cx */
						/* a is perpendicular to the projection of ax onto the y-z plane
							(i) and a_angle is the angle between ax and the x axis.  a_angle
							must be between 0 and pi because the angle is in the plane
							defined by the x axis and i and ax must be in the non-negative
							i half of this plane */
						if (0.0 < (a_magnitude = sqrt(ax2*ax2 + ax3*ax3)))
						{
							a1 = 0.0;
							a2 = -ax3 / a_magnitude;
							a3 = ax2 / a_magnitude;
							a_angle = clamped_acos(ax1);
						}
						else
						{
							a1 = 0.0;
							a2 = 1.0;
							a3 = 0.0;
							if (0 > ax1)
							{
								a_angle = (ZnReal)PI;
							}
							else
							{
								a_angle = 0.0;
							}
						}
						/* rotating by a_angle about a takes the x-axis onto ax and the
							y-axis onto j */
						j1 = -sin(a_angle) * a3;
						j2 = (1.0 - cos(a_angle)) * a2 * a2 + cos(a_angle);
						j3 = (1.0 - cos(a_angle)) * a2 * a3;
						/* bx is perpendicular to ax, because they are orthonormal, and
							j is perpendiculat to ax because they are the images of the y-
							and x- axes under rotation.  So bx and j are in the plane
							perpendicular to ax */
						/* now rotate about ax (b) to get j onto bx */
						b1 = ax1;
						b2 = ax2;
						b3 = ax3;
						/* because j and bx are unit length cos(b_angle)=j dot b */
						b_angle = clamped_acos(j1*bx1 + j2*bx2 + j3*bx3);
						/* acos gives an angle between 0 and pi.  If j cross bx is in the
							same direction as b, then the angle is clockwise, if it is in
							the opposite direction then it is anti-clockwise.   */
						/*???DB.  Lose accuracy fairly badly here.  Need to have
							tolerance so that there are not different numbers of lines for
							ndiff */
#define ZERO_ROTATION_TOLERANCE 0.001
/*							if (0.0 != b_angle)*/
						if (ZERO_ROTATION_TOLERANCE < fabs(b_angle))
						{
							/* get c = j1 (x) bx */
							c1 = j2*bx3 - j3*bx2;
							c2 = j3*bx1 - j1*bx3;
							c3 = j1*bx2 - j2*bx1;
							/* the magnitude of c is the absolute value of sin(b_angle)
								because j and bx are unit length.  For small theta,
								sin(theta) is approximately theta */
							c_magnitude = sqrt(c1*c1 + c2*c2 + c3*c3);
							if (ZERO_ROTATION_TOLERANCE < fabs(c_magnitude))
							{
								dp = b1*c1 + b2*c2 + b3*c3;
								if (dp < 0.0)
								{
									/* make clockwise */
									b_angle = -b_angle;
								}
							}
							else
							{
								/* b_angle is close to +/- PI (have already checked that
									b_angle isn't near 0 */
								b_angle = (ZnReal)PI;
							}
						}
						fprintf(vrml_file,"Transform {\n");
						fprintf(vrml_file,"  translation %f %f %f\n", x,y,z);
						/* if possible, try to avoid having two Transform nodes */
/*							if ((0.0 != a_angle)&&(0.0 != b_angle))*/
						if ((ZERO_ROTATION_TOLERANCE < fabs(a_angle))&&
							(ZERO_ROTATION_TOLERANCE < fabs(b_angle)))
						{
							/* b-rotation must wrap a-rotation to occur after it */
							fprintf(vrml_file,"  rotation %f %f %f %f\n",b1,b2,b3,b_angle);
							fprintf(vrml_file,"  children [\n");
							fprintf(vrml_file,"    Transform {\n");
						}
/*							if (0.0 != a_angle)*/
						if (ZERO_ROTATION_TOLERANCE < fabs(a_angle))
						{
							fprintf(vrml_file,"    rotation %f %f %f %f\n",a1,a2,a3,
								a_angle);
						}
/*							else if (0.0 != b_angle)*/
						else if (ZERO_ROTATION_TOLERANCE < fabs(b_angle))
						{
							fprintf(vrml_file,"    rotation %f %f %f %f\n",b1,b2,b3,
								b_angle);
						}
						fprintf(vrml_file,"    scale   %f %f %f\n", s1,s2,s3);
						fprintf(vrml_file,"    children [\n");

						/* set the spectrum for this datum, if any */
						if (material_copy)
						{
							MANAGER_COPY_WITHOUT_IDENTIFIER(Graphical_material,name)
								(material_copy, material);
							Spectrum_render_value_on_material(spectrum,material_copy,
								number_of_data_components,glyph_set->data+i*number_of_data_components);
							/*???RC temporary until we have a struct Glyph - actual glyph to use
								is at GT_object_get_next_object(glyph) when glyph is in mirror mode */
							/* note no DEF/USE when coloured by a spectrum as will always be
									different every time it is rendered */
							write_graphics_object_vrml(vrml_file, glyph, time,
								(struct LIST(VRML_prototype) *)NULL, /*object_is_glyph*/1,
								material_copy, /*gt_object_already_defined*/0);
						}
						else
						{
							write_graphics_object_vrml(vrml_file, glyph, time,
								vrml_prototype_list, /*object_is_glyph*/1,
								material, /*gt_object_already_defined*/0<i);
						}
						fprintf(vrml_file,"    ]\n");
/*							if ((0.0 != a_angle)&&(0.0 != b_angle))*/
						if ((ZERO_ROTATION_TOLERANCE < fabs(a_angle))&&
							(ZERO_ROTATION_TOLERANCE < fabs(b_angle)))
						{
							fprintf(vrml_file,"    } #Transform\n");
							fprintf(vrml_file,"  ]\n");
						}
						fprintf(vrml_file,"} #Transform\n");
					}
				}

				point++;
				axis1++;
				axis2++;
				axis3++;
				scale++;
			}
			if (0 < number_of_skew_glyph_axes)
			{
				display_message(WARNING_MESSAGE, "draw_glyph_set_vrml.  "
					"%d glyph(s) not rendered because they have skewed axes",
					number_of_skew_glyph_axes);
			}
			if (material_copy)
			{
				cmzn_graphics_material_destroy(&material_copy);
			}
		}
		/* output label at each point, if supplied */
		char **static_labels = 0;
		for (int labelNumber = 0; labelNumber < 3; ++labelNumber)
		{
			if (glyph_set->static_label_text[labelNumber])
			{
				static_labels = glyph_set->static_label_text;
				break;
			}
		}
		label = glyph_set->labels;
		if (label || static_labels)
		{
			point=glyph_set->point_list;
			if (data_spectrum)
			{
				material_copy=cmzn_graphics_material_create_private();
				cmzn_graphics_material_set_name(material_copy, "render_vrml_copy");
			}
			else
			{
				material_copy = (struct Graphical_material *)NULL;
			}
			const int number_of_glyphs =
				cmzn_glyph_repeat_mode_get_number_of_glyphs(glyph_repeat_mode);
			for (i = 0; i < number_of_points; i++)
			{
				for (int glyph_number = 0; glyph_number < number_of_glyphs; glyph_number++)
				{
					if (cmzn_glyph_repeat_mode_glyph_number_has_label(glyph_repeat_mode, glyph_number) &&
						((label && (glyph_number == 0) && (*label)) || (static_labels && static_labels[glyph_number])))
					{
						resolve_glyph_axes(glyph_repeat_mode, glyph_number,
							glyph_set->base_size, glyph_set->scale_factors, glyph_set->offset,
							*point, *axis1, *axis2, *axis3, *scale,
							temp_point, temp_axis1, temp_axis2, temp_axis3);
						if ((glyph_repeat_mode == CMZN_GLYPH_REPEAT_MIRROR) && ((*scale)[0] < 0.0f))
						{
							for (int j = 0; j < 3; ++j)
							{
								temp_point[j] += (
									(1.0 - glyph_set->label_offset[0])*temp_axis1[j] +
									glyph_set->label_offset[1]*temp_axis2[j] +
									glyph_set->label_offset[2]*temp_axis3[j]);
							}
						}
						else
						{
							for (int j = 0; j < 3; ++j)
							{
								temp_point[j] += (
									glyph_set->label_offset[0]*temp_axis1[j] +
									glyph_set->label_offset[1]*temp_axis2[j] +
									glyph_set->label_offset[2]*temp_axis3[j]);
							}
						}
						x = temp_point[0];
						y = temp_point[1];
						z = temp_point[2];
						fprintf(vrml_file,"Transform {\n");
						fprintf(vrml_file,"  translation %f %f %f\n", x, y, z);
						fprintf(vrml_file,"  children [\n");
						fprintf(vrml_file,"    Billboard {\n");
						fprintf(vrml_file,"      axisOfRotation 0 0 0\n");
						fprintf(vrml_file,"      children [\n");
						fprintf(vrml_file,"Shape {\n");
						fprintf(vrml_file,"  appearance\n");
						if (material)
						{
							fprintf(vrml_file,"Appearance {\n");
							fprintf(vrml_file,"  material\n");
							if (material_copy)
							{
								MANAGER_COPY_WITHOUT_IDENTIFIER(Graphical_material,name)
									(material_copy, material);
								Spectrum_render_value_on_material(spectrum,material_copy,
									number_of_data_components,glyph_set->data+i*number_of_data_components);
								activate_material_vrml(vrml_file,material_copy,
									(struct LIST(VRML_prototype) *)NULL,
									/*no_define*/1,/*emissive_only*/0);
								/*???RC text is drawn as a textured surface, hence */
							}
							else
							{
								activate_material_vrml(vrml_file,material,
									(struct LIST(VRML_prototype) *)NULL,
									/*no_define*/1,/*emissive_only*/0);
							}
							fprintf(vrml_file,"} #Appearance\n");
						}
						else
						{
							fprintf(vrml_file,"IS line_appearance\n");
						}
						fprintf(vrml_file,"  geometry Text {\n");
						fprintf(vrml_file,"    string [\n");
						char *text = 0;
						int error = 0;
						if (static_labels && static_labels[glyph_number])
						{
							append_string(&text, static_labels[glyph_number], &error);
						}
						if (label && (*label) && (glyph_number == 0))
						{
							append_string(&text, *label, &error);
						}
						if (text)
						{
							make_valid_token(&text);
							if (('\"' == text[0]) || ('\'' == text[0]))
							{
								fprintf(vrml_file,"      %s\n", text);
							}
							else
							{
								fprintf(vrml_file,"      \"%s\"\n", text);
							}
							DEALLOCATE(text);
						}
						fprintf(vrml_file,"    ]\n");
						fprintf(vrml_file,"  } #Text\n");
						fprintf(vrml_file,"} #Shape\n");
						fprintf(vrml_file,"      ]\n");
						fprintf(vrml_file,"    } #Billboard\n");
						fprintf(vrml_file,"  ]\n");
						fprintf(vrml_file,"} #Transform\n");
					}
				}
				point++;
				axis1++;
				axis2++;
				axis3++;
				scale++;
				label++;
			}
			if (material_copy)
			{
				cmzn_graphics_material_destroy(&material_copy);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"draw_glyph_set_vrml. Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

static int draw_point_set_vrml(FILE *vrml_file,int n_pts,Triple *point_list,
	char **text,gtMarkerType marker_type,ZnReal marker_size,int number_of_data_components,
	GLfloat *data,struct Graphical_material *material,struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 10 May 1999

DESCRIPTION :
Writes VRML code to the file handle which represents the given pointset.
==============================================================================*/
{
	char **text_string;
	ZnReal delta, x, y, z;
	int i, offset, return_code;
	Triple *point;
	struct Graphical_material *material_copy;

	ENTER(draw_point_set_vrml);
	if (vrml_file&&point_list&&(1<n_pts)&&
		((g_NO_DATA==number_of_data_components)||(data&&material&&spectrum)))
	{
		switch(marker_type)
		{
			case g_NO_MARKER:
			{
				/* Do nothing here */
			} break;
			case g_POINT_MARKER:
			{
				fprintf(vrml_file,"Shape {\n");
				fprintf(vrml_file,"  appearance\n");
				if (material)
				{
					fprintf(vrml_file,"Appearance {\n");
					fprintf(vrml_file,"  material\n");
					activate_material_vrml(vrml_file,material,
						(struct LIST(VRML_prototype) *)NULL,
						/*no_define_material*/0,/*emissive_only*/1);
					fprintf(vrml_file,"} #Appearance\n");
				}
				else
				{
					fprintf(vrml_file,"IS line_appearance\n");
				}
				fprintf(vrml_file,"  geometry PointSet {\n");
				fprintf(vrml_file,"    coord Coordinate {\n");
				fprintf(vrml_file,"      point [\n");
				point=point_list;
				for (i=0;i<n_pts;i++)
				{
					fprintf(vrml_file,"        %f %f %f,\n",
						(*point)[0],(*point)[1],(*point)[2]);
					point++;
				}
				fprintf(vrml_file,"      ]\n");
				fprintf(vrml_file,"    }\n");
				if (number_of_data_components && data && spectrum)
				{
					spectrum_start_render_vrml(vrml_file,spectrum,material);
					for (i=0;i<n_pts;i++)
					{
						spectrum_render_vrml_value(vrml_file,spectrum,material,
							number_of_data_components,data+i*number_of_data_components);
					}
					spectrum_end_render_vrml(vrml_file, spectrum);
				}
				fprintf(vrml_file,"  } #PointSet\n");
				fprintf(vrml_file,"} #Shape\n");
			} break;
			case g_PLUS_MARKER:
			{
				fprintf(vrml_file,"Shape {\n");
				fprintf(vrml_file,"  appearance\n");
				if (material)
				{
					fprintf(vrml_file,"Appearance {\n");
					fprintf(vrml_file,"  material\n");
					activate_material_vrml(vrml_file,material,
						(struct LIST(VRML_prototype) *)NULL,
						/*no_define_material*/0,/*emissive_only*/1);
					fprintf(vrml_file,"} #Appearance\n");
				}
				else
				{
					fprintf(vrml_file,"IS line_appearance\n");
				}
				fprintf(vrml_file,"  geometry IndexedLineSet {\n");
				fprintf(vrml_file,"    coord Coordinate {\n");
				fprintf(vrml_file,"      point [\n");
				point=point_list;
				for (i=0;i<n_pts;i++)
				{
					x = (*point)[0];
					y = (*point)[1];
					z = (*point)[2];
					delta = marker_size / 2.0;
					fprintf(vrml_file,"        %f %f %f,\n",x - delta, y, z);
					fprintf(vrml_file,"        %f %f %f,\n",x + delta, y, z);
					fprintf(vrml_file,"        %f %f %f,\n",x, y - delta, z);
					fprintf(vrml_file,"        %f %f %f,\n",x, y + delta, z);
					fprintf(vrml_file,"        %f %f %f,\n",x, y, z - delta);
					fprintf(vrml_file,"        %f %f %f,\n",x, y, z + delta);
					point++;
				}
				fprintf(vrml_file,"      ]\n");
				fprintf(vrml_file,"    }\n");
				fprintf(vrml_file,"    coordIndex   [\n");
				for (i=0;i<n_pts;i++)
				{
					fprintf(vrml_file,"      %d,%d,-1\n",6*i,6*i+1);
					fprintf(vrml_file,"      %d,%d,-1\n",6*i+2,6*i+3);
					fprintf(vrml_file,"      %d,%d,-1\n",6*i+4,6*i+5);
				}
				fprintf(vrml_file,"    ]\n");
				if (number_of_data_components && data && spectrum)
				{
					fprintf(vrml_file,"    colorPerVertex FALSE\n");
					spectrum_start_render_vrml(vrml_file,spectrum,material);
					for (i=0;i<n_pts;i++)
					{
						spectrum_render_vrml_value(vrml_file,spectrum,material,
							number_of_data_components,data+i*number_of_data_components);
						spectrum_render_vrml_value(vrml_file,spectrum,material,
							number_of_data_components,data+i*number_of_data_components);
						spectrum_render_vrml_value(vrml_file,spectrum,material,
							number_of_data_components,data+i*number_of_data_components);
					}
					spectrum_end_render_vrml(vrml_file, spectrum);
				}
				fprintf(vrml_file,"  } #IndexedLineSet\n");
				fprintf(vrml_file,"} #Shape\n");
			} break;
			case g_DERIVATIVE_MARKER:
			{
				fprintf(vrml_file,"Shape {\n");
				fprintf(vrml_file,"  appearance\n");
				if (material)
				{
					fprintf(vrml_file,"Appearance {\n");
					fprintf(vrml_file,"  material\n");
					activate_material_vrml(vrml_file,material,
						(struct LIST(VRML_prototype) *)NULL,
						/*no_define_material*/0,/*emissive_only*/1);
					fprintf(vrml_file,"} #Appearance\n");
				}
				else
				{
					fprintf(vrml_file,"IS line_appearance\n");
				}
				fprintf(vrml_file,"  geometry IndexedLineSet {\n");
				fprintf(vrml_file,"    coord Coordinate {\n");
				fprintf(vrml_file,"      point [\n");
				point=point_list;
				for (i=0;i<n_pts;i++)
				{
					fprintf(vrml_file,"        %f %f %f,\n",
						(*point)[0],(*point)[1],(*point)[2]);
					point++;
					fprintf(vrml_file,"        %f %f %f,\n",
						(*point)[0],(*point)[1],(*point)[2]);
					point++;
					fprintf(vrml_file,"        %f %f %f,\n",
						(*point)[0],(*point)[1],(*point)[2]);
					point++;
					fprintf(vrml_file,"        %f %f %f,\n",
						(*point)[0],(*point)[1],(*point)[2]);
					point++;
				}
				fprintf(vrml_file,"      ]\n");
				fprintf(vrml_file,"    }\n");
				fprintf(vrml_file,"    coordIndex [\n");
				for (i=0;i<n_pts;i++)
				{
					fprintf(vrml_file,"      %d,%d,-1,\n",4*i,4*i+1);
					fprintf(vrml_file,"      %d,%d,-1,\n",4*i,4*i+2);
					fprintf(vrml_file,"      %d,%d,-1,\n",4*i,4*i+3);
				}
				fprintf(vrml_file,"  ]\n");
				if (number_of_data_components && data && spectrum)
				{
					fprintf(vrml_file,"    colorPerVertex FALSE\n");
					spectrum_start_render_vrml(vrml_file,spectrum,material);
					for (i=0;i<n_pts;i++)
					{
						spectrum_render_vrml_value(vrml_file,spectrum,material,
							number_of_data_components,data+i*number_of_data_components);
					}
					spectrum_end_render_vrml(vrml_file, spectrum);
				}
				fprintf(vrml_file,"  } #IndexedLineSet\n");
				fprintf(vrml_file,"} #Shape\n");
			} break;
		}
		if (material&&Graphical_material_get_texture(material))
		{
			display_message(INFORMATION_MESSAGE,"draw_point_set_vrml.  "
				"VRML does not allow a texture to be applied to a pointset");
		}
		text_string=text;
		if (text_string)
		{
			if (g_DERIVATIVE_MARKER==marker_type)
			{
				offset=4;
			}
			else
			{
				offset=1;
			}
			point=point_list;
			if (number_of_data_components && data && material && spectrum)
			{
				material_copy=cmzn_graphics_material_create_private();
				cmzn_graphics_material_set_name(material_copy, "render_vrml_copy");
			}
			else
			{
				material_copy = (struct Graphical_material *)NULL;
			}
			for (i=0;i<n_pts;i++)
			{
				x = (*point)[0];
				y = (*point)[1];
				z = (*point)[2];
				fprintf(vrml_file,"Transform {\n");
				fprintf(vrml_file,"  translation %f %f %f\n", x, y, z);
				fprintf(vrml_file,"  children [\n");
				fprintf(vrml_file,"    Billboard {\n");
				fprintf(vrml_file,"      axisOfRotation 0 0 0\n");
				fprintf(vrml_file,"      children [\n");
				fprintf(vrml_file,"Shape {\n");
				fprintf(vrml_file,"  appearance\n");
				if (material)
				{
					fprintf(vrml_file,"Appearance {\n");
					fprintf(vrml_file,"  material\n");
					if (material_copy)
					{
						MANAGER_COPY_WITHOUT_IDENTIFIER(Graphical_material,name)
							(material_copy, material);
						Spectrum_render_value_on_material(spectrum,material_copy,
							number_of_data_components,data+i*number_of_data_components);
						activate_material_vrml(vrml_file,material_copy,
							(struct LIST(VRML_prototype) *)NULL,
							/*no_define*/1,/*emissive_only*/0);
						/*???RC text is drawn as a textured surface, hence */
					}
					else
					{
						activate_material_vrml(vrml_file,material,
							(struct LIST(VRML_prototype) *)NULL,
							/*no_define*/1,/*emissive_only*/0);
					}
					fprintf(vrml_file,"}\n");
				}
				else
				{
					fprintf(vrml_file,"IS line_appearance\n");
				}
				fprintf(vrml_file,"  geometry Text {\n");
				fprintf(vrml_file,"    string [\n");
				fprintf(vrml_file,"      \"%s\"\n",*text_string);
				fprintf(vrml_file,"    ]\n");
				fprintf(vrml_file,"  } #Text\n");
				fprintf(vrml_file,"} #Shape\n");
				fprintf(vrml_file,"      ]\n");
				fprintf(vrml_file,"    } #Billboard\n");
				fprintf(vrml_file,"  ]\n");
				fprintf(vrml_file,"} #Transform\n");
				point += offset;
				text_string++;
			}
			if (material_copy)
			{
				cmzn_graphics_material_destroy(&material_copy);
			}
		}
		return_code = 1;
	}
	else
	{
		if (1<n_pts)
		{
			display_message(ERROR_MESSAGE,
				"draw_point_set_vrml.  Invalid argument(s)");
			return_code=0;
		}
		else
		{
			return_code=1;
		}
	}
	LEAVE;

	return (return_code);
} /* draw_point_set_vrml */

static int draw_polyline_vrml(FILE *vrml_file,GLfloat *point_list,
	int number_of_data_components,GLfloat *data,
	struct Graphical_material *material,struct Spectrum *spectrum,int n_pts,
	enum GT_polyline_type polyline_type)
/*******************************************************************************
LAST MODIFIED : 9 May 1999

DESCRIPTION :
Writes VRML code to the file handle which represents the given
continuous polyline. If data or spectrum are NULL they are ignored.
==============================================================================*/
{
	int i,number_of_points,return_code;

	ENTER(draw_polyline_vrml);
	const bool data_spectrum = (0 < number_of_data_components) && (0 != data) &&
		(0 != spectrum) && (0 != material);
	if (vrml_file&&point_list&&(1<n_pts))
	{
		return_code=1;
		switch (polyline_type)
		{
			case g_PLAIN:
			{
				number_of_points=n_pts;
			} break;
			case g_PLAIN_DISCONTINUOUS:
			{
				/* n_pts = number of line segments in this case */
				number_of_points=2*n_pts;
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"draw_polyline_vrml.  Unsupported polyline_type");
				return_code=0;
			}
		}
		if (return_code)
		{
			fprintf(vrml_file,"Shape {\n");
			fprintf(vrml_file,"  appearance\n");
			if (material)
			{
				fprintf(vrml_file,"Appearance {\n");
				fprintf(vrml_file,"  material\n");
				activate_material_vrml(vrml_file,material,
					(struct LIST(VRML_prototype) *)NULL,
					/*no_define_material*/0,/*emissive_only*/1);
				fprintf(vrml_file,"} #Appearance\n");
			}
			else
			{
				fprintf(vrml_file,"IS line_appearance\n");
			}
			fprintf(vrml_file,"  geometry IndexedLineSet {\n");
			fprintf(vrml_file,"    coord Coordinate {\n");
			fprintf(vrml_file,"      point [\n");
			for (i=0;i<number_of_points;i++)
			{
				fprintf(vrml_file,"        %f %f %f,\n",point_list[i*3+0],
					point_list[i*3+1],point_list[i*3+2]);
			}
			fprintf(vrml_file,"      ]\n");
			fprintf(vrml_file,"    }\n");
			if (data_spectrum)
			{
				fprintf(vrml_file,"    colorPerVertex TRUE\n");
				spectrum_start_render_vrml(vrml_file,spectrum,material);
				for (i=0;i<number_of_points;i++)
				{
					spectrum_render_vrml_value(vrml_file,spectrum,material,
						number_of_data_components,data+i*number_of_data_components);
				}
				spectrum_end_render_vrml(vrml_file, spectrum);
			}
			if (material&&Graphical_material_get_texture(material))
			{
				display_message(INFORMATION_MESSAGE,"draw_polyline_vrml.  "
					"VRML does not allow a texture to be applied to a line");
			}
			fprintf(vrml_file,"    coordIndex [\n");
			switch (polyline_type)
			{
				case g_PLAIN:
				{
					fprintf(vrml_file,"      ");
					for (i=0;i<n_pts;i++)
					{
						fprintf(vrml_file,"%d,",i);
					}
					fprintf(vrml_file,"-1\n");
				} break;
				case g_PLAIN_DISCONTINUOUS:
				{
					for (i=0;i<n_pts;i++)
					{
						fprintf(vrml_file,"      %d,%d,-1\n",2*i,2*i+1);
					}
				} break;
				default:
				{
				} break;
			}
			fprintf(vrml_file,"    ]\n");
			fprintf(vrml_file,"  } #IndexedLineSet\n");
			fprintf(vrml_file,"} #Shape\n");
		}
	}
	else
	{
		if (1<n_pts)
		{
			display_message(ERROR_MESSAGE,"draw_polyline_vrml.  Invalid argument(s)");
			return_code=0;
		}
		else
		{
			return_code=1;
		}
	}
	LEAVE;

	return (return_code);
} /* draw_polyline_vrml */

static int draw_surface_vrml(FILE *vrml_file,Triple *surfpts, Triple *normalpts,
	Triple *texturepts, int number_of_data_components, GLfloat *data,
	struct Graphical_material *material,struct Spectrum *spectrum,int npts1,
	int npts2,enum GT_surface_type surface_type,enum cmzn_graphics_render_polygon_mode render_polygon_mode,
	gtPolygonType polygon_type,
	struct LIST(VRML_prototype) *vrml_prototype_list)
/*******************************************************************************
LAST MODIFIED : 9 July 1999

DESCRIPTION :
==============================================================================*/
{
	int i,j,return_code;
	int index = 0,index_1,index_2,number_of_points;
	struct Texture *texture;
	Triple *triple;

	ENTER(draw_surface_vrml);
	return_code=0;
	const bool data_spectrum = (0 < number_of_data_components) && (0 != data) &&
		(0 != spectrum) && (0 != material);
	if (surfpts&&(0<npts1)&&(1<npts2))
	{
		switch (surface_type)
		{
			case g_SHADED:
			case g_SHADED_TEXMAP:
			{
				switch (polygon_type)
				{
					case g_QUADRILATERAL:
					{
						number_of_points=npts1*npts2;
						return_code=1;
					} break;
					case g_TRIANGLE:
					{
						number_of_points=(npts1*(npts1+1))/2;
						return_code=1;
					} break;
					default:
					{
					} break;
				}
			} break;
			case g_SH_DISCONTINUOUS:
			case g_SH_DISCONTINUOUS_TEXMAP:
			{
				switch (polygon_type)
				{
					case g_GENERAL_POLYGON:
					case g_QUADRILATERAL:
					case g_TRIANGLE:
					{
						number_of_points=npts1*npts2;
						return_code=1;
					} break;
				}
			} break;
			default:
			{
			} break;
		}
		if (return_code)
		{
			fprintf(vrml_file,"Shape {\n");
			fprintf(vrml_file,"  appearance\n");
			fprintf(vrml_file,"Appearance {\n");
			fprintf(vrml_file,"  material\n");
			activate_material_vrml(vrml_file,material,
				vrml_prototype_list,
				/*no_define_material*/0,/*emissive_only*/0);
			texture = Graphical_material_get_texture(material);
			if (texture)
			{
				write_texture_vrml(vrml_file, texture);
			}
			fprintf(vrml_file,"}\n");
			if (CMZN_GRAPHICS_RENDER_POLYGON_WIREFRAME == render_polygon_mode)
			{
				fprintf(vrml_file,"  geometry IndexedLineSet {\n");
			}
			else
			{
				fprintf(vrml_file,"  geometry IndexedFaceSet {\n");
				fprintf(vrml_file,"    solid FALSE\n");
			}
			fprintf(vrml_file,"    coord Coordinate {\n");
			fprintf(vrml_file,"      point [\n");
			triple=surfpts;
			for (i=number_of_points;i>0;i--)
			{
				fprintf(vrml_file,"        %f %f %f,\n",(*triple)[0],(*triple)[1],
					(*triple)[2]);
				triple++;
			}
			fprintf(vrml_file,"      ]\n");
			fprintf(vrml_file,"    }\n");
			if (CMZN_GRAPHICS_RENDER_POLYGON_SHADED == render_polygon_mode)
			{
				triple=normalpts;
				if (triple)
				{
					fprintf(vrml_file,"    normal Normal {\n");
					fprintf(vrml_file,"      vector [\n");
					for (i=number_of_points;i>0;i--)
					{
						fprintf(vrml_file,"        %f %f %f,\n",(*triple)[0],(*triple)[1],
							(*triple)[2]);
						triple++;
					}
					fprintf(vrml_file,"      ]\n");
					fprintf(vrml_file,"    }\n");
				}
			}
			if (data_spectrum)
			{
				spectrum_start_render_vrml(vrml_file,spectrum,material);
				for (i=0;i<number_of_points;i++)
				{
					spectrum_render_vrml_value(vrml_file,spectrum,material,
						number_of_data_components,data+number_of_data_components*i);
				}
				spectrum_end_render_vrml(vrml_file, spectrum);
			}
			/* texture coordinates */
			if (CMZN_GRAPHICS_RENDER_POLYGON_SHADED == render_polygon_mode)
			{
				triple=texturepts;
				if (triple)
				{
					fprintf(vrml_file,"    texCoord TextureCoordinate {\n");
					fprintf(vrml_file,"      point [\n");
					for (i=number_of_points;i>0;i--)
					{
						fprintf(vrml_file,"        %f %f,\n",(*triple)[0],(*triple)[1]);
						triple++;
					}
					fprintf(vrml_file,"      ]\n");
					fprintf(vrml_file,"    }\n");
				}
			}
			/* polygon definitions */
			fprintf(vrml_file,"    coordIndex [\n");
			switch (surface_type)
			{
				case g_SHADED:
				case g_SHADED_TEXMAP:
				{
					switch (polygon_type)
					{
						case g_QUADRILATERAL:
						{
							index=0;
							if (CMZN_GRAPHICS_RENDER_POLYGON_WIREFRAME == render_polygon_mode)
							{
								for (j=0;j<npts2-1;j++)
								{
									for (i=0;i<npts1-1;i++)
									{
										/* -1 finishes of the polygon vertex index list */
										fprintf(vrml_file,"      %d,%d,%d,%d,%d,-1\n",index,
											index+1,index+npts1+1,index+npts1,index);
										index++;
									}
									index++;
								}
							}
							else
							{
								for (j=0;j<npts2-1;j++)
								{
									for (i=0;i<npts1-1;i++)
									{
										/* -1 finishes of the polygon vertex index list */
										fprintf(vrml_file,"      %d,%d,%d,%d,-1\n",index,
											index+1,index+npts1+1,index+npts1);
										index++;
									}
									index++;
								}
							}
						} break;
						case g_TRIANGLE:
						{
							/* triangle strip */
							index_1=0;
							index_2=index_1+npts1;
							if (CMZN_GRAPHICS_RENDER_POLYGON_WIREFRAME == render_polygon_mode)
							{
								for (i=npts1-1;i>0;i--)
								{
									for (j=i-1;j>0;j--)
									{
										/* -1 finishes of the polygon vertex index list */
										fprintf(vrml_file,"      %d,%d,%d,%d,-1\n",
											index_1, index_1 + 1, index_2, index_1);
										index_1++;
										fprintf(vrml_file,"      %d,%d,%d,%d,-1\n",
											index_1, index_2 + 1, index_2, index_1);
										index_2++;
									}
									/* -1 finishes of the polygon vertex index list */
									fprintf(vrml_file,"      %d,%d,%d,%d,-1\n",
										index_1, index_1 + 1, index_2, index_1);
									index_1 += 2;
									index_2++;
								}
							}
							else
							{
								for (i=npts1-1;i>0;i--)
								{
									for (j=i-1;j>0;j--)
									{
										/* -1 finishes of the polygon vertex index list */
										fprintf(vrml_file,"      %d,%d,%d,-1\n",
											index_1, index_1 + 1, index_2);
										index_1++;
										fprintf(vrml_file,"      %d,%d,%d,-1\n",
											index_1, index_2 + 1, index_2);
										index_2++;
									}
									/* -1 finishes of the polygon vertex index list */
									fprintf(vrml_file,"      %d,%d,%d,-1\n",
										index_1, index_1 + 1, index_2);
									index_1 += 2;
									index_2++;
								}
							}
						} break;
						default:
						{
						} break;
					}
				} break;
				case g_SH_DISCONTINUOUS:
				case g_SH_DISCONTINUOUS_TEXMAP:
				{
					if (CMZN_GRAPHICS_RENDER_POLYGON_WIREFRAME == render_polygon_mode)
					{
						/* npts1 = number of polygons */
						for (i=0;i<npts1;i++)
						{
							fprintf(vrml_file,"      ");
							/* npts2 = number of vertices per polygon */
							for (j=npts2;j>0;j--)
							{
								fprintf(vrml_file,"%d,",index);
								index++;
							}
							/* -1 finishes of the polygon vertex index list */
							fprintf(vrml_file,"%d,-1\n",i*npts2);
						}
					}
					else
					{
						/* npts1 = number of polygons */
						index = 0;
						for (i=0;i<npts1;i++)
						{
							fprintf(vrml_file,"      ");
							/* npts2 = number of vertices per polygon */
							for (j=npts2;j>0;j--)
							{
								fprintf(vrml_file,"%d,",index);
								index++;
							}
							/* -1 finishes of the polygon vertex index list */
							fprintf(vrml_file,"-1\n");
						}
					}
				} break;
				default:
				{
				} break;
			}
			fprintf(vrml_file,"    ]\n");
			if (CMZN_GRAPHICS_RENDER_POLYGON_WIREFRAME == render_polygon_mode)
			{
				fprintf(vrml_file,"  } #IndexedLineSet\n");
			}
			else
			{
				fprintf(vrml_file,"  } #IndexedFaceSet\n");
			}
			fprintf(vrml_file,"} #Shape\n");
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"draw_surface_vrml.  Unsupported surface_type");
			return_code=0;
		}
	}
	else
	{
		if ((0<npts1)&&(1<npts2))
		{
			display_message(ERROR_MESSAGE,
				"draw_surface_vrml.  Invalid argument(s)");
			return_code=0;
		}
		else
		{
			return_code=1;
		}
	}
	LEAVE;

	return (return_code);
} /* draw_surface_vrml */

static int draw_voltex_vrml(FILE *vrml_file,
	int number_of_vertices, struct VT_iso_vertex **vertex_list,
	int number_of_triangles, struct VT_iso_triangle **triangle_list,
	int number_of_data_components,
	struct Graphical_material *default_material,
	struct Spectrum *spectrum,
	struct LIST(VRML_prototype) *vrml_prototype_list)
/*******************************************************************************
LAST MODIFIED : 8 August 2002

DESCRIPTION :
==============================================================================*/
{
	int i, return_code;

	ENTER(draw_voltex_vrml);
	return_code=0;
	const bool data_spectrum = (0 < number_of_data_components) &&
		(0 != spectrum) && (0 != default_material);
	if (triangle_list && vertex_list && (0<number_of_triangles))
	{
		/* more efficient case: no separate materials per polygon */
		fprintf(vrml_file, "Shape {\n");
		fprintf(vrml_file, "  appearance\n");
		if (default_material)
		{
			fprintf(vrml_file, "Appearance {\n");
			fprintf(vrml_file, "  material\n");
			activate_material_vrml(vrml_file, default_material,
				vrml_prototype_list, /*no_define*/0, /*emissive_only*/0);
			fprintf(vrml_file, "} #Appearance\n");
		}
		else
		{
			fprintf(vrml_file, "IS surface_appearance\n");
		}
		fprintf(vrml_file,"  geometry IndexedFaceSet {\n");
		fprintf(vrml_file,"    solid FALSE\n");
		fprintf(vrml_file,"    coord Coordinate {\n");
		fprintf(vrml_file,"      point [\n");
		for (i = 0; i < number_of_vertices; i++)
		{
			fprintf(vrml_file,"        %f %f %f,\n",
				vertex_list[i]->coordinates[0],
				vertex_list[i]->coordinates[1],
				vertex_list[i]->coordinates[2]);
		}
		fprintf(vrml_file,"      ]\n");
		fprintf(vrml_file,"    }\n");
		if (data_spectrum)
		{
			spectrum_start_render_vrml(vrml_file, spectrum, default_material);
			GLfloat *data = new GLfloat[number_of_data_components];
			for (i = 0; i < number_of_vertices; i++)
			{
				CAST_TO_OTHER(data, vertex_list[i]->data, GLfloat, number_of_data_components);
				spectrum_render_vrml_value(vrml_file, spectrum, default_material,
					number_of_data_components, data);
			}
			delete[] data;
			spectrum_end_render_vrml(vrml_file, spectrum);
		}
		fprintf(vrml_file,"    coordIndex [\n");
		for (i = 0; i < number_of_triangles; i++)
		{
			fprintf(vrml_file,"      %d,%d,%d,-1\n",
				triangle_list[i]->vertices[0]->index,
				triangle_list[i]->vertices[2]->index,
				triangle_list[i]->vertices[1]->index);
		}
		fprintf(vrml_file,"    ]\n");
		fprintf(vrml_file,"  } #IndexedFaceSet\n");
		fprintf(vrml_file,"} #Shape\n");
	}
	return_code = 1;
	LEAVE;

	return (return_code);
} /* draw_voltex_vrml */

int makevrml(FILE *vrml_file,gtObject *object,ZnReal time,
	struct LIST(VRML_prototype) *vrml_prototype_list)
/*******************************************************************************
LAST MODIFIED : 20 March 2003

DESCRIPTION :
Convert graphical object into API object.
Only writes the geometry field.
???RC Crap
==============================================================================*/
{
	ZnReal proportion = 0.0, *times;
	int group = 0, itime, number_of_times, return_code;
	/* struct GT_nurbs *nurbs; */
	struct GT_glyph_set *interpolate_glyph_set,*glyph_set,*glyph_set_2;
	struct GT_point *point;
	struct GT_pointset *interpolate_point_set,*point_set,*point_set_2;
	struct GT_polyline *interpolate_line,*line,*line_2;
	struct GT_surface *interpolate_surface,*surface,*surface_2;
	/* struct GT_userdef *userdef; */
	struct GT_voltex *voltex;
	union GT_primitive_list *primitive_list1 = NULL, *primitive_list2 = NULL;

	ENTER(makevrml);
	return_code=1;
	if (vrml_file&&object)
	{
		number_of_times = object->number_of_times;
		if (0 < number_of_times)
		{
			itime = number_of_times;
			group = 0;
			if ((itime>1)&&(times=object->times))
			{
				itime--;
				times += itime;
				if (time>= *times)
				{
					proportion=0;
				}
				else
				{
					while ((itime>0)&&(time< *times))
					{
						itime--;
						times--;
					}
					if (time< *times)
					{
						proportion=0;
					}
					else
					{
						proportion=times[1]-times[0];
						if (proportion>0)
						{
							proportion=time-times[0]/proportion;
						}
						else
						{
							proportion=0;
						}
					}
				}
			}
			else
			{
				itime=0;
				proportion=0;
			}
			if (object->primitive_lists &&
				(primitive_list1 = object->primitive_lists + itime))
			{
				if (proportion > 0)
				{
					if (!(primitive_list2 = object->primitive_lists + itime + 1))
					{
						display_message(ERROR_MESSAGE,
							"makevrml.  Invalid primitive_list");
						return_code = 0;
					}
				}
				else
				{
					primitive_list2 = (union GT_primitive_list *)NULL;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"makevrml.  Invalid primitive_lists");
				return_code = 0;
			}
		}
		if ((0 < number_of_times) && return_code)
		{
			switch (GT_object_get_type(object))
			{
				case g_GLYPH_SET:
				{
					glyph_set = primitive_list1->gt_glyph_set.first;
					if (glyph_set)
					{
						if (glyph_set->ptrnext)
						{
							fprintf(vrml_file,"Group {\n");
							fprintf(vrml_file,"  children [\n");
							group = 1;
						}
						if (proportion>0)
						{
							glyph_set_2 = primitive_list2->gt_glyph_set.first;
							while (glyph_set&&glyph_set_2)
							{
								interpolate_glyph_set=morph_GT_glyph_set(proportion,glyph_set,glyph_set_2);
								if (interpolate_glyph_set)
								{
									draw_glyph_set_vrml(vrml_file, interpolate_glyph_set,
										object->default_material,object->spectrum,
										time,vrml_prototype_list);
									DESTROY(GT_glyph_set)(&interpolate_glyph_set);
								}
								glyph_set = glyph_set->ptrnext;
								glyph_set_2 = glyph_set_2->ptrnext;
							}
						}
						else
						{
							while (glyph_set)
							{
								draw_glyph_set_vrml(vrml_file, glyph_set,
									object->default_material, object->spectrum,
									time, vrml_prototype_list);
								glyph_set = glyph_set->ptrnext;
							}
						}
						if (group)
						{
							fprintf(vrml_file,"  ]\n");
							fprintf(vrml_file,"} #Group\n");
						}
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,"makevrml.  Missing glyph_set");
						return_code=0;
					}
				} break;
				case g_NURBS:
				{
				} break;
				case g_POINT:
				{
					point = primitive_list1->gt_point.first;
					if (point)
					{
						draw_point_set_vrml(vrml_file,
							1, point->position, &(point->text), point->marker_type,
							point->marker_size,point->n_data_components,point->data,
							object->default_material,object->spectrum);
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,"makevrml.  Missing point");
						return_code=0;
					}
				} break;
				case g_POINTSET:
				{
					point_set = primitive_list1->gt_pointset.first;
					if (point_set)
					{
						if (point_set->ptrnext)
						{
							fprintf(vrml_file,"Group {\n");
							fprintf(vrml_file,"  children [\n");
							group = 1;
						}
						if (proportion>0)
						{
							point_set_2 = primitive_list2->gt_pointset.first;
							while (point_set&&point_set_2)
							{
								interpolate_point_set=morph_GT_pointset(proportion,point_set, point_set_2);
								if (interpolate_point_set)
								{
									draw_point_set_vrml(vrml_file,
										interpolate_point_set->n_pts,
										interpolate_point_set->pointlist,
										interpolate_point_set->text,
										interpolate_point_set->marker_type,
										interpolate_point_set->marker_size,
										interpolate_point_set->n_data_components,
										interpolate_point_set->data,
										object->default_material,object->spectrum);
									DESTROY(GT_pointset)(&interpolate_point_set);
								}
								point_set=point_set->ptrnext;
								point_set_2=point_set_2->ptrnext;
							}
						}
						else
						{
							draw_point_set_vrml(vrml_file,
								point_set->n_pts,point_set->pointlist,
								point_set->text,point_set->marker_type,point_set->marker_size,
								point_set->n_data_components,point_set->data,
								object->default_material,object->spectrum);
						}
						if (group)
						{
							fprintf(vrml_file,"  ]\n");
							fprintf(vrml_file,"} #Group\n");
						}
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,"makevrml.  Missing point");
						return_code=0;
					}
				} break;
				case g_POLYLINE:
				{
					line = primitive_list1->gt_polyline.first;
					if (line)
					{
						if (line->ptrnext)
						{
							fprintf(vrml_file,"Group {\n");
							fprintf(vrml_file,"  children [\n");
							group = 1;
						}
						if (0<proportion)
						{
							line_2 = primitive_list2->gt_polyline.first;
							while (line&&line_2)
							{
								interpolate_line=morph_GT_polyline(proportion,line,line_2);
								if (interpolate_line)
								{
									draw_polyline_vrml(vrml_file,*interpolate_line->pointlist,
										interpolate_line->n_data_components,interpolate_line->data,
										object->default_material,object->spectrum,
										interpolate_line->n_pts,interpolate_line->polyline_type);
									DESTROY(GT_polyline)(&interpolate_line);
								}
								line=line->ptrnext;
								line_2=line_2->ptrnext;
							}
						}
						else
						{
							while (line)
							{
								draw_polyline_vrml(vrml_file,*line->pointlist,
									line->n_data_components,line->data,
									object->default_material,object->spectrum,
									line->n_pts,line->polyline_type);
								line=line->ptrnext;
							}
						}
						if (group)
						{
							fprintf(vrml_file,"  ]\n");
							fprintf(vrml_file,"} #Group\n");
						}
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,"makevrml.  Missing polyline");
						return_code=0;
					}
				} break;
				case g_POLYLINE_VERTEX_BUFFERS:
				{
					GT_polyline_vertex_buffers *line;
					line = primitive_list1->gt_polyline_vertex_buffers;
					if (line)
					{
						unsigned int line_index;
						unsigned int line_count =
							object->vertex_array->get_number_of_vertices(
							GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_ELEMENT_INDEX_START);

						GLfloat *position_buffer, *data_buffer, *normal_buffer;
						unsigned int position_values_per_vertex, position_vertex_count,
						data_values_per_vertex, data_vertex_count, normal_values_per_vertex,
						normal_vertex_count;

						if (line_count > 1)
						{
							fprintf(vrml_file,"Group {\n");
							fprintf(vrml_file,"  children [\n");
							group = 1;
						}

						position_buffer = 0;
						object->vertex_array->get_float_vertex_buffer(
							GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_POSITION,
							&position_buffer, &position_values_per_vertex, &position_vertex_count);

						data_buffer = 0;
						object->vertex_array->get_float_vertex_buffer(
							GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_DATA,
							&data_buffer, &data_values_per_vertex, &data_vertex_count);

						normal_buffer = 0;
						object->vertex_array->get_float_vertex_buffer(
							GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_NORMAL,
							&normal_buffer, &normal_values_per_vertex, &normal_vertex_count);

						for (line_index = 0; line_index < line_count; line_index++)
						{
							int object_name;
							object->vertex_array->get_integer_attribute(
								GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_ID,
								line_index, 1, &object_name);

							unsigned int index_start, index_count;
							GLfloat *position_vertex, *data_vertex;

							object->vertex_array->get_unsigned_integer_attribute(
								GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_ELEMENT_INDEX_START,
								line_index, 1, &index_start);
							object->vertex_array->get_unsigned_integer_attribute(
								GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_ELEMENT_INDEX_COUNT,
								line_index, 1, &index_count);

							position_vertex = position_buffer +
								position_values_per_vertex * index_start;
							if (data_buffer)
							{
								data_vertex = data_buffer +
									data_values_per_vertex * index_start;
							}
							else
							{
								data_values_per_vertex = 0;
								data_vertex = NULL;
							}
							draw_polyline_vrml(vrml_file, position_vertex,
								data_values_per_vertex, data_vertex,
								object->default_material,object->spectrum,
								index_count, line->polyline_type);
						}
						if (group)
						{
							fprintf(vrml_file,"  ]\n");
							fprintf(vrml_file,"} #Group\n");
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"makevrml.  Missing line");
						return_code=0;
					}
				} break;
				case g_SURFACE:
				{
					surface = primitive_list1->gt_surface.first;
					if (surface)
					{
						if (surface->ptrnext)
						{
							fprintf(vrml_file,"Group {\n");
							fprintf(vrml_file,"  children [\n");
							group = 1;
						}
						if (0<proportion)
						{
							surface_2 = primitive_list2->gt_surface.first;
							while (surface&&surface_2)
							{
								interpolate_surface=morph_GT_surface(proportion,surface,surface_2);
								if (interpolate_surface)
								{
									draw_surface_vrml(vrml_file,interpolate_surface->pointlist,
										interpolate_surface->normallist,
										interpolate_surface->texturelist,
										interpolate_surface->n_data_components,
										interpolate_surface->data,object->default_material,
										object->spectrum,interpolate_surface->n_pts1,
										interpolate_surface->n_pts2,surface->surface_type,
										surface->render_polygon_mode,surface->polygon,vrml_prototype_list);
									DESTROY(GT_surface)(&interpolate_surface);
								}
								surface=surface->ptrnext;
								surface_2=surface_2->ptrnext;
							}
						}
						else
						{
							while (surface)
							{
								draw_surface_vrml(vrml_file,surface->pointlist,
									surface->normallist,surface->texturelist,
									surface->n_data_components,surface->data,
									object->default_material,object->spectrum,surface->n_pts1,
									surface->n_pts2,surface->surface_type,surface->render_polygon_mode,
									surface->polygon,vrml_prototype_list);
								surface=surface->ptrnext;
							}
						}
						if (group)
						{
							fprintf(vrml_file,"  ]\n");
							fprintf(vrml_file,"} #Group\n");
						}
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,"makevrml.  Missing surface");
						return_code=0;
					}
				} break;
				case g_USERDEF:
				{
				} break;
				case g_VOLTEX:
				{
					voltex = primitive_list1->gt_voltex.first;
					if (voltex)
					{
						if (voltex->ptrnext)
						{
							fprintf(vrml_file,"Group {\n");
							fprintf(vrml_file,"  children [\n");
							group = 1;
						}
						while (voltex)
						{
							draw_voltex_vrml(vrml_file,
								voltex->number_of_vertices, voltex->vertex_list,
								voltex->number_of_triangles, voltex->triangle_list,
								voltex->n_data_components,
								object->default_material,object->spectrum,
								vrml_prototype_list);
							voltex=voltex->ptrnext;
						}
						if (group)
						{
							fprintf(vrml_file,"  ]\n");
							fprintf(vrml_file,"} #Group\n");
						}
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,"makevrml.  Missing voltex");
						return_code=0;
					}
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,"makevrml.  Invalid object type");
					return_code=0;
				} break;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"makevrml.  Missing object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* makevrml */

static int write_graphics_object_vrml(FILE *vrml_file,
	struct GT_object *gt_object,ZnReal time,
	struct LIST(VRML_prototype) *vrml_prototype_list,
	int object_is_glyph,struct Graphical_material *default_material,
	int gt_object_already_prototyped)
/*******************************************************************************
LAST MODIFIED : 21 June 2001

DESCRIPTION :
==============================================================================*/
{
	char *dot_pointer, *prototype_name = NULL, *material_name;
	int group, in_def, return_code;
	struct GT_object *temp_gt_object;
	struct VRML_prototype *vrml_prototype;
  /* DPN 21 June 2001 - Using fgetpos() like this is wrong! */
	long file_pointer,save_file_pointer = 0L;

	ENTER(write_graphics_object_vrml);
	if (vrml_file&&gt_object&&gt_object->name&&default_material)
	{
		group = 0;
		in_def = 0;
		return_code=1;

		const char *parsed_name = NULL;
		if (object_is_glyph)
			parsed_name = duplicate_string(gt_object->name);
		else
			parsed_name = duplicate_string(gt_object->name+1);
		char *temp_string = NULL;
		const char *num_string = "";
		if ((temp_string = (char *)strrchr(parsed_name, '/')))
		{
			num_string = temp_string+1;
			temp_string[0] = '\0';
		}
		while ((temp_string = (char *)strchr(parsed_name, '/')))
		{
			temp_string[0] = '_';
		}
		if (GET_NAME(Graphical_material)(default_material, &material_name)&&
			ALLOCATE(prototype_name,char,strlen(parsed_name)+strlen(num_string)+10+strlen(material_name)))
		{
			if (object_is_glyph)
			{
				sprintf(prototype_name,"glyph_%s%s_%s",parsed_name, num_string, material_name);
			}
			else
			{
				sprintf(prototype_name,"object_%s%s_%s",parsed_name, num_string, material_name);
			}
			/* Can't have certain characters (.: ) in a name */
			while ((dot_pointer = strchr(prototype_name, '.'))
				|| (dot_pointer = strchr(prototype_name, ' '))
				|| (dot_pointer = strchr(prototype_name, ':')))
			{
				*dot_pointer = '_';
			}
			temp_gt_object=gt_object;
			if (vrml_prototype_list)
			{
				vrml_prototype=CREATE(VRML_prototype)(prototype_name,(struct Texture *)NULL,
					default_material, gt_object,/*time*/0);
				if (gt_object_already_prototyped ||
					FIRST_OBJECT_IN_LIST_THAT(VRML_prototype)(VRML_prototype_is_identical,
					(void *)vrml_prototype,vrml_prototype_list))
				{
					fprintf(vrml_file,"USE %s\n", prototype_name);
					/* Don't write the objects */
					temp_gt_object=(struct GT_object *)NULL;
					DESTROY(VRML_prototype)(&vrml_prototype);
				}
				else
				{
					fprintf(vrml_file,"DEF %s\n", prototype_name);
					/* save file pointer so we can see if anything was output */
					save_file_pointer = ftell(vrml_file);
					in_def = 1;
					ADD_OBJECT_TO_LIST(VRML_prototype)(vrml_prototype,vrml_prototype_list);
				}
			}
			if (temp_gt_object)
			{
				if (GT_object_get_next_object(temp_gt_object))
				{
					fprintf(vrml_file,"Group {\n");
					fprintf(vrml_file,"  children [\n");
					group = 1;
				}
				while ((NULL != temp_gt_object)&&return_code)
				{
					if (temp_gt_object->default_material)
					{
						return_code=
							makevrml(vrml_file,temp_gt_object,time,vrml_prototype_list);
					}
					else
					{
						/* Temporarily set the correct material in objects that don't have one */
						temp_gt_object->default_material = default_material;
						return_code=
							makevrml(vrml_file,temp_gt_object,time,vrml_prototype_list);
						temp_gt_object->default_material = (struct Graphical_material *)NULL;
					}
					temp_gt_object=GT_object_get_next_object(temp_gt_object);
				}
				if (group)
				{
					fprintf(vrml_file,"  ]\n");
					fprintf(vrml_file,"} #Group\n");
				}
				if (in_def)
				{
					/* check if anything was output; if not, add a dummy node */
					file_pointer = ftell(vrml_file);
					if (file_pointer == save_file_pointer)
					{
						fprintf(vrml_file,"Group {\n");
						fprintf(vrml_file,"# Dummy group node for empty object\n");
						fprintf(vrml_file,"} #Group\n");
					}
					fprintf(vrml_file, "#END DEF %s\n", prototype_name);
				}
			}
			DEALLOCATE(prototype_name);
			DEALLOCATE(material_name);
		}
		DEALLOCATE(parsed_name);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_graphics_object_vrml.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* write_graphics_object_vrml */

struct Export_to_vrml_data
{
	FILE *vrml_file;
	cmzn_scene_id scene;
	cmzn_scenefilter_id filter;
	/* store materials, glyphs and gt_objects that have been DEFined already so
		 we can USE them again */
	struct LIST(VRML_prototype) *vrml_prototype_list;
}; /* struct Export_to_vrml_data */

static int graphics_object_export_to_vrml(struct GT_object *gt_object,
	double time,void *export_to_vrml_data_void)
/*******************************************************************************
LAST MODIFIED : 9 May 1999

DESCRIPTION :
Write the graphics object to the <file>.
???RC <time> is a double because that is expected for a
graphics_object_tree_iterator_function
==============================================================================*/
{
	FILE *vrml_file;
	int return_code = 0;
	struct Export_to_vrml_data *export_to_vrml_data;

	ENTER(graphics_object_export_to_vrml);
	if (gt_object&&(export_to_vrml_data=
		(struct Export_to_vrml_data *)export_to_vrml_data_void))
	{
		vrml_file=export_to_vrml_data->vrml_file;
		switch(GT_object_get_type(gt_object))
		{
			case g_POINT:
			case g_POINTSET:
			case g_POLYLINE:
			case g_POLYLINE_VERTEX_BUFFERS:
			case g_GLYPH_SET:
			case g_VOLTEX:
			case g_SURFACE:
			{
				return_code=write_graphics_object_vrml(vrml_file,gt_object,time,
					export_to_vrml_data->vrml_prototype_list,
					/*object_is_glyph*/0,gt_object->default_material,
					/*gt_object_already_defined*/0);
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,"graphics_object_export_to_vrml.  "
					"The graphics object %s is of a type not yet supported",
					gt_object->name);
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"graphics_object_export_to_vrml.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* graphics_object_export_to_vrml */

/*
Global functions
----------------
*/
int export_to_vrml(char *file_name, cmzn_scene_id scene,
	cmzn_scenefilter_id filter)
/******************************************************************************
LAST MODIFIED : 19 October 2001

DESCRIPTION :
Renders the visible objects to a VRML file.
==============================================================================*/
{
	double centre_x, centre_y, centre_z, radius, size_x, size_y, size_z;
	FILE *vrml_file;
	int return_code;
	struct Export_to_vrml_data export_to_vrml_data;

	if (file_name&&scene)
	{
		build_Scene(scene, filter);
		/* open file and add header */
		vrml_file=fopen(file_name,"w");
		if (vrml_file)
		{
			/* 1. Write the VRML header */
			/*???RC Add copyright message, source/how to contact UniServices??
				Treat this as Advertising... */
			fprintf(vrml_file,"#VRML V2.0 utf8\n# CMGUI VRML Generator\n");
			export_to_vrml_data.vrml_file=vrml_file;
			export_to_vrml_data.scene=scene;
			export_to_vrml_data.filter=filter;
			export_to_vrml_data.vrml_prototype_list=NULL;
			/* 2. Write scene graph */
			/* transform.... */
			/* draw objects */
			fprintf(vrml_file,"Group {\n");
			fprintf(vrml_file,"  children [\n");

			if (cmzn_scene_get_global_graphics_range(scene, filter,
				&centre_x, &centre_y, &centre_z, &size_x, &size_y, &size_z) &&
				(radius = sqrt(size_x*size_x + size_y*size_y + size_z*size_z)))
			{
				fprintf(vrml_file,"    Viewpoint {\n");
				fprintf(vrml_file,"      description \"default\"\n");
				fprintf(vrml_file,"      position %f %f %f\n",
					centre_x, centre_y, centre_z + radius);
				fprintf(vrml_file,"    } #Viewpoint\n");
			}
			fprintf(vrml_file,"    NavigationInfo {\n");
			fprintf(vrml_file,"      type [\"EXAMINE\",\"ANY\"]\n");
			fprintf(vrml_file,"    } #NavigationInfo\n");
			export_to_vrml_data.vrml_prototype_list=
				CREATE(LIST(VRML_prototype))();
			return_code=for_each_graphics_object_in_scene_tree(scene, filter,
				graphics_object_export_to_vrml,(void *)&export_to_vrml_data);
			DESTROY(LIST(VRML_prototype))(&export_to_vrml_data.vrml_prototype_list);
			fprintf(vrml_file,"  ]\n");
			fprintf(vrml_file,"} #Group\n");
			/* set lights... */
			fclose(vrml_file);
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"export_to_vrml.  Could not open vrml file");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"export_to_vrml.  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} /* export_to_vrml */
