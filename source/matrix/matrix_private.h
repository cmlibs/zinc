/*******************************************************************************
FILE : matrix_private.h

LAST MODIFIED : 1 May 2003

DESCRIPTION :
Structures private to the matrix routines.
==============================================================================*/
#if !defined (MATRIX_PRIVATE_H)
#define MATRIX_PRIVATE_H

#include "matrix.h"

#define MATRIX_GROWTH_FACTOR 2


/*******************************************************************************
LAST MODIFIED : 23 April 2003

DESCRIPTION :
Interface to the Fortran BLAS and LAPACK routines. Names are lower case with
a trailing underscore, and variables are passed by reference.
==============================================================================*/
#define Dsum   dsum_
#define Dasum  dasum_
#define Dnrm2  dnrm2_
#define Ddot   ddot_
#define Dcopy  dcopy_
#define Daxpy  daxpy_
#define Dgemv  dgemv_
#define Dgemm  dgemm_

#define Dgetrf dgetrf_
#define Dgetrs dgetrs_

double Dsum(int *n,double *x,int *incx);
double Dasum(int *n,double *x,int *incx);
double Dnrm2(int *n,double *x,int *incx);
double Ddot(int *n,double *x,int *incx,double *y,int *incy);
void Dcopy(int *n,double *x,int *incx,double *y,int *incy);
void Daxpy(int *n,double *alpha,double *x,int *incx,double *y,int *incy);
void Dgemv(char *trans,int *m,int *n,double *alpha,double *a,int *lda,
  double *x,int *incx,double *beta,double *y,int *incy,int trans_len);
void Dgemm(char *transa,char *transb,int *m,int *n,int *k,double *alpha,
  double *a,int *lda,double *b,int *ldb, double *beta, double *c,int *ldc,
  int len_transa,int len_transb);

void Dgetrf(int *m,int *n,double *a,int *lda,int *pivot,int *stat);
void Dgetrs(char *trans,int *n,int *nrhs,double *a,int *lda,int *pivot,
  double *b,int *ldb,int *stat,unsigned int trans_len);

/*
Module types
------------
*/
struct Matrix
/*******************************************************************************
LAST MODIFIED : 17 March 2003

DESCRIPTION :
An object which contains a matrix.
==============================================================================*/
{
  enum Matrix_type matrix_type;     /* Matrix type */
  int              num_row;         /* Number of rows of matrix */
  int              num_col;         /* Number of columns of matrix */
  int              num_elem;        /* Number of elements in data vector */
  int              n_size;          /* Size of data vector */
  char             *name;           /* Symbolic name of array */
  int              *irow;           /* Row index */
  int              *icol;           /* Column index */
  Matrix_value     *value;          /* Data segment */
}; /* struct Matrix */


struct Factor
/*******************************************************************************
LAST MODIFIED : 23 April 2003

DESCRIPTION :
A generic bucket holding a factorised matrix.
==============================================================================*/
{
  enum Factor_type factor_type;     /* Factor type */
  void             *factor_data;    /* Factorisation data */
}; /* struct Factor */


struct Factor_Lapack_LU
/*******************************************************************************
LAST MODIFIED : 23 April 2003

DESCRIPTION :
An object holding the data for the LAPACK LU factorisation.
==============================================================================*/
{
  int              num_row;         /* Number of rows of matrix */
  int              num_col;         /* Number of columns of matrix */
  Matrix_value     *array;          /* Array containing L and U factors */
  int              *pivot;          /* Permutation index */
}; /* struct Factor */


#endif /* !defined (MATRIX_PRIVATE_H) */
