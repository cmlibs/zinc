/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>
#include <cmath>

#include <zinc/field.hpp>
#include <zinc/fieldcache.hpp>
#include <zinc/fieldconstant.hpp>
#include <zinc/fieldfiniteelement.hpp>
#include <zinc/fieldgroup.hpp>
#include <zinc/fieldlogicaloperators.hpp>
#include <zinc/fieldmeshoperators.hpp>
#include <zinc/fieldmodule.hpp>
#include <zinc/fieldsubobjectgroup.hpp>
#include <zinc/region.hpp>
#include <zinc/streamregion.hpp>

#include "utilities/zinctestsetupcpp.hpp"
#include "utilities/fileio.hpp"

#include "test_resources.h"

#define FIELDML_OUTPUT_FOLDER "fieldmltest"

namespace {
ManageOutputFolder manageOutputFolderFieldML(FIELDML_OUTPUT_FOLDER);
}

namespace {

void check_twohermitecubes_noscalefactors_model(Fieldmodule& fm)
{
	int result;
	Field coordinates = fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinates.isValid());
	EXPECT_EQ(3, coordinates.getNumberOfComponents());
	EXPECT_TRUE(coordinates.isTypeCoordinate());
	Field pressure = fm.findFieldByName("pressure");
	EXPECT_TRUE(pressure.isValid());
	EXPECT_EQ(1, pressure.getNumberOfComponents());

	EXPECT_EQ(OK, result = fm.defineAllFaces());
	Mesh mesh3d = fm.findMeshByDimension(3);
	const int elementsCount = mesh3d.getSize();
	EXPECT_EQ(2, elementsCount);
	Mesh mesh2d = fm.findMeshByDimension(2);
	EXPECT_EQ(11, mesh2d.getSize());
	Mesh mesh1d = fm.findMeshByDimension(1);
	EXPECT_EQ(20, mesh1d.getSize());
	Nodeset nodes = fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_EQ(12, nodes.getSize());
	Fieldcache cache = fm.createFieldcache();
	const int divisions = 3;
	const double divisionSize = 1.0 / static_cast<double>(divisions);
	double coordinatesOut[3], pressureOut, xi[3];
	for (int e = 1; e <= elementsCount; ++e)
	{
		Element element = mesh3d.findElementByIdentifier(e);
		EXPECT_TRUE(element.isValid());
		Element::ShapeType shapeType = element.getShapeType();
		EXPECT_EQ(Element::SHAPE_TYPE_CUBE, shapeType);
		double xOffset = static_cast<double>(e - 1);
		double pressureOffset = 80000.0 + 10000.0*xOffset;
		for (int k = 0; k <= divisions; ++k)
		{
			xi[2] = k*divisionSize;
			for (int j = 0; j <= divisions; ++j)
			{
				xi[1] = j*divisionSize;
				for (int i = 0; i <= divisions; ++i)
				{
					xi[0] = i*divisionSize;
					EXPECT_EQ(OK, result = cache.setMeshLocation(element, 3, xi));
					EXPECT_EQ(OK, result = coordinates.evaluateReal(cache, 3, coordinatesOut));
					EXPECT_DOUBLE_EQ(xOffset + divisionSize*i, coordinatesOut[0]);
					EXPECT_DOUBLE_EQ(divisionSize*j, coordinatesOut[1]);
					EXPECT_DOUBLE_EQ(divisionSize*k, coordinatesOut[2]);
					EXPECT_EQ(OK, result = pressure.evaluateReal(cache, 1, &pressureOut));
					EXPECT_DOUBLE_EQ(pressureOffset + 10000.0*divisionSize*i, pressureOut);
				}
			}
		}
	}

	Field isExterior = fm.createFieldIsExterior();
	EXPECT_TRUE(isExterior.isValid());
	FieldElementGroup exteriorFacesGroup = fm.createFieldElementGroup(mesh2d);
	EXPECT_TRUE(exteriorFacesGroup.isValid());
	EXPECT_EQ(OK, result = exteriorFacesGroup.setName("exterior"));
	MeshGroup exteriorFacesMeshGroup = exteriorFacesGroup.getMeshGroup();
	EXPECT_TRUE(exteriorFacesMeshGroup.isValid());
	EXPECT_EQ(OK, result = exteriorFacesMeshGroup.addElementsConditional(isExterior));
	EXPECT_EQ(10, exteriorFacesMeshGroup.getSize());
}

}

// a model consisting of two cubes with tricubic Hermite coordinates and trilinear pressure.
TEST(ZincRegion, fieldml_twohermitecubes_noscalefactors)
{
	ZincTestSetupCpp zinc;
	int result;

	EXPECT_EQ(OK, result = zinc.root_region.readFile(
		TestResources::getLocation(TestResources::FIELDIO_EX_TWOHERMITECUBES_NOSCALEFACTORS_RESOURCE)));
	check_twohermitecubes_noscalefactors_model(zinc.fm);

#if 0
	// test writing and re-reading into different region
	EXPECT_EQ(OK, result = zinc.root_region.writeFile(FIELDML_OUTPUT_FOLDER "/twohermitecubes_noscalefactors.fieldml"));
	Region testRegion = zinc.root_region.createChild("test");
	EXPECT_EQ(OK, result = testRegion.readFile(FIELDML_OUTPUT_FOLDER "/twohermitecubes_noscalefactors.fieldml"));
	Fieldmodule testFm = testRegion.getFieldmodule();
	check_twohermitecubes_noscalefactors_model(testFm);
#endif
}

