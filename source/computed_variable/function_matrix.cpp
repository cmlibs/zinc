//******************************************************************************
// FILE : function_matrix.cpp
//
// LAST MODIFIED : 19 July 2004
//
// DESCRIPTION :
//???DB.  Should be template?
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
#include "computed_variable/function_variable_value_scalar.hpp"


// module typedefs
// ===============

typedef boost::intrusive_ptr< Function_variable_matrix<Scalar> >
	Function_variable_matrix_scalar_handle;


// module classes
// ==============

// class Function_variable_matrix_coefficients
// -------------------------------------------

// forward declaration so that can use _handle
class Function_variable_matrix_coefficients;
typedef boost::intrusive_ptr<Function_variable_matrix_coefficients>
	Function_variable_matrix_coefficients_handle;

class Function_variable_matrix_coefficients :
	public Function_variable_matrix<Scalar>
//******************************************************************************
// LAST MODIFIED : 19 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	friend class Function_matrix;
	public:
		// constructor
		Function_variable_matrix_coefficients(
			const Function_matrix_handle function_matrix):
			Function_variable_matrix<Scalar>(function_matrix){};
		Function_variable_matrix_coefficients(
			const Function_matrix_handle function_matrix,
			const Function_size_type row,const Function_size_type column):
			Function_variable_matrix<Scalar>(function_matrix,row,column){};
		~Function_variable_matrix_coefficients(){}
	public:
		Function_variable_handle clone() const
		{
			return (Function_variable_handle(
				new Function_variable_matrix_coefficients(*this)));
		};
		Function_variable_matrix_scalar_handle operator()(
			Function_size_type row,Function_size_type column)
		{
			Function_variable_matrix_scalar_handle result(0);

			if ((row<=number_of_rows())&&(column<=number_of_columns()))
			{
				result=Function_variable_matrix_scalar_handle(
					new Function_variable_matrix_coefficients(
					boost::dynamic_pointer_cast<Function_matrix,Function>(
					function_private),row,column));
			}

			return (result);
		};
		Function_size_type number_of_rows() const
		{
			return ((boost::dynamic_pointer_cast<Function_matrix,Function>(
				function_private))->number_of_rows());
		};
		Function_size_type number_of_columns() const
		{
			return ((boost::dynamic_pointer_cast<Function_matrix,Function>(
				function_private))->number_of_columns());
		};
		bool get_entry(Scalar& value) const
		{
			bool result;

			result=false;
			if ((0<row)&&(row<=number_of_rows())&&
				(0<column)&&(column<=number_of_columns()))
			{
				result=true;
				value=(*(boost::dynamic_pointer_cast<Function_matrix,Function>(
					function_private)))(row-1,column-1);
			}

			return (result);
		};
	private:
		// copy constructor
		Function_variable_matrix_coefficients(
			const Function_variable_matrix_coefficients& variable):
			Function_variable_matrix<Scalar>(variable){};
};


// global classes
// ==============

// class Function_matrix
// ---------------------

Function_matrix::Function_matrix(Matrix& values):Function(),values(values){}
//******************************************************************************
// LAST MODIFIED : 20 February 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================

Function_matrix::~Function_matrix()
//******************************************************************************
// LAST MODIFIED : 20 February 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	// do nothing
}

string_handle Function_matrix::get_string_representation()
//******************************************************************************
// LAST MODIFIED : 20 February 2004
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

Function_variable_handle Function_matrix::input()
//******************************************************************************
// LAST MODIFIED : 19 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_matrix_coefficients(
		Function_matrix_handle(this))));
}

Function_variable_handle Function_matrix::output()
//******************************************************************************
// LAST MODIFIED : 19 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_matrix_coefficients(
		Function_matrix_handle(this))));
}

Function_variable_handle Function_matrix::entry(Function_size_type row,
	Function_size_type column)
//******************************************************************************
// LAST MODIFIED : 19 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_matrix_coefficients(
		Function_matrix_handle(this),row,column)));
}

Scalar& Function_matrix::operator()(Function_size_type row,
	Function_size_type column)
//******************************************************************************
// LAST MODIFIED : 20 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (values(row,column));
}

Function_matrix_handle Function_matrix::sub_matrix(Function_size_type row_low,
	Function_size_type row_high,Function_size_type column_low,
	Function_size_type column_high) const
//******************************************************************************
// LAST MODIFIED : 20 February 2004
//
// DESCRIPTION :
// Returns the specified sub-matrix.
//==============================================================================
{
	Function_matrix_handle result;

	if ((0<row_low)&&(row_low<=row_high)&&(row_high<=values.size1())&&
		(0<column_low)&&(column_low<=column_high)&&(column_high<=values.size2()))
	{
		Function_size_type i,j,number_of_columns=column_high-column_low+1,
			number_of_rows=row_high-row_low+1;
		Matrix temp_matrix(number_of_rows,number_of_columns);

		for (i=0;i<number_of_rows;i++)
		{
			for (j=0;j<number_of_columns;j++)
			{
				temp_matrix(i,j)=values(i+row_low-1,j+column_low-1);
			}
		}
		result=Function_matrix_handle(new Function_matrix(temp_matrix));
	}
	else
	{
		Matrix temp_matrix(0,0);

		result=Function_matrix_handle(new Function_matrix(temp_matrix));
	}

	return (result);
}

