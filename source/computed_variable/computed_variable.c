/*******************************************************************************
FILE : computed_variable.c

LAST MODIFIED : 19 May 2003

DESCRIPTION :
Cmiss_variable's are expressions that are constructed for:
- display eg. difference between measured and calculated positions
- minimization eg. fitting by minimizing the difference between measured and
	calculated positions
- solution eg. solving a FEM variational formulation equals zero

A Cmiss_variable is a variable which is used in or is the result of a
calculation so can include element/xi, time, nodal parameters and element scale
factors.

When differentiating Cmiss_variable's the dependent and independent variables
are specified.  Any unspecified variables are constant.

Cmiss_variable's are able to be:
- evaluated at a point (specific choice of values for independent variables)
- differentiated at a point (specific choice of values for independent
	variables ie. not symbolic)
- composed ie. the results of one Cmiss_variable can replace independent
	variables for another Cmiss_variable

The shared interface to Cmiss_variable's is defined here, but there are
separate modules for specific types eg finite element, gradient and coordinate
transformations.

NOTES :
1 To allow compilation, caching and reuse, there should be a
	Cmiss_variable_calculation object to be used in the following manner
		calculation=CREATE(Cmiss_variable_calculation)(variable)
		j=1,p
			Cmiss_variable_calculation_set_variable_value(calculation,variable_ij
				value_ij),i=1,nj
			Cmiss_variable_calculation_set_derivative(calculation,derivative_ij),
				i=1,mj
			Cmiss_variable_calculation_evaluate(calculation)
		DESTROY(Cmiss_variable_calculation)(&calculation)
	The Cmiss_variable_calculation_set's are not changing the variable, just
	changing the overrides.  Assuming that the variable is not being changed,
	this allows calculation shortcuts to be made.  The
	Cmiss_variable_calculation object allows the cache to be outside of the
	Cmiss_variable.
2 To specify a derivative, a list of Cmiss_variable's is used eg
	d^3/dxi1dxi3^2 is represented by the list (xi1,xi3,xi3).
3 Have split off computed_variable_utilities and computed_variable_commands in
	order to focus on core of Cmiss_variable's.
4 To say that the results of one computed field should replace independent
	variables for a second computed field (composition of mathematical functions),
	use Cmiss_variable_set_source.
5 Specifying variable values in evaluate/is_defined is only for independent
	variables.  If a source has been specified for a variable, it is no longer
	independent.
6 Cmiss_variables are not invertable ie. if the results are specified for an
	evaluation, the dofs are not calculated.  Instead
	- Newton-Raphson can be done explicitly in the calling program or command file
	- use Cmiss_variable_inverse_set_type
7 A Cmiss_variable can be generic eg a mesh location or a time for any finite
	element computed variable.
8 To get a string representation of a variable use Cmiss_variable_get_value
	with a value of type string.

???DB.  Something needs doing about FE_value - has spread too far?  Currently
	set up in general/value.h .  Maybe just needs renaming?  Do as part of moving
	Cmiss_value to value?
???DB.  Do without components?
???DB.  Need Cmiss_variable_changed?
==============================================================================*/
#include "computed_variable/computed_value_matrix.h"
#include "computed_variable/computed_value_private.h"
#include "computed_variable/computed_variable_private.h"
#include "general/indexed_list_private.h"
#include "general/debug.h"
#include "general/list_private.h"
#include "general/manager_private.h"
#include "general/mystring.h"
#include "user_interface/message.h"

/*
Module constants
----------------
*/

/*
Module methods
--------------
*/
static char Cmiss_value_derivative_matrix_type_string[]="Derivative_matrix";

#if defined (OLD_CODE)
/*???DB.  Changing from fe_value_matrix to matrix and to storing the blocks
	within fe_value_matrix as separate matrices */
struct Cmiss_value_derivative_matrix_type_specific_data
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
The number of rows is the number of values for the <dependent_variable>.  The
number of columns is:
	product(1+number_of_values,for each independent_variable)-1
The columns are:
	d(dependent_variable)/d(independent_variables[0])
	d(dependent_variable)/d(independent_variables[1])
	d2(dependent_variable)/d(independent_variables[0])d(independent_variables[1])
	d(dependent_variable)/d(independent_variables[2])
	d2(dependent_variable)/d(independent_variables[0])d(independent_variables[2])
	d2(dependent_variable)/d(independent_variables[1])d(independent_variables[2])
	d3(dependent_variable)/d(independent_variables[0])d(independent_variables[1])
		d(independent_variables[2])
	...
	d(dependent_variable)/d(independent_variables[order-1])
	d2(dependent_variable)/d(independent_variables[0])
		d(independent_variables[order-1])
	d2(dependent_variable)/d(independent_variables[1])
		d(independent_variables[order-1])
	d3(dependent_variable)/d(independent_variables[0])d(independent_variables[1])
		d(independent_variables[order-1])
	d2(dependent_variable)/d(independent_variables[2])
		d(independent_variables[order-1])
	d3(dependent_variable)/d(independent_variables[0])d(independent_variables[2])
		d(independent_variables[order-1])
	d3(dependent_variable)/d(independent_variables[1])d(independent_variables[2])
		d(independent_variables[order-1])
	d4(dependent_variable)/d(independent_variables[0])d(independent_variables[1])
		d(independent_variables[2])d(independent_variables[order-1])
	...
	d{order}(dependent_variable)/d(independent_variables[0])
		d(independent_variables[1]) ... d(independent_variables[order-1])
==============================================================================*/
{
	int order;
	Cmiss_value_id fe_value_matrix;
	Cmiss_variable_id dependent_variable,*independent_variables;
}; /* struct Cmiss_value_derivative_matrix_type_specific_data */
#endif /* defined (OLD_CODE) */

struct Cmiss_value_derivative_matrix_type_specific_data
/*******************************************************************************
LAST MODIFIED : 5 May 2003

DESCRIPTION :
If not NULL, <matrices> has 2**<order>-1 entries which are:
	d(dependent_variable)/d(independent_variables[0])
	d(dependent_variable)/d(independent_variables[1])
	d2(dependent_variable)/d(independent_variables[0])d(independent_variables[1])
	d(dependent_variable)/d(independent_variables[2])
	d2(dependent_variable)/d(independent_variables[0])d(independent_variables[2])
	d2(dependent_variable)/d(independent_variables[1])d(independent_variables[2])
	d3(dependent_variable)/d(independent_variables[0])d(independent_variables[1])
		d(independent_variables[2])
	...
	d(dependent_variable)/d(independent_variables[order-1])
	d2(dependent_variable)/d(independent_variables[0])
		d(independent_variables[order-1])
	d2(dependent_variable)/d(independent_variables[1])
		d(independent_variables[order-1])
	d3(dependent_variable)/d(independent_variables[0])d(independent_variables[1])
		d(independent_variables[order-1])
	d2(dependent_variable)/d(independent_variables[2])
		d(independent_variables[order-1])
	d3(dependent_variable)/d(independent_variables[0])d(independent_variables[2])
		d(independent_variables[order-1])
	d3(dependent_variable)/d(independent_variables[1])d(independent_variables[2])
		d(independent_variables[order-1])
	d4(dependent_variable)/d(independent_variables[0])d(independent_variables[1])
		d(independent_variables[2])d(independent_variables[order-1])
	...
	d{order}(dependent_variable)/d(independent_variables[0])
		d(independent_variables[1]) ... d(independent_variables[order-1])

The number of rows for each entry is the number of values for the
<dependent_variable>.  The number of columns for each entry is
	product(1+number_of_values, for each <independent_variable> involved in the
		entry)
==============================================================================*/
{
	int order;
	Cmiss_value_id *matrices;
	Cmiss_variable_id dependent_variable,*independent_variables;
}; /* struct Cmiss_value_derivative_matrix_type_specific_data */

#if defined (OLD_CODE)
/*???DB.  Changing from fe_value_matrix to matrix and to storing the blocks
	within fe_value_matrix as separate matrices */
static START_CMISS_VALUE_CLEAR_TYPE_SPECIFIC_FUNCTION(derivative_matrix)
{
	int i;

	DEACCESS(Cmiss_value)(&(data->fe_value_matrix));
	DEACCESS(Cmiss_variable)(&(data->dependent_variable));
	for (i=0;i<data->order;i++)
	{
		DEACCESS(Cmiss_variable)(&((data->independent_variables)[i]));
	}
	DEALLOCATE(data->independent_variables);
	return_code=1;
}
END_CMISS_VALUE_CLEAR_TYPE_SPECIFIC_FUNCTION(derivative_matrix)
#endif /* defined (OLD_CODE) */

static START_CMISS_VALUE_CLEAR_TYPE_SPECIFIC_FUNCTION(derivative_matrix)
{
	Cmiss_value_id *matrix;
	int i,number_of_matrices;

	DEACCESS(Cmiss_variable)(&(data->dependent_variable));
	number_of_matrices=1;
	for (i=0;i<data->order;i++)
	{
		DEACCESS(Cmiss_variable)(&((data->independent_variables)[i]));
		number_of_matrices *= 2;
	}
	number_of_matrices -= 1;
	DEALLOCATE(data->independent_variables);
	if (matrix=data->matrices)
	{
		for (i=number_of_matrices;i>0;i--)
		{
			DEACCESS(Cmiss_value)(matrix);
			matrix++;
		}
		DEALLOCATE(data->matrices);
	}
	return_code=1;
}
END_CMISS_VALUE_CLEAR_TYPE_SPECIFIC_FUNCTION(derivative_matrix)

#if defined (OLD_CODE)
/*???DB.  Changing from fe_value_matrix to matrix and to storing the blocks
	within fe_value_matrix as separate matrices */
static START_CMISS_VALUE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION(
	derivative_matrix)
{
	int i;

	if (destination->fe_value_matrix=CREATE(Cmiss_value)())
	{
		ACCESS(Cmiss_value)(destination->fe_value_matrix);
		if (Cmiss_value_copy(destination->fe_value_matrix,
			source->fe_value_matrix))
		{
			destination->dependent_variable=
				ACCESS(Cmiss_variable)(source->dependent_variable);
			if (0<source->order)
			{
				if (ALLOCATE(destination->independent_variables,
					Cmiss_variable_id ,source->order))
				{
					destination->order=source->order;
					for (i=0;i<source->order;i++)
					{
						(destination->independent_variables)[i]=ACCESS(Cmiss_variable)(
							(source->independent_variables)[i]);
					}
				}
				else
				{
					DEACCESS(Cmiss_variable)(&(source->dependent_variable));
					DEACCESS(Cmiss_value)(&(destination->fe_value_matrix));
					DEALLOCATE(destination);
				}
			}
			else
			{
				destination->order=0;
				destination->independent_variables=(Cmiss_variable_id *)NULL;
			}
		}
		else
		{
			DEACCESS(Cmiss_value)(&(destination->fe_value_matrix));
			DEALLOCATE(destination);
		}
	}
	else
	{
		DEALLOCATE(destination);
	}
}
END_CMISS_VALUE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION(derivative_matrix)
#endif /* defined (OLD_CODE) */

static START_CMISS_VALUE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION(
	derivative_matrix)
{
	Cmiss_value_id *destination_matrix,*source_matrix;
	int i,number_of_matrices;

	destination->dependent_variable=
		ACCESS(Cmiss_variable)(source->dependent_variable);
	if (0<source->order)
	{
		if (ALLOCATE(destination->independent_variables,
			Cmiss_variable_id ,source->order))
		{
			destination->order=source->order;
			number_of_matrices=1;
			for (i=0;i<source->order;i++)
			{
				(destination->independent_variables)[i]=ACCESS(Cmiss_variable)(
					(source->independent_variables)[i]);
				number_of_matrices *= 2;
			}
			number_of_matrices -= 1;
			if (source_matrix=source->matrices)
			{
				if (ALLOCATE(destination_matrix,Cmiss_value_id,number_of_matrices))
				{
					destination->matrices=destination_matrix;
					i=number_of_matrices;
					while (destination&&(i>0))
					{
						if ((*destination_matrix=CREATE(Cmiss_value)())&&
							Cmiss_value_copy(*destination_matrix,*source_matrix))
						{
							ACCESS(Cmiss_value)(*destination_matrix);
							source_matrix++;
							destination_matrix++;
							i--;
						}
						else
						{
							while (i<number_of_matrices)
							{
								destination_matrix++;
								DEACCESS(Cmiss_value)(destination_matrix);
								i++;
							}
							DEALLOCATE(destination->matrices);
							for (i=0;i<source->order;i++)
							{
								DEACCESS(Cmiss_variable)(
									(destination->independent_variables)+i);
							}
							DEALLOCATE(destination->independent_variables);
							DEACCESS(Cmiss_variable)(&(source->dependent_variable));
							DEALLOCATE(destination);
						}
					}
				}
				else
				{
					for (i=0;i<source->order;i++)
					{
						DEACCESS(Cmiss_variable)((destination->independent_variables)+i);
					}
					DEALLOCATE(destination->independent_variables);
					DEACCESS(Cmiss_variable)(&(source->dependent_variable));
					DEALLOCATE(destination);
				}
			}
		}
		else
		{
			DEACCESS(Cmiss_variable)(&(source->dependent_variable));
			DEALLOCATE(destination);
		}
	}
	else
	{
		destination->order=0;
		destination->independent_variables=(Cmiss_variable_id *)NULL;
		destination->matrices=(Cmiss_value_id *)NULL;
	}
}
END_CMISS_VALUE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION(derivative_matrix)

#if defined (OLD_CODE)
/*???DB.  Changing from fe_value_matrix to matrix and to storing the blocks
	within fe_value_matrix as separate matrices */
static START_CMISS_VALUE_MULTIPLY_AND_ACCUMULATE_TYPE_SPECIFIC_FUNCTION(
	derivative_matrix)
