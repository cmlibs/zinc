/*******************************************************************************
FILE : control_curve_editor_dialog.c

LAST MODIFIED : 25 November 1999

DESCRIPTION :
Routines for creating an element group editor dialog shell and standard buttons.
Form set aside for the actual element group editor.
==============================================================================*/
#include <stdio.h>
#include "general/debug.h"
#include "select/select_control_curve.h"
#include "curve/control_curve.h"
#include "curve/control_curve_editor.h"
#include "curve/control_curve_editor_dialog.h"
#include "curve/control_curve_editor_dialog.uid64"
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
static int control_curve_editor_dialog_hierarchy_open=0;
static MrmHierarchy control_curve_editor_dialog_hierarchy;
#endif /* defined (MOTIF) */

struct Control_curve_editor_dialog
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Contains all the information needed for the control_curve_editor_dialog widget.
==============================================================================*/
{
	struct Control_curve *current_curve;
	struct Callback_data update_callback;
	struct MANAGER(Control_curve) *control_curve_manager;
	void *control_curve_manager_callback_id;
	/* following flag on while manager modify of curve is performed by dialog
		 so that manager callback is not interpreted as coming from elsewhere */
	int applying_now;
	struct User_interface *user_interface;
	Widget select_form,select_widget,editor_form,editor_widget;
	Widget ok_button,apply_button,revert_button,cancel_button;
	Widget *dialog_address,dialog,widget,dialog_parent;
}; /* control_curve_editor_dialog_struct */

/*
Module functions
----------------
*/

