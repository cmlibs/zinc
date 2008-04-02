/*******************************************************************************
FILE : computed_field_function.h

LAST MODIFIED : 31 March 2008

DESCRIPTION :
Implements a "function" computed_field which returns the values of
<result_field> with respect to the <source_field> values 
being the inputs for <reference_field>.
The sequence of operations <reference_field> to <result_field> 
become a function operating on the input <source_field> values.
which converts fields, field components
and real values in any order into a single vector field.
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
#if !defined (COMPUTED_FIELD_FUNCTION_H)
#define COMPUTED_FIELD_FUNCTION_H

int Computed_field_set_type_function(struct Computed_field *field,
	struct Computed_field *source_field, struct Computed_field *result_field,
	struct Computed_field *reference_field);
/*******************************************************************************
LAST MODIFIED : 31 March 2008

DESCRIPTION :
Converts <field> into a function field which returns the values of
<result_field> with respect to the <source_field> values 
being the inputs for <reference_field>.
The sequence of operations <reference_field> to <result_field> 
become a function operating on the input <source_field> values.
==============================================================================*/

int Computed_field_get_type_function(struct Computed_field *field,
	struct Computed_field **source_field, struct Computed_field **result_field,
	struct Computed_field **reference_field);
/*******************************************************************************
LAST MODIFIED : 31 March 2008

DESCRIPTION :
If the field is of type COMPUTED_FIELD_FUNCTION, the function returns the three
fields which define the field.
==============================================================================*/

int Computed_field_register_types_function(
	struct Computed_field_package *computed_field_package);
/*******************************************************************************
LAST MODIFIED : 31 March 2008

DESCRIPTION :
==============================================================================*/
#endif /* !defined (COMPUTED_FIELD_FUNCTION_H) */
