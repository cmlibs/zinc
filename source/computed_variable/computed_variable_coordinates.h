/*******************************************************************************
FILE : computed_variable_coordinates.h

LAST MODIFIED : 28 July 2003

DESCRIPTION :
Implements computed variables which transform between coordinate systems.
==============================================================================*/
#if !defined (__CMISS_VARIABLE_COORDINATES_H__)
#define __CMISS_VARIABLE_COORDINATES_H__

#include "computed_variable/computed_variable.h"

/*
Global functions
----------------
*/
int Cmiss_variable_coordinates_set_type(Cmiss_variable_id variable,
	int number_of_coordinates);
/*******************************************************************************
LAST MODIFIED : 29 June 2003

DESCRIPTION :
Converts the <variable> into a coordinates Cmiss_variable.

Only used to name independent variables and so can't be evaluated.
==============================================================================*/

PROTOTYPE_CMISS_VARIABLE_IS_TYPE_FUNCTION(coordinates);

int Cmiss_variable_coordinates_get_type(Cmiss_variable_id variable,
	int *dimension_address);
/*******************************************************************************
LAST MODIFIED : 28 July 2003

DESCRIPTION :
If <variable> is of type coordinates gets its <*dimension_address>.
==============================================================================*/

int Cmiss_variable_spheroidal_coordinates_focus_set_type(
	Cmiss_variable_id variable);
/*******************************************************************************
LAST MODIFIED : 27 June 2003

DESCRIPTION :
Converts the <variable> into a spheroidal_coordinates_focus Cmiss_variable.

Only used to name independent variables and so can't be evaluated.
==============================================================================*/

PROTOTYPE_CMISS_VARIABLE_IS_TYPE_FUNCTION(spheroidal_coordinates_focus);

int Cmiss_variable_prolate_spheroidal_to_rectangular_cartesian_set_type(
	Cmiss_variable_id variable);
/*******************************************************************************
LAST MODIFIED : 27 June 2003

DESCRIPTION :
Converts the <variable> into a prolate_spheroidal_to_rectangular_cartesian
Cmiss_variable.

Independent variables are: coordinates and spheroidal_coordinates_focus.
==============================================================================*/

PROTOTYPE_CMISS_VARIABLE_IS_TYPE_FUNCTION(
	prolate_spheroidal_to_rectangular_cartesian);

#endif /* !defined (__CMISS_VARIABLE_COORDINATES_H__) */
