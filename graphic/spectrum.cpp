
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

	result = Cmiss_spectrum_set_attribute_integer(spectrum,
		CMISS_SPECTRUM_ATTRIBUTE_IS_MANAGED, 1);
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
	EXPECT_EQ(true, sm.isValid());

	int result = sm.beginChange();
	EXPECT_EQ(CMISS_OK, result);

	Spectrum spectrum = sm.createSpectrum();
	EXPECT_EQ(true, spectrum.isValid());

	result = spectrum.setName("default");
	EXPECT_EQ(CMISS_OK, result);

	result = sm.endChange();
	EXPECT_EQ(CMISS_OK, result);

	result = sm.setDefaultSpectrum( spectrum);
	EXPECT_EQ(CMISS_OK, result);

	result = spectrum.setAttributeInteger(
		spectrum.ATTRIBUTE_IS_MANAGED, 1);
	EXPECT_EQ(CMISS_OK, result);

	spectrum = sm.findSpectrumByName("default");
	EXPECT_EQ(true, spectrum.isValid());

	spectrum = sm.getDefaultSpectrum();
	EXPECT_EQ(true, spectrum.isValid());
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

	result = Cmiss_spectrum_set_attribute_integer(spectrum,
		CMISS_SPECTRUM_ATTRIBUTE_IS_MANAGED, 1);
	EXPECT_EQ(CMISS_OK, result);

	result = Cmiss_spectrum_get_attribute_integer(spectrum,
		CMISS_SPECTRUM_ATTRIBUTE_IS_MANAGED);
	EXPECT_EQ(1, result);

	result = Cmiss_spectrum_set_rainbow(spectrum);
	EXPECT_EQ(CMISS_OK, result);

	result = Cmiss_spectrum_set_minimum_and_maximum(spectrum, 10.0, 100.0);
	EXPECT_EQ(CMISS_OK, result);

	double double_result = Cmiss_spectrum_get_minimum(spectrum);
	EXPECT_EQ(10.0, double_result);

	double_result = Cmiss_spectrum_get_maximum(spectrum);
	EXPECT_EQ(100.0, double_result);

	Cmiss_spectrum_destroy(&spectrum);

	Cmiss_spectrum_module_destroy(&sm);
}

TEST(Cmiss_spectrum_api, valid_args_cpp)
{
	ZincTestSetupCpp zinc;

	SpectrumModule sm = zinc.gm.getSpectrumModule();
	EXPECT_EQ(true, sm.isValid());

	int result = sm.beginChange();
	EXPECT_EQ(CMISS_OK, result);

	Spectrum spectrum = sm.createSpectrum();
	EXPECT_EQ(true, spectrum.isValid());

	result = spectrum.setName("default");
	EXPECT_EQ(CMISS_OK, result);

	result = sm.endChange();
	EXPECT_EQ(CMISS_OK, result);

	result = spectrum.setAttributeInteger(
		spectrum.ATTRIBUTE_IS_MANAGED, 1);
	EXPECT_EQ(CMISS_OK, result);

	result = spectrum.getAttributeInteger(
		spectrum.ATTRIBUTE_IS_MANAGED);
	EXPECT_EQ(1, result);

	result = spectrum.setRainbow();
	EXPECT_EQ(CMISS_OK, result);

	result = spectrum.setMinimumAndMaximum(10.0, 100.0);
	EXPECT_EQ(CMISS_OK, result);

	double double_result = spectrum.getMinimum();
	EXPECT_EQ(10.0, double_result);

	double_result = spectrum.getMaximum();
	EXPECT_EQ(100.0, double_result);
}
