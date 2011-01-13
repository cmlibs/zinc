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

#ifndef CMISS_SELECTION_HANDLER_ID_DEFINED
struct Cmiss_selection_handler;
typedef struct Cmiss_selection_handler * Cmiss_selection_handler_id;
#define CMISS_SELECTION_HANDLER_ID_DEFINED
#endif
#ifndef CMISS_SELECTION_EVENT_ID_DEFINED
struct Cmiss_selection_event;
typedef struct Cmiss_selection_event * Cmiss_selection_event_id;
#define CMISS_SELECTION_EVENT_ID_DEFINED
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

enum Cmiss_selection_change_type Cmiss_selection_event_get_change_detail(
	Cmiss_selection_event_id selection_event);

/***************************************************************************//**
 * Assign the callback function and user_data for the selection handler.
 * This function does not start the callback.
 */
int Cmiss_selection_handler_set_callback(Cmiss_selection_handler_id selection_handler,
		Cmiss_selection_handler_callback_function function, void *user_data_in);

int Cmiss_selection_handler_get_hierarchical(Cmiss_selection_handler_id selection_handler);

int Cmiss_selection_handler_set_hierarchical(Cmiss_selection_handler_id selection_handler,
	int hierarchical_flag);

/***************************************************************************//**
 * Stop and clear selection callback.
 */
int Cmiss_selection_handler_clear_callback(Cmiss_selection_handler_id selection_handler);

int Cmiss_selection_handler_destroy(Cmiss_selection_handler_id *selection_handler);

#endif /* __CMISS_SELECTION_H__ */
