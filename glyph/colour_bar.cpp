
#include <gtest/gtest.h>

#include <zinc/core.h>
#include <zinc/glyph.h>
#include <zinc/spectrum.h>

#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"

class ZincTestSetupSpectrum : public ZincTestSetup
{
public:
	cmzn_spectrum_module_id spectrumModule;
	cmzn_spectrum_id defaultSpectrum;

	ZincTestSetupSpectrum() :
		ZincTestSetup(),
		spectrumModule(cmzn_graphics_module_get_spectrum_module(gm)),
		defaultSpectrum(cmzn_spectrum_module_get_default_spectrum(spectrumModule))
	{
		EXPECT_NE(static_cast<cmzn_spectrum_module *>(0), this->spectrumModule);
		EXPECT_NE(static_cast<cmzn_spectrum *>(0), this->defaultSpectrum);
	}

	~ZincTestSetupSpectrum()
	{
		cmzn_spectrum_module_destroy(&spectrumModule);
		cmzn_spectrum_destroy(&defaultSpectrum);
	}
};

class ZincTestSetupSpectrumCpp : public ZincTestSetupCpp
{
public:
	SpectrumModule spectrumModule;
	Spectrum defaultSpectrum;

	ZincTestSetupSpectrumCpp() :
		ZincTestSetupCpp(),
		spectrumModule(gm.getSpectrumModule()),
		defaultSpectrum(spectrumModule.getDefaultSpectrum())
	{
		EXPECT_TRUE(this->spectrumModule.isValid());
		EXPECT_TRUE(this->defaultSpectrum.isValid());
	}

	~ZincTestSetupSpectrumCpp()
	{
	}
};

TEST(cmzn_glyph_colour_bar, create)
{
	ZincTestSetupSpectrum zinc;

	cmzn_glyph_colour_bar_id colourBar = cmzn_glyph_module_create_colour_bar(zinc.glyph_module, zinc.defaultSpectrum);
	EXPECT_NE(static_cast<cmzn_glyph_colour_bar *>(0), colourBar);

	cmzn_glyph_colour_bar_destroy(&colourBar);
}

TEST(cmzn_glyph_colour_bar, create_cpp)
{
	ZincTestSetupSpectrumCpp zinc;

	GlyphColourBar colourBar = zinc.glyphModule.createColourBar(zinc.defaultSpectrum);
	EXPECT_TRUE(colourBar.isValid());
}

TEST(cmzn_glyph_colour_bar, cast)
{
	ZincTestSetupSpectrum zinc;

	cmzn_glyph_id glyph = cmzn_glyph_colour_bar_base_cast(cmzn_glyph_module_create_colour_bar(zinc.glyph_module, zinc.defaultSpectrum));
	EXPECT_NE(static_cast<cmzn_glyph *>(0), glyph);

	cmzn_glyph_colour_bar_id colourBar = cmzn_glyph_cast_colour_bar(glyph);
	EXPECT_EQ(reinterpret_cast<cmzn_glyph_colour_bar *>(glyph), colourBar);

	EXPECT_EQ(glyph, cmzn_glyph_colour_bar_base_cast(colourBar));

	cmzn_glyph_colour_bar_destroy(&colourBar);
	cmzn_glyph_destroy(&glyph);
}

TEST(cmzn_glyph_colour_bar, cast_cpp)
{
	ZincTestSetupSpectrumCpp zinc;

	Glyph glyph = zinc.glyphModule.createColourBar(zinc.defaultSpectrum);
	EXPECT_TRUE(glyph.isValid());

	GlyphColourBar colourBar(glyph);
	EXPECT_TRUE(colourBar.isValid());

	// try any base class API
	EXPECT_EQ(CMISS_OK, colourBar.setManaged(true));
}

