//******************************************************************************
// FILE : variable_derivative_matrix.cpp
//
// LAST MODIFIED : 16 December 2003
//
// DESCRIPTION :
//==============================================================================

#include <algorithm>
#include <iterator>
#include <new>
#include <sstream>
#include <stdexcept>
#include <string>
#include <typeinfo>

//???DB.  Put in include?
const bool Assert_on=true;

template<class Assertion,class Exception>inline void Assert(
	Assertion assertion,Exception exception)
{
	if (Assert_on&&!(assertion)) throw exception;
}

#include "computed_variable/variable_derivative_matrix.hpp"

// class Variable_derivative_matrix
// --------------------------------

Variable_derivative_matrix::Variable_derivative_matrix(
	const Variable_handle& dependent_variable,
	const std::list<Variable_input_handle>& independent_variables,
	const std::list<Matrix>& matrices) : Variable(),
	dependent_variable(dependent_variable),
	independent_variables(independent_variables),matrices(matrices)
//******************************************************************************
// LAST MODIFIED : 19 November 2003
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	//???DB.  Check that consistent?
}

class Variable_derivative_matrix_create_matrices_inner_functor
//******************************************************************************
// LAST MODIFIED : 12 October 2003
//
// DESCRIPTION :
// A unary function (Functor) for creating and zeroing the list of matrices for
// a derivative matrix variable from a list of independent variables.  Inner
// loop.
//==============================================================================
{
	public:
		Variable_derivative_matrix_create_matrices_inner_functor(
			const Variable_handle& dependent_variable,
			const Variable_input_handle& independent_variable,
			int number_of_independent_values,std::list<Matrix>& matrices,
			std::list< std::list<Variable_input_handle> >&
			matrix_independent_variables):
			number_of_independent_values(number_of_independent_values),
			matrices(matrices),
			matrix_independent_variables(matrix_independent_variables),
			matrix_independent_variables_iterator(
			matrix_independent_variables.begin()),
			dependent_variable(dependent_variable),
			independent_variable(independent_variable)
		{
		};
		~Variable_derivative_matrix_create_matrices_inner_functor() {};
		int operator() (Matrix& matrix)
		{
			Matrix new_matrix((matrix.size1)(),
				number_of_independent_values*(matrix.size2)());
			std::list<Variable_input_handle> new_matrix_independent_variables;

			new_matrix_independent_variables= *matrix_independent_variables_iterator;
			new_matrix_independent_variables.push_back(independent_variable);
			dependent_variable->evaluate_derivative_local(new_matrix,
				new_matrix_independent_variables);
			matrices.push_back(new_matrix);
			matrix_independent_variables.push_back(new_matrix_independent_variables);
			matrix_independent_variables_iterator++;

			return (1);
		};
	private:
		int number_of_independent_values;
		std::list<Matrix>& matrices;
		std::list< std::list<Variable_input_handle> >& matrix_independent_variables;
		std::list< std::list<Variable_input_handle> >::iterator
			matrix_independent_variables_iterator;
		const Variable_handle& dependent_variable;
		const Variable_input_handle& independent_variable;
};

class Variable_derivative_matrix_create_matrices_outer_functor
//******************************************************************************
// LAST MODIFIED : 25 November 2003
//
// DESCRIPTION :
// A unary function (Functor) for creating and zeroing the list of matrices for
// a derivative matrix variable from a list of independent variables.  Outer
// loop.
//==============================================================================
{
	public:
		Variable_derivative_matrix_create_matrices_outer_functor(
			const Variable_handle& dependent_variable,std::list<Matrix>& matrices):
			number_of_dependent_values(dependent_variable->size()),matrices(matrices),
			matrix_independent_variables(),dependent_variable(dependent_variable)
		{};
		~Variable_derivative_matrix_create_matrices_outer_functor() {};
		int operator() (const Variable_input_handle& independent_variable)
		{
			int number_of_independent_values=independent_variable->size();
			Matrix new_matrix(number_of_dependent_values,
				number_of_independent_values);
			std::list<Variable_input_handle> new_matrix_independent_variables;
			std::list<Matrix>::iterator last;

			new_matrix_independent_variables.push_back(independent_variable);
			dependent_variable->evaluate_derivative_local(new_matrix,
				new_matrix_independent_variables);
			matrices.push_back(new_matrix);
			matrix_independent_variables.push_back(new_matrix_independent_variables);
			last=matrices.end();
			last--;
			std::for_each(matrices.begin(),last,
				Variable_derivative_matrix_create_matrices_inner_functor(
				dependent_variable,independent_variable,number_of_independent_values,
				matrices,matrix_independent_variables));

			return (1);
		};
	private:
		int number_of_dependent_values;
		std::list<Matrix>& matrices;
		std::list< std::list<Variable_input_handle> > matrix_independent_variables;
		const Variable_handle& dependent_variable;
};

