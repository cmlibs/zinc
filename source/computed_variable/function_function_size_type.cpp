//******************************************************************************
// FILE : function_function_size_type.cpp
//
// LAST MODIFIED : 30 June 2004
//
// DESCRIPTION :
//==============================================================================

#include <sstream>

#include "computed_variable/function_function_size_type.hpp"
#include "computed_variable/function_variable.hpp"
#include "computed_variable/function_variable_value_function_size_type.hpp"

// module classes
// ==============

// forward declaration so that can use _handle
class Function_variable_function_size_type;
typedef boost::intrusive_ptr<Function_variable_function_size_type>
	Function_variable_function_size_type_handle;


// class Function_variable_iterator_representation_atomic_function_size_type
// -------------------------------------------------------------------------

class Function_variable_iterator_representation_atomic_function_size_type :
	public Function_variable_iterator_representation
//******************************************************************************
// LAST MODIFIED : 25 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	public:
		// constructor
		Function_variable_iterator_representation_atomic_function_size_type(
			const bool begin,Function_variable_function_size_type_handle variable);
		// a "virtual" constructor
		Function_variable_iterator_representation *clone();
	protected:
		// destructor
		~Function_variable_iterator_representation_atomic_function_size_type();
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
		Function_variable_iterator_representation_atomic_function_size_type(const
			Function_variable_iterator_representation_atomic_function_size_type&);
	private:
		Function_variable_function_size_type_handle atomic_variable,variable;
};

static bool
	Function_variable_function_size_type_set_function_size_type_function(
	Function_size_type& value,const Function_variable_handle variable);


// class Function_variable_function_size_type
// ------------------------------------------

class Function_variable_function_size_type : public Function_variable
//******************************************************************************
// LAST MODIFIED : 30 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	friend class Function_function_size_type;
	friend class
		Function_variable_iterator_representation_atomic_function_size_type;
	friend bool
		Function_variable_function_size_type_set_function_size_type_function(
		Function_size_type& value,const Function_variable_handle variable);
	public:
		// constructor
		Function_variable_function_size_type(
			const Function_function_size_type_handle function_function_size_type):
			Function_variable(function_function_size_type)
		{
			if (function_function_size_type)
			{
				value_private=Function_variable_value_handle(
					new Function_variable_value_function_size_type(
					Function_variable_function_size_type_set_function_size_type_function
					));
			}
		};
		~Function_variable_function_size_type(){}
	// inherited
	public:
		Function_variable_handle clone() const
		{
			return (Function_variable_handle(new Function_variable_function_size_type(
				*this)));
		};
		string_handle get_string_representation()
		{
			string_handle return_string(0);

			if (return_string=new std::string)
			{
				std::ostringstream out;

				out << "function_size_type";
				*return_string=out.str();
			}

			return (return_string);
		};
		Function_variable_iterator begin_atomic() const
		{
			return (Function_variable_iterator(
				new Function_variable_iterator_representation_atomic_function_size_type(
				true,Function_variable_function_size_type_handle(
				const_cast<Function_variable_function_size_type*>(this)))));
		};
		Function_variable_iterator end_atomic() const
		{
			return (Function_variable_iterator(
				new Function_variable_iterator_representation_atomic_function_size_type(
				false,Function_variable_function_size_type_handle(
				const_cast<Function_variable_function_size_type*>(this)))));
		};
		std::reverse_iterator<Function_variable_iterator> rbegin_atomic() const
		{
			return (std::reverse_iterator<Function_variable_iterator>(
				new Function_variable_iterator_representation_atomic_function_size_type(
				false,Function_variable_function_size_type_handle(
				const_cast<Function_variable_function_size_type*>(this)))));
		};
		std::reverse_iterator<Function_variable_iterator> rend_atomic() const
		{
			return (std::reverse_iterator<Function_variable_iterator>(
				new Function_variable_iterator_representation_atomic_function_size_type(
				true,Function_variable_function_size_type_handle(
				const_cast<Function_variable_function_size_type*>(this)))));
		};
		Function_size_type number_differentiable()
		{
			return (0);
		};
		bool equality_atomic(const Function_variable_handle& variable) const
		{
			bool result;
			Function_variable_function_size_type_handle variable_function_size_type;

			result=false;
			if (variable_function_size_type=boost::dynamic_pointer_cast<
				Function_variable_function_size_type,Function_variable>(variable))
			{
				if (function()==variable_function_size_type->function())
				{
					result=true;
				}
			}

			return (result);
		};
	private:
		// copy constructor
		Function_variable_function_size_type(
			const Function_variable_function_size_type& variable):
			Function_variable(variable){};
		// assignment
		Function_variable_function_size_type& operator=(
			const Function_variable_function_size_type&);
};

static bool
	Function_variable_function_size_type_set_function_size_type_function(
	Function_size_type& value,const Function_variable_handle variable)
{
	bool result;
	Function_function_size_type_handle function_function_size_type;
	Function_variable_function_size_type_handle function_size_type_variable;

	result=false;
	if ((function_size_type_variable=boost::dynamic_pointer_cast<
		Function_variable_function_size_type,Function_variable>(variable))&&
		(function_function_size_type=boost::dynamic_pointer_cast<
		Function_function_size_type,Function>(function_size_type_variable->
		function())))
	{
		value=function_function_size_type->value();
		result=true;
	}

	return (result);
}


// class Function_variable_iterator_representation_atomic_function_size_type
// -------------------------------------------------------------------------

Function_variable_iterator_representation_atomic_function_size_type::
	Function_variable_iterator_representation_atomic_function_size_type(
	const bool begin,Function_variable_function_size_type_handle variable):
	atomic_variable(0),variable(variable)
