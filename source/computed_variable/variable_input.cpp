//******************************************************************************
// FILE : variable_input.cpp
//
// LAST MODIFIED : 23 October 2003
//
// DESCRIPTION :
//==============================================================================

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

#include "computed_variable/variable_input.hpp"

// class Variable_input
// --------------------

Variable_input::Variable_input()
#if defined (USE_INTRUSIVE_SMART_POINTER)
	:reference_count(0)
#endif
//******************************************************************************
// LAST MODIFIED : 16 October 2003
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{}

Variable_input::~Variable_input()
//******************************************************************************
// LAST MODIFIED : 9 September 2003
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	// do nothing
}

#if defined (USE_INTRUSIVE_SMART_POINTER)
void intrusive_ptr_add_ref(Variable_input *input)
//******************************************************************************
// LAST MODIFIED : 16 October 2003
//
// DESCRIPTION :
//==============================================================================
{
	if (input)
	{
		(input->reference_count)++;
	}
}
#endif // defined (USE_INTRUSIVE_SMART_POINTER)

#if defined (USE_INTRUSIVE_SMART_POINTER)
void intrusive_ptr_release(Variable_input *input)
//******************************************************************************
// LAST MODIFIED : 16 October 2003
//
// DESCRIPTION :
//==============================================================================
{
	if (input)
	{
		(input->reference_count)--;
		if (input->reference_count<=0)
		{
			delete input;
		}
	}
}
#endif // defined (USE_INTRUSIVE_SMART_POINTER)

#if defined (OLD_CODE)
Variable_input::size_type Variable_input::size()
//******************************************************************************
// LAST MODIFIED : 11 October 2003
//
// DESCRIPTION :
// Needed for setting up virtual table by compiler.
//==============================================================================
{
	return (0);
}
#endif
