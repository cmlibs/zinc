//******************************************************************************
// FILE : function_matrix_implementation.cpp
//
// LAST MODIFIED : 16 May 2005
//
// DESCRIPTION :
//???DB.  Should be linear transformation (with Function_variable_matrix as an
//  input?
//???DB.  solve method becomes an invert?
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

#include <sstream>

// to use lapack with ublas
#include <boost/numeric/bindings/traits/ublas_matrix.hpp>
#include <boost/numeric/bindings/traits/std_vector.hpp>
#include <boost/numeric/bindings/lapack/gesv.hpp>

namespace lapack = boost::numeric::bindings::lapack;

#include "computed_variable/function_matrix.hpp"
#include "computed_variable/function_variable.hpp"
#include "computed_variable/function_variable_matrix.hpp"
#include "computed_variable/function_variable_value_specific.hpp"

#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#include "computed_variable/function_identity.hpp"
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

#define Function_matrix_INPUT_AND_OUTPUT_ARE_DISTINCT

#if defined (Function_matrix_INPUT_AND_OUTPUT_ARE_DISTINCT)
// module classes
// ==============

// class Function_variable_matrix_input
// ------------------------------------

EXPORT template<typename Value_type>
class Function_variable_matrix_input :
	public Function_variable_matrix<Value_type>
//******************************************************************************
// LAST MODIFIED : 15 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	public:
		// constructor
		Function_variable_matrix_input(
			const boost::intrusive_ptr< Function_matrix<Value_type> >
			function_matrix):Function_variable_matrix<Value_type>(function_matrix){};
		Function_variable_matrix_input(
			const boost::intrusive_ptr< Function_matrix<Value_type> >
			function_matrix,const Function_size_type row,
			const Function_size_type column):Function_variable_matrix<Value_type>(
			function_matrix,row,column){};
		~Function_variable_matrix_input(){};
	public:
		Function_variable_handle clone() const
		{
			return (Function_variable_handle(
				new Function_variable_matrix_input<Value_type>(*this)));
		};
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		Function_handle derivative(
			const std::list<Function_variable_handle>& independent_variables)
		{
			return (Function_handle(new Function_derivatnew_identity(
				Function_variable_handle(this),independent_variables)));
		}
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		//???DB.  Should operator() and get_entry do an evaluate?
		boost::intrusive_ptr< Function_variable_matrix<Value_type> > operator()(
			Function_size_type row=1,Function_size_type column=1) const
		{
			boost::intrusive_ptr< Function_matrix<Value_type> > function_matrix;
			boost::intrusive_ptr< Function_variable_matrix<Value_type> > result(0);

			if ((function_matrix=boost::dynamic_pointer_cast<
				Function_matrix<Value_type>,Function>(this->function_private))&&
				(row<=this->number_of_rows())&&(column<=this->number_of_columns()))
			{
				result=boost::intrusive_ptr< Function_variable_matrix<Value_type> >(
					new Function_variable_matrix_input<Value_type>(function_matrix,row,
					column));
			}

			return (result);
		};
	private:
		// copy constructor
		Function_variable_matrix_input(
			const Function_variable_matrix_input<Value_type>& variable):
			Function_variable_matrix<Value_type>(variable){};
};


#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
// class Function_derivatnew_matrix_output
// ---------------------------------------

EXPORT template<typename Value_type>
class Function_derivatnew_matrix_output : public Function_derivatnew
//******************************************************************************
// LAST MODIFIED : 16 May 2005
//
// DESCRIPTION :
// ???DB.  Same as Function_derivatnew_identity except for equivalent.
//==============================================================================
{
	public:
		// for construction exception
		class Construction_exception {};
		// constructor
		Function_derivatnew_matrix_output(
			const Function_variable_handle& dependent_variable,
			const std::list<Function_variable_handle>& independent_variables);
		// destructor
		~Function_derivatnew_matrix_output();
	// inherited
	private:
#if defined (EVALUATE_RETURNS_VALUE)
		virtual Function_handle evaluate(Function_variable_handle atomic_variable);
#else // defined (EVALUATE_RETURNS_VALUE)
		virtual bool evaluate(Function_variable_handle atomic_variable);
#endif // defined (EVALUATE_RETURNS_VALUE)
};
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)


