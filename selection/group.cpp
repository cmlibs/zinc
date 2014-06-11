/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#if 0
#include <zinc/core.h>
#include <zinc/field.h>
#include <zinc/fieldfiniteelement.h>
#include <zinc/fieldgroup.h>
#include <zinc/fieldsubobjectgroup.h>
#include <zinc/node.h>
#include <zinc/scene.h>
#include <zinc/selection.h>
#endif

#include <zinc/element.hpp>
#include <zinc/field.hpp>
#include <zinc/fieldcache.hpp>
#include <zinc/fieldcomposite.hpp>
#include <zinc/fieldconstant.hpp>
#include <zinc/fieldfiniteelement.hpp>
#include <zinc/fieldgroup.hpp>
#include <zinc/fieldlogicaloperators.hpp>
#include <zinc/fieldsubobjectgroup.hpp>
#include <zinc/node.hpp>
#include <zinc/region.hpp>

#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"

#include "test_resources.h"

TEST(ZincFieldElementGroup, add_remove_conditional)
{
	ZincTestSetupCpp zinc;
	int result;

	EXPECT_EQ(OK, result = zinc.root_region.readFile(
		TestResources::getLocation(TestResources::FIELDMODULE_TWO_CUBES_RESOURCE)));

	FieldGroup group = zinc.fm.createFieldGroup();
	EXPECT_TRUE(group.isValid());

	int size1, size2, size3;
	Mesh mesh3d = zinc.fm.findMeshByDimension(3);
	EXPECT_EQ(2, size3 = mesh3d.getSize());
	Mesh mesh2d = zinc.fm.findMeshByDimension(2);
	EXPECT_EQ(11, size2 = mesh2d.getSize());
	Mesh mesh1d = zinc.fm.findMeshByDimension(1);
	EXPECT_EQ(20, size1 = mesh1d.getSize());

	FieldElementGroup facesGroup = group.createFieldElementGroup(mesh2d);
	EXPECT_TRUE(facesGroup.isValid());
	MeshGroup facesMeshGroup = facesGroup.getMeshGroup();
	EXPECT_TRUE(facesMeshGroup.isValid());
	EXPECT_EQ(0, facesMeshGroup.getSize());

	EXPECT_EQ(ERROR_ARGUMENT, facesMeshGroup.addElementsConditional(Field()));
	EXPECT_EQ(ERROR_ARGUMENT, facesMeshGroup.removeElementsConditional(Field()));

	FieldElementGroup linesGroup = group.createFieldElementGroup(mesh1d);
	EXPECT_TRUE(linesGroup.isValid());
	MeshGroup linesMeshGroup = linesGroup.getMeshGroup();
	EXPECT_TRUE(linesMeshGroup.isValid());
	EXPECT_EQ(0, linesMeshGroup.getSize());

	FieldIsOnFace isOnFaceField = zinc.fm.createFieldIsOnFace(Element::FACE_TYPE_XI1_0);
	EXPECT_TRUE(isOnFaceField.isValid());

	const double oneValue = 1.0;
	FieldConstant trueField = zinc.fm.createFieldConstant(1, &oneValue);
	EXPECT_TRUE(trueField.isValid());

	Element element1 = mesh2d.findElementByIdentifier(1);
	Element element2 = mesh2d.findElementByIdentifier(2);

	EXPECT_EQ(OK, facesMeshGroup.addElementsConditional(isOnFaceField));
	EXPECT_EQ(2, result = facesMeshGroup.getSize());
	EXPECT_TRUE(facesMeshGroup.containsElement(element1));
	EXPECT_TRUE(facesMeshGroup.containsElement(element2));

	EXPECT_EQ(0, linesMeshGroup.getSize());

	// copy into another element group to test conditional add/removal
	// optimisation for smaller groups
	FieldElementGroup otherFacesGroup = zinc.fm.createFieldElementGroup(mesh2d);
	EXPECT_TRUE(otherFacesGroup.isValid());
	MeshGroup otherFacesMeshGroup = otherFacesGroup.getMeshGroup();
	EXPECT_TRUE(otherFacesMeshGroup.isValid());
	EXPECT_EQ(OK, otherFacesMeshGroup.addElementsConditional(facesGroup));
	EXPECT_EQ(2, result = otherFacesMeshGroup.getSize());
	EXPECT_TRUE(otherFacesMeshGroup.containsElement(element1));
	EXPECT_TRUE(otherFacesMeshGroup.containsElement(element2));

	// add all faces for conditional removal
	EXPECT_EQ(OK, facesMeshGroup.addElementsConditional(trueField));
	EXPECT_EQ(11, result = facesMeshGroup.getSize());

	EXPECT_EQ(OK, facesMeshGroup.removeElementsConditional(isOnFaceField));
	EXPECT_EQ(9, result = facesMeshGroup.getSize());
	EXPECT_FALSE(facesMeshGroup.containsElement(element1));
	EXPECT_FALSE(facesMeshGroup.containsElement(element2));

	// test using self as a conditional
	EXPECT_EQ(OK, facesMeshGroup.removeElementsConditional(facesGroup));
	EXPECT_EQ(0, facesMeshGroup.getSize());

	EXPECT_EQ(OK, facesMeshGroup.addElementsConditional(trueField));
	EXPECT_EQ(11, result = facesMeshGroup.getSize());

	EXPECT_EQ(OK, facesMeshGroup.removeElementsConditional(otherFacesGroup));
	EXPECT_EQ(9, result = facesMeshGroup.getSize());
	EXPECT_FALSE(facesMeshGroup.containsElement(element1));
	EXPECT_FALSE(facesMeshGroup.containsElement(element2));

	// check not an error to remove nodes already removed
	EXPECT_EQ(OK, facesMeshGroup.removeAllElements());
	EXPECT_EQ(0, result = facesMeshGroup.getSize());
	EXPECT_EQ(OK, facesMeshGroup.removeElementsConditional(otherFacesGroup));
}

