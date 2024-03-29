/*******************************************************************************
FILE : render_wavefront.cpp

LAST MODIFIED : 20 March 2003

DESCRIPTION :
Renders gtObjects to Wavefront OBJ file
==============================================================================*/
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#include <math.h>
#include <sstream>

#include "general/debug.h"
#include "general/indexed_list_private.h"
#include "general/mystring.h"
#include "graphics/graphics_object.h"
#include "graphics/material.hpp"
#include "graphics/render_wavefront.hpp"
#include "graphics/scene.hpp"
#include "graphics/spectrum.h"
#include "general/message.h"
#include "graphics/graphics_object_private.hpp"


/*
Module types
------------
*/

struct Wavefront_vertex_position
/*******************************************************************************
LAST MODIFIED : 8 July 1999

DESCRIPTION :
==============================================================================*/
{
	ZnReal x;
	ZnReal y;
	ZnReal z;
}; /* struct Wavefront_vertex_position */

struct Wavefront_vertex
/*******************************************************************************
LAST MODIFIED : 8 July 1999

DESCRIPTION :
==============================================================================*/
{
	struct Wavefront_vertex_position *position;
	int index;

	int access_count;
}; /* struct Wavefront_vertex */

/*
Module variables
----------------
*/
static int file_vertex_index, file_normal_vertex_index, file_texture_vertex_index;

/*
Module functions
----------------
*/
int compare_vertex_location(struct Wavefront_vertex_position *position_1,
	struct Wavefront_vertex_position *position_2)
/*******************************************************************************
LAST MODIFIED : 7 July 1999

DESCRIPTION :
Returns -1 if location_1 < location_2, 0 if location_1 = location_2 within a
small tolerance and 1 if location_1 > location_2.  X is considered first, then
Y and the Z.
==============================================================================*/
{
	const ZnReal tolerance = 1e-3f;
	int return_code;

	ENTER(compare_vertex_location);
	if (position_1->x < (position_2->x - tolerance))
	{
		return_code= -1;
	}
	else
	{
		if (position_1->x > (position_2->x + tolerance))
		{
			return_code=1;
		}
		else
		{
			if (position_1->y < (position_2->y - tolerance))
			{
				return_code= -1;
			}
			else
			{
				if (position_1->y > (position_2->y + tolerance))
				{
					return_code=1;
				}
				else
				{
					if (position_1->z < (position_2->z - tolerance))
					{
						return_code= -1;
					}
					else
					{
						if (position_1->z > (position_2->z + tolerance))
						{
							return_code=1;
						}
						else
						{
							return_code=0;
						}
					}
				}
			}
		}
	}
	LEAVE;

	return (return_code);
} /* compare_position */

struct Wavefront_vertex *CREATE(Wavefront_vertex)
	(int index, ZnReal x, ZnReal y, ZnReal z)
/*******************************************************************************
LAST MODIFIED : 7 July 1999

DESCRIPTION :
==============================================================================*/
{
	struct Wavefront_vertex *vertex;
	struct Wavefront_vertex_position *vertex_position;

	ENTER(CREATE(Wavefront_vertex));

	if (ALLOCATE(vertex,struct Wavefront_vertex,1)&&
		 (ALLOCATE(vertex_position,struct Wavefront_vertex_position,1)))
	{
		vertex->position = vertex_position;
		vertex_position->x = x;
		vertex_position->y = y;
		vertex_position->z = z;
		vertex->index = index;
		vertex->access_count = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Wavefront_vertex).  Not enough memory");
		vertex = (struct Wavefront_vertex *)NULL;
	}
	LEAVE;

	return (vertex);
} /* CREATE(Wavefront_vertex) */

