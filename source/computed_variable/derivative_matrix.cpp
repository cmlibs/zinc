//******************************************************************************
// FILE : derivative_matrix.cpp
//
// LAST MODIFIED : 11 January 2005
//
// DESCRIPTION :
//==============================================================================

// to use lapack with ublas
#include <boost/numeric/bindings/traits/ublas_matrix.hpp>
#include <boost/numeric/bindings/traits/std_vector.hpp>
#include <boost/numeric/bindings/lapack/gesv.hpp>

namespace lapack = boost::numeric::bindings::lapack;

#include "computed_variable/derivative_matrix.hpp"

// global classes
// ==============

// class Derivative_matrix
// -----------------------

Derivative_matrix::Derivative_matrix():std::list<Matrix>(),
	number_of_dependent_values(0),order(0),numbers_of_independent_values()
//******************************************************************************
// LAST MODIFIED : 23 December 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
}

Derivative_matrix::Derivative_matrix(const std::list<Matrix>& matrices):
	std::list<Matrix>(matrices)
//******************************************************************************
// LAST MODIFIED : 11 January 2005
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	bool valid;
	Function_size_type i;

	// check that consistent
	valid=false;
	i=matrices.size()+1;
	order=0;
	while ((i>1)&&(0==i%2))
	{
		i /= 2;
		order++;
	}
	if ((1==i)&&(0<order))
	{
		bool *independent_variables;
		Function_size_type j,number_of_independent_values;
		std::list<Matrix>::const_iterator iterator;

		numbers_of_independent_values.resize(order);
		i=0;
		independent_variables=new bool[order+1];
		valid=(0!=independent_variables);
		iterator=matrices.begin();
		number_of_dependent_values=iterator->size1();
		while (valid&&(i<order))
		{
			numbers_of_independent_values[i]=iterator->size2();
			valid=(iterator->size1()==number_of_dependent_values);
			iterator++;
			for (j=0;j<i;j++)
			{
				independent_variables[j]=false;
			}
			independent_variables[i]=true;
			independent_variables[0]= !independent_variables[0];
			while (valid&&independent_variables[i])
			{
				number_of_independent_values=1;
				for (j=0;j<=i;j++)
				{
					if (independent_variables[j])
					{
						number_of_independent_values *= numbers_of_independent_values[j];
					}
				}
				valid=((iterator->size1()==number_of_dependent_values)&&
					(iterator->size2()==number_of_independent_values));
				iterator++;
				j=0;
				while ((j<order)&&independent_variables[j])
				{
					independent_variables[j]=false;
					j++;
				}
				independent_variables[j]=true;
			}
			i++;
		}
		delete [] independent_variables;
	}
	if (!valid)
	{
		throw Derivative_matrix::Construction_exception();
	}
}

Derivative_matrix::Derivative_matrix(
	const Derivative_matrix& derivative_matrix):
	std::list<Matrix>(derivative_matrix),
	number_of_dependent_values(derivative_matrix.number_of_dependent_values),
	order(derivative_matrix.order),
	numbers_of_independent_values(derivative_matrix.numbers_of_independent_values)
//******************************************************************************
// LAST MODIFIED : 23 December 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{}

#if defined (OLD_CODE)
//???DB.  Doesn't inherit.  Best to use the default memberwise
Derivative_matrix& Derivative_matrix::operator=(
	const Derivative_matrix& derivative_matrix)
//******************************************************************************
// LAST MODIFIED : 23 December 2004
//
// DESCRIPTION :
// Assignment operator.
//==============================================================================
{
	number_of_dependent_values=derivative_matrix.number_of_dependent_values;
	order=derivative_matrix.order;
	numbers_of_independent_values=derivative_matrix.numbers_of_independent_values;

	return (*this);
}
#endif // defined (OLD_CODE)

Derivative_matrix::~Derivative_matrix()
//******************************************************************************
// LAST MODIFIED : 22 December 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{}

Derivative_matrix Derivative_matrix::operator*(
	const Derivative_matrix& derivative_g) const
