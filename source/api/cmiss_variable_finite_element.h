/*******************************************************************************
FILE : api/cmiss_variable_finite_element.h

LAST MODIFIED : 4 November 2004

DESCRIPTION :
The public interface to the Cmiss_variable_finite_element object.
==============================================================================*/
#ifndef __API_CMISS_VARIABLE_FINITE_ELEMENT_H__
#define __API_CMISS_VARIABLE_FINITE_ELEMENT_H__

/* If this is going to be in the API then it needs to have an interface there */
#include "general/object.h"
#include "api/cmiss_finite_element.h"
#include "api/cmiss_variable.h"
#include "api/cmiss_region.h"

Cmiss_variable_id CREATE(Cmiss_variable_finite_element)(Cmiss_region_id region,
	char *field_name,char *component_name);
/*******************************************************************************
LAST MODIFIED : 4 November 2004

DESCRIPTION :
Creates a Cmiss_variable which represents the <field_name> in <region>.  If
<component_name> is not NULL then that is used to select a particular component.
==============================================================================*/

Cmiss_variable_id CREATE(Cmiss_variable_element_xi)(char *name, int dimension);
/*******************************************************************************
LAST MODIFIED : 13 August 2003

DESCRIPTION :
Creates a Cmiss_variable which represents an element xi location of <dimension>.
==============================================================================*/

Cmiss_variable_id CREATE(Cmiss_variable_nodal_value)(char *name,
	Cmiss_variable_id fe_variable, struct Cmiss_node *node,
	enum FE_nodal_value_type value_type, int version);
/*******************************************************************************
LAST MODIFIED : 14 August 2003

DESCRIPTION :
Creates a Cmiss_variable which represents a nodal degree of freedom or a set of
nodal_degrees of freedom.
==============================================================================*/

#endif /* __API_CMISS_VARIABLE_FINITE_ELEMENT_H__ */
