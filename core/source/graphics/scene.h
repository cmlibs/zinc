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
#include <string>
#include "opencmiss/zinc/scene.h"
#include "computed_field/computed_field.h"
#include "general/any_object.h"
#include "general/any_object_prototype.h"
#include "general/manager.h"
#include "general/object.h"
#include "graphics/graphics.h"
#include "graphics/graphics_library.h"
#include "context/context.h"
#include "region/cmiss_region.h"
#include "opencmiss/zinc/types/timenotifierid.h"
#include "general/enumerator_private.hpp"

typedef std::list<cmzn_selectionnotifier *> cmzn_selectionnotifier_list;

struct cmzn_scene
/*******************************************************************************
LAST MODIFIED : 16 October 2008

DESCRIPTION :
Structure for maintaining a graphical scene of region.
==============================================================================*/
{
	/* the region being drawn */
	struct cmzn_region *region;
	cmzn_fieldmodulenotifier *fieldmodulenotifier;
	/* settings shared by whole scene */
	/* default coordinate field for graphics drawn with settings below */
	struct Computed_field *default_coordinate_field;
	/* list of objects interested in changes to the cmzn_scene */
	struct cmzn_scene_callback_data *update_callback_list;
	struct LIST(cmzn_graphics) *list_of_graphics;
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
	unsigned int picking_name;
	cmzn_field_group_id selection_group;
	bool selectionChanged;
	cmzn_selectionnotifier_list *selectionnotifier_list;

public:
	void addSelectionnotifier(cmzn_selectionnotifier *selectionnotifier);

	void removeSelectionnotifier(cmzn_selectionnotifier *selectionnotifier);
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
 * @param  scene_address the address of pointer to the scene to destroy.
 * @return  1 if successfully destroy scene, otherwise 0
 */
int DESTROY(cmzn_scene)(struct cmzn_scene **scene_address);

int cmzn_scene_get_picking_name(struct cmzn_scene *scene);

/**
 * Return non-accessed handle to the scene for this region.
 * Currently, a cmzn_region may have at most one scene.
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
 * Wrapper for accessing the list of graphics in <cmzn_scene>.
 * @param scene target for that scene
 * @param conditional_function conditional function for the list
 * @param data void pointer to data to pass into the conditional function
 * @return Return the first graphics that fullfill the conditional function
 */
struct cmzn_graphics *cmzn_scene_get_first_graphics_with_condition(
	struct cmzn_scene *scene,
	LIST_CONDITIONAL_FUNCTION(cmzn_graphics) *conditional_function,
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
	MANAGER_ITERATOR_FUNCTION(cmzn_material) *iterator_function,
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
	struct MANAGER_MESSAGE(cmzn_material) *manager_message);

/***************************************************************************//**
 * Private method for informing scene of spectrum manager changes.
 * Should only be called by cmzn_graphics_module.
 */
void cmzn_scene_spectrum_change(struct cmzn_scene *scene,
	struct MANAGER_MESSAGE(cmzn_spectrum) *manager_message);

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
	int (*cmzn_scene_tree_iterator_function)(struct cmzn_scene *scene,
		void *user_data),	void *user_data);

/***************************************************************************//**
 * Returns the position of <graphics> in <scene>.
 */
int cmzn_scene_get_graphics_position(
	struct cmzn_scene *scene,
	struct cmzn_graphics *graphics);

/***************************************************************************//**
 * Returns true if <scene1> and <scene2> match in
 * main attributes of scene, graphics etc. such that they would produce
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

int for_each_graphics_in_cmzn_scene(struct cmzn_scene *scene,
	int (*cmzn_scene_graphics_iterator_function)(struct cmzn_graphics *graphics,
		void *user_data),	void *user_data);

/***************************************************************************//**
 * Get region of scene. Not accessed.
 */
struct cmzn_region *cmzn_scene_get_region_internal(struct cmzn_scene *scene);

/***************************************************************************//**
 *Copies the cmzn_scene contents from source to destination, keeping any
 *graphics objects from the destination that will not change with the new graphics
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
 * This function will deaccess any computed fields being used by the scene
 * when the scene itself is not present in any scene.
 *
 * @param scene  pointer to the scene.
 *
 * @return Return 1 if successfully detach fields from scene otherwise 0.
 */
int cmzn_scene_detach_fields(struct cmzn_scene *scene);

PROTOTYPE_OBJECT_FUNCTIONS(cmzn_scene);

PROTOTYPE_ANY_OBJECT(cmzn_scene);



/***************************************************************************//**
 * Remove selection groups from scene tree if they are empty.
 */