int DESTROY(Wavefront_vertex)(struct Wavefront_vertex **vertex_address)
/*******************************************************************************
LAST MODIFIED : 7 July 1999

DESCRIPTION :
Frees memory/deaccess mapping at <*vertex_address>.
==============================================================================*/
{
	int return_code;

	ENTER(DESTROY(Wavefront_vertex));
	if (vertex_address&&*vertex_address)
	{
		if (0 >= (*vertex_address)->access_count)
		{
			DEALLOCATE((*vertex_address)->position);
			DEALLOCATE(*vertex_address);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"DESTROY(Wavefront_vertex).  Positive access_count");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Wavefront_vertex).  Missing mapping");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Wavefront_vertex) */

DECLARE_OBJECT_FUNCTIONS(Wavefront_vertex)
DECLARE_LIST_TYPES(Wavefront_vertex);
FULL_DECLARE_INDEXED_LIST_TYPE(Wavefront_vertex);
DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Wavefront_vertex,
	position,struct Wavefront_vertex_position *,compare_vertex_location)
DECLARE_INDEXED_LIST_FUNCTIONS(Wavefront_vertex)
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(
	Wavefront_vertex,
	position,struct Wavefront_vertex_position *,compare_vertex_location)

static int activate_material_wavefront(std::ostringstream& wavefront_output,
	cmzn_material *material,
	cmzn_material **current_material)
