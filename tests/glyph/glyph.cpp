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
#include <zinc/spectrum.hpp>

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

class GlyphmodulecallbackRecordChange : public Glyphmodulecallback
{
	int changeFlags;

	virtual void operator()(const Glyphmoduleevent &glyphmoduleevent)
	{
		this->changeFlags = glyphmoduleevent.getSummaryGlyphChangeFlags();
	}

public:
	GlyphmodulecallbackRecordChange() :
		Glyphmodulecallback(),
			changeFlags(Glyph::CHANGE_FLAG_NONE)
	{ }

	void clear()
	{
		changeFlags = Glyph::CHANGE_FLAG_NONE;
	}

	int getChangeSummary() const
	{
		return this->changeFlags;
	}

};

TEST(ZincGlyphmodulenotifier, changeCallback)
{
	ZincTestSetupCpp zinc;
	int result;

	Glyphmodule gm = zinc.context.getGlyphmodule();
	EXPECT_TRUE(gm.isValid());

	Glyphmodulenotifier glyphmodulenotifier = gm.createGlyphmodulenotifier();
	EXPECT_TRUE(glyphmodulenotifier.isValid());

	GlyphmodulecallbackRecordChange callback;
	EXPECT_EQ(CMZN_OK, result = glyphmodulenotifier.setCallback(callback));

	Glyph glyph = gm.getDefaultPointGlyph();
	EXPECT_TRUE(glyph.isValid());

	Spectrummodule sm = zinc.scene.getSpectrummodule();
	Spectrum spectrum = sm.getDefaultSpectrum();
	GlyphColourBar colourBarGlyph = gm.createGlyphColourBar(spectrum);
	EXPECT_TRUE(colourBarGlyph.isValid());
	result = callback.getChangeSummary();
	EXPECT_EQ(Glyph::CHANGE_FLAG_ADD, result);

	EXPECT_EQ(OK, colourBarGlyph.setTickLength(1.234));
	result = callback.getChangeSummary();
	EXPECT_EQ(Glyph::CHANGE_FLAG_FULL_RESULT | Glyph::CHANGE_FLAG_DEFINITION, result);

	colourBarGlyph = GlyphColourBar();
	EXPECT_FALSE(colourBarGlyph.isValid());
	result = callback.getChangeSummary();
	EXPECT_EQ(Glyph::CHANGE_FLAG_REMOVE, result);
}
