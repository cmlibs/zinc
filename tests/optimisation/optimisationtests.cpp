/*
 * Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include "zinctestsetup.hpp"
#include <cmlibs/zinc/core.h>
#include <cmlibs/zinc/field.h>
#include <cmlibs/zinc/fieldconstant.h>
#include <cmlibs/zinc/optimisation.h>

#include "zinctestsetupcpp.hpp"
#include <cmlibs/zinc/field.hpp>
#include <cmlibs/zinc/fieldarithmeticoperators.hpp>
#include <cmlibs/zinc/fieldassignment.hpp>
#include <cmlibs/zinc/fieldcache.hpp>
#include <cmlibs/zinc/fieldcomposite.hpp>
#include <cmlibs/zinc/fieldconstant.hpp>
#include <cmlibs/zinc/fieldderivatives.hpp>
#include <cmlibs/zinc/fieldfiniteelement.hpp>
#include <cmlibs/zinc/fieldgroup.hpp>
#include <cmlibs/zinc/fieldlogicaloperators.hpp>
#include <cmlibs/zinc/fieldmatrixoperators.hpp>
#include <cmlibs/zinc/fieldmeshoperators.hpp>
#include <cmlibs/zinc/fieldnodesetoperators.hpp>
#include <cmlibs/zinc/fieldvectoroperators.hpp>
#include <cmlibs/zinc/optimisation.hpp>

#include "test_resources.h"
#include <cmath>

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

    EXPECT_DOUBLE_EQ(0.0, cmzn_optimisation_get_attribute_real(optimisation, CMZN_OPTIMISATION_ATTRIBUTE_FIELD_PARAMETERS_TIME));
    EXPECT_EQ(OK, result = cmzn_optimisation_set_attribute_real(optimisation, CMZN_OPTIMISATION_ATTRIBUTE_FIELD_PARAMETERS_TIME, 1.234));
    EXPECT_DOUBLE_EQ(1.234, cmzn_optimisation_get_attribute_real(optimisation, CMZN_OPTIMISATION_ATTRIBUTE_FIELD_PARAMETERS_TIME));

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
    EXPECT_EQ(OK, result = optimisation.setMethod(Optimisation::METHOD_NEWTON));
    EXPECT_EQ(Optimisation::METHOD_NEWTON, optimisation.getMethod());

    EXPECT_DOUBLE_EQ(0.0, optimisation.getAttributeReal(Optimisation::ATTRIBUTE_FIELD_PARAMETERS_TIME));
    EXPECT_EQ(OK, result = optimisation.setAttributeReal(Optimisation::ATTRIBUTE_FIELD_PARAMETERS_TIME, 1.234));
    EXPECT_DOUBLE_EQ(1.234, optimisation.getAttributeReal(Optimisation::ATTRIBUTE_FIELD_PARAMETERS_TIME));

    // made-up fields to test objective/dependent field APIs
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
    EXPECT_EQ(OK, result = optimisation.addDependentField(f1));
    EXPECT_EQ(OK, result = optimisation.addDependentField(f2));
    EXPECT_EQ(OK, result = optimisation.addDependentField(f3));
    EXPECT_EQ(ERROR_ARGUMENT, result = optimisation.addDependentField(f1));
    temp = optimisation.getFirstDependentField();
    EXPECT_EQ(f1, temp);
    temp = optimisation.getNextDependentField(temp);
    EXPECT_EQ(f2, temp);
    temp = optimisation.getNextDependentField(temp);
    EXPECT_EQ(f3, temp);
    temp = optimisation.getNextDependentField(temp);
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

    EXPECT_EQ(ERROR_ARGUMENT, result = optimisation.removeDependentField(f4)); // not in use
    EXPECT_EQ(OK, result = optimisation.removeDependentField(f1));
    temp = optimisation.getFirstDependentField();
    EXPECT_EQ(f2, temp);
    temp = optimisation.getNextDependentField(temp);
    EXPECT_EQ(f3, temp);
    temp = optimisation.getNextDependentField(temp);
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
        resourcePath("optimisation/tricubic.exfile").c_str()));
    Field referenceCoordinates = zinc.fm.findFieldByName("coordinates");
    EXPECT_TRUE(referenceCoordinates.isValid());
    EXPECT_EQ(OK, referenceCoordinates.setName("reference_coordinates"));
    EXPECT_EQ(OK, result = zinc.root_region.readFile(
        resourcePath("optimisation/tricubic.exfile").c_str()));
    Field coordinates = zinc.fm.findFieldByName("coordinates");
    EXPECT_TRUE(coordinates.isValid());

    zinc.fm.beginChange();
    Nodeset nodeset = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
    EXPECT_TRUE(nodeset.isValid());

    const int end2NodeIdentifiers[] = { 4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,64 };
    const int end2NodeIdentifiersSize = sizeof(end2NodeIdentifiers)/sizeof(int);
    FieldGroup end2Group = zinc.fm.createFieldGroup();
    NodesetGroup end2Nodeset = end2Group.createNodesetGroup(nodeset);
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
    EXPECT_EQ(OK, result = optimisation.addDependentField(coordinates));
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
        resourcePath("fieldmodule/cube.exformat").c_str()));
    Field referenceCoordinates = zinc.fm.findFieldByName("coordinates");
    EXPECT_TRUE(referenceCoordinates.isValid());
    EXPECT_EQ(OK, referenceCoordinates.setName("reference_coordinates"));
    EXPECT_EQ(OK, result = zinc.root_region.readFile(
        resourcePath("fieldmodule/cube.exformat").c_str()));
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
    FieldGroup group = zinc.fm.createFieldGroup();
    EXPECT_TRUE(group.isValid());
    NodesetGroup nodesetGroup = group.createNodesetGroup(nodes);
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
    EXPECT_EQ(RESULT_OK, result = optimisation.addDependentField(zOffset));
    EXPECT_EQ(RESULT_OK, result = optimisation.addFieldassignment(fieldassignment));
    EXPECT_EQ(RESULT_OK, result = optimisation.setAttributeInteger(Optimisation::ATTRIBUTE_MAXIMUM_ITERATIONS, 10));

    EXPECT_EQ(RESULT_OK, result = optimisation.optimise());
    char *solutionReport = optimisation.getSolutionReport();
    EXPECT_NE((char *)0, solutionReport);
    printf("%s\n", solutionReport);
    cmzn_deallocate(solutionReport);

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
        resourcePath("fieldmodule/two_cubes_hermite_nocross.ex2").c_str()));
    Field referenceCoordinates = zinc.fm.findFieldByName("coordinates");
    EXPECT_TRUE(referenceCoordinates.isValid());
    EXPECT_EQ(OK, referenceCoordinates.setName("referenceCoordinates"));
    EXPECT_EQ(OK, result = zinc.root_region.readFile(
        resourcePath("fieldmodule/two_cubes_hermite_nocross.ex2").c_str()));
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
    FieldGroup group = zinc.fm.createFieldGroup();
    EXPECT_TRUE(group.isValid());
    NodesetGroup nodesetGroup = group.createNodesetGroup(nodes);
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
    EXPECT_EQ(RESULT_OK, result = optimisation.addDependentField(s));
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
    EXPECT_EQ(RESULT_OK, zinc.root_region.readFile(resourcePath("fieldmodule/embedding_issue3614.exregion").c_str()));

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
    EXPECT_EQ(OK, optimisation.addDependentField(coordinates));
    EXPECT_EQ(OK, optimisation.setAttributeInteger(Optimisation::ATTRIBUTE_MAXIMUM_ITERATIONS, 1));

    EXPECT_EQ(OK, optimisation.optimise());
    char *solutionReport = optimisation.getSolutionReport();
    EXPECT_NE((char *)0, solutionReport);
    printf("%s", solutionReport);

    EXPECT_EQ(RESULT_OK, sumErrorSquared.evaluateReal(fieldcache, 1, &outSum));
    EXPECT_NEAR(0.0, outSum, TOL);
    cmzn_deallocate(solutionReport);
}

// Use NEWTON method to solve least squares fit of two hermite cubes without cross derivatives
// to data points in an ellipsoid shape.
TEST(ZincOptimisation, leastSquaresFitNewtonSmooth)
{
    ZincTestSetupCpp zinc;

    // a handy model with nodes 1-4 in the corners of a square and nodes 5-8 with host locations
    std::string filename = resourcePath("fieldmodule/two_cubes_hermite_nocross.ex2");
    EXPECT_EQ(RESULT_OK, zinc.root_region.readFile(filename.c_str()));

    Field referenceCoordinates = zinc.fm.findFieldByName("coordinates");
    EXPECT_TRUE(referenceCoordinates.isValid());
    EXPECT_EQ(OK, referenceCoordinates.setName("reference_coordinates"));
    EXPECT_EQ(OK, zinc.root_region.readFile(filename.c_str()));
    Field coordinates = zinc.fm.findFieldByName("coordinates");
    EXPECT_TRUE(coordinates.isValid());

    Mesh mesh3d = zinc.fm.findMeshByDimension(3);
    EXPECT_TRUE(mesh3d.isValid());
    const int elementCount = mesh3d.getSize();
    EXPECT_EQ(elementCount, 2);
    Mesh mesh2d = zinc.fm.findMeshByDimension(2);
    EXPECT_TRUE(mesh2d.isValid());

    zinc.fm.beginChange();

    // generate regular data points over ellipse
    FieldFiniteElement dataCoordinates = zinc.fm.createFieldFiniteElement(/*numberOfComponents*/3);
    EXPECT_TRUE(dataCoordinates.isValid());
    EXPECT_EQ(RESULT_OK, dataCoordinates.setName("data_coordinates"));
    EXPECT_EQ(RESULT_OK, dataCoordinates.setTypeCoordinate(true));
    EXPECT_EQ(RESULT_OK, dataCoordinates.setCoordinateSystemType(Field::COORDINATE_SYSTEM_TYPE_RECTANGULAR_CARTESIAN));
    EXPECT_EQ(RESULT_OK, dataCoordinates.setManaged(true));
    EXPECT_EQ(RESULT_OK, dataCoordinates.setComponentName(1, "x"));
    EXPECT_EQ(RESULT_OK, dataCoordinates.setComponentName(2, "y"));
    EXPECT_EQ(RESULT_OK, dataCoordinates.setComponentName(3, "z"));
    // want to store mesh locations at the data points
    FieldStoredMeshLocation storedMeshLocationVolume = zinc.fm.createFieldStoredMeshLocation(mesh3d);
    EXPECT_TRUE(storedMeshLocationVolume.isValid());
    EXPECT_EQ(RESULT_OK, storedMeshLocationVolume.setName("host_location"));
    Nodeset datapoints = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_DATAPOINTS);
    EXPECT_TRUE(datapoints.isValid());
    Nodetemplate nodetemplate = datapoints.createNodetemplate();
    EXPECT_TRUE(nodetemplate.isValid());
    EXPECT_EQ(RESULT_OK, nodetemplate.defineField(dataCoordinates));
    EXPECT_EQ(RESULT_OK, nodetemplate.defineField(storedMeshLocationVolume));
    const int pointsCountAlong = 16;
    const int pointsCountAround = 16;
    const double xRadius = 1.2;
    const double yRadius = 0.9;
    const double zRadius = 0.6;
    const double xCentre = 1.0;
    const double yCentre = 0.5;
    const double zCentre = 0.5;
    const double pi = 3.1415926535897932384626433832795;
    const double radiansPerPointAlong = pi/static_cast<double>(pointsCountAlong);
    const double radiansPerPointAround = 2.0*pi/static_cast<double>(pointsCountAround);
    Fieldcache fieldcache = zinc.fm.createFieldcache();
    double x[3];
    for (int i = 0; i < pointsCountAlong; ++i)
    {
        const double radiansAlong = (i + 0.5)*radiansPerPointAlong;
        x[0] = xCentre + xRadius*cos(radiansAlong);
        const double ySize = yRadius*sin(radiansAlong);
        const double zSize = zRadius*sin(radiansAlong);
        for (int j = 0; j < pointsCountAround; ++j)
        {
            const double radiansAround = (j + 0.5)*radiansPerPointAround;
            x[1] = yCentre + ySize*cos(radiansAround);
            x[2] = zCentre + zSize*sin(radiansAround);
            Node node = datapoints.createNode(-1, nodetemplate);
            EXPECT_EQ(RESULT_OK, fieldcache.setNode(node));
            EXPECT_EQ(RESULT_OK, dataCoordinates.assignReal(fieldcache, 3, x));
        }
    }

    FieldGroup outside = zinc.fm.createFieldGroup();
    EXPECT_TRUE(outside.isValid());
    EXPECT_EQ(RESULT_OK, outside.setName("outside"));
    MeshGroup outsideMeshGroup = outside.createMeshGroup(mesh2d);
    EXPECT_TRUE(outsideMeshGroup.isValid());
    outsideMeshGroup.addElementsConditional(zinc.fm.createFieldIsExterior());
    EXPECT_EQ(10, outsideMeshGroup.getSize());

    // need to find locations on the 2D outside mesh group but store locations on mesh3d
    FieldFindMeshLocation findMeshLocationOutside = zinc.fm.createFieldFindMeshLocation(dataCoordinates, coordinates, mesh3d);
    EXPECT_TRUE(findMeshLocationOutside.isValid());
    EXPECT_EQ(RESULT_OK, findMeshLocationOutside.setSearchMesh(outsideMeshGroup));
    EXPECT_EQ(RESULT_OK, findMeshLocationOutside.setSearchMode(FieldFindMeshLocation::SEARCH_MODE_NEAREST));
    Fieldassignment fieldassignment = storedMeshLocationVolume.createFieldassignment(findMeshLocationOutside);
    EXPECT_EQ(RESULT_OK, fieldassignment.setNodeset(datapoints));
    EXPECT_EQ(RESULT_OK, fieldassignment.assign());
    fieldassignment = Fieldassignment();

    FieldEmbedded projectedCoordinates = zinc.fm.createFieldEmbedded(coordinates, storedMeshLocationVolume);
    EXPECT_TRUE(projectedCoordinates.isValid());
    FieldSubtract delta = projectedCoordinates - dataCoordinates;
    EXPECT_TRUE(delta.isValid());
    FieldDotProduct delta_sq = zinc.fm.createFieldDotProduct(delta, delta);
    EXPECT_TRUE(delta_sq.isValid());

    FieldNodesetSum outsideSurfaceFitObjective = zinc.fm.createFieldNodesetSum(delta_sq, datapoints);
    EXPECT_TRUE(outsideSurfaceFitObjective.isValid());
    EXPECT_EQ(RESULT_OK, outsideSurfaceFitObjective.setElementMapField(storedMeshLocationVolume));

    // test getting element stiffness matrix for displacement gradient
    FieldGradient dx_dX = zinc.fm.createFieldGradient(coordinates, referenceCoordinates);
    EXPECT_TRUE(dx_dX.isValid());
    FieldMultiply square_dx_dX = dx_dX * dx_dX;
    EXPECT_TRUE(square_dx_dX.isValid());
    const double alpha9[9] = { 0.001, 0.001, 0.001, 0.001, 0.001, 0.001, 0.001, 0.001, 0.001 };
    FieldConstant alpha = zinc.fm.createFieldConstant(9, alpha9);
    EXPECT_TRUE(alpha.isValid());
    FieldDotProduct sumScaledSquare_dx_dX = zinc.fm.createFieldDotProduct(alpha, square_dx_dX);
    EXPECT_TRUE(sumScaledSquare_dx_dX.isValid());

    FieldGradient d2x_dX2 = zinc.fm.createFieldGradient(dx_dX, referenceCoordinates);
    EXPECT_TRUE(d2x_dX2.isValid());
    FieldMultiply square_d2x_dX2 = d2x_dX2 * d2x_dX2;
    EXPECT_TRUE(square_d2x_dX2.isValid());
    const double beta27[27] = {
        0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01,
        0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01 ,
        0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01, 0.01 };
    FieldConstant beta = zinc.fm.createFieldConstant(27, beta27);
    EXPECT_TRUE(beta.isValid());
    FieldDotProduct sumScaledSquare_d2x_dX2 = zinc.fm.createFieldDotProduct(beta, square_d2x_dX2);
    EXPECT_TRUE(sumScaledSquare_d2x_dX2.isValid());

    FieldMeshIntegral smoothingObjective = zinc.fm.createFieldMeshIntegral(sumScaledSquare_dx_dX + sumScaledSquare_d2x_dX2, referenceCoordinates, mesh3d);
    EXPECT_TRUE(smoothingObjective.isValid());
    const int pointCount = 3;
    EXPECT_EQ(RESULT_OK, smoothingObjective.setNumbersOfPoints(1, &pointCount));

    zinc.fm.endChange();

    // solve optimisation
    Optimisation optimisation = zinc.fm.createOptimisation();
    EXPECT_TRUE(optimisation.isValid());
    EXPECT_EQ(OK, optimisation.setMethod(Optimisation::METHOD_NEWTON));
    EXPECT_EQ(OK, optimisation.addObjectiveField(outsideSurfaceFitObjective));
    EXPECT_EQ(OK, optimisation.addObjectiveField(smoothingObjective));
    EXPECT_EQ(OK, optimisation.addDependentField(coordinates));
    EXPECT_EQ(OK, optimisation.setAttributeInteger(Optimisation::ATTRIBUTE_MAXIMUM_ITERATIONS, 1));

    EXPECT_EQ(OK, optimisation.optimise());
    char *solutionReport = optimisation.getSolutionReport();
    EXPECT_NE((char *)0, solutionReport);
    printf("%s", solutionReport);
    cmzn_deallocate(solutionReport);

    const double TOL = 1.0E-8;
    double outSurfaceFitObjectiveValue;
    EXPECT_EQ(RESULT_OK, outsideSurfaceFitObjective.evaluateReal(fieldcache, 1, &outSurfaceFitObjectiveValue));
    EXPECT_NEAR(0.57100132382309376, outSurfaceFitObjectiveValue, TOL);

    double outSmoothingObjectiveValue;
    EXPECT_EQ(RESULT_OK, smoothingObjective.evaluateReal(fieldcache, 1, &outSmoothingObjectiveValue));
    EXPECT_NEAR(0.31669258057011818, outSmoothingObjectiveValue, TOL);
}

