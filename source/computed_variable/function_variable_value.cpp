//******************************************************************************
// FILE : function_variable_value.cpp
//
// LAST MODIFIED : 21 July 2004
//
// DESCRIPTION :
//==============================================================================

#include "computed_variable/function_variable_value.hpp"

// global classes
// ==============

// class Function_variable_value
// =============================

Function_variable_value::Function_variable_value():reference_count(0){}

Function_variable_value::~Function_variable_value(){}

void intrusive_ptr_add_ref(Function_variable_value *variable_value)
//******************************************************************************
// LAST MODIFIED : 25 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	if (variable_value)
	{
		(variable_value->reference_count)++;
	}
}

void intrusive_ptr_release(Function_variable_value *variable_value)
//******************************************************************************
// LAST MODIFIED : 25 February 2004
//
// DESCRIPTION :
//==============================================================================
{
	if (variable_value)
	{
		(variable_value->reference_count)--;
		if (variable_value->reference_count<=0)
		{
			delete variable_value;
		}
	}
}
