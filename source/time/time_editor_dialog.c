/*******************************************************************************
FILE : time_editor_dialog.c

LAST MODIFIED : 24 November 1999

DESCRIPTION :
This module creates a time_editor_dialog.
==============================================================================*/
#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/Protocols.h>
#include "general/callback.h"
#include "general/debug.h"
#include "time/time_keeper.h"
#include "time/time_editor.h"
#include "time/time_editor_dialog.h"
#include "time/time_editor_dialog.uidh"
#include "user_interface/message.h"

/*
Module Constants
----------------
*/
/* UIL Identifiers */
#define time_editor_dialog_editor_form_ID 1

/*
Global Types
------------
*/
struct Time_editor_dialog_struct
/*******************************************************************************
LAST MODIFIED : 13 December 2001

DESCRIPTION :
Contains all the information carried by the time_editor_dialog widget.
Note that we just hold a pointer to the time_editor_dialog.
==============================================================================*/
{
	struct Callback_data update_callback;
	/*store the address of pointer, so can set to null from callback*/
	struct Time_editor_dialog_struct **time_editor_dialog_address;
	struct Time_keeper *current_value;
	Widget editor_form, editor_widget;
	Widget dialog,widget,dialog_parent;
	struct User_interface *user_interface;
}; /* time_editor_dialog_struct */

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int time_editor_dialog_hierarchy_open=0;
static MrmHierarchy time_editor_dialog_hierarchy;
#endif /* defined (MOTIF) */

/*
Module functions
----------------
*/

static void time_editor_dialog_identify_widget(Widget widget,int widget_num,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 24 November 1999

DESCRIPTION :
Finds the id of the widgets on the time_editor_dialog widget.
==============================================================================*/
{
	struct Time_editor_dialog_struct *temp_time_editor_dialog;

	ENTER(time_editor_dialog_identify_widget);
	USE_PARAMETER(reason);
	/* find out which time_editor_dialog widget we are in */
	XtVaGetValues(widget, XmNuserData,&temp_time_editor_dialog,NULL);
	switch (widget_num)
	{
		case time_editor_dialog_editor_form_ID:
		{
			temp_time_editor_dialog->editor_form = widget;
		} break;
		default:
		{
			display_message(WARNING_MESSAGE,
				"time_editor_dialog_identify_widget.  Invalid widget number");
		} break;
	}
	LEAVE;
} /* time_editor_dialog_identify_widget */

int  time_editor_dialog_destroy(
	struct Time_editor_dialog_struct *time_editor_dialog)
/*******************************************************************************
LAST MODIFIED : 13 December 2001

DESCRIPTION :Destroys the <time_editor_dialog> - tidies up all details - mem etc
==============================================================================*/
{
	int return_code;

	ENTER(time_editor_dialog_destroy);	
	if(time_editor_dialog)
	{
		return_code=1;
		destroy_Shell_list_item_from_shell(&time_editor_dialog->dialog,
			time_editor_dialog->user_interface);
		DESTROY(Time_editor)(&time_editor_dialog->editor_widget);
		/* deaccess the time_editor_dialog */
		XtDestroyWidget(time_editor_dialog->dialog);
		time_editor_dialog->dialog=(Widget)NULL;	
		/* set the pointer to the time_editor_dialog to NULL */
		if (time_editor_dialog->time_editor_dialog_address)
		{
			*(time_editor_dialog->time_editor_dialog_address)=
				(struct Time_editor_dialog_struct *)NULL;
		}		
		/* deallocate the memory for the user data */
		DEALLOCATE(time_editor_dialog);	
		LEAVE;
	}
	else
	{	
		return_code=0;
		display_message(WARNING_MESSAGE,
				"time_editor_dialog_destroy. Invalid argument");
	}
	return(return_code);
}/* time_editor_dialog_destroy */


static void time_editor_dialog_close_CB(Widget caller,
	void *time_editor_dialog_void,void *cbs)
/*******************************************************************************
LAST MODIFIED : 13 December 2001

DESCRIPTION :
Callback for the time_editor_dialog dialog
==============================================================================*/
{
	struct Time_editor_dialog_struct *time_editor_dialog;

	ENTER(time_editor_dialog_close_CB);
	USE_PARAMETER(caller);
	USE_PARAMETER(cbs);
	if (time_editor_dialog=(struct Time_editor_dialog_struct *)time_editor_dialog_void)
	{	
		time_editor_dialog_destroy(time_editor_dialog);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"time_editor_dialog_close_CB.  Missing time_editor_dialog");
	}
	LEAVE;
} /* time_editor_dialog_close_CB */

