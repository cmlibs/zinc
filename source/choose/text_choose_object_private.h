/*******************************************************************************
FILE : text_choose_object_private.h

LAST MODIFIED : 28 June 1999

DESCRIPTION :
Macros for implementing an option menu dialog control for choosing an object
from its manager. Handles manager messages to keep the menu up-to-date.
Calls the client-specified callback routine if a different object is chosen.
==============================================================================*/
#if !defined (TEXT_CHOOSE_OBJECT_PRIVATE_H)
#define TEXT_CHOOSE_OBJECT_PRIVATE_H

#include <stdio.h>
#if defined (MOTIF)
#include <Xm/Xm.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#endif /* defined (MOTIF) */
#include "general/debug.h"
#include "choose/text_choose_object.h"
#include "choose/text_choose_object.uid64"
#include "user_interface/gui_dialog_macros.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module variables
----------------
*/
#if defined (FULL_NAMES)
#define TEXT_CHOOSE_OBJECT_STRUCT_( object_type ) \
	text_choose_object_struct_ ## object_type
#else
#define TEXT_CHOOSE_OBJECT_STRUCT_( object_type ) costru ## object_type
#endif
#define TEXT_CHOOSE_OBJECT_STRUCT( object_type ) \
	TEXT_CHOOSE_OBJECT_STRUCT_(object_type)

#define FULL_DECLARE_TEXT_CHOOSE_OBJECT_STRUCT_TYPE( object_type ) \
struct TEXT_CHOOSE_OBJECT_STRUCT(object_type) \
/***************************************************************************** \
LAST MODIFIED : 18 August 1998 \
\
DESCRIPTION : \
Contains information required by the text_choose_object control dialog. \
============================================================================*/ \
{ \
	struct object_type *current_object,*last_updated_object; \
	struct MANAGER(object_type) *object_manager; \
	void *manager_callback_id; \
	MANAGER_CONDITIONAL_FUNCTION(object_type) *conditional_function; \
	int (*object_to_string)(struct object_type *,char **); \
	struct object_type *(*string_to_object)(char *, \
		struct MANAGER(object_type) *); \
	Widget widget_parent,widget; \
	struct Callback_data update_callback; \
} /* struct TEXT_CHOOSE_OBJECT_STRUCT(object_type) */

#if defined (MOTIF)
static int text_choose_object_hierarchy_open=0;
static MrmHierarchy text_choose_object_hierarchy;
#endif /* defined (MOTIF) */

/*
Module functions
----------------
*/
#if defined (FULL_NAMES)
#define TEXT_CHOOSE_OBJECT_DIALOG_NAME_( object_type ) \
	text_choose_object_ ## object_type
#else
#define TEXT_CHOOSE_OBJECT_DIALOG_NAME_( object_type ) co_ ## object_type
#endif
#define TEXT_CHOOSE_OBJECT_DIALOG_NAME( object_type ) \
	TEXT_CHOOSE_OBJECT_DIALOG_NAME_( object_type )

#if defined (FULL_NAMES)
#define TEXT_CHOOSE_OBJECT_UPDATE_( object_type ) \
	text_choose_object_update_ ## object_type
#else
#define TEXT_CHOOSE_OBJECT_UPDATE_( object_type ) cou ## object_type
#endif
#define TEXT_CHOOSE_OBJECT_UPDATE( object_type ) TEXT_CHOOSE_OBJECT_UPDATE_(object_type)

#define DECLARE_TEXT_CHOOSE_OBJECT_UPDATE_FUNCTION( object_type ) \
static int TEXT_CHOOSE_OBJECT_UPDATE(object_type)( \
	struct TEXT_CHOOSE_OBJECT_STRUCT(object_type) *text_chooser) \
