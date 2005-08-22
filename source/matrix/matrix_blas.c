/*******************************************************************************
FILE : matrix_blas.c

LAST MODIFIED : 4 May 2003

DESCRIPTION :
Structures and functions for a basic vector operations.
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

#include <stdio.h>
#include "matrix/matrix.h"
#include "matrix/matrix_private.h"

/*
Module types
------------
*/

/*
Private functions
-----------------
*/

/*
Global functions
----------------
*/
int Matrix_set(struct Matrix *matrix,Matrix_value value)
/*******************************************************************************
LAST MODIFIED : 1 May 2003

DESCRIPTION :
Set the elements of <matrix> to <value>.
==============================================================================*/
{
  int return_code;
  int i,size;

  ENTER(Matrix_sum);
  if (matrix)
  {
    switch (matrix->matrix_type)
    {
      case DENSE:
      {
        size=(matrix->num_row)*(matrix->num_col);
        for (i=0;i<size;i++)
        {
          matrix->value[i]=value;
        }
        return_code=1;
      } break;

      case COMPRESSED_ROW:
      case SPARSE_POPULATE_FIRST:
      case SPARSE_POPULATE_LAST:
      case SPARSE_POPULATE_SUM:
      {
        display_message(ERROR_MESSAGE,"Matrix_set.  "
          "Not yet implimented for sparse matricies");
        return_code=0;
      } break;

      default:
      {
        display_message(ERROR_MESSAGE,"Matrix_set.  "
          "Unknown matrix type %d",matrix->matrix_type);
        return_code=0;
      } break;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Matrix_set.  "
      "Invalid argument.  %p",matrix);
    return_code=0;
  }
  LEAVE;

  return (return_code);
} /* Matrix_set */


#if defined (OLD_CODE)
/*???DB.  Can't fine Dsum in BLAS and not sure why we want to sum all the
	entries of a matrix together */
int Matrix_sum(struct Matrix *matrix,Matrix_value *sum_address)
/*******************************************************************************
LAST MODIFIED : 4 May 2003

DESCRIPTION :
Calculate the sum of the elements of <matrix>, returning the result in
<sum_address>.
==============================================================================*/
{
  int return_code;
  int size,one = 1;

  ENTER(Matrix_sum);
	return_code=0;
  if (matrix&&sum_address)
  {
    switch (matrix->matrix_type)
    {
      case DENSE:
      {
        size=(matrix->num_row)*(matrix->num_col);
        *sum_address=Dsum(&size,matrix->value,&one);
        return_code=1;
      } break;

      case COMPRESSED_ROW:
      {
        size=matrix->num_elem;
        *sum_address=Dsum(&size,matrix->value,&one);
        return_code=1;
      } break;

      /* Sparse populate matrix: not yet implimented */
      case SPARSE_POPULATE_FIRST:
      case SPARSE_POPULATE_LAST:
      case SPARSE_POPULATE_SUM:
      {
        display_message(ERROR_MESSAGE,"Matrix_sum.  "
          "Not yet implimented for sparse populate matricies");
        return_code=0;
      } break;

      default:
      {
        display_message(ERROR_MESSAGE,"Matrix_sum.  "
          "Unknown matrix type %d",matrix->matrix_type);
        return_code=0;
      } break;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Matrix_sum.  "
      "Invalid argument(s).  %p %p",matrix,sum_address);
    return_code=0;
  }
  LEAVE;

  return (return_code);
} /* Matrix_sum */
#endif /* defined (OLD_CODE) */


int Matrix_asum(struct Matrix *matrix,Matrix_value *asum_address)
/*******************************************************************************
LAST MODIFIED : 24 April 2003

DESCRIPTION :
Calculate the sum of the absolute value of the elements of <matrix>, returning
the result in <asum_address>.
==============================================================================*/
{
  int return_code;
  int size,one = 1;

  ENTER(Matrix_asum);
  if (matrix&&asum_address)
  {
    switch (matrix->matrix_type)
    {
      case DENSE:
      {
        size=(matrix->num_row)*(matrix->num_col);
        *asum_address=Dasum(&size,matrix->value,&one);
        return_code=1;
      } break;

      case COMPRESSED_ROW:
      {
        size=matrix->num_elem;
        *asum_address=Dasum(&size,matrix->value,&one);
        return_code=1;
      } break;

      /* Sparse populate matrix: not yet implimented */
      case SPARSE_POPULATE_FIRST:
      case SPARSE_POPULATE_LAST:
      case SPARSE_POPULATE_SUM:
      {
        display_message(ERROR_MESSAGE,"Matrix_asum.  "
          "Not yet implimented for sparse populate matricies");
        return_code=0;
      } break;

      default:
      {
        display_message(ERROR_MESSAGE,"Matrix_asum.  "
          "Unknown matrix type %d",matrix->matrix_type);
        return_code=0;
      } break;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Matrix_asum.  "
      "Invalid argument(s).  %p %p",matrix,asum_address);
    return_code=0;
  }
  LEAVE;

  return (return_code);
} /* Matrix_asum */


