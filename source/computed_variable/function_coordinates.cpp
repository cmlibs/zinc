//******************************************************************************
// FILE : function_coordinates.cpp
//
// LAST MODIFIED : 4 June 2004
//
// DESCRIPTION :
//???DB.
// Function_variable_iterator_representation_atomic_rectangular_cartesian
// is a copy of
// Function_variable_iterator_representation_atomic_finite_element
// and
// Function_variable_rectangular_cartesian
// is a copy of
// Function_variable_finite_element
// How to abstract?
//==============================================================================

#include "computed_variable/function_coordinates.hpp"
#include "computed_variable/function_matrix.hpp"
#include "computed_variable/function_variable.hpp"
#include "computed_variable/function_variable_value_scalar.hpp"

// module classes
// ==============

// forward declaration so that can use _handle
class Function_variable_rectangular_cartesian;
typedef boost::intrusive_ptr<Function_variable_rectangular_cartesian>
	Function_variable_rectangular_cartesian_handle;

// class Function_variable_iterator_representation_atomic_rectangular_cartesian
// ----------------------------------------------------------------------------

class Function_variable_iterator_representation_atomic_rectangular_cartesian:
	public Function_variable_iterator_representation
//******************************************************************************
// LAST MODIFIED : 3 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	public:
		// constructor
		Function_variable_iterator_representation_atomic_rectangular_cartesian(
			const bool begin,Function_variable_rectangular_cartesian_handle variable);
		// a "virtual" constructor
		Function_variable_iterator_representation *clone();
	protected:
		// destructor
		~Function_variable_iterator_representation_atomic_rectangular_cartesian();
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
		Function_variable_iterator_representation_atomic_rectangular_cartesian(const
			Function_variable_iterator_representation_atomic_rectangular_cartesian&);
	private:
		Function_variable_rectangular_cartesian_handle atomic_variable,variable;
};


// class Function_variable_rectangular_cartesian
// ---------------------------------------------

class Function_variable_rectangular_cartesian : public Function_variable
//******************************************************************************
// LAST MODIFIED : 4 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	friend class Function_prolate_spheroidal_to_rectangular_cartesian;
	friend class
		Function_variable_iterator_representation_atomic_rectangular_cartesian;
	friend bool
		is_atomic(Function_variable_rectangular_cartesian_handle variable);
	public:
		// constructors.  A zero component_number indicates all components
		Function_variable_rectangular_cartesian(
			const Function_prolate_spheroidal_to_rectangular_cartesian_handle&
			function_prolate_spheroidal_to_rectangular_cartesian,
			Function_size_type component_number=0):
			function_prolate_spheroidal_to_rectangular_cartesian(
			function_prolate_spheroidal_to_rectangular_cartesian),
			component_number(component_number){};
		Function_variable_rectangular_cartesian(
			const Function_prolate_spheroidal_to_rectangular_cartesian_handle&
			function_prolate_spheroidal_to_rectangular_cartesian,
			const std::string component_name):
			function_prolate_spheroidal_to_rectangular_cartesian(
			function_prolate_spheroidal_to_rectangular_cartesian),
			component_number(0)
		{
			if (function_prolate_spheroidal_to_rectangular_cartesian)
			{
				Function_size_type local_component_number;

				local_component_number=0;
				if ((std::string("x")==component_name)||
					(std::string("X")==component_name))
				{
					local_component_number=1;
				}
				else if ((std::string("y")==component_name)||
					(std::string("Y")==component_name))
				{
					local_component_number=2;
				}
				else if ((std::string("z")==component_name)||
					(std::string("Z")==component_name))
				{
					local_component_number=3;
				}
				if ((0<local_component_number)&&(local_component_number<=
					function_prolate_spheroidal_to_rectangular_cartesian->
					number_of_components()))
				{
					component_number=local_component_number;
				}
			}
		};
		// destructor
		~Function_variable_rectangular_cartesian(){};
	// inherited
	public:
		Function_variable_handle clone() const
		{
			return (Function_variable_rectangular_cartesian_handle(
				new Function_variable_rectangular_cartesian(*this)));
		};
		Function_handle function()
		{
			return (function_prolate_spheroidal_to_rectangular_cartesian);
		}
		string_handle get_string_representation()
		{
			string_handle return_string(0);

			if (return_string=new std::string)
			{
				std::ostringstream out;

				out << "rectangular cartesian";
				switch (component_number)
				{
					case 0:
					{
						// do nothing
					} break;
					case 1:
					{
						out << ".x";
					} break;
					case 2:
					{
						out << ".y";
					} break;
					case 3:
					{
						out << ".z";
					} break;
					default:
					{
						out << "[" << component_number << "]";
					} break;
				}
				*return_string=out.str();
			}

			return (return_string);
		};
		virtual Function_variable_iterator begin_atomic() const
		{
			return (Function_variable_iterator(new
				Function_variable_iterator_representation_atomic_rectangular_cartesian(
				true,Function_variable_rectangular_cartesian_handle(
				const_cast<Function_variable_rectangular_cartesian*>(this)))));
		};
		virtual Function_variable_iterator end_atomic() const
		{
			return (Function_variable_iterator(new
				Function_variable_iterator_representation_atomic_rectangular_cartesian(
				false,Function_variable_rectangular_cartesian_handle(
				const_cast<Function_variable_rectangular_cartesian*>(this)))));
		};
		std::reverse_iterator<Function_variable_iterator> rbegin_atomic() const
		{
			return (std::reverse_iterator<Function_variable_iterator>(new
				Function_variable_iterator_representation_atomic_rectangular_cartesian(
				false,Function_variable_rectangular_cartesian_handle(
				const_cast<Function_variable_rectangular_cartesian*>(this)))));
		};
		std::reverse_iterator<Function_variable_iterator> rend_atomic() const
		{
			return (std::reverse_iterator<Function_variable_iterator>(new
				Function_variable_iterator_representation_atomic_rectangular_cartesian(
				true,Function_variable_rectangular_cartesian_handle(
				const_cast<Function_variable_rectangular_cartesian*>(this)))));
		};
		Function_size_type number_differentiable()
		{
			Function_size_type result;

			result=0;
			if (this&&function_prolate_spheroidal_to_rectangular_cartesian)
			{
				Function_size_type number_of_components=
					function_prolate_spheroidal_to_rectangular_cartesian->
					number_of_components();
				if (component_number<=number_of_components)
				{
					if (0==component_number)
					{
						result=number_of_components;
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
			Function_variable_rectangular_cartesian_handle
				variable_rectangular_cartesian;

			result=false;
			if (variable_rectangular_cartesian=boost::dynamic_pointer_cast<
				Function_variable_rectangular_cartesian,Function_variable>(variable))
			{
				if ((variable_rectangular_cartesian->
					function_prolate_spheroidal_to_rectangular_cartesian==
					function_prolate_spheroidal_to_rectangular_cartesian)&&
					(variable_rectangular_cartesian->component_number==component_number))
				{
					result=true;
				}
			}

			return (result);
		};
	private:
		// copy constructor
		Function_variable_rectangular_cartesian(const
			Function_variable_rectangular_cartesian& variable_rectangular_cartesian):
			Function_variable(),function_prolate_spheroidal_to_rectangular_cartesian(
			variable_rectangular_cartesian.
			function_prolate_spheroidal_to_rectangular_cartesian),
			component_number(variable_rectangular_cartesian.component_number){};
		// assignment
		Function_variable_rectangular_cartesian& operator=(
			const Function_variable_rectangular_cartesian&);
	private:
		Function_prolate_spheroidal_to_rectangular_cartesian_handle
			function_prolate_spheroidal_to_rectangular_cartesian;
		Function_size_type component_number;
};

bool is_atomic(Function_variable_rectangular_cartesian_handle variable)
{
	bool result;

	result=false;
	if (variable&&(0!=variable->component_number))
	{
		result=true;
	}

	return (result);
}


// class Function_variable_iterator_representation_atomic_rectangular_cartesian
// ---------------------------------------------------------------------

Function_variable_iterator_representation_atomic_rectangular_cartesian::
	Function_variable_iterator_representation_atomic_rectangular_cartesian(
	const bool begin,Function_variable_rectangular_cartesian_handle variable):
	atomic_variable(0),variable(variable)
//******************************************************************************
// LAST MODIFIED : 3 June 2004
//
// DESCRIPTION :
// Constructor.  If <begin> then the constructed iterator points to the first
// atomic variable, otherwise it points to one past the last atomic variable.
//==============================================================================
{
	if (begin&&variable&&
		(variable->function_prolate_spheroidal_to_rectangular_cartesian))
	{
		if (atomic_variable=boost::dynamic_pointer_cast<
			Function_variable_rectangular_cartesian,Function_variable>(
			variable->clone()))
		{
			if (0==variable->component_number)
			{
				if (0<(variable->function_prolate_spheroidal_to_rectangular_cartesian->
					number_of_components)())
				{
					atomic_variable->component_number=1;
				}
				else
				{
					// end
					atomic_variable=0;
				}
			}
			else
			{
				if (variable->component_number>(variable->
					function_prolate_spheroidal_to_rectangular_cartesian->
					number_of_components)())
				{
					// end
					atomic_variable=0;
				}
			}
			// component is an output and cannot be set so leave value_private zero
		}
	}
}

Function_variable_iterator_representation
	*Function_variable_iterator_representation_atomic_rectangular_cartesian::
	clone()
//******************************************************************************
// LAST MODIFIED : 3 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_variable_iterator_representation *result;

	result=0;
	if (this)
	{
		result=new
			Function_variable_iterator_representation_atomic_rectangular_cartesian(
			*this);
	}

	return (result);
}

Function_variable_iterator_representation_atomic_rectangular_cartesian::
	~Function_variable_iterator_representation_atomic_rectangular_cartesian()
//******************************************************************************
// LAST MODIFIED : 3 June 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	// do nothing
}

