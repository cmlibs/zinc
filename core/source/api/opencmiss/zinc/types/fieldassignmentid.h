/**
 * @file fieldassignmentid.h
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMZN_FIELDASSIGNMENTID_H__
#define CMZN_FIELDASSIGNMENTID_H__

/**
 * @brief  Object for assigning values of a field from a source field.
 *
 * Object describing and performing assignment of a source field to a
 * target field such that the target field will afterwards give as near as
 * possible the same values over its domain as the source.
 * Beware this object may not handle all cases such as element-based fields.
 * It is initially designed to assign nodal parameters including derivatives.
 */
struct cmzn_fieldassignment;
typedef struct cmzn_fieldassignment *cmzn_fieldassignment_id;

#endif
