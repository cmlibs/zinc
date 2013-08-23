/**
 * FILE : glyph.cpp
 *
 * Glyphs produce a GT_object to represent simple geometric shapes such as
 * cylinders, arrows and axes which are (or should) fit into a unit (1x1x1) cube,
 * with the major axes of the glyph aligned with the x, y and z axes. 
 * The logical centre of each glyph should be at (0,0,0). This should be
 * interpreted as follows:
 * - if the glyph is symmetrical along any axis, its coordinates in that
 * direction should vary from -0.5 to +0.5;
 * - if the glyph involves any sort of arrow that is unsymmetric in its direction
 * (ie. is single-headed), (0,0,0) should be at the base of the arrow.
 * - axes should therefore be centred at (0,0,0) and extend to 1 in each axis
 * direction. Axis titles "x", "y" and "z" may be outside the unit cube.
 */
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
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cmath>
#include "zinc/types/fontid.h"
#include "zinc/status.h"
#include "zinc/graphicsmaterial.h"
#include "general/debug.h"
#include "general/enumerator_private.hpp"
#include "general/manager_private.h"
#include "general/message.h"
#include "general/mystring.h"
#include "graphics/glyph.hpp"
#include "graphics/glyph_axes.hpp"
#include "graphics/glyph_circular.hpp"
#include "graphics/graphics_object.h"
#include "graphics/graphics_object_private.hpp"
#include "graphics/spectrum.h"
#include "graphics/graphics_object.hpp"
#include "graphics/render_gl.h"

/*
Module functions
----------------
*/

PROTOTYPE_ACCESS_OBJECT_FUNCTION(Cmiss_glyph)
{
	return object->access();
}

PROTOTYPE_DEACCESS_OBJECT_FUNCTION(Cmiss_glyph)
{
	return Cmiss_glyph::deaccess(object_address);
}

PROTOTYPE_REACCESS_OBJECT_FUNCTION(Cmiss_glyph)
{
	if (object_address)
	{
		if (new_object)
			new_object->access();
		if (*object_address)
			Cmiss_glyph::deaccess(object_address);
		*object_address = new_object;
		return 1;
	}
	return 0;
}

DECLARE_DEFAULT_GET_OBJECT_NAME_FUNCTION(Cmiss_glyph)

DECLARE_INDEXED_LIST_STL_FUNCTIONS(Cmiss_glyph)
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_STL_FUNCTION(Cmiss_glyph,name,const char *)
DECLARE_INDEXED_LIST_STL_IDENTIFIER_CHANGE_FUNCTIONS(Cmiss_glyph,name);

FULL_DECLARE_MANAGER_TYPE(Cmiss_glyph);

DECLARE_LOCAL_MANAGER_FUNCTIONS(Cmiss_glyph)

DECLARE_MANAGER_FUNCTIONS(Cmiss_glyph, manager)

DECLARE_DEFAULT_MANAGED_OBJECT_NOT_IN_USE_FUNCTION(Cmiss_glyph, manager)

DECLARE_MANAGER_IDENTIFIER_WITHOUT_MODIFY_FUNCTIONS(Cmiss_glyph, name, const char *, manager)

Cmiss_glyph::~Cmiss_glyph()
{
	if (name)
		DEALLOCATE(name);
}

int Cmiss_glyph::setName(const char *newName)
{
	int return_code;
	if (newName)
	{
		if (this->name && (0 == strcmp(this->name, newName)))
		{
			return CMISS_OK;
		}
		return_code = CMISS_OK;
		Cmiss_set_Cmiss_glyph *allGlyphs = 0;
		bool restore_changed_object_to_lists = false;
		if (this->manager)
		{
			allGlyphs = reinterpret_cast<Cmiss_set_Cmiss_glyph *>(this->manager->object_list);
			Cmiss_glyph *existingGlyph = FIND_BY_IDENTIFIER_IN_MANAGER(Cmiss_glyph, name)(newName, this->manager);
			if (existingGlyph)
			{
				display_message(ERROR_MESSAGE, "Cmiss_glyph::setName.  Glyph named '%s' already exists.", newName);
				return_code = CMISS_ERROR_ARGUMENT;
			}
			else
			{
				// this temporarily removes the object from all related lists
				restore_changed_object_to_lists = allGlyphs->begin_identifier_change(this);
				if (!restore_changed_object_to_lists)
				{
					display_message(ERROR_MESSAGE, "Cmiss_glyph::setName.  "
						"Could not safely change identifier in manager");
					return_code = CMISS_ERROR_GENERAL;
				}
			}
		}
		if (CMISS_OK == return_code)
		{
			if (this->name)
				DEALLOCATE(this->name);
			this->name = duplicate_string(newName);
		}
		if (restore_changed_object_to_lists)
		{
			allGlyphs->end_identifier_change();
		}
		if (this->manager && return_code)
		{
			MANAGED_OBJECT_CHANGE(Cmiss_glyph)(this, MANAGER_CHANGE_IDENTIFIER(Cmiss_glyph));
		}
	}
	else
	{
		return_code = CMISS_ERROR_ARGUMENT;
	}
	return (return_code);
}

static int tick_mark_get_grid_spacing(FE_value *minor_grid_size,
	int *minor_grids_per_major,FE_value scale, FE_value min_minor_grid,
	FE_value min_major_grid)
