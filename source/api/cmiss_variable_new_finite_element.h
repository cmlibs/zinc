/*******************************************************************************
FILE : api/cmiss_variable_new_finite_element.h

LAST MODIFIED : 12 November 2003

DESCRIPTION :
The public interface to the Cmiss_variable_new finite_element object.
==============================================================================*/
#ifndef __API_CMISS_VARIABLE_NEW_FINITE_ELEMENT_H__
#define __API_CMISS_VARIABLE_NEW_FINITE_ELEMENT_H__

#include "api/cmiss_finite_element.h"
#include "api/cmiss_variable_new.h"

/*
Global functions
----------------
*/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

Cmiss_variable_new_id Cmiss_variable_new_element_xi_create(
	struct Cmiss_element *element,unsigned int number_of_xi,Scalar *xi);
/*******************************************************************************
LAST MODIFIED : 12 November 2003

DESCRIPTION :
Creates a Cmiss_variable_new element/xi which represents the supplied <element>
and <xi>.  <element> should be non-NULL.  If <xi> is non-NULL, then it should
have <number_of_xi> values and <number_of_xi> should equal dimension(<element>).
==============================================================================*/

Cmiss_variable_new_input_id Cmiss_variable_new_input_element_xi_element_xi(
	Cmiss_variable_new_id variable_element_xi);
/*******************************************************************************
LAST MODIFIED : 11 November 2003

DESCRIPTION :
Returns the element/xi input for the <variable_element_xi>.
==============================================================================*/

Cmiss_variable_new_input_id Cmiss_variable_new_input_element_xi_xi(
	Cmiss_variable_new_id variable_element_xi,unsigned int number_of_indices,
	unsigned int *indices);
/*******************************************************************************
LAST MODIFIED : 11 November 2003

DESCRIPTION :
Returns the xi input made up of the specified <indices> for the
<variable_finite_element>.  If <number_of_indices> is zero or <indices> is NULL
then the input refers to all values.
==============================================================================*/

Cmiss_variable_new_id Cmiss_variable_new_finite_element_create(
	struct Cmiss_FE_field *field,char *component_name);
/*******************************************************************************
LAST MODIFIED : 11 November 2003

DESCRIPTION :
Creates a Cmiss_variable_new finite_element which represents the supplied
<field>.  If <component_name> is not NULL then that is used to select a
particular component.
==============================================================================*/

Cmiss_variable_new_input_id Cmiss_variable_new_input_finite_element_element_xi(
	Cmiss_variable_new_id variable_finite_element);
/*******************************************************************************
LAST MODIFIED : 11 November 2003

DESCRIPTION :
Returns the element/xi input for the <variable_finite_element>.
==============================================================================*/

Cmiss_variable_new_input_id
	Cmiss_variable_new_input_finite_element_nodal_values(
	Cmiss_variable_new_id variable_finite_element,struct Cmiss_node *node,
	enum FE_nodal_value_type value_type,int version);
/*******************************************************************************
LAST MODIFIED : 11 November 2003

DESCRIPTION :
Returns the nodal values input for the <variable_finite_element>.
==============================================================================*/

Cmiss_variable_new_input_id Cmiss_variable_new_input_finite_element_xi(
	Cmiss_variable_new_id variable_finite_element,unsigned int number_of_indices,
	unsigned int *indices);
/*******************************************************************************
LAST MODIFIED : 11 November 2003

DESCRIPTION :
Returns the xi input made up of the specified <indices> for the
<variable_finite_element>.  If <number_of_indices> is zero or <indices> is NULL
then the input refers to all values.
==============================================================================*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __API_CMISS_VARIABLE_NEW_FINITE_ELEMENT_H__ */
