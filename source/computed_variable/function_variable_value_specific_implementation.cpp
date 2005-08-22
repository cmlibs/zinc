//******************************************************************************
// FILE : function_variable_value_specific_implementation.cpp
//
// LAST MODIFIED : 18 August 2004
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
//
#include "computed_variable/function_variable_wrapper.hpp"

// global classes
// ==============

// template class Function_variable_value_specific
// ===============================================

EXPORT template<typename Value_type>
Function_variable_value_specific<Value_type>::
	Function_variable_value_specific(
	bool (*set_function)(Value_type&,const Function_variable_handle)):
	set_function(set_function){}

EXPORT template<typename Value_type>
Function_variable_value_specific<Value_type>::
	~Function_variable_value_specific<Value_type>(){}

EXPORT template<typename Value_type>
const std::string Function_variable_value_specific<Value_type>::type()
{
	return (type_string);
}

EXPORT template<typename Value_type>
bool Function_variable_value_specific<Value_type>::set(Value_type& value,
	const Function_variable_handle variable)
{
	bool result;

	result=false;
	if (set_function)
	{
		Function_variable_handle wrapped_variable;
		Function_variable_wrapper_handle temp_variable;

		wrapped_variable=variable;
		while (temp_variable=boost::dynamic_pointer_cast<Function_variable_wrapper,
			Function_variable>(wrapped_variable))
		{
			wrapped_variable=temp_variable->get_wrapped();
		}
		result=(*set_function)(value,wrapped_variable);
	}

	return (result);
}
