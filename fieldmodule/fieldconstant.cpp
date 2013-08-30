#include <gtest/gtest.h>

#include <zinc/zincconfigure.h>
#include <zinc/field.h>
#include <zinc/fieldarithmeticoperators.h>
#include <zinc/fieldconstant.h>
#include <zinc/status.h>

#include "zinctestsetup.hpp"

// Issue 3348: Assigning to a constant field invalidates only its own values in
// the field cache. It needs to invalidate all value caches for that field and
// all fields that depend on it in all existing field caches (as is done when
// finite element fields are assigned to).
TEST(Cmiss_field_constant, issue_3348_assign)
{
	ZincTestSetup zinc;

	const double value1 = 2.0;
	Cmiss_field_id f1 = Cmiss_field_module_create_constant(zinc.fm, 1, &value1);
	EXPECT_NE((Cmiss_field_id)0, f1);

	const double value2 = 1.0;
	Cmiss_field_id f2 = Cmiss_field_module_create_constant(zinc.fm, 1, &value2);
	EXPECT_NE((Cmiss_field_id)0, f2);

	Cmiss_field_id f3 = Cmiss_field_module_create_add(zinc.fm, f1, f2);
	EXPECT_NE((Cmiss_field_id)0, f3);

	const double expectedSum = value1 + value2;

	Cmiss_field_cache_id cache1 = Cmiss_field_module_create_cache(zinc.fm);
	Cmiss_field_cache_id cache2 = Cmiss_field_module_create_cache(zinc.fm);
	double value = 0.0;
	int result;
	EXPECT_EQ(CMISS_OK, result = Cmiss_field_evaluate_real(f3, cache1, 1, &value));
	EXPECT_EQ(expectedSum, value);
	EXPECT_EQ(CMISS_OK, result = Cmiss_field_evaluate_real(f3, cache2, 1, &value));
	EXPECT_EQ(expectedSum, value);
	const double newValue1 = 5.0;
	const double newExpectedSum = newValue1 + value2;
	EXPECT_EQ(CMISS_OK, result = Cmiss_field_assign_real(f1, cache1, 1, &newValue1));
	EXPECT_EQ(CMISS_OK, result = Cmiss_field_evaluate_real(f1, cache1, 1, &value));
	EXPECT_EQ(newValue1, value);
	EXPECT_EQ(CMISS_OK, result = Cmiss_field_evaluate_real(f1, cache2, 1, &value));
	EXPECT_EQ(newValue1, value);
	EXPECT_EQ(CMISS_OK, result = Cmiss_field_evaluate_real(f3, cache1, 1, &value));
	EXPECT_EQ(newExpectedSum, value);
	EXPECT_EQ(CMISS_OK, result = Cmiss_field_evaluate_real(f3, cache2, 1, &value));
	EXPECT_EQ(newExpectedSum, value);
	Cmiss_field_cache_destroy(&cache1);
	Cmiss_field_cache_destroy(&cache2);

	Cmiss_field_destroy(&f1);
	Cmiss_field_destroy(&f2);
	Cmiss_field_destroy(&f3);
}
