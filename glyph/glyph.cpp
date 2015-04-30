/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include <zinc/core.h>

#include <zinc/glyph.hpp>

#include "zinctestsetupcpp.hpp"

TEST(ZincGlyphiterator, iteration)
{
	ZincTestSetupCpp zinc;

	Glyph arrow = zinc.glyphmodule.findGlyphByGlyphShapeType(Glyph::SHAPE_TYPE_ARROW);

	Glyph zzz = zinc.glyphmodule.createGlyphAxes(arrow, 0.1);
	EXPECT_TRUE(zzz.isValid());
	EXPECT_EQ(OK, zzz.setName("zzz"));
	char *name = zzz.getName();
	EXPECT_STREQ("zzz", name);
	cmzn_deallocate(name);
	name = 0;

	Glyph aaa = zinc.glyphmodule.createGlyphAxes(arrow, 0.1);
	EXPECT_TRUE(aaa.isValid());
	EXPECT_EQ(OK, aaa.setName("aaa"));

	Glyph aab = zinc.glyphmodule.createGlyphAxes(arrow, 0.1);
	EXPECT_TRUE(aab.isValid());
	EXPECT_EQ(OK, aab.setName("aab"));

	Glyphiterator iter = zinc.glyphmodule.createGlyphiterator();
	EXPECT_TRUE(iter.isValid());
	Glyph g;
	EXPECT_EQ(aaa, g = iter.next());
	EXPECT_EQ(aab, g = iter.next());
	Glyph tmp;
	while ((tmp = iter.next()).isValid())
	{
		g = tmp;
	}
	EXPECT_EQ(zzz, g);
}
