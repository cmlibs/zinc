//******************************************************************************
// FILE : function_derivative_matrix.cpp
//
// LAST MODIFIED : 14 January 2005
//
// DESCRIPTION :
//==============================================================================

#include <sstream>

#include "computed_variable/function_derivative_matrix.hpp"
#include "computed_variable/function_matrix.hpp"
#include "computed_variable/function_variable_composite.hpp"
#include "computed_variable/function_variable_matrix.hpp"
#include "computed_variable/function_variable_value_scalar.hpp"

// module typedefs
// ===============

typedef boost::intrusive_ptr< Function_matrix<Scalar> >
	Function_matrix_scalar_handle;

typedef boost::intrusive_ptr< Function_variable_matrix<Scalar> >
	Function_variable_matrix_scalar_handle;

// module classes
// ==============

// class Function_derivative_matrix_get_matrix_functor
// ---------------------------------------------------

class Function_derivative_matrix_get_matrix_functor
//******************************************************************************
// LAST MODIFIED : 13 August 2004
//
// DESCRIPTION :
//==============================================================================
{
	public:
		Function_derivative_matrix_get_matrix_functor(Function_size_type& index,
			std::list<Matrix>::reverse_iterator& matrix_reverse_iterator,
			std::list<Matrix>& matrices,
			std::list<Function_variable_handle>& partial_independent_variables):
			matrix_reverse_iterator(matrix_reverse_iterator),
			partial_independent_variable_iterator(partial_independent_variables.
			rbegin()),index(index),offset(1+matrices.size())
		{
			matrix_reverse_iterator=matrices.rbegin();
			index=partial_independent_variables.size();
		};
		int operator() (Function_variable_handle& independent_variable)
		{
			bool found;

			offset /=2;
			if (index>0)
			{
				found=equivalent(*partial_independent_variable_iterator,
					independent_variable);
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
					matrix_reverse_iterator++;
				}
			}

			return (0);
		}
	private:
		std::list<Matrix>::reverse_iterator& matrix_reverse_iterator;
		std::list<Function_variable_handle>::reverse_iterator
			partial_independent_variable_iterator;
		Function_size_type& index;
		Function_size_type offset;
};


// class Function_variable_matrix_derivative_matrix
// ------------------------------------------------

// forward declaration so that can use _handle
class Function_variable_matrix_derivative_matrix;
typedef boost::intrusive_ptr<Function_variable_matrix_derivative_matrix>
	Function_variable_matrix_derivative_matrix_handle;

class Function_variable_matrix_derivative_matrix :
	public Function_variable_matrix<Scalar>
