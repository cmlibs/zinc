//******************************************************************************
// FILE : variable_matrix.cpp
//
// LAST MODIFIED : 9 February 2004
//
// DESCRIPTION :
//???DB.  Should be template?
//==============================================================================
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

#include "computed_variable/variable_base.hpp"

#include <new>
#include <sstream>
#include <string>

// to use lapack with ublas
#include <boost/numeric/bindings/traits/ublas_matrix.hpp>
#include <boost/numeric/bindings/traits/std_vector.hpp>
#include <boost/numeric/bindings/lapack/gesv.hpp>

namespace lapack = boost::numeric::bindings::lapack;

#include "computed_variable/variable_matrix.hpp"
#include "computed_variable/variable_vector.hpp"

// module classes
// ==============

// class Variable_input_matrix_values
// ----------------------------------

class Variable_input_matrix_values;

#if defined (USE_INTRUSIVE_SMART_POINTER)
typedef boost::intrusive_ptr<Variable_input_matrix_values>
	Variable_input_matrix_values_handle;
#elif defined (USE_SMART_POINTER)
typedef boost::shared_ptr<Variable_input_matrix_values>
	Variable_input_matrix_values_handle;
#else
typedef Variable_input_matrix_values * Variable_input_matrix_values_handle;
#endif

class Variable_input_matrix_values : public
#if defined (USE_VARIABLE_INPUT)
	Variable_input
#else // defined (USE_VARIABLE_INPUT)
	Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	friend class Variable_matrix;
	public:
		Variable_input_matrix_values(const Variable_matrix_handle& variable_matrix):
			variable_matrix(variable_matrix),indices(){};
		Variable_input_matrix_values(const Variable_matrix_handle& variable_matrix,
			Variable_size_type row,Variable_size_type column):variable_matrix(
			variable_matrix),indices(1)
		{
			indices[0].first=row;
			indices[0].second=column;
		};
		Variable_input_matrix_values(const Variable_matrix_handle& variable_matrix,
			const ublas::vector< std::pair<Variable_size_type,Variable_size_type> >&
			indices):variable_matrix(variable_matrix),indices(indices){};
		~Variable_input_matrix_values(){};
#if defined (USE_ITERATORS)
		// copy constructor
		Variable_input_matrix_values(
			const Variable_input_matrix_values& input_matrix_values):
#if defined (USE_VARIABLE_INPUT)
			Variable_input
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
			(),
			variable_matrix(input_matrix_values.variable_matrix),
			indices(input_matrix_values.indices){};
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			clone() const
		{
			return (Variable_input_matrix_values_handle(
				new Variable_input_matrix_values(*this)));
		}
		//???DB.  To be done
		virtual bool is_atomic();
#if defined (USE_ITERATORS_NESTED)
		virtual Iterator begin_atomic_inputs();
		virtual Iterator end_atomic_inputs();
#else // defined (USE_ITERATORS_NESTED)
#if defined (DO_NOT_USE_ITERATOR_TEMPLATES)
		virtual Variable_input_iterator begin_atomic_inputs();
		virtual Variable_input_iterator end_atomic_inputs();
#else // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
#if defined (USE_VARIABLE_INPUT)
		virtual Handle_iterator<Variable_input_handle> begin_atomic_inputs();
		virtual Handle_iterator<Variable_input_handle> end_atomic_inputs();
#else // defined (USE_VARIABLE_INPUT)
		virtual Handle_iterator<Variable_io_specifier_handle> begin_atomic();
		virtual Handle_iterator<Variable_io_specifier_handle> end_atomic();
#endif // defined (USE_VARIABLE_INPUT)
#endif // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
#endif // defined (USE_ITERATORS_NESTED)
#endif // defined (USE_ITERATORS)
		virtual Variable_size_type
#if defined (USE_ITERATORS)
			number_differentiable
#else // defined (USE_ITERATORS)
			size
