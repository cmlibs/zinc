/*******************************************************************************
FILE : computed_field_vector_operations.h

LAST MODIFIED : 4 July 2000

DESCRIPTION :
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
#if !defined (COMPUTED_FIELD_VECTOR_OPERATIONS_H)
#define COMPUTED_FIELD_VECTOR_OPERATIONS_H

int Computed_field_register_types_vector_operations(
	struct Computed_field_package *computed_field_package);
/*******************************************************************************
LAST MODIFIED : 6 July 2000

DESCRIPTION :
==============================================================================*/

/***************************************************************************//**
 * Creates a field returning the values of source vector field normalised to
 * unit length.
 * 
 * @param field_module  Region field module which will own new field.
 * @param source_field  Source field to normalise.
 * @return Newly created field
 */
struct Computed_field *Computed_field_create_normalise(
	struct Cmiss_field_module *field_module,
	struct Computed_field *source_field);

/***************************************************************************//**
 * Creates a vector field with (dimension) components whose values are given by
 * the cross product of the (dimension-1) source fields. 
 * 
 * @param field_module  Region field module which will own new field.
 * @param dimension  Dimension of the cross product = number of components of
 * resulting field. Only 2, 3 and 4 dimensions supported.
 * @param source_fields  Array of (dimension-1) fields with number of components
 * equal to (dimension).
 * @return Newly created field
 */
struct Computed_field *Computed_field_create_cross_product(
	struct Cmiss_field_module *field_module,
	int dimension, struct Computed_field **source_fields);

/***************************************************************************//**
 * Creates a scalar field whose value is the dot product of the two supplied
 * source fields, which must have equal numbers of components. 
 * 
 * @param field_module  Region field module which will own new field.
 * @param source_field_one  First source field.
 * @param source_field_two  Second source field.
 * @return Newly created field
 */
struct Computed_field *Computed_field_create_dot_product(
	struct Cmiss_field_module *field_module,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two);




/***************************************************************************//**
 * Creates a scalar field returning the magnitude of the vector source field. 
 * 
 * @param field_module  Region field module which will own new field.
 * @param source_field  Source field to normalise.
 * @return Newly created field
 */
struct Computed_field *Computed_field_create_magnitude(
	struct Cmiss_field_module *field_module,
	struct Computed_field *source_field);


int Computed_field_set_type_dot_product(struct Computed_field *field,
	struct Computed_field *source_field_one,
	struct Computed_field *source_field_two);
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_DOT_PRODUCT with the supplied
fields, <source_field_one> and <source_field_two>.  Sets the number of 
components to one.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
#endif /* !defined (COMPUTED_FIELD_VECTOR_OPERATIONS_H) */
