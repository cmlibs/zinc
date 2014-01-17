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
#include <zinc/field.h>
#include <zinc/fieldfiniteelement.h>
#include <zinc/fieldmodule.h>
#include <zinc/graphics.h>
#include <zinc/node.h>
#include <zinc/region.h>
#include <zinc/scene.h>
#include <zinc/scenefilter.h>

#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"
#include "zinc/graphics.hpp"
#include "zinc/field.hpp"
#include "zinc/fieldmodule.hpp"
#include "zinc/fieldfiniteelement.hpp"
#include "zinc/fieldsubobjectgroup.hpp"
#include "zinc/node.hpp"
#include "zinc/region.hpp"

#include "test_resources.h"

TEST(cmzn_scene_convert_to_point_cloud, surface_points)
{
	ZincTestSetup zinc;

	EXPECT_EQ(CMZN_OK, cmzn_region_read_file(zinc.root_region, TestResources::getLocation(TestResources::FIELDMODULE_CUBE_RESOURCE)));

	cmzn_field_id graphicCoordinateField = cmzn_fieldmodule_find_field_by_name(zinc.fm, "coordinates");
	EXPECT_NE(static_cast<cmzn_field_id>(0), graphicCoordinateField);
	cmzn_graphics_id gr = cmzn_scene_create_graphics_surfaces(zinc.scene);
	EXPECT_NE(static_cast<cmzn_graphics_id>(0), gr);
	EXPECT_EQ(CMZN_OK, cmzn_graphics_set_coordinate_field(gr, graphicCoordinateField));
	cmzn_field_destroy(&graphicCoordinateField);

	cmzn_region_id outputRegion = cmzn_region_create_child(zinc.root_region, "output");
	EXPECT_NE(static_cast<cmzn_region_id>(0), outputRegion);
	cmzn_fieldmodule_id outputFm = cmzn_region_get_fieldmodule(outputRegion);
	EXPECT_NE(static_cast<cmzn_fieldmodule_id>(0), outputFm);
	cmzn_field_id outputCoordinateField = cmzn_fieldmodule_create_field_finite_element(outputFm, 3);
	EXPECT_NE(static_cast<cmzn_field_id>(0), outputCoordinateField);
	EXPECT_EQ(CMZN_OK, cmzn_field_set_name(outputCoordinateField, "coordinates"));
	EXPECT_EQ(CMZN_OK, cmzn_field_set_managed(outputCoordinateField, true));
	cmzn_nodeset_id nodeset = cmzn_fieldmodule_find_nodeset_by_field_domain_type(outputFm, CMZN_FIELD_DOMAIN_TYPE_NODES);
	EXPECT_NE(static_cast<cmzn_nodeset_id>(0), nodeset);

	EXPECT_EQ(CMZN_OK, cmzn_scene_convert_to_point_cloud(zinc.scene,
		static_cast<cmzn_scenefilter_id>(0), nodeset, outputCoordinateField,
		/*lineDensity*/0.0, /*lineDensityScaleFactor*/0.0,
		/*surfaceDensity*/100.0, /*surfaceDensityScaleFactor*/0.0));
	int numberOfPoints = cmzn_nodeset_get_size(nodeset);
	EXPECT_GT(numberOfPoints, 550);
	EXPECT_LT(numberOfPoints, 650);

	cmzn_nodeset_destroy(&nodeset);
	cmzn_field_destroy(&outputCoordinateField);
	cmzn_fieldmodule_destroy(&outputFm);
	cmzn_region_destroy(&outputRegion);
	EXPECT_EQ(CMZN_OK, cmzn_graphics_destroy(&gr));
}

TEST(cmzn_scene_convert_to_point_cloud, surface_points_cpp)
{
	ZincTestSetupCpp zinc;

	EXPECT_EQ(CMZN_OK, zinc.root_region.readFile(TestResources::getLocation(TestResources::FIELDMODULE_CUBE_RESOURCE)));

	Field graphicCoordinateField = zinc.fm.findFieldByName("coordinates");
	EXPECT_TRUE(graphicCoordinateField.isValid());
	GraphicsSurfaces su = zinc.scene.createGraphicsSurfaces();
	EXPECT_TRUE(su.isValid());
	EXPECT_EQ(CMZN_OK, su.setCoordinateField(graphicCoordinateField));

	Region outputRegion = zinc.root_region.createChild("output");
	EXPECT_TRUE(outputRegion.isValid());
	Fieldmodule outputFm = outputRegion.getFieldmodule();
	EXPECT_TRUE(outputFm.isValid());
	Field outputCoordinateField = outputFm.createFieldFiniteElement(3);
	EXPECT_TRUE(outputCoordinateField.isValid());
	EXPECT_EQ(CMZN_OK, outputCoordinateField.setName("coordinates"));
	EXPECT_EQ(CMZN_OK, outputCoordinateField.setManaged(true));
	// put output into a node group
	Nodeset masterNodeset = outputFm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_TRUE(masterNodeset.isValid());
	FieldNodeGroup nodeGroupField = outputFm.createFieldNodeGroup(masterNodeset);
	EXPECT_EQ(CMZN_OK, nodeGroupField.setName("bob"));
	EXPECT_EQ(CMZN_OK, nodeGroupField.setManaged(true));
	NodesetGroup nodeset = nodeGroupField.getNodesetGroup();
	EXPECT_TRUE(nodeset.isValid());

	EXPECT_EQ(CMZN_OK, zinc.scene.convertToPointCloud(Scenefilter(),
		nodeset, outputCoordinateField,
		/*lineDensity*/0.0, /*lineDensityScaleFactor*/0.0,
		/*surfaceDensity*/100.0, /*surfaceDensityScaleFactor*/0.0));
	int numberOfPoints = nodeset.getSize();
	EXPECT_GT(numberOfPoints, 550);
	EXPECT_LT(numberOfPoints, 650);
}

