/*******************************************************************************
FILE : choose_object.h

LAST MODIFIED : 10 September 1998

DESCRIPTION :
Macros for implementing an option menu dialog control for choosing an object
from its manager (subject to an optional conditional function). Handles manager
messages to keep the menu up-to-date.
Calls the client-specified callback routine if a different object is chosen.
==============================================================================*/
#if !defined (CHOOSE_OBJECT_H)
#define CHOOSE_OBJECT_H

#include <Xm/Xm.h>
#include "general/callback.h"

/*
Global Types
------------
*/

/*
Global Functions
---------------
*/
#if defined (FULL_NAMES)
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
	MANAGER_CONDITIONAL_FUNCTION(object_type) *conditional_function) \
/***************************************************************************** \
LAST MODIFIED : 10 September 1998 \
\
DESCRIPTION : \
Creates an option menu from which an object from the manager may be chosen. \
The optional conditional function permits a subset of objects in the manager \
to be selectable. \
============================================================================*/

#if defined (FULL_NAMES)
#define CHOOSE_OBJECT_SET_CALLBACK_( object_type ) \
	choose_object_set_callback_ ## object_type
#else
#define CHOOSE_OBJECT_SET_CALLBACK_( object_type ) cosc ## object_type
#endif
#define CHOOSE_OBJECT_SET_CALLBACK( object_type ) \
	CHOOSE_OBJECT_SET_CALLBACK_(object_type)

#define PROTOTYPE_CHOOSE_OBJECT_SET_CALLBACK_FUNCTION( object_type ) \
int CHOOSE_OBJECT_SET_CALLBACK(object_type)(Widget choose_object_widget, \
	struct Callback_data *new_callback) \
/***************************************************************************** \
LAST MODIFIED : 23 July 1997 \
\
DESCRIPTION : \
Changes the callback item of the choose_object_widget. \
============================================================================*/

#if defined (FULL_NAMES)
#define CHOOSE_OBJECT_SET_OBJECT_( object_type ) \
	choose_object_set_object_ ## object_type
#else
#define CHOOSE_OBJECT_SET_OBJECT_( object_type ) coso ## object_type
#endif
#define CHOOSE_OBJECT_SET_OBJECT( object_type ) \
	CHOOSE_OBJECT_SET_OBJECT_(object_type)

#define PROTOTYPE_CHOOSE_OBJECT_SET_OBJECT_FUNCTION( object_type ) \
int CHOOSE_OBJECT_SET_OBJECT(object_type)(Widget choose_object_widget, \
	struct object_type *new_object) \
/***************************************************************************** \
LAST MODIFIED : 23 July 1997 \
\
DESCRIPTION : \
Changes the chosen object in the choose_object_widget. \
============================================================================*/

#if defined (FULL_NAMES)
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
LAST MODIFIED : 23 July 1997 \
\
DESCRIPTION : \
Returns a pointer to the callback item of the choose_object_widget. \
============================================================================*/

#if defined (FULL_NAMES)
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
LAST MODIFIED : 23 July 1997 \
\
DESCRIPTION : \
Returns the currently chosen object in the choose_object_widget. \
============================================================================*/

#define PROTOTYPE_CHOOSE_OBJECT_GLOBAL_FUNCTIONS( object_type) \
PROTOTYPE_CREATE_CHOOSE_OBJECT_WIDGET_FUNCTION(object_type); \
PROTOTYPE_CHOOSE_OBJECT_SET_CALLBACK_FUNCTION(object_type); \
PROTOTYPE_CHOOSE_OBJECT_SET_OBJECT_FUNCTION(object_type); \
PROTOTYPE_CHOOSE_OBJECT_GET_CALLBACK_FUNCTION(object_type); \
PROTOTYPE_CHOOSE_OBJECT_GET_OBJECT_FUNCTION(object_type)

#endif /* !defined (CHOOSE_OBJECT_H) */
