//******************************************************************************
// FILE : function_matrix_implementation.cpp
//
// LAST MODIFIED : 22 August 2004
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


// module classes
// ==============

// class Function_variable_matrix_coefficients
// -------------------------------------------

EXPORT template<typename Value_type>
class Function_variable_matrix_coefficients :
	public Function_variable_matrix<Value_type>
//******************************************************************************
// LAST MODIFIED : 6 August 2004
//
// DESCRIPTION :
//==============================================================================
{
	friend class Function_matrix<Value_type>;
	public:
		// constructor
		Function_variable_matrix_coefficients(
			const boost::intrusive_ptr< Function_matrix<Value_type> >
			function_matrix):Function_variable_matrix<Value_type>(function_matrix){};
		Function_variable_matrix_coefficients(
			const boost::intrusive_ptr< Function_matrix<Value_type> > function_matrix,
			const Function_size_type row,const Function_size_type column):
			Function_variable_matrix<Value_type>(function_matrix,row,column){};
		~Function_variable_matrix_coefficients(){}
	public:
		Function_variable_handle clone() const
		{
			return (Function_variable_handle(
				new Function_variable_matrix_coefficients<Value_type>(*this)));
		};
		boost::intrusive_ptr< Function_variable_matrix<Value_type> > operator()(
			Function_size_type row,Function_size_type column) const
		{
			boost::intrusive_ptr< Function_variable_matrix<Value_type> > result(0);

			if ((row<=number_of_rows())&&(column<=number_of_columns()))
			{
				result=boost::intrusive_ptr< Function_variable_matrix<Value_type> >(
					new Function_variable_matrix_coefficients<Value_type>(
					boost::dynamic_pointer_cast<Function_matrix<Value_type>,Function>(
					function_private),row,column));
			}

			return (result);
		};
		Function_size_type number_of_rows() const
		{
			return ((boost::dynamic_pointer_cast<Function_matrix<Value_type>,
				Function>(function_private))->number_of_rows());
		};
		Function_size_type number_of_columns() const
		{
			return ((boost::dynamic_pointer_cast<Function_matrix<Value_type>,
				Function>(function_private))->number_of_columns());
		};
		bool get_entry(Value_type& value) const
		{
			bool result;

			result=false;
			if ((0<row)&&(row<=number_of_rows())&&
				(0<column)&&(column<=number_of_columns()))
			{
				result=true;
				value=(*(boost::dynamic_pointer_cast<Function_matrix<Value_type>,
					Function>(function_private)))(row-1,column-1);
			}

			return (result);
		};
	private:
		// copy constructor
		Function_variable_matrix_coefficients(
			const Function_variable_matrix_coefficients<Value_type>& variable):
			Function_variable_matrix<Value_type>(variable){};
};


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
// LAST MODIFIED : 6 August 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(
		new Function_variable_matrix_coefficients<Value_type>(
		boost::intrusive_ptr< Function_matrix<Value_type> >(this))));
}

EXPORT template<typename Value_type>
Function_variable_handle Function_matrix<Value_type>::output()
//******************************************************************************
// LAST MODIFIED : 6 August 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(
		new Function_variable_matrix_coefficients<Value_type>(
		boost::intrusive_ptr< Function_matrix<Value_type> >(this))));
}

EXPORT template<typename Value_type>
bool Function_matrix<Value_type>::operator==(const Function& function) const
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

	return (result);
}

EXPORT template<typename Value_type>
Function_variable_handle Function_matrix<Value_type>::entry(
	Function_size_type row,Function_size_type column)
//******************************************************************************
// LAST MODIFIED : 6 August 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(
		new Function_variable_matrix_coefficients<Value_type>(
		boost::intrusive_ptr< Function_matrix<Value_type> >(this),row,column)));
}

EXPORT template<typename Value_type>
Value_type& Function_matrix<Value_type>::operator()(Function_size_type row,
	Function_size_type column)
//******************************************************************************
// LAST MODIFIED : 6 August 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (values(row,column));
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
	const boost::intrusive_ptr< Function_matrix<Value_type> >& rhs)
//******************************************************************************
// LAST MODIFIED : 6 August 2004
//
// DESCRIPTION :
// A x = B
// n by n  n by m  n by m
//==============================================================================
{
	boost::intrusive_ptr< Function_matrix<Value_type> > result(0);
	Function_size_type number_of_rhss,size_A;

	if (this&&(0<(size_A=number_of_rows()))&&(number_of_columns()==size_A)&&
		rhs&&(rhs->number_of_rows()==size_A)&&
		(0<(number_of_rhss=rhs->number_of_columns())))
	{
		ublas::matrix<Value_type,ublas::column_major>
			A(size_A,size_A),X(size_A,number_of_rhss);
		std::vector<int> ipiv(size_A);

		A=values;
		X=rhs->values;
		lapack::gesv(A,ipiv,X);
		result=boost::intrusive_ptr< Function_matrix<Value_type> >(
			new Function_matrix<Value_type>(X));
	}

	return (result);
}

EXPORT template<typename Value_type>
Function_handle Function_matrix<Value_type>::evaluate(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 6 August 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (get_value(atomic_variable));
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

#if defined (__GNUC__)
template<>
bool Function_matrix<Scalar>::evaluate_derivative(Scalar& derivative,
	Function_variable_handle atomic_variable,
	std::list<Function_variable_handle>& atomic_independent_variables);
#endif // defined (__GNUC__)

EXPORT template<typename Value_type>
bool Function_matrix<Value_type>::set_value(
	Function_variable_handle atomic_variable,
	Function_variable_handle atomic_value)
//******************************************************************************
// LAST MODIFIED : 13 August 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	boost::intrusive_ptr< Function_variable_matrix_coefficients<Value_type> >
		atomic_matrix_variable;
	boost::intrusive_ptr< Function_variable_value_specific<Value_type> >
		value_type;

	result=false;
	if ((atomic_matrix_variable=boost::dynamic_pointer_cast<
		Function_variable_matrix_coefficients<Value_type>,Function_variable>(
		atomic_variable))&&
		equivalent(Function_handle(this),atomic_matrix_variable->function())&&
		atomic_value&&(atomic_value->value())&&(value_type=
		boost::dynamic_pointer_cast<Function_variable_value_specific<Value_type>,
		Function_variable_value>(atomic_value->value())))
	{
		result=value_type->set(values((atomic_matrix_variable->row)-1,
			(atomic_matrix_variable->column)-1),atomic_value);
	}

	return (result);
}

EXPORT template<typename Value_type>
Function_handle Function_matrix<Value_type>::get_value(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 13 August 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle result(0);
	boost::intrusive_ptr< Function_variable_matrix_coefficients<Value_type> >
		atomic_variable_matrix_coefficients;
	ublas::matrix<Value_type,ublas::column_major> result_matrix(1,1);

	if (atomic_variable&&
		equivalent(Function_handle(this),(atomic_variable->function)())&&
		(atomic_variable_matrix_coefficients=boost::dynamic_pointer_cast<
		Function_variable_matrix_coefficients<Value_type>,Function_variable>(
		atomic_variable))&&
		(atomic_variable_matrix_coefficients->get_entry)(result_matrix(0,0)))
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
