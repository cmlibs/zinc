//******************************************************************************
// FILE : variable_input_composite.hpp
//
// LAST MODIFIED : 2 February 2004
//
// DESCRIPTION :
//==============================================================================
#if !defined (__VARIABLE_INPUT_COMPOSITE_HPP__)
#define __VARIABLE_INPUT_COMPOSITE_HPP__

#include "computed_variable/variable_base.hpp"

#include <list>

#if defined (GENERALIZE_COMPOSITE_INPUT)
#include "computed_variable/variable.hpp"
#else // defined (GENERALIZE_COMPOSITE_INPUT)
#include "computed_variable/variable_input.hpp"
#endif // defined (GENERALIZE_COMPOSITE_INPUT)

class Variable_input_composite : public Variable_input
//******************************************************************************
// LAST MODIFIED : 2 February 2004
//
// DESCRIPTION :
// A composite of other input(s).
//
// Composite inputs are "flat" in the sense that there list of inputs does
// not contain composite inputs.  This means that the constructors have to
// flatten the list.
//==============================================================================
{
#if defined (GENERALIZE_COMPOSITE_INPUT)
	protected:
		// constructor.  Protected so that can't create "plain"
		//   Variable_input_composites
		Variable_input_composite();
		// destructor.  Virtual for proper destruction of derived classes
		virtual ~Variable_input_composite();
#else // defined (GENERALIZE_COMPOSITE_INPUT)
	public:
		// constructor
		Variable_input_composite(const Variable_input_handle& input_1,
			const Variable_input_handle& input_2);
		Variable_input_composite(std::list<Variable_input_handle>& inputs_list);
#endif // defined (GENERALIZE_COMPOSITE_INPUT)
	public:
#if defined (USE_ITERATORS)
		virtual bool is_atomic();
#if defined (USE_ITERATORS_NESTED)
		virtual Iterator begin_atomic_inputs();
		virtual Iterator end_atomic_inputs();
#else // defined (USE_ITERATORS_NESTED)
#if defined (DO_NOT_USE_ITERATOR_TEMPLATES)
		virtual Variable_input_iterator begin_atomic_inputs();
		virtual Variable_input_iterator end_atomic_inputs();
#else // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
		virtual Handle_iterator<Variable_input_handle> begin_atomic_inputs();
		virtual Handle_iterator<Variable_input_handle> end_atomic_inputs();
#endif // defined (DO_NOT_USE_ITERATOR_TEMPLATES)
#endif // defined (USE_ITERATORS_NESTED)
		virtual Variable_size_type number_differentiable();
		virtual Variable_input_handle clone() const;
#else // defined (USE_ITERATORS)
		virtual Variable_size_type size();
#endif // defined (USE_ITERATORS)
#if defined (GENERALIZE_COMPOSITE_INPUT)
		virtual Variable_handle evaluate_derivative(
			Variable_handle dependent_variable,
			std::list<Variable_input_handle>& independent_variables,
			std::list<Variable_input_handle>::iterator&
			composite_independent_variable,
			std::list<Variable_input_value_handle>& values)=0;
		virtual Variable_handle get_input_value(Variable_handle variable)=0;
#else // defined (GENERALIZE_COMPOSITE_INPUT)
		// assignment
		Variable_input_composite& operator=(const Variable_input_composite&);
		// define inherited virtual methods
		virtual bool operator==(const Variable_input&);
		// to access composite input list iterators
		std::list<Variable_input_handle>::iterator begin();
		std::list<Variable_input_handle>::iterator end();
#endif // defined (GENERALIZE_COMPOSITE_INPUT)
#if defined (USE_SCALAR_MAPPING)
		virtual std::list< std::pair<Variable_size_type,Variable_size_type> >
			scalar_mapping_target(Variable_input_handle source)=0;
#endif // defined (USE_SCALAR_MAPPING)
#if defined (VARIABLE_INPUT_METHODS_FOR_SET_OPERATIONS)
	private:
		virtual std::list< std::pair<Variable_size_type,Variable_size_type> >
			scalar_mapping_local(const Variable_input_handle& target);
		virtual Variable_input_handle operator_plus_local(
			const Variable_input_handle& second);
		virtual Variable_input_handle operator_minus_local(
			const Variable_input_handle& second);
		virtual Variable_input_handle intersect_local(
			const Variable_input_handle& second);
#endif // defined (VARIABLE_INPUT_METHODS_FOR_SET_OPERATIONS)
#if defined (GENERALIZE_COMPOSITE_INPUT)
#else // defined (GENERALIZE_COMPOSITE_INPUT)
	private:
		std::list<Variable_input_handle> inputs_list;
#endif // defined (GENERALIZE_COMPOSITE_INPUT)
};

#if defined (USE_INTRUSIVE_SMART_POINTER)
typedef boost::intrusive_ptr<Variable_input_composite>
	Variable_input_composite_handle;
#elif defined (USE_SMART_POINTER)
typedef boost::shared_ptr<Variable_input_composite>
	Variable_input_composite_handle;
#else
typedef Variable_input_composite * Variable_input_composite_handle;
#endif

#endif /* !defined (__VARIABLE_INPUT_COMPOSITE_HPP__) */
