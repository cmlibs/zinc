//******************************************************************************
// FILE : variable_composition.cpp
//
// LAST MODIFIED : 9 February 2004
//
// DESCRIPTION :
//==============================================================================

#include "computed_variable/variable_base.hpp"

#include <new>
#include <sstream>
#include <string>
#include <stdio.h>

#include "computed_variable/variable_composite.hpp"
#include "computed_variable/variable_composition.hpp"
#include "computed_variable/variable_derivative_matrix.hpp"
#include "computed_variable/variable_identity.hpp"
#if defined (USE_VARIABLE_INPUT)
#include "computed_variable/variable_input_composite.hpp"
#else // defined (USE_VARIABLE_INPUT)
#include "computed_variable/variable_io_specifier_composite.hpp"
#endif // defined (USE_VARIABLE_INPUT)
#if defined (GENERALIZE_COMPOSITE_INPUT)
#include "computed_variable/variable_input_composite_union.hpp"
#endif // defined (GENERALIZE_COMPOSITE_INPUT)

// global classes
// ==============

// class Variable_composition
// --------------------------

Variable_composition::Variable_composition(
	const Variable_handle dependent_variable,
	const std::list<Variable_input_value_handle> input_source_list):Variable(),
	dependent_variable(dependent_variable),input_source_list(input_source_list)
//******************************************************************************
// LAST MODIFIED : 17 November 2003
//
// DESCRIPTION :
//==============================================================================
{
}

Variable_composition::Variable_composition(
	const Variable_composition& variable_composition):Variable(),
	dependent_variable(variable_composition.dependent_variable),
	input_source_list(variable_composition.input_source_list)
//******************************************************************************
// LAST MODIFIED : 17 November 2003
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
}

Variable_composition& Variable_composition::operator=(
	const Variable_composition& variable_composition)
//******************************************************************************
// LAST MODIFIED : 17 November 2003
//
// DESCRIPTION :
// Assignment operator.
//==============================================================================
{
	dependent_variable=variable_composition.dependent_variable;
	input_source_list=variable_composition.input_source_list;

	return (*this);
}

Variable_composition::~Variable_composition()
//******************************************************************************
// LAST MODIFIED : 17 November 2003
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	// do nothing
}

#if defined (USE_ITERATORS)
//???DB.  To be done
//???DB.  What is the analog of input iterators for variables?  Components?
#else // defined (USE_ITERATORS)
Variable_size_type Variable_composition::size() const
//******************************************************************************
// LAST MODIFIED : 17 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	return (dependent_variable->size());
}

Vector *Variable_composition::scalars()
//******************************************************************************
// LAST MODIFIED : 18 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	return (evaluate_local()->scalars());
}
#endif // defined (USE_ITERATORS)

class Variable_composition_evaluate_input_source_functor
//******************************************************************************
// LAST MODIFIED : 2 February 2004
//
// DESCRIPTION :
// Functor for evaluating the source variables to make a values list.
//==============================================================================
{
	public:
		Variable_composition_evaluate_input_source_functor(
			std::list<Variable_input_value_handle>& values,
			std::list<Variable_input_value_handle>& source_values):values(values),
			source_values(source_values) {};
		~Variable_composition_evaluate_input_source_functor() {};
		int operator() (Variable_input_value_handle& input_source)
		{
#if defined (USE_VARIABLE_INPUT)
			Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
				input=input_source->input();
			Variable_handle value=(input_source->value())->evaluate(values);

			source_values.push_back(Variable_input_value_handle(
				new Variable_input_value(input,value)));
			
			return (0);
		}
	private:
		std::list<Variable_input_value_handle>& values;
		std::list<Variable_input_value_handle>& source_values;
};

Variable_handle Variable_composition::evaluate(
	std::list<Variable_input_value_handle>& values)
