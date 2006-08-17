/*******************************************************************************
FILE : curve_editor_dialog.c

LAST MODIFIED : 23 May 2001

DESCRIPTION :
Routines for creating an element group editor dialog shell and standard buttons.
Form set aside for the actual element group editor.
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
#include <stdio.h>
#include "general/debug.h"
#include "select/select_curve.h"
#include "curve/curve.h"
#include "curve/curve_editor.h"
#include "curve/curve_editor_dialog.h"
static char curve_editor_dialog_uidh[] =
#include "curve/curve_editor_dialog.uidh"
	;
#if defined (OLD_CODE)
#include "user_interface/confirmation.h"
#endif /* defined (OLD_CODE) */
#include "user_interface/gui_dialog_macros.h"
#include "user_interface/message.h"

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int curve_editor_dialog_hierarchy_open=0;
static MrmHierarchy curve_editor_dialog_hierarchy;
#endif /* defined (MOTIF) */

struct Curve_editor_dialog
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Contains all the information needed for the curve_editor_dialog widget.
==============================================================================*/
{
	struct Curve *current_curve;
	struct Callback_data update_callback;
	struct MANAGER(Curve) *curve_manager;
	void *curve_manager_callback_id;
	/* following flag on while manager modify of curve is performed by dialog
		 so that manager callback is not interpreted as coming from elsewhere */
	int applying_now;
	struct User_interface *user_interface;
	Widget select_form,select_widget,editor_form,editor_widget;
	Widget ok_button,apply_button,revert_button,cancel_button;
	Widget *dialog_address,dialog,widget,dialog_parent;
}; /* curve_editor_dialog_struct */

/*
Module functions
----------------
*/

