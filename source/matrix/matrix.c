/*******************************************************************************
FILE : matrix.c

LAST MODIFIED : 20 July 2003

DESCRIPTION :
Structures and functions for a basic matrix structure. Contains a matrix struct
that can contain Dense, Compressed Row, and Sparse Populate matricies.
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

#include <stdlib.h>
#include <stdio.h>
#include "matrix/matrix.h"
#include "matrix/matrix_private.h"

/*
Module types
------------
*/


/*
Private data
------------
*/
static int *global_irow=NULL;
static int *global_icol=NULL;
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Global row and column vectors, used by the sorting function called by
Matrix_convert_sparse_populate_to_compressed_row.
==============================================================================*/

/*
Private functions
-----------------
*/
static void Matrix_reset_matrix(struct Matrix *matrix)
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Clear the entries in <matrix>, deallocate memory and set pointers to NULL.
Used when a reallocate fails leaving the existing entries in <matrix> in an 
uncertain but probibly trashed state.
==============================================================================*/
{
  ENTER(Matrix_clear_matrix);
  DEALLOCATE(matrix->value);
  DEALLOCATE(matrix->icol);
  DEALLOCATE(matrix->irow);
  matrix->icol=NULL;
  matrix->irow=NULL;
  matrix->value=NULL;
  matrix->num_row=0;
  matrix->num_col=0;
  matrix->n_size=0;
  matrix->num_elem=0;
  matrix->matrix_type=DENSE;
  LEAVE;
} /* Matrix_clear_matrix */

#if defined (OLD_CODE)
static int Matrix_find_index_linear(int *index,int ilow,int ihigh,
  int index_target,int *index_address)
/*******************************************************************************
LAST MODIFIED : 4 April 2003

DESCRIPTION :
Find the entry in <index> (within the range <ilow> <= i < <ihigh>) having the
value <index_target>, returning the position in <index_address>. Uses a linear
search, and if no element is found the element of the next highest element is
returned.
==============================================================================*/
{
  int i,return_code;

  ENTER(Matrix_find_index_linear);
  for (i=ilow;i<ihigh;i++)
  {
    /* If we have found the entry */
    if (index[i]>=index_target)
    {
      *index_address=i;
      if (index[i]==index_target)
      {
        return_code=1;
      }
      else
      {
        return_code=0;
      }
      goto end_loop; /* Jump out of loop. */
    }
  }
  /* Entry not found */
  *index_address=ihigh;
  return_code=0;

 end_loop:;
  LEAVE;

  return (return_code);
} /* Matrix_find_index_linear */
#endif /* defined (OLD_CODE) */


static int Matrix_find_index_bisection(int *index,int ilow,int ihigh,
  int index_target,int *index_address)
/*******************************************************************************
LAST MODIFIED : 4 April 2003

DESCRIPTION :
Find the entry in <index> (within the range <ilow> <= i < <ihigh>) having the
value <index_target>, returning the position in <index_address>. Uses a 
bisection search, and if no element is found the element of the next highest
element is returned.
==============================================================================*/
{
  int j_lower,j_mid,j_upper,return_code;

  ENTER(Matrix_find_index_bisection);
  j_lower=ilow-1;
  j_upper=ihigh;
  while ((j_upper-j_lower)>1)
  {
    j_mid=(j_lower+j_upper) >> 1; /* I love C's integer arithmetic */
    if (index[j_mid]<index_target)
    {
      j_lower=j_mid;
    }
    else
    {
      j_upper=j_mid;
    }
  }
  *index_address=j_upper;

  if ((j_upper<ihigh)&&(index[j_upper]==index_target))
  {
    return_code=1;
  }
  else
  {
    return_code=0;
  }
  LEAVE;

  return (return_code);
} /* Matrix_find_index_bisection */


static int Matrix_convert_compressed_row_to_dense(struct Matrix *matrix)
/*******************************************************************************
LAST MODIFIED : 8 April 2003

DESCRIPTION :
Convert a compressed row matrix to a dense matrix.
==============================================================================*/
{
  int i,ij_sparse,ij_dense,num_elem,return_code;
  Matrix_value *value;

  ENTER(Matrix_convert_compressed_row_to_dense);
  num_elem=(matrix->num_row)*(matrix->num_col);
  if (ALLOCATE(value,Matrix_value,num_elem))
  {
    for (i=0;i<num_elem;i++)
    {
      value[i]=0.0;
    }
    for (i=0;i<(matrix->num_row);i++)
    {
      for (ij_sparse=(matrix->irow[i]);
           ij_sparse<(matrix->irow[i+1]);
           ij_sparse++)
      {
        ij_dense=i+(matrix->icol[ij_sparse])*(matrix->num_row);
        value[ij_dense]=(matrix->value[ij_sparse]);
      }
    }
    DEALLOCATE(matrix->value);
    DEALLOCATE(matrix->irow);
    DEALLOCATE(matrix->icol);
    matrix->value=value;
    matrix->irow=NULL;
    matrix->icol=NULL;
    matrix->n_size=num_elem;
    matrix->num_elem=0;
    matrix->matrix_type=DENSE;

    return_code=0;
  }
  else
  {
    display_message(ERROR_MESSAGE,"Matrix_convert_compressed_row_to_dense.  "
      "Could not allocate memory for dense matrix");
    return_code=0;
  }
  LEAVE;

  return (return_code);
} /* Matrix_convert_compressed_row_to_dense */


static int Matrix_convert_compressed_row_to_sparse_populate(
  struct Matrix *matrix)
/*******************************************************************************
LAST MODIFIED : 8 April 2003

DESCRIPTION :
Convert a compressed row matrix to a sparse populate matrix.
==============================================================================*/
{
  int i,ij,ij_sparse,num_elem,return_code;
  int *irow;

  ENTER(Matrix_convert_compressed_row_to_sparse_populate);
  num_elem=matrix->n_size;
  /* Allocate memory, and set irow[] values */
  if (ALLOCATE(irow,int,num_elem))
  {
    ij_sparse=0;
    for (i=0;i<(matrix->num_row);i++)
    {
      for (ij=matrix->irow[i];ij<matrix->irow[i+1];ij++)
      {
        irow[ij_sparse]=i+1;
        ij_sparse++;
      }
    }

    /* Convert the icol values -- for SPARSE_POPULATE they are 1 based
       but for COMPRESSED_ROW the are 0 based */
    for (i=0;i<num_elem;i++)
    {
      matrix->icol[i]++;
    }

    /* Deallocate old storage, and shift new pointers */
    DEALLOCATE(matrix->irow);
    matrix->irow=irow;
    matrix->num_elem=num_elem;
    matrix->matrix_type=SPARSE_POPULATE;

    return_code=1;
  }
  else
  {
    display_message(ERROR_MESSAGE,
      "Matrix_convert_compressed_row_to_sparse_populate.  "
      "Could not allocate memory for sparse matrix");
    return_code=0;
  }
  LEAVE;

  return (return_code);
} /* Matrix_convert_compressed_row_to_sparse_populate */


static int Matrix_convert_sparse_populate_to_dense(struct Matrix *matrix)
/*******************************************************************************
LAST MODIFIED : 8 April 2003

DESCRIPTION :
Convert a sparse populate matrix to a dense matrix.
==============================================================================*/
{
  int i,ij,num_elem,return_code;
  Matrix_value *value;

  ENTER(Matrix_convert_sparse_populate_to_dense);
  num_elem=(matrix->num_row)*(matrix->num_col);
  if (ALLOCATE(value,Matrix_value,num_elem))
  {
    for (i=0;i<num_elem;i++)
    {
      value[i]=0.0;
    }

    switch (matrix->matrix_type)
    {
      case SPARSE_POPULATE_FIRST:
      {
        for (i=(matrix->num_elem)-1;i<=0;i--)
        {
          ij=(matrix->irow[i]-1)+(matrix->icol[i]-1)*(matrix->num_row);
          value[ij]=matrix->value[i];
        }
      } break;

      case SPARSE_POPULATE_LAST:
      {
        for (i=0;i<(matrix->num_elem);i++)
        {
          ij=(matrix->irow[i]-1)+(matrix->icol[i]-1)*(matrix->num_row);
          value[ij]=matrix->value[i];
        }
      } break;

      case SPARSE_POPULATE_SUM:
      {
        for (i=0;i<(matrix->num_elem);i++)
        {
          ij=(matrix->irow[i]-1)+(matrix->icol[i]-1)*(matrix->num_row);
          value[ij]+=matrix->value[i];
        }
      } break;
    }

    DEALLOCATE(matrix->value);
    DEALLOCATE(matrix->irow);
    DEALLOCATE(matrix->icol);
    matrix->value=value;
    matrix->irow=NULL;
    matrix->icol=NULL;
    matrix->n_size=num_elem;
    matrix->num_elem=0;
    matrix->matrix_type=DENSE;

    return_code=0;
  }
  else
  {
    display_message(ERROR_MESSAGE,"Matrix_convert_sparse_populate_to_dense.  "
      "Could not allocate memory for dense matrix");
    return_code=0;
  }
  LEAVE;

  return (return_code);
} /* Matrix_convert_sparse_populate_to_dense */


