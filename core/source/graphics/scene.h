/*******************************************************************************
FILE : scene.h

==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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
#include "zinc/types/timeid.h"

typedef std::list<cmzn_selection_handler *> Selection_handler_list;

struct cmzn_scene
/*******************************************************************************
LAST MODIFIED : 16 October 2008

DESCRIPTION :
Structure for maintaining a graphical scene of region.
==============================================================================*/
{
	/* the [FE] region being drawn */
	struct cmzn_region *region;
	struct FE_region *fe_region;
	struct FE_region *data_fe_region;
	int fe_region_callback_set, data_fe_region_callback_set;
	/* settings shared by whole scene */
	/* default coordinate field for graphics drawn with settings below */
	struct Computed_field *default_coordinate_field;
	/* list of objects interested in changes to the cmzn_scene */
	struct cmzn_scene_callback_data *update_callback_list;
	/* managers for updating graphics in response to global changes */
	struct MANAGER(Computed_field) *computed_field_manager;
	struct LIST(cmzn_graphic) *list_of_graphics;
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
	struct cmzn_graphics_module *graphics_module;
	cmzn_timenotifier_id time_notifier;
	/* callback list for transformation changes */
	struct LIST(CMZN_CALLBACK_ITEM(cmzn_scene_transformation)) *transformation_callback_list;
	struct LIST(CMZN_CALLBACK_ITEM(cmzn_scene_top_region_change)) *top_region_change_callback_list;
	unsigned int position;
	cmzn_field_group_id selection_group;
	int selection_removed; // flag set when selection_group cleared
	Selection_handler_list *selection_handler_list;
}; /* struct cmzn_scene */

struct MANAGER_MESSAGE(cmzn_tessellation);

typedef int(*cmzn_scene_callback)(struct cmzn_scene *scene,
	void *user_data);

DECLARE_CMZN_CALLBACK_TYPES(cmzn_scene_transformation, struct cmzn_scene *, \
	gtMatrix *, void);

DECLARE_CMZN_CALLBACK_TYPES(cmzn_scene_top_region_change, struct cmzn_scene *, \
	struct cmzn_scene *, void);

struct cmzn_scene *cmzn_scene_create_internal(struct cmzn_region *cmiss_region,
	struct cmzn_graphics_module *graphics_module);

/** @return  Handle to graphics module. Up to caller to destroy */
cmzn_graphics_module * cmzn_scene_get_graphics_module(cmzn_scene_id scene);

/***************************************************************************//**
 * Destroy cmzn_scene and clean up the memory it uses.
 *
 * @param  cmiss_scene_address the address to the pointer of
 *   the cmiss_scene_address to be deleted
 * @return  1 if successfully destroy cmiss_scene, otherwise 0
 */
int DESTROY(cmzn_scene)(
	struct cmzn_scene **cmiss_scene_address);

int cmzn_scene_get_position(struct cmzn_scene *scene);

int cmzn_scene_set_position(struct cmzn_scene *scene, unsigned int position);
/***************************************************************************//**
 * Set the position of the scene
 * @param scene to be edited
 * @param position Position to be set for the cmiss_scene
 * @return If successfully set the position, otherwise NULL
 */

/**
 * Return non-accessed handle to the scene for this region.
 * Currently, a cmzn_region may have at most one cmiss_scene.
 * Private; do not use out of zinc library.
 * @param cmiss_region  The region of query.
 * @return Non-accessed handle to scene for region, or 0 if none.
 */
struct cmzn_scene *cmzn_region_get_scene_private(struct cmzn_region *region);

/***************************************************************************//**
 * Deaccess the scene of the region
 * @param region The region to deaccess scene from
 * @return If successfully deaccess scene from region returns 1, otherwise 0
 */
int cmzn_region_deaccess_scene(struct cmzn_region *region);

/***************************************************************************//**
 * Wrapper for accessing the list of graphic in <cmzn_scene>.
 * @param cmiss_scene target for that scene
 * @param conditional_function conditional function for the list
 * @param data void pointer to data to pass into the conditional function
 * @return Return the first graphic that fullfill the conditional function
 */
