/*
 * Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include "zinctestsetup.hpp"
#include <cmlibs/zinc/core.h>
#include <cmlibs/zinc/field.h>
#include <cmlibs/zinc/fieldcache.h>
#include <cmlibs/zinc/fieldcomposite.h>
#include <cmlibs/zinc/fieldconstant.h>

#include "zinctestsetupcpp.hpp"
#include <cmlibs/zinc/field.hpp>
#include <cmlibs/zinc/fieldcache.hpp>
#include <cmlibs/zinc/fieldcomposite.hpp>
#include <cmlibs/zinc/fieldconstant.hpp>

TEST(cmzn_field_component, single_valid_args)
{
	ZincTestSetup zinc;

	const double sourceValues[] = { 2.0, 4.0, 6.0 };
	cmzn_field_id f1 = cmzn_fieldmodule_create_field_constant(zinc.fm, 3, sourceValues);
	EXPECT_NE((cmzn_field_id)0, f1);

	const int component_index = 2;
	cmzn_field_id f2 = cmzn_fieldmodule_create_field_component(zinc.fm, f1, component_index);
	EXPECT_NE((cmzn_field_id)0, f2);
	EXPECT_EQ(1, cmzn_field_get_number_of_components(f2));

	cmzn_field_component_id fc = cmzn_field_cast_component(f2);
	EXPECT_NE((cmzn_field_component_id)0, fc);

	cmzn_fieldcache_id cache = cmzn_fieldmodule_create_fieldcache(zinc.fm);
	double value = 0.0;
	EXPECT_EQ(CMZN_OK, cmzn_field_evaluate_real(f2, cache, 1, &value));
	EXPECT_EQ(sourceValues[component_index - 1], value);
	 
	int temp_component_index = cmzn_field_component_get_component_index(fc);
	EXPECT_EQ(component_index, temp_component_index);
	const int new_component_index = 3;
	EXPECT_EQ(CMZN_OK, cmzn_field_component_set_component_index(fc, new_component_index));
	temp_component_index = cmzn_field_component_get_component_index(fc);
	EXPECT_EQ(new_component_index, temp_component_index);
	EXPECT_EQ(CMZN_OK, cmzn_field_evaluate_real(f2, cache, 1, &value));
	EXPECT_EQ(sourceValues[new_component_index - 1], value);

	cmzn_fieldcache_destroy(&cache);
	cmzn_field_component_destroy(&fc);
	cmzn_field_destroy(&f1);
	cmzn_field_destroy(&f2);
}

TEST(cmzn_field_component, single_invalid_args)
{
	ZincTestSetup zinc;
	const double values[] = { 2.0, 4.0, 6.0 };
	cmzn_field_id f1 = cmzn_fieldmodule_create_field_constant(zinc.fm, 3, values);
	EXPECT_NE((cmzn_field_id)0, f1);

	cmzn_field_component_id notComponent = cmzn_field_cast_component(f1);
	EXPECT_EQ((cmzn_field_component_id)0, notComponent);

	const int component_index = 2;
	EXPECT_EQ((cmzn_field_id)0, cmzn_fieldmodule_create_field_component(0, f1, component_index));
	EXPECT_EQ((cmzn_field_id)0, cmzn_fieldmodule_create_field_component(zinc.fm, 0, component_index));
	EXPECT_EQ((cmzn_field_id)0, cmzn_fieldmodule_create_field_component(zinc.fm, f1, 0));
	EXPECT_EQ((cmzn_field_id)0, cmzn_fieldmodule_create_field_component(zinc.fm, f1, 4));

	cmzn_field_id f2 = cmzn_fieldmodule_create_field_component(zinc.fm, f1, component_index);
	EXPECT_NE((cmzn_field_id)0, f2);
	EXPECT_EQ(1, cmzn_field_get_number_of_components(f2));

	cmzn_field_component_id fc = cmzn_field_cast_component(f2);
	EXPECT_NE((cmzn_field_component_id)0, fc);

	EXPECT_EQ(0, cmzn_field_component_get_component_index((cmzn_field_component_id)0));
	EXPECT_EQ(ERROR_ARGUMENT, cmzn_field_component_set_component_index(fc, 0));
	EXPECT_EQ(ERROR_ARGUMENT, cmzn_field_component_set_component_index(fc, 4));

	cmzn_field_component_destroy(&fc);
	cmzn_field_destroy(&f2);
	cmzn_field_destroy(&f1);
}

TEST(zincFieldComponent, single_valid_args)
{
	ZincTestSetupCpp zinc;

	const double sourceValues[] = { 2.0, 4.0, 6.0 };
	FieldConstant f1 = zinc.fm.createFieldConstant(3, sourceValues);
	EXPECT_TRUE(f1.isValid());

	const int component_index = 2;
	FieldComponent f2 = zinc.fm.createFieldComponent(f1, component_index);
	EXPECT_TRUE(f2.isValid());
	EXPECT_EQ(1, f2.getNumberOfComponents());

	FieldComponent isComponent = f2.castComponent();
	EXPECT_TRUE(isComponent.isValid());

	Fieldcache cache = zinc.fm.createFieldcache();
	double value = 0.0;
	EXPECT_EQ(OK, f2.evaluateReal(cache, 1, &value));
	EXPECT_EQ(sourceValues[component_index - 1], value);

	int temp_component_index = f2.getComponentIndex();
	EXPECT_EQ(component_index, temp_component_index);
	const int new_component_index = 3;
	EXPECT_EQ(OK, f2.setComponentIndex(new_component_index));
	temp_component_index = f2.getComponentIndex();
	EXPECT_EQ(new_component_index, temp_component_index);
	EXPECT_EQ(OK, f2.evaluateReal(cache, 1, &value));
	EXPECT_EQ(sourceValues[new_component_index - 1], value);
}

TEST(zincFieldComponent, single_invalid_args)
{
	ZincTestSetupCpp zinc;

	const double sourceValues[] = { 2.0, 4.0, 6.0 };
	const int component_index = 2;
	FieldConstant f1 = zinc.fm.createFieldConstant(3, sourceValues);
	EXPECT_TRUE(f1.isValid());

	FieldComponent notComponent = f1.castComponent();
	EXPECT_FALSE(notComponent.isValid());

	EXPECT_FALSE(zinc.fm.createFieldComponent(Field(), component_index).isValid());
	EXPECT_FALSE(zinc.fm.createFieldComponent(f1, 0).isValid());
	EXPECT_FALSE(zinc.fm.createFieldComponent(f1, 4).isValid());

	FieldComponent f2 = zinc.fm.createFieldComponent(f1, component_index);
	EXPECT_TRUE(f2.isValid());
	EXPECT_EQ(1, f2.getNumberOfComponents());

	EXPECT_EQ(0, notComponent.getComponentIndex());
	EXPECT_EQ(ERROR_ARGUMENT, f2.setComponentIndex(0));
	EXPECT_EQ(ERROR_ARGUMENT, f2.setComponentIndex(4));
}

TEST(zincFieldComponent, multiple_valid_args)
{
	ZincTestSetupCpp zinc;

	const double sourceValues[] = { 2.0, 4.0, 6.0 };
	FieldConstant f1 = zinc.fm.createFieldConstant(3, sourceValues);
	EXPECT_TRUE(f1.isValid());

	const int sourceComponentIndexes[2] = { 2, 1 };
	FieldComponent f2 = zinc.fm.createFieldComponent(f1, 2, sourceComponentIndexes);
	EXPECT_TRUE(f2.isValid());
	EXPECT_EQ(2, f2.getNumberOfComponents());

	FieldComponent isComponent = f2.castComponent();
	EXPECT_TRUE(isComponent.isValid());

	Fieldcache cache = zinc.fm.createFieldcache();
	double outValues[2];
	EXPECT_EQ(OK, f2.evaluateReal(cache, 2, outValues));
	for (int i = 0; i < 2; ++i)
		EXPECT_EQ(sourceValues[sourceComponentIndexes[i] - 1], outValues[i]);

	int temp_component_index = f2.getSourceComponentIndex(2);
	EXPECT_EQ(sourceComponentIndexes[1], temp_component_index);
	const int new_component_index = 3;
	EXPECT_EQ(OK, f2.setSourceComponentIndex(2, new_component_index));
	temp_component_index = f2.getSourceComponentIndex(2);
	EXPECT_EQ(new_component_index, temp_component_index);
	EXPECT_EQ(OK, f2.evaluateReal(cache, 2, outValues));
	EXPECT_EQ(sourceValues[sourceComponentIndexes[0] - 1], outValues[0]);
	EXPECT_EQ(sourceValues[new_component_index - 1], outValues[1]);
}

TEST(zincFieldComponent, multiple_invalid_args)
{
	ZincTestSetupCpp zinc;

	const double sourceValues[] = { 2.0, 4.0, 6.0 };
	FieldConstant f1 = zinc.fm.createFieldConstant(3, sourceValues);
	EXPECT_TRUE(f1.isValid());

	FieldComponent notComponent = f1.castComponent();
	EXPECT_FALSE(notComponent.isValid());

	const int sourceComponentIndexes[2] = { 2, 1 };
	EXPECT_FALSE(zinc.fm.createFieldComponent(Field(), 2, sourceComponentIndexes).isValid());
	EXPECT_FALSE(zinc.fm.createFieldComponent(f1, 0, sourceComponentIndexes).isValid());
	EXPECT_FALSE(zinc.fm.createFieldComponent(f1, 2, 0).isValid());
	const int badSourceComponentIndexes1[2] = { 2, 0 };
	const int badSourceComponentIndexes2[2] = { 4, 1 };
	EXPECT_FALSE(zinc.fm.createFieldComponent(f1, 2, badSourceComponentIndexes1).isValid());
	EXPECT_FALSE(zinc.fm.createFieldComponent(f1, 2, badSourceComponentIndexes2).isValid());

	FieldComponent f2 = zinc.fm.createFieldComponent(f1, 2, sourceComponentIndexes);
	EXPECT_TRUE(f2.isValid());
	EXPECT_EQ(2, f2.getNumberOfComponents());
	
	EXPECT_EQ(0, notComponent.getSourceComponentIndex(1));
	EXPECT_EQ(0, f2.getSourceComponentIndex(0));
	EXPECT_EQ(0, f2.getSourceComponentIndex(3));
	EXPECT_EQ(ERROR_ARGUMENT, f2.setSourceComponentIndex(0, 1));
	EXPECT_EQ(ERROR_ARGUMENT, f2.setSourceComponentIndex(3, 3));
	EXPECT_EQ(ERROR_ARGUMENT, f2.setSourceComponentIndex(1, 0));
	EXPECT_EQ(ERROR_ARGUMENT, f2.setSourceComponentIndex(2, 4));
}

// Test repeated source fields are merged in Concatenate field
TEST(ZincFieldConcatenate, repeatedSourceFields)
{
	ZincTestSetupCpp zinc;

	const double zeroValue = 0.0;
	Field zero = zinc.fm.createFieldConstant(1, &zeroValue);
	EXPECT_TRUE(zero.isValid());
	const double oneValue = 1.0;
	Field one = zinc.fm.createFieldConstant(1, &oneValue);
	EXPECT_TRUE(one.isValid());
	Field componentFields[] = { zero, one, zero };
	Field concatenate = zinc.fm.createFieldConcatenate(3, componentFields);
	EXPECT_TRUE(concatenate.isValid());

	// check two instances of zero have been merged
	EXPECT_EQ(2, concatenate.getNumberOfSourceFields());
	EXPECT_EQ(zero, concatenate.getSourceField(1));
	EXPECT_EQ(one, concatenate.getSourceField(2));

	// check evaluation
	Fieldcache cache = zinc.fm.createFieldcache();
	EXPECT_TRUE(cache.isValid());
	double values[3];
	EXPECT_EQ(RESULT_OK, concatenate.evaluateReal(cache, 3, values));
	EXPECT_DOUBLE_EQ(0.0, values[0]);
	EXPECT_DOUBLE_EQ(1.0, values[1]);
	EXPECT_DOUBLE_EQ(0.0, values[2]);
}
