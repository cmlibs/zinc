/*******************************************************************************
FILE : scene.h

==============================================================================*/
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *LAST MODIFIED : 16 October 2008

DESCRIPTION :
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2010
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

#if !defined (SCENE_H)
#define SCENE_H

#include <list>
#include "zinc/scene.h"
#include "computed_field/computed_field.h"
#include "general/any_object.h"
#include "general/any_object_prototype.h"
#include "general/manager.h"
#include "general/object.h"
#include "graphics/graphic.h"
#include "graphics/graphics_library.h"
#include "context/context.h"
#include "region/cmiss_region.h"

typedef std::list<Cmiss_selection_handler *> Selection_handler_list;

struct Cmiss_scene
/*******************************************************************************
LAST MODIFIED : 16 October 2008

DESCRIPTION :
Structure for maintaining a graphical scene of region.
==============================================================================*/
{
	/* the [FE] region being drawn */
	struct Cmiss_region *region;
	struct FE_region *fe_region;
	struct FE_region *data_fe_region;
	int fe_region_callback_set, data_fe_region_callback_set;
	/* settings shared by whole scene */
	/* default coordinate field for graphics drawn with settings below */
	struct Computed_field *default_coordinate_field;
	/* list of objects interested in changes to the Cmiss_scene */
	struct Cmiss_scene_callback_data *update_callback_list;
	/* managers for updating graphics in response to global changes */
	struct MANAGER(Computed_field) *computed_field_manager;
	struct LIST(Cmiss_graphic) *list_of_graphics;
	void *computed_field_manager_callback_id;
	/* level of cache in effect */
	int cache;
	int changed; /**< true if scene has changed and haven't updated clients */
	/* for accessing objects */
	int access_count;
	gtMatrix *transformation;
	bool visibility_flag;
	/* transformaiton field for time dependent transformation */
	struct Computed_field *transformation_field;
	int transformation_time_callback_flag;
	/* curve approximation with line segments over elements */
	int *element_divisions;
	int element_divisions_size;
	/* number of segments used around cylinders */
	int circle_discretization;
	/* optional native_discretization for graphics drawn with settings below */
	struct FE_field *native_discretization_field;
	struct Cmiss_graphics_module *graphics_module;
	Cmiss_time_notifier *time_notifier;
	/* callback list for transformation changes */
	struct LIST(CMISS_CALLBACK_ITEM(Cmiss_scene_transformation)) *transformation_callback_list;
	struct LIST(CMISS_CALLBACK_ITEM(Cmiss_scene_top_region_change)) *top_region_change_callback_list;
	unsigned int position;
	Cmiss_field_group_id selection_group;
	int selection_removed; // flag set when selection_group cleared
	Selection_handler_list *selection_handler_list;
}; /* struct Cmiss_scene */

struct MANAGER_MESSAGE(Cmiss_tessellation);

typedef int(*Cmiss_scene_callback)(struct Cmiss_scene *scene,
	void *user_data);

DECLARE_CMISS_CALLBACK_TYPES(Cmiss_scene_transformation, struct Cmiss_scene *, \
	gtMatrix *, void);

DECLARE_CMISS_CALLBACK_TYPES(Cmiss_scene_top_region_change, struct Cmiss_scene *, \
	struct Cmiss_scene *, void);

struct Cmiss_scene *Cmiss_scene_create_internal(struct Cmiss_region *cmiss_region,
	struct Cmiss_graphics_module *graphics_module);

/** @return  Handle to graphics module. Up to caller to destroy */
Cmiss_graphics_module_id Cmiss_scene_get_graphics_module(Cmiss_scene_id scene);

/***************************************************************************//**
 * Destroy Cmiss_scene and clean up the memory it uses.
 *
 * @param  cmiss_scene_address the address to the pointer of
 *   the cmiss_scene_address to be deleted
 * @return  1 if successfully destroy cmiss_scene, otherwise 0
 */
int DESTROY(Cmiss_scene)(
	struct Cmiss_scene **cmiss_scene_address);