struct cmzn_graphic *cmzn_scene_get_first_graphic_with_condition(
	struct cmzn_scene *cmiss_scene,
	LIST_CONDITIONAL_FUNCTION(cmzn_graphic) *conditional_function,
	void *data);

/***************************************************************************//**
 * Adds a callback routine which is called whenever a cmzn_scene is aware of
 * changes.
 */
int cmzn_scene_add_callback(struct cmzn_scene *scene,
	cmzn_scene_callback callback, void *user_data);

/***************************************************************************//**
 * Removes a callback which was added previously
 */
int cmzn_scene_remove_callback(struct cmzn_scene *scene,
	cmzn_scene_callback callback, void *user_data);

/***************************************************************************//**
 * Attempt to guess which field is the most appropriate to use as a coordinate
 * field for graphics.
 * @param scene  The scene whose graphics need a coordinate field.
 * @param domain_type  Type of domain to get coordinate field for. Not used
 * currently.
 * @return non-accessed field
 */
cmzn_field_id cmzn_scene_guess_coordinate_field(
	struct cmzn_scene *scene, cmzn_field_domain_type domain_type);

/***************************************************************************//**
 * Iterates through every material used by the scene.
 */
int cmzn_scene_for_each_material(struct cmzn_scene *scene,
	MANAGER_ITERATOR_FUNCTION(Graphical_material) *iterator_function,
	void *user_data);

/***************************************************************************//**
 * Lists the general graphics defined for <scene> - as a
 * set of commands that can be used to reproduce the groups appearance. The
 * <command_prefix> should generally contain "gfx modify g_element" while the
 * optional <command_suffix> may describe the scene (eg. "scene default").
 * Note the command prefix is expected to contain the name of the region.
 */
int cmzn_scene_list_commands(struct cmzn_scene *scene,
	const char *command_prefix, const char *command_suffix);

/**
 * Private method for informing scene of glyph manager changes.
 * Propagates changes hierarchically to child scenes to minimise messages.
 * Should only be called by cmzn_graphics_module.
 */
void cmzn_scene_glyph_change(struct cmzn_scene *scene,
	struct MANAGER_MESSAGE(cmzn_glyph) *manager_message);

/***************************************************************************//**
 * Private method for informing scene of material manager changes.
 * Should only be called by cmzn_graphics_module.
 */
void cmzn_scene_material_change(struct cmzn_scene *scene,
	struct MANAGER_MESSAGE(Graphical_material) *manager_message);

/***************************************************************************//**
 * Private method for informing scene of spectrum manager changes.
 * Should only be called by cmzn_graphics_module.
 */
void cmzn_scene_spectrum_change(struct cmzn_scene *scene,
	struct MANAGER_MESSAGE(Spectrum) *manager_message);

/***************************************************************************//**
 * Private method for informing scene of tessellation manager changes.
 * Should only be called by cmzn_graphics_module.
 */
void cmzn_scene_tessellation_change(struct cmzn_scene *scene,
	struct MANAGER_MESSAGE(cmzn_tessellation) *manager_message);

/***************************************************************************//**
 * Private method for informing scene of font manager changes.
 * Should only be called by cmzn_graphics_module.
 */
void cmzn_scene_font_change(struct cmzn_scene *scene,
	struct MANAGER_MESSAGE(cmzn_font) *manager_message);

int for_each_child_scene_in_scene_tree(
	struct cmzn_scene *scene,
	int (*cmiss_scene_tree_iterator_function)(struct cmzn_scene *scene,
		void *user_data),	void *user_data);

/***************************************************************************//**
 * Returns the position of <graphic> in <scene>.
 */
int cmzn_scene_get_graphic_position(
	struct cmzn_scene *scene,
	struct cmzn_graphic *graphic);

/***************************************************************************//**
 * Returns true if <scene1> and <scene2> match in
 * main attributes of scene, graphic etc. such that they would produce
 * the same graphics.
 */
int cmzn_scenes_match(struct cmzn_scene *scene1,
	struct cmzn_scene *scene2);

/***************************************************************************//**
 * Creates a cmzn_scene that is a copy of <existing_scene> -
 * WITHOUT copying graphics objects, and WITHOUT manager and selection callbacks.
 */
struct cmzn_scene *create_editor_copy_cmzn_scene(
	struct cmzn_scene *existing_scene);