#endif // defined (USE_ITERATORS)
			()
		{
			Variable_size_type result;

			result=indices.size();
			if (0==result)
			{
				result=variable_matrix->
#if defined (USE_ITERATORS)
					number_differentiable
#else // defined (USE_ITERATORS)
					size
#endif // defined (USE_VARIABLE_ITERATORS)
					();
			}

			return (result);
		};
		virtual bool operator==(const
#if defined (USE_VARIABLE_INPUT)
			Variable_input
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
			& input)
		{
			try
			{
				const Variable_input_matrix_values& input_matrix_values=
					dynamic_cast<const Variable_input_matrix_values&>(input);
				bool result;

				result=false;
				if ((variable_matrix==input_matrix_values.variable_matrix)&&
					(indices.size()==input_matrix_values.indices.size()))
				{
					int i=indices.size();

					result=true;
					while (result&&(i>0))
					{
						i--;
						if (!(indices[i]==input_matrix_values.indices[i]))
						{
							result=false;
						}
					}
				}

				return (result);
			}
			catch (std::bad_cast)
			{
				return (false);
			}
		};
#if defined (USE_SCALAR_MAPPING)
	private:
		virtual std::list< std::pair<Variable_size_type,Variable_size_type> >
			scalar_mapping_local(Variable_input_handle target)
		{
			std::list< std::pair<Variable_size_type,Variable_size_type> > result(0);
			const Variable_input_matrix_values_handle input_matrix_values=
#if defined (USE_SMART_POINTER)
				boost::dynamic_pointer_cast<Variable_input_matrix_values,
#if defined (USE_VARIABLE_INPUT)
				Variable_input
#else // defined (USE_VARIABLE_INPUT)
				Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
				>
#else /* defined (USE_SMART_POINTER) */
				dynamic_cast<Variable_input_matrix_values *>
#endif /* defined (USE_SMART_POINTER) */
				(target);
			Variable_size_type source_size,target_size;

			target_size=target->size();
			source_size=size();
			if (input_matrix_values)
			{
				if (variable_matrix==input_matrix_values->variable_matrix)
				{
					const Variable_size_type
						number_of_columns=variable_matrix->number_of_columns(),
						number_of_rows=variable_matrix->number_of_rows();

					if (0==indices.size())
					{
						if (0==(input_matrix_values->indices).size())
						{
							result.push_back(
								std::pair<Variable_size_type,Variable_size_type>(0,0));
						}
						else
						{
							Assert(target_size==(input_matrix_values->indices).size(),
								std::logic_error(
								"Variable_input_matrix_values::scalar_mapping_local.  "
								"Error in calculating target size 1"));
							
							Variable_size_type i,index,j,k;

							index=0;
							for (i=0;i<number_of_rows;i++)
							{
								for (j=0;j<number_of_columns;j++)
								{
									k=0;
									while ((k<target_size)&&
										!((i==(input_matrix_values->indices)[k].first)&&
										(j==(input_matrix_values->indices)[k].second)))
									{
										k++;
									}
									if ((0==index)||
										(index-(result.back()).first!=k-(result.back()).second))
									{
										result.push_back(std::pair<Variable_size_type,
											Variable_size_type>(index,k));
									}
									index++;
								}
							}
						}
					}
					else
					{
						Assert(source_size==indices.size(),std::logic_error(
							"Variable_input_matrix_values::scalar_mapping_local.  "
							"Error in calculating source size"));
						if (0==(input_matrix_values->indices).size())
						{
							Variable_size_type i,j;

							for (i=0;i<source_size;i++)
							{
								j=number_of_rows*indices[i].first+indices[i].second;
								if ((0==i)||(i-(result.back()).first!=j-(result.back()).second))
								{
									result.push_back(std::pair<Variable_size_type,
										Variable_size_type>(i,j));
								}
							}
						}
						else
						{
							Assert(target_size==(input_matrix_values->indices).size(),
								std::logic_error(
								"Variable_input_matrix_values::scalar_mapping_local.  "
								"Error in calculating target size 2"));

							Variable_size_type i,j;

							for (i=0;i<source_size;i++)
							{
								j=0;
								while ((j<target_size)&&
									!(
									(indices[i].first==(input_matrix_values->indices)[j].first)&&
									(indices[i].second==(input_matrix_values->indices)[j].second)
									))
								{
									j++;
								}
								if ((0==i)||(i-(result.back()).first!=j-(result.back()).second))
								{
									result.push_back(std::pair<Variable_size_type,
										Variable_size_type>(i,j));
								}
							}
						}
					}
				}
			}
			if (0==result.size())
			{
				result.push_back(std::pair<Variable_size_type,Variable_size_type>(
					0,target_size));
			}
			if (0<source_size)
			{
				result.push_back(std::pair<Variable_size_type,Variable_size_type>(
					source_size,target_size));
			}

			return (result);
		};
