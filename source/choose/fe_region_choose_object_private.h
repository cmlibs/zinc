/*******************************************************************************
FILE : fe_region_choose_object_private.h

LAST MODIFIED : 20 March 2003

DESCRIPTION :
Macros for implementing an option menu dialog control for choosing an object
from its manager (subject to an optional conditional function). Handles manager
messages to keep the menu up-to-date.
Calls the client-specified callback routine if a different object is chosen.
==============================================================================*/
#if !defined (FE_REGION_CHOOSE_OBJECT_PRIVATE_H)
#define FE_REGION_CHOOSE_OBJECT_PRIVATE_H

#include <stdio.h>
#include "general/debug.h"
#include "choose/fe_region_choose_object.h"
#include "choose/chooser.h"
#include "finite_element/finite_element_region.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module variables
----------------
*/
#if ! defined (SHORT_NAMES)
#define FE_REGION_CHOOSE_OBJECT( object_type ) fe_region_choose_object_type_ ## object_type
#else
#define FE_REGION_CHOOSE_OBJECT( object_type ) cotype ## object_type
#endif

#define FULL_DECLARE_FE_REGION_CHOOSE_OBJECT_TYPE( object_type ) \
struct FE_REGION_CHOOSE_OBJECT(object_type) \
/***************************************************************************** \
LAST MODIFIED : 18 April 2000 \
\
DESCRIPTION : \
Contains information required by the fe_region_choose_object control dialog. \
============================================================================*/ \
{ \
	struct FE_region *fe_region; \
	LIST_CONDITIONAL_FUNCTION(object_type) *conditional_function; \
	void *conditional_function_user_data; \
	struct Chooser *chooser; \
	struct Callback_data update_callback; \
	Widget widget,parent; \
   int number_of_items; \
   void **items; \
   char **item_names; \
} /* struct FE_REGION_CHOOSE_OBJECT(object_type) */

/*
Module functions
----------------
*/

#if ! defined (SHORT_NAMES)
#define FE_REGION_CHOOSE_OBJECT_UPDATE( object_type ) \
	fe_region_choose_object_update_ ## object_type
#else
#define FE_REGION_CHOOSE_OBJECT_UPDATE( object_type ) cou ## object_type
#endif

#define DECLARE_FE_REGION_CHOOSE_OBJECT_UPDATE_FUNCTION( object_type ) \
static int FE_REGION_CHOOSE_OBJECT_UPDATE(object_type)( \
	struct FE_REGION_CHOOSE_OBJECT(object_type) *fe_region_choose_object) \