/***************************************************************************** \
LAST MODIFIED : 18 August 1998 \
\
DESCRIPTION : \
Tells CMGUI about the current values. Sends a pointer to the current object. \
Avoids sending repeated updates if the object address has not changed. \
============================================================================*/ \
{ \
	int return_code; \
/*	char *temp_name; */ \
\
	ENTER(TEXT_CHOOSE_OBJECT_UPDATE(object_type)); \
	if (text_chooser) \
	{ \
		if (text_chooser->current_object != \
			text_chooser->last_updated_object) \
		{ \
/* \
			printf("text_choose_object_update(" #object_type "): "); \
			if (text_chooser->current_object) \
			{ \
				if (GET_NAME(object_type)( \
					text_chooser->current_object,&temp_name)) \
				{ \
					printf("%s\n",temp_name); \
					DEALLOCATE(temp_name); \
				} \
				else \
				{ \
					printf("NULL\n"); \
				} \
			} \
			else \
			{ \
				printf("NULL\n"); \
			} \
*/ \
			if (text_chooser->update_callback.procedure) \
			{ \
				/* now call the procedure with the user data and the position data */ \
				(text_chooser->update_callback.procedure)( \
					text_chooser->widget,text_chooser->update_callback.data, \
					text_chooser->current_object); \
			} \
			text_chooser->last_updated_object= \
				text_chooser->current_object; \
		} \
		return_code=1; \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"TEXT_CHOOSE_OBJECT_UPDATE(" #object_type ").  Invalid argument(s)"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* TEXT_CHOOSE_OBJECT_UPDATE(object_type) */

#if defined (FULL_NAMES)
#define TEXT_CHOOSE_OBJECT_DESTROY_CB_( object_type ) \
	text_choose_object_destroy_cb_ ## object_type
#else
#define TEXT_CHOOSE_OBJECT_DESTROY_CB_( object_type ) codc ## object_type
#endif
#define TEXT_CHOOSE_OBJECT_DESTROY_CB( object_type ) \
	TEXT_CHOOSE_OBJECT_DESTROY_CB_(object_type)

#define DECLARE_TEXT_CHOOSE_OBJECT_DESTROY_CB_FUNCTION( object_type ) \
static void TEXT_CHOOSE_OBJECT_DESTROY_CB(object_type)(Widget widget, \
	XtPointer client_data,XtPointer call_data) \
/***************************************************************************** \
LAST MODIFIED : 28 June 1999 \
\
DESCRIPTION : \
Callback for the text_choose_object dialog - tidies up all memory allocation. \
============================================================================*/ \
{ \
	struct TEXT_CHOOSE_OBJECT_STRUCT(object_type) *text_chooser; \
\
	ENTER(TEXT_CHOOSE_OBJECT_DESTROY_CB(object_type)); \
	USE_PARAMETER(call_data); \
	if (widget&&(text_chooser= \
		(struct TEXT_CHOOSE_OBJECT_STRUCT(object_type) *)client_data)) \
	{ \
		if (text_chooser->manager_callback_id) \
		{ \
			MANAGER_DEREGISTER(object_type)(text_chooser->manager_callback_id, \
				text_chooser->object_manager); \
		} \
		DEALLOCATE(text_chooser); \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE,"TEXT_CHOOSE_OBJECT_DESTROY_CB(" \
			#object_type ").  Invalid argument(s)"); \
	} \
	LEAVE; \
} /* TEXT_CHOOSE_OBJECT_DESTROY_CB(object_type) */

#if defined (FULL_NAMES)
#define TEXT_CHOOSE_OBJECT_SELECT_OBJECT_( object_type ) \
	text_choose_object_select_object_ ## object_type
#else
#define TEXT_CHOOSE_OBJECT_SELECT_OBJECT_( object_type ) tcoso ## object_type
#endif
#define TEXT_CHOOSE_OBJECT_SELECT_OBJECT( object_type ) \
	TEXT_CHOOSE_OBJECT_SELECT_OBJECT_(object_type)

#define DECLARE_TEXT_CHOOSE_OBJECT_SELECT_OBJECT_FUNCTION( object_type ) \
static int TEXT_CHOOSE_OBJECT_SELECT_OBJECT(object_type)( \
	struct TEXT_CHOOSE_OBJECT_STRUCT(object_type) *text_chooser, \
	struct object_type *new_object) \
/***************************************************************************** \
LAST MODIFIED : 10 September 1999 \
\
DESCRIPTION : \
Makes sure the <new_object> is valid for this text chooser, then calls an \
update in case it has changed, and writes the new object string in the widget. \
============================================================================*/ \
{ \
	static char *null_object_name="<NONE>"; \
	char *object_name; \
	int return_code; \
\
	ENTER(TEXT_CHOOSE_OBJECT_SELECT_OBJECT(object_type)); \
	/* check arguments */ \
	if (text_chooser) \
	{ \
		if (new_object&& \
			(!IS_MANAGED(object_type)(new_object,text_chooser->object_manager)|| \
			(text_chooser->conditional_function && \
			!(text_chooser->conditional_function(new_object,(void *)NULL))))) \
		{ \
			display_message(ERROR_MESSAGE, \
				"TEXT_CHOOSE_OBJECT_SELECT_OBJECT(object_type).  Invalid object"); \
			new_object=(struct object_type *)NULL; \
		} \
		if (new_object) \
		{ \
			text_chooser->current_object=new_object; \
		} \
		else \
		{ \
			if (!text_chooser->current_object) \
			{ \
				text_chooser->current_object=FIRST_OBJECT_IN_MANAGER_THAT(object_type)(\
					text_chooser->conditional_function,(void *)NULL, \
					text_chooser->object_manager); \
			} \
		} \
		/* write out the current_object */ \
		if (text_chooser->current_object) \
		{ \
			if (text_chooser->object_to_string(text_chooser->current_object, \
				&object_name)) \
			{ \
				XtVaSetValues(text_chooser->widget,XmNvalue,object_name,NULL); \
				DEALLOCATE(object_name); \
			} \
		} \
		else \
		{ \
			XtVaSetValues(text_chooser->widget,XmNvalue,null_object_name,NULL); \
		} \
		/* inform the client of and change */ \
		TEXT_CHOOSE_OBJECT_UPDATE(object_type)(text_chooser); \
		return_code=1; \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"TEXT_CHOOSE_OBJECT_SELECT_OBJECT(" #object_type \
			").  Invalid argument(s)"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* TEXT_CHOOSE_OBJECT_SELECT_OBJECT(object_type) */

#if defined (FULL_NAMES)
#define TEXT_CHOOSE_OBJECT_CB_( object_type ) \
	text_choose_object_cb_ ## object_type
#else
#define TEXT_CHOOSE_OBJECT_CB_( object_type ) comc ## object_type
#endif
#define TEXT_CHOOSE_OBJECT_CB( object_type ) \
	TEXT_CHOOSE_OBJECT_CB_(object_type)

#define DECLARE_TEXT_CHOOSE_OBJECT_CB_FUNCTION( object_type ) \
static void TEXT_CHOOSE_OBJECT_CB(object_type)(Widget widget, \
	XtPointer client_data,XtPointer call_data) \
/***************************************************************************** \
LAST MODIFIED : 28 June 1999 \
\
DESCRIPTION : \
Callback for the text field - change of object. \
============================================================================*/ \
{ \
	char *object_name; \
	struct TEXT_CHOOSE_OBJECT_STRUCT(object_type) *text_chooser; \
\
	ENTER(TEXT_CHOOSE_OBJECT_CB(object_type)); \
	USE_PARAMETER(call_data); \
	if (widget&&(text_chooser= \
		(struct TEXT_CHOOSE_OBJECT_STRUCT(object_type) *)client_data)) \
	{ \
		/* Get the object name string */ \
		XtVaGetValues(widget,XmNvalue,&object_name,NULL); \
		TEXT_CHOOSE_OBJECT_SELECT_OBJECT(object_type)(text_chooser, \
			text_chooser->string_to_object(object_name, \
				text_chooser->object_manager)); \
		if (object_name) \
		{ \
			XtFree(object_name); \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"TEXT_CHOOSE_OBJECT_CB(" #object_type ").  Invalid argument(s)"); \
	} \
	LEAVE; \
} /* TEXT_CHOOSE_OBJECT_CB(object_type) */

#if defined (FULL_NAMES)
#define TEXT_CHOOSE_OBJECT_GLOBAL_OBJECT_CHANGE_( object_type ) \
	text_choose_object_global_object_change_ ## object_type
#else
#define TEXT_CHOOSE_OBJECT_GLOBAL_OBJECT_CHANGE_( object_type ) cogoc ## object_type
#endif
#define TEXT_CHOOSE_OBJECT_GLOBAL_OBJECT_CHANGE( object_type ) \
	TEXT_CHOOSE_OBJECT_GLOBAL_OBJECT_CHANGE_(object_type)

#define DECLARE_TEXT_CHOOSE_OBJECT_GLOBAL_OBJECT_CHANGE_FUNCTION( object_type ) \
static void TEXT_CHOOSE_OBJECT_GLOBAL_OBJECT_CHANGE(object_type)( \
	struct MANAGER_MESSAGE(object_type) *message,void *data) \
/***************************************************************************** \
LAST MODIFIED : 18 August 1998 \
\
DESCRIPTION : \
Updates the chosen object and text field in response to manager messages. \
============================================================================*/ \
{ \
	struct TEXT_CHOOSE_OBJECT_STRUCT(object_type) *text_chooser; \
\
	ENTER(TEXT_CHOOSE_OBJECT_GLOBAL_OBJECT_CHANGE(object_type)); \
	if (message&&(text_chooser= \
		(struct TEXT_CHOOSE_OBJECT_STRUCT(object_type) *)data)) \
	{ \
		switch (message->change) \
		{ \
			case MANAGER_CHANGE_ALL(object_type): \
			{ \
				if ((text_chooser->current_object)&&!IS_MANAGED(object_type)( \
					text_chooser->current_object,text_chooser->object_manager)) \
				{ \
					text_chooser->current_object=(struct object_type *)NULL; \
				} \
				TEXT_CHOOSE_OBJECT_SELECT_OBJECT(object_type)(text_chooser, \
					text_chooser->current_object); \
			} break; \
			case MANAGER_CHANGE_DELETE(object_type): \
			{ \
				if (message->object_changed==text_chooser->current_object) \
				{ \
					TEXT_CHOOSE_OBJECT_SELECT_OBJECT(object_type)(text_chooser, \
						(struct object_type *)NULL); \
				} \
			} break; \
			case MANAGER_CHANGE_ADD(object_type): \
			{ \
				if (!text_chooser->current_object) \
				{ \
					TEXT_CHOOSE_OBJECT_SELECT_OBJECT(object_type)(text_chooser, \
						(struct object_type *)NULL); \
				} \
			} break; \
			case MANAGER_CHANGE_IDENTIFIER(object_type): \
			case MANAGER_CHANGE_OBJECT(object_type): \
			{ \
				if (message->object_changed==text_chooser->current_object) \
				{ \
					TEXT_CHOOSE_OBJECT_SELECT_OBJECT(object_type)(text_chooser, \
						text_chooser->current_object); \
				} \
			} break; \
			case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(object_type): \
			{ \
				/* do nothing */ \
			} break; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"TEXT_CHOOSE_OBJECT_GLOBAL_OBJECT_CHANGE(" #object_type \
			").  Invalid argument(s)"); \
	} \
	LEAVE; \
} /* TEXT_CHOOSE_OBJECT_GLOBAL_OBJECT_CHANGE(object_type) */

/*
Global functions
----------------
*/
#define DECLARE_CREATE_TEXT_CHOOSE_OBJECT_WIDGET_FUNCTION( object_type ) \
PROTOTYPE_CREATE_TEXT_CHOOSE_OBJECT_WIDGET_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 18 August 1998 \
\
DESCRIPTION : \
Creates an option menu from which an object from the manager may be chosen. \
============================================================================*/ \
{ \
	MrmType text_choose_object_dialog_class; \
	struct TEXT_CHOOSE_OBJECT_STRUCT(object_type) *text_chooser; \
	static MrmRegisterArg callback_list[]= \
	{ \
		{"text_choose_object_destroy_CB",(XtPointer) \
			TEXT_CHOOSE_OBJECT_DESTROY_CB(object_type)}, \
		{"text_choose_object_CB",(XtPointer) \
			TEXT_CHOOSE_OBJECT_CB(object_type)} \
	}; \
	static MrmRegisterArg identifier_list[]= \
	{ \
		{"Text_choose_object_structure",(XtPointer)NULL} \
	}; \
	Widget return_widget; \
\
	ENTER(CREATE_TEXT_CHOOSE_OBJECT_WIDGET(object_type)); \
	return_widget=(Widget)NULL; \
	/* check arguments */ \
	if (parent&&object_manager&&object_to_string&&string_to_object) \
	{ \
		if (MrmOpenHierarchy_base64_string(text_choose_object_uid64, \
			&text_choose_object_hierarchy,&text_choose_object_hierarchy_open)) \
		{ \
			/* allocate memory */ \
			if (ALLOCATE(text_chooser, \
				struct TEXT_CHOOSE_OBJECT_STRUCT(object_type),1)) \
			{ \
				/* initialise the structure */ \
				text_chooser->object_manager=object_manager; \
				text_chooser->manager_callback_id=(void *)NULL; \
				text_chooser->conditional_function=conditional_function; \
				text_chooser->object_to_string=object_to_string; \
				text_chooser->string_to_object=string_to_object; \
				text_chooser->widget_parent=parent; \
				text_chooser->widget=(Widget)NULL; \
				text_chooser->update_callback.procedure=(Callback_procedure *)NULL; \
				text_chooser->update_callback.data=(void *)NULL; \
				text_chooser->current_object=(struct object_type *)NULL; \
				text_chooser->last_updated_object=(struct object_type *)NULL; \
				/* register the callbacks */ \
				if (MrmSUCCESS==MrmRegisterNamesInHierarchy( \
					text_choose_object_hierarchy, \
					callback_list,XtNumber(callback_list))) \
				{ \
					/* assign and register the identifiers */ \
					identifier_list[0].value=(XtPointer)text_chooser; \
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy( \
						text_choose_object_hierarchy, \
						identifier_list,XtNumber(identifier_list))) \
					{ \
						/* fetch text_choose_object control widget */ \
						if (MrmSUCCESS==MrmFetchWidget(text_choose_object_hierarchy, \
							"text_choose_object",text_chooser->widget_parent, \
							&(text_chooser->widget),&text_choose_object_dialog_class)) \
						{ \
							/* register for any object changes */ \
							text_chooser->manager_callback_id=MANAGER_REGISTER(object_type)( \
								TEXT_CHOOSE_OBJECT_GLOBAL_OBJECT_CHANGE(object_type), \
								(void *)text_chooser,text_chooser->object_manager); \
							TEXT_CHOOSE_OBJECT_SET_OBJECT(object_type)( \
								text_chooser->widget,current_object); \
							XtManageChild(text_chooser->widget); \
							return_widget=text_chooser->widget; \
						} \
						else \
						{ \
							display_message(ERROR_MESSAGE, \
								"CREATE_TEXT_CHOOSE_OBJECT_WIDGET(" #object_type \
								").  Could not fetch text_choose_object dialog"); \
							DEALLOCATE(text_chooser); \
						} \
					} \
					else \
					{ \
						display_message(ERROR_MESSAGE, \
							"CREATE_TEXT_CHOOSE_OBJECT_WIDGET(" #object_type \
							").  Could not register identifiers"); \
						DEALLOCATE(text_chooser); \
					} \
				} \
				else \
				{ \
					display_message(ERROR_MESSAGE, \
						"CREATE_TEXT_CHOOSE_OBJECT_WIDGET(" #object_type \
						").  Could not register callbacks"); \
					DEALLOCATE(text_chooser); \
				} \
			} \
			else \
			{ \
				display_message(ERROR_MESSAGE, \
					"CREATE_TEXT_CHOOSE_OBJECT_WIDGET(" #object_type \
					").  Could not allocate control window structure"); \
			} \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"CREATE_TEXT_CHOOSE_OBJECT_WIDGET(" #object_type \
				").  Could not open hierarchy"); \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"CREATE_TEXT_CHOOSE_OBJECT_WIDGET(" #object_type ").  Invalid argument(s)"); \
	} \
	LEAVE; \
\
	return (return_widget); \
} /* CREATE_TEXT_CHOOSE_OBJECT_WIDGET(object_type) */

#define DECLARE_TEXT_CHOOSE_OBJECT_SET_CALLBACK_FUNCTION( object_type ) \
PROTOTYPE_TEXT_CHOOSE_OBJECT_SET_CALLBACK_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 18 August 1998 \
\
DESCRIPTION : \
Changes the callback item of the text_choose_object_widget. \
============================================================================*/ \
{ \
	int return_code; \
	struct TEXT_CHOOSE_OBJECT_STRUCT(object_type) *text_chooser; \
\
	ENTER(TEXT_CHOOSE_OBJECT_SET_CALLBACK(object_type)); \
	/* check arguments */ \
	if (text_choose_object_widget&&new_callback) \
	{ \
		/* Get the pointer to the data for the text_choose_object dialog */ \
		XtVaGetValues(text_choose_object_widget,XmNuserData, \
			&text_chooser,NULL); \
		if (text_chooser) \
		{ \
			text_chooser->update_callback.procedure= \
				new_callback->procedure; \
			text_chooser->update_callback.data=new_callback->data; \
			return_code=1; \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"TEXT_CHOOSE_OBJECT_SET_CALLBACK(" #object_type \
				").  Missing widget data"); \
			return_code=0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"TEXT_CHOOSE_OBJECT_SET_CALLBACK(" #object_type ").  Missing widget"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* TEXT_CHOOSE_OBJECT_SET_CALLBACK(object_type) */

#define DECLARE_TEXT_CHOOSE_OBJECT_SET_OBJECT_FUNCTION( object_type ) \
PROTOTYPE_TEXT_CHOOSE_OBJECT_SET_OBJECT_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 28 June 1999 \
\
DESCRIPTION : \
Changes the chosen object in the text_choose_object_widget. \
============================================================================*/ \
{ \
	int return_code; \
	struct TEXT_CHOOSE_OBJECT_STRUCT(object_type) *text_chooser; \
\
	ENTER(TEXT_CHOOSE_OBJECT_SET_OBJECT(object_type)); \
	if (text_choose_object_widget) \
	{ \
		/* Get the pointer to the data for the text_choose_object dialog */ \
		XtVaGetValues(text_choose_object_widget,XmNuserData, \
			&text_chooser,NULL); \
		if (text_chooser) \
		{ \
			/* set the last_updated_object to the new_object since it is what the \
				 rest of the application now thinks is the chosen object. */ \
			text_chooser->last_updated_object=new_object; \
			return_code=TEXT_CHOOSE_OBJECT_SELECT_OBJECT(object_type)( \
				text_chooser,new_object); \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE,"TEXT_CHOOSE_OBJECT_SET_OBJECT(" \
				#object_type ").  Missing widget data"); \
			return_code=0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"TEXT_CHOOSE_OBJECT_SET_OBJECT(" #object_type ").  Missing widget"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* TEXT_CHOOSE_OBJECT_SET_OBJECT(object_type) */

#define DECLARE_TEXT_CHOOSE_OBJECT_GET_CALLBACK_FUNCTION( object_type ) \
PROTOTYPE_TEXT_CHOOSE_OBJECT_GET_CALLBACK_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 18 August 1998 \
\
DESCRIPTION : \
Returns a pointer to the callback item of the text_choose_object_widget. \
============================================================================*/ \
{ \
	struct Callback_data *return_address; \
	struct TEXT_CHOOSE_OBJECT_STRUCT(object_type) *text_chooser; \
\
	ENTER(TEXT_CHOOSE_OBJECT_GET_CALLBACK(object_type)); \
	/* check arguments */ \
	if (text_choose_object_widget) \
	{ \
		/* Get the pointer to the data for the text_choose_object dialog */ \
		XtVaGetValues(text_choose_object_widget,XmNuserData, \
			&text_chooser,NULL); \
		if (text_chooser) \
		{ \
			return_address=&(text_chooser->update_callback); \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"TEXT_CHOOSE_OBJECT_GET_CALLBACK(" #object_type \
				").  Missing widget data"); \
			return_address=(struct Callback_data *)NULL; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"TEXT_CHOOSE_OBJECT_GET_CALLBACK(" #object_type ").  Missing widget"); \
		return_address=(struct Callback_data *)NULL; \
	} \
	LEAVE; \
\
	return (return_address); \
} /* TEXT_CHOOSE_OBJECT_GET_CALLBACK(object_type) */

#define DECLARE_TEXT_CHOOSE_OBJECT_GET_OBJECT_FUNCTION( object_type ) \
PROTOTYPE_TEXT_CHOOSE_OBJECT_GET_OBJECT_FUNCTION( object_type ) \
/***************************************************************************** \
LAST MODIFIED : 18 August 1998 \
\
DESCRIPTION : \
Returns the currently chosen object in the text_choose_object_widget. \
============================================================================*/ \
{ \
	struct object_type *return_address; \
	struct TEXT_CHOOSE_OBJECT_STRUCT(object_type) *text_chooser; \
\
	ENTER(TEXT_CHOOSE_OBJECT_GET_OBJECT(object_type)); \
	/* check arguments */ \
	if (text_choose_object_widget) \
	{ \
		/* Get the pointer to the data for the text_choose_object dialog */ \
		XtVaGetValues(text_choose_object_widget,XmNuserData, \
			&text_chooser,NULL); \
		if (text_chooser) \
		{ \
			return_address=text_chooser->current_object; \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE,"TEXT_CHOOSE_OBJECT_GET_OBJECT(" \
				#object_type ").  Missing widget data"); \
			return_address=(struct object_type *)NULL; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"TEXT_CHOOSE_OBJECT_GET_OBJECT(" #object_type ").  Missing widget"); \
		return_address=(struct object_type *)NULL; \
	} \
	LEAVE; \
\
	return (return_address); \
} /* TEXT_CHOOSE_OBJECT_GET_OBJECT(object_type) */

#define DECLARE_TEXT_CHOOSE_OBJECT_MODULE_FUNCTIONS( object_type ) \
DECLARE_TEXT_CHOOSE_OBJECT_UPDATE_FUNCTION(object_type) \
DECLARE_TEXT_CHOOSE_OBJECT_DESTROY_CB_FUNCTION(object_type) \
DECLARE_TEXT_CHOOSE_OBJECT_SELECT_OBJECT_FUNCTION(object_type) \
DECLARE_TEXT_CHOOSE_OBJECT_CB_FUNCTION(object_type) \
DECLARE_TEXT_CHOOSE_OBJECT_GLOBAL_OBJECT_CHANGE_FUNCTION(object_type)

#define DECLARE_TEXT_CHOOSE_OBJECT_GLOBAL_FUNCTIONS( object_type ) \
DECLARE_CREATE_TEXT_CHOOSE_OBJECT_WIDGET_FUNCTION(object_type) \
DECLARE_TEXT_CHOOSE_OBJECT_SET_CALLBACK_FUNCTION(object_type) \
DECLARE_TEXT_CHOOSE_OBJECT_SET_OBJECT_FUNCTION(object_type) \
DECLARE_TEXT_CHOOSE_OBJECT_GET_CALLBACK_FUNCTION(object_type) \
DECLARE_TEXT_CHOOSE_OBJECT_GET_OBJECT_FUNCTION(object_type)
#endif /* !defined (TEXT_CHOOSE_OBJECT_PRIVATE_H) */
