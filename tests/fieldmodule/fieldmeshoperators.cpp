/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cmath>
#include <gtest/gtest.h>

#include <opencmiss/zinc/element.hpp>
#include <opencmiss/zinc/field.hpp>
#include <opencmiss/zinc/fieldarithmeticoperators.hpp>
#include <opencmiss/zinc/fieldcache.hpp>
#include <opencmiss/zinc/fieldcomposite.hpp>
#include <opencmiss/zinc/fieldconstant.hpp>
#include <opencmiss/zinc/fieldlogicaloperators.hpp>
#include <opencmiss/zinc/fieldmeshoperators.hpp>
#include <opencmiss/zinc/fieldsubobjectgroup.hpp>
#include <opencmiss/zinc/fieldtime.hpp>
#include <opencmiss/zinc/fieldtrigonometry.hpp>
#include <opencmiss/zinc/fieldvectoroperators.hpp>
#include "zinctestsetupcpp.hpp"

#include "test_resources.h"

TEST(ZincFieldMeshIntegral, quadrature)
{
	ZincTestSetupCpp zinc;
	int result;

	EXPECT_EQ(OK, result = zinc.root_region.readFile(
		TestResources::getLocation(TestResources::FIELDMODULE_ALLSHAPES_RESOURCE)));

	zinc.fm.beginChange();
	const double one = 1.0;
	Field integrandField = zinc.fm.createFieldConstant(1, &one);
	EXPECT_TRUE(integrandField.isValid());
	Field coordinateField = zinc.fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinateField.isValid());

	Mesh mesh3d = zinc.fm.findMeshByDimension(3);
	EXPECT_TRUE(mesh3d.isValid());
	// create mesh group containing exterior faces for integrating to get surface area
	Mesh masterMesh2d = zinc.fm.findMeshByDimension(2);
	EXPECT_TRUE(masterMesh2d.isValid());
	FieldElementGroup elementGroup = zinc.fm.createFieldElementGroup(masterMesh2d);
	EXPECT_TRUE(elementGroup.isValid());
	MeshGroup exteriorMesh2d = elementGroup.getMeshGroup();
	EXPECT_TRUE(exteriorMesh2d.isValid());
	const int exteriorFaceIdentifiers[] = { 2,4,5,8,9,10,12,13,14,16,17,18,19,20,21,22,23,24 };
	const int size = sizeof(exteriorFaceIdentifiers)/sizeof(int);
	for (int i = 0; i < size; ++i)
	{
		Element face = masterMesh2d.findElementByIdentifier(exteriorFaceIdentifiers[i]);
		EXPECT_TRUE(face.isValid());
		EXPECT_EQ(OK, result = exteriorMesh2d.addElement(face));
	}
	zinc.fm.endChange();

	Mesh mesh1d = zinc.fm.findMeshByDimension(1);
	EXPECT_TRUE(mesh1d.isValid());

	FieldMeshIntegral volumeField = zinc.fm.createFieldMeshIntegral(integrandField, coordinateField, mesh3d);
	EXPECT_TRUE(volumeField.isValid());
	EXPECT_EQ(Element::QUADRATURE_RULE_GAUSSIAN, volumeField.getElementQuadratureRule());
	FieldMeshIntegral surfaceAreaField = zinc.fm.createFieldMeshIntegral(integrandField, coordinateField, exteriorMesh2d);
	EXPECT_TRUE(surfaceAreaField.isValid());
	EXPECT_EQ(Element::QUADRATURE_RULE_GAUSSIAN, surfaceAreaField.getElementQuadratureRule());
	FieldMeshIntegral lengthField = zinc.fm.createFieldMeshIntegral(integrandField, coordinateField, mesh1d);
	EXPECT_TRUE(lengthField.isValid());
	EXPECT_EQ(Element::QUADRATURE_RULE_GAUSSIAN, lengthField.getElementQuadratureRule());

	const double scaleFactors[3] = { 1.5, 0.75, 2.0 };
	Field scalingField = zinc.fm.createFieldConstant(3, scaleFactors);
	EXPECT_TRUE(scalingField.isValid());
	Field scaledCoordinateField = zinc.fm.createFieldMultiply(coordinateField, scalingField);
	EXPECT_TRUE(scaledCoordinateField.isValid());

	FieldMeshIntegral scaledVolumeField = zinc.fm.createFieldMeshIntegral(integrandField, scaledCoordinateField, mesh3d);
	EXPECT_TRUE(scaledVolumeField.isValid());
	EXPECT_EQ(Element::QUADRATURE_RULE_GAUSSIAN, scaledVolumeField.getElementQuadratureRule());
	FieldMeshIntegral scaledSurfaceAreaField = zinc.fm.createFieldMeshIntegral(integrandField, scaledCoordinateField, exteriorMesh2d);
	EXPECT_TRUE(scaledSurfaceAreaField.isValid());
	EXPECT_EQ(Element::QUADRATURE_RULE_GAUSSIAN, scaledSurfaceAreaField.getElementQuadratureRule());
	FieldMeshIntegral scaledLengthField = zinc.fm.createFieldMeshIntegral(integrandField, scaledCoordinateField, mesh1d);
	EXPECT_TRUE(scaledLengthField.isValid());
	EXPECT_EQ(Element::QUADRATURE_RULE_GAUSSIAN, scaledLengthField.getElementQuadratureRule());

	const double expectedVolume = 3.0 + 1.0/6.0;
	const double sqrt_2 = sqrt(2.0);
	const double expectedSurfaceArea = 11.5 + sqrt_2*(2.0 + 0.5*sqrt(0.5 + 1.0));

	const double expectedScaledVolume = expectedVolume*scaleFactors[0]*scaleFactors[1]*scaleFactors[2];
	const double expectedScaledSurfaceArea = 26.910372866754685;

	const double expectedLength = 26.0 + 7.0*sqrt_2;
	const double expectedScaledLength = 49.553154822033292;

	double volume = 0, scaledVolume = 0;
	double surfaceArea = 0, scaledSurfaceArea = 0;
	double length = 0, scaledLength = 0;
	const double tolerance = 1.0E-12;

	Fieldcache cache = zinc.fm.createFieldcache();
	EXPECT_TRUE(cache.isValid());

	int numbersOfPoints[3];
	for (numbersOfPoints[0] = 1; numbersOfPoints[0] <= 4; ++numbersOfPoints[0])
		for (numbersOfPoints[1] = 1; numbersOfPoints[1] <= 4; ++numbersOfPoints[1])
			for (numbersOfPoints[2] = 1; numbersOfPoints[2] <= 4; ++numbersOfPoints[2])
			{
				EXPECT_EQ(OK, volumeField.setNumbersOfPoints(3, numbersOfPoints));
				EXPECT_EQ(OK, surfaceAreaField.setNumbersOfPoints(3, numbersOfPoints));
				EXPECT_EQ(OK, lengthField.setNumbersOfPoints(3, numbersOfPoints));
				EXPECT_EQ(OK, scaledVolumeField.setNumbersOfPoints(3, numbersOfPoints));
				EXPECT_EQ(OK, scaledSurfaceAreaField.setNumbersOfPoints(3, numbersOfPoints));
				EXPECT_EQ(OK, scaledLengthField.setNumbersOfPoints(3, numbersOfPoints));

				EXPECT_EQ(OK, volumeField.evaluateReal(cache, 1, &volume));
				EXPECT_NEAR(expectedVolume, volume, tolerance);
				EXPECT_EQ(OK, surfaceAreaField.evaluateReal(cache, 1, &surfaceArea));
				EXPECT_NEAR(expectedSurfaceArea, surfaceArea, tolerance);
				EXPECT_EQ(OK, lengthField.evaluateReal(cache, 1, &length));
				EXPECT_NEAR(expectedLength, length, tolerance);

				EXPECT_EQ(OK, scaledVolumeField.evaluateReal(cache, 1, &scaledVolume));
				EXPECT_NEAR(expectedScaledVolume, scaledVolume, tolerance);
				EXPECT_EQ(OK, scaledSurfaceAreaField.evaluateReal(cache, 1, &scaledSurfaceArea));
				EXPECT_NEAR(expectedScaledSurfaceArea, scaledSurfaceArea, tolerance);
				EXPECT_EQ(OK, scaledLengthField.evaluateReal(cache, 1, &scaledLength));
				EXPECT_NEAR(expectedScaledLength, scaledLength, tolerance);
			}

	// test per-element evaluation to get element volumes
	EXPECT_EQ(6, mesh3d.getSize());
	const double expectedElementVolumes[6] = { 0.5, 0.5, 0.5, 0.5, 1.0, 1.0/6.0 };
	double elementVolume;
	for (int numberOfPoints = 1; numberOfPoints <= 4; ++numberOfPoints)
	{
		EXPECT_EQ(OK, volumeField.setNumbersOfPoints(1, &numberOfPoints));
		for (int i = 0; i < 6; ++i)
		{
			Element element = mesh3d.findElementByIdentifier(i + 1);
			EXPECT_TRUE(element.isValid());
			EXPECT_EQ(OK, cache.setElement(element));
			EXPECT_EQ(OK, volumeField.evaluateReal(cache, 1, &elementVolume));
			EXPECT_NEAR(expectedElementVolumes[i], elementVolume, tolerance);
		}
	}

	// test midpoint quadrature

	EXPECT_EQ(OK, volumeField.setElementQuadratureRule(Element::QUADRATURE_RULE_MIDPOINT));
	EXPECT_EQ(Element::QUADRATURE_RULE_MIDPOINT, volumeField.getElementQuadratureRule());
	EXPECT_EQ(OK, surfaceAreaField.setElementQuadratureRule(Element::QUADRATURE_RULE_MIDPOINT));
	EXPECT_EQ(Element::QUADRATURE_RULE_MIDPOINT, surfaceAreaField.getElementQuadratureRule());
	EXPECT_EQ(OK, lengthField.setElementQuadratureRule(Element::QUADRATURE_RULE_MIDPOINT));
	EXPECT_EQ(Element::QUADRATURE_RULE_MIDPOINT, lengthField.getElementQuadratureRule());

	EXPECT_EQ(OK, scaledVolumeField.setElementQuadratureRule(Element::QUADRATURE_RULE_MIDPOINT));
	EXPECT_EQ(Element::QUADRATURE_RULE_MIDPOINT, scaledVolumeField.getElementQuadratureRule());
	EXPECT_EQ(OK, scaledSurfaceAreaField.setElementQuadratureRule(Element::QUADRATURE_RULE_MIDPOINT));
	EXPECT_EQ(Element::QUADRATURE_RULE_MIDPOINT, scaledSurfaceAreaField.getElementQuadratureRule());
	EXPECT_EQ(OK, scaledLengthField.setElementQuadratureRule(Element::QUADRATURE_RULE_MIDPOINT));
	EXPECT_EQ(Element::QUADRATURE_RULE_MIDPOINT, scaledLengthField.getElementQuadratureRule());

	// must clear cache location otherwise evaluating at the last element
	cache.clearLocation();

	for (numbersOfPoints[0] = 1; numbersOfPoints[0] <= 2; ++numbersOfPoints[0])
		for (numbersOfPoints[1] = 3; numbersOfPoints[1] <= 4; ++numbersOfPoints[1])
			for (numbersOfPoints[2] = 5; numbersOfPoints[2] <= 6; ++numbersOfPoints[2])
			{
				EXPECT_EQ(OK, volumeField.setNumbersOfPoints(3, numbersOfPoints));
				EXPECT_EQ(OK, surfaceAreaField.setNumbersOfPoints(3, numbersOfPoints));
				EXPECT_EQ(OK, lengthField.setNumbersOfPoints(3, numbersOfPoints));
				EXPECT_EQ(OK, scaledVolumeField.setNumbersOfPoints(3, numbersOfPoints));
				EXPECT_EQ(OK, scaledSurfaceAreaField.setNumbersOfPoints(3, numbersOfPoints));
				EXPECT_EQ(OK, scaledLengthField.setNumbersOfPoints(3, numbersOfPoints));

				EXPECT_EQ(OK, volumeField.evaluateReal(cache, 1, &volume));
				EXPECT_NEAR(expectedVolume, volume, tolerance);
				EXPECT_EQ(OK, surfaceAreaField.evaluateReal(cache, 1, &surfaceArea));
				EXPECT_NEAR(expectedSurfaceArea, surfaceArea, tolerance);
				EXPECT_EQ(OK, lengthField.evaluateReal(cache, 1, &length));
				EXPECT_NEAR(expectedLength, length, tolerance);

				EXPECT_EQ(OK, scaledVolumeField.evaluateReal(cache, 1, &scaledVolume));
				EXPECT_NEAR(expectedScaledVolume, scaledVolume, tolerance);
				EXPECT_EQ(OK, scaledSurfaceAreaField.evaluateReal(cache, 1, &scaledSurfaceArea));
				EXPECT_NEAR(expectedScaledSurfaceArea, scaledSurfaceArea, tolerance);
				EXPECT_EQ(OK, scaledLengthField.evaluateReal(cache, 1, &scaledLength));
				EXPECT_NEAR(expectedScaledLength, scaledLength, tolerance);
			}

	// non-linear scaling
	Field pair[2];
	const double two_constants[2] = { 1.5, 2.0 };
	pair[0] = zinc.fm.createFieldConstant(2, two_constants);
	EXPECT_TRUE(pair[0].isValid());
	Field xField = zinc.fm.createFieldComponent(coordinateField, 1);
	EXPECT_TRUE(xField.isValid());
	const double xScale = 0.75;
	Field xScaleField = zinc.fm.createFieldConstant(1, &xScale);
	EXPECT_TRUE(xScaleField.isValid());
	Field scaledxField = zinc.fm.createFieldMultiply(xField, xScaleField);
	EXPECT_TRUE(scaledxField.isValid());
	Field cosx = zinc.fm.createFieldCos(scaledxField);
	pair[1] = cosx;
	EXPECT_TRUE(pair[1].isValid());
	Field nonLinearScalingField = zinc.fm.createFieldConcatenate(2, pair);
	Field nonLinearScaledCoordinateField = zinc.fm.createFieldMultiply(coordinateField, nonLinearScalingField);
	EXPECT_TRUE(nonLinearScaledCoordinateField.isValid());

	FieldMeshIntegral nonLinearScaledVolumeField = zinc.fm.createFieldMeshIntegral(integrandField, nonLinearScaledCoordinateField, mesh3d);
	EXPECT_TRUE(nonLinearScaledVolumeField.isValid());
	FieldMeshIntegral nonLinearScaledSurfaceAreaField = zinc.fm.createFieldMeshIntegral(integrandField, nonLinearScaledCoordinateField, exteriorMesh2d);
	EXPECT_TRUE(nonLinearScaledSurfaceAreaField.isValid());

	const double constProduct = two_constants[0]*two_constants[1];
	Field constProductField = zinc.fm.createFieldConstant(1, &constProduct);
	Field cosxScaling = zinc.fm.createFieldMultiply(cosx, constProductField);
	FieldMeshIntegral nonLinearScaledVolumeField2 = zinc.fm.createFieldMeshIntegral(cosxScaling, coordinateField, mesh3d);
	EXPECT_TRUE(nonLinearScaledVolumeField2.isValid());
	FieldMeshIntegral nonLinearScaledSurfaceAreaField2 = zinc.fm.createFieldMeshIntegral(cosxScaling, coordinateField, exteriorMesh2d);
	EXPECT_TRUE(nonLinearScaledSurfaceAreaField2.isValid());