//******************************************************************************
// LAST MODIFIED : 22 November 2004
//
// DESCRIPTION :
// <column> and <row> start from one when referencing a matrix entry.  Zero
// indicates all.
//==============================================================================
{
	friend class Function_derivative_matrix;
	public:
		// constructor
		Function_variable_matrix_derivative_matrix(
			const Function_derivative_matrix_handle function_derivative_matrix,
			const std::list<Function_variable_handle>& partial_independent_variables):
			Function_variable_matrix<Scalar>(function_derivative_matrix,0,0),
			partial_independent_variables(partial_independent_variables)
		{
			if (function_derivative_matrix)
			{
				if (0<(this->partial_independent_variables).size())
				{
					Function_size_type index;

					std::for_each(
						(function_derivative_matrix->independent_variables).rbegin(),
						(function_derivative_matrix->independent_variables).rend(),
						Function_derivative_matrix_get_matrix_functor(index,
						matrix_reverse_iterator,function_derivative_matrix->matrices,
						this->partial_independent_variables));
					if (0!=index)
					{
						(this->partial_independent_variables).clear();
						matrix_reverse_iterator=
							(function_derivative_matrix->matrices).rend();
					}
				}
				else
				{
					matrix_reverse_iterator=(function_derivative_matrix->matrices).rend();
				}
			}
			else
			{
				(this->partial_independent_variables).clear();
			}
		};
		Function_variable_matrix_derivative_matrix(
			const Function_derivative_matrix_handle function_derivative_matrix,
			const std::list<Function_variable_handle>& partial_independent_variables,
			const Function_size_type row,const Function_size_type column):
			Function_variable_matrix<Scalar>(function_derivative_matrix,row,column),
			partial_independent_variables(partial_independent_variables)
		{
			if (function_derivative_matrix)
			{
				if (0<partial_independent_variables.size())
				{
					Function_size_type index;

					std::for_each(
						(function_derivative_matrix->independent_variables).rbegin(),
						(function_derivative_matrix->independent_variables).rend(),
						Function_derivative_matrix_get_matrix_functor(index,
						matrix_reverse_iterator,function_derivative_matrix->matrices,
						this->partial_independent_variables));
					if (0==index)
					{
						if (this->row_private>matrix_reverse_iterator->size1())
						{
							this->row_private=0;
						}
						if (this->column_private>matrix_reverse_iterator->size2())
						{
							this->column_private=0;
						}
						if ((0!=this->row_private)&&(0!=this->column_private))
						{
							value_private=Function_variable_value_handle(
								new Function_variable_value_specific<Scalar>(
								Function_variable_matrix_set_value_function<Scalar>));
						}
					}
					else
					{
						(this->partial_independent_variables).clear();
						matrix_reverse_iterator=
							(function_derivative_matrix->matrices).rend();
						this->row_private=0;
						this->column_private=0;
					}
				}
				else
				{
					matrix_reverse_iterator=(function_derivative_matrix->matrices).rend();
					this->row_private=0;
					this->column_private=0;
				}
			}
			else
			{
				(this->partial_independent_variables).clear();
				this->row_private=0;
				this->column_private=0;
			}
		};
		// destructor
		~Function_variable_matrix_derivative_matrix(){};
	// inherited
	public:
		Function_variable_handle clone() const
		{
			Function_derivative_matrix_handle function_derivative_matrix=
				boost::dynamic_pointer_cast<Function_derivative_matrix,Function>(
				function());
			Function_size_type i;
			Function_variable_handle result(0);

			if (function_derivative_matrix)
			{
				std::list<Function_variable_handle>
					local_partial_independent_variables(0);
				std::list<Function_variable_handle>::const_iterator iterator;

				if (0<(i=partial_independent_variables.size()))
				{
					iterator=partial_independent_variables.begin();
					while (i>0)
					{
						if (*iterator)
						{
							local_partial_independent_variables.push_back(
								(*iterator)->clone());
						}
						else
						{
							local_partial_independent_variables.push_back(
								Function_variable_handle(0));
						}
						iterator++;
						i--;
					}
				}
				// constructor sets value_private
				result=Function_variable_handle(
					new Function_variable_matrix_derivative_matrix(
					function_derivative_matrix,local_partial_independent_variables,
					row_private,column_private));
			}

			return (result);
		};
		string_handle get_string_representation()
		{
			Function_derivative_matrix_handle function_derivative_matrix=
				boost::dynamic_pointer_cast<Function_derivative_matrix,Function>(
				function());
			string_handle return_string(0),temp_string;

			if (0==partial_independent_variables.size())
			{
				if (function_derivative_matrix)
				{
					return_string=function_derivative_matrix->get_string_representation();
				}
			}
			else
			{
				if (return_string=new std::string)
				{
					std::list<Function_variable_handle>::iterator independent_variable;
					std::ostringstream out;

					out << "d" << partial_independent_variables.size() << "(";
					if (function_derivative_matrix&&
						(function_derivative_matrix->dependent_variable))
					{
						if (temp_string=function_derivative_matrix->dependent_variable->
							get_string_representation())
						{
							out << *temp_string;
							delete temp_string;
						}
					}
					out << ")/d(";
					independent_variable=partial_independent_variables.begin();
					while (independent_variable!=partial_independent_variables.end())
					{
						if (temp_string=(*independent_variable)->
							get_string_representation())
						{
							out << *temp_string;
							delete temp_string;
						}
						independent_variable++;
						if (independent_variable!=partial_independent_variables.end())
						{
							out << ",";
						}
					}
					out << ")" << partial_independent_variables.size();
					if ((0!=row_private)||(0!=column_private))
					{
						out << "[";
						if (0!=row_private)
						{
							out << row_private;
						}
						else
						{
							out << "*";
						}
						out << ",";
						if (0!=column_private)
						{
							out << column_private;
						}
						else
						{
							out << "*";
						}
						out << "]";
					}
					*return_string=out.str();
				}
			}

			return (return_string);
		};
		Function_variable_matrix_scalar_handle operator()(
			Function_size_type row,Function_size_type column) const
		{
			Function_variable_matrix_scalar_handle result(0);

			if ((row<=number_of_rows())&&(column<=number_of_columns()))
			{
				result=Function_variable_matrix_scalar_handle(
					new Function_variable_matrix_derivative_matrix(
					boost::dynamic_pointer_cast<Function_derivative_matrix,Function>(
					function_private),partial_independent_variables,row,column));
			}

			return (result);
		};
		Function_size_type number_of_rows() const
		{
			Function_size_type result;

			result=0;
			if (this)
			{
				result=(matrix_reverse_iterator->size1)();
			}

			return (result);
		};
		Function_size_type number_of_columns() const
		{
			Function_size_type result;

			result=0;
			if (this)
			{
				result=(matrix_reverse_iterator->size2)();
			}

			return (result);
		};
		bool get_entry(Scalar& value) const
		{
			bool result;
			result=false;
			if (this)
			{
				value=(*matrix_reverse_iterator)(row_private,column_private);
				result=true;
			}

			return (result);
		};
	private:
		bool equality_atomic(const Function_variable_handle& variable) const
		{
			bool result;
			Function_variable_matrix_derivative_matrix_handle
				variable_derivative_matrix;

			result=false;
			if (variable_derivative_matrix=boost::dynamic_pointer_cast<
				Function_variable_matrix_derivative_matrix,Function_variable>(variable))
			{
				if (equivalent(function(),variable_derivative_matrix->function())&&
					(row_private==variable_derivative_matrix->row_private)&&
					(column_private==variable_derivative_matrix->column_private))
				{
					std::list<Function_variable_handle>::const_iterator
						variable_iterator_1,variable_iterator_1_end,variable_iterator_2,
						variable_iterator_2_end;

					variable_iterator_1=partial_independent_variables.begin();
					variable_iterator_1_end=partial_independent_variables.end();
					variable_iterator_2=
						variable_derivative_matrix->partial_independent_variables.begin();
					variable_iterator_2_end=
						variable_derivative_matrix->partial_independent_variables.end();
					while ((variable_iterator_1!=variable_iterator_1_end)&&
						(variable_iterator_2!=variable_iterator_2_end)&&
						equivalent(*variable_iterator_1,*variable_iterator_2))
					{
						variable_iterator_1++;
						variable_iterator_2++;
					}
					result=((variable_iterator_1==variable_iterator_1_end)&&
						(variable_iterator_2==variable_iterator_2_end));
				}
			}

			return (result);
		};
	private:
		// copy constructor
		Function_variable_matrix_derivative_matrix(
			const Function_variable_matrix_derivative_matrix& variable):
			Function_variable_matrix<Scalar>(variable),
			partial_independent_variables(variable.partial_independent_variables),
			matrix_reverse_iterator(variable.matrix_reverse_iterator){};
	private:
		std::list<Function_variable_handle> partial_independent_variables;
		std::list<Matrix>::reverse_iterator matrix_reverse_iterator;
};


