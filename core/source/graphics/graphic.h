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

#include "zinc/fieldgroup.h"
#include "zinc/graphic.h"
#include "computed_field/computed_field.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_to_graphics_object.h"
#include "graphics/auxiliary_graphics_types.h"
#include "graphics/element_point_ranges.h"
#include "graphics/font.h"
#include "graphics/graphics_object.h"
#include "general/enumerator.h"
#include "general/list.h"
#include "graphics/material.h"
#include "graphics/spectrum.h"

struct Cmiss_graphic_point_attributes;
struct Cmiss_graphic_line_attributes;

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
	struct Computed_field *subgroup_field;
	struct Computed_field *coordinate_field;
	enum Graphics_select_mode select_mode;
	enum Cmiss_field_domain_type domain_type;

	/* for 1-D and 2-D elements only */
	bool exterior;
	/* face number is from -1 to 5, where -1 is none/all, 0 is xi1=0, 1 is xi1=1,
		 2 is xi2=0 etc. */
	Cmiss_element_face_type face;
	/* For surfaces only at the moment */
	struct Computed_field *texture_coordinate_field;

	/* line attributes */
	FE_value line_base_size[3];
	FE_value line_scale_factors[3];
	Cmiss_field_id line_orientation_scale_field;

	/* for contours only */
	struct Computed_field *isoscalar_field;
	int number_of_isovalues;
	/* If the isovalues array is set then these values are used,
		otherwise number_of_isovalues values are distributed from
		first_isovalue to last_isovalue including these values for n>1 */
	double *isovalues, first_isovalue, last_isovalue,
		decimation_threshold;

	/* point attributes */
	struct GT_object *glyph;
	enum Cmiss_glyph_repeat_mode glyph_repeat_mode;
	FE_value point_offset[3];
	FE_value point_base_size[3];
	FE_value point_scale_factors[3];
	FE_value label_offset[3];
	char *label_text[3];
	struct Computed_field *point_orientation_scale_field;
	struct Computed_field *signed_scale_field;
	struct Computed_field *label_field;
	struct Computed_field *label_density_field;

	/* for element_points only */
	enum Xi_discretization_mode xi_discretization_mode;
	struct Computed_field *xi_point_density_field;
	struct FE_field *native_discretization_field;
	struct Cmiss_tessellation *tessellation;
	int overlay_flag;
	int overlay_order;
	/* for settings starting in a particular element */
	struct FE_element *seed_element;
	/* for settings requiring an exact xi location */
	Triple seed_xi;
	enum Streamline_type streamline_type;
	struct Computed_field *stream_vector_field;
	int reverse_track;
	GLfloat streamline_length;
	enum Streamline_data_type streamline_data_type;
	/* streamline seed nodeset and field giving mesh location */
	Cmiss_nodeset_id seed_nodeset;
	struct Computed_field *seed_node_mesh_location_field;

	/* appearance settings */
	/* for all graphic types */
	bool visibility_flag;
	struct Graphical_material *material, *selected_material,
		*secondary_material;
	struct Computed_field *data_field;
	struct Spectrum *spectrum;
	int autorange_spectrum_flag;
	/* for glyphsets */
	struct Cmiss_font *font;
	/* for surfaces */
	enum Cmiss_graphics_render_type render_type;
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
	enum Cmiss_graphics_coordinate_system coordinate_system;
// 	/* for accessing objects */
	int access_count;

	inline Cmiss_graphic *access()
	{
		++access_count;
		return this;
	}

	static inline int deaccess(Cmiss_graphic **graphic_address);

};

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
	CMISS_GRAPHIC_ATTRIBUTE_EXTERIOR_FLAG,
	CMISS_GRAPHIC_ATTRIBUTE_FACE,
	CMISS_GRAPHIC_ATTRIBUTE_GLYPH,
	CMISS_GRAPHIC_ATTRIBUTE_LABEL_FIELD,
	CMISS_GRAPHIC_ATTRIBUTE_LINE_WIDTH,
	CMISS_GRAPHIC_ATTRIBUTE_NATIVE_DISCRETIZATION_FIELD,
	CMISS_GRAPHIC_ATTRIBUTE_RENDER_TYPE,
	CMISS_GRAPHIC_ATTRIBUTE_TESSELLATION,
	CMISS_GRAPHIC_ATTRIBUTE_TEXTURE_COORDINATE_FIELD,
	CMISS_GRAPHIC_ATTRIBUTE_XI_DISCRETIZATION_MODE,
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

struct Cmiss_graphic_to_graphics_object_data
{
	Cmiss_field_cache_id field_cache;
	/* graphics object names are preceded by this */
	const char *name_prefix;
	/* default_rc_coordinate_field to use if NULL in any settings */
	struct Computed_field *rc_coordinate_field,
		*wrapper_orientation_scale_field,*wrapper_stream_vector_field,
		*selection_group_field;
	struct Cmiss_region *region;
	Cmiss_field_module_id field_module;
	struct FE_region *fe_region;
	struct FE_region *data_fe_region;
	// master mesh being converted into graphics:
	Cmiss_mesh_id master_mesh;
	// different from master mesh if iterating over group; can ignore subgroup_field if set:
	Cmiss_mesh_id iteration_mesh;
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
	struct Cmiss_graphics_filter *graphics_filter;
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
	int modify_this_graphic;
	Cmiss_field_group_id group; // optional group field for migrating group regions
}; /* struct Modify_graphic_data */

/***************************************************************************//**
 * Subset of command data passed to rendition modify routines.
 */
