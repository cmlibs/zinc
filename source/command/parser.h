/*******************************************************************************
FILE : parser.h

LAST MODIFIED : 22 December 2000

DESCRIPTION :
Public interface for the beginnings of a simple parser (although at the moment
it is nothing but a strings container, and some comparison functions)
==============================================================================*/
#if !defined (PARSER_H)
#define PARSER_H

#include "general/object.h"
#include "general/value.h"

/*
Global types
------------
*/
struct Parse_state;

typedef int (*modifier_function)(struct Parse_state *state,void *to_be_modified,
		void *user_data);

/*
Global constants
----------------
*/
#define PARSER_HELP_STRING "?"
#define PARSER_RECURSIVE_HELP_STRING "??"

/*
Global structures
-----------------
*/

struct Parse_state
/*******************************************************************************
LAST MODIFIED : 12 June 1996

DESCRIPTION :
???DB.  Need an access_count ?
==============================================================================*/
{
	char **tokens;
	int number_of_tokens;
	int current_index;
	char *current_token;
	char *command_string;
}; /* struct Parse_state */

struct Modifier_entry
/*******************************************************************************
LAST MODIFIED : 18 June 1996

DESCRIPTION :
This structure contains the variable <to_be_modified> and the <modifier>
function to be used when a particular <option> is encounterd when parsing a
command.  The <modifier> function will be called with the current parse state
and the variable <to_be_modified>.  If the <modifier> function is called with a
NULL <state> and a NULL variable <to_be_modified> it should write a description
of the values it expects to the command window.
==============================================================================*/
{
	char *option;
	void *to_be_modified;
	void *user_data;
	modifier_function modifier;
}; /* struct Modifier_entry */

struct Set_vector_with_help_data
/*******************************************************************************
LAST MODIFIED : 7 October 1998

DESCRIPTION :
Data structure used with function set_double_vector_with_help to allow a vector
of double values to be parsed, with custom help text to be printed out as well.
???RC  Could be used with other types: float, int etc.
==============================================================================*/
{
	/* number of values to be read in */
	int num_values;
	/* text to print in help mode */
	char *help_text;
	/* Should initially be cleared to 0, set to 1 if values read in */
	char set;
};

/*
Global functions
----------------
*/
int fuzzy_string_compare(char *first,char *second);
/*******************************************************************************
LAST MODIFIED : 21 June 1999

DESCRIPTION :
This is a case insensitive compare disregarding certain characters (whitespace,
dashes and underscores).  For example, "Ambient Colour" matches the following:

"AMBIENT_COLOUR", "ambient_colour", "ambientColour", "Ambient_Colour",
"ambient-colour", "AmBiEnTcOlOuR", "Ambient-- Colour"

and a large set of even louder versions. The strings are compared up to the
length of the shortest of first and second.

Returns 1 if the strings match, 0 if they do not.
==============================================================================*/

int fuzzy_string_compare_same_length(char *first,char *second);
/*******************************************************************************
LAST MODIFIED : 14 August 1998

DESCRIPTION :
Same as fuzzy_string_compare except that the two reduced strings must be the
same length.
==============================================================================*/

int process_option(struct Parse_state *state,
	struct Modifier_entry *modifier_table);
/*******************************************************************************
LAST MODIFIED : 18 June 1996

DESCRIPTION :
If the <state->current_string> is "?", then the options in the <modifier_table>
and the values expected for each will be written to the command window and 1
returned.  Otherwise, the <modifier_table> is searched for entries whose option
field matchs <state->current_string>.  If no matchs are found, then if the
terminating entry in the <modifier_table> has a modifier function it is called,
otherwise an error message is written and 0 returned.  If one match is found,
then the modifier function of the entry is called and its return value returned.
If more than one match is found then the possible matchs are written to the
command window and 0 is returned.  Note that <process_option> is a modifier
function.
==============================================================================*/

int process_multiple_options(struct Parse_state *state,
	struct Modifier_entry *modifier_table);
/*******************************************************************************
LAST MODIFIED : 27 September 1996

DESCRIPTION :
==============================================================================*/

