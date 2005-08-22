//******************************************************************************
// FILE : function_derivative_matrix.hpp
//
// LAST MODIFIED : 22 February 2005
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
#if !defined (__FUNCTION_DERIVATIVE_MATRIX_HPP__)
#define __FUNCTION_DERIVATIVE_MATRIX_HPP__

#include <list>

#include "computed_variable/function.hpp"
#include "computed_variable/function_variable.hpp"

#define SEPARATE_ATOMIC_AND_MATRIX_SPLITTING

class Function_derivative_matrix;

typedef boost::intrusive_ptr<Function_derivative_matrix>
	Function_derivative_matrix_handle;

class Function_derivative_matrix : public Function
//******************************************************************************
// LAST MODIFIED : 21 February 2005
//
// DESCRIPTION :
//==============================================================================
{
	friend class Function_variable_matrix_derivative_matrix;
	friend class Function_variable_dependent;
		//???DB.  So that can access matrices.back().
		//???DB.    Not good.  Have a value method?
#if defined (OLD_CODE)
	friend class Function_variable_composition;
	friend Function_handle Function_variable::evaluate_derivative(
		std::list<Function_variable_handle>& independent_variables);
#endif // defined (OLD_CODE)
	template<class Value_type_1,class Value_type_2>
		friend bool equivalent(boost::intrusive_ptr<Value_type_1> const &,
		boost::intrusive_ptr<Value_type_2> const &);
	public:
		// for construction exception
		class Construction_exception {};
	// inherited
	public:
		string_handle get_string_representation();
		Function_variable_handle input();
		Function_variable_handle output();
	// additional
	public:
		// get the specified partial derivative
		Function_handle matrix(
			std::list<Function_variable_handle>& partial_independent_variables);
		// implement the chain rule for differentiation
		friend Function_derivative_matrix_handle
			Function_derivative_matrix_compose(
			const Function_variable_handle& dependent_variable,
			const Function_derivative_matrix_handle& derivative_f,
			const Function_derivative_matrix_handle& derivative_g);
		// calculate the composition inverse
		Function_handle inverse();
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
#if defined (OLD_CODE)
	private:
#else // defined (OLD_CODE)
	public:
#endif // defined (OLD_CODE)
		Function_derivative_matrix(
			const Function_variable_handle& dependent_variable,
			const std::list<Function_variable_handle>& independent_variables,
			const std::list<Matrix>& matrices);
		// calls (dependent_variable->function())->evaluate_derivative to fill in
		//   the matrices.  Used by Function_variable::evaluate_derivative
		Function_derivative_matrix(
			const Function_variable_handle& dependent_variable,
			const std::list<Function_variable_handle>& independent_variables);
	private:
		// copy constructor
		Function_derivative_matrix(const Function_derivative_matrix&);
		// assignment
		Function_derivative_matrix& operator=(const Function_derivative_matrix&);
		// destructor
		virtual ~Function_derivative_matrix();
		// equality
		bool operator==(const Function&) const;
	private:
		//???DB.  Should be Function_variable rather Function_variable_handle
		//  because when Function_variable changes derivative matrix doesn't
		//  automatically change?
		Function_variable_handle dependent_variable;
		std::list<Function_variable_handle> independent_variables;
		std::list<Matrix> matrices;
};

#endif /* !defined (__FUNCTION_DERIVATIVE_MATRIX_HPP__) */
