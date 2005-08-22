//******************************************************************************
// FILE : variable_identity.cpp
//
// LAST MODIFIED : 9 February 2004
//
// DESCRIPTION :
// Used when calculating derivatives eg. composition and inverse.
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

#include "computed_variable/variable_identity.hpp"
#include "computed_variable/variable_input_composite.hpp"

// global classes
// ==============

// class Variable_identity
// -----------------------

Variable_identity::Variable_identity(const
#if defined (USE_VARIABLE_INPUT)
	Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
	Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	input):Variable(),input_private(input) {}
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================

Variable_identity::Variable_identity(
	const Variable_identity& variable_identity):Variable(),
	input_private(variable_identity.input_private) {}
//******************************************************************************
// LAST MODIFIED : 17 December 2003
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================

Variable_identity& Variable_identity::operator=(
	const Variable_identity& variable_identity)
//******************************************************************************
// LAST MODIFIED : 17 December 2003
//
// DESCRIPTION :
// Assignment operator.
//==============================================================================
{
	this->input_private=variable_identity.input_private;

	return (*this);
}

Variable_identity::~Variable_identity() {}
//******************************************************************************
// LAST MODIFIED : 17 December 2003
//
// DESCRIPTION :
// Destructor.
//==============================================================================

Variable_size_type Variable_identity::
#if defined (USE_ITERATORS)
	number_differentiable
#else // defined (USE_ITERATORS)
	size
#endif // defined (USE_ITERATORS)
	() const
//******************************************************************************
// LAST MODIFIED : 20 January 2004
//
// DESCRIPTION :
// Get the number of scalars in the result.
//==============================================================================
{
#if defined (USE_ITERATORS)
	return (input_private->number_differentiable());
#else // defined (USE_ITERATORS)
	return (input_private->size());
#endif // defined (USE_ITERATORS)
}

#if defined (USE_ITERATORS)
#else // defined (USE_ITERATORS)
Vector *Variable_identity::scalars()
//******************************************************************************
// LAST MODIFIED : 17 December 2003
//
// DESCRIPTION :
// Get the scalars in the result.
//==============================================================================
{
	//???DB.  Could have a variable as part of data, but don't know how to
	//  get and not needed for calculating derivative of composition
	return ((Vector *)0);
}
#endif // defined (USE_ITERATORS)

#if defined (USE_VARIABLE_INPUT)
Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	Variable_identity::input()
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	return (input_private);
}

Variable_handle Variable_identity::clone() const
//******************************************************************************
// LAST MODIFIED : 17 December 2003
//
// DESCRIPTION :
//==============================================================================
{
	return (Variable_identity_handle(new Variable_identity(*this)));
}

Variable_handle Variable_identity::evaluate_local()
//******************************************************************************
// LAST MODIFIED : 17 December 2003
//
// DESCRIPTION :
//==============================================================================
{
	//???DB.  Could have a variable as part of data, but don't know how to
	//  get and not needed for calculating derivative of composition
	return (Variable_handle(0));
}