TEST(cmzn_scene_convert_to_point_cloud, line_points)
{
	ZincTestSetup zinc;

	EXPECT_EQ(CMZN_OK, cmzn_region_read_file(zinc.root_region, TestResources::getLocation(TestResources::FIELDMODULE_CUBE_RESOURCE)));

	cmzn_field_id graphicCoordinateField = cmzn_fieldmodule_find_field_by_name(zinc.fm, "coordinates");
	EXPECT_NE(static_cast<cmzn_field_id>(0), graphicCoordinateField);
	cmzn_graphics_id gr = cmzn_scene_create_graphics_lines(zinc.scene);
	EXPECT_NE(static_cast<cmzn_graphics_id>(0), gr);
	EXPECT_EQ(CMZN_OK, cmzn_graphics_set_coordinate_field(gr, graphicCoordinateField));
	cmzn_field_destroy(&graphicCoordinateField);

	cmzn_region_id outputRegion = cmzn_region_create_child(zinc.root_region, "output");
	EXPECT_NE(static_cast<cmzn_region_id>(0), outputRegion);
	cmzn_fieldmodule_id outputFm = cmzn_region_get_fieldmodule(outputRegion);
	EXPECT_NE(static_cast<cmzn_fieldmodule_id>(0), outputFm);
	cmzn_field_id outputCoordinateField = cmzn_fieldmodule_create_field_finite_element(outputFm, 3);
	EXPECT_NE(static_cast<cmzn_field_id>(0), outputCoordinateField);
	EXPECT_EQ(CMZN_OK, cmzn_field_set_name(outputCoordinateField, "coordinates"));
	EXPECT_EQ(CMZN_OK, cmzn_field_set_managed(outputCoordinateField, true));
	cmzn_nodeset_id nodeset = cmzn_fieldmodule_find_nodeset_by_field_domain_type(outputFm, CMZN_FIELD_DOMAIN_TYPE_NODES);
	EXPECT_NE(static_cast<cmzn_nodeset_id>(0), nodeset);

	EXPECT_EQ(CMZN_OK, cmzn_scene_convert_to_point_cloud(zinc.scene,
		static_cast<cmzn_scenefilter_id>(0), nodeset, outputCoordinateField,
		/*lineDensity*/50.0, /*lineDensityScaleFactor*/0.0,
		/*surfaceDensity*/0.0, /*surfaceDensityScaleFactor*/0.0));
	int numberOfPoints = cmzn_nodeset_get_size(nodeset);
	EXPECT_GT(numberOfPoints, 550);
	EXPECT_LT(numberOfPoints, 650);

	cmzn_nodeset_destroy(&nodeset);
	cmzn_field_destroy(&outputCoordinateField);
	cmzn_fieldmodule_destroy(&outputFm);
	cmzn_region_destroy(&outputRegion);
	EXPECT_EQ(CMZN_OK, cmzn_graphics_destroy(&gr));
}

TEST(cmzn_scene_convert_to_point_cloud, line_points_cpp)
{
	ZincTestSetupCpp zinc;

	EXPECT_EQ(CMZN_OK, zinc.root_region.readFile(TestResources::getLocation(TestResources::FIELDMODULE_CUBE_RESOURCE)));

	Field graphicCoordinateField = zinc.fm.findFieldByName("coordinates");
	EXPECT_TRUE(graphicCoordinateField.isValid());
	GraphicsLines su = zinc.scene.createGraphicsLines();
	EXPECT_TRUE(su.isValid());
	EXPECT_EQ(CMZN_OK, su.setCoordinateField(graphicCoordinateField));

	Region outputRegion = zinc.root_region.createChild("output");
	EXPECT_TRUE(outputRegion.isValid());
	Fieldmodule outputFm = outputRegion.getFieldmodule();
	EXPECT_TRUE(outputFm.isValid());
	Field outputCoordinateField = outputFm.createFieldFiniteElement(3);
	EXPECT_TRUE(outputCoordinateField.isValid());
	EXPECT_EQ(CMZN_OK, outputCoordinateField.setName("coordinates"));
	EXPECT_EQ(CMZN_OK, outputCoordinateField.setManaged(true));
	Nodeset nodeset = outputFm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_TRUE(nodeset.isValid());

	EXPECT_EQ(CMZN_OK, zinc.scene.convertToPointCloud(Scenefilter(),
		nodeset, outputCoordinateField,
		/*lineDensity*/50.0, /*lineDensityScaleFactor*/0.0,
		/*surfaceDensity*/0.0, /*surfaceDensityScaleFactor*/0.0));
	int numberOfPoints = nodeset.getSize();
	EXPECT_GT(numberOfPoints, 550);
	EXPECT_LT(numberOfPoints, 650);
}
