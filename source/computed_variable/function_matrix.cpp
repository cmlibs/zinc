//******************************************************************************
// FILE : function_matrix.cpp
//
// LAST MODIFIED : 11 April 2004
//
// DESCRIPTION :
//???DB.  Should be template?
//==============================================================================

#include <sstream>

// to use lapack with ublas
#include <boost/numeric/bindings/traits/ublas_matrix.hpp>
#include <boost/numeric/bindings/traits/std_vector.hpp>
#include <boost/numeric/bindings/lapack/gesv.hpp>

namespace lapack = boost::numeric::bindings::lapack;

#include "computed_variable/function_matrix.hpp"
#include "computed_variable/function_variable.hpp"
#include "computed_variable/function_variable_value_scalar.hpp"

// module classes
// ==============

// forward declaration so that can use _handle
class Function_variable_matrix;
typedef boost::intrusive_ptr<Function_variable_matrix>
	Function_variable_matrix_handle;


// class Function_variable_iterator_representation_atomic_matrix
// -------------------------------------------------------------

class Function_variable_iterator_representation_atomic_matrix :
	public Function_variable_iterator_representation
//******************************************************************************
// LAST MODIFIED : 3 March 2004
//
// DESCRIPTION :
//==============================================================================
{
	public:
		// constructor
		Function_variable_iterator_representation_atomic_matrix(const bool begin,
			Function_variable_matrix_handle variable);
		// a "virtual" constructor
		Function_variable_iterator_representation *clone();
	protected:
		// destructor
		~Function_variable_iterator_representation_atomic_matrix();
	private:
		// increment
		void increment();
		// decrement
		void decrement();
		// equality
		bool equality(
			const Function_variable_iterator_representation* representation);
		// dereference
		Function_variable_handle dereference() const;
	private:
		// copy constructor
		Function_variable_iterator_representation_atomic_matrix(
			const Function_variable_iterator_representation_atomic_matrix&);
	private:
		Function_variable_matrix_handle atomic_variable,variable;
};

static bool Function_variable_matrix_set_scalar_function(Scalar& value,
	const Function_variable_handle variable);


// class Function_variable_matrix
// ------------------------------

