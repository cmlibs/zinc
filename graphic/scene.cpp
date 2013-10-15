
#include <gtest/gtest.h>

#include <zinc/zincconfigure.h>
#include <zinc/status.h>
#include <zinc/core.h>
#include <zinc/fieldarithmeticoperators.h>
#include <zinc/fieldcomposite.h>
#include <zinc/fieldconstant.h>
#include <zinc/fieldvectoroperators.h>
#include <zinc/graphics.h>
#include <zinc/region.h>
#include <zinc/spectrum.h>

#include <zinc/fieldarithmeticoperators.hpp>
#include <zinc/fieldcomposite.hpp>
#include <zinc/fieldconstant.hpp>
#include <zinc/fieldvectoroperators.hpp>
#include <zinc/graphics.hpp>
#include "zinc/scenefilter.hpp"
#include "zinc/spectrum.hpp"

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
	Field offsetYField = zinc.fm.createFieldAdd(yField, offsetField);
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
	Graphics noGraphics;
	ASSERT_EQ(CMZN_OK, result = zinc.scene.moveGraphicsBefore(surfaces, noGraphics));

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
	ASSERT_EQ(CMZN_ERROR_ARGUMENT, result = result = zinc.scene.moveGraphicsBefore(points, noGraphics));

	ASSERT_EQ(CMZN_OK, result = zinc.scene.removeAllGraphics());
}
