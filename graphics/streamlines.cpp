/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include <zinc/status.h>
#include <zinc/core.h>
#include <zinc/context.h>
#include <zinc/region.h>
#include <zinc/fieldmodule.h>
#include <zinc/scene.h>
#include <zinc/field.h>
#include <zinc/fieldconstant.h>
#include <zinc/graphics.h>

#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"
#include "zinc/fieldconstant.hpp"
#include "zinc/graphics.hpp"

TEST(cmzn_graphics_streamlines, create_cast)
{
	ZincTestSetup zinc;

	cmzn_graphics_id gr = cmzn_scene_create_graphics(zinc.scene, CMZN_GRAPHICS_TYPE_STREAMLINES);
	EXPECT_NE(static_cast<cmzn_graphics *>(0), gr);
	EXPECT_EQ(CMZN_GRAPHICS_TYPE_STREAMLINES, cmzn_graphics_get_type(gr));

	cmzn_graphics_streamlines_id st = cmzn_graphics_cast_streamlines(gr);
	EXPECT_EQ(reinterpret_cast<cmzn_graphics_streamlines *>(gr), st);
	EXPECT_EQ(static_cast<cmzn_graphics_points *>(0), cmzn_graphics_cast_points(gr));

	// Must not to destroy the returned base handle
	EXPECT_EQ(gr, cmzn_graphics_streamlines_base_cast(st));

	EXPECT_EQ(CMZN_OK, cmzn_graphics_streamlines_destroy(&st));
	EXPECT_EQ(CMZN_OK, cmzn_graphics_destroy(&gr));
}

TEST(cmzn_graphics_streamlines, create_cpp)
{
	ZincTestSetupCpp zinc;

	GraphicsStreamlines st = zinc.scene.createGraphicsStreamlines();
	EXPECT_TRUE(st.isValid());
	EXPECT_EQ(Graphics::TYPE_STREAMLINES, st.getType());
}

TEST(cmzn_graphics_streamlines, cast_cpp)
{
	ZincTestSetupCpp zinc;

	Graphics gr = zinc.scene.createGraphics(Graphics::TYPE_STREAMLINES);
	EXPECT_TRUE(gr.isValid());

	EXPECT_FALSE(gr.castContours().isValid());
	EXPECT_FALSE(gr.castLines().isValid());
	EXPECT_FALSE(gr.castPoints().isValid());
	EXPECT_FALSE(gr.castSurfaces().isValid());
	GraphicsStreamlines st = gr.castStreamlines();
	EXPECT_TRUE(st.isValid());

	// try any base class API
	Material material = st.getMaterial();
	ASSERT_TRUE(material.isValid());
}

TEST(cmzn_graphics_streamlines, stream_vector_field)
{
	ZincTestSetup zinc;

	cmzn_graphics_id gr = cmzn_scene_create_graphics_streamlines(zinc.scene);
	cmzn_graphics_streamlines_id st = cmzn_graphics_cast_streamlines(gr);
	cmzn_graphics_destroy(&gr);
	EXPECT_NE(static_cast<cmzn_graphics_streamlines *>(0), st);

	const double values[] = { 1.0, 2.0, 3.0 };
	cmzn_field_id stream_vector_field = cmzn_fieldmodule_create_field_constant(zinc.fm,
		sizeof(values)/sizeof(double), values);
	EXPECT_NE(static_cast<cmzn_field *>(0), stream_vector_field);

	EXPECT_EQ((cmzn_field_id)0, cmzn_graphics_streamlines_get_stream_vector_field(st));
	EXPECT_EQ(CMZN_OK, cmzn_graphics_streamlines_set_stream_vector_field(st, stream_vector_field));

	cmzn_field_id temp_stream_vector_field = cmzn_graphics_streamlines_get_stream_vector_field(st);
	EXPECT_EQ(stream_vector_field, temp_stream_vector_field);
	cmzn_field_destroy(&temp_stream_vector_field);
	cmzn_field_destroy(&stream_vector_field);

	EXPECT_EQ(CMZN_OK, cmzn_graphics_streamlines_set_stream_vector_field(st, 0));
	EXPECT_EQ(static_cast<cmzn_field *>(0), cmzn_graphics_streamlines_get_stream_vector_field(st));

	cmzn_graphics_streamlines_destroy(&st);
}