//******************************************************************************
// LAST MODIFIED : 1 January 2004
//
// DESCRIPTION :
// Overloading Variable::evaluate.
//==============================================================================
{
	std::list<Variable_input_value_handle> current_values(0),no_values(0),
		source_values(0);
	Variable_handle result(0);

	// get the specified values
	std::for_each(values.begin(),values.end(),Variable_get_input_values(
		Variable_handle(this),current_values));
	// override the specified values
	std::for_each(values.begin(),values.end(),Variable_set_input_values(
		Variable_handle(this)));
	// evaluate source variables
	std::for_each(input_source_list.begin(),input_source_list.end(),
		Variable_composition_evaluate_input_source_functor(no_values,
		source_values));
	// evaluate
	result=dependent_variable->evaluate(source_values);
	// reset the current values
	std::for_each(current_values.rbegin(),current_values.rend(),
		Variable_set_input_values(Variable_handle(this)));

	return (result);
}

Variable_handle Variable_composition::evaluate_derivative(
	std::list<
#if defined (USE_VARIABLE_INPUT)
	Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
	Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	>& independent_variables,std::list<Variable_input_value_handle>& values)
//******************************************************************************
// LAST MODIFIED : 2 February 2004
//
// DESCRIPTION :
// Overloading Variable::evaluate_derivative.
//
// ???DB.  Check that independent variables don't match inputs that have been
//   re-directed to sources
//==============================================================================
{
	bool valid_independent_variables;
	std::list<Variable_handle> g_variables(0);
	std::list<
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
		> f_inputs(0),f_independent_variables(0);
	std::list<
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
		>::iterator independent_variables_iterator;
	std::list<Variable_input_value_handle> current_values(0),no_values(0),
		source_values(0);
	std::list<Variable_input_value_handle>::iterator input_source_iterator;
	Variable_handle result(0);
	Variable_size_type i;

	// get the specified values
	std::for_each(values.begin(),values.end(),Variable_get_input_values(
		Variable_handle(this),current_values));
	// override the specified values
	std::for_each(values.begin(),values.end(),Variable_set_input_values(
		Variable_handle(this)));
	// evaluate source variables
	std::for_each(input_source_list.begin(),input_source_list.end(),
		Variable_composition_evaluate_input_source_functor(no_values,
		source_values));
	valid_independent_variables=true;
	// set up f inputs and g
	// assume that the inputs in the specified input, source pairs are independent
		//???DB.  Check when create?
	// check for differentiating wrt input/source variables
	input_source_iterator=input_source_list.begin();
	i=input_source_list.size();
	while (valid_independent_variables&&(i>0))
	{
		if (independent_variables.end()==std::find(independent_variables.begin(),
			independent_variables.end(),(*input_source_iterator)->input()))
		{
			g_variables.push_back((*input_source_iterator)->value());
			f_inputs.push_back((*input_source_iterator)->input());
		}
		else
		{
			valid_independent_variables=false;
		}
		input_source_iterator++;
		i--;
	}
	if (valid_independent_variables)
	{
		// check for repeated independent variables
		independent_variables_iterator=independent_variables.begin();
		for (i=independent_variables.size();i>0;i--)
		{
			if (f_inputs.end()==std::find(f_inputs.begin(),f_inputs.end(),
				*independent_variables_iterator))
			{
				g_variables.push_back(Variable_handle(new Variable_identity(
					*independent_variables_iterator)));
				f_inputs.push_back(*independent_variables_iterator);
			}
			independent_variables_iterator++;
		}
#if defined (GENERALIZE_COMPOSITE_INPUT)
		Variable_input_composite_union_handle
#else // defined (GENERALIZE_COMPOSITE_INPUT)
#if defined (USE_VARIABLE_INPUT)
		Variable_input_composite_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_composite_handle
#endif // defined (USE_VARIABLE_INPUT)
#endif // defined (GENERALIZE_COMPOSITE_INPUT)
			f_independent_variable(new
#if defined (GENERALIZE_COMPOSITE_INPUT)
			Variable_input_composite_union
#else // defined (GENERALIZE_COMPOSITE_INPUT)
#if defined (USE_VARIABLE_INPUT)
			Variable_input_composite
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier_composite
#endif // defined (USE_VARIABLE_INPUT)
#endif // defined (GENERALIZE_COMPOSITE_INPUT)
			(f_inputs));
		Variable_composite_handle g(new Variable_composite(g_variables));
		for (i=independent_variables.size();i>0;i--)
		{
			f_independent_variables.push_back(f_independent_variable);
		}
		Variable_derivative_matrix_handle derivative_f=
#if defined (USE_SMART_POINTER)
			boost::dynamic_pointer_cast<Variable_derivative_matrix,Variable>
#else /* defined (USE_SMART_POINTER) */
			dynamic_cast<Variable_derivative_matrix *>
#endif /* defined (USE_SMART_POINTER) */
			((dependent_variable->evaluate_derivative)(f_independent_variables,
			source_values));
		Variable_derivative_matrix_handle derivative_g=
#if defined (USE_SMART_POINTER)
			boost::dynamic_pointer_cast<Variable_derivative_matrix,Variable>
#else /* defined (USE_SMART_POINTER) */
			dynamic_cast<Variable_derivative_matrix *>
#endif /* defined (USE_SMART_POINTER) */
			((g->evaluate_derivative)(independent_variables,no_values));
		if (derivative_f&&derivative_g)
		{
			result=Variable_derivative_matrix_compose(Variable_handle(this),
				derivative_f,derivative_g);
		}
	}
	// reset the current values
	std::for_each(current_values.rbegin(),current_values.rend(),
		Variable_set_input_values(this));

	return (result);
}

