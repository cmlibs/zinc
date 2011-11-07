/***************************************************************************//**
 * cmiss_graphic.h
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

#if !defined (CMISS_GRAPHIC_H)
#define CMISS_GRAPHIC_H

#include "api/cmiss_graphic.h"
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

struct Cmiss_graphic;
struct Cmiss_graphics_module;

typedef enum Cmiss_graphic_type Cmiss_graphic_type_enum;

PROTOTYPE_ENUMERATOR_FUNCTIONS(Cmiss_graphic_type);

/***************************************************************************//**
 * Unique identifier for each graphic attribute, used to query whether it can be
 * used with each Cmiss_graphic_type.
 * Must update Cmiss_graphic_type_uses_attribute as new attribute IDs defined.
 * @see Cmiss_graphic_type_uses_attribute()
 */
enum Cmiss_graphic_attribute
{
	CMISS_GRAPHIC_ATTRIBUTE_DISCRETIZATION,
	CMISS_GRAPHIC_ATTRIBUTE_LABEL_FIELD,
	CMISS_GRAPHIC_ATTRIBUTE_NATIVE_DISCRETIZATION_FIELD,
	CMISS_GRAPHIC_ATTRIBUTE_TESSELLATION
};

enum Cmiss_graphic_string_details
{
  GRAPHIC_STRING_GEOMETRY,
	GRAPHIC_STRING_COMPLETE,
	GRAPHIC_STRING_COMPLETE_PLUS
}; /* enum Cmiss_graphic_string_details */

/***************************************************************************//**
 * Data for formating output with GT_element_settings_list_contents function.
 */
struct Cmiss_graphic_list_data
{
	const char *line_prefix,*line_suffix;
	enum Cmiss_graphic_string_details graphic_string_detail;
}; /* Cmiss_graphic_list_data */

/***************************************************************************//**
 * Enumerator controlling how glyph orientation and scaling parameters are set.
 * Modes other than GENERAL restrict the options available and their
 * interpretation. For example, in GENERAL mode the base size can be a*b*c, but in
 * VECTOR mode it is restricted to be 0*a*a.
 * Note: the first value will be 0 by the ANSI standard, with each subsequent entry
 * incremented by 1. This pattern is expected by the ENUMERATOR macros.
 * Must ensure the ENUMERATOR_STRING function returns a string for each value here.
 */
enum Graphic_glyph_scaling_mode
{
	GRAPHIC_GLYPH_SCALING_CONSTANT,
	GRAPHIC_GLYPH_SCALING_SCALAR,
	GRAPHIC_GLYPH_SCALING_VECTOR,
	GRAPHIC_GLYPH_SCALING_AXES,
	GRAPHIC_GLYPH_SCALING_GENERAL
}; /* enum Glyph_scaling_mode */

struct Cmiss_graphic_update_time_behaviour_data
{
	/* flag set by Cmiss_rendition if the default coordinate field depends on time */
	int default_coordinate_depends_on_time;
	/* flag set by settings if any of the settings depend on time */
	int time_dependent;
};

struct Cmiss_graphic_range
{
	struct Graphics_object_range_struct *graphics_object_range;
	enum Cmiss_graphics_coordinate_system coordinate_system;
};

PROTOTYPE_ENUMERATOR_FUNCTIONS(Graphic_glyph_scaling_mode);

struct Cmiss_graphic_to_graphics_object_data
{
	Cmiss_field_cache_id field_cache;
	/* graphics object names are preceded by this */
	const char *name_prefix;
	/* default_rc_coordinate_field to use if NULL in any settings */
	struct Computed_field *rc_coordinate_field,
		*wrapper_orientation_scale_field,*wrapper_stream_vector_field,
		*group_field;
	struct Cmiss_region *region;
	Cmiss_field_module_id field_module;
	struct FE_region *fe_region;
	struct FE_region *data_fe_region;
	Cmiss_mesh_id surface_mesh; // mesh being converted into surface graphics
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
	/** The number of components in the data field */
	int number_of_data_values;
	/** A buffer allocated large enough for evaluating the data field */
	FE_value *data_copy_buffer;

	struct Iso_surface_specification *iso_surface_specification;
	struct Cmiss_scene *scene;
	/* additional values for passing to element_to_graphics_object */
	struct Cmiss_graphic *graphic;
	int top_level_number_in_xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
};

struct Modify_rendition_data
/*******************************************************************************
LAST MODIFIED : 5 June 1998

DESCRIPTION :
Structure modified by g_element modify routines.
==============================================================================*/
{
	char delete_flag;
	int position;
	struct Cmiss_graphic *graphic;
}; /* struct Modify_graphic_data */

/***************************************************************************//**
 * Subset of command data passed to rendition modify routines.
 */