Variable_derivative_matrix::Variable_derivative_matrix(
	const Variable_handle& dependent_variable,
	const std::list<Variable_input_handle>& independent_variables) :
	Variable(),dependent_variable(dependent_variable),
	independent_variables(independent_variables),matrices()
//******************************************************************************
// LAST MODIFIED : 25 November 2003
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	std::for_each(independent_variables.begin(),independent_variables.end(),
		Variable_derivative_matrix_create_matrices_outer_functor(
		dependent_variable,matrices));
}

Variable_derivative_matrix::Variable_derivative_matrix(
	const Variable_derivative_matrix& derivative_matrix) : Variable(),
	dependent_variable(derivative_matrix.dependent_variable),
	independent_variables(derivative_matrix.independent_variables),
	matrices(derivative_matrix.matrices){}
//******************************************************************************
// LAST MODIFIED : 16 October 2003
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================

Variable_derivative_matrix::~Variable_derivative_matrix()
//******************************************************************************
// LAST MODIFIED : 2 October 2003
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	//???DB.  To be done
}

class Variable_derivative_matrix_calculate_size_functor
//******************************************************************************
// LAST MODIFIED : 14 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	public:
		Variable_derivative_matrix_calculate_size_functor(
			Variable_size_type& result):result(result){};
		int operator() (const Matrix& matrix)
		{
			result += (matrix.size1())*(matrix.size2());
			return (0);
		}
		Variable_size_type& result;
};

Variable_size_type Variable_derivative_matrix::size() const
//******************************************************************************
// LAST MODIFIED : 14 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	Variable_size_type result=0;

	std::for_each(matrices.begin(),matrices.end(),
		Variable_derivative_matrix_calculate_size_functor(result));

	return (result);
}

class Variable_derivative_matrix_get_scalars_functor
//******************************************************************************
// LAST MODIFIED : 24 October 2003
//
// DESCRIPTION :
//==============================================================================
{
	public:
		Variable_derivative_matrix_get_scalars_functor(Vector& values):
			index(0),values(values){};
		int operator() (Matrix& matrix)
		{
			Variable_size_type i,j,number_of_columns=matrix.size2(),
				number_of_rows=matrix.size1();

			for (i=0;i<number_of_rows;i++)
			{
				for (j=0;j<number_of_columns;j++)
				{
					values[index]=matrix(i,j);
					index++;
				}
			}

			return (0);
		}
	private:
		Variable_size_type index;
		Vector& values;
};

Vector *Variable_derivative_matrix::scalars()
//******************************************************************************
// LAST MODIFIED : 24 October 2003
//
// DESCRIPTION :
//==============================================================================
{
	Vector *values_vector;

	if (values_vector=new Vector(size()))
	{
		std::for_each(matrices.begin(),matrices.end(),
			Variable_derivative_matrix_get_scalars_functor(*values_vector));
	}

	return (values_vector);
}