#endif // defined (USE_SCALAR_MAPPING)
	private:
		Variable_matrix_handle variable_matrix;
		ublas::vector< std::pair<Variable_size_type,Variable_size_type> > indices;
};

// global classes
// ==============

// class Variable_matrix
// ---------------------

Variable_matrix::Variable_matrix(Matrix& values):Variable(),values(values)
//******************************************************************************
// LAST MODIFIED : 23 October 2003
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
}

Variable_matrix::Variable_matrix(const Variable_matrix& variable_matrix):
	Variable(),values(variable_matrix.values)
//******************************************************************************
// LAST MODIFIED : 23 October 2003
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
}

Variable_matrix& Variable_matrix::operator=(
	const Variable_matrix& variable_matrix)
//******************************************************************************
// LAST MODIFIED : 23 October 2003
//
// DESCRIPTION :
// Assignment operator.
//==============================================================================
{
	this->values=variable_matrix.values;

	return (*this);
}

Variable_matrix::~Variable_matrix()
//******************************************************************************
// LAST MODIFIED : 23 October 2003
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	// do nothing
}

Scalar& Variable_matrix::operator()(Variable_size_type row,
	Variable_size_type column)
//******************************************************************************
// LAST MODIFIED : 10 December 2003
//
// DESCRIPTION :
//==============================================================================
{
	return (values(row,column));
}

Variable_size_type Variable_matrix::
#if defined (USE_ITERATORS)
	number_differentiable
#else // defined (USE_ITERATORS)
	size
#endif // defined (USE_VARIABLE_ITERATORS)
	() const
//******************************************************************************
// LAST MODIFIED : 21 January 2004
//
// DESCRIPTION :
//==============================================================================
{
	return ((values.size1())*(values.size2()));
}

#if defined (USE_ITERATORS)
#else // defined (USE_ITERATORS)
Vector *Variable_matrix::scalars()
//******************************************************************************
// LAST MODIFIED : 24 October 2003
//
// DESCRIPTION :
//==============================================================================
{
	Vector *values_vector;

	if (values_vector=new Vector(size()))
	{
		Variable_size_type i,j,k,number_of_columns=values.size2(),
			number_of_rows=values.size1();

		k=0;
		for (i=0;i<number_of_rows;i++)
		{
			for (j=0;j<number_of_columns;j++)
			{
				(*values_vector)[k]=values(i,j);
				k++;
			}
		}
	}

	return (values_vector);
}
#endif // defined (USE_VARIABLE_ITERATORS)

Variable_matrix_handle Variable_matrix::sub_matrix(Variable_size_type row_low,
	Variable_size_type row_high,Variable_size_type column_low,
	Variable_size_type column_high) const
//******************************************************************************
// LAST MODIFIED : 6 November 2003
//
// DESCRIPTION :
// Returns the specified sub-matrix.
//==============================================================================
{
	Variable_matrix_handle result;

	if ((0<row_low)&&(row_low<=row_high)&&(row_high<=values.size1())&&
		(0<column_low)&&(column_low<=column_high)&&(column_high<=values.size2()))
	{
		Variable_size_type i,j,number_of_columns=column_high-column_low+1,
			number_of_rows=row_high-row_low+1;
		Matrix temp_matrix(number_of_rows,number_of_columns);

		for (i=0;i<number_of_rows;i++)
		{
			for (j=0;j<number_of_columns;j++)
			{
				temp_matrix(i,j)=values(i+row_low-1,j+column_low-1);
			}
		}
		result=Variable_matrix_handle(new Variable_matrix(temp_matrix));
	}
	else
	{
		Matrix temp_matrix(0,0);

		result=Variable_matrix_handle(new Variable_matrix(temp_matrix));
	}

	return (result);
}