/***************************************************************************** \
LAST MODIFIED : 24 January 2000 \
\
DESCRIPTION : \
Tells CMGUI about the current values. Sends a pointer to the current object. \
============================================================================*/ \
{ \
	int return_code; \
\
	ENTER(FE_REGION_CHOOSE_OBJECT_UPDATE(object_type)); \
	if (fe_region_choose_object) \
	{ \
		if (fe_region_choose_object->update_callback.procedure) \
		{ \
			/* now call the procedure with the user data */ \
			(fe_region_choose_object->update_callback.procedure)( \
				fe_region_choose_object->widget,fe_region_choose_object->update_callback.data, \
				(struct object_type *) \
				Chooser_get_item(fe_region_choose_object->chooser)); \
		} \
		return_code=1; \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"FE_REGION_CHOOSE_OBJECT_UPDATE(" #object_type ").  Invalid argument(s)"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* FE_REGION_CHOOSE_OBJECT_UPDATE(object_type) */

#if ! defined (SHORT_NAMES)
#define FE_REGION_CHOOSE_OBJECT_UPDATE_CB( object_type ) \
	fe_region_choose_object_update_cb_ ## object_type
#else
#define FE_REGION_CHOOSE_OBJECT_UPDATE_CB( object_type ) couc ## object_type
#endif

#define DECLARE_FE_REGION_CHOOSE_OBJECT_UPDATE_CB_FUNCTION( object_type ) \
static void FE_REGION_CHOOSE_OBJECT_UPDATE_CB(object_type)(Widget widget, \
	void *fe_region_choose_object_void,void *dummy_void) \
/***************************************************************************** \
LAST MODIFIED : 21 January 2000 \
\
DESCRIPTION : \
Callback for the fe_region_choose_object dialog - tidies up all memory allocation. \
============================================================================*/ \
{ \
	struct FE_REGION_CHOOSE_OBJECT(object_type) *fe_region_choose_object; \
\
	ENTER(FE_REGION_CHOOSE_OBJECT_UPDATE_CB(object_type)); \
	USE_PARAMETER(widget); \
	USE_PARAMETER(dummy_void); \
	if (fe_region_choose_object= \
		(struct FE_REGION_CHOOSE_OBJECT(object_type) *)fe_region_choose_object_void) \
	{ \
		FE_REGION_CHOOSE_OBJECT_UPDATE(object_type)(fe_region_choose_object); \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"FE_REGION_CHOOSE_OBJECT_UPDATE_CB(" #object_type ").  Invalid argument"); \
	} \
	LEAVE; \
} /* FE_REGION_CHOOSE_OBJECT_UPDATE_CB(object_type) */

#if ! defined (SHORT_NAMES)
#define FE_REGION_CHOOSE_OBJECT_ADD_TO_LIST_DATA( object_type ) \
	fe_region_choose_object_add_to_list_data_ ## object_type
#else
#define FE_REGION_CHOOSE_OBJECT_ADD_TO_LIST_DATA( object_type ) coatld ## object_type
#endif

#define DECLARE_FE_REGION_CHOOSE_OBJECT_ADD_TO_LIST_DATA( object_type ) \
struct FE_REGION_CHOOSE_OBJECT_ADD_TO_LIST_DATA(object_type) \
/***************************************************************************** \
LAST MODIFIED : 18 April 2000 \
\
DESCRIPTION : \
Data for adding objects to an allocated list. Handles conditional function. \
============================================================================*/ \
{ \
  char **item_names; \
	int number_of_items; \
	LIST_CONDITIONAL_FUNCTION(object_type) *conditional_function; \
	void *conditional_function_user_data; \
  void **items; \
} /* struct FE_REGION_CHOOSE_OBJECT_ADD_TO_LIST_DATA(object_type) */

#if ! defined (SHORT_NAMES)
#define FE_REGION_CHOOSE_OBJECT_ADD_TO_LIST( object_type ) \
	fe_region_choose_object_add_to_list_ ## object_type
#else
#define FE_REGION_CHOOSE_OBJECT_ADD_TO_LIST( object_type ) coatl ## object_type
#endif

#define DECLARE_FE_REGION_CHOOSE_OBJECT_ADD_TO_LIST_FUNCTION( object_type ) \
static int FE_REGION_CHOOSE_OBJECT_ADD_TO_LIST(object_type)( \
	struct object_type *object,void *add_data_void) \
/***************************************************************************** \
LAST MODIFIED : 18 April 2000 \
\
DESCRIPTION : \
Puts the <object> at the array position pointed to by <list_position>. \
============================================================================*/ \
{ \
	int return_code; \
	struct FE_REGION_CHOOSE_OBJECT_ADD_TO_LIST_DATA(object_type) *add_data; \
 \
	ENTER(FE_REGION_CHOOSE_OBJECT_ADD_TO_LIST(object_type)); \
	if (object&&(add_data=(struct FE_REGION_CHOOSE_OBJECT_ADD_TO_LIST_DATA( \
		object_type) *)add_data_void)&&add_data->items&&add_data->item_names) \
	{ \
		return_code=1; \
		if (!(add_data->conditional_function)||(add_data->conditional_function)( \
			object,add_data->conditional_function_user_data)) \
		{ \
			if (GET_NAME(object_type)(object,add_data->item_names + \
				add_data->number_of_items)) \
			{ \
				add_data->items[add_data->number_of_items] = (void *)object; \
				(add_data->number_of_items++); \
			} \
			else \
			{ \
				display_message(ERROR_MESSAGE,"FE_REGION_CHOOSE_OBJECT_ADD_TO_LIST(" \
					#object_type ").  Could not get name of object"); \
				return_code=0; \
			} \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE,"FE_REGION_CHOOSE_OBJECT_ADD_TO_LIST(" \
			#object_type ").  Invalid argument(s)"); \
		return_code=0; \
	} \
	LEAVE; \
 \
	return (return_code); \
} /* FE_REGION_CHOOSE_OBJECT_ADD_TO_LIST(object_type) */

#if ! defined (SHORT_NAMES)
#define FE_REGION_CHOOSE_OBJECT_BUILD_ITEMS( object_type ) \
	fe_region_choose_object_get_items_ ## object_type
#else
#define FE_REGION_CHOOSE_OBJECT_BUILD_ITEMS( object_type ) cogi ## object_type
#endif

#define DECLARE_FE_REGION_CHOOSE_OBJECT_BUILD_ITEMS_FUNCTION( object_type ) \
static int FE_REGION_CHOOSE_OBJECT_BUILD_ITEMS(object_type)( \
	struct FE_REGION_CHOOSE_OBJECT(object_type) *fe_region_choose_object) \
/***************************************************************************** \
LAST MODIFIED : 10 January 2003 \
\
DESCRIPTION : \
Updates the arrays of all the choosable objects and their names. \
============================================================================*/ \
{ \
	int i,max_number_of_objects,return_code; \
	struct FE_REGION_CHOOSE_OBJECT_ADD_TO_LIST_DATA(object_type) add_to_list_data; \
\
	ENTER(FE_REGION_CHOOSE_OBJECT_BUILD_ITEMS(object_type)); \
	return_code=0; \
	if (fe_region_choose_object) \
	{ \
		max_number_of_objects = \
			FE_region_get_number_of_FE_fields(fe_region_choose_object->fe_region); \
		add_to_list_data.conditional_function= \
			fe_region_choose_object->conditional_function; \
		add_to_list_data.conditional_function_user_data= \
			fe_region_choose_object->conditional_function_user_data; \
		add_to_list_data.number_of_items=0; \
		add_to_list_data.items=(void **)NULL; \
		add_to_list_data.item_names=(char **)NULL; \
		if (((0==max_number_of_objects) || \
			ALLOCATE(add_to_list_data.items,void *,max_number_of_objects) && \
			ALLOCATE(add_to_list_data.item_names,char *,max_number_of_objects))&& \
			FE_region_for_each_FE_field(fe_region_choose_object->fe_region, \
				FE_REGION_CHOOSE_OBJECT_ADD_TO_LIST(object_type), \
				(void *)&add_to_list_data)) \
		{ \
         if (fe_region_choose_object->items) \
         { \
				DEALLOCATE(fe_region_choose_object->items); \
         } \
         if (fe_region_choose_object->item_names) \
         { \
				for (i=0;i<fe_region_choose_object->number_of_items;i++) \
				{ \
					DEALLOCATE(fe_region_choose_object->item_names[i]); \
				} \
			   DEALLOCATE(fe_region_choose_object->item_names); \
         } \
			fe_region_choose_object->number_of_items = add_to_list_data.number_of_items; \
			fe_region_choose_object->items = add_to_list_data.items; \
			fe_region_choose_object->item_names = add_to_list_data.item_names; \
			return_code=1; \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"FE_REGION_CHOOSE_OBJECT_BUILD_ITEMS(" #object_type ").  Failed"); \
			if (add_to_list_data.items) \
			{ \
				DEALLOCATE(add_to_list_data.items); \
			} \
			if (add_to_list_data.item_names) \
			{  \
				for (i=0;i<add_to_list_data.number_of_items;i++) \
				{ \
					DEALLOCATE(add_to_list_data.item_names[i]); \
				} \
				DEALLOCATE(add_to_list_data.item_names); \
			} \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"FE_REGION_CHOOSE_OBJECT_BUILD_ITEMS(" #object_type ").  Invalid argument(s)"); \
	} \
	LEAVE; \
 \
	return (return_code); \
} /* FE_REGION_CHOOSE_OBJECT_BUILD_ITEMS(object_type) */

#if ! defined (SHORT_NAMES)
#define FE_REGION_CHOOSE_OBJECT_FE_REGION_CHANGE( object_type ) \
	fe_region_choose_object_fe_region_change_ ## object_type
#else
#define FE_REGION_CHOOSE_OBJECT_FE_REGION_CHANGE( object_type ) cofc ## object_type
#endif

#define DECLARE_FE_REGION_CHOOSE_OBJECT_FE_REGION_CHANGE_FUNCTION( object_type ) \
static void FE_REGION_CHOOSE_OBJECT_FE_REGION_CHANGE(object_type)( \
	struct FE_region *fe_region, struct FE_region_changes *changes, \
	void *fe_region_choose_object_void) \
/***************************************************************************** \
LAST MODIFIED : 20 March 2003 \
\
DESCRIPTION : \
Rebuilds the fe_region_choose object menu in response to manager messages. \
Tries to minimise menu rebuilds as much as possible, since these cause \
annoying flickering on the screen. \
============================================================================*/ \
{ \
	enum CHANGE_LOG_CHANGE(object_type) change_summary; \
	struct FE_REGION_CHOOSE_OBJECT(object_type) *fe_region_choose_object; \
\
	ENTER(FE_REGION_CHOOSE_OBJECT_FE_REGION_CHANGE(object_type)); \
	if (fe_region && changes && (fe_region_choose_object = \
		(struct FE_REGION_CHOOSE_OBJECT(object_type) *) \
		fe_region_choose_object_void)) \
	{ \
		if (CHANGE_LOG_GET_CHANGE_SUMMARY(object_type)( \
			FE_region_changes_get_ ## object_type ## _changes(changes), \
			&change_summary)) \
		{ \
			/* menu may need updating on anything but RELATED_OBJECT_CHANGED */ \
			/*???RC note that less menu updating could be achieved for the case with \
			 the conditional function, however, much better solution is for menu \
			 to be flagged for rebuild here, then rebuilt just in time for items to \
			be selected. */ \
			if ((change_summary & CHANGE_LOG_OBJECT_ADDED(object_type)) || \
				(change_summary & CHANGE_LOG_OBJECT_REMOVED(object_type)) || \
				(change_summary & CHANGE_LOG_OBJECT_IDENTIFIER_CHANGED(object_type))|| \
				(((LIST_CONDITIONAL_FUNCTION(object_type) *)NULL != \
					fe_region_choose_object->conditional_function) && \
					(change_summary & \
						CHANGE_LOG_OBJECT_NOT_IDENTIFIER_CHANGED(object_type)))) \
			{ \
				if (!(FE_REGION_CHOOSE_OBJECT_BUILD_ITEMS(object_type)( \
					fe_region_choose_object) && \
					Chooser_build_main_menu(fe_region_choose_object->chooser, \
						fe_region_choose_object->number_of_items, \
						fe_region_choose_object->items, \
						fe_region_choose_object->item_names, \
						Chooser_get_item(fe_region_choose_object->chooser)))) \
				{ \
					display_message(ERROR_MESSAGE, \
						"FE_REGION_CHOOSE_OBJECT_FE_REGION_CHANGE(" #object_type \
						").  Could not update menu"); \
				} \
			} \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"FE_REGION_CHOOSE_OBJECT_FE_REGION_CHANGE(" #object_type \
			").  Invalid argument(s)"); \
	} \
	LEAVE; \
} /* FE_REGION_CHOOSE_OBJECT_FE_REGION_CHANGE(object_type) */

#if ! defined (SHORT_NAMES)
#define FE_REGION_CHOOSE_OBJECT_DESTROY_CB( object_type ) \
	fe_region_choose_object_destroy_cb_ ## object_type
#else
#define FE_REGION_CHOOSE_OBJECT_DESTROY_CB( object_type ) codc ## object_type
#endif

#define DECLARE_FE_REGION_CHOOSE_OBJECT_DESTROY_CB_FUNCTION( object_type ) \
static void FE_REGION_CHOOSE_OBJECT_DESTROY_CB(object_type)(Widget widget, \
	XtPointer client_data,XtPointer reason) \
/***************************************************************************** \
LAST MODIFIED : 10 January 2003 \
\
DESCRIPTION : \
Callback for the fe_region_choose_object dialog - tidies up all memory allocation. \
============================================================================*/ \
{ \
	int i; \
	struct FE_REGION_CHOOSE_OBJECT(object_type) *fe_region_choose_object; \
\
	ENTER(FE_REGION_CHOOSE_OBJECT_DESTROY_CB(object_type)); \
	USE_PARAMETER(widget); \
	USE_PARAMETER(reason); \
	if (fe_region_choose_object=(struct FE_REGION_CHOOSE_OBJECT(object_type) *)client_data) \
	{ \
		DESTROY(Chooser)(&(fe_region_choose_object->chooser)); \
		FE_region_remove_callback(fe_region_choose_object->fe_region, \
			FE_REGION_CHOOSE_OBJECT_FE_REGION_CHANGE(object_type), \
			(void *)fe_region_choose_object); \
    if (fe_region_choose_object->items) \
    { \
			DEALLOCATE(fe_region_choose_object->items); \
    } \
    if (fe_region_choose_object->item_names) \
    { \
			for (i=0;i<fe_region_choose_object->number_of_items;i++) \
			{ \
				DEALLOCATE(fe_region_choose_object->item_names[i]); \
			} \
		  DEALLOCATE(fe_region_choose_object->item_names); \
    } \
		DEALLOCATE(fe_region_choose_object); \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"FE_REGION_CHOOSE_OBJECT_DESTROY_CB(" #object_type ").  Missing fe_region_choose_object"); \
	} \
	LEAVE; \
} /* FE_REGION_CHOOSE_OBJECT_DESTROY_CB(object_type) */

/*
Global functions
----------------
*/

#define DECLARE_CREATE_FE_REGION_CHOOSE_OBJECT_WIDGET_FUNCTION( object_type ) \
PROTOTYPE_CREATE_FE_REGION_CHOOSE_OBJECT_WIDGET_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 21 November 2001 \
\
DESCRIPTION : \
Creates an option menu from which an object from the manager may be chosen. \
The optional conditional function permits a subset of objects in the manager \
to be selectable. \
<user_interface> supplies fonts. \
============================================================================*/ \
{ \
	struct Callback_data callback; \
	struct FE_REGION_CHOOSE_OBJECT(object_type) *fe_region_choose_object; \
	Widget return_widget; \
\
	ENTER(CREATE_FE_REGION_CHOOSE_OBJECT_WIDGET(object_type)); \
	return_widget=(Widget)NULL; \
	if (fe_region && parent && user_interface) \
	{ \
		if (ALLOCATE(fe_region_choose_object,struct FE_REGION_CHOOSE_OBJECT(object_type),1)) \
		{ \
			/* initialise the structure */ \
			fe_region_choose_object->chooser=(struct Chooser *)NULL; \
			fe_region_choose_object->widget=(Widget)NULL; \
			fe_region_choose_object->parent=parent; \
			fe_region_choose_object->update_callback.procedure= \
				(Callback_procedure *)NULL; \
			fe_region_choose_object->update_callback.data=(void *)NULL; \
			fe_region_choose_object->fe_region = fe_region; \
			fe_region_choose_object->conditional_function=conditional_function; \
			fe_region_choose_object->conditional_function_user_data= \
				conditional_function_user_data; \
      fe_region_choose_object->number_of_items = 0; \
      fe_region_choose_object->items = (void **)NULL; \
      fe_region_choose_object->item_names = (char **)NULL; \
		  if (FE_REGION_CHOOSE_OBJECT_BUILD_ITEMS(object_type)(fe_region_choose_object)) \
			{ \
				if (fe_region_choose_object->chooser = \
					CREATE(Chooser)(parent, fe_region_choose_object->number_of_items, \
					fe_region_choose_object->items, fe_region_choose_object->item_names, \
					(void *)current_object, &(fe_region_choose_object->widget), \
					user_interface)) \
				{ \
					/* add fe_region_choose_object as user data to fe_region_chooser widget */ \
					XtVaSetValues(fe_region_choose_object->widget,XmNuserData,fe_region_choose_object,NULL); \
					/* add destroy callback for fe_region_chooser widget */ \
					XtAddCallback(fe_region_choose_object->widget,XmNdestroyCallback, \
						FE_REGION_CHOOSE_OBJECT_DESTROY_CB(object_type),(XtPointer)fe_region_choose_object); \
					/* get updates when fe_region_chooser changes */ \
					callback.data=(void *)fe_region_choose_object; \
					callback.procedure=FE_REGION_CHOOSE_OBJECT_UPDATE_CB(object_type); \
					Chooser_set_update_callback(fe_region_choose_object->chooser, \
						&callback); \
					/* register for any object changes */ \
					FE_region_add_callback(fe_region, \
						FE_REGION_CHOOSE_OBJECT_FE_REGION_CHANGE(object_type), \
						(void *)fe_region_choose_object); \
					return_widget=fe_region_choose_object->widget; \
				} \
				else \
				{ \
					display_message(ERROR_MESSAGE, \
						"CREATE_FE_REGION_CHOOSE_OBJECT_WIDGET(" #object_type \
						").  Could not create fe_region_chooser"); \
					DEALLOCATE(fe_region_choose_object); \
				} \
			} \
			else \
			{ \
				display_message(ERROR_MESSAGE, \
					"CREATE_FE_REGION_CHOOSE_OBJECT_WIDGET(" #object_type \
					").  Could not get items"); \
				DEALLOCATE(fe_region_choose_object); \
			} \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"CREATE_FE_REGION_CHOOSE_OBJECT_WIDGET(" #object_type \
				").  Not enough memory"); \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"CREATE_FE_REGION_CHOOSE_OBJECT_WIDGET(" #object_type \
			").  Invalid argument(s)"); \
	} \
	LEAVE; \
\
	return (return_widget); \
} /* CREATE_FE_REGION_CHOOSE_OBJECT_WIDGET(object_type) */

#define DECLARE_FE_REGION_CHOOSE_OBJECT_GET_CALLBACK_FUNCTION( object_type ) \
PROTOTYPE_FE_REGION_CHOOSE_OBJECT_GET_CALLBACK_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 9 June 2000 \
\
DESCRIPTION : \
Returns a pointer to the callback item of the fe_region_choose_object_widget. \
============================================================================*/ \
{ \
	struct Callback_data *callback; \
	struct FE_REGION_CHOOSE_OBJECT(object_type) *fe_region_choose_object; \
\
	ENTER(FE_REGION_CHOOSE_OBJECT_GET_CALLBACK(object_type)); \
	if (fe_region_choose_object_widget) \
	{ \
		fe_region_choose_object=(struct FE_REGION_CHOOSE_OBJECT(object_type) *)NULL; \
		/* Get the pointer to the data for the fe_region_choose_object dialog */ \
		XtVaGetValues(fe_region_choose_object_widget,XmNuserData,&fe_region_choose_object,NULL); \
		if (fe_region_choose_object) \
		{ \
			callback=&(fe_region_choose_object->update_callback); \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"FE_REGION_CHOOSE_OBJECT_GET_CALLBACK(" #object_type \
				").  Missing widget data"); \
			callback=(struct Callback_data *)NULL; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"FE_REGION_CHOOSE_OBJECT_GET_CALLBACK(" #object_type ").  Missing widget"); \
		callback=(struct Callback_data *)NULL; \
	} \
	LEAVE; \
\
	return (callback); \
} /* FE_REGION_CHOOSE_OBJECT_GET_CALLBACK(object_type) */

#define DECLARE_FE_REGION_CHOOSE_OBJECT_SET_CALLBACK_FUNCTION( object_type ) \
PROTOTYPE_FE_REGION_CHOOSE_OBJECT_SET_CALLBACK_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 9 June 2000 \
\
DESCRIPTION : \
Changes the callback item of the fe_region_choose_object_widget. \
============================================================================*/ \
{ \
	int return_code; \
	struct FE_REGION_CHOOSE_OBJECT(object_type) *fe_region_choose_object; \
\
	ENTER(FE_REGION_CHOOSE_OBJECT_SET_CALLBACK(object_type)); \
	if (fe_region_choose_object_widget&&new_callback) \
	{ \
		fe_region_choose_object=(struct FE_REGION_CHOOSE_OBJECT(object_type) *)NULL; \
		/* Get the pointer to the data for the fe_region_choose_object dialog */ \
		XtVaGetValues(fe_region_choose_object_widget,XmNuserData,&fe_region_choose_object,NULL); \
		if (fe_region_choose_object) \
		{ \
			fe_region_choose_object->update_callback.procedure=new_callback->procedure; \
			fe_region_choose_object->update_callback.data=new_callback->data; \
			return_code=1; \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"FE_REGION_CHOOSE_OBJECT_SET_CALLBACK(" #object_type \
				").  Missing widget data"); \
			return_code=0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"FE_REGION_CHOOSE_OBJECT_SET_CALLBACK(" #object_type ").  Missing widget"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* FE_REGION_CHOOSE_OBJECT_SET_CALLBACK(object_type) */

#define DECLARE_FE_REGION_CHOOSE_OBJECT_GET_OBJECT_FUNCTION( object_type ) \
PROTOTYPE_FE_REGION_CHOOSE_OBJECT_GET_OBJECT_FUNCTION( object_type ) \
/***************************************************************************** \
LAST MODIFIED : 9 June 2000 \
\
DESCRIPTION : \
Returns the currently chosen object in the fe_region_choose_object_widget. \
============================================================================*/ \
{ \
	struct object_type *object; \
	struct FE_REGION_CHOOSE_OBJECT(object_type) *fe_region_choose_object; \
\
	ENTER(FE_REGION_CHOOSE_OBJECT_GET_OBJECT(object_type)); \
	if (fe_region_choose_object_widget) \
	{ \
		fe_region_choose_object=(struct FE_REGION_CHOOSE_OBJECT(object_type) *)NULL; \
		/* Get the pointer to the data for the fe_region_choose_object dialog */ \
		XtVaGetValues(fe_region_choose_object_widget,XmNuserData,&fe_region_choose_object,NULL); \
		if (fe_region_choose_object) \
		{ \
			object=(struct object_type *)Chooser_get_item(fe_region_choose_object->chooser); \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"FE_REGION_CHOOSE_OBJECT_GET_OBJECT(" #object_type ").  Missing widget data"); \
			object=(struct object_type *)NULL; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"FE_REGION_CHOOSE_OBJECT_GET_OBJECT(" #object_type ").  Missing widget"); \
		object=(struct object_type *)NULL; \
	} \
	LEAVE; \
\
	return (object); \
} /* FE_REGION_CHOOSE_OBJECT_GET_OBJECT(object_type) */

#define DECLARE_FE_REGION_CHOOSE_OBJECT_SET_OBJECT_FUNCTION( object_type ) \
PROTOTYPE_FE_REGION_CHOOSE_OBJECT_SET_OBJECT_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 9 June 2000 \
\
DESCRIPTION : \
Changes the chosen object in the fe_region_choose_object_widget. \
============================================================================*/ \
{ \
	int return_code; \
	struct FE_REGION_CHOOSE_OBJECT(object_type) *fe_region_choose_object; \
\
	ENTER(FE_REGION_CHOOSE_OBJECT_SET_OBJECT(object_type)); \
	if (fe_region_choose_object_widget) \
	{ \
		fe_region_choose_object=(struct FE_REGION_CHOOSE_OBJECT(object_type) *)NULL; \
		/* Get the pointer to the data for the fe_region_choose_object dialog */ \
		XtVaGetValues(fe_region_choose_object_widget,XmNuserData,&fe_region_choose_object,NULL); \
		if (fe_region_choose_object) \
		{ \
			return_code=Chooser_set_item(fe_region_choose_object->chooser,(void *)new_object); \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"FE_REGION_CHOOSE_OBJECT_SET_OBJECT(" #object_type ").  Missing widget data"); \
			return_code=0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"FE_REGION_CHOOSE_OBJECT_SET_OBJECT(" #object_type ").  Missing widget"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* FE_REGION_CHOOSE_OBJECT_SET_OBJECT(object_type) */

#define DECLARE_FE_REGION_CHOOSE_OBJECT_CHANGE_CONDITIONAL_FUNCTION_FUNCTION( \
	object_type ) \
PROTOTYPE_FE_REGION_CHOOSE_OBJECT_CHANGE_CONDITIONAL_FUNCTION_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 9 June 2000 \
\
DESCRIPTION : \
Changes the conditional_function and user_data limiting the available \
selection of objects. Also allows new_object to be set simultaneously. \
============================================================================*/ \
{ \
	int return_code; \
	struct FE_REGION_CHOOSE_OBJECT(object_type) *fe_region_choose_object; \
\
	ENTER(FE_REGION_CHOOSE_OBJECT_CHANGE_CONDITIONAL_FUNCTION(object_type)); \
	if (fe_region_choose_object_widget) \
	{ \
		fe_region_choose_object=(struct FE_REGION_CHOOSE_OBJECT(object_type) *)NULL; \
		/* Get the pointer to the data for the fe_region_choose_object dialog */ \
		XtVaGetValues(fe_region_choose_object_widget,XmNuserData,&fe_region_choose_object,NULL); \
		if (fe_region_choose_object) \
		{ \
			fe_region_choose_object->conditional_function=conditional_function; \
			fe_region_choose_object->conditional_function_user_data= \
				conditional_function_user_data; \
		  if (FE_REGION_CHOOSE_OBJECT_BUILD_ITEMS(object_type)(fe_region_choose_object)) \
			{ \
				return_code=Chooser_build_main_menu(fe_region_choose_object->chooser, \
					fe_region_choose_object->number_of_items,fe_region_choose_object->items, \
				   fe_region_choose_object->item_names,(void *)new_object); \
			} \
			else \
			{ \
				return_code=0; \
			} \
			if (!return_code) \
			{ \
				display_message(ERROR_MESSAGE, \
					"FE_REGION_CHOOSE_OBJECT_CHANGE_CONDITIONAL_FUNCTION(" #object_type \
					").  Could not update menu"); \
			} \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"FE_REGION_CHOOSE_OBJECT_CHANGE_CONDITIONAL_FUNCTION(" #object_type \
				").  Missing widget data"); \
			return_code=0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"FE_REGION_CHOOSE_OBJECT_CHANGE_CONDITIONAL_FUNCTION(" #object_type \
			").  Missing widget"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* FE_REGION_CHOOSE_OBJECT_CHANGE_CONDITIONAL_FUNCTION(object_type) */

#define DECLARE_FE_REGION_CHOOSE_OBJECT_MODULE_FUNCTIONS( object_type ) \
DECLARE_FE_REGION_CHOOSE_OBJECT_UPDATE_FUNCTION(object_type) \
DECLARE_FE_REGION_CHOOSE_OBJECT_UPDATE_CB_FUNCTION(object_type) \
DECLARE_FE_REGION_CHOOSE_OBJECT_ADD_TO_LIST_DATA(object_type); \
DECLARE_FE_REGION_CHOOSE_OBJECT_ADD_TO_LIST_FUNCTION(object_type) \
DECLARE_FE_REGION_CHOOSE_OBJECT_BUILD_ITEMS_FUNCTION(object_type) \
DECLARE_FE_REGION_CHOOSE_OBJECT_FE_REGION_CHANGE_FUNCTION(object_type) \
DECLARE_FE_REGION_CHOOSE_OBJECT_DESTROY_CB_FUNCTION(object_type)

#define DECLARE_FE_REGION_CHOOSE_OBJECT_GLOBAL_FUNCTIONS( object_type ) \
DECLARE_CREATE_FE_REGION_CHOOSE_OBJECT_WIDGET_FUNCTION(object_type) \
DECLARE_FE_REGION_CHOOSE_OBJECT_GET_CALLBACK_FUNCTION(object_type) \
DECLARE_FE_REGION_CHOOSE_OBJECT_SET_CALLBACK_FUNCTION(object_type) \
DECLARE_FE_REGION_CHOOSE_OBJECT_GET_OBJECT_FUNCTION(object_type) \
DECLARE_FE_REGION_CHOOSE_OBJECT_SET_OBJECT_FUNCTION(object_type) \
DECLARE_FE_REGION_CHOOSE_OBJECT_CHANGE_CONDITIONAL_FUNCTION_FUNCTION(object_type)

#endif /* !defined (FE_REGION_CHOOSE_OBJECT_PRIVATE_H) */