int Cmiss_scene_get_position(struct Cmiss_scene *scene);

int Cmiss_scene_set_position(struct Cmiss_scene *scene, unsigned int position);
/***************************************************************************//**
 * Set the position of the scene
 * @param scene to be edited
 * @param position Position to be set for the cmiss_scene
 * @return If successfully set the position, otherwise NULL
 */

/**
 * Return non-accessed handle to the scene for this region.
 * Currently, a Cmiss_region may have at most one cmiss_scene.
 * Private; do not use out of zinc library.
 * @param cmiss_region  The region of query.
 * @return Non-accessed handle to scene for region, or 0 if none.
 */
struct Cmiss_scene *Cmiss_region_get_scene_private(struct Cmiss_region *region);

/**
 * Return accessed handle to the scene for this region.
 * Currently, a Cmiss_region may have at most one cmiss_scene.
 * Currently used by cmgui, but not approved for zinc external API!
 * @param cmiss_region  The region of query.
 * @return Accessed handle to scene for region, or 0 if none.
 */
struct Cmiss_scene *Cmiss_region_get_scene_internal(struct Cmiss_region *region);

/***************************************************************************//**
 * Deaccess the scene of the region
 * @param region The region to deaccess scene from
 * @return If successfully deaccess scene from region returns 1, otherwise 0
 */
int Cmiss_region_deaccess_scene(struct Cmiss_region *region);

/***************************************************************************//**
 * Wrapper for accessing the list of graphic in <Cmiss_scene>.
 * @param cmiss_scene target for that scene
 * @param conditional_function conditional function for the list
 * @param data void pointer to data to pass into the conditional function
 * @return Return the first graphic that fullfill the conditional function
 */
struct Cmiss_graphic *Cmiss_scene_get_first_graphic_with_condition(
	struct Cmiss_scene *cmiss_scene,
	LIST_CONDITIONAL_FUNCTION(Cmiss_graphic) *conditional_function,
	void *data);

/***************************************************************************//**
 * Adds a callback routine which is called whenever a Cmiss_scene is aware of
 * changes.
 */
int Cmiss_scene_add_callback(struct Cmiss_scene *scene,
	Cmiss_scene_callback callback, void *user_data);

/***************************************************************************//**
 * Removes a callback which was added previously
 */
int Cmiss_scene_remove_callback(struct Cmiss_scene *scene,
	Cmiss_scene_callback callback, void *user_data);

/***************************************************************************//**
 * Attempt to guess which field is the most appropriate to use as a coordinate
 * field for graphics.
 * @param scene  The scene whose graphics need a coordinate field.
 * @param domain_type  Type of domain to get coordinate field for. Not used
 * currently.
 * @return non-accessed field
 */
Cmiss_field_id Cmiss_scene_guess_coordinate_field(
	struct Cmiss_scene *scene, Cmiss_field_domain_type domain_type);

/***************************************************************************//**
 * Iterates through every material used by the scene.
 */
int Cmiss_scene_for_each_material(struct Cmiss_scene *scene,
	MANAGER_ITERATOR_FUNCTION(Graphical_material) *iterator_function,
	void *user_data);

/***************************************************************************//**
 * Lists the general graphics defined for <scene> - as a
 * set of commands that can be used to reproduce the groups appearance. The
 * <command_prefix> should generally contain "gfx modify g_element" while the
 * optional <command_suffix> may describe the scene (eg. "scene default").
 * Note the command prefix is expected to contain the name of the region.
 */
int Cmiss_scene_list_commands(struct Cmiss_scene *scene,
	const char *command_prefix, const char *command_suffix);

/**
 * Private method for informing scene of glyph manager changes.
 * Propagates changes hierarchically to child scenes to minimise messages.
 * Should only be called by Cmiss_graphics_module.
 */
void Cmiss_scene_glyph_change(struct Cmiss_scene *scene,
	struct MANAGER_MESSAGE(Cmiss_glyph) *manager_message);

