/**
 * @file fieldgroupid.h
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMZN_FIELDGROUPID_H__
#define CMZN_FIELDGROUPID_H__

/**
 * @brief A generic group field used for grouping local subobjects.
 *
 * A generic group field type able to record which local subobjects are in the
 * group (recorded by attached node group and element group fields), or whether
 * the entire local region is in the group. This field returns true/1 at domain
 * locations in the group, false/0 otherwise.
 * The group field also maintains links to child groups in child regions, for
 * building subsets of entire region trees.
 */
struct cmzn_field_group;
typedef struct cmzn_field_group *cmzn_field_group_id;

#endif
