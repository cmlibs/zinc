/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include <opencmiss/zinc/node.hpp>
#include <opencmiss/zinc/field.hpp>
#include <opencmiss/zinc/fieldcache.hpp>
#include <opencmiss/zinc/fieldgroup.hpp>

#include "utilities/zinctestsetupcpp.hpp"

#include "test_resources.h"

// Test reading EX file containing both a data and node group of same name.
TEST(FieldIO, data_and_node_group)
{
	ZincTestSetupCpp zinc;
	int result;

	EXPECT_EQ(OK, result = zinc.root_region.readFile(
		TestResources::getLocation(TestResources::FIELDIO_EXF_DATA_AND_NODE_GROUP_RESOURCE)));

	FieldGroup group = zinc.fm.findFieldByName("bob").castGroup();
	EXPECT_TRUE(group.isValid());

	Nodeset nodes = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	NodesetGroup nodeGroup = group.getFieldNodeGroup(nodes).getNodesetGroup();
	EXPECT_TRUE(nodeGroup.isValid());
	EXPECT_EQ(1, nodeGroup.getSize());
	Node node = nodeGroup.findNodeByIdentifier(1);
	EXPECT_TRUE(node.isValid());

	Nodeset datapoints = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_DATAPOINTS);
	NodesetGroup dataGroup = group.getFieldNodeGroup(datapoints).getNodesetGroup();
	EXPECT_TRUE(dataGroup.isValid());
	EXPECT_EQ(1, dataGroup.getSize());
	Node datapoint = dataGroup.findNodeByIdentifier(2);
	EXPECT_TRUE(datapoint.isValid());
}

// Test can overwrite nodes and elements with different definitions
TEST(FieldIO, changeNodeElementDefinition)
{
	ZincTestSetupCpp zinc;

	EXPECT_EQ(RESULT_OK, zinc.root_region.readFile(TestResources::getLocation(TestResources::FIELDIO_EX_TWOHERMITECUBES_NOSCALEFACTORS_RESOURCE)));
	EXPECT_EQ(RESULT_OK, zinc.root_region.readFile(TestResources::getLocation(TestResources::FIELDMODULE_EX2_TWO_CUBES_HERMITE_NOCROSS_RESOURCE)));
}

// Test reading EX file with an empty string value
TEST(FieldIO, exEmptyString)
{
	ZincTestSetupCpp zinc;

	EXPECT_EQ(RESULT_OK, zinc.root_region.readFile(TestResources::getLocation(TestResources::FIELDIO_EXF_EMPTY_STRING_RESOURCE)));

	Nodeset nodes = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	Node node1 = nodes.findNodeByIdentifier(1);
	EXPECT_TRUE(node1.isValid());
	Node node2 = nodes.findNodeByIdentifier(2);
	EXPECT_TRUE(node2.isValid());

	Field name = zinc.fm.findFieldByName("name");
	EXPECT_TRUE(name.isValid());
	Fieldcache fieldcache = zinc.fm.createFieldcache();
	char *s;
	EXPECT_EQ(RESULT_OK, fieldcache.setNode(node1));
	s = name.evaluateString(fieldcache);
	EXPECT_STREQ("", s);
	EXPECT_EQ(RESULT_OK, fieldcache.setNode(node2));
	s = name.evaluateString(fieldcache);
	EXPECT_STREQ("bob", s);
}
