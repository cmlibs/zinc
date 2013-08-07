
#include <gtest/gtest.h>

#include <zinc/zincconfigure.h>
#include <zinc/status.h>
#include <zinc/core.h>
#include <zinc/spectrum.h>

#include "zinctestsetup.hpp"
#include "zinctestsetupcpp.hpp"
#include "zinc/spectrum.hpp"

TEST(Cmiss_spectrum_module_api, valid_args)
{
	ZincTestSetup zinc;

	Cmiss_spectrum_module_id sm = Cmiss_graphics_module_get_spectrum_module(zinc.gm);
	EXPECT_NE(static_cast<Cmiss_spectrum_module *>(0), sm);

	int result = Cmiss_spectrum_module_begin_change(sm);
	EXPECT_EQ(CMISS_OK, result);

	Cmiss_spectrum_id spectrum = Cmiss_spectrum_module_create_spectrum(sm);
	EXPECT_NE(static_cast<Cmiss_spectrum *>(0), spectrum);

	result = Cmiss_spectrum_set_name(spectrum, "default");
	EXPECT_EQ(CMISS_OK, result);

	result = Cmiss_spectrum_module_end_change(sm);
	EXPECT_EQ(CMISS_OK, result);

	result = Cmiss_spectrum_module_set_default_spectrum(sm, spectrum);
	EXPECT_EQ(CMISS_OK, result);

	result = Cmiss_spectrum_set_managed(spectrum, 1);
	EXPECT_EQ(CMISS_OK, result);

	Cmiss_spectrum_destroy(&spectrum);

	spectrum = Cmiss_spectrum_module_find_spectrum_by_name(sm, "default");
	EXPECT_NE(static_cast<Cmiss_spectrum *>(0), spectrum);

	Cmiss_spectrum_destroy(&spectrum);

	spectrum = Cmiss_spectrum_module_get_default_spectrum(sm);
	EXPECT_NE(static_cast<Cmiss_spectrum *>(0), spectrum);

	Cmiss_spectrum_destroy(&spectrum);

	Cmiss_spectrum_module_destroy(&sm);
}

TEST(Cmiss_spectrum_module_api, valid_args_cpp)
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

TEST(Cmiss_spectrum_api, valid_args)
{
	ZincTestSetup zinc;

	Cmiss_spectrum_module_id sm = Cmiss_graphics_module_get_spectrum_module(zinc.gm);
	EXPECT_NE(static_cast<Cmiss_spectrum_module *>(0), sm);

	int result = Cmiss_spectrum_module_begin_change(sm);
	EXPECT_EQ(CMISS_OK, result);

	Cmiss_spectrum_id spectrum = Cmiss_spectrum_module_create_spectrum(sm);
	EXPECT_NE(static_cast<Cmiss_spectrum *>(0), spectrum);

	result = Cmiss_spectrum_set_name(spectrum, "default");
	EXPECT_EQ(CMISS_OK, result);

	result = Cmiss_spectrum_module_end_change(sm);
	EXPECT_EQ(CMISS_OK, result);

	result = Cmiss_spectrum_set_managed(spectrum, 1);
	EXPECT_EQ(CMISS_OK, result);

	result = Cmiss_spectrum_is_managed(spectrum);
	EXPECT_EQ(1, result);

	Cmiss_spectrum_component_id component1= Cmiss_spectrum_create_component(spectrum);
	EXPECT_NE(static_cast<Cmiss_spectrum_component *>(0), component1);

	result = Cmiss_spectrum_set_minimum_and_maximum(spectrum, 10.0, 100.0);
	EXPECT_EQ(CMISS_OK, result);

	double double_result = Cmiss_spectrum_get_minimum(spectrum);
	EXPECT_EQ(10.0, double_result);

	double_result = Cmiss_spectrum_get_maximum(spectrum);
	EXPECT_EQ(100.0, double_result);

	Cmiss_spectrum_component_id component2= Cmiss_spectrum_create_component(spectrum);
	EXPECT_NE(static_cast<Cmiss_spectrum_component *>(0), component2);

	Cmiss_spectrum_component_id component1_clone= Cmiss_spectrum_get_first_component(spectrum);
	EXPECT_EQ(component1_clone, component1);

	Cmiss_spectrum_component_id component2_clone = Cmiss_spectrum_get_next_component(spectrum, component1_clone);
	EXPECT_EQ(component2_clone, component2);

	Cmiss_spectrum_component_id component3 = Cmiss_spectrum_get_next_component(spectrum, component2_clone);
	EXPECT_EQ(0, component3);

	Cmiss_spectrum_component_destroy(&component1_clone);

	Cmiss_spectrum_component_destroy(&component2_clone);

	component1_clone = Cmiss_spectrum_get_previous_component(spectrum, component2);
	EXPECT_EQ(component1_clone, component1);

	Cmiss_spectrum_component_destroy(&component1_clone);

	result = Cmiss_spectrum_component_set_attribute_real(
		component1, CMISS_SPECTRUM_COMPONENT_ATTRIBUTE_RANGE_MAXIMUM, 20.0);
	EXPECT_EQ(CMISS_OK, result);

	double_result = Cmiss_spectrum_component_get_attribute_real(
		component1, CMISS_SPECTRUM_COMPONENT_ATTRIBUTE_RANGE_MAXIMUM);
	EXPECT_EQ(20.0, double_result);

	result = Cmiss_spectrum_component_set_active(component1, false);
	EXPECT_EQ(CMISS_OK, result);

	bool bool_result = Cmiss_spectrum_component_get_active(component1);
	EXPECT_EQ(false, bool_result);

	result = Cmiss_spectrum_component_set_reverse_flag(component1, true);
	EXPECT_EQ(CMISS_OK, result);

	bool_result = Cmiss_spectrum_component_get_reverse_flag(component1);
	EXPECT_EQ(true, bool_result);

	result = Cmiss_spectrum_component_set_extend_above_flag(component1, false);
	EXPECT_EQ(CMISS_OK, result);

	bool_result = Cmiss_spectrum_component_get_extend_above_flag(component1);
	EXPECT_EQ(false, bool_result);

	result = Cmiss_spectrum_component_set_extend_below_flag(component1, false);
	EXPECT_EQ(CMISS_OK, result);

	bool_result = Cmiss_spectrum_component_get_extend_below_flag(component1);
	EXPECT_EQ(false, bool_result);

	result = Cmiss_spectrum_component_set_field_component_lookup_number(component1,	2);
	EXPECT_EQ(CMISS_OK, result);

	result = Cmiss_spectrum_component_get_field_component_lookup_number(component1);
	EXPECT_EQ(2, result);

	result = Cmiss_spectrum_component_set_number_of_bands(component1,	6);
	EXPECT_EQ(CMISS_OK, result);

	result = Cmiss_spectrum_component_get_number_of_bands(component1);
	EXPECT_EQ(6, result);

	result = Cmiss_spectrum_component_set_interpolation_mode(component1,
		CMISS_SPECTRUM_COMPONENT_INTERPOLATION_LOG);
	EXPECT_EQ(CMISS_OK, result);

	enum Cmiss_spectrum_component_interpolation_mode interpolation_mode =
		Cmiss_spectrum_component_get_interpolation_mode(component1);
	EXPECT_EQ(CMISS_SPECTRUM_COMPONENT_INTERPOLATION_LOG, interpolation_mode);

	result = Cmiss_spectrum_component_set_colour_mapping(component1,
		CMISS_SPECTRUM_COMPONENT_COLOUR_MAPPING_MONOCHROME);
	EXPECT_EQ(CMISS_OK, result);

	enum Cmiss_spectrum_component_colour_mapping _colour_mapping =
		Cmiss_spectrum_component_get_colour_mapping(component1);
	EXPECT_EQ(CMISS_SPECTRUM_COMPONENT_COLOUR_MAPPING_MONOCHROME, _colour_mapping);

	Cmiss_spectrum_component_destroy(&component2);

	Cmiss_spectrum_component_destroy(&component1);

	Cmiss_spectrum_destroy(&spectrum);

	Cmiss_spectrum_module_destroy(&sm);
}

