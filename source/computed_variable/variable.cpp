//******************************************************************************
// FILE : variable.cpp
//
// LAST MODIFIED : 9 September 2003
//
// DESCRIPTION :
//
// NOTES :
// 1 To get correct linking with C functions, need
//     extern "C"
//     {
//     }
//   around header files
// 2 Start by using old dynamic memory allocation ie use new(nothrow)
// 3 Need -lstdc++ when start using standard library
// 4 I was trying too much by doing Pimpl at the same time as
//   Variable_inputs.  So moved Pimpl to *.pimpl and got rid of Pimpl for the
//   moment
//==============================================================================

#include <algorithm>
#include <new>
#include <string>
using namespace std;

#include "computed_variable/variable.h"

Variable_input::Variable_input(string& specification_string)
//******************************************************************************
// LAST MODIFIED : 8 September 2003
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	//???DB.  To be done
	this->specification_string=specification_string;
} // Variable_input::Variable_input

Variable_input::Variable_input(char *specification_string)
//******************************************************************************
// LAST MODIFIED : 8 September 2003
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	//???DB.  To be done
	this->specification_string=specification_string;
} // Variable_input::Variable_input

Variable_input::Variable_input(const Variable_input& input)
//******************************************************************************
// LAST MODIFIED : 8 September 2003
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
	specification_string=input.specification_string;
} // Variable_input::Variable_input

Variable_input_value::Variable_input_value(Variable_input *input,
	Variable *value)
//******************************************************************************
// LAST MODIFIED : 7 September 2003
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	input_private=input;
	value_private=value;
} // Variable_input_value::Variable_input_value

Variable_input_value::Variable_input_value(
	const Variable_input_value& input_value)
//******************************************************************************
// LAST MODIFIED : 8 September 2003
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
	input_private=input_value.input_private;
	value_private=input_value.value_private;
} // Variable_input::Variable_input_value

Variable *Variable_input_value::value()
//******************************************************************************
// LAST MODIFIED : 7 September 2003
//
// DESCRIPTION :
//==============================================================================
{
	return (value_private);
} // Variable_input_value::value

Variable_input *Variable_input_value::input()
//******************************************************************************
// LAST MODIFIED : 7 September 2003
//
// DESCRIPTION :
//==============================================================================
{
	return (input_private);
} // Variable_input_value::input

Variable::Variable(const string& name)
//******************************************************************************
// LAST MODIFIED : 8 September 2003
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	this->name=name;
} // Variable::Variable

Variable::Variable(const char *name)
//******************************************************************************
// LAST MODIFIED : 8 September 2003
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	this->name=name;
} // Variable::Variable

Variable::~Variable()
//******************************************************************************
// LAST MODIFIED : 7 September 2003
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	//do nothing
} // Variable::Variable

class Get_set_input_value
//******************************************************************************
// LAST MODIFIED : 1 September 2003
//
// DESCRIPTION :
// A unary function (Functor)
//???DB.  What about the return value?
//==============================================================================
{
	public:
		Get_set_input_value(Variable *variable);
		~Get_set_input_value();
		int operator() (const Variable_input_value& input_value);
	private:
		Variable *variable;
}; // Get_set_input_value

int Get_set_input_value::operator() (const Variable_input_value& input_value)
//******************************************************************************
// LAST MODIFIED : 1 September 2003
//
// DESCRIPTION :
//==============================================================================
{
	int return_code;

	return_code=(variable->get_set_input_value)(
		(Variable_input_value&)input_value);

	return (return_code);
} // Get_set_input_value::operator()

Get_set_input_value::Get_set_input_value(Variable *variable)
//******************************************************************************
// LAST MODIFIED : 1 September 2003
//
// DESCRIPTION :
//==============================================================================
{
	this->variable=variable;
} // Get_set_input_value::Get_set_input_value

Get_set_input_value::~Get_set_input_value()
//******************************************************************************
// LAST MODIFIED : 1 September 2003
//
// DESCRIPTION :
//==============================================================================
{
	delete variable;
} // Get_set_input_value::~Get_set_input_value

Variable *Variable::evaluate(list<Variable_input_value> *values)
//******************************************************************************
// LAST MODIFIED : 7 September 2003
//
// DESCRIPTION :
//==============================================================================
{
	Variable *variable;

	// swap the specified <values> with the current values
	for_each(values->begin(),values->end(),Get_set_input_value(this));
	// do the local evaluate
	variable=(evaluate_local)();
	// reset the current values
	for_each(values->begin(),values->end(),Get_set_input_value(this));

	return (variable);
} // Variable::evaluate

int Variable::get_set_input_value(Variable_input_value& input_value)
//******************************************************************************
// LAST MODIFIED : 7 September 2003
//
// DESCRIPTION :
//==============================================================================
{
	int return_code;

	// do the local get_set_input_value
	return_code=(get_set_input_value_local)(input_value);

	return (return_code);
} // Variable::get_set_input_value

string *Variable::get_string_representation()
//******************************************************************************
// LAST MODIFIED : 8 September 2003
//
// DESCRIPTION :
//==============================================================================
{
	// return the sub-class get_string_representation
	return ((get_string_representation_local)());
} // Variable::get_string_representation
