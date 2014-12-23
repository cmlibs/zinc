/*******************************************************************************
FILE : spectrum.h

LAST MODIFIED : 15 October 1998

DESCRIPTION :
Spectrum structures and support code.
==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined(SPECTRUM_H)
#define SPECTRUM_H

#include "zinc/zincconfigure.h"
#include "zinc/spectrum.h"

#if defined (USE_GLEW)
#include <GL/glew.h>
#endif /* USE_GLEW */
#include "general/value.h"
#include "general/list.h"
#include "general/manager.h"
#include "general/object.h"
#include "general/manager_private.h"

struct cmzn_material;
struct cmzn_spectrumcomponent;
/*
Global types
------------
*/

struct cmzn_spectrum
/*******************************************************************************
LAST MODIFIED : 14 May 1998

DESCRIPTION :
Spectrum type is private.
==============================================================================*/
{
	ZnReal maximum,minimum;
	char *name;
	bool overwrite_colour;
	struct LIST(cmzn_spectrumcomponent) *list_of_components;

	struct Texture *colour_lookup_texture;
	int cache, changed;
	/* after clearing in create, following to be modified only by manager */
	struct MANAGER(cmzn_spectrum) *manager;
	int manager_change_status;
	bool is_managed_flag;
	/* the number of structures that point to this spectrum.  The spectrum
		cannot be destroyed while this is greater than 0 */
	int access_count;
}; /* struct cmzn_spectrum */

DECLARE_LIST_TYPES(cmzn_spectrum);

DECLARE_MANAGER_TYPES(cmzn_spectrum);

PROTOTYPE_MANAGER_GET_OWNER_FUNCTION(cmzn_spectrum, struct cmzn_spectrummodule);

enum Spectrum_colour_components
/*******************************************************************************
LAST MODIFIED : 4 October 2006

DESCRIPTION :
Used to identify the colour components modified by a spectrum.
==============================================================================*/
{
	SPECTRUM_COMPONENT_NONE = 0,
	SPECTRUM_COMPONENT_RED = 1,
	SPECTRUM_COMPONENT_GREEN = 2,
	SPECTRUM_COMPONENT_BLUE = 4,
	SPECTRUM_COMPONENT_MONOCHROME = 8,
	SPECTRUM_COMPONENT_ALPHA = 16
};

enum Spectrum_simple_type
{
	UNKNOWN_SPECTRUM,
	RED_TO_BLUE_SPECTRUM,
	BLUE_TO_RED_SPECTRUM,
	LOG_RED_TO_BLUE_SPECTRUM,
	LOG_BLUE_TO_RED_SPECTRUM,
	BLUE_WHITE_RED_SPECTRUM
};

/*
Global functions
----------------
*/
PROTOTYPE_OBJECT_FUNCTIONS(cmzn_spectrum);

PROTOTYPE_COPY_OBJECT_FUNCTION(cmzn_spectrum);

PROTOTYPE_LIST_FUNCTIONS(cmzn_spectrum);

PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(cmzn_spectrum,name,const char *);

PROTOTYPE_MANAGER_COPY_FUNCTIONS(cmzn_spectrum,name,const char *);

PROTOTYPE_MANAGER_FUNCTIONS(cmzn_spectrum);

PROTOTYPE_MANAGER_IDENTIFIER_FUNCTIONS(cmzn_spectrum,name,const char *);

struct cmzn_spectrumcomponent *cmzn_spectrum_get_component_at_position(
	 struct cmzn_spectrum *spectrum,int position);
/*******************************************************************************
LAST MODIFIED : 30 August 2007

DESCRIPTION :
Wrapper for accessing the component in <spectrum>.
==============================================================================*/

int Spectrum_set_simple_type(struct cmzn_spectrum *spectrum,
	enum Spectrum_simple_type type);
/*******************************************************************************
LAST MODIFIED : 15 January 2001

DESCRIPTION :
A convienience routine that allows a spectrum to be automatically set into
some predetermined simple types.
==============================================================================*/

enum Spectrum_simple_type Spectrum_get_simple_type(struct cmzn_spectrum *spectrum);
/*******************************************************************************
LAST MODIFIED : 28 July 1998

DESCRIPTION :
A convienience routine that interrogates a spectrum to see if it is one of the
simple types.  If it does not comform exactly to one of the simple types then
it returns UNKNOWN_SPECTRUM
==============================================================================*/

int Spectrum_add_component(struct cmzn_spectrum *spectrum,
	struct cmzn_spectrumcomponent *component,int position);
/*******************************************************************************
LAST MODIFIED : 24 July 1998

DESCRIPTION :
Adds the <component> to <spectrum> at the given <position>, where 1 is
the top of the list (rendered first), and values less than 1 or greater than the
last position in the list cause the component to be added at its end, with a
position one greater than the last.
==============================================================================*/

int cmzn_spectrum_get_component_position(struct cmzn_spectrum *spectrum,
	struct cmzn_spectrumcomponent *component);
/*******************************************************************************
LAST MODIFIED : 24 July 1998

DESCRIPTION :
Returns the position of <component> in <spectrum>.
==============================================================================*/

int set_Spectrum_minimum(struct cmzn_spectrum *spectrum,ZnReal minimum);
/*******************************************************************************
LAST MODIFIED : 29 July 1998

DESCRIPTION :
A function to set the spectrum minimum.
==============================================================================*/