void Function_variable_iterator_representation_atomic_rectangular_cartesian::
	increment()
//******************************************************************************
// LAST MODIFIED : 3 June 2004
//
// DESCRIPTION :
// Increments the iterator to the next atomic variable.  NULL <atomic_variable>
// is the end iterator.
//==============================================================================
{
	if (atomic_variable)
	{
		if (0==variable->component_number)
		{
			if (atomic_variable->component_number<
				(variable->function_prolate_spheroidal_to_rectangular_cartesian->
				number_of_components)())
			{
				(atomic_variable->component_number)++;
			}
			else
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

void Function_variable_iterator_representation_atomic_rectangular_cartesian::
	decrement()
//******************************************************************************
// LAST MODIFIED : 3 June 2004
//
// DESCRIPTION :
// Decrements the iterator to the next atomic variable.  NULL <atomic_variable>
// is the end iterator.
//==============================================================================
{
	if (atomic_variable)
	{
		if (0==variable->component_number)
		{
			if (1<atomic_variable->component_number)
			{
				(atomic_variable->component_number)--;
			}
			else
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
	else
	{
		if (variable&&
			(variable->function_prolate_spheroidal_to_rectangular_cartesian))
		{
			if (atomic_variable=boost::dynamic_pointer_cast<
				Function_variable_rectangular_cartesian,Function_variable>(
				variable->clone()))
			{
				if (0==variable->component_number)
				{
					atomic_variable->component_number=
						(variable->function_prolate_spheroidal_to_rectangular_cartesian->
						number_of_components)();
					if (0==atomic_variable->component_number)
					{
						// end
						atomic_variable=0;
					}
				}
				else
				{
					if (variable->component_number>
						(variable->function_prolate_spheroidal_to_rectangular_cartesian->
						number_of_components)())
					{
						// end
						atomic_variable=0;
					}
				}
				// component is an output and cannot be set so leave value_private zero
			}
		}
	}
}

bool Function_variable_iterator_representation_atomic_rectangular_cartesian::
	equality(const Function_variable_iterator_representation * representation)
//******************************************************************************
// LAST MODIFIED : 3 June 2004
//
// DESCRIPTION :
// Tests <*this> and <*representation> for equality.
//==============================================================================
{
	bool result;
	const Function_variable_iterator_representation_atomic_rectangular_cartesian
		*representation_rectangular_cartesian=dynamic_cast<const
		Function_variable_iterator_representation_atomic_rectangular_cartesian *>(
		representation);

	result=false;
	if (representation_rectangular_cartesian)
	{
		if (((0==atomic_variable)&&
			(0==representation_rectangular_cartesian->atomic_variable))||
			(atomic_variable&&
			(representation_rectangular_cartesian->atomic_variable)&&
			(*atomic_variable==
			*(representation_rectangular_cartesian->atomic_variable))))
		{
			result=true;
		}
	}

	return (result);
}

Function_variable_handle
	Function_variable_iterator_representation_atomic_rectangular_cartesian::
	dereference() const
//******************************************************************************
// LAST MODIFIED : 3 June 2004
//
// DESCRIPTION :
// Returns the atomic variable for the iterator.
//==============================================================================
{
	return (atomic_variable);
}

Function_variable_iterator_representation_atomic_rectangular_cartesian::
	Function_variable_iterator_representation_atomic_rectangular_cartesian(const
	Function_variable_iterator_representation_atomic_rectangular_cartesian&
	representation):Function_variable_iterator_representation(),
	atomic_variable(0),variable(representation.variable)
//******************************************************************************
// LAST MODIFIED : 3 June 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
	if (representation.atomic_variable)
	{
		atomic_variable=boost::dynamic_pointer_cast<
			Function_variable_rectangular_cartesian,Function_variable>(
			(representation.atomic_variable)->clone());
	}
}

// forward declaration so that can use _handle
class Function_variable_prolate_spheroidal;
typedef boost::intrusive_ptr<Function_variable_prolate_spheroidal>
	Function_variable_prolate_spheroidal_handle;


// class Function_variable_iterator_representation_atomic_prolate_spheroidal
// -------------------------------------------------------------------------

class Function_variable_iterator_representation_atomic_prolate_spheroidal :
	public Function_variable_iterator_representation
//******************************************************************************
// LAST MODIFIED : 4 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	public:
		// constructor
		Function_variable_iterator_representation_atomic_prolate_spheroidal(
			const bool begin,Function_variable_prolate_spheroidal_handle variable);
		// a "virtual" constructor
		Function_variable_iterator_representation *clone();
	protected:
		// destructor
		~Function_variable_iterator_representation_atomic_prolate_spheroidal();
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
		Function_variable_iterator_representation_atomic_prolate_spheroidal(const
			Function_variable_iterator_representation_atomic_prolate_spheroidal&);
	private:
		Function_variable_prolate_spheroidal_handle atomic_variable,variable;
};

static bool Function_variable_prolate_spheroidal_set_scalar_function(
	Scalar& value,const Function_variable_handle variable);


// class Function_variable_prolate_spheroidal
// ------------------------------------------

class Function_variable_prolate_spheroidal : public Function_variable
//******************************************************************************
// LAST MODIFIED : 3 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	friend class Function_prolate_spheroidal_to_rectangular_cartesian;
	friend
		class Function_variable_iterator_representation_atomic_prolate_spheroidal;
	friend bool is_atomic(Function_variable_prolate_spheroidal_handle variable);
	friend bool Function_variable_prolate_spheroidal_set_scalar_function(
		Scalar& value,const Function_variable_handle variable);
	public:
		// constructor
		Function_variable_prolate_spheroidal(
			const Function_prolate_spheroidal_to_rectangular_cartesian_handle&
			function_prolate_spheroidal_to_rectangular_cartesian,bool lambda=true,
			bool mu=true,bool theta=true,bool focus=false):focus(focus),
			lambda(lambda),mu(mu),theta(theta),
			function_prolate_spheroidal_to_rectangular_cartesian(
			function_prolate_spheroidal_to_rectangular_cartesian)
		{
			if (function_prolate_spheroidal_to_rectangular_cartesian&&
				((lambda&&!mu&&!theta&&!focus)||(!lambda&&mu&&!theta&&!focus)||
				(!lambda&&!mu&&theta&&!focus)||(!lambda&&!mu&&!theta&&focus)))
			{
				value_private=Function_variable_value_handle(
					new Function_variable_value_scalar(
					Function_variable_prolate_spheroidal_set_scalar_function));
			}
		};
		~Function_variable_prolate_spheroidal(){};
	// inherited
	public:
		Function_variable_handle clone() const
		{
			return (Function_variable_handle(new Function_variable_prolate_spheroidal(
				*this)));
		};
		Function_handle function()
		{
			return (function_prolate_spheroidal_to_rectangular_cartesian);
		};
		string_handle get_string_representation()
		{
			string_handle return_string(0);

			if (return_string=new std::string)
			{
				bool comma;
				std::ostringstream out;

				out << "(";
				comma=false;
				if (lambda)
				{
					out << "lambda";
					comma=true;
				}
				if (comma)
				{
					out << ",";
				}
				if (mu)
				{
					out << "mu";
					comma=true;
				}
				if (comma)
				{
					out << ",";
				}
				if (theta)
				{
					out << "theta";
					comma=true;
				}
				if (comma)
				{
					out << ",";
				}
				if (focus)
				{
					out << "focus";
					comma=true;
				}
				out << ")";
				*return_string=out.str();
			}

			return (return_string);
		};
		Function_variable_iterator begin_atomic() const
		{
			return (Function_variable_iterator(
				new Function_variable_iterator_representation_atomic_prolate_spheroidal(
				true,Function_variable_prolate_spheroidal_handle(const_cast<
				Function_variable_prolate_spheroidal*>(this)))));
		};
		Function_variable_iterator end_atomic() const
		{
			return (Function_variable_iterator(
				new Function_variable_iterator_representation_atomic_prolate_spheroidal(
				false,Function_variable_prolate_spheroidal_handle(const_cast<
				Function_variable_prolate_spheroidal*>(this)))));
		};
		std::reverse_iterator<Function_variable_iterator> rbegin_atomic() const
		{
			return (std::reverse_iterator<Function_variable_iterator>(
				new Function_variable_iterator_representation_atomic_prolate_spheroidal(
				false,Function_variable_prolate_spheroidal_handle(const_cast<
				Function_variable_prolate_spheroidal*>(this)))));
		};
		std::reverse_iterator<Function_variable_iterator> rend_atomic() const
		{
			return (std::reverse_iterator<Function_variable_iterator>(
				new Function_variable_iterator_representation_atomic_prolate_spheroidal(
				true,Function_variable_prolate_spheroidal_handle(const_cast<
				Function_variable_prolate_spheroidal*>(this)))));
		};
		Function_size_type number_differentiable()
		{
			Function_size_type result;

			result=0;
			if (function_prolate_spheroidal_to_rectangular_cartesian)
			{
				if (lambda)
				{
					result++;
				}
				if (mu)
				{
					result++;
				}
				if (theta)
				{
					result++;
				}
				if (focus)
				{
					result++;
				}
			}

			return (result);
		};
		bool equality_atomic(const Function_variable_handle& variable) const
		{
			bool result;
			Function_variable_prolate_spheroidal_handle variable_prolate_spheroidal;

			result=false;
			if (variable_prolate_spheroidal=boost::dynamic_pointer_cast<
				Function_variable_prolate_spheroidal,Function_variable>(variable))
			{
				result=((function_prolate_spheroidal_to_rectangular_cartesian==
					variable_prolate_spheroidal->
					function_prolate_spheroidal_to_rectangular_cartesian)&&
					(lambda==variable_prolate_spheroidal->lambda)&&
					(mu==variable_prolate_spheroidal->mu)&&
					(theta==variable_prolate_spheroidal->theta)&&
					(focus==variable_prolate_spheroidal->focus));
			}

			return (result);
		};
		// copy constructor
		Function_variable_prolate_spheroidal(
			const Function_variable_prolate_spheroidal& variable):
			Function_variable(variable),focus(variable.focus),lambda(variable.lambda),
			mu(variable.mu),theta(variable.theta),
			function_prolate_spheroidal_to_rectangular_cartesian(variable.
			function_prolate_spheroidal_to_rectangular_cartesian){};
		// assignment
		Function_variable_prolate_spheroidal& operator=(
			const Function_variable_prolate_spheroidal&);
	private:
		bool focus,lambda,mu,theta;
		Function_prolate_spheroidal_to_rectangular_cartesian_handle
			function_prolate_spheroidal_to_rectangular_cartesian;
};

bool is_atomic(Function_variable_prolate_spheroidal_handle variable)
{
	bool result;

	result=false;
	if (variable&&(
		((variable->lambda)&&!(variable->mu)&&!(variable->theta)&&
		!(variable->focus))||
		(!(variable->lambda)&&(variable->mu)&&!(variable->theta)&&
		!(variable->focus))||
		(!(variable->lambda)&&!(variable->mu)&&(variable->theta)&&
		!(variable->focus))||
		(!(variable->lambda)&&!(variable->mu)&&!(variable->theta)&&
		(variable->focus))))
	{
		result=true;
	}

	return (result);
}

static bool Function_variable_prolate_spheroidal_set_scalar_function(
	Scalar& value,const Function_variable_handle variable)
{
	bool result;
	Function_variable_prolate_spheroidal_handle variable_prolate_spheroidal;

	result=false;
	if ((variable_prolate_spheroidal=boost::dynamic_pointer_cast<
		Function_variable_prolate_spheroidal,Function_variable>(variable))&&
		is_atomic(variable_prolate_spheroidal)&&
		(variable_prolate_spheroidal->
		function_prolate_spheroidal_to_rectangular_cartesian))
	{
		if (variable_prolate_spheroidal->lambda)
		{
			value=(variable_prolate_spheroidal->
				function_prolate_spheroidal_to_rectangular_cartesian->lambda_value)();
			result=true;
		}
		else if (variable_prolate_spheroidal->mu)
		{
			value=(variable_prolate_spheroidal->
				function_prolate_spheroidal_to_rectangular_cartesian->mu_value)();
			result=true;
		}
		else if (variable_prolate_spheroidal->theta)
		{
			value=(variable_prolate_spheroidal->
				function_prolate_spheroidal_to_rectangular_cartesian->theta_value)();
			result=true;
		}
		else if (variable_prolate_spheroidal->focus)
		{
			value=(variable_prolate_spheroidal->
				function_prolate_spheroidal_to_rectangular_cartesian->focus_value)();
			result=true;
		}
	}

	return (result);
}


// class Function_variable_iterator_representation_atomic_prolate_spheroidal
// -------------------------------------------------------------------------

Function_variable_iterator_representation_atomic_prolate_spheroidal::
	Function_variable_iterator_representation_atomic_prolate_spheroidal(
	const bool begin,Function_variable_prolate_spheroidal_handle variable):
	atomic_variable(0),variable(variable)
//******************************************************************************
// LAST MODIFIED : 3 June 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	if (begin&&variable)
	{
		if (atomic_variable=boost::dynamic_pointer_cast<
			Function_variable_prolate_spheroidal,Function_variable>(
			variable->clone()))
		{
			if (atomic_variable->lambda)
			{
				atomic_variable->mu=false;
				atomic_variable->theta=false;
				atomic_variable->focus=false;
			}
			else if (atomic_variable->mu)
			{
				atomic_variable->theta=false;
				atomic_variable->focus=false;
			}
			else if (atomic_variable->theta)
			{
				atomic_variable->focus=false;
			}
			atomic_variable->value_private=Function_variable_value_handle(
				new Function_variable_value_scalar(
				Function_variable_prolate_spheroidal_set_scalar_function
				));
		}
	}
}

Function_variable_iterator_representation
	*Function_variable_iterator_representation_atomic_prolate_spheroidal::clone()
//******************************************************************************
// LAST MODIFIED : 3 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_variable_iterator_representation *result;

	result=0;
	if (this)
	{
		result=new
			Function_variable_iterator_representation_atomic_prolate_spheroidal(
			*this);
	}

	return (result);
}

