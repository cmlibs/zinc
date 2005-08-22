/*******************************************************************************
FILE : computed_field_set.h

LAST MODIFIED : 4 December 2003

DESCRIPTION :
Functions for selecting computed fields from the commands line.
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
#if !defined (COMPUTED_FIELD_SET_H)
#define COMPUTED_FIELD_SET_H

#include "command/parser.h"
#include "computed_field/computed_field.h"

/*
Global types
------------
*/

struct Set_Computed_field_conditional_data
/*******************************************************************************
LAST MODIFIED : 3 February 1999

DESCRIPTION :
User data structure passed to set_Computed_field_conditional, containing the
computed_field_package and the optional conditional_function (and
conditional_function_user_data) for selecting a field out of a subset of the
fields in the manager.
==============================================================================*/
{
	MANAGER_CONDITIONAL_FUNCTION(Computed_field) *conditional_function;
	void *conditional_function_user_data;
	struct MANAGER(Computed_field) *computed_field_manager;
}; /* struct Set_Computed_field_conditional_data */

struct Set_Computed_field_array_data
/*******************************************************************************
LAST MODIFIED : 9 March 1999

DESCRIPTION :
User data structure passed to set_Computed_field_array, containing the number of
fields in the array, and a pointer to a Set_Computed_field_conditional_data
structure for passing to set_Computed_field_conditional - this is done for each
element in the array.
==============================================================================*/
{
	int number_of_fields;
	struct Set_Computed_field_conditional_data *conditional_data;
}; /* struct Set_Computed_field_array_data */

/*
Global functions
----------------
*/

int set_Computed_field_conditional(struct Parse_state *state,
	void *field_address_void, void *set_field_data_void);
/*******************************************************************************
LAST MODIFIED : 17 December 2001

DESCRIPTION :
Modifier function to set the field from a command. <set_field_data_void> should
point to a struct Set_Computed_field_conditional_data containing the
computed_field_manager and an optional conditional function for narrowing the
range of fields available for selection. If the conditional_function is NULL,
no criteria are placed on the chosen field.
Allows the construction field.component name to automatically make a component
wrapper for field and add it to the manager.
==============================================================================*/

int Option_table_add_Computed_field_conditional_entry(
	struct Option_table *option_table, char *token, 
	struct Computed_field **field_address, 
	struct Set_Computed_field_conditional_data *set_field_data);
/*******************************************************************************
LAST MODIFIED : 4 December 2003

DESCRIPTION :
Adds the given <token> to the <option_table>.
==============================================================================*/

int set_Computed_field_array(struct Parse_state *state,
	void *field_array_void, void *set_field_array_data_void);
/*******************************************************************************
LAST MODIFIED : 17 December 2001

DESCRIPTION :
Modifier function to set an array of field from a command.
<set_field_array_data_void> should point to a struct
Set_Computed_field_array_conditional_data containing the number_of_fields in the
array, the computed_field_package and an optional conditional function for
narrowing the range of fields available for selection.
Works by repeatedly calling set_Computed_field_conditional.
???RC Make this globally available for calling any modifier function?
==============================================================================*/

#endif /* !defined (COMPUTED_FIELD_SET_H) */
