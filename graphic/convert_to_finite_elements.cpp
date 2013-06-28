
#include <gtest/gtest.h>

#include <zinc/zincconfigure.h>
#include <zinc/status.h>
#include <zinc/core.h>
#include <zinc/context.h>
#include <zinc/field.h>
#include <zinc/fieldfiniteelement.h>
#include <zinc/fieldmodule.h>
#include <zinc/graphic.h>
#include <zinc/graphicsfilter.h>
#include <zinc/node.h>
#include <zinc/region.h>
#include <zinc/rendition.h>

#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"
#include "zinc/graphic.hpp"
#include "zinc/field.hpp"
#include "zinc/fieldmodule.hpp"
#include "zinc/fieldtypesfiniteelement.hpp"
#include "zinc/node.hpp"
#include "zinc/region.hpp"

#include "test_resources.h"

TEST(Cmiss_rendition_convert_to_point_cloud, surface_points)
{
	ZincTestSetup zinc;

	EXPECT_EQ(CMISS_OK, Cmiss_region_read_file(zinc.root_region, TestResources::getLocation(TestResources::FIELDMODULE_CUBE_RESOURCE)));

	Cmiss_field_id graphicCoordinateField = Cmiss_field_module_find_field_by_name(zinc.fm, "coordinates");
	EXPECT_NE(static_cast<Cmiss_field_id>(0), graphicCoordinateField);
	Cmiss_graphic_id gr = Cmiss_graphic_surfaces_base_cast(Cmiss_rendition_create_graphic_surfaces(zinc.ren));
	EXPECT_NE(static_cast<Cmiss_graphic_id>(0), gr);
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_set_coordinate_field(gr, graphicCoordinateField));
	Cmiss_field_destroy(&graphicCoordinateField);

	Cmiss_region_id outputRegion = Cmiss_region_create_child(zinc.root_region, "output");
	EXPECT_NE(static_cast<Cmiss_region_id>(0), outputRegion);
	Cmiss_field_module_id outputFm = Cmiss_region_get_field_module(outputRegion);
	EXPECT_NE(static_cast<Cmiss_field_module_id>(0), outputFm);
	Cmiss_field_id outputCoordinateField = Cmiss_field_module_create_finite_element(outputFm, 3);
	EXPECT_NE(static_cast<Cmiss_field_id>(0), outputCoordinateField);
	EXPECT_EQ(CMISS_OK, Cmiss_field_set_name(outputCoordinateField, "coordinates"));
	EXPECT_EQ(CMISS_OK, Cmiss_field_set_managed(outputCoordinateField, true));
	Cmiss_nodeset_id nodeset = Cmiss_field_module_find_nodeset_by_domain_type(outputFm, CMISS_FIELD_DOMAIN_NODES);
	EXPECT_NE(static_cast<Cmiss_nodeset_id>(0), nodeset);

	EXPECT_EQ(CMISS_OK, Cmiss_rendition_convert_to_point_cloud(zinc.ren,
		static_cast<Cmiss_graphics_filter_id>(0), nodeset, outputCoordinateField,
		/*lineDensity*/0.0, /*lineDensityScaleFactor*/0.0,
		/*surfaceDensity*/100.0, /*surfaceDensityScaleFactor*/0.0));
	int numberOfPoints = Cmiss_nodeset_get_size(nodeset);
	EXPECT_GT(numberOfPoints, 550);
	EXPECT_LT(numberOfPoints, 650);

	Cmiss_nodeset_destroy(&nodeset);
	Cmiss_field_destroy(&outputCoordinateField);
	Cmiss_field_module_destroy(&outputFm);
	Cmiss_region_destroy(&outputRegion);
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_destroy(&gr));
}

TEST(Cmiss_rendition_convert_to_point_cloud, surface_points_cpp)
{
	ZincTestSetupCpp zinc;

	EXPECT_EQ(CMISS_OK, zinc.root_region.readFile(TestResources::getLocation(TestResources::FIELDMODULE_CUBE_RESOURCE)));

	Field graphicCoordinateField = zinc.fm.findFieldByName("coordinates");
	EXPECT_TRUE(graphicCoordinateField.isValid());
	GraphicSurfaces su = zinc.ren.createGraphicSurfaces();
	EXPECT_TRUE(su.isValid());
	EXPECT_EQ(CMISS_OK, su.setCoordinateField(graphicCoordinateField));

	Region outputRegion = zinc.root_region.createChild("output");
	EXPECT_TRUE(outputRegion.isValid());
	FieldModule outputFm = outputRegion.getFieldModule();
	EXPECT_TRUE(outputFm.isValid());
	Field outputCoordinateField = outputFm.createFiniteElement(3);
	EXPECT_TRUE(outputCoordinateField.isValid());
	EXPECT_EQ(CMISS_OK, outputCoordinateField.setName("coordinates"));
	EXPECT_EQ(CMISS_OK, outputCoordinateField.setManaged(true));
	// put output into a node group
	Nodeset masterNodeset = outputFm.findNodesetByDomainType(Field::DOMAIN_NODES);
	EXPECT_TRUE(masterNodeset.isValid());
	FieldNodeGroup nodeGroupField = outputFm.createNodeGroup(masterNodeset);
	EXPECT_EQ(CMISS_OK, nodeGroupField.setName("bob"));
	EXPECT_EQ(CMISS_OK, nodeGroupField.setManaged(true));
	NodesetGroup nodeset = nodeGroupField.getNodeset();
	EXPECT_TRUE(nodeset.isValid());

	GraphicsFilter noFilter;
	EXPECT_EQ(CMISS_OK, zinc.ren.convertToPointCloud(noFilter,
		nodeset, outputCoordinateField,
		/*lineDensity*/0.0, /*lineDensityScaleFactor*/0.0,
		/*surfaceDensity*/100.0, /*surfaceDensityScaleFactor*/0.0));
	int numberOfPoints = nodeset.getSize();
	EXPECT_GT(numberOfPoints, 550);
	EXPECT_LT(numberOfPoints, 650);
}

