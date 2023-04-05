/*
 * Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include <cmlibs/zinc/core.h>
#include <cmlibs/zinc/context.h>
#include <cmlibs/zinc/field.h>
#include <cmlibs/zinc/fieldcache.h>
#include <cmlibs/zinc/fieldconstant.h>
#include <cmlibs/zinc/fieldcomposite.h>
#include <cmlibs/zinc/fieldmodule.h>
#include <cmlibs/zinc/fieldnodesetoperators.h>
#include <cmlibs/zinc/nodeset.h>

#include <cmlibs/zinc/fieldconstant.hpp>
#include <cmlibs/zinc/fieldfiniteelement.hpp>
#include <cmlibs/zinc/fieldnodesetoperators.hpp>
#include <cmlibs/zinc/fieldsubobjectgroup.hpp>

#include "test_resources.h"
#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"


TEST(cmzn_fieldmodule_create_field_nodeset_minimum, invalid_args)
{
	cmzn_context_id context = cmzn_context_create("test");
	cmzn_region_id root_region = cmzn_context_get_default_region(context);
	cmzn_fieldmodule_id fm = cmzn_region_get_fieldmodule(root_region);

	EXPECT_NE(static_cast<cmzn_fieldmodule *>(0), fm);

	cmzn_field_id f0 = cmzn_fieldmodule_create_field_nodeset_minimum(0, 0, 0);
	EXPECT_EQ(0, f0);

	cmzn_field_id f1 = cmzn_fieldmodule_create_field_nodeset_minimum(fm, 0, 0);
	EXPECT_EQ(0, f1);

	cmzn_nodeset_id ns = cmzn_fieldmodule_find_nodeset_by_field_domain_type(fm, CMZN_FIELD_DOMAIN_TYPE_NODES);
	EXPECT_NE(static_cast<cmzn_nodeset *>(0), ns);

	cmzn_field_id f2 = cmzn_fieldmodule_create_field_nodeset_minimum(fm, 0, ns);
	EXPECT_EQ(0, f2);

	double values[] = {6.0, 1.0, 2.5};
	cmzn_field_id f3 = cmzn_fieldmodule_create_field_constant(fm, 3, values);

	cmzn_field_id f4 = cmzn_fieldmodule_create_field_nodeset_minimum(fm, f3, ns);
	EXPECT_NE(static_cast<cmzn_field *>(0), f4);

	cmzn_fieldcache_id fc = cmzn_fieldmodule_create_fieldcache(fm);

	double outvalues[3];
	int result = cmzn_field_evaluate_real(f2, fc, 3, outvalues);
	EXPECT_NE(CMZN_OK, result);

	cmzn_field_destroy(&f0);
	cmzn_field_destroy(&f1);
	cmzn_field_destroy(&f2);
	cmzn_field_destroy(&f3);
	cmzn_field_destroy(&f4);
	cmzn_fieldcache_destroy(&fc);
	cmzn_nodeset_destroy(&ns);
	cmzn_fieldmodule_destroy(&fm);
	cmzn_region_destroy(&root_region);
	cmzn_context_destroy(&context);
}

TEST(cmzn_fieldmodule_create_field_nodeset_minimum, valid_args)
{
	cmzn_context_id context = cmzn_context_create("test");
	cmzn_region_id root_region = cmzn_context_get_default_region(context);
	cmzn_fieldmodule_id fm = cmzn_region_get_fieldmodule(root_region);

	cmzn_region_read_file(root_region,
        resourcePath("fieldmodule/nodes.exnode").c_str());

	cmzn_field_id f1 = cmzn_fieldmodule_find_field_by_name(fm, "coordinates");

	cmzn_nodeset_id ns = cmzn_fieldmodule_find_nodeset_by_field_domain_type(fm, CMZN_FIELD_DOMAIN_TYPE_NODES);
	EXPECT_NE(static_cast<cmzn_nodeset *>(0), ns);

	cmzn_field_id f2 = cmzn_fieldmodule_create_field_nodeset_minimum(fm, f1, ns);
	EXPECT_NE(static_cast<cmzn_field *>(0), f2);

	cmzn_fieldcache_id fc = cmzn_fieldmodule_create_fieldcache(fm);

	double outvalues[3];
	int result = cmzn_field_evaluate_real(f2, fc, 3, outvalues);
	EXPECT_EQ(CMZN_OK, result);
	EXPECT_EQ(-1.0, outvalues[0]);
	EXPECT_EQ(0.0, outvalues[1]);
	EXPECT_EQ(0.0, outvalues[2]);

	cmzn_field_destroy(&f1);
	cmzn_field_destroy(&f2);
	cmzn_fieldcache_destroy(&fc);
	cmzn_nodeset_destroy(&ns);
	cmzn_fieldmodule_destroy(&fm);
	cmzn_region_destroy(&root_region);
	cmzn_context_destroy(&context);
}

TEST(cmzn_fieldmodule_create_field_nodeset_maximum, invalid_args)
{
	cmzn_context_id context = cmzn_context_create("test");
	cmzn_region_id root_region = cmzn_context_get_default_region(context);
	cmzn_fieldmodule_id fm = cmzn_region_get_fieldmodule(root_region);

	EXPECT_NE(static_cast<cmzn_fieldmodule *>(0), fm);

	cmzn_field_id f0 = cmzn_fieldmodule_create_field_nodeset_maximum(0, 0, 0);
	EXPECT_EQ(0, f0);

	cmzn_field_id f1 = cmzn_fieldmodule_create_field_nodeset_maximum(fm, 0, 0);
	EXPECT_EQ(0, f1);

	cmzn_nodeset_id ns = cmzn_fieldmodule_find_nodeset_by_field_domain_type(fm, CMZN_FIELD_DOMAIN_TYPE_NODES);
	EXPECT_NE(static_cast<cmzn_nodeset *>(0), ns);

	cmzn_field_id f2 = cmzn_fieldmodule_create_field_nodeset_maximum(fm, 0, ns);
	EXPECT_EQ(0, f2);

	double values[] = {6.0, 1.0, 2.5};
	cmzn_field_id f3 = cmzn_fieldmodule_create_field_constant(fm, 3, values);

	cmzn_field_id f4 = cmzn_fieldmodule_create_field_nodeset_maximum(fm, f3, ns);
	EXPECT_NE(static_cast<cmzn_field *>(0), f4);

	cmzn_fieldcache_id fc = cmzn_fieldmodule_create_fieldcache(fm);

	double outvalues[3];
	int result = cmzn_field_evaluate_real(f2, fc, 3, outvalues);
	EXPECT_NE(CMZN_OK, result);

	cmzn_field_destroy(&f0);
	cmzn_field_destroy(&f1);
	cmzn_field_destroy(&f2);
	cmzn_field_destroy(&f3);
	cmzn_field_destroy(&f4);
	cmzn_fieldcache_destroy(&fc);
	cmzn_nodeset_destroy(&ns);
	cmzn_fieldmodule_destroy(&fm);
	cmzn_region_destroy(&root_region);
	cmzn_context_destroy(&context);
}

TEST(cmzn_fieldmodule_create_field_nodeset_maximum, valid_args)
{
	cmzn_context_id context = cmzn_context_create("test");
	cmzn_region_id root_region = cmzn_context_get_default_region(context);
	cmzn_fieldmodule_id fm = cmzn_region_get_fieldmodule(root_region);

    cmzn_region_read_file(root_region, resourcePath("fieldmodule/nodes.exnode").c_str());

	cmzn_field_id f1 = cmzn_fieldmodule_find_field_by_name(fm, "coordinates");

	cmzn_nodeset_id ns = cmzn_fieldmodule_find_nodeset_by_field_domain_type(fm, CMZN_FIELD_DOMAIN_TYPE_NODES);
	EXPECT_NE(static_cast<cmzn_nodeset *>(0), ns);

	cmzn_field_id f2 = cmzn_fieldmodule_create_field_nodeset_maximum(fm, f1, ns);
	EXPECT_NE(static_cast<cmzn_field *>(0), f2);

	cmzn_fieldcache_id fc = cmzn_fieldmodule_create_fieldcache(fm);

	double outvalues[3];
	int result = cmzn_field_evaluate_real(f2, fc, 3, outvalues);
	EXPECT_EQ(CMZN_OK, result);
	EXPECT_EQ(2.0, outvalues[0]);
	EXPECT_EQ(2.0, outvalues[1]);
	EXPECT_EQ(1.0, outvalues[2]);

	cmzn_field_destroy(&f1);
	cmzn_field_destroy(&f2);
	cmzn_fieldcache_destroy(&fc);
	cmzn_nodeset_destroy(&ns);
	cmzn_fieldmodule_destroy(&fm);
	cmzn_region_destroy(&root_region);
	cmzn_context_destroy(&context);
};

TEST(cmzn_fieldmodule_create_field_nodeset_maximum, multiplecomponents)
{
	ZincTestSetup zinc;
	int result = 0;
    cmzn_region_read_file(zinc.root_region, resourcePath("fieldmodule/nodes.exnode").c_str());
	cmzn_field_id f1 = cmzn_fieldmodule_find_field_by_name(zinc.fm, "coordinates");
	cmzn_field_id f2 = cmzn_fieldmodule_create_field_component(zinc.fm, f1, 1);
	cmzn_field_id f3 = cmzn_fieldmodule_create_field_component(zinc.fm, f1, 2);
	cmzn_field_id f4 = cmzn_fieldmodule_create_field_component(zinc.fm, f1, 3);
	EXPECT_NE(static_cast<cmzn_field *>(0), f2);
	EXPECT_NE(static_cast<cmzn_field *>(0), f3);
	EXPECT_NE(static_cast<cmzn_field *>(0), f4);

	cmzn_nodeset_id ns = cmzn_fieldmodule_find_nodeset_by_name(zinc.fm, "nodes");
	EXPECT_NE(static_cast<cmzn_nodeset *>(0), ns);
	cmzn_field_id f5 = cmzn_fieldmodule_create_field_nodeset_maximum(zinc.fm, f2, ns);
	cmzn_field_id f6 = cmzn_fieldmodule_create_field_nodeset_maximum(zinc.fm, f3, ns);
	cmzn_field_id f7 = cmzn_fieldmodule_create_field_nodeset_maximum(zinc.fm, f4, ns);
	EXPECT_NE(static_cast<cmzn_field *>(0), f5);
	EXPECT_NE(static_cast<cmzn_field *>(0), f6);
	EXPECT_NE(static_cast<cmzn_field *>(0), f7);
	cmzn_fieldcache_id fc = cmzn_fieldmodule_create_fieldcache(zinc.fm);

	double outvalues[1];
	result = cmzn_field_evaluate_real(f5, fc, 1, outvalues);
	EXPECT_EQ(CMZN_OK, result);
	EXPECT_EQ(2.0, outvalues[0]);
	result = cmzn_field_evaluate_real(f6, fc, 1, outvalues);
	EXPECT_EQ(CMZN_OK, result);
	EXPECT_EQ(2.0, outvalues[0]);
	result = cmzn_field_evaluate_real(f7, fc, 1, outvalues);
	EXPECT_EQ(CMZN_OK, result);
	EXPECT_EQ(1.0, outvalues[0]);

	cmzn_fieldcache_destroy(&fc);
	cmzn_nodeset_destroy(&ns);
	cmzn_field_destroy(&f1);
	cmzn_field_destroy(&f2);
	cmzn_field_destroy(&f3);
	cmzn_field_destroy(&f4);
	cmzn_field_destroy(&f5);
	cmzn_field_destroy(&f6);
	cmzn_field_destroy(&f7);
}

TEST(NodesetOperators, args)
{
	ZincTestSetupCpp zinc;

	const double one = 1.0;
	FieldConstant constant = zinc.fm.createFieldConstant(1, &one);
	EXPECT_TRUE(constant.isValid());
	FieldStringConstant stringConstant = zinc.fm.createFieldStringConstant("string");
	EXPECT_TRUE(stringConstant.isValid());
	Nodeset nodeset = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_TRUE(nodeset.isValid());
	Mesh mesh = zinc.fm.findMeshByDimension(3);
	EXPECT_TRUE(mesh.isValid());
	FieldStoredMeshLocation storedMeshLocation = zinc.fm.createFieldStoredMeshLocation(mesh);
	EXPECT_TRUE(storedMeshLocation.isValid());
	FieldNodeGroup nodeGroup = zinc.fm.createFieldNodeGroup(nodeset);
	EXPECT_TRUE(nodeGroup.isValid());
	NodesetGroup nodesetGroup = nodeGroup.getNodesetGroup();
	EXPECT_TRUE(nodesetGroup.isValid());

	Field noField;
	Nodeset noNodeset;
	FieldNodesetSum nodesetSum;
	nodesetSum = zinc.fm.createFieldNodesetSum(stringConstant, nodeset);
	EXPECT_FALSE(nodesetSum.isValid());
	nodesetSum = zinc.fm.createFieldNodesetSum(constant, noNodeset);
	EXPECT_FALSE(nodesetSum.isValid());
	nodesetSum = zinc.fm.createFieldNodesetSum(constant, nodeset);
	EXPECT_TRUE(nodesetSum.isValid());
	EXPECT_FALSE(nodesetSum.getElementMapField().isValid());
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, nodesetSum.setElementMapField(constant));
	EXPECT_EQ(RESULT_OK, nodesetSum.setElementMapField(storedMeshLocation));
	EXPECT_EQ(storedMeshLocation, nodesetSum.getElementMapField());

	EXPECT_EQ(nodeset, nodesetSum.getNodeset());
	EXPECT_EQ(RESULT_ERROR_ARGUMENT, nodesetSum.setNodeset(noNodeset));
	EXPECT_EQ(RESULT_OK, nodesetSum.setNodeset(nodesetGroup));
	EXPECT_EQ(nodesetGroup, nodesetSum.getNodeset());

	Field field1 = nodesetSum;
	FieldNodesetOperator tmpNodesetSum = field1.castNodesetOperator();
	EXPECT_EQ(nodesetSum, tmpNodesetSum);
	EXPECT_EQ(storedMeshLocation, tmpNodesetSum.getElementMapField());

	EXPECT_EQ(RESULT_OK, nodesetSum.setElementMapField(noField));
	EXPECT_FALSE(nodesetSum.getElementMapField().isValid());
}

// Test evaluation of all nodeset operators with combinations of embedded nodes and groups
TEST(NodesetOperators, ElementGroupEvaluation)
{
	ZincTestSetupCpp zinc;

	// a handy model with nodes 1-4 in the corners of a square and nodes 5-8 with host locations
    EXPECT_EQ(RESULT_OK, zinc.root_region.readFile(resourcePath("fieldmodule/embedding_issue3614.exregion").c_str()));

	Field coordinates = zinc.fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinates.isValid());
	Field hostLocation = zinc.fm.findFieldByName("host_location");
	EXPECT_TRUE(hostLocation.isValid());
	EXPECT_EQ(Field::VALUE_TYPE_MESH_LOCATION, hostLocation.getValueType());
	Field cmissNumber = zinc.fm.findFieldByName("cmiss_number");
	EXPECT_TRUE(cmissNumber.isValid());

	Mesh mesh = zinc.fm.findMeshByDimension(2);
	EXPECT_TRUE(mesh.isValid());
	Element element = mesh.findElementByIdentifier(1);
	EXPECT_TRUE(element.isValid());
	Nodeset nodeset = zinc.fm.findNodesetByFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_TRUE(nodeset.isValid());
	// make a node group containing the odd numbered nodes
	FieldNodeGroup nodeGroup = zinc.fm.createFieldNodeGroup(nodeset);
	NodesetGroup nodesetGroup = nodeGroup.getNodesetGroup();
	EXPECT_TRUE(nodesetGroup.isValid());
	for (int i = 1; i <= 7; i += 2)
		EXPECT_EQ(RESULT_OK, nodesetGroup.addNode(nodeset.findNodeByIdentifier(i)));

	FieldNodesetOperator nodesetOperators[6] =
	{
		zinc.fm.createFieldNodesetSum(cmissNumber, nodeset),
		zinc.fm.createFieldNodesetMean(cmissNumber, nodeset),
		zinc.fm.createFieldNodesetSumSquares(cmissNumber, nodeset),
		zinc.fm.createFieldNodesetMeanSquares(cmissNumber, nodeset),
		zinc.fm.createFieldNodesetMinimum(cmissNumber, nodeset),
		zinc.fm.createFieldNodesetMaximum(cmissNumber, nodeset)
	};
	Fieldcache fieldcache = zinc.fm.createFieldcache();
	EXPECT_TRUE(fieldcache.isValid());
	EXPECT_EQ(RESULT_OK, fieldcache.setElement(element));
	Field noField;
	const double TOL = 1.0E-7;
	const double expectedValues1[6][4] =
	{
		// all, group, element, element & group
		{  36.0,  16.0,  26.0,  12.0 },  // Sum
		{   4.5,   4.0,   6.5,   6.0 },  // Mean
		{ 204.0,  84.0, 174.0,  74.0 },  // SumSquares
		{  25.5,  21.0,  43.5,  37.0 },  // MeanSquares
		{   1.0,   1.0,   5.0,   5.0 },  // Minimum
		{   8.0,   7.0,   8.0,   7.0 }   // Maximum
	};
	double value;
	int result;
	for (int f = 0; f < 6; ++f)
	{
		for (int j = 0; j < 4; ++j)
		{
			//std::cerr << "f " << f << "j" << j << "\n";
			EXPECT_EQ(RESULT_OK, nodesetOperators[f].setNodeset((j % 2) ? nodesetGroup : nodeset));
			EXPECT_EQ(RESULT_OK, nodesetOperators[f].setElementMapField((j >= 2) ? hostLocation : noField));
			result = nodesetOperators[f].evaluateReal(fieldcache, 1, &value);
			EXPECT_EQ(RESULT_OK, result);
			EXPECT_NEAR(expectedValues1[f][j], value, TOL);
		}
	}

	// destroy node 5 and remove node 7 from group and check new answers
	EXPECT_EQ(RESULT_OK, nodeset.destroyNode(nodeset.findNodeByIdentifier(5)));
	EXPECT_EQ(RESULT_OK, nodesetGroup.removeNode(nodeset.findNodeByIdentifier(7)));

	// expectedValue -1.0 used where no result expected
	const double expectedValues2[6][4] =
	{
		// all, group, element, element & group
		{  31.0,      4.0,      21.0,  0.0 },  // Sum
		{  31.0/7.0,  2.0,       7.0, -1.0 },  // Mean
		{     179.0, 10.0,     149.0,  0.0 },  // SumSquares
		{ 179.0/7.0,  5.0, 149.0/3.0, -1.0 },  // MeanSquares
		{       1.0,  1.0,       6.0, -1.0 },  // Minimum
		{       8.0,  3.0,       8.0, -1.0 }   // Maximum
	};
	for (int f = 0; f < 6; ++f)
	{
		for (int j = 0; j < 4; ++j)
		{
			//std::cerr << "f " << f << "j" << j << "\n";
			EXPECT_EQ(RESULT_OK, nodesetOperators[f].setNodeset((j % 2) ? nodesetGroup : nodeset));
			EXPECT_EQ(RESULT_OK, nodesetOperators[f].setElementMapField((j >= 2) ? hostLocation : noField));
			result = nodesetOperators[f].evaluateReal(fieldcache, 1, &value);
			if (expectedValues2[f][j] < 0.0)
			{
				EXPECT_EQ(RESULT_ERROR_GENERAL, result);
			}
			else
			{
				EXPECT_EQ(RESULT_OK, result);
				EXPECT_NEAR(expectedValues2[f][j], value, TOL);
			}
		}
	}
}
