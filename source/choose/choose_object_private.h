/*******************************************************************************
FILE : choose_object_private.h

LAST MODIFIED : 21 October 1999

DESCRIPTION :
Macros for implementing an option menu dialog control for choosing an object
from its manager (subject to an optional conditional function). Handles manager
messages to keep the menu up-to-date.
Calls the client-specified callback routine if a different object is chosen.
==============================================================================*/
#if !defined (CHOOSE_OBJECT_PRIVATE_H)
#define CHOOSE_OBJECT_PRIVATE_H

#include <stdio.h>
#include "general/debug.h"
#include "choose/choose_object.h"
#include "choose/chooser.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module variables
----------------
*/
#if defined (FULL_NAMES)
#define CHOOSE_OBJECT_( object_type ) choose_object_type_ ## object_type
#else
#define CHOOSE_OBJECT_( object_type ) cotype ## object_type
#endif
#define CHOOSE_OBJECT( object_type ) CHOOSE_OBJECT_(object_type)

#define FULL_DECLARE_CHOOSE_OBJECT_TYPE( object_type ) \
struct CHOOSE_OBJECT(object_type) \
/***************************************************************************** \
LAST MODIFIED : 21 January 2000 \
\
DESCRIPTION : \
Contains information required by the choose_object control dialog. \
============================================================================*/ \
{ \
	struct MANAGER(object_type) *object_manager; \
	MANAGER_CONDITIONAL_FUNCTION(object_type) *conditional_function; \
	void *manager_callback_id; \
	struct Chooser *chooser; \
	struct Callback_data update_callback; \
	Widget widget,parent; \
} /* struct CHOOSE_OBJECT(object_type) */

/*
Module functions
----------------
*/

#if defined (FULL_NAMES)
#define CHOOSE_OBJECT_UPDATE_( object_type ) \
	choose_object_update_ ## object_type
#else
#define CHOOSE_OBJECT_UPDATE_( object_type ) cou ## object_type
#endif
#define CHOOSE_OBJECT_UPDATE( object_type ) \
	CHOOSE_OBJECT_UPDATE_(object_type)

#define DECLARE_CHOOSE_OBJECT_UPDATE_FUNCTION( object_type ) \
static int CHOOSE_OBJECT_UPDATE(object_type)( \
	struct CHOOSE_OBJECT(object_type) *choose_object) \
