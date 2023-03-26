/*
 * Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include <opencmiss/zinc/core.h>
#include <opencmiss/zinc/field.hpp>
#include <opencmiss/zinc/fieldassignment.hpp>
#include <opencmiss/zinc/fieldcache.hpp>
#include <opencmiss/zinc/fieldconditional.hpp>
#include <opencmiss/zinc/fieldconstant.hpp>
#include <opencmiss/zinc/fieldfiniteelement.hpp>
#include <opencmiss/zinc/fieldlogicaloperators.hpp>
#include <opencmiss/zinc/fieldsubobjectgroup.hpp>

#include "zinctestsetupcpp.hpp"
#include "test_resources.h"

// Issue 214: FieldIf should report value type of source fields 2, 3. Mesh location case.
TEST(ZincFieldIf, valueTypeMeshLocation)
{
	ZincTestSetupCpp zinc;

	// a handy model with nodes 1-4 in the corners of a square and nodes 5-8 with host locations
    EXPECT_EQ(RESULT_OK, zinc.root_region.readFile(resourcePath("fieldmodule/embedding_issue3614.exregion").c_str()));

	Mesh mesh2d = zinc.fm.findMeshByDimension(2);
	EXPECT_TRUE(mesh2d.isValid());
	Field coordinates = zinc.fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinates.isValid());
	Field dataCoordinates = zinc.fm.findFieldByName("data_coordinates");
	EXPECT_TRUE(dataCoordinates.isValid());
	Field hostLocation = zinc.fm.findFieldByName("host_location");
	EXPECT_TRUE(hostLocation.isValid());
	EXPECT_EQ(Field::VALUE_TYPE_MESH_LOCATION, hostLocation.getValueType());

	FieldFindMeshLocation findHostLocation = zinc.fm.createFieldFindMeshLocation(dataCoordinates, coordinates, mesh2d);
	EXPECT_EQ(RESULT_OK, findHostLocation.setSearchMode(FieldFindMeshLocation::SEARCH_MODE_NEAREST));
	EXPECT_EQ(Field::VALUE_TYPE_MESH_LOCATION, findHostLocation.getValueType());
	FieldIsDefined hasDataCoordinates = zinc.fm.createFieldIsDefined(dataCoordinates);
	EXPECT_TRUE(hasDataCoordinates.isValid());

	FieldIf conditionalHostLocation = zinc.fm.createFieldIf(hasDataCoordinates, findHostLocation, hostLocation);
	EXPECT_TRUE(conditionalHostLocation.isValid());
	EXPECT_EQ(Field::VALUE_TYPE_MESH_LOCATION, conditionalHostLocation.getValueType());

	Element element1 = mesh2d.findElementByIdentifier(1);
	EXPECT_TRUE(element1.isValid());

	const double expectedFindXi[4][2] = {
		{ 0.12, 0.10 },
		{ 0.76, 0.20 },
		{ 0.92, 0.80 },
		{ 0.31, 0.80 }
	};
	const double expectedXi[4][2] = {
		{ 0.10, 0.10 },
		{ 0.75, 0.25 },
		{ 0.90, 0.75 },
		{ 0.33, 0.90 }
	};
	Nodeset nodes = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_TRUE(nodes.isValid());
	Fieldcache fieldcache = zinc.fm.createFieldcache();
	Element element;
	double xi[2];
	const double TOL = 1.0E-9;
	for (int n = 0; n < 4; ++n)
	{
		Node node = nodes.findNodeByIdentifier(n + 5);
		EXPECT_TRUE(node.isValid());
		EXPECT_EQ(RESULT_OK, fieldcache.setNode(node));
		element = conditionalHostLocation.evaluateMeshLocation(fieldcache, 2, xi);
		EXPECT_EQ(element1, element);
		for (int c = 0; c < 2; ++c)
			EXPECT_NEAR(expectedFindXi[n][c], xi[c], TOL);
		element = findHostLocation.evaluateMeshLocation(fieldcache, 2, xi);
		EXPECT_EQ(element1, element);
		for (int c = 0; c < 2; ++c)
			EXPECT_NEAR(expectedFindXi[n][c], xi[c], TOL);
		element = hostLocation.evaluateMeshLocation(fieldcache, 2, xi);
		EXPECT_EQ(element1, element);
		for (int c = 0; c < 2; ++c)
			EXPECT_DOUBLE_EQ(expectedXi[n][c], xi[c]);
	}

	// test field assignment works:
	Fieldassignment fieldassignment = hostLocation.createFieldassignment(conditionalHostLocation);
	EXPECT_TRUE(fieldassignment.isValid());
	// only defined on nodes 5-8:
	EXPECT_EQ(RESULT_WARNING_PART_DONE, fieldassignment.assign());
	for (int n = 0; n < 4; ++n)
	{
		Node node = nodes.findNodeByIdentifier(n + 5);
		EXPECT_TRUE(node.isValid());
		EXPECT_EQ(RESULT_OK, fieldcache.setNode(node));
		element = hostLocation.evaluateMeshLocation(fieldcache, 2, xi);
		EXPECT_EQ(element1, element);
		for (int c = 0; c < 2; ++c)
			EXPECT_NEAR(expectedFindXi[n][c], xi[c], TOL);
	}
}

// Test can't create FieldIf for source fields with different host meshes
TEST(ZincFieldIf, valueTypeMeshLocationDifferentHostMesh)
{
	ZincTestSetupCpp zinc;

	Mesh mesh2d = zinc.fm.findMeshByDimension(2);
	EXPECT_TRUE(mesh2d.isValid());
	Mesh mesh3d = zinc.fm.findMeshByDimension(3);
	EXPECT_TRUE(mesh3d.isValid());

	FieldStoredMeshLocation meshLocation2d = zinc.fm.createFieldStoredMeshLocation(mesh2d);
	EXPECT_TRUE(meshLocation2d.isValid());
	FieldStoredMeshLocation meshLocation3d = zinc.fm.createFieldStoredMeshLocation(mesh3d);
	EXPECT_TRUE(meshLocation3d.isValid());
	const double constOne = 1.0;
	FieldConstant one = zinc.fm.createFieldConstant(1, &constOne);
	EXPECT_TRUE(one.isValid());

	FieldIf conditionalMeshLocation = zinc.fm.createFieldIf(one, meshLocation2d, meshLocation3d);
	EXPECT_FALSE(conditionalMeshLocation.isValid());
	conditionalMeshLocation = zinc.fm.createFieldIf(one, meshLocation2d, meshLocation2d);
	EXPECT_TRUE(conditionalMeshLocation.isValid());
	conditionalMeshLocation = zinc.fm.createFieldIf(one, meshLocation3d, meshLocation3d);
	EXPECT_TRUE(conditionalMeshLocation.isValid());

	// test can't make field assignment with different host meshes:
	Fieldassignment fieldassignment = meshLocation3d.createFieldassignment(meshLocation2d);
	EXPECT_FALSE(fieldassignment.isValid());
}

// Issue 214: FieldIf should report value type of source fields 2, 3. String case.
TEST(ZincFieldIf, valueTypeString)
{
	ZincTestSetupCpp zinc;
    char *text = nullptr;

	FieldStringConstant constantString = zinc.fm.createFieldStringConstant("fred");
	EXPECT_TRUE(constantString.isValid());
	EXPECT_EQ(Field::VALUE_TYPE_STRING, constantString.getValueType());
	FieldStoredString storedString = zinc.fm.createFieldStoredString();
	EXPECT_TRUE(storedString.isValid());
	EXPECT_EQ(Field::VALUE_TYPE_STRING, storedString.getValueType());

	Nodeset nodes = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_TRUE(nodes.isValid());
	Nodetemplate nodetemplate = nodes.createNodetemplate();
	EXPECT_TRUE(nodetemplate.isValid());
	EXPECT_EQ(RESULT_OK, nodetemplate.defineField(storedString));

	Node node1 = nodes.createNode(1, nodetemplate);
	EXPECT_TRUE(node1.isValid());
	Fieldcache fieldcache = zinc.fm.createFieldcache();
	EXPECT_EQ(RESULT_OK, fieldcache.setNode(node1));
	EXPECT_EQ(RESULT_OK, storedString.assignString(fieldcache, "bob"));

	const double constOne = 1.0;
	FieldConstant one = zinc.fm.createFieldConstant(1, &constOne);
	FieldIf conditionalString = zinc.fm.createFieldIf(one, storedString, constantString);
	EXPECT_TRUE(conditionalString.isValid());
	EXPECT_EQ(Field::VALUE_TYPE_STRING, conditionalString.getValueType());

	// reset field cache and evaluate fields
	fieldcache = zinc.fm.createFieldcache();
	EXPECT_EQ(RESULT_OK, fieldcache.setNode(node1));
    EXPECT_STREQ("bob", text = storedString.evaluateString(fieldcache));
    cmzn_deallocate(text);
    EXPECT_STREQ("bob", text = conditionalString.evaluateString(fieldcache));
    cmzn_deallocate(text);
    EXPECT_STREQ("fred", text = constantString.evaluateString(fieldcache));
    cmzn_deallocate(text);

	// test field assignment works:
	Fieldassignment fieldassignment = storedString.createFieldassignment(constantString);
	EXPECT_TRUE(fieldassignment.isValid());
	EXPECT_EQ(RESULT_OK, fieldassignment.assign());
	EXPECT_EQ(RESULT_OK, constantString.assignString(fieldcache, "bob"));

    EXPECT_STREQ("fred", text = storedString.evaluateString(fieldcache));
    cmzn_deallocate(text);
    EXPECT_STREQ("fred", text = conditionalString.evaluateString(fieldcache));
    cmzn_deallocate(text);
    EXPECT_STREQ("bob", text = constantString.evaluateString(fieldcache));
    cmzn_deallocate(text);
}