int Matrix_norm2(struct Matrix *matrix,Matrix_value *norm_address)
/*******************************************************************************
LAST MODIFIED : 24 April 2003

DESCRIPTION :
Calculate the second norm of <matrix>, returning the result in <norm_address>.
==============================================================================*/
{
  int return_code;
  int size,one = 1;

  ENTER(Matrix_norm2);
  if (matrix&&norm_address)
  {
    switch (matrix->matrix_type)
    {
      case DENSE:
      {
        size=(matrix->num_row)*(matrix->num_col);
        *norm_address=Dnrm2(&size,matrix->value,&one);
        return_code=1;
      } break;

      case COMPRESSED_ROW:
      {
        size=matrix->num_elem;
        *norm_address=Dnrm2(&size,matrix->value,&one);
        return_code=1;
      } break;

      /* Sparse populate matrix: not yet implimented */
      case SPARSE_POPULATE_FIRST:
      case SPARSE_POPULATE_LAST:
      case SPARSE_POPULATE_SUM:
      {
        display_message(ERROR_MESSAGE,"Matrix_norm2.  "
          "Not yet implimented for sparse populate matricies");
        return_code=0;
      } break;

      default:
      {
        display_message(ERROR_MESSAGE,"Matrix_norm2.  "
          "Unknown matrix type %d",matrix->matrix_type);
        return_code=0;
      } break;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Matrix_norm2.  "
      "Invalid argument(s).  %p %p",matrix,norm_address);
    return_code=0;
  }
  LEAVE;

  return (return_code);
} /* Matrix_norm2 */


int Matrix_dot(struct Matrix *matrix_1,struct Matrix *matrix_2,
  Matrix_value *dot_address)
/*******************************************************************************
LAST MODIFIED : 1 May 2003

DESCRIPTION :
Calculate the dot product of the one dimensional arrays (ie: vectors) 
<matrix_1> and <matrix_2>.
==============================================================================*/
{
  int return_code;
  int size_1,size_2,one = 1;

  ENTER(Matrix_dot);
  if (matrix_1&&matrix_2&&dot_address)
  {
    if ((matrix_1->matrix_type)==DENSE && (matrix_2->matrix_type)==DENSE)
    {
      size_1=(matrix_1->num_row)*(matrix_1->num_col);
      size_2=(matrix_2->num_row)*(matrix_2->num_col);

      if (size_1==size_2)
      {
        *dot_address=Ddot(&size_1,matrix_1->value,&one,matrix_2->value,&one);
        return_code=1;
      }
      else
      {
        display_message(ERROR_MESSAGE,"Matrix_dot.  "
          "Array sizes do not match");
        return_code=0;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"Matrix_dot.  "
        "Only implimented for DENSE matricies");
      return_code=0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Matrix_dot.  "
      "Invalid argument(s).  %p %p %p",matrix_1,matrix_2,dot_address);
    return_code=0;
  }
  LEAVE;

  return (return_code);
} /* Matrix_dot */


int Matrix_axpy(struct Matrix *matrix_x,struct Matrix *matrix_y,
  Matrix_value alpha)
