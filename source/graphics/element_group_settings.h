/*******************************************************************************
FILE : element_group_settings.h

LAST MODIFIED : 21 February 2005

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
#if !defined (ELEMENT_GROUP_SETTINGS_H)
#define ELEMENT_GROUP_SETTINGS_H

#include "computed_field/computed_field.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_to_graphics_object.h"
#include "graphics/auxiliary_graphics_types.h"
#include "graphics/element_point_ranges.h"
#include "graphics/graphics_object.h"
#include "general/enumerator.h"
#include "general/list.h"
#include "graphics/material.h"
#include "graphics/spectrum.h"
#include "graphics/volume_texture.h"

/*
Global types
------------
*/
/*???RC Forward declare struct GT_object to cope with circular inclusion
	difficulties */
struct GT_object;
struct GT_element_settings;

enum GT_element_settings_type
/*******************************************************************************
LAST MODIFIED : 20 March 2001

DESCRIPTION :
Note: the first value will be 0 by the ANSI standard, with each subsequent entry
incremented by 1. This pattern is expected by the ENUMERATOR macros.
Must ensure the ENUMERATOR_STRING function returns a string for each value here.
==============================================================================*/
{
	GT_ELEMENT_SETTINGS_NODE_POINTS,
	GT_ELEMENT_SETTINGS_DATA_POINTS,
	GT_ELEMENT_SETTINGS_LINES,
	GT_ELEMENT_SETTINGS_CYLINDERS,
	GT_ELEMENT_SETTINGS_SURFACES,
	GT_ELEMENT_SETTINGS_ISO_SURFACES,
	GT_ELEMENT_SETTINGS_ELEMENT_POINTS,
	GT_ELEMENT_SETTINGS_STREAMLINES,
	GT_ELEMENT_SETTINGS_VOLUMES /* Must be last as disabled in
		 WX_USER_INTERFACE, see 
		 PROTOTYPE_ENUMERATOR_STRING_FUNCTION(GT_element_settings_type) */
}; /* enum GT_element_settings_type */

enum Glyph_scaling_mode
/*******************************************************************************
LAST MODIFIED : 20 March 2001

DESCRIPTION :
Enumerator controlling how glyph orientation and scaling parameters are set.
Modes other than GENERAL restrict the options available and their
interpretation. For example, in GENERAL mode the base size can be a*b*c, but in
VECTOR mode it is restricted to be 0*a*a.
Note: the first value will be 0 by the ANSI standard, with each subsequent entry
incremented by 1. This pattern is expected by the ENUMERATOR macros.
Must ensure the ENUMERATOR_STRING function returns a string for each value here.
==============================================================================*/
{
	GLYPH_SCALING_CONSTANT,
	GLYPH_SCALING_SCALAR,
	GLYPH_SCALING_VECTOR,
	GLYPH_SCALING_AXES,
	GLYPH_SCALING_GENERAL
}; /* enum Glyph_scaling_mode */

enum GT_element_settings_string_details
/*******************************************************************************
LAST MODIFIED : 25 June 1998

DESCRIPTION :
Parameter for selecting detail included by GT_element_settings_string:
SETTINGS_STRING_GEOMETRY = only those settings that control the geometry;
SETTINGS_STRING_COMPLETE = all settings including appearance.
SETTINGS_STRING_COMPLETE_PLUS = as above but preceded by position too.
==============================================================================*/
{
	SETTINGS_STRING_GEOMETRY,
	SETTINGS_STRING_COMPLETE,
	SETTINGS_STRING_COMPLETE_PLUS
}; /* enum GT_element_settings_string_details */

struct GT_element_settings_list_data
/*******************************************************************************
LAST MODIFIED : 22 July 1998

DESCRIPTION :
Data for formating output with GT_element_settings_list_contents function.
==============================================================================*/
{
	const char *line_prefix,*line_suffix;
	enum GT_element_settings_string_details settings_string_detail;
}; /* GT_element_settings_list_data */

struct Modify_g_element_data
/*******************************************************************************
LAST MODIFIED : 5 June 1998

DESCRIPTION :
Structure modified by g_element modify routines.
==============================================================================*/
{
	struct Scene *scene;
	char delete_flag;
	int position;
	struct GT_element_settings *settings;
}; /* struct Modify_g_element_data */

struct G_element_command_data
/*******************************************************************************
LAST MODIFIED : 6 March 2003

DESCRIPTION :
Subset of command data passed to g_element modify routines.
==============================================================================*/
{
	struct Graphical_material *default_material;
	struct Graphics_font *default_font;
	struct Graphics_font_package *graphics_font_package;
	struct LIST(GT_object *) glyph_list;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Cmiss_region *region;
	/* root_region used for seeding streamlines from the nodes in a region */
	struct Cmiss_region *root_region;
	struct MANAGER(Graphical_material) *graphical_material_manager;
	struct MANAGER(Scene) *scene_manager;
	struct Spectrum *default_spectrum;
	struct MANAGER(Spectrum) *spectrum_manager;
	struct MANAGER(Texture) *texture_manager;
	struct MANAGER(VT_volume_texture) *volume_texture_manager;
	struct User_interface *user_interface;
}; /* struct G_element_command_data */

struct GT_element_settings_to_graphics_object_data
/*******************************************************************************
LAST MODIFIED : 30 April 2003

DESCRIPTION :
Data required to produce or edit a graphics object for a GT_element_settings
object.
==============================================================================*/
{
	/* graphics object names are preceded by this */
	const char *name_prefix;
	/* default_rc_coordinate_field to use if NULL in any settings */
	struct Computed_field *rc_coordinate_field,*default_rc_coordinate_field,
		*wrapper_orientation_scale_field,*wrapper_stream_vector_field;
	struct Cmiss_region *region;
	struct FE_region *fe_region;
	struct FE_region *data_fe_region; /*???RC temporary */
	struct Element_discretization *element_discretization;
	struct FE_field *native_discretization_field;
	int circle_discretization;
	FE_value time;
	/* flag indicating that graphics_objects be built for all visible settings
		 currently without them */
	int build_graphics;
	/* graphics object in which existing graphics that do not need rebuilding
		 are stored. They are transferred from the settings->graphics_object and
		 matched by object_name. Note that the update of existing graphics
		 relies on them being stored in the same order as the new additions.
		 Set to NULL to turn off */
	struct GT_object *existing_graphics;
	/* for highlighting of selected objects */
	struct LIST(Element_point_ranges) *selected_element_point_ranges_list;
	struct LIST(FE_element) *selected_element_list;
	struct LIST(FE_node) *selected_data_list,*selected_node_list;
	
	/** The number of components in the data field */
	int number_of_data_values;
	/** A buffer allocated large enough for evaluating the data field */
	FE_value *data_copy_buffer;

	/* additional values for passing to element_to_graphics_object */
	struct GT_element_settings *settings;
};