class Variable_derivative_matrix_get_matrix_functor
//******************************************************************************
// LAST MODIFIED : 6 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	public:
		Variable_derivative_matrix_get_matrix_functor(Variable_size_type& index,
			std::list<Matrix>::reverse_iterator& matrix_iterator,
			std::list<Matrix>& matrices,
			std::list<Variable_input_handle>& partial_independent_variables):
			matrix_iterator(matrix_iterator),partial_independent_variable_iterator(
			partial_independent_variables.rbegin()),index(index),
			offset(1+matrices.size())
		{
			matrix_iterator=matrices.rbegin();
			index=partial_independent_variables.size();
		};
		int operator() (Variable_input_handle& independent_variable)
		{
			bool found;

			offset /=2;
			if (index>0)
			{
				found=(**partial_independent_variable_iterator== *independent_variable);
			}
			else
			{
				found=false;
			}
			if (found)
			{
				index--;
				partial_independent_variable_iterator++;
			}
			else
			{
				int i;

				for (i=offset;i>0;i--)
				{
					matrix_iterator++;
				}
			}

			return (0);
		}
	private:
		std::list<Matrix>::reverse_iterator& matrix_iterator;
		std::list<Variable_input_handle>::reverse_iterator
			partial_independent_variable_iterator;
		Variable_size_type& index;
		Variable_size_type offset;
};

Variable_matrix_handle Variable_derivative_matrix::matrix(
	std::list<Variable_input_handle>& partial_independent_variables)
//******************************************************************************
// LAST MODIFIED : 6 November 2003
//
// DESCRIPTION :
// Returns the specified partial derivative (<partial_independent_variables>).
//
// ???DB.  Extend so that can have an independent variable that is a subset of
//   one of the independent variables for the derivative matrix.  eg nodal
//   values for a particular node as a subset of all nodal values
//==============================================================================
{
	std::list<Matrix>::reverse_iterator matrix_iterator;
	Variable_matrix_handle result;
	Variable_size_type index;

	std::for_each((this->independent_variables).rbegin(),
		(this->independent_variables).rend(),
		Variable_derivative_matrix_get_matrix_functor(index,matrix_iterator,
		matrices,partial_independent_variables));
	if (0==index)
	{
		result=Variable_matrix_handle(new Variable_matrix(*matrix_iterator));
	}
	else
	{
		Matrix temp_matrix(0,0);

		result=Variable_matrix_handle(new Variable_matrix(temp_matrix));
	}

	return (result);
}

Variable_derivative_matrix_handle Variable_derivative_matrix_compose(
	const Variable_handle& dependent_variable,
	const Variable_derivative_matrix_handle& derivative_f,
	const Variable_derivative_matrix_handle& derivative_g)
//******************************************************************************
// LAST MODIFIED : 12 December 2003
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
	Variable_derivative_matrix_handle result(0);
	Variable_size_type i;

#if defined (DEBUG)
	//???debug
	std::cout << "enter Variable_derivative_matrix_compose" << std::endl;
