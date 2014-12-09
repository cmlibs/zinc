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

ManageOutputFolder manageOutputFolderFieldML(FIELDML_OUTPUT_FOLDER);

namespace {

void check_cube_model(Fieldmodule& fm)
{
	int result;
	Field coordinates = fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinates.isValid());
	EXPECT_EQ(3, coordinates.getNumberOfComponents());
	EXPECT_TRUE(coordinates.isTypeCoordinate());

	Field pressure = fm.findFieldByName("pressure");
	EXPECT_TRUE(pressure.isValid());
	EXPECT_EQ(1, pressure.getNumberOfComponents());
	EXPECT_FALSE(pressure.isTypeCoordinate());

	EXPECT_EQ(OK, result = fm.defineAllFaces());
	Mesh mesh3d = fm.findMeshByDimension(3);
	EXPECT_EQ(1, mesh3d.getSize());
	Mesh mesh2d = fm.findMeshByDimension(2);
	EXPECT_EQ(6, mesh2d.getSize());
	Mesh mesh1d = fm.findMeshByDimension(1);
	EXPECT_EQ(12, mesh1d.getSize());
	Nodeset nodes = fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_EQ(8, nodes.getSize());
	Element element = mesh3d.findElementByIdentifier(1);
	EXPECT_TRUE(element.isValid());
	EXPECT_EQ(Element::SHAPE_TYPE_CUBE, element.getShapeType());

	const double valueOne = 1.0;
	Field one = fm.createFieldConstant(1, &valueOne);
	FieldMeshIntegral volume = fm.createFieldMeshIntegral(one, coordinates, mesh3d);
	const int numberOfPoints = 2;
	EXPECT_EQ(OK, result = volume.setNumbersOfPoints(1, &numberOfPoints));
	FieldMeshIntegral surfacePressureIntegral = fm.createFieldMeshIntegral(pressure, coordinates, mesh2d);
	EXPECT_EQ(OK, result = surfacePressureIntegral.setNumbersOfPoints(1, &numberOfPoints));

	Fieldcache cache = fm.createFieldcache();
	double outVolume;
	EXPECT_EQ(OK, result = volume.evaluateReal(cache, 1, &outVolume));
	ASSERT_DOUBLE_EQ(1.0, outVolume);
	double outSurfacePressureIntegral;
	EXPECT_EQ(OK, result = surfacePressureIntegral.evaluateReal(cache, 1, &outSurfacePressureIntegral));
	ASSERT_DOUBLE_EQ(540000.0, outSurfacePressureIntegral);
}

}

// cube model defines a 3-D RC coordinates field and 1-D pressure field
// using the same trilinear Lagrange scalar template.
// field dofs and mesh nodes connectivity are inline text in the fieldml document
TEST(ZincRegion, fieldml_cube)
{
	ZincTestSetupCpp zinc;
	int result;

	EXPECT_EQ(OK, result = zinc.root_region.readFile(
		TestResources::getLocation(TestResources::FIELDIO_FIELDML_CUBE_RESOURCE)));
	check_cube_model(zinc.fm);

	// test writing and re-reading into different region
	EXPECT_EQ(OK, result = zinc.root_region.writeFile(FIELDML_OUTPUT_FOLDER "/cube.fieldml"));
	Region testRegion = zinc.root_region.createChild("test");
	EXPECT_EQ(OK, result = testRegion.readFile(FIELDML_OUTPUT_FOLDER "/cube.fieldml"));
	Fieldmodule testFm = testRegion.getFieldmodule();
	check_cube_model(testFm);
}