/***************************************************************************//**
 * Private method for informing scene of material manager changes.
 * Should only be called by Cmiss_graphics_module.
 */
void Cmiss_scene_material_change(struct Cmiss_scene *scene,
	struct MANAGER_MESSAGE(Graphical_material) *manager_message);

/***************************************************************************//**
 * Private method for informing scene of spectrum manager changes.
 * Should only be called by Cmiss_graphics_module.
 */
void Cmiss_scene_spectrum_change(struct Cmiss_scene *scene,
	struct MANAGER_MESSAGE(Spectrum) *manager_message);

/***************************************************************************//**
 * Private method for informing scene of tessellation manager changes.
 * Should only be called by Cmiss_graphics_module.
 */
void Cmiss_scene_tessellation_change(struct Cmiss_scene *scene,
	struct MANAGER_MESSAGE(Cmiss_tessellation) *manager_message);

/***************************************************************************//**
 * Private method for informing scene of font manager changes.
 * Should only be called by Cmiss_graphics_module.
 */
void Cmiss_scene_font_change(struct Cmiss_scene *scene,
	struct MANAGER_MESSAGE(Cmiss_font) *manager_message);

int for_each_child_scene_in_scene_tree(
	struct Cmiss_scene *scene,
	int (*cmiss_scene_tree_iterator_function)(struct Cmiss_scene *scene,
		void *user_data),	void *user_data);

/***************************************************************************//**
 * Returns the position of <graphic> in <scene>.
 */
int Cmiss_scene_get_graphic_position(
	struct Cmiss_scene *scene,
	struct Cmiss_graphic *graphic);

/***************************************************************************//**
 * Returns true if <scene1> and <scene2> match in
 * main attributes of scene, graphic etc. such that they would produce
 * the same graphics.
 */
int Cmiss_scenes_match(struct Cmiss_scene *scene1,
	struct Cmiss_scene *scene2);

/***************************************************************************//**
 * Creates a Cmiss_scene that is a copy of <existing_scene> -
 * WITHOUT copying graphics objects, and WITHOUT manager and selection callbacks.
 */
struct Cmiss_scene *create_editor_copy_Cmiss_scene(
	struct Cmiss_scene *existing_scene);

int for_each_graphic_in_Cmiss_scene(
	struct Cmiss_scene *scene,
	int (*cmiss_scene_graphic_iterator_function)(struct Cmiss_graphic *graphic,
		void *user_data),	void *user_data);

/***************************************************************************//**
 * Get region of scene
 */
struct Cmiss_region *Cmiss_scene_get_region(
	struct Cmiss_scene *scene);

/***************************************************************************//**
 *Copies the Cmiss_scene contents from source to destination, keeping any
 *graphics objects from the destination that will not change with the new graphic
 *from source. Used to apply the changed Cmiss_scene from the editor to the
 *actual Cmiss_scene.
 */
int Cmiss_scene_modify(struct Cmiss_scene *destination,
	struct Cmiss_scene *source);

int Cmiss_scene_add_transformation_callback(struct Cmiss_scene *scene,
	CMISS_CALLBACK_FUNCTION(Cmiss_scene_transformation) *function, void *user_data);

int Cmiss_scene_remove_transformation_callback(
	struct Cmiss_scene *scene,
	CMISS_CALLBACK_FUNCTION(Cmiss_scene_transformation) *function, void *user_data);


int Cmiss_scene_has_transformation(struct Cmiss_scene *scene);

void Cmiss_scene_remove_time_dependent_transformation(struct Cmiss_scene *scene);

int Cmiss_scene_set_transformation_with_time_callback(struct Cmiss_scene *scene,
	struct Computed_field *transformation_field);

int Cmiss_scene_set_transformation(struct Cmiss_scene *scene,
	gtMatrix *transformation);

int Cmiss_scene_get_transformation(struct Cmiss_scene *scene,
	gtMatrix *transformation);

