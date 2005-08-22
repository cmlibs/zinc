//******************************************************************************
// FILE : function_gradient.hpp
//
// LAST MODIFIED : 21 October 2004
//
// DESCRIPTION :
// ***Not used***
// API now uses a combination of Function_matrix_transpose and
// Function_derivative.
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
#if !defined (__FUNCTION_GRADIENT_HPP__)
#define __FUNCTION_GRADIENT_HPP__

#include <list>
#include "computed_variable/function.hpp"

class Function_gradient;

typedef boost::intrusive_ptr<Function_gradient> Function_gradient_handle;

class Function_gradient : public Function
//******************************************************************************
// LAST MODIFIED : 25 August 2004
//
// DESCRIPTION :
// A gradient of another function.
//
// No storage for result and so can't be inverted.
// ???DB.  Could compose with a Function_matrix and then would be invertable?
//==============================================================================
{
	friend class Function_variable_gradient;
	friend class Function_variable_iterator_representation_atomic_gradient;
	template<class Value_type_1,class Value_type_2>
		friend bool equivalent(boost::intrusive_ptr<Value_type_1> const &,
		boost::intrusive_ptr<Value_type_2> const &);
	public:
		// constructor
		Function_gradient(const Function_variable_handle& dependent_variable,
			const Function_variable_handle& independent_variable);
		// destructor
		~Function_gradient();
	// inherited
	public:
		string_handle get_string_representation();
		Function_variable_handle input();
		Function_variable_handle output();
	private:
		Function_handle evaluate(Function_variable_handle atomic_variable);
		bool evaluate_derivative(Scalar& derivative,
			Function_variable_handle atomic_variable,
			std::list<Function_variable_handle>& atomic_independent_variables);
		bool set_value(Function_variable_handle atomic_variable,
			Function_variable_handle atomic_value);
		Function_handle get_value(Function_variable_handle atomic_variable);
	private:
		// copy constructor
		Function_gradient(const Function_gradient&);
		// assignment
		Function_gradient& operator=(const Function_gradient&);
		// equality
		bool operator==(const Function&) const;
	private:
		Function_variable_handle dependent_variable_private;
		Function_variable_handle independent_variable_private;
};

#endif /* !defined (__FUNCTION_GRADIENT_HPP__) */
