//******************************************************************************
// FILE : variable_basic.h
//
// LAST MODIFIED : 8 September 2003
//
// DESCRIPTION :
// Basic types - scalar, vector, matrix
//==============================================================================
#if !defined (__VARIABLE_BASIC_H__)
#define __VARIABLE_BASIC_H__

#include "computed_variable/variable.h"

typedef float Scalar;

class Variable_scalar : public Variable
//******************************************************************************
// LAST MODIFIED : 8 September 2003
//
// DESCRIPTION :
// An identity variable whose input/output is a scalar
//==============================================================================
{
	// can be used by any function
	public:
		// constructor
		Variable_scalar(const string& name,const Scalar value);
		Variable_scalar(const char *name,const Scalar value);
		// destructor
		~Variable_scalar();
	private:
		Variable *evaluate_local();
		int get_set_input_value_local(Variable_input_value & input_value);
		string *get_string_representation_local();
		Scalar value;
}; // class Variable_scalar
#endif /* !defined (__VARIABLE_BASIC_H__) */
