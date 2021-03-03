/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include "zinctestsetup.hpp"
#include <opencmiss/zinc/core.h>
#include <opencmiss/zinc/field.h>
#include <opencmiss/zinc/fieldconstant.h>
#include <opencmiss/zinc/optimisation.h>

#include "zinctestsetupcpp.hpp"
#include <opencmiss/zinc/field.hpp>
#include <opencmiss/zinc/fieldarithmeticoperators.hpp>
#include <opencmiss/zinc/fieldassignment.hpp>
#include <opencmiss/zinc/fieldcache.hpp>
#include <opencmiss/zinc/fieldcomposite.hpp>
#include <opencmiss/zinc/fieldconstant.hpp>
#include <opencmiss/zinc/fieldderivatives.hpp>
#include <opencmiss/zinc/fieldfiniteelement.hpp>
#include <opencmiss/zinc/fieldlogicaloperators.hpp>
#include <opencmiss/zinc/fieldmatrixoperators.hpp>
#include <opencmiss/zinc/fieldmeshoperators.hpp>
#include <opencmiss/zinc/fieldnodesetoperators.hpp>
#include <opencmiss/zinc/fieldsubobjectgroup.hpp>
#include <opencmiss/zinc/fieldvectoroperators.hpp>
#include <opencmiss/zinc/optimisation.hpp>

#include "test_resources.h"

TEST(cmzn_optimisation, arguments)
{
	ZincTestSetup zinc;
	int result;

	cmzn_optimisation_id optimisation = cmzn_fieldmodule_create_optimisation(zinc.fm);
	EXPECT_NE(static_cast<cmzn_optimisation_id>(0), optimisation);

	EXPECT_EQ(CMZN_OPTIMISATION_METHOD_QUASI_NEWTON, cmzn_optimisation_get_method(optimisation));
	EXPECT_EQ(OK, result = cmzn_optimisation_set_method(optimisation, CMZN_OPTIMISATION_METHOD_LEAST_SQUARES_QUASI_NEWTON));
	EXPECT_EQ(CMZN_OPTIMISATION_METHOD_LEAST_SQUARES_QUASI_NEWTON, cmzn_optimisation_get_method(optimisation));
	EXPECT_EQ(OK, result = cmzn_optimisation_set_method(optimisation, CMZN_OPTIMISATION_METHOD_NEWTON));
	EXPECT_EQ(CMZN_OPTIMISATION_METHOD_NEWTON, cmzn_optimisation_get_method(optimisation));

	cmzn_optimisation_destroy(&optimisation);
}

