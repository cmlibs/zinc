/*******************************************************************************
FILE : element_group_settings.c

LAST MODIFIED : 30 April 2003

DESCRIPTION :
GT_element_settings structure and routines for describing and manipulating the
appearance of graphical finite element groups.
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
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "general/debug.h"
#include "general/enumerator_private.h"
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
#include "finite_element/finite_element_to_streamlines.h"
#include "graphics/auxiliary_graphics_types.h"
#include "graphics/element_group_settings.h"
#include "graphics/font.h"
#include "graphics/graphics_object.h"
#include "graphics/scene.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Global variables
----------------
*/
/*
Module types
------------
*/

struct GT_element_settings
/*******************************************************************************
LAST MODIFIED : 14 March 2003

DESCRIPTION :
Stores one group of settings for a single line/surface/etc. part of the
finite element group rendition.
==============================================================================*/
{
	/* position identifier for ordering settings in list */
	int position;

	/* name for identifying settings */
	char *name;

	/* geometry settings */
	/* for all graphic types */
	enum GT_element_settings_type settings_type;
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
	double *iso_values, decimation_threshold;
	/* for node_points, data_points and element_points only */
	struct GT_object *glyph;
	enum Glyph_scaling_mode glyph_scaling_mode;
	Triple glyph_centre, glyph_scale_factors, glyph_size;
	struct Computed_field *orientation_scale_field;
	struct Computed_field *variable_scale_field;
	struct Computed_field *label_field;
	/* for element_points and iso_surfaces */
	enum Use_element_type use_element_type;
	/* for element_points only */
	enum Xi_discretization_mode xi_discretization_mode;
	struct Computed_field *xi_point_density_field;
	struct FE_field *native_discretization_field;
	struct Element_discretization discretization;
	/* for volumes only */
	struct VT_volume_texture *volume_texture;
	/* SAB Added for text access only */
	struct Computed_field *displacement_map_field;
	int displacement_map_xi_direction;
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
	struct GT_object *graphics_object;
	/* flag indicating the graphics_object needs rebuilding */
	int graphics_changed;
	/* flag indicating that selected graphics have changed */
	int selected_graphics_changed;
	/* flag indicating that this settings needs to be regenerated when time
		changes */
	int time_dependent;

	/* for accessing objects */
	int access_count;
}; /* struct GT_element_settings */

FULL_DECLARE_INDEXED_LIST_TYPE(GT_element_settings);

/*
Module functions
----------------
*/
DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(GT_element_settings,position,int, \
	compare_int)

static int GT_element_settings_uses_FE_element(
	struct GT_element_settings *settings,struct FE_element *element,
	struct FE_region *fe_region)
