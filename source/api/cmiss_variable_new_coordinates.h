/*******************************************************************************
FILE : api/cmiss_variable_new_coordinates.h

LAST MODIFIED : 20 November 2003

DESCRIPTION :
The public interface to the Cmiss_variable_new coordinates objects.
==============================================================================*/
#ifndef __API_CMISS_VARIABLE_NEW_COORDINATES_H__
#define __API_CMISS_VARIABLE_NEW_COORDINATES_H__

#include "api/cmiss_variable_new.h"

/*
Global functions
----------------
*/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

Cmiss_variable_new_id
	Cmiss_variable_new_prolate_spheroidal_to_rectangular_cartesian_create(
	Scalar lambda,Scalar mu,Scalar theta,Scalar focus);
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
Creates a Cmiss_variable_new prolate_spheroidal_to_rectangular_cartesian.
==============================================================================*/

Cmiss_variable_new_input_id
	Cmiss_variable_new_input_prolate_spheroidal_to_rectangular_cartesian_prolate(
	Cmiss_variable_new_id variable_prolate_spheroidal_to_rectangular_cartesian);
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
Returns the prolate input for the
<variable_prolate_spheroidal_to_rectangular_cartesian>.
==============================================================================*/

Cmiss_variable_new_input_id
	Cmiss_variable_new_input_prolate_spheroidal_to_rectangular_cartesian_lambda(
	Cmiss_variable_new_id variable_prolate_spheroidal_to_rectangular_cartesian);
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
Returns the lambda input for the
<variable_prolate_spheroidal_to_rectangular_cartesian>.
==============================================================================*/

Cmiss_variable_new_input_id
	Cmiss_variable_new_input_prolate_spheroidal_to_rectangular_cartesian_mu(
	Cmiss_variable_new_id variable_prolate_spheroidal_to_rectangular_cartesian);
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
Returns the mu input for the
<variable_prolate_spheroidal_to_rectangular_cartesian>.
==============================================================================*/

Cmiss_variable_new_input_id
	Cmiss_variable_new_input_prolate_spheroidal_to_rectangular_cartesian_theta(
	Cmiss_variable_new_id variable_prolate_spheroidal_to_rectangular_cartesian);
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
Returns the theta input for the
<variable_prolate_spheroidal_to_rectangular_cartesian>.
==============================================================================*/

Cmiss_variable_new_input_id
	Cmiss_variable_new_input_prolate_spheroidal_to_rectangular_cartesian_focus(
	Cmiss_variable_new_id variable_prolate_spheroidal_to_rectangular_cartesian);
/*******************************************************************************
LAST MODIFIED : 20 November 2003

DESCRIPTION :
Returns the focus input for the
<variable_prolate_spheroidal_to_rectangular_cartesian>.
==============================================================================*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __API_CMISS_VARIABLE_NEW_COORDINATES_H__ */
