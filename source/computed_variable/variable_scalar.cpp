//******************************************************************************
// FILE : variable_scalar.cpp
//
// LAST MODIFIED : 9 February 2004
//
// DESCRIPTION :
//???DB.  Should be template?
//==============================================================================

#include "computed_variable/variable_base.hpp"

#include <new>
#include <sstream>
#include <string>
#include <stdio.h>

#include "computed_variable/variable_scalar.hpp"

// module classes
// ==============

// class Variable_input_scalar_value
// ---------------------------------

class Variable_input_scalar_value;

#if defined (USE_INTRUSIVE_SMART_POINTER)
typedef boost::intrusive_ptr<Variable_input_scalar_value>
	Variable_input_scalar_value_handle;
#elif defined (USE_SMART_POINTER)
typedef boost::shared_ptr<Variable_input_scalar_value>
	Variable_input_scalar_value_handle;
#else
typedef Variable_input_scalar_value * Variable_input_scalar_value_handle;
#endif

class Variable_input_scalar_value : public
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
	friend class Variable_scalar;
	public:
		Variable_input_scalar_value(const Variable_scalar_handle& variable_scalar):
			variable_scalar(variable_scalar){};
		~Variable_input_scalar_value(){};
#if defined (USE_ITERATORS)
		// copy constructor
		Variable_input_scalar_value(
			const Variable_input_scalar_value& input_scalar_value):
#if defined (USE_VARIABLE_INPUT)
			Variable_input
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
			(),
			variable_scalar(input_scalar_value.variable_scalar){};
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			clone() const
		{
			return (Variable_input_scalar_value_handle(
				new Variable_input_scalar_value(*this)));
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
			return (1);
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
				{
					const Variable_input_scalar_value& input_scalar_value=
						dynamic_cast<const Variable_input_scalar_value&>(input);

					return (variable_scalar==input_scalar_value.variable_scalar);
				}
			}
			catch (std::bad_cast)
			{
				return (false);
			}
		};
	private:
		Variable_scalar_handle variable_scalar;
};

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

Variable_size_type Variable_scalar::
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
// Destructor.
//==============================================================================
{
	return (1);
}

#if defined (USE_ITERATORS)
#else // defined (USE_ITERATORS)
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
#endif // defined (USE_VARIABLE_ITERATORS)

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

#if defined (USE_VARIABLE_INPUT)
Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	Variable_scalar::input_value()
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
// Returns the value input for a scalar.
//==============================================================================
{
	return (
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
		(new Variable_input_scalar_value(Variable_scalar_handle(this))));
}

Scalar Variable_scalar::norm() const
//******************************************************************************
// LAST MODIFIED : 11 December 2003
//
// DESCRIPTION :
//==============================================================================
{
	Scalar result;

	result=value;
	if (result<0)
	{
		result= -result;
	}

	return (result);
}

Variable_handle Variable_scalar::operator-(const Variable& second) const
//******************************************************************************
// LAST MODIFIED : 15 December 2003
//
// DESCRIPTION :
//==============================================================================
{
	Variable_scalar_handle result(0);

	try
	{
		const Variable_scalar& second_scalar=
			dynamic_cast<const Variable_scalar&>(second);

		if (this&&(result=Variable_scalar_handle(new Variable_scalar(*this))))
		{
			result->value -= second_scalar.value;
		}
	}
	catch (std::bad_cast)
	{
		// do nothing
	}

	return (result);
}

Variable_handle Variable_scalar::operator-=(const Variable& second)
//******************************************************************************
// LAST MODIFIED : 15 December 2003
//
// DESCRIPTION :
//==============================================================================
{
	try
	{
		const Variable_scalar& second_scalar=
			dynamic_cast<const Variable_scalar&>(second);

		if (this)
		{
			value -= second_scalar.value;
		}
	}
	catch (std::bad_cast)
	{
		// do nothing
	}

	return (Variable_scalar_handle(this));
}

Variable_handle Variable_scalar::clone() const
//******************************************************************************
// LAST MODIFIED : 8 December 2003
//
// DESCRIPTION :
//==============================================================================
{
	return (Variable_scalar_handle(new Variable_scalar(*this)));
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

#if defined (USE_ITERATORS)
//???DB.  To be done
#else // defined (USE_ITERATORS)
bool Variable_scalar::evaluate_derivative_local(Matrix& matrix,
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
	Variable_input_scalar_value_handle input_value_handle;

	result=true;
	// matrix is zero'd on entry
	if ((1==independent_variables.size())&&(input_value_handle=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_input_scalar_value,
#if defined (USE_VARIABLE_INPUT)
		Variable_input
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
		>(independent_variables.front())
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

	return (result);
}
#endif // defined (USE_ITERATORS)

Variable_handle Variable_scalar::get_input_value_local(
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
	Variable_scalar_handle value_scalar;
	Variable_input_scalar_value_handle input_value_handle;

	if ((input_value_handle=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_input_scalar_value,
#if defined (USE_VARIABLE_INPUT)
		Variable_input
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
		>(input)
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

bool Variable_scalar::set_input_value_local(const
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
	& value)
//******************************************************************************
// LAST MODIFIED : 9 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	Variable_input_scalar_value_handle input_value_handle;
	Vector *values_vector;

	result=false;
	if ((input_value_handle=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_input_scalar_value,
#if defined (USE_VARIABLE_INPUT)
		Variable_input
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
		>(input)
#else /* defined (USE_SMART_POINTER) */
		dynamic_cast<Variable_input_scalar_value *>(input)
#endif /* defined (USE_SMART_POINTER) */
		)&&(input_value_handle->variable_scalar==Variable_scalar_handle(this))&&
		(1==value->
#if defined (USE_ITERATORS)
		number_differentiable
#else // defined (USE_ITERATORS)
		size
#endif // defined (USE_VARIABLE_ITERATORS)
		())&&(values_vector=
#if defined (USE_ITERATORS)
		//???DB.  To be done
		0
#else // defined (USE_ITERATORS)
		value->scalars()
#endif // defined (USE_VARIABLE_ITERATORS)
		))
	{
		this->value=(*values_vector)[0];
		delete values_vector;
		result=true;
	}

	return (result);
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
