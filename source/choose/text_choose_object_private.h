/*******************************************************************************
FILE : text_choose_object_private.h

LAST MODIFIED : 9 June 2000

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
#include <Xm/TextF.h>
#endif /* defined (MOTIF) */
#include "general/debug.h"
#include "choose/text_choose_object.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module variables
----------------
*/
#if defined (FULL_NAMES)
#define TEXT_CHOOSE_OBJECT_( object_type ) \
	text_choose_object_ ## object_type
#else
#define TEXT_CHOOSE_OBJECT_( object_type ) tcot ## object_type
#endif
#define TEXT_CHOOSE_OBJECT( object_type ) \
	TEXT_CHOOSE_OBJECT_(object_type)

#define FULL_DECLARE_TEXT_CHOOSE_OBJECT_TYPE( object_type ) \
struct TEXT_CHOOSE_OBJECT(object_type) \
/***************************************************************************** \
LAST MODIFIED : 9 June 2000 \
\
DESCRIPTION : \
Contains information required by the text_choose_object control dialog. \
============================================================================*/ \
{ \
	struct object_type *current_object,*last_updated_object; \
	struct MANAGER(object_type) *object_manager; \
	void *manager_callback_id; \
	MANAGER_CONDITIONAL_FUNCTION(object_type) *conditional_function; \
	void *conditional_function_user_data; \
	int (*object_to_string)(struct object_type *,char **); \
	struct object_type *(*string_to_object)(char *, \
		struct MANAGER(object_type) *); \
	Widget widget_parent,widget; \
	struct Callback_data update_callback; \
} /* struct TEXT_CHOOSE_OBJECT(object_type) */

/*
Module functions
----------------
*/

#if defined (FULL_NAMES)
#define TEXT_CHOOSE_OBJECT_UPDATE_( object_type ) \
	text_choose_object_update_ ## object_type
#else
#define TEXT_CHOOSE_OBJECT_UPDATE_( object_type ) tcou ## object_type
#endif
#define TEXT_CHOOSE_OBJECT_UPDATE( object_type ) \
	TEXT_CHOOSE_OBJECT_UPDATE_(object_type)

#define DECLARE_TEXT_CHOOSE_OBJECT_UPDATE_FUNCTION( object_type ) \
static int TEXT_CHOOSE_OBJECT_UPDATE(object_type)( \
	struct TEXT_CHOOSE_OBJECT(object_type) *text_choose_object) \
