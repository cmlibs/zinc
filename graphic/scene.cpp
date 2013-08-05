
#include <gtest/gtest.h>

#include <zinc/zincconfigure.h>
#include <zinc/status.h>
#include <zinc/core.h>
#include <zinc/fieldarithmeticoperators.h>
#include <zinc/fieldcomposite.h>
#include <zinc/fieldconstant.h>
#include <zinc/fieldvectoroperators.h>
#include <zinc/graphic.h>
#include <zinc/region.h>
#include <zinc/spectrum.h>

#include <zinc/fieldtypesarithmeticoperators.hpp>
#include <zinc/fieldtypescomposite.hpp>
#include <zinc/fieldtypesconstant.hpp>
#include <zinc/fieldtypesvectoroperators.hpp>
#include <zinc/graphic.hpp>
#include "zinc/spectrum.hpp"

#include "test_resources.h"
#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"

class ZincTestSetupSpectrum : public ZincTestSetup
{
public:
	Cmiss_spectrum_module_id spectrumModule;
	Cmiss_spectrum_id defaultSpectrum;

	ZincTestSetupSpectrum() :
		ZincTestSetup(),
		spectrumModule(Cmiss_graphics_module_get_spectrum_module(gm)),
		defaultSpectrum(Cmiss_spectrum_module_get_default_spectrum(spectrumModule))
	{
		EXPECT_NE(static_cast<Cmiss_spectrum_module *>(0), this->spectrumModule);
		EXPECT_NE(static_cast<Cmiss_spectrum *>(0), this->defaultSpectrum);
	}

	~ZincTestSetupSpectrum()
	{
		Cmiss_spectrum_module_destroy(&spectrumModule);
		Cmiss_spectrum_destroy(&defaultSpectrum);
	}
};

class ZincTestSetupSpectrumCpp : public ZincTestSetupCpp
{
public:
	SpectrumModule spectrumModule;
	Spectrum defaultSpectrum;

	ZincTestSetupSpectrumCpp() :
		ZincTestSetupCpp(),
		spectrumModule(gm.getSpectrumModule()),
		defaultSpectrum(spectrumModule.getDefaultSpectrum())
	{
		EXPECT_TRUE(this->spectrumModule.isValid());
		EXPECT_TRUE(this->defaultSpectrum.isValid());
	}

	~ZincTestSetupSpectrumCpp()
	{
	}
};

TEST(Cmiss_scene, get_spectrum_data_range)
{
	ZincTestSetupSpectrum zinc;

	int result;

	EXPECT_EQ(CMISS_OK, result = Cmiss_region_read_file(zinc.root_region, TestResources::getLocation(TestResources::FIELDMODULE_CUBE_RESOURCE)));

	Cmiss_field_id coordinateField = Cmiss_field_module_find_field_by_name(zinc.fm, "coordinates");
	EXPECT_NE(static_cast<Cmiss_field_id>(0), coordinateField);
	Cmiss_field_id magnitudeField = Cmiss_field_module_create_magnitude(zinc.fm, coordinateField);
	EXPECT_NE(static_cast<Cmiss_field_id>(0), magnitudeField);
	const double offset = -0.5;
	Cmiss_field_id offsetField = Cmiss_field_module_create_constant(zinc.fm, 1, &offset);
	EXPECT_NE(static_cast<Cmiss_field_id>(0), offsetField);
	Cmiss_field_id yField = Cmiss_field_module_create_component(zinc.fm, coordinateField, 2);
	EXPECT_NE(static_cast<Cmiss_field_id>(0), yField);
	Cmiss_field_id offsetYField = Cmiss_field_module_create_add(zinc.fm, yField, offsetField);
	EXPECT_NE(static_cast<Cmiss_field_id>(0), offsetYField);
	Cmiss_field_id sourceFields[] = { magnitudeField, offsetYField };
	Cmiss_field_id dataField = Cmiss_field_module_create_concatenate(zinc.fm, 2, sourceFields);
	EXPECT_NE(static_cast<Cmiss_field_id>(0), dataField);

	Cmiss_graphic_id gr = Cmiss_graphic_surfaces_base_cast(Cmiss_scene_create_graphic_surfaces(zinc.scene));
	EXPECT_NE(static_cast<Cmiss_graphic_id>(0), gr);
	EXPECT_EQ(CMISS_OK, result = Cmiss_graphic_set_coordinate_field(gr, coordinateField));
	EXPECT_EQ(CMISS_OK, result = Cmiss_graphic_set_data_field(gr, dataField));
	EXPECT_EQ(CMISS_OK, result = Cmiss_graphic_set_spectrum(gr, zinc.defaultSpectrum));

	double minimumValues[3], maximumValues[3];
	int maxRanges = Cmiss_scene_get_spectrum_data_range(zinc.scene, static_cast<Cmiss_graphics_filter_id>(0),
		zinc.defaultSpectrum, 3, minimumValues, maximumValues);
	EXPECT_EQ(2, maxRanges);
	ASSERT_DOUBLE_EQ(0.0, minimumValues[0]);
	ASSERT_DOUBLE_EQ(1.7320508f, maximumValues[0]);
	ASSERT_DOUBLE_EQ(-0.5, minimumValues[1]);
	ASSERT_DOUBLE_EQ(0.5, maximumValues[1]);

	EXPECT_EQ(CMISS_OK, result = Cmiss_graphic_set_data_field(gr, offsetYField));
	maxRanges = Cmiss_scene_get_spectrum_data_range(zinc.scene, static_cast<Cmiss_graphics_filter_id>(0),
		zinc.defaultSpectrum, 3, minimumValues, maximumValues);
	EXPECT_EQ(1, maxRanges);
	ASSERT_DOUBLE_EQ(-0.5, minimumValues[0]);
	ASSERT_DOUBLE_EQ(0.5, maximumValues[0]);

	Cmiss_graphic_destroy(&gr);
	Cmiss_field_destroy(&dataField);
	Cmiss_field_destroy(&offsetYField);
	Cmiss_field_destroy(&yField);
	Cmiss_field_destroy(&offsetField);
	Cmiss_field_destroy(&magnitudeField);
	Cmiss_field_destroy(&coordinateField);
}