/*******************************************************************************
LAST MODIFIED : 19 September 2005

DESCRIPTION :
Returns the spacing of minor and major (major include labels in this case) grids
that cross the drawing area. From an initial, desired <minor_grid_spacing>, this
is enlarged by multiplying by 2,2.5,2 (to give 2x,5x,10x,20x the original value)
until the grid is spaced apart by at least <min_minor_grid_pixels> on the
screen. FE_Value values are multiplied by <scale> to give pixel values.
A suitable number of minor_grids_per_major is then calculated, ensuring that
major grid lines are spaced apart by at least <min_major_grid_pixels>.
Copied from the control curve editor.
==============================================================================*/
{
	int return_code,j;

	ENTER(tick_mark_get_grid_spacing);
	if (*minor_grid_size&&(0< *minor_grid_size)&&minor_grids_per_major&&
		(0.0!=scale))
	{
		j=1;
		while (fabs(scale*(*minor_grid_size)) < min_minor_grid)
		{
			j=(j+1)%3;
			if (j)
			{
				*minor_grid_size *= 2.0;
			}
			else
			{
				*minor_grid_size *= 2.5;
			}
		}
		*minor_grids_per_major=1;
		if (0==j)
		{
			j=1;
			if (fabs(scale*(*minor_grids_per_major)*(*minor_grid_size))<
				min_major_grid)
			{
				*minor_grids_per_major *= 2;
			}
		}
		while (fabs(scale*(*minor_grids_per_major)*(*minor_grid_size))<
			min_major_grid)
		{
			j=(j+1)%2;
			if (j)
			{
				*minor_grids_per_major *= 5;
			}
			else
			{
				*minor_grids_per_major *= 2;
			}
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"tick_mark_get_grid_spacing.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* tick_mark_get_grid_spacing */

static int draw_glyph_axes_general(Triple *coordinate_scaling, 
	int label_bounds_dimension, int label_bounds_components, ZnReal *label_bounds,
	Triple *label_density,
	int primary_axis_number, int label_bounds_component,
	ZnReal major_cross_min, ZnReal major_cross_max,
	ZnReal minor_cross_min, ZnReal minor_cross_max, FE_value minor_grid_size,
	int minor_grids_per_major, FE_value min_minor_grid, FE_value min_major_grid,
	struct Graphical_material *material, struct Graphical_material *secondary_material,
	struct Cmiss_font *font, Render_graphics *renderer)
/*******************************************************************************
LAST MODIFIED : 22 November 2005

DESCRIPTION :
Renders the label_bounds as lines and labels.  
==============================================================================*/
{
	const char *name = "axes_ticks_temporary";
	char **label_strings;
	FE_value grid_scale;
	int first, j, label_bounds_offset, last, number_of_major_lines, 
		number_of_minor_lines, number_of_labels, number_of_ticks, return_code;
	ZnReal axis_position, fabs_length, length, log_scale;
	struct GT_object *graphics_object;
	struct GT_pointset *label_set;
	struct GT_polyline *polyline;
	Triple *label_string_locations, *label_vertex, *major_linepoints, *major_vertex, 
		*minor_linepoints, *minor_vertex;

	ENTER(draw_glyph_axes_general);
	if ((label_bounds_dimension > 0) && (label_bounds_components > 0) && label_bounds)
	{
		return_code=1;

		switch(primary_axis_number)
		{
			case 0:
			default:
			{
				label_bounds_offset = label_bounds_components;
			} break;
			case 1:
			{
				label_bounds_offset = 2 * label_bounds_components;
			} break;
			case 2:
			{
				label_bounds_offset = 4 * label_bounds_components;
			} break;
		}

		length = label_bounds[label_bounds_offset + label_bounds_component] -
			label_bounds[label_bounds_component];
		fabs_length = fabs(length);

		if (fabs_length > 1e-7)
		{
			minor_grid_size *= fabs_length;
			log_scale = ceil(-0.5 + log10(minor_grid_size * 2.0));
			minor_grid_size = pow(10, log_scale) / 2.0;
			if (label_density)
			{
				grid_scale = 0.01 * (*label_density)[0] / fabs_length;				
			}
			else
			{
				grid_scale = (*coordinate_scaling)[0] / fabs_length;
			}
			
			tick_mark_get_grid_spacing(&minor_grid_size,
				&minor_grids_per_major, grid_scale, min_minor_grid, min_major_grid);
			
			first=(int)ceil(label_bounds[label_bounds_component] / minor_grid_size);
			last=(int)floor(label_bounds[label_bounds_offset + label_bounds_component]
				/ minor_grid_size);
		}
		else
		{
			length = 1e-7f;
			if (label_density)
			{
				grid_scale = 0.01 * (*label_density)[0];				
			}
			else
			{
				grid_scale = 0.01 * (*coordinate_scaling)[0];
			}

			minor_grids_per_major = 5;
  			tick_mark_get_grid_spacing(&minor_grid_size,
				&minor_grids_per_major, grid_scale, min_minor_grid, min_major_grid);

			first = (int)ceil(label_bounds[label_bounds_component] / minor_grid_size);
			last = first;
		}								
		if (first > last)
		{
			j = last;
			last = first;
			first = j;
		}
		number_of_ticks = last - first + 1;

		number_of_minor_lines = 0;
		number_of_major_lines = 0;
		number_of_labels = 0;
		/* This is too much memory but saves us working it out in advance */
		if (ALLOCATE(major_linepoints, Triple, 2 * number_of_ticks) && 
			ALLOCATE(minor_linepoints, Triple, 2 * number_of_ticks) && 
			ALLOCATE(label_string_locations, Triple, number_of_ticks) &&
			ALLOCATE(label_strings, char *, number_of_ticks))
		{
			minor_vertex = minor_linepoints;
			major_vertex = major_linepoints;
			label_vertex = label_string_locations;
			for (j=first;j<=last;j++)
			{
				if (number_of_ticks > 1)
				{
					axis_position = ((ZnReal)j * minor_grid_size - label_bounds[label_bounds_component]) / length;
				}
				else
				{
					axis_position = 0.0;
				}
				if (0==(j % minor_grids_per_major))
				{
					/* Major */
					switch (primary_axis_number)
					{
						case 0:
						default:
						{
							(*major_vertex)[0] = axis_position;
							(*major_vertex)[1] = major_cross_min;
							(*major_vertex)[2] = 0.0;
							major_vertex++;
							(*major_vertex)[0] = axis_position;
							(*major_vertex)[1] = major_cross_max;
							(*major_vertex)[2] = 0.0;
							major_vertex++;
							(*label_vertex)[0] = axis_position;
							(*label_vertex)[1] = 0.0;
							(*label_vertex)[2] = 0.0;
							label_vertex++;
						} break;
						case 1:
						{
							(*major_vertex)[0] = major_cross_min;
							(*major_vertex)[1] = axis_position;
							(*major_vertex)[2] = 0.0;
							major_vertex++;
							(*major_vertex)[0] = major_cross_max;
							(*major_vertex)[1] = axis_position;
							(*major_vertex)[2] = 0.0;
							major_vertex++;
							(*label_vertex)[0] = 0.0;
							(*label_vertex)[1] = axis_position;
							(*label_vertex)[2] = 0.0;
							label_vertex++;
						} break;
						case 2:
						{
							(*major_vertex)[0] = major_cross_min;
							(*major_vertex)[1] = 0.0;
							(*major_vertex)[2] = axis_position;
							major_vertex++;
							(*major_vertex)[0] = major_cross_max;
							(*major_vertex)[1] = 0.0;
							(*major_vertex)[2] = axis_position;
							major_vertex++;
							(*label_vertex)[0] = 0.0;
							(*label_vertex)[1] = 0.0;
							(*label_vertex)[2] = axis_position;
							label_vertex++;
						} break;
					}
					number_of_major_lines++;

					if (ALLOCATE(label_strings[number_of_labels], char, 50))
					{
						sprintf(label_strings[number_of_labels], "%1g",
							(ZnReal)j * minor_grid_size);
					}
					number_of_labels++;
				}
				else
				{
					/* Minor */
					switch (primary_axis_number)
					{
						case 0:
						default:
						{
							(*minor_vertex)[0] = axis_position;
							(*minor_vertex)[1] = minor_cross_min;
							(*minor_vertex)[2] = 0.0;
							minor_vertex++;
							(*minor_vertex)[0] = axis_position;
							(*minor_vertex)[1] = minor_cross_max;
							(*minor_vertex)[2] = 0.0;
							minor_vertex++;
						} break;
						case 1:
						{
							(*minor_vertex)[0] = minor_cross_min;
							(*minor_vertex)[1] = axis_position;
							(*minor_vertex)[2] = 0.0;
							minor_vertex++;
							(*minor_vertex)[0] = minor_cross_max;
							(*minor_vertex)[1] = axis_position;
							(*minor_vertex)[2] = 0.0;
							minor_vertex++;
						} break;
						case 2:
						{
							(*minor_vertex)[0] = minor_cross_min;
							(*minor_vertex)[1] = 0.0;
							(*minor_vertex)[2] = axis_position;
							minor_vertex++;
							(*minor_vertex)[0] = minor_cross_max;
							(*minor_vertex)[1] = 0.0;
							(*minor_vertex)[2] = axis_position;
							minor_vertex++;
						} break;
					}
					number_of_minor_lines++;
				}
			}
			polyline=CREATE(GT_polyline)(g_PLAIN_DISCONTINUOUS,
				number_of_major_lines, major_linepoints,/*normalpoints*/(Triple *)NULL,g_NO_DATA,(GLfloat *)NULL);
			if (polyline)
			{
				graphics_object=CREATE(GT_object)(name,g_POLYLINE,
					/*use the default material, must be before graphics which specify a material*/
					(struct Graphical_material *)NULL);
				if (GT_OBJECT_ADD(GT_polyline)(graphics_object,/*time*/0.0,polyline))
				{
					renderer->Graphics_object_render_immediate(graphics_object);
				}
				else
				{
					DESTROY(GT_polyline)(&polyline);
					return_code = 0;
				}
				DEACCESS(GT_object)(&graphics_object);
			}
			else
			{
				DEALLOCATE(major_linepoints);
			}
			label_set = CREATE(GT_pointset)(number_of_labels, label_string_locations,
				label_strings, g_NO_MARKER, /*marker_size*/0, /*n_data_components*/0,
				(GLfloat *)NULL, /*names*/(int *)NULL, font);
			if (label_set)
			{
				graphics_object=CREATE(GT_object)(name, g_POINTSET, material);
				if (GT_OBJECT_ADD(GT_pointset)(graphics_object,/*time*/0.0,label_set))
				{
					renderer->Graphics_object_render_immediate(graphics_object);
				}
				else
				{
					DESTROY(GT_pointset)(&label_set);
					return_code = 0;
				}
				DEACCESS(GT_object)(&graphics_object);
			}
			else
			{
				DEALLOCATE(label_string_locations);
				for (j = 0 ; j < number_of_labels ; j++)
				{
					DEALLOCATE(label_strings[j]);
				}
				DEALLOCATE(label_strings);
			}
			polyline=CREATE(GT_polyline)(g_PLAIN_DISCONTINUOUS,
				number_of_minor_lines, minor_linepoints,/*normalpoints*/(Triple *)NULL,g_NO_DATA,(GLfloat *)NULL);
			if (polyline)
			{
				graphics_object=CREATE(GT_object)(name,g_POLYLINE, secondary_material);
				if (GT_OBJECT_ADD(GT_polyline)(graphics_object,/*time*/0.0,polyline))
				{
					renderer->Graphics_object_render_immediate(graphics_object);
				}
				else
				{
					DESTROY(GT_polyline)(&polyline);
					return_code = 0;
				}
				DEACCESS(GT_object)(&graphics_object);
			}
			else
			{
				DEALLOCATE(minor_linepoints);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"draw_glyph_axes_general.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* draw_glyph_axes_general */

static int draw_glyph_axes_ticks(Triple *coordinate_scaling, 
	int label_bounds_dimension, int label_bounds_components, ZnReal *label_bounds,
	Triple *label_density,
	struct Graphical_material *material, struct Graphical_material *secondary_material,
	struct Cmiss_font *font, Render_graphics *renderer)
/*******************************************************************************
LAST MODIFIED : 22 November 2005

DESCRIPTION :
Renders the label_bounds as a grid in the first two axis directions.  
An assumption is made that the label values have some alignment with the axis 
so only the first component of the label_bounds is drawn.
==============================================================================*/
{
	int return_code;

	ENTER(draw_glyph_axes_ticks);
	if ((label_bounds_dimension > 0) && (label_bounds_components > 0)
		&& label_bounds)
	{
		return_code = draw_glyph_axes_general(coordinate_scaling, 
			label_bounds_dimension, label_bounds_components, label_bounds,
			label_density,
			/*primary_axis_number*/0, /*label_bounds_component*/0,
			/*major_cross_min*/-0.05f, /*major_cross_max*/0.05f,
			/*minor_cross_min*/-0.01f, /*minor_cross_max*/0.01f,
			/*minor_grid_size*/0.01, /*minor_grids_per_major*/5,
			/*min_minor_grid*/0.01, /*min_major_grid*/0.1,
			material, secondary_material, font, renderer);
	}
	else
	{
		display_message(ERROR_MESSAGE,"draw_glyph_axes_ticks.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* draw_glyph_axes_ticks */

static int draw_glyph_grid_lines(Triple *coordinate_scaling, 
	int label_bounds_dimension, int label_bounds_components, ZnReal *label_bounds,
	Triple *label_density,
	struct Graphical_material *material, struct Graphical_material *secondary_material,
	struct Cmiss_font *font, Render_graphics *renderer)
/*******************************************************************************
LAST MODIFIED : 22 November 2005

DESCRIPTION :
Renders the label_bounds as a grid in the first two axis directions.  
It is assumed that the label values have some alignment with the axes so only
the first component of the label_bounds is drawn along the first axis and the 
second component on the second axis.
==============================================================================*/
{
	int return_code;

	ENTER(draw_glyph_axes_ticks);
	if ((label_bounds_dimension > 0) && (label_bounds_components > 0)
		&& label_bounds)
	{
		return_code = draw_glyph_axes_general(coordinate_scaling, 
			label_bounds_dimension, label_bounds_components, label_bounds,
			label_density,
			/*primary_axis_number*/0, /*label_bounds_component*/0,
			/*major_cross_min*/-0.01f, /*major_cross_max*/1.01f,
			/*minor_cross_min*/-0.01f, /*minor_cross_max*/1.01f,
			/*minor_grid_size*/0.01, /*minor_grids_per_major*/2,
			/*min_minor_grid*/0.1, /*min_major_grid*/0.2,
			material, secondary_material, font, renderer);
		return_code = draw_glyph_axes_general(coordinate_scaling, 
			label_bounds_dimension, label_bounds_components, label_bounds,
			label_density,
			/*primary_axis_number*/1, /*label_bounds_component*/1,
			/*major_cross_min*/-0.01f, /*major_cross_max*/1.01f,
			/*minor_cross_min*/-0.01f, /*minor_cross_max*/1.01f,
			/*minor_grid_size*/0.01, /*minor_grids_per_major*/2,
			/*min_minor_grid*/0.1, /*min_major_grid*/0.2,
			material, secondary_material, font, renderer);
	}
	else
	{
		display_message(ERROR_MESSAGE,"draw_glyph_axes_ticks.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* draw_glyph_axes_ticks */

/*
Global functions
----------------
*/

PROTOTYPE_ENUMERATOR_STRING_FUNCTION(Cmiss_glyph_repeat_mode)
{
	switch (enumerator_value)
	{
		case CMISS_GLYPH_REPEAT_NONE:
			return "REPEAT_NONE";
			break;
		case CMISS_GLYPH_REPEAT_AXES_2D:
			return "REPEAT_AXES_2D";
			break;
		case CMISS_GLYPH_REPEAT_AXES_3D:
			return "REPEAT_AXES_3D";
			break;
		case CMISS_GLYPH_REPEAT_MIRROR:
			return "REPEAT_MIRROR";
			break;
		default:
			// fall through to normal return
			break;
	}
	return 0;
}

DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS(Cmiss_glyph_repeat_mode)

int Cmiss_glyph_repeat_mode_get_number_of_glyphs(
	enum Cmiss_glyph_repeat_mode glyph_repeat_mode)
{
	switch (glyph_repeat_mode)
	{
		case CMISS_GLYPH_REPEAT_NONE:
			return 1;
			break;
		case CMISS_GLYPH_REPEAT_AXES_2D:
		case CMISS_GLYPH_REPEAT_MIRROR:
			return 2;
			break;
		case CMISS_GLYPH_REPEAT_AXES_3D:
			return 3;
			break;
		default:
			// fall through to normal return
			break;
	}
	return 0;
}

bool Cmiss_glyph_repeat_mode_glyph_number_has_label(
	enum Cmiss_glyph_repeat_mode glyph_repeat_mode, int glyph_number)
{
	switch (glyph_repeat_mode)
	{
		case CMISS_GLYPH_REPEAT_NONE:
		case CMISS_GLYPH_REPEAT_MIRROR:
			return (glyph_number == 0);
			break;
		case CMISS_GLYPH_REPEAT_AXES_2D:
			return (glyph_number < 2);
			break;
		case CMISS_GLYPH_REPEAT_AXES_3D:
			return (glyph_number < 3);
			break;
		default:
			// fall through to normal return
			break;
	}
	return false;
}

void resolve_glyph_axes(
	enum Cmiss_glyph_repeat_mode glyph_repeat_mode, int glyph_number,
	Triple base_size, Triple scale_factors, Triple offset,
	Triple point, Triple axis1, Triple axis2, Triple axis3, Triple scale,
	Triple final_point, Triple final_axis1, Triple final_axis2, Triple final_axis3)
{
	switch (glyph_repeat_mode)
	{
	case CMISS_GLYPH_REPEAT_NONE:
	case CMISS_GLYPH_REPEAT_MIRROR:
	default:
		{
			Triple axis_scale;
			for (int j = 0; j < 3; j++)
			{
				const FE_value sign = (scale[j] < 0.0) ? -1.0 : 1.0;
				axis_scale[j] = sign*base_size[j] + scale[j]*scale_factors[j];
			}
			for (int j = 0; j < 3; j++)
			{
				final_axis1[j] = axis1[j]*axis_scale[0];
				final_axis2[j] = axis2[j]*axis_scale[1];
				final_axis3[j] = axis3[j]*axis_scale[2];
				final_point[j] = point[j]
					+ offset[0]*final_axis1[j]
					+ offset[1]*final_axis2[j]
					+ offset[2]*final_axis3[j];
				if (glyph_repeat_mode == CMISS_GLYPH_REPEAT_MIRROR)
				{
					if (glyph_number == 1)
					{
						final_axis1[j] = -final_axis1[j];
						final_axis2[j] = -final_axis2[j];
						final_axis3[j] = -final_axis3[j];
					}
					if (scale[0] < 0.0)
					{
						// shift glyph origin to end of axis1 
						final_point[j] -= final_axis1[j];
					}
				}
			}
			/* if required, reverse axis3 to maintain right-handed coordinate system */
			if (0.0 > (
				final_axis3[0]*(final_axis1[1]*final_axis2[2] -
					final_axis1[2]*final_axis2[1]) +
				final_axis3[1]*(final_axis1[2]*final_axis2[0] -
					final_axis1[0]*final_axis2[2]) +
				final_axis3[2]*(final_axis1[0]*final_axis2[1] -
					final_axis1[1]*final_axis2[0])))
			{
				final_axis3[0] = -final_axis3[0];
				final_axis3[1] = -final_axis3[1];
				final_axis3[2] = -final_axis3[2];
			}
		} break;
	case CMISS_GLYPH_REPEAT_AXES_2D:
	case CMISS_GLYPH_REPEAT_AXES_3D:
		{
			Triple axis_scale;
			for (int j = 0; j < 3; j++)
			{
				const FE_value sign = (scale[j] < 0.0) ? -1.0 : 1.0;
				axis_scale[j] = sign*base_size[0] + scale[j]*scale_factors[0];
			}
			for (int j = 0; j < 3; j++)
			{
				final_point[j] = point[j]
					+ offset[0]*axis_scale[0]*axis1[j]
					+ offset[1]*axis_scale[1]*axis2[j]
					+ offset[2]*axis_scale[2]*axis3[j];
			}
			GLfloat *use_axis1, *use_axis2;
			const GLfloat use_scale = scale[glyph_number];
			if (glyph_number == 0)
			{
				use_axis1 = axis1;
				use_axis2 = axis2;
			}
			else if (glyph_number == 1)
			{
				use_axis1 = axis2;
				use_axis2 = (glyph_repeat_mode == CMISS_GLYPH_REPEAT_AXES_2D) ? axis1 : axis3;
			}
			else // if (glyph_number == 2)
			{
				use_axis1 = axis3;
				use_axis2 = axis1;
			}
			GLfloat final_scale1 = base_size[0] + use_scale*scale_factors[0];
			final_axis1[0] = use_axis1[0]*final_scale1;
			final_axis1[1] = use_axis1[1]*final_scale1;
			final_axis1[2] = use_axis1[2]*final_scale1;
			// final_axis3 = normalise(final_axis1 x use_axis2)*scale[0];
			final_axis3[0] = final_axis1[1]*use_axis2[2] - use_axis2[1]*final_axis1[2];
			final_axis3[1] = final_axis1[2]*use_axis2[0] - use_axis2[2]*final_axis1[0];
			final_axis3[2] = final_axis1[0]*use_axis2[1] - final_axis1[1]*use_axis2[0];
			GLfloat magnitude = sqrt(final_axis3[0]*final_axis3[0] + final_axis3[1]*final_axis3[1] + final_axis3[2]*final_axis3[2]);
			if (0.0 < magnitude)
			{
				GLfloat scaling = (base_size[2] + use_scale*scale_factors[2]) / magnitude;
				if ((glyph_repeat_mode == CMISS_GLYPH_REPEAT_AXES_2D) && (glyph_number > 0))
				{
					scaling *= -1.0;
				}
				final_axis3[0] *= scaling;
				final_axis3[1] *= scaling;
				final_axis3[2] *= scaling;
			}
			// final_axis2 = normalise(final_axis3 x final_axis1)*scale[0];
			final_axis2[0] = final_axis3[1]*final_axis1[2] - final_axis1[1]*final_axis3[2];
			final_axis2[1] = final_axis3[2]*final_axis1[0] - final_axis1[2]*final_axis3[0];
			final_axis2[2] = final_axis3[0]*final_axis1[1] - final_axis3[1]*final_axis1[0];
			magnitude = sqrt(final_axis2[0]*final_axis2[0] + final_axis2[1]*final_axis2[1] + final_axis2[2]*final_axis2[2]);
			if (0.0 < magnitude)
			{
				GLfloat scaling = (base_size[1] + use_scale*scale_factors[1]) / magnitude;
				final_axis2[0] *= scaling;
				final_axis2[1] *= scaling;
				final_axis2[2] *= scaling;
			}
		} break;
	}			
}

/**
 * Creates a graphics object named <name> consisting of a line from <0,0,0> to
 * <1,0,0> with 4 arrow head ticks <head_length> long and <half_head_width> out
 * from the shaft.
 */
struct GT_object *create_GT_object_arrow_line(const char *name,ZnReal head_length,
	ZnReal half_head_width)
{
	int j;
	struct GT_object *glyph = 0;
	struct GT_polyline *polyline;
	Triple *points,*vertex;

	ENTER(create_GT_object_arrow_line);
	if (name)
	{
		polyline=(struct GT_polyline *)NULL;
		if (ALLOCATE(points,Triple,10))
		{
			vertex=points;
			/* most coordinates are 0.0, so clear them all to that */
			for (j=0;j<10;j++)
			{
				(*vertex)[0]=0.0;
				(*vertex)[1]=0.0;
				(*vertex)[2]=0.0;
				vertex++;
			}
			/* x-axis */
			points[ 1][0]=1.0;
			points[ 2][0]=1.0;
			points[ 3][0]=1.0-head_length;
			points[ 3][1]=half_head_width;
			points[ 4][0]=1.0;
			points[ 5][0]=1.0-head_length;
			points[ 5][2]=half_head_width;
			points[ 6][0]=1.0;
			points[ 7][0]=1.0-head_length;
			points[ 7][1]=-half_head_width;
			points[ 8][0]=1.0;
			points[ 9][0]=1.0-head_length;
			points[ 9][2]=-half_head_width;
			if (!(polyline=CREATE(GT_polyline)(g_PLAIN_DISCONTINUOUS,
				5,points,/*normalpoints*/(Triple *)NULL,g_NO_DATA,(GLfloat *)NULL)))
			{
				DEALLOCATE(points);
			}
		}
		if (polyline)
		{
			glyph=CREATE(GT_object)(name,g_POLYLINE,(struct Graphical_material *)NULL);
			if (!GT_OBJECT_ADD(GT_polyline)(glyph,/*time*/0.0,polyline))
			{
				DEACCESS(GT_object)(&glyph);
				DESTROY(GT_polyline)(&polyline);
			}
		}
		if (!glyph)
		{
			display_message(ERROR_MESSAGE,
				"create_GT_object_arrow_line.  Error creating glyph");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_GT_object_arrow_line.  Invalid argument(s)");
	}
	LEAVE;

	return (glyph);
}

/**
 * Creates a graphics object named <name> consisting of three axis arrows heading
 * from <0,0,0> to 1 in each of their directions. The arrows are made up of lines,
 * with a 4-way arrow head so it looks normal from the other two axes. If <labels>
 * is specified then it is assumed to point to an array of 3 strings which will
 * be used to label each arrow and are attached to it so
 * that the two objects are displayed and destroyed together. The labels are
 * located on the respective axes <label_offset> past 1.0.
 * The length and width of the arrow heads are specified by the final parameters.
 */
struct GT_object *create_GT_object_axes(const char *name, int make_solid, ZnReal head_length,
	ZnReal half_head_width,const char **labels, ZnReal label_offset,
	struct Cmiss_font *font)
{
	char *glyph_name,**text;
	int j;
	struct Colour colour;
	struct Graphical_material *material;
	struct GT_object *glyph = 0, *last_object = 0;
	struct GT_pointset *pointset;
	struct GT_polyline *polyline;
	Triple *points,*vertex;

	ENTER(create_GT_object_axes);
	if (name)
	{
		last_object = (struct GT_object *)NULL;
		if (make_solid)
		{
			if (ALLOCATE(glyph_name, char, strlen(name) + 8))
			{
				glyph = create_GT_object_arrow_solid(name, /*primary_axis*/1,
					/*number_of_segments_around*/12, /*shaft_length*/2.f/3.f,
					/*shaft_radius*/1.f/20.f, /*cone_radius*/1.f/8.f);
				material = Cmiss_graphics_material_create_private();
				Cmiss_graphics_material_set_name(material, "red");
				colour.red = 1;
				colour.green = 0;
				colour.blue = 0;
				Graphical_material_set_diffuse(material, &colour);
				set_GT_object_default_material(glyph, material);
				Cmiss_graphics_material_destroy(&material);
				last_object = glyph;

				sprintf(glyph_name, "%s_arrow2", name);
				GT_object *arrow2 = create_GT_object_arrow_solid(glyph_name, /*primary_axis*/2,
					/*number_of_segments_around*/12, /*shaft_length*/2.f/3.f,
					/*shaft_radius*/1.f/20.f, /*cone_radius*/1.f/8.f);
				material = Cmiss_graphics_material_create_private();
				Cmiss_graphics_material_set_name(material, "green");
				colour.red = 0;
				colour.green = 1;
				colour.blue = 0;
				Graphical_material_set_diffuse(material, &colour);
				set_GT_object_default_material(arrow2, material);
				Cmiss_graphics_material_destroy(&material);
				GT_object_set_next_object(last_object, arrow2);
				last_object = arrow2;
				DEACCESS(GT_object)(&arrow2);

				sprintf(glyph_name, "%s_arrow3", name);
				GT_object *arrow3 = create_GT_object_arrow_solid(glyph_name, /*primary_axis*/3,
					/*number_of_segments_around*/12, /*shaft_length*/2.f/3.f,
					/*shaft_radius*/1.f/20.f, /*cone_radius*/1.f/8.f);
				material = Cmiss_graphics_material_create_private();
				Cmiss_graphics_material_set_name(material, "blue");
				colour.red = 0;
				colour.green = 0;
				colour.blue = 1;
				Graphical_material_set_diffuse(material, &colour);
				set_GT_object_default_material(arrow3, material);
				Cmiss_graphics_material_destroy(&material);
				GT_object_set_next_object(last_object, arrow3);
				last_object = arrow3;
				DEACCESS(GT_object)(&arrow3);
				
				DEALLOCATE(glyph_name);
			}
		}
		else
		{
			polyline=(struct GT_polyline *)NULL;
			if (ALLOCATE(points,Triple,30))
			{
				vertex=points;
				/* most coordinates are 0.0, so clear them all to that */
				for (j=0;j<30;j++)
				{
					(*vertex)[0]=0.0;
					(*vertex)[1]=0.0;
					(*vertex)[2]=0.0;
					vertex++;
				}
				/* x-axis */
				points[ 1][0]=1.0;
				points[ 2][0]=1.0;
				points[ 3][0]=1.0-head_length;
				points[ 3][1]=half_head_width;
				points[ 4][0]=1.0;
				points[ 5][0]=1.0-head_length;
				points[ 5][2]=half_head_width;
				points[ 6][0]=1.0;
				points[ 7][0]=1.0-head_length;
				points[ 7][1]=-half_head_width;
				points[ 8][0]=1.0;
				points[ 9][0]=1.0-head_length;
				points[ 9][2]=-half_head_width;
				/* y-axis */
				points[11][1]=1.0;
				points[12][1]=1.0;
				points[13][1]=1.0-head_length;
				points[13][2]=half_head_width;
				points[14][1]=1.0;
				points[15][1]=1.0-head_length;
				points[15][0]=half_head_width;
				points[16][1]=1.0;
				points[17][1]=1.0-head_length;
				points[17][2]=-half_head_width;
				points[18][1]=1.0;
				points[19][1]=1.0-head_length;
				points[19][0]=-half_head_width;
				/* z-axis */
				points[21][2]=1.0;
				points[22][2]=1.0;
				points[23][2]=1.0-head_length;
				points[23][0]=half_head_width;
				points[24][2]=1.0;
				points[25][2]=1.0-head_length;
				points[25][1]=half_head_width;
				points[26][2]=1.0;
				points[27][2]=1.0-head_length;
				points[27][0]=-half_head_width;
				points[28][2]=1.0;
				points[29][2]=1.0-head_length;
				points[29][1]=-half_head_width;
				polyline=CREATE(GT_polyline)(g_PLAIN_DISCONTINUOUS,
					15,points,/*normalpoints*/(Triple *)NULL,g_NO_DATA,(GLfloat *)NULL);
				if (polyline)
				{
					glyph=CREATE(GT_object)(name,g_POLYLINE, (struct Graphical_material *)NULL);
					GT_OBJECT_ADD(GT_polyline)(glyph,/*time*/0.0,polyline);
					last_object = glyph;
				}
				else
				{
					DEALLOCATE(points);
				}
			}
		}
		if (glyph && labels)
		{
			pointset=(struct GT_pointset *)NULL;
			if (ALLOCATE(points,Triple,3)&&
				ALLOCATE(text,char *,3)&&
				ALLOCATE(text[0],char,strlen(labels[0]) + 1)&&
				ALLOCATE(text[1],char,strlen(labels[1]) + 1)&&
				ALLOCATE(text[2],char,strlen(labels[2]) + 1)&&
				ALLOCATE(glyph_name,char,strlen(name)+8))
			{
				sprintf(glyph_name,"%s_labels",name);
				points[0][0]=1.0+label_offset;
				points[0][1]=0.0;
				points[0][2]=0.0;
				strcpy(text[0],labels[0]);
				points[1][0]=0.0;
				points[1][1]=1.0+label_offset;
				points[1][2]=0.0;
				strcpy(text[1],labels[1]);
				points[2][0]=0.0;
				points[2][1]=0.0;
				points[2][2]=1.0+label_offset;
				strcpy(text[2],labels[2]);
				pointset=CREATE(GT_pointset)(3,points,text,g_NO_MARKER,0.0,
					g_NO_DATA,(GLfloat *)NULL,(int *)NULL, font);
				if (pointset)
				{
					GT_object *labels_object = CREATE(GT_object)(glyph_name,g_POINTSET,(struct Graphical_material *)NULL);
					GT_OBJECT_ADD(GT_pointset)(labels_object,/*time*/0.0,pointset);
					GT_object_set_next_object(last_object, labels_object);
					DEACCESS(GT_object)(&labels_object);
				}
				else
				{
					DEALLOCATE(text[0]);
					DEALLOCATE(text[1]);
					DEALLOCATE(text[2]);
					DEALLOCATE(text);
					DEALLOCATE(points);
				}
				DEALLOCATE(glyph_name);
			}
		}
		if (!glyph)
		{
			display_message(ERROR_MESSAGE,"create_GT_object_axes.  Error creating glyph");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"create_GT_object_axes.  Invalid argument(s)");
	}
	LEAVE;

	return (glyph);
}

/**
 * Creates a graphics object named <name> consisting of a 3 lines:
 * from <-0.5,0,0> to <+0.5,0,0>
 * from <0,-0.5,0> to <0,+0.5,0>
 * from <0,0,-0.5> to <0,0,+0.5>
 */
struct GT_object *create_GT_object_cross(const char *name)
{
	struct GT_object *glyph = 0;
	struct GT_polyline *polyline;
	Triple *points;

	ENTER(create_GT_object_cross);
	if (name)
	{
		polyline=(struct GT_polyline *)NULL;
		if (ALLOCATE(points,Triple,6))
		{
			/* x-line */
			points[0][0]=-0.5;
			points[0][1]=0.0;
			points[0][2]=0.0;
			points[1][0]=+0.5;
			points[1][1]=0.0;
			points[1][2]=0.0;
			/* y-line */
			points[2][0]=0.0;
			points[2][1]=-0.5;
			points[2][2]=0.0;
			points[3][0]=0.0;
			points[3][1]=+0.5;
			points[3][2]=0.0;
			/* z-line */
			points[4][0]=0.0;
			points[4][1]=0.0;
			points[4][2]=-0.5;
			points[5][0]=0.0;
			points[5][1]=0.0;
			points[5][2]=+0.5;
			if (!(polyline=CREATE(GT_polyline)(g_PLAIN_DISCONTINUOUS,
				3,points,/*normalpoints*/(Triple *)NULL,g_NO_DATA,(GLfloat *)NULL)))
			{
				DEALLOCATE(points);
			}
		}
		if (polyline)
		{
			glyph=CREATE(GT_object)(name,g_POLYLINE, (struct Graphical_material *)NULL);
			if (!GT_OBJECT_ADD(GT_polyline)(glyph,/*time*/0.0,polyline))
			{
				DEACCESS(GT_object)(&glyph);
				DESTROY(GT_polyline)(&polyline);
			}
		}
		if (!glyph)
		{
			display_message(ERROR_MESSAGE,"create_GT_object_cross.  Error creating glyph");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"create_GT_object_cross.  Invalid argument(s)");
	}
	LEAVE;

	return (glyph);
}

/**
 * Creates a graphics object named <name> consisting of a unit-sized GT_surface
 * cube centred at <0,0,0>.
 */
struct GT_object *create_GT_object_cube_solid(const char *name)
{
	ZnReal factor;
	int a, b, c, i;
	struct GT_object *glyph = 0;
	struct GT_surface *surface;
	Triple *point, *points, *normalpoint, *normalpoints;

	ENTER(create_GT_object_cube_solid);
	if (name)
	{
		surface=(struct GT_surface *)NULL;
		if (ALLOCATE(points,Triple,24)&&
			ALLOCATE(normalpoints,Triple,24))
		{
			point = points;
			normalpoint = normalpoints;
			/* all coordinates are +0.5 or -0.5, so clear them all to former */
			for (i = 0; i < 6; i++)
			{
				a = i / 2;
				if ((2*a) == i)
				{
					factor = -1.0;
				}
				else
				{
					factor = 1.0;
				}
				b = (a + 1) % 3;
				c = (a + 2) % 3;
				/* vertices */
				(*point)[a] = 0.5*factor;
				(*point)[b] = 0.5*factor;
				(*point)[c] = 0.5;
				point++;
				(*point)[a] = 0.5*factor;
				(*point)[b] = -0.5*factor;
				(*point)[c] = 0.5;
				point++;
				(*point)[a] = 0.5*factor;
				(*point)[b] = -0.5*factor;
				(*point)[c] = -0.5;
				point++;
				(*point)[a] = 0.5*factor;
				(*point)[b] = 0.5*factor;
				(*point)[c] = -0.5;
				point++;
				/* normals */
				(*normalpoint)[a] = factor;
				(*normalpoint)[b] = 0.0;
				(*normalpoint)[c] = 0.0;
				normalpoint++;
				(*normalpoint)[a] = factor;
				(*normalpoint)[b] = 0.0;
				(*normalpoint)[c] = 0.0;
				normalpoint++;
				(*normalpoint)[a] = factor;
				(*normalpoint)[b] = 0.0;
				(*normalpoint)[c] = 0.0;
				normalpoint++;
				(*normalpoint)[a] = factor;
				(*normalpoint)[b] = 0.0;
				(*normalpoint)[c] = 0.0;
				normalpoint++;
			}
			if (!(surface=CREATE(GT_surface)(g_SH_DISCONTINUOUS,CMISS_GRAPHIC_RENDER_POLYGON_SHADED,
				g_QUADRILATERAL,6,4,points,normalpoints,/*tangentpoints*/(Triple *)NULL,
				/*texturepoints*/(Triple *)NULL,g_NO_DATA,(GLfloat *)NULL)))
			{
				DEALLOCATE(points);
				DEALLOCATE(normalpoints);
			}
		}
		if (surface)
		{
			glyph=CREATE(GT_object)(name,g_SURFACE,	(struct Graphical_material *)NULL);
			if (!GT_OBJECT_ADD(GT_surface)(glyph,/*time*/0.0,surface))
			{
				DEACCESS(GT_object)(&glyph);
				DESTROY(GT_surface)(&surface);
			}
		}
		if (!glyph)
		{
			display_message(ERROR_MESSAGE,
				"create_GT_object_cube_solid.  Error creating glyph");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"create_GT_object_cube_solid.  Invalid argument(s)");
	}
	LEAVE;

	return (glyph);
}

/**
 * Creates a graphics object named <name> consisting of lines marking a
 * unit-sized wireframe cube centred at <0,0,0>.
 */
struct GT_object *create_GT_object_cube_wireframe(const char *name)
{
	int a, b, c, i;
	struct GT_object *glyph = 0;
	struct GT_polyline *polyline;
	Triple *points, *vertex;

	ENTER(create_GT_object_cube_wireframe);
	if (name)
	{
		polyline = (struct GT_polyline *)NULL;
		if (ALLOCATE(points, Triple, 24))
		{
			vertex=points;
			/* all coordinates are +0.5 or -0.5, so clear them all to former */
			for (a = 0; a < 3; a++)
			{
				b = (a + 1) % 3;
				c = (a + 2) % 3;
				for (i = 0; i < 8; i++)
				{
					if (0 == (i % 2))
					{
						(*vertex)[a] = -0.5;
					}
					else
					{
						(*vertex)[a] = 0.5;
					}
					if (2 > (i % 4))
					{
						(*vertex)[b] = -0.5;
					}
					else
					{
						(*vertex)[b] = 0.5;
					}
					if (4 > i)
					{
						(*vertex)[c] = -0.5;
					}
					else
					{
						(*vertex)[c] = 0.5;
					}
					vertex++;
				}
			}
			if (!(polyline=CREATE(GT_polyline)(g_PLAIN_DISCONTINUOUS,
				12,points,/*normalpoints*/(Triple *)NULL,g_NO_DATA,(GLfloat *)NULL)))
			{
				DEALLOCATE(points);
			}
		}
		if (polyline)
		{
			glyph=CREATE(GT_object)(name,g_POLYLINE, (struct Graphical_material *)NULL);
			if (!GT_OBJECT_ADD(GT_polyline)(glyph,/*time*/0.0,polyline))
			{
				DEACCESS(GT_object)(&glyph);
				DESTROY(GT_polyline)(&polyline);
			}
		}
		if (!glyph)
		{
			display_message(ERROR_MESSAGE,
				"create_GT_object_cube_wireframe.  Error creating glyph");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_GT_object_cube_wireframe.  Invalid argument(s)");
	}
	LEAVE;

	return (glyph);
}

/**
 * Creates a graphics object named <name> consisting of a line from <0,0,0> to
 * <1,0,0>.
 */
struct GT_object *create_GT_object_line(const char *name)
{
	struct GT_object *glyph = 0;
	struct GT_polyline *polyline;
	Triple *points;

	ENTER(create_GT_object_line);
	if (name)
	{
		polyline=(struct GT_polyline *)NULL;
		if (ALLOCATE(points,Triple,2))
		{
			points[0][0]=0.0;
			points[0][1]=0.0;
			points[0][2]=0.0;
			points[1][0]=1.0;
			points[1][1]=0.0;
			points[1][2]=0.0;
			if (!(polyline=CREATE(GT_polyline)(g_PLAIN_DISCONTINUOUS,
				1,points,/*normalpoints*/(Triple *)NULL,g_NO_DATA,(GLfloat *)NULL)))
			{
				DEALLOCATE(points);
			}
		}
		if (polyline)
		{
			glyph=CREATE(GT_object)(name,g_POLYLINE, (struct Graphical_material *)NULL);
			if (!GT_OBJECT_ADD(GT_polyline)(glyph,/*time*/0.0,polyline))
			{
				DEACCESS(GT_object)(&glyph);
				DESTROY(GT_polyline)(&polyline);
			}
		}
		if (!glyph)
		{
			display_message(ERROR_MESSAGE,"create_GT_object_line.  Error creating glyph");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"create_GT_object_line.  Invalid argument(s)");
	}
	LEAVE;

	return (glyph);
}

/**
 * Creates a graphics object named <name> consisting of a single point at
 * <0,0,0>.
 * The point will be drawn with the given <marker_type> and <marker_size>.
 */
struct GT_object *create_GT_object_point(const char *name,gtMarkerType marker_type,
	ZnReal marker_size)
{
	struct GT_object *glyph = 0;
	struct GT_pointset *pointset;
	Triple *points;

	ENTER(create_GT_object_point);
	if (name)
	{
		pointset=(struct GT_pointset *)NULL;
		if (ALLOCATE(points,Triple,1))
		{
			(*points)[0]=0.0;
			(*points)[1]=0.0;
			(*points)[2]=0.0;
			if (!(pointset=CREATE(GT_pointset)(1,points,(char **)NULL,marker_type,
				marker_size,g_NO_DATA,(GLfloat *)NULL,(int *)NULL, (struct Cmiss_font *)NULL)))
			{
				DEALLOCATE(points);
			}
		}
		if (pointset)
		{
			glyph=CREATE(GT_object)(name,g_POINTSET, (struct Graphical_material *)NULL);
			if (!GT_OBJECT_ADD(GT_pointset)(glyph,/*time*/0.0,pointset))
			{
				DEACCESS(GT_object)(&glyph);
				DESTROY(GT_pointset)(&pointset);
			}
		}
		if (!glyph)
		{
			display_message(ERROR_MESSAGE,"create_GT_object_point.  Error creating glyph");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"create_GT_object_point.  Invalid argument(s)");
	}
	LEAVE;

	return (glyph);
}

/**
 * Creates a graphics object named <name> resembling a square sheet spanning from
 * coordinate <-0.5,-0.5,0> to <0.5,0.5,0>.
 * If define_texturepoints is true then texture coordinates will also be defined
 * ranging from <0.0,0.0,0.0> to <1.0,1.0,0.0>.
 */
struct GT_object *create_GT_object_sheet(const char *name, int define_texturepoints)
{
	struct GT_object *glyph = 0;
	struct GT_surface *surface;
	Triple *point,*points,*normalpoints, *texturepoints;

	ENTER(create_GT_object_sheet);
	if (name)
	{
		surface=(struct GT_surface *)NULL;
		texturepoints = (Triple *)NULL;
		if (ALLOCATE(points,Triple,4)&&
			ALLOCATE(normalpoints,Triple,4)&&
			(!define_texturepoints || ALLOCATE(texturepoints,Triple,4)))
		{
			point = points;
			/* vertices */
			(*point)[0] = -0.5;
			(*point)[1] = -0.5;
			(*point)[2] = 0.0;
			point++;
			(*point)[0] = 0.5;
			(*point)[1] = -0.5;
			(*point)[2] = 0.0;
			point++;
			(*point)[0] = 0.5;
			(*point)[1] = 0.5;
			(*point)[2] = 0.0;
			point++;
			(*point)[0] = -0.5;
			(*point)[1] = 0.5;
			(*point)[2] = 0.0;
			/* normals */
			point = normalpoints;
			(*point)[0] = 0.0;
			(*point)[1] = 0.0;
			(*point)[2] = 1.0;
			point++;
			(*point)[0] = 0.0;
			(*point)[1] = 0.0;
			(*point)[2] = 1.0;
			point++;
			(*point)[0] = 0.0;
			(*point)[1] = 0.0;
			(*point)[2] = 1.0;
			point++;
			(*point)[0] = 0.0;
			(*point)[1] = 0.0;
			(*point)[2] = 1.0;
			point++;
			/* texture coordinates */
			if (define_texturepoints)
			{
				point = texturepoints;
				(*point)[0] = 0.0;
				(*point)[1] = 0.0;
				(*point)[2] = 0.0;
				point++;
				(*point)[0] = 1.0;
				(*point)[1] = 0.0;
				(*point)[2] = 0.0;
				point++;
				(*point)[0] = 1.0;
				(*point)[1] = 1.0;
				(*point)[2] = 0.0;
				point++;
				(*point)[0] = 0.0;
				(*point)[1] = 1.0;
				(*point)[2] = 0.0;
				point++;
			}
			if (!(surface=CREATE(GT_surface)(g_SH_DISCONTINUOUS_TEXMAP,CMISS_GRAPHIC_RENDER_POLYGON_SHADED,
				g_QUADRILATERAL,1,4,points,normalpoints,/*tangentpoints*/(Triple *)NULL,
				texturepoints,g_NO_DATA,(GLfloat *)NULL)))
			{
				DEALLOCATE(points);
				DEALLOCATE(normalpoints);
			}
		}
		if (surface)
		{
			glyph=CREATE(GT_object)(name,g_SURFACE,	(struct Graphical_material *)NULL);
			if (!GT_OBJECT_ADD(GT_surface)(glyph,/*time*/0.0,surface))
			{
				DEACCESS(GT_object)(&glyph);
				DESTROY(GT_surface)(&surface);
			}
		}
		if (!glyph)
		{
			display_message(ERROR_MESSAGE,
				"create_GT_object_sheet.  Error creating glyph");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"create_GT_object_sheet.  Invalid argument(s)");
	}
	LEAVE;

	return (glyph);
}

Cmiss_glyph_id Cmiss_glyph_access(Cmiss_glyph_id glyph)
{
	if (glyph)
		glyph->access();
	return glyph;
}

int Cmiss_glyph_destroy(Cmiss_glyph_id *glyph_address)
{
	if (glyph_address)
	{
		Cmiss_glyph::deaccess(glyph_address);
		return CMISS_OK;
	}
	return CMISS_ERROR_ARGUMENT;
}

char *Cmiss_glyph_get_name(Cmiss_glyph_id glyph)
{
	if (glyph)
		return duplicate_string(glyph->getName());
	return 0;
}

int Cmiss_glyph_set_name(Cmiss_glyph_id glyph, const char *name)
{
	if (glyph && name)
		return glyph->setName(name);
	return CMISS_ERROR_ARGUMENT;
}

bool Cmiss_glyph_is_managed(Cmiss_glyph_id glyph)
{
	if (glyph)
		return glyph->isManaged();
	return 0;
}

int Cmiss_glyph_set_managed(Cmiss_glyph_id glyph, bool value)
{
	if (glyph)
	{
		glyph->setManaged(value);
		return CMISS_OK;
	}
	return CMISS_ERROR_ARGUMENT;
}

Cmiss_glyph_module::Cmiss_glyph_module(Cmiss_graphics_material_module *materialModuleIn) :
	materialModule(Cmiss_graphics_material_module_access(materialModuleIn)),
	manager(CREATE(MANAGER(Cmiss_glyph))()),
	defaultPointGlyph(0),
	access_count(1)
{
}

Cmiss_glyph_module::~Cmiss_glyph_module()
{
	Cmiss_graphics_material_module_destroy(&materialModule);
	if (defaultPointGlyph)
	{
		DEACCESS(Cmiss_glyph)(&(this->defaultPointGlyph));
	}
	DESTROY(MANAGER(Cmiss_glyph))(&(this->manager));
}

/**
  * If name is not already used by an existing glyph, set name, managed flag
	* and add to glyph module.
	* Always free handle to glyph.
	*/
void Cmiss_glyph_module::defineGlyph(const char *name, Cmiss_glyph *glyph, Cmiss_glyph_type type)
{
	if (0 == FIND_BY_IDENTIFIER_IN_MANAGER(Cmiss_glyph,name)(name, this->getManager()))
	{
		glyph->setName(name);
		glyph->setManaged(true);
		glyph->setType(type);
		this->addGlyph(glyph);
	}
	Cmiss_glyph_destroy(&glyph);
}

/**
 * If there is no glyph with the same name as graphicsObject, create a
 * static glyph of that name wrapping it and manage it.
 * Always deaccesses the supplied graphics object to clean it up whether
 * wrapping it or not.
 * @return  true if glyph added, false if not.
 */
bool Cmiss_glyph_module::defineGlyphStatic(GT_object*& graphicsObject, Cmiss_glyph_type type)
{
	bool glyphAdded = true;
	char *name = 0;
	GET_NAME(GT_object)(graphicsObject, &name);
	if (0 == FIND_BY_IDENTIFIER_IN_MANAGER(Cmiss_glyph,name)(name, this->getManager()))
	{
		GT_object_set_glyph_type(graphicsObject, type);
		Cmiss_glyph *glyph = Cmiss_glyph_static::create(graphicsObject);
		glyph->setType(type);
		glyph->setName(name);
		glyph->setManaged(true);
		this->addGlyph(glyph);
		Cmiss_glyph::deaccess(&glyph);
	}
	else
	{
		glyphAdded = false;
	}
	DEACCESS(GT_object)(&graphicsObject);
	DEALLOCATE(name);
	return glyphAdded;
}

Cmiss_glyph *Cmiss_glyph_module::findGlyphByType(enum Cmiss_glyph_type glyph_type)
{
	Cmiss_set_Cmiss_glyph *glyphs = this->getGlyphListPrivate();
	for (Cmiss_set_Cmiss_glyph::iterator iter = glyphs->begin(); iter != glyphs->end(); ++iter)
	{
		if ((*iter)->getType() == glyph_type)
		{
			return *iter;
		}
	}
	return 0;
}

void Cmiss_glyph_module::addGlyph(Cmiss_glyph *glyph)
{
	if (glyph->manager)
	{
		display_message(ERROR_MESSAGE, "Cmiss_glyph_module::addGlyph.  Glyph already managed");
		return;
	}
	if ((0 == glyph->getName()) || (static_cast<Cmiss_glyph*>(0) != 
		FIND_BY_IDENTIFIER_IN_MANAGER(Cmiss_glyph,name)(glyph->getName(), this->manager)))
	{
		char tempName[20];
		int i = NUMBER_IN_MANAGER(Cmiss_glyph)(this->manager);
		do
		{
			i++;
			sprintf(tempName, "temp%d",i);
		}
		while (FIND_BY_IDENTIFIER_IN_MANAGER(Cmiss_glyph,name)(tempName, this->manager));
		glyph->setName(tempName);
	}
	ADD_OBJECT_TO_MANAGER(Cmiss_glyph)(glyph, this->manager);
}

int Cmiss_glyph_module::defineStandardGlyphs()
{
	beginChange();
	GT_object *graphicsObject = 0;
	Cmiss_glyph_id glyph = 0;
	Cmiss_glyph_axes_id axes = 0;

	graphicsObject = create_GT_object_arrow_line("arrow", 1.f/3.f, 0.5);
	this->defineGlyphStatic(graphicsObject, CMISS_GLYPH_ARROW);

	this->defineGlyph("arrow_solid", Cmiss_glyph_arrow_solid::create(1.0/3.0, 1.0/3.0), CMISS_GLYPH_ARROW_SOLID);

	graphicsObject = create_GT_object_arrow_line("axis", 0.1, 0.5);
	this->defineGlyphStatic(graphicsObject, CMISS_GLYPH_AXIS);

	this->defineGlyph("axis_solid", Cmiss_glyph_arrow_solid::create(0.1, 1.0/3.0), CMISS_GLYPH_AXIS_SOLID);

	this->defineGlyph("cone", Cmiss_glyph_cone::create(), CMISS_GLYPH_CONE);
	this->defineGlyph("cone_solid", Cmiss_glyph_cone_solid::create(), CMISS_GLYPH_CONE_SOLID);

	graphicsObject = create_GT_object_cross("cross");
	this->defineGlyphStatic(graphicsObject, CMISS_GLYPH_CROSS);

	graphicsObject = create_GT_object_cube_solid("cube_solid");
	this->defineGlyphStatic(graphicsObject, CMISS_GLYPH_CUBE_SOLID);

	graphicsObject = create_GT_object_cube_wireframe("cube_wireframe");
	this->defineGlyphStatic(graphicsObject, CMISS_GLYPH_CUBE_WIREFRAME);

	this->defineGlyph("cylinder", Cmiss_glyph_cylinder::create(), CMISS_GLYPH_CYLINDER);
	this->defineGlyph("cylinder_solid", Cmiss_glyph_cylinder_solid::create(), CMISS_GLYPH_CYLINDER_SOLID);

	graphicsObject = create_GT_object_sphere("diamond", 4, 2);
	this->defineGlyphStatic(graphicsObject, CMISS_GLYPH_DIAMOND);

	graphicsObject = create_GT_object_line("line");
	this->defineGlyphStatic(graphicsObject, CMISS_GLYPH_LINE);

	graphicsObject = create_GT_object_point("point", g_POINT_MARKER, 0);
	if (this->defineGlyphStatic(graphicsObject, CMISS_GLYPH_POINT) && (!this->defaultPointGlyph))
	{
		setDefaultPointGlyph(this->findGlyphByName("point"));
	}

	graphicsObject = create_GT_object_sheet("sheet", /*define_texturepoints*/0);
	this->defineGlyphStatic(graphicsObject, CMISS_GLYPH_SHEET);

	this->defineGlyph("sphere", Cmiss_glyph_sphere::create(), CMISS_GLYPH_SPHERE);

	Cmiss_glyph_id axis = this->findGlyphByName("axis");
	axes = Cmiss_glyph_axes::create(axis, /*axis_width*/0.1);
	this->defineGlyph("axes", axes, CMISS_GLYPH_AXES);
	axes = Cmiss_glyph_axes::create(axis, /*axis_width*/0.1);
	axes->setAxisLabel(1, "1");
	axes->setAxisLabel(2, "2");
	axes->setAxisLabel(3, "3");
	this->defineGlyph("axes_123", axes, CMISS_GLYPH_AXES_123);
	axes = Cmiss_glyph_axes::create(axis, /*axis_width*/0.1);
	axes->setAxisLabel(1, "x");
	axes->setAxisLabel(2, "y");
	axes->setAxisLabel(3, "z");
	this->defineGlyph("axes_xyz", axes, CMISS_GLYPH_AXES_XYZ);

	Cmiss_glyph_id arrow_solid = this->findGlyphByName("arrow_solid");
	axes = Cmiss_glyph_axes::create(arrow_solid, /*axis_width*/0.25);
	this->defineGlyph("axes_solid", axes, CMISS_GLYPH_AXES_SOLID);
	axes = Cmiss_glyph_axes::create(arrow_solid, /*axis_width*/0.25);
	axes->setAxisLabel(1, "1");
	axes->setAxisLabel(2, "2");
	axes->setAxisLabel(3, "3");
	this->defineGlyph("axes_solid_123", axes, CMISS_GLYPH_AXES_SOLID_123);
	axes = Cmiss_glyph_axes::create(arrow_solid, /*axis_width*/0.25);
	axes->setAxisLabel(1, "x");
	axes->setAxisLabel(2, "y");
	axes->setAxisLabel(3, "z");
	this->defineGlyph("axes_solid_xyz", axes, CMISS_GLYPH_AXES_SOLID_XYZ);

	Cmiss_graphics_material_id red = Cmiss_graphics_material_module_find_material_by_name(this->materialModule, "red");
	Cmiss_graphics_material_id green = Cmiss_graphics_material_module_find_material_by_name(this->materialModule, "green");
	Cmiss_graphics_material_id blue = Cmiss_graphics_material_module_find_material_by_name(this->materialModule, "blue");
	if (red && green && blue)
	{
		axes = Cmiss_glyph_axes::create(axis, /*axis_width*/0.1);
		axes->setAxisMaterial(1, red);
		axes->setAxisMaterial(2, green);
		axes->setAxisMaterial(3, blue);
		this->defineGlyph("axes_colour", axes, CMISS_GLYPH_AXES_COLOUR);

		axes = Cmiss_glyph_axes::create(arrow_solid, /*axis_width*/0.25);
		axes->setAxisMaterial(1, red);
		axes->setAxisMaterial(2, green);
		axes->setAxisMaterial(3, blue);
		this->defineGlyph("axes_solid_colour", axes, CMISS_GLYPH_AXES_SOLID_COLOUR);
	}
	Cmiss_graphics_material_destroy(&red);
	Cmiss_graphics_material_destroy(&green);
	Cmiss_graphics_material_destroy(&blue);

	endChange();

	return CMISS_OK;
}

int Cmiss_glyph_module::defineStandardCmguiGlyphs()
{
	beginChange();
	GT_object *graphicsObject = 0;

	Cmiss_glyph_id axis = this->findGlyphByName("axis");
	Cmiss_glyph_axes_id axes = Cmiss_glyph_axes::create(axis, /*axis_width*/0.1);
	if (axes)
	{
		axes->setAxisLabel(1, "f");
		axes->setAxisLabel(2, "s");
		axes->setAxisLabel(3, "n");
		this->defineGlyph("axes_fsn", axes, CMISS_GLYPH_TYPE_INVALID);
	}

	graphicsObject = create_GT_object_axes("grid_lines",
		/*make_solid*/0, /*head_length*/0.0, /*half_head_width*/0.0,
		/*labels*/(const char **)NULL, /*label_offset*/0.1f, (Cmiss_font*)0);
	Graphics_object_set_glyph_labels_function(graphicsObject, draw_glyph_grid_lines);
	this->defineGlyphStatic(graphicsObject, CMISS_GLYPH_TYPE_INVALID);

	graphicsObject = create_GT_object_line("line_ticks");
	Graphics_object_set_glyph_labels_function(graphicsObject, draw_glyph_axes_ticks);
	this->defineGlyphStatic(graphicsObject, CMISS_GLYPH_TYPE_INVALID);

	graphicsObject = create_GT_object_sheet("textured_sheet", /*define_texturepoints*/1);
	this->defineGlyphStatic(graphicsObject, CMISS_GLYPH_TYPE_INVALID);

	endChange();
	return CMISS_OK;
}

Cmiss_set_Cmiss_glyph *Cmiss_glyph_module::getGlyphListPrivate()
{
	return reinterpret_cast<Cmiss_set_Cmiss_glyph *>(this->manager->object_list);
}

Cmiss_glyph_module_id Cmiss_glyph_module_create(Cmiss_graphics_material_module *materialModule)
{
	return Cmiss_glyph_module::create(materialModule);
}

struct MANAGER(Cmiss_glyph) *Cmiss_glyph_module_get_manager(
	Cmiss_glyph_module_id glyph_module)
{
	if (glyph_module)
		return glyph_module->getManager();
	return 0;
}

Cmiss_glyph_module_id Cmiss_glyph_module_access(
	Cmiss_glyph_module_id glyph_module)
{
	if (glyph_module)
		return glyph_module->access();
	return 0;
}

int Cmiss_glyph_module_destroy(Cmiss_glyph_module_id *glyph_module_address)
{
	if (glyph_module_address)
	{
		Cmiss_glyph_module::deaccess(*glyph_module_address);
		return CMISS_OK;
	}
	return CMISS_ERROR_ARGUMENT;
}

int Cmiss_glyph_module_begin_change(Cmiss_glyph_module_id glyph_module)
{
	if (glyph_module)
		return glyph_module->beginChange();
	return CMISS_ERROR_ARGUMENT;
}

int Cmiss_glyph_module_end_change(Cmiss_glyph_module_id glyph_module)
{
	if (glyph_module)
		return glyph_module->endChange();
	return CMISS_ERROR_ARGUMENT;
}

int Cmiss_glyph_module_define_standard_glyphs(
	Cmiss_glyph_module_id glyph_module)
{
	if (glyph_module)
		return glyph_module->defineStandardGlyphs();
	return CMISS_ERROR_ARGUMENT;
}

int Cmiss_glyph_module_define_standard_cmgui_glyphs(
	Cmiss_glyph_module_id glyph_module)
{
	if (glyph_module)
		return glyph_module->defineStandardCmguiGlyphs();
	return CMISS_ERROR_ARGUMENT;
}

Cmiss_glyph_id Cmiss_glyph_module_find_glyph_by_name(
	Cmiss_glyph_module_id glyph_module, const char *name)
{
	Cmiss_glyph *glyph = 0;
	if (glyph_module)
	{
		glyph = glyph_module->findGlyphByName(name);
		if (glyph)
			glyph->access();
	}
	return glyph;
}

Cmiss_glyph_id Cmiss_glyph_module_find_glyph_by_type(
	Cmiss_glyph_module_id glyph_module, enum Cmiss_glyph_type glyph_type)
{
	Cmiss_glyph *glyph = 0;
	if (glyph_module)
	{
		glyph = glyph_module->findGlyphByType(glyph_type);
		if (glyph)
			glyph->access();
	}
	return glyph;
}

Cmiss_glyph_id Cmiss_glyph_module_get_default_point_glyph(
	Cmiss_glyph_module_id glyph_module)
{
	if (glyph_module)
		return glyph_module->getDefaultPointGlyph();
	return 0;
}

int Cmiss_glyph_module_set_default_point_glyph(
	Cmiss_glyph_module_id glyph_module, Cmiss_glyph_id glyph)
{
	if (glyph_module)
	{
		glyph_module->setDefaultPointGlyph(glyph);
		return CMISS_OK;
	}
	return CMISS_ERROR_ARGUMENT;
}

Cmiss_glyph *Cmiss_glyph_module_create_glyph_static(
	Cmiss_glyph_module_id glyphModule, GT_object *graphicsObject)
{
	if (glyphModule && graphicsObject)
	{
		Cmiss_glyph *glyph = Cmiss_glyph_static::create(graphicsObject);
		glyphModule->addGlyph(glyph);
		return glyph;
	}
	return 0;
}
