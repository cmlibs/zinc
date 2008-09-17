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
#include "general/list.h"
#include "general/manager.h"
#include "general/object.h"
#include "command/parser.h"

struct Graphical_material;
struct Spectrum_settings;
/*
Global types
------------
*/


struct Spectrum;
/*******************************************************************************
LAST MODIFIED : 18 October 1997

DESCRIPTION :
Spectrum type is private.
==============================================================================*/

DECLARE_LIST_TYPES(Spectrum);

DECLARE_MANAGER_TYPES(Spectrum);

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

PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Spectrum,name,char *);

PROTOTYPE_MANAGER_COPY_FUNCTIONS(Spectrum,name,char *);

PROTOTYPE_MANAGER_FUNCTIONS(Spectrum);

PROTOTYPE_MANAGER_IDENTIFIER_FUNCTIONS(Spectrum,name,char *);

struct Spectrum_settings *get_settings_at_position_in_Spectrum(
	 struct Spectrum *spectrum,int position);
/*******************************************************************************
LAST MODIFIED : 30 August 2007

DESCRIPTION :
Wrapper for accessing the settings in <spectrum>.
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

enum Spectrum_simple_type Spectrum_get_contoured_simple_type(struct Spectrum *spectrum);
/*******************************************************************************
LAST MODIFIED : 23 May 2000

DESCRIPTION :
A convienience routine that interrogates a spectrum to see if it is one of the
simple types, or a simple type with a contour(colour_mapping==SPECTRUM_BANDED) 
added as an extra, last, setting If it does not comform exactly to one of the 
simple types (or a simple type with a contour) then it returns UNKNOWN_SPECTRUM. 
See also Spectrum_get_simple_type.
==============================================================================*/

int Spectrum_overlay_contours(struct MANAGER(Spectrum) *spectrum_manager,
	struct Spectrum *spectrum,int number_of_bands,int band_proportions);
/*******************************************************************************
LAST MODIFIED : 22 May 2000

DESCRIPTION :
Checks if the last spectrum setting is SPECTRUM_BANDED, removes it if it is,
then adds a SPECTRUM_BANDED setting to the <spectrum> with <number_of_bands>,
<band_proportions>. Setting is added at the end of the list.
This function assumes the <spectum> is a simple with an added SPECTRUM_BANDED 
settings holding for the contours.
If <number_of_bands>==0, simply removes any existing contour band settings.
==============================================================================*/

int Spectrum_add_settings(struct Spectrum *spectrum,
	struct Spectrum_settings *settings,int position);
/*******************************************************************************
LAST MODIFIED : 24 July 1998

DESCRIPTION :
Adds the <settings> to <spectrum> at the given <position>, where 1 is
the top of the list (rendered first), and values less than 1 or greater than the
last position in the list cause the settings to be added at its end, with a
position one greater than the last.
==============================================================================*/

int Spectrum_remove_settings(struct Spectrum *spectrum,
	struct Spectrum_settings *settings);
/*******************************************************************************
LAST MODIFIED : 24 July 1998

DESCRIPTION :
Removes the <settings> from <spectrum> and decrements the position
of all subsequent settings.
==============================================================================*/

int Spectrum_remove_all_settings(struct Spectrum *spectrum);
/*******************************************************************************
LAST MODIFIED : 4 August 1998

DESCRIPTION :
Removes the all the settings from <spectrum>.
==============================================================================*/

int Spectrum_get_settings_position(struct Spectrum *spectrum,
	struct Spectrum_settings *settings);
/*******************************************************************************
LAST MODIFIED : 24 July 1998

DESCRIPTION :
Returns the position of <settings> in <spectrum>.
==============================================================================*/

int set_Spectrum(struct Parse_state *state,void *spectrum_address_void,
	void *spectrum_manager_void);
/*******************************************************************************
LAST MODIFIED : 13 August 1997

DESCRIPTION :
A modifier function to set the spectrum by finding in the spectrum manager
the name given in the next token of the parser
==============================================================================*/

