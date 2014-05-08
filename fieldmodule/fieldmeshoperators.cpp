/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cmath>
#include <gtest/gtest.h>

#include <zinc/element.hpp>
#include <zinc/field.hpp>
#include <zinc/fieldarithmeticoperators.hpp>
#include <zinc/fieldcache.hpp>
#include <zinc/fieldcomposite.hpp>
#include <zinc/fieldconstant.hpp>
#include <zinc/fieldmeshoperators.hpp>
#include <zinc/fieldsubobjectgroup.hpp>
#include <zinc/fieldtrigonometry.hpp>
#include "zinctestsetupcpp.hpp"

#include "test_resources.h"

TEST(ZincFieldMeshIntegral, integration)
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

	FieldMeshIntegral volumeField = zinc.fm.createFieldMeshIntegral(integrandField, coordinateField, mesh3d);
	EXPECT_TRUE(volumeField.isValid());
	FieldMeshIntegral surfaceAreaField = zinc.fm.createFieldMeshIntegral(integrandField, coordinateField, exteriorMesh2d);
	EXPECT_TRUE(surfaceAreaField.isValid());

	const double scaleFactors[3] = { 1.5, 0.75, 2.0 };
	Field scalingField = zinc.fm.createFieldConstant(3, scaleFactors);
	EXPECT_TRUE(scalingField.isValid());
	Field scaledCoordinateField = zinc.fm.createFieldMultiply(coordinateField, scalingField);
	EXPECT_TRUE(scaledCoordinateField.isValid());

	FieldMeshIntegral scaledVolumeField = zinc.fm.createFieldMeshIntegral(integrandField, scaledCoordinateField, mesh3d);
	EXPECT_TRUE(scaledVolumeField.isValid());
	FieldMeshIntegral scaledSurfaceAreaField = zinc.fm.createFieldMeshIntegral(integrandField, scaledCoordinateField, exteriorMesh2d);
	EXPECT_TRUE(scaledSurfaceAreaField.isValid());

	// non-linear scaling
	Field pair[2];
	const double two_constants[2] = { 1.5, 2.0 };
	pair[0] = zinc.fm.createFieldConstant(2, two_constants);
	EXPECT_TRUE(pair[0].isValid());
	Field x = zinc.fm.createFieldComponent(coordinateField, 1);
	EXPECT_TRUE(x.isValid());
	Field cosx = zinc.fm.createFieldCos(x);
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

	// Note: working around bug where cache is not invalidated after setting order

	const double expectedVolume = 3.0 + 1.0/6.0;
	const double sqrt_2 = sqrt(2.0);
	const double expectedSurfaceArea = 11.5 + sqrt_2*(2.0 + 0.5*sqrt(0.5 + 1.0));

	const double expectedScaledVolume = expectedVolume*scaleFactors[0]*scaleFactors[1]*scaleFactors[2];
	const double expectedScaledSurfaceArea = 26.910372866754685;

	const double expectedNonLinearScaledVolume[4] =
		{ 7.0559053802963243, 6.8104343003626475, 6.8106518841869459, 6.8111846989802229 };
	const double expectedNonLinearScaledSurfaceArea[4] =
		{ 34.143865450652747,	34.256878457593942,	34.225521083053025,	34.240971095640752 };
	double expectedNonLinearScaledSurfaceArea2[4] =
		{	28.225665628446976, 27.278460970304074, 27.281297642206969, 27.282378561895456 };

	double volume = 0, scaledVolume = 0, nonLinearScaledVolume = 0;
	double surfaceArea = 0, scaledSurfaceArea = 0, nonLinearScaledSurfaceArea = 0;
	const double tolerance = 1.0E-12;
	for (int order = 1; order <= 4; ++order)
	{
		Fieldcache cache = zinc.fm.createFieldcache();
		EXPECT_TRUE(cache.isValid());

		EXPECT_EQ(OK, volumeField.setOrder(order));
		EXPECT_EQ(OK, surfaceAreaField.setOrder(order));
		EXPECT_EQ(OK, scaledVolumeField.setOrder(order));
		EXPECT_EQ(OK, scaledSurfaceAreaField.setOrder(order));
		EXPECT_EQ(OK, nonLinearScaledVolumeField.setOrder(order));
		EXPECT_EQ(OK, nonLinearScaledSurfaceAreaField.setOrder(order));
		EXPECT_EQ(OK, nonLinearScaledVolumeField2.setOrder(order));
		EXPECT_EQ(OK, nonLinearScaledSurfaceAreaField2.setOrder(order));

		EXPECT_EQ(OK, volumeField.evaluateReal(cache, 1, &volume));
		EXPECT_NEAR(expectedVolume, volume, tolerance);
		EXPECT_EQ(OK, surfaceAreaField.evaluateReal(cache, 1, &surfaceArea));
		EXPECT_NEAR(expectedSurfaceArea, surfaceArea, tolerance);

		EXPECT_EQ(OK, scaledVolumeField.evaluateReal(cache, 1, &scaledVolume));
		EXPECT_NEAR(expectedScaledVolume, scaledVolume, tolerance);
		EXPECT_EQ(OK, scaledSurfaceAreaField.evaluateReal(cache, 1, &scaledSurfaceArea));
		EXPECT_NEAR(expectedScaledSurfaceArea, scaledSurfaceArea, tolerance);

		EXPECT_EQ(OK, nonLinearScaledVolumeField.evaluateReal(cache, 1, &nonLinearScaledVolume));
		EXPECT_NEAR(expectedNonLinearScaledVolume[order - 1], nonLinearScaledVolume, tolerance);
		EXPECT_EQ(OK, nonLinearScaledSurfaceAreaField.evaluateReal(cache, 1, &nonLinearScaledSurfaceArea));
		EXPECT_NEAR(expectedNonLinearScaledSurfaceArea[order - 1], nonLinearScaledSurfaceArea, tolerance);

		EXPECT_EQ(OK, nonLinearScaledVolumeField2.evaluateReal(cache, 1, &nonLinearScaledVolume));
		EXPECT_NEAR(expectedNonLinearScaledVolume[order - 1], nonLinearScaledVolume, tolerance);
		EXPECT_EQ(OK, nonLinearScaledSurfaceAreaField2.evaluateReal(cache, 1, &nonLinearScaledSurfaceArea));
		EXPECT_NEAR(expectedNonLinearScaledSurfaceArea2[order - 1], nonLinearScaledSurfaceArea, tolerance);
	}
}
