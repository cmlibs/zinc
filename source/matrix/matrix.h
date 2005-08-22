/*******************************************************************************
FILE : matrix.h

LAST MODIFIED : 4 May 2003

DESCRIPTION :
Structures and functions for a basic matrix structure.
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
#if !defined (MATRIX_H)
#define MATRIX_H

#include <stdio.h>
#include "general/debug.h"
#include "general/object.h"
#include "user_interface/message.h"

/*
Global types
------------
*/
typedef double Matrix_value;

struct Matrix;
/*******************************************************************************
LAST MODIFIED : 17 March 2003

DESCRIPTION :
An object which contains a matrix.
==============================================================================*/

enum Matrix_type
/*******************************************************************************
LAST MODIFIED : 17 March 2003

DESCRIPTION :
Names of matrix storage schemes.
??? SN. Should we have symmetric types as well? Maybe as a separate flag ??
==============================================================================*/
{
  DENSE=1,
  COMPRESSED_ROW=2,
  SPARSE_POPULATE_FIRST=3,
  SPARSE_POPULATE_LAST=4,
  SPARSE_POPULATE_SUM=5,
  SPARSE_POPULATE=5
}; /* enum Matrix_type */

struct Factor;
/*******************************************************************************
LAST MODIFIED : 23 April 2003

DESCRIPTION :
A generic bucket holding a factorised matrix.
==============================================================================*/

enum Factor_type
/*******************************************************************************
LAST MODIFIED : 23 April 2003

DESCRIPTION :
Names of matrix factorisation schemes.
==============================================================================*/
{
  NONE=0,
  LAPACK_LU=1,
  LAPACK_LSQ=2,
  LAPACK_SVD=3,
  LAPACK_CHOLESKY=4,
  SUPERLU=11,
  UMFPACK=12,
  HARWELL=13,
  ITER_POINT_JACOBI=21,
  ITER_JACOBI=22,
  ITER_INCOMPLETE_CHOLESKY=23,
  ITER_INCOMPLETE_CHOLESKY_1=24,
  ITER_INCOMPLETE_LDL=25,
  ITER_INCOMPLETE_LDL_1=26,
  ITER_INCOMPLETE_LU=27,
  ITER_INCOMPLETE_LU_1=28
}; /* enum Matrix_type */


/*
Global functions
----------------
*/
struct Matrix *CREATE(Matrix)(char *name,enum Matrix_type matrix_type,
  int m,int n);
/*******************************************************************************
LAST MODIFIED : 17 March 2003

DESCRIPTION :
Creates a matrix object of <matrix_type> with the specified dimensions <m>x<n>
and symbolic <name>.

???SN.  So do we want <name>?
==============================================================================*/

int Matrix_recreate(struct Matrix *matrix,enum Matrix_type matrix_type,
  int num_row,int num_col);
/*******************************************************************************
LAST MODIFIED : 1 May 2003

DESCRIPTION :
Recreates a matrix object of <matrix_type> with the specified dimensions
<num_row>x<num_col>, throwing away the data already contained in it.
==============================================================================*/

int DESTROY(Matrix)(struct Matrix **matrix_address);
/*******************************************************************************
LAST MODIFIED : 17 March 2003

DESCRIPTION :
Free memory/deaccess objects in matrix at <*matrix_address>.
==============================================================================*/

int Matrix_set_value(struct Matrix *matrix,int row,int column,
  Matrix_value value);
/*******************************************************************************
LAST MODIFIED : 17 March 2003

DESCRIPTION :
Sets the <value> at the (<row>,<column>) location of <matrix>.
==============================================================================*/

int Matrix_get_value(struct Matrix *matrix,int row,int column,
  Matrix_value *value_address);
/*******************************************************************************
LAST MODIFIED : 17 March 2003

DESCRIPTION :
Gets the value at the (<row>,<column>) location of <matrix> returning it in 
<*value_address>.
==============================================================================*/

int Matrix_set_values(struct Matrix *matrix,Matrix_value *values,int row_low,
  int row_high,int column_low,int column_high);
/*******************************************************************************
LAST MODIFIED : 2 May 2003

DESCRIPTION :
Sets the block of values within <matrix> located in the range 
<row_low:row_high><column_low:column_high>.
Note that <values> has Fortran ordering (ie: the row index increasing fastest).
==============================================================================*/

int Matrix_get_values(struct Matrix *matrix,Matrix_value *values,int row_low,
  int row_high,int column_low,int column_high);
/*******************************************************************************
LAST MODIFIED : 2 May 2003

DESCRIPTION :
Get the block of values within <matrix> located in the range 
<row_low:row_high><column_low:column_high>.
Note that <values> has Fortran ordering (ie: the row index increasing fastest).
==============================================================================*/

