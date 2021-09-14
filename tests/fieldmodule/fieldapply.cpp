/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include <opencmiss/zinc/changemanager.hpp>
#include <opencmiss/zinc/element.hpp>
#include <opencmiss/zinc/elementbasis.hpp>
#include <opencmiss/zinc/field.hpp>
#include <opencmiss/zinc/fieldalias.hpp>
#include <opencmiss/zinc/fieldapply.hpp>
#include <opencmiss/zinc/fieldarithmeticoperators.hpp>
#include <opencmiss/zinc/fieldassignment.hpp>
#include <opencmiss/zinc/fieldcomposite.hpp>
#include <opencmiss/zinc/fieldconstant.hpp>
#include <opencmiss/zinc/fieldfiniteelement.hpp>
#include <opencmiss/zinc/fieldtime.hpp>
#include <opencmiss/zinc/node.hpp>

#include "zinctestsetupcpp.hpp"

#include "test_resources.h"

// Test createFieldAlias now creates a FieldApply
TEST(ZincFieldApply, aliasMigration)
{
	ZincTestSetupCpp zinc;

	const double valuesIn[3] = { 1.0, 2.5, -3.0 };
	FieldConstant fieldConstant = zinc.fm.createFieldConstant(3, valuesIn);
	EXPECT_TRUE(fieldConstant.isValid());
	Field fieldAlias = zinc.fm.createFieldAlias(fieldConstant);
	EXPECT_TRUE(fieldAlias.isValid());
	// test cast to Apply field
	FieldApply fieldApply = fieldAlias.castApply();
	EXPECT_TRUE(fieldApply.isValid());
	EXPECT_EQ(0, fieldApply.getNumberOfBindings());

	// test evaluation of apply without arguments
	Fieldcache fieldcache = zinc.fm.createFieldcache();
	double valuesOut[3];
	EXPECT_EQ(RESULT_OK, fieldApply.evaluateReal(fieldcache, 3, valuesOut));
	for (int c = 0; c < 3; ++c)
		EXPECT_DOUBLE_EQ(valuesIn[c], valuesOut[c]);

	// test assignment - same region
	const double newValues[3] = { 1.2, 2.2, 3.2 };
	EXPECT_EQ(RESULT_OK, fieldApply.assignReal(fieldcache, 3, newValues));
	// re-create fieldcache to check really there
	fieldcache = zinc.fm.createFieldcache();
	EXPECT_EQ(RESULT_OK, fieldApply.evaluateReal(fieldcache, 3, valuesOut));
	for (int c = 0; c < 3; ++c)
		EXPECT_DOUBLE_EQ(newValues[c], valuesOut[c]);
}

