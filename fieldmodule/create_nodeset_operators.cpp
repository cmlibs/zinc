
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

TEST(Cmiss_field_module_create_nodeset_minimum, invalid_args)
{
	Cmiss_context_id context = Cmiss_context_create("test");
	Cmiss_region_id root_region = Cmiss_context_get_default_region(context);
	Cmiss_field_module_id fm = Cmiss_region_get_field_module(root_region);

	EXPECT_NE(static_cast<Cmiss_field_module *>(0), fm);

	Cmiss_field_id f0 = Cmiss_field_module_create_nodeset_minimum(0, 0, 0);
	EXPECT_EQ(0, f0);

	Cmiss_field_id f1 = Cmiss_field_module_create_nodeset_minimum(fm, 0, 0);
	EXPECT_EQ(0, f1);

	Cmiss_nodeset_id ns = Cmiss_field_module_find_nodeset_by_domain_type(fm, CMISS_FIELD_DOMAIN_NODES);
	EXPECT_NE(static_cast<Cmiss_nodeset *>(0), ns);

	Cmiss_field_id f2 = Cmiss_field_module_create_nodeset_minimum(fm, 0, ns);
	EXPECT_EQ(0, f2);

	double values[] = {6.0, 1.0, 2.5};
	Cmiss_field_id f3 = Cmiss_field_module_create_constant(fm, 3, values);

	Cmiss_field_id f4 = Cmiss_field_module_create_nodeset_minimum(fm, f3, ns);
	EXPECT_NE(static_cast<Cmiss_field *>(0), f4);

	Cmiss_field_cache_id fc = Cmiss_field_module_create_cache(fm);

	double outvalues[3];
	int result = Cmiss_field_evaluate_real(f2, fc, 3, outvalues);
	EXPECT_NE(CMISS_OK, result);

	Cmiss_field_destroy(&f0);
	Cmiss_field_destroy(&f1);
	Cmiss_field_destroy(&f2);
	Cmiss_field_destroy(&f3);
	Cmiss_field_destroy(&f4);
	Cmiss_field_cache_destroy(&fc);
	Cmiss_nodeset_destroy(&ns);
	Cmiss_field_module_destroy(&fm);
	Cmiss_region_destroy(&root_region);
	Cmiss_context_destroy(&context);
}

TEST(Cmiss_field_module_create_nodeset_minimum, valid_args)
{
	Cmiss_context_id context = Cmiss_context_create("test");
	Cmiss_region_id root_region = Cmiss_context_get_default_region(context);
	Cmiss_field_module_id fm = Cmiss_region_get_field_module(root_region);

	Cmiss_region_read_file(root_region,
		TestResources::getLocation(TestResources::FIELDMODULE_EXNODE_RESOURCE));

	Cmiss_field_id f1 = Cmiss_field_module_find_field_by_name(fm, "coordinates");

	Cmiss_nodeset_id ns = Cmiss_field_module_find_nodeset_by_domain_type(fm, CMISS_FIELD_DOMAIN_NODES);
	EXPECT_NE(static_cast<Cmiss_nodeset *>(0), ns);

	Cmiss_field_id f2 = Cmiss_field_module_create_nodeset_minimum(fm, f1, ns);
	EXPECT_NE(static_cast<Cmiss_field *>(0), f2);

	Cmiss_field_cache_id fc = Cmiss_field_module_create_cache(fm);

	double outvalues[3];
	int result = Cmiss_field_evaluate_real(f2, fc, 3, outvalues);
	EXPECT_EQ(CMISS_OK, result);
	EXPECT_EQ(-1.0, outvalues[0]);
	EXPECT_EQ(0.0, outvalues[1]);
	EXPECT_EQ(0.0, outvalues[2]);

	Cmiss_field_destroy(&f1);
	Cmiss_field_destroy(&f2);
	Cmiss_field_cache_destroy(&fc);
	Cmiss_nodeset_destroy(&ns);
	Cmiss_field_module_destroy(&fm);
	Cmiss_region_destroy(&root_region);
	Cmiss_context_destroy(&context);
}

TEST(Cmiss_field_module_create_nodeset_maximum, invalid_args)
{
	Cmiss_context_id context = Cmiss_context_create("test");
	Cmiss_region_id root_region = Cmiss_context_get_default_region(context);
	Cmiss_field_module_id fm = Cmiss_region_get_field_module(root_region);

	EXPECT_NE(static_cast<Cmiss_field_module *>(0), fm);

	Cmiss_field_id f0 = Cmiss_field_module_create_nodeset_maximum(0, 0, 0);
	EXPECT_EQ(0, f0);

	Cmiss_field_id f1 = Cmiss_field_module_create_nodeset_maximum(fm, 0, 0);
	EXPECT_EQ(0, f1);

	Cmiss_nodeset_id ns = Cmiss_field_module_find_nodeset_by_domain_type(fm, CMISS_FIELD_DOMAIN_NODES);
	EXPECT_NE(static_cast<Cmiss_nodeset *>(0), ns);

	Cmiss_field_id f2 = Cmiss_field_module_create_nodeset_maximum(fm, 0, ns);
	EXPECT_EQ(0, f2);

	double values[] = {6.0, 1.0, 2.5};
	Cmiss_field_id f3 = Cmiss_field_module_create_constant(fm, 3, values);

	Cmiss_field_id f4 = Cmiss_field_module_create_nodeset_maximum(fm, f3, ns);
	EXPECT_NE(static_cast<Cmiss_field *>(0), f4);

	Cmiss_field_cache_id fc = Cmiss_field_module_create_cache(fm);

	double outvalues[3];
	int result = Cmiss_field_evaluate_real(f2, fc, 3, outvalues);
	EXPECT_NE(CMISS_OK, result);

	Cmiss_field_destroy(&f0);
	Cmiss_field_destroy(&f1);
	Cmiss_field_destroy(&f2);
	Cmiss_field_destroy(&f3);
	Cmiss_field_destroy(&f4);
	Cmiss_field_cache_destroy(&fc);
	Cmiss_nodeset_destroy(&ns);
	Cmiss_field_module_destroy(&fm);
	Cmiss_region_destroy(&root_region);
	Cmiss_context_destroy(&context);
}

