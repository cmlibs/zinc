//******************************************************************************
// FILE : variable_coordinates.hpp
//
// LAST MODIFIED : 4 February 2004
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
// LAST MODIFIED : 4 February 2004
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
		// components are indivisible
#if defined (USE_ITERATORS)
		// returns the number of components that are differentiable
		virtual Variable_size_type number_differentiable() const;
#if defined (USE_VARIABLES_AS_COMPONENTS)
		virtual bool is_component();
#endif // defined (USE_VARIABLES_AS_COMPONENTS)
		// for stepping through the components that make up the Variable
#if defined (USE_VARIABLES_AS_COMPONENTS)
#if defined (USE_ITERATORS_NESTED)
		virtual Iterator begin_components();
		virtual Iterator end_components();
#else // defined (USE_ITERATORS_NESTED)
#if defined (DO_NOT_USE_ITERATOR_TEMPLATES)
#else // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
		virtual Handle_iterator<Variable_handle> begin_components();
		virtual Handle_iterator<Variable_handle> end_components();
#endif // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
#endif // defined (USE_ITERATORS_NESTED)
#else // defined (USE_VARIABLES_AS_COMPONENTS)
		virtual Handle_iterator<Variable_io_specifier_handle> begin_components();
		virtual Handle_iterator<Variable_io_specifier_handle> end_components();
#endif // defined (USE_VARIABLES_AS_COMPONENTS)
#else // defined (USE_ITERATORS)
		// get the number of scalars in the result
		Variable_size_type size() const;
		// get the scalars in the result
		Vector *scalars();
#endif // defined (USE_VARIABLE_ITERATORS)
		// input specifier
#if defined (USE_VARIABLE_INPUT)
		Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
		Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			input_prolate(),
			input_lambda(),
			input_mu(),
			input_theta(),
			input_focus();
		virtual Variable_handle clone() const;
	private:
		Variable_handle evaluate_local();
		bool evaluate_derivative_local(Matrix& matrix,
			std::list<
#if defined (USE_VARIABLE_INPUT)
			Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			>& independent_variables);
		Variable_handle get_input_value_local(
			const
#if defined (USE_VARIABLE_INPUT)
			Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			& input_atomic);
		bool set_input_value_local(const
#if defined (USE_VARIABLE_INPUT)
			Variable_input_handle
#else // defined (USE_VARIABLE_INPUT)
			Variable_io_specifier_handle
#endif // defined (USE_VARIABLE_INPUT)
			& input_atomic,
			const
#if defined (USE_VARIABLES_AS_COMPONENTS)
			Variable_handle
#else // defined (USE_VARIABLES_AS_COMPONENTS)
			Variable_io_specifier_handle
#endif // defined (USE_VARIABLES_AS_COMPONENTS)
			& value);
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
