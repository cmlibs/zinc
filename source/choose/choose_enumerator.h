/*******************************************************************************
FILE : choose_enumerator.h

LAST MODIFIED : 21 November 2001

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
#if !defined (CHOOSE_ENUMERATOR_H)
#define CHOOSE_ENUMERATOR_H

#include <Xm/Xm.h>
#include "general/callback_motif.h"
#include "user_interface/user_interface.h"

/*
Global Types
------------
*/

/*
Global Functions
---------------
*/

Widget create_choose_enumerator_widget(Widget parent,
	int number_of_valid_strings, char **valid_strings, char *enumerator_string,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 21 November 2001

DESCRIPTION :
Creates an editor for specifying a string out of the <valid_strings>, with the
<enumerator_string> chosen as the current_value. The string should be converted
to its appropriate type by a function like:
enum Enumerated_type Enumerated_type_from_string(char *string);
Note: Choose_enumerator will be automatically DESTROYed with its widgets.
<user_interface> supplies fonts.
==============================================================================*/

struct Callback_data *choose_enumerator_get_callback(
	Widget choose_enumerator_widget);
/*******************************************************************************
LAST MODIFIED : 21 January 2000

DESCRIPTION :
Returns a pointer to the callback item of the choose_enumerator_widget.
==============================================================================*/

int choose_enumerator_set_callback(Widget choose_enumerator_widget,
	struct Callback_data *new_callback);
/*******************************************************************************
LAST MODIFIED : 21 January 2000

DESCRIPTION :
Changes the callback item of the choose_enumerator_widget.
==============================================================================*/

char *choose_enumerator_get_string(Widget choose_enumerator_widget);
/*******************************************************************************
LAST MODIFIED : 21 January 2000

DESCRIPTION :
Returns the current enumerator_string in use by the editor. Calling function
must not destroy or modify the returned static string.
==============================================================================*/

int choose_enumerator_set_string(Widget choose_enumerator_widget,
	char *enumerator_string);
/*******************************************************************************
LAST MODIFIED : 21 January 2000

DESCRIPTION :
Changes the enumerator_string in the choose_enumerator_widget.
==============================================================================*/

int choose_enumerator_set_valid_strings(Widget choose_enumerator_widget,
	int number_of_valid_strings,char **valid_strings,char *enumerator_string);
/*******************************************************************************
LAST MODIFIED : 21 January 2000

DESCRIPTION :
Changes the list of <valid_strings> in the choose_enumerator_widget.
==============================================================================*/
#endif /* !defined (CHOOSE_ENUMERATOR_H) */
