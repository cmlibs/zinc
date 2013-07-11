#ifndef __ZINCTESTSETUP_HPP__
#define __ZINCTESTSETUP_HPP__

#include <gtest/gtest.h>

#include <zinc/zincconfigure.h>
#include <zinc/status.h>
#include <zinc/context.h>
#include <zinc/region.h>
#include <zinc/fieldmodule.h>

class ZincTestSetup
{
public:
	Cmiss_context_id context;
	Cmiss_region_id root_region;
	Cmiss_field_module_id fm;
	Cmiss_graphics_module_id gm;
	Cmiss_scene_id scene;

	ZincTestSetup() :
		context(Cmiss_context_create("test")),
		root_region(Cmiss_context_get_default_region(context)),
		fm(Cmiss_region_get_field_module(root_region))
	{
		EXPECT_NE(static_cast<Cmiss_field_module *>(0), fm);
	}

	~ZincTestSetup()
	{
		Cmiss_field_module_destroy(&fm);
		Cmiss_region_destroy(&root_region);
		Cmiss_context_destroy(&context);
	}
};

#endif // __ZINCTESTSETUP_HPP__