#ifdef OLD_CODE
	// old values for xScale = 1.0, ie. cosx = cos(xField) directly.
	// which had negative volume in part of one element
	const double expectedNonLinearScaledVolume[4] =
		{ 7.0559053802963243, 6.8104343003626475, 6.8106518841869459, 6.8111846989802229 };
	const double expectedNonLinearScaledSurfaceArea[4] =
		{ 34.143865450652747,	34.256878457593942,	34.225521083053025,	34.240971095640752 };
	double expectedNonLinearScaledSurfaceArea2[4] =
		{	28.225665628446976, 27.278460970304074, 27.281297642206969, 27.282378561895456 };
#endif
	const double expectedNonLinearScaledVolume[4] =
		{ 8.0633827703553891, 7.9076251581212436, 7.9076684902556282, 7.9078818241699338 };
	const double expectedNonLinearScaledSurfaceArea[4] =
		{ 35.077246331415274, 35.021495747953381, 35.021673194493403, 35.021707031197636 };
	double expectedNonLinearScaledSurfaceArea2[4] =
		{ 36.079008317562128, 35.392219543449272, 35.393251193873191, 35.393732818145985 };

	double nonLinearScaledVolume = 0;
	double nonLinearScaledSurfaceArea = 0;
	for (int nPoints = 1; nPoints <= 4; ++nPoints)
	{
		EXPECT_EQ(OK, nonLinearScaledVolumeField.setNumbersOfPoints(1, &nPoints));
		EXPECT_EQ(OK, nonLinearScaledSurfaceAreaField.setNumbersOfPoints(1, &nPoints));
		EXPECT_EQ(OK, nonLinearScaledVolumeField2.setNumbersOfPoints(1, &nPoints));
		EXPECT_EQ(OK, nonLinearScaledSurfaceAreaField2.setNumbersOfPoints(1, &nPoints));

		EXPECT_EQ(OK, nonLinearScaledVolumeField.evaluateReal(cache, 1, &nonLinearScaledVolume));
		EXPECT_NEAR(expectedNonLinearScaledVolume[nPoints - 1], nonLinearScaledVolume, tolerance);
		EXPECT_EQ(OK, nonLinearScaledSurfaceAreaField.evaluateReal(cache, 1, &nonLinearScaledSurfaceArea));
		EXPECT_NEAR(expectedNonLinearScaledSurfaceArea[nPoints - 1], nonLinearScaledSurfaceArea, tolerance);

		EXPECT_EQ(OK, nonLinearScaledVolumeField2.evaluateReal(cache, 1, &nonLinearScaledVolume));
		EXPECT_NEAR(expectedNonLinearScaledVolume[nPoints - 1], nonLinearScaledVolume, tolerance);
		EXPECT_EQ(OK, nonLinearScaledSurfaceAreaField2.evaluateReal(cache, 1, &nonLinearScaledSurfaceArea));
		EXPECT_NEAR(expectedNonLinearScaledSurfaceArea2[nPoints - 1], nonLinearScaledSurfaceArea, tolerance);
	}
}

