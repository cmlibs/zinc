//******************************************************************************
// FILE : function_inverse.hpp
//
// LAST MODIFIED : 25 January 2005
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
#if !defined (__FUNCTION_INVERSE_HPP__)
#define __FUNCTION_INVERSE_HPP__

#include "computed_variable/function.hpp"

class Function_inverse;

typedef boost::intrusive_ptr<Function_inverse> Function_inverse_handle;

class Function_inverse : public Function
//******************************************************************************
// LAST MODIFIED : 25 January 2005
//
// DESCRIPTION :
// An inverse of another function.  Evaluates using a Newton-Raphson iteration
// Evaluates derivative by calculating the derivative of independent_variable
// and "multiplying" by the inverse of the derivative, of the same order, of
// independent_variable with respect to dependent_variable.  Note that the
// derivative of independent_variable with respect to independent_variable is
// the identity and the second derivative of independent_variable with respect
// to independent_variable is zero.
//==============================================================================
{
	friend class Function_derivatnew_inverse;
	friend class Function_variable_independent;
	friend class Function_variable_iterator_representation_atomic_independent;
	friend class Function_variable_dependent;
	friend class Function_variable_dependent_estimate;
	template<class Value_type_1,class Value_type_2>
		friend bool equivalent(boost::intrusive_ptr<Value_type_1> const &,
		boost::intrusive_ptr<Value_type_2> const &);
	public:
		// constructor
		Function_inverse(const Function_variable_handle & dependent_variable,
			Function_variable_handle& independent_variable);
		// destructor
		~Function_inverse();
	// inherited
	public:
		string_handle get_string_representation();
		Function_variable_handle input();
		Function_variable_handle output();
	// additional
	public:
		Function_variable_handle
			independent(),
				//???DB.  Extend to components?
			step_tolerance(),
			value_tolerance(),
			maximum_iterations(),
			dependent_estimate();
		Scalar step_tolerance_value();
		Scalar value_tolerance_value();
		Function_size_type maximum_iterations_value();
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
		Function_inverse(const Function_inverse&);
		// assignment
		Function_inverse& operator=(const Function_inverse&);
		// equality
		bool operator==(const Function&) const;
	private:
		Function_size_type maximum_iterations_private;
		Function_variable_handle dependent_variable,independent_variable;
		Scalar step_tolerance_private,value_tolerance_private;
};

#endif /* !defined (__FUNCTION_INVERSE_HPP__) */
