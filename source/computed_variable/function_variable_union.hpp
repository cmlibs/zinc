//******************************************************************************
// FILE : function_variable_union.hpp
//
// LAST MODIFIED : 7 December 2004
//
// DESCRIPTION :
// A union of variables.  The list of atomic specifiers [begin_atomic(),
// end_atomic()) will not repeat atomic variables if they appear in previous
// variables (will repeat if appear in current variable).
//==============================================================================
#if !defined (__FUNCTION_VARIABLE_UNION_HPP__)
#define __FUNCTION_VARIABLE_UNION_HPP__

#include <list>

#include "computed_variable/function_variable.hpp"

class Function_variable_union : public Function_variable
//******************************************************************************
// LAST MODIFIED : 7 December 2004
//
// DESCRIPTION :
// A union of other variable(s).
//==============================================================================
{
	friend class Function_variable_iterator_representation_atomic_union;
	template<class Value_type_1,class Value_type_2>
		friend bool equivalent(boost::intrusive_ptr<Value_type_1> const &,
		boost::intrusive_ptr<Value_type_2> const &);
	public:
		// constructor
		Function_variable_union(const Function_variable_handle& variable_1,
			const Function_variable_handle& variable_2);
		Function_variable_union(const Function_handle& function,
			const Function_variable_handle& variable_1,
			const Function_variable_handle& variable_2);
		Function_variable_union(
			std::list<Function_variable_handle>& variables_list);
		Function_variable_union(const Function_handle& function,
			std::list<Function_variable_handle>& variables_list);
	// inherited
	public:
		Function_variable_handle clone() const;
		string_handle get_string_representation();
		Function_variable_iterator begin_atomic() const;
		Function_variable_iterator end_atomic() const;
		std::reverse_iterator<Function_variable_iterator> rbegin_atomic() const;
		std::reverse_iterator<Function_variable_iterator> rend_atomic() const;
#if defined (CIRCULAR_SMART_POINTERS)
		virtual void add_dependent_function(const Function_handle);
		virtual void remove_dependent_function(const Function_handle);
#else // defined (CIRCULAR_SMART_POINTERS)
		virtual void add_dependent_function(Function*);
		virtual void remove_dependent_function(Function*);
#endif // defined (CIRCULAR_SMART_POINTERS)
	private:
		virtual bool equality_atomic(const Function_variable_handle&) const;
	private:
		// copy constructor
		Function_variable_union(const Function_variable_union&);
		// assignment
		Function_variable_union& operator=(const Function_variable_union&);
	private:
		std::list<Function_variable_handle> variables_list;
};

typedef boost::intrusive_ptr<Function_variable_union>
	Function_variable_union_handle;

#endif /* !defined (__FUNCTION_VARIABLE_UNION_HPP__) */