struct Rendition_command_data
{
	struct Cmiss_graphics_module *graphics_module;
	struct Cmiss_rendition *rendition;
	struct Graphical_material *default_material;
	struct Graphics_font *default_font;
	struct Graphics_font_package *graphics_font_package;
	struct MANAGER(GT_object *) glyph_manager;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Cmiss_region *region;
	/* root_region used for seeding streamlines from the nodes in a region */
	struct Cmiss_region *root_region;
	struct MANAGER(Graphical_material) *graphical_material_manager;
	struct Spectrum *default_spectrum;
	struct MANAGER(Spectrum) *spectrum_manager;
}; /* struct Rendition_command_data */

struct Cmiss_graphic_Computed_field_change_data
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
	int selection_changed;
};

DECLARE_LIST_TYPES(Cmiss_graphic);

/***************************************************************************//**
 * Queries whether an attribute can be used with supplied graphic_type.
 * @param graphic_type  The type of graphic to query.
 * @param attribute  Identifier of graphic attribute to query about.
 * @return  1 if the attribute can be used with graphic type, otherwise 0.
 */
int Cmiss_graphic_type_uses_attribute(enum Cmiss_graphic_type graphic_type,
	enum Cmiss_graphic_attribute attribute);

/***************************************************************************//**
 * Created with access_count = 1.
 */
struct Cmiss_graphic *CREATE(Cmiss_graphic)(
	enum Cmiss_graphic_type graphic_type);

int DESTROY(Cmiss_graphic)(struct Cmiss_graphic **cmiss_graphic_address);

/***************************************************************************//** 
 * Adds the <settings> to <list_of_settings> at the given <position>, where 1 is
 * the top of the list (rendered first), and values less than 1 or greater than the
 * last position in the list cause the settings to be added at its end, with a
 * position one greater than the last.
 * @param graphic The graphic to be added
 * @param position Given position of the item
 * @param list_of_graphic The list of graphic
 * @return If successfully add graphic to list of graphic returns 1, otherwise 0
 */
int Cmiss_graphic_add_to_list(struct Cmiss_graphic *graphic,
	int position,struct LIST(Cmiss_graphic) *list_of_graphic);

/***************************************************************************//**
 * Removes the <graphic> from <list_of_graphic> and decrements the position
 * of all subsequent graphic.
 */
int Cmiss_graphic_remove_from_list(struct Cmiss_graphic *graphic,
	struct LIST(Cmiss_graphic) *list_of_graphic);

/***************************************************************************//**
 * Changes the contents of <graphic> to match <new_graphic>, with no change in
 * position in <list_of_graphic>.
 */
int Cmiss_graphic_modify_in_list(struct Cmiss_graphic *graphic,
	struct Cmiss_graphic *new_graphic,
	struct LIST(Cmiss_graphic) *list_of_graphic);

PROTOTYPE_OBJECT_FUNCTIONS(Cmiss_graphic);
PROTOTYPE_LIST_FUNCTIONS(Cmiss_graphic);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Cmiss_graphic,position,int);

/***************************************************************************//** 
 * Returns a string describing the graphic, suitable for entry into the command
 * line. Parameter <graphic_detail> selects whether appearance graphic are
 * included in the string. User must remember to DEALLOCATE the name afterwards.
 * ???RC When we have editing of graphic, rather than creating from scratch each
 * time as we do now with our text commands, we must ensure that defaults are
 * restored by commands generated by this string, eg. must have coordinate NONE
 * if no coordinate field. Currently only write if we have a field.
 * @param graphic The cmiss graphic
 * @param graphic_detail given detail of the string
 * @return the Cmiss_graphic_string
 */
char *Cmiss_graphic_string(struct Cmiss_graphic *graphic,
	enum Cmiss_graphic_string_details graphic_detail);

/***************************************************************************//**
 * @return  1 if both graphic and its rendition visibility flags are set,
 * otherwise 0. 
 */
int Cmiss_graphic_and_rendition_visibility_flags_set(struct Cmiss_graphic *graphic);

/***************************************************************************//**
 * @return  1 if graphic is one of the graphical representations of region,
 * otherwise 0.
 */
int Cmiss_graphic_is_from_region_hierarchical(struct Cmiss_graphic *graphic, struct Cmiss_region *region);

/***************************************************************************//**
 * Returns true if the graphics are output with names that identify
 * the elements they are calculated from.
 */
int Cmiss_graphic_selects_elements(struct Cmiss_graphic *graphic);

/***************************************************************************//**
 * Returns the dimension of the <graphic>, which varies for some graphic types.
 * @param graphic Cmiss graphic
 * @param fe_region  Used for iso_surfaces and element_points with USE_ELEMENT
 * type. Gives the highest dimension for which elements exist. If omitted uses 3.
 * @return the dimension of the graphic
 */
int Cmiss_graphic_get_dimension(struct Cmiss_graphic *graphic, struct FE_region *fe_region);

#if defined (USE_OPENCASCADE)
/**
 * Returns 1 if the graphics are output with names that identify
 * the elements they are calculated from.  Otherwise it returns 0.
 */
