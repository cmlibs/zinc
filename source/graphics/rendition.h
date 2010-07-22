/*******************************************************************************
FILE : rendition.h

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

#if !defined (RENDITION_H)
#define RENDITION_H

#include "api/cmiss_rendition.h"
#include "computed_field/computed_field.h"
#include "general/any_object.h"
#include "general/any_object_prototype.h"
#include "general/object.h"
#include "graphics/graphic.h"
#include "context/context.h"
#include "region/cmiss_region.h"

struct Cmiss_rendition;

typedef int(*Cmiss_rendition_callback)(struct Cmiss_rendition *rendition,
	void *user_data);

DECLARE_CMISS_CALLBACK_TYPES(Cmiss_rendition_transformation, struct Cmiss_rendition *, \
	gtMatrix *, void);

struct Cmiss_rendition *Cmiss_rendition_create_internal(struct Cmiss_region *cmiss_region,
	struct Cmiss_graphics_module *graphics_module);

/***************************************************************************//** 
 * Destroy Cmiss_rendition and clean up the memory it uses.
 *
 * @param  cmiss_rendition_address the address to the pointer of 
 *   the cmiss_rendition_address to be deleted
 * @return  1 if successfully destroy cmiss_rendition, otherwise 0
 */
int DESTROY(Cmiss_rendition)(
	struct Cmiss_rendition **cmiss_rendition_address);

int execute_Cmiss_rendition(struct Cmiss_rendition *rendition);

int Cmiss_rendition_get_position(struct Cmiss_rendition *rendition);

int Cmiss_rendition_set_position(struct Cmiss_rendition *rendition, unsigned int position);
/***************************************************************************//** 
 * Set the position of the rendition
 * @param rendition to be edited
 * @param position Position to be set for the cmiss_rendition
 * @return If successfully set the position, otherwise NULL
 */

/***************************************************************************//** 
 * Get the range of coordinates of visible graphics in the rendition and all its
 * child region renditions.
 * 
 * @param rendition  The rendition to get the range of.
 * @param scene  The scene to filter rendition contents.
 * @param graphics_object_range_void void pointer to graphics_object_range
 * @return If successfully get the range, otherwise NULL
 */
int Cmiss_rendition_get_range(struct Cmiss_rendition *rendition,
	struct Cmiss_scene *scene, struct Graphics_object_range_struct *graphics_object_range);

/***************************************************************************//** 
 * Currently, a Cmiss_region may have at most one cmiss_rendition.
 * @param cmiss_region The region of interest
 * @return If rendition found returns it, otherwise NULL
 */
struct Cmiss_rendition *Cmiss_region_get_rendition_internal(struct Cmiss_region *cmiss_region);

/***************************************************************************//**
 * Call before making several changes to the cmiss_rendition so only a single
 * change message is sent. Call Cmiss_rendition_end_cache at the end of the
 * changes.
 */
int Cmiss_rendition_begin_cache(struct Cmiss_rendition *rendition);

/***************************************************************************//**
 * Call after making changes preceded by a call to Cmiss_rendition_begin_cache to
 * enable a final message to be sent to clients.
 */
int Cmiss_rendition_end_cache(struct Cmiss_rendition *rendition);

/***************************************************************************//** 
 * Enable callback to the rendition from changes on graphics
 * @param rendition The rendition to be edited
 * @return If successfully ensable callback to rendition for region returns 1,
 * otherwise 0
 */
int Cmiss_rendition_set_graphics_managers_callback(struct Cmiss_rendition *rendition);

/***************************************************************************//** 
 * Remove callback to the rendition from changes on graphics
 * @param rendition The rendition to be edited
 * @return If successfully disable callback to rendition for region returns 1,
 * otherwise 0
 */
int Cmiss_rendition_unset_graphics_managers_callback(struct Cmiss_rendition *rendition);

/***************************************************************************//**
 * Deaccess the rendition of the region
 * @param region The region to deaccess rendition from
 * @return If successfully deaccess rendition from region returns 1, otherwise 0
 */
int Cmiss_region_deaccess_rendition(struct Cmiss_region *region);

int Cmiss_rendition_call_renderer(struct Cmiss_rendition *rendition, 
	void *renderer_void);

/***************************************************************************//** 
 * Wrapper for accessing the list of graphic in <Cmiss_rendition>.
 * @param cmiss_rendition target for that rendition
 * @param conditional_function conditional function for the list
 * @param data void pointer to data to pass into the conditional function
 * @return Return the first graphic that fullfill the conditional function
 */
struct Cmiss_graphic *first_graphic_in_Cmiss_rendition_that(
	struct Cmiss_rendition *cmiss_rendition,
	LIST_CONDITIONAL_FUNCTION(Cmiss_graphic) *conditional_function,
	void *data);

