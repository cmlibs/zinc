//******************************************************************************
// FILE : variable_input_composite_union.hpp
//
// LAST MODIFIED : 28 January 2004
//
// DESCRIPTION :
//==============================================================================
#if !defined (__VARIABLE_INPUT_COMPOSITE_UNION_HPP__)
#define __VARIABLE_INPUT_COMPOSITE_UNION_HPP__

#include "computed_variable/variable_input_composite.hpp"

#if defined (GENERALIZE_COMPOSITE_INPUT)
class Variable_input_composite_union : public Variable_input_composite
//******************************************************************************
// LAST MODIFIED : 28 January 2004
//
// DESCRIPTION :
// A union of other input(s).  Unions do not have repeated inputs.
//
// ???DB.  What about destructor?
//==============================================================================
{
	public:
		// constructor
		Variable_input_composite_union(const Variable_input_handle& input_1,
			const Variable_input_handle& input_2);
		Variable_input_composite_union(
			std::list<Variable_input_handle>& inputs_list);
		// assignment
		Variable_input_composite_union& operator=(
			const Variable_input_composite_union&);
		// define inherited virtual methods
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
		virtual bool operator==(const Variable_input&);
		virtual Variable_handle evaluate_derivative(
			Variable_handle dependent_variable,
			std::list<Variable_input_handle>& independent_variables,
			std::list<Variable_input_handle>::iterator&
			composite_independent_variable,
			std::list<Variable_input_value_handle>& values);
		virtual Variable_handle get_input_value(Variable_handle variable);
#if defined (USE_SCALAR_MAPPING)
		virtual std::list< std::pair<Variable_size_type,Variable_size_type> >
			scalar_mapping_target(Variable_input_handle source);
#endif // defined (USE_SCALAR_MAPPING)
#if defined (USE_SCALAR_MAPPING)
	private:
		virtual std::list< std::pair<Variable_size_type,Variable_size_type> >
			scalar_mapping_local(Variable_input_handle target);
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
	private:
		std::list<Variable_input_handle> inputs_list;
};

#if defined (USE_INTRUSIVE_SMART_POINTER)
typedef boost::intrusive_ptr<Variable_input_composite_union>
	Variable_input_composite_union_handle;
#elif defined (USE_SMART_POINTER)
typedef boost::shared_ptr<Variable_input_composite_union>
	Variable_input_composite_union_handle;
#else
typedef Variable_input_composite_union * Variable_input_composite_union_handle;
#endif
#endif // defined (GENERALIZE_COMPOSITE_INPUT)

#endif /* !defined (__VARIABLE_INPUT_COMPOSITE_UNION_HPP__) */
