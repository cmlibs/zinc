/*******************************************************************************
FILE : choose_enumerator.h

LAST MODIFIED : 20 January 2000

DESCRIPTION :
Widgets for editing an enumerated value. To overcome type differences, the
enumerated values are stored internally as pointers to static strings. This
means the strings it refers to are those returned by a function (eg.):
char *Enumerated_type_string(enum Enumerated_type type)
where the returned string is NOT allocated but returned as an address in the
code, eg. return_string="enumerated_name_1";
The input to the chooser is an allocated array containing pointers to the
valid static strings, returned from a function like:
char **Enumerated_type_get_valid_strings(&number_of_valid_strings);
==============================================================================*/
#if !defined (CHOOSE_ENUMERATOR_H)
#define CHOOSE_ENUMERATOR_H

#include <Xm/Xm.h>
#include "general/callback.h"

/*
Global Types
------------
*/
struct Choose_enumerator;

/*
Global Functions
---------------
*/
struct Choose_enumerator *CREATE(Choose_enumerator)(Widget parent,
	int number_of_valid_strings,char **valid_strings,char *enumerator_string);
/*******************************************************************************
LAST MODIFIED : 20 January 2000

DESCRIPTION :
Creates an editor for specifying a string out of the <valid_strings>, with the
<enumerator_string> chosen as the current_value. The string should be converted
to its appropriate type by a function like:
enum Enumerated_type Enumerated_type_from_string(char *string);
Note: Choose_enumerator will be automatically DESTROYed with its widgets.
==============================================================================*/

struct Callback_data *choose_enumerator_get_callback(
	struct Choose_enumerator *choose_enumerator);
/*******************************************************************************
LAST MODIFIED : 19 January 2000

DESCRIPTION :
Returns a pointer to the callback item of the choose_enumerator_widget.
==============================================================================*/

int choose_enumerator_set_callback(struct Choose_enumerator *choose_enumerator,
	struct Callback_data *new_callback);
/*******************************************************************************
LAST MODIFIED : 19 January 2000

DESCRIPTION :
Changes the callback item of the choose_enumerator_widget.
==============================================================================*/

char *choose_enumerator_get_string(struct Choose_enumerator *choose_enumerator);
/*******************************************************************************
LAST MODIFIED : 19 January 2000

DESCRIPTION :
Returns the current enumerator_string in use by the editor. Calling function
must not destroy or modify the returned static string.
==============================================================================*/

int choose_enumerator_set_string(struct Choose_enumerator *choose_enumerator,
	char *enumerator_string);
/*******************************************************************************
LAST MODIFIED : 19 January 2000

DESCRIPTION :
Changes the enumerator_string in the choose_enumerator_widget.
==============================================================================*/

int choose_enumerator_set_valid_strings(
	struct Choose_enumerator *choose_enumerator,int number_of_valid_strings,
	char **valid_strings,char *enumerator_string);
/*******************************************************************************
LAST MODIFIED : 20 January 2000

DESCRIPTION :
Changes the list of <valid_strings> in the choose_enumerator_widget.
==============================================================================*/
#endif /* !defined (CHOOSE_ENUMERATOR_H) */
