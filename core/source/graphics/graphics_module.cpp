/**
 * FILE : graphics_module.cpp
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "zinc/glyph.h"
#include "zinc/graphicsmaterial.h"
#include "zinc/graphicsmodule.h"
#include "zinc/scene.h"
#include "general/debug.h"
#include "general/object.h"
#include "general/mystring.h"
#include "graphics/font.h"
#include "graphics/glyph.hpp"
#include "graphics/graphics_object.h"
#include "graphics/material.h"
#include "graphics/scene.h"
#include "graphics/scene_viewer.h"
#include "graphics/spectrum.h"
#include "graphics/graphics_module.h"
#include "graphics/light_model.h"
#include "graphics/graphics_filter.hpp"
#include "graphics/tessellation.hpp"
#include "region/cmiss_region_private.h"
#include "region/cmiss_region.h"
#include "time/time_keeper.hpp"
#include "general/message.h"
#include <list>

struct cmzn_graphics_module
{
	/* attribute managers and defaults: */
	struct cmzn_glyph_module *glyph_module;
	void *glyph_manager_callback_id;
	struct cmzn_graphics_material_module *material_module;
	void *material_manager_callback_id;
	Light_module *light_module;
	cmzn_spectrum_module_id spectrum_module;
	void *spectrum_manager_callback_id;
	cmzn_font_module_id font_module;
	void *font_manager_callback_id;
	cmzn_scene_viewer_module_id scene_viewer_module;
	Light_model_module *light_model_module;
	struct cmzn_time_keeper *default_time_keeper;
	cmzn_tessellation_module_id tessellation_module;
	struct cmzn_graphics_filter_module *graphics_filter_module;
	void *tessellation_manager_callback_id;
	int access_count;
	cmzn_region_id root_region;
	std::list<cmzn_region*> *member_regions_list;
};

