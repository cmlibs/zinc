/*******************************************************************************
FILE : choose_object_list.h

LAST MODIFIED : 10 September 1998

DESCRIPTION :
???RC Version of choose_object using lists instead of managers.
Macros for implementing an option menu dialog control for choosing an object
from a list (subject to an optional conditional function).
Calls the client-specified callback routine if a different object is chosen.
==============================================================================*/
#if !defined (CHOOSE_OBJECT_LIST_H)
#define CHOOSE_OBJECT_LIST_H

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
#define CREATE_CHOOSE_OBJECT_LIST_WIDGET_( object_type ) \
	create_choose_object_list_widget_ ## object_type
#else
#define CREATE_CHOOSE_OBJECT_LIST_WIDGET_( object_type ) ccolw ## object_type
#endif
#define CREATE_CHOOSE_OBJECT_LIST_WIDGET( object_type ) \
	CREATE_CHOOSE_OBJECT_LIST_WIDGET_(object_type)

#define PROTOTYPE_CREATE_CHOOSE_OBJECT_LIST_WIDGET_FUNCTION( object_type ) \
Widget CREATE_CHOOSE_OBJECT_LIST_WIDGET(object_type)(Widget parent, \
	struct object_type *current_object, \
	struct LIST(object_type) *list_of_objects, \
	LIST_CONDITIONAL_FUNCTION(object_type) *conditional_function) \
/***************************************************************************** \
LAST MODIFIED : 10 September 1998 \
\
DESCRIPTION : \
Creates an option menu from which an object from the manager may be chosen. \
The optional conditional function permits a subset of objects in the list \
to be selectable. \
============================================================================*/

#if defined (FULL_NAMES)
#define CHOOSE_OBJECT_LIST_SET_CALLBACK_( object_type ) \
	choose_object_list_set_callback_ ## object_type
#else
#define CHOOSE_OBJECT_LIST_SET_CALLBACK_( object_type ) colsc ## object_type
#endif
#define CHOOSE_OBJECT_LIST_SET_CALLBACK( object_type ) \
	CHOOSE_OBJECT_LIST_SET_CALLBACK_(object_type)

#define PROTOTYPE_CHOOSE_OBJECT_LIST_SET_CALLBACK_FUNCTION( object_type ) \
int CHOOSE_OBJECT_LIST_SET_CALLBACK(object_type)( \
	Widget choose_object_list_widget,struct Callback_data *new_callback) \
/***************************************************************************** \
LAST MODIFIED : 24 July 1997 \
\
DESCRIPTION : \
Changes the callback item of the choose_object_list_widget. \
============================================================================*/

#if defined (FULL_NAMES)
#define CHOOSE_OBJECT_LIST_SET_OBJECT_( object_type ) \
	choose_object_list_set_object_ ## object_type
#else
#define CHOOSE_OBJECT_LIST_SET_OBJECT_( object_type ) colso ## object_type
#endif
#define CHOOSE_OBJECT_LIST_SET_OBJECT( object_type ) \
	CHOOSE_OBJECT_LIST_SET_OBJECT_(object_type)

#define PROTOTYPE_CHOOSE_OBJECT_LIST_SET_OBJECT_FUNCTION( object_type ) \
int CHOOSE_OBJECT_LIST_SET_OBJECT(object_type)( \
	Widget choose_object_list_widget,struct object_type *new_object) \
/***************************************************************************** \
LAST MODIFIED : 24 July 1997 \
\
DESCRIPTION : \
Changes the chosen object in the choose_object_list_widget. \
============================================================================*/

#if defined (FULL_NAMES)
#define CHOOSE_OBJECT_LIST_GET_CALLBACK_( object_type ) \
	choose_object_list_get_callback_ ## object_type
#else
#define CHOOSE_OBJECT_LIST_GET_CALLBACK_( object_type ) colgc ## object_type
#endif
#define CHOOSE_OBJECT_LIST_GET_CALLBACK( object_type ) \
	CHOOSE_OBJECT_LIST_GET_CALLBACK_(object_type)

#define PROTOTYPE_CHOOSE_OBJECT_LIST_GET_CALLBACK_FUNCTION( object_type ) \
struct Callback_data *CHOOSE_OBJECT_LIST_GET_CALLBACK(object_type)( \
	Widget choose_object_list_widget) \
/***************************************************************************** \
LAST MODIFIED : 24 July 1997 \
\
DESCRIPTION : \
Returns a pointer to the callback item of the choose_object_list_widget. \
============================================================================*/

#if defined (FULL_NAMES)
#define CHOOSE_OBJECT_LIST_GET_OBJECT_( object_type ) \
	choose_object_list_get_object_ ## object_type
#else
#define CHOOSE_OBJECT_LIST_GET_OBJECT_( object_type ) colgo ## object_type
#endif
#define CHOOSE_OBJECT_LIST_GET_OBJECT( object_type ) \
	CHOOSE_OBJECT_LIST_GET_OBJECT_(object_type)

#define PROTOTYPE_CHOOSE_OBJECT_LIST_GET_OBJECT_FUNCTION( object_type ) \
struct object_type *CHOOSE_OBJECT_LIST_GET_OBJECT(object_type)( \
	Widget choose_object_list_widget) \
/***************************************************************************** \
LAST MODIFIED : 24 July 1997 \
\
DESCRIPTION : \
Returns the currently chosen object in the choose_object_list_widget. \
============================================================================*/

#if defined (FULL_NAMES)
#define CHOOSE_OBJECT_LIST_REFRESH_( object_type ) \
	choose_object_list_refresh_ ## object_type
#else
#define CHOOSE_OBJECT_LIST_REFRESH_( object_type ) colr ## object_type
#endif
#define CHOOSE_OBJECT_LIST_REFRESH( object_type ) \
	CHOOSE_OBJECT_LIST_REFRESH_(object_type)

#define PROTOTYPE_CHOOSE_OBJECT_LIST_REFRESH_FUNCTION( object_type ) \
int CHOOSE_OBJECT_LIST_REFRESH(object_type)(Widget choose_object_list_widget) \
/***************************************************************************** \
LAST MODIFIED : 24 July 1997 \
\
DESCRIPTION : \
Tells the choose_object_list_widget that the list has changed. \
============================================================================*/

#define PROTOTYPE_CHOOSE_OBJECT_LIST_GLOBAL_FUNCTIONS( object_type) \
PROTOTYPE_CREATE_CHOOSE_OBJECT_LIST_WIDGET_FUNCTION(object_type); \
PROTOTYPE_CHOOSE_OBJECT_LIST_SET_CALLBACK_FUNCTION(object_type); \
PROTOTYPE_CHOOSE_OBJECT_LIST_SET_OBJECT_FUNCTION(object_type); \
PROTOTYPE_CHOOSE_OBJECT_LIST_GET_CALLBACK_FUNCTION(object_type); \
PROTOTYPE_CHOOSE_OBJECT_LIST_GET_OBJECT_FUNCTION(object_type); \
PROTOTYPE_CHOOSE_OBJECT_LIST_REFRESH_FUNCTION(object_type)

#endif /* !defined (CHOOSE_OBJECT_LIST_H) */
