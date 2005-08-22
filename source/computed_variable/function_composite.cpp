//******************************************************************************
// FILE : function_composite.cpp
//
// LAST MODIFIED : 7 April 2005
//
// DESCRIPTION :
//???DB.  Should make a Matrix if it can?  Changes type in constructors?
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

#include <sstream>

#include "computed_variable/function_composite.hpp"
#include "computed_variable/function_variable_composite.hpp"
#include "computed_variable/function_variable_union.hpp"
#include "computed_variable/function_variable_value_scalar.hpp"

// global classes
// ==============

// class Function_composite
// ------------------------

Function_composite::Function_composite(const Function_handle& function_1,
	const Function_handle& function_2):Function(),functions_list(0)
//******************************************************************************
// LAST MODIFIED : 6 December 2004
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	if (function_1&&function_2)
	{
		functions_list.push_back(function_1);
		function_1->add_dependent_function(this);
		functions_list.push_back(function_2);
		function_2->add_dependent_function(this);
	}
	else
	{
		throw Function_composite::Construction_exception();
	}
}

Function_composite::Function_composite(
	std::list<Function_handle>& functions_list):Function(),
	functions_list(functions_list)
//******************************************************************************
// LAST MODIFIED : 7 April 2005
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	bool valid;
	Function_size_type i;
	std::list<Function_handle>::iterator iterator;

	valid=false;
	if (0<(i=(this->functions_list).size()))
	{
		valid=true;
		iterator=(this->functions_list).begin();
		while (valid&&(i>0))
		{
			valid=(0!= *iterator);
			--i;
			++iterator;
		}
		if (valid)
		{
			iterator=(this->functions_list).begin();
			for (i=(this->functions_list).size();i>0;--i)
			{
				(*iterator)->add_dependent_function(this);
				++iterator;
			}
		}
	}
	if (!valid)
	{
		throw Function_composite::Construction_exception();
	}
}

Function_composite::~Function_composite()
//******************************************************************************
// LAST MODIFIED : 7 April 2005
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
#if defined (CIRCULAR_SMART_POINTERS)
	// do nothing
#else // defined (CIRCULAR_SMART_POINTERS)
	Function_size_type i;
	std::list<Function_handle>::iterator iterator;
	
	iterator=functions_list.begin();
	for (i=functions_list.size();i>0;--i)
	{
		(*iterator)->remove_dependent_function(this);
		++iterator;
	}
#endif // defined (CIRCULAR_SMART_POINTERS)
}

string_handle Function_composite::get_string_representation()
//******************************************************************************
// LAST MODIFIED : 7 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	std::list<Function_handle>::iterator function_iterator=functions_list.begin();
	std::ostringstream out;
	string_handle return_string,temp_string;
	Function_size_type i=functions_list.size();

	if (return_string=new std::string)
	{
		out << "[" << i << "](";
		while (i>0)
		{
			if ((*function_iterator)&&
				(temp_string=(*function_iterator)->get_string_representation()))
			{
				out << *temp_string;
				delete temp_string;
			}
			++function_iterator;
			--i;
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

Function_variable_handle Function_composite::input()
//******************************************************************************
// LAST MODIFIED : 7 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	std::list<Function_handle>::iterator function_iterator=functions_list.begin();
	std::list<Function_variable_handle> variables_list(0);
	Function_size_type i;

	for (i=functions_list.size();i>0;--i)
	{
		if (*function_iterator)
		{
			variables_list.push_back((*function_iterator)->input());
		}
		++function_iterator;
	}

	return (Function_variable_handle(new Function_variable_union(
		Function_handle(this),variables_list)));
}

Function_variable_handle Function_composite::output()
//******************************************************************************
// LAST MODIFIED : 7 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	std::list<Function_handle>::iterator function_iterator=functions_list.begin();
	std::list<Function_variable_handle> variables_list(0);
	Function_size_type i;

	for (i=functions_list.size();i>0;--i)
	{
		if (*function_iterator)
		{
			variables_list.push_back((*function_iterator)->output());
		}
		++function_iterator;
	}

	return (Function_variable_handle(new Function_variable_composite(
		Function_handle(this),variables_list)));
}

