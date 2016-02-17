/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include <zinc/status.h>
#include <zinc/core.h>
#include <zinc/graphics.h>
#include <zinc/scenefilter.h>

#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"
#include "opencmiss/zinc/scenefilter.hpp"

TEST(cmzn_scenefiltermodule_api, valid_args)
{
	ZincTestSetup zinc;

	cmzn_scenefiltermodule_id sfm = cmzn_context_get_scenefiltermodule(zinc.context);
	EXPECT_NE(static_cast<cmzn_scenefiltermodule *>(0), sfm);

	int result = cmzn_scenefiltermodule_begin_change(sfm);
	EXPECT_EQ(CMZN_OK, result);

	cmzn_scenefilter_id filter = cmzn_scenefiltermodule_create_scenefilter_visibility_flags(sfm);
	EXPECT_NE(static_cast<cmzn_scenefilter *>(0), filter);

	result = cmzn_scenefilter_set_name(filter, "new_default");
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_scenefiltermodule_end_change(sfm);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_scenefiltermodule_set_default_scenefilter(sfm, filter);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_scenefilter_set_managed(filter, true);
	EXPECT_EQ(CMZN_OK, result);

	cmzn_scenefilter_destroy(&filter);

	filter = cmzn_scenefiltermodule_find_scenefilter_by_name(sfm, "new_default");
	EXPECT_NE(static_cast<cmzn_scenefilter *>(0), filter);

	cmzn_scenefilter_destroy(&filter);

	filter = cmzn_scenefiltermodule_get_default_scenefilter(sfm);
	EXPECT_NE(static_cast<cmzn_scenefilter *>(0), filter);

	cmzn_scenefilter_destroy(&filter);

	filter = cmzn_scenefiltermodule_create_scenefilter_graphics_name(sfm, "lines");
	EXPECT_NE(static_cast<cmzn_scenefilter *>(0), filter);

	cmzn_scenefilter_destroy(&filter);

	filter = cmzn_scenefiltermodule_create_scenefilter_graphics_type(sfm, CMZN_GRAPHICS_TYPE_POINTS);
	EXPECT_NE(static_cast<cmzn_scenefilter *>(0), filter);

	cmzn_scenefilter_destroy(&filter);

	filter = cmzn_scenefiltermodule_create_scenefilter_region(sfm, zinc.root_region);
	EXPECT_NE(static_cast<cmzn_scenefilter *>(0), filter);

	cmzn_scenefilter_destroy(&filter);

	cmzn_scenefilter_operator_id filterOperator;

	filter = cmzn_scenefiltermodule_create_scenefilter_operator_and(sfm);
	EXPECT_NE(static_cast<cmzn_scenefilter *>(0), filter);

	filterOperator = cmzn_scenefilter_cast_operator(filter);
	EXPECT_NE(static_cast<cmzn_scenefilter_operator *>(0), filterOperator);
	cmzn_scenefilter_operator_destroy(&filterOperator);

	cmzn_scenefilter_destroy(&filter);

	filter = cmzn_scenefiltermodule_create_scenefilter_operator_or(sfm);
	EXPECT_NE(static_cast<cmzn_scenefilter *>(0), filter);

	filterOperator = cmzn_scenefilter_cast_operator(filter);
	EXPECT_NE(static_cast<cmzn_scenefilter_operator *>(0), filterOperator);
	cmzn_scenefilter_operator_destroy(&filterOperator);

	cmzn_scenefilter_destroy(&filter);

	cmzn_scenefiltermodule_destroy(&sfm);
}