TEST(cmzn_glyph_colour_bar, valid_attributes)
{
	ZincTestSetupSpectrum zinc;
	int result;

	cmzn_glyph_colour_bar_id colourBar = cmzn_glyph_module_create_colour_bar(zinc.glyph_module, zinc.defaultSpectrum);
	EXPECT_NE(static_cast<cmzn_glyph_colour_bar *>(0), colourBar);

	const char *nameIn = "Bob";
	char *name = cmzn_glyph_get_name(cmzn_glyph_colour_bar_base_cast(colourBar));
	EXPECT_NE(static_cast<char *>(0), name);
	cmzn_deallocate(name);
	EXPECT_EQ(CMISS_OK, result = cmzn_glyph_set_name(cmzn_glyph_colour_bar_base_cast(colourBar), nameIn));
	name = cmzn_glyph_get_name(cmzn_glyph_colour_bar_base_cast(colourBar));
	EXPECT_STREQ(nameIn, name);
	cmzn_deallocate(name);

	double value;
	double valuesOut[3];
	const double valuesIn[3] = { 1.5, 2.0, 3.0 };

	EXPECT_EQ(CMISS_OK, result = cmzn_glyph_colour_bar_get_axis(colourBar, 3, valuesOut));
	EXPECT_EQ(0.0, valuesOut[0]);
	EXPECT_EQ(1.0, valuesOut[1]);
	EXPECT_EQ(0.0, valuesOut[2]);
	EXPECT_EQ(CMISS_OK, result = cmzn_glyph_colour_bar_set_axis(colourBar, 3, valuesIn));
	EXPECT_EQ(CMISS_OK, result = cmzn_glyph_colour_bar_get_axis(colourBar, 3, valuesOut));
	EXPECT_EQ(valuesIn[0], valuesOut[0]);
	EXPECT_EQ(valuesIn[1], valuesOut[1]);
	EXPECT_EQ(valuesIn[2], valuesOut[2]);

	EXPECT_EQ(CMISS_OK, result = cmzn_glyph_colour_bar_get_centre(colourBar, 3, valuesOut));
	EXPECT_EQ(0.0, valuesOut[0]);
	EXPECT_EQ(0.0, valuesOut[1]);
	EXPECT_EQ(0.0, valuesOut[2]);
	EXPECT_EQ(CMISS_OK, result = cmzn_glyph_colour_bar_set_centre(colourBar, 3, valuesIn));
	EXPECT_EQ(CMISS_OK, result = cmzn_glyph_colour_bar_get_centre(colourBar, 3, valuesOut));
	EXPECT_EQ(valuesIn[0], valuesOut[0]);
	EXPECT_EQ(valuesIn[1], valuesOut[1]);
	EXPECT_EQ(valuesIn[2], valuesOut[2]);

	const double extendLengthIn = 0.6;
	EXPECT_EQ(0.05, value = cmzn_glyph_colour_bar_get_extend_length(colourBar));
	EXPECT_EQ(CMISS_OK, result = cmzn_glyph_colour_bar_set_extend_length(colourBar, extendLengthIn));
	EXPECT_EQ(extendLengthIn, value = cmzn_glyph_colour_bar_get_extend_length(colourBar));

	const int labelDivisionsIn = 15;
	EXPECT_EQ(10, value = cmzn_glyph_colour_bar_get_label_divisions(colourBar));
	EXPECT_EQ(CMISS_OK, result = cmzn_glyph_colour_bar_set_label_divisions(colourBar, labelDivisionsIn));
	EXPECT_EQ(labelDivisionsIn, value = cmzn_glyph_colour_bar_get_label_divisions(colourBar));

	EXPECT_EQ(0, cmzn_glyph_colour_bar_get_label_material(colourBar));
	cmzn_graphics_material_module_id materialModule = cmzn_graphics_module_get_material_module(zinc.gm);
	cmzn_graphics_material_id defaultMaterial = cmzn_graphics_material_module_get_default_material(materialModule);
	EXPECT_NE(static_cast<cmzn_graphics_material_id>(0), defaultMaterial);
	EXPECT_EQ(CMISS_OK, result = cmzn_glyph_colour_bar_set_label_material(colourBar, defaultMaterial));
	cmzn_graphics_material_id tempMaterial = cmzn_glyph_colour_bar_get_label_material(colourBar);
	EXPECT_EQ(defaultMaterial, tempMaterial);
	cmzn_graphics_material_destroy(&tempMaterial);
	EXPECT_EQ(CMISS_OK, result = cmzn_glyph_colour_bar_set_label_material(colourBar, 0));
	tempMaterial = cmzn_glyph_colour_bar_get_label_material(colourBar);
	EXPECT_EQ(static_cast<cmzn_graphics_material_id>(0), tempMaterial);
	cmzn_graphics_material_destroy(&defaultMaterial);
	cmzn_graphics_material_module_destroy(&materialModule);

	const char *numberFormatIn = "%+5.2f %%";
	char *numberFormat = cmzn_glyph_colour_bar_get_number_format(colourBar);
	EXPECT_STREQ("%+.4e", numberFormat);
	cmzn_deallocate(numberFormat);
	EXPECT_EQ(CMISS_OK, result = cmzn_glyph_colour_bar_set_number_format(colourBar, numberFormatIn));
	numberFormat = cmzn_glyph_colour_bar_get_number_format(colourBar);
	EXPECT_STREQ(numberFormatIn, numberFormat);
	cmzn_deallocate(numberFormat);

	EXPECT_EQ(CMISS_OK, result = cmzn_glyph_colour_bar_get_side_axis(colourBar, 3, valuesOut));
	EXPECT_EQ(0.1, valuesOut[0]);
	EXPECT_EQ(0.0, valuesOut[1]);
	EXPECT_EQ(0.0, valuesOut[2]);
	EXPECT_EQ(CMISS_OK, result = cmzn_glyph_colour_bar_set_side_axis(colourBar, 3, valuesIn));
	EXPECT_EQ(CMISS_OK, result = cmzn_glyph_colour_bar_get_side_axis(colourBar, 3, valuesOut));
	EXPECT_EQ(valuesIn[0], valuesOut[0]);
	EXPECT_EQ(valuesIn[1], valuesOut[1]);
	EXPECT_EQ(valuesIn[2], valuesOut[2]);

	const double tickLengthIn = 0.6;
	EXPECT_EQ(0.05, value = cmzn_glyph_colour_bar_get_tick_length(colourBar));
	EXPECT_EQ(CMISS_OK, result = cmzn_glyph_colour_bar_set_tick_length(colourBar, tickLengthIn));
	EXPECT_EQ(tickLengthIn, value = cmzn_glyph_colour_bar_get_tick_length(colourBar));

	cmzn_glyph_colour_bar_destroy(&colourBar);
}

