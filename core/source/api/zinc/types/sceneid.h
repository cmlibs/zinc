/**
 * FILE : sceneid.h
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMZN_SCENEID_H__
#define CMZN_SCENEID_H__

/**
 * A handle to zinc scene, zinc scene contains a top region to display
 * scene of itself and all of its child regions, it also store a a
 * collections of objects that make up a 3-D graphical model - lights,
 * materials, primitives, etc. Also contains interface rouitines for having
 * these converted to display lists and assembled into a single display list.
 * This single display list, however it is up to others - ie. the Scene_viewer
 * to display.
 */
	struct cmzn_scene;
	typedef struct cmzn_scene * cmzn_scene_id;

#endif
