//******************************************************************************
// FILE : function_identity.cpp
//
// LAST MODIFIED : 7 July 2004
//
// DESCRIPTION :
//???DB.  Need to be able to get to the variable it wraps so that can do
//  operations like += for variables?
//???DB.  Are union, intersection, composite wrappers?
//==============================================================================

#include <sstream>

#include "computed_variable/function_identity.hpp"
#if defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
#include "computed_variable/function_variable.hpp"
#else // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
#include "computed_variable/function_variable_wrapper.hpp"
#endif // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)

// module classes
// ==============

#if defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
// forward declaration so that can use _handle
class Function_variable_identity;
typedef boost::intrusive_ptr<Function_variable_identity>
	Function_variable_identity_handle;

// class Function_variable_iterator_representation_atomic_identity
// ---------------------------------------------------------------

class Function_variable_iterator_representation_atomic_identity:
	public Function_variable_iterator_representation
//******************************************************************************
// LAST MODIFIED : 25 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	public:
		// constructor
		Function_variable_iterator_representation_atomic_identity(
			const bool begin,Function_variable_identity_handle variable);
		// a "virtual" constructor
		Function_variable_iterator_representation *clone();
	protected:
		// destructor
		~Function_variable_iterator_representation_atomic_identity();
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
		Function_variable_iterator_representation_atomic_identity(const
			Function_variable_iterator_representation_atomic_identity&);
	private:
		Function_variable_identity_handle atomic_variable,variable;
		Function_variable_iterator atomic_iterator,atomic_iterator_begin,
			atomic_iterator_end;
};


// class Function_variable_identity
// --------------------------------

class Function_variable_identity : public Function_variable
//******************************************************************************
// LAST MODIFIED : 30 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	friend class Function_identity;
	friend class Function_variable_iterator_representation_atomic_identity;
	public:
		// constructor
		Function_variable_identity(
			const Function_identity_handle& function_identity):
			Function_variable(function_identity),atomic_variable(0){};
		// constructor.  A zero <atomic_variable> indicates the whole variable
		Function_variable_identity(
			const Function_identity_handle& function_identity,
			Function_variable_handle& atomic_variable):
			Function_variable(function_identity),atomic_variable(0)
		{
			if (function_identity)
			{
				if (atomic_variable&&(function_identity->variable_private))
				{
					Function_variable_iterator atomic_variable_iterator,
						end_atomic_variable_iterator;

					atomic_variable_iterator=
						function_identity->variable_private->begin_atomic();
					end_atomic_variable_iterator=
						function_identity->variable_private->end_atomic();
					while ((atomic_variable_iterator!=end_atomic_variable_iterator)&&
						(*atomic_variable_iterator!=atomic_variable))
					{
						atomic_variable_iterator++;
					}
					if (atomic_variable_iterator!=end_atomic_variable_iterator)
					{
						this->atomic_variable=atomic_variable;
					}
				}
			}
		};
		// destructor
		~Function_variable_identity(){};
	// inherited
	public:
		Function_variable_handle clone() const
		{
			return (Function_variable_identity_handle(
				new Function_variable_identity(*this)));
		};
		string_handle get_string_representation()
		{
			string_handle return_string(0);

			if (this&&(return_string=new std::string))
			{
				Function_identity_handle function_identity=
					boost::dynamic_pointer_cast<Function_identity,Function>(
					function());
				std::ostringstream out;

				out << "identity(";
				if (atomic_variable)
				{
					out << *(atomic_variable->get_string_representation());
				}
				else
				{
					if (function_identity&&(function_identity->variable_private))
					{
						out << *(function_identity->variable_private->
							get_string_representation());
					}
				}
				out << ")";
				*return_string=out.str();
			}

			return (return_string);
		};
		virtual Function_variable_iterator begin_atomic() const
		{
			return (Function_variable_iterator(new
				Function_variable_iterator_representation_atomic_identity(
				true,Function_variable_identity_handle(
				const_cast<Function_variable_identity*>(this)))));
		};
		virtual Function_variable_iterator end_atomic() const
		{
			return (Function_variable_iterator(new
				Function_variable_iterator_representation_atomic_identity(
				false,Function_variable_identity_handle(
				const_cast<Function_variable_identity*>(this)))));
		};
		std::reverse_iterator<Function_variable_iterator> rbegin_atomic() const
		{
			return (std::reverse_iterator<Function_variable_iterator>(new
				Function_variable_iterator_representation_atomic_identity(
				false,Function_variable_identity_handle(
				const_cast<Function_variable_identity*>(this)))));
		};
		std::reverse_iterator<Function_variable_iterator> rend_atomic() const
		{
			return (std::reverse_iterator<Function_variable_iterator>(new
				Function_variable_iterator_representation_atomic_identity(
				true,Function_variable_identity_handle(
				const_cast<Function_variable_identity*>(this)))));
		};
		Function_size_type number_differentiable()
		{
			Function_size_type result;

			result=0;
			if (this)
			{
				Function_identity_handle function_identity=
					boost::dynamic_pointer_cast<Function_identity,Function>(
					function());

				if (atomic_variable)
				{
					result=atomic_variable->number_differentiable();
				}
				else
				{
					if (function_identity&&(function_identity->variable_private))
					{
						result=function_identity->variable_private->number_differentiable();
					}
				}
			}

			return (result);
		};
		bool equality_atomic(const Function_variable_handle& variable) const
		{
			bool result;
			Function_variable_identity_handle variable_identity;

			result=false;
			if (variable_identity=boost::dynamic_pointer_cast<
				Function_variable_identity,Function_variable>(variable))
			{
				if ((variable_identity->function()==function())&&
					(((0==variable_identity->atomic_variable)&&
					(0==atomic_variable))||(atomic_variable&&
					(variable_identity->atomic_variable)&&
					(*(variable_identity->atomic_variable)== *atomic_variable))))
				{
					result=true;
				}
			}

			return (result);
		};
	private:
		bool is_atomic()
		{
			bool result;

			result=false;
			if (this&&atomic_variable)
			{
				result=true;
			}

			return (result);
		};
	private:
		// copy constructor
		Function_variable_identity(
			const Function_variable_identity& variable_identity):
			Function_variable(variable_identity),
			atomic_variable(variable_identity.atomic_variable){};
		// assignment
		Function_variable_identity& operator=(
			const Function_variable_identity&);
	private:
		// if zero then all
		Function_variable_handle atomic_variable;
};