Function_variable_iterator_representation_atomic_prolate_spheroidal::
	~Function_variable_iterator_representation_atomic_prolate_spheroidal()
//******************************************************************************
// LAST MODIFIED : 3 June 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	// do nothing
}

void Function_variable_iterator_representation_atomic_prolate_spheroidal::
	increment()
//******************************************************************************
// LAST MODIFIED : 3 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	if (variable&&
		(variable->function_prolate_spheroidal_to_rectangular_cartesian)&&
		(atomic_variable))
	{
		if (atomic_variable->lambda)
		{
			atomic_variable->lambda=false;
			if (variable->mu)
			{
				atomic_variable->mu=true;
			}
			else if (variable->theta)
			{
				atomic_variable->theta=true;
			}
			else if (variable->focus)
			{
				atomic_variable->focus=true;
			}
			else
			{
				atomic_variable=0;
			}
		}
		else if (atomic_variable->mu)
		{
			atomic_variable->mu=false;
			if (variable->theta)
			{
				atomic_variable->theta=true;
			}
			else if (variable->focus)
			{
				atomic_variable->focus=true;
			}
			else
			{
				atomic_variable=0;
			}
		}
		else if (atomic_variable->theta)
		{
			atomic_variable->theta=false;
			if (variable->focus)
			{
				atomic_variable->focus=true;
			}
			else
			{
				atomic_variable=0;
			}
		}
		else
		{
			atomic_variable=0;
		}
	}
}

