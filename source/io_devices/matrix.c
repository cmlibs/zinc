/*******************************************************************************
FILE : matrix.c

LAST MODIFIED : 6 January 1998

DESCRIPTION :
Contains routines equivalent in function to those from GL.
==============================================================================*/
#include <string.h>
/* for memcpy() */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "general/debug.h"
#include "io_devices/matrix.h"
#include "io_devices/conversion.h"

void matrix_print(Gmatrix *current)
/*******************************************************************************
LAST MODIFIED : 1 December 1994

DESCRIPTION :
Used for debugging to view a matrix.
==============================================================================*/
{
	int i,j;

	ENTER(matrix_print);
	for (i=0;i<GMATRIX_SIZE;i++)
	{
		for (j=0;j<GMATRIX_SIZE;j++)
		{
			printf("% lf ",current->data[i][j]);
		}
		printf("\n");
	}
	LEAVE;
} /* matrix_print */


void matrix_I(Gmatrix *current)
/*******************************************************************************
LAST MODIFIED : 18 July 1994

DESCRIPTION :
Turns the matrix into the identity
==============================================================================*/
{
	int i,j;

	ENTER(matrix_I);
	for (i=0;i<GMATRIX_SIZE;i++)
	{
		for (j=0;j<GMATRIX_SIZE;j++)
		{
			if (j==i)
			{
				current->data[i][j] = 1.0;
			}
			else
			{
				current->data[i][j] = 0.0;
			}
		}
	}
	LEAVE;
} /* matrix_I */


void matrix_inverse(Gmatrix *current,Gmatrix *inverse)
/*******************************************************************************
LAST MODIFIED : 4 November 1994

DESCRIPTION :
Returns the inverse of current in inverse.
==============================================================================*/
{
	GMATRIX_PRECISION determinant;

	ENTER(matrix_I);
	/* GMH hack to get 2d inverse working */
	/* check for 2d */
	if((current->data[2][2]==1.0)&&
		(current->data[0][2]==0.0)&&(current->data[2][0]==0.0)&&
		(current->data[1][2]==0.0)&&(current->data[2][1]==0.0))
	{
		/* 2d matrix */
		matrix_I(inverse);
		determinant = current->data[0][0]*current->data[1][1]-
			current->data[1][0]*current->data[0][1];
		inverse->data[0][0] = current->data[1][1]/determinant;
		inverse->data[1][1] = current->data[0][0]/determinant;
		inverse->data[1][0] = -current->data[1][0]/determinant;
		inverse->data[0][1] = -current->data[0][1]/determinant;
	}
	else
	{
		printf("???TEMP matrix_inverse is not implemented yet!");
	}
	LEAVE;
} /* matrix_inverse */


void matrix_copy(Gmatrix *dest,Gmatrix *source)
/*******************************************************************************
LAST MODIFIED : 21 July 1994

DESCRIPTION :
Copies source to destination.
==============================================================================*/
{
	ENTER(matrix_copy);
	memcpy(dest,source,sizeof(Gmatrix));
	LEAVE;
} /* matrix_copy */


void matrix_copy_transpose(Gmatrix *dest,Gmatrix *source)
/*******************************************************************************
LAST MODIFIED : 2 December 1994

DESCRIPTION :
Copies the transpose of source to destination.
==============================================================================*/
{
	int i,j;

	ENTER(matrix_copy_transpose);
	for(i=0;i<GMATRIX_SIZE;i++)
	{
		for(j=0;j<GMATRIX_SIZE;j++)
		{
			dest->data[i][j] = source->data[j][i];
		}
	}
	LEAVE;
} /* matrix_copy */


void matrix_vector_unit(GMATRIX_PRECISION *vector)
/*******************************************************************************
LAST MODIFIED : 02 April 1995

DESCRIPTION :
Changes the vector to unit length.
==============================================================================*/
{
	int i;
	GMATRIX_PRECISION sum;

	ENTER(matrix_vector_unit);
	sum = 0.0;
	for(i=0;i<3;i++)
	{
		sum += vector[i]*vector[i];
	}
	sum = sqrt(sum);
	if (sum>0)
	{
		for(i=0;i<3;i++)
		{
			vector[i] = vector[i]/sum;
		}
	}
	LEAVE;
} /* matrix_vector_unit */


void matrix_premult(Gmatrix *current,Gmatrix *pre_matrix)
/*******************************************************************************
LAST MODIFIED : 18 July 1994

DESCRIPTION :
Premultiplies current by pre_matrix.
==============================================================================*/
{
	int i,j,k;
	Gmatrix temp;

	ENTER(matrix_premult);
	matrix_copy(&temp,current);
	for (i=0;i<GMATRIX_SIZE;i++)
	{
		for (j=0;j<GMATRIX_SIZE;j++)
		{
			current->data[i][j] = 0.0;
			for (k=0;k<GMATRIX_SIZE;k++)
			{
				current->data[i][j] += pre_matrix->data[i][k]*temp.data[k][j];
			}
		}
	}
	LEAVE;
} /* matrix_premult */


