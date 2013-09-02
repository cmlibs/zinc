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
	cmzn_context_id context;
	cmzn_region_id root_region;
	cmzn_field_module_id fm;
	cmzn_graphics_module_id gm;
	cmzn_scene_id scene;

	ZincTestSetup() :
		context(cmzn_context_create("test")),
		root_region(cmzn_context_get_default_region(context)),
		fm(cmzn_region_get_field_module(root_region))
	{
		EXPECT_NE(static_cast<cmzn_field_module *>(0), fm);
	}

	~ZincTestSetup()
	{
		cmzn_field_module_destroy(&fm);
		cmzn_region_destroy(&root_region);
		cmzn_context_destroy(&context);
	}
};

#endif // __ZINCTESTSETUP_HPP__