struct Rendition_command_data
{
	struct Cmiss_graphics_module *graphics_module;
	struct Cmiss_rendition *rendition;
	struct Graphical_material *default_material;
	struct Cmiss_font *default_font;
	Cmiss_glyph_module_id glyph_module;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Cmiss_region *region;
	/* root_region used for seeding streamlines from the nodes in a region */
	struct Cmiss_region *root_region;
	Cmiss_graphics_material_module_id material_module;
	Cmiss_tessellation_module_id tessellation_module;
	struct Spectrum *default_spectrum;
	struct MANAGER(Spectrum) *spectrum_manager;
}; /* struct Rendition_command_data */

struct Cmiss_graphic_Computed_field_change_data
{
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

int Cmiss_graphic::deaccess(Cmiss_graphic **graphic_address)
{
	return DEACCESS(Cmiss_graphic)(graphic_address);
}

/***************************************************************************//**
 * Returns a string summarising the graphic type, name and subset field.
 * @param graphic  The graphic.
 * @return  Allocated string.
 */
char *Cmiss_graphic_get_summary_string(struct Cmiss_graphic *graphic);

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

/**
 * Returns the dimension of the domain used in the graphic. Note for domain
 * type CMISS_DOMAIN_ELEMENTS_HIGHEST_DIMENSION the region is checked for the
 * highest dimension.
 * @param graphic  The graphic to query.
 * @return  The dimension of the graphic domain, or -1 on error.
 */
int Cmiss_graphic_get_domain_dimension(struct Cmiss_graphic *graphic);

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

/**
 * Gets the streamline data type which allows extended options for colouring
 * streamlines beyond the data field, particularly STREAM_TRAVEL_SCALAR.
 */
enum Streamline_data_type Cmiss_graphic_get_streamline_data_type(
	Cmiss_graphic_id graphic);

/**
 * Sets the streamline data type which allows extended options for colouring
 * streamlines beyond the data field, particularly STREAM_TRAVEL_SCALAR.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
int Cmiss_graphic_set_streamline_data_type(Cmiss_graphic_id graphic,
	enum Streamline_data_type streamline_data_type);

/**
 * Fills the top_level_number_in_xi array with the discretization computed for
 * the graphic taking into account the tessellation and non-linearity of the
 * coordinate field.
 *
 * @param max_dimensions  Size of supplied top_level_number_in_xi array.
 * @param top_level_number_in_xi  Array to receive values.
 */
int Cmiss_graphic_get_top_level_number_in_xi(struct Cmiss_graphic *graphic,
	int max_dimensions, int *top_level_number_in_xi);

/**
 * Returns the native_discretization field used by <graphic>.
 * For type CMISS_GRAPHIC_ELEMENT_POINTS only.
 */
struct FE_field *Cmiss_graphic_get_native_discretization_field(
	struct Cmiss_graphic *graphic);

int Cmiss_graphic_set_native_discretization_field(
	  struct Cmiss_graphic *graphic, struct FE_field *native_discretization_field);

/**
 * Get iso surface decimation threshold.
 */
double Cmiss_graphic_contours_get_decimation_threshold(
	Cmiss_graphic_contours_id contours_graphic);

/**
 * Set iso surface decimation threshold, controlling polygon reduction.
 * Converts iso surface into a voltex first.
 * @return  Status CMISS_OK on success, otherwise CMISS_ERROR_ARGUMENT.
 */
int Cmiss_graphic_contours_set_decimation_threshold(
	Cmiss_graphic_contours_id contours_graphic, double decimation_threshold);

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
	GLfloat *streamline_length);

/***************************************************************************//**
 * For graphic_type CMISS_RENDITION_STREAMLINES only.
 */
int Cmiss_graphic_set_streamline_parameters(
	struct Cmiss_graphic *graphic,enum Streamline_type streamline_type,
	struct Computed_field *stream_vector_field,int reverse_track,
	GLfloat streamline_length);

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
 * Cmiss_graphic list conditional function returning 1 if graphic has the
 * name.
 */
int Cmiss_graphic_same_name(struct Cmiss_graphic *graphic,
	void *name_void);

/***************************************************************************//**
 * Writes out the <graphic> as a text string in the command window with the
 * <graphic_string_detail>, <line_prefix> and <line_suffix> given in the
 * <list_data>.
 */
int Cmiss_graphic_list_contents(struct Cmiss_graphic *graphic,
	void *list_data_void);

/***************************************************************************//**
 * Returns the position of <graphic> in <list_of_grpahic>.
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
 * Gets the graphic's stored name, or generates one based on its position.
 * @return  Allocated string.
 */
char *Cmiss_graphic_get_name_internal(struct Cmiss_graphic *graphic);

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
 * Inform graphic of changes in the tessellation manager. Marks affected
 * graphics for rebuilding and sets flag for informing clients of rendition.
 *
 * @param tessellation_manager_message_void  Pointer to
 * struct MANAGER_MESSAGE(Cmiss_tessellation).
 */
int Cmiss_graphic_tessellation_change(struct Cmiss_graphic *graphic,
	void *tessellation_manager_message_void);

/***************************************************************************//**
 * Inform graphic of changes in the font manager. Marks affected
 * graphics for rebuilding and sets flag for informing clients of rendition.
 *
 * @param font_manager_message_void  Pointer to
 * struct MANAGER_MESSAGE(Cmiss_font).
 */
int Cmiss_graphic_font_change(struct Cmiss_graphic *graphic,
	void *font_manager_message_void);

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