// class Function_variable_matrix_output
// -------------------------------------

EXPORT template<typename Value_type>
class Function_variable_matrix_output :
	public Function_variable_matrix<Value_type>
//******************************************************************************
// LAST MODIFIED : 16 May 2005
//
// DESCRIPTION :
//==============================================================================
{
	public:
		// constructor
		Function_variable_matrix_output(
			const boost::intrusive_ptr< Function_matrix<Value_type> >
			function_matrix):Function_variable_matrix<Value_type>(function_matrix){};
		Function_variable_matrix_output(
			const boost::intrusive_ptr< Function_matrix<Value_type> >
			function_matrix,const Function_size_type row,
			const Function_size_type column):Function_variable_matrix<Value_type>(
			function_matrix,row,column){};
		~Function_variable_matrix_output(){};
	public:
		Function_variable_handle clone() const
		{
			return (Function_variable_handle(
				new Function_variable_matrix_output<Value_type>(*this)));
		};
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		Function_handle derivative(
			const std::list<Function_variable_handle>& independent_variables)
		{
			return (Function_handle(new Function_derivatnew_matrix_output<Value_type>(
				Function_variable_handle(this),independent_variables)));
		}
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		//???DB.  Should operator() and get_entry do an evaluate?
		boost::intrusive_ptr< Function_variable_matrix<Value_type> > operator()(
			Function_size_type row=1,Function_size_type column=1) const
		{
			boost::intrusive_ptr< Function_matrix<Value_type> > function_matrix;
			boost::intrusive_ptr< Function_variable_matrix<Value_type> > result(0);

			if ((function_matrix=boost::dynamic_pointer_cast<
				Function_matrix<Value_type>,Function>(this->function_private))&&
				(row<=this->number_of_rows())&&(column<=this->number_of_columns()))
			{
				result=boost::intrusive_ptr< Function_variable_matrix<Value_type> >(
					new Function_variable_matrix_output<Value_type>(function_matrix,row,
					column));
			}

			return (result);
		};
	private:
		// copy constructor
		Function_variable_matrix_output(
			const Function_variable_matrix_output<Value_type>& variable):
			Function_variable_matrix<Value_type>(variable){};
};


#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
// class Function_derivatnew_matrix_output
// ---------------------------------------

EXPORT template<typename Value_type>
Function_derivatnew_matrix_output<Value_type>::
	Function_derivatnew_matrix_output(
	const Function_variable_handle& dependent_variable,
	const std::list<Function_variable_handle>& independent_variables):
	Function_derivatnew(dependent_variable,independent_variables){}
//******************************************************************************
// LAST MODIFIED : 16 May 2005
//
// DESCRIPTION :
// Constructor.
//==============================================================================

EXPORT template<typename Value_type>
Function_derivatnew_matrix_output<Value_type>::
	~Function_derivatnew_matrix_output(){}
//******************************************************************************
// LAST MODIFIED : 16 May 2005
//
// DESCRIPTION :
// Destructor.
//==============================================================================

EXPORT template<typename Value_type>
#if defined (EVALUATE_RETURNS_VALUE)
Function_handle
#else // defined (EVALUATE_RETURNS_VALUE)
bool
#endif // defined (EVALUATE_RETURNS_VALUE)
	Function_derivatnew_matrix_output<Value_type>::evaluate(
	Function_variable_handle
#if defined (EVALUATE_RETURNS_VALUE)
	atomic_variable
#else // defined (EVALUATE_RETURNS_VALUE)
#endif // defined (EVALUATE_RETURNS_VALUE)
	)