void Function_variable_iterator_representation_atomic_prolate_spheroidal::
	decrement()
//******************************************************************************
// LAST MODIFIED : 3 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	if (variable&&
		(variable->function_prolate_spheroidal_to_rectangular_cartesian))
	{
		if (atomic_variable)
		{
			if (atomic_variable->focus)
			{
				atomic_variable->focus=false;
				if (variable->theta)
				{
					atomic_variable->theta=true;
				}
				else if (variable->mu)
				{
					atomic_variable->mu=true;
				}
				else if (variable->lambda)
				{
					atomic_variable->lambda=true;
				}
				else
				{
					atomic_variable=0;
				}
			}
			else if (atomic_variable->theta)
			{
				atomic_variable->theta=false;
				if (variable->mu)
				{
					atomic_variable->mu=true;
				}
				else if (variable->lambda)
				{
					atomic_variable->lambda=true;
				}
				else
				{
					atomic_variable=0;
				}
			}
			else if (atomic_variable->mu)
			{
				atomic_variable->mu=false;
				if (variable->lambda)
				{
					atomic_variable->lambda=true;
				}
				else
				{
					atomic_variable=0;
				}
			}
			else
			{
				atomic_variable=0;
			}
		}
		else
		{
			if (atomic_variable=boost::dynamic_pointer_cast<
				Function_variable_prolate_spheroidal,Function_variable>(
				variable->clone()))
			{
				if (atomic_variable->lambda)
				{
					atomic_variable->mu=false;
					atomic_variable->theta=false;
					atomic_variable->focus=false;
				}
				else if (atomic_variable->mu)
				{
					atomic_variable->theta=false;
					atomic_variable->focus=false;
				}
				else if (atomic_variable->theta)
				{
					atomic_variable->focus=false;
				}
				atomic_variable->value_private=Function_variable_value_handle(
					new Function_variable_value_scalar(
					Function_variable_prolate_spheroidal_set_scalar_function));
			}
		}
	}
}