struct Option_table *CREATE(Option_table)(void);
/*******************************************************************************
LAST MODIFIED : 15 December 1999

DESCRIPTION :
Creates an Option_table for text parsing.
==============================================================================*/

int DESTROY(Option_table)(struct Option_table **option_table_address);
/*******************************************************************************
LAST MODIFIED : 15 December 1999

DESCRIPTION :
==============================================================================*/

int Option_table_add_entry(struct Option_table *option_table,char *token,
	void *to_be_modified,void *user_data,modifier_function modifier);
/*******************************************************************************
LAST MODIFIED : 15 December 1999

DESCRIPTION :
Adds the given <token> etc. to the option table, enlarging the table as needed.
Note that if any error occurs, the option_table is marked as being invalid and
no further errors will be reported on subsequent calls.
==============================================================================*/

int Option_table_add_suboption_table(struct Option_table *option_table,
	struct Option_table *suboption_table);
/*******************************************************************************
LAST MODIFIED : 18 December 1999

DESCRIPTION :
Checks that <suboption_table> is valid, and if so, adds it to <option_table>.
On calling this function, <suboption_table> is owned by <option_table> and the
latter is responsible for destroying it. It will be destroyed immediately if it
is invalid or cannot be added to list of entries.
Mechanism currently used to handle enumerated options, though it does not insist
that only one valid enumerator is entered.
Note that if any error occurs, the option_table is marked as being invalid and
no further errors will be reported on subsequent calls.
Note must not make any further changes to suboption_table after it is made part
of option_table!
==============================================================================*/

int Option_table_add_enumerator(struct Option_table *option_table,
	int number_of_valid_strings,char **valid_strings,
	char **enumerator_string_address);
/*******************************************************************************
LAST MODIFIED : 20 December 1999

DESCRIPTION :
Adds a newly created suboption table for all the valid_strings for the
enumerator. The <valid_strings> array should contain <number_of_valid_strings>
pointers to static strings, one per enumerator option. Responsibility for
deallocating this array is left to the calling function. The static string value
of the enumerator is maintained in <enumerator_string_address> and it is up to
the calling function to convert back to an enumerated value.
Note that if any error occurs, the option_table is marked as being invalid and
no further errors will be reported on subsequent calls.
==============================================================================*/

int Option_table_add_switch(struct Option_table *option_table,
	char *on_string,char *off_string,int *value_address);
/*******************************************************************************
LAST MODIFIED : 12 May 2000

DESCRIPTION :
Adds a newly created suboption table containing 2 items:
an <on_string> token that invokes set_int_switch with <value_address>;
an <off_string> token that invokes unset_int_switch with <value_address>;
The <on_string> and <off_string> should be static, eg. passed in quotes.
==============================================================================*/

int Option_table_parse(struct Option_table *option_table,
	struct Parse_state *state);
/*******************************************************************************
LAST MODIFIED : 15 December 1999

DESCRIPTION :
Parses the options in the <option_table>, giving only one option a chance to be
entered.
==============================================================================*/

int Option_table_multi_parse(struct Option_table *option_table,
	struct Parse_state *state);
/*******************************************************************************
LAST MODIFIED : 15 December 1999

DESCRIPTION :
Parses the options in the <option_table>, giving all options a chance to be
entered.
==============================================================================*/

struct Parse_state *create_Parse_state(char *command_string);
/*******************************************************************************
LAST MODIFIED : 12 June 1996

DESCRIPTION :
Creates a Parse_state structure which contains
- a trimmed copy of the <command_string>
- the <command_string> split into tokens
NB
1 ! and # indicate that the rest of the command string is a comment (not split
	into tokens
2 Variables are converted into values
==============================================================================*/

int destroy_Parse_state(struct Parse_state **state_address);
/*******************************************************************************
LAST MODIFIED : 18 November 1994

DESCRIPTION :
==============================================================================*/

int Parse_state_help_mode(struct Parse_state *state);
/*******************************************************************************
LAST MODIFIED : 12 May 2000

DESCRIPTION :
Returns 1 if the current_token in <state> is either of
PARSER_HELP_STRING or PARSER_RECURSIVE_HELP_STRING.
==============================================================================*/

