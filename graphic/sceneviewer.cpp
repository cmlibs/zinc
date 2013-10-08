#include <gtest/gtest.h>

#include <zinc/zincconfigure.h>
#include <zinc/status.h>
#include <zinc/core.h>
#include <zinc/context.h>
#include <zinc/sceneviewer.h>

#include "zinctestsetup.hpp"

TEST(cmzn_sceneviewer_api, destroy_context_before_scene_viewer)
{
	 cmzn_context_id context = cmzn_context_create("test");
	 cmzn_graphics_module_id gm = cmzn_context_get_graphics_module(context);
	 cmzn_sceneviewermodule_id svModule = cmzn_graphics_module_get_sceneviewermodule(gm);
	 cmzn_sceneviewer_id sv = cmzn_sceneviewermodule_create_sceneviewer(svModule,
	  CMZN_SCENEVIEWER_BUFFERING_ANY_MODE, CMZN_SCENEVIEWER_STEREO_ANY_MODE);
	 cmzn_sceneviewermodule_destroy(&svModule);
	 cmzn_graphics_module_destroy(&gm);
	 cmzn_context_destroy(&context);
	 cmzn_sceneviewer_destroy(&sv);
}

TEST(cmzn_sceneviewer_api, set_background_invalid_args)
{
	ZincTestSetup z;

	cmzn_sceneviewermodule_id svm = cmzn_graphics_module_get_sceneviewermodule(z.gm);
	cmzn_sceneviewer_id sv = cmzn_sceneviewermodule_create_sceneviewer(svm, CMZN_SCENEVIEWER_BUFFERING_ANY_MODE, CMZN_SCENEVIEWER_STEREO_ANY_MODE);

	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_sceneviewer_set_background_colour_component_rgb(0, 0.0, 0.0, 0.0));

	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_sceneviewer_set_background_colour_rgb(0, 0));

	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_sceneviewer_set_background_colour_rgb(sv, 0));

	EXPECT_EQ(CMZN_OK, cmzn_sceneviewer_set_background_colour_component_rgb(sv, -1.0, 0.2, 0.8));

	double rgb[3] = {-0.3, -1.0, 2.99};
	EXPECT_EQ(CMZN_OK, cmzn_sceneviewer_set_background_colour_rgb(sv, rgb));

	cmzn_sceneviewer_destroy(&sv);
	cmzn_sceneviewermodule_destroy(&svm);
}

TEST(cmzn_sceneviewer_api, set_background_valid_args)
{
	ZincTestSetup z;

	cmzn_sceneviewermodule_id svm = cmzn_graphics_module_get_sceneviewermodule(z.gm);

	cmzn_sceneviewer_id sv = cmzn_sceneviewermodule_create_sceneviewer(svm, CMZN_SCENEVIEWER_BUFFERING_ANY_MODE, CMZN_SCENEVIEWER_STEREO_ANY_MODE);
	EXPECT_EQ(CMZN_OK, cmzn_sceneviewer_set_background_colour_component_rgb(sv, 0.5, 0.2, 0.8));

	double rgb[3] = {0.3, 1.0, 0.99};
	EXPECT_EQ(CMZN_OK, cmzn_sceneviewer_set_background_colour_rgb(sv, rgb));

	cmzn_sceneviewer_destroy(&sv);
	cmzn_sceneviewermodule_destroy(&svm);
}

TEST(cmzn_sceneviewer_api, get_background_rgb_invalid_args)
{
	ZincTestSetup z;
	cmzn_sceneviewermodule_id svm = cmzn_graphics_module_get_sceneviewermodule(z.gm);
	cmzn_sceneviewer_id sv = cmzn_sceneviewermodule_create_sceneviewer(svm, CMZN_SCENEVIEWER_BUFFERING_ANY_MODE, CMZN_SCENEVIEWER_STEREO_ANY_MODE);

	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_sceneviewer_get_background_colour_rgb(0, 0));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_sceneviewer_get_background_colour_rgb(sv, 0));

	cmzn_sceneviewer_destroy(&sv);
	cmzn_sceneviewermodule_destroy(&svm);
}

TEST(cmzn_sceneviewer_api, get_background_rgb)
{
	ZincTestSetup z;
	double rgb[3] = {0.0, 0.0, 0.0};

	cmzn_sceneviewermodule_id svm = cmzn_graphics_module_get_sceneviewermodule(z.gm);
	cmzn_sceneviewer_id sv = cmzn_sceneviewermodule_create_sceneviewer(svm, CMZN_SCENEVIEWER_BUFFERING_ANY_MODE, CMZN_SCENEVIEWER_STEREO_ANY_MODE);

	EXPECT_EQ(CMZN_OK, cmzn_sceneviewer_set_background_colour_component_rgb(sv, 0.5, 0.2, 0.8));
	EXPECT_EQ(CMZN_OK, cmzn_sceneviewer_get_background_colour_rgb(sv, rgb));

	EXPECT_EQ(0.5, rgb[0]);
	EXPECT_EQ(0.2, rgb[1]);
	EXPECT_EQ(0.8, rgb[2]);

	cmzn_sceneviewer_destroy(&sv);
	cmzn_sceneviewermodule_destroy(&svm);
}

