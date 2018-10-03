/**
 * FILE : graphics_module.cpp
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "opencmiss/zinc/glyph.h"
#include "opencmiss/zinc/material.h"
#include "opencmiss/zinc/scene.h"
#include "opencmiss/zinc/timekeeper.h"
#include "general/debug.h"
#include "general/object.h"
#include "general/mystring.h"
#include "graphics/font.h"
#include "graphics/glyph.hpp"
#include "graphics/graphics_object.h"
#include "graphics/material.h"
#include "graphics/scene.h"
#include "graphics/scene_viewer.h"
#include "graphics/shader.hpp"
#include "graphics/shader_program.hpp"
#include "graphics/shader_uniforms.hpp"
#include "graphics/spectrum.h"
#include "graphics/graphics_module.h"
#include "graphics/scenefilter.hpp"
#include "graphics/tessellation.hpp"
#include "region/cmiss_region_private.h"
#include "region/cmiss_region.h"
#include "general/message.h"
#include <list>

struct cmzn_graphics_module
{
	/* attribute managers and defaults: */
	cmzn_fontmodule_id fontmodule;
	void *font_manager_callback_id;
	struct cmzn_glyphmodule *glyphmodule;
	void *glyph_manager_callback_id;
	cmzn_lightmodule *lightmodule;
	cmzn_materialmodule *materialmodule;
	void *material_manager_callback_id;
	cmzn_scenefiltermodule *scenefiltermodule;
	cmzn_sceneviewermodule_id sceneviewermodule;
	cmzn_spectrummodule_id spectrummodule;
	void *spectrum_manager_callback_id;
	cmzn_tessellationmodule_id tessellationmodule;
	cmzn_shadermodule_id shadermodule;
	void *tessellation_manager_callback_id;
	void *shaderprogram_manager_callback_id;
	void *shaderuniforms_manager_callback_id;
	cmzn_timekeepermodule *timekeepermodule;
	int access_count;
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
	struct MANAGER_MESSAGE(cmzn_material) *message, void *graphics_module_void)
{
	cmzn_graphics_module *graphics_module = reinterpret_cast<cmzn_graphics_module *>(graphics_module_void);
	if (message && graphics_module)
	{
		int change_summary = MANAGER_MESSAGE_GET_CHANGE_SUMMARY(cmzn_material)(message);
		if (change_summary & MANAGER_CHANGE_RESULT(cmzn_material))
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
	struct MANAGER_MESSAGE(cmzn_spectrum) *message, void *graphics_module_void)
{
	cmzn_graphics_module *graphics_module = reinterpret_cast<cmzn_graphics_module *>(graphics_module_void);
	if (message && graphics_module)
	{
		int change_summary = MANAGER_MESSAGE_GET_CHANGE_SUMMARY(cmzn_spectrum)(message);
		if (change_summary & MANAGER_CHANGE_RESULT(cmzn_spectrum))
		{
			// update colour_bar glyphs, if any
			graphics_module->glyphmodule->beginChange();
			cmzn_set_cmzn_glyph *glyphList = graphics_module->glyphmodule->getGlyphListPrivate();
			for (cmzn_set_cmzn_glyph::iterator iter = glyphList->begin(); iter != glyphList->end(); ++iter)
			{
				cmzn_glyph *glyph = *iter;
				glyph->spectrumChange(message);
			}
			graphics_module->glyphmodule->endChange();
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

void cmzn_graphics_module_shaderprogram_manager_callback(
	struct MANAGER_MESSAGE(cmzn_shaderprogram) *message, void *graphics_module_void)
{
	cmzn_graphics_module *graphics_module = reinterpret_cast<cmzn_graphics_module *>(graphics_module_void);
	if (message && graphics_module)
	{
		int change_summary = MANAGER_MESSAGE_GET_CHANGE_SUMMARY(cmzn_shaderprogram)(message);
		if (change_summary & MANAGER_CHANGE_RESULT(cmzn_shaderprogram))
		{
			FOR_EACH_OBJECT_IN_MANAGER(cmzn_material)(
				cmzn_material_shaderprogram_changed, (void *)message,
				cmzn_materialmodule_get_manager(graphics_module->materialmodule));
		}
	}
}

void cmzn_graphics_module_shaderuniforms_manager_callback(
	struct MANAGER_MESSAGE(cmzn_shaderuniforms) *message, void *graphics_module_void)
{
	cmzn_graphics_module *graphics_module = reinterpret_cast<cmzn_graphics_module *>(graphics_module_void);
	if (message && graphics_module)
	{
		int change_summary = MANAGER_MESSAGE_GET_CHANGE_SUMMARY(cmzn_shaderuniforms)(message);
		if (change_summary & MANAGER_CHANGE_RESULT(cmzn_shaderuniforms))
		{
			FOR_EACH_OBJECT_IN_MANAGER(cmzn_material)(
				cmzn_material_shaderuniforms_changed, (void *)message,
				cmzn_materialmodule_get_manager(graphics_module->materialmodule));
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
	cmzn_context *context)
{
	struct cmzn_graphics_module *module;

	ENTER(cmzn_rendtion_graphics_module_create);
	if (context)
	{
		if (ALLOCATE(module, struct cmzn_graphics_module, 1))
		{
			module->lightmodule = cmzn_lightmodule_create();
			module->materialmodule = NULL;
			module->sceneviewermodule = NULL;
			module->spectrummodule=cmzn_spectrummodule_create();
			module->scenefiltermodule=cmzn_scenefiltermodule_create();
			module->shadermodule = cmzn_shadermodule_create();
			module->fontmodule = cmzn_fontmodule_create();
			module->font_manager_callback_id =
				MANAGER_REGISTER(cmzn_font)(cmzn_graphics_module_font_manager_callback,
					(void *)module, cmzn_fontmodule_get_manager(module->fontmodule));
			module->materialmodule = cmzn_materialmodule_create(
					cmzn_spectrummodule_get_manager(module->spectrummodule));
			module->glyphmodule = cmzn_glyphmodule_create(module->materialmodule);
			module->glyph_manager_callback_id =
				MANAGER_REGISTER(cmzn_glyph)(cmzn_graphics_module_glyph_manager_callback,
					(void *)module, cmzn_glyphmodule_get_manager(module->glyphmodule));
			module->material_manager_callback_id =
				MANAGER_REGISTER(cmzn_material)(cmzn_graphics_module_material_manager_callback,
					(void *)module, cmzn_materialmodule_get_manager(module->materialmodule));
			module->spectrum_manager_callback_id =
				MANAGER_REGISTER(cmzn_spectrum)(cmzn_graphics_module_spectrum_manager_callback,
					(void *)module, cmzn_spectrummodule_get_manager(module->spectrummodule));
			module->timekeepermodule = cmzn_context_get_timekeepermodule(context);
			module->tessellationmodule = cmzn_tessellationmodule_create();
			module->member_regions_list = new std::list<cmzn_region*>;
			module->tessellation_manager_callback_id =
				MANAGER_REGISTER(cmzn_tessellation)(cmzn_graphics_module_tessellation_manager_callback,
					(void *)module, cmzn_tessellationmodule_get_manager(module->tessellationmodule));
			module->shaderprogram_manager_callback_id =
				MANAGER_REGISTER(cmzn_shaderprogram)(cmzn_graphics_module_shaderprogram_manager_callback,
					(void *)module, cmzn_shadermodule_get_program_manager(module->shadermodule));
			module->shaderuniforms_manager_callback_id =
				MANAGER_REGISTER(cmzn_shaderuniforms)(cmzn_graphics_module_shaderuniforms_manager_callback,
					(void *)module, cmzn_shadermodule_get_uniforms_manager(module->shadermodule));
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

struct cmzn_materialmodule *cmzn_graphics_module_get_materialmodule(
	struct cmzn_graphics_module *graphics_module)
{
	if (graphics_module && graphics_module->materialmodule)
	{
		return cmzn_materialmodule_access(graphics_module->materialmodule);
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
				cmzn_glyphmodule_get_manager(graphics_module->glyphmodule));
			MANAGER_DEREGISTER(cmzn_material)(
				graphics_module->material_manager_callback_id,
				cmzn_materialmodule_get_manager(graphics_module->materialmodule));
			MANAGER_DEREGISTER(cmzn_shaderprogram)(
				graphics_module->shaderprogram_manager_callback_id,
				cmzn_shadermodule_get_program_manager(graphics_module->shadermodule));
			MANAGER_DEREGISTER(cmzn_shaderuniforms)(
				graphics_module->shaderuniforms_manager_callback_id,
				cmzn_shadermodule_get_uniforms_manager(graphics_module->shadermodule));
			MANAGER_DEREGISTER(cmzn_spectrum)(
				graphics_module->spectrum_manager_callback_id, cmzn_spectrummodule_get_manager(graphics_module->spectrummodule));
			MANAGER_DEREGISTER(cmzn_tessellation)(
				graphics_module->tessellation_manager_callback_id,
				cmzn_tessellationmodule_get_manager(graphics_module->tessellationmodule));
			MANAGER_DEREGISTER(cmzn_font)(
				graphics_module->font_manager_callback_id,
				cmzn_fontmodule_get_manager(graphics_module->fontmodule));
			/* This will remove all callbacks used by the scene_viewer projection_field callback */
			cmzn_glyphmodule_destroy(&graphics_module->glyphmodule);
			if (graphics_module->lightmodule)
				cmzn_lightmodule_destroy(&graphics_module->lightmodule);
			if (graphics_module->shadermodule)
				cmzn_shadermodule_destroy(&graphics_module->shadermodule);
			if (graphics_module->spectrummodule)
				cmzn_spectrummodule_destroy(&graphics_module->spectrummodule);
			if (graphics_module->fontmodule)
				cmzn_fontmodule_destroy(&graphics_module->fontmodule);
			if (graphics_module->materialmodule)
				cmzn_materialmodule_destroy(&graphics_module->materialmodule);
			if (graphics_module->scenefiltermodule)
				cmzn_scenefiltermodule_destroy(&graphics_module->scenefiltermodule);
			cmzn_timekeepermodule_destroy(&graphics_module->timekeepermodule);
			if (graphics_module->tessellationmodule)
				cmzn_tessellationmodule_destroy(&graphics_module->tessellationmodule);
			cmzn_sceneviewermodule_destroy(&graphics_module->sceneviewermodule);
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
				return_code = 1;
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

struct MANAGER(cmzn_spectrum) *cmzn_graphics_module_get_spectrum_manager(
	struct cmzn_graphics_module *graphics_module)
{
	if (graphics_module && graphics_module->spectrummodule)
	{
		return cmzn_spectrummodule_get_manager(graphics_module->spectrummodule);
	}

	return NULL;
}

cmzn_spectrummodule_id cmzn_graphics_module_get_spectrummodule(
	struct cmzn_graphics_module *graphics_module)
{
	if (graphics_module && graphics_module->spectrummodule)
	{
		return cmzn_spectrummodule_access(graphics_module->spectrummodule);
	}
	return 0;
}

struct cmzn_font *cmzn_graphics_module_get_default_font(
	struct cmzn_graphics_module *graphics_module)
{
	if (graphics_module && graphics_module->fontmodule)
	{
		return cmzn_fontmodule_get_default_font(graphics_module->fontmodule);
	}
	return 0;
}

cmzn_fontmodule_id cmzn_graphics_module_get_fontmodule(
	struct cmzn_graphics_module *graphics_module)
{
	if (graphics_module && graphics_module->fontmodule)
	{
		return cmzn_fontmodule_access(graphics_module->fontmodule);
	}
	return 0;
}

struct MANAGER(cmzn_font) *cmzn_graphics_module_get_font_manager(
	struct cmzn_graphics_module *graphics_module)
{
	if (graphics_module && graphics_module->fontmodule)
	{
		return cmzn_fontmodule_get_manager(graphics_module->fontmodule);
	}
	return 0;
}

cmzn_glyphmodule_id cmzn_graphics_module_get_glyphmodule(
	cmzn_graphics_module * graphics_module)
{
	if (graphics_module)
	{
		cmzn_glyphmodule_access(graphics_module->glyphmodule);
		return graphics_module->glyphmodule;
	}
	return 0;
}

cmzn_sceneviewermodule_id cmzn_graphics_module_get_sceneviewermodule(
	cmzn_graphics_module * graphics_module)
{
	cmzn_sceneviewermodule *sceneviewermodule = NULL;
	if (graphics_module)
	{
		if (!graphics_module->sceneviewermodule)
		{
			cmzn_light *default_light =
				cmzn_lightmodule_get_default_light(graphics_module->lightmodule);
			cmzn_light *default_ambient_light =
				cmzn_lightmodule_get_default_ambient_light(graphics_module->lightmodule);
			Colour default_background_colour;
			default_background_colour.red = 0.0;
			default_background_colour.green = 0.0;
			default_background_colour.blue = 0.0;
			default_background_colour.alpha = 1.0;
			cmzn_scenefiltermodule_id filterModule = cmzn_graphics_module_get_scenefiltermodule(graphics_module);
			graphics_module->sceneviewermodule = cmzn_sceneviewermodule::create(
				&default_background_colour,
				graphics_module->lightmodule, default_light,
				default_ambient_light,
				filterModule);
			cmzn_scenefiltermodule_destroy(&filterModule);
			cmzn_light_destroy(&default_ambient_light);
			cmzn_light_destroy(&default_light);
		}
		sceneviewermodule = cmzn_sceneviewermodule_access(graphics_module->sceneviewermodule);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_graphics_module_get_sceneviewermodule.  "
			"Missing context");
	}
	return sceneviewermodule;
}

cmzn_shadermodule_id cmzn_graphics_module_get_shadermodule(
	struct cmzn_graphics_module *graphics_module)
{
	if (graphics_module && graphics_module->shadermodule)
	{
		return cmzn_shadermodule_access(graphics_module->shadermodule);
	}
	return 0;
}

cmzn_tessellationmodule_id cmzn_graphics_module_get_tessellationmodule(
	struct cmzn_graphics_module *graphics_module)
{
	if (graphics_module && graphics_module->tessellationmodule)
	{
		return cmzn_tessellationmodule_access(graphics_module->tessellationmodule);
	}
	return 0;
}

cmzn_timekeepermodule *cmzn_graphics_module_get_timekeepermodule(
	struct cmzn_graphics_module *graphics_module)
{
	if (graphics_module && graphics_module->timekeepermodule)
		return graphics_module->timekeepermodule->access();
	return 0;
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
	cmzn_graphics_module * graphics_module, cmzn_region_id region)
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

cmzn_scenefiltermodule_id cmzn_graphics_module_get_scenefiltermodule(
	struct cmzn_graphics_module *graphics_module)
{
	if (graphics_module && graphics_module->scenefiltermodule)
	{
		return cmzn_scenefiltermodule_access(graphics_module->scenefiltermodule);
	}
	return 0;
}

struct MANAGER(cmzn_scenefilter) *cmzn_graphics_module_get_filter_manager(
		struct cmzn_graphics_module *graphics_module)
{
	if (graphics_module && graphics_module->scenefiltermodule)
	{
		return cmzn_scenefiltermodule_get_manager(graphics_module->scenefiltermodule);
	}
	return 0;
}

cmzn_lightmodule *cmzn_graphics_module_get_lightmodule(
	struct cmzn_graphics_module *graphics_module)
{
	if (graphics_module && graphics_module->lightmodule)
	{
		return cmzn_lightmodule_access(graphics_module->lightmodule);
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
			scene->detachFromOwner();
		}
	}
}