namespace {

/**
 * Callback for changes in the glyph manager.
 * Informs all renditions about the changes.
 */
void cmzn_graphics_module_glyph_manager_callback(
	struct MANAGER_MESSAGE(cmzn_glyph) *message, void *graphics_module_void)
{
	cmzn_graphics_module *graphics_module = reinterpret_cast<cmzn_graphics_module *>(graphics_module_void);
	if (message && graphics_module)
	{
		int change_summary = MANAGER_MESSAGE_GET_CHANGE_SUMMARY(cmzn_glyph)(message);
		if (change_summary & MANAGER_CHANGE_RESULT(cmzn_glyph))
		{
			std::list<cmzn_region*>::iterator region_iter;
			for (region_iter = graphics_module->member_regions_list->begin();
				region_iter != graphics_module->member_regions_list->end(); ++region_iter)
			{
				cmzn_region *region = *region_iter;
				if (cmzn_region_is_root(region))
				{
					cmzn_scene *scene = cmzn_region_get_scene_private(region);
					cmzn_scene_glyph_change(scene, message);
				}
			}
		}
	}
}

/**
 * Callback for changes in the material manager.
 * Informs all scenes about the changes.
 */
void cmzn_graphics_module_material_manager_callback(
	struct MANAGER_MESSAGE(Graphical_material) *message, void *graphics_module_void)
{
	cmzn_graphics_module *graphics_module = reinterpret_cast<cmzn_graphics_module *>(graphics_module_void);
	if (message && graphics_module)
	{
		int change_summary = MANAGER_MESSAGE_GET_CHANGE_SUMMARY(Graphical_material)(message);
		if (change_summary & MANAGER_CHANGE_RESULT(Graphical_material))
		{
			std::list<cmzn_region*>::iterator region_iter;
			for (region_iter = graphics_module->member_regions_list->begin();
				region_iter != graphics_module->member_regions_list->end(); ++region_iter)
			{
				cmzn_region *region = *region_iter;
				if (cmzn_region_is_root(region))
				{
					cmzn_scene *scene = cmzn_region_get_scene_private(region);
					cmzn_scene_material_change(scene, message);
				}
			}
		}
	}
}

/**
 * Callback for changes in the spectrum manager.
 * Informs all scenes about the changes.
 */
void cmzn_graphics_module_spectrum_manager_callback(
	struct MANAGER_MESSAGE(Spectrum) *message, void *graphics_module_void)
{
	cmzn_graphics_module *graphics_module = reinterpret_cast<cmzn_graphics_module *>(graphics_module_void);
	if (message && graphics_module)
	{
		int change_summary = MANAGER_MESSAGE_GET_CHANGE_SUMMARY(Spectrum)(message);
		if (change_summary & MANAGER_CHANGE_RESULT(Spectrum))
		{
			// update colour_bar glyphs, if any
			graphics_module->glyph_module->beginChange();
			cmzn_set_cmzn_glyph *glyphList = graphics_module->glyph_module->getGlyphListPrivate();
			for (cmzn_set_cmzn_glyph::iterator iter = glyphList->begin(); iter != glyphList->end(); ++iter)
			{
				cmzn_glyph *glyph = *iter;
				glyph->spectrumChange(message);
			}
			graphics_module->glyph_module->endChange();
			std::list<cmzn_region*>::iterator region_iter;
			for (region_iter = graphics_module->member_regions_list->begin();
				region_iter != graphics_module->member_regions_list->end(); ++region_iter)
			{
				cmzn_region *region = *region_iter;
				if (cmzn_region_is_root(region))
				{
					cmzn_scene *scene = cmzn_region_get_scene_private(region);
					cmzn_scene_spectrum_change(scene, message);
				}
			}
		}
	}
}

/**
 * Callback for changes in the tessellation manager.
 * Informs all scenes about the changes.
 */
void cmzn_graphics_module_tessellation_manager_callback(
	struct MANAGER_MESSAGE(cmzn_tessellation) *message, void *graphics_module_void)
{
	cmzn_graphics_module *graphics_module = reinterpret_cast<cmzn_graphics_module *>(graphics_module_void);
	if (message && graphics_module)
	{
		int change_summary = MANAGER_MESSAGE_GET_CHANGE_SUMMARY(cmzn_tessellation)(message);
		if (change_summary & MANAGER_CHANGE_RESULT(cmzn_tessellation))
		{
			std::list<cmzn_region*>::iterator region_iter;
			for (region_iter = graphics_module->member_regions_list->begin();
				region_iter != graphics_module->member_regions_list->end(); ++region_iter)
			{
				cmzn_region *region = *region_iter;
				if (cmzn_region_is_root(region))
				{
					cmzn_scene *scene = cmzn_region_get_scene_private(region);
					cmzn_scene_tessellation_change(scene, message);
				}
			}
		}
	}
}

/**
 * Callback for changes in the font manager.
 * Informs all renditions about the changes.
 */
void cmzn_graphics_module_font_manager_callback(
	struct MANAGER_MESSAGE(cmzn_font) *message, void *graphics_module_void)
{
	cmzn_graphics_module *graphics_module = reinterpret_cast<cmzn_graphics_module *>(graphics_module_void);
	if (message && graphics_module)
	{
		int change_summary = MANAGER_MESSAGE_GET_CHANGE_SUMMARY(cmzn_font)(message);
		if (change_summary & MANAGER_CHANGE_RESULT(cmzn_font))
		{
			std::list<cmzn_region*>::iterator region_iter;
			for (region_iter = graphics_module->member_regions_list->begin();
				region_iter != graphics_module->member_regions_list->end(); ++region_iter)
			{
				cmzn_region *region = *region_iter;
				if (cmzn_region_is_root(region))
				{
					cmzn_scene *scene = cmzn_region_get_scene_private(region);
					cmzn_scene_font_change(scene, message);
				}
			}
		}
	}
}

}

struct cmzn_graphics_module *cmzn_graphics_module_create(
	struct Context *context)
{
	struct cmzn_graphics_module *module;

