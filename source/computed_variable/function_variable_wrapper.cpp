//******************************************************************************
// FILE : function_variable_wrapper.cpp
//
// LAST MODIFIED : 7 July 2004
//
// DESCRIPTION :
//==============================================================================

#include <sstream>

#include "computed_variable/function_variable_wrapper.hpp"

// module classes
// ==============

// class Function_variable_iterator_representation_atomic_wrapper
// --------------------------------------------------------------

class Function_variable_iterator_representation_atomic_wrapper:
	public Function_variable_iterator_representation
//******************************************************************************
// LAST MODIFIED : 7 July 2004
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
			if (begin&&variable&&(variable->wrapped_variable))
			{
				if (atomic_variable=boost::dynamic_pointer_cast<
					Function_variable_wrapper,Function_variable>(variable->clone()))
				{
					atomic_iterator_begin=variable->wrapped_variable->begin_atomic();
					atomic_iterator_end=variable->wrapped_variable->end_atomic();
					if (atomic_iterator_begin!=atomic_iterator_end)
					{
						atomic_iterator=atomic_iterator_begin;
						atomic_variable->wrapped_variable= *atomic_iterator;
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
				atomic_iterator++;
				if (atomic_iterator!=atomic_iterator_end)
				{
					atomic_variable->wrapped_variable= *atomic_iterator;
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
					atomic_iterator--;
					atomic_variable->wrapped_variable= *atomic_iterator;
				}
				else
				{
					atomic_variable=0;
				}
			}
			else
			{
				if (variable&&(variable->wrapped_variable))
				{
					if (atomic_variable=boost::dynamic_pointer_cast<
						Function_variable_wrapper,Function_variable>(variable->clone()))
					{
						atomic_iterator_begin=variable->wrapped_variable->begin_atomic();
						atomic_iterator_end=variable->wrapped_variable->end_atomic();
						if (atomic_iterator_begin!=atomic_iterator_end)
						{
							atomic_iterator=atomic_iterator_end;
							atomic_iterator--;
							atomic_variable->wrapped_variable= *atomic_iterator;
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
				if (
					((0==atomic_variable)&&(0==representation_wrapper->atomic_variable))||
					(atomic_variable&&(representation_wrapper->atomic_variable)&&
					(*atomic_variable== *(representation_wrapper->atomic_variable))))
				{
					result=true;
				}
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
// LAST MODIFIED : 7 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_variable_handle result(0);

	if (wrapped_variable)
	{
		result=Function_variable_handle(new Function_variable_wrapper(
			function(),wrapped_variable->clone()));
	}

	return (result);
}

string_handle Function_variable_wrapper::get_string_representation()
//******************************************************************************
// LAST MODIFIED : 7 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	string_handle return_string(0);

	if (this&&wrapped_variable)
	{
		return_string=wrapped_variable->get_string_representation();
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
// LAST MODIFIED : 7 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_size_type result;

	result=0;
	if (this&&wrapped_variable)
	{
		result=wrapped_variable->number_differentiable();
	}

	return (result);
}

Function_variable_handle Function_variable_wrapper::get_wrapped()
//******************************************************************************
// LAST MODIFIED : 7 July 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (wrapped_variable);
}

bool Function_variable_wrapper::equality_atomic(
	const Function_variable_handle& variable) const
//******************************************************************************
// LAST MODIFIED : 7 July 2004
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
		if ((variable_wrapper->function()==function())&&
			(((0==variable_wrapper->wrapped_variable)&&(0==wrapped_variable))||
			(wrapped_variable&&(variable_wrapper->wrapped_variable)&&
			(*(variable_wrapper->wrapped_variable)== *wrapped_variable))))
		{
			result=true;
		}
	}

	return (result);
}

Function_variable_wrapper::Function_variable_wrapper(
	const Function_handle& wrapping_function,
	const Function_variable_handle& wrapped_variable):
	Function_variable(wrapping_function),wrapped_variable(wrapped_variable){}
//******************************************************************************
// LAST MODIFIED : 7 July 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================

Function_variable_wrapper::Function_variable_wrapper(
	const Function_variable_wrapper& variable_wrapper):
	Function_variable(variable_wrapper),
	wrapped_variable(variable_wrapper.wrapped_variable){}
//******************************************************************************
// LAST MODIFIED : 7 July 2004
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
