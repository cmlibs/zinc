/*******************************************************************************
FILE : fe_region_choose_object.h

LAST MODIFIED : 13 January 2003

DESCRIPTION :
Macros for implementing an option menu dialog control for choosing an object
from its manager (subject to an optional conditional function). Handles manager
messages to keep the menu up-to-date.
Calls the client-specified callback routine if a different object is chosen.
==============================================================================*/
#if !defined (FE_REGION_CHOOSE_OBJECT_H)
#define FE_REGION_CHOOSE_OBJECT_H

#include <Xm/Xm.h>
#include "general/callback.h"
#include "finite_element/finite_element_region.h"
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
#define CREATE_FE_REGION_CHOOSE_OBJECT_WIDGET_( object_type ) \
	create_fe_region_choose_object_widget_ ## object_type
#else
#define CREATE_FE_REGION_CHOOSE_OBJECT_WIDGET_( object_type ) cfcow ## object_type
#endif
#define CREATE_FE_REGION_CHOOSE_OBJECT_WIDGET( object_type ) \
	CREATE_FE_REGION_CHOOSE_OBJECT_WIDGET_(object_type)

#define PROTOTYPE_CREATE_FE_REGION_CHOOSE_OBJECT_WIDGET_FUNCTION( object_type ) \
Widget CREATE_FE_REGION_CHOOSE_OBJECT_WIDGET(object_type)(Widget parent, \
  struct FE_region *fe_region, struct object_type *current_object, \
	LIST_CONDITIONAL_FUNCTION(object_type) *conditional_function, \
	void *conditional_function_user_data, \
	struct User_interface *user_interface) \
/***************************************************************************** \
LAST MODIFIED : 13 January 2003 \
\
DESCRIPTION : \
Creates an option menu from which an object from the manager may be chosen. \
The optional conditional function permits a subset of objects in the manager \
to be selectable. \
<user_interface> supplies fonts. \
============================================================================*/

#if ! defined (SHORT_NAMES)
#define FE_REGION_CHOOSE_OBJECT_GET_CALLBACK_( object_type ) \
	fe_region_choose_object_get_callback_ ## object_type
#else
#define FE_REGION_CHOOSE_OBJECT_GET_CALLBACK_( object_type ) cogc ## object_type
#endif
#define FE_REGION_CHOOSE_OBJECT_GET_CALLBACK( object_type ) \
	FE_REGION_CHOOSE_OBJECT_GET_CALLBACK_(object_type)

#define PROTOTYPE_FE_REGION_CHOOSE_OBJECT_GET_CALLBACK_FUNCTION( object_type ) \
struct Callback_data *FE_REGION_CHOOSE_OBJECT_GET_CALLBACK(object_type)( \
	Widget fe_region_choose_object_widget) \
/***************************************************************************** \
LAST MODIFIED : 10 January 2003 \
\
DESCRIPTION : \
Returns a pointer to the callback item of the fe_region_choose_object widget. \
============================================================================*/

#if ! defined (SHORT_NAMES)
#define FE_REGION_CHOOSE_OBJECT_SET_CALLBACK_( object_type ) \
	fe_region_choose_object_set_callback_ ## object_type
#else
#define FE_REGION_CHOOSE_OBJECT_SET_CALLBACK_( object_type ) cosc ## object_type
#endif
#define FE_REGION_CHOOSE_OBJECT_SET_CALLBACK( object_type ) \
	FE_REGION_CHOOSE_OBJECT_SET_CALLBACK_(object_type)

#define PROTOTYPE_FE_REGION_CHOOSE_OBJECT_SET_CALLBACK_FUNCTION( object_type ) \
int FE_REGION_CHOOSE_OBJECT_SET_CALLBACK(object_type)( \
	Widget fe_region_choose_object_widget,struct Callback_data *new_callback) \
/***************************************************************************** \
LAST MODIFIED : 10 January 2003 \
\
DESCRIPTION : \
Changes the callback item of the fe_region_choose_object widget. \
============================================================================*/

#if ! defined (SHORT_NAMES)
#define FE_REGION_CHOOSE_OBJECT_GET_OBJECT_( object_type ) \
	fe_region_choose_object_get_object_ ## object_type
