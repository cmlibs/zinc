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
#include <opencmiss/zinc/context.h>
#include <opencmiss/zinc/field.h>
#include <opencmiss/zinc/fieldfiniteelement.h>
#include <opencmiss/zinc/fieldmodule.h>
#include <opencmiss/zinc/graphics.h>
#include <opencmiss/zinc/node.h>
#include <opencmiss/zinc/region.h>
#include <opencmiss/zinc/scene.h>
#include <opencmiss/zinc/scenefilter.h>

#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"
#include "opencmiss/zinc/graphics.hpp"
#include "opencmiss/zinc/field.hpp"
#include "opencmiss/zinc/fieldmodule.hpp"
#include "opencmiss/zinc/fieldfiniteelement.hpp"
#include "opencmiss/zinc/fieldsubobjectgroup.hpp"
#include "opencmiss/zinc/node.hpp"
#include "opencmiss/zinc/region.hpp"

#include "test_resources.h"

TEST(cmzn_scene_convert_to_point_cloud, surface_points)
{
	ZincTestSetup zinc;

    EXPECT_EQ(CMZN_OK, cmzn_region_read_file(zinc.root_region, resourcePath("fieldmodule/cube.exformat").c_str()));

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

    EXPECT_EQ(CMZN_OK, zinc.root_region.readFile(resourcePath("fieldmodule/cube.exformat").c_str()));

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

    EXPECT_EQ(CMZN_OK, cmzn_region_read_file(zinc.root_region,resourcePath("fieldmodule/cube.exformat").c_str()));

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

    EXPECT_EQ(CMZN_OK, zinc.root_region.readFile(resourcePath("fieldmodule/cube.exformat").c_str()));

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

TEST(cmzn_scene_convert_points_to_nodes, element_points)
{
	ZincTestSetup zinc;

    EXPECT_EQ(CMZN_OK, cmzn_region_read_file(zinc.root_region, resourcePath("fieldmodule/cube.exformat").c_str()));

	cmzn_field_id graphicCoordinateField = cmzn_fieldmodule_find_field_by_name(zinc.fm, "coordinates");
	EXPECT_NE(static_cast<cmzn_field_id>(0), graphicCoordinateField);

	cmzn_graphics_id gr = cmzn_scene_create_graphics_points(zinc.scene);
	EXPECT_NE(static_cast<cmzn_graphics_id>(0), gr);

	EXPECT_EQ(CMZN_OK, cmzn_graphics_set_field_domain_type(gr, CMZN_FIELD_DOMAIN_TYPE_MESH_HIGHEST_DIMENSION));

	EXPECT_EQ(CMZN_OK, cmzn_graphics_set_coordinate_field(gr, graphicCoordinateField));
	cmzn_field_destroy(&graphicCoordinateField);

	cmzn_tessellationmodule_id tessellationmodule = cmzn_context_get_tessellationmodule(zinc.context);
	cmzn_tessellation_id default_tessellation = cmzn_tessellationmodule_get_default_points_tessellation(tessellationmodule);
	EXPECT_NE(static_cast<cmzn_tessellation_id>(0), default_tessellation);

	int temp[3];
	temp[0] = 2;
	temp[1] = 2;
	temp[2] = 2;
	EXPECT_EQ(CMZN_OK, cmzn_tessellation_set_minimum_divisions(default_tessellation, 3, &(temp[0])));
	cmzn_tessellation_destroy(&default_tessellation);
	cmzn_tessellationmodule_destroy(&tessellationmodule);

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

	EXPECT_EQ(CMZN_OK, cmzn_scene_convert_points_to_nodes(zinc.scene,
		static_cast<cmzn_scenefilter_id>(0), nodeset, outputCoordinateField));
	int numberOfPoints = cmzn_nodeset_get_size(nodeset);

	EXPECT_EQ(numberOfPoints, 8);

	cmzn_nodeset_destroy(&nodeset);
	cmzn_field_destroy(&outputCoordinateField);
	cmzn_fieldmodule_destroy(&outputFm);
	cmzn_region_destroy(&outputRegion);
	EXPECT_EQ(CMZN_OK, cmzn_graphics_destroy(&gr));
}

TEST(cmzn_scene_convert_points_to_nodes,element_points_cpp)
{
	ZincTestSetupCpp zinc;

    EXPECT_EQ(CMZN_OK, zinc.root_region.readFile(resourcePath("fieldmodule/cube.exformat").c_str()));

	Field graphicCoordinateField = zinc.fm.findFieldByName("coordinates");
	EXPECT_TRUE(graphicCoordinateField.isValid());

	GraphicsPoints points = zinc.scene.createGraphicsPoints();
	EXPECT_TRUE(points.isValid());

	EXPECT_EQ(OK, points.setFieldDomainType(Field::DOMAIN_TYPE_MESH_HIGHEST_DIMENSION));

	EXPECT_EQ(CMZN_OK, points.setCoordinateField(graphicCoordinateField));

	Tessellationmodule tm = zinc.context.getTessellationmodule();
	Tessellation defaultTessellation = tm.getDefaultPointsTessellation();
	EXPECT_TRUE(defaultTessellation.isValid());

	int temp[3];
	temp[0] = 2;
	temp[1] = 2;
	temp[2] = 2;
	EXPECT_EQ(CMZN_OK, defaultTessellation.setMinimumDivisions(3, &(temp[0])));

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

	EXPECT_EQ(CMZN_OK, zinc.scene.convertPointsToNodes(Scenefilter(),
		nodeset, outputCoordinateField));
	int numberOfPoints = nodeset.getSize();
	EXPECT_EQ(numberOfPoints, 8);
}
