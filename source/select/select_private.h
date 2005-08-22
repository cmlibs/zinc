/*******************************************************************************
FILE : select_private.h

LAST MODIFIED : 26 November 2001

DESCRIPTION :
Creates a scrolled list of objects based upon their name.  Allows the user
to add, delete and rename the objects.  Interfaces with the global manager
for each type it supports.
New version using macros to handle different object types.
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
#if !defined (SELECT_PRIVATE_H)
#define SELECT_PRIVATE_H

#include <stddef.h>
#include <math.h>
#include <Mrm/MrmPublic.h>
#include <X11/Xlib.h>
#include <Xm/Xm.h>
#include <Xm/ScrollBar.h>
#include <Xm/TextF.h>
#include "general/debug.h"
#include "select/select.h"
#if !defined (NO_SELECT_UIDH)
#include "select/select.uidh"
#endif /* !defined (NO_SELECT_UIDH) */
#include "user_interface/gui_dialog_macros.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module types
------------
*/
#if ! defined (SHORT_NAMES)
#define SELECT_STRUCT_( object_type )  select_struct_ ## object_type
#else
#define SELECT_STRUCT_( object_type )  sstru ## object_type
#endif
#define SELECT_STRUCT( object_type )  SELECT_STRUCT_(object_type)

/* Must call this macro with object_type before using any select functions */
#define FULL_DECLARE_SELECT_STRUCT_TYPE( object_type ) \
struct SELECT_STRUCT(object_type) \
/***************************************************************************** \
LAST MODIFIED : 6 May 1997 \
\
DESCRIPTION : \
Contains information required by the select widget of the given object_type. \
============================================================================*/ \
{ \
	int delete_position; \
	enum Select_appearance appearance; \
	struct Callback_data update_callback; \
	struct object_type *current_object,*last_updated_object; \
	struct MANAGER(object_type) *object_manager; \
	void *manager_callback_id; \
	Widget create_btn,delete_btn,edit_label,edit_name,edit_text,list_rowcol, \
		list_scroll,main_form,rename_btn,text_label,text_text; \
	Widget widget_parent,widget,*widget_address; \
} /* struct SELECT_STRUCT(object_type) */

/*
Module variables
----------------
*/
#if defined (MOTIF)
extern int select_hierarchy_open;
extern MrmHierarchy select_hierarchy;
#endif /* defined (MOTIF) */

/*
Module functions
----------------
*/
#if ! defined (SHORT_NAMES)
#define SELECT_REGISTER_UIL_NAMES_( object_type ) \
	select_register_UIL_names_ ## object_type
#else
#define SELECT_REGISTER_UIL_NAMES_( object_type )  srun ## object_type
#endif
#define SELECT_REGISTER_UIL_NAMES( object_type ) \
	SELECT_REGISTER_UIL_NAMES_(object_type)

#define PROTOTYPE_SELECT_REGISTER_UIL_NAMES_FUNCTION( object_type ) \
static int SELECT_REGISTER_UIL_NAMES(object_type)( \
	struct SELECT_STRUCT(object_type) *temp_select) \
/***************************************************************************** \
LAST MODIFIED : 13 May 1997 \
\
DESCRIPTION : \
(re)registers UIL names for the select widget of the given object type. \
Also ensures the select_structure has the correct address. \
============================================================================*/

#if ! defined (SHORT_NAMES)
#define SELECT_MANAGER_MODIFY_IDENTIFIER_AS_NAME_( object_type ) \
	select_manager_modify_identifier_as_name_ ## object_type
#else
#define SELECT_MANAGER_MODIFY_IDENTIFIER_AS_NAME_( object_type ) \
	smmian ## object_type
#endif
#define SELECT_MANAGER_MODIFY_IDENTIFIER_AS_NAME( object_type ) \
	SELECT_MANAGER_MODIFY_IDENTIFIER_AS_NAME_(object_type)

#define PROTOTYPE_SELECT_MANAGER_MODIFY_IDENTIFIER_AS_NAME_FUNCTION( \
	object_type ) \
static int SELECT_MANAGER_MODIFY_IDENTIFIER_AS_NAME(object_type)( \
	struct object_type *object,char *object_name, \
	struct MANAGER(object_type) *object_manager) \
/***************************************************************************** \
LAST MODIFIED : 9 May 1997 \
\
DESCRIPTION : \
Macro interface for MANAGER_MODIFY_IDENTIFIER(object_type,identifier) \
Default version assumes the object has a name member and simply substitutes \
the real MANAGER function. \
Objects with say, an integer identifier must convert the string first. \
============================================================================*/

#define DECLARE_DEFAULT_SELECT_MANAGER_MODIFY_IDENTIFIER_AS_NAME_FUNCTION( \
	object_type ) \
PROTOTYPE_SELECT_MANAGER_MODIFY_IDENTIFIER_AS_NAME_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 9 May 1997 \
\
DESCRIPTION : \
Assumes the object has a "name" members so simply substitutes \
MANAGER_MODIFY_IDENTIFIER function. \
???RC Should this really know about object->name? \
============================================================================*/ \
{ \
	int return_code; \
\
	ENTER(SELECT_MANAGER_MODIFY_IDENTIFIER_AS_NAME(object_type)); \
	return_code=MANAGER_MODIFY_IDENTIFIER(object_type,name)( \
		object,object_name,object_manager); \
	LEAVE; \
\
	return return_code; \
} /* SELECT_MANAGER_MODIFY_IDENTIFIER_AS_NAME(object_type) */

#if ! defined (SHORT_NAMES)
#define SELECT_FIND_BY_IDENTIFIER_AS_NAME_IN_MANAGER_( object_type ) \
	select_find_by_identifier_as_name_in_manager_ ## object_type
#else
#define SELECT_FIND_BY_IDENTIFIER_AS_NAME_IN_MANAGER_( object_type ) \
	sfbianim ## object_type
#endif
#define SELECT_FIND_BY_IDENTIFIER_AS_NAME_IN_MANAGER( object_type ) \
	SELECT_FIND_BY_IDENTIFIER_AS_NAME_IN_MANAGER_(object_type)

#define PROTOTYPE_SELECT_FIND_BY_IDENTIFIER_AS_NAME_IN_MANAGER_FUNCTION( \
	object_type ) \
static struct object_type *SELECT_FIND_BY_IDENTIFIER_AS_NAME_IN_MANAGER( \
	object_type)(char *object_name, struct MANAGER(object_type) *object_manager) \
/***************************************************************************** \
LAST MODIFIED : 9 May 1997 \
\
DESCRIPTION : \
Macro interface for FIND_BY_IDENTIFIER_IN_MANAGER(object_type,identifier) \
Default version assumes the object has a name member and simply substitutes \
the real MANAGER function. \
Objects with say, an integer identifier must convert the string first. \
============================================================================*/

#define DECLARE_DEFAULT_SELECT_FIND_BY_IDENTIFIER_AS_NAME_IN_MANAGER_FUNCTION( \
object_type ) \
PROTOTYPE_SELECT_FIND_BY_IDENTIFIER_AS_NAME_IN_MANAGER_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 9 May 1997 \
\
DESCRIPTION : \
Macro interface for FIND_BY_IDENTIFIER_IN_MANAGER(object_type,identifier) \
Assumes the object has a "name" members so simply substitutes \
FIND_BY_IDENTIFIER_IN_MANAGER function. \
???RC Should this really know about object->name? \
============================================================================*/ \
{ \
	struct object_type *return_object_ptr; \
\
	ENTER(SELECT_FIND_BY_IDENTIFIER_AS_NAME_IN_MANAGER_(object_type)); \
	return_object_ptr=FIND_BY_IDENTIFIER_IN_MANAGER(object_type,name)( \
		object_name,object_manager); \
	LEAVE; \
\
	return return_object_ptr; \
} /* SELECT_FIND_BY_IDENTIFIER_AS_NAME_IN_MANAGER_(object_type) */