	ENTER(cmzn_rendtion_graphics_module_create);
	if (context)
	{
		if (ALLOCATE(module, struct cmzn_graphics_module, 1))
		{
			module->light_module = Light_module_create();
			module->light_model_module = Light_model_module_create();
			module->material_module = NULL;
			module->scene_viewer_module = NULL;
			module->spectrum_module=cmzn_spectrum_module_create();
			module->graphics_filter_module=cmzn_graphics_filter_module_create();
			module->font_module = cmzn_font_module_create();
			module->font_manager_callback_id =
				MANAGER_REGISTER(cmzn_font)(cmzn_graphics_module_font_manager_callback,
					(void *)module, cmzn_font_module_get_manager(module->font_module));
			module->root_region = cmzn_context_get_default_region(context);
			module->material_module = cmzn_graphics_material_module_create(
					cmzn_spectrum_module_get_manager(module->spectrum_module));
			module->glyph_module = cmzn_glyph_module_create(module->material_module);
			module->glyph_manager_callback_id =
				MANAGER_REGISTER(cmzn_glyph)(cmzn_graphics_module_glyph_manager_callback,
					(void *)module, cmzn_glyph_module_get_manager(module->glyph_module));
			module->material_manager_callback_id =
				MANAGER_REGISTER(Graphical_material)(cmzn_graphics_module_material_manager_callback,
					(void *)module, cmzn_graphics_material_module_get_manager(module->material_module));
			module->spectrum_manager_callback_id =
				MANAGER_REGISTER(Spectrum)(cmzn_graphics_module_spectrum_manager_callback,
					(void *)module, cmzn_spectrum_module_get_manager(module->spectrum_module));
			module->default_time_keeper = cmzn_context_get_default_time_keeper(context);
			module->tessellation_module = cmzn_tessellation_module_create();
			module->member_regions_list = new std::list<cmzn_region*>;
			module->tessellation_manager_callback_id =
				MANAGER_REGISTER(cmzn_tessellation)(cmzn_graphics_module_tessellation_manager_callback,
					(void *)module, cmzn_tessellation_module_get_manager(module->tessellation_module));
			module->access_count = 1;
		}
		else
		{
			module = (cmzn_graphics_module *)NULL;
			display_message(ERROR_MESSAGE,
			"cmzn_rendtion_graphics_module_create. Not enough memory for cmzn scene graphics module");
		}
	}
	else
	{
		module = (cmzn_graphics_module *)NULL;
		display_message(ERROR_MESSAGE,"cmzn_rendtion_graphics_module_create.  Invalid argument(s)");
	}
	LEAVE;

	return (module);
}

struct cmzn_graphics_material_module *cmzn_graphics_module_get_material_module(
	struct cmzn_graphics_module *graphics_module)
{
	if (graphics_module && graphics_module->material_module)
	{
		return cmzn_graphics_material_module_access(graphics_module->material_module);
	}

	return 0;
}

struct cmzn_graphics_module *cmzn_graphics_module_access(
	struct cmzn_graphics_module *graphics_module)
{
	if (graphics_module)
	{
		graphics_module->access_count++;
	}
	return graphics_module;
}

int cmzn_graphics_module_remove_member_regions_scene(
	struct cmzn_graphics_module *graphics_module)
{
	int return_code = 0;

	if (graphics_module && graphics_module->member_regions_list)
	{
		std::list<cmzn_region*>::iterator pos;
		for (pos = graphics_module->member_regions_list->begin();
				pos != graphics_module->member_regions_list->end(); ++pos)
		{
			// clean up scene between begin/end change so fields and other objects
			// destroyed when scene destroyed do not cause messages to be sent
			cmzn_region_begin_change(*pos);
			cmzn_region_deaccess_scene(*pos);
			cmzn_region_end_change(*pos);
		}
		graphics_module->member_regions_list->clear();
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_graphics_module_remove_member_regions_scene.  "
			"Invalid argument(s)");
	}

	return return_code;
}