// global classes
// ==============

// class Function_derivative_matrix
// --------------------------------

string_handle Function_derivative_matrix::get_string_representation()
//******************************************************************************
// LAST MODIFIED : 3 March 2004
//
// DESCRIPTION :
//==============================================================================
{
	string_handle return_string,temp_string;

	if (return_string=new std::string)
	{
		std::list<Function_variable_handle>::iterator independent_variable;
		std::ostringstream out;

		out << "derivative_matrix(";
		if (dependent_variable)
		{
			if (temp_string=dependent_variable->get_string_representation())
			{
				out << *temp_string;
				delete temp_string;
			}
		}
		out << ";";
		independent_variable=independent_variables.begin();
		while (independent_variable!=independent_variables.end())
		{
			if (temp_string=(*independent_variable)->get_string_representation())
			{
				out << *temp_string;
				delete temp_string;
			}
			independent_variable++;
			if (independent_variable!=independent_variables.end())
			{
				out << ",";
			}
		}
		out << ")";
		*return_string=out.str();
	}

	return (return_string);
}

Function_variable_handle Function_derivative_matrix::input()
//******************************************************************************
// LAST MODIFIED : 19 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_variable_handle result(0);
	std::list<Function_variable_handle> composite_variables_list(0);
	std::list<Function_variable_handle>::iterator independent_variable_iterator;
	std::list< std::list<Function_variable_handle> >
		matrix_independent_variables_list(0);
	std::list< std::list<Function_variable_handle> >::iterator last,
		matrix_independent_variables_iterator;

	for (independent_variable_iterator=independent_variables.begin();
		independent_variable_iterator!=independent_variables.end();
		independent_variable_iterator++)
	{
		Function_variable_handle independent_variable=
			*independent_variable_iterator;
		std::list<Function_variable_handle> matrix_independent_variables(0);

		matrix_independent_variables.push_back(independent_variable);
		composite_variables_list.push_back(Function_variable_handle(
			new Function_variable_matrix_derivative_matrix(
			Function_derivative_matrix_handle(this),matrix_independent_variables)));
		matrix_independent_variables_list.push_back(matrix_independent_variables);
		last=matrix_independent_variables_list.end();
		last--;
		for (matrix_independent_variables_iterator=
			matrix_independent_variables_list.begin();
			matrix_independent_variables_iterator!=last;
			matrix_independent_variables_iterator++)
		{
			matrix_independent_variables= *matrix_independent_variables_iterator;
			matrix_independent_variables.push_back(independent_variable);
			composite_variables_list.push_back(Function_variable_handle(
				new Function_variable_matrix_derivative_matrix(
				Function_derivative_matrix_handle(this),
				matrix_independent_variables)));
		}
	}

	return (Function_variable_handle(new Function_variable_composite(
		Function_derivative_matrix_handle(this),composite_variables_list)));
}