struct GT_element_settings_Computed_field_change_data
/*******************************************************************************
LAST MODIFIED : 14 March 2003

DESCRIPTION :
User data for function GT_element_settings_computed_field_change.
==============================================================================*/
{
	FE_value time;
	/* rebuild_graphics flag should start at 0, and is set if any graphics
		 objects must be rebuilt as a result of this change */
	int graphics_changed;
	/* the default coordinate field for the whole GT_element_group for when
		 not overridden by settings->coordinate_field */
	struct Computed_field *default_coordinate_field;
	/* the list of fields that have changed
		 before passing, enlarge to include all other fields that depend on them */
	struct LIST(Computed_field) *changed_field_list;
}; /* struct GT_element_settings_Computed_field_change_data */

struct GT_element_settings_nearest_node_data
/*******************************************************************************
LAST MODIFIED : 15 February 2000

DESCRIPTION :
Structure for passing to GT_element_settings_get_nearest_node iterator.
==============================================================================*/
{
	/* centre must be in the coordinate space of the fields in the settings */
	FE_value centre[3],distance;
	/* flag indicating if the end of a node vector was closest, not position */
	int edit_vector;
	struct Computed_field *default_coordinate_field;
	struct FE_node *node;
	struct GT_element_settings *settings;
	struct LIST(FE_node) *node_list;
}; /* struct GT_element_settings_get_nearest_node_data */

struct GT_element_settings_Graphical_material_change_data
/*******************************************************************************
LAST MODIFIED : 7 June 2001

DESCRIPTION :
Data to pass to GT_element_settings_Graphical_material_change.
==============================================================================*/
{
	struct LIST(Graphical_material) *changed_material_list;
	/* flag indicating if the GT_element_group rendition has changed */
	int changed;
};

struct GT_element_settings_Spectrum_change_data
/*******************************************************************************
LAST MODIFIED : 7 June 2001

DESCRIPTION :
Data to pass to GT_element_settings_Spectrum_change.
==============================================================================*/
{
	struct LIST(Spectrum) *changed_spectrum_list;
	/* flag indicating if the GT_element_group rendition has changed */
	int changed;
};

struct GT_element_settings_Texture_change_data
/*******************************************************************************
LAST MODIFIED : 7 June 2001

DESCRIPTION :
Data to pass to GT_element_settings_Texture_change.
==============================================================================*/
{
	struct LIST(Texture) *changed_texture_list;
	/* flag indicating if the GT_element_group rendition has changed */
	int changed;
};

struct GT_element_settings_update_time_behaviour_data
/*******************************************************************************
LAST MODIFIED : 6 December 2001

DESCRIPTION :
Data to pass to GT_element_settings_update_time_behaviour.
==============================================================================*/
{
	/* flag set by GT_element group if the default coordinate field depends on time */
	int default_coordinate_depends_on_time;
	/* flag set by settings if any of the settings depend on time */
	int time_dependent;
};

/*
Global functions
----------------
*/

PROTOTYPE_ENUMERATOR_FUNCTIONS(GT_element_settings_type);

int GT_element_settings_type_uses_dimension(
	enum GT_element_settings_type settings_type, int dimension);
/*******************************************************************************
LAST MODIFIED : 20 March 2001

DESCRIPTION :
Returns true if the particular <settings_type> can deal with nodes/elements of
the given <dimension>. Note a <dimension> of -1 is taken to mean any dimension.
==============================================================================*/

int GT_element_settings_type_uses_dimension_conditional(
	enum GT_element_settings_type settings_type, void *dimension_address_void);
/*******************************************************************************
LAST MODIFIED : 20 March 2001

DESCRIPTION :
Calls GT_element_settings_type_uses_dimension for the <settings_type> and
integer dimension pointed to by <dimension_address_void>.
==============================================================================*/

PROTOTYPE_ENUMERATOR_FUNCTIONS(Glyph_scaling_mode);

PROTOTYPE_OBJECT_FUNCTIONS(GT_element_settings);
DECLARE_LIST_TYPES(GT_element_settings);

struct GT_element_settings *CREATE(GT_element_settings)(
	enum GT_element_settings_type);
/*******************************************************************************
LAST MODIFIED : 2 July 1997

DESCRIPTION :
Allocates memory and assigns fields for a struct GT_element_settings.
==============================================================================*/

int DESTROY(GT_element_settings)(struct GT_element_settings **settings_ptr);
/*******************************************************************************
LAST MODIFIED : 2 July 1997

DESCRIPTION :
Frees the memory for the fields of <**settings_ptr>, frees the memory for
<**settings_ptr> and sets <*settings_ptr> to NULL.
==============================================================================*/

int GT_element_settings_copy_without_graphics_object(
	struct GT_element_settings *destination, struct GT_element_settings *source);
/*******************************************************************************
LAST MODIFIED : 2 April 2003

DESCRIPTION :
Copies the GT_element contents from source to destination.
Notes:
destination->access_count is not changed by COPY.
graphics_object is NOT copied; destination->graphics_object is cleared.
==============================================================================*/

PROTOTYPE_LIST_FUNCTIONS(GT_element_settings);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(GT_element_settings,position,int);

int GET_NAME(GT_element_settings)(struct GT_element_settings *object,
	char **name_ptr);
/*****************************************************************************
LAST MODIFIED : 5 July 1999

DESCRIPTION :
============================================================================*/

int GT_element_settings_set_name(struct GT_element_settings *settings, char *name);
/*******************************************************************************
LAST MODIFIED : 5 July 1999

DESCRIPTION :
Sets the <name> used by <settings>.
==============================================================================*/

struct Computed_field *GT_element_settings_get_coordinate_field(
	struct GT_element_settings *settings);
/*******************************************************************************
LAST MODIFIED : 15 February 1999

DESCRIPTION :
Returns the coordinate field used by <settings>.
==============================================================================*/

int GT_element_settings_set_coordinate_field(
	struct GT_element_settings *settings,struct Computed_field *coordinate_field);