static int Matrix_sort_sparse_populate_compar(const void *a,const void *b)
/*******************************************************************************
LAST MODIFIED : 8 April 2003

DESCRIPTION :
Cimparison function used by qsort() when ordering the sparse data. data is 
sorted in ascending row order, and in ascending column within each row. For
elements having the same location within the array the initial order is kept,
to ensure that the SPARSE_POPULATE_FIRST and _LAST types work.
==============================================================================*/
{
  int *ia,*ib,return_code;

  ia=(int *)a;
  ib=(int *)b;

  /* Sort by row */
  if (global_irow[*ia]<global_irow[*ib])
  {
    return_code=-1;
  }
  else if (global_irow[*ia]>global_irow[*ib])
  {
    return_code=1;
  }

  /* Sort by column */
  else if (global_icol[*ia]<global_icol[*ib])
  {
    return_code=-1;
  }
  else if (global_icol[*ia]>global_icol[*ib])
  {
    return_code=1;
  }

  /* Sort by index */
  else if ((*ia)<(*ib))
  {
    return_code=-1;
  }
  else
  {
    return_code=1;
  }

  return (return_code);
}


static int Matrix_convert_sparse_populate_to_compressed_row(
  struct Matrix *matrix)
/*******************************************************************************
LAST MODIFIED : 8 April 2003

DESCRIPTION :
Convert a sparse populate matrix to a compressed row matrix.
==============================================================================*/
{
  int i,irow_current,ij_old,ij_new,num_elem,return_code;
  int *irow_new,*icol_new,*irow_old,*icol_old,*index;
  Matrix_value *value_new,*value_old;

  ENTER(Matrix_convert_sparse_populate_to_compressed_row);
  num_elem=(matrix->num_elem);
  if ((num_elem>0)&&
      (ALLOCATE(index,int,num_elem))&&
      (ALLOCATE(irow_new,int,(matrix->num_row)+1))&&
      (ALLOCATE(icol_new,int,num_elem))&&
      (ALLOCATE(value_new,Matrix_value,num_elem)))
  {
    irow_old=matrix->irow;
    icol_old=matrix->icol;
    value_old=matrix->value;

    /* Sort the index -- keep track of the irow and icol data with
       the global pointers. Ugly but OK for first implimentation.

       I would imagine that by first splitting the data up into each row,
       and then sorting each row separatly, the sorting might be performed
       more efficently */
    for (i=0;i<num_elem;i++)
    {
      index[i]=i;
    }
    global_irow=matrix->irow;
    global_icol=matrix->icol;
    qsort(index,num_elem,sizeof(int),Matrix_sort_sparse_populate_compar);
    global_irow=NULL;
    global_icol=NULL;

    /* Load the new row/column/value vectors */
    irow_new[0]=0;
    irow_current=1;
    ij_new=-1;
    for (ij_old=0;ij_old<num_elem;ij_old++)
    {
      /* New row */
      if (irow_old[index[ij_old]]!=irow_current)
      {
        ij_new++;
        for (i=irow_current;i<irow_old[index[ij_old]];i++)
        {
          irow_new[i]=ij_new;
        }
        icol_new[ij_new]=icol_old[index[ij_old]]-1;
        value_new[ij_new]=value_old[index[ij_old]];
        irow_current=irow_new[i]+1;
      }
      /* New column */
      else if (icol_old[index[ij_old]]!=icol_new[ij_new]+1)
      {
        ij_new++;
        icol_new[ij_new]=icol_old[index[ij_old]]-1;
        value_new[ij_new]=value_old[index[ij_old]];
      }
      /* Repeat location: don't update for SPARSE_POPULATE_FIRST */
      else
      {
        if (matrix->matrix_type==SPARSE_POPULATE_SUM)
        {
          value_new[ij_new]+=value_old[index[ij_old]];
        }
        else if (matrix->matrix_type==SPARSE_POPULATE_LAST)
        {
          value_new[ij_new]=value_old[index[ij_old]];
        }
      }
    }
    ij_new++;
    for (i=irow_current+1;i<=matrix->num_row;i++)
    {
      irow_new[i]=ij_new;
    }
    num_elem=ij_new;
    irow_new[matrix->num_row+1]=num_elem;

    /* Realloc the new vectors, and copy the data back */
    DEALLOCATE(irow_old);
    DEALLOCATE(icol_old);
    DEALLOCATE(value_old);
    if ((ALLOCATE(icol_old,int,num_elem))&&
        (ALLOCATE(value_old,Matrix_value,num_elem)))
    {
      for (i=0;i<num_elem;i++)
      {
        icol_old[i]=icol_new[i];
        value_old[i]=value_new[i];
      }
      matrix->irow=irow_new;
      matrix->icol=icol_old;
      matrix->value=value_old;
      matrix->n_size=num_elem;
      matrix->num_elem=0;
      matrix->matrix_type=COMPRESSED_ROW;
      DEALLOCATE(icol_new);
      DEALLOCATE(value_new);

      return_code=0;
    }
    else
    {
      display_message(ERROR_MESSAGE,
        "Matrix_convert_sparse_populate_to_compressed_row.  "
        "Could not allocate memory for sparse matrix");
      return_code=0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,
      "Matrix_convert_sparse_populate_to_compressed_row.  "
      "Could not allocate memory for sparse matrix");
    DEALLOCATE(icol_new);
    DEALLOCATE(irow_new);
    return_code=0;
  }
  LEAVE;

  return (return_code);
} /* Matrix_convert_sparse_populate_to_compressed_row */


static int Matrix_convert_dense_to_compressed_row(struct Matrix *matrix)
/*******************************************************************************
LAST MODIFIED : 8 April 2003

DESCRIPTION :
Convert a dense matrix to a compressed row matrix.
==============================================================================*/
{
  int i,j,ij_sparse,ij_dense,num_elem,return_code;
  int *irow,*icol;
  Matrix_value *value;

  ENTER(Matrix_convert_dense_to_compressed_row);
  /* Count the number of non-zero elements */
  num_elem=0;
  for (i=0;i<(matrix->n_size);i++)
  {
    if (matrix->value[i]!=0.0)
    {
      num_elem++;
    }
  }

  /* Allocate memory, and copy across values */
  if ((ALLOCATE(irow,int,(matrix->num_row)+1))&&
      (ALLOCATE(icol,int,num_elem))&&
      (ALLOCATE(value,Matrix_value,num_elem)))
  {
    irow[0]=0;
    ij_sparse=0;
    for (i=0;i<(matrix->num_row);i++)
    {
      ij_dense=i;
      irow[i+1]=irow[i];
      for (j=0;j<(matrix->num_col);j++)
      {
        if ((matrix->value[ij_dense])!=0.0)
        {
          value[ij_sparse]=(matrix->value[ij_dense]);
          icol[ij_sparse]=j;
          irow[i+1]++;
          ij_sparse++;
        }
        ij_dense+=(matrix->num_row);
      }
    }

    /* Deallocate old storage, and shift new pointers */
    DEALLOCATE(matrix->value);
    matrix->value=value;
    matrix->irow=irow;
    matrix->icol=icol;
    matrix->n_size=num_elem;
    matrix->num_elem=0;
    matrix->matrix_type=COMPRESSED_ROW;

    return_code=1;
  }
  else
  {
    display_message(ERROR_MESSAGE,"Matrix_convert_dense_to_compressed_row.  "
      "Could not allocate memory for sparse matrix");
    DEALLOCATE(icol);
    DEALLOCATE(irow);
    return_code=0;
  }
  LEAVE;

  return (return_code);
} /* Matrix_convert_dense_to_compressed_row */