// Also reads cube model, but tries to read it as EX format which should fail
TEST(ZincStreaminformationRegion, fileFormat)
{
	ZincTestSetupCpp zinc;
	int result;

	StreaminformationRegion streamInfo = zinc.root_region.createStreaminformationRegion();
	EXPECT_TRUE(streamInfo.isValid());
	StreamresourceFile fileResource = streamInfo.createStreamresourceFile(
		TestResources::getLocation(TestResources::FIELDIO_FIELDML_CUBE_RESOURCE));
	EXPECT_TRUE(fileResource.isValid());
	StreaminformationRegion::FileFormat fileFormat = streamInfo.getFileFormat();
	EXPECT_EQ(StreaminformationRegion::FILE_FORMAT_AUTOMATIC, fileFormat);
	EXPECT_EQ(OK, result = streamInfo.setFileFormat(StreaminformationRegion::FILE_FORMAT_EX));
	EXPECT_EQ(StreaminformationRegion::FILE_FORMAT_EX, fileFormat = streamInfo.getFileFormat());
	result = zinc.root_region.read(streamInfo);
	EXPECT_EQ(ERROR_GENERAL, result); // not in EX format
	EXPECT_EQ(OK, result = streamInfo.setFileFormat(StreaminformationRegion::FILE_FORMAT_FIELDML));
	EXPECT_EQ(StreaminformationRegion::FILE_FORMAT_FIELDML, fileFormat = streamInfo.getFileFormat());
	result = zinc.root_region.read(streamInfo);
	EXPECT_EQ(OK, result); // in FieldML format
	check_cube_model(zinc.fm);
}

namespace {

void check_tetmesh_model(Fieldmodule& fm)
{
	int result;
	Field coordinates = fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinates.isValid());
	EXPECT_EQ(3, coordinates.getNumberOfComponents());
	EXPECT_TRUE(coordinates.isTypeCoordinate());

	EXPECT_EQ(OK, result = fm.defineAllFaces());
	Mesh mesh3d = fm.findMeshByDimension(3);
	EXPECT_EQ(102, mesh3d.getSize());
	Mesh mesh2d = fm.findMeshByDimension(2);
	EXPECT_EQ(232, mesh2d.getSize());
	Mesh mesh1d = fm.findMeshByDimension(1);
	EXPECT_EQ(167, mesh1d.getSize());
	Nodeset nodes = fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_EQ(38, nodes.getSize());
	for (int i = 1; i < 102; ++i)
	{
		Element element = mesh3d.findElementByIdentifier(i);
		EXPECT_TRUE(element.isValid());
		Element::ShapeType shapeType = element.getShapeType();
		EXPECT_EQ(Element::SHAPE_TYPE_TETRAHEDRON, shapeType);
	}

	const double valueOne = 1.0;
	Field one = fm.createFieldConstant(1, &valueOne);
	FieldMeshIntegral volume = fm.createFieldMeshIntegral(one, coordinates, mesh3d);
	EXPECT_TRUE(volume.isValid());

	FieldElementGroup exteriorFacesGroup = fm.createFieldElementGroup(mesh2d);
	EXPECT_TRUE(exteriorFacesGroup.isValid());
	EXPECT_EQ(OK, result = exteriorFacesGroup.setManaged(true));
	MeshGroup exteriorFacesMeshGroup = exteriorFacesGroup.getMeshGroup();
	EXPECT_TRUE(exteriorFacesMeshGroup.isValid());
	FieldIsExterior isExterior = fm.createFieldIsExterior();
	EXPECT_TRUE(isExterior.isValid());
	exteriorFacesMeshGroup.addElementsConditional(isExterior);
	EXPECT_EQ(56, exteriorFacesMeshGroup.getSize());
	FieldMeshIntegral surfaceArea = fm.createFieldMeshIntegral(one, coordinates, exteriorFacesMeshGroup);
	EXPECT_TRUE(surfaceArea.isValid());

	Fieldcache cache = fm.createFieldcache();
	double outVolume;
	EXPECT_EQ(OK, result = volume.evaluateReal(cache, 1, &outVolume));
	//ASSERT_DOUBLE_EQ(0.41723178864303812, outVolume);
	EXPECT_NEAR(0.41723178864303812, outVolume, 0.5E-7);
	double outSurfaceArea;
	EXPECT_EQ(OK, result = surfaceArea.evaluateReal(cache, 1, &outSurfaceArea));
	//ASSERT_DOUBLE_EQ(2.7717561493468423, outSurfaceArea);
	EXPECT_NEAR(2.7717561493468423, outSurfaceArea, 1.0E-7);
}

}

