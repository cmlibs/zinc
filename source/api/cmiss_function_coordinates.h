/*******************************************************************************
FILE : api/cmiss_function_coordinates.h

LAST MODIFIED : 4 June 2004

DESCRIPTION :
The public interface to the Cmiss_function coordinates objects.
==============================================================================*/
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