TEST(ZincFieldMeshIntegral, midpoint_quadrature_large_numbers)
{
	ZincTestSetupCpp zinc;
	int result;

	Mesh mesh3d = zinc.fm.findMeshByDimension(3);
	EXPECT_TRUE(mesh3d.isValid());
	Elementtemplate elementTemplate = mesh3d.createElementtemplate();
	EXPECT_TRUE(elementTemplate.isValid());
	EXPECT_EQ(OK, result = elementTemplate.setElementShapeType(Element::SHAPE_TYPE_CUBE));

	Element element = mesh3d.createElement(-1, elementTemplate);
	EXPECT_TRUE(element.isValid());

	Field xiField = zinc.fm.findFieldByName("xi");
	EXPECT_TRUE(xiField.isValid());

	const double one = 1.0;
	Field oneField = zinc.fm.createFieldConstant(1, &one);
	EXPECT_TRUE(oneField.isValid());
	Field magXiField = zinc.fm.createFieldMagnitude(xiField);
	EXPECT_TRUE(magXiField.isValid());
	Field magXi_lt_oneField = zinc.fm.createFieldLessThan(magXiField, oneField);
	EXPECT_TRUE(magXi_lt_oneField.isValid());

	FieldMeshIntegral volumeField = zinc.fm.createFieldMeshIntegral(magXi_lt_oneField, xiField, mesh3d);
	EXPECT_TRUE(volumeField.isValid());
	EXPECT_EQ(OK, result = volumeField.setElementQuadratureRule(Element::QUADRATURE_RULE_MIDPOINT));

	for (int np = 0; np <= 8; np += 4)
	{
		// calculate volume within |xi| < 1.0, discretely sampled
		const int nPoints1 = 4 + np;
		const int nPoints2 = 6 + np;
		const int nPoints3 = 8 + np;
		double expectedVolume = 0.0;
		for (int n1 = 0; n1 < nPoints1; ++n1)
		{
			double xi1 = (n1 + 0.5) / static_cast<double>(nPoints1);
			for (int n2 = 0; n2 < nPoints2; ++n2)
			{
				double xi2 = (n2 + 0.5) / static_cast<double>(nPoints2);
				for (int n3 = 0; n3 < nPoints3; ++n3)
				{
					double xi3 = (n3 + 0.5) / static_cast<double>(nPoints3);
					double mag = sqrt(xi1*xi1 + xi2*xi2 + xi3*xi3);
					if (mag < 1.0)
						expectedVolume += 1.0;
				}
			}
		}
		expectedVolume /= static_cast<double>(nPoints1*nPoints2*nPoints3);

		const int numbersOfPoints[3] = { nPoints1, nPoints2, nPoints3 };
		EXPECT_EQ(OK, result = volumeField.setNumbersOfPoints(3, numbersOfPoints));

		Fieldcache cache = zinc.fm.createFieldcache();
		double volume;
		EXPECT_EQ(OK, result = volumeField.evaluateReal(cache, 1, &volume));
		const double tolerance = 1.0E-12;
		EXPECT_NEAR(expectedVolume, volume, tolerance);
	}
}

