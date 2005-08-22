/*******************************************************************************
FILE : computed_field_component_operations.h

LAST MODIFIED : 13 July 2000

DESCRIPTION :
Implements a number of basic component wise operations on computed fields.
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
#if !defined (COMPUTED_FIELD_COMPONENT_OPERATIONS_H)
#define COMPUTED_FIELD_COMPONENT_OPERATIONS_H

int Computed_field_register_types_component_operations(
	struct Computed_field_package *computed_field_package);
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
==============================================================================*/

int Computed_field_set_type_add(struct Computed_field *field,
	struct Computed_field *source_field_one, FE_value scale_factor1,
	struct Computed_field *source_field_two, FE_value scale_factor2);
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_ADD with the supplied
fields, <source_field_one> and <source_field_two>.  Sets the number of 
components equal to the source_fields.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/

int Computed_field_set_type_divide_components(struct Computed_field *field,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two);
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_DIVIDE_COMPONENTS with the supplied
fields, <source_field_one> and <source_field_two>.  Sets the number of 
components equal to the source_fields.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/

int Computed_field_set_type_offset(struct Computed_field *field,
	struct Computed_field *source_field, FE_value *offsets);
/*******************************************************************************
LAST MODIFIED : 21 January 2002

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_OFFSET which returns the values of the
<source_field> plus the <offsets>.
The <offsets> array must therefore contain as many FE_values as there are
components in <source_field>; this is the number of components in the field.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
#endif /* !defined (COMPUTED_FIELD_COMPONENT_OPERATIONS_H) */
