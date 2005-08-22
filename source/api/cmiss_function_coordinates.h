/*******************************************************************************
FILE : api/cmiss_function_coordinates.h

LAST MODIFIED : 4 June 2004

DESCRIPTION :
The public interface to the Cmiss_function coordinates objects.
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
#ifndef __API_CMISS_FUNCTION_COORDINATES_H__
#define __API_CMISS_FUNCTION_COORDINATES_H__

#include "api/cmiss_function.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

Cmiss_function_id
	Cmiss_function_prolate_spheroidal_to_rectangular_cartesian_create(
	Scalar lambda,Scalar mu,Scalar theta,Scalar focus);
/*******************************************************************************
LAST MODIFIED : 4 June 2004

DESCRIPTION :
Creates a Cmiss_function prolate_spheroidal_to_rectangular_cartesian.
==============================================================================*/

Cmiss_function_variable_id
	Cmiss_function_prolate_spheroidal_to_rectangular_cartesian_component(
	Cmiss_function_id function_prolate_spheroidal_to_rectangular_cartesian,
	char *name,unsigned int number);
/*******************************************************************************
LAST MODIFIED : 4 June 2004

DESCRIPTION :
Returns a variable that refers to a component (x,y,z) of the
<function_prolate_spheroidal_to_rectangular_cartesian>.  If <name> is not NULL,
then the component with the <name> is specified.  If <name> is NULL, then the
component with the <number> is specified.  Component <number> 1 is the first
component.
==============================================================================*/

Cmiss_function_variable_id
	Cmiss_function_variable_prolate_spheroidal_to_rectangular_cartesian_prolate(
	Cmiss_function_id function_prolate_spheroidal_to_rectangular_cartesian);
/*******************************************************************************
LAST MODIFIED : 4 June 2004

DESCRIPTION :
Returns the prolate variable for the
<function_prolate_spheroidal_to_rectangular_cartesian>.
==============================================================================*/

Cmiss_function_variable_id
	Cmiss_function_variable_prolate_spheroidal_to_rectangular_cartesian_lambda(
	Cmiss_function_id function_prolate_spheroidal_to_rectangular_cartesian);
/*******************************************************************************
LAST MODIFIED : 4 June 2004

DESCRIPTION :
Returns the lambda variable for the
<function_prolate_spheroidal_to_rectangular_cartesian>.
==============================================================================*/

Cmiss_function_variable_id
	Cmiss_function_variable_prolate_spheroidal_to_rectangular_cartesian_mu(
	Cmiss_function_id function_prolate_spheroidal_to_rectangular_cartesian);
/*******************************************************************************
LAST MODIFIED : 4 June 2004

DESCRIPTION :
Returns the mu variable for the
<function_prolate_spheroidal_to_rectangular_cartesian>.
==============================================================================*/

Cmiss_function_variable_id
	Cmiss_function_variable_prolate_spheroidal_to_rectangular_cartesian_theta(
	Cmiss_function_id function_prolate_spheroidal_to_rectangular_cartesian);
/*******************************************************************************
LAST MODIFIED : 4 June 2004

DESCRIPTION :
Returns the theta variable for the
<function_prolate_spheroidal_to_rectangular_cartesian>.
==============================================================================*/

Cmiss_function_variable_id
	Cmiss_function_variable_prolate_spheroidal_to_rectangular_cartesian_focus(
	Cmiss_function_id function_prolate_spheroidal_to_rectangular_cartesian);
/*******************************************************************************
LAST MODIFIED : 4 June 2004

DESCRIPTION :
Returns the focus variable for the
<function_prolate_spheroidal_to_rectangular_cartesian>.
==============================================================================*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __API_CMISS_FUNCTION_COORDINATES_H__ */