// class Function_variable_iterator_representation_atomic_identity
// ---------------------------------------------------------------

Function_variable_iterator_representation_atomic_identity::
	Function_variable_iterator_representation_atomic_identity(
	const bool begin,Function_variable_identity_handle variable):
	atomic_variable(0),variable(variable),atomic_iterator(0),
	atomic_iterator_begin(0),atomic_iterator_end(0)
//******************************************************************************
// LAST MODIFIED : 30 June 2004
//
// DESCRIPTION :
// Constructor.  If <begin> then the constructed iterator points to the first
// atomic variable, otherwise it points to one past the last atomic variable.
//==============================================================================
{
	if (begin&&variable)
	{
		Function_identity_handle function_identity=
			boost::dynamic_pointer_cast<Function_identity,Function>(
			variable->function());

		if (function_identity&&(atomic_variable=boost::dynamic_pointer_cast<
			Function_variable_identity,Function_variable>(variable->clone())))
		{
			atomic_iterator=0;
			atomic_iterator_begin=0;
			atomic_iterator_end=0;
			if (variable->atomic_variable)
			{
				atomic_iterator_begin=variable->atomic_variable->begin_atomic();
				atomic_iterator_end=variable->atomic_variable->end_atomic();
			}
			else
			{
				Function_variable_handle variable_local;

				if (variable_local=function_identity->variable_private)
				{
					atomic_iterator_begin=variable_local->begin_atomic();
					atomic_iterator_end=variable_local->end_atomic();
				}
				else
				{
					// end
					atomic_variable=0;
				}
			}
			if (atomic_variable&&(atomic_iterator_begin!=atomic_iterator_end))
			{
				atomic_iterator=atomic_iterator_begin;
			}
			else
			{
				// end
				atomic_variable=0;
			}
		}
	}
}

Function_variable_iterator_representation
	*Function_variable_iterator_representation_atomic_identity::clone()
//******************************************************************************
// LAST MODIFIED : 25 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_variable_iterator_representation *result;

	result=0;
	if (this)
	{
		result=new Function_variable_iterator_representation_atomic_identity(
			*this);
	}

	return (result);
}

Function_variable_iterator_representation_atomic_identity::
	~Function_variable_iterator_representation_atomic_identity()
//******************************************************************************
// LAST MODIFIED : 25 June 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	// do nothing
}