bool Function_variable_iterator_representation_atomic_prolate_spheroidal::
	equality(const Function_variable_iterator_representation * representation)
//******************************************************************************
// LAST MODIFIED : 3 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	const Function_variable_iterator_representation_atomic_prolate_spheroidal
		*representation_prolate_spheroidal=dynamic_cast<
		const Function_variable_iterator_representation_atomic_prolate_spheroidal*>(
		representation);

	result=false;
	if (representation_prolate_spheroidal)
	{
		if (((0==atomic_variable)&&
			(0==representation_prolate_spheroidal->atomic_variable))||
			(atomic_variable&&(representation_prolate_spheroidal->atomic_variable)&&
			(*atomic_variable==
			*(representation_prolate_spheroidal->atomic_variable))))
		{
			result=true;
		}
	}

	return (result);
}

Function_variable_handle
	Function_variable_iterator_representation_atomic_prolate_spheroidal::
	dereference() const
//******************************************************************************
// LAST MODIFIED : 3 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (atomic_variable);
}

Function_variable_iterator_representation_atomic_prolate_spheroidal::
	Function_variable_iterator_representation_atomic_prolate_spheroidal(const
	Function_variable_iterator_representation_atomic_prolate_spheroidal&
	representation):Function_variable_iterator_representation(),
	atomic_variable(0),variable(representation.variable)
//******************************************************************************
// LAST MODIFIED : 3 June 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
	if (representation.atomic_variable)
	{
		atomic_variable=boost::dynamic_pointer_cast<
			Function_variable_prolate_spheroidal,Function_variable>(
			(representation.atomic_variable)->clone());
	}
}


// global classes
// ==============

// class Function_prolate_spheroidal_to_rectangular_cartesian
// ----------------------------------------------------------

Function_prolate_spheroidal_to_rectangular_cartesian::
	Function_prolate_spheroidal_to_rectangular_cartesian(const Scalar lambda,
	const Scalar mu,const Scalar theta,const Scalar focus):Function(),
	number_of_components_private(3),focus_private(focus),lambda_private(lambda),
	mu_private(mu),theta_private(theta){}
//******************************************************************************
// LAST MODIFIED : 4 June 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================

Function_prolate_spheroidal_to_rectangular_cartesian::
	~Function_prolate_spheroidal_to_rectangular_cartesian()
//******************************************************************************
// LAST MODIFIED : 3 June 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	// do nothing
}

string_handle Function_prolate_spheroidal_to_rectangular_cartesian::
	get_string_representation()
//******************************************************************************
// LAST MODIFIED : 3 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	string_handle return_string;
	std::ostringstream out;

	if (return_string=new std::string)
	{
		out << "prolate spheroidal to rectangular cartesian";
		*return_string=out.str();
	}

	return (return_string);
}

Function_variable_handle Function_prolate_spheroidal_to_rectangular_cartesian::
	input()
//******************************************************************************
// LAST MODIFIED : 3 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_prolate_spheroidal(
		Function_prolate_spheroidal_to_rectangular_cartesian_handle(this),true,true,
		true,true)));
}

Function_variable_handle Function_prolate_spheroidal_to_rectangular_cartesian::
	output()
//******************************************************************************
// LAST MODIFIED : 4 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_rectangular_cartesian(
		Function_prolate_spheroidal_to_rectangular_cartesian_handle(this))));
}

Function_variable_handle Function_prolate_spheroidal_to_rectangular_cartesian::
	component(std::string component_name)