#if ! defined (SHORT_NAMES)
#define SELECT_MANAGER_CREATE_( object_type ) \
	select_manager_create_ ## object_type
#else
#define SELECT_MANAGER_CREATE_( object_type )  smc ## object_type
#endif
#define SELECT_MANAGER_CREATE( object_type ) \
	SELECT_MANAGER_CREATE_(object_type)

#define PROTOTYPE_SELECT_MANAGER_CREATE_FUNCTION( object_type ) \
static struct object_type *SELECT_MANAGER_CREATE(object_type)( \
	struct object_type *template_object, \
	struct MANAGER(object_type) *object_manager) \
/***************************************************************************** \
LAST MODIFIED : 20 April 2000 \
\
DESCRIPTION : \
Creates a new struct object_type with a unique identifier in <object_manager>. \
It <template_object> is supplied, the new object will be a copy of it and its \
identifier may be derived from it. \
???RC Should be part of manager.h \
============================================================================*/

#define DECLARE_DEFAULT_SELECT_MANAGER_CREATE_FUNCTION( object_type ) \
	PROTOTYPE_SELECT_MANAGER_CREATE_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 20 April 2000 \
\
DESCRIPTION : \
Manager copy creator assuming it has a name identifier. \
============================================================================*/ \
{ \
	char *new_object_name,*temp_name; \
	static char default_name[]="new"; \
	struct object_type *new_object; \
\
	ENTER(SELECT_MANAGER_CREATE(object_type)); \
	if (object_manager) \
	{ \
		/* get a unique name for the new object */ \
		if (!(template_object&& \
			GET_NAME(object_type)(template_object,&new_object_name))) \
		{ \
			if (ALLOCATE(new_object_name,char,strlen(default_name)+1)) \
			{ \
				strcpy(new_object_name,default_name); \
			} \
			else \
			{ \
				display_message(ERROR_MESSAGE,"SELECT_MANAGER_CREATE(" #object_type \
					").  Could not allocate space for new name"); \
			} \
		} \
		/* Ensure the new identifier is not used by some object in the manager */ \
		/* Keep on appending "+" onto the name until it is unique: */ \
		while ((new_object_name)&&(FIND_BY_IDENTIFIER_IN_MANAGER(object_type,name) \
			(new_object_name,object_manager))) \
		{ \
			if (REALLOCATE(temp_name,new_object_name,char, \
				strlen(new_object_name)+2)) \
			{ \
				new_object_name=temp_name; \
				strcat(new_object_name,"+"); \
			} \
			else \
			{ \
				DEALLOCATE(new_object_name);  \
				display_message(ERROR_MESSAGE,"SELECT_MANAGER_CREATE(" #object_type \
					").  Could not give object a unique name"); \
			} \
		} \
		if (new_object_name) \
		{ \
			new_object=CREATE(object_type)(new_object_name); \
			/* copy template_object contents into new object */ \
			if (template_object) \
			{ \
				MANAGER_COPY_WITHOUT_IDENTIFIER(object_type,name)( \
					new_object,template_object); \
			} \
			DEALLOCATE(new_object_name); \
		} \
		else \
		{ \
			new_object=(struct object_type *)NULL; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE,"SELECT_MANAGER_CREATE(" #object_type \
			").  Missing object_manager"); \
		new_object=(struct object_type *)NULL; \
	} \
	LEAVE; \
\
	return (new_object); \
} /* SELECT_MANAGER_CREATE(object_type) */

#if ! defined (SHORT_NAMES)
#define SELECT_UPDATE_( object_type )  select_update_ ## object_type
#else
#define SELECT_UPDATE_( object_type )  selud ## object_type
#endif
#define SELECT_UPDATE( object_type )  SELECT_UPDATE_(object_type)

#define DECLARE_SELECT_UPDATE_FUNCTION( object_type ) \
static void SELECT_UPDATE(object_type)( \
	struct SELECT_STRUCT(object_type) *temp_select) \
