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
	cmzn_selectionevent_id selectionevent,	void *client_data);

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
* Returns a new reference to the selection event with reference count incremented.
* Caller is responsible for destroying the new reference.
*
* @param selectionevent  The selection event to obtain a new reference to.
* @return  New selectionnotifier reference with incremented reference count.
*/
ZINC_API cmzn_selectionevent_id cmzn_selectionevent_access(
	cmzn_selectionevent_id selectionevent);

/**
 * Destroys this reference to the selection event (and sets it to NULL).
 * Internally this just decrements the reference count.
 *
 * Note: At the end of the cmzn_selectionnotifier_callback_function, the caller
 * will destroy the event argument so users do not need to call this destroy
 * function unless, an additional reference count has been added through
 * cmzn_selectionevent_access function.
 *
 * @param selectionevent_address  Address of selection event handle to destroy.
 * @return  Status CMZN_OK on success, any other value on failure.
 */
ZINC_API int cmzn_selectionevent_destroy(cmzn_selectionevent_id *selectionevent_address);

/**
 * Get logical OR of flags indicating how the selection has changed, whether in
 * the local region or hierarchically.
 * @see cmzn_selectionevent_change_flags
 *
 * @param selectionevent  Handle to the selection event.
 * @return  The local change type flags.
 */
ZINC_API int cmzn_selectionevent_get_change_summary(
	cmzn_selectionevent_id selectionevent);

#ifdef __cplusplus
}
#endif

#endif /* CMZN_SELECTION_H__ */