TEST(ZincScene, getSpectrumDataRange)
{
	ZincTestSetupSpectrumCpp zinc;

	int result;

	EXPECT_EQ(CMISS_OK, result = zinc.root_region.readFile(TestResources::getLocation(TestResources::FIELDMODULE_CUBE_RESOURCE)));

	Field coordinateField = zinc.fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinateField.isValid());
	Field magnitudeField = zinc.fm.createMagnitude(coordinateField);
	EXPECT_TRUE(magnitudeField.isValid());
	const double offset = -0.5;
	Field offsetField = zinc.fm.createConstant(1, &offset);
	EXPECT_TRUE(offsetField.isValid());
	Field yField = zinc.fm.createComponent(coordinateField, 2);
	EXPECT_TRUE(yField.isValid());
	Field offsetYField = zinc.fm.createAdd(yField, offsetField);
	EXPECT_TRUE(offsetYField.isValid());
	Field sourceFields[] = { magnitudeField, offsetYField };
	Field dataField = zinc.fm.createConcatenate(2, sourceFields);
	EXPECT_TRUE(dataField.isValid());

	Graphic gr = zinc.scene.createGraphicSurfaces();
	EXPECT_TRUE(gr.isValid());
	EXPECT_EQ(CMISS_OK, result = gr.setCoordinateField(coordinateField));
	EXPECT_EQ(CMISS_OK, result = gr.setDataField(dataField));
	EXPECT_EQ(CMISS_OK, result = gr.setSpectrum(zinc.defaultSpectrum));

	double minimumValues[3], maximumValues[3];
	GraphicsFilterModule gfm = zinc.gm.getFilterModule();
	GraphicsFilter defaultFilter = gfm.getDefaultFilter();

	int maxRanges = zinc.scene.getSpectrumDataRange(defaultFilter,
		zinc.defaultSpectrum, 3, minimumValues, maximumValues);
	EXPECT_EQ(2, maxRanges);
	ASSERT_DOUBLE_EQ(0.0, minimumValues[0]);
	ASSERT_DOUBLE_EQ(1.7320508f, maximumValues[0]);
	ASSERT_DOUBLE_EQ(-0.5, minimumValues[1]);
	ASSERT_DOUBLE_EQ(0.5, maximumValues[1]);

	EXPECT_EQ(CMISS_OK, result = gr.setDataField(offsetYField));
	maxRanges = zinc.scene.getSpectrumDataRange(defaultFilter,
		zinc.defaultSpectrum, 3, minimumValues, maximumValues);
	EXPECT_EQ(1, maxRanges);
	ASSERT_DOUBLE_EQ(-0.5, minimumValues[0]);
	ASSERT_DOUBLE_EQ(0.5, maximumValues[0]);
}

