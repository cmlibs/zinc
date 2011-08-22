/*******************************************************************************
FILE : computed_field_find_xi.h

LAST MODIFIED : 18 April 2005

DESCRIPTION :
Implements algorithm to find the element and xi location at which a field
has a particular value.
==============================================================================*/
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
#if !defined (COMPUTED_FIELD_FIND_XI_H)
#define COMPUTED_FIELD_FIND_XI_H

#include "region/cmiss_region.h"

struct Graphics_buffer_package;

struct Computed_field_find_element_xi_cache;
/*******************************************************************************
LAST MODIFIED : 4 July 2000

DESCRIPTION :
struct Computed_field_find_element_xi_cache is private.
==============================================================================*/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/***************************************************************************//**
 * Find location in mesh or element where the field has same or nearest value to
 * the prescribed values. This routine is either called directly by
 * Computed_field_find_element_xi or if that field is propagating it's values
 * backwards, it is called by the last ancestor field implementing
 * propagate_find_element_xi.
 *
 * @param field  The field whose values need to match.
 * @param values  Array of values to match or get nearest to. Implementation
 * promises to copy this, hence can pass a pointer to field cache values.
 * @param number_of_values  The size of the values array, must equal the number
 * of components of field.
 * @param time  The time at which field values are evaluated.
 * @param element_address  Address to return element in. If mesh is omitted,
 * must point at a single element to search.
 * @param xi  Array of same dimension as mesh or element to return chart
 * coordinates in.
 * @param mesh  The mesh to search over. Can be omitted if element specified.
 * @param find_nearest  Set to 1 to find location of nearest field value, or 0
 * to find exact match.
 * @return  1 if search carried out without error including when no element is
 * found, or 0 if failed.
 */
int Computed_field_perform_find_element_xi(struct Computed_field *field,
	const FE_value *values, int number_of_values, FE_value time,
	struct FE_element **element_address, FE_value *xi,
	Cmiss_mesh_id search_mesh, int find_nearest);

int DESTROY(Computed_field_find_element_xi_cache)
	  (struct Computed_field_find_element_xi_cache **cache_address);
/*******************************************************************************
LAST MODIFIED : 20 June 2000

DESCRIPTION :
Frees memory/deaccess cache at <*cache_address>.
==============================================================================*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* !defined (COMPUTED_FIELD_FIND_XI_H) */
