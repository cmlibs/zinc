/*******************************************************************************
FILE : api/cmiss_variable_coordinates.h

LAST MODIFIED : 20 August 2003

DESCRIPTION :
The public interface to the Cmiss_variable_coordinates object.
==============================================================================*/
#ifndef __API_CMISS_VARIABLE_COORDINATES_H__
#define __API_CMISS_VARIABLE_COORDINATES_H__

/* If this is going to be in the API then it needs to have an interface there */
#include "general/object.h"
#include "api/cmiss_variable.h"

Cmiss_variable_id CREATE(Cmiss_variable_coordinates)(char *name,
	int dimension);
/*******************************************************************************
LAST MODIFIED : 20 August 2003

DESCRIPTION :
Creates a Cmiss_variable which represents a coordinate of the specified <dimension>.
==============================================================================*/

Cmiss_variable_id CREATE(Cmiss_variable_spheroidal_coordinates_focus)(char *name);
/*******************************************************************************
LAST MODIFIED : 20 August 2003

DESCRIPTION :
Creates a Cmiss_variable which represents a spheroidal coordinate focus.
==============================================================================*/

Cmiss_variable_id CREATE(Cmiss_variable_prolate_spheroidal_to_rectangular_cartesian)(char *name);
/*******************************************************************************
LAST MODIFIED : 20 August 2003

DESCRIPTION :
Creates a Cmiss_variable which represents a coordinate focus.
==============================================================================*/
#endif /* __API_CMISS_VARIABLE_COORDINATES_H__ */