TEST(cmzn_scenefiltermodule_api, valid_args_cpp)
{
	ZincTestSetupCpp zinc;

	Scenefiltermodule sfm = zinc.context.getScenefiltermodule();
	EXPECT_TRUE(sfm.isValid());

	int result = sfm.beginChange();
	EXPECT_EQ(CMZN_OK, result);

	Scenefilter filter = sfm.createScenefilterVisibilityFlags();
	EXPECT_TRUE(filter.isValid());

	result = filter.setName("new_default");
	EXPECT_EQ(CMZN_OK, result);

	result = sfm.endChange();
	EXPECT_EQ(CMZN_OK, result);

	result = sfm.setDefaultScenefilter(filter);
	EXPECT_EQ(CMZN_OK, result);

	result = filter.setManaged(true);
	EXPECT_EQ(CMZN_OK, result);

	filter = sfm.findScenefilterByName("new_default");
	EXPECT_TRUE(filter.isValid());

	filter = sfm.getDefaultScenefilter();
	EXPECT_TRUE(filter.isValid());

	filter =  sfm.createScenefilterGraphicsName("lines");
	EXPECT_TRUE(filter.isValid());

	filter = sfm.createScenefilterGraphicsType(Graphics::TYPE_POINTS);
	EXPECT_TRUE(filter.isValid());

	filter = sfm.createScenefilterRegion(zinc.root_region);
	EXPECT_TRUE(filter.isValid());

	ScenefilterOperator filterOperator;

	filter = sfm.createScenefilterOperatorAnd();
	EXPECT_TRUE(filter.isValid());
	filterOperator = filter.castOperator();
	EXPECT_TRUE(filterOperator.isValid());

	filter = sfm.createScenefilterOperatorOr();
	EXPECT_TRUE(filter.isValid());
	filterOperator = filter.castOperator();
	EXPECT_TRUE(filterOperator.isValid());
}

