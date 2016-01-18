/***************************************************************************//**
 * context.cpp
 *
 * The main root structure of cmgui.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <algorithm>
#include <cstdlib>
#include "zinc/fieldgroup.h"
#include "configure/version.h"
#include "context/context.h"
#include "curve/curve.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "general/object.h"
#include "graphics/scene_viewer.h"
#include "graphics/graphics_module.h"
#include "graphics/scene.h"
#include "region/cmiss_region.h"
#include "zinc/timekeeper.h"
//-- #include "user_interface/event_dispatcher.h"
/* following is temporary until circular references are removed for cmzn_region  */
#include "region/cmiss_region_private.h"

cmzn_context::cmzn_context(const char *idIn) :
	id(duplicate_string(idIn)),
	logger(cmzn_logger::create()),
	root_region(0),
	element_point_ranges_selection(0),
	io_stream_package(0),
	timekeepermodule(cmzn_timekeepermodule::create()),
	curve_manager(0),
	graphics_module(0),
	access_count(1)
{
}

cmzn_context::~cmzn_context()
{
	if (this->id)
		DEALLOCATE(this->id);
	if (this->graphics_module)
	{
		cmzn_graphics_module_remove_external_callback_dependency(
			this->graphics_module);
		cmzn_graphics_module_destroy(&this->graphics_module);
	}
	if (this->root_region)
	{
		/* need the following due to circular references where field owned by region references region itself;
			* when following removed also remove #include "region/cmiss_region_private.h". Also scene
			* has a computed_field manager callback so it must be deleted before detaching fields hierarchical */
		cmzn_region_detach_fields_hierarchical(this->root_region);
		DEACCESS(cmzn_region)(&this->root_region);
	}
	// clear regions' pointers to this context
	for (std::list<cmzn_region*>::iterator iter = this->allRegions.begin(); iter != this->allRegions.end(); ++iter)
		cmzn_region_set_context_private(*iter, 0);
	if (this->element_point_ranges_selection)
		DESTROY(Element_point_ranges_selection)(&this->element_point_ranges_selection);
	if (this->curve_manager)
		DESTROY(MANAGER(Curve))(&this->curve_manager);
	if (this->io_stream_package)
		DESTROY(IO_stream_package)(&this->io_stream_package);
	cmzn_timekeepermodule::deaccess(this->timekeepermodule);

	/* Write out any memory blocks still ALLOCATED when MEMORY_CHECKING is
		on.  When MEMORY_CHECKING is off this function does nothing */
	list_memory(/*count_number*/0, /*show_pointers*/0, /*increment_counter*/0,
		/*show_structures*/1);
}
	
cmzn_context *cmzn_context::create(const char *id)
{
	cmzn_context *context = new cmzn_context(id);
	if (context && context->id && context->timekeepermodule)
		return context;
	delete context;
	return 0;
}

cmzn_region *cmzn_context::createRegion()
{
	// all regions share element shapes and bases
	cmzn_region *baseRegion = (this->allRegions.size() > 0) ? this->allRegions.front() : 0;
	cmzn_region *region = cmzn_region_create_internal(baseRegion);
	if (region)
	{
		this->allRegions.push_back(region);
		cmzn_region_set_context_private(region, this);
		cmzn_graphics_module_enable_scenes(this->getGraphicsmodule(), region);
	}
	return region;
}

void cmzn_context::removeRegion(cmzn_region *region)
{
	std::list<cmzn_region*>::iterator iter = std::find(this->allRegions.begin(), this->allRegions.end(), region);
	if (iter != this->allRegions.end())
	{
		cmzn_region_set_context_private(region, 0);
		this->allRegions.erase(iter);
	}
}

cmzn_graphics_module *cmzn_context::getGraphicsmodule()
{
	if (!this->graphics_module)
		this->graphics_module = cmzn_graphics_module_create(this);
	return this->graphics_module;
}

cmzn_context *cmzn_context_create(const char *id)
{
	return cmzn_context::create(id);
}

cmzn_context *cmzn_context_access(cmzn_context *context)
{
	if (context)
		return context->access();
	return 0;
}

int cmzn_context_destroy(cmzn_context **context_address)
{
	if (context_address)
		return cmzn_context::deaccess(*context_address);
	return CMZN_ERROR_ARGUMENT;
}

