//******************************************************************************
// FILE : function_composite.cpp
//
// LAST MODIFIED : 6 December 2004
//
// DESCRIPTION :
//???DB.  Should make a Matrix if it can?  Changes type in constructors?
//==============================================================================

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
// LAST MODIFIED : 6 December 2004
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
			i--;
			iterator++;
		}
		if (valid)
		{
			iterator=(this->functions_list).begin();
			for (i=(this->functions_list).size();i>0;i--)
			{
				(*iterator)->add_dependent_function(this);
				iterator++;
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
// LAST MODIFIED : 6 December 2004
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
	for (i=functions_list.size();i>0;i--)
	{
		(*iterator)->remove_dependent_function(this);
		iterator++;
	}
#endif // defined (CIRCULAR_SMART_POINTERS)
}

string_handle Function_composite::get_string_representation()
//******************************************************************************
// LAST MODIFIED : 5 Auguat 2004
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
			function_iterator++;
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

Function_variable_handle Function_composite::input()
//******************************************************************************
// LAST MODIFIED : 5 August 2004
//
// DESCRIPTION :
//==============================================================================
{
	std::list<Function_handle>::iterator function_iterator=functions_list.begin();
	std::list<Function_variable_handle> variables_list(0);
	Function_size_type i;

	for (i=functions_list.size();i>0;i--)
	{
		if (*function_iterator)
		{
			variables_list.push_back((*function_iterator)->input());
		}
		function_iterator++;
	}

	return (Function_variable_handle(new Function_variable_union(
		Function_handle(this),variables_list)));
}

Function_variable_handle Function_composite::output()
//******************************************************************************
// LAST MODIFIED : 5 August 2004
//
// DESCRIPTION :
//==============================================================================
{
	std::list<Function_handle>::iterator function_iterator=functions_list.begin();
	std::list<Function_variable_handle> variables_list(0);
	Function_size_type i;

	for (i=functions_list.size();i>0;i--)
	{
		if (*function_iterator)
		{
			variables_list.push_back((*function_iterator)->output());
		}
		function_iterator++;
	}

	return (Function_variable_handle(new Function_variable_composite(
		Function_handle(this),variables_list)));
}

bool Function_composite::operator==(const Function& function) const
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
				function_iterator_1++;
				function_iterator_2++;
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

Function_handle Function_composite::evaluate(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 10 March 2004
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
		i--;
		function_iterator++;
	}

	return (result);
}

bool Function_composite::evaluate_derivative(Scalar& derivative,
	Function_variable_handle atomic_variable,
	std::list<Function_variable_handle>& atomic_independent_variables)
//******************************************************************************
// LAST MODIFIED : 10 March 2004
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
		i--;
		function_iterator++;
	}

	return (result);
}

bool Function_composite::set_value(Function_variable_handle atomic_variable,
	Function_variable_handle atomic_value)
//******************************************************************************
// LAST MODIFIED : 10 March 2004
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
		i--;
		function_iterator++;
	}

	return (result);
}

Function_handle Function_composite::get_value(
	Function_variable_handle atomic_variable)
//******************************************************************************
// LAST MODIFIED : 22 June 2004
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
		i--;
		function_iterator++;
	}

	return (result);
}

Function_composite::Function_composite(
	const Function_composite& function_composite):Function(),
	functions_list(function_composite.functions_list)
//******************************************************************************
// LAST MODIFIED : 6 December 2004
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
	Function_size_type i;
	std::list<Function_handle>::iterator iterator;

	iterator=functions_list.begin();
	for (i=functions_list.size();i>0;i--)
	{
		(*iterator)->add_dependent_function(this);
		iterator++;
	}
}

Function_composite& Function_composite::operator=(
	const Function_composite& function_composite)
//******************************************************************************
// LAST MODIFIED : 6 December 2004
//
// DESCRIPTION :
// Assignment operator.
//==============================================================================
{
	Function_size_type i;
	std::list<Function_handle>::const_iterator const_iterator;
	std::list<Function_handle>::iterator iterator;

	const_iterator=function_composite.functions_list.begin();
	for (i=function_composite.functions_list.size();i>0;i--)
	{
		(*const_iterator)->add_dependent_function(this);
		const_iterator++;
	}
	iterator=functions_list.begin();
	for (i=functions_list.size();i>0;i--)
	{
		(*iterator)->remove_dependent_function(this);
		iterator++;
	}
	functions_list=function_composite.functions_list;

	return (*this);
}
