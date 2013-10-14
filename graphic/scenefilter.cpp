
#include <gtest/gtest.h>

#include <zinc/zincconfigure.h>
#include <zinc/status.h>
#include <zinc/core.h>
#include <zinc/graphic.h>
#include <zinc/scenefilter.h>

#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"
#include "zinc/scenefilter.hpp"

TEST(cmzn_scenefiltermodule_api, valid_args)
{
	ZincTestSetup zinc;

	cmzn_scenefiltermodule_id sfm = cmzn_context_get_scenefiltermodule(zinc.context);
	EXPECT_NE(static_cast<cmzn_scenefiltermodule *>(0), sfm);

	int result = cmzn_scenefiltermodule_begin_change(sfm);
	EXPECT_EQ(CMZN_OK, result);

	cmzn_scenefilter_id filter = cmzn_scenefiltermodule_create_scenefilter_visibility_flags(sfm);
	EXPECT_NE(static_cast<cmzn_scenefilter *>(0), filter);

	result = cmzn_scenefilter_set_name(filter, "default");
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_scenefiltermodule_end_change(sfm);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_scenefiltermodule_set_default_scenefilter(sfm, filter);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_scenefilter_set_managed(filter, true);
	EXPECT_EQ(CMZN_OK, result);

	cmzn_scenefilter_destroy(&filter);

	filter = cmzn_scenefiltermodule_find_scenefilter_by_name(sfm, "default");
	EXPECT_NE(static_cast<cmzn_scenefilter *>(0), filter);

	cmzn_scenefilter_destroy(&filter);

	filter = cmzn_scenefiltermodule_get_default_scenefilter(sfm);
	EXPECT_NE(static_cast<cmzn_scenefilter *>(0), filter);

	cmzn_scenefilter_destroy(&filter);

	filter = cmzn_scenefiltermodule_create_scenefilter_graphic_name(sfm, "lines");
	EXPECT_NE(static_cast<cmzn_scenefilter *>(0), filter);

	cmzn_scenefilter_destroy(&filter);

	filter = cmzn_scenefiltermodule_create_scenefilter_graphic_type(sfm, CMZN_GRAPHIC_POINTS);
	EXPECT_NE(static_cast<cmzn_scenefilter *>(0), filter);

	cmzn_scenefilter_destroy(&filter);

	filter = cmzn_scenefiltermodule_create_scenefilter_region(sfm, zinc.root_region);
	EXPECT_NE(static_cast<cmzn_scenefilter *>(0), filter);

	cmzn_scenefilter_destroy(&filter);

	filter = cmzn_scenefiltermodule_create_scenefilter_operator_and(sfm);
	EXPECT_NE(static_cast<cmzn_scenefilter *>(0), filter);

	cmzn_scenefilter_destroy(&filter);

	filter = cmzn_scenefiltermodule_create_scenefilter_operator_or(sfm);
	EXPECT_NE(static_cast<cmzn_scenefilter *>(0), filter);

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

	result = filter.setName("default");
	EXPECT_EQ(CMZN_OK, result);

	result = sfm.endChange();
	EXPECT_EQ(CMZN_OK, result);

	result = sfm.setDefaultScenefilter(filter);
	EXPECT_EQ(CMZN_OK, result);

	result = filter.setManaged(true);
	EXPECT_EQ(CMZN_OK, result);

	filter = sfm.findScenefilterByName("default");
	EXPECT_TRUE(filter.isValid());

	filter = sfm.getDefaultScenefilter();
	EXPECT_TRUE(filter.isValid());

	filter =  sfm.createScenefilterGraphicName("lines");
	EXPECT_TRUE(filter.isValid());

	filter = sfm.createScenefilterGraphicType(Graphic::POINTS);
	EXPECT_TRUE(filter.isValid());

	filter = sfm.createScenefilterRegion(zinc.root_region);
	EXPECT_TRUE(filter.isValid());

	filter = sfm.createScenefilterOperatorAnd();
	EXPECT_TRUE(filter.isValid());

	filter = sfm.createScenefilterOperatorOr();
	EXPECT_TRUE(filter.isValid());
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

	cmzn_scenefilter_id graphic_type_filter1 = cmzn_scenefiltermodule_create_scenefilter_graphic_type(sfm, CMZN_GRAPHIC_POINTS);
	EXPECT_NE(static_cast<cmzn_scenefilter *>(0), graphic_type_filter1);
	EXPECT_FALSE(cmzn_scenefilter_is_managed(graphic_type_filter1));
	cmzn_scenefilter_id graphic_type_filter2 = cmzn_scenefiltermodule_create_scenefilter_graphic_type(sfm, CMZN_GRAPHIC_LINES);
	EXPECT_NE(static_cast<cmzn_scenefilter *>(0), graphic_type_filter2);
	cmzn_scenefilter_id domain_type_filter1 = cmzn_scenefiltermodule_create_scenefilter_domain_type(sfm, CMZN_FIELD_DOMAIN_NODES);
	EXPECT_NE(static_cast<cmzn_scenefilter *>(0), domain_type_filter1);
	cmzn_scenefilter_id domain_type_filter2 = cmzn_scenefiltermodule_create_scenefilter_domain_type(sfm, CMZN_FIELD_DOMAIN_MESH_1D);
	EXPECT_NE(static_cast<cmzn_scenefilter *>(0), domain_type_filter2);

	cmzn_graphic_id graphic = cmzn_scene_create_graphic(zinc.scene, CMZN_GRAPHIC_LINES);
	EXPECT_NE(static_cast<cmzn_graphic *>(0), graphic);

	result = cmzn_scenefilter_evaluate_graphic(graphic_type_filter1, graphic);
	EXPECT_EQ(0, result);
	result = cmzn_scenefilter_evaluate_graphic(graphic_type_filter2, graphic);
	EXPECT_EQ(1, result);
	result = cmzn_scenefilter_evaluate_graphic(domain_type_filter1, graphic);
	EXPECT_EQ(0, result);
	result = cmzn_scenefilter_evaluate_graphic(domain_type_filter2, graphic);
	EXPECT_EQ(1, result);

	result = cmzn_scenefilter_evaluate_graphic(filter, graphic);
	EXPECT_EQ(1, result);

	cmzn_scenefilter_id and_filter = cmzn_scenefiltermodule_create_scenefilter_operator_and(sfm);
	EXPECT_NE(static_cast<cmzn_scenefilter *>(0), and_filter);

	cmzn_scenefilter_id or_filter = cmzn_scenefiltermodule_create_scenefilter_operator_or(sfm);
	EXPECT_NE(static_cast<cmzn_scenefilter *>(0), or_filter);

	cmzn_scenefilter_operator_id and_operator = cmzn_scenefilter_cast_operator(and_filter);
	EXPECT_NE(static_cast<cmzn_scenefilter_operator *>(0), and_operator);

	cmzn_scenefilter_operator_id or_operator = cmzn_scenefilter_cast_operator(or_filter);
	EXPECT_NE(static_cast<cmzn_scenefilter_operator *>(0), or_operator);

	result = cmzn_scenefilter_operator_append_operand(and_operator, graphic_type_filter1);
	EXPECT_EQ(1, result);

	cmzn_scenefilter_operator_insert_operand_before(and_operator, filter, graphic_type_filter1);
	EXPECT_EQ(1, result);

	result = cmzn_scenefilter_operator_set_operand_is_active(and_operator, filter, 1);
	EXPECT_EQ(1, result);

	result = cmzn_scenefilter_operator_get_operand_is_active(and_operator, filter);
	EXPECT_EQ(1, result);

	result = cmzn_scenefilter_operator_set_operand_is_active(and_operator, graphic_type_filter1, 1);
	EXPECT_EQ(1, result);

	result = cmzn_scenefilter_operator_get_operand_is_active(and_operator, graphic_type_filter1);
	EXPECT_EQ(1, result);

	cmzn_scenefilter_id temp_filter = cmzn_scenefilter_operator_get_first_operand(
		and_operator);
	result = (filter == temp_filter);
	EXPECT_EQ(1, result);

	cmzn_scenefilter_id temp_filter2 = cmzn_scenefilter_operator_get_next_operand(
		and_operator, temp_filter);
	result = (graphic_type_filter1 == temp_filter2);
	EXPECT_EQ(1, result);

	result = cmzn_scenefilter_operator_append_operand(or_operator, graphic_type_filter1);
	EXPECT_EQ(1, result);

	cmzn_scenefilter_operator_insert_operand_before(or_operator, filter, graphic_type_filter1);
	EXPECT_EQ(1, result);

	result = cmzn_scenefilter_evaluate_graphic(and_filter, graphic);
	EXPECT_EQ(0, result);

	result = cmzn_scenefilter_evaluate_graphic(or_filter, graphic);
	EXPECT_EQ(1, result);

	result = cmzn_scenefilter_operator_remove_operand(
		or_operator, graphic_type_filter1);
	EXPECT_EQ(1, result);

	cmzn_scenefilter_destroy(&temp_filter2);

	cmzn_scenefilter_destroy(&temp_filter);

	cmzn_graphic_destroy(&graphic);

	cmzn_scenefilter_operator_destroy(&and_operator);

	cmzn_scenefilter_operator_destroy(&or_operator);

	cmzn_scenefilter_destroy(&and_filter);

	cmzn_scenefilter_destroy(&or_filter);

	cmzn_scenefilter_destroy(&graphic_type_filter1);
	cmzn_scenefilter_destroy(&graphic_type_filter2);
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

	Scenefilter graphic_type_filter1 = sfm.createScenefilterGraphicType(Graphic::POINTS);
	EXPECT_TRUE(graphic_type_filter1.isValid());
	Scenefilter graphic_type_filter2 = sfm.createScenefilterGraphicType(Graphic::LINES);
	EXPECT_TRUE(graphic_type_filter2.isValid());
	Scenefilter domain_type_filter1 = sfm.createScenefilterDomainType(Field::DOMAIN_NODES);
	EXPECT_TRUE(domain_type_filter1.isValid());
	Scenefilter domain_type_filter2 = sfm.createScenefilterDomainType(Field::DOMAIN_MESH_1D);
	EXPECT_TRUE(domain_type_filter2.isValid());

	Graphic graphic = zinc.scene.createGraphic(Graphic::LINES);
	EXPECT_TRUE(graphic.isValid());

	result = graphic_type_filter1.evaluateGraphic(graphic);
	EXPECT_EQ(0, result);
	result = graphic_type_filter2.evaluateGraphic(graphic);
	EXPECT_EQ(1, result);
	result = domain_type_filter1.evaluateGraphic(graphic);
	EXPECT_EQ(0, result);
	result = domain_type_filter2.evaluateGraphic(graphic);
	EXPECT_EQ(1, result);

	result = filter.evaluateGraphic(graphic);
	EXPECT_EQ(1, result);

	ScenefilterOperator and_operator = sfm.createScenefilterOperatorAnd();
	EXPECT_TRUE(and_operator.isValid());

	ScenefilterOperator or_operator = sfm.createScenefilterOperatorOr();
	EXPECT_TRUE(or_operator.isValid());

	result = and_operator.appendOperand(graphic_type_filter1);
	EXPECT_EQ(1, result);

	result = and_operator.insertOperandBefore(filter, graphic_type_filter1);
	EXPECT_EQ(1, result);

	result = and_operator.setOperandIsActive(filter, 1);
	EXPECT_EQ(1, result);

	result = and_operator.getOperandIsActive(filter);
	EXPECT_EQ(1, result);

	result = and_operator.setOperandIsActive(graphic_type_filter1, 1);
	EXPECT_EQ(1, result);

	result = and_operator.getOperandIsActive(graphic_type_filter1);
	EXPECT_EQ(1, result);

	Scenefilter temp_filter = and_operator.getFirstOperand();
	result = (filter.getId() == temp_filter.getId());
	EXPECT_EQ(1, result);

	temp_filter = and_operator.getNextOperand(temp_filter);
	result = (graphic_type_filter1.getId() == temp_filter.getId());
	EXPECT_EQ(1, result);

	result = or_operator.appendOperand(graphic_type_filter1);
	EXPECT_EQ(1, result);

	result = or_operator.insertOperandBefore(filter, graphic_type_filter1);
	EXPECT_EQ(1, result);

	result = and_operator.evaluateGraphic(graphic);
	EXPECT_EQ(0, result);

	result = or_operator.evaluateGraphic(graphic);
	EXPECT_EQ(1, result);

	result = or_operator.removeOperand(graphic_type_filter1);
	EXPECT_EQ(1, result);
}
