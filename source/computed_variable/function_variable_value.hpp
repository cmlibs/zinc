//******************************************************************************
// FILE : function_variable_value.hpp
//
// LAST MODIFIED : 21 July 2004
//
// DESCRIPTION :
// An abstract class for accessing the value of variable.  A mediator which
// allows a variable to set its value from another variable only having to know
// about its value type.
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
#if !defined (__FUNCTION_VARIABLE_VALUE_HPP__)
#define __FUNCTION_VARIABLE_VALUE_HPP__

#include <string>

#include "computed_variable/function_base.hpp"

class Function_variable_value
//******************************************************************************
// LAST MODIFIED : 20 February 2004
//
// DESCRIPTION :
// A variable's value type.
//==============================================================================
{
	public:
		// destructor
		virtual ~Function_variable_value();
		virtual const std::string type()=0;
	protected:
		// constructor
		Function_variable_value();
	private:
		// copy operations are private and undefined to prevent copying
		Function_variable_value(const Function_variable_value&);
		void operator=(const Function_variable_value&);
	private:
		int reference_count;
		friend void intrusive_ptr_add_ref(Function_variable_value *);
		friend void intrusive_ptr_release(Function_variable_value *);
};

typedef boost::intrusive_ptr<Function_variable_value>
	Function_variable_value_handle;

#endif /* !defined (__FUNCTION_VARIABLE_VALUE_HPP__) */