/*******************************************************************************
LAST MODIFIED : 14 March 2003

DESCRIPTION :
GT_element_settings list conditional function returning 1 if the element
would contribute any graphics generated from the GT_element_settings. The
<fe_region> is provided to allow any checks against parent elements to
ensure the relevant parent elements are also in the group.
==============================================================================*/
{
	enum CM_element_type cm_element_type;
	int dimension,return_code;

	ENTER(GT_element_settings_uses_FE_element);
	if (settings && element && fe_region)
	{
		dimension=GT_element_settings_get_dimension(settings);
		if ((GT_ELEMENT_SETTINGS_ELEMENT_POINTS==settings->settings_type)||
			(GT_ELEMENT_SETTINGS_ISO_SURFACES==settings->settings_type))
		{
			cm_element_type=Use_element_type_CM_element_type(
				settings->use_element_type);
		}
		else if (GT_ELEMENT_SETTINGS_STREAMLINES==settings->settings_type)
		{
			cm_element_type=CM_ELEMENT;
		}
		else
		{
			cm_element_type=CM_ELEMENT_TYPE_INVALID;
		}
		return_code = FE_region_FE_element_meets_topological_criteria(fe_region,
			element, dimension, cm_element_type, settings->exterior, settings->face);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_uses_FE_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_uses_FE_element */

struct GT_element_settings_select_graphics_data
{
	struct FE_region *fe_region;
	struct GT_element_settings *settings;
};

static int Element_point_ranges_select_in_graphics_object(
	struct Element_point_ranges *element_point_ranges,void *select_data_void)
/*******************************************************************************
LAST MODIFIED : 14 March 2003

DESCRIPTION :
If <settings> is of type GT_ELEMENT_SETTINGS_ELEMENT_POINTS and has a graphics
object, determines from the identifier of <element_point_ranges>, whether it
applies to the settings, and if so selects any ranges in it in the
graphics_object.
==============================================================================*/
{
	int i,return_code,top_level_number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	struct CM_element_information cm;
	struct Element_point_ranges_identifier element_point_ranges_identifier,
		temp_element_point_ranges_identifier;
	struct GT_element_settings_select_graphics_data *select_data;
	struct FE_element *element,*top_level_element;
	struct FE_region *fe_region;
	struct GT_element_settings *settings;
	struct Multi_range *ranges;

	ENTER(Element_point_ranges_select_in_graphics_object);
	if (element_point_ranges&&(select_data=
		(struct GT_element_settings_select_graphics_data *)select_data_void)&&
		(fe_region=select_data->fe_region)&&
		(settings=select_data->settings)&&
		(GT_ELEMENT_SETTINGS_ELEMENT_POINTS==settings->settings_type)&&
		settings->graphics_object)
	{
		return_code=1;
		if (Element_point_ranges_get_identifier(element_point_ranges,
			&element_point_ranges_identifier) &&
			(element = element_point_ranges_identifier.element) &&
			get_FE_element_identifier(element, &cm))
		{
			/* do not select if graphics already selected for element */
			if (!GT_object_is_graphic_selected(settings->graphics_object,
				CM_element_information_to_graphics_name(&cm), &ranges))
			{
				/* special handling for cell_corners which are always calculated on
					 top_level_elements */
				if (((CM_ELEMENT == cm.type) &&
					(XI_DISCRETIZATION_CELL_CORNERS==settings->xi_discretization_mode)) ||
					((XI_DISCRETIZATION_CELL_CORNERS!=settings->xi_discretization_mode) &&
						GT_element_settings_uses_FE_element(settings, element, fe_region)))
				{
					top_level_element=(struct FE_element *)NULL;
					top_level_number_in_xi[0]=settings->discretization.number_in_xi1;
					top_level_number_in_xi[1]=settings->discretization.number_in_xi2;
					top_level_number_in_xi[2]=settings->discretization.number_in_xi3;
					if (FE_region_get_FE_element_discretization(fe_region, element,
						settings->face,settings->native_discretization_field,
						top_level_number_in_xi,&top_level_element,
						temp_element_point_ranges_identifier.number_in_xi))
					{
						temp_element_point_ranges_identifier.element=element;
						temp_element_point_ranges_identifier.top_level_element=
							top_level_element;
						temp_element_point_ranges_identifier.xi_discretization_mode=
							settings->xi_discretization_mode;
						/*???RC temporary, hopefully */
						for (i=0;i<3;i++)
						{
							temp_element_point_ranges_identifier.exact_xi[i]=
								settings->seed_xi[i];
						}
						if (0==compare_Element_point_ranges_identifier(
							&element_point_ranges_identifier,
							&temp_element_point_ranges_identifier))
						{
							if (ranges=CREATE(Multi_range)())
							{
								if (!(Multi_range_copy(ranges,Element_point_ranges_get_ranges(
									element_point_ranges))&&
									GT_object_select_graphic(settings->graphics_object,
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

static int FE_element_select_graphics(struct FE_element *element,
	void *select_data_void)
/*******************************************************************************
LAST MODIFIED : 14 March 2003

DESCRIPTION :
If the <settings> uses <element>, select it in its graphics object.
==============================================================================*/
{
	int return_code;
	struct CM_element_information cm;
	struct GT_element_settings_select_graphics_data *select_data;

	ENTER(FE_element_select_graphics);
	if (element&&(select_data=
		(struct GT_element_settings_select_graphics_data *)select_data_void)&&
		select_data->fe_region&&select_data->settings&&
		select_data->settings->graphics_object)
	{
		if (GT_element_settings_uses_FE_element(select_data->settings,element,
			select_data->fe_region))
		{
			return_code =
				get_FE_element_identifier(element, &cm) &&
				GT_object_select_graphic(select_data->settings->graphics_object,
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
			"FE_element_select_graphics.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_element_select_graphics */

static int FE_element_select_graphics_element_points(struct FE_element *element,
	void *select_data_void)
/*******************************************************************************
LAST MODIFIED : 14 March 2003

DESCRIPTION :
Special version of FE_element_select_graphics for element_points, which
are calculated on the top_level_element in Xi_discretization_mode CELL_CORNERS.
==============================================================================*/
{
	int return_code;
	struct CM_element_information cm;
	struct FE_region *fe_region;
	struct GT_element_settings *settings;
	struct GT_element_settings_select_graphics_data *select_data;

	ENTER(FE_element_select_graphics_element_points);
	if (element&&(select_data=
		(struct GT_element_settings_select_graphics_data *)select_data_void)&&
		(fe_region=select_data->fe_region)&&
		(settings=select_data->settings)&&
		(GT_ELEMENT_SETTINGS_ELEMENT_POINTS==settings->settings_type)&&
		settings->graphics_object)
	{
		/* special handling for cell_corners which are always calculated on
			 top_level_elements */
		get_FE_element_identifier(element, &cm);
		if (((CM_ELEMENT == cm.type) &&
			(XI_DISCRETIZATION_CELL_CORNERS==settings->xi_discretization_mode)) ||
			((XI_DISCRETIZATION_CELL_CORNERS!=settings->xi_discretization_mode) &&
				GT_element_settings_uses_FE_element(settings,element,fe_region)))
		{
			return_code=
				GT_object_select_graphic(select_data->settings->graphics_object,
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

/*
Global functions
----------------
*/

PROTOTYPE_ENUMERATOR_STRING_FUNCTION(GT_element_settings_type)
{
	char *enumerator_string;

	ENTER(ENUMERATOR_STRING(GT_element_settings_type));
	switch (enumerator_value)
	{
		case GT_ELEMENT_SETTINGS_NODE_POINTS:
		{
			enumerator_string = "node_points";
		} break;
		case GT_ELEMENT_SETTINGS_DATA_POINTS:
		{
			enumerator_string = "data_points";
		} break;
		case GT_ELEMENT_SETTINGS_LINES:
		{
			enumerator_string = "lines";
		} break;
		case GT_ELEMENT_SETTINGS_CYLINDERS:
		{
			enumerator_string = "cylinders";
		} break;
		case GT_ELEMENT_SETTINGS_SURFACES:
		{
			enumerator_string = "surfaces";
		} break;
		case GT_ELEMENT_SETTINGS_ISO_SURFACES:
		{
			enumerator_string = "iso_surfaces";
		} break;
		case GT_ELEMENT_SETTINGS_ELEMENT_POINTS:
		{
			enumerator_string = "element_points";
		} break;
		case GT_ELEMENT_SETTINGS_STREAMLINES:
		{
			enumerator_string = "streamlines";
		} break;
#if ! defined (WX_USER_INTERFACE)
		case GT_ELEMENT_SETTINGS_VOLUMES:
		{
			enumerator_string = "volumes";
		} break;
#endif /* ! defined (WX_USER_INTERFACE) */
		default:
		{
			enumerator_string = (char *)NULL;
		} break;
	}
	LEAVE;

	return (enumerator_string);
} /* ENUMERATOR_STRING(GT_element_settings_type) */

DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS(GT_element_settings_type)

int GT_element_settings_type_uses_dimension(
	enum GT_element_settings_type settings_type, int dimension)
/*******************************************************************************
LAST MODIFIED : 20 March 2001

DESCRIPTION :
Returns true if the particular <settings_type> can deal with nodes/elements of
the given <dimension>. Note a <dimension> of -1 is taken to mean any dimension.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_settings_type_uses_dimension);
	switch (settings_type)
	{
		case GT_ELEMENT_SETTINGS_NODE_POINTS:
		case GT_ELEMENT_SETTINGS_DATA_POINTS:
		{
			return_code = ((-1 == dimension) || (0 == dimension));
		} break;
		case GT_ELEMENT_SETTINGS_LINES:
		case GT_ELEMENT_SETTINGS_CYLINDERS:
		{
			return_code = ((-1 == dimension) || (1 == dimension));
		} break;
		case GT_ELEMENT_SETTINGS_SURFACES:
		{
			return_code = ((-1 == dimension) || (2 == dimension));
		} break;
		case GT_ELEMENT_SETTINGS_VOLUMES:
		{
			return_code = ((-1 == dimension) || (3 == dimension));
		} break;
		case GT_ELEMENT_SETTINGS_STREAMLINES:
		{
			return_code = ((-1 == dimension) || (2 == dimension) || 
				(3 == dimension));
		} break;
		case GT_ELEMENT_SETTINGS_ELEMENT_POINTS:
		case GT_ELEMENT_SETTINGS_ISO_SURFACES:
		{
			return_code = ((-1 == dimension) ||
				(1 == dimension) || (2 == dimension) || (3 == dimension));
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"GT_element_settings_type_uses_dimension.  Unknown settings type");
			return_code = 0;
		} break;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_type_uses_dimension */

int GT_element_settings_type_uses_dimension_conditional(
	enum GT_element_settings_type settings_type, void *dimension_address_void)
/*******************************************************************************
LAST MODIFIED : 20 March 2001

DESCRIPTION :
Calls GT_element_settings_type_uses_dimension for the <settings_type> and
integer dimension pointed to by <dimension_address_void>.
==============================================================================*/
{
	int *dimension_address, return_code;

	ENTER(GT_element_settings_type_uses_dimension_conditional);
	if (dimension_address = (int *)dimension_address_void)
	{
		return_code = GT_element_settings_type_uses_dimension(settings_type,
			*dimension_address);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_type_uses_dimension_conditional.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_type_uses_dimension_conditional */

PROTOTYPE_ENUMERATOR_STRING_FUNCTION(Glyph_scaling_mode)
{
	char *enumerator_string;

	ENTER(ENUMERATOR_STRING(Glyph_scaling_mode));
	switch (enumerator_value)
	{
		case GLYPH_SCALING_CONSTANT:
		{
			enumerator_string = "constant";
		} break;
		case GLYPH_SCALING_SCALAR:
		{
			enumerator_string = "scalar";
		} break;
		case GLYPH_SCALING_VECTOR:
		{
			enumerator_string = "vector";
		} break;
		case GLYPH_SCALING_AXES:
		{
			enumerator_string = "axes";
		} break;
		case GLYPH_SCALING_GENERAL:
		{
			enumerator_string = "general";
		} break;
		default:
		{
			enumerator_string = (char *)NULL;
		} break;
	}
	LEAVE;

	return (enumerator_string);
} /* ENUMERATOR_STRING(Glyph_scaling_mode) */

DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS(Glyph_scaling_mode)

DECLARE_OBJECT_FUNCTIONS(GT_element_settings)
DECLARE_INDEXED_LIST_FUNCTIONS(GT_element_settings)
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(GT_element_settings, \
	position,int,compare_int)

struct GT_element_settings *CREATE(GT_element_settings)(
	enum GT_element_settings_type settings_type)
/*******************************************************************************
LAST MODIFIED : 14 March 2003

DESCRIPTION :
Allocates memory and assigns fields for a struct GT_element_settings.
==============================================================================*/
{
	struct GT_element_settings *settings;

	ENTER(CREATE(GT_element_settings));
	if ((GT_ELEMENT_SETTINGS_NODE_POINTS==settings_type)||
		(GT_ELEMENT_SETTINGS_DATA_POINTS==settings_type)||
		(GT_ELEMENT_SETTINGS_LINES==settings_type)||
		(GT_ELEMENT_SETTINGS_CYLINDERS==settings_type)||
		(GT_ELEMENT_SETTINGS_SURFACES==settings_type)||
		(GT_ELEMENT_SETTINGS_ISO_SURFACES==settings_type)||
		(GT_ELEMENT_SETTINGS_ELEMENT_POINTS==settings_type)||
		(GT_ELEMENT_SETTINGS_VOLUMES==settings_type)||
		(GT_ELEMENT_SETTINGS_STREAMLINES==settings_type))
	{
		if (ALLOCATE(settings,struct GT_element_settings,1))
		{
			settings->position=0;
			settings->name = (char *)NULL;

			/* geometry settings defaults */
			/* for all graphic types */
			settings->settings_type=settings_type;
			settings->coordinate_field=(struct Computed_field *)NULL;
			/* For surfaces only at the moment */
			settings->texture_coordinate_field=(struct Computed_field *)NULL;
			/* for 1-D and 2-D elements only */
			settings->exterior=0;
			settings->face=-1; /* any face */
			/* for cylinders only */
			settings->constant_radius=0.0;
			settings->radius_scale_factor=1.0;
			settings->radius_scalar_field=(struct Computed_field *)NULL;
			/* for iso_surfaces only */
			settings->iso_scalar_field=(struct Computed_field *)NULL;
			settings->number_of_iso_values=0;
			settings->iso_values=(double *)NULL;
			settings->decimation_threshold = 0.0;
			/* for node_points, data_points and element_points only */
			settings->glyph=(struct GT_object *)NULL;
			settings->glyph_scaling_mode = GLYPH_SCALING_GENERAL;
			settings->glyph_centre[0]=0.0;
			settings->glyph_centre[1]=0.0;
			settings->glyph_centre[2]=0.0;
			settings->glyph_scale_factors[0]=1.0;
			settings->glyph_scale_factors[1]=1.0;
			settings->glyph_scale_factors[2]=1.0;
			settings->glyph_size[0]=1.0;
			settings->glyph_size[1]=1.0;
			settings->glyph_size[2]=1.0;
			settings->orientation_scale_field=(struct Computed_field *)NULL;
			settings->variable_scale_field=(struct Computed_field *)NULL;
			settings->label_field=(struct Computed_field *)NULL;
			settings->select_mode=GRAPHICS_SELECT_ON;
			/* for element_points and iso_surfaces */
			settings->use_element_type=USE_ELEMENTS;
			/* for element_points only */
			settings->xi_discretization_mode=XI_DISCRETIZATION_CELL_CENTRES;
			settings->xi_point_density_field = (struct Computed_field *)NULL;
			settings->native_discretization_field=(struct FE_field *)NULL;
			/* default to 1*1*1 discretization for fastest possible display.
				 Important since model may have a *lot* of elements */
			settings->discretization.number_in_xi1=1;
			settings->discretization.number_in_xi2=1;
			settings->discretization.number_in_xi3=1;
			/* for volumes only */
			settings->volume_texture=(struct VT_volume_texture *)NULL;
			settings->displacement_map_field=(struct Computed_field *)NULL;
			settings->displacement_map_xi_direction = 12;
			/* for settings starting in a particular element */
			settings->seed_element=(struct FE_element *)NULL;
			/* for settings requiring an exact xi location */
			settings->seed_xi[0]=0.5;
			settings->seed_xi[1]=0.5;
			settings->seed_xi[2]=0.5;
			/* for streamlines only */
			settings->streamline_type=STREAM_LINE;
			settings->stream_vector_field=(struct Computed_field *)NULL;
			settings->reverse_track=0;
			settings->streamline_length=1.0;
			settings->streamline_width=1.0;
			settings->seed_node_region = (struct FE_region *)NULL;
			settings->seed_node_region_path = (char *)NULL;
			settings->seed_node_coordinate_field = (struct Computed_field *)NULL;

			/* appearance settings defaults */
			/* for all graphic types */
			settings->visibility=1;
			settings->material=(struct Graphical_material *)NULL;
			settings->secondary_material=(struct Graphical_material *)NULL;
			settings->selected_material=(struct Graphical_material *)NULL;
			settings->data_field=(struct Computed_field *)NULL;
			settings->spectrum=(struct Spectrum *)NULL;
			settings->autorange_spectrum_flag = 0;
			/* for glyphsets */
			settings->font = (struct Graphics_font *)NULL;
			/* for surfaces and volumes */
			settings->render_type = RENDER_TYPE_SHADED;
			/* for streamlines only */
			settings->streamline_data_type=STREAM_NO_DATA;
			/* for lines */
			settings->line_width = 0;

			/* rendering information defaults */
			settings->graphics_object = (struct GT_object *)NULL;
			settings->graphics_changed = 1;
			settings->selected_graphics_changed = 0;
			settings->time_dependent = 0;

			settings->access_count=0;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(GT_element_settings).  Insufficient memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(GT_element_settings).  Invalid graphic type");
		settings=(struct GT_element_settings *)NULL;
	}
	LEAVE;

	return (settings);
} /* CREATE(GT_element_settings) */

int DESTROY(GT_element_settings)(struct GT_element_settings **settings_ptr)
/*******************************************************************************
LAST MODIFIED : 7 June 2001

DESCRIPTION :
Frees the memory for the fields of <**settings_ptr>, frees the memory for
<**settings_ptr> and sets <*settings_ptr> to NULL.
==============================================================================*/
{
	struct GT_element_settings *settings;
	int return_code;

	ENTER(DESTROY(GT_element_settings));
	if (settings_ptr&&(settings= *settings_ptr))
	{
		/*???RC temp check access_count is zero! */
		if (0==settings->access_count)
		{
			if (settings->name)
			{
				DEALLOCATE(settings->name);
			}
			if (settings->graphics_object)
			{
				DEACCESS(GT_object)(&(settings->graphics_object));
			}
			if (settings->coordinate_field)
			{
				DEACCESS(Computed_field)(&(settings->coordinate_field));
			}
			if (settings->texture_coordinate_field)
			{
				DEACCESS(Computed_field)(&(settings->texture_coordinate_field));
			}
			if (settings->radius_scalar_field)
			{
				DEACCESS(Computed_field)(&(settings->radius_scalar_field));
			}
			if (settings->iso_scalar_field)
			{
				DEACCESS(Computed_field)(&(settings->iso_scalar_field));
			}
			if (settings->iso_values)
			{
				DEALLOCATE(settings->iso_values);
			}
			if (settings->glyph)
			{
				GT_object_remove_callback(settings->glyph, GT_element_settings_glyph_change,
					(void *)settings);
				DEACCESS(GT_object)(&(settings->glyph));
			}
			if (settings->orientation_scale_field)
			{
				DEACCESS(Computed_field)(&(settings->orientation_scale_field));
			}
			if (settings->variable_scale_field)
			{
				DEACCESS(Computed_field)(&(settings->variable_scale_field));
			}
			if (settings->label_field)
			{
				DEACCESS(Computed_field)(&(settings->label_field));
			}
			if (settings->volume_texture)
			{
				DEACCESS(VT_volume_texture)(&(settings->volume_texture));
			}
			if (settings->displacement_map_field)
			{
				DEACCESS(Computed_field)(&(settings->displacement_map_field));
			}
			if (settings->xi_point_density_field)
			{
				DEACCESS(Computed_field)(&(settings->xi_point_density_field));
			}
			if (settings->native_discretization_field)
			{
				DEACCESS(FE_field)(&(settings->native_discretization_field));
			}
			if (settings->stream_vector_field)
			{
				DEACCESS(Computed_field)(&(settings->stream_vector_field));
			}
			/* appearance settings */
			if (settings->material)
			{
				DEACCESS(Graphical_material)(&(settings->material));
			}
			if (settings->secondary_material)
			{
				DEACCESS(Graphical_material)(&(settings->secondary_material));
			}
			if (settings->selected_material)
			{
				DEACCESS(Graphical_material)(&(settings->selected_material));
			}
			if (settings->data_field)
			{
				DEACCESS(Computed_field)(&(settings->data_field));
			}
			if (settings->spectrum)
			{
				DEACCESS(Spectrum)(&(settings->spectrum));
			}
			if (settings->font)
			{
				DEACCESS(Graphics_font)(&(settings->font));
			}
			if (settings->seed_element)
			{
				DEACCESS(FE_element)(&(settings->seed_element));
			}
			if (settings->seed_node_region)
			{
				DEACCESS(FE_region)(&(settings->seed_node_region));
			}
			if (settings->seed_node_region_path)
			{
				DEALLOCATE(settings->seed_node_region_path);
			}
			if (settings->seed_node_coordinate_field)
			{
				DEACCESS(Computed_field)(&(settings->seed_node_coordinate_field));
			}
			DEALLOCATE(*settings_ptr);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"DESTROY(GT_element_settings).  Non-zero access_count");
			*settings_ptr=(struct GT_element_settings *)NULL;
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(GT_element_settings).  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(GT_element_settings) */

int GT_element_settings_copy_without_graphics_object(
	struct GT_element_settings *destination, struct GT_element_settings *source)
/*******************************************************************************
LAST MODIFIED : 2 April 2003

DESCRIPTION :
Copies the GT_element contents from source to destination.
Notes:
destination->access_count is not changed by COPY.
graphics_object is NOT copied; destination->graphics_object is cleared.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_settings_copy_without_graphics_object);
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
			strcpy(destination->name, source->name);
		}

		/* copy geometry settings */
		/* for all graphic types */
		destination->settings_type=source->settings_type;
		REACCESS(Computed_field)(&(destination->coordinate_field),
			source->coordinate_field);
		destination->select_mode=source->select_mode;
		/* for surfaces only at the moment */
		REACCESS(Computed_field)(&(destination->texture_coordinate_field),
			source->texture_coordinate_field);
		/* for 1-D and 2-D elements only */
		destination->exterior=source->exterior;
		destination->face=source->face;
		/* for cylinders only */
		if (GT_ELEMENT_SETTINGS_CYLINDERS==source->settings_type)
		{
			GT_element_settings_set_radius_parameters(destination,
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
		if (GT_ELEMENT_SETTINGS_ISO_SURFACES==source->settings_type)
		{
			GT_element_settings_set_iso_surface_parameters(destination,
				source->iso_scalar_field,source->number_of_iso_values,
				source->iso_values, source->decimation_threshold);
		}
		else
		{
			if (destination->iso_scalar_field)
			{
				DEACCESS(Computed_field)(&destination->iso_scalar_field);
			}
		}
		/* for node_points, data_points and element_points only */
		if ((GT_ELEMENT_SETTINGS_NODE_POINTS==source->settings_type)||
			(GT_ELEMENT_SETTINGS_DATA_POINTS==source->settings_type)||
			(GT_ELEMENT_SETTINGS_ELEMENT_POINTS==source->settings_type))
		{
			GT_element_settings_set_glyph_parameters(destination,
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
					GT_element_settings_glyph_change, (void *)destination);
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
		REACCESS(Computed_field)(&(destination->label_field),source->label_field);
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
		REACCESS(FE_field)(&(destination->native_discretization_field),
			source->native_discretization_field);
		/* for volumes only */
		REACCESS(VT_volume_texture)(&(destination->volume_texture),
			source->volume_texture);
		REACCESS(Computed_field)(&(destination->displacement_map_field),
			source->displacement_map_field);
		/* for settings starting in a particular element */
		REACCESS(FE_element)(&(destination->seed_element),
			source->seed_element);
		/* for settings requiring an exact xi location */
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

		/* copy appearance settings */
		/* for all graphic types */
		destination->visibility=source->visibility;
		destination->line_width = source->line_width;
		REACCESS(Graphical_material)(&(destination->material),source->material);
		REACCESS(Graphical_material)(&(destination->secondary_material),
			source->secondary_material);
		GT_element_settings_set_render_type(destination,source->render_type);
		if (GT_ELEMENT_SETTINGS_STREAMLINES==source->settings_type)
		{
			GT_element_settings_set_data_spectrum_parameters_streamlines(destination,
				source->streamline_data_type,source->data_field,source->spectrum);
		}
		else
		{
			GT_element_settings_set_data_spectrum_parameters(destination,
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
			display_message(ERROR_MESSAGE,"GT_element_settings_copy_without_graphics_object.  "
				"Error copying settings");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"GT_element_settings_copy_without_graphics_object.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_copy_without_graphics_object */

int GET_NAME(GT_element_settings)(struct GT_element_settings *object,
	char **name_ptr)
/*****************************************************************************
LAST MODIFIED : 5 July 1999

DESCRIPTION :
============================================================================*/
{
	int return_code;

	ENTER(GET_NAME(GT_element_settings));
	if (object&&name_ptr)
	{
		if (object->name)
		{
			if (ALLOCATE(*name_ptr,char,strlen(object->name)+1))
			{
				strcpy(*name_ptr,object->name);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"GET_NAME(GT_element_settings).  Could not allocate space for name");
				return_code=0;
			}
		}
		else
		{
			/* Use the position number */
			if (ALLOCATE(*name_ptr,char,30))
			{
				sprintf(*name_ptr,"%d",object->position);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"GET_NAME(GT_element_settings).  Could not allocate space for position name");
				return_code=0;
			}
			
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GET_NAME(GT_element_settings).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GET_NAME(GT_element_settings) */

int GT_element_settings_set_name(struct GT_element_settings *settings, char *name)
/*******************************************************************************
LAST MODIFIED : 5 July 1999

DESCRIPTION :
Sets the <name> used by <settings>.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_settings_set_name);
	if (settings&&name)
	{
		if (settings->name)
		{
			DEALLOCATE(settings->name);
		}
		if (ALLOCATE(settings->name, char, strlen(name) + 1))
		{
			strcpy(settings->name, name);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_set_name.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_set_name */

struct Computed_field *GT_element_settings_get_coordinate_field(
	struct GT_element_settings *settings)
/*******************************************************************************
LAST MODIFIED : 15 February 1999

DESCRIPTION :
Returns the coordinate field used by <settings>.
==============================================================================*/
{
	struct Computed_field *coordinate_field;

	ENTER(GT_element_settings_get_coordinate_field);
	if (settings)
	{
		coordinate_field=settings->coordinate_field;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_get_coordinate_field.  Invalid argument(s)");
		coordinate_field=(struct Computed_field *)NULL;
	}
	LEAVE;

	return (coordinate_field);
} /* GT_element_settings_get_coordinate_field */

int GT_element_settings_set_coordinate_field(
	struct GT_element_settings *settings,struct Computed_field *coordinate_field)
/*******************************************************************************
LAST MODIFIED : 19 March 1999

DESCRIPTION :
Sets the <coordinate_field> used by <settings>. If the coordinate field is
NULL, the settings uses the default coordinate_field from the graphical element
group.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_settings_set_coordinate_field);
	if (settings&&((!coordinate_field)||
		(3>=Computed_field_get_number_of_components(coordinate_field))))
	{
		REACCESS(Computed_field)(&(settings->coordinate_field),coordinate_field);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_set_coordinate_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_set_coordinate_field */

struct Computed_field *GT_element_settings_get_texture_coordinate_field(
	struct GT_element_settings *settings)
/*******************************************************************************
LAST MODIFIED : 1 June 1999

DESCRIPTION :
Returns the texture coordinate field used by <settings>.
==============================================================================*/
{
	struct Computed_field *texture_coordinate_field;

	ENTER(GT_element_settings_get_texture_coordinate_field);
	if (settings)
	{
		texture_coordinate_field=settings->texture_coordinate_field;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_get_texture_coordinate_field.  Invalid argument(s)");
		texture_coordinate_field=(struct Computed_field *)NULL;
	}
	LEAVE;

	return (texture_coordinate_field);
} /* GT_element_settings_get_texture_coordinate_field */

int GT_element_settings_set_texture_coordinate_field(
	struct GT_element_settings *settings,struct Computed_field *texture_coordinate_field)
/*******************************************************************************
LAST MODIFIED : 1 June 1999

DESCRIPTION :
Sets the <texture_coordinate_field> used by <settings>.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_settings_set_texture_coordinate_field);
	if (settings&&((!texture_coordinate_field)||
		(3>=Computed_field_get_number_of_components(texture_coordinate_field))))
	{
		REACCESS(Computed_field)(&(settings->texture_coordinate_field),texture_coordinate_field);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_set_texture_coordinate_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_set_texture_coordinate_field */

int GT_element_settings_get_autorange_spectrum_flag(
	struct GT_element_settings *settings)
/*******************************************************************************
LAST MODIFIED : 24 January 2002

DESCRIPTION :
Returns the value of the autoranging_spectrum_flag from <settings>.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_settings_get_autorange_spectrum_flag);
	if (settings)
	{
		return_code = settings->autorange_spectrum_flag;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_get_autorange_spectrum_flag.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_get_autorange_spectrum_flag */

int GT_element_settings_set_autorange_spectrum_flag(
	struct GT_element_settings *settings, int flag)
/*******************************************************************************
LAST MODIFIED : 24 January 2002

DESCRIPTION :
Sets the autorange_spectrum_flag used by <settings> to value of <flag>.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_settings_set_autorange_spectrum_flag);
	if (settings)
	{
		settings->autorange_spectrum_flag = flag;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_set_autorange_spectrum_flag.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_set_autorange_spectrum_flag */

int GT_element_settings_get_data_range_for_autoranging_spectrum(
	struct GT_element_settings *settings, void *data_void)
/*******************************************************************************
LAST MODIFIED : 18 January 2002

DESCRIPTION :
Expands the range to include the data values of only those fe_region settings
in the <scene> which are autoranging and which point to this spectrum.
==============================================================================*/
{
	int return_code;
	struct Scene_get_data_range_for_spectrum_data *data;

	ENTER(GT_element_settings_get_data_spectrum_parameters);
	if (settings &&
		(data = (struct Scene_get_data_range_for_spectrum_data *)data_void))
	{
		if (settings->visibility && (settings->spectrum == data->spectrum) &&
			settings->autorange_spectrum_flag)
		{
			get_graphics_object_data_range(settings->graphics_object,
				(void *)&(data->range));
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_get_data_spectrum_parameters.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_get_data_spectrum_parameters */

int GT_element_settings_get_data_spectrum_parameters(
	struct GT_element_settings *settings,
	struct Computed_field **data_field,struct Spectrum **spectrum)
/*******************************************************************************
LAST MODIFIED : 24 March 1999

DESCRIPTION :
Returns parameters used for colouring the graphics created from <settings> with
the <data_field> by the <spectrum>.
For type GT_ELEMENT_SETTINGS_STREAMLINES use function
GT_element_settings_get_data_spectrum_parameters_streamlines instead.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_settings_get_data_spectrum_parameters);
	if (settings&&data_field&&spectrum&&
		(GT_ELEMENT_SETTINGS_STREAMLINES != settings->settings_type))
	{
		*data_field=settings->data_field;
		*spectrum=settings->spectrum;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_get_data_spectrum_parameters.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_get_data_spectrum_parameters */

int GT_element_settings_set_data_spectrum_parameters(
	struct GT_element_settings *settings,struct Computed_field *data_field,
	struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 24 March 1999

DESCRIPTION :
Sets parameters used for colouring the graphics created from <settings> with
the <data_field> by the <spectrum>.
For type GT_ELEMENT_SETTINGS_STREAMLINES use function
GT_element_settings_set_data_spectrum_parameters_streamlines instead.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_settings_set_data_spectrum_parameters);
	if (settings&&((!data_field)||spectrum)&&
		(GT_ELEMENT_SETTINGS_STREAMLINES != settings->settings_type))
	{
		return_code=1;
		REACCESS(Computed_field)(&(settings->data_field),data_field);
		if (!data_field)
		{
			/* don't want settings accessing spectrum when not using it: */
			spectrum=(struct Spectrum *)NULL;
		}
		REACCESS(Spectrum)(&(settings->spectrum),spectrum);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_set_data_spectrum_parameters.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_set_data_spectrum_parameters */

int GT_element_settings_get_data_spectrum_parameters_streamlines(
	struct GT_element_settings *settings,
	enum Streamline_data_type *streamline_data_type,
	struct Computed_field **data_field,struct Spectrum **spectrum)
/*******************************************************************************
LAST MODIFIED : 24 March 1999

DESCRIPTION :
Special version of GT_element_settings_get_data_spectrum_parameters for type
GT_ELEMENT_SETTINGS_STREAMLINES only - handles the extended range of scalar data
options for streamlines - eg. STREAM_TRAVEL_SCALAR.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_settings_get_data_spectrum_parameters_streamlines);
	if (settings&&streamline_data_type&&data_field&&spectrum&&
		(GT_ELEMENT_SETTINGS_STREAMLINES==settings->settings_type))
	{
		*streamline_data_type=settings->streamline_data_type;
		*data_field=settings->data_field;
		*spectrum=settings->spectrum;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_get_data_spectrum_parameters_streamlines.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_get_data_spectrum_parameters_streamlines */

int GT_element_settings_set_data_spectrum_parameters_streamlines(
	struct GT_element_settings *settings,
	enum Streamline_data_type streamline_data_type,
	struct Computed_field *data_field,struct Spectrum *spectrum)
/*******************************************************************************
LAST MODIFIED : 24 March 1999

DESCRIPTION :
Special version of GT_element_settings_set_data_spectrum_parameters for type
GT_ELEMENT_SETTINGS_STREAMLINES only - handles the extended range of scalar data
options for streamlines - eg. STREAM_TRAVEL_SCALAR.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_settings_set_data_spectrum_parameters_streamlines);
	if (settings&&((STREAM_FIELD_SCALAR!=streamline_data_type)||data_field)&&
		((STREAM_NO_DATA==streamline_data_type)||spectrum)&&
		(GT_ELEMENT_SETTINGS_STREAMLINES==settings->settings_type))
	{
		return_code=1;
		settings->streamline_data_type=streamline_data_type;
		if (STREAM_FIELD_SCALAR!=streamline_data_type)
		{
			/* don't want settings accessing data_field when not using it: */
			data_field=(struct Computed_field *)NULL;
		}
		REACCESS(Computed_field)(&(settings->data_field),data_field);
		if (STREAM_NO_DATA==streamline_data_type)
		{
			/* don't want settings accessing spectrum when not using it: */
			spectrum=(struct Spectrum *)NULL;
		}
		REACCESS(Spectrum)(&(settings->spectrum),spectrum);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_set_data_spectrum_parameters_streamlines.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_set_data_spectrum_parameters_streamlines */

enum Render_type GT_element_settings_get_render_type(
	struct GT_element_settings *settings)
/*******************************************************************************
LAST MODIFIED : 19 March 2001

DESCRIPTION :
Get the type for how the graphics will be rendered in GL.
==============================================================================*/
{
	enum Render_type render_type;

	ENTER(GT_element_settings_get_render_type);
	if (settings)
	{
		render_type=settings->render_type;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_get_render_type.  Invalid argument(s)");
		render_type = RENDER_TYPE_SHADED;
	}
	LEAVE;

	return (render_type);
} /* GT_element_settings_get_render_type */

int GT_element_settings_set_render_type(
	struct GT_element_settings *settings, enum Render_type render_type)
/*******************************************************************************
LAST MODIFIED : 19 March 2001

DESCRIPTION :
Set the type for how the graphics will be rendered in GL.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_settings_set_render_type);
	if (settings)
	{
		return_code = 1;
		settings->render_type = render_type;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_set_render_type.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_set_render_type */

int GT_element_settings_get_line_width(struct GT_element_settings *settings)
/*******************************************************************************
LAST MODIFIED : 22 April 2004

DESCRIPTION :
Get the settings line width.  If it is 0 then the line will use the scene default.
==============================================================================*/
{
	int line_width;

	ENTER(GT_element_settings_get_line_width);
	if (settings)
	{
		line_width=settings->line_width;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_get_line_width.  Invalid argument(s)");
		line_width = 0;
	}
	LEAVE;

	return (line_width);
} /* GT_element_settings_get_line_width */

int GT_element_settings_set_line_width(struct GT_element_settings *settings,
	int line_width)
/*******************************************************************************
LAST MODIFIED : 22 April 2004

DESCRIPTION :
Sets the settings line width.  If it is 0 then the line will use the scene default.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_settings_set_line_width);
	if (settings)
	{
		return_code = 1;
		settings->line_width = line_width;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_set_line_width.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_set_line_width */

int GT_element_settings_get_dimension(struct GT_element_settings *settings)
/*******************************************************************************
LAST MODIFIED : 20 December 1999

DESCRIPTION :
Returns the dimension of the <settings>, which varies for some settings types.
==============================================================================*/
{
	int dimension;

	ENTER(GT_element_settings_get_dimension);
	if (settings)
	{
		switch (settings->settings_type)
		{
			case GT_ELEMENT_SETTINGS_NODE_POINTS:
			case GT_ELEMENT_SETTINGS_DATA_POINTS:
			{
				dimension=0;
			} break;
			case GT_ELEMENT_SETTINGS_LINES:
			case GT_ELEMENT_SETTINGS_CYLINDERS:
			{
				dimension=1;
			} break;
			case GT_ELEMENT_SETTINGS_SURFACES:
			{
				dimension=2;
			} break;
			case GT_ELEMENT_SETTINGS_VOLUMES:
			case GT_ELEMENT_SETTINGS_STREAMLINES:
			{
				dimension=3;
			} break;
			case GT_ELEMENT_SETTINGS_ELEMENT_POINTS:
			case GT_ELEMENT_SETTINGS_ISO_SURFACES:
			{
				dimension=Use_element_type_dimension(settings->use_element_type);
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"GT_element_settings_get_dimension.  Unknown settings type");
				dimension=-1;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_get_dimension.  Invalid argument(s)");
		dimension=0;
	}
	LEAVE;

	return (dimension);
} /* GT_element_settings_get_dimension */

int GT_element_settings_get_discretization(struct GT_element_settings *settings,
	struct Element_discretization *discretization)
/*******************************************************************************
LAST MODIFIED : 15 February 1999

DESCRIPTION :
Returns the <discretization> of points where glyphs are displayed for <settings>
of type GT_ELEMENT_SETTINGS_ELEMENT_POINTS.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_settings_get_discretization);
	if (settings&&discretization&&
		(GT_ELEMENT_SETTINGS_ELEMENT_POINTS==settings->settings_type))
	{
		discretization->number_in_xi1=settings->discretization.number_in_xi1;
		discretization->number_in_xi2=settings->discretization.number_in_xi2;
		discretization->number_in_xi3=settings->discretization.number_in_xi3;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_get_discretization.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_get_discretization */

int GT_element_settings_set_discretization(struct GT_element_settings *settings,
	struct Element_discretization *discretization,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 15 February 1999

DESCRIPTION :
Sets the <discretization> of points where glyphs are displayed for <settings>
of type GT_ELEMENT_SETTINGS_ELEMENT_POINTS.
???RC user_interface argument not checked as may not be needed in
check_Element_discretization().
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_settings_set_discretization);
	if (settings&&discretization&&
		(GT_ELEMENT_SETTINGS_ELEMENT_POINTS==settings->settings_type))
	{
		if (return_code=check_Element_discretization(discretization,user_interface))
		{
			settings->discretization.number_in_xi1=discretization->number_in_xi1;
			settings->discretization.number_in_xi2=discretization->number_in_xi2;
			settings->discretization.number_in_xi3=discretization->number_in_xi3;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_set_discretization.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_set_discretization */

int GT_element_settings_get_exterior(struct GT_element_settings *settings)
/*******************************************************************************
LAST MODIFIED : 28 June 1999

DESCRIPTION :
Returns 1 if <settings> is only using exterior elements.
For 1-D and 2-D settings types only.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_settings_get_exterior);
	if (settings&&(
		GT_element_settings_type_uses_dimension(settings->settings_type,1)||
		GT_element_settings_type_uses_dimension(settings->settings_type,2)))
	{
		return_code=settings->exterior;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_get_exterior.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_get_exterior */

int GT_element_settings_set_exterior(struct GT_element_settings *settings,
	int exterior)
/*******************************************************************************
LAST MODIFIED : 28 June 1999

DESCRIPTION :
Value of <exterior> flag sets whether <settings> uses exterior elements.
For 1-D and 2-D settings types only.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_settings_set_exterior);
	if (settings&&(
		GT_element_settings_type_uses_dimension(settings->settings_type,1)||
		GT_element_settings_type_uses_dimension(settings->settings_type,2)))
	{
		return_code=1;
		/* ensure flags are 0 or 1 to simplify comparison with other settings */
		if (exterior)
		{
			settings->exterior = 1;
		}
		else
		{
			settings->exterior = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_set_exterior.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_set_exterior */

int GT_element_settings_get_face(struct GT_element_settings *settings,int *face)
/*******************************************************************************
LAST MODIFIED : 28 June 1999

DESCRIPTION :
Returns 1 if the settings refers to a particular face, while the face number,
where 0 is xi1=0, 1 is xi1=1, 2 is xi2=0 etc. returned in <face>.
For 1-D and 2-D settings types only.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_settings_get_face);
	if (settings&&face&&(
		GT_element_settings_type_uses_dimension(settings->settings_type,1)||
		GT_element_settings_type_uses_dimension(settings->settings_type,2)))
	{
		return_code=(0 <= settings->face);
		*face=settings->face;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_get_face.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_get_face */

int GT_element_settings_set_face(struct GT_element_settings *settings,int face)
/*******************************************************************************
LAST MODIFIED : 28 June 1999

DESCRIPTION :
Sets the <face> used by <settings>, where 0 is xi1=0, 1 is xi1=1, 2 is xi2=0,
etc. If a value outside of 0 to 5 is passed, no face is specified.
For 1-D and 2-D settings types only.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_settings_set_face);
	if (settings&&(
		GT_element_settings_type_uses_dimension(settings->settings_type,1)||
		GT_element_settings_type_uses_dimension(settings->settings_type,2)))
	{
		return_code=1;
		/* want -1 to represent none/all faces */
		if ((0>face)||(5<face))
		{
			face = -1;
		}
		settings->face = face;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_set_face.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_set_face */

enum Graphics_select_mode GT_element_settings_get_select_mode(
	struct GT_element_settings *settings)
/*******************************************************************************
LAST MODIFIED : 19 March 2001

DESCRIPTION :
Returns the enumerator determining whether names are output with the graphics
for the settings, and if so which graphics are output depending on their
selection status.
==============================================================================*/
{
	enum Graphics_select_mode select_mode;

	ENTER(GT_element_settings_get_select_mode);
	if (settings)
	{
		select_mode = settings->select_mode;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_get_select_mode.  Invalid argument(s)");
		select_mode = GRAPHICS_NO_SELECT;
	}
	LEAVE;

	return (select_mode);
} /* GT_element_settings_get_select_mode */

int GT_element_settings_set_select_mode(struct GT_element_settings *settings,
	enum Graphics_select_mode select_mode)
/*******************************************************************************
LAST MODIFIED : 23 February 2000

DESCRIPTION :
Sets the enumerator determining whether names are output with the graphics
for the settings, and if so which graphics are output depending on their
selection status.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_settings_set_select_mode);
	if (settings)
	{
		settings->select_mode = select_mode;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_set_select_mode.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_set_select_mode */

int GT_element_settings_get_glyph_parameters(
	struct GT_element_settings *settings,
	struct GT_object **glyph, enum Glyph_scaling_mode *glyph_scaling_mode,
	Triple glyph_centre, Triple glyph_size,
	struct Computed_field **orientation_scale_field, Triple glyph_scale_factors,
	struct Computed_field **variable_scale_field)
/*******************************************************************************
LAST MODIFIED : 10 November 2000

DESCRIPTION :
Returns the current glyph and parameters for orienting and scaling it.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_settings_get_glyph_parameters);
	if (settings && glyph && glyph_scaling_mode && glyph_centre && glyph_size &&
		((GT_ELEMENT_SETTINGS_NODE_POINTS==settings->settings_type) ||
			(GT_ELEMENT_SETTINGS_DATA_POINTS==settings->settings_type) ||
			(GT_ELEMENT_SETTINGS_ELEMENT_POINTS==settings->settings_type)) &&
		orientation_scale_field && glyph_scale_factors && variable_scale_field)
	{
		*glyph = settings->glyph;
		*glyph_scaling_mode = settings->glyph_scaling_mode;
		glyph_centre[0] = settings->glyph_centre[0];
		glyph_centre[1] = settings->glyph_centre[1];
		glyph_centre[2] = settings->glyph_centre[2];
		glyph_size[0] = settings->glyph_size[0];
		glyph_size[1] = settings->glyph_size[1];
		glyph_size[2] = settings->glyph_size[2];
		*orientation_scale_field = settings->orientation_scale_field;
		glyph_scale_factors[0] = settings->glyph_scale_factors[0];
		glyph_scale_factors[1] = settings->glyph_scale_factors[1];
		glyph_scale_factors[2] = settings->glyph_scale_factors[2];
		*variable_scale_field = settings->variable_scale_field;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_get_glyph_parameters.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_get_glyph_parameters */

int GT_element_settings_set_glyph_parameters(
	struct GT_element_settings *settings,
	struct GT_object *glyph, enum Glyph_scaling_mode glyph_scaling_mode,
	Triple glyph_centre, Triple glyph_size,
	struct Computed_field *orientation_scale_field, Triple glyph_scale_factors,
	struct Computed_field *variable_scale_field)
/*******************************************************************************
LAST MODIFIED : 10 November 2000

DESCRIPTION :
Sets the glyph and parameters for orienting and scaling it.
See function make_glyph_orientation_scale_axes in
finite_element/finite_element_to_graphics object for explanation of how the
<orientation_scale_field> is used.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_settings_set_glyph_parameters);
	if (settings && glyph && glyph_centre && glyph_size &&
		((GT_ELEMENT_SETTINGS_NODE_POINTS==settings->settings_type)||
			(GT_ELEMENT_SETTINGS_DATA_POINTS==settings->settings_type)||
			(GT_ELEMENT_SETTINGS_ELEMENT_POINTS==settings->settings_type))&&
		((!orientation_scale_field) || Computed_field_is_orientation_scale_capable(
			orientation_scale_field,(void *)NULL)) && glyph_scale_factors &&
		((!variable_scale_field) || Computed_field_has_up_to_3_numerical_components(
			variable_scale_field,(void *)NULL)))
	{
		if (settings->glyph)
		{
			GT_object_remove_callback(settings->glyph,
				GT_element_settings_glyph_change, (void *)settings);
		}
		REACCESS(GT_object)(&(settings->glyph),glyph);
		GT_object_add_callback(settings->glyph, GT_element_settings_glyph_change,
				(void *)settings);
		settings->glyph_scaling_mode = glyph_scaling_mode;
		settings->glyph_centre[0] = glyph_centre[0];
		settings->glyph_centre[1] = glyph_centre[1];
		settings->glyph_centre[2] = glyph_centre[2];
		settings->glyph_size[0] = glyph_size[0];
		settings->glyph_size[1] = glyph_size[1];
		settings->glyph_size[2] = glyph_size[2];
		REACCESS(Computed_field)(&(settings->orientation_scale_field),
			orientation_scale_field);
		settings->glyph_scale_factors[0]=glyph_scale_factors[0];
		settings->glyph_scale_factors[1]=glyph_scale_factors[1];
		settings->glyph_scale_factors[2]=glyph_scale_factors[2];
		REACCESS(Computed_field)(&(settings->variable_scale_field),
			variable_scale_field);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_set_glyph_parameters.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_set_glyph_parameters */

int GT_element_settings_get_iso_surface_parameters(
	struct GT_element_settings *settings,struct Computed_field **iso_scalar_field,
	int *number_of_iso_values, double **iso_values, double *decimation_threshold)
/*******************************************************************************
LAST MODIFIED : 21 February 2005

DESCRIPTION :
Returns parameters for the iso_surface: iso_scalar_field = iso_value.
For settings_type GT_ELEMENT_SETTINGS_ISO_SURFACES only.
==============================================================================*/
{
	int i, return_code;

	ENTER(GT_element_settings_get_iso_surface_parameters);
	if (settings&&iso_scalar_field&&number_of_iso_values&&iso_values&&
		decimation_threshold&&
		(GT_ELEMENT_SETTINGS_ISO_SURFACES==settings->settings_type))
	{
		*iso_scalar_field=settings->iso_scalar_field;
		*decimation_threshold=settings->decimation_threshold;
		if (0 < settings->number_of_iso_values)
		{
			if (ALLOCATE(*iso_values, double, settings->number_of_iso_values))
			{
				for (i = 0 ; i < settings->number_of_iso_values ; i++)
				{
					(*iso_values)[i] = settings->iso_values[i];
				}
				*number_of_iso_values = settings->number_of_iso_values;
				return_code = 1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"GT_element_settings_get_iso_surface_parameters.  "
					"Could not allocate memory/");
				return_code=0;
			}
		}
		else
		{
			*number_of_iso_values=settings->number_of_iso_values;
			*iso_values=(double *)NULL;
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_get_iso_surface_parameters.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_get_iso_surface_parameters */

int GT_element_settings_set_iso_surface_parameters(
	struct GT_element_settings *settings,struct Computed_field *iso_scalar_field,
	int number_of_iso_values, double *iso_values, double decimation_threshold)
/*******************************************************************************
LAST MODIFIED : 21 February 2005

DESCRIPTION :
Sets parameters for the iso_surface: iso_scalar_field = iso_value.
For settings_type GT_ELEMENT_SETTINGS_ISO_SURFACES only - must call this after
CREATE to define a valid iso_surface.
==============================================================================*/
{
	int i, return_code;

	ENTER(GT_element_settings_set_iso_surface_parameters);
	if (settings&&iso_scalar_field&&
		(1==Computed_field_get_number_of_components(iso_scalar_field))&&
		(GT_ELEMENT_SETTINGS_ISO_SURFACES==settings->settings_type))
	{
		return_code=1;
		REACCESS(Computed_field)(&(settings->iso_scalar_field),iso_scalar_field);
		settings->decimation_threshold = decimation_threshold;
		if (0 < number_of_iso_values)
		{
			if (REALLOCATE(settings->iso_values, settings->iso_values, double,
					number_of_iso_values))
			{
				settings->number_of_iso_values = number_of_iso_values;
				for (i = 0 ; i < number_of_iso_values ; i++)
				{
					settings->iso_values[i] = iso_values[i];
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"GT_element_settings_set_iso_surface_parameters.  "
					"Could not allocate memory.");
				return_code=0;
			}
		}
		else
		{
			if (settings->iso_values)
			{
				DEALLOCATE(settings->iso_values);
				settings->iso_values = (double *)NULL;
			}
			settings->number_of_iso_values = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_set_iso_surface_parameters.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_set_iso_surface_parameters */

int GT_element_settings_get_label_field(struct GT_element_settings *settings,
	struct Computed_field **label_field, struct Graphics_font **font)
/*******************************************************************************
LAST MODIFIED : 18 November 2005

DESCRIPTION :
Returns the label_field and font used by <settings>. For settings types
GT_ELEMENT_SETTINGS_NODE_POINTS, GT_ELEMENT_SETTINGS_DATA_POINTS and
GT_ELEMENT_SETTINGS_ELEMENT_POINTS only.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_settings_get_label_field);
	if (settings&&
		((GT_ELEMENT_SETTINGS_NODE_POINTS==settings->settings_type)||
			(GT_ELEMENT_SETTINGS_DATA_POINTS==settings->settings_type)||
			(GT_ELEMENT_SETTINGS_ELEMENT_POINTS==settings->settings_type)))
	{
		*label_field = settings->label_field;
		*font = settings->font;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_get_label_field.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_get_label_field */

int GT_element_settings_set_label_field(
	struct GT_element_settings *settings,struct Computed_field *label_field,
	struct Graphics_font *font)
/*******************************************************************************
LAST MODIFIED : 18 November 2005

DESCRIPTION :
Sets the <label_field> used by <settings>. For settings types
GT_ELEMENT_SETTINGS_NODE_POINTS, GT_ELEMENT_SETTINGS_DATA_POINTS and
GT_ELEMENT_SETTINGS_ELEMENT_POINTS only. The field specified will be evaluated
as a string and displayed beside the glyph.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_settings_set_label_field);
	if (settings&&((!label_field)||
		((GT_ELEMENT_SETTINGS_NODE_POINTS==settings->settings_type)||
			(GT_ELEMENT_SETTINGS_DATA_POINTS==settings->settings_type)||
			(GT_ELEMENT_SETTINGS_ELEMENT_POINTS==settings->settings_type))) &&
		(!label_field || font))
	{
		REACCESS(Computed_field)(&(settings->label_field), label_field);
		REACCESS(Graphics_font)(&(settings->font), font);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_set_label_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_set_label_field */

struct Graphical_material *GT_element_settings_get_material(
	struct GT_element_settings *settings)
/*******************************************************************************
LAST MODIFIED : 5 June 1998

DESCRIPTION :
Returns the material used by <settings>.
==============================================================================*/
{
	struct Graphical_material *material;

	ENTER(GT_element_settings_get_material);
	if (settings)
	{
		material=settings->material;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_get_material.  Invalid argument(s)");
		material=(struct Graphical_material *)NULL;
	}
	LEAVE;

	return (material);
} /* GT_element_settings_get_material */

int GT_element_settings_set_material(struct GT_element_settings *settings,
	struct Graphical_material *material)
/*******************************************************************************
LAST MODIFIED : 19 March 1999

DESCRIPTION :
Sets the <material> used by <settings>. Must set the material for each new
settings created.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_settings_set_material);
	if (settings&&material)
	{
		return_code=1;
		REACCESS(Graphical_material)(&(settings->material),material);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_set_material.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_set_material */

struct Graphical_material *GT_element_settings_get_secondary_material(
	struct GT_element_settings *settings)
/*******************************************************************************
LAST MODIFIED : 30 September 2005

DESCRIPTION :
Returns the secondary_material used by <settings>.
==============================================================================*/
{
	struct Graphical_material *material;

	ENTER(GT_element_settings_get_secondary_material);
	if (settings)
	{
		material=settings->secondary_material;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_get_secondary_material.  Invalid argument(s)");
		material=(struct Graphical_material *)NULL;
	}
	LEAVE;

	return (material);
} /* GT_element_settings_get_secondary_material */

int GT_element_settings_set_secondary_material(struct GT_element_settings *settings,
	struct Graphical_material *material)
/*******************************************************************************
LAST MODIFIED : 30 September 2005

DESCRIPTION :
Sets the <secondary_material> used by <settings>.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_settings_set_secondary_material);
	if (settings&&material)
	{
		return_code=1;
		REACCESS(Graphical_material)(&(settings->secondary_material),
			material);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_set_secondary_material.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_set_secondary_material */

struct Graphical_material *GT_element_settings_get_selected_material(
	struct GT_element_settings *settings)
/*******************************************************************************
LAST MODIFIED : 18 February 2000

DESCRIPTION :
Returns the selected material used by <settings>.
Selected objects relevant to the settings are displayed with this material.
==============================================================================*/
{
	struct Graphical_material *selected_material;

	ENTER(GT_element_settings_get_selected_material);
	if (settings)
	{
		selected_material=settings->selected_material;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_get_selected_material.  Invalid argument(s)");
		selected_material=(struct Graphical_material *)NULL;
	}
	LEAVE;

	return (selected_material);
} /* GT_element_settings_get_selected_material */

int GT_element_settings_set_selected_material(
	struct GT_element_settings *settings,
	struct Graphical_material *selected_material)
/*******************************************************************************
LAST MODIFIED : 18 February 2000

DESCRIPTION :
Sets the <selected_material> used by <settings>.
Selected objects relevant to the settings are displayed with this material.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_settings_set_selected_material);
	if (settings&&selected_material)
	{
		return_code=1;
		REACCESS(Graphical_material)(&(settings->selected_material),
			selected_material);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_set_selected_material.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_set_selected_material */

struct FE_field *GT_element_settings_get_native_discretization_field(
	struct GT_element_settings *settings)
/*******************************************************************************
LAST MODIFIED : 3 March 1999

DESCRIPTION :
Returns the native_discretization field used by <settings>.
For type GT_ELEMENT_SETTINGS_ELEMENT_POINTS only.
==============================================================================*/
{
	struct FE_field *native_discretization_field;

	ENTER(GT_element_settings_get_native_discretization_field);
	if (settings&&
		(GT_ELEMENT_SETTINGS_ELEMENT_POINTS==settings->settings_type))
	{
		native_discretization_field=settings->native_discretization_field;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_get_native_discretization_field.  "
			"Invalid argument(s)");
		native_discretization_field=(struct FE_field *)NULL;
	}
	LEAVE;

	return (native_discretization_field);
} /* GT_element_settings_get_native_discretization_field */

int GT_element_settings_set_native_discretization_field(
	struct GT_element_settings *settings,
	struct FE_field *native_discretization_field)
/*******************************************************************************
LAST MODIFIED : 19 March 1999

DESCRIPTION :
Sets the <native_discretization_field> used by <settings>. If the settings have
such a field, and the field is element based over the element in question, then
the native discretization of that field in that element is used instead of the
settings->discretization.
For type GT_ELEMENT_SETTINGS_ELEMENT_POINTS only.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_settings_set_native_discretization_field);
	if (settings&&
		(GT_ELEMENT_SETTINGS_ELEMENT_POINTS==settings->settings_type))
	{
		return_code=1;
		REACCESS(FE_field)(&(settings->native_discretization_field),
			native_discretization_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_set_native_discretization_field.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_set_native_discretization_field */

int GT_element_settings_get_radius_parameters(
	struct GT_element_settings *settings,float *constant_radius,
	float *radius_scale_factor,struct Computed_field **radius_scalar_field)
/*******************************************************************************
LAST MODIFIED : 15 February 1999

DESCRIPTION :
Returns the current radius parameters which are used in the expression:
radius = constant_radius + radius_scale_factor*radius_scalar_field.
For settings_type GT_ELEMENT_SETTINGS_CYLINDERS only.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_settings_get_radius_parameters);
	if (settings&&constant_radius&&radius_scale_factor&&radius_scalar_field&&
		(GT_ELEMENT_SETTINGS_CYLINDERS==settings->settings_type))
	{
		return_code=1;
		*constant_radius=settings->constant_radius;
		*radius_scale_factor=settings->radius_scale_factor;
		*radius_scalar_field=settings->radius_scalar_field;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_get_radius_parameters.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_get_radius_parameters */

int GT_element_settings_set_radius_parameters(
	struct GT_element_settings *settings,float constant_radius,
	float radius_scale_factor,struct Computed_field *radius_scalar_field)
/*******************************************************************************
LAST MODIFIED : 19 March 1999

DESCRIPTION :
Sets the current radius parameters which are used in the expression:
radius = constant_radius + radius_scale_factor*radius_scalar_field.
For settings_type GT_ELEMENT_SETTINGS_CYLINDERS only.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_settings_set_radius_parameters);
	if (settings&&(GT_ELEMENT_SETTINGS_CYLINDERS==settings->settings_type)&&
		((!radius_scalar_field)||
			(1==Computed_field_get_number_of_components(radius_scalar_field))))
	{
		return_code=1;
		settings->constant_radius=constant_radius;
		settings->radius_scale_factor=radius_scale_factor;
		REACCESS(Computed_field)(&(settings->radius_scalar_field),
			radius_scalar_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_set_radius_parameters.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_set_radius_parameters */

struct FE_element *GT_element_settings_get_seed_element(
	struct GT_element_settings *settings)
/*******************************************************************************
LAST MODIFIED : 18 August 1998

DESCRIPTION :
For settings starting in a particular element.
==============================================================================*/
{
	struct FE_element *seed_element;

	ENTER(GT_element_settings_get_seed_element);
	if (settings&&((GT_ELEMENT_SETTINGS_VOLUMES==settings->settings_type)||
		(GT_ELEMENT_SETTINGS_STREAMLINES==settings->settings_type)))
	{
		seed_element=settings->seed_element;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_get_seed_element.  Invalid argument(s)");
		seed_element=(struct FE_element *)NULL;
	}
	LEAVE;

	return (seed_element);
} /* GT_element_settings_get_seed_element */

int GT_element_settings_set_seed_element(struct GT_element_settings *settings,
	struct FE_element *seed_element)
/*******************************************************************************
LAST MODIFIED : 19 March 1999

DESCRIPTION :
For settings starting in a particular element.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_settings_set_seed_element);
	if (settings&&((GT_ELEMENT_SETTINGS_VOLUMES==settings->settings_type)||
		(GT_ELEMENT_SETTINGS_STREAMLINES==settings->settings_type)))
	{
		REACCESS(FE_element)(&settings->seed_element,seed_element);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_set_seed_element.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_set_seed_element */

int GT_element_settings_get_seed_xi(struct GT_element_settings *settings,
	Triple seed_xi)
/*******************************************************************************
LAST MODIFIED : 7 June 2000

DESCRIPTION :
For settings_types GT_ELEMENT_SETTINGS_ELEMENT_POINTS or
GT_ELEMENT_SETTINGS_STREAMLINES only.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_settings_get_seed_xi);
	if (settings&&seed_xi&&(
		(GT_ELEMENT_SETTINGS_ELEMENT_POINTS==settings->settings_type)||
		(GT_ELEMENT_SETTINGS_STREAMLINES==settings->settings_type)))
	{
		seed_xi[0]=settings->seed_xi[0];
		seed_xi[1]=settings->seed_xi[1];
		seed_xi[2]=settings->seed_xi[2];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_get_seed_xi.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_get_seed_xi */

int GT_element_settings_set_seed_xi(struct GT_element_settings *settings,
	Triple seed_xi)
/*******************************************************************************
LAST MODIFIED : 7 June 2000

DESCRIPTION :
For settings_types GT_ELEMENT_SETTINGS_ELEMENT_POINTS or
GT_ELEMENT_SETTINGS_STREAMLINES only.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_settings_set_seed_xi);
	if (settings&&seed_xi&&(
		(GT_ELEMENT_SETTINGS_ELEMENT_POINTS==settings->settings_type)||
		(GT_ELEMENT_SETTINGS_STREAMLINES==settings->settings_type)))
	{
		settings->seed_xi[0]=seed_xi[0];
		settings->seed_xi[1]=seed_xi[1];
		settings->seed_xi[2]=seed_xi[2];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_set_seed_xi.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_set_seed_xi */

int GT_element_settings_get_streamline_parameters(
	struct GT_element_settings *settings,enum Streamline_type *streamline_type,
	struct Computed_field **stream_vector_field,int *reverse_track,
	float *streamline_length,float *streamline_width)
/*******************************************************************************
LAST MODIFIED : 24 March 1999

DESCRIPTION :
For settings_type GT_ELEMENT_SETTINGS_STREAMLINES only.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_settings_get_streamline_parameters);
	if (settings&&streamline_type&&stream_vector_field&&reverse_track&&
		streamline_length&&streamline_width&&
		(GT_ELEMENT_SETTINGS_STREAMLINES==settings->settings_type))
	{
		*streamline_type=settings->streamline_type;
		*stream_vector_field=settings->stream_vector_field;
		*reverse_track=settings->reverse_track;
		*streamline_length=settings->streamline_length;
		*streamline_width=settings->streamline_width;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_get_streamline_parameters.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_get_streamline_parameters */

int GT_element_settings_set_streamline_parameters(
	struct GT_element_settings *settings,enum Streamline_type streamline_type,
	struct Computed_field *stream_vector_field,int reverse_track,
	float streamline_length,float streamline_width)
/*******************************************************************************
LAST MODIFIED : 24 March 1999

DESCRIPTION :
For settings_type GT_ELEMENT_SETTINGS_STREAMLINES only.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_settings_set_streamline_parameters);
	if (settings&&stream_vector_field&&
		(GT_ELEMENT_SETTINGS_STREAMLINES==settings->settings_type))
	{
		settings->streamline_type=streamline_type;
		REACCESS(Computed_field)(&(settings->stream_vector_field),
			stream_vector_field);
		settings->reverse_track=reverse_track;
		settings->streamline_length=streamline_length;
		settings->streamline_width=streamline_width;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_set_streamline_parameters.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_set_streamline_parameters */

enum Use_element_type GT_element_settings_get_use_element_type(
	struct GT_element_settings *settings)
/*******************************************************************************
LAST MODIFIED : 20 March 2001

DESCRIPTION :
Returns the type of elements used by the settings.
For <settings> type GT_ELEMENT_SETTINGS_ELEMENT_POINTS and
GT_ELEMENT_SETTINGS_ISO_SURFACES only.
==============================================================================*/
{
	enum Use_element_type use_element_type;

	ENTER(GT_element_settings_get_use_element_type);
	if (settings&&((GT_ELEMENT_SETTINGS_ELEMENT_POINTS==settings->settings_type)||
		(GT_ELEMENT_SETTINGS_ISO_SURFACES==settings->settings_type)))
	{
		use_element_type=settings->use_element_type;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_get_use_element_type.  Invalid argument(s)");
		use_element_type = USE_ELEMENTS;
	}
	LEAVE;

	return (use_element_type);
} /* GT_element_settings_get_use_element_type */

int GT_element_settings_set_use_element_type(
	struct GT_element_settings *settings,enum Use_element_type use_element_type)
/*******************************************************************************
LAST MODIFIED : 28 January 2000

DESCRIPTION :
Sets the type of elements used by the settings.
For <settings> type GT_ELEMENT_SETTINGS_ELEMENT_POINTS and
GT_ELEMENT_SETTINGS_ISO_SURFACES only.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_settings_set_use_element_type);
	if (settings&&((GT_ELEMENT_SETTINGS_ELEMENT_POINTS==settings->settings_type)||
		(GT_ELEMENT_SETTINGS_ISO_SURFACES==settings->settings_type)))
	{
		settings->use_element_type=use_element_type;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_set_use_element_type.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_set_use_element_type */

int GT_element_settings_get_visibility(struct GT_element_settings *settings)
/*******************************************************************************
LAST MODIFIED : 10 June 1998

DESCRIPTION :
Returns 1 if the graphics are visible for <settings>, otherwise 0.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_settings_get_visibility);
	if (settings)
	{
		return_code=settings->visibility;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_get_visibility.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_get_visibility */

int GT_element_settings_set_visibility(struct GT_element_settings *settings,
	int visibility)
/*******************************************************************************
LAST MODIFIED : 10 June 1998

DESCRIPTION :
Sets the visibility of the graphics described by the settings, where 1 is
visible, 0 is invisible.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_settings_set_visibility);
	if (settings)
	{
		return_code=1;
		if ((visibility && !settings->visibility) ||
			(!visibility && settings->visibility))
		{
			settings->visibility = !settings->visibility;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_set_visibility.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_set_visibility */

struct VT_volume_texture *GT_element_settings_get_volume_texture(
	struct GT_element_settings *settings)
/*******************************************************************************
LAST MODIFIED : 27 July 1998

DESCRIPTION :
For settings_type GT_ELEMENT_SETTINGS_VOLUMES only.
==============================================================================*/
{
	struct VT_volume_texture *volume_texture;

	ENTER(GT_element_settings_get_volume_texture);
	if (settings&&(GT_ELEMENT_SETTINGS_VOLUMES==settings->settings_type))
	{
		volume_texture=settings->volume_texture;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_get_volume_texture.  Invalid argument(s)");
		volume_texture=(struct VT_volume_texture *)NULL;
	}
	LEAVE;

	return (volume_texture);
} /* GT_element_settings_get_volume_texture */

int GT_element_settings_set_volume_texture(struct GT_element_settings *settings,
	struct VT_volume_texture *volume_texture)
/*******************************************************************************
LAST MODIFIED : 19 March 1999

DESCRIPTION :
For settings_type GT_ELEMENT_SETTINGS_VOLUMES only.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_settings_set_volume_texture);
	if (settings&&volume_texture&&
		(GT_ELEMENT_SETTINGS_VOLUMES==settings->settings_type))
	{
		REACCESS(VT_volume_texture)(&(settings->volume_texture),volume_texture);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_set_volume_texture.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_set_volume_texture */

int GT_element_settings_get_xi_discretization(
	struct GT_element_settings *settings,
	enum Xi_discretization_mode *xi_discretization_mode,
	struct Computed_field **xi_point_density_field)
/*******************************************************************************
LAST MODIFIED : 3 May 2001

DESCRIPTION :
Returns the <xi_discretization_mode> and <xi_point_density_field> controlling
where glyphs are displayed for <settings> of type
GT_ELEMENT_SETTINGS_ELEMENT_POINTS. <xi_point_density_field> is used only with
XI_DISCRETIZATION_CELL_DENSITY and XI_DISCRETIZATION_CELL_POISSON modes.
Either <xi_discretization_mode> or <xi_point_density_field> addresses may be
omitted, if that value is not required.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_settings_get_xi_discretization);
	if (settings &&
		(GT_ELEMENT_SETTINGS_ELEMENT_POINTS == settings->settings_type))
	{
		if (xi_discretization_mode)
		{
			*xi_discretization_mode = settings->xi_discretization_mode;
		}
		if (xi_point_density_field)
		{
			*xi_point_density_field = settings->xi_point_density_field;
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_get_xi_discretization.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_get_xi_discretization */

int GT_element_settings_set_xi_discretization(
	struct GT_element_settings *settings,
	enum Xi_discretization_mode xi_discretization_mode,
	struct Computed_field *xi_point_density_field)
/*******************************************************************************
LAST MODIFIED : 3 May 2001

DESCRIPTION :
Sets the xi_discretization_mode controlling where glyphs are displayed for
<settings> of type GT_ELEMENT_SETTINGS_ELEMENT_POINTS. Must supply a scalar
<xi_point_density_field> if the new mode is XI_DISCRETIZATION_CELL_DENSITY or
XI_DISCRETIZATION_CELL_POISSON.
==============================================================================*/
{
	int need_density_field, return_code;

	ENTER(GT_element_settings_set_xi_discretization);
	need_density_field =
		(XI_DISCRETIZATION_CELL_DENSITY == xi_discretization_mode) ||
		(XI_DISCRETIZATION_CELL_POISSON == xi_discretization_mode);

	if (settings && ((!need_density_field && !xi_point_density_field) ||
		(need_density_field && xi_point_density_field &&
			Computed_field_is_scalar(xi_point_density_field, (void *)NULL))))
	{
		settings->xi_discretization_mode = xi_discretization_mode;
		REACCESS(Computed_field)(&(settings->xi_point_density_field),
			xi_point_density_field);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_set_xi_discretization.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_set_xi_discretization */

struct GT_object *GT_element_settings_get_graphics_object(
	struct GT_element_settings *settings)
/*******************************************************************************
LAST MODIFIED : 28 June 1999

DESCRIPTION :
Returns a pointer to the graphics_object in <settings>, if any. Should be used
with care - currently only used for determining picking.
==============================================================================*/
{
	struct GT_object *graphics_object
	
	ENTER(GT_element_settings_get_graphics_object);
	if (settings)
	{
		graphics_object=settings->graphics_object;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_get_graphics_object.  Invalid argument(s)");
		graphics_object=(struct GT_object *)NULL;
	}
	LEAVE;

	return (graphics_object);
} /* GT_element_settings_get_graphics_object */

int GT_element_settings_has_name(struct GT_element_settings *settings,
	void *name_void)
/*******************************************************************************
LAST MODIFIED : 7 July 1999

DESCRIPTION :
Settings iterator function returning true if <settings> has the 
specified <name>.  If the settings doesn't have a name then the position
number is converted to a string and that is compared to the supplied <name>.
==============================================================================*/
{
	char *name, temp_name[30];
	int return_code;

	ENTER(GT_element_settings_has_name);
	if (settings && (name=(char *)name_void))
	{
		return_code = 0;
		if (settings->name)
		{
			return_code=!strcmp(name,settings->name);
		}
		if (!return_code)
		{
			/* Compare with number if the settings
			 has no name or the name didn't match */
			sprintf(temp_name, "%d", settings->position);
			return_code=!strcmp(name,temp_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_has_name.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_has_name */

int GT_element_settings_has_embedded_field(
	struct GT_element_settings *settings,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 25 May 2001

DESCRIPTION :
Returns 1 if the <settings> use any embedded_fields.
==============================================================================*/
{
	int return_code;
	
	ENTER(GT_element_settings_has_embedded_field);
	USE_PARAMETER(dummy_void);
	if (settings)
	{
		return_code = 0;
		/*???RC Only relevent to data_points and node_points at present; hence
			limit our search to them. Review once we use embedded fields for elements
			too */
		if (((GT_ELEMENT_SETTINGS_DATA_POINTS == settings->settings_type) ||
			(GT_ELEMENT_SETTINGS_NODE_POINTS == settings->settings_type)) &&
			((settings->coordinate_field &&
				Computed_field_depends_on_embedded_field(settings->coordinate_field)) ||
				(settings->orientation_scale_field &&
					Computed_field_depends_on_embedded_field(
						settings->orientation_scale_field)) ||
				(settings->variable_scale_field &&
					Computed_field_depends_on_embedded_field(
						settings->variable_scale_field)) ||
				(settings->data_field &&
					Computed_field_depends_on_embedded_field(settings->data_field)) ||
				(settings->label_field &&
					Computed_field_depends_on_embedded_field(settings->label_field))))
		{
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_has_embedded_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_has_embedded_field */

int GT_element_settings_update_time_behaviour(
	struct GT_element_settings *settings, void *update_time_behaviour_void)
/*******************************************************************************
LAST MODIFIED : 6 December 2001

DESCRIPTION :
Updates the internal flag used whenever a time callback is received by the settings
and if the <settings> is time dependent then sets the flag in the 
update_time_behaviour_data structure.
If the <settings> is not time_dependent then the flag is not touched, as it
is used by an iterator to see if any one of the settings in a graphical element
group are time dependent.
==============================================================================*/
{
	int return_code, time_dependent;
	struct GT_element_settings_update_time_behaviour_data *data;
	
	ENTER(GT_element_settings_update_time_behaviour);
	if (settings && (data = 
		(struct GT_element_settings_update_time_behaviour_data *)
		update_time_behaviour_void))
	{
		return_code = 1;
		time_dependent = 0;
		if (settings->glyph && (1 < GT_object_get_number_of_times(settings->glyph)))
		{
			time_dependent = 1;
		}
		if (settings->coordinate_field)
		{
			if (Computed_field_has_multiple_times(settings->coordinate_field))
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
		if (settings->texture_coordinate_field && Computed_field_has_multiple_times(
			settings->texture_coordinate_field))
		{
			time_dependent = 1;
		}
		if (settings->radius_scalar_field && Computed_field_has_multiple_times(
			settings->radius_scalar_field))
		{
			time_dependent = 1;
		}
		if (settings->iso_scalar_field && Computed_field_has_multiple_times(
			settings->iso_scalar_field))
		{
			time_dependent = 1;
		}
		if (settings->orientation_scale_field && 
			Computed_field_has_multiple_times(settings->orientation_scale_field))
		{
			time_dependent = 1;
		}
		if (settings->variable_scale_field && 
			Computed_field_has_multiple_times(settings->variable_scale_field))
		{
			time_dependent = 1;
		}
		if (settings->label_field && 
			Computed_field_has_multiple_times(settings->label_field))
		{
			time_dependent = 1;
		}
		if (settings->variable_scale_field && 
			Computed_field_has_multiple_times(settings->variable_scale_field))
		{
			time_dependent = 1;
		}
		if (settings->displacement_map_field && 
			Computed_field_has_multiple_times(settings->displacement_map_field))
		{
			time_dependent = 1;
		}
		if (settings->stream_vector_field && 
			Computed_field_has_multiple_times(settings->stream_vector_field))
		{
			time_dependent = 1;
		}
		if (settings->data_field && 
			Computed_field_has_multiple_times(settings->data_field))
		{
			time_dependent = 1;
		}
		/* Or any field that is pointed to has multiple times...... */

		settings->time_dependent = time_dependent;
		if (time_dependent)
		{
			data->time_dependent = time_dependent;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_update_time_behaviour.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_update_time_behaviour */

static int GT_element_settings_clear_graphics(
	struct GT_element_settings *settings)
/*******************************************************************************
LAST MODIFIED : 25 March 2003

DESCRIPTION :
Clears the graphics stored in the settings and marks them for rebuild.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_settings_clear_graphics);
	if (settings)
	{
		if (settings->graphics_object)
		{
			/*???RC Once again, assumes graphics stored at time 0.0 */
			GT_object_remove_primitives_at_time(
				settings->graphics_object, /*time*/0.0,
				(GT_object_primitive_object_name_conditional_function *)NULL,
				(void *)NULL);
		}
		settings->graphics_changed = 1;
		settings->selected_graphics_changed = 1;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_clear_graphics.  Missing settings");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_clear_graphics */

int GT_element_settings_default_coordinate_field_change(
	struct GT_element_settings *settings,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 25 March 2003

DESCRIPTION :
Deaccesses any graphics_object in <settings> if it uses the default coordinate
field from the gt_element_group - that is, settings->coordinate_field is NULL.
==============================================================================*/
{
	int return_code;
	
	ENTER(GT_element_settings_default_coordinate_field_change);
	USE_PARAMETER(dummy_void);
	if (settings)
	{
		if ((struct Computed_field *)NULL == settings->coordinate_field)
		{
			GT_element_settings_clear_graphics(settings);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_default_coordinate_field_change.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_default_coordinate_field_change */

int GT_element_settings_element_discretization_change(
	struct GT_element_settings *settings,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 28 November 2000

DESCRIPTION :
Deaccesses any graphics_object in <settings> if it would have to be rebuilt
due to a change in element discretization. Currently element_points, volumes,
streamlines and 0-D types do not require a rebuild in this case.
==============================================================================*/
{
	int return_code;
	
	ENTER(GT_element_settings_element_discretization_change);
	USE_PARAMETER(dummy_void);
	if (settings)
	{
		if ((GT_element_settings_type_uses_dimension(settings->settings_type, 1) ||
			GT_element_settings_type_uses_dimension(settings->settings_type, 2) ||
			GT_element_settings_type_uses_dimension(settings->settings_type, 3)) &&
			(GT_ELEMENT_SETTINGS_ELEMENT_POINTS != settings->settings_type) &&
			(GT_ELEMENT_SETTINGS_STREAMLINES != settings->settings_type) &&
			(GT_ELEMENT_SETTINGS_VOLUMES != settings->settings_type))
		{
			GT_element_settings_clear_graphics(settings);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_element_discretization_change.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_element_discretization_change */

int GT_element_settings_circle_discretization_change(
	struct GT_element_settings *settings,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 28 June 1999

DESCRIPTION :
Deaccesses any graphics_object in <settings> if it would have to be rebuilt
due to a change in element discretization.
==============================================================================*/
{
	int return_code;
	
	ENTER(GT_element_settings_circle_discretization_change);
	USE_PARAMETER(dummy_void);
	if (settings)
	{
		if (GT_ELEMENT_SETTINGS_CYLINDERS == settings->settings_type)
		{
			GT_element_settings_clear_graphics(settings);
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_circle_discretization_change.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_circle_discretization_change */

static int GT_element_settings_Computed_field_or_ancestor_satisfies_condition(
	struct GT_element_settings *settings,
	struct Computed_field *default_coordinate_field,
	LIST_CONDITIONAL_FUNCTION(Computed_field) *conditional_function,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 2 May 2003

DESCRIPTION :
Returns true if <settings> uses any Computed_field that satisfies
<conditional_function> with <user_data>. The <default_coordinate_field> is
used if the settings has no coodinate field of its own.
==============================================================================*/
{
	int return_code;
	struct Computed_field *coordinate_field;

	ENTER(GT_element_settings_Computed_field_or_ancestor_satisfies_condition);
	if (settings && conditional_function)
	{
		return_code = 0;
		/* compare geometry settings */
		/* for all graphic types */
		/* settings can get default coordinate field from gt_element_group */
		if (settings->coordinate_field)
		{
			coordinate_field = settings->coordinate_field;
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
		else if (settings->texture_coordinate_field &&
			Computed_field_or_ancestor_satisfies_condition(
				settings->texture_coordinate_field, conditional_function, user_data))
		{
			return_code = 1;
		}
		/* for cylinders only */
		else if ((GT_ELEMENT_SETTINGS_CYLINDERS == settings->settings_type) &&
			settings->radius_scalar_field &&
			Computed_field_or_ancestor_satisfies_condition(
				settings->radius_scalar_field, conditional_function, user_data))
		{
			return_code = 1;
		}
		/* for iso_surfaces only */
		else if ((GT_ELEMENT_SETTINGS_ISO_SURFACES == settings->settings_type) &&
			(settings->iso_scalar_field &&
			Computed_field_or_ancestor_satisfies_condition(
				settings->iso_scalar_field, conditional_function, user_data)))
		{
			return_code = 1;
		}
		/* for node_points, data_points and element_points only */
		else if (((GT_ELEMENT_SETTINGS_NODE_POINTS == settings->settings_type) ||
			(GT_ELEMENT_SETTINGS_DATA_POINTS == settings->settings_type) ||
			(GT_ELEMENT_SETTINGS_ELEMENT_POINTS == settings->settings_type)) &&
			(settings->orientation_scale_field &&
				Computed_field_or_ancestor_satisfies_condition(
					settings->orientation_scale_field, conditional_function, user_data))||
			(settings->variable_scale_field &&
				Computed_field_or_ancestor_satisfies_condition(
					settings->variable_scale_field, conditional_function, user_data)) ||
			(settings->label_field &&
				Computed_field_or_ancestor_satisfies_condition(
					settings->label_field, conditional_function, user_data)))
		{
			return_code = 1;
		}
		/* for element_points with a density field only */
		else if ((GT_ELEMENT_SETTINGS_ELEMENT_POINTS == settings->settings_type)
			&& ((XI_DISCRETIZATION_CELL_DENSITY ==
				settings->xi_discretization_mode) ||
				(XI_DISCRETIZATION_CELL_POISSON ==
					settings->xi_discretization_mode)) &&
			Computed_field_or_ancestor_satisfies_condition(
				settings->xi_point_density_field, conditional_function, user_data))
		{
			return_code = 1;
		}
		/* for volumes only */
		else if ((GT_ELEMENT_SETTINGS_VOLUMES == settings->settings_type)&&
			(settings->displacement_map_field &&
				Computed_field_or_ancestor_satisfies_condition(
					settings->displacement_map_field, conditional_function, user_data)))
		{
			return_code = 1;
		}
		/* for streamlines only */
		else if ((GT_ELEMENT_SETTINGS_STREAMLINES == settings->settings_type) &&
			settings->stream_vector_field &&
			Computed_field_or_ancestor_satisfies_condition(
				settings->stream_vector_field, conditional_function, user_data))
		{
			return_code = 1;
		}
		/* appearance settings for all settings types */
		else if (settings->data_field &&
			Computed_field_or_ancestor_satisfies_condition(
				settings->data_field, conditional_function, user_data))
		{
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_Computed_field_or_ancestor_satisfies_condition.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_Computed_field_or_ancestor_satisfies_condition */

static int GT_element_settings_uses_changed_FE_field(
	struct GT_element_settings *settings,
	struct Computed_field *default_coordinate_field,
	struct CHANGE_LOG(FE_field) *fe_field_change_log)
/*******************************************************************************
LAST MODIFIED : 14 March 2003

DESCRIPTION :
Returns true if <settings> uses an FE_field noted as changed in the
<fe_field_change_log>. Currently, the only direct usage is the
native_discretization_field GT_ELEMENT_SETTINGS_ELEMENT_POINTS.
However, also checks whether any computed fields in use are indirectly affected
by the changed FE_fields.
==============================================================================*/
{
	enum CHANGE_LOG_CHANGE(FE_field) fe_field_change;
	int return_code;

	ENTER(GT_element_settings_uses_changed_FE_field);
	if (settings && default_coordinate_field && fe_field_change_log)
	{
		if ((GT_ELEMENT_SETTINGS_ELEMENT_POINTS == settings->settings_type) &&
			settings->native_discretization_field &&
			CHANGE_LOG_QUERY(FE_field)(fe_field_change_log,
				settings->native_discretization_field, &fe_field_change) &&
			(CHANGE_LOG_OBJECT_UNCHANGED(FE_field) != fe_field_change))
		{
			return_code = 1;
		}
		else
		{
			return_code =
				GT_element_settings_Computed_field_or_ancestor_satisfies_condition(
					settings, default_coordinate_field,
					Computed_field_contains_changed_FE_field,
					(void *)fe_field_change_log);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_uses_changed_FE_field.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_uses_changed_FE_field */

static int FE_element_as_graphics_name_is_removed_or_modified(
	int graphics_name, void *data_void)
/*******************************************************************************
LAST MODIFIED : 14 March 2003

DESCRIPTION :
Returns true if the element identified by <graphics_name> from the FE_region in
data was removed or its identifier or non-identifier contents modified.
<data_void> points at a struct GT_element_settings_FE_region_change_data.
==============================================================================*/
{
	int return_code;
	struct CM_element_information cm;
	struct FE_element *element;
	struct GT_element_settings_FE_region_change_data *data;

	ENTER(FE_element_as_graphics_name_is_removed_or_modified);
	return_code = 0;
	if (data = (struct GT_element_settings_FE_region_change_data *)data_void)
	{
		if (CM_element_information_from_graphics_name(&cm, graphics_name))
		{
			if (element = FE_region_get_FE_element_from_identifier(data->fe_region,
				&cm))
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

int GT_element_settings_FE_region_change(
	struct GT_element_settings *settings, void *data_void)
/*******************************************************************************
LAST MODIFIED : 17 February 2003

DESCRIPTION :
Tells <settings> about changes to the FE_region it is operating. If changes
affects its graphics, sets them up to be rebuilt.
<data_void> points at a struct GT_element_settings_FE_region_change_data.
==============================================================================*/
{
	int fe_field_related_object_change, return_code;
	struct GT_element_settings_FE_region_change_data *data;
	
	ENTER(GT_element_settings_FE_region_change);
	if (settings &&
		(data = (struct GT_element_settings_FE_region_change_data *)data_void))
	{
		if (settings->graphics_object)
		{
			switch (settings->settings_type)
			{
				case GT_ELEMENT_SETTINGS_DATA_POINTS:
				{
					/* only affected by data_FE_region */
				} break;
				case GT_ELEMENT_SETTINGS_NODE_POINTS:
				{
					/* must always rebuild if identifiers changed */
					if ((data->fe_node_change_summary &
						CHANGE_LOG_OBJECT_IDENTIFIER_CHANGED(FE_node)) || 
						(GT_element_settings_uses_changed_FE_field(settings,
							data->default_coordinate_field, data->fe_field_changes) && (
								(data->fe_field_change_summary & (
									CHANGE_LOG_OBJECT_IDENTIFIER_CHANGED(FE_field) |
									CHANGE_LOG_OBJECT_NOT_IDENTIFIER_CHANGED(FE_field))) ||
								((data->fe_field_change_summary &
									CHANGE_LOG_RELATED_OBJECT_CHANGED(FE_field)) &&
									(0 < data->number_of_fe_node_changes)))))
					{
						/* currently node points are always rebuilt from scratch */
						GT_object_remove_primitives_at_time(settings->graphics_object,
							data->time,
							(GT_object_primitive_object_name_conditional_function *)NULL,
							(void *)NULL);
						settings->graphics_changed = 1;
						settings->selected_graphics_changed = 1;
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
						(GT_element_settings_uses_changed_FE_field(settings,
							data->default_coordinate_field, data->fe_field_changes) && (
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
							GT_object_remove_primitives_at_time(settings->graphics_object,
								data->time, FE_element_as_graphics_name_is_removed_or_modified,
								data_void);
						}
						else
						{
							/* full rebuild for changed identifiers, FE_field definition
								 changes or many node/element field changes */
							GT_object_remove_primitives_at_time(settings->graphics_object,
								data->time,
								(GT_object_primitive_object_name_conditional_function *)NULL,
								(void *)NULL);
						}
						settings->graphics_changed = 1;
						settings->selected_graphics_changed = 1;
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
			"GT_element_settings_FE_region_change.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_FE_region_change */

int GT_element_settings_data_FE_region_change(
	struct GT_element_settings *settings, void *data_void)
/*******************************************************************************
LAST MODIFIED : 5 February 2003

DESCRIPTION :
Tells <settings> about changes to the FE_region it is operating. If changes
affects its graphics, sets them up to be rebuilt.
<data_void> points at a struct GT_element_settings_FE_region_change_data.
???RC Temporary until data_root_region/data_hack removed.
==============================================================================*/
{
	int return_code;
	struct GT_element_settings_FE_region_change_data *data;
	
	ENTER(GT_element_settings_data_FE_region_change);
	if (settings &&
		(data = (struct GT_element_settings_FE_region_change_data *)data_void))
	{
		if (settings->graphics_object)
		{
			switch (settings->settings_type)
			{
				case GT_ELEMENT_SETTINGS_DATA_POINTS:
				{
					/* rebuild on any changes */
					if ((0 < data->number_of_fe_node_changes) &&
						GT_element_settings_uses_changed_FE_field(settings,
							data->default_coordinate_field, data->fe_field_changes))
					{
						/* currently node points are always rebuilt from scratch */
						GT_object_remove_primitives_at_time(settings->graphics_object,
							data->time,
							(GT_object_primitive_object_name_conditional_function *)NULL,
							(void *)NULL);
						settings->graphics_changed = 1;
						settings->selected_graphics_changed = 1;
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
			"GT_element_settings_data_FE_region_change.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_data_FE_region_change */

int GT_element_settings_time_change(
	struct GT_element_settings *settings,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 25 March 2003

DESCRIPTION :
Notifies the <settings> that time has changed.
==============================================================================*/
{
	int return_code;
	
	ENTER(GT_element_settings_time_change);
	USE_PARAMETER(dummy_void);
	if (settings)
	{
		return_code = 1;
		if (settings->glyph && (1 < GT_object_get_number_of_times(settings->glyph)))
		{
			GT_object_changed(settings->glyph);
		}
		if (settings->time_dependent)
		{
			GT_element_settings_clear_graphics(settings);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_time_change.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_time_change */

int GT_element_settings_glyph_change(
	struct GT_object *glyph,void *settings_void)
/*******************************************************************************
LAST MODIFIED : 26 October 2000 (None Tree Hill Day)

DESCRIPTION :
Notifies the <settings> that the glyph used has changed.
==============================================================================*/
{
	int return_code;
	
	ENTER(GT_element_settings_glyph_change);
	if (glyph && settings_void)
	{
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_glyph_change.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_glyph_change */

int GT_element_settings_uses_dimension(struct GT_element_settings *settings,
	void *dimension_void)
/*******************************************************************************
LAST MODIFIED : 20 March 2001

DESCRIPTION :
Iterator function returning true if the settings uses nodes/elements of the
given <dimension>. The special value of -1 denotes all dimensions and always
returns true.
==============================================================================*/
{
	int *dimension, return_code;

	ENTER(GT_element_settings_uses_dimension);
	if (settings&&(dimension=(int *)dimension_void))
	{
		return_code = GT_element_settings_type_uses_dimension(
			settings->settings_type, *dimension);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_uses_dimension.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_uses_dimension */

int GT_element_settings_selects_elements(struct GT_element_settings *settings)
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
Returns true if the graphics for <settings> are output with names that identify
the elements they are calculated from.
==============================================================================*/
{
	int return_code;
	
	ENTER(GT_element_settings_selects_elements);
	if (settings)
	{
		return_code=(GRAPHICS_NO_SELECT != settings->select_mode)&&(
			(GT_ELEMENT_SETTINGS_LINES==settings->settings_type)||
			(GT_ELEMENT_SETTINGS_CYLINDERS==settings->settings_type)||
			(GT_ELEMENT_SETTINGS_SURFACES==settings->settings_type)||
			(GT_ELEMENT_SETTINGS_ELEMENT_POINTS==settings->settings_type)||
			(GT_ELEMENT_SETTINGS_ISO_SURFACES==settings->settings_type)||
			(GT_ELEMENT_SETTINGS_VOLUMES==settings->settings_type));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_selects_elements.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_selects_elements */

enum GT_element_settings_type GT_element_settings_get_settings_type(
	struct GT_element_settings *settings)
/*******************************************************************************
LAST MODIFIED : 20 March 2001

DESCRIPTION :
Returns the settings type of the <settings>, eg. GT_ELEMENT_SETTINGS_LINES.
==============================================================================*/
{
	enum GT_element_settings_type settings_type;

	ENTER(GT_element_settings_get_settings_type);
	if (settings)
	{
		settings_type=settings->settings_type;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_get_settings_type.  Invalid argument(s)");
		settings_type = GT_ELEMENT_SETTINGS_LINES;
	}
	LEAVE;

	return (settings_type);
} /* GT_element_settings_get_settings_type */

int GT_element_settings_type_matches(struct GT_element_settings *settings,
	void *settings_type_void)
/*******************************************************************************
LAST MODIFIED : 2 July 1997

DESCRIPTION :
Returns 1 if the settings are of the specified settings_type.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_settings_type_matches);
	if (settings)
	{
		return_code=(settings->settings_type ==
			(enum GT_element_settings_type)settings_type_void);
	}
	else
	{
		display_message(ERROR_MESSAGE,"GT_element_settings_type_matches.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_type_matches */

int GT_element_settings_add_to_list(struct GT_element_settings *settings,
	int position,struct LIST(GT_element_settings) *list_of_settings)
/*******************************************************************************
LAST MODIFIED : 8 June 1998

DESCRIPTION :
Adds the <settings> to <list_of_settings> at the given <position>, where 1 is
the top of the list (rendered first), and values less than 1 or greater than the
last position in the list cause the settings to be added at its end, with a
position one greater than the last.
==============================================================================*/
{
	int last_position,return_code;
	struct GT_element_settings *settings_in_way;

	ENTER(GT_element_settings_add_to_list);
	if (settings&&list_of_settings&&
		!IS_OBJECT_IN_LIST(GT_element_settings)(settings,list_of_settings))
	{
		return_code=1;
		last_position=NUMBER_IN_LIST(GT_element_settings)(list_of_settings);
		if ((1>position)||(position>last_position))
		{
			/* add to end of list */
			position=last_position+1;
		}
		ACCESS(GT_element_settings)(settings);
		while (return_code&&settings)
		{
			settings->position=position;
			/* is there already a settings with that position? */
			if (settings_in_way=FIND_BY_IDENTIFIER_IN_LIST(GT_element_settings,
				position)(position,list_of_settings))
			{
				/* remove the old settings to make way for the new */
				ACCESS(GT_element_settings)(settings_in_way);
				REMOVE_OBJECT_FROM_LIST(GT_element_settings)(
					settings_in_way,list_of_settings);
			}
			if (ADD_OBJECT_TO_LIST(GT_element_settings)(settings,list_of_settings))
			{
				DEACCESS(GT_element_settings)(&settings);
				/* the old, in-the-way settings now become the new settings */
				settings=settings_in_way;
				position++;
			}
			else
			{
				DEACCESS(GT_element_settings)(&settings);
				if (settings_in_way)
				{
					DEACCESS(GT_element_settings)(&settings_in_way);
				}
				display_message(ERROR_MESSAGE,"GT_element_settings_add_to_list.  "
					"Could not add settings - settings lost");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_add_to_list.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_add_to_list */

int GT_element_settings_remove_from_list(struct GT_element_settings *settings,
	struct LIST(GT_element_settings) *list_of_settings)
/*******************************************************************************
LAST MODIFIED : 16 June 1998

DESCRIPTION :
Removes the <settings> from <list_of_settings> and decrements the position
of all subsequent settings.
==============================================================================*/
{
	int return_code,next_position;

	ENTER(GT_element_settings_remove_from_list);
	if (settings&&list_of_settings)
	{
		if (IS_OBJECT_IN_LIST(GT_element_settings)(settings,list_of_settings))
		{
			next_position=settings->position+1;
			return_code=REMOVE_OBJECT_FROM_LIST(GT_element_settings)(
				settings,list_of_settings);
			/* decrement position of all remaining settings */
			while (return_code&&(settings=FIND_BY_IDENTIFIER_IN_LIST(
				GT_element_settings,position)(next_position,list_of_settings)))
			{
				ACCESS(GT_element_settings)(settings);
				REMOVE_OBJECT_FROM_LIST(GT_element_settings)(settings,list_of_settings);
				(settings->position)--;
				if (ADD_OBJECT_TO_LIST(GT_element_settings)(settings,list_of_settings))
				{
					next_position++;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"GT_element_settings_remove_from_list.  "
						"Could not readjust positions - settings lost");
					return_code=0;
				}
				DEACCESS(GT_element_settings)(&settings);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"GT_element_settings_remove_from_list.  Settings not in list");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_remove_from_list.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_remove_from_list */

int GT_element_settings_modify_in_list(struct GT_element_settings *settings,
	struct GT_element_settings *new_settings,
	struct LIST(GT_element_settings) *list_of_settings)
/*******************************************************************************
LAST MODIFIED : 16 June 1998

DESCRIPTION :
Changes the contents of <settings> to match <new_settings>, with no change in
position in <list_of_settings>.
==============================================================================*/
{
	int return_code,old_position;

	ENTER(GT_element_settings_modify_in_list);
	if (settings&&new_settings&&list_of_settings)
	{
		if (IS_OBJECT_IN_LIST(GT_element_settings)(settings,list_of_settings))
		{
			/* save the current position */
			old_position=settings->position;
			return_code=GT_element_settings_copy_without_graphics_object(settings,new_settings);
			settings->position=old_position;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"GT_element_settings_modify_in_list.  Settings not in list");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_modify_in_list.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_modify_in_list */

int GT_element_settings_get_position_in_list(
	struct GT_element_settings *settings,
	struct LIST(GT_element_settings) *list_of_settings)
/*******************************************************************************
LAST MODIFIED : 9 June 1998

DESCRIPTION :
Returns the position of <settings> in <list_of_settings>.
==============================================================================*/
{
	int position;

	ENTER(GT_element_settings_get_position_in_list);
	if (settings&&list_of_settings)
	{
		if (IS_OBJECT_IN_LIST(GT_element_settings)(settings,list_of_settings))
		{
			position=settings->position;
		}
		else
		{
			position=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_get_position_in_list.  Invalid argument(s)");
		position=0;
	}
	LEAVE;

	return (position);
} /* GT_element_settings_get_position_in_list */

int GT_element_settings_same_geometry(struct GT_element_settings *settings,
	void *second_settings_void)
/*******************************************************************************
LAST MODIFIED : 10 November 2000

DESCRIPTION :
GT_element_settings list conditional function returning 1 iff the two
settings describe EXACTLY the same geometry.
==============================================================================*/
{
	int dimension,i,return_code;
	struct GT_element_settings *second_settings;

	ENTER(GT_element_settings_same_geometry);
	if (settings
		&&(second_settings=(struct GT_element_settings *)second_settings_void))
	{
		return_code=1;

		/* compare geometry settings */
		/* for all graphic types */
		if (return_code)
		{
			/* note: different if names are different */
			return_code=
				(settings->settings_type==second_settings->settings_type)&&
				(settings->coordinate_field==second_settings->coordinate_field)&&
				((((char *)NULL==settings->name)&&((char *)NULL==second_settings->name))
					||((settings->name)&&(second_settings->name)&&
						(0==strcmp(settings->name,second_settings->name))))&&
				(settings->select_mode==second_settings->select_mode);
		}
		/* for 1-D and 2-D elements only */
		if (return_code)
		{
			dimension=GT_element_settings_get_dimension(settings);
			if ((1==dimension)||(2==dimension))
			{
				return_code=(settings->exterior == second_settings->exterior)&&
					(settings->face == second_settings->face);
			}
		}
		/* for cylinders only */
		if (return_code&&(GT_ELEMENT_SETTINGS_CYLINDERS==settings->settings_type))
		{
			return_code=(settings->constant_radius==second_settings->constant_radius)
				&&(settings->radius_scalar_field==second_settings->radius_scalar_field)
				&&(settings->radius_scale_factor==second_settings->radius_scale_factor);
		}
		/* for iso_surfaces only */
		if (return_code&&
			(GT_ELEMENT_SETTINGS_ISO_SURFACES==settings->settings_type))
		{
			return_code=(settings->number_of_iso_values==
				second_settings->number_of_iso_values)&&
				(settings->decimation_threshold==second_settings->decimation_threshold)&&
				(settings->iso_scalar_field==second_settings->iso_scalar_field);
			i = 0;
			while (return_code && (i < settings->number_of_iso_values))
			{
				if (settings->iso_values[i] != second_settings->iso_values[i])
				{
					return_code = 0;
				}
				i++;
			}
		}
		/* for node_points, data_points and element_points only */
		if (return_code&&
			((GT_ELEMENT_SETTINGS_NODE_POINTS==settings->settings_type)||
				(GT_ELEMENT_SETTINGS_DATA_POINTS==settings->settings_type)||
				(GT_ELEMENT_SETTINGS_ELEMENT_POINTS==settings->settings_type)))
		{
			return_code=
				(settings->glyph==second_settings->glyph)&&
				(settings->glyph_scaling_mode==second_settings->glyph_scaling_mode)&&
				(settings->glyph_size[0]==second_settings->glyph_size[0])&&
				(settings->glyph_size[1]==second_settings->glyph_size[1])&&
				(settings->glyph_size[2]==second_settings->glyph_size[2])&&
				(settings->glyph_scale_factors[0]==
					second_settings->glyph_scale_factors[0])&&
				(settings->glyph_scale_factors[1]==
					second_settings->glyph_scale_factors[1])&&
				(settings->glyph_scale_factors[2]==
					second_settings->glyph_scale_factors[2])&&
				(settings->glyph_centre[0]==second_settings->glyph_centre[0])&&
				(settings->glyph_centre[1]==second_settings->glyph_centre[1])&&
				(settings->glyph_centre[2]==second_settings->glyph_centre[2])&&
				(settings->orientation_scale_field==
					second_settings->orientation_scale_field)&&
				(settings->variable_scale_field==
					second_settings->variable_scale_field)&&
				(settings->label_field==second_settings->label_field);
		}
		/* for element_points and iso_surfaces */
		if (return_code&&
			((GT_ELEMENT_SETTINGS_ELEMENT_POINTS==settings->settings_type)||
				(GT_ELEMENT_SETTINGS_ISO_SURFACES==settings->settings_type)))
		{
			return_code=
				(settings->use_element_type==second_settings->use_element_type);
		}
		/* for element_points only */
		if (return_code&&
			(GT_ELEMENT_SETTINGS_ELEMENT_POINTS==settings->settings_type))
		{
			return_code=
				(settings->xi_discretization_mode==
					second_settings->xi_discretization_mode)&&
				(settings->xi_point_density_field==
					second_settings->xi_point_density_field)&&
				(settings->native_discretization_field==
					second_settings->native_discretization_field)&&
				(settings->discretization.number_in_xi1==
					second_settings->discretization.number_in_xi1)&&
				(settings->discretization.number_in_xi2==
					second_settings->discretization.number_in_xi2)&&
				(settings->discretization.number_in_xi3==
					second_settings->discretization.number_in_xi3);
		}
		/* for volumes only */
		if (return_code&&(GT_ELEMENT_SETTINGS_VOLUMES==settings->settings_type))
		{
			return_code=
				(settings->volume_texture==second_settings->volume_texture)&&
				(settings->displacement_map_field==
					second_settings->displacement_map_field)&&
				(settings->displacement_map_xi_direction==
					second_settings->displacement_map_xi_direction);
		}
		/* for settings starting in a particular element */
		if (return_code&&((GT_ELEMENT_SETTINGS_VOLUMES==settings->settings_type)||
			(GT_ELEMENT_SETTINGS_STREAMLINES==settings->settings_type)))
		{
			return_code=
				(settings->seed_element==second_settings->seed_element);
		}
		/* for settings requiring an exact xi location */
		if (return_code&&(
			(GT_ELEMENT_SETTINGS_ELEMENT_POINTS==settings->settings_type)||
			(GT_ELEMENT_SETTINGS_STREAMLINES==settings->settings_type)))
		{
			return_code=
				(settings->seed_xi[0]==second_settings->seed_xi[0])&&
				(settings->seed_xi[1]==second_settings->seed_xi[1])&&
				(settings->seed_xi[2]==second_settings->seed_xi[2]);
		}
		/* for streamlines only */
		if (return_code&&(GT_ELEMENT_SETTINGS_STREAMLINES==settings->settings_type))
		{
			return_code=
				(settings->streamline_type==second_settings->streamline_type)&&
				(settings->stream_vector_field==second_settings->stream_vector_field)&&
				(settings->reverse_track==second_settings->reverse_track)&&
				(settings->streamline_length==second_settings->streamline_length)&&
				(settings->streamline_width==second_settings->streamline_width)&&
				(settings->seed_node_region==second_settings->seed_node_region)&&
				(settings->seed_node_coordinate_field==second_settings->seed_node_coordinate_field);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_same_geometry.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_same_geometry */

int GT_element_settings_same_name_or_geometry(struct GT_element_settings *settings,
	void *second_settings_void)
/*******************************************************************************
LAST MODIFIED : 5 November 2002

DESCRIPTION :
GT_element_settings list conditional function returning 1 iff the two
settings have the same name or describe EXACTLY the same geometry.
==============================================================================*/
{
	int return_code;
	struct GT_element_settings *second_settings;

	ENTER(GT_element_settings_same_name_or_geometry);
	if (settings
		&&(second_settings=(struct GT_element_settings *)second_settings_void))
	{
		if (settings->name && second_settings->name)
		{
			return_code = (0==strcmp(settings->name,second_settings->name));
		}
		else
		{
			return_code = GT_element_settings_same_geometry(settings,
				second_settings_void);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_same_name_or_geometry.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_same_name_or_geometry */

int GT_element_settings_same_non_trivial(struct GT_element_settings *settings,
	void *second_settings_void)
/*******************************************************************************
LAST MODIFIED : 19 March 1999

DESCRIPTION :
GT_element_settings list conditional function returning 1 iff the two
settings have the same geometry and the same nontrivial appearance
characteristics. Trivial appearance characteristics are the material,
visibility and spectrum.
==============================================================================*/
{
	int return_code;
	struct GT_element_settings *second_settings;

	ENTER(GT_element_settings_same_non_trivial);
	if (settings
		&&(second_settings=(struct GT_element_settings *)second_settings_void))
	{
		return_code=
			GT_element_settings_same_geometry(settings,second_settings_void)&&
			(settings->data_field==second_settings->data_field)&&
			(settings->render_type==second_settings->render_type)&&
			(settings->line_width==second_settings->line_width)&&
			(settings->texture_coordinate_field==second_settings->texture_coordinate_field)&&
			((GT_ELEMENT_SETTINGS_STREAMLINES != settings->settings_type)||
				(settings->streamline_data_type==
					second_settings->streamline_data_type));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_same_non_trivial.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_same_non_trivial */

int GT_element_settings_same_non_trivial_with_graphics_object(
	struct GT_element_settings *settings,void *second_settings_void)
/*******************************************************************************
LAST MODIFIED : 11 June 1998

DESCRIPTION :
Same as GT_element_settings_same_non_trivial except <settings> must also have
a graphics_object. Used for getting graphics objects from previous settings
that are the same except for trivial differences such as the material and
spectrum which can be changed in the graphics object to match the new settings .
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_settings_same_non_trivial_with_graphics_object);
	if (settings)
	{
		return_code=settings->graphics_object&&
			GT_element_settings_same_non_trivial(settings,second_settings_void);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_same_non_trivial_with_graphics_object.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_same_non_trivial_with_graphics_object */

int GT_element_settings_match(struct GT_element_settings *settings1,
	struct GT_element_settings *settings2)
/*******************************************************************************
LAST MODIFIED : 14 November 2001

DESCRIPTION :
Returns true if <settings1> and <settings2> would product identical graphics.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_settings_match);
	if (settings1 && settings2)
	{
		return_code=
			GT_element_settings_same_geometry(settings1, (void *)settings2) &&
			(settings1->visibility == settings2->visibility) &&
			(settings1->material == settings2->material) &&
			(settings1->secondary_material == settings2->secondary_material) &&
			(settings1->line_width == settings2->line_width) &&
			(settings1->selected_material == settings2->selected_material) &&
			(settings1->data_field == settings2->data_field) &&
			(settings1->spectrum == settings2->spectrum) &&
			(settings1->font == settings2->font) &&
			(settings1->render_type == settings2->render_type) &&
			(settings1->texture_coordinate_field ==
				settings2->texture_coordinate_field) &&
			((GT_ELEMENT_SETTINGS_STREAMLINES != settings1->settings_type) ||
				(settings1->streamline_data_type ==
					settings2->streamline_data_type));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_match.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_match */

int GT_element_settings_extract_graphics_object_from_list(
	struct GT_element_settings *settings,void *list_of_settings_void)
/*******************************************************************************
LAST MODIFIED : 2 April 2001

DESCRIPTION :
If <settings> does not already have a graphics object, this function attempts
to find settings in <list_of_settings> which differ only trivially in material,
spectrum etc. AND have a graphics object. If such a settings is found, the
graphics_object is moved from the matching settings and put in <settings>, while
any trivial differences are fixed up in the graphics_obejct.
==============================================================================*/
{
	int return_code;
	struct GT_element_settings *matching_settings;
	struct LIST(GT_element_settings) *list_of_settings;

	ENTER(GT_element_settings_extract_graphics_object_from_list);
	if (settings&&(list_of_settings=
		(struct LIST(GT_element_settings) *)list_of_settings_void))
	{
		return_code = 1;
		if (!(settings->graphics_object))
		{
			if (matching_settings = FIRST_OBJECT_IN_LIST_THAT(GT_element_settings)(
				GT_element_settings_same_non_trivial_with_graphics_object,
				(void *)settings,list_of_settings))
			{
				/* make sure graphics_changed and selected_graphics_changed flags
					 are brought across */
				settings->graphics_object = matching_settings->graphics_object;
				settings->graphics_changed = matching_settings->graphics_changed;
				settings->selected_graphics_changed =
					matching_settings->selected_graphics_changed;
				/* reset graphics_object and flags in matching_settings */
				matching_settings->graphics_object = (struct GT_object *)NULL;
				matching_settings->graphics_changed = 1;
				matching_settings->selected_graphics_changed = 1;
				/* make sure settings and graphics object have same material and
					 spectrum */
				if (settings->material != matching_settings->material)
				{
					set_GT_object_default_material(settings->graphics_object,
						settings->material);
				}
				if (settings->secondary_material !=
					matching_settings->secondary_material)
				{
					set_GT_object_secondary_material(settings->graphics_object,
						settings->secondary_material);
				}
				if (settings->selected_material !=
					matching_settings->selected_material)
				{
					set_GT_object_selected_material(settings->graphics_object,
						settings->selected_material);
				}
				if (settings->data_field && settings->spectrum)
				{
					if (settings->spectrum != get_GT_object_spectrum(settings->graphics_object))
					{
						set_GT_object_Spectrum(settings->graphics_object,
							(void *)settings->spectrum);
					}
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_extract_graphics_object_from_list.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_extract_graphics_object_from_list */

char *GT_element_settings_string(struct GT_element_settings *settings,
	enum GT_element_settings_string_details settings_detail)
/*******************************************************************************
LAST MODIFIED : 14 March 2003

DESCRIPTION :
Returns a string describing the settings, suitable for entry into the command
line. Parameter <settings_detail> selects whether appearance settings are
included in the string. User must remember to DEALLOCATE the name afterwards.
???RC When we have editing of settings, rather than creating from scratch each
time as we do now with our text commands, we must ensure that defaults are
restored by commands generated by this string, eg. must have coordinate NONE
if no coordinate field. Currently only write if we have a field.
==============================================================================*/
{
	char *settings_string,temp_string[100],*name;
	int dimension,error,i;

	ENTER(GT_element_settings_string);
	settings_string=(char *)NULL;
	if (settings&&(
		(SETTINGS_STRING_GEOMETRY==settings_detail)||
		(SETTINGS_STRING_COMPLETE==settings_detail)||
		(SETTINGS_STRING_COMPLETE_PLUS==settings_detail)))
	{
		error=0;
		if (SETTINGS_STRING_COMPLETE_PLUS==settings_detail)
		{
			if (settings->name)
			{
				sprintf(temp_string,"%i. (%s) ",settings->position, settings->name);
			}
			else
			{
				sprintf(temp_string,"%i. ",settings->position);
			}
			append_string(&settings_string,temp_string,&error);
		}

		/* show geometry settings */
		/* for all graphic types */
		/* write settings type = "points", "lines" etc. */
		append_string(&settings_string,
			ENUMERATOR_STRING(GT_element_settings_type)(settings->settings_type),
			&error);
		if (settings->name)
		{
			sprintf(temp_string," as %s", settings->name);
			append_string(&settings_string,temp_string,&error);
		}
		if (settings->coordinate_field)
		{
			append_string(&settings_string," coordinate ",&error);
			name=(char *)NULL;
			if (GET_NAME(Computed_field)(settings->coordinate_field,&name))
			{
				/* put quotes around name if it contains special characters */
				make_valid_token(&name);
				append_string(&settings_string,name,&error);
				DEALLOCATE(name);
			}
			else
			{
				append_string(&settings_string,"NONE",&error);
			}
		}
		/* for 1-D and 2-D elements only */
		dimension=GT_element_settings_get_dimension(settings);
		if ((1==dimension)||(2==dimension))
		{
			if (settings->exterior)
			{
				append_string(&settings_string," exterior",&error);
			}
			if (-1 != settings->face)
			{
				append_string(&settings_string," face",&error);
				switch (settings->face)
				{
					case 0:
					{
						append_string(&settings_string," xi1_0",&error);
					} break;
					case 1:
					{
						append_string(&settings_string," xi1_1",&error);
					} break;
					case 2:
					{
						append_string(&settings_string," xi2_0",&error);
					} break;
					case 3:
					{
						append_string(&settings_string," xi2_1",&error);
					} break;
					case 4:
					{
						append_string(&settings_string," xi3_0",&error);
					} break;
					case 5:
					{
						append_string(&settings_string," xi3_1",&error);
					} break;
					default:
					{
						display_message(ERROR_MESSAGE,
							"GT_element_settings_string.  Invalid face number");
						DEALLOCATE(settings_string);
						error=1;
					} break;
				}
			}
		}
		/* for element_points and iso_surfaces */
		if (GT_ELEMENT_SETTINGS_LINES==settings->settings_type)
		{
			if (0 != settings->line_width)
			{
				sprintf(temp_string, " line_width %d", settings->line_width);
				append_string(&settings_string,temp_string,&error);
			}
		}
		/* for cylinders only */
		if (GT_ELEMENT_SETTINGS_CYLINDERS==settings->settings_type)
		{
			if (0.0 != settings->constant_radius)
			{
				sprintf(temp_string," constant_radius %g",settings->constant_radius);
				append_string(&settings_string,temp_string,&error);
			}
			if (settings->radius_scalar_field)
			{
				if (GET_NAME(Computed_field)(settings->radius_scalar_field,&name))
				{
					/* put quotes around name if it contains special characters */
					make_valid_token(&name);
					append_string(&settings_string," radius_scalar ",&error);
					append_string(&settings_string,name,&error);
					DEALLOCATE(name);
					if (1.0 != settings->radius_scale_factor)
					{
						sprintf(temp_string," scale_factor %g",
							settings->radius_scale_factor);
						append_string(&settings_string,temp_string,&error);
					}
				}
				else
				{
					DEALLOCATE(settings_string);
					error=1;
				}
			}
		}
		/* for iso_surfaces only */
		if (GT_ELEMENT_SETTINGS_ISO_SURFACES==settings->settings_type)
		{
			if (GET_NAME(Computed_field)(settings->iso_scalar_field,&name))
			{
				/* put quotes around name if it contains special characters */
				make_valid_token(&name);
				append_string(&settings_string," iso_scalar ",&error);
				append_string(&settings_string,name,&error);
				DEALLOCATE(name);
			}
			else
			{
				DEALLOCATE(settings_string);
				error=1;
			}
			sprintf(temp_string," iso_values");
			append_string(&settings_string,temp_string,&error);
			for (i = 0 ; i < settings->number_of_iso_values ; i++)
			{
				sprintf(temp_string, " %g", settings->iso_values[i]);
				append_string(&settings_string,temp_string,&error);				
			}
			if (settings->decimation_threshold > 0.0)
			{
				sprintf(temp_string," decimation_threshold %g",
					settings->decimation_threshold);
				append_string(&settings_string,temp_string,&error);
			}
		}
		/* for node_points, data_points and element_points only */
		if ((GT_ELEMENT_SETTINGS_NODE_POINTS==settings->settings_type)||
			(GT_ELEMENT_SETTINGS_DATA_POINTS==settings->settings_type)||
			(GT_ELEMENT_SETTINGS_ELEMENT_POINTS==settings->settings_type))
		{
			if (settings->glyph)
			{
				append_string(&settings_string," glyph ",&error);
				if (GET_NAME(GT_object)(settings->glyph, &name))
				{
					append_string(&settings_string,name,&error);
					DEALLOCATE(name);
				}
				append_string(&settings_string," ",&error);
				append_string(&settings_string,
					ENUMERATOR_STRING(Glyph_scaling_mode)(settings->glyph_scaling_mode),
					&error);
				sprintf(temp_string," size \"%g*%g*%g\"",settings->glyph_size[0],
					settings->glyph_size[1],settings->glyph_size[2]);
				append_string(&settings_string,temp_string,&error);
				sprintf(temp_string," centre %g,%g,%g",settings->glyph_centre[0],
					settings->glyph_centre[1],settings->glyph_centre[2]);
				append_string(&settings_string,temp_string,&error);
				if (settings->font)
				{
					append_string(&settings_string," font ",&error);
					if (GET_NAME(Graphics_font)(settings->font, &name))
					{
						append_string(&settings_string,name,&error);
						DEALLOCATE(name);
					}
				}
				if (settings->label_field)
				{
					if (GET_NAME(Computed_field)(settings->label_field,&name))
					{
						/* put quotes around name if it contains special characters */
						make_valid_token(&name);
						append_string(&settings_string," label ",&error);
						append_string(&settings_string,name,&error);
						DEALLOCATE(name);
					}
				}
				if (settings->orientation_scale_field)
				{
					if (GET_NAME(Computed_field)(settings->orientation_scale_field,&name))
					{
						/* put quotes around name if it contains special characters */
						make_valid_token(&name);
						append_string(&settings_string," orientation ",&error);
						append_string(&settings_string,name,&error);
						DEALLOCATE(name);
					}
					else
					{
						DEALLOCATE(settings_string);
						error=1;
					}
				}
				if (settings->variable_scale_field)
				{
					if (GET_NAME(Computed_field)(settings->variable_scale_field,&name))
					{
						/* put quotes around name if it contains special characters */
						make_valid_token(&name);
						append_string(&settings_string," variable_scale ",&error);
						append_string(&settings_string,name,&error);
						DEALLOCATE(name);
					}
					else
					{
						DEALLOCATE(settings_string);
						error=1;
					}
				}
				if (settings->orientation_scale_field || settings->variable_scale_field)
				{
					sprintf(temp_string," scale_factors \"%g*%g*%g\"",
						settings->glyph_scale_factors[0],
						settings->glyph_scale_factors[1],
						settings->glyph_scale_factors[2]);
					append_string(&settings_string,temp_string,&error);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"GT_element_settings_string.  Settings missing glyph");
				DEALLOCATE(settings_string);
				error=1;
			}
		}
		/* for element_points and iso_surfaces */
		if ((GT_ELEMENT_SETTINGS_ELEMENT_POINTS==settings->settings_type)||
			(GT_ELEMENT_SETTINGS_ISO_SURFACES==settings->settings_type))
		{
			sprintf(temp_string, " %s",
				ENUMERATOR_STRING(Use_element_type)(settings->use_element_type));
			append_string(&settings_string,temp_string,&error);
		}
		/* for element_points only */
		if (GT_ELEMENT_SETTINGS_ELEMENT_POINTS==settings->settings_type)
		{
			append_string(&settings_string," ",&error);
			append_string(&settings_string, ENUMERATOR_STRING(Xi_discretization_mode)(
				settings->xi_discretization_mode), &error);
			if (XI_DISCRETIZATION_EXACT_XI != settings->xi_discretization_mode)
			{
				if ((XI_DISCRETIZATION_CELL_DENSITY ==
					settings->xi_discretization_mode) ||
					(XI_DISCRETIZATION_CELL_POISSON == settings->xi_discretization_mode))
				{
					append_string(&settings_string, " density ", &error);
					if (GET_NAME(Computed_field)(settings->xi_point_density_field,&name))
					{
						/* put quotes around name if it contains special characters */
						make_valid_token(&name);
						append_string(&settings_string, name, &error);
						DEALLOCATE(name);
					}
					else
					{
						DEALLOCATE(settings_string);
						error = 1;
					}
				}
				sprintf(temp_string,
					" discretization \"%d*%d*%d\" native_discretization ",
					settings->discretization.number_in_xi1,
					settings->discretization.number_in_xi2,
					settings->discretization.number_in_xi3);
				append_string(&settings_string,temp_string,&error);
				if (settings->native_discretization_field)
				{
					if (GET_NAME(FE_field)(settings->native_discretization_field,&name))
					{
						/* put quotes around name if it contains special characters */
						make_valid_token(&name);
						append_string(&settings_string,name,&error);
						DEALLOCATE(name);
					}
					else
					{
						DEALLOCATE(settings_string);
						error=1;
					}
				}
				else
				{
					append_string(&settings_string,"NONE",&error);
				}
			}
		}
		/* for volumes only */
		if (GT_ELEMENT_SETTINGS_VOLUMES==settings->settings_type)
		{
			if (settings->volume_texture)
			{
				append_string(&settings_string," vtexture ",&error);
				append_string(&settings_string,settings->volume_texture->name,
					&error);
			}
			if (settings->displacement_map_field)
			{
				if (GET_NAME(Computed_field)(settings->displacement_map_field,&name))
				{
					/* put quotes around name if it contains special characters */
					make_valid_token(&name);
					append_string(&settings_string," displacement_map_field ",&error);
					append_string(&settings_string,name,&error);
					DEALLOCATE(name);
				}
				else
				{
					DEALLOCATE(settings_string);
					error=1;
				}
				sprintf(temp_string," displacment_map_xi_direction %d",
					settings->displacement_map_xi_direction);
				append_string(&settings_string,temp_string,&error);
			}
		}
		/* for settings starting in a particular element */
		if ((GT_ELEMENT_SETTINGS_VOLUMES==settings->settings_type)||
			(GT_ELEMENT_SETTINGS_STREAMLINES==settings->settings_type))
		{
			if (settings->seed_element)
			{
				sprintf(temp_string, " seed_element %d",
					FE_element_get_cm_number(settings->seed_element));
				append_string(&settings_string, temp_string, &error);
			}
		}

		/* for settings requiring an exact xi location */
		if (((GT_ELEMENT_SETTINGS_ELEMENT_POINTS==settings->settings_type)&&
			(XI_DISCRETIZATION_EXACT_XI == settings->xi_discretization_mode))||
			(GT_ELEMENT_SETTINGS_STREAMLINES==settings->settings_type))
		{
			sprintf(temp_string," xi %g,%g,%g",
				settings->seed_xi[0],settings->seed_xi[1],settings->seed_xi[2]);
			append_string(&settings_string,temp_string,&error);
		}

		/* for streamlines only */
		if (GT_ELEMENT_SETTINGS_STREAMLINES==settings->settings_type)
		{
			append_string(&settings_string," ",&error);
			append_string(&settings_string,
				ENUMERATOR_STRING(Streamline_type)(settings->streamline_type),&error);
			if (GET_NAME(Computed_field)(settings->stream_vector_field,&name))
			{
				/* put quotes around name if it contains special characters */
				make_valid_token(&name);
				append_string(&settings_string," vector ",&error);
				append_string(&settings_string,name,&error);
				DEALLOCATE(name);
			}
			else
			{
				DEALLOCATE(settings_string);
				error=1;
			}
			if (settings->reverse_track)
			{
				append_string(&settings_string," reverse_track ",&error);
			}
			sprintf(temp_string," length %g width %g ",
				settings->streamline_length,settings->streamline_width);
			append_string(&settings_string,temp_string,&error);
			append_string(&settings_string,
				ENUMERATOR_STRING(Streamline_data_type)(settings->streamline_data_type),&error);
			if (settings->seed_node_region_path)
			{
				sprintf(temp_string," seed_node_region %s", settings->seed_node_region_path);
				append_string(&settings_string,temp_string,&error);
			}
			if (settings->seed_node_coordinate_field)
			{
				if (GET_NAME(Computed_field)(settings->seed_node_coordinate_field,&name))
				{
					/* put quotes around name if it contains special characters */
					make_valid_token(&name);
					append_string(&settings_string," seed_node_coordinate_field ",&error);
					append_string(&settings_string,name,&error);
					DEALLOCATE(name);
				}
				else
				{
					DEALLOCATE(settings_string);
					error=1;
				}
			}
		}
		append_string(&settings_string," ",&error);
		append_string(&settings_string,
			ENUMERATOR_STRING(Graphics_select_mode)(settings->select_mode),&error);

		if ((SETTINGS_STRING_COMPLETE==settings_detail)||
			(SETTINGS_STRING_COMPLETE_PLUS==settings_detail))
		{
			/* show appearance settings */
			/* for all graphic types */
			if (!settings->visibility)
			{
				append_string(&settings_string," invisible",&error);
			}
			if (settings->material&&
				GET_NAME(Graphical_material)(settings->material,&name))
			{
				/* put quotes around name if it contains special characters */
				make_valid_token(&name);
				append_string(&settings_string," material ",&error);
				append_string(&settings_string,name,&error);
				DEALLOCATE(name);
			}
			if (settings->secondary_material&&
				GET_NAME(Graphical_material)(settings->secondary_material,&name))
			{
				/* put quotes around name if it contains special characters */
				make_valid_token(&name);
				append_string(&settings_string," secondary_material ",&error);
				append_string(&settings_string,name,&error);
				DEALLOCATE(name);
			}
			if (settings->texture_coordinate_field)
			{
				if (GET_NAME(Computed_field)(settings->texture_coordinate_field,&name))
				{
					/* put quotes around name if it contains special characters */
					make_valid_token(&name);
					append_string(&settings_string," texture_coordinates ",&error);
					append_string(&settings_string,name,&error);
					DEALLOCATE(name);
				}
				else
				{
					DEALLOCATE(settings_string);
					error=1;
				}
			}
			if (settings->data_field)
			{
				if (GET_NAME(Computed_field)(settings->data_field,&name))
				{
					/* put quotes around name if it contains special characters */
					make_valid_token(&name);
					append_string(&settings_string," data ",&error);
					append_string(&settings_string,name,&error);
					DEALLOCATE(name);
				}
				else
				{
					DEALLOCATE(settings_string);
					error=1;
				}
				if (settings->spectrum&&
					GET_NAME(Spectrum)(settings->spectrum,&name))
				{
					/* put quotes around name if it contains special characters */
					make_valid_token(&name);
					append_string(&settings_string," spectrum ",&error);
					append_string(&settings_string,name,&error);
					DEALLOCATE(name);
				}
			}
			if (settings->selected_material&&
				GET_NAME(Graphical_material)(settings->selected_material,&name))
			{
				/* put quotes around name if it contains special characters */
				make_valid_token(&name);
				append_string(&settings_string," selected_material ",&error);
				append_string(&settings_string,name,&error);
				DEALLOCATE(name);
			}
			/* for surfaces and volumes */
			if ((GT_ELEMENT_SETTINGS_SURFACES==settings->settings_type) 
				|| (GT_ELEMENT_SETTINGS_VOLUMES==settings->settings_type)
				|| (GT_ELEMENT_SETTINGS_ISO_SURFACES==settings->settings_type))
			{
				append_string(&settings_string," ",&error);
				append_string(&settings_string,
					ENUMERATOR_STRING(Render_type)(settings->render_type),&error);
			}
		}
		if (error)
		{
			display_message(ERROR_MESSAGE,
				"GT_element_settings_string.  Error creating string");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_string.  Invalid argument(s)");
	}
	LEAVE;

	return settings_string;
} /* GT_element_settings_string */

int GT_element_settings_list_contents(struct GT_element_settings *settings,
	void *list_data_void)
/*******************************************************************************
LAST MODIFIED : 22 July 1998

DESCRIPTION :
Writes out the <settings> as a text string in the command window with the
<settings_string_detail>, <line_prefix> and <line_suffix> given in the
<list_data>.
==============================================================================*/
{
	int return_code;
	char *settings_string,line[40];
	struct GT_element_settings_list_data *list_data;

	ENTER(GT_element_settings_list_contents);
	if (settings&&
		(list_data=(struct GT_element_settings_list_data *)list_data_void))
	{
		if (settings_string=GT_element_settings_string(settings,
			list_data->settings_string_detail))
		{
			if (list_data->line_prefix)
			{
				display_message(INFORMATION_MESSAGE,list_data->line_prefix);
			}
			display_message(INFORMATION_MESSAGE,settings_string);
			if (list_data->line_suffix)
			{
				display_message(INFORMATION_MESSAGE,list_data->line_suffix);
			}
			/*???RC temp */
			if ((SETTINGS_STRING_COMPLETE_PLUS==list_data->settings_string_detail)&&
				(settings->access_count != 1))
			{
				sprintf(line," (access count = %i)",settings->access_count);
				display_message(INFORMATION_MESSAGE,line);
			}
			display_message(INFORMATION_MESSAGE,"\n");
			DEALLOCATE(settings_string);
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
			"GT_element_settings_list_contents.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_list_contents */

int GT_element_settings_copy_and_put_in_list(
	struct GT_element_settings *settings,void *list_of_settings_void)
/*******************************************************************************
LAST MODIFIED : 10 June 1998

DESCRIPTION :
GT_element_settings iterator function for copying a list_of_settings.
Makes a copy of the settings and puts it in the list_of_settings.
==============================================================================*/
{
	int return_code;
	struct GT_element_settings *copy_settings;
	struct LIST(GT_element_settings) *list_of_settings;

	ENTER(GT_element_settings_copy_and_put_in_list);
	if (settings&&(list_of_settings=
		(struct LIST(GT_element_settings) *)list_of_settings_void))
	{
		/* create new settings to take the copy */
		if (copy_settings=CREATE(GT_element_settings)(settings->settings_type))
		{
			/* copy and insert in list */
			if (!(return_code=GT_element_settings_copy_without_graphics_object(
				copy_settings,settings)&&
				ADD_OBJECT_TO_LIST(GT_element_settings)(copy_settings,
					list_of_settings)))
			{
				DESTROY(GT_element_settings)(&copy_settings);
				display_message(ERROR_MESSAGE,
					"GT_element_settings_copy_and_put_in_list.  "
					"Could not put copy in list");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"GT_element_settings_copy_and_put_in_list.  Could not create copy");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_copy_and_put_in_list.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_copy_and_put_in_list */

static int FE_element_to_graphics_object(struct FE_element *element,
	void *settings_to_object_data_void)
/*******************************************************************************
LAST MODIFIED : 14 March 2003

DESCRIPTION :
Converts a finite element into a graphics object with the supplied settings.
==============================================================================*/
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
	struct GT_element_settings *settings;
	struct GT_element_settings_to_graphics_object_data *settings_to_object_data;
	struct GT_glyph_set *glyph_set;
	struct GT_polyline *polyline;
	struct GT_surface *surface;
	struct GT_voltex *voltex;
	struct Multi_range *ranges;
	Triple *xi_points;

	ENTER(FE_element_to_graphics_object);
	if (element && (settings_to_object_data =
		(struct GT_element_settings_to_graphics_object_data *)
		settings_to_object_data_void) &&
		(settings = settings_to_object_data->settings) &&
		settings->graphics_object)
	{
		return_code = 1;
		get_FE_element_identifier(element, &cm);
		element_graphics_name = CM_element_information_to_graphics_name(&cm);
		/* proceed only if settings uses this element */
		if (GT_element_settings_uses_FE_element(settings, element,
			settings_to_object_data->fe_region))
		{
			if ((GRAPHICS_DRAW_SELECTED == settings->select_mode) ||
				(GRAPHICS_DRAW_UNSELECTED == settings->select_mode))
			{
				draw_selected = (GRAPHICS_DRAW_SELECTED==settings->select_mode);
				name_selected = IS_OBJECT_IN_LIST(FE_element)(element,
					settings_to_object_data->selected_element_list);
				draw_element = ((draw_selected && name_selected) ||
					((!draw_selected) && (!name_selected)));
			}
			else
			{
				draw_element = 1;
			}
			/* determine discretization of element for graphic */
			top_level_element = (struct FE_element *)NULL;
			if (GT_ELEMENT_SETTINGS_ELEMENT_POINTS == settings->settings_type)
			{
				/* element_points always have their own discretization */
				top_level_number_in_xi[0] = settings->discretization.number_in_xi1;
				top_level_number_in_xi[1] = settings->discretization.number_in_xi2;
				top_level_number_in_xi[2] = settings->discretization.number_in_xi3;
				native_discretization_field = settings->native_discretization_field;
			}
			else
			{
				top_level_number_in_xi[0] =
					settings_to_object_data->element_discretization->number_in_xi1;
				top_level_number_in_xi[1] =
					settings_to_object_data->element_discretization->number_in_xi2;
				top_level_number_in_xi[2] =
					settings_to_object_data->element_discretization->number_in_xi3;
				native_discretization_field =
					settings_to_object_data->native_discretization_field;
			}
			if (FE_region_get_FE_element_discretization(
				settings_to_object_data->fe_region, element, settings->face,
				native_discretization_field, top_level_number_in_xi, &top_level_element,
				number_in_xi))
			{
				element_dimension = get_FE_element_dimension(element);
				/* g_element renditions use only one time = 0.0. Must take care. */
				time = 0.0;
				switch (settings->settings_type)
				{
					case GT_ELEMENT_SETTINGS_LINES:
					{
						if (settings_to_object_data->existing_graphics)
						{
							polyline = GT_OBJECT_EXTRACT_FIRST_PRIMITIVES_AT_TIME(GT_polyline)
								(settings_to_object_data->existing_graphics, time,
									element_graphics_name);
						}
						else
						{
							polyline = (struct GT_polyline *)NULL;
						}
						if (draw_element)
						{
							if (polyline ||
								(polyline = create_GT_polyline_from_FE_element(element,
									settings_to_object_data->rc_coordinate_field,
									settings->data_field, number_in_xi[0], top_level_element,
									settings_to_object_data->time, settings->line_width)))
							{
								if (!GT_OBJECT_ADD(GT_polyline)(
									settings->graphics_object, time, polyline))
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
						else
						{
							if (polyline)
							{
								DESTROY(GT_polyline)(&polyline);
							}
						}
					} break;
					case GT_ELEMENT_SETTINGS_CYLINDERS:
					{
						if (settings_to_object_data->existing_graphics)
						{
							surface = GT_OBJECT_EXTRACT_FIRST_PRIMITIVES_AT_TIME(GT_surface)
								(settings_to_object_data->existing_graphics, time,
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
									settings_to_object_data->rc_coordinate_field,
									settings->data_field, settings->constant_radius,
									settings->radius_scale_factor, settings->radius_scalar_field,
									number_in_xi[0],
									settings_to_object_data->circle_discretization,
									settings->texture_coordinate_field,
									top_level_element, settings_to_object_data->time)))
							{
								if (!GT_OBJECT_ADD(GT_surface)(
									settings->graphics_object, time, surface))
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
					case GT_ELEMENT_SETTINGS_SURFACES:
					{
						if (settings_to_object_data->existing_graphics)
						{
							surface = GT_OBJECT_EXTRACT_FIRST_PRIMITIVES_AT_TIME(GT_surface)
								(settings_to_object_data->existing_graphics, time,
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
									settings_to_object_data->rc_coordinate_field,
									settings->texture_coordinate_field, settings->data_field,
									number_in_xi[0], number_in_xi[1],
									/*reverse_normals*/0, top_level_element,settings->render_type,
									settings_to_object_data->time)))
							{
								if (!GT_OBJECT_ADD(GT_surface)(
									settings->graphics_object, time, surface))
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
					case GT_ELEMENT_SETTINGS_ISO_SURFACES:
					{
						switch (GT_object_get_type(settings->graphics_object))
						{
							case g_VOLTEX:
							{
								if (3 == element_dimension)
								{
									if (settings_to_object_data->existing_graphics)
									{
										voltex =
											GT_OBJECT_EXTRACT_FIRST_PRIMITIVES_AT_TIME(GT_voltex)
											(settings_to_object_data->existing_graphics, time,
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
												settings->graphics_object, time, voltex))
											{
												DESTROY(GT_voltex)(&voltex);
												return_code = 0;
											}
										}
										else
										{
											for (i = 0 ; i < settings->number_of_iso_values ; i++)
											{
												return_code = create_iso_surfaces_from_FE_element(element,
													settings->iso_values[i],
													settings_to_object_data->time,
													(struct Clipping *)NULL,
													settings_to_object_data->rc_coordinate_field,
													settings->data_field, settings->iso_scalar_field,
													settings->texture_coordinate_field,
													number_in_xi, settings->decimation_threshold,
													settings->graphics_object, settings->render_type);
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
							case g_POLYLINE:
							{
								if (2 == element_dimension)
								{
									if (settings_to_object_data->existing_graphics)
									{
										polyline =
											GT_OBJECT_EXTRACT_FIRST_PRIMITIVES_AT_TIME(GT_polyline)
											(settings_to_object_data->existing_graphics, time,
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
												settings->graphics_object, time, polyline))
											{
												DESTROY(GT_polyline)(&polyline);
												return_code = 0;
											}
										}
										else
										{
											for (i = 0 ; i < settings->number_of_iso_values ; i++)
											{
												return_code = create_iso_lines_from_FE_element(element,
													settings_to_object_data->rc_coordinate_field,
													settings->iso_scalar_field, settings->iso_values[i],
													settings_to_object_data->time,
													settings->data_field, number_in_xi[0], number_in_xi[1],
													top_level_element, settings->graphics_object, 
													/*graphics_object_time*/0.0);
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
					case GT_ELEMENT_SETTINGS_ELEMENT_POINTS:
					{
						glyph_set = (struct GT_glyph_set *)NULL;
						if (settings_to_object_data->existing_graphics)
						{
							glyph_set =
								GT_OBJECT_EXTRACT_FIRST_PRIMITIVES_AT_TIME(GT_glyph_set)(
									settings_to_object_data->existing_graphics, time,
									element_graphics_name);
						}
						if (!glyph_set)
						{
							for (i = 0; i < 3; i++)
							{
								element_point_ranges_identifier.exact_xi[i] =
									settings->seed_xi[i];
							}
							if (FE_element_get_xi_points(element,
								settings->xi_discretization_mode, number_in_xi,
								element_point_ranges_identifier.exact_xi,
								settings_to_object_data->rc_coordinate_field,
								settings->xi_point_density_field,
								&number_of_xi_points, &xi_points,
								settings_to_object_data->time))
							{
								get_FE_element_identifier(element, &cm);
								element_graphics_name =
									CM_element_information_to_graphics_name(&cm);
								top_level_xi_point_numbers = (int *)NULL;
								if (XI_DISCRETIZATION_CELL_CORNERS ==
									settings->xi_discretization_mode)
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
									settings->xi_discretization_mode;
								use_element_dimension = get_FE_element_dimension(use_element);
								for (i = 0; i < use_element_dimension; i++)
								{
									element_point_ranges_identifier.number_in_xi[i] =
										use_number_in_xi[i];
								}
								if (element_point_ranges = FIND_BY_IDENTIFIER_IN_LIST(
									Element_point_ranges, identifier)(
										&element_point_ranges_identifier,
										settings_to_object_data->selected_element_point_ranges_list))
								{
									ranges = Element_point_ranges_get_ranges(element_point_ranges);
								}
								element_selected = IS_OBJECT_IN_LIST(FE_element)(use_element,
									settings_to_object_data->selected_element_list);
								base_size[0] = (FE_value)(settings->glyph_size[0]);
								base_size[1] = (FE_value)(settings->glyph_size[1]);
								base_size[2] = (FE_value)(settings->glyph_size[2]);
								centre[0] = (FE_value)(settings->glyph_centre[0]);
								centre[1] = (FE_value)(settings->glyph_centre[1]);
								centre[2] = (FE_value)(settings->glyph_centre[2]);
								scale_factors[0] = (FE_value)(settings->glyph_scale_factors[0]);
								scale_factors[1] = (FE_value)(settings->glyph_scale_factors[1]);
								scale_factors[2] = (FE_value)(settings->glyph_scale_factors[2]);
								/* NOT an error if no glyph_set produced == empty selection */
								if ((0 < number_of_xi_points) &&
									(glyph_set = create_GT_glyph_set_from_FE_element(
										use_element, top_level_element,
										settings_to_object_data->rc_coordinate_field,
										number_of_xi_points, xi_points,
										settings->glyph, base_size, centre, scale_factors,
										settings_to_object_data->wrapper_orientation_scale_field,
										settings->variable_scale_field, settings->data_field,
										settings->font, settings->label_field, settings->select_mode,
										element_selected, ranges, top_level_xi_point_numbers,
										settings_to_object_data->time)))
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
								settings->graphics_object,time,glyph_set))
							{
								DESTROY(GT_glyph_set)(&glyph_set);
								return_code = 0;
							}
						}
					} break;
					case GT_ELEMENT_SETTINGS_VOLUMES:
					{
						if (settings_to_object_data->existing_graphics)
						{
							voltex = GT_OBJECT_EXTRACT_FIRST_PRIMITIVES_AT_TIME(GT_voltex)
								(settings_to_object_data->existing_graphics, time,
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
									settings_to_object_data->rc_coordinate_field,
									settings->data_field,
									settings->volume_texture, settings->render_type,
									settings->displacement_map_field,
									settings->displacement_map_xi_direction,
									settings->texture_coordinate_field,
									settings_to_object_data->time)))
							{
								if (!GT_OBJECT_ADD(GT_voltex)(
									settings->graphics_object, time, voltex))
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
					case GT_ELEMENT_SETTINGS_STREAMLINES:
					{
						/* use local copy of seed_xi since tracking function updates it */
						initial_xi[0] = settings->seed_xi[0];
						initial_xi[1] = settings->seed_xi[1];
						initial_xi[2] = settings->seed_xi[2];
						if (STREAM_LINE==settings->streamline_type)
						{
							if (polyline = create_GT_polyline_streamline_FE_element(element,
								initial_xi, settings_to_object_data->rc_coordinate_field,
								settings_to_object_data->wrapper_stream_vector_field,
								settings->reverse_track, settings->streamline_length,
								settings->streamline_data_type, settings->data_field,
								settings_to_object_data->time,settings_to_object_data->fe_region))
							{
								if (!GT_OBJECT_ADD(GT_polyline)(settings->graphics_object,
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
						else if ((settings->streamline_type == STREAM_RIBBON)||
							(settings->streamline_type == STREAM_EXTRUDED_RECTANGLE)||
							(settings->streamline_type == STREAM_EXTRUDED_ELLIPSE)||
							(settings->streamline_type == STREAM_EXTRUDED_CIRCLE))
						{
							if (surface = create_GT_surface_streamribbon_FE_element(element,
								initial_xi, settings_to_object_data->rc_coordinate_field,
								settings_to_object_data->wrapper_stream_vector_field,
								settings->reverse_track, settings->streamline_length,
								settings->streamline_width, settings->streamline_type,
								settings->streamline_data_type, settings->data_field,
									settings_to_object_data->time,settings_to_object_data->fe_region))
							{
								if (!GT_OBJECT_ADD(GT_surface)(settings->graphics_object,
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
							"Unknown element settings type");
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

static int GT_element_node_to_streamline(struct FE_node *node,
	void *settings_to_object_data_void)
/*******************************************************************************
LAST MODIFIED : 1 June 2006

DESCRIPTION :
Creates a node point seeded with the seed_node_coordinate_field at the node.
==============================================================================*/
{
	FE_value coordinates[3], xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	int element_dimension, number_of_components, return_code;
	struct FE_element *element;
	struct GT_element_settings *settings;
	struct GT_element_settings_to_graphics_object_data *settings_to_object_data;
	struct GT_polyline *polyline;
	struct GT_surface *surface;

	ENTER(node_to_streamline);

	if (node&&(settings_to_object_data =
		(struct GT_element_settings_to_graphics_object_data *)
		settings_to_object_data_void) &&
		(settings = settings_to_object_data->settings) &&
		settings->graphics_object)
	{
		element_dimension = 3;
		if ((3 >= (number_of_components = Computed_field_get_number_of_components(
			settings->seed_node_coordinate_field)))
			&& (number_of_components == Computed_field_get_number_of_components(
			settings_to_object_data->rc_coordinate_field)))
		{
			/* determine if the element is required */
			if (Computed_field_is_defined_at_node(settings->seed_node_coordinate_field, node)
				&& Computed_field_evaluate_at_node(settings->seed_node_coordinate_field, node,
					settings_to_object_data->time, coordinates)
				&& Computed_field_find_element_xi(settings_to_object_data->rc_coordinate_field,
					coordinates, number_of_components, &element, xi,
					element_dimension, settings_to_object_data->region,
					/*propagate_field*/0, /*find_nearest_location*/0))
			{
				if (STREAM_LINE==settings->streamline_type)
				{
					if (polyline=create_GT_polyline_streamline_FE_element(element,
							xi,settings_to_object_data->rc_coordinate_field,
							settings_to_object_data->wrapper_stream_vector_field,
							settings->reverse_track, settings->streamline_length,
							settings->streamline_data_type, settings->data_field,
							settings_to_object_data->time,settings_to_object_data->fe_region))
					{
						if (!(return_code=GT_OBJECT_ADD(GT_polyline)(
									settings->graphics_object,
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
				else if ((settings->streamline_type == STREAM_RIBBON)||
					(settings->streamline_type == STREAM_EXTRUDED_RECTANGLE)||
					(settings->streamline_type == STREAM_EXTRUDED_ELLIPSE)||
					(settings->streamline_type == STREAM_EXTRUDED_CIRCLE))
				{
					if (surface=create_GT_surface_streamribbon_FE_element(element,
							xi,settings_to_object_data->rc_coordinate_field,
							settings_to_object_data->wrapper_stream_vector_field,
							settings->reverse_track, settings->streamline_length,
							settings->streamline_width, settings->streamline_type,
							settings->streamline_data_type, settings->data_field,
							settings_to_object_data->time,settings_to_object_data->fe_region))
					{
						if (!(return_code=GT_OBJECT_ADD(GT_surface)(
									settings->graphics_object,
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
						"GT_element_node_to_streamline.  Unknown streamline type");
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
			"GT_element_node_to_streamline.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* node_to_streamline */

int GT_element_settings_to_graphics_object(
	struct GT_element_settings *settings,void *settings_to_object_data_void)
/*******************************************************************************
LAST MODIFIED : 30 April 2003

DESCRIPTION :
Creates a GT_object and fills it with the objects described by settings.
The graphics object is stored with with the settings it was created from.
==============================================================================*/
{
	char *existing_name, *graphics_object_name, *settings_name, *settings_string;
	FE_value base_size[3], centre[3], scale_factors[3];
	float time;
	enum GT_object_type graphics_object_type;
	int return_code;
	struct Computed_field *coordinate_field;
	struct Coordinate_system *coordinate_system;
	struct FE_element *first_element;
	struct FE_region *fe_region;
	struct GT_element_settings_to_graphics_object_data *settings_to_object_data;
	struct GT_glyph_set *glyph_set;
	struct LIST(FE_node) *selected_node_list;
	struct Multi_range *subranges;
	struct GT_element_settings_select_graphics_data select_data;

	ENTER(GT_element_settings_to_graphics_object);
	if (settings && (settings_to_object_data =
		(struct GT_element_settings_to_graphics_object_data *)
		settings_to_object_data_void) &&
		(((GT_ELEMENT_SETTINGS_DATA_POINTS == settings->settings_type) &&
			(fe_region = settings_to_object_data->data_fe_region)) ||
			(fe_region = settings_to_object_data->fe_region)))
	{
		/* all primitives added at time 0.0 */
		time = 0.0;
		coordinate_field = settings_to_object_data->default_rc_coordinate_field;
		return_code = 1;
		/* build only if visible... */
		if (settings->visibility)
		{
			/* build only if changed... */
			if (settings->graphics_changed)
			{
				/* override the default_coordinate_field with one chosen in settings */
				if (settings->coordinate_field)
				{
					coordinate_field = settings->coordinate_field;
				}
				if (coordinate_field)
				{
					/* RC coordinate_field to pass to FE_element_to_graphics_object */
					if ((settings_to_object_data->rc_coordinate_field=
							Computed_field_begin_wrap_coordinate_field(coordinate_field)) &&
						((!settings->orientation_scale_field) ||
							(settings_to_object_data->wrapper_orientation_scale_field =
								Computed_field_begin_wrap_orientation_scale_field(
									settings->orientation_scale_field,
									settings_to_object_data->rc_coordinate_field))) &&
						((!settings->stream_vector_field) ||
							(settings_to_object_data->wrapper_stream_vector_field =
								Computed_field_begin_wrap_orientation_scale_field(
									settings->stream_vector_field,
									settings_to_object_data->rc_coordinate_field))))
					{
#if defined (DEBUG)
						/*???debug*/
						if (settings_string = GT_element_settings_string(settings,
								SETTINGS_STRING_COMPLETE_PLUS))
						{
							printf("> building %s\n", settings_string);
							DEALLOCATE(settings_string);
						}
#endif /* defined (DEBUG) */
						settings_to_object_data->existing_graphics =
							(struct GT_object *)NULL;
						/* work out the name the graphics object is to have */
						GET_NAME(GT_element_settings)(settings, &settings_name);
						if (settings_to_object_data->name_prefix && settings_name &&
							ALLOCATE(graphics_object_name, char,
								strlen(settings_to_object_data->name_prefix) +
								strlen(settings_name) + 4))
						{
							sprintf(graphics_object_name, "%s_%s",
								settings_to_object_data->name_prefix, settings_name);
							if (settings->graphics_object)
							{
								/* replace the graphics object name */
								GT_object_set_name(settings->graphics_object,
									graphics_object_name);
								if (GT_object_has_primitives_at_time(settings->graphics_object,
										time))
								{
#if defined (DEBUG)
									/*???debug*/printf("  EDIT EXISTING GRAPHICS!\n");
#endif /* defined (DEBUG) */
									GET_NAME(GT_object)(settings->graphics_object, &existing_name);
									settings_to_object_data->existing_graphics =
										CREATE(GT_object)(existing_name,
											GT_object_get_type(settings->graphics_object),
											get_GT_object_default_material(settings->graphics_object));
									DEALLOCATE(existing_name);
									GT_object_transfer_primitives_at_time(
										settings_to_object_data->existing_graphics,
										settings->graphics_object, time);
								}
							}
							else
							{
								switch (settings->settings_type)
								{
									case GT_ELEMENT_SETTINGS_LINES:
									{
										graphics_object_type = g_POLYLINE;
									} break;
									case GT_ELEMENT_SETTINGS_CYLINDERS:
									case GT_ELEMENT_SETTINGS_SURFACES:
									{
										graphics_object_type = g_SURFACE;
									} break;
									case GT_ELEMENT_SETTINGS_ISO_SURFACES:
									{
										switch (settings->use_element_type)
										{
											case USE_ELEMENTS:
											{
												graphics_object_type = g_VOLTEX;
												if (first_element = FE_region_get_first_FE_element_that(
														 fe_region, FE_element_is_top_level, (void *)NULL))
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
													"GT_element_settings_to_graphics_object.  "
													"USE_LINES not supported for iso_scalar");
												return_code = 0;
											} break;
											default:
											{
												display_message(ERROR_MESSAGE,
													"GT_element_settings_to_graphics_object.  "
													"Unknown use_element_type");
												return_code = 0;
											} break;
										}
									} break;
									case GT_ELEMENT_SETTINGS_NODE_POINTS:
									case GT_ELEMENT_SETTINGS_DATA_POINTS:
									case GT_ELEMENT_SETTINGS_ELEMENT_POINTS:
									{
										graphics_object_type = g_GLYPH_SET;
									} break;
									case GT_ELEMENT_SETTINGS_VOLUMES:
									{
										graphics_object_type = g_VOLTEX;
									} break;
									case GT_ELEMENT_SETTINGS_STREAMLINES:
									{
										if (STREAM_LINE == settings->streamline_type)
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
											"GT_element_settings_to_graphics_object.  "
											"Unknown element settings type");
										return_code = 0;
									} break;
								}
								if (return_code)
								{
									settings->graphics_object = CREATE(GT_object)(
										graphics_object_name, graphics_object_type,
										settings->material);
									GT_object_set_select_mode(settings->graphics_object,
										settings->select_mode);
									if (settings->secondary_material)
									{
										set_GT_object_secondary_material(settings->graphics_object,
											settings->secondary_material);
									}
									if (settings->selected_material)
									{
										set_GT_object_selected_material(settings->graphics_object,
											settings->selected_material);
									}
									ACCESS(GT_object)(settings->graphics_object);
								}
							}
							DEALLOCATE(graphics_object_name);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"GT_element_settings_to_graphics_object.  "
								"Unable to make graphics object name");
							return_code = 0;
						}
						DEALLOCATE(settings_name);
						if (settings->graphics_object)
						{
							settings->selected_graphics_changed=1;
							coordinate_system = Computed_field_get_coordinate_system(coordinate_field);
							if (NORMALISED_WINDOW_COORDINATES == coordinate_system->type)
							{
								GT_object_set_coordinate_system(settings->graphics_object,
									g_NDC_COORDINATES);
							}
							/* need settings for FE_element_to_graphics_object routine */
							settings_to_object_data->settings=settings;
							switch (settings->settings_type)
							{
								case GT_ELEMENT_SETTINGS_NODE_POINTS:
								case GT_ELEMENT_SETTINGS_DATA_POINTS:
								{
									/* currently all nodes put together in a single GT_glyph_set,
										so rebuild all even if editing a single node or element */
									GT_object_remove_primitives_at_time(
										settings->graphics_object, time,
										(GT_object_primitive_object_name_conditional_function *)NULL,
										(void *)NULL);
									base_size[0] = (FE_value)(settings->glyph_size[0]);
									base_size[1] = (FE_value)(settings->glyph_size[1]);
									base_size[2] = (FE_value)(settings->glyph_size[2]);
									centre[0] = (FE_value)(settings->glyph_centre[0]);
									centre[1] = (FE_value)(settings->glyph_centre[1]);
									centre[2] = (FE_value)(settings->glyph_centre[2]);
									scale_factors[0] = (FE_value)(settings->glyph_scale_factors[0]);
									scale_factors[1] = (FE_value)(settings->glyph_scale_factors[1]);
									scale_factors[2] = (FE_value)(settings->glyph_scale_factors[2]);
									if (settings->settings_type == GT_ELEMENT_SETTINGS_DATA_POINTS)
									{
										selected_node_list = settings_to_object_data->selected_data_list;
									}
									else
									{
										selected_node_list = settings_to_object_data->selected_node_list;
									}
									glyph_set = create_GT_glyph_set_from_FE_region_nodes(
										fe_region, settings_to_object_data->rc_coordinate_field,
										settings->glyph, base_size, centre, scale_factors,
										settings_to_object_data->time,
										settings_to_object_data->wrapper_orientation_scale_field,
										settings->variable_scale_field, settings->data_field,
										settings->font, settings->label_field, settings->select_mode,
										selected_node_list);
									/* NOT an error if no glyph_set produced == empty group */
									if (glyph_set)
									{
										if (!GT_OBJECT_ADD(GT_glyph_set)(settings->graphics_object,
												time,glyph_set))
										{
											DESTROY(GT_glyph_set)(&glyph_set);
											return_code=0;
										}
									}
								} break;
								case GT_ELEMENT_SETTINGS_CYLINDERS:
								case GT_ELEMENT_SETTINGS_LINES:
								case GT_ELEMENT_SETTINGS_SURFACES:
								case GT_ELEMENT_SETTINGS_ELEMENT_POINTS:
								{
									return_code = FE_region_for_each_FE_element(fe_region,
										FE_element_to_graphics_object, settings_to_object_data_void);
								} break;
								case GT_ELEMENT_SETTINGS_ISO_SURFACES:
								{
									return_code = FE_region_for_each_FE_element(fe_region,
										
										FE_element_to_graphics_object, 
										settings_to_object_data_void);
									/* If the isosurface is a volume we can decimate and
										then normalise, otherwise if it is a polyline
										representing a isolines, skip over. */
									if (g_VOLTEX == GT_object_get_type(settings->graphics_object))
									{
										/* Decimate */
										if (settings->decimation_threshold > 0.0)
										{
											GT_object_decimate_GT_voltex(settings->graphics_object,
												settings->decimation_threshold);
										}
										/* Normalise normals now that the entire mesh has been calculated */
										GT_object_normalise_GT_voltex_normals(settings->graphics_object);
									}
								} break;
								case GT_ELEMENT_SETTINGS_VOLUMES:
								{
									/* does volume texture extend beyond a single element? */
									if (settings->volume_texture && (
											 (settings->volume_texture->ximin[0] < 0.0) ||
											 (settings->volume_texture->ximin[1] < 0.0) ||
											 (settings->volume_texture->ximin[2] < 0.0) ||
											 (settings->volume_texture->ximax[0] > 1.0) ||
											 (settings->volume_texture->ximax[1] > 1.0) ||
											 (settings->volume_texture->ximax[2] > 1.0)))
									{
										/* then must rebuild all graphics */
										if (settings_to_object_data->existing_graphics)
										{
											DESTROY(GT_object)(
												&(settings_to_object_data->existing_graphics));
										}
									}
									if (settings->seed_element)
									{
										return_code = FE_element_to_graphics_object(
											settings->seed_element, settings_to_object_data_void);
									}
									else
									{
										return_code = FE_region_for_each_FE_element(fe_region,
											FE_element_to_graphics_object,
											settings_to_object_data_void);
									}
								} break;
								case GT_ELEMENT_SETTINGS_STREAMLINES:
								{
									/* must always regenerate ALL streamlines since they can cross
										into other elements */
									if (settings_to_object_data->existing_graphics)
									{
										DESTROY(GT_object)(
											&(settings_to_object_data->existing_graphics));
									}
									if (settings->seed_element)
									{
										return_code = FE_element_to_graphics_object(
											settings->seed_element, settings_to_object_data_void);
									}
									else if (settings->seed_node_region &&
										settings->seed_node_coordinate_field)
									{
										return_code = FE_region_for_each_FE_node(
											settings->seed_node_region,
											GT_element_node_to_streamline, settings_to_object_data_void);
									}
									else
									{
										return_code = FE_region_for_each_FE_element(fe_region,
											FE_element_to_graphics_object,
											settings_to_object_data_void);
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
								if ((settings->data_field)||
									((GT_ELEMENT_SETTINGS_STREAMLINES == settings->settings_type) &&
										(STREAM_NO_DATA != settings->streamline_data_type)))
								{
									set_GT_object_Spectrum(settings->graphics_object,
										(void *)(settings->spectrum));
								}
								/* mark display list as needing updating */
								settings->graphics_changed = 0;
								GT_object_changed(settings->graphics_object);
							}
							else
							{
								settings_string = GT_element_settings_string(settings,
									SETTINGS_STRING_COMPLETE_PLUS);
								display_message(ERROR_MESSAGE,
									"GT_element_settings_to_graphics_object.  "
									"Could not build '%s'",settings_string);
								DEALLOCATE(settings_string);
								/* set return_code to 1, so rest of settings can be built */
								return_code = 1;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"GT_element_settings_to_graphics_object.  "
								"Could not create graphics object");
							return_code = 0;
						}
						if (settings_to_object_data->existing_graphics)
						{
							DESTROY(GT_object)(&(settings_to_object_data->existing_graphics));
						}
						if (settings->stream_vector_field)
						{
							Computed_field_clear_cache(
								settings_to_object_data->wrapper_stream_vector_field);
							Computed_field_end_wrap(
								&(settings_to_object_data->wrapper_stream_vector_field));
						}
						if (settings->orientation_scale_field)
						{
							Computed_field_end_wrap(
								&(settings_to_object_data->wrapper_orientation_scale_field));
						}
						Computed_field_end_wrap(
							&(settings_to_object_data->rc_coordinate_field));
						if (settings->seed_node_coordinate_field)
						{
							Computed_field_clear_cache(settings->seed_node_coordinate_field);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"GT_element_settings_to_graphics_object.  "
							"Could not get rc_coordinate_field wrapper");
						return_code = 0;
					}
				}
			}
			if (settings->selected_graphics_changed)
			{
				if (settings->graphics_object)
				{
					GT_object_clear_selected_graphic_list(settings->graphics_object);
					if ((GRAPHICS_SELECT_ON == settings->select_mode) ||
						(GRAPHICS_DRAW_SELECTED == settings->select_mode))
					{
						switch (settings->settings_type)
						{
							case GT_ELEMENT_SETTINGS_DATA_POINTS:
							{
								if (0<NUMBER_IN_LIST(FE_node)(
									settings_to_object_data->selected_data_list))
								{
									if (subranges=CREATE(Multi_range)())
									{
										FOR_EACH_OBJECT_IN_LIST(FE_node)(
											add_FE_node_number_to_Multi_range,(void *)subranges,
											settings_to_object_data->selected_data_list);
										if (!GT_object_select_graphic(settings->graphics_object,
											0,subranges))
										{
											DESTROY(Multi_range)(&subranges);
										}
									}
								}
							} break;
							case GT_ELEMENT_SETTINGS_NODE_POINTS:
							{
								if (0<NUMBER_IN_LIST(FE_node)(
									settings_to_object_data->selected_node_list))
								{
									if (subranges=CREATE(Multi_range)())
									{
										FOR_EACH_OBJECT_IN_LIST(FE_node)(
											add_FE_node_number_to_Multi_range,(void *)subranges,
											settings_to_object_data->selected_node_list);
										if (!GT_object_select_graphic(settings->graphics_object,
											0,subranges))
										{
											DESTROY(Multi_range)(&subranges);
										}
									}
								}
							} break;
							case GT_ELEMENT_SETTINGS_CYLINDERS:
							case GT_ELEMENT_SETTINGS_LINES:
							case GT_ELEMENT_SETTINGS_SURFACES:
							case GT_ELEMENT_SETTINGS_ISO_SURFACES:
							case GT_ELEMENT_SETTINGS_VOLUMES:
							{
								select_data.fe_region=fe_region;
								select_data.settings=settings;
								FOR_EACH_OBJECT_IN_LIST(FE_element)(
									FE_element_select_graphics,(void *)&select_data,
									settings_to_object_data->selected_element_list);
							} break;
							case GT_ELEMENT_SETTINGS_ELEMENT_POINTS:
							{
								select_data.fe_region=fe_region;
								select_data.settings=settings;
								FOR_EACH_OBJECT_IN_LIST(FE_element)(
									FE_element_select_graphics_element_points,
									(void *)&select_data,
									settings_to_object_data->selected_element_list);
								/* select Element_point_ranges for glyph_sets not already
									 selected as elements */
								FOR_EACH_OBJECT_IN_LIST(Element_point_ranges)(
									Element_point_ranges_select_in_graphics_object,
									(void *)&select_data,
									settings_to_object_data->selected_element_point_ranges_list);
							} break;
							case GT_ELEMENT_SETTINGS_STREAMLINES:
							{
								/* no element to select by since go through several */
							} break;
							default:
							{
								display_message(ERROR_MESSAGE,
									"GT_element_settings_to_graphics_object.  "
									"Unknown settings type");
							} break;
						}
					}
				}
				settings->selected_graphics_changed = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_to_graphics_object.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_to_graphics_object */

int GT_element_settings_selected_element_points_change(
	struct GT_element_settings *settings,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 28 February 2000

DESCRIPTION :
Tells <settings> that if the graphics resulting from it depend on the currently
selected element points, then they should be updated.
Must call GT_element_settings_to_graphics_object afterwards to complete.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_settings_selected_element_points_change);
	USE_PARAMETER(dummy_void);
	if (settings)
	{
		return_code=1;
		if (settings->graphics_object&&
			(GT_ELEMENT_SETTINGS_ELEMENT_POINTS==settings->settings_type))
		{
			switch (settings->select_mode)
			{
				case GRAPHICS_SELECT_ON:
				{
					/* for efficiency, just request update of selected graphics */
					settings->selected_graphics_changed=1;
				} break;
				case GRAPHICS_NO_SELECT:
				{
					/* nothing to do as no names put out with graphic */
				} break;
				case GRAPHICS_DRAW_SELECTED:
				case GRAPHICS_DRAW_UNSELECTED:
				{
					/* need to rebuild graphics_object from scratch */
					GT_element_settings_clear_graphics(settings);
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"GT_element_settings_selected_element_points_change.  "
						"Unknown select_mode");
				} break;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_selected_element_points_change.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_selected_element_points_change */

int GT_element_settings_selected_elements_change(
	struct GT_element_settings *settings,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 25 February 2000

DESCRIPTION :
Tells <settings> that if the graphics resulting from it depend on the currently
selected elements, then they should be updated.
Must call GT_element_settings_to_graphics_object afterwards to complete.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_settings_selected_elements_change);
	USE_PARAMETER(dummy_void);
	if (settings)
	{
		return_code=1;
		if (settings->graphics_object&&(
			GT_element_settings_type_uses_dimension(settings->settings_type,1)||
			GT_element_settings_type_uses_dimension(settings->settings_type,2)||
			GT_element_settings_type_uses_dimension(settings->settings_type,3)||
			GT_element_settings_has_embedded_field(settings, NULL)))
		{
			switch (settings->select_mode)
			{
				case GRAPHICS_SELECT_ON:
				{
					/* for efficiency, just request update of selected graphics */
					settings->selected_graphics_changed=1;
				} break;
				case GRAPHICS_NO_SELECT:
				{
					/* nothing to do as no names put out with graphic */
				} break;
				case GRAPHICS_DRAW_SELECTED:
				case GRAPHICS_DRAW_UNSELECTED:
				{
					/* need to rebuild graphics_object from scratch */
					GT_element_settings_clear_graphics(settings);
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"GT_element_settings_selected_elements_change.  "
						"Unknown select_mode");
				} break;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_selected_elements_change.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_selected_elements_change */

int GT_element_settings_selected_nodes_change(
	struct GT_element_settings *settings,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 25 February 2000

DESCRIPTION :
Tells <settings> that if the graphics resulting from it depend on the currently
selected nodes, then they should be updated.
Must call GT_element_settings_to_graphics_object afterwards to complete.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_settings_selected_nodes_change);
	USE_PARAMETER(dummy_void);
	if (settings)
	{
		return_code=1;
		if (settings->graphics_object&&
			(GT_ELEMENT_SETTINGS_NODE_POINTS==settings->settings_type))
		{
			switch (settings->select_mode)
			{
				case GRAPHICS_SELECT_ON:
				{
					/* for efficiency, just request update of selected graphics */
					settings->selected_graphics_changed=1;
				} break;
				case GRAPHICS_NO_SELECT:
				{
					/* nothing to do as no names put out with graphic */
				} break;
				case GRAPHICS_DRAW_SELECTED:
				case GRAPHICS_DRAW_UNSELECTED:
				{
					/* need to rebuild graphics_object from scratch */
					GT_element_settings_clear_graphics(settings);
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"GT_element_settings_selected_nodes_change.  Unknown select_mode");
				} break;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_selected_nodes_change.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_selected_nodes_change */

int GT_element_settings_selected_data_change(
	struct GT_element_settings *settings,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 28 April 2000

DESCRIPTION :
Tells <settings> that if the graphics resulting from it depend on the currently
selected nodes, then they should be updated.
Must call GT_element_settings_to_graphics_object afterwards to complete.
==============================================================================*/
{
	int return_code;

	ENTER(GT_element_settings_selected_data_change);
	USE_PARAMETER(dummy_void);
	if (settings)
	{
		return_code=1;
		if (settings->graphics_object&&
			(GT_ELEMENT_SETTINGS_DATA_POINTS==settings->settings_type))
		{
			switch (settings->select_mode)
			{
				case GRAPHICS_SELECT_ON:
				{
					/* for efficiency, just request update of selected graphics */
					settings->selected_graphics_changed=1;
				} break;
				case GRAPHICS_NO_SELECT:
				{
					/* nothing to do as no names put out with graphic */
				} break;
				case GRAPHICS_DRAW_SELECTED:
				case GRAPHICS_DRAW_UNSELECTED:
				{
					/* need to rebuild graphics_object from scratch */
					GT_element_settings_clear_graphics(settings);
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"GT_element_settings_selected_data_change.  Unknown select_mode");
				} break;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_selected_data_change.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_selected_data_change */

int GT_element_settings_compile_visible_settings(
	struct GT_element_settings *settings, void *context_void)
/*******************************************************************************
LAST MODIFIED : 18 November 2005

DESCRIPTION :
If the settings visibility flag is set and it has a graphics_object, the
graphics_object is compiled.
==============================================================================*/
{
	int return_code;
	struct GT_object_compile_context *context;

	ENTER(GT_element_settings_compile_visible_graphics_object);
	if (settings && (context = (struct GT_object_compile_context *)context_void))
	{
		if (settings->graphics_object && settings->visibility)
		{
			return_code = compile_GT_object(settings->graphics_object, context);
		}
		else
		{
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_compile_visible_settings.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_compile_visible_settings */

int GT_element_settings_execute_visible_settings(
	struct GT_element_settings *settings, void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 12 October 2005

DESCRIPTION :
If the settings visibility flag is set and it has a graphics_object, the
graphics_object is executed, while the position of the settings in the list
is put out as a name to identify the object in OpenGL picking.
==============================================================================*/
{
	int return_code;
	
	ENTER(GT_element_settings_execute_visible_settings);
	USE_PARAMETER(dummy_void);
	if (settings)
	{
		if (settings->graphics_object && settings->visibility)
		{
#if defined (OPENGL_API)
			/* use position in list as name for GL picking */
			glLoadName((GLuint)settings->position);
#endif /* defined (OPENGL_API) */
			return_code=execute_GT_object(settings->graphics_object);
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_execute_visible_settings.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_execute_visible_settings */

int GT_element_settings_Computed_field_change(
	struct GT_element_settings *settings, void *change_data_void)
/*******************************************************************************
LAST MODIFIED : 25 March 2003

DESCRIPTION :
Iterator function telling the <settings> that the computed fields in the
<changed_field_list> have changed.
Clears graphics for settings affected by the change.
<change_data_void> points at a
struct GT_element_settings_Computed_field_change_data.
==============================================================================*/
{
	int return_code;
	struct GT_element_settings_Computed_field_change_data *change_data;

	ENTER(GT_element_settings_Computed_field_change);
	if (settings && (change_data =
		(struct GT_element_settings_Computed_field_change_data *)change_data_void))
	{
		return_code = 1;
		if (GT_element_settings_Computed_field_or_ancestor_satisfies_condition(
			settings, change_data->default_coordinate_field,
			Computed_field_is_in_list, (void *)change_data->changed_field_list))
		{
			GT_element_settings_clear_graphics(settings);
			change_data->graphics_changed = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_Computed_field_change.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_Computed_field_change */

int GT_element_settings_Graphical_material_change(
	struct GT_element_settings *settings, void *material_change_data_void)
/*******************************************************************************
LAST MODIFIED : 13 March 2002

DESCRIPTION :
If <settings> has a graphics object that plots data, and it uses any material in
the <changed_material_list>, the graphics object display list is marked as being
not current. This is required since the spectrum combines its colour with the
material at the time it is compiled - it may not update correctly if only the
material's display list is recompiled.
If the settings are visible, the changed flag is set.
Second argument is a struct GT_element_settings_Graphical_material_change_data.
==============================================================================*/
{
	int return_code;
	struct GT_element_settings_Graphical_material_change_data
		*material_change_data;

	ENTER(GT_element_settings_Graphical_material_change);
	if (settings && (material_change_data =
		(struct GT_element_settings_Graphical_material_change_data *)
		material_change_data_void))
	{
		if (IS_OBJECT_IN_LIST(Graphical_material)(
			settings->material, material_change_data->changed_material_list) ||
			(settings->secondary_material &&
				IS_OBJECT_IN_LIST(Graphical_material)(settings->secondary_material,
					material_change_data->changed_material_list)) ||
			(settings->selected_material && 
				IS_OBJECT_IN_LIST(Graphical_material)(settings->selected_material,
					material_change_data->changed_material_list)))
		{
			if (settings->graphics_object)
			{
				GT_object_Graphical_material_change(settings->graphics_object,
					(struct LIST(Graphical_material) *)NULL);
			}
			/* GT_element_group only changed if settings are visible */
			if (settings->visibility)
			{
				material_change_data->changed = 1;
			}
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_Graphical_material_change.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_Graphical_material_change */

int GT_element_settings_Spectrum_change(
	struct GT_element_settings *settings, void *spectrum_change_data_void)
/*******************************************************************************
LAST MODIFIED : 12 March 2002

DESCRIPTION :
If <settings> has a graphics object that plots a scalar using a spectrum in the
<changed_spectrum_list>, the graphics object display list is marked as being
not current. If the settings are visible, the changed flag is set.
Second argument is a struct GT_element_settings_Spectrum_change_data.
==============================================================================*/
{
	int return_code;
	struct GT_element_settings_Spectrum_change_data *spectrum_change_data;
	struct Spectrum *colour_lookup;

	ENTER(GT_element_settings_Spectrum_change);
	if (settings && (spectrum_change_data =
		(struct GT_element_settings_Spectrum_change_data *)
		spectrum_change_data_void))
	{
		if (settings->spectrum && IS_OBJECT_IN_LIST(Spectrum)(settings->spectrum,
			spectrum_change_data->changed_spectrum_list))
		{
			if (settings->graphics_object)
			{
				GT_object_Spectrum_change(settings->graphics_object,
					(struct LIST(Spectrum) *)NULL);
			}
			/* GT_element_group only changed if settings are visible */
			if (settings->visibility)
			{
				spectrum_change_data->changed = 1;
			}
		}
		/* The material gets it's own notification of the change, 
			it should propagate that to the GT_element_settings */
		if (settings->material && (colour_lookup = 
				Graphical_material_get_colour_lookup_spectrum(settings->material))
			&& IS_OBJECT_IN_LIST(Spectrum)(colour_lookup,
				spectrum_change_data->changed_spectrum_list))
		{
			if (settings->graphics_object)
			{
				GT_object_Graphical_material_change(settings->graphics_object,
					(struct LIST(Graphical_material) *)NULL);
			}
			/* GT_element_group only changed if settings are visible */
			if (settings->visibility)
			{
				spectrum_change_data->changed = 1;
			}
		}
		
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_Spectrum_change.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_Spectrum_change */

int GT_element_settings_get_visible_graphics_object_range(
	struct GT_element_settings *settings,void *graphics_object_range_void)
/*******************************************************************************
LAST MODIFIED : 15 June 1998

DESCRIPTION :
If there is a visible graphics_object in <settings>, expands the
<graphics_object_range> to include its range.
==============================================================================*/
{
	int return_code;
	
	ENTER(GT_element_settings_get_visible_graphics_object_range);
	if (settings)
	{
		if (settings->graphics_object && settings->visibility)
		{
			return_code=get_graphics_object_range(settings->graphics_object,
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
			"GT_element_settings_get_visible_graphics_object_range.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_get_visible_graphics_object_range */

int gfx_modify_g_element_node_points(struct Parse_state *state,
	void *modify_g_element_data_void,void *g_element_command_data_void)
/*******************************************************************************
LAST MODIFIED : 20 March 2001

DESCRIPTION :
Executes a GFX MODIFY G_ELEMENT NODE_POINTS command.
If return_code is 1, returns the completed Modify_g_element_data with the
parsed settings. Note that the settings are ACCESSed once on valid return.
==============================================================================*/
{
	char *font_name, *glyph_scaling_mode_string, *select_mode_string, **valid_strings;
	enum Glyph_scaling_mode glyph_scaling_mode;
	enum Graphics_select_mode select_mode;
	int number_of_components,number_of_valid_strings,return_code,
		visibility;
	struct Computed_field *orientation_scale_field, *variable_scale_field;
	struct Graphics_font *new_font;
	struct GT_element_settings *settings;
	struct GT_object *glyph;
	struct G_element_command_data *g_element_command_data;
	struct Modify_g_element_data *modify_g_element_data;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_data_field_data, set_label_field_data, set_orientation_scale_field_data,
		set_variable_scale_field_data;
	Triple glyph_centre, glyph_scale_factors, glyph_size;

	ENTER(gfx_modify_g_element_node_points);
	if (state)
	{
		if (g_element_command_data=(struct G_element_command_data *)
			g_element_command_data_void)
		{
			if (modify_g_element_data=
				(struct Modify_g_element_data *)modify_g_element_data_void)
			{
				/* create the gt_element_settings: */
				settings = CREATE(GT_element_settings)(GT_ELEMENT_SETTINGS_NODE_POINTS);
				/* if we are specifying the name then copy the existings settings values so
					we can modify from these rather than the default */
				if (modify_g_element_data->settings)
				{
					if (GT_ELEMENT_SETTINGS_NODE_POINTS ==modify_g_element_data->settings->settings_type)
					{
						GT_element_settings_copy_without_graphics_object(settings, modify_g_element_data->settings); 
					}
					DEACCESS(GT_element_settings)(&modify_g_element_data->settings);
				}
				if (settings)
				{
					/* access since deaccessed in gfx_modify_g_element */
					modify_g_element_data->settings = ACCESS(GT_element_settings)(settings);
					/* set essential parameters not set by CREATE function */
					if (!GT_element_settings_get_material(settings))
					{
						GT_element_settings_set_material(settings,
							g_element_command_data->default_material);
					}
					if (!GT_element_settings_get_selected_material(settings))
					{
						GT_element_settings_set_selected_material(settings,
							FIND_BY_IDENTIFIER_IN_MANAGER(Graphical_material,name)(
								"default_selected",
								g_element_command_data->graphical_material_manager));
					}
					if (!settings->font)
					{
						settings->font = ACCESS(Graphics_font)(
							g_element_command_data->default_font);
					}
					font_name = (char *)NULL;
					orientation_scale_field = (struct Computed_field *)NULL;
					variable_scale_field = (struct Computed_field *)NULL;
					GT_element_settings_get_glyph_parameters(settings,
						&glyph, &glyph_scaling_mode, glyph_centre, glyph_size,
						&orientation_scale_field, glyph_scale_factors,
						&variable_scale_field);
					/* default to point glyph for fastest possible display */
					if (!glyph)
					{
						glyph = FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)("point",
							g_element_command_data->glyph_list);
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
					visibility = settings->visibility;

					option_table=CREATE(Option_table)();
					/* as */
					Option_table_add_entry(option_table,"as",&(settings->name),
						(void *)1,set_name);
					/* centre */
					Option_table_add_entry(option_table,"centre",glyph_centre,
						&(number_of_components),set_float_vector);
					/* coordinate */
					set_coordinate_field_data.computed_field_manager=
						g_element_command_data->computed_field_manager;
					set_coordinate_field_data.conditional_function=
						Computed_field_has_up_to_3_numerical_components;
					set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
					Option_table_add_entry(option_table,"coordinate",
						&(settings->coordinate_field),&set_coordinate_field_data,
						set_Computed_field_conditional);
					/* data */
					set_data_field_data.computed_field_manager=
						g_element_command_data->computed_field_manager;
					set_data_field_data.conditional_function=
						Computed_field_has_numerical_components;
					set_data_field_data.conditional_function_user_data=(void *)NULL;
					Option_table_add_entry(option_table,"data",&(settings->data_field),
						&set_data_field_data,set_Computed_field_conditional);
					/* delete */
					Option_table_add_entry(option_table,"delete",
						&(modify_g_element_data->delete_flag),NULL,set_char_flag);
					/* font */
					Option_table_add_name_entry(option_table, "font",
						&font_name);
					/* glyph */
					Option_table_add_entry(option_table,"glyph",&glyph,
						g_element_command_data->glyph_list,set_Graphics_object);
					/* label */
					set_label_field_data.computed_field_manager=
						g_element_command_data->computed_field_manager;
					set_label_field_data.conditional_function=
						(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
					set_label_field_data.conditional_function_user_data=(void *)NULL;
					Option_table_add_entry(option_table,"label",&(settings->label_field),
						&set_label_field_data,set_Computed_field_conditional);
					/* material */
					Option_table_add_entry(option_table,"material",&(settings->material),
						g_element_command_data->graphical_material_manager,
						set_Graphical_material);
					/* glyph scaling mode */
					glyph_scaling_mode_string =
						ENUMERATOR_STRING(Glyph_scaling_mode)(glyph_scaling_mode);
					valid_strings = ENUMERATOR_GET_VALID_STRINGS(Glyph_scaling_mode)(
						&number_of_valid_strings,
						(ENUMERATOR_CONDITIONAL_FUNCTION(Glyph_scaling_mode) *)NULL,
						(void *)NULL);
					Option_table_add_enumerator(option_table, number_of_valid_strings,
						valid_strings, &glyph_scaling_mode_string);
					DEALLOCATE(valid_strings);
					/* orientation */
					set_orientation_scale_field_data.computed_field_manager=
						g_element_command_data->computed_field_manager;
					set_orientation_scale_field_data.conditional_function=
						Computed_field_is_orientation_scale_capable;
					set_orientation_scale_field_data.conditional_function_user_data=
						(void *)NULL;
					Option_table_add_entry(option_table,"orientation",
						&orientation_scale_field,&set_orientation_scale_field_data,
						set_Computed_field_conditional);
					/* position */
					Option_table_add_entry(option_table,"position",
						&(modify_g_element_data->position),NULL,set_int_non_negative);
					/* scale_factors */
					Option_table_add_entry(option_table,"scale_factors",
						glyph_scale_factors,"*",set_special_float3);
					/* scene */
					Option_table_add_entry(option_table,"scene",
						&(modify_g_element_data->scene),
						g_element_command_data->scene_manager,set_Scene);
					/* select_mode */
					select_mode = GT_element_settings_get_select_mode(settings);
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
						&(settings->selected_material),
						g_element_command_data->graphical_material_manager,
						set_Graphical_material);
					/* size */
					Option_table_add_entry(option_table,"size",
						glyph_size,"*",set_special_float3);
					/* spectrum */
					Option_table_add_entry(option_table,"spectrum",
						&(settings->spectrum),g_element_command_data->spectrum_manager,
						set_Spectrum);
					/* variable_scale */
					set_variable_scale_field_data.computed_field_manager =
						g_element_command_data->computed_field_manager;
					set_variable_scale_field_data.conditional_function =
						Computed_field_has_up_to_3_numerical_components;
					set_variable_scale_field_data.conditional_function_user_data =
						(void *)NULL;
					Option_table_add_entry(option_table,"variable_scale",
						&variable_scale_field, &set_variable_scale_field_data,
						set_Computed_field_conditional);
					/* visible/invisible */
					Option_table_add_switch(option_table, "visible", "invisible", &visibility);
					if (return_code=Option_table_multi_parse(option_table,state))
					{
						if (settings->data_field&&!settings->spectrum)
						{
							settings->spectrum=ACCESS(Spectrum)(
								g_element_command_data->default_spectrum);
						}
						settings->visibility = visibility;
						if (font_name && (new_font = Graphics_font_package_get_font
								(g_element_command_data->graphics_font_package, font_name)))
						{
							REACCESS(Graphics_font)(&settings->font, new_font);
						}
						if (glyph)
						{
							STRING_TO_ENUMERATOR(Glyph_scaling_mode)(
								glyph_scaling_mode_string, &glyph_scaling_mode);
							GT_element_settings_set_glyph_parameters(settings,
								glyph, glyph_scaling_mode, glyph_centre, glyph_size,
								orientation_scale_field,glyph_scale_factors,
								variable_scale_field);
							GT_object_add_callback(settings->glyph,
								GT_element_settings_glyph_change, (void *)settings);
						}
						else
						{
							display_message(WARNING_MESSAGE,
								"No glyph specified for node_points");
							return_code=0;
						}
						STRING_TO_ENUMERATOR(Graphics_select_mode)(select_mode_string,
							&select_mode);
						GT_element_settings_set_select_mode(settings, select_mode);
					}
					DESTROY(Option_table)(&option_table);
					if (!return_code)
					{
						/* parse error, help */
						DEACCESS(GT_element_settings)(&(modify_g_element_data->settings));
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
						"gfx_modify_g_element_node_points.  Could not create settings");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_modify_g_element_node_points.  No modify data");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_modify_g_element_node_points.  "
				"Missing g_element_command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_g_element_node_points.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_g_element_node_points */

int gfx_modify_g_element_data_points(struct Parse_state *state,
	void *modify_g_element_data_void,void *g_element_command_data_void)
/*******************************************************************************
LAST MODIFIED : 19 March 2001

DESCRIPTION :
Executes a GFX MODIFY G_ELEMENT DATA_POINTS command.
If return_code is 1, returns the completed Modify_g_element_data with the
parsed settings. Note that the settings are ACCESSed once on valid return.
==============================================================================*/
{
	char *font_name, *glyph_scaling_mode_string, *select_mode_string, **valid_strings;
	enum Glyph_scaling_mode glyph_scaling_mode;
	enum Graphics_select_mode select_mode;
	int number_of_components,number_of_valid_strings,return_code,visibility;
	struct Computed_field *orientation_scale_field, *variable_scale_field;
	struct Graphics_font *new_font;
	struct GT_element_settings *settings;
	struct GT_object *glyph;
	struct G_element_command_data *g_element_command_data;
	struct Modify_g_element_data *modify_g_element_data;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_data_field_data, set_label_field_data, set_orientation_scale_field_data,
		set_variable_scale_field_data;
	Triple glyph_centre, glyph_scale_factors, glyph_size;

	ENTER(gfx_modify_g_element_data_points);
	if (state)
	{
		if (g_element_command_data=(struct G_element_command_data *)
			g_element_command_data_void)
		{
			if (modify_g_element_data=
				(struct Modify_g_element_data *)modify_g_element_data_void)
			{
				/* create the gt_element_settings: */
				settings = CREATE(GT_element_settings)(GT_ELEMENT_SETTINGS_DATA_POINTS);
				/* if we are specifying the name then copy the existings settings values so
					we can modify from these rather than the default */
				if (modify_g_element_data->settings)
				{
					if (GT_ELEMENT_SETTINGS_DATA_POINTS ==modify_g_element_data->settings->settings_type)
					{
						GT_element_settings_copy_without_graphics_object(settings, modify_g_element_data->settings); 
					}
					DEACCESS(GT_element_settings)(&modify_g_element_data->settings);
				}
				if (settings)
				{
					/* access since deaccessed in gfx_modify_g_element */
					modify_g_element_data->settings = ACCESS(GT_element_settings)(settings);
					/* set essential parameters not set by CREATE function */
					if (!GT_element_settings_get_material(settings))
					{
						GT_element_settings_set_material(settings,
							g_element_command_data->default_material);
					}
					if (!GT_element_settings_get_selected_material(settings))
					{
						GT_element_settings_set_selected_material(settings,
							FIND_BY_IDENTIFIER_IN_MANAGER(Graphical_material,name)(
								"default_selected",
								g_element_command_data->graphical_material_manager));
					}
					if (!settings->font)
					{
						settings->font = ACCESS(Graphics_font)(
							g_element_command_data->default_font);
					}
					font_name = (char *)NULL;
					orientation_scale_field = (struct Computed_field *)NULL;
					variable_scale_field = (struct Computed_field *)NULL;
					GT_element_settings_get_glyph_parameters(settings,
						&glyph, &glyph_scaling_mode, glyph_centre, glyph_size,
						&orientation_scale_field, glyph_scale_factors,
						&variable_scale_field);
					/* default to point glyph for fastest possible display */
					if (!glyph)
					{
						glyph = FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)("point",
							g_element_command_data->glyph_list);
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
					visibility = settings->visibility;

					option_table=CREATE(Option_table)();
					/* as */
					Option_table_add_entry(option_table,"as",&(settings->name),
						(void *)1,set_name);
					/* centre */
					Option_table_add_entry(option_table,"centre",glyph_centre,
						&(number_of_components),set_float_vector);
					/* coordinate */
					set_coordinate_field_data.computed_field_manager=
						g_element_command_data->computed_field_manager;
					set_coordinate_field_data.conditional_function=
						Computed_field_has_up_to_3_numerical_components;
					set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
					Option_table_add_entry(option_table,"coordinate",
						&(settings->coordinate_field),&set_coordinate_field_data,
						set_Computed_field_conditional);
					/* data */
					set_data_field_data.computed_field_manager=
						g_element_command_data->computed_field_manager;
					set_data_field_data.conditional_function=
						Computed_field_has_numerical_components;
					set_data_field_data.conditional_function_user_data=(void *)NULL;
					Option_table_add_entry(option_table,"data",&(settings->data_field),
						&set_data_field_data,set_Computed_field_conditional);
					/* delete */
					Option_table_add_entry(option_table,"delete",
						&(modify_g_element_data->delete_flag),NULL,set_char_flag);
					/* font */
					Option_table_add_name_entry(option_table, "font",
						&font_name);
					/* glyph */
					Option_table_add_entry(option_table,"glyph",&glyph,
						g_element_command_data->glyph_list,set_Graphics_object);
					/* label */
					set_label_field_data.computed_field_manager=
						g_element_command_data->computed_field_manager;
					set_label_field_data.conditional_function=
						(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
					set_label_field_data.conditional_function_user_data=(void *)NULL;
					Option_table_add_entry(option_table,"label",&(settings->label_field),
						&set_label_field_data,set_Computed_field_conditional);
					/* material */
					Option_table_add_entry(option_table,"material",&(settings->material),
						g_element_command_data->graphical_material_manager,
						set_Graphical_material);
					/* glyph scaling mode */
					glyph_scaling_mode_string =
						ENUMERATOR_STRING(Glyph_scaling_mode)(glyph_scaling_mode);
					valid_strings = ENUMERATOR_GET_VALID_STRINGS(Glyph_scaling_mode)(
						&number_of_valid_strings,
						(ENUMERATOR_CONDITIONAL_FUNCTION(Glyph_scaling_mode) *)NULL,
						(void *)NULL);
					Option_table_add_enumerator(option_table, number_of_valid_strings,
						valid_strings, &glyph_scaling_mode_string);
					DEALLOCATE(valid_strings);
					/* orientation */
					set_orientation_scale_field_data.computed_field_manager=
						g_element_command_data->computed_field_manager;
					set_orientation_scale_field_data.conditional_function=
						Computed_field_is_orientation_scale_capable;
					set_orientation_scale_field_data.conditional_function_user_data=
						(void *)NULL;
					Option_table_add_entry(option_table,"orientation",
						&orientation_scale_field,&set_orientation_scale_field_data,
						set_Computed_field_conditional);
					/* position */
					Option_table_add_entry(option_table,"position",
						&(modify_g_element_data->position),NULL,set_int_non_negative);
					/* scale_factors */
					Option_table_add_entry(option_table,"scale_factors",
						glyph_scale_factors,"*",set_special_float3);
					/* scene */
					Option_table_add_entry(option_table,"scene",
						&(modify_g_element_data->scene),
						g_element_command_data->scene_manager,set_Scene);
					/* select_mode */
					select_mode = GT_element_settings_get_select_mode(settings);
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
						&(settings->selected_material),
						g_element_command_data->graphical_material_manager,
						set_Graphical_material);
					/* size */
					Option_table_add_entry(option_table,"size",
						glyph_size,"*",set_special_float3);
					/* spectrum */
					Option_table_add_entry(option_table,"spectrum",
						&(settings->spectrum),g_element_command_data->spectrum_manager,
						set_Spectrum);
					/* variable_scale */
					set_variable_scale_field_data.computed_field_manager =
						g_element_command_data->computed_field_manager;
					set_variable_scale_field_data.conditional_function =
						Computed_field_has_up_to_3_numerical_components;
					set_variable_scale_field_data.conditional_function_user_data =
						(void *)NULL;
					Option_table_add_entry(option_table,"variable_scale",
						&variable_scale_field, &set_variable_scale_field_data,
						set_Computed_field_conditional);
					/* visible/invisible */
					Option_table_add_switch(option_table, "visible", "invisible", &visibility);
					if (return_code=Option_table_multi_parse(option_table,state))
					{
						if (settings->data_field&&!settings->spectrum)
						{
							settings->spectrum=ACCESS(Spectrum)(
								g_element_command_data->default_spectrum);
						}
						settings->visibility = visibility;
						if (font_name && (new_font = Graphics_font_package_get_font
								(g_element_command_data->graphics_font_package, font_name)))
						{
							REACCESS(Graphics_font)(&settings->font, new_font);
						}
						if (glyph)
						{
							STRING_TO_ENUMERATOR(Glyph_scaling_mode)(
								glyph_scaling_mode_string, &glyph_scaling_mode);
							GT_element_settings_set_glyph_parameters(settings,
								glyph, glyph_scaling_mode, glyph_centre, glyph_size,
								orientation_scale_field,glyph_scale_factors,
								variable_scale_field);
							GT_object_add_callback(settings->glyph,
								GT_element_settings_glyph_change, (void *)settings);
						}
						else
						{
							display_message(WARNING_MESSAGE,
								"No glyph specified for data_points");
							return_code=0;
						}
						STRING_TO_ENUMERATOR(Graphics_select_mode)(select_mode_string,
							&select_mode);
						GT_element_settings_set_select_mode(settings, select_mode);
					}
					DESTROY(Option_table)(&option_table);
					if (!return_code)
					{
						/* parse error, help */
						DEACCESS(GT_element_settings)(&(modify_g_element_data->settings));
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
						"gfx_modify_g_element_data_points.  Could not create settings");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_modify_g_element_data_points.  No modify data");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_modify_g_element_data_points.  "
				"Missing g_element_command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_g_element_data_points.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_g_element_data_points */

int gfx_modify_g_element_lines(struct Parse_state *state,
	void *modify_g_element_data_void,void *g_element_command_data_void)
/*******************************************************************************
LAST MODIFIED : 19 March 2001

DESCRIPTION :
Executes a GFX MODIFY G_ELEMENT LINES command.
If return_code is 1, returns the completed Modify_g_element_data with the
parsed settings. Note that the settings are ACCESSed once on valid return.
==============================================================================*/
{
	char *select_mode_string,**valid_strings;
	enum Graphics_select_mode select_mode;
	int number_of_valid_strings,return_code, visibility;
	struct Modify_g_element_data *modify_g_element_data;
	struct GT_element_settings *settings;
	struct G_element_command_data *g_element_command_data;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_data_field_data;

	ENTER(gfx_modify_g_element_lines);
	if (state)
	{
		if (g_element_command_data=(struct G_element_command_data *)
			g_element_command_data_void)
		{
			if (modify_g_element_data=
				(struct Modify_g_element_data *)modify_g_element_data_void)
			{
				/* create the gt_element_settings: */
				settings = CREATE(GT_element_settings)(GT_ELEMENT_SETTINGS_LINES);
				/* if we are specifying the name then copy the existings settings values so
					we can modify from these rather than the default */
				if (modify_g_element_data->settings)
				{
					if (GT_ELEMENT_SETTINGS_LINES ==modify_g_element_data->settings->settings_type)
					{
						GT_element_settings_copy_without_graphics_object(settings, modify_g_element_data->settings); 
					}
					DEACCESS(GT_element_settings)(&modify_g_element_data->settings);
				}
				if (settings)
				{
					/* access since deaccessed in gfx_modify_g_element */
					modify_g_element_data->settings = ACCESS(GT_element_settings)(settings);
					/* set essential parameters not set by CREATE function */
					if (!GT_element_settings_get_material(settings))
					{
						GT_element_settings_set_material(settings,
							g_element_command_data->default_material);
					}
					if (!GT_element_settings_get_selected_material(settings))
					{
						GT_element_settings_set_selected_material(settings,
							FIND_BY_IDENTIFIER_IN_MANAGER(Graphical_material,name)(
								"default_selected",
								g_element_command_data->graphical_material_manager));
					}
					visibility = settings->visibility;
					option_table=CREATE(Option_table)();
					/* as */
					Option_table_add_entry(option_table,"as",&(settings->name),
						(void *)1,set_name);
					/* coordinate */
					set_coordinate_field_data.computed_field_manager=
						g_element_command_data->computed_field_manager;
					set_coordinate_field_data.conditional_function=
						Computed_field_has_up_to_3_numerical_components;
					set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
					Option_table_add_entry(option_table,"coordinate",
						&(settings->coordinate_field),&set_coordinate_field_data,
						set_Computed_field_conditional);
					/* data */
					set_data_field_data.computed_field_manager=
						g_element_command_data->computed_field_manager;
					set_data_field_data.conditional_function=
						Computed_field_has_numerical_components;
					set_data_field_data.conditional_function_user_data=(void *)NULL;
					Option_table_add_entry(option_table,"data",&(settings->data_field),
						&set_data_field_data,set_Computed_field_conditional);
					/* delete */
					Option_table_add_entry(option_table,"delete",
						&(modify_g_element_data->delete_flag),NULL,set_char_flag);
					/* exterior */
					Option_table_add_entry(option_table,"exterior",&(settings->exterior),
						NULL,set_char_flag);
					/* face */
					Option_table_add_entry(option_table,"face",&(settings->face),
						NULL,set_exterior);
					/* line_width */
					Option_table_add_int_non_negative_entry(option_table,"line_width",
						&(settings->line_width));
					/* material */
					Option_table_add_entry(option_table,"material",&(settings->material),
						g_element_command_data->graphical_material_manager,
						set_Graphical_material);
					/* position */
					Option_table_add_entry(option_table,"position",
						&(modify_g_element_data->position),NULL,set_int_non_negative);
					/* scene */
					Option_table_add_entry(option_table,"scene",
						&(modify_g_element_data->scene),
						g_element_command_data->scene_manager,set_Scene);
					/* select_mode */
					select_mode = GT_element_settings_get_select_mode(settings);
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
						&(settings->selected_material),
						g_element_command_data->graphical_material_manager,
						set_Graphical_material);
					/* spectrum */
					Option_table_add_entry(option_table,"spectrum",
						&(settings->spectrum),g_element_command_data->spectrum_manager,
						set_Spectrum);
					/* visible/invisible */
					Option_table_add_switch(option_table, "visible", "invisible", &visibility);
					if (return_code=Option_table_multi_parse(option_table,state))
					{
						if (settings->data_field&&!settings->spectrum)
						{
							settings->spectrum=ACCESS(Spectrum)(
								g_element_command_data->default_spectrum);
						}
						settings->visibility = visibility;
						STRING_TO_ENUMERATOR(Graphics_select_mode)(select_mode_string,
							&select_mode);
						GT_element_settings_set_select_mode(settings, select_mode);
					}
					DESTROY(Option_table)(&option_table);
					if (!return_code)
					{
						/* parse error, help */
						DEACCESS(GT_element_settings)(&(modify_g_element_data->settings));
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_modify_g_element_lines.  Could not create settings");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_modify_g_element_lines.  No modify data");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_modify_g_element_lines.  "
				"Missing g_element_command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_modify_g_element_lines.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_g_element_lines */

int gfx_modify_g_element_cylinders(struct Parse_state *state,
	void *modify_g_element_data_void,void *g_element_command_data_void)
/*******************************************************************************
LAST MODIFIED : 11 October 2002

DESCRIPTION :
Executes a GFX MODIFY G_ELEMENT CYLINDERS command.
If return_code is 1, returns the completed Modify_g_element_data with the
parsed settings. Note that the settings are ACCESSed once on valid return.
==============================================================================*/
{
	char *select_mode_string,**valid_strings;
	enum Graphics_select_mode select_mode;
	int number_of_valid_strings,return_code, visibility;
	struct Modify_g_element_data *modify_g_element_data;
	struct GT_element_settings *settings;
	struct G_element_command_data *g_element_command_data;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_data_field_data, set_radius_scalar_field_data,
		set_texture_coordinate_field_data;

	ENTER(gfx_modify_g_element_cylinders);
	if (state)
	{
		if (g_element_command_data=(struct G_element_command_data *)
			g_element_command_data_void)
		{
			if (modify_g_element_data=
				(struct Modify_g_element_data *)modify_g_element_data_void)
			{
				/* create the gt_element_settings: */
				settings = CREATE(GT_element_settings)(GT_ELEMENT_SETTINGS_CYLINDERS);
				/* if we are specifying the name then copy the existings settings values so
					we can modify from these rather than the default */
				if (modify_g_element_data->settings)
				{
					if (GT_ELEMENT_SETTINGS_CYLINDERS ==modify_g_element_data->settings->settings_type)
					{
						GT_element_settings_copy_without_graphics_object(settings, modify_g_element_data->settings); 
					}
					DEACCESS(GT_element_settings)(&modify_g_element_data->settings);
				}
				if (settings)
				{
					/* access since deaccessed in gfx_modify_g_element */
					modify_g_element_data->settings = ACCESS(GT_element_settings)(settings);
					/* set essential parameters not set by CREATE function */
					if (!GT_element_settings_get_material(settings))
					{
						GT_element_settings_set_material(settings,
							g_element_command_data->default_material);
					}
					if (!GT_element_settings_get_selected_material(settings))
					{
						GT_element_settings_set_selected_material(settings,
							FIND_BY_IDENTIFIER_IN_MANAGER(Graphical_material,name)(
								"default_selected",
								g_element_command_data->graphical_material_manager));
					}
					visibility = settings->visibility;
					option_table=CREATE(Option_table)();
					/* as */
					Option_table_add_entry(option_table,"as",&(settings->name),
						(void *)1,set_name);
					/* constant_radius */
					Option_table_add_entry(option_table,"constant_radius",
						&(settings->constant_radius),NULL,set_float);
					/* coordinate */
					set_coordinate_field_data.computed_field_manager=
						g_element_command_data->computed_field_manager;
					set_coordinate_field_data.conditional_function=
						Computed_field_has_up_to_3_numerical_components;
					set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
					Option_table_add_entry(option_table,"coordinate",
						&(settings->coordinate_field),&set_coordinate_field_data,
						set_Computed_field_conditional);
					/* data */
					set_data_field_data.computed_field_manager=
						g_element_command_data->computed_field_manager;
					set_data_field_data.conditional_function=
						Computed_field_has_numerical_components;
					set_data_field_data.conditional_function_user_data=(void *)NULL;
					Option_table_add_entry(option_table,"data",&(settings->data_field),
						&set_data_field_data,set_Computed_field_conditional);
					/* delete */
					Option_table_add_entry(option_table,"delete",
						&(modify_g_element_data->delete_flag),NULL,set_char_flag);
					/* exterior */
					Option_table_add_entry(option_table,"exterior",&(settings->exterior),
						NULL,set_char_flag);
					/* face */
					Option_table_add_entry(option_table,"face",&(settings->face),
						NULL,set_exterior);
					/* material */
					Option_table_add_entry(option_table,"material",&(settings->material),
						g_element_command_data->graphical_material_manager,
						set_Graphical_material);
					/* position */
					Option_table_add_entry(option_table,"position",
						&(modify_g_element_data->position),NULL,set_int_non_negative);
					/* radius_scalar */
					set_radius_scalar_field_data.computed_field_manager=
						g_element_command_data->computed_field_manager;
					set_radius_scalar_field_data.conditional_function=
						Computed_field_is_scalar;
					set_radius_scalar_field_data.conditional_function_user_data=
						(void *)NULL;
					Option_table_add_entry(option_table,"radius_scalar",
						&(settings->radius_scalar_field),&set_radius_scalar_field_data,
						set_Computed_field_conditional);
					/* scale_factor */
					Option_table_add_entry(option_table,"scale_factor",
						&(settings->radius_scale_factor),NULL,set_float);
					/* scene */
					Option_table_add_entry(option_table,"scene",
						&(modify_g_element_data->scene),
						g_element_command_data->scene_manager,set_Scene);
					/* select_mode */
					select_mode = GT_element_settings_get_select_mode(settings);
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
						&(settings->selected_material),
						g_element_command_data->graphical_material_manager,
						set_Graphical_material);
					/* spectrum */
					Option_table_add_entry(option_table,"spectrum",
						&(settings->spectrum),g_element_command_data->spectrum_manager,
						set_Spectrum);
					/* texture_coordinates */
					set_texture_coordinate_field_data.computed_field_manager=
						g_element_command_data->computed_field_manager;
					set_texture_coordinate_field_data.conditional_function=
						Computed_field_has_up_to_3_numerical_components;
					set_texture_coordinate_field_data.conditional_function_user_data=
						(void *)NULL;
					Option_table_add_entry(option_table,"texture_coordinates",
						&(settings->texture_coordinate_field),
						&set_texture_coordinate_field_data,set_Computed_field_conditional);
					/* visible/invisible */
					Option_table_add_switch(option_table, "visible", "invisible", &visibility);
					if (return_code=Option_table_multi_parse(option_table,state))
					{
						if (settings->data_field&&!settings->spectrum)
						{
							settings->spectrum=ACCESS(Spectrum)(
								g_element_command_data->default_spectrum);
						}
						settings->visibility = visibility;
						STRING_TO_ENUMERATOR(Graphics_select_mode)(select_mode_string,
							&select_mode);
						GT_element_settings_set_select_mode(settings, select_mode);
					}
					DESTROY(Option_table)(&option_table);
					if (!return_code)
					{
						/* parse error, help */
						DEACCESS(GT_element_settings)(&(modify_g_element_data->settings));
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_modify_g_element_cylinders.  Could not create settings");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_modify_g_element_cylinders.  No modify data");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_modify_g_element_cylinders.  "
				"Missing g_element_command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_g_element_cylinders.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_g_element_cylinders */

int gfx_modify_g_element_surfaces(struct Parse_state *state,
	void *modify_g_element_data_void,void *g_element_command_data_void)
/*******************************************************************************
LAST MODIFIED : 19 March 2001

DESCRIPTION :
Executes a GFX MODIFY G_ELEMENT SURFACES command.
If return_code is 1, returns the completed Modify_g_element_data with the
parsed settings. Note that the settings are ACCESSed once on valid return.
==============================================================================*/
{
	char *render_type_string,*select_mode_string,**valid_strings;
	enum Graphics_select_mode select_mode;
	enum Render_type render_type;
	int number_of_valid_strings,return_code, visibility;
	struct Modify_g_element_data *modify_g_element_data;
	struct GT_element_settings *settings;
	struct G_element_command_data *g_element_command_data;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_data_field_data, set_texture_coordinate_field_data;

	ENTER(gfx_modify_g_element_surfaces);
	if (state)
	{
		if (g_element_command_data=(struct G_element_command_data *)
			g_element_command_data_void)
		{
			if (modify_g_element_data=
				(struct Modify_g_element_data *)modify_g_element_data_void)
			{
				/* create the gt_element_settings: */
				settings = CREATE(GT_element_settings)(GT_ELEMENT_SETTINGS_SURFACES);
				/* if we are specifying the name then copy the existings settings values so
					we can modify from these rather than the default */
				if (modify_g_element_data->settings)
				{
					if (GT_ELEMENT_SETTINGS_SURFACES ==modify_g_element_data->settings->settings_type)
					{
						GT_element_settings_copy_without_graphics_object(settings, modify_g_element_data->settings); 
					}
					DEACCESS(GT_element_settings)(&modify_g_element_data->settings);
				}
				if (settings)
				{
					/* access since deaccessed in gfx_modify_g_element */
					modify_g_element_data->settings = ACCESS(GT_element_settings)(settings);
					/* set essential parameters not set by CREATE function */
					if (!GT_element_settings_get_material(settings))
					{
						GT_element_settings_set_material(settings,
							g_element_command_data->default_material);
					}
					if (!GT_element_settings_get_selected_material(settings))
					{
						GT_element_settings_set_selected_material(settings,
							FIND_BY_IDENTIFIER_IN_MANAGER(Graphical_material,name)(
								"default_selected",
								g_element_command_data->graphical_material_manager));
					}
					visibility = settings->visibility;
					option_table=CREATE(Option_table)();
					/* as */
					Option_table_add_entry(option_table,"as",&(settings->name),
						(void *)1,set_name);
					/* coordinate */
					set_coordinate_field_data.computed_field_manager=
						g_element_command_data->computed_field_manager;
					set_coordinate_field_data.conditional_function=
						Computed_field_has_up_to_3_numerical_components;
					set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
					Option_table_add_entry(option_table,"coordinate",
						&(settings->coordinate_field),&set_coordinate_field_data,
						set_Computed_field_conditional);
					/* data */
					set_data_field_data.computed_field_manager=
						g_element_command_data->computed_field_manager;
					set_data_field_data.conditional_function=
						Computed_field_has_numerical_components;
					set_data_field_data.conditional_function_user_data=(void *)NULL;
					Option_table_add_entry(option_table,"data",&(settings->data_field),
						&set_data_field_data,set_Computed_field_conditional);
					/* delete */
					Option_table_add_entry(option_table,"delete",
						&(modify_g_element_data->delete_flag),NULL,set_char_flag);
					/* exterior */
					Option_table_add_entry(option_table,"exterior",&(settings->exterior),
						NULL,set_char_flag);
					/* face */
					Option_table_add_entry(option_table,"face",&(settings->face),
						NULL,set_exterior);
					/* material */
					Option_table_add_entry(option_table,"material",&(settings->material),
						g_element_command_data->graphical_material_manager,
						set_Graphical_material);
					/* position */
					Option_table_add_entry(option_table,"position",
						&(modify_g_element_data->position),NULL,set_int_non_negative);
					/* render_type */
					render_type = settings->render_type;
					render_type_string = ENUMERATOR_STRING(Render_type)(render_type);
					valid_strings = ENUMERATOR_GET_VALID_STRINGS(Render_type)(
						&number_of_valid_strings,
						(ENUMERATOR_CONDITIONAL_FUNCTION(Render_type) *)NULL, (void *)NULL);
					Option_table_add_enumerator(option_table,number_of_valid_strings,
						valid_strings,&render_type_string);
					DEALLOCATE(valid_strings);
					/* scene */
					Option_table_add_entry(option_table,"scene",
						&(modify_g_element_data->scene),
						g_element_command_data->scene_manager,set_Scene);
					/* select_mode */
					select_mode = GT_element_settings_get_select_mode(settings);
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
						&(settings->selected_material),
						g_element_command_data->graphical_material_manager,
						set_Graphical_material);
					/* spectrum */
					Option_table_add_entry(option_table,"spectrum",
						&(settings->spectrum),g_element_command_data->spectrum_manager,
						set_Spectrum);
					/* texture_coordinates */
					set_texture_coordinate_field_data.computed_field_manager=
						g_element_command_data->computed_field_manager;
					set_texture_coordinate_field_data.conditional_function=
						Computed_field_has_up_to_3_numerical_components;
					set_texture_coordinate_field_data.conditional_function_user_data=
						(void *)NULL;
					Option_table_add_entry(option_table,"texture_coordinates",
						&(settings->texture_coordinate_field),
						&set_texture_coordinate_field_data,set_Computed_field_conditional);
					/* visible/invisible */
					Option_table_add_switch(option_table, "visible", "invisible", &visibility);
					if (return_code=Option_table_multi_parse(option_table,state))
					{
						if (settings->data_field&&!settings->spectrum)
						{
							settings->spectrum=ACCESS(Spectrum)(
								g_element_command_data->default_spectrum);
						}
						settings->visibility = visibility;
						STRING_TO_ENUMERATOR(Graphics_select_mode)(select_mode_string,
							&select_mode);
						GT_element_settings_set_select_mode(settings, select_mode);
						STRING_TO_ENUMERATOR(Render_type)(render_type_string, &render_type);
						GT_element_settings_set_render_type(settings, render_type);
					}
					DESTROY(Option_table)(&option_table);
					if (!return_code)
					{
						/* parse error, help */
						DEACCESS(GT_element_settings)(&(modify_g_element_data->settings));
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_modify_g_element_surfaces.  Could not create settings");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_modify_g_element_surfaces.  No modify data");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_modify_g_element_surfaces.  "
				"Missing g_element_command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"gfx_modify_g_element_surfaces.  "
			"Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_g_element_surfaces */

int gfx_modify_g_element_iso_surfaces(struct Parse_state *state,
	void *modify_g_element_data_void,void *g_element_command_data_void)
/*******************************************************************************
LAST MODIFIED : 21 February 2005

DESCRIPTION :
Executes a GFX MODIFY G_ELEMENT ISO_SURFACES command.
If return_code is 1, returns the completed Modify_g_element_data with the
parsed settings. Note that the settings are ACCESSed once on valid return.
==============================================================================*/
{
	char *render_type_string,*select_mode_string, *use_element_type_string,
		**valid_strings;
	enum Graphics_select_mode select_mode;
	enum Render_type render_type;
	enum Use_element_type use_element_type;
	int number_of_valid_strings,return_code,visibility;
	struct Modify_g_element_data *modify_g_element_data;
	struct GT_element_settings *settings;
	struct G_element_command_data *g_element_command_data;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_data_field_data,set_iso_scalar_field_data,
		set_texture_coordinate_field_data;

	ENTER(gfx_modify_g_element_iso_surfaces);
	if (state)
	{
		if ((g_element_command_data=
			(struct G_element_command_data *)g_element_command_data_void)&&
			(computed_field_manager=g_element_command_data->computed_field_manager))
		{
			if (modify_g_element_data=
				(struct Modify_g_element_data *)modify_g_element_data_void)
			{
				/* create the gt_element_settings: */
				settings = CREATE(GT_element_settings)(GT_ELEMENT_SETTINGS_ISO_SURFACES);
				/* if we are specifying the name then copy the existings settings values so
					we can modify from these rather than the default */
				if (modify_g_element_data->settings)
				{
					if (GT_ELEMENT_SETTINGS_ISO_SURFACES ==modify_g_element_data->settings->settings_type)
					{
						GT_element_settings_copy_without_graphics_object(settings, modify_g_element_data->settings); 
					}
					DEACCESS(GT_element_settings)(&modify_g_element_data->settings);
				}
				if (settings)
				{
					/* access since deaccessed in gfx_modify_g_element */
					modify_g_element_data->settings = ACCESS(GT_element_settings)(settings);
					/* set essential parameters not set by CREATE function */
					if (!GT_element_settings_get_material(settings))
					{
						GT_element_settings_set_material(settings,
							g_element_command_data->default_material);
					}
					if (!GT_element_settings_get_selected_material(settings))
					{
						GT_element_settings_set_selected_material(settings,
							FIND_BY_IDENTIFIER_IN_MANAGER(Graphical_material,name)(
								"default_selected",
								g_element_command_data->graphical_material_manager));
					}
					/* must start with valid iso_scalar_field: */
					if (!settings->iso_scalar_field)
					{
						settings->iso_scalar_field=ACCESS(Computed_field)
							(FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
							Computed_field_is_scalar,(void *)NULL,computed_field_manager));
					}
					visibility = settings->visibility;
					option_table=CREATE(Option_table)();
					/* as */
					Option_table_add_entry(option_table,"as",&(settings->name),
						(void *)1,set_name);
					/* coordinate */
					set_coordinate_field_data.computed_field_manager=
						g_element_command_data->computed_field_manager;
					set_coordinate_field_data.conditional_function=
						Computed_field_has_up_to_3_numerical_components;
					set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
					Option_table_add_entry(option_table,"coordinate",
						&(settings->coordinate_field),&set_coordinate_field_data,
						set_Computed_field_conditional);
					/* data */
					set_data_field_data.computed_field_manager=
						g_element_command_data->computed_field_manager;
					set_data_field_data.conditional_function=
						Computed_field_has_numerical_components;
					set_data_field_data.conditional_function_user_data=(void *)NULL;
					Option_table_add_entry(option_table,"data",&(settings->data_field),
						&set_data_field_data,set_Computed_field_conditional);
					/* decimation_threshold */
					Option_table_add_double_entry(option_table, "decimation_threshold",
						&(settings->decimation_threshold));
					/* delete */
					Option_table_add_entry(option_table,"delete",
						&(modify_g_element_data->delete_flag),NULL,set_char_flag);
					/* exterior */
					Option_table_add_entry(option_table,"exterior",&(settings->exterior),
						NULL,set_char_flag);
					/* face */
					Option_table_add_entry(option_table,"face",&(settings->face),
						NULL,set_exterior);
					/* iso_scalar */
					set_iso_scalar_field_data.computed_field_manager=
						g_element_command_data->computed_field_manager;
					set_iso_scalar_field_data.conditional_function=
						Computed_field_is_scalar;
					set_iso_scalar_field_data.conditional_function_user_data=(void *)NULL;
					Option_table_add_entry(option_table,"iso_scalar",
						&(settings->iso_scalar_field),&set_iso_scalar_field_data,
						set_Computed_field_conditional);
					/* iso_values */
					Option_table_add_variable_length_double_vector_entry(option_table,
						"iso_values", &(settings->number_of_iso_values), &(settings->iso_values));
					/* material */
					Option_table_add_entry(option_table,"material",&(settings->material),
						g_element_command_data->graphical_material_manager,
						set_Graphical_material);
					/* position */
					Option_table_add_entry(option_table,"position",
						&(modify_g_element_data->position),NULL,set_int_non_negative);
					/* render_type */
					render_type = settings->render_type;
					render_type_string = ENUMERATOR_STRING(Render_type)(render_type);
					valid_strings = ENUMERATOR_GET_VALID_STRINGS(Render_type)(
						&number_of_valid_strings,
						(ENUMERATOR_CONDITIONAL_FUNCTION(Render_type) *)NULL, (void *)NULL);
					Option_table_add_enumerator(option_table,number_of_valid_strings,
						valid_strings,&render_type_string);
					DEALLOCATE(valid_strings);
					/* scene */
					Option_table_add_entry(option_table,"scene",
						&(modify_g_element_data->scene),
						g_element_command_data->scene_manager,set_Scene);
					/* select_mode */
					select_mode = GT_element_settings_get_select_mode(settings);
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
						&(settings->selected_material),
						g_element_command_data->graphical_material_manager,
						set_Graphical_material);
					/* spectrum */
					Option_table_add_entry(option_table,"spectrum",
						&(settings->spectrum),g_element_command_data->spectrum_manager,
						set_Spectrum);
					/* texture_coordinates */
					set_texture_coordinate_field_data.computed_field_manager=
						g_element_command_data->computed_field_manager;
					set_texture_coordinate_field_data.conditional_function=
						Computed_field_has_numerical_components;
					set_texture_coordinate_field_data.conditional_function_user_data=
						(void *)NULL;
					Option_table_add_entry(option_table,"texture_coordinates",
						&(settings->texture_coordinate_field),
						&set_texture_coordinate_field_data,set_Computed_field_conditional);
					/* use_elements/use_faces/use_lines */
					use_element_type = settings->use_element_type;
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
					if (return_code=Option_table_multi_parse(option_table,state))
					{
						if ((struct Computed_field *)NULL==settings->iso_scalar_field)
						{
							display_message(ERROR_MESSAGE,
								"gfx_modify_g_element_iso_surfaces.  Missing iso_scalar field");
							return_code=0;
						}
						STRING_TO_ENUMERATOR(Use_element_type)(use_element_type_string,
							&use_element_type);
						GT_element_settings_set_use_element_type(settings,
							use_element_type);
						if (settings->data_field&&!settings->spectrum)
						{
							settings->spectrum=ACCESS(Spectrum)(
								g_element_command_data->default_spectrum);
						}
						settings->visibility = visibility;
						STRING_TO_ENUMERATOR(Graphics_select_mode)(select_mode_string,
							&select_mode);
						GT_element_settings_set_select_mode(settings, select_mode);
						STRING_TO_ENUMERATOR(Render_type)(render_type_string, &render_type);
						GT_element_settings_set_render_type(settings, render_type);
					}
					DESTROY(Option_table)(&option_table);
					if (!return_code)
					{
						/* parse error, help */
						DEACCESS(GT_element_settings)(&(modify_g_element_data->settings));
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_modify_g_element_iso_surfaces.  Could not create settings");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"gfx_modify_g_element_iso_surfaces.  No modify data");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"gfx_modify_g_element_iso_surfaces.  "
				"Missing g_element_command_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_g_element_iso_surfaces.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_g_element_iso_surfaces */

int gfx_modify_g_element_element_points(struct Parse_state *state,
	void *modify_g_element_data_void,void *g_element_command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 March 2003

DESCRIPTION :
Executes a GFX MODIFY G_ELEMENT ELEMENT_POINTS command.
If return_code is 1, returns the completed Modify_g_element_data with the
parsed settings. Note that the settings are ACCESSed once on valid return.
==============================================================================*/
{
	char *font_name, *glyph_scaling_mode_string,  *select_mode_string,
		*use_element_type_string,	**valid_strings, *xi_discretization_mode_string;
	enum Glyph_scaling_mode glyph_scaling_mode;
	enum Graphics_select_mode select_mode;
	enum Use_element_type use_element_type;
	enum Xi_discretization_mode xi_discretization_mode;
	int number_of_components,number_of_valid_strings,return_code,visibility;
	struct Computed_field *orientation_scale_field, *variable_scale_field,
		*xi_point_density_field;
	struct FE_region *fe_region;
	struct Graphics_font *new_font;
	struct GT_element_settings *settings;
	struct GT_object *glyph;
	struct G_element_command_data *g_element_command_data;
	struct Modify_g_element_data *modify_g_element_data;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_data_field_data, set_label_field_data, set_orientation_scale_field_data,
		set_variable_scale_field_data, set_xi_point_density_field_data;
	Triple glyph_centre, glyph_scale_factors, glyph_size;

	ENTER(gfx_modify_g_element_element_points);
	if (state && (g_element_command_data=(struct G_element_command_data *)
		g_element_command_data_void) &&
		(modify_g_element_data=
			(struct Modify_g_element_data *)modify_g_element_data_void) &&
		(fe_region = Cmiss_region_get_FE_region(g_element_command_data->region)))
	{
		/* create the gt_element_settings: */
		settings = CREATE(GT_element_settings)(GT_ELEMENT_SETTINGS_ELEMENT_POINTS);
		/* if we are specifying the name then copy the existings settings values so
			we can modify from these rather than the default */
		if (modify_g_element_data->settings)
		{
			if (GT_ELEMENT_SETTINGS_ELEMENT_POINTS ==modify_g_element_data->settings->settings_type)
			{
				GT_element_settings_copy_without_graphics_object(settings, modify_g_element_data->settings); 
			}
			DEACCESS(GT_element_settings)(&modify_g_element_data->settings);
		}
		if (settings)
		{
			/* access since deaccessed in gfx_modify_g_element */
			modify_g_element_data->settings = ACCESS(GT_element_settings)(settings);
			/* set essential parameters not set by CREATE function */
			if (!GT_element_settings_get_material(settings))
			{
				GT_element_settings_set_material(settings,
					g_element_command_data->default_material);
			}
			if (!GT_element_settings_get_selected_material(settings))
			{
				GT_element_settings_set_selected_material(settings,
					FIND_BY_IDENTIFIER_IN_MANAGER(Graphical_material,name)(
						"default_selected",
						g_element_command_data->graphical_material_manager));
			}
			if (!settings->font)
			{
				settings->font = ACCESS(Graphics_font)(
					g_element_command_data->default_font);
			}
			font_name = (char *)NULL;
			orientation_scale_field = (struct Computed_field *)NULL;
			variable_scale_field = (struct Computed_field *)NULL;
			xi_point_density_field = (struct Computed_field *)NULL;
			GT_element_settings_get_glyph_parameters(settings,
				&glyph, &glyph_scaling_mode, glyph_centre, glyph_size,
				&orientation_scale_field, glyph_scale_factors,
				&variable_scale_field);
			/* default to point glyph for fastest possible display */
			if (!glyph)
			{
				glyph = FIND_BY_IDENTIFIER_IN_LIST(GT_object,name)("point",
					g_element_command_data->glyph_list);
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
			GT_element_settings_get_xi_discretization(settings,
				&xi_discretization_mode, &xi_point_density_field);
			if (xi_point_density_field)
			{
				ACCESS(Computed_field)(xi_point_density_field);
			}
			number_of_components = 3;
			visibility = settings->visibility;

			option_table=CREATE(Option_table)();
			/* as */
			Option_table_add_entry(option_table,"as",&(settings->name),
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
				g_element_command_data->computed_field_manager;
			set_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"coordinate",
				&(settings->coordinate_field),&set_coordinate_field_data,
				set_Computed_field_conditional);
			/* data */
			set_data_field_data.computed_field_manager=
				g_element_command_data->computed_field_manager;
			set_data_field_data.conditional_function=
				Computed_field_has_numerical_components;
			set_data_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"data",&(settings->data_field),
				&set_data_field_data,set_Computed_field_conditional);
			/* delete */
			Option_table_add_entry(option_table,"delete",
				&(modify_g_element_data->delete_flag),NULL,set_char_flag);
			/* density */
			set_xi_point_density_field_data.computed_field_manager =
				g_element_command_data->computed_field_manager;
			set_xi_point_density_field_data.conditional_function =
				Computed_field_is_scalar;
			set_xi_point_density_field_data.conditional_function_user_data =
				(void *)NULL;
			Option_table_add_entry(option_table, "density",
				&xi_point_density_field, &set_xi_point_density_field_data,
				set_Computed_field_conditional);
			/* discretization */
			Option_table_add_entry(option_table,"discretization",
				&(settings->discretization),g_element_command_data->user_interface,
				set_Element_discretization);
			/* exterior */
			Option_table_add_entry(option_table,"exterior",&(settings->exterior),
				NULL,set_char_flag);
			/* face */
			Option_table_add_entry(option_table,"face",&(settings->face),
				NULL,set_exterior);
			/* font */
			Option_table_add_name_entry(option_table, "font",
				&font_name);
			/* glyph */
			Option_table_add_entry(option_table,"glyph",&glyph,
				g_element_command_data->glyph_list,set_Graphics_object);
			/* label */
			set_label_field_data.computed_field_manager=
				g_element_command_data->computed_field_manager;
			set_label_field_data.conditional_function=
				(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
			set_label_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"label",&(settings->label_field),
				&set_label_field_data,set_Computed_field_conditional);
			/* material */
			Option_table_add_entry(option_table,"material",&(settings->material),
				g_element_command_data->graphical_material_manager,
				set_Graphical_material);
			/* native_discretization */
			Option_table_add_set_FE_field_from_FE_region(option_table,
				"native_discretization", &(settings->native_discretization_field),
				fe_region);
			/* glyph scaling mode */
			glyph_scaling_mode_string =
				ENUMERATOR_STRING(Glyph_scaling_mode)(glyph_scaling_mode);
			valid_strings = ENUMERATOR_GET_VALID_STRINGS(Glyph_scaling_mode)(
				&number_of_valid_strings,
				(ENUMERATOR_CONDITIONAL_FUNCTION(Glyph_scaling_mode) *)NULL,
				(void *)NULL);
			Option_table_add_enumerator(option_table, number_of_valid_strings,
				valid_strings, &glyph_scaling_mode_string);
			DEALLOCATE(valid_strings);
			/* orientation */
			set_orientation_scale_field_data.computed_field_manager=
				g_element_command_data->computed_field_manager;
			set_orientation_scale_field_data.conditional_function=
				Computed_field_is_orientation_scale_capable;
			set_orientation_scale_field_data.conditional_function_user_data=
				(void *)NULL;
			Option_table_add_entry(option_table,"orientation",
				&orientation_scale_field,&set_orientation_scale_field_data,
				set_Computed_field_conditional);
			/* position */
			Option_table_add_entry(option_table,"position",
				&(modify_g_element_data->position),NULL,set_int_non_negative);
			/* scale_factors */
			Option_table_add_entry(option_table,"scale_factors",
				glyph_scale_factors,"*",set_special_float3);
			/* scene */
			Option_table_add_entry(option_table,"scene",
				&(modify_g_element_data->scene),
				g_element_command_data->scene_manager,set_Scene);
			/* select_mode */
			select_mode = GT_element_settings_get_select_mode(settings);
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
				&(settings->selected_material),
				g_element_command_data->graphical_material_manager,
				set_Graphical_material);
			/* size */
			Option_table_add_entry(option_table,"size",
				glyph_size,"*",set_special_float3);
			/* spectrum */
			Option_table_add_entry(option_table,"spectrum",
				&(settings->spectrum),g_element_command_data->spectrum_manager,
				set_Spectrum);
			/* use_elements/use_faces/use_lines */
			use_element_type = settings->use_element_type;
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
				settings->seed_xi,&number_of_components,set_float_vector);
			/* variable_scale */
			set_variable_scale_field_data.computed_field_manager =
				g_element_command_data->computed_field_manager;
			set_variable_scale_field_data.conditional_function =
				Computed_field_has_up_to_3_numerical_components;
			set_variable_scale_field_data.conditional_function_user_data =
				(void *)NULL;
			Option_table_add_entry(option_table,"variable_scale",
				&variable_scale_field, &set_variable_scale_field_data,
				set_Computed_field_conditional);
			/* visible/invisible */
			Option_table_add_switch(option_table, "visible", "invisible", &visibility);
			if (return_code=Option_table_multi_parse(option_table,state))
			{
				if (settings->data_field&&!settings->spectrum)
				{
					settings->spectrum=ACCESS(Spectrum)(
						g_element_command_data->default_spectrum);
				}
				settings->visibility = visibility;
				STRING_TO_ENUMERATOR(Xi_discretization_mode)(
					xi_discretization_mode_string, &xi_discretization_mode);
				if (((XI_DISCRETIZATION_CELL_DENSITY != xi_discretization_mode) &&
					(XI_DISCRETIZATION_CELL_POISSON != xi_discretization_mode)) ||
					xi_point_density_field)
				{
					GT_element_settings_set_xi_discretization(settings,
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
				GT_element_settings_set_use_element_type(settings,
					use_element_type);
				if (font_name && (new_font = Graphics_font_package_get_font
						(g_element_command_data->graphics_font_package, font_name)))
				{
					REACCESS(Graphics_font)(&settings->font, new_font);
				}
				if (font_name)
				{
					DEALLOCATE(font_name);
				}
				if (glyph)
				{
					STRING_TO_ENUMERATOR(Glyph_scaling_mode)(
						glyph_scaling_mode_string, &glyph_scaling_mode);
					GT_element_settings_set_glyph_parameters(settings,
						glyph, glyph_scaling_mode, glyph_centre, glyph_size,
						orientation_scale_field,glyph_scale_factors,
						variable_scale_field);
					GT_object_add_callback(settings->glyph,
						GT_element_settings_glyph_change, (void *)settings);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"No glyph specified for element_points");
					return_code=0;
				}
				STRING_TO_ENUMERATOR(Graphics_select_mode)(select_mode_string,
					&select_mode);
				GT_element_settings_set_select_mode(settings, select_mode);
			}
			DESTROY(Option_table)(&option_table);
			if (!return_code)
			{
				/* parse error, help */
				DEACCESS(GT_element_settings)(&(modify_g_element_data->settings));
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
				"gfx_modify_g_element_element_points.  Could not create settings");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_g_element_element_points.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_g_element_element_points */

int gfx_modify_g_element_streamlines(struct Parse_state *state,
	void *modify_g_element_data_void,void *g_element_command_data_void)
/*******************************************************************************
LAST MODIFIED : 14 March 2003

DESCRIPTION :
Executes a GFX MODIFY G_ELEMENT STREAMLINES command.
If return_code is 1, returns the completed Modify_g_element_data with the
parsed settings. Note that the settings are ACCESSed once on valid return.
==============================================================================*/
{
	char reverse_track,*select_mode_string,
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
	struct GT_element_settings *settings;
	struct G_element_command_data *g_element_command_data;
	struct Modify_g_element_data *modify_g_element_data;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_coordinate_field_data,
		set_data_field_data, set_seed_node_coordinate_field_data,
		set_stream_vector_field_data;

	ENTER(gfx_modify_g_element_streamlines);
	if (state && (g_element_command_data =
		(struct G_element_command_data *)g_element_command_data_void) &&
		(modify_g_element_data =
			(struct Modify_g_element_data *)modify_g_element_data_void) &&
		(fe_region = Cmiss_region_get_FE_region(g_element_command_data->region)))
	{
		/* create the gt_element_settings: */
		settings = CREATE(GT_element_settings)(GT_ELEMENT_SETTINGS_STREAMLINES);
		/* if we are specifying the name then copy the existings settings values so
			we can modify from these rather than the default */
		if (modify_g_element_data->settings)
		{
			if (GT_ELEMENT_SETTINGS_STREAMLINES ==modify_g_element_data->settings->settings_type)
			{
				GT_element_settings_copy_without_graphics_object(settings, modify_g_element_data->settings); 
			}
			DEACCESS(GT_element_settings)(&modify_g_element_data->settings);
		}
		if (settings)
		{
			/* access since deaccessed in gfx_modify_g_element */
			modify_g_element_data->settings = ACCESS(GT_element_settings)(settings);
			/* set essential parameters not set by CREATE function */
			if (!GT_element_settings_get_material(settings))
			{
				GT_element_settings_set_material(settings,
					g_element_command_data->default_material);
			}
			if (!GT_element_settings_get_selected_material(settings))
			{
				GT_element_settings_set_selected_material(settings,
					FIND_BY_IDENTIFIER_IN_MANAGER(Graphical_material,name)(
						"default_selected",
						g_element_command_data->graphical_material_manager));
			}
			/* The value stored in the settings is an integer rather than a char */
			if (settings->reverse_track)
			{
				reverse_track = 1;
			}
			else
			{
				reverse_track = 0;
			}
			visibility = settings->visibility;
			option_table=CREATE(Option_table)();
			/* as */
			Option_table_add_entry(option_table,"as",&(settings->name),
				(void *)1,set_name);
			/* coordinate */
			set_coordinate_field_data.computed_field_manager=
				g_element_command_data->computed_field_manager;
			set_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_coordinate_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"coordinate",
				&(settings->coordinate_field),&set_coordinate_field_data,
				set_Computed_field_conditional);
			/* data */
			set_data_field_data.computed_field_manager=
				g_element_command_data->computed_field_manager;
			set_data_field_data.conditional_function=
				Computed_field_has_numerical_components;
			set_data_field_data.conditional_function_user_data=(void *)NULL;
			Option_table_add_entry(option_table,"data",&(settings->data_field),
				&set_data_field_data,set_Computed_field_conditional);
			/* delete */
			Option_table_add_entry(option_table,"delete",
				&(modify_g_element_data->delete_flag),NULL,set_char_flag);
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
				&(settings->streamline_length),NULL,set_float);
			/* material */
			Option_table_add_entry(option_table,"material",&(settings->material),
				g_element_command_data->graphical_material_manager,
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
				&(modify_g_element_data->position),NULL,set_int_non_negative);
			/* reverse */
			/*???RC use negative length to denote reverse track instead? */
			Option_table_add_entry(option_table,"reverse_track",
				&reverse_track,NULL,set_char_flag);
			/* scene */
			Option_table_add_entry(option_table,"scene",
				&(modify_g_element_data->scene),
				g_element_command_data->scene_manager,set_Scene);
			/* seed_element */
			Option_table_add_entry(option_table, "seed_element",
				&(settings->seed_element), fe_region,
				set_FE_element_top_level_FE_region);
			/* seed_node_region */
			Option_table_add_entry(option_table, "seed_node_region",
				&settings->seed_node_region_path,
				g_element_command_data->root_region, set_Cmiss_region_path);
			/* seed_node_coordinate_field */
			set_seed_node_coordinate_field_data.computed_field_manager=
				g_element_command_data->computed_field_manager;
			set_seed_node_coordinate_field_data.conditional_function=
				Computed_field_has_up_to_3_numerical_components;
			set_seed_node_coordinate_field_data.conditional_function_user_data=
				(void *)NULL;
			Option_table_add_entry(option_table,"seed_node_coordinate_field",
				&(settings->seed_node_coordinate_field),&set_seed_node_coordinate_field_data,
				set_Computed_field_conditional);
			/* select_mode */
			select_mode = GT_element_settings_get_select_mode(settings);
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
				&(settings->selected_material),
				g_element_command_data->graphical_material_manager,
				set_Graphical_material);
			/* spectrum */
			Option_table_add_entry(option_table,"spectrum",
				&(settings->spectrum),g_element_command_data->spectrum_manager,
				set_Spectrum);
			/* vector */
			set_stream_vector_field_data.computed_field_manager=
				g_element_command_data->computed_field_manager;
			set_stream_vector_field_data.conditional_function=
				Computed_field_is_stream_vector_capable;
			set_stream_vector_field_data.conditional_function_user_data=
				(void *)NULL;
			Option_table_add_entry(option_table,"vector",
				&(settings->stream_vector_field),&set_stream_vector_field_data,
				set_Computed_field_conditional);
			/* width */
			Option_table_add_entry(option_table,"width",
				&(settings->streamline_width),NULL,set_float);
			/* visible/invisible */
			Option_table_add_switch(option_table, "visible", "invisible", &visibility);
			/* xi */
			number_of_components = 3;
			Option_table_add_entry(option_table,"xi",
				settings->seed_xi,&number_of_components,set_float_vector);
			if (return_code = Option_table_multi_parse(option_table,state))
			{
				settings->visibility = visibility;
				if (!(settings->stream_vector_field))
				{
					display_message(ERROR_MESSAGE,"Must specify a vector");
					return_code=0;
				}
				if (settings->seed_node_region_path)
				{
					if (!(Cmiss_region_get_region_from_path(
						g_element_command_data->root_region,
						settings->seed_node_region_path, &seed_node_region) &&
						REACCESS(FE_region)(&settings->seed_node_region,
						Cmiss_region_get_FE_region(seed_node_region))))
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_streamlines.  Invalid seed_node_region");
						return_code = 0;
					}
				}
				if ((settings->seed_node_coordinate_field && (!settings->seed_node_region)) ||
					((!settings->seed_node_coordinate_field) && settings->seed_node_region))
				{
					display_message(ERROR_MESSAGE,
						"If you specify a seed_node_coordinate_field then you must also specify a "
						"seed_node_region");
					return_code = 0;
				}
				if (return_code)
				{
					GT_element_settings_get_streamline_parameters(settings,
						&streamline_type,&stream_vector_field,&reverse_track_int,
						&length,&width);
					STRING_TO_ENUMERATOR(Streamline_type)(streamline_type_string,
						&streamline_type);
					STRING_TO_ENUMERATOR(Streamline_data_type)(
						streamline_data_type_string, &streamline_data_type);
					if (settings->data_field)
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
					if ((STREAM_NO_DATA!=streamline_data_type)&&!settings->spectrum)
					{
						settings->spectrum=ACCESS(Spectrum)(
							g_element_command_data->default_spectrum);
					}
					GT_element_settings_set_streamline_parameters(
						settings,streamline_type,stream_vector_field,(int)reverse_track,
						length,width);
					GT_element_settings_set_data_spectrum_parameters_streamlines(
						settings,streamline_data_type,settings->data_field,
						settings->spectrum);
				}
				STRING_TO_ENUMERATOR(Graphics_select_mode)(select_mode_string,
					&select_mode);
				GT_element_settings_set_select_mode(settings, select_mode);
			}
			DESTROY(Option_table)(&option_table);
			if (!return_code)
			{
				/* parse error, help */
				DEACCESS(GT_element_settings)(&(modify_g_element_data->settings));
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"gfx_modify_g_element_streamlines.  Could not create settings");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_g_element_streamlines.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_g_element_streamlines */