// Test applying a field imported from another region which maps from
// coordinates to mesh location to another field's values.
TEST(ZincFieldApply, importEmbeddingMap)
{
	ZincTestSetupCpp zinc;

	EXPECT_EQ(CMZN_OK, zinc.root_region.readFile(TestResources::getLocation(TestResources::FIELDIO_EX2_CUBE_RESOURCE)));
	Mesh mesh3d = zinc.fm.findMeshByDimension(3);
	EXPECT_TRUE(mesh3d.isValid());
	Element element = mesh3d.findElementByIdentifier(1);
	EXPECT_TRUE(element.isValid());
	Field coordinates = zinc.fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinates.isValid());
	Field pressure = zinc.fm.findFieldByName("pressure");
	EXPECT_TRUE(pressure.isValid());

	FieldArgumentReal coordinatesArgument = zinc.fm.createFieldArgumentReal(3);
	EXPECT_TRUE(coordinatesArgument.isValid());
	EXPECT_TRUE(coordinatesArgument.castArgumentReal().isValid());

	FieldFindMeshLocation findMeshLocation = zinc.fm.createFieldFindMeshLocation(coordinatesArgument, coordinates, mesh3d);
	EXPECT_TRUE(findMeshLocation.isValid());
	FieldEmbedded embeddedCoordinates = zinc.fm.createFieldEmbedded(coordinates, findMeshLocation);
	EXPECT_TRUE(embeddedCoordinates.isValid());
	FieldEmbedded embeddedPressure = zinc.fm.createFieldEmbedded(pressure, findMeshLocation);
	EXPECT_TRUE(embeddedPressure.isValid());

	Region childRegion = zinc.root_region.createChild("child");
	Fieldmodule childFm = childRegion.getFieldmodule();
	EXPECT_TRUE(childFm.isValid());
	// create a 1D element with coordinates diagonally across the cube
	Mesh childMesh1d = childFm.findMeshByDimension(1);
	EXPECT_TRUE(childMesh1d.isValid());
	Fieldcache childFieldcache = childFm.createFieldcache();
	EXPECT_TRUE(childFieldcache.isValid());
	FieldFiniteElement childCoordinates;
	Element childElement;
	{
		ChangeManager<Fieldmodule> changeField(childFm);
		childCoordinates = childFm.createFieldFiniteElement(3);
		EXPECT_TRUE(childCoordinates.isValid());
		Nodeset nodes = childFm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
		Nodetemplate nodetemplate = nodes.createNodetemplate();
		EXPECT_EQ(RESULT_OK, nodetemplate.defineField(childCoordinates));
		const double times[2] = { 0.0, 1.0 };
		Timesequence timesequence = zinc.fm.getMatchingTimesequence(2, times);
		EXPECT_TRUE(timesequence.isValid());
		EXPECT_EQ(RESULT_OK, nodetemplate.setTimesequence(childCoordinates, timesequence));
		const double childCoordinatesIn[2][2][3] =
		{
			{ { 1.0, 0.0, 0.0 }, { 0.9, 0.2, 0.1 } },
			{ { 0.0, 0.6, 1.0 }, { 0.5, 0.6, 0.3 } }
		};
		for (int n = 0; n < 2; ++n)
		{
			Node node = nodes.createNode(n + 1, nodetemplate);
			EXPECT_EQ(RESULT_OK, childFieldcache.setNode(node));
			for (int t = 0; t < 2; ++t)
			{
				EXPECT_EQ(RESULT_OK, childFieldcache.setTime(times[t]));
				EXPECT_EQ(RESULT_OK, childCoordinates.assignReal(childFieldcache, 3, childCoordinatesIn[n][t]));
			}
		}
		Elementbasis elementbasis = childFm.createElementbasis(1, Elementbasis::FUNCTION_TYPE_LINEAR_LAGRANGE);
		Elementtemplate childElementtemplate = childMesh1d.createElementtemplate();
		EXPECT_EQ(RESULT_OK, childElementtemplate.setElementShapeType(Element::SHAPE_TYPE_LINE));
		Elementfieldtemplate eft = childMesh1d.createElementfieldtemplate(elementbasis);
		EXPECT_EQ(RESULT_OK, childElementtemplate.defineField(childCoordinates, -1, eft));
		childElement = childMesh1d.createElement(1, childElementtemplate);
		EXPECT_TRUE(childElement.isValid());
		const int nodeIdentifiers[2] = { 1, 2 };
		EXPECT_EQ(RESULT_OK, childElement.setNodesByIdentifier(eft, 2, nodeIdentifiers));
	}
	Field childCmissNumber = childFm.findFieldByName("cmiss_number");
	EXPECT_TRUE(childCmissNumber.isValid());
	FieldArgumentReal childCoordinatesArgument = childFm.createFieldArgumentReal(3);
	EXPECT_TRUE(childCoordinatesArgument.isValid());
	// test applying embedded coordinates
	FieldApply hostCoordinates = childFm.createFieldApply(embeddedCoordinates);
	EXPECT_TRUE(hostCoordinates.isValid());
	EXPECT_EQ(RESULT_OK, hostCoordinates.setName("hostCoordinates"));
	EXPECT_EQ(0, hostCoordinates.getNumberOfBindings());
	const double childXi = 0.3;
	EXPECT_EQ(RESULT_OK, childFieldcache.setMeshLocation(childElement, 1, &childXi));
	EXPECT_EQ(RESULT_OK, childFieldcache.setTime(0.0));
	// test can't evaluate until argument bindings are made
	EXPECT_FALSE(hostCoordinates.getBindArgumentSourceField(coordinatesArgument).isValid());
	const double TOL = 1.0E-12;
	const double PTOL = 1.0E-6;
	double coordinatesOut[3], pressureOut;
	const double expectedCoordinates[3] = { 0.7, 0.18, 0.3 };
	EXPECT_EQ(RESULT_ERROR_GENERAL, hostCoordinates.evaluateReal(childFieldcache, 3, coordinatesOut));
	// test some invalid bindings first:
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, hostCoordinates.setBindArgumentSourceField(coordinates, childCoordinates));  // not a FieldArgument
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, hostCoordinates.setBindArgumentSourceField(childCoordinatesArgument, childCmissNumber));  // argument from wrong region
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, hostCoordinates.setBindArgumentSourceField(coordinatesArgument, coordinates));  // source from wrong region
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, hostCoordinates.setBindArgumentSourceField(coordinatesArgument, childCmissNumber));  // wrong number of components
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, hostCoordinates.setBindArgumentSourceField(coordinatesArgument, hostCoordinates));  // can't depend on itself
	EXPECT_EQ(RESULT_OK, hostCoordinates.setBindArgumentSourceField(coordinatesArgument, childCoordinates));
	EXPECT_EQ(childCoordinates, hostCoordinates.getBindArgumentSourceField(coordinatesArgument));
	EXPECT_EQ(1, hostCoordinates.getNumberOfBindings());
	EXPECT_FALSE(hostCoordinates.getBindArgumentField(0).isValid());
	EXPECT_EQ(coordinatesArgument, hostCoordinates.getBindArgumentField(1));
	EXPECT_FALSE(hostCoordinates.getBindArgumentField(2).isValid());
	// have another host field to inspect if findMeshLocation result is cached
	FieldApply hostPressure = childFm.createFieldApply(embeddedPressure);
	EXPECT_TRUE(hostPressure.isValid());
	EXPECT_EQ(RESULT_OK, hostPressure.setName("hostPressure"));
	EXPECT_EQ(RESULT_OK, hostPressure.setBindArgumentSourceField(coordinatesArgument, childCoordinates));
	EXPECT_TRUE(hostCoordinates.isDefinedAtLocation(childFieldcache));
	EXPECT_EQ(RESULT_OK, hostCoordinates.evaluateReal(childFieldcache, 3, coordinatesOut));
	for (int i = 0; i < 3; ++i)
		EXPECT_NEAR(expectedCoordinates[i], coordinatesOut[i], TOL);
	EXPECT_EQ(RESULT_OK, childCoordinates.evaluateReal(childFieldcache, 3, coordinatesOut));
	for (int i = 0; i < 3; ++i)
		EXPECT_NEAR(expectedCoordinates[i], coordinatesOut[i], TOL);
	// test applying embedded pressure
	EXPECT_EQ(RESULT_OK, hostPressure.evaluateReal(childFieldcache, 1, &pressureOut));
	EXPECT_NEAR(91024.0, pressureOut, PTOL);
	Fieldcache fieldcache = zinc.fm.createFieldcache();
	EXPECT_FALSE(coordinatesArgument.isDefinedAtLocation(fieldcache));
	EXPECT_EQ(RESULT_OK, fieldcache.setMeshLocation(element, 3, expectedCoordinates));
	EXPECT_EQ(RESULT_OK, pressure.evaluateReal(fieldcache, 1, &pressureOut));
	EXPECT_NEAR(91024.0, pressureOut, PTOL);
	// test modifying coordinates affects pressure
	const double scaleValues[3] = { 1.1, 0.6, 1.5 };
	FieldConstant scale = zinc.fm.createFieldConstant(3, scaleValues);
	EXPECT_TRUE(scale.isValid());
	FieldMultiply scaleCoordinates = coordinates*scale;
	EXPECT_TRUE(scaleCoordinates.isValid());
	Fieldassignment fieldassignment = coordinates.createFieldassignment(scaleCoordinates);
	EXPECT_EQ(RESULT_OK, fieldassignment.assign());
	EXPECT_EQ(RESULT_OK, hostPressure.evaluateReal(childFieldcache, 1, &pressureOut));
	EXPECT_NEAR(90654.545454545456, pressureOut, PTOL);
	double scaleXi[3];
	for (int i = 0; i < 3; ++i)
		scaleXi[i] = expectedCoordinates[i]/scaleValues[i];
	EXPECT_EQ(RESULT_OK, fieldcache.setMeshLocation(element, 3, scaleXi));
	EXPECT_EQ(RESULT_OK, pressure.evaluateReal(fieldcache, 1, &pressureOut));
	EXPECT_NEAR(90654.545454545456, pressureOut, PTOL);

	// test time lookup which forces an extra cache
	const double half = 0.5;
	FieldConstant time05 = zinc.fm.createFieldConstant(1, &half);
	EXPECT_TRUE(time05.isValid());
	Field timeCoordinates = zinc.fm.createFieldTimeLookup(coordinatesArgument, time05);
	EXPECT_TRUE(timeCoordinates.isValid());
	FieldApply hostTimeCoordinates = childFm.createFieldApply(timeCoordinates);
	EXPECT_TRUE(hostTimeCoordinates.isValid());
	EXPECT_EQ(RESULT_OK, hostTimeCoordinates.setName("hostTimeCoordinates"));
	EXPECT_EQ(RESULT_OK, hostTimeCoordinates.setBindArgumentSourceField(coordinatesArgument, childCoordinates));
	EXPECT_EQ(RESULT_OK, childFieldcache.setMeshLocation(childElement, 1, &childXi));
	EXPECT_EQ(RESULT_OK, hostTimeCoordinates.evaluateReal(childFieldcache, 3, coordinatesOut));
	EXPECT_NEAR(0.74, coordinatesOut[0], TOL);
	EXPECT_NEAR(0.25, coordinatesOut[1], TOL);
	EXPECT_NEAR(0.23, coordinatesOut[2], TOL);
	// compare against actual answers calculated directly in child field cache
	EXPECT_EQ(RESULT_OK, childFieldcache.setMeshLocation(childElement, 1, &childXi));
	EXPECT_EQ(RESULT_OK, childFieldcache.setTime(0.5));
	EXPECT_EQ(RESULT_OK, childCoordinates.evaluateReal(childFieldcache, 3, coordinatesOut));
	EXPECT_NEAR(0.74, coordinatesOut[0], TOL);
	EXPECT_NEAR(0.25, coordinatesOut[1], TOL);
	EXPECT_NEAR(0.23, coordinatesOut[2], TOL);
	// since the problem is all set up, test caching of recent time values
	EXPECT_EQ(RESULT_OK, childFieldcache.setTime(0.0));
	EXPECT_EQ(RESULT_OK, childCoordinates.evaluateReal(childFieldcache, 3, coordinatesOut));
	EXPECT_NEAR(0.7, coordinatesOut[0], TOL);
	EXPECT_NEAR(0.18, coordinatesOut[1], TOL);
	EXPECT_NEAR(0.3, coordinatesOut[2], TOL);
	EXPECT_EQ(RESULT_OK, childFieldcache.setTime(0.8));
	EXPECT_EQ(RESULT_OK, childCoordinates.evaluateReal(childFieldcache, 3, coordinatesOut));
	EXPECT_NEAR(0.764, coordinatesOut[0], TOL);
	EXPECT_NEAR(0.292, coordinatesOut[1], TOL);
	EXPECT_NEAR(0.188, coordinatesOut[2], TOL);
	EXPECT_EQ(RESULT_OK, childFieldcache.setTime(0.2));
	EXPECT_EQ(RESULT_OK, childCoordinates.evaluateReal(childFieldcache, 3, coordinatesOut));
	EXPECT_NEAR(0.716, coordinatesOut[0], TOL);
	EXPECT_NEAR(0.208, coordinatesOut[1], TOL);
	EXPECT_NEAR(0.272, coordinatesOut[2], TOL);
	// test removing binding
	EXPECT_EQ(RESULT_OK, hostCoordinates.setBindArgumentSourceField(coordinatesArgument, Field()));
	EXPECT_FALSE(hostCoordinates.getBindArgumentSourceField(coordinatesArgument).isValid());
	EXPECT_EQ(0, hostCoordinates.getNumberOfBindings());
}

