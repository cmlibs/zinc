//******************************************************************************
// FILE : function_variable_wrapper.cpp
//
// LAST MODIFIED : 10 May 2005
//
// DESCRIPTION :
//==============================================================================

#include <sstream>

#include "computed_variable/function.hpp"
#include "computed_variable/function_variable_wrapper.hpp"

#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#include "computed_variable/function_derivative.hpp"
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

// module classes
// ==============

#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
// class Function_derivatnew_wrapper
// ---------------------------------

class Function_derivatnew_wrapper : public Function_derivatnew
//******************************************************************************
// LAST MODIFIED : 10 May 2005
//
// DESCRIPTION :
//==============================================================================
{
	public:
		// for construction exception
		class Construction_exception {};
		// constructor
		Function_derivatnew_wrapper(
			const Function_variable_handle& dependent_variable,
			const std::list<Function_variable_handle>& independent_variables);
		// destructor
		~Function_derivatnew_wrapper();
	// inherited
	private:
#if defined (EVALUATE_RETURNS_VALUE)
		virtual Function_handle evaluate(Function_variable_handle atomic_variable);
#else // defined (EVALUATE_RETURNS_VALUE)
		virtual bool evaluate(Function_variable_handle atomic_variable);
#endif // defined (EVALUATE_RETURNS_VALUE)
	private:
		Function_derivatnew_handle wrapped_derivative;
};

Function_derivatnew_wrapper::Function_derivatnew_wrapper(
	const Function_variable_handle& dependent_variable,
	const std::list<Function_variable_handle>& independent_variables):
	Function_derivatnew(dependent_variable,independent_variables)
//******************************************************************************
// LAST MODIFIED : 10 May 2005
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	Function_variable_wrapper_handle variable_wrapper;

	if (variable_wrapper=boost::dynamic_pointer_cast<
		Function_variable_wrapper,Function_variable>(dependent_variable))
	{
		Function_variable_handle wrapped_variable;

		if ((wrapped_variable=variable_wrapper->get_wrapped())&&
			(wrapped_derivative=boost::dynamic_pointer_cast<
			Function_derivatnew,Function>(wrapped_variable->derivative(
			independent_variables))))
		{
			wrapped_derivative->add_dependent_function(this);
		}
		else
		{
			throw Function_derivatnew_wrapper::Construction_exception();
		}
	}
	else
	{
		throw Function_derivatnew_wrapper::Construction_exception();
	}
}

Function_derivatnew_wrapper::~Function_derivatnew_wrapper()
//******************************************************************************
// LAST MODIFIED : 10 May 2005
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
#if defined (CIRCULAR_SMART_POINTERS)
	// do nothing
#else // defined (CIRCULAR_SMART_POINTERS)
	if (wrapped_derivative)
	{
		wrapped_derivative->remove_dependent_function(this);
	}
#endif // defined (CIRCULAR_SMART_POINTERS)
}

#if defined (EVALUATE_RETURNS_VALUE)
Function_handle
#else // defined (EVALUATE_RETURNS_VALUE)
bool
#endif // defined (EVALUATE_RETURNS_VALUE)
	Function_derivatnew_wrapper::evaluate(
	Function_variable_handle
#if defined (EVALUATE_RETURNS_VALUE)
	atomic_variable
#else // defined (EVALUATE_RETURNS_VALUE)
#endif // defined (EVALUATE_RETURNS_VALUE)
	)
//******************************************************************************
// LAST MODIFIED : 10 May 2005
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
		Function_variable_wrapper_handle dependent_variable_wrapper;

#if defined (EVALUATE_RETURNS_VALUE)
#else // defined (EVALUATE_RETURNS_VALUE)
		result=false;
#endif // defined (EVALUATE_RETURNS_VALUE)
		if (dependent_variable_wrapper=boost::dynamic_pointer_cast<
			Function_variable_wrapper,Function_variable>(dependent_variable))
		{
			Function_variable_handle wrapped_output;

			derivative_matrix.clear();
			if ((wrapped_output=wrapped_derivative->output())&&
				(wrapped_output->evaluate()))
			{
				//???DB.  Should be able to get away without doing this assignment, but
				//  not sure how to do get_value (overloaded).  Need to know about
				//  Function_variable_derivatnew?
				derivative_matrix=wrapped_derivative->derivative_matrix;
				set_evaluated();
#if defined (EVALUATE_RETURNS_VALUE)
#else // defined (EVALUATE_RETURNS_VALUE)
				result=true;
#endif // defined (EVALUATE_RETURNS_VALUE)
			}
		}
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


// class Function_variable_iterator_representation_atomic_wrapper
// --------------------------------------------------------------

class Function_variable_iterator_representation_atomic_wrapper:
	public Function_variable_iterator_representation
