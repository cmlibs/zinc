/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include <opencmiss/zinc/status.h>
#include <opencmiss/zinc/core.h>
#include <opencmiss/zinc/fieldarithmeticoperators.h>
#include <opencmiss/zinc/fieldcomposite.h>
#include <opencmiss/zinc/fieldconstant.h>
#include <opencmiss/zinc/fieldvectoroperators.h>
#include <opencmiss/zinc/graphics.h>
#include <opencmiss/zinc/region.h>
#include <opencmiss/zinc/scenefilter.h>
#include <opencmiss/zinc/streamscene.h>
#include <opencmiss/zinc/stream.h>
#include <opencmiss/zinc/sceneviewer.h>
#include <opencmiss/zinc/spectrum.h>

#include <opencmiss/zinc/fieldarithmeticoperators.hpp>
#include <opencmiss/zinc/fieldcomposite.hpp>
#include <opencmiss/zinc/fieldconstant.hpp>
#include <opencmiss/zinc/fieldvectoroperators.hpp>
#include <opencmiss/zinc/graphics.hpp>
#include <opencmiss/zinc/scene.hpp>
#include <opencmiss/zinc/scenefilter.hpp>
#include <opencmiss/zinc/sceneviewer.hpp>
#include <opencmiss/zinc/spectrum.hpp>
#include <opencmiss/zinc/streamscene.hpp>

#include "test_resources.h"
#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"

class ZincTestSetupSpectrum : public ZincTestSetup
{
public:
	cmzn_spectrummodule_id spectrummodule;
	cmzn_spectrum_id defaultSpectrum;

	ZincTestSetupSpectrum() :
		ZincTestSetup(),
		spectrummodule(cmzn_context_get_spectrummodule(context)),
		defaultSpectrum(cmzn_spectrummodule_get_default_spectrum(spectrummodule))
	{
		EXPECT_NE(static_cast<cmzn_spectrummodule *>(0), this->spectrummodule);
		EXPECT_NE(static_cast<cmzn_spectrum *>(0), this->defaultSpectrum);
	}

	~ZincTestSetupSpectrum()
	{
		cmzn_spectrummodule_destroy(&spectrummodule);
		cmzn_spectrum_destroy(&defaultSpectrum);
	}
};

class ZincTestSetupSpectrumCpp : public ZincTestSetupCpp
{
public:
	Spectrummodule spectrummodule;
	Spectrum defaultSpectrum;

	ZincTestSetupSpectrumCpp() :
		ZincTestSetupCpp(),
		spectrummodule(context.getSpectrummodule()),
		defaultSpectrum(spectrummodule.getDefaultSpectrum())
	{
		EXPECT_TRUE(this->spectrummodule.isValid());
		EXPECT_TRUE(this->defaultSpectrum.isValid());
	}

	~ZincTestSetupSpectrumCpp()
	{
	}
};

TEST(cmzn_scene, get_modules)
{
	ZincTestSetup zinc;

	cmzn_fontmodule_id fontmodule = cmzn_scene_get_fontmodule(zinc.scene);
	EXPECT_NE(static_cast<cmzn_fontmodule_id>(0), fontmodule);
	cmzn_fontmodule_destroy(&fontmodule);

	cmzn_glyphmodule_id glyphmodule = cmzn_scene_get_glyphmodule(zinc.scene);
	EXPECT_NE(static_cast<cmzn_glyphmodule_id>(0), glyphmodule);
	cmzn_glyphmodule_destroy(&glyphmodule);

	cmzn_materialmodule_id materialmodule = cmzn_scene_get_materialmodule(zinc.scene);
	EXPECT_NE(static_cast<cmzn_materialmodule_id>(0), materialmodule);
	cmzn_materialmodule_destroy(&materialmodule);

	cmzn_scenefiltermodule_id scenefiltermodule = cmzn_scene_get_scenefiltermodule(zinc.scene);
	EXPECT_NE(static_cast<cmzn_scenefiltermodule_id>(0), scenefiltermodule);
	cmzn_scenefiltermodule_destroy(&scenefiltermodule);

	cmzn_sceneviewermodule_id sceneviewermodule = cmzn_scene_get_sceneviewermodule(zinc.scene);
	EXPECT_NE(static_cast<cmzn_sceneviewermodule_id>(0), sceneviewermodule);
	cmzn_sceneviewermodule_destroy(&sceneviewermodule);

	cmzn_spectrummodule_id spectrummodule = cmzn_scene_get_spectrummodule(zinc.scene);
	EXPECT_NE(static_cast<cmzn_spectrummodule_id>(0), spectrummodule);
	cmzn_spectrummodule_destroy(&spectrummodule);

	cmzn_tessellationmodule_id tessellationmodule = cmzn_scene_get_tessellationmodule(zinc.scene);
	EXPECT_NE(static_cast<cmzn_tessellationmodule_id>(0), tessellationmodule);
	cmzn_tessellationmodule_destroy(&tessellationmodule);

	cmzn_timekeepermodule_id timekeepermodule = cmzn_scene_get_timekeepermodule(zinc.scene);
	EXPECT_NE(static_cast<cmzn_timekeepermodule_id>(0), timekeepermodule);
	cmzn_timekeepermodule_destroy(&timekeepermodule);
}

