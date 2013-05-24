
#include <gtest/gtest.h>

#include <zinc/zincconfigure.h>
#include <zinc/core.h>
#include <zinc/context.h>
#include <zinc/region.h>
#include <zinc/fieldmodule.h>
#include <zinc/field.h>
#include <zinc/fieldvectoroperators.h>
#include <zinc/fieldconstant.h>
#include <zinc/status.h>

#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"
#include "zinc/fieldtypesconstant.hpp"
#include "zinc/fieldtypesvectoroperators.hpp"

TEST(Cmiss_field_sum_components, create)
{
	ZincTestSetup zinc;

	const double values[] = { 2.0, 4.0, 6.0 };
	Cmiss_field_id f1 = Cmiss_field_module_create_constant(zinc.fm, 3, values);
	EXPECT_NE((Cmiss_field_id)0, f1);

	Cmiss_field_id f2 = Cmiss_field_module_create_sum_components(zinc.fm, f1);
	EXPECT_NE((Cmiss_field_id)0, f2);

	Cmiss_field_cache_id cache = Cmiss_field_module_create_cache(zinc.fm);
	double value = 0.0;
	EXPECT_EQ(CMISS_OK, Cmiss_field_evaluate_real(f2, cache, 1, &value));
	ASSERT_DOUBLE_EQ(12.0, value);
	Cmiss_field_cache_destroy(&cache);

	// test invalid arguments
	EXPECT_EQ((Cmiss_field_id)0, Cmiss_field_module_create_sum_components(0, f1));
	EXPECT_EQ((Cmiss_field_id)0, Cmiss_field_module_create_sum_components(zinc.fm, 0));

	Cmiss_field_destroy(&f1);
	Cmiss_field_destroy(&f2);
}

TEST(zincFieldSumComponents, create)
{
	ZincTestSetupCpp zinc;

	const double values[] = { 2.0, 4.0, 6.0 };
	FieldConstant f1 = zinc.fm.createConstant(3, values);
	EXPECT_EQ(true, f1.isValid());

	FieldSumComponents f2 = zinc.fm.createSumComponents(f1);
	EXPECT_EQ(true, f2.isValid());

	FieldCache cache = zinc.fm.createCache();
	double value = 0.0;
	EXPECT_EQ(CMISS_OK, f2.evaluateReal(cache, 1, &value));
	ASSERT_DOUBLE_EQ(12.0, value);

	// test invalid arguments
	Field noField;
	FieldSumComponents f3 = zinc.fm.createSumComponents(noField);
	EXPECT_EQ(false, f3.isValid());
}
