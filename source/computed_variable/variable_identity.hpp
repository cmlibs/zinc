//******************************************************************************
// FILE : variable_identity.hpp
//
// LAST MODIFIED : 17 December 2003
//
// DESCRIPTION :
// Used when calculating derivatives eg. composition and inverse.
//==============================================================================
#if !defined (__VARIABLE_IDENTITY_HPP__)
#define __VARIABLE_DENTITY_HPP__

#include "computed_variable/variable.hpp"

// global classes
// ==============

class Variable_identity : public Variable
//******************************************************************************
// LAST MODIFIED : 17 December 2003
//
// DESCRIPTION :
// An identity variable with a specified input.  Used in calculating
// derivatives.
//==============================================================================
{
	public:
		// constructor
		Variable_identity(const Variable_input_handle);
		// copy constructor
		Variable_identity(const Variable_identity&);
		// assignment
		Variable_identity& operator=(const Variable_identity&);
		// destructor
		~Variable_identity();
		// get the number of scalars in the result
		Variable_size_type size() const;
		// get the scalars in the result
		Vector *scalars();
		// input specifier
		Variable_input_handle input();
		Variable_handle clone() const;
	private:
		Variable_handle evaluate_local();
		void evaluate_derivative_local(Matrix&,std::list<Variable_input_handle>&);
		Variable_handle get_input_value_local(const Variable_input_handle&);
		int set_input_value_local(const Variable_input_handle&,
			const Variable_handle&);
		string_handle get_string_representation_local();
	private:
		Variable_input_handle input_private;
};

#if defined (USE_INTRUSIVE_SMART_POINTER)
typedef boost::intrusive_ptr<Variable_identity> Variable_identity_handle;
#elif defined (USE_SMART_POINTER)
typedef boost::shared_ptr<Variable_identity> Variable_identity_handle;
#else
typedef Variable_identity * Variable_identity_handle;
#endif

#endif // !defined (__VARIABLE_IDENTITY_HPP__)