int Cmiss_region_modify_rendition(struct Cmiss_region *region,
	struct Scene *scene, struct Cmiss_graphic *graphic,
	int delete_flag, int position);

/***************************************************************************//**
 * Adds a callback routine which is called whenever a Cmiss_rendition is aware of
 * changes.
 */
int Cmiss_rendition_add_callback(struct Cmiss_rendition *rendition, 
	Cmiss_rendition_callback callback, void *user_data);

/***************************************************************************//**
 * Removes a callback which was added previously
 */
int Cmiss_rendition_remove_callback(struct Cmiss_rendition *rendition,
	Cmiss_rendition_callback callback, void *user_data);

/***************************************************************************//**
 * Executes a GFX MODIFY RENDITION GENERAL command.
 * Allows general rendition to be changed (eg. discretization) and
 * updates graphics of settings affected by the changes (probably all).
 */
int gfx_modify_rendition_general(struct Parse_state *state,
	void *cmiss_region_void, void *scene_void);

/***************************************************************************//**
 * Returns the circle discretization of the gt_element_group.
 */
int Cmiss_rendition_get_circle_discretization(
	struct Cmiss_rendition *rendition);

/***************************************************************************//**
 * Sets the circle discretization of the rendition.
 */
int Cmiss_rendition_set_circle_discretization(
	struct Cmiss_rendition *rendition, int circle_discretization);

/***************************************************************************//**
 * Returns the default coordinate field of the <rendition>.
 */
struct Computed_field *Cmiss_rendition_get_default_coordinate_field(
	struct Cmiss_rendition *rendition);

/*******************************************************************************
 * Sets the <default_coordinate_field> used by <rendition>. Settings without
 * a specific coordinate field will use this one.
 */
int Cmiss_rendition_set_default_coordinate_field(
	struct Cmiss_rendition *rendition,
	struct Computed_field *default_coordinate_field);

/***************************************************************************//**
 *Returns the element discretization of the rendition.
 */
int Cmiss_rendition_get_element_discretization(
	struct Cmiss_rendition *rendition,
	struct Element_discretization *element_discretization);

/***************************************************************************//**
 * Sets the element discretization of the rendition.
 */
int Cmiss_rendition_set_element_discretization(
	struct Cmiss_rendition *rendition,
	struct Element_discretization *element_discretization);

/***************************************************************************//**
 * Returns the default coordinate field of the <rendition>.
 */
struct FE_field *Cmiss_rendition_get_native_discretization_field(
	struct Cmiss_rendition *rendition);

/***************************************************************************//**
 * Sets the <native_discretization_field> used by <rendition>. If the field
 * is not NULL and is element-based in a given element, its native discretization
 * is used in preference to the global element_discretization.
 */
int Cmiss_rendition_set_native_discretization_field(
	struct Cmiss_rendition *rendition,
	struct FE_field *native_discretization_field);

/***************************************************************************//**
 * Iterates through every material used by the scene.
 */
int Cmiss_rendition_for_each_material(struct Cmiss_rendition *rendition,	
	MANAGER_ITERATOR_FUNCTION(Graphical_material) *iterator_function,
	void *user_data);

/***************************************************************************//**
 * Lists the general graphics defined for <rendition> - as a
 * set of commands that can be used to reproduce the groups appearance. The
 * <command_prefix> should generally contain "gfx modify g_element" while the
 * optional <command_suffix> may describe the scene (eg. "scene default").
 * Note the command prefix is expected to contain the name of the region.
 */
int Cmiss_rendition_list_commands(struct Cmiss_rendition *rendition,
	 char *command_prefix,char *command_suffix);

/***************************************************************************//**
 * Lists the general graphic defined for <rendition>.
 */
int Cmiss_rendition_list_contents(struct Cmiss_rendition *rendition);

/***************************************************************************//**
 * If any of the graphic in <rendition> use materials with spectrums in the
 * <changed_material_list>, clear their graphics objects and call
 * Cmiss_rendition_changed.
 */
int Cmiss_rendition_Graphical_material_change(struct Cmiss_rendition *rendition,
	void *changed_material_list_void);

int for_each_rendition_in_Cmiss_rendition(
	struct Cmiss_rendition *rendition,
	int (*cmiss_rendition_tree_iterator_function)(struct Cmiss_rendition *rendition,
		void *user_data),	void *user_data);

/***************************************************************************//**
 * Returns the position of <graphic> in <rendition>.
 */
int Cmiss_rendition_get_graphic_position(
	struct Cmiss_rendition *rendition,
	struct Cmiss_graphic *graphic);

/***************************************************************************//**
 * Returns true if <rendition1> and <rendition2> match in
 * main attributes of rendition, graphic etc. such that they would produce
 * the same graphics.
 */
int Cmiss_renditions_match(struct Cmiss_rendition *rendition1,
	struct Cmiss_rendition *rendition2);