Variable_size_type Variable_matrix::number_of_rows() const
//******************************************************************************
// LAST MODIFIED : 6 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	return (values.size1());
}

Variable_size_type Variable_matrix::number_of_columns() const
//******************************************************************************
// LAST MODIFIED : 6 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	return (values.size2());
}

Variable_vector_handle Variable_matrix::solve(const Variable_handle& variable)
//******************************************************************************
// LAST MODIFIED : 21 January 2004
//
// DESCRIPTION :
//==============================================================================
{
	Vector *variable_scalars=0;
	Variable_vector_handle result(0);

	if (variable&&(variable_scalars=
#if defined (USE_ITERATORS)
		//???DB.  To be done
		0
#else // defined (USE_ITERATORS)
		variable->scalars()
#endif // defined (USE_VARIABLE_ITERATORS)
		))
	{
		Variable_size_type n_rows;

		n_rows=number_of_rows();
		if ((0<n_rows)&&(0<number_of_columns())&&(variable_scalars->size()==n_rows))
		{
			Matrix rhs(n_rows,1);
			std::vector<int> ipiv(n_rows);
			Variable_size_type i;

			for (i=0;i<n_rows;i++)
			{
				rhs(i,0)=(*variable_scalars)[i];
			}
			lapack::gesv(values,ipiv,rhs);
			for (i=0;i<n_rows;i++)
			{
				(*variable_scalars)[i]=rhs(i,0);
			}
			result=Variable_vector_handle(new Variable_vector(*variable_scalars));
		}
		delete variable_scalars;
	}

	return (result);
}

Variable_matrix_handle Variable_matrix::solve(const Variable_matrix_handle& rhs)
//******************************************************************************
// LAST MODIFIED : 7 December 2003
//
// DESCRIPTION :
//==============================================================================
{
	Variable_matrix_handle result(0);
	Variable_size_type n_columns,n_rows;

	n_rows=number_of_rows();
	n_columns=number_of_columns();
	if ((0<n_rows)&&(0<n_columns)&&rhs&&(rhs->number_of_columns()==n_rows))
	{
		Matrix answer(n_rows,n_columns);
		std::vector<int> ipiv(n_rows);

		answer=rhs->values;
		lapack::gesv(values,ipiv,answer);
		result=Variable_matrix_handle(new Variable_matrix(answer));
	}

	return (result);
}

Variable_vector_handle Variable_matrix::solve(const Variable_vector_handle& rhs)
//******************************************************************************
// LAST MODIFIED : 21 January 2004
//
// DESCRIPTION :
//==============================================================================
{
	Variable_vector_handle result(0);
	Variable_size_type n_rows;

	n_rows=number_of_rows();
	if ((0<n_rows)&&(0<number_of_columns())&&rhs&&(rhs->
#if defined (USE_ITERATORS)
		number_differentiable
#else // defined (USE_ITERATORS)
		size
#endif // defined (USE_VARIABLE_ITERATORS)
		()==n_rows))
	{
		Matrix answer(n_rows,1);
		std::vector<int> ipiv(n_rows);
		Variable_size_type i;
		Vector vector_answer(n_rows);

		for (i=0;i<n_rows;i++)
		{
			answer(i,0)=(*rhs)[i];
		}
		lapack::gesv(values,ipiv,answer);
		for (i=0;i<n_rows;i++)
		{
			vector_answer[i]=answer(i,0);
		}
		result=Variable_vector_handle(new Variable_vector(vector_answer));
	}

	return (result);
}

#if defined (USE_VARIABLE_INPUT)
Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	Variable_matrix::input_values()
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
// Returns the values input for a matrix.
//==============================================================================
{
	return (
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
		(new Variable_input_matrix_values(Variable_matrix_handle(this))));
}

#if defined (USE_VARIABLE_INPUT)
Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	Variable_matrix::input_values(Variable_size_type row,
	Variable_size_type column)
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
// Returns the values input for a matrix.
//==============================================================================
{
	return (
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
		(new Variable_input_matrix_values(Variable_matrix_handle(this),row,
		column)));
}