int for_each_graphic_in_cmzn_scene(
	struct cmzn_scene *scene,
	int (*cmiss_scene_graphic_iterator_function)(struct cmzn_graphic *graphic,
		void *user_data),	void *user_data);

/***************************************************************************//**
 * Get region of scene
 */
struct cmzn_region *cmzn_scene_get_region(
	struct cmzn_scene *scene);

/***************************************************************************//**
 *Copies the cmzn_scene contents from source to destination, keeping any
 *graphics objects from the destination that will not change with the new graphic
 *from source. Used to apply the changed cmzn_scene from the editor to the
 *actual cmzn_scene.
 */
int cmzn_scene_modify(struct cmzn_scene *destination,
	struct cmzn_scene *source);

int cmzn_scene_add_transformation_callback(struct cmzn_scene *scene,
	CMZN_CALLBACK_FUNCTION(cmzn_scene_transformation) *function, void *user_data);

int cmzn_scene_remove_transformation_callback(
	struct cmzn_scene *scene,
	CMZN_CALLBACK_FUNCTION(cmzn_scene_transformation) *function, void *user_data);


int cmzn_scene_has_transformation(struct cmzn_scene *scene);

void cmzn_scene_remove_time_dependent_transformation(struct cmzn_scene *scene);

int cmzn_scene_set_transformation_with_time_callback(struct cmzn_scene *scene,
	struct Computed_field *transformation_field);

int cmzn_scene_set_transformation(struct cmzn_scene *scene,
	gtMatrix *transformation);

int cmzn_scene_get_transformation(struct cmzn_scene *scene,
	gtMatrix *transformation);

/***************************************************************************//**
 * This function will remove field manager and its callback to the scene.
 *
 * @param cmiss_scene  pointer to the cmiss_scene.
 *
 * @return Return 1 if successfully remove field manager and its callback
 *    from scene otherwise 0.
 */
int cmzn_scene_remove_field_manager_and_callback(struct cmzn_scene *scene);

/***************************************************************************//**
 * This function will deaccess any computed fields being used by the scene
 * when the scene itself is not present in any scene.
 *
 * @param cmiss_scene  pointer to the cmiss_scene.
 *
 * @return Return 1 if successfully detach fields from scene otherwise 0.
 */
int cmzn_scene_detach_fields(struct cmzn_scene *scene);

PROTOTYPE_OBJECT_FUNCTIONS(cmzn_scene);

PROTOTYPE_ANY_OBJECT(cmzn_scene);

cmzn_field_group_id cmzn_scene_get_or_create_selection_group(cmzn_scene_id scene);

/***************************************************************************//**
 * Remove selection groups from scene tree if they are empty.
 */
void cmzn_scene_flush_tree_selections(cmzn_scene_id scene);

int cmzn_scene_create_node_list_selection(cmzn_scene_id scene,
	struct LIST(FE_node) *node_list);

/***************************************************************************//**
 * Set default graphic attributes depending on type, e.g. tessellation,
 * materials, etc.
 */
int cmzn_scene_set_minimum_graphic_defaults(struct cmzn_scene *scene,
	struct cmzn_graphic *graphic);

/***************************************************************************//**
 * Set additional default attributes for backward compatibility with gfx modify
 * g_element commands: default coordinate, discretization etc.
 */
int cmzn_scene_set_graphics_defaults_gfx_modify(struct cmzn_scene *scene,
	struct cmzn_graphic *graphic);

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
int cmzn_scene_add_graphic(cmzn_scene_id scene, cmzn_graphic_id graphic, int pos);

int list_cmzn_scene_transformation_commands(struct cmzn_scene *scene,
	void *command_prefix_void);

int list_cmzn_scene_transformation(struct cmzn_scene *scene);

int cmzn_scene_add_selection_from_node_list(cmzn_scene_id scene,
	struct LIST(FE_node) *node_list, int use_data);

int cmzn_scene_remove_selection_from_node_list(cmzn_scene_id scene,
	struct LIST(FE_node) *node_list, int use_data);

int cmzn_scene_add_selection_from_element_list_of_dimension(cmzn_scene_id scene,
	struct LIST(FE_element) *element_list, int dimension);

