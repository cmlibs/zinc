
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
#include "zinc/fieldconstant.hpp"
#include "zinc/fieldcomposite.hpp"

TEST(cmzn_fieldmodule_create_field_component, valid_args)
{
	ZincTestSetup zinc;

	const double values[] = { 2.0, 4.0, 6.0 };
	const int component_index = 2;
	cmzn_field_id f1 = cmzn_fieldmodule_create_field_constant(zinc.fm, 3, values);
	EXPECT_NE((cmzn_field_id)0, f1);

	cmzn_field_id f2 = cmzn_fieldmodule_create_field_component(zinc.fm, f1, component_index);
	EXPECT_NE((cmzn_field_id)0, f2);

	cmzn_fieldcache_id cache = cmzn_fieldmodule_create_fieldcache(zinc.fm);
	double value = 0.0;
	EXPECT_EQ(CMZN_OK, cmzn_field_evaluate_real(f2, cache, 1, &value));
	EXPECT_EQ(values[component_index - 1], value);
	cmzn_fieldcache_destroy(&cache);

	cmzn_field_destroy(&f1);
	cmzn_field_destroy(&f2);
}

TEST(cmzn_fieldmodule_create_field_component, invalid_args)
{
	ZincTestSetup zinc;
	const double values[] = { 2.0, 4.0, 6.0 };
	const int component_index = 2;
	cmzn_field_id f1 = cmzn_fieldmodule_create_field_constant(zinc.fm, 3, values);
	EXPECT_NE((cmzn_field_id)0, f1);

	EXPECT_EQ((cmzn_field_id)0, cmzn_fieldmodule_create_field_component(0, f1, component_index));
	EXPECT_EQ((cmzn_field_id)0, cmzn_fieldmodule_create_field_component(zinc.fm, 0, component_index));
	EXPECT_EQ((cmzn_field_id)0, cmzn_fieldmodule_create_field_component(zinc.fm, f1, 0));
	EXPECT_EQ((cmzn_field_id)0, cmzn_fieldmodule_create_field_component(zinc.fm, f1, 4));

	cmzn_field_destroy(&f1);
}

TEST(zincFieldModule_createComponent, valid_args)
{
	ZincTestSetupCpp zinc;

	const double values[] = { 2.0, 4.0, 6.0 };
	const int component_index = 2;
	FieldConstant f1 = zinc.fm.createFieldConstant(3, values);
	EXPECT_TRUE(f1.isValid());

	FieldComponent f2 = zinc.fm.createFieldComponent(f1, component_index);
	EXPECT_TRUE(f2.isValid());

	Fieldcache cache = zinc.fm.createFieldcache();
	double value = 0.0;
	EXPECT_EQ(CMZN_OK, f2.evaluateReal(cache, 1, &value));
	EXPECT_EQ(values[component_index - 1], value);
}

TEST(zincFieldModule_createComponent, invalid_args)
{
	ZincTestSetupCpp zinc;

	const double values[] = { 2.0, 4.0, 6.0 };
	const int component_index = 2;
	FieldConstant f1 = zinc.fm.createFieldConstant(3, values);
	EXPECT_TRUE(f1.isValid());

	Field noField;
	EXPECT_FALSE(zinc.fm.createFieldComponent(noField, component_index).isValid());
	EXPECT_FALSE(zinc.fm.createFieldComponent(f1, 0).isValid());
	EXPECT_FALSE(zinc.fm.createFieldComponent(f1, 4).isValid());
}