TEST(ZincFieldNodeGroup, add_remove_conditional)
{
	ZincTestSetupCpp zinc;
	int result;

	EXPECT_EQ(OK, result = zinc.root_region.readFile(
		TestResources::getLocation(TestResources::FIELDMODULE_TWO_CUBES_RESOURCE)));

	FieldGroup group = zinc.fm.createFieldGroup();
	EXPECT_TRUE(group.isValid());

	int size;
	Nodeset nodeset = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_EQ(12, size = nodeset.getSize());

	FieldNodeGroup nodesGroup = group.createFieldNodeGroup(nodeset);
	EXPECT_TRUE(nodesGroup.isValid());
	NodesetGroup nodesetGroup = nodesGroup.getNodesetGroup();
	EXPECT_TRUE(nodesetGroup.isValid());
	EXPECT_EQ(0, nodesetGroup.getSize());

	EXPECT_EQ(ERROR_ARGUMENT, nodesetGroup.addNodesConditional(Field()));
	EXPECT_EQ(ERROR_ARGUMENT, nodesetGroup.removeNodesConditional(Field()));

	Field coordinates = zinc.fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinates.isValid());
	FieldComponent x = zinc.fm.createFieldComponent(coordinates, 1);
	EXPECT_TRUE(x.isValid());
	const double fifteen = 15.0;
	FieldGreaterThan x_gt_15 = x > zinc.fm.createFieldConstant(1, &fifteen);
	EXPECT_TRUE(x_gt_15.isValid());

	const double oneValue = 1.0;
	FieldConstant trueField = zinc.fm.createFieldConstant(1, &oneValue);
	EXPECT_TRUE(trueField.isValid());

	EXPECT_EQ(OK, nodesetGroup.addNodesConditional(x_gt_15));
	EXPECT_EQ(4, result = nodesetGroup.getSize());
	for (int i = 3; i <= 12; i += 3)
		EXPECT_TRUE(nodesetGroup.containsNode(nodeset.findNodeByIdentifier(i)));

	// copy into another element group to test conditional add/removal
	// optimisation for smaller groups
	FieldNodeGroup otherNodesGroup = zinc.fm.createFieldNodeGroup(nodeset);
	EXPECT_TRUE(otherNodesGroup.isValid());
	NodesetGroup otherNodesetGroup = otherNodesGroup.getNodesetGroup();
	EXPECT_TRUE(otherNodesetGroup.isValid());
	EXPECT_EQ(OK, otherNodesetGroup.addNodesConditional(nodesGroup));
	EXPECT_EQ(4, result = otherNodesetGroup.getSize());
	for (int i = 3; i <= 12; i += 3)
		EXPECT_TRUE(otherNodesetGroup.containsNode(nodeset.findNodeByIdentifier(i)));

	// add all faces for conditional removal
	EXPECT_EQ(OK, nodesetGroup.addNodesConditional(trueField));
	EXPECT_EQ(12, result = nodesetGroup.getSize());

	EXPECT_EQ(OK, nodesetGroup.removeNodesConditional(x_gt_15));
	EXPECT_EQ(8, result = nodesetGroup.getSize());
	for (int i = 3; i <= 12; i += 3)
		EXPECT_FALSE(nodesetGroup.containsNode(nodeset.findNodeByIdentifier(i)));

	// test using self as a conditional
	EXPECT_EQ(OK, nodesetGroup.removeNodesConditional(nodesGroup));
	EXPECT_EQ(0, nodesetGroup.getSize());

	EXPECT_EQ(OK, nodesetGroup.addNodesConditional(trueField));
	EXPECT_EQ(12, result = nodesetGroup.getSize());

	EXPECT_EQ(OK, nodesetGroup.removeNodesConditional(otherNodesGroup));
	EXPECT_EQ(8, result = nodesetGroup.getSize());
	for (int i = 3; i <= 12; i += 3)
		EXPECT_FALSE(nodesetGroup.containsNode(nodeset.findNodeByIdentifier(i)));

	// check not an error to remove nodes already removed
	EXPECT_EQ(OK, nodesetGroup.removeAllNodes());
	EXPECT_EQ(0, result = nodesetGroup.getSize());
	EXPECT_EQ(OK, nodesetGroup.removeNodesConditional(otherNodesGroup));
}