/***************************************************************************** \
LAST MODIFIED : 18 August 1998 \
\
DESCRIPTION : \
Tells CMGUI about the current values. Sends a pointer to the current object. \
Avoids sending repeated updates if the object address has not changed. \
============================================================================*/ \
{ \
	int return_code; \
\
	ENTER(TEXT_CHOOSE_OBJECT_UPDATE(object_type)); \
	if (text_choose_object) \
	{ \
		if (text_choose_object->current_object != \
			text_choose_object->last_updated_object) \
		{ \
			if (text_choose_object->update_callback.procedure) \
			{ \
				/* now call the procedure with the user data and the position data */ \
				(text_choose_object->update_callback.procedure)( \
					text_choose_object->widget,text_choose_object->update_callback.data, \
					text_choose_object->current_object); \
			} \
			text_choose_object->last_updated_object= \
				text_choose_object->current_object; \
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
#define TEXT_CHOOSE_OBJECT_DESTROY_CB_( object_type ) tcodc ## object_type
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
	struct TEXT_CHOOSE_OBJECT(object_type) *text_choose_object; \
\
	ENTER(TEXT_CHOOSE_OBJECT_DESTROY_CB(object_type)); \
	USE_PARAMETER(call_data); \
	if (widget&&(text_choose_object= \
		(struct TEXT_CHOOSE_OBJECT(object_type) *)client_data)) \
	{ \
		if (text_choose_object->manager_callback_id) \
		{ \
			MANAGER_DEREGISTER(object_type)(text_choose_object->manager_callback_id, \
				text_choose_object->object_manager); \
		} \
		DEALLOCATE(text_choose_object); \
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
	struct TEXT_CHOOSE_OBJECT(object_type) *text_choose_object, \
	struct object_type *new_object) \
/***************************************************************************** \
LAST MODIFIED : 6 June 2000 \
\
DESCRIPTION : \
Makes sure the <new_object> is valid for this text chooser, then calls an \
update in case it has changed, and writes the new object string in the widget. \
============================================================================*/ \
{ \
	char *current_string; \
	static char *null_object_name="<NONE>"; \
	char *object_name; \
	int return_code; \
\
	ENTER(TEXT_CHOOSE_OBJECT_SELECT_OBJECT(object_type)); \
	/* check arguments */ \
	if (text_choose_object) \
	{ \
		if (current_string=XmTextFieldGetString(text_choose_object->widget)) \
		{ \
			if (new_object&&(!IS_MANAGED(object_type)(new_object, \
				text_choose_object->object_manager)|| \
				(text_choose_object->conditional_function && \
					!(text_choose_object->conditional_function(new_object, \
						text_choose_object->conditional_function_user_data))))) \
			{ \
				display_message(ERROR_MESSAGE, \
					"TEXT_CHOOSE_OBJECT_SELECT_OBJECT(object_type).  Invalid object"); \
				new_object=(struct object_type *)NULL; \
			} \
			if (new_object) \
			{ \
				text_choose_object->current_object=new_object; \
			} \
			else \
			{ \
				if (!text_choose_object->current_object) \
				{ \
						text_choose_object->current_object= \
						FIRST_OBJECT_IN_MANAGER_THAT(object_type)(\
							text_choose_object->conditional_function, \
							text_choose_object->conditional_function_user_data, \
							text_choose_object->object_manager); \
				} \
			} \
			/* write out the current_object */ \
			if (text_choose_object->current_object) \
			{ \
				if (text_choose_object->object_to_string( \
					text_choose_object->current_object,&object_name)) \
				{ \
					if (strcmp(object_name,current_string)) \
					{ \
						XmTextFieldSetString(text_choose_object->widget,object_name); \
					} \
					DEALLOCATE(object_name); \
				} \
			} \
			else \
			{ \
				if (strcmp(null_object_name,current_string)) \
				{ \
					XmTextFieldSetString(text_choose_object->widget,null_object_name); \
				} \
			} \
			/* inform the client of any change */ \
			TEXT_CHOOSE_OBJECT_UPDATE(object_type)(text_choose_object); \
			XtFree(current_string); \
		} \
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
#define TEXT_CHOOSE_OBJECT_CB_( object_type ) tcocb ## object_type
#endif
#define TEXT_CHOOSE_OBJECT_CB( object_type ) \
	TEXT_CHOOSE_OBJECT_CB_(object_type)

#define DECLARE_TEXT_CHOOSE_OBJECT_CB_FUNCTION( object_type ) \
static void TEXT_CHOOSE_OBJECT_CB(object_type)(Widget widget, \
	XtPointer client_data,XtPointer call_data) \
/***************************************************************************** \
LAST MODIFIED : 6 June 2000 \
\
DESCRIPTION : \
Callback for the text field - change of object. \
============================================================================*/ \
{ \
	char *object_name; \
	struct TEXT_CHOOSE_OBJECT(object_type) *text_choose_object; \
\
	ENTER(TEXT_CHOOSE_OBJECT_CB(object_type)); \
	USE_PARAMETER(call_data); \
	if (widget&&(text_choose_object= \
		(struct TEXT_CHOOSE_OBJECT(object_type) *)client_data)) \
	{ \
		/* Get the object name string */ \
		if (object_name=XmTextFieldGetString(widget)) \
		{ \
			TEXT_CHOOSE_OBJECT_SELECT_OBJECT(object_type)(text_choose_object, \
				text_choose_object->string_to_object(object_name, \
					text_choose_object->object_manager)); \
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
#define TEXT_CHOOSE_OBJECT_GLOBAL_OBJECT_CHANGE_( object_type ) \
	tcogoc ## object_type
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
	struct TEXT_CHOOSE_OBJECT(object_type) *text_choose_object; \
\
	ENTER(TEXT_CHOOSE_OBJECT_GLOBAL_OBJECT_CHANGE(object_type)); \
	if (message&&(text_choose_object= \
		(struct TEXT_CHOOSE_OBJECT(object_type) *)data)) \
	{ \
		switch (message->change) \
		{ \
			case MANAGER_CHANGE_ALL(object_type): \
			{ \
				if ((text_choose_object->current_object)&&!IS_MANAGED(object_type)( \
					text_choose_object->current_object, \
					text_choose_object->object_manager)) \
				{ \
					text_choose_object->current_object=(struct object_type *)NULL; \
				} \
				TEXT_CHOOSE_OBJECT_SELECT_OBJECT(object_type)(text_choose_object, \
					text_choose_object->current_object); \
			} break; \
			case MANAGER_CHANGE_DELETE(object_type): \
			{ \
				if (message->object_changed==text_choose_object->current_object) \
				{ \
					TEXT_CHOOSE_OBJECT_SELECT_OBJECT(object_type)(text_choose_object, \
						(struct object_type *)NULL); \
				} \
			} break; \
			case MANAGER_CHANGE_ADD(object_type): \
			{ \
				if (!text_choose_object->current_object) \
				{ \
					TEXT_CHOOSE_OBJECT_SELECT_OBJECT(object_type)(text_choose_object, \
						(struct object_type *)NULL); \
				} \
			} break; \
			case MANAGER_CHANGE_IDENTIFIER(object_type): \
			case MANAGER_CHANGE_OBJECT(object_type): \
			{ \
				if (message->object_changed==text_choose_object->current_object) \
				{ \
					TEXT_CHOOSE_OBJECT_SELECT_OBJECT(object_type)(text_choose_object, \
						text_choose_object->current_object); \
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
LAST MODIFIED : 9 June 2000 \
\
DESCRIPTION : \
Creates an option menu from which an object from the manager may be chosen. \
============================================================================*/ \
{ \
	Arg args[6]; \
	struct TEXT_CHOOSE_OBJECT(object_type) *text_choose_object; \
	Widget return_widget; \
\
	ENTER(CREATE_TEXT_CHOOSE_OBJECT_WIDGET(object_type)); \
	return_widget=(Widget)NULL; \
	if (parent&&object_manager&&object_to_string&&string_to_object) \
	{ \
		if (ALLOCATE(text_choose_object, \
			struct TEXT_CHOOSE_OBJECT(object_type),1)) \
		{ \
			/* initialise the structure */ \
			text_choose_object->object_manager=object_manager; \
			text_choose_object->manager_callback_id=(void *)NULL; \
			text_choose_object->conditional_function=conditional_function; \
			text_choose_object->conditional_function_user_data= \
				conditional_function_user_data; \
			text_choose_object->object_to_string=object_to_string; \
			text_choose_object->string_to_object=string_to_object; \
			text_choose_object->widget_parent=parent; \
			text_choose_object->widget=(Widget)NULL; \
			text_choose_object->update_callback.procedure= \
				(Callback_procedure *)NULL; \
			text_choose_object->update_callback.data=(void *)NULL; \
			text_choose_object->current_object=(struct object_type *)NULL; \
			text_choose_object->last_updated_object=(struct object_type *)NULL; \
			XtSetArg(args[0],XmNleftAttachment,XmATTACH_FORM); \
			XtSetArg(args[1],XmNrightAttachment,XmATTACH_FORM); \
			XtSetArg(args[2],XmNtopAttachment,XmATTACH_FORM); \
			XtSetArg(args[3],XmNbottomAttachment,XmATTACH_FORM); \
			XtSetArg(args[4],XmNuserData,(XtPointer)text_choose_object); \
			XtSetArg(args[5],XmNcolumns,(XtPointer)12); \
			if (text_choose_object->widget= \
				XmCreateTextField(parent,"text_choose_object",args,6))\
			{ \
				/* add callbacks for chooser widget */ \
				XtAddCallback(text_choose_object->widget,XmNdestroyCallback, \
					TEXT_CHOOSE_OBJECT_DESTROY_CB(object_type), \
					(XtPointer)text_choose_object); \
				XtAddCallback(text_choose_object->widget,XmNlosingFocusCallback, \
					TEXT_CHOOSE_OBJECT_CB(object_type),(XtPointer)text_choose_object); \
				XtAddCallback(text_choose_object->widget,XmNactivateCallback, \
					TEXT_CHOOSE_OBJECT_CB(object_type),(XtPointer)text_choose_object); \
				/* register for any object changes */ \
				text_choose_object->manager_callback_id=MANAGER_REGISTER(object_type)( \
					TEXT_CHOOSE_OBJECT_GLOBAL_OBJECT_CHANGE(object_type), \
					(void *)text_choose_object,text_choose_object->object_manager); \
				TEXT_CHOOSE_OBJECT_SET_OBJECT(object_type)( \
					text_choose_object->widget,current_object); \
				XtManageChild(text_choose_object->widget); \
				return_widget=text_choose_object->widget; \
			} \
			else \
			{ \
				display_message(ERROR_MESSAGE, \
					"CREATE_TEXT_CHOOSE_OBJECT_WIDGET(" #object_type \
					").  Could not create text field widget"); \
				DEALLOCATE(text_choose_object); \
			} \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"CREATE_TEXT_CHOOSE_OBJECT_WIDGET(" #object_type \
				").  Could not allocate structure"); \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"CREATE_TEXT_CHOOSE_OBJECT_WIDGET(" #object_type \
			").  Invalid argument(s)"); \
	} \
	LEAVE; \
\
	return (return_widget); \
} /* CREATE_TEXT_CHOOSE_OBJECT_WIDGET(object_type) */

#define DECLARE_TEXT_CHOOSE_OBJECT_GET_CALLBACK_FUNCTION( object_type ) \
PROTOTYPE_TEXT_CHOOSE_OBJECT_GET_CALLBACK_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 9 June 2000 \
\
DESCRIPTION : \
Returns a pointer to the callback item of the text_choose_object_widget. \
============================================================================*/ \
{ \
	struct Callback_data *return_address; \
	struct TEXT_CHOOSE_OBJECT(object_type) *text_choose_object; \
\
	ENTER(TEXT_CHOOSE_OBJECT_GET_CALLBACK(object_type)); \
	if (text_choose_object_widget) \
	{ \
		text_choose_object=(struct TEXT_CHOOSE_OBJECT(object_type) *)NULL; \
		/* Get the pointer to the data for the text_choose_object dialog */ \
		XtVaGetValues(text_choose_object_widget,XmNuserData, \
			&text_choose_object,NULL); \
		if (text_choose_object) \
		{ \
			return_address=&(text_choose_object->update_callback); \
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

#define DECLARE_TEXT_CHOOSE_OBJECT_SET_CALLBACK_FUNCTION( object_type ) \
PROTOTYPE_TEXT_CHOOSE_OBJECT_SET_CALLBACK_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 9 June 2000 \
\
DESCRIPTION : \
Changes the callback item of the text_choose_object_widget. \
============================================================================*/ \
{ \
	int return_code; \
	struct TEXT_CHOOSE_OBJECT(object_type) *text_choose_object; \
\
	ENTER(TEXT_CHOOSE_OBJECT_SET_CALLBACK(object_type)); \
	if (text_choose_object_widget&&new_callback) \
	{ \
		text_choose_object=(struct TEXT_CHOOSE_OBJECT(object_type) *)NULL; \
		/* Get the pointer to the data for the text_choose_object dialog */ \
		XtVaGetValues(text_choose_object_widget,XmNuserData, \
			&text_choose_object,NULL); \
		if (text_choose_object) \
		{ \
			text_choose_object->update_callback.procedure= \
				new_callback->procedure; \
			text_choose_object->update_callback.data=new_callback->data; \
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

#define DECLARE_TEXT_CHOOSE_OBJECT_GET_OBJECT_FUNCTION( object_type ) \
PROTOTYPE_TEXT_CHOOSE_OBJECT_GET_OBJECT_FUNCTION( object_type ) \
/***************************************************************************** \
LAST MODIFIED : 9 June 2000 \
\
DESCRIPTION : \
Returns the currently chosen object in the text_choose_object_widget. \
============================================================================*/ \
{ \
	struct object_type *return_address; \
	struct TEXT_CHOOSE_OBJECT(object_type) *text_choose_object; \
\
	ENTER(TEXT_CHOOSE_OBJECT_GET_OBJECT(object_type)); \
	if (text_choose_object_widget) \
	{ \
		text_choose_object=(struct TEXT_CHOOSE_OBJECT(object_type) *)NULL; \
		/* Get the pointer to the data for the text_choose_object dialog */ \
		XtVaGetValues(text_choose_object_widget,XmNuserData, \
			&text_choose_object,NULL); \
		if (text_choose_object) \
		{ \
			return_address=text_choose_object->current_object; \
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

#define DECLARE_TEXT_CHOOSE_OBJECT_SET_OBJECT_FUNCTION( object_type ) \
PROTOTYPE_TEXT_CHOOSE_OBJECT_SET_OBJECT_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 9 June 2000 \
\
DESCRIPTION : \
Changes the chosen object in the text_choose_object_widget. \
============================================================================*/ \
{ \
	int return_code; \
	struct TEXT_CHOOSE_OBJECT(object_type) *text_choose_object; \
\
	ENTER(TEXT_CHOOSE_OBJECT_SET_OBJECT(object_type)); \
	if (text_choose_object_widget) \
	{ \
		text_choose_object=(struct TEXT_CHOOSE_OBJECT(object_type) *)NULL; \
		/* Get the pointer to the data for the text_choose_object dialog */ \
		XtVaGetValues(text_choose_object_widget,XmNuserData, \
			&text_choose_object,NULL); \
		if (text_choose_object) \
		{ \
			/* set the last_updated_object to the new_object since it is what the \
				 rest of the application now thinks is the chosen object. */ \
			text_choose_object->last_updated_object=new_object; \
			return_code=TEXT_CHOOSE_OBJECT_SELECT_OBJECT(object_type)( \
				text_choose_object,new_object); \
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

#define DECLARE_TEXT_CHOOSE_OBJECT_CHANGE_CONDITIONAL_FUNCTION_FUNCTION( \
	object_type ) \
PROTOTYPE_TEXT_CHOOSE_OBJECT_CHANGE_CONDITIONAL_FUNCTION_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 9 June 2000 \
\
DESCRIPTION : \
Changes the conditional_function and user_data limiting the available \
selection of objects. Also allows new_object to be set simultaneously. \
============================================================================*/ \
{ \
	int return_code; \
	struct TEXT_CHOOSE_OBJECT(object_type) *text_choose_object; \
\
	ENTER(TEXT_CHOOSE_OBJECT_CHANGE_CONDITIONAL_FUNCTION(object_type)); \
	if (text_choose_object_widget) \
	{ \
		text_choose_object=(struct TEXT_CHOOSE_OBJECT(object_type) *)NULL; \
		/* Get the pointer to the data for the text_choose_object dialog */ \
		XtVaGetValues(text_choose_object_widget, \
			XmNuserData,&text_choose_object,NULL); \
		if (text_choose_object) \
		{ \
			text_choose_object->conditional_function=conditional_function; \
			text_choose_object->conditional_function_user_data= \
				conditional_function_user_data; \
			/* set the last_updated_object to the new_object since it is what the \
				 rest of the application now thinks is the chosen object. */ \
			text_choose_object->last_updated_object=new_object; \
			return_code=TEXT_CHOOSE_OBJECT_SELECT_OBJECT(object_type)( \
				text_choose_object,new_object); \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"TEXT_CHOOSE_OBJECT_CHANGE_CONDITIONAL_FUNCTION(" #object_type \
				").  Missing widget data"); \
			return_code=0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"TEXT_CHOOSE_OBJECT_CHANGE_CONDITIONAL_FUNCTION(" #object_type \
			").  Missing widget"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* TEXT_CHOOSE_OBJECT_CHANGE_CONDITIONAL_FUNCTION(object_type) */

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
DECLARE_TEXT_CHOOSE_OBJECT_GET_OBJECT_FUNCTION(object_type) \
DECLARE_TEXT_CHOOSE_OBJECT_CHANGE_CONDITIONAL_FUNCTION_FUNCTION(object_type)

#endif /* !defined (TEXT_CHOOSE_OBJECT_PRIVATE_H) */