/***************************************************************************** \
LAST MODIFIED : 5 December 2000 \
\
DESCRIPTION : \
Tells CMGUI about the current values. Returns a pointer to the currently \
selected object. \
============================================================================*/ \
{ \
	ENTER(SELECT_UPDATE(object_type)); \
	if (temp_select) \
	{ \
		if (temp_select->current_object != temp_select->last_updated_object) \
		{ \
			if (temp_select->update_callback.procedure) \
			{ \
				(temp_select->update_callback.procedure)(temp_select->widget, \
					temp_select->update_callback.data,temp_select->current_object); \
			} \
			temp_select->last_updated_object=temp_select->current_object; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"SELECT_UPDATE(" #object_type ").  Invalid argument(s)"); \
	} \
	LEAVE; \
} /* SELECT_UPDATE(object_type) */

#if ! defined (SHORT_NAMES)
#define CREATE_SUB_WIDGET_( object_type )  create_sub_widget_ ## object_type
#else
#define CREATE_SUB_WIDGET_( object_type )  cresw ## object_type
#endif
#define CREATE_SUB_WIDGET( object_type )  CREATE_SUB_WIDGET_(object_type)

#define DECLARE_CREATE_SUB_WIDGET_FUNCTION( object_type ) \
static int CREATE_SUB_WIDGET(object_type)( \
	struct object_type *object,void *args) \
/***************************************************************************** \
LAST MODIFIED : 24 October 1997 \
\
DESCRIPTION : \
Is called for each object, and adds a subwidget to the rowcol. \
============================================================================*/ \
{ \
	Arg override_arg; \
	char *object_name; \
	int return_code; \
	MrmType select_item_class; \
	struct SELECT_STRUCT(object_type) *temp_select; \
	Widget temp_widget; \
	XmString new_string; \
\
	ENTER(CREATE_SUB_WIDGET(object_type)); \
	temp_select=(struct SELECT_STRUCT(object_type) *)args; \
	if (select_hierarchy_open) \
	{ \
		XtSetArg(override_arg,XmNuserData,object); \
		temp_widget=(Widget)NULL; \
		if (MrmSUCCESS==MrmFetchWidgetOverride(select_hierarchy,"select_item", \
			temp_select->list_rowcol,NULL,&override_arg,1,&temp_widget, \
			&select_item_class)) \
		{ \
			XtManageChild(temp_widget); \
			if (return_code=GET_NAME(object_type)(object,&object_name)) \
			{ \
				new_string=XmStringCreateSimple(object_name); \
				DEALLOCATE(object_name); \
				XtVaSetValues(temp_widget,XmNlabelString,new_string,NULL); \
				XmStringFree(new_string); \
				return_code=1; \
			} \
		} \
		else \
		{ \
			display_message(WARNING_MESSAGE, \
				"CREATE_SUB_WIDGET(" #object_type \
				").  Could not fetch submenu widget"); \
			return_code=0; \
		} \
	} \
	else \
	{ \
		display_message(WARNING_MESSAGE, \
			"CREATE_SUB_WIDGET(" #object_type ").  Hierarchy not open"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* CREATE_SUB_WIDGET(object_type) */

#if ! defined (SHORT_NAMES)
#define SELECT_UPDATE_LIST_( object_type )  select_update_list_ ## object_type
#else
#define SELECT_UPDATE_LIST_( object_type )  sul ## object_type
#endif
#define SELECT_UPDATE_LIST( object_type )  SELECT_UPDATE_LIST_(object_type)

#define DECLARE_SELECT_UPDATE_LIST_FUNCTION( object_type ) \
static void SELECT_UPDATE_LIST(object_type)( \
	struct SELECT_STRUCT(object_type) *temp_select) \
/***************************************************************************** \
LAST MODIFIED : 7 May 1997 \
\
DESCRIPTION : \
We know that the list of objects has changed, so delete all current widgets \
in the scrolled list, and create the new widgets. \
============================================================================*/ \
{ \
	Arg override_arg; \
	MrmType select_rowcol_class; \
	Widget temp_widget; \
\
	ENTER(SELECT_UPDATE_LIST(object_type)); \
	if (select_hierarchy_open) \
	{ \
		/* delete all widgets */ \
		XtDestroyWidget(temp_select->list_rowcol); \
		/* now create the new ones */ \
		/* create the rowcol */ \
		/* Must reregister UIL names before fetching rowcol */ \
		SELECT_REGISTER_UIL_NAMES(object_type)(temp_select); \
		/* no need to override args now: */ \
		XtSetArg(override_arg,XmNuserData,temp_select); \
		temp_widget=(Widget)NULL; \
		if (MrmSUCCESS==MrmFetchWidgetOverride(select_hierarchy, \
			"select_list_rowcol",temp_select->list_scroll,NULL,&override_arg,1, \
			&temp_widget,&select_rowcol_class)) \
		{ \
			XtManageChild(temp_widget); \
		} \
		else \
		{ \
			display_message(WARNING_MESSAGE, \
				"SELECT_UPDATE_LIST(" #object_type \
				").  Could not fetch submenu widget"); \
		} \
		/* create the sub widgets */ \
		FOR_EACH_OBJECT_IN_MANAGER(object_type)(CREATE_SUB_WIDGET(object_type), \
			temp_select,temp_select->object_manager); \
	} \
	else \
	{ \
		display_message(WARNING_MESSAGE, \
			"SELECT_UPDATE_LIST(" #object_type ").  Hierarchy not open"); \
	} \
	LEAVE; \
} /* SELECT_UPDATE_LIST(object_type) */

#if ! defined (SHORT_NAMES)
#define SELECT_DIALOG_NAME_( object_type ) \
	select_ ## object_type
#else
#define SELECT_DIALOG_NAME_( object_type ) sdn_ ## object_type
#endif
#define SELECT_DIALOG_NAME( object_type ) \
	SELECT_DIALOG_NAME_( object_type )

#define DECLARE_SELECT_IDENTIFY_FUNCTIONS( object_type ) \
DECLARE_DIALOG_IDENTIFY_FUNCTION(SELECT_DIALOG_NAME(object_type), \
	SELECT_STRUCT(object_type),main_form) \
DECLARE_DIALOG_IDENTIFY_FUNCTION(SELECT_DIALOG_NAME(object_type), \
	SELECT_STRUCT(object_type),edit_name) \
DECLARE_DIALOG_IDENTIFY_FUNCTION(SELECT_DIALOG_NAME(object_type), \
	SELECT_STRUCT(object_type),edit_label) \
DECLARE_DIALOG_IDENTIFY_FUNCTION(SELECT_DIALOG_NAME(object_type), \
	SELECT_STRUCT(object_type),edit_text) \
DECLARE_DIALOG_IDENTIFY_FUNCTION(SELECT_DIALOG_NAME(object_type), \
	SELECT_STRUCT(object_type),create_btn) \
DECLARE_DIALOG_IDENTIFY_FUNCTION(SELECT_DIALOG_NAME(object_type), \
	SELECT_STRUCT(object_type),delete_btn) \
DECLARE_DIALOG_IDENTIFY_FUNCTION(SELECT_DIALOG_NAME(object_type), \
	SELECT_STRUCT(object_type),rename_btn) \
DECLARE_DIALOG_IDENTIFY_FUNCTION(SELECT_DIALOG_NAME(object_type), \
	SELECT_STRUCT(object_type),text_label) \
DECLARE_DIALOG_IDENTIFY_FUNCTION(SELECT_DIALOG_NAME(object_type), \
	SELECT_STRUCT(object_type),text_text) \
DECLARE_DIALOG_IDENTIFY_FUNCTION(SELECT_DIALOG_NAME(object_type), \
	SELECT_STRUCT(object_type),list_scroll) \
DECLARE_DIALOG_IDENTIFY_FUNCTION(SELECT_DIALOG_NAME(object_type), \
	SELECT_STRUCT(object_type),list_rowcol)

#if ! defined (SHORT_NAMES)
#define SELECT_DESTROY_CB_( object_type )  select_destroy_cb_ ## object_type
#else
#define SELECT_DESTROY_CB_( object_type )  sdcb ## object_type
#endif
#define SELECT_DESTROY_CB( object_type )  SELECT_DESTROY_CB_(object_type)

#define DECLARE_SELECT_DESTROY_CB_FUNCTION( object_type ) \
static void SELECT_DESTROY_CB(object_type)(Widget w,int *tag, \
	unsigned long *reason) \
/***************************************************************************** \
LAST MODIFIED : 7 May 1997 \
\
DESCRIPTION : \
Callback for the select dialog - tidies up all memory allocation. \
============================================================================*/ \
{ \
	struct SELECT_STRUCT(object_type) *temp_select; \
\
	ENTER(SELECT_DESTROY_CB(object_type)); \
	USE_PARAMETER(tag); \
	USE_PARAMETER(reason); \
	/* get the pointer to the data for the select dialog */ \
	XtVaGetValues(w,XmNuserData,&temp_select,NULL); \
	*(temp_select->widget_address)=(Widget)NULL; \
	MANAGER_DEREGISTER(object_type)(temp_select->manager_callback_id, \
		temp_select->object_manager); \
	/* deallocate the memory for the user data */ \
	DEALLOCATE(temp_select); \
	LEAVE; \
} /* SELECT_DESTROY_CB(object_type) */

#if ! defined (SHORT_NAMES)
#define SELECT_SELECT_OBJECT_( object_type ) \
	select_select_object_ ## object_type
#else
#define SELECT_SELECT_OBJECT_( object_type )  sso ## object_type
#endif
#define SELECT_SELECT_OBJECT( object_type )  SELECT_SELECT_OBJECT_(object_type)

#define DECLARE_SELECT_SELECT_OBJECT_FUNCTION( object_type ) \
static void SELECT_SELECT_OBJECT(object_type)( \
	struct SELECT_STRUCT(object_type) *temp_select) \
/***************************************************************************** \
LAST MODIFIED : 5 December 2000 \
\
DESCRIPTION : \
Finds the current object in the list, then selects it. \
If selected object changes in this routine the client is informed. \
============================================================================*/ \
{ \
	char *object_name; \
	int bottom,i,increment,num_children,num_visible,p_increment,scroll_position, \
		selected,size,top; \
	struct object_type *temp_object; \
	Widget *child_list,v_scrollbar; \
\
	ENTER(SELECT_SELECT_OBJECT(object_type)); \
	/* check arguments */ \
	if (temp_select&&temp_select->object_manager) \
	{ \
		switch (temp_select->appearance) \
		{ \
			case SELECT_LIST: \
			{ \
				selected= -1; \
				XtVaGetValues(temp_select->list_rowcol,XmNnumChildren,&num_children, \
					XmNchildren,&child_list,NULL); \
				if (0<num_children) \
				{ \
					for (i=0;i<num_children;i++) \
					{ \
						XtVaGetValues(child_list[i],XmNuserData,&temp_object,NULL); \
						XtVaSetValues(child_list[i],XmNset,FALSE,NULL); \
						if (temp_object==temp_select->current_object) \
						{ \
							selected=i; \
						} \
					} \
					/* if no object selected, take one from list */ \
					if (0 > selected) \
					{ \
						/* if just deleted an object, want to select next one after it */ \
						selected=temp_select->delete_position; \
						if (num_children <= selected) \
						{ \
							selected=num_children-1; \
						} \
						XtVaGetValues(child_list[selected],XmNuserData,&temp_object,NULL); \
						temp_select->current_object=temp_object; \
					} \
					XtVaSetValues(child_list[selected],XmNset,TRUE,NULL); \
				} \
				else \
				{ \
					temp_select->current_object=(struct object_type *)NULL; \
				} \
				temp_select->delete_position=0; \
				if (selected>=0) \
				{ \
					XtVaGetValues(temp_select->list_scroll,XmNverticalScrollBar, \
						&v_scrollbar,NULL); \
					/* set scrolled window so that the selected item is at the top */ \
					if (v_scrollbar) \
					{ \
						XtVaGetValues(v_scrollbar,XmNmaximum,&top,XmNminimum,&bottom, \
							XmNsliderSize,&size,XmNincrement,&increment,XmNpageIncrement, \
							&p_increment,NULL); \
						/* num_visible/total=scrollbar_size/tot_size */ \
						num_visible=(num_children*size)/(top-bottom); \
						if (num_children==num_visible) \
						{ \
							scroll_position=0; \
							/* do not update the scrollbar */ \
						} \
						else \
						{ \
							scroll_position=((top-bottom-size)*selected)/ \
								(num_children-num_visible); \
							/* do some checking so we dont generate an error */ \
							if (scroll_position<0) \
							{ \
								scroll_position=0; \
							} \
							if (scroll_position>(top-bottom-size)) \
							{ \
								scroll_position=top-bottom-size; \
							} \
							XmScrollBarSetValues(v_scrollbar,scroll_position,size,increment, \
								p_increment,TRUE); \
						} \
					} \
				} \
			} break; \
			case SELECT_TEXT: \
			{ \
				if (!temp_select->current_object) \
				{ \
					/* select first object in manager as current object */ \
					temp_select->current_object = \
						FIRST_OBJECT_IN_MANAGER_THAT(object_type)( \
							(MANAGER_CONDITIONAL_FUNCTION(object_type) *)NULL, \
							(void *)NULL, temp_select->object_manager); \
				} \
				if (temp_select->current_object) \
				{ \
					if (GET_NAME(object_type)(temp_select->current_object,&object_name)) \
					{ \
						XmTextFieldSetString(temp_select->text_text,object_name); \
						DEALLOCATE(object_name); \
					} \
				} \
				else \
				{ \
					/* show default string for NULL object */ \
					XmTextFieldSetString(temp_select->text_text,"<none>"); \
				} \
			} break; \
			default: \
			{ \
				display_message(ERROR_MESSAGE, \
					"select_select_object.  Invalid appearance"); \
			} break; \
		} \
		/* inform the client of the change */ \
		SELECT_UPDATE(object_type)(temp_select); \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"SELECT_SELECT_OBJECT(" #object_type ").  Invalid argument(s)"); \
	} \
	LEAVE; \
} /* SELECT_SELECT_OBJECT(object_type) */

#if ! defined (SHORT_NAMES)
#define SELECT_CREATE_CB_( object_type )  select_create_CB_ ## object_type
#else
#define SELECT_CREATE_CB_( object_type )  scbcb ## object_type
#endif
#define SELECT_CREATE_CB( object_type )  SELECT_CREATE_CB_(object_type)

#define DECLARE_SELECT_CREATE_CB_FUNCTION( object_type ) \
static void SELECT_CREATE_CB(object_type)(Widget w,int *tag, \
	XmAnyCallbackStruct *reason) \
/***************************************************************************** \
LAST MODIFIED : 20 April 2000 \
\
DESCRIPTION : \
Creates a new object with a unique identifier and adds it to the manager. \
============================================================================*/ \
{ \
	struct object_type *new_object; \
	struct SELECT_STRUCT(object_type) *temp_select; \
\
	ENTER(SELECT_CREATE_CB(object_type)); \
	USE_PARAMETER(tag); \
	USE_PARAMETER(reason); \
	XtVaGetValues(w,XmNuserData,&temp_select,NULL); \
	/* this means they do not want to keep any naming changes */ \
	XtUnmanageChild(temp_select->edit_name); \
	if (new_object=SELECT_MANAGER_CREATE(object_type)( \
		temp_select->current_object,temp_select->object_manager)) \
	{ \
		if (ADD_OBJECT_TO_MANAGER(object_type)( \
			new_object,temp_select->object_manager)) \
		{ \
			temp_select->current_object=new_object; \
			switch (temp_select->appearance) \
			{ \
				case SELECT_LIST: \
				{ \
					/* display the correct names */ \
					SELECT_UPDATE_LIST(object_type)(temp_select); \
				} break; \
				case SELECT_TEXT: \
				{ \
					/* do nothing */ \
				} break; \
				default: \
				{ \
					display_message(ERROR_MESSAGE, \
						"SELECT_CREATE_CB(" #object_type ").  Invalid appearance"); \
				} break; \
			} \
			SELECT_SELECT_OBJECT(object_type)(temp_select); \
			SELECT_UPDATE(object_type)(temp_select); \
		} \
		else \
		{ \
			DESTROY(object_type)(&new_object); \
			display_message(ERROR_MESSAGE, \
				"SELECT_CREATE_CB(" #object_type \
				").  Could not add new " #object_type); \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"SELECT_CREATE_CB(" #object_type \
			").  Could not create new " #object_type); \
	} \
	LEAVE; \
} /* SELECT_CREATE_CB(object_type) */

#if ! defined (SHORT_NAMES)
#define SELECT_DELETE_CB_( object_type )  select_delete_CB_ ## object_type
#else
#define SELECT_DELETE_CB_( object_type )  sdbcb ## object_type
#endif
#define SELECT_DELETE_CB( object_type )  SELECT_DELETE_CB_(object_type)

#define DECLARE_SELECT_DELETE_CB_FUNCTION( object_type ) \
static void SELECT_DELETE_CB(object_type)(Widget w,int *tag, \
	XmAnyCallbackStruct *reason) \
/***************************************************************************** \
LAST MODIFIED : 8 May 1997 \
\
DESCRIPTION : \
Deletes the currently selected object. \
============================================================================*/ \
{ \
	struct SELECT_STRUCT(object_type) *temp_select; \
\
	ENTER(SELECT_DELETE_CB(object_type)); \
	USE_PARAMETER(tag); \
	USE_PARAMETER(reason); \
	XtVaGetValues(w,XmNuserData,&temp_select,NULL); \
	/* this means they dont want to keep any naming changes */ \
	XtUnmanageChild(temp_select->edit_name); \
	if (temp_select->current_object) \
	{ \
		if (!REMOVE_OBJECT_FROM_MANAGER(object_type)( \
			temp_select->current_object,temp_select->object_manager)) \
		{ \
			display_message(ERROR_MESSAGE, \
				"SELECT_DELETE_CB(" #object_type \
				").  Could not delete " #object_type); \
		} \
	} \
	LEAVE; \
} /* SELECT_DELETE_CB(object_type) */

#if ! defined (SHORT_NAMES)
#define SELECT_RENAME_CB_( object_type )  select_rename_CB_ ## object_type
#else
#define SELECT_RENAME_CB_( object_type )  srbcb ## object_type
#endif
#define SELECT_RENAME_CB( object_type )  SELECT_RENAME_CB_(object_type)

#define DECLARE_SELECT_RENAME_CB_FUNCTION( object_type ) \
static void SELECT_RENAME_CB(object_type)(Widget w,int *tag, \
	XmAnyCallbackStruct *reason) \
/***************************************************************************** \
LAST MODIFIED : 8 May 1997 \
\
DESCRIPTION : \
Rename button pressed. Turns on and sets the rename text field. \
============================================================================*/ \
{ \
	char *object_name; \
	struct SELECT_STRUCT(object_type) *temp_select; \
\
	ENTER(SELECT_RENAME_CB(object_type)); \
	USE_PARAMETER(tag); \
	USE_PARAMETER(reason); \
	XtVaGetValues(w,XmNuserData,&temp_select,NULL); \
	if (temp_select->current_object) \
	{ \
		/* Copy current object name into rename text field: */ \
		if (GET_NAME(object_type)(temp_select->current_object,&object_name)) \
		{ \
			XtVaSetValues(temp_select->edit_text,XmNvalue,object_name,NULL); \
			DEALLOCATE(object_name); \
			XtManageChild(temp_select->edit_name); \
		} \
	} \
	else \
	{ \
		XtUnmanageChild(temp_select->edit_name); \
	} \
	LEAVE; \
} /* SELECT_RENAME_CB(object_type) */

#if ! defined (SHORT_NAMES)
#define SELECT_NAME_CB_( object_type )  select_name_CB_ ## object_type
#else
#define SELECT_NAME_CB_( object_type )  sncb ## object_type
#endif
#define SELECT_NAME_CB( object_type )  SELECT_NAME_CB_(object_type)

#define DECLARE_SELECT_NAME_CB_FUNCTION( object_type ) \
static void SELECT_NAME_CB(object_type)(Widget w,int *tag, \
	XmAnyCallbackStruct *reason) \
/***************************************************************************** \
LAST MODIFIED : 20 July 2000 \
\
DESCRIPTION : \
Takes the name from the text field widget, and makes it the name of the \
current object. \
============================================================================*/ \
{ \
	char *new_object_name; \
	struct SELECT_STRUCT(object_type) *temp_select; \
\
	ENTER(SELECT_NAME_CB(object_type)); \
	USE_PARAMETER(tag); \
	USE_PARAMETER(reason); \
	XtVaGetValues(w,XmNuserData,&temp_select,XmNvalue,&new_object_name,NULL); \
	if (temp_select->current_object) \
	{ \
		SELECT_MANAGER_MODIFY_IDENTIFIER_AS_NAME(object_type)( \
			temp_select->current_object,new_object_name, \
			temp_select->object_manager); \
		/* remove rename text widget (whether or not it is there): */ \
		XtUnmanageChild(temp_select->edit_name); \
	} \
	XtFree(new_object_name); \
	LEAVE; \
} /* SELECT_NAME_CB(object_type) */

#if ! defined (SHORT_NAMES)
#define SELECT_SELECT_TEXT_CB_( object_type ) \
	select_select_text_CB_ ## object_type
#else
#define SELECT_SELECT_TEXT_CB_( object_type )  sstcb ## object_type
#endif
#define SELECT_SELECT_TEXT_CB( object_type ) \
	SELECT_SELECT_TEXT_CB_(object_type)

#define DECLARE_SELECT_SELECT_TEXT_CB_FUNCTION( object_type ) \
static void SELECT_SELECT_TEXT_CB(object_type)(Widget w,int *tag, \
	XmAnyCallbackStruct *reason) \
/***************************************************************************** \
LAST MODIFIED : 1 October 1997 \
\
DESCRIPTION : \
Takes the name from the text field widget, and searches for the corresponding \
object in the manager, then selects it. \
Should see if selected object changes - if not, do not re-select it \
============================================================================*/ \
{ \
	char *entered_name; \
	struct object_type *object_ptr; \
	struct SELECT_STRUCT(object_type) *temp_select; \
\
	ENTER(SELECT_SELECT_TEXT_CB(object_type)); \
	USE_PARAMETER(tag); \
	USE_PARAMETER(reason); \
	XtVaGetValues(w,XmNuserData,&temp_select,XmNvalue,&entered_name,NULL); \
	object_ptr=SELECT_FIND_BY_IDENTIFIER_AS_NAME_IN_MANAGER(object_type)( \
		entered_name,temp_select->object_manager); \
	if (object_ptr) \
	{ \
		temp_select->current_object=object_ptr; \
	} \
	SELECT_SELECT_OBJECT(object_type)(temp_select); \
	XtFree(entered_name); \
	LEAVE; \
} /* SELECT_SELECT_TEXT_CB(object_type) */

#if ! defined (SHORT_NAMES)
#define SELECT_FOCUS_CB_( object_type )  select_focus_CB_ ## object_type
#else
#define SELECT_FOCUS_CB_( object_type )  sfcb ## object_type
#endif
#define SELECT_FOCUS_CB( object_type )  SELECT_FOCUS_CB_(object_type)

#define DECLARE_SELECT_FOCUS_CB_FUNCTION( object_type ) \
static void SELECT_FOCUS_CB(object_type)(Widget w,int *tag, \
	XmToggleButtonCallbackStruct *reason) \
/***************************************************************************** \
LAST MODIFIED : 1 October 1997 \
\
DESCRIPTION : \
An item in the list has been selected, so set the current_value to that item, \
and then do an update. \
============================================================================*/ \
{ \
	struct SELECT_STRUCT(object_type) *temp_select; \
	struct object_type *selected_object; \
\
	ENTER(SELECT_FOCUS_CB(object_type)); \
	USE_PARAMETER(tag); \
	/* get SELECT_STRUCT from parent: */ \
	XtVaGetValues(XtParent(w),XmNuserData,&temp_select,NULL); \
	/* remove rename text widget (whether or not it is there): */ \
	XtUnmanageChild(temp_select->edit_name); \
	if (reason->set) \
	{ \
		XtVaGetValues(w,XmNuserData,&selected_object,NULL); \
		temp_select->current_object=selected_object; \
		SELECT_UPDATE(object_type)(temp_select); \
	} \
	LEAVE; \
} /* SELECT_FOCUS_CB(object_type) */

#if ! defined (SHORT_NAMES)
#define SELECT_GLOBAL_OBJECT_CHANGE_( object_type ) \
	select_global_object_change_ ## object_type
#else
#define SELECT_GLOBAL_OBJECT_CHANGE_( object_type )  sgoc ## object_type
#endif
#define SELECT_GLOBAL_OBJECT_CHANGE( object_type ) \
	SELECT_GLOBAL_OBJECT_CHANGE_(object_type)

#define DECLARE_SELECT_GLOBAL_OBJECT_CHANGE_FUNCTION( object_type ) \
static void SELECT_GLOBAL_OBJECT_CHANGE(object_type)( \
	struct MANAGER_MESSAGE(object_type) *message,void *data) \
/***************************************************************************** \
LAST MODIFIED : 5 December 2000 \
\
DESCRIPTION : \
Something has changed globally about the objects this widget uses, \
so may need to select a new object. \
============================================================================*/ \
{ \
	struct SELECT_STRUCT(object_type) *temp_select; \
	int num_children,i; \
	struct object_type *temp_object; \
	Widget *child_list; \
\
	ENTER(SELECT_GLOBAL_OBJECT_CHANGE(object_type)); \
	temp_select=(struct SELECT_STRUCT(object_type) *)data; \
	if (message->change != MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(object_type)) \
	{ \
		XtUnmanageChild(temp_select->edit_name); \
		switch (message->change) \
		{ \
			case MANAGER_CHANGE_REMOVE(object_type): \
			{ \
				switch (temp_select->appearance) \
				{ \
					case SELECT_LIST: \
					{ \
						if (IS_OBJECT_IN_LIST(object_type)(temp_select->current_object, \
							message->changed_object_list)) \
						{ \
							/* find positionn of deleted item so next can be selected */ \
							XtVaGetValues(temp_select->list_rowcol, \
								XmNnumChildren,&num_children,XmNchildren,&child_list,NULL); \
							for (i=0;i<num_children;i++) \
							{ \
								XtVaGetValues(child_list[i],XmNuserData,&temp_object,NULL); \
								if (temp_object==temp_select->current_object) \
								{ \
									temp_select->delete_position=i; \
								} \
							} \
						} \
						/* list/names of objects may have changed: rebuild */ \
						SELECT_UPDATE_LIST(object_type)(temp_select); \
					} break; \
					case SELECT_TEXT: \
					{ \
						if (IS_OBJECT_IN_LIST(object_type)(temp_select->current_object, \
							message->changed_object_list)) \
						{ \
							temp_select->current_object = (struct object_type *)NULL; \
						} \
					} break; \
					default: \
					{ \
						display_message(ERROR_MESSAGE, \
							"SELECT_GLOBAL_OBJECT_CHANGE(" #object_type \
							").  Invalid appearance"); \
					} break; \
				} \
				/* if current_object not in menu SELECT_SELECT_OBJECT */ \
				/* will select the first item in it, and call an update */ \
				SELECT_SELECT_OBJECT(object_type)(temp_select); \
			} break; \
			case MANAGER_CHANGE_ADD(object_type): \
			case MANAGER_CHANGE_IDENTIFIER(object_type): \
			case MANAGER_CHANGE_OBJECT(object_type): \
			{ \
				switch (temp_select->appearance) \
				{ \
					case SELECT_LIST: \
					{ \
						/* list/names of objects may have changed: rebuild */ \
						SELECT_UPDATE_LIST(object_type)(temp_select); \
					} break; \
					case SELECT_TEXT: \
					{ \
					} break; \
					default: \
					{ \
						display_message(ERROR_MESSAGE, \
							"SELECT_GLOBAL_OBJECT_CHANGE(" #object_type \
							").  Invalid appearance"); \
					} break; \
				} \
				/* if current_object not in menu SELECT_SELECT_OBJECT */ \
				/* will select the first item in it, and call an update */ \
				SELECT_SELECT_OBJECT(object_type)(temp_select); \
			} break; \
			case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(object_type): \
			{ \
				/* do nothing */ \
			} break; \
			default: \
			{ \
				display_message(WARNING_MESSAGE, \
					"SELECT_GLOBAL_OBJECT_CHANGE(" #object_type \
					").  Unknown MANAGER_CHANGE message constant"); \
			} break; \
		} \
	} \
	LEAVE; \
} /* SELECT_GLOBAL_OBJECT_CHANGE(object_type) */

#if ! defined (SHORT_NAMES)
#define SELECT_CREATE_LIST_( object_type )  select_create_list_ ## object_type
#else
#define SELECT_CREATE_LIST_( object_type )  scl ## object_type
#endif
#define SELECT_CREATE_LIST( object_type )  SELECT_CREATE_LIST_(object_type)

#define DECLARE_SELECT_CREATE_LIST_FUNCTION( object_type ) \
static int SELECT_CREATE_LIST(object_type)( \
	struct SELECT_STRUCT(object_type) *temp_select) \
/***************************************************************************** \
LAST MODIFIED : 7 May 1997 \
\
DESCRIPTION : \
Inserts an item list into the select widget (for appearance SELECT_LIST). \
============================================================================*/ \
{ \
	Arg override_arg; \
	int return_code; \
	MrmType select_item_class; \
	Widget temp_widget; \
\
	ENTER(SELECT_CREATE_LIST(object_type)); \
	if (select_hierarchy_open) \
	{ \
		XtSetArg(override_arg,XmNuserData,temp_select); \
		temp_widget=(Widget)NULL; \
		if (MrmSUCCESS==MrmFetchWidgetOverride(select_hierarchy, \
			"select_list_scroll",temp_select->main_form,NULL,&override_arg,1, \
			&temp_widget,&select_item_class)) \
		{ \
			XtManageChild(temp_widget); \
			return_code=1; \
		} \
		else \
		{ \
			display_message(WARNING_MESSAGE, \
				"SELECT_CREATE_LIST(" #object_type ").  Could not fetch list widget"); \
			return_code=0; \
		} \
	} \
	else \
	{ \
		display_message(WARNING_MESSAGE, \
			"SELECT_CREATE_LIST(" #object_type ").  Hierarchy not open"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* SELECT_CREATE_LIST(object_type) */

#if ! defined (SHORT_NAMES)
#define SELECT_CREATE_TEXT_( object_type )  select_create_text_ ## object_type
#else
#define SELECT_CREATE_TEXT_( object_type )  sct ## object_type
#endif
#define SELECT_CREATE_TEXT( object_type )  SELECT_CREATE_TEXT_(object_type)

#define DECLARE_SELECT_CREATE_TEXT_FUNCTION( object_type , object_label ) \
static int SELECT_CREATE_TEXT(object_type)( \
	struct SELECT_STRUCT(object_type) *temp_select) \
/***************************************************************************** \
LAST MODIFIED : 7 May 1997 \
\
DESCRIPTION : \
Inserts a text form into the select widget (for appearance SELECT_TEXT). \
============================================================================*/ \
{ \
	Arg override_arg; \
	int return_code; \
	MrmType select_item_class; \
	Widget temp_widget; \
	XmString new_string; \
\
	ENTER(SELECT_CREATE_TEXT(object_type)); \
	if (select_hierarchy_open) \
	{ \
		XtSetArg(override_arg,XmNuserData,temp_select); \
		temp_widget=(Widget)NULL; \
		if (MrmSUCCESS==MrmFetchWidgetOverride(select_hierarchy, \
			"select_text_form",temp_select->main_form,NULL,&override_arg,1, \
			&temp_widget,&select_item_class)) \
		{ \
			XtManageChild(temp_widget); \
			/* name the label */ \
			/*???RC Make macro GET_OBJECT_LABEL(object_type) in object.h? */ \
			new_string=XmStringCreateSimple(object_label); \
			XtVaSetValues(temp_select->text_label,XmNlabelString,new_string,NULL); \
			XmStringFree(new_string); \
			return_code=1; \
		} \
		else \
		{ \
			display_message(WARNING_MESSAGE, \
				"SELECT_CREATE_TEXT(" #object_type ").  Could not fetch text widget"); \
			return_code=0; \
		} \
	} \
	else \
	{ \
		display_message(WARNING_MESSAGE, \
			"SELECT_CREATE_TEXT(" #object_type ").  Hierarchy not open"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* SELECT_CREATE_TEXT(object_type) */

#define DECLARE_SELECT_REGISTER_UIL_NAMES_FUNCTION( object_type ) \
PROTOTYPE_SELECT_REGISTER_UIL_NAMES_FUNCTION(object_type) \
{ \
	int return_code; \
	static MrmRegisterArg UIL_name_list[]= \
	{ \
		{"select_structure",(XtPointer)NULL}, \
		{"select_identify_main_form",(XtPointer) \
			DIALOG_IDENTIFY(SELECT_DIALOG_NAME(object_type),main_form)}, \
		{"select_identify_edit_name",(XtPointer) \
			DIALOG_IDENTIFY(SELECT_DIALOG_NAME(object_type),edit_name)}, \
		{"select_identify_edit_label",(XtPointer) \
			DIALOG_IDENTIFY(SELECT_DIALOG_NAME(object_type),edit_label)}, \
		{"select_identify_edit_text",(XtPointer) \
			DIALOG_IDENTIFY(SELECT_DIALOG_NAME(object_type),edit_text)}, \
		{"select_identify_create_btn",(XtPointer) \
			DIALOG_IDENTIFY(SELECT_DIALOG_NAME(object_type),create_btn)}, \
		{"select_identify_delete_btn",(XtPointer) \
			DIALOG_IDENTIFY(SELECT_DIALOG_NAME(object_type),delete_btn)}, \
		{"select_identify_rename_btn",(XtPointer) \
			DIALOG_IDENTIFY(SELECT_DIALOG_NAME(object_type),rename_btn)}, \
		{"select_identify_text_label",(XtPointer) \
			DIALOG_IDENTIFY(SELECT_DIALOG_NAME(object_type),text_label)}, \
		{"select_identify_text_text",(XtPointer) \
			DIALOG_IDENTIFY(SELECT_DIALOG_NAME(object_type),text_text)}, \
		{"select_identify_list_scroll",(XtPointer) \
			DIALOG_IDENTIFY(SELECT_DIALOG_NAME(object_type),list_scroll)}, \
		{"select_identify_list_rowcol",(XtPointer) \
			DIALOG_IDENTIFY(SELECT_DIALOG_NAME(object_type),list_rowcol)}, \
		{"select_destroy_CB",(XtPointer)SELECT_DESTROY_CB(object_type)}, \
		{"select_focus_CB",(XtPointer)SELECT_FOCUS_CB(object_type)}, \
		{"select_create_CB",(XtPointer)SELECT_CREATE_CB(object_type)}, \
		{"select_delete_CB",(XtPointer)SELECT_DELETE_CB(object_type)}, \
		{"select_rename_CB",(XtPointer)SELECT_RENAME_CB(object_type)}, \
		{"select_name_CB",(XtPointer)SELECT_NAME_CB(object_type)}, \
		{"select_select_text_CB",(XtPointer)SELECT_SELECT_TEXT_CB(object_type)} \
	}; \
\
	ENTER(SELECT_REGISTER_UIL_NAMES(object_type)); \
	/* Assign value of name pointing at SELECT_STRUCT(object_type) */ \
	UIL_name_list[0].value=(XtPointer)temp_select; \
	/* register UIL names of callbacks and identifiers */ \
	if (MrmSUCCESS==MrmRegisterNamesInHierarchy(select_hierarchy, \
		UIL_name_list,XtNumber(UIL_name_list))) \
	{ \
		return_code=1; \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"SELECT_REGISTER_UIL_NAMES(" #object_type \
			").  Could not register UIL names"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* SELECT_REGISTER_UIL_NAMES(object_type) */

/*
Global Functions
----------------
*/
#define DECLARE_CREATE_SELECT_WIDGET_FUNCTION( object_type ) \
PROTOTYPE_CREATE_SELECT_WIDGET_FUNCTION( object_type ) \
/***************************************************************************** \
LAST MODIFIED : 12 May 1997 \
\
DESCRIPTION : \
Creates a selection widget that will allow the user to choose an object based \
upon its name. \
============================================================================*/ \
{ \
	MrmType select_dialog_class; \
	struct SELECT_STRUCT(object_type) *temp_select=NULL; \
	Widget return_widget; \
\
	ENTER(CREATE_SELECT_WIDGET(object_type)); \
	return_widget=(Widget)NULL; \
	if (MrmOpenHierarchy_base64_string(select_uidh, \
		&select_hierarchy,&select_hierarchy_open)) \
	{ \
		/* allocate memory */ \
		if (ALLOCATE(temp_select,struct SELECT_STRUCT(object_type),1)) \
		{ \
			/* initialise the structure */ \
			temp_select->widget_parent=parent; \
			temp_select->widget=(Widget)NULL; \
			temp_select->widget_address=select_widget; \
			temp_select->appearance=appearance; \
			temp_select->manager_callback_id=(void *)NULL; \
			temp_select->object_manager=object_manager; \
			temp_select->last_updated_object=(struct object_type *)NULL; \
			temp_select->delete_position=0; \
			if (!selected_object) \
			{ \
				temp_select->current_object= \
					FIRST_OBJECT_IN_MANAGER_THAT(object_type)( \
					(MANAGER_CONDITIONAL_FUNCTION(object_type) *)NULL, \
					(void *)NULL,temp_select->object_manager); \
			} \
			else \
			{ \
				temp_select->current_object=selected_object; \
			} \
			temp_select->main_form=(Widget)NULL; \
			temp_select->edit_name=(Widget)NULL; \
			temp_select->edit_label=(Widget)NULL; \
			temp_select->edit_text=(Widget)NULL; \
			temp_select->create_btn=(Widget)NULL; \
			temp_select->delete_btn=(Widget)NULL; \
			temp_select->rename_btn=(Widget)NULL; \
			temp_select->text_label=(Widget)NULL; \
			temp_select->text_text=(Widget)NULL; \
			temp_select->list_scroll=(Widget)NULL; \
			temp_select->list_rowcol=(Widget)NULL; \
			temp_select->update_callback.procedure=(Callback_procedure *)NULL; \
			temp_select->update_callback.data=NULL; \
			if (SELECT_REGISTER_UIL_NAMES(object_type)(temp_select)) \
			{ \
				/* fetch select control widget */ \
				if (MrmSUCCESS==MrmFetchWidget(select_hierarchy,"select_widget", \
					temp_select->widget_parent,&(temp_select->widget), \
					&select_dialog_class)) \
				{ \
					XtManageChild(temp_select->widget); \
					/* create the correct list or text widget, and update it */ \
					switch (temp_select->appearance) \
					{ \
						case SELECT_LIST: \
						{ \
							SELECT_CREATE_LIST(object_type)(temp_select); \
							/* display the correct names */ \
							SELECT_UPDATE_LIST(object_type)(temp_select); \
							SELECT_SELECT_OBJECT(object_type)(temp_select); \
						} break; \
						case SELECT_TEXT: \
						{ \
							SELECT_CREATE_TEXT(object_type)(temp_select); \
							/* display the correct names */ \
							SELECT_SELECT_OBJECT(object_type)(temp_select); \
						} break; \
						default: \
						{ \
							display_message(ERROR_MESSAGE, \
								"CREATE_SELECT_WIDGET(" #object_type \
								").  Invalid appearance"); \
						} break; \
					} \
					temp_select->manager_callback_id=MANAGER_REGISTER(object_type)( \
						SELECT_GLOBAL_OBJECT_CHANGE(object_type), \
						temp_select,temp_select->object_manager); \
					/* unmanage the renaming bit */ \
					XtUnmanageChild(temp_select->edit_name); \
					return_widget=temp_select->widget; \
				} \
				else \
				{ \
					display_message(ERROR_MESSAGE, \
						"CREATE_SELECT_WIDGET(" #object_type \
						").  Could not fetch select dialog"); \
					DEALLOCATE(temp_select); \
				} \
			} \
			else \
			{ \
				display_message(ERROR_MESSAGE, \
					"CREATE_SELECT_WIDGET(" #object_type \
					").  Could not register UIL names"); \
				DEALLOCATE(temp_select); \
			} \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE, \
				"CREATE_SELECT_WIDGET(" #object_type \
				").  Could not allocate control window structure"); \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE, \
			"CREATE_SELECT_WIDGET(" #object_type \
			").  Could not open hierarchy"); \
	} \
	if (select_widget) \
	{ \
		*select_widget=return_widget; \
	} \
	LEAVE; \
\
	return (return_widget); \
} /* CREATE_SELECT_WIDGET(object_type) */

#define DECLARE_SELECT_SET_UPDATE_CB_FUNCTION( object_type ) \
PROTOTYPE_SELECT_SET_UPDATE_CB_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 25 May 1997 \
\
DESCRIPTION : \
Changes the callback item of the input widget. \
============================================================================*/ \
{ \
	int return_code; \
	struct SELECT_STRUCT(object_type) *temp_select; \
\
	ENTER(SELECT_SET_UPDATE_CB(object_type)); \
	/* check arguments */ \
	if (select_widget) \
	{ \
		/* get pointer to the data for the select dialog */ \
		XtVaGetValues(select_widget,XmNuserData,&temp_select,NULL); \
		if (temp_select) \
		{ \
			temp_select->update_callback.procedure=cb_data->procedure; \
			temp_select->update_callback.data=cb_data->data; \
			return_code=1; \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE,"SELECT_SET_UPDATE_CB(" #object_type \
				").  Missing XmNuserData"); \
			return_code=0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE,"SELECT_SET_UPDATE_CB(" #object_type \
			").  Missing select_widget"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* SELECT_SET_UPDATE_CB(object_type) */

#define DECLARE_SELECT_SET_SELECT_ITEM_FUNCTION( object_type ) \
PROTOTYPE_SELECT_SET_SELECT_ITEM_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 20 April 2000 \
\
DESCRIPTION : \
Changes the selected object in the select widget. \
============================================================================*/ \
{ \
	int return_code; \
	struct SELECT_STRUCT(object_type) *temp_select; \
\
	ENTER(SELECT_SET_SELECT_ITEM(object_type)); \
	/* check arguments */ \
	if (select_widget) \
	{ \
		/* get the pointer to the data for the select dialog */ \
		XtVaGetValues(select_widget,XmNuserData,&temp_select,NULL); \
		if (temp_select) \
		{ \
			temp_select->last_updated_object=selected_object; \
			temp_select->current_object=selected_object; \
			SELECT_SELECT_OBJECT(object_type)(temp_select); \
			return_code=1; \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE,"SELECT_SET_SELECT_ITEM(" #object_type \
				").  Missing XmNuserData"); \
			return_code=0; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE,"SELECT_SET_SELECT_ITEM(" #object_type \
			").  Missing select_widget"); \
		return_code=0; \
	} \
	LEAVE; \
\
	return (return_code); \
} /* SELECT_SET_SELECT_ITEM(object_type) */

#define DECLARE_SELECT_GET_UPDATE_CB_FUNCTION( object_type ) \
PROTOTYPE_SELECT_GET_UPDATE_CB_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 25 May 1997 \
\
DESCRIPTION : \
Returns a pointer to the callback item of the select widget. \
============================================================================*/ \
{ \
	struct Callback_data *update_callback; \
	struct SELECT_STRUCT(object_type) *temp_select; \
\
	ENTER(SELECT_GET_UPDATE_CB(object_type)); \
	/* check arguments */ \
	if (select_widget) \
	{ \
		/* get the pointer to the data for the select dialog */ \
		XtVaGetValues(select_widget,XmNuserData,&temp_select,NULL); \
		if (temp_select) \
		{ \
			update_callback= &(temp_select->update_callback); \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE,"SELECT_GET_UPDATE_CB(" #object_type \
				").  Missing XmNuserData"); \
			update_callback=(struct Callback_data *)NULL; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE,"SELECT_GET_UPDATE_CB(" #object_type \
			").  Missing select_widget"); \
		update_callback=(struct Callback_data *)NULL; \
	} \
	LEAVE; \
\
	return (update_callback); \
} /* SELECT_GET_UPDATE_CB(object_type) */

#define DECLARE_SELECT_GET_SELECT_ITEM_FUNCTION( object_type ) \
PROTOTYPE_SELECT_GET_SELECT_ITEM_FUNCTION(object_type) \
/***************************************************************************** \
LAST MODIFIED : 25 May 1997 \
\
DESCRIPTION : \
Returns a pointer to the selected object in the select widget. \
============================================================================*/ \
{ \
	struct object_type *selected_object; \
	struct SELECT_STRUCT(object_type) *temp_select; \
\
	ENTER(SELECT_GET_SELECT_ITEM(object_type)); \
	/* check arguments */ \
	if (select_widget) \
	{ \
		/* get the pointer to the data for the select dialog */ \
		XtVaGetValues(select_widget,XmNuserData,&temp_select,NULL); \
		if (temp_select) \
		{ \
			selected_object=temp_select->current_object; \
		} \
		else \
		{ \
			display_message(ERROR_MESSAGE,"SELECT_GET_SELECT_ITEM(" #object_type \
				").  Missing XmNuserData"); \
			selected_object=(struct object_type *)NULL; \
		} \
	} \
	else \
	{ \
		display_message(ERROR_MESSAGE,"SELECT_GET_SELECT_ITEM(" #object_type \
			").  Missing select_widget"); \
		selected_object=(struct object_type *)NULL; \
	} \
	LEAVE; \
\
	return (selected_object); \
} /* SELECT_GET_SELECT_ITEM(object_type) */

#define DECLARE_SELECT_MODULE_FUNCTIONS( object_type , object_label ) \
PROTOTYPE_SELECT_REGISTER_UIL_NAMES_FUNCTION(object_type); \
DECLARE_SELECT_UPDATE_FUNCTION(object_type) \
DECLARE_CREATE_SUB_WIDGET_FUNCTION(object_type) \
DECLARE_SELECT_UPDATE_LIST_FUNCTION(object_type) \
DECLARE_SELECT_IDENTIFY_FUNCTIONS(object_type) \
DECLARE_SELECT_DESTROY_CB_FUNCTION(object_type) \
DECLARE_SELECT_SELECT_OBJECT_FUNCTION(object_type) \
DECLARE_SELECT_CREATE_CB_FUNCTION(object_type) \
DECLARE_SELECT_DELETE_CB_FUNCTION(object_type) \
DECLARE_SELECT_RENAME_CB_FUNCTION(object_type) \
DECLARE_SELECT_NAME_CB_FUNCTION(object_type) \
DECLARE_SELECT_SELECT_TEXT_CB_FUNCTION(object_type) \
DECLARE_SELECT_FOCUS_CB_FUNCTION(object_type) \
DECLARE_SELECT_GLOBAL_OBJECT_CHANGE_FUNCTION(object_type) \
DECLARE_SELECT_CREATE_LIST_FUNCTION(object_type) \
DECLARE_SELECT_CREATE_TEXT_FUNCTION(object_type,object_label) \
DECLARE_SELECT_REGISTER_UIL_NAMES_FUNCTION(object_type)

#define DECLARE_SELECT_GLOBAL_FUNCTIONS( object_type ) \
DECLARE_CREATE_SELECT_WIDGET_FUNCTION(object_type) \
DECLARE_SELECT_SET_UPDATE_CB_FUNCTION(object_type) \
DECLARE_SELECT_SET_SELECT_ITEM_FUNCTION(object_type) \
DECLARE_SELECT_GET_UPDATE_CB_FUNCTION(object_type) \
DECLARE_SELECT_GET_SELECT_ITEM_FUNCTION(object_type)

#endif /* !defined (SELECT_PRIVATE_H) */
