/***************************************************************************//**
 * FILE : sceneviewerid.h
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMZN_SCENEVIEWERID_H__
#define CMZN_SCENEVIEWERID_H__

	struct cmzn_scene_viewer_module;
	typedef struct cmzn_scene_viewer_module * cmzn_scene_viewer_module_id;

	//-- struct cmzn_scene_viewer_input;
	//-- typedef struct cmzn_scene_viewer_input *cmzn_scene_viewer_input_id;

	struct cmzn_scene_viewer;
	typedef struct cmzn_scene_viewer *cmzn_scene_viewer_id;

	/* The cmzn_scene_viewer_input describes the input event */
	typedef int (*cmzn_scene_viewer_input_callback)(
		cmzn_scene_viewer_id scene_viewer,
		struct cmzn_scene_viewer_input *, void *user_data);

	typedef void (*cmzn_scene_viewer_callback)(cmzn_scene_viewer_id scene_viewer,
		void *callback_data, void *user_data);

#endif