static int Matrix_convert_dense_to_sparse_populate(struct Matrix *matrix)
/*******************************************************************************
LAST MODIFIED : 8 April 2003

DESCRIPTION :
Convert a dense matrix to a sparse populate matrix.
==============================================================================*/
{
  int i,j,ij_sparse,ij_dense,num_elem,return_code;
  int *irow,*icol;
  Matrix_value *value;

  ENTER(Matrix_convert_dense_to_sparse_populate);
  /* Count the number of non-zero elements */
  num_elem=0;
  for (i=0;i<(matrix->n_size);i++)
  {
    if (matrix->value[i]!=0.0)
    {
      num_elem++;
    }
  }

  /* Allocate memory, and copy across values */
  if ((ALLOCATE(irow,int,num_elem))&&
      (ALLOCATE(icol,int,num_elem))&&
      (ALLOCATE(value,Matrix_value,num_elem)))
  {
    ij_sparse=0;
    for (i=0;i<(matrix->num_row);i++)
    {
      ij_dense=i;
      for (j=0;j<(matrix->num_col);j++)
      {
        if ((matrix->value[ij_dense])!=0.0)
        {
          value[ij_sparse]=(matrix->value[ij_dense]);
          icol[ij_sparse]=j+1;
          irow[ij_sparse]=i+1;
          ij_sparse++;
        }
        ij_dense+=(matrix->num_row);
      }
    }

    /* Deallocate old storage, and shift new pointers */
    DEALLOCATE(matrix->value);
    matrix->value=value;
    matrix->irow=irow;
    matrix->icol=icol;
    matrix->n_size=num_elem;
    matrix->num_elem=num_elem;
    matrix->matrix_type=SPARSE_POPULATE;

    return_code=1;
  }
  else
  {
    display_message(ERROR_MESSAGE,"Matrix_convert_dense_to_sparse_populate.  "
      "Could not allocate memory for sparse matrix");
    DEALLOCATE(icol);
    DEALLOCATE(irow);
    return_code=0;
  }
  LEAVE;

  return (return_code);
} /* Matrix_convert_dense_to_sparse_populate */


static int Matrix_set_dimensions_dense(struct Matrix *matrix,int num_row,
  int num_col)
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Sets the dimensions of the dense <matrix> to <num_row> x <num_col>.

This could be done in a more efficent method, both in terms of number of 
operations and in memory usage (you can use a realloc rather than alloc
and copy), but it is coded in the simple method below for clarity
==============================================================================*/
{
  int i,j,ij_old,ij_new,new_size,num_col_min,num_row_min,return_code;
  Matrix_value *value;

  ENTER(Matrix_set_dimensions_dense);
  /* Find our array dimensions (Fortran has these amazing min() and max()
     intrinsics ... */
  if (num_col<(matrix->num_col)) {
    num_col_min=num_col;
  }
  else {
    num_col_min=(matrix->num_col);
  }
  if (num_row<(matrix->num_row)) {
    num_row_min=num_row;
  }
  else {
    num_row_min=(matrix->num_row);
  }
  new_size=num_row*num_col;

  if (ALLOCATE(value,Matrix_value,new_size))
  {
    /* Zero the new array, then copy across the old values */
    for (i=0;i<new_size;i++)
    {
      value[i]=0.0;
    }
    for (j=0;j<num_col_min;j++)
    {
      for (i=0;i<num_row_min;i++)
      {
        ij_old=i+j*(matrix->num_row);
        ij_new=i+j*(num_row);
        value[ij_new]=matrix->value[ij_old];
      }
    }
    DEALLOCATE(matrix->value);
    matrix->value=value;
    matrix->num_row=num_row;
    matrix->num_col=num_col;
    matrix->n_size=new_size;

    return_code=1;
  }
  else
  {
    display_message(ERROR_MESSAGE,"Matrix_set_dimensions_dense.  "
      "Error allocating memory");
    return_code=0;            
  }
  LEAVE;

  return (return_code);
} /* Matrix_set_dimensions_dense */


static int Matrix_set_dimensions_compressed_row(struct Matrix *matrix,
  int num_row,int num_col)
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Sets the dimensions of the compressed row <matrix> to <num_row> x <num_col>.

This could be done in a more efficent method, both in terms of number of 
operations and in memory usage (you can use a realloc rather than alloc
and copy), but it is coded in the simple method below for clarity
==============================================================================*/
{
  int i,ij_new,ij_old,ihigh,new_size,num_row_min,return_code;
  int *irow,*icol;
  Matrix_value *value;

  ENTER(Matrix_set_compressed_row);
  /* Find our array dimensions (Fortran has these amazing min() and max()
     intrinsics ... */
  if (num_row<(matrix->num_row)) {
    num_row_min=num_row;
  }
  else {
    num_row_min=(matrix->num_row);
  }
  ihigh=(matrix->irow[num_row_min+1]);

  /* Count the new number of elements */
  for (i=0,new_size=0;i<ihigh;i++)
  {
    if ((matrix->icol[i])<num_col)
    {
      new_size++;
    }
  }

  /* Allocate and copy across */
  if (ALLOCATE(irow,int,num_row+1)&&
      ALLOCATE(icol,int,new_size)&&
      ALLOCATE(value,Matrix_value,new_size))
  {
    for (i=0,ij_new=0,irow[0]=0;i<num_row_min;i++)
    {
      for (ij_old=(matrix->irow[i]);ij_old<(matrix->irow[i+1]);ij_old++)
      {
        if ((matrix->icol[ij_old])<num_col)
        {
          icol[ij_new]=(matrix->icol[ij_old]);
          value[ij_new]=(matrix->value[ij_old]);
          ij_new++;
        }
      }
      irow[i+1]=ij_new;
    }
    for (i=num_row_min+1;i<num_row+1;i++)
    {
      irow[i]=ij_new;
    }

    /* Clear old memory, and set pointers etc */
    DEALLOCATE(matrix->value);
    DEALLOCATE(matrix->icol);
    DEALLOCATE(matrix->irow);
    matrix->value=value;
    matrix->irow=irow;
    matrix->icol=icol;
    matrix->n_size=new_size;
    matrix->num_row=num_row;
    matrix->num_col=num_col;

    return_code=1;            
  }
  else
  {
    display_message(ERROR_MESSAGE,"Matrix_set_dimensions_compressed_row.  "
      "Error allocating memory");
    return_code=0;            
  }
  LEAVE;

  return (return_code);
} /* Matrix_set_dimensions_compressed_row */


static int Matrix_set_dimensions_sparse_populate(struct Matrix *matrix,
  int num_row,int num_col)
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Sets the dimensions of the sparse populate <matrix> to <num_row> x <num_col>.
==============================================================================*/
{
  int i,ij_old,ij_new,new_size,return_code;
  int *irow,*icol;
  Matrix_value *value;

  ENTER(Matrix_set_sparse_populate);
  /* If we are increasing size, just reset array dimensions */
  if ((matrix->num_row)<=num_row&&(matrix->num_col)<=num_col)
  {
    /* If we have a zero size array we will have to make some space */
    if (matrix->n_size<=0)
    {
      if (num_row>num_col)
      {
        new_size=num_row;
      }
      else
      {
        new_size=num_col;
      }
      if (ALLOCATE(irow,int,new_size)&&
          ALLOCATE(icol,int,new_size)&&
          ALLOCATE(value,Matrix_value,new_size))
      {
        /* Just in case these were pointing to something ... */
        DEALLOCATE(matrix->value);
        DEALLOCATE(matrix->icol);
        DEALLOCATE(matrix->irow);
        matrix->irow=irow;
        matrix->icol=icol;
        matrix->value=value;
        matrix->num_row=num_row;
        matrix->num_col=num_col;
        matrix->n_size=new_size;
        for (i=0;i<new_size;i++)
        {
          matrix->irow[i]=0;
          matrix->icol[i]=0;
          matrix->value[i]=0.0;
        }
        return_code=1;
      }
      else
      {
        display_message(ERROR_MESSAGE,"Matrix_set_dimensions_sparse_populate.  "
          "Error allocating memory");
        return_code=0;
      }
    }
    /* Otherwise just increase the dimension flags */
    else
    {
      matrix->num_row=num_row;
      matrix->num_col=num_col;
      return_code=1;
    }
  }

  /* Otherwise we have to remove the unwanted elements */
  else
  {
    for (ij_old=0,ij_new=0;ij_old<matrix->num_elem;ij_old++)
    {
      if ((matrix->icol[ij_old])<num_col&&(matrix->irow[ij_old])<num_row)
      {
        matrix->icol[ij_new]=matrix->icol[ij_old];
        matrix->irow[ij_new]=matrix->irow[ij_old];
        matrix->value[ij_new]=matrix->value[ij_old];
        ij_new++;
      }
    }
    matrix->num_elem=ij_new;
    matrix->num_row=num_row;
    matrix->num_col=num_col;

#ifndef DEALLOC_MEMORY_ON_RESIZE

    /* Clear memory */
    for (i=(matrix->num_elem);i<(matrix->n_size);i++)
    {
      matrix->icol[i]=0;
      matrix->irow[i]=0;
      matrix->value[i]=0.0;
    }

    return_code=1;

#else /* DEALLOC_MEMORY_ON_RESIZE */

    /* See if we want to remove old memory:
       this code is not yet tested */
    num_elem=matrix->num_elem;
    new_size=matrix->n_size;
    new_size_tmp=new_size >> 1;
    while (num_elem>new_size_tmp)
    {
      new_size=new_size_tmp;
      new_size_tmp=new_size_tmp >> 1;
    }
    if (new_size<matrix->n_size)
    {
      if ((REALLOCATE(irow,matrix->irow,int,new_size))&&
          (REALLOCATE(icol,matrix->icol,int,new_size))&&
          (REALLOCATE(value,matrix->value,Matrix_value,new_size)))
      {
        matrix->irow=irow;
        matrix->icol=icol;
        matrix->value=value;
        matrix->n_size=new_size;

        return_code=1;
      }
      else
      {
        display_message(ERROR_MESSAGE,"Matrix_copy_sparse_populate.  "
          "Error reallocating memory");
        Matrix_reset_matrix(matrix);
        return_code=0;
      }
    }

    /* Clear memory */
    if (return_code)
    {
      for (i=(matrix->num_elem);i<(matrix->n_size);i++)
      {
        matrix->icol[i]=0;
        matrix->irow[i]=0;
        matrix->value[i]=0.0;
      }
    }
#endif
  }
  LEAVE;

  return (return_code);
} /* Matrix_set_dimensions_sparse_populate */