DECLARE_DIALOG_IDENTIFY_FUNCTION(curve_editor_dialog, \
	Curve_editor_dialog,select_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(curve_editor_dialog, \
	Curve_editor_dialog,editor_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(curve_editor_dialog, \
	Curve_editor_dialog,ok_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(curve_editor_dialog, \
	Curve_editor_dialog,apply_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(curve_editor_dialog, \
	Curve_editor_dialog,revert_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(curve_editor_dialog, \
	Curve_editor_dialog,cancel_button)

static void curve_editor_dialog_destroy_CB(Widget widget,int *tag,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Callback for when the curve_editor_dialog is closed. Tidies up all
details: dynamic allocations, etc.
==============================================================================*/
{
	struct Curve_editor_dialog *curve_editor_dialog;

	ENTER(curve_editor_dialog_destroy_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	if (widget)
	{
		/* get the pointer to the data for the widget */
		XtVaGetValues(widget,XmNuserData,&curve_editor_dialog,NULL);
		if (curve_editor_dialog)
		{
			if (curve_editor_dialog->curve_manager_callback_id)
			{
				/* stop receiving control curve manager messages */
				MANAGER_DEREGISTER(Curve)(
					curve_editor_dialog->curve_manager_callback_id,
					curve_editor_dialog->curve_manager);
			}
			/* clear the curve_editor_dialog for the client */
			*(curve_editor_dialog->dialog_address)=(Widget)NULL;
			/* deallocate the memory for the user data */
			DEALLOCATE(curve_editor_dialog);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"curve_editor_dialog_destroy_CB.  "
				"Missing curve_editor_dialog");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"curve_editor_dialog_destroy_CB.  Missing widget");
	}
	LEAVE;
} /* curve_editor_dialog_destroy_CB */

int curve_editor_apply_changes(
	struct Curve_editor_dialog *curve_editor_dialog)
/*******************************************************************************
LAST MODIFIED : 25 November 1999

DESCRIPTION :
Gets the curve from the editor and modifies the current_curve to match it.
==============================================================================*/
{
	int return_code;
	struct Curve *curve;

	ENTER(curve_editor_apply_changes);
	if (curve_editor_dialog)
	{
		if (curve_editor_dialog->current_curve)
		{
			if (curve=curve_editor_get_curve(
				curve_editor_dialog->editor_widget))
			{
				/* set flag so manager modify not seen as coming from elsewhere */
				curve_editor_dialog->applying_now=1;
				return_code=MANAGER_MODIFY_NOT_IDENTIFIER(Curve,name)(
					curve_editor_dialog->current_curve,curve,
					curve_editor_dialog->curve_manager);
				curve_editor_dialog->applying_now=0;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"curve_editor_apply_changes.  Missing curve");
				return_code=0;
			}
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"curve_editor_apply_changes.  Missing dialog");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* curve_editor_apply_changes */


static void curve_editor_dialog_ok_CB(Widget widget,int *tag,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Applies the changes to the curve globally, then closes the dialog.
==============================================================================*/
{
	struct Curve_editor_dialog *curve_editor_dialog;

	ENTER(curve_editor_dialog_ok_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	if (widget)
	{
		/* get the pointer to the data for the widget */
		XtVaGetValues(widget,XmNuserData,&curve_editor_dialog,NULL);
		if (curve_editor_apply_changes(curve_editor_dialog))
		{
			/* close the dialog shell */
			XtDestroyWidget(curve_editor_dialog->dialog);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"curve_editor_dialog_ok_CB.  Could not apply changes");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"curve_editor_dialog_ok_CB.  Missing widget");
	}
	LEAVE;
} /* curve_editor_dialog_ok_CB */

static void curve_editor_dialog_apply_CB(Widget widget,int *tag,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Applies the changes to the curve globally.
==============================================================================*/
{
	struct Curve_editor_dialog *curve_editor_dialog;

	ENTER(curve_editor_dialog_apply_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	if (widget)
	{
		/* get the pointer to the data for the widget */
		XtVaGetValues(widget,XmNuserData,&curve_editor_dialog,NULL);
		curve_editor_apply_changes(curve_editor_dialog);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"curve_editor_dialog_apply_CB.  Missing widget");
	}
	LEAVE;
} /* curve_editor_dialog_apply_CB */

static void curve_editor_dialog_revert_CB(Widget widget,int *tag,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 25 September 1997

DESCRIPTION :
Passes the current_curve to the curve editor to restore its previous
state.
==============================================================================*/
{
	struct Curve_editor_dialog *curve_editor_dialog;

	ENTER(curve_editor_dialog_revert_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	if (widget)
	{
		/* get the pointer to the data for the widget */
		XtVaGetValues(widget,XmNuserData,&curve_editor_dialog,NULL);
		if (curve_editor_dialog)
		{
			curve_editor_set_curve(curve_editor_dialog->editor_widget,
				curve_editor_dialog->current_curve);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"curve_editor_dialog_revert_CB.  Missing widget data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"curve_editor_dialog_revert_CB.  Missing widget");
	}
	LEAVE;
} /* curve_editor_dialog_revert_CB */

static void curve_editor_dialog_cancel_CB(Widget widget,int *tag,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 25 September 1997

DESCRIPTION :
Closes the dialog without making any changes.
==============================================================================*/
{
	struct Curve_editor_dialog *curve_editor_dialog;

	ENTER(curve_editor_dialog_cancel_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	if (widget)
	{
		/* get the pointer to the data for the widget */
		XtVaGetValues(widget,XmNuserData,&curve_editor_dialog,NULL);
		if (curve_editor_dialog)
		{
			/* close the dialog shell */
			XtDestroyWidget(curve_editor_dialog->dialog);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"curve_editor_dialog_cancel_CB.  Missing widget data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"curve_editor_dialog_cancel_CB.  Missing widget");
	}
	LEAVE;
} /* curve_editor_dialog_cancel_CB */

static void curve_editor_Curve_change(
	struct MANAGER_MESSAGE(Curve) *message,
	void *curve_editor_dialog_void)
/*******************************************************************************
LAST MODIFIED : 23 May 2001

DESCRIPTION :
Something has changed globally in the Curve manager. Passes on messages
about changes as stemming from computed_field_manager for fields of type
COMPUTED_FIELD_CURVE_LOOKUP.
==============================================================================*/
{
	struct Curve_editor_dialog *curve_editor_dialog;

	ENTER(curve_editor_Curve_change);
	if (message && (curve_editor_dialog =
		(struct Curve_editor_dialog *)curve_editor_dialog_void))
	{
		if (!curve_editor_dialog->applying_now)
		{
			switch (message->change)
			{
				case MANAGER_CHANGE_OBJECT(Curve):
				case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Curve):
				{
					/* if current curve has changed global, reinstate it in the editor */
					if (IS_OBJECT_IN_LIST(Curve)(
						curve_editor_dialog->current_curve, message->changed_object_list))
					{
						curve_editor_set_curve(curve_editor_dialog->editor_widget,
							curve_editor_dialog->current_curve);
					}
				} break;
				case MANAGER_CHANGE_ADD(Curve):
				case MANAGER_CHANGE_REMOVE(Curve):
				case MANAGER_CHANGE_IDENTIFIER(Curve):
				{
					/* do nothing */
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"curve_editor_Curve_change.  "
						"Unknown manager message");
				} break;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"curve_editor_Curve_change.  Invalid argument(s)");
	}
	LEAVE;
} /* curve_editor_Curve_change */

static void curve_editor_dialog_update_selection(Widget select_widget,
	void *user_data,void *curve_void)
/*******************************************************************************
LAST MODIFIED : 25 November 1999

DESCRIPTION :
Finds which curve is selected, and informs the editor widget.
==============================================================================*/
{
	struct Curve_editor_dialog *curve_editor_dialog;

	ENTER(curve_editor_dialog_update_selection);
	if (select_widget)
	{
		if (curve_editor_dialog=
			(struct Curve_editor_dialog *)user_data)
		{
			curve_editor_dialog->current_curve=(struct Curve *)curve_void;
			curve_editor_set_curve(curve_editor_dialog->editor_widget,
				curve_editor_dialog->current_curve);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"curve_editor_dialog_update_selection.  Invalid argument(s)");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"curve_editor_dialog_update_selection.  Missing select widget");
	}
	LEAVE;
} /* curve_editor_dialog_update_selection */

static Widget create_curve_editor_dialog(
	Widget *curve_editor_dialog_address,Widget parent,
	struct MANAGER(Curve) *curve_manager,
	struct Curve *curve,struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
==============================================================================*/
{
	int init_widgets;
	MrmType curve_editor_dialog_dialog_class;
	struct Callback_data callback;
	struct Curve_editor_dialog *curve_editor_dialog;
	static MrmRegisterArg callback_list[]=
	{
		{"curve_edd_id_select_form",(XtPointer)
			DIALOG_IDENTIFY(curve_editor_dialog,select_form)},
		{"curve_edd_id_editor_form",(XtPointer)
			DIALOG_IDENTIFY(curve_editor_dialog,editor_form)},
		{"curve_edd_id_ok_btn",(XtPointer)
			DIALOG_IDENTIFY(curve_editor_dialog,ok_button)},
		{"curve_edd_id_apply_btn",(XtPointer)
			DIALOG_IDENTIFY(curve_editor_dialog,apply_button)},
		{"curve_edd_id_revert_btn",(XtPointer)
			DIALOG_IDENTIFY(curve_editor_dialog,revert_button)},
		{"curve_edd_id_cancel_btn",(XtPointer)
			DIALOG_IDENTIFY(curve_editor_dialog,cancel_button)},
		{"curve_edd_destroy_CB",(XtPointer)
			curve_editor_dialog_destroy_CB},
		{"curve_edd_ok_CB",(XtPointer)
			curve_editor_dialog_ok_CB},
		{"curve_edd_apply_CB",(XtPointer)
			curve_editor_dialog_apply_CB},
		{"curve_edd_revert_CB",(XtPointer)
			curve_editor_dialog_revert_CB},
		{"curve_edd_cancel_CB",(XtPointer)
			curve_editor_dialog_cancel_CB}
	};
	static MrmRegisterArg identifier_list[]=
	{
		{"curve_edd_structure",(XtPointer)NULL}
	};
	Widget return_widget;

	ENTER(create_curve_editor_dialog);
	return_widget=(Widget)NULL;
	/* check arguments */
	if (curve_editor_dialog_address&&parent&&curve_manager&&
		user_interface)
	{
		if (MrmOpenHierarchy_binary_string(curve_editor_dialog_uidh, sizeof(curve_editor_dialog_uidh),
			&curve_editor_dialog_hierarchy,
			&curve_editor_dialog_hierarchy_open))
		{
			/* allocate memory */
			if (ALLOCATE(curve_editor_dialog,
				struct Curve_editor_dialog,1))
			{
				/* initialise the structure */
				curve_editor_dialog->dialog_parent=parent;
				curve_editor_dialog->dialog_address=
					curve_editor_dialog_address;
				curve_editor_dialog->current_curve=(struct Curve *)NULL;
				curve_editor_dialog->curve_manager=curve_manager;
				curve_editor_dialog->curve_manager_callback_id=(void *)NULL;
				curve_editor_dialog->applying_now=0;
				curve_editor_dialog->user_interface=user_interface;
				curve_editor_dialog->widget=(Widget)NULL;
				curve_editor_dialog->dialog=(Widget)NULL;
				curve_editor_dialog->select_form=(Widget)NULL;
				curve_editor_dialog->select_widget=(Widget)NULL;
				curve_editor_dialog->editor_form=(Widget)NULL;
				curve_editor_dialog->editor_widget=(Widget)NULL;
				curve_editor_dialog->ok_button=(Widget)NULL;
				curve_editor_dialog->apply_button=(Widget)NULL;
				curve_editor_dialog->revert_button=(Widget)NULL;
				curve_editor_dialog->cancel_button=(Widget)NULL;
				curve_editor_dialog->update_callback.procedure=
					(Callback_procedure *)NULL;
				curve_editor_dialog->update_callback.data=NULL;
				/* make the dialog shell */
				if (curve_editor_dialog->dialog=XtVaCreatePopupShell(
					"Curve Editor",topLevelShellWidgetClass,parent,
					XmNallowShellResize,FALSE,NULL))
				{
					/* register the callbacks */
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
						curve_editor_dialog_hierarchy,callback_list,
						XtNumber(callback_list)))
					{
						/* assign and register the identifiers */
						identifier_list[0].value=(XtPointer)curve_editor_dialog;
						if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
							curve_editor_dialog_hierarchy,identifier_list,
							XtNumber(identifier_list)))
						{
							/* fetch dialog widget */
							if (MrmSUCCESS==MrmFetchWidget(
								curve_editor_dialog_hierarchy,
								"curve_edd_widget",curve_editor_dialog->dialog,
								&(curve_editor_dialog->widget),
								&curve_editor_dialog_dialog_class))
							{
								XtManageChild(curve_editor_dialog->widget);
								init_widgets=1;
								/* create subwidgets */
								if (!CREATE_SELECT_WIDGET(Curve)(
									&curve_editor_dialog->select_widget,
									curve_editor_dialog->select_form,SELECT_LIST,
									curve_editor_dialog->current_curve,curve_manager))
								{
									display_message(ERROR_MESSAGE,
										"create_curve_editor_dialog.  "
										"Could not create select widget.");
									init_widgets=0;
								}
								if (!create_curve_editor_widget(
									&(curve_editor_dialog->editor_widget),
									curve_editor_dialog->editor_form,(struct Curve *)NULL,
									user_interface))
								{
									init_widgets=0;
								}
								if (init_widgets)
								{
									/* register for control curve manager callbacks */
									curve_editor_dialog->curve_manager_callback_id=
										MANAGER_REGISTER(Curve)(
											curve_editor_Curve_change,
											(void *)curve_editor_dialog,curve_manager);
									callback.procedure=
										curve_editor_dialog_update_selection;
									callback.data=(void *)curve_editor_dialog;
									SELECT_SET_UPDATE_CB(Curve)(
										curve_editor_dialog->select_widget,&callback);
									curve_editor_dialog_set_curve(
										curve_editor_dialog->dialog,curve);
									XtRealizeWidget(curve_editor_dialog->dialog);
									XtPopup(curve_editor_dialog->dialog,XtGrabNone);
									return_widget=curve_editor_dialog->dialog;
								}
								else
								{
									XtDestroyWidget(curve_editor_dialog->dialog);
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"create_curve_editor_dialog.  "
									"Could not fetch curve_editor_dialog");
								DEALLOCATE(curve_editor_dialog);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"create_curve_editor_dialog.  "
								"Could not register identifiers");
							DEALLOCATE(curve_editor_dialog);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_curve_editor_dialog.  "
							"Could not register callbacks");
						DEALLOCATE(curve_editor_dialog);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_curve_editor_dialog.  "
						"Could not create popup shell");
					DEALLOCATE(curve_editor_dialog);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_curve_editor_dialog.  "
					"Could not allocate curve_editor_dialog");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_curve_editor_dialog.  Could not open hierarchy");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_curve_editor_dialog.  Invalid argument(s)");
	}
	if (curve_editor_dialog_address&&return_widget)
	{
		*curve_editor_dialog_address=return_widget;
	}
	LEAVE;

	return (return_widget);
} /* create_curve_editor_dialog */

/*
Global functions
----------------
*/
int curve_editor_dialog_set_curve(
	Widget curve_editor_dialog_widget,struct Curve *curve)
/*******************************************************************************
LAST MODIFIED : 8 November 1999

DESCRIPTION :
Sets the curve being edited by the curve editor dialog.
==============================================================================*/
{
	int num_children,return_code;
	struct Curve_editor_dialog *curve_editor_dialog;
	Widget *child_list;

	ENTER(curve_editor_dialog_set_curve);
	if (curve_editor_dialog_widget)
	{
		/* get the pointer to the curve for the dialog */
		XtVaGetValues(curve_editor_dialog_widget,
			XmNchildren,&child_list,
			XmNnumChildren,&num_children,
			NULL);
		if (num_children==1)
		{
			XtVaGetValues(child_list[0],XmNuserData,&curve_editor_dialog,NULL);
			if (curve_editor_dialog)
			{
				if (curve)
				{
					if (!IS_MANAGED(Curve)(curve,
						curve_editor_dialog->curve_manager))
					{
						display_message(ERROR_MESSAGE,
							"curve_editor_dialog_set_curve.  Unmanaged curve");
						curve=(struct Curve *)NULL;
					}
				}
				if (!curve)
				{
					curve=FIRST_OBJECT_IN_MANAGER_THAT(Curve)(
						(MANAGER_CONDITIONAL_FUNCTION(Curve) *)NULL,(void *)NULL,
						curve_editor_dialog->curve_manager);
				}
				/* Must not ACCESS current curve - otherwise cannot be destroyed */
				curve_editor_dialog->current_curve=curve;
				SELECT_SET_SELECT_ITEM(Curve)(curve_editor_dialog->select_widget,
					curve_editor_dialog->current_curve);
				curve_editor_set_curve(curve_editor_dialog->editor_widget,
					curve_editor_dialog->current_curve);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"curve_editor_dialog_set_curve.  Missing dialog data");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"curve_editor_dialog_set_curve.  Invalid dialog");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"curve_editor_dialog_set_curve.  Missing dialog");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* curve_editor_dialog_set_curve */

int curve_editor_dialog_set_cursor_parameter(
	Widget curve_editor_dialog_widget,float parameter)
/*******************************************************************************
LAST MODIFIED : 8 November 1999

DESCRIPTION :
==============================================================================*/
{
	int num_children,return_code;
	struct Curve_editor_dialog *curve_editor_dialog;
	Widget *child_list;

	ENTER(curve_editor_dialog_set_cursor_parameter);
	if (curve_editor_dialog_widget)
	{
		/* get the pointer to the curve for the dialog */
		XtVaGetValues(curve_editor_dialog_widget,
			XmNchildren,&child_list,
			XmNnumChildren,&num_children,
			NULL);
		if (num_children==1)
		{
			XtVaGetValues(child_list[0],XmNuserData,&curve_editor_dialog,NULL);
			if (curve_editor_dialog)
			{
				curve_editor_set_cursor_parameter(curve_editor_dialog->editor_widget,
					parameter );
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"curve_editor_dialog_set_cursor_parameter.  Missing dialog data");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"curve_editor_dialog_set_cursor_parameter.  Invalid dialog");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"curve_editor_dialog_set_cursor_parameter.  Missing dialog");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* curve_editor_dialog_set_cursor_parameter */

int bring_up_curve_editor_dialog(
	Widget *curve_editor_dialog_address,Widget parent,
	struct MANAGER(Curve) *curve_manager,
	struct Curve *curve,struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 8 November 1999

DESCRIPTION :
If there is a curve editor dialog in existence, then bring it to the
front, else create a new one.
==============================================================================*/
{
	int return_code;

	ENTER(bring_up_curve_editor_dialog);
	if (curve_editor_dialog_address)
	{
		if (*curve_editor_dialog_address)
		{
			curve_editor_dialog_set_curve(
				*curve_editor_dialog_address,curve);
			XtPopup(*curve_editor_dialog_address,XtGrabNone);
			return_code=1;
		}
		else
		{
			if (create_curve_editor_dialog(
				curve_editor_dialog_address,parent,curve_manager,
				curve,user_interface))
			{
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"bring_up_curve_editor_dialog.  Error creating dialog");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"bring_up_curve_editor_dialog.  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* bring_up_curve_editor_dialog */