TEST(Cmiss_spectrum_api, valid_args_cpp)
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

	result = spectrum.setMinimumAndMaximum(10.0, 100.0);
	EXPECT_EQ(CMISS_OK, result);

	double double_result = spectrum.getMinimum();
	EXPECT_EQ(10.0, double_result);

	double_result = spectrum.getMaximum();
	EXPECT_EQ(100.0, double_result);

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

	result = component1.setAttributeReal(component1.ATTRIBUTE_RANGE_MAXIMUM, 20.0);
	EXPECT_EQ(CMISS_OK, result);

	double_result = component1.getAttributeReal(
		component1.ATTRIBUTE_RANGE_MAXIMUM);
	EXPECT_EQ(20.0, double_result);

	result = component1.setActive(false);
	EXPECT_EQ(CMISS_OK, result);

	bool bool_result = component1.getActive();
	EXPECT_EQ(false, bool_result);

	result = component1.setReverseFlag(true);
	EXPECT_EQ(CMISS_OK, result);

	bool_result = component1.getReverseFlag();
	EXPECT_EQ(true, bool_result);

	result = component1.setExtendAboveFlag(false);
	EXPECT_EQ(CMISS_OK, result);

	bool_result = component1.getExtendAboveFlag();
	EXPECT_EQ(false, bool_result);

	result = component1.setExtendBelowFlag(false);
	EXPECT_EQ(CMISS_OK, result);

	bool_result = component1.getExtendBelowFlag();
	EXPECT_EQ(false, bool_result);

	result = component1.setFieldComponentLookupNumber(2);
	EXPECT_EQ(CMISS_OK, result);

	result = component1.getFieldComponentLookupNumber();
	EXPECT_EQ(2, result);

	result = component1.setNumberOfBands(6);
	EXPECT_EQ(CMISS_OK, result);

	result = component1.getNumberOfBands();
	EXPECT_EQ(6, result);

	result = component1.setInterpolationMode(component1.INTERPOLATION_LOG);
	EXPECT_EQ(CMISS_OK, result);

	enum SpectrumComponent::InterpolationMode interpolation_mode = component1.getInterpolationMode();
	EXPECT_EQ(SpectrumComponent::INTERPOLATION_LOG, interpolation_mode);

	result = component1.setColourMapping(component1.COLOUR_MAPPING_MONOCHROME);
	EXPECT_EQ(CMISS_OK, result);

	enum SpectrumComponent::ComponentColourMapping colour_mapping = component1.getColourMapping();
	EXPECT_EQ(SpectrumComponent::COLOUR_MAPPING_MONOCHROME, colour_mapping);

}
