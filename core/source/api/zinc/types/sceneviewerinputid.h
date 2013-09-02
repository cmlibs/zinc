/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2010-2011
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef CMZN_SCENEVIEWERINPUTID_H__
#define CMZN_SCENEVIEWERINPUTID_H__

/**
 * The type of scene viewer input button.
 */
enum cmzn_scene_viewer_input_button_type
{
	CMISS_SCENE_VIEWER_INPUT_BUTTON_INVALID = -1,
	CMISS_SCENE_VIEWER_INPUT_BUTTON_LEFT,
	CMISS_SCENE_VIEWER_INPUT_BUTTON_MIDDLE,
	CMISS_SCENE_VIEWER_INPUT_BUTTON_RIGHT,
	CMISS_SCENE_VIEWER_INPUT_BUTTON_SCROLL_DOWN,
	CMISS_SCENE_VIEWER_INPUT_BUTTON_SCROLL_UP
};

struct cmzn_scene_viewer_input;
typedef struct cmzn_scene_viewer_input *cmzn_scene_viewer_input_id;

typedef int cmzn_scene_viewer_input_modifier;

#endif