int Cmiss_graphic_selects_cad_primitives(struct Cmiss_graphic *graphic);
#endif /* defined (USE_OPENCASCADE) */

/***************************************************************************//**
 *Returns the settings type of the <graphic>, eg. CMISS_GRAPHIC_LINES.
 */
enum Cmiss_graphic_type Cmiss_graphic_get_graphic_type(
	struct Cmiss_graphic *graphic);

/***************************************************************************//**
 *Returns 1 if the graphic type of the <graphic> is same as the one specified.
 */
int Cmiss_graphic_is_graphic_type(struct Cmiss_graphic *graphic,
	enum Cmiss_graphic_type graphic_type);

int Cmiss_graphic_set_face(struct Cmiss_graphic *graphic, int face);

int Cmiss_graphic_set_exterior(struct Cmiss_graphic *graphic,
	int exterior);

/***************************************************************************//**
 * Returns the type of elements used by the graphic.
 * For <graphic> type CMISS_GRAPHIC_ELEMENT_POINTS and
 * CMISS_GRAPHIC_ISO_SURFACES only.
 */
enum Use_element_type Cmiss_graphic_get_use_element_type(
	struct Cmiss_graphic *graphic);

/***************************************************************************//**
 * Returns the label_field and font used by <graphic>. For graphic types
 * CMISS_GRAPHIC_NODE_POINTS, CMISS_GRAPHIC_DATA_POINTS and
 * CMISS_GRAPHIC_ELEMENT_POINTS only.
 */
int Cmiss_graphic_get_label_field(struct Cmiss_graphic *graphic,
	struct Computed_field **label_field, struct Graphics_font **font);


int Cmiss_graphic_set_label_field(
	struct Cmiss_graphic *graphic,struct Computed_field *label_field,
	struct Graphics_font *font);

int Cmiss_graphic_get_visibility_field(struct Cmiss_graphic *graphic,
	struct Computed_field **visibility_field);

int Cmiss_graphic_set_visibility_field(
	struct Cmiss_graphic *graphic,struct Computed_field *visibility_field);

int Cmiss_graphic_to_graphics_object(
	struct Cmiss_graphic *graphic,void *graphic_to_object_data_void);

/***************************************************************************//** 
 * If the settings visibility flag is set and it has a graphics_object, the
 * graphics_object is compiled.
 * @param graphic The graphic to be edit
 * @param context_void Void pointer to the GT_object_compile_context
 * @return If successfully compile visible setting returns 1, else 0
 */
int Cmiss_graphic_compile_visible_graphic(
	struct Cmiss_graphic *graphic, void *renderer_void);

/***************************************************************************//**
 * Notifies the <graphic> that the glyph used has changed.
 */
int Cmiss_graphic_glyph_change(
	struct GT_object *glyph,void *graphic_void);

/***************************************************************************//** 
 * If the settings visibility flag is set and it has a graphics_object, the
 * graphics_object is executed, while the position of the settings in the list
 * is put out as a name to identify the object in OpenGL picking.
 * @param graphic The graphic to be edit
 * @param dummy_void void pointer to NULL
 * @return If successfully execute visible graphic returns 1, else 0
 */
int Cmiss_graphic_execute_visible_graphic(
	struct Cmiss_graphic *graphic, void *renderer_void);

int Cmiss_graphic_get_visible_graphics_object_range(
	struct Cmiss_graphic *graphic,void *graphic_range_void);

struct GT_object *Cmiss_graphic_get_graphics_object(
	struct Cmiss_graphic *graphic);

/***************************************************************************//**
 * Returns true if the particular <graphic_type> can deal with nodes/elements of
 * the given <dimension>. Note a <dimension> of -1 is taken to mean any dimension.
 */
int Cmiss_graphic_type_uses_dimension(
	enum Cmiss_graphic_type graphic_type, int dimension);

/***************************************************************************//**
 * Returns the enumerator determining whether names are output with the graphics
 * for the settings, and if so which graphics are output depending on their
 * selection status.
 */
enum Graphics_select_mode Cmiss_graphic_get_select_mode(
	struct Cmiss_graphic *graphic);

/***************************************************************************//**
 * Sets the enumerator determining whether names are output with the graphics
 * for the settings, and if so which graphics are output depending on their
 * selection status.
 */
int Cmiss_graphic_set_select_mode(struct Cmiss_graphic *graphic,
	enum Graphics_select_mode select_mode);

/***************************************************************************//**
 * Returns the field supplying coordinates for the graphic.
 *
 * @param graphic  The graphic to query.
 * @return  The coordinate field pointer, or NULL if none or invalid graphic.
 */
struct Computed_field *Cmiss_graphic_get_coordinate_field(
	struct Cmiss_graphic *graphic);

/***************************************************************************//**
 * Returns the material used by <graphic>.
 */
struct Graphical_material *Cmiss_graphic_get_material(
	struct Cmiss_graphic *graphic);

