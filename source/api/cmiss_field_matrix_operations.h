/*******************************************************************************
FILE : cmiss_field_matrix_operations.h

LAST MODIFIED : 17 June 2011

DESCRIPTION :
The public interface to the Cmiss_fields that perform matrix operations.
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
#ifndef __CMISS_FIELD_MATRIX_OPERATIONS_H__
#define __CMISS_FIELD_MATRIX_OPERATIONS_H__

#include "api/cmiss_field.h"

/**
 * Creates a field returning the N eigenvalues of symmetric N*N component source
 * field.
 *
 * @param field_module  Region field module which will own new field.
 * @param source_field  N*N component square symmetric matrix field.
 * @return Newly created field
 */
Cmiss_field_id Cmiss_field_module_create_eigenvalues(
	Cmiss_field_module_id field_module,
	Cmiss_field_id source_field);

/**
 * Creates a field returning the N, N-dimensional eigenvectors computed with the
 * source eigenvalues field. Sets the number of components equal to N*N, where
 * N is the number of components in the <eigenvalues_field>.
 *
 * @param field_module  Region field module which will own new field.
 * @param eigenvalues_field  Eigenvalues type field.
 * @return Newly created field
 */
Cmiss_field_id Cmiss_field_module_create_eigenvectors(
	Cmiss_field_module_id field_module,
	Cmiss_field_id eigenvalues_field);

/**
 * Creates a field returning the inverse of N*N symmetric matrix valued source
 * field.
 *
 * @param field_module  Region field module which will own new field.
 * @param source_field  N*N component square symmetric matrix field.
 * @return Newly created field
 */
Cmiss_field_id Cmiss_field_module_create_matrix_invert(
	Cmiss_field_module_id field_module,
	Cmiss_field_id source_field);

/**
 * Creates a field returning the values resulting from matrix multiplication
 * <source_field1> x <source_field2>, with <number_of_rows> rows in both
 * <source_field1> and the result. From the <number_of_rows> the columns in
 * <source_field1>, rows in <source_field2> and then columns in <source_field2>
 * are implied and checked.
 *
 * @param field_module  Region field module which will own new field.
 * @param number_of_rows  Number of rows N in source_field1 and result.
 * @param source_field1  N rows * M columns component matrix field 1.
 * @param source_field2  M rows * P columns component matrix field 2.
 * @return Newly created matrix with N*P components.
 */
Cmiss_field_id Cmiss_field_module_create_matrix_multiply(
	Cmiss_field_module_id field_module,
	int number_of_rows, Cmiss_field_id source_field1,
	Cmiss_field_id source_field2);

/**
 * Creates a field returning the transpose of N*M matrix source_field.
 * The source_number_of_rows is specified; source_number_of_columns is computed
 * as source_field->number_of_components / <source_number_of_rows>;
 * this division must have no remainder.
 *
 * @param field_module  Region field module which will own new field.
 * @param source_number_of_rows  Number of rows N in source_field.
 * @param source_field  N rows * M columns component matrix field.
 * @return Newly created M*N component transposed matrix field.
 */
Cmiss_field_id Cmiss_field_module_create_transpose(
	Cmiss_field_module_id field_module,
	int source_number_of_rows, Cmiss_field_id source_field);

#endif /* __CMISS_FIELD_MATRIX_OPERATIONS_H__ */