// Use NEWTON method to solve least squares fit of best fit line for field with two times
TEST(ZincOptimisation, fitLineTime)
{
    ZincTestSetupCpp zinc;

    EXPECT_EQ(RESULT_OK, zinc.root_region.readFile(resourcePath("optimisation/fit_line_time.exf").c_str()));

    FieldFiniteElement coordinates = zinc.fm.findFieldByName("coordinates").castFiniteElement();
    EXPECT_TRUE(coordinates.isValid());
    FieldFiniteElement dataCoordinates = zinc.fm.findFieldByName("data_coordinates").castFiniteElement();
    EXPECT_TRUE(dataCoordinates.isValid());
    FieldStoredMeshLocation hostLocation = zinc.fm.findFieldByName("host_location").castStoredMeshLocation();
    EXPECT_TRUE(hostLocation.isValid());

    Nodeset nodes = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
    EXPECT_EQ(2, nodes.getSize());
    Mesh mesh1d = zinc.fm.findMeshByDimension(1);
    EXPECT_EQ(1, mesh1d.getSize());
    Nodeset datapoints = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_DATAPOINTS);
    EXPECT_EQ(2, datapoints.getSize());

    FieldEmbedded hostCoordinates = zinc.fm.createFieldEmbedded(coordinates, hostLocation);
    EXPECT_TRUE(hostCoordinates.isValid());
    FieldSubtract delta = hostCoordinates - dataCoordinates;
    EXPECT_TRUE(delta.isValid());
    FieldDotProduct delta_sq = zinc.fm.createFieldDotProduct(delta, delta);
    EXPECT_TRUE(delta_sq.isValid());

    const double times[2] = { 0.0, 1.0 };

    FieldNodesetSum dataFitObjective = zinc.fm.createFieldNodesetSum(delta_sq, datapoints);
    EXPECT_TRUE(dataFitObjective.isValid());
    EXPECT_EQ(RESULT_OK, dataFitObjective.setElementMapField(hostLocation));

    Optimisation optimisation = zinc.fm.createOptimisation();
    EXPECT_TRUE(optimisation.isValid());
    EXPECT_EQ(RESULT_OK, optimisation.setMethod(Optimisation::METHOD_NEWTON));
    EXPECT_EQ(RESULT_OK, optimisation.addObjectiveField(dataFitObjective));
    EXPECT_EQ(RESULT_OK, optimisation.addDependentField(coordinates));
    EXPECT_EQ(RESULT_OK, optimisation.setAttributeInteger(Optimisation::ATTRIBUTE_MAXIMUM_ITERATIONS, 1));

    for (int timeIndex = 0; timeIndex < 2; ++timeIndex)
    {
        double time = times[timeIndex];
        EXPECT_EQ(RESULT_OK, optimisation.setAttributeReal(Optimisation::ATTRIBUTE_FIELD_PARAMETERS_TIME, time));
        EXPECT_EQ(RESULT_OK, optimisation.optimise());
    }
    Fieldcache fieldcache = zinc.fm.createFieldcache();
    const double expectedTimeNodeCoordinates[2][2][3] =
    {
        { { -0.5, -0.5, -0.5 }, {  1.5,  1.5,  1.5 } },
        { {  0.0,  1.0,  1.0 }, {  1.0,  0.0,  0.0 } }
    };
    double x[3];
    const double TOL = 1.0E-10;
    for (int timeIndex = 0; timeIndex < 2; ++timeIndex)
    {
        double time = times[timeIndex];
        EXPECT_EQ(RESULT_OK, fieldcache.setTime(time));
        for (int nodeIndex = 0; nodeIndex < 2; ++nodeIndex)
        {
            Node node = nodes.findNodeByIdentifier(nodeIndex + 1);
            EXPECT_EQ(RESULT_OK, fieldcache.setNode(node));
            EXPECT_EQ(RESULT_OK, coordinates.getNodeParameters(fieldcache, -1, Node::VALUE_LABEL_VALUE, 1, 3, x));
            for (int c = 0; c < 3; ++c)
            {
                EXPECT_NEAR(expectedTimeNodeCoordinates[timeIndex][nodeIndex][c], x[c], TOL);
            }
        }
    }
    // check optimise fails if parameters are not held the time set
    EXPECT_EQ(RESULT_OK, optimisation.setAttributeReal(Optimisation::ATTRIBUTE_FIELD_PARAMETERS_TIME, 0.5));
    EXPECT_EQ(RESULT_ERROR_GENERAL, optimisation.optimise());
}

