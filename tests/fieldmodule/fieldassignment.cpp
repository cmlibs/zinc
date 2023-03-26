/*
 * Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include <opencmiss/zinc/changemanager.hpp>
#include <opencmiss/zinc/element.hpp>
#include <opencmiss/zinc/field.hpp>
#include <opencmiss/zinc/fieldapply.hpp>
#include <opencmiss/zinc/fieldarithmeticoperators.hpp>
#include <opencmiss/zinc/fieldassignment.hpp>
#include <opencmiss/zinc/fieldcache.hpp>
#include <opencmiss/zinc/fieldcomposite.hpp>
#include <opencmiss/zinc/fieldcoordinatetransformation.hpp>
#include <opencmiss/zinc/fieldconstant.hpp>
#include <opencmiss/zinc/fieldderivatives.hpp>
#include <opencmiss/zinc/fieldfiniteelement.hpp>
#include <opencmiss/zinc/fieldlogicaloperators.hpp>
#include <opencmiss/zinc/fieldmeshoperators.hpp>
#include <opencmiss/zinc/fieldsubobjectgroup.hpp>
#include <opencmiss/zinc/mesh.hpp>
#include <opencmiss/zinc/node.hpp>
#include <opencmiss/zinc/nodeset.hpp>
#include <opencmiss/zinc/streamregion.hpp>
#include <opencmiss/zinc/status.hpp>

#include "utilities/zinctestsetupcpp.hpp"
#include "test_resources.h"

TEST(ZincFieldassignment, cubeOffsetScale)
{
	ZincTestSetupCpp zinc;

    EXPECT_EQ(RESULT_OK, zinc.root_region.readFile(resourcePath("fieldmodule/cube.exformat").c_str()));

	Nodeset nodes = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_TRUE(nodes.isValid());

	Field coordinates = zinc.fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinates.isValid());
	const double scaleValues[3] = { 1.25, 2.0, 0.75 };
	Field scale = zinc.fm.createFieldConstant(3, scaleValues);
	EXPECT_TRUE(scale.isValid());
	const double offsetValues[3] = { 0.1, 0.2, -0.1 };
	Field offset = zinc.fm.createFieldConstant(3, offsetValues);
	EXPECT_TRUE(offset.isValid());
	Field newCoordinates = (coordinates*scale) + offset;
	EXPECT_TRUE(newCoordinates.isValid());
	EXPECT_EQ(3, newCoordinates.getNumberOfComponents());

	Fieldassignment fieldassignment = coordinates.createFieldassignment(newCoordinates);
	EXPECT_TRUE(fieldassignment.isValid());
	EXPECT_EQ(coordinates, fieldassignment.getTargetField());
	EXPECT_EQ(newCoordinates, fieldassignment.getSourceField());
	EXPECT_FALSE(fieldassignment.getConditionalField().isValid());
	EXPECT_FALSE(fieldassignment.getNodeset().isValid());

	Fieldcache cache = zinc.fm.createFieldcache();
	EXPECT_TRUE(cache.isValid());

	const double expectedCoordinates[8][3] = {
		{ 0.0, 0.0, 0.0 },
		{ 1.0, 0.0, 0.0 },
		{ 0.0, 1.0, 0.0 },
		{ 1.0, 1.0, 0.0 },
		{ 0.0, 0.0, 1.0 },
		{ 1.0, 0.0, 1.0 },
		{ 0.0, 1.0, 1.0 },
		{ 1.0, 1.0, 1.0 }
	};

	double x[3], newx[3];
	for (int n =  0; n < 8; ++n)
	{
		Node node = nodes.findNodeByIdentifier(n + 1);
		EXPECT_TRUE(node.isValid());
		EXPECT_EQ(RESULT_OK, cache.setNode(node));
		EXPECT_EQ(RESULT_OK, coordinates.evaluateReal(cache, 3, x));
		EXPECT_EQ(RESULT_OK, newCoordinates.evaluateReal(cache, 3, newx));
		for (int c = 0; c < 3; ++c)
		{
			EXPECT_EQ(expectedCoordinates[n][c], x[c]);
			EXPECT_EQ(expectedCoordinates[n][c]*scaleValues[c] + offsetValues[c], newx[c]);
		}
	}

	EXPECT_EQ(RESULT_OK, fieldassignment.assign());

	for (int n = 0; n < 8; ++n)
	{
		Node node = nodes.findNodeByIdentifier(n + 1);
		EXPECT_TRUE(node.isValid());
		EXPECT_EQ(RESULT_OK, cache.setNode(node));
		EXPECT_EQ(RESULT_OK, coordinates.evaluateReal(cache, 3, x));
		for (int c = 0; c < 3; ++c)
		{
			EXPECT_EQ(expectedCoordinates[n][c]*scaleValues[c] + offsetValues[c], x[c]);
		}
	}

	// reset and try nodeset and conditional field options

    EXPECT_EQ(RESULT_OK, zinc.root_region.readFile(resourcePath("fieldmodule/cube.exformat").c_str()));

	FieldNodeGroup nodeGroup = zinc.fm.createFieldNodeGroup(nodes);
	EXPECT_TRUE(nodeGroup.isValid());
	NodesetGroup nodesetGroup = nodeGroup.getNodesetGroup();
	EXPECT_TRUE(nodesetGroup.isValid());
	for (int n = 0; n < 4; ++n)
	{
		Node node = nodes.findNodeByIdentifier(n + 1);
		EXPECT_EQ(RESULT_OK, nodesetGroup.addNode(node));
	}
	EXPECT_EQ(RESULT_OK, fieldassignment.setNodeset(nodesetGroup));
	EXPECT_EQ(nodesetGroup, fieldassignment.getNodeset());
	EXPECT_EQ(RESULT_OK, fieldassignment.assign());

	for (int n = 0; n < 8; ++n)
	{
		Node node = nodes.findNodeByIdentifier(n + 1);
		EXPECT_TRUE(node.isValid());
		EXPECT_EQ(RESULT_OK, cache.setNode(node));
		EXPECT_EQ(RESULT_OK, coordinates.evaluateReal(cache, 3, x));
		for (int c = 0; c < 3; ++c)
		{
			if (n < 4)
			{
				EXPECT_EQ(expectedCoordinates[n][c]*scaleValues[c] + offsetValues[c], x[c]);
			}
			else
			{
				EXPECT_EQ(expectedCoordinates[n][c], x[c]);
			}
		}
	}

	Nodeset noNodeset;
	EXPECT_EQ(RESULT_OK, fieldassignment.setNodeset(noNodeset));
	EXPECT_FALSE(fieldassignment.getNodeset().isValid());
	FieldNot conditionalField = zinc.fm.createFieldNot(nodeGroup);
	EXPECT_EQ(RESULT_OK, fieldassignment.setConditionalField(conditionalField));
	EXPECT_EQ(conditionalField, fieldassignment.getConditionalField());
	EXPECT_EQ(RESULT_OK, fieldassignment.assign());
	for (int n = 0; n < 8; ++n)
	{
		Node node = nodes.findNodeByIdentifier(n + 1);
		EXPECT_TRUE(node.isValid());
		EXPECT_EQ(RESULT_OK, cache.setNode(node));
		EXPECT_EQ(RESULT_OK, coordinates.evaluateReal(cache, 3, x));
		for (int c = 0; c < 3; ++c)
		{
			EXPECT_EQ(expectedCoordinates[n][c]*scaleValues[c] + offsetValues[c], x[c]);
		}
	}
}

TEST(ZincFieldassignment, transformDerivatives)
{
	ZincTestSetupCpp zinc;

	// read coordinates and save a copy
    EXPECT_EQ(RESULT_OK, zinc.root_region.readFile(resourcePath("fieldmodule/two_cubes_hermite_nocross.ex2").c_str()));
	FieldFiniteElement copyCoordinates = zinc.fm.findFieldByName("coordinates").castFiniteElement();
	EXPECT_TRUE(copyCoordinates.isValid());
	EXPECT_EQ(RESULT_OK, copyCoordinates.setName("copyCoordinates"));
    EXPECT_EQ(RESULT_OK, zinc.root_region.readFile(resourcePath("fieldmodule/two_cubes_hermite_nocross.ex2").c_str()));
	FieldFiniteElement coordinates = zinc.fm.findFieldByName("coordinates").castFiniteElement();
	EXPECT_TRUE(coordinates.isValid());

	Mesh mesh3d = zinc.fm.findMeshByDimension(3);
	EXPECT_TRUE(mesh3d.isValid());
	Nodeset nodes = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_TRUE(nodes.isValid());

	// offset radius and scale coordinates
	const double offsetValues[3] = { 1.0, 0.0, 0.0 };
	Field offset = zinc.fm.createFieldConstant(3, offsetValues);
	EXPECT_TRUE(offset.isValid());
	const double scaleValues[3] = { 2.0, 0.78539816339744830961566084581988, 0.5 };
	Field scale = zinc.fm.createFieldConstant(3, scaleValues);
	EXPECT_TRUE(scale.isValid());
	Field offsetScaleCoordinates = (coordinates + offset)*scale;
	EXPECT_TRUE(offsetScaleCoordinates.isValid());

	// cast coordinates to spherical polar coordinate system and convert to RC; this is target coordinates
	Field sphericalCoordinates = zinc.fm.createFieldIdentity(offsetScaleCoordinates);
	EXPECT_TRUE(sphericalCoordinates.isValid());
	EXPECT_EQ(RESULT_OK, sphericalCoordinates.setCoordinateSystemType(Field::COORDINATE_SYSTEM_TYPE_SPHERICAL_POLAR));
	Field newCoordinates = zinc.fm.createFieldCoordinateTransformation(sphericalCoordinates);
	EXPECT_TRUE(newCoordinates.isValid());
	EXPECT_EQ(RESULT_OK, newCoordinates.setCoordinateSystemType(Field::COORDINATE_SYSTEM_TYPE_RECTANGULAR_CARTESIAN));
	const double oneValue = 1.0;
	Field one = zinc.fm.createFieldConstant(1, &oneValue);
	EXPECT_TRUE(one.isValid());

	const int numberOfGaussPoints = 4;
	FieldMeshIntegral targetVolume = zinc.fm.createFieldMeshIntegral(one, newCoordinates, mesh3d);
	EXPECT_TRUE(targetVolume.isValid());
	EXPECT_EQ(RESULT_OK, targetVolume.setNumbersOfPoints(1, &numberOfGaussPoints));
	FieldMeshIntegral actualVolume = zinc.fm.createFieldMeshIntegral(one, coordinates, mesh3d);
	EXPECT_TRUE(actualVolume.isValid());
	EXPECT_EQ(RESULT_OK, actualVolume.setNumbersOfPoints(1, &numberOfGaussPoints));

	Fieldcache cache = zinc.fm.createFieldcache();
	EXPECT_TRUE(cache.isValid());

	double actualVolumeValue, targetVolumeValue;
	const double volumeTolerance = 1.0E-6;
	EXPECT_EQ(RESULT_OK, actualVolume.evaluateReal(cache, 1, &actualVolumeValue));
	EXPECT_NEAR(2.0, actualVolumeValue, volumeTolerance);
	EXPECT_EQ(RESULT_OK, targetVolume.evaluateReal(cache, 1, &targetVolumeValue));
	EXPECT_NEAR(26.106769000328580, targetVolumeValue, volumeTolerance);

	Fieldassignment fieldassignment = coordinates.createFieldassignment(newCoordinates);
	EXPECT_TRUE(fieldassignment.isValid());

	// compute expected derivatives from spherical polar vector coordinate transformation
	const double d1Values[3] = { 2.0, 0.0, 0.0 };
	Field d1 = zinc.fm.createFieldConstant(3, d1Values);
	EXPECT_TRUE(d1.isValid());
	EXPECT_EQ(RESULT_OK, d1.setCoordinateSystemType(Field::COORDINATE_SYSTEM_TYPE_SPHERICAL_POLAR));
	const double d2Values[3] = { 0.0, 0.78539816339744830961566084581988, 0.0 };
	Field d2 = zinc.fm.createFieldConstant(3, d2Values);
	EXPECT_TRUE(d2.isValid());
	EXPECT_EQ(RESULT_OK, d2.setCoordinateSystemType(Field::COORDINATE_SYSTEM_TYPE_SPHERICAL_POLAR));
	const double d3Values[3] = { 0.0, 0.0, 0.5 };
	Field d3 = zinc.fm.createFieldConstant(3, d3Values);
	EXPECT_TRUE(d3.isValid());
	EXPECT_EQ(RESULT_OK, d3.setCoordinateSystemType(Field::COORDINATE_SYSTEM_TYPE_SPHERICAL_POLAR));
	Field v1 = zinc.fm.createFieldVectorCoordinateTransformation(d1, sphericalCoordinates);
	EXPECT_TRUE(v1.isValid());
	EXPECT_EQ(RESULT_OK, v1.setCoordinateSystemType(Field::COORDINATE_SYSTEM_TYPE_RECTANGULAR_CARTESIAN));
	Field v2 = zinc.fm.createFieldVectorCoordinateTransformation(d2, sphericalCoordinates);
	EXPECT_TRUE(v2.isValid());
	EXPECT_EQ(RESULT_OK, v2.setCoordinateSystemType(Field::COORDINATE_SYSTEM_TYPE_RECTANGULAR_CARTESIAN));
	Field v3 = zinc.fm.createFieldVectorCoordinateTransformation(d3, sphericalCoordinates);
	EXPECT_TRUE(v3.isValid());
	EXPECT_EQ(RESULT_OK, v3.setCoordinateSystemType(Field::COORDINATE_SYSTEM_TYPE_RECTANGULAR_CARTESIAN));
	// Verify the computed values which are stored here:
	const double xValuesExpected[12][3] = {
		{2.0000000000000000, 0.00000000000000000, 0.00000000000000000},
		{ 4.0000000000000000, 0.00000000000000000, 0.00000000000000000 },
		{ 6.0000000000000000, 0.00000000000000000, 0.00000000000000000 },
		{ 1.4142135623730951, 1.4142135623730951, 0.00000000000000000 },
		{ 2.8284271247461903, 2.8284271247461903, 0.00000000000000000 },
		{ 4.2426406871192857, 4.2426406871192857, 0.00000000000000000 },
		{ 1.7551651237807455, 0.00000000000000000, 0.95885107720840601 },
		{ 3.5103302475614910, 0.00000000000000000, 1.9177021544168120 },
		{ 5.2654953713422366, 0.00000000000000000, 2.8765532316252180 },
		{ 1.2410891611274912, 1.2410891611274912, 0.95885107720840601 },
		{ 2.4821783222549825, 2.4821783222549825, 1.9177021544168120 },
		{ 3.7232674833824739, 3.7232674833824739, 2.8765532316252180 }
	};
	const double v1ValuesExpected[12][3] = {
		{ 2.0000000000000000, 0.00000000000000000, 0.00000000000000000 },
		{ 2.0000000000000000, 0.00000000000000000, 0.00000000000000000 },
		{ 2.0000000000000000, 0.00000000000000000, 0.00000000000000000 },
		{ 1.4142135623730951, 1.4142135623730951, 0.00000000000000000 },
		{ 1.4142135623730951, 1.4142135623730951, 0.00000000000000000 },
		{ 1.4142135623730951, 1.4142135623730951, 0.00000000000000000 },
		{ 1.7551651237807455, 0.00000000000000000, 0.95885107720840601 },
		{ 1.7551651237807455, 0.00000000000000000, 0.95885107720840601 },
		{ 1.7551651237807455, 0.00000000000000000, 0.95885107720840601 },
		{ 1.2410891611274912, 1.2410891611274912, 0.95885107720840601 },
		{ 1.2410891611274912, 1.2410891611274912, 0.95885107720840601 },
		{ 1.2410891611274912, 1.2410891611274912, 0.95885107720840601 }
	};
	const double v2ValuesExpected[12][3] = {
		{ 0.00000000000000000, 1.5707963267948966, 0.00000000000000000 },
		{ 0.00000000000000000, 3.1415926535897931, 0.00000000000000000 },
		{ 0.00000000000000000, 4.7123889803846897, 0.00000000000000000 },
		{ -1.1107207345395915, 1.1107207345395915, 0.00000000000000000 },
		{ -2.2214414690791831, 2.2214414690791831, 0.00000000000000000 },
		{ -3.3321622036187750, 3.3321622036187750, 0.00000000000000000 },
		{ 0.00000000000000000, 1.3785034646766525, 0.00000000000000000 },
		{ 0.00000000000000000, 2.7570069293533050, 0.00000000000000000 },
		{ 0.00000000000000000, 4.1355103940299571, 0.00000000000000000 },
		{ -0.97474914776201138, 0.97474914776201138, 0.00000000000000000 },
		{ -1.9494982955240228, 1.9494982955240228, 0.00000000000000000 },
		{ -2.9242474432860339, 2.9242474432860339, 0.00000000000000000 }
	};
	const double v3ValuesExpected[12][3] = {
		{ 0.00000000000000000, 0.00000000000000000, 1.0000000000000000 },
		{ 0.00000000000000000, 0.00000000000000000, 2.0000000000000000 },
		{ 0.00000000000000000, 0.00000000000000000, 3.0000000000000000 },
		{ 0.00000000000000000, 0.00000000000000000, 1.0000000000000000 },
		{ 0.00000000000000000, 0.00000000000000000, 2.0000000000000000 },
		{ 0.00000000000000000, 0.00000000000000000, 3.0000000000000000 },
		{ -0.47942553860420301, 0.00000000000000000, 0.87758256189037276 },
		{ -0.95885107720840601, 0.00000000000000000, 1.7551651237807455 },
		{ -1.4382766158126090, 0.00000000000000000, 2.6327476856711183 },
		{ -0.33900504942104487, -0.33900504942104487, 0.87758256189037276 },
		{ -0.67801009884208974, -0.67801009884208974, 1.7551651237807455 },
		{ -1.0170151482631347, -1.0170151482631347, 2.6327476856711183 }
	};

	const double derivativeToleranceFine = 1.0E-8;
	double xValues[12][3];
	double v1Values[12][3];
	double v2Values[12][3];
	double v3Values[12][3];
	for (int n = 0; n < 12; ++n)
	{
		Node node = nodes.findNodeByIdentifier(n + 1);
		EXPECT_TRUE(node.isValid());
		EXPECT_EQ(RESULT_OK, cache.setNode(node));
		EXPECT_EQ(RESULT_OK, newCoordinates.evaluateReal(cache, 3, xValues[n]));
		for (int c = 0; c < 3; ++c)
		{
			EXPECT_NEAR(xValuesExpected[n][c], xValues[n][c], derivativeToleranceFine);
		}
		EXPECT_EQ(RESULT_OK, v1.evaluateReal(cache, 3, v1Values[n]));
		for (int c = 0; c < 3; ++c)
		{
			EXPECT_NEAR(v1ValuesExpected[n][c], v1Values[n][c], derivativeToleranceFine);
		}
		EXPECT_EQ(RESULT_OK, v2.evaluateReal(cache, 3, v2Values[n]));
		for (int c = 0; c < 3; ++c)
		{
			EXPECT_NEAR(v2ValuesExpected[n][c], v2Values[n][c], derivativeToleranceFine);
		}
		EXPECT_EQ(RESULT_OK, v3.evaluateReal(cache, 3, v3Values[n]));
		for (int c = 0; c < 3; ++c)
		{
			EXPECT_NEAR(v3ValuesExpected[n][c], v3Values[n][c], derivativeToleranceFine);
		}
	}

	// assign the coordinates to their new values on the spheroid
	EXPECT_EQ(RESULT_OK, fieldassignment.assign());

	EXPECT_EQ(RESULT_OK, actualVolume.evaluateReal(cache, 1, &actualVolumeValue));
	EXPECT_NEAR(26.010465708524421, actualVolumeValue, volumeTolerance);

	// verify the nodal derivatives are close enough to the expected values
	// use coarser tolerance since derivatives are calculated using finite differences
	const double derivativeToleranceCoarse = 1.0E-6;
	double x[3], dx_ds1[3], dx_ds2[3], dx_ds3[3];
	for (int n = 0; n < 12; ++n)
	{
		Node node = nodes.findNodeByIdentifier(n + 1);
		EXPECT_TRUE(node.isValid());
		EXPECT_EQ(RESULT_OK, cache.setNode(node));
		EXPECT_EQ(RESULT_OK, coordinates.getNodeParameters(cache, -1, Node::VALUE_LABEL_VALUE, /*version*/1, 3, x));
		for (int c = 0; c < 3; ++c)
		{
			EXPECT_NEAR(xValuesExpected[n][c], x[c], derivativeToleranceFine);
		}
		EXPECT_EQ(RESULT_OK, coordinates.getNodeParameters(cache, -1, Node::VALUE_LABEL_D_DS1, /*version*/1, 3, dx_ds1));
		for (int c = 0; c < 3; ++c)
		{
			EXPECT_NEAR(v1ValuesExpected[n][c], dx_ds1[c], derivativeToleranceCoarse);
		}
		EXPECT_EQ(RESULT_OK, coordinates.getNodeParameters(cache, -1, Node::VALUE_LABEL_D_DS2, /*version*/1, 3, dx_ds2));
		for (int c = 0; c < 3; ++c)
		{
			EXPECT_NEAR(v2ValuesExpected[n][c], dx_ds2[c], derivativeToleranceCoarse);
		}
		EXPECT_EQ(RESULT_OK, coordinates.getNodeParameters(cache, -1, Node::VALUE_LABEL_D_DS3, /*version*/1, 3, dx_ds3));
		for (int c = 0; c < 3; ++c)
		{
			EXPECT_NEAR(v3ValuesExpected[n][c], dx_ds3[c], derivativeToleranceFine);
		}
	}

	// check that assignment of coordinates into indentically defined
	// finite element field copyCoordinates copies all derivatives
	Fieldassignment fieldassignment2 = copyCoordinates.createFieldassignment(coordinates);
	EXPECT_TRUE(fieldassignment2.isValid());
	EXPECT_EQ(RESULT_OK, fieldassignment2.assign());
	for (int n = 0; n < 12; ++n)
	{
		Node node = nodes.findNodeByIdentifier(n + 1);
		EXPECT_TRUE(node.isValid());
		EXPECT_EQ(RESULT_OK, cache.setNode(node));
		EXPECT_EQ(RESULT_OK, copyCoordinates.getNodeParameters(cache, -1, Node::VALUE_LABEL_VALUE, /*version*/1, 3, x));
		for (int c = 0; c < 3; ++c)
		{
			EXPECT_NEAR(xValuesExpected[n][c], x[c], derivativeToleranceFine);
		}
		EXPECT_EQ(RESULT_OK, copyCoordinates.getNodeParameters(cache, -1, Node::VALUE_LABEL_D_DS1, /*version*/1, 3, dx_ds1));
		for (int c = 0; c < 3; ++c)
		{
			EXPECT_NEAR(v1ValuesExpected[n][c], dx_ds1[c], derivativeToleranceCoarse);
		}
		EXPECT_EQ(RESULT_OK, copyCoordinates.getNodeParameters(cache, -1, Node::VALUE_LABEL_D_DS2, /*version*/1, 3, dx_ds2));
		for (int c = 0; c < 3; ++c)
		{
			EXPECT_NEAR(v2ValuesExpected[n][c], dx_ds2[c], derivativeToleranceCoarse);
		}
		EXPECT_EQ(RESULT_OK, copyCoordinates.getNodeParameters(cache, -1, Node::VALUE_LABEL_D_DS3, /*version*/1, 3, dx_ds3));
		for (int c = 0; c < 3; ++c)
		{
			EXPECT_NEAR(v3ValuesExpected[n][c], dx_ds3[c], derivativeToleranceFine);
		}
	}
}

