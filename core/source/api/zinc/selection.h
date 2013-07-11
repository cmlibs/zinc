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
 * Portions created by the Initial Developer are Copyright (C) 2010
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

#if !defined __CMISS_SELECTION_H__
#define __CMISS_SELECTION_H__

#include "types/selectionid.h"

#include "zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*Cmiss_selection_handler_callback_function)(
	Cmiss_selection_event_id selection_event,	void *client_data);

enum Cmiss_selection_change_type
{
	CMISS_SELECTION_NO_CHANGE = 0,
	CMISS_SELECTION_CLEAR = 1,     /*!< selection is empty, but wasn't before */
	CMISS_SELECTION_ADD = 2,       /*!< objects have been added only */
	CMISS_SELECTION_REMOVE = 3,    /*!< objects have been removed only */
	CMISS_SELECTION_REPLACE = 4   /*!< contents replaced: clear+add, add+remove */
};

/***************************************************************************//**
 * Return an allocated short name of the enum type from the provided enum.
 * User must call Cmiss_deallocate to destroy the successfully returned string.
 *
 * @param type  enum to be converted into string
 * @return  an allocated string which stored the short name of the enum.
 */
ZINC_API char *Cmiss_selection_event_type_enum_to_string(enum Cmiss_selection_change_type type);

/***************************************************************************//**
* Returns a new reference to the selection_handler with reference count incremented.
* Caller is responsible for destroying the new reference.
*
* @param selection_handler  The selection_handler to obtain a new reference to.
* @return  New selection_handler reference with incremented reference count.
*/
ZINC_API Cmiss_selection_handler_id Cmiss_selection_handler_access(
	Cmiss_selection_handler_id selection_handler);

/***************************************************************************//**
 * Destroys this reference to the selection handler (and sets it to NULL).
 * Internally this just decrements the reference count.
 *
 * @param selection_handler  Handle to the cmiss selection_handler.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_selection_handler_destroy(Cmiss_selection_handler_id *selection_handler);

/***************************************************************************//**
 * Stop and clear selection callback. This will stop the callback and also
 * remove the callback function from the seleciton handler.
 *
 * @param selection_handler  Handle to the cmiss selection_handler.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_selection_handler_clear_callback(Cmiss_selection_handler_id selection_handler);

/***************************************************************************//**
 * Assign the callback function and user_data for the selection handler.
 * This function also starts the callback.
 *
 * @see Cmiss_selection_handler_callback_function
 * @param selection_handler  Handle to the cmiss_selection_handler.
 * @param function  function to be called when event is triggered.
 * @param user_data_in  Void pointer to an user object. User is responsible for
 *   the life time of such object.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_selection_handler_set_callback(Cmiss_selection_handler_id selection_handler,
		Cmiss_selection_handler_callback_function function, void *user_data_in);

/***************************************************************************//**
* Returns a new reference to the selection_event with reference count incremented.
* Caller is responsible for destroying the new reference.
*
* @param selection_event  The selection_event to obtain a new reference to.
* @return  New selection_handler reference with incremented reference count.
*/
ZINC_API Cmiss_selection_event_id Cmiss_selection_event_access(
	Cmiss_selection_event_id selection_event);

/***************************************************************************//**
 * Destroys this reference to the selection event (and sets it to NULL).
 * Internally this just decrements the reference count.
 *
 * Note: At the end of the Cmiss_selection_handler_callback_function, the caller
 * will destroy the event argument so users do not need to call this destroy
 * function unless, an additional reference count has been added through
 * Cmiss_selection_event_access function.
 *
 * @param selection_event  Handle to the cmiss selection_handler.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_selection_event_destroy(Cmiss_selection_event_id *selection_event_address);

/***************************************************************************//**
 * Get the change type of the correct selection. This provides detail of the
 * selection changes.
 * @see Cmiss_selection_change_type
 *
 * @param selection_event  Handle to the cmiss_selection_event.
 * @return  Cmiss_selection_change_type  The type of recent changes.
 */
ZINC_API enum Cmiss_selection_change_type Cmiss_selection_event_get_change_type(
	Cmiss_selection_event_id selection_event);

/***************************************************************************//**
 * Inquire either the selection handler will trigger event when selection
 * of the local regions and its childrens scene has changed or only trigger
 * event with selection changes of the local region scene.
 *
 * @param selection_event  Handle to the cmiss_selection_event.
 * @return  1 if selection handler triggers hierarchical changes, 0 otherwise.
 */
ZINC_API int Cmiss_selection_handler_get_hierarchical(Cmiss_selection_handler_id selection_handler);

/***************************************************************************//**
 * Set either the selection handler will trigger event when selection
 * of the local regions and its childrens scene has changed or only trigger
 * event with selection changes of the local region scene.
 *
 * @param selection_event  Handle to the cmiss_selection_event.
 * @param hierarchical_flag  flag to be set.
 * @return  Status CMISS_OK on success, any other value on failure.
 */
ZINC_API int Cmiss_selection_handler_set_hierarchical(Cmiss_selection_handler_id selection_handler,
	int hierarchical_flag);

/***************************************************************************//**
 * User can use this function to inquire either the scene owning the
 * selection handler triggering this event has been destroyed. If it is destroyed,
 * no more event callback will be triggered after the current one.
 *
 * @param selection_event  Handle to the cmiss_selection_event.
 * @return  1 if scene is destroyed, 0 if not.
 */
ZINC_API int Cmiss_selection_event_owning_scene_is_destroyed(
	Cmiss_selection_event_id selection_event);

#ifdef __cplusplus
}
#endif

#endif /* __CMISS_SELECTION_H__ */
