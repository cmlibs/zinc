//******************************************************************************
// FILE : variable_composition.cpp
//
// LAST MODIFIED : 26 November 2003
//
// DESCRIPTION :
//==============================================================================

#include <new>
#include <sstream>
#include <string>
#include <stdio.h>

//???DB.  Put in include?
const bool Assert_on=true;

template<class Assertion,class Exception>inline void Assert(
	Assertion assertion,Exception exception)
{
	if (Assert_on&&!(assertion)) throw exception;
}

#include "computed_variable/variable_composite.hpp"
#include "computed_variable/variable_composition.hpp"
#include "computed_variable/variable_derivative_matrix.hpp"
#include "computed_variable/variable_input_composite.hpp"

// module classes
// ==============

class Variable_identity : public Variable
//******************************************************************************
// LAST MODIFIED : 24 November 2003
//
// DESCRIPTION :
// An identity variable with a specified input.  Used in calculating derivative
// of composition.
//==============================================================================
{
	// can be used by any function
	public:
		// constructor
		Variable_identity(const Variable_input_handle input):Variable(),
			input_private(input) {};
		// copy constructor
		Variable_identity(const Variable_identity& variable_identity):Variable(),
			input_private(variable_identity.input_private) {};
		// assignment
		Variable_identity& operator=(const Variable_identity& variable_identity)
		{
			this->input_private=variable_identity.input_private;

			return (*this);
		};
		// destructor
		~Variable_identity() {};
		// get the number of scalars in the result
		Variable_size_type size()
		{
			return (input_private->size());
		};
		// get the scalars in the result
		Vector *scalars()
		{
			//???DB.  Could have a variable as part of data, but don't know how to
			//  get and not needed for calculating derivative of composition
			return ((Vector *)0);
		};
		// input specifier
		Variable_input_handle input()
		{
			return (input_private);
		};
	private:
		Variable_handle evaluate_local()
		{
			//???DB.  Could have a variable as part of data, but don't know how to
			//  get and not needed for calculating derivative of composition
			return (Variable_handle(0));
		};
		void evaluate_derivative_local(Matrix& matrix,
			std::list<Variable_input_handle>& independent_variables)
		{
			// matrix is zero'd on entry
			if (1==independent_variables.size())
			{
				std::list<Variable_input_handle> dependent_inputs(0),
					independent_inputs(0);
				Variable_input_handle independent_input=independent_variables.front();
				Variable_input_composite_handle independent_input_composite=
#if defined (USE_SMART_POINTER)
					boost::dynamic_pointer_cast<Variable_input_composite,Variable_input>(
					independent_input);
#else /* defined (USE_SMART_POINTER) */
					dynamic_cast<Variable_input_composite *>(independent_input);
#endif /* defined (USE_SMART_POINTER) */
				Variable_input_handle dependent_input=input_private;
				Variable_input_composite_handle dependent_input_composite=
#if defined (USE_SMART_POINTER)
					boost::dynamic_pointer_cast<Variable_input_composite,Variable_input>(
					dependent_input);
#else /* defined (USE_SMART_POINTER) */
					dynamic_cast<Variable_input_composite *>(dependent_input);
#endif /* defined (USE_SMART_POINTER) */
				Variable_size_type column,i,j,k,row;

				Assert((matrix.size1()==dependent_input->size())&&
					(matrix.size2()==independent_input->size()),std::logic_error(
					"Variable_identity::evaluate_derivative_local.  "
					"Incorrect matrix size"));
				if (independent_input_composite)
				{
					independent_inputs.insert(independent_inputs.end(),
						independent_input_composite->begin(),
						independent_input_composite->end());
				}
				else
				{
					independent_inputs.push_back(independent_input);
				}
				if (dependent_input_composite)
				{
					dependent_inputs.insert(dependent_inputs.end(),
						dependent_input_composite->begin(),
						dependent_input_composite->end());
				}
				else
				{
					dependent_inputs.push_back(dependent_input);
				}
				std::list<Variable_input_handle>::iterator dependent_input_iterator=
					dependent_inputs.begin();
				row=0;
				for (i=dependent_inputs.size();i>0;i--)
				{
					std::list<Variable_input_handle>::iterator independent_input_iterator=
						independent_inputs.begin();
					Variable_size_type number_of_dependent=
						(*dependent_input_iterator)->size();

					column=0;
					for (j=independent_inputs.size();j>0;j--)
					{
						if (*dependent_input_iterator== *independent_input_iterator)
						{
							for (k=0;k<number_of_dependent;k++)
							{
								matrix(row+k,column+k)=1;
							}
						}
						column += (*independent_input_iterator)->size();
						independent_input_iterator++;
					}
					row += number_of_dependent;
					dependent_input_iterator++;
				}
			}
		};
		Variable_handle get_input_value_local(const Variable_input_handle&)
		{
			//???DB.  Could have a variable as part of data, but don't know how to
			//  get and not needed for calculating derivative of composition
			return (Variable_handle(0));
		};
		int set_input_value_local(const Variable_input_handle&,
			const Variable_handle&)
		{
			//???DB.  Could have a variable as part of data, but don't know how to
			//  get and not needed for calculating derivative of composition
			return (0);
		};
		string_handle get_string_representation_local()
		{
			string_handle return_string;
			std::ostringstream out;

			if (return_string=new std::string)
			{
				out << "identity";
				*return_string=out.str();
			}

			return (return_string);
		};
	private:
		Variable_input_handle input_private;
};