static int Matrix_copy_dense(struct Matrix *matrix_1,struct Matrix *matrix_2)
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Copies the contents of dense <matrix_1> into <matrix_2>.
==============================================================================*/
{
  int i,return_code;
  Matrix_value *value;

  ENTER(Matrix_copy_dense);
  /* If we are changing sizes, reallocate */
  if ((matrix_2->n_size)!=(matrix_1->n_size))
  {
    if (REALLOCATE(value,matrix_2->value,Matrix_value,(matrix_1->n_size)))
    {
      matrix_2->value=value;
      matrix_2->n_size=matrix_1->n_size;

      return_code=1;
    }
    else
    {
      display_message(ERROR_MESSAGE,"Matrix_copy.  "
        "Error reallocating memory");
      /* Clear old data since array is now trashed */
      Matrix_reset_matrix(matrix_2);
      return_code=0;
    }
  }
  else
  {
    return_code=1;
  }

  /* If we have sucessfully realloced (or didn't need to), copy
     across the data */
  if (return_code)
  {
    matrix_2->matrix_type=DENSE;
    DEALLOCATE(matrix_2->icol);
    DEALLOCATE(matrix_2->irow);
    matrix_2->icol=NULL;
    matrix_2->irow=NULL;
    matrix_2->num_row=matrix_1->num_row;
    matrix_2->num_col=matrix_1->num_col;
    matrix_2->num_elem=matrix_1->num_elem;
    for (i=0;i<(matrix_1->n_size);i++)
    {
      matrix_2->value[i]=matrix_1->value[i];
    }
  }
  LEAVE;

  return (return_code);
} /* Matrix_copy_dense */


static int Matrix_copy_compressed_row(struct Matrix *matrix_1,
  struct Matrix *matrix_2)
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Copies the contents of compressed row <matrix_1> into <matrix_2>.
==============================================================================*/
{
  int i,irow_size,icol_size,return_code;
  int *irow,*icol;
  Matrix_value *value;

  ENTER(Matrix_copy_compressed_row);
  irow_size=(matrix_1->num_row)+1;
  icol_size=(matrix_1->n_size);

  /* Reeallocate and copy data */
  if ((REALLOCATE(irow,matrix_2->irow,int,irow_size))&&
      (REALLOCATE(icol,matrix_2->icol,int,icol_size))&&
      (REALLOCATE(value,matrix_2->value,Matrix_value,icol_size)))
  {
    matrix_2->matrix_type=COMPRESSED_ROW;
    matrix_2->irow=irow;
    matrix_2->icol=icol;
    matrix_2->value=value;
    matrix_2->num_row=matrix_1->num_row;
    matrix_2->num_col=matrix_1->num_col;
    matrix_2->n_size=matrix_1->n_size;
    matrix_2->num_elem=matrix_1->num_elem;

    for (i=0;i<irow_size;i++)
    {
      matrix_2->irow[i]=matrix_1->irow[i];
    }
    for (i=0;i<icol_size;i++)
    {
      matrix_2->icol[i]=matrix_1->icol[i];
      matrix_2->value[i]=matrix_1->value[i];
    }
    return_code=1;
  }
  else
  {
    display_message(ERROR_MESSAGE,"Matrix_copy_compressed_row.  "
      "Error reallocating memory");
    /* Clear old data since array is now trashed */
    Matrix_reset_matrix(matrix_2);
    return_code=0;
  }
  LEAVE;

  return (return_code);
} /* Matrix_copy_compressed_row */


static int Matrix_copy_sparse_populate(struct Matrix *matrix_1,
  struct Matrix *matrix_2)
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Copies the contents of sparse populate <matrix_1> into <matrix_2>.
==============================================================================*/
{
  int i,new_size,return_code;
  int *irow,*icol;
  Matrix_value *value;

  ENTER(Matrix_copy_sparse_populate);
  new_size=(matrix_1->n_size);

  /* Reeallocate and copy data */
  if ((REALLOCATE(irow,matrix_2->irow,int,new_size))&&
      (REALLOCATE(icol,matrix_2->icol,int,new_size))&&
      (REALLOCATE(value,matrix_2->value,Matrix_value,new_size)))
  {
    matrix_2->matrix_type=matrix_1->matrix_type;
    matrix_2->irow=irow;
    matrix_2->icol=icol;
    matrix_2->value=value;
    matrix_2->num_row=matrix_1->num_row;
    matrix_2->num_col=matrix_1->num_col;
    matrix_2->n_size=matrix_1->n_size;
    matrix_2->num_elem=matrix_1->num_elem;

    for (i=0;i<new_size;i++)
    {
      matrix_2->irow[i]=matrix_1->irow[i];
      matrix_2->icol[i]=matrix_1->icol[i];
      matrix_2->value[i]=matrix_1->value[i];
    }
    return_code=1;
  }
  else
  {
    display_message(ERROR_MESSAGE,"Matrix_copy_sparse_populate.  "
      "Error reallocating memory");
    /* Clear old data since array is now trashed */
    Matrix_reset_matrix(matrix_2);
    return_code=0;
  }
  LEAVE;

  return (return_code);
} /* Matrix_copy_sparse_populate */


static struct Matrix *CREATE(Matrix_dense)(struct Matrix *matrix,
  int num_row,int num_col)
/*******************************************************************************
LAST MODIFIED : 20 July 2003

DESCRIPTION :
Creates a dense matrix with the specified dimensions <num_row>x<num_col>.
==============================================================================*/
{
  int i,n_size;
	struct Matrix *return_matrix;

  ENTER(CREATE(Matrix_dense));
	return_matrix=(struct Matrix *)NULL;
  n_size=(num_row*num_col);
  if ((0<n_size)&&matrix&&ALLOCATE(matrix->value,Matrix_value,n_size))
  {
    matrix->matrix_type=DENSE;
    matrix->irow=NULL;
    matrix->icol=NULL;
    matrix->num_row=num_row;
    matrix->num_col=num_col;
    matrix->n_size=n_size;
    matrix->num_elem=0;
    for (i=0;i<n_size;i++)
    {
      matrix->value[i]=0.0;
    }
		return_matrix=matrix;
  }
  else
  {
    display_message(ERROR_MESSAGE,"CREATE(Matrix_dense).  "
      "Could not allocate matrix->value.  %p %d %d",matrix,num_row,num_col);
		if (matrix)
		{
			DEALLOCATE(matrix->name);
			DEALLOCATE(matrix);
		}
  }
  LEAVE;

  return (return_matrix);
} /* CREATE(Matrix_dense) */


static struct Matrix *CREATE(Matrix_compressed_row)(struct Matrix *matrix,
  int num_row,int num_col)
