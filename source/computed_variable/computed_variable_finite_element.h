/*******************************************************************************
FILE : computed_variable_finite_element.h

LAST MODIFIED : 9 April 2003

DESCRIPTION :
Implements computed variables which interface to finite element fields:
- cmiss_number.  Returns the cmiss number of a node or an element
- embedded.  Used for node fields that give element/xi - data at material points
???DB.  Why is Cmiss_variable_set_type_embedded static?
???DB.  Extend to element/xi
- finite_element.  A wrapper for a FE_field
- node_value
- xi_coordinate.  Returns the xi coordinates as a vector of FE_values -
	basically the identity
???DB.  Currently only implemented for element/xi ie. doesn't return the
	xi_coordinates of the node - depends on element

NOTES :

???DB.  Add an integral variable type?

???DB.  Is a computed_variable_finite_element_utilities module needed?
???DB.  Consistency for is_type functions.  Currently some are iterator/
	conditional and some are not
==============================================================================*/
#if !defined (__CMISS_VARIABLE_FINITE_ELEMENT_H__)
#define __CMISS_VARIABLE_FINITE_ELEMENT_H__

#include "finite_element/finite_element.h"
#include "computed_variable/computed_value.h"
#include "computed_variable/computed_variable.h"

/*
Global types
------------
*/
struct Cmiss_variable_finite_element_package;
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Private package
==============================================================================*/

/*
Global functions
----------------
*/
int Cmiss_variable_element_value_set_type(Cmiss_variable_id variable,
	Cmiss_variable_id fe_variable,struct FE_element *element,
	int *grid_point,int version);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
Converts the <variable> into a element_value Cmiss_variable for the
specified <fe_variable> (a finite_element Cmiss_variable, all finite element
computed variables if NULL), <element> (all elements if NULL), <grid_point> (all
grid points if NULL), <version> (all versions if 0).
==============================================================================*/

PROTOTYPE_CMISS_VARIABLE_IS_TYPE_FUNCTION(element_value);

int Cmiss_variable_element_value_get_type(
	Cmiss_variable_id variable,
	Cmiss_variable_id *fe_variable_address,
	struct FE_element **element_address,int **grid_point_address,
	int *version_address);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
If <variable> is of type element_value, gets its <*fe_variable_address>,
<*element_address>, <*grid_point_address> and <*version_address>.

The calling program must not DEALLOCATE the returned <*grid_point_address>.
==============================================================================*/

int Cmiss_variable_element_xi_set_type(Cmiss_variable_id variable,
	int dimension);
/*******************************************************************************
LAST MODIFIED : 11 July 2003

DESCRIPTION :
Converts the <variable> into a element_xi Cmiss_variable with the specified
<dimension>.
==============================================================================*/

PROTOTYPE_CMISS_VARIABLE_IS_TYPE_FUNCTION(element_xi);

int Cmiss_variable_element_xi_get_type(Cmiss_variable_id variable,
	int *dimension_address);
/*******************************************************************************
LAST MODIFIED : 11 July 2003

DESCRIPTION :
If <variable> is of type element_xi, gets its <*dimension_address>.
==============================================================================*/

int Cmiss_variable_FE_time_set_type(Cmiss_variable_id variable,
	Cmiss_variable_id fe_variable);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
Converts the <variable> into a FE_time Cmiss_variable for the specified
<fe_variable> (a finite_element Cmiss_variable, all finite element
computed variables if NULL).
==============================================================================*/

PROTOTYPE_CMISS_VARIABLE_IS_TYPE_FUNCTION(FE_time);

int Cmiss_variable_FE_time_get_type(Cmiss_variable_id variable,
	Cmiss_variable_id *fe_variable_address,FE_value *fe_time_address);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
If <variable> is of type FE_time, gets its <*fe_variable_address> and
<*fe_time_address>.
==============================================================================*/

int Cmiss_variable_finite_element_set_type(
	Cmiss_variable_id variable,struct FE_field *fe_field,
	int component_number);
/*******************************************************************************
LAST MODIFIED : 16 February 2003

DESCRIPTION :
Converts the <variable> into a finite_element Cmiss_variable for the
specified <fe_field> and <component_number> (all components if -1).

Need pointer to <fe_field_manager> so can call MANAGED_OBJECT_NOT_IN_USE in
Cmiss_variable_finite_element_not_in_use.

???DB.  Assuming that the <fe_field> "knows" its FE_region (can get FE_field
	manager)
==============================================================================*/

PROTOTYPE_CMISS_VARIABLE_IS_TYPE_FUNCTION(finite_element);

int Cmiss_variable_finite_element_get_type(
	Cmiss_variable_id variable,struct FE_field **fe_field_address,
	int *component_number_address);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
If <variable> is of type finite_element, gets its <*fe_field_address> and
<*component_number_address>.
==============================================================================*/

int Cmiss_variable_nodal_value_set_type(Cmiss_variable_id variable,
	Cmiss_variable_id fe_variable,struct FE_node *node,
	enum FE_nodal_value_type value_type,int version);
/*******************************************************************************
LAST MODIFIED : 25 March 2003

DESCRIPTION :
Converts the <variable> into a nodal_value Cmiss_variable for the
specified <fe_variable> (a finite_element Cmiss_variable), <node> (all nodes
if NULL), <value_type> (all types if FE_NODAL_UNKNOWN), <version> (all versions
if 0).
==============================================================================*/

PROTOTYPE_CMISS_VARIABLE_IS_TYPE_FUNCTION(nodal_value);

int Cmiss_variable_nodal_value_get_type(Cmiss_variable_id variable,
	Cmiss_variable_id *fe_variable_address,struct FE_node **node_address,
	enum FE_nodal_value_type *value_type_address,int *version_address);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
If <variable> is of type nodal_value, gets its <*fe_variable_address>,
<*node_address>, <*value_type_address> and <*version_address>.
==============================================================================*/

int Cmiss_variable_scale_factor_set_type(Cmiss_variable_id variable,
	Cmiss_variable_id fe_variable,struct FE_element *element,
	int local_node_number,enum FE_nodal_value_type value_type,int version);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
Converts the <variable> into a scale_factor Cmiss_variable for the
specified <fe_variable> (a finite_element Cmiss_variable, all finite element
computed variables if NULL), <element> (all elements if NULL),
<local_node_number> (all local nodes if -1), <value_type> (all types if
FE_NODAL_UNKNOWN), <version> (all versions if 0).
==============================================================================*/

PROTOTYPE_CMISS_VARIABLE_IS_TYPE_FUNCTION(scale_factor);

int Cmiss_variable_scale_factor_get_type(Cmiss_variable_id variable,
	Cmiss_variable_id *fe_variable_address,struct FE_element **element_address,
	int *local_node_number_address,enum FE_nodal_value_type *value_type_address,
	int *version_address);
/*******************************************************************************
LAST MODIFIED : 23 January 2003

DESCRIPTION :
If <variable> is of type scale_factor, gets its <*fe_variable_address>,
<*element_address>, <*local_node_number_address>, <*value_type_address> and
<*version_address>.
==============================================================================*/
#endif /* !defined (__CMISS_VARIABLE_FINITE_ELEMENT_H__) */
