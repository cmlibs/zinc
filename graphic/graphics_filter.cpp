
#include <gtest/gtest.h>

#include <zinc/zincconfigure.h>
#include <zinc/status.h>
#include <zinc/core.h>
#include <zinc/graphic.h>
#include <zinc/graphicsfilter.h>

#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"
#include "zinc/graphicsfilter.hpp"

TEST(Cmiss_graphics_filter_module_api, valid_args)
{
	ZincTestSetup zinc;

	Cmiss_graphics_filter_module_id gfm = Cmiss_graphics_module_get_filter_module(zinc.gm);
	EXPECT_NE(static_cast<Cmiss_graphics_filter_module *>(0), gfm);

	int result = Cmiss_graphics_filter_module_begin_change(gfm);
	EXPECT_EQ(CMISS_OK, result);

	Cmiss_graphics_filter_id filter = Cmiss_graphics_filter_module_create_filter_visibility_flags(gfm);
	EXPECT_NE(static_cast<Cmiss_graphics_filter *>(0), filter);

	result = Cmiss_graphics_filter_set_name(filter, "default");
	EXPECT_EQ(CMISS_OK, result);

	result = Cmiss_graphics_filter_module_end_change(gfm);
	EXPECT_EQ(CMISS_OK, result);

	result = Cmiss_graphics_filter_module_set_default_filter(gfm, filter);
	EXPECT_EQ(CMISS_OK, result);

	result = Cmiss_graphics_filter_set_managed(filter, 1);
	EXPECT_EQ(CMISS_OK, result);

	Cmiss_graphics_filter_destroy(&filter);

	filter = Cmiss_graphics_filter_module_find_filter_by_name(gfm, "default");
	EXPECT_NE(static_cast<Cmiss_graphics_filter *>(0), filter);

	Cmiss_graphics_filter_destroy(&filter);

	filter = Cmiss_graphics_filter_module_get_default_filter(gfm);
	EXPECT_NE(static_cast<Cmiss_graphics_filter *>(0), filter);

	Cmiss_graphics_filter_destroy(&filter);

	filter = Cmiss_graphics_filter_module_create_filter_graphic_name(gfm, "lines");
	EXPECT_NE(static_cast<Cmiss_graphics_filter *>(0), filter);

	Cmiss_graphics_filter_destroy(&filter);

	filter = Cmiss_graphics_filter_module_create_filter_graphic_type(gfm, CMISS_GRAPHIC_NODE_POINTS);
	EXPECT_NE(static_cast<Cmiss_graphics_filter *>(0), filter);

	Cmiss_graphics_filter_destroy(&filter);

	filter = Cmiss_graphics_filter_module_create_filter_region(gfm, zinc.root_region);
	EXPECT_NE(static_cast<Cmiss_graphics_filter *>(0), filter);

	Cmiss_graphics_filter_destroy(&filter);

	filter = Cmiss_graphics_filter_module_create_filter_operator_and(gfm);
	EXPECT_NE(static_cast<Cmiss_graphics_filter *>(0), filter);

	Cmiss_graphics_filter_destroy(&filter);

	filter = Cmiss_graphics_filter_module_create_filter_operator_or(gfm);
	EXPECT_NE(static_cast<Cmiss_graphics_filter *>(0), filter);

	Cmiss_graphics_filter_destroy(&filter);

	Cmiss_graphics_filter_module_destroy(&gfm);
}

TEST(Cmiss_graphics_filter_module_api, valid_args_cpp)
{
	ZincTestSetupCpp zinc;

	GraphicsFilterModule gfm = zinc.gm.getFilterModule();
	EXPECT_EQ(true, gfm.isValid());

	int result = gfm.beginChange();
	EXPECT_EQ(CMISS_OK, result);

	GraphicsFilter filter = gfm.createFilterVisibilityFlags();
	EXPECT_EQ(true, filter.isValid());

	result = filter.setName("default");
	EXPECT_EQ(CMISS_OK, result);

	result = gfm.endChange();
	EXPECT_EQ(CMISS_OK, result);

	result = gfm.setDefaultFilter(filter);
	EXPECT_EQ(CMISS_OK, result);

	result = filter.setManaged(true);
	EXPECT_EQ(CMISS_OK, result);

	filter = gfm.findFilterByName("default");
	EXPECT_EQ(true, filter.isValid());

	filter = gfm.getDefaultFilter();
	EXPECT_EQ(true, filter.isValid());

	filter =  gfm.createFilterGraphicName("lines");
	EXPECT_EQ(true, filter.isValid());

	filter = gfm.createFilterGraphicType(Graphic::GRAPHIC_NODE_POINTS);
	EXPECT_EQ(true, filter.isValid());

	filter = gfm.createFilterRegion(zinc.root_region);
	EXPECT_EQ(true, filter.isValid());

	filter = gfm.createFilterOperatorAnd();
	EXPECT_EQ(true, filter.isValid());

	filter = gfm.createFilterOperatorOr();
	EXPECT_EQ(true, filter.isValid());
}