TEST(cmzn_graphics_streamlines, stream_vector_field_cpp)
{
	ZincTestSetupCpp zinc;

	GraphicsStreamlines st = zinc.scene.createGraphicsStreamlines();
	EXPECT_TRUE(st.isValid());

	Field tempStreamVectorField = st.getStreamVectorField();
	EXPECT_FALSE(tempStreamVectorField.isValid());

	const double values[] = { 1.0, 2.0, 3.0 };
	Field streamVectorField = zinc.fm.createFieldConstant(sizeof(values)/sizeof(double), values);
	EXPECT_TRUE(streamVectorField.isValid());

	EXPECT_EQ(CMZN_OK, st.setStreamVectorField(streamVectorField));
	tempStreamVectorField = st.getStreamVectorField();
	EXPECT_EQ(streamVectorField.getId(), tempStreamVectorField.getId());

	EXPECT_EQ(CMZN_OK, st.setStreamVectorField(Field()));
	tempStreamVectorField = st.getStreamVectorField();
	EXPECT_FALSE(tempStreamVectorField.isValid());
}

TEST(cmzn_graphics_streamlines, track_direction)
{
	ZincTestSetup zinc;

	cmzn_graphics_id gr = cmzn_scene_create_graphics_streamlines(zinc.scene);
	cmzn_graphics_streamlines_id st = cmzn_graphics_cast_streamlines(gr);
	cmzn_graphics_destroy(&gr);
	EXPECT_NE(static_cast<cmzn_graphics_streamlines *>(0), st);

	EXPECT_EQ(CMZN_GRAPHICS_STREAMLINES_TRACK_DIRECTION_FORWARD, cmzn_graphics_streamlines_get_track_direction(st));
	EXPECT_EQ(CMZN_OK, cmzn_graphics_streamlines_set_track_direction(st, CMZN_GRAPHICS_STREAMLINES_TRACK_DIRECTION_REVERSE));
	EXPECT_EQ(CMZN_GRAPHICS_STREAMLINES_TRACK_DIRECTION_REVERSE, cmzn_graphics_streamlines_get_track_direction(st));

	cmzn_graphics_streamlines_destroy(&st);
}

TEST(cmzn_graphics_streamlines, track_direction_cpp)
{
	ZincTestSetupCpp zinc;

	GraphicsStreamlines st = zinc.scene.createGraphicsStreamlines();
	EXPECT_TRUE(st.isValid());

	EXPECT_EQ(GraphicsStreamlines::TRACK_DIRECTION_FORWARD, st.getTrackDirection());
	EXPECT_EQ(CMZN_OK, st.setTrackDirection(GraphicsStreamlines::TRACK_DIRECTION_REVERSE));
	EXPECT_EQ(GraphicsStreamlines::TRACK_DIRECTION_REVERSE, st.getTrackDirection());
}

TEST(cmzn_graphics_streamlines, track_length)
{
	ZincTestSetup zinc;

	cmzn_graphics_id gr = cmzn_scene_create_graphics_streamlines(zinc.scene);
	cmzn_graphics_streamlines_id st = cmzn_graphics_cast_streamlines(gr);
	cmzn_graphics_destroy(&gr);
	EXPECT_NE(static_cast<cmzn_graphics_streamlines *>(0), st);

	const double trackLength = 500.0;
	EXPECT_DOUBLE_EQ(1.0, cmzn_graphics_streamlines_get_track_length(st));
	EXPECT_EQ(CMZN_OK, cmzn_graphics_streamlines_set_track_length(st, trackLength));
	EXPECT_DOUBLE_EQ(trackLength, cmzn_graphics_streamlines_get_track_length(st));

	cmzn_graphics_streamlines_destroy(&st);
}

TEST(cmzn_graphics_streamlines, track_length_cpp)
{
	ZincTestSetupCpp zinc;

	GraphicsStreamlines st = zinc.scene.createGraphicsStreamlines();
	EXPECT_TRUE(st.isValid());

	const double trackLength = 500.0;
	EXPECT_DOUBLE_EQ(1.0, st.getTrackLength());
	EXPECT_EQ(CMZN_OK, st.setTrackLength(trackLength));
	EXPECT_DOUBLE_EQ(trackLength, st.getTrackLength());
}