//******************************************************************************
// LAST MODIFIED : 23 December 2004
//
// DESCRIPTION :
// This function implements the chain rule for differentiation.
//
// If
//   Y=G(X), H(X)=F(G(X))
// then
//   dH/dXi=
//     sum{p=1,m:dF/dYp*dGp/dXi}
//   d2H/dXidXj=
//     sum{p=1,m:sum{q=1,m:d2F/dYpdYq*dGp/dXi*dGq/dXj}}+
//     sum{p=1,m:dF/dYp*d2Gp/dXidXj}
//   d3H/dXidXjdXk=
//     sum{p=1,m:sum{q=1,m:sum{r=1,m:d3F/dYpdYqdYr*dGp/dXi*dGq/dXj*dGr/dXk}}}+
//     sum{p=1,m:sum{q=1,m:d2F/dYpdYq*
//       [d2Gp/dXidXj*dGq/dXk+d2Gp/dXidXk*dGq/dXj+dGp/dXi*d2Gq/dXjdXk]}}+
//     sum{p=1,m:dF/dYp*d3Gp/dXidXjdXk}
//   d4H/dXidXjdXkdXl=
//     sum{p=1,m:sum{q=1,m:sum{r=1,m:sum{s=1,m:
//       d4F/dYpdYqdYrdYs*dGp/dXi*dGq/dXj*dGr/dXk*dGr/dXl}}}}+
//     sum{p=1,m:sum{q=1,m:sum{r=1,m:d3F/dYpdYqdYr*
//       [d2Gp/dXidXj*dGq/dXk*dGr/dXl+
//       d2Gp/dXidXk*dGq/dXj*dGr/dXl+
//       d2Gp/dXidXl*dGq/dXj*dGr/dXk+
//       dGp/dXi*d2Gq/dXjdXk*dGr/dXl+
//       dGp/dXi*d2Gq/dXjdXl*dGr/dXk+
//       dGp/dXi*dGq/dXj*d2Gr/dXkdXl]}}}+
//     sum{p=1,m:sum{q=1,m:d2F/dYpdYq*
//       [d3Gp/dXidXjdXk*dGq/dXl+
//       d3Gp/dXidXjdXl*dGq/dXk+
//       d3Gp/dXidXkdXl*dGq/dXj+
//       dGp/dXi*d3Gq/dXjdXkdXl+
//       d2Gp/dXidXj*d2Gq/dXkdXl+
//       d2Gp/dXidXk*d2Gq/dXjdXl+
//       d2Gp/dXidXl*d2Gq/dXjdXk]}}+
//     sum{p=1,m:dF/dYp*d4Gp/dXidXjdXkdXl}
//   ...
// where m=length(Y).
//
// There are some parts above which don't look symmetric eg the p's and q's in
//   [d2Gp/dXidXj*dGq/dXk+d2Gp/dXidXk*dGq/dXj+dGp/dXi*d2Gq/dXjdXk]
// However for "reasonable" functions the order of differentiation does not
// change the result.  Assuming the functions are "reasonable" gives
//   d2F/dYpdYq=d2F/dYqdYp
// and so
//   [d2Gp/dXidXj*dGq/dXk+d2Gp/dXidXk*dGq/dXj+dGp/dXi*d2Gq/dXjdXk]
// can be replaced by
//   [d2Gp/dXidXj*dGq/dXk+d2Gp/dXidXk*dGq/dXj+d2Gp/dXjdXk*dGq/dXi]
// even though they are not equal because
//   [d2Gp/dXidXj*dGq/dXk+d2Gp/dXidXk*dGq/dXj+d2Gp/dXjdXk*dGq/dXi]+
//   [d2Gq/dXidXj*dGp/dXk+d2Gq/dXidXk*dGp/dXj+d2Gq/dXjdXk*dGp/dXi]
//   =
//   [d2Gp/dXidXj*dGq/dXk+d2Gp/dXidXk*dGq/dXj+dGp/dXi*d2Gq/dXjdXk]+
//   [d2Gq/dXidXj*dGp/dXk+d2Gq/dXidXk*dGp/dXj+dGq/dXi*d2Gp/dXjdXk]
//
// To calculate a derivative of H, the derivatives of all orders of F and G are
// needed and so derivatives of all orders are stored in the derivative matrices
// in the following order
//   d/dX,d/dY,d2/dXdY,d/dZ,d2/dXdZ,d2/dYdZ,d3/dXdYdZ
// NB.  This assumes that order of differentiation does not change the result
//
// In this function, f is the <dependent_variable> and g is the source
// variables for the composition variable.
//==============================================================================
{
	bool valid;
	Function_size_type i;
	std::list<Matrix> matrices_result(0);

	// check compatability
	valid=false;
	if (this&&(0<order)&&(order==derivative_g.order))
	{
		i=0;
		while ((i<order)&&(valid=(numbers_of_independent_values[i]==
			derivative_g.number_of_dependent_values)))
		{
			i++;
		}
	}
	if (valid)
	{
		bool found;
		bool *not_used;
		Scalar product,sum;
		std::list<Matrix>::const_iterator matrix_f,matrix_g;
		std::list<Matrix>::iterator matrix_result;
		Function_size_type column_number_result,index_result,j,k,l,
			number_of_columns_f,number_of_columns_result,
			number_of_intermediate_values,number_of_matrices,number_of_rows,offset_f,
			offset_g,order_result,p,q,r,row_number,s;
		Function_size_type *column_numbers_g,*index_f,*index_g,*mapping_g,
			*mapping_result,*numbers_of_independent_values,*product_orders,*order_g,
			*sub_order_g;
		std::list<Matrix>::const_iterator *matrices_g;

		// initialize
		number_of_rows=number_of_dependent_values;
		number_of_intermediate_values=derivative_g.number_of_dependent_values;
		not_used=new bool[order+1];
		column_numbers_g=new Function_size_type[order+1];
		index_f=new Function_size_type[order+1];
		index_g=new Function_size_type[order+1];
		mapping_g=new Function_size_type[order+1];
		mapping_result=new Function_size_type[order+1];
		numbers_of_independent_values=new Function_size_type[order];
		product_orders=new Function_size_type[order+1];
		order_g=new Function_size_type[order+1];
		sub_order_g=new Function_size_type[order+1];
		matrices_g=new std::list<Matrix>::const_iterator[order+1];
		if (not_used&&column_numbers_g&&index_f&&index_g&&mapping_g&&
			mapping_result&&numbers_of_independent_values&&product_orders&&order_g&&
			sub_order_g&&matrices_g)
		{
			number_of_matrices=1;
			matrix_g=derivative_g.begin();
			i=0;
			while (valid&&(i<order))
			{
				if (number_of_matrices<=derivative_g.size())
				{
					numbers_of_independent_values[i]=matrix_g->size2();
					for (j=number_of_matrices;j>0;j--)
					{
						Matrix new_matrix(number_of_rows,matrix_g->size2());

						matrices_result.push_back(new_matrix);
						matrix_g++;
					}
					number_of_matrices *= 2;
					i++;
				}
				else
				{
					valid=false;
				}
			}
			number_of_matrices -= 1;
			if ((number_of_matrices==size())&&
				(number_of_matrices==derivative_g.size()))
			{
				/* loop over dependent values (rows) */
				row_number=0;
				while (row_number<number_of_rows)
				{
					/* loop over derivatives (matrices) */
					matrix_result=matrices_result.begin();
					// index_result stores which derivative (matrix) of result (H) is
					//   being calculated
					index_result=1;
					while (index_result<=number_of_matrices)
					{
						// determine the derivative being evaluated and calculate its order
						//   and number of values
						order_result=0;
						number_of_columns_result=1;
						j=index_result;
						i=0;
						while (j>0)
						{
							if (j%2)
							{
								index_g[order_result]=0;
								mapping_result[order_result]=i;
								order_result++;
								number_of_columns_result *= numbers_of_independent_values[i];
							}
							j /= 2;
							i++;
						}
						// calculate the values for the derivative
						column_number_result=0;
						while (column_number_result<number_of_columns_result)
						{
							// loop over the sums for different order derivatives of f
							sum=(Scalar)0;
							matrix_f=begin();
							offset_f=1;
							j=0;
							while (j<order_result)
							{
								// initialize the orders for the derivatives of g that are
								//   multiplied together.  There are j+1 derivatives of g and
								//   their orders have to sum to the order of the derivative of
								//   result (order_result)
								for (l=0;l<j;l++)
								{
									product_orders[l]=1;
								}
								product_orders[j]=order_result-j;
								// loop over the possible ways of dividing the order_result
								//   independent variables, in mapping_result, into j+1
								//   non-empty sets, where the order of the sets and the order
								//   within the sets are not important.  For each possible way,
								//   loop across the row of matrix_f and down the columns of
								//   matrix_g and
								//   - calculate the product of the derivatives of g represented
								//     by the sets
								//   - multiply the product by the corresponding derivative of f
								//   - add the result to result
								do
								{
									// initialize the variable assigment
									// for each of the independent variables being differentiated
									//   with respect to in the product have:
									//   mapping_g - a reordering of the variables without
									//     changing the orders of the partial derivatives.  It is
									//     a location in order_g and sub_order_g
									//   order_g - the order of the partial derivative they're in
									//   sub_order_g - their position in the variables in partial
									//     derivatives of the same order
									r=0;
									l=0;
									while (l<=j)
									{
										q=0;
										do
										{
											for (p=0;p<product_orders[l];p++)
											{
												mapping_g[r]=r;
												order_g[r]=product_orders[l];
												sub_order_g[r]=q;
												r++;
												q++;
											}
											l++;
										} while ((l<=j)&&(product_orders[l]==product_orders[l-1]));
									}
									// find the column numbers of matrix g for the partial
									//   derivatives in the product
									// r is the number of the partial derivative within partial
									//   derivatives of the same order
									r=0;
									for (l=0;l<=j;l++)
									{
										// initialize the value position within the partial
										//   derivative of f
										index_f[l]=0;
										// determine which independent variables are used in the
										//   partial derivative of g
										matrix_g=derivative_g.begin();
										offset_g=1;
										for (p=0;p<order_result;p++)
										{
											q=mapping_g[p];
											// is the same partial derivative within the partial
											//   derivatives of the same order
											if ((product_orders[j]==order_g[q])&&
												(r==sub_order_g[q]/order_g[q]))
											{
												not_used[p]=false;
											}
											else
											{
												not_used[p]=true;
											}
											//???DB.  matrix_g += offset_g;
											for (s=offset_g;s>0;s--)
											{
												matrix_g++;
											}
											offset_g *= 2;
										}
										matrix_g--;
										for (p=order_result;p>0;p--)
										{
											offset_g /= 2;
											if (not_used[p-1])
											{
												//???DB.  matrix_g -= offset_g;
												for (s=offset_g;s>0;s--)
												{
													matrix_g--;
												}
											}
										}
										matrices_g[l]=matrix_g;
										// second the index of the value
										column_numbers_g[l]=0;
										for (p=0;p<order_result;p++)
										{
											if (!not_used[p])
											{
												column_numbers_g[l]=numbers_of_independent_values[
													mapping_result[p]]*column_numbers_g[l]+index_g[p];
											}
										}
										// increment r (the number of the partial derivative within
										//   partial derivatives of the same order
										if ((l<j)&&(product_orders[l]==product_orders[l+1]))
										{
											r++;
										}
										else
										{
											r=0;
										}
									}
									number_of_columns_f=matrix_f->size2();
									// loop across the row of matrix_f and down the columns of
									//   matrix_g
									k=0;
									while (k<number_of_columns_f)
									{
										// calculate the product
										product=(*matrix_f)(row_number,k);
										l=0;
										while (l<=j)
										{
											product *=
												(*matrices_g[l])(index_f[l],column_numbers_g[l]);
											l++;
										}
										// add to sum
										sum += product;
										// increment to next value for derivative in matrix_f and
										//   matrix_g
										k++;
										l=j;
										index_f[l]++;
										while ((l>0)&&(index_f[l]>=number_of_intermediate_values))
										{
											index_f[l]=0;
											l--;
											index_f[l]++;
										}
									}
									// move to the next choice for the j+1 sets
									// first try leaving the number of variables in each set the
									//   same (don't change product_orders).  Do this by running
									//   through the permutations of order_result things with
									//   restrictions
									//   - start with the permutation 0,1,2, ... order_result-1
									//   - for the current permutation
									//   - start at the end and run back looking for an entry
									//     which is less than one of the entries further on and
									//     for which the restrictions hold
									//   - find the smallest entry that is further on than the
									//     current entry, greater than the current entry and
									//     satisfies the restrictions
									//   - put the smallest in the current and add the remaining
									//     entries in increasing order
									l=order_result-1;
									q=mapping_g[l];
									found=false;
									while ((l>0)&&!found)
									{
										p=q;
										l--;
										q=mapping_g[l];
										// check if there is a value further on with a greater index
										//   (unrestricted permutations)
										if (q<p)
										{
											// apply restrictions
											// if have same order
											if (order_g[p]==order_g[q])
											{
												// check that p and q are not in the same partial
												//   derivative and that second set doesn't have values
												//   less than the first value of q's set
												// the order of sets of the same size being unimportant
												//   is equivalent to having the first value of the
												//   first set always less than all the values of the
												//   second set
												if ((sub_order_g[p]/order_g[p]!=
													sub_order_g[q]/order_g[p])&&
													(0!=sub_order_g[q]%order_g[p]))
												{
													found=true;
												}
											}
											else
											{
												// check that q hasn't been tried in a partial
												//   derivative of order_g[p]
												if (order_g[q]<order_g[p])
												{
													found=true;
												}
											}
										}
									}
									if (found)
									{
										// mark as unused the values after l
										for (p=0;p<order_result;p++)
										{
											not_used[p]=false;
										}
										for (p=l;p<order_result;p++)
										{
											not_used[mapping_g[p]]=true;
										}
										q=mapping_g[l];
										p=q;
										found=false;
										// find the smallest valid value after l which is greater
										//   than mapping_g[l]
										do
										{
											p++;
											if (not_used[p])
											{
												if (order_g[p]==order_g[q])
												{
													if ((sub_order_g[p]/order_g[p]!=
														sub_order_g[q]/order_g[p])&&
														(0!=sub_order_g[q]%order_g[p]))
													{
														found=true;
													}
												}
												else
												{
													if (order_g[p]>order_g[q])
													{
														found=true;
													}
												}
											}
										} while (!found);
										// put the smallest value in l
										mapping_g[l]=p;
										not_used[p]=false;
										// put the unused values in increasing order after l
										for (p=0;p<order_result;p++)
										{
											if (not_used[p])
											{
												l++;
												mapping_g[l]=p;
											}
										}
									}
									else
									{
										// look for another choice of the j+1 set sizes.  Having the
										//   order of the sets being unimportant is equivalent to
										//   having the sizes in non-decreasing order and starting
										//   with sizes 1,2,...order_result-j
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
												// have found a new choice of set sizes re-initialize
												//   the variable assignment
												(product_orders[l-1])++;
											}
										}
									}
								} while (l>0);
								offset_f *= 2;
								//???DB.  matrix_f += offset_f;
								for (s=offset_f;s>0;s--)
								{
									matrix_f++;
								}
								j++;
							}
							(*matrix_result)(row_number,column_number_result)=sum;
							// increment to next value for derivative in matrix_g
							j=order_result-1;
							index_g[j]++;
							k=mapping_result[j];
							while ((j>0)&&(index_g[j]>=numbers_of_independent_values[k]))
							{
								index_g[j]=0;
								j--;
								k=mapping_result[j];
								index_g[j]++;
							}
							// increment to next column in derivative (matrix) of result
							column_number_result++;
						}
						// increment to next derivative (matrix)
						index_result++;
						matrix_result++;
					}
					// increment to next row
					row_number++;
				}
			}
			else
			{
				valid=false;
			}
		}
		else
		{
			valid=false;
		}
		delete [] not_used;
		delete [] column_numbers_g;
		delete [] index_f;
		delete [] index_g;
		delete [] mapping_g;
		delete [] mapping_result;
		delete [] numbers_of_independent_values;
		delete [] product_orders;
		delete [] order_g;
		delete [] sub_order_g;
		delete [] matrices_g;
	}
	if (!valid)
	{
		throw Derivative_matrix::Calculation_exception();
	}

	return (Derivative_matrix(matrices_result));
}

