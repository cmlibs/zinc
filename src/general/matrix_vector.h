/*******************************************************************************
FILE : matrix_vector.h

LAST MODIFIED : 15 March 2005

DESCRIPTION:
Code for performing vector calculations - normalize, dot product etc. -, and
matrix calculations - multiplication, LU decomposition etc. - on small matrices.

Data format:
* All vectors and matrices are to be passed as one-dimensional arrays of
	double precision reals. Index v[0] is the first value.
* When vectors are passed as parameters to subroutines they are preceded by
	the [int] size of the vector. Similarly, matrix parameters are preceded by
	num_rows, num_columns, or just size if a square matrix is expected.
	Furthermore, if a routine expects vectors or matrices of a certain dimension,
	then the size will be evident from its name, eg. normalize3().
* Values in rows of matrices are sequential in the array. The first value in
	the second row is therefore m[num_columns].

Request:
Bit by bit all bits of vector and matrix code should be put in here - and not
be scattered around the source code as it is now. Routines known to contain
vector or matrix routines that should be in here are listed in matrix_vector.c.
==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (MATRIX_VECTOR_H)
#define MATRIX_VECTOR_H

#include "general/value.h"
#include <cmath>

int identity_matrix4(double *a);
/*******************************************************************************
LAST MODIFIED : 16 November 2004

DESCRIPTION :
Set matrix <a> to the 4x4 identity.
==============================================================================*/

/**
 * Returns the cross product of the two 3-component vectors = a (x) b.
 * @param result  Array to receive 3 component result.
 */
template <typename T>
inline void cross_product3(const T *a, const T *b, T *result)
{
	result[0] = a[1]*b[2] - a[2]*b[1];
	result[1] = a[2]*b[0] - a[0]*b[2];
	result[2] = a[0]*b[1] - a[1]*b[0];
}

/** Returns the dot product of the two 3-component vectors = a.b */
template <typename T>
inline T dot_product3(T *a, T *b)
{
	return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}

/**
 * Returns the norm/length/magnitude of 3-component vector <v>.
 */
template <typename T>
inline T norm3(const T *v)
{
	return sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}

/** Normalizes vector v - of dimension 3 - and returns its length.
 * Zero vectors are returned unchanged.
 */
template <typename T>
inline T normalize3(T *v)
{
	const T length = norm3(v);
	if (length > 0.0)
	{
		v[0] /= length;
		v[1] /= length;
		v[2] /= length;
	}
	return (length);
}

double scalar_triple_product3(double *a,double *b,double *c);
/*******************************************************************************
LAST MODIFIED : 28 January 1998

DESCRIPTION :
Returns the scalar triple product, [abc]=a.(bxc) of vectors a,b and c.
==============================================================================*/

int copy_matrix(int m,int n,double *a,double *a_copy);
/*******************************************************************************
LAST MODIFIED : 28 January 1998

DESCRIPTION :
Copies the contents of m rows x n columns matrix <a> to <a_copy>.
==============================================================================*/

int identity_matrix(int n, double *a);
/*******************************************************************************
LAST MODIFIED : 6 April 2001

DESCRIPTION :
Set matrix <a> to the <n> x <n> identity.
==============================================================================*/

int identity_matrix_FE_value(int n, FE_value *a);
/*******************************************************************************
LAST MODIFIED : 6 April 2001

DESCRIPTION :
Set matrix <a> to the <n> x <n> identity.
==============================================================================*/

int identity_matrix_float(int n, float *a);
/*******************************************************************************
LAST MODIFIED : 15 November 2001

DESCRIPTION :
Set matrix <a> to the <n> x <n> identity.
==============================================================================*/

/** Perform matrix multiplication result = a*b.
 * @param rowCount  Number of rows in a and result.
 * @param midCount  Number of columns in a and rows in b which are eliminated in dot product.
 * @param colCount  Number of columns in b and result.
 * @param a. First matrix to multiply.
 * @param b. Second matrix to multiply.
 * @param result.  Matrix to fill with result.
 */