//******************************************************************************
// LAST MODIFIED : 16 May 2005
//
// DESCRIPTION :
//==============================================================================
{
#if defined (EVALUATE_RETURNS_VALUE)
	Function_handle result(0);
#else // defined (EVALUATE_RETURNS_VALUE)
	bool result(true);
#endif // defined (EVALUATE_RETURNS_VALUE)

	if (!evaluated())
	{
		Function_size_type number_of_dependent_values=dependent_variable->
			number_differentiable();
		std::list<Function_variable_handle>::const_iterator
			independent_variable_iterator;
		std::list<Matrix> matrices;

#if defined (EVALUATE_RETURNS_VALUE)
#else // defined (EVALUATE_RETURNS_VALUE)
		result=false;
#endif // defined (EVALUATE_RETURNS_VALUE)
		for (independent_variable_iterator=independent_variables.begin();
			independent_variable_iterator!=independent_variables.end();
			++independent_variable_iterator)
		{
			Function_variable_handle independent_variable=
				*independent_variable_iterator;
			Function_size_type number_of_independent_values=independent_variable->
				number_differentiable();
			std::list<Matrix>::iterator matrix_iterator,last;

			// calculate the derivative of dependent variable with respect to
			//   independent variable and add to matrix list
			{
				Function_size_type column,row;
				Function_variable_iterator atomic_dependent_variable_iterator,
					atomic_dependent_variable_iterator_end,
					atomic_independent_variable_iterator,
					atomic_independent_variable_iterator_begin,
					atomic_independent_variable_iterator_end;
				Matrix new_matrix(number_of_dependent_values,
					number_of_independent_values);

				atomic_dependent_variable_iterator_end=dependent_variable->
					end_atomic();
				atomic_independent_variable_iterator_begin=independent_variable->
					begin_atomic();
				atomic_independent_variable_iterator_end=independent_variable->
					end_atomic();
				row=0;
				for (atomic_dependent_variable_iterator=dependent_variable->
					begin_atomic();atomic_dependent_variable_iterator!=
					atomic_dependent_variable_iterator_end;
					++atomic_dependent_variable_iterator)
				{
					Function_variable_handle atomic_dependent_variable=
						*atomic_dependent_variable_iterator;

					if (1==atomic_dependent_variable->number_differentiable())
					{
						boost::intrusive_ptr< Function_variable_matrix<Value_type> >
							atomic_dependent_variable_matrix,
							atomic_independent_variable_matrix;

#if defined (DEBUG)
						//???debug
						std::cout << *atomic_dependent_variable->get_string_representation() << " " << std::endl;
						//???debug
						std::cout << "  " << typeid(*atomic_dependent_variable).name() << std::endl;
#endif // defined (DEBUG)
						if (atomic_dependent_variable_matrix=boost::dynamic_pointer_cast<
							Function_variable_matrix_output<Value_type>,Function_variable>(
							atomic_dependent_variable))
						{
							column=0;
							for (atomic_independent_variable_iterator=
								atomic_independent_variable_iterator_begin;
								atomic_independent_variable_iterator!=
								atomic_independent_variable_iterator_end;
								atomic_independent_variable_iterator++)
							{
								Function_variable_handle atomic_independent_variable=
									*atomic_independent_variable_iterator;

								if (1==atomic_independent_variable->number_differentiable())
								{
									if (((atomic_independent_variable_matrix=
										boost::dynamic_pointer_cast<Function_variable_matrix_input<
										Value_type>,Function_variable>(
										atomic_independent_variable))||
										(atomic_independent_variable_matrix=
										boost::dynamic_pointer_cast<Function_variable_matrix_output<
										Value_type>,Function_variable>(
										atomic_independent_variable)))&&
										equivalent(atomic_dependent_variable_matrix->function(),
										atomic_independent_variable_matrix->function())&&
										(atomic_dependent_variable_matrix->row()==
										atomic_independent_variable_matrix->row())&&
										(atomic_dependent_variable_matrix->column()==
										atomic_independent_variable_matrix->column()))
									{
										new_matrix(row,column)=1;
									}
									else
									{
										new_matrix(row,column)=0;
									}
									++column;
								}
							}
							++row;
						}
						else if (atomic_dependent_variable_matrix=
							boost::dynamic_pointer_cast<Function_variable_matrix_input<
							Value_type>,Function_variable>(atomic_dependent_variable))
						{
							column=0;
							for (atomic_independent_variable_iterator=
								atomic_independent_variable_iterator_begin;
								atomic_independent_variable_iterator!=
								atomic_independent_variable_iterator_end;
								atomic_independent_variable_iterator++)
							{
								Function_variable_handle atomic_independent_variable=
									*atomic_independent_variable_iterator;

								if (1==atomic_independent_variable->number_differentiable())
								{
									if ((atomic_independent_variable_matrix=
										boost::dynamic_pointer_cast<Function_variable_matrix_input<
										Value_type>,Function_variable>(
										atomic_independent_variable))&&
										equivalent(atomic_dependent_variable_matrix->function(),
										atomic_independent_variable_matrix->function())&&
										(atomic_dependent_variable_matrix->row()==
										atomic_independent_variable_matrix->row())&&
										(atomic_dependent_variable_matrix->column()==
										atomic_independent_variable_matrix->column()))
									{
										new_matrix(row,column)=1;
									}
									else
									{
										new_matrix(row,column)=0;
									}
									++column;
								}
							}
							++row;
						}
					}
#if defined (OLD_CODE)
					{
						column=0;
						for (atomic_independent_variable_iterator=
							atomic_independent_variable_iterator_begin;
							atomic_independent_variable_iterator!=
							atomic_independent_variable_iterator_end;
							atomic_independent_variable_iterator++)
						{
							Function_variable_handle atomic_independent_variable=
								*atomic_independent_variable_iterator;

							if (1==atomic_independent_variable->number_differentiable())
							{
#if defined (DEBUG)
								//???debug
								std::cout << row << " " << column << " " << *atomic_dependent_variable->get_string_representation() << " " << *atomic_independent_variable->get_string_representation() << std::endl;
								//???debug
								std::cout << "  " << typeid(atomic_dependent_variable->function()).name() << " " << typeid(atomic_independent_variable->function()).name() << std::endl;
#endif // defined (DEBUG)
								if (equivalent(atomic_dependent_variable,
									atomic_independent_variable))
								{
									new_matrix(row,column)=1;
								}
								else
								{
									new_matrix(row,column)=0;
								}
								++column;
							}
						}
						++row;
					}
#endif // defined (OLD_CODE)
				}
				matrices.push_back(new_matrix);
			}
			last=matrices.end();
			--last;
			for (matrix_iterator=matrices.begin();matrix_iterator!=last;
				++matrix_iterator)
			{
				Matrix& matrix= *matrix_iterator;
				Matrix new_matrix((matrix.size1)(),
					number_of_independent_values*(matrix.size2)());

				new_matrix.clear();
				matrices.push_back(new_matrix);
			}
		}
		derivative_matrix=Derivative_matrix(matrices);
		set_evaluated();
#if defined (EVALUATE_RETURNS_VALUE)
#else // defined (EVALUATE_RETURNS_VALUE)
		result=true;
#endif // defined (EVALUATE_RETURNS_VALUE)
	}
#if defined (EVALUATE_RETURNS_VALUE)
	if (evaluated())
	{
		result=get_value(atomic_variable);
	}
#else // defined (EVALUATE_RETURNS_VALUE)
#endif // defined (EVALUATE_RETURNS_VALUE)

	return (result);
}
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#endif // defined (Function_matrix_INPUT_AND_OUTPUT_ARE_DISTINCT)


