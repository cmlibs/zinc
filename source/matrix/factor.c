/*******************************************************************************
FILE : factor.c

LAST MODIFIED : 13 August 2003

DESCRIPTION :
Structures and functions for factorising Matricies, and solving linear systems.
==============================================================================*/

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


/*
Private functions
-----------------
*/
static struct Factor_Lapack_LU *CREATE(Factor_Lapack_LU)(int num_row,
  int num_col)
/*******************************************************************************
LAST MODIFIED : 23 April 2003

DESCRIPTION :
Creates a struct to contain the LAPACK LU factorisation data.
==============================================================================*/
{
  int array_size,pivot_size;
  Matrix_value *array;
  int          *pivot;
  struct Factor_Lapack_LU *factor_lapack_lu;

  ENTER(CREATE(Factor_Lapack_LU));
  factor_lapack_lu=(struct Factor_Lapack_LU *)NULL;
  if (num_col>0&&num_row>0)
  {
    if (ALLOCATE(factor_lapack_lu,struct Factor_Lapack_LU,1))
    {
      array_size=num_row*num_col;
      if (num_col<num_row)
      {
        pivot_size=num_row;
      }
      else
      {
        pivot_size=num_col;
      }

      if (ALLOCATE(array,Matrix_value,array_size)&&
          ALLOCATE(pivot,int,pivot_size))
      {
        factor_lapack_lu->num_row=num_row;
        factor_lapack_lu->num_col=num_col;
        factor_lapack_lu->array=array;
        factor_lapack_lu->pivot=pivot;
      }
      else
      {
        display_message(ERROR_MESSAGE,"CREATE(Factor_Lapack_LU).  "
          "Could not allocate array and pivot objects");
        DEALLOCATE(factor_lapack_lu);
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"CREATE(Factor_Lapack_LU).  "
        "Could not allocate factor_lapack_lu object");
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"CREATE(Factor_Lapack_LU).  "
      "Invalid argument(s).  %d %d",num_row,num_col);
  }
  LEAVE;

  return (factor_lapack_lu);
} /* CREATE(Factor_Lapack_LU) */


static int DESTROY(Factor_Lapack_LU)(struct Factor_Lapack_LU
  **factor_data_address)
/*******************************************************************************
LAST MODIFIED : 23 April 2003

DESCRIPTION :
Free memory/deaccess objects in LAPACK LU factorisation object at
<*factor_data_address>.
==============================================================================*/
{
  int return_code;
  struct Factor_Lapack_LU *factor_data;

  ENTER(DESTROY(Factor_Lapack_LU));
  return_code=0;
  if (factor_data_address&&(factor_data= *factor_data_address))
  {
    DEALLOCATE(factor_data->array);
    DEALLOCATE(factor_data->pivot);
    DEALLOCATE(factor_data);
    *factor_data_address=(struct Factor_Lapack_LU *)NULL;
    return_code=1;
  }
  LEAVE;

  return (return_code);
} /* DESTROY(Factor_Lapack_LU) */


static int Factor_Lapack_LU(void *factor_data,struct Matrix *matrix)
/*******************************************************************************
LAST MODIFIED : 23 April 2003

DESCRIPTION :
Factorises a dense linear system using LAPACK's LU decomposition routine,
==============================================================================*/
{
  int return_code = 0;
  int num_row,num_col,lda,stat,size,one;
  struct Factor_Lapack_LU *data;

  ENTER(Factor_Lapack_LU);
  if (factor_data&&matrix)
  {
    data=(struct Factor_Lapack_LU *)factor_data;

    /* Check that we have a dense matrix */
    if (DENSE==(matrix->matrix_type))
    {
      /* Check dimensions match */
      if ((matrix->num_row==data->num_row)&&(matrix->num_col==data->num_col))
      {
        num_row=matrix->num_row;
        num_col=matrix->num_col;
        lda=num_row;
        stat=0;
        size=num_row*num_col;
        one=1;

        /* Copy the array across */
        Dcopy(&size,matrix->value,&one,data->array,&one);

        /* Call LAPACK */
        Dgetrf(&num_row,&num_col,data->array,&lda,data->pivot,&stat);

        /* Check error code */
        if (stat<0)
        {
          display_message(ERROR_MESSAGE,"Factor_Lapack_LU.  "
            "Dgetrf's %dth argument was in error",-stat);
          return_code=0;
        }
        else if (stat>0)
        {
          display_message(ERROR_MESSAGE,"Factor_Lapack_LU.  "
            "Dgetrf's U(%d,%d) element is 0. The system is singular",stat,stat);
          return_code=0;
        }
        else /* stat==0 */
        {
          return_code=1;
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,"Factor_Lapack_LU.  "
          "Matrix and factor data dimensions don't match.  %dx%d != %dx%d",
          matrix->num_row,matrix->num_col,data->num_row,data->num_col);
        return_code=0;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"Factor_Lapack_LU.  "
        "Can only factor Dense matricies.  %d",matrix->matrix_type);
      return_code=0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Factor_Lapack_LU.  "
      "Invalid argument(s).  %p %p",factor_data,matrix);
    return_code=0;
  }
  LEAVE;

  return (return_code);
} /* Factor_Lapack_LU */