Function_variable_handle Function_derivative_matrix::output()
//******************************************************************************
// LAST MODIFIED : 19 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (input());
}

bool Function_derivative_matrix::operator==(const Function& function) const
//******************************************************************************
// LAST MODIFIED : 13 August 2004
//
// DESCRIPTION :
// Equality operator.
//==============================================================================
{
	bool result;

	result=false;
	if (this)
	{
		try
		{
			const Function_derivative_matrix& function_derivative_matrix=
				dynamic_cast<const Function_derivative_matrix&>(function);

			if (equivalent(dependent_variable,
				function_derivative_matrix.dependent_variable))
			{
				std::list<Function_variable_handle>::const_iterator
					variable_iterator_1,variable_iterator_1_end,variable_iterator_2,
					variable_iterator_2_end;

				variable_iterator_1=independent_variables.begin();
				variable_iterator_1_end=independent_variables.end();
				variable_iterator_2=
					function_derivative_matrix.independent_variables.begin();
				variable_iterator_2_end=
					function_derivative_matrix.independent_variables.end();
				while ((variable_iterator_1!=variable_iterator_1_end)&&
					(variable_iterator_2!=variable_iterator_2_end)&&
					equivalent(*variable_iterator_1,*variable_iterator_2))
				{
					variable_iterator_1++;
					variable_iterator_2++;
				}
				result=((variable_iterator_1==variable_iterator_1_end)&&
					(variable_iterator_2==variable_iterator_2_end));
			}
		}
		catch (std::bad_cast)
		{
			// do nothing
		}
	}

	return (result);
}

Function_handle Function_derivative_matrix::matrix(
	std::list<Function_variable_handle>& partial_independent_variables)
//******************************************************************************
// LAST MODIFIED : 6 August 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_derivative_matrix_handle function_derivative_matrix;
	Function_handle result(0);
	Function_variable_matrix_derivative_matrix_handle matrix_variable(new
		Function_variable_matrix_derivative_matrix(
		Function_derivative_matrix_handle(this),partial_independent_variables));

	if (matrix_variable&&(function_derivative_matrix=boost::dynamic_pointer_cast<
		Function_derivative_matrix,Function>(matrix_variable->function()))&&
		(matrix_variable->matrix_reverse_iterator!=
		(function_derivative_matrix->matrices).rend()))
	{
		result=Function_handle(new Function_matrix<Scalar>(
			*(matrix_variable->matrix_reverse_iterator)));
	}

	return (result);
}

Function_derivative_matrix_handle Function_derivative_matrix_compose(
	const Function_variable_handle& dependent_variable,
	const Function_derivative_matrix_handle& derivative_f,
	const Function_derivative_matrix_handle& derivative_g)
