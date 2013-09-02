#include <gtest/gtest.h>

#include <zinc/zincconfigure.h>
#include <zinc/status.h>
#include <zinc/core.h>
#include <zinc/context.h>
#include <zinc/sceneviewer.h>

#include "zinctestsetup.hpp"

TEST(cmzn_scene_viewer_api, destroy_context_before_scene_viewer)
{
	 cmzn_context_id context = cmzn_context_create("test");
	 cmzn_graphics_module_id gm = cmzn_context_get_graphics_module(context);
	 cmzn_scene_viewer_module_id scene_viewer_module = cmzn_graphics_module_get_scene_viewer_module(gm);
	 cmzn_scene_viewer_id scene_viewer = cmzn_scene_viewer_module_create_scene_viewer(scene_viewer_module,
	  CMISS_SCENE_VIEWER_BUFFERING_ANY_MODE, CMISS_SCENE_VIEWER_STEREO_ANY_MODE);
	 cmzn_scene_viewer_module_destroy(&scene_viewer_module);
	 cmzn_graphics_module_destroy(&gm);
	 cmzn_context_destroy(&context);
	 cmzn_scene_viewer_destroy(&scene_viewer);
}

TEST(cmzn_scene_viewer_api, set_background_invalid_args)
{
	ZincTestSetup z;

	cmzn_scene_viewer_module_id svm = cmzn_graphics_module_get_scene_viewer_module(z.gm);
	cmzn_scene_viewer_id sv = cmzn_scene_viewer_module_create_scene_viewer(svm, CMISS_SCENE_VIEWER_BUFFERING_ANY_MODE, CMISS_SCENE_VIEWER_STEREO_ANY_MODE);

	EXPECT_EQ(CMISS_ERROR_ARGUMENT, cmzn_scene_viewer_set_background_colour_component_rgb(0, 0.0, 0.0, 0.0));

	EXPECT_EQ(CMISS_ERROR_ARGUMENT, cmzn_scene_viewer_set_background_colour_rgb(0, 0));

	EXPECT_EQ(CMISS_ERROR_ARGUMENT, cmzn_scene_viewer_set_background_colour_rgb(sv, 0));

	EXPECT_EQ(CMISS_OK, cmzn_scene_viewer_set_background_colour_component_rgb(sv, -1.0, 0.2, 0.8));

	double rgb[3] = {-0.3, -1.0, 2.99};
	EXPECT_EQ(CMISS_OK, cmzn_scene_viewer_set_background_colour_rgb(sv, rgb));

	cmzn_scene_viewer_destroy(&sv);
	cmzn_scene_viewer_module_destroy(&svm);
}

TEST(cmzn_scene_viewer_api, set_background_valid_args)
{
	ZincTestSetup z;

	cmzn_scene_viewer_module_id svm = cmzn_graphics_module_get_scene_viewer_module(z.gm);

	cmzn_scene_viewer_id sv = cmzn_scene_viewer_module_create_scene_viewer(svm, CMISS_SCENE_VIEWER_BUFFERING_ANY_MODE, CMISS_SCENE_VIEWER_STEREO_ANY_MODE);
	EXPECT_EQ(CMISS_OK, cmzn_scene_viewer_set_background_colour_component_rgb(sv, 0.5, 0.2, 0.8));

	double rgb[3] = {0.3, 1.0, 0.99};
	EXPECT_EQ(CMISS_OK, cmzn_scene_viewer_set_background_colour_rgb(sv, rgb));

	cmzn_scene_viewer_destroy(&sv);
	cmzn_scene_viewer_module_destroy(&svm);
}

