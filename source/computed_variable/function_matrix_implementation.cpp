//******************************************************************************
// FILE : function_matrix_implementation.cpp
//
// LAST MODIFIED : 26 January 2005
//
// DESCRIPTION :
//???DB.  Should be linear transformation (with Function_variable_matrix as an
//  input?
//???DB.  solve method becomes an invert?
//==============================================================================

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
// LAST MODIFIED : 1 September 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_matrix<Value_type>(
		boost::intrusive_ptr< Function_matrix<Value_type> >(this))));
}

EXPORT template<typename Value_type>
Function_variable_handle Function_matrix<Value_type>::output()
//******************************************************************************
// LAST MODIFIED : 1 September 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_matrix<Value_type>(
		boost::intrusive_ptr< Function_matrix<Value_type> >(this))));
}

EXPORT template<typename Value_type>
bool Function_matrix<Value_type>::operator==(const Function&
#if defined (OLD_CODE)
	function
#endif // defined (OLD_CODE)
	) const
//******************************************************************************
// LAST MODIFIED : 26 January 2005
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
					number_of_rows--;
					j=number_of_columns;
					while (result&&(j>0))
					{
						j--;
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
// LAST MODIFIED : 1 September 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_matrix<Value_type>(
		boost::intrusive_ptr< Function_matrix<Value_type> >(this),row,column)));
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
// LAST MODIFIED : 6 August 2004
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

		for (i=0;i<number_of_rows;i++)
		{
			for (j=0;j<number_of_columns;j++)
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
