/*******************************************************************************
FILE : spectrum_component.h

LAST MODIFIED : 4 October 2006

DESCRIPTION :
cmzn_spectrum_component structure and routines for describing and manipulating the
appearance of spectrums.
==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (SPECTRUM_COMPONENT_H)
#define SPECTRUM_COMPONENT_H

#include "general/enumerator.h"

/*
Global types
------------
*/



struct Spectrum_render_data
/*******************************************************************************
LAST MODIFIED : 4 October 2006

DESCRIPTION :
Used to pass information through iterator function to each setting when
rendering.
==============================================================================*/
{
	GLfloat *rgba;
	GLfloat material_rgba[4];
	GLfloat *data;
	int number_of_data_components;
}; /* struct Spectrum_render_data */

struct cmzn_spectrum_component
/*******************************************************************************
LAST MODIFIED : 14 July 1998

DESCRIPTION :
Stores one group of component for a single part of a spectrum scene.
==============================================================================*/
{
	/* non-accessed reference to spectrum */
	cmzn_spectrum_id spectrum;
	/* unique identifier for each component */
	int position;
	int component_number; /* Which data component this component uses (0 is first component)*/
	bool active, reverse; /* This corresponds to visiblity for graphical finite elements */
	enum cmzn_spectrum_component_scale_type component_scale;
	int changed;
	/* These specify the range of values over which the component operates */
	ZnReal maximum, minimum;
	/* These flags control whether the maximum, minumum values can be changed */
	int fix_maximum,fix_minimum;
	/* These flags control whether a component is transparent (has no effect)
		or is clamped at its extreme values outside it's minimum and maximum */
	bool extend_above, extend_below;
	/* These specify the limits of the converted value before it is rendered to
		a colour, i.e. red varies from <min_value> red at the <minimum> to
		<max_value> red at the <maximum> */
	ZnReal max_value, min_value;
	enum cmzn_spectrum_component_colour_mapping colour_mapping;
	ZnReal exaggeration, step_value;
	/* The number of bands in a banded contour and the proportion (out of 1000)
		of the black bands */
	int number_of_bands, black_band_proportion;
	bool is_field_lookup;
	/* CMZN_SPECTRUM_COMPONENT_INTERPOLATION_FIELD type */
	struct Computed_field *input_field;
	struct Computed_field *output_field;

#if defined (OPENGL_API)
	/* Texture number for banded and step spectrums */
	GLuint texture_id;
#endif /* defined (OPENGL_API) */

	/* For accessing objects */
	int access_count;
};

/*
Global functions
----------------
*/
PROTOTYPE_ENUMERATOR_FUNCTIONS(cmzn_spectrum_component_colour_mapping);

DECLARE_LIST_TYPES(cmzn_spectrum_component);

PROTOTYPE_OBJECT_FUNCTIONS(cmzn_spectrum_component);
PROTOTYPE_COPY_OBJECT_FUNCTION(cmzn_spectrum_component);
PROTOTYPE_LIST_FUNCTIONS(cmzn_spectrum_component);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(cmzn_spectrum_component, \
	position,int);

struct cmzn_spectrum_component *CREATE(cmzn_spectrum_component)(void);
/*******************************************************************************
LAST MODIFIED : 14 July 1998

DESCRIPTION :
Allocates memory and assigns fields for a struct cmzn_spectrum_component.
==============================================================================*/

int cmzn_spectrum_component_expand_maximum_component_index(
	struct cmzn_spectrum_component *component,void *component_index_void);
/*******************************************************************************
LAST MODIFIED : 27 September 2006

DESCRIPTION :
Iterator function to expand the integer stored at <component_index_void>
by the component numbers of each component so we can work out the maximum
component number used.  The first component_index is 0, so this means 1 component.
==============================================================================*/

int cmzn_spectrum_component_get_colour_components(
	struct cmzn_spectrum_component *component,void *colour_components_void);
/*******************************************************************************
LAST MODIFIED : 27 September 2006

DESCRIPTION :
Iterator function to accumulate the colour_components by setting bits
in <colour_components_void>.
==============================================================================*/

int cmzn_spectrum_component_copy_and_put_in_list(
	struct cmzn_spectrum_component *component,void *list_of_components_void);
/*******************************************************************************
LAST MODIFIED : 27 July 1998

DESCRIPTION :
cmzn_spectrum_component iterator function for copying a list_of_components.
Makes a copy of the component and puts it in the list_of_components.
==============================================================================*/

int cmzn_spectrum_component_get_black_band_proportion(struct cmzn_spectrum_component *component);
/*******************************************************************************
LAST MODIFIED : 31 July 1998

DESCRIPTION :
==============================================================================*/

int cmzn_spectrum_component_set_black_band_proportion(struct cmzn_spectrum_component *component,
	int proportion);
/*******************************************************************************
LAST MODIFIED : 31 July 1998

DESCRIPTION :
==============================================================================*/

int cmzn_spectrum_component_get_fix_minimum_flag(struct cmzn_spectrum_component *component);
/*******************************************************************************
LAST MODIFIED : 11 January 2001

DESCRIPTION :
Returns the fix_minimum flag of the cmzn_spectrum_component <spectrum>.
==============================================================================*/

int cmzn_spectrum_component_set_fix_minimum_flag(struct cmzn_spectrum_component *component,
	int fix_minimum);
/*******************************************************************************
LAST MODIFIED : 11 January 2001

DESCRIPTION :
Sets the fix_minimum flag of the cmzn_spectrum_component <component>.
==============================================================================*/

int cmzn_spectrum_component_get_fix_maximum_flag(struct cmzn_spectrum_component *component);
/*******************************************************************************
LAST MODIFIED : 11 January 2001

DESCRIPTION :
Returns the fix_maximum flag of the cmzn_spectrum_component <spectrum>.
==============================================================================*/

int cmzn_spectrum_component_set_fix_maximum_flag(struct cmzn_spectrum_component *component,
	int fix_maximum);
/*******************************************************************************
LAST MODIFIED : 11 January 2001

DESCRIPTION :
Sets the fix_maximum flag of the cmzn_spectrum_component <component>.
==============================================================================*/

int cmzn_spectrum_component_clear_changed(
	struct cmzn_spectrum_component *component,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Iterator function to set component->component_changed to 0 (unchanged).
==============================================================================*/

int cmzn_spectrum_component_enable(struct cmzn_spectrum_component *component,
	void *render_data_void);
/*******************************************************************************
LAST MODIFIED : 23 July 1998

DESCRIPTION :
==============================================================================*/

int cmzn_spectrum_component_activate(struct cmzn_spectrum_component *component,
	void *render_data_void);
/*******************************************************************************
LAST MODIFIED : 13 March 1998

DESCRIPTION :
Modifies the colour in the render data to represent the data value
passed in render data.
==============================================================================*/

int cmzn_spectrum_component_disable(struct cmzn_spectrum_component *component,
	void *render_data_void);
/*******************************************************************************
LAST MODIFIED : 23 July 1998

DESCRIPTION :
==============================================================================*/
#endif /* !defined (SPECTRUM_COMPONENT_H) */