int set_Spectrum_minimum(struct Spectrum *spectrum,float minimum);
/*******************************************************************************
LAST MODIFIED : 29 July 1998

DESCRIPTION :
A function to set the spectrum minimum.
==============================================================================*/

int set_Spectrum_maximum(struct Spectrum *spectrum,float maximum);
/*******************************************************************************
LAST MODIFIED : 29 July 1998

DESCRIPTION :
A function to set the spectrum maximum.
==============================================================================*/

int set_Spectrum_minimum_command(struct Parse_state *state,void *spectrum_ptr_void,
	void *dummy_user_data);
/*******************************************************************************
LAST MODIFIED : 29 July 1998

DESCRIPTION :
A modifier function to set the spectrum minimum.
==============================================================================*/

int set_Spectrum_maximum_command(struct Parse_state *state,void *spectrum_ptr_void,
	void *dummy_user_data);
/*******************************************************************************
LAST MODIFIED : 29 July 1998

DESCRIPTION :
A modifier function to set the spectrum maximum.
==============================================================================*/

float get_Spectrum_minimum(struct Spectrum *spectrum);
/*******************************************************************************
LAST MODIFIED : 13 August 1997

DESCRIPTION :
Returns the value of the spectrum minimum.
==============================================================================*/

float get_Spectrum_maximum(struct Spectrum *spectrum);
/*******************************************************************************
LAST MODIFIED : 13 August 1997

DESCRIPTION :
Returns the value of the spectrum maximum.
==============================================================================*/

int Spectrum_get_number_of_components(struct Spectrum *spectrum);
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
Calculates the range of the spectrum from the settings it contains and updates
the minimum and maximum contained inside it.
==============================================================================*/

int Spectrum_clear_all_fixed_flags(struct Spectrum *spectrum);
/*******************************************************************************
LAST MODIFIED : 16 January 2001

DESCRIPTION :
clears the fix_maximum, fix_minimum flag for all the settings in <spectrum>
==============================================================================*/

int Spectrum_set_minimum_and_maximum(struct Spectrum *spectrum,
	float minimum, float maximum);
/*******************************************************************************
LAST MODIFIED : 29 July 1998

DESCRIPTION :
Expands the range of this spectrum by adjusting the range of each settings
it contains.  The ratios of the different settings are preserved.
==============================================================================*/

char *Spectrum_get_name(struct Spectrum *spectrum);
/*******************************************************************************
LAST MODIFIED : 28 August 2007

DESCRIPTION :
Returns the string of the spectrum.
==============================================================================*/

int Spectrum_get_opaque_colour_flag(struct Spectrum *spectrum);
/*******************************************************************************
LAST MODIFIED : 30 July 1998

DESCRIPTION :
Returns the value of the spectrum opaque flag which indicates whether the
spectrum clears the material colour before applying the settings or not.
==============================================================================*/

int Spectrum_set_opaque_colour_flag(struct Spectrum *spectrum, int opaque);
/*******************************************************************************
LAST MODIFIED : 30 July 1998

DESCRIPTION :
Sets the value of the spectrum opaque flag which indicates whether the
spectrum clears the material colour before applying the settings or not.
==============================================================================*/

#if defined (OPENGL_API)
struct Spectrum_render_data *spectrum_start_renderGL(
	struct Spectrum *spectrum,struct Graphical_material *material,
	int number_of_data_components);
/*******************************************************************************
LAST MODIFIED : 3 June 1999

DESCRIPTION :
Initialises the graphics state for rendering values on the current material.
==============================================================================*/

int spectrum_renderGL_value(struct Spectrum *spectrum,
	struct Graphical_material *material,struct Spectrum_render_data *render_data,
	float *data);
/*******************************************************************************
LAST MODIFIED : 1 June 1999

DESCRIPTION :
Sets the graphics rendering state to represent the value 'data' in
accordance with the spectrum.
==============================================================================*/

int spectrum_end_renderGL(struct Spectrum *spectrum,
	struct Spectrum_render_data *render_data);