TEST(ZincOptimisation, arguments)
{
	ZincTestSetupCpp zinc;
	int result;

	Optimisation optimisation = zinc.fm.createOptimisation();
	EXPECT_TRUE(optimisation.isValid());

	EXPECT_EQ(Optimisation::METHOD_QUASI_NEWTON, optimisation.getMethod());
	EXPECT_EQ(OK, result = optimisation.setMethod(Optimisation::METHOD_LEAST_SQUARES_QUASI_NEWTON));
	EXPECT_EQ(Optimisation::METHOD_LEAST_SQUARES_QUASI_NEWTON, optimisation.getMethod());

	// made-up fields to test objective/independent field APIs
	FieldFiniteElement f1 = zinc.fm.createFieldFiniteElement(3);
	EXPECT_TRUE(f1.isValid());
	FieldFiniteElement f2 = zinc.fm.createFieldFiniteElement(1);
	EXPECT_TRUE(f2.isValid());
	FieldFiniteElement f3 = zinc.fm.createFieldFiniteElement(3);
	EXPECT_TRUE(f3.isValid());
	Field f4 = zinc.fm.createFieldMultiply(f1, f2);
	EXPECT_TRUE(f4.isValid());
	Field f5 = zinc.fm.createFieldMultiply(f4, f3);
	EXPECT_TRUE(f5.isValid());
	const double oneValue = 1.0;
	Field fcond = zinc.fm.createFieldConstant(1, &oneValue);
	EXPECT_TRUE(fcond.isValid());

	Field temp;
	EXPECT_EQ(OK, result = optimisation.addIndependentField(f1));
	EXPECT_EQ(OK, result = optimisation.addIndependentField(f2));
	EXPECT_EQ(OK, result = optimisation.addIndependentField(f3));
	EXPECT_EQ(ERROR_ARGUMENT, result = optimisation.addIndependentField(f1));
	temp = optimisation.getFirstIndependentField();
	EXPECT_EQ(f1, temp);
	temp = optimisation.getNextIndependentField(temp);
	EXPECT_EQ(f2, temp);
	temp = optimisation.getNextIndependentField(temp);
	EXPECT_EQ(f3, temp);
	temp = optimisation.getNextIndependentField(temp);
	EXPECT_FALSE(temp.isValid());
	temp = optimisation.getConditionalField(f1);
	EXPECT_FALSE(temp.isValid());
	temp = optimisation.getConditionalField(f2);
	EXPECT_FALSE(temp.isValid());
	temp = optimisation.getConditionalField(f3);
	EXPECT_FALSE(temp.isValid());
	EXPECT_EQ(ERROR_ARGUMENT, result = optimisation.setConditionalField(f2, f3)); // wrong #components
	EXPECT_EQ(OK, result = optimisation.setConditionalField(f2, fcond));
	temp = optimisation.getConditionalField(f2);
	EXPECT_EQ(fcond, temp);

	EXPECT_EQ(ERROR_ARGUMENT, result = optimisation.removeIndependentField(f4)); // not in use
	EXPECT_EQ(OK, result = optimisation.removeIndependentField(f1));
	temp = optimisation.getFirstIndependentField();
	EXPECT_EQ(f2, temp);
	temp = optimisation.getNextIndependentField(temp);
	EXPECT_EQ(f3, temp);
	temp = optimisation.getNextIndependentField(temp);
	EXPECT_FALSE(temp.isValid());
	temp = optimisation.getConditionalField(f2);
	EXPECT_EQ(fcond, temp);
	EXPECT_EQ(OK, result = optimisation.setConditionalField(f2, Field()));
	temp = optimisation.getConditionalField(f2);
	EXPECT_FALSE(temp.isValid());

	EXPECT_EQ(OK, result = optimisation.addObjectiveField(f4));
	EXPECT_EQ(OK, result = optimisation.addObjectiveField(f5));
	temp = optimisation.getFirstObjectiveField();
	EXPECT_EQ(f4, temp);
	temp = optimisation.getNextObjectiveField(temp);
	EXPECT_EQ(f5, temp);
	temp = optimisation.getNextObjectiveField(temp);
	EXPECT_FALSE(temp.isValid());

	EXPECT_EQ(ERROR_ARGUMENT, result = optimisation.removeObjectiveField(f1));
	EXPECT_EQ(OK, result = optimisation.removeObjectiveField(f5));
	temp = optimisation.getFirstObjectiveField();
	EXPECT_EQ(f4, temp);
	temp = optimisation.getNextObjectiveField(temp);
	EXPECT_FALSE(temp.isValid());
}

// A non-linear optimisation for deformation of a tricubic Lagrange unit cube.
// All nodes on end1 are fixed in x,y,z by being excluded as DOFs
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

	const double xConstValue = 0.1;
	FieldConstant xConst = zinc.fm.createFieldConstant(1, &xConstValue);
	EXPECT_TRUE(xConst.isValid());
	Field xCondition = x > xConst;
	EXPECT_TRUE(xCondition.isValid());

	Optimisation optimisation = zinc.fm.createOptimisation();
	EXPECT_TRUE(optimisation.isValid());

	EXPECT_EQ(OK, result = optimisation.addObjectiveField(end2Objective));
	EXPECT_EQ(OK, result = optimisation.addObjectiveField(WObjective));
	EXPECT_EQ(OK, result = optimisation.addIndependentField(coordinates));
	EXPECT_EQ(OK, result = optimisation.setConditionalField(coordinates, xCondition));
	EXPECT_EQ(OK, result = optimisation.setMethod(Optimisation::METHOD_LEAST_SQUARES_QUASI_NEWTON));
	EXPECT_EQ(OK, result = optimisation.setAttributeInteger(Optimisation::ATTRIBUTE_MAXIMUM_ITERATIONS, 10));

	EXPECT_EQ(OK, result = optimisation.optimise());
	char *solutionReport = optimisation.getSolutionReport();
	EXPECT_NE((char *)0, solutionReport);
	printf("%s", solutionReport);
	const char *dimensionText = strstr(solutionReport,
		"Dimension of the problem  = 144");
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
		0.011449513901451864,
		1.1196820095409539e-006,
		1.1196845955486078e-006,
		0.00039399120829730822,
		0.0014075399963259518,
		5.6687058307447950e-006,
		0.00039399120475952152,
		5.6687063105913627e-006,
		0.0014075399951463652,
		2.4300383826273377e-006
	};

	double WValues[10];
	const double tolerance = 1.0E-9;
	EXPECT_EQ(OK, result = WObjective.evaluateReal(cache, 10, WValues));
	for (int i = 0; i < 10; ++i)
		EXPECT_NEAR(expectedWValues[i], WValues[i], tolerance);

	FieldMeshIntegral volume = zinc.fm.createFieldMeshIntegral(one, coordinates, mesh3d);
	EXPECT_EQ(OK, volume.setNumbersOfPoints(1, &numGaussPoints));
	const double expectedVolumeValue = 1.0001085833573624;
	double volumeValue;
	EXPECT_EQ(OK, result = volume.evaluateReal(cache, 1, &volumeValue));
	EXPECT_NEAR(expectedVolumeValue, volumeValue, tolerance);

	cmzn_deallocate(solutionReport);
}

