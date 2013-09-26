/***************************************************************************//**
 * FILE : sceneviewerinput.h
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */
#ifndef CMZN_SCENEVIEWERINPUT_H__
#define CMZN_SCENEVIEWERINPUT_H__

#include "sceneviewer.h"
#include "types/sceneviewerinputid.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create a scene viewer input object for manually setting mouse or other input
 * event data.
 *
 * @param scene_viewer  Handle to cmzn_scene_viewer object.
 * @return  Handle to cmzn_scene_viewer_input on success, or 0 on failure.
 */
ZINC_API cmzn_scene_viewer_input_id cmzn_scene_viewer_get_input(cmzn_scene_viewer_id scene_viewer);

/**
 * Returns a new reference to the cmzn_scene_viewer_input with reference count incremented.
 * Caller is responsible for destroying the new reference.
 *
 * @param input  Handle to cmzn_scene_viewer_input object.
 * @return  Accessed handle to cmzn_scene_viewer_input.
 */
ZINC_API cmzn_scene_viewer_input_id cmzn_scene_viewer_input_access(cmzn_scene_viewer_input_id input);

/**
 * Destroys this reference to the cmzn_scene_viewer_input (and sets it to 0).
 * Internally this just decrements the reference count.
 *
 * @param address_input  Address of handle to cmzn_scene_viewer_input object.
 * @return  CMZN_OK if successfully deaccess cmzn_scene_viewer_input, any other value on
 * failure.
 */
ZINC_API int cmzn_scene_viewer_input_destroy(cmzn_scene_viewer_input_id *address_input);

/**
 * Set the position of the input
 *
 * @param input  Handle to cmzn_scene_viewer_input object.
 * @param x The x-coordinate of the input position.
 * @param y The y-coordinate of the input position.
 * @return  CMZN_OK if successful, any other value on failure.
 */
ZINC_API int cmzn_scene_viewer_input_set_position(cmzn_scene_viewer_input_id input, int x, int y);

/**
 * Set modifier for the input.
 *
 * @param input  Handle to cmzn_scene_viewer_input object.
 * @param modifier  The input modifier to set, an orred value of cmzn_scene_viewer_input_modifier_flags.
 * @return  CMZN_OK if successful, any other value on failure.
 */
ZINC_API int cmzn_scene_viewer_input_set_modifier(cmzn_scene_viewer_input_id input, cmzn_scene_viewer_input_modifier modifier);

/**
 * Set the button number for the input.
 *   1 ==> left mouse button.
 *   2 ==> middle mouse button.
 *   3 ==> right mouse button.
 *
 * @deprecated see cmzn_scene_viewer_input_set_button
 *
 * @param input  Handle to cmzn_scene_viewer_input object.
 * @param number  The numeric value of the button.
 * @return  CMZN_OK if successful, any other value on failure.
 */
ZINC_API int cmzn_scene_viewer_input_set_button_number(cmzn_scene_viewer_input_id input, int number);

/**
 * Set the button for the scene viewer input.
 *
 * @param input zinc scene viewer input.
 * @param button zinc scene viewer input button enumeration value.
 * @return CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_scene_viewer_input_set_button(cmzn_scene_viewer_input_id input, cmzn_scene_viewer_input_button_type button);

/**
 * Create a scene viewer input object for manually setting mouse or other input
 * event data.
 *
 * @param input  Handle to cmzn_scene_viewer_input object.
 * @param type  Enumerator for the input event type.
 * @return  CMZN_OK if successful, any other value on failure.
 */
ZINC_API int cmzn_scene_viewer_input_set_type(cmzn_scene_viewer_input_id input, cmzn_scene_viewer_input_event_type type);

#ifdef __cplusplus
}
#endif

#endif