#else
#define FE_REGION_CHOOSE_OBJECT_GET_OBJECT_( object_type ) cogo ## object_type
#endif
#define FE_REGION_CHOOSE_OBJECT_GET_OBJECT( object_type ) \
	FE_REGION_CHOOSE_OBJECT_GET_OBJECT_(object_type)

#define PROTOTYPE_FE_REGION_CHOOSE_OBJECT_GET_OBJECT_FUNCTION( object_type ) \
struct object_type *FE_REGION_CHOOSE_OBJECT_GET_OBJECT(object_type)( \
	Widget fe_region_choose_object_widget) \
/***************************************************************************** \
LAST MODIFIED : 10 January 2003 \
\
DESCRIPTION : \
Returns the currently chosen object in the fe_region_choose_object widget. \
============================================================================*/

#if ! defined (SHORT_NAMES)
#define FE_REGION_CHOOSE_OBJECT_SET_OBJECT_( object_type ) \
	fe_region_choose_object_set_object_ ## object_type
#else
#define FE_REGION_CHOOSE_OBJECT_SET_OBJECT_( object_type ) coso ## object_type
#endif
#define FE_REGION_CHOOSE_OBJECT_SET_OBJECT( object_type ) \
	FE_REGION_CHOOSE_OBJECT_SET_OBJECT_(object_type)

#define PROTOTYPE_FE_REGION_CHOOSE_OBJECT_SET_OBJECT_FUNCTION( object_type ) \
int FE_REGION_CHOOSE_OBJECT_SET_OBJECT(object_type)( \
	Widget fe_region_choose_object_widget,struct object_type *new_object) \
/***************************************************************************** \
LAST MODIFIED : 10 January 2003 \
\
DESCRIPTION : \
Changes the chosen object in the fe_region_choose_object widget. \
============================================================================*/

#if ! defined (SHORT_NAMES)
#define FE_REGION_CHOOSE_OBJECT_CHANGE_CONDITIONAL_FUNCTION_( object_type ) \
	fe_region_choose_object_change_conditional_function_ ## object_type
#else
#define FE_REGION_CHOOSE_OBJECT_CHANGE_CONDITIONAL_FUNCTION_( object_type ) \
	coccf ## object_type
#endif
#define FE_REGION_CHOOSE_OBJECT_CHANGE_CONDITIONAL_FUNCTION( object_type ) \
	FE_REGION_CHOOSE_OBJECT_CHANGE_CONDITIONAL_FUNCTION_(object_type)

#define PROTOTYPE_FE_REGION_CHOOSE_OBJECT_CHANGE_CONDITIONAL_FUNCTION_FUNCTION( \
	object_type ) \
int FE_REGION_CHOOSE_OBJECT_CHANGE_CONDITIONAL_FUNCTION(object_type)( \
	Widget fe_region_choose_object_widget, \
	LIST_CONDITIONAL_FUNCTION(object_type) *conditional_function, \
	void *conditional_function_user_data,struct object_type *new_object) \
/***************************************************************************** \
LAST MODIFIED : 10 January 2003 \
\
DESCRIPTION : \
Changes the conditional_function and user_data limiting the available \
selection of objects. Also allows new_object to be set simultaneously. \
============================================================================*/

#define PROTOTYPE_FE_REGION_CHOOSE_OBJECT_GLOBAL_FUNCTIONS( object_type) \
PROTOTYPE_CREATE_FE_REGION_CHOOSE_OBJECT_WIDGET_FUNCTION(object_type); \
PROTOTYPE_FE_REGION_CHOOSE_OBJECT_GET_CALLBACK_FUNCTION(object_type); \
PROTOTYPE_FE_REGION_CHOOSE_OBJECT_SET_CALLBACK_FUNCTION(object_type); \
PROTOTYPE_FE_REGION_CHOOSE_OBJECT_GET_OBJECT_FUNCTION(object_type); \
PROTOTYPE_FE_REGION_CHOOSE_OBJECT_SET_OBJECT_FUNCTION(object_type); \
PROTOTYPE_FE_REGION_CHOOSE_OBJECT_CHANGE_CONDITIONAL_FUNCTION_FUNCTION(object_type)

#endif /* !defined (FE_REGION_CHOOSE_OBJECT_H) */
