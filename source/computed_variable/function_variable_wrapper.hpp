//******************************************************************************
// FILE : function_variable_wrapper.hpp
//
// LAST MODIFIED : 10 May 2005
//
// DESCRIPTION :
// A variable that is a wrapper for another variable eg. the input/output
// variable for Function_identity.
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
#if !defined (__FUNCTION_VARIABLE_WRAPPER_HPP__)
#define __FUNCTION_VARIABLE_WRAPPER_HPP__

#include "computed_variable/function_variable.hpp"

class Function_variable_wrapper;

typedef boost::intrusive_ptr<Function_variable_wrapper>
	Function_variable_wrapper_handle;

class Function_variable_wrapper : public Function_variable
//******************************************************************************
// LAST MODIFIED : 10 May 2005
//
// DESCRIPTION :
// An identifier for another variable.
//==============================================================================
{
	friend class Function_derivatnew_wrapper;
	friend class Function_variable_iterator_representation_atomic_wrapper;
	template<class Value_type_1,class Value_type_2>
		friend bool equivalent(boost::intrusive_ptr<Value_type_1> const &,
		boost::intrusive_ptr<Value_type_2> const &);
	public:
		// constructors
		Function_variable_wrapper(const Function_handle& wrapping_function,
			const Function_variable_handle& wrapped_variable);
		// destructor.  Virtual for proper destruction of derived classes
		virtual ~Function_variable_wrapper();
	// inherited
	public:
		virtual Function_variable_handle clone() const;
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		// derivative creates a new Function which calculates the value of this
		//   variable differentiated with respect to the <independent_variables>
		virtual Function_handle derivative(
			const std::list<Function_variable_handle>& independent_variables);
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		virtual Function_variable_value_handle value();
		virtual bool set_value(Function_handle value);
		virtual bool rset_value(Function_handle value);
		virtual Function_handle get_value() const;
		virtual string_handle get_string_representation();
		virtual Function_variable_iterator begin_atomic() const;
		virtual Function_variable_iterator end_atomic() const;
		virtual std::reverse_iterator<Function_variable_iterator> rbegin_atomic()
			const;
		virtual std::reverse_iterator<Function_variable_iterator> rend_atomic()
			const;
		virtual Function_size_type number_differentiable();
		virtual Function_variable_handle operator-(const Function_variable&) const;
	// additional
	public:
		// get the wrapped variable
		virtual Function_variable_handle get_wrapped() const;
	private:
		virtual bool equality_atomic(const Function_variable_handle&) const;
	protected:
		// copy constructor
		Function_variable_wrapper(const Function_variable_wrapper&);
	protected:
		Function_variable_handle working_variable;
};

#endif /* !defined (__FUNCTION_VARIABLE_WRAPPER_HPP__) */