// global classes
// ==============

// class Function_matrix
// ---------------------

EXPORT template<typename Value_type>
Function_matrix<Value_type>::Function_matrix(
	ublas::matrix<Value_type,ublas::column_major>& values):Function(),
	values(values){}
//******************************************************************************
// LAST MODIFIED : 6 August 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================

EXPORT template<typename Value_type>
Function_matrix<Value_type>::~Function_matrix()
//******************************************************************************
// LAST MODIFIED : 6 August 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	// do nothing
}

EXPORT template<typename Value_type>
string_handle Function_matrix<Value_type>::get_string_representation()
//******************************************************************************
// LAST MODIFIED : 6 August 2004
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

EXPORT template<typename Value_type>
Function_variable_handle Function_matrix<Value_type>::input()
//******************************************************************************
// LAST MODIFIED : 1 March 2005
//
// DESCRIPTION :
//==============================================================================
{
#if defined (Function_matrix_INPUT_AND_OUTPUT_ARE_DISTINCT)
	return (Function_variable_handle(new Function_variable_matrix_input<
		Value_type>(boost::intrusive_ptr< Function_matrix<Value_type> >(this))));
#else // defined (Function_matrix_INPUT_AND_OUTPUT_ARE_DISTINCT)
	return (Function_variable_handle(new Function_variable_matrix<Value_type>(
		boost::intrusive_ptr< Function_matrix<Value_type> >(this)
#if defined (Function_variable_matrix_HAS_INPUT_ATTRIBUTE)
		,true
#endif // defined (Function_variable_matrix_HAS_INPUT_ATTRIBUTE)
		)));
#endif // defined (Function_matrix_INPUT_AND_OUTPUT_ARE_DISTINCT)
}

