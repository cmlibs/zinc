/***************************************************************************//**
 * FILE : fieldfiniteelementid.h
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMZN_FIELDFINITEELEMENTID_H__
#define CMZN_FIELDFINITEELEMENTID_H__

/** Handle to a finite_element type cmzn_field */
struct cmzn_field_finite_element;
typedef struct cmzn_field_finite_element *cmzn_field_finite_element_id;
#define CMZN_FIELD_FINITE_ELEMENT_ID_DEFINED

/** Handle to a find_mesh_location type cmzn_field */
struct cmzn_field_find_mesh_location;
typedef struct cmzn_field_find_mesh_location *cmzn_field_find_mesh_location_id;

/** Handle to a stored_mesh_location type cmzn_field */
struct cmzn_field_stored_mesh_location;
typedef struct cmzn_field_stored_mesh_location *cmzn_field_stored_mesh_location_id;

/** Handle to a stored_string type cmzn_field */
struct cmzn_field_stored_string_location;
typedef struct cmzn_field_stored_string *cmzn_field_stored_string_id;

/***************************************************************************//**
 * Enumeration controlling whether find_mesh_location returns location of exact
 * field value match, or nearest value.
 */
enum cmzn_field_find_mesh_location_search_mode
{
	CMZN_FIELD_FIND_MESH_LOCATION_SEARCH_MODE_FIND_EXACT = 1,
	/*!< Only location where mesh field value is exactly equal to source field is
	 * returned. This is the default search criterion.
	 */
	CMZN_FIELD_FIND_MESH_LOCATION_SEARCH_MODE_FIND_NEAREST = 2,
	/*!< Location of RMS nearest value of mesh field to source field is returned.
	 */
};

#endif