void Function_variable_iterator_representation_atomic_identity::increment()
//******************************************************************************
// LAST MODIFIED : 25 June 2004
//
// DESCRIPTION :
// Increments the iterator to the next atomic variable.  NULL <atomic_variable>
// is the end iterator.
//==============================================================================
{
	if (atomic_variable)
	{
		atomic_iterator++;
		if (atomic_iterator!=atomic_iterator_end)
		{
			atomic_variable->atomic_variable= *atomic_iterator;
		}
		else
		{
			atomic_variable=0;
		}
	}
}

void Function_variable_iterator_representation_atomic_identity::decrement()
//******************************************************************************
// LAST MODIFIED : 25 June 2004
//
// DESCRIPTION :
// Decrements the iterator to the next atomic variable.  NULL <atomic_variable>
// is the end iterator.
//==============================================================================
{
	if (atomic_variable)
	{
		if (atomic_iterator!=atomic_iterator_begin)
		{
			atomic_iterator--;
			atomic_variable->atomic_variable= *atomic_iterator;
		}
		else
		{
			atomic_variable=0;
		}
	}
	else
	{
		if (variable)
		{
			Function_identity_handle function_identity=
				boost::dynamic_pointer_cast<Function_identity,Function>(
				variable->function());

			if (function_identity&&(atomic_variable=boost::dynamic_pointer_cast<
				Function_variable_identity,Function_variable>(variable->clone())))
			{
				atomic_iterator=0;
				atomic_iterator_begin=0;
				atomic_iterator_end=0;
				if (variable->atomic_variable)
				{
					atomic_iterator_begin=variable->atomic_variable->begin_atomic();
					atomic_iterator_end=variable->atomic_variable->end_atomic();
				}
				else
				{
					Function_variable_handle variable_local;

					if (variable_local=function_identity->variable_private)
					{
						atomic_iterator_begin=variable_local->begin_atomic();
						atomic_iterator_end=variable_local->end_atomic();
					}
					else
					{
						// end
						atomic_variable=0;
					}
				}
				if (atomic_variable&&(atomic_iterator_begin!=atomic_iterator_end))
				{
					atomic_iterator=atomic_iterator_end;
					atomic_iterator--;
					atomic_variable->atomic_variable= *atomic_iterator;
				}
				else
				{
					// end
					atomic_variable=0;
				}
			}
		}
	}
}

bool Function_variable_iterator_representation_atomic_identity::equality(
	const Function_variable_iterator_representation * representation)
//******************************************************************************
// LAST MODIFIED : 25 June 2004
//
// DESCRIPTION :
// Tests <*this> and <*representation> for equality.
//==============================================================================
{
	bool result;
	const Function_variable_iterator_representation_atomic_identity
		*representation_identity=dynamic_cast<const
		Function_variable_iterator_representation_atomic_identity *>(
		representation);

	result=false;
	if (representation_identity)
	{
		if (((0==atomic_variable)&&(0==representation_identity->atomic_variable))||
			(atomic_variable&&(representation_identity->atomic_variable)&&
			(*atomic_variable== *(representation_identity->atomic_variable))))
		{
			result=true;
		}
	}

	return (result);
}

Function_variable_handle
	Function_variable_iterator_representation_atomic_identity::dereference() const
//******************************************************************************
// LAST MODIFIED : 25 June 2004
//
// DESCRIPTION :
// Returns the atomic variable for the iterator.
//==============================================================================
{
	return (atomic_variable);
}

Function_variable_iterator_representation_atomic_identity::
	Function_variable_iterator_representation_atomic_identity(const
	Function_variable_iterator_representation_atomic_identity&
	representation):Function_variable_iterator_representation(),
	atomic_variable(0),variable(representation.variable),
	atomic_iterator(representation.atomic_iterator),
	atomic_iterator_begin(representation.atomic_iterator_begin),
	atomic_iterator_end(representation.atomic_iterator_end)
//******************************************************************************
// LAST MODIFIED : 25 June 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
	if (representation.atomic_variable)
	{
		atomic_variable=boost::dynamic_pointer_cast<
			Function_variable_identity,Function_variable>(
			(representation.atomic_variable)->clone());
	}
}
#else // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
#endif // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)


// global classes
// ==============

// class Function_identity
// -----------------------

Function_identity::Function_identity(const Function_variable_handle& variable):
	Function(),variable_private(variable){}
//******************************************************************************
// LAST MODIFIED : 25 June 2004
//
// DESCRIPTION :
//==============================================================================

Function_identity::~Function_identity(){}
//******************************************************************************
// LAST MODIFIED : 25 June 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================