/***************************************************************************//**
 * Returns the selected material used by <graphic>.
 */
struct Graphical_material *Cmiss_graphic_get_selected_material(
	struct Cmiss_graphic *graphic);

/***************************************************************************//**
 * Returns parameters used for colouring the graphics created from <graphic> with
 * the <data_field> by the <spectrum>.
 * For type CMISS_GRAPHIC_STREAMLINES use function
 * Cmiss_graphic_get_data_spectrum_parameters_streamlines instead.
 */
int Cmiss_graphic_get_data_spectrum_parameters(
	struct Cmiss_graphic *graphic,
	struct Computed_field **data_field,struct Spectrum **spectrum);

/***************************************************************************//**
 * Sets parameters used for colouring the graphics created from <graphic> with
 * the <data_field> (currently only a scalar) by the <spectrum>.
 * For type CMISS_GRAPHIC_STREAMLINES use function
 * Cmiss_graphic_set_data_spectrum_parameters_streamlines instead.
 */
int Cmiss_graphic_set_data_spectrum_parameters(
	struct Cmiss_graphic *graphic,struct Computed_field *data_field,
	struct Spectrum *spectrum);

/***************************************************************************//**
 * Special version of Cmiss_graphic_get_data_spectrum_parameters for type
 * CMISS_GRAPHIC_STREAMLINES only - handles the extended range of scalar data
 * options for streamlines - eg. STREAM_TRAVEL_SCALAR.
 */
int Cmiss_graphic_get_data_spectrum_parameters_streamlines(
	struct Cmiss_graphic *graphic,
	enum Streamline_data_type *streamline_data_type,
	struct Computed_field **data_field,struct Spectrum **spectrum);

/***************************************************************************//**
 * Special version of Cmiss_graphic_set_data_spectrum_parameters for type
 * CMISS_GRAPHIC_STREAMLINES only - handles the extended range of scalar data
 * options for streamlines - eg. STREAM_TRAVEL_SCALAR.
 */
int Cmiss_graphic_set_data_spectrum_parameters_streamlines(
	struct Cmiss_graphic *graphic,
	enum Streamline_data_type streamline_data_type,
	struct Computed_field *data_field,struct Spectrum *spectrum);

/***************************************************************************//**
 * Returns 1 if <graphic> is only using exterior elements.
 * For 1-D and 2-D graphic types only.
 */
int Cmiss_graphic_get_exterior(struct Cmiss_graphic *graphic);

/***************************************************************************//**
 * Returns 1 if the graphic refers to a particular face, while the face number,
 * where 0 is xi1=0, 1 is xi1=1, 2 is xi2=0 etc. returned in <face>.
 * For 1-D and 2-D graphic types only.
 */
int Cmiss_graphic_get_face(struct Cmiss_graphic *graphic,int *face);

/***************************************************************************//**
 * Returns the texture coordinate field used by <graphic>.
 */
struct Computed_field *Cmiss_graphic_get_texture_coordinate_field(
	struct Cmiss_graphic *graphic);

/***************************************************************************//**
 * Returns the fixed discretization <graphic>.
 */
int Cmiss_graphic_get_discretization(struct Cmiss_graphic *graphic,
	struct Element_discretization *discretization);

int Cmiss_graphic_set_discretization(
  struct Cmiss_graphic *graphic, struct Element_discretization *discretization);

/***************************************************************************//**
 * Fills the top_level_number_in_xi array with the discretization computed for
 * the graphic taking into account the tessellation, non-linearity of the
 * coordinate field and fixed discretization, if supported for graphic type.
 *
 * @param max_dimensions  Size of supplied top_level_number_in_xi array.
 * @param top_level_number_in_xi  Array to receive values.
 */
int Cmiss_graphic_get_top_level_number_in_xi(struct Cmiss_graphic *graphic,
	int max_dimensions, int *top_level_number_in_xi);

/***************************************************************************//**
 * Returns the native_discretization field used by <graphic>.
 * For type CMISS_GRAPHIC_ELEMENT_POINTS only.
 */
struct FE_field *Cmiss_graphic_get_native_discretization_field(
	struct Cmiss_graphic *graphic);

int Cmiss_graphic_set_native_discretization_field(
      struct Cmiss_graphic *graphic, struct FE_field *native_discretization_field);

int Cmiss_graphic_get_circle_discretization(struct Cmiss_graphic *graphic);

int Cmiss_graphic_set_circle_discretization(
	struct Cmiss_graphic *graphic, int circle_discretization);

/***************************************************************************//**
 * Returns the current glyph and parameters for orienting and scaling it.
 */
int Cmiss_graphic_get_glyph_parameters(
	struct Cmiss_graphic *graphic,
	struct GT_object **glyph, enum Graphic_glyph_scaling_mode *glyph_scaling_mode,
	Triple glyph_centre, Triple glyph_size,
	struct Computed_field **orientation_scale_field, Triple glyph_scale_factors,
	struct Computed_field **variable_scale_field);