//******************************************************************************
// LAST MODIFIED : 13 August 2004
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
	Function_derivative_matrix_handle result(0);
	Function_size_type i;

	if (derivative_f&&derivative_g&&
		(0<(i=(derivative_f->independent_variables).size())))
	{
		std::list<Function_variable_handle>::iterator independent_variable_iterator;
		Function_variable_handle last_independent_variable;

		// check that all derivative_f independent variables are the same
		if (last_independent_variable=(derivative_f->independent_variables).back())
		{
			independent_variable_iterator=
				(derivative_f->independent_variables).begin();
			i--;
			while ((i>0)&&(*independent_variable_iterator)&&
				equivalent(*independent_variable_iterator,last_independent_variable))
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
			Function_size_type column_number_result,index_result,j,k,l,
				number_of_columns_f,number_of_columns_result,
				number_of_intermediate_values,number_of_matrices,number_of_rows,order,
				offset_f,offset_g,order_result,p,q,r,row_number,s;
			Function_size_type *column_numbers_g,*index_f,*index_g,*mapping_g,
				*mapping_result,*numbers_of_independent_values,*product_orders,*order_g,
				*sub_order_g;
			std::list<Matrix>::iterator *matrices_g;

			// initialize
			number_of_rows=((derivative_f->matrices).front()).size1();
			number_of_intermediate_values=((derivative_g->matrices).front()).size1();
			independent_variable_iterator=
				(derivative_g->independent_variables).begin();
			order=(derivative_g->independent_variables).size();
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
			matrices_g=new std::list<Matrix>::iterator[order+1];
			if (not_used&&column_numbers_g&&index_f&&index_g&&mapping_g&&
				mapping_result&&numbers_of_independent_values&&product_orders&&order_g&&
				sub_order_g&&matrices_g)
			{
				number_of_matrices=1;
				matrix_g=(derivative_g->matrices).begin();
				for (i=0;i<order;i++)
				{
					Assert(number_of_matrices<=(derivative_g->matrices).size(),
						std::logic_error("Function_derivative_matrix_compose.  "
						"Invalid number of matrices for derivative g"));
					numbers_of_independent_values[i]=matrix_g->size2();
					for (j=number_of_matrices;j>0;j--)
					{
						Matrix new_matrix(number_of_rows,matrix_g->size2());

						matrices_result.push_back(new_matrix);
						matrix_g++;
					}
					number_of_matrices *= 2;
				}
				number_of_matrices -= 1;
				Assert((number_of_matrices==(derivative_f->matrices).size())&&
					(number_of_matrices==(derivative_g->matrices).size()),
					std::logic_error("Function_derivative_matrix_compose.  "
					"Invalid number of matrices"));
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
				result=Function_derivative_matrix_handle(new Function_derivative_matrix(
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

	return (result);
}

Function_handle Function_derivative_matrix::inverse()
//******************************************************************************
// LAST MODIFIED : 13 August 2004
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
	Function_derivative_matrix_handle derivative(this);
	Function_derivative_matrix_handle result(0);
	Function_size_type i;

	if (derivative&&(0<(i=(derivative->independent_variables).size()))&&
		dependent_variable)
	{
		std::list<Function_variable_handle> inverse_independent_variables;
		std::list<Function_variable_handle>::iterator independent_variable_iterator;
		Function_variable_handle inverse_dependent_variable,
			inverse_independent_variable;

		// check that all derivative independent variables are the same
		if ((inverse_independent_variable=derivative->dependent_variable)&&
			(inverse_dependent_variable=(derivative->independent_variables).front()))
		{
			independent_variable_iterator=(derivative->independent_variables).begin();
			while ((i>0)&&(*independent_variable_iterator)&&
				equivalent(*independent_variable_iterator,inverse_dependent_variable))
			{
				inverse_independent_variables.push_back(inverse_independent_variable);
				independent_variable_iterator++;
				i--;
			}
		}
		if ((0==i)&&(((derivative->matrices).front()).size1()==
			((derivative->matrices).front()).size2()))
		{
			bool found;
			bool *not_used;
			Scalar product,sum;
			std::list<Matrix> matrices_result(0);
			std::list<Matrix>::iterator matrix_f,matrix_g;
			Function_size_type column_number_result,j,k,l,number_of_columns_f,
				number_of_columns_result,number_of_rows,number_of_steps,offset,offset_f,
				offset_g,order_result,p,q,r,row_number,s,step;
			Function_size_type number_of_matrices=(derivative->matrices).size();
			Function_size_type order=(derivative->independent_variables).size();
			Function_size_type *column_numbers_g,*index_f,*index_g,*mapping_g,
				*product_orders,*order_g,*sub_order_g;
			std::list<Matrix>::iterator *matrices_g;
			ublas::vector<Function_size_type> matrix_orders(number_of_matrices);
			ublas::vector<Matrix> order_matrices(order);

			// initialize
			number_of_rows=((derivative->matrices).front()).size1();
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
				Function_matrix_scalar_handle matrix_inverse_handle(0);

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
				if (matrix_inverse_handle=Function_matrix_scalar_handle(
					new Function_matrix<Scalar>(matrix_inverse)))
				{
					if (matrix_inverse_handle=Function_matrix<Scalar>(
						(derivative->matrices).front()).solve(matrix_inverse_handle))
					{
						for (i=0;i<number_of_rows;i++)
						{
							for (j=0;j<number_of_rows;j++)
							{
								matrix_inverse(i,j)=(*matrix_inverse_handle)(i+1,j+1);
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
				result=Function_derivative_matrix_handle(new Function_derivative_matrix(
					inverse_dependent_variable,inverse_independent_variables,
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

#if defined (EVALUATE_RETURNS_VALUE)
Function_handle Function_derivative_matrix::evaluate(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 23 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (get_value(atomic_variable));
}
#else // defined (EVALUATE_RETURNS_VALUE)
bool Function_derivative_matrix::evaluate(Function_variable_handle)
//******************************************************************************
// LAST MODIFIED : 14 January 2005
//
// DESCRIPTION :
//==============================================================================
{
	return (true);
}
#endif // defined (EVALUATE_RETURNS_VALUE)

bool Function_derivative_matrix::evaluate_derivative(Scalar& derivative,
	Function_variable_handle atomic_variable,
	std::list<Function_variable_handle>& atomic_independent_variables)
//******************************************************************************
// LAST MODIFIED : 13 August 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	Function_variable_matrix_derivative_matrix_handle
		atomic_dependent_variable,atomic_independent_variable;

	result=false;
	if (this)
	{
		if ((atomic_dependent_variable=boost::dynamic_pointer_cast<
			Function_variable_matrix_derivative_matrix,Function_variable>(
			atomic_variable))&&
			equivalent(Function_handle(this),atomic_dependent_variable->function())&&
			(1==atomic_dependent_variable->number_differentiable()))
		{
			result=true;
			if ((1==atomic_independent_variables.size())&&
				(atomic_independent_variable=boost::dynamic_pointer_cast<
				Function_variable_matrix_derivative_matrix,Function_variable>(
				atomic_independent_variables.front()))&&
				equivalent(atomic_dependent_variable,atomic_independent_variable))
			{
				derivative=1;
			}
			else
			{
				derivative=0;
			}
		}
	}

	return (result);
}

bool Function_derivative_matrix::set_value(
	Function_variable_handle atomic_variable,
	Function_variable_handle atomic_value)
//******************************************************************************
// LAST MODIFIED : 1 December 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	Function_variable_matrix_derivative_matrix_handle
		atomic_derivative_matrix_variable;
	Function_variable_value_scalar_handle value_scalar;

	result=false;
	if (this)
	{
		if ((atomic_derivative_matrix_variable=boost::dynamic_pointer_cast<
			Function_variable_matrix_derivative_matrix,Function_variable>(
			atomic_variable))&&equivalent(Function_handle(this),
			atomic_derivative_matrix_variable->function())&&
			atomic_value&&(atomic_value->value())&&
			(std::string("Scalar")==(atomic_value->value())->type())&&
			(value_scalar=boost::dynamic_pointer_cast<Function_variable_value_scalar,
			Function_variable_value>(atomic_value->value())))
		{
			result=value_scalar->set(
				(*(atomic_derivative_matrix_variable->matrix_reverse_iterator))(
				(atomic_derivative_matrix_variable->row_private),
				(atomic_derivative_matrix_variable->column_private)),atomic_value);
		}
		if (result)
		{
			set_not_evaluated();
		}
	}

	return (result);
}

Function_handle Function_derivative_matrix::get_value(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 13 August 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle result(0);
	Function_variable_matrix_derivative_matrix_handle
		atomic_variable_derivative_matrix;
	Matrix result_matrix(1,1);

	if (this)
	{
		if (equivalent(Function_handle(this),(atomic_variable->function)())&&
			(atomic_variable_derivative_matrix=boost::dynamic_pointer_cast<
			Function_variable_matrix_derivative_matrix,Function_variable>(
			atomic_variable))&&atomic_variable_derivative_matrix->get_entry(
			result_matrix(0,0)))
		{
			result=Function_handle(new Function_matrix<Scalar>(result_matrix));
		}
	}

	return (result);
}

Function_derivative_matrix::Function_derivative_matrix(
	const Function_variable_handle& dependent_variable,
	const std::list<Function_variable_handle>& independent_variables,
	const std::list<Matrix>& matrices) : Function(),
	dependent_variable(dependent_variable),
	independent_variables(independent_variables),matrices(matrices)
//******************************************************************************
// LAST MODIFIED : 13 February 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	//???DB.  Check that consistent?
}

Function_derivative_matrix::Function_derivative_matrix(
	const Function_variable_handle& dependent_variable,
	const std::list<Function_variable_handle>& independent_variables):
	Function(),dependent_variable(dependent_variable),
	independent_variables(independent_variables),matrices()
//******************************************************************************
// LAST MODIFIED : 12 November 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	Function_size_type number_of_dependent_values=dependent_variable->
		number_differentiable();
	std::list<Function_variable_handle>::const_iterator
		independent_variable_iterator;
	std::list< std::list<Function_variable_handle> > matrix_independent_variables;

#if defined (DEBUG)
	//???debug
	std::cout << "enter Function_derivative_matrix::Function_derivative_matrix.  " << std::endl;
#endif // defined (DEBUG)
	for (independent_variable_iterator=independent_variables.begin();
		independent_variable_iterator!=independent_variables.end();
		independent_variable_iterator++)
	{
		Function_variable_handle independent_variable=
			*independent_variable_iterator;
		Function_size_type number_of_independent_values=independent_variable->
			number_differentiable();
		std::list<Matrix>::iterator matrix_iterator,last;
		std::list< std::list<Function_variable_handle> >::iterator
			matrix_independent_variables_iterator;

		// calculate the derivative of dependent variable with respect to
		//   independent variable and add to matrix list
		{
			Function_size_type row,column;
			Function_variable_iterator atomic_dependent_variable_iterator(0);
			Matrix new_matrix(number_of_dependent_values,
				number_of_independent_values);
			std::list<Function_variable_handle> new_matrix_independent_variables;

			new_matrix_independent_variables.push_back(independent_variable);
			row=0;
			for (
				atomic_dependent_variable_iterator=dependent_variable->begin_atomic();
				atomic_dependent_variable_iterator!=dependent_variable->end_atomic();
				atomic_dependent_variable_iterator++)
			{
				Function_variable_handle atomic_dependent_variable=
					*atomic_dependent_variable_iterator;

				if (1==atomic_dependent_variable->number_differentiable())
				{
					Function_variable_iterator atomic_independent_variable_iterator(0);

					Assert(atomic_dependent_variable&&
						(atomic_dependent_variable->function()),std::logic_error(
						"Function_derivative_matrix::Function_derivative_matrix.  "
						"Atomic variable missing function()"));
					column=0;
					for (atomic_independent_variable_iterator=independent_variable->
						begin_atomic();atomic_independent_variable_iterator!=
						independent_variable->end_atomic();
						atomic_independent_variable_iterator++)
					{
						Function_variable_handle atomic_independent_variable=
							*atomic_independent_variable_iterator;

						if (1==atomic_independent_variable->number_differentiable())
						{
							std::list<Function_variable_handle>
								new_matrix_atomic_independent_variables;

#if defined (DEBUG)
							//???debug
							std::cout << "  " << row << " " << column << std::endl;
#endif // defined (DEBUG)
							new_matrix_atomic_independent_variables.push_back(
								atomic_independent_variable);
							if (!((atomic_dependent_variable->function())->
								evaluate_derivative(new_matrix(row,column),
								atomic_dependent_variable,
								new_matrix_atomic_independent_variables)))
							{
								//???debug
								std::cout << "throw Function_derivative_matrix::Construction_exception" << std::endl;
								std::cout << row << " " << column << std::endl;
								std::cout << *(atomic_dependent_variable->get_string_representation()) << " ";
								if (atomic_dependent_variable->function())
								{
									std::cout << *((atomic_dependent_variable->function())->get_string_representation());
								}
								else
								{
									std::cout << "no function";
								}
								std::cout << std::endl;
								std::cout << new_matrix_atomic_independent_variables.size() << " " << *((new_matrix_atomic_independent_variables.front())->get_string_representation()) << " ";
								if ((new_matrix_atomic_independent_variables.front())->function())
								{
									std::cout << *(((new_matrix_atomic_independent_variables.front())->function())->get_string_representation());
								}
								else
								{
									std::cout << "no function";
								}
								std::cout << std::endl;
#if defined (DEBUG)
#endif // defined (DEBUG)
								throw Function_derivative_matrix::Construction_exception();
							}
							column++;
						}
					}
					row++;
				}
			}
			matrices.push_back(new_matrix);
			matrix_independent_variables.push_back(new_matrix_independent_variables);
		}
		last=matrices.end();
		last--;
		matrix_independent_variables_iterator=matrix_independent_variables.begin();
		for (matrix_iterator=matrices.begin();matrix_iterator!=last;
			matrix_iterator++)
		{
			Function_size_type row,column;
			Function_variable_iterator atomic_dependent_variable_iterator(0);
			Matrix& matrix= *matrix_iterator;
			Matrix new_matrix((matrix.size1)(),
				number_of_independent_values*(matrix.size2)());
			std::list<Function_variable_handle> new_matrix_independent_variables;

			new_matrix_independent_variables= *matrix_independent_variables_iterator;
			new_matrix_independent_variables.push_back(independent_variable);
			//???DB.  Generalize code above to have a set up matrix function?
			row=0;
			for (
				atomic_dependent_variable_iterator=dependent_variable->begin_atomic();
				atomic_dependent_variable_iterator!=dependent_variable->end_atomic();
				atomic_dependent_variable_iterator++)
			{
				Function_variable_handle atomic_dependent_variable=
					*atomic_dependent_variable_iterator;

				if (1==atomic_dependent_variable->number_differentiable())
				{
					bool no_derivative;
					Function_size_type i;
					std::vector<Function_variable_iterator>
						atomic_independent_variable_iterators(
						new_matrix_independent_variables.size());
					std::list<Function_variable_handle>::iterator
						new_matrix_independent_variables_iterator;

					Assert(atomic_dependent_variable&&
						(atomic_dependent_variable->function()),std::logic_error(
						"Function_derivative_matrix::Function_derivative_matrix.  "
						"Atomic variable missing function()"));
					new_matrix_independent_variables_iterator=
						new_matrix_independent_variables.begin();
					i=0;
					no_derivative=false;
					while ((new_matrix_independent_variables_iterator!=
						new_matrix_independent_variables.end())&&!no_derivative)
					{
						atomic_independent_variable_iterators[i]=
							(*new_matrix_independent_variables_iterator)->begin_atomic();
						while ((atomic_independent_variable_iterators[i]!=
							(*new_matrix_independent_variables_iterator)->end_atomic())&&
							(1!=(*(atomic_independent_variable_iterators[i]))->
							number_differentiable()))
						{
							atomic_independent_variable_iterators[i]++;
						}
						if (atomic_independent_variable_iterators[i]==
							(*new_matrix_independent_variables_iterator)->end_atomic())
						{
							no_derivative=true;
						}
						else
						{
							new_matrix_independent_variables_iterator++;
						}
						i++;
					}
					if (new_matrix_independent_variables_iterator==
						new_matrix_independent_variables.end())
					{
						column=0;
						do
						{
							std::list<Function_variable_handle>
								new_matrix_atomic_independent_variables(0);

							i=new_matrix_independent_variables.size();
							while (i>0)
							{
								i--;
								new_matrix_atomic_independent_variables.push_front(
									*(atomic_independent_variable_iterators[i]));
							}
							if (!((atomic_dependent_variable->function())->
								evaluate_derivative(new_matrix(row,column),
								atomic_dependent_variable,
								new_matrix_atomic_independent_variables)))
							{
								//???debug
								std::cout << "throw Function_derivative_matrix::Construction_exception 2" << std::endl;
								std::cout << row << " " << column << std::endl;
								std::cout << *(atomic_dependent_variable->get_string_representation()) << " ";
								if (atomic_dependent_variable->function())
								{
									std::cout << *((atomic_dependent_variable->function())->get_string_representation());
								}
								else
								{
									std::cout << "no function";
								}
								std::cout << std::endl;
								std::cout << new_matrix_atomic_independent_variables.size() << " " << *((new_matrix_atomic_independent_variables.front())->get_string_representation()) << " ";
								if ((new_matrix_atomic_independent_variables.front())->function())
								{
									std::cout << *(((new_matrix_atomic_independent_variables.front())->function())->get_string_representation());
								}
								else
								{
									std::cout << "no function";
								}
								std::cout << std::endl;
#if defined (DEBUG)
#endif // defined (DEBUG)
								throw Function_derivative_matrix::Construction_exception();
							}
							// move to next column
							i=new_matrix_independent_variables.size();
							new_matrix_independent_variables_iterator=
								new_matrix_independent_variables.end();
							if (i>0)
							{
								i--;
								new_matrix_independent_variables_iterator--;
								atomic_independent_variable_iterators[i]++;
								while ((atomic_independent_variable_iterators[i]!=
									(*new_matrix_independent_variables_iterator)->end_atomic())&&
									(1!=(*(atomic_independent_variable_iterators[i]))->
									number_differentiable()))
								{
									atomic_independent_variable_iterators[i]++;
								}
								while ((i>0)&&
									((*new_matrix_independent_variables_iterator)->end_atomic()==
									atomic_independent_variable_iterators[i]))
								{
									atomic_independent_variable_iterators[i]=
										(*new_matrix_independent_variables_iterator)->
										begin_atomic();
									while ((atomic_independent_variable_iterators[i]!=
										(*new_matrix_independent_variables_iterator)->
										end_atomic())&&
										(1!=(*(atomic_independent_variable_iterators[i]))->
										number_differentiable()))
									{
										atomic_independent_variable_iterators[i]++;
									}
									i--;
									atomic_independent_variable_iterators[i]++;
									new_matrix_independent_variables_iterator--;
									while ((atomic_independent_variable_iterators[i]!=
										(*new_matrix_independent_variables_iterator)->
										end_atomic())&&
										(1!=(*(atomic_independent_variable_iterators[i]))->
										number_differentiable()))
									{
										atomic_independent_variable_iterators[i]++;
									}
								}
							}
							column++;
						} while ((new_matrix_independent_variables.front())->end_atomic()!=
							atomic_independent_variable_iterators[0]);
					}
					row++;
				}
			}
			matrices.push_back(new_matrix);
			matrix_independent_variables.push_back(new_matrix_independent_variables);
			matrix_independent_variables_iterator++;
		}
	}
#if defined (DEBUG)
	//???debug
	std::cout << "leave Function_derivative_matrix::Function_derivative_matrix.  " << std::endl;
#endif // defined (DEBUG)
}

Function_derivative_matrix::Function_derivative_matrix(
	const Function_derivative_matrix& derivative_matrix) :
	Function(derivative_matrix),
	dependent_variable(derivative_matrix.dependent_variable),
	independent_variables(derivative_matrix.independent_variables),
	matrices(derivative_matrix.matrices)
{}
//******************************************************************************
// LAST MODIFIED : 17 February 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================

Function_derivative_matrix& Function_derivative_matrix::operator=(
	const Function_derivative_matrix& derivative_matrix)
//******************************************************************************
// LAST MODIFIED : 17 February 2004
//
// DESCRIPTION :
// Assignment operator.
//==============================================================================
{
	dependent_variable=derivative_matrix.dependent_variable;
	independent_variables=derivative_matrix.independent_variables;
	matrices=derivative_matrix.matrices;

	return (*this);
}

Function_derivative_matrix::~Function_derivative_matrix()
//******************************************************************************
// LAST MODIFIED : 17 February 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	//???DB.  To be done
}