TEST(cmzn_sceneviewer_api, eye_position_invalid_args)
{
	ZincTestSetup z;
	cmzn_sceneviewermodule_id svm = cmzn_graphics_module_get_sceneviewermodule(z.gm);
	cmzn_sceneviewer_id sv = cmzn_sceneviewermodule_create_sceneviewer(svm, CMZN_SCENEVIEWER_BUFFERING_ANY_MODE, CMZN_SCENEVIEWER_STEREO_ANY_MODE);

	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_sceneviewer_set_eye_position(0, 0));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_sceneviewer_set_eye_position(sv, 0));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_sceneviewer_get_eye_position(0, 0));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_sceneviewer_get_eye_position(sv, 0));

	cmzn_sceneviewer_destroy(&sv);
	cmzn_sceneviewermodule_destroy(&svm);
}

TEST(cmzn_sceneviewer_api, eye_position_valid_args)
{
	ZincTestSetup z;
	cmzn_sceneviewermodule_id svm = cmzn_graphics_module_get_sceneviewermodule(z.gm);
	cmzn_sceneviewer_id sv = cmzn_sceneviewermodule_create_sceneviewer(svm, CMZN_SCENEVIEWER_BUFFERING_ANY_MODE, CMZN_SCENEVIEWER_STEREO_ANY_MODE);

	double eye[] = {3.0, 4.5, 6.7};
	double eyeOut[3];
	EXPECT_EQ(CMZN_OK, cmzn_sceneviewer_set_eye_position(sv, eye));
	EXPECT_EQ(CMZN_OK, cmzn_sceneviewer_get_eye_position(sv, eyeOut));
	EXPECT_EQ(eye[0], eyeOut[0]);
	EXPECT_EQ(eye[1], eyeOut[1]);
	EXPECT_EQ(eye[2], eyeOut[2]);

	cmzn_sceneviewer_destroy(&sv);
	cmzn_sceneviewermodule_destroy(&svm);
}

TEST(cmzn_sceneviewer_api, lookat_position_invalid_args)
{
	ZincTestSetup z;
	cmzn_sceneviewermodule_id svm = cmzn_graphics_module_get_sceneviewermodule(z.gm);
	cmzn_sceneviewer_id sv = cmzn_sceneviewermodule_create_sceneviewer(svm, CMZN_SCENEVIEWER_BUFFERING_ANY_MODE, CMZN_SCENEVIEWER_STEREO_ANY_MODE);

	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_sceneviewer_set_lookat_position(0, 0));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_sceneviewer_set_lookat_position(sv, 0));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_sceneviewer_get_lookat_position(0, 0));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_sceneviewer_get_lookat_position(sv, 0));

	cmzn_sceneviewer_destroy(&sv);
	cmzn_sceneviewermodule_destroy(&svm);
}

TEST(cmzn_sceneviewer_api, lookat_position_valid_args)
{
	ZincTestSetup z;
	cmzn_sceneviewermodule_id svm = cmzn_graphics_module_get_sceneviewermodule(z.gm);
	cmzn_sceneviewer_id sv = cmzn_sceneviewermodule_create_sceneviewer(svm, CMZN_SCENEVIEWER_BUFFERING_ANY_MODE, CMZN_SCENEVIEWER_STEREO_ANY_MODE);

	double lookat[] = {-2.0, 1.5, 16.7};
	double lookatOut[3];
	EXPECT_EQ(CMZN_OK, cmzn_sceneviewer_set_lookat_position(sv, lookat));
	EXPECT_EQ(CMZN_OK, cmzn_sceneviewer_get_lookat_position(sv, lookatOut));
	EXPECT_EQ(lookat[0], lookatOut[0]);
	EXPECT_EQ(lookat[1], lookatOut[1]);
	EXPECT_EQ(lookat[2], lookatOut[2]);

	cmzn_sceneviewer_destroy(&sv);
	cmzn_sceneviewermodule_destroy(&svm);
}

TEST(cmzn_sceneviewer_api, up_vector_invalid_args)
{
	ZincTestSetup z;
	cmzn_sceneviewermodule_id svm = cmzn_graphics_module_get_sceneviewermodule(z.gm);
	cmzn_sceneviewer_id sv = cmzn_sceneviewermodule_create_sceneviewer(svm, CMZN_SCENEVIEWER_BUFFERING_ANY_MODE, CMZN_SCENEVIEWER_STEREO_ANY_MODE);

	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_sceneviewer_set_up_vector(0, 0));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_sceneviewer_set_up_vector(sv, 0));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_sceneviewer_get_up_vector(0, 0));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, cmzn_sceneviewer_get_up_vector(sv, 0));

	cmzn_sceneviewer_destroy(&sv);
	cmzn_sceneviewermodule_destroy(&svm);
}