/***************************************************************************//**
 * Creates a Cmiss_rendition that is a copy of <existing_rendition> -
 * WITHOUT copying graphics objects, and WITHOUT manager and selection callbacks.
 */
struct Cmiss_rendition *create_editor_copy_Cmiss_rendition(
	struct Cmiss_rendition *existing_rendition);

int for_each_graphic_in_Cmiss_rendition(
	struct Cmiss_rendition *rendition,
	int (*cmiss_rendition_graphic_iterator_function)(struct Cmiss_graphic *graphic,
		void *user_data),	void *user_data);

/***************************************************************************//**
 * Get region of rendition
 */
struct Cmiss_region *Cmiss_rendition_get_region(
	struct Cmiss_rendition *rendition);

/***************************************************************************//**
 *Copies the Cmiss_rendition contents from source to destination, keeping any
 *graphics objects from the destination that will not change with the new graphic
 *from source. Used to apply the changed Cmiss_rendition from the editor to the
 *actual Cmiss_rendition.
 */
int Cmiss_rendition_modify(struct Cmiss_rendition *destination,
	struct Cmiss_rendition *source);

struct GT_element_group *Cmiss_rendition_get_gt_element_group(
	struct Cmiss_rendition *rendition); 

int Cmiss_rendition_set_visibility_flag(
	struct Cmiss_rendition *rendition, int visibility_flag);

int Cmiss_rendition_get_visibility_flag(
	struct Cmiss_rendition *rendition);

int Cmiss_rendition_add_transformation_callback(struct Cmiss_rendition *rendition,
	CMISS_CALLBACK_FUNCTION(Cmiss_rendition_transformation) *function, void *user_data);

int Cmiss_rendition_remove_transformation_callback(
	struct Cmiss_rendition *rendition,
	CMISS_CALLBACK_FUNCTION(Cmiss_rendition_transformation) *function, void *user_data);

int Cmiss_rendition_has_transformation(struct Cmiss_rendition *rendition);

void Cmiss_rendition_remove_time_dependent_transformation(struct Cmiss_rendition *rendition);

int Cmiss_rendition_set_transformation_with_time_callback(struct Cmiss_rendition *rendition,
	struct Computed_field *transformation_field);

int Cmiss_rendition_set_transformation(struct Cmiss_rendition *rendition,
	gtMatrix *transformation);

int Cmiss_rendition_get_transformation(struct Cmiss_rendition *rendition,
	gtMatrix *transformation);

int Cmiss_rendition_add_scene(struct Cmiss_rendition *rendition,
	struct Scene *scene, int hierarchical);

int Cmiss_rendition_remove_scene(struct Cmiss_rendition *rendition, struct Scene *scene);

/***************************************************************************//**
 * This function will deaccess any computed fields being used by the rendition
 * when the rendition itself is not present in any scene.
 *
 * @param cmiss_rendition  pointer to the cmiss_rendition.
 *
 * @return Return 1 if successfully detach fields from rendition otherwise 0.
 */
int Cmiss_rendition_detach_fields(struct Cmiss_rendition *rendition);

PROTOTYPE_OBJECT_FUNCTIONS(Cmiss_rendition);

PROTOTYPE_ANY_OBJECT(Cmiss_rendition);

int Cmiss_rendition_add_glyph(struct Cmiss_rendition *rendition, 
	struct GT_object *glyph, const char *cmiss_graphic_name);

struct Computed_field *Cmiss_rendition_get_selection_group(Cmiss_rendition_id rendition);

int Cmiss_rendition_has_selection_group(Cmiss_rendition_id rendition);

int Cmiss_rendition_remove_selection_group(Cmiss_rendition_id rendition);

int Cmiss_rendition_create_node_list_selection(Cmiss_rendition_id rendition,
	struct LIST(FE_node) *node_list);

/***************************************************************************//**
 * Adds the <graphic> to <rendition> at the given <position>, where 1 is
 * the top of the list (rendered first), and values less than 1 or greater than the
 * last position in the list cause the graphic to be added at its end, with a
 * position one greater than the last.
 *
 * @param rendition  The handle to the rendition.
 * @param graphic  The handle to the cmiss graphic which will be added to the
 *   rendition.
 * @param pos  The position to put the target graphic to.
 * @return  Returns 1 if successfully add graphic to rendition at pos, otherwise
 *   returns 0.
 */
int Cmiss_rendition_add_graphic(Cmiss_rendition_id rendition, Cmiss_graphic_id graphic, int pos);

int Cmiss_rendition_update_callback(struct Cmiss_rendition *rendition, void *dummy_void);

int Cmiss_rendition_notify_parent_rendition_callback(struct Cmiss_rendition *child_rendition,
	void *region_void);
#endif /* !defined (RENDITION_H) */

