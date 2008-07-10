/*******************************************************************************
FILE : choose_object_private.h

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
#if ! defined (SHORT_NAMES)
#define CHOOSE_OBJECT( object_type ) choose_object_type_ ## object_type
#else
#define CHOOSE_OBJECT( object_type ) cotype ## object_type
#endif

#define FULL_DECLARE_CHOOSE_OBJECT_TYPE( object_type ) \
struct CHOOSE_OBJECT(object_type) \
/***************************************************************************** \
LAST MODIFIED : 18 April 2000 \
\
DESCRIPTION : \
Contains information required by the choose_object control dialog. \
============================================================================*/ \
{ \
	struct MANAGER(object_type) *object_manager; \
	MANAGER_CONDITIONAL_FUNCTION(object_type) *conditional_function; \
	void *conditional_function_user_data; \
	void *manager_callback_id; \
	struct Chooser *chooser; \
	struct Callback_data update_callback; \
	Widget widget,parent; \
   int number_of_items; \
   void **items; \
   char **item_names; \
} /* struct CHOOSE_OBJECT(object_type) */

/*
Module functions
----------------
*/

#if ! defined (SHORT_NAMES)
#define CHOOSE_OBJECT_IS_ITEM_IN_CHOOSER( object_type ) \
	choose_object_is_in_chooser_ ## object_type
#else
#define CHOOSE_OBJECT_IS_ITEM_IN_CHOOSER( object_type ) coisc ## object_type
#endif

#define DECLARE_CHOOSE_OBJECT_IS_ITEM_IN_CHOOSER_FUNCTION( object_type ) \
static int CHOOSE_OBJECT_IS_ITEM_IN_CHOOSER(object_type)( \
	struct CHOOSE_OBJECT(object_type) *choose_object, \
   struct object_type *object) \
