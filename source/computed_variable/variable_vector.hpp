//******************************************************************************
// FILE : variable_vector.hpp
//
// LAST MODIFIED : 9 November 2003
//
// DESCRIPTION :
//==============================================================================
#if !defined (__VARIABLE_VECTOR_HPP__)
#define __VARIABLE_VECTOR_HPP__

#include <list>
#include "computed_variable/variable.hpp"

class Variable_vector : public Variable
//******************************************************************************
// LAST MODIFIED : 9 November 2003
//
// DESCRIPTION :
// An identity variable whose input/output is a vector.
//==============================================================================
{
	// can be used by any function
	public:
		// constructor
		Variable_vector(const Vector& values);
		// copy constructor
		Variable_vector(const Variable_vector&);
		// assignment
		Variable_vector& operator=(const Variable_vector&);
		// destructor
		~Variable_vector();
		// indexing
#if defined (NEW_CODE)
		const Scalar& operator[](Variable_size_type) const;
#endif // defined (NEW_CODE)
		Scalar& operator[](Variable_size_type);
		// get the number of scalars in the result
		Variable_size_type size();
		// get the scalars in the result
		Vector *scalars();
		// input specifier
		Variable_input_handle input_values();
		Variable_input_handle input_values(Variable_size_type);
		Variable_input_handle input_values(
			const boost::numeric::ublas::vector<Variable_size_type>);
	private:
		Variable_handle evaluate_local();
		void evaluate_derivative_local(Matrix& matrix,
			std::list<Variable_input_handle>& independent_variables);
		Variable_handle get_input_value_local(const Variable_input_handle& input);
		int set_input_value_local(const Variable_input_handle& input,
			const Variable_handle& value);
		string_handle get_string_representation_local();
	private:
		Vector values;
};

#if defined (USE_INTRUSIVE_SMART_POINTER)
typedef boost::intrusive_ptr<Variable_vector> Variable_vector_handle;
#elif defined (USE_SMART_POINTER)
typedef boost::shared_ptr<Variable_vector> Variable_vector_handle;
#else
typedef Variable_vector * Variable_vector_handle;
#endif

#endif /* !defined (__VARIABLE_VECTOR_HPP__) */