Function_size_type Function_matrix::number_of_rows() const
//******************************************************************************
// LAST MODIFIED : 20 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (values.size1());
}

Function_size_type Function_matrix::number_of_columns() const
//******************************************************************************
// LAST MODIFIED : 20 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (values.size2());
}

Function_matrix_handle Function_matrix::solve(const Function_matrix_handle& rhs)
//******************************************************************************
// LAST MODIFIED : 1 March 2004
//
// DESCRIPTION :
// A x = B
// n by n  n by m  n by m
//==============================================================================
{
	Function_matrix_handle result(0);
	Function_size_type number_of_rhss,size_A;

	if (this&&(0<(size_A=number_of_rows()))&&(number_of_columns()==size_A)&&
		rhs&&(rhs->number_of_rows()==size_A)&&
		(0<(number_of_rhss=rhs->number_of_columns())))
	{
		Matrix A(size_A,size_A),X(size_A,number_of_rhss);
		std::vector<int> ipiv(size_A);

		A=values;
		X=rhs->values;
		lapack::gesv(A,ipiv,X);
		result=Function_matrix_handle(new Function_matrix(X));
	}

	return (result);
}

Function_handle Function_matrix::evaluate(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 23 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (get_value(atomic_variable));
}

bool Function_matrix::evaluate_derivative(Scalar& derivative,
	Function_variable_handle atomic_variable,
	std::list<Function_variable_handle>& atomic_independent_variables)
//******************************************************************************
// LAST MODIFIED : 19 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	Function_variable_matrix_coefficients_handle
		atomic_dependent_variable,atomic_independent_variable;

	result=false;
	if ((atomic_dependent_variable=boost::dynamic_pointer_cast<
		Function_variable_matrix_coefficients,Function_variable>(atomic_variable))&&
		(Function_handle(this)==atomic_dependent_variable->function())&&
		(1==atomic_dependent_variable->number_differentiable()))
	{
		result=true;
		if ((1==atomic_independent_variables.size())&&
			(atomic_independent_variable=boost::dynamic_pointer_cast<
			Function_variable_matrix_coefficients,Function_variable>(
			atomic_independent_variables.front()))&&
			(*atomic_dependent_variable== *atomic_independent_variable))
		{
			derivative=1;
		}
		else
		{
			derivative=0;
		}
	}

	return (result);
}

bool Function_matrix::set_value(Function_variable_handle atomic_variable,
	Function_variable_handle atomic_value)
//******************************************************************************
// LAST MODIFIED : 19 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	Function_variable_matrix_coefficients_handle atomic_matrix_variable;
	Function_variable_value_scalar_handle value_scalar;

	result=false;
	if ((atomic_matrix_variable=boost::dynamic_pointer_cast<
		Function_variable_matrix_coefficients,Function_variable>(atomic_variable))&&
		(Function_handle(this)==atomic_matrix_variable->function())&&
		atomic_value&&(atomic_value->value())&&
		(std::string("Scalar")==(atomic_value->value())->type())&&
		(value_scalar=boost::dynamic_pointer_cast<Function_variable_value_scalar,
		Function_variable_value>(atomic_value->value())))
	{
		result=value_scalar->set(values((atomic_matrix_variable->row)-1,
			(atomic_matrix_variable->column)-1),atomic_value);
	}

	return (result);
}

Function_handle Function_matrix::get_value(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 19 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle result(0);
	Function_variable_matrix_coefficients_handle
		atomic_variable_matrix_coefficients;
	Matrix result_matrix(1,1);

	if (atomic_variable&&(Function_handle(this)==(atomic_variable->function)())&&
		(atomic_variable_matrix_coefficients=boost::dynamic_pointer_cast<
		Function_variable_matrix_coefficients,Function_variable>(atomic_variable))&&
		(atomic_variable_matrix_coefficients->get_entry)(result_matrix(0,0)))
	{
		result=Function_handle(new Function_matrix(result_matrix));
	}

	return (result);
}

Function_matrix::Function_matrix(const Function_matrix& function_matrix):
	Function(function_matrix),values(function_matrix.values){}
//******************************************************************************
// LAST MODIFIED : 25 February 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================

Function_matrix& Function_matrix::operator=(
	const Function_matrix& function_matrix)
//******************************************************************************
// LAST MODIFIED : 20 February 2004
//
// DESCRIPTION :
// Assignment operator.
//==============================================================================
{
	this->values=function_matrix.values;

	return (*this);
}
