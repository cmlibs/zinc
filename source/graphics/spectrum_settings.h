/*******************************************************************************
FILE : spectrum_settings.h

LAST MODIFIED : 14 July 1998

DESCRIPTION :
Spectrum_settings structure and routines for describing and manipulating the
appearance of spectrums.
==============================================================================*/
#if !defined (SPECTRUM_SETTINGS_H)
#define SPECTRUM_SETTINGS_H

/*
Global types
------------
*/
enum Spectrum_settings_type
/*******************************************************************************
LAST MODIFIED : 22 September 1998

DESCRIPTION :
==============================================================================*/
{
	SPECTRUM_INVALID_TYPE = -1,
	SPECTRUM_LINEAR,
	SPECTRUM_LOG
}; /* enum Spectrum_settings_type */

enum Spectrum_settings_colour_mapping
/*******************************************************************************
LAST MODIFIED : 22 September 1998

DESCRIPTION :
Map the value into a colour.
==============================================================================*/
{
	SPECTRUM_RAINBOW,
	SPECTRUM_RED,
	SPECTRUM_GREEN,
	SPECTRUM_BLUE,
	SPECTRUM_ALPHA,
	SPECTRUM_BANDED,
	SPECTRUM_STEP,
	SPECTRUM_BLUE_TO_WHITE,
	SPECTRUM_WHITE_TO_RED
}; /* enum Spectrum_settings_colour_mapping */

enum Spectrum_settings_render_type
/*******************************************************************************
LAST MODIFIED : 13 July 1998

DESCRIPTION :
Which material component is used for rendering.  Need to make them binary
separated so that we can compile a render_type of the whole spectrum to see if
it can be accelerated
==============================================================================*/
{
	SPECTRUM_AMBIENT_AND_DIFFUSE = 2,
	SPECTRUM_AMBIENT = 4,
	SPECTRUM_DIFFUSE = 8,
	SPECTRUM_EMISSION = 16,
	SPECTRUM_SPECULAR = 32
}; /* enum Spectrum_settings_render_type */

enum Spectrum_settings_string_details
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Parameter for selecting detail included by Spectrum_settings_string:
SETTINGS_STRING_SPACE_ONLY = only those settings that control the space;
SETTINGS_STRING_COMPLETE = all settings including appearance.
SETTINGS_STRING_COMPLETE_PLUS = as above, but with * added if settings_changed.
==============================================================================*/
{
	SPECTRUM_SETTINGS_STRING_SPACE_ONLY,
	SPECTRUM_SETTINGS_STRING_COMPLETE,
	SPECTRUM_SETTINGS_STRING_COMPLETE_PLUS
}; /* enum Spectrum_settings_string_details */

struct Spectrum_settings;
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Private
==============================================================================*/

enum Spectrum_settings_modify_command
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
==============================================================================*/
{
	SPECTRUM_SETTINGS_ADD_MODIFY,
	SPECTRUM_SETTINGS_ADD_MODIFY_TOP,
	SPECTRUM_SETTINGS_DELETE
};

struct Modify_spectrum_data
/*******************************************************************************
LAST MODIFIED : 4 August 1998

DESCRIPTION :
Structure modified by spectrum modify routine.
==============================================================================*/
{
	int position;
	float spectrum_minimum, spectrum_maximum;
	struct Spectrum_settings *settings;
}; /* struct Modify_spectrum_data */

struct Spectrum_command_data
/*******************************************************************************
LAST MODIFIED : 3 August 1998

DESCRIPTION :
Subset of command data passed to spectrum modify routines.
==============================================================================*/
{
	struct MANAGER(Spectrum) *spectrum_manager;
}; /* struct Spectrum_command_data */

struct Spectrum_settings_list_data
/*******************************************************************************
LAST MODIFIED : 5 August 1998

DESCRIPTION :
Data for formating output with Spectrum_list_contents function.
==============================================================================*/
{
	char *line_prefix,*line_suffix;
	enum Spectrum_settings_string_details settings_string_detail;
}; /* Spectrum_settings_list_data */

