/*******************************************************************************
FILE : api/cmiss_function_variable_composite.cpp

LAST MODIFIED : 17 March 2004

DESCRIPTION :
The public interface to the Cmiss_function_variable composite object.
==============================================================================*/
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

#include <new>
#include "api/cmiss_function_variable_composite.h"
#include "computed_variable/function_variable_composite.hpp"

/*
Global functions
----------------
*/
Cmiss_function_variable_id Cmiss_function_variable_composite_create(
	Cmiss_function_variable_list_id variables)
/*******************************************************************************
LAST MODIFIED : 17 March 2004

DESCRIPTION :
Creates a Cmiss_function_variable composite with the supplied <variables>.
==============================================================================*/
{
	Cmiss_function_variable_id result;
	std::list<Function_variable_handle> *variables_address;

	result=0;
	if (variables_address=reinterpret_cast<std::list<Function_variable_handle> *>(
    variables))
	{
		result=reinterpret_cast<Cmiss_function_variable_id>(
			new Function_variable_composite_handle(new Function_variable_composite(
			*variables_address)));
	}

	return (result);
}
