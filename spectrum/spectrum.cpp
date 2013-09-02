
#include <gtest/gtest.h>

#include <zinc/zincconfigure.h>
#include <zinc/status.h>
#include <zinc/core.h>
#include <zinc/spectrum.h>

#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"
#include "zinc/spectrum.hpp"

TEST(cmzn_spectrum_module_api, valid_args)
{
	ZincTestSetup zinc;

	cmzn_spectrum_module_id sm = cmzn_graphics_module_get_spectrum_module(zinc.gm);
	EXPECT_NE(static_cast<cmzn_spectrum_module *>(0), sm);

	int result = cmzn_spectrum_module_begin_change(sm);
	EXPECT_EQ(CMISS_OK, result);

	cmzn_spectrum_id spectrum = cmzn_spectrum_module_create_spectrum(sm);
	EXPECT_NE(static_cast<cmzn_spectrum *>(0), spectrum);

	result = cmzn_spectrum_set_name(spectrum, "default");
	EXPECT_EQ(CMISS_OK, result);

	result = cmzn_spectrum_module_end_change(sm);
	EXPECT_EQ(CMISS_OK, result);

	result = cmzn_spectrum_module_set_default_spectrum(sm, spectrum);
	EXPECT_EQ(CMISS_OK, result);

	result = cmzn_spectrum_set_managed(spectrum, 1);
	EXPECT_EQ(CMISS_OK, result);

	cmzn_spectrum_destroy(&spectrum);

	spectrum = cmzn_spectrum_module_find_spectrum_by_name(sm, "default");
	EXPECT_NE(static_cast<cmzn_spectrum *>(0), spectrum);

	cmzn_spectrum_destroy(&spectrum);

	spectrum = cmzn_spectrum_module_get_default_spectrum(sm);
	EXPECT_NE(static_cast<cmzn_spectrum *>(0), spectrum);

	cmzn_spectrum_destroy(&spectrum);

	cmzn_spectrum_module_destroy(&sm);
}

TEST(cmzn_spectrum_module_api, valid_args_cpp)
{
	ZincTestSetupCpp zinc;

	SpectrumModule sm = zinc.gm.getSpectrumModule();
	EXPECT_TRUE(sm.isValid());

	int result = sm.beginChange();
	EXPECT_EQ(CMISS_OK, result);

	Spectrum spectrum = sm.createSpectrum();
	EXPECT_TRUE(spectrum.isValid());

	result = spectrum.setName("default");
	EXPECT_EQ(CMISS_OK, result);

	result = sm.endChange();
	EXPECT_EQ(CMISS_OK, result);

	result = sm.setDefaultSpectrum( spectrum);
	EXPECT_EQ(CMISS_OK, result);

	result = spectrum.setManaged(true);
	EXPECT_EQ(CMISS_OK, result);

	spectrum = sm.findSpectrumByName("default");
	EXPECT_TRUE(spectrum.isValid());

	spectrum = sm.getDefaultSpectrum();
	EXPECT_TRUE(spectrum.isValid());
}

TEST(cmzn_spectrum_api, valid_args)
{
	ZincTestSetup zinc;

	cmzn_spectrum_module_id sm = cmzn_graphics_module_get_spectrum_module(zinc.gm);
	EXPECT_NE(static_cast<cmzn_spectrum_module *>(0), sm);

	int result = cmzn_spectrum_module_begin_change(sm);
	EXPECT_EQ(CMISS_OK, result);

	cmzn_spectrum_id spectrum = cmzn_spectrum_module_create_spectrum(sm);
	EXPECT_NE(static_cast<cmzn_spectrum *>(0), spectrum);

	result = cmzn_spectrum_set_name(spectrum, "default");
	EXPECT_EQ(CMISS_OK, result);

	result = cmzn_spectrum_module_end_change(sm);
	EXPECT_EQ(CMISS_OK, result);

	result = cmzn_spectrum_set_managed(spectrum, 1);
	EXPECT_EQ(CMISS_OK, result);

	result = cmzn_spectrum_is_managed(spectrum);
	EXPECT_EQ(1, result);

	cmzn_spectrum_component_id component1= cmzn_spectrum_create_component(spectrum);
	EXPECT_NE(static_cast<cmzn_spectrum_component *>(0), component1);

	cmzn_spectrum_component_id component2= cmzn_spectrum_create_component(spectrum);
	EXPECT_NE(static_cast<cmzn_spectrum_component *>(0), component2);

	cmzn_spectrum_component_id component1_clone= cmzn_spectrum_get_first_component(spectrum);
	EXPECT_EQ(component1_clone, component1);

	cmzn_spectrum_component_id component2_clone = cmzn_spectrum_get_next_component(spectrum, component1_clone);
	EXPECT_EQ(component2_clone, component2);

	cmzn_spectrum_component_id component3 = cmzn_spectrum_get_next_component(spectrum, component2_clone);
	EXPECT_EQ(0, component3);

	cmzn_spectrum_component_destroy(&component1_clone);

	cmzn_spectrum_component_destroy(&component2_clone);

	component1_clone = cmzn_spectrum_get_previous_component(spectrum, component2);
	EXPECT_EQ(component1_clone, component1);

	cmzn_spectrum_component_destroy(&component1_clone);

	EXPECT_EQ(CMISS_OK, result = cmzn_spectrum_component_set_range_maximum(component1, 20.0));

	double double_result = cmzn_spectrum_component_get_range_maximum(component1);
	EXPECT_EQ(20.0, double_result);

	EXPECT_EQ(CMISS_OK, result = cmzn_spectrum_component_set_active(component1, false));

	bool bool_result = cmzn_spectrum_component_is_active(component1);
	EXPECT_FALSE(bool_result);

	EXPECT_EQ(CMISS_OK, result = cmzn_spectrum_component_set_colour_reverse(component1, true));

	bool_result = cmzn_spectrum_component_is_colour_reverse(component1);
	EXPECT_TRUE(bool_result);

	EXPECT_EQ(CMISS_OK, result = cmzn_spectrum_component_set_extend_above(
		component1, false));

	bool_result = cmzn_spectrum_component_is_extend_above(component1);
	EXPECT_FALSE(bool_result);

	EXPECT_EQ(CMISS_OK, result = cmzn_spectrum_component_set_extend_below(component1, false));

	bool_result = cmzn_spectrum_component_is_extend_below(component1);
	EXPECT_FALSE(bool_result);

	result = cmzn_spectrum_component_set_field_component(component1,	2);
	EXPECT_EQ(CMISS_OK, result);

	result = cmzn_spectrum_component_get_field_component(component1);
	EXPECT_EQ(2, result);

	result = cmzn_spectrum_component_set_number_of_bands(component1,	6);
	EXPECT_EQ(CMISS_OK, result);

	result = cmzn_spectrum_component_get_number_of_bands(component1);
	EXPECT_EQ(6, result);

	result = cmzn_spectrum_component_set_scale_type(component1,
		CMISS_SPECTRUM_COMPONENT_SCALE_LOG);
	EXPECT_EQ(CMISS_OK, result);

	enum cmzn_spectrum_component_scale_type scale_type =
		cmzn_spectrum_component_get_scale_type(component1);
	EXPECT_EQ(CMISS_SPECTRUM_COMPONENT_SCALE_LOG, scale_type);

	result = cmzn_spectrum_component_set_colour_mapping(component1,
		CMISS_SPECTRUM_COMPONENT_COLOUR_MAPPING_MONOCHROME);
	EXPECT_EQ(CMISS_OK, result);

	enum cmzn_spectrum_component_colour_mapping _colour_mapping =
		cmzn_spectrum_component_get_colour_mapping(component1);
	EXPECT_EQ(CMISS_SPECTRUM_COMPONENT_COLOUR_MAPPING_MONOCHROME, _colour_mapping);

	cmzn_spectrum_component_destroy(&component2);

	cmzn_spectrum_component_destroy(&component1);

	cmzn_spectrum_destroy(&spectrum);

	cmzn_spectrum_module_destroy(&sm);
}

