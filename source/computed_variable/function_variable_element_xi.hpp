//******************************************************************************
// FILE : function_variable_element_xi.hpp
//
// LAST MODIFIED : 2 July 2004
//
// DESCRIPTION :
// An element/xi variable.
//==============================================================================
#if !defined (__FUNCTION_VARIABLE_ELEMENT_XI_HPP__)
#define __FUNCTION_VARIABLE_ELEMENT_XI_HPP__

#include "computed_variable/function_variable.hpp"

class Function_variable_element_xi;

typedef boost::intrusive_ptr<Function_variable_element_xi>
	Function_variable_element_xi_handle;

class Function_variable_element_xi : public Function_variable
//******************************************************************************
// LAST MODIFIED : 2 July 2004
//
// DESCRIPTION :
// An identifier for an element/xi.  The xi index starts at 1.
//==============================================================================
{
	friend class Function_variable_iterator_representation_atomic_element_xi;
	// inherited
	public:
		string_handle get_string_representation();
		virtual Function_variable_iterator begin_atomic() const;
		virtual Function_variable_iterator end_atomic() const;
		std::reverse_iterator<Function_variable_iterator> rbegin_atomic() const;
		std::reverse_iterator<Function_variable_iterator> rend_atomic() const;
		virtual Function_size_type number_differentiable();
	// additional
	public:
		virtual Function_size_type number_of_xi() const=0;
		//???DB.  For Function_variable_element_xi_set_scalar_function and
		//   Function_variable_element_xi_set_element_function
		//???DB.  How does this relate to get_value?
		virtual bool get_element(struct FE_element*& element) const=0;
		virtual bool get_xi(Scalar& xi) const=0;
	private:
		virtual bool equality_atomic(const Function_variable_handle&) const;
	protected:
		// constructors.  Protected so that can't create "plain"
		//   Function_variable_element_xi's
		Function_variable_element_xi(const Function_handle& function,
			bool element=true,bool xi=true,
			const ublas::vector<Function_size_type>& indices=
			ublas::vector<Function_size_type>(0));
		Function_variable_element_xi(const Function_handle& function,
			Function_size_type index);
		Function_variable_element_xi(const Function_handle& function,
			const ublas::vector<Function_size_type>& indices);
		// copy constructor
		Function_variable_element_xi(
			const Function_variable_element_xi& variable_element_xi);
		// destructor.  Virtual for proper destruction of derived classes
		~Function_variable_element_xi();
	protected:
		bool element_private,xi_private;
		ublas::vector<Function_size_type> indices;
};

#endif // !defined (__FUNCTION_VARIABLE_ELEMENT_XI_HPP__)
