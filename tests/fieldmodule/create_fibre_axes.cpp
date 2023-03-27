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
#include <cmlibs/zinc/region.h>
#include <cmlibs/zinc/fieldmodule.h>
#include <cmlibs/zinc/field.h>
#include <cmlibs/zinc/fieldfibres.h>
#include <cmlibs/zinc/fieldconstant.h>

TEST(cmzn_fieldmodule_create_field_fibre_axes, invalid_args)
{
	cmzn_context_id context = cmzn_context_create("test");
	cmzn_region_id root_region = cmzn_context_get_default_region(context);
	cmzn_fieldmodule_id fm = cmzn_region_get_fieldmodule(root_region);

	EXPECT_NE(static_cast<cmzn_fieldmodule *>(0), fm);

	cmzn_field_id f0 = cmzn_fieldmodule_create_field_fibre_axes(0, 0, 0);
	EXPECT_EQ(0, f0);

	cmzn_field_id f1 = cmzn_fieldmodule_create_field_fibre_axes(fm, 0, 0);
	EXPECT_EQ(0, f1);

	double values[] = {3.0, 2.0, 1.0, 7.0};
	cmzn_field_id f2 = cmzn_fieldmodule_create_field_constant(fm, 3, values);
	cmzn_field_id f3 = cmzn_fieldmodule_create_field_constant(fm, 4, values);

	cmzn_field_id f4 = cmzn_fieldmodule_create_field_fibre_axes(fm, f2, f3);
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

