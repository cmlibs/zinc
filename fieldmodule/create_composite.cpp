
#include <gtest/gtest.h>

#include <zinc/zincconfigure.h>
#include <zinc/core.h>
#include <zinc/context.h>
#include <zinc/region.h>
#include <zinc/fieldmodule.h>
#include <zinc/field.h>
#include <zinc/fieldcomposite.h>
#include <zinc/fieldconstant.h>
#include <zinc/status.h>

#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"
#include "zinc/fieldtypesconstant.hpp"
#include "zinc/fieldtypescomposite.hpp"

TEST(cmzn_field_module_create_component, valid_args)
{
	ZincTestSetup zinc;

	const double values[] = { 2.0, 4.0, 6.0 };
	const int component_index = 2;
	cmzn_field_id f1 = cmzn_field_module_create_constant(zinc.fm, 3, values);
	EXPECT_NE((cmzn_field_id)0, f1);

	cmzn_field_id f2 = cmzn_field_module_create_component(zinc.fm, f1, component_index);
	EXPECT_NE((cmzn_field_id)0, f2);

	cmzn_field_cache_id cache = cmzn_field_module_create_cache(zinc.fm);
	double value = 0.0;
	EXPECT_EQ(CMZN_OK, cmzn_field_evaluate_real(f2, cache, 1, &value));
	EXPECT_EQ(values[component_index - 1], value);
	cmzn_field_cache_destroy(&cache);

	cmzn_field_destroy(&f1);
	cmzn_field_destroy(&f2);
}

TEST(cmzn_field_module_create_component, invalid_args)
{
	ZincTestSetup zinc;
	const double values[] = { 2.0, 4.0, 6.0 };
	const int component_index = 2;
	cmzn_field_id f1 = cmzn_field_module_create_constant(zinc.fm, 3, values);
	EXPECT_NE((cmzn_field_id)0, f1);

	EXPECT_EQ((cmzn_field_id)0, cmzn_field_module_create_component(0, f1, component_index));
	EXPECT_EQ((cmzn_field_id)0, cmzn_field_module_create_component(zinc.fm, 0, component_index));
	EXPECT_EQ((cmzn_field_id)0, cmzn_field_module_create_component(zinc.fm, f1, 0));
	EXPECT_EQ((cmzn_field_id)0, cmzn_field_module_create_component(zinc.fm, f1, 4));

	cmzn_field_destroy(&f1);
}

TEST(zincFieldModule_createComponent, valid_args)
{
	ZincTestSetupCpp zinc;

	const double values[] = { 2.0, 4.0, 6.0 };
	const int component_index = 2;
	FieldConstant f1 = zinc.fm.createConstant(3, values);
	EXPECT_TRUE(f1.isValid());

	FieldComponent f2 = zinc.fm.createComponent(f1, component_index);
	EXPECT_TRUE(f2.isValid());

	FieldCache cache = zinc.fm.createCache();
	double value = 0.0;
	EXPECT_EQ(CMZN_OK, f2.evaluateReal(cache, 1, &value));
	EXPECT_EQ(values[component_index - 1], value);
}

TEST(zincFieldModule_createComponent, invalid_args)
{
	ZincTestSetupCpp zinc;

	const double values[] = { 2.0, 4.0, 6.0 };
	const int component_index = 2;
	FieldConstant f1 = zinc.fm.createConstant(3, values);
	EXPECT_TRUE(f1.isValid());

	Field noField;
	EXPECT_FALSE(zinc.fm.createComponent(noField, component_index).isValid());
	EXPECT_FALSE(zinc.fm.createComponent(f1, 0).isValid());
	EXPECT_FALSE(zinc.fm.createComponent(f1, 4).isValid());
}
