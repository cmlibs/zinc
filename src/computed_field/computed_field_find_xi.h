/*******************************************************************************
FILE : computed_field_find_xi.h

LAST MODIFIED : 18 April 2005

DESCRIPTION :
Implements algorithm to find the element and xi location at which a field
has a particular value.
==============================================================================*/
/* Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#if !defined (COMPUTED_FIELD_FIND_XI_H)
#define COMPUTED_FIELD_FIND_XI_H

#include "region/cmiss_region.hpp"

class Computed_field_find_element_xi_cache;

class FeMeshFieldRanges;

/**
 * Find location in mesh or element where the field has same or nearest value to
 * the prescribed values.
 *
 * @param field  The field whose values need to match.
 * @param field_cache  cmzn_fieldcache for evaluating fields with. Time is
 * expected to have been set in the field_cache if needed.
 * @param findElementXiCache  Cache for storing values during search.
 * @param meshFieldRanges  Optional map from elements to ranges of field to
 * quickly eliminate elements too far away.
 * @param values  Array of values to match or get nearest to. Implementation
 * promises to copy this, hence can pass a pointer to field cache values.
 * @param number_of_values  The size of the values array, must equal the number
 * of components of field.
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
int Computed_field_find_element_xi(struct Computed_field *field,
	cmzn_fieldcache_id field_cache,
	Computed_field_find_element_xi_cache *findElementXiCache,
	const FeMeshFieldRanges *meshFieldRanges,
	const FE_value *values, int number_of_values,
	struct FE_element **element_address, FE_value *xi,
	cmzn_mesh *searchMesh, int find_nearest);

#endif /* !defined (COMPUTED_FIELD_FIND_XI_H) */