/*******************************************************************************
LAST MODIFIED : 20 March 2003

DESCRIPTION :
This function implements the chain rule for differentiation.

If
	Y=G(X), H(X)=F(G(X))
then
	dH/dXi=
		sum{p=1,m:dF/dYp*dGp/dXi}
	d2H/dXidXj=
		sum{p=1,m:sum{q=1,m:d2F/dYpdYq*dGp/dXi*dGq/dXj}}+
		sum{p=1,m:dF/dYp*d2Gp/dXidXj}
	d3H/dXidXjdXk=
		sum{p=1,m:sum{q=1,m:sum{r=1,m:d3F/dYpdYqdYr*dGp/dXi*dGq/dXj*dGr/dXk}}}+
		sum{p=1,m:sum{q=1,m:d2F/dYpdYq*
			[d2Gp/dXidXj*dGq/dXk+d2Gp/dXidXk*dGq/dXj+dGp/dXi*d2Gq/dXjdXk]}}+
		sum{p=1,m:dF/dYp*d3Gp/dXidXjdXk}
	d4H/dXidXjdXkdXl=
		sum{p=1,m:sum{q=1,m:sum{r=1,m:sum{s=1,m:
			d4F/dYpdYqdYrdYs*dGp/dXi*dGq/dXj*dGr/dXk*dGr/dXl}}}}+
		sum{p=1,m:sum{q=1,m:sum{r=1,m:d3F/dYpdYqdYr*
			[d2Gp/dXidXj*dGq/dXk*dGr/dXl+
			d2Gp/dXidXk*dGq/dXj*dGr/dXl+
			d2Gp/dXidXl*dGq/dXj*dGr/dXk+
			dGp/dXi*d2Gq/dXjdXk*dGr/dXl+
			dGp/dXi*d2Gq/dXjdXl*dGr/dXk+
			dGp/dXi*dGq/dXj*d2Gr/dXkdXl]}}}+
		sum{p=1,m:sum{q=1,m:d2F/dYpdYq*
			[d3Gp/dXidXjdXk*dGq/dXl+
			d3Gp/dXidXjdXl*dGq/dXk+
			d3Gp/dXidXkdXl*dGq/dXj+
			dGp/dXi*d3Gq/dXjdXkdXl+
			d2Gp/dXidXj*d2Gq/dXkdXl+
			d2Gp/dXidXk*d2Gq/dXjdXl+
			d2Gp/dXidXl*d2Gq/dXjdXk]}}+
		sum{p=1,m:dF/dYp*d4Gp/dXidXjdXkdXl}
	...
where m=length(Y).

There are some parts above which don't look symmetric eg the p's and q's in
	[d2Gp/dXidXj*dGq/dXk+d2Gp/dXidXk*dGq/dXj+dGp/dXi*d2Gq/dXjdXk]
However for "reasonable" functions the order differentiation does not change the
result.  Assuming the functions are "reasonable" gives
	d2F/dYpdYq=d2F/dYqdYp
and so
	[d2Gp/dXidXj*dGq/dXk+d2Gp/dXidXk*dGq/dXj+dGp/dXi*d2Gq/dXjdXk]
can be replaced by
	[d2Gp/dXidXj*dGq/dXk+d2Gp/dXidXk*dGq/dXj+d2Gp/dXjdXk*dGq/dXi]
even though they are not equal because
	[d2Gp/dXidXj*dGq/dXk+d2Gp/dXidXk*dGq/dXj+d2Gp/dXjdXk*dGq/dXi]+
	[d2Gq/dXidXj*dGp/dXk+d2Gq/dXidXk*dGp/dXj+d2Gq/dXjdXk*dGp/dXi]
	=
	[d2Gp/dXidXj*dGq/dXk+d2Gp/dXidXk*dGq/dXj+dGp/dXi*d2Gq/dXjdXk]+
	[d2Gq/dXidXj*dGp/dXk+d2Gq/dXidXk*dGp/dXj+dGq/dXi*d2Gp/dXjdXk]

To calculate a derivative of H, the derivatives of all orders of F and G are
needed and so derivatives of all orders are stored in the derivative matrices
in the following order
	d/dX,d/dY,d2/dXdY,d/dZ,d2/dXdZ,d2/dYdZ,d3/dXdYdZ
NB.  This assumes that order of differentiation does not change the result

In this function
	total corresponds to H
	value_1 corresponds to F
	value_2 corresponds to G
==============================================================================*/
{
	FE_value *fe_value_total,**fe_values_2,*fe_value_1,*fe_value_2,*matrix_total,
		*matrix_1,*matrix_2,product,sum;
	int dependent_value,found,i,*index_total,*index_1,*index_2,j,k,l,
		*mapping_total,*mapping_2,*not_used,number_of_columns_total,
		number_of_columns_1,number_of_columns_2,number_of_dependent_values,
		*numbers_of_independent_values,number_of_intermediate_values,
		number_of_values_total,number_of_values_1,number_of_values_2,*offset_1,
		*offset_2,order,order_total,*order_2,p,*product_orders,q,r,s,*sub_order_2;
	Cmiss_value_id value_type;
	struct Cmiss_value_derivative_matrix_type_specific_data *data_total,
		*data_1,*data_2;

	data_1=(struct Cmiss_value_derivative_matrix_type_specific_data *)
		Cmiss_value_get_type_specific_data(value_1);
	data_2=(struct Cmiss_value_derivative_matrix_type_specific_data *)
		Cmiss_value_get_type_specific_data(value_2);
	data_total=(struct Cmiss_value_derivative_matrix_type_specific_data *)
		Cmiss_value_get_type_specific_data(total);
	ASSERT_IF(data_1&&data_2&&data_total,return_code,0)
	{
		return_code=0;
		order=data_1->order;
		if (0<order)
		{
			/* check that are doing composition) */
			ALLOCATE(numbers_of_independent_values,int,order);
			ALLOCATE(index_total,int,order+1);
			ALLOCATE(index_1,int,order+1);
			ALLOCATE(index_2,int,order+1);
			ALLOCATE(offset_1,int,order+1);
			ALLOCATE(offset_2,int,order+1);
			ALLOCATE(mapping_total,int,order+1);
			ALLOCATE(mapping_2,int,order+1);
			ALLOCATE(order_2,int,order+1);
			ALLOCATE(sub_order_2,int,order+1);
			ALLOCATE(not_used,int,order+1);
			ALLOCATE(product_orders,int,order+1);
			ALLOCATE(fe_values_2,FE_value *,order);
			value_type=CREATE(Cmiss_value)();
			if (Cmiss_variable_same_variable(data_1->dependent_variable,
				data_total->dependent_variable)&&(order==data_2->order)&&
				(order==data_total->order)&&numbers_of_independent_values&&index_total&&
				index_1&&index_2&&offset_1&&offset_2&&mapping_total&&mapping_2&&
				order_2&&sub_order_2&&not_used&&product_orders&&fe_values_2&&value_type)
			{
				return_code=1;
				i=0;
				while (return_code&&(i<order))
				{
					if ((return_code=Cmiss_variable_same_variable(
						data_2->dependent_variable,(data_1->independent_variables)[i]))&&
						(return_code=Cmiss_variable_same_variable(
						(data_total->independent_variables)[i],
						(data_2->independent_variables)[i]))&&
						(return_code=Cmiss_variable_get_value_type(
						(data_total->independent_variables)[i],value_type)))
					{
						return_code=Cmiss_value_FE_value_vector_get_type(
							value_type,numbers_of_independent_values+i,(FE_value **)NULL);
					}
					i++;
				}
			}
			if (return_code)
			{
				/* extract arrays */
				if (Cmiss_value_FE_value_matrix_get_type(data_total->fe_value_matrix,
					&number_of_dependent_values,&number_of_columns_total,&matrix_total)&&
					Cmiss_value_FE_value_matrix_get_type(data_1->fe_value_matrix,
					(int *)NULL,&number_of_columns_1,&matrix_1)&&
					Cmiss_value_FE_value_matrix_get_type(data_2->fe_value_matrix,
					&number_of_intermediate_values,&number_of_columns_2,&matrix_2))
				{
					/* calculate offsets for first partial with respect to
						independent_variables[i] in matrix_1 and matrix_2 */
					offset_1[0]=0;
					offset_2[0]=0;
					for (i=1;i<=order;i++)
					{
						offset_1[i]=offset_1[i-1]*(number_of_intermediate_values+1)+
							number_of_intermediate_values;
						offset_2[i]=offset_2[i-1]*(numbers_of_independent_values[i-1]+1)+
							numbers_of_independent_values[i-1];
					}
					/* loop over dependent values (rows) */
					fe_value_total=matrix_total;
					for (dependent_value=number_of_dependent_values;dependent_value>0;
						dependent_value--)
					{
						/* loop over derivatives (columns) */
						/* index_total stores which derivative of total (H) is being
							calculated */
						for (i=0;i<=order;i++)
						{
							index_total[i]=0;
						}
						index_total[0]=1;
						while (0==index_total[order])
						{
							/* determine the derivative being evaluated and calculate its
								order and number of values */
							order_total=0;
							number_of_values_total=1;
							for (i=0;i<order;i++)
							{
								if (index_total[i])
								{
									index_2[order_total]=0;
									mapping_total[order_total]=i;
									order_total++;
									number_of_values_total *= numbers_of_independent_values[i];
								}
							}
							/* calculate the values for the derivative */
							for (i=number_of_values_total;i>0;i--)
							{
								/* loop over the sums for different order derivatives of
									value_1 */
								sum=0;
								number_of_values_1=number_of_intermediate_values;
								for (j=0;j<order_total;j++)
								{
									/* initialize the orders for the derivatives of value_2 that
										are multiplied together.  There are j+1 derivatives of
										value_2 and their orders have to sum to the order of the
										derivative of total (order_total) */
									for (l=0;l<j;l++)
									{
										product_orders[l]=1;
									}
									product_orders[j]=order_total-j;
									/* loop over the possible ways of dividing the order_total
										independent variables, in mapping_total, into j+1 non-empty
										sets, where the order of the sets and the order within the
										sets are not important.  For each possible way, loop across
										the row of matrix_1 and down the columns of matrix_2 and
										- calculate the product of the derivatives of value_2
											represented by the sets
										- multiply the product by the corresponding derivative of
											value_1
										- add the result to total */
									do
									{
										/* initialize the variable assigment */
										/* for each of the independent variables being
											differentiated with respect to in the product have
											mapping_2 - a reordering of the variables without changing
												the orders of the partial derivatives.  It is a location
												in order_2 and sub_order_2
											order_2 - the order of the partial derivative they're in
											sub_order_2 - their position in the variables in partial
												derivatives of the same order */
										r=0;
										l=0;
										while (l<=j)
										{
											q=0;
											do
											{
												for (p=0;p<product_orders[l];p++)
												{
													mapping_2[r]=r;
													order_2[r]=product_orders[l];
													sub_order_2[r]=q;
													r++;
													q++;
												}
												l++;
											} while ((l<=j)&&
												(product_orders[l]==product_orders[l-1]));
										}
										/* find the row and starting column within matrix_1 */
										fe_value_1=matrix_1+(offset_1[j+1]-number_of_values_1);
										/* find the column numbers of matrix 2 for the partial
											derivatives in the product */
										/* r is the number of the partial derivative within partial
											derivatives of the same order */
										r=0;
										for (l=0;l<=j;l++)
										{
											/* initialize the value position within the partial
												derivative of value_1 */
											index_1[l]=0;
											/* determine which independent variables are used in
												the partial derivative of value_2 */
											for (p=0;p<order_total;p++)
											{
												q=mapping_2[p];
												/* is the same partial derivative within the
													partial derivatives of the same order */
												if ((product_orders[j]==order_2[q])&&
													(r==sub_order_2[q]/order_2[q]))
												{
													not_used[p]=0;
												}
												else
												{
													not_used[p]=1;
												}
											}
											/* calculate the column index in matrix_2 */
											fe_value_2=matrix_2;
											/* first the start of the values for the partial
												derivative */
											number_of_values_2=1;
											q=0;
											for (p=order_total-1;p>=0;p--)
											{
												if (!not_used[p])
												{
													/*???DB.  Check/prove? */
													fe_value_2 += q+offset_2[p]*number_of_values_2;
													number_of_values_2 *=
														numbers_of_independent_values[p];
													q=number_of_values_2;
												}
											}
											/* second the index of the value */
											s=0;
											for (p=0;p<order_total;p++)
											{
												if (!not_used[p])
												{
													s=numbers_of_independent_values[mapping_total[p]]*
														s+index_2[p];
												}
											}
											fe_values_2[l]=fe_value_2+s;
											/* increment r (the number of the partial derivative
												within partial derivatives of the same order */
											if ((l<j)&&(product_orders[l]==product_orders[l+1]))
											{
												r++;
											}
											else
											{
												r=0;
											}
										}
										/* loop across the row of matrix_1 and down the columns of
											matrix_2 */
										for (k=number_of_values_1;k>0;k--)
										{
											/* calculate the product */
											product= *fe_value_1;
											for (l=0;l<=j;l++)
											{
												product *=
													*(fe_values_2[l]+index_1[l]*number_of_columns_2);
											}
											/* add to sum */
											sum += product;
											/* increment to next value for derivative in matrix_1
												and matrix_2 */
											fe_value_1++;
											l=j;
											index_1[l]++;
											while ((l>0)&&
												(index_1[l]>=number_of_intermediate_values))
											{
												index_1[l]=0;
												l--;
												index_1[l]++;
											}
										}
										/* move to the next choice for the j+1 sets */
										/* first try leaving the number of variables in each set the
											same (don't change product_orders).  Do this by running
											through the permutations of order_total things with
											restrictions
											- start with the permutation 0,1,2, ... order_total-1
											- for the current permutation
												- start at the end and run back looking for an entry
													which is less than one of the entries further on and
													for which the restrictions hold
												- find the smallest entry that is further on than the
													current entry, greater than the current entry and
													satisfies the restrictions
												- put the smallest in the current and add the remaining
													entries in increasing order */
										l=order_total-1;
										q=mapping_2[l];
										found=0;
										while ((l>0)&&!found)
										{
											p=q;
											l--;
											q=mapping_2[l];
											/* check if there is a value further on with a greater
												index (unrestricted permutations) */
											if (q<p)
											{
												/* apply restrictions */
												/* if have same order */
												if (order_2[p]==order_2[q])
												{
													/* check that p and q are not in the same partial
														derivative and that second set doesn't have values
														less than the first value of q's set */
													/* the order of sets of the same size being
														unimportant is equivalent to having the first
														value of the first set always less than all the
														values of the second set */
													if ((sub_order_2[p]/order_2[p]!=
														sub_order_2[q]/order_2[p])&&
														(0!=sub_order_2[q]%order_2[p]))
													{
														found=1;
													}
												}
												else
												{
													/* check that q hasn't been tried in a partial
														derivative of order_2[p] */
													if (order_2[q]<order_2[p])
													{
														found=1;
													}
												}
											}
										}
										if (found)
										{
											/* mark as unused the values after l */
											for (p=0;p<order_total;p++)
											{
												not_used[p]=0;
											}
											for (p=l;p<order_total;p++)
											{
												not_used[mapping_2[p]]=1;
											}
											q=mapping_2[l];
											p=q;
											found=0;
											/* find the smallest valid value after l which is
												greater than mapping_2[l] */
											do
											{
												p++;
												if (not_used[p])
												{
													if (order_2[p]==order_2[q])
													{
														if ((sub_order_2[p]/order_2[p]!=
															sub_order_2[q]/order_2[p])&&
															(0!=sub_order_2[q]%order_2[p]))
														{
															found=1;
														}
													}
													else
													{
														if (order_2[p]>order_2[q])
														{
															found=1;
														}
													}
												}
											} while (!found);
											/* put the smallest value in l */
											mapping_2[l]=p;
											not_used[p]=0;
											/* put the unused values in increasing order after l */
											for (p=0;p<order_total;p++)
											{
												if (not_used[p])
												{
													l++;
													mapping_2[l]=p;
												}
											}
										}
										else
										{
											/* look for another choice of the j+1 set sizes.  Having
												the order of the sets being unimportant is equivalent to
												having the sizes in non-decreasing order and starting
												with sizes 1,1,...order_total-j */
											l=j;
											while ((l>0)&&
												(product_orders[l]==product_orders[l-1]))
											{
												l--;
											}
											if (l>0)
											{
												(product_orders[l])--;
												while ((l>0)&&
													(product_orders[l]==product_orders[l-1]))
												{
													l--;
												}
												if (l>0)
												{
													/* have found a new choice of set sizes
														re-initialize the variable assignment */
													(product_orders[l-1])++;
												}
											}
										}
									} while (l>0);
									number_of_values_1 *= number_of_intermediate_values;
								}
								*fe_value_total += sum;
								fe_value_total++;
								/* increment to next value for derivative in matrix_2 */
								j=order_total-1;
								index_2[j]++;
								k=mapping_total[j];
								while ((j>0)&&(index_2[j]>=numbers_of_independent_values[k]))
								{
									index_2[j]=0;
									j--;
									k=mapping_total[j];
									index_2[j]++;
								}
							}
							/* increment to next derivative */
							i=0;
							while ((i<order)&&(index_total[i]))
							{
								i++;
							}
							for (j=0;j<i;j++)
							{
								index_total[j]=0;
							}
							index_total[i]=1;
						}
						/* move to next row of matrix_1 */
						for (i=0;i<=order;i++)
						{
							offset_1[i] += number_of_columns_1;
						}
					}
				}
				else
				{
					return_code=0;
				}
			}
			DESTROY(Cmiss_value)(&value_type);
			DEALLOCATE(fe_values_2);
			DEALLOCATE(product_orders);
			DEALLOCATE(not_used);
			DEALLOCATE(sub_order_2);
			DEALLOCATE(order_2);
			DEALLOCATE(mapping_2);
			DEALLOCATE(mapping_total);
			DEALLOCATE(offset_2);
			DEALLOCATE(offset_1);
			DEALLOCATE(index_2);
			DEALLOCATE(index_1);
			DEALLOCATE(index_total);
			DEALLOCATE(numbers_of_independent_values);
		}
	}
}
END_CMISS_VALUE_MULTIPLY_AND_ACCUMULATE_TYPE_SPECIFIC_FUNCTION(
	derivative_matrix)
#endif /* defined (OLD_CODE) */

static START_CMISS_VALUE_MULTIPLY_AND_ACCUMULATE_TYPE_SPECIFIC_FUNCTION(
	derivative_matrix)
