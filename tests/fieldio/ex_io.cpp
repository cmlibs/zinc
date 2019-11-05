/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include <opencmiss/zinc/field.hpp>
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