string_handle Function_identity::get_string_representation()
//******************************************************************************
// LAST MODIFIED : 25 June 2004
//
// DESCRIPTION :
//???DB.  Overload << instead of get_string_representation?
//==============================================================================
{
	string_handle return_string(0);

	if (return_string=new std::string)
	{
		std::ostringstream out;

		out << "identity(";
		if (variable_private)
		{
			out << *(variable_private->get_string_representation());
		}
		out << ")";
		*return_string=out.str();
	}

	return (return_string);
}

Function_variable_handle Function_identity::input()
//******************************************************************************
// LAST MODIFIED : 25 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new
#if defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
		Function_variable_identity(Function_identity_handle(this))
#else // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
		Function_variable_wrapper(Function_handle(this),variable_private)
#endif // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
		));
}

Function_variable_handle Function_identity::output()
//******************************************************************************
// LAST MODIFIED : 25 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new
#if defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
		Function_variable_identity(Function_identity_handle(this))
#else // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
		Function_variable_wrapper(Function_handle(this),variable_private)
#endif // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
		));
}

Function_handle Function_identity::evaluate(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 30 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle result(0);
#if defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
	Function_variable_identity_handle	
#else // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
	Function_variable_wrapper_handle	
#endif // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
		atomic_identity_variable;

	if ((atomic_identity_variable=boost::dynamic_pointer_cast<
#if defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
		Function_variable_identity
#else // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
		Function_variable_wrapper
#endif // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
		,Function_variable>(atomic_variable))&&
		(Function_handle(this)==atomic_identity_variable->function())
#if defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
		&&(atomic_identity_variable->is_atomic)()&&
		(atomic_identity_variable->atomic_variable)
#else // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
#endif // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
		)
	{
		result=(atomic_identity_variable->
#if defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
			atomic_variable
#else // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
			get_wrapped()
#endif // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
			->get_value)();
	}

	return (result);
}

bool Function_identity::evaluate_derivative(Scalar& derivative,
	Function_variable_handle atomic_variable,
	std::list<Function_variable_handle>& atomic_independent_variables)
//******************************************************************************
// LAST MODIFIED : 30 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	Function_variable_handle atomic_variable_local,atomic_independent_variable;
#if defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
	Function_variable_identity_handle
#else // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
	Function_variable_wrapper_handle
#endif // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
		atomic_variable_identity;

	result=false;
	if ((atomic_variable_identity=boost::dynamic_pointer_cast<
#if defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
		Function_variable_identity
#else // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
		Function_variable_wrapper
#endif // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
		,Function_variable>(atomic_variable))&&
		(Function_handle(this)==atomic_variable_identity->function())&&
#if defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
		(atomic_variable_identity->is_atomic)()&&
#else // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
#endif // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
		(1==atomic_variable_identity->number_differentiable())&&
		(atomic_variable_local=atomic_variable_identity->
#if defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
		atomic_variable
#else // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
		get_wrapped()
#endif // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
		))
	{
		result=true;
		if ((1==atomic_independent_variables.size())&&
			(atomic_independent_variable=boost::dynamic_pointer_cast<
#if defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
			Function_variable_identity
#else // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
			Function_variable_wrapper
#endif // defined (BEFORE_FUNCTION_VARIABLE_WRAPPER)
			,Function_variable>(
			atomic_independent_variables.front()))&&
			(*atomic_variable== *atomic_independent_variable))
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

bool Function_identity::set_value(Function_variable_handle atomic_variable,
	Function_variable_handle atomic_value)
//******************************************************************************
// LAST MODIFIED : 25 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	Function_handle function;

	result=false;
	if (variable_private&&(function=(variable_private->function)()))
	{
		result=(function->set_value)(atomic_variable,atomic_value);
	}

	return (result);
}

Function_handle Function_identity::get_value(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 25 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle function,result;

	result=0;
	if (variable_private&&(function=(variable_private->function)()))
	{
		result=(function->get_value)(atomic_variable);
	}

	return (result);
}

Function_identity::Function_identity(
	const Function_identity& function_identity):Function(),
	variable_private(function_identity.variable_private)
//******************************************************************************
// LAST MODIFIED : 25 June 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
}

Function_identity& Function_identity::operator=(
	const Function_identity& function_identity)
//******************************************************************************
// LAST MODIFIED : 25 June 2004
//
// DESCRIPTION :
// Assignment operator.
//==============================================================================
{
	variable_private=function_identity.variable_private;

	return (*this);
}
