/*
 * Zinc Library Unit Tests
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtest/gtest.h>

#include <opencmiss/zinc/status.h>
#include <opencmiss/zinc/core.h>
#include <opencmiss/zinc/spectrum.h>

#include "opencmiss/zinc/fieldarithmeticoperators.hpp"
#include "opencmiss/zinc/fieldconstant.hpp"
#include "opencmiss/zinc/graphics.hpp"
#include "opencmiss/zinc/spectrum.hpp"

#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"

#include "test_resources.h"


TEST(cmzn_spectrummodule_api, valid_args)
{
	ZincTestSetup zinc;

	cmzn_spectrummodule_id sm = cmzn_context_get_spectrummodule(zinc.context);
	EXPECT_NE(static_cast<cmzn_spectrummodule *>(0), sm);

	int result = cmzn_spectrummodule_begin_change(sm);
	EXPECT_EQ(CMZN_OK, result);

	cmzn_spectrum_id spectrum = cmzn_spectrummodule_create_spectrum(sm);
	EXPECT_NE(static_cast<cmzn_spectrum *>(0), spectrum);

	result = cmzn_spectrum_set_name(spectrum, "new_default");
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_spectrummodule_end_change(sm);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_spectrummodule_set_default_spectrum(sm, spectrum);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_spectrum_set_managed(spectrum, 1);
	EXPECT_EQ(CMZN_OK, result);

	cmzn_spectrum_destroy(&spectrum);

	spectrum = cmzn_spectrummodule_find_spectrum_by_name(sm, "new_default");
	EXPECT_NE(static_cast<cmzn_spectrum *>(0), spectrum);

	cmzn_spectrum_destroy(&spectrum);

	spectrum = cmzn_spectrummodule_get_default_spectrum(sm);
	EXPECT_NE(static_cast<cmzn_spectrum *>(0), spectrum);

	cmzn_spectrum_destroy(&spectrum);

	cmzn_spectrummodule_destroy(&sm);
}

TEST(cmzn_spectrummodule_api, valid_args_cpp)
{
	ZincTestSetupCpp zinc;

	Spectrummodule sm = zinc.context.getSpectrummodule();
	EXPECT_TRUE(sm.isValid());

	int result = sm.beginChange();
	EXPECT_EQ(CMZN_OK, result);

	Spectrum spectrum = sm.createSpectrum();
	EXPECT_TRUE(spectrum.isValid());

	result = spectrum.setName("new_default");
	EXPECT_EQ(CMZN_OK, result);

	result = sm.endChange();
	EXPECT_EQ(CMZN_OK, result);

	result = sm.setDefaultSpectrum( spectrum);
	EXPECT_EQ(CMZN_OK, result);

	result = spectrum.setManaged(true);
	EXPECT_EQ(CMZN_OK, result);

	spectrum = sm.findSpectrumByName("new_default");
	EXPECT_TRUE(spectrum.isValid());

	spectrum = sm.getDefaultSpectrum();
	EXPECT_TRUE(spectrum.isValid());
}

TEST(cmzn_spectrum_api, valid_args)
{
	ZincTestSetup zinc;

	cmzn_spectrummodule_id sm = cmzn_context_get_spectrummodule(zinc.context);
	EXPECT_NE(static_cast<cmzn_spectrummodule *>(0), sm);

	int result = cmzn_spectrummodule_begin_change(sm);
	EXPECT_EQ(CMZN_OK, result);

	cmzn_spectrum_id spectrum = cmzn_spectrummodule_create_spectrum(sm);
	EXPECT_NE(static_cast<cmzn_spectrum *>(0), spectrum);

	result = cmzn_spectrum_set_name(spectrum, "new_default");
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_spectrummodule_end_change(sm);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_spectrum_set_managed(spectrum, 1);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_spectrum_is_managed(spectrum);
	EXPECT_EQ(1, result);

	cmzn_spectrumcomponent_id component1= cmzn_spectrum_create_spectrumcomponent(spectrum);
	EXPECT_NE(static_cast<cmzn_spectrumcomponent *>(0), component1);

	cmzn_spectrumcomponent_id component2= cmzn_spectrum_create_spectrumcomponent(spectrum);
	EXPECT_NE(static_cast<cmzn_spectrumcomponent *>(0), component2);

	cmzn_spectrumcomponent_id component1_clone= cmzn_spectrum_get_first_spectrumcomponent(spectrum);
	EXPECT_EQ(component1_clone, component1);

	cmzn_spectrumcomponent_id component2_clone = cmzn_spectrum_get_next_spectrumcomponent(spectrum, component1_clone);
	EXPECT_EQ(component2_clone, component2);

	cmzn_spectrumcomponent_id component3 = cmzn_spectrum_get_next_spectrumcomponent(spectrum, component2_clone);
	EXPECT_EQ(0, component3);

	cmzn_spectrumcomponent_destroy(&component1_clone);

	cmzn_spectrumcomponent_destroy(&component2_clone);

	component1_clone = cmzn_spectrum_get_previous_spectrumcomponent(spectrum, component2);
	EXPECT_EQ(component1_clone, component1);

	cmzn_spectrumcomponent_destroy(&component1_clone);

	EXPECT_EQ(CMZN_OK, result = cmzn_spectrumcomponent_set_range_maximum(component1, 20.0));

	double double_result = cmzn_spectrumcomponent_get_range_maximum(component1);
	EXPECT_EQ(20.0, double_result);

	EXPECT_EQ(CMZN_OK, result = cmzn_spectrumcomponent_set_active(component1, false));

	bool bool_result = cmzn_spectrumcomponent_is_active(component1);
	EXPECT_FALSE(bool_result);

	EXPECT_EQ(CMZN_OK, result = cmzn_spectrumcomponent_set_colour_reverse(component1, true));

	bool_result = cmzn_spectrumcomponent_is_colour_reverse(component1);
	EXPECT_TRUE(bool_result);

	// check on by default
	bool_result = cmzn_spectrumcomponent_is_extend_above(component1);
	EXPECT_TRUE(bool_result);

	EXPECT_EQ(CMZN_OK, result = cmzn_spectrumcomponent_set_extend_above(
		component1, false));

	bool_result = cmzn_spectrumcomponent_is_extend_above(component1);
	EXPECT_FALSE(bool_result);

	// check on by default
	bool_result = cmzn_spectrumcomponent_is_extend_below(component1);
	EXPECT_TRUE(bool_result);

	EXPECT_EQ(CMZN_OK, result = cmzn_spectrumcomponent_set_extend_below(component1, false));

	bool_result = cmzn_spectrumcomponent_is_extend_below(component1);
	EXPECT_FALSE(bool_result);

	result = cmzn_spectrumcomponent_set_field_component(component1,	2);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_spectrumcomponent_get_field_component(component1);
	EXPECT_EQ(2, result);

	// check off by default
	bool_result = cmzn_spectrumcomponent_is_fix_maximum(component1);
	EXPECT_FALSE(bool_result);

	EXPECT_EQ(CMZN_OK, result = cmzn_spectrumcomponent_set_fix_maximum(component1, true));

	bool_result = cmzn_spectrumcomponent_is_fix_maximum(component1);
	EXPECT_TRUE(bool_result);

	// check off by default
	bool_result = cmzn_spectrumcomponent_is_fix_minimum(component1);
	EXPECT_FALSE(bool_result);

	EXPECT_EQ(CMZN_OK, result = cmzn_spectrumcomponent_set_fix_minimum(component1, true));

	bool_result = cmzn_spectrumcomponent_is_fix_minimum(component1);
	EXPECT_TRUE(bool_result);

	result = cmzn_spectrumcomponent_set_number_of_bands(component1,	6);
	EXPECT_EQ(CMZN_OK, result);

	result = cmzn_spectrumcomponent_get_number_of_bands(component1);
	EXPECT_EQ(6, result);

	result = cmzn_spectrumcomponent_set_scale_type(component1,
		CMZN_SPECTRUMCOMPONENT_SCALE_TYPE_LOG);
	EXPECT_EQ(CMZN_OK, result);

	enum cmzn_spectrumcomponent_scale_type scale_type =
		cmzn_spectrumcomponent_get_scale_type(component1);
	EXPECT_EQ(CMZN_SPECTRUMCOMPONENT_SCALE_TYPE_LOG, scale_type);

	result = cmzn_spectrumcomponent_set_colour_mapping_type(component1,
		CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_MONOCHROME);
	EXPECT_EQ(CMZN_OK, result);

	enum cmzn_spectrumcomponent_colour_mapping_type _colour_mapping_type =
		cmzn_spectrumcomponent_get_colour_mapping_type(component1);
	EXPECT_EQ(CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_MONOCHROME, _colour_mapping_type);

	cmzn_spectrumcomponent_destroy(&component2);

	cmzn_spectrumcomponent_destroy(&component1);

	cmzn_spectrum_destroy(&spectrum);

	cmzn_spectrummodule_destroy(&sm);
}

TEST(cmzn_spectrum_api, valid_args_cpp)
{
	ZincTestSetupCpp zinc;

	Spectrummodule sm = zinc.context.getSpectrummodule();
	EXPECT_TRUE(sm.isValid());

	int result = sm.beginChange();
	EXPECT_EQ(CMZN_OK, result);

	Spectrum spectrum = sm.createSpectrum();
	EXPECT_TRUE(spectrum.isValid());

	result = spectrum.setName("new_default");
	EXPECT_EQ(CMZN_OK, result);

	result = sm.endChange();
	EXPECT_EQ(CMZN_OK, result);

	result = spectrum.setManaged(true);
	EXPECT_EQ(CMZN_OK, result);

	EXPECT_TRUE(spectrum.isManaged());

	Spectrumcomponent component1 = spectrum.createSpectrumcomponent();
	EXPECT_TRUE(component1.isValid());

	Spectrumcomponent component2 = spectrum.createSpectrumcomponent();
	EXPECT_TRUE(component2.isValid());

	Spectrumcomponent component1_clone = spectrum.getFirstSpectrumcomponent();
	EXPECT_EQ(component1_clone.getId(), component1.getId());

	Spectrumcomponent component2_clone = spectrum.getNextSpectrumcomponent(component1_clone);
	EXPECT_EQ(component2_clone.getId(), component2.getId());

	Spectrumcomponent component3 = spectrum.getNextSpectrumcomponent(component2_clone);
	EXPECT_FALSE(component3.isValid());

	component1_clone = spectrum.getPreviousSpectrumcomponent(component2);
	EXPECT_EQ(component1_clone.getId(), component1.getId());

	EXPECT_EQ(CMZN_OK, result = component1.setRangeMaximum(20.0));

	double double_result = component1.getRangeMaximum();
	EXPECT_EQ(20.0, double_result);

	EXPECT_EQ(CMZN_OK, result = component1.setActive(false));

	bool bool_result = component1.isActive();
	EXPECT_FALSE(bool_result);

	result = component1.setColourReverse(true);
	EXPECT_EQ(CMZN_OK, result);

	bool_result = component1.isColourReverse();
	EXPECT_TRUE(bool_result);

	// check on by default
	bool_result = component1.isExtendAbove();
	EXPECT_TRUE(bool_result);
	result = component1.setExtendAbove(false);
	EXPECT_EQ(CMZN_OK, result);
	bool_result = component1.isExtendAbove();
	EXPECT_FALSE(bool_result);

	// check on by default
	bool_result = component1.isExtendBelow();
	EXPECT_TRUE(bool_result);
	result = component1.setExtendBelow(false);
	EXPECT_EQ(CMZN_OK, result);
	bool_result = component1.isExtendBelow();
	EXPECT_FALSE(bool_result);

	result = component1.setFieldComponent(2);
	EXPECT_EQ(CMZN_OK, result);

	result = component1.getFieldComponent();
	EXPECT_EQ(2, result);

	result = component1.setNumberOfBands(6);
	EXPECT_EQ(CMZN_OK, result);

	result = component1.getNumberOfBands();
	EXPECT_EQ(6, result);

	enum Spectrumcomponent::ScaleType scale_type = component1.getScaleType();
	EXPECT_EQ(Spectrumcomponent::SCALE_TYPE_LINEAR, scale_type);

	result = component1.setScaleType(component1.SCALE_TYPE_LOG);
	EXPECT_EQ(CMZN_OK, result);

	scale_type = component1.getScaleType();
	EXPECT_EQ(Spectrumcomponent::SCALE_TYPE_LOG, scale_type);

	enum Spectrumcomponent::ColourMappingType colour_mapping_type = component1.getColourMappingType();
	EXPECT_EQ(Spectrumcomponent::COLOUR_MAPPING_TYPE_RAINBOW, colour_mapping_type);

	result = component1.setColourMappingType(component1.COLOUR_MAPPING_TYPE_MONOCHROME);
	EXPECT_EQ(CMZN_OK, result);

	colour_mapping_type = component1.getColourMappingType();
	EXPECT_EQ(Spectrumcomponent::COLOUR_MAPPING_TYPE_MONOCHROME, colour_mapping_type);

}

TEST(cmzn_spectrum_api, description_io_cpp)
{
	ZincTestSetupCpp zinc;

	Spectrummodule sm = zinc.context.getSpectrummodule();
	EXPECT_TRUE(sm.isValid());

    std::string stringBuffer = fileContents("spectrum/spectrum_description.json");

    EXPECT_EQ(CMZN_OK, sm.readDescription(stringBuffer.c_str()));

	Spectrum spectrum = sm.findSpectrumByName("default");
	EXPECT_TRUE(spectrum.isValid());
	EXPECT_TRUE(spectrum.isManaged());

	spectrum = sm.findSpectrumByName("new_default");
	EXPECT_TRUE(spectrum.isValid());
	EXPECT_TRUE(spectrum.isManaged());

	char *name = sm.getDefaultSpectrum().getName();
	EXPECT_EQ(0, strcmp("new_default", name));
	cmzn_deallocate(name);

	EXPECT_FALSE(spectrum.isMaterialOverwrite());

	EXPECT_EQ(2, spectrum.getNumberOfSpectrumcomponents());

	Spectrumcomponent component1 = spectrum.getFirstSpectrumcomponent();
	EXPECT_TRUE(component1.isValid());

	Spectrumcomponent component2 = spectrum.getNextSpectrumcomponent(component1);
	EXPECT_TRUE(component2.isValid());

	Spectrumcomponent component3 = spectrum.getNextSpectrumcomponent(component2);
	EXPECT_FALSE(component3.isValid());

	Spectrumcomponent component1_clone = spectrum.getPreviousSpectrumcomponent(component2);
	EXPECT_EQ(component1_clone.getId(), component1.getId());

	EXPECT_DOUBLE_EQ(20.0, component1.getRangeMaximum());

	EXPECT_DOUBLE_EQ(5.0, component1.getRangeMinimum());

	EXPECT_DOUBLE_EQ(0.5, component1.getColourMaximum());

	EXPECT_DOUBLE_EQ(0.4, component1.getColourMinimum());

	EXPECT_DOUBLE_EQ(0.4, component1.getBandedRatio());

	EXPECT_DOUBLE_EQ(10, component1.getStepValue());

	EXPECT_DOUBLE_EQ(2.0, component1.getExaggeration());

	EXPECT_FALSE(component1.isActive());

	EXPECT_TRUE(component1.isColourReverse());

	EXPECT_FALSE(component1.isExtendAbove());

	EXPECT_FALSE(component1.isExtendBelow());

	EXPECT_TRUE(component1.isFixMaximum());

	EXPECT_TRUE(component1.isFixMinimum());

	EXPECT_EQ(2, component1.getFieldComponent());

	EXPECT_EQ(6, component1.getNumberOfBands());

	EXPECT_EQ(Spectrumcomponent::SCALE_TYPE_LOG, component1.getScaleType());

	EXPECT_EQ(Spectrumcomponent::COLOUR_MAPPING_TYPE_MONOCHROME, component1.getColourMappingType());

	char *return_string = sm.writeDescription();
	EXPECT_TRUE(return_string != 0);
	cmzn_deallocate(return_string);
}

TEST(cmzn_spectrum_api, iteration_cpp)
{
	ZincTestSetupCpp zinc;

	Spectrummodule spectrummodule = zinc.context.getSpectrummodule();
	EXPECT_TRUE(spectrummodule.isValid());

	Spectrum xxx = spectrummodule.createSpectrum();
	EXPECT_TRUE(xxx.isValid());
	EXPECT_EQ(CMZN_OK, xxx.setName("xxx"));
	Spectrum zzz = spectrummodule.createSpectrum();
	EXPECT_TRUE(zzz.isValid());
	EXPECT_EQ(CMZN_OK, zzz.setName("zzz"));

	Spectrum aaa = spectrummodule.createSpectrum();
	EXPECT_TRUE(aaa.isValid());
	EXPECT_EQ(CMZN_OK, aaa.setName("aaa"));

	Spectrum defaultSpectrum = spectrummodule.getDefaultSpectrum();
	EXPECT_TRUE(defaultSpectrum.isValid());

	Spectrumiterator iter = spectrummodule.createSpectrumiterator();
	EXPECT_TRUE(iter.isValid());
	Spectrum s;
	EXPECT_EQ(aaa, s = iter.next());
	EXPECT_EQ(defaultSpectrum, s = iter.next());
	EXPECT_EQ(xxx, s = iter.next());
	EXPECT_EQ(zzz, s = iter.next());
	EXPECT_FALSE((s = iter.next()).isValid());
}

namespace {

int getNumberOfSpectrums(Spectrummodule& sm)
{
	int count = 0;
	Spectrumiterator iter = sm.createSpectrumiterator();
	while (iter.next().isValid())
	{
		++count;
	}
	return count;
}

}

TEST(ZincSpectrummodule, defineStandardSpectrums)
{
	ZincTestSetupCpp zinc;

	Spectrummodule sm = zinc.context.getSpectrummodule();
	EXPECT_TRUE(sm.isValid());

	EXPECT_EQ(1, getNumberOfSpectrums(sm));

	Spectrum spectrum = sm.getDefaultSpectrum();
	EXPECT_TRUE(spectrum.isValid());
	Spectrum tmpSpectrum = sm.findSpectrumByName("default");
	EXPECT_EQ(spectrum, tmpSpectrum);

	EXPECT_EQ(OK, sm.defineStandardSpectrums());
	EXPECT_EQ(3, getNumberOfSpectrums(sm));

	Spectrum monoSpectrum = sm.findSpectrumByName("mono");
	EXPECT_TRUE(monoSpectrum.isValid());
	EXPECT_EQ(1, monoSpectrum.getNumberOfSpectrumcomponents());
	Spectrumcomponent sc = monoSpectrum.getFirstSpectrumcomponent();
	EXPECT_TRUE(sc.isValid());
	EXPECT_EQ(sc.COLOUR_MAPPING_TYPE_MONOCHROME, sc.getColourMappingType());

	Spectrum rgbSpectrum = sm.findSpectrumByName("rgb");
	EXPECT_TRUE(rgbSpectrum.isValid());
	EXPECT_EQ(3, rgbSpectrum.getNumberOfSpectrumcomponents());
	sc = rgbSpectrum.getFirstSpectrumcomponent();
	EXPECT_TRUE(sc.isValid());
	EXPECT_EQ(sc.COLOUR_MAPPING_TYPE_RED, sc.getColourMappingType());
	sc = rgbSpectrum.getNextSpectrumcomponent(sc);
	EXPECT_TRUE(sc.isValid());
	EXPECT_EQ(sc.COLOUR_MAPPING_TYPE_GREEN, sc.getColourMappingType());
	sc = rgbSpectrum.getNextSpectrumcomponent(sc);
	EXPECT_TRUE(sc.isValid());
	EXPECT_EQ(sc.COLOUR_MAPPING_TYPE_BLUE, sc.getColourMappingType());
}

class SpectrumcallbackRecordChange : public Spectrummodulecallback
{
	int changeFlags;

	virtual void operator()(const Spectrummoduleevent &spectrummoduleevent)
	{
		this->changeFlags = spectrummoduleevent.getSummarySpectrumChangeFlags();
	}

public:
	SpectrumcallbackRecordChange() :
		Spectrummodulecallback(),
			changeFlags(Spectrum::CHANGE_FLAG_NONE)
	{ }

	void clear()
	{
		changeFlags = Spectrum::CHANGE_FLAG_NONE;
	}

	int getChangeSummary() const
	{
		return this->changeFlags;
	}

};

TEST(ZincSpectrummodulenotifier, changeCallback)
{
	ZincTestSetupCpp zinc;
	int result;

	Spectrummodule sm = zinc.context.getSpectrummodule();
	EXPECT_TRUE(sm.isValid());

	Spectrummodulenotifier spectrummodulenotifier = sm.createSpectrummodulenotifier();
	EXPECT_TRUE(spectrummodulenotifier.isValid());

	SpectrumcallbackRecordChange callback;
	EXPECT_EQ(CMZN_OK, result = spectrummodulenotifier.setCallback(callback));

	Spectrum spectrum = sm.getDefaultSpectrum();
	EXPECT_TRUE(spectrum.isValid());

	Spectrumcomponent component = spectrum.getFirstSpectrumcomponent();
	EXPECT_TRUE(component.isValid());

	Spectrum new_spectrum = sm.createSpectrum();
	EXPECT_TRUE(new_spectrum.isValid());
	EXPECT_EQ(Spectrum::CHANGE_FLAG_ADD, callback.getChangeSummary());

	new_spectrum = Spectrum();
	EXPECT_FALSE(new_spectrum.isValid());
	EXPECT_EQ(Spectrum::CHANGE_FLAG_REMOVE, callback.getChangeSummary());

	component = spectrum.getFirstSpectrumcomponent();
	EXPECT_TRUE(component.isValid());

	EXPECT_EQ(CMZN_OK, component.setColourMaximum(0.9));
	EXPECT_EQ(Spectrum::CHANGE_FLAG_FULL_RESULT |Spectrum::CHANGE_FLAG_DEFINITION,
		callback.getChangeSummary());
}

TEST(ZincSpectrum, autorange)
{
	ZincTestSetupCpp zinc;
	int result;

    EXPECT_EQ(OK, result = zinc.root_region.readFile(resourcePath("fieldmodule/cube.exformat").c_str()));

	Field coordinates = zinc.fm.findFieldByName("coordinates");
	EXPECT_TRUE(coordinates.isValid());
	const double scaleValues[] = { 2.2, 3.7, 1.5 };
	FieldConstant scale = zinc.fm.createFieldConstant(3, scaleValues);
	EXPECT_TRUE(scale.isValid());
	const double offsetValues[] = { -0.1, -0.2, 0.3 };
	FieldConstant offset = zinc.fm.createFieldConstant(3, offsetValues);
	EXPECT_TRUE(offset.isValid());
	FieldMultiply scaledCoordinates = scale*coordinates;
	FieldAdd offsetScaledCoordinates = scaledCoordinates + offset;
	EXPECT_TRUE(offsetScaledCoordinates.isValid());

	GraphicsSurfaces surfaces = zinc.scene.createGraphicsSurfaces();
	EXPECT_TRUE(surfaces.isValid());
	EXPECT_EQ(OK, surfaces.setCoordinateField(coordinates));
	EXPECT_EQ(OK, surfaces.setDataField(offsetScaledCoordinates));
	Spectrummodule sm = zinc.context.getSpectrummodule();
	EXPECT_TRUE(sm.isValid());
	Spectrum spectrum = sm.createSpectrum();

	Spectrumcomponent sc1 = spectrum.createSpectrumcomponent();
	EXPECT_TRUE(sc1.isValid());
	EXPECT_EQ(OK, sc1.setFieldComponent(1));
	EXPECT_EQ(OK, sc1.setColourMappingType(Spectrumcomponent::COLOUR_MAPPING_TYPE_RED));
	EXPECT_EQ(OK, sc1.setRangeMinimum(0.0));
	EXPECT_EQ(OK, sc1.setRangeMaximum(1.0));
	Spectrumcomponent sc2 = spectrum.createSpectrumcomponent();
	EXPECT_TRUE(sc2.isValid());
	EXPECT_EQ(OK, sc2.setFieldComponent(1));
	EXPECT_EQ(OK, sc2.setColourMappingType(Spectrumcomponent::COLOUR_MAPPING_TYPE_GREEN));
	EXPECT_EQ(OK, sc2.setRangeMinimum(0.5));
	EXPECT_EQ(OK, sc2.setRangeMaximum(5.0));
	EXPECT_EQ(OK, sc2.setFixMaximum(true));
	Spectrumcomponent sc3 = spectrum.createSpectrumcomponent();
	EXPECT_TRUE(sc3.isValid());
	EXPECT_EQ(OK, sc3.setFieldComponent(2));
	EXPECT_EQ(OK, sc3.setColourMappingType(Spectrumcomponent::COLOUR_MAPPING_TYPE_BLUE));
	EXPECT_EQ(OK, sc3.setRangeMinimum(0.2));
	EXPECT_EQ(OK, sc3.setFixMinimum(true));
	EXPECT_EQ(OK, sc3.setRangeMaximum(0.2));
	Spectrumcomponent sc4 = spectrum.createSpectrumcomponent();
	EXPECT_TRUE(sc4.isValid());
	EXPECT_EQ(OK, sc4.setFieldComponent(3));
	EXPECT_EQ(OK, sc4.setColourMappingType(Spectrumcomponent::COLOUR_MAPPING_TYPE_ALPHA));
	EXPECT_EQ(OK, sc4.setRangeMinimum(-200.0));
	EXPECT_EQ(OK, sc4.setRangeMaximum(200.0));
	Spectrumcomponent sc5 = spectrum.createSpectrumcomponent();
	EXPECT_TRUE(sc5.isValid());
	EXPECT_EQ(OK, sc5.setFieldComponent(3));
	EXPECT_EQ(OK, sc5.setColourMappingType(Spectrumcomponent::COLOUR_MAPPING_TYPE_STEP));
	EXPECT_EQ(OK, sc5.setRangeMinimum(50.0));
	EXPECT_EQ(OK, sc5.setRangeMaximum(150.0));
	Spectrumcomponent sc6 = spectrum.createSpectrumcomponent();
	EXPECT_TRUE(sc6.isValid());
	EXPECT_EQ(OK, sc6.setFieldComponent(4));
	EXPECT_EQ(OK, sc6.setColourMappingType(Spectrumcomponent::COLOUR_MAPPING_TYPE_BANDED));
	EXPECT_EQ(OK, sc6.setRangeMinimum(-1.23));
	EXPECT_EQ(OK, sc6.setRangeMaximum(4.56));

	EXPECT_EQ(OK, surfaces.setSpectrum(spectrum));

	Scenefiltermodule scenefiltermodule = zinc.context.getScenefiltermodule();
	Scenefilter defaultScenefilter = scenefiltermodule.getDefaultScenefilter();

	double maximums[3], minimums[3];
	EXPECT_EQ(3, zinc.scene.getSpectrumDataRange(defaultScenefilter, spectrum, 3, minimums, maximums));
	const double tolerance = 1.0E-7;
	EXPECT_NEAR(-0.1, minimums[0], tolerance);
	EXPECT_NEAR(-0.2, minimums[1], tolerance);
	EXPECT_NEAR(0.3, minimums[2], tolerance);
	EXPECT_NEAR(2.1, maximums[0], tolerance);
	EXPECT_NEAR(3.5, maximums[1], tolerance);
	EXPECT_NEAR(1.8, maximums[2], tolerance);

	EXPECT_EQ(OK, spectrum.autorange(zinc.scene, defaultScenefilter));

	EXPECT_NEAR(-0.1, sc1.getRangeMinimum(), tolerance);
	EXPECT_NEAR(2.1, sc1.getRangeMaximum(), tolerance);

	EXPECT_NEAR(1.0, sc2.getRangeMinimum(), tolerance);
	EXPECT_NEAR(5.0, sc2.getRangeMaximum(), tolerance);

	EXPECT_NEAR(0.2, sc3.getRangeMinimum(), tolerance);
	EXPECT_NEAR(3.5, sc3.getRangeMaximum(), tolerance);

	EXPECT_NEAR(0.3, sc4.getRangeMinimum(), tolerance);
	EXPECT_NEAR(1.8, sc4.getRangeMaximum(), tolerance);

	EXPECT_NEAR(1.2375, sc5.getRangeMinimum(), tolerance);
	EXPECT_NEAR(1.6125, sc5.getRangeMaximum(), tolerance);

	EXPECT_NEAR(-1.23, sc6.getRangeMinimum(), tolerance);
	EXPECT_NEAR(4.56, sc6.getRangeMaximum(), tolerance);
}
