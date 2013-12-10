/*******************************************************************************
FILE : finite_element_region_private.h

LAST MODIFIED : 21 April 2005

DESCRIPTION :
Private interfaces to FE_region types and functions to be included only by
privileged finite_element modules.
==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (FINITE_ELEMENT_REGION_PRIVATE_H)
#define FINITE_ELEMENT_REGION_PRIVATE_H

#include "finite_element/finite_element_private.h"
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

/**
 * Tells the <fe_region> to notify any interested clients that the <element> has
 * been modified only for <fe_field>.  This is intended to be called from
 * finite_element.cpp only.
 */
void FE_region_notify_FE_element_field_change(struct FE_region *fe_region,
	struct FE_element *element, struct FE_field *fe_field);

/**
 * Private function for use by computed_field_finite_element field wrapping
 * code, to be called in response to region changes - tells it whether it to
 * create a cmiss_number field. Informs only once and requires the existence
 * of a node, element or data point (in the matching data_hack_fe_region).
 * 
 * @param fe_region the finite_element region to check for.
 * @return true if cmiss_number field needs to be made, 0 if not
 */
bool FE_region_need_add_cmiss_number_field(struct FE_region *fe_region);

/**
 * Private function for use by computed_field_finite_element field wrapping
 * code, to be called in response to region changes - tells it whether to
 * create an xi coordinates field. Informs only once - if any elements exist.
 * 
 * @param fe_region the finite_element region to check for.
 * @return true if cmiss_number field needs to be made, 0 if not
 */
bool FE_region_need_add_xi_field(struct FE_region *fe_region);

/**
 * Obtain private member.
 */
struct LIST(FE_element_field_info) *FE_region_get_FE_element_field_info_list_private(
	struct FE_region *fe_region);

#endif /* !defined (FINITE_ELEMENT_REGION_PRIVATE_H) */