// Use NEWTON method for an optimisation problem with a conditional field to limit included DOFs
// also test constraining coordinates on a face to x=0
TEST(ZincOptimisation, NewtonConditionalAndFaceIntegral)
{
    ZincTestSetupCpp zinc;

    EXPECT_EQ(RESULT_OK, zinc.root_region.readFile(resourcePath("fieldmodule/two_cubes.exformat").c_str()));
    FieldFiniteElement referenceCoordinates = zinc.fm.findFieldByName("coordinates").castFiniteElement();
    EXPECT_EQ(RESULT_OK, referenceCoordinates.setName("reference coordinates"));
    EXPECT_EQ(RESULT_OK, zinc.root_region.readFile(resourcePath("fieldmodule/two_cubes.exformat").c_str()));
    FieldFiniteElement coordinates = zinc.fm.findFieldByName("coordinates").castFiniteElement();
    EXPECT_TRUE(coordinates.isValid());

    Mesh mesh3d = zinc.fm.findMeshByDimension(3);
    EXPECT_TRUE(mesh3d.isValid());
    Element element1 = mesh3d.findElementByIdentifier(1);
    EXPECT_TRUE(element1.isValid());
    Mesh mesh2d = zinc.fm.findMeshByDimension(2);
    EXPECT_TRUE(mesh2d.isValid());
    Element face1 = mesh2d.findElementByIdentifier(1);
    EXPECT_TRUE(face1.isValid());
    Nodeset nodes = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
    EXPECT_TRUE(nodes.isValid());

    FieldGroup one = zinc.fm.createFieldGroup();
    EXPECT_TRUE(one.isValid());
    EXPECT_EQ(RESULT_OK, one.setName("one"));
    EXPECT_EQ(RESULT_OK, one.setSubelementHandlingMode(FieldGroup::SUBELEMENT_HANDLING_MODE_FULL));
    MeshGroup oneMesh3d = one.createMeshGroup(mesh3d);
    EXPECT_EQ(RESULT_OK, oneMesh3d.addElement(element1));

    FieldGroup left = zinc.fm.createFieldGroup();
    EXPECT_TRUE(left.isValid());
    EXPECT_EQ(RESULT_OK, left.setName("left"));
    MeshGroup leftFaceMeshGroup = left.createMeshGroup(mesh2d);
    EXPECT_EQ(RESULT_OK, leftFaceMeshGroup.addElement(face1));

    FieldComponent x = zinc.fm.createFieldComponent(coordinates, 1);
    const double scaleValue = 100.0;
    FieldConstant scale = zinc.fm.createFieldConstant(1, &scaleValue);
    FieldMultiply scalex = scale * x;
    FieldMultiply scalexSq = scalex * scalex;
    FieldMeshIntegral leftObjective = zinc.fm.createFieldMeshIntegral(scalexSq, referenceCoordinates, leftFaceMeshGroup);
    EXPECT_EQ(RESULT_OK, leftObjective.setName("Left objective"));
    const int numberOfPoints = 4;
    EXPECT_EQ(RESULT_OK, leftObjective.setNumbersOfPoints(1, &numberOfPoints));
    EXPECT_TRUE(leftObjective.isValid());

    Field displacement = coordinates - referenceCoordinates;
    FieldGradient strain = zinc.fm.createFieldGradient(displacement, referenceCoordinates);
    const double targetStrainValues[9] = { 0.2, 0.0, 0.0, 0.0, -0.1, 0.0, 0.0, 0.0, -0.2 };
    Field targetStrain = zinc.fm.createFieldConstant(9, targetStrainValues);
    Field deltaStrain = strain - targetStrain;
    Field stretchSq = zinc.fm.createFieldDotProduct(deltaStrain, deltaStrain);
    FieldMeshIntegral stretchObjective = zinc.fm.createFieldMeshIntegral(stretchSq, referenceCoordinates, mesh3d);
    EXPECT_EQ(RESULT_OK, stretchObjective.setName("Stretch objective"));
    EXPECT_EQ(RESULT_OK, stretchObjective.setNumbersOfPoints(1, &numberOfPoints));
    EXPECT_TRUE(stretchObjective.isValid());

    const int components[2] = { 2, 3 };
    Field yzDisplacement = zinc.fm.createFieldComponent(displacement, 2, components);
    Field displacementSq = zinc.fm.createFieldDotProduct(yzDisplacement, yzDisplacement);
    const double scaleDisplacementValue = 0.1;
    Field scaleDisplacementSq = displacementSq * zinc.fm.createFieldConstant(1, &scaleDisplacementValue);
    FieldMeshIntegral displacementObjective = zinc.fm.createFieldMeshIntegral(scaleDisplacementSq, referenceCoordinates, mesh3d);
    EXPECT_EQ(RESULT_OK, displacementObjective.setName("Displacement objective"));
    // leave on 1 point as only used to stop solution drifting from centre
    //EXPECT_EQ(RESULT_OK, displacementObjective.setNumbersOfPoints(1, &numberOfPoints));
    EXPECT_TRUE(displacementObjective.isValid());

    Fieldcache fieldcache = zinc.fm.createFieldcache();
    EXPECT_TRUE(fieldcache.isValid());

    // solve optimisation
    Optimisation optimisation = zinc.fm.createOptimisation();
    EXPECT_TRUE(optimisation.isValid());
    EXPECT_EQ(OK, optimisation.setMethod(Optimisation::METHOD_NEWTON));
    EXPECT_EQ(OK, optimisation.addObjectiveField(stretchObjective));
    EXPECT_EQ(OK, optimisation.addObjectiveField(displacementObjective));
    EXPECT_EQ(OK, optimisation.addObjectiveField(leftObjective));
    EXPECT_EQ(OK, optimisation.addDependentField(coordinates));
    EXPECT_EQ(OK, optimisation.setConditionalField(coordinates, one));
    EXPECT_EQ(OK, optimisation.setAttributeInteger(Optimisation::ATTRIBUTE_MAXIMUM_ITERATIONS, 1));

    const double expectedStrainValues2[4][9] =
    {
        { 0.201647355, 0.007547436829, 0.004796079083, 0.008173234501, -0.09089931891, 0.006020108833, 0.02452659972, 0.02452659972, -0.1831722107 },
        { 0.1960085938, 0.01916019571, -0.002543413266, 0.006234574972, -0.07980044164, 0.002806616804, 0.009875937356, 0.08205938939, -0.1963500754 },
        { 0.1992943576, -0.000961682262, 5.067218337e-05, 0.001652981781, -0.1043312657, 0.00125509515, 0.005853057598, -0.01394310219, -0.1969155053 },
        { 0, 0, -2.334789092e-07, 0, 0, 0.06312796163, 0, 0, -0.07897356122 }
    };
    const double TOL = 1.0E-10;
    int unchangedNodeIdentifiers[5] = { 3, 6, 9, 12, 1 };
    for (int fitCase = 1; fitCase <= 2; ++fitCase)
    {
        if (fitCase == 2)
        {
            // reset coordinates and exclude node 1 from solution
            // this tests element with some DOFs excluded from solution, which previously had a bug
            EXPECT_EQ(RESULT_OK, zinc.root_region.readFile(resourcePath("fieldmodule/two_cubes.exformat").c_str()));
            NodesetGroup oneNodes = one.getNodesetGroup(nodes);
            Node node1 = nodes.findNodeByIdentifier(1);
            EXPECT_EQ(RESULT_OK, oneNodes.removeNode(node1));
        }

        EXPECT_EQ(OK, optimisation.optimise());
        char* solutionReport = optimisation.getSolutionReport();
        EXPECT_NE((char*)0, solutionReport);
        cmzn_deallocate(solutionReport);

        // check final strains match target
        double outStrainValues[9];
        const double xi[4][3] =
        {
            { 0.5, 0.5, 0.5 },
            { 0.2, 0.7, 0.0 },
            { 0.6, 0.9, 0.9 },
            { 0.0, 0.0, 0.0 }
        };
        for (int p = 0; p < 4; ++p)
        {
            fieldcache.setMeshLocation(element1, 3, xi[p]);
            EXPECT_EQ(RESULT_OK, strain.evaluateReal(fieldcache, 9, outStrainValues));
            const double* expectedStrainValues = (fitCase == 1) ? targetStrainValues : expectedStrainValues2[p];
            for (int i = 0; i < 9; ++i)
            {
                EXPECT_NEAR(expectedStrainValues[i], outStrainValues[i], TOL);
            }
        }

        // check left constraint
        double outLeft;
        fieldcache.clearLocation();
        EXPECT_EQ(RESULT_OK, leftObjective.evaluateReal(fieldcache, 1, &outLeft));
        EXPECT_NEAR(0.0, outLeft, 5E-6);  // not exactly satisfied by fitCase 2

        // check no change to end nodes not part of conditional group
        const int nodeIdentifiersCount = (fitCase == 1) ? 4 : 5;
        const Node::ValueLabel valueLabels[4] = { Node::VALUE_LABEL_VALUE, Node::VALUE_LABEL_D_DS1, Node::VALUE_LABEL_D_DS2, Node::VALUE_LABEL_D2_DS1DS2 };
        for (int n = 0; n < nodeIdentifiersCount; ++n)
        {
            Node node = nodes.findNodeByIdentifier(unchangedNodeIdentifiers[n]);
            EXPECT_EQ(RESULT_OK, fieldcache.setNode(node));
            double ux[3];
            double dx[3];
            for (int v = 0; v < 4; ++v)
            {
                EXPECT_EQ(RESULT_OK, referenceCoordinates.getNodeParameters(fieldcache, -1, valueLabels[v], 1, 3, ux));
                EXPECT_EQ(RESULT_OK, coordinates.getNodeParameters(fieldcache, -1, valueLabels[v], 1, 3, dx));
                for (int c = 0; c < 3; ++c)
                {
                    EXPECT_DOUBLE_EQ(ux[c], dx[c]);
                }
            }
        }
    }
}