void cmzn_scene_flush_tree_selections(cmzn_scene_id scene);

int cmzn_scene_create_node_list_selection(cmzn_scene_id scene,
	struct LIST(FE_node) *node_list);

/***************************************************************************//**
 * Set default graphics attributes depending on type, e.g. tessellation,
 * materials, etc.
 */
int cmzn_scene_set_minimum_graphics_defaults(struct cmzn_scene *scene,
	struct cmzn_graphics *graphics);

/***************************************************************************//**
 * Adds the <graphics> to <scene> at the given <position>, where 1 is
 * the top of the list (rendered first), and values less than 1 or greater than the
 * last position in the list cause the graphics to be added at its end, with a
 * position one greater than the last.
 *
 * @param scene  The handle to the scene.
 * @param graphics  The handle to the cmiss graphics which will be added to the
 *   scene.
 * @param pos  The position to put the target graphics to.
 * @return  Returns 1 if successfully add graphics to scene at pos, otherwise
 *   returns 0.
 */
int cmzn_scene_add_graphics(cmzn_scene_id scene, cmzn_graphics_id graphics, int pos);

int list_cmzn_scene_transformation_commands(struct cmzn_scene *scene,
	void *command_prefix_void);

int list_cmzn_scene_transformation(struct cmzn_scene *scene);

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
 * Get graphics at position <pos> in the internal graphics list of scene.
 *
 * @param scene  The handle to the scene of which the graphics is located.
 * @param pos  The position of the graphics, starting from 0.
 * @return  returns the found graphics if found at pos, otherwise returns NULL.
 */
cmzn_graphics_id cmzn_scene_get_graphics_at_position(
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
	struct cmzn_graphics *graphics, int delete_flag, int position);

/**
 * Inform scene that it has changed and must be rebuilt.
 * Unless change caching is on, informs clients of scene that it has changed.
 * For internal use only; called by changed graphics to owning scene.
 */
void cmzn_scene_changed(struct cmzn_scene *scene);

/**
 * Get legacy default_coordinate_field set in cmgui command
 * "gfx modify g_element general".
 * @return  Non-accessed field, or 0 if none.
 */
cmzn_field *cmzn_scene_get_default_coordinate_field(cmzn_scene *scene);

/**
 * Set legacy default_coordinate_field from cmgui command
 * "gfx modify g_element general".
 */
int cmzn_scene_set_default_coordinate_field(cmzn_scene *scene,
	cmzn_field *default_coordinate_field);

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
 * @param scene  pointer to the scene.
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
	cmzn_region *region, const char *graphics_name, cmzn_scenefilter_id filter,
	graphics_object_tree_iterator_function iterator_function, void *user_data);

cmzn_scene *cmzn_scene_get_child_of_picking_name(cmzn_scene *scene, int position);

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
void cmzn_scene_detach_from_owner(cmzn_scene_id scene);

PROTOTYPE_ENUMERATOR_FUNCTIONS(cmzn_streaminformation_scene_io_data_type);

enum cmzn_streaminformation_scene_io_data_type
	cmzn_streaminformation_scene_io_data_type_enum_from_string(const char *string);

char *cmzn_streaminformation_scene_io_data_type_enum_to_string(
	enum cmzn_streaminformation_scene_io_data_type mode);

int Scene_render_threejs(cmzn_scene_id scene,
	cmzn_scenefilter_id scenefilter, const char *filename,
	int number_of_time_steps, double begin_time, double end_time,
	cmzn_streaminformation_scene_io_data_type export_mode,
	int *number_of_entries, std::string **output_string,
	int morphColours, int morphNormals, int morphVertices,
	int numberOfFiles, char **file_names);

int Scene_render_webgl(cmzn_scene_id scene,
	cmzn_scenefilter_id scenefilter, const char *name_prefix);

int Scene_get_number_of_graphics_with_type_in_tree(
	cmzn_scene_id scene, cmzn_scenefilter_id scenefilter, enum cmzn_graphics_type type);

int Scene_get_number_of_graphics_with_surface_vertices_in_tree(cmzn_scene_id scene,
	cmzn_scenefilter_id scenefilter);

/* this include surfaces graphics and line graphics with surfaces (cylinder)*/
int Scene_get_number_of_graphics_with_surface_vertices_in_tree(cmzn_scene_id scene,
	cmzn_scenefilter_id scenefilter);

/* Only glyphs with surfaces are compatible at this moment */
int Scene_get_number_of_web_compatible_glyph_in_tree(cmzn_scene_id scene,
	cmzn_scenefilter_id scenefilter);

#endif /* !defined (SCENE_H) */