/*******************************************************************************
LAST MODIFIED : 8 April 2003

DESCRIPTION :
Creates a compressed row matrix with the specified dimensions 
<num_row>x<num_col>.
==============================================================================*/
{
  int i;

  ENTER(CREATE(Matrix_compressed_row));
  if (ALLOCATE(matrix->irow,int,(num_row+1)))
  {
    matrix->matrix_type=COMPRESSED_ROW;
    matrix->icol=NULL;
    matrix->value=NULL;
    matrix->num_row=num_row;
    matrix->num_col=num_col;
    matrix->n_size=0;
    matrix->num_elem=0;
    for (i=0;i<(num_row+1);i++)
    {
      matrix->irow[i]=0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"CREATE(Matrix_compressed_row).  "
      "Could not allocate matrix->irow");
    DEALLOCATE(matrix->name);
    DEALLOCATE(matrix);
    matrix=NULL;
  }
  LEAVE;

  return (matrix);
} /* CREATE(Matrix_compressed_row) */


static struct Matrix *CREATE(Matrix_sparse_populate)(struct Matrix *matrix,
  enum Matrix_type matrix_type,int num_row,int num_col)
/*******************************************************************************
LAST MODIFIED : 8 April 2003

DESCRIPTION :
Creates a sparse populate matrix of type <matrix_type> and dimensions 
<num_row>x<num_col>.
==============================================================================*/
{
  int i,num_size;

  ENTER(CREATE(Matrix_sparse_populate));
  if (num_col>num_row) {
    num_size=num_col;
  }
  else {
    num_size=num_row;
  }

  if (ALLOCATE(matrix->irow,int,num_size)&&
      ALLOCATE(matrix->icol,int,num_size)&&
      ALLOCATE(matrix->value,Matrix_value,num_size))
  {
    matrix->matrix_type=matrix_type;
    matrix->num_row=num_row;
    matrix->num_col=num_col;
    matrix->n_size=num_size;
    matrix->num_elem=0;
    for (i=0;i<num_size;i++)
    {
      matrix->irow[i]=0;
      matrix->icol[i]=0;
      matrix->value[i]=0.0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"CREATE(Matrix_sparse_populate).  "
      "Could not allocate matrix->irow,icol,value");
    DEALLOCATE(matrix->icol);
    DEALLOCATE(matrix->irow);
    DEALLOCATE(matrix->name);
    DEALLOCATE(matrix);
    matrix=NULL;
  }
  LEAVE;

  return (matrix);
} /* CREATE(Matrix_sparse_populate) */


/*
Global functions
----------------
*/
struct Matrix *CREATE(Matrix)(char *name,enum Matrix_type matrix_type,
  int num_row,int num_col)
/*******************************************************************************
LAST MODIFIED : 17 March 2003

DESCRIPTION :
Creates a matrix object of <matrix_type> with the specified dimensions
<num_row>x<num_col> and symbolic <name>.

???SN.  So do we want <name>?
==============================================================================*/
{
  struct Matrix *matrix;

  ENTER(CREATE(Matrix));
  matrix=(struct Matrix *)NULL;
  if (name&&(num_row>=0)&&(num_col>=0))
  {
    if (ALLOCATE(matrix,struct Matrix,1))
    {
      /* Zero matrix values */
      matrix->name=NULL;
      matrix->irow=NULL;
      matrix->icol=NULL;
      matrix->value=NULL;

      /* Copy matrix name */
      if (ALLOCATE(matrix->name,char,strlen(name)+1))
      {
        strcpy(matrix->name,name);

        /* Allocate storage for matrix values */
        switch (matrix_type)
        {
          case DENSE:
          {
            matrix=CREATE(Matrix_dense)(matrix,num_row,num_col);
          } break;

          case COMPRESSED_ROW:
          {
            matrix=CREATE(Matrix_compressed_row)(matrix,num_row,num_col);
          } break;

          case SPARSE_POPULATE_FIRST:
          case SPARSE_POPULATE_LAST:
          case SPARSE_POPULATE_SUM:
          {
            matrix=CREATE(Matrix_sparse_populate)(matrix,matrix_type,
              num_row,num_col);
          } break;

          default:
          {
            display_message(ERROR_MESSAGE,"CREATE(Matrix).  "
              "Unknown matrix type %d",matrix_type);
            DEALLOCATE(matrix->name);
            DEALLOCATE(matrix);
            matrix=NULL;
          } break;
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,"CREATE(Matrix).  "
          "Could not allocate matrix->name");
        DEALLOCATE(matrix);
        matrix=NULL;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"CREATE(Matrix).  "
        "Could not allocate matrix");
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"CREATE(Matrix).  "
      "Invalid argument(s).  %p %d %d",name,num_row,num_col);
  }
  LEAVE;

  return (matrix);
} /* CREATE(Matrix) */


int Matrix_recreate(struct Matrix *matrix,enum Matrix_type matrix_type,
  int num_row,int num_col)
/*******************************************************************************
LAST MODIFIED : 1 May 2003

DESCRIPTION :
Recreates a matrix object of <matrix_type> with the specified dimensions
<num_row>x<num_col>, throwing away the data already contained in it.
==============================================================================*/
{
  int return_code;

  ENTER(Matrix_recreate);
  if (matrix&&(num_row>=0)&&(num_col>=0))
  {
    /* Allocate storage for matrix values */
    switch (matrix_type)
    {
      case DENSE:
      {
        Matrix_reset_matrix(matrix);
        matrix=CREATE(Matrix_dense)(matrix,num_row,num_col);
        return_code=(matrix!=0);
      } break;

      case COMPRESSED_ROW:
      {
        Matrix_reset_matrix(matrix);
        matrix=CREATE(Matrix_compressed_row)(matrix,num_row,num_col);
        return_code=(matrix!=0);
      } break;

      case SPARSE_POPULATE_FIRST:
      case SPARSE_POPULATE_LAST:
      case SPARSE_POPULATE_SUM:
      {
        Matrix_reset_matrix(matrix);
        matrix=CREATE(Matrix_sparse_populate)(matrix,matrix_type,
          num_row,num_col);
        return_code=(matrix!=0);
      } break;

      default:
      {
        display_message(ERROR_MESSAGE,"Matrix_recreate.  "
          "Unknown matrix type %d",matrix_type);
        return_code=0;
      } break;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Matrix_recreate.  "
      "Invalid argument(s).  %p %d %d",matrix,num_row,num_col);
    return_code=0;
  }
  LEAVE;

  return (return_code);
} /* Matrix_recreate */


int DESTROY(Matrix)(struct Matrix **matrix_address)
/*******************************************************************************
LAST MODIFIED : 17 March 2003

DESCRIPTION :
Free memory/deaccess objects in matrix at <*matrix_address>.
==============================================================================*/
{
  int return_code;
  struct Matrix *matrix;

  ENTER(DESTROY(Matrix));
  return_code=0;
  if (matrix_address&&(matrix= *matrix_address))
  {
    DEALLOCATE(matrix->value);
    DEALLOCATE(matrix->icol);
    DEALLOCATE(matrix->irow);
    DEALLOCATE(matrix->name);
    DEALLOCATE(matrix);
    *matrix_address=(struct Matrix *)NULL;
    return_code=1;
  }
  LEAVE;

  return (return_code);
} /* DESTROY(Matrix) */


int Matrix_set_value(struct Matrix *matrix,int row,int column,
  Matrix_value value)
