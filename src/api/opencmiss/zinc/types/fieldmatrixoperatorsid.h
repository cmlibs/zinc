/**
 * @file fieldatrixoperatorsid.h
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMZN_FIELDMATRIXOPERATORSID_H__
#define CMZN_FIELDMATRIXOPERATORSID_H__

/**
 * @brief  A field calculating the eigenvalues.
 *
 * A field returning the N eigenvalues of symmetric N*N component source
 * field.
 */
struct cmzn_field_eigenvalues;
typedef struct cmzn_field_eigenvalues *cmzn_field_eigenvalues_id;

/**
 * @brief  A field performing matrix multiplication.
 *
 * A field performing matrix multiplication on two source fields, with the
 * matrix sizes specified by first operand number of rows, also applying to
 * the result.
 */
struct cmzn_field_matrix_multiply;
typedef struct cmzn_field_matrix_multiply *cmzn_field_matrix_multiply_id;

/**
 * @brief  A field calculating the transpose of a matrix.
 *
 * A field giving the transpose of a source field representing a matrix with a
 * specified number of rows.
 */
struct cmzn_field_transpose;
typedef struct cmzn_field_transpose *cmzn_field_transpose_id;

#endif
