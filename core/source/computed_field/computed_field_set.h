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

/***************************************************************************//**
 * Return the field from the manager with the given name. If none is found with
 * that name, try to find the field component as fieldname.componentname, i.e.
 * with the '.' separator. Since FieldML examples frequently use '.' in the
 * field name itself, search after the last '.' character.
 * @param component_number_address  On successful return this will have the
 * value -1 for the whole field, or the component number from 0 to count - 1.
 * @return  Non-accessed field on success, or NULL on failure.
 */
cmzn_field_id Computed_field_manager_get_field_or_component(
	struct MANAGER(Computed_field) *computed_field_manager, const char *name,
	int *component_number_address);

#endif /* !defined (COMPUTED_FIELD_SET_H) */