#if defined (USE_INTRUSIVE_SMART_POINTER)
typedef boost::intrusive_ptr<Variable_identity> Variable_identity_handle;
#elif defined (USE_SMART_POINTER)
typedef boost::shared_ptr<Variable_identity> Variable_identity_handle;
#else
typedef Variable_identity * Variable_identity_handle;
#endif


// global classes
// ==============

// class Variable_composition
// --------------------------

Variable_composition::Variable_composition(
	const Variable_handle dependent_variable,
	const std::list<Variable_input_value_handle> input_source_list):Variable(),
	dependent_variable(dependent_variable),input_source_list(input_source_list)
//******************************************************************************
// LAST MODIFIED : 17 November 2003
//
// DESCRIPTION :
//==============================================================================
{
}

Variable_composition::Variable_composition(
	const Variable_composition& variable_composition):Variable(),
	dependent_variable(variable_composition.dependent_variable),
	input_source_list(variable_composition.input_source_list)
//******************************************************************************
// LAST MODIFIED : 17 November 2003
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
}

Variable_composition& Variable_composition::operator=(
	const Variable_composition& variable_composition)
//******************************************************************************
// LAST MODIFIED : 17 November 2003
//
// DESCRIPTION :
// Assignment operator.
//==============================================================================
{
	dependent_variable=variable_composition.dependent_variable;
	input_source_list=variable_composition.input_source_list;

	return (*this);
}

Variable_composition::~Variable_composition()
//******************************************************************************
// LAST MODIFIED : 17 November 2003
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	// do nothing
}

Variable_size_type Variable_composition::size()
//******************************************************************************
// LAST MODIFIED : 17 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	return (dependent_variable->size());
}

Vector *Variable_composition::scalars()
//******************************************************************************
// LAST MODIFIED : 18 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	return (evaluate_local()->scalars());
}

class Variable_composition_evaluate_input_source_functor
//******************************************************************************
// LAST MODIFIED : 21 November 2003
//
// DESCRIPTION :
// Functor for evaluating the source variables to make a values list.
//==============================================================================
{
	public:
		Variable_composition_evaluate_input_source_functor(
			std::list<Variable_input_value_handle>& values,
			std::list<Variable_input_value_handle>& values_extended):values(values),
			values_extended(values_extended) {};
		~Variable_composition_evaluate_input_source_functor() {};
		int operator() (Variable_input_value_handle& input_source)
		{
			Variable_input_handle input=input_source->input();
			Variable_handle value=(input_source->value())->evaluate(values);

			values_extended.push_back(Variable_input_value_handle(
				new Variable_input_value(input,value)));
			
			return (0);
		}
	private:
		std::list<Variable_input_value_handle>& values;
		std::list<Variable_input_value_handle>& values_extended;
};

