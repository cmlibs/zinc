/**
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

	struct cmzn_sceneviewermodule;
	typedef struct cmzn_sceneviewermodule * cmzn_sceneviewermodule_id;

	struct cmzn_sceneviewer;
	typedef struct cmzn_sceneviewer *cmzn_sceneviewer_id;

	/* The cmzn_sceneviewerinput describes the input event */
	typedef int (*cmzn_sceneviewerinput_callback)(
		cmzn_sceneviewer_id sceneviewer,
		struct cmzn_sceneviewerinput *, void *user_data);

	typedef void (*cmzn_sceneviewer_callback)(cmzn_sceneviewer_id sceneviewer,
		void *callback_data, void *user_data);

#endif