int cmzn_scene_remove_selection_from_element_list_of_dimension(cmzn_scene_id scene,
	struct LIST(FE_element) *element_list, int dimension);

/***************************************************************************//**
 * Returns the status of the inherited visibility flag of the scene.
 * This function returns 0 if the specified scene or any of its parents along
 * the path has the visibility flag set to 0 otherwise return 1;
 *
 * @param scene  The handle to the scene.
 * @return  1 if scene and all its parent are visible, otherwise 0.
 */
int cmzn_scene_is_visible_hierarchical(cmzn_scene_id scene);

/***************************************************************************//**
 * Get graphic at position <pos> in the internal graphics list of scene.
 *
 * @param scene  The handle to the scene of which the graphic is located.
 * @param pos  The position of the graphic, starting from 0.
 * @return  returns the found graphic if found at pos, otherwise returns NULL.
 */
cmzn_graphic_id cmzn_scene_get_graphic_at_position(
	cmzn_scene_id scene, int pos);

/***************************************************************************//**
 * Check either region has scene or not.
 *
 * @param region  The handle to cmiss_region.
 * @return  Returns 1 if scene is found in region, otherwise 0.
 */
int cmzn_region_has_scene(cmzn_region_id cmiss_region);

cmzn_field_id cmzn_scene_get_selection_group_private_for_highlighting(
	cmzn_scene_id scene);

int cmzn_region_modify_scene(struct cmzn_region *region,
	struct cmzn_graphic *graphic, int delete_flag, int position);


/**
 * Inform scene that it has changed and must be rebuilt.
 * Unless change caching is on, informs clients of scene that it has changed.
 * For internal use only; called by changed graphic to owning scene.
 */
void cmzn_scene_changed(struct cmzn_scene *scene);

int cmzn_scene_set_default_coordinate_field(
	struct cmzn_scene *scene,
	struct Computed_field *default_coordinate_field);

////  new APIs
int cmzn_scene_add_total_transformation_callback(struct cmzn_scene *child_scene,
	cmzn_scene_id scene, CMZN_CALLBACK_FUNCTION(cmzn_scene_transformation) *function,
	CMZN_CALLBACK_FUNCTION(cmzn_scene_top_region_change) *region_change_function,
	void *user_data);

int cmzn_scene_remove_total_transformation_callback(struct cmzn_scene *child_scene,
	cmzn_scene_id scene, CMZN_CALLBACK_FUNCTION(cmzn_scene_transformation) *function,
	CMZN_CALLBACK_FUNCTION(cmzn_scene_top_region_change) *region_change_function,
	void *user_data);

int cmzn_scene_notify_scene_viewer_callback(struct cmzn_scene *scene,
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
gtMatrix *cmzn_scene_get_total_transformation(
	struct cmzn_scene *scene, struct cmzn_scene *top_scene);

int cmzn_scene_get_global_graphics_range(cmzn_scene_id top_scene,
	cmzn_scenefilter_id filter,
	double *centre_x, double *centre_y, double *centre_z,
	double *size_x, double *size_y, double *size_z);

typedef int(*graphics_object_tree_iterator_function)(
	struct GT_object *graphics_object, double time, void *user_data);

int for_each_graphics_object_in_scene_tree(cmzn_scene_id scene,
	cmzn_scenefilter_id filter, graphics_object_tree_iterator_function iterator_function,
	void *user_data);

int Scene_export_region_graphics_object(cmzn_scene *scene,
	cmzn_region *region, const char *graphic_name, cmzn_scenefilter_id filter,
	graphics_object_tree_iterator_function iterator_function, void *user_data);

cmzn_scene *cmzn_scene_get_child_of_position(cmzn_scene *scene, int position);

int cmzn_scene_triggers_top_region_change_callback(
	struct cmzn_scene *scene);

struct cmzn_scene *CREATE(cmzn_scene)(struct cmzn_region *cmiss_region,
	struct cmzn_graphics_module *graphics_module);

/**
 * this function will remove callbacks relying on external objects.
 * This function is called when:
 * 	- owning region is being destroyed
 * 	- scene is being removed from graphics module
 * 	- context is being destroyed
 */
void cmzn_scene_detach_from_owner(cmzn_scene_id cmiss_scene);

#endif /* !defined (SCENE_H) */

