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
#include <opencmiss/zinc/fieldfiniteelement.hpp>
#include <opencmiss/zinc/streamregion.hpp>
#include <opencmiss/zinc/node.hpp>

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

// Test reading EX V3 file with a time sequence at nodes
TEST(FieldIO, ex3NodeTimesequence)
{
	ZincTestSetupCpp zinc;

	ASSERT_EQ(RESULT_OK, zinc.root_region.readFile(TestResources::getLocation(TestResources::FIELDIO_EX3_NODE_TIMESEQUENCE_RESOURCE)));

	Nodeset nodes = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_EQ(2, nodes.getSize());
	Node node1 = nodes.findNodeByIdentifier(1);
	EXPECT_TRUE(node1.isValid());

	const double TOL = 1.0E-7;

	FieldFiniteElement coordinates = zinc.fm.findFieldByName("coordinates").castFiniteElement();
	EXPECT_TRUE(coordinates.isValid());
	Nodetemplate nodetemplate = nodes.createNodetemplate();
	EXPECT_TRUE(nodetemplate.isValid());
	EXPECT_EQ(RESULT_OK, nodetemplate.defineFieldFromNode(coordinates, node1));
	Timesequence times1 = nodetemplate.getTimesequence(coordinates);
	EXPECT_EQ(6, times1.getNumberOfTimes());
	const double expectedx[6][3] = {
		{ 0.0, 0.0, 0.0 },
		{ 1.0, 0.1, 0.1 },
		{ 2.1, 0.2, 0.05 },
		{ 3.3, 0.3, 0.025 },
		{ 4.6, 0.4, 0.0125 },
		{ 6.0, 0.5, 0.00625 }
	};
	const double expecteddx[6] = { 6.0, 3.6, 1.2, 1.3, 1.4, 1.5 };
	double x[3], dx;
	Fieldcache fieldcache = zinc.fm.createFieldcache();
	fieldcache.setNode(node1);
	for (int t = 0; t < 6; ++t)
	{
		const double timeValue = static_cast<double>(t);
		EXPECT_NEAR(timeValue, times1.getTime(t + 1), TOL);
		EXPECT_EQ(RESULT_OK, fieldcache.setTime(timeValue));
		EXPECT_EQ(RESULT_OK, coordinates.getNodeParameters(fieldcache, -1, Node::VALUE_LABEL_VALUE, 1, 3, x));
		for (int c = 0; c < 3; ++c)
			EXPECT_NEAR(expectedx[t][c], x[c], TOL);
		EXPECT_EQ(RESULT_OK, coordinates.getNodeParameters(fieldcache, 1, Node::VALUE_LABEL_D_DS1, 1, 1, &dx));
		EXPECT_NEAR(expecteddx[t], dx, TOL);
	}

	Mesh mesh1d = zinc.fm.findMeshByDimension(1);
	EXPECT_EQ(1, mesh1d.getSize());
	Element element1 = mesh1d.findElementByIdentifier(1);
	EXPECT_TRUE(element1.isValid());
	const double xi = 0.4;
	const double phi1 = 0.5*(1.0 - xi);
	const double phi2 = 0.5*xi;
	const double expectedElementx[3] = {
		phi1*(expectedx[1][0] + expectedx[2][0]) + phi2*(expectedx[3][0] + expectedx[4][0]),
		phi1*(expectedx[1][1] + expectedx[2][1]) + phi2*(expectedx[3][1] + expectedx[4][1]),
		phi1*(expectedx[1][2] + expectedx[2][2]) + phi2*(expectedx[3][2] + expectedx[4][2])
	};
	EXPECT_EQ(RESULT_OK, fieldcache.setMeshLocation(element1, 1, &xi));
	EXPECT_EQ(RESULT_OK, fieldcache.setTime(1.5));
	EXPECT_EQ(RESULT_OK, coordinates.evaluateReal(fieldcache, 3, x));
	for (int c = 0; c < 3; ++c)
		EXPECT_NEAR(expectedElementx[c], x[c], TOL);

	// test time-varying integer field
	// note all numerical types are treated as real in the field interface
	Field count = zinc.fm.findFieldByName("count");  // note integer value type field can't be cast to finite element
	EXPECT_TRUE(count.isValid());
	EXPECT_EQ(Field::VALUE_TYPE_REAL, count.getValueType());
	EXPECT_EQ(RESULT_OK, nodetemplate.defineFieldFromNode(count, node1));
	Timesequence times2 = nodetemplate.getTimesequence(count);
	EXPECT_EQ(3, times2.getNumberOfTimes());
	const double expectedTimes2[3] = { 0.0, 1.5, 3.7 };
	const double expectedCounts[3] = { 1.0, 3.0, 5.0 };
	double countValue;
	fieldcache.setNode(node1);
	for (int t = 0; t < 3; ++t)
	{
		const double timeValue = times2.getTime(t + 1);
		EXPECT_NEAR(expectedTimes2[t], timeValue, TOL);
		EXPECT_EQ(RESULT_OK, fieldcache.setTime(timeValue));
		EXPECT_EQ(RESULT_OK, count.evaluateReal(fieldcache, 1, &countValue));
		EXPECT_NEAR(expectedCounts[t], countValue, TOL);
	}
}