TEST(cmzn_glyph_colour_bar, valid_attributes_cpp)
{
	ZincTestSetupSpectrumCpp zinc;
	int result;

	GlyphColourBar colourBar = zinc.glyphModule.createColourBar(zinc.defaultSpectrum);
	EXPECT_TRUE(colourBar.isValid());

	const char *nameIn = "Bob";
	char *name = colourBar.getName();
	EXPECT_NE(static_cast<char *>(0), name);
	cmzn_deallocate(name);
	EXPECT_EQ(CMISS_OK, result = colourBar.setName(nameIn));
	name = colourBar.getName();
	EXPECT_STREQ(nameIn, name);
	cmzn_deallocate(name);

	double value;
	double valuesOut[3];
	const double valuesIn[3] = { 1.5, 2.0, 3.0 };

	EXPECT_EQ(CMISS_OK, result = colourBar.getAxis(3, valuesOut));
	EXPECT_EQ(0.0, valuesOut[0]);
	EXPECT_EQ(1.0, valuesOut[1]);
	EXPECT_EQ(0.0, valuesOut[2]);
	EXPECT_EQ(CMISS_OK, result = colourBar.setAxis(3, valuesIn));
	EXPECT_EQ(CMISS_OK, result = colourBar.getAxis(3, valuesOut));
	EXPECT_EQ(valuesIn[0], valuesOut[0]);
	EXPECT_EQ(valuesIn[1], valuesOut[1]);
	EXPECT_EQ(valuesIn[2], valuesOut[2]);

	EXPECT_EQ(CMISS_OK, result = colourBar.getCentre(3, valuesOut));
	EXPECT_EQ(0.0, valuesOut[0]);
	EXPECT_EQ(0.0, valuesOut[1]);
	EXPECT_EQ(0.0, valuesOut[2]);
	EXPECT_EQ(CMISS_OK, result = colourBar.setCentre(3, valuesIn));
	EXPECT_EQ(CMISS_OK, result = colourBar.getCentre(3, valuesOut));
	EXPECT_EQ(valuesIn[0], valuesOut[0]);
	EXPECT_EQ(valuesIn[1], valuesOut[1]);
	EXPECT_EQ(valuesIn[2], valuesOut[2]);

	const double extendLengthIn = 0.6;
	EXPECT_EQ(0.05, value = colourBar.getExtendLength());
	EXPECT_EQ(CMISS_OK, result = colourBar.setExtendLength(extendLengthIn));
	EXPECT_EQ(extendLengthIn, value = colourBar.getExtendLength());

	const int labelDivisionsIn = 15;
	EXPECT_EQ(10, value = colourBar.getLabelDivisions());
	EXPECT_EQ(CMISS_OK, result = colourBar.setLabelDivisions(labelDivisionsIn));
	EXPECT_EQ(labelDivisionsIn, value = colourBar.getLabelDivisions());

	GraphicsMaterial tempMaterial = colourBar.getLabelMaterial();
	EXPECT_FALSE(tempMaterial.isValid());
	GraphicsMaterialModule materialModule = zinc.gm.getMaterialModule();
	GraphicsMaterial defaultMaterial = materialModule.getDefaultMaterial();
	EXPECT_TRUE(defaultMaterial.isValid());
	EXPECT_EQ(CMISS_OK, result = colourBar.setLabelMaterial(defaultMaterial));
	tempMaterial = colourBar.getLabelMaterial();
	EXPECT_EQ(defaultMaterial.getId(), tempMaterial.getId());
	GraphicsMaterial noMaterial;
	EXPECT_EQ(CMISS_OK, result = colourBar.setLabelMaterial(noMaterial));
	tempMaterial = colourBar.getLabelMaterial();
	EXPECT_FALSE(tempMaterial.isValid());

	const char *numberFormatIn = "%+5.2f %%";
	char *numberFormat = colourBar.getNumberFormat();
	EXPECT_STREQ("%+.4e", numberFormat);
	cmzn_deallocate(numberFormat);
	EXPECT_EQ(CMISS_OK, result = colourBar.setNumberFormat(numberFormatIn));
	numberFormat = colourBar.getNumberFormat();
	EXPECT_STREQ(numberFormatIn, numberFormat);
	cmzn_deallocate(numberFormat);

	EXPECT_EQ(CMISS_OK, result = colourBar.getSideAxis(3, valuesOut));
	EXPECT_EQ(0.1, valuesOut[0]);
	EXPECT_EQ(0.0, valuesOut[1]);
	EXPECT_EQ(0.0, valuesOut[2]);
	EXPECT_EQ(CMISS_OK, result = colourBar.setSideAxis(3, valuesIn));
	EXPECT_EQ(CMISS_OK, result = colourBar.getSideAxis(3, valuesOut));
	EXPECT_EQ(valuesIn[0], valuesOut[0]);
	EXPECT_EQ(valuesIn[1], valuesOut[1]);
	EXPECT_EQ(valuesIn[2], valuesOut[2]);

	const double tickLengthIn = 0.6;
	EXPECT_EQ(0.05, value = colourBar.getTickLength());
	EXPECT_EQ(CMISS_OK, result = colourBar.setTickLength(tickLengthIn));
	EXPECT_EQ(tickLengthIn, value = colourBar.getTickLength());
}