TEST(Cmiss_rendition_convert_to_point_cloud, line_points)
{
	ZincTestSetup zinc;

	EXPECT_EQ(CMISS_OK, Cmiss_region_read_file(zinc.root_region, TestResources::getLocation(TestResources::FIELDMODULE_CUBE_RESOURCE)));

	Cmiss_field_id graphicCoordinateField = Cmiss_field_module_find_field_by_name(zinc.fm, "coordinates");
	EXPECT_NE(static_cast<Cmiss_field_id>(0), graphicCoordinateField);
	Cmiss_graphic_id gr = Cmiss_graphic_lines_base_cast(Cmiss_rendition_create_graphic_lines(zinc.ren));
	EXPECT_NE(static_cast<Cmiss_graphic_id>(0), gr);
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_set_coordinate_field(gr, graphicCoordinateField));
	Cmiss_field_destroy(&graphicCoordinateField);

	Cmiss_region_id outputRegion = Cmiss_region_create_child(zinc.root_region, "output");
	EXPECT_NE(static_cast<Cmiss_region_id>(0), outputRegion);
	Cmiss_field_module_id outputFm = Cmiss_region_get_field_module(outputRegion);
	EXPECT_NE(static_cast<Cmiss_field_module_id>(0), outputFm);
	Cmiss_field_id outputCoordinateField = Cmiss_field_module_create_finite_element(outputFm, 3);
	EXPECT_NE(static_cast<Cmiss_field_id>(0), outputCoordinateField);
	EXPECT_EQ(CMISS_OK, Cmiss_field_set_name(outputCoordinateField, "coordinates"));
	EXPECT_EQ(CMISS_OK, Cmiss_field_set_managed(outputCoordinateField, true));
	Cmiss_nodeset_id nodeset = Cmiss_field_module_find_nodeset_by_domain_type(outputFm, CMISS_FIELD_DOMAIN_NODES);
	EXPECT_NE(static_cast<Cmiss_nodeset_id>(0), nodeset);

	EXPECT_EQ(CMISS_OK, Cmiss_rendition_convert_to_point_cloud(zinc.ren,
		static_cast<Cmiss_graphics_filter_id>(0), nodeset, outputCoordinateField,
		/*lineDensity*/50.0, /*lineDensityScaleFactor*/0.0,
		/*surfaceDensity*/0.0, /*surfaceDensityScaleFactor*/0.0));
	int numberOfPoints = Cmiss_nodeset_get_size(nodeset);
	EXPECT_GT(numberOfPoints, 550);
	EXPECT_LT(numberOfPoints, 650);

	Cmiss_nodeset_destroy(&nodeset);
	Cmiss_field_destroy(&outputCoordinateField);
	Cmiss_field_module_destroy(&outputFm);
	Cmiss_region_destroy(&outputRegion);
	EXPECT_EQ(CMISS_OK, Cmiss_graphic_destroy(&gr));
}

TEST(Cmiss_rendition_convert_to_point_cloud, line_points_cpp)
{
	ZincTestSetupCpp zinc;

	EXPECT_EQ(CMISS_OK, zinc.root_region.readFile(TestResources::getLocation(TestResources::FIELDMODULE_CUBE_RESOURCE)));

	Field graphicCoordinateField = zinc.fm.findFieldByName("coordinates");
	EXPECT_TRUE(graphicCoordinateField.isValid());
	GraphicLines su = zinc.ren.createGraphicLines();
	EXPECT_TRUE(su.isValid());
	EXPECT_EQ(CMISS_OK, su.setCoordinateField(graphicCoordinateField));

	Region outputRegion = zinc.root_region.createChild("output");
	EXPECT_TRUE(outputRegion.isValid());
	FieldModule outputFm = outputRegion.getFieldModule();
	EXPECT_TRUE(outputFm.isValid());
	Field outputCoordinateField = outputFm.createFiniteElement(3);
	EXPECT_TRUE(outputCoordinateField.isValid());
	EXPECT_EQ(CMISS_OK, outputCoordinateField.setName("coordinates"));
	EXPECT_EQ(CMISS_OK, outputCoordinateField.setManaged(true));
	Nodeset nodeset = outputFm.findNodesetByDomainType(Field::DOMAIN_NODES);
	EXPECT_TRUE(nodeset.isValid());

	GraphicsFilter noFilter;
	EXPECT_EQ(CMISS_OK, zinc.ren.convertToPointCloud(noFilter,
		nodeset, outputCoordinateField,
		/*lineDensity*/50.0, /*lineDensityScaleFactor*/0.0,
		/*surfaceDensity*/0.0, /*surfaceDensityScaleFactor*/0.0));
	int numberOfPoints = nodeset.getSize();
	EXPECT_GT(numberOfPoints, 550);
	EXPECT_LT(numberOfPoints, 650);
}
