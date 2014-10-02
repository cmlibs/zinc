/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include <zinc/field.hpp>
#include <zinc/fieldcache.hpp>
#include <zinc/fieldconstant.hpp>
#include <zinc/fieldfiniteelement.hpp>
#include <zinc/fieldmeshoperators.hpp>
#include <zinc/fieldmodule.hpp>
#include <zinc/fieldsubobjectgroup.hpp>
#include <zinc/region.hpp>
#include <zinc/streamregion.hpp>

#include "zinctestsetupcpp.hpp"

#include "test_resources.h"

// cube model defines a 3-D RC coordinates field and 1-D pressure field
// using the same trilinear Lagrange scalar template.
// field dofs and mesh nodes connectivity are inline text in the fieldml document
TEST(ZincRegion, read_fieldml_cube)
{
	ZincTestSetupCpp zinc;
	int result;

	EXPECT_EQ(OK, result = zinc.root_region.readFile(
		TestResources::getLocation(TestResources::FIELDIO_FIELDML_CUBE_RESOURCE)));

	Field coordinates = zinc.fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinates.isValid());
	EXPECT_EQ(3, coordinates.getNumberOfComponents());
	EXPECT_TRUE(coordinates.isTypeCoordinate());

	Field pressure = zinc.fm.findFieldByName("pressure");
	EXPECT_TRUE(pressure.isValid());
	EXPECT_EQ(1, pressure.getNumberOfComponents());
	EXPECT_FALSE(pressure.isTypeCoordinate());

	EXPECT_EQ(OK, result = zinc.fm.defineAllFaces());
	Mesh mesh3d = zinc.fm.findMeshByDimension(3);
	EXPECT_EQ(1, mesh3d.getSize());
	Mesh mesh2d = zinc.fm.findMeshByDimension(2);
	EXPECT_EQ(6, mesh2d.getSize());
	Mesh mesh1d = zinc.fm.findMeshByDimension(1);
	EXPECT_EQ(12, mesh1d.getSize());
	Nodeset nodes = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_EQ(8, nodes.getSize());

	const double valueOne = 1.0;
	Field one = zinc.fm.createFieldConstant(1, &valueOne);
	FieldMeshIntegral volume = zinc.fm.createFieldMeshIntegral(one, coordinates, mesh3d);
	const int numberOfPoints = 2;
	EXPECT_EQ(OK, result = volume.setNumbersOfPoints(1, &numberOfPoints));
	FieldMeshIntegral surfacePressureIntegral = zinc.fm.createFieldMeshIntegral(pressure, coordinates, mesh2d);
	EXPECT_EQ(OK, result = surfacePressureIntegral.setNumbersOfPoints(1, &numberOfPoints));

	Fieldcache cache = zinc.fm.createFieldcache();
	double outVolume;
	EXPECT_EQ(OK, result = volume.evaluateReal(cache, 1, &outVolume));
	ASSERT_DOUBLE_EQ(1.0, outVolume);
	double outSurfacePressureIntegral;
	EXPECT_EQ(OK, result = surfacePressureIntegral.evaluateReal(cache, 1, &outSurfacePressureIntegral));
	ASSERT_DOUBLE_EQ(540000.0, outSurfacePressureIntegral);
}

// tetmesh model defines a 3-D RC coordinates field over a tetrahedral
// mesh in approximate unit sphere shape with trilinearSimplex basis/
// node coordinates and connectivity are read from separate files
TEST(ZincRegion, read_fieldml_tetmesh)
{
	ZincTestSetupCpp zinc;
	int result;

	EXPECT_EQ(OK, result = zinc.root_region.readFile(
		TestResources::getLocation(TestResources::FIELDIO_FIELDML_TETMESH_RESOURCE)));

	Field coordinates = zinc.fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinates.isValid());
	EXPECT_EQ(3, coordinates.getNumberOfComponents());
	EXPECT_TRUE(coordinates.isTypeCoordinate());

	EXPECT_EQ(OK, result = zinc.fm.defineAllFaces());
	Mesh mesh3d = zinc.fm.findMeshByDimension(3);
	EXPECT_EQ(102, mesh3d.getSize());
	Mesh mesh2d = zinc.fm.findMeshByDimension(2);
	EXPECT_EQ(232, mesh2d.getSize());
	Mesh mesh1d = zinc.fm.findMeshByDimension(1);
	EXPECT_EQ(167, mesh1d.getSize());
	Nodeset nodes = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_EQ(38, nodes.getSize());

	const double valueOne = 1.0;
	Field one = zinc.fm.createFieldConstant(1, &valueOne);
	FieldMeshIntegral volume = zinc.fm.createFieldMeshIntegral(one, coordinates, mesh3d);
	EXPECT_TRUE(volume.isValid());

	FieldElementGroup exteriorFacesGroup = zinc.fm.createFieldElementGroup(mesh2d);
	EXPECT_TRUE(exteriorFacesGroup.isValid());
	EXPECT_EQ(OK, result = exteriorFacesGroup.setManaged(true));
	MeshGroup exteriorFacesMeshGroup = exteriorFacesGroup.getMeshGroup();
	EXPECT_TRUE(exteriorFacesMeshGroup.isValid());
	FieldIsExterior isExterior = zinc.fm.createFieldIsExterior();
	EXPECT_TRUE(isExterior.isValid());
	exteriorFacesMeshGroup.addElementsConditional(isExterior);
	EXPECT_EQ(56, exteriorFacesMeshGroup.getSize());
	FieldMeshIntegral surfaceArea = zinc.fm.createFieldMeshIntegral(one, coordinates, exteriorFacesMeshGroup);
	EXPECT_TRUE(surfaceArea.isValid());

	Fieldcache cache = zinc.fm.createFieldcache();
	double outVolume;
	EXPECT_EQ(OK, result = volume.evaluateReal(cache, 1, &outVolume));
	ASSERT_DOUBLE_EQ(0.41723178864303812, outVolume);
	double outSurfaceArea;
	EXPECT_EQ(OK, result = surfaceArea.evaluateReal(cache, 1, &outSurfaceArea));
	ASSERT_DOUBLE_EQ(2.7717561493468423, outSurfaceArea);
}