// tetmesh model defines a 3-D RC coordinates field over a tetrahedral
// mesh in approximate unit sphere shape with trilinearSimplex basis/
// node coordinates and connectivity are read from separate files
TEST(ZincRegion, fieldml_tetmesh)
{
	ZincTestSetupCpp zinc;
	int result;

	EXPECT_EQ(OK, result = zinc.root_region.readFile(
		TestResources::getLocation(TestResources::FIELDIO_FIELDML_TETMESH_RESOURCE)));
	check_tetmesh_model(zinc.fm);

	// check can't merge cube model since it redefines element 1 shape
	result = zinc.root_region.readFile(
		TestResources::getLocation(TestResources::FIELDIO_FIELDML_CUBE_RESOURCE));
	EXPECT_EQ(ERROR_INCOMPATIBLE_DATA, result);

	// test writing and re-reading into different region
	EXPECT_EQ(OK, result = zinc.root_region.writeFile(FIELDML_OUTPUT_FOLDER "/tetmesh.fieldml"));
	Region testRegion = zinc.root_region.createChild("test");
	EXPECT_EQ(OK, result = testRegion.readFile(FIELDML_OUTPUT_FOLDER "/tetmesh.fieldml"));
	Fieldmodule testFm = testRegion.getFieldmodule();
	check_tetmesh_model(testFm);
}

namespace {

void check_wheel_model(Fieldmodule& fm)
{
	int result;
	Field coordinates = fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinates.isValid());
	EXPECT_EQ(3, coordinates.getNumberOfComponents());
	EXPECT_TRUE(coordinates.isTypeCoordinate());

	EXPECT_EQ(OK, result = fm.defineAllFaces());
	Mesh mesh3d = fm.findMeshByDimension(3);
	EXPECT_EQ(12, mesh3d.getSize());
	Mesh mesh2d = fm.findMeshByDimension(2);
	EXPECT_EQ(48, mesh2d.getSize());
	Mesh mesh1d = fm.findMeshByDimension(1);
	EXPECT_EQ(61, mesh1d.getSize());
	Nodeset nodes = fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_EQ(129, nodes.getSize());
	for (int i = 1; i < 12; ++i)
	{
		Element element = mesh3d.findElementByIdentifier(i);
		EXPECT_TRUE(element.isValid());
		Element::ShapeType shapeType = element.getShapeType();
		if (i <= 6)
			EXPECT_EQ(Element::SHAPE_TYPE_WEDGE12, shapeType);
		else
			EXPECT_EQ(Element::SHAPE_TYPE_CUBE, shapeType);
	}

	const double valueOne = 1.0;
	Field one = fm.createFieldConstant(1, &valueOne);
	FieldMeshIntegral volume = fm.createFieldMeshIntegral(one, coordinates, mesh3d);
	EXPECT_TRUE(volume.isValid());
	const int pointCount = 2;
	EXPECT_EQ(OK, result = volume.setNumbersOfPoints(1, &pointCount));

	FieldElementGroup exteriorFacesGroup = fm.createFieldElementGroup(mesh2d);
	EXPECT_TRUE(exteriorFacesGroup.isValid());
	EXPECT_EQ(OK, result = exteriorFacesGroup.setManaged(true));
	MeshGroup exteriorFacesMeshGroup = exteriorFacesGroup.getMeshGroup();
	EXPECT_TRUE(exteriorFacesMeshGroup.isValid());
	FieldIsExterior isExterior = fm.createFieldIsExterior();
	EXPECT_TRUE(isExterior.isValid());
	exteriorFacesMeshGroup.addElementsConditional(isExterior);
	EXPECT_EQ(30, exteriorFacesMeshGroup.getSize());
	FieldMeshIntegral surfaceArea = fm.createFieldMeshIntegral(one, coordinates, exteriorFacesMeshGroup);
	EXPECT_TRUE(surfaceArea.isValid());
	EXPECT_EQ(OK, result = surfaceArea.setNumbersOfPoints(1, &pointCount));

	Fieldcache cache = fm.createFieldcache();
	double outVolume;
	EXPECT_EQ(OK, result = volume.evaluateReal(cache, 1, &outVolume));
	//ASSERT_DOUBLE_EQ(100.28718664065387, outVolume);
	EXPECT_NEAR(100.28718664065387, outVolume, 5.0E-5);
	double outSurfaceArea;
	EXPECT_EQ(OK, result = surfaceArea.evaluateReal(cache, 1, &outSurfaceArea));
	//ASSERT_DOUBLE_EQ(150.53218306379620, outSurfaceArea);
	EXPECT_NEAR(150.53218306379620, outSurfaceArea, 1.0E-4);
}

}