/*******************************************************************************
LAST MODIFIED : 17 March 2003

DESCRIPTION :
Sets the <value> at the (<row>,<column>) location of <matrix>.
==============================================================================*/
{
  int ij,ilow,ihigh,new_size,return_code;
  int *irow,*icol;
  Matrix_value *value_pointer;

  ENTER(Matrix_set_value);
  if (matrix)
  {
    if ((row<=(matrix->num_row))&&(row>0)&&
        (column<=(matrix->num_col))&&(column>0))
    {
      switch (matrix->matrix_type)
      {
        /* Dense matrix */
        case DENSE:
        {
          ij=(row-1)+((column-1)*matrix->num_row);
          matrix->value[ij]=value;
          return_code=1;
        } break;

        /* Sparse matrix: can only replace existing locations */
        case COMPRESSED_ROW:
        {
          ilow=matrix->irow[row-1];
          ihigh=matrix->irow[row];
          /* if (Matrix_find_index_linear(matrix->icol,ilow,ihigh,column-1,&ij)) */
          if (Matrix_find_index_bisection(matrix->icol,ilow,ihigh,column-1,&ij))
          {
            matrix->value[ij]=value;
            return_code=1;
          }
          else
          {
            return_code=0;
          }
        } break;

        /* Sparse populate: not yet implimented */
        case SPARSE_POPULATE_FIRST:
        case SPARSE_POPULATE_LAST:
        case SPARSE_POPULATE_SUM:
        {
          /* Grow the matrix */
          if ((matrix->n_size)<=(matrix->num_elem))
          {
            new_size=MATRIX_GROWTH_FACTOR*(matrix->n_size);
            if (REALLOCATE(irow,matrix->irow,int,new_size)&&
                REALLOCATE(icol,matrix->icol,int,new_size)&&
                REALLOCATE(value_pointer,matrix->value,Matrix_value,new_size))
            {
              matrix->icol=icol;
              matrix->irow=irow;
              matrix->value=value_pointer;
              matrix->n_size=new_size;
            }
            else
            {
              display_message(ERROR_MESSAGE,"Matrix_set_value.  "
                "Error growing sparse matrix storage");
              return_code=0;
              break;
            }
          }
          /* If we grew the matrix OK (or we didn't need to grow it)
             add the new variable */
          matrix->irow[matrix->num_elem]=row;
          matrix->icol[matrix->num_elem]=column;
          matrix->value[matrix->num_elem]=value;
          matrix->num_elem++;
          return_code=1;
        } break;

        /* Error */
        default:
        {
          display_message(ERROR_MESSAGE,"Matrix_set_value.  "
            "Unknown matrix type %d",matrix->matrix_type);
          return_code=0;
        } break;
      } /* switch (matrix->matrix_type) */
    }
    else
    {
      display_message(ERROR_MESSAGE,"Matrix_set_value.  "
        "Element out of bounds 1 <= %d <= %d,  1 <= %d <= %d",
        row,matrix->num_row,column,matrix->num_col);
      return_code=0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Matrix_set_value.  "
      "Invalid argument.  %p",matrix);
    return_code=0;
  }
  LEAVE;

  return (return_code);
} /* Matrix_set_value */


int Matrix_get_value(struct Matrix *matrix,int row,int column,
  Matrix_value *value_address)
/*******************************************************************************
LAST MODIFIED : 3 July 2003

DESCRIPTION :
Gets the value at the (<row>,<column>) location of <matrix> returning it in 
<*value_address>.
==============================================================================*/
{
  int i,ij,ilow,ihigh,return_code;

  ENTER(Matrix_get_value);
  if (matrix&&value_address)
  {
    if ((row<=(matrix->num_row))&&(row>0)&&
        (column<=(matrix->num_col))&&(column>0))
    {
      switch (matrix->matrix_type)
      {
        case DENSE:
        {
          ij=(row-1)+((column-1)*matrix->num_row);
          *value_address=(matrix->value[ij]);
          return_code=1;
        } break;

        case COMPRESSED_ROW:
        {
          ilow=matrix->irow[row-1];
          ihigh=matrix->irow[row];
          /* if (Matrix_find_index_linear(matrix->icol,ilow,ihigh,column-1,&ij)) */
          if (Matrix_find_index_bisection(matrix->icol,ilow,ihigh,column-1,&ij))
          {
            *value_address=(matrix->value[ij]);
          }
          else
          {
            *value_address=0.0;
          }
          return_code=1;
        } break;

        case SPARSE_POPULATE_FIRST:
        {
          *value_address=0.0;
          for (i=0;i<(matrix->num_elem);i++)
          {
            if ((matrix->irow[i]==row)&&(matrix->icol[i]==column))
            {
              *value_address=matrix->value[i];
              break;
            }
          }
        } break;
        case SPARSE_POPULATE_LAST:
        {
          *value_address=0.0;
          for (i=(matrix->num_elem)-1;i>=0;i--)
          {
            if ((matrix->irow[i]==row)&&(matrix->icol[i]==column))
            {
              *value_address=matrix->value[i];
              break;
            }
          }
        } break;
        case SPARSE_POPULATE_SUM:
        {
          *value_address=0.0;
          for (i=(matrix->num_elem)-1;i>=0;i--)
          {
            if ((matrix->irow[i]==row)&&(matrix->icol[i]==column))
            {
              *value_address+=matrix->value[i];
            }
          }
        } break;

        default:
        {
          display_message(ERROR_MESSAGE,"Matrix_get_value.  "
            "Unknown matrix type %d",matrix->matrix_type);
          return_code=0;
        } break;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"Matrix_get_value.  "
        "Element out of bounds 1 <= %d <= %d,  1 <= %d <= %d",
        row,matrix->num_row,column,matrix->num_col);
      return_code=0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Matrix_get_value.  "
      "Invalid argument(s).  %p %p",matrix,value_address);
    return_code=0;
  }
  LEAVE;

  return (return_code);
} /* Matrix_get_value */


int Matrix_set_values(struct Matrix *matrix,Matrix_value *values,int row_low,
  int row_high,int column_low,int column_high)
/*******************************************************************************
LAST MODIFIED : 2 May 2003

DESCRIPTION :
Sets the block of values within <matrix> located in the range
<row_low:row_high><column_low:column_high>.
Note that <values> has Fortran ordering (ie: the row index increasing fastest).
==============================================================================*/
{
  int i,j,k,ij,num_row,num_col,num_block,new_size;
  int *irow,*icol;
  Matrix_value *value_pointer;
  int return_code;

  ENTER(Matrix_set_values);
  if (matrix&&values)
  {
    if ((row_low>0)&&(column_low>0)&&
        (row_high<=(matrix->num_row))&&(column_high<=(matrix->num_col))&&
        (row_low<=row_high)&&(column_low<=column_high))
    {
      num_row=(row_high-row_low)+1;
      num_col=(column_high-column_low)+1;
      num_block=num_row+num_col;

      switch (matrix->matrix_type)
      {
        /* Dense matrix */
        case DENSE:
        {
          k=0;
          for (j=0;j<num_col;j++)
          {
            ij=(row_low-1)+((column_low+j-1)*matrix->num_row);
            for (i=0;i<num_row;i++)
            {
              matrix->value[ij]=values[k];
              ij++;
              k++;
            }
          }
          return_code=1;
        } break;

        /* Sparse matrix: can only replace existing locations */
        case COMPRESSED_ROW:
        {
          display_message(ERROR_MESSAGE,"Matrix_set_values.  "
            "Not implimented for COMPRESSED_ROW matricies");
          return_code=0;
        } break;

        /* Sparse populate: not yet implimented */
        case SPARSE_POPULATE_FIRST:
        case SPARSE_POPULATE_LAST:
        case SPARSE_POPULATE_SUM:
        {
          /* Grow the matrix */
          if (((matrix->n_size)+num_block)<=(matrix->num_elem))
          {
            new_size=(matrix->n_size);
            while (new_size<((matrix->n_size)+num_block))
            {
              new_size=MATRIX_GROWTH_FACTOR*new_size;
            }
            if (REALLOCATE(irow,matrix->irow,int,new_size)&&
                REALLOCATE(icol,matrix->icol,int,new_size)&&
                REALLOCATE(value_pointer,matrix->value,Matrix_value,new_size))
            {
              matrix->icol=icol;
              matrix->irow=irow;
              matrix->value=value_pointer;
              matrix->n_size=new_size;
            }
            else
            {
              display_message(ERROR_MESSAGE,"Matrix_set_values.  "
                "Error growing sparse matrix storage");
              return_code=0;
              break;
            }
          }
          /* If we grew the matrix OK (or we didn't need to grow it)
             add the new variable */
          for (k=0,j=column_low;j<=column_high;j++)
          {
            for (i=row_low;i<=row_high;i++)
            {
              matrix->irow[matrix->num_elem]=i;
              matrix->icol[matrix->num_elem]=j;
              matrix->value[matrix->num_elem]=values[k];
              matrix->num_elem++;
              k++;
            }
          }
          return_code=1;
        } break;

        /* Error */
        default:
        {
          display_message(ERROR_MESSAGE,"Matrix_set_values.  "
            "Unknown matrix type %d",matrix->matrix_type);
          return_code=0;
        } break;
      } /* switch (matrix->matrix_type) */
    }
    else
    {
      display_message(ERROR_MESSAGE,"Matrix_set_values.  "
        "Value block out of bounds 1 <= %d <= %d <= %d,  1 <= %d %d <= %d",
        row_low,row_high,matrix->num_row,column_low,column_high,
        matrix->num_col);
      return_code=0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Matrix_set_values.  "
      "Invalid argument(s).  %p %p",matrix,values);
    return_code=0;
  }
  LEAVE;

  return (return_code);  
} /* Matrix_set_values */


int Matrix_get_values(struct Matrix *matrix,Matrix_value *values,int row_low,
  int row_high,int column_low,int column_high)
