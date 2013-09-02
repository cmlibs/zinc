
#include <gtest/gtest.h>

#include <zinc/zincconfigure.h>
#include <zinc/status.h>
#include <zinc/core.h>
#include <zinc/context.h>
#include <zinc/region.h>
#include <zinc/fieldmodule.h>
#include <zinc/field.h>
#include <zinc/fieldnodesetoperators.h>
#include <zinc/fieldconstant.h>
#include <zinc/fieldcomposite.h>
#include <zinc/node.h>

#include "test_resources.h"
#include "zinctestsetup.hpp"

TEST(cmzn_field_module_create_nodeset_minimum, invalid_args)
{
	cmzn_context_id context = cmzn_context_create("test");
	cmzn_region_id root_region = cmzn_context_get_default_region(context);
	cmzn_field_module_id fm = cmzn_region_get_field_module(root_region);

	EXPECT_NE(static_cast<cmzn_field_module *>(0), fm);

	cmzn_field_id f0 = cmzn_field_module_create_nodeset_minimum(0, 0, 0);
	EXPECT_EQ(0, f0);

	cmzn_field_id f1 = cmzn_field_module_create_nodeset_minimum(fm, 0, 0);
	EXPECT_EQ(0, f1);

	cmzn_nodeset_id ns = cmzn_field_module_find_nodeset_by_domain_type(fm, CMZN_FIELD_DOMAIN_NODES);
	EXPECT_NE(static_cast<cmzn_nodeset *>(0), ns);

	cmzn_field_id f2 = cmzn_field_module_create_nodeset_minimum(fm, 0, ns);
	EXPECT_EQ(0, f2);

	double values[] = {6.0, 1.0, 2.5};
	cmzn_field_id f3 = cmzn_field_module_create_constant(fm, 3, values);

	cmzn_field_id f4 = cmzn_field_module_create_nodeset_minimum(fm, f3, ns);
	EXPECT_NE(static_cast<cmzn_field *>(0), f4);

	cmzn_field_cache_id fc = cmzn_field_module_create_cache(fm);

	double outvalues[3];
	int result = cmzn_field_evaluate_real(f2, fc, 3, outvalues);
	EXPECT_NE(CMZN_OK, result);

	cmzn_field_destroy(&f0);
	cmzn_field_destroy(&f1);
	cmzn_field_destroy(&f2);
	cmzn_field_destroy(&f3);
	cmzn_field_destroy(&f4);
	cmzn_field_cache_destroy(&fc);
	cmzn_nodeset_destroy(&ns);
	cmzn_field_module_destroy(&fm);
	cmzn_region_destroy(&root_region);
	cmzn_context_destroy(&context);
}

TEST(cmzn_field_module_create_nodeset_minimum, valid_args)
{
	cmzn_context_id context = cmzn_context_create("test");
	cmzn_region_id root_region = cmzn_context_get_default_region(context);
	cmzn_field_module_id fm = cmzn_region_get_field_module(root_region);

	cmzn_region_read_file(root_region,
		TestResources::getLocation(TestResources::FIELDMODULE_EXNODE_RESOURCE));

	cmzn_field_id f1 = cmzn_field_module_find_field_by_name(fm, "coordinates");

	cmzn_nodeset_id ns = cmzn_field_module_find_nodeset_by_domain_type(fm, CMZN_FIELD_DOMAIN_NODES);
	EXPECT_NE(static_cast<cmzn_nodeset *>(0), ns);

	cmzn_field_id f2 = cmzn_field_module_create_nodeset_minimum(fm, f1, ns);
	EXPECT_NE(static_cast<cmzn_field *>(0), f2);

	cmzn_field_cache_id fc = cmzn_field_module_create_cache(fm);

	double outvalues[3];
	int result = cmzn_field_evaluate_real(f2, fc, 3, outvalues);
	EXPECT_EQ(CMZN_OK, result);
	EXPECT_EQ(-1.0, outvalues[0]);
	EXPECT_EQ(0.0, outvalues[1]);
	EXPECT_EQ(0.0, outvalues[2]);

	cmzn_field_destroy(&f1);
	cmzn_field_destroy(&f2);
	cmzn_field_cache_destroy(&fc);
	cmzn_nodeset_destroy(&ns);
	cmzn_field_module_destroy(&fm);
	cmzn_region_destroy(&root_region);
	cmzn_context_destroy(&context);
}

