//******************************************************************************
// FILE : function_composition.cpp
//
// LAST MODIFIED : 24 August 2004
//
// DESCRIPTION :
//==============================================================================

#include <sstream>

#include "computed_variable/function_composition.hpp"
#include "computed_variable/function_derivative_matrix.hpp"
#include "computed_variable/function_matrix.hpp"
#include "computed_variable/function_variable.hpp"
#include "computed_variable/function_variable_intersection.hpp"
#include "computed_variable/function_variable_union.hpp"
#include "computed_variable/function_variable_value_scalar.hpp"
#include "computed_variable/function_variable_wrapper.hpp"

// module classes
// ==============

// class Function_variable_composition
// -----------------------------------

// forward declaration so that can use _handle
class Function_variable_composition;
typedef boost::intrusive_ptr<Function_variable_composition>
	Function_variable_composition_handle;

class Function_variable_composition : public Function_variable_wrapper
//******************************************************************************
// LAST MODIFIED : 18 August 2004
//
// DESCRIPTION :
// If <working_variable> is NULL then this wraps the <output_variable> of
// <function_composition>, otherwise <working_variable> is an atomic variable
// in the <output_variable>.
//==============================================================================
{
	friend class Function_composition;
	friend class Function_variable_iterator_representation_atomic_composition;
	public:
		// constructor
		Function_variable_composition(
			const Function_composition_handle& function_composition):
			Function_variable_wrapper(function_composition,0){};
		// constructor.  A zero <atomic_output_variable> indicates the whole output
		//   variable
		Function_variable_composition(
			const Function_composition_handle& function_composition,
			Function_variable_handle& atomic_output_variable):
			Function_variable_wrapper(function_composition,0)
		{
			if (function_composition)
			{
				if (atomic_output_variable&&(function_composition->output_private))
				{
					Function_variable_iterator atomic_variable_iterator,
						end_atomic_variable_iterator;

					atomic_variable_iterator=
						function_composition->output_private->begin_atomic();
					end_atomic_variable_iterator=
						function_composition->output_private->end_atomic();
					while ((atomic_variable_iterator!=end_atomic_variable_iterator)&&
						!equivalent(*atomic_variable_iterator,atomic_output_variable))
					{
						atomic_variable_iterator++;
					}
					if (atomic_variable_iterator!=end_atomic_variable_iterator)
					{
						this->working_variable=atomic_output_variable;
					}
				}
			}
		};
		// destructor
		~Function_variable_composition(){};
	// inherited
	public:
		Function_variable_handle clone() const
		{
			return (Function_variable_composition_handle(
				new Function_variable_composition(*this)));
		};
		Function_handle evaluate()
		{
			Function_composition_handle function_composition=
				boost::dynamic_pointer_cast<Function_composition,Function>(function());
			Function_handle result(0);
			Function_variable_handle wrapped_variable=get_wrapped();

			if (function_composition&&wrapped_variable)
			{
				Function_handle input_current(0);

				if ((function_composition->input_private)&&
					(function_composition->value_private))
				{
					input_current=(function_composition->input_private)->get_value();
					(function_composition->input_private)->set_value(
						function_composition->value_private->evaluate());
				}
				result=wrapped_variable->evaluate();
				if (input_current)
				{
					(function_composition->input_private)->rset_value(input_current);
				}
			}

			return (result);
		}
		Function_handle evaluate_derivative(
			std::list<Function_variable_handle>& independent_variables)
		{
			Function_composition_handle function_composition=
				boost::dynamic_pointer_cast<Function_composition,Function>(function());
			Function_handle result(0);
			Function_variable_handle wrapped_variable=get_wrapped();

			if (function_composition&&wrapped_variable)
			{
				bool valid_independent_variables;
				Function_handle input_current(0);
				Function_size_type i;
				std::list<Function_variable_handle>::iterator
					independent_variables_iterator;

				// check for differentiating wrt input/source variables
				valid_independent_variables=true;
				i=independent_variables.size();
				independent_variables_iterator=independent_variables.begin();
				while (valid_independent_variables&&(i>0))
				{
					Function_variable_intersection_handle input_intersection(
						new Function_variable_intersection(
						function_composition->input_private,
						*independent_variables_iterator));

					if (input_intersection&&
						(0==input_intersection->number_differentiable()))
					{
						Function_variable_intersection_handle value_intersection(
							new Function_variable_intersection(
							function_composition->value_private,
							*independent_variables_iterator));

						if ((0==value_intersection)||
							(0!=value_intersection->number_differentiable()))
						{
							valid_independent_variables=false;
						}
					}
					else
					{
						valid_independent_variables=false;
					}
					independent_variables_iterator++;
					i--;
				}
				if (valid_independent_variables)
				{
					Function_variable_union_handle independent_variables_union(
						new Function_variable_union(independent_variables));
					Function_variable_union_handle g_dependent_variable(
						new Function_variable_union(function_composition->value_private,
						independent_variables_union));
					Function_derivative_matrix_handle derivative_g(
						new Function_derivative_matrix(g_dependent_variable,
						independent_variables));

					if (derivative_g)
					{
						Function_derivative_matrix_handle derivative_f(0);
						Function_handle input_current(0);
						Function_variable_union_handle f_independent_variable(
							new Function_variable_union(function_composition->input_private,
							independent_variables_union));
						std::list<Function_variable_handle> f_independent_variables(
							independent_variables.size(),f_independent_variable);

						if ((function_composition->input_private)&&
							(function_composition->value_private))
						{
							input_current=(function_composition->input_private)->get_value();
							(function_composition->input_private)->set_value(
								function_composition->value_private->evaluate());
						}
						derivative_f=new Function_derivative_matrix(wrapped_variable,
							f_independent_variables);
						if (derivative_f)
						{
							Function_derivative_matrix_handle derivative_matrix=
								Function_derivative_matrix_compose(
								Function_variable_handle(this),derivative_f,derivative_g);

							if (derivative_matrix)
							{
								result=derivative_matrix->matrix(independent_variables);
							}
						}
						if (input_current)
						{
							(function_composition->input_private)->rset_value(input_current);
						}
					}
				}
			}

			return (result);
		}
		string_handle get_string_representation()
		{
			Function_composition_handle function_composition=
				boost::dynamic_pointer_cast<Function_composition,Function>(function());
			Function_variable_handle wrapped_variable=get_wrapped();
			string_handle return_string(0);

			if (return_string=new std::string)
			{
				std::ostringstream out;

				if (wrapped_variable)
				{
					out << *(wrapped_variable->get_string_representation());
				}
				out << "(";
				if (function_composition)
				{
					if (function_composition->input_private)
					{
						out << *(function_composition->input_private->
							get_string_representation());
					}
					out << "=";
					if (function_composition->value_private)
					{
						out << *(function_composition->value_private->
							get_string_representation());
					}
				}
				out << ")";
				*return_string=out.str();
			}

			return (return_string);
		};
		Function_variable_handle get_wrapped() const
		{
			Function_composition_handle function_composition=
				boost::dynamic_pointer_cast<Function_composition,Function>(function());
			Function_variable_handle result(0);

			if (this)
			{
				if (!(result=working_variable))
				{
					if (function_composition)
					{
						result=function_composition->output_private;
					}
				}
			}

			return (result);
		};
	private:
		// copy constructor
		Function_variable_composition(
			const Function_variable_composition& variable_composition):
			Function_variable_wrapper(variable_composition){};
		// assignment
		Function_variable_composition& operator=(
			const Function_variable_composition&);
};