/*******************************************************************************
LAST MODIFIED : 2 May 2003

DESCRIPTION :
Get the block of values within <matrix> located in the range 
<row_low:row_high><column_low:column_high>.
Note that <values> has Fortran ordering (ie: the row index increasing fastest).
==============================================================================*/
{
  int num_row,num_col,num_block;
  int i,j,k,ij;
  int return_code;

  ENTER(Matrix_get_values);
  if (matrix&&values)
  {
    if ((row_low>0)&&(column_low>0)&&
        (row_high<=(matrix->num_row))&&(column_high<=(matrix->num_col))&&
        (row_low<=row_high)&&(column_low<=column_high))
    {
      num_row=(row_high-row_low)+1;
      num_col=(column_high-column_low)+1;
      num_block=num_row+num_col;

      switch (matrix->matrix_type)
      {
        /* Dense matrix */
        case DENSE:
        {
          for (k=0,j=0;j<num_col;j++)
          {
            ij=(row_low-1)+((column_low+j-1)*matrix->num_row);
            for (i=0;i<num_row;i++)
            {
              values[k]=matrix->value[ij];
              ij++;
              k++;
            }
          }
          return_code=1;
        } break;

        /* Sparse matrix */
        case COMPRESSED_ROW:
        {
          for (i=0;i<num_block;i++)
          {
            values[i]=0.0;
          }
          display_message(ERROR_MESSAGE,"Matrix_get_values.  "
            "Not implimented for COMPRESSED_ROW matricies");
          return_code=0;
        } break;

        /* Sparse populate: not yet implimented */
        case SPARSE_POPULATE_FIRST:
        case SPARSE_POPULATE_LAST:
        case SPARSE_POPULATE_SUM:
        {
          for (i=0;i<num_block;i++)
          {
            values[i]=0.0;
          }
          display_message(ERROR_MESSAGE,"Matrix_get_values.  "
            "Not implimented for SPARSE_POPULATE matricies");
          return_code=0;
        } break;

        /* Error */
        default:
        {
          display_message(ERROR_MESSAGE,"Matrix_get_values.  "
            "Unknown matrix type %d",matrix->matrix_type);
          return_code=0;
        } break;
      } /* switch (matrix->matrix_type) */
    }
    else
    {
      display_message(ERROR_MESSAGE,"Matrix_get_values.  "
        "Value block out of bounds 1 <= %d <= %d <= %d,  1 <= %d %d <= %d",
        row_low,row_high,matrix->num_row,column_low,column_high,
        matrix->num_col);
      return_code=0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Matrix_get_values.  "
      "Invalid argument(s).  %p %p",matrix,values);
    return_code=0;
  }
  LEAVE;

  return (return_code);  
} /* Matrix_get_values */


int Matrix_set_type(struct Matrix *matrix,enum Matrix_type matrix_type)
/*******************************************************************************
LAST MODIFIED : 17 March 2003

DESCRIPTION :
Set the storage <matrix_type> of <matrix>.

A matrix can be created as SPARSE_POPULATE and when the sparsity pattern has
been defined using Matrix_set_value its type can be changed to SPARSE for
computation.
==============================================================================*/
{
  int return_code;

  ENTER(Matrix_set_type);
  if (matrix)
  {
    /* If types are same, do nothing */
    if (matrix_type==(matrix->matrix_type))
    {
      return_code=1;
    }

    /* Convert to Dense */
    else if (matrix_type==DENSE)
    {
      switch(matrix->matrix_type)
      {
        /* Compressed Row -> Dense */
        case COMPRESSED_ROW:
        {
          return_code=Matrix_convert_compressed_row_to_dense(matrix);
        } break;

        /* Sparse Populate -> Dense */
        case SPARSE_POPULATE_FIRST:
        case SPARSE_POPULATE_LAST:
        case SPARSE_POPULATE_SUM:
        {
          return_code=Matrix_convert_sparse_populate_to_dense(matrix);
        } break;
      }
    }

    /* Convert to Compressed Row */
    else if (matrix_type==COMPRESSED_ROW)
    {
      switch(matrix->matrix_type)
      {
        /* Dense -> Compressed Row */
        case DENSE:
        {
          return_code=Matrix_convert_dense_to_compressed_row(matrix);
        } break;

        /* Sparse Populate -> Compressed Row */
        case SPARSE_POPULATE_FIRST:
        case SPARSE_POPULATE_LAST:
        case SPARSE_POPULATE_SUM:
        {
          return_code=Matrix_convert_sparse_populate_to_compressed_row(matrix);
        } break;
      }
    }

    /* Convert to Sparse Populate */
    else if (matrix_type==SPARSE_POPULATE       ||
             matrix_type==SPARSE_POPULATE_FIRST ||
             matrix_type==SPARSE_POPULATE_LAST  ||
             matrix_type==SPARSE_POPULATE_SUM)
    {
      switch(matrix->matrix_type)
      {
        /* Dense -> Sparse Populate */
        case DENSE:
        {
          return_code=Matrix_convert_dense_to_sparse_populate(matrix);
        } break;

        /* Compressed Row -> Sparse Populate */
        case COMPRESSED_ROW:
        {
          return_code=Matrix_convert_compressed_row_to_sparse_populate(matrix);
        } break;

        /* Sparse Populate A -> Sparse Populate B */
        case SPARSE_POPULATE_FIRST:
        case SPARSE_POPULATE_LAST:
        case SPARSE_POPULATE_SUM:
        {
          return_code=1;
        } break;
      }

      /* If we have sucessfully converted, set the matrix to the desired
         sub-species of Sparse Populate matrix */
      if (return_code)
      {
        matrix->matrix_type=matrix_type;
      }
    }

    /* Unknown matrix type */
    else
    {
      display_message(ERROR_MESSAGE,"Matrix_set_type.  "
        "Unknown Matrix_type %d",matrix_type);
      return_code=0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Matrix_set_type.  "
      "Invalid argument.  %p",matrix);
    return_code=0;
  }
  LEAVE;

  return (return_code);
} /* Matrix_set_type */


int Matrix_get_type(struct Matrix *matrix,enum Matrix_type *type_address)
/*******************************************************************************
LAST MODIFIED : 17 March 2003

DESCRIPTION :
Gets the storage type of <matrix>, returning it in <*type_address>.
==============================================================================*/
{
  int return_code;

  ENTER(Matrix_get_type);
  if (matrix&&type_address)
  {
    *type_address=(matrix->matrix_type);
    return_code=1;
  }
  else
  {
    display_message(ERROR_MESSAGE,"Matrix_get_type.  "
      "Invalid argument(s).  %p %p",matrix,type_address);
    return_code=0;
  }
  LEAVE;

  return (return_code);
} /* Matrix_get_type */


int Matrix_set_dimensions(struct Matrix *matrix,int num_row,int num_col)
/*******************************************************************************
LAST MODIFIED : 17 March 2003

DESCRIPTION :
Sets the dimensions of <matrix> to <num_row> x <num_col>.
==============================================================================*/
{
  int return_code;

  ENTER(Matrix_set_dimensions);
  if (matrix&&(num_row>=0)&&(num_col>=0))
  {
    switch (matrix->matrix_type)
    {
      case DENSE:
      {
        return_code=Matrix_set_dimensions_dense(matrix,num_row,num_col);
      } break;

      /* Compressed row */
      case COMPRESSED_ROW:
      {
        return_code=Matrix_set_dimensions_compressed_row(matrix,num_row,
          num_col);
      } break;

      case SPARSE_POPULATE_FIRST:
      case SPARSE_POPULATE_LAST:
      case SPARSE_POPULATE_SUM:
      {
        return_code=Matrix_set_dimensions_sparse_populate(matrix,num_row,
          num_col);
      } break;

      default:
      {
        display_message(ERROR_MESSAGE,"Matrix_set_dimensions.  "
          "Unknown matrix type %d",matrix->matrix_type);
        return_code=0;
      } break;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Matrix_set_dimensions.  "
      "Invalid argument(s).  %p %d %d",matrix,num_row,num_col);
    return_code=0;
  }
  LEAVE;

  return (return_code);
} /* Matrix_set_dimensions */

int Matrix_get_dimensions(struct Matrix *matrix,int *num_row_address,
  int *num_col_address)
/*******************************************************************************
LAST MODIFIED : 17 March 2003

DESCRIPTION :
Gets the dimensions of <matrix>, returning them in <*num_row_address> and
<*num_col_address>.
==============================================================================*/
{
  int return_code;

  ENTER(Matrix_get_dimensions);
  if (matrix&&num_col_address&&num_row_address)
  {
    *num_row_address=(matrix->num_row);
    *num_col_address=(matrix->num_col);
    return_code=1;
  }
  else
  {
    display_message(ERROR_MESSAGE,"Matrix_get_dimensions.  "
      "Invalid argument(s).  %p %p %p",matrix,num_col_address,num_row_address);
    return_code=0;
  }
  LEAVE;

  return (return_code);
} /* Matrix_get_dimensions */