//******************************************************************************
// LAST MODIFIED : 30 June 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	if (begin&&variable&&(variable->function()))
	{
		//???DB.  OK not to clone because not changing
		atomic_variable=variable;
	}
}

Function_variable_iterator_representation
	*Function_variable_iterator_representation_atomic_function_size_type::clone()
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
		result=
			new Function_variable_iterator_representation_atomic_function_size_type(
			*this);
	}

	return (result);
}

Function_variable_iterator_representation_atomic_function_size_type::
	~Function_variable_iterator_representation_atomic_function_size_type()
//******************************************************************************
// LAST MODIFIED : 25 June 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	// do nothing
}

void Function_variable_iterator_representation_atomic_function_size_type::
	increment()
//******************************************************************************
// LAST MODIFIED : 25 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	if (atomic_variable)
	{
		atomic_variable=0;
	}
}

void Function_variable_iterator_representation_atomic_function_size_type::
	decrement()
//******************************************************************************
// LAST MODIFIED : 30 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	if (atomic_variable)
	{
		atomic_variable=0;
	}
	else
	{
		if (variable&&(variable->function()))
		{
			//???DB.  OK not to clone because not changing
			atomic_variable=variable;
		}
	}
}

bool Function_variable_iterator_representation_atomic_function_size_type::
	equality(const Function_variable_iterator_representation * representation)
//******************************************************************************
// LAST MODIFIED : 25 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	const Function_variable_iterator_representation_atomic_function_size_type
		*representation_function_size_type=dynamic_cast<const
		Function_variable_iterator_representation_atomic_function_size_type *>(
		representation);

	result=false;
	if (representation_function_size_type)
	{
		if (((0==atomic_variable)&&
			(0==representation_function_size_type->atomic_variable))||
			(atomic_variable&&(representation_function_size_type->atomic_variable)&&
			(*atomic_variable== *(representation_function_size_type->atomic_variable))
			))
		{
			result=true;
		}
	}

	return (result);
}

Function_variable_handle
	Function_variable_iterator_representation_atomic_function_size_type::
	dereference() const
//******************************************************************************
// LAST MODIFIED : 25 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (atomic_variable);
}

Function_variable_iterator_representation_atomic_function_size_type::
	Function_variable_iterator_representation_atomic_function_size_type(const
	Function_variable_iterator_representation_atomic_function_size_type&
	representation):Function_variable_iterator_representation(),
	atomic_variable(0),variable(representation.variable)
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
			Function_variable_function_size_type,Function_variable>(
			(representation.atomic_variable)->clone());
	}
}


// global classes
// ==============

// class Function_function_size_type
// ---------------------------------

Function_function_size_type::Function_function_size_type(
	Function_size_type& value):Function(),value_private(value){}
//******************************************************************************
// LAST MODIFIED : 25 June 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================

Function_function_size_type::~Function_function_size_type()
//******************************************************************************
// LAST MODIFIED : 25 June 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	// do nothing
}

string_handle Function_function_size_type::get_string_representation()
//******************************************************************************
// LAST MODIFIED : 25 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	string_handle return_string;
	std::ostringstream out;

	if (return_string=new std::string)
	{
		out << value_private;
		*return_string=out.str();
	}

	return (return_string);
}

Function_variable_handle Function_function_size_type::input()
//******************************************************************************
// LAST MODIFIED : 25 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_function_size_type(
		Function_function_size_type_handle(this))));
}

Function_variable_handle Function_function_size_type::output()
//******************************************************************************
// LAST MODIFIED : 25 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_function_size_type(
		Function_function_size_type_handle(this))));
}

Function_size_type Function_function_size_type::value() const
//******************************************************************************
// LAST MODIFIED : 25 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (value_private);
}

Function_handle Function_function_size_type::evaluate(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 25 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (get_value(atomic_variable));
}

bool Function_function_size_type::evaluate_derivative(Scalar&,
	Function_variable_handle,std::list<Function_variable_handle>&)
//******************************************************************************
// LAST MODIFIED : 25 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (false);
}

bool Function_function_size_type::set_value(
	Function_variable_handle atomic_variable,
	Function_variable_handle atomic_value)
//******************************************************************************
// LAST MODIFIED : 30 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	Function_variable_function_size_type_handle
		atomic_function_size_type_variable;
	Function_variable_value_function_size_type_handle value_function_size_type;

	result=false;
	if ((atomic_function_size_type_variable=boost::dynamic_pointer_cast<
		Function_variable_function_size_type,Function_variable>(atomic_variable))&&
		(Function_handle(this)==atomic_function_size_type_variable->function())&&
		atomic_value&&(atomic_value->value())&&
		(std::string("Function_size_type")==(atomic_value->value())->type())&&
		(value_function_size_type=boost::dynamic_pointer_cast<
		Function_variable_value_function_size_type,Function_variable_value>(
		atomic_value->value())))
	{
		result=value_function_size_type->set(value_private,atomic_value);
	}

	return (result);
}

Function_handle Function_function_size_type::get_value(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 25 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle result(0);

	if (atomic_variable&&(Function_handle(this)==(atomic_variable->function)()))
	{
		result=Function_handle(new Function_function_size_type(*this));
	}

	return (result);
}

Function_function_size_type::Function_function_size_type(
	const Function_function_size_type& function_function_size_type):
	Function(function_function_size_type),
	value_private(function_function_size_type.value_private){}
//******************************************************************************
// LAST MODIFIED : 25 June 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================

Function_function_size_type& Function_function_size_type::operator=(
	const Function_function_size_type& function_function_size_type)
//******************************************************************************
// LAST MODIFIED : 25 June 2004
//
// DESCRIPTION :
// Assignment operator.
//==============================================================================
{
	this->value_private=function_function_size_type.value_private;

	return (*this);
}