/*******************************************************************************
LAST MODIFIED : 15 February 1999

DESCRIPTION :
Sets the <coordinate_field> used by <settings>. If the coordinate field is
NULL, the settings uses the default coordinate_field from the graphical element
group.
==============================================================================*/

struct Computed_field *GT_element_settings_get_texture_coordinate_field(
	struct GT_element_settings *settings);
/*******************************************************************************
LAST MODIFIED : 1 June 1999

DESCRIPTION :
Returns the texture coordinate field used by <settings>.
==============================================================================*/

int GT_element_settings_set_texture_coordinate_field(
	struct GT_element_settings *settings,struct Computed_field *texture_coordinate_field);
/*******************************************************************************
LAST MODIFIED : 1 June 1999

DESCRIPTION :
Sets the <texture_coordinate_field> used by <settings>.
==============================================================================*/

int GT_element_settings_get_data_spectrum_parameters(
	struct GT_element_settings *settings,
	struct Computed_field **data_field,struct Spectrum **spectrum);
/*******************************************************************************
LAST MODIFIED : 24 March 1999

DESCRIPTION :
Returns parameters used for colouring the graphics created from <settings> with
the <data_field> (currently only a scalar) by the <spectrum>.
For type GT_ELEMENT_SETTINGS_STREAMLINES use function
GT_element_settings_get_data_spectrum_parameters_streamlines instead.
==============================================================================*/

int GT_element_settings_get_autorange_spectrum_flag(
	struct GT_element_settings *settings);
/*******************************************************************************
LAST MODIFIED : 24 January 2002

DESCRIPTION :
Returns the value of the autoranging_spectrum_flag from <settings>.
==============================================================================*/

int GT_element_settings_set_autorange_spectrum_flag(
	struct GT_element_settings *settings, int flag);
/*******************************************************************************
LAST MODIFIED : 24 January 2002

DESCRIPTION :
Sets the autorange_spectrum_flag used by <settings> to value of <flag>.
==============================================================================*/

int GT_element_settings_get_data_range_for_autoranging_spectrum(
	struct GT_element_settings *settings, void *data_void);
/*******************************************************************************
LAST MODIFIED : 18 January 2002

DESCRIPTION :
Expands the range to include the data values of only those element_group settings
in the <scene> which are autoranging and which point to this spectrum.
==============================================================================*/

int GT_element_settings_set_data_spectrum_parameters(
	struct GT_element_settings *settings,struct Computed_field *data_field,
	struct Spectrum *spectrum);
/*******************************************************************************
LAST MODIFIED : 24 March 1999

DESCRIPTION :
Sets parameters used for colouring the graphics created from <settings> with
the <data_field> (currently only a scalar) by the <spectrum>.
For type GT_ELEMENT_SETTINGS_STREAMLINES use function
GT_element_settings_set_data_spectrum_parameters_streamlines instead.
==============================================================================*/

int GT_element_settings_get_data_spectrum_parameters_streamlines(
	struct GT_element_settings *settings,
	enum Streamline_data_type *streamline_data_type,
	struct Computed_field **data_field,struct Spectrum **spectrum);
/*******************************************************************************
LAST MODIFIED : 24 March 1999

DESCRIPTION :
Special version of GT_element_settings_get_data_spectrum_parameters for type
GT_ELEMENT_SETTINGS_STREAMLINES only - handles the extended range of scalar data
options for streamlines - eg. STREAM_TRAVEL_SCALAR.
==============================================================================*/

int GT_element_settings_set_data_spectrum_parameters_streamlines(
	struct GT_element_settings *settings,
	enum Streamline_data_type streamline_data_type,
	struct Computed_field *data_field,struct Spectrum *spectrum);
/*******************************************************************************
LAST MODIFIED : 24 March 1999

DESCRIPTION :
Special version of GT_element_settings_set_data_spectrum_parameters for type
GT_ELEMENT_SETTINGS_STREAMLINES only - handles the extended range of scalar data
options for streamlines - eg. STREAM_TRAVEL_SCALAR.
==============================================================================*/

enum Render_type GT_element_settings_get_render_type(
	struct GT_element_settings *settings);
/*******************************************************************************
LAST MODIFIED : 2 May 2000

DESCRIPTION :
Get the type for how the graphics will be rendered in GL.
==============================================================================*/

int GT_element_settings_set_render_type(
	struct GT_element_settings *settings, enum Render_type render_type);
/*******************************************************************************
LAST MODIFIED : 2 May 2000

DESCRIPTION :
Set the type for how the graphics will be rendered in GL.
==============================================================================*/

int GT_element_settings_get_line_width(struct GT_element_settings *settings);
/*******************************************************************************
LAST MODIFIED : 22 April 2004

DESCRIPTION :
Get the settings line width.  If it is 0 then the line will use the scene default.
==============================================================================*/

int GT_element_settings_set_line_width(struct GT_element_settings *settings,
	int line_width);
/*******************************************************************************
LAST MODIFIED : 22 April 2004

DESCRIPTION :
Sets the settings line width.  If it is 0 then the line will use the scene default.
==============================================================================*/

int GT_element_settings_get_dimension(struct GT_element_settings *settings);
/*******************************************************************************
LAST MODIFIED : 15 September 1998

DESCRIPTION :
Returns the dimension of the <settings>, which varies for some settings types.
==============================================================================*/

int GT_element_settings_get_discretization(struct GT_element_settings *settings,
	struct Element_discretization *discretization);
/*******************************************************************************
LAST MODIFIED : 15 February 1999

DESCRIPTION :
Returns the <discretization> of points where glyphs are displayed for <settings>
of type GT_ELEMENT_SETTINGS_ELEMENT_POINTS.
==============================================================================*/