int shift_Parse_state(struct Parse_state *state,int shift);
/*******************************************************************************
LAST MODIFIED : 18 November 1994

DESCRIPTION :
==============================================================================*/

int display_parse_state_location(struct Parse_state *state);
/*******************************************************************************
LAST MODIFIED : 27 June 1996

DESCRIPTION :
Shows the current location in the parse <state>.
==============================================================================*/

int Parse_state_append_to_command_string(struct Parse_state *state,
	char *addition);
/*******************************************************************************
LAST MODIFIED : 29 October 1999

DESCRIPTION :
Appends the <addition> string to the end of the current command_string stored in
the <state>.  Useful for changing the kept history echoed to the command window.
==============================================================================*/

int parse_variable(char **token);
/*******************************************************************************
LAST MODIFIED : 17 February 1998

DESCRIPTION :
Replaces occurrences of %<f/i/z/l><nnn>% with the value of that variable.  May
be called recursively.  Reallocates <*token>, so there is no problem with
over-writing.
==============================================================================*/

int execute_variable_command(struct Parse_state *parse_state,
	void *dummy_to_be_modified,void *user_data);
/*******************************************************************************
LAST MODIFIED : 18 June 1996

DESCRIPTION :
Executes a VARIABLE command.
==============================================================================*/

int execute_assign_variable(struct Parse_state *state,
	void *dummy_to_be_modified,void *dummy_user_data);
/*******************************************************************************
LAST MODIFIED : 10 March 2000

DESCRIPTION :
Executes an ASSIGN VARIABLE command.  Does a very small subset of the intended
use of this command.
==============================================================================*/

int destroy_assign_variable_list(void);
/*******************************************************************************
LAST MODIFIED : 10 March 2000

DESCRIPTION :
Clean up the global assign_variable_list.
==============================================================================*/

int set_name(struct Parse_state *state,void *name_address_void,
	void *prefix_space);
/*******************************************************************************
LAST MODIFIED : 27 May 1997

DESCRIPTION :
Allocates memory for a name, then copies the passed string into it.
==============================================================================*/

int set_names(struct Parse_state *state,void *names_void,
	void *number_of_names_address_void);
/*******************************************************************************
LAST MODIFIED : 10 May 2000

DESCRIPTION :
Modifier function for reading number_of_names (>0) string names from
<state>. User data consists of a pointer to an integer containing the
number_of_names, while <names_void> should point to a large enough space to
store the number_of_names pointers. The names in this array must either be NULL
or pointing to allocated strings.
==============================================================================*/

int set_int(struct Parse_state *state,void *value_address_void,
	void *dummy_user_data);
/*******************************************************************************
LAST MODIFIED : 18 June 1996

DESCRIPTION :
A modifier function for setting a int.
==============================================================================*/

int set_int_optional(struct Parse_state *state,void *value_address_void,
	void *dummy_user_data);
/*******************************************************************************
LAST MODIFIED : 25 February 1997

DESCRIPTION :
If the next token is an integer then the int is set to that value otherwise the
int is set to 1.
==============================================================================*/

int set_int_non_negative(struct Parse_state *state,void *value_address_void,
	void *dummy_user_data);
/*******************************************************************************
LAST MODIFIED : 27 February 1997

DESCRIPTION :
A modifier function for setting a int to a non-negative value.
==============================================================================*/

int set_int_positive(struct Parse_state *state,void *value_address_void,
	void *dummy_user_data);
/*******************************************************************************
LAST MODIFIED : 18 June 1996

DESCRIPTION :
A modifier function for setting a int to a positive value.
==============================================================================*/

int set_int_and_char_flag(struct Parse_state *state,void *value_address_void,
	void *flag_address_void);
/*******************************************************************************
LAST MODIFIED : 22 December 2000

DESCRIPTION :
A modifier function for setting an int, and a char flag in the user data to
indicate that the int has been set.
==============================================================================*/

int set_int_vector(struct Parse_state *state,void *values_address_void,
	void *number_of_components_address_void);