int create_time_editor_dialog(
	struct Time_editor_dialog_struct **address,Widget parent,
	struct Time_keeper *time_keeper,struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 13 December 2001

DESCRIPTION :
Creates a dialog widget that allows the user to edit time.
==============================================================================*/
{
	Atom WM_DELETE_WINDOW;
	int init_widgets,return_code;
	MrmType time_editor_dialog_dialog_class;
	struct Time_editor_dialog_struct *time_editor_dialog=NULL;
	static MrmRegisterArg callback_list[]=
	{		
		{"time_editor_d_identify_widget",
			(XtPointer)time_editor_dialog_identify_widget}
	};
	static MrmRegisterArg identifier_list[]=
	{

		{"time_editor_d_structure",(XtPointer)NULL},
		{"time_editor_d_editor_form_ID",
		 (XtPointer)time_editor_dialog_editor_form_ID}
	};

	ENTER(create_time_editor_dialog);
	return_code=0;
	if (parent && user_interface)
	{
		if (MrmOpenHierarchy_base64_string(time_editor_dialog_uidh,
			&time_editor_dialog_hierarchy,&time_editor_dialog_hierarchy_open))
		{
			/* allocate memory */
			if (ALLOCATE(time_editor_dialog,
				struct Time_editor_dialog_struct,1))
			{
				/* initialise the structure */
				time_editor_dialog->time_editor_dialog_address=address;
				if(address)
				{
					*address=time_editor_dialog;
				}
				time_editor_dialog->dialog_parent=parent;				
				/* current_value set in time_editor_dialog_set_time */
				time_editor_dialog->current_value=
					(struct Time_keeper *)NULL;
				time_editor_dialog->widget=(Widget)NULL;
				time_editor_dialog->dialog=(Widget)NULL;
				time_editor_dialog->update_callback.procedure=
					(Callback_procedure *)NULL;
				time_editor_dialog->update_callback.data=NULL;
				time_editor_dialog->user_interface = user_interface;
				/* make the dialog shell */
				if (time_editor_dialog->dialog=XtVaCreatePopupShell(
					"Time Editor",topLevelShellWidgetClass,parent,XmNallowShellResize,
					TRUE,NULL))
				{
					/* register the callbacks */
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
						time_editor_dialog_hierarchy,callback_list,
						XtNumber(callback_list)))
					{
						/* assign and register the identifiers */
						identifier_list[0].value=(XtPointer)time_editor_dialog;
						if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
							time_editor_dialog_hierarchy,identifier_list,
							XtNumber(identifier_list)))
						{
							/* fetch position window widget */
							if (MrmSUCCESS==MrmFetchWidget(time_editor_dialog_hierarchy,
								"time_editor_dialog_widget",time_editor_dialog->dialog,
								&(time_editor_dialog->widget),
								&time_editor_dialog_dialog_class))
							{
								XtManageChild(time_editor_dialog->widget);
								/* set the mode toggle to the correct position */
								init_widgets=1;
								if (!CREATE(Time_editor)(
									&(time_editor_dialog->editor_widget),
									time_editor_dialog->editor_form,
									(struct Time_keeper *)NULL,user_interface))
								{
									display_message(ERROR_MESSAGE,
						"create_time_editor_dialog.  Could not create editor widget.");
									init_widgets=0;
								}
								if (init_widgets)
								{
									/* set current_value to init_data */
									if(time_keeper)
									{
										time_editor_dialog_set_time_keeper(
											time_editor_dialog->dialog, time_keeper);
									}
									XtRealizeWidget(time_editor_dialog->dialog);
									XtPopup(time_editor_dialog->dialog, XtGrabNone);									
									create_Shell_list_item(&time_editor_dialog->dialog,
										user_interface);
									/* Set up window manager callback for close window message */
									WM_DELETE_WINDOW=XmInternAtom(
										XtDisplay(time_editor_dialog->dialog),"WM_DELETE_WINDOW",False);
									XmAddWMProtocolCallback(time_editor_dialog->dialog,
										WM_DELETE_WINDOW,time_editor_dialog_close_CB,time_editor_dialog);	
									return_code=1;
								}
								else
								{
									DEALLOCATE(time_editor_dialog);
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
			"create_time_editor_dialog.  Could not fetch time_editor_dialog");
								DEALLOCATE(time_editor_dialog);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
							"create_time_editor_dialog.  Could not register identifiers");
							DEALLOCATE(time_editor_dialog);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_time_editor_dialog.  Could not register callbacks");
						DEALLOCATE(time_editor_dialog);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_time_editor_dialog.  Could not create popup shell.");
					DEALLOCATE(time_editor_dialog);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
	"create_time_editor_dialog.  Could not allocate time_editor_dialog");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_time_editor_dialog.  Could not open hierarchy");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_time_editor_dialog.  Invalid argument(s)");
	}	
	LEAVE;

	return (return_code);
} /* create_time_editor_dialog */

