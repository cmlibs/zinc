//******************************************************************************
// FILE : function_variable_matrix.hpp
//
// LAST MODIFIED : 2 September 2004
//
// DESCRIPTION :
// A variable that is a matrix.
//
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
// LAST MODIFIED : 2 September 2004
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
	template<class Value_type_1,class Value_type_2>
		friend bool equivalent(boost::intrusive_ptr<Value_type_1> const &,
		boost::intrusive_ptr<Value_type_2> const &);
	public:
		Function_variable_matrix(const Function_handle function);
		Function_variable_matrix(const Function_handle function,
			const Function_size_type row,const Function_size_type column);
	// inherited
	public:
		virtual Function_variable_handle clone() const;
		virtual string_handle get_string_representation();
		virtual Function_handle evaluate();
		virtual Function_handle get_value();
		virtual Function_variable_iterator begin_atomic() const;
		virtual Function_variable_iterator end_atomic() const;
		virtual std::reverse_iterator<Function_variable_iterator> rbegin_atomic()
			const;
		virtual std::reverse_iterator<Function_variable_iterator> rend_atomic()
			const;
		virtual Function_size_type number_differentiable();
		virtual Scalar norm() const;
		virtual Function_variable_handle operator-(const Function_variable&) const;
	// additional
	public:
		// get a matrix entry variable
		virtual boost::intrusive_ptr< Function_variable_matrix<Value_type> >
			operator()(Function_size_type row=1,Function_size_type column=1) const;
#if defined (TO_BE_DONE)
		virtual boost::intrusive_ptr< Function_variable_matrix<Value_type> >
			sub_matrix(Function_size_type row_low,Function_size_type row_high,
			Function_size_type column_low,Function_size_type column_high) const=0;
#endif // defined (TO_BE_DONE)
		virtual Function_size_type number_of_rows() const;
		virtual Function_size_type number_of_columns() const;
		virtual Function_size_type row() const;
		virtual Function_size_type column() const;
		//???DB.  For Function_variable_matrix_set_scalar_function
		//???DB.  How does this relate to get_value?
		virtual bool get_entry(Value_type& value) const;
	private:
		virtual bool equality_atomic(const Function_variable_handle&) const;
	protected:
		// constructors.  Protected so that can't create "plain"
		//   Function_variable_matrix's
		// copy constructor
		Function_variable_matrix(const Function_variable_matrix&);
		// destructor.  Virtual for proper destruction of derived classes
		virtual ~Function_variable_matrix();
	protected:
		Function_size_type column_private,row_private;
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