/***************************************************************************//**
 * Sets the glyph and parameters for orienting and scaling it.
 * See function make_glyph_orientation_scale_axes in
 * finite_element/finite_element_to_graphics object for explanation of how the
 * <orientation_scale_field> is used.
 */
int Cmiss_graphic_set_glyph_parameters(
	struct Cmiss_graphic *graphic,
	struct GT_object *glyph, enum Graphic_glyph_scaling_mode glyph_scaling_mode,
	Triple glyph_centre, Triple glyph_size,
	struct Computed_field *orientation_scale_field, Triple glyph_scale_factors,
	struct Computed_field *variable_scale_field);

/***************************************************************************//**
 * Returns parameters for the iso_surface: iso_scalar_field = iso_value.
 * Either the iso_values are stored in the array <iso_values> or they are
 * distributed from <first_iso_value> to <last_iso_value>.
 * For graphic_type GT_ELEMENT_GRAPHIC_ISO_SURFACES only.
 */
int Cmiss_graphic_get_iso_surface_parameters(
	struct Cmiss_graphic *graphic,struct Computed_field **iso_scalar_field,
	int *number_of_iso_values, double **iso_values,
	double *first_iso_value, double *last_iso_value, 
	double *decimation_threshold);

/***************************************************************************//**
 * Sets parameters for the iso_surface: iso_scalar_field = iso_value.
 * Either the iso_values are stored in the array <iso_values> or they are
 * distributed from <first_iso_value> to <last_iso_value>.
 * For settings_type CMISS_GRAPHIC_ISO_SURFACES only - must call this after
 * CREATE to define a valid iso_surface.
 */
int Cmiss_graphic_set_iso_surface_parameters(
	struct Cmiss_graphic *graphic,struct Computed_field *iso_scalar_field,
	int number_of_iso_values, double *iso_values,
	double first_iso_value, double last_iso_value,
	double decimation_threshold);

/***************************************************************************//**
 * Returns the current radius parameters which are used in the expression:
 * radius = constant_radius + radius_scale_factor*radius_scalar_field.
 *For graphic_type CMISS_GRAPHIC_CYLINDERS only.
 */
int Cmiss_graphic_get_radius_parameters(
	struct Cmiss_graphic *graphic,float *constant_radius,
	float *radius_scale_factor,struct Computed_field **radius_scalar_field);

/***************************************************************************//**
 * Sets the current radius parameters which are used in the expression:
 * radius = constant_radius + radius_scale_factor*radius_scalar_field.
 * For settings_type CMISS_GRAPHIC_CYLINDERS only.
 * @param graphic cmiss graphic to be modified
 * @param constant_radius constant radius of the cylinder
 * @param radius_scale_factor scale factor of radius
 * @param radius_scalar_field computed field used to scale the radius
 * @return If succesfully set the radius parameters returns 1, else 0
 */
int Cmiss_graphic_set_radius_parameters(
	struct Cmiss_graphic *graphic,float constant_radius,
	float radius_scale_factor,struct Computed_field *radius_scalar_field);

/***************************************************************************//**
 * For graphic starting in a particular element.
 */
struct FE_element *Cmiss_graphic_get_seed_element(
	struct Cmiss_graphic *graphic);

int Cmiss_graphic_set_seed_element(struct Cmiss_graphic *graphic,
	struct FE_element *seed_element);

/***************************************************************************//**
 * For graphic_types CMISS_GRAPHIC_ELEMENT_POINTS or
 * CMISS_GRAPHIC_STREAMLINES only.
 */
int Cmiss_graphic_get_seed_xi(struct Cmiss_graphic *graphic,
	Triple seed_xi);

int Cmiss_graphic_set_seed_xi(struct Cmiss_graphic *graphic,
	Triple seed_xi);

/***************************************************************************//**
 * Get the graphic line width.  If it is 0 then the line will use the scene default.
 */
int Cmiss_graphic_get_line_width(struct Cmiss_graphic *graphic);

int Cmiss_graphic_set_line_width(struct Cmiss_graphic *graphic, int line_width);

/***************************************************************************//**
 * For graphic_type CMISS_RENDITION_STREAMLINES only.
 */
int Cmiss_graphic_get_streamline_parameters(
	struct Cmiss_graphic *graphic,enum Streamline_type *streamline_type,
	struct Computed_field **stream_vector_field,int *reverse_track,
	float *streamline_length,float *streamline_width);

/***************************************************************************//**
 * For graphic_type CMISS_RENDITION_STREAMLINES only.
 */
int Cmiss_graphic_set_streamline_parameters(
	struct Cmiss_graphic *graphic,enum Streamline_type streamline_type,
	struct Computed_field *stream_vector_field,int reverse_track,
	float streamline_length,float streamline_width);