#if defined (USE_VARIABLE_INPUT)
Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	Variable_matrix::input_values(
	const ublas::vector< std::pair<Variable_size_type,Variable_size_type> >
	indices)
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
// Returns the values input for a matrix.
//==============================================================================
{
	return (
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
		(new Variable_input_matrix_values(
		Variable_matrix_handle(this),indices)));
}

Variable_handle Variable_matrix::clone() const
//******************************************************************************
// LAST MODIFIED : 8 December 2003
//
// DESCRIPTION :
//==============================================================================
{
	return (Variable_matrix_handle(new Variable_matrix(*this)));
}

Variable_handle Variable_matrix::evaluate_local()
//******************************************************************************
// LAST MODIFIED : 23 October 2003
//
// DESCRIPTION :
// Duplicate <this> so that <this> isn't changed by operations on the result.
//
//???DB.  Should this turn the matrix into a vector?  No this is done by scalars
//==============================================================================
{
	return (Variable_handle(new Variable_matrix(*this)));
}

#if defined (USE_ITERATORS)
//???DB.  To be done
#else // defined (USE_ITERATORS)
bool Variable_matrix::evaluate_derivative_local(Matrix& matrix,
	std::list<
#if defined (USE_VARIABLE_INPUT)
	Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
	Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	>& independent_variables)
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	Variable_size_type i,j,k,number_of_values;
	Variable_input_matrix_values_handle input_values_handle;

	result=true;
	// matrix is zero'd on entry
	if ((1==independent_variables.size())&&(input_values_handle=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_input_matrix_values,
#if defined (USE_VARIABLE_INPUT)
		Variable_input
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
		>(independent_variables.front())
#else /* defined (USE_SMART_POINTER) */
		dynamic_cast<Variable_input_matrix_values *>(independent_variables.front())
#endif /* defined (USE_SMART_POINTER) */
		)&&(input_values_handle->variable_matrix==Variable_handle(this)))
	{
		Assert((this->size()==matrix.size1())&&
			(input_values_handle->size()==matrix.size2()),std::logic_error(
			"Variable_matrix::evaluate_derivative_local.  "
			"Incorrect matrix size"));
		number_of_values=(input_values_handle->indices).size();
		if (0==number_of_values)
		{
			number_of_values=this->size();
			for (i=0;i<number_of_values;i++)
			{
				matrix(i,i)=1;
			}
		}
		else
		{
			Variable_size_type number_of_columns=values.size2(),
				number_of_rows=values.size1();

			for (k=0;k<number_of_values;k++)
			{
				i=(input_values_handle->indices)[k].first;
				j=(input_values_handle->indices)[k].second;
				if ((0<i)&&(i<=number_of_rows)&&(0<j)&&(j<=number_of_columns))
				{
					matrix((i-1)*number_of_columns+(j-1),k)=1;
				}
			}
		}
	}

	return (result);
}
#endif // defined (USE_ITERATORS)

Variable_handle Variable_matrix::get_input_value_local(
	const
#if defined (USE_VARIABLE_INPUT)
	Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
	Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	& input)
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	Variable_vector_handle values_vector;
	Variable_input_matrix_values_handle input_values_handle;

	if ((input_values_handle=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_input_matrix_values,
#if defined (USE_VARIABLE_INPUT)
		Variable_input
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
		>(input)
#else /* defined (USE_SMART_POINTER) */
		dynamic_cast<Variable_input_matrix_values *>(input)
#endif /* defined (USE_SMART_POINTER) */
		)&&(input_values_handle->variable_matrix==Variable_matrix_handle(this)))
	{
		if (0==(input_values_handle->indices).size())
		{
			Variable_size_type i,j,k,number_of_columns=values.size2(),
				number_of_rows=values.size1();
			Vector extracted_values(number_of_rows*number_of_columns);

			k=0;
			for (i=0;i<number_of_rows;i++)
			{
				for (j=0;j<number_of_columns;j++)
				{
					extracted_values[k]=values(i,j);
					k++;
				}
			}
			values_vector=Variable_vector_handle(new Variable_vector(
				extracted_values));
		}
		else
		{
			Variable_size_type i,j,k,number_of_columns=values.size2(),
				number_of_rows=values.size1(),
				number_of_selected_values=input_values_handle->
#if defined (USE_ITERATORS)
				number_differentiable
#else // defined (USE_ITERATORS)
				size
#endif // defined (USE_VARIABLE_ITERATORS)
				();
			Vector selected_values(number_of_selected_values);

			for (k=0;k<number_of_selected_values;k++)
			{
				i=(input_values_handle->indices)[k].first;
				j=(input_values_handle->indices)[k].second;
				if ((0<i)&&(i<=number_of_rows)&&(0<j)&&(j<=number_of_columns))
				{
					selected_values[k]=values(i-1,j-1);
				}
				else
				{
					selected_values[k]=(Scalar)0;
				}
			}
			values_vector=
				Variable_vector_handle(new Variable_vector(selected_values));
		}
	}
	else
	{
		values_vector=Variable_vector_handle((Variable_vector *)0);
	}

	return (values_vector);
}

