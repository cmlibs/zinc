
#include <gtest/gtest.h>

#include <zinc/zincconfigure.h>
#include <zinc/status.h>
#include <zinc/core.h>
#include <zinc/context.h>
#include <zinc/region.h>
#include <zinc/fieldmodule.h>
#include <zinc/scene.h>
#include <zinc/field.h>
#include <zinc/fieldconstant.h>
#include <zinc/graphic.h>

#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"
#include "zinc/fieldtypesconstant.hpp"
#include "zinc/graphic.hpp"

TEST(Cmiss_graphic_streamlines, create)
{
	ZincTestSetup zinc;

	Cmiss_graphic_streamlines_id st = Cmiss_scene_create_graphic_streamlines(zinc.scene);
	EXPECT_NE(static_cast<Cmiss_graphic_streamlines *>(0), st);

	EXPECT_EQ(CMISS_OK, Cmiss_graphic_streamlines_destroy(&st));
}

TEST(Cmiss_graphic_streamlines, create_cpp)
{
	ZincTestSetupCpp zinc;

	GraphicStreamlines st = zinc.scene.createGraphicStreamlines();
	EXPECT_TRUE(st.isValid());
}

TEST(Cmiss_graphic_streamlines, cast)
{
	ZincTestSetup zinc;

	Cmiss_graphic_id gr = Cmiss_scene_create_graphic(zinc.scene, CMISS_GRAPHIC_STREAMLINES);
	EXPECT_NE(static_cast<Cmiss_graphic *>(0), gr);

	Cmiss_graphic_streamlines_id st = Cmiss_graphic_cast_streamlines(gr);
	EXPECT_EQ(reinterpret_cast<Cmiss_graphic_streamlines *>(gr), st);
	EXPECT_EQ(static_cast<Cmiss_graphic_points *>(0), Cmiss_graphic_cast_points(gr));

	EXPECT_EQ(gr, Cmiss_graphic_streamlines_base_cast(st));

	EXPECT_EQ(CMISS_OK, Cmiss_graphic_streamlines_destroy(&st));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_destroy(&gr));
}

TEST(Cmiss_graphic_streamlines, cast_cpp)
{
	ZincTestSetupCpp zinc;

	Graphic gr = zinc.scene.createGraphic(Graphic::GRAPHIC_STREAMLINES);
	EXPECT_TRUE(gr.isValid());

	GraphicStreamlines st(gr);
	EXPECT_TRUE(st.isValid());

	GraphicPoints po(gr);
	EXPECT_FALSE(po.isValid());

	// try any base class API
	GraphicsMaterial material = st.getMaterial();
	ASSERT_TRUE(material.isValid());
}

TEST(Cmiss_graphic_streamlines, stream_vector_field)
{
	ZincTestSetup zinc;

	Cmiss_graphic_streamlines_id st = Cmiss_scene_create_graphic_streamlines(zinc.scene);
	EXPECT_NE(static_cast<Cmiss_graphic_streamlines *>(0), st);

	const double values[] = { 1.0, 2.0, 3.0 };
	Cmiss_field_id stream_vector_field = Cmiss_field_module_create_constant(zinc.fm,
		sizeof(values)/sizeof(double), values);
	EXPECT_NE(static_cast<Cmiss_field *>(0), stream_vector_field);

	EXPECT_EQ((Cmiss_field_id)0, Cmiss_graphic_streamlines_get_stream_vector_field(st));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_streamlines_set_stream_vector_field(st, stream_vector_field));

	Cmiss_field_id temp_stream_vector_field = Cmiss_graphic_streamlines_get_stream_vector_field(st);
	EXPECT_EQ(stream_vector_field, temp_stream_vector_field);
	Cmiss_field_destroy(&temp_stream_vector_field);
	Cmiss_field_destroy(&stream_vector_field);

	EXPECT_EQ(CMISS_OK, Cmiss_graphic_streamlines_set_stream_vector_field(st, 0));
	EXPECT_EQ(static_cast<Cmiss_field *>(0), Cmiss_graphic_streamlines_get_stream_vector_field(st));

	Cmiss_graphic_streamlines_destroy(&st);
}

