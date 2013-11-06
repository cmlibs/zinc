/*******************************************************************************
FILE : scene.cpp

==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <algorithm>
#include <list>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "zinc/core.h"
#include "zinc/glyph.h"
#include "zinc/graphics.h"
#include "zinc/material.h"
#include "zinc/node.h"
#include "zinc/scenepicker.h"
#include "zinc/scene.h"
#include "zinc/status.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_finite_element.h"
#include "computed_field/computed_field_group.h"
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_wrappers.h"
#include "zinc/fieldsubobjectgroup.h"
#include "region/cmiss_region.h"
#include "finite_element/finite_element_region.h"
#include "graphics/graphics.h"
#include "graphics/graphics_module.h"
#include "graphics/scene_viewer.h"
#include "graphics/scene.h"
#include "general/any_object_private.h"
#include "general/any_object_definition.h"
#include "general/callback_private.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "region/cmiss_region_private.h"
#include "general/object.h"
#include "graphics/graphics_library.h"
#include "graphics/material.h"
#include "graphics/render_to_finite_elements.h"
#include "graphics/scene_picker.hpp"
#include "graphics/spectrum.h"
#include "time/time.h"
#include "time/time_keeper.hpp"
#include "general/message.h"
#include "computed_field/computed_field_private.hpp"
#include "computed_field/computed_field_group_base.hpp"
#include "graphics/selection.hpp"
#include "graphics/scene.hpp"
#include "graphics/render_gl.h"
#include "graphics/tessellation.hpp"

FULL_DECLARE_CMZN_CALLBACK_TYPES(cmzn_scene_transformation, \
	struct cmzn_scene *, gtMatrix *);

FULL_DECLARE_CMZN_CALLBACK_TYPES(cmzn_scene_top_region_change, \
	struct cmzn_scene *, struct cmzn_scene *);

struct cmzn_scene_callback_data
{
	cmzn_scene_callback callback;
	void *callback_user_data;
	cmzn_scene_callback_data *next;
}; /* struct cmzn_scene_callback_data */

static int UNIQUE_SCENE_NAME = 1000;
int GET_UNIQUE_SCENE_NAME()
{
	return UNIQUE_SCENE_NAME++;
}

static void cmzn_scene_region_change(struct cmzn_region *region,
	struct cmzn_region_changes *region_changes, void *scene_void);

DEFINE_CMZN_CALLBACK_MODULE_FUNCTIONS(cmzn_scene_transformation, void)

DEFINE_CMZN_CALLBACK_FUNCTIONS(cmzn_scene_transformation, \
	struct cmzn_scene *, gtMatrix *)

DEFINE_CMZN_CALLBACK_MODULE_FUNCTIONS(cmzn_scene_top_region_change, void)

DEFINE_CMZN_CALLBACK_FUNCTIONS(cmzn_scene_top_region_change, \
	struct cmzn_scene *, struct cmzn_scene *);

static int cmzn_scene_update_time_behaviour(struct cmzn_scene *scene);

/**
 * Informs registered clients of change in the scene.
 */
static int cmzn_scene_inform_clients(
	struct cmzn_scene *scene)
{
	int return_code = 1;
	if (scene)
	{
		// update_time_behaviour should be checked for efficiency:
		cmzn_scene_update_time_behaviour(scene);
		cmzn_region *parentRegion = cmzn_region_get_parent_internal(scene->region);
		cmzn_scene *parentScene = cmzn_region_get_scene_private(parentRegion);
		if (parentScene)
		{
			cmzn_scene_changed(parentScene);
		}
		cmzn_scene_callback_data *callback_data = scene->update_callback_list;
		while (callback_data)
		{
			(callback_data->callback)(scene,
					callback_data->callback_user_data);
				callback_data = callback_data->next;
		}
		scene->changed = 0;
	}
	else
	{
		return_code = 0;
	}
	return (return_code);
}

void cmzn_scene_changed(struct cmzn_scene *scene)
{
	if (scene)
	{
		scene->changed = 1;
		if (0 == scene->cache)
		{
			cmzn_scene_inform_clients(scene);
		}
	}
}

int cmzn_scene_begin_change(cmzn_scene_id scene)
{
	if (scene)
	{
		/* increment cache to allow nesting */
		(scene->cache)++;
		return 1;
	}
	else
	{
		return 0;
	}
}

int cmzn_scene_end_change(cmzn_scene_id scene)
{
	if (scene)
	{
		/* decrement cache to allow nesting */
		(scene->cache)--;
		/* once cache has run out, inform clients of any changes */
		if (0 == scene->cache)
		{
			if (scene->changed)
			{
				cmzn_scene_inform_clients(scene);
			}
		}
		return 1;
	}
	else
	{
		return 0;
	}
}

/***************************************************************************//**
 * ANY_OBJECT cleanup function for cmzn_scene attached to cmzn_region.
 * Called when region is destroyed.
 */
