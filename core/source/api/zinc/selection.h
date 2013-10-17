/**
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

typedef void (*cmzn_selectionnotifier_callback_function)(
	cmzn_selectionevent_id selection_event,	void *client_data);

enum cmzn_selectionevent_change_type
{
	CMZN_SELECTIONEVENT_CHANGE_NONE = 0,
	CMZN_SELECTIONEVENT_CHANGE_CLEAR = 1,     /*!< selection is empty, but wasn't before */
	CMZN_SELECTIONEVENT_CHANGE_ADD = 2,       /*!< objects have been added only */
	CMZN_SELECTIONEVENT_CHANGE_REMOVE = 3,    /*!< objects have been removed only */
	CMZN_SELECTIONEVENT_CHANGE_REPLACE = 4    /*!< contents replaced: clear+add, add+remove */
};

/**
 * Return an allocated short name of the enum type from the provided enum.
 * User must call cmzn_deallocate to destroy the successfully returned string.
 *
 * @param type  enum to be converted into string
 * @return  an allocated string which stored the short name of the enum.
 */
ZINC_API char *cmzn_selectionevent_change_type_enum_to_string(enum cmzn_selectionevent_change_type type);

/**
* Returns a new reference to the selection notifier with reference count incremented.
* Caller is responsible for destroying the new reference.
*
* @param selectionnotifier  The selection notifier to obtain a new reference to.
* @return  New selectionnotifier reference with incremented reference count.
*/
ZINC_API cmzn_selectionnotifier_id cmzn_selectionnotifier_access(
	cmzn_selectionnotifier_id selectionnotifier);

/**
 * Destroys this reference to the selection notifier (and sets it to NULL).
 * Internally this just decrements the reference count.
 *
 * @param selectionnotifier  Handle to the selection notifier.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_selectionnotifier_destroy(cmzn_selectionnotifier_id *selectionnotifier);

/**
 * Stop and clear selection callback. This will stop the callback and also
 * remove the callback function from the selection notifier.
 *
 * @param selectionnotifier  Handle to the selection notifier.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_selectionnotifier_clear_callback(cmzn_selectionnotifier_id selectionnotifier);

/**
 * Assign the callback function and user_data for the selection notifier.
 * This function also starts the callback.
 *
 * @see cmzn_selectionnotifier_callback_function
 * @param selectionnotifier  Handle to the selection notifier.
 * @param function  function to be called when event is triggered.
 * @param user_data_in  Void pointer to an user object. User is responsible for
 *   the life time of such object.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_selectionnotifier_set_callback(cmzn_selectionnotifier_id selectionnotifier,
		cmzn_selectionnotifier_callback_function function, void *user_data_in);

/**
* Returns a new reference to the selection_event with reference count incremented.
* Caller is responsible for destroying the new reference.
*
* @param selection_event  The selection_event to obtain a new reference to.
* @return  New selectionnotifier reference with incremented reference count.
*/
ZINC_API cmzn_selectionevent_id cmzn_selectionevent_access(
	cmzn_selectionevent_id selection_event);

/**
 * Destroys this reference to the selection event (and sets it to NULL).
 * Internally this just decrements the reference count.
 *
 * Note: At the end of the cmzn_selectionnotifier_callback_function, the caller
 * will destroy the event argument so users do not need to call this destroy
 * function unless, an additional reference count has been added through
 * cmzn_selectionevent_access function.
 *
 * @param selection_event  Handle to the selection notifier.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_selectionevent_destroy(cmzn_selectionevent_id *selection_event_address);

/**
 * Get the change type of the correct selection. This provides detail of the
 * selection changes.
 * @see cmzn_selectionevent_change_type
 *
 * @param selection_event  Handle to the selection_event.
 * @return  cmzn_selectionevent_change_type  The type of recent changes.
 */
ZINC_API enum cmzn_selectionevent_change_type cmzn_selectionevent_get_change_type(
	cmzn_selectionevent_id selection_event);

/**
 * Get whether receiving selection events from child regions in addition to the
 * local region.
 *
 * @param selectionnotifier  The selection notifier to query.
 * @return  Boolean true if receiving hierarchical events, false if not.
 */
ZINC_API bool cmzn_selectionnotifier_is_hierarchical(
	cmzn_selectionnotifier_id selectionnotifier);

/**
 * Set whether to handle selection events from child regions in addition to the
 * local region.
 *
 * @param selectionnotifier  The selection notifier to modify.
 * @param hierarchical  New value of flag: true = hierarchical, false = not.
 * @return  Status CMZN_OK on success, otherwise CMZN_ERROR_ARGUMENT.
 */
ZINC_API int cmzn_selectionnotifier_set_hierarchical(
	cmzn_selectionnotifier_id selectionnotifier, bool hierarchical);

/**
 * User can use this function to inquire either the scene owning the
 * selection notifier triggering this event has been destroyed. If it is destroyed,
 * no more event callback will be triggered after the current one.
 *
 * @param selection_event  Handle to the selection_event.
 * @return  1 if scene is destroyed, 0 if not.
 */
ZINC_API int cmzn_selectionevent_owning_scene_is_destroyed(
	cmzn_selectionevent_id selection_event);

#ifdef __cplusplus
}
#endif

#endif /* CMZN_SELECTION_H__ */
