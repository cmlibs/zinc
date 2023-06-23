/*
 * Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include <cmlibs/zinc/context.h>
#include <cmlibs/zinc/core.h>
#include <cmlibs/zinc/field.h>
#include <cmlibs/zinc/fieldcache.h>
#include <cmlibs/zinc/fieldconditional.h>
#include <cmlibs/zinc/fieldconstant.h>
#include <cmlibs/zinc/fieldmodule.h>
#include <cmlibs/zinc/region.h>

TEST(cmzn_fieldmodule_create_field_if, invalid_args)
{
	cmzn_context_id context = cmzn_context_create("test");
	cmzn_region_id root_region = cmzn_context_get_default_region(context);
	cmzn_fieldmodule_id fm = cmzn_region_get_fieldmodule(root_region);

	EXPECT_NE(static_cast<cmzn_fieldmodule *>(0), fm);

	cmzn_field_id f0 = cmzn_fieldmodule_create_field_if(0, 0, 0, 0);
	EXPECT_EQ(0, f0);

	cmzn_field_id f1 = cmzn_fieldmodule_create_field_if(fm, 0, 0, 0);
	EXPECT_EQ(0, f1);

	double values[] = {3.0, 2.0, 1.0};
	cmzn_field_id f2 = cmzn_fieldmodule_create_field_constant(fm, 3, values);
	cmzn_field_id f3 = cmzn_fieldmodule_create_field_constant(fm, 3, values);

	cmzn_field_id f4 = cmzn_fieldmodule_create_field_if(fm, 0, f2, f3);
	EXPECT_EQ(0, f4);

	cmzn_field_destroy(&f0);
	cmzn_field_destroy(&f1);
	cmzn_field_destroy(&f2);
	cmzn_field_destroy(&f3);
	cmzn_field_destroy(&f4);
	cmzn_fieldmodule_destroy(&fm);
	cmzn_region_destroy(&root_region);
	cmzn_context_destroy(&context);
}

TEST(cmzn_fieldmodule_create_field_if, valid_args)
{
	cmzn_context_id context = cmzn_context_create("test");
	cmzn_region_id root_region = cmzn_context_get_default_region(context);
	cmzn_fieldmodule_id fm = cmzn_region_get_fieldmodule(root_region);

	double values[] = {3.0, 2.0, 1.0};
	cmzn_field_id f1 = cmzn_fieldmodule_create_field_constant(fm, 3, values);
	cmzn_field_id f2 = cmzn_fieldmodule_create_field_constant(fm, 3, values);
	cmzn_field_id f3 = cmzn_fieldmodule_create_field_constant(fm, 3, values);

	cmzn_field_id f4 = cmzn_fieldmodule_create_field_if(fm, f1, f2, f3);
	EXPECT_NE(static_cast<cmzn_field *>(0), f4);

	cmzn_field_destroy(&f1);
	cmzn_field_destroy(&f2);
	cmzn_field_destroy(&f3);
	cmzn_field_destroy(&f4);
	cmzn_fieldmodule_destroy(&fm);
	cmzn_region_destroy(&root_region);
	cmzn_context_destroy(&context);
}

TEST(cmzn_fieldmodule_create_field_if, single_component_switch)
{
	cmzn_context_id context = cmzn_context_create("test");
	cmzn_region_id root_region = cmzn_context_get_default_region(context);
	cmzn_fieldmodule_id fm = cmzn_region_get_fieldmodule(root_region);

	double onevalue[] = {5.0};
	double values1[] = {3.0, 2.0, 1.0};
	double values2[] = {6.0, 5.0, 4.0};
	cmzn_field_id f1 = cmzn_fieldmodule_create_field_constant(fm, 1, onevalue);
	cmzn_field_id f2 = cmzn_fieldmodule_create_field_constant(fm, 3, values1);
	cmzn_field_id f3 = cmzn_fieldmodule_create_field_constant(fm, 3, values2);
	cmzn_field_id f4 = cmzn_fieldmodule_create_field_if(fm, f1, f2, f3);
	EXPECT_NE(static_cast<cmzn_field *>(0), f4);

	cmzn_fieldcache_id fc = cmzn_fieldmodule_create_fieldcache(fm);

	double outvalues[3];
	cmzn_field_evaluate_real(f4, fc, 3, outvalues);
	EXPECT_EQ(3.0, outvalues[0]);
	EXPECT_EQ(2.0, outvalues[1]);
	EXPECT_EQ(1.0, outvalues[2]);

	cmzn_field_destroy(&f1);
	cmzn_field_destroy(&f2);
	cmzn_field_destroy(&f3);
	cmzn_field_destroy(&f4);
	cmzn_fieldcache_destroy(&fc);
	cmzn_fieldmodule_destroy(&fm);
	cmzn_region_destroy(&root_region);
	cmzn_context_destroy(&context);
}

TEST(cmzn_fieldmodule_create_field_if, string_constant)
{
	cmzn_context_id context = cmzn_context_create("test");
	cmzn_region_id root_region = cmzn_context_get_default_region(context);
	cmzn_fieldmodule_id fm = cmzn_region_get_fieldmodule(root_region);

	double onevalue[] = {0.0};
	cmzn_field_id f1 = cmzn_fieldmodule_create_field_constant(fm, 1, onevalue);
	cmzn_field_id f2 = cmzn_fieldmodule_create_field_string_constant(fm, "mystring");
	cmzn_field_id f3 = cmzn_fieldmodule_create_field_string_constant(fm, "myotherstring");
	cmzn_field_id f4 = cmzn_fieldmodule_create_field_if(fm, f1, f2, f3);
	EXPECT_NE(static_cast<cmzn_field *>(0), f4);

	cmzn_fieldcache_id fc = cmzn_fieldmodule_create_fieldcache(fm);
	char *outstring = cmzn_field_evaluate_string(f4, fc);
	EXPECT_EQ('o', outstring[2]);

	cmzn_deallocate(outstring);
	cmzn_fieldcache_destroy(&fc);
	cmzn_field_destroy(&f1);
	cmzn_field_destroy(&f2);
	cmzn_field_destroy(&f3);
	cmzn_field_destroy(&f4);
	cmzn_fieldmodule_destroy(&fm);
	cmzn_region_destroy(&root_region);
	cmzn_context_destroy(&context);
}