// global classes
// ==============

// class Function_composition
// --------------------------

Function_composition::Function_composition(
	const Function_variable_handle output,const Function_variable_handle input,
	const Function_variable_handle value):Function(),input_private(input),
	output_private(output),value_private(value)
//******************************************************************************
// LAST MODIFIED : 11 June 2004
//
// DESCRIPTION :
//==============================================================================
{
}

Function_composition::~Function_composition()
//******************************************************************************
// LAST MODIFIED : 11 June 2004
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	// do nothing
}

string_handle Function_composition::get_string_representation()
//******************************************************************************
// LAST MODIFIED : 14 June 2004
//
// DESCRIPTION :
//???DB.  Overload << instead of get_string_representation?
//==============================================================================
{
	string_handle return_string;

	if (return_string=new std::string)
	{
		std::ostringstream out;

		out << *(output_private->get_string_representation()) << "(" <<
			*(input_private->get_string_representation()) << "=" <<
			*(value_private->get_string_representation()) << ")";
		*return_string=out.str();
	}

	return (return_string);
}

Function_variable_handle Function_composition::input()
//******************************************************************************
// LAST MODIFIED : 11 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(0));
}

Function_variable_handle Function_composition::output()
//******************************************************************************
// LAST MODIFIED : 11 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_variable_handle(new Function_variable_composition(
		Function_composition_handle(this))));
}