namespace {

void checkAssignNodeValueVersions(Fieldmodule& fm, const double *offset, const double *scale, bool squared = false)
{
	FieldFiniteElement coordinates = fm.findFieldByName("coordinates").castFiniteElement();
	EXPECT_TRUE(coordinates.isValid());
	FieldFiniteElement texture_coordinates = fm.findFieldByName("texture_coordinates").castFiniteElement();
	EXPECT_TRUE(texture_coordinates.isValid());
	const int componentCount = texture_coordinates.getNumberOfComponents();
	EXPECT_EQ(3, componentCount);

	Mesh mesh3d = fm.findMeshByDimension(3);
	EXPECT_TRUE(mesh3d.isValid());
	Nodeset nodes = fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_TRUE(nodes.isValid());

	Fieldcache cache = fm.createFieldcache();
	EXPECT_TRUE(cache.isValid());

	// check current texture_coordinates in elements and nodes
	const int numberInXi = squared ? 1 : 4;
	const double xiScale = 1.0 / static_cast<double>(numberInXi);
	double xi[3], tex1[3], tex2, dtex1, dtex2;
	const double tolerance = 1.0E-9;
	for (int e = 0; e < 2; ++e)
	{
		Element element = mesh3d.findElementByIdentifier(e + 1);
		EXPECT_TRUE(element.isValid());
		for (int k = 0; k <= numberInXi; ++k)
		{
			xi[2] = k*xiScale;
			for (int j = 0; j <= numberInXi; ++j)
			{
				xi[1] = j*xiScale;
				for (int i = 0; i <= numberInXi; ++i)
				{
					xi[0] = i*xiScale;
					EXPECT_EQ(RESULT_OK, cache.setMeshLocation(element, 3, xi));
					EXPECT_EQ(RESULT_OK, texture_coordinates.evaluateReal(cache, 3, tex1));
					double expectedValues[3] =
					{
						scale[0]*(offset[0] + (e + xi[0])*0.5),
						scale[1]*(offset[1] + xi[1]),
						scale[2]*(offset[2] + xi[2])
					};
					if (squared)
					{
						for (int c = 0; c < 3; ++c)
						{
							expectedValues[c] *= expectedValues[c];
						}
					}
					EXPECT_NEAR(expectedValues[0], tex1[0], tolerance);
					EXPECT_NEAR(expectedValues[1], tex1[1], tolerance);
					EXPECT_NEAR(expectedValues[2], tex1[2], tolerance);
				}
			}
		}
	}

	const int versionedNodeIds[4] = { 1, 3, 5, 7 };
	const double expected_tex1[4][3] = {
		{ 0.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 }, { 0.0, 0.0, 1.0 }, { 0.0, 1.0, 1.0 }
	};
	for (int n = 0; n < 4; ++n)
	{
		Node node = nodes.findNodeByIdentifier(versionedNodeIds[n]);
		EXPECT_TRUE(node.isValid());
		EXPECT_EQ(RESULT_OK, cache.setNode(node));
		EXPECT_EQ(RESULT_OK, texture_coordinates.getNodeParameters(cache, -1, Node::VALUE_LABEL_VALUE, /*version*/1, 3, tex1));
		EXPECT_EQ(RESULT_OK, texture_coordinates.getNodeParameters(cache, 1, Node::VALUE_LABEL_D_DS1, /*version*/1, 3, &dtex1));
		EXPECT_EQ(RESULT_OK, texture_coordinates.getNodeParameters(cache, 1, Node::VALUE_LABEL_VALUE, /*version*/2, 3, &tex2));
		EXPECT_EQ(RESULT_OK, texture_coordinates.getNodeParameters(cache, 1, Node::VALUE_LABEL_D_DS1, /*version*/2, 3, &dtex2));
		double expectedValues[4] =
		{
			scale[0]*(offset[0] + expected_tex1[n][0]),
			scale[1]*(offset[1] + expected_tex1[n][1]),
			scale[2]*(offset[2] + expected_tex1[n][2]),
			scale[0]*(offset[0] + 1.0)
		};
		double expectedDerivativeValues1 = 0.5*scale[0];
		double expectedDerivativeValues2 = 0.5*scale[0];
		if (squared)
		{
			expectedDerivativeValues1 *= 2.0*expectedValues[0];
			expectedDerivativeValues2 *= 2.0*expectedValues[3];
			for (int c = 0; c < 4; ++c)
			{
				expectedValues[c] *= expectedValues[c];
			}
		}
		EXPECT_NEAR(expectedValues[0], tex1[0], tolerance);
		EXPECT_NEAR(expectedValues[1], tex1[1], tolerance);
		EXPECT_NEAR(expectedValues[2], tex1[2], tolerance);
		EXPECT_NEAR(expectedValues[3], tex2, tolerance);
		EXPECT_NEAR(expectedDerivativeValues1, dtex1, tolerance);
		EXPECT_NEAR(expectedDerivativeValues2, dtex2, tolerance);
	}
}

}

