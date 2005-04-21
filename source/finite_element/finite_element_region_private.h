/*******************************************************************************
FILE : finite_element_region_private.h

LAST MODIFIED : 21 April 2005

DESCRIPTION :
Private interfaces to FE_region types and functions to be included only by
privileged finite_element modules.
==============================================================================*/
#if !defined (FINITE_ELEMENT_REGION_PRIVATE_H)
#define FINITE_ELEMENT_REGION_PRIVATE_H

#include "finite_element/finite_element_region.h"

/*
Private functions
-----------------
*/

struct FE_field_info *FE_region_get_FE_field_info(
	struct FE_region *fe_region);
/*******************************************************************************
LAST MODIFIED : 2 April 2003

DESCRIPTION :
Returns a struct FE_field_info for <fe_region>.
This is an object private to FE_region that is common between all fields
owned by FE_region. FE_fields access this object, but this object maintains
a non-ACCESSed pointer to <fe_region> so fields can determine which FE_region
they belong to.
==============================================================================*/

struct FE_node_field_info *FE_region_get_FE_node_field_info(
	struct FE_region *fe_region,
	int number_of_values,	struct LIST(FE_node_field) *fe_node_field_list);
/*******************************************************************************
LAST MODIFIED : 19 February 2003

DESCRIPTION :
Returns a struct FE_node_field_info for the supplied <fe_node_field_list> with
<number_of_values>. The <fe_region> maintains an internal list of these
structures so they can be shared between nodes.
If <node_field_list> is omitted, an empty list is assumed.
==============================================================================*/

int FE_region_remove_FE_node_field_info(struct FE_region *fe_region,
	struct FE_node_field_info *fe_node_field_info);
/*******************************************************************************
LAST MODIFIED : 19 February 2003

DESCRIPTION :
Provided EXCLUSIVELY for the use of DEACCESS and REACCESS functions.
Called when the access_count of <fe_node_field_info> drops to 1 so that
<fe_region> can destroy FE_node_field_info not in use.
==============================================================================*/

struct FE_element_field_info *FE_region_get_FE_element_field_info(
	struct FE_region *fe_region,
	struct LIST(FE_element_field) *fe_element_field_list);
/*******************************************************************************
LAST MODIFIED : 20 February 2003

DESCRIPTION :
Returns a struct FE_element_field_info for the supplied <fe_element_field_list>.
The <fe_region> maintains an internal list of these structures so they can be
shared between elements.
If <element_field_list> is omitted, an empty list is assumed.
==============================================================================*/

int FE_region_remove_FE_element_field_info(struct FE_region *fe_region,
	struct FE_element_field_info *fe_element_field_info);
/*******************************************************************************
LAST MODIFIED : 19 February 2003

DESCRIPTION :
Provided EXCLUSIVELY for the use of DEACCESS and REACCESS functions.
Called when the access_count of <fe_element_field_info> drops to 1 so that
<fe_region> can destroy FE_element_field_info not in use.
==============================================================================*/

int FE_region_notify_FE_node_field_change(struct FE_region *fe_region,
	struct FE_node *node, struct FE_field *fe_field);
/*******************************************************************************
LAST MODIFIED : 21 April 2005

DESCRIPTION :
Tells the <fe_region> to notify any interested clients that the <node> has
been modified only for <fe_field>.  This is intended to be called by 
<finite_element.c> only as any external code will call through the modify
functions in <finite_element.c>.
==============================================================================*/

int FE_region_notify_FE_element_field_change(struct FE_region *fe_region,
	struct FE_element *element, struct FE_field *fe_field);
/*******************************************************************************
LAST MODIFIED : 21 April 2005

DESCRIPTION :
Tells the <fe_region> to notify any interested clients that the <element> has
been modified only for <fe_field>.  This is intended to be called by 
<finite_element.c> only as any external code will call through the modify
functions in <finite_element.c>.
==============================================================================*/
#endif /* !defined (FINITE_ELEMENT_REGION_PRIVATE_H) */