Derivative_matrix Derivative_matrix::inverse()
//******************************************************************************
// LAST MODIFIED : 23 December 2004
//
// DESCRIPTION :
// Compute the composition inverse from the chain rule and the relation
//   identity=U_inv(U(X))
// Then using
//   H=identity
//   F=U_inv
//   G=U
// in Function_derivative_matrix_compose gives
//   dU_inv/dY=(dU/dX)^(-1)
//   d2U_inv/dY2=
//     -[sum{p=1,m:dF/dYp*d2Gp/dXidXj}]
//     *(dU/dX)^(-1)*(dU/dX)^(-1)
//   d3U_inv/dY3=
//     -[sum{p=1,m:sum{q=1,m:d2F/dYpdYq*
//       [d2Gp/dXidXj*dGq/dXk+d2Gp/dXidXk*dGq/dXj+dGp/dXi*d2Gq/dXjdXk]}}+
//     sum{p=1,m:dF/dYp*d3Gp/dXidXjdXk}]
//     *(dU/dX)^(-1)*(dU/dX)^(-1)*(dU/dX)^(-1)
//   d4U_inv/dY4=
//     -[sum{p=1,m:sum{q=1,m:sum{r=1,m:d3F/dYpdYqdYr*
//       [d2Gp/dXidXj*dGq/dXk*dGr/dXl+
//       d2Gp/dXidXk*dGq/dXj*dGr/dXl+
//       d2Gp/dXidXl*dGq/dXj*dGr/dXk+
//       dGp/dXi*d2Gq/dXjdXk*dGr/dXl+
//       dGp/dXi*d2Gq/dXjdXl*dGr/dXk+
//       dGp/dXi*dGq/dXj*d2Gr/dXkdXl]}}}+
//     sum{p=1,m:sum{q=1,m:d2F/dYpdYq*
//       [d3Gp/dXidXjdXk*dGq/dXl+
//       d3Gp/dXidXjdXl*dGq/dXk+
//       d3Gp/dXidXkdXl*dGq/dXj+
//       dGp/dXi*d3Gq/dXjdXkdXl+
//       d2Gp/dXidXj*d2Gq/dXkdXl+
//       d2Gp/dXidXk*d2Gq/dXjdXl+
//       d2Gp/dXidXl*d2Gq/dXjdXk]}}+
//     sum{p=1,m:dF/dYp*d4Gp/dXidXjdXkdXl}]
//     *(dU/dX)^(-1)*(dU/dX)^(-1)*(dU/dX)^(-1)
//   ...
// where m=length(Y).
//
//
// If
//   Y=G(X), H(X)=F(G(X))
// then
//   dH/dXi=
//     sum{p=1,m:dF/dYp*dGp/dXi}
//   d2H/dXidXj=
//     sum{p=1,m:sum{q=1,m:d2F/dYpdYq*dGp/dXi*dGq/dXj}}+
//     sum{p=1,m:dF/dYp*d2Gp/dXidXj}
//   d3H/dXidXjdXk=
//     sum{p=1,m:sum{q=1,m:sum{r=1,m:d3F/dYpdYqdYr*dGp/dXi*dGq/dXj*dGr/dXk}}}+
//     sum{p=1,m:sum{q=1,m:d2F/dYpdYq*
//       [d2Gp/dXidXj*dGq/dXk+d2Gp/dXidXk*dGq/dXj+dGp/dXi*d2Gq/dXjdXk]}}+
//     sum{p=1,m:dF/dYp*d3Gp/dXidXjdXk}
//   d4H/dXidXjdXkdXl=
//     sum{p=1,m:sum{q=1,m:sum{r=1,m:sum{s=1,m:
//       d4F/dYpdYqdYrdYs*dGp/dXi*dGq/dXj*dGr/dXk*dGr/dXl}}}}+
//     sum{p=1,m:sum{q=1,m:sum{r=1,m:d3F/dYpdYqdYr*
//       [d2Gp/dXidXj*dGq/dXk*dGr/dXl+
//       d2Gp/dXidXk*dGq/dXj*dGr/dXl+
//       d2Gp/dXidXl*dGq/dXj*dGr/dXk+
//       dGp/dXi*d2Gq/dXjdXk*dGr/dXl+
//       dGp/dXi*d2Gq/dXjdXl*dGr/dXk+
//       dGp/dXi*dGq/dXj*d2Gr/dXkdXl]}}}+
//     sum{p=1,m:sum{q=1,m:d2F/dYpdYq*
//       [d3Gp/dXidXjdXk*dGq/dXl+
//       d3Gp/dXidXjdXl*dGq/dXk+
//       d3Gp/dXidXkdXl*dGq/dXj+
//       dGp/dXi*d3Gq/dXjdXkdXl+
//       d2Gp/dXidXj*d2Gq/dXkdXl+
//       d2Gp/dXidXk*d2Gq/dXjdXl+
//       d2Gp/dXidXl*d2Gq/dXjdXk]}}+
//     sum{p=1,m:dF/dYp*d4Gp/dXidXjdXkdXl}
//   ...
// where m=length(Y).
//
// There are some parts above which don't look symmetric eg the p's and q's in
//   [d2Gp/dXidXj*dGq/dXk+d2Gp/dXidXk*dGq/dXj+dGp/dXi*d2Gq/dXjdXk]
// However for "reasonable" functions the order of differentiation does not
// change the result.  Assuming the functions are "reasonable" gives
//   d2F/dYpdYq=d2F/dYqdYp
// and so
//   [d2Gp/dXidXj*dGq/dXk+d2Gp/dXidXk*dGq/dXj+dGp/dXi*d2Gq/dXjdXk]
// can be replaced by
//   [d2Gp/dXidXj*dGq/dXk+d2Gp/dXidXk*dGq/dXj+d2Gp/dXjdXk*dGq/dXi]
// even though they are not equal because
//   [d2Gp/dXidXj*dGq/dXk+d2Gp/dXidXk*dGq/dXj+d2Gp/dXjdXk*dGq/dXi]+
//   [d2Gq/dXidXj*dGp/dXk+d2Gq/dXidXk*dGp/dXj+d2Gq/dXjdXk*dGp/dXi]
//   =
//   [d2Gp/dXidXj*dGq/dXk+d2Gp/dXidXk*dGq/dXj+dGp/dXi*d2Gq/dXjdXk]+
//   [d2Gq/dXidXj*dGp/dXk+d2Gq/dXidXk*dGp/dXj+dGq/dXi*d2Gp/dXjdXk]
//
// To calculate a derivative of H, the derivatives of all orders of F and G are
// needed and so derivatives of all orders are stored in the derivative matrices
// in the following order
//   d/dX,d/dY,d2/dXdY,d/dZ,d2/dXdZ,d2/dYdZ,d3/dXdYdZ
// NB.  This assumes that order of differentiation does not change the result
//
// In this function, f is the <dependent_variable> and g is the source
// variables for the composition variable.
//==============================================================================
{
	bool valid;
	Function_size_type i;
	std::list<Matrix> matrices_result(0);

	// check compatability
	valid=false;
	if (this&&(0<order))
	{
		i=0;
		while ((i<order)&&
			(valid=(number_of_dependent_values==numbers_of_independent_values[i])))
		{
			i++;
		}
	}
	if (valid)
	{
		bool found;
		bool *not_used;
		Scalar product,sum;
		std::list<Matrix>::iterator matrix_f,matrix_g;
		Function_size_type column_number_result,j,k,l,number_of_columns_f,
			number_of_columns_result,number_of_rows,number_of_steps,offset,offset_f,
			offset_g,order_result,p,q,r,row_number,s,step;
		Function_size_type number_of_matrices=size();
		Function_size_type *column_numbers_g,*index_f,*index_g,*mapping_g,
			*product_orders,*order_g,*sub_order_g;
		std::list<Matrix>::iterator *matrices_g;
		ublas::vector<Function_size_type> matrix_orders(number_of_matrices);
		ublas::vector<Matrix> order_matrices(order);

		// initialize
		number_of_rows=front().size1();
		not_used=new bool[order+1];
		column_numbers_g=new Function_size_type[order+1];
		index_f=new Function_size_type[order+1];
		index_g=new Function_size_type[order+1];
		mapping_g=new Function_size_type[order+1];
		product_orders=new Function_size_type[order+1];
		order_g=new Function_size_type[order+1];
		sub_order_g=new Function_size_type[order+1];
		matrices_g=new std::list<Matrix>::iterator[order+1];
		if (not_used&&column_numbers_g&&index_f&&index_g&&mapping_g&&
			product_orders&&order_g&&sub_order_g&&matrices_g)
		{
			Matrix& matrix_inverse=order_matrices[0];
			std::vector<int> ipiv(number_of_rows);

			// calculate first order derivative
			matrix_inverse.resize(number_of_rows,number_of_rows);
			for (i=0;i<number_of_rows;i++)
			{
				for (j=0;j<number_of_rows;j++)
				{
					if (i==j)
					{
						matrix_inverse(i,j)=(Scalar)1;
					}
					else
					{
						matrix_inverse(i,j)=(Scalar)0;
					}
				}
			}
			lapack::gesv(front(),ipiv,matrix_inverse);
			matrices_result.push_back(matrix_inverse);
			number_of_matrices=1;
			matrix_orders[0]=0;
			order_matrices[0].resize(number_of_rows,number_of_rows);
			order_matrices[0]=matrix_inverse;
			// loop of order.  Can use order rather than matrix because all the
			//   independent variables are the same and so all matrices for an
			//   order are the same
			number_of_columns_result=number_of_rows*number_of_rows;
			order_result=2;
			while (order_result<=order)
			{
				Matrix& matrix_new=order_matrices[order_result-1];

				matrix_new.resize(number_of_rows,number_of_columns_result);
				// calculate order_result derivative
				for (i=0;i<=order_result;i++)
				{
					index_g[i]=0;
				}
				// calculate the values for the derivative
				column_number_result=0;
				while (column_number_result<number_of_columns_result)
				{
					/* loop over dependent values (rows) */
					row_number=0;
					while (row_number<number_of_rows)
					{
						// loop over the sums for different order derivatives of f
						sum=(Scalar)0;
						matrix_f=matrices_result.begin();
						offset_f=1;
						j=0;
						while (j<order_result-1)
						{
							// initialize the orders for the derivatives of g that are
							//   multiplied together.  There are j+1 derivatives of g and
							//   their orders have to sum to the order of the derivative
							//   of result (order_result)
							for (l=0;l<j;l++)
							{
								product_orders[l]=1;
							}
							product_orders[j]=order_result-j;
							// loop over the possible ways of dividing the order_result
							//   independent variables into j+1 non-empty sets, where the
							//   order of the sets and the order within the sets are not
							//   important.  For each possible way, loop across the row of
							//   matrix_f and down the columns of matrix_g and
							//   - calculate the product of the derivatives of g
							//     represented by the sets
							//   - multiply the product by the corresponding derivative of
							//     f
							//   - add the result to result
							do
							{
								// initialize the variable assigment
								// for each of the independent variables being
								//   differentiated with respect to in the product have:
								//   mapping_g - a reordering of the variables without
								//     changing the orders of the partial derivatives.  It
								//     is a location in order_g and sub_order_g
								//   order_g - the order of the partial derivative they're
								//     in
								//   sub_order_g - their position in the variables in
								//     partial derivatives of the same order
								r=0;
								l=0;
								while (l<=j)
								{
									q=0;
									do
									{
										for (p=0;p<product_orders[l];p++)
										{
											mapping_g[r]=r;
											order_g[r]=product_orders[l];
											sub_order_g[r]=q;
											r++;
											q++;
										}
										l++;
									} while ((l<=j)&&
										(product_orders[l]==product_orders[l-1]));
								}
								// find the column numbers of matrix g for the partial
								//   derivatives in the product
								// r is the number of the partial derivative within partial
								//   derivatives of the same order
								r=0;
								for (l=0;l<=j;l++)
								{
									// initialize the value position within the partial
									//   derivative of f
									index_f[l]=0;
									// determine which independent variables are used in the
									//   partial derivative of g
									matrix_g=begin();
									offset_g=1;
									for (p=0;p<order_result;p++)
									{
										q=mapping_g[p];
										// is the same partial derivative within the partial
										//   derivatives of the same order
										if ((product_orders[j]==order_g[q])&&
											(r==sub_order_g[q]/order_g[q]))
										{
											not_used[p]=false;
										}
										else
										{
											not_used[p]=true;
										}
										//???DB.  matrix_g += offset_g;
										for (s=offset_g;s>0;s--)
										{
											matrix_g++;
										}
										offset_g *= 2;
									}
									matrix_g--;
									for (p=order_result;p>0;p--)
									{
										offset_g /= 2;
										if (not_used[p-1])
										{
											//???DB.  matrix_g -= offset_g;
											for (s=offset_g;s>0;s--)
											{
												matrix_g--;
											}
										}
									}
									matrices_g[l]=matrix_g;
									// second the index of the value
									column_numbers_g[l]=0;
									for (p=0;p<order_result;p++)
									{
										if (!not_used[p])
										{
//												column_numbers_g[l]=numbers_of_independent_values[
//													mapping_result[p]]*column_numbers_g[l]+index_g[p];
											column_numbers_g[l]=
												number_of_rows*column_numbers_g[l]+index_g[p];
										}
									}
									// increment r (the number of the partial derivative
									//   within partial derivatives of the same order
									if ((l<j)&&(product_orders[l]==product_orders[l+1]))
									{
										r++;
									}
									else
									{
										r=0;
									}
								}
								number_of_columns_f=matrix_f->size2();
								// loop across the row of matrix_f and down the columns of
								//   matrix_g
								k=0;
								while (k<number_of_columns_f)
								{
									// calculate the product
									product=(*matrix_f)(row_number,k);
									l=0;
									while (l<=j)
									{
										product *=
											(*matrices_g[l])(index_f[l],column_numbers_g[l]);
										l++;
									}
									// add to sum
									sum += product;
									// increment to next value for derivative in matrix_f and
									//   matrix_g
									k++;
									l=j;
									index_f[l]++;
									while ((l>0)&&(index_f[l]>=number_of_rows))
									{
										index_f[l]=0;
										l--;
										index_f[l]++;
									}
								}
								// move to the next choice for the j+1 sets
								// first try leaving the number of variables in each set the
								//   same (don't change product_orders).  Do this by running
								//   through the permutations of order_result things with
								//   restrictions
								//   - start with the permutation 0,1,2, ... order_result-1
								//   - for the current permutation
								//   - start at the end and run back looking for an entry
								//     which is less than one of the entries further on and
								//     for which the restrictions hold
								//   - find the smallest entry that is further on than the
								//     current entry, greater than the current entry and
								//     satisfies the restrictions
								//   - put the smallest in the current and add the remaining
								//     entries in increasing order
								l=order_result-1;
								q=mapping_g[l];
								found=false;
								while ((l>0)&&!found)
								{
									p=q;
									l--;
									q=mapping_g[l];
									// check if there is a value further on with a greater
									//   index (unrestricted permutations)
									if (q<p)
									{
										// apply restrictions
										// if have same order
										if (order_g[p]==order_g[q])
										{
											// check that p and q are not in the same partial
											//   derivative and that second set doesn't have
											//   values less than the first value of q's set
											// the order of sets of the same size being
											//   unimportant is equivalent to having the first
											//   value of the first set always less than all the
											//   values of the second set
											if ((sub_order_g[p]/order_g[p]!=
												sub_order_g[q]/order_g[p])&&
												(0!=sub_order_g[q]%order_g[p]))
											{
												found=true;
											}
										}
										else
										{
											// check that q hasn't been tried in a partial
											//   derivative of order_g[p]
											if (order_g[q]<order_g[p])
											{
												found=true;
											}
										}
									}
								}
								if (found)
								{
									// mark as unused the values after l
									for (p=0;p<order_result;p++)
									{
										not_used[p]=false;
									}
									for (p=l;p<order_result;p++)
									{
										not_used[mapping_g[p]]=true;
									}
									q=mapping_g[l];
									p=q;
									found=false;
									// find the smallest valid value after l which is greater
									//   than mapping_g[l]
									do
									{
										p++;
										if (not_used[p])
										{
											if (order_g[p]==order_g[q])
											{
												if ((sub_order_g[p]/order_g[p]!=
													sub_order_g[q]/order_g[p])&&
													(0!=sub_order_g[q]%order_g[p]))
												{
													found=true;
												}
											}
											else
											{
												if (order_g[p]>order_g[q])
												{
													found=true;
												}
											}
										}
									} while (!found);
									// put the smallest value in l
									mapping_g[l]=p;
									not_used[p]=false;
									// put the unused values in increasing order after l
									for (p=0;p<order_result;p++)
									{
										if (not_used[p])
										{
											l++;
											mapping_g[l]=p;
										}
									}
								}
								else
								{
									// look for another choice of the j+1 set sizes.  Having
									//   the order of the sets being unimportant is equivalent
									//   to having the sizes in non-decreasing order and
									//   starting with sizes 1,2,...order_result-j
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
											// have found a new choice of set sizes re-initialize
											//   the variable assignment
											(product_orders[l-1])++;
										}
									}
								}
							} while (l>0);
							offset_f *= 2;
							//???DB.  matrix_f += offset_f;
							for (s=offset_f;s>0;s--)
							{
								matrix_f++;
							}
							j++;
						}
						matrix_new(row_number,column_number_result)= -sum;
						// increment to next row
						row_number++;
					}
					// increment to next value for derivative in matrix_g
					j=order_result-1;
					index_g[j]++;
//							k=mapping_result[j];
//							while ((j>0)&&(index_g[j]>=numbers_of_independent_values[k]))
					while ((j>0)&&(index_g[j]>=number_of_rows))
					{
						index_g[j]=0;
						j--;
//								k=mapping_result[j];
						index_g[j]++;
					}
					// increment to next column in derivative (matrix) of result
					column_number_result++;
				}
				// "multiply" by inverse of first order derivative order_result times
				number_of_steps=1;
				step=column_number_result/number_of_rows;
				for (k=order_result;k>0;k--)
				{
					Matrix matrix_temp(number_of_rows,number_of_rows);

					for (l=0;l<number_of_steps;l++)
					{
						for (p=0;p<step;p++)
						{
							offset=l*number_of_rows*step+p;
							for (j=0;j<number_of_rows;j++)
							{
								for (i=0;i<number_of_rows;i++)
								{
									matrix_temp(i,j)=matrix_new(i,offset);
								}
								offset += step;
							}
							// multiply matrix_temp by matrix_inverse and put back
							offset=l*number_of_rows*step+p;
							for (j=0;j<number_of_rows;j++)
							{
								for (i=0;i<number_of_rows;i++)
								{
									sum=0;
									for (q=0;q<number_of_rows;q++)
									{
										sum += matrix_temp(i,q)*matrix_inverse(q,j);
									}
									matrix_new(i,offset)=sum;
								}
								offset += step;
							}
						}
					}
					number_of_steps *= number_of_rows;
					step /= number_of_rows;
				}
				// update matrices_result list
				k=matrices_result.size();
				matrices_result.push_back(matrix_inverse);
				matrix_orders[k]=0;
				for (i=0;i<k;i++)
				{
					matrix_orders[i+k+1]=matrix_orders[i]+1;
					matrices_result.push_back(order_matrices[matrix_orders[i+k+1]]);
				}
				number_of_columns_result *= number_of_rows;
				order_result++;
			}
		}
		delete [] not_used;
		delete [] column_numbers_g;
		delete [] index_f;
		delete [] index_g;
		delete [] mapping_g;
		delete [] product_orders;
		delete [] order_g;
		delete [] sub_order_g;
		delete [] matrices_g;
	}
	if (!valid)
	{
		throw Derivative_matrix::Calculation_exception();
	}

	return (Derivative_matrix(matrices_result));
}