TEST(ZincScene, getModules)
{
	ZincTestSetupCpp zinc;

	EXPECT_TRUE(zinc.scene.getFontmodule().isValid());
	EXPECT_TRUE(zinc.scene.getGlyphmodule().isValid());
	EXPECT_TRUE(zinc.scene.getMaterialmodule().isValid());
	EXPECT_TRUE(zinc.scene.getScenefiltermodule().isValid());
	EXPECT_TRUE(zinc.scene.getSceneviewermodule().isValid());
	EXPECT_TRUE(zinc.scene.getSpectrummodule().isValid());
	EXPECT_TRUE(zinc.scene.getTessellationmodule().isValid());
	EXPECT_TRUE(zinc.scene.getTimekeepermodule().isValid());
}

TEST(cmzn_scene, get_region)
{
	ZincTestSetup zinc;

	cmzn_region_id region = cmzn_scene_get_region(zinc.scene);
	EXPECT_EQ(zinc.root_region, region);
	cmzn_region_destroy(&region);
}

TEST(ZincScene, getRegion)
{
	ZincTestSetupCpp zinc;

	EXPECT_EQ(zinc.root_region, zinc.scene.getRegion());
}

TEST(cmzn_scene, get_spectrum_data_range)
{
	ZincTestSetupSpectrum zinc;

	int result;

	EXPECT_EQ(CMZN_OK, result = cmzn_region_read_file(zinc.root_region, TestResources::getLocation(TestResources::FIELDMODULE_CUBE_RESOURCE)));

	cmzn_field_id coordinateField = cmzn_fieldmodule_find_field_by_name(zinc.fm, "coordinates");
	EXPECT_NE(static_cast<cmzn_field_id>(0), coordinateField);
	cmzn_field_id magnitudeField = cmzn_fieldmodule_create_field_magnitude(zinc.fm, coordinateField);
	EXPECT_NE(static_cast<cmzn_field_id>(0), magnitudeField);
	const double offset = -0.5;
	cmzn_field_id offsetField = cmzn_fieldmodule_create_field_constant(zinc.fm, 1, &offset);
	EXPECT_NE(static_cast<cmzn_field_id>(0), offsetField);
	cmzn_field_id yField = cmzn_fieldmodule_create_field_component(zinc.fm, coordinateField, 2);
	EXPECT_NE(static_cast<cmzn_field_id>(0), yField);
	cmzn_field_id offsetYField = cmzn_fieldmodule_create_field_add(zinc.fm, yField, offsetField);
	EXPECT_NE(static_cast<cmzn_field_id>(0), offsetYField);
	cmzn_field_id sourceFields[] = { magnitudeField, offsetYField };
	cmzn_field_id dataField = cmzn_fieldmodule_create_field_concatenate(zinc.fm, 2, sourceFields);
	EXPECT_NE(static_cast<cmzn_field_id>(0), dataField);

	cmzn_graphics_id gr = cmzn_scene_create_graphics_surfaces(zinc.scene);
	EXPECT_NE(static_cast<cmzn_graphics_id>(0), gr);
	EXPECT_EQ(CMZN_OK, result = cmzn_graphics_set_coordinate_field(gr, coordinateField));
	EXPECT_EQ(CMZN_OK, result = cmzn_graphics_set_data_field(gr, dataField));
	EXPECT_EQ(CMZN_OK, result = cmzn_graphics_set_spectrum(gr, zinc.defaultSpectrum));

	double minimumValues[3], maximumValues[3];
	int maxRanges = cmzn_scene_get_spectrum_data_range(zinc.scene, static_cast<cmzn_scenefilter_id>(0),
		zinc.defaultSpectrum, 3, minimumValues, maximumValues);
	EXPECT_EQ(2, maxRanges);
	ASSERT_DOUBLE_EQ(0.0, minimumValues[0]);
	ASSERT_DOUBLE_EQ(1.7320508f, maximumValues[0]);
	ASSERT_DOUBLE_EQ(-0.5, minimumValues[1]);
	ASSERT_DOUBLE_EQ(0.5, maximumValues[1]);

	EXPECT_EQ(CMZN_OK, result = cmzn_graphics_set_data_field(gr, offsetYField));
	maxRanges = cmzn_scene_get_spectrum_data_range(zinc.scene, static_cast<cmzn_scenefilter_id>(0),
		zinc.defaultSpectrum, 3, minimumValues, maximumValues);
	EXPECT_EQ(1, maxRanges);
	ASSERT_DOUBLE_EQ(-0.5, minimumValues[0]);
	ASSERT_DOUBLE_EQ(0.5, maximumValues[0]);

	cmzn_graphics_destroy(&gr);
	cmzn_field_destroy(&dataField);
	cmzn_field_destroy(&offsetYField);
	cmzn_field_destroy(&yField);
	cmzn_field_destroy(&offsetField);
	cmzn_field_destroy(&magnitudeField);
	cmzn_field_destroy(&coordinateField);
}

