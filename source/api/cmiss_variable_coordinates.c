/*******************************************************************************
FILE : api/cmiss_variable_coordinates.c

LAST MODIFIED : 20 August 2003

DESCRIPTION :
The public interface to the Cmiss_variable_coordinates object.
==============================================================================*/
#include "api/cmiss_variable_coordinates.h"
#include "computed_variable/computed_variable_coordinates.h"
#include "general/debug.h"
#include "user_interface/message.h"

/*
Global functions
----------------
*/

Cmiss_variable_id CREATE(Cmiss_variable_coordinates)(char *name,
	int dimension)
/*******************************************************************************
LAST MODIFIED : 20 August 2003

DESCRIPTION :
Creates a Cmiss_variable which represents a coordinate of the specified <dimension>.
==============================================================================*/
{
	Cmiss_variable_id return_variable;

	ENTER(CREATE(Cmiss_variable_coordinates));
	return_variable = (Cmiss_variable_id)NULL;
	if (name)
	{
		if (return_variable=CREATE(Cmiss_variable)((struct Cmiss_variable_package *)NULL,name))
		{
			ACCESS(Cmiss_variable)(return_variable);
			if (!Cmiss_variable_coordinates_set_type(return_variable, dimension))
			{
				DEACCESS(Cmiss_variable)(&return_variable);
			}
		}
	}
	LEAVE;

	return (return_variable);
} /* CREATE(Cmiss_variable_coordinates) */

Cmiss_variable_id CREATE(Cmiss_variable_spheroidal_coordinates_focus)(char *name)
/*******************************************************************************
LAST MODIFIED : 20 August 2003

DESCRIPTION :
Creates a Cmiss_variable which represents a coordinate focus.
==============================================================================*/
{
	Cmiss_variable_id return_variable;

	ENTER(CREATE(Cmiss_variable_spheroidal_coordinates_focus));
	return_variable = (Cmiss_variable_id)NULL;
	if (name)
	{
		if (return_variable=CREATE(Cmiss_variable)((struct Cmiss_variable_package *)NULL,name))
		{
			ACCESS(Cmiss_variable)(return_variable);
			if (!Cmiss_variable_spheroidal_coordinates_focus_set_type(return_variable))
			{
				DEACCESS(Cmiss_variable)(&return_variable);
			}
		}
	}
	LEAVE;

	return (return_variable);
} /* CREATE(Cmiss_variable_spheroidal_coordinates_focus) */

Cmiss_variable_id CREATE(Cmiss_variable_prolate_spheroidal_to_rectangular_cartesian)(char *name)
/*******************************************************************************
LAST MODIFIED : 20 August 2003

DESCRIPTION :
Creates a Cmiss_variable which represents a coordinate focus.
==============================================================================*/
{
	Cmiss_variable_id return_variable;

	ENTER(CREATE(Cmiss_variable_prolate_spheroidal_to_rectangular_cartesian));
	return_variable = (Cmiss_variable_id)NULL;
	if (name)
	{
		if (return_variable=CREATE(Cmiss_variable)((struct Cmiss_variable_package *)NULL,name))
		{
			ACCESS(Cmiss_variable)(return_variable);
			if (!Cmiss_variable_prolate_spheroidal_to_rectangular_cartesian_set_type(return_variable))
			{
				DEACCESS(Cmiss_variable)(&return_variable);
			}
		}
	}
	LEAVE;

	return (return_variable);
} /* CREATE(Cmiss_variable_prolate_spheroidal_to_rectangular_cartesian) */