/*******************************************************************************
LAST MODIFIED : 5 May 2003

DESCRIPTION :
This function implements the chain rule for differentiation.

If
	Y=G(X), H(X)=F(G(X))
then
	dH/dXi=
		sum{p=1,m:dF/dYp*dGp/dXi}
	d2H/dXidXj=
		sum{p=1,m:sum{q=1,m:d2F/dYpdYq*dGp/dXi*dGq/dXj}}+
		sum{p=1,m:dF/dYp*d2Gp/dXidXj}
	d3H/dXidXjdXk=
		sum{p=1,m:sum{q=1,m:sum{r=1,m:d3F/dYpdYqdYr*dGp/dXi*dGq/dXj*dGr/dXk}}}+
		sum{p=1,m:sum{q=1,m:d2F/dYpdYq*
			[d2Gp/dXidXj*dGq/dXk+d2Gp/dXidXk*dGq/dXj+dGp/dXi*d2Gq/dXjdXk]}}+
		sum{p=1,m:dF/dYp*d3Gp/dXidXjdXk}
	d4H/dXidXjdXkdXl=
		sum{p=1,m:sum{q=1,m:sum{r=1,m:sum{s=1,m:
			d4F/dYpdYqdYrdYs*dGp/dXi*dGq/dXj*dGr/dXk*dGr/dXl}}}}+
		sum{p=1,m:sum{q=1,m:sum{r=1,m:d3F/dYpdYqdYr*
			[d2Gp/dXidXj*dGq/dXk*dGr/dXl+
			d2Gp/dXidXk*dGq/dXj*dGr/dXl+
			d2Gp/dXidXl*dGq/dXj*dGr/dXk+
			dGp/dXi*d2Gq/dXjdXk*dGr/dXl+
			dGp/dXi*d2Gq/dXjdXl*dGr/dXk+
			dGp/dXi*dGq/dXj*d2Gr/dXkdXl]}}}+
		sum{p=1,m:sum{q=1,m:d2F/dYpdYq*
			[d3Gp/dXidXjdXk*dGq/dXl+
			d3Gp/dXidXjdXl*dGq/dXk+
			d3Gp/dXidXkdXl*dGq/dXj+
			dGp/dXi*d3Gq/dXjdXkdXl+
			d2Gp/dXidXj*d2Gq/dXkdXl+
			d2Gp/dXidXk*d2Gq/dXjdXl+
			d2Gp/dXidXl*d2Gq/dXjdXk]}}+
		sum{p=1,m:dF/dYp*d4Gp/dXidXjdXkdXl}
	...
where m=length(Y).

There are some parts above which don't look symmetric eg the p's and q's in
	[d2Gp/dXidXj*dGq/dXk+d2Gp/dXidXk*dGq/dXj+dGp/dXi*d2Gq/dXjdXk]
However for "reasonable" functions the order differentiation does not change the
result.  Assuming the functions are "reasonable" gives
	d2F/dYpdYq=d2F/dYqdYp
and so
	[d2Gp/dXidXj*dGq/dXk+d2Gp/dXidXk*dGq/dXj+dGp/dXi*d2Gq/dXjdXk]
can be replaced by
	[d2Gp/dXidXj*dGq/dXk+d2Gp/dXidXk*dGq/dXj+d2Gp/dXjdXk*dGq/dXi]
even though they are not equal because
	[d2Gp/dXidXj*dGq/dXk+d2Gp/dXidXk*dGq/dXj+d2Gp/dXjdXk*dGq/dXi]+
	[d2Gq/dXidXj*dGp/dXk+d2Gq/dXidXk*dGp/dXj+d2Gq/dXjdXk*dGp/dXi]
	=
	[d2Gp/dXidXj*dGq/dXk+d2Gp/dXidXk*dGq/dXj+dGp/dXi*d2Gq/dXjdXk]+
	[d2Gq/dXidXj*dGp/dXk+d2Gq/dXidXk*dGp/dXj+dGq/dXi*d2Gp/dXjdXk]

To calculate a derivative of H, the derivatives of all orders of F and G are
needed and so derivatives of all orders are stored in the derivative matrices
in the following order
	d/dX,d/dY,d2/dXdY,d/dZ,d2/dXdZ,d2/dYdZ,d3/dXdYdZ
NB.  This assumes that order of differentiation does not change the result

In this function
	total corresponds to H
	value_1 corresponds to F
	value_2 corresponds to G
==============================================================================*/
{
	Cmiss_value_id *matrices_2,*matrix_total,*matrix_1,*matrix_2,value_type;
	int *column_numbers_2,column_number_total,found,i,index_total,*index_1,
		*index_2,j,k,l,*mapping_total,*mapping_2,*not_used,number_of_columns_total,
		number_of_columns_1,*numbers_of_independent_values,
		number_of_intermediate_values,number_of_matrices,number_of_rows,offset_1,
		offset_2,order,order_total,*order_2,p,*product_orders,q,r,row_number,
		*sub_order_2;
	Matrix_value product,sum,value;
	struct Cmiss_value_derivative_matrix_type_specific_data *data_total,
		*data_1,*data_2;

	data_1=(struct Cmiss_value_derivative_matrix_type_specific_data *)
		Cmiss_value_get_type_specific_data(value_1);
	data_2=(struct Cmiss_value_derivative_matrix_type_specific_data *)
		Cmiss_value_get_type_specific_data(value_2);
	data_total=(struct Cmiss_value_derivative_matrix_type_specific_data *)
		Cmiss_value_get_type_specific_data(total);
	ASSERT_IF(data_1&&data_2&&data_total,return_code,0)
	{
		return_code=0;
		order=data_1->order;
		if (0<order)
		{
			/* check that are doing composition) */
			ALLOCATE(numbers_of_independent_values,int,order);
			ALLOCATE(index_1,int,order+1);
			ALLOCATE(index_2,int,order+1);
			ALLOCATE(mapping_total,int,order+1);
			ALLOCATE(mapping_2,int,order+1);
			ALLOCATE(order_2,int,order+1);
			ALLOCATE(sub_order_2,int,order+1);
			ALLOCATE(not_used,int,order+1);
			ALLOCATE(product_orders,int,order+1);
			ALLOCATE(column_numbers_2,int,order+1);
			ALLOCATE(matrices_2,Cmiss_value_id,order+1);
			value_type=CREATE(Cmiss_value)();
			if (Cmiss_variable_same_variable(data_1->dependent_variable,
				data_total->dependent_variable)&&(order==data_2->order)&&
				(order==data_total->order)&&numbers_of_independent_values&&
				index_1&&index_2&&mapping_total&&mapping_2&&
				order_2&&sub_order_2&&not_used&&product_orders&&column_numbers_2&&
				matrices_2&&value_type&&
				Cmiss_variable_get_value_type(data_1->dependent_variable,value_type))
			{
				return_code=Cmiss_value_FE_value_vector_get_type(value_type,
					&number_of_rows,(FE_value **)NULL);
				i=0;
				number_of_matrices=1;
				while (return_code&&(i<order))
				{
					if ((return_code=Cmiss_variable_same_variable(
						data_2->dependent_variable,(data_1->independent_variables)[i]))&&
						(return_code=Cmiss_variable_same_variable(
						(data_total->independent_variables)[i],
						(data_2->independent_variables)[i]))&&
						(return_code=Cmiss_variable_get_value_type(
						(data_total->independent_variables)[i],value_type)))
					{
						return_code=Cmiss_value_FE_value_vector_get_type(
							value_type,numbers_of_independent_values+i,(FE_value **)NULL);
					}
					number_of_matrices *= 2;
					i++;
				}
				number_of_matrices -= 1;
				if (return_code&&(return_code=Cmiss_variable_get_value_type(data_2->
					dependent_variable,value_type)))
				{
					return_code=Cmiss_value_FE_value_vector_get_type(value_type,
						&number_of_intermediate_values,(FE_value **)NULL);
				}
			}
			if (return_code)
			{
				/* loop over dependent values (rows) */
				row_number=1;
				while (return_code&&(row_number<=number_of_rows))
				{
					/* loop over derivatives (matrices) */
					matrix_total=data_total->matrices;
					/* index_total stores which derivative (matrix) of total (H) is being
						calculated */
					index_total=1;
					while (return_code&&(index_total<=number_of_matrices))
					{
						/* determine the derivative being evaluated and calculate its
							order and number of values */
						order_total=0;
						number_of_columns_total=1;
						j=index_total;
						i=0;
						while (j>0)
						{
							if (j%2)
							{
								index_2[order_total]=0;
								mapping_total[order_total]=i;
								order_total++;
								number_of_columns_total *= numbers_of_independent_values[i];
							}
							j /= 2;
							i++;
						}
						/* calculate the values for the derivative */
						column_number_total=1;
						while (return_code&&(column_number_total<=number_of_columns_total))
						{
							/* loop over the sums for different order derivatives of
								value_1 */
							sum=(Matrix_value)0;
							matrix_1=data_1->matrices;
							offset_1=1;
							j=0;
							for (j=0;j<order_total;j++)
							{
								/* initialize the orders for the derivatives of value_2 that
									are multiplied together.  There are j+1 derivatives of
									value_2 and their orders have to sum to the order of the
									derivative of total (order_total) */
								for (l=0;l<j;l++)
								{
									product_orders[l]=1;
								}
								product_orders[j]=order_total-j;
								/* loop over the possible ways of dividing the order_total
									independent variables, in mapping_total, into j+1 non-empty
									sets, where the order of the sets and the order within the
									sets are not important.  For each possible way, loop across
									the row of matrix_1 and down the columns of matrix_2 and
									- calculate the product of the derivatives of value_2
										represented by the sets
									- multiply the product by the corresponding derivative of
										value_1
									- add the result to total */
								do
								{
									/* initialize the variable assigment */
									/* for each of the independent variables being
										differentiated with respect to in the product have
										mapping_2 - a reordering of the variables without changing
											the orders of the partial derivatives.  It is a location
											in order_2 and sub_order_2
										order_2 - the order of the partial derivative they're in
										sub_order_2 - their position in the variables in partial
											derivatives of the same order */
									r=0;
									l=0;
									while (l<=j)
									{
										q=0;
										do
										{
											for (p=0;p<product_orders[l];p++)
											{
												mapping_2[r]=r;
												order_2[r]=product_orders[l];
												sub_order_2[r]=q;
												r++;
												q++;
											}
											l++;
										} while ((l<=j)&&
											(product_orders[l]==product_orders[l-1]));
									}
									/* find the column numbers of matrix 2 for the partial
										derivatives in the product */
									/* r is the number of the partial derivative within partial
										derivatives of the same order */
									r=0;
									for (l=0;l<=j;l++)
									{
										/* initialize the value position within the partial
											derivative of value_1 */
										index_1[l]=0;
										/* determine which independent variables are used in
											the partial derivative of value_2 */
										matrix_2=data_2->matrices;
										offset_2=1;
										for (p=0;p<order_total;p++)
										{
											q=mapping_2[p];
											/* is the same partial derivative within the
												partial derivatives of the same order */
											if ((product_orders[j]==order_2[q])&&
												(r==sub_order_2[q]/order_2[q]))
											{
												not_used[p]=0;
											}
											else
											{
												not_used[p]=1;
											}
											matrix_2 += offset_2;
											offset_2 *= 2;
										}
										for (p=order_total-1;p>=0;p--)
										{
											offset_2 /= 2;
											if (not_used[p])
											{
												matrix_2 -= offset_2;
											}
										}
										matrices_2[l]= *matrix_2;
										/* second the index of the value */
										column_numbers_2[l]=0;
										for (p=0;p<order_total;p++)
										{
											if (!not_used[p])
											{
												column_numbers_2[l]=numbers_of_independent_values[
													mapping_total[p]]*column_numbers_2[l]+index_2[p];
											}
										}
										column_numbers_2[l] += 1;
										/* increment r (the number of the partial derivative
											within partial derivatives of the same order */
										if ((l<j)&&(product_orders[l]==product_orders[l+1]))
										{
											r++;
										}
										else
										{
											r=0;
										}
									}
									if (return_code=Cmiss_value_matrix_get_dimensions(*matrix_1,
										(int *)NULL,&number_of_columns_1))
									{
										/* loop across the row of matrix_1 and down the columns of
											matrix_2 */
										k=1;
										while (return_code&&(k<=number_of_columns_1))
										{
											/* calculate the product */
											if (return_code=Cmiss_value_matrix_get_value(*matrix_1,
												row_number,k,&product))
											{
												l=0;
												while (return_code&&(l<=j))
												{
													if (return_code=Cmiss_value_matrix_get_value(
														matrices_2[l],index_1[l],column_numbers_2[l],
														&value))
													{
														product *= value;
													}
													l++;
												}
												if (return_code)
												{
													/* add to sum */
													sum += product;
													/* increment to next value for derivative in matrix_1
														and matrix_2 */
													k++;
													l=j;
													index_1[l]++;
													while ((l>0)&&
														(index_1[l]>=number_of_intermediate_values))
													{
														index_1[l]=0;
														l--;
														index_1[l]++;
													}
												}
											}
										}
									}
									/* move to the next choice for the j+1 sets */
									/* first try leaving the number of variables in each set the
										same (don't change product_orders).  Do this by running
										through the permutations of order_total things with
										restrictions
										- start with the permutation 0,1,2, ... order_total-1
										- for the current permutation
											- start at the end and run back looking for an entry
												which is less than one of the entries further on and
												for which the restrictions hold
											- find the smallest entry that is further on than the
												current entry, greater than the current entry and
												satisfies the restrictions
											- put the smallest in the current and add the remaining
												entries in increasing order */
									l=order_total-1;
									q=mapping_2[l];
									found=0;
									while ((l>0)&&!found)
									{
										p=q;
										l--;
										q=mapping_2[l];
										/* check if there is a value further on with a greater
											index (unrestricted permutations) */
										if (q<p)
										{
											/* apply restrictions */
											/* if have same order */
											if (order_2[p]==order_2[q])
											{
												/* check that p and q are not in the same partial
													derivative and that second set doesn't have values
													less than the first value of q's set */
												/* the order of sets of the same size being
													unimportant is equivalent to having the first
													value of the first set always less than all the
													values of the second set */
												if ((sub_order_2[p]/order_2[p]!=
													sub_order_2[q]/order_2[p])&&
													(0!=sub_order_2[q]%order_2[p]))
												{
													found=1;
												}
											}
											else
											{
												/* check that q hasn't been tried in a partial
													derivative of order_2[p] */
												if (order_2[q]<order_2[p])
												{
													found=1;
												}
											}
										}
									}
									if (found)
									{
										/* mark as unused the values after l */
										for (p=0;p<order_total;p++)
										{
											not_used[p]=0;
										}
										for (p=l;p<order_total;p++)
										{
											not_used[mapping_2[p]]=1;
										}
										q=mapping_2[l];
										p=q;
										found=0;
										/* find the smallest valid value after l which is
											greater than mapping_2[l] */
										do
										{
											p++;
											if (not_used[p])
											{
												if (order_2[p]==order_2[q])
												{
													if ((sub_order_2[p]/order_2[p]!=
														sub_order_2[q]/order_2[p])&&
														(0!=sub_order_2[q]%order_2[p]))
													{
														found=1;
													}
												}
												else
												{
													if (order_2[p]>order_2[q])
													{
														found=1;
													}
												}
											}
										} while (!found);
										/* put the smallest value in l */
										mapping_2[l]=p;
										not_used[p]=0;
										/* put the unused values in increasing order after l */
										for (p=0;p<order_total;p++)
										{
											if (not_used[p])
											{
												l++;
												mapping_2[l]=p;
											}
										}
									}
									else
									{
										/* look for another choice of the j+1 set sizes.  Having
											the order of the sets being unimportant is equivalent to
											having the sizes in non-decreasing order and starting
											with sizes 1,1,...order_total-j */
										l=j;
										while ((l>0)&&
											(product_orders[l]==product_orders[l-1]))
										{
											l--;
										}
										if (l>0)
										{
											(product_orders[l])--;
											while ((l>0)&&
												(product_orders[l]==product_orders[l-1]))
											{
												l--;
											}
											if (l>0)
											{
												/* have found a new choice of set sizes
													re-initialize the variable assignment */
												(product_orders[l-1])++;
											}
										}
									}
								} while (l>0);
								matrix_1 += offset_1;
								offset_1 *= 2;
								j++;
							}
							if (return_code&&(return_code=Cmiss_value_matrix_get_value(
								*matrix_total,row_number,column_number_total,&value)))
							{
								value += sum;
								if (return_code=Cmiss_value_matrix_set_value(*matrix_total,
									row_number,column_number_total,value))
								{
									/* increment to next value for derivative in matrix_2 */
									j=order_total-1;
									index_2[j]++;
									k=mapping_total[j];
									while ((j>0)&&(index_2[j]>=numbers_of_independent_values[k]))
									{
										index_2[j]=0;
										j--;
										k=mapping_total[j];
										index_2[j]++;
									}
									/* increment to next column in derivative (matrix) of total */
									column_number_total++;
								}
							}
						}
						/* increment to next derivative (matrix) */
						index_total++;
						matrix_total++;
					}
					/* increment to next row */
					row_number++;
				}
			}
			DESTROY(Cmiss_value)(&value_type);
			DEALLOCATE(matrices_2);
			DEALLOCATE(column_numbers_2);
			DEALLOCATE(product_orders);
			DEALLOCATE(not_used);
			DEALLOCATE(sub_order_2);
			DEALLOCATE(order_2);
			DEALLOCATE(mapping_2);
			DEALLOCATE(mapping_total);
			DEALLOCATE(index_2);
			DEALLOCATE(index_1);
			DEALLOCATE(numbers_of_independent_values);
		}
	}
}
END_CMISS_VALUE_MULTIPLY_AND_ACCUMULATE_TYPE_SPECIFIC_FUNCTION(
	derivative_matrix)

#if defined (OLD_CODE)
/*???DB.  Changing from fe_value_matrix to matrix and to storing the blocks
	within fe_value_matrix as separate matrices */
static START_CMISS_VALUE_SAME_SUB_TYPE_TYPE_SPECIFIC_FUNCTION(
	derivative_matrix)
/*******************************************************************************
LAST MODIFIED : 20 March 2003

DESCRIPTION :
==============================================================================*/
{
	int i;
	struct Cmiss_value_derivative_matrix_type_specific_data *data_1,*data_2;

	data_1=(struct Cmiss_value_derivative_matrix_type_specific_data *)
		Cmiss_value_get_type_specific_data(value_1);
	data_2=(struct Cmiss_value_derivative_matrix_type_specific_data *)
		Cmiss_value_get_type_specific_data(value_2);
	ASSERT_IF(data_1&&data_2,return_code,0)
	{
		if ((data_1->order==data_2->order)&&Cmiss_value_same_sub_type(
			data_1->fe_value_matrix,data_2->fe_value_matrix)&&
			Cmiss_variable_same_variable(data_1->dependent_variable,
			data_2->dependent_variable)&&(data_1->independent_variables)&&
			(data_2->independent_variables))
		{
			return_code=1;
			i=0;
			while ((i<data_1->order)&&(return_code=Cmiss_variable_same_variable(
				(data_1->independent_variables)[i],(data_2->independent_variables)[i])))
			{
				i++;
			}
		}
	}
}
END_CMISS_VALUE_SAME_SUB_TYPE_TYPE_SPECIFIC_FUNCTION(derivative_matrix)
#endif /* defined (OLD_CODE) */

static START_CMISS_VALUE_SAME_SUB_TYPE_TYPE_SPECIFIC_FUNCTION(
	derivative_matrix)
/*******************************************************************************
LAST MODIFIED : 5 May 2003

DESCRIPTION :
==============================================================================*/
{
	int i,number_of_matrices;
	struct Cmiss_value_derivative_matrix_type_specific_data *data_1,*data_2;

	data_1=(struct Cmiss_value_derivative_matrix_type_specific_data *)
		Cmiss_value_get_type_specific_data(value_1);
	data_2=(struct Cmiss_value_derivative_matrix_type_specific_data *)
		Cmiss_value_get_type_specific_data(value_2);
	ASSERT_IF(data_1&&data_2,return_code,0)
	{
		if ((data_1->order==data_2->order)&&
			(((data_1->matrices)&&(data_2->matrices))||
			(!(data_1->matrices)&&!(data_2->matrices)))&&
			Cmiss_variable_same_variable(data_1->dependent_variable,
			data_2->dependent_variable)&&(data_1->independent_variables)&&
			(data_2->independent_variables))
		{
			return_code=1;
			i=0;
			number_of_matrices=1;
			while ((i<data_1->order)&&(return_code=Cmiss_variable_same_variable(
				(data_1->independent_variables)[i],(data_2->independent_variables)[i])))
			{
				i++;
				number_of_matrices *= 2;
			}
			number_of_matrices -= 1;
			if (return_code&&(data_1->matrices))
			{
				i=0;
				while ((i<number_of_matrices)&&(return_code=Cmiss_value_same_sub_type(
					(data_1->matrices)[i],(data_2->matrices)[i])))
				{
					i++;
				}
			}
		}
	}
}
END_CMISS_VALUE_SAME_SUB_TYPE_TYPE_SPECIFIC_FUNCTION(derivative_matrix)