/*
Global functions
----------------
*/
DECLARE_LIST_TYPES(Spectrum_settings);

PROTOTYPE_OBJECT_FUNCTIONS(Spectrum_settings);
PROTOTYPE_COPY_OBJECT_FUNCTION(Spectrum_settings);
PROTOTYPE_LIST_FUNCTIONS(Spectrum_settings);
PROTOTYPE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Spectrum_settings, \
	position,int);

struct Spectrum_settings *CREATE(Spectrum_settings)(void);
/*******************************************************************************
LAST MODIFIED : 14 July 1998

DESCRIPTION :
Allocates memory and assigns fields for a struct Spectrum_settings.
==============================================================================*/

int DESTROY(Spectrum_settings)(struct Spectrum_settings **settings_ptr);
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Frees the memory for the fields of <**settings_ptr>, frees the memory for
<**settings_ptr> and sets <*settings_ptr> to NULL.
==============================================================================*/

int Spectrum_settings_copy_and_put_in_list(
	struct Spectrum_settings *settings,void *list_of_settings_void);
/*******************************************************************************
LAST MODIFIED : 27 July 1998

DESCRIPTION :
Spectrum_settings iterator function for copying a list_of_settings.
Makes a copy of the settings and puts it in the list_of_settings.
==============================================================================*/

int Spectrum_settings_type_matches(
	struct Spectrum_settings *settings,void *settings_type_void);
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Returns 1 if the settings are of the specified settings_type.
==============================================================================*/

int Spectrum_settings_add(struct Spectrum_settings *settings,
	int position,
	struct LIST(Spectrum_settings) *list_of_settings);
/*******************************************************************************
LAST MODIFIED : 24 July 1998

DESCRIPTION :
Adds the new_settings in the list_of_settings at the given <priority>.
==============================================================================*/

int Spectrum_settings_remove(struct Spectrum_settings *settings,
	struct LIST(Spectrum_settings) *list_of_settings);
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Removes settings from list_of_settings.
==============================================================================*/

int Spectrum_settings_modify(struct Spectrum_settings *settings,
	struct Spectrum_settings *new_settings,
	struct LIST(Spectrum_settings) *list_of_settings);
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Changes the contents of settings to match new_settings, with no change in
priority.
==============================================================================*/

int Spectrum_settings_get_position(
	struct Spectrum_settings *settings,
	struct LIST(Spectrum_settings) *list_of_settings);
/*******************************************************************************
LAST MODIFIED : 24 June 1998

DESCRIPTION :
Returns the position of <settings> in <list_of_settings>.
==============================================================================*/

int Spectrum_settings_all_changed(struct LIST(Spectrum_settings) *);
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Sets the settings->changed flag of all settings in the list.
For use after (eg.) discretization change.
==============================================================================*/

int Spectrum_settings_type_changed(
	enum Spectrum_settings_type settings_type,
	struct LIST(Spectrum_settings) *list_of_settings);
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Sets the settings->changed flag of all settings in list.
For use after (eg.) change of discretization.
==============================================================================*/

int Spectrum_settings_same_space(struct Spectrum_settings *settings,
	void *second_settings_void);
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
LIST(Spectrum_settings) conditional function returning 1 iff the two
Spectrum_settings describe the same space.
==============================================================================*/

char *Spectrum_settings_string(struct Spectrum_settings *settings,
	enum Spectrum_settings_string_details settings_detail);
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Returns a string describing the settings, suitable for entry into the command
line. Parameter <settings_detail> selects whether appearance settings are
included in the string. User must remember to DEALLOCATE the name afterwards.
==============================================================================*/

int Spectrum_settings_show(struct Spectrum_settings *settings,
	void *settings_detail_void);
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Writes out the settings as a text string.
==============================================================================*/

