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
 * Field attribute describing the type of space that its values are to be
 * interpreted in. Although it is usually set for all fields (default is
 * rectangular cartesian, RC), the attribute is only relevant when field is
 * used to supply coordinates or vector values, e.g. to graphics, where it
 * prompts automatic conversion to the underlying RC coordinate system.
 */
enum cmzn_field_coordinate_system_type
{
	CMZN_FIELD_COORDINATE_SYSTEM_TYPE_INVALID = 0,
	CMZN_FIELD_COORDINATE_SYSTEM_TYPE_RECTANGULAR_CARTESIAN = 1,
	CMZN_FIELD_COORDINATE_SYSTEM_TYPE_CYLINDRICAL_POLAR = 2,
	CMZN_FIELD_COORDINATE_SYSTEM_TYPE_SPHERICAL_POLAR = 3,
	CMZN_FIELD_COORDINATE_SYSTEM_TYPE_PROLATE_SPHEROIDAL = 4,
		/*!< @see cmzn_field_set_coordinate_system_focus */
	CMZN_FIELD_COORDINATE_SYSTEM_TYPE_OBLATE_SPHEROIDAL = 5,
		/*!< @see cmzn_field_set_coordinate_system_focus */
	CMZN_FIELD_COORDINATE_SYSTEM_TYPE_FIBRE = 6,
		/*!< For Euler angles specifying fibre axes orientation from default
		 * aligned with element xi coordinates. */
};

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

/**
 * The types of values fields may produce.
 * @see cmzn_field_get_value_type
 */
enum cmzn_field_value_type
{
	CMZN_FIELD_VALUE_TYPE_INVALID = 0,
	CMZN_FIELD_VALUE_TYPE_REAL = 1,
	CMZN_FIELD_VALUE_TYPE_STRING = 2,
	CMZN_FIELD_VALUE_TYPE_MESH_LOCATION = 3
};

#endif