static int cmzn_scene_void_detach_from_cmzn_region(void *scene_void)
{
	int return_code;
	struct cmzn_scene *scene;

	ENTER(cmzn_scene_void_detach_from_cmzn_region);
	if ((scene = (struct cmzn_scene *)scene_void))
	{
		// graphics_module pointer is cleared if graphics_module is being destroyed
		if (scene->graphics_module)
		{
			cmzn_graphics_module_remove_member_region(scene->graphics_module,
				scene->region);
		}
		scene->region = (struct cmzn_region *)NULL;
		cmzn_scene_detach_from_owner(scene);
		return_code = DEACCESS(cmzn_scene)(&scene);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_rendtion_void_detach_from_cmzn_region.  Missing void cmzn_scene");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

/***************************************************************************//**
 * Allocates memory and assigns fields for a cmiss scene for the given
 * cmiss_region. Access count is set to 1.
 */
struct cmzn_scene *CREATE(cmzn_scene)(struct cmzn_region *cmiss_region,
	struct cmzn_graphics_module *graphics_module)
{
	struct FE_region *fe_region, *data_fe_region;
	struct cmzn_scene *scene;

	ENTER(CREATE(cmzn_scene));
	data_fe_region = NULL;
	if (cmiss_region && (fe_region = cmzn_region_get_FE_region(cmiss_region)))
	{
		data_fe_region = FE_region_get_data_FE_region(fe_region);
		if (ALLOCATE(scene, struct cmzn_scene, 1))
		{
			scene->list_of_graphics = NULL;
			if (NULL != (scene->list_of_graphics =
					CREATE(LIST(cmzn_graphics))()))
			{
				scene->region = cmiss_region;
				scene->fe_region = ACCESS(FE_region)(fe_region);
				scene->data_fe_region = ACCESS(FE_region)(data_fe_region);
				scene->fe_region_callback_set = 0;
				scene->data_fe_region_callback_set = 0;

				/* legacy general settings used as defaults for new graphics */
				scene->element_divisions = NULL;
				scene->element_divisions_size = 0;
				scene->circle_discretization = 0;
				scene->default_coordinate_field = (struct Computed_field *)NULL;
				scene->visibility_flag = true;
				scene->update_callback_list=
					(struct cmzn_scene_callback_data *)NULL;
				/* managers and callback ids */
				scene->computed_field_manager=cmzn_region_get_Computed_field_manager(
					 cmiss_region);
				scene->computed_field_manager_callback_id=(void *)NULL;
				scene->transformation = (gtMatrix *)NULL;
				scene->graphics_module =	graphics_module;
				scene->time_notifier = NULL;
				scene->cache = 0;
				scene->changed = 0;
				scene->position = 0;
				scene->transformation_callback_list =
					CREATE(LIST(CMZN_CALLBACK_ITEM(cmzn_scene_transformation)))();
				scene->top_region_change_callback_list =
					CREATE(LIST(CMZN_CALLBACK_ITEM(cmzn_scene_top_region_change)))();
				scene->transformation_field = NULL;
				scene->transformation_time_callback_flag = 0;
				scene->selection_group = NULL;
				scene->selectionnotifier_list = NULL;
			}
			else
			{
				DESTROY(LIST(cmzn_graphics))(
					&(scene->list_of_graphics));
				DEALLOCATE(scene);
				scene = (struct cmzn_scene *)NULL;
			}
			scene->access_count = 1;
		}
		else
		{
			DEALLOCATE(scene);
			scene = (struct cmzn_scene *)NULL;
		}
		if (!scene)
		{
			display_message(ERROR_MESSAGE,
				"CREATE(cmzn_scene).  Insufficient memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(cmzn_scene).  Invalid argument(s)");
		scene=(struct cmzn_scene *)NULL;
	}
	LEAVE;

	return (scene);
} /* CREATE(cmzn_scene) */

cmzn_field_id cmzn_scene_guess_coordinate_field(
	struct cmzn_scene *scene, cmzn_field_domain_type domain_type)
{
	cmzn_field_id coordinate_field = 0;
	// could be smarter here:
	USE_PARAMETER(domain_type);
	/* if we don't have a computed_field_manager, we are working on an
	   "editor copy" which does not update graphics; the
	   default_coordinate_field will have been supplied by the global object */
	if (scene && scene->computed_field_manager)
	{
		coordinate_field = FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
			Computed_field_is_coordinate_field, (void *)NULL, scene->computed_field_manager);
	}
	return coordinate_field;
}

/***************************************************************************//**
 * Get legacy default_coordinate_field set in gfx modify g_element general
 * command, if any.
 * @return non-accessed field
 */
struct Computed_field *cmzn_scene_get_default_coordinate_field(
	struct cmzn_scene *scene)
{
	if (scene)
		return scene->default_coordinate_field;
	return 0;
}

/***************************************************************************//**
 * Set legacy default_coordinate_field in gfx modify g_element general command.
 */
int cmzn_scene_set_default_coordinate_field(
	struct cmzn_scene *scene,
	struct Computed_field *default_coordinate_field)
{
	int return_code = 1;
	if (scene && (!default_coordinate_field ||
		Computed_field_has_up_to_3_numerical_components(default_coordinate_field, (void *)0)))
	{
		REACCESS(Computed_field)(&(scene->default_coordinate_field),
			default_coordinate_field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_scene_set_default_coordinate_field.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

static void cmzn_scene_FE_region_change(struct FE_region *fe_region,
	struct FE_region_changes *changes, void *scene_void)
{
	struct cmzn_scene *scene;
	struct cmzn_graphics_FE_region_change_data data;

	ENTER(cmzn_scene_FE_region_change);
	if (fe_region && changes &&
		(scene = (struct cmzn_scene *)scene_void))
	{
		CHANGE_LOG_GET_CHANGE_SUMMARY(FE_field)(changes->fe_field_changes,
			&data.fe_field_change_summary);
		data.fe_field_changes = changes->fe_field_changes;
		CHANGE_LOG_GET_CHANGE_SUMMARY(FE_node)(changes->fe_node_changes,
			&data.fe_node_change_summary);
		CHANGE_LOG_GET_NUMBER_OF_CHANGES(FE_node)(changes->fe_node_changes,
			&data.number_of_fe_node_changes);
		data.fe_node_changes = changes->fe_node_changes;
		for (int dim = 0; dim < MAXIMUM_ELEMENT_XI_DIMENSIONS; ++dim)
		{
			CHANGE_LOG_GET_CHANGE_SUMMARY(FE_element)(changes->fe_element_changes[dim],
				&(data.fe_element_change_summary[dim]));
			CHANGE_LOG_GET_NUMBER_OF_CHANGES(FE_element)(changes->fe_element_changes[dim],
				&(data.number_of_fe_element_changes[dim]));
			data.fe_element_changes[dim] = changes->fe_element_changes[dim];
		}
		/*???RC Is there a better way of getting time to here? */
		data.time = 0;
		data.fe_region = fe_region;
		cmzn_scene_begin_change(scene);
		FOR_EACH_OBJECT_IN_LIST(cmzn_graphics)(
			cmzn_graphics_FE_region_change, (void *)&data,
			scene->list_of_graphics);
		cmzn_scene_end_change(scene);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_scene_FE_region_change.  Invalid argument(s)");
	}
	LEAVE;
} /* cmzn_scene_FE_region_change */
static void cmzn_scene_data_FE_region_change(struct FE_region *fe_region,
	struct FE_region_changes *changes, void *scene_void)
{
	struct cmzn_scene *scene;
	struct cmzn_graphics_FE_region_change_data data;

	ENTER(cmzn_scene_data_FE_region_change);
	if (fe_region && changes &&
		(scene = (struct cmzn_scene *)scene_void))
	{
		CHANGE_LOG_GET_CHANGE_SUMMARY(FE_field)(changes->fe_field_changes,
			&data.fe_field_change_summary);
		data.fe_field_changes = changes->fe_field_changes;
		CHANGE_LOG_GET_CHANGE_SUMMARY(FE_node)(changes->fe_node_changes,
			&data.fe_node_change_summary);
		CHANGE_LOG_GET_NUMBER_OF_CHANGES(FE_node)(changes->fe_node_changes,
			&data.number_of_fe_node_changes);
		data.fe_node_changes = changes->fe_node_changes;
		for (int dim = 0; dim < MAXIMUM_ELEMENT_XI_DIMENSIONS; ++dim)
		{
			CHANGE_LOG_GET_CHANGE_SUMMARY(FE_element)(changes->fe_element_changes[dim],
				&(data.fe_element_change_summary[dim]));
			CHANGE_LOG_GET_NUMBER_OF_CHANGES(FE_element)(changes->fe_element_changes[dim],
				&(data.number_of_fe_element_changes[dim]));
			data.fe_element_changes[dim] = changes->fe_element_changes[dim];
		}
		/*???RC Is there a better way of getting time to here? */
		if (scene->time_notifier)
		{
			data.time = cmzn_timenotifier_get_time(scene->time_notifier);
		}
		else
		{
			data.time = 0;
		}

		data.fe_region = fe_region;
		cmzn_scene_begin_change(scene);
		FOR_EACH_OBJECT_IN_LIST(cmzn_graphics)(
			cmzn_graphics_data_FE_region_change, (void *)&data,
			scene->list_of_graphics);
		cmzn_scene_end_change(scene);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_scene_data_FE_region_change.  Invalid argument(s)");
	}
	LEAVE;
} /* cmzn_scene_data_FE_region_change */

static void cmzn_scene_Computed_field_change(
	struct MANAGER_MESSAGE(Computed_field) *message,void *scene_void)
{
	cmzn_scene *scene = reinterpret_cast<cmzn_scene*>(scene_void);
	if (message && scene)
	{
		struct cmzn_graphics_Computed_field_change_data change_data;
		bool local_selection_changed = false;
		change_data.changed_field_list = MANAGER_MESSAGE_GET_CHANGE_LIST(Computed_field)(
			message, MANAGER_CHANGE_RESULT(Computed_field));
		if (scene->selection_group)
		{
			const cmzn_field_change_detail *base_change_detail = 0;
			int change_flags = Computed_field_manager_message_get_object_change_and_detail(
				message, cmzn_field_group_base_cast(scene->selection_group), &base_change_detail);
			if (change_flags & (
				(MANAGER_CHANGE_RESULT(Computed_field) | MANAGER_CHANGE_ADD(Computed_field))))
			{
				if (base_change_detail)
				{
					const cmzn_field_hierarchical_group_change_detail *group_change_detail =
						dynamic_cast<const cmzn_field_hierarchical_group_change_detail *>(base_change_detail);
					int group_local_change = group_change_detail->getLocalChangeSummary();
					if (group_local_change != CMZN_FIELD_GROUP_CHANGE_NONE)
						local_selection_changed = true;
					int group_nonlocal_change = group_change_detail->getNonlocalChangeSummary();
					if (scene->selectionnotifier_list && (0 < scene->selectionnotifier_list->size()))
					{
						cmzn_selectionevent_id event = new cmzn_selectionevent();
						event->changeFlags = cmzn_field_group_change_type_to_selectionevent_change_type(group_local_change)
							| cmzn_field_group_change_type_to_selectionevent_change_type(group_nonlocal_change);
						for (cmzn_selectionnotifier_list::iterator iter = scene->selectionnotifier_list->begin();
							iter != scene->selectionnotifier_list->end(); ++iter)
						{
							(*iter)->notify(event);
						}
						cmzn_selectionevent_destroy(&event);
					}
				}
				// ensure child scene selection_group matches the appropriate subgroup or none if none
				cmzn_region_id child_region = cmzn_region_get_first_child(scene->region);
				while ((NULL != child_region))
				{
					cmzn_scene_id child_scene = cmzn_region_get_scene_private(child_region);
					if (child_scene)
					{
						cmzn_field_group_id child_group =
							cmzn_field_group_get_subregion_group(scene->selection_group, child_region);
						cmzn_scene_set_selection_group(child_scene, child_group);
						if (child_group)
						{
							cmzn_field_group_destroy(&child_group);
						}
					}
					cmzn_region_reaccess_next_sibling(&child_region);
				}
			}
		}
		if (change_data.changed_field_list || local_selection_changed)
		{
			change_data.selection_changed = local_selection_changed;
			cmzn_scene_begin_change(scene);
			FOR_EACH_OBJECT_IN_LIST(cmzn_graphics)(cmzn_graphics_Computed_field_change,
				(void *)&change_data, scene->list_of_graphics);
			cmzn_scene_end_change(scene);
			if (change_data.changed_field_list)
				DESTROY(LIST(Computed_field))(&change_data.changed_field_list);
		}
	}
}

int cmzn_region_attach_scene(struct cmzn_region *region,
	struct cmzn_scene *scene)
{
	int return_code;
	struct Any_object *any_object;

	ENTER(cmzn_region_attach_scene);

	if (NULL != (any_object = CREATE(ANY_OBJECT(cmzn_scene))(scene)) &&
		cmzn_region_private_attach_any_object(region, any_object))
	{
		cmzn_region_add_callback(scene->region,
			cmzn_scene_region_change, (void *)scene);
		scene->fe_region_callback_set =
			FE_region_add_callback(scene->fe_region,
				cmzn_scene_FE_region_change, (void *)scene);
		if (scene->data_fe_region)
		{
			scene->data_fe_region_callback_set =
				FE_region_add_callback(scene->data_fe_region,
					cmzn_scene_data_FE_region_change, (void *)scene);
		}
		/* request callbacks from any managers supplied */
		if (scene->computed_field_manager)
		{
			scene->computed_field_manager_callback_id=
				MANAGER_REGISTER(Computed_field)(
					cmzn_scene_Computed_field_change,(void *)scene,
					scene->computed_field_manager);
		}
		Any_object_set_cleanup_function(any_object,
			cmzn_scene_void_detach_from_cmzn_region);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_region_attach_scene. Could not attach object.");
		DESTROY(Any_object)(&any_object);
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

struct cmzn_scene *cmzn_scene_create_internal(struct cmzn_region *cmiss_region,
	struct cmzn_graphics_module *graphics_module)
{
	struct cmzn_scene *scene;

	ENTER(cmzn_scene_create);
	if (cmiss_region && graphics_module)
	{
		scene = CREATE(cmzn_scene)(cmiss_region, graphics_module);
		{
			if (!(scene && cmzn_region_attach_scene(cmiss_region,
						scene)))
			{
				DEACCESS(cmzn_scene)(&scene);
			}
			else
			{
				cmzn_graphics_module_add_member_region(graphics_module, cmiss_region);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(cmzn_scene).  Invalid argument(s)");
		scene = NULL;
	}
	LEAVE;

	return (scene);
}

void cmzn_scene_add_child_region(struct cmzn_scene *scene,
	struct cmzn_region *child_region)
{
	struct cmzn_scene *child_scene;
	if (scene && child_region &&
		(NULL != (child_scene = cmzn_scene_create_internal(
			child_region, scene->graphics_module))))
	{
		cmzn_scene_set_position(child_scene,	GET_UNIQUE_SCENE_NAME());
		struct cmzn_region *temp_region = cmzn_region_get_first_child(
			child_region);
		while (temp_region)
		{
			if (!cmzn_region_has_scene(temp_region))
			{
				cmzn_scene_add_child_region(child_scene,
					temp_region);
			}
			cmzn_region_reaccess_next_sibling(&temp_region);
		}
	}
}

int cmzn_scene_update_child_scene(struct cmzn_scene *scene)
{
	int return_code;

	ENTER(cmzn_scene_update_child_scene);
	if (scene)
	{
		cmzn_scene_begin_change(scene);
		/* ensure we have a graphical element for each child region */
		struct cmzn_region *child_region = cmzn_region_get_first_child(scene->region);
		while (child_region)
		{
			if (!cmzn_region_has_scene(child_region))
			{
				cmzn_scene_add_child_region(scene,
					child_region);
			}
			cmzn_region_reaccess_next_sibling(&child_region);
		}
		cmzn_scene_end_change(scene);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_scene_update_child_scene.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

static void cmzn_scene_region_change(struct cmzn_region *region,
	struct cmzn_region_changes *region_changes, void *scene_void)
{
	struct cmzn_region *child_region;
	struct cmzn_scene *scene;

	ENTER(Scene_cmzn_region_change);

	if (region && region_changes && (scene = (struct cmzn_scene *)scene_void))
	{
		if (region_changes->children_changed)
		{
			cmzn_scene_begin_change(scene);
			if (region_changes->child_added)
			{
				child_region = region_changes->child_added;
				cmzn_scene_add_child_region(scene, child_region);
				cmzn_scene_changed(scene);
			}
			else if (region_changes->child_removed)
			{
				/* flag it as changed to trigger callback on scene */
				cmzn_scene_changed(scene);
			}
			else
			{
				cmzn_scene_update_child_scene(scene);
			}
			cmzn_scene_end_change(scene);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_cmzn_region_change.  Invalid argument(s)");
	}
	LEAVE;
} /* cmzn_scene_region_change */

DECLARE_OBJECT_FUNCTIONS(cmzn_scene);
DEFINE_ANY_OBJECT(cmzn_scene);

int cmzn_scene_set_minimum_graphics_defaults(struct cmzn_scene *scene,
	struct cmzn_graphics *graphics)
{
	int return_code = 1;
	if (scene && graphics)
	{
		cmzn_graphics_type graphics_type = cmzn_graphics_get_graphics_type(graphics);

		cmzn_tessellationmodule_id tessellationModule =
			cmzn_graphics_module_get_tessellationmodule(scene->graphics_module);
		cmzn_tessellation *tessellation =
			((graphics_type == CMZN_GRAPHICS_TYPE_POINTS) || (graphics_type == CMZN_GRAPHICS_TYPE_STREAMLINES)) ?
			cmzn_tessellationmodule_get_default_points_tessellation(tessellationModule) :
			cmzn_tessellationmodule_get_default_tessellation(tessellationModule);
		cmzn_graphics_set_tessellation(graphics, tessellation);
		cmzn_tessellation_destroy(&tessellation);
		cmzn_tessellationmodule_destroy(&tessellationModule);

		cmzn_graphicspointattributes_id point_attributes = cmzn_graphics_get_graphicspointattributes(graphics);
		if (point_attributes)
		{
			cmzn_font *font = cmzn_graphics_module_get_default_font(scene->graphics_module);
			cmzn_graphicspointattributes_set_font(point_attributes, font);
			cmzn_font_destroy(&font);
			cmzn_glyph_id glyph = cmzn_graphicspointattributes_get_glyph(point_attributes);
			if (!glyph)
			{
				cmzn_glyphmodule_id glyphmodule = cmzn_graphics_module_get_glyphmodule(scene->graphics_module);
				glyph = cmzn_glyphmodule_get_default_point_glyph(glyphmodule);
				cmzn_glyphmodule_destroy(&glyphmodule);
				cmzn_graphicspointattributes_set_glyph(point_attributes, glyph);
			}
			cmzn_glyph_destroy(&glyph);
			cmzn_graphicspointattributes_destroy(&point_attributes);
		}

		struct cmzn_materialmodule *materialmodule = cmzn_graphics_module_get_materialmodule(scene->graphics_module);
		cmzn_material *default_material =
			cmzn_materialmodule_get_default_material(materialmodule);
		cmzn_graphics_set_material(graphics, default_material);
		cmzn_material_destroy(&default_material);
		cmzn_material *default_selected =
			cmzn_materialmodule_get_default_selected_material(materialmodule);
		cmzn_graphics_set_selected_material(graphics, default_selected);
		cmzn_material_destroy(&default_selected);
		cmzn_materialmodule_destroy(&materialmodule);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_scene_set_minimum_graphics_defaults.  Invalid argument(s)");
		return_code = 0;
	}
	return return_code;
}

/**
 * Apply legacy default coordinate field, element, circle and native discretization.
 * Assumes cmzn_scene_set_minimum_graphics_defaults has been called first
 */
int cmzn_scene_set_graphics_defaults_gfx_modify(struct cmzn_scene *scene,
	struct cmzn_graphics *graphics)
{
	int return_code = 1;
	if (scene && graphics)
	{
		cmzn_graphics_type graphics_type = cmzn_graphics_get_graphics_type(graphics);
		cmzn_field_domain_type domain_type = cmzn_graphics_get_field_domain_type(graphics);

		if ((graphics_type != CMZN_GRAPHICS_TYPE_POINTS) || (domain_type != CMZN_FIELD_DOMAIN_TYPE_POINT))
		{
			cmzn_field_id coordinate_field = cmzn_scene_get_default_coordinate_field(scene);
			if (!coordinate_field)
				coordinate_field = cmzn_scene_guess_coordinate_field(scene, domain_type);
			if (coordinate_field)
				cmzn_graphics_set_coordinate_field(graphics, coordinate_field);
		}

		bool use_element_discretization = (0 != scene->element_divisions) &&
			(graphics_type != CMZN_GRAPHICS_TYPE_POINTS) && (graphics_type != CMZN_GRAPHICS_TYPE_STREAMLINES);
		bool use_circle_discretization = (scene->circle_discretization >= 3) &&
			(graphics_type == CMZN_GRAPHICS_TYPE_LINES);
		if (use_circle_discretization)
		{
			cmzn_graphicslineattributes_id lineAttr = cmzn_graphics_get_graphicslineattributes(graphics);
			use_circle_discretization = (cmzn_graphicslineattributes_get_shape_type(lineAttr) ==
				CMZN_GRAPHICSLINEATTRIBUTES_SHAPE_TYPE_CIRCLE_EXTRUSION);
			cmzn_graphicslineattributes_destroy(&lineAttr);
		}
		if (use_element_discretization || use_circle_discretization)
		{
			cmzn_tessellationmodule_id tessellationModule =
				cmzn_graphics_module_get_tessellationmodule(scene->graphics_module);
			cmzn_tessellation_id currentTessellation = cmzn_graphics_get_tessellation(graphics);
			cmzn_tessellation_id tessellation =
				cmzn_tessellationmodule_find_or_create_fixed_tessellation(tessellationModule,
					use_element_discretization ? scene->element_divisions_size : 0,
					use_element_discretization ? scene->element_divisions : 0,
					use_circle_discretization ? scene->circle_discretization : 0,
					currentTessellation);
			cmzn_graphics_set_tessellation(graphics, tessellation);
			cmzn_tessellation_destroy(&tessellation);
			cmzn_tessellation_destroy(&currentTessellation);
			cmzn_tessellationmodule_destroy(&tessellationModule);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_scene_set_graphics_defaults_gfx_modify.  Invalid argument(s)");
		return_code = 0;
	}
	return return_code;
}

int cmzn_scene_add_graphics(struct cmzn_scene *scene,
	struct cmzn_graphics *graphics,int position)
{
	int return_code;
	if (scene && graphics && (NULL == cmzn_graphics_get_scene_private(graphics)))
	{
		return_code=cmzn_graphics_add_to_list(graphics,position,
			scene->list_of_graphics);
		cmzn_graphics_set_scene_private(graphics, scene);
		cmzn_scene_changed(scene);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_scene_add_graphics.  Invalid argument(s)");
		return_code=0;
	}
	return (return_code);
}

int cmzn_scene_remove_graphics(struct cmzn_scene *scene,
	struct cmzn_graphics *graphics)
{
	if (scene && graphics && (scene == cmzn_graphics_get_scene_private(graphics)))
	{
		cmzn_graphics_set_scene_private(graphics, NULL);
		cmzn_graphics_remove_from_list(graphics, scene->list_of_graphics);
		cmzn_scene_changed(scene);
		return CMZN_OK;
	}
	return CMZN_ERROR_ARGUMENT;
}

/***************************************************************************//**
 * Changes the contents of <graphics> to match <new_graphics>, with no change in
 * position in <scene>.
 */
int cmzn_scene_modify_graphics(struct cmzn_scene *scene,
	struct cmzn_graphics *graphics,struct cmzn_graphics *new_graphics)
{
	int return_code;
	if (scene&&graphics&&new_graphics)
	{
		return_code=cmzn_graphics_modify_in_list(graphics,new_graphics,
			scene->list_of_graphics);
		cmzn_scene_changed(scene);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_scene_modify_graphics.  Invalid argument(s)");
		return_code=0;
	}
	return (return_code);
}

static int cmzn_scene_build_graphics_objects(
	struct cmzn_scene *scene, cmzn_scenefilter *scenefilter,
	FE_value time, const char *name_prefix)
{
	int return_code = 1;
	struct cmzn_graphics_to_graphics_object_data graphics_to_object_data;

	ENTER(cmzn_scene_build_graphics_objects);
	if (scene)
	{
		if ((cmzn_scene_get_number_of_graphics(scene) > 0))
		{
			// use begin/end cache to avoid field manager messages being sent when
			// field wrappers are created and destroyed
			MANAGER_BEGIN_CACHE(Computed_field)(scene->computed_field_manager);
			graphics_to_object_data.name_prefix = name_prefix;
			graphics_to_object_data.rc_coordinate_field = (struct Computed_field *) NULL;
			graphics_to_object_data.wrapper_orientation_scale_field
				= (struct Computed_field *) NULL;
			graphics_to_object_data.wrapper_stream_vector_field = (struct Computed_field *) NULL;
			graphics_to_object_data.region = scene->region;
			graphics_to_object_data.field_module = cmzn_region_get_fieldmodule(scene->region);
			graphics_to_object_data.field_cache = cmzn_fieldmodule_create_fieldcache(graphics_to_object_data.field_module);
			graphics_to_object_data.fe_region = scene->fe_region;
			graphics_to_object_data.data_fe_region = scene->data_fe_region;
			graphics_to_object_data.master_mesh = 0;
			graphics_to_object_data.iteration_mesh = 0;
			graphics_to_object_data.scenefilter = scenefilter;
			graphics_to_object_data.time = time;
			graphics_to_object_data.selection_group_field = cmzn_field_group_base_cast(
				cmzn_scene_get_selection_group(scene));
			graphics_to_object_data.iso_surface_specification = NULL;
			return_code = FOR_EACH_OBJECT_IN_LIST(cmzn_graphics)(
				cmzn_graphics_to_graphics_object, (void *) &graphics_to_object_data,
				scene->list_of_graphics);
			MANAGER_END_CACHE(Computed_field)(scene->computed_field_manager);
			if (graphics_to_object_data.selection_group_field)
			{
				cmzn_field_destroy(&graphics_to_object_data.selection_group_field);
			}
			cmzn_fieldcache_destroy(&graphics_to_object_data.field_cache);
			cmzn_fieldmodule_destroy(&graphics_to_object_data.field_module);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_scene_build_graphics_objects.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

int cmzn_scene_get_position(struct cmzn_scene *scene)
{
	int return_code;

	ENTER(cmzn_scene_set_position);
	if (scene)
	{
		return_code = scene->position;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_scene_set_position.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* cmzn_scene_set_position */



int cmzn_scene_set_position(struct cmzn_scene *scene, unsigned int position)
{
	int return_code;

	ENTER(cmzn_scene_set_position);
	if (scene&&(0<position))
	{
		scene->position=position;
		return_code=1;

	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_scene_set_position.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* cmzn_scene_set_position */

/***************************************************************************//**
 * Get the range of coordinates of visible graphics in the scene and all its
 * child region scenes.
 *
 * @param scene  The scene to get the range of.
 * @param scenefilter  The filter to apply to reduce scene contents.
 * @param graphics_object_range_void void pointer to graphics_object_range
 * @return If successfully get the range, otherwise NULL
 */
int cmzn_scene_get_range(cmzn_scene_id scene,
	cmzn_scene_id top_scene,
	cmzn_scenefilter_id filter,
	struct Graphics_object_range_struct *graphics_object_range)
{
	double coordinates[4],transformed_coordinates[4];
	gtMatrix *transformation = NULL;
	int i,j,k,return_code;
	struct Graphics_object_range_struct temp_graphics_object_range;
	struct cmzn_graphics_range graphics_range;

	if (top_scene && scene && graphics_object_range)
	{
		/* must first build graphics objects */
		Render_graphics_build_objects renderer;
		renderer.set_Scene(scene);
		renderer.setScenefilter(filter);

		renderer.cmzn_scene_compile(scene);
		if (NULL != (transformation = cmzn_scene_get_total_transformation(
			scene, top_scene)))
		{
			temp_graphics_object_range.first = 1;
			graphics_range.graphics_object_range = &temp_graphics_object_range;
		}
		else
		{
			graphics_range.graphics_object_range = graphics_object_range;
		}
		graphics_range.filter = filter;
		graphics_range.coordinate_system = CMZN_SCENE_COORDINATE_SYSTEM_LOCAL;
		return_code = FOR_EACH_OBJECT_IN_LIST(cmzn_graphics)(
			cmzn_graphics_get_visible_graphics_object_range, (void *)&graphics_range,
			scene->list_of_graphics);
		if (return_code&&transformation&&(!temp_graphics_object_range.first))
		{
			coordinates[3]=1.0;
			/* transform and compare ranges of each of 8 corners of the cube */
			for (i=0;i<8;i++)
			{
				if (i & 1)
				{
					coordinates[0]=temp_graphics_object_range.maximum[0];
				}
				else
				{
					coordinates[0]=temp_graphics_object_range.minimum[0];
				}
				if (i & 2)
				{
					coordinates[1]=temp_graphics_object_range.maximum[1];
				}
				else
				{
					coordinates[1]=temp_graphics_object_range.minimum[1];
				}
				if (i & 4)
				{
					coordinates[2]=temp_graphics_object_range.maximum[2];
				}
				else
				{
					coordinates[2]=temp_graphics_object_range.minimum[2];
				}
				for (j=0;j<4;j++)
				{
					transformed_coordinates[j]=0.0;
					for (k=0;k<4;k++)
					{
						transformed_coordinates[j] +=
							(*transformation)[k][j]*coordinates[k];
					}
				}
				if (0.0<transformed_coordinates[3])
				{
					transformed_coordinates[0] /= transformed_coordinates[3];
					transformed_coordinates[1] /= transformed_coordinates[3];
					transformed_coordinates[2] /= transformed_coordinates[3];
					for (j=0;j<3;j++)
					{
						if (graphics_object_range->first)
						{
							graphics_object_range->minimum[j]=
								graphics_object_range->maximum[j]=transformed_coordinates[j];
						}
						else
						{
							if (transformed_coordinates[j] >
								graphics_object_range->maximum[j])
							{
								graphics_object_range->maximum[j]=transformed_coordinates[j];
							}
							else if (transformed_coordinates[j] <
								graphics_object_range->minimum[j])
							{
								graphics_object_range->minimum[j]=transformed_coordinates[j];
							}
						}
					}
					graphics_object_range->first=0;
				}
			}
		}
		graphics_range.graphics_object_range = graphics_object_range;
		graphics_range.coordinate_system = CMZN_SCENE_COORDINATE_SYSTEM_WORLD;
		return_code = FOR_EACH_OBJECT_IN_LIST(cmzn_graphics)(
			cmzn_graphics_get_visible_graphics_object_range, (void *)&graphics_range,
			scene->list_of_graphics);
		if (transformation)
			DEALLOCATE(transformation);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_scene_get_range.  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} /* cmzn_scene_get_range */

cmzn_graphics_module * cmzn_scene_get_graphics_module(cmzn_scene_id scene)
{
	return cmzn_graphics_module_access(scene->graphics_module);
}

struct cmzn_scene *cmzn_region_get_scene_private(struct cmzn_region *region)
{
	cmzn_scene *scene = 0;
	if (region)
	{
		scene = FIRST_OBJECT_IN_LIST_THAT(ANY_OBJECT(cmzn_scene))(
			(ANY_OBJECT_CONDITIONAL_FUNCTION(cmzn_scene) *)NULL, (void *)NULL,
			cmzn_region_private_get_any_object_list(region));
	}
	return scene;
}

cmzn_scene_id cmzn_region_get_scene(cmzn_region_id region)
{
	return cmzn_scene_access(cmzn_region_get_scene_private(region));
}

/** @return non-accessed parent region scene, if any */
static cmzn_scene_id cmzn_scene_get_parent_scene_internal(cmzn_scene_id scene)
{
	cmzn_scene_id parent_scene = 0;
	if (scene)
	{
		cmzn_region_id parent_region = cmzn_region_get_parent_internal(scene->region);
		if (parent_region)
		{
			parent_scene = FIRST_OBJECT_IN_LIST_THAT(ANY_OBJECT(cmzn_scene))(
				(ANY_OBJECT_CONDITIONAL_FUNCTION(cmzn_scene) *)NULL, (void *)NULL,
				cmzn_region_private_get_any_object_list(parent_region));
		}
	}
	return parent_scene;
}

int cmzn_region_has_scene(struct cmzn_region *cmiss_region)
{
	int return_code = 0;

	ENTER(cmzn_region_has_scene);
	if (cmiss_region)
	{
		if (FIRST_OBJECT_IN_LIST_THAT(ANY_OBJECT(cmzn_scene))(
			(ANY_OBJECT_CONDITIONAL_FUNCTION(cmzn_scene) *)NULL, (void *)NULL,
			cmzn_region_private_get_any_object_list(cmiss_region)))
		{
			return_code = 1;
		}
		else
		{
			return_code = 0;
		}
	}
	LEAVE;

	return (return_code);
}

int cmzn_region_deaccess_scene(struct cmzn_region *region)
{
	int return_code = 1;
	struct cmzn_scene *scene;

	if (region)
	{
		struct LIST(Any_object) *list = cmzn_region_private_get_any_object_list(region);
		if (NUMBER_IN_LIST(Any_object)(list) > 0)
		{
			scene = FIRST_OBJECT_IN_LIST_THAT(ANY_OBJECT(cmzn_scene))(
				(ANY_OBJECT_CONDITIONAL_FUNCTION(cmzn_scene) *)NULL, (void *)NULL,	list);
			if (scene)
			{
				/* Clear graphics module to avoid being called back when scene is detached
				 * from region. @see cmzn_scene_void_detach_from_cmzn_region */
				cmzn_scene_detach_from_owner(scene);
				scene->graphics_module = NULL;
				REMOVE_OBJECT_FROM_LIST(ANY_OBJECT(cmzn_scene))(scene, list);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_region_deaccess_scene. Scene does not exist");
		return_code = 0;
	}
	return (return_code);
}

int cmzn_scene_destroy(struct cmzn_scene **scene)
{
	int return_code;

	ENTER(cmzn_scene_destroy);
	if (scene && *scene)
	{
		return_code = DEACCESS(cmzn_scene)(scene);
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return return_code;
}

void cmzn_scene_glyph_change(struct cmzn_scene *scene,
	struct MANAGER_MESSAGE(cmzn_glyph) *manager_message)
{
	if (scene && manager_message)
	{
		cmzn_scene_begin_change(scene);
		FOR_EACH_OBJECT_IN_LIST(cmzn_graphics)(cmzn_graphics_glyph_change,
			(void *)manager_message, scene->list_of_graphics);
		// inform child scenes of changes
		cmzn_region *child = cmzn_region_get_first_child(scene->region);
		while (child)
		{
			cmzn_scene_id child_scene = cmzn_region_get_scene_private(child);
			cmzn_scene_glyph_change(child_scene, manager_message);
			cmzn_region_reaccess_next_sibling(&child);
		}
		cmzn_scene_end_change(scene);
	}
}

void cmzn_scene_material_change(struct cmzn_scene *scene,
	struct MANAGER_MESSAGE(Graphical_material) *manager_message)
{
	if (scene && manager_message)
	{
		cmzn_scene_begin_change(scene);
		FOR_EACH_OBJECT_IN_LIST(cmzn_graphics)(
			cmzn_material_change, (void *)manager_message,
			scene->list_of_graphics);
		// inform child scenes of changes
		cmzn_region *child = cmzn_region_get_first_child(scene->region);
		while (child)
		{
			cmzn_scene_id child_scene = cmzn_region_get_scene_private(child);
			cmzn_scene_material_change(child_scene, manager_message);
			cmzn_region_reaccess_next_sibling(&child);
		}
		cmzn_scene_end_change(scene);
	}
}

void cmzn_scene_spectrum_change(struct cmzn_scene *scene,
	struct MANAGER_MESSAGE(Spectrum) *manager_message)
{
	if (scene && manager_message)
	{
		cmzn_scene_begin_change(scene);
		FOR_EACH_OBJECT_IN_LIST(cmzn_graphics)(
			cmzn_graphics_spectrum_change, (void *)manager_message,
			scene->list_of_graphics);
		// inform child scenes of changes
		cmzn_region *child = cmzn_region_get_first_child(scene->region);
		while (child)
		{
			cmzn_scene_id child_scene = cmzn_region_get_scene_private(child);
			cmzn_scene_spectrum_change(child_scene, manager_message);
			cmzn_region_reaccess_next_sibling(&child);
		}
		cmzn_scene_end_change(scene);
	}
}

void cmzn_scene_tessellation_change(struct cmzn_scene *scene,
	struct MANAGER_MESSAGE(cmzn_tessellation) *manager_message)
{
	if (scene && manager_message)
	{
		cmzn_scene_begin_change(scene);
		FOR_EACH_OBJECT_IN_LIST(cmzn_graphics)(
			cmzn_graphics_tessellation_change, (void *)manager_message,
			scene->list_of_graphics);
		// inform child scenes of changes
		cmzn_region *child = cmzn_region_get_first_child(scene->region);
		while (child)
		{
			cmzn_scene_id child_scene = cmzn_region_get_scene_private(child);
			cmzn_scene_tessellation_change(child_scene, manager_message);
			cmzn_region_reaccess_next_sibling(&child);
		}
		cmzn_scene_end_change(scene);
	}
}

void cmzn_scene_font_change(struct cmzn_scene *scene,
	struct MANAGER_MESSAGE(cmzn_font) *manager_message)
{
	if (scene && manager_message)
	{
		cmzn_scene_begin_change(scene);
		FOR_EACH_OBJECT_IN_LIST(cmzn_graphics)(
			cmzn_graphics_font_change, (void *)manager_message,
			scene->list_of_graphics);
		cmzn_region *child = cmzn_region_get_first_child(scene->region);
		while (child)
		{
			cmzn_scene_id child_scene = cmzn_region_get_scene_private(child);
			cmzn_scene_font_change(child_scene, manager_message);
			cmzn_region_reaccess_next_sibling(&child);
		}
		cmzn_scene_end_change(scene);
	}
}

int cmzn_scene_compile_graphics(cmzn_scene *scene,
	Render_graphics_compile_members *renderer)
{
	int return_code;

	if (scene)
	{
		/* check whether scene contents need building */
		return_code = cmzn_scene_build_graphics_objects(scene,
			renderer->getScenefilter(), renderer->time, renderer->name_prefix);
	/* Call the renderer to compile each of the graphics */
		FOR_EACH_OBJECT_IN_LIST(cmzn_graphics)(
			cmzn_graphics_compile_visible_graphics, (void *)renderer,
			scene->list_of_graphics);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_scene_compile_graphics.  Invalid argument(s)");
		return_code = 0;
	}

	return (return_code);
} /* cmzn_scene_compile_members  */

int cmzn_scene_compile_scene(cmzn_scene *scene,
	Render_graphics_compile_members *renderer)
{
	int return_code = 1;

	if (scene)
	{
		if (scene->time_notifier)
		{
			renderer->time = cmzn_timenotifier_get_time(scene->time_notifier);
		}
		else
		{
			renderer->time = 0;
		}
		renderer->name_prefix = cmzn_region_get_path(scene->region);
		return_code = renderer->cmzn_scene_compile_members(scene);
		DEALLOCATE(renderer->name_prefix);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_scene_compile.  Invalid argument(s)");
		return_code = 0;
	}

	return (return_code);
}

int cmzn_scene_graphics_render_opengl(struct cmzn_scene *scene,
	Render_graphics_opengl *renderer)
/*******************************************************************************
LAST MODIFIED : 27 June 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	if (scene)
	{
		glPushName(0);
		FOR_EACH_OBJECT_IN_LIST(cmzn_graphics)(
			cmzn_graphics_execute_visible_graphics,
			renderer, scene->list_of_graphics);
		glPopName();
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_scene_graphics_render_opengl.  Invalid argument(s)");
		return_code = 0;
	}

	return (return_code);
} /* cmzn_scene_graphics_render_opengl */

int cmzn_scene_render_child_scene(struct cmzn_scene *scene,
	Render_graphics_opengl *renderer)
{
	int return_code;
	struct cmzn_region *region, *child_region = NULL;
	struct cmzn_scene *child_scene;

	ENTER(cmzn_scene_execute_child_scene);
	if (scene && scene->region)
	{
		return_code = 1;
		region = ACCESS(cmzn_region)(scene->region);
		child_region = cmzn_region_get_first_child(region);
		while (child_region)
		{
			child_scene = cmzn_region_get_scene_private(child_region);
			if (child_scene)
			{
				renderer->cmzn_scene_execute(child_scene);
			}
			cmzn_region_reaccess_next_sibling(&child_region);
		}
		DEACCESS(cmzn_region)(&region);
	}
	else
	{
		display_message(ERROR_MESSAGE,"cmzn_scene_render_child_scene.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);

}

int execute_cmzn_scene(struct cmzn_scene *scene,
	Render_graphics_opengl *renderer)
{
	int return_code;

	ENTER(execute_cmzn_scene);
	if (scene)
	{
		return_code = 1;
		/* put out the name (position) of the scene: */
		//
		//printf("%i \n", scene->position);
		glLoadName((GLuint)scene->position);
		/* save a matrix multiply when identity transformation */
		if(scene->transformation)
		{
			/* Save starting modelview matrix */
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glPushAttrib(GL_TRANSFORM_BIT);
			glEnable(GL_NORMALIZE);
			/* perform individual object transformation */
			wrapperMultiplyCurrentMatrix(scene->transformation);
		}
		if (scene->time_notifier)
		{
			renderer->time = cmzn_timenotifier_get_time(scene->time_notifier);
		}
		else
		{
			renderer->time = 0;
		}
		return_code = renderer->cmzn_scene_execute_graphics(scene);
		return_code = renderer->cmzn_scene_execute_child_scene(scene);
		if (scene->transformation)
		{
			/* Restore starting modelview matrix */
			glPopAttrib();
			glPopMatrix();
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_cmzn_scene.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

struct cmzn_graphics *cmzn_scene_get_first_graphics_with_condition(
	struct cmzn_scene *scene,
	LIST_CONDITIONAL_FUNCTION(cmzn_graphics) *conditional_function,
	void *data)
{
	struct cmzn_graphics *graphics;

	if (scene)
	{
		graphics=FIRST_OBJECT_IN_LIST_THAT(cmzn_graphics)(
			conditional_function,data,scene->list_of_graphics);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_scene_get_first_graphics_with_condition.  Invalid arguments");
		graphics=(struct cmzn_graphics *)NULL;
	}

	return (graphics);
}

cmzn_graphics_id cmzn_scene_find_graphics_by_name(cmzn_scene_id scene,
	const char *graphics_name)
{
	if (scene && graphics_name)
	{
		cmzn_graphics_id graphics = FIRST_OBJECT_IN_LIST_THAT(cmzn_graphics)(
			cmzn_graphics_same_name, (void *)graphics_name, scene->list_of_graphics);
		if (graphics)
			return cmzn_graphics_access(graphics);
	}

	return NULL;
}

int cmzn_region_modify_scene(struct cmzn_region *region,
	struct cmzn_graphics *graphics, int delete_flag, int position)
{
	int return_code;

	ENTER(cmzn_region_modify_scene);
	if (region && graphics)
	{
		struct cmzn_scene *scene = cmzn_region_get_scene_private(region);
		if (scene)
		{
			cmzn_graphics *same_graphics = 0;
			// can only edit graphics with same name
			char *name = cmzn_graphics_get_name(graphics);
			if (name)
			{
				same_graphics = cmzn_scene_get_first_graphics_with_condition(
					scene, cmzn_graphics_same_name, (void *)name);
				DEALLOCATE(name);
			}
			if (delete_flag)
			{
				/* delete */
				if (same_graphics)
				{
					return_code = (CMZN_OK == cmzn_scene_remove_graphics(scene, same_graphics));
				}
				else
				{
					return_code = 1;
				}
			}
			else
			{
				/* add/modify */
				if (same_graphics)
				{
					ACCESS(cmzn_graphics)(same_graphics);
					if (-1 != position)
					{
						/* move same_graphics to new position */
						cmzn_scene_remove_graphics(scene, same_graphics);
						cmzn_scene_add_graphics(scene, same_graphics, position);
					}
					/* modify same_graphics to match new ones */
					return_code = 1;
					if (!cmzn_graphics_get_scene_private(same_graphics))
						cmzn_graphics_set_scene_private(same_graphics, scene);
					DEACCESS(cmzn_graphics)(&same_graphics);
				}
				else
				{
					return_code = 0;
					if (NULL != (same_graphics = CREATE(cmzn_graphics)(
								 cmzn_graphics_get_graphics_type(graphics))))
					{
						if (cmzn_graphics_copy_without_graphics_object(
							same_graphics, graphics))
						{
							return_code = cmzn_scene_add_graphics(scene,
								same_graphics, position);
						}
						DEACCESS(cmzn_graphics)(&same_graphics);
					}
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"cmzn_region_modify_scene.  Region scene cannot be found");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_region_modify_scene.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* cmzn_region_modify_scene */

int cmzn_scene_add_callback(struct cmzn_scene *scene,
	cmzn_scene_callback callback, void *user_data)
{
	int return_code;
	struct cmzn_scene_callback_data *callback_data, *previous;

	ENTER(cmzn_scene_add_callback);

	if (scene && callback)
	{
		if(ALLOCATE(callback_data, struct cmzn_scene_callback_data, 1))
		{
			callback_data->callback = callback;
			callback_data->callback_user_data = user_data;
			callback_data->next = (struct cmzn_scene_callback_data *)NULL;
			if(scene->update_callback_list)
			{
				previous = scene->update_callback_list;
				while(previous->next)
				{
					previous = previous->next;
				}
				previous->next = callback_data;
			}
			else
			{
				scene->update_callback_list = callback_data;
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"cmzn_scene_add_callback.  Unable to allocate callback data structure");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_scene_add_callback.  Missing scene object or callback");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}

int cmzn_scene_remove_callback(struct cmzn_scene *scene,
	cmzn_scene_callback callback, void *user_data)
{
	int return_code;
	struct cmzn_scene_callback_data *callback_data, *previous;

	ENTER(cmzn_scene_remove_callback);

	if (scene && callback && scene->update_callback_list)
	{
		callback_data = scene->update_callback_list;
		if((callback_data->callback == callback)
			&& (callback_data->callback_user_data == user_data))
		{
			scene->update_callback_list = callback_data->next;
			DEALLOCATE(callback_data);
			return_code = 1;
		}
		else
		{
			return_code = 0;
			while(!return_code && callback_data->next)
			{
				previous = callback_data;
				callback_data = callback_data->next;
				if((callback_data->callback == callback)
					&& (callback_data->callback_user_data == user_data))
				{
					previous->next = callback_data->next;
					DEALLOCATE(callback_data);
					return_code = 1;
				}
			}
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,
"cmzn_scene_remove_callback.  Unable to find callback and user_data specified");
			}
		}
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* cmzn_scene_remove_callback */

int cmzn_scene_for_each_material(struct cmzn_scene *scene,
	MANAGER_ITERATOR_FUNCTION(Graphical_material) *iterator_function,
	void *user_data)
{
	int return_code;

	ENTER(cmzn_scene_for_each_material);
	if (scene && iterator_function && user_data)
	{
		/* Could be smarter if there was a reduced number used by the
			scene, however for now just do every material in the manager */
		cmzn_materialmodule_id materialmodule =
			cmzn_graphics_module_get_materialmodule(scene->graphics_module);
		return_code = FOR_EACH_OBJECT_IN_MANAGER(Graphical_material)(
			iterator_function, user_data, cmzn_materialmodule_get_manager(materialmodule));
		cmzn_materialmodule_destroy(&materialmodule);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_scene_for_each_material.  Invalid arguments.");
		return_code=0;
	}
	LEAVE;

	return return_code;
}

int for_each_child_scene_in_scene_tree(
	struct cmzn_scene *scene,
	int (*cmzn_scene_tree_iterator_function)(struct cmzn_scene *scene,
		void *user_data),	void *user_data)
{
	int return_code;
	struct cmzn_region *region, *child_region = NULL;
	struct cmzn_scene *child_scene;

	ENTER(for_each_child_scene_in_scene_tree);
	if (scene)
	{
		region = ACCESS(cmzn_region)(scene->region);
		return_code = (*cmzn_scene_tree_iterator_function)(scene, user_data);
		if (return_code)
		{
			child_region = cmzn_region_get_first_child(region);
			while (child_region)
			{
				child_scene = cmzn_region_get_scene_private(child_region);
				if (child_scene)
				{
					return_code = for_each_child_scene_in_scene_tree(
						child_scene, cmzn_scene_tree_iterator_function,user_data);
				}
				cmzn_region_reaccess_next_sibling(&child_region);
			}
		}
		DEACCESS(cmzn_region)(&region);
	}
	else
	{
		display_message(ERROR_MESSAGE,"for_each_child_scene_in_scene_tree.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}

int cmzn_scene_get_graphics_position(
	struct cmzn_scene *scene, struct cmzn_graphics *graphics)
{
	int position;
	if (scene&&graphics)
	{
		position = cmzn_graphics_get_position_in_list(graphics,
			scene->list_of_graphics);
	}
	else
	{
		position = 0;
	}
	return (position);
}

int cmzn_scenes_match(struct cmzn_scene *scene1,
	struct cmzn_scene *scene2)
{
	int i, number_of_graphics, return_code;
	struct cmzn_graphics *graphics1, *graphics2;

	ENTER(cmzn_scenes_match);
	if (scene1 && scene2)
	{
		number_of_graphics = NUMBER_IN_LIST(cmzn_graphics)(scene1->list_of_graphics);
		if ((scene1->fe_region == scene2->fe_region) &&
			(scene1->data_fe_region == scene2->data_fe_region) &&
			(number_of_graphics == NUMBER_IN_LIST(cmzn_graphics)(scene2->list_of_graphics)))
		{
			return_code = 1;
			for (i = 1; return_code && (i <= number_of_graphics); i++)
			{
				graphics1 = FIND_BY_IDENTIFIER_IN_LIST(cmzn_graphics, position)(
					i, scene1->list_of_graphics);
				graphics2 = FIND_BY_IDENTIFIER_IN_LIST(cmzn_graphics, position)(
					i, scene2->list_of_graphics);
				return_code = cmzn_graphics_match(graphics1, graphics2);
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
			"cmzn_scenes_match.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* cmzn_scenes_match */

/***************************************************************************//**
 * All general settings are deprecated.
 */
static void cmzn_scene_copy_general_settings(struct cmzn_scene *destination,
	struct cmzn_scene *source)
{
	if (source->element_divisions)
	{
		int *temp = NULL;
		REALLOCATE(temp, destination->element_divisions, int, source->element_divisions_size);
		if (temp)
		{
			for (int i = 0; i < source->element_divisions_size; i++)
			{
				temp[i] = source->element_divisions[i];
			}
			destination->element_divisions = temp;
			destination->element_divisions_size = source->element_divisions_size;
		}
	}
	else
	{
		DEALLOCATE(destination->element_divisions);
		destination->element_divisions_size = 0;
	}
	destination->circle_discretization = source->circle_discretization;
	REACCESS(Computed_field)(&(destination->default_coordinate_field), source->default_coordinate_field);
}

int cmzn_scene_copy(struct cmzn_scene *destination,
	struct cmzn_scene *source)
/***************************************************************************//**
 * Copies the cmzn_scene contents from source to destination.
 * Pointers to graphics_objects are cleared in the destination list of graphics.
 * NOTES:
 * - not a full copy; does not copy groups, selection etc. Use copy_create for
 * this task so that callbacks can be set up for these.
 * - does not copy graphics objects to graphics in destination.
 */
{
	int return_code;

	ENTER(cmzn_scene_copy);
	if (destination&&source)
	{
		cmzn_scene_copy_general_settings(destination, source);
		/* empty original list_of_graphics */
		REMOVE_ALL_OBJECTS_FROM_LIST(cmzn_graphics)(
			destination->list_of_graphics);
		/* put copy of each settings in source list in destination list */
		FOR_EACH_OBJECT_IN_LIST(cmzn_graphics)(
			cmzn_graphics_copy_and_put_in_list,
			(void *)destination->list_of_graphics,source->list_of_graphics);
		FOR_EACH_OBJECT_IN_LIST(cmzn_graphics)(
			cmzn_graphics_set_scene_for_list_private,
			destination, destination->list_of_graphics);
		destination->visibility_flag = source->visibility_flag;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_scene_copy.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* cmzn_scene_copy */

struct cmzn_scene *create_editor_copy_cmzn_scene(
	struct cmzn_scene *existing_scene)
{
	struct cmzn_scene *scene;

	ENTER(create_editor_copy_cmzn_scene);
	if (existing_scene)
	{
		/* make an empty cmzn_scene for the same groups */
		if (NULL != (scene = CREATE(cmzn_scene)(
			existing_scene->region, existing_scene->graphics_module)))
		{
			/* copy settings WITHOUT graphics objects; do not cause whole function
				 to fail if copy fails */
			cmzn_scene_copy(scene,existing_scene);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_editor_copy_cmzn_scene.  Invalid argument(s)");
		scene=(struct cmzn_scene *)NULL;
	}
	LEAVE;

	return (scene);
} /* create_editor_copy_cmzn_scene */

int for_each_graphics_in_cmzn_scene(struct cmzn_scene *scene,
	int (*cmzn_scene_graphics_iterator_function)(struct cmzn_graphics *graphics,
		void *user_data),	void *user_data)
{
	int return_code = 0;

	ENTER( for_each_graphics_in_cmzn_scene);
	if (scene&&cmzn_scene_graphics_iterator_function)
	{
		return_code = FOR_EACH_OBJECT_IN_LIST(cmzn_graphics)(
			*cmzn_scene_graphics_iterator_function,user_data,
			scene->list_of_graphics);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"for_each_graphics_in_cmzn_scene.  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
}

int cmzn_scene_get_number_of_graphics(struct cmzn_scene *scene)
{
	int number_of_graphics;
	if (scene)
	{
		number_of_graphics =
			NUMBER_IN_LIST(cmzn_graphics)(scene->list_of_graphics);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_group_get_number_of_graphics.  Invalid argument(s)");
		number_of_graphics = 0;
	}
	return (number_of_graphics);
}

struct cmzn_graphics *cmzn_scene_get_graphics_at_position(
	struct cmzn_scene *scene,int position)
{
	struct cmzn_graphics *graphics;

	ENTER(get_graphics_at_position_in_cmzn_scene);
	if (scene)
	{
		graphics=FIND_BY_IDENTIFIER_IN_LIST(cmzn_graphics,
			position)(position,scene->list_of_graphics);
		if (graphics)
		{
			ACCESS(cmzn_graphics)(graphics);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"get_graphics_at_position_in_cmzn_scene.  Invalid arguments");
		graphics=(struct cmzn_graphics *)NULL;
	}
	LEAVE;

	return (graphics);
} /* get_graphics_at_position_in_cmzn_scene */

struct cmzn_region *cmzn_scene_get_region(
	struct cmzn_scene *scene)
{
	struct cmzn_region *region;

	ENTER(cmzn_scene_get_region);
	if (scene)
	{
		region=scene->region;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_scene_get_region.  Invalid arguments");
		region=(struct cmzn_region *)NULL;
	}
	LEAVE;

	return (region);
}

int cmzn_scene_modify(struct cmzn_scene *destination,
	struct cmzn_scene *source)
{
	int return_code;
	struct LIST(cmzn_graphics) *temp_list_of_graphics;

	ENTER(cmzn_scene_modify);
	if (destination && source)
	{
		if (NULL != (temp_list_of_graphics = CREATE(LIST(cmzn_graphics))()))
		{
			cmzn_scene_copy_general_settings(destination, source);
			/* make copy of source graphics without graphics objects */
			FOR_EACH_OBJECT_IN_LIST(cmzn_graphics)(
				cmzn_graphics_copy_and_put_in_list,
				(void *)temp_list_of_graphics, source->list_of_graphics);
			/* extract graphics objects that can be reused from destination list */
			FOR_EACH_OBJECT_IN_LIST(cmzn_graphics)(
				cmzn_graphics_extract_graphics_object_from_list,
				(void *)destination->list_of_graphics, temp_list_of_graphics);
			FOR_EACH_OBJECT_IN_LIST(cmzn_graphics)(
				cmzn_graphics_set_scene_for_list_private,
				destination, temp_list_of_graphics);
			/* replace the destination list of graphics with temp_list_of_graphics */
			struct LIST(cmzn_graphics) *destroy_list_of_graphics = destination->list_of_graphics;
			destination->list_of_graphics = temp_list_of_graphics;
			/* destroy list afterwards to avoid manager messages halfway through change */
			DESTROY(LIST(cmzn_graphics))(&destroy_list_of_graphics);
			/* inform the client of the change */
			cmzn_scene_changed(destination);
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"cmzn_scene_modify.  Could not create temporary list");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_scene_modify.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* cmzn_scene_modify */

bool cmzn_scene_get_visibility_flag(
	struct cmzn_scene *scene)
{
	if (scene)
		return scene->visibility_flag;
	return false;
}

int cmzn_scene_set_visibility_flag(struct cmzn_scene *scene,
	bool visibility_flag)
{
	if (scene)
	{
		if (scene->visibility_flag != visibility_flag)
		{
			scene->visibility_flag = visibility_flag;
			cmzn_scene_changed(scene);
		}
		return CMZN_OK;
	}
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_scene_is_visible_hierarchical(
	struct cmzn_scene *scene)
{
	int return_code = 1;
	if (scene)
	{
		return_code = scene->visibility_flag;
		if (return_code)
		{
			cmzn_scene *parent_scene = cmzn_scene_get_parent_scene_internal(scene);
			return_code = cmzn_scene_is_visible_hierarchical(parent_scene);
		}
	}
	return return_code;
}

struct cmzn_scene_spectrum_data_range
{
	struct Spectrum *spectrum;
	Graphics_object_data_range range;

	cmzn_scene_spectrum_data_range(cmzn_spectrum *spectrumIn, int valuesCount,
		double *minimumValues, double *maximumValues) :
		spectrum(spectrumIn),
		range(valuesCount, minimumValues, maximumValues)
	{
	}
};

/**
 * Expands the data range to include the data values of the graphics object
 * if it uses the specified spectrum.
 */
static int Graphics_object_get_spectrum_data_range_iterator(
	struct GT_object *graphics_object, double time, void *rangeData_void)
{
	USE_PARAMETER(time);
	struct cmzn_scene_spectrum_data_range *rangeData =
		reinterpret_cast<struct cmzn_scene_spectrum_data_range *>(rangeData_void);
	if (graphics_object && rangeData)
	{
		if (get_GT_object_spectrum(graphics_object) == rangeData->spectrum )
		{
			get_graphics_object_data_range(graphics_object, &(rangeData->range));
		}
		return 1;
	}
	return 0;
}

int cmzn_scene_get_spectrum_data_range(cmzn_scene_id scene,
	cmzn_scenefilter_id filter, cmzn_spectrum_id spectrum,
	int valuesCount, double *minimumValuesOut, double *maximumValuesOut)
{
	if (scene && spectrum && (0 < valuesCount) && minimumValuesOut && maximumValuesOut)
	{
		build_Scene(scene, filter);
		cmzn_scene_spectrum_data_range rangeData(spectrum, valuesCount, minimumValuesOut, maximumValuesOut);
		for_each_graphics_object_in_scene_tree(scene, filter,
			Graphics_object_get_spectrum_data_range_iterator, (void *)&rangeData);
		return rangeData.range.getMaxRanges();
	}
	return 0;
}

int cmzn_scene_add_transformation_callback(struct cmzn_scene *scene,
	CMZN_CALLBACK_FUNCTION(cmzn_scene_transformation) *function, void *user_data)
{
	int return_code;

	ENTER(cmzn_scene_add_transformation_callback);
	if (scene && function)
	{
		if (CMZN_CALLBACK_LIST_ADD_CALLBACK(cmzn_scene_transformation)(
			scene->transformation_callback_list, function, user_data))
		{
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"cmzn_scene_add_transformation_callback.  Could not add callback");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_scene_add_transformation_callback.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* cmzn_scene_add_transformation_callback */

int cmzn_scene_remove_transformation_callback(
	struct cmzn_scene *scene,
	CMZN_CALLBACK_FUNCTION(cmzn_scene_transformation) *function, void *user_data)
{
	int return_code;

	ENTER(cmzn_scene_remove_transformation_callback);
	if (scene && function)
	{
		if (CMZN_CALLBACK_LIST_REMOVE_CALLBACK(cmzn_scene_transformation)(
			scene->transformation_callback_list, function,user_data))
		{
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"cmzn_scene_remove_transformation_callback.  "
				"Could not remove callback");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_scene_remove_transformation_callback.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* cmzn_scene_remove_transformation_callback */

int cmzn_scene_has_transformation(struct cmzn_scene *scene)
{
	int return_code;

	ENTER(cmzn_scene_has_transformation);
	if (scene)
	{
		if(scene->transformation)
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
			"cmzn_scene_has_transformation.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* cmzn_scene_has_transformation */

int cmzn_scene_get_transformation(struct cmzn_scene *scene,
	gtMatrix *transformation)
{
	int i, j, return_code;

	ENTER(cmzn_scene_get_transformation);
	if (scene)
	{
		if(scene->transformation)
		{
			for (i=0;i<4;i++)
			{
				for (j=0;j<4;j++)
				{
					(*transformation)[i][j] = (*scene->transformation)[i][j];
				}
			}
		}
		else
		{
			/* Set the identity */
			(*transformation)[0][0] = 1.0;
			(*transformation)[0][1] = 0.0;
			(*transformation)[0][2] = 0.0;
			(*transformation)[0][3] = 0.0;
			(*transformation)[1][0] = 0.0;
			(*transformation)[1][1] = 1.0;
			(*transformation)[1][2] = 0.0;
			(*transformation)[1][3] = 0.0;
			(*transformation)[2][0] = 0.0;
			(*transformation)[2][1] = 0.0;
			(*transformation)[2][2] = 1.0;
			(*transformation)[2][3] = 0.0;
			(*transformation)[3][0] = 0.0;
			(*transformation)[3][1] = 0.0;
			(*transformation)[3][2] = 0.0;
			(*transformation)[3][3] = 1.0;
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_scene_get_transformation.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* cmzn_scene_get_transformation */

int cmzn_scene_set_transformation(struct cmzn_scene *scene,
	gtMatrix *transformation)
{
	int i, j, return_code;

	ENTER(cmzn_scene_set_transformation);
	if (scene)
	{
		return_code = 1;
		if ((!transformation) || gtMatrix_is_identity(transformation))
		{
			if (scene->transformation)
			{
				DEALLOCATE(scene->transformation);
			}
		}
		else
		{
			if (scene->transformation)
			{
				if (!gtMatrix_match(transformation, scene->transformation))
				{
					for (i = 0; i < 4; i++)
					{
						for (j = 0; j < 4; j++)
						{
							(*scene->transformation)[i][j] = (*transformation)[i][j];
						}
					}
				}
			}
			else
			{
				if (ALLOCATE(scene->transformation, gtMatrix, 1))
				{
					for (i = 0; i < 4; i++)
					{
						for (j = 0; j < 4; j++)
						{
							(*scene->transformation)[i][j] = (*transformation)[i][j];
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE, "cmzn_scene_set_transformation.  "
						"Unable to allocate transformation");
					return_code = 0;
				}
			}
		}
		CMZN_CALLBACK_LIST_CALL(cmzn_scene_transformation)(
			scene->transformation_callback_list, scene,
			scene->transformation);
		cmzn_scene_changed(scene);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_scene_set_transformation.  Missing scene");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* cmzn_scene_set_transformation */

int cmzn_scene_trigger_time_dependent_transformation(cmzn_scene_id scene, double current_time)
{
	int return_code;
	FE_value *values;
	gtMatrix transformation_matrix;

	if (scene && scene->transformation_field)
	{
		if (ALLOCATE(values, FE_value, 16))
		{
			cmzn_fieldmodule_id field_module = cmzn_region_get_fieldmodule(scene->region);
			cmzn_fieldcache_id field_cache = cmzn_fieldmodule_create_fieldcache(field_module);
			cmzn_fieldcache_set_time(field_cache, current_time);
			if (cmzn_field_evaluate_real(scene->transformation_field,
				field_cache, /*number_of_values*/16, values))
			{
				int i, j, k;
				k = 0;
				for (i = 0; i < 4; i++)
				{
					for (j = 0; j < 4; j++)
					{
						transformation_matrix[i][j] = values[k];
						k++;
					}
				}
				return_code = cmzn_scene_set_transformation(scene,
					&transformation_matrix);
			}
			else
			{
				return_code = 0;
			}
			cmzn_fieldcache_destroy(&field_cache);
			cmzn_fieldmodule_destroy(&field_module);
			DEALLOCATE(values);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"cmzn_scene_set_time_dependent_transformation.  "
				"Unable to allocate values.");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_scene_set_time_dependent_transformation.  "
			"invalid argument.");
		return_code=0;
	}

	return (return_code);
}

void cmzn_scene_remove_time_dependent_transformation(struct cmzn_scene *scene)
{
	if (scene->transformation_time_callback_flag)
	{
		 DEACCESS(Computed_field)(&(scene->transformation_field));
		 scene->transformation_time_callback_flag = 0;
	}
}

int cmzn_scene_set_transformation_with_time_callback(struct cmzn_scene *scene,
	 struct Computed_field *transformation_field)
{
	 int return_code = 0;

	 if (scene && transformation_field)
	 {
			if (scene->time_notifier)
			{
				 cmzn_scene_remove_time_dependent_transformation(scene);
				 scene->transformation_field=
						ACCESS(Computed_field)(transformation_field);
				 cmzn_scene_trigger_time_dependent_transformation(scene,
					 cmzn_timenotifier_get_time(scene->time_notifier));
				 scene->transformation_time_callback_flag = 1;
				 return_code = 1;
			}
			return_code = scene->transformation_time_callback_flag;
	 }
	 else
	 {
			display_message(ERROR_MESSAGE,
				 "cmzn_scene_set_transformation_with_time_callback.  "
				 "Invalid argument(s).");
			return_code=0;
	 }

	 return (return_code);
}

static int cmzn_scene_time_update_callback(cmzn_timenotifierevent_id timenotifierevent,
	void *scene_void)
{
	int return_code;
	struct cmzn_scene *scene;

	if ((scene=(struct cmzn_scene *)scene_void))
	{
		cmzn_scene_begin_change(scene);
		if (scene->transformation_time_callback_flag)
		{
			cmzn_scene_trigger_time_dependent_transformation(scene,
				cmzn_timenotifierevent_get_time(timenotifierevent));
		}
		FOR_EACH_OBJECT_IN_LIST(cmzn_graphics)(
			cmzn_graphics_time_change,NULL,
			scene->list_of_graphics);
		cmzn_scene_end_change(scene);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_scene_time_update_callback.  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
}

int cmzn_scene_has_multiple_times(
	struct cmzn_scene *scene)
{
	int return_code;
	struct cmzn_graphics_update_time_behaviour_data data;

	ENTER(cmzn_scene_has_multiple_times);
	if (scene)
	{
		data.default_coordinate_depends_on_time = 0;
		data.time_dependent = 0;
		FOR_EACH_OBJECT_IN_LIST(cmzn_graphics)(
			cmzn_graphics_update_time_behaviour, (void *)&data,
			scene->list_of_graphics);
		return_code = data.time_dependent;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_scene_has_multiple_times.  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* cmzn_scene_has_multiple_times */

cmzn_timenotifier *cmzn_scene_get_time_notifier(struct cmzn_scene *scene)
{
	cmzn_timenotifier *return_time;

	if (scene)
	{
		return_time=scene->time_notifier;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_scene_get_time_notifier.  Missing scene");
		return_time=(cmzn_timenotifier *)NULL;
	}

	return (return_time);
}

int cmzn_scene_set_time_notifier(struct cmzn_scene *scene,
	cmzn_timenotifier *time)
{
	int return_code;

	if (scene)
	{
		if (scene->time_notifier != time)
		{
			if (scene->time_notifier)
			{
				cmzn_timenotifier_clear_callback(scene->time_notifier);
			}
			REACCESS(Time_object)(&(scene->time_notifier),time);
			if (time)
			{
				cmzn_timenotifier_set_callback(scene->time_notifier,
					cmzn_scene_time_update_callback, scene);
			}
		}
		return_code=1;
	}
	else
	{
		return_code=0;
	}

	return (return_code);
} /* cmzn_scene_set_time_notifier */

static int cmzn_scene_update_time_behaviour(struct cmzn_scene *scene)
{
	int return_code;
	cmzn_timenotifier *time;

	if (scene)
	{
		return_code = 1;
		/* Ensure the Scene object has a time object if and only if the
			graphics object has more than one time */
		if(cmzn_scene_has_multiple_times(scene))
		{
			time = cmzn_scene_get_time_notifier(scene);
			if(!time)
			{
				time = Time_object_create_regular(/*update_frequency*/10.0,
					/*time_offset*/0.0);
				cmzn_scene_set_time_notifier(scene, time);
				struct cmzn_timekeeper *time_keeper = cmzn_graphics_module_get_timekeeper_internal(
					scene->graphics_module);
				if(time_keeper)
				{
					time_keeper->addTimeObject(time);
					DEACCESS(cmzn_timekeeper)(&time_keeper);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"cmzn_scene_update_time_behaviour.  "
						"Missing time keeper ");
					return_code =0;
				}
				cmzn_timenotifier_destroy(&time);
			}
		}
		else
		{
			time = cmzn_scene_get_time_notifier(scene);
			if(time == NULL)
			{
				cmzn_scene_set_time_notifier(scene,
					(cmzn_timenotifier *)NULL);
			}
		}
	}
	else
	{
		return_code=0;
	}
	return (return_code);
}

void cmzn_scene_detach_from_owner(cmzn_scene_id scene)
{
	if (scene)
	{
		// first notify selection clients as they call some scene APIs
		if (scene->selectionnotifier_list)
		{
			for (cmzn_selectionnotifier_list::iterator iter = scene->selectionnotifier_list->begin();
				iter != scene->selectionnotifier_list->end(); ++iter)
			{
				cmzn_selectionnotifier_id selectionnotifier = *iter;
				selectionnotifier->sceneDestroyed();
				cmzn_selectionnotifier::deaccess(selectionnotifier);
			}
			delete scene->selectionnotifier_list;
			scene->selectionnotifier_list = 0;
		}
		if (scene->computed_field_manager &&
			scene->computed_field_manager_callback_id)
		{
			MANAGER_DEREGISTER(Computed_field)(
				scene->computed_field_manager_callback_id,
				scene->computed_field_manager);
			scene->computed_field_manager_callback_id = 0;
			scene->computed_field_manager = 0;
		}
		cmzn_scene_remove_time_dependent_transformation(scene);
		if (scene->transformation_callback_list)
		{
			DESTROY(LIST(CMZN_CALLBACK_ITEM(cmzn_scene_transformation)))(
				&(scene->transformation_callback_list));
		}
		if (scene->top_region_change_callback_list)
		{
			DESTROY(LIST(CMZN_CALLBACK_ITEM(cmzn_scene_top_region_change)))(
				&(scene->top_region_change_callback_list));
		}
		if (scene->fe_region_callback_set)
		{
			FE_region_remove_callback(scene->fe_region,
				cmzn_scene_FE_region_change, (void *)scene);
			scene->fe_region_callback_set = 0;
		}
		if (scene->data_fe_region && scene->data_fe_region_callback_set)
		{
			FE_region_remove_callback(scene->data_fe_region,
				cmzn_scene_data_FE_region_change, (void *)scene);
			scene->data_fe_region_callback_set = 0;
		}
		if (scene->data_fe_region)
		{
			DEACCESS(FE_region)(&(scene->data_fe_region));
		}
		if (scene->fe_region)
		{
			DEACCESS(FE_region)(&(scene->fe_region));
		}
		struct cmzn_scene_callback_data *callback_data, *next;
		callback_data = scene->update_callback_list;
		while(callback_data)
		{
			next = callback_data->next;
			DEALLOCATE(callback_data);
			callback_data = next;
		}
		scene->update_callback_list = 0;
	}
}

int DESTROY(cmzn_scene)(struct cmzn_scene **scene_address)
{
	int return_code;
	struct cmzn_scene *scene;
	if (scene_address && (scene = *scene_address))
	{
		cmzn_scene_detach_from_owner(scene);
		if (scene->selection_group)
		{
			cmzn_field_group_destroy(&scene->selection_group);
		}
		if (scene->transformation)
		{
			DEALLOCATE(scene->transformation);
		}
		if (scene->time_notifier)
		{
			cmzn_timenotifier_destroy(&scene->time_notifier);
		}
		if (scene->default_coordinate_field)
		{
			DEACCESS(Computed_field)(&(scene->default_coordinate_field));
		}
		if (scene->element_divisions)
		{
			DEALLOCATE(scene->element_divisions);
		}
		if (scene->list_of_graphics)
		{
			DESTROY(LIST(cmzn_graphics))(&(scene->list_of_graphics));
		}
		if (scene->transformation_field)
		{
			DEACCESS(Computed_field)(&(scene->transformation_field));
		}
		DEALLOCATE(*scene_address);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(cmzn_scene).  Invalid argument(s)");
		return_code = 0;
	}
	return (return_code);
}

int cmzn_scene_set_selection_group(cmzn_scene_id scene,
	cmzn_field_group_id selection_group)
{
	if (!scene)
		return 0;
	int return_code = 1;
	if (selection_group != scene->selection_group)
	{
		cmzn_scene_begin_change(scene);
		bool isEmpty = true;
		if (selection_group)
		{
			isEmpty = cmzn_field_group_is_empty(selection_group);
			cmzn_field_access(cmzn_field_group_base_cast(selection_group));
		}
		bool wasEmpty = true;
		if (scene->selection_group)
		{
			wasEmpty = cmzn_field_group_is_empty(scene->selection_group) &&
				(!cmzn_field_group_was_modified(scene->selection_group));
			cmzn_field_group_destroy(&scene->selection_group);
		}
		scene->selection_group = selection_group;
		// ensure child scene selection_group matches the appropriate subgroup or none if none
		cmzn_field_group_id child_group = 0;
		cmzn_region_id child_region = cmzn_region_get_first_child(scene->region);
		while ((NULL != child_region))
		{
			cmzn_scene_id child_scene = cmzn_region_get_scene_private(child_region);
			if (child_scene)
			{
				if (selection_group)
				{
					child_group = cmzn_field_group_get_subregion_group(selection_group, child_region);
				}
				if (child_group != child_scene->selection_group)
				{
					cmzn_scene_set_selection_group(child_scene, child_group);
				}
				if (child_group)
				{
					cmzn_field_group_destroy(&child_group);
				}
			}
			cmzn_region_reaccess_next_sibling(&child_region);
		}
		if (!(wasEmpty && isEmpty))
		{
			if (scene->selectionnotifier_list)
			{
				// notify clients of changed selection
				cmzn_selectionevent *event = new cmzn_selectionevent();
				if (!wasEmpty)
					event->changeFlags |= CMZN_SELECTIONEVENT_CHANGE_FLAG_REMOVE;
				if (!isEmpty)
					event->changeFlags |= CMZN_SELECTIONEVENT_CHANGE_FLAG_ADD;
				for (cmzn_selectionnotifier_list::iterator iter = scene->selectionnotifier_list->begin();
					iter != scene->selectionnotifier_list->end(); ++iter)
				{
					(*iter)->notify(event);
				}
				cmzn_selectionevent_destroy(&event);
			}
			FOR_EACH_OBJECT_IN_LIST(cmzn_graphics)(
				cmzn_graphics_update_selected, NULL, scene->list_of_graphics);
		}
		cmzn_scene_changed(scene);
		cmzn_scene_end_change(scene);
	}
	return return_code;
}

cmzn_field_id cmzn_scene_get_selection_group_private_for_highlighting(cmzn_scene_id scene)
{
	cmzn_field_id selection_group = 0;
	if (scene)
	{
		if (scene->selection_group)
		{
			selection_group = cmzn_field_group_base_cast(scene->selection_group);
		}
	}
	return selection_group;
}


cmzn_field_group_id cmzn_scene_get_selection_group(cmzn_scene_id scene)
{
	cmzn_field_group_id selection_group = 0;
	if (scene)
	{
		if (scene->selection_group)
		{
			selection_group = scene->selection_group;
			cmzn_field_access(cmzn_field_group_base_cast(selection_group));
		}
	}
	return selection_group;
}

cmzn_field_group_id cmzn_scene_get_or_create_selection_group(cmzn_scene_id scene)
{
	cmzn_field_group_id selection_group = cmzn_scene_get_selection_group(scene);
	if (!selection_group && scene)
	{
		cmzn_scene_id parent_scene = cmzn_scene_get_parent_scene_internal(scene);
		if (parent_scene)
		{
			cmzn_field_group_id parent_selection_group = cmzn_scene_get_or_create_selection_group(parent_scene);
			selection_group = cmzn_field_group_get_subregion_group(parent_selection_group, scene->region);
			if (!selection_group)
			{
				selection_group = cmzn_field_group_create_subregion_group(parent_selection_group, scene->region);
			}
			cmzn_field_group_destroy(&parent_selection_group);
		}
		else
		{
			// find by name or create
			const char *default_selection_group_name = "cmiss_selection";
			cmzn_fieldmodule_id field_module = cmzn_region_get_fieldmodule(scene->region);
			cmzn_field_id field = cmzn_fieldmodule_find_field_by_name(field_module, default_selection_group_name);
			if (field)
			{
				selection_group = cmzn_field_cast_group(field);
				cmzn_field_destroy(&field);
			}
			if (!selection_group)
			{
				field = cmzn_fieldmodule_create_field_group(field_module);
				cmzn_field_set_name(field, default_selection_group_name);
				selection_group = cmzn_field_cast_group(field);
				cmzn_field_destroy(&field);
			}
			cmzn_fieldmodule_destroy(&field_module);
		}
		if (selection_group)
		{
			cmzn_scene_set_selection_group(scene, selection_group);
		}
	}
	return selection_group;
}

int cmzn_scene_change_selection_from_node_list(cmzn_scene_id scene,
		struct LIST(FE_node) *node_list, int add_flag, int use_data)
{
	int return_code = 1;

	ENTER(cmzn_scene_add_selection_from_node_list);
	if (scene && node_list && (NUMBER_IN_LIST(FE_node)(node_list) > 0))
	{
		cmzn_fieldmodule_id field_module = cmzn_region_get_fieldmodule(scene->region);
		cmzn_fieldmodule_begin_change(field_module);
		cmzn_field_group_id selection_group = cmzn_scene_get_or_create_selection_group(scene);
		cmzn_nodeset_id temp_nodeset = cmzn_fieldmodule_find_nodeset_by_domain_type(
			field_module, use_data ? CMZN_FIELD_DOMAIN_TYPE_DATAPOINTS : CMZN_FIELD_DOMAIN_TYPE_NODES);
		cmzn_field_node_group_id node_group = cmzn_field_group_get_node_group(selection_group, temp_nodeset);
		if (!node_group)
			node_group = cmzn_field_group_create_node_group(selection_group, temp_nodeset);
		cmzn_nodeset_destroy(&temp_nodeset);
		cmzn_nodeset_group_id nodeset_group = cmzn_field_node_group_get_nodeset(node_group);
		cmzn_field_node_group_destroy(&node_group);
		cmzn_nodeiterator_id iterator = CREATE_LIST_ITERATOR(FE_node)(node_list);
		cmzn_node_id node = 0;
		while (0 != (node = cmzn_nodeiterator_next_non_access(iterator)))
		{
			if (add_flag)
			{
				cmzn_nodeset_group_add_node(nodeset_group, node);
			}
			else
			{
				cmzn_nodeset_group_remove_node(nodeset_group, node);
			}
		}
		cmzn_nodeiterator_destroy(&iterator);
		cmzn_nodeset_group_destroy(&nodeset_group);
		cmzn_field_group_destroy(&selection_group);
		cmzn_fieldmodule_end_change(field_module);
		cmzn_fieldmodule_destroy(&field_module);
	}
	LEAVE;

	return (return_code);
}

int cmzn_scene_add_selection_from_node_list(cmzn_scene_id scene,
	struct LIST(FE_node) *node_list, int use_data)
/*******************************************************************************
LAST MODIFIED : 28 April 2000

DESCRIPTION :
Create a node list selection
==============================================================================*/
{
	int return_code = 0;
	return_code = cmzn_scene_change_selection_from_node_list(scene,
		node_list, /*add_flag*/1, use_data);
	return return_code;
}

int cmzn_scene_remove_selection_from_node_list(cmzn_scene_id scene,
	struct LIST(FE_node) *node_list, int use_data)
{
	int return_code = 0;
	if (cmzn_scene_change_selection_from_node_list(scene,
		node_list, /*add_flag*/0, use_data))
	{
		cmzn_scene_flush_tree_selections(scene);
		return_code = 1;
	}
	return return_code;
}

void cmzn_scene_flush_tree_selections(cmzn_scene_id scene)
{
	if (scene && scene->selection_group)
	{
		cmzn_field_group_remove_empty_subgroups(scene->selection_group);
		if (cmzn_field_group_is_empty(scene->selection_group))
		{
			cmzn_scene_set_selection_group(scene, 0);
		}
	}
}

int cmzn_scene_change_selection_from_element_list_of_dimension(cmzn_scene_id scene,
	struct LIST(FE_element) *element_list, int add_flag, int dimension)
{
	int return_code = 1;

	ENTER(cmzn_scene_change_selection_from_element_list_of_dimension);
	if (scene && element_list && (NUMBER_IN_LIST(FE_element)(element_list) > 0))
	{
		cmzn_fieldmodule_id field_module = cmzn_region_get_fieldmodule(scene->region);
		cmzn_fieldmodule_begin_change(field_module);
		cmzn_field_group_id selection_group = cmzn_scene_get_or_create_selection_group(scene);
		cmzn_mesh_id temp_mesh = cmzn_fieldmodule_find_mesh_by_dimension(field_module, dimension);
		cmzn_field_element_group_id element_group = cmzn_field_group_get_element_group(selection_group, temp_mesh);
		if (!element_group)
			element_group = cmzn_field_group_create_element_group(selection_group, temp_mesh);
		cmzn_mesh_destroy(&temp_mesh);
		cmzn_mesh_group_id mesh_group = cmzn_field_element_group_get_mesh(element_group);
		cmzn_field_element_group_destroy(&element_group);
		cmzn_elementiterator_id iterator = CREATE_LIST_ITERATOR(FE_element)(element_list);
		cmzn_element_id element = 0;
		while (0 != (element = cmzn_elementiterator_next_non_access(iterator)))
		{
			if (add_flag)
			{
				cmzn_mesh_group_add_element(mesh_group, element);
			}
			else
			{
				cmzn_mesh_group_remove_element(mesh_group, element);
			}
		}
		cmzn_elementiterator_destroy(&iterator);
		cmzn_mesh_group_destroy(&mesh_group);
		cmzn_field_group_destroy(&selection_group);
		cmzn_fieldmodule_end_change(field_module);
		cmzn_fieldmodule_destroy(&field_module);
	}
	LEAVE;

	return (return_code);
}

int cmzn_scene_add_selection_from_element_list_of_dimension(cmzn_scene_id scene,
	struct LIST(FE_element) *element_list, int dimension)
{
	return cmzn_scene_change_selection_from_element_list_of_dimension(scene,
		element_list, /*add_flag*/1, dimension);
}

int cmzn_scene_remove_selection_from_element_list_of_dimension(cmzn_scene_id scene,
	struct LIST(FE_element) *element_list, int dimension)
{
	int return_code = 0;
	if (cmzn_scene_change_selection_from_element_list_of_dimension(scene,
			element_list, /*add_flag*/0, dimension))
	{
		cmzn_scene_flush_tree_selections(scene);
		return_code = 1;
	}
	return return_code;
}

int cmzn_scene_remove_field_manager_and_callback(struct cmzn_scene *scene)
{
	int return_code = 0;
	if (scene->computed_field_manager &&
			scene->computed_field_manager_callback_id)
	{
			MANAGER_DEREGISTER(Computed_field)(
				scene->computed_field_manager_callback_id,
				scene->computed_field_manager);
			scene->computed_field_manager_callback_id = NULL;
			scene->computed_field_manager = NULL;
			return_code = 1;
	}
	return return_code;
}

int cmzn_scene_detach_fields(struct cmzn_scene *scene)
{
	int return_code = 1;
	if (scene)
	{
		//if (!scene->list_of_scene || scene->list_of_scene->empty())
		{
			cmzn_scene_remove_time_dependent_transformation(scene);
			if (scene->default_coordinate_field)
			{
				DEACCESS(Computed_field)(&(scene->default_coordinate_field));
			}
			if (scene->list_of_graphics)
			{
				FOR_EACH_OBJECT_IN_LIST(cmzn_graphics)(
					cmzn_graphics_detach_fields, (void *)NULL,
					scene->list_of_graphics);
			}
			if (scene->transformation_field)
			{
				DEACCESS(Computed_field)(&(scene->transformation_field));
			}
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_scene_detach_fields.  Invalid argument(s)");
		return_code = 0;
	}

	return return_code;
}

cmzn_scene *cmzn_scene_access(cmzn_scene_id scene)
{
	return (ACCESS(cmzn_scene)(scene));
}

int list_cmzn_scene_transformation_commands(struct cmzn_scene *scene,
	void *command_prefix_void)
/*******************************************************************************
LAST MODIFIED : 22 January 2002

DESCRIPTION :
Iterator function for writing the transformation in effect for <scene>
as a command, using the given <command_prefix>.
==============================================================================*/
{
	char *command_prefix, *region_name;
	int i,j,return_code;
	gtMatrix transformation_matrix;

	ENTER(list_cmzn_scene_transformation_commands);
	if (scene&&(command_prefix=(char *)command_prefix_void))
	{
		return_code=cmzn_scene_get_transformation(scene,
			&transformation_matrix);
		if (return_code)
		{
			region_name = cmzn_region_get_path(scene->region);
			/* put quotes around name if it contains special characters */
			make_valid_token(&region_name);
			display_message(INFORMATION_MESSAGE, "%s %s", command_prefix,
					region_name);
			DEALLOCATE(region_name);
			for (i=0;i<4;i++)
			{
				for (j=0;j<4;j++)
				{
					display_message(INFORMATION_MESSAGE," %g",
						(transformation_matrix)[i][j]);
				}
			}
			display_message(INFORMATION_MESSAGE,";\n");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_cmzn_scene_transformation_commands.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* list_cmzn_scene_transformation_commands */

int list_cmzn_scene_transformation(struct cmzn_scene *scene)
/*******************************************************************************
LAST MODIFIED : 26 April 1999

DESCRIPTION :
Iterator function for writing the transformation in effect for <scene>
in an easy-to-interpret matrix multiplication form.
==============================================================================*/
{
	const char *coordinate_symbol="xyzh";
	int i,return_code;
	gtMatrix transformation_matrix;

	ENTER(list_cmzn_scene_transformation);
	if (scene)
	{
		return_code=cmzn_scene_get_transformation(scene,
			&transformation_matrix);
		if (return_code)
		{
			char *region_name = cmzn_region_get_path(scene->region);
			display_message(INFORMATION_MESSAGE,"%s transformation:\n",
				region_name);
			DEALLOCATE(region_name);
			for (i=0;i<4;i++)
			{
				display_message(INFORMATION_MESSAGE,
					"  |%c.out| = | %13.6e %13.6e %13.6e %13.6e | . |%c.in|\n",
					coordinate_symbol[i],
					transformation_matrix[0][i],transformation_matrix[1][i],
					transformation_matrix[2][i],transformation_matrix[3][i],
					coordinate_symbol[i]);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_cmzn_scene_transformation.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* list_cmzn_scene_transformation */

int cmzn_scene_convert_to_point_cloud(cmzn_scene_id scene,
	cmzn_scenefilter_id filter, cmzn_nodeset_id nodeset,
	cmzn_field_id coordinate_field,
	double line_density, double line_density_scale_factor,
	double surface_density, double surface_density_scale_factor)
{
	cmzn_region_id destination_region = cmzn_nodeset_get_region_internal(nodeset);
	if (scene && nodeset && coordinate_field &&
		(Computed_field_get_region(coordinate_field) == destination_region) &&
		(CMZN_FIELD_VALUE_TYPE_REAL == cmzn_field_get_value_type(coordinate_field)) &&
		(3 >= cmzn_field_get_number_of_components(coordinate_field)))
	{
		int return_code = render_to_finite_elements(scene->region,
			/*graphics_name*/static_cast<const char *>(0), filter,
			RENDER_TO_FINITE_ELEMENTS_SURFACE_NODE_CLOUD, destination_region,
			static_cast<cmzn_field_group_id>(0), coordinate_field, nodeset,
			line_density, line_density_scale_factor,
			surface_density, surface_density_scale_factor);
		return return_code ? CMZN_OK : CMZN_ERROR_GENERAL;
	}
	return CMZN_ERROR_ARGUMENT;
}

cmzn_graphics_id cmzn_scene_create_graphics(cmzn_scene_id scene,
		enum cmzn_graphics_type graphics_type)
{
	cmzn_graphics_id graphics = NULL;
	if (scene)
	{
		if (NULL != (graphics=CREATE(cmzn_graphics)(graphics_type)))
		{
			cmzn_scene_set_minimum_graphics_defaults(scene, graphics);
			cmzn_scene_add_graphics(scene, graphics, -1);
		}
	}
	return graphics;
}

cmzn_graphics_id cmzn_scene_create_graphics_contours(
	cmzn_scene_id scene)
{
	return cmzn_scene_create_graphics(scene, CMZN_GRAPHICS_TYPE_CONTOURS);
}

cmzn_graphics_id cmzn_scene_create_graphics_lines(
	cmzn_scene_id scene)
{
	return cmzn_scene_create_graphics(scene, CMZN_GRAPHICS_TYPE_LINES);
}

cmzn_graphics_id cmzn_scene_create_graphics_points(
	cmzn_scene_id scene)
{
	return cmzn_scene_create_graphics(scene, CMZN_GRAPHICS_TYPE_POINTS);
}

cmzn_graphics_id cmzn_scene_create_graphics_streamlines(
	cmzn_scene_id scene)
{
	return cmzn_scene_create_graphics(scene, CMZN_GRAPHICS_TYPE_STREAMLINES);
}

cmzn_graphics_id cmzn_scene_create_graphics_surfaces(
	cmzn_scene_id scene)
{
	return cmzn_scene_create_graphics(scene, CMZN_GRAPHICS_TYPE_SURFACES);
}

void cmzn_scene::addSelectionnotifier(cmzn_selectionnotifier *selectionnotifier)
{
	if (selectionnotifier)
	{
		if (!this->selectionnotifier_list)
			this->selectionnotifier_list = new cmzn_selectionnotifier_list();
		this->selectionnotifier_list->push_back(selectionnotifier->access());
	}
}

void cmzn_scene::removeSelectionnotifier(cmzn_selectionnotifier *selectionnotifier)
{
	if (selectionnotifier && this->selectionnotifier_list)
	{
		cmzn_selectionnotifier_list::iterator iter = std::find(
			this->selectionnotifier_list->begin(), this->selectionnotifier_list->end(), selectionnotifier);
		if (iter != this->selectionnotifier_list->end())
		{
			cmzn_selectionnotifier::deaccess(selectionnotifier);
			this->selectionnotifier_list->erase(iter);
		}
	}
}

cmzn_selectionnotifier_id cmzn_scene_create_selectionnotifier(cmzn_scene_id scene)
{
	cmzn_selectionnotifier_id selectionnotifier = NULL;
	if (scene)
	{
		selectionnotifier = cmzn_selectionnotifier::create(scene);
	}
	return selectionnotifier;
}

cmzn_graphics_id cmzn_scene_get_first_graphics(cmzn_scene_id scene)
{
	struct cmzn_graphics *graphics = NULL;
	if (scene)
	{
		graphics=FIND_BY_IDENTIFIER_IN_LIST(cmzn_graphics, position)(
			1, scene->list_of_graphics);
		if (graphics)
		{
			ACCESS(cmzn_graphics)(graphics);
		}
	}
	return graphics;
}

cmzn_graphics_id cmzn_scene_get_next_graphics(cmzn_scene_id scene,
	cmzn_graphics_id ref_graphics)
{
	struct cmzn_graphics *graphics = NULL;
	if (scene)
	{
		int ref_pos = cmzn_scene_get_graphics_position(scene, ref_graphics);
		if (ref_pos > 0)
		{
			graphics=FIND_BY_IDENTIFIER_IN_LIST(cmzn_graphics,position)(
				ref_pos+1, scene->list_of_graphics);
			if (graphics)
			{
				ACCESS(cmzn_graphics)(graphics);
			}
		}
	}
	return graphics;
}

cmzn_graphics_id cmzn_scene_get_previous_graphics(cmzn_scene_id scene,
	cmzn_graphics_id ref_graphics)
{
	struct cmzn_graphics *graphics = NULL;
	if (scene)
	{
		int ref_pos = cmzn_scene_get_graphics_position(scene, ref_graphics);
		if (ref_pos > 1)
		{
			graphics=FIND_BY_IDENTIFIER_IN_LIST(cmzn_graphics,position)(
				ref_pos-1, scene->list_of_graphics);
			if (graphics)
			{
				ACCESS(cmzn_graphics)(graphics);
			}
		}
	}
	return graphics;
}

int cmzn_scene_move_graphics_before(cmzn_scene_id scene,
	cmzn_graphics_id graphics, cmzn_graphics_id ref_graphics)
{
	int return_code = CMZN_ERROR_GENERAL;
	if (scene && graphics &&
		(cmzn_graphics_get_scene_private(graphics) == scene) && ((0 == ref_graphics) ||
			(cmzn_graphics_get_scene_private(graphics) ==
				cmzn_graphics_get_scene_private(ref_graphics))))
	{
		cmzn_graphics_id current_graphics = ACCESS(cmzn_graphics)(graphics);
		const int position = cmzn_scene_get_graphics_position(scene, ref_graphics);
		if (CMZN_OK == cmzn_scene_remove_graphics(scene, current_graphics))
		{
			if (cmzn_scene_add_graphics(scene, current_graphics, position))
			{
				return_code = CMZN_OK;
			}
		}
		DEACCESS(cmzn_graphics)(&current_graphics);
	}
	else
	{
		return_code = CMZN_ERROR_ARGUMENT;
	}
	return return_code;
}

int cmzn_scene_remove_all_graphics(cmzn_scene_id scene)
{
	int return_code = CMZN_OK;
	if (scene)
	{
		cmzn_scene_begin_change(scene);
		cmzn_graphics_id graphics = 0;
		while ((0 != (graphics =
			cmzn_scene_get_first_graphics_with_condition(scene,
				(LIST_CONDITIONAL_FUNCTION(cmzn_graphics) *)NULL, (void *)NULL))))
		{
			if (CMZN_OK != cmzn_scene_remove_graphics(scene, graphics))
			{
				return_code = CMZN_ERROR_GENERAL;
				break;
			}
		}
		cmzn_scene_end_change(scene);
	}
	else
	{
		return_code = CMZN_ERROR_ARGUMENT;
	}
	return return_code;
}

cmzn_scene *cmzn_scene_get_child_of_position(cmzn_scene *scene, int position)
{
	cmzn_scene *scene_of_position = NULL;

	if (scene && (position != 0))
	{
		if (position == (cmzn_scene_get_position(scene)))
		{
			scene_of_position = cmzn_scene_access(scene);
		}
		cmzn_region_id child_region = cmzn_region_get_first_child(scene->region);
		while (child_region && (scene_of_position == 0))
		{
			cmzn_scene *child_scene = cmzn_region_get_scene_private(child_region);
			if (child_scene)
			{
				scene_of_position = cmzn_scene_get_child_of_position(child_scene, position);
			}
			cmzn_region_reaccess_next_sibling(&child_region);
		}
		if (child_region)
		{
			cmzn_region_destroy(&child_region);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"cmzn_scene_get_child_of_position.  Invalid argument(s)");
	}

	return scene_of_position;
}

int build_Scene(cmzn_scene_id scene, cmzn_scenefilter_id filter)
{
	if (scene)
	{
		Render_graphics_build_objects renderer;
		return renderer.Scene_compile(scene, filter);
	}

	return 0;
} /* build_Scene */

int cmzn_scene_compile_tree(cmzn_scene *scene,
	Render_graphics_compile_members *renderer)
{
	int return_code = 1;

	if (scene)
	{
		cmzn_region_id child_region = cmzn_region_get_first_child(scene->region);
		while (child_region)
		{
			cmzn_scene_id child_scene = cmzn_region_get_scene_private(child_region);
			if (child_scene)
			{
				cmzn_scene_compile_tree(child_scene, renderer);
			}
			cmzn_region_reaccess_next_sibling(&child_region);
		}
		if (scene->time_notifier)
		{
			renderer->time = cmzn_timenotifier_get_time(scene->time_notifier);
		}
		else
		{
			renderer->time = 0;
		}
		renderer->name_prefix = cmzn_region_get_path(scene->region);
		return_code = renderer->cmzn_scene_compile_members(scene);
		DEALLOCATE(renderer->name_prefix);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_scene_compile.  Invalid argument(s)");
		return_code = 0;
	}

	return (return_code);
}

int Scene_render_opengl(cmzn_scene *scene, Render_graphics_opengl *renderer)
{
	int return_code = 1;

	if (scene && renderer)
	{
		glPushName(0);
		renderer->cmzn_scene_execute(scene);
		glPopName();
	}
	else
	{
		display_message(ERROR_MESSAGE, "Scene_render_opengl.  Invalid argument(s)");
		return_code = 0;
	}

	return (return_code);
} /* Scene_render_opengl */


int cmzn_scene_add_total_transformation_callback(struct cmzn_scene *child_scene,
	cmzn_scene_id scene, CMZN_CALLBACK_FUNCTION(cmzn_scene_transformation) *function,
	CMZN_CALLBACK_FUNCTION(cmzn_scene_top_region_change) *region_change_function,
	void *user_data)
{
	int return_code = 1;
	if (child_scene && scene)
	{
		struct cmzn_region *child_region = cmzn_scene_get_region(child_scene);
		struct cmzn_region *parent = cmzn_region_get_parent_internal(child_region);

		if ((scene != child_scene) || parent)
		{
			struct cmzn_scene *parent_scene = cmzn_region_get_scene_private(parent);
			if (parent_scene)
			{
				return_code = cmzn_scene_add_total_transformation_callback(parent_scene, scene, function,
					region_change_function, user_data);
			}
		}
		if (return_code)
			return_code = CMZN_CALLBACK_LIST_ADD_CALLBACK(cmzn_scene_transformation)(
				child_scene->transformation_callback_list, function, user_data);
		if (scene == child_scene)
			return_code &= CMZN_CALLBACK_LIST_ADD_CALLBACK(cmzn_scene_top_region_change)(
				child_scene->top_region_change_callback_list, region_change_function, user_data);
	}
	else
	{
		return_code = 0;
	}
	return return_code;
}

int cmzn_scene_remove_total_transformation_callback(struct cmzn_scene *child_scene,
	cmzn_scene_id scene, CMZN_CALLBACK_FUNCTION(cmzn_scene_transformation) *function,
	CMZN_CALLBACK_FUNCTION(cmzn_scene_top_region_change) *region_change_function,
	void *user_data)
{
	int return_code = 1;
	if (scene && child_scene)
	{
		struct cmzn_region *child_region = cmzn_scene_get_region(child_scene);
		struct cmzn_region *parent = cmzn_region_get_parent_internal(child_region);

		if ((child_scene != scene) || parent)
		{
			struct cmzn_scene *parent_scene = cmzn_region_get_scene_private(parent);
			if (parent_scene)
			{
				return_code = cmzn_scene_remove_total_transformation_callback(parent_scene, scene, function,
					region_change_function, user_data);
			}
		}
		if (return_code)
			return_code = CMZN_CALLBACK_LIST_REMOVE_CALLBACK(cmzn_scene_transformation)(
				scene->transformation_callback_list, function,user_data);
		if (scene == child_scene)
			return_code &= CMZN_CALLBACK_LIST_REMOVE_CALLBACK(cmzn_scene_top_region_change)(
				scene->top_region_change_callback_list, region_change_function, user_data);
	}
	else
	{
		return_code = 0;
	}
	return return_code;
}

int cmzn_scene_triggers_top_region_change_callback(
	struct cmzn_scene *scene)
{
	if (scene && scene->top_region_change_callback_list)
	{
		return CMZN_CALLBACK_LIST_CALL(cmzn_scene_top_region_change)(
			scene->top_region_change_callback_list, scene,
			NULL);
	}
	return 0;
}

int cmzn_scene_notify_scene_viewer_callback(struct cmzn_scene *scene,
	void *scene_viewer_void)
{
	cmzn_sceneviewer_id scene_viewer = (struct cmzn_sceneviewer *)scene_viewer_void;
	if (scene && scene_viewer)
	{
		return  Scene_viewer_scene_change(scene_viewer);
	}

	return 0;
}

gtMatrix *cmzn_scene_get_total_transformation(
	struct cmzn_scene *scene, struct cmzn_scene *top_scene)
{
	gtMatrix *transformation = NULL;
	int use_local_transformation = 0;

	if (scene && top_scene)
	{
		struct cmzn_region *region = cmzn_scene_get_region(scene);
		struct cmzn_region *parent = cmzn_region_get_parent_internal(region);

		if ((top_scene != scene) || parent)
		{
			struct cmzn_scene *parent_scene = cmzn_region_get_scene_private(parent);
			if (parent_scene)
			{
				gtMatrix *parent_transformation =
					cmzn_scene_get_total_transformation(parent_scene, top_scene);
				transformation = parent_transformation;
				if (transformation)
				{
					if (scene->transformation)
					{
						multiply_gtMatrix(scene->transformation, transformation, transformation);
					}
				}
				else
				{
					/* top level of transformation set by user*/
					use_local_transformation = 1;
				}
			}
		}
		else
		{
			/* top level of transformation */
			use_local_transformation = 1;
		}
		/* allocate a gtMatrix for the top/first found transformation */
		if (use_local_transformation && scene->transformation &&
			(ALLOCATE(transformation, gtMatrix, 1)))
		{
			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					(*transformation)[i][j] = (*scene->transformation)[i][j];
				}
			}
		}
	}
	return transformation;
}

int cmzn_scene_get_global_graphics_range_internal(cmzn_scene_id top_scene,
	cmzn_scene_id scene, cmzn_scenefilter_id filter,
	struct Graphics_object_range_struct *graphics_object_range)
{
	int return_code = 0;
	if (scene && graphics_object_range)
	{
		cmzn_region_id child_region = cmzn_region_get_first_child(scene->region);
		while (child_region)
		{
			cmzn_scene_id child_scene = cmzn_region_get_scene_private(child_region);
			if (child_scene)
			{
				cmzn_scene_get_global_graphics_range_internal(top_scene, child_scene, filter, graphics_object_range);
			}
			cmzn_region_reaccess_next_sibling(&child_region);
		}
		return_code &= cmzn_scene_get_range(scene, top_scene,filter, graphics_object_range);
	}
	return return_code;
}

int cmzn_scene_get_global_graphics_range(cmzn_scene_id top_scene,
	cmzn_scenefilter_id filter,
	double *centre_x, double *centre_y, double *centre_z,
	double *size_x, double *size_y, double *size_z)
{
	double max_x, max_y, max_z, min_x, min_y, min_z;
	int return_code = 1;
	struct Graphics_object_range_struct graphics_object_range;

	if (top_scene && centre_x && centre_y && centre_z && size_x && size_y && size_z)
	{
		/* must first build graphics objects */
		build_Scene(top_scene, filter);
		/* get range of visible graphics_objects in scene */
		graphics_object_range.first = 1;
		graphics_object_range.minimum[0] = 0.0;
		graphics_object_range.minimum[1] = 0.0;
		graphics_object_range.minimum[2] = 0.0;
		graphics_object_range.maximum[0] = 0.0;
		graphics_object_range.maximum[1] = 0.0;
		graphics_object_range.maximum[2] = 0.0;
		cmzn_scene_id scene = top_scene;
		cmzn_scene_get_global_graphics_range_internal(top_scene, scene, filter, &graphics_object_range);
		if (graphics_object_range.first)
		{
			/* nothing in the scene; return zeros */
			*centre_x = *centre_y = *centre_z = 0.0;
			*size_x = *size_y = *size_z =0.0;
		}
		else
		{
			/* get centre and size of smallest cube enclosing visible scene 		struct cmzn_region *top_region = scene->region;*/
			max_x = (double)graphics_object_range.maximum[0];
			max_y = (double)graphics_object_range.maximum[1];
			max_z = (double)graphics_object_range.maximum[2];
			min_x = (double)graphics_object_range.minimum[0];
			min_y = (double)graphics_object_range.minimum[1];
			min_z = (double)graphics_object_range.minimum[2];
			*centre_x = 0.5*(max_x + min_x);
			*centre_y = 0.5*(max_y + min_y);
			*centre_z = 0.5*(max_z + min_z);
			*size_x = max_x - min_x;
			*size_y = max_y - min_y;
			*size_z = max_z - min_z;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_scene_get_global_graphics_range.  Invalid argument(s)");
		return_code = 0;
	}

	return (return_code);
}

struct Scene_graphics_object_iterator_data
{
	const char *graphics_name;
	graphics_object_tree_iterator_function iterator_function;
	void *user_data;
	cmzn_scenefilter_id filter;
};

static int Scene_graphics_objects_in_cmzn_graphics_iterator(
	struct cmzn_graphics *graphics, void *data_void)
{
	int return_code;
	struct GT_object *graphics_object;
	struct Scene_graphics_object_iterator_data *data;

	if (graphics && (data = (struct Scene_graphics_object_iterator_data *)data_void))
	{
		if (!data->graphics_name ||
			cmzn_graphics_has_name(graphics, (void *)data->graphics_name))
		{
			if ((( 0 == data->filter) || cmzn_scenefilter_evaluate_graphics(data->filter, graphics)) &&
				(graphics_object = cmzn_graphics_get_graphics_object(
					 graphics)))
			{
				(data->iterator_function)(graphics_object, 0.0, data->user_data);
			}
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_graphics_objects_in_cmzn_graphics_iterator.  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
}

/* New functions from old scene object */
static int cmzn_region_recursive_for_each_graphics_object(cmzn_region *region,
	Scene_graphics_object_iterator_data *data)
{
	int return_code = 0;
	if (region && data)
	{
		// a bit naughty using this internal API, but Scene doesn't yet have
		// pointer to graphics_module...
		cmzn_scene *scene = cmzn_region_get_scene_private(region);
		if (scene)
		{
			return_code = for_each_graphics_in_cmzn_scene(scene,
				Scene_graphics_objects_in_cmzn_graphics_iterator, (void *)data);
			if (return_code)
			{
				cmzn_region *child_region = cmzn_region_get_first_child(region);
				while (child_region)
				{
					if (!cmzn_region_recursive_for_each_graphics_object(
						child_region, data))
					{
						return_code = 0;
						break;
					}
					cmzn_region_reaccess_next_sibling(&child_region);
				}
				if (child_region)
				{
					cmzn_region_destroy(&child_region);
				}
			}
		}
	}
	return (return_code);
}

int for_each_graphics_object_in_scene_tree(cmzn_scene_id scene,
	cmzn_scenefilter_id filter, graphics_object_tree_iterator_function iterator_function,
	void *user_data)
{
	int return_code = 0;

	if (scene && iterator_function)
	{
		Scene_graphics_object_iterator_data data;
		data.iterator_function = iterator_function;
		data.user_data = user_data;
		data.graphics_name = NULL;
		data.filter = filter;
		return_code =
			cmzn_region_recursive_for_each_graphics_object(scene->region, &data);
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"for_each_graphics_object_in_scene_tree.  Failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"for_each_graphics_object_in_scene_tree.  Invalid argument(s)");
	}

	return (return_code);
}

int Scene_export_region_graphics_object(cmzn_scene *scene,
	cmzn_region *region, const char *graphics_name, cmzn_scenefilter_id filter,
	graphics_object_tree_iterator_function iterator_function, void *user_data)
{
	int return_code = 0;

	if (scene && region && iterator_function && user_data)
	{
		Scene_graphics_object_iterator_data data;
		data.iterator_function = iterator_function;
		data.user_data = user_data;
		data.graphics_name = graphics_name;
		data.filter = filter;
		if (cmzn_region_contains_subregion(scene->region, region))
		{
			struct cmzn_scene *export_scene = cmzn_region_get_scene_private(region);
			if (export_scene)
			{
				return_code = for_each_graphics_in_cmzn_scene(export_scene,
					Scene_graphics_objects_in_cmzn_graphics_iterator, (void *)&data);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_export_region_graphics_object.  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
}

cmzn_scenepicker_id cmzn_scene_create_scenepicker(cmzn_scene_id scene)
{
	if (scene)
	{
		cmzn_scenefiltermodule_id filter_module = cmzn_graphics_module_get_scenefiltermodule(
			scene->graphics_module);
		cmzn_scenepicker_id scenepicker = cmzn_scenepicker_create(filter_module);
		cmzn_scenepicker_set_scene(scenepicker, scene);
		cmzn_scenefiltermodule_destroy(&filter_module);
		return scenepicker;
	}
	return 0;
}