//******************************************************************************
// LAST MODIFIED : 4 June 2004
//
// DESCRIPTION :
// Returns the component output.
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_rectangular_cartesian(
		Function_prolate_spheroidal_to_rectangular_cartesian_handle(this),
		component_name)));
}

Function_variable_handle Function_prolate_spheroidal_to_rectangular_cartesian::
	component(Function_size_type component_number)
//******************************************************************************
// LAST MODIFIED : 4 June 2004
//
// DESCRIPTION :
// Returns the component output.
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_rectangular_cartesian(
		Function_prolate_spheroidal_to_rectangular_cartesian_handle(this),
		component_number)));
}

Function_variable_handle Function_prolate_spheroidal_to_rectangular_cartesian::
	focus()
//******************************************************************************
// LAST MODIFIED : 3 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_prolate_spheroidal(
		Function_prolate_spheroidal_to_rectangular_cartesian_handle(this),false,
		false,false,true)));
}

Function_variable_handle Function_prolate_spheroidal_to_rectangular_cartesian::
	lambda()
//******************************************************************************
// LAST MODIFIED : 3 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_prolate_spheroidal(
		Function_prolate_spheroidal_to_rectangular_cartesian_handle(this),true,
		false,false,false)));
}

Function_variable_handle Function_prolate_spheroidal_to_rectangular_cartesian::
	mu()
//******************************************************************************
// LAST MODIFIED : 3 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_prolate_spheroidal(
		Function_prolate_spheroidal_to_rectangular_cartesian_handle(this),false,
		true,false,false)));
}

Function_variable_handle Function_prolate_spheroidal_to_rectangular_cartesian::
	prolate()
//******************************************************************************
// LAST MODIFIED : 3 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_prolate_spheroidal(
		Function_prolate_spheroidal_to_rectangular_cartesian_handle(this),true,true,
		true,false)));
}

Function_variable_handle Function_prolate_spheroidal_to_rectangular_cartesian::
	theta()
//******************************************************************************
// LAST MODIFIED : 3 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_prolate_spheroidal(
		Function_prolate_spheroidal_to_rectangular_cartesian_handle(this),false,
		false,true,false)));
}

Scalar Function_prolate_spheroidal_to_rectangular_cartesian::focus_value()
//******************************************************************************
// LAST MODIFIED : 4 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (focus_private);
}

Scalar Function_prolate_spheroidal_to_rectangular_cartesian::lambda_value()
//******************************************************************************
// LAST MODIFIED : 4 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (lambda_private);
}

Scalar Function_prolate_spheroidal_to_rectangular_cartesian::mu_value()
//******************************************************************************
// LAST MODIFIED : 4 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (mu_private);
}

Scalar Function_prolate_spheroidal_to_rectangular_cartesian::theta_value()
//******************************************************************************
// LAST MODIFIED : 4 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (theta_private);
}

Function_size_type Function_prolate_spheroidal_to_rectangular_cartesian::
	number_of_components()
//******************************************************************************
// LAST MODIFIED : 4 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (number_of_components_private);
}

Function_handle Function_prolate_spheroidal_to_rectangular_cartesian::evaluate(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 4 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle result(0);
	Function_variable_prolate_spheroidal_handle
		atomic_variable_prolate_spheroidal;
	Function_variable_rectangular_cartesian_handle
		atomic_variable_rectangular_cartesian;

	if ((atomic_variable_prolate_spheroidal=boost::dynamic_pointer_cast<
		Function_variable_prolate_spheroidal,Function_variable>(atomic_variable))&&
		(Function_handle(this)==atomic_variable_prolate_spheroidal->
		function_prolate_spheroidal_to_rectangular_cartesian))
	{
		Matrix result_matrix(1,1);

		if (Function_variable_prolate_spheroidal_set_scalar_function(
			result_matrix(0,0),atomic_variable))
		{
			result=Function_handle(new Function_matrix(result_matrix));
		}
	}
	else if ((atomic_variable_rectangular_cartesian=boost::dynamic_pointer_cast<
		Function_variable_rectangular_cartesian,Function_variable>(
		atomic_variable))&&(Function_handle(this)==
		atomic_variable_rectangular_cartesian->
		function_prolate_spheroidal_to_rectangular_cartesian)&&
		(0<atomic_variable_rectangular_cartesian->component_number)&&
		(atomic_variable_rectangular_cartesian->component_number<=
		number_of_components()))
	{
		Matrix result_matrix(1,1);

		result_matrix(0,0)=0;
		switch (atomic_variable_rectangular_cartesian->component_number)
		{
			case 1:
			{
				result_matrix(0,0)=(Scalar)((double)focus_private*
					cosh((double)lambda_private)*cos((double)mu_private));
			} break;
			case 2:
			{
				result_matrix(0,0)=(Scalar)((double)focus_private*
					sinh((double)lambda_private)*sin((double)mu_private)*
					cos((double)theta_private));
			} break;
			case 3:
			{
				result_matrix(0,0)=(Scalar)((double)focus_private*
					sinh((double)lambda_private)*sin((double)mu_private)*
					sin((double)theta_private));
			} break;
		}
		result=Function_handle(new Function_matrix(result_matrix));
	}

	return (result);
}

bool Function_prolate_spheroidal_to_rectangular_cartesian::evaluate_derivative(
	Scalar& derivative,Function_variable_handle atomic_variable,
	std::list<Function_variable_handle>& atomic_independent_variables)