/***************************************************************************//**
 * Sets the type of elements used by the graphic.
 * For <graphic> type CMISS_GRAPHIC_ELEMENT_POINTS and
 * CMISS_GRAPHIC_ISO_SURFACES only.
 */
int Cmiss_graphic_set_use_element_type(
	struct Cmiss_graphic *graphic,enum Use_element_type use_element_type);

/***************************************************************************//**
 * Returns the <xi_discretization_mode> and <xi_point_density_field> controlling
 * where glyphs are displayed for <graphic> of type
 * CMISS_GRAPHIC_ELEMENT_POINTS. <xi_point_density_field> is used only with
 * XI_DISCRETIZATION_CELL_DENSITY and XI_DISCRETIZATION_CELL_POISSON modes.
 * Either <xi_discretization_mode> or <xi_point_density_field> addresses may be
 * omitted, if that value is not required.
 */
int Cmiss_graphic_get_xi_discretization(
	struct Cmiss_graphic *graphic,
	enum Xi_discretization_mode *xi_discretization_mode,
	struct Computed_field **xi_point_density_field);

/***************************************************************************//**
 * Sets the xi_discretization_mode controlling where glyphs are displayed for
 * <graphic> of type CMISS_GRAPHIC_ELEMENT_POINTS. Must supply a scalar
 * <xi_point_density_field> if the new mode is XI_DISCRETIZATION_CELL_DENSITY or
 * XI_DISCRETIZATION_CELL_POISSON.
 */
int Cmiss_graphic_set_xi_discretization(
	struct Cmiss_graphic *graphic,
	enum Xi_discretization_mode xi_discretization_mode,
	struct Computed_field *xi_point_density_field);

/***************************************************************************//**
 * Copies the cmiss_graphic contents from source to destination.
 * Notes:
 * destination->access_count is not changed by COPY.
 * graphics_object is NOT copied; destination->graphics_object is cleared.
 * @param destination Copy the graphic to this address
 * @param source the source cmiss graphic
 * @return If successfully copy the graphic returns 1, else 0
 */
int Cmiss_graphic_copy_without_graphics_object(
	struct Cmiss_graphic *destination, struct Cmiss_graphic *source);

/***************************************************************************//** 
 * Graphic iterator function returning true if <graphic> has the 
 * specified <name>.  If the graphic doesn't have a name then the position
 * number is converted to a string and that is compared to the supplied <name>.
 * @param graphic The graphic
 * @param name_void void pointer to name
 * @return If graphic has name <name> returns 1, else 0
 */
int Cmiss_graphic_has_name(struct Cmiss_graphic *graphic,
	void *name_void);

/***************************************************************************//**
 * LIST(Cmiss_graphic) conditional function returning 1 if the two
 * graphics describe the same geometry.
 */
int Cmiss_graphic_same_geometry(struct Cmiss_graphic *graphic,
	void *second_graphic_void);

/***************************************************************************//**
 * Cmiss_graphic list conditional function returning 1 if graphic has the
 * name.
 */
int Cmiss_graphic_same_name(struct Cmiss_graphic *graphic,
	void *name_void);

/***************************************************************************//**
 * Cmiss_graphic list conditional function returning 1 if the two
 * graphics have the same name or describe EXACTLY the same geometry.
 */
int Cmiss_graphic_same_name_or_geometry(struct Cmiss_graphic *graphic,
	void *second_graphic_void);

/***************************************************************************//**
 * Executes a GFX MODIFY RENDITION NODE_POINTS command.
 * If return_code is 1, returns the completed Modify_g_element_data with the
 * parsed graphic. Note that the graphic are ACCESSed once on valid return.
 */
int gfx_modify_rendition_node_points(struct Parse_state *state,
	void *modify_rendition_data_void,void *rendition_command_data_void);

/***************************************************************************//**
 * Executes a GFX MODIFY RENDITION DATA_POINTS command.
 * If return_code is 1, returns the completed Modify_rendition_data with the
 * parsed graphic. Note that the graphic are ACCESSed once on valid return.
 */
int gfx_modify_rendition_data_points(struct Parse_state *state,
	void *modify_rendition_data_void,void *rendition_command_data_void);

/***************************************************************************//**
 * Executes a GFX MODIFY RENDITION LINES command.
 * If return_code is 1, returns the completed Modify_rendition_data with the
 * parsed graphic. Note that the graphic are ACCESSed once on valid return.
 */
int gfx_modify_rendition_lines(struct Parse_state *state,
	void *modify_rendition_data_void,void *rendition_command_data_void);

/***************************************************************************//**
 * Executes a GFX MODIFY RENDITION CYLINDERS command.
 * If return_code is 1, returns the completed Modify_rendition_data with the
 * parsed graphic. Note that the graphic are ACCESSed once on valid return.
 */
int gfx_modify_rendition_cylinders(struct Parse_state *state,
	void *modify_rendition_data_void,void *rendition_command_data_void);