TEST(cmzn_scene_viewer_api, get_background_rgb_invalid_args)
{
	ZincTestSetup z;
	cmzn_scene_viewer_module_id svm = cmzn_graphics_module_get_scene_viewer_module(z.gm);
	cmzn_scene_viewer_id sv = cmzn_scene_viewer_module_create_scene_viewer(svm, CMISS_SCENE_VIEWER_BUFFERING_ANY_MODE, CMISS_SCENE_VIEWER_STEREO_ANY_MODE);

	EXPECT_EQ(CMISS_ERROR_ARGUMENT, cmzn_scene_viewer_get_background_colour_rgb(0, 0));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, cmzn_scene_viewer_get_background_colour_rgb(sv, 0));

	cmzn_scene_viewer_destroy(&sv);
	cmzn_scene_viewer_module_destroy(&svm);
}

TEST(cmzn_scene_viewer_api, get_background_rgb)
{
	ZincTestSetup z;
	double rgb[3] = {0.0, 0.0, 0.0};

	cmzn_scene_viewer_module_id svm = cmzn_graphics_module_get_scene_viewer_module(z.gm);
	cmzn_scene_viewer_id sv = cmzn_scene_viewer_module_create_scene_viewer(svm, CMISS_SCENE_VIEWER_BUFFERING_ANY_MODE, CMISS_SCENE_VIEWER_STEREO_ANY_MODE);

	EXPECT_EQ(CMISS_OK, cmzn_scene_viewer_set_background_colour_component_rgb(sv, 0.5, 0.2, 0.8));
	EXPECT_EQ(CMISS_OK, cmzn_scene_viewer_get_background_colour_rgb(sv, rgb));

	EXPECT_EQ(0.5, rgb[0]);
	EXPECT_EQ(0.2, rgb[1]);
	EXPECT_EQ(0.8, rgb[2]);

	cmzn_scene_viewer_destroy(&sv);
	cmzn_scene_viewer_module_destroy(&svm);
}

TEST(cmzn_scene_viewer_api, eye_position_invalid_args)
{
	ZincTestSetup z;
	cmzn_scene_viewer_module_id svm = cmzn_graphics_module_get_scene_viewer_module(z.gm);
	cmzn_scene_viewer_id sv = cmzn_scene_viewer_module_create_scene_viewer(svm, CMISS_SCENE_VIEWER_BUFFERING_ANY_MODE, CMISS_SCENE_VIEWER_STEREO_ANY_MODE);

	EXPECT_EQ(CMISS_ERROR_ARGUMENT, cmzn_scene_viewer_set_eye_position(0, 0));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, cmzn_scene_viewer_set_eye_position(sv, 0));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, cmzn_scene_viewer_get_eye_position(0, 0));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, cmzn_scene_viewer_get_eye_position(sv, 0));

	cmzn_scene_viewer_destroy(&sv);
	cmzn_scene_viewer_module_destroy(&svm);
}

TEST(cmzn_scene_viewer_api, eye_position_valid_args)
{
	ZincTestSetup z;
	cmzn_scene_viewer_module_id svm = cmzn_graphics_module_get_scene_viewer_module(z.gm);
	cmzn_scene_viewer_id sv = cmzn_scene_viewer_module_create_scene_viewer(svm, CMISS_SCENE_VIEWER_BUFFERING_ANY_MODE, CMISS_SCENE_VIEWER_STEREO_ANY_MODE);

	double eye[] = {3.0, 4.5, 6.7};
	double eyeOut[3];
	EXPECT_EQ(CMISS_OK, cmzn_scene_viewer_set_eye_position(sv, eye));
	EXPECT_EQ(CMISS_OK, cmzn_scene_viewer_get_eye_position(sv, eyeOut));
	EXPECT_EQ(eye[0], eyeOut[0]);
	EXPECT_EQ(eye[1], eyeOut[1]);
	EXPECT_EQ(eye[2], eyeOut[2]);

	cmzn_scene_viewer_destroy(&sv);
	cmzn_scene_viewer_module_destroy(&svm);
}

