//******************************************************************************
// FILE : variable_scalar.cpp
//
// LAST MODIFIED : 26 November 2003
//
// DESCRIPTION :
//???DB.  Should be template?
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

#include "computed_variable/variable_scalar.hpp"

// module classes
// ==============

// class Variable_input_scalar_value
// ---------------------------------

class Variable_input_scalar_value : public Variable_input
//******************************************************************************
// LAST MODIFIED : 26 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	friend class Variable_scalar;
	public:
		Variable_input_scalar_value(const Variable_scalar_handle& variable_scalar):
			variable_scalar(variable_scalar){};
		~Variable_input_scalar_value(){};
		Variable_size_type size()
		{
			return (1);
		};
		virtual bool operator==(const Variable_input& input)
		try
		{
			{
				const Variable_input_scalar_value& input_scalar_value=
					dynamic_cast<const Variable_input_scalar_value&>(input);

				return (variable_scalar==input_scalar_value.variable_scalar);
			}
			catch (std::bad_cast)
			{
				return (false);
			};
		};
	private:
		Variable_scalar_handle variable_scalar;
};

#if defined (USE_INTRUSIVE_SMART_POINTER)
typedef boost::intrusive_ptr<Variable_input_scalar_value>
	Variable_input_scalar_value_handle;
#elif defined (USE_SMART_POINTER)
typedef boost::shared_ptr<Variable_input_scalar_value>
	Variable_input_scalar_value_handle;
#else
typedef Variable_input_scalar_value * Variable_input_scalar_value_handle;
#endif

// global classes
// ==============

// class Variable_scalar
// ---------------------

Variable_scalar::Variable_scalar(const Scalar value):
	Variable(),value(value)
//******************************************************************************
// LAST MODIFIED : 7 October 2003
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
}

Variable_scalar::Variable_scalar(const Variable_scalar& variable_scalar):
	Variable(),value(variable_scalar.value)
//******************************************************************************
// LAST MODIFIED : 11 October 2003
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
}

Variable_scalar& Variable_scalar::operator=(
	const Variable_scalar& variable_scalar)
//******************************************************************************
// LAST MODIFIED : 2 October 2003
//
// DESCRIPTION :
// Assignment operator.
//==============================================================================
{
	this->value=variable_scalar.value;

	return (*this);
}

Variable_size_type Variable_scalar::size()
//******************************************************************************
// LAST MODIFIED : 24 October 2003
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	return (1);
}

Vector *Variable_scalar::scalars()
//******************************************************************************
// LAST MODIFIED : 24 October 2003
//
// DESCRIPTION :
//==============================================================================
{
	Vector *values_vector;

	if (values_vector=new Vector(1))
	{
		(*values_vector)[0]=value;
	}

	return (values_vector);
}

Variable_scalar::~Variable_scalar()
//******************************************************************************
// LAST MODIFIED : 7 September 2003
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	// do nothing
}

Variable_input_handle Variable_scalar::input_value()
//******************************************************************************
// LAST MODIFIED : 11 October 2003
//
// DESCRIPTION :
// Returns the value input for a scalar.
//==============================================================================
{
	return (Variable_input_handle(new Variable_input_scalar_value(
		Variable_scalar_handle(this))));
}

Variable_handle Variable_scalar::evaluate_local()
//******************************************************************************
// LAST MODIFIED : 12 October 2003
//
// DESCRIPTION :
// Duplicate <this> so that <this> isn't changed by operations on the result.
//==============================================================================
{
	return (Variable_handle(new Variable_scalar(*this)));
}

void Variable_scalar::evaluate_derivative_local(Matrix& matrix,
	std::list<Variable_input_handle>& independent_variables)
//******************************************************************************
// LAST MODIFIED : 17 October 2003
//
// DESCRIPTION :
//==============================================================================
{
	Variable_input_scalar_value_handle input_value_handle;

	// matrix is zero'd on entry
	if ((1==independent_variables.size())&&(input_value_handle=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_input_scalar_value,Variable_input>(
		independent_variables.front())
#else /* defined (USE_SMART_POINTER) */
		dynamic_cast<Variable_input_scalar_value *>(independent_variables.front())
#endif /* defined (USE_SMART_POINTER) */
		)&&(input_value_handle->variable_scalar==Variable_handle(this)))
	{
		Assert((1==matrix.size1())&&(1==matrix.size2()),std::logic_error(
			"Variable_scalar::evaluate_derivative_local.  "
			"Incorrect matrix size"));
		matrix(0,0)=1;
	}
}

Variable_handle Variable_scalar::get_input_value_local(
	const Variable_input_handle& input)
//******************************************************************************
// LAST MODIFIED : 15 October 2003
//
// DESCRIPTION :
//==============================================================================
{
	Variable_scalar_handle value_scalar;
	Variable_input_scalar_value_handle input_value_handle;

	if ((input_value_handle=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_input_scalar_value,Variable_input>(
		input)
#else /* defined (USE_SMART_POINTER) */
		dynamic_cast<Variable_input_scalar_value *>(input)
#endif /* defined (USE_SMART_POINTER) */
		)&&(input_value_handle->variable_scalar==Variable_scalar_handle(this)))
	{
		value_scalar=Variable_scalar_handle(new Variable_scalar(*this));
	}
	else
	{
		value_scalar=Variable_scalar_handle((Variable_scalar *)0);
	}

	return (value_scalar);
}

int Variable_scalar::set_input_value_local(const Variable_input_handle& input,
	const Variable_handle& value)
//******************************************************************************
// LAST MODIFIED : 24 October 2003
//
// DESCRIPTION :
//==============================================================================
{
	int return_code;
	Variable_input_scalar_value_handle input_value_handle;
	Vector *values_vector;

	return_code=0;
	if ((input_value_handle=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_input_scalar_value,Variable_input>(
		input)
#else /* defined (USE_SMART_POINTER) */
		dynamic_cast<Variable_input_scalar_value *>(input)
#endif /* defined (USE_SMART_POINTER) */
		)&&(input_value_handle->variable_scalar==Variable_scalar_handle(this))&&
		(1==value->size())&&(values_vector=value->scalars()))
	{
		this->value=(*values_vector)[0];
		return_code=1;
		delete values_vector;
	}

	return (return_code);
}

string_handle Variable_scalar::get_string_representation_local()
//******************************************************************************
// LAST MODIFIED : 12 October 2003
//
// DESCRIPTION :
//==============================================================================
{
	string_handle return_string;
	std::ostringstream out;

	if (return_string=new std::string)
	{
		out << value;
		*return_string=out.str();
	}

	return (return_string);
}
