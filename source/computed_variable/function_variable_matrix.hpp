//******************************************************************************
// FILE : function_variable_matrix.hpp
//
// LAST MODIFIED : 21 July 2004
//
// DESCRIPTION :
// A variable that is a matrix.
//
//???DB.  Should be template?
//???DB.  Transpose?
//==============================================================================
#if !defined (__FUNCTION_VARIABLE_MATRIX_HPP__)
#define __FUNCTION_VARIABLE_MATRIX_HPP__

#include "computed_variable/function_variable.hpp"

EXPORT template<typename Value_type>
	class Function_variable_iterator_representation_atomic_matrix;

EXPORT template<typename Value_type>
class Function_variable_matrix : public Function_variable
//******************************************************************************
// LAST MODIFIED : 15 July 2004
//
// DESCRIPTION :
// An identifier for a matrix.
//
// <column> and <row> start from one when referencing a matrix entry.  Zero
// indicates all.
//==============================================================================
{
	friend class
		Function_variable_iterator_representation_atomic_matrix<Value_type>;
	// inherited
	public:
		virtual string_handle get_string_representation();
		virtual Function_variable_iterator begin_atomic() const;
		virtual Function_variable_iterator end_atomic() const;
		virtual std::reverse_iterator<Function_variable_iterator> rbegin_atomic()
			const;
		virtual std::reverse_iterator<Function_variable_iterator> rend_atomic()
			const;
		virtual Function_size_type number_differentiable();
	// additional
	public:
		// get a matrix entry variable
		virtual boost::intrusive_ptr< Function_variable_matrix<Value_type> >
			operator()(Function_size_type row=1,Function_size_type column=1)=0;
#if defined (TO_BE_DONE)
		virtual boost::intrusive_ptr< Function_variable_matrix<Value_type> >
			sub_matrix(Function_size_type row_low,Function_size_type row_high,
			Function_size_type column_low,Function_size_type column_high) const=0;
#endif // defined (TO_BE_DONE)
		virtual Function_size_type number_of_rows() const=0;
		virtual Function_size_type number_of_columns() const=0;
		//???DB.  For Function_variable_matrix_set_scalar_function
		//???DB.  How does this relate to get_value?
		virtual bool get_entry(Value_type& value) const=0;
	private:
		virtual bool equality_atomic(const Function_variable_handle&) const;
	protected:
		// constructors.  Protected so that can't create "plain"
		//   Function_variable_matrix's
		Function_variable_matrix(const Function_handle function);
		Function_variable_matrix(const Function_handle function,
			const Function_size_type row,const Function_size_type column);
		// copy constructor
		Function_variable_matrix(const Function_variable_matrix&);
		// destructor.  Virtual for proper destruction of derived classes
		virtual ~Function_variable_matrix();
	protected:
		Function_size_type column,row;
};

EXPORT template<typename Value_type>
bool Function_variable_matrix_set_value_function(Value_type& value,
	const Function_variable_handle variable);
//******************************************************************************
// LAST MODIFIED : 5 July 2004
//
// DESCRIPTION :
// Needed when a derived class needs its own
// class Function_variable_iterator_representation_atomic_*.
//==============================================================================

#if !defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)
#include "computed_variable/function_variable_matrix_implementation.cpp"
#endif // !defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)

#endif /* !defined (__FUNCTION_VARIABLE_MATRIX_HPP__) */