// test that field assignment on finite element fields with multiple value versions
// evaluates source field as a function of the correct version, and assigns to that version
TEST(ZincFieldassignment, assignNodeValueVersions)
{
	ZincTestSetupCpp zinc;

	// read texture_coordinates and save a copy to assign back from later
    EXPECT_EQ(RESULT_OK, zinc.root_region.readFile(resourcePath("fieldmodule/cylinder_texture.ex2").c_str()));
	Field orig_texture_coordinates = zinc.fm.findFieldByName("texture_coordinates");
	EXPECT_TRUE(orig_texture_coordinates.isValid());
	EXPECT_EQ(RESULT_OK, orig_texture_coordinates.setName("orig_texture_coordinates"));
	// reload to get texture_coordinates to modify
    EXPECT_EQ(RESULT_OK, zinc.root_region.readFile(resourcePath("fieldmodule/cylinder_texture.ex2").c_str()));
	FieldFiniteElement texture_coordinates = zinc.fm.findFieldByName("texture_coordinates").castFiniteElement();
	EXPECT_TRUE(texture_coordinates.isValid());

	const double offsetZero[3] = { 0.0, 0.0, 0.0 };
	const double scaleOne[3] = { 1.0, 1.0, 1.0 };
	checkAssignNodeValueVersions(zinc.fm, offsetZero, scaleOne);

	const double offsetValues[3] = { 1.0, 0.1, 0.2 };
	Field offset = zinc.fm.createFieldConstant(3, offsetValues);
	EXPECT_TRUE(offset.isValid());
	Field offset_texture_coordinates = zinc.fm.createFieldAdd(texture_coordinates, offset);
	EXPECT_TRUE(offset_texture_coordinates.isValid());
	Fieldassignment fieldassignment = texture_coordinates.createFieldassignment(offset_texture_coordinates);
	EXPECT_TRUE(fieldassignment.isValid());
	EXPECT_EQ(RESULT_OK, fieldassignment.assign());
	checkAssignNodeValueVersions(zinc.fm, offsetValues, scaleOne);

	const double scaleValues[3] = { 2.0, 1.5, 0.75 };
	Field scale = zinc.fm.createFieldConstant(3, scaleValues);
	EXPECT_TRUE(scale.isValid());
	Field scale_texture_coordinates = zinc.fm.createFieldMultiply(texture_coordinates, scale);
	EXPECT_TRUE(scale_texture_coordinates.isValid());
	fieldassignment = texture_coordinates.createFieldassignment(scale_texture_coordinates);
	EXPECT_TRUE(fieldassignment.isValid());
	EXPECT_EQ(RESULT_OK, fieldassignment.assign());
	checkAssignNodeValueVersions(zinc.fm, offsetValues, scaleValues);

	Field squared_texture_coordinates = zinc.fm.createFieldMultiply(texture_coordinates, texture_coordinates);
	EXPECT_TRUE(squared_texture_coordinates.isValid());
	fieldassignment = texture_coordinates.createFieldassignment(squared_texture_coordinates);
	EXPECT_TRUE(fieldassignment.isValid());
	EXPECT_EQ(RESULT_OK, fieldassignment.assign());
	checkAssignNodeValueVersions(zinc.fm, offsetValues, scaleValues, true);

	// assign back from orig_texture_coordinates to test finite element to finite element assignment
	fieldassignment = texture_coordinates.createFieldassignment(orig_texture_coordinates);
	EXPECT_TRUE(fieldassignment.isValid());
	EXPECT_EQ(RESULT_OK, fieldassignment.assign());
	checkAssignNodeValueVersions(zinc.fm, offsetZero, scaleOne);
}

