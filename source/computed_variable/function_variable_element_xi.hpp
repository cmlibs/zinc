//******************************************************************************
// FILE : function_variable_element_xi.hpp
//
// LAST MODIFIED : 13 August 2004
//
// DESCRIPTION :
// An element/xi variable.
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
#if !defined (__FUNCTION_VARIABLE_ELEMENT_XI_HPP__)
#define __FUNCTION_VARIABLE_ELEMENT_XI_HPP__

#include "computed_variable/function_variable.hpp"

class Function_variable_element_xi;

typedef boost::intrusive_ptr<Function_variable_element_xi>
	Function_variable_element_xi_handle;

class Function_variable_element_xi : public Function_variable
//******************************************************************************
// LAST MODIFIED : 13 August 2004
//
// DESCRIPTION :
// An identifier for an element/xi.  The xi index starts at 1.
//==============================================================================
{
	friend class Function_variable_iterator_representation_atomic_element_xi;
	template<class Value_type_1,class Value_type_2>
		friend bool equivalent(boost::intrusive_ptr<Value_type_1> const &,
		boost::intrusive_ptr<Value_type_2> const &);
	// inherited
	public:
		string_handle get_string_representation();
		virtual Function_variable_iterator begin_atomic() const;
		virtual Function_variable_iterator end_atomic() const;
		std::reverse_iterator<Function_variable_iterator> rbegin_atomic() const;
		std::reverse_iterator<Function_variable_iterator> rend_atomic() const;
		virtual Function_size_type number_differentiable();
		virtual Function_variable_handle operator+=(const Function_variable&);
	// additional
	public:
		virtual Function_size_type number_of_xi() const=0;
		//???DB.  For Function_variable_element_xi_set_scalar_function and
		//   Function_variable_element_xi_set_element_function
		//???DB.  How does this relate to get_value?
		virtual bool get_element(struct FE_element*& element) const=0;
		virtual bool get_xi(Scalar& xi) const=0;
	private:
		virtual bool equality_atomic(const Function_variable_handle&) const;
	protected:
		// constructors.  Protected so that can't create "plain"
		//   Function_variable_element_xi's
		Function_variable_element_xi(const Function_handle& function,
			bool element=true,bool xi=true,
			const ublas::vector<Function_size_type>& indices=
			ublas::vector<Function_size_type>(0));
		Function_variable_element_xi(const Function_handle& function,
			Function_size_type index);
		Function_variable_element_xi(const Function_handle& function,
			const ublas::vector<Function_size_type>& indices);
		// copy constructor
		Function_variable_element_xi(
			const Function_variable_element_xi& variable_element_xi);
		// destructor.  Virtual for proper destruction of derived classes
		~Function_variable_element_xi();
	protected:
		bool element_private,xi_private;
		ublas::vector<Function_size_type> indices;
};

#endif // !defined (__FUNCTION_VARIABLE_ELEMENT_XI_HPP__)