/***************************************************************************** \
LAST MODIFIED : 6 July 2000 \
\
DESCRIPTION : \
Tells CMGUI about the current values. Sends a pointer to the current object. \
============================================================================*/ \
{ \
	int i, return_code; \
\
	ENTER(CHOOSE_OBJECT_IS_ITEM_IN_CHOOSER(object_type)); \
	if (choose_object && object) \
	{ \
      return_code = 0; \
		for (i = 0 ; !return_code && (i < choose_object->number_of_items) ; i++) \
		{ \
			if (object == choose_object->items[i]) \
			{ \
				return_code=1; \
			} \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"CHOOSE_OBJECT_IS_ITEM_IN_CHOOSER(" #object_type ").  Invalid argument(s)"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* CHOOSE_OBJECT_IS_ITEM_IN_CHOOSER(object_type) */

#if ! defined (SHORT_NAMES)
#define CHOOSE_OBJECT_UPDATE( object_type ) \
	choose_object_update_ ## object_type
#else
#define CHOOSE_OBJECT_UPDATE( object_type ) cou ## object_type
#endif

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

#if ! defined (SHORT_NAMES)
#define CHOOSE_OBJECT_DESTROY_CB( object_type ) \
	choose_object_destroy_cb_ ## object_type
#else
#define CHOOSE_OBJECT_DESTROY_CB( object_type ) codc ## object_type
#endif

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
   int i; \
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
      if (choose_object->items) \
      { \
			DEALLOCATE(choose_object->items); \
      } \
      if (choose_object->item_names) \
      { \
			for (i=0;i<choose_object->number_of_items;i++) \
			{ \
				DEALLOCATE(choose_object->item_names[i]); \
			} \
		   DEALLOCATE(choose_object->item_names); \
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

#if ! defined (SHORT_NAMES)
#define CHOOSE_OBJECT_UPDATE_CB( object_type ) \
	choose_object_update_cb_ ## object_type
#else
#define CHOOSE_OBJECT_UPDATE_CB( object_type ) couc ## object_type
#endif

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

#if ! defined (SHORT_NAMES)
#define CHOOSE_OBJECT_ADD_TO_LIST_DATA( object_type ) \
	choose_object_add_to_list_data_ ## object_type
#else
#define CHOOSE_OBJECT_ADD_TO_LIST_DATA( object_type ) coatld ## object_type
#endif

#define DECLARE_CHOOSE_OBJECT_ADD_TO_LIST_DATA( object_type ) \
struct CHOOSE_OBJECT_ADD_TO_LIST_DATA(object_type) \
/***************************************************************************** \
LAST MODIFIED : 18 April 2000 \
\
DESCRIPTION : \
Data for adding objects to an allocated list. Handles conditional function. \
============================================================================*/ \
{ \
  char **item_names; \
	int number_of_items; \
	MANAGER_CONDITIONAL_FUNCTION(object_type) *conditional_function; \
	void *conditional_function_user_data; \
  void **items; \
} /* struct CHOOSE_OBJECT_ADD_TO_LIST_DATA(object_type) */

#if ! defined (SHORT_NAMES)
#define CHOOSE_OBJECT_ADD_TO_LIST( object_type ) \
	choose_object_add_to_list_ ## object_type
#else
#define CHOOSE_OBJECT_ADD_TO_LIST( object_type ) coatl ## object_type
#endif

#define DECLARE_CHOOSE_OBJECT_ADD_TO_LIST_FUNCTION( object_type ) \
static int CHOOSE_OBJECT_ADD_TO_LIST(object_type)( \
	struct object_type *object,void *add_data_void) \
/***************************************************************************** \
LAST MODIFIED : 18 April 2000 \
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

#if ! defined (SHORT_NAMES)
#define CHOOSE_OBJECT_BUILD_ITEMS( object_type ) \
	choose_object_get_items_ ## object_type
#else
#define CHOOSE_OBJECT_BUILD_ITEMS( object_type ) cogi ## object_type
#endif

#define DECLARE_CHOOSE_OBJECT_BUILD_ITEMS_FUNCTION( object_type ) \
static int CHOOSE_OBJECT_BUILD_ITEMS(object_type)( \
	struct CHOOSE_OBJECT(object_type) *choose_object) \
/***************************************************************************** \
LAST MODIFIED : 6 July 2000 \
\
DESCRIPTION : \
Updates the arrays of all the choosable objects and their names. \
============================================================================*/ \
{ \
	int i,max_number_of_objects,return_code; \
	struct CHOOSE_OBJECT_ADD_TO_LIST_DATA(object_type) add_to_list_data; \
\
	ENTER(CHOOSE_OBJECT_BUILD_ITEMS(object_type)); \
	return_code=0; \
	if (choose_object) \
	{ \
		max_number_of_objects= \
			NUMBER_IN_MANAGER(object_type)(choose_object->object_manager); \
		add_to_list_data.conditional_function= \
			choose_object->conditional_function; \
		add_to_list_data.conditional_function_user_data= \
			choose_object->conditional_function_user_data; \
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
         if (choose_object->items) \
         { \
				DEALLOCATE(choose_object->items); \
         } \
         if (choose_object->item_names) \
         { \
				for (i=0;i<choose_object->number_of_items;i++) \
				{ \
					DEALLOCATE(choose_object->item_names[i]); \
				} \
			   DEALLOCATE(choose_object->item_names); \
         } \
			choose_object->number_of_items = add_to_list_data.number_of_items; \
			choose_object->items = add_to_list_data.items; \
			choose_object->item_names = add_to_list_data.item_names; \
			return_code=1; \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"CHOOSE_OBJECT_BUILD_ITEMS(" #object_type ").  Failed"); \
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
			"CHOOSE_OBJECT_BUILD_ITEMS(" #object_type ").  Invalid argument(s)"); \
	} \
	LEAVE; \
 \
	return (return_code); \
} /* CHOOSE_OBJECT_BUILD_ITEMS(object_type) */

#if ! defined (SHORT_NAMES)
#define CHOOSE_OBJECT_OBJECT_STATUS_CHANGED( object_type ) \
	choose_object_object_status_changed_ ## object_type
#else
#define CHOOSE_OBJECT_OBJECT_STATUS_CHANGED( object_type ) coosc ## object_type
#endif

#define DECLARE_CHOOSE_OBJECT_OBJECT_STATUS_CHANGED_FUNCTION( object_type ) \
static int CHOOSE_OBJECT_OBJECT_STATUS_CHANGED(object_type)( \
	struct object_type *object, void *choose_object_void) \
/***************************************************************************** \
LAST MODIFIED : 18 May 2001 \
\
DESCRIPTION : \
Returns true if <object> is in the chooser but should not be, or is not in the \
chooser and should be. \
============================================================================*/ \
{ \
	int object_is_in_chooser, object_should_be_in_chooser, return_code; \
	struct CHOOSE_OBJECT(object_type) *choose_object; \
 \
	ENTER(CHOOSE_OBJECT_OBJECT_STATUS_CHANGED(object_type)); \
	if (object && (choose_object = \
		(struct CHOOSE_OBJECT(object_type) *)choose_object_void)) \
	{ \
		if (choose_object->conditional_function) \
		{ \
			object_is_in_chooser = CHOOSE_OBJECT_IS_ITEM_IN_CHOOSER(object_type)( \
				choose_object, object); \
			object_should_be_in_chooser = \
				(choose_object->conditional_function)(object, \
					choose_object->conditional_function_user_data); \
			return_code = \
				(object_is_in_chooser && (!object_should_be_in_chooser)) || \
				((!object_is_in_chooser) && object_should_be_in_chooser); \
		} \
		else \
		{ \
			return_code = 0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE,"CHOOSE_OBJECT_OBJECT_STATUS_CHANGED(" \
			#object_type ").  Invalid argument(s)"); \
		return_code = 0; \
	} \
	LEAVE; \
 \
	return (return_code); \
} /* CHOOSE_OBJECT_OBJECT_STATUS_CHANGED(object_type) */

#if ! defined (SHORT_NAMES)
#define CHOOSE_OBJECT_GLOBAL_OBJECT_CHANGE( object_type ) \
	choose_object_global_object_change_ ## object_type
#else
#define CHOOSE_OBJECT_GLOBAL_OBJECT_CHANGE( object_type ) cogoc ## object_type
#endif

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
	int update_menu; \
	struct CHOOSE_OBJECT(object_type) *choose_object; \
\
	ENTER(CHOOSE_OBJECT_GLOBAL_OBJECT_CHANGE(object_type)); \
	if (message && (choose_object= \
		(struct CHOOSE_OBJECT(object_type) *)data)) \
	{ \
		update_menu = 0; \
		if ((MANAGER_CHANGE_ADD(object_type) == message->change) || \
			(MANAGER_CHANGE_REMOVE(object_type) == message->change) || \
			(MANAGER_CHANGE_IDENTIFIER(object_type) == message->change) || \
			(MANAGER_CHANGE_OBJECT(object_type) == message->change)) \
		{ \
			/* ensure menu updates if no conditional function, or  \
				 conditional function satisfied for any of the changed objects */ \
			if (!(choose_object->conditional_function) || \
				((struct object_type *)NULL != \
					FIRST_OBJECT_IN_LIST_THAT(object_type)( \
						choose_object->conditional_function, \
						choose_object->conditional_function_user_data, \
						message->changed_object_list))) \
			{ \
					update_menu = 1; \
			} \
		} \
		if ((MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(object_type) == message->change) \
			|| (MANAGER_CHANGE_OBJECT(object_type) == message->change)) \
		{ \
			/* ensure menu updates if there is no conditional function and the \
				 status of an object in the chooser has changed */ \
			if (choose_object->conditional_function && \
				((struct object_type *)NULL != \
					FIRST_OBJECT_IN_LIST_THAT(object_type)( \
						CHOOSE_OBJECT_OBJECT_STATUS_CHANGED(object_type), \
						(void *)choose_object, message->changed_object_list))) \
			{ \
				update_menu = 1; \
			} \
		} \
		if (update_menu) \
		{ \
		  if (!(CHOOSE_OBJECT_BUILD_ITEMS(object_type)(choose_object) && \
				Chooser_build_main_menu(choose_object->chooser, \
					choose_object->number_of_items,choose_object->items, \
					choose_object->item_names, \
					Chooser_get_item(choose_object->chooser)))) \
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
	struct CHOOSE_OBJECT(object_type) *choose_object; \
	Widget return_widget; \
\
	ENTER(CREATE_CHOOSE_OBJECT_WIDGET(object_type)); \
	return_widget=(Widget)NULL; \
	if (object_manager && parent && user_interface) \
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
			choose_object->conditional_function_user_data= \
				conditional_function_user_data; \
         choose_object->number_of_items = 0; \
         choose_object->items = (void **)NULL; \
         choose_object->item_names = (char **)NULL; \
		  if (CHOOSE_OBJECT_BUILD_ITEMS(object_type)(choose_object)) \
			{ \
				if (choose_object->chooser = \
					CREATE(Chooser)(parent, choose_object->number_of_items, \
					choose_object->items, choose_object->item_names, \
					(void *)current_object, &(choose_object->widget), \
					user_interface)) \
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
LAST MODIFIED : 9 June 2000 \
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
		choose_object=(struct CHOOSE_OBJECT(object_type) *)NULL; \
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
LAST MODIFIED : 9 June 2000 \
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
		choose_object=(struct CHOOSE_OBJECT(object_type) *)NULL; \
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
LAST MODIFIED : 9 June 2000 \
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
		choose_object=(struct CHOOSE_OBJECT(object_type) *)NULL; \
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
LAST MODIFIED : 9 June 2000 \
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
		choose_object=(struct CHOOSE_OBJECT(object_type) *)NULL; \
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

#define DECLARE_CHOOSE_OBJECT_CHANGE_CONDITIONAL_FUNCTION_FUNCTION( \
	object_type ) \
PROTOTYPE_CHOOSE_OBJECT_CHANGE_CONDITIONAL_FUNCTION_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 9 June 2000 \
\
DESCRIPTION : \
Changes the conditional_function and user_data limiting the available \
selection of objects. Also allows new_object to be set simultaneously. \
============================================================================*/ \
{ \
	int return_code; \
	struct CHOOSE_OBJECT(object_type) *choose_object; \
\
	ENTER(CHOOSE_OBJECT_CHANGE_CONDITIONAL_FUNCTION(object_type)); \
	if (choose_object_widget) \
	{ \
		choose_object=(struct CHOOSE_OBJECT(object_type) *)NULL; \
		/* Get the pointer to the data for the choose_object dialog */ \
		XtVaGetValues(choose_object_widget,XmNuserData,&choose_object,NULL); \
		if (choose_object) \
		{ \
			choose_object->conditional_function=conditional_function; \
			choose_object->conditional_function_user_data= \
				conditional_function_user_data; \
		  if (CHOOSE_OBJECT_BUILD_ITEMS(object_type)(choose_object)) \
			{ \
				return_code=Chooser_build_main_menu(choose_object->chooser, \
					choose_object->number_of_items,choose_object->items, \
				   choose_object->item_names,(void *)new_object); \
			} \
			else \
			{ \
				return_code=0; \
			} \
			if (!return_code) \
			{ \
				display_message(ERROR_MESSAGE, \
					"CHOOSE_OBJECT_CHANGE_CONDITIONAL_FUNCTION(" #object_type \
					").  Could not update menu"); \
			} \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"CHOOSE_OBJECT_CHANGE_CONDITIONAL_FUNCTION(" #object_type \
				").  Missing widget data"); \
			return_code=0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"CHOOSE_OBJECT_CHANGE_CONDITIONAL_FUNCTION(" #object_type \
			").  Missing widget"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* CHOOSE_OBJECT_CHANGE_CONDITIONAL_FUNCTION(object_type) */

#define DECLARE_CHOOSE_OBJECT_CHANGE_MANAGER_FUNCTION(	\
	object_type ) \
PROTOTYPE_CHOOSE_OBJECT_CHANGE_MANAGER_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 17 June 2008 \
\
DESCRIPTION : \
AW: Changes the manager and user_data limiting the available \
selection of objects. Also allows new_object to be set simultaneously. \
============================================================================*/ \
{ \
	int return_code; \
	struct CHOOSE_OBJECT(object_type) *choose_object; \
\
	ENTER(CHOOSE_OBJECT_CHANGE_MANAGER(object_type)); \
	if (choose_object_widget) \
	{ \
		choose_object=(struct CHOOSE_OBJECT(object_type) *)NULL; \
		/* Get the pointer to the data for the choose_object dialog */ \
		XtVaGetValues(choose_object_widget,XmNuserData,&choose_object,NULL); \
		if (choose_object) \
		{ \
			if (choose_object->object_manager != object_manager) \
			{ \
			  choose_object->object_manager=object_manager; \
				if (CHOOSE_OBJECT_BUILD_ITEMS(object_type)(choose_object)) \
				{ \
				 	return_code=Chooser_build_main_menu(choose_object->chooser, \
				 		 choose_object->number_of_items,choose_object->items, \
					 	 choose_object->item_names,(void *)new_object); \
				} \
				else	\
				{ \
				 	return_code=0; \
				} \
			} \
			else	\
			{ \
			 	return_code=1; \
			} \
			if (!return_code)												\
			{ \
				display_message(ERROR_MESSAGE, \
					"CHOOSE_OBJECT_CHANGE_MANAGER(" #object_type \
					").  Could not update menu"); \
			} \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"CHOOSE_OBJECT_CHANGE_MANAGER(" #object_type \
				").  Missing widget data"); \
			return_code=0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"CHOOSE_OBJECT_CHANGE_MANAGER(" #object_type \
			").  Missing widget"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* CHOOSE_OBJECT_CHANGE_MANAGER_FUNCTION(object_type) */

#define DECLARE_CHOOSE_OBJECT_MODULE_FUNCTIONS( object_type ) \
DECLARE_CHOOSE_OBJECT_IS_ITEM_IN_CHOOSER_FUNCTION(object_type) \
DECLARE_CHOOSE_OBJECT_UPDATE_FUNCTION(object_type) \
DECLARE_CHOOSE_OBJECT_DESTROY_CB_FUNCTION(object_type) \
DECLARE_CHOOSE_OBJECT_UPDATE_CB_FUNCTION(object_type) \
DECLARE_CHOOSE_OBJECT_ADD_TO_LIST_DATA(object_type); \
DECLARE_CHOOSE_OBJECT_ADD_TO_LIST_FUNCTION(object_type) \
DECLARE_CHOOSE_OBJECT_BUILD_ITEMS_FUNCTION(object_type) \
DECLARE_CHOOSE_OBJECT_OBJECT_STATUS_CHANGED_FUNCTION(object_type) \
DECLARE_CHOOSE_OBJECT_GLOBAL_OBJECT_CHANGE_FUNCTION(object_type)

#define DECLARE_CHOOSE_OBJECT_GLOBAL_FUNCTIONS( object_type ) \
DECLARE_CREATE_CHOOSE_OBJECT_WIDGET_FUNCTION(object_type) \
DECLARE_CHOOSE_OBJECT_GET_CALLBACK_FUNCTION(object_type) \
DECLARE_CHOOSE_OBJECT_SET_CALLBACK_FUNCTION(object_type) \
DECLARE_CHOOSE_OBJECT_GET_OBJECT_FUNCTION(object_type) \
DECLARE_CHOOSE_OBJECT_SET_OBJECT_FUNCTION(object_type) \
DECLARE_CHOOSE_OBJECT_CHANGE_CONDITIONAL_FUNCTION_FUNCTION(object_type)\
DECLARE_CHOOSE_OBJECT_CHANGE_MANAGER_FUNCTION(object_type)
#endif /* !defined (CHOOSE_OBJECT_PRIVATE_H) */
