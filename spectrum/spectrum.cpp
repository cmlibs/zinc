
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

	int double_result = Cmiss_spectrum_component_get_attribute_real(
		component1, CMISS_SPECTRUM_COMPONENT_ATTRIBUTE_RANGE_MAXIMUM);
	EXPECT_EQ(20.0, double_result);

	result = Cmiss_spectrum_component_set_attribute_boolean(component1,
		CMISS_SPECTRUM_COMPONENT_ATTRIBUTE_IS_ACTIVE, false);
	EXPECT_EQ(CMISS_OK, result);

	bool bool_result = Cmiss_spectrum_component_get_attribute_boolean(component1,
		CMISS_SPECTRUM_COMPONENT_ATTRIBUTE_IS_ACTIVE);
	EXPECT_FALSE(bool_result);

	result = Cmiss_spectrum_component_set_attribute_boolean(component1,
		CMISS_SPECTRUM_COMPONENT_ATTRIBUTE_IS_COLOUR_REVERSE, true);
	EXPECT_EQ(CMISS_OK, result);

	bool_result = Cmiss_spectrum_component_get_attribute_boolean(component1,
		CMISS_SPECTRUM_COMPONENT_ATTRIBUTE_IS_COLOUR_REVERSE);
	EXPECT_TRUE(bool_result);

	result = Cmiss_spectrum_component_set_attribute_boolean(component1,
		CMISS_SPECTRUM_COMPONENT_ATTRIBUTE_IS_EXTEND_ABOVE, false);
	EXPECT_EQ(CMISS_OK, result);

	bool_result = Cmiss_spectrum_component_get_attribute_boolean(component1,
		CMISS_SPECTRUM_COMPONENT_ATTRIBUTE_IS_EXTEND_ABOVE);
	EXPECT_FALSE(bool_result);

	result = Cmiss_spectrum_component_set_attribute_boolean(component1,
		CMISS_SPECTRUM_COMPONENT_ATTRIBUTE_IS_EXTEND_BELOW, false);
	EXPECT_EQ(CMISS_OK, result);

	bool_result = Cmiss_spectrum_component_get_attribute_boolean(component1,
		CMISS_SPECTRUM_COMPONENT_ATTRIBUTE_IS_EXTEND_BELOW);
	EXPECT_FALSE(bool_result);

	result = Cmiss_spectrum_component_set_field_component(component1,	2);
	EXPECT_EQ(CMISS_OK, result);

	result = Cmiss_spectrum_component_get_field_component(component1);
	EXPECT_EQ(2, result);

	result = Cmiss_spectrum_component_set_number_of_bands(component1,	6);
	EXPECT_EQ(CMISS_OK, result);

	result = Cmiss_spectrum_component_get_number_of_bands(component1);
	EXPECT_EQ(6, result);

	result = Cmiss_spectrum_component_set_scale_type(component1,
		CMISS_SPECTRUM_COMPONENT_SCALE_LOG);
	EXPECT_EQ(CMISS_OK, result);

	enum Cmiss_spectrum_component_scale_type scale_type =
		Cmiss_spectrum_component_get_scale_type(component1);
	EXPECT_EQ(CMISS_SPECTRUM_COMPONENT_SCALE_LOG, scale_type);

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

	double double_result = component1.getAttributeReal(component1.ATTRIBUTE_RANGE_MAXIMUM);
	EXPECT_EQ(20.0, double_result);

	result = component1.setAttributeBoolean(component1.ATTRIBUTE_IS_ACTIVE, false);
	EXPECT_EQ(CMISS_OK, result);

	bool bool_result = component1.getAttributeBoolean(component1.ATTRIBUTE_IS_ACTIVE);
	EXPECT_FALSE(bool_result);

	result = component1.setAttributeBoolean(component1.ATTRIBUTE_IS_COLOUR_REVERSE, true);
	EXPECT_EQ(CMISS_OK, result);

	bool_result = component1.getAttributeBoolean(component1.ATTRIBUTE_IS_COLOUR_REVERSE);
	EXPECT_TRUE(bool_result);

	result = component1.setAttributeBoolean(component1.ATTRIBUTE_IS_EXTEND_ABOVE, false);
	EXPECT_EQ(CMISS_OK, result);

	bool_result = component1.getAttributeBoolean(component1.ATTRIBUTE_IS_EXTEND_ABOVE);
	EXPECT_FALSE(bool_result);

	result = component1.setAttributeBoolean(component1.ATTRIBUTE_IS_EXTEND_BELOW, false);
	EXPECT_EQ(CMISS_OK, result);

	bool_result = component1.getAttributeBoolean(component1.ATTRIBUTE_IS_EXTEND_BELOW);
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
