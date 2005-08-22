//******************************************************************************
// FILE : function_identity.hpp
//
// LAST MODIFIED : 11 May 2005
//
// DESCRIPTION :
// Used when calculating derivatives eg. composition and inverse.
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
#if !defined (__FUNCTION_IDENTITY_HPP__)
#define __FUNCTION_IDENTITY_HPP__

#include "computed_variable/function.hpp"

#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#include "computed_variable/function_derivative.hpp"
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

class Function_identity;

typedef boost::intrusive_ptr<Function_identity> Function_identity_handle;

class Function_identity : public Function
//******************************************************************************
// LAST MODIFIED : 14 January 2005
//
// DESCRIPTION :
// An identity function for a specified variable.  Used in calculating
// derivatives.
//==============================================================================
{
	friend class Function_variable_iterator_representation_atomic_identity;
	template<class Value_type_1,class Value_type_2>
		friend bool equivalent(boost::intrusive_ptr<Value_type_1> const &,
		boost::intrusive_ptr<Value_type_2> const &);
	public:
		// constructor
		Function_identity(const Function_variable_handle& variable);
		// destructor
		~Function_identity();
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
		Function_identity(const Function_identity&);
		// assignment
		Function_identity& operator=(const Function_identity&);
		// equality
		bool operator==(const Function&) const;
	private:
		Function_variable_handle variable_private;
};

#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
// class Function_derivatnew_identity
// ----------------------------------

class Function_derivatnew_identity : public Function_derivatnew
//******************************************************************************
// LAST MODIFIED : 15 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	public:
		// for construction exception
		class Construction_exception {};
		// constructor
		Function_derivatnew_identity(
			const Function_variable_handle& dependent_variable,
			const std::list<Function_variable_handle>& independent_variables);
		// destructor
		~Function_derivatnew_identity();
	// inherited
	private:
#if defined (EVALUATE_RETURNS_VALUE)
		virtual Function_handle evaluate(Function_variable_handle atomic_variable);
#else // defined (EVALUATE_RETURNS_VALUE)
		virtual bool evaluate(Function_variable_handle atomic_variable);
#endif // defined (EVALUATE_RETURNS_VALUE)
};
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

#endif // !defined (__FUNCTION_IDENTITY_HPP__)