TEST(Cmiss_scene, visibility_flag)
{
	ZincTestSetup zinc;
	EXPECT_TRUE(Cmiss_scene_get_visibility_flag(zinc.scene));
	EXPECT_EQ(CMISS_OK, Cmiss_scene_set_visibility_flag(zinc.scene, false));
	EXPECT_FALSE(Cmiss_scene_get_visibility_flag(zinc.scene));
}

TEST(ZincScene, visibilityFlag)
{
	ZincTestSetupCpp zinc;
	EXPECT_TRUE(zinc.scene.getVisibilityFlag());
	EXPECT_EQ(CMISS_OK, zinc.scene.setVisibilityFlag(false));
	EXPECT_FALSE(zinc.scene.getVisibilityFlag());
}

TEST(Cmiss_scene, graphic_list)
{
	ZincTestSetup zinc;
	Cmiss_graphic_id lines = Cmiss_graphic_lines_base_cast(Cmiss_scene_create_graphic_lines(zinc.scene));
	EXPECT_NE(static_cast<Cmiss_graphic_id>(0), lines);
	Cmiss_graphic_id points = Cmiss_graphic_points_base_cast(Cmiss_scene_create_graphic_points(zinc.scene));
	EXPECT_NE(static_cast<Cmiss_graphic_id>(0), points);
	Cmiss_graphic_id surfaces = Cmiss_graphic_surfaces_base_cast(Cmiss_scene_create_graphic_surfaces(zinc.scene));
	EXPECT_NE(static_cast<Cmiss_graphic_id>(0), surfaces);

	Cmiss_graphic_id gr = 0;
	gr = Cmiss_scene_get_first_graphic(zinc.scene);
	ASSERT_EQ(lines, gr);
	Cmiss_graphic_destroy(&gr);
	gr = Cmiss_scene_get_next_graphic(zinc.scene, lines);
	ASSERT_EQ(points, gr);
	Cmiss_graphic_destroy(&gr);
	gr = Cmiss_scene_get_next_graphic(zinc.scene, points);
	ASSERT_EQ(surfaces, gr);
	Cmiss_graphic_destroy(&gr);
	gr = Cmiss_scene_get_next_graphic(zinc.scene, surfaces);
	ASSERT_EQ(static_cast<Cmiss_graphic_id>(0), gr);

	// reverse iteration
	gr = Cmiss_scene_get_previous_graphic(zinc.scene, points);
	ASSERT_EQ(lines, gr);
	Cmiss_graphic_destroy(&gr);
	gr = Cmiss_scene_get_previous_graphic(zinc.scene, lines);
	ASSERT_EQ(static_cast<Cmiss_graphic_id>(0), gr);

	int result;
	ASSERT_EQ(CMISS_OK, result = Cmiss_scene_move_graphic_before(zinc.scene, surfaces, lines));

	gr = Cmiss_scene_get_first_graphic(zinc.scene);
	ASSERT_EQ(surfaces, gr);
	Cmiss_graphic_destroy(&gr);
	gr = Cmiss_scene_get_next_graphic(zinc.scene, surfaces);
	ASSERT_EQ(lines, gr);
	Cmiss_graphic_destroy(&gr);
	gr = Cmiss_scene_get_next_graphic(zinc.scene, lines);
	ASSERT_EQ(points, gr);
	Cmiss_graphic_destroy(&gr);
	gr = Cmiss_scene_get_next_graphic(zinc.scene, points);
	ASSERT_EQ(static_cast<Cmiss_graphic_id>(0), gr);

	// move to end of list
	ASSERT_EQ(CMISS_OK, result = Cmiss_scene_move_graphic_before(zinc.scene, surfaces, static_cast<Cmiss_graphic_id>(0)));

	gr = Cmiss_scene_get_first_graphic(zinc.scene);
	ASSERT_EQ(lines, gr);
	Cmiss_graphic_destroy(&gr);
	gr = Cmiss_scene_get_next_graphic(zinc.scene, lines);
	ASSERT_EQ(points, gr);
	Cmiss_graphic_destroy(&gr);
	gr = Cmiss_scene_get_next_graphic(zinc.scene, points);
	ASSERT_EQ(surfaces, gr);
	Cmiss_graphic_destroy(&gr);
	gr = Cmiss_scene_get_next_graphic(zinc.scene, surfaces);
	ASSERT_EQ(static_cast<Cmiss_graphic_id>(0), gr);

	ASSERT_EQ(CMISS_OK, result = Cmiss_scene_remove_graphic(zinc.scene, points));

	gr = Cmiss_scene_get_first_graphic(zinc.scene);
	ASSERT_EQ(lines, gr);
	Cmiss_graphic_destroy(&gr);
	gr = Cmiss_scene_get_next_graphic(zinc.scene, lines);
	ASSERT_EQ(surfaces, gr);
	Cmiss_graphic_destroy(&gr);
	gr = Cmiss_scene_get_next_graphic(zinc.scene, surfaces);
	ASSERT_EQ(static_cast<Cmiss_graphic_id>(0), gr);

	// can't re-add points graphic that has been removed
	ASSERT_EQ(CMISS_ERROR_ARGUMENT, result = Cmiss_scene_move_graphic_before(zinc.scene, points, static_cast<Cmiss_graphic_id>(0)));

	ASSERT_EQ(CMISS_OK, result = Cmiss_scene_remove_all_graphics(zinc.scene));

	Cmiss_graphic_destroy(&lines);
	Cmiss_graphic_destroy(&points);
	Cmiss_graphic_destroy(&surfaces);
}

