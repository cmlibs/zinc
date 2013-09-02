
#include <gtest/gtest.h>

#include <zinc/zincconfigure.h>
#include <zinc/status.h>
#include <zinc/core.h>
#include <zinc/graphic.h>
#include <zinc/graphicsfilter.h>

#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"
#include "zinc/graphicsfilter.hpp"

TEST(cmzn_graphics_filter_module_api, valid_args)
{
	ZincTestSetup zinc;

	cmzn_graphics_filter_module_id gfm = cmzn_graphics_module_get_filter_module(zinc.gm);
	EXPECT_NE(static_cast<cmzn_graphics_filter_module *>(0), gfm);

	int result = cmzn_graphics_filter_module_begin_change(gfm);
	EXPECT_EQ(CMZN_OK, result);

	cmzn_graphics_filter_id filter = cmzn_graphics_filter_module_create_filter_visibility_flags(gfm);
	EXPECT_NE(static_cast<cmzn_graphics_filter *>(0), filter);

	result = cmzn_graphics_filter_set_name(filter, "default");
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_graphics_filter_module_end_change(gfm);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_graphics_filter_module_set_default_filter(gfm, filter);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_graphics_filter_set_managed(filter, true);
	EXPECT_EQ(CMZN_OK, result);

	cmzn_graphics_filter_destroy(&filter);

	filter = cmzn_graphics_filter_module_find_filter_by_name(gfm, "default");
	EXPECT_NE(static_cast<cmzn_graphics_filter *>(0), filter);

	cmzn_graphics_filter_destroy(&filter);

	filter = cmzn_graphics_filter_module_get_default_filter(gfm);
	EXPECT_NE(static_cast<cmzn_graphics_filter *>(0), filter);

	cmzn_graphics_filter_destroy(&filter);

	filter = cmzn_graphics_filter_module_create_filter_graphic_name(gfm, "lines");
	EXPECT_NE(static_cast<cmzn_graphics_filter *>(0), filter);

	cmzn_graphics_filter_destroy(&filter);

	filter = cmzn_graphics_filter_module_create_filter_graphic_type(gfm, CMZN_GRAPHIC_POINTS);
	EXPECT_NE(static_cast<cmzn_graphics_filter *>(0), filter);

	cmzn_graphics_filter_destroy(&filter);

	filter = cmzn_graphics_filter_module_create_filter_region(gfm, zinc.root_region);
	EXPECT_NE(static_cast<cmzn_graphics_filter *>(0), filter);

	cmzn_graphics_filter_destroy(&filter);

	filter = cmzn_graphics_filter_module_create_filter_operator_and(gfm);
	EXPECT_NE(static_cast<cmzn_graphics_filter *>(0), filter);

	cmzn_graphics_filter_destroy(&filter);

	filter = cmzn_graphics_filter_module_create_filter_operator_or(gfm);
	EXPECT_NE(static_cast<cmzn_graphics_filter *>(0), filter);

	cmzn_graphics_filter_destroy(&filter);

	cmzn_graphics_filter_module_destroy(&gfm);
}

TEST(cmzn_graphics_filter_module_api, valid_args_cpp)
{
	ZincTestSetupCpp zinc;

	GraphicsFilterModule gfm = zinc.gm.getFilterModule();
	EXPECT_TRUE(gfm.isValid());

	int result = gfm.beginChange();
	EXPECT_EQ(CMZN_OK, result);

	GraphicsFilter filter = gfm.createFilterVisibilityFlags();
	EXPECT_TRUE(filter.isValid());

	result = filter.setName("default");
	EXPECT_EQ(CMZN_OK, result);

	result = gfm.endChange();
	EXPECT_EQ(CMZN_OK, result);

	result = gfm.setDefaultFilter(filter);
	EXPECT_EQ(CMZN_OK, result);

	result = filter.setManaged(true);
	EXPECT_EQ(CMZN_OK, result);

	filter = gfm.findFilterByName("default");
	EXPECT_TRUE(filter.isValid());

	filter = gfm.getDefaultFilter();
	EXPECT_TRUE(filter.isValid());

	filter =  gfm.createFilterGraphicName("lines");
	EXPECT_TRUE(filter.isValid());

	filter = gfm.createFilterGraphicType(Graphic::POINTS);
	EXPECT_TRUE(filter.isValid());

	filter = gfm.createFilterRegion(zinc.root_region);
	EXPECT_TRUE(filter.isValid());

	filter = gfm.createFilterOperatorAnd();
	EXPECT_TRUE(filter.isValid());

	filter = gfm.createFilterOperatorOr();
	EXPECT_TRUE(filter.isValid());
}