int cmzn_context_get_version(cmzn_context_id context, int *version_out)
{
	if (context && version_out)
	{
		version_out[0] = ZINC_MAJOR_VERSION;
		version_out[1] = ZINC_MINOR_VERSION;
		version_out[2] = ZINC_PATCH_VERSION;
		return CMZN_OK;
	}

	return CMZN_ERROR_ARGUMENT;
}

int cmzn_context_get_revision(cmzn_context_id context)
{
	if (context)
	{
		return atoi(ZINC_REVISION);
	}

	return 0;
}

char *cmzn_context_get_version_string(cmzn_context_id context)
{
	if (context)
	{
		char *version_string = new char[1000];
		sprintf(version_string, "%d.%d.%d.r%s",
			ZINC_MAJOR_VERSION, ZINC_MINOR_VERSION, ZINC_PATCH_VERSION, ZINC_REVISION);
		if (0 == strcmp(ZINC_BUILD_TYPE, "debug"))
		{
			sprintf(version_string, "%s.Debug",	version_string);
		}
		char *output_string = duplicate_string(version_string);
		delete[] version_string;
		return output_string;
	}

	return 0;
}

struct cmzn_region *cmzn_context_get_default_region(cmzn_context *context)
{
	if (context)
	{
		if (!context->root_region)
			context->root_region = context->createRegion();
		return ACCESS(cmzn_region)(context->root_region);
	}
	display_message(ERROR_MESSAGE, "Zinc Context getDefaultRegion():  Missing context");
	return 0;
}

int cmzn_context_set_default_region(cmzn_context_id context,
	cmzn_region_id region)
{
	if (context)
	{
		if (region && (cmzn_region_get_context_private(region) != context))
		{
			display_message(ERROR_MESSAGE, "Zinc Context setDefaultRegion():  Region is from a different context");	
			return CMZN_ERROR_ARGUMENT_CONTEXT;
		}
		REACCESS(cmzn_region)(&(context->root_region), region);
		return CMZN_OK;
	}
	display_message(ERROR_MESSAGE, "Zinc Context setDefaultRegion():  Missing context");
	return CMZN_ERROR_ARGUMENT;
}

struct cmzn_graphics_module *cmzn_context_get_graphics_module(cmzn_context *context)
{
	if (context)
		return cmzn_graphics_module_access(context->getGraphicsmodule());
	display_message(ERROR_MESSAGE, "cmzn_context_get_default_graphics_module.  Missing context");
	return 0;
}

struct cmzn_region *cmzn_context_create_region(cmzn_context *context)
{
	if (context)
		return context->createRegion();
	display_message(ERROR_MESSAGE, "Zinc Context createRegion():  Missing context");
	return 0;
}

struct Element_point_ranges_selection *cmzn_context_get_element_point_ranges_selection(
	cmzn_context *context)
{
	struct Element_point_ranges_selection *element_point_ranges_selection = NULL;
	if (context)
	{
		if (!context->element_point_ranges_selection)
		{
			context->element_point_ranges_selection = CREATE(Element_point_ranges_selection)();
		}
		element_point_ranges_selection = context->element_point_ranges_selection;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_context_get_element_point_ranges_selection.  Missing context.");
	}
	return element_point_ranges_selection;
}

struct IO_stream_package *cmzn_context_get_default_IO_stream_package(
	cmzn_context *context)
{
	struct IO_stream_package *io_stream_package = NULL;
	if (context)
	{
		if (!context->io_stream_package)
		{
			context->io_stream_package = CREATE(IO_stream_package)();
		}
		io_stream_package = context->io_stream_package;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_context_get_default_IO_stream_package.  Missing context.");
	}

	return io_stream_package;
}

cmzn_timekeepermodule_id cmzn_context_get_timekeepermodule(cmzn_context_id context)
{
	if (context && context->timekeepermodule)
		return context->timekeepermodule->access();
	return 0;
}

struct MANAGER(Curve) *cmzn_context_get_default_curve_manager(
	cmzn_context_id context)
{
	MANAGER(Curve) *curve_manager = NULL;
	if (context)
	{
		if (!context->curve_manager)
		{
			context->curve_manager = CREATE(MANAGER(Curve))();
		}
		curve_manager = context->curve_manager;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cmzn_context_get_default_curve_manager.  "
			"Missing context");
	}
	return curve_manager;
}