namespace {

void check_figure8_model(Fieldmodule& fm)
{
	int result;
	Field coordinates = fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinates.isValid());
	EXPECT_EQ(3, coordinates.getNumberOfComponents());
	EXPECT_TRUE(coordinates.isTypeCoordinate());

	EXPECT_EQ(OK, result = fm.defineAllFaces());
	Mesh mesh3d = fm.findMeshByDimension(3);
	const int elementsCount = mesh3d.getSize();
	EXPECT_EQ(7, elementsCount);
	Mesh mesh2d = fm.findMeshByDimension(2);
	EXPECT_EQ(34, mesh2d.getSize());
	Mesh mesh1d = fm.findMeshByDimension(1);
	EXPECT_EQ(52, mesh1d.getSize());
	Nodeset nodes = fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_EQ(24, nodes.getSize());
	for (int e = 1; e <= elementsCount; ++e)
	{
		Element element = mesh3d.findElementByIdentifier(e);
		EXPECT_TRUE(element.isValid());
		Element::ShapeType shapeType = element.getShapeType();
		EXPECT_EQ(Element::SHAPE_TYPE_CUBE, shapeType);
	}

	FieldGroup exteriorGroup = fm.createFieldGroup();
	EXPECT_TRUE(exteriorGroup.isValid());
	EXPECT_EQ(OK, result = exteriorGroup.setName("exterior"));
	EXPECT_EQ(OK, result = exteriorGroup.setSubelementHandlingMode(FieldGroup::SUBELEMENT_HANDLING_MODE_FULL));

	FieldElementGroup exteriorFacesGroup = exteriorGroup.getFieldElementGroup(mesh2d);
	EXPECT_FALSE(exteriorFacesGroup.isValid());
	exteriorFacesGroup = exteriorGroup.createFieldElementGroup(mesh2d);
	EXPECT_TRUE(exteriorFacesGroup.isValid());
	FieldIsExterior isExterior = fm.createFieldIsExterior();
	EXPECT_TRUE(isExterior.isValid());
	MeshGroup exteriorFacesMeshGroup = exteriorFacesGroup.getMeshGroup();
	EXPECT_EQ(OK, result = exteriorFacesMeshGroup.addElementsConditional(isExterior));
	EXPECT_EQ(26, exteriorFacesMeshGroup.getSize());

	FieldElementGroup exteriorLinesGroup = exteriorGroup.getFieldElementGroup(mesh1d);
	EXPECT_TRUE(exteriorLinesGroup.isValid());
	MeshGroup exteriorLinesMeshGroup = exteriorLinesGroup.getMeshGroup();
	EXPECT_EQ(52, exteriorLinesMeshGroup.getSize());

	const double valueOne = 1.0;
	const int numberOfVolumePoints = 3;
	Field one = fm.createFieldConstant(1, &valueOne);
	FieldMeshIntegral volume = fm.createFieldMeshIntegral(one, coordinates, mesh3d);
	EXPECT_EQ(OK, result = volume.setNumbersOfPoints(1, &numberOfVolumePoints));
	FieldMeshIntegral surfaceArea = fm.createFieldMeshIntegral(one, coordinates, exteriorFacesMeshGroup);
	// more surface Gauss points gives greater accuracy due to additional nonlinearity
	// of sqrt function in computing dA from 3-dimensional coordinates.
	const int numberOfSurfacePoints = 4;
	EXPECT_TRUE(surfaceArea.isValid());
	EXPECT_EQ(OK, result = surfaceArea.setNumbersOfPoints(1, &numberOfSurfacePoints));

	Fieldcache cache = fm.createFieldcache();
	double outVolume;
	EXPECT_EQ(OK, result = volume.evaluateReal(cache, 1, &outVolume));
	EXPECT_NEAR(31.445204155883715, outVolume, 1.0E-6);
	double outSurfaceArea;
	EXPECT_EQ(OK, result = surfaceArea.evaluateReal(cache, 1, &outSurfaceArea));
	EXPECT_NEAR(87.495697863403507, outSurfaceArea, 0.5E-5);
}

}

// a tricubic Hermite model, 7 elements in a Figure 8 shape. Uses versions.
TEST(ZincRegion, fieldml_figure8)
{
	ZincTestSetupCpp zinc;
	int result;

	EXPECT_EQ(OK, result = zinc.root_region.readFile(
		TestResources::getLocation(TestResources::FIELDIO_EX_HERMITE_FIGURE8_RESOURCE)));
	check_figure8_model(zinc.fm);

#if 0
	// test writing and re-reading into different region
	EXPECT_EQ(OK, result = zinc.root_region.writeFile(FIELDML_OUTPUT_FOLDER "/figure8.fieldml"));
	Region testRegion = zinc.root_region.createChild("test");
	EXPECT_EQ(OK, result = testRegion.readFile(FIELDML_OUTPUT_FOLDER "/figure8.fieldml"));
	Fieldmodule testFm = testRegion.getFieldmodule();
	check_figure8_model(testFm);
#endif
}
