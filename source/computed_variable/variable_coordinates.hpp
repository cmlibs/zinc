//******************************************************************************
// FILE : variable_coordinates.hpp
//
// LAST MODIFIED : 20 November 2003
//
// DESCRIPTION :
// Implements variables which transform between coordinate systems.
//==============================================================================
#if !defined (__VARIABLE_COORDINATES_HPP__)
#define __VARIABLE_COORDINATES_HPP__

#include <list>
#include "computed_variable/variable.hpp"

class Variable_prolate_spheroidal_to_rectangular_cartesian : public Variable
//******************************************************************************
// LAST MODIFIED : 20 November 2003
//
// DESCRIPTION :
// Converts from prolate spheroidal to rectangular cartesian.
//==============================================================================
{
	public:
		// constructor
		Variable_prolate_spheroidal_to_rectangular_cartesian(const Scalar lambda=0,
			const Scalar mu=0,const Scalar theta=0,const Scalar focus=1);
		// copy constructor
		Variable_prolate_spheroidal_to_rectangular_cartesian(
			const Variable_prolate_spheroidal_to_rectangular_cartesian&);
		// assignment
		Variable_prolate_spheroidal_to_rectangular_cartesian& operator=(
			const Variable_prolate_spheroidal_to_rectangular_cartesian&);
		// destructor
		~Variable_prolate_spheroidal_to_rectangular_cartesian();
		// get the number of scalars in the result
		Variable_size_type size();
		// get the scalars in the result
		Vector *scalars();
		// input specifier
		Variable_input_handle input_prolate();
		Variable_input_handle input_lambda();
		Variable_input_handle input_mu();
		Variable_input_handle input_theta();
		Variable_input_handle input_focus();
	private:
		Variable_handle evaluate_local();
		void evaluate_derivative_local(Matrix& matrix,
			std::list<Variable_input_handle>& independent_variables);
		Variable_handle get_input_value_local(const Variable_input_handle& input);
		int set_input_value_local(const Variable_input_handle& input,
			const Variable_handle& value);
		string_handle get_string_representation_local();
	private:
		Scalar focus,lambda,mu,theta;
};

#if defined (USE_INTRUSIVE_SMART_POINTER)
typedef
	boost::intrusive_ptr<Variable_prolate_spheroidal_to_rectangular_cartesian>
	Variable_prolate_spheroidal_to_rectangular_cartesian_handle;
#elif defined (USE_SMART_POINTER)
typedef boost::shared_ptr<Variable_prolate_spheroidal_to_rectangular_cartesian>
	Variable_prolate_spheroidal_to_rectangular_cartesian_handle;
#else
typedef Variable_prolate_spheroidal_to_rectangular_cartesian *
	Variable_prolate_spheroidal_to_rectangular_cartesian_handle;
#endif

#endif /* !defined (__VARIABLE_COORDINATES_HPP__) */
