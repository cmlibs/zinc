//******************************************************************************
// FILE : variable_input_composite.cpp
//
// LAST MODIFIED : 24 November 2003
//
// DESCRIPTION :
//==============================================================================

#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <typeinfo>

//???DB.  Put in include?
//???DB.  With smart pointers?
const bool Assert_on=true;

template<class Assertion,class Exception>inline void Assert(
	Assertion assertion,Exception exception)
{
	if (Assert_on&&!(assertion)) throw exception;
}

#include "computed_variable/variable_input_composite.hpp"

// class Variable_input_composite
// ------------------------------

Variable_input_composite::Variable_input_composite(
	std::list<Variable_input_handle>& inputs_list) : Variable_input(),
	inputs_list(0)
//******************************************************************************
// LAST MODIFIED : 24 November 2003
//
// DESCRIPTION :
// Constructor.  Needs to "flatten" the <inputs_list> ie expand any composites.
//==============================================================================
{
	std::list<Variable_input_handle>::iterator input_iterator;
	Variable_size_type i;

	// "flatten" the <inputs_list>.  Does not need to be recursive because
	//   composite inputs have flat lists
	input_iterator=inputs_list.begin();
	for (i=inputs_list.size();i>0;i--)
	{
		Variable_input_composite_handle input_composite=
#if defined (USE_SMART_POINTER)
			boost::dynamic_pointer_cast<Variable_input_composite,Variable_input>(
			*input_iterator);
#else /* defined (USE_SMART_POINTER) */
			dynamic_cast<Variable_input_composite *>(*input_iterator);
#endif /* defined (USE_SMART_POINTER) */

		if (input_composite)
		{
			(this->inputs_list).insert((this->inputs_list).end(),
				(input_composite->inputs_list).begin(),
				(input_composite->inputs_list).end());
		}
		else
		{
			(this->inputs_list).push_back(*input_iterator);
		}
		input_iterator++;
	}
}

Variable_input_composite::Variable_input_composite(
	Variable_input_handle& input_1,Variable_input_handle& input_2):
	Variable_input(),inputs_list(0)
//******************************************************************************
// LAST MODIFIED : 24 November 2003
//
// DESCRIPTION :
// Constructor.  Needs to "flatten" the <inputs_list> ie expand any composites.
//==============================================================================
{
	Variable_input_composite_handle input_1_composite=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_input_composite,Variable_input>(
		input_1);
#else /* defined (USE_SMART_POINTER) */
		dynamic_cast<Variable_input_composite *>(input_1);
#endif /* defined (USE_SMART_POINTER) */
	Variable_input_composite_handle input_2_composite=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_input_composite,Variable_input>(
		input_2);
#else /* defined (USE_SMART_POINTER) */
		dynamic_cast<Variable_input_composite *>(input_2);
#endif /* defined (USE_SMART_POINTER) */

	//  Variable_composite rather than deriving Variable_input_composite
	if (input_1_composite)
	{
		inputs_list.insert(inputs_list.end(),
			(input_1_composite->inputs_list).begin(),
			(input_1_composite->inputs_list).end());
	}
	else
	{
		inputs_list.push_back(input_1);
	}
	if (input_2_composite)
	{
		inputs_list.insert(inputs_list.end(),
			(input_2_composite->inputs_list).begin(),
			(input_2_composite->inputs_list).end());
	}
	else
	{
		inputs_list.push_back(input_2);
	}
}

Variable_input_composite& Variable_input_composite::operator=(
	const Variable_input_composite& input_composite)
//******************************************************************************
// LAST MODIFIED : 9 October 2003
//
// DESCRIPTION :
// Assignment operator.
//???DB.  Same as implicit?
//==============================================================================
{
	//???DB.  Does assignment for super class first?
	inputs_list=input_composite.inputs_list;

	return (*this);
}

class Variable_input_composite_sum_sizes_functor
//******************************************************************************
// LAST MODIFIED : 14 November 2003
//
// DESCRIPTION :
// A unary function (functor) for summing the sizes of the inputs that make a
// composite input.
//==============================================================================
{
	public:
		Variable_input_composite_sum_sizes_functor(Variable_size_type& sum):
			sum(sum)
		{
			sum=0;
		};
		int operator() (Variable_input_handle& input)
		{
			sum += input->size();
			return (0);
		};
	private:
		Variable_size_type& sum;
};

Variable_size_type Variable_input_composite::size()
//******************************************************************************
// LAST MODIFIED : 14 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	Variable_size_type sum;

	// get the specified values
	std::for_each(inputs_list.begin(),inputs_list.end(),
		Variable_input_composite_sum_sizes_functor(sum));

	return (sum);
}

bool Variable_input_composite::operator==(const Variable_input& input)
//******************************************************************************
// LAST MODIFIED : 6 November 2003
//
// DESCRIPTION :
//==============================================================================
try
{
	const Variable_input_composite& input_composite=
		dynamic_cast<const Variable_input_composite&>(input);

	return (inputs_list==input_composite.inputs_list);
}
catch (std::bad_cast)
{
	return (false);
}

std::list<Variable_input_handle>::iterator Variable_input_composite::begin()
//******************************************************************************
// LAST MODIFIED : 6 October 2003
//
// DESCRIPTION :
//==============================================================================
{
	return (inputs_list.begin());
}

std::list<Variable_input_handle>::iterator Variable_input_composite::end()
//******************************************************************************
// LAST MODIFIED : 6 October 2003
//
// DESCRIPTION :
//==============================================================================
{
	return (inputs_list.end());
}