TEST(cmzn_scenefilter_api, valid_args)
{
	ZincTestSetup zinc;

	cmzn_scenefiltermodule_id sfm = cmzn_context_get_scenefiltermodule(zinc.context);
	EXPECT_NE(static_cast<cmzn_scenefiltermodule *>(0), sfm);

	cmzn_scenefilter_id filter = cmzn_scenefiltermodule_get_default_scenefilter(sfm);
	EXPECT_NE(static_cast<cmzn_scenefilter *>(0), filter);

	int result = cmzn_scenefilter_set_name(filter, "visibility_flag");
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_scenefilter_set_managed(filter, true);
	EXPECT_EQ(CMZN_OK, result);
	EXPECT_TRUE(cmzn_scenefilter_is_managed(filter));

	EXPECT_FALSE(cmzn_scenefilter_is_inverse(filter));
	EXPECT_EQ(CMZN_OK, result = cmzn_scenefilter_set_inverse(filter, true));
	EXPECT_TRUE(cmzn_scenefilter_is_inverse(filter));
	EXPECT_EQ(CMZN_OK, result = cmzn_scenefilter_set_inverse(filter, false));

	cmzn_scenefilter_id graphics_type_filter1 = cmzn_scenefiltermodule_create_scenefilter_graphics_type(sfm, CMZN_GRAPHICS_TYPE_POINTS);
	EXPECT_NE(static_cast<cmzn_scenefilter *>(0), graphics_type_filter1);
	EXPECT_FALSE(cmzn_scenefilter_is_managed(graphics_type_filter1));
	cmzn_scenefilter_id graphics_type_filter2 = cmzn_scenefiltermodule_create_scenefilter_graphics_type(sfm, CMZN_GRAPHICS_TYPE_LINES);
	EXPECT_NE(static_cast<cmzn_scenefilter *>(0), graphics_type_filter2);
	cmzn_scenefilter_id domain_type_filter1 = cmzn_scenefiltermodule_create_scenefilter_field_domain_type(sfm, CMZN_FIELD_DOMAIN_TYPE_NODES);
	EXPECT_NE(static_cast<cmzn_scenefilter *>(0), domain_type_filter1);
	cmzn_scenefilter_id domain_type_filter2 = cmzn_scenefiltermodule_create_scenefilter_field_domain_type(sfm, CMZN_FIELD_DOMAIN_TYPE_MESH1D);
	EXPECT_NE(static_cast<cmzn_scenefilter *>(0), domain_type_filter2);

	cmzn_graphics_id graphics = cmzn_scene_create_graphics(zinc.scene, CMZN_GRAPHICS_TYPE_LINES);
	EXPECT_NE(static_cast<cmzn_graphics *>(0), graphics);

	result = cmzn_scenefilter_evaluate_graphics(graphics_type_filter1, graphics);
	EXPECT_EQ(0, result);
	result = cmzn_scenefilter_evaluate_graphics(graphics_type_filter2, graphics);
	EXPECT_EQ(1, result);
	result = cmzn_scenefilter_evaluate_graphics(domain_type_filter1, graphics);
	EXPECT_EQ(0, result);
	result = cmzn_scenefilter_evaluate_graphics(domain_type_filter2, graphics);
	EXPECT_EQ(1, result);

	result = cmzn_scenefilter_evaluate_graphics(filter, graphics);
	EXPECT_EQ(1, result);

	cmzn_scenefilter_id and_filter = cmzn_scenefiltermodule_create_scenefilter_operator_and(sfm);
	EXPECT_NE(static_cast<cmzn_scenefilter *>(0), and_filter);

	cmzn_scenefilter_id or_filter = cmzn_scenefiltermodule_create_scenefilter_operator_or(sfm);
	EXPECT_NE(static_cast<cmzn_scenefilter *>(0), or_filter);

	cmzn_scenefilter_operator_id and_operator = cmzn_scenefilter_cast_operator(and_filter);
	EXPECT_NE(static_cast<cmzn_scenefilter_operator *>(0), and_operator);

	cmzn_scenefilter_operator_id or_operator = cmzn_scenefilter_cast_operator(or_filter);
	EXPECT_NE(static_cast<cmzn_scenefilter_operator *>(0), or_operator);

	result = cmzn_scenefilter_operator_append_operand(and_operator, graphics_type_filter1);
	EXPECT_EQ(1, result);

	cmzn_scenefilter_operator_insert_operand_before(and_operator, filter, graphics_type_filter1);
	EXPECT_EQ(1, result);

	EXPECT_EQ(CMZN_OK, result = cmzn_scenefilter_operator_set_operand_active(and_operator, filter, false));
	EXPECT_FALSE(cmzn_scenefilter_operator_is_operand_active(and_operator, filter));
	EXPECT_EQ(CMZN_OK, result = cmzn_scenefilter_operator_set_operand_active(and_operator, filter, true));
	EXPECT_TRUE(cmzn_scenefilter_operator_is_operand_active(and_operator, filter));

	EXPECT_EQ(CMZN_OK, result = cmzn_scenefilter_operator_set_operand_active(and_operator, graphics_type_filter1, true));
	EXPECT_TRUE(cmzn_scenefilter_operator_is_operand_active(and_operator, graphics_type_filter1));

	cmzn_scenefilter_id temp_filter = cmzn_scenefilter_operator_get_first_operand(
		and_operator);
	result = (filter == temp_filter);
	EXPECT_EQ(1, result);

	cmzn_scenefilter_id temp_filter2 = cmzn_scenefilter_operator_get_next_operand(
		and_operator, temp_filter);
	result = (graphics_type_filter1 == temp_filter2);
	EXPECT_EQ(1, result);

	result = cmzn_scenefilter_operator_append_operand(or_operator, graphics_type_filter1);
	EXPECT_EQ(1, result);

	cmzn_scenefilter_operator_insert_operand_before(or_operator, filter, graphics_type_filter1);
	EXPECT_EQ(1, result);

	result = cmzn_scenefilter_evaluate_graphics(and_filter, graphics);
	EXPECT_EQ(0, result);

	result = cmzn_scenefilter_evaluate_graphics(or_filter, graphics);
	EXPECT_EQ(1, result);

	result = cmzn_scenefilter_operator_remove_operand(
		or_operator, graphics_type_filter1);
	EXPECT_EQ(1, result);

	cmzn_scenefilter_destroy(&temp_filter2);

	cmzn_scenefilter_destroy(&temp_filter);

	cmzn_graphics_destroy(&graphics);

	cmzn_scenefilter_operator_destroy(&and_operator);

	cmzn_scenefilter_operator_destroy(&or_operator);

	cmzn_scenefilter_destroy(&and_filter);

	cmzn_scenefilter_destroy(&or_filter);

	cmzn_scenefilter_destroy(&graphics_type_filter1);
	cmzn_scenefilter_destroy(&graphics_type_filter2);
	cmzn_scenefilter_destroy(&domain_type_filter1);
	cmzn_scenefilter_destroy(&domain_type_filter2);

	cmzn_scenefilter_destroy(&filter);

	cmzn_scenefiltermodule_destroy(&sfm);
}

