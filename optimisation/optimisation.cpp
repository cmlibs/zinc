/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include "zinctestsetup.hpp"
#include <zinc/core.h>
#include <zinc/field.h>
#include <zinc/fieldconstant.h>
#include <zinc/optimisation.h>

#include "zinctestsetupcpp.hpp"
#include <zinc/field.hpp>
#include <zinc/fieldarithmeticoperators.hpp>
#include <zinc/fieldcache.hpp>
#include <zinc/fieldcomposite.hpp>
#include <zinc/fieldconstant.hpp>
#include <zinc/fieldderivatives.hpp>
#include <zinc/fieldmatrixoperators.hpp>
#include <zinc/fieldmeshoperators.hpp>
#include <zinc/fieldnodesetoperators.hpp>
#include <zinc/fieldsubobjectgroup.hpp>
#include <zinc/optimisation.hpp>

#include "test_resources.h"

TEST(cmzn_optimisation, valid_args)
{
	ZincTestSetup zinc;
	int result;

	cmzn_optimisation_id optimisation = cmzn_fieldmodule_create_optimisation(zinc.fm);
	EXPECT_NE(static_cast<cmzn_optimisation_id>(0), optimisation);

	EXPECT_EQ(CMZN_OPTIMISATION_METHOD_QUASI_NEWTON, cmzn_optimisation_get_method(optimisation));
	EXPECT_EQ(OK, result = cmzn_optimisation_set_method(optimisation, CMZN_OPTIMISATION_METHOD_LEAST_SQUARES_QUASI_NEWTON));
	EXPECT_EQ(CMZN_OPTIMISATION_METHOD_LEAST_SQUARES_QUASI_NEWTON, cmzn_optimisation_get_method(optimisation));

	cmzn_optimisation_destroy(&optimisation);
}

TEST(ZincOptimisation, valid_args)
{
	ZincTestSetupCpp zinc;
	int result;

	Optimisation optimisation = zinc.fm.createOptimisation();
	EXPECT_TRUE(optimisation.isValid());

	EXPECT_EQ(Optimisation::METHOD_QUASI_NEWTON, optimisation.getMethod());
	EXPECT_EQ(OK, result = optimisation.setMethod(Optimisation::METHOD_LEAST_SQUARES_QUASI_NEWTON));
	EXPECT_EQ(Optimisation::METHOD_LEAST_SQUARES_QUASI_NEWTON, optimisation.getMethod());
}