TEST(Cmiss_graphic_streamlines, stream_vector_field_cpp)
{
	ZincTestSetupCpp zinc;

	GraphicStreamlines st = zinc.scene.createGraphicStreamlines();
	EXPECT_TRUE(st.isValid());

	Field tempStreamVectorField = st.getStreamVectorField();
	EXPECT_FALSE(tempStreamVectorField.isValid());

	const double values[] = { 1.0, 2.0, 3.0 };
	Field streamVectorField = zinc.fm.createConstant(sizeof(values)/sizeof(double), values);
	EXPECT_TRUE(streamVectorField.isValid());

	EXPECT_EQ(CMISS_OK, st.setStreamVectorField(streamVectorField));
	tempStreamVectorField = st.getStreamVectorField();
	EXPECT_EQ(streamVectorField.getId(), tempStreamVectorField.getId());

	Field noField;
	EXPECT_EQ(CMISS_OK, st.setStreamVectorField(noField));
	tempStreamVectorField = st.getStreamVectorField();
	EXPECT_FALSE(tempStreamVectorField.isValid());
}

TEST(Cmiss_graphic_streamlines, track_direction)
{
	ZincTestSetup zinc;

	Cmiss_graphic_streamlines_id st = Cmiss_scene_create_graphic_streamlines(zinc.scene);
	EXPECT_NE(static_cast<Cmiss_graphic_streamlines *>(0), st);

	EXPECT_EQ(CMISS_GRAPHIC_STREAMLINES_FORWARD_TRACK, Cmiss_graphic_streamlines_get_track_direction(st));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_streamlines_set_track_direction(st, CMISS_GRAPHIC_STREAMLINES_REVERSE_TRACK));
	EXPECT_EQ(CMISS_GRAPHIC_STREAMLINES_REVERSE_TRACK, Cmiss_graphic_streamlines_get_track_direction(st));

	Cmiss_graphic_streamlines_destroy(&st);
}

TEST(Cmiss_graphic_streamlines, track_direction_cpp)
{
	ZincTestSetupCpp zinc;

	GraphicStreamlines st = zinc.scene.createGraphicStreamlines();
	EXPECT_TRUE(st.isValid());

	EXPECT_EQ(GraphicStreamlines::FORWARD_TRACK, st.getTrackDirection());
	EXPECT_EQ(CMISS_OK, st.setTrackDirection(GraphicStreamlines::REVERSE_TRACK));
	EXPECT_EQ(GraphicStreamlines::REVERSE_TRACK, st.getTrackDirection());
}

TEST(Cmiss_graphic_streamlines, track_length)
{
	ZincTestSetup zinc;

	Cmiss_graphic_streamlines_id st = Cmiss_scene_create_graphic_streamlines(zinc.scene);
	EXPECT_NE(static_cast<Cmiss_graphic_streamlines *>(0), st);

	const double trackLength = 500.0;
	EXPECT_DOUBLE_EQ(1.0, Cmiss_graphic_streamlines_get_track_length(st));
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_streamlines_set_track_length(st, trackLength));
	EXPECT_DOUBLE_EQ(trackLength, Cmiss_graphic_streamlines_get_track_length(st));

	Cmiss_graphic_streamlines_destroy(&st);
}

TEST(Cmiss_graphic_streamlines, track_length_cpp)
{
	ZincTestSetupCpp zinc;

	GraphicStreamlines st = zinc.scene.createGraphicStreamlines();
	EXPECT_TRUE(st.isValid());

	const double trackLength = 500.0;
	EXPECT_DOUBLE_EQ(1.0, st.getTrackLength());
	EXPECT_EQ(CMISS_OK, st.setTrackLength(trackLength));
	EXPECT_DOUBLE_EQ(trackLength, st.getTrackLength());
}
