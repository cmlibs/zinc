//******************************************************************************
// FILE : function_variable_composite.hpp
//
// LAST MODIFIED : 13 August 2004
//
// DESCRIPTION :
// A list of specifiers joined together end on end.  There can be repeats in the
// list of atomic specifiers [begin_atomic(),end_atomic()).
//==============================================================================
#if !defined (__FUNCTION_VARIABLE_COMPOSITE_HPP__)
#define __FUNCTION_VARIABLE_COMPOSITE_HPP__

#include <list>

#include "computed_variable/function_variable.hpp"

class Function_variable_composite : public Function_variable
//******************************************************************************
// LAST MODIFIED : 13 August 2004
//
// DESCRIPTION :
// A composite of other variable(s).
//
#if defined (COMPOSITE_FLATTENING)
// Composite variables are "flat" in the sense that the list of
// variables does not contain composite variables.  This means that the
// constructors have to flatten the list.
//???DB.  Not sure if this "flattening" is useful
#endif // defined (COMPOSITE_FLATTENING)
//==============================================================================
{
	friend class Function_variable_iterator_representation_atomic_composite;
	template<class Value_type_1,class Value_type_2>
		friend bool equivalent(boost::intrusive_ptr<Value_type_1> const &,
		boost::intrusive_ptr<Value_type_2> const &);
	public:
		// constructor
		Function_variable_composite(const Function_variable_handle& variable_1,
			const Function_variable_handle& variable_2);
		Function_variable_composite(const Function_handle& function,
			const Function_variable_handle& variable_1,
			const Function_variable_handle& variable_2);
		Function_variable_composite(
			std::list<Function_variable_handle>& variables_list);
		Function_variable_composite(const Function_handle& function,
			std::list<Function_variable_handle>& variables_list);
	// inherited
	public:
		Function_variable_handle clone() const;
		string_handle get_string_representation();
		Function_variable_iterator begin_atomic() const;
		Function_variable_iterator end_atomic() const;
		std::reverse_iterator<Function_variable_iterator> rbegin_atomic() const;
		std::reverse_iterator<Function_variable_iterator> rend_atomic() const;
		Function_size_type number_differentiable();
	private:
		virtual bool equality_atomic(const Function_variable_handle&) const;
	private:
		// copy constructor
		Function_variable_composite(const Function_variable_composite&);
		// assignment
		Function_variable_composite& operator=(const Function_variable_composite&);
	private:
		std::list<Function_variable_handle> variables_list;
};

typedef boost::intrusive_ptr<Function_variable_composite>
	Function_variable_composite_handle;

#endif /* !defined (__FUNCTION_VARIABLE_COMPOSITE_HPP__) */