int cmzn_graphics_module_destroy(
	struct cmzn_graphics_module **graphics_module_address)
{
	int return_code = 0;
	struct cmzn_graphics_module *graphics_module = NULL;

	if (NULL != (graphics_module = *graphics_module_address))
	{
		graphics_module->access_count--;
		if (0 == graphics_module->access_count)
		{
			MANAGER_DEREGISTER(cmzn_glyph)(
				graphics_module->glyph_manager_callback_id,
				cmzn_glyph_module_get_manager(graphics_module->glyph_module));
			if (graphics_module->root_region)
				cmzn_region_destroy(&graphics_module->root_region);
			MANAGER_DEREGISTER(Graphical_material)(
				graphics_module->material_manager_callback_id,
				cmzn_graphics_material_module_get_manager(graphics_module->material_module));
			MANAGER_DEREGISTER(Spectrum)(
				graphics_module->spectrum_manager_callback_id, cmzn_spectrum_module_get_manager(graphics_module->spectrum_module));
			MANAGER_DEREGISTER(cmzn_tessellation)(
				graphics_module->tessellation_manager_callback_id,
				cmzn_tessellation_module_get_manager(graphics_module->tessellation_module));
			MANAGER_DEREGISTER(cmzn_font)(
				graphics_module->font_manager_callback_id,
				cmzn_font_module_get_manager(graphics_module->font_module));
			/* This will remove all callbacks used by the scene_viewer projection_field callback */
			cmzn_glyph_module_destroy(&graphics_module->glyph_module);
			if (graphics_module->light_module)
				Light_module_destroy(&graphics_module->light_module);
			if (graphics_module->light_model_module)
				Light_model_module_destroy(&graphics_module->light_model_module);
			if (graphics_module->spectrum_module)
				cmzn_spectrum_module_destroy(&graphics_module->spectrum_module);
			if (graphics_module->font_module)
				cmzn_font_module_destroy(&graphics_module->font_module);
			if (graphics_module->material_module)
				cmzn_graphics_material_module_destroy(&graphics_module->material_module);
			if (graphics_module->graphics_filter_module)
				cmzn_graphics_filter_module_destroy(&graphics_module->graphics_filter_module);
			if (graphics_module->default_time_keeper)
				cmzn_time_keeper_destroy(&graphics_module->default_time_keeper);
			if (graphics_module->tessellation_module)
				cmzn_tessellation_module_destroy(&graphics_module->tessellation_module);
			cmzn_scene_viewer_module_destroy(&graphics_module->scene_viewer_module);
			if (graphics_module->member_regions_list)
			{
				cmzn_graphics_module_remove_member_regions_scene(graphics_module);
				delete graphics_module->member_regions_list;
			}
			DEALLOCATE(*graphics_module_address);
		}
		*graphics_module_address = NULL;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_graphics_module_destroy.  Missing graphics module");
		return_code = 0;
	}

	return return_code;
}

int cmzn_graphics_module_create_scene(
	struct cmzn_graphics_module *graphics_module,
	struct cmzn_region *cmiss_region)
{
	struct cmzn_scene *scene;
	int return_code;

