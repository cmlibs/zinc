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
 * Module managing all tessellation objects. It maintains separate default
 * tessellations for points and continuous graphics, the default points
 * tessellation having only 1 point in each direction.
 */
struct cmzn_tessellationmodule;
typedef struct cmzn_tessellationmodule * cmzn_tessellationmodule_id;

/**
 * The tessellation controls the number of polygons or line segments used to
 * draw element surfaces and lines, and circular forms in graphics; the density
 * of point sampling and the piecewise linear approximation of elements
 * generally.
 */
struct cmzn_tessellation;
typedef struct cmzn_tessellation * cmzn_tessellation_id;

#endif
