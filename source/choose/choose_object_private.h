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

#include <string.h>
#if defined (MOTIF)
#include <Xm/Xm.h>
#include <Xm/PushBG.h>
#include <Xm/CascadeBG.h>
#include <Xm/RowColumn.h>
#endif /* defined (MOTIF) */
#include "general/debug.h"
#include "general/mystring.h"
#include "choose/choose_object.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

#define MAX_CHOOSER_ROWS 16

/*
Module variables
----------------
*/
#if defined (FULL_NAMES)
#define CHOOSE_OBJECT_STRUCT_( object_type ) \
	choose_object_struct_ ## object_type
#else
#define CHOOSE_OBJECT_STRUCT_( object_type ) costru ## object_type
#endif
#define CHOOSE_OBJECT_STRUCT( object_type ) CHOOSE_OBJECT_STRUCT_(object_type)

#define FULL_DECLARE_CHOOSE_OBJECT_STRUCT_TYPE( object_type ) \
struct CHOOSE_OBJECT_STRUCT(object_type) \
/***************************************************************************** \
LAST MODIFIED : 7 September 1999 \
\
DESCRIPTION : \
Contains information required by the choose_object control dialog. \
============================================================================*/ \
{ \
	struct object_type *current_object,*last_updated_object; \
	struct MANAGER(object_type) *object_manager; \
	MANAGER_CONDITIONAL_FUNCTION(object_type) *conditional_function; \
	void *manager_callback_id; \
	struct Callback_data update_callback; \
	Widget main_cascade,main_menu,widget,widget_parent; \
} /* struct CHOOSE_OBJECT_STRUCT(object_type) */

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
#define CHOOSE_OBJECT_UPDATE( object_type ) CHOOSE_OBJECT_UPDATE_(object_type)

#define DECLARE_CHOOSE_OBJECT_UPDATE_FUNCTION( object_type ) \
static int CHOOSE_OBJECT_UPDATE(object_type)( \
	struct CHOOSE_OBJECT_STRUCT(object_type) *choose_object) \