/*******************************************************************************
LAST MODIFIED : 13 March 1998

DESCRIPTION :
Resets the graphics state after rendering values on current material.
==============================================================================*/
#endif /* defined (OPENGL_API) */

int Spectrum_render_value_on_material(struct Spectrum *spectrum,
	struct Graphical_material *material, int number_of_data_components,
	float *data);
/*******************************************************************************
LAST MODIFIED : 4 October 2006

DESCRIPTION :
Uses the <spectrum> to modify the <material> to represent the <number_of_data_components>
<data> values given.
==============================================================================*/

int Spectrum_value_to_rgba(struct Spectrum *spectrum,int number_of_data_components,
	float *data, float *rgba);
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

struct LIST(Spectrum_settings) *get_Spectrum_settings_list(
	struct Spectrum *spectrum );
/*******************************************************************************
LAST MODIFIED : 12 March 1998

DESCRIPTION :
Returns the settings list that describes the spectrum.  This
is the pointer to the object inside the spectrum so do not
destroy it, any changes to it change that spectrum
==============================================================================*/

int gfx_destroy_spectrum(struct Parse_state *state,
	void *dummy_to_be_modified,void *Spectrum_manager_void);
/*******************************************************************************
LAST MODIFIED : 28 October 1997

DESCRIPTION :
Executes a GFX DESTROY SPECTRUM command.
==============================================================================*/

int for_each_spectrum_list_or_write_commands(struct Spectrum *spectrum,void *write_enabled_void);
/*******************************************************************************
LAST MODIFIED : 18 August 2007

DESCRIPTION :
For each spectrum in manager, list the spectrum commands to the command windows or write them out.
==============================================================================*/

int Spectrum_list_commands(struct Spectrum *spectrum,
	char *command_prefix,char *command_suffix);
/*******************************************************************************
LAST MODIFIED : 5 August 1998

DESCRIPTION :
Writes the properties of the <spectrum> to the command window in a
form that can be directly pasted into a com file.
==============================================================================*/

int Spectrum_list_contents(struct Spectrum *spectrum,void *dummy);
/*******************************************************************************
LAST MODIFIED : 5 August 1998

DESCRIPTION :
Writes the properties of the <spectrum> to the command window.
==============================================================================*/

struct Spectrum *CREATE(Spectrum)(const char *name);
/*******************************************************************************
LAST MODIFIED : 13 August 1997

DESCRIPTION :
Allocates memory and assigns fields for a Spectrum object.
==============================================================================*/

int DESTROY(Spectrum)(struct Spectrum **spectrum_ptr);
/*******************************************************************************
LAST MODIFIED : 11 August 1997

DESCRIPTION :
Frees the memory for the fields of <**spectrum>, frees the memory for
<**spectrum> and sets <*spectrum> to NULL.
==============================================================================*/

PROTOTYPE_GET_OBJECT_NAME_FUNCTION(Spectrum);

struct Graphics_buffer;

int Spectrum_compile_colour_lookup(struct Spectrum *spectrum,
	struct Graphics_buffer *graphics_buffer);
/*******************************************************************************
LAST MODIFIED : 18 August 2007

DESCRIPTION :
Rebuilds the display_list for <spectrum> if it is not current.
==============================================================================*/

int Spectrum_execute_colour_lookup(struct Spectrum *spectrum);
/*******************************************************************************
LAST MODIFIED : 10 May 2005

DESCRIPTION :
Activates <spectrum> by calling its display list. If the display list is not
current, an error is reported.
If a NULL <spectrum> is supplied, spectrums are disabled.
==============================================================================*/

int Spectrum_get_colour_lookup_sizes(struct Spectrum *spectrum,
	int *lookup_dimension, int **lookup_sizes);
/*******************************************************************************
LAST MODIFIED : 2 May 2007

DESCRIPTION :
Returns the sizes used for the colour lookup spectrums internal texture.
==============================================================================*/


#endif /* !defined(SPECTRUM_H) */
