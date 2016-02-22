/*
 * OpenCMISS-Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include <opencmiss/zinc/core.h>
#include <opencmiss/zinc/glyph.h>
#include <opencmiss/zinc/spectrum.h>

#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"

class ZincTestSetupSpectrum : public ZincTestSetup
{
public:
	cmzn_spectrummodule_id spectrummodule;
	cmzn_spectrum_id defaultSpectrum;

	ZincTestSetupSpectrum() :
		ZincTestSetup(),
		spectrummodule(cmzn_context_get_spectrummodule(context)),
		defaultSpectrum(cmzn_spectrummodule_get_default_spectrum(spectrummodule))
	{
		EXPECT_NE(static_cast<cmzn_spectrummodule *>(0), this->spectrummodule);
		EXPECT_NE(static_cast<cmzn_spectrum *>(0), this->defaultSpectrum);
	}

	~ZincTestSetupSpectrum()
	{
		cmzn_spectrummodule_destroy(&spectrummodule);
		cmzn_spectrum_destroy(&defaultSpectrum);
	}
};

class ZincTestSetupSpectrumCpp : public ZincTestSetupCpp
{
public:
	Spectrummodule spectrummodule;
	Spectrum defaultSpectrum;

	ZincTestSetupSpectrumCpp() :
		ZincTestSetupCpp(),
		spectrummodule(context.getSpectrummodule()),
		defaultSpectrum(spectrummodule.getDefaultSpectrum())
	{
		EXPECT_TRUE(this->spectrummodule.isValid());
		EXPECT_TRUE(this->defaultSpectrum.isValid());
	}

	~ZincTestSetupSpectrumCpp()
	{
	}
};

TEST(cmzn_glyph_colour_bar, create_cast)
{
	ZincTestSetupSpectrum zinc;
	cmzn_glyph_id glyph = cmzn_glyphmodule_create_glyph_colour_bar(zinc.glyphmodule, zinc.defaultSpectrum);
	EXPECT_NE(static_cast<cmzn_glyph_id>(0), glyph);
	cmzn_glyph_colour_bar_id colourBar = cmzn_glyph_cast_colour_bar(glyph);
	EXPECT_EQ(reinterpret_cast<cmzn_glyph_colour_bar *>(glyph), colourBar);
	cmzn_spectrum_id tmpSpectrum = cmzn_glyph_colour_bar_get_spectrum(colourBar);
	EXPECT_EQ(zinc.defaultSpectrum, tmpSpectrum);
	cmzn_spectrum_destroy(&tmpSpectrum);
	EXPECT_EQ(glyph, cmzn_glyph_colour_bar_base_cast(colourBar));
	cmzn_glyph_colour_bar_destroy(&colourBar);
	cmzn_glyph_destroy(&glyph);
}

TEST(ZincGlyphColourBar, create_cast)
{
	ZincTestSetupSpectrumCpp zinc;

	Glyph glyph = zinc.glyphmodule.createGlyphColourBar(zinc.defaultSpectrum);
	EXPECT_TRUE(glyph.isValid());
	GlyphColourBar colourBar = glyph.castColourBar();
	EXPECT_TRUE(colourBar.isValid());
	Spectrum tmpSpectrum = colourBar.getSpectrum();
	EXPECT_EQ(zinc.defaultSpectrum, tmpSpectrum);
	// try any base class API
	EXPECT_EQ(CMZN_OK, colourBar.setManaged(true));
}

TEST(cmzn_glyph_colour_bar, valid_attributes)
{
	ZincTestSetupSpectrum zinc;
	int result;

	cmzn_glyph_id glyph = cmzn_glyphmodule_create_glyph_colour_bar(zinc.glyphmodule, zinc.defaultSpectrum);
	EXPECT_NE(static_cast<cmzn_glyph*>(0), glyph);

	const char *nameIn = "Bob";
	char *name = cmzn_glyph_get_name(glyph);
	EXPECT_NE(static_cast<char *>(0), name);
	cmzn_deallocate(name);
	EXPECT_EQ(CMZN_OK, result = cmzn_glyph_set_name(glyph, nameIn));
	name = cmzn_glyph_get_name(glyph);
	EXPECT_STREQ(nameIn, name);
	cmzn_deallocate(name);

	cmzn_glyph_colour_bar_id colourBar = cmzn_glyph_cast_colour_bar(glyph);
	EXPECT_NE(static_cast<cmzn_glyph_colour_bar *>(0), colourBar);

	double value;
	double valuesOut[3];
	const double valuesIn[3] = { 1.5, 2.0, 3.0 };

	EXPECT_EQ(CMZN_OK, result = cmzn_glyph_colour_bar_get_axis(colourBar, 3, valuesOut));
	EXPECT_EQ(0.0, valuesOut[0]);
	EXPECT_EQ(1.0, valuesOut[1]);
	EXPECT_EQ(0.0, valuesOut[2]);
	EXPECT_EQ(CMZN_OK, result = cmzn_glyph_colour_bar_set_axis(colourBar, 3, valuesIn));
	EXPECT_EQ(CMZN_OK, result = cmzn_glyph_colour_bar_get_axis(colourBar, 3, valuesOut));
	EXPECT_EQ(valuesIn[0], valuesOut[0]);
	EXPECT_EQ(valuesIn[1], valuesOut[1]);
	EXPECT_EQ(valuesIn[2], valuesOut[2]);

	EXPECT_EQ(CMZN_OK, result = cmzn_glyph_colour_bar_get_centre(colourBar, 3, valuesOut));
	EXPECT_EQ(0.0, valuesOut[0]);
	EXPECT_EQ(0.0, valuesOut[1]);
	EXPECT_EQ(0.0, valuesOut[2]);
	EXPECT_EQ(CMZN_OK, result = cmzn_glyph_colour_bar_set_centre(colourBar, 3, valuesIn));
	EXPECT_EQ(CMZN_OK, result = cmzn_glyph_colour_bar_get_centre(colourBar, 3, valuesOut));
	EXPECT_EQ(valuesIn[0], valuesOut[0]);
	EXPECT_EQ(valuesIn[1], valuesOut[1]);
	EXPECT_EQ(valuesIn[2], valuesOut[2]);

	const double extendLengthIn = 0.6;
	EXPECT_EQ(0.05, value = cmzn_glyph_colour_bar_get_extend_length(colourBar));
	EXPECT_EQ(CMZN_OK, result = cmzn_glyph_colour_bar_set_extend_length(colourBar, extendLengthIn));
	EXPECT_EQ(extendLengthIn, value = cmzn_glyph_colour_bar_get_extend_length(colourBar));

	const int labelDivisionsIn = 15;
	EXPECT_EQ(10, value = cmzn_glyph_colour_bar_get_label_divisions(colourBar));
	EXPECT_EQ(CMZN_OK, result = cmzn_glyph_colour_bar_set_label_divisions(colourBar, labelDivisionsIn));
	EXPECT_EQ(labelDivisionsIn, value = cmzn_glyph_colour_bar_get_label_divisions(colourBar));

	EXPECT_EQ(0, cmzn_glyph_colour_bar_get_label_material(colourBar));
	cmzn_materialmodule_id materialModule = cmzn_context_get_materialmodule(zinc.context);
	cmzn_material_id defaultMaterial = cmzn_materialmodule_get_default_material(materialModule);
	EXPECT_NE(static_cast<cmzn_material_id>(0), defaultMaterial);
	EXPECT_EQ(CMZN_OK, result = cmzn_glyph_colour_bar_set_label_material(colourBar, defaultMaterial));
	cmzn_material_id tempMaterial = cmzn_glyph_colour_bar_get_label_material(colourBar);
	EXPECT_EQ(defaultMaterial, tempMaterial);
	cmzn_material_destroy(&tempMaterial);
	EXPECT_EQ(CMZN_OK, result = cmzn_glyph_colour_bar_set_label_material(colourBar, 0));
	tempMaterial = cmzn_glyph_colour_bar_get_label_material(colourBar);
	EXPECT_EQ(static_cast<cmzn_material_id>(0), tempMaterial);
	cmzn_material_destroy(&defaultMaterial);
	cmzn_materialmodule_destroy(&materialModule);

	const char *numberFormatIn = "%+5.2f %%";
	char *numberFormat = cmzn_glyph_colour_bar_get_number_format(colourBar);
	EXPECT_STREQ("%+.4e", numberFormat);
	cmzn_deallocate(numberFormat);
	EXPECT_EQ(CMZN_OK, result = cmzn_glyph_colour_bar_set_number_format(colourBar, numberFormatIn));
	numberFormat = cmzn_glyph_colour_bar_get_number_format(colourBar);
	EXPECT_STREQ(numberFormatIn, numberFormat);
	cmzn_deallocate(numberFormat);

	EXPECT_EQ(CMZN_OK, result = cmzn_glyph_colour_bar_get_side_axis(colourBar, 3, valuesOut));
	EXPECT_EQ(0.1, valuesOut[0]);
	EXPECT_EQ(0.0, valuesOut[1]);
	EXPECT_EQ(0.0, valuesOut[2]);
	EXPECT_EQ(CMZN_OK, result = cmzn_glyph_colour_bar_set_side_axis(colourBar, 3, valuesIn));
	EXPECT_EQ(CMZN_OK, result = cmzn_glyph_colour_bar_get_side_axis(colourBar, 3, valuesOut));
	EXPECT_EQ(valuesIn[0], valuesOut[0]);
	EXPECT_EQ(valuesIn[1], valuesOut[1]);
	EXPECT_EQ(valuesIn[2], valuesOut[2]);

	const double tickLengthIn = 0.6;
	EXPECT_EQ(0.05, value = cmzn_glyph_colour_bar_get_tick_length(colourBar));
	EXPECT_EQ(CMZN_OK, result = cmzn_glyph_colour_bar_set_tick_length(colourBar, tickLengthIn));
	EXPECT_EQ(tickLengthIn, value = cmzn_glyph_colour_bar_get_tick_length(colourBar));

	cmzn_glyph_colour_bar_destroy(&colourBar);
	cmzn_glyph_destroy(&glyph);
}

TEST(ZincGlyphColourBar, valid_attributes)
{
	ZincTestSetupSpectrumCpp zinc;
	int result;

	GlyphColourBar colourBar = zinc.glyphmodule.createGlyphColourBar(zinc.defaultSpectrum);
	EXPECT_TRUE(colourBar.isValid());

	const char *nameIn = "Bob";
	char *name = colourBar.getName();
	EXPECT_NE(static_cast<char *>(0), name);
	cmzn_deallocate(name);
	EXPECT_EQ(CMZN_OK, result = colourBar.setName(nameIn));
	name = colourBar.getName();
	EXPECT_STREQ(nameIn, name);
	cmzn_deallocate(name);

	double value;
	double valuesOut[3];
	const double valuesIn[3] = { 1.5, 2.0, 3.0 };

	EXPECT_EQ(CMZN_OK, result = colourBar.getAxis(3, valuesOut));
	EXPECT_EQ(0.0, valuesOut[0]);
	EXPECT_EQ(1.0, valuesOut[1]);
	EXPECT_EQ(0.0, valuesOut[2]);
	EXPECT_EQ(CMZN_OK, result = colourBar.setAxis(3, valuesIn));
	EXPECT_EQ(CMZN_OK, result = colourBar.getAxis(3, valuesOut));
	EXPECT_EQ(valuesIn[0], valuesOut[0]);
	EXPECT_EQ(valuesIn[1], valuesOut[1]);
	EXPECT_EQ(valuesIn[2], valuesOut[2]);

	EXPECT_EQ(CMZN_OK, result = colourBar.getCentre(3, valuesOut));
	EXPECT_EQ(0.0, valuesOut[0]);
	EXPECT_EQ(0.0, valuesOut[1]);
	EXPECT_EQ(0.0, valuesOut[2]);
	EXPECT_EQ(CMZN_OK, result = colourBar.setCentre(3, valuesIn));
	EXPECT_EQ(CMZN_OK, result = colourBar.getCentre(3, valuesOut));
	EXPECT_EQ(valuesIn[0], valuesOut[0]);
	EXPECT_EQ(valuesIn[1], valuesOut[1]);
	EXPECT_EQ(valuesIn[2], valuesOut[2]);

	const double extendLengthIn = 0.6;
	EXPECT_EQ(0.05, value = colourBar.getExtendLength());
	EXPECT_EQ(CMZN_OK, result = colourBar.setExtendLength(extendLengthIn));
	EXPECT_EQ(extendLengthIn, value = colourBar.getExtendLength());

	const int labelDivisionsIn = 15;
	EXPECT_EQ(10, value = colourBar.getLabelDivisions());
	EXPECT_EQ(CMZN_OK, result = colourBar.setLabelDivisions(labelDivisionsIn));
	EXPECT_EQ(labelDivisionsIn, value = colourBar.getLabelDivisions());

	Material tempMaterial = colourBar.getLabelMaterial();
	EXPECT_FALSE(tempMaterial.isValid());
	Materialmodule materialModule = zinc.context.getMaterialmodule();
	Material defaultMaterial = materialModule.getDefaultMaterial();
	EXPECT_TRUE(defaultMaterial.isValid());
	EXPECT_EQ(CMZN_OK, result = colourBar.setLabelMaterial(defaultMaterial));
	tempMaterial = colourBar.getLabelMaterial();
	EXPECT_EQ(defaultMaterial.getId(), tempMaterial.getId());
	EXPECT_EQ(CMZN_OK, result = colourBar.setLabelMaterial(Material()));
	tempMaterial = colourBar.getLabelMaterial();
	EXPECT_FALSE(tempMaterial.isValid());

	const char *numberFormatIn = "%+5.2f %%";
	char *numberFormat = colourBar.getNumberFormat();
	EXPECT_STREQ("%+.4e", numberFormat);
	cmzn_deallocate(numberFormat);
	EXPECT_EQ(CMZN_OK, result = colourBar.setNumberFormat(numberFormatIn));
	numberFormat = colourBar.getNumberFormat();
	EXPECT_STREQ(numberFormatIn, numberFormat);
	cmzn_deallocate(numberFormat);

	EXPECT_EQ(CMZN_OK, result = colourBar.getSideAxis(3, valuesOut));
	EXPECT_EQ(0.1, valuesOut[0]);
	EXPECT_EQ(0.0, valuesOut[1]);
	EXPECT_EQ(0.0, valuesOut[2]);
	EXPECT_EQ(CMZN_OK, result = colourBar.setSideAxis(3, valuesIn));
	EXPECT_EQ(CMZN_OK, result = colourBar.getSideAxis(3, valuesOut));
	EXPECT_EQ(valuesIn[0], valuesOut[0]);
	EXPECT_EQ(valuesIn[1], valuesOut[1]);
	EXPECT_EQ(valuesIn[2], valuesOut[2]);

	const double tickLengthIn = 0.6;
	EXPECT_EQ(0.05, value = colourBar.getTickLength());
	EXPECT_EQ(CMZN_OK, result = colourBar.setTickLength(tickLengthIn));
	EXPECT_EQ(tickLengthIn, value = colourBar.getTickLength());
}

TEST(cmzn_glyph_colour_bar, invalid_attributes)
{
	ZincTestSetupSpectrum zinc;
	int result;

	cmzn_glyph_id noGlyphmoduleColourBar = cmzn_glyphmodule_create_glyph_colour_bar(static_cast<cmzn_glyphmodule_id>(0), zinc.defaultSpectrum);
	EXPECT_EQ(static_cast<cmzn_glyph*>(0), noGlyphmoduleColourBar);

	cmzn_glyph_id noSpectrumColourBar = cmzn_glyphmodule_create_glyph_colour_bar(zinc.glyphmodule, static_cast<cmzn_spectrum_id>(0));
	EXPECT_EQ(static_cast<cmzn_glyph*>(0), noSpectrumColourBar);

	cmzn_glyph_id glyph = cmzn_glyphmodule_create_glyph_colour_bar(zinc.glyphmodule, zinc.defaultSpectrum);
	EXPECT_NE(static_cast<cmzn_glyph*>(0), glyph);
	const char *nameIn = "Bob";
	char *name = cmzn_glyph_get_name(static_cast<cmzn_glyph_id>(0));
	EXPECT_EQ(static_cast<char *>(0), name);
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = cmzn_glyph_set_name(static_cast<cmzn_glyph_id>(0), nameIn));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = cmzn_glyph_set_name(glyph, static_cast<const char *>(0)));

	cmzn_glyph_colour_bar_id colourBar = cmzn_glyph_cast_colour_bar(glyph);
	EXPECT_NE(static_cast<cmzn_glyph_colour_bar *>(0), colourBar);

	double value;
	double valuesOut[3];
	const double valuesIn[3] = { 1.5, 2.0, 3.0 };

	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_get_axis(0, 3, valuesOut));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_get_axis(colourBar, 0, valuesOut));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_get_axis(colourBar, 3, 0));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_set_axis(0, 3, valuesIn));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_set_axis(colourBar, 0, valuesIn));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_set_axis(colourBar, 3, 0));

	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_get_centre(0, 3, valuesOut));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_get_centre(colourBar, 0, valuesOut));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_get_centre(colourBar, 3, 0));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_set_centre(0, 3, valuesIn));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_set_centre(colourBar, 0, valuesIn));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_set_centre(colourBar, 3, 0));

	const double extendLengthIn = 0.6;
	EXPECT_EQ(0.0, value = cmzn_glyph_colour_bar_get_extend_length(0));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_set_extend_length(0, extendLengthIn));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_set_extend_length(colourBar, -1.0));

	const int labelDivisionsIn = 15;
	EXPECT_EQ(0, value = cmzn_glyph_colour_bar_get_label_divisions(0));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_set_label_divisions(0, labelDivisionsIn));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_set_label_divisions(colourBar, 0));

	EXPECT_EQ(0, cmzn_glyph_colour_bar_get_label_material(0));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_set_label_material(0, 0));

	char *numberFormat = cmzn_glyph_colour_bar_get_number_format(0);
	EXPECT_EQ(static_cast<char *>(0), numberFormat);
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_set_number_format(0, "%+5.2f %%"));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_set_number_format(colourBar, 0));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_set_number_format(colourBar, ""));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_set_number_format(colourBar, "%F %g"));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_set_number_format(colourBar, "%500f"));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_set_number_format(colourBar, "%d"));

	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_get_side_axis(0, 3, valuesOut));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_get_side_axis(colourBar, 0, valuesOut));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_get_side_axis(colourBar, 3, 0));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_set_side_axis(0, 3, valuesIn));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_set_side_axis(colourBar, 0, valuesIn));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_set_side_axis(colourBar, 3, 0));

	const double tickLengthIn = 0.6;
	EXPECT_EQ(0.0, value = cmzn_glyph_colour_bar_get_tick_length(0));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_set_tick_length(0, tickLengthIn));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_set_tick_length(colourBar, -1.0));

	cmzn_glyph_colour_bar_destroy(&colourBar);
	cmzn_glyph_destroy(&glyph);
}

TEST(ZincGlyphColourBar, invalid_attributes)
{
	ZincTestSetupSpectrumCpp zinc;
	int result;

	GlyphColourBar noSpectrumColourBar = zinc.glyphmodule.createGlyphColourBar(Spectrum());
	EXPECT_FALSE(noSpectrumColourBar.isValid());

	GlyphColourBar colourBar = zinc.glyphmodule.createGlyphColourBar(zinc.defaultSpectrum);
	EXPECT_TRUE(colourBar.isValid());

	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = colourBar.setName(0));

	double valuesOut[3];
	const double valuesIn[3] = { 1.5, 2.0, 3.0 };

	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = colourBar.getAxis(0, valuesOut));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = colourBar.getAxis(3, 0));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = colourBar.setAxis(0, valuesIn));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = colourBar.setAxis(3, 0));

	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = colourBar.getCentre(0, valuesOut));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = colourBar.getCentre(3, 0));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = colourBar.setCentre(0, valuesIn));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = colourBar.setCentre(3, 0));

	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = colourBar.setLabelDivisions(0));

	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = colourBar.setExtendLength(-1.0));

	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = colourBar.setNumberFormat(0));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = colourBar.setNumberFormat(""));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = colourBar.setNumberFormat("%F %g"));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = colourBar.setNumberFormat("%500f"));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = colourBar.setNumberFormat("%d"));

	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = colourBar.getSideAxis(0, valuesOut));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = colourBar.getSideAxis(3, 0));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = colourBar.setSideAxis(0, valuesIn));
	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = colourBar.setSideAxis(3, 0));

	EXPECT_EQ(CMZN_ERROR_ARGUMENT, result = colourBar.setTickLength(-1.0));
}