EXPORT template<typename Value_type>
Function_variable_handle Function_matrix<Value_type>::output()
//******************************************************************************
// LAST MODIFIED : 1 March 2005
//
// DESCRIPTION :
//==============================================================================
{
#if defined (Function_matrix_INPUT_AND_OUTPUT_ARE_DISTINCT)
	return (Function_variable_handle(new Function_variable_matrix_output<
		Value_type>(boost::intrusive_ptr< Function_matrix<Value_type> >(this))));
#else // defined (Function_matrix_INPUT_AND_OUTPUT_ARE_DISTINCT)
	return (Function_variable_handle(new Function_variable_matrix<Value_type>(
		boost::intrusive_ptr< Function_matrix<Value_type> >(this)
#if defined (Function_variable_matrix_HAS_INPUT_ATTRIBUTE)
		,false
#endif // defined (Function_variable_matrix_HAS_INPUT_ATTRIBUTE)
		)));
#endif // defined (Function_matrix_INPUT_AND_OUTPUT_ARE_DISTINCT)
}

EXPORT template<typename Value_type>
const ublas::matrix<Value_type,ublas::column_major>&
	Function_matrix<Value_type>::matrix()
//******************************************************************************
// LAST MODIFIED : 23 February 2005
//
// DESCRIPTION :
//==============================================================================
{
	return (values);
}

EXPORT template<typename Value_type>
bool Function_matrix<Value_type>::operator==(const Function&
#if defined (OLD_CODE)
	function
#endif // defined (OLD_CODE)
	) const
//******************************************************************************
// LAST MODIFIED : 7 April 2005
//
// DESCRIPTION :
// Equality operator.
//==============================================================================
{
	bool result;

	result=false;
#if defined (OLD_CODE)
	//???DB.  Removed so that can create unique functions from a matrix
	if (this)
	{
		try
		{
			const Function_matrix<Value_type>& function_matrix=
				dynamic_cast<const Function_matrix<Value_type>&>(function);
			Function_size_type j,number_of_columns,number_of_rows;

			if (((number_of_rows=values.size1())==function_matrix.values.size1())&&
				((number_of_columns=values.size2())==function_matrix.values.size2()))
			{
				result=true;
				while (result&&(number_of_rows>0))
				{
					--number_of_rows;
					j=number_of_columns;
					while (result&&(j>0))
					{
						--j;
						result=(values(number_of_rows,j)==
							function_matrix.values(number_of_rows,j));
					}
				}
			}
		}
		catch (std::bad_cast)
		{
			// do nothing
		}
	}
#endif // defined (OLD_CODE)

	return (result);
}

EXPORT template<typename Value_type>
Function_variable_handle Function_matrix<Value_type>::entry(
	Function_size_type row,Function_size_type column)