// wheel_direct model defines a 3-D RC coordinates field over a wheel mesh
// consisting of 6 wedge elements in the centre, and 6 cube elements around
// them, all coordinates interpolated with triquadratic bases.
// This model tests having variant element shapes and a piecewise field
// template which directly maps element to function (basis + parameter map).
// It also reads shapeids, node coordinates and connectivity (for wedge and
// cube connectivity) from separate files, and the connectivity data uses
// dictionary of keys (DOK) format with key data in the first column of the
// same file.
TEST(ZincRegion, fieldml_wheel_direct)
{
	ZincTestSetupCpp zinc;
	int result;
	EXPECT_EQ(OK, result = zinc.root_region.readFile(
		TestResources::getLocation(TestResources::FIELDIO_FIELDML_WHEEL_DIRECT_RESOURCE)));
	check_wheel_model(zinc.fm);
}

// wheel_indirect model is the same as the wheel_direct model except that it
// uses a more efficient indirect element-to-function map
TEST(ZincRegion, fieldml_wheel_indirect)
{
	ZincTestSetupCpp zinc;
	int result;
	EXPECT_EQ(OK, result = zinc.root_region.readFile(
		TestResources::getLocation(TestResources::FIELDIO_FIELDML_WHEEL_INDIRECT_RESOURCE)));
	check_wheel_model(zinc.fm);

	// test writing and re-reading into different region
	EXPECT_EQ(OK, result = zinc.root_region.writeFile(FIELDML_OUTPUT_FOLDER "/wheel.fieldml"));
	Region testRegion = zinc.root_region.createChild("test");
	EXPECT_EQ(OK, result = testRegion.readFile(FIELDML_OUTPUT_FOLDER "/wheel.fieldml"));
	Fieldmodule testFm = testRegion.getFieldmodule();
	check_wheel_model(testFm);
}

