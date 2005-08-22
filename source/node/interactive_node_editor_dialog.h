/*******************************************************************************
FILE : interactive_node_editor_dialog.h

LAST MODIFIED : 1 March 1997

DESCRIPTION :
This module creates a free interactive_node_editor_dialog input device, using
two dof3, two control and one input widget.  The position is given relative to
some coordinate system, and the returned value is a global one.
==============================================================================*/
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
#if !defined (IN_EDITOR_DIALOG_H)
#define IN_EDITOR_DIALOG_H

#include "command/command.h"
#include "general/callback.h"
#include "finite_element/finite_element.h"

/*
UIL Identifiers
---------------
*/
/*???DB.  Why do these have to be global ? */
#define in_editor_dialog_select_form_ID 1
#define in_editor_dialog_editor_form_ID 2
#define in_editor_dialog_comfile_label_ID 3
#define in_editor_dialog_comfile_text_ID 4

/*
Global Types
------------
*/
enum Interactive_node_editor_dialog_data_type
/*******************************************************************************
LAST MODIFIED : 1 March 1997

DESCRIPTION :
Contains the different types of data items that are valid for the
interactive_node_editor widget.
==============================================================================*/
{
	IN_EDITOR_DIALOG_UPDATE_CB,
	IN_EDITOR_DIALOG_DATA
}; /* in_editor_dialog_data_type */

#define IN_EDITOR_DIALOG_NUM_CALLBACKS 1

struct Interactive_node_editor_dialog_struct
/*******************************************************************************
LAST MODIFIED : 21 July 1996

DESCRIPTION :
Contains all the information carried by the interactive_node_editor_dialog widget.
Note that we just hold a pointer to the interactive_node_editor_dialog, and must access
and deaccess it.
==============================================================================*/
{
	struct FE_node *current_value;
	struct MANAGER(FE_node) *manager;
	struct Callback_data callback_array[IN_EDITOR_DIALOG_NUM_CALLBACKS];
	char *command;
	Widget editor_form,editor_widget,select_form,select_widget;
	Widget comfile_label,comfile_text;
	Widget *dialog_address,dialog,widget,dialog_parent;
	struct Execute_command *execute_command;
}; /* in_editor_dialog_struct */

/*
Global Functions
----------------
*/
int in_editor_dialog_set_data(Widget in_editor_dialog_widget,
	enum Interactive_node_editor_dialog_data_type data_type,void *data);
/*******************************************************************************
LAST MODIFIED : 2 April 1995

DESCRIPTION :
If <in_editor_dialog_widget> is not NULL, then change the data item on
<interactive_node_editor_dialog widget>.  Otherwise, change the data item on
<interactive_node_editor_dialog>.
==============================================================================*/

void *in_editor_dialog_get_data(Widget in_editor_dialog_widget,
	enum Interactive_node_editor_dialog_data_type data_type);
/*******************************************************************************
LAST MODIFIED : 2 April 1995

DESCRIPTION :
If <in_editor_dialog_widget> is not NULL, then get the data item from
<interactive_node_editor_dialog widget>.  Otherwise, get the data item from
<interactive_node_editor_dialog>.
==============================================================================*/

int bring_up_interactive_node_editor_dialog(
	Widget *interactive_node_editor_dialog,Widget parent,
	struct MANAGER(FE_node) *node_manager,struct Execute_command *execute_command,
	struct FE_node *init_data);
/*******************************************************************************
LAST MODIFIED : 21 July 1995

DESCRIPTION :
If there is a material editor dialog in existence, then bring it to the front,
otherwise it creates a new one.  Assumes we will only ever want one material
editor at a time.  This implementation may be changed later.
==============================================================================*/
#endif