#endif // defined (DEBUG)
	if (derivative_f&&derivative_g&&
		(0<(i=(derivative_f->independent_variables).size())))
	{
		std::list<Variable_input_handle>::iterator independent_variable_iterator;
		Variable_input_handle last_independent_variable;

#if defined (DEBUG)
		//???debug
		std::cout << "derivative_f=" << (derivative_f->matrices).front() <<
			std::endl;
		//???debug
		std::cout << "derivative_g=" << (derivative_g->matrices).front() <<
			std::endl;
#endif // defined (DEBUG)
		// check that all derivative_f independent variables are the same
		if (last_independent_variable=(derivative_f->independent_variables).back())
		{
			independent_variable_iterator=
				(derivative_f->independent_variables).begin();
			i--;
			while ((i>0)&&(*independent_variable_iterator)&&
				(**independent_variable_iterator== *last_independent_variable))
			{
				independent_variable_iterator++;
				i--;
			}
		}
		if (0==i)
		{
			bool found;
			bool *not_used;
			Scalar product,sum;
			std::list<Matrix> matrices_result(0);
			std::list<Matrix>::iterator matrix_f,matrix_g,matrix_result;
			Variable_size_type column_number_result,index_result,j,k,l,
				number_of_columns_f,number_of_columns_result,
				number_of_intermediate_values,number_of_matrices,number_of_rows,order,
				offset_f,offset_g,order_result,p,q,r,row_number,s;
			Variable_size_type *column_numbers_g,*index_f,*index_g,*mapping_g,
				*mapping_result,*numbers_of_independent_values,*product_orders,*order_g,
				*sub_order_g;
			std::list<Matrix>::iterator *matrices_g;

			// initialize
			number_of_matrices=(derivative_g->matrices).size();
			number_of_rows=derivative_f->dependent_variable->size();
			number_of_intermediate_values=(derivative_g->dependent_variable)->size();
			independent_variable_iterator=
				(derivative_g->independent_variables).begin();
			order=(derivative_g->independent_variables).size();
			not_used=new bool[order+1];
			column_numbers_g=new Variable_size_type[order+1];
			index_f=new Variable_size_type[order+1];
			index_g=new Variable_size_type[order+1];
			mapping_g=new Variable_size_type[order+1];
			mapping_result=new Variable_size_type[order+1];
			numbers_of_independent_values=new Variable_size_type[order];
			product_orders=new Variable_size_type[order+1];
			order_g=new Variable_size_type[order+1];
			sub_order_g=new Variable_size_type[order+1];
			matrices_g=new std::list<Matrix>::iterator[order+1];
			if (not_used&&column_numbers_g&&index_f&&index_g&&mapping_g&&
				mapping_result&&numbers_of_independent_values&&product_orders&&order_g&&
				sub_order_g&&matrices_g)
			{
				for (i=0;i<order;i++)
				{
					numbers_of_independent_values[i]=
						(*independent_variable_iterator)->size();
					independent_variable_iterator++;
				}
				matrix_g=(derivative_g->matrices).begin();
				for (i=0;i<number_of_matrices;i++)
				{
					Matrix new_matrix(number_of_rows,matrix_g->size2());

					matrices_result.push_back(new_matrix);
					matrix_g++;
				}
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
							matrix_f=(derivative_f->matrices).begin();
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
										matrix_g=(derivative_g->matrices).begin();
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
				result=Variable_derivative_matrix_handle(new Variable_derivative_matrix(
					dependent_variable,derivative_g->independent_variables,
					matrices_result));
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
	}
#if defined (DEBUG)
	//???debug
	std::cout << "leave Variable_derivative_matrix_compose" << std::endl;
#endif // defined (DEBUG)

	return (result);
}

Variable_handle Variable_derivative_matrix::inverse(
	const Variable_inverse_handle& dependent_variable)
//******************************************************************************
// LAST MODIFIED : 16 December 2003
//
// DESCRIPTION :
// Compute the composition inverse from the chain rule and the relation
//   identity=U_inv(U(X))
// Then using
//   H=identity
//   F=U_inv
//   G=U
// in Variable_derivative_matrix_compose gives
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
	Variable_derivative_matrix_handle derivative(this);
	Variable_derivative_matrix_handle result(0);
	Variable_size_type i;

	if (derivative&&(0<(i=(derivative->independent_variables).size()))&&
		dependent_variable)
	{
		std::list<Variable_input_handle> inverse_independent_variables;
		std::list<Variable_input_handle>::iterator independent_variable_iterator;
		Variable_input_handle inverse_dependent_variable,
			inverse_independent_variable;

		// check that all derivative independent variables are the same
		if ((inverse_dependent_variable=dependent_variable->dependent_variable)&&
			(inverse_independent_variable=dependent_variable->input_independent()))
		{
			independent_variable_iterator=(derivative->independent_variables).begin();
			while ((i>0)&&(*independent_variable_iterator)&&
				(**independent_variable_iterator== *inverse_dependent_variable))
			{
				inverse_independent_variables.push_back(inverse_independent_variable);
				independent_variable_iterator++;
				i--;
			}
		}
		if ((0==i)&&((derivative->dependent_variable)->size()==
			((derivative->independent_variables).back())->size()))
		{
			bool found;
			bool *not_used;
			Scalar product,sum;
			std::list<Matrix> matrices_result(0);
			std::list<Matrix>::iterator matrix_f,matrix_g;
			Variable_size_type column_number_result,j,k,l,number_of_columns_f,
				number_of_columns_result,number_of_rows,number_of_steps,offset,offset_f,
				offset_g,order_result,p,q,r,row_number,s,step;
			Variable_size_type number_of_matrices=(derivative->matrices).size();
			Variable_size_type order=(derivative->independent_variables).size();
			Variable_size_type *column_numbers_g,*index_f,*index_g,*mapping_g,
				*product_orders,*order_g,*sub_order_g;
			std::list<Matrix>::iterator *matrices_g;
			ublas::vector<Variable_size_type> matrix_orders(number_of_matrices);
			ublas::vector<Matrix> order_matrices(order);

			// initialize
			number_of_rows=derivative->dependent_variable->size();
			not_used=new bool[order+1];
			column_numbers_g=new Variable_size_type[order+1];
			index_f=new Variable_size_type[order+1];
			index_g=new Variable_size_type[order+1];
			mapping_g=new Variable_size_type[order+1];
			product_orders=new Variable_size_type[order+1];
			order_g=new Variable_size_type[order+1];
			sub_order_g=new Variable_size_type[order+1];
			matrices_g=new std::list<Matrix>::iterator[order+1];
			if (not_used&&column_numbers_g&&index_f&&index_g&&mapping_g&&
				product_orders&&order_g&&sub_order_g&&matrices_g)
			{
				Matrix& matrix_inverse=order_matrices[0];
				Variable_matrix_handle matrix_inverse_handle(0);

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
				if (matrix_inverse_handle=Variable_matrix_handle(new Variable_matrix(
					matrix_inverse)))
				{
					if (matrix_inverse_handle=Variable_matrix(
						(derivative->matrices).front()).solve(matrix_inverse_handle))
					{
						for (i=0;i<number_of_rows;i++)
						{
							for (j=0;j<number_of_rows;j++)
							{
								matrix_inverse(i,j)=(*matrix_inverse_handle)(i,j);
							}
						}
					}
					matrix_inverse_handle=0;
				}
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
										matrix_g=(derivative->matrices).begin();
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
				result=Variable_derivative_matrix_handle(new Variable_derivative_matrix(
					dependent_variable,inverse_independent_variables,
					matrices_result));
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
	}

	return (result);
}

Variable_handle Variable_derivative_matrix::clone() const
//******************************************************************************
// LAST MODIFIED : 8 December 2003
//
// DESCRIPTION :
//==============================================================================
{
	return (Variable_derivative_matrix_handle(new Variable_derivative_matrix(
		*this)));
}

Variable_handle Variable_derivative_matrix::evaluate_local()
//******************************************************************************
// LAST MODIFIED : 5 November 2003
//
// DESCRIPTION :
// Duplicate <this> so that <this> isn't changed by operations on the result.
//==============================================================================
{
	return (Variable_handle(new Variable_derivative_matrix(*this)));
}

void Variable_derivative_matrix::evaluate_derivative_local(Matrix &matrix,
	std::list<Variable_input_handle>& independent_variables)
//******************************************************************************
// LAST MODIFIED : 12 October 2003
//
// DESCRIPTION :
//==============================================================================
{
	//???DB.  To be done
	// matrix is zero'd on entry
	//???DB.  Need to set up input for Variable_derivative_matrix
	if (1==independent_variables.size())
	{
		//???DB.  Really want to return identity matrix
		//???DB.  Need to check matrix size
		matrix(0,0)=1;
	}
}

Variable_handle Variable_derivative_matrix::get_input_value_local(
	const Variable_input_handle& input)
//******************************************************************************
// LAST MODIFIED : 16 October 2003
//
// DESCRIPTION :
//==============================================================================
{
	Variable_handle value;

	//???DB.  To be done
	//???DB.  Temporary code to use up arguments
	if (0<(input->size()))
	{
		value=Variable_derivative_matrix_handle(new Variable_derivative_matrix(
			*this));
	}
	else
	{
		value=Variable_handle((Variable *)0);
	}

	return (value);
}

int Variable_derivative_matrix::set_input_value_local(
	const Variable_input_handle& input,const Variable_handle& value)
//******************************************************************************
// LAST MODIFIED : 2 October 2003
//
// DESCRIPTION :
//==============================================================================
{
	int return_code;

	//???DB.  To be done
	//???DB.  Temporary code to use up arguments
	return_code=0;
	if ((0<(input->size()))&&(value->get_string_representation()))
	{
		return_code=1;
	}

	return (return_code);
}

string_handle Variable_derivative_matrix::get_string_representation_local()
//******************************************************************************
// LAST MODIFIED : 17 October 2003
//
// DESCRIPTION :
//==============================================================================
{
	string_handle return_string;
	std::ostringstream out;

	if (return_string=new std::string)
	{
		out << matrices.back();
		*return_string=out.str();
	}

	return (return_string);
}
