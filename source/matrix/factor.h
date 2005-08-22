/*******************************************************************************
FILE : factor.h

LAST MODIFIED : 4 May 2003

DESCRIPTION :
Structures and functions for factorising Matricies, and solving linear systems.
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