/***************************************************************************** \
LAST MODIFIED : 24 January 2000 \
\
DESCRIPTION : \
Tells CMGUI about the current values. Sends a pointer to the current object. \
============================================================================*/ \
{ \
	int return_code; \
\
	ENTER(CHOOSE_OBJECT_UPDATE(object_type)); \
	if (choose_object) \
	{ \
		if (choose_object->update_callback.procedure) \
		{ \
			/* now call the procedure with the user data */ \
			(choose_object->update_callback.procedure)( \
				choose_object->widget,choose_object->update_callback.data, \
				(struct object_type *) \
				Chooser_get_item(choose_object->chooser)); \
		} \
		return_code=1; \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"CHOOSE_OBJECT_UPDATE(" #object_type ").  Invalid argument(s)"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* CHOOSE_OBJECT_UPDATE(object_type) */

#if defined (FULL_NAMES)
#define CHOOSE_OBJECT_DESTROY_CB_( object_type ) \
	choose_object_destroy_cb_ ## object_type
#else
#define CHOOSE_OBJECT_DESTROY_CB_( object_type ) codc ## object_type
#endif
#define CHOOSE_OBJECT_DESTROY_CB( object_type ) \
	CHOOSE_OBJECT_DESTROY_CB_(object_type)

#define DECLARE_CHOOSE_OBJECT_DESTROY_CB_FUNCTION( object_type ) \
static void CHOOSE_OBJECT_DESTROY_CB(object_type)(Widget widget, \
	XtPointer client_data,XtPointer reason) \
/***************************************************************************** \
LAST MODIFIED : 21 January 2000 \
\
DESCRIPTION : \
Callback for the choose_object dialog - tidies up all memory allocation. \
============================================================================*/ \
{ \
	struct CHOOSE_OBJECT(object_type) *choose_object; \
\
	ENTER(CHOOSE_OBJECT_DESTROY_CB(object_type)); \
	USE_PARAMETER(widget); \
	USE_PARAMETER(reason); \
	if (choose_object=(struct CHOOSE_OBJECT(object_type) *)client_data) \
	{ \
		DESTROY(Chooser)(&(choose_object->chooser)); \
		if (choose_object->manager_callback_id) \
		{ \
			MANAGER_DEREGISTER(object_type)( \
				choose_object->manager_callback_id, \
				choose_object->object_manager); \
		} \
		DEALLOCATE(choose_object); \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"CHOOSE_OBJECT_DESTROY_CB(" #object_type ").  Missing choose_object"); \
	} \
	LEAVE; \
} /* CHOOSE_OBJECT_DESTROY_CB(object_type) */

#if defined (FULL_NAMES)
#define CHOOSE_OBJECT_UPDATE_CB_( object_type ) \
	choose_object_update_cb_ ## object_type
#else
#define CHOOSE_OBJECT_UPDATE_CB_( object_type ) couc ## object_type
#endif
#define CHOOSE_OBJECT_UPDATE_CB( object_type ) \
	CHOOSE_OBJECT_UPDATE_CB_(object_type)

#define DECLARE_CHOOSE_OBJECT_UPDATE_CB_FUNCTION( object_type ) \
static void CHOOSE_OBJECT_UPDATE_CB(object_type)(Widget widget, \
	void *choose_object_void,void *dummy_void) \
/***************************************************************************** \
LAST MODIFIED : 21 January 2000 \
\
DESCRIPTION : \
Callback for the choose_object dialog - tidies up all memory allocation. \
============================================================================*/ \
{ \
	struct CHOOSE_OBJECT(object_type) *choose_object; \
\
	ENTER(CHOOSE_OBJECT_UPDATE_CB(object_type)); \
	USE_PARAMETER(widget); \
	USE_PARAMETER(dummy_void); \
	if (choose_object= \
		(struct CHOOSE_OBJECT(object_type) *)choose_object_void) \
	{ \
		CHOOSE_OBJECT_UPDATE(object_type)(choose_object); \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"CHOOSE_OBJECT_UPDATE_CB(" #object_type ").  Invalid argument"); \
	} \
	LEAVE; \
} /* CHOOSE_OBJECT_UPDATE_CB(object_type) */

#if defined (FULL_NAMES)
#define CHOOSE_OBJECT_ADD_TO_LIST_DATA_( object_type ) \
	choose_object_add_to_list_data_ ## object_type
#else
#define CHOOSE_OBJECT_ADD_TO_LIST_DATA_( object_type ) coatld ## object_type
#endif
#define CHOOSE_OBJECT_ADD_TO_LIST_DATA( object_type ) \
	CHOOSE_OBJECT_ADD_TO_LIST_DATA_(object_type)

#define DECLARE_CHOOSE_OBJECT_ADD_TO_LIST_DATA( object_type ) \
struct CHOOSE_OBJECT_ADD_TO_LIST_DATA(object_type) \
/***************************************************************************** \
LAST MODIFIED : 21 January 2000 \
\
DESCRIPTION : \
Data for adding objects to an allocated list. Handles conditional function. \
============================================================================*/ \
{ \
  char **item_names; \
	int number_of_items; \
	MANAGER_CONDITIONAL_FUNCTION(object_type) *conditional_function; \
  void **items; \
} /* struct CHOOSE_OBJECT_ADD_TO_LIST_DATA(object_type) */

#if defined (FULL_NAMES)
#define CHOOSE_OBJECT_ADD_TO_LIST_( object_type ) \
	choose_object_add_to_list_ ## object_type
#else
#define CHOOSE_OBJECT_ADD_TO_LIST_( object_type ) coatl ## object_type
#endif
#define CHOOSE_OBJECT_ADD_TO_LIST( object_type ) \
	CHOOSE_OBJECT_ADD_TO_LIST_(object_type)

#define DECLARE_CHOOSE_OBJECT_ADD_TO_LIST_FUNCTION( object_type ) \
static int CHOOSE_OBJECT_ADD_TO_LIST(object_type)( \
	struct object_type *object,void *add_data_void) \
/***************************************************************************** \
LAST MODIFIED : 21 January 2000 \
\
DESCRIPTION : \
Puts the <object> at the array position pointed to by <list_position>. \
============================================================================*/ \
{ \
	int return_code; \
	struct CHOOSE_OBJECT_ADD_TO_LIST_DATA(object_type) *add_data; \
 \
	ENTER(CHOOSE_OBJECT_ADD_TO_LIST(object_type)); \
	if (object&&(add_data=(struct CHOOSE_OBJECT_ADD_TO_LIST_DATA( \
		object_type) *)add_data_void)&&add_data->items&&add_data->item_names) \
	{ \
		return_code=1; \
		if (!(add_data->conditional_function)|| \
			(add_data->conditional_function)(object,(void *)NULL)) \
		{ \
			if (GET_NAME(object_type)(object,add_data->item_names + \
				add_data->number_of_items)) \
			{ \
				add_data->items[add_data->number_of_items] = (void *)object; \
				(add_data->number_of_items++); \
			} \
			else \
			{ \
				display_message(ERROR_MESSAGE,"CHOOSE_OBJECT_ADD_TO_LIST(" \
					#object_type ").  Could not get name of object"); \
				return_code=0; \
			} \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE,"CHOOSE_OBJECT_ADD_TO_LIST(" \
			#object_type ").  Invalid argument(s)"); \
		return_code=0; \
	} \
	LEAVE; \
 \
	return (return_code); \
} /* CHOOSE_OBJECT_ADD_TO_LIST(object_type) */

#if defined (FULL_NAMES)
#define CHOOSE_OBJECT_GET_ITEMS_( object_type ) \
	choose_object_get_items_ ## object_type
#else
#define CHOOSE_OBJECT_GET_ITEMS_( object_type ) cogi ## object_type
#endif
#define CHOOSE_OBJECT_GET_ITEMS( object_type ) \
	CHOOSE_OBJECT_GET_ITEMS_(object_type)

#define DECLARE_CHOOSE_OBJECT_GET_ITEMS_FUNCTION( object_type ) \
static int CHOOSE_OBJECT_GET_ITEMS(object_type)( \
	struct CHOOSE_OBJECT(object_type) *choose_object, \
	int *number_of_items,void ***items_address,char ***item_names_address) \
/***************************************************************************** \
LAST MODIFIED : 21 January 2000 \
\
DESCRIPTION : \
Allocates and fills an array of all the choosable objects and their names. \
============================================================================*/ \
{ \
	int i,max_number_of_objects,return_code; \
	struct CHOOSE_OBJECT_ADD_TO_LIST_DATA(object_type) add_to_list_data; \
\
	ENTER(CHOOSE_OBJECT_GET_ITEMS(object_type)); \
	return_code=0; \
	if (choose_object&&number_of_items&&items_address&&item_names_address) \
	{ \
		max_number_of_objects= \
			NUMBER_IN_MANAGER(object_type)(choose_object->object_manager); \
		add_to_list_data.conditional_function= \
			choose_object->conditional_function; \
		add_to_list_data.number_of_items=0; \
		add_to_list_data.items=(void **)NULL; \
		add_to_list_data.item_names=(char **)NULL; \
		if (((0==max_number_of_objects) || \
			ALLOCATE(add_to_list_data.items,void *,max_number_of_objects) && \
			ALLOCATE(add_to_list_data.item_names,char *,max_number_of_objects))&& \
			FOR_EACH_OBJECT_IN_MANAGER(object_type)( \
				CHOOSE_OBJECT_ADD_TO_LIST(object_type),(void *)&add_to_list_data, \
				choose_object->object_manager)) \
		{ \
			*number_of_items = add_to_list_data.number_of_items; \
			*items_address = add_to_list_data.items; \
			*item_names_address = add_to_list_data.item_names; \
			return_code=1; \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"CHOOSE_OBJECT_GET_ITEMS(" #object_type ").  Failed"); \
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
			"CHOOSE_OBJECT_GET_ITEMS(" #object_type ").  Invalid argument(s)"); \
	} \
	LEAVE; \
 \
	return (return_code); \
} /* CHOOSE_OBJECT_GET_ITEMS(object_type) */

#if defined (FULL_NAMES)
#define CHOOSE_OBJECT_GLOBAL_OBJECT_CHANGE_( object_type ) \
	choose_object_global_object_change_ ## object_type
#else
#define CHOOSE_OBJECT_GLOBAL_OBJECT_CHANGE_( object_type ) cogoc ## object_type
#endif
#define CHOOSE_OBJECT_GLOBAL_OBJECT_CHANGE( object_type ) \
	CHOOSE_OBJECT_GLOBAL_OBJECT_CHANGE_(object_type)

#define DECLARE_CHOOSE_OBJECT_GLOBAL_OBJECT_CHANGE_FUNCTION( object_type ) \
static void CHOOSE_OBJECT_GLOBAL_OBJECT_CHANGE(object_type)( \
	struct MANAGER_MESSAGE(object_type) *message,void *data) \
/***************************************************************************** \
LAST MODIFIED : 21 January 2000 \
\
DESCRIPTION : \
Rebuilds the choose object menu in response to manager messages. \
Tries to minimise menu rebuilds as much as possible, since these cause \
annoying flickering on the screen. \
============================================================================*/ \
{ \
	char **item_names; \
	int i,number_of_items,update_menu; \
	struct CHOOSE_OBJECT(object_type) *choose_object; \
	void **items; \
\
	ENTER(CHOOSE_OBJECT_GLOBAL_OBJECT_CHANGE(object_type)); \
	if (message&&(choose_object= \
		(struct CHOOSE_OBJECT(object_type) *)data)) \
	{ \
		update_menu=0; \
		switch (message->change) \
		{ \
			case MANAGER_CHANGE_ALL(object_type): \
			case MANAGER_CHANGE_OBJECT(object_type): \
			{ \
				/* always update the menu - these seldom, if ever happen */ \
				update_menu=1; \
			}; break; \
			case MANAGER_CHANGE_DELETE(object_type): \
			case MANAGER_CHANGE_ADD(object_type): \
			case MANAGER_CHANGE_IDENTIFIER(object_type): \
			{ \
				/* only update menu if no conditional function or included by it */ \
				if (!(choose_object->conditional_function)|| \
					(choose_object->conditional_function)(message->object_changed, \
						(void *)NULL)) \
				{ \
					update_menu=1; \
				} \
			}; break; \
			case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(object_type): \
			{ \
				/* update menu only if there is a conditional function since change \
					 may have changed object's availability to chooser. Inefficient. */ \
				if (choose_object->conditional_function) \
				{ \
					update_menu=1; \
				} \
			} break; \
		} \
		if (update_menu) \
		{ \
		  if (CHOOSE_OBJECT_GET_ITEMS(object_type)(choose_object, \
				&number_of_items,&items,&item_names)) \
			{ \
				update_menu=Chooser_build_main_menu(choose_object->chooser, \
					number_of_items,items,item_names, \
					Chooser_get_item(choose_object->chooser)); \
				if (items) \
				{ \
					DEALLOCATE(items); \
				} \
				if (item_names) \
				{  \
					for (i=0;i<number_of_items;i++) \
					{ \
						DEALLOCATE(item_names[i]); \
					} \
					DEALLOCATE(item_names); \
				} \
			} \
			else \
			{ \
				update_menu=0; \
			} \
			if (!update_menu) \
			{ \
				display_message(ERROR_MESSAGE, \
					"CHOOSE_OBJECT_GLOBAL_OBJECT_CHANGE(" #object_type \
					").  Could not update menu"); \
			} \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"CHOOSE_OBJECT_GLOBAL_OBJECT_CHANGE(" #object_type \
			").  Invalid argument(s)"); \
	} \
	LEAVE; \
} /* CHOOSE_OBJECT_GLOBAL_OBJECT_CHANGE(object_type) */

/*
Global functions
----------------
*/

#define DECLARE_CREATE_CHOOSE_OBJECT_WIDGET_FUNCTION( object_type ) \
PROTOTYPE_CREATE_CHOOSE_OBJECT_WIDGET_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 21 January 2000 \
\
DESCRIPTION : \
Creates an option menu from which an object from the manager may be chosen. \
The optional conditional function permits a subset of objects in the manager \
to be selectable. \
============================================================================*/ \
{ \
	char **item_names; \
	int i,number_of_items; \
	struct Callback_data callback; \
	struct CHOOSE_OBJECT(object_type) *choose_object; \
	void **items; \
	Widget return_widget; \
\
	ENTER(CREATE_CHOOSE_OBJECT_WIDGET(object_type)); \
	return_widget=(Widget)NULL; \
	if (object_manager&&parent) \
	{ \
		if (ALLOCATE(choose_object,struct CHOOSE_OBJECT(object_type),1)) \
		{ \
			/* initialise the structure */ \
			choose_object->manager_callback_id=(void *)NULL; \
			choose_object->chooser=(struct Chooser *)NULL; \
			choose_object->widget=(Widget)NULL; \
			choose_object->parent=parent; \
			choose_object->update_callback.procedure= \
				(Callback_procedure *)NULL; \
			choose_object->update_callback.data=(void *)NULL; \
			choose_object->object_manager=object_manager; \
			choose_object->conditional_function=conditional_function; \
		  if (CHOOSE_OBJECT_GET_ITEMS(object_type)(choose_object, \
				&number_of_items,&items,&item_names)) \
			{ \
				if (choose_object->chooser= \
					CREATE(Chooser)(parent,number_of_items,items,item_names, \
					(void *)current_object,&(choose_object->widget))) \
				{ \
					/* add choose_object as user data to chooser widget */ \
					XtVaSetValues(choose_object->widget,XmNuserData,choose_object,NULL); \
					/* add destroy callback for chooser widget */ \
					XtAddCallback(choose_object->widget,XmNdestroyCallback, \
						CHOOSE_OBJECT_DESTROY_CB(object_type),(XtPointer)choose_object); \
					/* get updates when chooser changes */ \
					callback.data=(void *)choose_object; \
					callback.procedure=CHOOSE_OBJECT_UPDATE_CB(object_type); \
					Chooser_set_update_callback(choose_object->chooser, \
						&callback); \
					/* register for any object changes */ \
					choose_object->manager_callback_id= \
						MANAGER_REGISTER(object_type)( \
						CHOOSE_OBJECT_GLOBAL_OBJECT_CHANGE(object_type), \
						(void *)choose_object,choose_object->object_manager); \
					return_widget=choose_object->widget; \
				} \
				else \
				{ \
					display_message(ERROR_MESSAGE, \
						"CREATE_CHOOSE_OBJECT_WIDGET(" #object_type \
						").  Could not create chooser"); \
					DEALLOCATE(choose_object); \
				} \
				if (items) \
				{ \
					DEALLOCATE(items); \
				} \
				if (item_names) \
				{  \
					for (i=0;i<number_of_items;i++) \
					{ \
						DEALLOCATE(item_names[i]); \
					} \
					DEALLOCATE(item_names); \
				} \
			} \
			else \
			{ \
				display_message(ERROR_MESSAGE, \
					"CREATE_CHOOSE_OBJECT_WIDGET(" #object_type \
					").  Could not get items"); \
				DEALLOCATE(choose_object); \
			} \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"CREATE_CHOOSE_OBJECT_WIDGET(" #object_type \
				").  Not enough memory"); \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"CREATE_CHOOSE_OBJECT_WIDGET(" #object_type \
			").  Invalid argument(s)"); \
	} \
	LEAVE; \
\
	return (return_widget); \
} /* CREATE_CHOOSE_OBJECT_WIDGET(object_type) */

#define DECLARE_CHOOSE_OBJECT_GET_CALLBACK_FUNCTION( object_type ) \
PROTOTYPE_CHOOSE_OBJECT_GET_CALLBACK_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 21 January 2000 \
\
DESCRIPTION : \
Returns a pointer to the callback item of the choose_object_widget. \
============================================================================*/ \
{ \
	struct Callback_data *callback; \
	struct CHOOSE_OBJECT(object_type) *choose_object; \
\
	ENTER(CHOOSE_OBJECT_GET_CALLBACK(object_type)); \
	if (choose_object_widget) \
	{ \
		/* Get the pointer to the data for the choose_object dialog */ \
		XtVaGetValues(choose_object_widget,XmNuserData,&choose_object,NULL); \
		if (choose_object) \
		{ \
			callback=&(choose_object->update_callback); \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"CHOOSE_OBJECT_GET_CALLBACK(" #object_type \
				").  Missing widget data"); \
			callback=(struct Callback_data *)NULL; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"CHOOSE_OBJECT_GET_CALLBACK(" #object_type ").  Missing widget"); \
		callback=(struct Callback_data *)NULL; \
	} \
	LEAVE; \
\
	return (callback); \
} /* CHOOSE_OBJECT_GET_CALLBACK(object_type) */

#define DECLARE_CHOOSE_OBJECT_SET_CALLBACK_FUNCTION( object_type ) \
PROTOTYPE_CHOOSE_OBJECT_SET_CALLBACK_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 21 January 2000 \
\
DESCRIPTION : \
Changes the callback item of the choose_object_widget. \
============================================================================*/ \
{ \
	int return_code; \
	struct CHOOSE_OBJECT(object_type) *choose_object; \
\
	ENTER(CHOOSE_OBJECT_SET_CALLBACK(object_type)); \
	if (choose_object_widget&&new_callback) \
	{ \
		/* Get the pointer to the data for the choose_object dialog */ \
		XtVaGetValues(choose_object_widget,XmNuserData,&choose_object,NULL); \
		if (choose_object) \
		{ \
			choose_object->update_callback.procedure=new_callback->procedure; \
			choose_object->update_callback.data=new_callback->data; \
			return_code=1; \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"CHOOSE_OBJECT_SET_CALLBACK(" #object_type \
				").  Missing widget data"); \
			return_code=0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"CHOOSE_OBJECT_SET_CALLBACK(" #object_type ").  Missing widget"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* CHOOSE_OBJECT_SET_CALLBACK(object_type) */

#define DECLARE_CHOOSE_OBJECT_GET_OBJECT_FUNCTION( object_type ) \
PROTOTYPE_CHOOSE_OBJECT_GET_OBJECT_FUNCTION( object_type ) \
/***************************************************************************** \
LAST MODIFIED : 21 January 2000 \
\
DESCRIPTION : \
Returns the currently chosen object in the choose_object_widget. \
============================================================================*/ \
{ \
	struct object_type *object; \
	struct CHOOSE_OBJECT(object_type) *choose_object; \
\
	ENTER(CHOOSE_OBJECT_GET_OBJECT(object_type)); \
	if (choose_object_widget) \
	{ \
		/* Get the pointer to the data for the choose_object dialog */ \
		XtVaGetValues(choose_object_widget,XmNuserData,&choose_object,NULL); \
		if (choose_object) \
		{ \
			object=(struct object_type *)Chooser_get_item(choose_object->chooser); \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"CHOOSE_OBJECT_GET_OBJECT(" #object_type ").  Missing widget data"); \
			object=(struct object_type *)NULL; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"CHOOSE_OBJECT_GET_OBJECT(" #object_type ").  Missing widget"); \
		object=(struct object_type *)NULL; \
	} \
	LEAVE; \
\
	return (object); \
} /* CHOOSE_OBJECT_GET_OBJECT(object_type) */

#define DECLARE_CHOOSE_OBJECT_SET_OBJECT_FUNCTION( object_type ) \
PROTOTYPE_CHOOSE_OBJECT_SET_OBJECT_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 21 January 2000 \
\
DESCRIPTION : \
Changes the chosen object in the choose_object_widget. \
============================================================================*/ \
{ \
	int return_code; \
	struct CHOOSE_OBJECT(object_type) *choose_object; \
\
	ENTER(CHOOSE_OBJECT_SET_OBJECT(object_type)); \
	if (choose_object_widget) \
	{ \
		/* Get the pointer to the data for the choose_object dialog */ \
		XtVaGetValues(choose_object_widget,XmNuserData,&choose_object,NULL); \
		if (choose_object) \
		{ \
			return_code=Chooser_set_item(choose_object->chooser,(void *)new_object); \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"CHOOSE_OBJECT_SET_OBJECT(" #object_type ").  Missing widget data"); \
			return_code=0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"CHOOSE_OBJECT_SET_OBJECT(" #object_type ").  Missing widget"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* CHOOSE_OBJECT_SET_OBJECT(object_type) */

#define DECLARE_CHOOSE_OBJECT_MODULE_FUNCTIONS( object_type ) \
DECLARE_CHOOSE_OBJECT_UPDATE_FUNCTION(object_type) \
DECLARE_CHOOSE_OBJECT_DESTROY_CB_FUNCTION(object_type) \
DECLARE_CHOOSE_OBJECT_UPDATE_CB_FUNCTION(object_type) \
DECLARE_CHOOSE_OBJECT_ADD_TO_LIST_DATA(object_type); \
DECLARE_CHOOSE_OBJECT_ADD_TO_LIST_FUNCTION(object_type) \
DECLARE_CHOOSE_OBJECT_GET_ITEMS_FUNCTION(object_type) \
DECLARE_CHOOSE_OBJECT_GLOBAL_OBJECT_CHANGE_FUNCTION(object_type)

#define DECLARE_CHOOSE_OBJECT_GLOBAL_FUNCTIONS( object_type ) \
DECLARE_CREATE_CHOOSE_OBJECT_WIDGET_FUNCTION(object_type) \
DECLARE_CHOOSE_OBJECT_GET_CALLBACK_FUNCTION(object_type) \
DECLARE_CHOOSE_OBJECT_SET_CALLBACK_FUNCTION(object_type) \
DECLARE_CHOOSE_OBJECT_GET_OBJECT_FUNCTION(object_type) \
DECLARE_CHOOSE_OBJECT_SET_OBJECT_FUNCTION(object_type)

#endif /* !defined (CHOOSE_OBJECT_PRIVATE_H) */
