/**
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
enum cmzn_sceneviewerinput_button_type
{
	CMZN_SCENEVIEWERINPUT_BUTTON_TYPE_INVALID = -1,
	CMZN_SCENEVIEWERINPUT_BUTTON_LEFT,
	CMZN_SCENEVIEWERINPUT_BUTTON_MIDDLE,
	CMZN_SCENEVIEWERINPUT_BUTTON_RIGHT,
	CMZN_SCENEVIEWERINPUT_BUTTON_SCROLL_DOWN,
	CMZN_SCENEVIEWERINPUT_BUTTON_SCROLL_UP
};

/**
 * Specifies the scene viewer input event type.
 */
enum cmzn_sceneviewerinput_event_type
{
	CMZN_SCENEVIEWERINPUT_EVENT_TYPE_INVALID,
	CMZN_SCENEVIEWERINPUT_EVENT_MOTION_NOTIFY,
	CMZN_SCENEVIEWERINPUT_EVENT_BUTTON_PRESS,
	CMZN_SCENEVIEWERINPUT_EVENT_BUTTON_RELEASE,
	CMZN_SCENEVIEWERINPUT_EVENT_KEY_PRESS,
	CMZN_SCENEVIEWERINPUT_EVENT_KEY_RELEASE
};

/**
 * Specifies the scene viewer input modifier flags.
 */
enum cmzn_sceneviewerinput_modifier_flags
{
	CMZN_SCENEVIEWERINPUT_MODIFIER_NONE = 0,
	CMZN_SCENEVIEWERINPUT_MODIFIER_SHIFT = 1,
	CMZN_SCENEVIEWERINPUT_MODIFIER_CONTROL = 2,
	CMZN_SCENEVIEWERINPUT_MODIFIER_ALT = 4,
	CMZN_SCENEVIEWERINPUT_MODIFIER_BUTTON1 = 8
};

struct cmzn_sceneviewerinput;
typedef struct cmzn_sceneviewerinput *cmzn_sceneviewerinput_id;

/**
 * Type for passing logical OR of #cmzn_sceneviewerinput_modifier_flags
 */
typedef int cmzn_sceneviewerinput_modifiers_type;

#endif
