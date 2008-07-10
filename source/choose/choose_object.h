/*******************************************************************************
FILE : choose_object.h

LAST MODIFIED : 21 November 2001

DESCRIPTION :
Macros for implementing an option menu dialog control for choosing an object
from its manager (subject to an optional conditional function). Handles manager
messages to keep the menu up-to-date.
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
#if !defined (CHOOSE_OBJECT_H)
#define CHOOSE_OBJECT_H

#include <Xm/Xm.h>
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

#if ! defined (SHORT_NAMES)
#define CREATE_CHOOSE_OBJECT_WIDGET_( object_type ) \
	create_choose_object_widget_ ## object_type
#else
#define CREATE_CHOOSE_OBJECT_WIDGET_( object_type ) ccow ## object_type
#endif
#define CREATE_CHOOSE_OBJECT_WIDGET( object_type ) \
	CREATE_CHOOSE_OBJECT_WIDGET_(object_type)

#define PROTOTYPE_CREATE_CHOOSE_OBJECT_WIDGET_FUNCTION( object_type ) \
Widget CREATE_CHOOSE_OBJECT_WIDGET(object_type)(Widget parent, \
	struct object_type *current_object, \
	struct MANAGER(object_type) *object_manager, \
	LIST_CONDITIONAL_FUNCTION(object_type) *conditional_function, \
	void *conditional_function_user_data, \
	struct User_interface *user_interface) \
/***************************************************************************** \
LAST MODIFIED : 21 November 2001 \
\
DESCRIPTION : \
Creates an option menu from which an object from the manager may be chosen. \
The optional conditional function permits a subset of objects in the manager \
to be selectable. \
<user_interface> supplies fonts. \
============================================================================*/

#if ! defined (SHORT_NAMES)
#define CHOOSE_OBJECT_GET_CALLBACK_( object_type ) \
	choose_object_get_callback_ ## object_type
#else
#define CHOOSE_OBJECT_GET_CALLBACK_( object_type ) cogc ## object_type
#endif
#define CHOOSE_OBJECT_GET_CALLBACK( object_type ) \
	CHOOSE_OBJECT_GET_CALLBACK_(object_type)

#define PROTOTYPE_CHOOSE_OBJECT_GET_CALLBACK_FUNCTION( object_type ) \
struct Callback_data *CHOOSE_OBJECT_GET_CALLBACK(object_type)( \
	Widget choose_object_widget) \
/***************************************************************************** \
LAST MODIFIED : 21 January 2000 \
\
DESCRIPTION : \
Returns a pointer to the callback item of the choose_object widget. \
============================================================================*/

#if ! defined (SHORT_NAMES)
#define CHOOSE_OBJECT_SET_CALLBACK_( object_type ) \
	choose_object_set_callback_ ## object_type
#else
#define CHOOSE_OBJECT_SET_CALLBACK_( object_type ) cosc ## object_type
#endif
#define CHOOSE_OBJECT_SET_CALLBACK( object_type ) \
	CHOOSE_OBJECT_SET_CALLBACK_(object_type)

#define PROTOTYPE_CHOOSE_OBJECT_SET_CALLBACK_FUNCTION( object_type ) \
int CHOOSE_OBJECT_SET_CALLBACK(object_type)( \
	Widget choose_object_widget,struct Callback_data *new_callback) \
/***************************************************************************** \
LAST MODIFIED : 21 January 2000 \
\
DESCRIPTION : \
Changes the callback item of the choose_object widget. \
============================================================================*/

#if ! defined (SHORT_NAMES)
#define CHOOSE_OBJECT_GET_OBJECT_( object_type ) \
	choose_object_get_object_ ## object_type
#else
#define CHOOSE_OBJECT_GET_OBJECT_( object_type ) cogo ## object_type
#endif
#define CHOOSE_OBJECT_GET_OBJECT( object_type ) \
	CHOOSE_OBJECT_GET_OBJECT_(object_type)

#define PROTOTYPE_CHOOSE_OBJECT_GET_OBJECT_FUNCTION( object_type ) \
struct object_type *CHOOSE_OBJECT_GET_OBJECT(object_type)( \
	Widget choose_object_widget) \
/***************************************************************************** \
LAST MODIFIED : 21 January 2000 \
\
DESCRIPTION : \
Returns the currently chosen object in the choose_object widget. \
============================================================================*/

