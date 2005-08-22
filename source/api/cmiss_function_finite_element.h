/*******************************************************************************
FILE : api/cmiss_function_finite_element.h

LAST MODIFIED : 17 February 2005

DESCRIPTION :
The public interface to the Cmiss_function_element, Cmiss_function_element_xi
and Cmiss_function_finite_element objects.
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
#ifndef __API_CMISS_FUNCTION_FINITE_ELEMENT_H__
#define __API_CMISS_FUNCTION_FINITE_ELEMENT_H__

#include "api/cmiss_finite_element.h"
#include "api/cmiss_function.h"
#include "api/cmiss_region.h"
#include "api/cmiss_time_sequence.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

Cmiss_function_id Cmiss_function_element_create(Cmiss_element_id element);
/*******************************************************************************
LAST MODIFIED : 26 April 2004

DESCRIPTION :
Creates a Cmiss_function which represents the <element>.
==============================================================================*/

int Cmiss_function_element_dimension(Cmiss_function_id function_element,
	unsigned int *dimension_address);
/*******************************************************************************
LAST MODIFIED : 26 April 2004

DESCRIPTION :
Gets the <*dimension_address> of the <function_element>.  Returns a non-zero for
success.
==============================================================================*/

int Cmiss_function_element_element_value(Cmiss_function_id function_element,
	Cmiss_element_id *element_address);
/*******************************************************************************
LAST MODIFIED : 26 April 2004

DESCRIPTION :
Gets the <*element_address> of the <function_element>.  Returns a non-zero for
success.

NB.  The calling program should use ACCESS(Cmiss_element) and
DEACCESS(Cmiss_element) to manage the lifetime of the returned element
==============================================================================*/

Cmiss_function_id Cmiss_function_element_xi_create(Cmiss_element_id element,
	unsigned int number_of_xi,Scalar *xi);
/*******************************************************************************
LAST MODIFIED : 5 May 2004

DESCRIPTION :
Creates a Cmiss_function which represents the <element>/<xi> location.  The
number of values in <xi>, <number_of_xi> and the dimension of <element> should
be equal.
==============================================================================*/

Cmiss_function_variable_id Cmiss_function_element_xi_element(
	Cmiss_function_id function_element_xi);
/*******************************************************************************
LAST MODIFIED : 26 April 2004

DESCRIPTION :
Returns a variable that refers to the element part of the <function_element_xi>.
==============================================================================*/

Cmiss_function_variable_id Cmiss_function_element_xi_xi(
	Cmiss_function_id function_element_xi);
/*******************************************************************************
LAST MODIFIED : 26 April 2004

DESCRIPTION :
Returns a variable that refers to the xi part of the <function_element_xi>.
==============================================================================*/

Cmiss_function_variable_id Cmiss_function_element_xi_xi_entry(
	Cmiss_function_id function_element_xi,unsigned int index);
/*******************************************************************************
LAST MODIFIED : 26 April 2004

DESCRIPTION :
Returns a variable that refers to the xi entry (<index>) of the
<function_element_xi>.  <index> number 1 is the first entry.
==============================================================================*/

int Cmiss_function_element_xi_number_of_xi(
	Cmiss_function_id function_element_xi,unsigned int *number_of_xi_address);
/*******************************************************************************
LAST MODIFIED : 26 April 2004

DESCRIPTION :
Gets the <*number_of_xi_address> of the <function_element_xi>.  Returns a
non-zero for success.
==============================================================================*/

int Cmiss_function_element_xi_element_value(
	Cmiss_function_id function_element_xi,Cmiss_element_id *element_address);
/*******************************************************************************
LAST MODIFIED : 26 April 2004

DESCRIPTION :
Gets the <*element_address> of the <function_element_xi>.  Returns a non-zero
for success.

NB.  The calling program should use ACCESS(Cmiss_element) and
DEACCESS(Cmiss_element) to manage the lifetime of the returned element
==============================================================================*/

int Cmiss_function_element_xi_xi_value(Cmiss_function_id function_element_xi,
	unsigned int index,Scalar *value_address);
/*******************************************************************************
LAST MODIFIED : 26 April 2004

DESCRIPTION :
Gets the <*value_address> for the specified xi entry (<index>) of the
<function_element_xi>.  <index> number 1 is the first entry.  Returns a non-zero
for success.
==============================================================================*/

Cmiss_function_id Cmiss_function_finite_element_create(Cmiss_region_id region,
	char *field);
