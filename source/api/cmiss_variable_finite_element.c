/*******************************************************************************
FILE : api/cmiss_variable_finite_element.c

LAST MODIFIED : 13 August 2003

DESCRIPTION :
The public interface to the Cmiss_variable_finite_element object.
==============================================================================*/
#include "api/cmiss_variable_finite_element.h"
#include "computed_variable/computed_variable_finite_element.h"
#include "general/debug.h"
#include "user_interface/message.h"

/*
Global functions
----------------
*/

Cmiss_variable_id CREATE(Cmiss_variable_finite_element)(struct Cmiss_FE_field *field,
	char *component_name)
/*******************************************************************************
LAST MODIFIED : 13 August 2003

DESCRIPTION :
Creates a Cmiss_variable which represents the supplied <field>.  If <component_name>
is not NULL then that is used to select a particular component.
==============================================================================*/
{
	char *name;
	int component_number;
	Cmiss_variable_id return_variable;

	ENTER(CREATE(Cmiss_variable_finite_element));
	return_variable = (Cmiss_variable_id)NULL;
	if (field)
	{
		name=(char *)NULL;
		if (GET_NAME(FE_field)(field,&name)&&(return_variable=CREATE(Cmiss_variable)(
			(struct Cmiss_variable_package *)NULL,name)))
		{
			ACCESS(Cmiss_variable)(return_variable);
			if (name)
			{
				DEALLOCATE(name);
			}
			if (component_name)
			{
				component_number=get_FE_field_number_of_components(field);
				if (0<component_number)
				{
					name=(char *)NULL;
					do
					{
						component_number--;
						if (name)
						{
							DEALLOCATE(name);
						}
						name=get_FE_field_component_name(field,component_number);
					} while ((component_number>0)&&strcmp(name,component_name));
					if (!name||strcmp(name,component_name))
					{
						DEACCESS(Cmiss_variable)(&return_variable);
					}
					if (name)
					{
						DEALLOCATE(name);
					}
				}
			}
			else
			{
				component_number= -1;
			}
			if (return_variable)
			{
				if (!Cmiss_variable_finite_element_set_type(return_variable,field,
						 component_number))
				{
					DEACCESS(Cmiss_variable)(&return_variable);
				}
			}
		}
		else
		{
			if (name)
			{
				DEALLOCATE(name);
			}
		}
	}
	LEAVE;

	return (return_variable);
} /* CREATE(Cmiss_variable_finite_element) */


Cmiss_variable_id CREATE(Cmiss_variable_element_xi)(char *name, int dimension)
/*******************************************************************************
LAST MODIFIED : 13 August 2003

DESCRIPTION :
Creates a Cmiss_variable which represents an element xi location of <dimension>.
==============================================================================*/
{
	Cmiss_variable_id return_variable;

	ENTER(CREATE(Cmiss_variable_element_xi));
	return_variable = (Cmiss_variable_id)NULL;
	if (name)
	{
		if (return_variable=CREATE(Cmiss_variable)((struct Cmiss_variable_package *)NULL,name))
		{
			ACCESS(Cmiss_variable)(return_variable);
			if (!Cmiss_variable_element_xi_set_type(return_variable, dimension))
			{
				DEACCESS(Cmiss_variable)(&return_variable);
			}
		}
	}
	LEAVE;

	return (return_variable);
} /* CREATE(Cmiss_variable_element_xi) */

Cmiss_variable_id CREATE(Cmiss_variable_nodal_value)(char *name,
	Cmiss_variable_id fe_variable, struct FE_node *node,
	enum FE_nodal_value_type value_type, int version)
/*******************************************************************************
LAST MODIFIED : 14 August 2003

DESCRIPTION :
Creates a Cmiss_variable which represents a nodal degree of freedom or a set of
nodal_degrees of freedom.
==============================================================================*/
{
	Cmiss_variable_id return_variable;

	ENTER(CREATE(Cmiss_variable_nodal_value));
	return_variable = (Cmiss_variable_id)NULL;
	if (name)
	{
		if (return_variable=CREATE(Cmiss_variable)((struct Cmiss_variable_package *)NULL,name))
		{
			ACCESS(Cmiss_variable)(return_variable);
			if (!Cmiss_variable_nodal_value_set_type(return_variable, fe_variable,
				node, value_type, version))
			{
				DEACCESS(Cmiss_variable)(&return_variable);
			}
		}
	}
	LEAVE;

	return (return_variable);
} /* CREATE(Cmiss_variable_nodal_value) */
