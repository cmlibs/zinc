/*******************************************************************************
FILE : choose_field_component.h

LAST MODIFIED : 21 November 2001

DESCRIPTION :
Specialized chooser widget that allows a component of an FE_field to be
selected from an option menu.
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
#if !defined (CHOOSE_FIELD_COMPONENT_H)
#define CHOOSE_FIELD_COMPONENT_H

#include <Xm/Xm.h>
#include "finite_element/finite_element.h"
#include "general/callback_motif.h"
#include "user_interface/user_interface.h"

/*
Global Types
------------
*/

/*
Global Functions
---------------
*/

Widget create_choose_field_component_widget(Widget parent,
	struct FE_field *field, int component_no,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 21 November 2001

DESCRIPTION :
Creates an option menu from which a component of the <field> may be chosen,
initially with the given <component_no>.
Note: Choose_field_component will be automatically DESTROYed with its widgets.
<user_interface> supplies fonts.
==============================================================================*/

struct Callback_data *choose_field_component_get_callback(
	Widget choose_field_component_widget);
/*****************************************************************************
LAST MODIFIED : 9 February 2000

DESCRIPTION :
Returns a pointer to the callback item of the choose_field_component_widget.
============================================================================*/

int choose_field_component_set_callback(Widget choose_field_component_widget,
	struct Callback_data *new_callback);
/*****************************************************************************
LAST MODIFIED : 9 February 2000

DESCRIPTION :
Changes the callback item of the choose_field_component_widget.
============================================================================*/

int choose_field_component_get_field_component(
	Widget choose_field_component_widget,struct FE_field **field,
	int *component_no);
/*****************************************************************************
LAST MODIFIED : 9 February 2000

DESCRIPTION :
Returns the current field and component number in the
choose_field_component_widget.
============================================================================*/

int choose_field_component_set_field_component(
	Widget choose_field_component_widget,
	struct FE_field *field,int component_no);
/*****************************************************************************
LAST MODIFIED : 9 February 2000

DESCRIPTION :
Changes the field component in the choose_field_component_widget.
============================================================================*/
#endif /* !defined (CHOOSE_FIELD_COMPONENT_H) */