namespace {

void create_mixed_template_squares(Fieldmodule& fm)
{
	int result;

	FieldFiniteElement coordinates = fm.createFieldFiniteElement(/*numberOfComponents*/2);
	EXPECT_TRUE(coordinates.isValid());
	EXPECT_EQ(OK, result = coordinates.setName("coordinates"));
	EXPECT_EQ(OK, result = coordinates.setTypeCoordinate(true));
	EXPECT_EQ(OK, result = coordinates.setManaged(true));
	EXPECT_EQ(OK, result = coordinates.setComponentName(1, "x"));
	EXPECT_EQ(OK, result = coordinates.setComponentName(2, "y"));

	FieldFiniteElement pressure = fm.createFieldFiniteElement(/*numberOfComponents*/1);
	EXPECT_TRUE(pressure.isValid());
	EXPECT_EQ(OK, result = pressure.setName("pressure"));
	EXPECT_EQ(OK, result = pressure.setManaged(true));

	FieldFiniteElement temperature = fm.createFieldFiniteElement(/*numberOfComponents*/1);
	EXPECT_TRUE(temperature.isValid());
	EXPECT_EQ(OK, result = temperature.setName("temperature"));
	EXPECT_EQ(OK, result = temperature.setManaged(true));

	Nodeset nodeset = fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_TRUE(nodeset.isValid());

	Nodetemplate nodetemplate_cpt = nodeset.createNodetemplate();
	EXPECT_EQ(OK, result = nodetemplate_cpt.defineField(coordinates));
	EXPECT_EQ(OK, result = nodetemplate_cpt.defineField(pressure));
	EXPECT_EQ(OK, result = nodetemplate_cpt.defineField(temperature));
	Nodetemplate nodetemplate_cp = nodeset.createNodetemplate();
	EXPECT_EQ(OK, result = nodetemplate_cp.defineField(coordinates));
	EXPECT_EQ(OK, result = nodetemplate_cp.defineField(pressure));
	Nodetemplate nodetemplate_ct = nodeset.createNodetemplate();
	EXPECT_EQ(OK, result = nodetemplate_ct.defineField(coordinates));
	EXPECT_EQ(OK, result = nodetemplate_ct.defineField(temperature));
	Nodetemplate nodetemplate_c = nodeset.createNodetemplate();
	EXPECT_EQ(OK, result = nodetemplate_c.defineField(coordinates));
	Fieldcache fieldcache = fm.createFieldcache();
	for (int j = 0; j < 9; ++j)
		for (int i = 0; i < 9; ++i)
		{
			Node node;
			int identifier = j*9 + i + 1;
			const bool linearNode = (0 == (i % 2)) && (0 == (j % 2));
			bool hasPressure = (j < 7) && linearNode;
			bool hasTemperature = (j > 1);
			if (hasPressure)
			{
				if (hasTemperature)
					node = nodeset.createNode(identifier, nodetemplate_cpt);
				else
					node = nodeset.createNode(identifier, nodetemplate_cp);
			}
			else if (hasTemperature)
				node = nodeset.createNode(identifier, nodetemplate_ct);
			else
				node = nodeset.createNode(identifier, nodetemplate_c);
			EXPECT_EQ(OK, fieldcache.setNode(node));
			double coordinatesValues[2] = { i*0.5, j*0.5 };
			EXPECT_EQ(OK, coordinates.assignReal(fieldcache, 2, coordinatesValues));
			if (hasPressure)
			{
				double pressureValues = fabs((double)(i - j));
				EXPECT_EQ(OK, pressure.assignReal(fieldcache, 1, &pressureValues));
			}
			if (hasTemperature)
			{
				double temperatureValues = j*j + i*i;
				EXPECT_EQ(OK, temperature.assignReal(fieldcache, 1, &temperatureValues));
			}
		}

	Mesh mesh = fm.findMeshByDimension(2);
	EXPECT_TRUE(mesh.isValid());

	Elementbasis bilinearBasis = fm.createElementbasis(2, Elementbasis::FUNCTION_TYPE_LINEAR_LAGRANGE);
	EXPECT_TRUE(bilinearBasis.isValid());
	Elementbasis biquadraticBasis = fm.createElementbasis(2, Elementbasis::FUNCTION_TYPE_QUADRATIC_LAGRANGE);
	EXPECT_TRUE(biquadraticBasis.isValid());

	int biquadraticLocalNodeIndexes[9] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	int bilinearLocalNodeIndexes[4] = { 1, 3, 7, 9 };
	Elementtemplate elementtemplate_cpt = mesh.createElementtemplate();
	EXPECT_EQ(OK, result = elementtemplate_cpt.setElementShapeType(Element::SHAPE_TYPE_SQUARE));
	EXPECT_EQ(OK, result = elementtemplate_cpt.setNumberOfNodes(9));
	EXPECT_EQ(OK, result = elementtemplate_cpt.defineFieldSimpleNodal(coordinates, /*componentNumber*/1, biquadraticBasis, 9, biquadraticLocalNodeIndexes));
	EXPECT_EQ(OK, result = elementtemplate_cpt.defineFieldSimpleNodal(coordinates, /*componentNumber*/2, bilinearBasis, 4, bilinearLocalNodeIndexes));
	EXPECT_EQ(OK, result = elementtemplate_cpt.defineFieldSimpleNodal(pressure, /*componentNumber*/-1, bilinearBasis, 4, bilinearLocalNodeIndexes));
	EXPECT_EQ(OK, result = elementtemplate_cpt.defineFieldSimpleNodal(temperature, /*componentNumber*/-1, biquadraticBasis, 9, biquadraticLocalNodeIndexes));
	Elementtemplate elementtemplate_cp = mesh.createElementtemplate();
	EXPECT_EQ(OK, result = elementtemplate_cp.setElementShapeType(Element::SHAPE_TYPE_SQUARE));
	EXPECT_EQ(OK, result = elementtemplate_cp.setNumberOfNodes(9));
	EXPECT_EQ(OK, result = elementtemplate_cp.defineFieldSimpleNodal(coordinates, /*componentNumber*/1, biquadraticBasis, 9, biquadraticLocalNodeIndexes));
	EXPECT_EQ(OK, result = elementtemplate_cp.defineFieldSimpleNodal(coordinates, /*componentNumber*/2, bilinearBasis, 4, bilinearLocalNodeIndexes));
	EXPECT_EQ(OK, result = elementtemplate_cp.defineFieldSimpleNodal(pressure, /*componentNumber*/-1, bilinearBasis, 4, bilinearLocalNodeIndexes));
	Elementtemplate elementtemplate_ct = mesh.createElementtemplate();
	EXPECT_EQ(OK, result = elementtemplate_ct.setElementShapeType(Element::SHAPE_TYPE_SQUARE));
	EXPECT_EQ(OK, result = elementtemplate_ct.setNumberOfNodes(9));
	EXPECT_EQ(OK, result = elementtemplate_ct.defineFieldSimpleNodal(coordinates, /*componentNumber*/1, biquadraticBasis, 9, biquadraticLocalNodeIndexes));
	EXPECT_EQ(OK, result = elementtemplate_ct.defineFieldSimpleNodal(coordinates, /*componentNumber*/2, bilinearBasis, 4, bilinearLocalNodeIndexes));
	EXPECT_EQ(OK, result = elementtemplate_ct.defineFieldSimpleNodal(temperature, /*componentNumber*/-1, biquadraticBasis, 9, biquadraticLocalNodeIndexes));
	for (int j = 0; j < 4; ++j)
		for (int i = 0; i < 4; ++i)
		{
			const bool hasPressure = j < 3;
			const bool hasTemperature = j > 0;
			Elementtemplate elementtemplate;
			if (hasPressure)
			{
				if (hasTemperature)
					elementtemplate = elementtemplate_cpt;
				else
					elementtemplate = elementtemplate_cp;
			}
			else
				elementtemplate = elementtemplate_ct;
			const int baseNodeIdentifier = 18*j + 2*i + 1;
			for (int n = 0; n < 9; ++n)
			{
				const int nodeIdentifier = baseNodeIdentifier + (n / 3)*9 + (n % 3);
				Node node = nodeset.findNodeByIdentifier(nodeIdentifier);
				EXPECT_TRUE(node.isValid());
				EXPECT_EQ(OK, result = elementtemplate.setNode(n + 1, node));
			}
			const int elementIdentifier = j*4 + i + 1;
			EXPECT_EQ(OK, result = mesh.defineElement(elementIdentifier, elementtemplate));
		}
}

void check_mixed_template_squares(Fieldmodule& fm)
{
	int result;
	Field coordinates = fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinates.isValid());
	EXPECT_EQ(2, coordinates.getNumberOfComponents());
	EXPECT_TRUE(coordinates.isTypeCoordinate());
	Field pressure = fm.findFieldByName("pressure");
	EXPECT_TRUE(pressure.isValid());
	EXPECT_EQ(1, pressure.getNumberOfComponents());
	Field temperature = fm.findFieldByName("temperature");
	EXPECT_TRUE(temperature.isValid());
	EXPECT_EQ(1, temperature.getNumberOfComponents());