enum Spectrum_settings_type Spectrum_settings_get_type
	(struct Spectrum_settings *settings);
/*******************************************************************************
LAST MODIFIED : 12 March 1998

DESCRIPTION :
Returns the type of the Spectrum_settings <settings>.
==============================================================================*/

int Spectrum_settings_set_type
	(struct Spectrum_settings *settings,
	enum Spectrum_settings_type type);
/*******************************************************************************
LAST MODIFIED : 13 March 1998

DESCRIPTION :
Sets the type of the Spectrum_settings <settings>.
==============================================================================*/

int Spectrum_settings_set_active(struct Spectrum_settings *settings,
	int active);
/*******************************************************************************
LAST MODIFIED : 24 July 1998

DESCRIPTION :
Sets settings->settings_active to the <active> value;
==============================================================================*/

int Spectrum_settings_get_active(struct Spectrum_settings *settings);
/*******************************************************************************
LAST MODIFIED : 27 July 1998

DESCRIPTION :
Gets the settings->active value;
==============================================================================*/

int Spectrum_settings_get_component_number(struct Spectrum_settings *settings);
/*******************************************************************************
LAST MODIFIED : 3 June 1999

DESCRIPTION :
Returns the component_number of the Spectrum_settings <spectrum>.
==============================================================================*/

int Spectrum_settings_set_component_number(struct Spectrum_settings *settings,
	int component_number);
/*******************************************************************************
LAST MODIFIED : 3 June 1999

DESCRIPTION :
Sets the component_number of the Spectrum_settings <settings>.
==============================================================================*/

int Spectrum_settings_get_reverse_flag(struct Spectrum_settings *settings);
/*******************************************************************************
LAST MODIFIED : 14 July 1998

DESCRIPTION :
Returns the reverse flag of the Spectrum_settings <spectrum>.
==============================================================================*/

int Spectrum_settings_set_reverse_flag(struct Spectrum_settings *settings,
	int reverse);
/*******************************************************************************
LAST MODIFIED : 14 July 1998

DESCRIPTION :
Sets the reverse flag of the Spectrum_settings <settings>.
==============================================================================*/

enum Spectrum_settings_colour_mapping Spectrum_settings_get_colour_mapping(struct Spectrum_settings *settings);
/*******************************************************************************
LAST MODIFIED : 14 July 1998

DESCRIPTION :
Returns the colour mapping of the Spectrum_settings <spectrum>.
==============================================================================*/

int Spectrum_settings_set_colour_mapping(struct Spectrum_settings *settings,
	enum Spectrum_settings_colour_mapping type);
/*******************************************************************************
LAST MODIFIED : 14 July 1998

DESCRIPTION :
Sets the colour mapping of the Spectrum_settings <settings>.
==============================================================================*/

float Spectrum_settings_get_exaggeration(struct Spectrum_settings *settings);
/*******************************************************************************
LAST MODIFIED : 14 July 1998

DESCRIPTION :
Returns the first type parameter of the Spectrum_settings <spectrum>.
==============================================================================*/

int Spectrum_settings_set_exaggeration(struct Spectrum_settings *settings,
	float param1);
/*******************************************************************************
LAST MODIFIED : 14 July 1998

DESCRIPTION :
Sets the first type parameter of the Spectrum_settings <settings>.
==============================================================================*/

int Spectrum_settings_get_number_of_bands(struct Spectrum_settings *settings);
/*******************************************************************************
LAST MODIFIED : 31 July 1998

DESCRIPTION :
==============================================================================*/

int Spectrum_settings_set_number_of_bands(struct Spectrum_settings *settings,
	int bands);
/*******************************************************************************
LAST MODIFIED : 31 July 1998

DESCRIPTION :
==============================================================================*/

int Spectrum_settings_get_black_band_proportion(struct Spectrum_settings *settings);
/*******************************************************************************
LAST MODIFIED : 31 July 1998

DESCRIPTION :
==============================================================================*/

