/*******************************************************************************
FILE : computed_value_derivative_matrix.c

LAST MODIFIED : 24 July 2003

DESCRIPTION :
Implements the derivative matrix computed value.
==============================================================================*/
#include "computed_variable/computed_value_derivative_matrix.h"
#include "computed_variable/computed_value_matrix.h"
#include "computed_variable/computed_value_private.h"
#include "general/debug.h"
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

static START_CMISS_VALUE_GET_REALS_TYPE_SPECIFIC_FUNCTION(derivative_matrix)
{
	Cmiss_value_id *matrix;
	FE_value *destination_real,*destination_reals;
	int column_number,i,number_of_columns,number_of_matrices,number_of_reals,
		number_of_rows,row_number;
	Matrix_value matrix_value;

	if (matrix=data->matrices)
	{
		number_of_matrices=1;
		for (i=0;i<data->order;i++)
		{
			number_of_matrices *= 2;
		}
		number_of_matrices -= 1;
		i=number_of_matrices;
		number_of_reals=0;
		while ((i>0)&&(return_code=Cmiss_value_matrix_get_dimensions(*matrix,
			&number_of_rows,&number_of_columns)))
		{
			number_of_reals += number_of_rows*number_of_columns;
			matrix++;
		}
		if (return_code)
		{
			if (!reals_address||ALLOCATE(destination_reals,FE_value,number_of_reals))
			{
				if (reals_address)
				{
					destination_real=destination_reals;
					i=number_of_matrices;
					matrix=data->matrices;
					while ((i>0)&&(return_code=Cmiss_value_matrix_get_dimensions(*matrix,
						&number_of_rows,&number_of_columns)))
					{
						row_number=1;
						while (return_code&&(row_number<=number_of_rows))
						{
							column_number=1;
							while ((column_number<=number_of_columns)&&(return_code=
								Cmiss_value_matrix_get_value(*matrix,row_number,column_number,
								&matrix_value)))
							{
								*destination_real=(FE_value)matrix_value;
								destination_real++;
								column_number++;
							}
							row_number++;
						}
						matrix++;
					}
					if (return_code)
					{
						*reals_address=destination_reals;
					}
					else
					{
						DEALLOCATE(destination_real);
					}
				}
				if (return_code)
				{
					*number_of_reals_address=number_of_reals;
				}
			}
			else
			{
				return_code=0;
			}
		}
	}
}
END_CMISS_VALUE_GET_REALS_TYPE_SPECIFIC_FUNCTION(derivative_matrix)

static START_CMISS_VALUE_MULTIPLY_AND_ACCUMULATE_TYPE_SPECIFIC_FUNCTION(
	derivative_matrix)
/*******************************************************************************
LAST MODIFIED : 24 July 2003

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
		*sub_order_2,temp_int;
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
			if ((order==data_2->order)&&(order==data_total->order)&&
				numbers_of_independent_values&&index_1&&index_2&&mapping_total&&
				mapping_2&&order_2&&sub_order_2&&not_used&&product_orders&&
				column_numbers_2&&matrices_2&&value_type&&
				Cmiss_variable_get_value_type(data_1->dependent_variable,value_type))
			{
				if (return_code=Cmiss_value_FE_value_vector_get_type(value_type,
					&number_of_rows,(FE_value **)NULL)&&(return_code=
					Cmiss_variable_get_value_type(data_2->dependent_variable,value_type)))
				{
					if (return_code=Cmiss_value_FE_value_vector_get_type(value_type,
						&number_of_intermediate_values,(FE_value **)NULL)&&(return_code=
						Cmiss_variable_get_value_type(data_total->dependent_variable,
						value_type)))
					{
						if (return_code=Cmiss_value_FE_value_vector_get_type(value_type,
							&temp_int,(FE_value **)NULL))
						{
							if (number_of_rows!=temp_int)
							{
								display_message(ERROR_MESSAGE,
									"Cmiss_value_derivative_matrix_multiply_and_accumulate_type_specific.  "
									"Dependent variables mis-match");
								return_code=0;
							}
						}
					}
				}
				i=0;
				number_of_matrices=1;
				while (return_code&&(i<order))
				{
					if (
#if defined (OLD_CODE)
						/* data_2->dependent_variable will be the "independent variable" and
							(data_1->independent_variables)[i] will be the "source variable"
							and these don't have to be the same */
						(return_code=Cmiss_variable_same_variable(
						data_2->dependent_variable,(data_1->independent_variables)[i]))&&
#else /* defined (OLD_CODE) */
						/* instead make sure that the number of values match */
						(return_code=Cmiss_variable_get_value_type(
						(data_1->independent_variables)[i],value_type))&&
						(return_code=Cmiss_value_FE_value_vector_get_type(
						value_type,&temp_int,(FE_value **)NULL))&&
						(temp_int==number_of_intermediate_values)&&
#endif /* defined (OLD_CODE) */
						(return_code=Cmiss_variable_same_variable(
						(data_total->independent_variables)[i],
						(data_2->independent_variables)[i]))&&
						(return_code=Cmiss_variable_get_value_type(
						(data_total->independent_variables)[i],value_type)))
					{
#if defined (OLD_CODE)
						/*???DB.  This is happening too much - has to know about other
							types */
						if (CMISS_VALUE_IS_TYPE(FE_value)(value_type))
						{
							numbers_of_independent_values[i]=1;
						}
						else if (CMISS_VALUE_IS_TYPE(FE_value_vector)(value_type))
						{
							return_code=Cmiss_value_FE_value_vector_get_type(value_type,
								numbers_of_independent_values+i,(FE_value **)NULL);
						}
						else if (CMISS_VALUE_IS_TYPE(element_xi)(value_type))
						{
							return_code=Cmiss_value_element_xi_get_type(value_type,
								numbers_of_independent_values+i,(struct FE_element **)NULL,
								(FE_value **)NULL);
						}
						else
						{
							return_code=0;
						}
#endif /* defined (OLD_CODE) */
						return_code=Cmiss_value_get_reals(value_type,
							numbers_of_independent_values+i,(FE_value **)NULL);
						if (numbers_of_independent_values[i]<=0)
						{
							return_code=0;
						}
					}
					number_of_matrices *= 2;
					i++;
				}
				number_of_matrices -= 1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_value_derivative_matrix_multiply_and_accumulate_type_specific.  "
					"Variables mis-match or could not allocate working storage.  "
					"%d, %d, %d, %p, %p, %p, %p, %p, %p, %p, %p, %p, %p, %p, %p",
					order,data_2->order,data_total->order,numbers_of_independent_values,
					index_1,index_2,mapping_total,mapping_2,order_2,sub_order_2,not_used,
					product_orders,column_numbers_2,matrices_2,value_type);
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
							while (return_code&&(j<order_total))
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
										matrix_2--;
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
														matrices_2[l],index_1[l]+1,column_numbers_2[l],
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
								} while (return_code&&(l>0));
								offset_1 *= 2;
								matrix_1 += offset_1;
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

/*
Global functions
----------------
*/
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