/*******************************************************************************
LAST MODIFIED : 27 March 2000

DESCRIPTION :
Modifier function for reading number_of_components (>0) ints from <state>.
User data consists of a pointer to an integer containing number_of_components,
while <values_address_void> should point to a large enough space to store the
number_of_components ints.
Now prints current contents of the vector with help.
==============================================================================*/

int set_float(struct Parse_state *state,void *value_address_void,
	void *dummy_user_data);
/*******************************************************************************
LAST MODIFIED : 18 June 1996

DESCRIPTION :
A modifier function for setting a float.
==============================================================================*/

int set_float_and_char_flag(struct Parse_state *state,void *value_address_void,
	void *flag_address_void);
/*******************************************************************************
LAST MODIFIED : 9 September 1999

DESCRIPTION :
A modifier function for setting a float, and a char flag in the user data to
indicate that the float has been set.
==============================================================================*/

int set_float_positive(struct Parse_state *state,void *value_address_void,
	void *dummy_user_data);
/*******************************************************************************
LAST MODIFIED : 18 June 1996

DESCRIPTION :
A modifier function for setting a float to a positive value.
==============================================================================*/

int set_float_non_negative(struct Parse_state *state,void *value_address_void,
	void *dummy_user_data);
/*******************************************************************************
LAST MODIFIED : 18 June 1996

DESCRIPTION :
A modifier function for setting a float to a non-negative value.
==============================================================================*/

int set_float_0_to_1_inclusive(struct Parse_state *state,
	void *value_address_void,void *user_data);
/*******************************************************************************
LAST MODIFIED : 18 June 1996

DESCRIPTION :
A modifier function for setting a float to a value in [0,1].
==============================================================================*/

int set_double(struct Parse_state *state,void *value_address_void,
	void *dummy_user_data);
/*******************************************************************************
LAST MODIFIED : 21 July 1996

DESCRIPTION :
A modifier function for setting a double.
==============================================================================*/

int set_double_and_char_flag(struct Parse_state *state,void *value_address_void,
	void *flag_address_void);
/*******************************************************************************
LAST MODIFIED : 19 October 1998

DESCRIPTION :
A modifier function for setting a double, and a char flag in the user data to
indicate that the double has been set.
???SAB  The user_data could be used to supply many more helpful things such as
	limits on the double or a string used in the help.
==============================================================================*/

int set_special_float3(struct Parse_state *state,void *values_address_void,
	void *separation_char_address_void);
/*******************************************************************************
LAST MODIFIED : 9 July 1998

DESCRIPTION :
Modifier function for setting a float[3] from a token with 1 to 3 characters
separated by the character at <separation_char_address> which may be either an
asterisk or a comma. If '*' is used missing components take the values of the
last number entered, eg '3' -> 3,3,3, while  '2.4*7.6' becomes 2.4,7.6,7.6.
This functionality is useful for setting the size of glyphs. If the separation
character is ',', values of unspecified components are left untouched, useful
for setting glyph offsets which default to zero or some other number.
Missing a number by putting two separators together works as expected, eg:
'1.2**3.0' returns 1.2,1.2,3.0, '*2' gives 0.0,2.0,2.0 while ',,3' changes the
third component of the float only to 3.
==============================================================================*/

int set_float_vector(struct Parse_state *state,void *values_address_void,
	void *number_of_components_address_void);
/*******************************************************************************
LAST MODIFIED : 12 September 1997

DESCRIPTION :
Modifier function for reading number_of_components (>0) floats from <state>.
User data consists of a pointer to an integer containing number_of_components,
while <values_address_void> should point to a large enough space to store the
number_of_components floats.
==============================================================================*/

int set_FE_value_array(struct Parse_state *state,void *values_address_void,
	void *number_of_components_address_void);
/*******************************************************************************
LAST MODIFIED : 11 March 1999

DESCRIPTION :
Modifier function for reading number_of_components (>0) FE_values from <state>.
User data consists of a pointer to an integer containing number_of_components,
while <values_address_void> should point to a large enough space to store the
number_of_components FE_values.
Now prints current contents of the vector with help.
==============================================================================*/

