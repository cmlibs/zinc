//******************************************************************************
// FILE : variable_input_composite.hpp
//
// LAST MODIFIED : 24 November 2003
//
// DESCRIPTION :
//==============================================================================
#if !defined (__VARIABLE_INPUT_COMPOSITE_HPP__)
#define __VARIABLE_INPUT_COMPOSITE_HPP__

#include <list>

#include "computed_variable/variable_input.hpp"

class Variable_input_composite : public Variable_input
//******************************************************************************
// LAST MODIFIED : 24 November 2003
//
// DESCRIPTION :
// A composite of other input(s).
//
// Composite inputs are "flat" in the sense that there list of inputs does
// not contain composite inputs.  This means that the constructors have to
// flatten the list.
//==============================================================================
{
	public:
		// constructor
		Variable_input_composite(Variable_input_handle& input_1,
			Variable_input_handle& input_2);
		Variable_input_composite(std::list<Variable_input_handle>& inputs_list);
		// assignment
		Variable_input_composite& operator=(const Variable_input_composite&);
		// get the number of reals specified
			// ???DB.  What if can't determine or has non-real(s)?
		Variable_size_type size();
		virtual bool operator==(const Variable_input&);
		// to access composite input list iterators
		std::list<Variable_input_handle>::iterator begin();
		std::list<Variable_input_handle>::iterator end();
	private:
		std::list<Variable_input_handle> inputs_list;
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