TEST(cmzn_scenefilter_api, valid_args_cpp)
{
	ZincTestSetupCpp zinc;

	Scenefiltermodule sfm = zinc.context.getScenefiltermodule();
	EXPECT_TRUE(sfm.isValid());

	Scenefilter filter = sfm.getDefaultScenefilter();
	EXPECT_TRUE(filter.isValid());

	int result = filter.setName("visibility_flag");
	EXPECT_EQ(CMZN_OK, result);

	result = filter.setManaged(true);
	EXPECT_EQ(CMZN_OK, result);
	EXPECT_TRUE(filter.isManaged());

	EXPECT_FALSE(filter.isInverse());
	EXPECT_EQ(CMZN_OK, result = filter.setInverse(true));
	EXPECT_TRUE(filter.isInverse());
	EXPECT_EQ(CMZN_OK, result = filter.setInverse(false));

	Scenefilter graphics_type_filter1 = sfm.createScenefilterGraphicsType(Graphics::TYPE_POINTS);
	EXPECT_TRUE(graphics_type_filter1.isValid());
	Scenefilter graphics_type_filter2 = sfm.createScenefilterGraphicsType(Graphics::TYPE_LINES);
	EXPECT_TRUE(graphics_type_filter2.isValid());
	Scenefilter domain_type_filter1 = sfm.createScenefilterFieldDomainType(Field::DOMAIN_TYPE_NODES);
	EXPECT_TRUE(domain_type_filter1.isValid());
	Scenefilter domain_type_filter2 = sfm.createScenefilterFieldDomainType(Field::DOMAIN_TYPE_MESH1D);
	EXPECT_TRUE(domain_type_filter2.isValid());

	Graphics graphics = zinc.scene.createGraphics(Graphics::TYPE_LINES);
	EXPECT_TRUE(graphics.isValid());

	result = graphics_type_filter1.evaluateGraphics(graphics);
	EXPECT_EQ(0, result);
	result = graphics_type_filter2.evaluateGraphics(graphics);
	EXPECT_EQ(1, result);
	result = domain_type_filter1.evaluateGraphics(graphics);
	EXPECT_EQ(0, result);
	result = domain_type_filter2.evaluateGraphics(graphics);
	EXPECT_EQ(1, result);

	result = filter.evaluateGraphics(graphics);
	EXPECT_EQ(1, result);

	ScenefilterOperator and_operator = sfm.createScenefilterOperatorAnd();
	EXPECT_TRUE(and_operator.isValid());

	ScenefilterOperator or_operator = sfm.createScenefilterOperatorOr();
	EXPECT_TRUE(or_operator.isValid());

	result = and_operator.appendOperand(graphics_type_filter1);
	EXPECT_EQ(1, result);

	result = and_operator.insertOperandBefore(filter, graphics_type_filter1);
	EXPECT_EQ(1, result);

	EXPECT_EQ(OK, result = and_operator.setOperandActive(filter, false));
	EXPECT_FALSE(and_operator.isOperandActive(filter));
	EXPECT_EQ(OK, result = and_operator.setOperandActive(filter, true));
	EXPECT_TRUE(and_operator.isOperandActive(filter));

	EXPECT_EQ(OK, result = and_operator.setOperandActive(graphics_type_filter1, true));
	EXPECT_TRUE(and_operator.isOperandActive(graphics_type_filter1));

	Scenefilter temp_filter = and_operator.getFirstOperand();
	result = (filter.getId() == temp_filter.getId());
	EXPECT_EQ(1, result);

	temp_filter = and_operator.getNextOperand(temp_filter);
	result = (graphics_type_filter1.getId() == temp_filter.getId());
	EXPECT_EQ(1, result);

	result = or_operator.appendOperand(graphics_type_filter1);
	EXPECT_EQ(1, result);

	result = or_operator.insertOperandBefore(filter, graphics_type_filter1);
	EXPECT_EQ(1, result);

	result = and_operator.evaluateGraphics(graphics);
	EXPECT_EQ(0, result);

	result = or_operator.evaluateGraphics(graphics);
	EXPECT_EQ(1, result);

	result = or_operator.removeOperand(graphics_type_filter1);
	EXPECT_EQ(1, result);
}