TEST(Cmiss_field_module_create_nodeset_maximum, valid_args)
{
	Cmiss_context_id context = Cmiss_context_create("test");
	Cmiss_region_id root_region = Cmiss_context_get_default_region(context);
	Cmiss_field_module_id fm = Cmiss_region_get_field_module(root_region);

	Cmiss_region_read_file(root_region, TestResources::getLocation(TestResources::FIELDMODULE_EXNODE_RESOURCE));

	Cmiss_field_id f1 = Cmiss_field_module_find_field_by_name(fm, "coordinates");

	Cmiss_nodeset_id ns = Cmiss_field_module_find_nodeset_by_domain_type(fm, CMISS_FIELD_DOMAIN_NODES);
	EXPECT_NE(static_cast<Cmiss_nodeset *>(0), ns);

	Cmiss_field_id f2 = Cmiss_field_module_create_nodeset_maximum(fm, f1, ns);
	EXPECT_NE(static_cast<Cmiss_field *>(0), f2);

	Cmiss_field_cache_id fc = Cmiss_field_module_create_cache(fm);

	double outvalues[3];
	int result = Cmiss_field_evaluate_real(f2, fc, 3, outvalues);
	EXPECT_EQ(CMISS_OK, result);
	EXPECT_EQ(2.0, outvalues[0]);
	EXPECT_EQ(2.0, outvalues[1]);
	EXPECT_EQ(1.0, outvalues[2]);

	Cmiss_field_destroy(&f1);
	Cmiss_field_destroy(&f2);
	Cmiss_field_cache_destroy(&fc);
	Cmiss_nodeset_destroy(&ns);
	Cmiss_field_module_destroy(&fm);
	Cmiss_region_destroy(&root_region);
	Cmiss_context_destroy(&context);
};

TEST(Cmiss_field_module_create_nodeset_maximum, multiplecomponents)
{
	ZincTestSetup zinc;
	int result = 0;
	Cmiss_region_read_file(zinc.root_region, TestResources::getLocation(TestResources::FIELDMODULE_EXNODE_RESOURCE));
	Cmiss_field_id f1 = Cmiss_field_module_find_field_by_name(zinc.fm, "coordinates");
	Cmiss_field_id f2 = Cmiss_field_module_create_component(zinc.fm, f1, 1);
	Cmiss_field_id f3 = Cmiss_field_module_create_component(zinc.fm, f1, 2);
	Cmiss_field_id f4 = Cmiss_field_module_create_component(zinc.fm, f1, 3);
	EXPECT_NE(static_cast<Cmiss_field *>(0), f2);
	EXPECT_NE(static_cast<Cmiss_field *>(0), f3);
	EXPECT_NE(static_cast<Cmiss_field *>(0), f4);

	Cmiss_nodeset_id ns = Cmiss_field_module_find_nodeset_by_name(zinc.fm, "cmiss_nodes");
	EXPECT_NE(static_cast<Cmiss_nodeset *>(0), ns);
	Cmiss_field_id f5 = Cmiss_field_module_create_nodeset_maximum(zinc.fm, f2, ns);
	Cmiss_field_id f6 = Cmiss_field_module_create_nodeset_maximum(zinc.fm, f3, ns);
	Cmiss_field_id f7 = Cmiss_field_module_create_nodeset_maximum(zinc.fm, f4, ns);
	EXPECT_NE(static_cast<Cmiss_field *>(0), f5);
	EXPECT_NE(static_cast<Cmiss_field *>(0), f6);
	EXPECT_NE(static_cast<Cmiss_field *>(0), f7);
	Cmiss_field_cache_id fc = Cmiss_field_module_create_cache(zinc.fm);

	double outvalues[1];
	result = Cmiss_field_evaluate_real(f5, fc, 1, outvalues);
	EXPECT_EQ(CMISS_OK, result);
	EXPECT_EQ(2.0, outvalues[0]);
	result = Cmiss_field_evaluate_real(f6, fc, 1, outvalues);
	EXPECT_EQ(CMISS_OK, result);
	EXPECT_EQ(2.0, outvalues[0]);
	result = Cmiss_field_evaluate_real(f7, fc, 1, outvalues);
	EXPECT_EQ(CMISS_OK, result);
	EXPECT_EQ(1.0, outvalues[0]);

	Cmiss_field_cache_destroy(&fc);
	Cmiss_nodeset_destroy(&ns);
	Cmiss_field_destroy(&f1);
	Cmiss_field_destroy(&f2);
	Cmiss_field_destroy(&f3);
	Cmiss_field_destroy(&f4);
	Cmiss_field_destroy(&f5);
	Cmiss_field_destroy(&f6);
	Cmiss_field_destroy(&f7);

}