//******************************************************************************
// LAST MODIFIED : 7 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	public:
		// constructor
		Function_variable_iterator_representation_atomic_wrapper(
			const bool begin,Function_variable_wrapper_handle variable):
			atomic_variable(0),variable(variable),atomic_iterator(0),
			atomic_iterator_begin(0),atomic_iterator_end(0)
		{
			Function_variable_handle wrapped_variable;

			if (begin&&variable&&(wrapped_variable=variable->get_wrapped()))
			{
				if (atomic_variable=boost::dynamic_pointer_cast<
					Function_variable_wrapper,Function_variable>(variable->clone()))
				{
					atomic_iterator_begin=wrapped_variable->begin_atomic();
					atomic_iterator_end=wrapped_variable->end_atomic();
					if (atomic_iterator_begin!=atomic_iterator_end)
					{
						atomic_iterator=atomic_iterator_begin;
						atomic_variable->working_variable= *atomic_iterator;
					}
					else
					{
						// end
						atomic_variable=0;
					}
				}
			}
		};
		// a "virtual" constructor
		Function_variable_iterator_representation *clone()
		{
			Function_variable_iterator_representation *result;

			result=0;
			if (this)
			{
				result=new Function_variable_iterator_representation_atomic_wrapper(
					*this);
			}

			return (result);
		};
	protected:
		// destructor
		~Function_variable_iterator_representation_atomic_wrapper(){};
	private:
		// increment
		void increment()
		{
			if (atomic_variable)
			{
				++atomic_iterator;
				if (atomic_iterator!=atomic_iterator_end)
				{
					atomic_variable->working_variable= *atomic_iterator;
				}
				else
				{
					atomic_variable=0;
				}
			}
		};
		// decrement
		void decrement()
		{
			if (atomic_variable)
			{
				if (atomic_iterator!=atomic_iterator_begin)
				{
					--atomic_iterator;
					atomic_variable->working_variable= *atomic_iterator;
				}
				else
				{
					atomic_variable=0;
				}
			}
			else
			{
				Function_variable_handle wrapped_variable;

				if (variable&&(wrapped_variable=variable->get_wrapped()))
				{
					if (atomic_variable=boost::dynamic_pointer_cast<
						Function_variable_wrapper,Function_variable>(variable->clone()))
					{
						atomic_iterator_begin=wrapped_variable->begin_atomic();
						atomic_iterator_end=wrapped_variable->end_atomic();
						if (atomic_iterator_begin!=atomic_iterator_end)
						{
							atomic_iterator=atomic_iterator_end;
							--atomic_iterator;
							atomic_variable->working_variable= *atomic_iterator;
						}
						else
						{
							// end
							atomic_variable=0;
						}
					}
				}
			}
		};
		// equality
		bool equality(
			const Function_variable_iterator_representation* representation)
		{
			bool result;
			const Function_variable_iterator_representation_atomic_wrapper
				*representation_wrapper=dynamic_cast<const
				Function_variable_iterator_representation_atomic_wrapper *>(
				representation);

			result=false;
			if (representation_wrapper)
			{
				result=equivalent(atomic_variable,
					representation_wrapper->atomic_variable);
			}

			return (result);
		};
		// dereference
		Function_variable_handle dereference() const
		{
			return (atomic_variable);
		};
	private:
		// copy constructor
		Function_variable_iterator_representation_atomic_wrapper(const
			Function_variable_iterator_representation_atomic_wrapper&
			representation):Function_variable_iterator_representation(),
			atomic_variable(0),variable(representation.variable),
			atomic_iterator(representation.atomic_iterator),
			atomic_iterator_begin(representation.atomic_iterator_begin),
			atomic_iterator_end(representation.atomic_iterator_end)
		{
			if (representation.atomic_variable)
			{
				atomic_variable=boost::dynamic_pointer_cast<
					Function_variable_wrapper,Function_variable>(
					(representation.atomic_variable)->clone());
			}
		};
	private:
		Function_variable_wrapper_handle atomic_variable,variable;
		Function_variable_iterator atomic_iterator,atomic_iterator_begin,
			atomic_iterator_end;
};


// global classes
// ==============

// class Function_variable_wrapper
// -------------------------------

Function_variable_handle Function_variable_wrapper::clone() const
//******************************************************************************
// LAST MODIFIED : 22 November 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_variable_handle result(0),wrapped_variable;

	if (this&&(wrapped_variable=get_wrapped()))
	{
		wrapped_variable=wrapped_variable->clone();
		result=Function_variable_handle(new Function_variable_wrapper(
			function(),wrapped_variable));
	}

	return (result);
}

#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
Function_handle Function_variable_wrapper::derivative(
	const std::list<Function_variable_handle>& independent_variables)
//******************************************************************************
// LAST MODIFIED : 10 May 2005
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_handle(new Function_derivatnew_wrapper(
		Function_variable_handle(this),independent_variables)));
}
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

Function_variable_value_handle Function_variable_wrapper::value()
//******************************************************************************
// LAST MODIFIED : 18 August 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_variable_handle wrapped_variable;
	Function_variable_value_handle result(0);

	if (this&&(wrapped_variable=get_wrapped()))
	{
		result=wrapped_variable->value();
	}

	return (result);
}

