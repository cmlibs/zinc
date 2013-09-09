/***************************************************************************//**
 * FILE : sceneviewerinputid.h
 *
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMZN_SCENEVIEWERINPUTID_H__
#define CMZN_SCENEVIEWERINPUTID_H__

/**
 * The type of scene viewer input button.
 */
enum cmzn_scene_viewer_input_button_type
{
	CMZN_SCENE_VIEWER_INPUT_BUTTON_INVALID = -1,
	CMZN_SCENE_VIEWER_INPUT_BUTTON_LEFT,
	CMZN_SCENE_VIEWER_INPUT_BUTTON_MIDDLE,
	CMZN_SCENE_VIEWER_INPUT_BUTTON_RIGHT,
	CMZN_SCENE_VIEWER_INPUT_BUTTON_SCROLL_DOWN,
	CMZN_SCENE_VIEWER_INPUT_BUTTON_SCROLL_UP
};

struct cmzn_scene_viewer_input;
typedef struct cmzn_scene_viewer_input *cmzn_scene_viewer_input_id;

typedef int cmzn_scene_viewer_input_modifier;

#endif