	EXPECT_EQ(OK, result = fm.defineAllFaces());
	Mesh mesh3d = fm.findMeshByDimension(3);
	EXPECT_EQ(0, mesh3d.getSize());
	Mesh mesh2d = fm.findMeshByDimension(2);
	int meshSize;
	EXPECT_EQ(16, meshSize = mesh2d.getSize());
	Mesh mesh1d = fm.findMeshByDimension(1);
	EXPECT_EQ(40, mesh1d.getSize());
	Nodeset nodes = fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	int nodesetSize;
	EXPECT_EQ(81, nodesetSize = nodes.getSize());
	Fieldcache fieldcache = fm.createFieldcache();
	EXPECT_TRUE(fieldcache.isValid());
	for (int e = 1; e < meshSize; ++e)
	{
		Element element = mesh2d.findElementByIdentifier(e);
		EXPECT_TRUE(element.isValid());
		Element::ShapeType shapeType = element.getShapeType();
		EXPECT_EQ(Element::SHAPE_TYPE_SQUARE, shapeType);
		EXPECT_EQ(OK, result = fieldcache.setElement(element));
		EXPECT_TRUE(coordinates.isDefinedAtLocation(fieldcache));
		bool hasPressure = pressure.isDefinedAtLocation(fieldcache);
		if (e < 13)
			EXPECT_TRUE(hasPressure);
		else
			EXPECT_FALSE(hasPressure);
		bool hasTemperature = temperature.isDefinedAtLocation(fieldcache);
		if (e > 4)
			EXPECT_TRUE(hasTemperature);
		else
			EXPECT_FALSE(hasTemperature);
	}
	for (int n = 1; n < nodesetSize; ++n)
	{
		Node node = nodes.findNodeByIdentifier(n);
		EXPECT_TRUE(node.isValid());
		EXPECT_EQ(OK, result = fieldcache.setNode(node));
		EXPECT_TRUE(coordinates.isDefinedAtLocation(fieldcache));
		const int i = (n - 1) % 9;
		const int j = (n - 1) / 9;
		const bool linearNode = (0 == (i % 2)) && (0 == (j % 2));
		const bool expectedHasPressure = (j < 7) && linearNode;
		const bool expectedHasTemperature = (j > 1);
		bool hasPressure = pressure.isDefinedAtLocation(fieldcache);
		if (expectedHasPressure)
			EXPECT_TRUE(hasPressure);
		else
			EXPECT_FALSE(hasPressure);
		bool hasTemperature = temperature.isDefinedAtLocation(fieldcache);
		if (expectedHasTemperature)
			EXPECT_TRUE(hasTemperature);
		else
			EXPECT_FALSE(hasTemperature);
	}