/***************************************************************************//** 
 * Executes a GFX MODIFY RENDITION SURFACES command.
 * If return_code is 1, returns the completed Modify_rendition_data with the
 * parsed graphic. Note that the graphic are ACCESSed once on valid return.
 * @param state Parse state
 * @param modify_rendition_data_void void pointer to a container object
 * @param command_data_void void pointer to a container object
 * @return if successfully modify surface returns 1, else 0
 */
int gfx_modify_rendition_surfaces(struct Parse_state *state,
	void *modify_rendition_data_void,void *rendition_command_data_void);

/***************************************************************************//**
 * Executes a GFX MODIFY RENDITION ISO_SURFACES command.
 * If return_code is 1, returns the completed Modify_rendition_data with the
 * parsed graphic. Note that the graphic are ACCESSed once on valid return.
 */
int gfx_modify_rendition_iso_surfaces(struct Parse_state *state,
	void *modify_rendition_data_void,void *rendition_command_data_void);

/***************************************************************************//**
 * Executes a GFX MODIFY RENDITION ELEMENT_POINTS command.
 * If return_code is 1, returns the completed Modify_rendition_data with the
 * parsed graphic. Note that the graphic are ACCESSed once on valid return.
 */
int gfx_modify_rendition_element_points(struct Parse_state *state,
	void *modify_rendition_data_void,void *rendition_command_data_void);

int gfx_modify_rendition_point(struct Parse_state *state,
		void *modify_rendition_data_void,void *rendition_command_data_void);

/***************************************************************************//**
 * Executes a GFX MODIFY RENDITION STREAMLINES command.
 * If return_code is 1, returns the completed Modify_rendition_data with the
 * parsed graphic. Note that the graphic are ACCESSed once on valid return.
 */
int gfx_modify_rendition_streamlines(struct Parse_state *state,
	void *modify_rendition_data_void,void *rendition_command_data_void);

/***************************************************************************//**
 * Writes out the <graphic> as a text string in the command window with the
 * <graphic_string_detail>, <line_prefix> and <line_suffix> given in the
 * <list_data>.
 */
int Cmiss_graphic_list_contents(struct Cmiss_graphic *graphic,
	void *list_data_void);

/***************************************************************************//**
 * Returns the position of <grpahic> in <list_of_grpahic>.
 */
int Cmiss_graphic_get_position_in_list(
	struct Cmiss_graphic *graphic,
	struct LIST(Cmiss_graphic) *list_of_graphic);

/***************************************************************************//**
 * Returns true if <graphic1> and <graphic2> would produce identical graphics.
 */
int Cmiss_graphic_match(struct Cmiss_graphic *graphic1,
	struct Cmiss_graphic *graphic2);

/***************************************************************************//**
 * Cmiss_graphic iterator function for copying a list_of_graphic.
 * Makes a copy of the graphic and puts it in the list_of_graphic.
 */
int Cmiss_graphic_copy_and_put_in_list(
	struct Cmiss_graphic *graphic,void *list_of_graphic_void);

/***************************************************************************//**
 *Returns 1 if the graphic are of the specified graphic_type.
 */
int Cmiss_graphic_type_matches(struct Cmiss_graphic *graphic,
	void *graphic_type_void);

/***************************************************************************//**
 * If <graphic> does not already have a graphics object, this function attempts
 * to find graphic in <list_of_graphic> which differ only trivially in material,
 * spectrum etc. AND have a graphics object. If such a graphic is found, the
 * graphics_object is moved from the matching graphic and put in <graphic>, while
 * any trivial differences are fixed up in the graphics_obejct.
 */
int Cmiss_graphic_extract_graphics_object_from_list(
	struct Cmiss_graphic *graphic,void *list_of_graphic_void);

/***************************************************************************//**
 * Same as Cmiss_graphic_same_non_trivial except <graphic> must also have
 * a graphics_object. Used for getting graphics objects from previous graphic
 * that are the same except for trivial differences such as the material and
 * spectrum which can be changed in the graphics object to match the new graphic .
 */
int Cmiss_graphic_same_non_trivial_with_graphics_object(
	struct Cmiss_graphic *graphic,void *second_graphic_void);

/***************************************************************************//**
 * For graphic_type CMISS_GRAPHIC_VOLUMES only.
 */
int Cmiss_graphic_set_volume_texture(struct Cmiss_graphic *graphic,
	struct VT_volume_texture *volume_texture);

int Cmiss_graphic_get_name_internal(struct Cmiss_graphic *graphic,
	const char **name_ptr);

int Cmiss_graphic_time_change(
	struct Cmiss_graphic *graphic,void *dummy_void);

int Cmiss_graphic_update_time_behaviour(
	struct Cmiss_graphic *graphic, void *update_time_behaviour_void);

int Cmiss_graphic_FE_region_change(
	struct Cmiss_graphic *graphic, void *data_void);