DECLARE_DIALOG_IDENTIFY_FUNCTION(control_curve_editor_dialog, \
	Control_curve_editor_dialog,select_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(control_curve_editor_dialog, \
	Control_curve_editor_dialog,editor_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(control_curve_editor_dialog, \
	Control_curve_editor_dialog,ok_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(control_curve_editor_dialog, \
	Control_curve_editor_dialog,apply_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(control_curve_editor_dialog, \
	Control_curve_editor_dialog,revert_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(control_curve_editor_dialog, \
	Control_curve_editor_dialog,cancel_button)

static void control_curve_editor_dialog_destroy_CB(Widget widget,int *tag,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Callback for when the control_curve_editor_dialog is closed. Tidies up all
details: dynamic allocations, etc.
==============================================================================*/
{
	struct Control_curve_editor_dialog *curve_editor_dialog;

	ENTER(control_curve_editor_dialog_destroy_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	if (widget)
	{
		/* get the pointer to the data for the widget */
		XtVaGetValues(widget,XmNuserData,&curve_editor_dialog,NULL);
		if (curve_editor_dialog)
		{
			if (curve_editor_dialog->control_curve_manager_callback_id)
			{
				/* stop receiving control curve manager messages */
				MANAGER_DEREGISTER(Control_curve)(
					curve_editor_dialog->control_curve_manager_callback_id,
					curve_editor_dialog->control_curve_manager);
			}
			/* clear the control_curve_editor_dialog for the client */
			*(curve_editor_dialog->dialog_address)=(Widget)NULL;
			/* deallocate the memory for the user data */
			DEALLOCATE(curve_editor_dialog);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"control_curve_editor_dialog_destroy_CB.  "
				"Missing control_curve_editor_dialog");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"control_curve_editor_dialog_destroy_CB.  Missing widget");
	}
	LEAVE;
} /* control_curve_editor_dialog_destroy_CB */

int control_curve_editor_apply_changes(
	struct Control_curve_editor_dialog *curve_editor_dialog)
/*******************************************************************************
LAST MODIFIED : 25 November 1999

DESCRIPTION :
Gets the curve from the editor and modifies the current_curve to match it.
==============================================================================*/
{
	int return_code;
	struct Control_curve *curve;

	ENTER(control_curve_editor_apply_changes);
	if (curve_editor_dialog)
	{
		if (curve_editor_dialog->current_curve)
		{
			if (curve=control_curve_editor_get_curve(
				curve_editor_dialog->editor_widget))
			{
				/* set flag so manager modify not seen as coming from elsewhere */
				curve_editor_dialog->applying_now=1;
				return_code=MANAGER_MODIFY_NOT_IDENTIFIER(Control_curve,name)(
					curve_editor_dialog->current_curve,curve,
					curve_editor_dialog->control_curve_manager);
				curve_editor_dialog->applying_now=0;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"control_curve_editor_apply_changes.  Missing curve");
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
			"control_curve_editor_apply_changes.  Missing dialog");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* control_curve_editor_apply_changes */


static void control_curve_editor_dialog_ok_CB(Widget widget,int *tag,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Applies the changes to the curve globally, then closes the dialog.
==============================================================================*/
{
	struct Control_curve_editor_dialog *curve_editor_dialog;

	ENTER(control_curve_editor_dialog_ok_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	if (widget)
	{
		/* get the pointer to the data for the widget */
		XtVaGetValues(widget,XmNuserData,&curve_editor_dialog,NULL);
		if (control_curve_editor_apply_changes(curve_editor_dialog))
		{
			/* close the dialog shell */
			XtDestroyWidget(curve_editor_dialog->dialog);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"control_curve_editor_dialog_ok_CB.  Could not apply changes");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"control_curve_editor_dialog_ok_CB.  Missing widget");
	}
	LEAVE;
} /* control_curve_editor_dialog_ok_CB */

static void control_curve_editor_dialog_apply_CB(Widget widget,int *tag,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Applies the changes to the curve globally.
==============================================================================*/
{
	struct Control_curve_editor_dialog *curve_editor_dialog;

	ENTER(control_curve_editor_dialog_apply_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	if (widget)
	{
		/* get the pointer to the data for the widget */
		XtVaGetValues(widget,XmNuserData,&curve_editor_dialog,NULL);
		control_curve_editor_apply_changes(curve_editor_dialog);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"control_curve_editor_dialog_apply_CB.  Missing widget");
	}
	LEAVE;
} /* control_curve_editor_dialog_apply_CB */

static void control_curve_editor_dialog_revert_CB(Widget widget,int *tag,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 25 September 1997

DESCRIPTION :
Passes the current_curve to the curve editor to restore its previous
state.
==============================================================================*/
{
	struct Control_curve_editor_dialog *curve_editor_dialog;

	ENTER(control_curve_editor_dialog_revert_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	if (widget)
	{
		/* get the pointer to the data for the widget */
		XtVaGetValues(widget,XmNuserData,&curve_editor_dialog,NULL);
		if (curve_editor_dialog)
		{
			control_curve_editor_set_curve(curve_editor_dialog->editor_widget,
				curve_editor_dialog->current_curve);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"control_curve_editor_dialog_revert_CB.  Missing widget data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"control_curve_editor_dialog_revert_CB.  Missing widget");
	}
	LEAVE;
} /* control_curve_editor_dialog_revert_CB */

static void control_curve_editor_dialog_cancel_CB(Widget widget,int *tag,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 25 September 1997

DESCRIPTION :
Closes the dialog without making any changes.
==============================================================================*/
{
	struct Control_curve_editor_dialog *curve_editor_dialog;

	ENTER(control_curve_editor_dialog_cancel_CB);
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
				"control_curve_editor_dialog_cancel_CB.  Missing widget data");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"control_curve_editor_dialog_cancel_CB.  Missing widget");
	}
	LEAVE;
} /* control_curve_editor_dialog_cancel_CB */

static void control_curve_editor_Control_curve_change(
	struct MANAGER_MESSAGE(Control_curve) *message,
	void *curve_editor_dialog_void)
/*******************************************************************************
LAST MODIFIED : 25 November 1999

DESCRIPTION :
Something has changed globally in the Control_curve manager. Passes on messages
about changes as stemming from computed_field_manager for fields of type
COMPUTED_FIELD_CURVE_LOOKUP.
==============================================================================*/
{
	struct Control_curve_editor_dialog *curve_editor_dialog;

	ENTER(control_curve_editor_Control_curve_change);
	if (message&&(curve_editor_dialog=
		(struct Control_curve_editor_dialog *)curve_editor_dialog_void))
	{
		if (!curve_editor_dialog->applying_now)
		{
			switch (message->change)
			{
				case MANAGER_CHANGE_ALL(Control_curve):
				case MANAGER_CHANGE_OBJECT(Control_curve):
				case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Control_curve):
				{
					if (!(message->object_changed)||
						(message->object_changed == curve_editor_dialog->current_curve))
					{
#if defined (OLD_CODE)
						if (confirmation_question_yes_no("Curve Editor","Current curve has "
							"been modified elsewhere. Would you like to revert to it?",
							curve_editor_dialog->dialog,curve_editor_dialog->user_interface))
						{
#endif /* defined (OLD_CODE) */
							control_curve_editor_set_curve(curve_editor_dialog->editor_widget,
								curve_editor_dialog->current_curve);
#if defined (OLD_CODE)
						}
#endif /* defined (OLD_CODE) */
					}
				} break;
				case MANAGER_CHANGE_ADD(Control_curve):
				case MANAGER_CHANGE_DELETE(Control_curve):
				case MANAGER_CHANGE_IDENTIFIER(Control_curve):
				{
					/* do nothing */
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"control_curve_editor_Control_curve_change.  "
						"Unknown manager message");
				} break;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"control_curve_editor_Control_curve_change.  Invalid argument(s)");
	}
	LEAVE;
} /* control_curve_editor_Control_curve_change */

static void control_curve_editor_dialog_update_selection(Widget select_widget,
	void *user_data,void *curve_void)
/*******************************************************************************
LAST MODIFIED : 25 November 1999

DESCRIPTION :
Finds which curve is selected, and informs the editor widget.
==============================================================================*/
{
	struct Control_curve_editor_dialog *curve_editor_dialog;

	ENTER(control_curve_editor_dialog_update_selection);
	if (select_widget)
	{
		if (curve_editor_dialog=
			(struct Control_curve_editor_dialog *)user_data)
		{
			curve_editor_dialog->current_curve=(struct Control_curve *)curve_void;
			control_curve_editor_set_curve(curve_editor_dialog->editor_widget,
				curve_editor_dialog->current_curve);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"control_curve_editor_dialog_update_selection.  Invalid argument(s)");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"control_curve_editor_dialog_update_selection.  Missing select widget");
	}
	LEAVE;
} /* control_curve_editor_dialog_update_selection */

static Widget create_control_curve_editor_dialog(
	Widget *control_curve_editor_dialog_address,Widget parent,
	struct MANAGER(Control_curve) *control_curve_manager,
	struct Control_curve *curve,struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
==============================================================================*/
{
	int init_widgets;
	MrmType control_curve_editor_dialog_dialog_class;
	struct Callback_data callback;
	struct Control_curve_editor_dialog *curve_editor_dialog;
	static MrmRegisterArg callback_list[]=
	{
		{"curve_edd_id_select_form",(XtPointer)
			DIALOG_IDENTIFY(control_curve_editor_dialog,select_form)},
		{"curve_edd_id_editor_form",(XtPointer)
			DIALOG_IDENTIFY(control_curve_editor_dialog,editor_form)},
		{"curve_edd_id_ok_btn",(XtPointer)
			DIALOG_IDENTIFY(control_curve_editor_dialog,ok_button)},
		{"curve_edd_id_apply_btn",(XtPointer)
			DIALOG_IDENTIFY(control_curve_editor_dialog,apply_button)},
		{"curve_edd_id_revert_btn",(XtPointer)
			DIALOG_IDENTIFY(control_curve_editor_dialog,revert_button)},
		{"curve_edd_id_cancel_btn",(XtPointer)
			DIALOG_IDENTIFY(control_curve_editor_dialog,cancel_button)},
		{"curve_edd_destroy_CB",(XtPointer)
			control_curve_editor_dialog_destroy_CB},
		{"curve_edd_ok_CB",(XtPointer)
			control_curve_editor_dialog_ok_CB},
		{"curve_edd_apply_CB",(XtPointer)
			control_curve_editor_dialog_apply_CB},
		{"curve_edd_revert_CB",(XtPointer)
			control_curve_editor_dialog_revert_CB},
		{"curve_edd_cancel_CB",(XtPointer)
			control_curve_editor_dialog_cancel_CB}
	};
	static MrmRegisterArg identifier_list[]=
	{
		{"curve_edd_structure",(XtPointer)NULL}
	};
	Widget return_widget;

	ENTER(create_control_curve_editor_dialog);
	return_widget=(Widget)NULL;
	/* check arguments */
	if (control_curve_editor_dialog_address&&parent&&control_curve_manager&&
		user_interface)
	{
		if (MrmOpenHierarchy_base64_string(control_curve_editor_dialog_uid64,
			&control_curve_editor_dialog_hierarchy,
			&control_curve_editor_dialog_hierarchy_open))
		{
			/* allocate memory */
			if (ALLOCATE(curve_editor_dialog,
				struct Control_curve_editor_dialog,1))
			{
				/* initialise the structure */
				curve_editor_dialog->dialog_parent=parent;
				curve_editor_dialog->dialog_address=
					control_curve_editor_dialog_address;
				curve_editor_dialog->current_curve=(struct Control_curve *)NULL;
				curve_editor_dialog->control_curve_manager=control_curve_manager;
				curve_editor_dialog->control_curve_manager_callback_id=(void *)NULL;
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
						control_curve_editor_dialog_hierarchy,callback_list,
						XtNumber(callback_list)))
					{
						/* assign and register the identifiers */
						identifier_list[0].value=(XtPointer)curve_editor_dialog;
						if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
							control_curve_editor_dialog_hierarchy,identifier_list,
							XtNumber(identifier_list)))
						{
							/* fetch dialog widget */
							if (MrmSUCCESS==MrmFetchWidget(
								control_curve_editor_dialog_hierarchy,
								"curve_edd_widget",curve_editor_dialog->dialog,
								&(curve_editor_dialog->widget),
								&control_curve_editor_dialog_dialog_class))
							{
								XtManageChild(curve_editor_dialog->widget);
								init_widgets=1;
								/* create subwidgets */
								if (!CREATE_SELECT_WIDGET(Control_curve)(
									&curve_editor_dialog->select_widget,
									curve_editor_dialog->select_form,SELECT_LIST,
									curve_editor_dialog->current_curve,control_curve_manager))
								{
									display_message(ERROR_MESSAGE,
										"create_control_curve_editor_dialog.  "
										"Could not create select widget.");
									init_widgets=0;
								}
								if (!create_control_curve_editor_widget(
									&(curve_editor_dialog->editor_widget),
									curve_editor_dialog->editor_form,(struct Control_curve *)NULL,
									user_interface))
								{
									init_widgets=0;
								}
								if (init_widgets)
								{
									/* register for control curve manager callbacks */
									curve_editor_dialog->control_curve_manager_callback_id=
										MANAGER_REGISTER(Control_curve)(
											control_curve_editor_Control_curve_change,
											(void *)curve_editor_dialog,control_curve_manager);
									callback.procedure=
										control_curve_editor_dialog_update_selection;
									callback.data=(void *)curve_editor_dialog;
									SELECT_SET_UPDATE_CB(Control_curve)(
										curve_editor_dialog->select_widget,&callback);
									control_curve_editor_dialog_set_curve(
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
									"create_control_curve_editor_dialog.  "
									"Could not fetch control_curve_editor_dialog");
								DEALLOCATE(curve_editor_dialog);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"create_control_curve_editor_dialog.  "
								"Could not register identifiers");
							DEALLOCATE(curve_editor_dialog);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_control_curve_editor_dialog.  "
							"Could not register callbacks");
						DEALLOCATE(curve_editor_dialog);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_control_curve_editor_dialog.  "
						"Could not create popup shell");
					DEALLOCATE(curve_editor_dialog);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_control_curve_editor_dialog.  "
					"Could not allocate control_curve_editor_dialog");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_control_curve_editor_dialog.  Could not open hierarchy");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_control_curve_editor_dialog.  Invalid argument(s)");
	}
	if (control_curve_editor_dialog_address&&return_widget)
	{
		*control_curve_editor_dialog_address=return_widget;
	}
	LEAVE;

	return (return_widget);
} /* create_control_curve_editor_dialog */

/*
Global functions
----------------
*/
int control_curve_editor_dialog_set_curve(
	Widget control_curve_editor_dialog_widget,struct Control_curve *curve)
/*******************************************************************************
LAST MODIFIED : 8 November 1999

DESCRIPTION :
Sets the curve being edited by the curve editor dialog.
==============================================================================*/
{
	int num_children,return_code;
	struct Control_curve_editor_dialog *curve_editor_dialog;
	Widget *child_list;

	ENTER(control_curve_editor_dialog_set_curve);
	if (control_curve_editor_dialog_widget)
	{
		/* get the pointer to the curve for the dialog */
		XtVaGetValues(control_curve_editor_dialog_widget,
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
					if (!IS_MANAGED(Control_curve)(curve,
						curve_editor_dialog->control_curve_manager))
					{
						display_message(ERROR_MESSAGE,
							"control_curve_editor_dialog_set_curve.  Unmanaged curve");
						curve=(struct Control_curve *)NULL;
					}
				}
				if (!curve)
				{
					curve=FIRST_OBJECT_IN_MANAGER_THAT(Control_curve)(
						(MANAGER_CONDITIONAL_FUNCTION(Control_curve) *)NULL,(void *)NULL,
						curve_editor_dialog->control_curve_manager);
				}
				/* Must not ACCESS current curve - otherwise cannot be destroyed */
				curve_editor_dialog->current_curve=curve;
				SELECT_SET_SELECT_ITEM(Control_curve)(curve_editor_dialog->select_widget,
					curve_editor_dialog->current_curve);
				control_curve_editor_set_curve(curve_editor_dialog->editor_widget,
					curve_editor_dialog->current_curve);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"control_curve_editor_dialog_set_curve.  Missing dialog data");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"control_curve_editor_dialog_set_curve.  Invalid dialog");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"control_curve_editor_dialog_set_curve.  Missing dialog");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* control_curve_editor_dialog_set_curve */

int control_curve_editor_dialog_set_cursor_parameter(
	Widget control_curve_editor_dialog_widget,float parameter)
/*******************************************************************************
LAST MODIFIED : 8 November 1999

DESCRIPTION :
==============================================================================*/
{
	int num_children,return_code;
	struct Control_curve_editor_dialog *curve_editor_dialog;
	Widget *child_list;

	ENTER(control_curve_editor_dialog_set_cursor_parameter);
	if (control_curve_editor_dialog_widget)
	{
		/* get the pointer to the curve for the dialog */
		XtVaGetValues(control_curve_editor_dialog_widget,
			XmNchildren,&child_list,
			XmNnumChildren,&num_children,
			NULL);
		if (num_children==1)
		{
			XtVaGetValues(child_list[0],XmNuserData,&curve_editor_dialog,NULL);
			if (curve_editor_dialog)
			{
				control_curve_editor_set_cursor_parameter(curve_editor_dialog->editor_widget,
					parameter );
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"control_curve_editor_dialog_set_cursor_parameter.  Missing dialog data");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"control_curve_editor_dialog_set_cursor_parameter.  Invalid dialog");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"control_curve_editor_dialog_set_cursor_parameter.  Missing dialog");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* control_curve_editor_dialog_set_cursor_parameter */

int bring_up_control_curve_editor_dialog(
	Widget *control_curve_editor_dialog_address,Widget parent,
	struct MANAGER(Control_curve) *control_curve_manager,
	struct Control_curve *curve,struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 8 November 1999

DESCRIPTION :
If there is a curve editor dialog in existence, then bring it to the
front, else create a new one.
==============================================================================*/
{
	int return_code;

	ENTER(bring_up_control_curve_editor_dialog);
	if (control_curve_editor_dialog_address)
	{
		if (*control_curve_editor_dialog_address)
		{
			control_curve_editor_dialog_set_curve(
				*control_curve_editor_dialog_address,curve);
			XtPopup(*control_curve_editor_dialog_address,XtGrabNone);
			return_code=1;
		}
		else
		{
			if (create_control_curve_editor_dialog(
				control_curve_editor_dialog_address,parent,control_curve_manager,
				curve,user_interface))
			{
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"bring_up_control_curve_editor_dialog.  Error creating dialog");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"bring_up_control_curve_editor_dialog.  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* bring_up_control_curve_editor_dialog */