static int Matrix_solve_factored_Lapack_LU(struct Factor *factor,
  struct Matrix *b,struct Matrix *x)
/*******************************************************************************
LAST MODIFIED : 24 April 2003

DESCRIPTION :
Solve the system A<x> = <b>, using the Lapack LU factorisation of A stored in
<factor>.
==============================================================================*/
{
  int return_code;
  int num_row,num_col;
  int lda_rhs,num_rhs;
  int lda,stat;
  char *trans;
  unsigned int trans_len;
  struct Factor_Lapack_LU *data;

  ENTER(Matrix_solve_factored_Lapack_LU);
  if (factor&&b&&x)
  {
    data=(struct Factor_Lapack_LU *)factor->factor_data;
    num_row=data->num_row;
    num_col=data->num_col;
    lda=num_row;

    if (num_row==num_col)
    {
      if (Matrix_get_dimensions(b,&lda_rhs,&num_rhs))
      {
        if (num_row==lda_rhs)
        {
          if (Matrix_copy(b,x))
          {
            trans = "N";
            trans_len = strlen(trans);
            stat = 0;

            /* Solve using LAPACK */
            Dgetrs(trans,&num_row,&num_rhs,data->array,&lda,data->pivot,
                   x->value,&lda_rhs,&stat,trans_len);

            /* Check error code */
            if (stat<0)
            {
              display_message(ERROR_MESSAGE,"Factor_Lapack_LU.  "
                "Dgetrs's %dth argument was in error",-stat);
              return_code=0;
            }
            else /* stat==0 */
            {
              return_code=1;
            }
          }
          else
          {
            display_message(ERROR_MESSAGE,"Matrix_solve_factored_Lapack_LU.  "
              "Error copying matrix b into x");
            return_code=0;
          }
        }
        else
        {
          display_message(ERROR_MESSAGE,"Matrix_solve_factored_Lapack_LU.  "
            "Array dimensions don't match the length of the RHS");
          return_code=0;
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,"Matrix_solve_factored_Lapack_LU.  "
          "Array is not square; %d != %d",num_row,num_col);
        return_code=0;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"Matrix_solve_factored_Lapack_LU.  "
        "Error geting the matrix dimensions");
      return_code=0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Matrix_solve_factored_Lapack_LU.  "
      "Invalid argument(s).  %p %p %p %p",factor,x,b);
    return_code=0;
  }
  LEAVE;

  return (return_code);
} /* Matrix_solve_factored_Lapack_LU */


static void *CREATE(Factor_data)(int num_row,int num_col,
  enum Factor_type factor_type)
/*******************************************************************************
LAST MODIFIED : 23 April 2003

DESCRIPTION :
Creates a struct to contain the factorisation data of type <factor_type>.
==============================================================================*/
{
  void *factor_data;

  ENTER(CREATE(Factor_data));
  factor_data=NULL;
  switch (factor_type)
  {
    case LAPACK_LU:
    {
      factor_data=(void *)CREATE(Factor_Lapack_LU)(num_row,num_col);
    } break;

     default:
    {
      display_message(ERROR_MESSAGE,"CREATE(Factor_data).  "
        "Unknown factorisation type %d",factor_type);
      factor_data=NULL;
    } break;
  }
  LEAVE;

  return (factor_data);
} /* CREATE(Factor_data) */