TEST(ZincScene, getSpectrumDataRange)
{
	ZincTestSetupSpectrumCpp zinc;

	int result;

	EXPECT_EQ(CMZN_OK, result = zinc.root_region.readFile(TestResources::getLocation(TestResources::FIELDMODULE_CUBE_RESOURCE)));

	Field coordinateField = zinc.fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinateField.isValid());
	Field magnitudeField = zinc.fm.createFieldMagnitude(coordinateField);
	EXPECT_TRUE(magnitudeField.isValid());
	const double offset = -0.5;
	Field offsetField = zinc.fm.createFieldConstant(1, &offset);
	EXPECT_TRUE(offsetField.isValid());
	Field yField = zinc.fm.createFieldComponent(coordinateField, 2);
	EXPECT_TRUE(yField.isValid());
	// test operator overloading for addition:
	Field offsetYField = yField + offsetField;
	EXPECT_TRUE(offsetYField.isValid());
	Field sourceFields[] = { magnitudeField, offsetYField };
	Field dataField = zinc.fm.createFieldConcatenate(2, sourceFields);
	EXPECT_TRUE(dataField.isValid());

	Graphics gr = zinc.scene.createGraphicsSurfaces();
	EXPECT_TRUE(gr.isValid());
	EXPECT_EQ(CMZN_OK, result = gr.setCoordinateField(coordinateField));
	EXPECT_EQ(CMZN_OK, result = gr.setDataField(dataField));
	EXPECT_EQ(CMZN_OK, result = gr.setSpectrum(zinc.defaultSpectrum));

	double minimumValues[3], maximumValues[3];
	Scenefiltermodule sfm = zinc.context.getScenefiltermodule();
	Scenefilter defaultFilter = sfm.getDefaultScenefilter();

	int maxRanges = zinc.scene.getSpectrumDataRange(defaultFilter,
		zinc.defaultSpectrum, 3, minimumValues, maximumValues);
	EXPECT_EQ(2, maxRanges);
	ASSERT_DOUBLE_EQ(0.0, minimumValues[0]);
	ASSERT_DOUBLE_EQ(1.7320508f, maximumValues[0]);
	ASSERT_DOUBLE_EQ(-0.5, minimumValues[1]);
	ASSERT_DOUBLE_EQ(0.5, maximumValues[1]);

	EXPECT_EQ(CMZN_OK, result = gr.setDataField(offsetYField));
	maxRanges = zinc.scene.getSpectrumDataRange(defaultFilter,
		zinc.defaultSpectrum, 3, minimumValues, maximumValues);
	EXPECT_EQ(1, maxRanges);
	ASSERT_DOUBLE_EQ(-0.5, minimumValues[0]);
	ASSERT_DOUBLE_EQ(0.5, maximumValues[0]);
}

