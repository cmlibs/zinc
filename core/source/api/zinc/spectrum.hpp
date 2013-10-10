/**
 * FILE : spectrum.hpp
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_SPECTRUM_HPP__
#define CMZN_SPECTRUM_HPP__

#include "zinc/spectrum.h"

namespace OpenCMISS
{
namespace Zinc
{

class Spectrumcomponent
{
protected:
	cmzn_spectrumcomponent_id id;

public:

	Spectrumcomponent() :
		id(0)
	{
	}

	// takes ownership of C handle, responsibility for destroying it
	explicit Spectrumcomponent(
		cmzn_spectrumcomponent_id in_spectrumcomponent_id) :
		id(in_spectrumcomponent_id)
	{
	}

	Spectrumcomponent(const Spectrumcomponent& spectrumComponent) :
		id(cmzn_spectrumcomponent_access(spectrumComponent.id))
	{
	}

	Spectrumcomponent& operator=(const Spectrumcomponent& spectrumComponent)
	{
		cmzn_spectrumcomponent_id temp_id = cmzn_spectrumcomponent_access(
			spectrumComponent.id);
		if (0 != id)
		{
			cmzn_spectrumcomponent_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~Spectrumcomponent()
	{
		if (0 != id)
		{
			cmzn_spectrumcomponent_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	enum ScaleType
	{
		SCALE_INVALID = CMZN_SPECTRUMCOMPONENT_SCALE_INVALID,
		SCALE_LINEAR = CMZN_SPECTRUMCOMPONENT_SCALE_LINEAR,
		SCALE_LOG = CMZN_SPECTRUMCOMPONENT_SCALE_LOG
	};

	enum ColourMapping
	{
		COLOUR_MAPPING_INVALID = CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_INVALID,
		COLOUR_MAPPING_ALPHA = CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_ALPHA,
		COLOUR_MAPPING_BANDED = CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_BANDED,
		COLOUR_MAPPING_BLUE = CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_BLUE,
		COLOUR_MAPPING_GREEN = CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_GREEN,
		COLOUR_MAPPING_MONOCHROME = CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_MONOCHROME,
		COLOUR_MAPPING_RAINBOW = CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_RAINBOW,
		COLOUR_MAPPING_RED = CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_RED,
		COLOUR_MAPPING_STEP = CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_STEP,
		COLOUR_MAPPING_WHITE_TO_BLUE = CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_WHITE_TO_BLUE,
		COLOUR_MAPPING_WHITE_TO_RED = CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_WHITE_TO_RED,
		COLOUR_MAPPING_WHITE_TO_GREEN = CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_WHITE_TO_GREEN
	};

	cmzn_spectrumcomponent_id getId()
	{
		return id;
	}

	double getRangeMinumum()
	{
		return cmzn_spectrumcomponent_get_range_minimum(id);
	}

	int setRangeMinimum(double value)
	{
		return cmzn_spectrumcomponent_set_range_minimum(id, value);
	}

	double getRangeMaximum()
	{
		return cmzn_spectrumcomponent_get_range_maximum(id);
	}

	int setRangeMaximum(double value)
	{
		return cmzn_spectrumcomponent_set_range_maximum(id, value);
	}

	double getColourMinumum()
	{
		return cmzn_spectrumcomponent_get_colour_minimum(id);
	}

	int setColourMinimum(double value)
	{
		return cmzn_spectrumcomponent_set_colour_minimum(id,value);
	}

	double getColourMaxumum()
	{
		return cmzn_spectrumcomponent_get_colour_maximum(id);
	}

	int setColourMaximum(double value)
	{
		return cmzn_spectrumcomponent_set_colour_maximum(id, value);
	}

	double getStepValue()
	{
		return cmzn_spectrumcomponent_get_step_value(id);
	}

	int setStepValue(double value)
	{
		return cmzn_spectrumcomponent_set_step_value(id, value);
	}

	double getExaggeration()
	{
		return cmzn_spectrumcomponent_get_exaggeration(id);
	}

	int setExaggeration(double value)
	{
		return cmzn_spectrumcomponent_set_exaggeration(id, value);
	}

	double getBandedRatio()
	{
		return cmzn_spectrumcomponent_get_banded_ratio(id);
	}

	int setBandedRatio(double value)
	{
		return cmzn_spectrumcomponent_set_banded_ratio(id, value);
	}

	bool isActive()
	{
		return cmzn_spectrumcomponent_is_active(id);
	}

	int setActive(bool active)
	{
		return cmzn_spectrumcomponent_set_active(id, active);
	}

	bool isColourReverse()
	{
		return cmzn_spectrumcomponent_is_colour_reverse(id);
	}

	int setColourReverse(bool reverse)
	{
		return cmzn_spectrumcomponent_set_colour_reverse(id, reverse);
	}

	bool isExtendAbove()
	{
		return cmzn_spectrumcomponent_is_extend_above(id);
	}

	int setExtendAbove(bool extend_above)
	{
		return cmzn_spectrumcomponent_set_extend_above(id, extend_above);
	}

	bool isExtendBelow()
	{
		return cmzn_spectrumcomponent_is_extend_below(id);
	}

	int setExtendBelow(bool extend_below)
	{
		return cmzn_spectrumcomponent_set_extend_below(id, extend_below);
	}

	int getFieldComponent()
	{
		return cmzn_spectrumcomponent_get_field_component(id);
	}

	int setFieldComponent(int componentNumber)
	{
		return cmzn_spectrumcomponent_set_field_component(id,
			componentNumber);
	}

	int getNumberOfBands()
	{
		return cmzn_spectrumcomponent_get_number_of_bands(id);
	}

	int setNumberOfBands(int numberOfBands)
	{
		return cmzn_spectrumcomponent_set_number_of_bands(id, numberOfBands);
	}

	ScaleType getScaleType()
	{
		return static_cast<ScaleType>(cmzn_spectrumcomponent_get_scale_type(
			id));
	}

	int setScaleType(ScaleType scaleType)
	{
		return cmzn_spectrumcomponent_set_scale_type(id,
			static_cast<cmzn_spectrumcomponent_scale_type>(scaleType));
	}

	ColourMapping getColourMapping()
	{
		return static_cast<ColourMapping>(cmzn_spectrumcomponent_get_colour_mapping(
			id));
	}

	int setColourMapping(ColourMapping colourMapping)
	{
		return cmzn_spectrumcomponent_set_colour_mapping(id,
			static_cast<cmzn_spectrumcomponent_colour_mapping>(colourMapping));
	}

};

class Spectrum
{
protected:
	cmzn_spectrum_id id;

public:

	Spectrum() :
		id(0)
	{
	}

	// takes ownership of C handle, responsibility for destroying it
	explicit Spectrum(cmzn_spectrum_id in_spectrum_id) :
		id(in_spectrum_id)
	{
	}

	Spectrum(const Spectrum& spectrum) :
		id(cmzn_spectrum_access(spectrum.id))
	{
	}

	Spectrum& operator=(const Spectrum& spectrum)
	{
		cmzn_spectrum_id temp_id = cmzn_spectrum_access(spectrum.id);
		if (0 != id)
		{
			cmzn_spectrum_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~Spectrum()
	{
		if (0 != id)
		{
			cmzn_spectrum_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	cmzn_spectrum_id getId()
	{
		return id;
	}

	bool isManaged()
	{
		return cmzn_spectrum_is_managed(id);
	}

	int setManaged(bool value)
	{
		return cmzn_spectrum_set_managed(id, value);
	}

	int beginChange()
	{
		return cmzn_spectrum_begin_change(id);
	}

	int endChange()
	{
		return cmzn_spectrum_end_change(id);
	}

	char *getName()
	{
		return cmzn_spectrum_get_name(id);
	}

	int setName(const char *name)
	{
		return cmzn_spectrum_set_name(id, name);
	}

	int getNumberOfSpectrumcomponents()
	{
		return cmzn_spectrum_get_number_of_spectrumcomponents(id);
	}

	Spectrumcomponent createSpectrumcomponent()
	{
		return Spectrumcomponent(cmzn_spectrum_create_spectrumcomponent(id));
	}

	Spectrumcomponent getFirstSpectrumcomponent()
	{
		return Spectrumcomponent(cmzn_spectrum_get_first_spectrumcomponent(id));
	}

	Spectrumcomponent getNextSpectrumcomponent(Spectrumcomponent& refComponent)
	{
		return Spectrumcomponent(
			cmzn_spectrum_get_next_spectrumcomponent(id, refComponent.getId()));
	}

	Spectrumcomponent getPreviousSpectrumcomponent(Spectrumcomponent& refComponent)
	{
		return Spectrumcomponent(
			cmzn_spectrum_get_previous_spectrumcomponent(id, refComponent.getId()));
	}

	int moveSpectrumcomponentBefore(Spectrumcomponent& component,
		Spectrumcomponent& refComponent)
	{
		return cmzn_spectrum_move_spectrumcomponent_before(id, component.getId(),
			refComponent.getId());
	}

	int removeSpectrumcomponent(Spectrumcomponent component)
	{
		return cmzn_spectrum_remove_spectrumcomponent(id, component.getId());
	}

	int removeAllSpectrumcomponents()
	{
		return cmzn_spectrum_remove_all_spectrumcomponents(id);
	}

	bool isMaterialOverwrite()
	{
		return cmzn_spectrum_is_material_overwrite(id);
	}

	int setMaterialOverwrite(bool overwrite)
	{
		return cmzn_spectrum_set_material_overwrite(id, overwrite);
	}

};

class Spectrummodule
{
protected:
	cmzn_spectrummodule_id id;

public:

	Spectrummodule() :
		id(0)
	{
	}

	// takes ownership of C handle, responsibility for destroying it
	explicit Spectrummodule(cmzn_spectrummodule_id in_spectrummodule_id) :
		id(in_spectrummodule_id)
	{
	}

	Spectrummodule(const Spectrummodule& spectrummodule) :
		id(cmzn_spectrummodule_access(spectrummodule.id))
	{
	}

	Spectrummodule& operator=(const Spectrummodule& spectrummodule)
	{
		cmzn_spectrummodule_id temp_id = cmzn_spectrummodule_access(
			spectrummodule.id);
		if (0 != id)
		{
			cmzn_spectrummodule_destroy(&id);
		}
		id = temp_id;
		return *this;
	}

	~Spectrummodule()
	{
		if (0 != id)
		{
			cmzn_spectrummodule_destroy(&id);
		}
	}

	bool isValid()
	{
		return (0 != id);
	}

	cmzn_spectrummodule_id getId()
	{
		return id;
	}

	Spectrum createSpectrum()
	{
		return Spectrum(cmzn_spectrummodule_create_spectrum(id));
	}

	Spectrum findSpectrumByName(const char *name)
	{
		return Spectrum(cmzn_spectrummodule_find_spectrum_by_name(id, name));
	}

	int beginChange()
	{
		return cmzn_spectrummodule_begin_change(id);
	}

	int endChange()
	{
		return cmzn_spectrummodule_end_change(id);
	}

	Spectrum getDefaultSpectrum()
	{
		return Spectrum(cmzn_spectrummodule_get_default_spectrum(id));
	}

	int setDefaultSpectrum(Spectrum &spectrum)
	{
		return cmzn_spectrummodule_set_default_spectrum(id, spectrum.getId());
	}
};

} // namespace Zinc
}

#endif