int set_Spectrum_maximum(struct cmzn_spectrum *spectrum,ZnReal maximum);
/*******************************************************************************
LAST MODIFIED : 29 July 1998

DESCRIPTION :
A function to set the spectrum maximum.
==============================================================================*/

int Spectrum_get_number_of_data_components(struct cmzn_spectrum *spectrum);
/*******************************************************************************
LAST MODIFIED : 4 October 2006

DESCRIPTION :
Returns the number_of_components used by the spectrum.
==============================================================================*/

enum Spectrum_colour_components
Spectrum_get_colour_components(struct cmzn_spectrum *spectrum);
/*******************************************************************************
LAST MODIFIED : 4 October 2006

DESCRIPTION :
Returns a bit mask for the colour components modified by the spectrum.
==============================================================================*/

int Spectrum_calculate_range(struct cmzn_spectrum *spectrum);
/*******************************************************************************
LAST MODIFIED : 22 July 1998

DESCRIPTION :
Calculates the range of the spectrum from the component it contains and updates
the minimum and maximum contained inside it.
==============================================================================*/

int Spectrum_set_minimum_and_maximum(struct cmzn_spectrum *spectrum,
	ZnReal minimum, ZnReal maximum);
/*******************************************************************************
LAST MODIFIED : 29 July 1998

DESCRIPTION :
Expands the range of this spectrum by adjusting the range of each component
it contains.  The ratios of the different component are preserved.
==============================================================================*/

char *Spectrum_get_name(struct cmzn_spectrum *spectrum);
/*******************************************************************************
LAST MODIFIED : 28 August 2007

DESCRIPTION :
Returns the string of the spectrum.
==============================================================================*/

int Spectrum_render_value_on_material(struct cmzn_spectrum *spectrum,
	struct cmzn_material *material, int number_of_data_components,
	GLfloat *data);
/*******************************************************************************
LAST MODIFIED : 4 October 2006

DESCRIPTION :
Uses the <spectrum> to modify the <material> to represent the <number_of_data_components>
<data> values given.
==============================================================================*/

int Spectrum_value_to_rgba(struct cmzn_spectrum *spectrum,int number_of_data_components,
	FE_value *data, ZnReal *rgba);
/*******************************************************************************
LAST MODIFIED : 4 October 2006

DESCRIPTION :
Uses the <spectrum> to calculate RGBA components to represent the
<number_of_data_components> <data> values.
<rgba> is assumed to be an array of four values for red, green, blue and alpha.
==============================================================================*/

int Spectrum_end_value_to_rgba(struct cmzn_spectrum *spectrum);
/*******************************************************************************
LAST MODIFIED : 13 September 2007

DESCRIPTION :
Resets the caches and graphics state after rendering values.
==============================================================================*/

struct LIST(cmzn_spectrumcomponent) *get_cmzn_spectrumcomponent_list(
	struct cmzn_spectrum *spectrum );
/*******************************************************************************
LAST MODIFIED : 12 March 1998

DESCRIPTION :
Returns the component list that describes the spectrum.  This
is the pointer to the object inside the spectrum so do not
destroy it, any changes to it change that spectrum
==============================================================================*/

cmzn_spectrum *cmzn_spectrum_create_private();
/*******************************************************************************
LAST MODIFIED : 13 August 1997

DESCRIPTION :
Allocates memory and assigns fields for a Spectrum object.
==============================================================================*/

struct MANAGER(cmzn_spectrum) *cmzn_spectrummodule_get_manager(
	cmzn_spectrummodule_id spectrummodule);

struct cmzn_spectrummodule *cmzn_spectrummodule_create();

PROTOTYPE_GET_OBJECT_NAME_FUNCTION(cmzn_spectrum);

int Spectrum_get_colour_lookup_sizes(struct cmzn_spectrum *spectrum,
	int *lookup_dimension, int **lookup_sizes);
/*******************************************************************************
LAST MODIFIED : 2 May 2007

DESCRIPTION :
Returns the sizes used for the colour lookup spectrums internal texture.
==============================================================================*/

int Spectrum_manager_set_owner(struct MANAGER(cmzn_spectrum) *manager,
	struct cmzn_graphics_module *graphics_module);

int cmzn_spectrum_changed(cmzn_spectrum_id spectrum);

/**
 * Set spectrum maximum and minimum.
 *
 * @deprecated
 * @param spectrum  Handle to a cmiss_spectrum object.
 * @param minimum  Minimum value of the spectrum.
 * @param maximum  Maximum value of the spectrum.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
int cmzn_spectrum_set_minimum_and_maximum(cmzn_spectrum_id spectrum, double minimum, double maximum);

/**
 * Get the minimum value from the given spectrum.
 *
 * @param spectrum  Handle to a cmiss_spectrum object.
 * @return  the minimum value, 0.0 on failure.
 */
double cmzn_spectrum_get_minimum(cmzn_spectrum_id spectrum);

/**
 * Get the maximum value from the given spectrum.
 *
 * @param spectrum  Handle to a cmiss_spectrum object.
 * @return  the maximum value, 0.0 on failure.
 */
double cmzn_spectrum_get_maximum(cmzn_spectrum_id spectrum);

#endif /* !defined(SPECTRUM_H) */
