/***************************************************************************//**
 * FILE : finite_element_basis.cpp
 *
 * Implementation of finite element basis functions.
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/*???DB.  Testing */
#define DOUBLE_FOR_DOT_PRODUCT

#include <cstdio>
#include <cmath>
#include <vector>
#include "opencmiss/zinc/zincconfigure.h"
#include "finite_element/finite_element_basis.h"
#include "general/debug.h"
#include "general/geometry.h"
#ifndef HAVE_HEAPSORT
#	include "general/heapsort.h"
#else
# 	include <cstdlib>
#endif
#include "general/indexed_list_private.h"
#include "general/manager_private.h"
#include "general/mystring.h"
#include "general/message.h"

/*
Module types
------------
*/

struct FE_basis
/*******************************************************************************
LAST MODIFIED : 9 October 2002

DESCRIPTION :
Stores the information for calculating basis function values from xi
coordinates.  For each of basis there will be only one copy stored in a global
list.
==============================================================================*/
{
	/* an integer array that specifies the basis as a "product" of the bases for
		the different coordinates.  The first entry is the number_of_xi_coordinates.
		The other entries are the upper triangle of a <number_of_xi_coordinates> by
		<number_of_xi_coordinates> matrix.  The entries on the diagonal specify the
		basis type for each coordinate and entries in the same row/column indicate
		associated coordinates eg.
		1.  CUBIC_HERMITE       0              0
											CUBIC_HERMITE        0
																		LINEAR_LAGRANGE
			has cubic variation in xi1 and xi2 and linear variation in xi3 (8 nodes).
		2.  SERENDIPITY       0            1
										CUBIC_HERMITE      0
																	SERENDIPITY
			has cubic variation in xi2 and 2-D serendipity variation for xi1 and
			xi3 (16 nodes).
		3.  CUBIC_HERMITE        0               0
											LINEAR_SIMPLEX         1
																			LINEAR_SIMPLEX
			has cubic variation in xi1 and 2-D linear simplex variation for xi2 and
			xi3 (6 nodes)
		4.  POLYGON        0           5
								LINEAR_LAGRANGE    0
																POLYGON
			has linear variation in xi2 and a 2-D 5-gon for xi1 and xi3 (12 nodes,
			5-gon has 5 peripheral nodes and a centre node) */
	int *type;
	/* the number of basis functions */
	int number_of_basis_functions;
	/* the blending matrix is a linear mapping from the basis used (eg. cubic
		Hermite) to the standard basis (eg. Chebyshev polynomials).  In some cases,
		for example a non-polynomial basis, this may be NULL, which indicates the
		identity matrix */
	FE_value *blending_matrix;
	/* this array gives the row number of the last non-zero value for each of the
		number_of_standard_basis_functions columns in blending_matrix, if exists */
	int *blending_matrix_column_size;
	/* to calculate the values of the "standard" basis functions */
	int number_of_standard_basis_functions;
	void *arguments;
	Standard_basis_function *standard_basis;
	// node index for n'th basis function dof: increasing from 0
	int *parameterNodes;
	// derivative type for n'th basis function dof
	// uses bits to indicate derivative: bit 0=dxi1, bit 1=dxi2, bit 2=dxi3
	// will need development if significantly different bases are added
	int *parameterDerivatives;

	/* after clearing in create, following to be modified only by manager */
	struct MANAGER(FE_basis) *manager;
	int manager_change_status;

	/* the number of structures that point to this basis.  The basis cannot be
		destroyed while this is greater than 0 */
	int access_count;
}; /* struct FE_basis */

FULL_DECLARE_INDEXED_LIST_TYPE(FE_basis);

FULL_DECLARE_MANAGER_TYPE(FE_basis);

/*
Module variables
----------------
*/

/* blending matricies for the monomial basis */

static FE_value linear_lagrange_blending_matrix[]=
{
	1,-1, /* phi1 = 1 - x */
	0, 1  /* phi2 = x     */
};

static FE_value quadratic_lagrange_blending_matrix[]=
{
	1,-3, 2, /* phi1 = 1 - 3x + 2xx */
	0, 4,-4,
	0,-1, 2
};

static FE_value cubic_lagrange_blending_matrix[]=
{
	1,-5.5,  9  , -4.5,
	0, 9  ,-22.5, 13.5,
	0,-4.5, 18  ,-13.5,
	0, 1  , -4.5,  4.5
};

static FE_value cubic_hermite_blending_matrix[]=
{
	1,0,-3, 2,
	0,1,-2, 1,
	0,0, 3,-2,
	0,0,-1, 1
};

static FE_value lagrange_hermite_blending_matrix[]=
{
	1,-2, 1,
	0, 2,-1,
	0,-1, 1
};

static FE_value hermite_lagrange_blending_matrix[]=
{
	1,0,-1,
	0,1,-1,
	0,0, 1
};

/*???DB.  Start by blending to full monomial.  Think about reduced later */
static FE_value linear_simplex_2d_blending_matrix[]=
{
	/* coefficients of 1 x y xy; all cooefficients of xy are zero, so can be "reduced" */
	1,-1,-1, 0, /* phi1 = 1 - x - y */
	0, 1, 0, 0,
	0, 0, 1, 0
};

static FE_value quadratic_simplex_2d_blending_matrix[]=
{
	/* coefficients of 1 x  xx y xy xxy yy xyy xxyy; starting to be wasteful with lots of zero terms */
	1,-3, 2,-3, 4, 0, 2, 0, 0, /* phi1 = 1 - 3x + 2xx - 3y + 4xy + 2yy */
	0, 4,-4, 0,-4, 0, 0, 0, 0,
	0,-1, 2, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 4,-4, 0,-4, 0, 0,
	0, 0, 0, 0, 4, 0, 0, 0, 0,
	0, 0, 0,-1, 0, 0, 2, 0, 0
};

static FE_value linear_simplex_3d_blending_matrix[]=
{
	1,-1,-1, 0,-1, 0, 0, 0, /* i.e. phi(0) = 1 - xi1 - xi2 - xi3; no quadratic or cubic (xi1*xi2*xi3) terms */
	0, 1, 0, 0, 0, 0, 0, 0,
	0, 0, 1, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 1, 0, 0, 0
};

static FE_value quadratic_simplex_3d_blending_matrix[]=
{
	/* Very wasteful! 27 standard basis functions with as few as 1 non-zero coefficient!
	   Intend to use simplex basis functions directly once testing examples running.
	   coefficients of monomial set, with x=xi1 y=xi2 z=xi3
		 1  x   xx   y   xy   xxy   yy   xyy   xxyy
		 z  xz  xxz  yz  xyz  xxyz  yyz  xyyz  xxyyz
		 zz xzz xxzz yzz xyzz xxyzz yyzz xyyzz xxyyzz */
	1,-3, 2,-3, 4,  0,  2,  0,0, -3, 4,  0,  4,  0,0,0,0,0,  2,  0,0,0,0,0,0,0,0,
	0, 4,-4, 0,-4,  0,  0,  0,0,  0,-4,  0,  0,  0,0,0,0,0,  0,  0,0,0,0,0,0,0,0,
	0,-1, 2, 0, 0,  0,  0,  0,0,  0, 0,  0,  0,  0,0,0,0,0,  0,  0,0,0,0,0,0,0,0,
	0, 0, 0, 4,-4,  0, -4,  0,0,  0, 0,  0, -4,  0,0,0,0,0,  0,  0,0,0,0,0,0,0,0,
	0, 0, 0, 0, 4,  0,  0,  0,0,  0, 0,  0,  0,  0,0,0,0,0,  0,  0,0,0,0,0,0,0,0,
	0, 0, 0,-1, 0,  0,  2,  0,0,  0, 0,  0,  0,  0,0,0,0,0,  0,  0,0,0,0,0,0,0,0,
	0, 0, 0, 0, 0,  0,  0,  0,0,  4,-4,  0, -4,  0,0,0,0,0, -4,  0,0,0,0,0,0,0,0,
	0, 0, 0, 0, 0,  0,  0,  0,0,  0, 4,  0,  0,  0,0,0,0,0,  0,  0,0,0,0,0,0,0,0,
	0, 0, 0, 0, 0,  0,  0,  0,0,  0, 0,  0,  4,  0,0,0,0,0,  0,  0,0,0,0,0,0,0,0,
	0, 0, 0, 0, 0,  0,  0,  0,0, -1, 0,  0,  0,  0,0,0,0,0,  2,  0,0,0,0,0,0,0,0
};



#if defined (NEW_NEW_CODE)
/*???DB.  For reduced monomial */
/* simplex elements have zero coefficients for most/all higher order monomial terms, e.g. xi1*xi1 or xi1*xi2*xi2, so
 * in theory these could be removed from the monomial basis. Similarly for serendipity element bases which
 * are like Lagrangian elements but omitting certain interior or midface nodes associated with higher order monomial terms */
static FE_value linear_simplex_2d_blending_matrix[]=
{
	1,-1,-1,
	0, 1, 0,
	0, 0, 1
};

static FE_value quadratic_simplex_2d_blending_matrix[]=
{
	1,-3, 2,-3,-4, 2,
	0, 4,-4, 0,-4, 0,
	0,-1, 2, 0, 0, 0,
	0, 0, 0, 4,-4,-4,
	0, 0, 0, 0, 4, 0,
	0, 0, 0,-1, 0, 2
};

static FE_value linear_simplex_3d_blending_matrix[]=
{
	1,-1,-1,-1,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1
};
#endif /* defined (NEW_NEW_CODE) */

/*
Module functions
----------------
*/

