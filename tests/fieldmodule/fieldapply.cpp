/*
 * Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include <cmlibs/zinc/core.h>
#include <cmlibs/zinc/changemanager.hpp>
#include <cmlibs/zinc/element.hpp>
#include <cmlibs/zinc/elementbasis.hpp>
#include <cmlibs/zinc/field.hpp>
#include <cmlibs/zinc/fieldapply.hpp>
#include <cmlibs/zinc/fieldarithmeticoperators.hpp>
#include <cmlibs/zinc/fieldassignment.hpp>
#include <cmlibs/zinc/fieldcomposite.hpp>
#include <cmlibs/zinc/fieldconstant.hpp>
#include <cmlibs/zinc/fieldfiniteelement.hpp>
#include <cmlibs/zinc/fieldtime.hpp>
#include <cmlibs/zinc/node.hpp>

#include "zinctestsetupcpp.hpp"

#include "test_resources.h"

// Test applying a field imported from another region which maps from
// coordinates to mesh location to another field's values.
TEST(ZincFieldApply, importEmbeddingMap)
{
	ZincTestSetupCpp zinc;

    EXPECT_EQ(CMZN_OK, zinc.root_region.readFile(resourcePath("cube.ex2").c_str()));
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
	// test dependencies
	EXPECT_TRUE(hostCoordinates.dependsOnField(embeddedCoordinates));
	EXPECT_TRUE(embeddedCoordinates.dependsOnField(coordinatesArgument));
	EXPECT_TRUE(hostCoordinates.dependsOnField(coordinatesArgument));
	EXPECT_TRUE(hostCoordinates.dependsOnField(childCoordinates));
	EXPECT_FALSE(coordinatesArgument.dependsOnField(childCoordinates));
	EXPECT_FALSE(embeddedCoordinates.dependsOnField(childCoordinates));
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

// check the region/field structure of ZincFieldApply assign test
void checkZincFieldApply_assign(Region& root)
{
	Fieldmodule fieldmodule = root.getFieldmodule();
	FieldArgumentReal argument = fieldmodule.findFieldByName("argument").castArgumentReal();
	EXPECT_TRUE(argument.isValid());
	EXPECT_EQ(3, argument.getNumberOfComponents());
	FieldComponent component = fieldmodule.findFieldByName("component").castComponent();
	EXPECT_TRUE(component.isValid());
	EXPECT_EQ(1, component.getNumberOfComponents());
	Region child = root.findChildByName("child");
	Fieldmodule childFm = child.getFieldmodule();
	FieldFiniteElement childCoordinates = childFm.findFieldByName("coordinates").castFiniteElement();
	EXPECT_TRUE(childCoordinates.isValid());
	EXPECT_EQ(3, childCoordinates.getNumberOfComponents());
	EXPECT_TRUE(childCoordinates.isTypeCoordinate());
	FieldApply hostComponent = childFm.findFieldByName("hostComponent").castApply();
	EXPECT_TRUE(hostComponent.isValid());
	EXPECT_EQ(1, hostComponent.getNumberOfComponents());
	EXPECT_EQ(3, hostComponent.getNumberOfSourceFields());
	EXPECT_EQ(component, hostComponent.getSourceField(1));
	EXPECT_EQ(1, hostComponent.getNumberOfBindings());
	EXPECT_EQ(argument, hostComponent.getBindArgumentField(1));
	EXPECT_EQ(childCoordinates, hostComponent.getBindArgumentSourceField(argument));
}

// Test assign to values in another region through apply
// Also tests serialisation
TEST(ZincFieldApply, assign)
{
	ZincTestSetupCpp zinc;

	FieldArgumentReal argument = zinc.fm.createFieldArgumentReal(3);
	EXPECT_TRUE(argument.isValid());
	EXPECT_EQ(RESULT_OK, argument.setName("argument"));
	FieldComponent component = zinc.fm.createFieldComponent(argument, 2);
	EXPECT_TRUE(component.isValid());
	EXPECT_EQ(RESULT_OK, component.setName("component"));

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
		EXPECT_EQ(RESULT_OK, childCoordinates.setName("coordinates"));
		EXPECT_EQ(RESULT_OK, childCoordinates.setTypeCoordinate(true));
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
	EXPECT_EQ(RESULT_OK, hostComponent.setName("hostComponent"));
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
	checkZincFieldApply_assign(zinc.root_region);

	// test writing field descriptions and reading into new region trees in different orders
	char *rootDescription = zinc.fm.writeDescription();
	char *childDescription = childFm.writeDescription();

	// test reading root then child descriptions
	Region root1 = zinc.context.createRegion();
	EXPECT_EQ(RESULT_OK, root1.setName("root1")); // for debugging purposes
	Fieldmodule fieldmodule1 = root1.getFieldmodule();
	EXPECT_EQ(RESULT_OK, fieldmodule1.readDescription(rootDescription));
	Region child1 = root1.createChild("child");
	Fieldmodule child1Fm = child1.getFieldmodule();
	EXPECT_EQ(RESULT_OK, child1Fm.readDescription(childDescription));
	checkZincFieldApply_assign(root1);

	// test reading child then root descriptions & look for placeholder evaluate and automatic argument
	Region root2 = zinc.context.createRegion();
	EXPECT_EQ(RESULT_OK, root2.setName("root2")); // for debugging purposes
	Region child2 = root2.createChild("child");
	Fieldmodule child2Fm = child2.getFieldmodule();
	EXPECT_EQ(RESULT_OK, child2Fm.readDescription(childDescription));
	Fieldmodule fieldmodule2 = root2.getFieldmodule();
	// see if a dummy constant component field has been created
	Field dummyComponent = fieldmodule2.findFieldByName("component");
	EXPECT_TRUE(dummyComponent.isValid());
	EXPECT_FALSE(dummyComponent.castComponent().isValid());
	// see if an argument real has been automatically made
	FieldArgumentReal argument2 = fieldmodule2.findFieldByName("argument").castArgumentReal();
	EXPECT_TRUE(argument2.isValid());
	EXPECT_EQ(RESULT_OK, fieldmodule2.readDescription(rootDescription));
	checkZincFieldApply_assign(root2);

	cmzn_deallocate(childDescription);
	cmzn_deallocate(rootDescription);
}

// Set up apply of a finite element field in a sibling region
// Serialise it and ensure reading EX file over other region can re-establish the
// finite element field.
// A bit academic as can't yet apply a finite element field properly, but
// can evaluate it if you give a target domain location (for now).
TEST(ZincFieldApply, serialiseSibling)
{
	ZincTestSetupCpp zinc;

	Region child1 = zinc.root_region.createChild("child1");
	Fieldmodule child1Fm = child1.getFieldmodule();
	EXPECT_TRUE(child1Fm.isValid());

    EXPECT_EQ(CMZN_OK, child1.readFile(resourcePath("cube.ex2").c_str()));
	Mesh mesh3d = child1Fm.findMeshByDimension(3);
	EXPECT_TRUE(mesh3d.isValid());
	Element element = mesh3d.findElementByIdentifier(1);
	EXPECT_TRUE(element.isValid());
	FieldFiniteElement coordinates = child1Fm.findFieldByName("coordinates").castFiniteElement();
	EXPECT_TRUE(coordinates.isValid());

	Region child2 = zinc.root_region.createChild("child2");
	Fieldmodule child2Fm = child2.getFieldmodule();
	EXPECT_TRUE(child2Fm.isValid());

	FieldApply applyCoordinates = child2Fm.createFieldApply(coordinates);
	EXPECT_EQ(RESULT_OK, applyCoordinates.setName("applyCoordinates"));
	EXPECT_TRUE(applyCoordinates.isValid());

	// try to evaluate at a child1 mesh location
	// note this is not promised to work in future
	Fieldcache fieldcache2 = child2Fm.createFieldcache();
	EXPECT_TRUE(fieldcache2.isValid());
	const double xi[3] = { 0.2, 0.8, 0.3 };
	const double TOL = 1.0E-12;
	EXPECT_EQ(RESULT_OK, fieldcache2.setMeshLocation(element, 3, xi));
	double coordinatesOut[3];
	EXPECT_EQ(RESULT_OK, applyCoordinates.evaluateReal(fieldcache2, 3, coordinatesOut));
	for (int i = 0; i < 3; ++i)
		EXPECT_NEAR(xi[i], coordinatesOut[i], TOL);

	// test writing field descriptions for child2 and re-reading to automatically create child1 and field
	// then read EX file to overwrite dummy evaluate field.
	char *child2Description = child2Fm.writeDescription();

	// test reading root then child descriptions
	Region rootb = zinc.context.createRegion();
	EXPECT_EQ(RESULT_OK, rootb.setName("rootb")); // for debugging purposes

	Region childb2 = rootb.createChild("child2");
	Fieldmodule childb2Fm = childb2.getFieldmodule();
	EXPECT_EQ(RESULT_OK, childb2Fm.readDescription(child2Description));
	// check child1 has been created and dummy coordinates field:
	Region childb1 = rootb.findChildByName("child1");
	EXPECT_TRUE(childb1.isValid());
	Fieldmodule childb1Fm = childb1.getFieldmodule();
	Field coordinatesb = childb1Fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinatesb.isValid());
	EXPECT_EQ(3, coordinatesb.getNumberOfComponents());
	EXPECT_FALSE(coordinatesb.castFiniteElement().isValid());
    EXPECT_EQ(CMZN_OK, childb1.readFile(resourcePath("cube.ex2").c_str()));
	// dummyCop constant handle should be defunct
	EXPECT_TRUE(coordinatesb.castFiniteElement().isValid());
	FieldFiniteElement coordinatesb2 = childb1Fm.findFieldByName("coordinates").castFiniteElement();
	EXPECT_EQ(coordinatesb, coordinatesb2);

	cmzn_deallocate(child2Description);
}

// Test evaluation results when rebinding same argument to different sources
TEST(ZincFieldApply, rebinding)
{
	ZincTestSetupCpp zinc;

	const double valueOne = 1.0;
	FieldConstant one = zinc.fm.createFieldConstant(1, &valueOne);
	EXPECT_TRUE(one.isValid());
	EXPECT_EQ(RESULT_OK, one.setName("one"));
	const double valueTwo = 2.0;
	FieldConstant two = zinc.fm.createFieldConstant(1, &valueTwo);
	EXPECT_TRUE(two.isValid());
	EXPECT_EQ(RESULT_OK, two.setName("two"));
	FieldArgumentReal argument = zinc.fm.createFieldArgumentReal(1);
	EXPECT_TRUE(argument.isValid());
	EXPECT_EQ(RESULT_OK, argument.setName("argument"));
	FieldApply applyOne = zinc.fm.createFieldApply(argument);
	EXPECT_TRUE(applyOne.isValid());
	EXPECT_EQ(RESULT_OK, applyOne.setName("applyOne"));
	EXPECT_EQ(RESULT_OK, applyOne.setBindArgumentSourceField(argument, one));
	FieldApply applyTwo = zinc.fm.createFieldApply(argument);
	EXPECT_TRUE(applyTwo.isValid());
	EXPECT_EQ(RESULT_OK, applyTwo.setName("applyTwo"));
	EXPECT_EQ(RESULT_OK, applyTwo.setBindArgumentSourceField(argument, two));
	FieldAdd addArgumentApplyOne = zinc.fm.createFieldAdd(argument, applyOne);
	EXPECT_TRUE(addArgumentApplyOne.isValid());
	EXPECT_EQ(RESULT_OK, addArgumentApplyOne.setName("addArgumentApplyOne"));
	FieldApply applyAddArgumentApplyOne = zinc.fm.createFieldApply(addArgumentApplyOne);
	EXPECT_TRUE(applyAddArgumentApplyOne.isValid());
	EXPECT_EQ(RESULT_OK, applyAddArgumentApplyOne.setName("applyAddArgumentApplyOne"));
	EXPECT_EQ(RESULT_OK, applyAddArgumentApplyOne.setBindArgumentSourceField(argument, one));
	FieldAdd addArgumentApplyTwo = zinc.fm.createFieldAdd(argument, applyTwo);
	EXPECT_TRUE(addArgumentApplyTwo.isValid());
	EXPECT_EQ(RESULT_OK, addArgumentApplyTwo.setName("addArgumentApplyTwo"));
	FieldApply applyAddArgumentApplyTwo = zinc.fm.createFieldApply(addArgumentApplyTwo);
	EXPECT_TRUE(applyAddArgumentApplyTwo.isValid());
	EXPECT_EQ(RESULT_OK, applyAddArgumentApplyTwo.setName("applyAddArgumentApplyTwo"));
	EXPECT_EQ(RESULT_OK, applyAddArgumentApplyTwo.setBindArgumentSourceField(argument, one));

	Fieldcache fieldcache = zinc.fm.createFieldcache();
	EXPECT_TRUE(fieldcache.isValid());
	double valueOut;
	// check that values for one binding are not accepted for a different binding
	EXPECT_EQ(RESULT_OK, applyOne.evaluateReal(fieldcache, 1, &valueOut));
	EXPECT_DOUBLE_EQ(1.0, valueOut);
	EXPECT_EQ(RESULT_OK, applyTwo.evaluateReal(fieldcache, 1, &valueOut));
	EXPECT_DOUBLE_EQ(2.0, valueOut);
	// check rebinding of argument with the same source
	EXPECT_EQ(RESULT_OK, applyAddArgumentApplyOne.evaluateReal(fieldcache, 1, &valueOut));
	EXPECT_DOUBLE_EQ(2.0, valueOut);
	// check rebinding of argument with a different source
	EXPECT_EQ(RESULT_OK, applyAddArgumentApplyTwo.evaluateReal(fieldcache, 1, &valueOut));
	EXPECT_DOUBLE_EQ(3.0, valueOut);
}

// Test can't make infinite cycle of multiple bindings, but otherwise
// can bind source fields that are functions of other arguments
TEST(ZincFieldApply, cycleBindings)
{
	ZincTestSetupCpp zinc;

	FieldArgumentReal argument1 = zinc.fm.createFieldArgumentReal(3);
	EXPECT_TRUE(argument1.isValid());
	FieldArgumentReal argument2 = zinc.fm.createFieldArgumentReal(3);
	EXPECT_TRUE(argument2.isValid());
	FieldAdd addArguments = zinc.fm.createFieldAdd(argument1, argument2);
	EXPECT_TRUE(addArguments.isValid());
	FieldMultiply squaredArgument1 = zinc.fm.createFieldMultiply(argument1, argument1);
	EXPECT_TRUE(squaredArgument1.isValid());
	EXPECT_EQ(RESULT_OK, squaredArgument1.setName("squaredArgument1"));
	FieldMultiply squaredArgument2 = zinc.fm.createFieldMultiply(argument2, argument2);
	EXPECT_TRUE(squaredArgument2.isValid());
	EXPECT_EQ(RESULT_OK, squaredArgument2.setName("squaredArgument2"));
	FieldAdd doubleAddArguments = zinc.fm.createFieldAdd(addArguments, addArguments);
	EXPECT_TRUE(addArguments.isValid());

	FieldApply applyAddArguments = zinc.fm.createFieldApply(addArguments);
	EXPECT_TRUE(applyAddArguments.isValid());
	EXPECT_TRUE(applyAddArguments.dependsOnField(addArguments));
	EXPECT_TRUE(applyAddArguments.dependsOnField(argument1));
	EXPECT_TRUE(applyAddArguments.dependsOnField(argument2));
	EXPECT_FALSE(applyAddArguments.dependsOnField(squaredArgument1));
	EXPECT_FALSE(applyAddArguments.dependsOnField(squaredArgument2));
	EXPECT_FALSE(applyAddArguments.dependsOnField(doubleAddArguments));
	// test some invalid bindings on a dummy apply field
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, applyAddArguments.setBindArgumentSourceField(argument1, applyAddArguments));  // source field depends on apply field
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, applyAddArguments.setBindArgumentSourceField(argument1, doubleAddArguments));  // source field depends on evaluate field
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, applyAddArguments.setBindArgumentSourceField(argument2, addArguments));  // source field depends on evaluate field
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, applyAddArguments.setBindArgumentSourceField(argument1, squaredArgument1));  // source field depends on argument
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, applyAddArguments.setBindArgumentSourceField(argument2, squaredArgument2));  // source field depends on argument
	EXPECT_EQ(RESULT_OK, applyAddArguments.setBindArgumentSourceField(argument1, squaredArgument2));  // OK = eliminates argument1, so apply only depends on argument2
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, applyAddArguments.setBindArgumentSourceField(argument2, squaredArgument1));  // NOT OK = infinite recursion between the arguments
	const double constantValues[3] = { 1.0, 2.0, 3.0 };
	FieldConstant constants = zinc.fm.createFieldConstant(3, constantValues);
	EXPECT_TRUE(constants.isValid());
	EXPECT_EQ(RESULT_OK, constants.setName("constants"));
	EXPECT_EQ(RESULT_OK, applyAddArguments.setBindArgumentSourceField(argument2, constants));  // OK

	Fieldcache fieldcache = zinc.fm.createFieldcache();
	EXPECT_TRUE(fieldcache.isValid());
	double valuesOut[3];
	EXPECT_EQ(RESULT_OK, applyAddArguments.evaluateReal(fieldcache, 3, valuesOut));
	const double TOL = 1.0E-12;
	EXPECT_NEAR(2.0, valuesOut[0], TOL);
	EXPECT_NEAR(6.0, valuesOut[1], TOL);
	EXPECT_NEAR(12.0, valuesOut[2], TOL);
}