#if ! defined (SHORT_NAMES)
#define CHOOSE_OBJECT_SET_OBJECT_( object_type ) \
	choose_object_set_object_ ## object_type
#else
#define CHOOSE_OBJECT_SET_OBJECT_( object_type ) coso ## object_type
#endif
#define CHOOSE_OBJECT_SET_OBJECT( object_type ) \
	CHOOSE_OBJECT_SET_OBJECT_(object_type)

#define PROTOTYPE_CHOOSE_OBJECT_SET_OBJECT_FUNCTION( object_type ) \
int CHOOSE_OBJECT_SET_OBJECT(object_type)( \
	Widget choose_object_widget,struct object_type *new_object) \
/***************************************************************************** \
LAST MODIFIED : 21 January 2000 \
\
DESCRIPTION : \
Changes the chosen object in the choose_object widget. \
============================================================================*/

#if ! defined (SHORT_NAMES)
#define CHOOSE_OBJECT_CHANGE_CONDITIONAL_FUNCTION_( object_type ) \
	choose_object_change_conditional_function_ ## object_type
#else
#define CHOOSE_OBJECT_CHANGE_CONDITIONAL_FUNCTION_( object_type ) \
	coccf ## object_type
#endif
#define CHOOSE_OBJECT_CHANGE_CONDITIONAL_FUNCTION( object_type ) \
	CHOOSE_OBJECT_CHANGE_CONDITIONAL_FUNCTION_(object_type)

#define PROTOTYPE_CHOOSE_OBJECT_CHANGE_CONDITIONAL_FUNCTION_FUNCTION( \
	object_type ) \
int CHOOSE_OBJECT_CHANGE_CONDITIONAL_FUNCTION(object_type)( \
	Widget choose_object_widget, \
	LIST_CONDITIONAL_FUNCTION(object_type) *conditional_function, \
	void *conditional_function_user_data,struct object_type *new_object) \
/***************************************************************************** \
LAST MODIFIED : 18 April 2000 \
\
DESCRIPTION : \
Changes the conditional_function and user_data limiting the available \
selection of objects. Also allows new_object to be set simultaneously. \
============================================================================*/

#if ! defined (SHORT_NAMES)
#define CHOOSE_OBJECT_CHANGE_MANAGER_( object_type ) \
	choose_object_change_manager_ ## object_type
#else
#define CHOOSE_OBJECT_CHANGE_MANAGER_( object_type ) \
	coccf ## object_type
#endif
#define CHOOSE_OBJECT_CHANGE_MANAGER( object_type ) \
	CHOOSE_OBJECT_CHANGE_MANAGER_(object_type)

#define PROTOTYPE_CHOOSE_OBJECT_CHANGE_MANAGER_FUNCTION( \
	object_type ) \
int CHOOSE_OBJECT_CHANGE_MANAGER(object_type)( \
	Widget choose_object_widget, \
	struct MANAGER(object_type) *object_manager,	\
	struct object_type *new_object) \
/***************************************************************************** \
LAST MODIFIED : 17 June 2008 \
\
DESCRIPTION : \
Changes the manager of the chooser and limiting the available object. \
Also allows new_object to be set simultaneously.											\
============================================================================*/


#define PROTOTYPE_CHOOSE_OBJECT_GLOBAL_FUNCTIONS( object_type) \
PROTOTYPE_CREATE_CHOOSE_OBJECT_WIDGET_FUNCTION(object_type); \
PROTOTYPE_CHOOSE_OBJECT_GET_CALLBACK_FUNCTION(object_type); \
PROTOTYPE_CHOOSE_OBJECT_SET_CALLBACK_FUNCTION(object_type); \
PROTOTYPE_CHOOSE_OBJECT_GET_OBJECT_FUNCTION(object_type); \
PROTOTYPE_CHOOSE_OBJECT_SET_OBJECT_FUNCTION(object_type); \
PROTOTYPE_CHOOSE_OBJECT_CHANGE_CONDITIONAL_FUNCTION_FUNCTION(object_type); \
PROTOTYPE_CHOOSE_OBJECT_CHANGE_MANAGER_FUNCTION(object_type);
#endif /* !defined (CHOOSE_OBJECT_H) */
