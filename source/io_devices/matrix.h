/*******************************************************************************
FILE : matrix.h

LAST MODIFIED : 02 April 1995

DESCRIPTION :
Contains routines for manipulating graphics (Gmatrices) matrices.

NOTE:
Be aware that if you change MATRIX_SIZE, you may have to change existing code
to conform (especially where matrix/vector operations are concerned).
Most uses of this module will be for either 3 or 4 element matrices.
==============================================================================*/
#if !defined (MATRIX_H)
#define MATRIX_H


/*
Module Constants
----------------
*/
#define GMATRIX_SIZE 3
#define GMATRIX_PRECISION double
/*
Structure Definitions
---------------------
*/
struct Gmatrix_struct
/*******************************************************************************
LAST MODIFIED : 2 December 1994

DESCRIPTION :
Definition of a Gmatrix
==============================================================================*/
{
	GMATRIX_PRECISION data[GMATRIX_SIZE][GMATRIX_SIZE];
};

/*
Type Definitions
----------------
*/
typedef struct Gmatrix_struct Gmatrix;

/*
Global Functions
----------------
*/
void matrix_print(Gmatrix *current);
/*******************************************************************************
LAST MODIFIED : 1 December 1994

DESCRIPTION :
Used for debugging to view a matrix.
==============================================================================*/

void matrix_I(Gmatrix *current);
/*******************************************************************************
LAST MODIFIED : 18 July 1994

DESCRIPTION :
Turns the matrix into the identity
==============================================================================*/

void matrix_inverse(Gmatrix *current,Gmatrix *inverse);
/*******************************************************************************
LAST MODIFIED : 2 December 1994

DESCRIPTION :
Returns the inverse of current in inverse.
==============================================================================*/

void matrix_copy(Gmatrix *dest,Gmatrix *source);
/*******************************************************************************
LAST MODIFIED : 21 July 1994

DESCRIPTION :
Copies source to destination.
==============================================================================*/

void matrix_copy_transpose(Gmatrix *dest,Gmatrix *source);
/*******************************************************************************
LAST MODIFIED : 2 December 1994

DESCRIPTION :
Copies the transpose of source to destination.
==============================================================================*/

void matrix_vector_unit(GMATRIX_PRECISION *vector);
/*******************************************************************************
LAST MODIFIED : 02 April 1995

DESCRIPTION :
Changes the vector to unit length.
==============================================================================*/

void matrix_premult(Gmatrix *current,Gmatrix *pre_matrix);
/*******************************************************************************
LAST MODIFIED : 18 July 1994

DESCRIPTION :
Premultiplies current by pre_matrix.
==============================================================================*/

void matrix_postmult(Gmatrix *current,Gmatrix *post_matrix);
/*******************************************************************************
LAST MODIFIED : 2 December 1994

DESCRIPTION :
Postmultiplies current by post_matrix.
==============================================================================*/

void matrix_premult_vector(GMATRIX_PRECISION *current,Gmatrix *pre_matrix);
/*******************************************************************************
LAST MODIFIED : 18 July 1994

DESCRIPTION :
Premultiplies current by pre_matrix.
==============================================================================*/

void matrix_postmult_vector(GMATRIX_PRECISION *current,Gmatrix *post_matrix);
/*******************************************************************************
LAST MODIFIED : 2 December 1994

DESCRIPTION :
Postmultiplies current by post_matrix.
==============================================================================*/

void matrix_mult(Gmatrix *pre_matrix,Gmatrix *post_matrix,Gmatrix *new_matrix);
/*******************************************************************************
LAST MODIFIED : 2 December 1994

DESCRIPTION :
Multiplies pre_matrix by post_matrix, and then returns the answer in new_matrix.
==============================================================================*/

void matrix_rotate(Gmatrix *current,double angle,char axis);
/*******************************************************************************
LAST MODIFIED : 18 July 1994

DESCRIPTION :
Rotates in a right hand sense about the axis.
==============================================================================*/
#endif