bool Function_variable_wrapper::set_value(Function_handle value)
//******************************************************************************
// LAST MODIFIED : 18 August 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result(false);
	Function_variable_handle wrapped_variable;

	if (this&&(wrapped_variable=get_wrapped()))
	{
		result=wrapped_variable->set_value(value);
	}

	return (result);
}

bool Function_variable_wrapper::rset_value(Function_handle value)
//******************************************************************************
// LAST MODIFIED : 18 August 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result(false);
	Function_variable_handle wrapped_variable;

	if (this&&(wrapped_variable=get_wrapped()))
	{
		result=wrapped_variable->rset_value(value);
	}

	return (result);
}

Function_handle Function_variable_wrapper::get_value() const
//******************************************************************************
// LAST MODIFIED : 18 August 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle result(0);
	Function_variable_handle wrapped_variable;

	if (this&&(wrapped_variable=this->get_wrapped()))
	{
		result=wrapped_variable->get_value();
	}

	return (result);
}

string_handle Function_variable_wrapper::get_string_representation()
//******************************************************************************
// LAST MODIFIED : 18 August 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_variable_handle wrapped_variable;
	string_handle return_string(0);

	if (this&&(wrapped_variable=get_wrapped()))
	{
		if (return_string=new std::string)
		{
			std::ostringstream out;

			out << "wrapped(";
			out << *(wrapped_variable->get_string_representation());
			out << ")";
			*return_string=out.str();
		}
	}

	return (return_string);
}

Function_variable_iterator Function_variable_wrapper::begin_atomic() const
//******************************************************************************
// LAST MODIFIED : 7 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_iterator(new
		Function_variable_iterator_representation_atomic_wrapper(
		true,Function_variable_wrapper_handle(
		const_cast<Function_variable_wrapper*>(this)))));
}

Function_variable_iterator Function_variable_wrapper::end_atomic() const
//******************************************************************************
// LAST MODIFIED : 7 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_iterator(new
		Function_variable_iterator_representation_atomic_wrapper(
		false,Function_variable_wrapper_handle(
		const_cast<Function_variable_wrapper*>(this)))));
}

std::reverse_iterator<Function_variable_iterator>
	Function_variable_wrapper::rbegin_atomic() const
//******************************************************************************
// LAST MODIFIED : 7 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (std::reverse_iterator<Function_variable_iterator>(new
		Function_variable_iterator_representation_atomic_wrapper(
		false,Function_variable_wrapper_handle(
		const_cast<Function_variable_wrapper*>(this)))));
}

std::reverse_iterator<Function_variable_iterator>
	Function_variable_wrapper::rend_atomic() const
//******************************************************************************
// LAST MODIFIED : 7 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (std::reverse_iterator<Function_variable_iterator>(new
		Function_variable_iterator_representation_atomic_wrapper(
		true,Function_variable_wrapper_handle(
		const_cast<Function_variable_wrapper*>(this)))));
}

Function_size_type Function_variable_wrapper::number_differentiable()
//******************************************************************************
// LAST MODIFIED : 18 August 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_variable_handle wrapped_variable;
	Function_size_type result;

	result=0;
	if (this&&(wrapped_variable=get_wrapped()))
	{
		result=wrapped_variable->number_differentiable();
	}

	return (result);
}

Function_variable_handle Function_variable_wrapper::operator-(
	const Function_variable& second) const
//******************************************************************************
// LAST MODIFIED : 18 August 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_variable_handle wrapped_variable,result(0);

	if (this&&(wrapped_variable=get_wrapped()))
	{
		result=wrapped_variable->operator-(second);
	}

	return (result);
}

Function_variable_handle Function_variable_wrapper::get_wrapped() const
//******************************************************************************
// LAST MODIFIED : 18 August 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (working_variable);
}

bool Function_variable_wrapper::equality_atomic(
	const Function_variable_handle& variable) const
//******************************************************************************
// LAST MODIFIED : 18 August 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	Function_variable_wrapper_handle variable_wrapper;

	result=false;
	if (variable_wrapper=boost::dynamic_pointer_cast<
		Function_variable_wrapper,Function_variable>(variable))
	{
		if (equivalent(variable_wrapper->function(),function())&&
			equivalent(variable_wrapper->get_wrapped(),get_wrapped()))
		{
			result=true;
		}
	}

	return (result);
}

Function_variable_wrapper::Function_variable_wrapper(
	const Function_handle& wrapping_function,
	const Function_variable_handle& wrapped_variable):
	Function_variable(wrapping_function),working_variable(wrapped_variable){}
//******************************************************************************
// LAST MODIFIED : 18 August 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================

Function_variable_wrapper::Function_variable_wrapper(
	const Function_variable_wrapper& variable_wrapper):
	Function_variable(variable_wrapper),
	working_variable(variable_wrapper.working_variable){}
//******************************************************************************
// LAST MODIFIED : 18 August 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================

Function_variable_wrapper::~Function_variable_wrapper(){}
//******************************************************************************
// LAST MODIFIED : 7 July 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================
