/*******************************************************************************
FILE : choose_object_list_private.h

LAST MODIFIED : 28 June 1999

DESCRIPTION :
???RC Version of choose_object using lists instead of managers.
Macros for implementing an option menu dialog control for choosing an object
from a list (subject to an optional conditional function).
Calls the client-specified callback routine if a different object is chosen.
==============================================================================*/
#if !defined (CHOOSE_OBJECT_LIST_PRIVATE_H)
#define CHOOSE_OBJECT_LIST_PRIVATE_H

#include <stdio.h>
#if defined (MOTIF)
#include <Xm/Xm.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#endif /* defined (MOTIF) */
#include "general/debug.h"
#include "choose/choose_object_list.h"
#include "choose/choose_object.uidh"
#include "user_interface/gui_dialog_macros.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module variables
----------------
*/
#if defined (FULL_NAMES)
#define CHOOSE_OBJECT_LIST_STRUCT_( object_type ) \
	choose_object_list_struct_ ## object_type
#else
#define CHOOSE_OBJECT_LIST_STRUCT_( object_type ) colstru ## object_type
#endif
#define CHOOSE_OBJECT_LIST_STRUCT( object_type ) \
	CHOOSE_OBJECT_LIST_STRUCT_(object_type)

#define FULL_DECLARE_CHOOSE_OBJECT_LIST_STRUCT_TYPE( object_type ) \
struct CHOOSE_OBJECT_LIST_STRUCT(object_type) \
/***************************************************************************** \
LAST MODIFIED : 10 September 1998 \
\
DESCRIPTION : \
Contains information required by the choose_object_list control dialog. \
============================================================================*/ \
{ \
	struct object_type *current_object,*last_updated_object; \
	struct LIST(object_type) *list_of_objects; \
	LIST_CONDITIONAL_FUNCTION(object_type) *conditional_function; \
	Widget widget_parent,widget,option,menu; \
	struct Callback_data update_callback; \
} /* struct CHOOSE_OBJECT_LIST_STRUCT(object_type) */

#if defined (MOTIF)
static int choose_object_list_hierarchy_open=0;
static MrmHierarchy choose_object_list_hierarchy;
#endif /* defined (MOTIF) */

/*
Module functions
----------------
*/
#if defined (FULL_NAMES)
#define CHOOSE_OBJECT_LIST_DIALOG_NAME_( object_type ) \
	choose_object_list_ ## object_type
#else
#define CHOOSE_OBJECT_LIST_DIALOG_NAME_( object_type ) col_ ## object_type
#endif
#define CHOOSE_OBJECT_LIST_DIALOG_NAME( object_type ) \
	CHOOSE_OBJECT_LIST_DIALOG_NAME_( object_type )

#define DECLARE_CHOOSE_OBJECT_LIST_DIALOG_IDENTIFY_FUNCTIONS( object_type ) \
DECLARE_DIALOG_IDENTIFY_FUNCTION(CHOOSE_OBJECT_LIST_DIALOG_NAME(object_type), \
	CHOOSE_OBJECT_LIST_STRUCT(object_type),option) \
DECLARE_DIALOG_IDENTIFY_FUNCTION(CHOOSE_OBJECT_LIST_DIALOG_NAME(object_type), \
	CHOOSE_OBJECT_LIST_STRUCT(object_type),menu) \

#if defined (FULL_NAMES)
#define CHOOSE_OBJECT_LIST_UPDATE_( object_type ) \
	choose_object_list_update_ ## object_type
#else
#define CHOOSE_OBJECT_LIST_UPDATE_( object_type ) colu ## object_type
#endif
#define CHOOSE_OBJECT_LIST_UPDATE( object_type ) \
	CHOOSE_OBJECT_LIST_UPDATE_(object_type)

#define DECLARE_CHOOSE_OBJECT_LIST_UPDATE_FUNCTION( object_type ) \
static int CHOOSE_OBJECT_LIST_UPDATE(object_type)( \
	struct CHOOSE_OBJECT_LIST_STRUCT(object_type) *temp_choose_object_list) \