// Test assign to values in another region through apply
TEST(ZincFieldApply, assign)
{
	ZincTestSetupCpp zinc;

	FieldArgumentReal argument = zinc.fm.createFieldArgumentReal(3);
	EXPECT_TRUE(argument.isValid());
	FieldComponent component = zinc.fm.createFieldComponent(argument, 2);
	EXPECT_TRUE(component.isValid());

	Region childRegion = zinc.root_region.createChild("child");
	Fieldmodule childFm = childRegion.getFieldmodule();
	EXPECT_TRUE(childFm.isValid());
	Fieldcache childFieldcache = childFm.createFieldcache();
	EXPECT_TRUE(childFieldcache.isValid());

	FieldFiniteElement childCoordinates;
	const double coordinatesIn[3] = { 1.1, 2.2, 3.3 };
	Node node1;
	{
		ChangeManager<Fieldmodule> changeField(childFm);
		childCoordinates = childFm.createFieldFiniteElement(3);
		EXPECT_TRUE(childCoordinates.isValid());
		Nodeset nodes = childFm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
		Nodetemplate nodetemplate = nodes.createNodetemplate();
		EXPECT_EQ(RESULT_OK, nodetemplate.defineField(childCoordinates));
		node1 = nodes.createNode(1, nodetemplate);
		EXPECT_EQ(RESULT_OK, childFieldcache.setNode(node1));
		EXPECT_EQ(RESULT_OK, childCoordinates.assignReal(childFieldcache, 3, coordinatesIn));
	}

	EXPECT_EQ(RESULT_OK, childFieldcache.setNode(node1));
	double coordinatesOut[3];
	EXPECT_EQ(RESULT_OK, childCoordinates.evaluateReal(childFieldcache, 3, coordinatesOut));
	for (int i = 0; i < 3; ++i)
		EXPECT_DOUBLE_EQ(coordinatesIn[i], coordinatesOut[i]);

	FieldApply hostComponent = childFm.createFieldApply(component);
	EXPECT_TRUE(hostComponent.isValid());
	EXPECT_EQ(RESULT_OK, hostComponent.setBindArgumentSourceField(argument, childCoordinates));

	double y;
	EXPECT_EQ(RESULT_OK, hostComponent.evaluateReal(childFieldcache, 1, &y));
	EXPECT_DOUBLE_EQ(coordinatesIn[1], y);

	const double newY = 1.234;
	EXPECT_EQ(RESULT_OK, hostComponent.assignReal(childFieldcache, 1, &newY));
	EXPECT_EQ(RESULT_OK, childCoordinates.evaluateReal(childFieldcache, 3, coordinatesOut));
	EXPECT_DOUBLE_EQ(coordinatesIn[0], coordinatesOut[0]);
	EXPECT_DOUBLE_EQ(newY, coordinatesOut[1]);
	EXPECT_DOUBLE_EQ(coordinatesIn[2], coordinatesOut[2]);

	EXPECT_EQ(RESULT_OK, hostComponent.evaluateReal(childFieldcache, 1, &y));
	EXPECT_DOUBLE_EQ(newY, y);
}

