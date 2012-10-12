/*
 * UnitTestImageBrowser.cpp
 *
 *  Created on: Feb 26, 2011
 *      Author: jchu014
 */

#include <gtest/gtest.h>

extern "C"
{
#include <zn/cmgui_configure.h>
#include <zn/cmiss_core.h>
#include <zn/cmiss_context.h>
#include <zn/cmiss_region.h>
#include <zn/cmiss_field_module.h>
#include <zn/cmiss_field.h>
#include <zn/cmiss_field_conditional.h>
#include <zn/cmiss_field_constant.h>
}

TEST(Cmiss_field_module_create_if, invalid_args)
{
	Cmiss_context_id context = Cmiss_context_create("test");
	Cmiss_region_id root_region = Cmiss_context_get_default_region(context);
	Cmiss_field_module_id fm = Cmiss_region_get_field_module(root_region);

	EXPECT_NE(static_cast<Cmiss_field_module *>(0), fm);

	Cmiss_field_id f0 = Cmiss_field_module_create_if(0, 0, 0, 0);
	EXPECT_EQ(0, f0);

	Cmiss_field_id f1 = Cmiss_field_module_create_if(fm, 0, 0, 0);
	EXPECT_EQ(0, f1);

	double values[] = {3.0, 2.0, 1.0};
	Cmiss_field_id f2 = Cmiss_field_module_create_constant(fm, 3, values);
	Cmiss_field_id f3 = Cmiss_field_module_create_constant(fm, 3, values);

	Cmiss_field_id f4 = Cmiss_field_module_create_if(fm, 0, f2, f3);
	EXPECT_EQ(0, f4);

	Cmiss_field_destroy(&f0);
	Cmiss_field_destroy(&f1);
	Cmiss_field_destroy(&f2);
	Cmiss_field_destroy(&f3);
	Cmiss_field_destroy(&f4);
	Cmiss_field_module_destroy(&fm);
	Cmiss_region_destroy(&root_region);
	Cmiss_context_destroy(&context);
}

TEST(Cmiss_field_module_create_if, valid_args)
{
	Cmiss_context_id context = Cmiss_context_create("test");
	Cmiss_region_id root_region = Cmiss_context_get_default_region(context);
	Cmiss_field_module_id fm = Cmiss_region_get_field_module(root_region);

	double values[] = {3.0, 2.0, 1.0};
	Cmiss_field_id f1 = Cmiss_field_module_create_constant(fm, 3, values);
	Cmiss_field_id f2 = Cmiss_field_module_create_constant(fm, 3, values);
	Cmiss_field_id f3 = Cmiss_field_module_create_constant(fm, 3, values);

	Cmiss_field_id f4 = Cmiss_field_module_create_if(fm, f1, f2, f3);
	EXPECT_NE(static_cast<Cmiss_field *>(0), f4);

	Cmiss_field_destroy(&f1);
	Cmiss_field_destroy(&f2);
	Cmiss_field_destroy(&f3);
	Cmiss_field_destroy(&f4);
	Cmiss_field_module_destroy(&fm);
	Cmiss_region_destroy(&root_region);
	Cmiss_context_destroy(&context);
}

TEST(Cmiss_field_module_create_if, single_component_switch)
{
	Cmiss_context_id context = Cmiss_context_create("test");
	Cmiss_region_id root_region = Cmiss_context_get_default_region(context);
	Cmiss_field_module_id fm = Cmiss_region_get_field_module(root_region);

	double onevalue[] = {5.0};
	double values1[] = {3.0, 2.0, 1.0};
	double values2[] = {6.0, 5.0, 4.0};
	Cmiss_field_id f1 = Cmiss_field_module_create_constant(fm, 1, onevalue);
	Cmiss_field_id f2 = Cmiss_field_module_create_constant(fm, 3, values1);
	Cmiss_field_id f3 = Cmiss_field_module_create_constant(fm, 3, values2);
	Cmiss_field_id f4 = Cmiss_field_module_create_if(fm, f1, f2, f3);
	EXPECT_NE(static_cast<Cmiss_field *>(0), f4);

	Cmiss_field_cache_id fc = Cmiss_field_module_create_cache(fm);

	double outvalues[3];
	Cmiss_field_evaluate_real(f4, fc, 3, outvalues);
	EXPECT_EQ(3.0, outvalues[0]);
	EXPECT_EQ(2.0, outvalues[1]);
	EXPECT_EQ(1.0, outvalues[2]);

	Cmiss_field_destroy(&f1);
	Cmiss_field_destroy(&f2);
	Cmiss_field_destroy(&f3);
	Cmiss_field_destroy(&f4);
	Cmiss_field_cache_destroy(&fc);
	Cmiss_field_module_destroy(&fm);
	Cmiss_region_destroy(&root_region);
	Cmiss_context_destroy(&context);
}

TEST(Cmiss_field_module_create_if, string_constant)
{
	Cmiss_context_id context = Cmiss_context_create("test");
	Cmiss_region_id root_region = Cmiss_context_get_default_region(context);
	Cmiss_field_module_id fm = Cmiss_region_get_field_module(root_region);

	double onevalue[] = {0.0};
	Cmiss_field_id f1 = Cmiss_field_module_create_constant(fm, 1, onevalue);
	Cmiss_field_id f2 = Cmiss_field_module_create_string_constant(fm, "mystring");
	Cmiss_field_id f3 = Cmiss_field_module_create_string_constant(fm, "myotherstring");
	Cmiss_field_id f4 = Cmiss_field_module_create_if(fm, f1, f2, f3);
	EXPECT_NE(static_cast<Cmiss_field *>(0), f4);

	Cmiss_field_cache_id fc = Cmiss_field_module_create_cache(fm);
	char *outstring = Cmiss_field_evaluate_string(f4, fc);
	EXPECT_EQ('o', outstring[2]);

	Cmiss_deallocate(outstring);

	Cmiss_field_destroy(&f1);
	Cmiss_field_destroy(&f2);
	Cmiss_field_destroy(&f3);
	Cmiss_field_destroy(&f4);
	Cmiss_field_module_destroy(&fm);
	Cmiss_region_destroy(&root_region);
	Cmiss_context_destroy(&context);
}