bool Function_composite::operator==(const Function& function) const
//******************************************************************************
// LAST MODIFIED : 7 April 2005
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
			const Function_composite& function_composite=
				dynamic_cast<const Function_composite&>(function);

			std::list<Function_handle>::const_iterator function_iterator_1,
				function_iterator_1_end,function_iterator_2,function_iterator_2_end;

			function_iterator_1=functions_list.begin();
			function_iterator_1_end=functions_list.end();
			function_iterator_2=function_composite.functions_list.begin();
			function_iterator_2_end=function_composite.functions_list.end();
			while ((function_iterator_1!=function_iterator_1_end)&&
				(function_iterator_2!=function_iterator_2_end)&&
				equivalent(*function_iterator_1,*function_iterator_2))
			{
				++function_iterator_1;
				++function_iterator_2;
			}
			result=((function_iterator_1==function_iterator_1_end)&&
				(function_iterator_2==function_iterator_2_end));
		}
		catch (std::bad_cast)
		{
			// do nothing
		}
	}

	return (result);
}

#if defined (EVALUATE_RETURNS_VALUE)
Function_handle Function_composite::evaluate(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 7 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle result(0);
	Function_size_type i=functions_list.size();
	std::list<Function_handle>::iterator function_iterator=functions_list.begin();

	while ((!result)&&(i>0))
	{
		if (*function_iterator)
		{
			result=(*function_iterator)->evaluate(atomic_variable);
		}
		--i;
		++function_iterator;
	}

	return (result);
}
#else // defined (EVALUATE_RETURNS_VALUE)
bool Function_composite::evaluate(Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 7 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	bool result(true);
	Function_size_type i=functions_list.size();
	std::list<Function_handle>::iterator function_iterator=functions_list.begin();

	while (result&&(i>0))
	{
		if (*function_iterator)
		{
			result=(*function_iterator)->evaluate(atomic_variable);
		}
		--i;
		++function_iterator;
	}

	return (result);
}
#endif // defined (EVALUATE_RETURNS_VALUE)

bool Function_composite::evaluate_derivative(Scalar& derivative,
	Function_variable_handle atomic_variable,
	std::list<Function_variable_handle>& atomic_independent_variables)
//******************************************************************************
// LAST MODIFIED : 7 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	Function_size_type i=functions_list.size();
	std::list<Function_handle>::iterator function_iterator=functions_list.begin();

	result=false;
	while ((!result)&&(i>0))
	{
		if (*function_iterator)
		{
			result=(*function_iterator)->evaluate_derivative(derivative,
				atomic_variable,atomic_independent_variables);
		}
		--i;
		++function_iterator;
	}

	return (result);
}

bool Function_composite::set_value(Function_variable_handle atomic_variable,
	Function_variable_handle atomic_value)
//******************************************************************************
// LAST MODIFIED : 7 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	Function_size_type i=functions_list.size();
	std::list<Function_handle>::iterator function_iterator=functions_list.begin();

	result=false;
	while ((!result)&&(i>0))
	{
		if (*function_iterator)
		{
			result=(*function_iterator)->set_value(atomic_variable,atomic_value);
		}
		--i;
		++function_iterator;
	}

	return (result);
}

Function_handle Function_composite::get_value(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 7 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	Function_handle result(0);
	Function_size_type i=functions_list.size();
	std::list<Function_handle>::iterator function_iterator=functions_list.begin();

	while ((!result)&&(i>0))
	{
		if (*function_iterator)
		{
			result=(*function_iterator)->get_value(atomic_variable);
		}
		--i;
		++function_iterator;
	}

	return (result);
}

Function_composite::Function_composite(
	const Function_composite& function_composite):Function(),
	functions_list(function_composite.functions_list)
//******************************************************************************
// LAST MODIFIED : 7 April 2005
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
	Function_size_type i;
	std::list<Function_handle>::iterator iterator;

	iterator=functions_list.begin();
	for (i=functions_list.size();i>0;--i)
	{
		(*iterator)->add_dependent_function(this);
		++iterator;
	}
}

Function_composite& Function_composite::operator=(
	const Function_composite& function_composite)
//******************************************************************************
// LAST MODIFIED : 7 April 2005
//
// DESCRIPTION :
// Assignment operator.
//==============================================================================
{
	Function_size_type i;
	std::list<Function_handle>::const_iterator const_iterator;
	std::list<Function_handle>::iterator iterator;

	const_iterator=function_composite.functions_list.begin();
	for (i=function_composite.functions_list.size();i>0;--i)
	{
		(*const_iterator)->add_dependent_function(this);
		++const_iterator;
	}
	iterator=functions_list.begin();
	for (i=functions_list.size();i>0;--i)
	{
		(*iterator)->remove_dependent_function(this);
		++iterator;
	}
	functions_list=function_composite.functions_list;

	return (*this);
}
