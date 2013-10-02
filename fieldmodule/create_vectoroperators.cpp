
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
#include "zinc/fieldconstant.hpp"
#include "zinc/fieldvectoroperators.hpp"

TEST(cmzn_field_cross_product, create_evaluate_2d)
{
	ZincTestSetup zinc;

	const double values1[] = { 1.0, 1.0 };
	cmzn_field_id f1 = cmzn_fieldmodule_create_field_constant(zinc.fm, 2, values1);
	EXPECT_NE((cmzn_field_id)0, f1);

	cmzn_field_id f2 = cmzn_fieldmodule_create_field_cross_product(zinc.fm, 1, &f1);
	EXPECT_NE((cmzn_field_id)0, f2);

	cmzn_fieldcache_id cache = cmzn_fieldmodule_create_fieldcache(zinc.fm);
	double values[2];
	EXPECT_EQ(CMZN_OK, cmzn_field_evaluate_real(f2, cache, 2, values));
	ASSERT_DOUBLE_EQ(-1.0, values[0]);
	ASSERT_DOUBLE_EQ(1.0, values[1]);
	cmzn_fieldcache_destroy(&cache);

	// test invalid arguments
	const double one = 1.0;
	cmzn_field_id f4 = cmzn_fieldmodule_create_field_constant(zinc.fm, 1, &one);
	EXPECT_NE((cmzn_field_id)0, f4);
	EXPECT_EQ((cmzn_field_id)0, cmzn_fieldmodule_create_field_cross_product(0, 1, &f1));
	EXPECT_EQ((cmzn_field_id)0, cmzn_fieldmodule_create_field_cross_product(zinc.fm, 0, &f1));
	EXPECT_EQ((cmzn_field_id)0, cmzn_fieldmodule_create_field_cross_product(zinc.fm, 1, 0));
	EXPECT_EQ((cmzn_field_id)0, cmzn_fieldmodule_create_field_cross_product(zinc.fm, 1, &f4));

	cmzn_field_destroy(&f1);
	cmzn_field_destroy(&f2);
	cmzn_field_destroy(&f4);
}

TEST(zincFieldCrossProduct, create_evaluate_2d)
{
	ZincTestSetupCpp zinc;

	const double values1[] = { 1.0, 1.0 };
	FieldConstant f1 = zinc.fm.createFieldConstant(2, values1);
	EXPECT_TRUE(f1.isValid());

	FieldCrossProduct f2 = zinc.fm.createFieldCrossProduct(1, &f1);
	EXPECT_TRUE(f2.isValid());

	Fieldcache cache = zinc.fm.createFieldcache();
	double values[2];
	EXPECT_EQ(CMZN_OK, f2.evaluateReal(cache, 2, values));
	ASSERT_DOUBLE_EQ(-1.0, values[0]);
	ASSERT_DOUBLE_EQ(1.0, values[1]);

	// test invalid arguments
	const double one = 1.0;
	FieldConstant f4 = zinc.fm.createFieldConstant(1, &one);
	EXPECT_TRUE(f4.isValid());
	Field noField;
	EXPECT_FALSE(zinc.fm.createFieldCrossProduct(0, &f1).isValid());
	EXPECT_FALSE(zinc.fm.createFieldCrossProduct(1, &noField).isValid());
	EXPECT_FALSE(zinc.fm.createFieldCrossProduct(1, &f4).isValid());
}

TEST(cmzn_field_cross_product, create_evaluate_3d)
{
	ZincTestSetup zinc;

	const double values1[] = { 2.0, 0.0, 0.0 };
	cmzn_field_id f1 = cmzn_fieldmodule_create_field_constant(zinc.fm, 3, values1);
	EXPECT_NE((cmzn_field_id)0, f1);
	const double values2[] = { 0.0, 1.5, 0.0 };
	cmzn_field_id f2 = cmzn_fieldmodule_create_field_constant(zinc.fm, 3, values2);
	EXPECT_NE((cmzn_field_id)0, f2);

	cmzn_field_id f3 = cmzn_fieldmodule_create_field_cross_product_3d(zinc.fm, f1, f2);
	EXPECT_NE((cmzn_field_id)0, f3);

	cmzn_fieldcache_id cache = cmzn_fieldmodule_create_fieldcache(zinc.fm);
	double values[3];
	EXPECT_EQ(CMZN_OK, cmzn_field_evaluate_real(f3, cache, 3, values));
	ASSERT_DOUBLE_EQ(0.0, values[0]);
	ASSERT_DOUBLE_EQ(0.0, values[1]);
	ASSERT_DOUBLE_EQ(3.0, values[2]);
	cmzn_fieldcache_destroy(&cache);

	// test invalid arguments
	const double one = 1.0;
	cmzn_field_id f4 = cmzn_fieldmodule_create_field_constant(zinc.fm, 1, &one);
	EXPECT_NE((cmzn_field_id)0, f4);
	EXPECT_EQ((cmzn_field_id)0, cmzn_fieldmodule_create_field_cross_product_3d(0, f1, f2));
	EXPECT_EQ((cmzn_field_id)0, cmzn_fieldmodule_create_field_cross_product_3d(zinc.fm, 0, f2));
	EXPECT_EQ((cmzn_field_id)0, cmzn_fieldmodule_create_field_cross_product_3d(zinc.fm, f1, 0));
	EXPECT_EQ((cmzn_field_id)0, cmzn_fieldmodule_create_field_cross_product_3d(zinc.fm, f1, f4));

	cmzn_field_destroy(&f1);
	cmzn_field_destroy(&f2);
	cmzn_field_destroy(&f3);
	cmzn_field_destroy(&f4);
}