static int DESTROY(Factor_data)(void **factor_data_address,enum Factor_type 
  factor_type)
/*******************************************************************************
LAST MODIFIED : 23 April 2003

DESCRIPTION :
Free memory/deaccess objects in the factorisation object pointed to by the
<factor_data> address within the <*factor_address> object.
==============================================================================*/
{
  int return_code;
  void *factor_data;

  ENTER(DESTROY(Factor_data));
  return_code=0;
  if (factor_data_address&&(factor_data= *factor_data_address))
  {
    /* Deallocate storage for factor data structure */
    switch (factor_type)
    {
      case LAPACK_LU:
      {
        DESTROY(Factor_Lapack_LU)((struct Factor_Lapack_LU **)&(factor_data));
        return_code=1;
      } break;

      default:
      {
        display_message(ERROR_MESSAGE,"DESTROY(Factor_data).  "
          "Unknown factorisation type %d",factor_type);
        return_code=0;
      } break;
    } /* switch (factor_type) */
  }
  else
  {
    display_message(ERROR_MESSAGE,"DESTROY(Factor_data).  "
      "Invalid argument.  %p",factor_data_address);
  }
  LEAVE;

  return (return_code);
} /* DESTROY(Factor_data) */


static int Matrix_factor_private(void *factor_data,struct Matrix *matrix,
  enum Factor_type factor_type)
/*******************************************************************************
LAST MODIFIED : 23 April 2003

DESCRIPTION :
Factorise <matrix> by the <factor_type> method, storing the data in the
previously allocated <factor_data> which must be of the same type as 
<factor_type>.
==============================================================================*/
{
  int return_code;

  ENTER(Matrix_factor_private);
  if (factor_data&&matrix)
  {
    switch (factor_type)
    {
      case LAPACK_LU:
      {
        return_code=Factor_Lapack_LU(factor_data,matrix);
      } break;

      default:
      {
        display_message(ERROR_MESSAGE,"Matrix_factor_private.  "
          "Unknown factorisation type %d",factor_type);
        return_code=0;
      } break;
      
    } /* switch (factor_type) */
  }
  else
  {
    display_message(ERROR_MESSAGE,"Matrix_factor_private.  "
      "Invalid argument(s).  %p %p",factor_data,matrix);
    return_code=0;
  }
  LEAVE;

  return (return_code);
} /* Matrix_factor_private */


static int Matrix_solve_factored_private(struct Factor *factor,struct Matrix *b,
  struct Matrix *x)
/*******************************************************************************
LAST MODIFIED : 24 April 2003

DESCRIPTION :
==============================================================================*/
{
  int return_code;

  ENTER(Matrix_solve_factored_private);
  if (factor&&b&&x)
  {
    switch (factor->factor_type)
    {
      case LAPACK_LU:
      {
        return_code=Matrix_solve_factored_Lapack_LU(factor,b,x);
      } break;

      default:
      {
        display_message(ERROR_MESSAGE,"Matrix_solve_factored_private.  "
          "Unknown factorisation type %d",factor->factor_type);
        return_code=0;
      } break;
      
    } /* switch (factor->factor_type) */
  }
  else
  {
    display_message(ERROR_MESSAGE,"Matrix_solve_factored_private.  "
      "Invalid argument(s).  %p %p %p",factor,b,x);
    return_code=0;
  }
  LEAVE;

  return (return_code);
} /* Matrix_solve_factored_private */


/*
Public functions
----------------
*/
struct Factor *CREATE(Factor)(void)
/*******************************************************************************
LAST MODIFIED : 23 April 2003

DESCRIPTION :
Creates an empty factorisation object.
==============================================================================*/
{
  struct Factor *factor;

  ENTER(CREATE(Factor));
  factor=(struct Factor *)NULL;
  if (ALLOCATE(factor,struct Factor,1))
  {
    factor->factor_type=NONE;
    factor->factor_data=NULL;
  }
  else
  {
    display_message(ERROR_MESSAGE,"CREATE(Factor).  "
      "Could not allocate factor object");
  }
  LEAVE;

  return (factor);
} /* CREATE(Factor) */