Variable_handle Variable_composition::clone() const
//******************************************************************************
// LAST MODIFIED : 8 December 2003
//
// DESCRIPTION :
//==============================================================================
{
	return (Variable_composition_handle(new Variable_composition(*this)));
}

Variable_handle Variable_composition::evaluate_local()
//******************************************************************************
// LAST MODIFIED : 17 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	// should not come here - handled by overloading Variable::evaluate
	Assert(false,std::logic_error(
		"Variable_composition::evaluate_local.  "
		"Should not come here"));
	
	return (0);
}

bool Variable_composition::evaluate_derivative_local(Matrix&,
	std::list<
#if defined (USE_VARIABLE_INPUT)
	Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
	Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	>&)
//******************************************************************************
// LAST MODIFIED : 2 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	// should not come here - handled by overloading Variable::evaluate_derivative
	Assert(false,std::logic_error(
		"Variable_composition::evaluate_derivative_local.  "
		"Should not come here"));

	return (false);
}

Variable_handle Variable_composition::get_input_value_local(
	const
#if defined (USE_VARIABLE_INPUT)
	Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
	Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	& input)
//******************************************************************************
// LAST MODIFIED : 9 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	std::list<Variable_input_value_handle>::iterator
		input_source_iterator=input_source_list.begin();
	Variable_handle result;
	Variable_size_type i=input_source_list.size();

	result=dependent_variable->get_input_value(input);
	while ((i>0)&&!result)
	{
		result=((*input_source_iterator)->value())->get_input_value(input);
		input_source_iterator++;
		i--;
	}

	return (result);
}

bool Variable_composition::set_input_value_local(
	const
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
	std::list<Variable_input_value_handle>::iterator
		input_source_iterator=input_source_list.begin();
	Variable_size_type i=input_source_list.size();

	result=dependent_variable->set_input_value(input,value);
	while (i>0)
	{
		if (((*input_source_iterator)->value())->set_input_value(input,value))
		{
			result=true;
		}
		input_source_iterator++;
		i--;
	}

	return (result);
}

string_handle Variable_composition::get_string_representation_local()
//******************************************************************************
// LAST MODIFIED : 19 November 2003
//
// DESCRIPTION :
//???DB.  Overload << instead of get_string_representation?
//==============================================================================
{
	std::list<Variable_input_value_handle>::iterator
		input_source_iterator=input_source_list.begin();
	std::ostringstream out;
	string_handle return_string;
	Variable_size_type i=input_source_list.size();

	if (return_string=new std::string)
	{
		out << *(dependent_variable->get_string_representation()) << "(";
		while (i>0)
		{
			out << *(((*input_source_iterator)->value())->
				get_string_representation());
			input_source_iterator++;
			i--;
			if (i>0)
			{
				out << ",";
			}
		}
		out << ")";
		*return_string=out.str();
	}

	return (return_string);
}