//******************************************************************************
// LAST MODIFIED : 5 March 2005
//
// DESCRIPTION :
//==============================================================================
{
#if defined (Function_matrix_INPUT_AND_OUTPUT_ARE_DISTINCT)
	return (Function_variable_handle(new Function_variable_matrix_input<
		Value_type>(boost::intrusive_ptr< Function_matrix<Value_type> >(this),row,
		column)));
#else // defined (Function_matrix_INPUT_AND_OUTPUT_ARE_DISTINCT)
	return (Function_variable_handle(new Function_variable_matrix<Value_type>(
		boost::intrusive_ptr< Function_matrix<Value_type> >(this),
#if defined (Function_variable_matrix_HAS_INPUT_ATTRIBUTE)
		false,
#endif // defined (Function_variable_matrix_HAS_INPUT_ATTRIBUTE)
		row,column)));
#endif // defined (Function_matrix_INPUT_AND_OUTPUT_ARE_DISTINCT)
}

EXPORT template<typename Value_type>
Value_type& Function_matrix<Value_type>::operator()(Function_size_type row,
	Function_size_type column)
//******************************************************************************
// LAST MODIFIED : 1 September 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (values(row-1,column-1));
}

EXPORT template<typename Value_type>
boost::intrusive_ptr< Function_matrix<Value_type> >
	Function_matrix<Value_type>::sub_matrix(Function_size_type row_low,
	Function_size_type row_high,Function_size_type column_low,
	Function_size_type column_high) const
//******************************************************************************
// LAST MODIFIED : 7 April 2005
//
// DESCRIPTION :
// Returns the specified sub-matrix.
//==============================================================================
{
	boost::intrusive_ptr< Function_matrix<Value_type> > result;

	if ((0<row_low)&&(row_low<=row_high)&&(row_high<=values.size1())&&
		(0<column_low)&&(column_low<=column_high)&&(column_high<=values.size2()))
	{
		Function_size_type i,j,number_of_columns=column_high-column_low+1,
			number_of_rows=row_high-row_low+1;
		ublas::matrix<Value_type,ublas::column_major>
			temp_matrix(number_of_rows,number_of_columns);

		for (i=0;i<number_of_rows;++i)
		{
			for (j=0;j<number_of_columns;++j)
			{
				temp_matrix(i,j)=values(i+row_low-1,j+column_low-1);
			}
		}
		result=boost::intrusive_ptr< Function_matrix<Value_type> >(
			new Function_matrix<Value_type>(temp_matrix));
	}
	else
	{
		ublas::matrix<Value_type,ublas::column_major> temp_matrix(0,0);

		result=boost::intrusive_ptr< Function_matrix<Value_type> >(
			new Function_matrix<Value_type>(temp_matrix));
	}

	return (result);
}

EXPORT template<typename Value_type>
Function_size_type Function_matrix<Value_type>::number_of_rows() const
//******************************************************************************
// LAST MODIFIED : 6 August 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (values.size1());
}

EXPORT template<typename Value_type>
Function_size_type Function_matrix<Value_type>::number_of_columns() const
//******************************************************************************
// LAST MODIFIED : 6 August 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (values.size2());
}

EXPORT template<typename Value_type>
boost::intrusive_ptr< Function_matrix<Value_type> >
	Function_matrix<Value_type>::solve(
	const boost::intrusive_ptr< Function_matrix<Value_type> >&)
//******************************************************************************
// LAST MODIFIED : 14 September 2004
//
// DESCRIPTION :
// A x = B
// n by n  n by m  n by m
//==============================================================================
{
	return (0);
}

template<>
boost::intrusive_ptr< Function_matrix<Scalar> >
	Function_matrix<Scalar>::solve(
	const boost::intrusive_ptr< Function_matrix<Scalar> >&);

EXPORT template<typename Value_type>
bool Function_matrix<Value_type>::determinant(Value_type&)
//******************************************************************************
// LAST MODIFIED : 14 September 2004
//
// DESCRIPTION :
// Zero for a non-square matrix.
//==============================================================================
{
	return (0);
}

