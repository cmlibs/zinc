/*******************************************************************************
FILE : spectrum.h

LAST MODIFIED : 15 October 1998

DESCRIPTION :
Spectrum structures and support code.
==============================================================================*/
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
#if !defined(SPECTRUM_H)
#define SPECTRUM_H

#include "zinc/zincconfigure.h"
#include "zinc/graphicsmodule.h"
#include "zinc/spectrum.h"

#if defined (USE_GLEW)
#include <GL/glew.h>
#endif /* USE_GLEW */
#include "general/value.h"
#include "general/list.h"
#include "general/manager.h"
#include "general/object.h"
#include "general/manager_private.h"

struct Cmiss_graphics_material;
struct Cmiss_spectrum_component;
/*
Global types
------------
*/

#define Spectrum Cmiss_spectrum
struct Spectrum
/*******************************************************************************
LAST MODIFIED : 14 May 1998

DESCRIPTION :
Spectrum type is private.
==============================================================================*/
{
	ZnReal maximum,minimum;
	char *name;
	bool overwrite_colour;
	struct LIST(Cmiss_spectrum_component) *list_of_components;

	struct Texture *colour_lookup_texture;
	int cache, changed;
	/* after clearing in create, following to be modified only by manager */
	struct MANAGER(Spectrum) *manager;
	int manager_change_status;
	bool is_managed_flag;
	/* the number of structures that point to this spectrum.  The spectrum
		cannot be destroyed while this is greater than 0 */
	int access_count;
}; /* struct Spectrum */

DECLARE_LIST_TYPES(Spectrum);

DECLARE_MANAGER_TYPES(Spectrum);

PROTOTYPE_MANAGER_GET_OWNER_FUNCTION(Spectrum, struct Cmiss_spectrum_module);

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
PROTOTYPE_OBJECT_FUNCTIONS(Spectrum);

PROTOTYPE_COPY_OBJECT_FUNCTION(Spectrum);

PROTOTYPE_LIST_FUNCTIONS(Spectrum);

PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Spectrum,name,const char *);

PROTOTYPE_MANAGER_COPY_FUNCTIONS(Spectrum,name,const char *);

PROTOTYPE_MANAGER_FUNCTIONS(Spectrum);

PROTOTYPE_MANAGER_IDENTIFIER_FUNCTIONS(Spectrum,name,const char *);

struct Cmiss_spectrum_component *Cmiss_spectrum_get_component_at_position(
	 struct Spectrum *spectrum,int position);
/*******************************************************************************
LAST MODIFIED : 30 August 2007

DESCRIPTION :
Wrapper for accessing the component in <spectrum>.
==============================================================================*/

int Spectrum_set_simple_type(struct Spectrum *spectrum,
	enum Spectrum_simple_type type);
/*******************************************************************************
LAST MODIFIED : 15 January 2001

DESCRIPTION :
A convienience routine that allows a spectrum to be automatically set into
some predetermined simple types.
==============================================================================*/

enum Spectrum_simple_type Spectrum_get_simple_type(struct Spectrum *spectrum);
/*******************************************************************************
LAST MODIFIED : 28 July 1998

DESCRIPTION :
A convienience routine that interrogates a spectrum to see if it is one of the
simple types.  If it does not comform exactly to one of the simple types then
it returns UNKNOWN_SPECTRUM
==============================================================================*/

int Spectrum_add_component(struct Spectrum *spectrum,
	struct Cmiss_spectrum_component *component,int position);
/*******************************************************************************
LAST MODIFIED : 24 July 1998

DESCRIPTION :
Adds the <component> to <spectrum> at the given <position>, where 1 is
the top of the list (rendered first), and values less than 1 or greater than the
last position in the list cause the component to be added at its end, with a
position one greater than the last.
==============================================================================*/

int Cmiss_spectrum_get_component_position(struct Spectrum *spectrum,
	struct Cmiss_spectrum_component *component);
/*******************************************************************************
LAST MODIFIED : 24 July 1998

DESCRIPTION :
Returns the position of <component> in <spectrum>.
==============================================================================*/

int set_Spectrum_minimum(struct Spectrum *spectrum,ZnReal minimum);
/*******************************************************************************
LAST MODIFIED : 29 July 1998

DESCRIPTION :
A function to set the spectrum minimum.
==============================================================================*/

int set_Spectrum_maximum(struct Spectrum *spectrum,ZnReal maximum);
/*******************************************************************************
LAST MODIFIED : 29 July 1998

DESCRIPTION :
A function to set the spectrum maximum.
==============================================================================*/

int Spectrum_get_number_of_data_components(struct Spectrum *spectrum);
/*******************************************************************************
LAST MODIFIED : 4 October 2006

DESCRIPTION :
Returns the number_of_components used by the spectrum.
==============================================================================*/

