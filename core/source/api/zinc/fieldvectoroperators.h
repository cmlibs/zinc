/***************************************************************************//**
 * FILE : cmiss_field_vector_operators.h
 *
 * The public interface to the Cmiss_fields that perform vector operations.
 */
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
 * Portions created by the Initial Developer are Copyright (C) 2011
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
#ifndef __CMISS_FIELD_VECTOR_OPERATORS_H__
#define __CMISS_FIELD_VECTOR_OPERATORS_H__

#include "types/fieldid.h"
#include "types/fieldmoduleid.h"

#include "zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************//**
 * Creates a vector field with (dimension) components whose values are given by
 * the cross product of the (dimension-1) source fields.
 *
 * @param field_module  Region field module which will own new field.
 * @param dimension  Dimension of the cross product = number of components of
 * resulting field. Only 2, 3 and 4 dimensions supported.
 * @param source_fields  Array of (dimension-1) fields with number of components
 * equal to (dimension).
 * @return  Newly created field
 */
ZINC_API Cmiss_field_id Cmiss_field_module_create_cross_product(
	Cmiss_field_module_id field_module, int dimension,
	Cmiss_field_id *source_fields);

/***************************************************************************//**
 * Convenience function creating a field giving the 3-D cross product of two
 * 3-component vector source fields. Resulting field has 3 components.
 *
 * @param field_module  Region field module which will own new field.
 * @param source_field_one  First source field. Must have 3 components.
 * @param source_field_two  Second source field.  Must have 3 components.
 * @return  Newly created field
 */
ZINC_API Cmiss_field_id Cmiss_field_module_create_cross_product_3d(
	Cmiss_field_module_id field_module, Cmiss_field_id source_field_one,
	Cmiss_field_id source_field_two);

/***************************************************************************//**
 * Creates a scalar field whose value is the dot product of the two supplied
 * source fields, which must have equal numbers of components.
 *
 * @param field_module  Region field module which will own new field.
 * @param source_field_one  First source field.
 * @param source_field_two  Second source field.
 * @return  Newly created field
 */
ZINC_API Cmiss_field_id Cmiss_field_module_create_dot_product(
	Cmiss_field_module_id field_module, Cmiss_field_id source_field_one,
	Cmiss_field_id source_field_two);

/***************************************************************************//**
 * Creates a scalar field returning the magnitude of the vector source field.
 *
 * @param field_module  Region field module which will own new field.
 * @param source_field  Source field to normalise.
 * @return  Newly created field
 */
ZINC_API Cmiss_field_id Cmiss_field_module_create_magnitude(
	Cmiss_field_module_id field_module, Cmiss_field_id source_field);

/***************************************************************************//**
 * Creates a field returning the values of source vector field normalised to
 * unit length.
 *
 * @param field_module  Region field module which will own new field.
 * @param source_field  Source field to normalise.
 * @return  Newly created field
 */
ZINC_API Cmiss_field_id Cmiss_field_module_create_normalise(
	Cmiss_field_module_id field_module, Cmiss_field_id source_field);

#ifdef __cplusplus
}
#endif

#endif /* __CMISS_FIELD_VECTOR_OPERATORS_H__ */