Variable_handle Variable_composition::evaluate(
	std::list<Variable_input_value_handle>& values)
//******************************************************************************
// LAST MODIFIED : 25 November 2003
//
// DESCRIPTION :
// Overloading Variable::evaluate.
//==============================================================================
{
	std::list<Variable_input_value_handle> values_extended(values);

	std::for_each(input_source_list.begin(),input_source_list.end(),
		Variable_composition_evaluate_input_source_functor(values,values_extended));

	return (dependent_variable->evaluate(values_extended));
}

Variable_handle Variable_composition::evaluate_derivative(
	std::list<Variable_input_handle>& independent_variables,
	std::list<Variable_input_value_handle>& values)
//******************************************************************************
// LAST MODIFIED : 26 November 2003
//
// DESCRIPTION :
// Overloading Variable::evaluate_derivative.
//
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
//
// ???DB.  Check that independent variables don't match inputs that have been
//   re-directed to sources
//==============================================================================
{
	bool valid_independent_variables;
	std::list<Variable_handle> g_variables(0);
	std::list<Variable_input_handle> f_inputs(0),f_independent_variables(0);
	std::list<Variable_input_handle>::iterator independent_variables_iterator;
	std::list<Variable_input_value_handle> values_extended(values);
	std::list<Variable_input_value_handle>::iterator input_source_iterator;
	Variable_handle result(0);
	Variable_size_type i;

	valid_independent_variables=true;
	// set up f inputs and g
	// assume that the inputs in the specified input, source pairs are independent
		//???DB.  Check when create?
	// check for differentiating wrt input/source variables
	input_source_iterator=input_source_list.begin();
	i=input_source_list.size();
	while (valid_independent_variables&&(i>0))
	{
		if (independent_variables.end()==std::find(independent_variables.begin(),
			independent_variables.end(),(*input_source_iterator)->input()))
		{
			g_variables.push_back((*input_source_iterator)->value());
			f_inputs.push_back((*input_source_iterator)->input());
		}
		else
		{
			valid_independent_variables=false;
		}
		input_source_iterator++;
		i--;
	}
	if (valid_independent_variables)
	{
		// check for repeated independent variables
		independent_variables_iterator=independent_variables.begin();
		for (i=independent_variables.size();i>0;i--)
		{
			if (f_inputs.end()==std::find(f_inputs.begin(),f_inputs.end(),
				*independent_variables_iterator))
			{
				g_variables.push_back(Variable_handle(new Variable_identity(
					*independent_variables_iterator)));
				f_inputs.push_back(*independent_variables_iterator);
			}
			independent_variables_iterator++;
		}
		// set up extended values list for f
		std::for_each(input_source_list.begin(),input_source_list.end(),
			Variable_composition_evaluate_input_source_functor(values,
			values_extended));
		Variable_input_composite_handle f_independent_variable(
			new Variable_input_composite(f_inputs));
		Variable_composite_handle g(new Variable_composite(g_variables));
		for (i=independent_variables.size();i>0;i--)
		{
			f_independent_variables.push_back(f_independent_variable);
		}
		Variable_derivative_matrix_handle derivative_f=
#if defined (USE_SMART_POINTER)
			boost::dynamic_pointer_cast<Variable_derivative_matrix,Variable>
#else /* defined (USE_SMART_POINTER) */
			dynamic_cast<Variable_derivative_matrix *>
#endif /* defined (USE_SMART_POINTER) */
			((dependent_variable->evaluate_derivative)(f_independent_variables,
			values_extended));
		Variable_derivative_matrix_handle derivative_g=
#if defined (USE_SMART_POINTER)
			boost::dynamic_pointer_cast<Variable_derivative_matrix,Variable>
#else /* defined (USE_SMART_POINTER) */
			dynamic_cast<Variable_derivative_matrix *>
#endif /* defined (USE_SMART_POINTER) */
			((g->evaluate_derivative)(independent_variables,values));
		if (derivative_f&&derivative_g)
		{
			bool found;
			bool *not_used;
			Scalar product,sum;
			std::list<Matrix> matrices_result(0);
			std::list<Matrix>::iterator matrix_f,matrix_g,matrix_result;
			std::list<Variable_input_handle>::iterator independent_variable_iterator;
			Variable_size_type column_number_result,i,index_result,j,k,l,
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
			number_of_intermediate_values=derivative_g->dependent_variable->size();
			independent_variable_iterator=
				(derivative_g->independent_variables).begin();
			order=(derivative_g->independent_variables).size();
			not_used=new bool[order+1];
			column_numbers_g=new Variable_size_type[order+1];
			index_f=new Variable_size_type[order+1];
			mapping_g=new Variable_size_type[order+1];
			mapping_result=new Variable_size_type[order+1];
			numbers_of_independent_values=new Variable_size_type[order];
			product_orders=new Variable_size_type[order+1];
			order_g=new Variable_size_type[order+1];
			sub_order_g=new Variable_size_type[order+1];
			matrices_g=new std::list<Matrix>::iterator[order+1];
			if (not_used&&column_numbers_g&&index_f&&mapping_g&&mapping_result&&
				numbers_of_independent_values&&product_orders&&order_g&&sub_order_g&&
				matrices_g)
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
				result=Variable_handle(new Variable_derivative_matrix(
					Variable_handle(this),independent_variables,matrices_result));
			}
			delete [] not_used;
			delete [] column_numbers_g;
			delete [] index_f;
			delete [] mapping_g;
			delete [] mapping_result;
			delete [] numbers_of_independent_values;
			delete [] product_orders;
			delete [] order_g;
			delete [] sub_order_g;
			delete [] matrices_g;
		}
	}

	return (result);
}

