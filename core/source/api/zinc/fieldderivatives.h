/*******************************************************************************
FILE : cmiss_field_derivatives.h

DESCRIPTION :
The public interface to the cmzn_fields that perform derivation.
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
 * Portions created by the Initial Developer are Copyright (C) 2008
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 * Shane Blackett (shane at blackett.co.nz)
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
#ifndef CMZN_FIELDDERIVATIVES_H__
#define CMZN_FIELDDERIVATIVES_H__

#include "types/fieldid.h"
#include "types/fieldmoduleid.h"

#include "zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Creates a field returning the derivative of the field with respect to element
 * xi_index as its primary value. Returned field has same number of components
 * as source field.
 *
 * @param field_module  Region field module which will own new field.
 * @param source_field  Source field to get derivative number.
 * @param xi_index  Element coordinate system index for derivative, from 1 to
 * element dimension.
 * @return Newly created field
 */
ZINC_API cmzn_field_id cmzn_field_module_create_derivative(
	cmzn_field_module_id field_module,
	cmzn_field_id source_field, int xi_index);

/**
 * Creates a field returning the curl of vector_field at location given by
 * coordinate_field. All fields including return field have 3 components.
 *
 * @param field_module  Region field module which will own new field.
 * @param vector_field  Vector field from which curl is evaluated. Must have
 * rectangular cartesian coordinate system.
 * @param coordinate_field  Field supplying location.
 * @return Newly created field
 */
ZINC_API cmzn_field_id cmzn_field_module_create_curl(
	cmzn_field_module_id field_module,
	cmzn_field_id vector_field, cmzn_field_id coordinate_field);

/**
 * Creates a scalar field returning the divergence of vector field within
 * coordinate field.
 * The number of components of <vector_field> and <coordinate_field> must be the
 * same and less than or equal to 3.
 *
 * @param field_module  Region field module which will own new field.
 * @param vector_field  Vector field from which divergence is evaluated. Must
 * have rectangular cartesian coordinate system.
 * @param coordinate_field  Field supplying location.
 * @return Newly created field
 */
ZINC_API cmzn_field_id cmzn_field_module_create_divergence(
	cmzn_field_module_id field_module,
	cmzn_field_id vector_field, cmzn_field_id coordinate_field);

/**
 * Creates a field returning the curl of vector_field at location given by
 * coordinate_field. All fields including return field have 3 components.
 * Converts a field returning the gradient of a source field with respect to
 * a given coordinate field. Calculation will only succeed in any element with
 * xi-dimension equal to the number of components in the <coordinate_field>.
 * Sets the number of components in return field to the product of the number
 * of components in the <source_field> and <coordinate_field>.
 * Note the <source_field> does not have to be a scalar. If it has more than 1
 * component, all the derivatives of its first component w.r.t. the components
 * of <coordinate_field> will be returned first, followed by those of the second
 * component, etc. Hence, this function can return the standard gradient of a
 * scalar source_field, and the deformation gradient if a deformed coordinate
 * field is passed as the source_field.
 *
 * @param field_module  Region field module which will own new field.
 * @param source_field  Field to calculate gradients for.
 * @param coordinate_field  Field supplying location.
 * @return Newly created field
 */
ZINC_API cmzn_field_id cmzn_field_module_create_gradient(
	cmzn_field_module_id field_module,
	cmzn_field_id source_field, cmzn_field_id coordinate_field);

#ifdef __cplusplus
}
#endif

#endif /* CMZN_FIELDDERIVATIVES_H__ */