TEST(cmzn_graphics_filter_api, valid_args)
{
	ZincTestSetup zinc;

	cmzn_graphics_filter_module_id gfm = cmzn_graphics_module_get_filter_module(zinc.gm);
	EXPECT_NE(static_cast<cmzn_graphics_filter_module *>(0), gfm);

	cmzn_graphics_filter_id filter = cmzn_graphics_filter_module_get_default_filter(gfm);
	EXPECT_NE(static_cast<cmzn_graphics_filter *>(0), filter);

	int result = cmzn_graphics_filter_set_name(filter, "visibility_flag");
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_graphics_filter_set_managed(filter, true);
	EXPECT_EQ(CMZN_OK, result);
	EXPECT_TRUE(cmzn_graphics_filter_is_managed(filter));

	EXPECT_FALSE(cmzn_graphics_filter_is_inverse(filter));
	EXPECT_EQ(CMZN_OK, result = cmzn_graphics_filter_set_inverse(filter, true));
	EXPECT_TRUE(cmzn_graphics_filter_is_inverse(filter));
	EXPECT_EQ(CMZN_OK, result = cmzn_graphics_filter_set_inverse(filter, false));

	cmzn_graphics_filter_id graphic_type_filter1 = cmzn_graphics_filter_module_create_filter_graphic_type(gfm, CMZN_GRAPHIC_POINTS);
	EXPECT_NE(static_cast<cmzn_graphics_filter *>(0), graphic_type_filter1);
	EXPECT_FALSE(cmzn_graphics_filter_is_managed(graphic_type_filter1));
	cmzn_graphics_filter_id graphic_type_filter2 = cmzn_graphics_filter_module_create_filter_graphic_type(gfm, CMZN_GRAPHIC_LINES);
	EXPECT_NE(static_cast<cmzn_graphics_filter *>(0), graphic_type_filter2);
	cmzn_graphics_filter_id domain_type_filter1 = cmzn_graphics_filter_module_create_filter_domain_type(gfm, CMZN_FIELD_DOMAIN_NODES);
	EXPECT_NE(static_cast<cmzn_graphics_filter *>(0), domain_type_filter1);
	cmzn_graphics_filter_id domain_type_filter2 = cmzn_graphics_filter_module_create_filter_domain_type(gfm, CMZN_FIELD_DOMAIN_MESH_1D);
	EXPECT_NE(static_cast<cmzn_graphics_filter *>(0), domain_type_filter2);

	cmzn_graphic_id graphic = cmzn_scene_create_graphic(zinc.scene, CMZN_GRAPHIC_LINES);
	EXPECT_NE(static_cast<cmzn_graphic *>(0), graphic);

	result = cmzn_graphics_filter_evaluate_graphic(graphic_type_filter1, graphic);
	EXPECT_EQ(0, result);
	result = cmzn_graphics_filter_evaluate_graphic(graphic_type_filter2, graphic);
	EXPECT_EQ(1, result);
	result = cmzn_graphics_filter_evaluate_graphic(domain_type_filter1, graphic);
	EXPECT_EQ(0, result);
	result = cmzn_graphics_filter_evaluate_graphic(domain_type_filter2, graphic);
	EXPECT_EQ(1, result);

	result = cmzn_graphics_filter_evaluate_graphic(filter, graphic);
	EXPECT_EQ(1, result);

	cmzn_graphics_filter_id and_filter = cmzn_graphics_filter_module_create_filter_operator_and(gfm);
	EXPECT_NE(static_cast<cmzn_graphics_filter *>(0), and_filter);

	cmzn_graphics_filter_id or_filter = cmzn_graphics_filter_module_create_filter_operator_or(gfm);
	EXPECT_NE(static_cast<cmzn_graphics_filter *>(0), or_filter);

	cmzn_graphics_filter_operator_id and_operator = cmzn_graphics_filter_cast_operator(and_filter);
	EXPECT_NE(static_cast<cmzn_graphics_filter_operator *>(0), and_operator);

	cmzn_graphics_filter_operator_id or_operator = cmzn_graphics_filter_cast_operator(or_filter);
	EXPECT_NE(static_cast<cmzn_graphics_filter_operator *>(0), or_operator);

	result = cmzn_graphics_filter_operator_append_operand(and_operator, graphic_type_filter1);
	EXPECT_EQ(1, result);

	cmzn_graphics_filter_operator_insert_operand_before(and_operator, filter, graphic_type_filter1);
	EXPECT_EQ(1, result);

	result = cmzn_graphics_filter_operator_set_operand_is_active(and_operator, filter, 1);
	EXPECT_EQ(1, result);

	result = cmzn_graphics_filter_operator_get_operand_is_active(and_operator, filter);
	EXPECT_EQ(1, result);

	result = cmzn_graphics_filter_operator_set_operand_is_active(and_operator, graphic_type_filter1, 1);
	EXPECT_EQ(1, result);

	result = cmzn_graphics_filter_operator_get_operand_is_active(and_operator, graphic_type_filter1);
	EXPECT_EQ(1, result);

	cmzn_graphics_filter_id temp_filter = cmzn_graphics_filter_operator_get_first_operand(
		and_operator);
	result = (filter == temp_filter);
	EXPECT_EQ(1, result);

	cmzn_graphics_filter_id temp_filter2 = cmzn_graphics_filter_operator_get_next_operand(
		and_operator, temp_filter);
	result = (graphic_type_filter1 == temp_filter2);
	EXPECT_EQ(1, result);

	result = cmzn_graphics_filter_operator_append_operand(or_operator, graphic_type_filter1);
	EXPECT_EQ(1, result);

	cmzn_graphics_filter_operator_insert_operand_before(or_operator, filter, graphic_type_filter1);
	EXPECT_EQ(1, result);

	result = cmzn_graphics_filter_evaluate_graphic(and_filter, graphic);
	EXPECT_EQ(0, result);

	result = cmzn_graphics_filter_evaluate_graphic(or_filter, graphic);
	EXPECT_EQ(1, result);

	result = cmzn_graphics_filter_operator_remove_operand(
		or_operator, graphic_type_filter1);
	EXPECT_EQ(1, result);

	cmzn_graphics_filter_destroy(&temp_filter2);

	cmzn_graphics_filter_destroy(&temp_filter);

	cmzn_graphic_destroy(&graphic);

	cmzn_graphics_filter_operator_destroy(&and_operator);

	cmzn_graphics_filter_operator_destroy(&or_operator);

	cmzn_graphics_filter_destroy(&and_filter);

	cmzn_graphics_filter_destroy(&or_filter);

	cmzn_graphics_filter_destroy(&graphic_type_filter1);
	cmzn_graphics_filter_destroy(&graphic_type_filter2);
	cmzn_graphics_filter_destroy(&domain_type_filter1);
	cmzn_graphics_filter_destroy(&domain_type_filter2);

	cmzn_graphics_filter_destroy(&filter);

	cmzn_graphics_filter_module_destroy(&gfm);
}

