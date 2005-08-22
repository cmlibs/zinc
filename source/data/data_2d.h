/*******************************************************************************
FILE : data_2d.h

LAST MODIFIED : 29 January 1999

DESCRIPTION :
Allows 2d digitisation of data from an arbitrary client.
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
#if !defined (DATA_2D_H)
#define DATA_2D_H

#include <Xm/Xm.h>
#include "general/callback_motif.h"
#include "finite_element/finite_element.h"

/*
Global Types
------------
*/
#define DATA_2D_PRECISION double
#define DATA_2D_PRECISION_STRING "lf"
#define DATA_2D_MAX_DIMENSION 3

enum D2_data_type
/*******************************************************************************
LAST MODIFIED : 20 February 1996

DESCRIPTION :
Contains the different types of callbacks that are valid for the data_2d
control widget.
==============================================================================*/
{
	DATA_2D_UPDATE_CB,
	DATA_2D_SELECT_CB,
	DATA_2D_DATA,
	DATA_2D_SELECT_RADIUS
}; /* DG_callback_type */

enum D2_mode
/*******************************************************************************
LAST MODIFIED : 20 February 1996

DESCRIPTION :
Contains the current editing mode of the widget.
==============================================================================*/
{
	DATA_2D_SELECT,
	DATA_2D_DIGITISE,
	DATA_2D_MOVE
}; /* DG_mode */

#define DATA_2D_NUM_CALLBACKS 2

struct Data_2d_modify_data
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Contains information required by the data_2d control dialog.
==============================================================================*/
{
	int number;
	DATA_2D_PRECISION *data;
}; /* Data_2d_modify_data */

struct Data_2d_select_data
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Contains information for the select callback.
==============================================================================*/
{
	int num_selected;
	int *selected_nodes;
}; /* Data_2d_select_data */

struct D2_struct
/*******************************************************************************
LAST MODIFIED : 23 March 1999

DESCRIPTION :
Contains information required by the data_2d control dialog.
==============================================================================*/
{
	DATA_2D_PRECISION button_down[DATA_2D_MAX_DIMENSION];
	DATA_2D_PRECISION button_up[DATA_2D_MAX_DIMENSION];
	void *manager_callback_id;
	void *group_manager_callback_id;
	struct FE_field *data_2d_field;
	struct GROUP(FE_node) *current_value;
	struct MANAGER(GROUP(FE_node)) *group_manager;
	struct MANAGER(FE_field) *fe_field_manager;
	struct MANAGER(FE_node) *node_manager;
	struct Callback_data callback_array[DATA_2D_NUM_CALLBACKS];
	DATA_2D_PRECISION select_radius;
	int dimension;
	enum D2_mode mode;
	Widget data_list,control,control_digitise,control_delete,control_move,
		mode_label;
	Widget widget_parent,widget,*widget_address;
}; /* DG_struct */

/*
UIL Identifiers
---------------
*/
#define data_2d_datalist_ID        1
#define data_2d_control_ID            2
#define data_2d_control_digitise_ID            3
#define data_2d_control_delete_ID            4
#define data_2d_control_move_ID            5
#define data_2d_mode_ID            6

/*
Global Functions
---------------
*/
Widget create_data_2d_widget(Widget *data_2d_widget,Widget parent,int dimension,
	struct MANAGER(FE_field) *fe_field_manager,
	struct MANAGER(FE_node) *node_manager,
	struct MANAGER(GROUP(FE_node)) *group_manager,
	struct GROUP(FE_node) *init_data,DATA_2D_PRECISION select_radius);
/*******************************************************************************
LAST MODIFIED : 29 January 1999

DESCRIPTION :
Creates a data_2d widget that will receive data from an arbitrary client and
handle transformations etc.
==============================================================================*/

int data_2d_set_data(Widget data_2d_widget,
	enum D2_data_type data_type,void *data);
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Changes a data item of the data_2d widget.
==============================================================================*/

void *data_2d_get_data(Widget data_2d_widget,
	enum D2_data_type data_type);
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Returns a pointer to a data item of the data_2d widget.
==============================================================================*/

int data_2d_selection(Widget data_2d_widget,DATA_2D_PRECISION *data,
	int button_down,int single);
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Notifies the grabber about the selection process.
==============================================================================*/
#endif