TEST(cmzn_scene, visibility_flag)
{
	ZincTestSetup zinc;
	EXPECT_TRUE(cmzn_scene_get_visibility_flag(zinc.scene));
	EXPECT_EQ(CMZN_OK, cmzn_scene_set_visibility_flag(zinc.scene, false));
	EXPECT_FALSE(cmzn_scene_get_visibility_flag(zinc.scene));
}

TEST(ZincScene, visibilityFlag)
{
	ZincTestSetupCpp zinc;
	EXPECT_TRUE(zinc.scene.getVisibilityFlag());
	EXPECT_EQ(CMZN_OK, zinc.scene.setVisibilityFlag(false));
	EXPECT_FALSE(zinc.scene.getVisibilityFlag());
}

TEST(cmzn_scene, graphics_list)
{
	ZincTestSetup zinc;
	cmzn_graphics_id lines = cmzn_scene_create_graphics_lines(zinc.scene);
	EXPECT_NE(static_cast<cmzn_graphics_id>(0), lines);
	cmzn_graphics_id points = cmzn_scene_create_graphics_points(zinc.scene);
	EXPECT_NE(static_cast<cmzn_graphics_id>(0), points);
	cmzn_graphics_id surfaces = cmzn_scene_create_graphics_surfaces(zinc.scene);
	EXPECT_NE(static_cast<cmzn_graphics_id>(0), surfaces);

	cmzn_graphics_id gr = 0;
	gr = cmzn_scene_get_first_graphics(zinc.scene);
	ASSERT_EQ(lines, gr);
	cmzn_graphics_destroy(&gr);
	gr = cmzn_scene_get_next_graphics(zinc.scene, lines);
	ASSERT_EQ(points, gr);
	cmzn_graphics_destroy(&gr);
	gr = cmzn_scene_get_next_graphics(zinc.scene, points);
	ASSERT_EQ(surfaces, gr);
	cmzn_graphics_destroy(&gr);
	gr = cmzn_scene_get_next_graphics(zinc.scene, surfaces);
	ASSERT_EQ(static_cast<cmzn_graphics_id>(0), gr);

	// reverse iteration
	gr = cmzn_scene_get_previous_graphics(zinc.scene, points);
	ASSERT_EQ(lines, gr);
	cmzn_graphics_destroy(&gr);
	gr = cmzn_scene_get_previous_graphics(zinc.scene, lines);
	ASSERT_EQ(static_cast<cmzn_graphics_id>(0), gr);

	int result;
	ASSERT_EQ(CMZN_OK, result = cmzn_scene_move_graphics_before(zinc.scene, surfaces, lines));

	gr = cmzn_scene_get_first_graphics(zinc.scene);
	ASSERT_EQ(surfaces, gr);
	cmzn_graphics_destroy(&gr);
	gr = cmzn_scene_get_next_graphics(zinc.scene, surfaces);
	ASSERT_EQ(lines, gr);
	cmzn_graphics_destroy(&gr);
	gr = cmzn_scene_get_next_graphics(zinc.scene, lines);
	ASSERT_EQ(points, gr);
	cmzn_graphics_destroy(&gr);
	gr = cmzn_scene_get_next_graphics(zinc.scene, points);
	ASSERT_EQ(static_cast<cmzn_graphics_id>(0), gr);

	// move to end of list
	ASSERT_EQ(CMZN_OK, result = cmzn_scene_move_graphics_before(zinc.scene, surfaces, static_cast<cmzn_graphics_id>(0)));

	gr = cmzn_scene_get_first_graphics(zinc.scene);
	ASSERT_EQ(lines, gr);
	cmzn_graphics_destroy(&gr);
	gr = cmzn_scene_get_next_graphics(zinc.scene, lines);
	ASSERT_EQ(points, gr);
	cmzn_graphics_destroy(&gr);
	gr = cmzn_scene_get_next_graphics(zinc.scene, points);
	ASSERT_EQ(surfaces, gr);
	cmzn_graphics_destroy(&gr);
	gr = cmzn_scene_get_next_graphics(zinc.scene, surfaces);
	ASSERT_EQ(static_cast<cmzn_graphics_id>(0), gr);

	ASSERT_EQ(CMZN_OK, result = cmzn_scene_remove_graphics(zinc.scene, points));

	gr = cmzn_scene_get_first_graphics(zinc.scene);
	ASSERT_EQ(lines, gr);
	cmzn_graphics_destroy(&gr);
	gr = cmzn_scene_get_next_graphics(zinc.scene, lines);
	ASSERT_EQ(surfaces, gr);
	cmzn_graphics_destroy(&gr);
	gr = cmzn_scene_get_next_graphics(zinc.scene, surfaces);
	ASSERT_EQ(static_cast<cmzn_graphics_id>(0), gr);

	// can't re-add points graphics that has been removed
	ASSERT_EQ(CMZN_ERROR_ARGUMENT, result = cmzn_scene_move_graphics_before(zinc.scene, points, static_cast<cmzn_graphics_id>(0)));

	ASSERT_EQ(CMZN_OK, result = cmzn_scene_remove_all_graphics(zinc.scene));

	cmzn_graphics_destroy(&lines);
	cmzn_graphics_destroy(&points);
	cmzn_graphics_destroy(&surfaces);
}