TEST(Cmiss_graphics_filter_api, valid_args)
{
	ZincTestSetup zinc;

	Cmiss_graphics_filter_module_id gfm = Cmiss_graphics_module_get_filter_module(zinc.gm);
	EXPECT_NE(static_cast<Cmiss_graphics_filter_module *>(0), gfm);

	Cmiss_graphics_filter_id filter = Cmiss_graphics_filter_module_get_default_filter(gfm);
	EXPECT_NE(static_cast<Cmiss_graphics_filter *>(0), filter);

	int result = Cmiss_graphics_filter_set_name(filter, "visibility_flag");
	EXPECT_EQ(CMISS_OK, result);

	result = Cmiss_graphics_filter_set_managed(filter, 1);
	EXPECT_EQ(CMISS_OK, result);

	result = Cmiss_graphics_filter_set_attribute_integer(filter,
		CMISS_GRAPHICS_FILTER_ATTRIBUTE_IS_INVERSE, 0);
	EXPECT_EQ(CMISS_OK, result);

	EXPECT_EQ(1, Cmiss_graphics_filter_is_managed(filter));

	result = Cmiss_graphics_filter_get_attribute_integer(filter,
		CMISS_GRAPHICS_FILTER_ATTRIBUTE_IS_INVERSE);
	EXPECT_EQ(0, result);

	Cmiss_graphics_filter_id graphic_type_filter = Cmiss_graphics_filter_module_create_filter_graphic_type(gfm, CMISS_GRAPHIC_POINT);
	EXPECT_NE(static_cast<Cmiss_graphics_filter *>(0), graphic_type_filter);

	Cmiss_graphic_id graphic = Cmiss_rendition_create_graphic(zinc.ren, CMISS_GRAPHIC_NODE_POINTS);
	EXPECT_NE(static_cast<Cmiss_graphic *>(0), graphic);

	result = Cmiss_graphics_filter_evaluate_graphic(graphic_type_filter, graphic);
	EXPECT_EQ(0, result);

	result = Cmiss_graphics_filter_evaluate_graphic(filter, graphic);
	EXPECT_EQ(1, result);

	Cmiss_graphics_filter_id and_filter = Cmiss_graphics_filter_module_create_filter_operator_and(gfm);
	EXPECT_NE(static_cast<Cmiss_graphics_filter *>(0), and_filter);

	Cmiss_graphics_filter_id or_filter = Cmiss_graphics_filter_module_create_filter_operator_or(gfm);
	EXPECT_NE(static_cast<Cmiss_graphics_filter *>(0), or_filter);

	Cmiss_graphics_filter_operator_id and_operator = Cmiss_graphics_filter_cast_operator(and_filter);
	EXPECT_NE(static_cast<Cmiss_graphics_filter_operator *>(0), and_operator);

	Cmiss_graphics_filter_operator_id or_operator = Cmiss_graphics_filter_cast_operator(or_filter);
	EXPECT_NE(static_cast<Cmiss_graphics_filter_operator *>(0), or_operator);

	result = Cmiss_graphics_filter_operator_append_operand(and_operator, graphic_type_filter);
	EXPECT_EQ(1, result);

	Cmiss_graphics_filter_operator_insert_operand_before(and_operator, filter, graphic_type_filter);
	EXPECT_EQ(1, result);

	result = Cmiss_graphics_filter_operator_set_operand_is_active(and_operator, filter, 1);
	EXPECT_EQ(1, result);

	result = Cmiss_graphics_filter_operator_get_operand_is_active(and_operator, filter);
	EXPECT_EQ(1, result);

	result = Cmiss_graphics_filter_operator_set_operand_is_active(and_operator, graphic_type_filter, 1);
	EXPECT_EQ(1, result);

	result = Cmiss_graphics_filter_operator_get_operand_is_active(and_operator, graphic_type_filter);
	EXPECT_EQ(1, result);

	Cmiss_graphics_filter_id temp_filter = Cmiss_graphics_filter_operator_get_first_operand(
		and_operator);
	result = (filter == temp_filter);
	EXPECT_EQ(1, result);

	Cmiss_graphics_filter_id temp_filter2 = Cmiss_graphics_filter_operator_get_next_operand(
		and_operator, temp_filter);
	result = (graphic_type_filter == temp_filter2);
	EXPECT_EQ(1, result);

	result = Cmiss_graphics_filter_operator_append_operand(or_operator, graphic_type_filter);
	EXPECT_EQ(1, result);

	Cmiss_graphics_filter_operator_insert_operand_before(or_operator, filter, graphic_type_filter);
	EXPECT_EQ(1, result);

	result = Cmiss_graphics_filter_evaluate_graphic(and_filter, graphic);
	EXPECT_EQ(0, result);

	result = Cmiss_graphics_filter_evaluate_graphic(or_filter, graphic);
	EXPECT_EQ(1, result);

	result = Cmiss_graphics_filter_operator_remove_operand(
		or_operator, graphic_type_filter);
	EXPECT_EQ(1, result);

	Cmiss_graphics_filter_destroy(&temp_filter2);

	Cmiss_graphics_filter_destroy(&temp_filter);

	Cmiss_graphic_destroy(&graphic);

	Cmiss_graphics_filter_operator_destroy(&and_operator);

	Cmiss_graphics_filter_operator_destroy(&or_operator);

	Cmiss_graphics_filter_destroy(&and_filter);

	Cmiss_graphics_filter_destroy(&or_filter);

	Cmiss_graphics_filter_destroy(&graphic_type_filter);

	Cmiss_graphics_filter_destroy(&filter);

	Cmiss_graphics_filter_module_destroy(&gfm);
}

