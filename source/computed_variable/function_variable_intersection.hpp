//******************************************************************************
// FILE : function_variable_intersection.hpp
//
// LAST MODIFIED : 9 May 2005
//
// DESCRIPTION :
// A intersection of variables.  The list of atomic specifiers [begin_atomic(),
// end_atomic()) will have no repeats.
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
#if !defined (__FUNCTION_VARIABLE_INTERSECTION_HPP__)
#define __FUNCTION_VARIABLE_INTERSECTION_HPP__

#include <list>

#include "computed_variable/function_variable.hpp"

class Function_variable_intersection : public Function_variable
//******************************************************************************
// LAST MODIFIED : 9 May 2005
//
// DESCRIPTION :
// A intersection of other variable(s).
//==============================================================================
{
	friend class Function_derivatnew_intersection;
	friend class Function_variable_iterator_representation_atomic_intersection;
	template<class Value_type_1,class Value_type_2>
		friend bool equivalent(boost::intrusive_ptr<Value_type_1> const &,
		boost::intrusive_ptr<Value_type_2> const &);
	public:
		// constructor
		Function_variable_intersection(const Function_variable_handle& variable_1,
			const Function_variable_handle& variable_2);
		Function_variable_intersection(const Function_handle& function,
			const Function_variable_handle& variable_1,
			const Function_variable_handle& variable_2);
		Function_variable_intersection(
			std::list<Function_variable_handle>& variables_list);
		Function_variable_intersection(const Function_handle& function,
			std::list<Function_variable_handle>& variables_list);
	// inherited
	public:
		Function_variable_handle clone() const;
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		// derivative creates a new Function which calculates the value of this
		//   variable differentiated with respect to the <independent_variables>
		virtual Function_handle derivative(
			const std::list<Function_variable_handle>& independent_variables);
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		string_handle get_string_representation();
		Function_variable_iterator begin_atomic() const;
		Function_variable_iterator end_atomic() const;
		std::reverse_iterator<Function_variable_iterator> rbegin_atomic() const;
		std::reverse_iterator<Function_variable_iterator> rend_atomic() const;
#if defined (CIRCULAR_SMART_POINTERS)
		virtual void add_dependent_function(const Function_handle);
		virtual void remove_dependent_function(const Function_handle);
#else // defined (CIRCULAR_SMART_POINTERS)
		virtual void add_dependent_function(Function*);
		virtual void remove_dependent_function(Function*);
#endif // defined (CIRCULAR_SMART_POINTERS)
	private:
		virtual bool equality_atomic(const Function_variable_handle&) const;
	private:
		// copy constructor
		Function_variable_intersection(const Function_variable_intersection&);
		// assignment
		Function_variable_intersection& operator=(
			const Function_variable_intersection&);
	private:
		std::list<Function_variable_handle> variables_list;
};

typedef boost::intrusive_ptr<Function_variable_intersection>
	Function_variable_intersection_handle;

#endif /* !defined (__FUNCTION_VARIABLE_INTERSECTION_HPP__) */