TEST(ZincScene, graphics_list)
{
	ZincTestSetupCpp zinc;
	GraphicsLines lines = zinc.scene.createGraphicsLines();
	EXPECT_TRUE(lines.isValid());
	GraphicsPoints points = zinc.scene.createGraphicsPoints();
	EXPECT_TRUE(points.isValid());
	GraphicsSurfaces surfaces = zinc.scene.createGraphicsSurfaces();
	EXPECT_TRUE(surfaces.isValid());

	Graphics gr;
	gr = zinc.scene.getFirstGraphics();
	ASSERT_EQ(lines.getId(), gr.getId());
	gr = zinc.scene.getNextGraphics(gr);
	ASSERT_EQ(points.getId(), gr.getId());
	gr = zinc.scene.getNextGraphics(gr);
	ASSERT_EQ(surfaces.getId(), gr.getId());
	gr = zinc.scene.getNextGraphics(gr);
	ASSERT_FALSE(gr.isValid());

	// reverse iteration
	gr = zinc.scene.getPreviousGraphics(points);
	ASSERT_EQ(lines.getId(), gr.getId());
	gr = zinc.scene.getPreviousGraphics(gr);
	ASSERT_FALSE(gr.isValid());

	int result;
	ASSERT_EQ(CMZN_OK, result = zinc.scene.moveGraphicsBefore(surfaces, lines));

	gr = zinc.scene.getFirstGraphics();
	ASSERT_EQ(surfaces.getId(), gr.getId());
	gr = zinc.scene.getNextGraphics(gr);
	ASSERT_EQ(lines.getId(), gr.getId());
	gr = zinc.scene.getNextGraphics(gr);
	ASSERT_EQ(points.getId(), gr.getId());
	gr = zinc.scene.getNextGraphics(gr);
	ASSERT_FALSE(gr.isValid());

	// move to end of list
	ASSERT_EQ(CMZN_OK, result = zinc.scene.moveGraphicsBefore(surfaces, Graphics()));

	gr = zinc.scene.getFirstGraphics();
	ASSERT_EQ(lines.getId(), gr.getId());
	gr = zinc.scene.getNextGraphics(gr);
	ASSERT_EQ(points.getId(), gr.getId());
	gr = zinc.scene.getNextGraphics(gr);
	ASSERT_EQ(surfaces.getId(), gr.getId());
	gr = zinc.scene.getNextGraphics(gr);
	ASSERT_FALSE(gr.isValid());

	ASSERT_EQ(CMZN_OK, result = zinc.scene.removeGraphics(points));

	gr = zinc.scene.getFirstGraphics();
	ASSERT_EQ(lines.getId(), gr.getId());
	gr = zinc.scene.getNextGraphics(gr);
	ASSERT_EQ(surfaces.getId(), gr.getId());
	gr = zinc.scene.getNextGraphics(gr);
	ASSERT_FALSE(gr.isValid());

	// can't re-add points graphics that has been removed
	ASSERT_EQ(CMZN_ERROR_ARGUMENT, result = zinc.scene.moveGraphicsBefore(points, Graphics()));

	ASSERT_EQ(CMZN_OK, result = zinc.scene.removeAllGraphics());
}