TEST(ZincScene, graphic_list)
{
	ZincTestSetupCpp zinc;
	GraphicLines lines = zinc.scene.createGraphicLines();
	EXPECT_TRUE(lines.isValid());
	GraphicPoints points = zinc.scene.createGraphicPoints();
	EXPECT_TRUE(points.isValid());
	GraphicSurfaces surfaces = zinc.scene.createGraphicSurfaces();
	EXPECT_TRUE(surfaces.isValid());

	Graphic gr;
	gr = zinc.scene.getFirstGraphic();
	ASSERT_EQ(lines.getId(), gr.getId());
	gr = zinc.scene.getNextGraphic(gr);
	ASSERT_EQ(points.getId(), gr.getId());
	gr = zinc.scene.getNextGraphic(gr);
	ASSERT_EQ(surfaces.getId(), gr.getId());
	gr = zinc.scene.getNextGraphic(gr);
	ASSERT_FALSE(gr.isValid());

	// reverse iteration
	gr = zinc.scene.getPreviousGraphic(points);
	ASSERT_EQ(lines.getId(), gr.getId());
	gr = zinc.scene.getPreviousGraphic(gr);
	ASSERT_FALSE(gr.isValid());

	int result;
	ASSERT_EQ(CMISS_OK, result = zinc.scene.moveGraphicBefore(surfaces, lines));

	gr = zinc.scene.getFirstGraphic();
	ASSERT_EQ(surfaces.getId(), gr.getId());
	gr = zinc.scene.getNextGraphic(gr);
	ASSERT_EQ(lines.getId(), gr.getId());
	gr = zinc.scene.getNextGraphic(gr);
	ASSERT_EQ(points.getId(), gr.getId());
	gr = zinc.scene.getNextGraphic(gr);
	ASSERT_FALSE(gr.isValid());

	// move to end of list
	Graphic noGraphic;
	ASSERT_EQ(CMISS_OK, result = zinc.scene.moveGraphicBefore(surfaces, noGraphic));

	gr = zinc.scene.getFirstGraphic();
	ASSERT_EQ(lines.getId(), gr.getId());
	gr = zinc.scene.getNextGraphic(gr);
	ASSERT_EQ(points.getId(), gr.getId());
	gr = zinc.scene.getNextGraphic(gr);
	ASSERT_EQ(surfaces.getId(), gr.getId());
	gr = zinc.scene.getNextGraphic(gr);
	ASSERT_FALSE(gr.isValid());

	ASSERT_EQ(CMISS_OK, result = zinc.scene.removeGraphic(points));

	gr = zinc.scene.getFirstGraphic();
	ASSERT_EQ(lines.getId(), gr.getId());
	gr = zinc.scene.getNextGraphic(gr);
	ASSERT_EQ(surfaces.getId(), gr.getId());
	gr = zinc.scene.getNextGraphic(gr);
	ASSERT_FALSE(gr.isValid());

	// can't re-add points graphic that has been removed
	ASSERT_EQ(CMISS_ERROR_ARGUMENT, result = result = zinc.scene.moveGraphicBefore(points, noGraphic));

	ASSERT_EQ(CMISS_OK, result = zinc.scene.removeAllGraphics());
}
