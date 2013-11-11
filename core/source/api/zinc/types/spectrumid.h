/**
 * FILE : spectrumid.h
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMZN_SPECTRUMID_H__
#define CMZN_SPECTRUMID_H__

struct cmzn_spectrumcomponent;
typedef struct cmzn_spectrumcomponent *cmzn_spectrumcomponent_id;

struct cmzn_spectrum;
typedef struct cmzn_spectrum *cmzn_spectrum_id;

struct cmzn_spectrummodule;
typedef struct cmzn_spectrummodule *cmzn_spectrummodule_id;

enum cmzn_spectrumcomponent_scale_type
{
	CMZN_SPECTRUMCOMPONENT_SCALE_TYPE_INVALID = 0,
	CMZN_SPECTRUMCOMPONENT_SCALE_TYPE_LINEAR = 1,
	/*!< default scale type
	 * The colour value on spectrum will be interpolated linearly in range when
	 * this mode is chosen.
	 */
	CMZN_SPECTRUMCOMPONENT_SCALE_TYPE_LOG = 2
};

/**
 * Colour mapping mode for specctrum component. Appearances of these mappings
 * can ne alterd by the various APIs provided in spectrum and spectrum components
 * APIs.
 */
enum cmzn_spectrumcomponent_colour_mapping_type
{
	CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_INVALID = 0,
	CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_ALPHA = 1,
	/*!< This colour mapping alters the alpha (transparency value) for
	 * primitives.
	 * This mode does not alter the rgb value and
	 * should be used with other spectrum component or with
	 * overwrite_material set to 0 in spectrum.
	 */
	CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_BANDED = 2,
	/*!< This colour mapping create non-coloured strips/bands.
	 *  appearances can be altered by value of CMZN_SPECTRUMCOMPONENT_ATTRIBUTE_BANDED_RATIO
	 *  and value of number of bands.
	 *  This mode does not alter the rgb value except for the bands and
	 *  should be used with other spectrum component or with
	 *  overwrite_material set to 0 in spectrum.
	 */
	CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_BLUE = 3,
	/*!< This colour mapping create a colour spectrum from black to blue.
	 */
	CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_GREEN = 4,
	/*!< This colour mapping create a colour spectrum from black to green.
	 */
	CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_MONOCHROME = 5,
	/*!< This colour mapping create a monochrome (grey scale) spectrum.
	 */
	CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_RAINBOW = 6,
	/*!< default colour mapping type
	 * This colour mapping create a spectrum from blue to red, similar
	 * to the colour of a rainbow.
	 */
	CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_RED = 7,
	/*!< This colour mapping create a colour spectrum from black to red.
	 */
	CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_STEP = 8,
	/*!< This colour mapping create a spectrum with only two colours, red and green.
	 * The boundary between red and green can be altered by
	 * CMZN_SPECTRUMCOMPONENT_ATTRIBUTE_STEP_VALUE.
	 */
	CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_WHITE_TO_BLUE = 9,
	/*!< This colour mapping create a colour spectrum from black to blue.
	 */
	CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_WHITE_TO_RED = 10,
	/*!< This colour mapping create a colour spectrum from black to red.
	 */
	CMZN_SPECTRUMCOMPONENT_COLOUR_MAPPING_TYPE_WHITE_TO_GREEN = 11
	/*!< This colour mapping create a colour spectrum from black to green.
	 */
};

#endif
