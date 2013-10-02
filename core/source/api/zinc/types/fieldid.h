/***************************************************************************//**
 * FILE : fieldid.h
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMZN_FIELDID_H__
#define CMZN_FIELDID_H__

	struct cmzn_field;
	typedef struct cmzn_field *cmzn_field_id;

	struct cmzn_fielditerator;
	typedef struct cmzn_fielditerator * cmzn_fielditerator_id;

/**
 * An enum specifying the field domain (without domain objects)
 */
enum cmzn_field_domain_type
{
	CMZN_FIELD_DOMAIN_TYPE_INVALID = 0,
	CMZN_FIELD_DOMAIN_POINT = 1,
		/*!< A single point for the region */
	CMZN_FIELD_DOMAIN_NODES = 2,
		/*!< The set of node points */
	CMZN_FIELD_DOMAIN_DATA = 4,
		/*!< The set of data points */
	CMZN_FIELD_DOMAIN_MESH_1D = 8,
		/*!< The set of 1-D elements and edge lines of 2-D or 3-D elements */
	CMZN_FIELD_DOMAIN_MESH_2D = 16,
		/*!< The set of 2-D elements and faces of 3-D elements */
	CMZN_FIELD_DOMAIN_MESH_3D = 32,
		/*!< The set of 3-D elements */
	CMZN_FIELD_DOMAIN_MESH_HIGHEST_DIMENSION = 64
		/*!< The set of elements of highest dimension in region */
};

#endif