cmzn_sceneviewermodule_id cmzn_context_get_sceneviewermodule(
	cmzn_context_id context)
{
	if (context)
	{
		struct cmzn_graphics_module *graphicsModule =
			cmzn_context_get_graphics_module(context);
		cmzn_sceneviewermodule_id sceneviewermodule =
			cmzn_graphics_module_get_sceneviewermodule(graphicsModule);
		cmzn_graphics_module_destroy(&graphicsModule);
		return sceneviewermodule;
	}

	return 0;
}

cmzn_lightmodule_id cmzn_context_get_lightmodule(
	cmzn_context_id context)
{
	if (context)
	{
		struct cmzn_graphics_module *graphicsModule =
			cmzn_context_get_graphics_module(context);
		cmzn_lightmodule_id lightmodule =
			cmzn_graphics_module_get_lightmodule(graphicsModule);
		cmzn_graphics_module_destroy(&graphicsModule);
		return lightmodule;
	}

	return 0;
}

cmzn_materialmodule_id cmzn_context_get_materialmodule(
	cmzn_context_id context)
{
	if (context)
	{
		struct cmzn_graphics_module *graphicsModule =
			cmzn_context_get_graphics_module(context);
		cmzn_materialmodule_id materialmodule =
			cmzn_graphics_module_get_materialmodule(graphicsModule);
		cmzn_graphics_module_destroy(&graphicsModule);
		return materialmodule;
	}

	return 0;
}

cmzn_scenefiltermodule_id cmzn_context_get_scenefiltermodule(
	cmzn_context_id context)
{
	if (context)
	{
		struct cmzn_graphics_module *graphicsModule =
			cmzn_context_get_graphics_module(context);
		cmzn_scenefiltermodule_id scenefiltermodule =
			cmzn_graphics_module_get_scenefiltermodule(graphicsModule);
		cmzn_graphics_module_destroy(&graphicsModule);
		return scenefiltermodule;
	}

	return 0;
}

cmzn_fontmodule_id cmzn_context_get_fontmodule(
	cmzn_context_id context)
{
	if (context)
	{
		struct cmzn_graphics_module *graphicsModule =
			cmzn_context_get_graphics_module(context);
		cmzn_fontmodule_id fontmodule =
			cmzn_graphics_module_get_fontmodule(graphicsModule);
		cmzn_graphics_module_destroy(&graphicsModule);
		return fontmodule;
	}

	return 0;
}

cmzn_tessellationmodule_id cmzn_context_get_tessellationmodule(
	cmzn_context_id context)
{
	if (context)
	{
		struct cmzn_graphics_module *graphicsModule =
			cmzn_context_get_graphics_module(context);
		cmzn_tessellationmodule_id tessellationmodule =
			cmzn_graphics_module_get_tessellationmodule(graphicsModule);
		cmzn_graphics_module_destroy(&graphicsModule);
		return tessellationmodule;
	}

	return 0;
}

cmzn_spectrummodule_id cmzn_context_get_spectrummodule(
	cmzn_context_id context)
{
	if (context)
	{
		struct cmzn_graphics_module *graphicsModule =
			cmzn_context_get_graphics_module(context);
		cmzn_spectrummodule_id spectrummodule =
			cmzn_graphics_module_get_spectrummodule(graphicsModule);
		cmzn_graphics_module_destroy(&graphicsModule);
		return spectrummodule;
	}

	return 0;
}

cmzn_glyphmodule_id cmzn_context_get_glyphmodule(
	cmzn_context_id context)
{
	if (context)
	{
		struct cmzn_graphics_module *graphicsModule =
			cmzn_context_get_graphics_module(context);
		cmzn_glyphmodule_id glyphmodule =
			cmzn_graphics_module_get_glyphmodule(graphicsModule);
		cmzn_graphics_module_destroy(&graphicsModule);
		return glyphmodule;
	}

	return 0;
}

cmzn_logger_id cmzn_context_get_logger(cmzn_context_id context)
{
	if (context)
	{
		return cmzn_logger_access(context->logger);
	}

	return 0;
}
