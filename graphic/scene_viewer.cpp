#include <gtest/gtest.h>

#include <zinc/zincconfigure.h>
#include <zinc/status.h>
#include <zinc/core.h>
#include <zinc/context.h>
#include <zinc/sceneviewer.h>

#include "zinctestsetup.hpp"

TEST(Cmiss_scene_viewer_api, destroy_context_before_scene_viewer)
{
	 Cmiss_context_id context = Cmiss_context_create("test");
	 Cmiss_graphics_module_id gm = Cmiss_context_get_graphics_module(context);
	 Cmiss_scene_viewer_module_id scene_viewer_module = Cmiss_graphics_module_get_scene_viewer_module(gm);
	 Cmiss_scene_viewer_id scene_viewer = Cmiss_scene_viewer_module_create_scene_viewer(scene_viewer_module,
	  CMISS_SCENE_VIEWER_BUFFERING_ANY_MODE, CMISS_SCENE_VIEWER_STEREO_ANY_MODE);
	 Cmiss_scene_viewer_module_destroy(&scene_viewer_module);
	 Cmiss_graphics_module_destroy(&gm);
	 Cmiss_context_destroy(&context);
	 Cmiss_scene_viewer_destroy(&scene_viewer);
}

TEST(Cmiss_scene_viewer_api, set_background_invalid_args)
{
	ZincTestSetup z;

	Cmiss_scene_viewer_module_id svm = Cmiss_graphics_module_get_scene_viewer_module(z.gm);

	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_scene_viewer_set_background_colour_component_rgb(0, 0.0, 0.0, 0.0));

	Cmiss_scene_viewer_id sv = Cmiss_scene_viewer_module_create_scene_viewer(svm, CMISS_SCENE_VIEWER_BUFFERING_ANY_MODE, CMISS_SCENE_VIEWER_STEREO_ANY_MODE);
	EXPECT_EQ(CMISS_OK, Cmiss_scene_viewer_set_background_colour_component_rgb(sv, -1.0, 0.2, 0.8));

	double rgb[3] = {-0.3, -1.0, 2.99};
	EXPECT_EQ(CMISS_OK, Cmiss_scene_viewer_set_background_colour_rgb(sv, rgb));

	Cmiss_scene_viewer_destroy(&sv);
	Cmiss_scene_viewer_module_destroy(&svm);
}

TEST(Cmiss_scene_viewer_api, set_background_valid_args)
{
	ZincTestSetup z;

	Cmiss_scene_viewer_module_id svm = Cmiss_graphics_module_get_scene_viewer_module(z.gm);

	Cmiss_scene_viewer_id sv = Cmiss_scene_viewer_module_create_scene_viewer(svm, CMISS_SCENE_VIEWER_BUFFERING_ANY_MODE, CMISS_SCENE_VIEWER_STEREO_ANY_MODE);
	EXPECT_EQ(CMISS_OK, Cmiss_scene_viewer_set_background_colour_component_rgb(sv, 0.5, 0.2, 0.8));

	double rgb[3] = {0.3, 1.0, 0.99};
	EXPECT_EQ(CMISS_OK, Cmiss_scene_viewer_set_background_colour_rgb(sv, rgb));

	Cmiss_scene_viewer_destroy(&sv);
	Cmiss_scene_viewer_module_destroy(&svm);
}

TEST(Cmiss_scene_viewer_api, get_background_rgb)
{
	ZincTestSetup z;
	double rgb[3] = {0.0, 0.0, 0.0};

	EXPECT_EQ(CMISS_ERROR_ARGUMENT, Cmiss_scene_viewer_set_background_colour_component_rgb(0, 0.5, 0.2, 0.8));

	Cmiss_scene_viewer_module_id svm = Cmiss_graphics_module_get_scene_viewer_module(z.gm);

	Cmiss_scene_viewer_id sv = Cmiss_scene_viewer_module_create_scene_viewer(svm, CMISS_SCENE_VIEWER_BUFFERING_ANY_MODE, CMISS_SCENE_VIEWER_STEREO_ANY_MODE);
	EXPECT_EQ(CMISS_OK, Cmiss_scene_viewer_set_background_colour_component_rgb(sv, 0.5, 0.2, 0.8));
	EXPECT_EQ(CMISS_OK, Cmiss_scene_viewer_get_background_colour_rgb(sv, rgb));

	EXPECT_EQ(0.5, rgb[0]);
	EXPECT_EQ(0.2, rgb[1]);
	EXPECT_EQ(0.8, rgb[2]);

	Cmiss_scene_viewer_destroy(&sv);
	Cmiss_scene_viewer_module_destroy(&svm);
}