TEST(Cmiss_graphics_filter_api, valid_args_cpp)
{
	ZincTestSetupCpp zinc;

	GraphicsFilterModule gfm = zinc.gm.getFilterModule();
	EXPECT_EQ(true, gfm.isValid());

	GraphicsFilter filter = gfm.getDefaultFilter();
	EXPECT_EQ(true, filter.isValid());

	int result = filter.setName("visibility_flag");
	EXPECT_EQ(CMISS_OK, result);

	result = filter.setManaged(true);
	EXPECT_EQ(CMISS_OK, result);

	result = filter.setAttributeInteger(filter.ATTRIBUTE_IS_INVERSE, 0);
	EXPECT_EQ(CMISS_OK, result);

	EXPECT_EQ(true, filter.isManaged());

	result = filter.getAttributeInteger(filter.ATTRIBUTE_IS_INVERSE);
	EXPECT_EQ(0, result);

	GraphicsFilter graphic_type_filter = gfm.createFilterGraphicType(Graphic::GRAPHIC_POINT);
	EXPECT_EQ(true, graphic_type_filter.isValid());

	Graphic graphic = zinc.ren.createGraphic(Graphic::GRAPHIC_NODE_POINTS);
	EXPECT_EQ(true, graphic.isValid());

	result = graphic_type_filter.evaluateGraphic(graphic);
	EXPECT_EQ(0, result);

	result = filter.evaluateGraphic(graphic);
	EXPECT_EQ(1, result);

	GraphicsFilterOperator and_operator = gfm.createFilterOperatorAnd();
	EXPECT_EQ(true, and_operator.isValid());

	GraphicsFilterOperator or_operator = gfm.createFilterOperatorOr();
	EXPECT_EQ(true, or_operator.isValid());

	result = and_operator.appendOperand(graphic_type_filter);
	EXPECT_EQ(1, result);

	result = and_operator.insertOperandBefore(filter, graphic_type_filter);
	EXPECT_EQ(1, result);

	result = and_operator.setOperandIsActive(filter, 1);
	EXPECT_EQ(1, result);

	result = and_operator.getOperandIsActive(filter);
	EXPECT_EQ(1, result);

	result = and_operator.setOperandIsActive(graphic_type_filter, 1);
	EXPECT_EQ(1, result);

	result = and_operator.getOperandIsActive(graphic_type_filter);
	EXPECT_EQ(1, result);

	GraphicsFilter temp_filter = and_operator.getFirstOperand();
	result = (filter.getId() == temp_filter.getId());
	EXPECT_EQ(1, result);

	temp_filter = and_operator.getNextOperand(temp_filter);
	result = (graphic_type_filter.getId() == temp_filter.getId());
	EXPECT_EQ(1, result);

	result = or_operator.appendOperand(graphic_type_filter);
	EXPECT_EQ(1, result);

	result = or_operator.insertOperandBefore(filter, graphic_type_filter);
	EXPECT_EQ(1, result);

	result = and_operator.evaluateGraphic(graphic);
	EXPECT_EQ(0, result);

	result = or_operator.evaluateGraphic(graphic);
	EXPECT_EQ(1, result);

	result = or_operator.removeOperand(graphic_type_filter);
	EXPECT_EQ(1, result);
}