TEST(cmzn_sceneviewer_api, up_vector_valid_args)
{
	ZincTestSetup z;
	cmzn_sceneviewermodule_id svm = cmzn_graphics_module_get_sceneviewermodule(z.gm);
	cmzn_sceneviewer_id sv = cmzn_sceneviewermodule_create_sceneviewer(svm, CMZN_SCENEVIEWER_BUFFERING_ANY_MODE, CMZN_SCENEVIEWER_STEREO_ANY_MODE);

	double upVector[] = {2.0, 0.0, 0.0};
	double upVectorOut[3];
	EXPECT_EQ(CMZN_OK, cmzn_sceneviewer_set_up_vector(sv, upVector));
	EXPECT_EQ(CMZN_OK, cmzn_sceneviewer_get_up_vector(sv, upVectorOut));
	EXPECT_EQ(1.0, upVectorOut[0]);
	EXPECT_EQ(upVector[1], upVectorOut[1]);
	EXPECT_EQ(upVector[2], upVectorOut[2]);

	cmzn_sceneviewer_destroy(&sv);
	cmzn_sceneviewermodule_destroy(&svm);
}

TEST(cmzn_sceneviewer_api, get_set)
{
	cmzn_context_id context = cmzn_context_create("test");
	cmzn_graphics_module_id gm = cmzn_context_get_graphics_module(context);
	cmzn_sceneviewermodule_id svModule = cmzn_graphics_module_get_sceneviewermodule(gm);
	cmzn_sceneviewer_id sv = cmzn_sceneviewermodule_create_sceneviewer(svModule,
		CMZN_SCENEVIEWER_BUFFERING_ANY_MODE, CMZN_SCENEVIEWER_STEREO_ANY_MODE);

	cmzn_sceneviewer_interact_mode interact_mode = cmzn_sceneviewer_get_interact_mode(sv);
	EXPECT_EQ(CMZN_SCENEVIEWER_INTERACT_STANDARD, interact_mode);
	EXPECT_EQ(CMZN_OK, cmzn_sceneviewer_set_interact_mode(sv, CMZN_SCENEVIEWER_INTERACT_2D));
	interact_mode = cmzn_sceneviewer_get_interact_mode(sv);
	EXPECT_EQ(CMZN_SCENEVIEWER_INTERACT_2D, interact_mode);

	cmzn_sceneviewer_viewport_mode viewport_mode = cmzn_sceneviewer_get_viewport_mode(sv);
	EXPECT_EQ(CMZN_SCENEVIEWER_VIEWPORT_RELATIVE, viewport_mode);
	EXPECT_EQ(CMZN_OK, cmzn_sceneviewer_set_viewport_mode(sv, CMZN_SCENEVIEWER_VIEWPORT_ABSOLUTE));
	viewport_mode = cmzn_sceneviewer_get_viewport_mode(sv);
	EXPECT_EQ(CMZN_SCENEVIEWER_VIEWPORT_ABSOLUTE, viewport_mode);

	cmzn_sceneviewer_projection_mode projection_mode = cmzn_sceneviewer_get_projection_mode(sv);
	EXPECT_EQ(CMZN_SCENEVIEWER_PROJECTION_PARALLEL, projection_mode);
	EXPECT_EQ(CMZN_OK, cmzn_sceneviewer_set_projection_mode(sv, CMZN_SCENEVIEWER_PROJECTION_PERSPECTIVE));
	projection_mode = cmzn_sceneviewer_get_projection_mode(sv);
	EXPECT_EQ(CMZN_SCENEVIEWER_PROJECTION_PERSPECTIVE, projection_mode);

	cmzn_sceneviewer_blending_mode blending_mode = cmzn_sceneviewer_get_blending_mode(sv);
	EXPECT_EQ(CMZN_SCENEVIEWER_BLENDING_NORMAL, blending_mode);
	EXPECT_EQ(CMZN_OK, cmzn_sceneviewer_set_blending_mode(sv, CMZN_SCENEVIEWER_BLENDING_NONE));
	blending_mode = cmzn_sceneviewer_get_blending_mode(sv);
	EXPECT_EQ(CMZN_SCENEVIEWER_BLENDING_NONE, blending_mode);

	double value;
	ASSERT_DOUBLE_EQ(1.2309594173407747, value = cmzn_sceneviewer_get_view_angle(sv));
	EXPECT_EQ(CMZN_OK, cmzn_sceneviewer_set_view_angle(sv, 0.3));
	ASSERT_DOUBLE_EQ(0.3, value = cmzn_sceneviewer_get_view_angle(sv));

	cmzn_sceneviewermodule_destroy(&svModule);
	cmzn_graphics_module_destroy(&gm);
	cmzn_context_destroy(&context);
	cmzn_sceneviewer_destroy(&sv);
}