TEST(cmzn_graphics_filter_api, valid_args_cpp)
{
	ZincTestSetupCpp zinc;

	GraphicsFilterModule gfm = zinc.gm.getFilterModule();
	EXPECT_TRUE(gfm.isValid());

	GraphicsFilter filter = gfm.getDefaultFilter();
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

	GraphicsFilter graphic_type_filter1 = gfm.createFilterGraphicType(Graphic::POINTS);
	EXPECT_TRUE(graphic_type_filter1.isValid());
	GraphicsFilter graphic_type_filter2 = gfm.createFilterGraphicType(Graphic::LINES);
	EXPECT_TRUE(graphic_type_filter2.isValid());
	GraphicsFilter domain_type_filter1 = gfm.createFilterDomainType(Field::DOMAIN_NODES);
	EXPECT_TRUE(domain_type_filter1.isValid());
	GraphicsFilter domain_type_filter2 = gfm.createFilterDomainType(Field::DOMAIN_MESH_1D);
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

	GraphicsFilterOperator and_operator = gfm.createFilterOperatorAnd();
	EXPECT_TRUE(and_operator.isValid());

	GraphicsFilterOperator or_operator = gfm.createFilterOperatorOr();
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

	GraphicsFilter temp_filter = and_operator.getFirstOperand();
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