// A non-linear optimisation for deformation of a tricubic Lagrange unit cube.
// All nodes on end1 are fixed in x,y,z by a penalty
// All nodes on end2 are fixed to x=1.5 by a penalty
// Incompressible and strain penalties are applied at Gauss points.
// The solution proves that the cube gets thinner, symmetrically at end2 as it extends
TEST(ZincOptimisation, tricubicFit)
{
	ZincTestSetupCpp zinc;
	int result;

	EXPECT_EQ(OK, result = zinc.root_region.readFile(
		TestResources::getLocation(TestResources::OPTIMISATION_CUBE_TRICUBIC_LAGRANGE_RESOURCE)));
	Field referenceCoordinates = zinc.fm.findFieldByName("coordinates");
	EXPECT_TRUE(referenceCoordinates.isValid());
	EXPECT_EQ(OK, referenceCoordinates.setName("reference_coordinates"));
	EXPECT_EQ(OK, result = zinc.root_region.readFile(
		TestResources::getLocation(TestResources::OPTIMISATION_CUBE_TRICUBIC_LAGRANGE_RESOURCE)));
	Field coordinates = zinc.fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinates.isValid());

	zinc.fm.beginChange();
	Nodeset nodeset = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_TRUE(nodeset.isValid());
	const int end1NodeIdentifiers[] = { 1,5,9,13,17,21,25,29,33,37,41,45,49,53,57,61 };
	const int end1NodeIdentifiersSize = sizeof(end1NodeIdentifiers)/sizeof(int);
	FieldNodeGroup end1NodeGroup = zinc.fm.createFieldNodeGroup(nodeset);
	NodesetGroup end1Nodeset = end1NodeGroup.getNodesetGroup();
	EXPECT_TRUE(end1Nodeset.isValid());
	for (int i = 0; i < end1NodeIdentifiersSize; ++i)
	{
		Node node = nodeset.findNodeByIdentifier(end1NodeIdentifiers[i]);
		EXPECT_TRUE(node.isValid());
		EXPECT_EQ(OK, result = end1Nodeset.addNode(node));
	}
	const int end2NodeIdentifiers[] = { 4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,64 };
	const int end2NodeIdentifiersSize = sizeof(end2NodeIdentifiers)/sizeof(int);
	FieldNodeGroup end2NodeGroup = zinc.fm.createFieldNodeGroup(nodeset);
	NodesetGroup end2Nodeset = end2NodeGroup.getNodesetGroup();
	EXPECT_TRUE(end2Nodeset.isValid());
	for (int i = 0; i < end2NodeIdentifiersSize; ++i)
	{
		Node node = nodeset.findNodeByIdentifier(end2NodeIdentifiers[i]);
		EXPECT_TRUE(node.isValid());
		EXPECT_EQ(OK, result = end2Nodeset.addNode(node));
	}
	zinc.fm.endChange();

	Field displacement = zinc.fm.createFieldSubtract(coordinates, referenceCoordinates);

	const double penalty1Value = 10.0;
	FieldConstant penalty1 = zinc.fm.createFieldConstant(1, &penalty1Value);
	FieldNodesetSumSquares end1Objective = zinc.fm.createFieldNodesetSumSquares(penalty1*displacement, end1Nodeset);
	EXPECT_TRUE(end1Objective.isValid());

	FieldComponent x = zinc.fm.createFieldComponent(coordinates, 1);
	const double xEndValue = 1.5;
	FieldConstant xEnd = zinc.fm.createFieldConstant(1, &xEndValue);
	FieldNodesetSumSquares end2Objective = zinc.fm.createFieldNodesetSumSquares(penalty1*(x - xEnd), end2Nodeset);
	EXPECT_TRUE(end2Objective.isValid());

	const double alphaValue = 0.2;
	Field alpha = zinc.fm.createFieldConstant(1, &alphaValue);
	Field displacementGradient = zinc.fm.createFieldGradient(displacement, referenceCoordinates);

	Field F = zinc.fm.createFieldGradient(coordinates, referenceCoordinates);
	Field detF = zinc.fm.createFieldDeterminant(F);
	const double oneValue = 1.0;
	Field one = zinc.fm.createFieldConstant(1, &oneValue);

	Field WComponents[] = { alpha*displacementGradient, penalty1*(detF - one) };
	Field W = zinc.fm.createFieldConcatenate(2, WComponents);

	Mesh mesh3d = zinc.fm.findMeshByDimension(3);
	FieldMeshIntegralSquares WObjective = zinc.fm.createFieldMeshIntegralSquares(W, referenceCoordinates, mesh3d);
	EXPECT_TRUE(WObjective.isValid());
	const int numGaussPoints = 4;
	EXPECT_EQ(OK, result = WObjective.setNumbersOfPoints(1, &numGaussPoints));

	Optimisation optimisation = zinc.fm.createOptimisation();
	EXPECT_TRUE(optimisation.isValid());

	EXPECT_EQ(OK, result = optimisation.addObjectiveField(end1Objective));
	EXPECT_EQ(OK, result = optimisation.addObjectiveField(end2Objective));
	EXPECT_EQ(OK, result = optimisation.addObjectiveField(WObjective));
	EXPECT_EQ(OK, result = optimisation.addIndependentField(coordinates));
	EXPECT_EQ(OK, result = optimisation.setMethod(Optimisation::METHOD_LEAST_SQUARES_QUASI_NEWTON));
	EXPECT_EQ(OK, result = optimisation.setAttributeInteger(Optimisation::ATTRIBUTE_MAXIMUM_ITERATIONS, 10));

	EXPECT_EQ(OK, result = optimisation.optimise());
	char *solutionReport = optimisation.getSolutionReport();
	EXPECT_NE((char *)0, solutionReport);
	printf("%s", solutionReport);
	const char *dimensionText = strstr(solutionReport,
		"Dimension of the problem  = 192");
	EXPECT_NE((const char *)0, dimensionText);
	const char *convergedText = strstr(solutionReport,
		"Return code               = 2 (Algorithm converged - Difference in successive fcn values is less than tolerance)");
	EXPECT_NE((const char *)0, convergedText);
	const char *iterationsText = strstr(solutionReport,
		"No. iterations taken      = 5");
	EXPECT_NE((const char *)0, iterationsText);

	Fieldcache cache = zinc.fm.createFieldcache();
	EXPECT_TRUE(cache.isValid());
	const double expectedWValues[10] =
	{
		0.011447361785109545,
		1.1123649670134983e-006,
		1.1123718443471489e-006,
		0.00039367551802260676,
		0.0014073263139825962,
		5.6615827265801324e-006,
		0.00039367550896460436,
		5.6615832637350045e-006,
		0.0014073262275251103,
		2.4282421107936186e-006
	};
	double WValues[10];
	const double tolerance = 1.0E-12;
	EXPECT_EQ(OK, result = WObjective.evaluateReal(cache, 10, WValues));
	for (int i = 0; i < 10; ++i)
		EXPECT_NEAR(expectedWValues[i], WValues[i], tolerance);

	FieldMeshIntegral volume = zinc.fm.createFieldMeshIntegral(one, coordinates, mesh3d);
	EXPECT_EQ(OK, volume.setNumbersOfPoints(1, &numGaussPoints));
	const double expectedVolumeValue = 1.0001085604484732;
	double volumeValue;
	EXPECT_EQ(OK, result = volume.evaluateReal(cache, 1, &volumeValue));
	EXPECT_NEAR(expectedVolumeValue, volumeValue, tolerance);

	cmzn_deallocate(solutionReport);
}