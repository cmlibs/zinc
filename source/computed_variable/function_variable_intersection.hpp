//******************************************************************************
// FILE : function_variable_intersection.hpp
//
// LAST MODIFIED : 19 March 2004
//
// DESCRIPTION :
// A intersection of variables.  The list of atomic specifiers [begin_atomic(),
// end_atomic()) will have no repeats.
//==============================================================================
#if !defined (__FUNCTION_VARIABLE_INTERSECTION_HPP__)
#define __FUNCTION_VARIABLE_INTERSECTION_HPP__

#include <list>

#include "computed_variable/function_variable.hpp"

class Function_variable_intersection : public Function_variable
//******************************************************************************
// LAST MODIFIED : 19 March 2004
//
// DESCRIPTION :
// A intersection of other variable(s).
//==============================================================================
{
	friend class Function_variable_iterator_representation_atomic_intersection;
	public:
		// constructor
		Function_variable_intersection(const Function_variable_handle& variable_1,
			const Function_variable_handle& variable_2);
		Function_variable_intersection(const Function_handle& function,
			const Function_variable_handle& variable_1,
			const Function_variable_handle& variable_2);
		Function_variable_intersection(
			std::list<Function_variable_handle>& variables_list);
		Function_variable_intersection(const Function_handle& function,
			std::list<Function_variable_handle>& variables_list);
	// inherited
	public:
		Function_variable_handle clone() const;
		string_handle get_string_representation();
		Function_variable_iterator begin_atomic() const;
		Function_variable_iterator end_atomic() const;
		std::reverse_iterator<Function_variable_iterator> rbegin_atomic() const;
		std::reverse_iterator<Function_variable_iterator> rend_atomic() const;
	private:
		virtual bool equality_atomic(const Function_variable_handle&) const;
	private:
		// copy constructor
		Function_variable_intersection(const Function_variable_intersection&);
		// assignment
		Function_variable_intersection& operator=(
			const Function_variable_intersection&);
	private:
		std::list<Function_variable_handle> variables_list;
};

typedef boost::intrusive_ptr<Function_variable_intersection>
	Function_variable_intersection_handle;

#endif /* !defined (__FUNCTION_VARIABLE_INTERSECTION_HPP__) */
