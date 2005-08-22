//******************************************************************************
// FILE : function_variable_matrix.hpp
//
// LAST MODIFIED : 27 June 2005
//
// DESCRIPTION :
// A variable that is a matrix.
//
//???DB.  Transpose?
//==============================================================================
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
#if !defined (__FUNCTION_VARIABLE_MATRIX_HPP__)
#define __FUNCTION_VARIABLE_MATRIX_HPP__

#include "computed_variable/function_variable.hpp"

//???DB.  Debugging
//#define Function_variable_matrix_HAS_INPUT_ATTRIBUTE

EXPORT template<typename Value_type>
	class Function_variable_iterator_representation_atomic_matrix;

EXPORT template<typename Value_type>
class Function_variable_matrix : public Function_variable
//******************************************************************************
// LAST MODIFIED : 27 June 2005
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
		Function_variable_matrix(const Function_handle function
#if defined (Function_variable_matrix_HAS_INPUT_ATTRIBUTE)
			,const bool input
#endif // defined (Function_variable_matrix_HAS_INPUT_ATTRIBUTE)
			);
		Function_variable_matrix(const Function_handle function,
#if defined (Function_variable_matrix_HAS_INPUT_ATTRIBUTE)
			const bool input,
#endif // defined (Function_variable_matrix_HAS_INPUT_ATTRIBUTE)
			const Function_size_type row,const Function_size_type column);
	// inherited
	public:
		virtual Function_variable_handle clone() const;
		virtual string_handle get_string_representation();
#if defined (EVALUATE_RETURNS_VALUE)
		virtual Function_handle evaluate();
#else // defined (EVALUATE_RETURNS_VALUE)
		virtual bool evaluate();
#endif // defined (EVALUATE_RETURNS_VALUE)
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		virtual Function_handle derivative(
			const std::list<Function_variable_handle>&);
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		virtual Function_handle get_value() const;
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
#if defined (Function_variable_matrix_HAS_INPUT_ATTRIBUTE)
		// if true then input variable otherwise output variable
		bool input_private;
#endif // defined (Function_variable_matrix_HAS_INPUT_ATTRIBUTE)
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