/***************************************************************************** \
LAST MODIFIED : 8 September 1999 \
\
DESCRIPTION : \
Tells CMGUI about the current values. Sends a pointer to the current object. \
Avoids sending repeated updates if the object address has not changed. \
============================================================================*/ \
{ \
	int return_code; \
\
	ENTER(CHOOSE_OBJECT_UPDATE(object_type)); \
	if (choose_object) \
	{ \
		if (choose_object->current_object != \
			choose_object->last_updated_object) \
		{ \
			if (choose_object->update_callback.procedure) \
			{ \
				/* now call the procedure with the user data */ \
				(choose_object->update_callback.procedure)( \
					choose_object->widget,choose_object->update_callback.data, \
					choose_object->current_object); \
			} \
			choose_object->last_updated_object= \
				choose_object->current_object; \
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
#define CHOOSE_OBJECT_SELECT_MENU_ITEM_( object_type ) \
	choose_object_select_menu_item_ ## object_type
#else
#define CHOOSE_OBJECT_SELECT_MENU_ITEM_( object_type ) cosmi ## object_type
#endif
#define CHOOSE_OBJECT_SELECT_MENU_ITEM( object_type ) \
	CHOOSE_OBJECT_SELECT_MENU_ITEM_(object_type)

#define DECLARE_CHOOSE_OBJECT_SELECT_MENU_ITEM_FUNCTION( object_type ) \
static int CHOOSE_OBJECT_SELECT_MENU_ITEM(object_type)( \
	struct CHOOSE_OBJECT_STRUCT(object_type) *choose_object) \
/***************************************************************************** \
LAST MODIFIED : 21 October 1999 \
\
DESCRIPTION : \
Finds the current_object in the manager and selects it. If the current_object \
is NULL or not managed, the first item in the manager is selected. \
The client is informed if the current object changes inside this routine. \
============================================================================*/ \
{ \
	char *null_object_name="<none>"; \
	char *object_name; \
	int return_code; \
	XmString new_string; \
\
	ENTER(CHOOSE_OBJECT_SELECT_MENU_ITEM(object_type)); \
	if (choose_object) \
	{ \
		if ((!choose_object->current_object)||!IS_MANAGED(object_type)( \
			choose_object->current_object,choose_object->object_manager)) \
		{ \
			choose_object->current_object=FIRST_OBJECT_IN_MANAGER_THAT(object_type)( \
				choose_object->conditional_function,(void *)NULL, \
				choose_object->object_manager); \
		} \
		if (choose_object->current_object) \
		{ \
			GET_NAME(object_type)(choose_object->current_object,&object_name); \
			new_string=XmStringCreateSimple(object_name); \
			DEALLOCATE(object_name); \
		} \
		else \
		{ \
			new_string=XmStringCreateSimple(null_object_name); \
		} \
		XtVaSetValues(choose_object->main_cascade, \
			XmNlabelString,(XtPointer)new_string,NULL); \
		XmStringFree(new_string); \
		/* inform the client of the change */ \
		CHOOSE_OBJECT_UPDATE(object_type)(choose_object); \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"CHOOSE_OBJECT_SELECT_MENU_ITEM(" #object_type \
			").  Invalid argument(s)"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* CHOOSE_OBJECT_SELECT_MENU_ITEM(object_type) */

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
LAST MODIFIED : 9 September 1999 \
\
DESCRIPTION : \
Callback for the choose_object dialog - tidies up all memory allocation. \
============================================================================*/ \
{ \
	struct CHOOSE_OBJECT_STRUCT(object_type) *choose_object; \
\
	ENTER(CHOOSE_OBJECT_DESTROY_CB(object_type)); \
	USE_PARAMETER(widget); \
	USE_PARAMETER(reason); \
	if (choose_object=(struct CHOOSE_OBJECT_STRUCT(object_type) *)client_data) \
	{ \
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
#define CHOOSE_OBJECT_MENU_CB_( object_type ) \
	choose_object_menu_cb_ ## object_type
#else
#define CHOOSE_OBJECT_MENU_CB_( object_type ) comc ## object_type
#endif
#define CHOOSE_OBJECT_MENU_CB( object_type ) \
	CHOOSE_OBJECT_MENU_CB_(object_type)

#define DECLARE_CHOOSE_OBJECT_MENU_CB_FUNCTION( object_type ) \
static void CHOOSE_OBJECT_MENU_CB(object_type)(Widget widget, \
	XtPointer client_data,XtPointer call_data) \
/***************************************************************************** \
LAST MODIFIED : 8 September 1999 \
\
DESCRIPTION : \
Callback for the option menu - change of object. \
============================================================================*/ \
{ \
	struct CHOOSE_OBJECT_STRUCT(object_type) *choose_object; \
	Widget menu_item_widget; \
\
	ENTER(CHOOSE_OBJECT_MENU_CB(object_type)); \
	if (widget&&(choose_object= \
		(struct CHOOSE_OBJECT_STRUCT(object_type) *)client_data)) \
	{ \
		/* get the widget from the call data */ \
		if (menu_item_widget=((XmRowColumnCallbackStruct *)call_data)->widget) \
		{ \
			/* Get the object this menu item represents and make it current */ \
			XtVaGetValues(menu_item_widget,XmNuserData, \
				&(choose_object->current_object),NULL); \
			/* inform the client of the change */ \
			/* Always want an update if menu clicked on, so change history: */ \
			choose_object->last_updated_object=(struct object_type *)NULL; \
			CHOOSE_OBJECT_SELECT_MENU_ITEM(object_type)(choose_object); \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"CHOOSE_OBJECT_MENU_CB(" #object_type \
				").  Could not find the activated menu item"); \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"CHOOSE_OBJECT_MENU_CB(" #object_type ").  Invalid argument(s)"); \
	} \
	LEAVE; \
} /* CHOOSE_OBJECT_MENU_CB(object_type) */

#if defined (FULL_NAMES)
#define CHOOSE_OBJECT_BUILD_MENU_( object_type ) \
	choose_object_build_menu_ ## object_type
#else
#define CHOOSE_OBJECT_BUILD_MENU_( object_type ) cobm ## object_type
#endif
#define CHOOSE_OBJECT_BUILD_MENU( object_type ) \
	CHOOSE_OBJECT_BUILD_MENU_(object_type)

#define DECLARE_CHOOSE_OBJECT_BUILD_MENU_FUNCTION( object_type ) \
static Widget CHOOSE_OBJECT_BUILD_MENU(object_type)(Widget parent, \
	int number_of_objects,struct object_type **objects, \
	struct CHOOSE_OBJECT_STRUCT(object_type) *choose_object) \
/***************************************************************************** \
LAST MODIFIED : 8 September 1999 \
\
DESCRIPTION : \
Creates a PullDownMenu widget for choosing any of the <number_of_objects> \
<objects>. If there are more than MAX_CHOOSER_ROWS objects then this function \
builds menus of cascade buttons with their own submenus. \
============================================================================*/ \
{ \
	Arg args[3]; \
	char *null_object_name="<none>"; \
	char *last_object_name,*object_name; \
	int error,i,max_submenu_items,objects_in_menu,objects_in_submenu, \
		objects_remaining,submenu_levels; \
	struct object_type *last_object,*object,**subobjects; \
	Widget button,menu,submenu; \
	XmString new_string; \
 \
	ENTER(CHOOSE_OBJECT_BUILD_MENU(object_type)); \
	menu=(Widget)NULL; \
	if (parent&&((0==number_of_objects)|| \
		((0<number_of_objects)&&objects))&&choose_object) \
	{ \
		if (number_of_objects<=MAX_CHOOSER_ROWS) \
		{ \
			/* menu of push buttons representing objects */ \
			if (0==number_of_objects) \
			{ \
				objects_in_menu=1; \
				subobjects=(struct object_type **)NULL; \
			} \
			else \
			{ \
				objects_in_menu=number_of_objects; \
				subobjects=objects; \
			} \
			XtSetArg(args[0],XmNbuttonType,XmPUSHBUTTON); \
			if (menu=XmCreatePulldownMenu(parent,"menu",args,1)) \
			{ \
				/* get callbacks from all the push buttons in menu */ \
				XtAddCallback(menu,XmNentryCallback, \
					CHOOSE_OBJECT_MENU_CB(object_type),(XtPointer)choose_object); \
				for (i=0;(i<objects_in_menu)&&menu;i++) \
				{ \
					if (subobjects) \
					{ \
						object = *subobjects; \
					} \
					else \
					{ \
						object = (struct object_type *)NULL; \
					} \
					XtSetArg(args[0],XmNuserData,(XtPointer)object); \
					object_name=null_object_name; \
					if (object) \
					{ \
						GET_NAME(object_type)(object,&object_name); \
					} \
					new_string=XmStringCreateSimple(object_name); \
					XtSetArg(args[1],XmNlabelString,(XtPointer)new_string); \
					if (button=XmCreatePushButtonGadget(menu,object_name,args,2)) \
					{ \
						XtManageChild(button); \
					} \
					else \
					{ \
						display_message(ERROR_MESSAGE, \
							"CHOOSE_OBJECT_BUILD_MENU(" #object_type \
							").  Could not create push button"); \
						XtDestroyWidget(menu); \
					} \
					XmStringFree(new_string); \
					if (object) \
					{ \
						DEALLOCATE(object_name); \
					} \
					subobjects++; \
				} \
			} \
			else \
			{ \
				display_message(ERROR_MESSAGE,"CHOOSE_OBJECT_BUILD_MENU(" #object_type \
					").  Could not create menu"); \
			} \
		} \
		else \
		{ \
			/* menu of cascade buttons with attached submenus */ \
			/* get the largest number of objects we can put in the submenus */ \
			submenu_levels=1; \
			max_submenu_items=MAX_CHOOSER_ROWS; \
			while (max_submenu_items*MAX_CHOOSER_ROWS < number_of_objects) \
			{ \
				max_submenu_items *= MAX_CHOOSER_ROWS; \
				submenu_levels++; \
			} \
			/* get the minimum number of submenus for this menu */ \
			objects_in_menu=2; \
			while (max_submenu_items*objects_in_menu < number_of_objects) \
			{ \
				objects_in_menu++; \
			} \
			/* get average (rounded up) number of objects in submenus */ \
			objects_in_submenu=MAX_CHOOSER_ROWS; \
			while (max_submenu_items*objects_in_menu >= number_of_objects) \
			{ \
				objects_in_submenu--; \
				max_submenu_items=objects_in_submenu; \
				for (i=1;i<submenu_levels;i++) \
				{ \
					max_submenu_items *= objects_in_submenu; \
				} \
			} \
			objects_in_submenu++; \
			XtSetArg(args[0],XmNbuttonType,XmCASCADEBUTTON); \
			if (menu=XmCreatePulldownMenu(parent,"menu",args,1)) \
			{ \
				subobjects=objects; \
				objects_remaining=number_of_objects; \
				for (i=0;(i<objects_in_menu)&&menu;i++) \
				{ \
					if (objects_remaining == ((objects_in_menu-i)*(objects_in_submenu-1))) \
					{ \
						objects_in_submenu--; \
					} \
					object= *subobjects; \
					object_name=(char *)NULL; \
					error=0; \
					GET_NAME(object_type)(object,&object_name); \
					if (10<strlen(object_name)) \
					{ \
						object_name[11]='\0'; \
					} \
					append_string(&object_name," ... ",&error); \
					last_object = *(subobjects+objects_in_submenu-1); \
					GET_NAME(object_type)(last_object,&last_object_name); \
					if (10<strlen(last_object_name)) \
					{ \
						last_object_name[11]='\0'; \
					} \
					append_string(&object_name,last_object_name,&error); \
					new_string=XmStringCreateSimple(object_name); \
					XtSetArg(args[0],XmNlabelString,(XtPointer)new_string); \
					if ((submenu=CHOOSE_OBJECT_BUILD_MENU(object_type)(menu, \
						objects_in_submenu,subobjects,choose_object))&& \
						(button=XmCreateCascadeButtonGadget(menu,object_name,args,1))) \
					{ \
						/* attach cascade button to new submenu */ \
						XtVaSetValues(button, \
							XmNsubMenuId,(XtPointer)submenu,NULL); \
						XtManageChild(button); \
					} \
					else \
					{ \
						display_message(ERROR_MESSAGE, \
							"CHOOSE_OBJECT_BUILD_MENU(" #object_type \
							").  Could not create cascade button or menu"); \
						XtDestroyWidget(menu); \
					} \
					XmStringFree(new_string); \
					DEALLOCATE(object_name); \
					DEALLOCATE(last_object_name); \
					subobjects += objects_in_submenu; \
					objects_remaining -= objects_in_submenu; \
				} \
			} \
			else \
			{ \
				display_message(ERROR_MESSAGE, \
					"CHOOSE_OBJECT_BUILD_MENU(" #object_type ").  Could not create menu"); \
			} \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"CHOOSE_OBJECT_BUILD_MENU(" #object_type ").  Invalid argument(s)"); \
	} \
	LEAVE; \
 \
	return (menu); \
} /* CHOOSE_OBJECT_BUILD_MENU(object_type) */

#if defined (FULL_NAMES)
#define CHOOSE_OBJECT_ADD_TO_LIST_STRUCT_( object_type ) \
	choose_object_add_to_list_struct_ ## object_type
#else
#define CHOOSE_OBJECT_ADD_TO_LIST_STRUCT_( object_type ) coatls ## object_type
#endif
#define CHOOSE_OBJECT_ADD_TO_LIST_STRUCT( object_type ) \
	CHOOSE_OBJECT_ADD_TO_LIST_STRUCT_(object_type)

#define DECLARE_CHOOSE_OBJECT_ADD_TO_LIST_STRUCT_TYPE( object_type ) \
struct CHOOSE_OBJECT_ADD_TO_LIST_STRUCT(object_type) \
/***************************************************************************** \
LAST MODIFIED : 8 September 1999 \
\
DESCRIPTION : \
Data for adding objects to an allocated list. Handles conditional function. \
============================================================================*/ \
{ \
	int number_of_objects; \
	struct CHOOSE_OBJECT_STRUCT(object_type) *choose_object; \
	struct object_type **list_position; \
} /* struct CHOOSE_OBJECT_ADD_TO_LIST_STRUCT(object_type) */

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
LAST MODIFIED : 8 September 1999 \
\
DESCRIPTION : \
Puts the <object> at the array position pointed to by <list_position>. \
============================================================================*/ \
{ \
	int return_code; \
	struct CHOOSE_OBJECT_ADD_TO_LIST_STRUCT(object_type) *add_data; \
 \
	ENTER(CHOOSE_OBJECT_ADD_TO_LIST(object_type)); \
	if (object&&(add_data=(struct CHOOSE_OBJECT_ADD_TO_LIST_STRUCT(object_type) *) \
		add_data_void)&&add_data->choose_object&&add_data->list_position) \
	{ \
		if (!(add_data->choose_object->conditional_function)|| \
			(add_data->choose_object->conditional_function)(object,(void *)NULL)) \
		{ \
			*(add_data->list_position) = object; \
			(add_data->list_position)++; \
			(add_data->number_of_objects++); \
		} \
		return_code=1; \
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
#define CHOOSE_OBJECT_BUILD_MAIN_MENU_( object_type ) \
	choose_object_build_main_menu_ ## object_type
#else
#define CHOOSE_OBJECT_BUILD_MAIN_MENU_( object_type ) cobmm ## object_type
#endif
#define CHOOSE_OBJECT_BUILD_MAIN_MENU( object_type ) \
	CHOOSE_OBJECT_BUILD_MAIN_MENU_(object_type)

#define DECLARE_CHOOSE_OBJECT_BUILD_MAIN_MENU_FUNCTION( object_type ) \
static int CHOOSE_OBJECT_BUILD_MAIN_MENU(object_type)( \
	struct CHOOSE_OBJECT_STRUCT(object_type) *choose_object) \
/***************************************************************************** \
LAST MODIFIED : 8 September 1999 \
\
DESCRIPTION : \
Allocates and fills an array of all the choosable objects, then calls \
CHOOSE_OBJECT_BUILD_MENU to convert them into a hierarchical menu. \
Clears existing menu and detaches it from the main_cascade if required. \
The new menu is attached to the main_cascade button. \
============================================================================*/ \
{ \
	int max_number_of_objects,return_code; \
	struct CHOOSE_OBJECT_ADD_TO_LIST_STRUCT(object_type) add_to_list_data; \
	struct object_type **objects; \
	Widget menu; \
 \
	ENTER(CHOOSE_OBJECT_BUILD_MAIN_MENU(object_type)); \
	return_code=0; \
	if (choose_object&&choose_object->main_cascade) \
	{ \
		max_number_of_objects= \
			NUMBER_IN_MANAGER(object_type)(choose_object->object_manager); \
		objects=(struct object_type **)NULL; \
		if ((0==max_number_of_objects)|| \
			ALLOCATE(objects,struct object_type *,max_number_of_objects)) \
		{ \
			add_to_list_data.choose_object=choose_object; \
			add_to_list_data.list_position=objects; \
			add_to_list_data.number_of_objects=0; \
			if (FOR_EACH_OBJECT_IN_MANAGER(object_type)( \
				CHOOSE_OBJECT_ADD_TO_LIST(object_type),(void *)&add_to_list_data, \
				choose_object->object_manager)) \
			{ \
				if (menu=CHOOSE_OBJECT_BUILD_MENU(object_type)( \
					choose_object->widget,add_to_list_data.number_of_objects,objects, \
					choose_object)) \
				{ \
					/* attach cascade button to new menu */ \
					XtVaSetValues(choose_object->main_cascade, \
						XmNsubMenuId,(XtPointer)menu,NULL); \
					/* clear existing menu */ \
					if (choose_object->main_menu) \
					{ \
						XtDestroyWidget(choose_object->main_menu); \
					} \
					choose_object->main_menu=menu; \
					return_code=1; \
				} \
			} \
			DEALLOCATE(objects); \
		} \
		if (!return_code) \
		{ \
			display_message(ERROR_MESSAGE, \
				"CHOOSE_OBJECT_BUILD_MAIN_MENU(" #object_type ").  Failed"); \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"CHOOSE_OBJECT_BUILD_MAIN_MENU(" #object_type ").  Invalid argument(s)"); \
	} \
	LEAVE; \
 \
	return (return_code); \
} /* CHOOSE_OBJECT_BUILD_MAIN_MENU(object_type) */

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
LAST MODIFIED : 8 September 1999 \
\
DESCRIPTION : \
Rebuilds the choose object menu in response to manager messages. \
Tries to minimise menu rebuilds as much as possible, since these cause \
annoying flickering on the screen. \
============================================================================*/ \
{ \
	int update_menu; \
	struct CHOOSE_OBJECT_STRUCT(object_type) *choose_object; \
\
	ENTER(CHOOSE_OBJECT_GLOBAL_OBJECT_CHANGE(object_type)); \
	/* checking arguments */ \
	if (message&&(choose_object= \
		(struct CHOOSE_OBJECT_STRUCT(object_type) *)data)) \
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
			CHOOSE_OBJECT_BUILD_MAIN_MENU(object_type)(choose_object); \
			/* if current_object not in menu CHOOSE_OBJECT_SELECT_MENU_ITEM */ \
			/* will select the first item in it, and call an update */ \
			CHOOSE_OBJECT_SELECT_MENU_ITEM(object_type)(choose_object); \
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
LAST MODIFIED : 8 September 1999 \
\
DESCRIPTION : \
Creates an option menu from which an object from the manager may be chosen. \
The optional conditional function permits a subset of objects in the manager \
to be selectable. \
============================================================================*/ \
{ \
	Arg args[7]; \
	int num_children; \
	struct CHOOSE_OBJECT_STRUCT(object_type) *choose_object; \
	Widget *child_list,return_widget; \
	XmFontList fontlist; \
\
	ENTER(CREATE_CHOOSE_OBJECT_WIDGET(object_type)); \
	return_widget=(Widget)NULL; \
	if (object_manager&&parent) \
	{ \
		if (ALLOCATE(choose_object, \
			struct CHOOSE_OBJECT_STRUCT(object_type),1)) \
		{ \
			/* initialise the structure */ \
			choose_object->manager_callback_id=(void *)NULL; \
			choose_object->widget_parent=parent; \
			choose_object->main_cascade=(Widget)NULL; \
			choose_object->main_menu=(Widget)NULL; \
			choose_object->update_callback.procedure= \
				(Callback_procedure *)NULL; \
			choose_object->update_callback.data=(void *)NULL; \
			choose_object->current_object=(struct object_type *)NULL; \
			choose_object->last_updated_object=(struct object_type *)NULL; \
			choose_object->object_manager=object_manager; \
			choose_object->conditional_function=conditional_function; \
			XtSetArg(args[0],XmNleftAttachment,XmATTACH_FORM); \
			XtSetArg(args[1],XmNrightAttachment,XmATTACH_FORM); \
			XtSetArg(args[2],XmNtopAttachment,XmATTACH_FORM); \
			XtSetArg(args[3],XmNbottomAttachment,XmATTACH_FORM); \
			XtSetArg(args[4],XmNuserData,(XtPointer)choose_object); \
			XtSetArg(args[5],XmNmarginHeight,0); \
			XtSetArg(args[6],XmNmarginWidth,0); \
			if (choose_object->widget=XmCreateMenuBar(parent,"chooser",args,7)) \
			{ \
				/* add destroy callback for top widget */ \
				XtAddCallback(choose_object->widget,XmNdestroyCallback, \
					CHOOSE_OBJECT_DESTROY_CB(object_type),(XtPointer)choose_object); \
				if (choose_object->main_cascade= \
					XmCreateCascadeButtonGadget(choose_object->widget,"cascade",NULL,0)) \
				{ \
					if (CHOOSE_OBJECT_BUILD_MAIN_MENU(object_type)(choose_object)) \
					{ \
						/* dodgy: steal font from child cascade buttons for main_cascade */ \
						XtVaGetValues(choose_object->main_menu, \
							XmNnumChildren,&num_children,XmNchildren,&child_list,NULL); \
						if ((0<num_children)&&child_list) \
						{ \
							XtVaGetValues(child_list[0], \
								XmNfontList,(XtPointer)&fontlist,NULL); \
							XtVaSetValues(choose_object->main_cascade, \
								XmNfontList,(XtPointer)fontlist,NULL); \
						} \
						XtManageChild(choose_object->main_cascade); \
						XtManageChild(choose_object->widget); \
						/* ensure first object in manager chosen */ \
						CHOOSE_OBJECT_SELECT_MENU_ITEM(object_type)(choose_object); \
						/* register for any object changes */ \
						choose_object->manager_callback_id= \
							MANAGER_REGISTER(object_type)( \
							CHOOSE_OBJECT_GLOBAL_OBJECT_CHANGE(object_type), \
							(void *)choose_object,choose_object->object_manager); \
						CHOOSE_OBJECT_SET_OBJECT(object_type)( \
							choose_object->widget,current_object); \
						return_widget=choose_object->widget; \
					} \
					else \
					{ \
						display_message(ERROR_MESSAGE,"CREATE_CHOOSE_OBJECT_WIDGET(" \
							#object_type ").  Could not build menu"); \
						DEALLOCATE(choose_object); \
					} \
				} \
				else \
				{ \
					display_message(ERROR_MESSAGE, \
						"CREATE_CHOOSE_OBJECT_WIDGET(" #object_type \
						").  Could not make main cascade button"); \
					DEALLOCATE(choose_object); \
				} \
			} \
			else \
			{ \
				display_message(ERROR_MESSAGE, \
					"CREATE_CHOOSE_OBJECT_WIDGET(" #object_type \
					").  Could not make main row column"); \
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
			"CREATE_CHOOSE_OBJECT_WIDGET(" #object_type ").  Invalid argument(s)"); \
	} \
	LEAVE; \
\
	return (return_widget); \
} /* CREATE_CHOOSE_OBJECT_WIDGET(object_type) */

#define DECLARE_CHOOSE_OBJECT_SET_CALLBACK_FUNCTION( object_type ) \
PROTOTYPE_CHOOSE_OBJECT_SET_CALLBACK_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 23 July 1997 \
\
DESCRIPTION : \
Changes the callback item of the choose_object_widget. \
============================================================================*/ \
{ \
	int return_code; \
	struct CHOOSE_OBJECT_STRUCT(object_type) *choose_object; \
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

#define DECLARE_CHOOSE_OBJECT_SET_OBJECT_FUNCTION( object_type ) \
PROTOTYPE_CHOOSE_OBJECT_SET_OBJECT_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 10 September 1998 \
\
DESCRIPTION : \
Changes the chosen object in the choose_object_widget. \
============================================================================*/ \
{ \
	int return_code; \
	struct CHOOSE_OBJECT_STRUCT(object_type) *choose_object; \
\
	ENTER(CHOOSE_OBJECT_SET_OBJECT(object_type)); \
	if (choose_object_widget) \
	{ \
		/* Get the pointer to the data for the choose_object dialog */ \
		XtVaGetValues(choose_object_widget,XmNuserData,&choose_object,NULL); \
		if (choose_object) \
		{ \
			/* set last_updated_object to avoid update once set */ \
			choose_object->last_updated_object=new_object; \
			choose_object->current_object=new_object; \
			CHOOSE_OBJECT_SELECT_MENU_ITEM(object_type)(choose_object); \
			return_code=1; \
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

#define DECLARE_CHOOSE_OBJECT_GET_CALLBACK_FUNCTION( object_type ) \
PROTOTYPE_CHOOSE_OBJECT_GET_CALLBACK_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 23 July 1997 \
\
DESCRIPTION : \
Returns a pointer to the callback item of the choose_object_widget. \
============================================================================*/ \
{ \
	struct Callback_data *return_address; \
	struct CHOOSE_OBJECT_STRUCT(object_type) *choose_object; \
\
	ENTER(CHOOSE_OBJECT_GET_CALLBACK(object_type)); \
	if (choose_object_widget) \
	{ \
		/* Get the pointer to the data for the choose_object dialog */ \
		XtVaGetValues(choose_object_widget,XmNuserData,&choose_object,NULL); \
		if (choose_object) \
		{ \
			return_address=&(choose_object->update_callback); \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"CHOOSE_OBJECT_GET_CALLBACK(" #object_type \
				").  Missing widget data"); \
			return_address=(struct Callback_data *)NULL; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"CHOOSE_OBJECT_GET_CALLBACK(" #object_type ").  Missing widget"); \
		return_address=(struct Callback_data *)NULL; \
	} \
	LEAVE; \
\
	return (return_address); \
} /* CHOOSE_OBJECT_GET_CALLBACK(object_type) */

#define DECLARE_CHOOSE_OBJECT_GET_OBJECT_FUNCTION( object_type ) \
PROTOTYPE_CHOOSE_OBJECT_GET_OBJECT_FUNCTION( object_type ) \
/***************************************************************************** \
LAST MODIFIED : 23 July 1997 \
\
DESCRIPTION : \
Returns the currently chosen object in the choose_object_widget. \
============================================================================*/ \
{ \
	struct object_type *return_address; \
	struct CHOOSE_OBJECT_STRUCT(object_type) *choose_object; \
\
	ENTER(CHOOSE_OBJECT_GET_OBJECT(object_type)); \
	if (choose_object_widget) \
	{ \
		/* Get the pointer to the data for the choose_object dialog */ \
		XtVaGetValues(choose_object_widget,XmNuserData,&choose_object,NULL); \
		if (choose_object) \
		{ \
			return_address=choose_object->current_object; \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"CHOOSE_OBJECT_GET_OBJECT(" #object_type ").  Missing widget data"); \
			return_address=(struct object_type *)NULL; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"CHOOSE_OBJECT_GET_OBJECT(" #object_type ").  Missing widget"); \
		return_address=(struct object_type *)NULL; \
	} \
	LEAVE; \
\
	return (return_address); \
} /* CHOOSE_OBJECT_GET_OBJECT(object_type) */

#define DECLARE_CHOOSE_OBJECT_MODULE_FUNCTIONS( object_type ) \
DECLARE_CHOOSE_OBJECT_UPDATE_FUNCTION(object_type) \
DECLARE_CHOOSE_OBJECT_SELECT_MENU_ITEM_FUNCTION(object_type) \
DECLARE_CHOOSE_OBJECT_DESTROY_CB_FUNCTION(object_type) \
DECLARE_CHOOSE_OBJECT_MENU_CB_FUNCTION(object_type) \
DECLARE_CHOOSE_OBJECT_BUILD_MENU_FUNCTION(object_type) \
DECLARE_CHOOSE_OBJECT_ADD_TO_LIST_STRUCT_TYPE(object_type); \
DECLARE_CHOOSE_OBJECT_ADD_TO_LIST_FUNCTION(object_type) \
DECLARE_CHOOSE_OBJECT_BUILD_MAIN_MENU_FUNCTION(object_type) \
DECLARE_CHOOSE_OBJECT_GLOBAL_OBJECT_CHANGE_FUNCTION(object_type)

#define DECLARE_CHOOSE_OBJECT_GLOBAL_FUNCTIONS( object_type ) \
DECLARE_CREATE_CHOOSE_OBJECT_WIDGET_FUNCTION(object_type) \
DECLARE_CHOOSE_OBJECT_SET_CALLBACK_FUNCTION(object_type) \
DECLARE_CHOOSE_OBJECT_SET_OBJECT_FUNCTION(object_type) \
DECLARE_CHOOSE_OBJECT_GET_CALLBACK_FUNCTION(object_type) \
DECLARE_CHOOSE_OBJECT_GET_OBJECT_FUNCTION(object_type)

#endif /* !defined (CHOOSE_OBJECT_PRIVATE_H) */