void matrix_postmult(Gmatrix *current,Gmatrix *post_matrix)
/*******************************************************************************
LAST MODIFIED : 2 December 1994

DESCRIPTION :
Postmultiplies current by post_matrix.
==============================================================================*/
{
	int i,j,k;
	Gmatrix temp;

	ENTER(matrix_postmult);
	matrix_copy(&temp,current);
	for (i=0;i<GMATRIX_SIZE;i++)
	{
		for (j=0;j<GMATRIX_SIZE;j++)
		{
			current->data[i][j] = 0.0;
			for (k=0;k<GMATRIX_SIZE;k++)
			{
				current->data[i][j] += temp.data[i][k]*post_matrix->data[k][j];
			}
		}
	}
	LEAVE;
} /* matrix_premult */


void matrix_premult_vector(GMATRIX_PRECISION *current,Gmatrix *pre_matrix)
/*******************************************************************************
LAST MODIFIED : 18 July 1994

DESCRIPTION :
Premultiplies current by pre_matrix.
==============================================================================*/
{
	int i,j;
	GMATRIX_PRECISION temp[GMATRIX_SIZE];

	ENTER(matrix_premult_vector);
	for (i=0;i<GMATRIX_SIZE;i++)
	{
		temp[i] = current[i];
	}
	for (i=0;i<GMATRIX_SIZE;i++)
	{
		current[i] = 0.0;
		for (j=0;j<GMATRIX_SIZE;j++)
		{
			current[i] += pre_matrix->data[i][j]*temp[j];
		}
	}
	LEAVE;
} /* matrix_premult_vector */


void matrix_postmult_vector(GMATRIX_PRECISION *current,Gmatrix *post_matrix)
/*******************************************************************************
LAST MODIFIED : 2 December 1994

DESCRIPTION :
Postmultiplies current by post_matrix.
==============================================================================*/
{
	int i,j;
	GMATRIX_PRECISION temp[GMATRIX_SIZE];

	ENTER(matrix_postmult_vector);
	for (i=0;i<GMATRIX_SIZE;i++)
	{
		temp[i] = current[i];
	}
	for (i=0;i<GMATRIX_SIZE;i++)
	{
		current[i] = 0.0;
		for (j=0;j<GMATRIX_SIZE;j++)
		{
			current[i] += temp[j]*post_matrix->data[j][i];
		}
	}
	LEAVE;
} /* matrix_postmult_vector */


void matrix_mult(Gmatrix *pre_matrix,Gmatrix *post_matrix,Gmatrix *new_matrix)
/*******************************************************************************
LAST MODIFIED : 2 December 1994

DESCRIPTION :
Multiplies pre_matrix by post_matrix, and then returns the answer in new_matrix.
==============================================================================*/
{
	int i,j,k;

	ENTER(matrix_mult);
	for (i=0;i<GMATRIX_SIZE;i++)
	{
		for (j=0;j<GMATRIX_SIZE;j++)
		{
			new_matrix->data[i][j] = 0.0;
			for (k=0;k<GMATRIX_SIZE;k++)
			{
				new_matrix->data[i][j] += pre_matrix->data[i][k]*post_matrix->data[k][j];
			}
		}
	}
	LEAVE;
} /* matrix_mult */


void matrix_rotate(Gmatrix *current,double angle,char axis)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Rotates in a right hand sense about the axis.
==============================================================================*/
{
	double cos_angle,sin_angle,new_angle;
	Gmatrix temp;

	ENTER(matrix_rotate);
	new_angle = angle*PI_180;
	sin_angle = sin(new_angle);
	cos_angle = cos(new_angle);
	matrix_I(&temp);
	switch (axis)
	{
		case 'x':
		case 'X':
		{
			temp.data[1][1] = cos_angle;
			temp.data[2][2] = cos_angle;
			temp.data[2][1] = sin_angle;
			temp.data[1][2] = -sin_angle;
		}; break;
		case 'y':
		case 'Y':
		{
			temp.data[0][0] = cos_angle;
			temp.data[2][2] = cos_angle;
			temp.data[2][0] = -sin_angle;
			temp.data[0][2] = sin_angle;
		}; break;
		case 'z':
		case 'Z':
		{
			temp.data[0][0] = cos_angle;
			temp.data[1][1] = cos_angle;
			temp.data[1][0] = sin_angle;
			temp.data[0][1] = -sin_angle;
		}; break;
	};
	matrix_premult(current,&temp);
	LEAVE;
} /* matrix_rotate */