/*???DB.  Have put here because in general should not know definition of
	struct Cmiss_variable */

static char Cmiss_variable_derivative_type_string[]="Derivative";

struct Cmiss_variable_derivative_type_specific_data
/*******************************************************************************
LAST MODIFIED : 21 March 2003

DESCRIPTION :
==============================================================================*/
{
	/* differentiate */
	Cmiss_variable_id variable;
	/* with respect to */
	int order;
	Cmiss_variable_id *independent_variables;
}; /* struct Cmiss_variable_derivative_type_specific_data */

static START_CMISS_VARIABLE_CLEAR_TYPE_SPECIFIC_FUNCTION(derivative)
{
	int i;

	DEACCESS(Cmiss_variable)(&(data->variable));
	if (data->independent_variables)
	{
		for (i=0;i<data->order;i++)
		{
			DEACCESS(Cmiss_variable)(&((data->independent_variables)[i]));
		}
		DEALLOCATE(data->independent_variables);
	}
	return_code=1;
}
END_CMISS_VARIABLE_CLEAR_TYPE_SPECIFIC_FUNCTION(derivative)

static START_CMISS_VARIABLE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION(derivative)
{
	int i;

	if ((0<source->order)&&ALLOCATE(destination->independent_variables,
		Cmiss_variable_id ,source->order))
	{
		destination->variable=ACCESS(Cmiss_variable)(source->variable);
		destination->order=source->order;
		for (i=0;i<source->order;i++)
		{
			(destination->independent_variables)[i]=
				ACCESS(Cmiss_variable)((source->independent_variables)[i]);
		}
	}
	else
	{
		DEALLOCATE(destination);
	}
}
END_CMISS_VARIABLE_DUPLICATE_DATA_TYPE_SPECIFIC_FUNCTION(derivative)

static START_CMISS_VARIABLE_EVALUATE_DERIVATIVE_TYPE_SPECIFIC_FUNCTION(
	derivative)
{
	int i;
	Cmiss_variable_id *augmented_independent_variables;
	struct Cmiss_variable_derivative_type_specific_data *data;

	data=(struct Cmiss_variable_derivative_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&(data->variable)&&(0<data->order)&&
		(data->independent_variables),return_code,0)
	{
		if (ALLOCATE(augmented_independent_variables,Cmiss_variable_id ,
			order+(data->order)))
		{
			/*???DB.  Should be ACCESSing? */
			for (i=0;i<order;i++)
			{
				augmented_independent_variables[i]=independent_variables[i];
			}
			for (i=0;i<data->order;i++)
			{
				augmented_independent_variables[order+i]=
					(data->independent_variables)[i];
			}
			return_code=Cmiss_variable_evaluate_derivative(data->variable,
				order+(data->order),augmented_independent_variables,
				(struct LIST(Cmiss_variable_value) *)NULL,value);
			DEALLOCATE(augmented_independent_variables);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_variable_derivative_evaluate_derivative_type_specific.  "
				"Could not allocate augmented_independent_variables");
		}
	}
}
END_CMISS_VARIABLE_EVALUATE_DERIVATIVE_TYPE_SPECIFIC_FUNCTION(derivative)

struct Cmiss_variable_add_source_derivative_data
/*******************************************************************************
LAST MODIFIED : 21 March 2003

DESCRIPTION :
==============================================================================*/
{
	int order;
	Cmiss_value_id total_value;
	Cmiss_variable_id *independent_variables;
}; /* struct Cmiss_variable_add_source_derivative_data */

static int Cmiss_variable_add_source_derivative(
	Cmiss_variable_id variable,Cmiss_variable_id source_variable,
	Cmiss_variable_id independent_variable,void *data_void)
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	Cmiss_value_id value_1,value_2;
	struct Cmiss_variable_add_source_derivative_data *data;

	ENTER(Cmiss_variable_add_source_derivative);
	return_code=0;
	/* check arguments */
	if (variable&&source_variable&&independent_variable&&
		(data=(struct Cmiss_variable_add_source_derivative_data *)data_void))
	{
		if (value_1=CREATE(Cmiss_value)())
		{
			if (value_2=CREATE(Cmiss_value)())
			{
				if (Cmiss_variable_evaluate_derivative(variable,1,
					&independent_variable,(struct LIST(Cmiss_variable_value) *)NULL,
					value_1))
				{
					if (Cmiss_variable_evaluate_derivative(source_variable,
						data->order,data->independent_variables,
						(struct LIST(Cmiss_variable_value) *)NULL,value_2))
					{
						return_code=Cmiss_value_multiply_and_accumulate(
							data->total_value,value_1,value_2);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Cmiss_variable_add_source_derivative.  "
							"Could not evaluate derivative of <variable> with respect to "
							"<independent_variable>");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Cmiss_variable_add_source_derivative.  "
						"Could not evaluate derivative of <variable> with respect to "
						"<independent_variable>");
				}
				DESTROY(Cmiss_value)(&value_2);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_variable_add_source_derivative.  "
					"Could not create <value_1>");
			}
			DESTROY(Cmiss_value)(&value_1);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Cmiss_variable_add_source_derivative.  "
				"Could not create <value_1>");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_variable_add_source_derivative.  "
			"Invalid argument(s).  %p %p %p %p",variable,source_variable,
			independent_variable,data_void);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_add_source_derivative */

static START_CMISS_VARIABLE_EVALUATE_TYPE_SPECIFIC_FUNCTION(derivative)
{
	struct Cmiss_variable_add_source_derivative_data source_variable_data;
	struct Cmiss_variable_derivative_type_specific_data *data;

	data=(struct Cmiss_variable_derivative_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&(data->variable)&&(0<data->order)&&
		(data->independent_variables),return_code,0)
	{
		/* derivative=
			d(data->variable)/d(data->independent_variables)+
			sum{d(data->variable)/d(data->variable->independent_variable)*
			d(data->variable->source_variable)/d(data->independent_variables)}
			*/
		if (return_code=Cmiss_variable_evaluate_derivative(data->variable,
			data->order,data->independent_variables,
			(struct LIST(Cmiss_variable_value) *)NULL,value))
		{
			source_variable_data.total_value=value;
			source_variable_data.order=data->order;
			source_variable_data.independent_variables=data->independent_variables;
			return_code=Cmiss_variable_for_each_source_variable(variable,
				Cmiss_variable_add_source_derivative,(void *)&source_variable_data);
		}
	}
}
END_CMISS_VARIABLE_EVALUATE_TYPE_SPECIFIC_FUNCTION(derivative)

static START_CMISS_VARIABLE_GET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(derivative)
{
	struct Cmiss_variable_derivative_type_specific_data *data;

	data=(struct Cmiss_variable_derivative_type_specific_data *)
		Cmiss_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data&&(data->variable),return_code,0)
	{
		return_code=Cmiss_variable_get_independent_variable_value(
			data->variable,independent_variable,value);
	}
}
END_CMISS_VARIABLE_GET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(derivative)

static START_CMISS_VARIABLE_GET_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(derivative)
{
	struct Cmiss_variable_derivative_type_specific_data *data;

	data=(struct Cmiss_variable_derivative_type_specific_data *)
		Cmiss_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data&&(data->variable),return_code,0)
	{
		return_code=Cmiss_variable_get_set_independent_variable_value(
			data->variable,independent_variable,value);
	}
}
END_CMISS_VARIABLE_GET_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(derivative)

#if defined (OLD_CODE)
/*???DB.  Changing from fe_value_matrix to matrix and to storing the blocks
	within fe_value_matrix as separate matrices */
