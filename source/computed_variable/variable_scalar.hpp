//******************************************************************************
// FILE : variable_scalar.hpp
//
// LAST MODIFIED : 15 December 2003
//
// DESCRIPTION :
//==============================================================================
#if !defined (__VARIABLE_SCALAR_HPP__)
#define __VARIABLE_SCALAR_HPP__

#include <list>
#include "computed_variable/variable.hpp"

class Variable_scalar : public Variable
//******************************************************************************
// LAST MODIFIED : 15 December 2003
//
// DESCRIPTION :
// An identity variable whose input/output is a scalar
//==============================================================================
{
	// can be used by any function
	public:
		// constructor
		Variable_scalar(const Scalar value);
		// copy constructor
		Variable_scalar(const Variable_scalar&);
		// assignment
		Variable_scalar& operator=(const Variable_scalar&);
		// destructor
		~Variable_scalar();
		// get the number of scalars in the result
		Variable_size_type size() const;
		// get the scalars in the result
		Vector *scalars();
		// input specifier
		Variable_input_handle input_value();
		virtual Scalar norm() const;
		virtual Variable_handle operator-(const Variable&) const;
		virtual Variable_handle operator-=(const Variable&);
		virtual Variable_handle clone() const;
	private:
		Variable_handle evaluate_local();
		void evaluate_derivative_local(Matrix& matrix,
			std::list<Variable_input_handle>& independent_variables);
		Variable_handle get_input_value_local(const Variable_input_handle& input);
		int set_input_value_local(const Variable_input_handle& input,
			const Variable_handle& value);
		string_handle get_string_representation_local();
	private:
		Scalar value;
};

#if defined (USE_INTRUSIVE_SMART_POINTER)
typedef boost::intrusive_ptr<Variable_scalar> Variable_scalar_handle;
#elif defined (USE_SMART_POINTER)
typedef boost::shared_ptr<Variable_scalar> Variable_scalar_handle;
#else
typedef Variable_scalar * Variable_scalar_handle;
#endif

#endif /* !defined (__VARIABLE_SCALAR_HPP__) */