int set_double_vector(struct Parse_state *state,void *values_address_void,
	void *number_of_components_address_void);
/*******************************************************************************
LAST MODIFIED : 14 January 1998

DESCRIPTION :
Modifier function for reading number_of_components (>0) double from <state>.
User data consists of a pointer to an integer containing number_of_components,
while <values_address_void> should point to a large enough space to store the
number_of_components doubles.
==============================================================================*/

int set_double_vector_with_help(struct Parse_state *state,
	void *vector_void,void *set_vector_with_help_data_void);
/*******************************************************************************
LAST MODIFIED : 7 October 1998

DESCRIPTION :
Modifier function to parse a variable number of double values with appropriate
help text.
==============================================================================*/

int set_char_flag(struct Parse_state *state,void *to_be_modified,
	void *user_data);
/*******************************************************************************
LAST MODIFIED : 18 June 1996

DESCRIPTION :
A modifier function for setting a character flag to 1.
==============================================================================*/

int unset_char_flag(struct Parse_state *state,void *value_address_void,
	void *dummy_user_data);
/*******************************************************************************
LAST MODIFIED : 5 November 1997

DESCRIPTION :
A modifier function for setting a character flag to 0.
==============================================================================*/

int set_int_switch(struct Parse_state *state,void *value_address_void,
	void *token_void);
/*******************************************************************************
LAST MODIFIED : 12 May 2000

DESCRIPTION :
A modifier function for setting an integer switch to 1.
If the value is currently set, this is indicated in the help, with the <token>
if supplied, otherwise the word CURRENT.
If the option's <token> is supplied and its value is currently set, it 
==============================================================================*/

int unset_int_switch(struct Parse_state *state,void *value_address_void,
	void *token_void);
/*******************************************************************************
LAST MODIFIED : 12 May 2000

DESCRIPTION :
A modifier function for setting an integer switch to 0.
If the value is currently unset, this is indicated in the help, with the <token>
if supplied, otherwise the word CURRENT.
If the option's <token> is supplied and its value is currently set, it 
==============================================================================*/

int unset_int_switch(struct Parse_state *state,void *value_address_void,
	void *dummy_user_data);
/*******************************************************************************
LAST MODIFIED : 12 May 2000

DESCRIPTION :
A modifier function for setting an integer switch to 0.
==============================================================================*/

int set_file_name(struct Parse_state *state,void *name_address_void,
	void *directory_name_address_void);
/*******************************************************************************
LAST MODIFIED : 23 September 1996

DESCRIPTION :
Allows the user to specify "special" directories, eg examples.  Allocates the
memory for the file name string.
==============================================================================*/

int set_nothing(struct Parse_state *state,void *dummy_to_be_modified,
	void *dummy_user_data);
/*******************************************************************************
LAST MODIFIED : 1 October 1996

DESCRIPTION :
Always succeeds and does nothing to the parse <state>.
???DB.  Temporary ?
==============================================================================*/

int set_integer_range(struct Parse_state *state,
	void *integer_range_address_void,void *dummy_user_data);
/*******************************************************************************
LAST MODIFIED : 29 September 1997

DESCRIPTION :
integer_range= *((int **)integer_range_address_void) is an array.
integer_range[0] is the number of pairs of ints in the rest of integer_range, so
that the size of integer_range is 1+2*integer_range[0].  integer_range is
ordered integer_range[2*i+1]<=integer_range[2*i+2]<integer_range[2*i+3] for
i>=0.  The integers in integer_range are those j for which
integer_range[2*i+1]<=j<=integer_range[2*i+2] for some i>=0.

This routine updates the integer_range based on the current token which can be
of two forms - # or #..#
==============================================================================*/

int set_enum(struct Parse_state *state,void *set_value_address_void,
	void *enum_value_address_void);
/*******************************************************************************
LAST MODIFIED : 19 November 1998

DESCRIPTION :
A modifier function for setting an enumerated type variable to a specified
value.
NB.  *enum_value_address_void is put in *set_value_address_void
???DB.  Unwieldy.  Can it be done better ?
==============================================================================*/
#endif /* !defined (PARSER_H) */