// Test that field assignment can assign field derivatives with multiple versions for
// a destination finite element field when the source field is an embedding map from
// an Apply field. This uses the gradient finite difference calculation at nodes.
TEST(ZincFieldassignment, assignApplyEmbeddingDerivativesVersions)
{
	ZincTestSetupCpp zinc;

    EXPECT_EQ(RESULT_OK, zinc.root_region.readFile(resourcePath("fieldmodule/embed_host.exf").c_str()));

	Mesh mesh3d = zinc.fm.findMeshByDimension(3);
	EXPECT_TRUE(mesh3d.isValid());
	Field fittedCoordinates = zinc.fm.findFieldByName("fitted coordinates");
	EXPECT_TRUE(fittedCoordinates.isValid());
	const char *materialCoordinatesName = "material coordinates";
	Field materialCoordinates = zinc.fm.findFieldByName(materialCoordinatesName);
	EXPECT_TRUE(materialCoordinates.isValid());

	FieldArgumentReal coordinatesArgument = zinc.fm.createFieldArgumentReal(3);
	EXPECT_TRUE(coordinatesArgument.isValid());
	FieldFindMeshLocation findHostLocationFittedCoordinates = zinc.fm.createFieldFindMeshLocation(coordinatesArgument, fittedCoordinates, mesh3d);
	EXPECT_TRUE(findHostLocationFittedCoordinates.isValid());
	FieldEmbedded hostMaterialCoordinates = zinc.fm.createFieldEmbedded(materialCoordinates, findHostLocationFittedCoordinates);
	EXPECT_TRUE(hostMaterialCoordinates.isValid());

	Region dataRegion = zinc.root_region.createChild("data");
	EXPECT_TRUE(dataRegion.isValid());
	Fieldmodule dataFm = dataRegion.getFieldmodule();
	EXPECT_TRUE(dataFm.isValid());
    EXPECT_EQ(RESULT_OK, dataRegion.readFile(resourcePath("fieldmodule/embed_network.exf").c_str()));
	FieldFiniteElement dataCoordinates = dataFm.findFieldByName("coordinates").castFiniteElement();
	EXPECT_TRUE(dataCoordinates.isValid());
	{
		ChangeManager<Fieldmodule> changeFields(dataFm);

		// temporarily rename coordinates, write and re-read, to copy to "material coordinates"
		EXPECT_EQ(RESULT_OK, dataCoordinates.setName(materialCoordinatesName));
		StreaminformationRegion sir = dataRegion.createStreaminformationRegion();
		StreamresourceMemory srm = sir.createStreamresourceMemory();
		sir.setResourceFieldNames(srm, 1, &materialCoordinatesName);
		EXPECT_EQ(RESULT_OK, dataRegion.write(sir));
		const void *buffer;
		unsigned int bufferSize;
		EXPECT_EQ(RESULT_OK, srm.getBuffer(&buffer, &bufferSize));

		EXPECT_EQ(RESULT_OK, dataCoordinates.setName("coordinates"));

		StreaminformationRegion sir2 = dataRegion.createStreaminformationRegion();
		StreamresourceMemory srm2 = sir2.createStreamresourceMemoryBuffer(buffer, bufferSize);
		EXPECT_EQ(RESULT_OK, dataRegion.read(sir2));
	}
	FieldFiniteElement dataMaterialCoordinates = dataFm.findFieldByName(materialCoordinatesName).castFiniteElement();
	EXPECT_TRUE(dataMaterialCoordinates.isValid());

	FieldApply applyHostMaterialCoordinates = dataFm.createFieldApply(hostMaterialCoordinates);
	EXPECT_TRUE(applyHostMaterialCoordinates.isValid());
	EXPECT_EQ(RESULT_OK, applyHostMaterialCoordinates.setBindArgumentSourceField(coordinatesArgument, dataCoordinates));

	Fieldassignment fieldassignment = dataMaterialCoordinates.createFieldassignment(applyHostMaterialCoordinates);
	EXPECT_TRUE(fieldassignment.isValid());
	EXPECT_EQ(RESULT_OK, fieldassignment.assign());

	Element element1 = mesh3d.findElementByIdentifier(1);
	EXPECT_TRUE(element1.isValid());

	const int dataDerivativeVersionsCount[4] = { 1, 3, 1, 1 };
	Nodeset dataNodes = dataFm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	Fieldcache hostFieldcache = zinc.fm.createFieldcache();
	Fieldcache dataFieldcache = dataFm.createFieldcache();

	// Use precise gradient of fitted coordinates w.r.t. material Coordinates to transform derivatives
	FieldGradient gradFittedMaterial = zinc.fm.createFieldGradient(materialCoordinates, fittedCoordinates);
	EXPECT_TRUE(gradFittedMaterial.isValid());
	double dx_dX[9];

	double dataX[3] = { 0.0, 0.0, 0.0 };
	double dataD[3];
	FieldConstant constCoordinates = zinc.fm.createFieldConstant(3, dataX);
	EXPECT_TRUE(constCoordinates.isValid());
	FieldFindMeshLocation findFittedMeshLocation = zinc.fm.createFieldFindMeshLocation(constCoordinates, fittedCoordinates, mesh3d);
	EXPECT_TRUE(findFittedMeshLocation.isValid());
	EXPECT_EQ(RESULT_OK, findFittedMeshLocation.setSearchMode(FieldFindMeshLocation::SEARCH_MODE_NEAREST));
	double dataXi[3];
	double expectedMX[3], actualMX[3];
	double expectedMD[3], actualMD[3];
	const double XTOL = 1.0E-12;
	const double DTOL = 1.0E-9;  // coarser as finite difference approximation in actual values
	for (int n = 0; n < 4; ++n)
	{
		Node node = dataNodes.findNodeByIdentifier(n + 1);
		EXPECT_TRUE(node.isValid());
		// find dataXi from dataCoordinates
		EXPECT_EQ(RESULT_OK, dataFieldcache.setNode(node));
		EXPECT_EQ(RESULT_OK, dataCoordinates.getNodeParameters(dataFieldcache, -1, Node::VALUE_LABEL_VALUE, 1, 3, dataX));
		EXPECT_EQ(RESULT_OK, constCoordinates.assignReal(hostFieldcache, 3, dataX));
		EXPECT_EQ(element1, findFittedMeshLocation.evaluateMeshLocation(hostFieldcache, 3, dataXi));

		EXPECT_EQ(RESULT_OK, hostFieldcache.setMeshLocation(element1, 3, dataXi));
		EXPECT_EQ(RESULT_OK, materialCoordinates.evaluateReal(hostFieldcache, 3, expectedMX));
		EXPECT_EQ(RESULT_OK, dataMaterialCoordinates.getNodeParameters(dataFieldcache, -1, Node::VALUE_LABEL_VALUE, 1, 3, actualMX));
		for (int c = 0; c < 3; ++c)
		{
			EXPECT_NEAR(expectedMX[c], actualMX[c], XTOL);
		}

		EXPECT_EQ(RESULT_OK, gradFittedMaterial.evaluateReal(hostFieldcache, 9, dx_dX));
		for (int v = 0; v < dataDerivativeVersionsCount[n]; ++v)
		{
			// get data coordinates derivatives and transform with gradient
			// which gives transformed derivatives by an independent and precise calculation
			EXPECT_EQ(RESULT_OK, dataCoordinates.getNodeParameters(dataFieldcache, -1, Node::VALUE_LABEL_D_DS1, v + 1, 3, dataD));
			for (int i = 0; i < 3; ++i)
			{
				expectedMD[i] = 0.0;
				for (int j = 0; j < 3; ++j)
				{
					expectedMD[i] += dx_dX[i*3 + j] * dataD[j];
				}
			}
			EXPECT_EQ(RESULT_OK, dataMaterialCoordinates.getNodeParameters(dataFieldcache, -1, Node::VALUE_LABEL_D_DS1, v + 1, 3, actualMD));
			for (int c = 0; c < 3; ++c)
			{
				EXPECT_NEAR(expectedMD[c], actualMD[c], DTOL);
			}
		}
	}
}

