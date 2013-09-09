/***************************************************************************//**
 * FILE : selection.h
 */
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef CMZN_SELECTION_H__
#define CMZN_SELECTION_H__

#include "types/selectionid.h"

#include "zinc/zincsharedobject.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*cmzn_selection_handler_callback_function)(
	cmzn_selection_event_id selection_event,	void *client_data);

enum cmzn_selection_change_type
{
	CMZN_SELECTION_NO_CHANGE = 0,
	CMZN_SELECTION_CLEAR = 1,     /*!< selection is empty, but wasn't before */
	CMZN_SELECTION_ADD = 2,       /*!< objects have been added only */
	CMZN_SELECTION_REMOVE = 3,    /*!< objects have been removed only */
	CMZN_SELECTION_REPLACE = 4   /*!< contents replaced: clear+add, add+remove */
};

/***************************************************************************//**
 * Return an allocated short name of the enum type from the provided enum.
 * User must call cmzn_deallocate to destroy the successfully returned string.
 *
 * @param type  enum to be converted into string
 * @return  an allocated string which stored the short name of the enum.
 */
ZINC_API char *cmzn_selection_event_type_enum_to_string(enum cmzn_selection_change_type type);

/***************************************************************************//**
* Returns a new reference to the selection_handler with reference count incremented.
* Caller is responsible for destroying the new reference.
*
* @param selection_handler  The selection_handler to obtain a new reference to.
* @return  New selection_handler reference with incremented reference count.
*/
ZINC_API cmzn_selection_handler_id cmzn_selection_handler_access(
	cmzn_selection_handler_id selection_handler);

/***************************************************************************//**
 * Destroys this reference to the selection handler (and sets it to NULL).
 * Internally this just decrements the reference count.
 *
 * @param selection_handler  Handle to the zinc selection_handler.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_selection_handler_destroy(cmzn_selection_handler_id *selection_handler);

/***************************************************************************//**
 * Stop and clear selection callback. This will stop the callback and also
 * remove the callback function from the seleciton handler.
 *
 * @param selection_handler  Handle to the zinc selection_handler.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_selection_handler_clear_callback(cmzn_selection_handler_id selection_handler);

/***************************************************************************//**
 * Assign the callback function and user_data for the selection handler.
 * This function also starts the callback.
 *
 * @see cmzn_selection_handler_callback_function
 * @param selection_handler  Handle to the selection_handler.
 * @param function  function to be called when event is triggered.
 * @param user_data_in  Void pointer to an user object. User is responsible for
 *   the life time of such object.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_selection_handler_set_callback(cmzn_selection_handler_id selection_handler,
		cmzn_selection_handler_callback_function function, void *user_data_in);

/***************************************************************************//**
* Returns a new reference to the selection_event with reference count incremented.
* Caller is responsible for destroying the new reference.
*
* @param selection_event  The selection_event to obtain a new reference to.
* @return  New selection_handler reference with incremented reference count.
*/
ZINC_API cmzn_selection_event_id cmzn_selection_event_access(
	cmzn_selection_event_id selection_event);

/***************************************************************************//**
 * Destroys this reference to the selection event (and sets it to NULL).
 * Internally this just decrements the reference count.
 *
 * Note: At the end of the cmzn_selection_handler_callback_function, the caller
 * will destroy the event argument so users do not need to call this destroy
 * function unless, an additional reference count has been added through
 * cmzn_selection_event_access function.
 *
 * @param selection_event  Handle to the zinc selection_handler.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_selection_event_destroy(cmzn_selection_event_id *selection_event_address);

/***************************************************************************//**
 * Get the change type of the correct selection. This provides detail of the
 * selection changes.
 * @see cmzn_selection_change_type
 *
 * @param selection_event  Handle to the selection_event.
 * @return  cmzn_selection_change_type  The type of recent changes.
 */
ZINC_API enum cmzn_selection_change_type cmzn_selection_event_get_change_type(
	cmzn_selection_event_id selection_event);

/***************************************************************************//**
 * Inquire either the selection handler will trigger event when selection
 * of the local regions and its childrens scene has changed or only trigger
 * event with selection changes of the local region scene.
 *
 * @param selection_event  Handle to the selection_event.
 * @return  1 if selection handler triggers hierarchical changes, 0 otherwise.
 */
ZINC_API int cmzn_selection_handler_get_hierarchical(cmzn_selection_handler_id selection_handler);

/***************************************************************************//**
 * Set either the selection handler will trigger event when selection
 * of the local regions and its childrens scene has changed or only trigger
 * event with selection changes of the local region scene.
 *
 * @param selection_event  Handle to the selection_event.
 * @param hierarchical_flag  flag to be set.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_selection_handler_set_hierarchical(cmzn_selection_handler_id selection_handler,
	int hierarchical_flag);

/***************************************************************************//**
 * User can use this function to inquire either the scene owning the
 * selection handler triggering this event has been destroyed. If it is destroyed,
 * no more event callback will be triggered after the current one.
 *
 * @param selection_event  Handle to the selection_event.
 * @return  1 if scene is destroyed, 0 if not.
 */
ZINC_API int cmzn_selection_event_owning_scene_is_destroyed(
	cmzn_selection_event_id selection_event);

#ifdef __cplusplus
}
#endif

#endif /* CMZN_SELECTION_H__ */