	FieldIsDefined pressureDefined = fm.createFieldIsDefined(pressure);
	EXPECT_TRUE(pressureDefined.isValid());
	FieldElementGroup pressureGroup = fm.createFieldElementGroup(mesh2d);
	EXPECT_TRUE(pressureGroup.isValid());
	MeshGroup pressureMesh = pressureGroup.getMeshGroup();
	EXPECT_EQ(OK, pressureMesh.addElementsConditional(pressureDefined));

	FieldIsDefined temperatureDefined = fm.createFieldIsDefined(temperature);
	EXPECT_TRUE(temperatureDefined.isValid());
	FieldElementGroup temperatureGroup = fm.createFieldElementGroup(mesh2d);
	EXPECT_TRUE(temperatureGroup.isValid());
	MeshGroup temperatureMesh = temperatureGroup.getMeshGroup();
	EXPECT_EQ(OK, temperatureMesh.addElementsConditional(temperatureDefined));

	const int pointCount = 1;
	FieldMeshIntegral pressureIntegral = fm.createFieldMeshIntegral(pressure, coordinates, pressureMesh);
	EXPECT_TRUE(pressureIntegral.isValid());
	EXPECT_EQ(OK, result = pressureIntegral.setNumbersOfPoints(1, &pointCount));
	FieldMeshIntegral temperatureIntegral = fm.createFieldMeshIntegral(temperature, coordinates, temperatureMesh);
	EXPECT_TRUE(temperatureIntegral.isValid());
	EXPECT_EQ(OK, result = temperatureIntegral.setNumbersOfPoints(1, &pointCount));

	double pressureIntegralValue;
	EXPECT_EQ(OK, result = pressureIntegral.evaluateReal(fieldcache, 1, &pressureIntegralValue));
	EXPECT_DOUBLE_EQ(31.0, pressureIntegralValue);
	double temperatureIntegralValue;
	EXPECT_EQ(OK, result = temperatureIntegral.evaluateReal(fieldcache, 1, &temperatureIntegralValue));
	EXPECT_DOUBLE_EQ(584.0, temperatureIntegralValue);
}

}

// 2D example with different templates for components of the coordinates field
// and for two different scalar fields including mix of bilinear and
// biquadratic elements, with latter two fields not defined on whole mesh.
TEST(ZincRegion, mixed_template_squares)
{
	ZincTestSetupCpp zinc;
	int result;

	create_mixed_template_squares(zinc.fm);
	check_mixed_template_squares(zinc.fm);

	// test writing and re-reading in EX format
	EXPECT_EQ(OK, result = zinc.root_region.writeFile(FIELDML_OUTPUT_FOLDER "/mixed_template_squares.exregion"));
	Region testRegion1 = zinc.root_region.createChild("test1");
	EXPECT_EQ(OK, result = testRegion1.readFile(FIELDML_OUTPUT_FOLDER "/mixed_template_squares.exregion"));
	Fieldmodule testFm1 = testRegion1.getFieldmodule();
	check_mixed_template_squares(testFm1);

	// test writing and re-reading in FieldML format
	EXPECT_EQ(OK, result = zinc.root_region.writeFile(FIELDML_OUTPUT_FOLDER "/mixed_template_squares.fieldml"));
	Region testRegion2 = zinc.root_region.createChild("test2");
	EXPECT_EQ(OK, result = testRegion2.readFile(FIELDML_OUTPUT_FOLDER "/mixed_template_squares.fieldml"));
	Fieldmodule testFm2 = testRegion2.getFieldmodule();
	check_mixed_template_squares(testFm2);
}