/*
Global functions
----------------
*/
int time_editor_dialog_get_callback(Widget time_editor_dialog_widget,
	struct Callback_data *callback)
/*******************************************************************************
LAST MODIFIED : 8 December 1998

DESCRIPTION :
Returns the update_callback for the time editor_dialog widget.
==============================================================================*/
{
	int return_code,num_children;
	struct Time_editor_dialog_struct *time_editor_dialog;
	Widget *child_list;

	ENTER(time_editor_dialog_get_callback);
	/* check arguments */
	if (time_editor_dialog_widget&&callback)
	{
		/* get the pointer to the data for the dialog */
		XtVaGetValues(time_editor_dialog_widget,
			XmNchildren,&child_list,
			XmNnumChildren,&num_children,
			NULL);
		if (num_children==1)
		{
			XtVaGetValues(child_list[0],XmNuserData,&time_editor_dialog,
				NULL);
			if (time_editor_dialog)
			{
				callback->procedure=
					time_editor_dialog->update_callback.procedure;
				callback->data=time_editor_dialog->update_callback.data;
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"time_editor_dialog_get_callback.  Missing widget data");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"time_editor_dialog_get_callback.  Invalid dialog");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"time_editor_dialog_get_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* time_editor_dialog_get_callback */

int time_editor_dialog_set_callback(Widget time_editor_dialog_widget,
	struct Callback_data *callback)
/*******************************************************************************
LAST MODIFIED : 8 December 1998

DESCRIPTION :
Changes the update_callback for the time editor_dialog widget.
==============================================================================*/
{
	int return_code,num_children;
	struct Time_editor_dialog_struct *time_editor_dialog;
	Widget *child_list;

	ENTER(time_editor_dialog_set_callback);
	if (time_editor_dialog_widget&&callback)
	{
		/* get the pointer to the data for the dialog */
		XtVaGetValues(time_editor_dialog_widget,
			XmNchildren,&child_list,
			XmNnumChildren,&num_children,
			NULL);
		if (num_children==1)
		{
			XtVaGetValues(child_list[0],XmNuserData,&time_editor_dialog,
				NULL);
			if (time_editor_dialog)
			{
				time_editor_dialog->update_callback.procedure=
					callback->procedure;
				time_editor_dialog->update_callback.data=callback->data;
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"time_editor_dialog_set_callback.  Missing widget data");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"time_editor_dialog_set_callback.  Invalid dialog");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"time_editor_dialog_set_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* time_editor_dialog_set_callback */

struct Time_keeper *time_editor_dialog_get_time_keeper(
	Widget time_editor_dialog_widget)
/*******************************************************************************
LAST MODIFIED : 8 December 1998

DESCRIPTION :
If <time_editor_dialog_widget> is not NULL, then get the data item from
<time_editor_dialog widget>.  Otherwise, get the data item from
<time_editor_dialog>.
==============================================================================*/
{
	int num_children;
	struct Time_editor_dialog_struct *time_editor_dialog;
	struct Time_keeper *return_time_keeper;
	Widget *child_list;

	ENTER(time_editor_dialog_get_time_keeper);
	if (time_editor_dialog_widget)
	{
		XtVaGetValues(time_editor_dialog_widget,
			XmNchildren,&child_list,
			XmNnumChildren,&num_children,
			NULL);
		if (num_children==1)
		{
			XtVaGetValues(child_list[0],XmNuserData,&time_editor_dialog,
				NULL);
			if (time_editor_dialog)
			{
				return_time_keeper = time_editor_dialog->current_value;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"time_editor_dialog_get_time_keeper.  Missing widget data");
				return_time_keeper=(struct Time_keeper *)NULL;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"time_editor_dialog_get_time_keeper.  Invalid dialog");
			return_time_keeper=(struct Time_keeper *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"time_editor_dialog_get_time.  Missing dialog");
		return_time_keeper=(struct Time_keeper *)NULL;
	}
	LEAVE;

	return (return_time_keeper);
} /* time_editor_dialog_get_time_keeper */

int time_editor_dialog_set_time_keeper(Widget time_editor_dialog_widget,
	struct Time_keeper *time_keeper)
/*******************************************************************************
LAST MODIFIED : 8 December 1998

DESCRIPTION :
If <time_editor_dialog_widget> is not NULL, then change the data item on
<time_editor_dialog widget>.  Otherwise, change the data item on
<time_editor_dialog>.
==============================================================================*/
{
	int num_children,return_code;
	struct Time_editor_dialog_struct *time_editor_dialog;
	Widget *child_list;

	ENTER(time_editor_dialog_set_time);
	if (time_editor_dialog_widget)
	{
		/* get the pointer to the data for the dialog */
		XtVaGetValues(time_editor_dialog_widget,
			XmNchildren,&child_list,
			XmNnumChildren,&num_children,
			NULL);
		if (num_children==1)
		{
			XtVaGetValues(child_list[0],XmNuserData,&time_editor_dialog,
				NULL);
			if (time_editor_dialog)
			{
				return_code=1;
				time_editor_dialog->current_value = time_keeper;
				time_editor_set_time_keeper(
					time_editor_dialog->editor_widget,
					time_editor_dialog->current_value);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"time_editor_dialog_set_time_keeper.  Missing widget data");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"time_editor_dialog_set_time_keeper.  Invalid dialog");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"time_editor_dialog_set_time_keeper.  Missing dialog");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* time_editor_dialog_set_time_keeper */

int bring_up_time_editor_dialog(
	struct Time_editor_dialog_struct **the_time_editor_dialog,
	Widget parent, struct Time_keeper *time_keeper,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 13 December 2001

DESCRIPTION :
If there is a time_editor dialog in existence, then bring it to the front,
else create a new one.
==============================================================================*/
{
	int return_code;
	struct Time_editor_dialog_struct *time_editor_dialog;

	ENTER(bring_up_time_editor_dialog);
	if (the_time_editor_dialog&&time_keeper&&user_interface)
	{
		if ((time_editor_dialog=*the_time_editor_dialog)&&time_editor_dialog->dialog)
		{
			time_editor_dialog_set_time_keeper(time_editor_dialog->dialog,
				time_keeper);
			XtPopup(time_editor_dialog->dialog,XtGrabNone);
			return_code=1;
		}
		else
		{
			if (create_time_editor_dialog(the_time_editor_dialog,parent,
				time_keeper, user_interface))
			{
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"bring_up_time_editor_dialog.  Error creating dialog");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"bring_up_time_editor_dialog.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* bring_up_time_editor_dialog */