/*******************************************************************************
LAST MODIFIED : 29 April 2003

DESCRIPTION :
Perform an AXPY operation, <alpha>*<x> + <y> upon <maxtrix_x> and <matrix_y>,
returning the result in matrix y.
==============================================================================*/
{
  int return_code;
  int size_x,size_y,one=1;

  ENTER(Matrix_saxpy);
  if (matrix_x&&matrix_y)
  {
    if ((matrix_x->matrix_type)==DENSE && (matrix_y->matrix_type)==DENSE)
    {
      size_x=(matrix_x->num_row)*(matrix_x->num_col);
      size_y=(matrix_y->num_row)*(matrix_y->num_col);

      if (size_x==size_y)
      {
        Daxpy(&size_x,&alpha,matrix_x->value,&one,matrix_y->value,&one);
        return_code=1;
      }
      else
      {
        display_message(ERROR_MESSAGE,"Matrix_axpy.  "
          "Array sizes do not match");
        return_code=0;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"Matrix_axpy.  "
        "Only implimented for DENSE matricies");
      return_code=0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Matrix_axpy.  "
      "Invalid argument(s).  %p %p",matrix_x,matrix_y);
    return_code=0;
  }
  LEAVE;

  return (return_code);
} /* Matrix_axpy */


int Matrix_matvec(struct Matrix *matrix_A,struct Matrix *matrix_x,
  struct Matrix *matrix_y,Matrix_value alpha,Matrix_value beta)
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
{
  int return_code;
  int m,n,lda,one=1,trans_len=1;
  int size_x,size_y;
  char *trans="N";

  ENTER(Matrix_matmul);
  if (matrix_A&&matrix_x&&matrix_y)
  {
    if (((matrix_x->matrix_type)==DENSE)&&((matrix_x->matrix_type)==DENSE)&&
        ((matrix_y->matrix_type)==DENSE || beta==0.0))
    {
      m=matrix_A->num_row;
      n=matrix_A->num_col;
      lda=m;
      size_x=(matrix_x->num_row)*(matrix_x->num_col);
      size_y=(matrix_y->num_row)*(matrix_y->num_col);

      if ((matrix_A->num_col==size_x)&&
          ((matrix_A->num_row==size_y && matrix_y->matrix_type==DENSE)||
           (beta==0.0 && Matrix_recreate(matrix_y,DENSE,m,1))))
      {
        Dgemv(trans,&m,&n,&alpha,matrix_A->value,&lda,matrix_x->value,&one,
              &beta,matrix_y->value,&one,trans_len);
        return_code=1;
      }
      else
      {
        display_message(ERROR_MESSAGE,"Matrix_matvec.  "
          "Array sizes do not match");
        return_code=0;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"Matrix_matvec.  "
        "Only implimented for DENSE matricies");
      return_code=0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Matrix_matvec.  "
      "Invalid argument(s).  %p %p %p",matrix_A,matrix_x,matrix_y);
    return_code=0;
  }
  LEAVE;

  return (return_code);
} /* Matrix_matvec */


int Matrix_matmul(struct Matrix *matrix_A,struct Matrix *matrix_B,
  struct Matrix *matrix_C,Matrix_value alpha,Matrix_value beta)
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
{
  int return_code,dimcheck_C;
  int m,n,k,lda,ldb,ldc,one=1;
  char *transA="N",*transB="N";

  ENTER(Matrix_matmul);
  if (matrix_A&&matrix_B&&matrix_C)
  {
    if (((matrix_A->matrix_type)==DENSE)&&((matrix_B->matrix_type)==DENSE)&&
        ((matrix_C->matrix_type)==DENSE || beta==0.0))
    {
      dimcheck_C=(matrix_A->num_row==matrix_C->num_row&&
                  matrix_B->num_col==matrix_C->num_col);
      m=matrix_A->num_row;
      n=matrix_B->num_col;
      k=matrix_A->num_col;
      lda=m;
      ldb=n;
      ldc=m;

      if ((matrix_A->num_col==matrix_B->num_row)&&
          ((matrix_C->matrix_type==DENSE && dimcheck_C)||
           (beta==0.0&&Matrix_recreate(matrix_C,DENSE,m,n))))
      {
        Dgemm(transA,transB,&m,&n,&k,&alpha,matrix_A->value,&lda,
              matrix_B->value,&ldb,&beta,matrix_C->value,&ldc,one,one);
        return_code=1;
      }
      else
      {
        display_message(ERROR_MESSAGE,"Matrix_matmul.  "
          "Array sizes do not match");
        return_code=0;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"Matrix_matmul.  "
        "Only implimented for DENSE matricies");
      return_code=0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Matrix_matmul.  "
      "Invalid argument(s).  %p %p %p",matrix_A,matrix_B,matrix_C);
    return_code=0;
  }
  LEAVE;

  return (return_code);
} /* Matrix_matmul */