int DESTROY(Factor)(struct Factor **factor_address)
/*******************************************************************************
LAST MODIFIED : 23 April 2003

DESCRIPTION :
Free memory/deaccess objects in factorisation object at <*factor_address>.
==============================================================================*/
{
  int return_code;
  struct Factor *factor;
  void *factor_data;

  ENTER(DESTROY(Factor));
  return_code=0;
  if (factor_address&&(factor= *factor_address))
  {
    /* Deallocate storage for factor data structure */
    if (factor->factor_data) {
      factor_data=factor->factor_data;
      DESTROY(Factor_data)(&factor_data,factor->factor_type);
    }
    DEALLOCATE(factor);
    *factor_address=(struct Factor *)NULL;
    return_code=1;
  }
  LEAVE;

  return (return_code);
} /* DESTROY(Factor) */


int Matrix_factorise(struct Factor *factor,struct Matrix *matrix,
  enum Factor_type factor_type)
/*******************************************************************************
LAST MODIFIED : 23 April 2003

DESCRIPTION :
Factorises <matrix> using the method of <factor_type>, returning the 
factorisation data in the previously created <factor>.
==============================================================================*/
{
  int num_row,num_col,return_code;
  void *factor_data;

  ENTER(Matrix_factorise);
  if (factor&&matrix)
  {
    /* Clear any data in the factorisation object */
    if (factor->factor_data)
    {
      factor_data=factor->factor_data;
      DESTROY(Factor_data)(&factor_data,factor->factor_type);
      factor->factor_data=NULL;
      factor->factor_type=NONE;
      factor_data=NULL;
    }

    /* Size of matrix */
    if (Matrix_get_dimensions(matrix,&num_row,&num_col))
    {
      /* Allocate the factorisation data */
      if (factor_data=CREATE(Factor_data)(num_row,num_col,factor_type))
      {
        /* Factorise */
        if (Matrix_factor_private(factor_data,matrix,factor_type))
        {
          factor->factor_type=factor_type;
          factor->factor_data=factor_data;
          return_code=1;
        }
        else
        {
          display_message(ERROR_MESSAGE,"Matrix_factorise.  "
            "Unable to factorise system");
          DESTROY(Factor_data)(&factor_data,factor_type);
          return_code=0;
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,"Matrix_factorise.  "
          "Unable to create factor data object");
        return_code=0;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"Matrix_factorise.  "
        "Unable to determine matrix size");
      return_code=0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Matrix_factorise.  "
      "Invalid argument(s).  %p %p",factor,matrix);
    return_code=0;
  }
  LEAVE;

  return (return_code);
} /* Matrix_factorise */


int Matrix_solve_factored(struct Factor *factor,struct Matrix *A,
  struct Matrix *b,struct Matrix *x)
/*******************************************************************************
LAST MODIFIED : 4 May 2003

DESCRIPTION :
Solve the system <A><x> = <b>, using the previously generated factorisation
of <A> stored in <factor>.
==============================================================================*/
{
  int return_code;
  enum Matrix_type matrix_type;

  ENTER(Matrix_solve_factored);
  if (factor&&A&&b&&x)
  {
    if ((NONE!=factor->factor_type)&&(NULL!=factor->factor_data))
    {
      if (Matrix_get_type(b,&matrix_type))
      {
        if (DENSE==matrix_type)
        {
          return_code=Matrix_solve_factored_private(factor,b,x);
        }
        else
        {
          display_message(ERROR_MESSAGE,"Matrix_solve_factored.  "
            "The RHS matrix (b) must be a DENSE matrix");
          return_code=0;
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,"Matrix_solve_factored.  "
          "Error determining matrix type for b");
        return_code=0;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"Matrix_solve_factored.  "
        "The Factor struct contains no data  %d %p",
        factor->factor_type,factor->factor_data);
      return_code=0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Matrix_solve_factored.  "
      "Invalid argument(s).  %p %p %p %p",factor,A,x,b);
    return_code=0;
  }
  LEAVE;

  return (return_code);
} /* Matrix_solve_factored */


int Matrix_check_soln(struct Matrix *A,struct Matrix *b,struct Matrix *x,
  FILE *stream)