#if defined (USE_ITERATORS)
//???DB.  To be done
#else // defined (USE_ITERATORS)
bool Variable_identity::evaluate_derivative_local(Matrix& matrix,
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

	result=true;
	// matrix is zero'd on entry
	if (1==independent_variables.size())
	{
#if defined (USE_SCALAR_MAPPING)
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			dependent_input=input_private,
			independent_input=independent_variables.front();
		std::list< std::pair<Variable_size_type,Variable_size_type> >
			independent_to_dependent_scalar_mapping=independent_input->scalar_mapping(
			dependent_input);
		
		if (0<independent_to_dependent_scalar_mapping.size())
		{
			Variable_size_type
				dependent_size=(independent_to_dependent_scalar_mapping.back()).second,
				independent_size=(independent_to_dependent_scalar_mapping.back()).first;

			Assert((matrix.size1()==dependent_size)&&
				(matrix.size2()==independent_size),std::logic_error(
				"Variable_identity::evaluate_derivative_local.  "
				"Incorrect matrix size"));

			std::list< std::pair<Variable_size_type,Variable_size_type> >::iterator
				mapping_iterator;
			Variable_size_type column,i,j,row;

			mapping_iterator=independent_to_dependent_scalar_mapping.begin();
			for (i=independent_to_dependent_scalar_mapping.size()-1;i>0;i--)
			{
				column=mapping_iterator->first;
				row=mapping_iterator->second;
				mapping_iterator++;
				if (row<dependent_size)
				{
					for (j=mapping_iterator->first-column;j>0;j--)
					{
						matrix(row,column)=1;
						row++;
						column++;
					}
				}
			}
		}
#elif defined (USE_ITERATORS)
		//???DB.  To be done
		//???DB.  Using up <matrix> argument
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			dependent_input=input_private,
			independent_input=independent_variables.front();
		Variable_size_type
			dependent_size=1,
			independent_size=1;

		Assert((matrix.size1()==dependent_size)&&
			(matrix.size2()==independent_size),std::logic_error(
			"Variable_identity::evaluate_derivative_local.  "
			"Incorrect matrix size"));
#else
		std::list<
#if defined (USE_VARIABLE_INPUT)
			Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			> dependent_inputs(0),independent_inputs(0);
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			independent_input=independent_variables.front();
		Variable_input_composite_handle independent_input_composite=
#if defined (USE_SMART_POINTER)
			boost::dynamic_pointer_cast<Variable_input_composite,
#if defined (USE_VARIABLE_INPUT)
			Variable_input
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
			>(independent_input);
#else /* defined (USE_SMART_POINTER) */
			dynamic_cast<Variable_input_composite *>(independent_input);
#endif /* defined (USE_SMART_POINTER) */
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			dependent_input=input_private;
		Variable_input_composite_handle dependent_input_composite=
#if defined (USE_SMART_POINTER)
			boost::dynamic_pointer_cast<Variable_input_composite,
#if defined (USE_VARIABLE_INPUT)
			Variable_input
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier
#endif // defined (USE_VARIABLE_INPUT)
			>(dependent_input);
#else /* defined (USE_SMART_POINTER) */
			dynamic_cast<Variable_input_composite *>(dependent_input);
#endif /* defined (USE_SMART_POINTER) */
		Variable_size_type column,i,j,k,row;

		Assert((matrix.size1()==dependent_input->size())&&
			(matrix.size2()==independent_input->size()),std::logic_error(
			"Variable_identity::evaluate_derivative_local.  "
			"Incorrect matrix size"));
		if (independent_input_composite)
		{
			independent_inputs.insert(independent_inputs.end(),
				independent_input_composite->begin(),
				independent_input_composite->end());
		}
		else
		{
			independent_inputs.push_back(independent_input);
		}
		if (dependent_input_composite)
		{
			dependent_inputs.insert(dependent_inputs.end(),
				dependent_input_composite->begin(),
				dependent_input_composite->end());
		}
		else
		{
			dependent_inputs.push_back(dependent_input);
		}
		std::list<
#if defined (USE_VARIABLE_INPUT)
			Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			>::iterator dependent_input_iterator=
			dependent_inputs.begin();
		row=0;
		for (i=dependent_inputs.size();i>0;i--)
		{
			std::list<
#if defined (USE_VARIABLE_INPUT)
				Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
				Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
				>::iterator independent_input_iterator=
				independent_inputs.begin();
			Variable_size_type number_of_dependent=
				(*dependent_input_iterator)->size();

			column=0;
			for (j=independent_inputs.size();j>0;j--)
			{
				if (*dependent_input_iterator== *independent_input_iterator)
				{
					for (k=0;k<number_of_dependent;k++)
					{
						matrix(row+k,column+k)=1;
					}
				}
				column += (*independent_input_iterator)->size();
				independent_input_iterator++;
			}
			row += number_of_dependent;
			dependent_input_iterator++;
		}
#endif
	}

	return (result);
}
#endif // defined (USE_ITERATORS)

Variable_handle Variable_identity::get_input_value_local(
	const
#if defined (USE_VARIABLE_INPUT)
	Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
	Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	&)
//******************************************************************************
// LAST MODIFIED : 3 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	//???DB.  Could have a variable as part of data, but don't know how to
	//  get and not needed for calculating derivative of composition
	return (Variable_handle(0));
}

bool Variable_identity::set_input_value_local(const
#if defined (USE_VARIABLE_INPUT)
	Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
	Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
	&,
	const
#if defined (USE_VARIABLES_AS_COMPONENTS)
	Variable_handle
#else // defined (USE_VARIABLES_AS_COMPONENTS)
	Variable_io_specifier_handle
#endif // defined (USE_VARIABLES_AS_COMPONENTS)
	&)
//******************************************************************************
// LAST MODIFIED : 9 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	//???DB.  Could have a variable as part of data, but don't know how to
	//  get and not needed for calculating derivative of composition
	return (false);
}

string_handle Variable_identity::get_string_representation_local()
//******************************************************************************
// LAST MODIFIED : 17 December 2003
//
// DESCRIPTION :
//==============================================================================
{
	string_handle return_string;
	std::ostringstream out;

	if (return_string=new std::string)
	{
		out << "identity";
		*return_string=out.str();
	}

	return (return_string);
}