bool Variable_matrix::set_input_value_local(const
#if defined (USE_VARIABLE_INPUT)
	Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
	Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	& input,
	const
#if defined (USE_VARIABLES_AS_COMPONENTS)
	Variable_handle
#else // defined (USE_VARIABLES_AS_COMPONENTS)
	Variable_io_specifier_handle
#endif // defined (USE_VARIABLES_AS_COMPONENTS)
	&
#if defined (USE_ITERATORS)
#else // defined (USE_ITERATORS)
	values
#endif // defined (USE_VARIABLE_ITERATORS)
	)
//******************************************************************************
// LAST MODIFIED : 9 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	Variable_input_matrix_values_handle input_values_handle;
	Vector *values_vector;

	result=false;
	if ((input_values_handle=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_input_matrix_values,
#if defined (USE_VARIABLE_INPUT)
		Variable_input
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
		>(input)
#else /* defined (USE_SMART_POINTER) */
		dynamic_cast<Variable_input_matrix_values *>(input)
#endif /* defined (USE_SMART_POINTER) */
		)&&(input_values_handle->variable_matrix==Variable_matrix_handle(this))&&
		(values_vector=
#if defined (USE_ITERATORS)
		//???DB.  To be done
		0
#else // defined (USE_ITERATORS)
		values->scalars()
#endif // defined (USE_VARIABLE_ITERATORS)
		))
	{
		Variable_size_type number_of_specified_values=
			(input_values_handle->indices).size();

		if (0==number_of_specified_values)
		{
			if (this->
#if defined (USE_ITERATORS)
				number_differentiable
#else // defined (USE_ITERATORS)
				size
#endif // defined (USE_VARIABLE_ITERATORS)
				()==values_vector->size())
			{
				Variable_size_type i,j,k,number_of_columns=(this->values).size2(),
					number_of_rows=(this->values).size1();

				k=0;
				for (i=0;i<number_of_rows;i++)
				{
					for (j=0;j<number_of_columns;j++)
					{
						(this->values)(i,j)=(*values_vector)[k];
						k++;
					}
				}
				result=true;
			}
		}
		else
		{
			if (number_of_specified_values==values_vector->size())
			{
				Variable_size_type i,j,k,number_of_columns=(this->values).size2(),
					number_of_rows=(this->values).size1();

				for (k=0;k<number_of_specified_values;k++)
				{
					i=(input_values_handle->indices)[k].first;
					j=(input_values_handle->indices)[k].second;
					if ((0<i)&&(i<=number_of_rows)&&(0<j)&&(j<=number_of_columns))
					{
						(this->values)(i-1,j-1)=(*values_vector)[k];
					}
				}
				result=true;
			}
		}
		delete values_vector;
	}

	return (result);
}

string_handle Variable_matrix::get_string_representation_local()
//******************************************************************************
// LAST MODIFIED : 23 October 2003
//
// DESCRIPTION :
//==============================================================================
{
	string_handle return_string;
	std::ostringstream out;

	if (return_string=new std::string)
	{
		out << values;
		*return_string=out.str();
	}

	return (return_string);
}