static int compare_FE_basis_type(int *basis_type_1,int *basis_type_2)
/*******************************************************************************
LAST MODIFIED : 2 November 1995

DESCRIPTION :
Returns -1 if basis_type_1 < basis_type_2, 0 if basis_type_1 = basis_type_2 and
1 if basis_type_1 > basis_type_2.
==============================================================================*/
{
	int return_code,*type_1,*type_2;
	int i;

	ENTER(compare_FE_basis_type);
	if (NULL != (type_1=basis_type_1))
	{
		if (NULL != (type_2=basis_type_2))
		{
			if ((i= *type_1)== *type_2)
			{
				if (i>0)
				{
					i *= i+1;
					i /= 2;
					do
					{
						type_1++;
						type_2++;
						i--;
					} while ((i>0)&&(*type_1== *type_2));
					if (*type_1< *type_2)
					{
						return_code= -1;
					}
					else
					{
						if (*type_1> *type_2)
						{
							return_code=1;
						}
						else
						{
							return_code=0;
						}
					}
				}
				else
				{
					return_code=0;
				}
			}
			else
			{
				if (*type_1< *type_2)
				{
					return_code= -1;
				}
				else
				{
					return_code=1;
				}
			}
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		if (basis_type_2)
		{
			return_code= -1;
		}
		else
		{
			return_code=0;
		}
	}
	LEAVE;

	return (return_code);
} /* compare_FE_basis_type */

DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(FE_basis,type,int *,compare_FE_basis_type)

static FE_value *tensor_product(int row_dimension_1,int column_dimension_1,
	FE_value *matrix_1,int row_dimension_2,int column_dimension_2,
	FE_value *matrix_2)
/*******************************************************************************
LAST MODIFIED : 17 June 1994

DESCRIPTION :
This function returns the tensor product (memory allocated within the function)
of <matrix_1> and <matrix_2>.  All matrices are assumed to be stored row-wise.
==============================================================================*/
{
	FE_value *product,*row_start_1,*row_start_2,*value,*value_1 = NULL, *value_2 = NULL;
	int i,j,k,l;

	ENTER(tensor_product);
	if ((row_dimension_1>0)&&(column_dimension_1>0)&&matrix_1&&
		(row_dimension_2>0)&&(column_dimension_2>0)&&matrix_2)
	{
		if (ALLOCATE(product,FE_value,
			row_dimension_1*column_dimension_1*row_dimension_2*column_dimension_2))
		{
			value=product;
			row_start_1=matrix_1;
			for (i=row_dimension_1;i>0;i--)
			{
				row_start_2=matrix_2;
				for (j=row_dimension_2;j>0;j--)
				{
					value_1=row_start_1;
					for (k=column_dimension_1;k>0;k--)
					{
						value_2=row_start_2;
						for (l=column_dimension_2;l>0;l--)
						{
							*value=(*value_2)*(*value_1);
							value++;
							value_2++;
						}
						value_1++;
					}
					row_start_2=value_2;
				}
				row_start_1=value_1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"tensor_product.  Could not allocate memory for product");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "tensor_product.  Invalid argument(s)");
		product=(FE_value *)NULL;
	}
	LEAVE;

	return (product);
} /* tensor_product */

static int sort_integers(const void *number_1_address, const void *number_2_address)
/*******************************************************************************
LAST MODIFIED : 10 May 2001

DESCRIPTION :
==============================================================================*/
{
	int number_1,number_2,return_code;

	ENTER(sort_integers);
	number_1= *((int *)number_1_address);
	number_2= *((int *)number_2_address);
	if (number_1<number_2)
	{
		return_code= -1;
	}
	else
	{
		if (number_1>number_2)
		{
			return_code=1;
		}
		else
		{
			return_code=0;
		}
	}
	LEAVE;

	return (return_code);
} /* sort_integers */

static int sort_basis_functions(const void *tuple_1_void,const void *tuple_2_void)
/*******************************************************************************
LAST MODIFIED : 13 May 2001

DESCRIPTION :
==============================================================================*/
{
	int i,number_of_xi_coordinates,return_code,*tuple_1,*tuple_2;

	ENTER(sort_basis_functions);
	tuple_1=(int *)tuple_1_void;
	tuple_2=(int *)tuple_2_void;
	number_of_xi_coordinates=tuple_1[1];
	i=number_of_xi_coordinates-1;
	tuple_1 += 2*number_of_xi_coordinates;
	tuple_2 += 2*number_of_xi_coordinates;
	while ((i>0)&&(*tuple_1== *tuple_2))
	{
		tuple_1 -= 2;
		tuple_2 -= 2;
	}
	if (*tuple_1< *tuple_2)
	{
		return_code= -1;
	}
	else
	{
		if (*tuple_1> *tuple_2)
		{
			return_code=1;
		}
		else
		{
			tuple_1=(int *)tuple_1_void;
			tuple_2=(int *)tuple_2_void;
			i=number_of_xi_coordinates-1;
			tuple_1 += 2*number_of_xi_coordinates+1;
			tuple_2 += 2*number_of_xi_coordinates+1;
			while ((i>0)&&(*tuple_1== *tuple_2))
			{
				tuple_1 -= 2;
				tuple_2 -= 2;
			}
			if (*tuple_1< *tuple_2)
			{
				return_code= -1;
			}
			else
			{
				if (*tuple_1> *tuple_2)
				{
					return_code=1;
				}
				else
				{
					return_code=0;
				}
			}
		}
	}
	LEAVE;

	return (return_code);
} /* sort_basis_functions */

int monomial_basis_functions(void *type_arguments,
	const FE_value *xi_coordinates, FE_value *function_values)
/*******************************************************************************
LAST MODIFIED : 29 January 1994

DESCRIPTION :
Calculates the monomial basis function values.  Expects the memory to already be
allocated for the <function_values>.  The first entry of the <type_arguments> is
the number of xi coordinates and the other entries are the orders of the
monomials for each xi coordinate.
NB.  xi_1 is varying fastest (xi_n slowest)
==============================================================================*/
{
	FE_value *temp_value, *value, xi, xi_power;
	int i,j,k,order,number_of_values,number_of_xi_coordinates,
		return_code;

	ENTER(monomial_basis_functions);
	const int *argument = reinterpret_cast<int *>(type_arguments);
	if (argument && xi_coordinates && function_values)
	{
		const FE_value *xi_coordinate = xi_coordinates;
		number_of_xi_coordinates= *argument;
		value=function_values;
		*value=1;
		number_of_values=1;
		for (i=number_of_xi_coordinates;i>0;i--)
		{
			xi= *xi_coordinate;
			xi_coordinate++;
			xi_power=xi;
			argument++;
			order= *argument;
			for (j=order;j>0;j--)
			{
				temp_value=function_values;
				for (k=number_of_values;k>0;k--)
				{
					value++;
					*value=(*temp_value)*xi_power;
					temp_value++;
				}
				xi_power *= xi;
			}
			number_of_values *= (order+1);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"monomial_basis_functions.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* monomial_basis_functions */

int polygon_basis_functions(void *type_arguments,
	const FE_value *xi_coordinates, FE_value *function_values)
/*******************************************************************************
LAST MODIFIED : 23 November 2001

SCRIPTION :
For a polygon
a) xi1 is circumferential and xi2 is radial.
b) the nodal basis functions are
	phi0 = 1 - xi2,  centre node
				/ xi2 * (1 - n*xi1), 0<=n*xi1<1
	phi1 = | xi2 * (n*xi1 - (n-1)), n-1<n*xi1<n
				\ 0, otherwise
				/ xi2 * (i - n*xi1), i-1<=n*xi1<i
	phii = | xi2 * (n*xi1 - (i-2)), i-2<n*xi1<i-1 ,  1<i<=n
				\ 0, otherwise
	where n is the number of polygon verticies/sides.
c) the standard basis functions are
				/ 1, xi1=1
	s001 = | 1, 0<=n*xi1<1
				\ 0, otherwise
	s00i = / 1, i-1<=n*xi1<i ,  2<=i<=n
				\ 0, otherwise
	s10i = / n*xi1-i+1, i-1<=n*xi1<i ,  1<=i<=n
				\ 0, otherwise
	s01i = xi2 * s00i ,  1<=i<=n
	s11i = xi2 * s10i ,  1<=i<=n
???DB.  Should this be
				/ 0.5, xi1=0 or xi1=1 or n*xi1=1
	s001 = | 1, 0<n*xi1<1
				\ 0, otherwise
				/ 0.5, n*xi1=i-1 or n*xi1=i
	s00i = | 1, i-1<n*xi1<i           ,  2<=i<=n
				\ 0, otherwise
				/ 0.5, n*xi1=i
	s10i = | n*xi1-i+1, i-1<n*xi1<i ,  1<=i<=n
				\ 0, otherwise
	s01i = xi2 * s00i ,  1<=i<=n
	s11i = xi2 * s10i ,  1<=i<=n
	to get derivatives right (better) at vertices ?
d) the blending matrix is
	1 1 1 . . . 1 0 0 0 . . . 0 -1 -1 -1 . . . -1  0  0  0 . . .  0
	0 0 0 . . . 0 0 0 0 . . . 0  1  0  0 . . .  0 -1  0  0 . . .  1
	0 0 0 . . . 0 0 0 0 . . . 0  0  1  0 . . .  0  1 -1  0 . . .  0
	0 0 0 . . . 0 0 0 0 . . . 0  0  0  1 . . .  0  0  1 -1 . . .  0
	.             .              .              .              .
	.             .              .              .              .
	.             .              .              .              .
	0 0 0 . . . 0 0 0 0 . . . 0  0  0  0 . . .  1  0  0  0 . 0 1 -1
==============================================================================*/
{
	FE_value basis_function10, basis_function11, save_value, *temp_value, *value,
		xi, xi_circumferential, xi_power, xi_radial;
	int i, j, k, number_of_polygon_verticies,
		number_of_values, number_of_xi_coordinates, offset00, offset01, offset10,
		offset11, order, polygon_offset, polygon_vertex, return_code;

	ENTER(polygon_basis_functions);
	const int *argument = reinterpret_cast<int *>(type_arguments);
	if (argument && xi_coordinates && function_values)
	{
		const FE_value *xi_coordinate = xi_coordinates;
		number_of_xi_coordinates= *argument;
		value=function_values;
		*value=1;
		number_of_values=1;
		for (i=number_of_xi_coordinates;i>0;i--)
		{
			xi= *xi_coordinate;
			argument++;
			order= *argument;
			if (order<0)
			{
				/* polygon */
				order= -order;
				/* need to distinguish between first and second polygon coordinates */
				if (order%2)
				{
					/* first polygon coordinate */
					order /= 2;
					polygon_offset=order%number_of_xi_coordinates;
					order /= number_of_xi_coordinates;
					/* first polygon coordinate is circumferential */
					xi_circumferential=xi;
					/* second polygon coordinate is radial */
					xi_radial=xi_coordinate[polygon_offset];
					/* may be called with xi outside the element (eg. during streamline
						tracking).  Wrap the circumferential coordinate */
					xi_circumferential -= floor(xi_circumferential);
					number_of_polygon_verticies=(-argument[polygon_offset])/2;
					/*???DB.  Need to do higher order polygons */
					basis_function10=
						xi_circumferential*(FE_value)number_of_polygon_verticies;
					polygon_vertex=(int)(basis_function10);
					basis_function10 -= (FE_value)polygon_vertex;
					if (number_of_polygon_verticies==polygon_vertex)
					{
						polygon_vertex=0;
					}
					basis_function11=xi_radial*basis_function10;
					temp_value=function_values;
					offset00=polygon_vertex*number_of_values;
					offset11=number_of_polygon_verticies*number_of_values;
					offset10=offset00+offset11;
					offset01=offset10+offset11;
					offset11=offset01+offset11;
					for (j=number_of_values;j>0;j--)
					{
						save_value= *temp_value;
						temp_value[offset00]=save_value;
						temp_value[offset10]=basis_function10*save_value;
						temp_value[offset01]=xi_radial*save_value;
						temp_value[offset11]=basis_function11*save_value;
						temp_value++;
					}
					temp_value=function_values;
					offset10=number_of_polygon_verticies*number_of_values;
					offset01=2*offset10;
					offset11=3*offset10;
					for (j=offset00;j>0;j--)
					{
						*temp_value=0;
						temp_value[offset10]=0;
						temp_value[offset01]=0;
						temp_value[offset11]=0;
						temp_value++;
					}
					temp_value += number_of_values;
					for (j=(number_of_polygon_verticies-polygon_vertex-1)*
						number_of_values;j>0;j--)
					{
						*temp_value=0;
						temp_value[offset10]=0;
						temp_value[offset01]=0;
						temp_value[offset11]=0;
						temp_value++;
					}
					number_of_values *= 4*number_of_polygon_verticies;
					value=function_values+(number_of_values-1);
				}
			}
			else
			{
				/* not polygon */
				xi_power=xi;
				for (j=order;j>0;j--)
				{
					temp_value=function_values;
					for (k=number_of_values;k>0;k--)
					{
						value++;
						*value=(*temp_value)*xi_power;
						temp_value++;
					}
					xi_power *= xi;
				}
				number_of_values *= (order+1);
			}
			xi_coordinate++;
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"polygon_basis_functions.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* polygon_basis_functions */

DECLARE_LOCAL_MANAGER_FUNCTIONS(FE_basis)

const int *FE_basis_get_basis_type(struct FE_basis *basis)
{
	if (!basis)
		return 0;
	return basis->type;
}

int FE_basis_get_number_of_functions(struct FE_basis *basis)
{
	if ((!basis) || (!basis->standard_basis))
		return 0;
	return basis->number_of_basis_functions;
}

int FE_basis_get_number_of_blended_functions(struct FE_basis *basis)
{
	if ((!basis) || (!basis->blending_matrix))
		return 0;
	return basis->number_of_standard_basis_functions;
}

FE_value *FE_basis_get_blended_element_values(struct FE_basis *basis,
	const FE_value *raw_element_values)
{
	if ((!basis) || (0 >= basis->number_of_basis_functions) ||
		(!basis->standard_basis) || (!basis->blending_matrix) ||
		(basis->number_of_standard_basis_functions < basis->number_of_basis_functions))
	{
		display_message(ERROR_MESSAGE, "FE_basis_get_blended_element_values.  Invalid basis.");
		return 0;
	}
	if (!raw_element_values)
	{
		display_message(ERROR_MESSAGE, "FE_basis_get_blended_element_values.  Missing element values.");
		return 0;
	}
	int number_of_blended_element_values = basis->number_of_standard_basis_functions;
	FE_value *blended_element_values = 0;
	ALLOCATE(blended_element_values, FE_value, number_of_blended_element_values);
	if (!blended_element_values)
	{
		return 0;
	}
	FE_value *blended_element_value = blended_element_values;
	double sum;
	const FE_value *element_value;
	FE_value *blending_matrix;
	for (int i = 0; i < number_of_blended_element_values; i++)
	{
		sum = 0.0;
		element_value = raw_element_values;
		blending_matrix = basis->blending_matrix + i;
		for (int j = basis->blending_matrix_column_size[i]; j > 0; j--)
		{
#if defined (DOUBLE_FOR_DOT_PRODUCT)
			sum += (double)(*blending_matrix)*(double)(*element_value);
#else /* defined (DOUBLE_FOR_DOT_PRODUCT) */
			sum += (*blending_matrix)*(*element_value);
#endif /* defined (DOUBLE_FOR_DOT_PRODUCT) */
			element_value++;
			blending_matrix += number_of_blended_element_values;
		}
		*blended_element_value = (FE_value)sum;
		blended_element_value++;
	}
	return blended_element_values;
}

FE_value *FE_basis_calculate_combined_blending_matrix(struct FE_basis *basis,
	int number_of_blended_values, int number_of_inherited_values,
	const FE_value *inherited_blend_matrix)
{
	if ((!basis) || (0 > basis->number_of_basis_functions) ||
		(!basis->standard_basis) || (!basis->blending_matrix) ||
		(number_of_blended_values != basis->number_of_standard_basis_functions))
	{
		display_message(ERROR_MESSAGE, "FE_basis_get_combined_blending_matrix.  Invalid basis.");
		return 0;
	}
	if ((0 >= number_of_inherited_values) || (!inherited_blend_matrix))
	{
		display_message(ERROR_MESSAGE, "FE_basis_get_combined_blending_matrix.  Missing inherited blending matrix.");
		return 0;
	}
	int number_of_element_values = basis->number_of_basis_functions;
	FE_value *combined_blending_matrix = 0;
	ALLOCATE(combined_blending_matrix, FE_value, number_of_element_values*number_of_inherited_values);
	if (!combined_blending_matrix)
	{
		return 0;
	}
	FE_value *transformation = combined_blending_matrix;
	const FE_value *row, *column;
	double sum;
	for (int i = 0; i < number_of_element_values; i++)
	{
		for (int j = 0; j < number_of_inherited_values; j++)
		{
			sum = 0;
			row = (basis->blending_matrix) + (i*number_of_blended_values);
			column = inherited_blend_matrix + j;
			for (int k = number_of_blended_values; k > 0; k--)
			{
#if defined (DOUBLE_FOR_DOT_PRODUCT)
				sum += (double)(*row)*(double)(*column);
#else /* defined (DOUBLE_FOR_DOT_PRODUCT) */
				sum += (*row)*(*column);
#endif /* defined (DOUBLE_FOR_DOT_PRODUCT) */
				row++;
				column += number_of_inherited_values;
			}
			*transformation = (FE_value)sum;
			transformation++;
		}
	}
	return combined_blending_matrix;
}

Standard_basis_function *FE_basis_get_standard_basis_function(struct FE_basis *basis)
{
	if (!basis)
		return 0;
	return basis->standard_basis;
}

const int *FE_basis_get_standard_basis_function_arguments(struct FE_basis *basis)
{
	if (!basis)
		return 0;
	return reinterpret_cast<const int *>(basis->arguments);
}

int calculate_standard_basis_transformation(struct FE_basis *basis,
	FE_value *coordinate_transformation,int inherited_dimension,
	int **inherited_arguments_address,int *number_of_inherited_values_address,
	Standard_basis_function **inherited_standard_basis_function_address,
	FE_value **blending_matrix_address)
{
	FE_value *blending_matrix,*expanded_coordinate_transformation,
		*inherited_value,*reorder_blending_matrix,scalar,*sumand,*transformation,
		*value;
	int basis_dimension,*field_to_element,i,inherited_polygon_offset,
		*inherited_standard_basis_argument,*inherited_standard_basis_arguments,j,k,
		l,m,need_reordering,number_of_inherited_values,number_of_polygon_verticies,
		number_of_values,offset,order,p,polygon_offset,polygon_vertex,
		*reorder_coordinate,*reorder_value,*reorder_values,return_code,row_size,
		*standard_basis_argument,*standard_basis_arguments;
	Standard_basis_function *standard_basis_function;
#if defined (DOUBLE_FOR_DOT_PRODUCT)
	double sum;
#else /* defined (DOUBLE_FOR_DOT_PRODUCT) */
	FE_value sum;
#endif /* defined (DOUBLE_FOR_DOT_PRODUCT) */

	ENTER(calculate_standard_basis_transformation);
#if defined (DEBUG_CODE)
	/*???debug */
	printf("enter calculate_standard_basis_transformation\n");
#endif /* defined (DEBUG_CODE) */
	return_code=0;
	if (basis&&(inherited_dimension>0)&&inherited_arguments_address&&
		inherited_standard_basis_function_address&&blending_matrix_address)
	{
		standard_basis_arguments=(int *)(basis->arguments);
		standard_basis_function=basis->standard_basis;
		basis_dimension=(basis->type)[0];
		if (monomial_basis_functions==standard_basis_function)
		{
			/* calculate the blending matrix from the monomial basis for the full
				<basis> to the inherited monomial basis */
			ALLOCATE(inherited_standard_basis_arguments,int,inherited_dimension+1);
			ALLOCATE(expanded_coordinate_transformation,FE_value,
				(basis_dimension+1)*(inherited_dimension+1));
			if (inherited_standard_basis_arguments&&
				expanded_coordinate_transformation)
			{
				inherited_standard_basis_argument=
					inherited_standard_basis_arguments;
				*inherited_standard_basis_argument=inherited_dimension;
				for (i=inherited_dimension;i>0;i--)
				{
					inherited_standard_basis_argument++;
					*inherited_standard_basis_argument=0;
				}
				standard_basis_argument=standard_basis_arguments;
				expanded_coordinate_transformation[0]=1;
				transformation=expanded_coordinate_transformation;
				for (j=inherited_dimension;j>0;j--)
				{
					transformation++;
					*transformation=0;
				}
				if (NULL != (value=coordinate_transformation))
				{
					for (i=basis_dimension;i>0;i--)
					{
						standard_basis_argument++;
						order= *standard_basis_argument;
						inherited_standard_basis_argument=
							inherited_standard_basis_arguments;
						for (j=inherited_dimension;j>=0;j--)
						{
							transformation++;
							*transformation= *value;
							value++;
							if (j>0)
							{
								inherited_standard_basis_argument++;
								if ((0!= *value)&&(order> *inherited_standard_basis_argument))
								{
									*inherited_standard_basis_argument=order;
								}
							}
						}
					}
				}
				else
				{
					for (i=basis_dimension*(inherited_dimension+1);i>0;i--)
					{
						transformation++;
						*transformation=0;
					}
					transformation=expanded_coordinate_transformation+
						(inherited_dimension+1);
					inherited_standard_basis_argument=
						inherited_standard_basis_arguments;
					i=1;
					while (i<=basis_dimension)
					{
						standard_basis_argument++;
						inherited_standard_basis_argument++;
						transformation[i]=1;
						i++;
						transformation += inherited_dimension+1;
						*inherited_standard_basis_argument= *standard_basis_argument;
					}
				}
				row_size=basis->number_of_standard_basis_functions;
				number_of_inherited_values=1;
				inherited_standard_basis_argument=
					inherited_standard_basis_arguments;
				for (j=inherited_dimension;j>0;j--)
				{
					inherited_standard_basis_argument++;
					number_of_inherited_values *= (*inherited_standard_basis_argument)+1;
				}
				if (ALLOCATE(blending_matrix,FE_value,
					row_size*number_of_inherited_values))
				{
					value=blending_matrix;
					*value=1;
					for (i=row_size*number_of_inherited_values-1;i>0;i--)
					{
						value++;
						*value=0;
					}
					standard_basis_argument=standard_basis_arguments;
					row_size=1;
					transformation=expanded_coordinate_transformation+
						(inherited_dimension+1);
					for (i=basis_dimension;i>0;i--)
					{
						standard_basis_argument++;
						order= *standard_basis_argument;
						for (j=0;j<order;j++)
						{
							offset=0;
							for (k=0;k<=inherited_dimension;k++)
							{
								if (0!= *transformation)
								{
									/* loop over blending matrix rows */
									value=blending_matrix+(j*row_size*number_of_inherited_values);
									for (l=0;l<row_size;l++)
									{
										for (p=number_of_inherited_values-offset;p>0;p--)
										{
											if (0!= *value)
											{
												value[row_size*number_of_inherited_values+offset] +=
													(*value)*(*transformation);
											}
											value++;
										}
										value += offset;
									}
								}
								if (k>0)
								{
									offset *= (1+inherited_standard_basis_arguments[k]);
								}
								else
								{
									offset=1;
								}
								transformation++;
							}
							transformation -= inherited_dimension+1;
						}
						transformation += inherited_dimension+1;
						row_size *= (order+1);
					}
					*inherited_arguments_address=inherited_standard_basis_arguments;
					*blending_matrix_address=blending_matrix;
					*number_of_inherited_values_address=number_of_inherited_values;
					*inherited_standard_basis_function_address=monomial_basis_functions;
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"calculate_standard_basis_transformation.  "
						"Insufficient memory for blending_matrix");
					DEALLOCATE(inherited_standard_basis_arguments);
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"calculate_standard_basis_transformation.  Insufficient memory 1");
				DEALLOCATE(inherited_standard_basis_arguments);
				return_code=0;
			}
			DEALLOCATE(expanded_coordinate_transformation);
		}
		else
		{
			if (polygon_basis_functions==standard_basis_function)
			{
				/* calculate the blending matrix from the standard basis for the full
					<basis> to the inherited standard basis */
				ALLOCATE(inherited_standard_basis_arguments,int,inherited_dimension+1);
				ALLOCATE(field_to_element,int,basis_dimension);
				ALLOCATE(reorder_coordinate,int,inherited_dimension);
				row_size=basis->number_of_standard_basis_functions;
				ALLOCATE(blending_matrix,FE_value,row_size*row_size);
				if (inherited_standard_basis_arguments&&field_to_element&&
					reorder_coordinate&&blending_matrix)
				{
					return_code=1;
					/* determine the correspondence between the xi coordinates for the
						element field and the xi coordinates for the element */
					/*???DB.  At present only able to do transformations that are one to
						one in the xi coordinates.  This is equivalent to one non-zero in
						each column of A and at most one non-zero in each row of A */
					if (NULL != (value=coordinate_transformation))
					{
						i=0;
						k=0;
						need_reordering=0;
						standard_basis_argument=standard_basis_arguments;
						while (return_code&&(i<basis_dimension))
						{
							field_to_element[i]=0;
							standard_basis_argument++;
							order= *standard_basis_argument;
							/* skip translate b */
							value++;
							/* examine matrix A */
							j=0;
							while (return_code&&(j<inherited_dimension))
							{
								if (0!= *value)
								{
									if (0==field_to_element[i])
									{
										field_to_element[i]=j+1;
										reorder_coordinate[k]=j;
										if (j!=k)
										{
											need_reordering=1;
										}
										k++;
									}
									else
									{
										display_message(ERROR_MESSAGE,
"calculate_standard_basis_transformation.  Coordinate transformation not 1 to 1");
										return_code=0;
									}
								}
								value++;
								j++;
							}
							i++;
						}
						if (return_code)
						{
							if (!need_reordering)
							{
								/* coordinate reordering is not needed */
								DEALLOCATE(reorder_coordinate);
							}
						}
					}
					else
					{
						need_reordering=0;
						DEALLOCATE(reorder_coordinate);
						for (i=inherited_dimension;i>0;i--)
						{
							field_to_element[i-1]=i;
						}
					}
					if (return_code)
					{
						/* calculate the arguments for the standard basis function */
						number_of_inherited_values=1;
						*inherited_standard_basis_arguments=inherited_dimension;
						standard_basis_argument=standard_basis_arguments;
						standard_basis_function=monomial_basis_functions;
						i=0;
						while (return_code&&(i<basis_dimension))
						{
							standard_basis_argument++;
							order= *standard_basis_argument;
							if (order<0)
							{
								/* polygon */
								order= -order;
								if (order%2)
								{
									/* first polygon coordinate */
									order /= 2;
									polygon_offset=order%basis_dimension;
									order /= basis_dimension;
									number_of_polygon_verticies=
										(-standard_basis_argument[polygon_offset])/2;
									if (field_to_element[i])
									{
										if (field_to_element[i+polygon_offset])
										{
											/* radial and circumferential coordinates inherited */
											standard_basis_function=polygon_basis_functions;
											inherited_polygon_offset=
												field_to_element[i+polygon_offset]-field_to_element[i];
											if (inherited_polygon_offset<0)
											{
												display_message(ERROR_MESSAGE,
													"calculate_standard_basis_transformation.  "
													"Reordering of polygon coordinates is not supported");
												return_code=0;
											}
											else
											{
												inherited_standard_basis_arguments[field_to_element[i]]=
													-(1+2*(inherited_polygon_offset+
													order*inherited_dimension));
												inherited_standard_basis_arguments[field_to_element[
													i+polygon_offset]]= -2*number_of_polygon_verticies;
											}
											number_of_inherited_values *=
												4*number_of_polygon_verticies;
										}
										else
										{
											/* circumferential coordinate inherited */
											inherited_standard_basis_arguments[field_to_element[i]]=
												order;
											number_of_inherited_values *= order+1;
										}
									}
									else
									{
										if (field_to_element[i+polygon_offset])
										{
											display_message(ERROR_MESSAGE,
												"calculate_standard_basis_transformation.  "
												"Cannot inherit polygon radial coordinate by itself");
											return_code=0;
										}
									}
								}
								else
								{
									/* second polygon coordinate.  Validity checked and argument
										assigned with first coordinate */
								}
							}
							else
							{
								/* not polygon */
								if (field_to_element[i])
								{
									inherited_standard_basis_arguments[field_to_element[i]]=order;
									number_of_inherited_values *= order+1;
								}
							}
							i++;
						}
						if (return_code)
						{
							/* calculate the blending matrix */
							/* start with the identity */
							value=blending_matrix;
							*value=1;
							for (i=row_size-1;i>0;i--)
							{
								for (j=row_size;j>0;j--)
								{
									value++;
									*value=0;
								}
								value++;
								*value=1;
							}
							if (NULL != (transformation=coordinate_transformation))
							{
								number_of_inherited_values=1;
								standard_basis_argument=standard_basis_arguments;
								number_of_values=basis->number_of_standard_basis_functions;
								offset=1;
								for (i=0;i<basis_dimension;i++)
								{
									standard_basis_argument++;
									order= *standard_basis_argument;
									if (order<0)
									{
										/* polygon */
										order= -order;
										if (order%2)
										{
											/* first polygon component */
											order /= 2;
											polygon_offset=order%basis_dimension;
											order /= basis_dimension;
											number_of_polygon_verticies=
												(-standard_basis_argument[polygon_offset])/2;
											/* NB.  Have already checked validity when calculating
												inherited arguments */
											if (field_to_element[i])
											{
												if (field_to_element[i+polygon_offset])
												{
													/* polygon is in projection */
													number_of_inherited_values *=
														4*number_of_polygon_verticies;
												}
												else
												{
													/* edge of polygon in projection */
													polygon_vertex=(int)(0.1+(*transformation)*
														(FE_value)number_of_polygon_verticies);
													value=blending_matrix;
													inherited_value=value;
													if (number_of_polygon_verticies-1==polygon_vertex)
													{
														for (j=number_of_values/
															(4*number_of_polygon_verticies*
															number_of_inherited_values);j>0;j--)
														{
															for (k=number_of_inherited_values;k>0;k--)
															{
																for (l=row_size;l>0;l--)
																{
																	scalar=value[polygon_vertex*
																		number_of_inherited_values*row_size]+
																		value[(polygon_vertex+
																		2*number_of_polygon_verticies)*
																		number_of_inherited_values*row_size];
																	inherited_value[
																		number_of_inherited_values*row_size]=
																		(*value)+
																		value[2*number_of_polygon_verticies*
																		number_of_inherited_values*row_size]-scalar;
																	*inherited_value=scalar;
																	inherited_value++;
																	value++;
																}
															}
															inherited_value +=
																number_of_inherited_values*row_size;
															value +=
																(4*number_of_polygon_verticies-1)*
																number_of_inherited_values*row_size;
														}
													}
													else
													{
														if (number_of_polygon_verticies==polygon_vertex)
														{
															polygon_vertex=0;
														}
														value += polygon_vertex*number_of_inherited_values*
															row_size;
														for (j=number_of_values/
															(4*number_of_polygon_verticies*
															number_of_inherited_values);j>0;j--)
														{
															for (k=number_of_inherited_values;k>0;k--)
															{
																for (l=row_size;l>0;l--)
																{
																	scalar= (*value)+
																		value[2*number_of_polygon_verticies*
																		number_of_inherited_values*row_size];
																	inherited_value[
																		number_of_inherited_values*row_size]=
																		value[number_of_inherited_values*row_size]+
																		value[(2*number_of_polygon_verticies+1)*
																		number_of_inherited_values*row_size]-
																		scalar;
																	*inherited_value=scalar;
																	inherited_value++;
																	value++;
																}
															}
															inherited_value +=
																number_of_inherited_values*row_size;
															value += (4*number_of_polygon_verticies-1)*
																number_of_inherited_values*row_size;
														}
													}
													number_of_values /= 4*number_of_polygon_verticies;
													number_of_values *= 2;
													number_of_inherited_values *= 2;
												}
											}
											else
											{
												/* polygon is not in projection */
												polygon_vertex=(int)(0.1+(*transformation)*
													(FE_value)number_of_polygon_verticies);
												if (number_of_polygon_verticies==polygon_vertex)
												{
													polygon_vertex=0;
												}
												value=blending_matrix;
												inherited_value=value;
												value +=
													polygon_vertex*number_of_inherited_values*row_size;
												for (j=number_of_values/(4*number_of_polygon_verticies*
													number_of_inherited_values);j>0;j--)
												{
													for (k=number_of_inherited_values;k>0;k--)
													{
														for (l=row_size;l>0;l--)
														{
															*inherited_value= *value+
																value[2*number_of_polygon_verticies*
																number_of_inherited_values*row_size];
															inherited_value++;
															value++;
														}
													}
													value += (4*number_of_polygon_verticies-1)*
														number_of_inherited_values*row_size;
												}
												number_of_values /=
													4*number_of_polygon_verticies;
											}
										}
									}
									else
									{
										/* not polygon */
										if (field_to_element[i])
										{
											/* in projection */
											number_of_inherited_values *= order+1;
										}
										else
										{
											/* not in projection */
											if (*transformation)
											{
												/* assume *transformation (b) is 1 */
												value=blending_matrix;
												inherited_value=value;
												for (j=number_of_values/
													((order+1)*number_of_inherited_values);j>0;j--)
												{
													for (k=number_of_inherited_values;k>0;k--)
													{
														for (l=row_size;l>0;l--)
														{
#if defined (DOUBLE_FOR_DOT_PRODUCT)
															sum=(double)(*value);
#else /* defined (DOUBLE_FOR_DOT_PRODUCT) */
															sum= *value;
#endif /* defined (DOUBLE_FOR_DOT_PRODUCT) */
															sumand=value;
															for (m=order;m>0;m--)
															{
																sumand += number_of_inherited_values*row_size;
#if defined (DOUBLE_FOR_DOT_PRODUCT)
																sum += (double)(*sumand);
#else /* defined (DOUBLE_FOR_DOT_PRODUCT) */
																sum += *sumand;
#endif /* defined (DOUBLE_FOR_DOT_PRODUCT) */
															}
															*inherited_value=(FE_value)sum;
															inherited_value++;
															value++;
														}
													}
													value += order*number_of_inherited_values*row_size;
												}
												number_of_values /= order+1;
											}
											else
											{
												/* *transformation (b) is 0 */
												value=blending_matrix;
												inherited_value=value;
												for (j=number_of_values/
													((order+1)*number_of_inherited_values);j>0;j--)
												{
													for (k=number_of_inherited_values*row_size;k>0;k--)
													{
														*inherited_value= *value;
														inherited_value++;
														value++;
													}
													value += order*number_of_inherited_values*row_size;
												}
												number_of_values /= order+1;
											}
										}
									}
									transformation += inherited_dimension+1;
								}
								/* transpose so that stored by columns */
								value=blending_matrix;
								for (i=0;i<row_size;i++)
								{
									inherited_value=blending_matrix+i;
									for (j=0;j<i;j++)
									{
										scalar= *inherited_value;
										*inherited_value= *value;
										*value=scalar;
										value++;
										inherited_value += row_size;
									}
									value += row_size-i;
								}
								/* compress by reducing the column length */
								value=blending_matrix;
								inherited_value=blending_matrix;
								for (j=row_size;j>0;j--)
								{
									for (i=number_of_inherited_values;i>0;i--)
									{
										*inherited_value= *value;
										inherited_value++;
										value++;
									}
									value += row_size-number_of_inherited_values;
								}
								if (reorder_coordinate)
								{
									ALLOCATE(reorder_blending_matrix,FE_value,
										number_of_inherited_values*row_size);
									ALLOCATE(reorder_values,int,number_of_inherited_values);
									if (reorder_blending_matrix&&reorder_values)
									{
										value=blending_matrix;
										blending_matrix=reorder_blending_matrix;
										reorder_blending_matrix=value;
										inherited_value=blending_matrix;
										for (l=row_size;l>0;l--)
										{
											*inherited_value= *value;
											value += number_of_inherited_values;
											inherited_value += number_of_inherited_values;
										}
										reorder_value=reorder_values;
										*reorder_value=0;
										k=1;
										for (i=0;i<inherited_dimension;i++)
										{
											offset=1;
											inherited_standard_basis_argument=
												inherited_standard_basis_arguments;
											for (j=reorder_coordinate[i];j>0;j--)
											{
												inherited_standard_basis_argument++;
												order= *inherited_standard_basis_argument;
												if (order<0)
												{
													/* polygon */
													order= -order;
													if (order%2)
													{
														/* first polygon coordinate */
														order /= 2;
														polygon_offset=order%inherited_dimension;
														number_of_polygon_verticies=
															(-inherited_standard_basis_argument[
															polygon_offset])/2;
														offset *= 4*number_of_polygon_verticies;
													}
												}
												else
												{
													/* not polygon */
													offset *= order+1;
												}
											}
											inherited_standard_basis_argument++;
											order= *inherited_standard_basis_argument;
											if (order<0)
											{
												/* polygon */
												order= -order;
												if (order%2)
												{
													/* first polygon coordinate */
													order /= 2;
													polygon_offset=order%inherited_dimension;
													order=4*(-inherited_standard_basis_argument[
														polygon_offset])/2;
												}
												else
												{
													order=0;
												}
											}
											reorder_value=reorder_values;
											for (j=0;j<k*order;j++)
											{
												reorder_value[k]=(*reorder_value)+offset;
												inherited_value=blending_matrix+reorder_value[k];
												value=reorder_blending_matrix+(j+k);
												for (l=row_size;l>0;l--)
												{
													*inherited_value= *value;
													value += number_of_inherited_values;
													inherited_value += number_of_inherited_values;
												}
												reorder_value++;
											}
											k *= (order+1);
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"calculate_standard_basis_transformation.  "
											"Insufficient memory 2");
										return_code=0;
									}
									DEALLOCATE(reorder_values);
									DEALLOCATE(reorder_blending_matrix);
								}
							}
							if (return_code)
							{
								*number_of_inherited_values_address=number_of_inherited_values;
								*inherited_standard_basis_function_address=
									standard_basis_function;
								*inherited_arguments_address=inherited_standard_basis_arguments;
								*blending_matrix_address=blending_matrix;
							}
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"calculate_standard_basis_transformation.  Insufficient memory 1");
					return_code=0;
				}
				if (!return_code)
				{
					DEALLOCATE(blending_matrix);
					DEALLOCATE(inherited_standard_basis_arguments);
				}
				DEALLOCATE(field_to_element);
				DEALLOCATE(reorder_coordinate);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"calculate_standard_basis_transformation.  Invalid basis");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(WARNING_MESSAGE,
			"calculate_standard_basis_transformation.  Invalid argument(s)");
		return_code=0;
	}
#if defined (DEBUG_CODE)
	/*???debug */
	printf("leave calculate_standard_basis_transformation %d\n",return_code);
#endif /* defined (DEBUG_CODE) */
	LEAVE;

	return (return_code);
} /* calculate_standard_basis_transformation */

/*
Global functions
----------------
*/

int FE_basis_is_non_linear(struct FE_basis *basis)
{
	if (basis)
	{
		int dimensions, i, j;
		const int *type;
		type = basis->type;
		dimensions = *type;
		++type;
		for (i = 0; i < dimensions; i++)
		{
			/* Admittedly, even linear lagrange elements have non-linear terms for 2 or more dimensions */
			if ((FE_BASIS_CONSTANT != *type) &&
				(LINEAR_LAGRANGE != *type) &&
				(LINEAR_SIMPLEX != *type) &&
				(POLYGON != *type))
			{
				return 1;
			}
			for (j = i; j < dimensions; j++)
			{
				++type;
			}
		}
	}
	return 0;
}

struct FE_basis *CREATE(FE_basis)(int *type)
/*******************************************************************************
LAST MODIFIED : 02 April 2008

DESCRIPTION :
A basis is created with the specified <type> (duplicated).  The basis is
returned.
==============================================================================*/
{
	char valid_type;
	FE_value *blending_matrix = NULL,*polygon_blending_matrix,*reorder_1,*reorder_2,
		*temp_matrix;
	int *argument,*arguments,*basis_function_number,*basis_function_numbers = NULL,
		fn,i,j,k,l,need_reorder,new_func_count,new_std_func_count,
		number_of_basis_functions,
		number_of_polygon_verticies,number_of_standard_basis_functions,
		number_of_xi_coordinates,offset_1,offset_2,polygon_offset,*reorder_offsets,
		*reorder_xi,*reorder_xi_entry,simplex_dimension,simplex_offset,
		simplex_type,step_1,step_2,*temp_int_ptr_1,*temp_int_ptr_2,temp_int_1,
		temp_int_2,temp_int_3,*type_column,*type_entry,xi_coordinate;
	Standard_basis_function *standard_basis;
	struct FE_basis *basis;

	ENTER(CREATE(FE_basis));
	if (type&&((number_of_xi_coordinates= *type)>0))
	{
		/* check that the type is valid */
		if (ALLOCATE(arguments,int,number_of_xi_coordinates+1)&&
			ALLOCATE(blending_matrix,FE_value,1)&&
			ALLOCATE(basis_function_numbers,int,2*(number_of_xi_coordinates+1))&&
			ALLOCATE(reorder_xi,int,number_of_xi_coordinates))
		{
#if defined (DEBUG_CODE)
			/*???debug */
			printf("create basis : %d\n",number_of_xi_coordinates);
#endif /* defined (DEBUG_CODE) */
			*arguments=number_of_xi_coordinates;
			*blending_matrix=1;
			/* assign a (2*<number_of_xi_coordinates>+1)-tuple to each basis function
				so that can order the basis functions (rows of the blending matrix) with
				node value label varying fastest, xi1 varying next fastest, xi2
				varying next fastest and so on. These consist of a recorder per function:
				0 #xi xi1node# xi1deriv# xi2node# xi2deriv# ... xiNnode# xiNderiv#
				Later, sequential numbers are put in the first column and after sorting
				these give the index into the blending functions for the sorted index. */
			basis_function_number=basis_function_numbers;
			for (i=2*(number_of_xi_coordinates+1);i>0;i--)
			{
				*basis_function_number=0;
				basis_function_number++;
			}
			basis_function_numbers[1]=number_of_xi_coordinates;
			valid_type=1;
			number_of_basis_functions=1;
			number_of_standard_basis_functions=1;
			xi_coordinate=0;
			type_entry=type+1;
			standard_basis=monomial_basis_functions;
			polygon_offset = 0;
			/* for non-tensor product bases (simplex and polygon), the blending
				matrix is initially calculated as the tensor product of the current
				blending matrix and the blending matrix for the non-tensor product
				basis.  This implies that all the coordinates for the non-tensor
				product basis will be consecutive.  The coordinates (columns of the
				blending matrix) then have to be reordered */
			reorder_xi_entry=reorder_xi;
			argument=arguments+1;
			while (valid_type&&(xi_coordinate<number_of_xi_coordinates))
			{
				xi_coordinate++;
				switch (*type_entry)
				{
					case FE_BASIS_CONSTANT:
					{
						*reorder_xi_entry=xi_coordinate;
						reorder_xi_entry++;
						*argument=0;
						argument++;
						valid_type = 1;
					} break;
					case LINEAR_LAGRANGE:
					{
						if (NULL != (temp_matrix = tensor_product(2,2,
							linear_lagrange_blending_matrix,number_of_basis_functions,
							number_of_standard_basis_functions,blending_matrix)))
						{
							DEALLOCATE(blending_matrix);
							blending_matrix=temp_matrix;
							temp_int_1=number_of_basis_functions*
								2*(number_of_xi_coordinates+1);
							if (REALLOCATE(temp_int_ptr_1,basis_function_numbers,int,
								2*temp_int_1))
							{
								*reorder_xi_entry=xi_coordinate;
								reorder_xi_entry++;
								basis_function_numbers=temp_int_ptr_1;
								memcpy(basis_function_numbers+temp_int_1,basis_function_numbers,
									temp_int_1*sizeof(int));
								basis_function_number=basis_function_numbers+(temp_int_1+
									2*xi_coordinate);
								for (i=number_of_basis_functions;i>0;i--)
								{
									*basis_function_number=1;
									basis_function_number[1]=0;
									basis_function_number += 2*(number_of_xi_coordinates+1);
								}
								number_of_basis_functions *= 2;
								number_of_standard_basis_functions *= 2;
								*argument=1;
								argument++;
								valid_type=1;
							}
							else
							{
								valid_type=0;
							}
						}
						else
						{
							valid_type=0;
						}
					} break;
					case QUADRATIC_LAGRANGE:
					{
						if (NULL != (temp_matrix = tensor_product(3,3,
							quadratic_lagrange_blending_matrix,number_of_basis_functions,
							number_of_standard_basis_functions,blending_matrix)))
						{
							DEALLOCATE(blending_matrix);
							blending_matrix=temp_matrix;
							temp_int_1=number_of_basis_functions*
								2*(number_of_xi_coordinates+1);
							if (REALLOCATE(temp_int_ptr_1,basis_function_numbers,int,
								3*temp_int_1))
							{
								*reorder_xi_entry=xi_coordinate;
								reorder_xi_entry++;
								basis_function_numbers=temp_int_ptr_1;
								memcpy(basis_function_numbers+temp_int_1,basis_function_numbers,
									temp_int_1*sizeof(int));
								memcpy(basis_function_numbers+(2*temp_int_1),
									basis_function_numbers,temp_int_1*sizeof(int));
								basis_function_number=basis_function_numbers+(temp_int_1+
									2*xi_coordinate);
								for (i=number_of_basis_functions;i>0;i--)
								{
									*basis_function_number=1;
									basis_function_number[1]=0;
									basis_function_number[temp_int_1]=2;
									basis_function_number[temp_int_1+1]=0;
									basis_function_number += 2*(number_of_xi_coordinates+1);
								}
								number_of_basis_functions *= 3;
								number_of_standard_basis_functions *= 3;
								*argument=2;
								argument++;
								valid_type=1;
							}
							else
							{
								valid_type=0;
							}
						}
						else
						{
							valid_type=0;
						}
					} break;
					case CUBIC_LAGRANGE:
					{
						if (NULL != (temp_matrix = tensor_product(4,4,cubic_lagrange_blending_matrix,
							number_of_basis_functions,number_of_standard_basis_functions,
							blending_matrix)))
						{
							DEALLOCATE(blending_matrix);
							blending_matrix=temp_matrix;
							temp_int_1=number_of_basis_functions*
								2*(number_of_xi_coordinates+1);
							if (REALLOCATE(temp_int_ptr_1,basis_function_numbers,int,
								4*temp_int_1))
							{
								*reorder_xi_entry=xi_coordinate;
								reorder_xi_entry++;
								basis_function_numbers=temp_int_ptr_1;
								memcpy(basis_function_numbers+temp_int_1,basis_function_numbers,
									temp_int_1*sizeof(int));
								memcpy(basis_function_numbers+(2*temp_int_1),
									basis_function_numbers,temp_int_1*sizeof(int));
								memcpy(basis_function_numbers+(3*temp_int_1),
									basis_function_numbers,temp_int_1*sizeof(int));
								basis_function_number=basis_function_numbers+(temp_int_1+
									2*xi_coordinate);
								for (i=number_of_basis_functions;i>0;i--)
								{
									*basis_function_number=1;
									basis_function_number[1]=0;
									basis_function_number[temp_int_1]=2;
									basis_function_number[temp_int_1+1]=0;
									basis_function_number[2*temp_int_1]=3;
									basis_function_number[2*temp_int_1+1]=0;
									basis_function_number += 2*(number_of_xi_coordinates+1);
								}
								number_of_basis_functions *= 4;
								number_of_standard_basis_functions *= 4;
								*argument=3;
								argument++;
								valid_type=1;
							}
							else
							{
								valid_type=0;
							}
						}
						else
						{
							valid_type=0;
						}
					} break;
					case CUBIC_HERMITE:
					{
						if (NULL != (temp_matrix = tensor_product(4,4,cubic_hermite_blending_matrix,
							number_of_basis_functions,number_of_standard_basis_functions,
							blending_matrix)))
						{
							DEALLOCATE(blending_matrix);
							blending_matrix=temp_matrix;
							temp_int_1=number_of_basis_functions*
								2*(number_of_xi_coordinates+1);
							if (REALLOCATE(temp_int_ptr_1,basis_function_numbers,int,
								4*temp_int_1))
							{
								*reorder_xi_entry=xi_coordinate;
								reorder_xi_entry++;
								basis_function_numbers=temp_int_ptr_1;
								memcpy(basis_function_numbers+temp_int_1,basis_function_numbers,
									temp_int_1*sizeof(int));
								memcpy(basis_function_numbers+(2*temp_int_1),
									basis_function_numbers,temp_int_1*sizeof(int));
								memcpy(basis_function_numbers+(3*temp_int_1),
									basis_function_numbers,temp_int_1*sizeof(int));
								basis_function_number=basis_function_numbers+(temp_int_1+
									2*xi_coordinate);
								for (i=number_of_basis_functions;i>0;i--)
								{
									*basis_function_number=0;
									basis_function_number[1]=1;
									basis_function_number[temp_int_1]=1;
									basis_function_number[temp_int_1+1]=0;
									basis_function_number[2*temp_int_1]=1;
									basis_function_number[2*temp_int_1+1]=1;
									basis_function_number += 2*(number_of_xi_coordinates+1);
								}
								number_of_basis_functions *= 4;
								number_of_standard_basis_functions *= 4;
								*argument=3;
								argument++;
								valid_type=1;
							}
							else
							{
								valid_type=0;
							}
						}
						else
						{
							valid_type=0;
						}
					} break;
					case HERMITE_LAGRANGE:
					{
						if (NULL != (temp_matrix = tensor_product(3,3,
							hermite_lagrange_blending_matrix,number_of_basis_functions,
							number_of_standard_basis_functions,blending_matrix)))
						{
							DEALLOCATE(blending_matrix);
							blending_matrix=temp_matrix;
							temp_int_1=number_of_basis_functions*
								2*(number_of_xi_coordinates+1);
							if (REALLOCATE(temp_int_ptr_1,basis_function_numbers,int,
								3*temp_int_1))
							{
								*reorder_xi_entry=xi_coordinate;
								reorder_xi_entry++;
								basis_function_numbers=temp_int_ptr_1;
								memcpy(basis_function_numbers+temp_int_1,basis_function_numbers,
									temp_int_1*sizeof(int));
								memcpy(basis_function_numbers+(2*temp_int_1),
									basis_function_numbers,temp_int_1*sizeof(int));
								basis_function_number=basis_function_numbers+(temp_int_1+
									2*xi_coordinate);
								for (i=number_of_basis_functions;i>0;i--)
								{
									*basis_function_number=0;
									basis_function_number[1]=1;
									basis_function_number[temp_int_1]=1;
									basis_function_number[temp_int_1+1]=0;
									basis_function_number += 2*(number_of_xi_coordinates+1);
								}
								number_of_basis_functions *= 3;
								number_of_standard_basis_functions *= 3;
								*argument=2;
								argument++;
								valid_type=1;
							}
							else
							{
								valid_type=0;
							}
						}
						else
						{
							valid_type=0;
						}
					} break;
					case LAGRANGE_HERMITE:
					{
						if (NULL != (temp_matrix = tensor_product(3,3,
							lagrange_hermite_blending_matrix,number_of_basis_functions,
							number_of_standard_basis_functions,blending_matrix)))
						{
							DEALLOCATE(blending_matrix);
							blending_matrix=temp_matrix;
							temp_int_1=number_of_basis_functions*
								2*(number_of_xi_coordinates+1);
							if (REALLOCATE(temp_int_ptr_1,basis_function_numbers,int,
								3*temp_int_1))
							{
								*reorder_xi_entry=xi_coordinate;
								reorder_xi_entry++;
								basis_function_numbers=temp_int_ptr_1;
								memcpy(basis_function_numbers+temp_int_1,basis_function_numbers,
									temp_int_1*sizeof(int));
								memcpy(basis_function_numbers+(2*temp_int_1),
									basis_function_numbers,temp_int_1*sizeof(int));
								basis_function_number=basis_function_numbers+(temp_int_1+
									2*xi_coordinate);
								for (i=number_of_basis_functions;i>0;i--)
								{
									*basis_function_number=1;
									basis_function_number[1]=0;
									basis_function_number[temp_int_1]=1;
									basis_function_number[temp_int_1+1]=1;
									basis_function_number += 2*(number_of_xi_coordinates+1);
								}
								number_of_basis_functions *= 3;
								number_of_standard_basis_functions *= 3;
								*argument=2;
								argument++;
								valid_type=1;
							}
							else
							{
								valid_type=0;
							}
						}
						else
						{
							valid_type=0;
						}
					} break;
					case POLYGON:
					{
						/* check if the other polygon coordinate is before */
						valid_type=2;
						number_of_polygon_verticies=0;
						type_column=type_entry;
						i=xi_coordinate-1;
						j=number_of_xi_coordinates-xi_coordinate;
						while (valid_type&&(i>0))
						{
							j++;
							type_column -= j;
							if (NO_RELATION!= *type_column)
							{
								if (0<number_of_polygon_verticies)
								{
									valid_type=0;
								}
								else
								{
									if ((number_of_polygon_verticies= *type_column)<3)
									{
										valid_type=0;
									}
								}
							}
							i--;
						}
						if (valid_type)
						{
							if (0<number_of_polygon_verticies)
							{
								/* make sure that it is not connected to a following xi */
								type_entry++;
								i=number_of_xi_coordinates-xi_coordinate;
								while (valid_type&&(i>0))
								{
									if (NO_RELATION!= *type_entry)
									{
										valid_type=0;
									}
									type_entry++;
									i--;
								}
							}
							else
							{
								/* check if the other polygon coordinate is after */
								type_entry++;
								i=number_of_xi_coordinates-xi_coordinate;
								while (valid_type&&(i>0))
								{
									if (NO_RELATION!= *type_entry)
									{
										if (0<number_of_polygon_verticies)
										{
											valid_type=0;
										}
										else
										{
											if ((number_of_polygon_verticies= *type_entry)>2)
											{
												polygon_offset=number_of_xi_coordinates-xi_coordinate+
													1-i;
												if (POLYGON!=type_entry[(polygon_offset*
													(2*(number_of_xi_coordinates-xi_coordinate+1)-
													polygon_offset+1))/2-polygon_offset])
												{
													valid_type=0;
												}
											}
											else
											{
												valid_type=0;
											}
										}
									}
									type_entry++;
									i--;
								}
								if (valid_type&&(0<number_of_polygon_verticies))
								{
									*reorder_xi_entry=xi_coordinate;
									reorder_xi_entry++;
									*reorder_xi_entry=xi_coordinate+polygon_offset;
									reorder_xi_entry++;
									/* see polygon_basis_functions for the polygon blending
										matrix */
									if (ALLOCATE(polygon_blending_matrix,FE_value,
										(number_of_polygon_verticies+1)*
										4*number_of_polygon_verticies))
									{
										temp_matrix=polygon_blending_matrix;
										for (j=number_of_polygon_verticies;j>0;j--)
										{
											*temp_matrix=1;
											temp_matrix++;
										}
										for (j=number_of_polygon_verticies;j>0;j--)
										{
											*temp_matrix=0;
											temp_matrix++;
										}
										for (j=number_of_polygon_verticies;j>0;j--)
										{
											*temp_matrix= -1;
											temp_matrix++;
										}
										for (j=(4*number_of_polygon_verticies+1)*
											number_of_polygon_verticies;j>0;j--)
										{
											*temp_matrix=0;
											temp_matrix++;
										}
										temp_matrix=polygon_blending_matrix+
											(6*number_of_polygon_verticies);
										*temp_matrix=1;
										temp_matrix += number_of_polygon_verticies;
										*temp_matrix= -1;
										temp_matrix[number_of_polygon_verticies-1]=1;
										step_1=3*number_of_polygon_verticies+1;
										step_2=number_of_polygon_verticies-1;
										for (j=number_of_polygon_verticies-1;j>0;j--)
										{
											temp_matrix += step_1;
											*temp_matrix=1;
											temp_matrix += step_2;
											*temp_matrix=1;
											temp_matrix++;
											*temp_matrix= -1;
										}
										if (NULL != (temp_matrix = tensor_product(
											number_of_polygon_verticies+1,
											4*number_of_polygon_verticies,polygon_blending_matrix,
											number_of_basis_functions,
											number_of_standard_basis_functions,blending_matrix)))
										{
											DEALLOCATE(blending_matrix);
											blending_matrix=temp_matrix;
											temp_int_1=number_of_basis_functions*
												2*(number_of_xi_coordinates+1);
											if (REALLOCATE(temp_int_ptr_1,basis_function_numbers,int,
												(number_of_polygon_verticies+1)*temp_int_1))
											{
												basis_function_numbers=temp_int_ptr_1;
												for (i=number_of_polygon_verticies;i>0;i--)
												{
													memcpy(basis_function_numbers+(i*temp_int_1),
														basis_function_numbers,temp_int_1*sizeof(int));
												}
												basis_function_number=basis_function_numbers+
													(2*xi_coordinate);
												for (i=number_of_basis_functions;i>0;i--)
												{
													for (j=number_of_polygon_verticies;j>0;j--)
													{
														basis_function_number[j*temp_int_1]=j;
														basis_function_number[j*temp_int_1+1]=0;
													}
													basis_function_number +=
														2*(number_of_xi_coordinates+1);
												}
												number_of_basis_functions *=
													number_of_polygon_verticies+1;
												number_of_standard_basis_functions *=
													4*number_of_polygon_verticies;
												*argument= -(1+2*(1/*polygon_offset*/+1/*order*/*
													number_of_xi_coordinates));
												argument++;
												*argument= -2*number_of_polygon_verticies;
												argument++;
												standard_basis=polygon_basis_functions;
												valid_type=2;
											}
											else
											{
												valid_type=0;
											}
										}
										else
										{
											valid_type=0;
										}
										DEALLOCATE(polygon_blending_matrix);
									}
									else
									{
										valid_type=0;
									}
								}
								else
								{
									valid_type=0;
								}
							}
						}
					} break;
					case LINEAR_SIMPLEX:
					case QUADRATIC_SIMPLEX:
					{
						int simplex_order = 0;
						/* simplex */
						/* to avoid increment/check of row */
						valid_type=2;
						/* determine if this is the first component of the simplex */
						simplex_dimension=1;
						type_column=type_entry;
						i=xi_coordinate-1;
						j=number_of_xi_coordinates-xi_coordinate;
						while (valid_type&&(i>0))
						{
							j++;
							type_column -= j;
							if (NO_RELATION!= *type_column)
							{
								simplex_dimension++;
							}
							i--;
						}
						if (1==simplex_dimension)
						{
							/* first component of the simplex */
							*reorder_xi_entry=xi_coordinate;
							reorder_xi_entry++;
							simplex_type= *type_entry;
								/*???DB.  Maybe able to remove if can work how to calculate the
									blending matrix for an arbitrary order */
							/* determine the simplex dimension */
							type_entry++;
							i=1;
							while (valid_type && (i<=number_of_xi_coordinates-xi_coordinate))
							{
								if (NO_RELATION!= *type_entry)
								{
									simplex_offset=i;
									if (simplex_type==type_entry[simplex_offset*
										(2*(number_of_xi_coordinates-xi_coordinate+1)+
										(1-simplex_offset))/2-i])
									{
										*reorder_xi_entry=xi_coordinate+i;
										reorder_xi_entry++;
										simplex_dimension++;
									}
									else
									{
										valid_type = 0;
									}
								}
								type_entry++;
								i++;
							}
							if (valid_type && (2<=simplex_dimension))
							{
								/*???DB.  Should be able to calculate the blending matrix for
									arbitrary dimension and arbitrary order, but get the basics
									going first */
								switch (simplex_type)
								{
									case LINEAR_SIMPLEX:
									{
										simplex_order=1;
										switch (simplex_dimension)
										{
											case 2:
											{
												if (NULL != (temp_matrix = tensor_product(3,4,
													linear_simplex_2d_blending_matrix,
													number_of_basis_functions,
													number_of_standard_basis_functions,blending_matrix)))
												{
													DEALLOCATE(blending_matrix);
													blending_matrix=temp_matrix;
													temp_int_1=number_of_basis_functions*
														2*(number_of_xi_coordinates+1);
													if (REALLOCATE(temp_int_ptr_1,basis_function_numbers,
														int,3*temp_int_1))
													{
														basis_function_numbers=temp_int_ptr_1;
														memcpy(basis_function_numbers+temp_int_1,
															basis_function_numbers,temp_int_1*sizeof(int));
														memcpy(basis_function_numbers+(2*temp_int_1),
															basis_function_numbers,temp_int_1*sizeof(int));
														basis_function_number=basis_function_numbers+
															(temp_int_1+2*xi_coordinate);
														temp_int_2=temp_int_1+
															2*(*(reorder_xi_entry-1)-xi_coordinate);
														for (i=number_of_basis_functions;i>0;i--)
														{
															*basis_function_number=1;
															basis_function_number[1]=0;
															basis_function_number[temp_int_2]=1;
															basis_function_number[temp_int_2+1]=0;
															basis_function_number +=
																2*(number_of_xi_coordinates+1);
														}
														number_of_basis_functions *= 3;
														number_of_standard_basis_functions *= 4;
													}
													else
													{
														valid_type=0;
													}
												}
												else
												{
													valid_type=0;
												}
											} break;
											case 3:
											{
												if (NULL != (temp_matrix = tensor_product(4,8,
													linear_simplex_3d_blending_matrix,
													number_of_basis_functions,
													number_of_standard_basis_functions,blending_matrix)))
												{
													DEALLOCATE(blending_matrix);
													blending_matrix=temp_matrix;
													temp_int_1=number_of_basis_functions*
														2*(number_of_xi_coordinates+1);
													if (REALLOCATE(temp_int_ptr_1,basis_function_numbers,
														int,4*temp_int_1))
													{
														basis_function_numbers=temp_int_ptr_1;
														memcpy(basis_function_numbers+temp_int_1,
															basis_function_numbers,temp_int_1*sizeof(int));
														memcpy(basis_function_numbers+(2*temp_int_1),
															basis_function_numbers,temp_int_1*sizeof(int));
														memcpy(basis_function_numbers+(3*temp_int_1),
															basis_function_numbers,temp_int_1*sizeof(int));
														basis_function_number=basis_function_numbers+
															(temp_int_1+2*xi_coordinate);
														temp_int_2=temp_int_1+
															2*(*(reorder_xi_entry-2)-xi_coordinate);
														temp_int_3=2*temp_int_1+
															2*(*(reorder_xi_entry-1)-xi_coordinate);
														for (i=number_of_basis_functions;i>0;i--)
														{
															*basis_function_number=1;
															basis_function_number[1]=0;
															basis_function_number[temp_int_2]=1;
															basis_function_number[temp_int_2+1]=0;
															basis_function_number[temp_int_3]=1;
															basis_function_number[temp_int_3+1]=0;
															basis_function_number +=
																2*(number_of_xi_coordinates+1);
														}
														number_of_basis_functions *= 4;
														number_of_standard_basis_functions *= 8;
													}
													else
													{
														valid_type=0;
													}
												}
												else
												{
													valid_type=0;
												}
											} break;
											default:
											{
												valid_type=0;
											} break;
										}
									} break;
									case QUADRATIC_SIMPLEX:
									{
										simplex_order=2;
										switch (simplex_dimension)
										{
											case 2:
											{
												if (NULL != (temp_matrix = tensor_product(6,9,
													quadratic_simplex_2d_blending_matrix,
													number_of_basis_functions,
													number_of_standard_basis_functions,blending_matrix)))
												{
													DEALLOCATE(blending_matrix);
													blending_matrix=temp_matrix;
													temp_int_1=number_of_basis_functions*
														2*(number_of_xi_coordinates+1);
													if (REALLOCATE(temp_int_ptr_1,basis_function_numbers,
														int,6*temp_int_1))
													{
														basis_function_numbers=temp_int_ptr_1;
														memcpy(basis_function_numbers+temp_int_1,
															basis_function_numbers,temp_int_1*sizeof(int));
														memcpy(basis_function_numbers+(2*temp_int_1),
															basis_function_numbers,temp_int_1*sizeof(int));
														memcpy(basis_function_numbers+(3*temp_int_1),
															basis_function_numbers,temp_int_1*sizeof(int));
														memcpy(basis_function_numbers+(4*temp_int_1),
															basis_function_numbers,temp_int_1*sizeof(int));
														memcpy(basis_function_numbers+(5*temp_int_1),
															basis_function_numbers,temp_int_1*sizeof(int));
														basis_function_number=basis_function_numbers+
															(temp_int_1+2*xi_coordinate);
														temp_int_2=2*(*(reorder_xi_entry-1)-xi_coordinate);
														for (i=number_of_basis_functions;i>0;i--)
														{
															*basis_function_number=1;
															basis_function_number[1]=0;
															basis_function_number[temp_int_1]=2;
															basis_function_number[temp_int_1+1]=0;
															basis_function_number[2*temp_int_1+temp_int_2]=1;
															basis_function_number[2*temp_int_1+temp_int_2+1]=
																0;
															basis_function_number[3*temp_int_1]=1;
															basis_function_number[3*temp_int_1+1]=0;
															basis_function_number[3*temp_int_1+temp_int_2]=1;
															basis_function_number[3*temp_int_1+temp_int_2+1]=
																0;
															basis_function_number[4*temp_int_1+temp_int_2]=2;
															basis_function_number[4*temp_int_1+temp_int_2+1]=
																0;
															basis_function_number +=
																2*(number_of_xi_coordinates+1);
														}
														number_of_basis_functions *= 6;
														number_of_standard_basis_functions *= 9;
													}
													else
													{
														valid_type=0;
													}
												}
												else
												{
													valid_type=0;
												}
											} break;
											case 3:
											{
												new_func_count = 10;
												new_std_func_count = 27;
												if (NULL != (temp_matrix = tensor_product(new_func_count,new_std_func_count,
													quadratic_simplex_3d_blending_matrix,
													number_of_basis_functions,
													number_of_standard_basis_functions,blending_matrix)))
												{
													DEALLOCATE(blending_matrix);
													blending_matrix=temp_matrix;
													temp_int_1=number_of_basis_functions*
														2*(number_of_xi_coordinates+1);
													if (REALLOCATE(temp_int_ptr_1,basis_function_numbers,
														int,new_func_count*temp_int_1))
													{
														basis_function_numbers=temp_int_ptr_1;
														for (fn=1; fn<new_func_count; ++fn)
														{
															memcpy(basis_function_numbers+(fn*temp_int_1),
																basis_function_numbers,temp_int_1*sizeof(int));
														}
														basis_function_number=basis_function_numbers+
															(temp_int_1+2*xi_coordinate);
														temp_int_2=2*(*(reorder_xi_entry-2)-xi_coordinate);
														temp_int_3=2*(*(reorder_xi_entry-1)-xi_coordinate);
														for (i=number_of_basis_functions;i>0;i--)
														{
															*basis_function_number=1;
															basis_function_number[1]=0;
															basis_function_number[temp_int_1]=2;
															basis_function_number[temp_int_1+1]=0;
															basis_function_number[2*temp_int_1+temp_int_2]=1;
															basis_function_number[2*temp_int_1+temp_int_2+1]=0;

															basis_function_number[3*temp_int_1]=1;
															basis_function_number[3*temp_int_1+1]=0;
															basis_function_number[3*temp_int_1+temp_int_2]=1;
															basis_function_number[3*temp_int_1+temp_int_2+1]=0;

															basis_function_number[4*temp_int_1+temp_int_2]=2;
															basis_function_number[4*temp_int_1+temp_int_2+1]=0;

															basis_function_number[5*temp_int_1+temp_int_3]=1;
															basis_function_number[5*temp_int_1+temp_int_3+1]=0;

															basis_function_number[6*temp_int_1]=1;
															basis_function_number[6*temp_int_1+1]=0;
															basis_function_number[6*temp_int_1+temp_int_3]=1;
															basis_function_number[6*temp_int_1+temp_int_3+1]=0;

															basis_function_number[7*temp_int_1+temp_int_2]=1;
															basis_function_number[7*temp_int_1+temp_int_2+1]=0;
															basis_function_number[7*temp_int_1+temp_int_3]=1;
															basis_function_number[7*temp_int_1+temp_int_3+1]=0;

															basis_function_number[8*temp_int_1+temp_int_3]=2;
															basis_function_number[8*temp_int_1+temp_int_3+1]=0;

															basis_function_number += 2*(number_of_xi_coordinates+1);
														}
														number_of_basis_functions *= new_func_count;
														number_of_standard_basis_functions *= new_std_func_count;
													}
													else
													{
														valid_type=0;
													}
												}
												else
												{
													valid_type=0;
												}
											} break;
											default:
											{
												valid_type=0;
											} break;
										}
									} break;
									default:
									{
										valid_type=0;
									} break;
								}
								for (i=simplex_dimension;i>0;i--)
								{
									*argument=simplex_order;
									argument++;
								}
							}
							else
							{
								valid_type=0;
							}
						}
						else
						{
							/* skip rest of row */
							type_entry += number_of_xi_coordinates-xi_coordinate+1;
						}
					} break;
					default:
					{
						valid_type=0;
					} break;
				}
				if (1==valid_type)
				{
					/* 1-dimensional basis component */
					type_entry++;
					i=number_of_xi_coordinates-xi_coordinate;
					while (valid_type&&(i>0))
					{
						if (NO_RELATION!= *type_entry)
						{
							valid_type=0;
						}
						type_entry++;
						i--;
					}
				}
			}
			if (valid_type)
			{
				ALLOCATE(basis, struct FE_basis, 1);
				if (basis)
				{
					ALLOCATE(basis->type, int, 1+number_of_xi_coordinates*(number_of_xi_coordinates+1)/2);
					ALLOCATE(basis->blending_matrix, FE_value, number_of_basis_functions*number_of_standard_basis_functions);
					ALLOCATE(basis->blending_matrix_column_size, int, number_of_standard_basis_functions);
					ALLOCATE(basis->parameterNodes, int, number_of_basis_functions);
					ALLOCATE(basis->parameterDerivatives, int, number_of_basis_functions);
				}
				if ((basis) && (basis->type) && (basis->blending_matrix) && (basis->blending_matrix_column_size) &&
					(basis->parameterNodes) && (basis->parameterDerivatives))
				{
					/* reorder the xi coordinates */
					need_reorder=0;
					for (i=0;i<number_of_xi_coordinates;i++)
					{
						if (i+1!=reorder_xi[i])
						{
							need_reorder=1;
						}
						reorder_xi[i]=number_of_xi_coordinates*reorder_xi[i]+i;
					}
					if (need_reorder)
					{
						ALLOCATE(reorder_offsets,int,number_of_standard_basis_functions);
						ALLOCATE(temp_int_ptr_1,int,number_of_xi_coordinates+1);
						if (reorder_offsets&&temp_int_ptr_1)
						{
							heapsort((void *)reorder_xi,number_of_xi_coordinates,sizeof(int),
								sort_integers);
							for (i=0;i<number_of_xi_coordinates;i++)
							{
								reorder_xi[i] %= number_of_xi_coordinates;
							}
							reorder_1=basis->blending_matrix;
							reorder_2=blending_matrix;
							for (i=number_of_basis_functions;i>0;i--)
							{
								*reorder_1= *reorder_2;
								reorder_1 += number_of_standard_basis_functions;
								reorder_2 += number_of_standard_basis_functions;
							}
							offset_1=1;
							reorder_offsets[0]=0;
							for (i=0;i<number_of_xi_coordinates;i++)
							{
								offset_2=1;
								for (j=reorder_xi[i];j>0;j--)
								{
									if ((temp_int_1=arguments[j])<0)
									{
										/* polygon */
										temp_int_1= -temp_int_1;
										if (temp_int_1%2)
										{
											/* first polygon coordinate */
											/* do nothing for second because all standard basis
												functions carried by first */
											temp_int_1 /= 2;
											temp_int_1 %= number_of_xi_coordinates;
											offset_2 *= 4*(-arguments[j+temp_int_1])/2;
										}
									}
									else
									{
										offset_2 *= temp_int_1+1;
									}
								}
								if ((temp_int_1=arguments[reorder_xi[i]+1])<0)
								{
									/* polygon */
									temp_int_1= -temp_int_1;
									if (temp_int_1%2)
									{
										/* first polygon coordinate */
										temp_int_1 /= 2;
										temp_int_1 %= number_of_xi_coordinates;
										temp_int_1=4*(-arguments[reorder_xi[i]+1+temp_int_1])/2-1;
									}
									else
									{
										/* do nothing for second because all standard basis
											functions carried by first */
										temp_int_1=0;
									}
								}
								for (j=1;j<=temp_int_1;j++)
								{
									for (k=0;k<offset_1;k++)
									{
										reorder_1=basis->blending_matrix+j*offset_1+k;
										reorder_offsets[j*offset_1+k]=
											reorder_offsets[(j-1)*offset_1+k]+offset_2;
										reorder_2=blending_matrix+reorder_offsets[j*offset_1+k];
										for (l=number_of_basis_functions;l>0;l--)
										{
											*reorder_1= *reorder_2;
											reorder_1 += number_of_standard_basis_functions;
											reorder_2 += number_of_standard_basis_functions;
										}
									}
								}
								offset_1 *= temp_int_1+1;
							}
							reorder_1=basis->blending_matrix;
							basis->blending_matrix=blending_matrix;
							blending_matrix=reorder_1;
							temp_int_ptr_1[0]=arguments[0];
							/* don't need to allow for reordering polygon coordinates because
								the exelem format only allows specification of bases with
								circumferential first */
							for (i=number_of_xi_coordinates;i>0;i--)
							{
								if ((temp_int_1=arguments[reorder_xi[i-1]+1])<0)
								{
									/* polygon */
									if ((-temp_int_1)%2)
									{
										temp_int_1= -temp_int_1;
										/* first polygon coordinate */
										temp_int_1 /= 2;
										polygon_offset=temp_int_1%number_of_xi_coordinates;
										temp_int_1 /= number_of_xi_coordinates;
										polygon_offset=reorder_xi[i-1+polygon_offset]-
											reorder_xi[i-1];
										temp_int_1= -(1+2*(polygon_offset+
											temp_int_1*number_of_xi_coordinates));
									}
								}
								temp_int_ptr_1[i]=temp_int_1;
							}
							temp_int_ptr_2=arguments;
							arguments=temp_int_ptr_1;
							temp_int_ptr_1=temp_int_ptr_2;
						}
						else
						{
							display_message(ERROR_MESSAGE, "CREATE(FE_basis).  Could not allocate memory for reordering xi");
							DEALLOCATE(basis->parameterDerivatives);
							DEALLOCATE(basis->parameterNodes);
							DEALLOCATE(basis->blending_matrix_column_size);
							DEALLOCATE(basis->blending_matrix);
							DEALLOCATE(basis->type);
							DEALLOCATE(arguments);
							DEALLOCATE(basis);
							basis = 0;
						}
						DEALLOCATE(reorder_offsets);
						DEALLOCATE(temp_int_ptr_1);
					}
					if (basis)
					{
						basis->access_count=0;
						basis->manager = (struct MANAGER(FE_basis) *)NULL;
						basis->manager_change_status = MANAGER_CHANGE_NONE(FE_basis);

						/* copy the basis type */
						int *basis_type = basis->type;
						type_entry = type;
						for (i = 1+number_of_xi_coordinates*(number_of_xi_coordinates+1)/2-1; i>=0; --i)
						{
							*basis_type= *type_entry;
							basis_type++;
							type_entry++;
						}
						basis->number_of_basis_functions=number_of_basis_functions;
						basis->number_of_standard_basis_functions=
							number_of_standard_basis_functions;
						/* reorder the basis functions */
						basis_function_number=basis_function_numbers;
						for (i=0;i<number_of_basis_functions;i++)
						{
							*basis_function_number=i;
							basis_function_number += 2*(number_of_xi_coordinates+1);
						}
						heapsort((void *)basis_function_numbers,number_of_basis_functions,
							2*(number_of_xi_coordinates+1)*sizeof(int),sort_basis_functions);
						/* reorder the blending matrix */
						reorder_1=basis->blending_matrix;
						basis_function_number=basis_function_numbers;
						for (i=0;i<number_of_basis_functions;i++)
						{
							reorder_2=blending_matrix+(((*basis_function_number)%
								number_of_basis_functions)*number_of_standard_basis_functions);
							for (j=0;j<number_of_standard_basis_functions;j++)
							{
								*reorder_1=  *reorder_2;
								reorder_1++;
								reorder_2++;
							}
							basis_function_number += 2*(number_of_xi_coordinates+1);
						}
						// calculate number of nodes and derivative per node */
						basis_function_number = basis_function_numbers;
						int *previous_basis_function_number = basis_function_number;
						int derivativeType;
						int nodeNumber = 0;
						for (i = 0; i < number_of_basis_functions; ++i)
						{
							for (j = 0; j < number_of_xi_coordinates; ++j)
								if (basis_function_number[2*j + 2] != previous_basis_function_number[2*j + 2])
								{
									++nodeNumber;
									break;
								}
							basis->parameterNodes[i] = nodeNumber;
							derivativeType = 0;
							for (j = 0; j < number_of_xi_coordinates; ++j)
								if (basis_function_number[2*j + 3])
									derivativeType += (1 << j);
							basis->parameterDerivatives[i] = derivativeType;
							previous_basis_function_number = basis_function_number;
							basis_function_number += 2*(number_of_xi_coordinates+1);
						}
						/* calculate the size of column containing non-zero entries, to reduce
							dot product calculation in global_to_element_map_values() */
						for (j=0;j<number_of_standard_basis_functions;j++)
						{
							reorder_1 = basis->blending_matrix + number_of_basis_functions*number_of_standard_basis_functions + j;
							for (i=number_of_basis_functions;i>0;i--)
							{
								reorder_1 -= number_of_standard_basis_functions;
								if (*reorder_1 != 0.0)
									break;
							}
							basis->blending_matrix_column_size[j] = i;
						}
/*???debug */
/*{
	FE_value *value;
	int i,j;

	printf("%d) %d %d\n",xi_coordinate,number_of_basis_functions,
		number_of_standard_basis_functions);
	value=basis->blending_matrix;
	for (i=number_of_standard_basis_functions;i>0;i--)
	{
		for (j=number_of_basis_functions;j>0;j--)
		{
			printf("%g ",*value);
			value++;
		}
		printf("\n");
	}
	for (i=0;i<number_of_basis_functions;i++)
	{
		printf("%d ",basis_function_numbers[i]);
	}
	printf("\n");
	for (i=0;i<=xi_coordinate;i++)
	{
		printf("%d ",arguments[i]);
	}
	printf("\n");
	printf("%p %p %p\n",standard_basis,monomial_basis_functions,
		polygon_basis_functions);
}*/
						basis->arguments=arguments;
						basis->standard_basis=standard_basis;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"CREATE(FE_basis).  Could not allocate memory for basis");
					if (basis)
					{
						DEALLOCATE(basis->parameterDerivatives);
						DEALLOCATE(basis->parameterNodes);
						DEALLOCATE(basis->blending_matrix_column_size);
						DEALLOCATE(basis->blending_matrix);
						DEALLOCATE(basis->type);
						DEALLOCATE(basis);
					}
					DEALLOCATE(arguments);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"CREATE(FE_basis).  Invalid basis type");
				DEALLOCATE(arguments);
				basis=(struct FE_basis *)NULL;
			}
			DEALLOCATE(basis_function_numbers);
			DEALLOCATE(blending_matrix);
			DEALLOCATE(reorder_xi);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(FE_basis).  Could not allocate memory for basis calculation");
			if (arguments)
			{
				DEALLOCATE(arguments);
				if (blending_matrix)
				{
					DEALLOCATE(blending_matrix);
					DEALLOCATE(basis_function_numbers);
				}
			}
			basis=(struct FE_basis *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(FE_basis).  Invalid argument(s)");
		basis=(struct FE_basis *)NULL;
	}
	LEAVE;

	return (basis);
} /* CREATE(FE_basis) */

int DESTROY(FE_basis)(struct FE_basis **basis_address)
/*******************************************************************************
LAST MODIFIED : 1 October 1995

DESCRIPTION :
Frees the memory for the basis and sets <*basis_address> to NULL.
==============================================================================*/
{
	int return_code;
	struct FE_basis *basis;

	ENTER(DESTROY(FE_basis));
	/* check the arguments */
	if ((basis_address)&&(basis= *basis_address))
	{
		if (0==basis->access_count)
		{
			DEALLOCATE(basis->type);
			DEALLOCATE(basis->blending_matrix);
			DEALLOCATE(basis->blending_matrix_column_size);
			DEALLOCATE(basis->parameterNodes);
			DEALLOCATE(basis->parameterDerivatives);
			DEALLOCATE(basis->arguments);
			DEALLOCATE(*basis_address);
			return_code=1;
		}
		else
		{
			return_code=1;
			*basis_address=(struct FE_basis *)NULL;
		}
	}
	else
	{
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(FE_basis) */

struct FE_basis *make_FE_basis(int *basis_type,
	struct MANAGER(FE_basis) *basis_manager )
{
	struct FE_basis *basis = 0;
	if (basis_type && basis_manager)
	{
		basis = FIND_BY_IDENTIFIER_IN_MANAGER(FE_basis,type)(basis_type,basis_manager);
		if (NULL == basis)
		{
			basis = CREATE(FE_basis)(basis_type);
			if (NULL != basis)
			{
				if (!ADD_OBJECT_TO_MANAGER(FE_basis)(basis,basis_manager))
				{
					DESTROY(FE_basis)(&basis);
					display_message(ERROR_MESSAGE,
						"make_FE_basis. Could not add basis to manager");
					basis = (struct FE_basis *)NULL;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"make_FE_basis Could not create a new basis");
				basis = (struct FE_basis *)NULL;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"make_FE_basis. Invalid arguments");
	}
	return (basis);
}

FE_basis *FE_basis_get_connectivity_basis(FE_basis *feBasis)
{
	int basisDimension = 0;
	if (!FE_basis_get_dimension(feBasis, &basisDimension))
		return 0;
	bool differentBasis = false;
	const int basisTypeSize = 1 + (basisDimension*(basisDimension + 1) / 2);
	int *basisType = new int[basisTypeSize];
	basisType[0] = basisDimension;
	for (int i = 1; i < basisTypeSize; ++i)
	{
		switch (feBasis->type[i])
		{
			case CUBIC_HERMITE:
			case LAGRANGE_HERMITE:
			case HERMITE_LAGRANGE:
				basisType[i] = LINEAR_LAGRANGE;
				differentBasis = true;
				break;
			default:
				basisType[i] = feBasis->type[i];
				break;
		}
	}
	if (differentBasis)
		return make_FE_basis(basisType, feBasis->manager);
	return feBasis;
}

DECLARE_OBJECT_FUNCTIONS(FE_basis)

DECLARE_INDEXED_LIST_FUNCTIONS(FE_basis)

DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(FE_basis,type,int *, \
	compare_FE_basis_type)
DECLARE_INDEXED_LIST_IDENTIFIER_CHANGE_FUNCTIONS(FE_basis,type)

PROTOTYPE_MANAGER_COPY_WITHOUT_IDENTIFIER_FUNCTION(FE_basis,type)
{
	int return_code = 1;
	if (source && destination)
	{
		// since basis is completely defined by its type,
		// just recreate source and swap contents with destination
		FE_basis *tmpBasis;
		ALLOCATE(tmpBasis, FE_basis, 1);
		FE_basis *copyBasis = CREATE(FE_basis)(source->type);
		if (tmpBasis && copyBasis)
		{
			*tmpBasis = *destination;
			*destination = *copyBasis;
			*copyBasis = *tmpBasis;
		}
		else
		{
			display_message(ERROR_MESSAGE, "MANAGER_COPY_WITHOUT_IDENTIFIER(FE_basis,type).  Could not allocate temporaries");
			return_code = 0;
		}
		DESTROY(FE_basis)(&copyBasis);
		DEALLOCATE(tmpBasis);
	}
	else
	{
		display_message(ERROR_MESSAGE, "MANAGER_COPY_WITHOUT_IDENTIFIER(FE_basis,type).  Invalid argument(s)");
		return_code = 0;
	}
	return return_code;
}

PROTOTYPE_MANAGER_COPY_IDENTIFIER_FUNCTION(FE_basis,type,int *)
{
	int *basis_type,*destination_type,i,number_of_xi_coordinates,return_code,size;

	ENTER(MANAGER_COPY_IDENTIFIER(FE_basis,type));
	if (destination&&(basis_type=type)&&((number_of_xi_coordinates= *type)>0))
	{
		size=1+(number_of_xi_coordinates*(number_of_xi_coordinates+1))/2;
		if (ALLOCATE(destination_type,int,size))
		{
			DEALLOCATE(destination->type);
			destination->type=destination_type;
			for (i=size;i>0;i--)
			{
				*destination_type= *basis_type;
				basis_type++;
				destination_type++;
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
"MANAGER_COPY_IDENTIFIER(FE_basis,type).  Could not allocate memory for type");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_IDENTIFIER(FE_basis,type).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_IDENTIFIER(FE_basis,type) */

PROTOTYPE_MANAGER_COPY_WITH_IDENTIFIER_FUNCTION(FE_basis,type)
{
	int return_code;

	ENTER(MANAGER_COPY_WITH_IDENTIFIER(FE_basis,type));
	if (source&&destination)
	{
		return_code = MANAGER_COPY_WITHOUT_IDENTIFIER(FE_basis,type)(
			destination,source);
		if (return_code)
		{
			return_code=MANAGER_COPY_IDENTIFIER(FE_basis,type)(destination,
				source->type);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_WITH_IDENTIFIER(FE_basis,type).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITH_IDENTIFIER(FE_basis,type) */

DECLARE_MANAGER_FUNCTIONS(FE_basis,manager)

DECLARE_DEFAULT_MANAGED_OBJECT_NOT_IN_USE_FUNCTION(FE_basis,manager)

DECLARE_MANAGER_IDENTIFIER_FUNCTIONS(FE_basis,type,int *,manager)

int *FE_basis_string_to_type_array(const char *basis_description_string)
/*******************************************************************************
LAST MODIFIED : 3 November 2004

DESCRIPTION :
Creates a type array from the <basis_description_string>.  Returns the type
array which can be used in CREATE(FE_basis) and make_FE_basis.
It is up to the calling function to DEALLOCATE the returned array.

Some examples of basis descriptions are:
1. c.Hermite*c.Hermite*l.Lagrange  This has cubic variation in xi1 and xi2 and
	linear variation in xi3.
2. c.Hermite*l.simplex(3)*l.simplex  This has cubic variation in xi1 and 2-D
	linear simplex variation for xi2 and xi3.
3. polygon(5,3)*l.Lagrange*polygon  This has linear variation in xi2 and a 2-D
	5-gon for xi1 and xi3.
==============================================================================*/
{
	const char *index,*start_basis_name;
	int *basis_type,component,i,j,*first_simplex,no_error,
		number_of_polygon_vertices,number_of_xi_coordinates,previous_component,
		*temp_basis_type,*xi_basis_type,xi_number;

	ENTER(FE_basis_string_to_type_array);
	basis_type=(int *)NULL;
	if (basis_description_string)
	{
		number_of_xi_coordinates=1;
		index=basis_description_string;
		while ((index=strchr(index,'*')))
		{
			index++;
			number_of_xi_coordinates++;
		}
		if (ALLOCATE(basis_type,int,
			1+(number_of_xi_coordinates*(1+ number_of_xi_coordinates))/2))
		{
			/* decipher the basis description */
			xi_number=0;
			start_basis_name=basis_description_string;
			/* skip leading blanks */
			while (' '== *start_basis_name)
			{
				start_basis_name++;
			}
			xi_basis_type=basis_type;
			*xi_basis_type=number_of_xi_coordinates;
			xi_basis_type++;
			no_error=1;
			while (no_error&&(xi_number<number_of_xi_coordinates))
			{
				xi_number++;
				/* determine the interpolation in the xi direction */
				if ((0==strncmp(start_basis_name,"l.simplex",9))||
					(0==strncmp(start_basis_name,"q.simplex",9)))
				{
					/*???debug */
					/*printf("simplex\n");*/
					if (0==strncmp(start_basis_name,"l.simplex",9))
					{
						*xi_basis_type=LINEAR_SIMPLEX;
					}
					else
					{
						*xi_basis_type=QUADRATIC_SIMPLEX;
					}
					/*???debug */
					/*printf("%p %d %d\n",xi_basis_type,*xi_basis_type,LINEAR_SIMPLEX);*/
					start_basis_name += 9;
					/* skip blanks */
					while (' '== *start_basis_name)
					{
						start_basis_name++;
					}
					/* check for links to other simplex components */
					if ('('== *start_basis_name)
					{
						/*???debug */
						/*printf("first simplex component\n");*/
						xi_basis_type++;
						/* assign links to other simplex components */
						previous_component=xi_number+1;
						if ((1==sscanf(start_basis_name,"(%d %n",&component,&i))&&
							(previous_component<=component)&&
							(component<=number_of_xi_coordinates))
						{
							do
							{
								start_basis_name += i;
								while (previous_component<component)
								{
									*xi_basis_type=NO_RELATION;
									xi_basis_type++;
									previous_component++;
								}
								*xi_basis_type=1;
								xi_basis_type++;
								previous_component++;
							} while ((')'!=start_basis_name[0])&&
								(1==sscanf(start_basis_name,"%*[; ]%d %n",&component,&i))&&
								(previous_component<=component)&&
								(component<=number_of_xi_coordinates));
							if (')'==start_basis_name[0])
							{
								/* fill rest of basis_type row with NO_RELATION */
								while (previous_component <= number_of_xi_coordinates)
								{
									*xi_basis_type=NO_RELATION;
									xi_basis_type++;
									previous_component++;
								}
								start_basis_name ++;
							}
							else
							{
								/* have no links to succeeding xi directions */
								display_message(ERROR_MESSAGE,
									"Invalid simplex component of basis");
								no_error=0;
							}
						}
						else
						{
							/* have no links to succeeding xi directions */
							display_message(ERROR_MESSAGE,
								"Invalid simplex component of basis");
							no_error=0;
						}
					}
					else
					{
						/*???debug */
						/*printf("not first simplex component\n");*/
						/* check that links have been assigned */
						temp_basis_type=xi_basis_type;
						i=xi_number-1;
						j=number_of_xi_coordinates-xi_number;
						first_simplex=(int *)NULL;
						while (no_error&&(i>0))
						{
							j++;
							temp_basis_type -= j;
							if (NO_RELATION!= *temp_basis_type)
							{
								/*???debug */
								/*printf("%p %p\n",xi_basis_type,(temp_basis_type-(xi_number-i)));
								  printf("%d %d\n",*xi_basis_type,*(temp_basis_type-(xi_number-i)));*/
								if (*xi_basis_type== *(temp_basis_type-(xi_number-i)))
								{
									first_simplex=temp_basis_type;
								}
								else
								{
									no_error=0;
								}
							}
							i--;
						}
						/*???debug */
						/*printf("%d %p\n",no_error,first_simplex);*/
						if (no_error&&first_simplex)
						{
							xi_basis_type++;
							first_simplex++;
							i=xi_number;
							while (i<number_of_xi_coordinates)
							{
								*xi_basis_type= *first_simplex;
								xi_basis_type++;
								first_simplex++;
								i++;
							}
						}
						else
						{
							no_error=0;
						}
					}
				}
				else
				{
					if (0==strncmp(start_basis_name,"polygon",7))
					{
						*xi_basis_type=POLYGON;
						start_basis_name += 7;
						/* skip blanks */
						while (' '== *start_basis_name)
						{
							start_basis_name++;
						}
						/* check for link to other polygon component */
						if ('('== *start_basis_name)
						{
							/* assign link to other polygon component */
							if ((2==sscanf(start_basis_name,"(%d ;%d )%n",
									  &number_of_polygon_vertices,&component,&i))&&
								(3<=number_of_polygon_vertices)&&
								(xi_number<component)&&
								(component<=number_of_xi_coordinates)&&
								('*'== start_basis_name[i]))
							{
								start_basis_name += i;
								/* assign link */
								xi_basis_type++;
								i=xi_number+1;
								while (i<component)
								{
									*xi_basis_type=NO_RELATION;
									xi_basis_type++;
									i++;
								}
								*xi_basis_type=number_of_polygon_vertices;
								xi_basis_type++;
								while (i<number_of_xi_coordinates)
								{
									*xi_basis_type=NO_RELATION;
									xi_basis_type++;
									i++;
								}
							}
							else
							{
								/* have no links to succeeding xi directions */
								display_message(ERROR_MESSAGE,
									"Invalid polygon component of basis");
								no_error=0;
							}
						}
						else
						{
							/* check that link has been assigned */
							temp_basis_type=xi_basis_type;
							i=xi_number-1;
							j=number_of_xi_coordinates-xi_number;
							number_of_polygon_vertices=0;
							while (no_error&&(i>0))
							{
								j++;
								temp_basis_type -= j;
								if (NO_RELATION!= *temp_basis_type)
								{
									if (0<number_of_polygon_vertices)
									{
										no_error=0;
									}
									else
									{
										if ((number_of_polygon_vertices= *temp_basis_type)<3)
										{
											no_error=0;
										}
									}
								}
								i--;
							}
							if (no_error&&(3<=number_of_polygon_vertices))
							{
								xi_basis_type++;
								i=xi_number;
								while (i<number_of_xi_coordinates)
								{
									*xi_basis_type=NO_RELATION;
									xi_basis_type++;
									i++;
								}
							}
							else
							{
								no_error=0;
							}
						}
					}
					else
					{
						if (0==strncmp(start_basis_name,"l.Lagrange",10))
						{
							*xi_basis_type=LINEAR_LAGRANGE;
							start_basis_name += 10;
						}
						else if (0==strncmp(start_basis_name,"q.Lagrange",10))
						{
							*xi_basis_type=QUADRATIC_LAGRANGE;
							start_basis_name += 10;
						}
						else if (0==strncmp(start_basis_name,"c.Lagrange",10))
						{
							*xi_basis_type=CUBIC_LAGRANGE;
							start_basis_name += 10;
						}
						else if (0==strncmp(start_basis_name,"c.Hermite",9))
						{
							*xi_basis_type=CUBIC_HERMITE;
							start_basis_name += 9;
						}
						else if (0==strncmp(start_basis_name,"constant",8))
						{
							*xi_basis_type=FE_BASIS_CONSTANT;
							start_basis_name += 8;
						}
						else if (0==strncmp(start_basis_name,"LagrangeHermite",15))
						{
							*xi_basis_type=LAGRANGE_HERMITE;
							start_basis_name += 15;
						}
						else if (0==strncmp(start_basis_name,"HermiteLagrange",15))
						{
							*xi_basis_type=HERMITE_LAGRANGE;
							start_basis_name += 15;
						}
						else
						{
							display_message(ERROR_MESSAGE, "Invalid basis type");
							no_error=0;
						}
						if (no_error)
						{
							/* skip blanks */
							while (' '== *start_basis_name)
							{
								start_basis_name++;
							}
							/* check for simplex elements */
							if ('('== *start_basis_name)
							{
								/* assign links to succeeding simplex xi directions */
								temp_basis_type=xi_basis_type;
								i=xi_number;
								while (no_error&&(i<number_of_xi_coordinates)&&
									(')'!= *start_basis_name))
								{
									temp_basis_type++;
									if (';'== *start_basis_name)
									{
										*temp_basis_type=NO_RELATION;
										start_basis_name++;
									}
									else
									{
										if (0==strncmp(start_basis_name,"l.Lagrange",10))
										{
											*xi_basis_type=LINEAR_LAGRANGE;
											start_basis_name += 10;
										}
										else if (0==strncmp(start_basis_name,"q.Lagrange",10))
										{
											*xi_basis_type=QUADRATIC_LAGRANGE;
											start_basis_name += 10;
										}
										else if (0==strncmp(start_basis_name,"c.Lagrange",10))
										{
											*xi_basis_type=CUBIC_LAGRANGE;
											start_basis_name += 10;
										}
										else if (0==strncmp(start_basis_name,"c.Hermite",9))
										{
											*xi_basis_type=CUBIC_HERMITE;
											start_basis_name += 9;
										}
										else if (0==strncmp(start_basis_name,"constant",8))
										{
											*xi_basis_type=FE_BASIS_CONSTANT;
											start_basis_name += 8;
										}
										else if (0==strncmp(start_basis_name,"LagrangeHermite",15))
										{
											*xi_basis_type=LAGRANGE_HERMITE;
											start_basis_name += 15;
										}
										else if (0==strncmp(start_basis_name,"HermiteLagrange",15))
										{
											*xi_basis_type=HERMITE_LAGRANGE;
											start_basis_name += 15;
										}
										else
										{
											display_message(ERROR_MESSAGE, "Invalid basis type");
											no_error=0;
										}
										if (';'== *start_basis_name)
										{
											start_basis_name++;
										}
									}
									i++;
								}
								if (no_error)
								{
									while (i<number_of_xi_coordinates)
									{
										temp_basis_type++;
										*temp_basis_type=NO_RELATION;
										i++;
									}
								}
							}
							else
							{
								temp_basis_type=xi_basis_type;
								for (i=xi_number;i<number_of_xi_coordinates;i++)
								{
									temp_basis_type++;
									*temp_basis_type=NO_RELATION;
								}
							}
							if (no_error&&(xi_number<number_of_xi_coordinates))
							{
								xi_basis_type += number_of_xi_coordinates-xi_number+1;
							}
						}
					}
				}
				if ('*' == *start_basis_name)
				{
					start_basis_name++;
				}
				else
				{
					if ('\0' != *start_basis_name)
					{
						display_message(ERROR_MESSAGE,"Invalid basis type");
						no_error=0;
					}
				}
			}
			if (!no_error)
			{
				display_message(ERROR_MESSAGE,"Invalid basis description");
				DEALLOCATE(basis_type);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE, "FE_basis_string_to_type_array.  "
				"Unable to allocate basis type array.");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_basis_string_to_type_array.  Invalid argument)");
	}
	LEAVE;

	return (basis_type);
} /* FE_basis_string_to_type_array */

/*******************************************************************************
 * Returns the string description of the basis type used in serialisation.
 * Currently limited to handling one polygon or one simplex. Will have to
 * be rewritten for 4-D and above elements.
 *
 * @param type_array  FE_basis type array - see struct FE_basis
 * @return  allocated basis description string
 */
static char *FE_basis_type_array_to_string(const int *type_array)
{
	char *basis_string, temp[20];
	const char *basis_type_string;
	const int *type, *relation_type;
	enum FE_basis_type basis_type;
	int dimension, error, i, linked_dimensions, number_of_polygon_vertices,
		xi_number;

	ENTER(FE_basis_type_array_to_string);
	basis_string = (char *)NULL;
	if (type_array && (0 < (dimension = *type_array)) &&
		(dimension <= MAXIMUM_ELEMENT_XI_DIMENSIONS))
	{
		type = type_array + 1;
		linked_dimensions = 0;
		error = 0;
		for (xi_number = 0; (xi_number < dimension) && !error; xi_number++)
		{
			basis_type = (enum FE_basis_type)(*type);
			basis_type_string = FE_basis_type_string(basis_type);
			if (NULL != basis_type_string)
			{
				append_string(&basis_string, basis_type_string, &error);
				switch (basis_type)
				{
					case CUBIC_HERMITE:
					case CUBIC_LAGRANGE:
					case FE_BASIS_CONSTANT:
					case HERMITE_LAGRANGE:
					case LAGRANGE_HERMITE:
					case LINEAR_LAGRANGE:
					case QUADRATIC_LAGRANGE:
					case BSPLINE:
					case FOURIER:
						/* not sure about the following two: */
					case SINGULAR:
					case TRANSITION:
					{
						/* no linking between dimensions */
					} break;
					case LINEAR_SIMPLEX:
					case QUADRATIC_SIMPLEX:
					case SERENDIPITY:
					{
						/* write (linked_xi[;linked_xi]) */
						/* logic currently limited to one simplex in shape - ok to 3D */
						if (0 == linked_dimensions)
						{
							linked_dimensions++;
							/* for first linked simplex dimension write (N1[;N2]) where N1
								 is first linked dimension, N2 is second - for tetrahedra */
							append_string(&basis_string, "(", &error);
							relation_type = type + 1;
							for (i = xi_number + 2; i <= dimension; i++)
							{
								if (*relation_type)
								{
									linked_dimensions++;
									if (linked_dimensions > 2)
									{
										append_string(&basis_string, ";", &error);
									}
									sprintf(temp, "%d", i);
									append_string(&basis_string, temp, &error);
								}
								relation_type++;
							}
							append_string(&basis_string, ")", &error);
							if (1 == linked_dimensions)
							{
								display_message(ERROR_MESSAGE,
									"FE_basis_type_array_to_string.  Too few linked dimensions in simplex");
								DEALLOCATE(basis_string);
								error = 1;
							}
						}
					} break;
					case POLYGON:
					{
						/* write (number_of_polygon_vertices;linked_xi) */
						/* logic currently limited to one polygon in basis - ok to 3D */
						if (0 == linked_dimensions)
						{
							linked_dimensions++;
							number_of_polygon_vertices = 0;
							relation_type = type + 1;
							for (i = xi_number + 2; i <= dimension; i++)
							{
								if (*relation_type)
								{
									linked_dimensions++;
									if (0 == number_of_polygon_vertices)
									{
										number_of_polygon_vertices = *relation_type;
										if (number_of_polygon_vertices >= 3)
										{
											sprintf(temp, "(%d;%d)", number_of_polygon_vertices, i);
											append_string(&basis_string, temp, &error);
										}
										else
										{
											display_message(ERROR_MESSAGE, "write_FE_basis.  "
												"Invalid number of vertices in polygon: %d",
												number_of_polygon_vertices);
											DEALLOCATE(basis_string);
											error = 1;
										}
									}
								}
								relation_type++;
							}
							if (2 != linked_dimensions)
							{
								display_message(ERROR_MESSAGE, "FE_basis_type_array_to_string.  "
									"Invalid number of linked dimensions in polygon: %d", linked_dimensions);
								DEALLOCATE(basis_string);
								error = 1;
							}
						}
					} break;
					default:
					{
						display_message(ERROR_MESSAGE,
							"write_FE_basis.  Unknown basis type: %s", basis_type_string);
						error = 1;
					}
				}
				type += (dimension - xi_number);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"FE_basis_type_array_to_string.  Invalid basis type");
				error = 1;
			}
			if (xi_number < (dimension - 1))
			{
				append_string(&basis_string, "*", &error);
			}
		}
	}
	LEAVE;

	return (basis_string);
}

char *FE_basis_get_description_string(struct FE_basis *basis)
{
	char *basis_string;

	ENTER(FE_basis_get_description_string);
	basis_string = (char *)NULL;
	if (basis && (basis->type))
	{
		basis_string = FE_basis_type_array_to_string(basis->type);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_basis_get_description_string.  Invalid basis");
	}
	LEAVE;

	return (basis_string);
}

const char *FE_basis_type_string(enum FE_basis_type basis_type)
/*******************************************************************************
LAST MODIFIED : 1 April 1999

DESCRIPTION :
Returns a pointer to a static string token for the given <basis_type>.
The calling function must not deallocate the returned string.
???RC Not complete
#### Must ensure implemented correctly for new FE_basis_types ####
==============================================================================*/
{
	const char *basis_type_string;

	ENTER(FE_basis_type_string);
	switch (basis_type)
	{
		case NO_RELATION:
		{
			basis_type_string="no_relation";
		} break;
		case BSPLINE:
		{
			basis_type_string="???bspline";
		} break;
		case CUBIC_HERMITE:
		{
			basis_type_string="c.Hermite";
		} break;
		case CUBIC_LAGRANGE:
		{
			basis_type_string="c.Lagrange";
		} break;
		case FE_BASIS_CONSTANT:
		{
			basis_type_string="constant";
		} break;
		case FOURIER:
		{
			basis_type_string="???fourier";
		} break;
		case HERMITE_LAGRANGE:
		{
			basis_type_string="HermiteLagrange";
		} break;
		case LAGRANGE_HERMITE:
		{
			basis_type_string="LagrangeHermite";
		} break;
		case LINEAR_LAGRANGE:
		{
			basis_type_string="l.Lagrange";
		} break;
		case LINEAR_SIMPLEX:
		{
			basis_type_string="l.simplex";
		} break;
		case POLYGON:
		{
			basis_type_string="polygon";
		} break;
		case QUADRATIC_LAGRANGE:
		{
			basis_type_string="q.Lagrange";
		} break;
		case QUADRATIC_SIMPLEX:
		{
			basis_type_string="q.simplex";
		} break;
		case SERENDIPITY:
		{
			basis_type_string="???serendipity";
		} break;
		case SINGULAR:
		{
			basis_type_string="???singular";
		} break;
		case TRANSITION:
		{
			basis_type_string="???transition";
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"FE_basis_type_string.  Invalid basis_type");
			basis_type_string=(const char *)NULL;
		} break;
	}
	LEAVE;

	return (basis_type_string);
} /* FE_basis_type_string */

int FE_basis_get_dimension(struct FE_basis *basis,
	int *dimension_address)
/*******************************************************************************
LAST MODIFIED : 6 November 2002

DESCRIPTION :
Returns the dimension of <basis>.
If fails, puts zero at <dimension_address>.
==============================================================================*/
{
	int return_code;

	ENTER(FE_basis_get_dimension);
	if (basis && basis->type && dimension_address)
	{
		*dimension_address = basis->type[0];
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_basis_get_dimension.  Invalid argument(s)");
		if (dimension_address)
		{
			*dimension_address = 0;
		}
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_basis_get_dimension */

int FE_basis_get_number_of_basis_functions(struct FE_basis *basis)
{
	if (basis)
		return basis->number_of_basis_functions;
	return 0;
}

int FE_basis_get_function_number_from_node_function(struct FE_basis *basis,
	int nodeNumber, int nodeFunctionIndex)
{
	if (basis && (0 <= nodeNumber) && (nodeNumber < FE_basis_get_number_of_nodes(basis)) &&
		(0 <= nodeFunctionIndex) && (nodeFunctionIndex < FE_basis_get_number_of_functions_per_node(basis, nodeNumber)))
	{
		int functionNumber = nodeFunctionIndex;
		for (int n = 0; n < nodeNumber; ++n)
			functionNumber += FE_basis_get_number_of_functions_per_node(basis, n);
		return functionNumber;
	}
	display_message(ERROR_MESSAGE, "FE_basis_get_function_number_from_node_function.  Invalid argument(s)");
	return -1;
}

int FE_basis_get_basis_node_function_number_limit(struct FE_basis *basis,
	int functionNumber)
{
	if (basis && (0 <= functionNumber) && (functionNumber < basis->number_of_basis_functions))
	{
		int functionNumberLimit = 0;
		const int nodeCount = FE_basis_get_number_of_nodes(basis);
		for (int n = 0; n < nodeCount; ++n)
		{
			functionNumberLimit += FE_basis_get_number_of_functions_per_node(basis, n);
			if (functionNumberLimit > functionNumber)
				return functionNumberLimit;
		}
	}
	display_message(ERROR_MESSAGE, "FE_basis_get_basis_node_function_number_limit.  Invalid argument(s)");
	return 0;
}

int FE_basis_get_number_of_nodes(struct FE_basis *basis)
{
	if (basis)
		return basis->parameterNodes[basis->number_of_basis_functions - 1] + 1;
	return 0;
}

int FE_basis_get_number_of_functions_per_node(struct FE_basis *basis, int nodeNumber)
{
	if ((!basis) || (nodeNumber < 0))
		return 0;
	const int functionCount = basis->number_of_basis_functions;
	for (int f = 0; f < functionCount; ++f)
	{
		if (basis->parameterNodes[f] == nodeNumber)
		{
			int numberOfFunctions = 1;
			for (++f; (f < functionCount) && (basis->parameterNodes[f] == nodeNumber); ++f)
				++numberOfFunctions;
			return numberOfFunctions;
		}
	}
	return 0;
}

FE_basis_type cmzn_elementbasis_function_type_to_FE_basis_type(
	cmzn_elementbasis_function_type functionType)
{
	FE_basis_type feBasisType = FE_BASIS_TYPE_INVALID;
	switch (functionType)
	{
	case CMZN_ELEMENTBASIS_FUNCTION_TYPE_CONSTANT:
		feBasisType = FE_BASIS_CONSTANT;
		break;
	case CMZN_ELEMENTBASIS_FUNCTION_TYPE_LINEAR_LAGRANGE:
		feBasisType = LINEAR_LAGRANGE;
		break;
	case CMZN_ELEMENTBASIS_FUNCTION_TYPE_QUADRATIC_LAGRANGE:
		feBasisType = QUADRATIC_LAGRANGE;
		break;
	case CMZN_ELEMENTBASIS_FUNCTION_TYPE_CUBIC_LAGRANGE:
		feBasisType = CUBIC_LAGRANGE;
		break;
	case CMZN_ELEMENTBASIS_FUNCTION_TYPE_LINEAR_SIMPLEX:
		feBasisType = LINEAR_SIMPLEX;
		break;
	case CMZN_ELEMENTBASIS_FUNCTION_TYPE_QUADRATIC_SIMPLEX:
		feBasisType = QUADRATIC_SIMPLEX;
		break;
	case CMZN_ELEMENTBASIS_FUNCTION_TYPE_CUBIC_HERMITE:
		feBasisType = CUBIC_HERMITE;
		break;
	case CMZN_ELEMENTBASIS_FUNCTION_TYPE_INVALID:
		break;
	}
	return feBasisType;
}

cmzn_elementbasis_function_type FE_basis_type_to_cmzn_elementbasis_function_type(
	FE_basis_type feBasisType)
{
	cmzn_elementbasis_function_type functionType = CMZN_ELEMENTBASIS_FUNCTION_TYPE_INVALID;
	switch (feBasisType)
	{
	case FE_BASIS_CONSTANT:
		functionType = CMZN_ELEMENTBASIS_FUNCTION_TYPE_CONSTANT;
		break;
	case LINEAR_LAGRANGE:
		functionType = CMZN_ELEMENTBASIS_FUNCTION_TYPE_LINEAR_LAGRANGE;
		break;
	case QUADRATIC_LAGRANGE:
		functionType = CMZN_ELEMENTBASIS_FUNCTION_TYPE_QUADRATIC_LAGRANGE;
		break;
	case CUBIC_LAGRANGE:
		functionType = CMZN_ELEMENTBASIS_FUNCTION_TYPE_CUBIC_LAGRANGE;
		break;
	case LINEAR_SIMPLEX:
		functionType = CMZN_ELEMENTBASIS_FUNCTION_TYPE_LINEAR_SIMPLEX;
		break;
	case QUADRATIC_SIMPLEX:
		functionType = CMZN_ELEMENTBASIS_FUNCTION_TYPE_QUADRATIC_SIMPLEX;
		break;
	case CUBIC_HERMITE:
		functionType = CMZN_ELEMENTBASIS_FUNCTION_TYPE_CUBIC_HERMITE;
		break;
	case FE_BASIS_TYPE_INVALID:
	case NO_RELATION:
	case BSPLINE:
	case FOURIER:
	case HERMITE_LAGRANGE:
	case LAGRANGE_HERMITE:
	case POLYGON:
	case SERENDIPITY:
	case SINGULAR:
	case TRANSITION:
		break;
	}
	return functionType;
}

int FE_basis_get_xi_basis_type(struct FE_basis *basis,
	int xi_number, enum FE_basis_type *basis_type_address)
{
	int i, offset, return_code;

	ENTER(FE_basis_get_xi_basis_type);
	if (basis && basis->type && (0 <= xi_number) &&
		(xi_number < basis->type[0]) && basis_type_address)
	{
		/* first value in basis->type is the dimension */
		offset = 1;
		for (i = 0; i < xi_number; i++)
		{
			offset += *(basis->type) - i;
		}
		*basis_type_address = (enum FE_basis_type)basis->type[offset];
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_basis_get_xi_basis_type.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

cmzn_elementbasis_function_type FE_basis_get_xi_elementbasis_function_type(
	FE_basis *feBasis, int xiNumber)
{
	FE_basis_type feBasisType;
	if (FE_basis_get_xi_basis_type(feBasis, xiNumber, &feBasisType))
		return FE_basis_type_to_cmzn_elementbasis_function_type(feBasisType);
	return CMZN_ELEMENTBASIS_FUNCTION_TYPE_INVALID;
}

int FE_basis_get_next_linked_xi_number(
	struct FE_basis *basis, int xi_number,
	int *next_xi_number_address, int *xi_link_number_address)
/*******************************************************************************
LAST MODIFIED : 6 November 2002

DESCRIPTION :
Returns in <next_xi_number_address> the next xi number higher than <xi_number>
which is linked in basis with it, plus in <xi_link_number_address> the number
denoting how it is linked; currently used only for polygon basiss to denote the
number of polygon sides.
If there is no remaining linked dimension, 0 is returned in both addresses.
<xi_number> is from 0 to one less than the basis dimension.
Also checks that the linked xi numbers have the same basis type.
==============================================================================*/
{
	enum FE_basis_type basis_type;
	int i, limit, offset, return_code;

	ENTER(FE_basis_get_next_linked_xi_number);
	if (basis && basis->type &&
		(0 <= xi_number) && (xi_number < *(basis->type)) &&
		next_xi_number_address && xi_link_number_address)
	{
		return_code = 1;
		offset = 1; /* The first element is the dimension */
		for (i = 0; i < xi_number; i++)
		{
			offset += *(basis->type) - i;
		}
		basis_type = (enum FE_basis_type)basis->type[offset];
		limit = *(basis->type) - xi_number;
		offset++;
		for (i = 1; (i < limit) && (0 == basis->type[offset]); i++)
		{
			offset++;
		}
		if (i < limit)
		{
			*next_xi_number_address = i + xi_number;
			*xi_link_number_address = basis->type[offset];
			/* finally check the basis type matches */
			offset = 1; /* The first element is the dimension */
			for (i = 0; i < *next_xi_number_address; i++)
			{
				offset += *(basis->type) - i;
			}
			if (basis->type[offset] != basis_type)
			{
				display_message(ERROR_MESSAGE,
					"FE_basis_get_next_linked_xi_number.  "
					"Basis has linked xi directions with different basis type");
				return_code = 0;
			}
		}
		else
		{
			*next_xi_number_address = 0;
			*xi_link_number_address = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_basis_get_next_linked_xi_number.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* FE_basis_get_next_linked_xi_number */

int standard_basis_function_is_monomial(Standard_basis_function *function,
	void *arguments_void)
/*******************************************************************************
LAST MODIFIED : 12 June 2002

DESCRIPTION :
Returns true if the standard basis function is a monomial.
==============================================================================*/
{
	int *arguments,return_code;

	ENTER(standard_basis_function_is_monomial);
	return_code=0;
	arguments=(int *)arguments_void;
	if ((monomial_basis_functions==function) && (NULL != arguments) &&
		(1 <= arguments[0]))
	{
		return_code=1;
	}
	LEAVE;

	return (return_code);
} /* standard_basis_function_is_monomial */

inline int FE_basis_type_to_number_of_nodes(enum FE_basis_type basis_type)
{
	switch (basis_type)
	{
	case LINEAR_LAGRANGE:
	case CUBIC_HERMITE:
	case LAGRANGE_HERMITE:
	case HERMITE_LAGRANGE:
		return 2;
		break;
	case QUADRATIC_LAGRANGE:
		return 3;
		break;
	case CUBIC_LAGRANGE:
		return 4;
		break;
	default:
		break;
	}
	return 0;
}

bool FE_basis_modify_theta_in_xi1(struct FE_basis *basis,
	enum FE_basis_modify_theta_mode mode, FE_value *values)
{
	if ((!basis) || (!values))
	{
		display_message(ERROR_MESSAGE, "FE_basis_modify_theta_in_xi1.  Invalid argument(s)");
		return false;
	}
	const int basis_dimension = *basis->type;
	if (!((1 == basis_dimension)
		|| ((2 == basis_dimension) && (NO_RELATION == basis->type[2]))
		|| ((3 == basis_dimension) && (NO_RELATION == basis->type[2])
			&& (NO_RELATION == basis->type[3]) && (NO_RELATION == basis->type[5]))))
	{
		display_message(ERROR_MESSAGE, "FE_basis_modify_theta_in_xi1.  Non tensor product bases are not supported");
		return false;
	}

	const int number_of_nodes_in_xi1 = FE_basis_type_to_number_of_nodes(
		static_cast<FE_basis_type>(basis->type[1]));
	const int number_of_nodes_in_xi2 = (basis_dimension < 2) ? 1 : FE_basis_type_to_number_of_nodes(
		static_cast<FE_basis_type>((2 == basis_dimension) ? basis->type[3] : basis->type[4]));
	const int number_of_nodes_in_xi3 = (basis_dimension < 3) ? 1 : FE_basis_type_to_number_of_nodes(
		static_cast<FE_basis_type>(basis->type[6]));

	// check whether one of the xi1 * +/-{xi2|xi3} planes has all zero values,
	// indicating it is on the apex, and to copy theta values from next row of
	// nodes to interpolate correctly.
	// This is a hack; versions or general maps should give the proper value.

	bool onAxisXi2_0 = (1 < number_of_nodes_in_xi2) ? true : false;
	bool onAxisXi2_1 = onAxisXi2_0;
	bool onAxisXi3_0 = (1 < number_of_nodes_in_xi3) ? true : false;
	bool onAxisXi3_1 = onAxisXi3_0;
	int valueIndex = 0;
	int *nodeValueIndexes = new int[number_of_nodes_in_xi1*number_of_nodes_in_xi2*number_of_nodes_in_xi3];
	for (int k = 0; k < number_of_nodes_in_xi3; ++k)
		for (int j = 0; j < number_of_nodes_in_xi2; ++j)
			for (int i = 0; i < number_of_nodes_in_xi1; ++i)
			{
				nodeValueIndexes[(k*number_of_nodes_in_xi2 + j)*number_of_nodes_in_xi1 + i] = valueIndex;
				const bool zeroValue = (values[valueIndex] == 0.0);
				if (j == 0)
					onAxisXi2_0 = onAxisXi2_0 && zeroValue;
				else if (j == (number_of_nodes_in_xi2 - 1))
					onAxisXi2_1 = onAxisXi2_1 && zeroValue;
				if (k == 0)
					onAxisXi3_0 = onAxisXi3_0 && zeroValue;
				else if (k == (number_of_nodes_in_xi3 - 1))
					onAxisXi3_1 = onAxisXi3_1 && zeroValue;
				++valueIndex;
				// increment until we have the next non-derivative value
				while ((valueIndex < basis->number_of_basis_functions) && (basis->parameterDerivatives[valueIndex]))
					++valueIndex;
			}

	if (onAxisXi3_0) // most common case in our models, so handle first
	{
		for (int j = 0; j < number_of_nodes_in_xi2; ++j)
			for (int i = 0; i < number_of_nodes_in_xi1; ++i)
				values[nodeValueIndexes[j*number_of_nodes_in_xi1 + i]] =
					values[nodeValueIndexes[(number_of_nodes_in_xi2 + j)*number_of_nodes_in_xi1 + i]];
	}
	else if (onAxisXi3_1)
	{
		for (int j = 0; j < number_of_nodes_in_xi2; ++j)
			for (int i = 0; i < number_of_nodes_in_xi1; ++i)
				values[nodeValueIndexes[((number_of_nodes_in_xi3 - 1)*number_of_nodes_in_xi2 + j)*number_of_nodes_in_xi1 + i]] =
					values[nodeValueIndexes[((number_of_nodes_in_xi3 - 2)*number_of_nodes_in_xi2 + j)*number_of_nodes_in_xi1 + i]];
	}
	else if (onAxisXi2_0)
	{
		for (int k = 0; k < number_of_nodes_in_xi3; ++k)
			for (int i = 0; i < number_of_nodes_in_xi1; ++i)
				values[nodeValueIndexes[k*number_of_nodes_in_xi2*number_of_nodes_in_xi1 + i]] =
					values[nodeValueIndexes[(k*number_of_nodes_in_xi2 + 1)*number_of_nodes_in_xi1 + i]];
	}
	else if (onAxisXi2_1)
	{
		for (int k = 0; k < number_of_nodes_in_xi3; ++k)
			for (int i = 0; i < number_of_nodes_in_xi1; ++i)
				values[nodeValueIndexes[(k*number_of_nodes_in_xi2 + number_of_nodes_in_xi2 - 1)*number_of_nodes_in_xi1 + i]] =
					values[nodeValueIndexes[(k*number_of_nodes_in_xi2 + number_of_nodes_in_xi2 - 2)*number_of_nodes_in_xi1 + i]];
	}

	// apply modify functions for consecutive nodes increasing in xi1 (around theta)
	const double TWO_PI = 2.0*PI;
	FE_value *thetaValue = values;
	int localNode = 0;
	FE_value offsetThetaXi2 = 0.0;
	FE_value offsetThetaXi3 = 0.0;
	for (int k = number_of_nodes_in_xi3 - 1; 0 <= k; --k)
	{
		FE_value lastThetaXi3 = *thetaValue;
		for (int j = number_of_nodes_in_xi2 - 1; 0 <= j; --j)
		{
			FE_value lastThetaXi2 = *thetaValue;
			for (int i = number_of_nodes_in_xi1 - 1; 0 < i; --i)
			{
				FE_value lastThetaXi1 = *thetaValue;
				thetaValue += FE_basis_get_number_of_functions_per_node(basis, localNode);
				++localNode;
				*thetaValue += (offsetThetaXi2 + offsetThetaXi3);
				switch (mode)
				{
				case FE_BASIS_MODIFY_THETA_MODE_CLOSEST_IN_XI1:
					if (lastThetaXi1 < (*thetaValue - PI))
						*thetaValue -= TWO_PI;
					else if (lastThetaXi1 >(*thetaValue + PI))
						*thetaValue += TWO_PI;
					break;
				case FE_BASIS_MODIFY_THETA_MODE_DECREASING_IN_XI1:
					if (lastThetaXi1 <= *thetaValue)
						*thetaValue -= TWO_PI;
					break;
				case FE_BASIS_MODIFY_THETA_MODE_INCREASING_IN_XI1:
					if (lastThetaXi1 >= *thetaValue)
						*thetaValue += TWO_PI;
					break;
				case FE_BASIS_MODIFY_THETA_MODE_NON_DECREASING_IN_XI1:
					if (lastThetaXi1 > *thetaValue)
						*thetaValue += TWO_PI;
					break;
				case FE_BASIS_MODIFY_THETA_MODE_NON_INCREASING_IN_XI1:
					if (lastThetaXi1 < *thetaValue)
						*thetaValue -= TWO_PI;
					break;
				case FE_BASIS_MODIFY_THETA_MODE_INVALID:
					break;
				}
			}
			thetaValue += FE_basis_get_number_of_functions_per_node(basis, localNode);
			++localNode;
			if (j != 0)
			{
				if (*thetaValue > (lastThetaXi2 + PI))
				{
					offsetThetaXi2 = -TWO_PI;
					*thetaValue += offsetThetaXi2;
				}
				else
				{
					if (*thetaValue < (lastThetaXi2 - PI))
					{
						offsetThetaXi2 = TWO_PI;
						*thetaValue += offsetThetaXi2;
					}
					else
					{
						offsetThetaXi2 = 0.0;
					}
				}
			}
		}
		if (k != 0)
		{
			offsetThetaXi2 = 0.0;
			if (*thetaValue > (lastThetaXi3 + PI))
			{
				offsetThetaXi3 = -TWO_PI;
				*thetaValue += offsetThetaXi3;
			}
			else
			{
				if (*thetaValue < (lastThetaXi3 - PI))
				{
					offsetThetaXi3 = TWO_PI;
					*thetaValue += offsetThetaXi3;
				}
				else
				{
					offsetThetaXi3 = 0.0;
				}
			}
		}
	}

	delete nodeValueIndexes;
	return true;
}

