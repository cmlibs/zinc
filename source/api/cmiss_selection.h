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

#include "api/types/cmiss_selection_id.h"

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
* Returns a new reference to the selection_handler with reference count incremented.
* Caller is responsible for destroying the new reference.
*
* @param selection_handler  The selection_handler to obtain a new reference to.
* @return  New selection_handler reference with incremented reference count.
*/
Cmiss_selection_handler_id Cmiss_selection_handler_access(
	Cmiss_selection_handler_id selection_handler);

/***************************************************************************//**
 * Destroys this reference to the tessellation (and sets it to NULL).
 * Internally this just decrements the reference count.
 *
 * @param selection_handler  Handle to the cmiss selection_handler.
 * @return  0 on failure, 1 on success.
 */
int Cmiss_selection_handler_destroy(Cmiss_selection_handler_id *selection_handler);

/***************************************************************************//**
 * Stop and clear selection callback. This will stop the callback and also
 * remove the callback function from the seleciton handler.
 *
 * @param selection_handler  Handle to the cmiss selection_handler.
 * @return  0 on failure, 1 on success.
 */
int Cmiss_selection_handler_clear_callback(Cmiss_selection_handler_id selection_handler);

/***************************************************************************//**
 * Assign the callback function and user_data for the selection handler.
 * This function also starts the callback.
 *
 * @see Cmiss_selection_handler_callback_function
 * @param selection_event  Handle to the cmiss_selection_event.
 * @param function  function to be called when event is triggered.
 * @param user_data_in  Void pointer to an user object. User is responsible for
 *   the life time of such object.
 * @return  1 on success, 0 on failure.
 */
int Cmiss_selection_handler_set_callback(Cmiss_selection_handler_id selection_handler,
		Cmiss_selection_handler_callback_function function, void *user_data_in);

/***************************************************************************//**
 * Get the change type of the correct selection. This provides detail of the
 * selection changes.
 * @see Cmiss_selection_change_type
 *
 * @param selection_event  Handle to the cmiss_selection_event.
 * @return  Cmiss_selection_change_type  The type of recent changes.
 */
enum Cmiss_selection_change_type Cmiss_selection_event_get_change_type(
	Cmiss_selection_event_id selection_event);

/***************************************************************************//**
 * Inquire either the selection handler will trigger event when selection
 * of the local regions and its childrens rendition has changed or only trigger
 * event with selection changes of the local region rendition.
 *
 * @param selection_event  Handle to the cmiss_selection_event.
 * @return  1 if selection handler triggers hierarchical changes, 0 otherwise.
 */
int Cmiss_selection_handler_get_hierarchical(Cmiss_selection_handler_id selection_handler);

/***************************************************************************//**
 * Set either the selection handler will trigger event when selection
 * of the local regions and its childrens rendition has changed or only trigger
 * event with selection changes of the local region rendition.
 *
 * @param selection_event  Handle to the cmiss_selection_event.
 * @param hierarchical_flag  flag to be set.
 * @return  0 on failure, 1 on success.
 */
int Cmiss_selection_handler_set_hierarchical(Cmiss_selection_handler_id selection_handler,
	int hierarchical_flag);

/***************************************************************************//**
 * User can use this function to inquire either the rendition owning the
 * selection handler triggering this event has been destroyed. If it is destroyed,
 * no more event callback will be triggered after the current one.
 *
 * @param selection_event  Handle to the cmiss_selection_event.
 * @return  1 if rendition is destroyed, 0 if not.
 */
int Cmiss_selection_event_owning_rendition_is_destroyed(
	Cmiss_selection_event_id selection_event);

#endif /* __CMISS_SELECTION_H__ */