/*******************************************************************************
LAST MODIFIED : 4 November 2004

DESCRIPTION :
Creates a Cmiss_function which represents the <field> in <region>.
==============================================================================*/

Cmiss_function_id 
   Cmiss_function_finite_element_create_standard_interpolation_rc_constant_time
   (Cmiss_region_id region, char *name, int number_of_components, 
	char **component_names);
/*******************************************************************************
LAST MODIFIED : 8 November 2004

DESCRIPTION :
Creates a Cmiss_function which represents a field named <name> in <region>.
The field will have a standard FE type interpolation, have a rectangular
cartesian coordinate system, it will have <number_of_components> components
which will be named <component_names>.
==============================================================================*/

Cmiss_function_variable_id Cmiss_function_finite_element_component(
	Cmiss_function_id function_finite_element,char *name,unsigned int number);
/*******************************************************************************
LAST MODIFIED : 26 April 2004

DESCRIPTION :
Returns a variable that refers to a component of the <function_finite_element>.
If <name> is not NULL, then the component with the <name> is specified.  If
<name> is NULL, then the component with the <number> is specified.  Component
<number> 1 is the first component.
==============================================================================*/

Cmiss_function_variable_id Cmiss_function_finite_element_nodal_values(
	Cmiss_function_id function_finite_element,char *component_name,
	unsigned int component_number,Cmiss_node_id node,
	enum FE_nodal_value_type value_type,unsigned int version,
	Cmiss_time_sequence_id time_sequence);
/*******************************************************************************
LAST MODIFIED : 18 November 2004

DESCRIPTION :
Returns a variable that refers to a subset of the nodal values for the
<function_finite_element>.

If <component_name> is not NULL, then the nodal values must be for the named
component.  If <component_name> is NULL and <component_number> is not zero then
the nodal values must be for that number component.

If <node> is not zero then the nodal values must be for that <node>.

If <value_type> is not FE_NODAL_UNKNOWN then the nodal values must be of that
<value_type>.

If <version> is not zero then the nodal values must be for that <version>.
==============================================================================*/

Cmiss_function_variable_id Cmiss_function_finite_element_element_xi(
	Cmiss_function_id function_finite_element);
/*******************************************************************************
LAST MODIFIED : 17 May 2004

DESCRIPTION :
Returns a variable that refers to the element/xi input of the
<function_finite_element>.
==============================================================================*/

Cmiss_function_variable_id Cmiss_function_finite_element_element(
	Cmiss_function_id function_finite_element);
/*******************************************************************************
LAST MODIFIED : 26 April 2004

DESCRIPTION :
Returns a variable that refers to the element part of the
<function_finite_element>.
==============================================================================*/

Cmiss_function_variable_id Cmiss_function_finite_element_time(
	Cmiss_function_id function_finite_element);
/*******************************************************************************
LAST MODIFIED : 17 February 2005

DESCRIPTION :
Returns a variable that refers to the time part of the
<function_finite_element>.
==============================================================================*/

Cmiss_function_variable_id Cmiss_function_finite_element_xi(
	Cmiss_function_id function_finite_element);
/*******************************************************************************
LAST MODIFIED : 26 April 2004

DESCRIPTION :
Returns a variable that refers to the xi part of the <function_finite_element>.
==============================================================================*/

Cmiss_function_variable_id Cmiss_function_finite_element_xi_entry(
	Cmiss_function_id function_finite_element,unsigned int index);
/*******************************************************************************
LAST MODIFIED : 26 April 2004

DESCRIPTION :
Returns a variable that refers to the xi entry (<index>) of the
<function_finite_element>.  <index> number 1 is the first entry.
==============================================================================*/

int Cmiss_function_finite_element_number_of_components(
	Cmiss_function_id function_finite_element,
	unsigned int *number_of_components_address);
/*******************************************************************************
LAST MODIFIED : 26 April 2004

DESCRIPTION :
Gets the <*number_of_components_address> of the <function_finite_element>.
Returns a non-zero for success.
==============================================================================*/

int Cmiss_function_finite_element_region(
	Cmiss_function_id function_finite_element,Cmiss_region_id *region_address);
/*******************************************************************************
LAST MODIFIED : 3 November 2004

DESCRIPTION :
Gets the <*region_address> of the <function_finite_element>.  Returns a non-zero
for success.

NB.  The calling program should use ACCESS(Cmiss_region) and
DEACCESS(Cmiss_region) to manage the lifetime of the returned element
==============================================================================*/