TEST(cmzn_scene_viewer_api, lookat_position_invalid_args)
{
	ZincTestSetup z;
	cmzn_scene_viewer_module_id svm = cmzn_graphics_module_get_scene_viewer_module(z.gm);
	cmzn_scene_viewer_id sv = cmzn_scene_viewer_module_create_scene_viewer(svm, CMISS_SCENE_VIEWER_BUFFERING_ANY_MODE, CMISS_SCENE_VIEWER_STEREO_ANY_MODE);

	EXPECT_EQ(CMISS_ERROR_ARGUMENT, cmzn_scene_viewer_set_lookat_position(0, 0));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, cmzn_scene_viewer_set_lookat_position(sv, 0));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, cmzn_scene_viewer_get_lookat_position(0, 0));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, cmzn_scene_viewer_get_lookat_position(sv, 0));

	cmzn_scene_viewer_destroy(&sv);
	cmzn_scene_viewer_module_destroy(&svm);
}

TEST(cmzn_scene_viewer_api, lookat_position_valid_args)
{
	ZincTestSetup z;
	cmzn_scene_viewer_module_id svm = cmzn_graphics_module_get_scene_viewer_module(z.gm);
	cmzn_scene_viewer_id sv = cmzn_scene_viewer_module_create_scene_viewer(svm, CMISS_SCENE_VIEWER_BUFFERING_ANY_MODE, CMISS_SCENE_VIEWER_STEREO_ANY_MODE);

	double lookat[] = {-2.0, 1.5, 16.7};
	double lookatOut[3];
	EXPECT_EQ(CMISS_OK, cmzn_scene_viewer_set_lookat_position(sv, lookat));
	EXPECT_EQ(CMISS_OK, cmzn_scene_viewer_get_lookat_position(sv, lookatOut));
	EXPECT_EQ(lookat[0], lookatOut[0]);
	EXPECT_EQ(lookat[1], lookatOut[1]);
	EXPECT_EQ(lookat[2], lookatOut[2]);

	cmzn_scene_viewer_destroy(&sv);
	cmzn_scene_viewer_module_destroy(&svm);
}

TEST(cmzn_scene_viewer_api, up_vector_invalid_args)
{
	ZincTestSetup z;
	cmzn_scene_viewer_module_id svm = cmzn_graphics_module_get_scene_viewer_module(z.gm);
	cmzn_scene_viewer_id sv = cmzn_scene_viewer_module_create_scene_viewer(svm, CMISS_SCENE_VIEWER_BUFFERING_ANY_MODE, CMISS_SCENE_VIEWER_STEREO_ANY_MODE);

	EXPECT_EQ(CMISS_ERROR_ARGUMENT, cmzn_scene_viewer_set_up_vector(0, 0));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, cmzn_scene_viewer_set_up_vector(sv, 0));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, cmzn_scene_viewer_get_up_vector(0, 0));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, cmzn_scene_viewer_get_up_vector(sv, 0));

	cmzn_scene_viewer_destroy(&sv);
	cmzn_scene_viewer_module_destroy(&svm);
}

TEST(cmzn_scene_viewer_api, up_vector_valid_args)
{
	ZincTestSetup z;
	cmzn_scene_viewer_module_id svm = cmzn_graphics_module_get_scene_viewer_module(z.gm);
	cmzn_scene_viewer_id sv = cmzn_scene_viewer_module_create_scene_viewer(svm, CMISS_SCENE_VIEWER_BUFFERING_ANY_MODE, CMISS_SCENE_VIEWER_STEREO_ANY_MODE);

	double upVector[] = {2.0, 0.0, 0.0};
	double upVectorOut[3];
	EXPECT_EQ(CMISS_OK, cmzn_scene_viewer_set_up_vector(sv, upVector));
	EXPECT_EQ(CMISS_OK, cmzn_scene_viewer_get_up_vector(sv, upVectorOut));
	EXPECT_EQ(1.0, upVectorOut[0]);
	EXPECT_EQ(upVector[1], upVectorOut[1]);
	EXPECT_EQ(upVector[2], upVectorOut[2]);

	cmzn_scene_viewer_destroy(&sv);
	cmzn_scene_viewer_module_destroy(&svm);
}

