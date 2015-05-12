/**
 * @file tessellationid.h
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMZN_TESSELLATIONID_H__
#define CMZN_TESSELLATIONID_H__

/**
 * @brief Module managing all tessellation objects.
 *
 * Module managing all tessellation objects. It maintains separate default
 * tessellations for points and continuous graphics, the default points
 * tessellation having only 1 point in each direction.
 */
struct cmzn_tessellationmodule;
typedef struct cmzn_tessellationmodule * cmzn_tessellationmodule_id;

/**
 * @brief The tessellation controls the number of polygons or line segments.
 *
 * The tessellation controls the number of polygons or line segments used to
 * draw element surfaces and lines, and circular forms in graphics; the density
 * of point sampling and the piecewise linear approximation of elements
 * generally.
 */
struct cmzn_tessellation;
typedef struct cmzn_tessellation * cmzn_tessellation_id;

/**
 * @brief An iterator for looping through all the tessellations in a
 * tessellation module.
 *
 * An iterator for looping through all the tessellations in a tessellation
 * module.
 */
struct cmzn_tessellationiterator;
typedef struct cmzn_tessellationiterator * cmzn_tessellationiterator_id;

#endif
