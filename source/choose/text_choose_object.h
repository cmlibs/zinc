/*******************************************************************************
FILE : text_choose_object.h

LAST MODIFIED : 18 August 1998

DESCRIPTION :
Macros for implementing an option menu dialog control for choosing an object
from its manager. Handles manager messages to keep the menu up-to-date.
Calls the client-specified callback routine if a different object is chosen.
==============================================================================*/
#if !defined (TEXT_CHOOSE_OBJECT_H)
#define TEXT_CHOOSE_OBJECT_H

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
#define CREATE_TEXT_CHOOSE_OBJECT_WIDGET_( object_type ) \
	create_text_choose_object_widget_ ## object_type
#else
#define CREATE_TEXT_CHOOSE_OBJECT_WIDGET_( object_type ) ctcow ## object_type
#endif
#define CREATE_TEXT_CHOOSE_OBJECT_WIDGET( object_type ) \
	CREATE_TEXT_CHOOSE_OBJECT_WIDGET_(object_type)

#define PROTOTYPE_CREATE_TEXT_CHOOSE_OBJECT_WIDGET_FUNCTION( object_type ) \
Widget CREATE_TEXT_CHOOSE_OBJECT_WIDGET(object_type)(Widget parent, \
	struct object_type *current_object, \
	struct MANAGER(object_type) *object_manager, \
	MANAGER_CONDITIONAL_FUNCTION(object_type) *conditional_function, \
	int (object_to_string)(struct object_type *,char **), \
	struct object_type *(string_to_object)(char *, \
		struct MANAGER(object_type) *)) \
/***************************************************************************** \
LAST MODIFIED : 18 August 1998 \
\
DESCRIPTION : \
Creates a text box from which an object from the manager may be entered. \
The optional <conditional_function> allows a selection of the objects in the \
manager to be valid. If it is NULL, all objects in the manager are usable. \
The <object_to_string> function must be supplied, and builds a string to \
describe the identifier of the object. The <string_to_object> function does \
the reverse. These functions are useful in the case of elements where only \
the element number (or face or line) is to be specified, not all three. \
============================================================================*/

#if defined (FULL_NAMES)
#define TEXT_CHOOSE_OBJECT_SET_CALLBACK_( object_type ) \
	text_choose_object_set_callback_ ## object_type
#else
#define TEXT_CHOOSE_OBJECT_SET_CALLBACK_( object_type ) cosc ## object_type
#endif
#define TEXT_CHOOSE_OBJECT_SET_CALLBACK( object_type ) \
	TEXT_CHOOSE_OBJECT_SET_CALLBACK_(object_type)

#define PROTOTYPE_TEXT_CHOOSE_OBJECT_SET_CALLBACK_FUNCTION( object_type ) \
int TEXT_CHOOSE_OBJECT_SET_CALLBACK(object_type)( \
	Widget text_choose_object_widget,struct Callback_data *new_callback) \
/***************************************************************************** \
LAST MODIFIED : 18 August 1998 \
\
DESCRIPTION : \
Changes the callback item of the text_choose_object_widget. \
============================================================================*/

#if defined (FULL_NAMES)
#define TEXT_CHOOSE_OBJECT_SET_OBJECT_( object_type ) \
	text_choose_object_set_object_ ## object_type
#else
#define TEXT_CHOOSE_OBJECT_SET_OBJECT_( object_type ) coso ## object_type
#endif
#define TEXT_CHOOSE_OBJECT_SET_OBJECT( object_type ) \
	TEXT_CHOOSE_OBJECT_SET_OBJECT_(object_type)

#define PROTOTYPE_TEXT_CHOOSE_OBJECT_SET_OBJECT_FUNCTION( object_type ) \
int TEXT_CHOOSE_OBJECT_SET_OBJECT(object_type)( \
	Widget text_choose_object_widget,struct object_type *new_object) \
/***************************************************************************** \
LAST MODIFIED : 18 August 1998 \
\
DESCRIPTION : \
Changes the chosen object in the text_choose_object_widget. \
============================================================================*/

#if defined (FULL_NAMES)
#define TEXT_CHOOSE_OBJECT_GET_CALLBACK_( object_type ) \
	text_choose_object_get_callback_ ## object_type
#else
#define TEXT_CHOOSE_OBJECT_GET_CALLBACK_( object_type ) cogc ## object_type
#endif
#define TEXT_CHOOSE_OBJECT_GET_CALLBACK( object_type ) \
	TEXT_CHOOSE_OBJECT_GET_CALLBACK_(object_type)

#define PROTOTYPE_TEXT_CHOOSE_OBJECT_GET_CALLBACK_FUNCTION( object_type ) \
struct Callback_data *TEXT_CHOOSE_OBJECT_GET_CALLBACK(object_type)( \
	Widget text_choose_object_widget) \
/***************************************************************************** \
LAST MODIFIED : 18 August 1998 \
\
DESCRIPTION : \
Returns a pointer to the callback item of the text_choose_object_widget. \
============================================================================*/

#if defined (FULL_NAMES)
#define TEXT_CHOOSE_OBJECT_GET_OBJECT_( object_type ) \
	text_choose_object_get_object_ ## object_type
#else
#define TEXT_CHOOSE_OBJECT_GET_OBJECT_( object_type ) cogo ## object_type
#endif
#define TEXT_CHOOSE_OBJECT_GET_OBJECT( object_type ) \
	TEXT_CHOOSE_OBJECT_GET_OBJECT_(object_type)

#define PROTOTYPE_TEXT_CHOOSE_OBJECT_GET_OBJECT_FUNCTION( object_type ) \
struct object_type *TEXT_CHOOSE_OBJECT_GET_OBJECT(object_type)( \
	Widget text_choose_object_widget) \
/***************************************************************************** \
LAST MODIFIED : 18 August 1998 \
\
DESCRIPTION : \
Returns the currently chosen object in the text_choose_object_widget. \
============================================================================*/

#define PROTOTYPE_TEXT_CHOOSE_OBJECT_GLOBAL_FUNCTIONS( object_type) \
PROTOTYPE_CREATE_TEXT_CHOOSE_OBJECT_WIDGET_FUNCTION(object_type); \
PROTOTYPE_TEXT_CHOOSE_OBJECT_SET_CALLBACK_FUNCTION(object_type); \
PROTOTYPE_TEXT_CHOOSE_OBJECT_SET_OBJECT_FUNCTION(object_type); \
PROTOTYPE_TEXT_CHOOSE_OBJECT_GET_CALLBACK_FUNCTION(object_type); \
PROTOTYPE_TEXT_CHOOSE_OBJECT_GET_OBJECT_FUNCTION(object_type)

#endif /* !defined (TEXT_CHOOSE_OBJECT_H) */