TEST(cmzn_glyph_colour_bar, invalid_attributes)
{
	ZincTestSetupSpectrum zinc;
	int result;

	cmzn_glyph_colour_bar_id noGlyphModuleColourBar = cmzn_glyph_module_create_colour_bar(static_cast<cmzn_glyph_module_id>(0), zinc.defaultSpectrum);
	EXPECT_EQ(static_cast<cmzn_glyph_colour_bar *>(0), noGlyphModuleColourBar);

	cmzn_glyph_colour_bar_id noSpectrumColourBar = cmzn_glyph_module_create_colour_bar(zinc.glyph_module, static_cast<cmzn_spectrum_id>(0));
	EXPECT_EQ(static_cast<cmzn_glyph_colour_bar *>(0), noSpectrumColourBar);

	cmzn_glyph_colour_bar_id colourBar = cmzn_glyph_module_create_colour_bar(zinc.glyph_module, zinc.defaultSpectrum);
	EXPECT_NE(static_cast<cmzn_glyph_colour_bar *>(0), colourBar);

	const char *nameIn = "Bob";
	char *name = cmzn_glyph_get_name(static_cast<cmzn_glyph_id>(0));
	EXPECT_EQ(static_cast<char *>(0), name);
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = cmzn_glyph_set_name(static_cast<cmzn_glyph_id>(0), nameIn));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = cmzn_glyph_set_name(cmzn_glyph_colour_bar_base_cast(colourBar), static_cast<const char *>(0)));

	double value;
	double valuesOut[3];
	const double valuesIn[3] = { 1.5, 2.0, 3.0 };

	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_get_axis(0, 3, valuesOut));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_get_axis(colourBar, 0, valuesOut));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_get_axis(colourBar, 3, 0));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_set_axis(0, 3, valuesIn));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_set_axis(colourBar, 0, valuesIn));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_set_axis(colourBar, 3, 0));

	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_get_centre(0, 3, valuesOut));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_get_centre(colourBar, 0, valuesOut));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_get_centre(colourBar, 3, 0));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_set_centre(0, 3, valuesIn));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_set_centre(colourBar, 0, valuesIn));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_set_centre(colourBar, 3, 0));

	const double extendLengthIn = 0.6;
	EXPECT_EQ(0.0, value = cmzn_glyph_colour_bar_get_extend_length(0));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_set_extend_length(0, extendLengthIn));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_set_extend_length(colourBar, -1.0));

	const int labelDivisionsIn = 15;
	EXPECT_EQ(0, value = cmzn_glyph_colour_bar_get_label_divisions(0));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_set_label_divisions(0, labelDivisionsIn));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_set_label_divisions(colourBar, 0));

	EXPECT_EQ(0, cmzn_glyph_colour_bar_get_label_material(0));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_set_label_material(0, 0));

	char *numberFormat = cmzn_glyph_colour_bar_get_number_format(0);
	EXPECT_EQ(static_cast<char *>(0), numberFormat);
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_set_number_format(0, "%+5.2f %%"));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_set_number_format(colourBar, 0));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_set_number_format(colourBar, ""));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_set_number_format(colourBar, "%F %g"));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_set_number_format(colourBar, "%500f"));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_set_number_format(colourBar, "%d"));

	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_get_side_axis(0, 3, valuesOut));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_get_side_axis(colourBar, 0, valuesOut));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_get_side_axis(colourBar, 3, 0));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_set_side_axis(0, 3, valuesIn));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_set_side_axis(colourBar, 0, valuesIn));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_set_side_axis(colourBar, 3, 0));

	const double tickLengthIn = 0.6;
	EXPECT_EQ(0.0, value = cmzn_glyph_colour_bar_get_tick_length(0));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_set_tick_length(0, tickLengthIn));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = cmzn_glyph_colour_bar_set_tick_length(colourBar, -1.0));

	cmzn_glyph_colour_bar_destroy(&colourBar);
}