TEST(cmzn_spectrum_api, valid_args_cpp)
{
	ZincTestSetupCpp zinc;

	SpectrumModule sm = zinc.gm.getSpectrumModule();
	EXPECT_TRUE(sm.isValid());

	int result = sm.beginChange();
	EXPECT_EQ(CMISS_OK, result);

	Spectrum spectrum = sm.createSpectrum();
	EXPECT_TRUE(spectrum.isValid());

	result = spectrum.setName("default");
	EXPECT_EQ(CMISS_OK, result);

	result = sm.endChange();
	EXPECT_EQ(CMISS_OK, result);

	result = spectrum.setManaged(true);
	EXPECT_EQ(CMISS_OK, result);

	EXPECT_TRUE(spectrum.isManaged());

	SpectrumComponent component1 = spectrum.createComponent();
	EXPECT_TRUE(component1.isValid());


	SpectrumComponent component2 = spectrum.createComponent();
	EXPECT_TRUE(component2.isValid());

	SpectrumComponent component1_clone = spectrum.getFirstComponent();
	EXPECT_EQ(component1_clone.getId(), component1.getId());

	SpectrumComponent component2_clone = spectrum.getNextComponent(component1_clone);
	EXPECT_EQ(component2_clone.getId(), component2.getId());

	SpectrumComponent component3 = spectrum.getNextComponent(component2_clone);
	EXPECT_FALSE(component3.isValid());

	component1_clone = spectrum.getPreviousComponent(component2);
	EXPECT_EQ(component1_clone.getId(), component1.getId());

	EXPECT_EQ(CMISS_OK, result = component1.setRangeMaximum(20.0));

	double double_result = component1.getRangeMaximum();
	EXPECT_EQ(20.0, double_result);

	EXPECT_EQ(CMISS_OK, result = component1.setActive(false));

	bool bool_result = component1.isActive();
	EXPECT_FALSE(bool_result);

	result = component1.setColourReverse(true);
	EXPECT_EQ(CMISS_OK, result);

	bool_result = component1.isColourReverse();
	EXPECT_TRUE(bool_result);

	result = component1.setExtendAbove(false);
	EXPECT_EQ(CMISS_OK, result);

	bool_result = component1.isExtendAbove();
	EXPECT_FALSE(bool_result);

	result = component1.setExtendBelow(false);
	EXPECT_EQ(CMISS_OK, result);

	bool_result = component1.isExtendBelow();
	EXPECT_FALSE(bool_result);

	result = component1.setFieldComponent(2);
	EXPECT_EQ(CMISS_OK, result);

	result = component1.getFieldComponent();
	EXPECT_EQ(2, result);

	result = component1.setNumberOfBands(6);
	EXPECT_EQ(CMISS_OK, result);

	result = component1.getNumberOfBands();
	EXPECT_EQ(6, result);

	result = component1.setScaleType(component1.SCALE_LOG);
	EXPECT_EQ(CMISS_OK, result);

	enum SpectrumComponent::ScaleType scale_type = component1.getScaleType();
	EXPECT_EQ(SpectrumComponent::SCALE_LOG, scale_type);

	result = component1.setColourMapping(component1.COLOUR_MAPPING_MONOCHROME);
	EXPECT_EQ(CMISS_OK, result);

	enum SpectrumComponent::ColourMapping colour_mapping = component1.getColourMapping();
	EXPECT_EQ(SpectrumComponent::COLOUR_MAPPING_MONOCHROME, colour_mapping);

}