// Test source field not being defined is detected by Fieldassignment
TEST(ZincFieldassignment, assignSourceNotDefined)
{
	ZincTestSetupCpp zinc;

	EXPECT_EQ(CMZN_OK, zinc.root_region.readFile(resourcePath("cube.ex2").c_str()));
	FieldFiniteElement bob = zinc.fm.findFieldByName("coordinates").castFiniteElement();
	EXPECT_TRUE(bob.isValid());
	EXPECT_EQ(RESULT_OK, bob.setName("bob"));
	EXPECT_EQ(CMZN_OK, zinc.root_region.readFile(resourcePath("cube.ex2").c_str()));
	FieldFiniteElement coordinates = zinc.fm.findFieldByName("coordinates").castFiniteElement();
	EXPECT_TRUE(coordinates.isValid());
	const double scaleValues[3] = { 2.0, 1.5, 0.75 };
	FieldConstant scale = zinc.fm.createFieldConstant(3, scaleValues);
	EXPECT_TRUE(scale.isValid());
	FieldMultiply scaledCoordinates = coordinates * scale;
	EXPECT_TRUE(scaledCoordinates.isValid());
	FieldFiniteElement undefinedCoordinates = zinc.fm.createFieldFiniteElement(3);
	EXPECT_TRUE(undefinedCoordinates.isValid());

	Fieldassignment fieldassignment = bob.createFieldassignment(scaledCoordinates);
	EXPECT_EQ(RESULT_OK, fieldassignment.assign());
	Fieldassignment fieldassignmentUndefined = bob.createFieldassignment(undefinedCoordinates);
	EXPECT_EQ(RESULT_ERROR_NOT_FOUND, fieldassignmentUndefined.assign());
}