class Function_variable_matrix : public Function_variable
//******************************************************************************
// LAST MODIFIED : 15 March 2004
//
// DESCRIPTION :
// <column> and <row> start from one when referencing a matrix entry.  Zero
// indicates all.
//==============================================================================
{
	friend class Function_matrix;
	friend class Function_variable_iterator_representation_atomic_matrix;
	friend bool is_atomic(Function_variable_matrix_handle variable);
	friend bool Function_variable_matrix_set_scalar_function(Scalar& value,
		const Function_variable_handle variable);
	public:
		// constructor
		Function_variable_matrix(const Function_matrix_handle function_matrix):
			Function_variable(),function_matrix(function_matrix),column(0),row(0)
		{
			if (function_matrix&&(1==(function_matrix->number_of_rows)())&&
				(1==(function_matrix->number_of_columns)()))
			{
				value_private=Function_variable_value_handle(
					new Function_variable_value_scalar(
					Function_variable_matrix_set_scalar_function));
			}
		};
		Function_variable_matrix(const Function_matrix_handle function_matrix,
			const Function_size_type row,const Function_size_type column):
			function_matrix(function_matrix),column(column),row(row)
		{
			if (function_matrix)
			{
				Function_size_type number_of_columns,number_of_rows;

				if (1==(number_of_rows=(function_matrix->number_of_rows)()))
				{
					this->row=1;
				}
				else
				{
					if (this->row>number_of_rows)
					{
						this->row=0;
					}
				}
				if (1==(number_of_columns=(function_matrix->number_of_columns)()))
				{
					this->column=1;
				}
				else
				{
					if (this->column>number_of_columns)
					{
						this->column=0;
					}
				}
				if ((0!=this->row)&&(0!=this->column))
				{
					value_private=Function_variable_value_handle(
						new Function_variable_value_scalar(
						Function_variable_matrix_set_scalar_function));
				}
			}
			else
			{
				this->row=0;
				this->column=0;
			}
		};
		~Function_variable_matrix(){}
	// inherited
	public:
		Function_variable_handle clone() const
		{
			return (Function_variable_handle(new Function_variable_matrix(*this)));
		};
		Function_handle function()
		{
			return (function_matrix);
		}
		string_handle get_string_representation()
		{
			string_handle return_string(0);

			if (return_string=new std::string)
			{
				std::ostringstream out;
				
				if (0==row)
				{
					if (0==column)
					{
						out << function_matrix->values;
					}
					else
					{
						Function_size_type i,
							number_of_rows=(function_matrix->number_of_rows)();
						Matrix out_matrix(number_of_rows,1);

						for (i=0;i<number_of_rows;i++)
						{
							out_matrix(i,0)=(function_matrix->values)(i,column-1);
						}
						out << out_matrix;
					}
				}
				else
				{
					if (0==column)
					{
						Function_size_type i,
							number_of_columns=(function_matrix->number_of_columns)();
						Matrix out_matrix(1,number_of_columns);

						for (i=0;i<number_of_columns;i++)
						{
							out_matrix(0,i)=(function_matrix->values)(row-1,i);
						}
						out << out_matrix;
					}
					else
					{
						Matrix out_matrix(1,1);

						out_matrix(0,0)=(function_matrix->values)(row-1,column-1);
						out << out_matrix;
					}
				}
				*return_string=out.str();
			}

			return (return_string);
		};
		Function_variable_iterator begin_atomic() const
		{
			return (Function_variable_iterator(
				new Function_variable_iterator_representation_atomic_matrix(
				true,Function_variable_matrix_handle(
				const_cast<Function_variable_matrix*>(this)))));
		};
		Function_variable_iterator end_atomic() const
		{
			return (Function_variable_iterator(
				new Function_variable_iterator_representation_atomic_matrix(
				false,Function_variable_matrix_handle(
				const_cast<Function_variable_matrix*>(this)))));
		};
		std::reverse_iterator<Function_variable_iterator> rbegin_atomic() const
		{
			return (std::reverse_iterator<Function_variable_iterator>(
				new Function_variable_iterator_representation_atomic_matrix(
				false,Function_variable_matrix_handle(
				const_cast<Function_variable_matrix*>(this)))));
		};
		std::reverse_iterator<Function_variable_iterator> rend_atomic() const
		{
			return (std::reverse_iterator<Function_variable_iterator>(
				new Function_variable_iterator_representation_atomic_matrix(
				true,Function_variable_matrix_handle(
				const_cast<Function_variable_matrix*>(this)))));
		};
		Function_size_type number_differentiable()
		{
			Function_size_type result;

			result=0;
			if (function_matrix)
			{
				if (0==row)
				{
					if (0==column)
					{
						result=(function_matrix->number_of_rows)()*
							(function_matrix->number_of_columns)();
					}
					else
					{
						result=(function_matrix->number_of_rows)();
					}
				}
				else
				{
					if (0==column)
					{
						result=(function_matrix->number_of_columns)();
					}
					else
					{
						result=1;
					}
				}
			}

			return (result);
		};
		bool equality_atomic(const Function_variable_handle& variable) const
		{
			bool result;
			Function_variable_matrix_handle variable_matrix;

			result=false;
			if (variable_matrix=boost::dynamic_pointer_cast<
				Function_variable_matrix,Function_variable>(variable))
			{
				result=((function_matrix==variable_matrix->function_matrix)&&
					(row==variable_matrix->row)&&(column==variable_matrix->column));
			}

			return (result);
		};
	private:
		// copy constructor
		Function_variable_matrix(const Function_variable_matrix& variable):
			Function_variable(variable),function_matrix(variable.function_matrix),
			column(variable.column),row(variable.row){};
		// assignment
		Function_variable_matrix& operator=(const Function_variable_matrix&);
	private:
		Function_matrix_handle function_matrix;
		Function_size_type column,row;
};

bool is_atomic(Function_variable_matrix_handle variable)
{
	bool result;

	result=false;
	if (variable&&(0!=variable->row)&&(0!=variable->column))
	{
		result=true;
	}

	return (result);
}