/***************************************************************************//**
 * This function will remove field manager and its callback to the scene.
 *
 * @param cmiss_scene  pointer to the cmiss_scene.
 *
 * @return Return 1 if successfully remove field manager and its callback
 *    from scene otherwise 0.
 */
int Cmiss_scene_remove_field_manager_and_callback(struct Cmiss_scene *scene);

/***************************************************************************//**
 * This function will deaccess any computed fields being used by the scene
 * when the scene itself is not present in any scene.
 *
 * @param cmiss_scene  pointer to the cmiss_scene.
 *
 * @return Return 1 if successfully detach fields from scene otherwise 0.
 */
int Cmiss_scene_detach_fields(struct Cmiss_scene *scene);

PROTOTYPE_OBJECT_FUNCTIONS(Cmiss_scene);

PROTOTYPE_ANY_OBJECT(Cmiss_scene);

Cmiss_field_group_id Cmiss_scene_get_or_create_selection_group(Cmiss_scene_id scene);

/***************************************************************************//**
 * Remove selection groups from scene tree if they are empty.
 */
void Cmiss_scene_flush_tree_selections(Cmiss_scene_id scene);

int Cmiss_scene_create_node_list_selection(Cmiss_scene_id scene,
	struct LIST(FE_node) *node_list);

/***************************************************************************//**
 * Set default graphic attributes depending on type, e.g. tessellation,
 * materials, etc.
 */
int Cmiss_scene_set_minimum_graphic_defaults(struct Cmiss_scene *scene,
	struct Cmiss_graphic *graphic);

/***************************************************************************//**
 * Set additional default attributes for backward compatibility with gfx modify
 * g_element commands: default coordinate, discretization etc.
 */
int Cmiss_scene_set_graphics_defaults_gfx_modify(struct Cmiss_scene *scene,
	struct Cmiss_graphic *graphic);

/***************************************************************************//**
 * Adds the <graphic> to <scene> at the given <position>, where 1 is
 * the top of the list (rendered first), and values less than 1 or greater than the
 * last position in the list cause the graphic to be added at its end, with a
 * position one greater than the last.
 *
 * @param scene  The handle to the scene.
 * @param graphic  The handle to the cmiss graphic which will be added to the
 *   scene.
 * @param pos  The position to put the target graphic to.
 * @return  Returns 1 if successfully add graphic to scene at pos, otherwise
 *   returns 0.
 */
int Cmiss_scene_add_graphic(Cmiss_scene_id scene, Cmiss_graphic_id graphic, int pos);

int list_Cmiss_scene_transformation_commands(struct Cmiss_scene *scene,
	void *command_prefix_void);

int list_Cmiss_scene_transformation(struct Cmiss_scene *scene);

int Cmiss_scene_add_selection_from_node_list(Cmiss_scene_id scene,
	struct LIST(FE_node) *node_list, int use_data);

int Cmiss_scene_remove_selection_from_node_list(Cmiss_scene_id scene,
	struct LIST(FE_node) *node_list, int use_data);

int Cmiss_scene_add_selection_from_element_list_of_dimension(Cmiss_scene_id scene,
	struct LIST(FE_element) *element_list, int dimension);

int Cmiss_scene_remove_selection_from_element_list_of_dimension(Cmiss_scene_id scene,
	struct LIST(FE_element) *element_list, int dimension);

/***************************************************************************//**
 * Returns the status of the inherited visibility flag of the scene.
 * This function returns 0 if the specified scene or any of its parents along
 * the path has the visibility flag set to 0 otherwise return 1;
 *
 * @param scene  The handle to the scene.
 * @return  1 if scene and all its parent are visible, otherwise 0.
 */
int Cmiss_scene_is_visible_hierarchical(Cmiss_scene_id scene);

/***************************************************************************//**
 * Get graphic at position <pos> in the internal graphics list of scene.
 *
 * @param scene  The handle to the scene of which the graphic is located.
 * @param pos  The position of the graphic, starting from 0.
 * @return  returns the found graphic if found at pos, otherwise returns NULL.
 */
