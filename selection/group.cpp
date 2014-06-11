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
#include <zinc/fieldconstant.hpp>
#include <zinc/fieldfiniteelement.hpp>
#include <zinc/fieldgroup.hpp>
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
}
