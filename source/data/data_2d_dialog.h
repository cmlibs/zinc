/*******************************************************************************
FILE : data_2d_dialog.h

LAST MODIFIED : 29 January 1999

DESCRIPTION :
This module creates a free data_2d_dialog input device, using two dof3,
two control and one input widget.  The position is given relative to some
coordinate system, and the returned value is a global one.
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
#if !defined (DATA_2D_DIALOG_H)
#define DATA_2D_DIALOG_H

#include "finite_element/finite_element.h"
#include "data/data_2d.h"

/*
Global types
------------
*/
enum data_2d_dialog_data_type
/*******************************************************************************
LAST MODIFIED : 22 February 1995

DESCRIPTION :
Contains the different types of data items that are valid for the
data_2d widget.
==============================================================================*/
{
	DATA_2D_DIALOG_UPDATE_CB,
	DATA_2D_DIALOG_SELECT_CB,
	DATA_2D_DIALOG_DESTROY_CB,
	DATA_2D_DIALOG_DATA,
	DATA_2D_DIALOG_SELECT_RADIUS
}; /* data_2d_dialog_data_type */

#define DATA_2D_DIALOG_NUM_CALLBACKS 3

/*
Global functions
----------------
*/
Widget create_data_2d_dialog(Widget *data_2d_dialog_widget,
	Widget parent,int dimension,
	struct MANAGER(FE_field) *fe_field_manager,
	struct MANAGER(FE_node) *node_manager,
	struct MANAGER(GROUP(FE_node)) *group_manager,
	struct GROUP(FE_node) *init_data,DATA_2D_PRECISION select_radius);
/*******************************************************************************
LAST MODIFIED : 29 January 1999

DESCRIPTION :
Creates a dialog widget that allows the user to edit the properties of any of
the materials contained in the global list.
==============================================================================*/

int data_2d_dialog_set_data(Widget data_2d_dialog_widget,
	enum data_2d_dialog_data_type data_type,void *data);
/*******************************************************************************
LAST MODIFIED : 2 April 1995

DESCRIPTION :
If <data_2d_dialog_widget> is not NULL, then change the data item on
<data_2d_dialog widget>.  Otherwise, change the data item on
<data_2d_dialog>.
==============================================================================*/

void *data_2d_dialog_get_data(Widget data_2d_dialog_widget,
	enum data_2d_dialog_data_type data_type);
/*******************************************************************************
LAST MODIFIED : 2 April 1995

DESCRIPTION :
If <data_2d_dialog_widget> is not NULL, then get the data item from
<data_2d_dialog widget>.  Otherwise, get the data item from
<data_2d_dialog>.
==============================================================================*/

int data_2d_dialog_selection(Widget data_2d_dialog_widget,DATA_2D_PRECISION *data,
	int button_down,int single);
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Notifies the grabber about the selection process.
==============================================================================*/

#endif
