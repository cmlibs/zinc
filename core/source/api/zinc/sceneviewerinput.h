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
 * Portions created by the Initial Developer are Copyright (C) 2005
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
#ifndef __CMISS_SCENE_VIEWER_INPUT_H__
#define __CMISS_SCENE_VIEWER_INPUT_H__

#include "sceneviewer.h"
#include "types/sceneviewerinputid.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create a scene viewer input object for manually setting mouse or other input
 * event data.
 *
 * @param scene_viewer  Handle to Cmiss_scene_viewer object.
 * @return  Handle to Cmiss_scene_viewer_input on success, or 0 on failure.
 */
ZINC_API Cmiss_scene_viewer_input_id Cmiss_scene_viewer_get_input(Cmiss_scene_viewer_id scene_viewer);

/**
 * Returns a new reference to the Cmiss_scene_viewer_input with reference count incremented.
 * Caller is responsible for destroying the new reference.
 *
 * @param input  Handle to Cmiss_scene_viewer_input object.
 * @return  Accessed handle to Cmiss_scene_viewer_input.
 */
ZINC_API Cmiss_scene_viewer_input_id Cmiss_scene_viewer_input_access(Cmiss_scene_viewer_input_id input);

/**
 * Destroys this reference to the Cmiss_scene_viewer_input (and sets it to 0).
 * Internally this just decrements the reference count.
 *
 * @param address_input  Address of handle to Cmiss_scene_viewer_input object.
 * @return  CMISS_OK if successfully deaccess Cmiss_scene_viewer_input, any other value on
 * failure.
 */
ZINC_API int Cmiss_scene_viewer_input_destroy(Cmiss_scene_viewer_input_id *address_input);

/**
 * Set the position of the input
 *
 * @param input  Handle to Cmiss_scene_viewer_input object.
 * @param x The x-coordinate of the input position.
 * @param y The y-coordinate of the input position.
 * @return  CMISS_OK if successful, any other value on failure.
 */
ZINC_API int Cmiss_scene_viewer_input_set_position(Cmiss_scene_viewer_input_id input, int x, int y);

/**
 * Set modifier for the input.
 *
 * @param input  Handle to Cmiss_scene_viewer_input object.
 * @param modifier  Enumerator for the modifier to set.
 * @return  Handle to Cmiss_scene_viewer_input on success, or 0 on failure.
 */
ZINC_API int Cmiss_scene_viewer_input_set_modifier(Cmiss_scene_viewer_input_id input, Cmiss_scene_viewer_input_modifier modifier);

/**
 * Get the modifier set for the given input.
 *
 * @param input  Handle to Cmiss_scene_viewer_input object.
 * @return  Enumerator for the input modifier.
 */
ZINC_API Cmiss_scene_viewer_input_modifier Cmiss_scene_viewer_input_get_modifier(Cmiss_scene_viewer_input_id input);

/**
 * Set the button number for the input.
 *   1 ==> left mouse button.
 *   2 ==> middle mouse button.
 *   3 ==> right mouse button.
 *
 * @deprecated see Cmiss_scene_viewer_input_set_button
 *
 * @param input  Handle to Cmiss_scene_viewer_input object.
 * @param number  The numeric value of the button.
 * @return  CMISS_OK if successful, any other value on failure.
 */
ZINC_API int Cmiss_scene_viewer_input_set_button_number(Cmiss_scene_viewer_input_id input, int number);

/**
 * Set the button for the scene viewer input.
 *
 * @param input Cmiss scene viewer input.
 * @param button Cmiss scene viewer input button enumeration value.
 * @return CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_scene_viewer_input_set_button(Cmiss_scene_viewer_input_id input, Cmiss_scene_viewer_input_button_type button);

/**
 * Create a scene viewer input object for manually setting mouse or other input
 * event data.
 *
 * @param input  Handle to Cmiss_scene_viewer_input object.
 * @param type  Enumerator for the input event type.
 * @return  CMISS_OK if successful, any other value on failure.
 */
ZINC_API int Cmiss_scene_viewer_input_set_type(Cmiss_scene_viewer_input_id input, Cmiss_scene_viewer_input_event_type type);

/**
 * Converts mouse button-press and motion events set in the input into viewport zoom
 * and translate transformations.
 *
 * @param scene_viewer  Handle to Cmiss_scene_viewer object.
 * @param input  Handle to Cmiss_scene_viewer_input object.
 * @return  CMISS_OK if successful, any other value on failure.
 */
ZINC_API int Cmiss_scene_viewer_input_viewport_transform(
	Cmiss_scene_viewer_id scene_viewer, Cmiss_scene_viewer_input_id input);

#ifdef __cplusplus
}
#endif

#endif /* __CMISS_SCENE_VIEWER_INPUT_H__ */




