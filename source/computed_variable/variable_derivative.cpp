//******************************************************************************
// FILE : variable_derivative.cpp
//
// LAST MODIFIED : 4 February 2004
//
// DESCRIPTION :
//==============================================================================
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include "computed_variable/variable_base.hpp"

#include <new>
#include <sstream>
#include <string>
#include <stdio.h>

#include "computed_variable/variable_derivative.hpp"

// global classes
// ==============

// class Variable_derivative
// -------------------------

Variable_derivative::Variable_derivative(
	const Variable_handle& dependent_variable,
	std::list<
#if defined (USE_VARIABLE_INPUT)
	Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
	Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	>& independent_variables):Variable(),
	dependent_variable(dependent_variable),
	independent_variables(independent_variables)
//******************************************************************************
// LAST MODIFIED : 2 February 2004
//
// DESCRIPTION :
//==============================================================================
{
}

Variable_derivative::Variable_derivative(
	const Variable_derivative& variable_derivative):Variable(),
	dependent_variable(variable_derivative.dependent_variable),
	independent_variables(variable_derivative.independent_variables)
//******************************************************************************
// LAST MODIFIED : 14 November 2003
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
}

Variable_derivative& Variable_derivative::operator=(
	const Variable_derivative& variable_derivative)
//******************************************************************************
// LAST MODIFIED : 14 November 2003
//
// DESCRIPTION :
// Assignment operator.
//==============================================================================
{
	dependent_variable=variable_derivative.dependent_variable;
	independent_variables=variable_derivative.independent_variables;

	return (*this);
}

Variable_derivative::~Variable_derivative()
//******************************************************************************
// LAST MODIFIED : 14 November 2003
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	// do nothing
}

Variable_handle Variable_derivative::evaluate_derivative(
	std::list<
#if defined (USE_VARIABLE_INPUT)
	Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
	Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	>& independent_variables,
	std::list<Variable_input_value_handle>& values)
//******************************************************************************
// LAST MODIFIED : 2 February 2004
//
// DESCRIPTION :
// Overload Variable::evaluate_derivative
//==============================================================================
{
	std::list<
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
		> merged_independent_variables=this->independent_variables;

	merged_independent_variables.insert(merged_independent_variables.end(),
		independent_variables.begin(),independent_variables.end());

	return (dependent_variable->evaluate_derivative(merged_independent_variables,
		values));
}

#if defined (USE_ITERATORS)
//???DB.  To be done
#else // defined (USE_ITERATORS)
class Variable_derivative_calculate_size_functor
//******************************************************************************
// LAST MODIFIED : 15 January 2004
//
// DESCRIPTION :
// A unary function (functor) for calculating the size of a derivative variable.
//==============================================================================
{
	public:
		Variable_derivative_calculate_size_functor(
			const Variable_handle& dependent_variable,Variable_size_type& size):
			size(size),dependent_variable_size(dependent_variable->size())
		{
			size=0;
		};
#if defined (USE_ITERATORS)
		//???DB.  To be done
		//???DB.  May be similar to this because atomic inputs have size 1?
		int operator() (const Variable_input_handle&)
		{
			size += size+dependent_variable_size;
			return (0);
		};
#else // defined (USE_ITERATORS)
		int operator() (const Variable_input_handle& input)
		{
			size += (size+dependent_variable_size)*(input->size());
			return (0);
		};
#endif // defined (USE_ITERATORS)
	private:
		Variable_size_type& size;
		Variable_size_type dependent_variable_size;
};

Variable_size_type Variable_derivative::size() const
//******************************************************************************
// LAST MODIFIED : 14 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	Variable_size_type size;

	// get the specified values
	std::for_each(independent_variables.begin(),independent_variables.end(),
		Variable_derivative_calculate_size_functor(dependent_variable,size));

	return (size);
}

Vector *Variable_derivative::scalars()
//******************************************************************************
// LAST MODIFIED : 14 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	return (evaluate_local()->scalars());
}
#endif // defined (USE_ITERATORS)

Variable_handle Variable_derivative::clone() const
//******************************************************************************
// LAST MODIFIED : 8 December 2003
//
// DESCRIPTION :
//==============================================================================
{
	return (Variable_derivative_handle(new Variable_derivative(*this)));
}

Variable_handle Variable_derivative::evaluate_local()
//******************************************************************************
// LAST MODIFIED : 14 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	std::list<Variable_input_value_handle> values(0);

	return (dependent_variable->evaluate_derivative(independent_variables,
		values));
}

bool Variable_derivative::evaluate_derivative_local(Matrix&,
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
		"Variable_derivative::evaluate_derivative_local.  "
		"Should not come here"));
	
	return (false);
}

Variable_handle Variable_derivative::get_input_value_local(
	const
#if defined (USE_VARIABLE_INPUT)
	Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
	Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	& input)
//******************************************************************************
// LAST MODIFIED : 2 February 2004
//
// DESCRIPTION :
//???DB.  Could use get_input_value_local
//==============================================================================
{
	return (dependent_variable->get_input_value(input));
}

bool Variable_derivative::set_input_value_local(
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
// LAST MODIFIED : 4 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (dependent_variable->set_input_value(input,value));
}

string_handle Variable_derivative::get_string_representation_local()
//******************************************************************************
// LAST MODIFIED : 14 November 2003
//
// DESCRIPTION :
//???DB.  Overload << instead of get_string_representation?
//???DB.  Need get_string_representation for inputs to do properly?
//==============================================================================
{
	string_handle return_string;
	std::ostringstream out;

	if (return_string=new std::string)
	{
		out << "derivative";
		*return_string=out.str();
	}

	return (return_string);
}