	ENTER(cmzn_region_add_scene);
	if (cmiss_region && graphics_module)
	{
		scene = FIRST_OBJECT_IN_LIST_THAT(ANY_OBJECT(cmzn_scene))(
			(ANY_OBJECT_CONDITIONAL_FUNCTION(cmzn_scene) *)NULL, (void *)NULL,
			cmzn_region_private_get_any_object_list(cmiss_region));
		if (!(scene))
		{
			if (NULL != (scene = cmzn_scene_create_internal(cmiss_region, graphics_module)))
			{
				cmzn_scene_set_position(scene, 1);
				return_code = 1;
			}
			else
			{
				return_code = 0;
				display_message(ERROR_MESSAGE,
					"cmzn_region_add_scene. Cannot create scene for region");
			}
		}
		else
		{
			return_code = 1;
			//ACCESS or not ?
			//ACCESS(cmzn_scene)(scene);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_region_add_scene. Invalid argument(s).");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

struct MANAGER(Spectrum) *cmzn_graphics_module_get_spectrum_manager(
	struct cmzn_graphics_module *graphics_module)
{
	if (graphics_module && graphics_module->spectrum_module)
	{
		return cmzn_spectrum_module_get_manager(graphics_module->spectrum_module);
	}

	return NULL;
}

cmzn_spectrum_module_id cmzn_graphics_module_get_spectrum_module(
	struct cmzn_graphics_module *graphics_module)
{
	if (graphics_module && graphics_module->spectrum_module)
	{
		return cmzn_spectrum_module_access(graphics_module->spectrum_module);
	}
	return 0;
}

struct cmzn_font *cmzn_graphics_module_get_default_font(
	struct cmzn_graphics_module *graphics_module)
{
	if (graphics_module && graphics_module->font_module)
	{
		return cmzn_font_module_get_default_font(graphics_module->font_module);
	}
	return 0;
}

cmzn_font_module_id cmzn_graphics_module_get_font_module(
	struct cmzn_graphics_module *graphics_module)
{
	if (graphics_module && graphics_module->font_module)
	{
		return cmzn_font_module_access(graphics_module->font_module);
	}
	return 0;
}

struct MANAGER(cmzn_font) *cmzn_graphics_module_get_font_manager(
	struct cmzn_graphics_module *graphics_module)
{
	if (graphics_module && graphics_module->font_module)
	{
		return cmzn_font_module_get_manager(graphics_module->font_module);
	}
	return 0;
}

cmzn_glyph_module_id cmzn_graphics_module_get_glyph_module(
	cmzn_graphics_module_id graphics_module)
{
	if (graphics_module)
	{
		cmzn_glyph_module_access(graphics_module->glyph_module);
		return graphics_module->glyph_module;
	}
	return 0;
}

cmzn_scene_viewer_module_id cmzn_graphics_module_get_scene_viewer_module(
	cmzn_graphics_module_id graphics_module)
{
	cmzn_scene_viewer_module *scene_viewer_module = NULL;
	if (graphics_module)
	{
		if (!graphics_module->scene_viewer_module)
		{
			Light *default_light =
				Light_module_get_default_light(graphics_module->light_module);
			Light_model *default_light_model =
				Light_model_module_get_default_light_model(graphics_module->light_model_module);
			Colour default_background_colour;
			default_background_colour.red = 0.0;
			default_background_colour.green = 0.0;
			default_background_colour.blue = 0.0;
			cmzn_graphics_filter_module_id filterModule = cmzn_graphics_module_get_filter_module(graphics_module);
			graphics_module->scene_viewer_module = CREATE(cmzn_scene_viewer_module)(
				&default_background_colour,
				/* interactive_tool_manager */0,
				graphics_module->light_module, default_light,
				graphics_module->light_model_module, default_light_model,
				filterModule);
			cmzn_graphics_filter_module_destroy(&filterModule);
			DEACCESS(Light_model)(&default_light_model);
			DEACCESS(Light)(&default_light);
		}
		scene_viewer_module = cmzn_scene_viewer_module_access(graphics_module->scene_viewer_module);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_graphics_module_get_scene_viewer_module.  "
			"Missing context");
	}
	return scene_viewer_module;
}

cmzn_tessellation_module_id cmzn_graphics_module_get_tessellation_module(
	struct cmzn_graphics_module *graphics_module)
{
	if (graphics_module && graphics_module->tessellation_module)
	{
		return cmzn_tessellation_module_access(graphics_module->tessellation_module);
	}
	return 0;
}

struct cmzn_time_keeper *cmzn_graphics_module_get_time_keeper_internal(
	struct cmzn_graphics_module *module)
{
	struct cmzn_time_keeper *time_keeper = NULL;

	if (module && module->default_time_keeper)
	{
		time_keeper = cmzn_time_keeper_access(module->default_time_keeper);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_graphics_module_get_time_keeper_internal.  Invalid argument(s)");
	}

	return time_keeper;
}

int cmzn_graphics_module_enable_scenes(
	struct cmzn_graphics_module *graphics_module,
	struct cmzn_region *cmiss_region)
{
	int return_code;
	struct cmzn_region *child_region;

	if (cmiss_region && graphics_module)
	{
		return_code = cmzn_graphics_module_create_scene(
			graphics_module, cmiss_region);
		if (return_code)
		{
			child_region = cmzn_region_get_first_child(cmiss_region);
			while (child_region)
			{
				return_code = cmzn_graphics_module_enable_scenes(
					graphics_module, child_region);
				cmzn_region_reaccess_next_sibling(&child_region);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"cmzn_scene.  "
			"Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
}

cmzn_scene_id cmzn_graphics_module_get_scene(
	cmzn_graphics_module_id graphics_module, cmzn_region_id region)
{
	struct cmzn_scene *scene = NULL;
	if (graphics_module && region)
	{
		scene = cmzn_region_get_scene_private(region);
		if (!scene)
		{
			cmzn_region_id top_region = cmzn_region_get_root(region);
			cmzn_graphics_module_enable_scenes(graphics_module, top_region);
			scene = cmzn_region_get_scene_private(region);
			cmzn_region_destroy(&top_region);
		}
	}
	return cmzn_scene_access(scene);
}

int cmzn_graphics_module_add_member_region(
	struct cmzn_graphics_module *graphics_module, struct cmzn_region *region)
{
	int return_code = 0;

	if (graphics_module && region)
	{
		if (graphics_module->member_regions_list)
		{
			graphics_module->member_regions_list->push_back(region);
			return_code = 1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_graphics_module_add_member_region.  Invalid argument(s)");
	}

	return return_code;
}

int cmzn_graphics_module_remove_member_region(
		struct cmzn_graphics_module *graphics_module, struct cmzn_region *region)
{
	int return_code;

	ENTER(cmzn_graphics_module_remove_member_region);
	if (graphics_module && graphics_module->member_regions_list && region)
	{
		graphics_module->member_regions_list->remove(region);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_graphics_module_remove_member_region.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

cmzn_graphics_filter_module_id cmzn_graphics_module_get_filter_module(
	struct cmzn_graphics_module *graphics_module)
{
	if (graphics_module && graphics_module->graphics_filter_module)
	{
		return cmzn_graphics_filter_module_access(graphics_module->graphics_filter_module);
	}
	return 0;
}

struct MANAGER(cmzn_graphics_filter) *cmzn_graphics_module_get_filter_manager(
		struct cmzn_graphics_module *graphics_module)
{
	if (graphics_module && graphics_module->graphics_filter_module)
	{
		return cmzn_graphics_filter_module_get_manager(graphics_module->graphics_filter_module);
	}
	return 0;
}

Light_module *cmzn_graphics_module_get_light_module(
	struct cmzn_graphics_module *graphics_module)
{
	if (graphics_module && graphics_module->light_module)
	{
		return Light_module_access(graphics_module->light_module);
	}

	return 0;
}

Light_model_module *cmzn_graphics_module_get_light_model_module(
	struct cmzn_graphics_module *graphics_module)
{
	if (graphics_module && graphics_module->light_model_module)
	{
		return Light_model_module_access(graphics_module->light_model_module);
	}

	return 0;
}

void cmzn_graphics_module_remove_external_callback_dependency(
	struct cmzn_graphics_module *graphics_module)
{
	if (graphics_module)
	{
		std::list<cmzn_region*>::iterator region_iter;
		for (region_iter = graphics_module->member_regions_list->begin();
			region_iter != graphics_module->member_regions_list->end(); ++region_iter)
		{
			cmzn_region *region = *region_iter;
			cmzn_scene *scene = cmzn_region_get_scene_private(region);
			cmzn_scene_detach_from_owner(scene);
		}
	}
}