static bool Function_variable_matrix_set_scalar_function(Scalar& value,
	const Function_variable_handle variable)
{
	bool result;
	Function_variable_matrix_handle matrix_variable;

	result=false;
	if ((matrix_variable=boost::dynamic_pointer_cast<Function_variable_matrix,
		Function_variable>(variable))&&is_atomic(matrix_variable)&&
		(matrix_variable->function_matrix))
	{
		value=(*(matrix_variable->function_matrix))((matrix_variable->row)-1,
			(matrix_variable->column)-1);
		result=true;
	}

	return (result);
}


// class Function_variable_iterator_representation_atomic_matrix
// -------------------------------------------------------------

Function_variable_iterator_representation_atomic_matrix::
	Function_variable_iterator_representation_atomic_matrix(
	const bool begin,Function_variable_matrix_handle variable):
	atomic_variable(0),variable(variable)
//******************************************************************************
// LAST MODIFIED : 10 March 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	if (begin&&variable)
	{
		if (atomic_variable=boost::dynamic_pointer_cast<Function_variable_matrix,
			Function_variable>(variable->clone()))
		{
			if (0==atomic_variable->row)
			{
				atomic_variable->row=1;
			}
			if (0==atomic_variable->column)
			{
				atomic_variable->column=1;
			}
			atomic_variable->value_private=Function_variable_value_handle(
				new Function_variable_value_scalar(
				Function_variable_matrix_set_scalar_function));
		}
	}
}

Function_variable_iterator_representation
	*Function_variable_iterator_representation_atomic_matrix::clone()
//******************************************************************************
// LAST MODIFIED : 3 March 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_variable_iterator_representation *result;

	result=0;
	if (this)
	{
		result=new Function_variable_iterator_representation_atomic_matrix(*this);
	}

	return (result);
}

Function_variable_iterator_representation_atomic_matrix::
	~Function_variable_iterator_representation_atomic_matrix()
//******************************************************************************
// LAST MODIFIED : 20 February 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	// do nothing
}

void Function_variable_iterator_representation_atomic_matrix::increment()
//******************************************************************************
// LAST MODIFIED : 2 March 2004
//
// DESCRIPTION :
//==============================================================================
{
	if (variable&&(variable->function_matrix)&&(atomic_variable))
	{
		if (0==variable->column)
		{
			(atomic_variable->column)++;
			if (atomic_variable->column>
				(variable->function_matrix->number_of_columns)())
			{
				if (0==variable->row)
				{
					(atomic_variable->column)=1;
					(atomic_variable->row)++;
					if (atomic_variable->row>
						(variable->function_matrix->number_of_rows)())
					{
						// end
						atomic_variable=0;
					}
				}
				else
				{
					// end
					atomic_variable=0;
				}
			}
		}
		else
		{
			if (0==variable->row)
			{
				(atomic_variable->row)++;
				if (atomic_variable->row>
					(variable->function_matrix->number_of_rows)())
				{
					// end
					atomic_variable=0;
				}
			}
			else
			{
				// end
				atomic_variable=0;
			}
		}
	}
}

void Function_variable_iterator_representation_atomic_matrix::decrement()
//******************************************************************************
// LAST MODIFIED : 2 March 2004
//
// DESCRIPTION :
//==============================================================================
{
	if (variable&&(variable->function_matrix))
	{
		if (atomic_variable)
		{
			if (0==variable->column)
			{
				(atomic_variable->column)--;
				if (atomic_variable->column<1)
				{
					if (0==variable->row)
					{
						atomic_variable->column=
							(variable->function_matrix->number_of_columns)();
						(atomic_variable->row)--;
						if (atomic_variable->row<1)
						{
							// end
							atomic_variable=0;
						}
					}
					else
					{
						// end
						atomic_variable=0;
					}
				}
			}
			else
			{
				if (0==variable->row)
				{
					(atomic_variable->row)--;
					if (atomic_variable->row<1)
					{
						// end
						atomic_variable=0;
					}
				}
				else
				{
					// end
					atomic_variable=0;
				}
			}
		}
		else
		{
			if (atomic_variable=boost::dynamic_pointer_cast<Function_variable_matrix,
				Function_variable>(variable->clone()))
			{
				if (0==atomic_variable->row)
				{
					atomic_variable->row=
						(variable->function_matrix->number_of_rows)();
				}
				if (0==atomic_variable->column)
				{
					atomic_variable->column=
						(variable->function_matrix->number_of_columns)();
				}
			}
		}
	}
}

