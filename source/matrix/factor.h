/*******************************************************************************
FILE : factor.h

LAST MODIFIED : 4 May 2003

DESCRIPTION :
Structures and functions for factorising Matricies, and solving linear systems.
==============================================================================*/

#include <stdlib.h>
#include <stdio.h>
#include "matrix/matrix.h"
#include "matrix/matrix_private.h"

struct Factor *CREATE(Factor)(void);
/*******************************************************************************
LAST MODIFIED : 23 April 2003

DESCRIPTION :
Creates an empty factorisation object.
==============================================================================*/

int DESTROY(Factor)(struct Factor **factor_address);
/*******************************************************************************
LAST MODIFIED : 23 April 2003

DESCRIPTION :
Free memory/deaccess objects in factorisation object at <*factor_address>.
==============================================================================*/

int Matrix_factorise(struct Factor *factor,struct Matrix *matrix,
  enum Factor_type factor_type);
/*******************************************************************************
LAST MODIFIED : 23 April 2003

DESCRIPTION :
Factorises <matrix> using the method of <factor_type>, returning the 
factorisation data in the previously created <factor>.
==============================================================================*/

int Matrix_solve_factored(struct Factor *factor,struct Matrix *A,
  struct Matrix *b,struct Matrix *x);
/*******************************************************************************
LAST MODIFIED : 24 April 2003

DESCRIPTION :
Solve the system <A><x> = <b>, using the previously generated factorisation
of <A> stored in <factor>.
==============================================================================*/

int Matrix_check_soln(struct Matrix *A,struct Matrix *b,struct Matrix *x,
  FILE *stream);
/*******************************************************************************
LAST MODIFIED : 2 May 2003

DESCRIPTION :
calculate the residual of the solved system <A><x> = <b>, and print out the
residual and the norms of the various arrays.
==============================================================================*/

int Matrix_solve(struct Matrix *A,struct Matrix *b,struct Matrix *x,
  enum Factor_type factor_type,FILE *stream);
/*******************************************************************************
LAST MODIFIED : 2 May 2003

DESCRIPTION :
Solve the system <A><x> = <b>, using the solver <factor_type>. If <stream> is
not null, the esidual of the system  is calculated and printed to that stream.
==============================================================================*/