template<>
bool Function_matrix<Scalar>::determinant(Scalar&);

EXPORT template<typename Value_type>
#if defined (EVALUATE_RETURNS_VALUE)
Function_handle Function_matrix<Value_type>::evaluate(
	Function_variable_handle atomic_variable)
#else // defined (EVALUATE_RETURNS_VALUE)
bool Function_matrix<Value_type>::evaluate(Function_variable_handle)
#endif // defined (EVALUATE_RETURNS_VALUE)
//******************************************************************************
// LAST MODIFIED : 13 January 2005
//
// DESCRIPTION :
//==============================================================================
{
#if defined (EVALUATE_RETURNS_VALUE)
	return (get_value(atomic_variable));
#else // defined (EVALUATE_RETURNS_VALUE)
	return (true);
#endif // defined (EVALUATE_RETURNS_VALUE)
}

EXPORT template<typename Value_type>
bool Function_matrix<Value_type>::evaluate_derivative(Scalar&,
	Function_variable_handle,std::list<Function_variable_handle>&)
//******************************************************************************
// LAST MODIFIED : 6 August 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (false);
}

template<>
bool Function_matrix<Scalar>::evaluate_derivative(Scalar& derivative,
	Function_variable_handle atomic_variable,
	std::list<Function_variable_handle>& atomic_independent_variables);

EXPORT template<typename Value_type>
bool Function_matrix<Value_type>::set_value(
	Function_variable_handle atomic_variable,
	Function_variable_handle atomic_value)
//******************************************************************************
// LAST MODIFIED : 1 December 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	boost::intrusive_ptr< Function_variable_matrix<Value_type> >
		atomic_matrix_variable;
	boost::intrusive_ptr< Function_variable_value_specific<Value_type> >
		value_type;

	result=false;
	if ((atomic_matrix_variable=boost::dynamic_pointer_cast<
		Function_variable_matrix<Value_type>,Function_variable>(atomic_variable))&&
		equivalent(Function_handle(this),atomic_matrix_variable->function())&&
		atomic_value&&(atomic_value->value())&&(value_type=
		boost::dynamic_pointer_cast<Function_variable_value_specific<Value_type>,
		Function_variable_value>(atomic_value->value())))
	{
		result=value_type->set(values((atomic_matrix_variable->row())-1,
			(atomic_matrix_variable->column())-1),atomic_value);
	}
	if (result)
	{
		set_not_evaluated();
	}

	return (result);
}

EXPORT template<typename Value_type>
Function_handle Function_matrix<Value_type>::get_value(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 1 September 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle result(0);
	boost::intrusive_ptr< Function_variable_matrix<Value_type> >
		atomic_variable_matrix;
	ublas::matrix<Value_type,ublas::column_major> result_matrix(1,1);

	if (atomic_variable&&
		equivalent(Function_handle(this),(atomic_variable->function)())&&
		(atomic_variable_matrix=boost::dynamic_pointer_cast<
		Function_variable_matrix<Value_type>,Function_variable>(
		atomic_variable))&&
		(atomic_variable_matrix->get_entry)(result_matrix(0,0)))
	{
		result=Function_handle(new Function_matrix<Value_type>(result_matrix));
	}

	return (result);
}

EXPORT template<typename Value_type>
Function_matrix<Value_type>::Function_matrix(
	const Function_matrix<Value_type>& function_matrix):Function(function_matrix),
	values(function_matrix.values){}
//******************************************************************************
// LAST MODIFIED : 6 August 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================

EXPORT template<typename Value_type>
Function_matrix<Value_type>& Function_matrix<Value_type>::operator=(
	const Function_matrix<Value_type>& function_matrix)
//******************************************************************************
// LAST MODIFIED : 6 August 2004
//
// DESCRIPTION :
// Assignment operator.
//==============================================================================
{
	this->values=function_matrix.values;

	return (*this);
}