// Optimise z displacement of nodes on top of a linear cube to double
// its volume using optimisation with partial field assignment
TEST(ZincOptimisation, addFieldassignment)
{
	ZincTestSetupCpp zinc;
	int result;

	// read twice to get copy of coordinates in 'reference_coordinates'
	EXPECT_EQ(OK, result = zinc.root_region.readFile(
		TestResources::getLocation(TestResources::FIELDMODULE_CUBE_RESOURCE)));
	Field referenceCoordinates = zinc.fm.findFieldByName("coordinates");
	EXPECT_TRUE(referenceCoordinates.isValid());
	EXPECT_EQ(OK, referenceCoordinates.setName("reference_coordinates"));
	EXPECT_EQ(OK, result = zinc.root_region.readFile(
		TestResources::getLocation(TestResources::FIELDMODULE_CUBE_RESOURCE)));
	Field coordinates = zinc.fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinates.isValid());

	Nodeset nodes = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_TRUE(nodes.isValid());
	Mesh mesh3d = zinc.fm.findMeshByDimension(3);
	EXPECT_TRUE(mesh3d.isValid());

	const double zeroValue = 0.0;
	Field zero = zinc.fm.createFieldConstant(1, &zeroValue);
	EXPECT_TRUE(zero.isValid());
	Field zOffset = zinc.fm.createFieldConstant(1, &zeroValue);
	EXPECT_TRUE(zOffset.isValid());
	Field offsetComponentFields[] = { zero, zero, zOffset };
	Field offset = zinc.fm.createFieldConcatenate(3, offsetComponentFields);
	EXPECT_TRUE(offset.isValid());

	Field newCoordinates = referenceCoordinates + offset;
	EXPECT_TRUE(newCoordinates.isValid());
	EXPECT_EQ(RESULT_OK, newCoordinates.setName("newCoordinates"));

	const double oneValue = 1.0;
	Field one = zinc.fm.createFieldConstant(1, &oneValue);
	EXPECT_TRUE(one.isValid());
	FieldMeshIntegral volume = zinc.fm.createFieldMeshIntegral(one, coordinates, mesh3d);
	const int numberOfGaussPoints = 1;
	EXPECT_EQ(RESULT_OK, volume.setNumbersOfPoints(1, &numberOfGaussPoints));
	const double twoValue = 2.0;
	Field two = zinc.fm.createFieldConstant(1, &twoValue);
	EXPECT_TRUE(two.isValid());
	Field diff = volume - two;
	Field objective = diff*diff;
	EXPECT_TRUE(objective.isValid());
	EXPECT_EQ(RESULT_OK, objective.setName("objective"));

	// make the set of nodes we wish to offset
	FieldNodeGroup nodeGroupField = zinc.fm.createFieldNodeGroup(nodes);
	EXPECT_TRUE(nodeGroupField.isValid());
	NodesetGroup nodesetGroup = nodeGroupField.getNodesetGroup();
	EXPECT_TRUE(nodesetGroup.isValid());
	const int optimiseNodeIdentifiers[] = { 6, 8 };
	const int iCount = sizeof(optimiseNodeIdentifiers)/sizeof(int);
	for (int i = 0; i < iCount; ++i)
	{
		Node node = nodes.findNodeByIdentifier(optimiseNodeIdentifiers[i]);
		EXPECT_TRUE(node.isValid());
		EXPECT_EQ(RESULT_OK, nodesetGroup.addNode(node));
	}
	EXPECT_EQ(iCount, nodesetGroup.getSize());

	Fieldassignment fieldassignment = coordinates.createFieldassignment(newCoordinates);
	EXPECT_EQ(RESULT_OK, fieldassignment.setNodeset(nodesetGroup));
	EXPECT_TRUE(fieldassignment.isValid());

	Fieldcache cache = zinc.fm.createFieldcache();
	EXPECT_TRUE(cache.isValid());

	double volumeValueOut;
	const double tolerance = 1.0E-6;
	EXPECT_EQ(RESULT_OK, volume.evaluateReal(cache, 1, &volumeValueOut));
	EXPECT_NEAR(1.0, volumeValueOut, tolerance);

	Optimisation optimisation = zinc.fm.createOptimisation();
	EXPECT_TRUE(optimisation.isValid());
	EXPECT_EQ(RESULT_OK, result = optimisation.setMethod(Optimisation::METHOD_QUASI_NEWTON));
	EXPECT_EQ(RESULT_OK, result = optimisation.addObjectiveField(objective));
	EXPECT_EQ(RESULT_OK, result = optimisation.addIndependentField(zOffset));
	EXPECT_EQ(RESULT_OK, result = optimisation.addFieldassignment(fieldassignment));
	EXPECT_EQ(RESULT_OK, result = optimisation.setAttributeInteger(Optimisation::ATTRIBUTE_MAXIMUM_ITERATIONS, 10));

	EXPECT_EQ(RESULT_OK, result = optimisation.optimise());
	char *solutionReport = optimisation.getSolutionReport();
	EXPECT_NE((char *)0, solutionReport);
	printf("%s\n", solutionReport);

	EXPECT_EQ(RESULT_OK, volume.evaluateReal(cache, 1, &volumeValueOut));
	EXPECT_NEAR(2.0, volumeValueOut, tolerance);
	double zOffsetOut;
	EXPECT_EQ(RESULT_OK, zOffset.evaluateReal(cache, 1, &zOffsetOut));
	EXPECT_NEAR(2.0, zOffsetOut, tolerance);

	double x[3];
	const double expectedX[8][3] =
	{
		{ 0.0, 0.0, 0.0 },
		{ 1.0, 0.0, 0.0 },
		{ 0.0, 1.0, 0.0 },
		{ 1.0, 1.0, 0.0 },
		{ 0.0, 0.0, 1.0 },
		{ 1.0, 0.0, 3.0 },
		{ 0.0, 1.0, 1.0 },
		{ 1.0, 1.0, 3.0 }
	};
	for (int n = 0; n < 8; ++n)
	{
		Node node = nodes.findNodeByIdentifier(n + 1);
		EXPECT_TRUE(node.isValid());
		EXPECT_EQ(RESULT_OK, cache.setNode(node));
		EXPECT_EQ(RESULT_OK, coordinates.evaluateReal(cache, 3, x));
		for (int c = 0; c < 3; ++c)
		{
			EXPECT_NEAR(expectedX[n][c], x[c], tolerance);
		}
	}
	cmzn_deallocate(solutionReport);
}