int GT_element_settings_set_discretization(struct GT_element_settings *settings,
	struct Element_discretization *discretization,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 15 February 1999

DESCRIPTION :
Sets the <discretization> of points where glyphs are displayed for <settings>
of type GT_ELEMENT_SETTINGS_ELEMENT_POINTS.
==============================================================================*/

int GT_element_settings_get_exterior(struct GT_element_settings *settings);
/*******************************************************************************
LAST MODIFIED : 5 June 1998

DESCRIPTION :
Returns 1 if <settings> is only using exterior elements.
For 1-D and 2-D settings types only.
==============================================================================*/

int GT_element_settings_set_exterior(struct GT_element_settings *settings,
	int exterior);
/*******************************************************************************
LAST MODIFIED : 5 June 1998

DESCRIPTION :
Value of <exterior> flag sets whether <settings> uses exterior elements.
For 1-D and 2-D settings types only.
==============================================================================*/

int GT_element_settings_get_face(struct GT_element_settings *settings,
	int *face);
/*******************************************************************************
LAST MODIFIED : 5 June 1998

DESCRIPTION :
Returns 1 if the settings refers to a particular face, with the face number,
where 0 is xi1=0, 1 is xi1=1, 2 is xi2=0 etc., returned in <face>.
For 1-D and 2-D settings types only.
==============================================================================*/

int GT_element_settings_set_face(struct GT_element_settings *settings,int face);
/*******************************************************************************
LAST MODIFIED : 5 June 1998

DESCRIPTION :
Sets the <face> used by <settings>, where 0 is xi1=0, 1 is xi1=1, 2 is xi2=0,
etc. If a value outside of 0 to 5 is passed, no face is specified.
For 1-D and 2-D settings types only.
==============================================================================*/

enum Graphics_select_mode GT_element_settings_get_select_mode(
	struct GT_element_settings *settings);
/*******************************************************************************
LAST MODIFIED : 23 February 2000

DESCRIPTION :
Returns the enumerator determining whether names are output with the graphics
for the settings, and if so which graphics are output depending on their
selection status.
==============================================================================*/

int GT_element_settings_set_select_mode(struct GT_element_settings *settings,
	enum Graphics_select_mode select_mode);
/*******************************************************************************
LAST MODIFIED : 23 February 2000

DESCRIPTION :
Sets the enumerator determining whether names are output with the graphics
for the settings, and if so which graphics are output depending on their
selection status.
==============================================================================*/

int GT_element_settings_get_glyph_parameters(
	struct GT_element_settings *settings,
	struct GT_object **glyph, enum Glyph_scaling_mode *glyph_scaling_mode,
	Triple glyph_centre, Triple glyph_size,
	struct Computed_field **orientation_scale_field, Triple glyph_scale_factors,
	struct Computed_field **variable_scale_field);
/*******************************************************************************
LAST MODIFIED : 16 November 2000

DESCRIPTION :
Returns the current glyph and parameters for orienting and scaling it.
==============================================================================*/

int GT_element_settings_set_glyph_parameters(
	struct GT_element_settings *settings,
	struct GT_object *glyph, enum Glyph_scaling_mode glyph_scaling_mode,
	Triple glyph_centre, Triple glyph_size,
	struct Computed_field *orientation_scale_field, Triple glyph_scale_factors,
	struct Computed_field *variable_scale_field);
/*******************************************************************************
LAST MODIFIED : 16 November 2000

DESCRIPTION :
Sets the glyph and parameters for orienting and scaling it.
See function make_glyph_orientation_scale_axes in
finite_element/finite_element_to_graphics object for explanation of how the
<orientation_scale_field> is used.
==============================================================================*/

int GT_element_settings_get_iso_surface_parameters(
	struct GT_element_settings *settings,struct Computed_field **iso_scalar_field,
	int *number_of_iso_values, double **iso_values,
	double *first_iso_value, double *last_iso_value, 
	double *decimation_threshold);
/*******************************************************************************
LAST MODIFIED : 13 July 2007

DESCRIPTION :
Returns parameters for the iso_surface: iso_scalar_field = iso_value.
Either the iso_values are stored in the array <iso_values> or they are
distributed from <first_iso_value> to <last_iso_value>.
For settings_type GT_ELEMENT_SETTINGS_ISO_SURFACES only.
==============================================================================*/

int GT_element_settings_set_iso_surface_parameters(
	struct GT_element_settings *settings,struct Computed_field *iso_scalar_field,
	int number_of_iso_values, double *iso_values,
	double first_iso_value, double last_iso_value,
	double decimation_threshold);
/*******************************************************************************
LAST MODIFIED : 13 July 2007

DESCRIPTION :
Sets parameters for the iso_surface: iso_scalar_field = iso_value.
Either the iso_values are stored in the array <iso_values> or they are
distributed from <first_iso_value> to <last_iso_value>.
For settings_type GT_ELEMENT_SETTINGS_ISO_SURFACES only - must call this after
CREATE to define a valid iso_surface.
==============================================================================*/

int GT_element_settings_get_label_field(struct GT_element_settings *settings,
	struct Computed_field **label_field, struct Graphics_font **font);
/*******************************************************************************
LAST MODIFIED : 18 November 2005

DESCRIPTION :
Returns the label_field used by <settings>. For settings types
GT_ELEMENT_SETTINGS_NODE_POINTS, GT_ELEMENT_SETTINGS_DATA_POINTS and
GT_ELEMENT_SETTINGS_ELEMENT_POINTS only.
==============================================================================*/

int GT_element_settings_set_label_field(
	struct GT_element_settings *settings,struct Computed_field *label_field,
	struct Graphics_font *font);
/*******************************************************************************
LAST MODIFIED : 18 November 2005

DESCRIPTION :
Sets the <label_field> used by <settings>. For settings types
GT_ELEMENT_SETTINGS_NODE_POINTS, GT_ELEMENT_SETTINGS_DATA_POINTS and
GT_ELEMENT_SETTINGS_ELEMENT_POINTS only. The field specified will be evaluated
as a string and displayed beside the glyph.
==============================================================================*/

struct Graphical_material *GT_element_settings_get_material(
	struct GT_element_settings *settings);
/*******************************************************************************
LAST MODIFIED : 5 June 1998

DESCRIPTION :
Returns the material used by <settings>.
==============================================================================*/

int GT_element_settings_set_material(struct GT_element_settings *settings,
	struct Graphical_material *material);
/*******************************************************************************
LAST MODIFIED : 5 June 1998

DESCRIPTION :
Sets the <material> used by <settings>. Must set the material for each new
settings created.
==============================================================================*/

struct Graphical_material *GT_element_settings_get_secondary_material(
	struct GT_element_settings *settings);
/*******************************************************************************
LAST MODIFIED : 30 September 2005

DESCRIPTION :
Returns the secondary_material used by <settings>.
==============================================================================*/

int GT_element_settings_set_secondary_material(struct GT_element_settings *settings,
	struct Graphical_material *material);
/*******************************************************************************
LAST MODIFIED : 30 September 2005

DESCRIPTION :
Sets the <secondary_material> used by <settings>.
==============================================================================*/

struct Graphical_material *GT_element_settings_get_selected_material(
	struct GT_element_settings *settings);
/*******************************************************************************
LAST MODIFIED : 18 February 2000

DESCRIPTION :
Returns the selected material used by <settings>.
Selected objects relevant to the settings are displayed with this material.
==============================================================================*/

int GT_element_settings_set_selected_material(
	struct GT_element_settings *settings,
	struct Graphical_material *selected_material);
/*******************************************************************************
LAST MODIFIED : 18 February 2000

DESCRIPTION :
Sets the <selected_material> used by <settings>.
Selected objects relevant to the settings are displayed with this material.
==============================================================================*/

struct FE_field *GT_element_settings_get_native_discretization_field(
	struct GT_element_settings *settings);
/*******************************************************************************
LAST MODIFIED : 3 March 1999

DESCRIPTION :
Returns the native_discretization field used by <settings>.
For type GT_ELEMENT_SETTINGS_ELEMENT_POINTS only.
==============================================================================*/

int GT_element_settings_set_native_discretization_field(
	struct GT_element_settings *settings,
	struct FE_field *native_discretization_field);
/*******************************************************************************
LAST MODIFIED : 3 March 1999

DESCRIPTION :
Sets the <native_discretization_field> used by <settings>. If the settings have
such a field, and the field is element based over the element in question, then
the native discretization of that field in that element is used instead of the
settings->discretization.
For type GT_ELEMENT_SETTINGS_ELEMENT_POINTS only.
==============================================================================*/

int GT_element_settings_get_radius_parameters(
	struct GT_element_settings *settings,float *constant_radius,
	float *radius_scale_factor,struct Computed_field **radius_scalar_field);
/*******************************************************************************
LAST MODIFIED : 15 February 1999

DESCRIPTION :
Returns the current radius parameters which are used in the expression:
radius = constant_radius + radius_scale_factor*radius_scalar_field.
For settings_type GT_ELEMENT_SETTINGS_CYLINDERS only.
==============================================================================*/

int GT_element_settings_set_radius_parameters(
	struct GT_element_settings *settings,float constant_radius,
	float radius_scale_factor,struct Computed_field *radius_scalar_field);
/*******************************************************************************
LAST MODIFIED : 15 February 1999

DESCRIPTION :
Sets the current radius parameters which are used in the expression:
radius = constant_radius + radius_scale_factor*radius_scalar_field.
For settings_type GT_ELEMENT_SETTINGS_CYLINDERS only.
==============================================================================*/

struct FE_element *GT_element_settings_get_seed_element(
	struct GT_element_settings *settings);
/*******************************************************************************
LAST MODIFIED : 18 August 1998

DESCRIPTION :
For settings starting in a particular element.
==============================================================================*/

int GT_element_settings_set_seed_element(struct GT_element_settings *settings,
	struct FE_element *seed_element);
/*******************************************************************************
LAST MODIFIED : 18 August 1998

DESCRIPTION :
For settings starting in a particular element.
==============================================================================*/

int GT_element_settings_get_seed_xi(struct GT_element_settings *settings,
	Triple seed_xi);
/*******************************************************************************
LAST MODIFIED : 7 June 2000

DESCRIPTION :
For settings_types GT_ELEMENT_SETTINGS_ELEMENT_POINTS or
GT_ELEMENT_SETTINGS_STREAMLINES only.
==============================================================================*/

int GT_element_settings_set_seed_xi(struct GT_element_settings *settings,
	Triple seed_xi);
/*******************************************************************************
LAST MODIFIED : 7 June 2000

DESCRIPTION :
For settings_types GT_ELEMENT_SETTINGS_ELEMENT_POINTS or
GT_ELEMENT_SETTINGS_STREAMLINES only.
==============================================================================*/

int GT_element_settings_get_streamline_parameters(
	struct GT_element_settings *settings,enum Streamline_type *streamline_type,
	struct Computed_field **stream_vector_field,int *reverse_track,
	float *streamline_length,float *streamline_width);
/*******************************************************************************
LAST MODIFIED : 24 March 1999

DESCRIPTION :
For settings_type GT_ELEMENT_SETTINGS_STREAMLINES only.
==============================================================================*/

int GT_element_settings_set_streamline_parameters(
	struct GT_element_settings *settings,enum Streamline_type streamline_type,
	struct Computed_field *stream_vector_field,int reverse_track,
	float streamline_length,float streamline_width);
/*******************************************************************************
LAST MODIFIED : 24 March 1999

DESCRIPTION :
For settings_type GT_ELEMENT_SETTINGS_STREAMLINES only.
==============================================================================*/

enum Use_element_type GT_element_settings_get_use_element_type(
	struct GT_element_settings *settings);
/*******************************************************************************
LAST MODIFIED : 28 January 2000

DESCRIPTION :
Returns the type of elements used by the settings.
For <settings> type GT_ELEMENT_SETTINGS_ELEMENT_POINTS and
GT_ELEMENT_SETTINGS_ISO_SURFACES only.
==============================================================================*/

int GT_element_settings_set_use_element_type(
	struct GT_element_settings *settings,enum Use_element_type use_element_type);
/*******************************************************************************
LAST MODIFIED : 28 January 2000

DESCRIPTION :
Sets the type of elements used by the settings.
For <settings> type GT_ELEMENT_SETTINGS_ELEMENT_POINTS and
GT_ELEMENT_SETTINGS_ISO_SURFACES only.
==============================================================================*/

int GT_element_settings_get_visibility(struct GT_element_settings *settings);
/*******************************************************************************
LAST MODIFIED : 10 June 1998

DESCRIPTION :
Returns 1 if the graphics are visible for <settings>, otherwise 0.
==============================================================================*/

int GT_element_settings_set_visibility(struct GT_element_settings *settings,
	int visibility);
/*******************************************************************************
LAST MODIFIED : 10 June 1998

DESCRIPTION :
Sets the visibility of the graphics described by the settings, where 1 is
visible, 0 is invisible.
==============================================================================*/

struct VT_volume_texture *GT_element_settings_get_volume_texture(
	struct GT_element_settings *settings);
/*******************************************************************************
LAST MODIFIED : 27 July 1998

DESCRIPTION :
For settings_type GT_ELEMENT_SETTINGS_VOLUMES only.
==============================================================================*/

int GT_element_settings_set_volume_texture(struct GT_element_settings *settings,
	struct VT_volume_texture *volume_texture);
/*******************************************************************************
LAST MODIFIED : 27 July 1998

DESCRIPTION :
For settings_type GT_ELEMENT_SETTINGS_VOLUMES only.
==============================================================================*/

int GT_element_settings_get_xi_discretization(
	struct GT_element_settings *settings,
	enum Xi_discretization_mode *xi_discretization_mode,
	struct Computed_field **xi_point_density_field);
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

int GT_element_settings_set_xi_discretization(
	struct GT_element_settings *settings,
	enum Xi_discretization_mode xi_discretization_mode,
	struct Computed_field *xi_point_density_field);
/*******************************************************************************
LAST MODIFIED : 3 May 2001

DESCRIPTION :
Sets the xi_discretization_mode controlling where glyphs are displayed for
<settings> of type GT_ELEMENT_SETTINGS_ELEMENT_POINTS. Must supply a scalar
<xi_point_density_field> if the new mode is XI_DISCRETIZATION_CELL_DENSITY or
XI_DISCRETIZATION_CELL_POISSON.
==============================================================================*/

struct GT_object *GT_element_settings_get_graphics_object(
	struct GT_element_settings *settings);
/*******************************************************************************
LAST MODIFIED : 12 June 1998

DESCRIPTION :
Returns a pointer to the graphics_object in <settings>, if any. Should be used
with care - currently only used for determining picking.
==============================================================================*/

int GT_element_settings_has_name(struct GT_element_settings *settings,
	void *name_void);
/*******************************************************************************
LAST MODIFIED : 7 July 1999

DESCRIPTION :
Settings iterator function returning true if <settings> has the 
specified <name>.  If the settings doesn't have a name then the position
number is converted to a string and that is compared to the supplied <name>.
==============================================================================*/

int GT_element_settings_has_embedded_field(
	struct GT_element_settings *settings,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 28 April 1999

DESCRIPTION :
Returns 1 if the <settings> use any embedded_fields.
==============================================================================*/

int GT_element_settings_update_time_behaviour(
	struct GT_element_settings *settings, void *update_time_behaviour_void);
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

int GT_element_settings_default_coordinate_field_change(
	struct GT_element_settings *settings,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 3 March 1999

DESCRIPTION :
Deaccesses any graphics_object in <settings> if it uses the default coordinate
field from the gt_element_group - that is, settings->coordinate_field is NULL.
==============================================================================*/

int GT_element_settings_element_discretization_change(
	struct GT_element_settings *settings,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 12 June 1998

DESCRIPTION :
Deaccesses any graphics_object in <settings> if it would have to be rebuilt
due to a change in element discretization.
==============================================================================*/

int GT_element_settings_circle_discretization_change(
	struct GT_element_settings *settings,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 12 June 1998

DESCRIPTION :
Deaccesses any graphics_object in <settings> if it would have to be rebuilt
due to a change in element discretization.
==============================================================================*/

struct GT_element_settings_FE_region_change_data
/*******************************************************************************
LAST MODIFIED : 4 February 2003

DESCRIPTION :
Data for passing to GT_element_settings_FE_region_change.
==============================================================================*/
{
	/* changes to fields with summary */
	enum CHANGE_LOG_CHANGE(FE_field) fe_field_change_summary;
	struct CHANGE_LOG(FE_field) *fe_field_changes;
	/* changes to nodes with summary and number_of_changes */
	enum CHANGE_LOG_CHANGE(FE_node) fe_node_change_summary;
	int number_of_fe_node_changes;
	struct CHANGE_LOG(FE_node) *fe_node_changes;
	/* changes to elements with summary and number_of_changes */
	enum CHANGE_LOG_CHANGE(FE_element) fe_element_change_summary;
	int number_of_fe_element_changes;
	struct CHANGE_LOG(FE_element) *fe_element_changes;
	FE_value time;
	/* following set if changes affect any of the graphics */
	int graphics_changed;
	/* the default coordinate field for the whole GT_element_group for when
		 not overridden by settings->coordinate_field */
	struct Computed_field *default_coordinate_field;
	/* the FE_region the settings apply to */
	struct FE_region *fe_region;
}; /* struct GT_element_settings_FE_region_change_data */

int GT_element_settings_FE_region_change(
	struct GT_element_settings *settings, void *data_void);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
Tells <settings> about changes to the FE_region it is operating. If changes
affects its graphics, sets them up to be rebuilt.
<data_void> points at a struct GT_element_settings_FE_region_change_data.
==============================================================================*/

int GT_element_settings_data_FE_region_change(
	struct GT_element_settings *settings, void *data_void);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
Tells <settings> about changes to the FE_region it is operating. If changes
affects its graphics, sets them up to be rebuilt.
<data_void> points at a struct GT_element_settings_FE_region_change_data.
???RC Temporary until data_root_region/data_hack removed.
==============================================================================*/

int GT_element_settings_time_change(
	struct GT_element_settings *settings,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 25 October 2000

DESCRIPTION :
Notifies the <settings> that time has changed.
==============================================================================*/

int GT_element_settings_glyph_change(
	struct GT_object *glyph,void *settings_void);
/*******************************************************************************
LAST MODIFIED : 26 October 2000 (None Tree Hill Day)

DESCRIPTION :
Notifies the <settings> that the glyph used has changed.
==============================================================================*/

int GT_element_settings_uses_dimension(struct GT_element_settings *settings,
	void *dimension_void);
/*******************************************************************************
LAST MODIFIED : 14 September 1998

DESCRIPTION :
Iterator function returning true if the settings uses nodes/elements of the
given <dimension>. The special value of -1 denotes all dimensions and always
returns true.
==============================================================================*/

int GT_element_settings_selects_elements(struct GT_element_settings *settings);
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
Returns true if the graphics for <settings> are output with names that identify
the elements they are calculated from.
==============================================================================*/

enum GT_element_settings_type GT_element_settings_get_settings_type(
	struct GT_element_settings *settings);
/*******************************************************************************
LAST MODIFIED : 5 June 1998

DESCRIPTION :
Returns the settings type of the <settings>, eg. GT_ELEMENT_SETTINGS_LINE.
==============================================================================*/

int GT_element_settings_type_matches(
	struct GT_element_settings *settings,void *settings_type_void);
/*******************************************************************************
LAST MODIFIED : 2 July 1997

DESCRIPTION :
Returns 1 if the settings are of the specified settings_type.
==============================================================================*/

int GT_element_settings_add_to_list(struct GT_element_settings *settings,
	int position,struct LIST(GT_element_settings) *list_of_settings);
/*******************************************************************************
LAST MODIFIED : 4 June 1998

DESCRIPTION :
Adds the <settings> to <list_of_settings> at the given <position>, where 1 is
the top of the list (rendered first), and values less than 1 or greater than the
last position in the list cause the settings to be added at its end, with a
position one greater than the last.
==============================================================================*/

int GT_element_settings_remove_from_list(struct GT_element_settings *settings,
	struct LIST(GT_element_settings) *list_of_settings);
/*******************************************************************************
LAST MODIFIED : 16 June 1998

DESCRIPTION :
Removes the <settings> from <list_of_settings> and decrements the position
of all subsequent settings.
==============================================================================*/

int GT_element_settings_modify_in_list(struct GT_element_settings *settings,
	struct GT_element_settings *new_settings,
	struct LIST(GT_element_settings) *list_of_settings);
/*******************************************************************************
LAST MODIFIED : 16 June 1998

DESCRIPTION :
Changes the contents of <settings> to match <new_settings>, with no change in
position in <list_of_settings>.
==============================================================================*/

int GT_element_settings_get_position_in_list(
	struct GT_element_settings *settings,
	struct LIST(GT_element_settings) *list_of_settings);
/*******************************************************************************
LAST MODIFIED : 9 June 1998

DESCRIPTION :
Returns the position of <settings> in <list_of_settings>.
==============================================================================*/

int GT_element_settings_same_geometry(struct GT_element_settings *settings,
	void *second_settings_void);
/*******************************************************************************
LAST MODIFIED : 11 June 1998

DESCRIPTION :
LIST(GT_element_settings) conditional function returning 1 iff the two
gt_element_settings describe the same geometry.
==============================================================================*/

int GT_element_settings_same_name_or_geometry(struct GT_element_settings *settings,
	void *second_settings_void);
/*******************************************************************************
LAST MODIFIED : 5 November 2002

DESCRIPTION :
GT_element_settings list conditional function returning 1 iff the two
settings have the same name or describe EXACTLY the same geometry.
==============================================================================*/

int GT_element_settings_same_non_trivial(struct GT_element_settings *settings,
	void *second_settings_void);
/*******************************************************************************
LAST MODIFIED : 11 June 1998

DESCRIPTION :
GT_element_settings list conditional function returning 1 iff the two
settings have the same geometry and the same nontrivial appearance
characteristics. Trivial appearance characteristics are the material,
visibility and spectrum.
==============================================================================*/

int GT_element_settings_same_non_trivial_with_graphics_object(
	struct GT_element_settings *settings,void *second_settings_void);
/*******************************************************************************
LAST MODIFIED : 11 June 1998

DESCRIPTION :
Same as GT_element_settings_same_non_trivial except <settings> must also have
a graphics_object. Used for getting graphics objects from previous settings
that are the same except for trivial differences such as the material and
spectrum which can be changed in the graphics object to match the new settings .
==============================================================================*/

int GT_element_settings_match(struct GT_element_settings *settings1,
	struct GT_element_settings *settings2);
/*******************************************************************************
LAST MODIFIED : 14 November 2001

DESCRIPTION :
Returns true if <settings1> and <settings2> would product identical graphics.
==============================================================================*/

int GT_element_settings_extract_graphics_object_from_list(
	struct GT_element_settings *settings,void *list_of_settings_void);
/*******************************************************************************
LAST MODIFIED : 11 June 1998

DESCRIPTION :
If <settings> does not already have a graphics object, this function attempts
to find settings in <list_of_settings> which differ only trivially in material,
spectrum etc. AND have a graphics object. If such a settings is found, the
graphics_object is moved from the matching settings and put in <settings>, while
any trivial differences are fixed up in the graphics_obejct.
==============================================================================*/

char *GT_element_settings_string(struct GT_element_settings *settings,
	enum GT_element_settings_string_details settings_detail);
/*******************************************************************************
LAST MODIFIED : 21 July 1997

DESCRIPTION :
Returns a string describing the settings, suitable for entry into the command
line. Parameter <settings_detail> selects whether appearance settings are
included in the string. User must remember to DEALLOCATE the name afterwards.
==============================================================================*/

int GT_element_settings_list_contents(struct GT_element_settings *settings,
	void *list_data_void);
/*******************************************************************************
LAST MODIFIED : 22 July 1998

DESCRIPTION :
Writes out the <settings> as a text string in the command window with the
<settings_string_detail>, <line_prefix> and <line_suffix> given in the
<list_data>.
==============================================================================*/

int GT_element_settings_copy_and_put_in_list(
	struct GT_element_settings *settings,void *list_of_settings_void);
/*******************************************************************************
LAST MODIFIED : 10 July 1997

DESCRIPTION :
GT_element_settings iterator function for copying a list_of_settings.
Makes a copy of the settings and puts it in the list_of_settings.
==============================================================================*/

int GT_element_settings_to_graphics_object(
	struct GT_element_settings *settings,void *settings_to_object_data_void);
/*******************************************************************************
LAST MODIFIED : 11 June 1998

DESCRIPTION :
Creates a GT_object and fills it with the objects described by settings.
The graphics object is stored with with the settings it was created from.
==============================================================================*/

int GT_element_settings_selected_element_points_change(
	struct GT_element_settings *settings,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 28 February 2000

DESCRIPTION :
Tells <settings> that if the graphics resulting from it depend on the currently
selected element points, then they should be updated.
Must call GT_element_settings_to_graphics_object afterwards to complete.
==============================================================================*/

int GT_element_settings_selected_elements_change(
	struct GT_element_settings *settings,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 25 February 2000

DESCRIPTION :
Tells <settings> that if the graphics resulting from it depend on the currently
selected elements, then they should be updated.
Must call GT_element_settings_to_graphics_object afterwards to complete.
==============================================================================*/

int GT_element_settings_selected_nodes_change(
	struct GT_element_settings *settings,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 25 February 2000

DESCRIPTION :
Tells <settings> that if the graphics resulting from it depend on the currently
selected nodes, then they should be updated.
Must call GT_element_settings_to_graphics_object afterwards to complete.
==============================================================================*/

int GT_element_settings_selected_data_change(
	struct GT_element_settings *settings,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 28 April 2000

DESCRIPTION :
Tells <settings> that if the graphics resulting from it depend on the currently
selected nodes, then they should be updated.
Must call GT_element_settings_to_graphics_object afterwards to complete.
==============================================================================*/

/***************************************************************************//**
 * If the settings visibility flag is set and it has a graphics_object, the
 * graphics_object is compiled. <renderer_void> must point to a valid
 * Render_graphics_opengl object.
 */
int GT_element_settings_compile_visible_settings(
	struct GT_element_settings *settings, void *renderer_void);

int GT_element_settings_execute_visible_settings(
	struct GT_element_settings *settings, void *context_void);
/*******************************************************************************
LAST MODIFIED : 12 October 2005

DESCRIPTION :
If the settings visibility flag is set and it has a graphics_object, the
graphics_object is executed, while the position of the settings in the list
is put out as a name to identify the object in OpenGL picking.
==============================================================================*/

int GT_element_settings_Computed_field_change(
	struct GT_element_settings *settings, void *change_data_void);
/*******************************************************************************
LAST MODIFIED : 22 January 2003

DESCRIPTION :
Iterator function telling the <settings> that the computed fields in the
<changed_field_list> have changed.
Clears graphics for settings affected by the change.
<change_data_void> points at a
struct GT_element_settings_Computed_field_change_data.
==============================================================================*/

int GT_element_settings_Graphical_material_change(
	struct GT_element_settings *settings, void *material_change_data_void);
/*******************************************************************************
LAST MODIFIED : 7 June 2001

DESCRIPTION :
If <settings> has a graphics object that plots data, and it uses any material in
the <changed_material_list>, the graphics object display list is marked as being
not current. This is required since the spectrum combines its colour with the
material at the time it is compiled - it may not update correctly if only the
material's display list is recompiled.
If the settings are visible, the changed flag is set.
Second argument is a struct GT_element_settings_Graphical_material_change_data.
==============================================================================*/

int GT_element_settings_Spectrum_change(
	struct GT_element_settings *settings, void *spectrum_change_data_void);
/*******************************************************************************
LAST MODIFIED : 7 June 2001

DESCRIPTION :
If <settings> has a graphics object that plots a scalar using a spectrum in the
<changed_spectrum_list>, the graphics object display list is marked as being
not current. If the settings are visible, the changed flag is set.
Second argument is a struct GT_element_settings_Spectrum_change_data.
==============================================================================*/

int GT_element_settings_get_visible_graphics_object_range(
	struct GT_element_settings *settings,void *graphics_object_range_void);
/*******************************************************************************
LAST MODIFIED : 15 June 1998

DESCRIPTION :
If there is a visible graphics_object in <settings>, expands the
<graphics_object_range> to include its range.
==============================================================================*/

int gfx_modify_g_element_node_points(struct Parse_state *state,
	void *modify_g_element_data_void,void *command_data_void);
/*******************************************************************************
LAST MODIFIED : 10 September 1998

DESCRIPTION :
Executes a GFX MODIFY G_ELEMENT NODE_POINTS command.
If return_code is 1, returns the completed Modify_g_element_data with the
parsed settings. Note that the settings are ACCESSed once on valid return.
==============================================================================*/

int gfx_modify_g_element_data_points(struct Parse_state *state,
	void *modify_g_element_data_void,void *command_data_void);
/*******************************************************************************
LAST MODIFIED : 10 September 1998

DESCRIPTION :
Executes a GFX MODIFY G_ELEMENT DATA_POINTS command.
If return_code is 1, returns the completed Modify_g_element_data with the
parsed settings. Note that the settings are ACCESSed once on valid return.
==============================================================================*/

int gfx_modify_g_element_lines(struct Parse_state *state,
	void *modify_g_element_data_void,void *command_data_void);
/*******************************************************************************
LAST MODIFIED : 3 July 1997

DESCRIPTION :
Executes a GFX MODIFY G_ELEMENT LINES command.
If return_code is 1, returns the completed Modify_g_element_data with the
parsed settings. Note that the settings are ACCESSed once on valid return.
==============================================================================*/

int gfx_modify_g_element_cylinders(struct Parse_state *state,
	void *modify_g_element_data_void,void *g_element_command_data_void);
/*******************************************************************************
LAST MODIFIED : 28 May 1998

DESCRIPTION :
Executes a GFX MODIFY G_ELEMENT CYLINDERS command.
If return_code is 1, returns the completed Modify_g_element_data with the
parsed settings. Note that the settings are ACCESSed once on valid return.
==============================================================================*/

int gfx_modify_g_element_surfaces(struct Parse_state *state,
	void *modify_g_element_data_void,void *command_data_void);
/*******************************************************************************
LAST MODIFIED : 3 July 1997

DESCRIPTION :
Executes a GFX MODIFY G_ELEMENT SURFACES command.
If return_code is 1, returns the completed Modify_g_element_data with the
parsed settings. Note that the settings are ACCESSed once on valid return.
==============================================================================*/

int gfx_modify_g_element_iso_surfaces(struct Parse_state *state,
	void *modify_g_element_data_void,void *g_element_command_data_void);
/*******************************************************************************
LAST MODIFIED : 26 June 1998

DESCRIPTION :
Executes a GFX MODIFY G_ELEMENT ISO_SURFACES command.
If return_code is 1, returns the completed Modify_g_element_data with the
parsed settings. Note that the settings are ACCESSed once on valid return.
==============================================================================*/

int gfx_modify_g_element_element_points(struct Parse_state *state,
	void *modify_g_element_data_void,void *g_element_command_data_void);
/*******************************************************************************
LAST MODIFIED : 19 August 1998

DESCRIPTION :
Executes a GFX MODIFY G_ELEMENT ELEMENT_POINTS command.
If return_code is 1, returns the completed Modify_g_element_data with the
parsed settings. Note that the settings are ACCESSed once on valid return.
==============================================================================*/

int gfx_modify_g_element_streamlines(struct Parse_state *state,
	void *modify_g_element_data_void,void *g_element_command_data_void);
/*******************************************************************************
LAST MODIFIED : 19 March 1999

DESCRIPTION :
Executes a GFX MODIFY G_ELEMENT STREAMLINES command.
If return_code is 1, returns the completed Modify_g_element_data with the
parsed settings. Note that the settings are ACCESSed once on valid return.
==============================================================================*/
#endif /* !defined (ELEMENT_GROUP_SETTINGS_H) */
