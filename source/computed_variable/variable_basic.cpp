//******************************************************************************
// FILE : variable_basic.cpp
//
// LAST MODIFIED : 9 September 2003
//
// DESCRIPTION :
// Basic types - scalar, vector, matrix
//
//???DB.  Should be templates?
//==============================================================================

#include <new>
#include <string>
#include <stdio.h>
using namespace std;

#include "computed_variable/variable_basic.h"

Variable *Variable_scalar::evaluate_local()
//******************************************************************************
// LAST MODIFIED : 7 September 2003
//
// DESCRIPTION :
//==============================================================================
{
	return (this);
} // Variable_scalar::evaluate_local

int Variable_scalar::get_set_input_value_local(
	Variable_input_value& input_value)
//******************************************************************************
// LAST MODIFIED : 7 September 2003
//
// DESCRIPTION :
//==============================================================================
{
	int return_code;

	//???DB.  Temporary code
	// fail
	return_code=0;
	if (input_value.value())
	{
		return_code=1;
	}

	return (return_code);
} // Variable_scalar::get_set_input_value_local

string *Variable_scalar::get_string_representation_local()
//******************************************************************************
// LAST MODIFIED : 8 September 2003
//
// DESCRIPTION :
//==============================================================================
{
	char temp_c_string[50];
		// large enough for one %g plus a few characters
		//???DB.  using temp_c_string for string until learn more about string class
	string *return_string;

	sprintf(temp_c_string,"%g",value);
	if (return_string=new(nothrow) string)
	{
		*return_string=temp_c_string;
	}

	return (return_string);
} // Variable_scalar::get_string_representation_local

Variable_scalar::Variable_scalar(const string& name,const Scalar value) :
	Variable(name)
//******************************************************************************
// LAST MODIFIED : 8 September 2003
//
// DESCRIPTION :
// Constructor.
//???DB.  What to do for new(nothrow) failure?  How to fail constructor?
//==============================================================================
{
	this->value=value;
} // Variable_scalar::Variable_scalar

Variable_scalar::Variable_scalar(const char *name,const Scalar value) :
	Variable(name)
//******************************************************************************
// LAST MODIFIED : 8 September 2003
//
// DESCRIPTION :
// Constructor.
//???DB.  What to do for new(nothrow) failure?  How to fail constructor?
//==============================================================================
{
	this->value=value;
} // Variable_scalar::Variable_scalar

Variable_scalar::~Variable_scalar()
//******************************************************************************
// LAST MODIFIED : 7 September 2003
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	// do nothing
} // Variable_scalar::Variable_scalar
