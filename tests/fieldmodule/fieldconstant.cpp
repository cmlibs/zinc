/*
 * Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include <cmlibs/zinc/core.h>
#include <cmlibs/zinc/field.h>
#include <cmlibs/zinc/fieldarithmeticoperators.h>
#include <cmlibs/zinc/fieldcache.h>
#include <cmlibs/zinc/fieldconstant.h>
#include <cmlibs/zinc/status.h>

#include <cmlibs/zinc/field.hpp>
#include <cmlibs/zinc/fieldcache.hpp>
#include <cmlibs/zinc/fieldconstant.hpp>

#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"

// Issue 3348: Assigning to a constant field invalidates only its own values in
// the field cache. It needs to invalidate all value caches for that field and
// all fields that depend on it in all existing field caches (as is done when
// finite element fields are assigned to).
TEST(cmzn_field_constant, issue_3348_assign)
{
	ZincTestSetup zinc;

	const double value1 = 2.0;
	cmzn_field_id f1 = cmzn_fieldmodule_create_field_constant(zinc.fm, 1, &value1);
	EXPECT_NE((cmzn_field_id)0, f1);

	const double value2 = 1.0;
	cmzn_field_id f2 = cmzn_fieldmodule_create_field_constant(zinc.fm, 1, &value2);
	EXPECT_NE((cmzn_field_id)0, f2);

	cmzn_field_id f3 = cmzn_fieldmodule_create_field_add(zinc.fm, f1, f2);
	EXPECT_NE((cmzn_field_id)0, f3);

	EXPECT_EQ(2, cmzn_field_get_number_of_source_fields(f3));
	cmzn_field_id tmp1 = cmzn_field_get_source_field(f3, 1);
	EXPECT_EQ(f1, tmp1);
	cmzn_field_destroy(&tmp1);
	cmzn_field_id tmp2 = cmzn_field_get_source_field(f3, 2);
	EXPECT_EQ(f2, tmp2);
	cmzn_field_destroy(&tmp2);

	const double expectedSum = value1 + value2;

	cmzn_fieldcache_id cache1 = cmzn_fieldmodule_create_fieldcache(zinc.fm);
	cmzn_fieldcache_id cache2 = cmzn_fieldmodule_create_fieldcache(zinc.fm);
	double value = 0.0;
	int result;
	EXPECT_EQ(CMZN_OK, result = cmzn_field_evaluate_real(f3, cache1, 1, &value));
	EXPECT_EQ(expectedSum, value);
	EXPECT_EQ(CMZN_OK, result = cmzn_field_evaluate_real(f3, cache2, 1, &value));
	EXPECT_EQ(expectedSum, value);

	const double newValue1 = 5.0;
	const double newExpectedSum1 = newValue1 + value2;
	EXPECT_EQ(CMZN_OK, result = cmzn_field_assign_real(f1, cache1, 1, &newValue1));
	EXPECT_EQ(CMZN_OK, result = cmzn_field_evaluate_real(f1, cache1, 1, &value));
	EXPECT_EQ(newValue1, value);
	EXPECT_EQ(CMZN_OK, result = cmzn_field_evaluate_real(f1, cache2, 1, &value));
	EXPECT_EQ(newValue1, value);
	EXPECT_EQ(CMZN_OK, result = cmzn_field_evaluate_real(f3, cache1, 1, &value));
	EXPECT_EQ(newExpectedSum1, value);
	EXPECT_EQ(CMZN_OK, result = cmzn_field_evaluate_real(f3, cache2, 1, &value));
	EXPECT_EQ(newExpectedSum1, value);

	// test similar change between begin/end change

	const double newValue2 = 7.5;
	const double newExpectedSum2 = newValue2 + value2;
	cmzn_fieldmodule_begin_change(zinc.fm);
	EXPECT_EQ(CMZN_OK, result = cmzn_field_assign_real(f1, cache1, 1, &newValue2));
	EXPECT_EQ(CMZN_OK, result = cmzn_field_evaluate_real(f1, cache1, 1, &value));
	EXPECT_EQ(newValue2, value);
	EXPECT_EQ(CMZN_OK, result = cmzn_field_evaluate_real(f1, cache2, 1, &value));
	EXPECT_EQ(newValue2, value);
	EXPECT_EQ(CMZN_OK, result = cmzn_field_evaluate_real(f3, cache1, 1, &value));
	EXPECT_EQ(newExpectedSum2, value);
	EXPECT_EQ(CMZN_OK, result = cmzn_field_evaluate_real(f3, cache2, 1, &value));
	EXPECT_EQ(newExpectedSum2, value);
	cmzn_fieldmodule_end_change(zinc.fm);

	cmzn_fieldcache_destroy(&cache1);
	cmzn_fieldcache_destroy(&cache2);

	cmzn_field_destroy(&f1);
	cmzn_field_destroy(&f2);
	cmzn_field_destroy(&f3);
}

TEST(ZincFieldConstant, valid_arguments)
{
	ZincTestSetupCpp zinc;

	const double values_in[3] = { 1.0, 2.5, -3.0 };
	FieldConstant fieldConstant = zinc.fm.createFieldConstant(3, values_in);
	EXPECT_TRUE(fieldConstant.isValid());
	// test cast
	FieldConstant tmpFieldConstant = fieldConstant.castConstant();
	EXPECT_EQ(fieldConstant, tmpFieldConstant);

	Fieldcache fieldcache = zinc.fm.createFieldcache();
	double values_out[3];
	EXPECT_EQ(RESULT_OK, fieldConstant.evaluateReal(fieldcache,3, values_out));
	for (int c = 0; c < 3; ++c)
		EXPECT_DOUBLE_EQ(values_in[c], values_out[c]);
}

TEST(ZincFieldStringConstant, valid_arguments)
{
	ZincTestSetupCpp zinc;

	const char string_in[] = "blah!";
	FieldStringConstant fieldStringConstant = zinc.fm.createFieldStringConstant(string_in);
	EXPECT_TRUE(fieldStringConstant.isValid());
	// test cast
	FieldStringConstant tmpFieldStringConstant = fieldStringConstant.castStringConstant();
	EXPECT_EQ(fieldStringConstant, tmpFieldStringConstant);

	Fieldcache fieldcache = zinc.fm.createFieldcache();
	char *string_out = fieldStringConstant.evaluateString(fieldcache);
	EXPECT_STREQ(string_in, string_out);
    cmzn_deallocate(string_out);
}
