//******************************************************************************
// FILE : function_matrix_dot_product.hpp
//
// LAST MODIFIED : 12 April 2005
//
// DESCRIPTION :
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
#if !defined (__FUNCTION_MATRIX_DOT_PRODUCT_HPP__)
#define __FUNCTION_MATRIX_DOT_PRODUCT_HPP__

#include <list>
#include <utility>
#include "computed_variable/function_matrix.hpp"
#include "computed_variable/function_variable_matrix.hpp"

template<typename Value_type>
class Function_derivatnew_matrix_dot_product;

template<typename Value_type>
class Function_variable_matrix_dot_product;

EXPORT template<typename Value_type>
class Function_matrix_dot_product : public Function_matrix<Value_type>
//******************************************************************************
// LAST MODIFIED : 12 April 2005
//
// DESCRIPTION :
// Output is the dot product of two matrix variables (sum of products of
// corresponding entries).  Input is the union of the matrix variables' inputs.
//==============================================================================
{
	template<class Value_type_1,class Value_type_2>
		friend bool equivalent(boost::intrusive_ptr<Value_type_1> const &,
		boost::intrusive_ptr<Value_type_2> const &);
	friend class Function_derivatnew_matrix_dot_product<Value_type>;
	friend class Function_variable_matrix_dot_product<Value_type>;
	public:
		// for construction exception
		class Invalid_argument {};
		// constructor
		Function_matrix_dot_product(const Function_variable_handle& variable_1,
			const Function_variable_handle& variable_2);
		// destructor
		~Function_matrix_dot_product();
	// inherited
	public:
		string_handle get_string_representation();
		Function_variable_handle input();
		Function_variable_handle output();
	private:
#if defined (EVALUATE_RETURNS_VALUE)
		Function_handle evaluate(Function_variable_handle atomic_variable);
#else // defined (EVALUATE_RETURNS_VALUE)
		bool evaluate(Function_variable_handle atomic_variable);
#endif // defined (EVALUATE_RETURNS_VALUE)
		bool evaluate_derivative(Scalar& derivative,
			Function_variable_handle atomic_variable,
			std::list<Function_variable_handle>& atomic_independent_variables);
		bool set_value(Function_variable_handle atomic_variable,
			Function_variable_handle atomic_value);
		Function_handle get_value(Function_variable_handle atomic_variable);
	private:
		// copy constructor
		Function_matrix_dot_product(const Function_matrix_dot_product&);
		// assignment
		Function_matrix_dot_product& operator=(const Function_matrix_dot_product&);
		// equality
		bool operator==(const Function&) const;
	private:
		Function_variable_handle variable_1_private,variable_2_private;
		static ublas::matrix<Value_type,ublas::column_major> constructor_values;
};

#if !defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)
#include "computed_variable/function_matrix_dot_product_implementation.cpp"
#endif // !defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)

#endif /* !defined (__FUNCTION_MATRIX_DOT_PRODUCT_HPP__) */