TEST(ZincFieldMeshIntegralSquares, quadrature)
{
	ZincTestSetupCpp zinc;
	int result;

	EXPECT_EQ(OK, result = zinc.root_region.readFile(
		TestResources::getLocation(TestResources::FIELDMODULE_ALLSHAPES_RESOURCE)));

	zinc.fm.beginChange();
	const double one_two[] = { 1.0, 2.0 };
	Field integrandField = zinc.fm.createFieldConstant(2, one_two);
	EXPECT_TRUE(integrandField.isValid());
	Field coordinateField = zinc.fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinateField.isValid());

	Mesh mesh3d = zinc.fm.findMeshByDimension(3);
	EXPECT_TRUE(mesh3d.isValid());

	FieldMeshIntegralSquares integralSquaresField = zinc.fm.createFieldMeshIntegralSquares(integrandField, coordinateField, mesh3d);
	EXPECT_TRUE(integralSquaresField.isValid());
	EXPECT_EQ(Element::QUADRATURE_RULE_GAUSSIAN, integralSquaresField.getElementQuadratureRule());

	const double expectedValue[] = { 3.0 + 1.0/6.0, 4.0*(3.0 + 1.0/6.0) };
	double value[2];
	const double tolerance = 1.0E-12;

	int numbersOfPoints[3];
	for (numbersOfPoints[0] = 1; numbersOfPoints[0] <= 4; ++numbersOfPoints[0])
		for (numbersOfPoints[1] = 1; numbersOfPoints[1] <= 4; ++numbersOfPoints[1])
			for (numbersOfPoints[2] = 1; numbersOfPoints[2] <= 4; ++numbersOfPoints[2])
			{
				Fieldcache cache = zinc.fm.createFieldcache();
				EXPECT_TRUE(cache.isValid());
				EXPECT_EQ(OK, integralSquaresField.setNumbersOfPoints(3, numbersOfPoints));

				EXPECT_EQ(OK, integralSquaresField.evaluateReal(cache, 2, value));
				EXPECT_NEAR(expectedValue[0], value[0], tolerance);
				EXPECT_NEAR(expectedValue[1], value[1], tolerance);
			}
	zinc.fm.endChange();
}