TEST(cmzn_scene, threejs_export_cpp)
{
	ZincTestSetupCpp zinc;

	int result;

	EXPECT_EQ(CMZN_OK, result = zinc.root_region.readFile(TestResources::getLocation(TestResources::FIELDMODULE_CUBE_RESOURCE)));

	GraphicsSurfaces surfaces = zinc.scene.createGraphicsSurfaces();
	EXPECT_TRUE(surfaces.isValid());

	Field coordinateField = zinc.fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinateField.isValid());

	EXPECT_EQ(CMZN_OK, result = surfaces.setCoordinateField(coordinateField));

	StreaminformationScene si = zinc.scene.createStreaminformationScene();
	EXPECT_TRUE(si.isValid());

	EXPECT_EQ(CMZN_OK, result = si.setIOFormat(si.IO_FORMAT_THREEJS));

	EXPECT_EQ(1, result = si.getNumberOfResourcesRequired());

	EXPECT_EQ(0, result = si.getNumberOfTimeSteps());

	double double_result = 0.0;
	EXPECT_EQ(0.0, double_result = si.getInitialTime());
	EXPECT_EQ(0.0, double_result = si.getFinishTime());

	EXPECT_EQ(CMZN_OK, result = si.setIODataType(si.IO_DATA_TYPE_COLOUR));

	StreamresourceMemory memeory_sr = si.createStreamresourceMemory();

	EXPECT_EQ(CMZN_OK, result = zinc.scene.write(si));

	char *memory_buffer;
	unsigned int size = 0;

	result = memeory_sr.getBuffer((void**)&memory_buffer, &size);
	EXPECT_EQ(CMZN_OK, result);

	char *temp_char = strstr ( memory_buffer, "vertices");
	EXPECT_NE(static_cast<char *>(0), temp_char);

	temp_char = strstr ( memory_buffer, "faces");
	EXPECT_NE(static_cast<char *>(0), temp_char);
}

TEST(cmzn_scene, threejs_export)
{
	ZincTestSetup zinc;

	int result;

	EXPECT_EQ(CMZN_OK, result = cmzn_region_read_file(zinc.root_region, TestResources::getLocation(TestResources::FIELDMODULE_CUBE_RESOURCE)));

	cmzn_graphics_id surfaces = cmzn_scene_create_graphics_surfaces(zinc.scene);
	EXPECT_NE(static_cast<cmzn_graphics_id>(0), surfaces);

	cmzn_field_id coordinateField = cmzn_fieldmodule_find_field_by_name(zinc.fm, "coordinates");
	EXPECT_NE(static_cast<cmzn_field_id>(0), coordinateField);

	EXPECT_EQ(CMZN_OK, result = cmzn_graphics_set_coordinate_field(surfaces, coordinateField));

	cmzn_streaminformation_id streaminformation = cmzn_scene_create_streaminformation_scene(zinc.scene);
	EXPECT_NE(static_cast<cmzn_streaminformation_id>(0), streaminformation);

	cmzn_streaminformation_scene_id scene_si = cmzn_streaminformation_cast_scene(streaminformation);
	EXPECT_NE(static_cast<cmzn_streaminformation_scene_id>(0), scene_si);

	EXPECT_EQ(CMZN_OK, result = cmzn_streaminformation_scene_set_io_format(
		scene_si, CMZN_STREAMINFORMATION_SCENE_IO_FORMAT_THREEJS));

	EXPECT_EQ(1, result = cmzn_streaminformation_scene_get_number_of_resources_required(scene_si));
	EXPECT_EQ(0, result = cmzn_streaminformation_scene_get_number_of_time_steps(scene_si));

	double double_result = 0.0;
	EXPECT_EQ(0.0, double_result = cmzn_streaminformation_scene_get_initial_time(scene_si));
	EXPECT_EQ(0.0, double_result = cmzn_streaminformation_scene_get_finish_time(scene_si));

	EXPECT_EQ(CMZN_OK, result = cmzn_streaminformation_scene_set_io_data_type(
		scene_si, CMZN_STREAMINFORMATION_SCENE_IO_DATA_TYPE_COLOUR));

	cmzn_streamresource_id data_sr = cmzn_streaminformation_create_streamresource_memory(streaminformation);

	EXPECT_EQ(CMZN_OK, result = cmzn_scene_write(zinc.scene, scene_si));

	cmzn_streamresource_memory_id memeory_sr = cmzn_streamresource_cast_memory(
		data_sr);

	char *memory_buffer;
	unsigned int size = 0;

	result = cmzn_streamresource_memory_get_buffer(memeory_sr, (void**)&memory_buffer, &size);
	EXPECT_EQ(CMZN_OK, result);

	char *temp_char = strstr ( memory_buffer, "vertices");
	EXPECT_NE(static_cast<char *>(0), temp_char);

	temp_char = strstr ( memory_buffer, "faces");
	EXPECT_NE(static_cast<char *>(0), temp_char);

	cmzn_field_destroy(&coordinateField);
	cmzn_streamresource_destroy(&data_sr);
	data_sr = cmzn_streamresource_memory_base_cast(memeory_sr);
	cmzn_streamresource_destroy(&data_sr);
	cmzn_streaminformation_scene_destroy(&scene_si);
	cmzn_streaminformation_destroy(&streaminformation);
	cmzn_graphics_destroy(&surfaces);
}

