/*******************************************************************************
FILE : api/cmiss_variable.h

LAST MODIFIED : 30 July 2003

DESCRIPTION :
The public interface to the Cmiss_variable object.
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
#ifndef __API_CMISS_VARIABLE_H__
#define __API_CMISS_VARIABLE_H__

#include "api/cmiss_value.h"
/* If this is going to be in the API then it needs to have an interface there */
#include "general/object.h"
#include "general/list.h"

/*
Global types
------------
*/

typedef struct Cmiss_variable *Cmiss_variable_id;
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
An object that can be evaluated from and differentiated with respect to other
variables.  It can be displayed, minimized or used in an equation.
==============================================================================*/

struct Cmiss_variable_value;
/*******************************************************************************
LAST MODIFIED : 22 January 2003

DESCRIPTION :
A variable/value pair for specifying the value of the variable.
==============================================================================*/

DECLARE_LIST_TYPES(Cmiss_variable_value);

/*
Global functions
----------------
*/

/* SAB Temporarily mangle the external name until we decide to mangle the 
	internal one instead */
Cmiss_variable_id CREATE(Cmiss_variable_API)(char *name);
/*******************************************************************************
LAST MODIFIED : 30 July 2003

DESCRIPTION :
Creates a Cmiss_variable with the supplied <name>.
==============================================================================*/

struct Cmiss_variable_value *CREATE(Cmiss_variable_value)(
	Cmiss_variable_id variable,Cmiss_value_id value);
/*******************************************************************************
LAST MODIFIED : 22 January 2003

DESCRIPTION :
Creates a <variable>/<value> pair.
==============================================================================*/

PROTOTYPE_OBJECT_FUNCTIONS(Cmiss_variable_value);

PROTOTYPE_LIST_FUNCTIONS(Cmiss_variable_value);

int Cmiss_variable_evaluate(struct Cmiss_variable_value *variable_value,
	struct LIST(Cmiss_variable_value) *values);
/*******************************************************************************
LAST MODIFIED : 4 February 2003

DESCRIPTION :
Calculates the <variable_value> with the specified <values> over-riding, but not
setting, the current values.
==============================================================================*/

int Cmiss_variable_evaluate_derivative(
	Cmiss_variable_id dependent_variable,int order,
	Cmiss_variable_id *independent_variables,
	struct LIST(Cmiss_variable_value) *values,
	Cmiss_value_id derivative_matrix);
/*******************************************************************************
LAST MODIFIED : 7 March 2003

DESCRIPTION :
Evaluates the <derivative_matrix> for the <order> degree derivative of
<dependent_variable> with respect to the <independent_variables>.
==============================================================================*/
#endif /* __API_CMISS_VARIABLE_H__ */