// Test issue evaluating timeLookup field at indexed locations such as used
// by mesh integral, and graphics. Was copying non-indexed location to extra cache.
TEST(FieldMeshIntegral, timeLookup)
{
	ZincTestSetupCpp zinc;

	EXPECT_EQ(RESULT_OK, zinc.root_region.readFile(TestResources::getLocation(TestResources::FIELDMODULE_CUBE_TRICUBIC_DEFORMED_RESOURCE)));

	Field deformed = zinc.fm.findFieldByName("deformed");
	EXPECT_TRUE(deformed.isValid());
	Mesh mesh3d = zinc.fm.findMeshByDimension(3);
	EXPECT_TRUE(mesh3d.isValid());
	Element element = mesh3d.findElementByIdentifier(1);
	EXPECT_TRUE(element.isValid());

	const double valueOne = 1.0;
	Field one = zinc.fm.createFieldConstant(1, &valueOne);
	const int numberOfPoints = 4;

	FieldMeshIntegral deformedVolume = zinc.fm.createFieldMeshIntegralSquares(one, deformed, mesh3d);
	EXPECT_TRUE(deformedVolume.isValid());
	EXPECT_EQ(RESULT_OK, deformedVolume.setElementQuadratureRule(Element::QUADRATURE_RULE_GAUSSIAN));
	EXPECT_EQ(RESULT_OK, deformedVolume.setNumbersOfPoints(1, &numberOfPoints));

	FieldTimeLookup deformedOne = zinc.fm.createFieldTimeLookup(deformed, one);
	EXPECT_TRUE(deformedOne.isValid());

	FieldMeshIntegral deformedOneVolume = zinc.fm.createFieldMeshIntegralSquares(one, deformedOne, mesh3d);
	EXPECT_TRUE(deformedOneVolume.isValid());
	EXPECT_EQ(RESULT_OK, deformedOneVolume.setElementQuadratureRule(Element::QUADRATURE_RULE_GAUSSIAN));
	EXPECT_EQ(RESULT_OK, deformedOneVolume.setNumbersOfPoints(1, &numberOfPoints));

	Fieldcache fieldcache = zinc.fm.createFieldcache();
	EXPECT_TRUE(fieldcache.isValid());
	double volumeOut;
	const double TOL = 1.0E-10;
	EXPECT_EQ(RESULT_OK, deformedVolume.evaluateReal(fieldcache, 1, &volumeOut));
	EXPECT_NEAR(1.3298844582623588, volumeOut, TOL);
	EXPECT_EQ(RESULT_OK, deformedOneVolume.evaluateReal(fieldcache, 1, &volumeOut));
	EXPECT_NEAR(1.3298844582623588, volumeOut, TOL);
}