// Optimise z displacement via a ramp in x of nodes on top of two hermite cubes
// to double its volume using optimisation with partial field assignment
// A prior 'reset' field assignment is used to allow derivative transformation
TEST(ZincOptimisation, addFieldassignmentReset)
{
	ZincTestSetupCpp zinc;
	int result;

	// read twice to get copy of coordinates in 'reference_coordinates'
	EXPECT_EQ(OK, result = zinc.root_region.readFile(
		TestResources::getLocation(TestResources::FIELDMODULE_EX2_TWO_CUBES_HERMITE_NOCROSS_RESOURCE)));
	Field referenceCoordinates = zinc.fm.findFieldByName("coordinates");
	EXPECT_TRUE(referenceCoordinates.isValid());
	EXPECT_EQ(OK, referenceCoordinates.setName("referenceCoordinates"));
	EXPECT_EQ(OK, result = zinc.root_region.readFile(
		TestResources::getLocation(TestResources::FIELDMODULE_EX2_TWO_CUBES_HERMITE_NOCROSS_RESOURCE)));
	Field coordinates = zinc.fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinates.isValid());

	Nodeset nodes = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_TRUE(nodes.isValid());
	Mesh mesh3d = zinc.fm.findMeshByDimension(3);
	EXPECT_TRUE(mesh3d.isValid());

	Field x = zinc.fm.createFieldComponent(coordinates, 1);
	EXPECT_TRUE(x.isValid());
	Field y = zinc.fm.createFieldComponent(coordinates, 2);
	EXPECT_TRUE(y.isValid());
	Field z = zinc.fm.createFieldComponent(coordinates, 3);
	EXPECT_TRUE(z.isValid());
	const double zeroValue = 0.0;
	Field s = zinc.fm.createFieldConstant(1, &zeroValue);
	EXPECT_TRUE(s.isValid());
	auto nz = z + s*x;
	EXPECT_TRUE(nz.isValid());
	const Field components[] = { x, y, nz };
	Field newCoordinates = zinc.fm.createFieldConcatenate(3, components);
	EXPECT_TRUE(newCoordinates.isValid());
	EXPECT_EQ(RESULT_OK, newCoordinates.setName("newCoordinates"));

	const double oneValue = 1.0;
	Field one = zinc.fm.createFieldConstant(1, &oneValue);
	EXPECT_TRUE(one.isValid());
	FieldMeshIntegral volume = zinc.fm.createFieldMeshIntegral(one, coordinates, mesh3d);
	const int numberOfGaussPoints = 3;
	EXPECT_EQ(RESULT_OK, volume.setNumbersOfPoints(1, &numberOfGaussPoints));
	const double fourValue = 4.0;
	Field four = zinc.fm.createFieldConstant(1, &fourValue);
	EXPECT_TRUE(four.isValid());
	Field diff = volume - four;
	Field objective = diff*diff;
	EXPECT_TRUE(objective.isValid());
	EXPECT_EQ(RESULT_OK, objective.setName("objective"));

	// make the set of nodes we wish to offset
	FieldNodeGroup nodeGroupField = zinc.fm.createFieldNodeGroup(nodes);
	EXPECT_TRUE(nodeGroupField.isValid());
	NodesetGroup nodesetGroup = nodeGroupField.getNodesetGroup();
	EXPECT_TRUE(nodesetGroup.isValid());
	const int optimiseNodeIdentifiers[] = { 7, 8, 9, 10, 11, 12 };
	const int iCount = sizeof(optimiseNodeIdentifiers)/sizeof(int);
	for (int i = 0; i < iCount; ++i)
	{
		Node node = nodes.findNodeByIdentifier(optimiseNodeIdentifiers[i]);
		EXPECT_TRUE(node.isValid());
		EXPECT_EQ(RESULT_OK, nodesetGroup.addNode(node));
	}
	EXPECT_EQ(iCount, nodesetGroup.getSize());

	// to avoid coordinates DOFs from drifting away, must reset with each evaluation
	// first assignment resets coordinates to equal the original reference coordinates
	Fieldassignment fieldassignmentReset = coordinates.createFieldassignment(referenceCoordinates);
	EXPECT_EQ(RESULT_OK, fieldassignmentReset.setNodeset(nodesetGroup));
	EXPECT_TRUE(fieldassignmentReset.isValid());
	// second assignment transforms a subset of nodes' dofs to affect the objective
	Fieldassignment fieldassignmentTransform = coordinates.createFieldassignment(newCoordinates);
	EXPECT_EQ(RESULT_OK, fieldassignmentTransform.setNodeset(nodesetGroup));
	EXPECT_TRUE(fieldassignmentTransform.isValid());

	Fieldcache cache = zinc.fm.createFieldcache();
	EXPECT_TRUE(cache.isValid());

	double volumeValueOut;
	const double tolerance = 1.0E-6;
	EXPECT_EQ(RESULT_OK, volume.evaluateReal(cache, 1, &volumeValueOut));
	EXPECT_NEAR(2.0, volumeValueOut, tolerance);

	Optimisation optimisation = zinc.fm.createOptimisation();
	EXPECT_TRUE(optimisation.isValid());
	EXPECT_EQ(RESULT_OK, result = optimisation.setMethod(Optimisation::METHOD_QUASI_NEWTON));
	EXPECT_EQ(RESULT_OK, result = optimisation.addObjectiveField(objective));
	EXPECT_EQ(RESULT_OK, result = optimisation.addIndependentField(s));
	EXPECT_EQ(RESULT_OK, result = optimisation.addFieldassignment(fieldassignmentReset));
	EXPECT_EQ(RESULT_OK, result = optimisation.addFieldassignment(fieldassignmentTransform));
	EXPECT_EQ(RESULT_OK, result = optimisation.setAttributeInteger(Optimisation::ATTRIBUTE_MAXIMUM_ITERATIONS, 10));

	EXPECT_EQ(RESULT_OK, result = optimisation.optimise());
	char *solutionReport = optimisation.getSolutionReport();
	EXPECT_NE((char *)0, solutionReport);
	printf("%s\n", solutionReport);

	EXPECT_EQ(RESULT_OK, volume.evaluateReal(cache, 1, &volumeValueOut));
	EXPECT_NEAR(4.0, volumeValueOut, tolerance);
	double sValueOut;
	EXPECT_EQ(RESULT_OK, s.evaluateReal(cache, 1, &sValueOut));
	EXPECT_NEAR(1.0, sValueOut, tolerance);
	cmzn_deallocate(solutionReport);
}

