//******************************************************************************
// FILE : function_coordinates.hpp
//
// LAST MODIFIED : 23 June 2004
//
// DESCRIPTION :
// Functions which transform between coordinate systems.
//==============================================================================
#if !defined (__FUNCTION_COORDINATES_HPP__)
#define __FUNCTION_COORDINATES_HPP__

#include <list>
#include "computed_variable/function.hpp"

class Function_prolate_spheroidal_to_rectangular_cartesian;

typedef
	boost::intrusive_ptr<Function_prolate_spheroidal_to_rectangular_cartesian>
	Function_prolate_spheroidal_to_rectangular_cartesian_handle;

class Function_prolate_spheroidal_to_rectangular_cartesian : public Function
//******************************************************************************
// LAST MODIFIED : 23 June 2004
//
// DESCRIPTION :
// Converts from prolate spheroidal to rectangular cartesian.
//==============================================================================
{
	public:
		// constructor
		Function_prolate_spheroidal_to_rectangular_cartesian(const Scalar lambda=0,
			const Scalar mu=0,const Scalar theta=0,const Scalar focus=1);
		// destructor
		~Function_prolate_spheroidal_to_rectangular_cartesian();
	// inherited
	public:
		string_handle get_string_representation();
		Function_variable_handle input();
		Function_variable_handle output();
	// additional
	public:
		// variables
		Function_variable_handle
			component(std::string component_name),
			component(Function_size_type component_number),
				// component_number 1 is the first component
			focus(),
			lambda(),
			prolate(),
			mu(),
			theta();
		// values
		Scalar
			focus_value(),
			lambda_value(),
			mu_value(),
			theta_value(),
			x_value(),
			y_value(),
			z_value();
		Function_size_type number_of_components();
	private:
		Function_handle evaluate(Function_variable_handle atomic_variable);
		bool evaluate_derivative(Scalar& derivative,
			Function_variable_handle atomic_variable,
			std::list<Function_variable_handle>& atomic_independent_variables);
		bool set_value(Function_variable_handle atomic_variable,
			Function_variable_handle atomic_value);
		Function_handle get_value(Function_variable_handle atomic_variable);
	private:
		// copy constructor
		Function_prolate_spheroidal_to_rectangular_cartesian(
			const Function_prolate_spheroidal_to_rectangular_cartesian&);
		// assignment
		Function_prolate_spheroidal_to_rectangular_cartesian& operator=(
			const Function_prolate_spheroidal_to_rectangular_cartesian&);
	private:
		Function_size_type number_of_components_private;
		Scalar focus_private,lambda_private,mu_private,theta_private,x_private,
			y_private,z_private;
};

#endif /* !defined (__FUNCTION_COORDINATES_HPP__) */