int Matrix_set_type(struct Matrix *matrix,enum Matrix_type type);
/*******************************************************************************
LAST MODIFIED : 17 March 2003

DESCRIPTION :
Set the storage <type> of <matrix>.

A matrix can be created as SPARSE_POPULATE and when the sparsity pattern has
been defined using Matrix_set_value its type can be changed to SPARSE for
computation.
==============================================================================*/

int Matrix_get_type(struct Matrix *matrix,enum Matrix_type *type_address);
/*******************************************************************************
LAST MODIFIED : 17 March 2003

DESCRIPTION :
Gets the storage type of <matrix>, returning it in <*type_address>.
==============================================================================*/

int Matrix_set_dimensions(struct Matrix *matrix,int num_row, int num_col);
/*******************************************************************************
LAST MODIFIED : 17 March 2003

DESCRIPTION :
Sets the dimensions of <matrix> to <num_row> x <num_col>.
==============================================================================*/

int Matrix_get_dimensions(struct Matrix *matrix,int *num_row_address,
  int *num_col_address);
/*******************************************************************************
LAST MODIFIED : 17 March 2003

DESCRIPTION :
Gets the dimensions of <matrix>, returning them in <*num_row_address> and
<*num_col_address>.
==============================================================================*/

int Matrix_copy(struct Matrix *matrix_1,struct Matrix *matrix_2);
/*******************************************************************************
LAST MODIFIED : 17 March 2003

DESCRIPTION :
Copies the contents of <matrix_1> into <matrix_2>.
==============================================================================*/

int Matrix_set(struct Matrix *matrix,Matrix_value value);
/*******************************************************************************
LAST MODIFIED : 1 May 2003

DESCRIPTION :
Set the elements of <matrix> to <value>.
==============================================================================*/

int Matrix_sum(struct Matrix *matrix,Matrix_value *sum_address);
/*******************************************************************************
LAST MODIFIED : 24 April 2003

DESCRIPTION :
Calculate the sum of the elements of <matrix>, returning the result in
<sum_address>.
==============================================================================*/

int Matrix_asum(struct Matrix *matrix,Matrix_value *asum_address);
/*******************************************************************************
LAST MODIFIED : 24 April 2003

DESCRIPTION :
Calculate the sum of the absolute value of the elements of <matrix>, returning
the result in <asum_address>.
==============================================================================*/

int Matrix_norm2(struct Matrix *matrix,Matrix_value *norm_address);
/*******************************************************************************
LAST MODIFIED : 24 April 2003

DESCRIPTION :
Calculate the second norm of <matrix>, returning the result in <norm_address>.
==============================================================================*/

int Matrix_dot(struct Matrix *matrix_1,struct Matrix *matrix_2,
  Matrix_value *dot_address);
/*******************************************************************************
LAST MODIFIED : 1 May 2003

DESCRIPTION :
Calculate the dot product of the one dimensional arrays (ie: vectors) 
<matrix_1> and <matrix_2>.
==============================================================================*/

int Matrix_axpy(struct Matrix *matrix_x,struct Matrix *matrix_y,
  Matrix_value alpha);
/*******************************************************************************
LAST MODIFIED : 29 April 2003

DESCRIPTION :
Perform an AXPY operation, <alpha>*<x> + <y> upon <maxtrix_x> and <matrix_y>,
returning the result in matrix y.
==============================================================================*/

int Matrix_matvec(struct Matrix *matrix_A,struct Matrix *matrix_x,
  struct Matrix *matrix_y,Matrix_value alpha,Matrix_value beta);
/*******************************************************************************
LAST MODIFIED : 1 May 2003

DESCRIPTION :
Perform a matrix vector multipication, y <- alpha.A.x + beta.y, where <A> is
a matrix, and <x> and <y> are one dimensional vectors, and <alpha> and <beta>
are scalars.

The number of columns of <A> must equal the length of <x>.

If <beta> is equal to 0.0 then <y> is cleared and set to the correct size. 
Otherwise the length of <y> must equal the number of rows of <A>.
==============================================================================*/

int Matrix_matmul(struct Matrix *matrix_A,struct Matrix *matrix_B,
  struct Matrix *matrix_C,Matrix_value alpha,Matrix_value beta);
/*******************************************************************************
LAST MODIFIED : 1 May 2003

DESCRIPTION :
Perform a matrix multipication, C <- alpha.A.B + beta.C, where <A>, <B> and <C>
are matricies, and <alpha> and <beta> are scalars.

The number of columns of <A> must equal the number of rows of <B>.

If <beta> is equal to 0.0 then <C> is cleared and set to the correct size. 
Otherwise the number of rows and columns of <C> must equal the number of
columns of <A> and the number of rows of <B>.
==============================================================================*/



#endif /* !defined (MATRIX_H) */