int Cmiss_graphic_data_FE_region_change(
	struct Cmiss_graphic *graphic, void *data_void);

int Cmiss_graphic_Computed_field_change(
	struct Cmiss_graphic *graphic, void *change_data_void);


struct Cmiss_graphic_FE_region_change_data
{
	/* changes to fields with summary */
	int fe_field_change_summary;
	struct CHANGE_LOG(FE_field) *fe_field_changes;
	/* changes to nodes with summary and number_of_changes */
	int fe_node_change_summary;
	int number_of_fe_node_changes;
	struct CHANGE_LOG(FE_node) *fe_node_changes;
	/* changes to elements with summary and number_of_changes */
	int fe_element_change_summary[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	int number_of_fe_element_changes[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	struct CHANGE_LOG(FE_element) *fe_element_changes[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	FE_value time;
	/* following set if changes affect any of the graphicsS */
	int graphics_changed;
	/* the FE_region the settings apply to */
	struct FE_region *fe_region;
	int element_type;
}; /* struct Cmiss_graphic_FE_region_change_data */

/***************************************************************************//**
 * Data to pass to Cmiss_graphics_material_change.
 */
struct Cmiss_graphics_material_change_data
{
	struct MANAGER_MESSAGE(Graphical_material) *manager_message;
	int graphics_changed;
};

/***************************************************************************//**
 * Inform graphic of changes in the material manager. Marks affected
 * graphics for rebuilding and sets flag for informing clients of rendition.
 * Note: only graphics combining a material with data/spectrum are updated;
 * pure material changes do not require update.
 *
 * @param material_change_data_void  Cmiss_graphics_material_change_data.
 */
int Cmiss_graphics_material_change(struct Cmiss_graphic *graphic,
	void *material_change_data_void);

/***************************************************************************//**
 * Data to pass to Cmiss_graphic_spectrum_change.
 */
struct Cmiss_graphic_spectrum_change_data
{
	struct MANAGER_MESSAGE(Spectrum) *manager_message;
	int graphics_changed;
};

/***************************************************************************//**
 * Inform graphic of changes in the spectrum manager. Marks affected
 * graphics for rebuilding and sets flag for informing clients of rendition.
 *
 * @param spectrum_change_data_void  Cmiss_graphic_spectrum_change_data.
 */
int Cmiss_graphic_spectrum_change(struct Cmiss_graphic *graphic,
	void *spectrum_change_data_void);

/***************************************************************************//**
 * Data to pass to Cmiss_graphic_tessellation_change.
 */
struct Cmiss_graphic_tessellation_change_data
{
	struct MANAGER_MESSAGE(Cmiss_tessellation) *manager_message;
	int graphics_changed;
};

/***************************************************************************//**
 * Inform graphic of changes in the tessellation manager. Marks affected
 * graphics for rebuilding and sets flag for informing clients of rendition.
 *
 * @param tessellation_change_data_void  Cmiss_graphic_tessellation_change_data.
 */
int Cmiss_graphic_tessellation_change(struct Cmiss_graphic *graphic,
	void *tessellation_change_data_void);

int Cmiss_graphic_set_customised_graphics_object(
	struct Cmiss_graphic *graphic, struct GT_object *graphics_object);

/* Overlay disabled
int Cmiss_graphic_enable_overlay(struct Cmiss_graphic *graphic, int flag);

int Cmiss_graphic_is_overlay(struct Cmiss_graphic *graphic);

int Cmiss_graphic_set_overlay_order(struct Cmiss_graphic *graphic, int order);

int Cmiss_graphic_get_overlay_order(struct Cmiss_graphic *graphic);
*/

/***************************************************************************//**
 * This function will deaccess any computed fields being used by graphic, this
 * should only be called from Cmiss_rendition_detach_fields.
 *
 * @param cmiss_graphic  pointer to the graphic.
 *
 * @return Return 1 if successfully detach fields from graphic otherwise 0.
 */
int Cmiss_graphic_detach_fields(struct Cmiss_graphic *graphic, void *dummy_void);

struct Cmiss_rendition *Cmiss_graphic_get_rendition_private(struct Cmiss_graphic *graphic);

int Cmiss_graphic_set_rendition_private(struct Cmiss_graphic *graphic,
	struct Cmiss_rendition *rendition);

int Cmiss_graphic_selected_element_points_change(
	struct Cmiss_graphic *graphic,void *dummy_void);

/***************************************************************************//**
 * A function to call set_rendition_private but with a void pointer to the
 * rendition passing into the function for list macro.
 *
 * @param cmiss_graphic  pointer to the graphic.
 * @param rendition_void  void pointer to the rendition.
 * @return Return 1 if successfully set the graphic.
 */
int Cmiss_graphic_set_rendition_for_list_private(
		struct Cmiss_graphic *graphic, void *rendition_void);

int Cmiss_graphic_update_selected(struct Cmiss_graphic *graphic, void *dummy_void);

#endif