int Spectrum_settings_set_black_band_proportion(struct Spectrum_settings *settings,
	int proportion);
/*******************************************************************************
LAST MODIFIED : 31 July 1998

DESCRIPTION :
==============================================================================*/

float Spectrum_settings_get_step_value(struct Spectrum_settings *settings);
/*******************************************************************************
LAST MODIFIED : 31 July 1998

DESCRIPTION :
Returns the first type parameter of the Spectrum_settings <spectrum>.
==============================================================================*/

int Spectrum_settings_set_step_value(struct Spectrum_settings *settings,
	float param1);
/*******************************************************************************
LAST MODIFIED : 31 July 1998

DESCRIPTION :
Sets the first type parameter of the Spectrum_settings <settings>.
==============================================================================*/

float Spectrum_settings_get_range_minimum(struct Spectrum_settings *settings);
/*******************************************************************************
LAST MODIFIED : 14 July 1998

DESCRIPTION :
==============================================================================*/

int Spectrum_settings_set_range_minimum(struct Spectrum_settings *settings,
	float value);
/*******************************************************************************
LAST MODIFIED : 14 July 1998

DESCRIPTION :
==============================================================================*/

float Spectrum_settings_get_range_maximum(struct Spectrum_settings *settings);
/*******************************************************************************
LAST MODIFIED : 14 July 1998

DESCRIPTION :
==============================================================================*/

int Spectrum_settings_set_range_maximum(struct Spectrum_settings *settings,
	float value);
/*******************************************************************************
LAST MODIFIED : 14 July 1998

DESCRIPTION :
==============================================================================*/

int Spectrum_settings_get_extend_above_flag(struct Spectrum_settings *settings);
/*******************************************************************************
LAST MODIFIED : 5 August 1998

DESCRIPTION :
Returns the extend_above flag of the Spectrum_settings <spectrum>.
==============================================================================*/

int Spectrum_settings_set_extend_above_flag(struct Spectrum_settings *settings,
	int extend_above);
/*******************************************************************************
LAST MODIFIED : 5 August 1998

DESCRIPTION :
Sets the extend_above flag of the Spectrum_settings <settings>.
==============================================================================*/

int Spectrum_settings_get_extend_below_flag(struct Spectrum_settings *settings);
/*******************************************************************************
LAST MODIFIED : 5 August 1998

DESCRIPTION :
Returns the extend_below flag of the Spectrum_settings <spectrum>.
==============================================================================*/

int Spectrum_settings_set_extend_below_flag(struct Spectrum_settings *settings,
	int extend_below);
/*******************************************************************************
LAST MODIFIED : 5 August 1998

DESCRIPTION :
Sets the extend_below flag of the Spectrum_settings <settings>.
==============================================================================*/

int Spectrum_settings_get_fix_minimum_flag(struct Spectrum_settings *settings);
/*******************************************************************************
LAST MODIFIED : 11 January 2001

DESCRIPTION :
Returns the fix_minimum flag of the Spectrum_settings <spectrum>.
==============================================================================*/

int Spectrum_settings_set_fix_minimum_flag(struct Spectrum_settings *settings,
	int fix_minimum);
/*******************************************************************************
LAST MODIFIED : 11 January 2001

DESCRIPTION :
Sets the fix_minimum flag of the Spectrum_settings <settings>.
==============================================================================*/

int Spectrum_settings_get_fix_maximum_flag(struct Spectrum_settings *settings);
/*******************************************************************************
LAST MODIFIED : 11 January 2001

DESCRIPTION :
Returns the fix_maximum flag of the Spectrum_settings <spectrum>.
==============================================================================*/

int Spectrum_settings_set_fix_maximum_flag(struct Spectrum_settings *settings,
	int fix_maximum);
/*******************************************************************************
LAST MODIFIED : 11 January 2001

DESCRIPTION :
Sets the fix_maximum flag of the Spectrum_settings <settings>.
==============================================================================*/

