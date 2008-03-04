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

#include "io_devices/conversion.h"

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

void matrix_euler(Gmatrix *direction,struct Dof3_data *euler);
/*******************************************************************************
LAST MODIFIED : 3 March 2008

DESCRIPTION :
***Moved from coord_trans.h to matrix.h so it can be used by non-motif cmgui.
Takes a direction cosine matrix and returns the equivalent euler angles in degrees.
Note that when the x axis is aligned with the z axis, then the distribution
between azimuth and roll is arbitrary, so we will say that it is solely made
up of roll.  Inverse formulae are taken from the Polhemus manual, page 156.
==============================================================================*/

void euler_matrix(struct Dof3_data *euler,Gmatrix *direction);
/*******************************************************************************
LAST MODIFIED : 3 March 2008

DESCRIPTION :
***Moved from coord_trans.h to matrix.h so it can be used by non-motif cmgui.
Returns the equivalent direction cosine matrix of the passed euler values.
Formulae are taken from the Polhemus manual, page 156.
==============================================================================*/
#endif
