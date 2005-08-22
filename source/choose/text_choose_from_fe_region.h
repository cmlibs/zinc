/*******************************************************************************
FILE : text_choose_from_fe_region.h

LAST MODIFIED : 19 March 2003

DESCRIPTION :
Macros for implementing an option menu dialog control for choosing an object
from its manager. Handles manager messages to keep the menu up-to-date.
Calls the client-specified callback routine if a different object is chosen.
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
#if !defined (TEXT_CHOOSE_FROM_FE_REGION_H)
#define TEXT_CHOOSE_FROM_FE_REGION_H

#include <Xm/Xm.h>
#include "general/callback_motif.h"

/*
Global Types
------------
*/

/*
Global Functions
---------------
*/
#if ! defined (SHORT_NAMES)
#define CREATE_TEXT_CHOOSE_FROM_FE_REGION_WIDGET_( object_type ) \
	create_text_choose_from_fe_region_widget_ ## object_type
#else
#define CREATE_TEXT_CHOOSE_FROM_FE_REGION_WIDGET_( object_type ) ctcow ## object_type
#endif
#define CREATE_TEXT_CHOOSE_FROM_FE_REGION_WIDGET( object_type ) \
	CREATE_TEXT_CHOOSE_FROM_FE_REGION_WIDGET_(object_type)

#define PROTOTYPE_CREATE_TEXT_CHOOSE_FROM_FE_REGION_WIDGET_FUNCTION( object_type ) \
Widget CREATE_TEXT_CHOOSE_FROM_FE_REGION_WIDGET(object_type)(Widget parent, \
	struct object_type *current_object, \
	struct FE_region *fe_region, \
	LIST_CONDITIONAL_FUNCTION(object_type) *conditional_function, \
	void *conditional_function_user_data, \
	int (object_to_string)(struct object_type *,char **), \
	struct object_type *(string_to_object)(struct FE_region *fe_region, char *)) \
/***************************************************************************** \
LAST MODIFIED : 19 March 2003 \
\
DESCRIPTION : \
Creates a text box from which an object from fe_regions may be entered. \
The optional <conditional_function> allows a selection of the objects in the \
manager to be valid. If it is NULL, all objects in the manager are usable. \
The <object_to_string> function must be supplied, and builds a string to \
describe the identifier of the object. The <string_to_from_fe_region> function does \
the reverse. These functions are useful in the case of elements where only \
the element number (or face or line) is to be specified, not all three. \
============================================================================*/

#if ! defined (SHORT_NAMES)
#define TEXT_CHOOSE_FROM_FE_REGION_GET_CALLBACK_( object_type ) \
	text_choose_from_fe_region_get_callback_ ## object_type
#else
#define TEXT_CHOOSE_FROM_FE_REGION_GET_CALLBACK_( object_type ) cogc ## object_type
#endif
#define TEXT_CHOOSE_FROM_FE_REGION_GET_CALLBACK( object_type ) \
	TEXT_CHOOSE_FROM_FE_REGION_GET_CALLBACK_(object_type)

#define PROTOTYPE_TEXT_CHOOSE_FROM_FE_REGION_GET_CALLBACK_FUNCTION( object_type ) \
struct Callback_data *TEXT_CHOOSE_FROM_FE_REGION_GET_CALLBACK(object_type)( \
	Widget text_choose_object_widget) \
/***************************************************************************** \
LAST MODIFIED : 28 January 2003 \
\
DESCRIPTION : \
Returns a pointer to the callback item of the text_choose_from_fe_region_widget. \
============================================================================*/

#if ! defined (SHORT_NAMES)
#define TEXT_CHOOSE_FROM_FE_REGION_SET_CALLBACK_( object_type ) \
	text_choose_from_fe_region_set_callback_ ## object_type
#else
#define TEXT_CHOOSE_FROM_FE_REGION_SET_CALLBACK_( object_type ) cosc ## object_type
#endif
#define TEXT_CHOOSE_FROM_FE_REGION_SET_CALLBACK( object_type ) \
	TEXT_CHOOSE_FROM_FE_REGION_SET_CALLBACK_(object_type)

#define PROTOTYPE_TEXT_CHOOSE_FROM_FE_REGION_SET_CALLBACK_FUNCTION( object_type ) \
int TEXT_CHOOSE_FROM_FE_REGION_SET_CALLBACK(object_type)( \
	Widget text_choose_object_widget,struct Callback_data *new_callback) \
/***************************************************************************** \
LAST MODIFIED : 28 January 2003 \
\
DESCRIPTION : \
Changes the callback item of the text_choose_from_fe_region_widget. \
============================================================================*/