TEST(cmzn_glyph_colour_bar, invalid_attributes_cpp)
{
	ZincTestSetupSpectrumCpp zinc;
	int result;

	Spectrum noSpectrum;
	GlyphColourBar noSpectrumColourBar = zinc.glyphModule.createColourBar(noSpectrum);
	EXPECT_FALSE(noSpectrumColourBar.isValid());

	GlyphColourBar colourBar = zinc.glyphModule.createColourBar(zinc.defaultSpectrum);
	EXPECT_TRUE(colourBar.isValid());

	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = colourBar.setName(0));

	double valuesOut[3];
	const double valuesIn[3] = { 1.5, 2.0, 3.0 };

	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = colourBar.getAxis(0, valuesOut));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = colourBar.getAxis(3, 0));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = colourBar.setAxis(0, valuesIn));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = colourBar.setAxis(3, 0));

	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = colourBar.getCentre(0, valuesOut));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = colourBar.getCentre(3, 0));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = colourBar.setCentre(0, valuesIn));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = colourBar.setCentre(3, 0));

	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = colourBar.setLabelDivisions(0));

	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = colourBar.setExtendLength(-1.0));

	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = colourBar.setNumberFormat(0));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = colourBar.setNumberFormat(""));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = colourBar.setNumberFormat("%F %g"));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = colourBar.setNumberFormat("%500f"));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = colourBar.setNumberFormat("%d"));

	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = colourBar.getSideAxis(0, valuesOut));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = colourBar.getSideAxis(3, 0));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = colourBar.setSideAxis(0, valuesIn));
	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = colourBar.setSideAxis(3, 0));

	EXPECT_EQ(CMISS_ERROR_ARGUMENT, result = colourBar.setTickLength(-1.0));
}
