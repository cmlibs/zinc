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
#include <zinc/font.h>

#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"
#include "zinc/font.hpp"

TEST(cmzn_fontmodule_api, valid_args)
{
	ZincTestSetup zinc;

	cmzn_fontmodule_id fontmodule = cmzn_context_get_fontmodule(zinc.context);
	EXPECT_NE(static_cast<cmzn_fontmodule *>(0), fontmodule);

	int result = cmzn_fontmodule_begin_change(fontmodule);
	EXPECT_EQ(CMZN_OK, result);

	cmzn_font_id font = cmzn_fontmodule_create_font(fontmodule);
	EXPECT_NE(static_cast<cmzn_font *>(0), font);

	result = cmzn_font_set_name(font, "new_default");
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_fontmodule_end_change(fontmodule);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_fontmodule_set_default_font(fontmodule, font);
	EXPECT_EQ(CMZN_OK, result);

	cmzn_font_destroy(&font);

	font = cmzn_fontmodule_find_font_by_name(fontmodule, "new_default");
	EXPECT_NE(static_cast<cmzn_font *>(0), font);

	cmzn_font_destroy(&font);

	font = cmzn_fontmodule_get_default_font(fontmodule);
	EXPECT_NE(static_cast<cmzn_font *>(0), font);

	cmzn_font_destroy(&font);

	cmzn_fontmodule_destroy(&fontmodule);
}

TEST(cmzn_fontmodule_api, valid_args_cpp)
{
	ZincTestSetupCpp zinc;

	Fontmodule fontmodule = zinc.context.getFontmodule();
	EXPECT_TRUE(fontmodule.isValid());

	int result = fontmodule.beginChange();
	EXPECT_EQ(CMZN_OK, result);

	Font font = fontmodule.createFont();
	EXPECT_TRUE(font.isValid());

	result = font.setName("new_default");
	EXPECT_EQ(CMZN_OK, result);

	result = fontmodule.endChange();
	EXPECT_EQ(CMZN_OK, result);

	result = fontmodule.setDefaultFont( font);
	EXPECT_EQ(CMZN_OK, result);

	font = fontmodule.findFontByName("new_default");
	EXPECT_TRUE(font.isValid());

	font = fontmodule.getDefaultFont();
	EXPECT_TRUE(font.isValid());
}

TEST(cmzn_font_api, valid_args)
{
	ZincTestSetup zinc;

	cmzn_fontmodule_id fontmodule = cmzn_context_get_fontmodule(zinc.context);
	EXPECT_NE(static_cast<cmzn_fontmodule *>(0), fontmodule);

	int result = cmzn_fontmodule_begin_change(fontmodule);
	EXPECT_EQ(CMZN_OK, result);

	cmzn_font_id font = cmzn_fontmodule_create_font(fontmodule);
	EXPECT_NE(static_cast<cmzn_font *>(0), font);

	result = cmzn_font_set_name(font, "new_default");
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_fontmodule_end_change(fontmodule);
	EXPECT_EQ(CMZN_OK, result);

	EXPECT_FALSE(cmzn_font_is_bold(font));
	result = cmzn_font_set_bold(font, true);
	EXPECT_EQ(CMZN_OK, result);
	EXPECT_TRUE(cmzn_font_is_bold(font));

	result = cmzn_font_set_depth(font,10.0);
	EXPECT_EQ(CMZN_OK, result);

	double depth = cmzn_font_get_depth(font);
	EXPECT_EQ(10.0, depth);

	EXPECT_FALSE(cmzn_font_is_italic(font));
	result = cmzn_font_set_italic(font, true);
	EXPECT_EQ(CMZN_OK, result);
	EXPECT_TRUE(cmzn_font_is_italic(font));

	result = cmzn_font_get_point_size(font);
	EXPECT_EQ(15, result); // default
	result = cmzn_font_set_point_size(font, 20);
	EXPECT_EQ(CMZN_OK, result);
	result = cmzn_font_get_point_size(font);
	EXPECT_EQ(20, result);

	result = cmzn_font_set_render_type(font, CMZN_FONT_RENDER_TYPE_POLYGON);
	EXPECT_EQ(CMZN_OK, result);

	enum cmzn_font_render_type render_type = cmzn_font_get_render_type(font);
	EXPECT_EQ(CMZN_FONT_RENDER_TYPE_POLYGON, render_type);

	result = cmzn_font_set_typeface_type(font, CMZN_FONT_TYPEFACE_TYPE_OPENSANS);
	EXPECT_EQ(CMZN_OK, result);

	enum cmzn_font_typeface_type typeface_type = cmzn_font_get_typeface_type(font);
	EXPECT_EQ(CMZN_FONT_TYPEFACE_TYPE_OPENSANS, typeface_type);

	cmzn_font_destroy(&font);

	cmzn_fontmodule_destroy(&fontmodule);
}

TEST(cmzn_font_api, valid_args_cpp)
{
	ZincTestSetupCpp zinc;

	Fontmodule fontmodule = zinc.context.getFontmodule();
	EXPECT_TRUE(fontmodule.isValid());

	int result = fontmodule.beginChange();
	EXPECT_EQ(CMZN_OK, result);

	Font font = fontmodule.createFont();
	EXPECT_TRUE(font.isValid());

	result = font.setName("new_default");
	EXPECT_EQ(CMZN_OK, result);

	result = fontmodule.endChange();
	EXPECT_EQ(CMZN_OK, result);

	EXPECT_FALSE(font.isBold());
	result = font.setBold(true);
	EXPECT_EQ(CMZN_OK, result);
	EXPECT_TRUE(font.isBold());

	result = font.setDepth(10.0);
	EXPECT_EQ(CMZN_OK, result);

	double depth = font.getDepth();
	EXPECT_EQ(10.0, depth);

	EXPECT_FALSE(font.isItalic());
	result = font.setItalic(true);
	EXPECT_EQ(CMZN_OK, result);
	EXPECT_TRUE(font.isItalic());

	result = font.getPointSize();
	EXPECT_EQ(15, result); // default
	result = font.setPointSize(20);
	EXPECT_EQ(CMZN_OK, result);
	result = font.getPointSize();
	EXPECT_EQ(20, result);

	result = font.setRenderType(font.RENDER_TYPE_POLYGON);
	EXPECT_EQ(CMZN_OK, result);

	enum Font::RenderType render_type = font.getRenderType();
	EXPECT_EQ(font.RENDER_TYPE_POLYGON, render_type);

	result = font.setTypefaceType(font.TYPEFACE_TYPE_OPENSANS);
	EXPECT_EQ(CMZN_OK, result);

	enum Font::TypefaceType typefaceType = font.getTypefaceType();
	EXPECT_EQ(font.TYPEFACE_TYPE_OPENSANS, typefaceType);

}