/*******************************************************************************
LAST MODIFIED : 2 October 2000

DESCRIPTION :
Writes Wavefront object file that defines the material
==============================================================================*/
{
	int return_code;

	ENTER(activate_material_wavefront);
    if (material)
	{
		if (current_material)
		{
			if (*current_material == material)
			{
				return_code = 1;
			}
			else
			{
                wavefront_output << "usemtl " << material->getName() << std::endl;
				*current_material = material;
				return_code=1;
			}
		}
		else
		{
            wavefront_output << "usemtl " << material->getName() << std::endl;
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
            "activate_material_wavefront.  Missing material");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* activate_material_wavefront */

static int makewavefront(std::ostringstream &wavefront_output, int full_comments,
	gtObject *object, ZnReal time);

int draw_glyph_set_wavefront(std::ostringstream& wavefront_output,
	GT_glyphset_vertex_buffers *glyph_set,	Graphics_vertex_array *vertex_array,
	cmzn_material *material)
/*******************************************************************************
LAST MODIFIED : 11 May 1999

DESCRIPTION :
Defines an object for the <glyph> and then draws that at <number_of_points>
points  given by the positions in <point_list> and oriented and scaled by
<axis1_list>, <axis2_list> and <axis3_list>.
==============================================================================*/
{
	int return_code = 0;
	struct GT_object *transformed_object = 0;

	if (glyph_set && vertex_array)
	{
		unsigned int position_values_per_vertex = 0, position_vertex_count = 0;
		GLfloat *position_buffer = 0;
		vertex_array->get_float_vertex_buffer(
			GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_POSITION,
			&position_buffer, &position_values_per_vertex, &position_vertex_count);

		/* try to draw points and lines faster */
		if (0==strcmp(glyph_set->glyph->name,"point"))
		{
			GLfloat *position = position_buffer;
			for (unsigned int i = 0; i < position_vertex_count; i++)
			{
                wavefront_output << "v " << position[0] << " " << position[1] << " " << position[2] << std::endl;
				position += position_values_per_vertex;
			}
		}
		else if (0==strcmp(glyph_set->glyph->name,"line"))
		{
			display_message(WARNING_MESSAGE,"draw_glyph_set_wavefront.  "
				"pointset glyphs not currently rendered in wavefront files (use a surface glyph).");
		}
		else
		{
			unsigned int axis1_values_per_vertex = 0, axis1_vertex_count = 0,
				axis2_values_per_vertex = 0, axis2_vertex_count = 0,
				axis3_values_per_vertex = 0, axis3_vertex_count = 0;
			GLfloat *axis1_buffer = 0, *axis2_buffer = 0, *axis3_buffer = 0;
			vertex_array->get_float_vertex_buffer(
				GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_AXIS1,
				&axis1_buffer, &axis1_values_per_vertex, &axis1_vertex_count);
			vertex_array->get_float_vertex_buffer(
				GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_AXIS2,
				&axis2_buffer, &axis2_values_per_vertex, &axis2_vertex_count);
			vertex_array->get_float_vertex_buffer(
				GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_AXIS3,
				&axis3_buffer, &axis3_values_per_vertex, &axis3_vertex_count);
			for (unsigned int i=0;i<position_vertex_count;i++)
			{
				// following method was removed, so can't output glyphs at present
				// GRC it would probably be better to just pass the transformation
				// to makewavefront and transform coordinates there
				transformed_object = 0; // transform_GT_object(glyph_set->glyph, transformation);
				display_message(WARNING_MESSAGE, "draw_glyph_set_wavefront.  Can't transform glyphs");
				break;
				if (transformed_object != 0)
				{
					set_GT_object_default_material(transformed_object,
						material);
                    makewavefront(wavefront_output, 1, transformed_object, 0);
					DEACCESS(GT_object)(&transformed_object);
				}
				position_buffer += position_values_per_vertex;
				axis1_buffer += axis1_values_per_vertex;
				axis2_buffer += axis2_values_per_vertex;
				axis3_buffer += axis3_values_per_vertex;
			}
		}
		return_code=1;
	}
	else
	{
			display_message(ERROR_MESSAGE,"draw_glyph_set_wavefront. Invalid argument(s)");
			return_code=0;
	}

	return (return_code);
} /* draw_glyph_set_wavefront */

int draw_surface_wavefront(std::ostringstream& wavefront_output,
	GT_surface_vertex_buffers *surface,
	Graphics_vertex_array *vertex_array,
	cmzn_material *material,
	cmzn_material **current_material)
/*******************************************************************************
LAST MODIFIED : 3 October 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code = 0;
	unsigned int i, j;

	if (surface&&vertex_array)
	{
		unsigned int surface_index;
		unsigned int surface_count =
			vertex_array->get_number_of_vertices(
			GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_ELEMENT_INDEX_START);
		GLfloat *position_buffer = 0,  *texture_coordinate0_buffer = 0;
		unsigned int position_values_per_vertex, position_vertex_count,
			texture_coordinate0_values_per_vertex,	texture_coordinate0_vertex_count;
		if (0 == surface_count)
			return 1;

		vertex_array->get_float_vertex_buffer(
			GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_POSITION,
			&position_buffer, &position_values_per_vertex, &position_vertex_count);
		vertex_array->get_float_vertex_buffer(
			GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_TEXTURE_COORDINATE_ZERO,
			&texture_coordinate0_buffer, &texture_coordinate0_values_per_vertex,
			&texture_coordinate0_vertex_count);
		unsigned int *index_vertex_buffer = 0, index_values_per_vertex = 0, index_vertex_count = 0;
		vertex_array->get_unsigned_integer_vertex_buffer(
			GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_STRIP_INDEX_ARRAY,
			&index_vertex_buffer, &index_values_per_vertex,
			&index_vertex_count);
        activate_material_wavefront(wavefront_output, material, current_material);
		for (i = 0; i< position_vertex_count; i++)
		{
            wavefront_output << "v " << position_buffer[0] << " " << position_buffer[1] << " " << position_buffer[2] << std::endl;
			position_buffer += position_values_per_vertex;
		}
		if (texture_coordinate0_buffer)
		{
			for (i = 0; i< texture_coordinate0_vertex_count; i++)
			{
                wavefront_output << "vt " << texture_coordinate0_buffer[0] << " " << texture_coordinate0_buffer[1] << " " << texture_coordinate0_buffer[2] << std::endl;
				texture_coordinate0_buffer += texture_coordinate0_values_per_vertex;
			}
		}
		for (surface_index = 0; surface_index < surface_count; surface_index++)
		{
			int object_name = 0;
			vertex_array->get_integer_attribute(
				GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_OBJECT_ID,
				surface_index, 1, &object_name);
			if (object_name > -1)
			{
				if (index_vertex_buffer)
				{
					unsigned int number_of_strips = 0;
					unsigned int strip_start = 0;
					vertex_array->get_unsigned_integer_attribute(
						GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_NUMBER_OF_STRIPS,
						surface_index, 1, &number_of_strips);
					vertex_array->get_unsigned_integer_attribute(
						GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_STRIP_START,
						surface_index, 1, &strip_start);
					for (i = 0; i < number_of_strips; i++)
					{
						unsigned int points_per_strip = 0;
						unsigned int index_start_for_strip = 0;
						vertex_array->get_unsigned_integer_attribute(GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_STRIP_INDEX_START,
							strip_start+i, 1, &index_start_for_strip);
						vertex_array->get_unsigned_integer_attribute(GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_NUMBER_OF_POINTS_FOR_STRIP,
							strip_start+i, 1, &points_per_strip);
						unsigned int *indices = &index_vertex_buffer[index_start_for_strip];
						for (j =0; j<points_per_strip - 2; j++)
						{
							if (0 == (j % 2))
							{
								if (texture_coordinate0_buffer)
								{
                                    wavefront_output << "f " << indices[j]+1 << "/" << indices[j]+1 << " " << indices[j+1]+1 << "/" << indices[j+1]+1 << " " << indices[j+2]+1 << "/" << indices[j+2]+1 << std::endl;
								}
								else
								{
                                    wavefront_output << "f " << indices[j]+1 << " " << indices[j+1]+1 << " " << indices[j+2]+1 << std::endl;
								}
							}
							else
							{
								if (texture_coordinate0_buffer)
								{
                                    wavefront_output << "f " << indices[j+1]+1 << "/" << indices[j+1]+1 << " " << indices[j]+1 << "/" << indices[j]+1 << " " << indices[j+2]+1 << "/" << indices[j+2]+1 << std::endl;
								}
								else
								{
                                    wavefront_output << "f " << indices[j+1]+1 << " " << indices[j]+1 << " " << indices[j+2]+1 << std::endl;
								}
							}
						}
					}
				}
				else
				{
					unsigned int index_start, index_count;
					vertex_array->get_unsigned_integer_attribute(
						GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_ELEMENT_INDEX_START,
						surface_index, 1, &index_start);
					vertex_array->get_unsigned_integer_attribute(
						GRAPHICS_VERTEX_ARRAY_ATTRIBUTE_TYPE_ELEMENT_INDEX_COUNT,
						surface_index, 1, &index_count);
					unsigned int number_of_triangles = index_count / 3;
					int current_index = index_start + 1;
					for (i = 0; i < number_of_triangles; i++)
					{
						if (texture_coordinate0_buffer)
						{
                            wavefront_output << "f " << current_index << "/" << current_index << " " << current_index+1 << "/" << current_index+1 << " " << current_index+2 << "/" << current_index+2 << std::endl;
						}
						else
						{
                            wavefront_output << "f " << current_index << " " << current_index+1 << " " << current_index+2 << std::endl;
						}
						current_index += 3;
					}
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"draw_surface_wavefront.  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} /* draw_surface_wavefront */

static int makewavefront(std::ostringstream& wavefront_output, int full_comments,
	gtObject *object, ZnReal time)
/*******************************************************************************
LAST MODIFIED : 20 March 2003

DESCRIPTION :
Convert graphical object into Wavefront object file.
==============================================================================*/
{
	int number_of_times, return_code;
	cmzn_material *current_material;

	/* check arguments */
	return_code = 1;
	current_material = (cmzn_material *)NULL;
	if (object)
	{
		number_of_times = object->number_of_times;
		if (0 < number_of_times)
		{
			if (!object->primitive_lists)
			{
				display_message(ERROR_MESSAGE,
					"makewavefront.  Invalid primitive_lists");
				return_code = 0;
			}
		}
		if ((0 < number_of_times) && return_code)
		{
			switch (object->object_type)
			{
				case g_GLYPH_SET_VERTEX_BUFFERS:
				{
					struct GT_glyphset_vertex_buffers *gt_glyphset_vertex_buffers =
						object->primitive_lists->gt_glyphset_vertex_buffers;
					if (gt_glyphset_vertex_buffers != 0)
					{
                            draw_glyph_set_wavefront(wavefront_output,
								gt_glyphset_vertex_buffers,
								object->vertex_array,
								object->default_material);
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,"makewavefront.  Missing glyph_set");
						return_code=0;
					}
				} break;
				case g_SURFACE_VERTEX_BUFFERS:
				{
					struct GT_surface_vertex_buffers *gt_surface_vertex_buffers =
						object->primitive_lists->gt_surface_vertex_buffers;
					if (gt_surface_vertex_buffers != 0)
					{
						switch (gt_surface_vertex_buffers->surface_type)
						{
							case g_SHADED:
							case g_SHADED_TEXMAP:
							case g_SH_DISCONTINUOUS:
							case g_SH_DISCONTINUOUS_TEXMAP:
							{
                                draw_surface_wavefront(wavefront_output,
									gt_surface_vertex_buffers,
									object->vertex_array,
									object->default_material,
									&current_material);
								return_code=1;
							} break;
							default:
							{
							} break;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"makewavefront.  Missing surface");
						return_code=0;
					}
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,"makewavefront.  Invalid object type");
					return_code=0;
				} break;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"makewavefront.  Missing object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* makewavefront */

struct Export_to_wavefront_data
{
    std::vector<std::string> object_names;
    std::vector<std::string> objects;
	int full_comments;
}; /* struct Export_to_wavefront_data */

static int graphics_object_export_to_wavefront(
	struct GT_object *gt_object, double time, void *export_to_wavefront_data_void)
/*******************************************************************************
LAST MODIFIED : 19 June 1998

DESCRIPTION :
Write the window object to the <wavefront_file_void>.
==============================================================================*/
{
	int return_code = 0;
	struct Export_to_wavefront_data *export_to_wavefront_data;

	ENTER(graphics_object_export_to_wavefront);
	/* check arguments */
	if (gt_object && (export_to_wavefront_data=
		(struct Export_to_wavefront_data *)export_to_wavefront_data_void))
	{
		switch(gt_object->object_type)
		{
			case g_GLYPH_SET_VERTEX_BUFFERS:
			case g_SURFACE_VERTEX_BUFFERS:
			{
                std::string parsed_name(gt_object->name);
                std::replace(parsed_name.begin(), parsed_name.end(), '/', '_');
                parsed_name += ".obj";

                export_to_wavefront_data->object_names.push_back(parsed_name);

                std::ostringstream ss;
                ss << "# CMGUI Wavefront Object file generator" << std::endl;
                ss << parsed_name << std::endl;
                ss << "mtllib global.mtl" << std::endl << std::endl;
                file_vertex_index = 0;
                file_normal_vertex_index = 0;
                file_texture_vertex_index = 0;
                return_code = makewavefront(ss,
                    export_to_wavefront_data->full_comments,
                    gt_object, time);

                export_to_wavefront_data->objects.push_back(ss.str());
                return_code = 1;
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,"graphics_object_export_to_wavefront.  "
					"The graphics object %s is of a type not yet supported", gt_object->name);
				return_code=0;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"graphics_object_export_to_wavefront.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* graphics_object_export_to_wavefront */

/*
Global functions
----------------
*/

std::vector<std::string> export_to_wavefront(cmzn_scene_id scene,
	cmzn_scenefilter_id filter, int full_comments)
/******************************************************************************
LAST MODIFIED : 31 May 2000

DESCRIPTION :
Renders the visible objects to Wavefront object files.
==============================================================================*/
{
    std::vector<std::string> content;
	struct Export_to_wavefront_data export_to_wavefront_data;

	ENTER(export_to_wavefront);
	if (scene)
	{
		build_Scene(scene, filter);
		/* Write all the graphics objects in the scene */
        /*???debug */
        display_message(WARNING_MESSAGE,
            "export_to_wavefront.  Not fully implemented");
        export_to_wavefront_data.full_comments = full_comments;

        int return_code = for_each_graphics_object_in_scene_tree(scene, filter,
            graphics_object_export_to_wavefront,(void *)&export_to_wavefront_data);

        if (return_code == CMZN_OK)
        {
            std::ostringstream ss;
            ss << "# CMGUI Wavefront Object file generator" << std::endl;
            for(const std::string &name : export_to_wavefront_data.object_names) {
                ss << "call " << name << std::endl;
            }
            content.push_back(ss.str());
            std::copy(export_to_wavefront_data.objects.begin(), export_to_wavefront_data.objects.end(), std::back_inserter(content));
        }

	}
	else
	{
		display_message(ERROR_MESSAGE,"export_to_wavefront.  Invalid argument(s)");
	}
	LEAVE;

    return content;
} /* export_to_wavefront */