bool Function_composition::operator==(const Function& function) const
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
			const Function_composition& function_composition=
				dynamic_cast<const Function_composition&>(function);

			result=(equivalent(input_private,(function_composition.input_private))&&
				equivalent(output_private,(function_composition.output_private))&&
				equivalent(value_private,(function_composition.value_private)));
		}
		catch (std::bad_cast)
		{
			// do nothing
		}
	}

	return (result);
}

Function_handle Function_composition::evaluate(Function_variable_handle)
//******************************************************************************
// LAST MODIFIED : 11 June 2004
//
// DESCRIPTION :
//==============================================================================
{
	// should not come here - handled by Function_variable_composition over-riding
	//   Function_variable::evaluate
	Assert(false,std::logic_error(
		"Function_composition::evaluate.  Should not come here"));
	
	return (0);
}

bool Function_composition::evaluate_derivative(Scalar& derivative,
	Function_variable_handle atomic_dependent_variable,
	std::list<Function_variable_handle>& atomic_independent_variables)
//******************************************************************************
// LAST MODIFIED : 18 August 2004
//
// DESCRIPTION :
// Can come here via Function_derivative_matrix::Function_derivative_matrix
// and the Function_variable_inverse::evaluate_derivative or
// Function_variable_dependent::evaluate_derivative or
// Function_variable::evaluate_derivative.
//==============================================================================
{
	bool result;
	boost::intrusive_ptr< Function_matrix<Scalar> > derivative_value;
	Function_variable_composition_handle atomic_variable_composition;

	result=false;
	if ((atomic_variable_composition=boost::dynamic_pointer_cast<
		Function_variable_composition,Function_variable>(
		atomic_dependent_variable))&&equivalent(Function_handle(this),
		atomic_variable_composition->function()))
	{
		if ((derivative_value=boost::dynamic_pointer_cast<
			Function_matrix<Scalar>,Function>(atomic_variable_composition->
			evaluate_derivative(atomic_independent_variables)))&&
			(1==derivative_value->number_of_rows())&&
			(1==derivative_value->number_of_columns()))
		{
			derivative=(*derivative_value)(1,1);
			result=true;
		}
	}

	return (result);
}

bool Function_composition::set_value(
	const Function_variable_handle atomic_variable,
	const Function_variable_handle atomic_value)
//******************************************************************************
// LAST MODIFIED : 18 August 2004
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	Function_handle function;
	Function_variable_composition_handle atomic_variable_composition;
	Function_variable_handle local_atomic_variable;

	result=false;
	if ((atomic_variable_composition=boost::dynamic_pointer_cast<
		Function_variable_composition,Function_variable>(atomic_variable))&&
		equivalent(Function_handle(this),atomic_variable_composition->function()))
	{
		local_atomic_variable=atomic_variable_composition->get_wrapped();
	}
	else
	{
		local_atomic_variable=atomic_variable;
	}
	if (output_private&&(function=(output_private->function)())&&
		(function->set_value)(local_atomic_variable,atomic_value))
	{
		result=true;
	}
	if (value_private&&(function=(value_private->function)())&&
		(function->set_value)(local_atomic_variable,atomic_value))
	{
		result=true;
	}

	return (result);
}

Function_handle Function_composition::get_value(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 18 August 2004
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle function,result;
	Function_variable_composition_handle atomic_variable_composition;
	Function_variable_handle local_atomic_variable;

	result=0;
	if ((atomic_variable_composition=boost::dynamic_pointer_cast<
		Function_variable_composition,Function_variable>(atomic_variable))&&
		equivalent(Function_handle(this),atomic_variable_composition->function()))
	{
		local_atomic_variable=atomic_variable_composition->get_wrapped();
	}
	else
	{
		local_atomic_variable=atomic_variable;
	}
	if (output_private&&(function=(output_private->function)()))
	{
		result=(function->get_value)(local_atomic_variable);
	}

	return (result);
}

Function_composition::Function_composition(
	const Function_composition& function_composition):Function(),
	input_private(function_composition.input_private),
	output_private(function_composition.output_private),
	value_private(function_composition.value_private)
//******************************************************************************
// LAST MODIFIED : 11 June 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
}

Function_composition& Function_composition::operator=(
	const Function_composition& function_composition)
//******************************************************************************
// LAST MODIFIED : 11 June 2004
//
// DESCRIPTION :
// Assignment operator.
//==============================================================================
{
	input_private=function_composition.input_private;
	output_private=function_composition.output_private;
	value_private=function_composition.value_private;

	return (*this);
}