template <typename T>
inline void matrixmult(int rowCount, int midCount, int colCount, const T *a, const T *b, T *result)
{
	const T *va = a;
	T *dest = result;
	for (int i = 0; i < rowCount; ++i)
	{
		for (int j = 0; j < colCount; ++j)
		{
			T sum = 0.0;
			for (int k = 0; k < midCount; ++k)
				sum += va[k]*b[k*colCount + j];
			*dest = sum;
			++dest;
		}
		va += midCount;
	}
}

int multiply_matrix(int m,int s,int n,double *a,double *b,double *c);
/*******************************************************************************
LAST MODIFIED : 28 January 1998

DESCRIPTION :
Returns in the m rows x n columns matrix <c> that is the product of
m x s matrix <a> and s x n matrix <b>.
==============================================================================*/

int print_matrix(int m,int n,double *a,char *number_format);
/*******************************************************************************
LAST MODIFIED : 28 January 1998

DESCRIPTION :
Prints out the contents of m rows x n columns matrix <a>.
==============================================================================*/

int transpose_matrix(int m,int n,double *a,double *a_transpose);
/*******************************************************************************
LAST MODIFIED : 28 January 1998

DESCRIPTION :
Return the transpose of m rows x n columns matrix <a> in <a_transpose> - which
will then be n rows x m columns.
==============================================================================*/

int matrix_is_symmetric(int n, double *a, double factor);
/*******************************************************************************
LAST MODIFIED : 7 November 2000

DESCRIPTION :
Returns true if n x n matrix <a> is symmetric about the main diagonal by
checking that the difference between all opposing off-diagonal values is less
<factor> times the largest [absolute] value in <a>.
==============================================================================*/

int LU_decompose(int n, double *a, int *indx, double *d,
	double singular_tolerance);
/*******************************************************************************
LAST MODIFIED : 15 March 2005

DESCRIPTION :
Performs LU decomposition on n x n matrix <a>. <indx> should be a preallocated
integer array of dimension n. On return, <a> contains the lower and upper
decomposed matrix such that a = LU. The indx array contains information about
the partial pivoting performed during decomposition, while the sign of *d
indicates whether an even =+ve, or odd =-ve number of row interchanges have been
performed - I think ;-).
<singular_tolerance> is compared to the magnitude of each pivot.  If any of the
pivot values are less than this parameter the return code will be zero.  The
decomposition is still completed
Adapted from "Numerical Recipes in C".
==============================================================================*/

int LU_backsubstitute(int n,double *a,int *indx,double *b);
/*******************************************************************************
LAST MODIFIED : 28 January 1998

DESCRIPTION :
Partner routine for LU_decompose which takes the n x n LU matrix (a) and indx
vector returned by LU_decompose and solves for x, where LU.x=b. The solution
vector x is returned in the former right-hand-side vector <b>.
Adapted from "Numerical Recipes in C".
==============================================================================*/

int Jacobi_eigenanalysis(int n, double *a, double *d, double *v, int *nrot);
/*******************************************************************************
LAST MODIFIED : 5 April 2002

DESCRIPTION :
Computes all eigenvalues and eigenvectors of real symmetric n x n matrix <a>.
The eigenvalues of <a> are returned in <d>, the columns of v contain the
normalised eigenvectors. <nrot> returns the number of jacobi rotations performed
by the algorithm. The returned eigenvalues/vectors are NOT sorted; use the
eigensort function if required.
The elements of <a> above the diagonal are destroyed by this function.
Adapted from "Numerical Recipes in C".
==============================================================================*/

int eigensort(int n, double *d, double *v);
/*******************************************************************************
LAST MODIFIED : 5 April 2002

DESCRIPTION :
Sorts the n eigenvalues in <d> and corresponding eigenvectors in the columns of
nxn matrix <v> from largest absolute value to smallest.
Adapted from "Numerical Recipes".
==============================================================================*/

/**
 * Calculates the inverse of 2x2 matrix <a>, returning it in <a_inv>.
 * Caller must ensure a and a_inv are valid.
 * @return  True on success, false on failure = singular matrix.
 */