// Test reading a single time from EX V3 file with a time sequence at nodes
TEST(FieldIO, ex3NodeTimesequenceSingleTime)
{
	ZincTestSetupCpp zinc;

	StreaminformationRegion sir = zinc.root_region.createStreaminformationRegion();
	StreamresourceFile srf = sir.createStreamresourceFile(TestResources::getLocation(TestResources::FIELDIO_EX3_NODE_TIMESEQUENCE_RESOURCE));
	EXPECT_TRUE(srf.isValid());
	ASSERT_EQ(RESULT_OK, sir.setResourceAttributeReal(srf, StreaminformationRegion::ATTRIBUTE_TIME, 1.8));
	EXPECT_EQ(RESULT_OK, zinc.root_region.read(sir));

	Nodeset nodes = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_EQ(2, nodes.getSize());
	Node node1 = nodes.findNodeByIdentifier(1);
	EXPECT_TRUE(node1.isValid());

	const double TOL = 1.0E-7;
	FieldFiniteElement coordinates = zinc.fm.findFieldByName("coordinates").castFiniteElement();
	EXPECT_TRUE(coordinates.isValid());
	Nodetemplate nodetemplate = nodes.createNodetemplate();
	EXPECT_TRUE(nodetemplate.isValid());
	EXPECT_EQ(RESULT_OK, nodetemplate.defineFieldFromNode(coordinates, node1));
	Timesequence times1 = nodetemplate.getTimesequence(coordinates);
	EXPECT_EQ(1, times1.getNumberOfTimes());
	EXPECT_NEAR(2.0, times1.getTime(1), TOL);
	Fieldcache fieldcache = zinc.fm.createFieldcache();
	fieldcache.setNode(node1);
	EXPECT_EQ(RESULT_OK, fieldcache.setTime(1.8));  // will use nearest time when evaluating
	const double expectedx[3] = { 2.1, 0.2, 0.05 };
	double x[3], dx;
	EXPECT_EQ(RESULT_OK, coordinates.getNodeParameters(fieldcache, -1, Node::VALUE_LABEL_VALUE, 1, 3, x));
	for (int c = 0; c < 3; ++c)
		EXPECT_NEAR(expectedx[c], x[c], TOL);
	EXPECT_EQ(RESULT_OK, coordinates.getNodeParameters(fieldcache, 1, Node::VALUE_LABEL_D_DS1, 1, 1, &dx));
	EXPECT_NEAR(1.2, dx, TOL);

	// test time-varying integer field
	// note all numerical types are treated as real in the field interface
	Field count = zinc.fm.findFieldByName("count");  // note integer value type field can't be cast to finite element
	EXPECT_TRUE(count.isValid());
	EXPECT_EQ(Field::VALUE_TYPE_REAL, count.getValueType());
	EXPECT_EQ(RESULT_OK, nodetemplate.defineFieldFromNode(count, node1));
	Timesequence times2 = nodetemplate.getTimesequence(count);
	EXPECT_EQ(1, times2.getNumberOfTimes());
	EXPECT_NEAR(1.5, times2.getTime(1), TOL);
	double countValue;
	EXPECT_EQ(RESULT_OK, count.evaluateReal(fieldcache, 1, &countValue));
	EXPECT_NEAR(3.0, countValue, TOL);
}