enum Spectrum_colour_components
Spectrum_get_colour_components(struct Spectrum *spectrum);
/*******************************************************************************
LAST MODIFIED : 4 October 2006

DESCRIPTION :
Returns a bit mask for the colour components modified by the spectrum.
==============================================================================*/

int Spectrum_calculate_range(struct Spectrum *spectrum);
/*******************************************************************************
LAST MODIFIED : 22 July 1998

DESCRIPTION :
Calculates the range of the spectrum from the component it contains and updates
the minimum and maximum contained inside it.
==============================================================================*/

int Spectrum_set_minimum_and_maximum(struct Spectrum *spectrum,
	ZnReal minimum, ZnReal maximum);
/*******************************************************************************
LAST MODIFIED : 29 July 1998

DESCRIPTION :
Expands the range of this spectrum by adjusting the range of each component
it contains.  The ratios of the different component are preserved.
==============================================================================*/

char *Spectrum_get_name(struct Spectrum *spectrum);
/*******************************************************************************
LAST MODIFIED : 28 August 2007

DESCRIPTION :
Returns the string of the spectrum.
==============================================================================*/

int Spectrum_render_value_on_material(struct Spectrum *spectrum,
	struct Cmiss_graphics_material *material, int number_of_data_components,
	GLfloat *data);
/*******************************************************************************
LAST MODIFIED : 4 October 2006

DESCRIPTION :
Uses the <spectrum> to modify the <material> to represent the <number_of_data_components>
<data> values given.
==============================================================================*/

int Spectrum_value_to_rgba(struct Spectrum *spectrum,int number_of_data_components,
	FE_value *data, ZnReal *rgba);
/*******************************************************************************
LAST MODIFIED : 4 October 2006

DESCRIPTION :
Uses the <spectrum> to calculate RGBA components to represent the
<number_of_data_components> <data> values.
<rgba> is assumed to be an array of four values for red, green, blue and alpha.
==============================================================================*/

int Spectrum_end_value_to_rgba(struct Spectrum *spectrum);
/*******************************************************************************
LAST MODIFIED : 13 September 2007

DESCRIPTION :
Resets the caches and graphics state after rendering values.
==============================================================================*/

struct LIST(Cmiss_spectrum_component) *get_Cmiss_spectrum_component_list(
	struct Spectrum *spectrum );
/*******************************************************************************
LAST MODIFIED : 12 March 1998

DESCRIPTION :
Returns the component list that describes the spectrum.  This
is the pointer to the object inside the spectrum so do not
destroy it, any changes to it change that spectrum
==============================================================================*/

Cmiss_spectrum *Cmiss_spectrum_create_private();
/*******************************************************************************
LAST MODIFIED : 13 August 1997

DESCRIPTION :
Allocates memory and assigns fields for a Spectrum object.
==============================================================================*/

struct MANAGER(Cmiss_spectrum) *Cmiss_spectrum_module_get_manager(
	Cmiss_spectrum_module_id spectrum_module);

struct Cmiss_spectrum_module *Cmiss_spectrum_module_create();

PROTOTYPE_GET_OBJECT_NAME_FUNCTION(Spectrum);

int Spectrum_get_colour_lookup_sizes(struct Spectrum *spectrum,
	int *lookup_dimension, int **lookup_sizes);
/*******************************************************************************
LAST MODIFIED : 2 May 2007

DESCRIPTION :
Returns the sizes used for the colour lookup spectrums internal texture.
==============================================================================*/

int Spectrum_manager_set_owner(struct MANAGER(Spectrum) *manager,
	struct Cmiss_graphics_module *graphics_module);

int Cmiss_spectrum_changed(Cmiss_spectrum_id spectrum);

/**
 * Set spectrum maximum and minimum.
 *
 * @deprecated
 * @param spectrum  Handle to a cmiss_spectrum object.
 * @param minimum  Minimum value of the spectrum.
 * @param maximum  Maximum value of the spectrum.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
int Cmiss_spectrum_set_minimum_and_maximum(Cmiss_spectrum_id spectrum, double minimum, double maximum);

/**
 * Get the minimum value from the given spectrum.
 *
 * @param spectrum  Handle to a cmiss_spectrum object.
 * @return  the minimum value, 0.0 on failure.
 */
double Cmiss_spectrum_get_minimum(Cmiss_spectrum_id spectrum);

/**
 * Get the maximum value from the given spectrum.
 *
 * @param spectrum  Handle to a cmiss_spectrum object.
 * @return  the maximum value, 0.0 on failure.
 */
double Cmiss_spectrum_get_maximum(Cmiss_spectrum_id spectrum);

#endif /* !defined(SPECTRUM_H) */