#if ! defined (SHORT_NAMES)
#define TEXT_CHOOSE_FROM_FE_REGION_GET_OBJECT_( object_type ) \
	text_choose_from_fe_region_get_object_ ## object_type
#else
#define TEXT_CHOOSE_FROM_FE_REGION_GET_OBJECT_( object_type ) cogo ## object_type
#endif
#define TEXT_CHOOSE_FROM_FE_REGION_GET_OBJECT( object_type ) \
	TEXT_CHOOSE_FROM_FE_REGION_GET_OBJECT_(object_type)

#define PROTOTYPE_TEXT_CHOOSE_FROM_FE_REGION_GET_OBJECT_FUNCTION( object_type ) \
struct object_type *TEXT_CHOOSE_FROM_FE_REGION_GET_OBJECT(object_type)( \
	Widget text_choose_object_widget) \
/***************************************************************************** \
LAST MODIFIED : 28 January 2003 \
\
DESCRIPTION : \
Returns the currently chosen object in the text_choose_from_fe_region_widget. \
============================================================================*/

#if ! defined (SHORT_NAMES)
#define TEXT_CHOOSE_FROM_FE_REGION_SET_OBJECT_( object_type ) \
	text_choose_from_fe_region_set_object_ ## object_type
#else
#define TEXT_CHOOSE_FROM_FE_REGION_SET_OBJECT_( object_type ) coso ## object_type
#endif
#define TEXT_CHOOSE_FROM_FE_REGION_SET_OBJECT( object_type ) \
	TEXT_CHOOSE_FROM_FE_REGION_SET_OBJECT_(object_type)

#define PROTOTYPE_TEXT_CHOOSE_FROM_FE_REGION_SET_OBJECT_FUNCTION( object_type ) \
int TEXT_CHOOSE_FROM_FE_REGION_SET_OBJECT(object_type)( \
	Widget text_choose_object_widget,struct object_type *new_object) \
/***************************************************************************** \
LAST MODIFIED : 28 January 2003 \
\
DESCRIPTION : \
Changes the chosen object in the text_choose_from_fe_region_widget. \
============================================================================*/

#if ! defined (SHORT_NAMES)
#define TEXT_CHOOSE_FROM_FE_REGION_CHANGE_CONDITIONAL_FUNCTION_( object_type ) \
	text_choose_from_fe_region_change_conditional_function_ ## object_type
#else
#define TEXT_CHOOSE_FROM_FE_REGION_CHANGE_CONDITIONAL_FUNCTION_( object_type ) \
	tcoccf ## object_type
#endif
#define TEXT_CHOOSE_FROM_FE_REGION_CHANGE_CONDITIONAL_FUNCTION( object_type ) \
	TEXT_CHOOSE_FROM_FE_REGION_CHANGE_CONDITIONAL_FUNCTION_(object_type)

#define PROTOTYPE_TEXT_CHOOSE_FROM_FE_REGION_CHANGE_CONDITIONAL_FUNCTION_FUNCTION( \
	object_type ) \
int TEXT_CHOOSE_FROM_FE_REGION_CHANGE_CONDITIONAL_FUNCTION(object_type)( \
	Widget text_choose_object_widget, \
	LIST_CONDITIONAL_FUNCTION(object_type) *conditional_function, \
	void *conditional_function_user_data,struct object_type *new_object) \
/***************************************************************************** \
LAST MODIFIED : 28 January 2003 \
\
DESCRIPTION : \
Changes the conditional_function and user_data limiting the available \
selection of objects. Also allows new_object to be set simultaneously. \
============================================================================*/

#define PROTOTYPE_TEXT_CHOOSE_FROM_FE_REGION_GLOBAL_FUNCTIONS( object_type) \
PROTOTYPE_CREATE_TEXT_CHOOSE_FROM_FE_REGION_WIDGET_FUNCTION(object_type); \
PROTOTYPE_TEXT_CHOOSE_FROM_FE_REGION_GET_CALLBACK_FUNCTION(object_type); \
PROTOTYPE_TEXT_CHOOSE_FROM_FE_REGION_SET_CALLBACK_FUNCTION(object_type); \
PROTOTYPE_TEXT_CHOOSE_FROM_FE_REGION_GET_OBJECT_FUNCTION(object_type); \
PROTOTYPE_TEXT_CHOOSE_FROM_FE_REGION_SET_OBJECT_FUNCTION(object_type); \
PROTOTYPE_TEXT_CHOOSE_FROM_FE_REGION_CHANGE_CONDITIONAL_FUNCTION_FUNCTION(object_type)

#endif /* !defined (TEXT_CHOOSE_FROM_FE_REGION_H) */