float Spectrum_settings_get_colour_value_minimum(struct Spectrum_settings *settings);
/*******************************************************************************
LAST MODIFIED : 30 July 1998

DESCRIPTION :
==============================================================================*/

int Spectrum_settings_set_colour_value_minimum(struct Spectrum_settings *settings,
	float value);
/*******************************************************************************
LAST MODIFIED : 30 July 1998

DESCRIPTION :
==============================================================================*/

float Spectrum_settings_get_colour_value_maximum(struct Spectrum_settings *settings);
/*******************************************************************************
LAST MODIFIED : 30 July 1998

DESCRIPTION :
==============================================================================*/

int Spectrum_settings_set_colour_value_maximum(struct Spectrum_settings *settings,
	float value);
/*******************************************************************************
LAST MODIFIED : 30 July 1998

DESCRIPTION :
==============================================================================*/

enum Spectrum_settings_render_type Spectrum_settings_get_render_type
	(struct Spectrum_settings *settings);
/*******************************************************************************
LAST MODIFIED : 13 July 1998

DESCRIPTION :
Returns the render type of the Spectrum_settings <settings>.
==============================================================================*/

int Spectrum_settings_set_render_type
	(struct Spectrum_settings *settings,
	enum Spectrum_settings_render_type type);
/*******************************************************************************
LAST MODIFIED : 13 July 1998

DESCRIPTION :
Sets the render type of the Spectrum_settings <settings>.
==============================================================================*/

int Spectrum_settings_copy_and_put_in_list(
	struct Spectrum_settings *settings,void *list_of_settings_void);
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Spectrum_settings iterator function for copying a list_of_settings.
Makes a copy of the settings and puts it in the list_of_settings.
==============================================================================*/

int Spectrum_settings_clear_settings_changed(
	struct Spectrum_settings *settings,void *dummy_void);
/*******************************************************************************
LAST MODIFIED : 10 March 1998

DESCRIPTION :
Iterator function to set settings->settings_changed to 0 (unchanged).
==============================================================================*/

int Spectrum_settings_enable(struct Spectrum_settings *settings,
	void *render_data_void);
/*******************************************************************************
LAST MODIFIED : 23 July 1998

DESCRIPTION :
==============================================================================*/

int Spectrum_settings_activate(struct Spectrum_settings *settings,
	void *render_data_void);
/*******************************************************************************
LAST MODIFIED : 13 March 1998

DESCRIPTION :
Modifies the material in the render data to represent the data value
passed in render data.
==============================================================================*/

int Spectrum_settings_disable(struct Spectrum_settings *settings,
	void *render_data_void);
/*******************************************************************************
LAST MODIFIED : 23 July 1998

DESCRIPTION :
==============================================================================*/

int Spectrum_settings_list_contents(struct Spectrum_settings *settings,
	void *list_data_void);
/*******************************************************************************
LAST MODIFIED : 5 August 1998

DESCRIPTION :
Writes out the <settings> as a text string in the command window with the
<settings_string_detail>, <line_prefix> and <line_suffix> given in the
<list_data>.
==============================================================================*/

int gfx_modify_spectrum_settings_linear(struct Parse_state *state,
	void *modify_spectrum_data_void,void *spectrum_command_data_void);
/*******************************************************************************
LAST MODIFIED : 4 August 1998

DESCRIPTION :
Executes a GFX MODIFY SPECTRUM LINEAR command.
If return_code is 1, returns the completed Modify_spectrum_data with the
parsed settings. Note that the settings are ACCESSed once on valid return.
==============================================================================*/

int gfx_modify_spectrum_settings_log(struct Parse_state *state,
	void *modify_spectrum_data_void,void *spectrum_command_data_void);
/*******************************************************************************
LAST MODIFIED : 4 August 1998

DESCRIPTION :
==============================================================================*/
#endif /* !defined (SPECTRUM_SETTINGS_H) */