/*******************************************************************************
LAST MODIFIED : 2 May 2003

DESCRIPTION :
calculate the residual of the solved system <A><x> = <b>, and print out the
residual and the norms of the various arrays.
==============================================================================*/
{
  int return_code;
  int size_x,size_b;
  struct Matrix *r;
  Matrix_value Anorm,xnorm,bnorm,rnorm,rnorm_scaled;

  ENTER(Matrix_check_soln);
  if (A&&b&&x&&stream)
  {
    if ((A->matrix_type==DENSE)&&
        (x->matrix_type==DENSE)&&
        (b->matrix_type==DENSE))
    {
      size_b=(b->num_row)*(b->num_col);
      size_x=(x->num_row)*(x->num_col);

      if ((A->num_col==size_x)&&(A->num_row==size_b))
      {
        if (r=CREATE(Matrix)("r",DENSE,size_b,1))
        {
          if (Matrix_copy(b,r)&&Matrix_matvec(A,x,r,-1.0,1.0))
          {
            if (Matrix_norm2(A,&Anorm)&&Matrix_norm2(b,&bnorm)&&
                Matrix_norm2(x,&xnorm)&&Matrix_norm2(r,&rnorm))
            {
              rnorm_scaled=Anorm*xnorm+bnorm;
              if (rnorm_scaled!=0.0)
              {
                rnorm_scaled=rnorm/rnorm_scaled;
              }
              fprintf(stream,"\n");
              fprintf(stream,"  Accuracy of Solution\n");
              fprintf(stream,"\n");
              fprintf(stream,"  Residual:        %g\n",rnorm);
              fprintf(stream,"  Scaled residual: %g\n",rnorm_scaled);
              fprintf(stream,"  Norm of A:       %g\n",Anorm);
              fprintf(stream,"  Norm of b:       %g\n",bnorm);
              fprintf(stream,"  Norm of x:       %g\n",xnorm);
              fprintf(stream,"\n");
              return_code=1;
            }
            else
            {
              display_message(ERROR_MESSAGE,"Matrix_check_soln.  "
                "Error calculating the matrix norms");
              return_code=0;
            }
          }
          else
          {
            display_message(ERROR_MESSAGE,"Matrix_check_soln.  "
              "Error calculating the residual");
            return_code=0;
          }
          DESTROY(Matrix)(&r);
        }
        else
        {
          display_message(ERROR_MESSAGE,"Matrix_check_soln.  "
            "Error allocating the residual vector r");
          return_code=0;
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,"Matrix_check_soln.  "
          "Array and vector dimensions do not match");
        return_code=0;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"Matrix_check_soln.  "
        "Only works for DENSE matricies at the moment");
      return_code=0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Matrix_check_soln.  "
      "Invalid argument(s).  %p %p %p %p",A,x,b,stream);
    return_code=0;
  }
  LEAVE;

  return (return_code);
} /* Matrix_check_soln */


int Matrix_solve(struct Matrix *A,struct Matrix *b,struct Matrix *x,
  enum Factor_type factor_type,FILE *stream)
/*******************************************************************************
LAST MODIFIED : 30 July 2003

DESCRIPTION :
Solve the system <A><x> = <b>, using the solver <factor_type>. If <stream> is
not null, the esidual of the system  is calculated and printed to that stream.
==============================================================================*/
{
  int return_code;
  struct Factor *factor;

  ENTER(Matrix_solve);
	return_code=0;
  if (A&&b&&x)
  {
    if (factor=CREATE(Factor)())
    {
      if (Matrix_factorise(factor,A,factor_type))
      {
        if (Matrix_solve_factored(factor,A,b,x))
        {
          if (stream)
          {
            if (Matrix_check_soln(A,b,x,stream))
            {
              return_code=1;
            }
            else
            {
              display_message(ERROR_MESSAGE,"Matrix_solve.  "
                "Error checking the solution");
              return_code=0;
            }
          }
          else
          {
            return_code=1;
          }
        }
        else
        {
          display_message(ERROR_MESSAGE,"Matrix_solve.  "
            "Error solving the factored system");
          return_code=0;
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,"Matrix_solve.  "
          "Error factorising system");
        return_code=0;
      }
      DESTROY(Factor)(&factor);
    }
    else
    {
      display_message(ERROR_MESSAGE,"Matrix_solve.  "
        "Error creating factor object");
      return_code=0;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"Matrix_solve.  "
      "Invalid argument(s).  %p %p %p %p",A,x,b,stream);
    return_code=0;
  }
  LEAVE;

  return (return_code);
} /* Matrix_solve */