TEST(cmzn_scene, graphics_description_cpp)
{
	ZincTestSetupCpp zinc;

	int result;

	EXPECT_EQ(CMZN_OK, result = zinc.root_region.readFile(TestResources::getLocation(TestResources::FIELDMODULE_CUBE_RESOURCE)));

	GraphicsSurfaces surfaces = zinc.scene.createGraphicsSurfaces();
	EXPECT_TRUE(surfaces.isValid());

	GraphicsPoints points = zinc.scene.createGraphicsPoints();
	EXPECT_TRUE(points.isValid());

	Field coordinateField = zinc.fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinateField.isValid());

	EXPECT_EQ(CMZN_OK, result = surfaces.setCoordinateField(coordinateField));

	StreaminformationScene si = zinc.scene.createStreaminformationScene();
	EXPECT_TRUE(si.isValid());

	EXPECT_EQ(CMZN_OK, result = si.setIOFormat(si.IO_FORMAT_DESCRIPTION));

	EXPECT_EQ(1, result = si.getNumberOfResourcesRequired());

	StreamresourceMemory memeory_sr = si.createStreamresourceMemory();

	EXPECT_EQ(CMZN_OK, result = zinc.scene.write(si));

	char *memory_buffer;
	unsigned int size = 0;

	result = memeory_sr.getBuffer((void**)&memory_buffer, &size);
	EXPECT_EQ(CMZN_OK, result);

	char *temp_char = strstr ( memory_buffer, "SURFACES");
	EXPECT_NE(static_cast<char *>(0), temp_char);

	temp_char = strstr ( memory_buffer, "POINTS");
	EXPECT_NE(static_cast<char *>(0), temp_char);

	temp_char = strstr ( memory_buffer, "\"BaseSize\" : [ 0, 0, 0 ],");
	EXPECT_NE(static_cast<char *>(0), temp_char);

	EXPECT_EQ(CMZN_OK, result = zinc.scene.removeGraphics(points));

	StreaminformationScene si2 = zinc.scene.createStreaminformationScene();
	EXPECT_TRUE(si2.isValid());

	StreamresourceMemory memeory_sr2 = si2.createStreamresourceMemoryBuffer((void *)memory_buffer, size);
	EXPECT_TRUE(memeory_sr2.isValid());

	EXPECT_EQ(CMZN_OK, result = si2.setIOFormat(si.IO_FORMAT_DESCRIPTION));

	EXPECT_EQ(CMZN_OK, result = si2.setOverwriteSceneGraphics(1));

	EXPECT_EQ(CMZN_OK, result = zinc.scene.read(si2));

	EXPECT_EQ(2, result = zinc.scene.getNumberOfGraphics());

}