bool Function_variable_iterator_representation_atomic_matrix::
	equality(const Function_variable_iterator_representation * representation)
//******************************************************************************
// LAST MODIFIED : 17 March 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	const Function_variable_iterator_representation_atomic_matrix
		*representation_matrix=dynamic_cast<
		const Function_variable_iterator_representation_atomic_matrix *>(
		representation);

	result=false;
	if (representation_matrix)
	{
		if (((0==atomic_variable)&&(0==representation_matrix->atomic_variable))||
			(atomic_variable&&(representation_matrix->atomic_variable)&&
			(*atomic_variable== *(representation_matrix->atomic_variable))))
		{
			result=true;
		}
	}

	return (result);
}

Function_variable_handle
	Function_variable_iterator_representation_atomic_matrix::dereference() const
//******************************************************************************
// LAST MODIFIED : 3 March 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (atomic_variable);
}

Function_variable_iterator_representation_atomic_matrix::
	Function_variable_iterator_representation_atomic_matrix(const
	Function_variable_iterator_representation_atomic_matrix& representation):
	Function_variable_iterator_representation(),atomic_variable(0),
	variable(representation.variable)
//******************************************************************************
// LAST MODIFIED : 3 March 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
	if (representation.atomic_variable)
	{
		atomic_variable=boost::dynamic_pointer_cast<Function_variable_matrix,
			Function_variable>((representation.atomic_variable)->clone());
	}
}


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
// LAST MODIFIED : 20 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_matrix(
		Function_matrix_handle(this))));
}

Function_variable_handle Function_matrix::output()
//******************************************************************************
// LAST MODIFIED : 20 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_matrix(
		Function_matrix_handle(this))));
}

Function_variable_handle Function_matrix::entry(Function_size_type row,
	Function_size_type column)
//******************************************************************************
// LAST MODIFIED : 17 March 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_matrix(
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
// LAST MODIFIED : 20 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle result(0);
	Matrix result_matrix(1,1);

	if ((Function_handle(this)==(atomic_variable->function)())&&
		Function_variable_matrix_set_scalar_function(
		result_matrix(0,0),atomic_variable))
	{
		result=Function_handle(new Function_matrix(result_matrix));
	}

	return (result);
}

bool Function_matrix::evaluate_derivative(Scalar& derivative,
	Function_variable_handle atomic_variable,
	std::list<Function_variable_handle>& atomic_independent_variables)
//******************************************************************************
// LAST MODIFIED : 5 March 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	Function_variable_matrix_handle atomic_dependent_variable,
		atomic_independent_variable;

	result=false;
	if ((atomic_dependent_variable=boost::dynamic_pointer_cast<
		Function_variable_matrix,Function_variable>(atomic_variable))&&
		(this==atomic_dependent_variable->function_matrix)&&
		is_atomic(atomic_dependent_variable)&&
		(1==atomic_dependent_variable->number_differentiable()))
	{
		result=true;
		if ((1==atomic_independent_variables.size())&&
			(atomic_independent_variable=boost::dynamic_pointer_cast<
			Function_variable_matrix,Function_variable>(
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
// LAST MODIFIED : 10 March 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	Function_variable_matrix_handle atomic_matrix_variable;
	Function_variable_value_scalar_handle value_scalar;

	result=false;
	if ((atomic_matrix_variable=boost::dynamic_pointer_cast<
		Function_variable_matrix,Function_variable>(atomic_variable))&&
		(this==atomic_matrix_variable->function_matrix)&&
		is_atomic(atomic_matrix_variable)&&atomic_value&&(atomic_value->value())&&
		(std::string("Scalar")==(atomic_value->value())->type())&&
		(value_scalar=boost::dynamic_pointer_cast<Function_variable_value_scalar,
		Function_variable_value>(atomic_value->value())))
	{
		result=value_scalar->set(values((atomic_matrix_variable->row)-1,
			(atomic_matrix_variable->column)-1),atomic_value);
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
