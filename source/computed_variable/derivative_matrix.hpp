//******************************************************************************
// FILE : derivative_matrix.hpp
//
// LAST MODIFIED : 23 December 2004
//
// DESCRIPTION :
// ???DB.  Maybe move into function_derivative?
//==============================================================================
#if !defined (__DERIVATIVE_MATRIX_HPP__)
#define __DERIVATIVE_MATRIX_HPP__

#include <list>
#include <vector>

#include "computed_variable/function_base.hpp"

class Derivative_matrix : public std::list<Matrix>
//******************************************************************************
// LAST MODIFIED : 23 December 2004
//
// DESCRIPTION :
//==============================================================================
{
	public:
		// for construction exception
		class Construction_exception {};
		// for calculation exception
		class Calculation_exception {};
		// constructors
		Derivative_matrix();
		Derivative_matrix(const std::list<Matrix>& matrices);
		// copy constructor
		Derivative_matrix(const Derivative_matrix&);
#if defined (OLD_CODE)
		// assignment
		Derivative_matrix& operator=(const Derivative_matrix&);
#endif // defined (OLD_CODE)
		// destructor
		~Derivative_matrix();
		// implement the chain rule for differentiation
		Derivative_matrix operator*(const Derivative_matrix&) const;
		// calculate the chain rule inverse
		Derivative_matrix inverse();
	private:
		Function_size_type number_of_dependent_values,order;
		std::vector<Function_size_type> numbers_of_independent_values;
};

#endif /* !defined (__DERIVATIVE_MATRIX_HPP__) */
