
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

TEST(Cmiss_field_cross_product, create_evaluate_2d)
{
	ZincTestSetup zinc;

	const double values1[] = { 1.0, 1.0 };
	Cmiss_field_id f1 = Cmiss_field_module_create_constant(zinc.fm, 2, values1);
	EXPECT_NE((Cmiss_field_id)0, f1);

	Cmiss_field_id f2 = Cmiss_field_module_create_cross_product(zinc.fm, 1, &f1);
	EXPECT_NE((Cmiss_field_id)0, f2);

	Cmiss_field_cache_id cache = Cmiss_field_module_create_cache(zinc.fm);
	double values[2];
	EXPECT_EQ(CMISS_OK, Cmiss_field_evaluate_real(f2, cache, 2, values));
	ASSERT_DOUBLE_EQ(-1.0, values[0]);
	ASSERT_DOUBLE_EQ(1.0, values[1]);
	Cmiss_field_cache_destroy(&cache);

	// test invalid arguments
	const double one = 1.0;
	Cmiss_field_id f4 = Cmiss_field_module_create_constant(zinc.fm, 1, &one);
	EXPECT_NE((Cmiss_field_id)0, f4);
	EXPECT_EQ((Cmiss_field_id)0, Cmiss_field_module_create_cross_product(0, 1, &f1));
	EXPECT_EQ((Cmiss_field_id)0, Cmiss_field_module_create_cross_product(zinc.fm, 0, &f1));
	EXPECT_EQ((Cmiss_field_id)0, Cmiss_field_module_create_cross_product(zinc.fm, 1, 0));
	EXPECT_EQ((Cmiss_field_id)0, Cmiss_field_module_create_cross_product(zinc.fm, 1, &f4));

	Cmiss_field_destroy(&f1);
	Cmiss_field_destroy(&f2);
	Cmiss_field_destroy(&f4);
}

TEST(zincFieldCrossProduct, create_evaluate_2d)
{
	ZincTestSetupCpp zinc;

	const double values1[] = { 1.0, 1.0 };
	FieldConstant f1 = zinc.fm.createConstant(2, values1);
	EXPECT_EQ(true, f1.isValid());

	FieldCrossProduct f2 = zinc.fm.createCrossProduct(1, &f1);
	EXPECT_EQ(true, f2.isValid());

	FieldCache cache = zinc.fm.createCache();
	double values[2];
	EXPECT_EQ(CMISS_OK, f2.evaluateReal(cache, 2, values));
	ASSERT_DOUBLE_EQ(-1.0, values[0]);
	ASSERT_DOUBLE_EQ(1.0, values[1]);

	// test invalid arguments
	const double one = 1.0;
	FieldConstant f4 = zinc.fm.createConstant(1, &one);
	EXPECT_EQ(true, f4.isValid());
	Field noField;
	Field f;
	f = zinc.fm.createCrossProduct(0, &f1);
	EXPECT_EQ(false, f.isValid());
	f = zinc.fm.createCrossProduct(1, &noField);
	EXPECT_EQ(false, f.isValid());
	f = zinc.fm.createCrossProduct(1, &f4);
	EXPECT_EQ(false, f.isValid());
}

TEST(Cmiss_field_cross_product, create_evaluate_3d)
{
	ZincTestSetup zinc;

	const double values1[] = { 2.0, 0.0, 0.0 };
	Cmiss_field_id f1 = Cmiss_field_module_create_constant(zinc.fm, 3, values1);
	EXPECT_NE((Cmiss_field_id)0, f1);
	const double values2[] = { 0.0, 1.5, 0.0 };
	Cmiss_field_id f2 = Cmiss_field_module_create_constant(zinc.fm, 3, values2);
	EXPECT_NE((Cmiss_field_id)0, f2);

	Cmiss_field_id f3 = Cmiss_field_module_create_cross_product_3d(zinc.fm, f1, f2);
	EXPECT_NE((Cmiss_field_id)0, f3);

	Cmiss_field_cache_id cache = Cmiss_field_module_create_cache(zinc.fm);
	double values[3];
	EXPECT_EQ(CMISS_OK, Cmiss_field_evaluate_real(f3, cache, 3, values));
	ASSERT_DOUBLE_EQ(0.0, values[0]);
	ASSERT_DOUBLE_EQ(0.0, values[1]);
	ASSERT_DOUBLE_EQ(3.0, values[2]);
	Cmiss_field_cache_destroy(&cache);

	// test invalid arguments
	const double one = 1.0;
	Cmiss_field_id f4 = Cmiss_field_module_create_constant(zinc.fm, 1, &one);
	EXPECT_NE((Cmiss_field_id)0, f4);
	EXPECT_EQ((Cmiss_field_id)0, Cmiss_field_module_create_cross_product_3d(0, f1, f2));
	EXPECT_EQ((Cmiss_field_id)0, Cmiss_field_module_create_cross_product_3d(zinc.fm, 0, f2));
	EXPECT_EQ((Cmiss_field_id)0, Cmiss_field_module_create_cross_product_3d(zinc.fm, f1, 0));
	EXPECT_EQ((Cmiss_field_id)0, Cmiss_field_module_create_cross_product_3d(zinc.fm, f1, f4));

	Cmiss_field_destroy(&f1);
	Cmiss_field_destroy(&f2);
	Cmiss_field_destroy(&f3);
	Cmiss_field_destroy(&f4);
}

TEST(zincFieldCrossProduct, create_evaluate_3d)
{
	ZincTestSetupCpp zinc;

	const double values1[] = { 2.0, 0.0, 0.0 };
	FieldConstant f1 = zinc.fm.createConstant(3, values1);
	EXPECT_EQ(true, f1.isValid());
	const double values2[] = { 0.0, 1.5, 0.0 };
	FieldConstant f2 = zinc.fm.createConstant(3, values2);
	EXPECT_EQ(true, f2.isValid());

	FieldCrossProduct f3 = zinc.fm.createCrossProduct(f1, f2);
	EXPECT_EQ(true, f3.isValid());

	FieldCache cache = zinc.fm.createCache();
	double values[3];
	EXPECT_EQ(CMISS_OK, f3.evaluateReal(cache, 3, values));
	ASSERT_DOUBLE_EQ(0.0, values[0]);
	ASSERT_DOUBLE_EQ(0.0, values[1]);
	ASSERT_DOUBLE_EQ(3.0, values[2]);

	// test invalid arguments
	const double one = 1.0;
	FieldConstant f4 = zinc.fm.createConstant(1, &one);
	EXPECT_EQ(true, f4.isValid());
	Field noField;
	Field f;
	f = zinc.fm.createCrossProduct(noField, f2);
	EXPECT_EQ(false, f.isValid());
	f = zinc.fm.createCrossProduct(f1, noField);
	EXPECT_EQ(false, f.isValid());
	f = zinc.fm.createCrossProduct(f1, f4);
	EXPECT_EQ(false, f.isValid());
}

TEST(Cmiss_field_sum_components, create_evaluate)
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

TEST(zincFieldSumComponents, create_evaluate)
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
