/*******************************************************************************
FILE : edit_var.h

LAST MODIFIED : 29 November 1997

DESCRIPTION :
Creates a widget that contains a description of a variable, and means to edit
it via a slider or text field.
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
#if !defined (EDIT_VAR_H)
#define EDIT_VAR_H

#include <Xm/Xm.h>
#include "general/callback_motif.h"

#define EDIT_VAR_PRECISION double
#define EDIT_VAR_PRECISION_STRING "lf"
#define EDIT_VAR_NUM_FORMAT "%6.4" EDIT_VAR_PRECISION_STRING
#define EDIT_VAR_STRING_SIZE 100
/*
Global Types
------------
*/
enum Edit_var_data_type
/*******************************************************************************
LAST MODIFIED : 29 November 1997

DESCRIPTION :
Contains the different types of data stored with the edit_var control widget.
==============================================================================*/
{
	EDIT_VAR_VALUE,
	EDIT_VAR_LOW_LIMIT,
	EDIT_VAR_HIGH_LIMIT
}; /* Edit_var_data_type */

/*
Global Functions
---------------
*/
Widget create_edit_var_widget(Widget parent,char *description,
	EDIT_VAR_PRECISION init_data,EDIT_VAR_PRECISION low_limit,
	EDIT_VAR_PRECISION high_limit);
/*******************************************************************************
LAST MODIFIED : 23 January 1995

DESCRIPTION :
Allows the user to set the value of a variable between the limits low_limit
and high_limit.
==============================================================================*/

int edit_var_get_callback(Widget edit_var_widget,
	struct Callback_data *callback);
/*******************************************************************************
LAST MODIFIED : 29 November 1997

DESCRIPTION :
Returns the update_callback for the edit_var widget.
==============================================================================*/

int edit_var_set_callback(Widget edit_var_widget,
	struct Callback_data *callback);
/*******************************************************************************
LAST MODIFIED : 29 November 1997

DESCRIPTION :
Changes the update_callback for the edit_var widget.
==============================================================================*/

int edit_var_get_data(Widget edit_var_widget,
	enum Edit_var_data_type data_type,EDIT_VAR_PRECISION *data);
/*******************************************************************************
LAST MODIFIED : 29 November 1997

DESCRIPTION :
Returns a data item of the edit_var widget.
==============================================================================*/

int edit_var_set_data(Widget edit_var_widget,
	enum Edit_var_data_type data_type,EDIT_VAR_PRECISION data);
/*******************************************************************************
LAST MODIFIED : 29 November 1997

DESCRIPTION :
Changes a data item of the edit_var widget.
==============================================================================*/
#endif