int Matrix_copy(struct Matrix *matrix_1,struct Matrix *matrix_2)
/*******************************************************************************
LAST MODIFIED : 17 March 2003

DESCRIPTION :
Copies the contents of <matrix_1> into <matrix_2>.
==============================================================================*/
{
  int return_code;

  ENTER(Matrix_copy);
  if (matrix_1&&matrix_2)
  {
    switch (matrix_1->matrix_type)
    {
      case DENSE:
      {
        return_code=Matrix_copy_dense(matrix_1,matrix_2);
      } break;

      case COMPRESSED_ROW:
      {
        return_code=Matrix_copy_compressed_row(matrix_1,matrix_2);
      } break;

      /* Sparse populate matrix: not yet implimented */
      case SPARSE_POPULATE_FIRST:
      case SPARSE_POPULATE_LAST:
      case SPARSE_POPULATE_SUM:
      {
        return_code=Matrix_copy_sparse_populate(matrix_1,matrix_2);
      } break;

      default:
      {
        display_message(ERROR_MESSAGE,"Matrix_copy.  "
          "Unknown matrix type %d",matrix_1->matrix_type);
        return_code=0;
      } break;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Matrix_copy.  "
      "Invalid argument(s).  %p %p",matrix_1,matrix_2);
    return_code=0;
  }
  LEAVE;

  return (return_code);
} /* Matrix_copy */


int Matrix_print(struct Matrix *matrix,FILE *stream,int print_values)
/*******************************************************************************
LAST MODIFIED : 17 March 2003

DESCRIPTION :
Prints the <matrix> to the file <stream>.  If <print_values> then the values
are printed.
==============================================================================*/
{
  int i,ij,j,num_size,return_code;
  Matrix_value *array;

  ENTER(Matrix_print);
  if (stream&&matrix)
  {
    fprintf(stream,"\n");
    fprintf(stream,"Matrix name:    %s\n",matrix->name);
    fprintf(stream,"       address: %p\n",matrix);
    fprintf(stream,"       type:    ");
    switch (matrix->matrix_type)
    {
      case DENSE:
      {
        fprintf(stream,"Dense\n");
      } break;
      case COMPRESSED_ROW:
      {
        fprintf(stream,"Compressed Row\n");
      } break;
      case SPARSE_POPULATE_FIRST:
      {
        fprintf(stream,"Sparse Populate (First)\n");
      } break;
      case SPARSE_POPULATE_LAST:
      {
        fprintf(stream,"Sparse Populate (Last)\n");
      } break;
      case SPARSE_POPULATE_SUM:
      {
        fprintf(stream,"Sparse Populate (Sum)\n");
      } break;
      default:
      {
        fprintf(stream,"Unknown\n");
      } break;
    }
    fprintf(stream,"       dim:      %d x %d\n",
      matrix->num_row,matrix->num_col);
    fprintf(stream,"       n_size:   %d\n",matrix->n_size);
    fprintf(stream,"       num_elem: %d\n",matrix->num_elem);
    fprintf(stream,"       irow:     %p\n",matrix->irow);
    fprintf(stream,"       icol:     %p\n",matrix->icol);
    fprintf(stream,"       value:    %p\n",matrix->value);

    /* Print out the matrix */
    if (print_values) {

      switch (matrix->matrix_type)
      {
        /* Dense matrix -- just print out the matrix */
        case DENSE:
        {
          fprintf(stream,"   ");
          for (j=0;j<(matrix->num_col);j++) {
            fprintf(stream," %8d",j+1);
          }
          fprintf(stream,"\n");
          for (i=0;i<(matrix->num_row);i++) {
            fprintf(stream,"%2d:",i+1);
            for (ij=i;ij<(matrix->n_size);ij+=(matrix->num_row)) {
              fprintf(stream," %8.3f",(matrix->value[ij]));
            }
            fprintf(stream,"\n");
          }
          fprintf(stream,"\n");
        } break;

        /* Compressed-row matrix -- print storage pattern, then the matrix */
        case COMPRESSED_ROW:
        {
          for (i=0;i<(matrix->num_row)+1;i++) {
            fprintf(stream," %3d",matrix->irow[i]);
          }
          fprintf(stream,"\n");
          for (i=0;i<(matrix->n_size);i++) {
            fprintf(stream," %3d",matrix->icol[i]);
          }
          fprintf(stream,"\n");
          for (i=0;i<(matrix->n_size);i++) {
            fprintf(stream," %8.3f",matrix->value[i]);
          }
          fprintf(stream,"\n\n");


          fprintf(stream,"   ");
          for (j=0;j<(matrix->num_col);j++) {
            fprintf(stream," %8d",j+1);
          }
          fprintf(stream,"\n");
          for (i=0;i<(matrix->num_row);i++) {
            fprintf(stream,"%2d:",i+1);
            ij=(matrix->irow[i]);
            for (j=0;j<(matrix->num_col);j++) {
              if (((matrix->icol[ij])==j)&&(ij<(matrix->irow[i+1]))) {
                fprintf(stream," %8.3f",(matrix->value[ij]));
                ij++;
              }
              else {
                fprintf(stream," %8.3f",0.0);
              }
            }
            fprintf(stream,"\n");
          }
          fprintf(stream,"\n\n");


          fprintf(stream,"\n");
        } break;

        /* Compressed-row matrix -- print storage pattern, then the matrix */
        case SPARSE_POPULATE_FIRST:
        case SPARSE_POPULATE_LAST:
        case SPARSE_POPULATE_SUM:
        {
          for (i=0;i<(matrix->n_size);i++) {
            fprintf(stream," %3d",matrix->irow[i]);
          }
          fprintf(stream,"\n");
          for (i=0;i<(matrix->n_size);i++) {
            fprintf(stream," %3d",matrix->icol[i]);
          }
          fprintf(stream,"\n");
          for (i=0;i<(matrix->n_size);i++) {
            fprintf(stream," %8.3f",matrix->value[i]);
          }
          fprintf(stream,"\n\n");

          /* Allocate a dense matrix, fill then print out */
          num_size=(matrix->num_row)*(matrix->num_col);
          if (ALLOCATE(array,Matrix_value,num_size))
          {
            for (i=0;i<num_size;i++)
            {
              array[i]=0.0;
            }
            switch (matrix->matrix_type)
            {
              case SPARSE_POPULATE_FIRST:
              {
                for (i=(matrix->num_elem)-1;i>=0;i--)
                {
                  ij=(matrix->irow[i]-1)+(matrix->icol[i]-1)*(matrix->num_row);
                  array[ij]=matrix->value[i];
                }
              } break;

              case SPARSE_POPULATE_LAST:
              {
                for (i=0;i<(matrix->num_elem);i++)
                {
                  ij=(matrix->irow[i]-1)+(matrix->icol[i]-1)*(matrix->num_row);
                  array[ij]=matrix->value[i];
                }
              } break;

              case SPARSE_POPULATE_SUM:
              {
                for (i=0;i<(matrix->num_elem);i++)
                {
                  ij=(matrix->irow[i]-1)+(matrix->icol[i]-1)*(matrix->num_row);
                  array[ij]+=matrix->value[i];
                }
              } break;
            } /* switch (matrix->matrix_type) */

            fprintf(stream,"   ");
            for (j=0;j<(matrix->num_col);j++) {
              fprintf(stream," %8d",j+1);
            }
            fprintf(stream,"\n");
            for (i=0;i<(matrix->num_row);i++) {
              fprintf(stream,"%2d:",i+1);
              for (ij=i;ij<num_size;ij+=(matrix->num_row)) {
                fprintf(stream," %8.3f",array[ij]);
              }
              fprintf(stream,"\n");
            }
            fprintf(stream,"\n");

            DEALLOCATE(array);
          }
          else
          {
            display_message(ERROR_MESSAGE,"Matrix_print.  "
              "malloc failure. %d",num_size);
            return_code=0;
          }
        } break;
      } /* switch (matrix->matrix_type) */
    }

    fprintf(stream,"\n");
    return_code=1;
  }
  else
  {
    display_message(ERROR_MESSAGE,"Matrix_print.  "
      "Invalid argument(s).  %p %p",matrix,stream);
    return_code=0;
  }
  LEAVE;

  return (return_code);
} /* Matrix_print */
