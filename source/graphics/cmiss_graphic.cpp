
/*******************************************************************************
FILE : cmiss_graphic.cpp

LAST MODIFIED : 22 October 2008

DESCRIPTION :
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
#include <string>
#if defined (BUILD_WITH_CMAKE)
#include "configure/cmgui_configure.h"
#endif /* defined (BUILD_WITH_CMAKE) */
extern "C" {
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "api/cmiss_graphic.h"
#include "api/cmiss_field_group.h"
#include "api/cmiss_field_sub_group_template.h"
#include "general/debug.h"
#include "general/enumerator_private_cpp.hpp"
#include "general/indexed_list_private.h"
#include "general/compare.h"
#include "general/multi_range.h"
#include "general/mystring.h"
#include "general/object.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_finite_element.h"
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_wrappers.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_discretization.h"
#include "finite_element/finite_element_region.h"
#include "finite_element/finite_element_to_graphics_object.h"
#include "finite_element/finite_element_to_iso_lines.h"
#include "finite_element/finite_element_to_iso_surfaces.h"
#include "finite_element/finite_element_to_streamlines.h"
#include "graphics/auxiliary_graphics_types.h"
#include "graphics/cmiss_graphic.h"
#include "graphics/font.h"
#include "graphics/graphics_object.h"
#include "graphics/scene.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"
}
#include "graphics/rendergl.hpp"
#if defined(USE_OPENCASCADE)
#	include "cad/computed_field_cad_geometry.h"
#	include "cad/computed_field_cad_topology.h"
#	include "cad/cad_geometry_to_graphics_object.h"
#endif /* defined(USE_OPENCASCADE) */

struct Cmiss_graphic
/*******************************************************************************
LAST MODIFIED : 14 March 2003

DESCRIPTION :
Stores one group of settings for a single line/surface/etc. part of the
finite element group rendition.
==============================================================================*/
{
	/* position identifier for ordering settings in list */
	int position;

	/* rendition which owns this graphic */
	struct Cmiss_rendition *rendition;
	
	/* name for identifying settings */
	const char *name;

// 	/* geometry settings */
// 	/* for all graphic types */
	enum Cmiss_graphic_type graphic_type;
	struct Computed_field *coordinate_field;
	enum Graphics_select_mode select_mode;

	/* for 1-D and 2-D elements only */
	char exterior;
	/* face number is from -1 to 5, where -1 is none/all, 0 is xi1=0, 1 is xi1=1,
		 2 is xi2=0 etc. */
	int face;
	/* For surfaces only at the moment */
	struct Computed_field *texture_coordinate_field;
	/* for cylinders only */
	/* use radius = constant_radius + scale_factor*radius_scalar_field */
	float constant_radius,radius_scale_factor;
	struct Computed_field *radius_scalar_field;
	/* for iso_surfaces only */
	struct Computed_field *iso_scalar_field;
	int number_of_iso_values;
	/* If the iso_values array is set then these values are used,
		otherwise number_of_iso_values values are distributed from 
		first_iso_value to last_iso_value including these values for n>1 */
	double *iso_values, first_iso_value, last_iso_value,
		decimation_threshold;
	/* for node_points, data_points and element_points only */
	struct GT_object *glyph;
	enum Graphic_glyph_scaling_mode glyph_scaling_mode;
	Triple glyph_centre, glyph_scale_factors, glyph_size;
	struct Computed_field *orientation_scale_field;
	struct Computed_field *variable_scale_field;
	struct Computed_field *label_field;
	struct Computed_field *label_density_field;
	struct Computed_field *visibility_field;
	/* for element_points and iso_surfaces */
	enum Use_element_type use_element_type;
	/* for element_points only */
	enum Xi_discretization_mode xi_discretization_mode;
	struct Computed_field *xi_point_density_field;
	struct FE_field *native_discretization_field;
	struct Element_discretization discretization;
	int circle_discretization;
	/* for volumes only */
	struct VT_volume_texture *volume_texture;
	/* SAB Added for text access only */
	struct Computed_field *displacement_map_field;
	int displacement_map_xi_direction;
	int overlay_flag;
	int overlay_order;
	/* for settings starting in a particular element */
	struct FE_element *seed_element;
	/* for settings requiring an exact xi location */
	Triple seed_xi;
	enum Streamline_type streamline_type;
	struct Computed_field *stream_vector_field;
	int reverse_track;
	float streamline_length, streamline_width;
	enum Streamline_data_type streamline_data_type;
	/* seed node region */
	struct FE_region *seed_node_region;
	char *seed_node_region_path; /* So that we can generate the defining string */
	/* seed element_xi field */
	struct Computed_field *seed_node_coordinate_field;

	/* appearance settings */
	/* for all graphic types */
	int visibility;
	struct Graphical_material *material, *selected_material,
		*secondary_material;
	struct Computed_field *data_field;
	struct Spectrum *spectrum;
	int autorange_spectrum_flag;
	/* for glyphsets */
	struct Graphics_font *font;
	/* for surfaces */
	enum Render_type render_type;
	/* for lines, a non zero line width overrides the default */
	int line_width;

	/* rendering information */
	/* the graphics_object generated for this settings */
	struct GT_object *graphics_object, *customised_graphics_object;
	/* flag indicating the graphics_object needs rebuilding */
	int graphics_changed;
	/* flag indicating that selected graphics have changed */
	int selected_graphics_changed;
	/* flag indicating that this settings needs to be regenerated when time
		changes */
	int time_dependent;

// 	/* for accessing objects */
	int access_count;
}; /* struct GT_element_settings */

FULL_DECLARE_INDEXED_LIST_TYPE(Cmiss_graphic);

DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Cmiss_graphic,position,int, \
	compare_int);

struct Cmiss_graphic_select_graphics_data
{
	struct FE_region *fe_region;
	struct Cmiss_graphic *graphic;
};

PROTOTYPE_ENUMERATOR_STRING_FUNCTION(Cmiss_graphic_type)
{
	const char *enumerator_string;

	ENTER(ENUMERATOR_STRING(Cmiss_graphic_type));
	switch (enumerator_value)
	{
		case CMISS_GRAPHIC_NODE_POINTS:
		{
			enumerator_string = "node_points";
		} break;
		case CMISS_GRAPHIC_DATA_POINTS:
		{
			enumerator_string = "data_points";
		} break;
		case CMISS_GRAPHIC_LINES:
		{
			enumerator_string = "lines";
		} break;
		case CMISS_GRAPHIC_CYLINDERS:
		{
			enumerator_string = "cylinders";
		} break;
		case CMISS_GRAPHIC_SURFACES:
		{
			enumerator_string = "surfaces";
		} break;
		case CMISS_GRAPHIC_ISO_SURFACES:
		{
			enumerator_string = "iso_surfaces";
		} break;
		case CMISS_GRAPHIC_ELEMENT_POINTS:
		{
			enumerator_string = "element_points";
		} break;
		case CMISS_GRAPHIC_STREAMLINES:
		{
			enumerator_string = "streamlines";
		} break;
		case CMISS_GRAPHIC_STATIC:
		{
			enumerator_string = "static_graphic";
		} break;
#if ! defined (WX_USER_INTERFACE)
		case CMISS_GRAPHIC_VOLUMES:
		{
			enumerator_string = "volumes";
		} break;
#endif /* ! defined (WX_USER_INTERFACE) */
		default:
		{
			enumerator_string = (const char *)NULL;
		} break;
	}
	LEAVE;

	return (enumerator_string);
} /* ENUMERATOR_STRING(Cmiss_graphic_type) */

DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS(Cmiss_graphic_type)

PROTOTYPE_ENUMERATOR_STRING_FUNCTION(Graphic_glyph_scaling_mode)
{
	const char *enumerator_string;

	ENTER(ENUMERATOR_STRING(Graphic_glyph_scaling_mode));
	switch (enumerator_value)
	{
		case GRAPHIC_GLYPH_SCALING_CONSTANT:
		{
			enumerator_string = "constant";
		} break;
		case GRAPHIC_GLYPH_SCALING_SCALAR:
		{
			enumerator_string = "scalar";
		} break;
		case GRAPHIC_GLYPH_SCALING_VECTOR:
		{
			enumerator_string = "vector";
		} break;
		case GRAPHIC_GLYPH_SCALING_AXES:
		{
			enumerator_string = "axes";
		} break;
		case GRAPHIC_GLYPH_SCALING_GENERAL:
		{
			enumerator_string = "general";
		} break;
		default:
		{
			enumerator_string = (const char *)NULL;
		} break;
	}
	LEAVE;

	return (enumerator_string);
} /* ENUMERATOR_STRING(Glyph_scaling_mode) */

DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS(Graphic_glyph_scaling_mode)

struct Cmiss_graphic *CREATE(Cmiss_graphic)(
	enum Cmiss_graphic_type graphic_type)
/*******************************************************************************
LAST MODIFIED : 14 March 2003

DESCRIPTION :
Allocates memory and assigns fields for a struct GT_element_settings.
==============================================================================*/
{
	struct Cmiss_graphic *graphic;

	ENTER(CREATE(Cmiss_graphic));
	if ((CMISS_GRAPHIC_NODE_POINTS==graphic_type)||
		(CMISS_GRAPHIC_DATA_POINTS==graphic_type)||
		(CMISS_GRAPHIC_LINES==graphic_type)||
		(CMISS_GRAPHIC_CYLINDERS==graphic_type)||
		(CMISS_GRAPHIC_SURFACES==graphic_type)||
		(CMISS_GRAPHIC_ISO_SURFACES==graphic_type)||
		(CMISS_GRAPHIC_ELEMENT_POINTS==graphic_type)||
		(CMISS_GRAPHIC_VOLUMES==graphic_type)||
		(CMISS_GRAPHIC_STREAMLINES==graphic_type)||
		(CMISS_GRAPHIC_STATIC==graphic_type))
	{
		if (ALLOCATE(graphic,struct Cmiss_graphic,1))
		{
			graphic->position=0;
			graphic->rendition = NULL;
			graphic->name = (char *)NULL;

			/* geometry settings defaults */
			/* for all graphic types */
			graphic->graphic_type=graphic_type;
			graphic->coordinate_field=(struct Computed_field *)NULL;
			/* For surfaces only at the moment */
			graphic->texture_coordinate_field=(struct Computed_field *)NULL;
			/* for 1-D and 2-D elements only */
			graphic->exterior=0;
			graphic->face=-1; /* any face */
			/* for cylinders only */
			graphic->constant_radius=0.0;
			graphic->radius_scale_factor=1.0;
			graphic->radius_scalar_field=(struct Computed_field *)NULL;
			/* for iso_surfaces only */
			graphic->iso_scalar_field=(struct Computed_field *)NULL;
			graphic->number_of_iso_values=0;
			graphic->iso_values=(double *)NULL;
			graphic->first_iso_value=0.0;
			graphic->last_iso_value=0.0;
			graphic->decimation_threshold = 0.0;
			/* for node_points, data_points and element_points only */
			graphic->glyph=(struct GT_object *)NULL;
			graphic->glyph_scaling_mode = GRAPHIC_GLYPH_SCALING_GENERAL;
			graphic->glyph_centre[0]=0.0;
			graphic->glyph_centre[1]=0.0;
			graphic->glyph_centre[2]=0.0;
			graphic->glyph_scale_factors[0]=1.0;
			graphic->glyph_scale_factors[1]=1.0;
			graphic->glyph_scale_factors[2]=1.0;
			graphic->glyph_size[0]=1.0;
			graphic->glyph_size[1]=1.0;
			graphic->glyph_size[2]=1.0;
			graphic->orientation_scale_field=(struct Computed_field *)NULL;
			graphic->variable_scale_field=(struct Computed_field *)NULL;
			graphic->label_field=(struct Computed_field *)NULL;
			graphic->label_density_field=(struct Computed_field *)NULL;
			graphic->visibility_field=(struct Computed_field *)NULL;
			graphic->select_mode=GRAPHICS_SELECT_ON;
			/* for element_points and iso_surfaces */
			graphic->use_element_type=USE_ELEMENTS;
			/* for element_points only */
			graphic->xi_discretization_mode=XI_DISCRETIZATION_CELL_CENTRES;
			graphic->xi_point_density_field = (struct Computed_field *)NULL;
			graphic->native_discretization_field=(struct FE_field *)NULL;
			/* default to 1*1*1 discretization for fastest possible display.
				 Important since model may have a *lot* of elements */
			graphic->discretization.number_in_xi1=1;
			graphic->discretization.number_in_xi2=1;
			graphic->discretization.number_in_xi3=1;
			graphic->circle_discretization=6;
			/* for volumes only */
			graphic->volume_texture=(struct VT_volume_texture *)NULL;
			graphic->displacement_map_field=(struct Computed_field *)NULL;
			graphic->displacement_map_xi_direction = 12;
			/* for settings starting in a particular element */
			graphic->seed_element=(struct FE_element *)NULL;
			/* for settings requiring an exact xi location */
			graphic->seed_xi[0]=0.5;
			graphic->seed_xi[1]=0.5;
			graphic->seed_xi[2]=0.5;
			/* for streamlines only */
			graphic->streamline_type=STREAM_LINE;
			graphic->stream_vector_field=(struct Computed_field *)NULL;
			graphic->reverse_track=0;
			graphic->streamline_length=1.0;
			graphic->streamline_width=1.0;
			graphic->seed_node_region = (struct FE_region *)NULL;
			graphic->seed_node_region_path = (char *)NULL;
			graphic->seed_node_coordinate_field = (struct Computed_field *)NULL;
			graphic->overlay_flag = 0;
			graphic->overlay_order = 1;
			/* appearance settings defaults */
			/* for all graphic types */
			graphic->visibility=1;
			graphic->material=(struct Graphical_material *)NULL;
			graphic->secondary_material=(struct Graphical_material *)NULL;
			graphic->selected_material=(struct Graphical_material *)NULL;
			graphic->data_field=(struct Computed_field *)NULL;
			graphic->spectrum=(struct Spectrum *)NULL;
			graphic->customised_graphics_object =(struct GT_object *)NULL;
			graphic->autorange_spectrum_flag = 0;
			/* for glyphsets */
			graphic->font = (struct Graphics_font *)NULL;
			/* for cylinders, surfaces and volumes */
			graphic->render_type = RENDER_TYPE_SHADED;
			/* for streamlines only */
			graphic->streamline_data_type=STREAM_NO_DATA;
			/* for lines */
			graphic->line_width = 0;

			/* rendering information defaults */
			graphic->graphics_object = (struct GT_object *)NULL;
			graphic->graphics_changed = 1;
			graphic->selected_graphics_changed = 0;
			graphic->time_dependent = 0;

			graphic->access_count=0;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Cmiss_graphic).  Insufficient memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Cmiss_graphic).  Invalid graphic type");
		graphic=(struct Cmiss_graphic *)NULL;
	}
	LEAVE;

	return (graphic);
} /* CREATE(Cmiss_graphic) */

int DESTROY(Cmiss_graphic)(
	struct Cmiss_graphic **cmiss_graphic_address)
{
	int return_code;
	struct Cmiss_graphic *graphic;