TEST(zincFieldCrossProduct, create_evaluate_3d)
{
	ZincTestSetupCpp zinc;

	const double values1[] = { 2.0, 0.0, 0.0 };
	FieldConstant f1 = zinc.fm.createFieldConstant(3, values1);
	EXPECT_TRUE(f1.isValid());
	const double values2[] = { 0.0, 1.5, 0.0 };
	FieldConstant f2 = zinc.fm.createFieldConstant(3, values2);
	EXPECT_TRUE(f2.isValid());

	FieldCrossProduct f3 = zinc.fm.createFieldCrossProduct(f1, f2);
	EXPECT_TRUE(f3.isValid());

	Fieldcache cache = zinc.fm.createFieldcache();
	double values[3];
	EXPECT_EQ(CMZN_OK, f3.evaluateReal(cache, 3, values));
	ASSERT_DOUBLE_EQ(0.0, values[0]);
	ASSERT_DOUBLE_EQ(0.0, values[1]);
	ASSERT_DOUBLE_EQ(3.0, values[2]);

	// test invalid arguments
	const double one = 1.0;
	FieldConstant f4 = zinc.fm.createFieldConstant(1, &one);
	EXPECT_TRUE(f4.isValid());
	Field noField;
	EXPECT_FALSE(zinc.fm.createFieldCrossProduct(noField, f2).isValid());
	EXPECT_FALSE(zinc.fm.createFieldCrossProduct(f1, noField).isValid());
	EXPECT_FALSE(zinc.fm.createFieldCrossProduct(f1, noField).isValid());
}

TEST(cmzn_field_sum_components, create_evaluate)
{
	ZincTestSetup zinc;

	const double values[] = { 2.0, 4.0, 6.0 };
	cmzn_field_id f1 = cmzn_fieldmodule_create_field_constant(zinc.fm, 3, values);
	EXPECT_NE((cmzn_field_id)0, f1);

	cmzn_field_id f2 = cmzn_fieldmodule_create_field_sum_components(zinc.fm, f1);
	EXPECT_NE((cmzn_field_id)0, f2);

	cmzn_fieldcache_id cache = cmzn_fieldmodule_create_fieldcache(zinc.fm);
	double value = 0.0;
	EXPECT_EQ(CMZN_OK, cmzn_field_evaluate_real(f2, cache, 1, &value));
	ASSERT_DOUBLE_EQ(12.0, value);
	cmzn_fieldcache_destroy(&cache);

	// test invalid arguments
	EXPECT_EQ((cmzn_field_id)0, cmzn_fieldmodule_create_field_sum_components(0, f1));
	EXPECT_EQ((cmzn_field_id)0, cmzn_fieldmodule_create_field_sum_components(zinc.fm, 0));

	cmzn_field_destroy(&f1);
	cmzn_field_destroy(&f2);
}

TEST(zincFieldSumComponents, create_evaluate)
{
	ZincTestSetupCpp zinc;

	const double values[] = { 2.0, 4.0, 6.0 };
	FieldConstant f1 = zinc.fm.createFieldConstant(3, values);
	EXPECT_TRUE(f1.isValid());

	FieldSumComponents f2 = zinc.fm.createFieldSumComponents(f1);
	EXPECT_TRUE(f2.isValid());

	Fieldcache cache = zinc.fm.createFieldcache();
	double value = 0.0;
	EXPECT_EQ(CMZN_OK, f2.evaluateReal(cache, 1, &value));
	ASSERT_DOUBLE_EQ(12.0, value);

	// test invalid arguments
	Field noField;
	FieldSumComponents f3 = zinc.fm.createFieldSumComponents(noField);
	EXPECT_FALSE(f3.isValid());
}