TEST(cmzn_scene, graphics_description)
{
	ZincTestSetup zinc;

	int result;

	EXPECT_EQ(CMZN_OK, result = cmzn_region_read_file(zinc.root_region, TestResources::getLocation(TestResources::FIELDMODULE_CUBE_RESOURCE)));

	cmzn_graphics_id surfaces = cmzn_scene_create_graphics_surfaces(zinc.scene);
	EXPECT_NE(static_cast<cmzn_graphics_id>(0), surfaces);

	cmzn_graphics_id points = cmzn_scene_create_graphics_points(zinc.scene);
	EXPECT_NE(static_cast<cmzn_graphics_id>(0), points);

	cmzn_field_id coordinateField = cmzn_fieldmodule_find_field_by_name(zinc.fm, "coordinates");
	EXPECT_NE(static_cast<cmzn_field_id>(0), coordinateField);

	EXPECT_EQ(CMZN_OK, result = cmzn_graphics_set_coordinate_field(surfaces, coordinateField));

	cmzn_streaminformation_id streaminformation = cmzn_scene_create_streaminformation_scene(zinc.scene);
	EXPECT_NE(static_cast<cmzn_streaminformation_id>(0), streaminformation);

	cmzn_streaminformation_scene_id scene_si = cmzn_streaminformation_cast_scene(streaminformation);
	EXPECT_NE(static_cast<cmzn_streaminformation_scene_id>(0), scene_si);

	EXPECT_EQ(CMZN_OK, result = cmzn_streaminformation_scene_set_io_format(
		scene_si, CMZN_STREAMINFORMATION_SCENE_IO_FORMAT_DESCRIPTION));

	EXPECT_EQ(1, result = cmzn_streaminformation_scene_get_number_of_resources_required(scene_si));

	cmzn_streamresource_id data_sr = cmzn_streaminformation_create_streamresource_memory(streaminformation);

	EXPECT_EQ(CMZN_OK, result = cmzn_scene_write(zinc.scene, scene_si));

	cmzn_streamresource_memory_id memeory_sr = cmzn_streamresource_cast_memory(
		data_sr);

	char *memory_buffer;
	unsigned int size = 0;

	result = cmzn_streamresource_memory_get_buffer(memeory_sr, (void**)&memory_buffer, &size);
	EXPECT_EQ(CMZN_OK, result);

	char *temp_char = strstr ( memory_buffer, "SURFACES");
	EXPECT_NE(static_cast<char *>(0), temp_char);

	temp_char = strstr ( memory_buffer, "POINTS");
	EXPECT_NE(static_cast<char *>(0), temp_char);

	cmzn_field_destroy(&coordinateField);
	cmzn_streamresource_destroy(&data_sr);
	data_sr = cmzn_streamresource_memory_base_cast(memeory_sr);
	cmzn_streamresource_destroy(&data_sr);
	cmzn_streaminformation_scene_destroy(&scene_si);
	cmzn_streaminformation_destroy(&streaminformation);
	cmzn_graphics_destroy(&surfaces);
	cmzn_graphics_destroy(&points);
}

TEST(cmzn_scene, new_region_has_scene)
{
	ZincTestSetup zinc;

	cmzn_scene *root_scene = cmzn_region_get_scene(zinc.root_region);
	EXPECT_NE(static_cast<cmzn_scene*>(0), root_scene);
	cmzn_scene_destroy(&root_scene);

	cmzn_region *region1 = cmzn_context_create_region(zinc.context);
	EXPECT_NE(static_cast<cmzn_region*>(0), region1);
	cmzn_scene *scene1 = cmzn_region_get_scene(region1);
	EXPECT_NE(static_cast<cmzn_scene*>(0), scene1);

	cmzn_region *region2 = cmzn_region_create_region(region1);
	EXPECT_NE(static_cast<cmzn_region*>(0), region2);
	cmzn_scene *scene2 = cmzn_region_get_scene(region2);
	EXPECT_NE(static_cast<cmzn_scene*>(0), scene2);

	cmzn_scene_destroy(&scene2);
	cmzn_region_destroy(&region2);
	cmzn_scene_destroy(&scene1);
	cmzn_region_destroy(&region1);
}

TEST(ZincScene, issue_3954_adding_child_region_destroys_its_scene)
{
	ZincTestSetupCpp zinc;

	Region r1 = zinc.root_region.createChild("bob");
	EXPECT_TRUE(r1.isValid());
	Region r2 = zinc.root_region.createRegion();
	EXPECT_TRUE(r2.isValid());
	EXPECT_EQ(OK, r2.setName("fred"));

	Scene s2 = r2.getScene();
	EXPECT_TRUE(s2.isValid());
	Graphics gr = s2.createGraphicsPoints();
	EXPECT_TRUE(gr.isValid());
	EXPECT_EQ(1, s2.getNumberOfGraphics());
	zinc.root_region.insertChildBefore(r2, r1);
	Scene s2b = r2.getScene();
	EXPECT_EQ(s2, s2b);
	EXPECT_EQ(1, s2b.getNumberOfGraphics());
}
