
#include <gtest/gtest.h>

#include <zinc/zincconfigure.h>
#include <zinc/status.h>
#include <zinc/core.h>
#include <zinc/font.h>

#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"
#include "zinc/font.hpp"

TEST(Cmiss_font_module_api, valid_args)
{
	ZincTestSetup zinc;

	Cmiss_font_module_id fontmodule = Cmiss_graphics_module_get_font_module(zinc.gm);
	EXPECT_NE(static_cast<Cmiss_font_module *>(0), fontmodule);

	int result = Cmiss_font_module_begin_change(fontmodule);
	EXPECT_EQ(CMISS_OK, result);

	Cmiss_font_id font = Cmiss_font_module_create_font(fontmodule);
	EXPECT_NE(static_cast<Cmiss_font *>(0), font);

	result = Cmiss_font_set_name(font, "default");
	EXPECT_EQ(CMISS_OK, result);

	result = Cmiss_font_module_end_change(fontmodule);
	EXPECT_EQ(CMISS_OK, result);

	result = Cmiss_font_module_set_default_font(fontmodule, font);
	EXPECT_EQ(CMISS_OK, result);

	Cmiss_font_destroy(&font);

	font = Cmiss_font_module_find_font_by_name(fontmodule, "default");
	EXPECT_NE(static_cast<Cmiss_font *>(0), font);

	Cmiss_font_destroy(&font);

	font = Cmiss_font_module_get_default_font(fontmodule);
	EXPECT_NE(static_cast<Cmiss_font *>(0), font);

	Cmiss_font_destroy(&font);

	Cmiss_font_module_destroy(&fontmodule);
}

TEST(Cmiss_font_module_api, valid_args_cpp)
{
	ZincTestSetupCpp zinc;

	FontModule fontmodule = zinc.gm.getFontModule();
	EXPECT_TRUE(fontmodule.isValid());

	int result = fontmodule.beginChange();
	EXPECT_EQ(CMISS_OK, result);

	Font font = fontmodule.createFont();
	EXPECT_TRUE(font.isValid());

	result = font.setName("default");
	EXPECT_EQ(CMISS_OK, result);

	result = fontmodule.endChange();
	EXPECT_EQ(CMISS_OK, result);

	result = fontmodule.setDefaultFont( font);
	EXPECT_EQ(CMISS_OK, result);

	font = fontmodule.findFontByName("default");
	EXPECT_TRUE(font.isValid());

	font = fontmodule.getDefaultFont();
	EXPECT_TRUE(font.isValid());
}

TEST(Cmiss_font_api, valid_args)
{
	ZincTestSetup zinc;

	Cmiss_font_module_id fontmodule = Cmiss_graphics_module_get_font_module(zinc.gm);
	EXPECT_NE(static_cast<Cmiss_font_module *>(0), fontmodule);

	int result = Cmiss_font_module_begin_change(fontmodule);
	EXPECT_EQ(CMISS_OK, result);

	Cmiss_font_id font = Cmiss_font_module_create_font(fontmodule);
	EXPECT_NE(static_cast<Cmiss_font *>(0), font);

	result = Cmiss_font_set_name(font, "default");
	EXPECT_EQ(CMISS_OK, result);

	result = Cmiss_font_module_end_change(fontmodule);
	EXPECT_EQ(CMISS_OK, result);

	result = Cmiss_font_set_bold(font, 1);
	EXPECT_EQ(CMISS_OK, result);

	result = Cmiss_font_get_bold(font);
	EXPECT_EQ(1, result);

	result = Cmiss_font_set_depth(font,10.0);
	EXPECT_EQ(CMISS_OK, result);

	double depth = Cmiss_font_get_depth(font);
	EXPECT_EQ(10.0, depth);

	result = Cmiss_font_set_italic(font, 1);
	EXPECT_EQ(CMISS_OK, result);

	result = Cmiss_font_get_italic(font);
	EXPECT_EQ(1, result);

	result = Cmiss_font_set_size(font, 20);
	EXPECT_EQ(CMISS_OK, result);

	result = Cmiss_font_get_size(font);
	EXPECT_EQ(20, result);

	result = Cmiss_font_set_render_type(font, CMISS_FONT_RENDER_TYPE_POLYGON);
	EXPECT_EQ(CMISS_OK, result);

	enum Cmiss_font_render_type render_type = Cmiss_font_get_render_type(font);
	EXPECT_EQ(CMISS_FONT_RENDER_TYPE_POLYGON, render_type);

	result = Cmiss_font_set_font_type(font, CMISS_FONT_TYPE_OpenSans);
	EXPECT_EQ(CMISS_OK, result);

	enum Cmiss_font_type font_type = Cmiss_font_get_font_type(font);
	EXPECT_EQ(CMISS_FONT_TYPE_OpenSans, font_type);

	Cmiss_font_destroy(&font);

	Cmiss_font_module_destroy(&fontmodule);
}

TEST(Cmiss_font_api, valid_args_cpp)
{
	ZincTestSetupCpp zinc;

	FontModule fontmodule = zinc.gm.getFontModule();
	EXPECT_TRUE(fontmodule.isValid());

	int result = fontmodule.beginChange();
	EXPECT_EQ(CMISS_OK, result);

	Font font = fontmodule.createFont();
	EXPECT_TRUE(font.isValid());

	result = font.setName("default");
	EXPECT_EQ(CMISS_OK, result);

	result = fontmodule.endChange();
	EXPECT_EQ(CMISS_OK, result);

	result = font.setBold(1);
	EXPECT_EQ(CMISS_OK, result);

	result = font.getBold();
	EXPECT_EQ(1, result);

	result = font.setDepth(10.0);
	EXPECT_EQ(CMISS_OK, result);

	double depth = font.getDepth();
	EXPECT_EQ(10.0, depth);

	result = font.setItalic(1);
	EXPECT_EQ(CMISS_OK, result);

	result = font.getItalic();
	EXPECT_EQ(1, result);

	result = font.setSize(20);
	EXPECT_EQ(CMISS_OK, result);

	result = font.getSize();
	EXPECT_EQ(20, result);

	result = font.setRenderType(font.RENDER_TYPE_POLYGON);
	EXPECT_EQ(CMISS_OK, result);

	enum Font::RenderType render_type = font.getRenderType();
	EXPECT_EQ(font.RENDER_TYPE_POLYGON, render_type);

	result = font.setFontType(font.FONT_TYPE_OpenSans);
	EXPECT_EQ(CMISS_OK, result);

	enum Font::FontType font_type = font.getFontType();
	EXPECT_EQ(font.FONT_TYPE_OpenSans, font_type);


}