// Use NEWTON method to solve least squares fit of square bilinear element field to 4 points project at known locations onto it
TEST(ZincOptimisation, leastSquaresFitNewton)
{
	ZincTestSetupCpp zinc;

	// a handy model with nodes 1-4 in the corners of a square and nodes 5-8 with host locations
	EXPECT_EQ(RESULT_OK, zinc.root_region.readFile(TestResources::getLocation(TestResources::FIELDMODULE_EMBEDDING_ISSUE3614_RESOURCE)));

	Field coordinates = zinc.fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinates.isValid());
	Field dataCoordinates = zinc.fm.findFieldByName("data_coordinates");
	EXPECT_TRUE(dataCoordinates.isValid());
	FieldStoredMeshLocation hostLocation = zinc.fm.findFieldByName("host_location").castStoredMeshLocation();
	EXPECT_TRUE(hostLocation.isValid());
	FieldEmbedded hostCoordinates = zinc.fm.createFieldEmbedded(coordinates, hostLocation);
	EXPECT_TRUE(hostCoordinates.isValid());
	FieldSubtract delta = hostCoordinates - dataCoordinates;
	EXPECT_TRUE(delta.isValid());
	FieldDotProduct errorSquared = zinc.fm.createFieldDotProduct(delta, delta);
	EXPECT_TRUE(errorSquared.isValid());

	Nodeset nodeset = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_TRUE(nodeset.isValid());
	FieldNodesetSum sumErrorSquared = zinc.fm.createFieldNodesetSum(errorSquared, nodeset);
	EXPECT_TRUE(sumErrorSquared.isValid());
	EXPECT_EQ(RESULT_OK, sumErrorSquared.setElementMapField(hostLocation));

	Fieldcache fieldcache = zinc.fm.createFieldcache();
	EXPECT_TRUE(fieldcache.isValid());
	double outSum;
	const double TOL = 1.0E-11;
	EXPECT_EQ(RESULT_OK, sumErrorSquared.evaluateReal(fieldcache, 1, &outSum));
	EXPECT_NEAR(0.5563, outSum, TOL);

	// solve optimisation
	Optimisation optimisation = zinc.fm.createOptimisation();
	EXPECT_TRUE(optimisation.isValid());
	EXPECT_EQ(OK, optimisation.setMethod(Optimisation::METHOD_NEWTON));
	EXPECT_EQ(OK, optimisation.addObjectiveField(sumErrorSquared));
	EXPECT_EQ(OK, optimisation.addIndependentField(coordinates));
	EXPECT_EQ(OK, optimisation.setAttributeInteger(Optimisation::ATTRIBUTE_MAXIMUM_ITERATIONS, 1));

	EXPECT_EQ(OK, optimisation.optimise());
	char *solutionReport = optimisation.getSolutionReport();
	EXPECT_NE((char *)0, solutionReport);
	printf("%s", solutionReport);

	EXPECT_EQ(RESULT_OK, sumErrorSquared.evaluateReal(fieldcache, 1, &outSum));
	EXPECT_NEAR(0.0, outSum, TOL);
}