/***************************************************************************** \
LAST MODIFIED : 10 September 1998 \
\
DESCRIPTION : \
Tells CMGUI about the current values. Sends a pointer to the current object. \
============================================================================*/ \
{ \
	int return_code; \
\
	ENTER(CHOOSE_OBJECT_LIST_UPDATE(object_type)); \
	if (temp_choose_object_list) \
	{ \
		if (temp_choose_object_list->current_object != \
			temp_choose_object_list->last_updated_object) \
		{ \
			if (temp_choose_object_list->update_callback.procedure) \
			{ \
				/* now call the procedure with the user data */ \
				(temp_choose_object_list->update_callback.procedure)( \
					temp_choose_object_list->widget, \
					temp_choose_object_list->update_callback.data, \
					temp_choose_object_list->current_object); \
			} \
			temp_choose_object_list->last_updated_object= \
				temp_choose_object_list->current_object; \
		} \
		return_code=1; \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"CHOOSE_OBJECT_LIST_UPDATE(" #object_type ").  Invalid argument(s)"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* CHOOSE_OBJECT_LIST_UPDATE(object_type) */

#if defined (FULL_NAMES)
#define CHOOSE_OBJECT_LIST_DESTROY_CB_( object_type ) \
	choose_object_list_destroy_cb_ ## object_type
#else
#define CHOOSE_OBJECT_LIST_DESTROY_CB_( object_type ) coldc ## object_type
#endif
#define CHOOSE_OBJECT_LIST_DESTROY_CB( object_type ) \
	CHOOSE_OBJECT_LIST_DESTROY_CB_(object_type)

#define DECLARE_CHOOSE_OBJECT_LIST_DESTROY_CB_FUNCTION( object_type ) \
static void CHOOSE_OBJECT_LIST_DESTROY_CB(object_type)(Widget widget,int *tag, \
	unsigned long *reason) \
/***************************************************************************** \
LAST MODIFIED : 28 June 1999 \
\
DESCRIPTION : \
Callback for the choose_object_list dialog - tidies up all memory allocation. \
============================================================================*/ \
{ \
	struct CHOOSE_OBJECT_LIST_STRUCT(object_type) *temp_choose_object_list; \
\
	ENTER(CHOOSE_OBJECT_LIST_DESTROY_CB(object_type)); \
	USE_PARAMETER(tag); \
	USE_PARAMETER(reason); \
	if (widget) \
	{ \
		/* Get the pointer to the data for the choose_object_list dialog */ \
		XtVaGetValues(widget,XmNuserData,&temp_choose_object_list,NULL); \
		if (temp_choose_object_list) \
		{ \
			DEALLOCATE(temp_choose_object_list); \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"CHOOSE_OBJECT_LIST_DESTROY_CB(" #object_type \
				").  Missing widget data"); \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"CHOOSE_OBJECT_LIST_DESTROY_CB(" #object_type ").  Missing widget"); \
	} \
	LEAVE; \
} /* CHOOSE_OBJECT_LIST_DESTROY_CB(object_type) */

#if defined (FULL_NAMES)
#define CHOOSE_OBJECT_LIST_ADD_MENU_ITEM_( object_type ) \
	choose_object_list_add_menu_item_ ## object_type
#else
#define CHOOSE_OBJECT_LIST_ADD_MENU_ITEM_( object_type ) colami ## object_type
#endif
#define CHOOSE_OBJECT_LIST_ADD_MENU_ITEM( object_type ) \
	CHOOSE_OBJECT_LIST_ADD_MENU_ITEM_(object_type)

#define DECLARE_CHOOSE_OBJECT_LIST_ADD_MENU_ITEM_FUNCTION( object_type ) \
static int CHOOSE_OBJECT_LIST_ADD_MENU_ITEM(object_type)( \
	struct object_type *object,void *temp_choose_object_list_void) \
/***************************************************************************** \
LAST MODIFIED : 10 September 1998 \
\
DESCRIPTION : \
Iterator function to add a PushButton for <object> to the parent PullDownMenu. \
============================================================================*/ \
{ \
	int return_code; \
	Widget temp_widget; \
	Arg args[2]; \
	XmString new_string; \
	char *object_name; \
	struct CHOOSE_OBJECT_LIST_STRUCT(object_type) *temp_choose_object_list; \
\
	ENTER(CHOOSE_OBJECT_LIST_ADD_MENU_ITEM(object_type)); \
	if (object&&(temp_choose_object_list= \
		(struct CHOOSE_OBJECT_LIST_STRUCT(object_type) *) \
		temp_choose_object_list_void) && temp_choose_object_list->menu) \
	{ \
		return_code=1; \
		/* only add object if it complies with the conditional function */ \
		if ((!temp_choose_object_list->conditional_function) || \
			(temp_choose_object_list->conditional_function)(object,(void *)NULL)) \
		{ \
			XtSetArg(args[0],XmNuserData,(XtPointer)object); \
			if (GET_NAME(object_type)(object,&object_name)) \
			{ \
				new_string=XmStringCreateSimple(object_name); \
				XtSetArg(args[1],XmNlabelString,(XtPointer)new_string); \
				if (temp_widget=XmCreatePushButtonGadget(temp_choose_object_list->menu,\
					object_name,args,2)) \
				{ \
					XtManageChild(temp_widget); \
				} \
				else \
				{ \
					display_message(ERROR_MESSAGE, \
						"CHOOSE_OBJECT_LIST_ADD_MENU_ITEM(" #object_type \
						").  Could not create menu item"); \
					return_code=0; \
				} \
				XmStringFree(new_string); \
				DEALLOCATE(object_name); \
			} \
			else \
			{ \
				display_message(ERROR_MESSAGE, \
					"CHOOSE_OBJECT_LIST_ADD_MENU_ITEM(" #object_type \
					").  Could not get object name"); \
				return_code=0; \
			} \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"CHOOSE_OBJECT_LIST_ADD_MENU_ITEM(" #object_type \
			").  Invalid arguments"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* CHOOSE_OBJECT_LIST_ADD_MENU_ITEM(object_type) */

#if defined (FULL_NAMES)
#define CHOOSE_OBJECT_LIST_MAKE_MENU_( object_type ) \
	choose_object_list_make_menu_ ## object_type
#else
#define CHOOSE_OBJECT_LIST_MAKE_MENU_( object_type ) colmm ## object_type
#endif
#define CHOOSE_OBJECT_LIST_MAKE_MENU( object_type ) \
	CHOOSE_OBJECT_LIST_MAKE_MENU_(object_type)

#define DECLARE_CHOOSE_OBJECT_LIST_MAKE_MENU_FUNCTION( object_type ) \
static int CHOOSE_OBJECT_LIST_MAKE_MENU(object_type)( \
	struct CHOOSE_OBJECT_LIST_STRUCT(object_type) *temp_choose_object_list) \
/***************************************************************************** \
LAST MODIFIED : 31 July 1997 \
\
DESCRIPTION : \
Destroys the menu widget and all items in it, refills the menu with all \
objects in the list. \
============================================================================*/ \
{ \
	char *null_object_name="<none>"; \
	int return_code,i,num_children; \
	Widget *child_list,*child_copy,temp_widget; \
	Arg args[2]; \
	XmString new_string; \
\
	ENTER(CHOOSE_OBJECT_LIST_MAKE_MENU(object_type)); \
	/* check arguments */ \
	if (temp_choose_object_list&&temp_choose_object_list->list_of_objects) \
	{ \
		/* unmanage and destroy current menu items */ \
		XtVaGetValues(temp_choose_object_list->menu,XmNnumChildren,&num_children, \
			XmNchildren,&child_list,NULL); \
		if (0<num_children) \
		{ \
			if (ALLOCATE(child_copy,Widget,num_children)) \
			{ \
				for (i=0;i<num_children;i++) \
				{ \
					child_copy[i]=child_list[i]; \
				} \
				for (i=0;i<num_children;i++) \
				{ \
					XtUnmanageChild(child_copy[i]); \
					XtDestroyWidget(child_copy[i]); \
				} \
				DEALLOCATE(child_copy); \
				return_code=1; \
			} \
			else \
			{ \
				display_message(ERROR_MESSAGE, \
					"CHOOSE_OBJECT_LIST_MAKE_MENU(" #object_type \
					").  Could not allocate space for copy of menu items"); \
				return_code=0; \
			} \
		} \
		else \
		{ \
			return_code=1; \
		} \
		if (return_code) \
		{ \
			if (0<NUMBER_IN_LIST(object_type)( \
				temp_choose_object_list->list_of_objects)) \
			{ \
				/* insert menu items for each object */ \
				FOR_EACH_OBJECT_IN_LIST(object_type)( \
					CHOOSE_OBJECT_LIST_ADD_MENU_ITEM(object_type), \
					(void *)temp_choose_object_list, \
					temp_choose_object_list->list_of_objects);\
			} \
			else \
			{ \
				/* add a NULL child to list */ \
				XtSetArg(args[0],XmNuserData,(XtPointer)NULL); \
				new_string=XmStringCreateSimple(null_object_name); \
				XtSetArg(args[1],XmNlabelString,(XtPointer)new_string); \
				if (temp_widget=XmCreatePushButtonGadget( \
					temp_choose_object_list->menu,null_object_name,args,2)) \
				{ \
					XtManageChild(temp_widget); \
					return_code=1; \
				} \
				else \
				{ \
					display_message(ERROR_MESSAGE, \
						"CHOOSE_OBJECT_LIST_MAKE_MENU(" #object_type \
						").  Could not create menu item"); \
					return_code=0; \
				} \
				XmStringFree(new_string); \
			} \
			/* only manage menu if it has children */ \
			XtVaGetValues(temp_choose_object_list->menu,XmNnumChildren, \
				&num_children,NULL); \
			if (0<num_children) \
			{ \
				XtManageChild(temp_choose_object_list->option); \
			} \
			else \
			{ \
				XtUnmanageChild(temp_choose_object_list->option); \
			} \
		} \
		else \
		{ \
			XtUnmanageChild(temp_choose_object_list->option); \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"CHOOSE_OBJECT_LIST_MAKE_MENU(" #object_type ").  Invalid argument(s)"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* CHOOSE_OBJECT_LIST_MAKE_MENU(object_type) */

#if defined (FULL_NAMES)
#define CHOOSE_OBJECT_LIST_SELECT_MENU_ITEM_( object_type ) \
	choose_object_list_select_menu_item_ ## object_type
#else
#define CHOOSE_OBJECT_LIST_SELECT_MENU_ITEM_( object_type ) \
	colsmi ## object_type
#endif
#define CHOOSE_OBJECT_LIST_SELECT_MENU_ITEM( object_type ) \
	CHOOSE_OBJECT_LIST_SELECT_MENU_ITEM_(object_type)

#define DECLARE_CHOOSE_OBJECT_LIST_SELECT_MENU_ITEM_FUNCTION( object_type ) \
static int CHOOSE_OBJECT_LIST_SELECT_MENU_ITEM(object_type)( \
	struct CHOOSE_OBJECT_LIST_STRUCT(object_type) *temp_choose_object_list) \
/***************************************************************************** \
LAST MODIFIED : 28 June 1999 \
\
DESCRIPTION : \
Finds the current_object in the menu and selects it. If the current_object \
is NULL or not in the menu, the first item in the list is selected. If the \
menu is empty, the current_object is NULLed. \
The client is informed if the current object changes inside this routine. \
============================================================================*/ \
{ \
	int return_code,i,num_children,selected; \
	Widget *child_list; \
	struct object_type *temp_object; \
\
	ENTER(CHOOSE_OBJECT_LIST_SELECT_MENU_ITEM(object_type)); \
	/* check arguments */ \
	if (temp_choose_object_list&&temp_choose_object_list->list_of_objects) \
	{ \
		XtVaGetValues(temp_choose_object_list->menu,XmNnumChildren, \
			&num_children,XmNchildren,&child_list,NULL); \
		if (0<num_children) \
		{ \
			/* if not a valid object, try to get one now */ \
			if ((!temp_choose_object_list->current_object)|| \
				(!(IS_OBJECT_IN_LIST(object_type)( \
				temp_choose_object_list->current_object, \
				temp_choose_object_list->list_of_objects)))|| \
				(temp_choose_object_list->conditional_function&& \
					(!(temp_choose_object_list->conditional_function)( \
						temp_choose_object_list->current_object,(void *)NULL)))) \
			{ \
				temp_choose_object_list->current_object= \
					FIRST_OBJECT_IN_LIST_THAT(object_type)( \
					temp_choose_object_list->conditional_function, \
					(void *)NULL,temp_choose_object_list->list_of_objects); \
			} \
			selected=-1; \
			for (i=0;i<num_children;i++) \
			{ \
				XtVaGetValues(child_list[i],XmNuserData,&temp_object,NULL); \
				if (temp_object==temp_choose_object_list->current_object) \
				{ \
					selected=i; \
				} \
			} \
			if (0>selected) \
			{ \
				/* make current settings match those of first item in list */ \
				XtVaGetValues(child_list[0],XmNuserData, \
					&(temp_choose_object_list->current_object),NULL); \
				selected=0; \
			} \
			XtVaSetValues(temp_choose_object_list->option,XmNmenuHistory, \
				child_list[selected],NULL); \
			return_code=1; \
		} \
		else \
		{ \
			temp_choose_object_list->current_object=(struct object_type *)NULL; \
			return_code=0; \
		} \
		/* inform the client of the change */ \
		CHOOSE_OBJECT_LIST_UPDATE(object_type)(temp_choose_object_list); \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"CHOOSE_OBJECT_LIST_SELECT_MENU_ITEM(" #object_type \
			").  Invalid argument(s)"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* CHOOSE_OBJECT_LIST_SELECT_MENU_ITEM(object_type) */

#if defined (FULL_NAMES)
#define CHOOSE_OBJECT_LIST_MENU_CB_( object_type ) \
	choose_object_list_menu_cb_ ## object_type
#else
#define CHOOSE_OBJECT_LIST_MENU_CB_( object_type ) colmc ## object_type
#endif
#define CHOOSE_OBJECT_LIST_MENU_CB( object_type ) \
	CHOOSE_OBJECT_LIST_MENU_CB_(object_type)

#define DECLARE_CHOOSE_OBJECT_LIST_MENU_CB_FUNCTION( object_type ) \
static void CHOOSE_OBJECT_LIST_MENU_CB(object_type)(Widget widget, \
	XtPointer client_data,XtPointer call_data) \
/***************************************************************************** \
LAST MODIFIED : 28 June 1999 \
\
DESCRIPTION : \
Callback for the option menu - change of object. \
============================================================================*/ \
{ \
	struct CHOOSE_OBJECT_LIST_STRUCT(object_type) *temp_choose_object_list; \
	Widget menu_item_widget; \
\
	ENTER(CHOOSE_OBJECT_LIST_MENU_CB(object_type)); \
	if (widget&&(temp_choose_object_list= \
		(struct CHOOSE_OBJECT_LIST_STRUCT(object_type) *)client_data)) \
	{ \
		/* get the widget from the call data */ \
		if (menu_item_widget=((XmRowColumnCallbackStruct *)call_data)->widget) \
		{ \
			/* Get the object this menu item represents and make it current */ \
			XtVaGetValues(menu_item_widget,XmNuserData, \
				&(temp_choose_object_list->current_object),NULL); \
			/* inform the client of the change */ \
			/* Always want an update if menu clicked on, so change history: */ \
			temp_choose_object_list->last_updated_object=(struct object_type *)NULL; \
			CHOOSE_OBJECT_LIST_UPDATE(object_type)(temp_choose_object_list); \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"CHOOSE_OBJECT_LIST_MENU_CB(" #object_type \
				").  Could not find the activated menu item"); \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"CHOOSE_OBJECT_LIST_MENU_CB(" #object_type ").  Invalid argument(s)"); \
	} \
	LEAVE; \
} /* CHOOSE_OBJECT_LIST_MENU_CB(object_type) */

/*
Global functions
----------------
*/
#define DECLARE_CREATE_CHOOSE_OBJECT_LIST_WIDGET_FUNCTION( object_type ) \
PROTOTYPE_CREATE_CHOOSE_OBJECT_LIST_WIDGET_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 10 September 1998 \
\
DESCRIPTION : \
Creates an option menu from which an object from the list may be chosen. \
============================================================================*/ \
{ \
	MrmType choose_object_list_dialog_class; \
	struct CHOOSE_OBJECT_LIST_STRUCT(object_type) *temp_choose_object_list; \
	static MrmRegisterArg callback_list[]= \
	{ \
		{"choose_object_identify_option",(XtPointer) \
			DIALOG_IDENTIFY(CHOOSE_OBJECT_LIST_DIALOG_NAME(object_type),option)}, \
		{"choose_object_identify_menu",(XtPointer) \
			DIALOG_IDENTIFY(CHOOSE_OBJECT_LIST_DIALOG_NAME(object_type),menu)}, \
		{"choose_object_destroy_CB",(XtPointer) \
			CHOOSE_OBJECT_LIST_DESTROY_CB(object_type)}, \
		{"choose_object_menu_CB",(XtPointer) \
			CHOOSE_OBJECT_LIST_MENU_CB(object_type)} \
	}; \
	static MrmRegisterArg identifier_list[]= \
	{ \
		{"Choose_object_structure",(XtPointer)NULL} \
	}; \
	Widget return_widget; \
\
	ENTER(CREATE_CHOOSE_OBJECT_LIST_WIDGET(object_type)); \
	return_widget=(Widget)NULL; \
	/* check arguments */ \
	if (list_of_objects&&parent) \
	{ \
		if (MrmOpenHierarchy_base64_string(choose_object_uidh, \
			&choose_object_list_hierarchy,&choose_object_list_hierarchy_open)) \
		{ \
			/* allocate memory */ \
			if (ALLOCATE(temp_choose_object_list, \
				struct CHOOSE_OBJECT_LIST_STRUCT(object_type),1)) \
			{ \
				/* initialise the structure */ \
				temp_choose_object_list->widget_parent=parent; \
				temp_choose_object_list->widget=(Widget)NULL; \
				temp_choose_object_list->option=(Widget)NULL; \
				temp_choose_object_list->menu=(Widget)NULL; \
				temp_choose_object_list->update_callback.procedure= \
					(Callback_procedure *)NULL; \
				temp_choose_object_list->update_callback.data=(void *)NULL; \
				temp_choose_object_list->current_object=(struct object_type *)NULL; \
				temp_choose_object_list->last_updated_object= \
					(struct object_type *)NULL; \
				temp_choose_object_list->list_of_objects=list_of_objects; \
				temp_choose_object_list->conditional_function=conditional_function; \
				/* register the callbacks */ \
				if (MrmSUCCESS==MrmRegisterNamesInHierarchy( \
					choose_object_list_hierarchy, \
					callback_list,XtNumber(callback_list))) \
				{ \
					/* assign and register the identifiers */ \
					identifier_list[0].value=(XtPointer)temp_choose_object_list; \
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy( \
						choose_object_list_hierarchy,identifier_list, \
						XtNumber(identifier_list))) \
					{ \
						/* fetch choose_object_list control widget */ \
						if (MrmSUCCESS==MrmFetchWidget(choose_object_list_hierarchy, \
							"choose_object_option", \
							temp_choose_object_list->widget_parent, \
							&(temp_choose_object_list->widget), \
							&choose_object_list_dialog_class)) \
						{ \
							CHOOSE_OBJECT_LIST_MAKE_MENU(object_type)( \
								temp_choose_object_list); \
							/* ensure first object in list chosen */ \
							CHOOSE_OBJECT_LIST_SELECT_MENU_ITEM(object_type)( \
								temp_choose_object_list); \
							CHOOSE_OBJECT_LIST_SET_OBJECT(object_type)( \
								temp_choose_object_list->widget,current_object); \
							return_widget=temp_choose_object_list->widget; \
						} \
						else \
						{ \
							display_message(ERROR_MESSAGE, \
								"CREATE_CHOOSE_OBJECT_LIST_WIDGET(" #object_type \
								").  Could not fetch choose_object_list dialog"); \
							DEALLOCATE(temp_choose_object_list); \
						} \
					} \
					else \
					{ \
						display_message(ERROR_MESSAGE, \
							"CREATE_CHOOSE_OBJECT_LIST_WIDGET(" #object_type \
							").  Could not register identifiers"); \
						DEALLOCATE(temp_choose_object_list); \
					} \
				} \
				else \
				{ \
					display_message(ERROR_MESSAGE, \
						"CREATE_CHOOSE_OBJECT_LIST_WIDGET(" #object_type \
						").  Could not register callbacks"); \
					DEALLOCATE(temp_choose_object_list); \
				} \
			} \
			else \
			{ \
				display_message(ERROR_MESSAGE, \
					"CREATE_CHOOSE_OBJECT_LIST_WIDGET(" #object_type \
					").  Could not allocate control window structure"); \
			} \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"CREATE_CHOOSE_OBJECT_LIST_WIDGET(" #object_type \
				").  Could not open hierarchy"); \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"CREATE_CHOOSE_OBJECT_LIST_WIDGET(" #object_type \
			").  Invalid argument(s)"); \
	} \
	LEAVE; \
\
	return (return_widget); \
} /* CREATE_CHOOSE_OBJECT_LIST_WIDGET(object_type) */

#define DECLARE_CHOOSE_OBJECT_LIST_SET_CALLBACK_FUNCTION( object_type ) \
PROTOTYPE_CHOOSE_OBJECT_LIST_SET_CALLBACK_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 24 July 1997 \
\
DESCRIPTION : \
Changes the callback item of the choose_object_list_widget. \
============================================================================*/ \
{ \
	int return_code; \
	struct CHOOSE_OBJECT_LIST_STRUCT(object_type) *temp_choose_object_list; \
\
	ENTER(CHOOSE_OBJECT_LIST_SET_CALLBACK(object_type)); \
	/* check arguments */ \
	if (choose_object_list_widget&&new_callback) \
	{ \
		/* Get the pointer to the data for the choose_object_list dialog */ \
		XtVaGetValues(choose_object_list_widget,XmNuserData, \
			&temp_choose_object_list,NULL); \
		if (temp_choose_object_list) \
		{ \
			temp_choose_object_list->update_callback.procedure= \
				new_callback->procedure; \
			temp_choose_object_list->update_callback.data=new_callback->data; \
			return_code=1; \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"CHOOSE_OBJECT_LIST_SET_CALLBACK(" #object_type \
				").  Missing widget data"); \
			return_code=0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"CHOOSE_OBJECT_LIST_SET_CALLBACK(" #object_type ").  Missing widget"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* CHOOSE_OBJECT_LIST_SET_CALLBACK(object_type) */

#define DECLARE_CHOOSE_OBJECT_LIST_SET_OBJECT_FUNCTION( object_type ) \
PROTOTYPE_CHOOSE_OBJECT_LIST_SET_OBJECT_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 10 September 1998 \
\
DESCRIPTION : \
Changes the chosen object in the choose_object_list_widget. \
============================================================================*/ \
{ \
	int return_code; \
	struct CHOOSE_OBJECT_LIST_STRUCT(object_type) *temp_choose_object_list; \
\
	ENTER(CHOOSE_OBJECT_LIST_SET_OBJECT(object_type)); \
	/* check arguments */ \
	if (choose_object_list_widget) \
	{ \
		/* Get the pointer to the data for the choose_object_list dialog */ \
		XtVaGetValues(choose_object_list_widget,XmNuserData, \
			&temp_choose_object_list,NULL); \
		if (temp_choose_object_list) \
		{ \
			/* set last_updated_object to avoid update once set */ \
			temp_choose_object_list->last_updated_object=new_object; \
			if (!(temp_choose_object_list->current_object)) \
			{ \
				/* make the menu since list may have changed */ \
				CHOOSE_OBJECT_LIST_MAKE_MENU(object_type)(temp_choose_object_list); \
			} \
			temp_choose_object_list->current_object=new_object; \
			CHOOSE_OBJECT_LIST_SELECT_MENU_ITEM(object_type)( \
				temp_choose_object_list); \
			return_code=1; \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"CHOOSE_OBJECT_LIST_SET_OBJECT(" #object_type \
				").  Missing widget data"); \
			return_code=0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"CHOOSE_OBJECT_LIST_SET_OBJECT(" #object_type ").  Missing widget"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* CHOOSE_OBJECT_LIST_SET_OBJECT(object_type) */

#define DECLARE_CHOOSE_OBJECT_LIST_GET_CALLBACK_FUNCTION( object_type ) \
PROTOTYPE_CHOOSE_OBJECT_LIST_GET_CALLBACK_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 24 July 1997 \
\
DESCRIPTION : \
Returns a pointer to the callback item of the choose_object_list_widget. \
============================================================================*/ \
{ \
	struct Callback_data *return_address; \
	struct CHOOSE_OBJECT_LIST_STRUCT(object_type) *temp_choose_object_list; \
\
	ENTER(CHOOSE_OBJECT_LIST_GET_CALLBACK(object_type)); \
	/* check arguments */ \
	if (choose_object_list_widget) \
	{ \
		/* Get the pointer to the data for the choose_object_list dialog */ \
		XtVaGetValues(choose_object_list_widget,XmNuserData, \
			&temp_choose_object_list,NULL); \
		if (temp_choose_object_list) \
		{ \
			return_address=&(temp_choose_object_list->update_callback); \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"CHOOSE_OBJECT_LIST_GET_CALLBACK(" #object_type \
				").  Missing widget data"); \
			return_address=(struct Callback_data *)NULL; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"CHOOSE_OBJECT_LIST_GET_CALLBACK(" #object_type ").  Missing widget"); \
		return_address=(struct Callback_data *)NULL; \
	} \
	LEAVE; \
\
	return (return_address); \
} /* CHOOSE_OBJECT_LIST_GET_CALLBACK(object_type) */

#define DECLARE_CHOOSE_OBJECT_LIST_GET_OBJECT_FUNCTION( object_type ) \
PROTOTYPE_CHOOSE_OBJECT_LIST_GET_OBJECT_FUNCTION( object_type ) \
/***************************************************************************** \
LAST MODIFIED : 24 July 1997 \
\
DESCRIPTION : \
Returns the currently chosen object in the choose_object_list_widget. \
============================================================================*/ \
{ \
	struct object_type *return_address; \
	struct CHOOSE_OBJECT_LIST_STRUCT(object_type) *temp_choose_object_list; \
\
	ENTER(CHOOSE_OBJECT_LIST_GET_OBJECT(object_type)); \
	/* check arguments */ \
	if (choose_object_list_widget) \
	{ \
		/* Get the pointer to the data for the choose_object_list dialog */ \
		XtVaGetValues(choose_object_list_widget,XmNuserData, \
			&temp_choose_object_list,NULL); \
		if (temp_choose_object_list) \
		{ \
			return_address=temp_choose_object_list->current_object; \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"CHOOSE_OBJECT_LIST_GET_OBJECT(" #object_type \
				").  Missing widget data"); \
			return_address=(struct object_type *)NULL; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"CHOOSE_OBJECT_LIST_GET_OBJECT(" #object_type ").  Missing widget"); \
		return_address=(struct object_type *)NULL; \
	} \
	LEAVE; \
\
	return (return_address); \
} /* CHOOSE_OBJECT_LIST_GET_OBJECT(object_type) */

#define DECLARE_CHOOSE_OBJECT_LIST_REFRESH_FUNCTION( object_type ) \
PROTOTYPE_CHOOSE_OBJECT_LIST_REFRESH_FUNCTION( object_type ) \
/***************************************************************************** \
LAST MODIFIED : 24 July 1997 \
\
DESCRIPTION : \
Tells the choose_object_list_widget that the list has changed. \
============================================================================*/ \
{ \
	int return_code; \
	struct CHOOSE_OBJECT_LIST_STRUCT(object_type) *temp_choose_object_list; \
\
	ENTER(CHOOSE_OBJECT_LIST_REFRESH(object_type)); \
	/* check arguments */ \
	if (choose_object_list_widget) \
	{ \
		/* Get the pointer to the data for the choose_object_list dialog */ \
		XtVaGetValues(choose_object_list_widget,XmNuserData, \
			&temp_choose_object_list,NULL); \
		if (temp_choose_object_list) \
		{ \
			CHOOSE_OBJECT_LIST_MAKE_MENU(object_type)(temp_choose_object_list); \
			CHOOSE_OBJECT_LIST_SELECT_MENU_ITEM(object_type)( \
				temp_choose_object_list); \
			return_code=1; \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"CHOOSE_OBJECT_LIST_REFRESH(" #object_type \
				").  Missing widget data"); \
			return_code=0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"CHOOSE_OBJECT_LIST_REFRESH(" #object_type ").  Missing widget"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* CHOOSE_OBJECT_LIST_REFRESH(object_type) */

#define DECLARE_CHOOSE_OBJECT_LIST_MODULE_FUNCTIONS( object_type ) \
DECLARE_CHOOSE_OBJECT_LIST_DIALOG_IDENTIFY_FUNCTIONS(object_type) \
DECLARE_CHOOSE_OBJECT_LIST_UPDATE_FUNCTION(object_type) \
DECLARE_CHOOSE_OBJECT_LIST_DESTROY_CB_FUNCTION(object_type) \
DECLARE_CHOOSE_OBJECT_LIST_ADD_MENU_ITEM_FUNCTION(object_type) \
DECLARE_CHOOSE_OBJECT_LIST_MAKE_MENU_FUNCTION(object_type) \
DECLARE_CHOOSE_OBJECT_LIST_SELECT_MENU_ITEM_FUNCTION(object_type) \
DECLARE_CHOOSE_OBJECT_LIST_MENU_CB_FUNCTION(object_type)

#define DECLARE_CHOOSE_OBJECT_LIST_GLOBAL_FUNCTIONS( object_type ) \
DECLARE_CREATE_CHOOSE_OBJECT_LIST_WIDGET_FUNCTION(object_type) \
DECLARE_CHOOSE_OBJECT_LIST_SET_CALLBACK_FUNCTION(object_type) \
DECLARE_CHOOSE_OBJECT_LIST_SET_OBJECT_FUNCTION(object_type) \
DECLARE_CHOOSE_OBJECT_LIST_GET_CALLBACK_FUNCTION(object_type) \
DECLARE_CHOOSE_OBJECT_LIST_GET_OBJECT_FUNCTION(object_type) \
DECLARE_CHOOSE_OBJECT_LIST_REFRESH_FUNCTION(object_type)
#endif /* !defined (CHOOSE_OBJECT_LIST_PRIVATE_H) */