	ENTER(DESTROY(Cmiss_graphic));
	if (cmiss_graphic_address && (graphic= *cmiss_graphic_address))
	{
		if (graphic->name)
		{
			DEALLOCATE(graphic->name);
		}
		if (graphic->customised_graphics_object)
		{
			DEACCESS(GT_object)(&(graphic->customised_graphics_object));
		}
		if (graphic->graphics_object)
		{
			DEACCESS(GT_object)(&(graphic->graphics_object));
		}
		if (graphic->coordinate_field)
		{
			DEACCESS(Computed_field)(&(graphic->coordinate_field));
		}
		if (graphic->texture_coordinate_field)
		{
			DEACCESS(Computed_field)(&(graphic->texture_coordinate_field));
		}
		if (graphic->radius_scalar_field)
		{
			DEACCESS(Computed_field)(&(graphic->radius_scalar_field));
		}
		if (graphic->iso_scalar_field)
		{
				DEACCESS(Computed_field)(&(graphic->iso_scalar_field));
		}
		if (graphic->iso_values)
		{
			DEALLOCATE(graphic->iso_values);
		}
		if (graphic->glyph)
		{
				GT_object_remove_callback(graphic->glyph, Cmiss_graphic_glyph_change,
					(void *)graphic);
				DEACCESS(GT_object)(&(graphic->glyph));
		}
		if (graphic->orientation_scale_field)
		{
			DEACCESS(Computed_field)(&(graphic->orientation_scale_field));
		}
		if (graphic->variable_scale_field)
		{
			DEACCESS(Computed_field)(&(graphic->variable_scale_field));
		}
		if (graphic->label_field)
		{
			DEACCESS(Computed_field)(&(graphic->label_field));
		}
		if (graphic->label_density_field)
		{
			DEACCESS(Computed_field)(&(graphic->label_density_field));
		}
		if (graphic->visibility_field)
		{
			DEACCESS(Computed_field)(&(graphic->visibility_field));
		}
		if (graphic->volume_texture)
		{
			DEACCESS(VT_volume_texture)(&(graphic->volume_texture));
		}
		if (graphic->displacement_map_field)
		{
			DEACCESS(Computed_field)(&(graphic->displacement_map_field));
		}
		if (graphic->xi_point_density_field)
		{
			DEACCESS(Computed_field)(&(graphic->xi_point_density_field));
		}
		if (graphic->native_discretization_field)
		{
			DEACCESS(FE_field)(&(graphic->native_discretization_field));
		}
		if (graphic->stream_vector_field)
		{
			DEACCESS(Computed_field)(&(graphic->stream_vector_field));
		}
		/* appearance graphic */
		if (graphic->material)
		{
			DEACCESS(Graphical_material)(&(graphic->material));
		}
		if (graphic->secondary_material)
		{
			DEACCESS(Graphical_material)(&(graphic->secondary_material));
		}
		if (graphic->selected_material)
		{
			DEACCESS(Graphical_material)(&(graphic->selected_material));
		}
		if (graphic->data_field)
		{
			DEACCESS(Computed_field)(&(graphic->data_field));
		}
		if (graphic->spectrum)
		{
			DEACCESS(Spectrum)(&(graphic->spectrum));
		}
		if (graphic->font)
		{
			DEACCESS(Graphics_font)(&(graphic->font));
		}
		if (graphic->seed_element)
		{
			DEACCESS(FE_element)(&(graphic->seed_element));
		}
		if (graphic->seed_node_region)
		{
			DEACCESS(FE_region)(&(graphic->seed_node_region));
		}
		if (graphic->seed_node_region_path)
		{
			DEALLOCATE(graphic->seed_node_region_path);
		}
		if (graphic->seed_node_coordinate_field)
		{
			DEACCESS(Computed_field)(&(graphic->seed_node_coordinate_field));
		}
		DEALLOCATE(*cmiss_graphic_address);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(Cmiss_graphic_address).  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}

/***************************************************************************//** 
 * Returns the dimension of the <graphic>, which varies for some graphic types.
 * @param graphic Cmiss graphic
 * @return the dimension of the graphic
 */
int Cmiss_graphic_get_dimension(struct Cmiss_graphic *graphic)
{
	int dimension;

	ENTER(cmiss_graphic_get_dimension);
	if (graphic)
	{
		switch (graphic->graphic_type)
		{
			case CMISS_GRAPHIC_NODE_POINTS:
			case CMISS_GRAPHIC_DATA_POINTS:
			case CMISS_GRAPHIC_STATIC:
			{
				dimension=0;
			} break;
			case CMISS_GRAPHIC_LINES:
			case CMISS_GRAPHIC_CYLINDERS:
			{
				dimension=1;
			} break;
			case CMISS_GRAPHIC_SURFACES:
			{
				dimension=2;
			} break;
			case CMISS_GRAPHIC_VOLUMES:
			case CMISS_GRAPHIC_STREAMLINES:
			{
				dimension=3;
			} break;
			case CMISS_GRAPHIC_ELEMENT_POINTS:
			case CMISS_GRAPHIC_ISO_SURFACES:
			{
				dimension=Use_element_type_dimension(graphic->use_element_type);
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_graphic_get_dimension.  Unknown graphic type");
				dimension=-1;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_dimension.  Invalid argument(s)");
		dimension=0;
	}
	LEAVE;

	return (dimension);
} /* Cmiss_graphic_get_dimension */

/***************************************************************************//** 
 * Cmiss_graphic list conditional function returning 1 if the element
 * would contribute any graphics generated from the Cmiss_graphic. The
 * <fe_region> is provided to allow any checks against parent elements to
 * ensure the relevant parent elements are also in the group.
 * @param graphic The cmiss graphic
 * @param element FE_element
 * @param fe_region provided to allow any checks against parent elements
 * @return 1 if the element would contribute any graphics generated from the Cmiss_graphic
 */
static int Cmiss_graphic_uses_FE_element(
	struct Cmiss_graphic *graphic,struct FE_element *element,
	struct FE_region *fe_region)
{
	enum CM_element_type cm_element_type;
	int dimension,return_code;

	ENTER(GT_element_graphic_uses_FE_element);
	if (graphic && element && fe_region)
	{
		dimension=Cmiss_graphic_get_dimension(graphic);
		if ((CMISS_GRAPHIC_ELEMENT_POINTS==graphic->graphic_type)||
			(CMISS_GRAPHIC_ISO_SURFACES==graphic->graphic_type))
		{
			cm_element_type=Use_element_type_CM_element_type(
				graphic->use_element_type);
		}
		else if (CMISS_GRAPHIC_STREAMLINES==graphic->graphic_type)
		{
			cm_element_type=CM_ELEMENT;
		}
		else
		{
			cm_element_type=CM_ELEMENT_TYPE_INVALID;
		}
		return_code = FE_region_FE_element_meets_topological_criteria(fe_region,
			element, dimension, cm_element_type, graphic->exterior, graphic->face);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_uses_FE_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* cmiss_graphic_uses_FE_element */

/***************************************************************************//** 
 * Special version of FE_element_select_graphics for element_points, which
 * are calculated on the top_level_element in Xi_discretization_mode CELL_CORNERS.
 * @param element FE_element
 * @param selected_data_void void pointer to Cmiss_graphic_select_graphics_data
 * @return return 1 if successfully select element point
 */
static int FE_element_select_graphics_element_points(struct FE_element *element,
	void *select_data_void)

{
	int return_code;
	struct CM_element_information cm;
	struct FE_region *fe_region;
	struct Cmiss_graphic *graphic;
	struct Cmiss_graphic_select_graphics_data *select_data;

	ENTER(FE_element_select_graphics_element_points);
	if (element&&(select_data=
		(struct Cmiss_graphic_select_graphics_data *)select_data_void)&&
		(fe_region=select_data->fe_region)&&
		(graphic=select_data->graphic)&&
		(CMISS_GRAPHIC_ELEMENT_POINTS==graphic->graphic_type)&&
		graphic->graphics_object)
	{
		/* special handling for cell_corners which are always calculated on
			 top_level_elements */
		get_FE_element_identifier(element, &cm);
		if (((CM_ELEMENT == cm.type) &&
			(XI_DISCRETIZATION_CELL_CORNERS==graphic->xi_discretization_mode)) ||
			((XI_DISCRETIZATION_CELL_CORNERS!=graphic->xi_discretization_mode) &&
				Cmiss_graphic_uses_FE_element(graphic,element,fe_region)))
		{
			return_code=
				GT_object_select_graphic(select_data->graphic->graphics_object,
					CM_element_information_to_graphics_name(&cm),
					(struct Multi_range *)NULL);
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_select_graphics_element_points.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_select_graphics_element_points */

/***************************************************************************//** 
 * Converts a finite element into a graphics object with the supplied graphic.
 * @param graphic The cmiss graphic
 * @param element FE_element
 * @param fe_region provided to allow any checks against parent elements
 * @return return 1 if the element would contribute any graphics generated from the Cmiss_graphic
 */
static int FE_element_to_graphics_object(struct FE_element *element,
	void *graphic_to_object_data_void)
{
	FE_value base_size[3], centre[3], initial_xi[3], scale_factors[3];
	float time;
	int draw_element, draw_selected, element_dimension, element_graphics_name,
		element_selected, i, name_selected,
		number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS],
		number_of_xi_points, return_code,
		top_level_number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS],
		*top_level_xi_point_numbers,
		use_element_dimension, *use_number_in_xi;
	struct CM_element_information cm;
	struct Element_point_ranges *element_point_ranges;
	struct Element_point_ranges_identifier element_point_ranges_identifier;
	struct FE_element *top_level_element,*use_element;
	struct FE_field *native_discretization_field;
	struct Cmiss_graphic *graphic;
	struct Cmiss_graphic_to_graphics_object_data *graphic_to_object_data;
	struct GT_glyph_set *glyph_set;
	struct GT_polyline *polyline;
	struct GT_surface *surface;
	struct GT_voltex *voltex;
	struct Multi_range *ranges;
	FE_value_triple *xi_points;

	ENTER(FE_element_to_graphics_object);
	if (element && (graphic_to_object_data =
		(struct Cmiss_graphic_to_graphics_object_data *)
			graphic_to_object_data_void) &&
		(graphic = graphic_to_object_data->graphic) &&
		graphic->graphics_object)
	{
		return_code = 1;
		get_FE_element_identifier(element, &cm);
		element_graphics_name = CM_element_information_to_graphics_name(&cm);
		/* proceed only if graphic uses this element */
		if (Cmiss_graphic_uses_FE_element(graphic, element,
			graphic_to_object_data->fe_region))
		{
			if ((GRAPHICS_DRAW_SELECTED == graphic->select_mode) ||
				(GRAPHICS_DRAW_UNSELECTED == graphic->select_mode))
			{
				draw_selected = (GRAPHICS_DRAW_SELECTED==graphic->select_mode);
				name_selected = 0;
				if (graphic_to_object_data->group_field)
				{
					Cmiss_field_group_id group_id = Cmiss_field_cast_group(graphic_to_object_data->group_field);
					if (group_id)
					{
						Cmiss_field_id temporary_handle = NULL;
						Cmiss_field_id element_group_field =
							Cmiss_field_group_get_element_group(group_id);
						if (element_group_field)
						{
							Cmiss_field_element_group_template_id element_group =
								Cmiss_field_cast_element_group_template(element_group_field);
							Cmiss_field_destroy(&element_group_field);
							if (element_group)
							{
								name_selected =
									Cmiss_field_element_group_template_is_element_selected(element_group, element);
								temporary_handle = reinterpret_cast<Computed_field *>(element_group);
								Cmiss_field_destroy(&temporary_handle);
							}
						}
						temporary_handle = reinterpret_cast<Computed_field *>(group_id);
			      Cmiss_field_destroy(&temporary_handle);
					}
				}
				draw_element = ((draw_selected && name_selected) ||
					((!draw_selected) && (!name_selected)));
			}
			else
			{
				draw_element = 1;
			}
			/* determine discretization of element for graphic */
			top_level_element = (struct FE_element *)NULL;
			top_level_number_in_xi[0] = graphic->discretization.number_in_xi1;
			top_level_number_in_xi[1] = graphic->discretization.number_in_xi2;
			top_level_number_in_xi[2] = graphic->discretization.number_in_xi3;
			native_discretization_field = graphic->native_discretization_field;
			if (FE_region_get_FE_element_discretization(
				graphic_to_object_data->fe_region, element, graphic->face,
				native_discretization_field, top_level_number_in_xi, &top_level_element,
				number_in_xi))
			{
				element_dimension = get_FE_element_dimension(element);
				/* g_element renditions use only one time = 0.0. Must take care. */
				time = 0.0;
				switch (graphic->graphic_type)
				{
					case CMISS_GRAPHIC_LINES:
					{
						if (graphic_to_object_data->existing_graphics)
						{
							/* So far ignore these */
						}
						if (draw_element)
						{
							return_code =
								FE_element_add_line_to_vertex_array(element,
								GT_object_get_vertex_set(graphic->graphics_object),
								graphic_to_object_data->rc_coordinate_field,
								graphic->data_field,
								graphic_to_object_data->number_of_data_values,
								graphic_to_object_data->data_copy_buffer,
								graphic->texture_coordinate_field,
								number_in_xi[0], top_level_element,
								graphic_to_object_data->time);
						}
					} break;
					case CMISS_GRAPHIC_CYLINDERS:
					{
						if (graphic_to_object_data->existing_graphics)
						{
							surface = GT_OBJECT_EXTRACT_FIRST_PRIMITIVES_AT_TIME(GT_surface)
								(graphic_to_object_data->existing_graphics, time,
									element_graphics_name);
						}
						else
						{
							surface = (struct GT_surface *)NULL;
						}
						if (draw_element)
						{
							if (surface ||
								(surface = create_cylinder_from_FE_element(element,
									graphic_to_object_data->rc_coordinate_field,
									graphic->data_field, graphic->constant_radius,
									graphic->radius_scale_factor, graphic->radius_scalar_field,
									number_in_xi[0],
									graphic->circle_discretization,
									graphic->texture_coordinate_field,
									top_level_element, graphic->render_type, 
									graphic_to_object_data->time)))
							{
								if (!GT_OBJECT_ADD(GT_surface)(
									graphic->graphics_object, time, surface))
								{
									DESTROY(GT_surface)(&surface);
									return_code = 0;
								}
							}
							else
							{
								return_code = 0;
							}
						}
						else
						{
							if (surface)
							{
								DESTROY(GT_surface)(&surface);
							}
						}
					} break;
					case CMISS_GRAPHIC_SURFACES:
					{
						if (graphic_to_object_data->existing_graphics)
						{
							surface = GT_OBJECT_EXTRACT_FIRST_PRIMITIVES_AT_TIME(GT_surface)
								(graphic_to_object_data->existing_graphics, time,
									element_graphics_name);
						}
						else
						{
							surface = (struct GT_surface *)NULL;
						}
						if (draw_element)
						{
							if (surface ||
								(surface = create_GT_surface_from_FE_element(element,
									graphic_to_object_data->rc_coordinate_field,
									graphic->texture_coordinate_field, graphic->data_field,
									number_in_xi[0], number_in_xi[1],
									/*reverse_normals*/0, top_level_element,graphic->render_type,
									graphic_to_object_data->time)))
							{
								if (!GT_OBJECT_ADD(GT_surface)(
									graphic->graphics_object, time, surface))
								{
									DESTROY(GT_surface)(&surface);
									return_code = 0;
								}
							}
							else
							{
								return_code = 0;
							}
						}
						else
						{
							if (surface)
							{
								DESTROY(GT_surface)(&surface);
							}
						}
					} break;
					case CMISS_GRAPHIC_ISO_SURFACES:
					{
						switch (GT_object_get_type(graphic->graphics_object))
						{
							case g_VOLTEX:
							{
								if (3 == element_dimension)
								{
									if (graphic_to_object_data->existing_graphics)
									{
										voltex =
											GT_OBJECT_EXTRACT_FIRST_PRIMITIVES_AT_TIME(GT_voltex)
											(graphic_to_object_data->existing_graphics, time,
												element_graphics_name);
									}
									else
									{
										voltex = (struct GT_voltex *)NULL;
									}
									if (draw_element)
									{
										if (voltex)
										{
											if (!GT_OBJECT_ADD(GT_voltex)(
												graphic->graphics_object, time, voltex))
											{
												DESTROY(GT_voltex)(&voltex);
												return_code = 0;
											}
										}
										else
										{
											if (graphic->iso_values)
											{
												for (i = 0 ; i < graphic->number_of_iso_values ; i++)
												{
													return_code = create_iso_surfaces_from_FE_element(element,
														graphic->iso_values[i],
														graphic_to_object_data->time,
														(struct Clipping *)NULL,
														graphic_to_object_data->rc_coordinate_field,
														graphic->data_field, graphic->iso_scalar_field,
														graphic->texture_coordinate_field,
														number_in_xi, graphic->decimation_threshold,
														graphic->graphics_object, graphic->render_type);
												}
											}
											else
											{
												double iso_value_range;
												if (graphic->number_of_iso_values > 1)
												{
													iso_value_range =
														(graphic->last_iso_value - graphic->first_iso_value)
														/ (double)(graphic->number_of_iso_values - 1);
												}
												else
												{
													iso_value_range = 0;
												}
												for (i = 0 ; i < graphic->number_of_iso_values ; i++)
												{
													double iso_value = 
														graphic->first_iso_value +
														(double)i * iso_value_range;
													return_code = create_iso_surfaces_from_FE_element(element,
														iso_value,
														graphic_to_object_data->time,
														(struct Clipping *)NULL,
														graphic_to_object_data->rc_coordinate_field,
														graphic->data_field, graphic->iso_scalar_field,
														graphic->texture_coordinate_field,
														number_in_xi, graphic->decimation_threshold,
														graphic->graphics_object, graphic->render_type);
												}
											}
										}
									}
									else
									{
										if (voltex)
										{
											DESTROY(GT_voltex)(&voltex);
										}
									}
								}
							} break;
							case g_SURFACE:
							{
								if (3 == element_dimension)
								{
									if (graphic_to_object_data->existing_graphics)
									{
										surface = GT_OBJECT_EXTRACT_FIRST_PRIMITIVES_AT_TIME(GT_surface)
											(graphic_to_object_data->existing_graphics, time,
												element_graphics_name);
									}
									else
									{
										surface = (struct GT_surface *)NULL;
									}
									if (draw_element)
									{
										if (NULL != surface)
										{
											if (!GT_OBJECT_ADD(GT_surface)(
												graphic->graphics_object, time, surface))
											{
												DESTROY(GT_surface)(&surface);
												return_code = 0;
											}
										}
										else
										{
											return_code = create_iso_surfaces_from_FE_element_new(element,
												graphic_to_object_data->time, number_in_xi,
												graphic_to_object_data->iso_surface_specification,
												graphic->graphics_object,
												graphic->render_type);
										}
									}
									else
									{
										if (surface)
										{
											DESTROY(GT_surface)(&surface);
										}
									}
								}
							} break;
							case g_POLYLINE:
							{
								if (2 == element_dimension)
								{
									if (graphic_to_object_data->existing_graphics)
									{
										polyline =
											GT_OBJECT_EXTRACT_FIRST_PRIMITIVES_AT_TIME(GT_polyline)
											(graphic_to_object_data->existing_graphics, time,
												element_graphics_name);
									}
									else
									{
										polyline = (struct GT_polyline *)NULL;
									}
									if (draw_element)
									{
										if (polyline)
										{
											if (!GT_OBJECT_ADD(GT_polyline)(
												graphic->graphics_object, time, polyline))
											{
												DESTROY(GT_polyline)(&polyline);
												return_code = 0;
											}
										}
										else
										{
											if (graphic->iso_values)
											{
												for (i = 0 ; i < graphic->number_of_iso_values ; i++)
												{
													return_code = create_iso_lines_from_FE_element(element,
														graphic_to_object_data->rc_coordinate_field,
														graphic->iso_scalar_field, graphic->iso_values[i],
														graphic_to_object_data->time,
														graphic->data_field, number_in_xi[0], number_in_xi[1],
														top_level_element, graphic->graphics_object, 
														/*graphics_object_time*/0.0, graphic->line_width);
												}
											}
											else
											{
												double iso_value_range;
												if (graphic->number_of_iso_values > 1)
												{
													iso_value_range =
														(graphic->last_iso_value - graphic->first_iso_value)
														/ (double)(graphic->number_of_iso_values - 1);
												}
												else
												{
													iso_value_range = 0;
												}
												for (i = 0 ; i < graphic->number_of_iso_values ; i++)
												{
													double iso_value = 
														graphic->first_iso_value +
														(double)i * iso_value_range;
													return_code = create_iso_lines_from_FE_element(element,
														graphic_to_object_data->rc_coordinate_field,
														graphic->iso_scalar_field, iso_value,
														graphic_to_object_data->time,
														graphic->data_field, number_in_xi[0], number_in_xi[1],
														top_level_element, graphic->graphics_object, 
														/*graphics_object_time*/0.0,graphic->line_width);
												}
											}
										}
									}
									else
									{
										if (polyline)
										{
											DESTROY(GT_polyline)(&polyline);
										}
									}
								}
							} break;
							default:
							{
								display_message(ERROR_MESSAGE,"FE_element_to_graphics_object.  "
									"Invalid graphic type for iso_scalar");
								return_code = 0;
							} break;
						}
					} break;
					case CMISS_GRAPHIC_ELEMENT_POINTS:
					{
						glyph_set = (struct GT_glyph_set *)NULL;
						if (graphic_to_object_data->existing_graphics)
						{
							glyph_set =
								GT_OBJECT_EXTRACT_FIRST_PRIMITIVES_AT_TIME(GT_glyph_set)(
									graphic_to_object_data->existing_graphics, time,
									element_graphics_name);
						}
						if (!glyph_set)
						{
							for (i = 0; i < 3; i++)
							{
								element_point_ranges_identifier.exact_xi[i] =
									graphic->seed_xi[i];
							}
							if (FE_element_get_xi_points(element,
								graphic->xi_discretization_mode, number_in_xi,
								element_point_ranges_identifier.exact_xi,
								graphic_to_object_data->rc_coordinate_field,
								graphic->xi_point_density_field,
								&number_of_xi_points, &xi_points,
								graphic_to_object_data->time))
							{
								get_FE_element_identifier(element, &cm);
								element_graphics_name =
									CM_element_information_to_graphics_name(&cm);
								top_level_xi_point_numbers = (int *)NULL;
								if (XI_DISCRETIZATION_CELL_CORNERS ==
									graphic->xi_discretization_mode)
								{
									FE_element_convert_xi_points_cell_corners_to_top_level(
										element, top_level_element, top_level_number_in_xi,
										number_of_xi_points, xi_points, &top_level_xi_point_numbers);
								}
								if (top_level_xi_point_numbers)
								{
									/* xi_points have been converted to top-level */
									use_element = top_level_element;
									use_number_in_xi = top_level_number_in_xi;
								}
								else
								{
									use_element = element;
									use_number_in_xi = number_in_xi;
								}
								ranges = (struct Multi_range *)NULL;
								element_point_ranges_identifier.element = use_element;
								element_point_ranges_identifier.top_level_element= 
									top_level_element;
								element_point_ranges_identifier.xi_discretization_mode =
									graphic->xi_discretization_mode;
								use_element_dimension = get_FE_element_dimension(use_element);
								for (i = 0; i < use_element_dimension; i++)
								{
									element_point_ranges_identifier.number_in_xi[i] =
										use_number_in_xi[i];
								}
								if (NULL != (element_point_ranges = FIND_BY_IDENTIFIER_IN_LIST(
									Element_point_ranges, identifier)(
										&element_point_ranges_identifier,
										graphic_to_object_data->selected_element_point_ranges_list)))
								{
									ranges = Element_point_ranges_get_ranges(element_point_ranges);
								}
								element_selected = IS_OBJECT_IN_LIST(FE_element)(use_element,
									graphic_to_object_data->selected_element_list);
								base_size[0] = (FE_value)(graphic->glyph_size[0]);
								base_size[1] = (FE_value)(graphic->glyph_size[1]);
								base_size[2] = (FE_value)(graphic->glyph_size[2]);
								centre[0] = (FE_value)(graphic->glyph_centre[0]);
								centre[1] = (FE_value)(graphic->glyph_centre[1]);
								centre[2] = (FE_value)(graphic->glyph_centre[2]);
								scale_factors[0] = (FE_value)(graphic->glyph_scale_factors[0]);
								scale_factors[1] = (FE_value)(graphic->glyph_scale_factors[1]);
								scale_factors[2] = (FE_value)(graphic->glyph_scale_factors[2]);
								/* NOT an error if no glyph_set produced == empty selection */
								if ((0 < number_of_xi_points) &&
									NULL != (glyph_set = create_GT_glyph_set_from_FE_element(
										use_element, top_level_element,
										graphic_to_object_data->rc_coordinate_field,
										number_of_xi_points, xi_points,
										graphic->glyph, base_size, centre, scale_factors,
										graphic_to_object_data->wrapper_orientation_scale_field,
										graphic->variable_scale_field, graphic->data_field,
										graphic->font, graphic->label_field, graphic->select_mode,
										element_selected, ranges, top_level_xi_point_numbers,
										graphic_to_object_data->time)))
								{
									/* set auxiliary_object_name for glyph_set to
										 element_graphics_name so we can edit */
									GT_glyph_set_set_auxiliary_integer_identifier(glyph_set,
										element_graphics_name);
								}
								if (top_level_xi_point_numbers)
								{
									DEALLOCATE(top_level_xi_point_numbers);
								}
								DEALLOCATE(xi_points);
							}
							else
							{
								return_code = 0;
							}
						}
						if (glyph_set)
						{
							if (!GT_OBJECT_ADD(GT_glyph_set)(
								graphic->graphics_object,time,glyph_set))
							{
								DESTROY(GT_glyph_set)(&glyph_set);
								return_code = 0;
							}
						}
					} break;
					case CMISS_GRAPHIC_VOLUMES:
					{
						if (graphic_to_object_data->existing_graphics)
						{
							voltex = GT_OBJECT_EXTRACT_FIRST_PRIMITIVES_AT_TIME(GT_voltex)
								(graphic_to_object_data->existing_graphics, time,
									element_graphics_name);
						}
						else
						{
							voltex = (struct GT_voltex *)NULL;
						}
						if (draw_element)
						{
							/* not an error if no voltex produced. Iso-value probably just
								 out of range of voltex data */
							if (voltex ||
								(voltex = generate_clipped_GT_voltex_from_FE_element(
									(struct Clipping *)NULL,element,
									graphic_to_object_data->rc_coordinate_field,
									graphic->data_field,
									graphic->volume_texture, graphic->render_type,
									graphic->displacement_map_field,
									graphic->displacement_map_xi_direction,
									graphic->texture_coordinate_field,
									graphic_to_object_data->time)))
							{
								if (!GT_OBJECT_ADD(GT_voltex)(
									graphic->graphics_object, time, voltex))
								{
									DESTROY(GT_voltex)(&voltex);
									return_code = 0;
								}
							}
						}
						else
						{
							if (voltex)
							{
								DESTROY(GT_voltex)(&voltex);
							}
						}
					} break;
					case CMISS_GRAPHIC_STREAMLINES:
					{
						/* use local copy of seed_xi since tracking function updates it */
						initial_xi[0] = graphic->seed_xi[0];
						initial_xi[1] = graphic->seed_xi[1];
						initial_xi[2] = graphic->seed_xi[2];
						if (STREAM_LINE==graphic->streamline_type)
						{
							if (NULL != (polyline = create_GT_polyline_streamline_FE_element(element,
								initial_xi, graphic_to_object_data->rc_coordinate_field,
								graphic_to_object_data->wrapper_stream_vector_field,
								graphic->reverse_track, graphic->streamline_length,
								graphic->streamline_data_type, graphic->data_field,
										graphic_to_object_data->time,graphic_to_object_data->fe_region)))
							{
								if (!GT_OBJECT_ADD(GT_polyline)(graphic->graphics_object,
									time, polyline))
								{
									DESTROY(GT_polyline)(&polyline);
									return_code = 0;
								}
							}
							else
							{
								return_code = 0;
							}
						}
						else if ((graphic->streamline_type == STREAM_RIBBON)||
							(graphic->streamline_type == STREAM_EXTRUDED_RECTANGLE)||
							(graphic->streamline_type == STREAM_EXTRUDED_ELLIPSE)||
							(graphic->streamline_type == STREAM_EXTRUDED_CIRCLE))
						{
							if (NULL != (surface = create_GT_surface_streamribbon_FE_element(element,
										initial_xi, graphic_to_object_data->rc_coordinate_field,
										graphic_to_object_data->wrapper_stream_vector_field,
										graphic->reverse_track, graphic->streamline_length,
										graphic->streamline_width, graphic->streamline_type,
										graphic->streamline_data_type, graphic->data_field,
										graphic_to_object_data->time,graphic_to_object_data->fe_region)))
							{
								if (!GT_OBJECT_ADD(GT_surface)(graphic->graphics_object,
									time,surface))
								{
									DESTROY(GT_surface)(&surface);
									return_code = 0;
								}
							}
							else
							{
								return_code = 0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"FE_element_to_graphics_object.  Unknown streamline type");
							return_code = 0;
						}
					} break;
					default:
					{
						display_message(ERROR_MESSAGE,"FE_element_to_graphics_object.  "
							"Unknown element graphic type");
						return_code = 0;
					} break;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"FE_element_to_graphics_object.  Could not get discretization");
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"FE_element_to_graphics_object.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_to_graphics_object */
 
/***************************************************************************//** 
 * Creates a node point seeded with the seed_node_coordinate_field at the node.
 * @param node The FE node
 * @param selected_data_void void pointer to Cmiss_graphic_select_graphics_data
 * @return 1 if successfully add strealine
==============================================================================*/
static int Cmiss_node_to_streamline(struct FE_node *node,
	void *graphic_to_object_data_void)
{
	FE_value coordinates[3], xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	int element_dimension, number_of_components, return_code = 1;
	struct FE_element *element;
	struct Cmiss_graphic *graphic;
	struct Cmiss_graphic_to_graphics_object_data *graphic_to_object_data;
	struct GT_polyline *polyline;
	struct GT_surface *surface;

	ENTER(node_to_streamline);

	if (node&&(graphic_to_object_data =
		(struct Cmiss_graphic_to_graphics_object_data *)
		graphic_to_object_data_void) &&
		(graphic = graphic_to_object_data->graphic) &&
		graphic->graphics_object)
	{
		element_dimension = 3;
		if ((3 >= (number_of_components = Computed_field_get_number_of_components(
			graphic->seed_node_coordinate_field)))
			&& (number_of_components == Computed_field_get_number_of_components(
			graphic_to_object_data->rc_coordinate_field)))
		{
			/* determine if the element is required */
			if (Computed_field_is_defined_at_node(graphic->seed_node_coordinate_field, node)
				&& Computed_field_evaluate_at_node(graphic->seed_node_coordinate_field, node,
					graphic_to_object_data->time, coordinates)
				&& Computed_field_find_element_xi(graphic_to_object_data->rc_coordinate_field,
					coordinates, number_of_components, /*time*/0, &element, xi,
					element_dimension, graphic_to_object_data->region,
					/*propagate_field*/0, /*find_nearest_location*/0))
			{
				if (STREAM_LINE==graphic->streamline_type)
				{
					if (NULL != (polyline=create_GT_polyline_streamline_FE_element(element,
							xi,graphic_to_object_data->rc_coordinate_field,
							graphic_to_object_data->wrapper_stream_vector_field,
							graphic->reverse_track, graphic->streamline_length,
							graphic->streamline_data_type, graphic->data_field,
								graphic_to_object_data->time,graphic_to_object_data->fe_region)))
					{
						if (!(return_code=GT_OBJECT_ADD(GT_polyline)(
									graphic->graphics_object,
									/*graphics_object_time*/0,polyline)))
						{
							DESTROY(GT_polyline)(&polyline);
						}
					}
					else
					{
						return_code=0;
					}
				}
				else if ((graphic->streamline_type == STREAM_RIBBON)||
					(graphic->streamline_type == STREAM_EXTRUDED_RECTANGLE)||
					(graphic->streamline_type == STREAM_EXTRUDED_ELLIPSE)||
					(graphic->streamline_type == STREAM_EXTRUDED_CIRCLE))
				{
					if (NULL != (surface=create_GT_surface_streamribbon_FE_element(element,
								xi,graphic_to_object_data->rc_coordinate_field,
								graphic_to_object_data->wrapper_stream_vector_field,
								graphic->reverse_track, graphic->streamline_length,
								graphic->streamline_width, graphic->streamline_type,
								graphic->streamline_data_type, graphic->data_field,
								graphic_to_object_data->time,graphic_to_object_data->fe_region)))
					{
						if (!(return_code=GT_OBJECT_ADD(GT_surface)(
									graphic->graphics_object,
									/*graphics_object_time*/0,surface)))
						{
							DESTROY(GT_surface)(&surface);
						}
					}
					else
					{
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Cmiss_node_to_streamline.  Unknown streamline type");
					return_code=0;
				}
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
			"Cmiss_node_to_streamline.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* node_to_streamline */

/***************************************************************************//** 
 * If <graphic> is of type CMISS_GRAPHIC_ELEMENT_POINTS and has a graphics
 * object, determines from the identifier of <element_point_ranges>, whether it
 * applies to the graphic, and if so selects any ranges in it in the
 * graphics_object
 * @param element_point_ranges
 * @param selected_data_void void pointer to Cmiss_graphic_select_graphics_data
 * @return return 1 if successfully select element point
 */
static int Element_point_ranges_select_in_graphics_object(
	struct Element_point_ranges *element_point_ranges,void *select_data_void)
{
	int i,return_code,top_level_number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	struct CM_element_information cm;
	struct Element_point_ranges_identifier element_point_ranges_identifier,
		temp_element_point_ranges_identifier;
	struct Cmiss_graphic_select_graphics_data *select_data;
	struct FE_element *element,*top_level_element;
	struct FE_region *fe_region;
	struct Cmiss_graphic *graphic;
	struct Multi_range *ranges;

	ENTER(Element_point_ranges_select_in_graphics_object);
	if (element_point_ranges&&(select_data=
		(struct Cmiss_graphic_select_graphics_data *)select_data_void)&&
		(fe_region=select_data->fe_region)&&
		(graphic=select_data->graphic)&&
		(CMISS_GRAPHIC_ELEMENT_POINTS==graphic->graphic_type)&&
		graphic->graphics_object)
	{
		return_code=1;
		if (Element_point_ranges_get_identifier(element_point_ranges,
			&element_point_ranges_identifier) &&
			(element = element_point_ranges_identifier.element) &&
			get_FE_element_identifier(element, &cm))
		{
			/* do not select if graphics already selected for element */
			if (!GT_object_is_graphic_selected(graphic->graphics_object,
				CM_element_information_to_graphics_name(&cm), &ranges))
			{
				/* special handling for cell_corners which are always calculated on
					 top_level_elements */
				if (((CM_ELEMENT == cm.type) &&
					(XI_DISCRETIZATION_CELL_CORNERS==graphic->xi_discretization_mode)) ||
					((XI_DISCRETIZATION_CELL_CORNERS!=graphic->xi_discretization_mode) &&
						Cmiss_graphic_uses_FE_element(graphic, element, fe_region)))
				{
					top_level_element=(struct FE_element *)NULL;
					top_level_number_in_xi[0]=graphic->discretization.number_in_xi1;
					top_level_number_in_xi[1]=graphic->discretization.number_in_xi2;
					top_level_number_in_xi[2]=graphic->discretization.number_in_xi3;
					if (FE_region_get_FE_element_discretization(fe_region, element,
						graphic->face,graphic->native_discretization_field,
						top_level_number_in_xi,&top_level_element,
						temp_element_point_ranges_identifier.number_in_xi))
					{
						temp_element_point_ranges_identifier.element=element;
						temp_element_point_ranges_identifier.top_level_element=
							top_level_element;
						temp_element_point_ranges_identifier.xi_discretization_mode=
							graphic->xi_discretization_mode;
						/*???RC temporary, hopefully */
						for (i=0;i<3;i++)
						{
							temp_element_point_ranges_identifier.exact_xi[i]=
								graphic->seed_xi[i];
						}
						if (0==compare_Element_point_ranges_identifier(
							&element_point_ranges_identifier,
							&temp_element_point_ranges_identifier))
						{
							if (NULL != (ranges=CREATE(Multi_range)()))
							{
								if (!(Multi_range_copy(ranges,Element_point_ranges_get_ranges(
									element_point_ranges))&&
									GT_object_select_graphic(graphic->graphics_object,
										CM_element_information_to_graphics_name(&cm),ranges)))
								{
									display_message(ERROR_MESSAGE,
										"Element_point_ranges_select_in_graphics_object.  "
										"Could not select ranges");
									DESTROY(Multi_range)(&ranges);
									return_code=0;
								}
							}
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Element_point_ranges_select_in_graphics_object.  "
							"Error getting discretization");
						return_code=0;
					}
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Element_point_ranges_select_in_graphics_object.  "
				"Could not get identifier");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_ranges_select_in_graphics_object.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_point_ranges_select_in_graphics_object */

int Cmiss_graphic_add_to_list(struct Cmiss_graphic *graphic,
	int position,struct LIST(Cmiss_graphic) *list_of_graphic)
{
	int last_position,return_code;
	struct Cmiss_graphic *graphic_in_way;

	ENTER(Cmiss_graphic_add_to_list);
	if (graphic&&list_of_graphic&&
		!IS_OBJECT_IN_LIST(Cmiss_graphic)(graphic,list_of_graphic))
	{
		return_code=1;
		last_position=NUMBER_IN_LIST(Cmiss_graphic)(list_of_graphic);
		if ((1>position)||(position>last_position))
		{
			/* add to end of list */
			position=last_position+1;
		}
		ACCESS(Cmiss_graphic)(graphic);
		while (return_code&&graphic)
		{
			graphic->position=position;
			/* is there already a graphic with that position? */
			if (NULL != (graphic_in_way=FIND_BY_IDENTIFIER_IN_LIST(Cmiss_graphic,
						position)(position,list_of_graphic)))
			{
				/* remove the old graphic to make way for the new */
				ACCESS(Cmiss_graphic)(graphic_in_way);
				REMOVE_OBJECT_FROM_LIST(Cmiss_graphic)(
					graphic_in_way,list_of_graphic);
			}
			if (ADD_OBJECT_TO_LIST(Cmiss_graphic)(graphic,list_of_graphic))
			{
				DEACCESS(Cmiss_graphic)(&graphic);
				/* the old, in-the-way graphic now become the new graphic */
				graphic=graphic_in_way;
				position++;
			}
			else
			{
				DEACCESS(Cmiss_graphic)(&graphic);
				if (graphic_in_way)
				{
					DEACCESS(Cmiss_graphic)(&graphic_in_way);
				}
				display_message(ERROR_MESSAGE,"Cmiss_graphic_add_to_list.  "
					"Could not add graphic - graphic lost");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_add_to_list.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}

int Cmiss_graphic_remove_from_list(struct Cmiss_graphic *graphic,
	struct LIST(Cmiss_graphic) *list_of_graphic)
{
	int return_code,next_position;

	ENTER(Cmiss_graphic_remove_from_list);
	if (graphic&&list_of_graphic)
	{
		if (IS_OBJECT_IN_LIST(Cmiss_graphic)(graphic,list_of_graphic))
		{
			next_position=graphic->position+1;
			return_code=REMOVE_OBJECT_FROM_LIST(Cmiss_graphic)(
				graphic,list_of_graphic);
			/* decrement position of all remaining graphic */
			while (return_code&&(graphic=FIND_BY_IDENTIFIER_IN_LIST(
				Cmiss_graphic,position)(next_position,list_of_graphic)))
			{
				ACCESS(Cmiss_graphic)(graphic);
				REMOVE_OBJECT_FROM_LIST(Cmiss_graphic)(graphic,list_of_graphic);
				(graphic->position)--;
				if (ADD_OBJECT_TO_LIST(Cmiss_graphic)(graphic,list_of_graphic))
				{
					next_position++;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Cmiss_graphic_remove_from_list.  "
						"Could not readjust positions - graphic lost");
					return_code=0;
				}
				DEACCESS(Cmiss_graphic)(&graphic);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_graphic_remove_from_list.  Graphic not in list");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_remove_from_list.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_remove_from_list */

int Cmiss_graphic_modify_in_list(struct Cmiss_graphic *graphic,
	struct Cmiss_graphic *new_graphic,
	struct LIST(Cmiss_graphic) *list_of_graphic)
{
	int return_code,old_position;

	ENTER(Cmiss_graphic_modify_in_list);
	if (graphic&&new_graphic&&list_of_graphic)
	{
		if (IS_OBJECT_IN_LIST(Cmiss_graphic)(graphic,list_of_graphic))
		{
			/* save the current position */
			old_position=graphic->position;
			return_code=Cmiss_graphic_copy_without_graphics_object(graphic,new_graphic);
			graphic->position=old_position;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_graphic_modify_in_list.  graphic not in list");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_modify_in_list.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_modify_in_list */

DECLARE_OBJECT_FUNCTIONS(Cmiss_graphic);
DECLARE_INDEXED_LIST_FUNCTIONS(Cmiss_graphic);
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Cmiss_graphic, \
	position,int,compare_int);

int Cmiss_graphic_selects_elements(struct Cmiss_graphic *graphic)
{
	int return_code;
	
	ENTER(Cmiss_graphic_selects_elements);
	if (graphic)
	{
		return_code=(GRAPHICS_NO_SELECT != graphic->select_mode)&&(
			(CMISS_GRAPHIC_LINES==graphic->graphic_type)||
			(CMISS_GRAPHIC_CYLINDERS==graphic->graphic_type)||
			(CMISS_GRAPHIC_SURFACES==graphic->graphic_type)||
			(CMISS_GRAPHIC_ELEMENT_POINTS==graphic->graphic_type)||
			(CMISS_GRAPHIC_ISO_SURFACES==graphic->graphic_type)||
			(CMISS_GRAPHIC_VOLUMES==graphic->graphic_type));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_selects_elements.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_selects_elements */

enum Cmiss_graphic_type Cmiss_graphic_get_graphic_type(
	struct Cmiss_graphic *graphic)
{
	enum Cmiss_graphic_type graphic_type;

	ENTER(Cmiss_graphic_get_graphic_type);
	if (graphic)
	{
		graphic_type=graphic->graphic_type;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_graphic_type.  Invalid argument(s)");
		graphic_type = CMISS_GRAPHIC_LINES;
	}
	LEAVE;

	return (graphic_type);
}

int Cmiss_graphic_is_graphic_type(struct Cmiss_graphic *graphic,
	enum Cmiss_graphic_type graphic_type)
{
	int return_code = 0;

	ENTER(Cmiss_graphic_is_graphic_type);
	if (graphic)
	{
		if (graphic->graphic_type==graphic_type)
		{
			return_code = 1;
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_is_graphic_type.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

int Cmiss_graphic_get_visibility(struct Cmiss_graphic *graphic)
{
	int return_code;

	ENTER(Cmiss_graphic_get_visibility);
	if (graphic)
	{
		return_code=graphic->visibility;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_visibility.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_get_visibility */

int Cmiss_graphic_get_glyph_parameters(
	struct Cmiss_graphic *graphic,
	struct GT_object **glyph, enum Graphic_glyph_scaling_mode *glyph_scaling_mode,
	Triple glyph_centre, Triple glyph_size,
	struct Computed_field **orientation_scale_field, Triple glyph_scale_factors,
	struct Computed_field **variable_scale_field)
{
	int return_code;

	ENTER(Cmiss_graphic_get_glyph_parameters);
	if (graphic && glyph && glyph_scaling_mode && glyph_centre && glyph_size &&
		((CMISS_GRAPHIC_NODE_POINTS==graphic->graphic_type) ||
			(CMISS_GRAPHIC_DATA_POINTS==graphic->graphic_type) ||
			(CMISS_GRAPHIC_ELEMENT_POINTS==graphic->graphic_type) ||
			(CMISS_GRAPHIC_STATIC==graphic->graphic_type)) &&
		orientation_scale_field && glyph_scale_factors && variable_scale_field)
	{
		*glyph = graphic->glyph;
		*glyph_scaling_mode = graphic->glyph_scaling_mode;
		glyph_centre[0] = graphic->glyph_centre[0];
		glyph_centre[1] = graphic->glyph_centre[1];
		glyph_centre[2] = graphic->glyph_centre[2];
		glyph_size[0] = graphic->glyph_size[0];
		glyph_size[1] = graphic->glyph_size[1];
		glyph_size[2] = graphic->glyph_size[2];
		*orientation_scale_field = graphic->orientation_scale_field;
		glyph_scale_factors[0] = graphic->glyph_scale_factors[0];
		glyph_scale_factors[1] = graphic->glyph_scale_factors[1];
		glyph_scale_factors[2] = graphic->glyph_scale_factors[2];
		*variable_scale_field = graphic->variable_scale_field;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_glyph_parameters.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_get_glyph_parameters */

struct Computed_field *Cmiss_graphic_get_coordinate_field(
	struct Cmiss_graphic *graphic)
{
	struct Computed_field *coordinate_field;

	ENTER(Cmiss_grpahic_get_coordinate_field);
	if (graphic)
	{
		coordinate_field=graphic->coordinate_field;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_coordinate_field.  Invalid argument(s)");
		coordinate_field=(struct Computed_field *)NULL;
	}
	LEAVE;

	return (coordinate_field);
} /* Cmiss_graphic_get_coordinate_field */

int Cmiss_graphic_set_coordinate_field(
	struct Cmiss_graphic *graphic,struct Computed_field *coordinate_field)
{
	int return_code;

	ENTER(Cmiss_graphic_set_coordinate_field);
	if (graphic&&((!coordinate_field)||
		(3>=Computed_field_get_number_of_components(coordinate_field))))
	{
		REACCESS(Computed_field)(&(graphic->coordinate_field),coordinate_field);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_coordinate_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_set_coordinate_field */

int Cmiss_graphic_set_general_coordinate_field(
	struct Cmiss_graphic *graphic,void *coordinate_field_void)
{
	int return_code;
	struct Computed_field *coordinate_field;

	ENTER(Cmiss_graphic_set_general_coordinate_field);
	if (graphic&&(coordinate_field = (struct Computed_field *)coordinate_field_void))
	{
		if (graphic->coordinate_field != coordinate_field)
		{
			Cmiss_graphic_set_coordinate_field(graphic,coordinate_field);
			Cmiss_graphic_default_coordinate_field_change(graphic, (void *)NULL);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_general_coordinate_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}

int Cmiss_graphic_set_face(struct Cmiss_graphic *graphic, int face)
{
	int return_code;

	ENTER(Cmiss_graphic_set_face);
	if (graphic&&(
		Cmiss_graphic_type_uses_dimension(graphic->graphic_type,1)||
		Cmiss_graphic_type_uses_dimension(graphic->graphic_type,2)))
	{
		return_code=1;
		/* want -1 to represent none/all faces */
		if ((0>face)||(5<face))
		{
			face = -1;
		}
		graphic->face = face;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_face.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_set_face */

int Cmiss_graphic_set_exterior(struct Cmiss_graphic *graphic,
	int exterior)
{
	int return_code;

	ENTER(Cmiss_graphic_set_exterior);
	if (graphic&&(
		Cmiss_graphic_type_uses_dimension(graphic->graphic_type,1)||
		Cmiss_graphic_type_uses_dimension(graphic->graphic_type,2)))
	{
		return_code=1;
		/* ensure flags are 0 or 1 to simplify comparison with other graphic */
		if (exterior)
		{
			graphic->exterior = 1;
		}
		else
		{
			graphic->exterior = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_exterior.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}

int Cmiss_graphic_update_non_trivial_GT_objects(struct Cmiss_graphic *graphic)
{
	int return_code = 0;

	ENTER(Cmiss_graphic_update_non_trivial_GT_objects);
	if (graphic && graphic->graphics_object)
	{	
		set_GT_object_default_material(graphic->graphics_object,
			graphic->material);
		set_GT_object_secondary_material(graphic->graphics_object,
			graphic->secondary_material);
		set_GT_object_selected_material(graphic->graphics_object,
			graphic->selected_material);
		int overlay = 0;
		if (graphic->overlay_flag)
		{
			overlay = graphic->overlay_order;
		}
		set_GT_object_overlay(graphic->graphics_object, overlay);
		if (graphic->data_field && graphic->spectrum)
		{
			set_GT_object_Spectrum(graphic->graphics_object,
				(void *)graphic->spectrum);
		}
		graphic->graphics_changed = 1;
		graphic->selected_graphics_changed = 1;
		return_code = 1;
	}
	LEAVE;

	return return_code;
}

int Cmiss_graphic_set_material(struct Cmiss_graphic *graphic,
	struct Graphical_material *material)
{
	int return_code;

	ENTER(Cmiss_graphic_set_material);
	if (graphic&&material)
	{
		return_code=1;
		REACCESS(Graphical_material)(&(graphic->material),material);
		Cmiss_graphic_update_non_trivial_GT_objects(graphic);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_material.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_set_material */

int Cmiss_graphic_set_label_field(
	struct Cmiss_graphic *graphic,struct Computed_field *label_field,
	struct Graphics_font *font)
{
	int return_code;

	ENTER(Cmiss_graphic_set_label_field);
	if (graphic&&((!label_field)||
		((CMISS_GRAPHIC_NODE_POINTS==graphic->graphic_type)||
			(CMISS_GRAPHIC_DATA_POINTS==graphic->graphic_type)||
			(CMISS_GRAPHIC_ELEMENT_POINTS==graphic->graphic_type))) &&
		(!label_field || font))
	{
		REACCESS(Computed_field)(&(graphic->label_field), label_field);
		REACCESS(Graphics_font)(&(graphic->font), font);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_label_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}

int Cmiss_graphic_set_selected_material(
	struct Cmiss_graphic *graphic,
	struct Graphical_material *selected_material)
{
	int return_code;

	ENTER(Cmiss_graphic_set_selected_material);
	if (graphic&&selected_material)
	{
		return_code=1;
		REACCESS(Graphical_material)(&(graphic->selected_material),
			selected_material);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_selected_material.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}

int Cmiss_graphic_get_name(struct Cmiss_graphic *graphic,
	const char **name_ptr)
{
	int return_code;
	char *temp_ptr = NULL;
	ENTER(Cmiss_graphic_get_name);
	if (graphic&&name_ptr)
	{
		if (graphic->name)
		{
			if (ALLOCATE(temp_ptr,char,strlen(graphic->name)+1))
			{
				strcpy(temp_ptr,graphic->name);
				*name_ptr = temp_ptr;
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_graphic_get_name.  Could not allocate space for name");
				return_code=0;
			}
		}
		else
		{
			/* Use the position number */
			if (ALLOCATE(temp_ptr,char,30))
			{
				sprintf(temp_ptr,"%d",graphic->position);
				*name_ptr = temp_ptr;
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_graphic_get_name.  Could not allocate space for position name");
				return_code=0;
			}
			
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_name.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_get_name */

int Cmiss_graphic_set_name(struct Cmiss_graphic *graphic, const char *name)
{
	int return_code;

	ENTER(Cmiss_graphic_set_name);
	if (graphic&&name)
	{
		if (graphic->name)
		{
			DEALLOCATE(graphic->name);
		}
		if (ALLOCATE(graphic->name, char, strlen(name) + 1))
		{
			strcpy((char *)graphic->name, name);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_name.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_set_name */


char *Cmiss_graphic_string(struct Cmiss_graphic *graphic,
	enum Cmiss_graphic_string_details graphic_detail)
{
	char *graphic_string,temp_string[100],*name;
	int dimension,error,i;

	ENTER(Cmiss_graphic_string);
	graphic_string=(char *)NULL;
	if (graphic&&(
		(GRAPHIC_STRING_GEOMETRY==graphic_detail)||
		(GRAPHIC_STRING_COMPLETE==graphic_detail)||
		(GRAPHIC_STRING_COMPLETE_PLUS==graphic_detail)))
	{
		error=0;
		if (GRAPHIC_STRING_COMPLETE_PLUS==graphic_detail)
		{
			if (graphic->name)
			{
				sprintf(temp_string,"%i. (%s) ",graphic->position, graphic->name);
			}
			else
			{
				sprintf(temp_string,"%i. ",graphic->position);
			}
			append_string(&graphic_string,temp_string,&error);
		}

		/* show geometry graphic */
		/* for all graphic types */
		/* write graphic type = "points", "lines" etc. */
		append_string(&graphic_string,
			ENUMERATOR_STRING(Cmiss_graphic_type)(graphic->graphic_type),
			&error);
		if (graphic->name)
		{
			sprintf(temp_string," as %s", graphic->name);
			append_string(&graphic_string,temp_string,&error);
		}
		if (graphic->coordinate_field)
		{
			append_string(&graphic_string," coordinate ",&error);
			name=(char *)NULL;
			if (GET_NAME(Computed_field)(graphic->coordinate_field,&name))
			{
				/* put quotes around name if it contains special characters */
				make_valid_token(&name);
				append_string(&graphic_string,name,&error);
				DEALLOCATE(name);
			}
			else
			{
				append_string(&graphic_string,"NONE",&error);
			}
		}
		/* for 1-D and 2-D elements only */
		dimension=Cmiss_graphic_get_dimension(graphic);
		if ((1==dimension)||(2==dimension))
		{
			if (graphic->exterior)
			{
				append_string(&graphic_string," exterior",&error);
			}
			if (-1 != graphic->face)
			{
				append_string(&graphic_string," face",&error);
				switch (graphic->face)
				{
					case 0:
					{
						append_string(&graphic_string," xi1_0",&error);
					} break;
					case 1:
					{
						append_string(&graphic_string," xi1_1",&error);
					} break;
					case 2:
					{
						append_string(&graphic_string," xi2_0",&error);
					} break;
					case 3:
					{
						append_string(&graphic_string," xi2_1",&error);
					} break;
					case 4:
					{
						append_string(&graphic_string," xi3_0",&error);
					} break;
					case 5:
					{
						append_string(&graphic_string," xi3_1",&error);
					} break;
					default:
					{
						display_message(ERROR_MESSAGE,
							"Cmiss_graphic_string.  Invalid face number");
						DEALLOCATE(graphic_string);
						error=1;
					} break;
				}
			}
		}
		
		if ((CMISS_GRAPHIC_DATA_POINTS!=graphic->graphic_type) &&
			(CMISS_GRAPHIC_NODE_POINTS!=graphic->graphic_type) &&
			(CMISS_GRAPHIC_ELEMENT_POINTS!=graphic->graphic_type) &&
			(CMISS_GRAPHIC_STREAMLINES!=graphic->graphic_type))
		{
			sprintf(temp_string, " discretization \"%d*%d*%d\"",
				graphic->discretization.number_in_xi1,
				graphic->discretization.number_in_xi2,
				graphic->discretization.number_in_xi3);
			append_string(&graphic_string,temp_string,&error);	
		}

		/* for element_points and iso_surfaces */
		if (CMISS_GRAPHIC_LINES==graphic->graphic_type)
		{
			if (0 != graphic->line_width)
			{
				sprintf(temp_string, " line_width %d", graphic->line_width);
				append_string(&graphic_string,temp_string,&error);
			}
		}
		/* for cylinders only */
		if (CMISS_GRAPHIC_CYLINDERS==graphic->graphic_type)
		{
			sprintf(temp_string," circle_discretization %d", 
				graphic->circle_discretization);
			append_string(&graphic_string,temp_string,&error);
			if (0.0 != graphic->constant_radius)
			{
				sprintf(temp_string," constant_radius %g",graphic->constant_radius);
				append_string(&graphic_string,temp_string,&error);
			}
			if (graphic->radius_scalar_field)
			{
				if (GET_NAME(Computed_field)(graphic->radius_scalar_field,&name))
				{
					/* put quotes around name if it contains special characters */
					make_valid_token(&name);
					append_string(&graphic_string," radius_scalar ",&error);
					append_string(&graphic_string,name,&error);
					DEALLOCATE(name);
					if (1.0 != graphic->radius_scale_factor)
					{
						sprintf(temp_string," scale_factor %g",
							graphic->radius_scale_factor);
						append_string(&graphic_string,temp_string,&error);
					}
				}
				else
				{
					DEALLOCATE(graphic_string);
					error=1;
				}
			}
		}
		/* for iso_surfaces only */
		if (CMISS_GRAPHIC_ISO_SURFACES==graphic->graphic_type)
		{
			if (GET_NAME(Computed_field)(graphic->iso_scalar_field,&name))
			{
				/* put quotes around name if it contains special characters */
				make_valid_token(&name);
				append_string(&graphic_string," iso_scalar ",&error);
				append_string(&graphic_string,name,&error);
				DEALLOCATE(name);
			}
			else
			{
				DEALLOCATE(graphic_string);
				error=1;
			}
			if (graphic->iso_values)
			{
				sprintf(temp_string," iso_values");
				append_string(&graphic_string,temp_string,&error);
				for (i = 0 ; i < graphic->number_of_iso_values ; i++)
				{
					sprintf(temp_string, " %g", graphic->iso_values[i]);
					append_string(&graphic_string,temp_string,&error);				
				}
			}
			else
			{
				sprintf(temp_string," number_of_iso_values %d",
					graphic->number_of_iso_values);
				append_string(&graphic_string,temp_string,&error);				
				sprintf(temp_string," first_iso_value %g",
					graphic->first_iso_value);
				append_string(&graphic_string,temp_string,&error);				
				sprintf(temp_string," last_iso_value %g",
					graphic->last_iso_value);
				append_string(&graphic_string,temp_string,&error);				
			}
			if (graphic->decimation_threshold > 0.0)
			{
				sprintf(temp_string," decimation_threshold %g",
					graphic->decimation_threshold);
				append_string(&graphic_string,temp_string,&error);
			}
		}
		/* for node_points, data_points and element_points only */
		if ((CMISS_GRAPHIC_NODE_POINTS==graphic->graphic_type)||
			(CMISS_GRAPHIC_DATA_POINTS==graphic->graphic_type)||
			(CMISS_GRAPHIC_ELEMENT_POINTS==graphic->graphic_type)||
			(CMISS_GRAPHIC_STATIC==graphic->graphic_type))
		{
			if (graphic->glyph)
			{
				append_string(&graphic_string," glyph ",&error);
				if (GET_NAME(GT_object)(graphic->glyph, &name))
				{
					append_string(&graphic_string,name,&error);
					DEALLOCATE(name);
				}
				append_string(&graphic_string," ",&error);
				append_string(&graphic_string,
					ENUMERATOR_STRING(Graphic_glyph_scaling_mode)(graphic->glyph_scaling_mode),
					&error);
				sprintf(temp_string," size \"%g*%g*%g\"",graphic->glyph_size[0],
					graphic->glyph_size[1],graphic->glyph_size[2]);
				append_string(&graphic_string,temp_string,&error);
				sprintf(temp_string," centre %g,%g,%g",graphic->glyph_centre[0],
					graphic->glyph_centre[1],graphic->glyph_centre[2]);
				append_string(&graphic_string,temp_string,&error);
				if (graphic->font)
				{
					append_string(&graphic_string," font ",&error);
					if (GET_NAME(Graphics_font)(graphic->font, &name))
					{
						append_string(&graphic_string,name,&error);
						DEALLOCATE(name);
					}
				}
				if (graphic->label_field)
				{
					if (GET_NAME(Computed_field)(graphic->label_field,&name))
					{
						/* put quotes around name if it contains special characters */
						make_valid_token(&name);
						append_string(&graphic_string," label ",&error);
						append_string(&graphic_string,name,&error);
						DEALLOCATE(name);
					}
				}
				if (graphic->label_density_field)
				{
					if (GET_NAME(Computed_field)(graphic->label_density_field,&name))
					{
						/* put quotes around name if it contains special characters */
						make_valid_token(&name);
						append_string(&graphic_string," ldensity ",&error);
						append_string(&graphic_string,name,&error);
						DEALLOCATE(name);
					}
				}
				if (graphic->visibility_field)
				{
					if (GET_NAME(Computed_field)(graphic->visibility_field,&name))
					{
						/* put quotes around name if it contains special characters */
						make_valid_token(&name);
						append_string(&graphic_string," visibility ",&error);
						append_string(&graphic_string,name,&error);
						DEALLOCATE(name);
					}
				}
				if (graphic->orientation_scale_field)
				{
					if (GET_NAME(Computed_field)(graphic->orientation_scale_field,&name))
					{
						/* put quotes around name if it contains special characters */
						make_valid_token(&name);
						append_string(&graphic_string," orientation ",&error);
						append_string(&graphic_string,name,&error);
						DEALLOCATE(name);
					}
					else
					{
						DEALLOCATE(graphic_string);
						error=1;
					}
				}
				if (graphic->variable_scale_field)
				{
					if (GET_NAME(Computed_field)(graphic->variable_scale_field,&name))
					{
						/* put quotes around name if it contains special characters */
						make_valid_token(&name);
						append_string(&graphic_string," variable_scale ",&error);
						append_string(&graphic_string,name,&error);
						DEALLOCATE(name);
					}
					else
					{
						DEALLOCATE(graphic_string);
						error=1;
					}
				}
				if (graphic->orientation_scale_field || graphic->variable_scale_field)
				{
					sprintf(temp_string," scale_factors \"%g*%g*%g\"",
						graphic->glyph_scale_factors[0],
						graphic->glyph_scale_factors[1],
						graphic->glyph_scale_factors[2]);
					append_string(&graphic_string,temp_string,&error);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_graphic_string.  Graphic missing glyph");
				DEALLOCATE(graphic_string);
				error=1;
			}
		}
		/* for element_points and iso_surfaces */
		if ((CMISS_GRAPHIC_ELEMENT_POINTS==graphic->graphic_type)||
			(CMISS_GRAPHIC_ISO_SURFACES==graphic->graphic_type))
		{
			sprintf(temp_string, " %s",
				ENUMERATOR_STRING(Use_element_type)(graphic->use_element_type));
			append_string(&graphic_string,temp_string,&error);
		}
		/* for element_points only */
		if (CMISS_GRAPHIC_ELEMENT_POINTS==graphic->graphic_type)
		{
			append_string(&graphic_string," ",&error);
			append_string(&graphic_string, ENUMERATOR_STRING(Xi_discretization_mode)(
				graphic->xi_discretization_mode), &error);
			if (XI_DISCRETIZATION_EXACT_XI != graphic->xi_discretization_mode)
			{
				if ((XI_DISCRETIZATION_CELL_DENSITY ==
					graphic->xi_discretization_mode) ||
					(XI_DISCRETIZATION_CELL_POISSON == graphic->xi_discretization_mode))
				{
					append_string(&graphic_string, " density ", &error);
					if (GET_NAME(Computed_field)(graphic->xi_point_density_field,&name))
					{
						/* put quotes around name if it contains special characters */
						make_valid_token(&name);
						append_string(&graphic_string, name, &error);
						DEALLOCATE(name);
					}
					else
					{
						DEALLOCATE(graphic_string);
						error = 1;
					}
				}
				sprintf(temp_string,
					" discretization \"%d*%d*%d\" native_discretization ",
					graphic->discretization.number_in_xi1,
					graphic->discretization.number_in_xi2,
					graphic->discretization.number_in_xi3);
				append_string(&graphic_string,temp_string,&error);
				if (graphic->native_discretization_field)
				{
					if (GET_NAME(FE_field)(graphic->native_discretization_field,&name))
					{
						/* put quotes around name if it contains special characters */
						make_valid_token(&name);
						append_string(&graphic_string,name,&error);
						DEALLOCATE(name);
					}
					else
					{
						DEALLOCATE(graphic_string);
						error=1;
					}
				}
				else
				{
					append_string(&graphic_string,"NONE",&error);
				}
			}
		}
		/* for volumes only */
		if (CMISS_GRAPHIC_VOLUMES==graphic->graphic_type)
		{
			if (graphic->volume_texture)
			{
				append_string(&graphic_string," vtexture ",&error);
				append_string(&graphic_string,graphic->volume_texture->name,
					&error);
			}
			if (graphic->displacement_map_field)
			{
				if (GET_NAME(Computed_field)(graphic->displacement_map_field,&name))
				{
					/* put quotes around name if it contains special characters */
					make_valid_token(&name);
					append_string(&graphic_string," displacement_map_field ",&error);
					append_string(&graphic_string,name,&error);
					DEALLOCATE(name);
				}
				else
				{
					DEALLOCATE(graphic_string);
					error=1;
				}
				sprintf(temp_string," displacment_map_xi_direction %d",
					graphic->displacement_map_xi_direction);
				append_string(&graphic_string,temp_string,&error);
			}
		}
		/* for graphic starting in a particular element */
		if ((CMISS_GRAPHIC_VOLUMES==graphic->graphic_type)||
			(CMISS_GRAPHIC_STREAMLINES==graphic->graphic_type))
		{
			if (graphic->seed_element)
			{
				sprintf(temp_string, " seed_element %d",
					FE_element_get_cm_number(graphic->seed_element));
				append_string(&graphic_string, temp_string, &error);
			}
		}

		/* for graphic requiring an exact xi location */
		if (((CMISS_GRAPHIC_ELEMENT_POINTS==graphic->graphic_type)&&
			(XI_DISCRETIZATION_EXACT_XI == graphic->xi_discretization_mode))||
			(CMISS_GRAPHIC_STREAMLINES==graphic->graphic_type))
		{
			sprintf(temp_string," xi %g,%g,%g",
				graphic->seed_xi[0],graphic->seed_xi[1],graphic->seed_xi[2]);
			append_string(&graphic_string,temp_string,&error);
		}

		/* for streamlines only */
		if (CMISS_GRAPHIC_STREAMLINES==graphic->graphic_type)
		{
			append_string(&graphic_string," ",&error);
			append_string(&graphic_string,
				ENUMERATOR_STRING(Streamline_type)(graphic->streamline_type),&error);
			if (GET_NAME(Computed_field)(graphic->stream_vector_field,&name))
			{
				/* put quotes around name if it contains special characters */
				make_valid_token(&name);
				append_string(&graphic_string," vector ",&error);
				append_string(&graphic_string,name,&error);
				DEALLOCATE(name);
			}
			else
			{
				DEALLOCATE(graphic_string);
				error=1;
			}
			if (graphic->reverse_track)
			{
				append_string(&graphic_string," reverse_track ",&error);
			}
			sprintf(temp_string," length %g width %g ",
				graphic->streamline_length,graphic->streamline_width);
			append_string(&graphic_string,temp_string,&error);
			append_string(&graphic_string,
				ENUMERATOR_STRING(Streamline_data_type)(graphic->streamline_data_type),&error);
			if (graphic->seed_node_region_path)
			{
				sprintf(temp_string," seed_node_region %s", graphic->seed_node_region_path);
				append_string(&graphic_string,temp_string,&error);
			}
			if (graphic->seed_node_coordinate_field)
			{
				if (GET_NAME(Computed_field)(graphic->seed_node_coordinate_field,&name))
				{
					/* put quotes around name if it contains special characters */
					make_valid_token(&name);
					append_string(&graphic_string," seed_node_coordinate_field ",&error);
					append_string(&graphic_string,name,&error);
					DEALLOCATE(name);
				}
				else
				{
					DEALLOCATE(graphic_string);
					error=1;
				}
			}
		}
		append_string(&graphic_string," ",&error);
		append_string(&graphic_string,
			ENUMERATOR_STRING(Graphics_select_mode)(graphic->select_mode),&error);

		if ((GRAPHIC_STRING_COMPLETE==graphic_detail)||
			(GRAPHIC_STRING_COMPLETE_PLUS==graphic_detail))
		{
			/* show appearance graphic */
			/* for all graphic types */
			if (!graphic->visibility)
			{
				append_string(&graphic_string," invisible",&error);
			}
			if (graphic->material&&
				GET_NAME(Graphical_material)(graphic->material,&name))
			{
				/* put quotes around name if it contains special characters */
				make_valid_token(&name);
				append_string(&graphic_string," material ",&error);
				append_string(&graphic_string,name,&error);
				DEALLOCATE(name);
			}
			if (graphic->secondary_material&&
				GET_NAME(Graphical_material)(graphic->secondary_material,&name))
			{
				/* put quotes around name if it contains special characters */
				make_valid_token(&name);
				append_string(&graphic_string," secondary_material ",&error);
				append_string(&graphic_string,name,&error);
				DEALLOCATE(name);
			}
			if (graphic->texture_coordinate_field)
			{
				if (GET_NAME(Computed_field)(graphic->texture_coordinate_field,&name))
				{
					/* put quotes around name if it contains special characters */
					make_valid_token(&name);
					append_string(&graphic_string," texture_coordinates ",&error);
					append_string(&graphic_string,name,&error);
					DEALLOCATE(name);
				}
				else
				{
					DEALLOCATE(graphic_string);
					error=1;
				}
			}
			if (graphic->data_field)
			{
				if (GET_NAME(Computed_field)(graphic->data_field,&name))
				{
					/* put quotes around name if it contains special characters */
					make_valid_token(&name);
					append_string(&graphic_string," data ",&error);
					append_string(&graphic_string,name,&error);
					DEALLOCATE(name);
				}
				else
				{
					DEALLOCATE(graphic_string);
					error=1;
				}
				if (graphic->spectrum&&
					GET_NAME(Spectrum)(graphic->spectrum,&name))
				{
					/* put quotes around name if it contains special characters */
					make_valid_token(&name);
					append_string(&graphic_string," spectrum ",&error);
					append_string(&graphic_string,name,&error);
					DEALLOCATE(name);
				}
			}
			if (graphic->selected_material&&
				GET_NAME(Graphical_material)(graphic->selected_material,&name))
			{
				/* put quotes around name if it contains special characters */
				make_valid_token(&name);
				append_string(&graphic_string," selected_material ",&error);
				append_string(&graphic_string,name,&error);
				DEALLOCATE(name);
			}
			/* for surfaces and volumes */
			if ((CMISS_GRAPHIC_CYLINDERS==graphic->graphic_type)
				|| (CMISS_GRAPHIC_SURFACES==graphic->graphic_type) 
				|| (CMISS_GRAPHIC_VOLUMES==graphic->graphic_type)
				|| (CMISS_GRAPHIC_ISO_SURFACES==graphic->graphic_type))
			{
				append_string(&graphic_string," ",&error);
				append_string(&graphic_string,
					ENUMERATOR_STRING(Render_type)(graphic->render_type),&error);
			}
		}
		if (error)
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_graphic_string.  Error creating string");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_string.  Invalid argument(s)");
	}
	LEAVE;

	return graphic_string;
} /* Cmiss_graphic_string */

int Cmiss_graphic_to_static_graphics_object_at_time(
	struct Cmiss_graphic *graphic, float time)
{
	FE_value base_size[3], centre[3], scale_factors[3];
	int return_code = 1;
	struct GT_glyph_set *glyph_set;

	ENTER(Cmiss_graphic_to_static_graphics_object_at_time);
	if (graphic)
	{
		if (!graphic->customised_graphics_object)
		{
			GT_object_remove_primitives_at_time(
				graphic->graphics_object, time,
				(GT_object_primitive_object_name_conditional_function *)NULL,
				(void *)NULL);
			base_size[0] = (FE_value)(graphic->glyph_size[0]);
			base_size[1] = (FE_value)(graphic->glyph_size[1]);
			base_size[2] = (FE_value)(graphic->glyph_size[2]);
			centre[0] = (FE_value)(graphic->glyph_centre[0]);
			centre[1] = (FE_value)(graphic->glyph_centre[1]);
			centre[2] = (FE_value)(graphic->glyph_centre[2]);
			scale_factors[0] = (FE_value)(graphic->glyph_scale_factors[0]);
			scale_factors[1] = (FE_value)(graphic->glyph_scale_factors[1]);
			scale_factors[2] = (FE_value)(graphic->glyph_scale_factors[2]);
			
			Triple *point_list, *axis1_list, *axis2_list, *axis3_list,
				*scale_list;
			ALLOCATE(point_list, Triple, 1);
			ALLOCATE(axis1_list, Triple, 1);
			ALLOCATE(axis2_list, Triple, 1);
			ALLOCATE(axis3_list, Triple, 1);
			ALLOCATE(scale_list, Triple, 1);
			glyph_set = CREATE(GT_glyph_set)(1,
				point_list, axis1_list, axis2_list, axis3_list, scale_list, graphic->glyph, graphic->font,
				/*labels*/(char **)NULL, /*n_data_components*/0, /*data*/(GTDATA *)NULL,
				/*label_bounds_dimension*/0, /*label_bounds_components*/0, /*label_bounds*/(float *)NULL,
				/*label_density_list*/(Triple *)NULL, /*object_name*/0, /*names*/(int *)NULL);
			/* NOT an error if no glyph_set produced == empty group */
			(*point_list)[0] = centre[0];
			(*point_list)[1] = centre[1];
			(*point_list)[2] = centre[2];
			(*axis1_list)[0] = base_size[0];
			(*axis1_list)[1] = 0.0;
			(*axis1_list)[2] = 0.0;
			(*axis2_list)[0] = 0.0;
			(*axis2_list)[1] = base_size[1];
			(*axis2_list)[2] = 0.0;
			(*axis3_list)[0] = 0.0;
			(*axis3_list)[1] = 0.0;
			(*axis3_list)[2] = base_size[2];
			(*scale_list)[0] = scale_factors[0];
			(*scale_list)[1] = scale_factors[1];
			(*scale_list)[2] = scale_factors[2];
			if (glyph_set)
			{
				if (!GT_OBJECT_ADD(GT_glyph_set)(graphic->graphics_object,
						time,glyph_set))
				{
					DESTROY(GT_glyph_set)(&glyph_set);
					return_code=0;
				}
			}
			int	overlay = 0;
			if (graphic->overlay_flag)
			{
				overlay = graphic->overlay_order;
			}
			set_GT_object_overlay(graphic->graphics_object, overlay);
		}
		else
		{
			if (graphic->graphics_object != graphic->customised_graphics_object)
			{
				REACCESS(GT_object)(&graphic->graphics_object, 
					graphic->customised_graphics_object);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_to_static_graphics_object_at_time.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

#if defined (USE_OPENCASCADE)
static int Cad_shape_to_graphics_object(struct Computed_field *field,
	struct Cmiss_graphic_to_graphics_object_data *graphic_to_object_data)
{
	int return_code = 0;
	float time = 0.0;
	struct Cmiss_graphic *graphic = graphic_to_object_data->graphic;

	switch (graphic->graphic_type)
	{
		case CMISS_GRAPHIC_SURFACES:
		{
			//printf( "Building cad geometry surfaces\n" );
			struct GT_surface *surface = create_surface_from_cad_shape(field, graphic_to_object_data->rc_coordinate_field, graphic->data_field, graphic->render_type);
			if (surface && GT_OBJECT_ADD(GT_surface)(graphic->graphics_object, time, surface))
			{
				//printf( "Surface added to graphics object\n" );
				return_code = 1;
			}
			else
			{
				DESTROY(GT_surface)(&surface);
			}
			break;
		}
		case CMISS_GRAPHIC_LINES:
		{
			//struct GT_object *graphics_object = settings->graphics_object;
			/*
			GT_polyline_vertex_buffers *lines =
				CREATE(GT_polyline_vertex_buffers)(
				g_PLAIN, settings->line_width);
			*/
			GT_polyline_vertex_buffers *lines = create_curves_from_cad_shape(field, graphic_to_object_data->rc_coordinate_field, graphic->data_field, graphic->graphics_object);
			if (lines && GT_OBJECT_ADD(GT_polyline_vertex_buffers)(
				graphic->graphics_object, lines))
			{
				printf("Adding lines for cad shape\n");
				return_code = 1;
			}
			else
			{
				//DESTROY(GT_polyline_vertex_buffers)(&lines);
				return_code = 0;
			}
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,"Cad_geometry_to_graphics_object.  "
				"Can't handle this type of graphic");
			return_code = 0;
		}
	}

	return return_code;
}
#endif /* (USE_OPENCASCADE) */

int Cmiss_graphic_to_graphics_object(
	struct Cmiss_graphic *graphic,void *graphic_to_object_data_void)
{
	char *existing_name, *graphics_object_name, *graphic_string;
	const char *graphic_name = NULL;
	FE_value base_size[3], centre[3], scale_factors[3];
	float time;
	enum GT_object_type graphics_object_type;
	int return_code;
	struct Computed_field *coordinate_field;
	struct Coordinate_system *coordinate_system;
	struct FE_element *first_element;
	struct FE_region *fe_region;
	struct Cmiss_graphic_to_graphics_object_data *graphic_to_object_data;
	struct GT_glyph_set *glyph_set = NULL;
	struct Cmiss_graphic_select_graphics_data select_data;

	ENTER(Cmiss_graphic_to_graphics_object);
	if (graphic && (graphic_to_object_data =
		(struct Cmiss_graphic_to_graphics_object_data *)
			graphic_to_object_data_void) &&
		(((CMISS_GRAPHIC_DATA_POINTS == graphic->graphic_type) &&
			(fe_region = graphic_to_object_data->data_fe_region)) ||
			(fe_region = graphic_to_object_data->fe_region)))
	{
		/* all primitives added at time 0.0 */
		time = 0.0;
		coordinate_field = graphic_to_object_data->default_rc_coordinate_field;
		return_code = 1;
		/* build only if visible... */
		// GRC need to ask scene if visible
		if (Cmiss_scene_shows_graphic(graphic_to_object_data->scene, graphic))
		{
			/* build only if changed... */
			if (graphic->graphics_changed)
			{
				/* override the default_coordinate_field with one chosen in graphic */
				if (graphic->coordinate_field)
				{
					coordinate_field = graphic->coordinate_field;
				}
 				if (coordinate_field)
				{
					/* RC coordinate_field to pass to FE_element_to_graphics_object */
					if (NULL != (graphic_to_object_data->rc_coordinate_field=
							Computed_field_begin_wrap_coordinate_field(coordinate_field)) &&
						((!graphic->orientation_scale_field) ||
							(graphic_to_object_data->wrapper_orientation_scale_field =
								Computed_field_begin_wrap_orientation_scale_field(
									graphic->orientation_scale_field,
									graphic_to_object_data->rc_coordinate_field))) &&
						((!graphic->stream_vector_field) ||
							(graphic_to_object_data->wrapper_stream_vector_field =
								Computed_field_begin_wrap_orientation_scale_field(
									graphic->stream_vector_field,
									graphic_to_object_data->rc_coordinate_field))))
					{
#if defined (DEBUG)
						/*???debug*/
						if (graphic_string = Cmiss_graphic_string(graphic,
								GRAPHIC_STRING_COMPLETE_PLUS))
						{
							printf("> building %s\n", graphic_string);
							DEALLOCATE(graphic_string);
						}
#endif /* defined (DEBUG) */
						graphic_to_object_data->existing_graphics =
							(struct GT_object *)NULL;
						/* work out the name the graphics object is to have */
						Cmiss_graphic_get_name(graphic, &graphic_name);
						if (graphic_to_object_data->name_prefix && graphic_name &&
							ALLOCATE(graphics_object_name, char,
								strlen(graphic_to_object_data->name_prefix) +
								strlen(graphic_name) + 4))
						{
							sprintf(graphics_object_name, "%s.%s",
								graphic_to_object_data->name_prefix, graphic_name);
							if (graphic->graphics_object)
							{
								/* replace the graphics object name */
								GT_object_set_name(graphic->graphics_object,
									graphics_object_name);
								if (GT_object_has_primitives_at_time(graphic->graphics_object,
										time))
								{
#if defined (DEBUG)
									/*???debug*/printf("  EDIT EXISTING GRAPHICS!\n");
#endif /* defined (DEBUG) */
									GET_NAME(GT_object)(graphic->graphics_object, &existing_name);
									graphic_to_object_data->existing_graphics =
										CREATE(GT_object)(existing_name,
											GT_object_get_type(graphic->graphics_object),
											get_GT_object_default_material(graphic->graphics_object));
									DEALLOCATE(existing_name);
									GT_object_transfer_primitives_at_time(
										graphic_to_object_data->existing_graphics,
										graphic->graphics_object, time);
								}
							}
							else
							{
								switch (graphic->graphic_type)
								{
									case CMISS_GRAPHIC_LINES:
									{
										graphics_object_type = g_POLYLINE_VERTEX_BUFFERS;
									} break;
									case CMISS_GRAPHIC_CYLINDERS:
									case CMISS_GRAPHIC_SURFACES:
									{
										graphics_object_type = g_SURFACE;
									} break;
									case CMISS_GRAPHIC_ISO_SURFACES:
									{
										switch (graphic->use_element_type)
										{
											case USE_ELEMENTS:
											{
												//graphics_object_type = g_VOLTEX; // for old isosurfaces
												graphics_object_type = g_SURFACE; // for new isosurfaces
												if (NULL != (first_element = FE_region_get_first_FE_element_that(
															 fe_region, FE_element_is_top_level, (void *)NULL)))
												{
													if (2 == get_FE_element_dimension(first_element))
													{
														graphics_object_type = g_POLYLINE;
													}
												}
											} break;
											case USE_FACES:
											{
												graphics_object_type = g_POLYLINE;
											} break;
											case USE_LINES:
											{
												display_message(ERROR_MESSAGE,
													"Cmiss_graphic_to_graphics_object.  "
													"USE_LINES not supported for iso_scalar");
												return_code = 0;
											} break;
											default:
											{
												display_message(ERROR_MESSAGE,
													"Cmiss_graphic_to_graphics_object.  "
													"Unknown use_element_type");
												return_code = 0;
											} break;
										}
									} break;
									case CMISS_GRAPHIC_NODE_POINTS:
									case CMISS_GRAPHIC_DATA_POINTS:
									case CMISS_GRAPHIC_ELEMENT_POINTS:
									case CMISS_GRAPHIC_STATIC:
									{
										graphics_object_type = g_GLYPH_SET;
									} break;
									case CMISS_GRAPHIC_VOLUMES:
									{
										graphics_object_type = g_VOLTEX;
									} break;
									case CMISS_GRAPHIC_STREAMLINES:
									{
										if (STREAM_LINE == graphic->streamline_type)
										{
											graphics_object_type = g_POLYLINE;
										}
										else
										{
											graphics_object_type = g_SURFACE;
										}
									} break;
									default:
									{
										display_message(ERROR_MESSAGE,
											"Cmiss_graphic_to_graphics_object.  "
											"Unknown graphic type");
										return_code = 0;
									} break;
								}
								if (return_code)
								{
									graphic->graphics_object = CREATE(GT_object)(
										graphics_object_name, graphics_object_type,
										graphic->material);
									GT_object_set_select_mode(graphic->graphics_object,
										graphic->select_mode);
									if (graphic->secondary_material)
									{
										set_GT_object_secondary_material(graphic->graphics_object,
											graphic->secondary_material);
									}
									if (graphic->selected_material)
									{
										set_GT_object_selected_material(graphic->graphics_object,
											graphic->selected_material);
									}
									ACCESS(GT_object)(graphic->graphics_object);
								}
							}
							DEALLOCATE(graphics_object_name);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Cmiss_graphic_to_graphics_object.  "
								"Unable to make graphics object name");
							return_code = 0;
						}
						if (graphic_name)
							DEALLOCATE(graphic_name);
						if (graphic->data_field)
						{
							graphic_to_object_data->number_of_data_values =
								Computed_field_get_number_of_components(graphic->data_field);
							ALLOCATE(graphic_to_object_data->data_copy_buffer,
								FE_value, graphic_to_object_data->number_of_data_values);
						}
						if (graphic->graphics_object)
						{
							graphic->selected_graphics_changed=1;
							coordinate_system = Computed_field_get_coordinate_system(coordinate_field);
							GT_object_set_coordinate_system(graphic->graphics_object,
								(NORMALISED_WINDOW_COORDINATES == coordinate_system->type) ?
									g_NDC_COORDINATES : g_MODEL_COORDINATES);
							/* need graphic for FE_element_to_graphics_object routine */
							graphic_to_object_data->graphic=graphic;
							switch (graphic->graphic_type)
							{
								case CMISS_GRAPHIC_NODE_POINTS:
								case CMISS_GRAPHIC_DATA_POINTS:
								{
									/* currently all nodes put together in a single GT_glyph_set,
										so rebuild all even if editing a single node or element */
									GT_object_remove_primitives_at_time(
										graphic->graphics_object, time,
										(GT_object_primitive_object_name_conditional_function *)NULL,
										(void *)NULL);
									base_size[0] = (FE_value)(graphic->glyph_size[0]);
									base_size[1] = (FE_value)(graphic->glyph_size[1]);
									base_size[2] = (FE_value)(graphic->glyph_size[2]);
									centre[0] = (FE_value)(graphic->glyph_centre[0]);
									centre[1] = (FE_value)(graphic->glyph_centre[1]);
									centre[2] = (FE_value)(graphic->glyph_centre[2]);
									scale_factors[0] = (FE_value)(graphic->glyph_scale_factors[0]);
									scale_factors[1] = (FE_value)(graphic->glyph_scale_factors[1]);
									scale_factors[2] = (FE_value)(graphic->glyph_scale_factors[2]);
									glyph_set = create_GT_glyph_set_from_FE_region_nodes(
										fe_region, graphic_to_object_data->rc_coordinate_field,
										graphic->glyph, base_size, centre, scale_factors,
										graphic_to_object_data->time,
										graphic_to_object_data->wrapper_orientation_scale_field,
										graphic->variable_scale_field, graphic->data_field, 
										graphic->font, graphic->label_field,
										graphic->label_density_field, graphic->visibility_field,
										graphic->select_mode,graphic_to_object_data->group_field);
									/* NOT an error if no glyph_set produced == empty group */
									if (glyph_set)
									{
										if (!GT_OBJECT_ADD(GT_glyph_set)(graphic->graphics_object,
												time,glyph_set))
										{
											DESTROY(GT_glyph_set)(&glyph_set);
											return_code=0;
										}
									}
								} break;
								case CMISS_GRAPHIC_STATIC:
								{
									Cmiss_graphic_to_static_graphics_object_at_time(
										graphic, time);
								} break;
								case CMISS_GRAPHIC_LINES:
								{
#if defined(USE_OPENCASCADE)
									// test here for domain of rc_coordinate_field
									// if it is a cad_geometry do something about it
									//if ( is_cad_geometry( settings_to_object_data->rc_coordinate_field->get_domain() ) )
									struct LIST(Computed_field) *domain_field_list = CREATE_LIST(Computed_field)();
									int return_code = Computed_field_get_domain( graphic_to_object_data->rc_coordinate_field, domain_field_list );
									if ( return_code )
									{
										//printf( "got domain of rc_coordinate_field (%d)\n", NUMBER_IN_LIST(Computed_field)(domain_field_list) );
										// so test for topology domain
										struct Computed_field *cad_topology_field = FIRST_OBJECT_IN_LIST_THAT(Computed_field)
											( Cmiss_field_is_type_cad_topology, (void *)NULL, domain_field_list );
										if ( cad_topology_field )
										{
											//printf( "hurrah, we have a cad topology domain.\n" );
											// if topology domain then draw item at location
											return_code = Cad_shape_to_graphics_object( cad_topology_field, graphic_to_object_data );
											DESTROY_LIST(Computed_field)(&domain_field_list);
											break;
										}
									}
									if ( domain_field_list )
										DESTROY_LIST(Computed_field)(&domain_field_list);
#endif /* defined(USE_OPENCASCADE) */
									GT_polyline_vertex_buffers *lines =
										CREATE(GT_polyline_vertex_buffers)(
										g_PLAIN, graphic->line_width);
									if (GT_OBJECT_ADD(GT_polyline_vertex_buffers)(
										graphic->graphics_object, lines))
									{
										return_code = FE_region_for_each_FE_element(fe_region,
											FE_element_to_graphics_object, graphic_to_object_data_void);
									}
									else
									{
										//DESTROY(GT_polyline_vertex_buffers)(lines);
										return_code = 0;
									}
								} break;
								case CMISS_GRAPHIC_SURFACES:
#if defined(USE_OPENCASCADE)
								{
									// test here for domain of rc_coordinate_field
									// if it is a cad_geometry do something about it
									//if ( is_cad_geometry( settings_to_object_data->rc_coordinate_field->get_domain() ) )
									struct LIST(Computed_field) *domain_field_list = CREATE_LIST(Computed_field)();
									int return_code = Computed_field_get_domain( graphic_to_object_data->rc_coordinate_field, domain_field_list );
									if ( return_code )
									{
										//printf( "got domain of rc_coordinate_field (%d)\n", NUMBER_IN_LIST(Computed_field)(domain_field_list) );
										// so test for topology domain
										struct Computed_field *cad_topology_field = FIRST_OBJECT_IN_LIST_THAT(Computed_field)
											( Cmiss_field_is_type_cad_topology, (void *)NULL, domain_field_list );
										if ( cad_topology_field )
										{
											//printf( "hurrah, we have a cad topology domain.\n" );
											// if topology domain then draw item at location
											return_code = Cad_shape_to_graphics_object( cad_topology_field, graphic_to_object_data );
											DESTROY_LIST(Computed_field)(&domain_field_list);
											break;
										}
									}
									if ( domain_field_list )
										DESTROY_LIST(Computed_field)(&domain_field_list);
								}
#endif /* defined(USE_OPENCASCADE) */
								case CMISS_GRAPHIC_CYLINDERS:
								case CMISS_GRAPHIC_ELEMENT_POINTS:
								{
									return_code = FE_region_for_each_FE_element(fe_region,
										FE_element_to_graphics_object, graphic_to_object_data_void);
								} break;
								case CMISS_GRAPHIC_ISO_SURFACES:
								{
									if (0 < graphic->number_of_iso_values)
									{
										if (g_SURFACE == GT_object_get_type(graphic->graphics_object))
										{
											graphic_to_object_data->iso_surface_specification =
												Iso_surface_specification_create(
													graphic->number_of_iso_values, graphic->iso_values,
													graphic->first_iso_value, graphic->last_iso_value,
													graphic_to_object_data->rc_coordinate_field,
													graphic->data_field,
													graphic->iso_scalar_field,
													graphic->texture_coordinate_field);
										}
										return_code = FE_region_for_each_FE_element(fe_region,
											FE_element_to_graphics_object,
											graphic_to_object_data_void);
										if (g_SURFACE == GT_object_get_type(graphic->graphics_object))
										{
											Iso_surface_specification_destroy(&graphic_to_object_data->iso_surface_specification);
											/* Decimate */
											if (graphic->decimation_threshold > 0.0)
											{
												GT_object_decimate_GT_surface(graphic->graphics_object,
													graphic->decimation_threshold);
											}
										}
										/* If the isosurface is a volume we can decimate and
											then normalise, otherwise if it is a polyline
											representing a isolines, skip over. */
										if (g_VOLTEX == GT_object_get_type(graphic->graphics_object))
										{
											/* Decimate */
											if (graphic->decimation_threshold > 0.0)
											{
												GT_object_decimate_GT_voltex(graphic->graphics_object,
													graphic->decimation_threshold);
											}
											/* Normalise normals now that the entire mesh has been calculated */
											GT_object_normalise_GT_voltex_normals(graphic->graphics_object);
										}
									}
								} break;
								case CMISS_GRAPHIC_VOLUMES:
								{
									/* does volume texture extend beyond a single element? */
									if (graphic->volume_texture && (
											 (graphic->volume_texture->ximin[0] < 0.0) ||
											 (graphic->volume_texture->ximin[1] < 0.0) ||
											 (graphic->volume_texture->ximin[2] < 0.0) ||
											 (graphic->volume_texture->ximax[0] > 1.0) ||
											 (graphic->volume_texture->ximax[1] > 1.0) ||
											 (graphic->volume_texture->ximax[2] > 1.0)))
									{
										/* then must rebuild all graphics */
										if (graphic_to_object_data->existing_graphics)
										{
											DESTROY(GT_object)(
												&(graphic_to_object_data->existing_graphics));
										}
									}
									if (graphic->seed_element)
									{
										return_code = FE_element_to_graphics_object(
											graphic->seed_element, graphic_to_object_data_void);
									}
									else
									{
										return_code = FE_region_for_each_FE_element(fe_region,
											FE_element_to_graphics_object,
											graphic_to_object_data_void);
									}
								} break;
								case CMISS_GRAPHIC_STREAMLINES:
								{
									/* must always regenerate ALL streamlines since they can cross
										into other elements */
									if (graphic_to_object_data->existing_graphics)
									{
										DESTROY(GT_object)(
											&(graphic_to_object_data->existing_graphics));
									}
									if (graphic->seed_element)
									{
										return_code = FE_element_to_graphics_object(
											graphic->seed_element, graphic_to_object_data_void);
									}
									else if (graphic->seed_node_region &&
										graphic->seed_node_coordinate_field)
									{
										return_code = FE_region_for_each_FE_node(
											graphic->seed_node_region,
											Cmiss_node_to_streamline, graphic_to_object_data_void);
									}
									else
									{
										return_code = FE_region_for_each_FE_element(fe_region,
											FE_element_to_graphics_object,
											graphic_to_object_data_void);
									}
								} break;
								default:
								{
									return_code = 0;
								} break;
							} /* end of switch */
							if (return_code)
							{
								/* set the spectrum in the graphics object - if required */
								if ((graphic->data_field)||
									((CMISS_GRAPHIC_STREAMLINES == graphic->graphic_type) &&
										(STREAM_NO_DATA != graphic->streamline_data_type)))
								{
									set_GT_object_Spectrum(graphic->graphics_object,
										(void *)(graphic->spectrum));
								}
								/* mark display list as needing updating */
								graphic->graphics_changed = 0;
								GT_object_changed(graphic->graphics_object);
							}
							else
							{
								graphic_string = Cmiss_graphic_string(graphic,
									GRAPHIC_STRING_COMPLETE_PLUS);
								display_message(ERROR_MESSAGE,
									"cmiss_graphic_to_graphics_object.  "
									"Could not build '%s'",graphic_string);
								DEALLOCATE(graphic_string);
								/* set return_code to 1, so rest of graphic can be built */
								return_code = 1;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"cmiss_graphic_to_graphics_object.  "
								"Could not create graphics object");
							return_code = 0;
						}
						if (graphic_to_object_data->existing_graphics)
						{
							DESTROY(GT_object)(&(graphic_to_object_data->existing_graphics));
						}
						if (graphic->stream_vector_field)
						{
							Computed_field_clear_cache(
								graphic_to_object_data->wrapper_stream_vector_field);
							Computed_field_end_wrap(
								&(graphic_to_object_data->wrapper_stream_vector_field));
						}
						if (graphic->orientation_scale_field)
						{
							Computed_field_end_wrap(
								&(graphic_to_object_data->wrapper_orientation_scale_field));
						}
						Computed_field_clear_cache(graphic_to_object_data->rc_coordinate_field);
						Computed_field_end_wrap(
							&(graphic_to_object_data->rc_coordinate_field));
						if (graphic->seed_node_coordinate_field)
						{
							Computed_field_clear_cache(graphic->seed_node_coordinate_field);
						}
						if (graphic->data_field)
						{
							graphic_to_object_data->number_of_data_values = 0;
							DEALLOCATE(graphic_to_object_data->data_copy_buffer);
							/* clear Computed_field caches so elements not accessed */
							Computed_field_clear_cache(graphic->data_field);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"cmiss_graphic_to_graphics_object.  "
							"Could not get rc_coordinate_field wrapper");
						return_code = 0;
					}
				}
				else if (graphic->graphic_type == CMISS_GRAPHIC_STATIC)
				{
					graphic_to_object_data->existing_graphics =
						(struct GT_object *)NULL;
					/* work out the name the graphics object is to have */
					Cmiss_graphic_get_name(graphic, &graphic_name);
					if (graphic_to_object_data->name_prefix && graphic_name &&
						ALLOCATE(graphics_object_name, char,
							strlen(graphic_to_object_data->name_prefix) +
							strlen(graphic_name) + 4))
					{
						sprintf(graphics_object_name, "%s_%s",
							graphic_to_object_data->name_prefix, graphic_name);
						if (graphic->graphics_object)
						{
							/* replace the graphics object name */
							GT_object_set_name(graphic->graphics_object,
								graphics_object_name);
							if (GT_object_has_primitives_at_time(graphic->graphics_object,
									time))
							{
								GET_NAME(GT_object)(graphic->graphics_object, &existing_name);
								graphic_to_object_data->existing_graphics =
									CREATE(GT_object)(existing_name,
										GT_object_get_type(graphic->graphics_object),
										get_GT_object_default_material(graphic->graphics_object));
								DEALLOCATE(existing_name);
								GT_object_transfer_primitives_at_time(
									graphic_to_object_data->existing_graphics,
									graphic->graphics_object, time);
							}
						}
						else 
						{
							graphics_object_type = g_GLYPH_SET;
							graphic->graphics_object = CREATE(GT_object)(
								graphics_object_name, graphics_object_type,
								graphic->material);
							GT_object_set_select_mode(graphic->graphics_object,
							graphic->select_mode);
							if (graphic->secondary_material)
							{
								set_GT_object_secondary_material(graphic->graphics_object,
									graphic->secondary_material);
							}
							if (graphic->selected_material)
							{
								set_GT_object_selected_material(graphic->graphics_object,
									graphic->selected_material);
							}
							ACCESS(GT_object)(graphic->graphics_object);
						}
						if (Cmiss_graphic_to_static_graphics_object_at_time(
									graphic, time))
						{
							graphic->graphics_changed = 0;
							GT_object_changed(graphic->graphics_object);
						}
						DEALLOCATE(graphics_object_name);
					}
					if (graphic_name)
					{
						DEALLOCATE(graphic_name);			
					}
				}
			}


			if (graphic->selected_graphics_changed)
			{
				if (graphic->graphics_object)
				{

					if ((GRAPHICS_SELECT_ON == graphic->select_mode) ||
						(GRAPHICS_DRAW_SELECTED == graphic->select_mode))
					{
						switch (graphic->graphic_type)
						{
							case CMISS_GRAPHIC_DATA_POINTS:
							{
								GT_object_set_node_highlight_functor(graphic->graphics_object, NULL);
							} break;
							case CMISS_GRAPHIC_NODE_POINTS:
							{
							  GT_object_set_node_highlight_functor(graphic->graphics_object,
							  	(void *)graphic_to_object_data->group_field);
							} break;
							case CMISS_GRAPHIC_CYLINDERS:
							case CMISS_GRAPHIC_LINES:
							case CMISS_GRAPHIC_SURFACES:
							case CMISS_GRAPHIC_ISO_SURFACES:
							case CMISS_GRAPHIC_VOLUMES:
							{
                GT_object_set_element_highlight_functor(graphic->graphics_object,
                  (void *)graphic_to_object_data->group_field);
							} break;
							case CMISS_GRAPHIC_ELEMENT_POINTS:
							{
								select_data.fe_region=fe_region;
								select_data.graphic=graphic;

								FOR_EACH_OBJECT_IN_LIST(FE_element)(
									FE_element_select_graphics_element_points,
									(void *)&select_data,
									graphic_to_object_data->selected_element_list);
								/* select Element_point_ranges for glyph_sets not already
									 selected as elements */
								FOR_EACH_OBJECT_IN_LIST(Element_point_ranges)(
									Element_point_ranges_select_in_graphics_object,
									(void *)&select_data,
									graphic_to_object_data->selected_element_point_ranges_list);
							} break;
							case CMISS_GRAPHIC_STATIC:
							case CMISS_GRAPHIC_STREAMLINES:
							{
								/* no element to select by since go through several */
							} break;
							default:
							{
								display_message(ERROR_MESSAGE,
									"cmiss_graphic_to_graphics_object.  "
									"Unknown graphic type");
							} break;
						}
					}
				}
				graphic->selected_graphics_changed = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmiss_graphic_to_graphics_object.  Invalid argument(s)");
		return_code = 0;
	}

	LEAVE;

	return (return_code);
}

int Cmiss_graphic_compile_visible_graphic(
	struct Cmiss_graphic *graphic, void *renderer_void)
{
	int return_code = 1;
	Render_graphics_opengl *renderer;

	ENTER(Cmiss_graphic_compile_visible_graphic);
	if (graphic && (renderer = static_cast<Render_graphics_opengl *>(renderer_void)))
	{
		return_code = 1;
		if (graphic->graphics_object &&
			Cmiss_scene_shows_graphic(renderer->get_scene(), graphic))
		{
#if defined (OPENGL_API)
			glLoadName((GLuint)graphic->position);
#endif
			return_code = renderer->Graphics_object_compile(graphic->graphics_object);
			if (graphic->overlay_flag)
			{
				return_code = renderer->Register_overlay_graphics_object(graphic->graphics_object);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_compile_visible_graphic.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_compile_visible_graphic */

int Cmiss_graphic_glyph_change(
	struct GT_object *glyph,void *graphic_void)
{
	int return_code;
	Cmiss_graphic *graphic =NULL;
	
	ENTER(Cmiss_graphic_glyph_change);
	graphic = (Cmiss_graphic *)graphic_void;
	if (glyph && graphic)
	{
		graphic->graphics_changed = 1;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_glyph_change.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_glyph_change */

int Cmiss_graphic_execute_visible_graphic(
	struct Cmiss_graphic *graphic, void *renderer_void)
{
	int return_code = 1;
	Render_graphics_opengl *renderer;

	ENTER(Cmiss_graphic_execute_visible_graphic);
	if (graphic && (renderer = static_cast<Render_graphics_opengl *>
			(renderer_void)))
	{
		return_code = 1;
		if (graphic->graphics_object &&
			Cmiss_scene_shows_graphic(renderer->get_scene(), graphic))
		{
			if (!graphic->overlay_flag)
			{
				//printf("     %i\n", graphic->position);
#if defined (OPENGL_API)
				/* use position in list as name for GL picking */
				glLoadName((GLuint)graphic->position);
#endif /* defined (OPENGL_API) */
				return_code = renderer->Graphics_object_execute(graphic->graphics_object);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_execute_visible_graphic.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_execute_visible_graphic */

/***************************************************************************//**
 * Clears the graphics stored in the graphic and marks them for rebuild.
 */
static int Cmiss_graphic_clear_graphics(
	struct Cmiss_graphic *graphic)
{
	int return_code;

	ENTER(Cmiss_graphic_clear_graphics);
	if (graphic)
	{
		if (graphic->graphics_object)
		{
			/*???RC Once again, assumes graphics stored at time 0.0 */
			GT_object_remove_primitives_at_time(
				graphic->graphics_object, /*time*/0.0,
				(GT_object_primitive_object_name_conditional_function *)NULL,
				(void *)NULL);
		}
		graphic->graphics_changed = 1;
		graphic->selected_graphics_changed = 1;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_clear_graphics.  Missing graphic");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_clear_graphics */

static int Cmiss_graphic_Computed_field_or_ancestor_satisfies_condition(
	struct Cmiss_graphic *graphic,
	struct Computed_field *default_coordinate_field,
	LIST_CONDITIONAL_FUNCTION(Computed_field) *conditional_function,
	void *user_data)
{
	int return_code;
	struct Computed_field *coordinate_field;

	ENTER(Cmiss_graphic_Computed_field_or_ancestor_satisfies_condition);
	if (graphic && conditional_function)
	{
		return_code = 0;
		/* compare geometry graphic */
		/* for all graphic types */
		/* graphic can get default coordinate field from gt_element_group */
		if (graphic->coordinate_field)
		{
			coordinate_field = graphic->coordinate_field;
		}
		else
		{
			coordinate_field = default_coordinate_field;
		}
		if (coordinate_field &&
			Computed_field_or_ancestor_satisfies_condition(
				coordinate_field, conditional_function, user_data))
		{
			return_code = 1;
		}
		/* currently for surfaces only */
		else if (graphic->texture_coordinate_field &&
			Computed_field_or_ancestor_satisfies_condition(
				graphic->texture_coordinate_field, conditional_function, user_data))
		{
			return_code = 1;
		}
		/* for cylinders only */
		else if ((CMISS_GRAPHIC_CYLINDERS == graphic->graphic_type) &&
			graphic->radius_scalar_field &&
			Computed_field_or_ancestor_satisfies_condition(
				graphic->radius_scalar_field, conditional_function, user_data))
		{
			return_code = 1;
		}
		/* for iso_surfaces only */
		else if ((CMISS_GRAPHIC_ISO_SURFACES == graphic->graphic_type) &&
			(graphic->iso_scalar_field &&
			Computed_field_or_ancestor_satisfies_condition(
				graphic->iso_scalar_field, conditional_function, user_data)))
		{
			return_code = 1;
		}
		/* for node_points, data_points and element_points only */
		else if (((CMISS_GRAPHIC_NODE_POINTS == graphic->graphic_type) ||
			(CMISS_GRAPHIC_DATA_POINTS == graphic->graphic_type) ||
			(CMISS_GRAPHIC_ELEMENT_POINTS == graphic->graphic_type)) &&
			((graphic->orientation_scale_field &&
				(Computed_field_or_ancestor_satisfies_condition(
					graphic->orientation_scale_field, conditional_function, user_data)))||
			(graphic->variable_scale_field &&
				(Computed_field_or_ancestor_satisfies_condition(
					graphic->variable_scale_field, conditional_function, user_data))) ||
			(graphic->label_field &&
				(Computed_field_or_ancestor_satisfies_condition(
					graphic->label_field, conditional_function, user_data))) ||
			(graphic->label_density_field &&
				(Computed_field_or_ancestor_satisfies_condition(
					graphic->label_density_field, conditional_function, user_data))) ||
			(graphic->visibility_field &&
				(Computed_field_or_ancestor_satisfies_condition(
					graphic->visibility_field, conditional_function, user_data)))))
		{
			return_code = 1;
		}
		/* for element_points with a density field only */
		else if ((CMISS_GRAPHIC_ELEMENT_POINTS == graphic->graphic_type)
			&& ((XI_DISCRETIZATION_CELL_DENSITY ==
				graphic->xi_discretization_mode) ||
				(XI_DISCRETIZATION_CELL_POISSON ==
					graphic->xi_discretization_mode)) &&
			Computed_field_or_ancestor_satisfies_condition(
				graphic->xi_point_density_field, conditional_function, user_data))
		{
			return_code = 1;
		}
		/* for volumes only */
		else if ((CMISS_GRAPHIC_VOLUMES == graphic->graphic_type)&&
			(graphic->displacement_map_field &&
				Computed_field_or_ancestor_satisfies_condition(
					graphic->displacement_map_field, conditional_function, user_data)))
		{
			return_code = 1;
		}
		/* for streamlines only */
		else if ((CMISS_GRAPHIC_STREAMLINES == graphic->graphic_type) &&
			graphic->stream_vector_field &&
			Computed_field_or_ancestor_satisfies_condition(
				graphic->stream_vector_field, conditional_function, user_data))
		{
			return_code = 1;
		}
		else if (CMISS_GRAPHIC_STATIC == graphic->graphic_type)
		{
			return_code = 0;
		}
		/* appearance graphic for all graphic types */
		else if (graphic->data_field &&
			Computed_field_or_ancestor_satisfies_condition(
				graphic->data_field, conditional_function, user_data))
		{
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_Computed_field_or_ancestor_satisfies_condition.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_Computed_field_or_ancestor_satisfies_condition */

static int Cmiss_graphic_uses_changed_FE_field(
	struct Cmiss_graphic *graphic,
	struct CHANGE_LOG(FE_field) *fe_field_change_log)
{
	enum CHANGE_LOG_CHANGE(FE_field) fe_field_change;
	int return_code;

	ENTER(Cmiss_graphic_uses_changed_FE_field);
	if (graphic && ((CMISS_GRAPHIC_STATIC==graphic->graphic_type) || 
			graphic->coordinate_field) && fe_field_change_log)
	{
		if ((CMISS_GRAPHIC_ELEMENT_POINTS == graphic->graphic_type) &&
			graphic->native_discretization_field &&
			CHANGE_LOG_QUERY(FE_field)(fe_field_change_log,
				graphic->native_discretization_field, &fe_field_change) &&
			(CHANGE_LOG_OBJECT_UNCHANGED(FE_field) != fe_field_change))
		{
			return_code = 1;
		}
		else if (CMISS_GRAPHIC_STATIC!=graphic->graphic_type)
		{
			return_code =
				Cmiss_graphic_Computed_field_or_ancestor_satisfies_condition(
					graphic, graphic->coordinate_field,
					Computed_field_contains_changed_FE_field,
					(void *)fe_field_change_log);
		}
		else
		{
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_uses_changed_FE_field.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_uses_changed_FE_field */

int Cmiss_graphic_Computed_field_change(
	struct Cmiss_graphic *graphic, void *change_data_void)
{
	int return_code;
	struct Cmiss_graphic_Computed_field_change_data *change_data;

	ENTER(Cmiss_graphic_Computed_field_change);
	if (graphic && (change_data =
		(struct Cmiss_graphic_Computed_field_change_data *)change_data_void))
	{
		return_code = 1;
		if (Cmiss_graphic_Computed_field_or_ancestor_satisfies_condition(
			graphic, change_data->default_coordinate_field,
			Computed_field_is_in_list, (void *)change_data->changed_field_list))
		{
			Cmiss_graphic_clear_graphics(graphic);
			change_data->graphics_changed = 1;
		}
	  if (change_data->group_field && Computed_field_or_ancestor_satisfies_condition(
	  		change_data->group_field,
	  		Computed_field_is_in_list, (void *)change_data->changed_field_list))
	  {
	  	if (graphic->graphics_object&&
	  			((CMISS_GRAPHIC_NODE_POINTS==graphic->graphic_type)||
	  				(CMISS_GRAPHIC_LINES==graphic->graphic_type) ||
	  				(CMISS_GRAPHIC_CYLINDERS==graphic->graphic_type) ||
	  				(CMISS_GRAPHIC_SURFACES==graphic->graphic_type)))
	  	{
	  		switch (graphic->select_mode)
	  		{
					case GRAPHICS_SELECT_ON:
					{
						/* for efficiency, just request update of selected graphics */
						Cmiss_graphic_clear_graphics(graphic);
						change_data->graphics_changed = 1;
					} break;
					case GRAPHICS_NO_SELECT:
					{
						/* nothing to do as no names put out with graphic */
					} break;
					case GRAPHICS_DRAW_SELECTED:
					case GRAPHICS_DRAW_UNSELECTED:
					{
						/* need to rebuild graphics_object from scratch */
						Cmiss_graphic_clear_graphics(graphic);
						change_data->graphics_changed = 1;
					} break;
					default:
					{
						display_message(ERROR_MESSAGE,
								"Cmiss_graphic_selected_nodes_change.  Unknown select_mode");
					} break;
	  		}
	  	}
	  }
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_Computed_field_change.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_Computed_field_change */

// GRC pass scene and filter list
int Cmiss_graphic_get_visible_graphics_object_range(
	struct Cmiss_graphic *graphic,void *graphics_object_range_void)
{
	int return_code;
	struct Graphics_object_range_struct *graphics_object_range =
		(struct Graphics_object_range_struct *)graphics_object_range_void;
	
	ENTER(Cmiss_graphic_get_visible_graphics_object_range);

	if (graphic && graphics_object_range)
	{
		if (graphic->graphics_object &&
				Cmiss_scene_shows_graphic(graphics_object_range->scene, graphic))
		{
			return_code=get_graphics_object_range(graphic->graphics_object,
				graphics_object_range_void);
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_visible_graphics_object_range.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_get_visible_graphics_object_range */

struct GT_object *Cmiss_graphic_get_graphics_object(
	struct Cmiss_graphic *graphic)
{
	struct GT_object *graphics_object
	
	ENTER(Cmiss_graphic_get_graphics_object);
	if (graphic)
	{
		graphics_object=graphic->graphics_object;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_graphics_object.  Invalid argument(s)");
		graphics_object=(struct GT_object *)NULL;
	}
	LEAVE;

	return (graphics_object);
} /* Cmiss_graphic_get_graphics_object */

int Cmiss_graphic_type_uses_dimension(
	enum Cmiss_graphic_type graphic_type, int dimension)
{
	int return_code;

	ENTER(Cmiss_graphic_type_uses_dimension);
	switch (graphic_type)
	{
		case CMISS_GRAPHIC_NODE_POINTS:
		case CMISS_GRAPHIC_DATA_POINTS:
		case CMISS_GRAPHIC_STATIC:
		{
			return_code = ((-1 == dimension) || (0 == dimension));
		} break;
		case CMISS_GRAPHIC_LINES:
		case CMISS_GRAPHIC_CYLINDERS:
		{
			return_code = ((-1 == dimension) || (1 == dimension));
		} break;
		case CMISS_GRAPHIC_SURFACES:
		{
			return_code = ((-1 == dimension) || (2 == dimension));
		} break;
		case CMISS_GRAPHIC_VOLUMES:
		{
			return_code = ((-1 == dimension) || (3 == dimension));
		} break;
		case CMISS_GRAPHIC_STREAMLINES:
		{
			return_code = ((-1 == dimension) || (2 == dimension) || 
				(3 == dimension));
		} break;
		case CMISS_GRAPHIC_ELEMENT_POINTS:
		case CMISS_GRAPHIC_ISO_SURFACES:
		{
			return_code = ((-1 == dimension) ||
				(1 == dimension) || (2 == dimension) || (3 == dimension));
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_graphic_type_uses_dimension.  Unknown graphic type");
			return_code = 0;
		} break;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_type_uses_dimension */

enum Graphics_select_mode Cmiss_graphic_get_select_mode(
	struct Cmiss_graphic *graphic)
{
	enum Graphics_select_mode select_mode;

	ENTER(Cmiss_graphic_get_select_mode);
	if (graphic)
	{
		select_mode = graphic->select_mode;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_select_mode.  Invalid argument(s)");
		select_mode = GRAPHICS_NO_SELECT;
	}
	LEAVE;

	return (select_mode);
} /* Cmiss_graphic_get_select_mode */


int Cmiss_graphic_set_select_mode(struct Cmiss_graphic *graphic,
	enum Graphics_select_mode select_mode)
{
	int return_code;

	ENTER(Cmiss_graphic_set_select_mode);
	if (graphic)
	{
		graphic->select_mode = select_mode;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_select_mode.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_set_select_mode */

struct Graphical_material *Cmiss_graphic_get_material(
	struct Cmiss_graphic *graphic)
{
	struct Graphical_material *material;

	ENTER(Cmiss_graphic_get_material);
	if (graphic)
	{
		material=graphic->material;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_material.  Invalid argument(s)");
		material=(struct Graphical_material *)NULL;
	}
	LEAVE;

	return (material);
} /* Cmiss_graphic_get_material */

struct Graphical_material *Cmiss_graphic_get_selected_material(
	struct Cmiss_graphic *graphic)
{
	struct Graphical_material *selected_material;

	ENTER(Cmiss_graphic_get_selected_material);
	if (graphic)
	{
		selected_material=graphic->selected_material;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_selected_material.  Invalid argument(s)");
		selected_material=(struct Graphical_material *)NULL;
	}
	LEAVE;

	return (selected_material);
} /* Cmiss_graphic_get_selected_material */


int Cmiss_graphic_set_data_spectrum_parameters(
	struct Cmiss_graphic *graphic,struct Computed_field *data_field,
	struct Spectrum *spectrum)
{
	int return_code;

	ENTER(Cmiss_graphic_set_data_spectrum_parameters);
	if (graphic&&((!data_field)||spectrum)&&
		(CMISS_GRAPHIC_STREAMLINES != graphic->graphic_type))
	{
		return_code=1;
		REACCESS(Computed_field)(&(graphic->data_field),data_field);
		if (!data_field)
		{
			/* don't want graphic accessing spectrum when not using it: */
			spectrum=(struct Spectrum *)NULL;
		}
		REACCESS(Spectrum)(&(graphic->spectrum),spectrum);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_data_spectrum_parameters.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_set_data_spectrum_parameters */

int Cmiss_graphic_set_data_spectrum_parameters_streamlines(
	struct Cmiss_graphic *graphic,
	enum Streamline_data_type streamline_data_type,
	struct Computed_field *data_field,struct Spectrum *spectrum)
{
	int return_code;

	ENTER(Cmiss_graphic_set_data_spectrum_parameters_streamlines);
	if (graphic&&((STREAM_FIELD_SCALAR!=streamline_data_type)||data_field)&&
		((STREAM_NO_DATA==streamline_data_type)||spectrum)&&
		(CMISS_GRAPHIC_STREAMLINES==graphic->graphic_type))
	{
		return_code=1;
		graphic->streamline_data_type=streamline_data_type;
		if (STREAM_FIELD_SCALAR!=streamline_data_type)
		{
			/* don't want graphic accessing data_field when not using it: */
			data_field=(struct Computed_field *)NULL;
		}
		REACCESS(Computed_field)(&(graphic->data_field),data_field);
		if (STREAM_NO_DATA==streamline_data_type)
		{
			/* don't want graphic accessing spectrum when not using it: */
			spectrum=(struct Spectrum *)NULL;
		}
		REACCESS(Spectrum)(&(graphic->spectrum),spectrum);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_data_spectrum_parameters_streamlines.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_set_data_spectrum_parameters_streamlines */

int Cmiss_graphic_set_render_type(
	struct Cmiss_graphic *graphic, enum Render_type render_type)
{
	int return_code;

	ENTER(Cmiss_graphic_set_render_type);
	if (graphic)
	{
		return_code = 1;
		if (graphic->render_type != render_type)
		{
			graphic->render_type = render_type;
			/* This will only affect the external API as it is modifying 
				 the original graphic with a graphics object. */
			if (graphic->graphics_object)
			{
				DEACCESS(GT_object)(&(graphic->graphics_object));
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_render_type.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);

} /* Cmiss_graphic_set_render_type */

int Cmiss_graphic_set_glyph_parameters(
	struct Cmiss_graphic *graphic,
	struct GT_object *glyph, enum Graphic_glyph_scaling_mode glyph_scaling_mode,
	Triple glyph_centre, Triple glyph_size,
	struct Computed_field *orientation_scale_field, Triple glyph_scale_factors,
	struct Computed_field *variable_scale_field)
{
	int return_code;

	ENTER(Cmiss_graphic_set_glyph_parameters);
	if (graphic && glyph && glyph_centre && glyph_size &&
		((CMISS_GRAPHIC_NODE_POINTS==graphic->graphic_type)||
			(CMISS_GRAPHIC_DATA_POINTS==graphic->graphic_type)||
			(CMISS_GRAPHIC_ELEMENT_POINTS==graphic->graphic_type) ||
			(CMISS_GRAPHIC_STATIC==graphic->graphic_type))&&
		((!orientation_scale_field) || Computed_field_is_orientation_scale_capable(
			orientation_scale_field,(void *)NULL)) && glyph_scale_factors &&
		((!variable_scale_field) || Computed_field_has_up_to_3_numerical_components(
			variable_scale_field,(void *)NULL)))
	{
		if (graphic->glyph)
		{
			GT_object_remove_callback(graphic->glyph,
				Cmiss_graphic_glyph_change, (void *)graphic);
		}
		REACCESS(GT_object)(&(graphic->glyph),glyph);
		GT_object_add_callback(graphic->glyph, Cmiss_graphic_glyph_change,
				(void *)graphic);
		graphic->glyph_scaling_mode = glyph_scaling_mode;
		graphic->glyph_centre[0] = glyph_centre[0];
		graphic->glyph_centre[1] = glyph_centre[1];
		graphic->glyph_centre[2] = glyph_centre[2];
		graphic->glyph_size[0] = glyph_size[0];
		graphic->glyph_size[1] = glyph_size[1];
		graphic->glyph_size[2] = glyph_size[2];
		REACCESS(Computed_field)(&(graphic->orientation_scale_field),
			orientation_scale_field);
		graphic->glyph_scale_factors[0]=glyph_scale_factors[0];
		graphic->glyph_scale_factors[1]=glyph_scale_factors[1];
		graphic->glyph_scale_factors[2]=glyph_scale_factors[2];
		REACCESS(Computed_field)(&(graphic->variable_scale_field),
			variable_scale_field);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_glyph_parameters.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_set_glyph_parameters */

int Cmiss_graphic_get_iso_surface_parameters(
	struct Cmiss_graphic *graphic,struct Computed_field **iso_scalar_field,
	int *number_of_iso_values, double **iso_values,
	double *first_iso_value, double *last_iso_value, 
	double *decimation_threshold)
{
	int i, return_code;

	ENTER(Cmiss_graphic_get_iso_surface_parameters);
	if (graphic&&iso_scalar_field&&number_of_iso_values&&iso_values&&
		decimation_threshold&&
		(CMISS_GRAPHIC_ISO_SURFACES==graphic->graphic_type))
	{
		*iso_scalar_field=graphic->iso_scalar_field;
		*decimation_threshold=graphic->decimation_threshold;
		if (0 < graphic->number_of_iso_values)
		{
			if (graphic->iso_values)
			{
				if (ALLOCATE(*iso_values, double, graphic->number_of_iso_values))
				{
					for (i = 0 ; i < graphic->number_of_iso_values ; i++)
					{
						(*iso_values)[i] = graphic->iso_values[i];
					}
					*number_of_iso_values = graphic->number_of_iso_values;
					return_code = 1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Cmiss_graphic_get_iso_surface_parameters.  "
						"Could not allocate memory.");
					return_code=0;
				}
			}
			else
			{
				*iso_values = (double *)NULL;
				*number_of_iso_values = graphic->number_of_iso_values;
				*first_iso_value = graphic->first_iso_value;
				*last_iso_value = graphic->last_iso_value;
				return_code = 1;
			}
		}
		else
		{
			*number_of_iso_values=graphic->number_of_iso_values;
			*iso_values=(double *)NULL;
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_iso_surface_parameters.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_get_iso_surface_parameters */

int Cmiss_graphic_set_iso_surface_parameters(
	struct Cmiss_graphic *graphic,struct Computed_field *iso_scalar_field,
	int number_of_iso_values, double *iso_values,
	double first_iso_value, double last_iso_value,
	double decimation_threshold)
{
	int i, return_code;

	ENTER(Cmiss_graphic_set_iso_surface_parameters);
	if (graphic&&iso_scalar_field&&
		(1==Computed_field_get_number_of_components(iso_scalar_field))&&
		(CMISS_GRAPHIC_ISO_SURFACES==graphic->graphic_type))
	{
		return_code=1;
		REACCESS(Computed_field)(&(graphic->iso_scalar_field),iso_scalar_field);
		graphic->decimation_threshold = decimation_threshold;
		if (0 < number_of_iso_values)
		{
			if (iso_values)
			{
				double *temp_values;
				if (REALLOCATE(temp_values, graphic->iso_values, double,
						number_of_iso_values))
				{
					graphic->iso_values = temp_values;
					graphic->number_of_iso_values = number_of_iso_values;
					for (i = 0 ; i < number_of_iso_values ; i++)
					{
						graphic->iso_values[i] = iso_values[i];
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"cmiss_graphic_set_iso_surface_parameters.  "
						"Could not allocate memory.");
					return_code=0;
				}
			}
			else
			{
				if (graphic->iso_values)
				{
					DEALLOCATE(graphic->iso_values);
				}
				graphic->number_of_iso_values = number_of_iso_values;
				graphic->first_iso_value = first_iso_value;
				graphic->last_iso_value = last_iso_value;
			}
		}
		else
		{
			if (graphic->iso_values)
			{
				DEALLOCATE(graphic->iso_values);
				graphic->iso_values = (double *)NULL;
			}
			graphic->number_of_iso_values = 0;
			graphic->first_iso_value = 0;
			graphic->last_iso_value = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_iso_surface_parameters.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_set_iso_surface_parameters */

int Cmiss_graphic_set_radius_parameters(
	struct Cmiss_graphic *graphic,float constant_radius,
	float radius_scale_factor,struct Computed_field *radius_scalar_field)
{
	int return_code;

	ENTER(Cmiss_graphic_set_radius_parameters);
	if (graphic&&(CMISS_GRAPHIC_CYLINDERS==graphic->graphic_type)&&
		((!radius_scalar_field)||
			(1==Computed_field_get_number_of_components(radius_scalar_field))))
	{
		return_code=1;
		graphic->constant_radius=constant_radius;
		graphic->radius_scale_factor=radius_scale_factor;
		REACCESS(Computed_field)(&(graphic->radius_scalar_field),
			radius_scalar_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_radius_parameters.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}

int Cmiss_graphic_get_streamline_parameters(
	struct Cmiss_graphic *graphic,enum Streamline_type *streamline_type,
	struct Computed_field **stream_vector_field,int *reverse_track,
	float *streamline_length,float *streamline_width)
{
	int return_code;

	ENTER(Cmiss_graphic_get_streamline_parameters);
	if (graphic&&streamline_type&&stream_vector_field&&reverse_track&&
		streamline_length&&streamline_width&&
		(CMISS_GRAPHIC_STREAMLINES==graphic->graphic_type))
	{
		*streamline_type=graphic->streamline_type;
		*stream_vector_field=graphic->stream_vector_field;
		*reverse_track=graphic->reverse_track;
		*streamline_length=graphic->streamline_length;
		*streamline_width=graphic->streamline_width;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_streamline_parameters.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_get_streamline_parameters */

int Cmiss_graphic_set_streamline_parameters(
	struct Cmiss_graphic *graphic,enum Streamline_type streamline_type,
	struct Computed_field *stream_vector_field,int reverse_track,
	float streamline_length,float streamline_width)
{
	int return_code;

	ENTER(Cmiss_graphic_set_streamline_parameters);
	if (graphic&&stream_vector_field&&
		(CMISS_GRAPHIC_STREAMLINES==graphic->graphic_type))
	{
		graphic->streamline_type=streamline_type;
		REACCESS(Computed_field)(&(graphic->stream_vector_field),
			stream_vector_field);
		graphic->reverse_track=reverse_track;
		graphic->streamline_length=streamline_length;
		graphic->streamline_width=streamline_width;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_streamline_parameters.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_set_streamline_parameters */

int Cmiss_graphic_set_use_element_type(
	struct Cmiss_graphic *graphic,enum Use_element_type use_element_type)
{
	int return_code;

	ENTER(Cmiss_graphic_set_use_element_type);
	if (graphic&&((CMISS_GRAPHIC_ELEMENT_POINTS==graphic->graphic_type)||
		(CMISS_GRAPHIC_ISO_SURFACES==graphic->graphic_type)))
	{
		graphic->use_element_type=use_element_type;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_use_element_type.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_set_use_element_type */

int Cmiss_graphic_get_xi_discretization(
	struct Cmiss_graphic *graphic,
	enum Xi_discretization_mode *xi_discretization_mode,
	struct Computed_field **xi_point_density_field)
{
	int return_code;

	ENTER(Cmiss_graphic_get_xi_discretization);
	if (graphic &&
		(CMISS_GRAPHIC_ELEMENT_POINTS == graphic->graphic_type))
	{
		if (xi_discretization_mode)
		{
			*xi_discretization_mode = graphic->xi_discretization_mode;
		}
		if (xi_point_density_field)
		{
			*xi_point_density_field = graphic->xi_point_density_field;
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_xi_discretization.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_get_xi_discretization */

int Cmiss_graphic_set_xi_discretization(
	struct Cmiss_graphic *graphic,
	enum Xi_discretization_mode xi_discretization_mode,
	struct Computed_field *xi_point_density_field)
{
	int need_density_field, return_code;

	ENTER(Cmiss_graphic_set_xi_discretization);
	need_density_field =
		(XI_DISCRETIZATION_CELL_DENSITY == xi_discretization_mode) ||
		(XI_DISCRETIZATION_CELL_POISSON == xi_discretization_mode);

	if (graphic && ((!need_density_field && !xi_point_density_field) ||
		(need_density_field && xi_point_density_field &&
			Computed_field_is_scalar(xi_point_density_field, (void *)NULL))))
	{
		graphic->xi_discretization_mode = xi_discretization_mode;
		REACCESS(Computed_field)(&(graphic->xi_point_density_field),
			xi_point_density_field);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_xi_discretization.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_set_xi_discretization */

int Cmiss_graphic_set_visibility(struct Cmiss_graphic *graphic,
	int visibility)
{
	int return_code;

	ENTER(Cmiss_graphic_set_visibility);
	if (graphic)
	{
		return_code=1;
		if (visibility != graphic->visibility)
		{
			graphic->visibility = visibility;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_visibility.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_set_visibility */

int Cmiss_graphic_default_coordinate_field_change(
	struct Cmiss_graphic *graphic,void *dummy_void)
{
	int return_code;
	
	ENTER(Cmiss_graphic_default_coordinate_field_change);
	USE_PARAMETER(dummy_void);
	if (graphic)
	{
		if ((struct Computed_field *)NULL == graphic->coordinate_field)
		{
			Cmiss_graphic_clear_graphics(graphic);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_default_coordinate_field_change.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_default_coordinate_field_change */

int Cmiss_graphic_element_discretization_change(
	struct Cmiss_graphic *graphic,void *dummy_void)
{
	int return_code;
	
	ENTER(Cmiss_graphic_element_discretization_change);
	USE_PARAMETER(dummy_void);
	if (graphic)
	{
		if ((Cmiss_graphic_type_uses_dimension(graphic->graphic_type, 1) ||
			Cmiss_graphic_type_uses_dimension(graphic->graphic_type, 2) ||
			Cmiss_graphic_type_uses_dimension(graphic->graphic_type, 3)) &&
			(CMISS_GRAPHIC_ELEMENT_POINTS != graphic->graphic_type) &&
			(CMISS_GRAPHIC_STREAMLINES != graphic->graphic_type) &&
			(CMISS_GRAPHIC_VOLUMES != graphic->graphic_type))
		{
			Cmiss_graphic_clear_graphics(graphic);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_element_discretization_change.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_element_discretization_change */

int Cmiss_graphic_set_discretization(
  struct Cmiss_graphic *graphic, struct Element_discretization *discretization)
{
	int return_code;

	ENTER(Cmiss_graphic_set_general_discretization);
	if (graphic && discretization)
	{
		if ((graphic->discretization.number_in_xi1 != 
				discretization->number_in_xi1) ||
			(graphic->discretization.number_in_xi2 !=
				discretization->number_in_xi2) || 
			(graphic->discretization.number_in_xi3 !=
				discretization->number_in_xi3))
		{
			graphic->discretization.number_in_xi1=discretization->number_in_xi1;
			graphic->discretization.number_in_xi2=discretization->number_in_xi2;
			graphic->discretization.number_in_xi3=discretization->number_in_xi3;
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_discretization.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_set_discretization */

int Cmiss_graphic_set_general_element_discretization(
	struct Cmiss_graphic *graphic, void *discretization_void)
{
	int return_code;
	struct Element_discretization *discretization;

	ENTER(Cmiss_graphic_set_general_discretization);
	if (graphic&& (discretization = (struct Element_discretization *)discretization_void))
	{
	  if (Cmiss_graphic_set_discretization(graphic, discretization))
	  {
	    Cmiss_graphic_element_discretization_change(graphic, (void *)NULL);
	  }
	  return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_discretization.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_set_discretization */

int Cmiss_graphic_circle_discretization_change(
	struct Cmiss_graphic *graphic,void *dummy_void)
{
	int return_code;
	
	ENTER(Cmiss_graphic_circle_discretization_change);
	USE_PARAMETER(dummy_void);
	if (graphic)
	{
		if (CMISS_GRAPHIC_CYLINDERS == graphic->graphic_type)
		{
			Cmiss_graphic_clear_graphics(graphic);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_circle_discretization_change.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_circle_discretization_change */

int Cmiss_graphic_copy_without_graphics_object(
	struct Cmiss_graphic *destination, struct Cmiss_graphic *source)
{
	int return_code;

	ENTER(Cmiss_graphic_copy_without_graphics_object);
	if (destination && source && (destination != source))
	{
		return_code = 1;
		destination->position = source->position;

		if (destination->name)
		{
			DEALLOCATE(destination->name);
		}
		if (source->name && ALLOCATE(destination->name, char, 
			strlen(source->name) + 1))
		{
			strcpy((char *)destination->name, source->name);
		}

		/* copy geometry graphic */
		/* for all graphic types */
		destination->graphic_type=source->graphic_type;
		REACCESS(Computed_field)(&(destination->coordinate_field),
			source->coordinate_field);
		destination->select_mode=source->select_mode;
		/* for surfaces only at the moment */
		REACCESS(Computed_field)(&(destination->texture_coordinate_field),
			source->texture_coordinate_field);
		/* for 1-D and 2-D elements only */
		destination->exterior=source->exterior;
		destination->face=source->face;
		destination->rendition=source->rendition;
		/* for cylinders only */
		if (CMISS_GRAPHIC_CYLINDERS==source->graphic_type)
		{
			Cmiss_graphic_set_radius_parameters(destination,
				source->constant_radius,source->radius_scale_factor,
				source->radius_scalar_field);
		}
		else
		{
			if (destination->radius_scalar_field)
			{
				DEACCESS(Computed_field)(&destination->radius_scalar_field);
			}
		}
		/* for iso_surfaces only */
		if (CMISS_GRAPHIC_ISO_SURFACES==source->graphic_type)
		{
			Cmiss_graphic_set_iso_surface_parameters(destination,
				source->iso_scalar_field,source->number_of_iso_values,
				source->iso_values, 
				source->first_iso_value, source->last_iso_value,
				source->decimation_threshold);
		}
		else
		{
			if (destination->iso_scalar_field)
			{
				DEACCESS(Computed_field)(&destination->iso_scalar_field);
			}
		}
		/* for node_points, data_points and element_points only */
		if ((CMISS_GRAPHIC_NODE_POINTS==source->graphic_type)||
			(CMISS_GRAPHIC_DATA_POINTS==source->graphic_type)||
			(CMISS_GRAPHIC_ELEMENT_POINTS==source->graphic_type)||
			(CMISS_GRAPHIC_STATIC==source->graphic_type))
		{
			Cmiss_graphic_set_glyph_parameters(destination,
				source->glyph, source->glyph_scaling_mode,
				source->glyph_centre, source->glyph_size,
				source->orientation_scale_field, source->glyph_scale_factors,
				source->variable_scale_field);
		}
		else
		{
			if (destination->glyph)
			{
				GT_object_remove_callback(destination->glyph, 
					Cmiss_graphic_glyph_change, (void *)destination);
				DEACCESS(GT_object)(&(destination->glyph));
			}
			if (destination->orientation_scale_field)
			{
				DEACCESS(Computed_field)(&destination->orientation_scale_field);
			}
			if (destination->variable_scale_field)
			{
				DEACCESS(Computed_field)(&destination->variable_scale_field);
			}
		}
		
		if (CMISS_GRAPHIC_STATIC==source->graphic_type)
		{
			destination->overlay_flag = source->overlay_flag;
			destination->overlay_order = source->overlay_order;
		}

		REACCESS(Computed_field)(&(destination->label_field),source->label_field);
		REACCESS(Computed_field)(&(destination->visibility_field),source->visibility_field);
		/* for element_points and iso_surfaces */
		destination->use_element_type=source->use_element_type;
		/* for element_points only */
		destination->xi_discretization_mode=source->xi_discretization_mode;
		REACCESS(Computed_field)(&(destination->xi_point_density_field),
			source->xi_point_density_field);
		destination->discretization.number_in_xi1=
			source->discretization.number_in_xi1;
		destination->discretization.number_in_xi2=
			source->discretization.number_in_xi2;
		destination->discretization.number_in_xi3=
			source->discretization.number_in_xi3;
		destination->circle_discretization=source->circle_discretization;
		REACCESS(FE_field)(&(destination->native_discretization_field),
			source->native_discretization_field);
		REACCESS(Computed_field)(&(destination->label_density_field),source->label_density_field);
		/* for volumes only */
		REACCESS(VT_volume_texture)(&(destination->volume_texture),
			source->volume_texture);
		REACCESS(Computed_field)(&(destination->displacement_map_field),
			source->displacement_map_field);
		/* for graphic starting in a particular element */
		REACCESS(FE_element)(&(destination->seed_element),
			source->seed_element);
		/* for graphic requiring an exact xi location */
		destination->seed_xi[0]=source->seed_xi[0];
		destination->seed_xi[1]=source->seed_xi[1];
		destination->seed_xi[2]=source->seed_xi[2];
		/* for streamlines only */
		destination->streamline_type=source->streamline_type;
		REACCESS(Computed_field)(&(destination->stream_vector_field),
			source->stream_vector_field);
		destination->reverse_track=source->reverse_track;
		destination->streamline_length=source->streamline_length;
		destination->streamline_width=source->streamline_width;
		REACCESS(FE_region)(&destination->seed_node_region, 
			source->seed_node_region);
		if (destination->seed_node_region_path)
		{
			DEALLOCATE(destination->seed_node_region_path);
		}
		if (source->seed_node_region_path)
		{
			destination->seed_node_region_path = duplicate_string( 
				source->seed_node_region_path);
		}
		else
		{
			destination->seed_node_region_path = (char *)NULL;
		}
		REACCESS(Computed_field)(&(destination->seed_node_coordinate_field),
			source->seed_node_coordinate_field);		

		/* copy appearance graphic */
		/* for all graphic types */
		destination->visibility=source->visibility;
		destination->line_width = source->line_width;
		REACCESS(Graphical_material)(&(destination->material),source->material);
		REACCESS(Graphical_material)(&(destination->secondary_material),
			source->secondary_material);
		Cmiss_graphic_set_render_type(destination,source->render_type);
		if (CMISS_GRAPHIC_STREAMLINES==source->graphic_type)
		{
			Cmiss_graphic_set_data_spectrum_parameters_streamlines(destination,
				source->streamline_data_type,source->data_field,source->spectrum);
		}
		else
		{
			Cmiss_graphic_set_data_spectrum_parameters(destination,
				source->data_field,source->spectrum);
		}
		REACCESS(Graphical_material)(&(destination->selected_material),
			source->selected_material);
		destination->autorange_spectrum_flag = source->autorange_spectrum_flag;
		REACCESS(Graphics_font)(&(destination->font), source->font);

		/* ensure destination graphics object is cleared */
		REACCESS(GT_object)(&(destination->graphics_object),
			(struct GT_object *)NULL);
		destination->graphics_changed = 1;
		destination->selected_graphics_changed = 1;

		if (!return_code)
		{
			display_message(ERROR_MESSAGE,"Cmiss_graphic_copy_without_graphics_object.  "
				"Error copying graphic");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_graphic_copy_without_graphics_object.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* cmiss_graphic_copy_without_graphics_object */

int Cmiss_graphic_has_name(struct Cmiss_graphic *graphic,
	void *name_void)
{
	char *name, temp_name[30];
	int return_code;

	ENTER(Cmiss_graphic_has_name);
	if (graphic && (name=(char *)name_void))
	{
		return_code = 0;
		if (graphic->name)
		{
			return_code=!strcmp(name,graphic->name);
		}
		if (!return_code)
		{
			/* Compare with number if the graphic
			 has no name or the name didn't match */
			sprintf(temp_name, "%d", graphic->position);
			return_code=!strcmp(name,temp_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmiss_graphic_has_name.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_has_name */

static int FE_element_as_graphics_name_is_removed_or_modified(
	int graphics_name, void *data_void)
{
	int return_code;
	struct CM_element_information cm;
	struct FE_element *element;
	struct Cmiss_graphic_FE_region_change_data *data;

	ENTER(FE_element_as_graphics_name_is_removed_or_modified);
	return_code = 0;
	if (NULL != (data = (struct Cmiss_graphic_FE_region_change_data *)data_void))
	{
		if (CM_element_information_from_graphics_name(&cm, graphics_name))
		{
			if (NULL != (element = FE_region_get_FE_element_from_identifier(data->fe_region,
						&cm)))
			{
				return_code = FE_element_or_parent_changed(element,
					data->fe_element_changes, data->fe_node_changes);
			}
			else
			{
				/* must have been removed or never in FE_region */
				return_code = 1;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_element_as_graphics_name_is_removed_or_modified.  "
			"Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* FE_element_as_graphics_name_is_removed_or_modified */

int Cmiss_graphic_FE_region_change(
	struct Cmiss_graphic *graphic, void *data_void)
{
	int fe_field_related_object_change, return_code;
	struct Cmiss_graphic_FE_region_change_data *data;
	
	ENTER(Cmiss_graphic_FE_region_change);
	if (graphic &&
		(data = (struct Cmiss_graphic_FE_region_change_data *)data_void))
	{
		if (graphic->graphics_object)
		{
			switch (graphic->graphic_type)
			{
				case CMISS_GRAPHIC_DATA_POINTS:
				{
					/* only affected by data_FE_region */
				} break;
				case CMISS_GRAPHIC_NODE_POINTS:
				{
					/* must always rebuild if identifiers changed */
					if ((data->fe_node_change_summary &
						CHANGE_LOG_OBJECT_IDENTIFIER_CHANGED(FE_node)) || 
						(Cmiss_graphic_uses_changed_FE_field(graphic,
							data->fe_field_changes) && (
								(data->fe_field_change_summary & (
									CHANGE_LOG_OBJECT_IDENTIFIER_CHANGED(FE_field) |
									CHANGE_LOG_OBJECT_NOT_IDENTIFIER_CHANGED(FE_field))) ||
								((data->fe_field_change_summary &
									CHANGE_LOG_RELATED_OBJECT_CHANGED(FE_field)) &&
									(0 < data->number_of_fe_node_changes)))))
					{
						/* currently node points are always rebuilt from scratch */
						GT_object_remove_primitives_at_time(graphic->graphics_object,
							data->time,
							(GT_object_primitive_object_name_conditional_function *)NULL,
								(void *)NULL);
						graphic->graphics_changed = 1;
						graphic->selected_graphics_changed = 1;
						data->graphics_changed = 1;
					}
				} break;
				default:
				{
					fe_field_related_object_change =
						CHANGE_LOG_OBJECT_UNCHANGED(FE_field);
					/* must always rebuild if identifiers changed */
					if ((data->fe_element_change_summary &
						CHANGE_LOG_OBJECT_IDENTIFIER_CHANGED(FE_element)) ||
						(Cmiss_graphic_uses_changed_FE_field(graphic,
							data->fe_field_changes) && (
								(data->fe_field_change_summary & (
									CHANGE_LOG_OBJECT_IDENTIFIER_CHANGED(FE_field) |
									CHANGE_LOG_OBJECT_NOT_IDENTIFIER_CHANGED(FE_field))) ||
								(fe_field_related_object_change = (
									(data->fe_field_change_summary &
										CHANGE_LOG_RELATED_OBJECT_CHANGED(FE_field)) && (
									(0 < data->number_of_fe_node_changes) ||
									(0 < data->number_of_fe_element_changes)))))))
					{
						if (fe_field_related_object_change && (
							((data->number_of_fe_node_changes*2) <
								FE_region_get_number_of_FE_nodes(data->fe_region)) &&
							((data->number_of_fe_element_changes*4) <
								FE_region_get_number_of_FE_elements(data->fe_region))))
						{
							/* partial rebuild for few node/element field changes */
							GT_object_remove_primitives_at_time(graphic->graphics_object,
								data->time, FE_element_as_graphics_name_is_removed_or_modified,
								data_void);
						}
						else
						{
							/* full rebuild for changed identifiers, FE_field definition
								 changes or many node/element field changes */
							GT_object_remove_primitives_at_time(graphic->graphics_object,
								data->time,
								(GT_object_primitive_object_name_conditional_function *)NULL,
								(void *)NULL);
						}
						graphic->graphics_changed = 1;
						graphic->selected_graphics_changed = 1;
						data->graphics_changed = 1;
					}
				} break;
			}
		}
		else
		{
			/* Graphics have definitely changed as they have not been built yet */
			data->graphics_changed = 1;
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_FE_region_change.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_FE_region_change */

int Cmiss_graphic_data_FE_region_change(
	struct Cmiss_graphic *graphic, void *data_void)
{
	int return_code;
	struct Cmiss_graphic_FE_region_change_data *data;
	
	ENTER(Cmiss_graphic_data_FE_region_change);
	if (graphic &&
		(data = (struct Cmiss_graphic_FE_region_change_data *)data_void))
	{
		if (graphic->graphics_object)
		{
			switch (graphic->graphic_type)
			{
				case CMISS_GRAPHIC_DATA_POINTS:
				{
					/* rebuild on any changes */
					if ((0 < data->number_of_fe_node_changes) &&
						Cmiss_graphic_uses_changed_FE_field(graphic,
							data->fe_field_changes))
					{
						/* currently node points are always rebuilt from scratch */
						GT_object_remove_primitives_at_time(graphic->graphics_object,
							data->time,
							(GT_object_primitive_object_name_conditional_function *)NULL,
							(void *)NULL);
						graphic->graphics_changed = 1;
						graphic->selected_graphics_changed = 1;
						data->graphics_changed = 1;
					}
				} break;
				default:
				{
					/* do nothing */
				} break;
			}
		}
		else
		{
			/* Graphics have definitely changed as they have not been built yet */
			data->graphics_changed = 1;
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_data_FE_region_change.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_data_FE_region_change */

int gfx_modify_rendition_surfaces(struct Parse_state *state,
	void *modify_rendition_data_void,void *rendition_command_data_void)
{
	const char *render_type_string,*select_mode_string,**valid_strings;
	enum Graphics_select_mode select_mode;
	enum Render_type render_type;
	int number_of_valid_strings,return_code, visibility;
	struct Modify_rendition_data *modify_rendition_data;
	struct Cmiss_graphic *graphic;
	struct Rendition_command_data *rendition_command_data;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_data_field_data, set_texture_coordinate_field_data;

	ENTER(gfx_modify_rendition_surfaces);
	if (state)
	{
		if (NULL != (rendition_command_data=(struct Rendition_command_data *)
				rendition_command_data_void))
		{
			if (NULL != (modify_rendition_data=
					(struct Modify_rendition_data *)modify_rendition_data_void))
			{
				/* create the Cmiss_graphic: */
				graphic = CREATE(Cmiss_graphic)(CMISS_GRAPHIC_SURFACES);
				/* if we are specifying the name then copy the existings graphic values so
					we can modify from these rather than the default */
				if (modify_rendition_data->graphic)
				{
					if (CMISS_GRAPHIC_SURFACES ==modify_rendition_data->graphic->graphic_type)
					{
						Cmiss_graphic_copy_without_graphics_object(graphic, modify_rendition_data->graphic); 
					}
					DEACCESS(Cmiss_graphic)(&modify_rendition_data->graphic);
				}
				else
				{
					Cmiss_graphic_set_native_discretization_field(
						graphic, modify_rendition_data->native_discretization_field);
					Cmiss_graphic_set_coordinate_field(
						graphic, modify_rendition_data->default_coordinate_field);
					Cmiss_graphic_set_circle_discretization(
						graphic, modify_rendition_data->circle_discretization);
					Cmiss_graphic_set_discretization(
						graphic, &(modify_rendition_data->element_discretization));
				}
				if (graphic)
				{
					/* access since deaccessed in gfx_modify_rendition */
					modify_rendition_data->graphic = ACCESS(Cmiss_graphic)(graphic);
					/* Set up the coordinate_field */
					if (!graphic->coordinate_field)
					{
						struct Computed_field *computed_field;
						struct FE_field *fe_field;
						struct FE_region *fe_region = 
							Cmiss_region_get_FE_region(rendition_command_data->region);
						struct FE_region *data_region = 
							FE_region_get_data_FE_region(fe_region);
						if (FE_region_get_default_coordinate_FE_field(fe_region, &fe_field) ||
							FE_region_get_default_coordinate_FE_field(data_region, &fe_field))
						{
							if (NULL != (computed_field = FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
										Computed_field_is_read_only_with_fe_field,
										(void *)fe_field, rendition_command_data->computed_field_manager)))
							{
								graphic->coordinate_field = 
									ACCESS(Computed_field)(computed_field);
							}
						}
					}
					/* set essential parameters not set by CREATE function */
					if (!Cmiss_graphic_get_material(graphic))
					{
						Cmiss_graphic_set_material(graphic,
							rendition_command_data->default_material);
					}
					if (!Cmiss_graphic_get_selected_material(graphic))
					{
						Cmiss_graphic_set_selected_material(graphic,
							FIND_BY_IDENTIFIER_IN_MANAGER(Graphical_material,name)(
								"default_selected",
								rendition_command_data->graphical_material_manager));
					}
					visibility = graphic->visibility;
					option_table=CREATE(Option_table)();
					/* as */
					Option_table_add_entry(option_table,"as",&(graphic->name),
						(void *)1,set_name);
					/* coordinate */
					set_coordinate_field_data.computed_field_manager=
						rendition_command_data->computed_field_manager;
					set_coordinate_field_data.conditional_function=
						Computed_field_has_up_to_3_numerical_components;
					set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
					Option_table_add_entry(option_table,"coordinate",
						&(graphic->coordinate_field),&set_coordinate_field_data,
						set_Computed_field_conditional);
					/* data */
					set_data_field_data.computed_field_manager=
						rendition_command_data->computed_field_manager;
					set_data_field_data.conditional_function=
						Computed_field_has_numerical_components;
					set_data_field_data.conditional_function_user_data=(void *)NULL;
					Option_table_add_entry(option_table,"data",&(graphic->data_field),
						&set_data_field_data,set_Computed_field_conditional);
					/* delete */
					Option_table_add_entry(option_table,"delete",
						&(modify_rendition_data->delete_flag),NULL,set_char_flag);
					/* discretization */
					Option_table_add_entry(option_table,"discretization",
						&(graphic->discretization),rendition_command_data->user_interface,
						set_Element_discretization);
					/* exterior */
					Option_table_add_entry(option_table,"exterior",&(graphic->exterior),
						NULL,set_char_flag);
					/* face */
					Option_table_add_entry(option_table,"face",&(graphic->face),
						NULL,set_exterior);
					/* material */
					Option_table_add_entry(option_table,"material",&(graphic->material),
						rendition_command_data->graphical_material_manager,
						set_Graphical_material);
					/* position */
					Option_table_add_entry(option_table,"position",
						&(modify_rendition_data->position),NULL,set_int_non_negative);
					/* render_type */
					render_type = graphic->render_type;
					render_type_string = ENUMERATOR_STRING(Render_type)(render_type);
					valid_strings = ENUMERATOR_GET_VALID_STRINGS(Render_type)(
						&number_of_valid_strings,
						(ENUMERATOR_CONDITIONAL_FUNCTION(Render_type) *)NULL, (void *)NULL);
					Option_table_add_enumerator(option_table,number_of_valid_strings,
						valid_strings,&render_type_string);
					DEALLOCATE(valid_strings);
					/* scene */
					Option_table_add_entry(option_table,"scene",
						&(modify_rendition_data->scene),
						rendition_command_data->scene_manager,set_Scene);
					/* select_mode */
					select_mode = Cmiss_graphic_get_select_mode(graphic);
					select_mode_string =
						ENUMERATOR_STRING(Graphics_select_mode)(select_mode);
					valid_strings = ENUMERATOR_GET_VALID_STRINGS(Graphics_select_mode)(
						&number_of_valid_strings,
						(ENUMERATOR_CONDITIONAL_FUNCTION(Graphics_select_mode) *)NULL,
						(void *)NULL);
					Option_table_add_enumerator(option_table,number_of_valid_strings,
						valid_strings,&select_mode_string);
					DEALLOCATE(valid_strings);
					/* selected_material */
					Option_table_add_entry(option_table,"selected_material",
						&(graphic->selected_material),
						rendition_command_data->graphical_material_manager,
						set_Graphical_material);
					/* spectrum */
					Option_table_add_entry(option_table,"spectrum",
						&(graphic->spectrum),rendition_command_data->spectrum_manager,
						set_Spectrum);
					/* texture_coordinates */
					set_texture_coordinate_field_data.computed_field_manager=
						rendition_command_data->computed_field_manager;
					set_texture_coordinate_field_data.conditional_function=
						Computed_field_has_up_to_3_numerical_components;
					set_texture_coordinate_field_data.conditional_function_user_data=
						(void *)NULL;
					Option_table_add_entry(option_table,"texture_coordinates",
						&(graphic->texture_coordinate_field),
						&set_texture_coordinate_field_data,set_Computed_field_conditional);
					/* visible/invisible */
					Option_table_add_switch(option_table, "visible", "invisible", &visibility);
					if ((return_code=Option_table_multi_parse(option_table,state)))
					{
						if (graphic->data_field&&!graphic->spectrum)
						{
							graphic->spectrum=ACCESS(Spectrum)(
								rendition_command_data->default_spectrum);
						}
						graphic->visibility = visibility;
						STRING_TO_ENUMERATOR(Graphics_select_mode)(select_mode_string,
							&select_mode);
						Cmiss_graphic_set_select_mode(graphic, select_mode);
						STRING_TO_ENUMERATOR(Render_type)(render_type_string, &render_type);
						Cmiss_graphic_set_render_type(graphic, render_type);
					}
					DESTROY(Option_table)(&option_table);
					if (!return_code)
					{
						/* parse error, help */
						DEACCESS(Cmiss_graphic)(&(modify_rendition_data->graphic));
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_modify_rendition_surfaces.  Could not create graphic");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_modify_rendition_surfaces.  No modify data");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_modify_rendition_surfaces.  "
				"Missing rendition_command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_modify_rendition_surfaces.  "
			"Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_rendition_surfaces */

int Cmiss_graphic_same_geometry(struct Cmiss_graphic *graphic,
	void *second_graphic_void)
{
	int dimension,i,return_code;
	struct Cmiss_graphic *second_graphic;

	ENTER(Cmiss_graphic_same_geometry);
	if (graphic
		&&(second_graphic=(struct Cmiss_graphic *)second_graphic_void))
	{
		return_code=1;

		/* compare geometry graphic */
		/* for all graphic types */
		if (return_code)
		{
			/* note: different if names are different */
			return_code=
				(graphic->graphic_type==second_graphic->graphic_type)&&
				(graphic->coordinate_field==second_graphic->coordinate_field)&&
				((((char *)NULL==graphic->name)&&((char *)NULL==second_graphic->name))
					||((graphic->name)&&(second_graphic->name)&&
						(0==strcmp(graphic->name,second_graphic->name))))&&
				(graphic->select_mode==second_graphic->select_mode);
		}
		/* for 1-D and 2-D elements only */
		if (return_code)
		{
			dimension=Cmiss_graphic_get_dimension(graphic);
			if ((1==dimension)||(2==dimension))
			{
				return_code=(graphic->exterior == second_graphic->exterior)&&
					(graphic->face == second_graphic->face);
			}
		}
		/* for cylinders only */
		if (return_code&&(CMISS_GRAPHIC_CYLINDERS==graphic->graphic_type))
		{
			return_code=(graphic->constant_radius==second_graphic->constant_radius)
				&&(graphic->radius_scalar_field==second_graphic->radius_scalar_field)
				&&(graphic->radius_scale_factor==second_graphic->radius_scale_factor);
		}
		/* for iso_surfaces only */
		if (return_code&&
			(CMISS_GRAPHIC_ISO_SURFACES==graphic->graphic_type))
		{
			return_code=(graphic->number_of_iso_values==
				second_graphic->number_of_iso_values)&&
				(graphic->decimation_threshold==second_graphic->decimation_threshold)&&
				(graphic->iso_scalar_field==second_graphic->iso_scalar_field);
			if (return_code)
			{
				if (graphic->iso_values)
				{
					if (second_graphic->iso_values)
					{
						i = 0;
						while (return_code && (i < graphic->number_of_iso_values))
						{
							if (graphic->iso_values[i] != second_graphic->iso_values[i])
							{
								return_code = 0;
							}
							i++;
						}
					}
					else
					{
						return_code = 0;
					}
				}
				else
				{
					if (second_graphic->iso_values)
					{
						return_code = 0;
					}
					else
					{
						return_code =
							(graphic->first_iso_value == second_graphic->first_iso_value)
							&& (graphic->last_iso_value == second_graphic->last_iso_value);
					}
				}
			}
		}
		/* for node_points, data_points and element_points only */
		if (return_code&&
			((CMISS_GRAPHIC_NODE_POINTS==graphic->graphic_type)||
				(CMISS_GRAPHIC_DATA_POINTS==graphic->graphic_type)||
				(CMISS_GRAPHIC_ELEMENT_POINTS==graphic->graphic_type)||
				(CMISS_GRAPHIC_STATIC==graphic->graphic_type) ))
		{
			return_code=
				(graphic->glyph==second_graphic->glyph)&&
				(graphic->glyph_scaling_mode==second_graphic->glyph_scaling_mode)&&
				(graphic->glyph_size[0]==second_graphic->glyph_size[0])&&
				(graphic->glyph_size[1]==second_graphic->glyph_size[1])&&
				(graphic->glyph_size[2]==second_graphic->glyph_size[2])&&
				(graphic->glyph_scale_factors[0]==
					second_graphic->glyph_scale_factors[0])&&
				(graphic->glyph_scale_factors[1]==
					second_graphic->glyph_scale_factors[1])&&
				(graphic->glyph_scale_factors[2]==
					second_graphic->glyph_scale_factors[2])&&
				(graphic->glyph_centre[0]==second_graphic->glyph_centre[0])&&
				(graphic->glyph_centre[1]==second_graphic->glyph_centre[1])&&
				(graphic->glyph_centre[2]==second_graphic->glyph_centre[2])&&
				(graphic->orientation_scale_field==
					second_graphic->orientation_scale_field)&&
				(graphic->variable_scale_field==
					second_graphic->variable_scale_field)&&
				(graphic->label_field==second_graphic->label_field)&&
				(graphic->label_density_field==second_graphic->label_density_field)&&
				(graphic->font==second_graphic->font) &&
				(graphic->visibility_field==second_graphic->visibility_field);
		}
		/* for element_points and iso_surfaces */
		if (return_code&&
			((CMISS_GRAPHIC_ELEMENT_POINTS==graphic->graphic_type)||
				(CMISS_GRAPHIC_ISO_SURFACES==graphic->graphic_type)))
		{
			return_code=
				(graphic->use_element_type==second_graphic->use_element_type);
		}
		/* for element_points only */
		if (return_code&&
			(CMISS_GRAPHIC_ELEMENT_POINTS==graphic->graphic_type))
		{
			return_code=
				(graphic->xi_discretization_mode==
					second_graphic->xi_discretization_mode)&&
				(graphic->xi_point_density_field==
					second_graphic->xi_point_density_field)&&
				(graphic->native_discretization_field==
					second_graphic->native_discretization_field)&&
				(graphic->discretization.number_in_xi1==
					second_graphic->discretization.number_in_xi1)&&
				(graphic->discretization.number_in_xi2==
					second_graphic->discretization.number_in_xi2)&&
				(graphic->discretization.number_in_xi3==
					second_graphic->discretization.number_in_xi3);
		}
		/* for volumes only */
		if (return_code&&(CMISS_GRAPHIC_VOLUMES==graphic->graphic_type))
		{
			return_code=
				(graphic->volume_texture==second_graphic->volume_texture)&&
				(graphic->displacement_map_field==
					second_graphic->displacement_map_field)&&
				(graphic->displacement_map_xi_direction==
					second_graphic->displacement_map_xi_direction);
		}
		/* for graphic starting in a particular element */
		if (return_code&&((CMISS_GRAPHIC_VOLUMES==graphic->graphic_type)||
			(CMISS_GRAPHIC_STREAMLINES==graphic->graphic_type)))
		{
			return_code=
				(graphic->seed_element==second_graphic->seed_element);
		}
		/* for graphic requiring an exact xi location */
		if (return_code&&(
			(CMISS_GRAPHIC_ELEMENT_POINTS==graphic->graphic_type)||
			(CMISS_GRAPHIC_STREAMLINES==graphic->graphic_type)))
		{
			return_code=
				(graphic->seed_xi[0]==second_graphic->seed_xi[0])&&
				(graphic->seed_xi[1]==second_graphic->seed_xi[1])&&
				(graphic->seed_xi[2]==second_graphic->seed_xi[2]);
		}
		/* for streamlines only */
		if (return_code&&(CMISS_GRAPHIC_STREAMLINES==graphic->graphic_type))
		{
			return_code=
				(graphic->streamline_type==second_graphic->streamline_type)&&
				(graphic->stream_vector_field==second_graphic->stream_vector_field)&&
				(graphic->reverse_track==second_graphic->reverse_track)&&
				(graphic->streamline_length==second_graphic->streamline_length)&&
				(graphic->streamline_width==second_graphic->streamline_width)&&
				(graphic->seed_node_region==second_graphic->seed_node_region)&&
				(graphic->seed_node_coordinate_field==second_graphic->seed_node_coordinate_field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_same_geometry.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_same_geometry */

int Cmiss_graphic_same_name_or_geometry(struct Cmiss_graphic *graphic,
	void *second_graphic_void)
{
	int return_code;
	struct Cmiss_graphic *second_graphic;

	ENTER(Cmiss_graphic_same_name_or_geometry);
	if (graphic
		&&(second_graphic=(struct Cmiss_graphic *)second_graphic_void))
	{
		if (graphic->name && second_graphic->name)
		{
			return_code = (0==strcmp(graphic->name,second_graphic->name));
		}
		else
		{
			return_code = Cmiss_graphic_same_geometry(graphic,
				second_graphic_void);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_same_name_or_geometry.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_same_name_or_geometry */

int gfx_modify_rendition_node_points(struct Parse_state *state,
	void *modify_rendition_data_void,void *rendition_command_data_void)
{
	char *font_name;
	const char *glyph_scaling_mode_string, *select_mode_string, **valid_strings;
	enum Graphic_glyph_scaling_mode glyph_scaling_mode;
	enum Graphics_select_mode select_mode;
	int number_of_components,number_of_valid_strings,return_code,
		visibility;
	struct Computed_field *orientation_scale_field, *variable_scale_field;
	struct Graphics_font *new_font;
	struct Cmiss_graphic *graphic;
	struct GT_object *glyph;
	struct Rendition_command_data *rendition_command_data;
	struct Modify_rendition_data *modify_rendition_data;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_data_field_data, set_label_field_data, set_label_density_field_data,
		set_visibility_field_data, set_orientation_scale_field_data,
		set_variable_scale_field_data;
	Triple glyph_centre, glyph_scale_factors, glyph_size;

	ENTER(gfx_modify_rendition_node_points);
	if (state)
	{
		if (NULL != (rendition_command_data=(struct Rendition_command_data *)
				rendition_command_data_void))
		{
			if (NULL != (modify_rendition_data=
					(struct Modify_rendition_data *)modify_rendition_data_void))
			{
				/* create the cmiss_graphic: */
				graphic = CREATE(Cmiss_graphic)(CMISS_GRAPHIC_NODE_POINTS);
				/* if we are specifying the name then copy the existings graphic values so
					we can modify from these rather than the default */
				if (modify_rendition_data->graphic)
				{
					if (CMISS_GRAPHIC_NODE_POINTS ==modify_rendition_data->graphic->graphic_type)
					{
						Cmiss_graphic_copy_without_graphics_object(graphic, modify_rendition_data->graphic); 
					}
					DEACCESS(Cmiss_graphic)(&modify_rendition_data->graphic);
				}
				else
				{
					Cmiss_graphic_set_native_discretization_field(
						graphic, modify_rendition_data->native_discretization_field);
					Cmiss_graphic_set_coordinate_field(
						graphic, modify_rendition_data->default_coordinate_field);
					Cmiss_graphic_set_circle_discretization(
						graphic, modify_rendition_data->circle_discretization);
					Cmiss_graphic_set_discretization(
						graphic, &(modify_rendition_data->element_discretization));
				}
				if (graphic)
				{
					/* access since deaccessed in gfx_modify_rendition */
					modify_rendition_data->graphic = ACCESS(Cmiss_graphic)(graphic);
					/* Set up the coordinate_field */
					if (!graphic->coordinate_field)
					{
						struct Computed_field *computed_field;
						struct FE_field *fe_field;
						struct FE_region *fe_region = 
							Cmiss_region_get_FE_region(rendition_command_data->region);
						struct FE_region *data_region = 
							FE_region_get_data_FE_region(fe_region);
						if (FE_region_get_default_coordinate_FE_field(fe_region, &fe_field) ||
							FE_region_get_default_coordinate_FE_field(data_region, &fe_field))
						{
							if (NULL != (computed_field = FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
										Computed_field_is_read_only_with_fe_field,
										(void *)fe_field, rendition_command_data->computed_field_manager)))
							{
								graphic->coordinate_field = 
									ACCESS(Computed_field)(computed_field);
							}
						}
					}
					/* set essential parameters not set by CREATE function */
					if (!Cmiss_graphic_get_material(graphic))
					{
						Cmiss_graphic_set_material(graphic,
							rendition_command_data->default_material);
					}
					if (!Cmiss_graphic_get_selected_material(graphic))
					{
						Cmiss_graphic_set_selected_material(graphic,
							FIND_BY_IDENTIFIER_IN_MANAGER(Graphical_material,name)(
								"default_selected",
								rendition_command_data->graphical_material_manager));
					}
					if (!graphic->font)
					{
						graphic->font = ACCESS(Graphics_font)(
							rendition_command_data->default_font);
					}
					font_name = (char *)NULL;
					orientation_scale_field = (struct Computed_field *)NULL;
					variable_scale_field = (struct Computed_field *)NULL;
					Cmiss_graphic_get_glyph_parameters(graphic,
						&glyph, &glyph_scaling_mode, glyph_centre, glyph_size,
						&orientation_scale_field, glyph_scale_factors,
						&variable_scale_field);
					/* default to point glyph for fasest possible display */
					if (!glyph)
					{
						glyph = FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)("point",
							rendition_command_data->glyph_list);
					}
					ACCESS(GT_object)(glyph);
					if (orientation_scale_field)
					{
						ACCESS(Computed_field)(orientation_scale_field);
					}
					if (variable_scale_field)
					{
						ACCESS(Computed_field)(variable_scale_field);
					}
					number_of_components = 3;
					visibility = graphic->visibility;

					option_table=CREATE(Option_table)();
					/* as */
					Option_table_add_entry(option_table,"as",&(graphic->name),
						(void *)1,set_name);
					/* centre */
					Option_table_add_entry(option_table,"centre",glyph_centre,
						&(number_of_components),set_float_vector);
					/* coordinate */
					set_coordinate_field_data.computed_field_manager=
						rendition_command_data->computed_field_manager;
					set_coordinate_field_data.conditional_function=
						Computed_field_has_up_to_3_numerical_components;
					set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
					Option_table_add_entry(option_table,"coordinate",
						&(graphic->coordinate_field),&set_coordinate_field_data,
						set_Computed_field_conditional);
					/* data */
					set_data_field_data.computed_field_manager=
						rendition_command_data->computed_field_manager;
					set_data_field_data.conditional_function=
						Computed_field_has_numerical_components;
					set_data_field_data.conditional_function_user_data=(void *)NULL;
					Option_table_add_entry(option_table,"data",&(graphic->data_field),
						&set_data_field_data,set_Computed_field_conditional);
					/* delete */
					Option_table_add_entry(option_table,"delete",
						&(modify_rendition_data->delete_flag),NULL,set_char_flag);
					/* font */
					Option_table_add_name_entry(option_table, "font",
						&font_name);
					/* glyph */
					Option_table_add_entry(option_table,"glyph",&glyph,
						rendition_command_data->glyph_list,set_Graphics_object);
					/* label */
					set_label_field_data.computed_field_manager=
						rendition_command_data->computed_field_manager;
					set_label_field_data.conditional_function=
						(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
					set_label_field_data.conditional_function_user_data=(void *)NULL;
					Option_table_add_entry(option_table,"label",&(graphic->label_field),
						&set_label_field_data,set_Computed_field_conditional);
					/* ldensity */
					set_label_density_field_data.computed_field_manager=
						rendition_command_data->computed_field_manager;
					set_label_density_field_data.conditional_function=
						Computed_field_has_up_to_3_numerical_components;
					set_label_density_field_data.conditional_function_user_data=(void *)NULL;
					Option_table_add_entry(option_table,"ldensity",&(graphic->label_density_field),
						&set_label_density_field_data,set_Computed_field_conditional);
					/* visibility_field */
					set_visibility_field_data.computed_field_manager=
						rendition_command_data->computed_field_manager;
					set_visibility_field_data.conditional_function=
						(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
					set_visibility_field_data.conditional_function_user_data=(void *)NULL;
					Option_table_add_entry(option_table,"visibility_field",&(graphic->visibility_field),
						&set_visibility_field_data,set_Computed_field_conditional);
					/* material */
					Option_table_add_entry(option_table,"material",&(graphic->material),
						rendition_command_data->graphical_material_manager,
						set_Graphical_material);
					/* glyph scaling mode */
					glyph_scaling_mode_string =
						ENUMERATOR_STRING(Graphic_glyph_scaling_mode)(glyph_scaling_mode);
					valid_strings = ENUMERATOR_GET_VALID_STRINGS(Graphic_glyph_scaling_mode)(
						&number_of_valid_strings,
						(ENUMERATOR_CONDITIONAL_FUNCTION(Graphic_glyph_scaling_mode) *)NULL,
						(void *)NULL);
					Option_table_add_enumerator(option_table, number_of_valid_strings,
						valid_strings, &glyph_scaling_mode_string);
					DEALLOCATE(valid_strings);
					/* orientation */
					set_orientation_scale_field_data.computed_field_manager=
						rendition_command_data->computed_field_manager;
					set_orientation_scale_field_data.conditional_function=
						Computed_field_is_orientation_scale_capable;
					set_orientation_scale_field_data.conditional_function_user_data=
						(void *)NULL;
					Option_table_add_entry(option_table,"orientation",
						&orientation_scale_field,&set_orientation_scale_field_data,
						set_Computed_field_conditional);
					/* position */
					Option_table_add_entry(option_table,"position",
						&(modify_rendition_data->position),NULL,set_int_non_negative);
					/* scale_factors */
					Option_table_add_special_float3_entry(option_table,"scale_factors",
						glyph_scale_factors,"*");
					/* scene */
					Option_table_add_entry(option_table,"scene",
						&(modify_rendition_data->scene),
						rendition_command_data->scene_manager,set_Scene);
					/* select_mode */
					select_mode = Cmiss_graphic_get_select_mode(graphic);
					select_mode_string =
						ENUMERATOR_STRING(Graphics_select_mode)(select_mode);
					valid_strings = ENUMERATOR_GET_VALID_STRINGS(Graphics_select_mode)(
						&number_of_valid_strings,
						(ENUMERATOR_CONDITIONAL_FUNCTION(Graphics_select_mode) *)NULL,
						(void *)NULL);
					Option_table_add_enumerator(option_table,number_of_valid_strings,
						valid_strings,&select_mode_string);
					DEALLOCATE(valid_strings);
					/* selected_material */
					Option_table_add_entry(option_table,"selected_material",
						&(graphic->selected_material),
						rendition_command_data->graphical_material_manager,
						set_Graphical_material);
					/* size */
					Option_table_add_special_float3_entry(option_table,"size",
						glyph_size,"*");
					/* spectrum */
					Option_table_add_entry(option_table,"spectrum",
						&(graphic->spectrum),rendition_command_data->spectrum_manager,
						set_Spectrum);
					/* variable_scale */
					set_variable_scale_field_data.computed_field_manager =
						rendition_command_data->computed_field_manager;
					set_variable_scale_field_data.conditional_function =
						Computed_field_has_up_to_3_numerical_components;
					set_variable_scale_field_data.conditional_function_user_data =
						(void *)NULL;
					Option_table_add_entry(option_table,"variable_scale",
						&variable_scale_field, &set_variable_scale_field_data,
						set_Computed_field_conditional);
					/* visible/invisible */
					Option_table_add_switch(option_table, "visible", "invisible", &visibility);
					if ((return_code=Option_table_multi_parse(option_table,state)))
					{
						if (graphic->data_field&&!graphic->spectrum)
						{
							graphic->spectrum=ACCESS(Spectrum)(
								rendition_command_data->default_spectrum);
						}
						graphic->visibility = visibility;
						if (font_name && (new_font = Graphics_font_package_get_font
								(rendition_command_data->graphics_font_package, font_name)))
						{
							REACCESS(Graphics_font)(&graphic->font, new_font);
						}
						if (glyph)
						{
							STRING_TO_ENUMERATOR(Graphic_glyph_scaling_mode)(
								glyph_scaling_mode_string, &glyph_scaling_mode);
							Cmiss_graphic_set_glyph_parameters(graphic,
								glyph, glyph_scaling_mode, glyph_centre, glyph_size,
								orientation_scale_field,glyph_scale_factors,
								variable_scale_field);
						}
						else
						{
							display_message(WARNING_MESSAGE,
								"No glyph specified for node_points");
							return_code=0;
						}
						STRING_TO_ENUMERATOR(Graphics_select_mode)(select_mode_string,
							&select_mode);
						Cmiss_graphic_set_select_mode(graphic, select_mode);
					}
					DESTROY(Option_table)(&option_table);
					if (!return_code)
					{
						/* parse error, help */
						DEACCESS(Cmiss_graphic)(&(modify_rendition_data->graphic));
					}
					if (font_name)
					{
						DEALLOCATE(font_name);
					}
					if (glyph)
					{
						DEACCESS(GT_object)(&glyph);
					}
					if (orientation_scale_field)
					{
						DEACCESS(Computed_field)(&orientation_scale_field);
					}
					if (variable_scale_field)
					{
						DEACCESS(Computed_field)(&variable_scale_field);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_modify_rendition_node_points.  Could not create graphic");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_modify_rendition_node_points.  No modify data");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_modify_rendition_node_points.  "
				"Missing rendition_command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_rendition_node_points.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_rendition_node_points */

int gfx_modify_rendition_data_points(struct Parse_state *state,
	void *modify_rendition_data_void,void *rendition_command_data_void)
{
	char *font_name; 
	const char *glyph_scaling_mode_string, *select_mode_string, **valid_strings;
	enum Graphic_glyph_scaling_mode glyph_scaling_mode;
	enum Graphics_select_mode select_mode;
	int number_of_components,number_of_valid_strings,return_code,visibility;
	struct Computed_field *orientation_scale_field, *variable_scale_field;
	struct Graphics_font *new_font;
	struct Cmiss_graphic *graphic;
	struct GT_object *glyph;
	struct Rendition_command_data *rendition_command_data;
	struct Modify_rendition_data *modify_rendition_data;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_data_field_data, set_label_field_data, set_visibility_field_data, set_orientation_scale_field_data,
		set_label_density_field_data, set_variable_scale_field_data;
	Triple glyph_centre, glyph_scale_factors, glyph_size;

	ENTER(gfx_modify_rendition_data_points);
	if (state)
	{
		if (NULL != (rendition_command_data=(struct Rendition_command_data *)
				rendition_command_data_void))
		{
			if (NULL != (modify_rendition_data=
					(struct Modify_rendition_data *)modify_rendition_data_void))
			{
				/* create the gt_element_graphic: */
				graphic = CREATE(Cmiss_graphic)(CMISS_GRAPHIC_DATA_POINTS);
				/* if we are specifying the name then copy the existings graphic values so
					we can modify from these rather than the default */
				if (modify_rendition_data->graphic)
				{
					if (CMISS_GRAPHIC_DATA_POINTS ==modify_rendition_data->graphic->graphic_type)
					{
						Cmiss_graphic_copy_without_graphics_object(graphic, modify_rendition_data->graphic); 
					}
					DEACCESS(Cmiss_graphic)(&modify_rendition_data->graphic);
				}
				else
				{
					Cmiss_graphic_set_native_discretization_field(
						graphic, modify_rendition_data->native_discretization_field);
					Cmiss_graphic_set_coordinate_field(
						graphic, modify_rendition_data->default_coordinate_field);
					Cmiss_graphic_set_circle_discretization(
						graphic, modify_rendition_data->circle_discretization);
					Cmiss_graphic_set_discretization(
						graphic, &(modify_rendition_data->element_discretization));
				}
				if (graphic)
				{
					/* access since deaccessed in gfx_modify_rendition */
					modify_rendition_data->graphic = ACCESS(Cmiss_graphic)(graphic);
					/* Set up the coordinate_field */
					if (!graphic->coordinate_field)
					{
						struct Computed_field *computed_field;
						struct FE_field *fe_field;
						struct FE_region *fe_region = 
							Cmiss_region_get_FE_region(rendition_command_data->region);
						struct FE_region *data_region = 
							FE_region_get_data_FE_region(fe_region);
						if (FE_region_get_default_coordinate_FE_field(fe_region, &fe_field) ||
							FE_region_get_default_coordinate_FE_field(data_region, &fe_field))
						{
							if (NULL != (computed_field = FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
										Computed_field_is_read_only_with_fe_field,
										(void *)fe_field, rendition_command_data->computed_field_manager)))
							{
								graphic->coordinate_field = 
									ACCESS(Computed_field)(computed_field);
							}
						}
					}
					/* set essential parameters not set by CREATE function */
					if (!Cmiss_graphic_get_material(graphic))
					{
						Cmiss_graphic_set_material(graphic,
							rendition_command_data->default_material);
					}
					if (!Cmiss_graphic_get_selected_material(graphic))
					{
						Cmiss_graphic_set_selected_material(graphic,
							FIND_BY_IDENTIFIER_IN_MANAGER(Graphical_material,name)(
								"default_selected",
								rendition_command_data->graphical_material_manager));
					}
					if (!graphic->font)
					{
						graphic->font = ACCESS(Graphics_font)(
							rendition_command_data->default_font);
					}
					font_name = (char *)NULL;
					orientation_scale_field = (struct Computed_field *)NULL;
					variable_scale_field = (struct Computed_field *)NULL;
					Cmiss_graphic_get_glyph_parameters(graphic,
						&glyph, &glyph_scaling_mode, glyph_centre, glyph_size,
						&orientation_scale_field, glyph_scale_factors,
						&variable_scale_field);
					/* default to point glyph for fastest possible display */
					if (!glyph)
					{
						glyph = FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)("point",
							rendition_command_data->glyph_list);
					}
					ACCESS(GT_object)(glyph);
					if (orientation_scale_field)
					{
						ACCESS(Computed_field)(orientation_scale_field);
					}
					if (variable_scale_field)
					{
						ACCESS(Computed_field)(variable_scale_field);
					}
					number_of_components = 3;
					visibility = graphic->visibility;

					option_table=CREATE(Option_table)();
					/* as */
					Option_table_add_entry(option_table,"as",&(graphic->name),
						(void *)1,set_name);
					/* centre */
					Option_table_add_entry(option_table,"centre",glyph_centre,
						&(number_of_components),set_float_vector);
					/* coordinate */
					set_coordinate_field_data.computed_field_manager=
						rendition_command_data->computed_field_manager;
					set_coordinate_field_data.conditional_function=
						Computed_field_has_up_to_3_numerical_components;
					set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
					Option_table_add_entry(option_table,"coordinate",
						&(graphic->coordinate_field),&set_coordinate_field_data,
						set_Computed_field_conditional);
					/* data */
					set_data_field_data.computed_field_manager=
						rendition_command_data->computed_field_manager;
					set_data_field_data.conditional_function=
						Computed_field_has_numerical_components;
					set_data_field_data.conditional_function_user_data=(void *)NULL;
					Option_table_add_entry(option_table,"data",&(graphic->data_field),
						&set_data_field_data,set_Computed_field_conditional);
					/* delete */
					Option_table_add_entry(option_table,"delete",
						&(modify_rendition_data->delete_flag),NULL,set_char_flag);
					/* font */
					Option_table_add_name_entry(option_table, "font",
						&font_name);
					/* glyph */
					Option_table_add_entry(option_table,"glyph",&glyph,
						rendition_command_data->glyph_list,set_Graphics_object);
					/* label */
					set_label_field_data.computed_field_manager=
						rendition_command_data->computed_field_manager;
					set_label_field_data.conditional_function=
						(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
					set_label_field_data.conditional_function_user_data=(void *)NULL;
					Option_table_add_entry(option_table,"label",&(graphic->label_field),
						&set_label_field_data,set_Computed_field_conditional);
					/* ldensity */
					set_label_density_field_data.computed_field_manager=
						rendition_command_data->computed_field_manager;
					set_label_density_field_data.conditional_function=
						Computed_field_has_up_to_3_numerical_components;
					set_label_density_field_data.conditional_function_user_data=(void *)NULL;
					Option_table_add_entry(option_table,"ldensity",&(graphic->label_density_field),
						&set_label_density_field_data,set_Computed_field_conditional);
					/* visibility_field */
					set_visibility_field_data.computed_field_manager=
						rendition_command_data->computed_field_manager;
					set_visibility_field_data.conditional_function=
						(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
					set_visibility_field_data.conditional_function_user_data=(void *)NULL;
					Option_table_add_entry(option_table,"visibility_field",&(graphic->visibility_field),
						&set_visibility_field_data,set_Computed_field_conditional);
					/* material */
					Option_table_add_entry(option_table,"material",&(graphic->material),
						rendition_command_data->graphical_material_manager,
						set_Graphical_material);
					/* glyph scaling mode */
					glyph_scaling_mode_string =
						ENUMERATOR_STRING(Graphic_glyph_scaling_mode)(glyph_scaling_mode);
					valid_strings = ENUMERATOR_GET_VALID_STRINGS(Graphic_glyph_scaling_mode)(
						&number_of_valid_strings,
						(ENUMERATOR_CONDITIONAL_FUNCTION(Graphic_glyph_scaling_mode) *)NULL,
						(void *)NULL);
					Option_table_add_enumerator(option_table, number_of_valid_strings,
						valid_strings, &glyph_scaling_mode_string);
					DEALLOCATE(valid_strings);
					/* orientation */
					set_orientation_scale_field_data.computed_field_manager=
						rendition_command_data->computed_field_manager;
					set_orientation_scale_field_data.conditional_function=
						Computed_field_is_orientation_scale_capable;
					set_orientation_scale_field_data.conditional_function_user_data=
						(void *)NULL;
					Option_table_add_entry(option_table,"orientation",
						&orientation_scale_field,&set_orientation_scale_field_data,
						set_Computed_field_conditional);
					/* position */
					Option_table_add_entry(option_table,"position",
						&(modify_rendition_data->position),NULL,set_int_non_negative);
					/* scale_factors */
					Option_table_add_special_float3_entry(option_table,"scale_factors",
						glyph_scale_factors,"*");
					/* scene */
					Option_table_add_entry(option_table,"scene",
						&(modify_rendition_data->scene),
						rendition_command_data->scene_manager,set_Scene);
					/* select_mode */
					select_mode = Cmiss_graphic_get_select_mode(graphic);
					select_mode_string =
						ENUMERATOR_STRING(Graphics_select_mode)(select_mode);
					valid_strings = ENUMERATOR_GET_VALID_STRINGS(Graphics_select_mode)(
						&number_of_valid_strings,
						(ENUMERATOR_CONDITIONAL_FUNCTION(Graphics_select_mode) *)NULL,
						(void *)NULL);
					Option_table_add_enumerator(option_table,number_of_valid_strings,
						valid_strings,&select_mode_string);
					DEALLOCATE(valid_strings);
					/* selected_material */
					Option_table_add_entry(option_table,"selected_material",
						&(graphic->selected_material),
						rendition_command_data->graphical_material_manager,
						set_Graphical_material);
					/* size */
					Option_table_add_special_float3_entry(option_table,"size",
						glyph_size,"*");
					/* spectrum */
					Option_table_add_entry(option_table,"spectrum",
						&(graphic->spectrum),rendition_command_data->spectrum_manager,
						set_Spectrum);
					/* variable_scale */
					set_variable_scale_field_data.computed_field_manager =
						rendition_command_data->computed_field_manager;
					set_variable_scale_field_data.conditional_function =
						Computed_field_has_up_to_3_numerical_components;
					set_variable_scale_field_data.conditional_function_user_data =
						(void *)NULL;
					Option_table_add_entry(option_table,"variable_scale",
						&variable_scale_field, &set_variable_scale_field_data,
						set_Computed_field_conditional);
					/* visible/invisible */
					Option_table_add_switch(option_table, "visible", "invisible", &visibility);
					if ((return_code=Option_table_multi_parse(option_table,state)))
					{
						if (graphic->data_field&&!graphic->spectrum)
						{
							graphic->spectrum=ACCESS(Spectrum)(
								rendition_command_data->default_spectrum);
						}
						graphic->visibility = visibility;
						if (font_name && (new_font = Graphics_font_package_get_font
								(rendition_command_data->graphics_font_package, font_name)))
						{
							REACCESS(Graphics_font)(&graphic->font, new_font);
						}
						if (glyph)
						{
							STRING_TO_ENUMERATOR(Graphic_glyph_scaling_mode)(
								glyph_scaling_mode_string, &glyph_scaling_mode);
							Cmiss_graphic_set_glyph_parameters(graphic,
								glyph, glyph_scaling_mode, glyph_centre, glyph_size,
								orientation_scale_field,glyph_scale_factors,
								variable_scale_field);
						}
						else
						{
							display_message(WARNING_MESSAGE,
								"No glyph specified for data_points");
							return_code=0;
						}
						STRING_TO_ENUMERATOR(Graphics_select_mode)(select_mode_string,
							&select_mode);
						Cmiss_graphic_set_select_mode(graphic, select_mode);
					}
					DESTROY(Option_table)(&option_table);
					if (!return_code)
					{
						/* parse error, help */
						DEACCESS(Cmiss_graphic)(&(modify_rendition_data->graphic));
					}
					if (font_name)
					{
						DEALLOCATE(font_name);
					}
					if (glyph)
					{
						DEACCESS(GT_object)(&glyph);
					}
					if (orientation_scale_field)
					{
						DEACCESS(Computed_field)(&orientation_scale_field);
					}
					if (variable_scale_field)
					{
						DEACCESS(Computed_field)(&variable_scale_field);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_modify_rendition_data_points.  Could not create graphic");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_modify_rendition_data_points.  No modify data");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_modify_rendition_data_points.  "
				"Missing rendition_command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_rendition_data_points.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_rendition_data_points */

int gfx_modify_rendition_static_graphic(struct Parse_state *state,
	void *modify_rendition_data_void,void *rendition_command_data_void)
{
	char *font_name;
	const char *glyph_scaling_mode_string, *select_mode_string, **valid_strings;
	enum Graphic_glyph_scaling_mode glyph_scaling_mode;
	enum Graphics_select_mode select_mode;
	int number_of_components,number_of_valid_strings,return_code,
		visibility;
	struct Computed_field *orientation_scale_field, *variable_scale_field;
	struct Graphics_font *new_font;
	struct Cmiss_graphic *graphic;
	struct GT_object *glyph;
	struct Rendition_command_data *rendition_command_data;
	struct Modify_rendition_data *modify_rendition_data;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_data_field_data, set_label_field_data, set_orientation_scale_field_data,
		set_variable_scale_field_data;
	Triple glyph_centre, glyph_scale_factors, glyph_size;

	ENTER(gfx_modify_rendition_node_points);
	if (state)
	{
		if (NULL != (rendition_command_data=(struct Rendition_command_data *)
				rendition_command_data_void))
		{
			if (NULL != (modify_rendition_data=
					(struct Modify_rendition_data *)modify_rendition_data_void))
			{
				/* create the cmiss_graphic: */
				graphic = CREATE(Cmiss_graphic)(CMISS_GRAPHIC_STATIC);
				/* if we are specifying the name then copy the existings graphic values so
					we can modify from these rather than the default */
				if (modify_rendition_data->graphic)
				{
					if (CMISS_GRAPHIC_STATIC ==modify_rendition_data->graphic->graphic_type)
					{
						Cmiss_graphic_copy_without_graphics_object(graphic, modify_rendition_data->graphic);
					}
					DEACCESS(Cmiss_graphic)(&modify_rendition_data->graphic);
				}
				else
				{
					Cmiss_graphic_set_native_discretization_field(
						graphic, modify_rendition_data->native_discretization_field);
					Cmiss_graphic_set_coordinate_field(
						graphic, modify_rendition_data->default_coordinate_field);
					Cmiss_graphic_set_circle_discretization(
						graphic, modify_rendition_data->circle_discretization);
					Cmiss_graphic_set_discretization(
						graphic, &(modify_rendition_data->element_discretization));
				}
				if (graphic)
				{
					/* access since deaccessed in gfx_modify_rendition */
					modify_rendition_data->graphic = ACCESS(Cmiss_graphic)(graphic);
					/* Set up the coordinate_field */
					if (!graphic->coordinate_field)
					{
						struct Computed_field *computed_field;
						struct FE_field *fe_field;
						struct FE_region *fe_region =
							Cmiss_region_get_FE_region(rendition_command_data->region);
						struct FE_region *data_region =
							FE_region_get_data_FE_region(fe_region);
						if (FE_region_get_default_coordinate_FE_field(fe_region, &fe_field) ||
							FE_region_get_default_coordinate_FE_field(data_region, &fe_field))
						{
							if (NULL != (computed_field = FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
										Computed_field_is_read_only_with_fe_field,
										(void *)fe_field, rendition_command_data->computed_field_manager)))
							{
								graphic->coordinate_field =
									ACCESS(Computed_field)(computed_field);
							}
						}
					}
					/* set essential parameters not set by CREATE function */
					if (!Cmiss_graphic_get_material(graphic))
					{
						Cmiss_graphic_set_material(graphic,
							rendition_command_data->default_material);
					}
					if (!Cmiss_graphic_get_selected_material(graphic))
					{
						Cmiss_graphic_set_selected_material(graphic,
							FIND_BY_IDENTIFIER_IN_MANAGER(Graphical_material,name)(
								"default_selected",
								rendition_command_data->graphical_material_manager));
					}
					if (!graphic->font)
					{
						graphic->font = ACCESS(Graphics_font)(
							rendition_command_data->default_font);
					}
					font_name = (char *)NULL;
					orientation_scale_field = (struct Computed_field *)NULL;
					variable_scale_field = (struct Computed_field *)NULL;
					Cmiss_graphic_get_glyph_parameters(graphic,
						&glyph, &glyph_scaling_mode, glyph_centre, glyph_size,
						&orientation_scale_field, glyph_scale_factors,
						&variable_scale_field);
					/* default to point glyph for fasest possible display */
					if (!glyph)
					{
						glyph = FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)("point",
							rendition_command_data->glyph_list);
					}
					ACCESS(GT_object)(glyph);
					if (orientation_scale_field)
					{
						ACCESS(Computed_field)(orientation_scale_field);
					}
					if (variable_scale_field)
					{
						ACCESS(Computed_field)(variable_scale_field);
					}
					number_of_components = 3;
					visibility = graphic->visibility;

					option_table=CREATE(Option_table)();
					/* as */
					Option_table_add_entry(option_table,"as",&(graphic->name),
						(void *)1,set_name);
					/* centre */
					Option_table_add_entry(option_table,"centre",glyph_centre,
						&(number_of_components),set_float_vector);
					/* coordinate */
					set_coordinate_field_data.computed_field_manager=
						rendition_command_data->computed_field_manager;
					set_coordinate_field_data.conditional_function=
						Computed_field_has_up_to_3_numerical_components;
					set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
					Option_table_add_entry(option_table,"coordinate",
						&(graphic->coordinate_field),&set_coordinate_field_data,
						set_Computed_field_conditional);
					/* data */
					set_data_field_data.computed_field_manager=
						rendition_command_data->computed_field_manager;
					set_data_field_data.conditional_function=
						Computed_field_has_numerical_components;
					set_data_field_data.conditional_function_user_data=(void *)NULL;
					Option_table_add_entry(option_table,"data",&(graphic->data_field),
						&set_data_field_data,set_Computed_field_conditional);
					/* delete */
					Option_table_add_entry(option_table,"delete",
						&(modify_rendition_data->delete_flag),NULL,set_char_flag);
					/* font */
					Option_table_add_name_entry(option_table, "font",
						&font_name);
					/* glyph */
					Option_table_add_entry(option_table,"glyph",&glyph,
						rendition_command_data->glyph_list,set_Graphics_object);
					/* label */
					set_label_field_data.computed_field_manager=
						rendition_command_data->computed_field_manager;
					set_label_field_data.conditional_function=
						(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
					set_label_field_data.conditional_function_user_data=(void *)NULL;
					Option_table_add_entry(option_table,"label",&(graphic->label_field),
						&set_label_field_data,set_Computed_field_conditional);
					/* material */
					Option_table_add_entry(option_table,"material",&(graphic->material),
						rendition_command_data->graphical_material_manager,
						set_Graphical_material);
					/* glyph scaling mode */
					glyph_scaling_mode_string =
						ENUMERATOR_STRING(Graphic_glyph_scaling_mode)(glyph_scaling_mode);
					valid_strings = ENUMERATOR_GET_VALID_STRINGS(Graphic_glyph_scaling_mode)(
						&number_of_valid_strings,
						(ENUMERATOR_CONDITIONAL_FUNCTION(Graphic_glyph_scaling_mode) *)NULL,
						(void *)NULL);
					Option_table_add_enumerator(option_table, number_of_valid_strings,
						valid_strings, &glyph_scaling_mode_string);
					DEALLOCATE(valid_strings);
					/* orientation */
					set_orientation_scale_field_data.computed_field_manager=
						rendition_command_data->computed_field_manager;
					set_orientation_scale_field_data.conditional_function=
						Computed_field_is_orientation_scale_capable;
					set_orientation_scale_field_data.conditional_function_user_data=
						(void *)NULL;
					Option_table_add_entry(option_table,"orientation",
						&orientation_scale_field,&set_orientation_scale_field_data,
						set_Computed_field_conditional);
					/* position */
					Option_table_add_entry(option_table,"position",
						&(modify_rendition_data->position),NULL,set_int_non_negative);
					/* scale_factors */
					Option_table_add_special_float3_entry(option_table,"scale_factors",
						glyph_scale_factors,"*");
					/* scene */
					Option_table_add_entry(option_table,"scene",
						&(modify_rendition_data->scene),
						rendition_command_data->scene_manager,set_Scene);
					/* select_mode */
					select_mode = Cmiss_graphic_get_select_mode(graphic);
					select_mode_string =
						ENUMERATOR_STRING(Graphics_select_mode)(select_mode);
					valid_strings = ENUMERATOR_GET_VALID_STRINGS(Graphics_select_mode)(
						&number_of_valid_strings,
						(ENUMERATOR_CONDITIONAL_FUNCTION(Graphics_select_mode) *)NULL,
						(void *)NULL);
					Option_table_add_enumerator(option_table,number_of_valid_strings,
						valid_strings,&select_mode_string);
					DEALLOCATE(valid_strings);
					/* selected_material */
					Option_table_add_entry(option_table,"selected_material",
						&(graphic->selected_material),
						rendition_command_data->graphical_material_manager,
						set_Graphical_material);
					/* size */
					Option_table_add_special_float3_entry(option_table,"size",
						glyph_size,"*");
					/* spectrum */
					Option_table_add_entry(option_table,"spectrum",
						&(graphic->spectrum),rendition_command_data->spectrum_manager,
						set_Spectrum);
					/* variable_scale */
					set_variable_scale_field_data.computed_field_manager =
						rendition_command_data->computed_field_manager;
					set_variable_scale_field_data.conditional_function =
						Computed_field_has_up_to_3_numerical_components;
					set_variable_scale_field_data.conditional_function_user_data =
						(void *)NULL;
					Option_table_add_entry(option_table,"variable_scale",
						&variable_scale_field, &set_variable_scale_field_data,
						set_Computed_field_conditional);
					/* visible/invisible */
					Option_table_add_switch(option_table, "visible", "invisible", &visibility);
					if ((return_code=Option_table_multi_parse(option_table,state)))
					{
						if (graphic->data_field&&!graphic->spectrum)
						{
							graphic->spectrum=ACCESS(Spectrum)(
								rendition_command_data->default_spectrum);
						}
						graphic->visibility = visibility;
						if (font_name && (new_font = Graphics_font_package_get_font
								(rendition_command_data->graphics_font_package, font_name)))
						{
							REACCESS(Graphics_font)(&graphic->font, new_font);
						}
						if (glyph)
						{
							STRING_TO_ENUMERATOR(Graphic_glyph_scaling_mode)(
								glyph_scaling_mode_string, &glyph_scaling_mode);
							Cmiss_graphic_set_glyph_parameters(graphic,
								glyph, glyph_scaling_mode, glyph_centre, glyph_size,
								orientation_scale_field,glyph_scale_factors,
								variable_scale_field);
						}
						else
						{
							display_message(WARNING_MESSAGE,
								"No glyph specified for node_points");
							return_code=0;
						}
						STRING_TO_ENUMERATOR(Graphics_select_mode)(select_mode_string,
							&select_mode);
						Cmiss_graphic_set_select_mode(graphic, select_mode);
					}
					DESTROY(Option_table)(&option_table);
					if (!return_code)
					{
						/* parse error, help */
						DEACCESS(Cmiss_graphic)(&(modify_rendition_data->graphic));
					}
					if (font_name)
					{
						DEALLOCATE(font_name);
					}
					if (glyph)
					{
						DEACCESS(GT_object)(&glyph);
					}
					if (orientation_scale_field)
					{
						DEACCESS(Computed_field)(&orientation_scale_field);
					}
					if (variable_scale_field)
					{
						DEACCESS(Computed_field)(&variable_scale_field);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_modify_rendition_node_points.  Could not create graphic");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_modify_rendition_node_points.  No modify data");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_modify_rendition_node_points.  "
				"Missing rendition_command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_rendition_node_points.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_rendition_node_points */


int gfx_modify_rendition_lines(struct Parse_state *state,
	void *modify_rendition_data_void,void *rendition_command_data_void)
{
	const char *select_mode_string,**valid_strings;
	enum Graphics_select_mode select_mode;
	int number_of_valid_strings,return_code, visibility;
	struct Modify_rendition_data *modify_rendition_data;
	struct Cmiss_graphic *graphic;
	struct Rendition_command_data *rendition_command_data;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_data_field_data, set_texture_coordinate_field_data;

	ENTER(gfx_modify_rendition_lines);
	if (state)
	{
		if (NULL != (rendition_command_data=(struct Rendition_command_data *)
				rendition_command_data_void))
		{
			if (NULL != (modify_rendition_data=
					(struct Modify_rendition_data *)modify_rendition_data_void))
			{
				/* create the cmiss_graphic: */
				graphic = CREATE(Cmiss_graphic)(CMISS_GRAPHIC_LINES);
				/* if we are specifying the name then copy the existings graphic values so
					we can modify from these rather than the default */
				if (modify_rendition_data->graphic)
				{
					if (CMISS_GRAPHIC_LINES ==modify_rendition_data->graphic->graphic_type)
					{
						Cmiss_graphic_copy_without_graphics_object(graphic, modify_rendition_data->graphic); 
					}
					DEACCESS(Cmiss_graphic)(&modify_rendition_data->graphic);
				}
				else
				{
					Cmiss_graphic_set_native_discretization_field(
						graphic, modify_rendition_data->native_discretization_field);
					Cmiss_graphic_set_coordinate_field(
						graphic, modify_rendition_data->default_coordinate_field);
					Cmiss_graphic_set_circle_discretization(
						graphic, modify_rendition_data->circle_discretization);
					Cmiss_graphic_set_discretization(
						graphic, &(modify_rendition_data->element_discretization));
				}
				if (graphic)
				{
					/* access since deaccessed in gfx_modify_rendition */
					modify_rendition_data->graphic = ACCESS(Cmiss_graphic)(graphic);
					/* Set up the coordinate_field */
					if (!graphic->coordinate_field)
					{
						struct Computed_field *computed_field;
						struct FE_field *fe_field;
						struct FE_region *fe_region = 
							Cmiss_region_get_FE_region(rendition_command_data->region);
						struct FE_region *data_region = 
							FE_region_get_data_FE_region(fe_region);
						if (FE_region_get_default_coordinate_FE_field(fe_region, &fe_field) ||
							FE_region_get_default_coordinate_FE_field(data_region, &fe_field))
						{
							if (NULL != (computed_field = FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
										Computed_field_is_read_only_with_fe_field,
										(void *)fe_field, rendition_command_data->computed_field_manager)))
							{
								graphic->coordinate_field = 
									ACCESS(Computed_field)(computed_field);
							}
						}
					}
					/* set essential parameters not set by CREATE function */
					if (!Cmiss_graphic_get_material(graphic))
					{
						Cmiss_graphic_set_material(graphic,
							rendition_command_data->default_material);
					}
					if (!Cmiss_graphic_get_selected_material(graphic))
					{
						Cmiss_graphic_set_selected_material(graphic,
							FIND_BY_IDENTIFIER_IN_MANAGER(Graphical_material,name)(
								"default_selected",
								rendition_command_data->graphical_material_manager));
					}
					visibility = graphic->visibility;
					option_table=CREATE(Option_table)();
					/* as */
					Option_table_add_entry(option_table,"as",&(graphic->name),
						(void *)1,set_name);
					/* coordinate */
					set_coordinate_field_data.computed_field_manager=
						rendition_command_data->computed_field_manager;
					set_coordinate_field_data.conditional_function=
						Computed_field_has_up_to_3_numerical_components;
					set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
					Option_table_add_entry(option_table,"coordinate",
						&(graphic->coordinate_field),&set_coordinate_field_data,
						set_Computed_field_conditional);
					/* data */
					set_data_field_data.computed_field_manager=
						rendition_command_data->computed_field_manager;
					set_data_field_data.conditional_function=
						Computed_field_has_numerical_components;
					set_data_field_data.conditional_function_user_data=(void *)NULL;
					Option_table_add_entry(option_table,"data",&(graphic->data_field),
						&set_data_field_data,set_Computed_field_conditional);
					/* delete */
					Option_table_add_entry(option_table,"delete",
						&(modify_rendition_data->delete_flag),NULL,set_char_flag);
					/* discretization */
					Option_table_add_entry(option_table,"discretization",
						&(graphic->discretization),rendition_command_data->user_interface,
						set_Element_discretization);
					/* exterior */
					Option_table_add_entry(option_table,"exterior",&(graphic->exterior),
						NULL,set_char_flag);
					/* face */
					Option_table_add_entry(option_table,"face",&(graphic->face),
						NULL,set_exterior);
					/* line_width */
					Option_table_add_int_non_negative_entry(option_table,"line_width",
						&(graphic->line_width));
					/* material */
					Option_table_add_entry(option_table,"material",&(graphic->material),
						rendition_command_data->graphical_material_manager,
						set_Graphical_material);
					/* multipass_pass1_material */
					Option_table_add_entry(option_table,"multipass_pass1_material",
						&(graphic->secondary_material),
						rendition_command_data->graphical_material_manager,
						set_Graphical_material);
					/* position */
					Option_table_add_entry(option_table,"position",
						&(modify_rendition_data->position),NULL,set_int_non_negative);
					/* scene */
					Option_table_add_entry(option_table,"scene",
						&(modify_rendition_data->scene),
						rendition_command_data->scene_manager,set_Scene);
					/* select_mode */
					select_mode = Cmiss_graphic_get_select_mode(graphic);
					select_mode_string =
						ENUMERATOR_STRING(Graphics_select_mode)(select_mode);
					valid_strings = ENUMERATOR_GET_VALID_STRINGS(Graphics_select_mode)(
						&number_of_valid_strings,
						(ENUMERATOR_CONDITIONAL_FUNCTION(Graphics_select_mode) *)NULL,
						(void *)NULL);
					Option_table_add_enumerator(option_table,number_of_valid_strings,
						valid_strings,&select_mode_string);
					DEALLOCATE(valid_strings);
					/* selected_material */
					Option_table_add_entry(option_table,"selected_material",
						&(graphic->selected_material),
						rendition_command_data->graphical_material_manager,
						set_Graphical_material);
					/* spectrum */
					Option_table_add_entry(option_table,"spectrum",
						&(graphic->spectrum),rendition_command_data->spectrum_manager,
						set_Spectrum);
					/* texture_coordinates */
					set_texture_coordinate_field_data.computed_field_manager=
						rendition_command_data->computed_field_manager;
					set_texture_coordinate_field_data.conditional_function=
						Computed_field_has_up_to_3_numerical_components;
					set_texture_coordinate_field_data.conditional_function_user_data=
						(void *)NULL;
					Option_table_add_entry(option_table,"texture_coordinates",
						&(graphic->texture_coordinate_field),
						&set_texture_coordinate_field_data,set_Computed_field_conditional);
					/* visible/invisible */
					Option_table_add_switch(option_table, "visible", "invisible", &visibility);
					if ((return_code=Option_table_multi_parse(option_table,state)))
					{
						if (graphic->data_field&&!graphic->spectrum)
						{
							graphic->spectrum=ACCESS(Spectrum)(
								rendition_command_data->default_spectrum);
						}
						graphic->visibility = visibility;
						STRING_TO_ENUMERATOR(Graphics_select_mode)(select_mode_string,
							&select_mode);
						Cmiss_graphic_set_select_mode(graphic, select_mode);
					}
					DESTROY(Option_table)(&option_table);
					if (!return_code)
					{
						/* parse error, help */
						DEACCESS(Cmiss_graphic)(&(modify_rendition_data->graphic));
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_modify_rendition_lines.  Could not create graphic");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_modify_rendition_lines.  No modify data");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_modify_rendition_lines.  "
				"Missing rendition_command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_modify_rendition_lines.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_rendition_lines */

int gfx_modify_rendition_cylinders(struct Parse_state *state,
	void *modify_rendition_data_void,void *rendition_command_data_void)
{
	const char *render_type_string,*select_mode_string,**valid_strings;
	enum Graphics_select_mode select_mode;
	enum Render_type render_type;
	int number_of_valid_strings,return_code, visibility;
	struct Modify_rendition_data *modify_rendition_data;
	struct Cmiss_graphic *graphic;
	struct Rendition_command_data *rendition_command_data;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_data_field_data, set_radius_scalar_field_data,
		set_texture_coordinate_field_data;

	ENTER(gfx_modify_rendition_cylinders);
	if (state)
	{
		if (NULL != (rendition_command_data=(struct Rendition_command_data *)
				rendition_command_data_void))
		{
			if (NULL != (modify_rendition_data=
					(struct Modify_rendition_data *)modify_rendition_data_void))
			{
				/* create the cmiss_graphic: */
				graphic = CREATE(Cmiss_graphic)(CMISS_GRAPHIC_CYLINDERS);
				/* if we are specifying the name then copy the existings graphic values so
					we can modify from these rather than the default */
				if (modify_rendition_data->graphic)
				{
					if (CMISS_GRAPHIC_CYLINDERS ==modify_rendition_data->graphic->graphic_type)
					{
						Cmiss_graphic_copy_without_graphics_object(graphic, modify_rendition_data->graphic); 
					}
					DEACCESS(Cmiss_graphic)(&modify_rendition_data->graphic);
				}
				else
				{
					Cmiss_graphic_set_native_discretization_field(
						graphic, modify_rendition_data->native_discretization_field);
					Cmiss_graphic_set_coordinate_field(
						graphic, modify_rendition_data->default_coordinate_field);
					Cmiss_graphic_set_circle_discretization(
						graphic, modify_rendition_data->circle_discretization);
					Cmiss_graphic_set_discretization(
						graphic, &(modify_rendition_data->element_discretization));
				}
				if (graphic)
				{
					/* access since deaccessed in gfx_modify_rendition */
					modify_rendition_data->graphic = ACCESS(Cmiss_graphic)(graphic);
					/* Set up the coordinate_field */
					if (!graphic->coordinate_field)
					{
						struct Computed_field *computed_field;
						struct FE_field *fe_field;
						struct FE_region *fe_region = 
							Cmiss_region_get_FE_region(rendition_command_data->region);
						struct FE_region *data_region = 
							FE_region_get_data_FE_region(fe_region);
						if (FE_region_get_default_coordinate_FE_field(fe_region, &fe_field) ||
							FE_region_get_default_coordinate_FE_field(data_region, &fe_field))
						{
							if (NULL != (computed_field = FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
										Computed_field_is_read_only_with_fe_field,
										(void *)fe_field, rendition_command_data->computed_field_manager)))
							{
								graphic->coordinate_field = 
									ACCESS(Computed_field)(computed_field);
							}
						}
					}
					/* set essential parameters not set by CREATE function */
					if (!Cmiss_graphic_get_material(graphic))
					{
						Cmiss_graphic_set_material(graphic,
							rendition_command_data->default_material);
					}
					if (!Cmiss_graphic_get_selected_material(graphic))
					{
						Cmiss_graphic_set_selected_material(graphic,
							FIND_BY_IDENTIFIER_IN_MANAGER(Graphical_material,name)(
								"default_selected",
								rendition_command_data->graphical_material_manager));
					}
					visibility = graphic->visibility;
					option_table=CREATE(Option_table)();
					/* as */
					Option_table_add_entry(option_table,"as",&(graphic->name),
						(void *)1,set_name);
					/* circle_discretization */
					Option_table_add_entry(option_table, "circle_discretization",
						(void *)&(graphic->circle_discretization), (void *)NULL,
						set_Circle_discretization);
					/* constant_radius */
					Option_table_add_entry(option_table,"constant_radius",
						&(graphic->constant_radius),NULL,set_float);
					/* coordinate */
					set_coordinate_field_data.computed_field_manager=
						rendition_command_data->computed_field_manager;
					set_coordinate_field_data.conditional_function=
						Computed_field_has_up_to_3_numerical_components;
					set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
					Option_table_add_entry(option_table,"coordinate",
						&(graphic->coordinate_field),&set_coordinate_field_data,
						set_Computed_field_conditional);
					/* data */
					set_data_field_data.computed_field_manager=
						rendition_command_data->computed_field_manager;
					set_data_field_data.conditional_function=
						Computed_field_has_numerical_components;
					set_data_field_data.conditional_function_user_data=(void *)NULL;
					Option_table_add_entry(option_table,"data",&(graphic->data_field),
						&set_data_field_data,set_Computed_field_conditional);
					/* discretization */
					Option_table_add_entry(option_table,"discretization",
						&(graphic->discretization),rendition_command_data->user_interface,
						set_Element_discretization);
					/* delete */
					Option_table_add_entry(option_table,"delete",
						&(modify_rendition_data->delete_flag),NULL,set_char_flag);
					/* exterior */
					Option_table_add_entry(option_table,"exterior",&(graphic->exterior),
						NULL,set_char_flag);
					/* face */
					Option_table_add_entry(option_table,"face",&(graphic->face),
						NULL,set_exterior);
					/* material */
					Option_table_add_entry(option_table,"material",&(graphic->material),
						rendition_command_data->graphical_material_manager,
						set_Graphical_material);
					/* position */
					Option_table_add_entry(option_table,"position",
						&(modify_rendition_data->position),NULL,set_int_non_negative);
					/* render_type */
					render_type = graphic->render_type;
					render_type_string = ENUMERATOR_STRING(Render_type)(render_type);
					valid_strings = ENUMERATOR_GET_VALID_STRINGS(Render_type)(
						&number_of_valid_strings,
						(ENUMERATOR_CONDITIONAL_FUNCTION(Render_type) *)NULL, (void *)NULL);
					Option_table_add_enumerator(option_table,number_of_valid_strings,
						valid_strings,&render_type_string);
					DEALLOCATE(valid_strings);
					/* radius_scalar */
					set_radius_scalar_field_data.computed_field_manager=
						rendition_command_data->computed_field_manager;
					set_radius_scalar_field_data.conditional_function=
						Computed_field_is_scalar;
					set_radius_scalar_field_data.conditional_function_user_data=
						(void *)NULL;
					Option_table_add_entry(option_table,"radius_scalar",
						&(graphic->radius_scalar_field),&set_radius_scalar_field_data,
						set_Computed_field_conditional);
					/* scale_factor */
					Option_table_add_entry(option_table,"scale_factor",
						&(graphic->radius_scale_factor),NULL,set_float);
					/* scene */
					Option_table_add_entry(option_table,"scene",
						&(modify_rendition_data->scene),
						rendition_command_data->scene_manager,set_Scene);
					/* select_mode */
					select_mode = Cmiss_graphic_get_select_mode(graphic);
					select_mode_string =
						ENUMERATOR_STRING(Graphics_select_mode)(select_mode);
					valid_strings = ENUMERATOR_GET_VALID_STRINGS(Graphics_select_mode)(
						&number_of_valid_strings,
						(ENUMERATOR_CONDITIONAL_FUNCTION(Graphics_select_mode) *)NULL,
						(void *)NULL);
					Option_table_add_enumerator(option_table,number_of_valid_strings,
						valid_strings,&select_mode_string);
					DEALLOCATE(valid_strings);
					/* selected_material */
					Option_table_add_entry(option_table,"selected_material",
						&(graphic->selected_material),
						rendition_command_data->graphical_material_manager,
						set_Graphical_material);
					/* spectrum */
					Option_table_add_entry(option_table,"spectrum",
						&(graphic->spectrum),rendition_command_data->spectrum_manager,
						set_Spectrum);
					/* texture_coordinates */
					set_texture_coordinate_field_data.computed_field_manager=
						rendition_command_data->computed_field_manager;
					set_texture_coordinate_field_data.conditional_function=
						Computed_field_has_up_to_3_numerical_components;
					set_texture_coordinate_field_data.conditional_function_user_data=
						(void *)NULL;
					Option_table_add_entry(option_table,"texture_coordinates",
						&(graphic->texture_coordinate_field),
						&set_texture_coordinate_field_data,set_Computed_field_conditional);
					/* visible/invisible */
					Option_table_add_switch(option_table, "visible", "invisible", &visibility);
					if ((return_code=Option_table_multi_parse(option_table,state)))
					{
						if (graphic->data_field&&!graphic->spectrum)
						{
							graphic->spectrum=ACCESS(Spectrum)(
								rendition_command_data->default_spectrum);
						}
						graphic->visibility = visibility;
						STRING_TO_ENUMERATOR(Graphics_select_mode)(select_mode_string,
							&select_mode);
						Cmiss_graphic_set_select_mode(graphic, select_mode);
						STRING_TO_ENUMERATOR(Render_type)(render_type_string, &render_type);
						Cmiss_graphic_set_render_type(graphic, render_type);
					}
					DESTROY(Option_table)(&option_table);
					if (!return_code)
					{
						/* parse error, help */
						DEACCESS(Cmiss_graphic)(&(modify_rendition_data->graphic));
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_modify_rendition_cylinders.  Could not create graphic");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_modify_rendition_cylinders.  No modify data");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_modify_rendition_cylinders.  "
				"Missing rendition_command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_rendition_cylinders.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_rendition_cylinders */

int gfx_modify_rendition_iso_surfaces(struct Parse_state *state,
	void *modify_rendition_data_void,void *rendition_command_data_void)
{
	const char *render_type_string,*select_mode_string, *use_element_type_string,
		**valid_strings;
	enum Graphics_select_mode select_mode;
	enum Render_type render_type;
	enum Use_element_type use_element_type;
	int number_of_valid_strings,range_number_of_iso_values,
		return_code,visibility;
	struct Modify_rendition_data *modify_rendition_data;
	struct Cmiss_graphic *graphic;
	struct Rendition_command_data *rendition_command_data;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_data_field_data,set_iso_scalar_field_data,
		set_texture_coordinate_field_data;

	ENTER(gfx_modify_rendition_iso_surfaces);
	if (state)
	{
		if (NULL != (rendition_command_data=
			(struct Rendition_command_data *)rendition_command_data_void))
		{
			if (NULL != (modify_rendition_data=
					(struct Modify_rendition_data *)modify_rendition_data_void))
			{
				/* create the cmiss_graphic: */
				graphic = CREATE(Cmiss_graphic)(CMISS_GRAPHIC_ISO_SURFACES);
				/* if we are specifying the name then copy the existings graphic values so
					we can modify from these rather than the default */
				if (modify_rendition_data->graphic)
				{
					if (CMISS_GRAPHIC_ISO_SURFACES ==modify_rendition_data->graphic->graphic_type)
					{
						Cmiss_graphic_copy_without_graphics_object(graphic, modify_rendition_data->graphic); 
					}
					DEACCESS(Cmiss_graphic)(&modify_rendition_data->graphic);
				}
				else
				{
					Cmiss_graphic_set_native_discretization_field(
						graphic, modify_rendition_data->native_discretization_field);
					Cmiss_graphic_set_coordinate_field(
						graphic, modify_rendition_data->default_coordinate_field);
					Cmiss_graphic_set_circle_discretization(
						graphic, modify_rendition_data->circle_discretization);
					Cmiss_graphic_set_discretization(
						graphic, &(modify_rendition_data->element_discretization));
				}
				if (graphic)
				{
					/* access since deaccessed in gfx_modify_rendition */
					modify_rendition_data->graphic = ACCESS(Cmiss_graphic)(graphic);
					/* Set up the coordinate_field */
					if (!graphic->coordinate_field)
					{
						struct Computed_field *computed_field;
						struct FE_field *fe_field;
						struct FE_region *fe_region = 
							Cmiss_region_get_FE_region(rendition_command_data->region);
						struct FE_region *data_region = 
							FE_region_get_data_FE_region(fe_region);
						if (FE_region_get_default_coordinate_FE_field(fe_region, &fe_field) ||
							FE_region_get_default_coordinate_FE_field(data_region, &fe_field))
						{
							if (NULL != (computed_field = FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
										Computed_field_is_read_only_with_fe_field,
										(void *)fe_field, rendition_command_data->computed_field_manager)))
							{
								graphic->coordinate_field = 
									ACCESS(Computed_field)(computed_field);
							}
						}
					}
					/* set essential parameters not set by CREATE function */
					if (!Cmiss_graphic_get_material(graphic))
					{
						Cmiss_graphic_set_material(graphic,
							rendition_command_data->default_material);
					}
					if (!Cmiss_graphic_get_selected_material(graphic))
					{
						Cmiss_graphic_set_selected_material(graphic,
							FIND_BY_IDENTIFIER_IN_MANAGER(Graphical_material,name)(
								"default_selected",
								rendition_command_data->graphical_material_manager));
					}
					visibility = graphic->visibility;
					range_number_of_iso_values = 0;
					option_table=CREATE(Option_table)();
					Option_table_add_help(option_table,
						"Create isosurfaces in volume elements <use_elements> or isolines in face or surface elements <use_faces>.  "
						"The isosurface will be generated at the values in the elements where <iso_scalar> equals the iso values specified.  "
						"The iso values can be specified either as a list with the <iso_values> option "
						"or by specifying <range_number_of_iso_values>, <first_iso_value> and <last_iso_value>.  "
						"The <as> parameter allows a name to be specified for this setting.  "
						"The <coordinate> parameter optionally overrides the groups default coordinate field.  "
						"If a <data> field is specified then the <spectrum> is used to render the data values as colour on the generated isosurface.  "
						"If a <decimation_threshold> is specified then the resulting iso_surface will be decimated according to the threshold.  "
						"If <delete> is specified then if the graphic matches an existing setting (either by parameters or name) then it will be removed.  "
						"If <exterior> is specified then only faces with one parent will be selected when <use_faces> is specified.  "
						"If <face> is specified then only that face will be selected when <use_faces> is specified.  "
						"The <material> is used to render the surface.  "
						"You can specify the <scene> the graphic is for.  "
						"You can specify the <position> the graphic has in the graphic list.  "
						"You can specify the <line_width>, this option only applies when <use_faces> is specified.  "
						"You can render a mesh as solid <render_shaded> or as a wireframe <render_wireframe>.  "
						"If <select_on> is active then the element tool will select the elements the iso_surface was generated from.  "
						"If <no_select> is active then the iso_surface cannot be selected.  "
						"If <draw_selected> is active then iso_surfaces will only be generated in elements that are selected.  "
						"Conversely, if <draw_unselected> is active then iso_surfaces will only be generated in elements that are not selected.  "
						"The <texture_coordinates> are used to lay out a texture if the <material> contains a texture.  "
						"A graphic can be made <visible> or <invisible>.  ");
					/* as */
					Option_table_add_entry(option_table,"as",&(graphic->name),
						(void *)1,set_name);
					/* coordinate */
					set_coordinate_field_data.computed_field_manager=
						rendition_command_data->computed_field_manager;
					set_coordinate_field_data.conditional_function=
						Computed_field_has_up_to_3_numerical_components;
					set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
					Option_table_add_entry(option_table,"coordinate",
						&(graphic->coordinate_field),&set_coordinate_field_data,
						set_Computed_field_conditional);
					/* data */
					set_data_field_data.computed_field_manager=
						rendition_command_data->computed_field_manager;
					set_data_field_data.conditional_function=
						Computed_field_has_numerical_components;
					set_data_field_data.conditional_function_user_data=(void *)NULL;
					Option_table_add_entry(option_table,"data",&(graphic->data_field),
						&set_data_field_data,set_Computed_field_conditional);
					/* decimation_threshold */
					Option_table_add_double_entry(option_table, "decimation_threshold",
						&(graphic->decimation_threshold));
					/* delete */
					Option_table_add_entry(option_table,"delete",
						&(modify_rendition_data->delete_flag),NULL,set_char_flag);
					/* discretization */
					Option_table_add_entry(option_table,"discretization",
						&(graphic->discretization),rendition_command_data->user_interface,
						set_Element_discretization);
					/* exterior */
					Option_table_add_entry(option_table,"exterior",&(graphic->exterior),
						NULL,set_char_flag);
					/* face */
					Option_table_add_entry(option_table,"face",&(graphic->face),
						NULL,set_exterior);
					/* iso_scalar */
					set_iso_scalar_field_data.computed_field_manager=
						rendition_command_data->computed_field_manager;
					set_iso_scalar_field_data.conditional_function=
						Computed_field_is_scalar;
					set_iso_scalar_field_data.conditional_function_user_data=(void *)NULL;
					Option_table_add_entry(option_table,"iso_scalar",
						&(graphic->iso_scalar_field),&set_iso_scalar_field_data,
						set_Computed_field_conditional);
					/* iso_values */
					Option_table_add_variable_length_double_vector_entry(option_table,
						"iso_values", &(graphic->number_of_iso_values), &(graphic->iso_values));
					/* material */
					Option_table_add_entry(option_table,"material",&(graphic->material),
						rendition_command_data->graphical_material_manager,
						set_Graphical_material);
					/* last_iso_value */
					Option_table_add_double_entry(option_table,"last_iso_value",
						&(graphic->last_iso_value));
					/* first_iso_value */
					Option_table_add_double_entry(option_table,"first_iso_value",
						&(graphic->first_iso_value));	
					/* position */
					Option_table_add_entry(option_table,"position",
						&(modify_rendition_data->position),NULL,set_int_non_negative);
					/* range_number_of_iso_values */
					/* Use a different temporary storage for this value so we
						can tell it from the original variable_length array
						"iso_values" above. */
					Option_table_add_int_positive_entry(option_table,
						"range_number_of_iso_values",
						&range_number_of_iso_values);
					Option_table_add_int_non_negative_entry(option_table,"line_width",
						&(graphic->line_width));
					/* render_type */
					render_type = graphic->render_type;
					render_type_string = ENUMERATOR_STRING(Render_type)(render_type);
					valid_strings = ENUMERATOR_GET_VALID_STRINGS(Render_type)(
						&number_of_valid_strings,
						(ENUMERATOR_CONDITIONAL_FUNCTION(Render_type) *)NULL, (void *)NULL);
					Option_table_add_enumerator(option_table,number_of_valid_strings,
						valid_strings,&render_type_string);
					DEALLOCATE(valid_strings);
					/* scene */
					Option_table_add_entry(option_table,"scene",
						&(modify_rendition_data->scene),
						rendition_command_data->scene_manager,set_Scene);
					/* select_mode */
					select_mode = Cmiss_graphic_get_select_mode(graphic);
					select_mode_string =
						ENUMERATOR_STRING(Graphics_select_mode)(select_mode);
					valid_strings = ENUMERATOR_GET_VALID_STRINGS(Graphics_select_mode)(
						&number_of_valid_strings,
						(ENUMERATOR_CONDITIONAL_FUNCTION(Graphics_select_mode) *)NULL,
						(void *)NULL);
					Option_table_add_enumerator(option_table,number_of_valid_strings,
						valid_strings,&select_mode_string);
					DEALLOCATE(valid_strings);
					/* selected_material */
					Option_table_add_entry(option_table,"selected_material",
						&(graphic->selected_material),
						rendition_command_data->graphical_material_manager,
						set_Graphical_material);
					/* spectrum */
					Option_table_add_entry(option_table,"spectrum",
						&(graphic->spectrum),rendition_command_data->spectrum_manager,
						set_Spectrum);
					/* texture_coordinates */
					set_texture_coordinate_field_data.computed_field_manager=
						rendition_command_data->computed_field_manager;
					set_texture_coordinate_field_data.conditional_function=
						Computed_field_has_numerical_components;
					set_texture_coordinate_field_data.conditional_function_user_data=
						(void *)NULL;
					Option_table_add_entry(option_table,"texture_coordinates",
						&(graphic->texture_coordinate_field),
						&set_texture_coordinate_field_data,set_Computed_field_conditional);
					/* use_elements/use_faces/use_lines */
					use_element_type = graphic->use_element_type;
					use_element_type_string =
						ENUMERATOR_STRING(Use_element_type)(use_element_type);
					valid_strings = ENUMERATOR_GET_VALID_STRINGS(Use_element_type)(
						&number_of_valid_strings,
						(ENUMERATOR_CONDITIONAL_FUNCTION(Use_element_type) *)NULL,
						(void *)NULL);
					Option_table_add_enumerator(option_table,number_of_valid_strings,
						valid_strings,&use_element_type_string);
					DEALLOCATE(valid_strings);
					/* visible/invisible */
					Option_table_add_switch(option_table, "visible", "invisible", &visibility);
					if ((return_code=Option_table_multi_parse(option_table,state)))
					{
						if (graphic->iso_values)
						{
							if (range_number_of_iso_values || 
								 graphic->first_iso_value ||
								 graphic->last_iso_value)
							{
								display_message(ERROR_MESSAGE,
									"When specifying a list of <iso_values> do not specify <range_number_of_iso_values>, <first_iso_value> or <last_iso_value>.");
								return_code=0;
							}
						}
						else
						{
							if (range_number_of_iso_values)
							{
								graphic->number_of_iso_values = 
									range_number_of_iso_values;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"You must either specify a list of <iso_values> or a range with <range_number_of_iso_values>, <first_iso_value> or <last_iso_value>.");
								return_code=0;
							}
						}
						if ((struct Computed_field *)NULL==graphic->iso_scalar_field)
						{
							display_message(ERROR_MESSAGE,
								"gfx_modify_rendition_iso_surfaces.  Missing iso_scalar field");
							return_code=0;
						}
						STRING_TO_ENUMERATOR(Use_element_type)(use_element_type_string,
							&use_element_type);
						Cmiss_graphic_set_use_element_type(graphic,
							use_element_type);
						if (graphic->data_field&&!graphic->spectrum)
						{
							graphic->spectrum=ACCESS(Spectrum)(
								rendition_command_data->default_spectrum);
						}
						graphic->visibility = visibility;
						STRING_TO_ENUMERATOR(Graphics_select_mode)(select_mode_string,
							&select_mode);
						Cmiss_graphic_set_select_mode(graphic, select_mode);
						STRING_TO_ENUMERATOR(Render_type)(render_type_string, &render_type);
						Cmiss_graphic_set_render_type(graphic, render_type);
					}
					DESTROY(Option_table)(&option_table);
					if (!return_code)
					{
						/* parse error, help */
						DEACCESS(Cmiss_graphic)(&(modify_rendition_data->graphic));
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_modify_rendition_iso_surfaces.  Could not create graphic");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_modify_rendition_iso_surfaces.  No modify data");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_modify_rendition_iso_surfaces.  "
				"Missing rendition_command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_rendition_iso_surfaces.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_rendition_iso_surfaces */

int gfx_modify_rendition_element_points(struct Parse_state *state,
	void *modify_rendition_data_void,void *rendition_command_data_void)
{
	char *font_name;
	const char *glyph_scaling_mode_string, *select_mode_string, 
		*use_element_type_string,	**valid_strings, *xi_discretization_mode_string;
	enum Graphic_glyph_scaling_mode glyph_scaling_mode;
	enum Graphics_select_mode select_mode;
	enum Use_element_type use_element_type;
	enum Xi_discretization_mode xi_discretization_mode;
	int number_of_components,number_of_valid_strings,return_code,visibility;
	struct Computed_field *orientation_scale_field, *variable_scale_field,
		*xi_point_density_field;
	struct FE_region *fe_region;
	struct Graphics_font *new_font;
	struct Cmiss_graphic *graphic;
	struct GT_object *glyph;
	struct Rendition_command_data *rendition_command_data;
	struct Modify_rendition_data *modify_rendition_data;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_data_field_data, set_label_field_data, set_orientation_scale_field_data,
		set_label_density_field_data, set_variable_scale_field_data, set_xi_point_density_field_data;
	Triple glyph_centre, glyph_scale_factors, glyph_size;

	ENTER(gfx_modify_rendition_element_points);
	if (state && (rendition_command_data=(struct Rendition_command_data *)
		rendition_command_data_void) &&
		(modify_rendition_data=
			(struct Modify_rendition_data *)modify_rendition_data_void) &&
		(fe_region = Cmiss_region_get_FE_region(rendition_command_data->region)))
	{
		/* create the cmiss_graphic: */
		graphic = CREATE(Cmiss_graphic)(CMISS_GRAPHIC_ELEMENT_POINTS);
		/* if we are specifying the name then copy the existings graphic values so
			we can modify from these rather than the default */
		if (modify_rendition_data->graphic)
		{
			if (CMISS_GRAPHIC_ELEMENT_POINTS ==modify_rendition_data->graphic->graphic_type)
			{
				Cmiss_graphic_copy_without_graphics_object(graphic, modify_rendition_data->graphic); 
			}
			DEACCESS(Cmiss_graphic)(&modify_rendition_data->graphic);
		}
		else
		{
			Cmiss_graphic_set_native_discretization_field(
				graphic, modify_rendition_data->native_discretization_field);
			Cmiss_graphic_set_coordinate_field(
				graphic, modify_rendition_data->default_coordinate_field);
			Cmiss_graphic_set_circle_discretization(
				graphic, modify_rendition_data->circle_discretization);
			Cmiss_graphic_set_discretization(
				graphic, &(modify_rendition_data->element_discretization));
		}
		if (graphic)
		{
			/* access since deaccessed in gfx_modify_rendition */
			modify_rendition_data->graphic = ACCESS(Cmiss_graphic)(graphic);
			/* Set up the coordinate_field */
			if (!graphic->coordinate_field)
			{
				struct Computed_field *computed_field;
				struct FE_field *fe_field;
				struct FE_region *fe_region = 
					Cmiss_region_get_FE_region(rendition_command_data->region);
				struct FE_region *data_region = 
					FE_region_get_data_FE_region(fe_region);
				if (FE_region_get_default_coordinate_FE_field(fe_region, &fe_field) ||
					FE_region_get_default_coordinate_FE_field(data_region, &fe_field))
				{
					if (NULL != (computed_field = FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
								Computed_field_is_read_only_with_fe_field,
								(void *)fe_field, rendition_command_data->computed_field_manager)))
					{
						graphic->coordinate_field = 
							ACCESS(Computed_field)(computed_field);
					}
				}
			}
			/* set essential parameters not set by CREATE function */
			if (!Cmiss_graphic_get_material(graphic))
			{
				Cmiss_graphic_set_material(graphic,
					rendition_command_data->default_material);
			}
			if (!Cmiss_graphic_get_selected_material(graphic))
			{
				Cmiss_graphic_set_selected_material(graphic,
					FIND_BY_IDENTIFIER_IN_MANAGER(Graphical_material,name)(
						"default_selected",
						rendition_command_data->graphical_material_manager));
			}
			if (!graphic->font)
			{
				graphic->font = ACCESS(Graphics_font)(
					rendition_command_data->default_font);
			}
			font_name = (char *)NULL;
			orientation_scale_field = (struct Computed_field *)NULL;
			variable_scale_field = (struct Computed_field *)NULL;
			xi_point_density_field = (struct Computed_field *)NULL;
			Cmiss_graphic_get_glyph_parameters(graphic,
				&glyph, &glyph_scaling_mode, glyph_centre, glyph_size,
				&orientation_scale_field, glyph_scale_factors,
				&variable_scale_field);
			/* default to point glyph for fastest possible display */
			if (!glyph)
			{
				glyph = FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)("point",
					rendition_command_data->glyph_list);
			}
			font_name = (char *)NULL;
			ACCESS(GT_object)(glyph);
			if (orientation_scale_field)
			{
				ACCESS(Computed_field)(orientation_scale_field);
			}
			if (variable_scale_field)
			{
				ACCESS(Computed_field)(variable_scale_field);
			}
			Cmiss_graphic_get_xi_discretization(graphic,
				&xi_discretization_mode, &xi_point_density_field);
			if (xi_point_density_field)
			{
				ACCESS(Computed_field)(xi_point_density_field);
			}
			number_of_components = 3;
			visibility = graphic->visibility;

			option_table=CREATE(Option_table)();
			/* as */
			Option_table_add_entry(option_table,"as",&(graphic->name),
				(void *)1,set_name);
			/* cell_centres/cell_corners/cell_density/exact_xi */ 
			xi_discretization_mode_string =
				ENUMERATOR_STRING(Xi_discretization_mode)(xi_discretization_mode);
			valid_strings = ENUMERATOR_GET_VALID_STRINGS(Xi_discretization_mode)(
				&number_of_valid_strings,
				(ENUMERATOR_CONDITIONAL_FUNCTION(Xi_discretization_mode) *)NULL,
				(void *)NULL);
			Option_table_add_enumerator(option_table,number_of_valid_strings,
				valid_strings,&xi_discretization_mode_string);
			DEALLOCATE(valid_strings);
			/* centre */
			Option_table_add_entry(option_table,"centre",glyph_centre,
				&(number_of_components),set_float_vector);
			/* coordinate */
			set_coordinate_field_data.computed_field_manager=
				rendition_command_data->computed_field_manager;
			set_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"coordinate",
				&(graphic->coordinate_field),&set_coordinate_field_data,
				set_Computed_field_conditional);
			/* data */
			set_data_field_data.computed_field_manager=
				rendition_command_data->computed_field_manager;
			set_data_field_data.conditional_function=
				Computed_field_has_numerical_components;
			set_data_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"data",&(graphic->data_field),
				&set_data_field_data,set_Computed_field_conditional);
			/* delete */
			Option_table_add_entry(option_table,"delete",
				&(modify_rendition_data->delete_flag),NULL,set_char_flag);
			/* density */
			set_xi_point_density_field_data.computed_field_manager =
				rendition_command_data->computed_field_manager;
			set_xi_point_density_field_data.conditional_function =
				Computed_field_is_scalar;
			set_xi_point_density_field_data.conditional_function_user_data =
				(void *)NULL;
			Option_table_add_entry(option_table, "density",
				&xi_point_density_field, &set_xi_point_density_field_data,
				set_Computed_field_conditional);
			/* discretization */
			Option_table_add_entry(option_table,"discretization",
				&(graphic->discretization),rendition_command_data->user_interface,
				set_Element_discretization);
			/* exterior */
			Option_table_add_entry(option_table,"exterior",&(graphic->exterior),
				NULL,set_char_flag);
			/* face */
			Option_table_add_entry(option_table,"face",&(graphic->face),
				NULL,set_exterior);
			/* font */
			Option_table_add_name_entry(option_table, "font",
				&font_name);
			/* glyph */
			Option_table_add_entry(option_table,"glyph",&glyph,
				rendition_command_data->glyph_list,set_Graphics_object);
			/* label */
			set_label_field_data.computed_field_manager=
				rendition_command_data->computed_field_manager;
			set_label_field_data.conditional_function=
				(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
			set_label_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"label",&(graphic->label_field),
				&set_label_field_data,set_Computed_field_conditional);
			/* ldensity */
			set_label_density_field_data.computed_field_manager=
				rendition_command_data->computed_field_manager;
			set_label_density_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_label_density_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"ldensity",&(graphic->label_density_field),
				&set_label_density_field_data,set_Computed_field_conditional);
			/* material */
			Option_table_add_entry(option_table,"material",&(graphic->material),
				rendition_command_data->graphical_material_manager,
				set_Graphical_material);
			/* native_discretization */
			Option_table_add_set_FE_field_from_FE_region(option_table,
				"native_discretization", &(graphic->native_discretization_field),
				fe_region);
			/* glyph scaling mode */
			glyph_scaling_mode_string =
				ENUMERATOR_STRING(Graphic_glyph_scaling_mode)(glyph_scaling_mode);
			valid_strings = ENUMERATOR_GET_VALID_STRINGS(Graphic_glyph_scaling_mode)(
				&number_of_valid_strings,
				(ENUMERATOR_CONDITIONAL_FUNCTION(Graphic_glyph_scaling_mode) *)NULL,
				(void *)NULL);
			Option_table_add_enumerator(option_table, number_of_valid_strings,
				valid_strings, &glyph_scaling_mode_string);
			DEALLOCATE(valid_strings);
			/* orientation */
			set_orientation_scale_field_data.computed_field_manager=
				rendition_command_data->computed_field_manager;
			set_orientation_scale_field_data.conditional_function=
				Computed_field_is_orientation_scale_capable;
			set_orientation_scale_field_data.conditional_function_user_data=
				(void *)NULL;
			Option_table_add_entry(option_table,"orientation",
				&orientation_scale_field,&set_orientation_scale_field_data,
				set_Computed_field_conditional);
			/* position */
			Option_table_add_entry(option_table,"position",
				&(modify_rendition_data->position),NULL,set_int_non_negative);
			/* scale_factors */
			Option_table_add_special_float3_entry(option_table,"scale_factors",
				glyph_scale_factors,"*");
			/* scene */
			Option_table_add_entry(option_table,"scene",
				&(modify_rendition_data->scene),
				rendition_command_data->scene_manager,set_Scene);
			/* select_mode */
			select_mode = Cmiss_graphic_get_select_mode(graphic);
			select_mode_string =
				ENUMERATOR_STRING(Graphics_select_mode)(select_mode);
			valid_strings = ENUMERATOR_GET_VALID_STRINGS(Graphics_select_mode)(
				&number_of_valid_strings,
				(ENUMERATOR_CONDITIONAL_FUNCTION(Graphics_select_mode) *)NULL,
				(void *)NULL);
			Option_table_add_enumerator(option_table,number_of_valid_strings,
				valid_strings,&select_mode_string);
			DEALLOCATE(valid_strings);
			/* selected_material */
			Option_table_add_entry(option_table,"selected_material",
				&(graphic->selected_material),
				rendition_command_data->graphical_material_manager,
				set_Graphical_material);
			/* size */
			Option_table_add_special_float3_entry(option_table,"size",
				glyph_size,"*");
			/* spectrum */
			Option_table_add_entry(option_table,"spectrum",
				&(graphic->spectrum),rendition_command_data->spectrum_manager,
				set_Spectrum);
			/* use_elements/use_faces/use_lines */
			use_element_type = graphic->use_element_type;
			use_element_type_string =
				ENUMERATOR_STRING(Use_element_type)(use_element_type);
			valid_strings = ENUMERATOR_GET_VALID_STRINGS(Use_element_type)(
				&number_of_valid_strings,
				(ENUMERATOR_CONDITIONAL_FUNCTION(Use_element_type) *)NULL,
				(void *)NULL);
			Option_table_add_enumerator(option_table,number_of_valid_strings,
				valid_strings,&use_element_type_string);
			DEALLOCATE(valid_strings);
			/* xi */
			Option_table_add_entry(option_table,"xi",
				graphic->seed_xi,&number_of_components,set_float_vector);
			/* variable_scale */
			set_variable_scale_field_data.computed_field_manager =
				rendition_command_data->computed_field_manager;
			set_variable_scale_field_data.conditional_function =
				Computed_field_has_up_to_3_numerical_components;
			set_variable_scale_field_data.conditional_function_user_data =
				(void *)NULL;
			Option_table_add_entry(option_table,"variable_scale",
				&variable_scale_field, &set_variable_scale_field_data,
				set_Computed_field_conditional);
			/* visible/invisible */
			Option_table_add_switch(option_table, "visible", "invisible", &visibility);
			if ((return_code=Option_table_multi_parse(option_table,state)))
			{
				if (graphic->data_field&&!graphic->spectrum)
				{
					graphic->spectrum=ACCESS(Spectrum)(
						rendition_command_data->default_spectrum);
				}
				graphic->visibility = visibility;
				STRING_TO_ENUMERATOR(Xi_discretization_mode)(
					xi_discretization_mode_string, &xi_discretization_mode);
				if (((XI_DISCRETIZATION_CELL_DENSITY != xi_discretization_mode) &&
					(XI_DISCRETIZATION_CELL_POISSON != xi_discretization_mode)) ||
					xi_point_density_field)
				{
					Cmiss_graphic_set_xi_discretization(graphic,
						xi_discretization_mode, xi_point_density_field);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"No density field specified for cell_density|cell_poisson");
					return_code = 0;
				}
				STRING_TO_ENUMERATOR(Use_element_type)(use_element_type_string,
					&use_element_type);
				Cmiss_graphic_set_use_element_type(graphic,
					use_element_type);
				if (font_name && (new_font = Graphics_font_package_get_font
						(rendition_command_data->graphics_font_package, font_name)))
				{
					REACCESS(Graphics_font)(&graphic->font, new_font);
				}
				if (font_name)
				{
					DEALLOCATE(font_name);
				}
				if (glyph)
				{
					STRING_TO_ENUMERATOR(Graphic_glyph_scaling_mode)(
						glyph_scaling_mode_string, &glyph_scaling_mode);
					Cmiss_graphic_set_glyph_parameters(graphic,
						glyph, glyph_scaling_mode, glyph_centre, glyph_size,
						orientation_scale_field,glyph_scale_factors,
						variable_scale_field);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"No glyph specified for element_points");
					return_code=0;
				}
				STRING_TO_ENUMERATOR(Graphics_select_mode)(select_mode_string,
					&select_mode);
				Cmiss_graphic_set_select_mode(graphic, select_mode);
			}
			DESTROY(Option_table)(&option_table);
			if (!return_code)
			{
				/* parse error, help */
				DEACCESS(Cmiss_graphic)(&(modify_rendition_data->graphic));
			}
			if (glyph)
			{
				DEACCESS(GT_object)(&glyph);
			}
			if (orientation_scale_field)
			{
				DEACCESS(Computed_field)(&orientation_scale_field);
			}
			if (variable_scale_field)
			{
				DEACCESS(Computed_field)(&variable_scale_field);
			}
			if (xi_point_density_field)
			{
				DEACCESS(Computed_field)(&xi_point_density_field);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_modify_rendition_element_points.  Could not create graphic");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_rendition_element_points.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_rendition_element_points */

int gfx_modify_rendition_streamlines(struct Parse_state *state,
	void *modify_rendition_data_void,void *rendition_command_data_void)
{
	char reverse_track;
	const char *select_mode_string,
		*streamline_data_type_string,*streamline_type_string,**valid_strings;
	enum Graphics_select_mode select_mode;
	enum Streamline_type streamline_type;
	enum Streamline_data_type streamline_data_type;
	float length, width;
	int number_of_components,number_of_valid_strings,return_code,
		reverse_track_int, visibility;
	struct Computed_field *stream_vector_field;
	struct Cmiss_region *seed_node_region;
	struct FE_region *fe_region;
	struct Cmiss_graphic *graphic;
	struct Rendition_command_data *rendition_command_data;
	struct Modify_rendition_data *modify_rendition_data;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_data_field_data, set_seed_node_coordinate_field_data,
		set_stream_vector_field_data;

	ENTER(gfx_modify_rendition_streamlines);
	if (state && (rendition_command_data =
		(struct Rendition_command_data *)rendition_command_data_void) &&
		(modify_rendition_data =
			(struct Modify_rendition_data *)modify_rendition_data_void) &&
		(fe_region = Cmiss_region_get_FE_region(rendition_command_data->region)))
	{
		/* create the cmiss_graphic: */
		graphic = CREATE(Cmiss_graphic)(CMISS_GRAPHIC_STREAMLINES);
		/* if we are specifying the name then copy the existings graphic values so
			we can modify from these rather than the default */
		if (modify_rendition_data->graphic)
		{
			if (CMISS_GRAPHIC_STREAMLINES ==modify_rendition_data->graphic->graphic_type)
			{
				Cmiss_graphic_copy_without_graphics_object(graphic, modify_rendition_data->graphic); 
			}
			DEACCESS(Cmiss_graphic)(&modify_rendition_data->graphic);
		}
		else
		{
			Cmiss_graphic_set_native_discretization_field(
				graphic, modify_rendition_data->native_discretization_field);
			Cmiss_graphic_set_coordinate_field(
				graphic, modify_rendition_data->default_coordinate_field);
			Cmiss_graphic_set_circle_discretization(
				graphic, modify_rendition_data->circle_discretization);
			Cmiss_graphic_set_discretization(
				graphic, &(modify_rendition_data->element_discretization));
		}
		if (graphic)
		{
			/* access since deaccessed in gfx_modify_rendition */
			modify_rendition_data->graphic = ACCESS(Cmiss_graphic)(graphic);
			/* Set up the coordinate_field */
			if (!graphic->coordinate_field)
			{
				struct Computed_field *computed_field;
				struct FE_field *fe_field;
				struct FE_region *fe_region = 
					Cmiss_region_get_FE_region(rendition_command_data->region);
				struct FE_region *data_region = 
					FE_region_get_data_FE_region(fe_region);
				if (FE_region_get_default_coordinate_FE_field(fe_region, &fe_field) ||
					FE_region_get_default_coordinate_FE_field(data_region, &fe_field))
				{
					if (NULL != (computed_field = FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
								Computed_field_is_read_only_with_fe_field,
								(void *)fe_field, rendition_command_data->computed_field_manager)))
					{
						graphic->coordinate_field = 
							ACCESS(Computed_field)(computed_field);
					}
				}
			}
			/* set essential parameters not set by CREATE function */
			if (!Cmiss_graphic_get_material(graphic))
			{
				Cmiss_graphic_set_material(graphic,
					rendition_command_data->default_material);
			}
			if (!Cmiss_graphic_get_selected_material(graphic))
			{
				Cmiss_graphic_set_selected_material(graphic,
					FIND_BY_IDENTIFIER_IN_MANAGER(Graphical_material,name)(
						"default_selected",
						rendition_command_data->graphical_material_manager));
			}
			/* The value stored in the graphic is an integer rather than a char */
			if (graphic->reverse_track)
			{
				reverse_track = 1;
			}
			else
			{
				reverse_track = 0;
			}
			visibility = graphic->visibility;
			option_table=CREATE(Option_table)();
			/* as */
			Option_table_add_entry(option_table,"as",&(graphic->name),
				(void *)1,set_name);
			/* coordinate */
			set_coordinate_field_data.computed_field_manager=
				rendition_command_data->computed_field_manager;
			set_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"coordinate",
				&(graphic->coordinate_field),&set_coordinate_field_data,
				set_Computed_field_conditional);
			/* data */
			set_data_field_data.computed_field_manager=
				rendition_command_data->computed_field_manager;
			set_data_field_data.conditional_function=
				Computed_field_has_numerical_components;
			set_data_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"data",&(graphic->data_field),
				&set_data_field_data,set_Computed_field_conditional);
			/* delete */
			Option_table_add_entry(option_table,"delete",
				&(modify_rendition_data->delete_flag),NULL,set_char_flag);
			/* discretization */
			Option_table_add_entry(option_table,"discretization",
				&(graphic->discretization),rendition_command_data->user_interface,
				set_Element_discretization);
			/* ellipse/line/rectangle/ribbon */
			streamline_type = STREAM_LINE;
			streamline_type_string =
				ENUMERATOR_STRING(Streamline_type)(streamline_type);
			valid_strings = ENUMERATOR_GET_VALID_STRINGS(Streamline_type)(
				&number_of_valid_strings,
				(ENUMERATOR_CONDITIONAL_FUNCTION(Streamline_type) *)NULL,
				(void *)NULL);
			Option_table_add_enumerator(option_table,number_of_valid_strings,
				valid_strings,&streamline_type_string);
			DEALLOCATE(valid_strings);
			/* length */
			Option_table_add_entry(option_table,"length",
				&(graphic->streamline_length),NULL,set_float);
			/* material */
			Option_table_add_entry(option_table,"material",&(graphic->material),
				rendition_command_data->graphical_material_manager,
				set_Graphical_material);
			/* no_data/field_scalar/magnitude_scalar/travel_scalar */
			streamline_data_type = STREAM_NO_DATA;
			streamline_data_type_string =
				ENUMERATOR_STRING(Streamline_data_type)(streamline_data_type);
			valid_strings = ENUMERATOR_GET_VALID_STRINGS(Streamline_data_type)(
				&number_of_valid_strings,
				(ENUMERATOR_CONDITIONAL_FUNCTION(Streamline_data_type) *)NULL,
				(void *)NULL);
			Option_table_add_enumerator(option_table, number_of_valid_strings,
				valid_strings, &streamline_data_type_string);
			DEALLOCATE(valid_strings);
			/* position */
			Option_table_add_entry(option_table,"position",
				&(modify_rendition_data->position),NULL,set_int_non_negative);
			/* reverse */
			/*???RC use negative length to denote reverse track instead? */
			Option_table_add_entry(option_table,"reverse_track",
				&reverse_track,NULL,set_char_flag);
			/* scene */
			Option_table_add_entry(option_table,"scene",
				&(modify_rendition_data->scene),
				rendition_command_data->scene_manager,set_Scene);
			/* seed_element */
			Option_table_add_entry(option_table, "seed_element",
				&(graphic->seed_element), fe_region,
				set_FE_element_top_level_FE_region);
			/* seed_node_region */
			Option_table_add_entry(option_table, "seed_node_region",
				&graphic->seed_node_region_path,
				rendition_command_data->root_region, set_Cmiss_region_path);
			/* seed_node_coordinate_field */
			set_seed_node_coordinate_field_data.computed_field_manager=
				rendition_command_data->computed_field_manager;
			set_seed_node_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_seed_node_coordinate_field_data.conditional_function_user_data=
				(void *)NULL;
			Option_table_add_entry(option_table,"seed_node_coordinate_field",
				&(graphic->seed_node_coordinate_field),&set_seed_node_coordinate_field_data,
				set_Computed_field_conditional);
			/* select_mode */
			select_mode = Cmiss_graphic_get_select_mode(graphic);
			select_mode_string =
				ENUMERATOR_STRING(Graphics_select_mode)(select_mode);
			valid_strings = ENUMERATOR_GET_VALID_STRINGS(Graphics_select_mode)(
				&number_of_valid_strings,
				(ENUMERATOR_CONDITIONAL_FUNCTION(Graphics_select_mode) *)NULL,
				(void *)NULL);
			Option_table_add_enumerator(option_table,number_of_valid_strings,
				valid_strings,&select_mode_string);
			DEALLOCATE(valid_strings);
			/* selected_material */
			Option_table_add_entry(option_table,"selected_material",
				&(graphic->selected_material),
				rendition_command_data->graphical_material_manager,
				set_Graphical_material);
			/* spectrum */
			Option_table_add_entry(option_table,"spectrum",
				&(graphic->spectrum),rendition_command_data->spectrum_manager,
				set_Spectrum);
			/* vector */
			set_stream_vector_field_data.computed_field_manager=
				rendition_command_data->computed_field_manager;
			set_stream_vector_field_data.conditional_function=
				Computed_field_is_stream_vector_capable;
			set_stream_vector_field_data.conditional_function_user_data=
				(void *)NULL;
			Option_table_add_entry(option_table,"vector",
				&(graphic->stream_vector_field),&set_stream_vector_field_data,
				set_Computed_field_conditional);
			/* width */
			Option_table_add_entry(option_table,"width",
				&(graphic->streamline_width),NULL,set_float);
			/* visible/invisible */
			Option_table_add_switch(option_table, "visible", "invisible", &visibility);
			/* xi */
			number_of_components = 3;
			Option_table_add_entry(option_table,"xi",
				graphic->seed_xi,&number_of_components,set_float_vector);
			if ((return_code = Option_table_multi_parse(option_table,state)))
			{
				graphic->visibility = visibility;
				if (!(graphic->stream_vector_field))
				{
					display_message(ERROR_MESSAGE,"Must specify a vector");
					return_code=0;
				}
				if (graphic->seed_node_region_path)
				{
					seed_node_region = Cmiss_region_find_subregion_at_path(
						rendition_command_data->root_region,
						graphic->seed_node_region_path);
					if (!(seed_node_region &&
						(REACCESS(FE_region)(&graphic->seed_node_region,
						Cmiss_region_get_FE_region(seed_node_region)))))
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_streamlines.  Invalid seed_node_region");
						return_code = 0;
					}
					Cmiss_region_destroy(&seed_node_region);
				}
				if ((graphic->seed_node_coordinate_field && (!graphic->seed_node_region)) ||
					((!graphic->seed_node_coordinate_field) && graphic->seed_node_region))
				{
					display_message(ERROR_MESSAGE,
						"If you specify a seed_node_coordinate_field then you must also specify a "
						"seed_node_region");
					return_code = 0;
				}
				if (return_code)
				{
					Cmiss_graphic_get_streamline_parameters(graphic,
						&streamline_type,&stream_vector_field,&reverse_track_int,
						&length,&width);
					STRING_TO_ENUMERATOR(Streamline_type)(streamline_type_string,
						&streamline_type);
					STRING_TO_ENUMERATOR(Streamline_data_type)(
						streamline_data_type_string, &streamline_data_type);
					if (graphic->data_field)
					{
						if (STREAM_FIELD_SCALAR != streamline_data_type)
						{
							display_message(WARNING_MESSAGE,
								"Must use field_scalar option with data; ensuring this");
							streamline_data_type=STREAM_FIELD_SCALAR;
						}
					}
					else
					{
						if (STREAM_FIELD_SCALAR == streamline_data_type)
						{
							display_message(WARNING_MESSAGE,
								"Must specify data field with field_scalar option");
							streamline_data_type=STREAM_NO_DATA;
						}
					}
					if ((STREAM_NO_DATA!=streamline_data_type)&&!graphic->spectrum)
					{
						graphic->spectrum=ACCESS(Spectrum)(
							rendition_command_data->default_spectrum);
					}
					Cmiss_graphic_set_streamline_parameters(
						graphic,streamline_type,stream_vector_field,(int)reverse_track,
						length,width);
					Cmiss_graphic_set_data_spectrum_parameters_streamlines(
						graphic,streamline_data_type,graphic->data_field,
						graphic->spectrum);
				}
				STRING_TO_ENUMERATOR(Graphics_select_mode)(select_mode_string,
					&select_mode);
				Cmiss_graphic_set_select_mode(graphic, select_mode);
			}
			DESTROY(Option_table)(&option_table);
			if (!return_code)
			{
				/* parse error, help */
				DEACCESS(Cmiss_graphic)(&(modify_rendition_data->graphic));
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_modify_rendition_streamlines.  Could not create graphic");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_rendition_streamlines.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_rendition_streamlines */

int Cmiss_graphic_list_contents(struct Cmiss_graphic *graphic,
	void *list_data_void)
{
	int return_code;
	char *graphic_string,line[40];
	struct Cmiss_graphic_list_data *list_data;

	ENTER(Cmiss_graphic_list_contents);
	if (graphic&&
		NULL != (list_data=(struct Cmiss_graphic_list_data *)list_data_void))
	{
		if (NULL != (graphic_string=Cmiss_graphic_string(graphic,
					list_data->graphic_string_detail)))
		{
			if (list_data->line_prefix)
			{
				display_message(INFORMATION_MESSAGE,list_data->line_prefix);
			}
			display_message(INFORMATION_MESSAGE,graphic_string);
			if (list_data->line_suffix)
			{
				display_message(INFORMATION_MESSAGE,list_data->line_suffix);
			}
			/*???RC temp */
			if ((GRAPHIC_STRING_COMPLETE_PLUS==list_data->graphic_string_detail)&&
				(graphic->access_count != 1))
			{
				sprintf(line," (access count = %i)",graphic->access_count);
				display_message(INFORMATION_MESSAGE,line);
			}
			display_message(INFORMATION_MESSAGE,"\n");
			DEALLOCATE(graphic_string);
			return_code=1;
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_list_contents.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

          	return (return_code);
} /* Cmiss_graphic_list_contents */

int Cmiss_graphic_Graphical_material_change(
	struct Cmiss_graphic *graphic, void *material_change_data_void)
{
	int return_code;
	struct Cmiss_graphic_Graphical_material_change_data
		*material_change_data;

	ENTER(Cmiss_graphic_Graphical_material_change);
	if (graphic && (material_change_data =
		(struct Cmiss_graphic_Graphical_material_change_data *)
		material_change_data_void))
	{
		if (graphic->material && (IS_OBJECT_IN_LIST(Graphical_material)(
			graphic->material, material_change_data->changed_material_list) ||
			(graphic->secondary_material &&
				IS_OBJECT_IN_LIST(Graphical_material)(graphic->secondary_material,
					material_change_data->changed_material_list)) ||
			(graphic->selected_material && 
				IS_OBJECT_IN_LIST(Graphical_material)(graphic->selected_material,
					material_change_data->changed_material_list))))
		{
			if (graphic->graphics_object)
			{
				GT_object_Graphical_material_change(graphic->graphics_object,
					(struct LIST(Graphical_material) *)NULL);
			}
			/* need a way to tell either graphic is used in any scene or not */
			material_change_data->changed = 1;
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_Graphical_material_change.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_Graphical_material_change */

int Cmiss_graphic_get_position_in_list(
	struct Cmiss_graphic *graphic,
	struct LIST(Cmiss_graphic) *list_of_graphic)
{
	int position;

	ENTER(Cmiss_graphic_get_position_in_list);
	if (graphic&&list_of_graphic)
	{
		if (IS_OBJECT_IN_LIST(Cmiss_graphic)(graphic,list_of_graphic))
		{
			position=graphic->position;
		}
		else
		{
			position=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_position_in_list.  Invalid argument(s)");
		position=0;
	}
	LEAVE;

	return (position);
} /* Cmiss_graphic_get_position_in_list */

int Cmiss_graphic_match(struct Cmiss_graphic *graphic1,
	struct Cmiss_graphic *graphic2)
{
	int return_code;

	ENTER(Cmiss_graphic_match);
	if (graphic1 && graphic2)
	{
		return_code=
			Cmiss_graphic_same_geometry(graphic1, (void *)graphic2) &&
			(graphic1->visibility == graphic2->visibility) &&
			(graphic1->material == graphic2->material) &&
			(graphic1->secondary_material == graphic2->secondary_material) &&
			(graphic1->line_width == graphic2->line_width) &&
			(graphic1->selected_material == graphic2->selected_material) &&
			(graphic1->data_field == graphic2->data_field) &&
			(graphic1->spectrum == graphic2->spectrum) &&
			(graphic1->font == graphic2->font) &&
			(graphic1->render_type == graphic2->render_type) &&
			(graphic1->texture_coordinate_field ==
				graphic2->texture_coordinate_field) &&
			((CMISS_GRAPHIC_STREAMLINES != graphic1->graphic_type) ||
				(graphic1->streamline_data_type ==
					graphic2->streamline_data_type));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_match.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_match */

int Cmiss_graphic_copy_and_put_in_list(
	struct Cmiss_graphic *graphic,void *list_of_graphic_void)
{
	int return_code;
	struct Cmiss_graphic *copy_graphic;
	struct LIST(Cmiss_graphic) *list_of_graphic;

	ENTER(Cmiss_graphic_copy_and_put_in_list);
	if (graphic&&NULL != (list_of_graphic=
		(struct LIST(Cmiss_graphic) *)list_of_graphic_void))
	{
		/* create new graphic to take the copy */
		if (NULL != (copy_graphic=CREATE(Cmiss_graphic)(graphic->graphic_type)))
		{
			/* copy and insert in list */
			if (!(return_code=Cmiss_graphic_copy_without_graphics_object(
				copy_graphic,graphic)&&
				ADD_OBJECT_TO_LIST(Cmiss_graphic)(copy_graphic,
					list_of_graphic)))
			{
				DESTROY(Cmiss_graphic)(&copy_graphic);
				display_message(ERROR_MESSAGE,
					"Cmiss_graphic_copy_and_put_in_list.  "
					"Could not put copy in list");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_graphic_copy_and_put_in_list.  Could not create copy");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_copy_and_put_in_list.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_copy_and_put_in_list */

int Cmiss_graphic_type_matches(struct Cmiss_graphic *graphic,
	void *graphic_type_void)
{
	int return_code;

	ENTER(Cmiss_graphic_type_matches);
	if (graphic)
	{
		return_code=((void *)graphic->graphic_type == graphic_type_void);
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_graphic_type_matches.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_type_matches */

/***************************************************************************//**
 * Cmiss_graphic list conditional function returning 1 iff the two
 * graphic have the same geometry and the same nontrivial appearance
 * characteristics. Trivial appearance characteristics are the material,
 * visibility and spectrum.
 */
int Cmiss_graphic_same_non_trivial(struct Cmiss_graphic *graphic,
	void *second_graphic_void)
{
	int return_code;
	struct Cmiss_graphic *second_graphic;

	ENTER(Cmiss_graphic_same_non_trivial);
	if (graphic
		&&(second_graphic=(struct Cmiss_graphic *)second_graphic_void))
	{
		return_code=
			Cmiss_graphic_same_geometry(graphic,second_graphic_void)&&
			(graphic->data_field==second_graphic->data_field)&&
			(graphic->render_type==second_graphic->render_type)&&
			(graphic->line_width==second_graphic->line_width)&&
			(graphic->texture_coordinate_field==second_graphic->texture_coordinate_field)&&
			((CMISS_GRAPHIC_STREAMLINES != graphic->graphic_type)||
				(graphic->streamline_data_type==
					second_graphic->streamline_data_type));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_same_non_trivial.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_same_non_trivial */


/***************************************************************************//**
 * Same as Cmiss_graphic_same_non_trivial except <graphic> must also have
 * a graphics_object. Used for getting graphics objects from previous graphic
 * that are the same except for trivial differences such as the material and
 * spectrum which can be changed in the graphics object to match the new graphic .
 */
int Cmiss_graphic_same_non_trivial_with_graphics_object(
	struct Cmiss_graphic *graphic,void *second_graphic_void)
{
	int return_code;

	ENTER(Cmiss_graphic_same_non_trivial_with_graphics_object);

	if (graphic)
	{
		return_code=graphic->graphics_object&&
			Cmiss_graphic_same_non_trivial(graphic,second_graphic_void);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_same_non_trivial_with_graphics_object.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_same_non_trivial_with_graphics_object */

/***************************************************************************//**
 * If <graphic> does not already have a graphics object, this function attempts
 * to find graphic in <list_of_graphic> which differ only trivially in material,
 * spectrum etc. AND have a graphics object. If such a graphic is found, the
 * graphics_object is moved from the matching graphic and put in <graphic>, while
 * any trivial differences are fixed up in the graphics_obejct.
 */
int Cmiss_graphic_extract_graphics_object_from_list(
	struct Cmiss_graphic *graphic,void *list_of_graphic_void)
{
	int return_code;
	struct Cmiss_graphic *matching_graphic;
	struct LIST(Cmiss_graphic) *list_of_graphic;

	ENTER(Cmiss_graphic_extract_graphics_object_from_list);
	if (graphic&&(list_of_graphic=
		(struct LIST(Cmiss_graphic) *)list_of_graphic_void))
	{
		return_code = 1;
		if (!(graphic->graphics_object))
		{
			if (NULL != (matching_graphic = FIRST_OBJECT_IN_LIST_THAT(Cmiss_graphic)(
				Cmiss_graphic_same_non_trivial_with_graphics_object,
				(void *)graphic,list_of_graphic)))
			{
				/* make sure graphics_changed and selected_graphics_changed flags
					 are brought across */
				graphic->graphics_object = matching_graphic->graphics_object;
				/* make sure graphic and graphics object have same material and
					 spectrum */
				Cmiss_graphic_update_non_trivial_GT_objects(graphic);
				graphic->graphics_changed = matching_graphic->graphics_changed;
				graphic->selected_graphics_changed =
					matching_graphic->selected_graphics_changed;
// 				graphic->overlay_flag = matching_graphic->overlay_flag;
// 				graphic->overlay_order = matching_graphic->overlay_order;
				/* reset graphics_object and flags in matching_graphic */
				matching_graphic->graphics_object = (struct GT_object *)NULL;
				matching_graphic->graphics_changed = 1;
				matching_graphic->selected_graphics_changed = 1;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_extract_graphics_object_from_list.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_extract_graphics_object_from_list */

int Cmiss_graphic_set_volume_texture(struct Cmiss_graphic *graphic,
	struct VT_volume_texture *volume_texture)
{
	int return_code;

	ENTER(Cmiss_graphic_set_volume_texture);
	if (graphic&&volume_texture&&
		(CMISS_GRAPHIC_VOLUMES==graphic->graphic_type))
	{
		REACCESS(VT_volume_texture)(&(graphic->volume_texture),volume_texture);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_volume_texture.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_set_volume_texture */

int Cmiss_graphic_get_data_spectrum_parameters(
	struct Cmiss_graphic *graphic,
	struct Computed_field **data_field,struct Spectrum **spectrum)
{
	int return_code;

	ENTER(Cmiss_graphic_get_data_spectrum_parameters);
	if (graphic&&data_field&&spectrum&&
		(CMISS_GRAPHIC_STREAMLINES != graphic->graphic_type))
	{
		*data_field=graphic->data_field;
		*spectrum=graphic->spectrum;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_data_spectrum_parameters.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_get_data_spectrum_parameters */

int Cmiss_graphic_get_radius_parameters(
	struct Cmiss_graphic *graphic,float *constant_radius,
	float *radius_scale_factor,struct Computed_field **radius_scalar_field)
{
	int return_code;

	ENTER(Cmiss_graphic_get_radius_parameters);
	if (graphic&&constant_radius&&radius_scale_factor&&radius_scalar_field&&
		(CMISS_GRAPHIC_CYLINDERS==graphic->graphic_type))
	{
		return_code=1;
		*constant_radius=graphic->constant_radius;
		*radius_scale_factor=graphic->radius_scale_factor;
		*radius_scalar_field=graphic->radius_scalar_field;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_radius_parameters.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_get_radius_parameters */

int Cmiss_graphic_get_label_field(struct Cmiss_graphic *graphic,
	struct Computed_field **label_field, struct Graphics_font **font)
{
	int return_code;

	ENTER(Cmiss_graphic_get_label_field);
	if (graphic&&
		((CMISS_GRAPHIC_NODE_POINTS==graphic->graphic_type)||
			(CMISS_GRAPHIC_DATA_POINTS==graphic->graphic_type)||
			(CMISS_GRAPHIC_ELEMENT_POINTS==graphic->graphic_type)||
			(CMISS_GRAPHIC_STATIC==graphic->graphic_type)))
	{
		*label_field = graphic->label_field;
		*font = graphic->font;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_label_field.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_get_label_field */

int Cmiss_graphic_get_visibility_field(struct Cmiss_graphic *graphic,
	struct Computed_field **visibility_field)
{
	int return_code;


	ENTER(Cmiss_graphic_get_label_field);
	if (graphic&&
		((CMISS_GRAPHIC_NODE_POINTS==graphic->graphic_type)||
			(CMISS_GRAPHIC_DATA_POINTS==graphic->graphic_type)))
	{
		*visibility_field = graphic->visibility_field;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_label_field.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

int Cmiss_graphic_set_visibility_field(
	struct Cmiss_graphic *graphic,struct Computed_field *visibility_field)
{
	int return_code;

	ENTER(Cmiss_graphic_set_visibility_field);
	if (graphic&&
		((CMISS_GRAPHIC_NODE_POINTS==graphic->graphic_type)||
			(CMISS_GRAPHIC_DATA_POINTS==graphic->graphic_type)))
	{
		REACCESS(Computed_field)(&(graphic->visibility_field), visibility_field);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_visibility_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_set_label_field */

enum Use_element_type Cmiss_graphic_get_use_element_type(
	struct Cmiss_graphic *graphic)
{
	enum Use_element_type use_element_type;

	ENTER(Cmiss_graphic_get_use_element_type);
	if (graphic&&((CMISS_GRAPHIC_ELEMENT_POINTS==graphic->graphic_type)||
		(CMISS_GRAPHIC_ISO_SURFACES==graphic->graphic_type)))
	{
		use_element_type=graphic->use_element_type;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_use_element_type.  Invalid argument(s)");
		use_element_type = USE_ELEMENTS;
	}
	LEAVE;

	return (use_element_type);
} /* Cmiss_graphic_get_use_element_type */

int Cmiss_graphic_get_discretization(struct Cmiss_graphic *graphic,
	struct Element_discretization *discretization)
{
	int return_code;

	ENTER(Cmiss_graphic_get_discretization);
	if (graphic&&discretization)
	{
		discretization->number_in_xi1=graphic->discretization.number_in_xi1;
		discretization->number_in_xi2=graphic->discretization.number_in_xi2;
		discretization->number_in_xi3=graphic->discretization.number_in_xi3;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_discretization.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_get_discretization */

struct FE_field *Cmiss_graphic_get_native_discretization_field(
	struct Cmiss_graphic *graphic)
{
	struct FE_field *native_discretization_field;

	ENTER(Cmiss_graphic_get_native_discretization_field);
	if (graphic)
	{
		native_discretization_field=graphic->native_discretization_field;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_native_discretization_field.  "
			"Invalid argument(s)");
		native_discretization_field=(struct FE_field *)NULL;
	}
	LEAVE;

	return (native_discretization_field);
} /* Cmiss_graphic_get_native_discretization_field */

int Cmiss_graphic_set_native_discretization_field(
	struct Cmiss_graphic *graphic, struct FE_field *native_discretization_field)
{
	int return_code;

	ENTER(Cmiss_graphic_set_native_discretization_field);
	if (graphic)
	{
		return_code=1;
		REACCESS(FE_field)(&(graphic->native_discretization_field),
			native_discretization_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_native_discretization_field.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}

int Cmiss_graphic_set_general_native_discretization_field(
	struct Cmiss_graphic *graphic, void *native_discretization_field_void)
{
	int return_code;
	struct FE_field *native_discretization_field =
		(struct FE_field *)native_discretization_field_void;
	if (graphic && native_discretization_field)
	{
		if (graphic->native_discretization_field != native_discretization_field)
		{
			Cmiss_graphic_set_native_discretization_field(
				graphic,native_discretization_field);
			Cmiss_graphic_element_discretization_change(
				graphic, (void *)NULL);
		}
		return_code = 1;
	}
	else

	{
		return_code = 0;
	}
	return return_code;
}

int Cmiss_graphic_get_circle_discretization(struct Cmiss_graphic *graphic)
{
	int circle_discretization;

	ENTER(Cmiss_graphic_get_discretization);
	if (graphic)
	{
	 circle_discretization = graphic->circle_discretization;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_circle_discretization.  Invalid argument(s)");
		circle_discretization = 0;
	}
	LEAVE;

	return (circle_discretization);
} /* Cmiss_graphic_get_discretization */

int Cmiss_graphic_set_circle_discretization(
	struct Cmiss_graphic *graphic,int circle_discretization)
{
	int return_code;

	ENTER(Cmiss_graphic_set_circle_discretization);
	if (graphic)
	{
		if (graphic->circle_discretization != circle_discretization
		    && (CMISS_GRAPHIC_CYLINDERS==graphic->graphic_type))
		{
			graphic->circle_discretization = circle_discretization;
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_circle_discretization.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}

int Cmiss_graphic_set_general_circle_discretization(
	struct Cmiss_graphic *graphic,void *circle_discretization_void)
{
	int return_code;
	int *circle_discretization;

	ENTER(Cmiss_graphic_set_general_circle_discretization);
	if (graphic &&
	    (circle_discretization = (int *)circle_discretization_void))
	{
		if (graphic->circle_discretization != *circle_discretization
		    && (CMISS_GRAPHIC_CYLINDERS==graphic->graphic_type))
		{
			graphic->circle_discretization = *circle_discretization;
			Cmiss_graphic_circle_discretization_change(graphic, (void *)NULL);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_general_circle_discretization.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}

struct FE_element *Cmiss_graphic_get_seed_element(
	struct Cmiss_graphic *graphic)
{
	struct FE_element *seed_element;

	ENTER(Cmiss_graphic_get_seed_element);
	if (graphic&&((CMISS_GRAPHIC_VOLUMES==graphic->graphic_type)||
		(CMISS_GRAPHIC_STREAMLINES==graphic->graphic_type)))
	{
		seed_element=graphic->seed_element;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_seed_element.  Invalid argument(s)");
		seed_element=(struct FE_element *)NULL;
	}
	LEAVE;

	return (seed_element);
} /* Cmiss_graphic_get_seed_element */

int Cmiss_graphic_set_seed_element(struct Cmiss_graphic *graphic,
	struct FE_element *seed_element)
/*******************************************************************************
LAST MODIFIED : 19 March 1999

DESCRIPTION :
For graphic starting in a particular element.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_graphic_set_seed_element);
	if (graphic&&((CMISS_GRAPHIC_VOLUMES==graphic->graphic_type)||
		(CMISS_GRAPHIC_STREAMLINES==graphic->graphic_type)))
	{
		REACCESS(FE_element)(&graphic->seed_element,seed_element);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_seed_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_set_seed_element */

int Cmiss_graphic_get_seed_xi(struct Cmiss_graphic *graphic,
	Triple seed_xi)
{
	int return_code;

	ENTER(Cmiss_graphic_get_seed_xi);
	if (graphic&&seed_xi&&(
		(CMISS_GRAPHIC_ELEMENT_POINTS==graphic->graphic_type)||
		(CMISS_GRAPHIC_STREAMLINES==graphic->graphic_type)))
	{
		seed_xi[0]=graphic->seed_xi[0];
		seed_xi[1]=graphic->seed_xi[1];
		seed_xi[2]=graphic->seed_xi[2];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_seed_xi.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_get_seed_xi */

int Cmiss_graphic_set_seed_xi(struct Cmiss_graphic *graphic,
	Triple seed_xi)
{
	int return_code;

	ENTER(Cmiss_graphic_set_seed_xi);
	if (graphic&&seed_xi&&(
		(CMISS_GRAPHIC_ELEMENT_POINTS==graphic->graphic_type)||
		(CMISS_GRAPHIC_STREAMLINES==graphic->graphic_type)))
	{
		graphic->seed_xi[0]=seed_xi[0];
		graphic->seed_xi[1]=seed_xi[1];
		graphic->seed_xi[2]=seed_xi[2];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_set_seed_xi.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_set_seed_xi */

int Cmiss_graphic_get_line_width(struct Cmiss_graphic *graphic)
{
	int line_width;

	ENTER(Cmiss_graphic_get_line_width);
	if (graphic)
	{
		line_width=graphic->line_width;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_line_width.  Invalid argument(s)");
		line_width = 0;
	}
	LEAVE;

	return (line_width);
} /* Cmiss_graphic_get_line_width */

int Cmiss_graphic_set_line_width(struct Cmiss_graphic *graphic, int line_width)
{
	int return_code;

	ENTER(Cmiss_graphic_set_line_width);
	if (graphic)
	{
	  return_code = 1;
	  graphic->line_width = line_width;
	}
	else
	{
	  display_message(ERROR_MESSAGE,
	    "Cmiss_graphic_set_line_width.  Invalid argument(s)");
	  return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_set_line_width */

int Cmiss_graphic_get_data_spectrum_parameters_streamlines(
	struct Cmiss_graphic *graphic,
	enum Streamline_data_type *streamline_data_type,
	struct Computed_field **data_field,struct Spectrum **spectrum)
{
	int return_code;

	ENTER(Cmiss_graphic_get_data_spectrum_parameters_streamlines);
	if (graphic&&streamline_data_type&&data_field&&spectrum&&
		(CMISS_GRAPHIC_STREAMLINES==graphic->graphic_type))
	{
		*streamline_data_type=graphic->streamline_data_type;
		*data_field=graphic->data_field;
		*spectrum=graphic->spectrum;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_data_spectrum_parameters_streamlines.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_get_data_spectrum_parameters_streamlines */

struct Computed_field *Cmiss_graphic_get_texture_coordinate_field(
	struct Cmiss_graphic *graphic)
{
	struct Computed_field *texture_coordinate_field;

	ENTER(Cmiss_graphic_get_texture_coordinate_field);
	if (graphic)
	{
		texture_coordinate_field=graphic->texture_coordinate_field;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_texture_coordinate_field.  Invalid argument(s)");
		texture_coordinate_field=(struct Computed_field *)NULL;
	}
	LEAVE;

	return (texture_coordinate_field);
} /* Cmiss_graphic_get_texture_coordinate_field */

int Cmiss_graphic_set_texture_coordinate_field(
   struct Cmiss_graphic *graphic, struct Computed_field *texture_coordinate_field)
{
  int return_code;

  ENTER(Cmiss_graphic_set_texture_coordinate_field);
  if (graphic)
  {
    return_code = 1;
    REACCESS(Computed_field)(&graphic->texture_coordinate_field, texture_coordinate_field);
  }
  else
  {
    return_code = 0;
    display_message(ERROR_MESSAGE,
      "Cmiss_graphic_set_texture_coordinate_field.  Invalid argument(s)");
  }
  LEAVE;

	return (return_code);
} /* Cmiss_graphic_set_texture_coordinate_field */

enum Render_type Cmiss_graphic_get_render_type(
	struct Cmiss_graphic *graphic)
{
	enum Render_type render_type;

	ENTER(Cmiss_graphic_get_render_type);
	if (graphic)
	{
		render_type=graphic->render_type;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_render_type.  Invalid argument(s)");
		render_type = RENDER_TYPE_SHADED;
	}
	LEAVE;

	return (render_type);
} /* Cmiss_graphic_get_render_type */

int Cmiss_graphic_get_exterior(struct Cmiss_graphic *graphic)
{
	int return_code;

	ENTER(Cmiss_graphic_get_exterior);
	if (graphic&&(
		Cmiss_graphic_type_uses_dimension(graphic->graphic_type,1)||
		Cmiss_graphic_type_uses_dimension(graphic->graphic_type,2)))
	{
		return_code=graphic->exterior;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_exterior.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_get_exterior */

int Cmiss_graphic_get_face(struct Cmiss_graphic *graphic,int *face)
{
	int return_code;

	ENTER(Cmiss_graphic_get_face);
	if (graphic&&face&&(
		Cmiss_graphic_type_uses_dimension(graphic->graphic_type,1)||
		Cmiss_graphic_type_uses_dimension(graphic->graphic_type,2)))
	{
		return_code=(0 <= graphic->face);
		*face=graphic->face;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_get_face.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_get_face */

int Cmiss_graphic_time_change(
	struct Cmiss_graphic *graphic,void *dummy_void)
{
	int return_code;
	
	ENTER(Cmiss_graphic_time_change);
	USE_PARAMETER(dummy_void);
	if (graphic)
	{
		return_code = 1;
		if (graphic->glyph && (1 < GT_object_get_number_of_times(graphic->glyph)))
		{
			GT_object_changed(graphic->glyph);
		}
		if (graphic->time_dependent)
		{
			Cmiss_graphic_clear_graphics(graphic);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_time_change.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_time_change */

int Cmiss_graphic_update_time_behaviour(
	struct Cmiss_graphic *graphic, void *update_time_behaviour_void)
{
	int return_code, time_dependent;
	struct Cmiss_graphic_update_time_behaviour_data *data;
	
	ENTER(Cmiss_graphic_update_time_behaviour);
	if (graphic && (data = 
		(struct Cmiss_graphic_update_time_behaviour_data *)
		update_time_behaviour_void))
	{
		return_code = 1;
		time_dependent = 0;
		if (graphic->glyph && (1 < GT_object_get_number_of_times(graphic->glyph)))
		{
			time_dependent = 1;
		}
		if (graphic->coordinate_field)
		{
			if (Computed_field_has_multiple_times(graphic->coordinate_field))
			{
				time_dependent = 1;
			}
		}
		else
		{
			if (data->default_coordinate_depends_on_time)
			{
				time_dependent = 1;
			}
		}
		if (graphic->texture_coordinate_field && Computed_field_has_multiple_times(
			graphic->texture_coordinate_field))
		{
			time_dependent = 1;
		}
		if (graphic->radius_scalar_field && Computed_field_has_multiple_times(
			graphic->radius_scalar_field))
		{
			time_dependent = 1;
		}
		if (graphic->iso_scalar_field && Computed_field_has_multiple_times(
			graphic->iso_scalar_field))
		{
			time_dependent = 1;
		}
		if (graphic->orientation_scale_field && 
			Computed_field_has_multiple_times(graphic->orientation_scale_field))
		{
			time_dependent = 1;
		}
		if (graphic->variable_scale_field && 
			Computed_field_has_multiple_times(graphic->variable_scale_field))
		{
			time_dependent = 1;
		}
		if (graphic->label_field && 
			Computed_field_has_multiple_times(graphic->label_field))
		{
			time_dependent = 1;
		}
		if (graphic->label_density_field &&
			Computed_field_has_multiple_times(graphic->label_density_field))
		{
			time_dependent = 1;
		}
		if (graphic->visibility_field && 
			Computed_field_has_multiple_times(graphic->visibility_field))
		{
			time_dependent = 1;
		}
		if (graphic->variable_scale_field && 
			Computed_field_has_multiple_times(graphic->variable_scale_field))
		{
			time_dependent = 1;
		}
		if (graphic->displacement_map_field && 
			Computed_field_has_multiple_times(graphic->displacement_map_field))
		{
			time_dependent = 1;
		}
		if (graphic->stream_vector_field && 
			Computed_field_has_multiple_times(graphic->stream_vector_field))
		{
			time_dependent = 1;
		}
		if (graphic->data_field && 
			Computed_field_has_multiple_times(graphic->data_field))
		{
			time_dependent = 1;
		}
		/* Or any field that is pointed to has multiple times...... */

		graphic->time_dependent = time_dependent;
		if (time_dependent)
		{
			data->time_dependent = time_dependent;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_update_time_behaviour.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_update_time_behaviour */

int Cmiss_graphic_Spectrum_change(
	struct Cmiss_graphic *graphic, void *spectrum_change_data_void)
/*******************************************************************************
LAST MODIFIED : 12 March 2002

DESCRIPTION :
If <graphic> has a graphics object that plots a scalar using a spectrum in the
<changed_spectrum_list>, the graphics object display list is marked as being
not current. If the graphic are visible, the changed flag is set.
Second argument is a struct Cmiss_graphic_Spectrum_change_data.
==============================================================================*/
{
	int return_code;
	struct Cmiss_graphic_Spectrum_change_data *spectrum_change_data;
	struct Spectrum *colour_lookup;

	ENTER(Cmiss_graphic_Spectrum_change);
	if (graphic && (spectrum_change_data =
		(struct Cmiss_graphic_Spectrum_change_data *)
		spectrum_change_data_void))
	{
		if (graphic->spectrum && IS_OBJECT_IN_LIST(Spectrum)(graphic->spectrum,
			spectrum_change_data->changed_spectrum_list))
		{
			if (graphic->graphics_object)
			{
				GT_object_Spectrum_change(graphic->graphics_object,
					(struct LIST(Spectrum) *)NULL);
			}
			/* need a way to tell either graphic is used in any scene or not */
			spectrum_change_data->changed = 1;
		}
		/* The material gets it's own notification of the change, 
			it should propagate that to the Cmiss_graphic */
		if (graphic->material && (colour_lookup = 
				Graphical_material_get_colour_lookup_spectrum(graphic->material))
			&& IS_OBJECT_IN_LIST(Spectrum)(colour_lookup,
				spectrum_change_data->changed_spectrum_list))
		{
			if (graphic->graphics_object)
			{
				GT_object_Graphical_material_change(graphic->graphics_object,
					(struct LIST(Graphical_material) *)NULL);
			}
			/* need a way to tell either graphic is used in any scene or not */
			spectrum_change_data->changed = 1;
		}
		
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_Spectrum_change.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_Spectrum_change */

int Cmiss_graphic_set_customised_graphics_object(
	struct Cmiss_graphic *graphic, struct GT_object *graphics_object)
{
	int return_code = 0;

	ENTER(Cmiss_graphic_set_customised_graphics_object);
	if (graphic)
	{
		switch (graphic->graphic_type)
		{
			case CMISS_GRAPHIC_STATIC:
			{
				if (graphic->customised_graphics_object != graphics_object)
				{
					REACCESS(GT_object)(&(graphic->customised_graphics_object), graphics_object);
					return_code = 1;
				}
			} break;
			default :
			{
				display_message(INFORMATION_MESSAGE,
					"Cmiss_graphic_customised_graphics_object.  Cannot set customised"
					"graphics_object for this cmiss graphic type");
			}break;
		}
	}
	else
	{
		display_message(INFORMATION_MESSAGE,
			"Cmiss_graphic_customised_graphics_object.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
}

int Cmiss_graphic_enable_overlay(struct Cmiss_graphic *graphic, int flag)
{
	int return_code = 0;

	ENTER(Cmiss_graphic_enable_overlay);
	if (graphic)
	{
		graphic->overlay_flag = flag;
		return_code = 1;
	}
	else
	{
		display_message(INFORMATION_MESSAGE,
			"Cmiss_graphic_enable_overlay.  Invalid argument(s)");
	}
	LEAVE;

	return return_code;
}

int Cmiss_graphic_is_overlay(struct Cmiss_graphic *graphic)
{
	int return_code = 0;

	ENTER(Cmiss_graphic_is_overlay);
	if (graphic)
	{
		return_code = graphic->overlay_flag;
	}
	else
	{
		display_message(INFORMATION_MESSAGE,
			"Cmiss_graphic_is_overlay.  Invalid argument(s)");
	}
	LEAVE;

	return return_code;
}

int Cmiss_graphic_set_overlay_order(struct Cmiss_graphic *graphic, int order)
{
	int return_code = 0;

	ENTER(Cmiss_graphic_set_overlay_order);
	if (graphic)
	{
		graphic->overlay_order = order;
		return_code = 1;
	}
	else
	{
		display_message(INFORMATION_MESSAGE,
			"Cmiss_graphic_set_overlay.  Invalid argument(s)");
	}
	LEAVE;

	return return_code;
}

int Cmiss_graphic_get_overlay_order(struct Cmiss_graphic *graphic)
{
	int return_code = 0;

	ENTER(Cmiss_graphic_get_overlay_order);
	if (graphic)
	{
		return_code = graphic->overlay_order;
	}
	else
	{
		display_message(INFORMATION_MESSAGE,
			"Cmiss_graphic_get_overlay_order.  Invalid argument(s)");
	}
	LEAVE;

	return return_code;
}

int Cmiss_graphic_detach_fields(struct Cmiss_graphic *graphic, void *dummy_void)
{
	int return_code = 1;
	USE_PARAMETER(dummy_void);

	if (graphic)
	{
		if (graphic->coordinate_field)
		{
			DEACCESS(Computed_field)(&(graphic->coordinate_field));
		}
		if (graphic->texture_coordinate_field)
		{
			DEACCESS(Computed_field)(&(graphic->texture_coordinate_field));
		}
		if (graphic->radius_scalar_field)
		{
			DEACCESS(Computed_field)(&(graphic->radius_scalar_field));
		}
		if (graphic->iso_scalar_field)
		{
			DEACCESS(Computed_field)(&(graphic->iso_scalar_field));
		}
		if (graphic->orientation_scale_field)
		{
			DEACCESS(Computed_field)(&(graphic->orientation_scale_field));
		}
		if (graphic->variable_scale_field)
		{
			DEACCESS(Computed_field)(&(graphic->variable_scale_field));
		}
		if (graphic->label_field)
		{
			DEACCESS(Computed_field)(&(graphic->label_field));
		}
		if (graphic->label_density_field)
		{
			DEACCESS(Computed_field)(&(graphic->label_density_field));
		}
		if (graphic->visibility_field)
		{
			DEACCESS(Computed_field)(&(graphic->visibility_field));
		}
		if (graphic->displacement_map_field)
		{
			DEACCESS(Computed_field)(&(graphic->displacement_map_field));
		}
		if (graphic->xi_point_density_field)
		{
			DEACCESS(Computed_field)(&(graphic->xi_point_density_field));
		}
		if (graphic->native_discretization_field)
		{
			DEACCESS(FE_field)(&(graphic->native_discretization_field));
		}
		if (graphic->stream_vector_field)
		{
			DEACCESS(Computed_field)(&(graphic->stream_vector_field));
		}
		if (graphic->data_field)
		{
			DEACCESS(Computed_field)(&(graphic->data_field));
		}
		if (graphic->seed_node_coordinate_field)
		{
			DEACCESS(Computed_field)(&(graphic->seed_node_coordinate_field));
		}
	}
	else
	{
		display_message(INFORMATION_MESSAGE,
			"Cmiss_graphic_detach_fields.  Invalid argument(s)");
		return_code = 0;
	}

	return return_code;
}

int Cmiss_graphic_selected_element_points_change(
	struct Cmiss_graphic *graphic,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 28 February 2000

DESCRIPTION :
Tells <graphic> that if the graphics resulting from it depend on the currently
selected element points, then they should be updated.
Must call Cmiss_graphic_to_graphics_object afterwards to complete.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_graphic_selected_element_points_change);
	USE_PARAMETER(dummy_void);
	if (graphic)
	{
		return_code=1;
		if (graphic->graphics_object&&
			(CMISS_GRAPHIC_ELEMENT_POINTS==graphic->graphic_type))
		{
			switch (graphic->select_mode)
			{
				case GRAPHICS_SELECT_ON:
				{
					/* for efficiency, just request update of selected graphics */
					Cmiss_graphic_clear_graphics(graphic);
				} break;
				case GRAPHICS_NO_SELECT:
				{
					/* nothing to do as no names put out with graphic */
				} break;
				case GRAPHICS_DRAW_SELECTED:
				case GRAPHICS_DRAW_UNSELECTED:
				{
					/* need to rebuild graphics_object from scratch */
					Cmiss_graphic_clear_graphics(graphic);
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"Cmiss_graphic_selected_element_points_change.  "
						"Unknown select_mode");
				} break;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_graphic_selected_element_points_change.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_graphic_selected_element_points_change */

struct Cmiss_rendition *Cmiss_graphic_get_rendition_private(struct Cmiss_graphic *graphic)
{
	if (graphic)
		return graphic->rendition;
	return NULL;
}

int Cmiss_graphic_set_rendition_private(struct Cmiss_graphic *graphic,
	struct Cmiss_rendition *rendition)
{
	if (graphic && ((NULL == rendition) || (NULL == graphic->rendition)))
	{
		graphic->rendition = rendition;
		return 1;
	}
	else
	{
		display_message(INFORMATION_MESSAGE,
			"Cmiss_graphic_set_rendition_private.  Invalid argument(s)");
	}
	return 0;
}