int Cmiss_function_finite_element_number_of_versions(
	Cmiss_function_id function_finite_element,unsigned int component_number,
	Cmiss_node_id node,unsigned int *number_of_versions_address);
/*******************************************************************************
LAST MODIFIED : 26 April 2004

DESCRIPTION :
Gets the <*number_of_versions_address> for the <component_number> and <node> of
the <function_finite_element>.  Returns a non-zero for success.
==============================================================================*/

int Cmiss_function_finite_element_number_of_derivatives(
	Cmiss_function_id function_finite_element,unsigned int component_number,
	Cmiss_node_id node,unsigned int *number_of_derivatives_address);
/*******************************************************************************
LAST MODIFIED : 26 April 2004

DESCRIPTION :
Gets the <*number_of_derivatives_address> for the <component_number> and <node>
of the <function_finite_element>.  Returns a non-zero for success.
==============================================================================*/

int Cmiss_function_finite_element_nodal_value_types(
	Cmiss_function_id function_finite_element,unsigned int component_number,
	Cmiss_node_id node,enum FE_nodal_value_type **nodal_value_types_address);
/*******************************************************************************
LAST MODIFIED : 26 April 2004

DESCRIPTION :
Gets the (1+number_of_derivatives) nodal value types for the <component_number>
and <node> of the <function_finite_element>.  Returns a non-zero for success.

NB.  The calling program should DEALLOCATE the returned array when it is no
longer needed.
==============================================================================*/

int Cmiss_function_finite_element_get_nodal_value(
	Cmiss_function_id function_finite_element,unsigned int component_number,
	Cmiss_node_id node,enum FE_nodal_value_type value_type,unsigned int version,
	Scalar time,Scalar *value_address);
/*******************************************************************************
LAST MODIFIED : 22 November 2004

DESCRIPTION :
Returns a non-zero and gets the value if exactly one nodal value is specified,
otherwise return zero.
==============================================================================*/

int Cmiss_function_finite_element_set_nodal_value(
	Cmiss_function_id function_finite_element,unsigned int component_number,
	Cmiss_node_id node,enum FE_nodal_value_type value_type,unsigned int version,
	Scalar time,Scalar value);
/*******************************************************************************
LAST MODIFIED : 22 November 2004

DESCRIPTION :
Returns a non-zero and sets the value if exactly one nodal value is specified,
otherwise return zero.
==============================================================================*/

int Cmiss_function_finite_element_number_of_xi(
	Cmiss_function_id function_finite_element,unsigned int *number_of_xi_address);
/*******************************************************************************
LAST MODIFIED : 26 April 2004

DESCRIPTION :
Gets the <*number_of_xi_address> of the <function_finite_element>.  Returns a
non-zero for success.
==============================================================================*/

int Cmiss_function_finite_element_element_value(
	Cmiss_function_id function_finite_element,
	Cmiss_element_id *element_address);
/*******************************************************************************
LAST MODIFIED : 26 April 2004

DESCRIPTION :
Gets the <*element_address> of the <function_finite_element>.  Returns a
non-zero for success.

NB.  The calling program should use ACCESS(Cmiss_element) and
DEACCESS(Cmiss_element) to manage the lifetime of the returned element
==============================================================================*/

int Cmiss_function_finite_element_xi_value(
	Cmiss_function_id function_finite_element,unsigned int index,
	Scalar *value_address);
/*******************************************************************************
LAST MODIFIED : 26 April 2004

DESCRIPTION :
Gets the <*value_address> for the specified xi entry (<index>) of the
<function_finite_element>.  <index> number 1 is the first entry.  Returns a
non-zero for success.
==============================================================================*/

int Cmiss_function_finite_element_define_on_Cmiss_node(
	Cmiss_function_id finite_element_function,
	Cmiss_node_id node,
	Cmiss_time_sequence_id time_sequence, 
	Cmiss_node_field_creator_id node_field_creator);
/*******************************************************************************
LAST MODIFIED : 10 November 2004

DESCRIPTION :
==============================================================================*/

int Cmiss_function_finite_element_define_tensor_product_basis_on_element(
	Cmiss_function_id finite_element_function, Cmiss_element_id element,
	int dimension, enum Cmiss_basis_type basis_type);
/*******************************************************************************
LAST MODIFIED : 1 December 2004

DESCRIPTION :
Defines a tensor product basis on the element with the specified <dimension>
and <basis_type>.  This does not support mixed basis types in the tensor product.
==============================================================================*/
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __API_CMISS_FUNCTION_FINITE_ELEMENT_H__ */
