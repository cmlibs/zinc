/*******************************************************************************
FILE : select.h

LAST MODIFIED : 25 May 1997

DESCRIPTION :
Creates a scrolled list of objects based upon their name.  Allows the user
to add, delete and rename the objects.  Interfaces with the global manager
for each type it supports.
???RC.  New version using macros to handle different object types.
==============================================================================*/
#if !defined (SELECT_H)
#define SELECT_H

#include <Xm/Xm.h>
#include "general/callback.h"

/*
Global Types
------------
*/
enum Select_appearance
/*******************************************************************************
LAST MODIFIED : 17 February 1995

DESCRIPTION :
The different data structures supported for this widget.
==============================================================================*/
{
	SELECT_LIST, /* a list of objects that may be clicked on */
	SELECT_TEXT  /* type in the name of the object */
}; /* enum Select_appearance */

/*
Global functions
----------------
*/
#if ! defined (SHORT_NAMES)
#define CREATE_SELECT_WIDGET_( object_type ) \
	create_select_widget_ ## object_type
#else
#define CREATE_SELECT_WIDGET_( object_type )  csw ## object_type
#endif
#define CREATE_SELECT_WIDGET( object_type )  CREATE_SELECT_WIDGET_(object_type)

#define PROTOTYPE_CREATE_SELECT_WIDGET_FUNCTION( object_type ) \
Widget CREATE_SELECT_WIDGET(object_type)(Widget *select_widget,Widget parent, \
	enum Select_appearance appearance,struct object_type *selected_object, \
	struct MANAGER(object_type) *object_manager) \
/***************************************************************************** \
LAST MODIFIED : 6 May 1997 \
\
DESCRIPTION : \
Creates a selection widget that will allow the user to choose an object based \
upon its name. \
============================================================================*/

#if ! defined (SHORT_NAMES)
#define SELECT_SET_UPDATE_CB_( object_type ) \
	select_set_update_cb_ ## object_type
#else
#define SELECT_SET_UPDATE_CB_( object_type ) ssuc ## object_type
#endif
#define SELECT_SET_UPDATE_CB( object_type ) \
	SELECT_SET_UPDATE_CB_(object_type)

#define PROTOTYPE_SELECT_SET_UPDATE_CB_FUNCTION( object_type ) \
int SELECT_SET_UPDATE_CB(object_type)(Widget select_widget, \
	struct Callback_data *cb_data) \
/***************************************************************************** \
LAST MODIFIED : 13 May 1997 \
\
DESCRIPTION : \
Changes the callback item of the select widget. \
============================================================================*/

#if ! defined (SHORT_NAMES)
#define SELECT_SET_SELECT_ITEM_( object_type ) \
	select_set_select_item_ ## object_type
#else
#define SELECT_SET_SELECT_ITEM_( object_type ) sss ## object_type
#endif
#define SELECT_SET_SELECT_ITEM( object_type ) \
	SELECT_SET_SELECT_ITEM_(object_type)

#define PROTOTYPE_SELECT_SET_SELECT_ITEM_FUNCTION( object_type ) \
int SELECT_SET_SELECT_ITEM(object_type)(Widget select_widget, \
	struct object_type *selected_object) \
/***************************************************************************** \
LAST MODIFIED : 13 May 1997 \
\
DESCRIPTION : \
Changes the selected object in the select widget. \
============================================================================*/

#if ! defined (SHORT_NAMES)
#define SELECT_GET_UPDATE_CB_( object_type ) \
	select_get_update_cb_ ## object_type
#else
#define SELECT_GET_UPDATE_CB_( object_type ) sgu ## object_type
#endif
#define SELECT_GET_UPDATE_CB( object_type ) \
	SELECT_GET_UPDATE_CB_(object_type)

#define PROTOTYPE_SELECT_GET_UPDATE_CB_FUNCTION( object_type ) \
struct Callback_data *SELECT_GET_UPDATE_CB(object_type)(Widget select_widget) \
/***************************************************************************** \
LAST MODIFIED : 13 May 1997 \
\
DESCRIPTION : \
Returns a pointer to the callback item of the select widget. \
============================================================================*/

#if ! defined (SHORT_NAMES)
#define SELECT_GET_SELECT_ITEM_( object_type ) \
	select_get_select_item_ ## object_type
#else
#define SELECT_GET_SELECT_ITEM_( object_type ) sgs ## object_type
#endif
#define SELECT_GET_SELECT_ITEM( object_type ) \
	SELECT_GET_SELECT_ITEM_(object_type)

#define PROTOTYPE_SELECT_GET_SELECT_ITEM_FUNCTION( object_type ) \
struct object_type *SELECT_GET_SELECT_ITEM(object_type)(Widget select_widget) \
/***************************************************************************** \
LAST MODIFIED : 13 May 1997 \
\
DESCRIPTION : \
Returns a pointer to the selected object in the select widget. \
============================================================================*/

#define PROTOTYPE_SELECT_GLOBAL_FUNCTIONS( object_type) \
PROTOTYPE_CREATE_SELECT_WIDGET_FUNCTION(object_type); \
PROTOTYPE_SELECT_SET_UPDATE_CB_FUNCTION(object_type); \
PROTOTYPE_SELECT_SET_SELECT_ITEM_FUNCTION(object_type); \
PROTOTYPE_SELECT_GET_UPDATE_CB_FUNCTION(object_type); \
PROTOTYPE_SELECT_GET_SELECT_ITEM_FUNCTION(object_type)

#endif /* !defined (SELECT_H) */