//******************************************************************************
// LAST MODIFIED : 4 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	Function_variable_prolate_spheroidal_handle
		atomic_variable_prolate_spheroidal;
	Function_variable_rectangular_cartesian_handle
		atomic_variable_rectangular_cartesian;

	result=false;
	if ((atomic_variable_prolate_spheroidal=boost::dynamic_pointer_cast<
		Function_variable_prolate_spheroidal,Function_variable>(atomic_variable))&&
		(this==atomic_variable_prolate_spheroidal->
		function_prolate_spheroidal_to_rectangular_cartesian)&&
		is_atomic(atomic_variable_prolate_spheroidal)&&
		(1==atomic_variable_prolate_spheroidal->number_differentiable()))
	{
		Function_variable_prolate_spheroidal_handle atomic_independent_variable;

		result=true;
		if ((1==atomic_independent_variables.size())&&
			(atomic_independent_variable=boost::dynamic_pointer_cast<
			Function_variable_prolate_spheroidal,Function_variable>(
			atomic_independent_variables.front()))&&
			(*atomic_variable_prolate_spheroidal== *atomic_independent_variable))
		{
			derivative=1;
		}
		else
		{
			derivative=0;
		}
	}
	else if ((atomic_variable_rectangular_cartesian=boost::dynamic_pointer_cast<
		Function_variable_rectangular_cartesian,Function_variable>(
		atomic_variable))&&(this==atomic_variable_rectangular_cartesian->
		function_prolate_spheroidal_to_rectangular_cartesian)&&
		is_atomic(atomic_variable_rectangular_cartesian)&&
		(1==atomic_variable_rectangular_cartesian->number_differentiable()))
	{
		bool zero_derivative;
		Function_variable_prolate_spheroidal_handle variable_prolate_spheroidal;
		Function_size_type focus_derivative_order,i,lambda_derivative_order,
			mu_derivative_order,theta_derivative_order;
		std::list<Function_variable_handle>::iterator independent_variable_iterator;

		// check for zero derivative and calculate derivative orders
		zero_derivative=false;
		lambda_derivative_order=0;
		mu_derivative_order=0;
		theta_derivative_order=0;
		focus_derivative_order=0;
		independent_variable_iterator=atomic_independent_variables.begin();
		i=atomic_independent_variables.size();
		while (!zero_derivative&&(i>0))
		{
			if ((variable_prolate_spheroidal=
				boost::dynamic_pointer_cast<Function_variable_prolate_spheroidal,
				Function_variable>(*independent_variable_iterator))&&
				(variable_prolate_spheroidal->
				function_prolate_spheroidal_to_rectangular_cartesian==
				Function_handle(this)))
			{
				if (variable_prolate_spheroidal->lambda)
				{
					lambda_derivative_order++;
				}
				else if (variable_prolate_spheroidal->mu)
				{
					mu_derivative_order++;
				}
				else if (variable_prolate_spheroidal->theta)
				{
					theta_derivative_order++;
				}
				else if (variable_prolate_spheroidal->focus)
				{
					focus_derivative_order++;
					if (1<focus_derivative_order)
					{
						zero_derivative=true;
					}
				}
			}
			else
			{
				zero_derivative=true;
			}
			independent_variable_iterator++;
			i--;
		}
		if (zero_derivative)
		{
			result=true;
			derivative=0;
		}
		else
		{
			Scalar focus_derivative;

			// focus_derivative_order<=1 from construction above
			if (0==focus_derivative_order)
			{
				focus_derivative=focus_private;
			}
			else
			{
				focus_derivative=1;
			}
			// assign derivatives
			switch (atomic_variable_rectangular_cartesian->component_number)
			{
				case 1:
				{
					switch (lambda_derivative_order)
					{
						case 0:
						{
							switch (mu_derivative_order)
							{
								case 0:
								{
									switch (theta_derivative_order)
									{
										case 0:
										{
											result=true;
											derivative=focus_derivative*
												cosh(lambda_private)*cos(mu_private);
										} break;
										case 1:
										{
											result=true;
											derivative=0;
										} break;
										case 2:
										{
											result=true;
											derivative=0;
										} break;
									}
								} break;
								case 1:
								{
									switch (theta_derivative_order)
									{
										case 0:
										{
											result=true;
											derivative= -focus_derivative*
												cosh(lambda_private)*sin(mu_private);
										} break;
										case 1:
										{
											result=true;
											derivative=0;
										} break;
									}
								} break;
								case 2:
								{
									switch (theta_derivative_order)
									{
										case 0:
										{
											result=true;
											derivative= -focus_derivative*
												cosh(lambda_private)*cos(mu_private);
										} break;
									}
								} break;
							}
						} break;
						case 1:
						{
							switch (mu_derivative_order)
							{
								case 0:
								{
									switch (theta_derivative_order)
									{
										case 0:
										{
											result=true;
											derivative=focus_derivative*
												sinh(lambda_private)*cos(mu_private);
										} break;
										case 1:
										{
											result=true;
											derivative=0;
										} break;
									}
								} break;
								case 1:
								{
									switch (theta_derivative_order)
									{
										case 0:
										{
											result=true;
											derivative= -focus_derivative*
												sinh(lambda_private)*sin(mu_private);
										} break;
									}
								} break;
							}
						} break;
						case 2:
						{
							// mu order
							switch (mu_derivative_order)
							{
								case 0:
								{
									switch (theta_derivative_order)
									{
										case 0:
										{
											result=true;
											derivative=focus_derivative*
												cosh(lambda_private)*cos(mu_private);
										} break;
									}
								} break;
							}
						} break;
					}
				} break;
				case 2:
				{
					switch (lambda_derivative_order)
					{
						case 0:
						{
							switch (mu_derivative_order)
							{
								case 0:
								{
									switch (theta_derivative_order)
									{
										case 0:
										{
											result=true;
											derivative=focus_derivative*
												sinh(lambda_private)*sin(mu_private)*cos(theta_private);
										} break;
										case 1:
										{
											result=true;
											derivative= -focus_derivative*
												sinh(lambda_private)*sin(mu_private)*sin(theta_private);
										} break;
										case 2:
										{
											result=true;
											derivative= -focus_derivative*
												sinh(lambda_private)*sin(mu_private)*cos(theta_private);
										} break;
									}
								} break;
								case 1:
								{
									switch (theta_derivative_order)
									{
										case 0:
										{
											result=true;
											derivative=focus_derivative*sinh(lambda_private)*
												cos(mu_private)*cos(theta_private);
										} break;
										case 1:
										{
											result=true;
											derivative= -focus_derivative*sinh(lambda_private)*
												cos(mu_private)*sin(theta_private);
										} break;
									}
								} break;
								case 2:
								{
									switch (theta_derivative_order)
									{
										case 0:
										{
											result=true;
											derivative= -focus_derivative*sinh(lambda_private)*
												sin(mu_private)*cos(theta_private);
										} break;
									}
								} break;
							}
						} break;
						case 1:
						{
							switch (mu_derivative_order)
							{
								case 0:
								{
									switch (theta_derivative_order)
									{
										case 0:
										{
											result=true;
											derivative=focus_derivative*
												cosh(lambda_private)*sin(mu_private)*cos(theta_private);
										} break;
										case 1:
										{
											result=true;
											derivative= -focus_derivative*
												cosh(lambda_private)*sin(mu_private)*sin(theta_private);
										} break;
									}
								} break;
								case 1:
								{
									switch (theta_derivative_order)
									{
										case 0:
										{
											result=true;
											derivative=focus_derivative*
												cosh(lambda_private)*cos(mu_private)*cos(theta_private);
										} break;
									}
								} break;
							}
						} break;
						case 2:
						{
							// mu order
							switch (mu_derivative_order)
							{
								case 0:
								{
									switch (theta_derivative_order)
									{
										case 0:
										{
											result=true;
											derivative=focus_derivative*
												sinh(lambda_private)*sin(mu_private)*cos(theta_private);
										} break;
									}
								} break;
							}
						} break;
					}
				} break;
				case 3:
				{
					switch (lambda_derivative_order)
					{
						case 0:
						{
							switch (mu_derivative_order)
							{
								case 0:
								{
									switch (theta_derivative_order)
									{
										case 0:
										{
											result=true;
											derivative=focus_derivative*
												sinh(lambda_private)*sin(mu_private)*sin(theta_private);
										} break;
										case 1:
										{
											result=true;
											derivative=focus_derivative*
												sinh(lambda_private)*sin(mu_private)*cos(theta_private);
										} break;
										case 2:
										{
											result=true;
											derivative= -focus_derivative*
												sinh(lambda_private)*sin(mu_private)*sin(theta_private);
										} break;
									}
								} break;
								case 1:
								{
									switch (theta_derivative_order)
									{
										case 0:
										{
											result=true;
											derivative=focus_derivative*
												sinh(lambda_private)*cos(mu_private)*sin(theta_private);
										} break;
										case 1:
										{
											result=true;
											derivative=focus_derivative*
												sinh(lambda_private)*cos(mu_private)*cos(theta_private);
										} break;
									}
								} break;
								case 2:
								{
									switch (theta_derivative_order)
									{
										case 0:
										{
											result=true;
											derivative= -focus_derivative*
												sinh(lambda_private)*sin(mu_private)*sin(theta_private);
										} break;
									}
								} break;
							}
						} break;
						case 1:
						{
							switch (mu_derivative_order)
							{
								case 0:
								{
									switch (theta_derivative_order)
									{
										case 0:
										{
											result=true;
											derivative=focus_derivative*
												cosh(lambda_private)*sin(mu_private)*sin(theta_private);
										} break;
										case 1:
										{
											result=true;
											derivative=focus_derivative*
												cosh(lambda_private)*sin(mu_private)*cos(theta_private);
										} break;
									}
								} break;
								case 1:
								{
									switch (theta_derivative_order)
									{
										case 0:
										{
											result=true;
											derivative=focus_derivative*
												cosh(lambda_private)*cos(mu_private)*sin(theta_private);
										} break;
									}
								} break;
							}
						} break;
						case 2:
						{
							// mu order
							switch (mu_derivative_order)
							{
								case 0:
								{
									switch (theta_derivative_order)
									{
										case 0:
										{
											result=true;
											derivative=focus_derivative*
												sinh(lambda_private)*sin(mu_private)*sin(theta_private);
										} break;
									}
								} break;
							}
						} break;
					}
				} break;
				default:
				{
					result=true;
					derivative=0;
				} break;
			}
		}
	}

	return (result);
}