static START_CMISS_VARIABLE_GET_VALUE_TYPE_TYPE_SPECIFIC_FUNCTION(derivative)
{
	int i,number_of_columns,number_of_rows,temp_number_of_columns;
	Cmiss_value_id temp_type_1,temp_type_2;
	Cmiss_variable_id *independent_variables;
	struct Cmiss_variable_derivative_type_specific_data *data;

	data=(struct Cmiss_variable_derivative_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&(data->variable)&&(0<data->order)&&
		(data->independent_variables),return_code,0)
	{
		if (temp_type_1=CREATE(Cmiss_value)())
		{
			if (return_code=Cmiss_variable_get_value_type(data->variable,
				temp_type_1))
			{
				/*???DB.  Not general, but hopefully OK for a while */
				number_of_rows=0;
				number_of_columns=0;
				if (CMISS_VALUE_IS_TYPE(FE_value)(temp_type_1))
				{
					number_of_rows=1;
				}
				else if (CMISS_VALUE_IS_TYPE(FE_value_vector)(temp_type_1))
				{
					if (!Cmiss_value_FE_value_vector_get_type(temp_type_1,
						&number_of_rows,(FE_value **)NULL))
					{
						number_of_rows=0;
					}
				}
				else if (CMISS_VALUE_IS_TYPE(derivative_matrix)(temp_type_1))
				{
					if (Cmiss_value_derivative_matrix_get_type(temp_type_1,
						(Cmiss_variable_id *)NULL,(int *)NULL,
						(Cmiss_variable_id **)NULL,&temp_type_2))
					{
						if (CMISS_VALUE_IS_TYPE(FE_value_matrix)(temp_type_2))
						{
							if (!Cmiss_value_FE_value_matrix_get_type(temp_type_2,
								&number_of_rows,&number_of_columns,(FE_value **)NULL))
							{
								number_of_rows=0;
							}
						}
					}
				}
				if (0<number_of_rows)
				{
					i=0;
					do
					{
						if (return_code=Cmiss_variable_get_value_type(
							(data->independent_variables)[i],temp_type_1))
						{
							if (CMISS_VALUE_IS_TYPE(FE_value)(temp_type_1))
							{
								number_of_columns=2*number_of_columns+1;
							}
							else if (CMISS_VALUE_IS_TYPE(FE_value_vector)(temp_type_1))
							{
								if (return_code=Cmiss_value_FE_value_vector_get_type(
									temp_type_1,&temp_number_of_columns,(FE_value **)NULL))
								{
									number_of_columns=
										number_of_columns*(temp_number_of_columns+1)+
										temp_number_of_columns;
								}
							}
						}
						i++;
					} while (return_code&&(i<data->order));
					if (return_code&&(0<number_of_columns))
					{
						if (ALLOCATE(independent_variables,Cmiss_variable_id ,
							data->order))
						{
							for (i=0;i<data->order;i++)
							{
								independent_variables[i]=(data->independent_variables)[i];
							}
							if (return_code=Cmiss_value_FE_value_matrix_set_type(
								temp_type_1,number_of_rows,number_of_columns,(FE_value *)NULL))
							{
								if (!Cmiss_value_derivative_matrix_set_type(type,
									data->variable,data->order,independent_variables,
									temp_type_1))
								{
									return_code=0;
								}
							}
							if (!return_code)
							{
								DEALLOCATE(independent_variables);
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
					return_code=0;
				}
			}
			if (!return_code)
			{
				DESTROY(Cmiss_value)(&temp_type_1);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_variable_derivative_get_value_type_type_specific.  "
				"Could not create <temp_type_1>");
		}
	}
}
END_CMISS_VARIABLE_GET_VALUE_TYPE_TYPE_SPECIFIC_FUNCTION(derivative)
#endif /* defined (OLD_CODE) */

static START_CMISS_VARIABLE_GET_VALUE_TYPE_TYPE_SPECIFIC_FUNCTION(derivative)
{
	int i,dependent_order;
	Cmiss_value_id temp_type_1;
	Cmiss_variable_id *dependent_independent_variables,dependent_variable,
		*independent_variables;
	struct Cmiss_variable_derivative_type_specific_data *data;

	data=(struct Cmiss_variable_derivative_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&(data->variable)&&(0<data->order)&&
		(data->independent_variables),return_code,0)
	{
		if (temp_type_1=CREATE(Cmiss_value)())
		{
			dependent_independent_variables=(Cmiss_variable_id *)NULL;
			dependent_variable=(Cmiss_variable_id)NULL;
			dependent_order=0;
			if (return_code=Cmiss_variable_get_value_type(data->variable,
				temp_type_1))
			{
				/*???DB.  Not general, but hopefully OK for a while */
				if (CMISS_VALUE_IS_TYPE(FE_value)(temp_type_1))
				{
					dependent_variable=data->variable;
				}
				else if (CMISS_VALUE_IS_TYPE(FE_value_vector)(temp_type_1))
				{
					dependent_variable=data->variable;
				}
				else if (CMISS_VALUE_IS_TYPE(derivative_matrix)(temp_type_1))
				{
					Cmiss_value_derivative_matrix_get_type(temp_type_1,
						&dependent_variable,&dependent_order,
						&dependent_independent_variables,(Cmiss_value_id **)NULL);
				}
				if (dependent_variable)
				{
					i=0;
					do
					{
						if (return_code=Cmiss_variable_get_value_type(
							(data->independent_variables)[i],temp_type_1))
						{
							if (!CMISS_VALUE_IS_TYPE(FE_value)(temp_type_1)&&
								!CMISS_VALUE_IS_TYPE(FE_value_vector)(temp_type_1))
							{
								return_code=0;
							}
						}
						i++;
					} while (return_code&&(i<data->order));
					if (return_code)
					{
						if (ALLOCATE(independent_variables,Cmiss_variable_id,
							dependent_order+(data->order)))
						{
							for (i=0;i<dependent_order;i++)
							{
								independent_variables[i]=dependent_independent_variables[i];
							}
							for (i=0;i<data->order;i++)
							{
								independent_variables[dependent_order+i]=
									(data->independent_variables)[i];
							}
							return_code=Cmiss_value_derivative_matrix_set_type(type,
								dependent_variable,dependent_order+(data->order),
								independent_variables,(Cmiss_value_id *)NULL);
							if (!return_code)
							{
								DEALLOCATE(independent_variables);
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
					return_code=0;
				}
			}
			DESTROY(Cmiss_value)(&temp_type_1);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_variable_derivative_get_value_type_type_specific.  "
				"Could not create <temp_type_1>");
		}
	}
}
END_CMISS_VARIABLE_GET_VALUE_TYPE_TYPE_SPECIFIC_FUNCTION(derivative)

static START_CMISS_VARIABLE_IS_DEFINED_TYPE_SPECIFIC_FUNCTION(derivative)
{
	struct Cmiss_variable_derivative_type_specific_data *data;

	data=(struct Cmiss_variable_derivative_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&(data->variable),return_code,0)
	{
		return_code=Cmiss_variable_is_defined(data->variable,
			(struct LIST(Cmiss_variable_value) *)NULL);
	}
}
END_CMISS_VARIABLE_IS_DEFINED_TYPE_SPECIFIC_FUNCTION(derivative)

static START_CMISS_VARIABLE_IS_INDEPENDENT_VARIABLE_OF_TYPE_SPECIFIC_FUNCTION(derivative)
{
	struct Cmiss_variable_derivative_type_specific_data *data;

	data=(struct Cmiss_variable_derivative_type_specific_data *)
		Cmiss_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data&&(data->variable),return_code,0)
	{
		return_code=Cmiss_variable_is_independent_variable_of(
			data->variable,independent_variable);
	}
}
END_CMISS_VARIABLE_IS_INDEPENDENT_VARIABLE_OF_TYPE_SPECIFIC_FUNCTION(derivative)

static START_CMISS_VARIABLE_NOT_IN_USE_TYPE_SPECIFIC_FUNCTION(derivative)
{
	int i;
	struct Cmiss_variable_derivative_type_specific_data *data;

	data=(struct Cmiss_variable_derivative_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable);
	ASSERT_IF(data&&(data->variable)&&(0<data->order)&&
		(data->independent_variables),return_code,0)
	{
		return_code=Cmiss_variable_not_in_use(data->variable);
		i=0;
		while (return_code&&(i<data->order))
		{
			return_code=Cmiss_variable_not_in_use(
				(data->independent_variables)[i]);
			i++;
		}
	}
}
END_CMISS_VARIABLE_NOT_IN_USE_TYPE_SPECIFIC_FUNCTION(derivative)

static START_CMISS_VARIABLE_OVERLAP_TYPE_SPECIFIC_FUNCTION(derivative)
{
	int i,j;
	struct Cmiss_variable_derivative_type_specific_data *data_1,*data_2;

	data_1=(struct Cmiss_variable_derivative_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable_1);
	data_2=(struct Cmiss_variable_derivative_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable_2);
	ASSERT_IF(data_1&&(data_1->variable)&&(0<data_1->order)&&
		(data_1->independent_variables)&&data_2&&(data_2->variable)&&
		(0<data_2->order)&&(data_2->independent_variables),return_code,0)
	{
		if (return_code=Cmiss_variable_overlap(data_1->variable,
			data_2->variable))
		{
			i=0;
			do
			{
				j=0;
				do
				{
					return_code=Cmiss_variable_overlap(
						(data_1->independent_variables)[i],
						(data_2->independent_variables)[j]);
					j++;
				} while (!return_code&&(j<data_2->order));
				i++;
			} while (!return_code&&(i<data_1->order));
		}
	}
}
END_CMISS_VARIABLE_OVERLAP_TYPE_SPECIFIC_FUNCTION(derivative)

static START_CMISS_VARIABLE_SAME_VARIABLE_TYPE_SPECIFIC_FUNCTION(derivative)
{
	int i,j;
	struct Cmiss_variable_derivative_type_specific_data *data_1,*data_2;

	data_1=(struct Cmiss_variable_derivative_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable_1);
	data_2=(struct Cmiss_variable_derivative_type_specific_data *)
		Cmiss_variable_get_type_specific_data(variable_2);
	ASSERT_IF(data_1&&(data_1->variable)&&(0<data_1->order)&&
		(data_1->independent_variables)&&data_2&&(data_2->variable)&&
		(0<data_2->order)&&(data_2->independent_variables),return_code,0)
	{
		if (data_1->order==data_2->order)
		{
			if (return_code=Cmiss_variable_same_variable(data_1->variable,
				data_2->variable))
			{
				i=0;
				do
				{
					j=0;
					do
					{
						return_code=Cmiss_variable_same_variable(
							(data_1->independent_variables)[i],
							(data_2->independent_variables)[j]);
						j++;
					} while (!return_code&&(j<data_2->order));
					i++;
				} while (return_code&&(i<data_1->order));
			}
		}
	}
}
END_CMISS_VARIABLE_SAME_VARIABLE_TYPE_SPECIFIC_FUNCTION(derivative)

static START_CMISS_VARIABLE_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(derivative)
{
	struct Cmiss_variable_derivative_type_specific_data *data;

	data=(struct Cmiss_variable_derivative_type_specific_data *)
		Cmiss_variable_get_type_specific_data(dependent_variable);
	ASSERT_IF(data&&(data->variable),return_code,0)
	{
		return_code=Cmiss_variable_set_independent_variable_value(data->variable,
			independent_variable,value);
	}
}
END_CMISS_VARIABLE_SET_INDEPENDENT_VARIABLE_VALUE_TYPE_SPECIFIC_FUNCTION(derivative)

/*
Module types
------------
*/
struct Cmiss_variable
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
==============================================================================*/
{
	/* the name/identifier of the Cmiss_variable */
	char *name;
	/* the type string identifies the variable's type.  It points to a string that
		is shared by all variables of the same type and should not be copied */
	char *type_string;
	/* information that is specific to the type */
	Cmiss_variable_type_specific_data *type_specific_data;
	/* methods.   To add a new method, add
		- a field here in struct Cmiss_variable
		- a function type declaration in computed_variable_private.h
		- an argument and an assigment to Cmiss_variable_establish_methods
		The compiler will force the other changes because
		Cmiss_variable_establish_methods is used in
		- Cmiss_variable_clear_type
		- CREATE(Cmiss_variable)
		- MANAGER_COPY_WITHOUT_IDENTIFIER(Cmiss_variable,name) */
	Cmiss_variable_clear_type_specific_function clear_type_specific_function;
	Cmiss_variable_duplicate_data_type_specific_function
		duplicate_data_type_specific_function;
	Cmiss_variable_evaluate_derivative_type_specific_function
		evaluate_derivative_type_specific_function;
	Cmiss_variable_evaluate_type_specific_function
		evaluate_type_specific_function;
	Cmiss_variable_get_independent_variable_value_type_specific_function
		get_independent_variable_value_type_specific_function;
	Cmiss_variable_get_set_independent_variable_value_type_specific_function
		get_set_independent_variable_value_type_specific_function;
	Cmiss_variable_get_value_type_type_specific_function
		get_value_type_type_specific_function;
	Cmiss_variable_is_defined_type_specific_function
		is_defined_type_specific_function;
	Cmiss_variable_is_independent_variable_of_type_specific_function
		is_independent_variable_of_type_specific_function;
	Cmiss_variable_not_in_use_type_specific_function
		not_in_use_type_specific_function;
	Cmiss_variable_overlap_type_specific_function overlap_type_specific_function;
	Cmiss_variable_same_variable_type_specific_function
		same_variable_type_specific_function;
	Cmiss_variable_set_independent_variable_value_type_specific_function
		set_independent_variable_value_type_specific_function;
	/* source variables for independent variables */
	int number_of_source_variables;
	Cmiss_variable_id *independent_variables;
	Cmiss_variable_id *source_variables;

	int access_count;
}; /* struct Cmiss_variable */

FULL_DECLARE_INDEXED_LIST_TYPE(Cmiss_variable);

FULL_DECLARE_MANAGER_TYPE(Cmiss_variable);

struct Cmiss_variable_value
/*******************************************************************************
LAST MODIFIED : 27 January 2003

DESCRIPTION :
A variable/value pair for specifying the value of the variable.
==============================================================================*/
{
	Cmiss_variable_id variable;
	Cmiss_value_id value;
	int access_count;
}; /* struct Cmiss_variable_value */

FULL_DECLARE_LIST_TYPE(Cmiss_variable_value);

struct Cmiss_variable_package
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Allows sharing of Cmiss_variables with the same name.
==============================================================================*/
{
	struct MANAGER(Cmiss_variable) *variable_manager;
}; /* struct Cmiss_variable_package */

/*
Module functions
----------------
*/
DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Cmiss_variable,name,char *,strcmp)

DECLARE_LOCAL_MANAGER_FUNCTIONS(Cmiss_variable)

static int Cmiss_variable_is_source_variable_conditional(
	Cmiss_variable_id variable,void *other_variable_void)
/*******************************************************************************
LAST MODIFIED : 4 February 2003

DESCRIPTION :
List conditional function returning true if <variable> is a source variable of
<other_variable>, ie. <other_variable> depends on <variable>.

NB.  Because used for a list of source variable is not
Cmiss_variable_depends_on_Cmiss_variable_conditional (other way round).
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_variable_is_source_variable_conditional);
	return_code=Cmiss_variable_depends_on_Cmiss_variable(
		(Cmiss_variable_id )other_variable_void,variable);
	LEAVE;

	return (return_code);
} /* Cmiss_variable_is_source_variable_conditional */

static int Cmiss_variable_get_set_independent_variable_value_iterator(
	struct Cmiss_variable_value *independent_variable_value,
	void *dependent_variable_void)
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
List iterator function returning true if swaps the <independent_variable_value>
with the current one for the <dependent_variable_void>.
==============================================================================*/
{
	int return_code;
	Cmiss_variable_id dependent_variable;

	ENTER(Cmiss_variable_get_set_independent_variable_value_iterator);
	return_code=0;
	dependent_variable=(Cmiss_variable_id )dependent_variable_void;
	ASSERT_IF(independent_variable_value&&dependent_variable&&
		(dependent_variable->
		get_set_independent_variable_value_type_specific_function),return_code,0)
	{
		return_code=(dependent_variable->
			get_set_independent_variable_value_type_specific_function)(
			dependent_variable,independent_variable_value->variable,
			independent_variable_value->value);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_get_set_independent_variable_value_iterator */

/*
Friend functions
----------------
*/
int Cmiss_variable_establish_methods(Cmiss_variable_id variable,
	Cmiss_variable_clear_type_specific_function clear_type_specific_function,
	Cmiss_variable_duplicate_data_type_specific_function
	duplicate_data_type_specific_function,
	Cmiss_variable_evaluate_derivative_type_specific_function
	evaluate_derivative_type_specific_function,
	Cmiss_variable_evaluate_type_specific_function
	evaluate_type_specific_function,
	Cmiss_variable_get_independent_variable_value_type_specific_function
	get_independent_variable_value_type_specific_function,
	Cmiss_variable_get_set_independent_variable_value_type_specific_function
	get_set_independent_variable_value_type_specific_function,
	Cmiss_variable_get_value_type_type_specific_function
	get_value_type_type_specific_function,
	Cmiss_variable_is_defined_type_specific_function
	is_defined_type_specific_function,
	Cmiss_variable_is_independent_variable_of_type_specific_function
	is_independent_variable_of_type_specific_function,
	Cmiss_variable_not_in_use_type_specific_function
	not_in_use_type_specific_function,
	Cmiss_variable_overlap_type_specific_function overlap_type_specific_function,
	Cmiss_variable_same_variable_type_specific_function
	same_variable_type_specific_function,
	Cmiss_variable_set_independent_variable_value_type_specific_function
	set_independent_variable_value_type_specific_function)
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Sets the methods for the <variable>.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_variable_establish_methods);
	return_code=0;
	if (variable)
	{
		variable->clear_type_specific_function=clear_type_specific_function;
		variable->duplicate_data_type_specific_function=
			duplicate_data_type_specific_function;
		variable->evaluate_derivative_type_specific_function=
			evaluate_derivative_type_specific_function;
		variable->evaluate_type_specific_function=evaluate_type_specific_function;
		variable->get_independent_variable_value_type_specific_function=
			get_independent_variable_value_type_specific_function;
		variable->get_set_independent_variable_value_type_specific_function=
			get_set_independent_variable_value_type_specific_function;
		variable->get_value_type_type_specific_function=
			get_value_type_type_specific_function;
		variable->is_defined_type_specific_function=
			is_defined_type_specific_function;
		variable->is_independent_variable_of_type_specific_function=
			is_independent_variable_of_type_specific_function;
		variable->not_in_use_type_specific_function=
			not_in_use_type_specific_function;
		variable->overlap_type_specific_function=overlap_type_specific_function;
		variable->same_variable_type_specific_function=
			same_variable_type_specific_function;
		variable->set_independent_variable_value_type_specific_function=
			set_independent_variable_value_type_specific_function;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_variable_establish_methods.  "
			"Missing variable");
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_establish_methods */

Cmiss_variable_type_specific_data *Cmiss_variable_get_type_specific_data(
	Cmiss_variable_id variable)
/*******************************************************************************
LAST MODIFIED : 6 February 2003

DESCRIPTION :
Returns the type specific data for the <variable>.
==============================================================================*/
{
	Cmiss_variable_type_specific_data *data;

	ENTER(Cmiss_variable_get_type_specific_data);
	data=(Cmiss_variable_type_specific_data *)NULL;
	ASSERT_IF(variable,data,(Cmiss_variable_type_specific_data *)NULL)
	{
		data=variable->type_specific_data;
	}
	LEAVE;

	return (data);
} /* Cmiss_variable_get_type_specific_data */

int Cmiss_variable_set_type_specific_information(
	Cmiss_variable_id variable,char *type_string,
	void *type_specific_data)
/*******************************************************************************
LAST MODIFIED : 24 April 2003

DESCRIPTION :
Sets the type specific information for the <variable>.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_variable_set_type_specific_information);
	return_code=0;
	/* check arguments */
	if (variable&&type_string&&type_specific_data)
	{
		variable->type_specific_data=type_specific_data;
		variable->type_string=type_string;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_variable_set_type_specific_information.  "
			"Invalid argument(s).  %p %p %p",variable,type_string,type_specific_data);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_set_type_specific_information */

int Cmiss_variable_clear_type(Cmiss_variable_id variable)
/*******************************************************************************
LAST MODIFIED : 25 April 2003

DESCRIPTION :
Used internally by DESTROY and Cmiss_variable_set_type_*() functions to
deallocate or deaccess data specific to any Cmiss_variable_type.  Functions
changing the type of the Cmiss_variable should
- allocate any dynamic data needed for the type
- call this function to clear what is currently in the variable
- then set values
to ensure that the variable is not left in an invalid state.
==============================================================================*/
{
	int i,return_code;
	Cmiss_variable_id *independent_variable_address,*source_variable_address;

	ENTER(Cmiss_variable_clear_type);
	return_code=0;
	/* check arguments */
	if (variable)
	{
		/* clear the type specific data first */
		if (variable->type_specific_data)
		{
			if (variable->clear_type_specific_function)
			{
				return_code=(variable->clear_type_specific_function)(variable);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_variable_clear_type.  "
					"Type specific data but no function to clear it");
				return_code=0;
			}
			DEALLOCATE(variable->type_specific_data);
		}
		/* clear all methods */
		Cmiss_variable_establish_methods(variable,
			(Cmiss_variable_clear_type_specific_function)NULL,
			(Cmiss_variable_duplicate_data_type_specific_function)NULL,
			(Cmiss_variable_evaluate_derivative_type_specific_function)NULL,
			(Cmiss_variable_evaluate_type_specific_function)NULL,
			(Cmiss_variable_get_independent_variable_value_type_specific_function)NULL,
			(Cmiss_variable_get_set_independent_variable_value_type_specific_function)NULL,
			(Cmiss_variable_get_value_type_type_specific_function)NULL,
			(Cmiss_variable_is_defined_type_specific_function)NULL,
			(Cmiss_variable_is_independent_variable_of_type_specific_function)NULL,
			(Cmiss_variable_not_in_use_type_specific_function)NULL,
			(Cmiss_variable_overlap_type_specific_function)NULL,
			(Cmiss_variable_same_variable_type_specific_function)NULL,
			(Cmiss_variable_set_independent_variable_value_type_specific_function)
			NULL);
		/* clear all sources */
		i=variable->number_of_source_variables;
		independent_variable_address=variable->independent_variables;
		source_variable_address=variable->source_variables;
		while (i>0)
		{
			DEACCESS(Cmiss_variable)(independent_variable_address);
			DEACCESS(Cmiss_variable)(source_variable_address);
			independent_variable_address++;
			source_variable_address++;
			i--;
		}
		DEALLOCATE(variable->independent_variables);
		DEALLOCATE(variable->source_variables);
		variable->number_of_source_variables=0;
		variable->type_string=(char *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_variable_clear_type.  "
			"Missing variable");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_clear_type */

int Cmiss_variable_not_in_use(Cmiss_variable_id variable)
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
To give access to Cmiss_variable_not_in_use_type_specific_function method.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_variable_not_in_use);
	return_code=0;
	/* check arguments */
	if (variable)
	{
		if (variable->not_in_use_type_specific_function)
		{
			return_code=(variable->not_in_use_type_specific_function)(variable);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_variable_not_in_use.  "
				"Type specific data but no function to clear it");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_variable_not_in_use.  "
			"Missing variable");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_not_in_use */

int Cmiss_variable_for_each_source_variable(
	Cmiss_variable_id variable,
	Cmiss_variable_source_variable_iterator iterator,void *user_data)
/*******************************************************************************
LAST MODIFIED : 28 April 2003

DESCRIPTION :
Applies the <iterator> to each of the <variable>s source/independent variable
pairs until finish or the <iterator> returns zero.
==============================================================================*/
{
	int i,return_code;
	Cmiss_variable_id *independent_variable_address;
	Cmiss_variable_id *source_variable_address;

	ENTER(Cmiss_variable_for_each_source_variable);
	return_code=0;
	/* check arguments */
	if (variable&&iterator)
	{
		return_code=1;
		i=variable->number_of_source_variables;
		source_variable_address=variable->source_variables;
		independent_variable_address=variable->independent_variables;
		while (return_code&&(i>0))
		{
			return_code=(iterator)(variable,*source_variable_address,
				*independent_variable_address,user_data);
			i--;
			source_variable_address++;
			independent_variable_address++;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_variable_for_each_source_variable.  "
			"Invalid argument(s).  %p %p",variable,iterator);
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_for_each_source_variable */

/*
Global functions
----------------
*/
#if defined (OLD_CODE)
/*???DB.  Changing from fe_value_matrix to matrix and to storing the blocks
	within fe_value_matrix as separate matrices */
int Cmiss_value_derivative_matrix_set_type(Cmiss_value_id value,
	Cmiss_variable_id dependent_variable,int order,
	Cmiss_variable_id *independent_variables,
	Cmiss_value_id fe_value_matrix)
/*******************************************************************************
LAST MODIFIED : 22 April 2003

DESCRIPTION :
Makes <value> of type derivative_matrix and sets its <fe_value_matrix>,
<dependent_variable>, <order> and <independent_variables>.  This function
ACCESSes the <dependent_variable> and <independent_variables>.  After success,
the <value> is responsible for DESTROY/DEACCESSing <fe_value_matrix>,
<dependent_variable> and <independent_variables>.

The number of rows is the number of values for the <dependent_variable>.  The
number of columns is:
	product(1+number_of_values,for each independent_variable)-1
The columns are:
	d(dependent_variable)/d(independent_variables[0])
	d(dependent_variable)/d(independent_variables[1])
	d2(dependent_variable)/d(independent_variables[0])d(independent_variables[1])
	d(dependent_variable)/d(independent_variables[2])
	d2(dependent_variable)/d(independent_variables[0])d(independent_variables[2])
	d2(dependent_variable)/d(independent_variables[1])d(independent_variables[2])
	d3(dependent_variable)/d(independent_variables[0])d(independent_variables[1])
		d(independent_variables[2])
	...
	d(dependent_variable)/d(independent_variables[order-1])
	d2(dependent_variable)/d(independent_variables[0])
		d(independent_variables[order-1])
	d2(dependent_variable)/d(independent_variables[1])
		d(independent_variables[order-1])
	d3(dependent_variable)/d(independent_variables[0])d(independent_variables[1])
		d(independent_variables[order-1])
	d2(dependent_variable)/d(independent_variables[2])
		d(independent_variables[order-1])
	d3(dependent_variable)/d(independent_variables[0])d(independent_variables[2])
		d(independent_variables[order-1])
	d3(dependent_variable)/d(independent_variables[1])d(independent_variables[2])
		d(independent_variables[order-1])
	d4(dependent_variable)/d(independent_variables[0])d(independent_variables[1])
		d(independent_variables[2])d(independent_variables[order-1])
	...
	d{order}(dependent_variable)/d(independent_variables[0])
		d(independent_variables[1]) ... d(independent_variables[order-1])
==============================================================================*/
{
	int i,return_code;
	struct Cmiss_value_derivative_matrix_type_specific_data *data;

	ENTER(Cmiss_value_derivative_matrix_set_type);
	return_code=0;
	/* check arguments */
	if (value&&(0<order)&&dependent_variable&&independent_variables&&
		CMISS_VALUE_IS_TYPE(FE_value_matrix)(fe_value_matrix))
	{
		i=0;
		while ((i<order)&&independent_variables[i])
		{
			i++;
		}
		if (i<=order)
		{
			/* 1.  Make dynamic allocations for any new type-specific data */
			if (ALLOCATE(data,
				struct Cmiss_value_derivative_matrix_type_specific_data,1))
			{
				/* 2.  Clear current type-specific data */
				Cmiss_value_clear_type(value);
				/* 3.  Establish the new type */
				Cmiss_value_set_type_specific_information(value,
					Cmiss_value_derivative_matrix_type_string,(void *)data);
				data->order=order;
				data->fe_value_matrix=ACCESS(Cmiss_value)(fe_value_matrix);
				data->dependent_variable=ACCESS(Cmiss_variable)(dependent_variable);
				data->independent_variables=independent_variables;
				for (i=0;i<order;i++)
				{
					ACCESS(Cmiss_variable)(independent_variables[i]);
				}
				/* set all the methods */
				return_code=CMISS_VALUE_ESTABLISH_METHODS(value,derivative_matrix);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_value_derivative_matrix_set_type.  "
					"Could not ALLOCATE type specific data");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_value_derivative_matrix_set_type.  "
				"Missing independent variable(s)");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_value_derivative_matrix_set_type.  "
			"Missing value");
	}
	LEAVE;

	return (return_code);
} /* Cmiss_value_derivative_matrix_set_type */
#endif /* defined (OLD_CODE) */

int Cmiss_value_derivative_matrix_set_type(Cmiss_value_id value,
	Cmiss_variable_id dependent_variable,int order,
	Cmiss_variable_id *independent_variables,
	Cmiss_value_id *matrices)
/*******************************************************************************
LAST MODIFIED : 7 May 2003

DESCRIPTION :
Makes <value> of type derivative_matrix and sets its <matrices>,
<dependent_variable>, <order> and <independent_variables>.  This function
ACCESSes the <dependent_variable>, <matrices> and <independent_variables>.
After success, the <value> is responsible for DEALLOCATE/DEACCESSing <matrices>,
<dependent_variable> and <independent_variables>.

<matrices> may be NULL.  If not, it should have 2**order-1 entries which are:
	d(dependent_variable)/d(independent_variables[0])
	d(dependent_variable)/d(independent_variables[1])
	d2(dependent_variable)/d(independent_variables[0])d(independent_variables[1])
	d(dependent_variable)/d(independent_variables[2])
	d2(dependent_variable)/d(independent_variables[0])d(independent_variables[2])
	d2(dependent_variable)/d(independent_variables[1])d(independent_variables[2])
	d3(dependent_variable)/d(independent_variables[0])d(independent_variables[1])
		d(independent_variables[2])
	...
	d(dependent_variable)/d(independent_variables[order-1])
	d2(dependent_variable)/d(independent_variables[0])
		d(independent_variables[order-1])
	d2(dependent_variable)/d(independent_variables[1])
		d(independent_variables[order-1])
	d3(dependent_variable)/d(independent_variables[0])d(independent_variables[1])
		d(independent_variables[order-1])
	d2(dependent_variable)/d(independent_variables[2])
		d(independent_variables[order-1])
	d3(dependent_variable)/d(independent_variables[0])d(independent_variables[2])
		d(independent_variables[order-1])
	d3(dependent_variable)/d(independent_variables[1])d(independent_variables[2])
		d(independent_variables[order-1])
	d4(dependent_variable)/d(independent_variables[0])d(independent_variables[1])
		d(independent_variables[2])d(independent_variables[order-1])
	...
	d{order}(dependent_variable)/d(independent_variables[0])
		d(independent_variables[1]) ... d(independent_variables[order-1])

The number of rows for each entry is the number of values for the
<dependent_variable>.  The number of columns for each entry is
	product(1+number_of_values, for each <independent_variable> involved in the
		entry)
==============================================================================*/
{
	Cmiss_value_id *matrix;
	int i,number_of_matrices,return_code;
	struct Cmiss_value_derivative_matrix_type_specific_data *data;

	ENTER(Cmiss_value_derivative_matrix_set_type);
	return_code=0;
	/* check arguments */
	if (value&&(0<order)&&dependent_variable&&independent_variables)
	{
		i=0;
		number_of_matrices=1;
		while ((i<order)&&independent_variables[i])
		{
			i++;
			number_of_matrices *= 2;
		}
		number_of_matrices -= 1;
		if (i==order)
		{
			if (matrix=matrices)
			{
				i=number_of_matrices;
				while ((i>0)&&CMISS_VALUE_IS_TYPE(matrix)(*matrix))
				{
					matrix++;
					i--;
				}
			}
			else
			{
				i=0;
			}
			if (0==i)
			{
				/* 1.  Make dynamic allocations for any new type-specific data */
				if (ALLOCATE(data,
					struct Cmiss_value_derivative_matrix_type_specific_data,1))
				{
					/* 2.  Clear current type-specific data */
					Cmiss_value_clear_type(value);
					/* 3.  Establish the new type */
					Cmiss_value_set_type_specific_information(value,
						Cmiss_value_derivative_matrix_type_string,(void *)data);
					data->order=order;
					data->matrices=matrices;
					if (matrix=matrices)
					{
						i=number_of_matrices;
						while (i>0)
						{
							ACCESS(Cmiss_value)(*matrix);
							matrix++;
							i--;
						}
					}
					data->dependent_variable=ACCESS(Cmiss_variable)(dependent_variable);
					data->independent_variables=independent_variables;
					for (i=0;i<order;i++)
					{
						ACCESS(Cmiss_variable)(independent_variables[i]);
					}
					/* set all the methods */
					return_code=CMISS_VALUE_ESTABLISH_METHODS(value,derivative_matrix);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Cmiss_value_derivative_matrix_set_type.  "
						"Could not ALLOCATE type specific data");
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_value_derivative_matrix_set_type.  "
				"Missing independent variable(s)");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_value_derivative_matrix_set_type.  "
			"Missing value");
	}
	LEAVE;

	return (return_code);
} /* Cmiss_value_derivative_matrix_set_type */

DECLARE_CMISS_VALUE_IS_TYPE_FUNCTION(derivative_matrix)

#if defined (OLD_CODE)
/*???DB.  Changing from fe_value_matrix to matrix and to storing the blocks
	within fe_value_matrix as separate matrices */
int Cmiss_value_derivative_matrix_get_type(Cmiss_value_id value,
	Cmiss_variable_id *dependent_variable_address,int *order_address,
	Cmiss_variable_id **independent_variables_address,
	Cmiss_value_id *fe_value_matrix_address)
/*******************************************************************************
LAST MODIFIED : 27 March 2003

DESCRIPTION :
If <value> is of type derivative_matrix, gets its <*fe_value_matrix_address>,
<*dependent_variable_address>, <*order_address> and
<*independent_variables_address>.

The calling program must not DEALLOCATE the returned structures.
==============================================================================*/
{
	int return_code;
	struct Cmiss_value_derivative_matrix_type_specific_data *data;

	ENTER(Cmiss_value_derivative_matrix_get_type);
	return_code=0;
	/* check arguments */
	if (value&&CMISS_VALUE_IS_TYPE(derivative_matrix)(value)&&
		(dependent_variable_address||order_address||independent_variables_address||
		fe_value_matrix_address))
	{
		data=(struct Cmiss_value_derivative_matrix_type_specific_data *)
			Cmiss_value_get_type_specific_data(value);
		ASSERT_IF(data,return_code,0)
		{
			if (order_address)
			{
				*order_address=data->order;
			}
			if (dependent_variable_address)
			{
				*dependent_variable_address=data->dependent_variable;
			}
			if (independent_variables_address)
			{
				*independent_variables_address=data->independent_variables;
			}
			if (fe_value_matrix_address)
			{
				*fe_value_matrix_address=data->fe_value_matrix;
			}
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_value_derivative_matrix_get_type.  "
			"Invalid argument(s).  %p %p %p %p %p",value,dependent_variable_address,
			order_address,independent_variables_address,fe_value_matrix_address);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_value_derivative_matrix_get_type */
#endif /* defined (OLD_CODE) */

int Cmiss_value_derivative_matrix_get_type(Cmiss_value_id value,
	Cmiss_variable_id *dependent_variable_address,int *order_address,
	Cmiss_variable_id **independent_variables_address,
	Cmiss_value_id **matrices_address)
/*******************************************************************************
LAST MODIFIED : 7 May 2003

DESCRIPTION :
If <value> is of type derivative_matrix, gets its <*dependent_variable_address>,
<*order_address>, <*independent_variables_address> and <*matrices_address>.

The calling program must not DEALLOCATE the returned structures.
==============================================================================*/
{
	int return_code;
	struct Cmiss_value_derivative_matrix_type_specific_data *data;

	ENTER(Cmiss_value_derivative_matrix_get_type);
	return_code=0;
	/* check arguments */
	if (value&&CMISS_VALUE_IS_TYPE(derivative_matrix)(value)&&
		(dependent_variable_address||order_address||independent_variables_address||
		matrices_address))
	{
		data=(struct Cmiss_value_derivative_matrix_type_specific_data *)
			Cmiss_value_get_type_specific_data(value);
		ASSERT_IF(data,return_code,0)
		{
			if (order_address)
			{
				*order_address=data->order;
			}
			if (dependent_variable_address)
			{
				*dependent_variable_address=data->dependent_variable;
			}
			if (independent_variables_address)
			{
				*independent_variables_address=data->independent_variables;
			}
			if (matrices_address)
			{
				*matrices_address=data->matrices;
			}
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_value_derivative_matrix_get_type.  "
			"Invalid argument(s).  %p %p %p %p %p",value,dependent_variable_address,
			order_address,independent_variables_address,matrices_address);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_value_derivative_matrix_get_type */

int Cmiss_value_derivative_matrix_get_matrix(Cmiss_value_id value,int order,
	Cmiss_variable_id *independent_variables,Cmiss_value_id *matrix_address)
/*******************************************************************************
LAST MODIFIED : 19 May 2003

DESCRIPTION :
If <value> is of type derivative_matrix, this function returns the specified
partial derivative (<order> and <independent_variables>) in <*matrix_address>.

???DB.  Extend so that can have an independent varible that is a subset of
	one of the independent variables for the derivative matrix.  eg nodal values
	for a particular node as a subset of all nodal values
==============================================================================*/
{
	Cmiss_value_id *matrix_ptr;
	Cmiss_variable_id *independent_variables_full;
	int found,i,j,offset,order_full,return_code;

	ENTER(Cmiss_value_derivative_matrix_get_matrix);
	return_code=0;
	/* check arguments */
	if (value&&(0<order)&&independent_variables&&matrix_address&&
		Cmiss_value_derivative_matrix_get_type(value,(Cmiss_variable_id *)NULL,
		&order_full,&independent_variables_full,&matrix_ptr))
	{
		return_code=1;
		offset=1;
		for (i=order_full;i>0;i--)
		{
			matrix_ptr += offset;
			offset *= 2;
		}
		matrix_ptr--;
		i=order-1;
		j=order_full-1;
		do
		{
			offset /= 2;
			if (i>=0)
			{
				found=Cmiss_variable_same_variable(independent_variables[i],
					independent_variables_full[j]);
			}
			else
			{
				found=0;
			}
			if (found)
			{
				i--;
			}
			else
			{
				matrix_ptr -= offset;
			}
			j--;
		} while (j>=0);
		if (i<0)
		{
			*matrix_address= *matrix_ptr;
		}
		else
		{
			*matrix_address=(Cmiss_value_id)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_value_derivative_matrix_get_matrix.  "
			"Invalid argument(s).  %p %d %p %p",value,order,independent_variables,
			matrix_address);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_value_derivative_matrix_get_matrix */

Cmiss_variable_id CREATE(Cmiss_variable)(
	struct Cmiss_variable_package *package,char *name)
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
If there is a <package> and a Cmiss_variable in the <package> with the
specified <name> then the Cmiss_variable is returned.

Otherwise, an empty variable with specified <name> and no type, dependent
variables or independent variables is created and returned.  If there is a
<package> then the created Cmiss_variables is put in the <package>.  The
<name> is copied.

Each type of variable has its own "set_type" function.
==============================================================================*/
{
	Cmiss_variable_id variable;

	ENTER(CREATE(Cmiss_variable));
	variable=(Cmiss_variable_id )NULL;
	if (name&&(!package||(package&&(package->variable_manager))))
	{
		if (package)
		{
			variable=FIND_BY_IDENTIFIER_IN_MANAGER(Cmiss_variable,name)(name,
				package->variable_manager);
		}
		if (!variable)
		{
			if (ALLOCATE(variable,struct Cmiss_variable,1)&&
				(variable->name=duplicate_string(name)))
			{
				/* initialise data */
				variable->type_string=(char *)NULL;
				variable->type_specific_data=NULL;
				/* initialise methods */
				Cmiss_variable_establish_methods(variable,
					(Cmiss_variable_clear_type_specific_function)NULL,
					(Cmiss_variable_duplicate_data_type_specific_function)NULL,
					(Cmiss_variable_evaluate_derivative_type_specific_function)NULL,
					(Cmiss_variable_evaluate_type_specific_function)NULL,
					(Cmiss_variable_get_independent_variable_value_type_specific_function)NULL,
					(Cmiss_variable_get_set_independent_variable_value_type_specific_function)NULL,
					(Cmiss_variable_get_value_type_type_specific_function)NULL,
					(Cmiss_variable_is_defined_type_specific_function)NULL,
					(Cmiss_variable_is_independent_variable_of_type_specific_function)
					NULL,
					(Cmiss_variable_not_in_use_type_specific_function)NULL,
					(Cmiss_variable_overlap_type_specific_function)NULL,
					(Cmiss_variable_same_variable_type_specific_function)NULL,
					(Cmiss_variable_set_independent_variable_value_type_specific_function)NULL);
				/* initialize source variables */
				variable->number_of_source_variables=0;
				variable->independent_variables=(Cmiss_variable_id *)NULL;
				variable->source_variables=(Cmiss_variable_id *)NULL;
				/* initialise access_count */
				variable->access_count=0;
				if (package)
				{
					if (!ADD_OBJECT_TO_MANAGER(Cmiss_variable)(variable,
						package->variable_manager))
					{
						display_message(ERROR_MESSAGE,"CREATE(Cmiss_variable).  "
							"Could not add variable to manager");
						DESTROY(Cmiss_variable)(&variable);
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"CREATE(Cmiss_variable).  "
					"Insufficient memory");
				DEALLOCATE(variable);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Cmiss_variable).  "
			"Missing name or invalid package.  %p %p",name,package);
	}
	LEAVE;

	return (variable);
} /* CREATE(Cmiss_variable) */

int DESTROY(Cmiss_variable)(Cmiss_variable_id *variable_address)
/*******************************************************************************
LAST MODIFIED : 5 February 2003

DESCRIPTION :
Frees memory/deaccess objects for Cmiss_variable at <*variable_address>.
==============================================================================*/
{
	int return_code;
	Cmiss_variable_id variable;

	ENTER(DESTROY(Cmiss_variable));
	return_code=0;
	if (variable_address&&(variable= *variable_address))
	{
		if (0>=variable->access_count)
		{
			DEALLOCATE(variable->name);
			Cmiss_variable_clear_type(variable);
			DEALLOCATE(*variable_address);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"DESTROY(Cmiss_variable).  Positive access_count");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Cmiss_variable).  Missing variable");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Cmiss_variable) */

int Cmiss_variable_copy(Cmiss_variable_id destination,
	Cmiss_variable_id source)
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Copies the type and contents from <source> to <destination>.

???DB.  What if the access_count>0?  Put in Cmiss_variable_clear_type?
==============================================================================*/
{
	int i,return_code;

	ENTER(Cmiss_variable_copy);
	return_code=0;
	/* check arguments */
	if (destination&&source)
	{
		if (Cmiss_variable_clear_type(destination))
		{
			/* initialise data */
			destination->type_string=source->type_string;
			if (source->duplicate_data_type_specific_function)
			{
				destination->type_specific_data=
					(source->duplicate_data_type_specific_function)(source);
			}
			/* initialise methods */
			Cmiss_variable_establish_methods(destination,
				source->clear_type_specific_function,
				source->duplicate_data_type_specific_function,
				source->evaluate_derivative_type_specific_function,
				source->evaluate_type_specific_function,
				source->get_independent_variable_value_type_specific_function,
				source->get_set_independent_variable_value_type_specific_function,
				source->get_value_type_type_specific_function,
				source->is_defined_type_specific_function,
				source->is_independent_variable_of_type_specific_function,
				source->not_in_use_type_specific_function,
				source->overlap_type_specific_function,
				source->same_variable_type_specific_function,
				source->set_independent_variable_value_type_specific_function);
			/* initialize source variables */
			destination->number_of_source_variables=
				source->number_of_source_variables;
			ALLOCATE(destination->independent_variables,Cmiss_variable_id ,
				destination->number_of_source_variables);
			ALLOCATE(destination->source_variables,Cmiss_variable_id ,
				destination->number_of_source_variables);
			if ((destination->independent_variables)&&(destination->source_variables))
			{
				for (i=0;i<destination->number_of_source_variables;i++)
				{
					(destination->independent_variables)[i]=ACCESS(Cmiss_variable)(
						(source->independent_variables)[i]);
					(destination->source_variables)[i]=ACCESS(Cmiss_variable)(
						(source->source_variables)[i]);
				}
				return_code=1;
			}
			else
			{
				DEALLOCATE(destination->independent_variables);
				DEALLOCATE(destination->source_variables);
				destination->number_of_source_variables=0;
				Cmiss_variable_clear_type(destination);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_variable_copy.  "
			"Invalid argument(s).  %p %p",destination,source);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_copy */

DECLARE_OBJECT_FUNCTIONS(Cmiss_variable)
DECLARE_DEFAULT_GET_OBJECT_NAME_FUNCTION(Cmiss_variable)

DECLARE_INDEXED_LIST_FUNCTIONS(Cmiss_variable)
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Cmiss_variable,name,
	char *,strcmp)

DECLARE_INDEXED_LIST_IDENTIFIER_CHANGE_FUNCTIONS(Cmiss_variable,name)

PROTOTYPE_MANAGER_COPY_WITH_IDENTIFIER_FUNCTION(Cmiss_variable,name)
{
	char *name;
	int return_code;

	ENTER(MANAGER_COPY_WITH_IDENTIFIER(Cmiss_variable,name));
	if (source&&destination)
	{
		if (source->name)
		{
			if (ALLOCATE(name,char,strlen(source->name)+1))
			{
				strcpy(name,source->name);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"MANAGER_COPY_WITH_IDENTIFIER(Cmiss_variable,name).  "
					"Insufficient memory");
				return_code=0;
			}
		}
		else
		{
			name=(char *)NULL;
			return_code=1;
		}
		if (return_code)
		{
			if (return_code=MANAGER_COPY_WITHOUT_IDENTIFIER(Cmiss_variable,name)(
				destination,source))
			{
				/* copy values */
				DEALLOCATE(destination->name);
				destination->name=name;
			}
			else
			{
				DEALLOCATE(name);
				display_message(ERROR_MESSAGE,
					"MANAGER_COPY_WITH_IDENTIFIER(Cmiss_variable,name).  "
					"Could not copy without identifier");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_WITH_IDENTIFIER(Cmiss_variable,name).  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITH_IDENTIFIER(Cmiss_variable,name) */

PROTOTYPE_MANAGER_COPY_WITHOUT_IDENTIFIER_FUNCTION(Cmiss_variable,name)
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Do not allow copy if:
- it creates a self-referencing variable (one that depends on itself) which will
	result in an infinite loop;

???RC.  Previously denied copy if read_only flag was set in the destination
	variable.  However, this makes it impossible to modify computed variables that
	wrap FE_fields when the latter changes.  Hence, now leave it up to
	define/destroy functions to check if read_only flag is set.
==============================================================================*/
{
	int i,return_code;
	Cmiss_variable_id *independent_variable_address,*source_variable_address;
	void *type_specific_data;

	ENTER(MANAGER_COPY_WITHOUT_IDENTIFIER(Cmiss_variable,name));
	if (source&&destination)
	{
		/* check <source> does not depend on <destination> else infinite loop */
		if (Cmiss_variable_depends_on_Cmiss_variable(source,destination))
		{
			display_message(ERROR_MESSAGE,
				"MANAGER_COPY_WITHOUT_IDENTIFIER(Cmiss_variable,name).  "
				"Cannot make variable depend on itself");
			return_code=0;
		}
		else
		{
			type_specific_data=NULL;
			return_code=1;
			if (source->type_specific_data)
			{
				if (source->duplicate_data_type_specific_function)
				{
					if (!(type_specific_data=
						(source->duplicate_data_type_specific_function)(source)))
					{
						display_message(ERROR_MESSAGE,
							"MANAGER_COPY_WITHOUT_IDENTIFIER(Cmiss_variable,name).  "
							"Type specific duplicate data function failed.");
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"MANAGER_COPY_WITHOUT_IDENTIFIER(Cmiss_variable,name).  "
						"Type specific data but no duplicate data function.");
					return_code=0;
				}
			}
			if (return_code)
			{
				/* free current type-specific data */
				Cmiss_variable_clear_type(destination);
				if (0<source->number_of_source_variables)
				{
					if (REALLOCATE(independent_variable_address,
						destination->independent_variables,Cmiss_variable_id ,
						source->number_of_source_variables))
					{
						destination->independent_variables=independent_variable_address;
						if (REALLOCATE(source_variable_address,
							destination->source_variables,Cmiss_variable_id ,
							source->number_of_source_variables))
						{
							destination->source_variables=source_variable_address;
						}
						else
						{
							return_code=0;
						}
					}
					else
					{
						return_code=0;
					}
				}
				if (return_code)
				{
					/* establish the new type */
					destination->type_string=source->type_string;
					destination->type_specific_data=type_specific_data;
					for (i=source->number_of_source_variables-1;i>0;i--)
					{
						(destination->source_variables)[i]=
							ACCESS(Cmiss_variable)((source->source_variables)[i]);
						(destination->independent_variables)[i]=
							ACCESS(Cmiss_variable)((source->independent_variables)[i]);
					}
					return_code=Cmiss_variable_establish_methods(destination,
						source->clear_type_specific_function,
						source->duplicate_data_type_specific_function,
						source->evaluate_derivative_type_specific_function,
						source->evaluate_type_specific_function,
						source->get_independent_variable_value_type_specific_function,
						source->get_set_independent_variable_value_type_specific_function,
						source->get_value_type_type_specific_function,
						source->is_defined_type_specific_function,
						source->is_independent_variable_of_type_specific_function,
						source->not_in_use_type_specific_function,
						source->overlap_type_specific_function,
						source->same_variable_type_specific_function,
						source->set_independent_variable_value_type_specific_function);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"MANAGER_COPY_WITHOUT_IDENTIFIER(Cmiss_variable,name).  "
						"Could not REALLOCATE source variable storage");
					return_code=0;
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_WITHOUT_IDENTIFIER(Cmiss_variable,name).  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITHOUT_IDENTIFIER(Cmiss_variable,name) */

PROTOTYPE_MANAGER_COPY_IDENTIFIER_FUNCTION(Cmiss_variable,name,char *)
{
	char *destination_name;
	int return_code;

	ENTER(MANAGER_COPY_IDENTIFIER(Cmiss_variable,name));
	return_code=0;
	if (name&&destination)
	{
		if (name)
		{
			if (ALLOCATE(destination_name,char,strlen(name)+1))
			{
				strcpy(destination_name,name);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"MANAGER_COPY_IDENTIFIER(Cmiss_variable,name).  "
					"Insufficient memory");
				return_code=0;
			}
		}
		else
		{
			name=(char *)NULL;
			return_code=1;
		}
		if (return_code)
		{
			/* copy name */
			DEALLOCATE(destination->name);
			destination->name=destination_name;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_IDENTIFIER(Cmiss_variable,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_IDENTIFIER(Cmiss_variable,name) */

DECLARE_MANAGER_FUNCTIONS(Cmiss_variable)

PROTOTYPE_MANAGED_OBJECT_NOT_IN_USE_FUNCTION(Cmiss_variable)
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Cmiss_variable requires a special version of this function because it may
use objects (eg FE_fields) that are also used.  If the computed variable is not
itself in use, it calls the variable's optional not_in_use function and bases
its result on that.

???DB.  What about source fields?
==============================================================================*/
{
	int return_code;

	ENTER(MANAGED_OBJECT_NOT_IN_USE(Cmiss_variable));
	return_code=0;
	if (manager&&object)
	{
		if (!(manager->locked))
		{
			if (IS_OBJECT_IN_LIST(Cmiss_variable)(object,manager->object_list))
			{
				if ((1==object->access_count)||
					((2==object->access_count)&&
					IS_OBJECT_IN_LIST(Cmiss_variable)(object,
					manager->message->changed_object_list)))
				{
					if (object->not_in_use_type_specific_function)
					{
						return_code=(object->not_in_use_type_specific_function)(object);
					}
					else
					{
						return_code=1;
					}
				}
			}
			else
			{
				display_message(WARNING_MESSAGE,
					"MANAGED_OBJECT_NOT_IN_USE(Cmiss_variable).  "
					"Object is not managed");
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,
				"MANAGED_OBJECT_NOT_IN_USE(Cmiss_variable).  Manager is locked");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGED_OBJECT_NOT_IN_USE(Cmiss_variable).  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* MANAGED_OBJECT_NOT_IN_USE(Cmiss_variable) */

DECLARE_ADD_OBJECT_TO_MANAGER_FUNCTION(Cmiss_variable,name)

DECLARE_MANAGER_MODIFY_FUNCTION(Cmiss_variable,name)
	/*???DB.  Don't need special version because don't have components */
DECLARE_MANAGER_MODIFY_NOT_IDENTIFIER_FUNCTION(Cmiss_variable,name)
	/*???DB.  Don't need special version because don't have components */

DECLARE_MANAGER_MODIFY_IDENTIFIER_FUNCTION(Cmiss_variable,name,char *)
DECLARE_FIND_BY_IDENTIFIER_IN_MANAGER_FUNCTION(Cmiss_variable,name,char *)

int Cmiss_variable_same_variable(Cmiss_variable_id variable_1,
	Cmiss_variable_id variable_2)
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Returns nonzero if <variable_1> and <variable_2> are the same variable (eg. the
value at node 10 for the finite element field bob) and zero otherwise.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_variable_same_variable);
	return_code=0;
	if (variable_1&&variable_2)
	{
		if (variable_1->same_variable_type_specific_function)
		{
			return_code=(variable_1->same_variable_type_specific_function)(variable_1,
				variable_2);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Cmiss_variable_same_variable.  "
				"Missing type specific function");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_variable_same_variable.  "
			"Invalid argument(s).  %p %p",variable_1,variable_2);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_same_variable */

int Cmiss_variable_overlap(Cmiss_variable_id variable_1,
	Cmiss_variable_id variable_2)
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Returns nonzero if <variable_1> and <variable_2> overlap (eg d/ds1 and all
values) and zero otherwise.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_variable_overlap);
	return_code=0;
	if (variable_1&&variable_2)
	{
		if (variable_1->overlap_type_specific_function)
		{
			return_code=(variable_1->overlap_type_specific_function)(variable_1,
				variable_2);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Cmiss_variable_overlap.  "
				"Missing type specific function");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_variable_overlap.  "
			"Invalid argument(s).  %p %p",variable_1,variable_2);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_overlap */

int Cmiss_variable_is_independent_variable_of(
	Cmiss_variable_id dependent_variable,
	Cmiss_variable_id independent_variable)
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Returns nonzero if <independent_variable> is an independent variable of
<dependent_variable> and zero otherwise.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_variable_is_independent_variable_of);
	return_code=0;
	if (dependent_variable&&independent_variable)
	{
		if (dependent_variable->is_independent_variable_of_type_specific_function)
		{
			return_code=
				(dependent_variable->is_independent_variable_of_type_specific_function)(
				dependent_variable,independent_variable);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_variable_is_independent_variable_of.  "
				"Missing type specific function");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_variable_is_independent_variable_of.  "
			"Invalid argument(s).  %p %p",dependent_variable,independent_variable);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_is_independent_variable_of */

char *Cmiss_variable_get_type_id_string(Cmiss_variable_id variable)
/*******************************************************************************
LAST MODIFIED : 7 February 2003

DESCRIPTION :
Returns the string which identifies the type.  The calling function must not
DEALLOCATE the returned string.
==============================================================================*/
{
	char *return_string;

	ENTER(Cmiss_variable_get_type_id_string);
	return_string=(char *)NULL;
	if (variable)
	{
		return_string=variable->type_string;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_variable_get_type_id_string.  "
			"Missing variable");
	}
	LEAVE;

	return (return_string);
} /* Cmiss_variable_get_type_id_string */

int Cmiss_variable_get_value_type(Cmiss_variable_id variable,
	Cmiss_value_id type)
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Sets the <type> to be the same as the type of the results of <variable>, but
does not set/calculate the actual value.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_variable_get_value_type);
	return_code=0;
	if (variable&&type)
	{
		if (variable->get_value_type_type_specific_function)
		{
			return_code=(variable->get_value_type_type_specific_function)(variable,
				type);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Cmiss_variable_get_value_type.  "
				"Missing type specific function");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_variable_get_value_type.  "
			"Invalid argument(s).  %p %p",variable,type);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_get_value_type */

int Cmiss_variable_same_value_type(Cmiss_variable_id variable_1,
	Cmiss_variable_id variable_2)
/*******************************************************************************
LAST MODIFIED : 13 February 2003

DESCRIPTION :
Returns nonzero if <variable_1> and <variable_2> have the same value type and
zero otherwise.
==============================================================================*/
{
	int return_code;
	Cmiss_value_id value_1,value_2;

	ENTER(Cmiss_variable_same_value_type);
	return_code=0;
	if (variable_1&&variable_2)
	{
		value_1=CREATE(Cmiss_value)();
		value_2=CREATE(Cmiss_value)();
		if (value_1&&value_2)
		{
			if (Cmiss_variable_get_value_type(variable_1,value_1)&&
				Cmiss_variable_get_value_type(variable_2,value_2))
			{
				return_code=Cmiss_value_same_sub_type(value_1,value_2);
			}
			else
			{
				display_message(ERROR_MESSAGE,"Cmiss_variable_same_value_type.  "
					"Could not get value types");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Cmiss_variable_same_value_type.  "
				"Could not create value types.  %p %p",value_1,value_2);
		}
		DESTROY(Cmiss_value)(&value_1);
		DESTROY(Cmiss_value)(&value_2);
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_variable_same_value_type.  "
			"Invalid argument(s).  %p %p",variable_1,variable_2);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_same_value_type */

int Cmiss_variable_set_independent_variable_source_variable(
	Cmiss_variable_id variable,
	Cmiss_variable_id independent_variable,
	Cmiss_variable_id source_variable)
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Sets the values for <independent_variable> of the <variable> to come from the
<source_variable>.  The types of the values for <independent_variable> and
<source_variable> must match.  If <source_variable> is NULL then a previous
setting of the source variable for <independent_variable> is cleared.
==============================================================================*/
{
	int i,j,overlapping_variables,return_code;
	Cmiss_variable_id *independent_variable_address,*source_variable_address;

	ENTER(Cmiss_variable_set_independent_variable_source_variable);
	return_code=0;
	if (variable&&independent_variable)
	{
		/* check that results of <independent_variable> and <source_variable>
			match */
		if (!source_variable||Cmiss_variable_same_value_type(
			independent_variable,source_variable))
		{
			/* pass on to source variables */
			return_code=1;
			i=variable->number_of_source_variables;
			j=0;
			overlapping_variables=0;
			independent_variable_address=variable->independent_variables;
			source_variable_address=variable->source_variables;
			while (return_code&&(i>0))
			{
				if (j>0)
				{
					*(independent_variable_address-j)= *independent_variable_address;
					*(source_variable_address-j)= *source_variable_address;
				}
				if (Cmiss_variable_same_variable(*independent_variable_address,
					independent_variable))
				{
					DEACCESS(Cmiss_variable)(independent_variable_address);
					DEACCESS(Cmiss_variable)(source_variable_address);
					j++;
				}
				else
				{
					if (!source_variable&&Cmiss_variable_overlap(
						*independent_variable_address,independent_variable))
					{
						overlapping_variables=1;
					}
					return_code=
						Cmiss_variable_set_independent_variable_source_variable(
						*source_variable_address,independent_variable,source_variable);
				}
				i--;
				source_variable_address++;
				independent_variable_address++;
			}
			variable->number_of_source_variables -= j;
			if (return_code)
			{
				/* add <source_variable> if it is an <independent_variable> of
					<variable> */
					/*???DB.  Immediate independent variable */
				if ((source_variable||overlapping_variables)&&
					Cmiss_variable_is_independent_variable_of(variable,
					independent_variable))
				{
					if (j<1)
					{
						if (REALLOCATE(source_variable_address,variable->source_variables,
							Cmiss_variable_id,(variable->number_of_source_variables)+1))
						{
							variable->source_variables=source_variable_address;
							if (REALLOCATE(independent_variable_address,
								variable->independent_variables,Cmiss_variable_id ,
								(variable->number_of_source_variables)+1))
							{
								variable->independent_variables=independent_variable_address;
							}
						}
					}
					else
					{
						independent_variable_address=variable->independent_variables;
						source_variable_address=variable->source_variables;
					}
					if (independent_variable_address&&source_variable_address)
					{
						independent_variable_address[variable->number_of_source_variables]=
							ACCESS(Cmiss_variable)(independent_variable);
						source_variable_address[variable->number_of_source_variables]=
							ACCESS(Cmiss_variable)(source_variable);
						(variable->number_of_source_variables)++;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Cmiss_variable_set_independent_variable_source_variable.  "
							"Insufficient memory for extending source/independent variable "
							"pairs");
					}
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_variable_set_independent_variable_source_variable.  "
				"Source and independent value type mis-match");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_variable_set_independent_variable_source_variable.  "
			"Invalid argument(s).  %p %p",variable,independent_variable);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_set_independent_variable_source_variable */

int Cmiss_variable_set_independent_variable_value(
	Cmiss_variable_id variable,
	Cmiss_variable_id independent_variable,
	Cmiss_value_id value)
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Sets the <value> of the <independent_variable> for the <variable>.
==============================================================================*/
{
	int i,return_code;
	Cmiss_variable_id *independent_variable_address;

	ENTER(Cmiss_variable_set_independent_variable_value);
	return_code=0;
	/* check arguments */
	if (variable&&independent_variable&&value)
	{
		if (variable->set_independent_variable_value_type_specific_function)
		{
			return_code=
				(variable->set_independent_variable_value_type_specific_function)(
				variable,independent_variable,value);
			i=variable->number_of_source_variables;
			independent_variable_address=variable->independent_variables;
			while (return_code&&(i>0))
			{
				return_code=Cmiss_variable_set_independent_variable_value(
					*independent_variable_address,independent_variable,value);
				independent_variable_address++;
				i--;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_variable_set_independent_variable_value.  "
				"Missing type specific function");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_variable_set_independent_variable_value.  "
			"Invalid argument(s).  %p %p %p",variable,independent_variable,value);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_set_independent_variable_value */

int Cmiss_variable_get_independent_variable_value(
	Cmiss_variable_id variable,
	Cmiss_variable_id independent_variable,
	Cmiss_value_id value)
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Gets the <value> of the <independent_variable> for the <variable>.  If the
<value> has no type, then the default type for the <independent_variable> is
used otherwise a representation in <value>'s type is attempted.
==============================================================================*/
{
	int i,return_code;
	Cmiss_variable_id *independent_variable_address;

	ENTER(Cmiss_variable_get_independent_variable_value);
	return_code=0;
	/* check arguments */
	if (variable&&independent_variable&&value)
	{
		if (variable->get_independent_variable_value_type_specific_function)
		{
			return_code=
				(variable->get_independent_variable_value_type_specific_function)(
				variable,independent_variable,value);
			i=variable->number_of_source_variables;
			independent_variable_address=variable->independent_variables;
			while ((!return_code)&&(i>0))
			{
				return_code=Cmiss_variable_get_independent_variable_value(
					*independent_variable_address,independent_variable,value);
				independent_variable_address++;
				i--;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_variable_get_independent_variable_value.  "
				"Missing type specific function");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_variable_get_independent_variable_value.  "
			"Invalid argument(s).  %p %p %p",variable,independent_variable,value);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_get_independent_variable_value */

int Cmiss_variable_get_set_independent_variable_value(
	Cmiss_variable_id variable,
	Cmiss_variable_id independent_variable,
	Cmiss_value_id value)
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Swaps the <value> and the current value for the <independent_variable> of the
<variable>.
==============================================================================*/
{
	int i,return_code;
	Cmiss_variable_id *independent_variable_address;

	ENTER(Cmiss_variable_get_set_independent_variable_value);
	return_code=0;
	/* check arguments */
	if (variable&&independent_variable&&value)
	{
		if (variable->get_set_independent_variable_value_type_specific_function)
		{
			return_code=
				(variable->get_set_independent_variable_value_type_specific_function)(
				variable,independent_variable,value);
			i=variable->number_of_source_variables;
			independent_variable_address=variable->independent_variables;
			while ((!return_code)&&(i>0))
			{
				return_code=Cmiss_variable_get_set_independent_variable_value(
					*independent_variable_address,independent_variable,value);
				independent_variable_address++;
				i--;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_variable_get_set_independent_variable_value.  "
				"Missing type specific function");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_variable_get_set_independent_variable_value.  "
			"Invalid argument(s).  %p %p %p",variable,independent_variable,value);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_get_set_independent_variable_value */

int Cmiss_variable_depends_on_Cmiss_variable(
	Cmiss_variable_id variable,Cmiss_variable_id other_variable)
/*******************************************************************************
LAST MODIFIED : 4 February 2002

DESCRIPTION :
Returns nonzero if the two variables are identical or if <other_variable> needs
to be evaluated in order to evaluate <variable>.

This function is used by MANAGER_COPY_WITHOUT_IDENTIFIER to ensure that the
destination variable will not depend on itself, thus leading to an infinite
loop, and also to prevent copying a variable over itself.

Parts of the program receiving manager messages for Cmiss_variables should
call this function with the variable=variable in use and
other_variable=modified variable to determine if the variable in use needs
updating.
==============================================================================*/
{
	int i,return_code;
	Cmiss_variable_id *source_variable_address;

	ENTER(Cmiss_variable_depends_on_Cmiss_variable);
	if (variable&&other_variable)
	{
		if (variable==other_variable)
		{
			return_code=1;
		}
		else
		{
			return_code=0;
			source_variable_address=variable->source_variables;
			i=variable->number_of_source_variables;
			while ((!return_code)&&(i>0))
			{
				return_code=Cmiss_variable_depends_on_Cmiss_variable(
					*source_variable_address,other_variable);
				i--;
				source_variable_address++;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_variable_depends_on_Cmiss_variable.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_depends_on_Cmiss_variable */

int Cmiss_variable_depends_on_Cmiss_variable_in_list(
	Cmiss_variable_id variable,
	struct LIST(Cmiss_variable) *variable_list)
/*******************************************************************************
LAST MODIFIED : 4 February 2003

DESCRIPTION :
Returns nonzero if <variable> depends on any variable in <variable_list>.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_variable_depends_on_Cmiss_variable_in_list);
	return_code=0;
	if (variable && variable_list)
	{
		if (FIRST_OBJECT_IN_LIST_THAT(Cmiss_variable)(
			Cmiss_variable_is_source_variable_conditional,
			(void *)variable,variable_list))
		{
			return_code=1;
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_variable_depends_on_Cmiss_variable_in_list.  "
			"Invalid argument(s).  %p %p",variable,variable_list);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_depends_on_Cmiss_variable_in_list */

struct Cmiss_variable_value *CREATE(Cmiss_variable_value)(
	Cmiss_variable_id variable,Cmiss_value_id value)
/*******************************************************************************
LAST MODIFIED : 22 April 2003

DESCRIPTION :
Creates a <variable>/<value> pair.
==============================================================================*/
{
	struct Cmiss_variable_value *variable_value;

	ENTER(CREATE(Cmiss_variable_value));
	if (ALLOCATE(variable_value,struct Cmiss_variable_value,1))
	{
		variable_value->variable=ACCESS(Cmiss_variable)(variable);
		variable_value->value=ACCESS(Cmiss_value)(value);
		variable_value->access_count=0;
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Cmiss_variable_value).  "
			"Insufficient memory");
	}
	LEAVE;

	return (variable_value);
} /* CREATE(Cmiss_variable_value) */

int DESTROY(Cmiss_variable_value)(
	struct Cmiss_variable_value **variable_value_address)
/*******************************************************************************
LAST MODIFIED : 22 April 2003

DESCRIPTION :
Frees memory/deaccess objects for Cmiss_variable_value at
<*variable_value_address>.
==============================================================================*/
{
	int return_code;

	ENTER(DESTROY(Cmiss_variable_value));
	return_code=0;
	if (variable_value_address&&(*variable_value_address))
	{
		if ((*variable_value_address)->access_count<=0)
		{
			DEACCESS(Cmiss_variable)(&((*variable_value_address)->variable));
			DEACCESS(Cmiss_value)(&((*variable_value_address)->value));
			DEALLOCATE(*variable_value_address);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,"DESTROY(Cmiss_variable_value).  "
				"Positive access_count");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(Cmiss_variable_value).  "
			"Missing variable_value");
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Cmiss_variable_value) */

DECLARE_OBJECT_FUNCTIONS(Cmiss_variable_value)

DECLARE_LIST_FUNCTIONS(Cmiss_variable_value)

Cmiss_value_id Cmiss_variable_value_get_value(
	struct Cmiss_variable_value *variable_value)
/*******************************************************************************
LAST MODIFIED : 7 February 2003

DESCRIPTION :
Returns the value for the <variable_value>.
==============================================================================*/
{
	Cmiss_value_id value;

	ENTER(Cmiss_variable_value_get_value);
	value=(Cmiss_value_id)NULL;
	/* check arguments */
	if (variable_value)
	{
		value=variable_value->value;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_variable_value_get_value.  "
			"Missing variable_value");
	}
	LEAVE;

	return (value);
} /* Cmiss_variable_value_get_value */

Cmiss_variable_id Cmiss_variable_value_get_variable(
	struct Cmiss_variable_value *variable_value)
/*******************************************************************************
LAST MODIFIED : 7 February 2003

DESCRIPTION :
Returns the variable for the <variable_value>.
==============================================================================*/
{
	Cmiss_variable_id variable;

	ENTER(Cmiss_variable_value_get_variable);
	variable=(Cmiss_variable_id )NULL;
	/* check arguments */
	if (variable_value)
	{
		variable=variable_value->variable;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_variable_value_get_variable.  "
			"Missing variable_value");
	}
	LEAVE;

	return (variable);
} /* Cmiss_variable_value_get_variable */

int Cmiss_variable_is_defined(Cmiss_variable_id variable,
	struct LIST(Cmiss_variable_value) *values)
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Returns true if <variable> can be calculated with the specified <values>
over-riding, but not setting, the current values.

???DB.  Could use evaluate, but that would be less efficient and currently
	would write error messages if it failed
==============================================================================*/
{
	int return_code;
#if defined (OLD_CODE)
	int i,return_code;
	Cmiss_variable_id *source_variable_address;
#endif /* defined (OLD_CODE) */

	ENTER(Cmiss_variable_is_defined);
	return_code=0;
	/* check arguments */
	if (variable&&values)
	{
		if (variable->is_defined_type_specific_function)
		{
			/* swap the specified <values> with the current values */
			if (FOR_EACH_OBJECT_IN_LIST(Cmiss_variable_value)(
				Cmiss_variable_get_set_independent_variable_value_iterator,
				(void *)variable,values))
			{
				/* is defined of source variables has to be type specific because
					different types may evaluate in different ways eg derivative type */
					/*???DB.  No?  The derivative is being evaluated for a certain choice
						of independent variable values which aren't differentiated */
				return_code=(variable->is_defined_type_specific_function)(variable);
#if defined (OLD_CODE)
				return_code=1;
				source_variable_address=variable->source_variables;
				i=variable->number_of_source_variables;
				while (return_code&&(i>0))
				{
					return_code=Cmiss_variable_is_defined(*source_variable_address,
						values);
					i--;
					source_variable_address++;
				}
				if (return_code)
				{
					return_code=(variable->is_defined_type_specific_function)(variable);
				}
#endif /* defined (OLD_CODE) */
				if (!FOR_EACH_OBJECT_IN_LIST(Cmiss_variable_value)(
					Cmiss_variable_get_set_independent_variable_value_iterator,
					(void *)variable,values))
				{
					display_message(ERROR_MESSAGE,"Cmiss_variable_is_defined.  "
						"Could not reset <values>");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"Cmiss_variable_is_defined.  "
					"Could not over-ride <values>");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Cmiss_variable_is_defined.  "
				"Missing type specific function");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_variable_is_defined.  "
			"Invalid argument(s).  %p %p",variable,values);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_is_defined */

int Cmiss_variable_evaluate(struct Cmiss_variable_value *variable_value,
	struct LIST(Cmiss_variable_value) *values)
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Calculates the <variable_value> with the specified <values> over-riding, but not
setting, the current values.

???DB.  Evaluate source_variables and also set?
==============================================================================*/
{
	int return_code;
	Cmiss_variable_id variable;

	ENTER(Cmiss_variable_evaluate);
	return_code=0;
	/* checking arguments */
	if (variable_value&&(variable=variable_value->variable))
	{
		if (variable->evaluate_type_specific_function)
		{
			/* swap the specified <values> with the current values */
			if (FOR_EACH_OBJECT_IN_LIST(Cmiss_variable_value)(
				Cmiss_variable_get_set_independent_variable_value_iterator,
				(void *)variable,values))
			{
				return_code=(variable->evaluate_type_specific_function)(variable_value);
				if (!FOR_EACH_OBJECT_IN_LIST(Cmiss_variable_value)(
					Cmiss_variable_get_set_independent_variable_value_iterator,
					(void *)variable,values))
				{
					display_message(ERROR_MESSAGE,"Cmiss_variable_evaluate.  "
						"Could not reset <values>");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"Cmiss_variable_evaluate.  "
					"Could not over-ride <values>");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Cmiss_variable_evaluate.  "
				"Missing type specific function");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_variable_evaluate.  "
			"Missing <variable_value> to evaluate");
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_evaluate */

int Cmiss_variable_evaluate_derivative(
	Cmiss_variable_id dependent_variable,int order,
	Cmiss_variable_id *independent_variables,
	struct LIST(Cmiss_variable_value) *values,
	Cmiss_value_id derivative_matrix)
/*******************************************************************************
LAST MODIFIED : 27 April 2003

DESCRIPTION :
Evaluates the <derivative_matrix> for the <order> degree derivative of
<dependent_variable> with respect to the <independent_variables>.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_variable_evaluate_derivative);
	return_code=0;
	/* checking arguments */
	if (dependent_variable&&(0<order)&&independent_variables&&derivative_matrix)
	{
		if (dependent_variable->evaluate_derivative_type_specific_function)
		{
			/* swap the specified <values> with the current values */
			if (!values||FOR_EACH_OBJECT_IN_LIST(Cmiss_variable_value)(
				Cmiss_variable_get_set_independent_variable_value_iterator,
				(void *)dependent_variable,values))
			{
				return_code=
					(dependent_variable->evaluate_derivative_type_specific_function)(
					dependent_variable,order,independent_variables,derivative_matrix);
				if (values&&!FOR_EACH_OBJECT_IN_LIST(Cmiss_variable_value)(
					Cmiss_variable_get_set_independent_variable_value_iterator,
					(void *)dependent_variable,values))
				{
					display_message(ERROR_MESSAGE,
						"Cmiss_variable_evaluate_derivative.  "
						"Could not reset <values>");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"Cmiss_variable_evaluate_derivative.  "
					"Could not over-ride <values>");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Cmiss_variable_evaluate_derivative.  "
				"Missing type specific function");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_variable_evaluate_derivative.  "
			"Invalid argument(s).  %p %d %p %p",dependent_variable,order,
			independent_variables,derivative_matrix);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_evaluate_derivative */

int Cmiss_variable_derivative_set_type(
	Cmiss_variable_id derivative,
	Cmiss_variable_id dependent_variable,int order,
	Cmiss_variable_id *independent_variables)
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Sets <derivative> to be the derivative of the <dependent_variable> with respect
to the <independent_variables>.  This function ACCESSes the <dependent_variable>
and <independent_variables>.  After success, the <derivative> is responsible for
DEACCESS/DEALLOCATEing <dependent_variable> and <independent_variables>.
==============================================================================*/
{
	int i,return_code;
	struct Cmiss_variable_derivative_type_specific_data *data;

	ENTER(Cmiss_variable_derivative_set_type);
	return_code=0;
	/* check arguments */
	if (derivative&&dependent_variable&&(0<order)&&independent_variables)
	{
		i=0;
		while ((i<order)&&independent_variables[i])
		{
			i++;
		}
		if (i<=order)
		{
			/* 1.  Make dynamic allocations for any new type-specific data */
			if (ALLOCATE(data,struct Cmiss_variable_derivative_type_specific_data,
				1))
			{
				/* 2.  Clear current type-specific data */
				Cmiss_variable_clear_type(derivative);
				/* 3.  Establish the new type */
				derivative->type_string=Cmiss_variable_derivative_type_string;
				derivative->type_specific_data=(void *)data;
				data->order=order;
				data->variable=ACCESS(Cmiss_variable)(dependent_variable);
				data->independent_variables=independent_variables;
				for (i=0;i<order;i++)
				{
					ACCESS(Cmiss_variable)(independent_variables[i]);
				}
				/* set all the methods */
				return_code=CMISS_VARIABLE_ESTABLISH_METHODS(derivative,derivative);
			}
			else
			{
				display_message(ERROR_MESSAGE,"Cmiss_variable_derivative_set_type.  "
					"Could not ALLOCATE type specific data");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_variable_derivative_set_type.  "
				"Missing independent variable(s)");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_variable_derivative_set_type.  "
			"Invalid argument(s).  %p %p %d %p\n",derivative,dependent_variable,
			order,independent_variables);
	}
	LEAVE;

	return (return_code);
} /* Cmiss_variable_derivative_set_type */

int Cmiss_variable_divergence_set_type(Cmiss_variable_id divergence,
	Cmiss_variable_id dependent_variable,Cmiss_variable_id independent_variable);
/*******************************************************************************
LAST MODIFIED : 7 March 2003

DESCRIPTION :
Sets <divergence> to be the divergence of the <dependent_variable> with respect
to the <independent_variable>.
==============================================================================*/
/*???DB.  To be done */

int Cmiss_variable_inverse_set_type(Cmiss_variable_id inverse_variable,
	Cmiss_variable_id variable,struct LIST(Cmiss_variable) *dependent_variables);
/*******************************************************************************
LAST MODIFIED : 24 January 2003

DESCRIPTION :
Sets <inverse_variable> to be the inverse of the <variable>.  Its independent
variables are the dependent variables of the <variable> and its
<dependent_variables> are independent variables of the <variable>.
==============================================================================*/
/*???DB.  To be done */

struct Cmiss_variable_package *CREATE(Cmiss_variable_package)(void)
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Creates a Cmiss_variable_package which is used for sharing of
Cmiss_variables with the same name.

???DB.  What about FE_ managers?  Do FE computed variables need there own
	package (part of set_type_ ?)
==============================================================================*/
{
	struct Cmiss_variable_package *package;

	ENTER(CREATE(Cmiss_variable_package));
	package=(struct Cmiss_variable_package *)NULL;
	if (ALLOCATE(package,struct Cmiss_variable_package,1))
	{
		if (!(package->variable_manager=CREATE_MANAGER(Cmiss_variable)()))
		{
			display_message(ERROR_MESSAGE,"CREATE(Cmiss_variable_package).  "
				"Could not create variable_manager");
			DEALLOCATE(package);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Cmiss_variable_package).  "
			"Could not allocate package");
	}
	LEAVE;

	return (package);
} /* CREATE(Cmiss_variable_package) */

int DESTROY(Cmiss_variable_package)(
	struct Cmiss_variable_package **package_address)
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Frees memory/deaccess objects in Cmiss_variable_package at <*package_address>.
==============================================================================*/
{
	int return_code;

	ENTER(DESTROY(Cmiss_variable_package));
	return_code=0;
	if (package_address)
	{
		if (*package_address)
		{
			DESTROY_MANAGER(Cmiss_variable)(&((*package_address)->variable_manager));
			DEALLOCATE(*package_address);
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(Cmiss_variable_package).  "
			"Could not allocate package");
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Cmiss_variable_package) */