Variable_handle Variable_composition::evaluate_local()
//******************************************************************************
// LAST MODIFIED : 17 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	// should not come here - handled by overloading Variable::evaluate
	Assert(false,std::logic_error(
		"Variable_composition::evaluate_local.  "
		"Should not come here"));
	
	return (0);
}

void Variable_composition::evaluate_derivative_local(Matrix&,
	std::list<Variable_input_handle>&)
//******************************************************************************
// LAST MODIFIED : 18 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	// should not come here - handled by overloading Variable::evaluate_derivative
	Assert(false,std::logic_error(
		"Variable_composition::evaluate_derivative_local.  "
		"Should not come here"));
}

Variable_handle Variable_composition::get_input_value_local(
	const Variable_input_handle& input)
//******************************************************************************
// LAST MODIFIED : 19 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	std::list<Variable_input_value_handle>::iterator
		input_source_iterator=input_source_list.begin();
	Variable_handle result;
	Variable_size_type i=input_source_list.size();

	result=dependent_variable->get_input_value(input);
	while ((i>0)&&!result)
	{
		result=((*input_source_iterator)->value())->get_input_value(input);
		input_source_iterator++;
		i--;
	}

	return (result);
}

int Variable_composition::set_input_value_local(
	const Variable_input_handle& input,const Variable_handle& value)
//******************************************************************************
// LAST MODIFIED : 19 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	int return_code;
	std::list<Variable_input_value_handle>::iterator
		input_source_iterator=input_source_list.begin();
	Variable_size_type i=input_source_list.size();

	return_code=dependent_variable->set_input_value(input,value);
	while ((i>0)&&(0==return_code))
	{
		return_code=((*input_source_iterator)->value())->set_input_value(input,
			value);
		input_source_iterator++;
		i--;
	}

	return (return_code);
}

string_handle Variable_composition::get_string_representation_local()
//******************************************************************************
// LAST MODIFIED : 19 November 2003
//
// DESCRIPTION :
//???DB.  Overload << instead of get_string_representation?
//==============================================================================
{
	std::list<Variable_input_value_handle>::iterator
		input_source_iterator=input_source_list.begin();
	std::ostringstream out;
	string_handle return_string;
	Variable_size_type i=input_source_list.size();

	if (return_string=new std::string)
	{
		out << *(dependent_variable->get_string_representation()) << "(";
		while (i>0)
		{
			out << *(((*input_source_iterator)->value())->
				get_string_representation());
			input_source_iterator++;
			i--;
			if (i>0)
			{
				out << ",";
			}
		}
		out << ")";
		*return_string=out.str();
	}

	return (return_string);
}
