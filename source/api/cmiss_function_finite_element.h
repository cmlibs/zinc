/*******************************************************************************
FILE : api/cmiss_function_finite_element.h

LAST MODIFIED : 17 May 2004

DESCRIPTION :
The public interface to the Cmiss_function_element, Cmiss_function_element_xi
and Cmiss_function_finite_element objects.
==============================================================================*/
#ifndef __API_CMISS_FUNCTION_FINITE_ELEMENT_H__
#define __API_CMISS_FUNCTION_FINITE_ELEMENT_H__

#include "api/cmiss_finite_element.h"
#include "api/cmiss_function.h"
#include "api/cmiss_region.h"

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

Cmiss_function_id Cmiss_function_finite_element_create(Cmiss_FE_field_id field);
/*******************************************************************************
LAST MODIFIED : 26 April 2004

DESCRIPTION :
Creates a Cmiss_function which represents the <field>.
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
	enum FE_nodal_value_type value_type,unsigned int version);
/*******************************************************************************
LAST MODIFIED : 26 April 2004

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
	Cmiss_function_id function_finite_element,Cmiss_FE_region_id *region_address);
/*******************************************************************************
LAST MODIFIED : 26 April 2004

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
	Scalar *value_address);
/*******************************************************************************
LAST MODIFIED : 26 April 2004

DESCRIPTION :
Returns a non-zero and gets the value if exactly one nodal value is specified,
otherwise return zero.
==============================================================================*/

int Cmiss_function_finite_element_set_nodal_value(
	Cmiss_function_id function_finite_element,unsigned int component_number,
	Cmiss_node_id node,enum FE_nodal_value_type value_type,unsigned int version,
	Scalar value);
/*******************************************************************************
LAST MODIFIED : 26 April 2004

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

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __API_CMISS_FUNCTION_FINITE_ELEMENT_H__ */