Cmiss_graphic_id Cmiss_scene_get_graphic_at_position(
	Cmiss_scene_id scene, int pos);

/***************************************************************************//**
 * Check either region has scene or not.
 *
 * @param region  The handle to cmiss_region.
 * @return  Returns 1 if scene is found in region, otherwise 0.
 */
int Cmiss_region_has_scene(Cmiss_region_id cmiss_region);

Cmiss_field_id Cmiss_scene_get_selection_group_private_for_highlighting(
	Cmiss_scene_id scene);

int Cmiss_region_modify_scene(struct Cmiss_region *region,
	struct Cmiss_graphic *graphic, int delete_flag, int position);


/**
 * Inform scene that it has changed and must be rebuilt.
 * Unless change caching is on, informs clients of scene that it has changed.
 * For internal use only; called by changed graphic to owning scene.
 */
void Cmiss_scene_changed(struct Cmiss_scene *scene);

int Cmiss_scene_set_default_coordinate_field(
	struct Cmiss_scene *scene,
	struct Computed_field *default_coordinate_field);

////  new APIs
int Cmiss_scene_add_total_transformation_callback(struct Cmiss_scene *child_scene,
	Cmiss_scene_id scene, CMISS_CALLBACK_FUNCTION(Cmiss_scene_transformation) *function,
	CMISS_CALLBACK_FUNCTION(Cmiss_scene_top_region_change) *region_change_function,
	void *user_data);

int Cmiss_scene_remove_total_transformation_callback(struct Cmiss_scene *child_scene,
	Cmiss_scene_id scene, CMISS_CALLBACK_FUNCTION(Cmiss_scene_transformation) *function,
	CMISS_CALLBACK_FUNCTION(Cmiss_scene_top_region_change) *region_change_function,
	void *user_data);

int Cmiss_scene_notify_scene_viewer_callback(struct Cmiss_scene *scene,
	void *scene_viewer_void);

/**************************************************************************//**
 * This function will return the gtMatrix containing the total transformation
 * from the top scene to scene.
 *
 * @param cmiss_scene  pointer to the scene.
 * @param top_scene  pointer to the the top scene.
 *
 * @return Return an allocated gtMatrix if successfully get a transformation
 * 	matrix. NULL if failed or if no transformation is set.
 */
gtMatrix *Cmiss_scene_get_total_transformation(
	struct Cmiss_scene *scene, struct Cmiss_scene *top_scene);

int Cmiss_scene_get_global_graphics_range(Cmiss_scene_id top_scene,
	Cmiss_graphics_filter_id filter,
	double *centre_x, double *centre_y, double *centre_z,
	double *size_x, double *size_y, double *size_z);

typedef int(*graphics_object_tree_iterator_function)(
	struct GT_object *graphics_object, double time, void *user_data);

int for_each_graphics_object_in_scene_tree(Cmiss_scene_id scene,
	Cmiss_graphics_filter_id filter, graphics_object_tree_iterator_function iterator_function,
	void *user_data);

int Scene_export_region_graphics_object(Cmiss_scene *scene,
	Cmiss_region *region, const char *graphic_name, Cmiss_graphics_filter_id filter,
	graphics_object_tree_iterator_function iterator_function, void *user_data);

Cmiss_scene *Cmiss_scene_get_child_of_position(Cmiss_scene *scene, int position);

int Cmiss_scene_triggers_top_region_change_callback(
	struct Cmiss_scene *scene);

struct Cmiss_scene *CREATE(Cmiss_scene)(struct Cmiss_region *cmiss_region,
	struct Cmiss_graphics_module *graphics_module);

/**
 * this function will remove callbacks relying on external objects.
 * This function is called when:
 * 	- owning region is being destroyed
 * 	- scene is being removed from graphics module
 * 	- context is being destroyed
 */
void Cmiss_scene_detach_from_owner(Cmiss_scene_id cmiss_scene);

#endif /* !defined (SCENE_H) */