template <typename T>
bool invert_matrix2(const T *a, T *a_inv)
{
	T maxValue = 0.0;
	for (int i = 0; i < 4; i++)
	{
		const T aAbs = fabs(a[i]);
		if (aAbs > maxValue)
			maxValue = aAbs;
	}
	const T determinant = a[0]*a[3] - a[1]*a[2];
	if ((maxValue <= 0.0) || (fabs(determinant) < 1.0E-6*maxValue))
		return false;
	const T one__determinant = 1.0 / determinant;
	a_inv[0] = a[3]*one__determinant;
	a_inv[1] = -a[1]*one__determinant;
	a_inv[2] = -a[2]*one__determinant;
	a_inv[3] = a[0]*one__determinant;
	return true;
}

/**
 * Calculates the inverse of 3x3 matrix <a>, returning it in <a_inv>.
 * Caller must ensure a and a_inv are valid.
 * @return  True on success, false on failure = singular matrix.
 */
template <typename T>
bool invert_matrix3(const T *a, T *a_inv)
{
	T maxValue = 0.0;
	for (int i = 0; i < 9; i++)
	{
		const T aAbs = fabs(a[i]);
		if (aAbs > maxValue)
			maxValue = aAbs;
	}
	const T determinant =
		a[0]*(a[4]*a[8] - a[5]*a[7]) +
		a[1]*(a[5]*a[6] - a[3]*a[8]) +
		a[2]*(a[3]*a[7] - a[4]*a[6]);
	if ((maxValue <= 0.0) || (fabs(determinant) < 1.0E-6*maxValue))
		return false;
	const T one__determinant = 1.0 / determinant;
	a_inv[0] = (a[4]*a[8] - a[5]*a[7])*one__determinant;
	a_inv[1] = (a[7]*a[2] - a[8]*a[1])*one__determinant;
	a_inv[2] = (a[1]*a[5] - a[2]*a[4])*one__determinant;
	a_inv[3] = (a[5]*a[6] - a[3]*a[8])*one__determinant;
	a_inv[4] = (a[8]*a[0] - a[6]*a[2])*one__determinant;
	a_inv[5] = (a[2]*a[3] - a[0]*a[5])*one__determinant;
	a_inv[6] = (a[3]*a[7] - a[4]*a[6])*one__determinant;
	a_inv[7] = (a[6]*a[1] - a[7]*a[0])*one__determinant;
	a_inv[8] = (a[0]*a[4] - a[1]*a[3])*one__determinant;
	return true;
}

/** Calculates and returns in <result> the matrix product of 3x3 matrices
 * <a> and <b>. Client must ensure a, b and result are valid. */
template <typename T> void multiply_matrix3(T *a, T *b, T *result)
{
	result[0] = a[0]*b[0] + a[1]*b[3] + a[2]*b[6];
	result[1] = a[0]*b[1] + a[1]*b[4] + a[2]*b[7];
	result[2] = a[0]*b[2] + a[1]*b[5] + a[2]*b[8];

	result[3] = a[3]*b[0] + a[4]*b[3] + a[5]*b[6];
	result[4] = a[3]*b[1] + a[4]*b[4] + a[5]*b[7];
	result[5] = a[3]*b[2] + a[4]*b[5] + a[5]*b[8];

	result[6] = a[6]*b[0] + a[7]*b[3] + a[8]*b[6];
	result[7] = a[6]*b[1] + a[7]*b[4] + a[8]*b[7];
	result[8] = a[6]*b[2] + a[7]*b[5] + a[8]*b[8];
}

/**
 * Calculates and returns in <result> the vector cross product of 2 3-component
 * FE_value vectors <a> and <b>.
 */
int cross_product_FE_value_vector3(const FE_value *a, const FE_value *b, FE_value *result);

/**
 * Calculates and returns in <result> the vector cross product of 3 4-component
 * FE_value vectors <a>, <b> and <c>.
 */
int cross_product_FE_value_vector4(const FE_value *a, const FE_value *b, const FE_value *c, FE_value *result);

/**
 * @return  Whether value is a factor of size i.e. that a product of value and
 * another natural number gives size.
 */
inline bool check_factor(int size, int value)
{
	return (1 <= value) && (value <= size) && (0 == (size % value));
}

#endif /* !defined (MATRIX_VECTOR_H) */
