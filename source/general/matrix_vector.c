/*******************************************************************************
FILE : matrix_vector.c

LAST MODIFIED : 7 November 2000

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
vector or matrix routines that should be in here are:
graphics/mcubes.c
graphics/matrix.c
==============================================================================*/
#include <math.h>
#include <stdio.h>
#include "general/debug.h"
#include "general/matrix_vector.h"
#include "user_interface/message.h"

#define TINY 1.0e-20

const double identity_matrix4[16]=
{
	1.0, 0.0, 0.0, 0.0,
	0.0, 1.0, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.0, 0.0, 0.0, 1.0
};

int cross_product3(double *a,double *b,double *result)
/*******************************************************************************
LAST MODIFIED : 28 January 1998

DESCRIPTION :
Returns in <result> the cross product of the two vectors = a (x) b.
==============================================================================*/
{
	int return_code;

	ENTER(cross_product3);
	if (a&&b&&result)
	{
		result[0] = a[1]*b[2] - a[2]*b[1];
		result[1] = a[2]*b[0] - a[0]*b[2];
		result[2] = a[0]*b[1] - a[1]*b[0];
	}
	else
	{
		display_message(ERROR_MESSAGE,"cross_product3.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* cross_product3 */

double dot_product3(double *a,double *b)
/*******************************************************************************
LAST MODIFIED : 28 January 1998

DESCRIPTION :
Returns the dot product of the two vectors = a.b.
==============================================================================*/
{
	double dot_product;

	ENTER(dot_product3);
	if (a&&b)
	{
		dot_product = a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
	}
	else
	{
		display_message(ERROR_MESSAGE,"dot_product3.  Invalid argument(s)");
		dot_product=0.0;
	}
	LEAVE;

	return (dot_product);
} /* dot_product3 */

double norm3(double *v)
/*******************************************************************************
LAST MODIFIED : 28 January 1998

DESCRIPTION :
Returns the norm/length/magnitude of vector <v>.
==============================================================================*/
{
	double norm;

	ENTER(norm3);
	if (v)
	{
		norm=sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
	}
	else
	{
		display_message(ERROR_MESSAGE,"norm3.  Missing vector");
		norm=0;
	}
	LEAVE;

	return (norm);
} /* norm3 */

double normalize3(double *v)
/*******************************************************************************
LAST MODIFIED : 26 November 1997

DESCRIPTION :
Normalizes vector v - of dimension 3 - and returns its length. If its length
is zero, v is converted to unit vector [1,0,0].
==============================================================================*/
{
	double length;

	ENTER(normalize_vector3);
	if (v)
	{
		if (0<(length=sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2])))
		{
			v[0] /= length;
			v[1] /= length;
			v[2] /= length;
		}
		else
		{
			v[0]=1.0;
			v[1]=0.0;
			v[2]=0.0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"normalize3.  Missing vector");
		length=0.0;
	}
	LEAVE;

	return (length);
} /* normalize_vector3 */

int normalize_float3(float vector[3])
/*******************************************************************************
LAST MODIFIED : 27 December 1995

DESCRIPTION :
Normalizes the given <vector>.
==============================================================================*/
{
	float norm;
	int return_code;

	ENTER(normalize);
	if (vector)
	{
		if ((norm=vector[0]*vector[0]+vector[1]*vector[1]+vector[2]*vector[2])>0)
		{
			norm=(float)sqrt((double)norm);
			vector[0] /= norm;
			vector[1] /= norm;
			vector[2] /= norm;
		}
		return_code=1;
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* normalize_float3 */

int normalized_crossproduct_float3(float vector_1[3],float vector_2[3],
	float result[3])
/*******************************************************************************
LAST MODIFIED : 28 December 1995

DESCRIPTION :
Calculates the normalized cross product of <vector_1> and <vector_2> and puts
it in <result>.
==============================================================================*/
{
	int return_code;

	ENTER(normalized_crossproduct);
	if (vector_1&&vector_2&&result)
	{
		result[0]=vector_1[1]*vector_2[2] - vector_2[1]*vector_1[2];
		result[1]=vector_1[2]*vector_2[0] - vector_2[2]*vector_1[0];
		result[2]=vector_1[0]*vector_2[1] - vector_1[1]*vector_2[0];
		return_code=normalize_float3(result);
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* normalized_crossproduct_float3 */

int crossproduct_float3(float vector_1[3],float vector_2[3],
	float result[3])
/*******************************************************************************
LAST MODIFIED : 22 September 1999

DESCRIPTION :
Calculates the normalized cross product of <vector_1> and <vector_2> and puts
it in <result>.
==============================================================================*/
{
	int return_code;

	ENTER(crossproduct_float3);
	if (vector_1&&vector_2&&result)
	{
		result[0]=vector_1[1]*vector_2[2] - vector_2[1]*vector_1[2];
		result[1]=vector_1[2]*vector_2[0] - vector_2[2]*vector_1[0];
		result[2]=vector_1[0]*vector_2[1] - vector_1[1]*vector_2[0];
		return_code=1;
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* crossproduct_float3 */

double scalar_triple_product3(double *a,double *b,double *c)
/*******************************************************************************
LAST MODIFIED : 28 January 1998

DESCRIPTION :
Returns the scalar triple product, [abc]=a.(bxc) of vectors a,b and c.
==============================================================================*/
{
	double triple_product;

	ENTER(scalar_triple_product3);
	if (a&&b&&c)
	{
		triple_product =
			(a[0]*(b[1]*c[2] - b[2]*c[1])
			+a[1]*(b[2]*c[0] - b[0]*c[2])
			+a[2]*(b[0]*c[1] - b[1]*c[0]));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"scalar_triple_product3.  Invalid argument(s)");
		triple_product=0.0;
	}
	LEAVE;

	return (triple_product);
} /* scalar_triple_product3 */

int copy_matrix(int m,int n,double *a,double *a_copy)
/*******************************************************************************
LAST MODIFIED : 28 January 1998

DESCRIPTION :
Copies the contents of m rows x n columns matrix <a> to <a_copy>.
==============================================================================*/
{
	int i,size,return_code;

	ENTER(copy_matrix);
	if ((0<m)&&(0<n)&&a&&a_copy)
	{
		size=m*n;
		for (i=0;i<size;i++)
		{
			a_copy[i]=a[i];
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"copy_matrix.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* copy_matrix */

int multiply_matrix(int m,int s,int n,double *a,double *b,double *c)
/*******************************************************************************
LAST MODIFIED : 28 January 1998

DESCRIPTION :
Returns in the m rows x n columns matrix <c> that is the product of
m x s matrix <a> and s x n matrix <b>.
==============================================================================*/
{
	double sum;
	int i,j,k,return_code;

	ENTER(multiply_matrix);
	if ((0<m)&&(0<s)&&(0<n)&&a&&b&&c)
	{
		for (i=0;i<m;i++)
		{
			for (j=0;j<n;j++)
			{
				sum=0.0;
				for (k=0;k<s;k++)
				{
					sum += a[i*s+k] * b[k*n+j];
				}
				c[i*n+j]=sum;
			}
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"multiply_matrix.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* multiply_matrix */


int multiply_matrix_float(int m,int s,int n,float *a,float *b,float *c)
/*******************************************************************************
LAST MODIFIED : 2 February 1998

DESCRIPTION :
Returns in the m rows x n columns matrix <c> that is the product of
m x s matrix <a> and s x n matrix <b>.
Uses matrices of floats, not doubles. For matrices od doubles, use
"multiply_matrix()"
==============================================================================*/
{
	float sum;
	int i,j,k,return_code;

	ENTER(multiply_matrix_float);
	if ((0<m)&&(0<s)&&(0<n)&&a&&b&&c)
	{
		for (i=0;i<m;i++)
		{
			for (j=0;j<n;j++)
			{
				sum=(float)0.0;
				for (k=0;k<s;k++)
				{
					sum += a[i*s+k] * b[k*n+j];
				}
				c[i*n+j]=sum;
			}
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"multiply_matrix_float.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* multiply_matrix_float */

int print_matrix(int m,int n,double *a,char *number_format)
/*******************************************************************************
LAST MODIFIED : 28 January 1998

DESCRIPTION :
Prints out the contents of m rows x n columns matrix <a>.
==============================================================================*/
{
	int i,j,return_code;

	ENTER(print_matrix);
	if ((0<m)&&(0<n)&&a&&number_format)
	{
		for (i=0;i<m;i++)
		{
			printf("|");
			for (j=0;j<n;++j)
			{
				printf(number_format,a[i*n+j]);
			}
			printf(" |\n");
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"print_matrix.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* print_matrix */

int transpose_matrix(int m,int n,double *a,double *a_transpose)
/*******************************************************************************
LAST MODIFIED : 28 January 1998

DESCRIPTION :
Return the transpose of m rows x n columns matrix <a> in <a_transpose> - which
will then be n rows x m columns.
==============================================================================*/
{
	int i,j,return_code;

	ENTER(transpose_matrix);
	if ((0<m)&&(0<n)&&a&&a_transpose)
	{
		for (i=0;i<m;i++)
		{
			for (j=0;j<n;j++)
			{
				a_transpose[j*m+i] = a[i*n+j];
			}
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"transpose_matrix.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* transpose_matrix */

int matrix_is_symmetric(int n, double *a, double factor)
/*******************************************************************************
LAST MODIFIED : 7 November 2000

DESCRIPTION :
Returns true if n x n matrix <a> is symmetric about the main diagonal by
checking that the difference between all opposing off-diagonal values is less
<factor> times the largest [absolute] value in <a>.
==============================================================================*/
{
	double allowed_difference;
	int i, j, matrix_size, return_code;

	ENTER(matrix_is_symmetric);
	if ((0<n) && a && (0.0 <= factor) && (1.0 >= factor))
	{
		return_code = 1;
		/* get the largest element in <a> */
		matrix_size = n * n;
		allowed_difference = 0.0;
		for (i = 0; i < matrix_size; i++)
		{
			if (fabs(a[i]) > allowed_difference)
			{
				allowed_difference = fabs(a[i]);
			}
		}
		allowed_difference *= factor;
		for (i = 0; (i < (n-1)) && return_code; i++)
		{
			for (j = i + 1; (j < n) && return_code; j++)
			{
				if (fabs(a[i*n + j] - a[j*n + i]) > allowed_difference)
				{
					return_code=0;
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"matrix_is_symmetric.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* matrix_is_symmetric */

int LU_decompose(int n,double *a,int *indx,double *d)
/*******************************************************************************
LAST MODIFIED : 25 July 1998

DESCRIPTION :
Performs LU decomposition on n x n matrix <a>. <indx> should be a preallocated
integer array of dimension n. On return, <a> contains the lower and upper
decomposed matrix such that a = LU. The indx array contains information about
the partial pivoting performed during decomposition, while the sign of *d
indicates whether an even =+ve, or odd =-ve number of row interchanges have been
performed - I think ;-).
Adapted from "Numerical Recipes in C".
==============================================================================*/
{
	int i,imax,j,k,return_code;
	double big,dum,sum,temp,*vv;

	ENTER(LU_decompose);
	if ((0<n)&&a&&indx&&d)
	{
		if (ALLOCATE(vv,double,n))
		{
			return_code=1;
			*d=1.0;
			/* get largest value in each row for partial pivoting */
			for (i=0;i<n;i++)
			{
				big=0.0;
				for (j=0;j<n;j++)
				{
					if ((temp=fabs(a[i*n+j])) > big)
					{
						big=temp;
					}
				}
				if (0.0<big)
				{
					vv[i]=1.0/big;
				}
				else
				{
					return_code=0;
				}
			}
			if (return_code)
			{
				for (j=0;j<n;j++)
				{
					for (i=0;i<j;i++)
					{
						sum=a[i*n+j];
						for (k=0;k<i;k++)
						{
							sum -= a[i*n+k]*a[k*n+j];
						}
						a[i*n+j]=sum;
					}
					big=0.0;
					for (i=j;i<n;i++)
					{
						sum=a[i*n+j];
						for (k=0;k<j;k++)
						{
							sum -= a[i*n+k]*a[k*n+j];
						}
						a[i*n+j]=sum;
						if ((dum=vv[i]*fabs(sum)) >= big)
						{
							big=dum;
							imax=i;
						}
					}
					if (j != imax)
					{
						for (k=0;k<n;k++)
						{
							dum=a[imax*n+k];
							a[imax*n+k]=a[j*n+k];
							a[j*n+k]=dum;
						}
						*d = -(*d);
						vv[imax]=vv[j];
					}
					indx[j]=imax;
					if (0.0==a[j*n+j])
					{
						a[j*n+j]=TINY;
					}
					if (j != (n-1))
					{
						dum=1.0/(a[j*n+j]);
						for (i=j+1;i<n;i++)
						{
							a[i*n+j] *= dum;
						}
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"LU_decompose.  Singular matrix");
			}
			DEALLOCATE(vv);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"LU_decompose.  Could not create vv vector");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"LU_decompose.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* LU_decompose */

int LU_backsubstitute(int n,double *a,int *indx,double *b)
/*******************************************************************************
LAST MODIFIED : 28 January 1998

DESCRIPTION :
Partner routine for LU_decompose which takes the n x n LU matrix (a) and indx
vector returned by LU_decompose and solves for x, where LU.x=b. The solution
vector x is returned in the former right-hand-side vector <b>.
Adapted from "Numerical Recipes in C".
==============================================================================*/
{
	int i,ii,ip,j,return_code;
	double sum;

	ENTER(LU_backsubstitute);
	if ((0<n)&&a&&indx&&b)
	{
		return_code=1;
		ii=-1;
		for (i=0;i<n;i++)
		{
			ip=indx[i];
			sum=b[ip];
			b[ip]=b[i];
			if (0 <= ii)
			{
				for (j=ii;j<=i-1;j++)
				{
					sum -= a[i*n+j]*b[j];
				}
			}
			else
			{
				if (sum)
				{
					ii = i;
				}
			}
			b[i]=sum;
		}
		for (i=n-1;i>=0;i--)
		{
			sum=b[i];
			for (j=i+1;j<n;j++)
			{
				sum -= a[i*n+j]*b[j];
			}
			b[i]=sum/a[i*n+i];
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"LU_backsubstitute.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* LU_backsubstitute */

#define JACOBI_EIGENANALYSIS_MAX_ITERATIONS 50

int Jacobi_eigenanalysis(int n, double *a, double *d, double *v, int *nrot)
/*******************************************************************************
LAST MODIFIED : 7 November 2000

DESCRIPTION :
Computes all eigenvalues and eigenvectors of real symmetric n x n matrix <a>.
The eigenvalues of <a> are returned in <d>, the columns of v contain the
normalised eigenvectors. <nrot> returns the number of jacobi rotations performed
by the algorithm.
The elements of <a> above the diagonal are destroyed by this function.
Adapted from "Numerical Recipes in C".
==============================================================================*/
{
	double tresh, theta, tau, t, sm, s, h, g, c, *b, *z;
	int converged, j, iq, ip, iteration, return_code;

	ENTER(Jacobi_eigenanalysis);
	return_code = 0;
	if ((0 < n) && a && d && v && nrot)
	{
		if (ALLOCATE(b, double, n) && ALLOCATE(z, double, n))
		{
			return_code = 1;
			/* eigen vector matrix starts out as identity */
			for (ip = 0; ip < n; ip++)
			{
				for (iq = 0; iq < n; iq++)
				{
					if (ip == iq)
					{
						v[ip*n + iq] = 1.0;
					}
					else
					{
						v[ip*n + iq] = 0.0;
					}
				}
			}
			for (ip = 0; ip < n; ip++)
			{
				b[ip] = d[ip] = a[ip*n + ip];
				z[ip] = 0.0;
			}
			*nrot = 0;
			converged = 0;
			for (iteration = 0; (!converged) &&
				(iteration < JACOBI_EIGENANALYSIS_MAX_ITERATIONS); iteration++)
			{
				sm = 0.0;
				for (ip = 0; ip < n - 1; ip++)
				{
					for (iq = ip + 1; iq < n; iq++)
					{
						sm += fabs(a[ip*n + iq]);
					}
				}
				if (0.0 == sm)
				{
					converged = 1;
				}
				else
				{
					if (iteration < 4)
					{
						tresh = 0.2*sm/(n*n);
					}
					else
					{
						tresh = 0.0;
					}
					for (ip = 0; ip < n-1; ip++)
					{
						for (iq = ip+1; iq < n; iq++)
						{
							g = 100.0*fabs(a[ip*n + iq]);
							if ((4 < iteration) && ((fabs(d[ip])+g) == fabs(d[ip])) &&
								((fabs(d[iq])+g) == fabs(d[iq])))
							{
								a[ip*n + iq] = 0.0;
							}
							else if (fabs(a[ip*n + iq]) > tresh)
							{
								h = d[iq] - d[ip];
								if ((fabs(h)+g) == fabs(h))
								{
									t = (a[ip*n + iq]) / h;
								}
								else
								{
									theta = 0.5*h / (a[ip*n + iq]);
									t = 1.0 / (fabs(theta) + sqrt(1.0 + theta*theta));
									if (theta < 0.0)
									{
										t = -t;
									}
								}
								c = 1.0 / sqrt(1 + t*t);
								s = t*c;
								tau = s / (1.0 + c);
								h = t*a[ip*n + iq];
								z[ip] -= h;
								z[iq] += h;
								d[ip] -= h;
								d[iq] += h;
								a[ip*n + iq] = 0.0;
								for (j = 0; j < ip; j++)
								{
									g = a[j*n + ip];
									h = a[j*n + iq];
									a[j*n + ip] = g - s*(h + g*tau);
									a[j*n + iq] = h + s*(g - h*tau);
								}
								for (j = ip + 1; j < iq; j++)
								{
									g = a[ip*n + j];
									h = a[j*n + iq];
									a[ip*n + j] = g - s*(h + g*tau);
									a[j*n + iq] = h + s*(g - h*tau);
								}
								for (j = iq + 1; j < n; j++)
								{
									g = a[ip*n + j];
									h = a[iq*n + j];
									a[ip*n + j] = g - s*(h + g*tau);
									a[iq*n + j] = h + s*(g - h*tau);
								}
								for (j = 0; j < n; j++)
								{
									g = v[j*n + ip];
									h = v[j*n + iq];
									v[j*n + ip] = g - s*(h + g*tau);
									v[j*n + iq] = h + s*(g - h*tau);
								}
								(*nrot)++;
							}
						}
					}
				}
				for (ip = 0 ; ip < n; ip++)
				{
					b[ip] += z[ip];
					d[ip] = b[ip];
					z[ip] = 0.0;
				}
			}
			if (converged)
			{
				return_code = 1;
			}
			else
			{
				display_message(ERROR_MESSAGE,"Jacobi_eigenanalysis.  "
					"Failed to converge after %d iterations",iteration);
			}
			DEALLOCATE(b);
			DEALLOCATE(z);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Jacobi_eigenanalysis.  Not enough memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Jacobi_eigenanalysis.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* Jacobi_eigenanalysis */

#undef JACOBI_EIGENANALYSIS_MAX_ITERATIONS

int invert_FE_value_matrix3(FE_value *a,FE_value *a_inv)
/*******************************************************************************
LAST MODIFIED : 15 March 1999

DESCRIPTION :
Calculates the inverse of 3X3 FE_value matrix <a>, returning it in <a_inv>.
==============================================================================*/
{
	FE_value determinant;
	int return_code;

	ENTER(invert_FE_value_matrix3);
	if (a&&a_inv&&(1.e-08 < fabs(determinant=
		a[0]*a[4]*a[8]+a[1]*a[5]*a[6]+
		a[2]*a[3]*a[7]-a[0]*a[5]*a[7]-
		a[1]*a[3]*a[8]-a[2]*a[4]*a[6])))
	{
		a_inv[0]=  (a[4]*a[8] - a[5]*a[7])/determinant;
		a_inv[1]= -(a[1]*a[8] - a[2]*a[7])/determinant;
		a_inv[2]=  (a[1]*a[5] - a[2]*a[4])/determinant;

		a_inv[3]= -(a[3]*a[8] - a[5]*a[6])/determinant;
		a_inv[4]=  (a[0]*a[8] - a[2]*a[6])/determinant;
		a_inv[5]= -(a[0]*a[5] - a[2]*a[3])/determinant;

		a_inv[6]=  (a[3]*a[7] - a[4]*a[6])/determinant;
 		a_inv[7]= -(a[0]*a[7] - a[1]*a[6])/determinant;
		a_inv[8]=  (a[0]*a[4] - a[1]*a[3])/determinant;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"invert_FE_value_matrix3.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* invert_FE_value_matrix3 */

int multiply_FE_value_matrix3(FE_value *a,FE_value *b,FE_value *result)
/*******************************************************************************
LAST MODIFIED : 15 March 1999

DESCRIPTION :
Calculates and returns in <result> the matrix product of 3x3 FE_value matrices
<a> and <b>.
==============================================================================*/
{
	int return_code;

	ENTER(multiply_FE_value_matrix3);
	if (a&&b&&result)
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

		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"multiply_FE_value_matrix3.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* multiply_FE_value_matrix3 */

int cross_product_FE_value_vector3(FE_value *a,FE_value *b,FE_value *result)
/*******************************************************************************
LAST MODIFIED : 12 July 2000

DESCRIPTION :
Calculates and returns in <result> the vector cross product of 2 3-component
FE_value vectors <a> and <b>.
==============================================================================*/
{
	int return_code;

	ENTER(cross_product_FE_value_vector3);
	if (a&&b&&result)
	{
		result[0] = a[1]*b[2] - a[2]*b[1];
		result[1] = a[2]*b[0] - a[0]*b[2];
		result[2] = a[0]*b[1] - a[1]*b[0];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cross_product_FE_value_vector3.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* cross_product_FE_value_vector3 */

int cross_product_FE_value_vector4(FE_value *a,FE_value *b,FE_value *c,
  FE_value *result)
/*******************************************************************************
LAST MODIFIED : 27 July 2000

DESCRIPTION :
Calculates and returns in <result> the vector cross product of 3 4-component
FE_value vectors <a>, <b> and <c>.
==============================================================================*/
{
	double  A, B, C, D, E, F;       /* Intermediate Values */
 	int return_code;

	ENTER(cross_product_FE_value_vector4);
	if (a&&b&&result)
	{
		/* Calculate intermediate values. */
		A = (b[0] * c[1]) - (b[1] * c[0]);
		B = (b[0] * c[2]) - (b[2] * c[0]);
		C = (b[0] * c[3]) - (b[3] * c[0]);
		D = (b[1] * c[2]) - (b[2] * c[1]);
		E = (b[1] * c[3]) - (b[3] * c[1]);
		F = (b[2] * c[3]) - (b[3] * c[2]);

		/* Calculate the result-vector components. */
		result[0] = (FE_value)(  (a[1] * F) - (a[2] * E) + (a[3] * D));
		result[1] = (FE_value)(- (a[0] * F) + (a[2] * C) - (a[3] * B));
		result[2] = (FE_value)(  (a[0] * E) - (a[1] * C) + (a[3] * A));
		result[3] = (FE_value)(- (a[0] * D) + (a[1] * B) - (a[2] * A));
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"cross_product_FE_value_vector3.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* cross_product_FE_value_vector3 */