TEST(cmzn_field_module_create_nodeset_maximum, invalid_args)
{
	cmzn_context_id context = cmzn_context_create("test");
	cmzn_region_id root_region = cmzn_context_get_default_region(context);
	cmzn_field_module_id fm = cmzn_region_get_field_module(root_region);

	EXPECT_NE(static_cast<cmzn_field_module *>(0), fm);

	cmzn_field_id f0 = cmzn_field_module_create_nodeset_maximum(0, 0, 0);
	EXPECT_EQ(0, f0);

	cmzn_field_id f1 = cmzn_field_module_create_nodeset_maximum(fm, 0, 0);
	EXPECT_EQ(0, f1);

	cmzn_nodeset_id ns = cmzn_field_module_find_nodeset_by_domain_type(fm, CMZN_FIELD_DOMAIN_NODES);
	EXPECT_NE(static_cast<cmzn_nodeset *>(0), ns);

	cmzn_field_id f2 = cmzn_field_module_create_nodeset_maximum(fm, 0, ns);
	EXPECT_EQ(0, f2);

	double values[] = {6.0, 1.0, 2.5};
	cmzn_field_id f3 = cmzn_field_module_create_constant(fm, 3, values);

	cmzn_field_id f4 = cmzn_field_module_create_nodeset_maximum(fm, f3, ns);
	EXPECT_NE(static_cast<cmzn_field *>(0), f4);

	cmzn_field_cache_id fc = cmzn_field_module_create_cache(fm);

	double outvalues[3];
	int result = cmzn_field_evaluate_real(f2, fc, 3, outvalues);
	EXPECT_NE(CMZN_OK, result);

	cmzn_field_destroy(&f0);
	cmzn_field_destroy(&f1);
	cmzn_field_destroy(&f2);
	cmzn_field_destroy(&f3);
	cmzn_field_destroy(&f4);
	cmzn_field_cache_destroy(&fc);
	cmzn_nodeset_destroy(&ns);
	cmzn_field_module_destroy(&fm);
	cmzn_region_destroy(&root_region);
	cmzn_context_destroy(&context);
}

TEST(cmzn_field_module_create_nodeset_maximum, valid_args)
{
	cmzn_context_id context = cmzn_context_create("test");
	cmzn_region_id root_region = cmzn_context_get_default_region(context);
	cmzn_field_module_id fm = cmzn_region_get_field_module(root_region);

	cmzn_region_read_file(root_region, TestResources::getLocation(TestResources::FIELDMODULE_EXNODE_RESOURCE));

	cmzn_field_id f1 = cmzn_field_module_find_field_by_name(fm, "coordinates");

	cmzn_nodeset_id ns = cmzn_field_module_find_nodeset_by_domain_type(fm, CMZN_FIELD_DOMAIN_NODES);
	EXPECT_NE(static_cast<cmzn_nodeset *>(0), ns);

	cmzn_field_id f2 = cmzn_field_module_create_nodeset_maximum(fm, f1, ns);
	EXPECT_NE(static_cast<cmzn_field *>(0), f2);

	cmzn_field_cache_id fc = cmzn_field_module_create_cache(fm);

	double outvalues[3];
	int result = cmzn_field_evaluate_real(f2, fc, 3, outvalues);
	EXPECT_EQ(CMZN_OK, result);
	EXPECT_EQ(2.0, outvalues[0]);
	EXPECT_EQ(2.0, outvalues[1]);
	EXPECT_EQ(1.0, outvalues[2]);

	cmzn_field_destroy(&f1);
	cmzn_field_destroy(&f2);
	cmzn_field_cache_destroy(&fc);
	cmzn_nodeset_destroy(&ns);
	cmzn_field_module_destroy(&fm);
	cmzn_region_destroy(&root_region);
	cmzn_context_destroy(&context);
};

TEST(cmzn_field_module_create_nodeset_maximum, multiplecomponents)
{
	ZincTestSetup zinc;
	int result = 0;
	cmzn_region_read_file(zinc.root_region, TestResources::getLocation(TestResources::FIELDMODULE_EXNODE_RESOURCE));
	cmzn_field_id f1 = cmzn_field_module_find_field_by_name(zinc.fm, "coordinates");
	cmzn_field_id f2 = cmzn_field_module_create_component(zinc.fm, f1, 1);
	cmzn_field_id f3 = cmzn_field_module_create_component(zinc.fm, f1, 2);
	cmzn_field_id f4 = cmzn_field_module_create_component(zinc.fm, f1, 3);
	EXPECT_NE(static_cast<cmzn_field *>(0), f2);
	EXPECT_NE(static_cast<cmzn_field *>(0), f3);
	EXPECT_NE(static_cast<cmzn_field *>(0), f4);

	cmzn_nodeset_id ns = cmzn_field_module_find_nodeset_by_name(zinc.fm, "nodes");
	EXPECT_NE(static_cast<cmzn_nodeset *>(0), ns);
	cmzn_field_id f5 = cmzn_field_module_create_nodeset_maximum(zinc.fm, f2, ns);
	cmzn_field_id f6 = cmzn_field_module_create_nodeset_maximum(zinc.fm, f3, ns);
	cmzn_field_id f7 = cmzn_field_module_create_nodeset_maximum(zinc.fm, f4, ns);
	EXPECT_NE(static_cast<cmzn_field *>(0), f5);
	EXPECT_NE(static_cast<cmzn_field *>(0), f6);
	EXPECT_NE(static_cast<cmzn_field *>(0), f7);
	cmzn_field_cache_id fc = cmzn_field_module_create_cache(zinc.fm);

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

	cmzn_field_cache_destroy(&fc);
	cmzn_nodeset_destroy(&ns);
	cmzn_field_destroy(&f1);
	cmzn_field_destroy(&f2);
	cmzn_field_destroy(&f3);
	cmzn_field_destroy(&f4);
	cmzn_field_destroy(&f5);
	cmzn_field_destroy(&f6);
	cmzn_field_destroy(&f7);

}