bool Function_prolate_spheroidal_to_rectangular_cartesian::set_value(
	Function_variable_handle atomic_variable,
	Function_variable_handle atomic_value)
//******************************************************************************
// LAST MODIFIED : 4 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	Function_variable_prolate_spheroidal_handle
		atomic_prolate_spheroidal_variable;
	Function_variable_value_scalar_handle value_scalar;

	result=false;
	if ((atomic_prolate_spheroidal_variable=
		boost::dynamic_pointer_cast<Function_variable_prolate_spheroidal,
		Function_variable>(atomic_variable))&&
		(this==atomic_prolate_spheroidal_variable->
		function_prolate_spheroidal_to_rectangular_cartesian)&&
		is_atomic(atomic_prolate_spheroidal_variable)&&
		atomic_value&&(atomic_value->value())&&
		(std::string("Scalar")==(atomic_value->value())->type())&&
		(value_scalar=boost::dynamic_pointer_cast<Function_variable_value_scalar,
		Function_variable_value>(atomic_value->value())))
	{
		if (atomic_prolate_spheroidal_variable->lambda)
		{
			result=value_scalar->set(lambda_private,atomic_value);
		}
		else if (atomic_prolate_spheroidal_variable->mu)
		{
			result=value_scalar->set(mu_private,atomic_value);
		}
		else if (atomic_prolate_spheroidal_variable->theta)
		{
			result=value_scalar->set(theta_private,atomic_value);
		}
		else if (atomic_prolate_spheroidal_variable->focus)
		{
			result=value_scalar->set(focus_private,atomic_value);
		}
	}

	return (result);
}

Function_prolate_spheroidal_to_rectangular_cartesian::
	Function_prolate_spheroidal_to_rectangular_cartesian(
	const Function_prolate_spheroidal_to_rectangular_cartesian& function):
	Function(),
	number_of_components_private(function.number_of_components_private),
	focus_private(function.focus_private),lambda_private(function.lambda_private),
	mu_private(function.mu_private),theta_private(function.theta_private){}
//******************************************************************************
// LAST MODIFIED : 4 June 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================

Function_prolate_spheroidal_to_rectangular_cartesian&
	Function_prolate_spheroidal_to_rectangular_cartesian::operator=(
	const Function_prolate_spheroidal_to_rectangular_cartesian& function)
//******************************************************************************
// LAST MODIFIED : 4 June 2004
//
// DESCRIPTION :
// Assignment operator.
//==============================================================================
{
	lambda_private=function.lambda_private;
	mu_private=function.mu_private;
	theta_private=function.theta_private;
	focus_private=function.focus_private;

	return (*this);
}
