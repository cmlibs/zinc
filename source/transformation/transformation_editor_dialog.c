/*******************************************************************************
FILE : transformation_editor_dialog.c

LAST MODIFIED : 19 October 1998

DESCRIPTION :
This module creates a free transformation_editor_dialog input device.
???DB.  At present just a shell around a transformation_editor
==============================================================================*/
#include <stdio.h>
#include "general/debug.h"
#include "io_devices/input_module.h"
#include "transformation/transformation_editor.h"
#include "transformation/transformation_editor_dialog.h"
#include "transformation/transformation_editor_dialog.uidh"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int transformation_editor_dialog_hierarchy_open=0;
static MrmHierarchy transformation_editor_dialog_hierarchy;
#endif /* defined (MOTIF) */

/*
Module functions
----------------
*/
static void transformation_editor_dialog_update(
	struct Transformation_editor_dialog_struct *temp_transformation_editor_dialog)
/*******************************************************************************
LAST MODIFIED : 7 September 1998

DESCRIPTION :
Tells CMGUI about the current values.
==============================================================================*/
{
	ENTER(transformation_editor_dialog_update);
	/* check arguments */
	if (temp_transformation_editor_dialog)
	{
		if (temp_transformation_editor_dialog->
			callback_array[TRANSFORMATION_EDITOR_DIALOG_UPDATE_CB].procedure)
		{
			/* now call the procedure with the user data and transformation */
			(temp_transformation_editor_dialog->
				callback_array[TRANSFORMATION_EDITOR_DIALOG_UPDATE_CB].procedure)
				(temp_transformation_editor_dialog->widget,
				temp_transformation_editor_dialog->
				callback_array[TRANSFORMATION_EDITOR_DIALOG_UPDATE_CB].data,
				temp_transformation_editor_dialog->current_value);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"transformation_editor_dialog_update.  Invalid argument");
	}
	LEAVE;
} /* transformation_editor_dialog_update */

static void transformation_editor_dialog_destroy_CB(Widget widget,int *tag,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 7 September 1998

DESCRIPTION :
Destroy callback for the transformation_editor_dialog dialog - tidies up all
details - mem etc
==============================================================================*/
{
	struct Transformation_editor_dialog_struct *temp_transformation_editor_dialog;

	ENTER(transformation_editor_dialog_destroy_CB);
	if (widget)
	{
		/* get the pointer to the data for the transformation_editor_dialog
			widget */
		XtVaGetValues(widget,XmNuserData,&temp_transformation_editor_dialog,NULL);
		if (temp_transformation_editor_dialog)
		{
			/* call the destroy callback if defined. Used chiefly to deallocate
				 data specific to each instance of the editor */
			if (temp_transformation_editor_dialog->
				callback_array[TRANSFORMATION_EDITOR_DIALOG_DESTROY_CB].procedure)
			{
				/* now call the procedure with the user data and transformation */
				(temp_transformation_editor_dialog->
					callback_array[TRANSFORMATION_EDITOR_DIALOG_DESTROY_CB].procedure)
					(temp_transformation_editor_dialog->widget,
						temp_transformation_editor_dialog->
						callback_array[TRANSFORMATION_EDITOR_DIALOG_DESTROY_CB].data,
						temp_transformation_editor_dialog->current_value);
			}
			/* deaccess the transformation_editor_dialog */
			if (temp_transformation_editor_dialog->dialog_address)
			{
				*(temp_transformation_editor_dialog->dialog_address) = (Widget)NULL;
			}
			DEALLOCATE(temp_transformation_editor_dialog);
		}
	}
	LEAVE;
} /* transformation_editor_dialog_destroy_CB */

static void transformation_editor_dialog_update_transformation(Widget widget,
	void *user_data,void *call_data)
/*******************************************************************************
LAST MODIFIED : 7 September 1998

DESCRIPTION :
Callback from transformation editor informing of change in transformation.
==============================================================================*/
{
	struct Cmgui_coordinate *new_transformation;
	struct Transformation_editor_dialog_struct *temp_transformation_editor_dialog;

	ENTER(transformation_editor_dialog_update_transformation);
	if ((temp_transformation_editor_dialog=
		(struct Transformation_editor_dialog_struct *)user_data)&&
		(new_transformation=(struct Cmgui_coordinate *)call_data))
	{
		temp_transformation_editor_dialog->current_value=new_transformation;
		/* inform the client of the changes */
		transformation_editor_dialog_update(temp_transformation_editor_dialog);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"transformation_editor_dialog_update_transformation.  "
			"Invalid argument(s)");
	}
	LEAVE;
} /* transformation_editor_dialog_update_transformation */

static Widget create_transformation_editor_dialog(
	Widget *transformation_editor_dialog_widget,Widget parent,
	struct Cmgui_coordinate *init_data,struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 13 December 1996

DESCRIPTION :
Creates a dialog widget that allows the user to edit the properties of a
graphics object.
==============================================================================*/
{
	int i;
	MrmType transformation_editor_dialog_dialog_class;
	static MrmRegisterArg callback_list[]=
	{
		{"trans_editor_d_destroy_CB",
			(XtPointer)transformation_editor_dialog_destroy_CB}
	};
	static MrmRegisterArg identifier_list[]=
	{
		{"trans_editor_d_structure",(XtPointer)NULL}
	};
	struct Callback_data callback;
	struct Transformation_editor_dialog_struct *temp_transformation_editor_dialog;
	Widget return_widget;

	ENTER(create_transformation_editor_dialog);
	/* check arguments */
	if (user_interface)
	{
		if (MrmOpenHierarchy_base64_string(transformation_editor_dialog_uidh,
			&transformation_editor_dialog_hierarchy,
			&transformation_editor_dialog_hierarchy_open))
		{
			/* allocate memory */
			if (ALLOCATE(temp_transformation_editor_dialog,
				struct Transformation_editor_dialog_struct,1))
			{
				/* initialise the structure */
				temp_transformation_editor_dialog->dialog_parent=parent;
				temp_transformation_editor_dialog->dialog_address=
					transformation_editor_dialog_widget;
				temp_transformation_editor_dialog->current_value=init_data;
				temp_transformation_editor_dialog->widget=(Widget)NULL;
				temp_transformation_editor_dialog->dialog=(Widget)NULL;
				temp_transformation_editor_dialog->editor_widget=(Widget)NULL;
				for(i=0;i<TRANSFORMATION_EDITOR_DIALOG_NUM_CALLBACKS;i++)
				{
					(temp_transformation_editor_dialog->callback_array[i]).procedure=
						(Callback_procedure *)NULL;
					(temp_transformation_editor_dialog->callback_array[i]).data=NULL;
				}
				/* make the dialog shell */
				if (temp_transformation_editor_dialog->dialog=XtVaCreatePopupShell(
					"Transformation Editor",topLevelShellWidgetClass,parent,
					XmNallowShellResize,TRUE,NULL))
					/*???DB.  Is TRUE correct here ? */
				{
					/* register the callbacks */
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
						transformation_editor_dialog_hierarchy,callback_list,
						XtNumber(callback_list)))
					{
						/* assign and register the identifiers */
						identifier_list[0].value=
							(XtPointer)temp_transformation_editor_dialog;
						if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
							transformation_editor_dialog_hierarchy,identifier_list,
							XtNumber(identifier_list)))
						{
							/* fetch transformation_editor widget */
							if (MrmSUCCESS==MrmFetchWidget(
								transformation_editor_dialog_hierarchy,
								"trans_editor_dialog_widge",
								temp_transformation_editor_dialog->dialog,
								&(temp_transformation_editor_dialog->widget),
								&transformation_editor_dialog_dialog_class))
							{
								XtManageChild(temp_transformation_editor_dialog->widget);
								if (create_transformation_editor_widget(
									&(temp_transformation_editor_dialog->editor_widget),
									temp_transformation_editor_dialog->widget,
									temp_transformation_editor_dialog->current_value))
								{
									XtRealizeWidget(temp_transformation_editor_dialog->dialog);
									XtPopup(temp_transformation_editor_dialog->dialog,XtGrabNone);
									/* ask for callbacks from transformation editor */
									callback.procedure=
										transformation_editor_dialog_update_transformation;
									callback.data=(void *)temp_transformation_editor_dialog;
									transformation_editor_set_data(
										temp_transformation_editor_dialog->editor_widget,
										TRANSFORMATION_EDITOR_UPDATE_CB,&callback);
#if defined (EXT_INPUT)
									/*???DB.  Is this needed ? */
									input_module_add_input_window(
										temp_transformation_editor_dialog->dialog,user_interface);
#endif
									return_widget=temp_transformation_editor_dialog->dialog;
								}
								else
								{
									display_message(ERROR_MESSAGE,
			"create_transformation_editor_dialog.  Could not create editor widget.");
									DEALLOCATE(temp_transformation_editor_dialog);
									return_widget=(Widget)NULL;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
								"create_transformation_editor_dialog.  Could not fetch dialog");
								DEALLOCATE(temp_transformation_editor_dialog);
								return_widget=(Widget)NULL;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
				"create_transformation_editor_dialog.  Could not register identifiers");
							DEALLOCATE(temp_transformation_editor_dialog);
							return_widget=(Widget)NULL;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
					"create_transformation_editor_dialog.  Could not register callbacks");
						DEALLOCATE(temp_transformation_editor_dialog);
						return_widget=(Widget)NULL;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
					"create_transformation_editor_dialog.  Could not create popup shell");
					DEALLOCATE(temp_transformation_editor_dialog);
					return_widget=(Widget)NULL;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_transformation_editor_dialog.  Insufficient memory");
				return_widget=(Widget)NULL;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_transformation_editor_dialog.  Could not open hierarchy");
			return_widget=(Widget)NULL;
		}
		if (transformation_editor_dialog_widget)
		{
			*transformation_editor_dialog_widget=return_widget;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_transformation_editor_dialog.  Missing user_interface");
	}
	LEAVE;

	return (return_widget);
} /* create_transformation_editor_dialog */

/*
Global functions
----------------
*/

int transformation_editor_dialog_set_data(
	Widget transformation_editor_dialog_widget,
	enum Transformation_editor_dialog_data_type data_type,void *data)
/*******************************************************************************
LAST MODIFIED : 7 September 1998

DESCRIPTION :
If <transformation_editor_dialog_widget> is not NULL, then change the data item
on <transformation_editor_dialog widget>.  Otherwise, change the data item on
<transformation_editor_dialog>.
==============================================================================*/
{
	int num_children,return_code;
	struct Transformation_editor_dialog_struct *temp_transformation_editor_dialog;
	Widget *child_list;

	ENTER(transformation_editor_dialog_set_data);
	if (transformation_editor_dialog_widget)
	{
		/* get the pointer to the data for the dialog */
		XtVaGetValues(transformation_editor_dialog_widget,
			XmNchildren,&child_list,
			XmNnumChildren,&num_children,
			NULL);
		if (1==num_children)
		{
			XtVaGetValues(child_list[0],
				XmNuserData,&temp_transformation_editor_dialog,
				NULL);
			switch (data_type)
			{
				case TRANSFORMATION_EDITOR_DIALOG_UPDATE_CB:
				{
					/*transformation_editor_set_data(temp_transformation_editor_dialog->
						editor_widget,TRANSFORMATION_EDITOR_UPDATE_CB,data);*/
          (temp_transformation_editor_dialog->
						callback_array)[TRANSFORMATION_EDITOR_DIALOG_UPDATE_CB].procedure=
						((struct Callback_data *)data)->procedure;
					(temp_transformation_editor_dialog->
						callback_array)[TRANSFORMATION_EDITOR_DIALOG_UPDATE_CB].data=
						((struct Callback_data *)data)->data;
					return_code=1;
				} break;
				case TRANSFORMATION_EDITOR_DIALOG_DESTROY_CB:
				{
					/*transformation_editor_set_data(temp_transformation_editor_dialog->
						editor_widget,TRANSFORMATION_EDITOR_DESTROY_CB,data);*/
          (temp_transformation_editor_dialog->
						callback_array)[TRANSFORMATION_EDITOR_DIALOG_DESTROY_CB].procedure=
						((struct Callback_data *)data)->procedure;
					(temp_transformation_editor_dialog->
						callback_array)[TRANSFORMATION_EDITOR_DIALOG_DESTROY_CB].data=
						((struct Callback_data *)data)->data;
					return_code=1;
				} break;
				case TRANSFORMATION_EDITOR_DIALOG_DATA:
				{
					temp_transformation_editor_dialog->current_value=
						(struct Cmgui_coordinate *)data;
					transformation_editor_set_data(
						temp_transformation_editor_dialog->editor_widget,
						TRANSFORMATION_EDITOR_DATA,
						temp_transformation_editor_dialog->current_value);
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"transformation_editor_dialog_set_data.  Invalid data type");
					return_code=0;
				} break;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"transformation_editor_dialog_set_data.  Invalid dialog");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"transformation_editor_dialog_set_data.  Missing dialog");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* transformation_editor_dialog_set_data */

void *transformation_editor_dialog_get_data(
	Widget transformation_editor_dialog_widget,
	enum Transformation_editor_dialog_data_type data_type)
/*******************************************************************************
LAST MODIFIED : 7 September 1998

DESCRIPTION :
If <transformation_editor_dialog_widget> is not NULL, then get the data item
from <transformation_editor_dialog widget>.  Otherwise, get the data item from
<transformation_editor_dialog>.
==============================================================================*/
{
	int num_children;
	static struct Callback_data callback;
	static Widget dat_widget;
	struct Transformation_editor_dialog_struct *temp_transformation_editor_dialog;
	Widget *child_list;
	void *data;

	ENTER(transformation_editor_dialog_get_data);
	if (transformation_editor_dialog_widget)
	{
		/* get the pointer to the data for the transformation_editor_dialog
			dialog */
		XtVaGetValues(transformation_editor_dialog_widget,
			XmNchildren,&child_list,
			XmNnumChildren,&num_children,
			NULL);
		if (1==num_children)
		{
			XtVaGetValues(child_list[0],XmNuserData,
				&temp_transformation_editor_dialog,NULL);
			switch (data_type)
			{
				case TRANSFORMATION_EDITOR_DIALOG_UPDATE_CB:
				{
					callback.procedure=
						(temp_transformation_editor_dialog->callback_array)
						[TRANSFORMATION_EDITOR_DIALOG_UPDATE_CB].procedure;
					callback.data=(temp_transformation_editor_dialog->callback_array)
						[TRANSFORMATION_EDITOR_DIALOG_UPDATE_CB].data;
					/*???DB.  Dangerous ? */
					data=(void *)(&callback);
				} break;
				case TRANSFORMATION_EDITOR_DIALOG_DESTROY_CB:
				{
					callback.procedure=
						(temp_transformation_editor_dialog->callback_array)
						[TRANSFORMATION_EDITOR_DIALOG_DESTROY_CB].procedure;
					callback.data=(temp_transformation_editor_dialog->callback_array)
						[TRANSFORMATION_EDITOR_DIALOG_DESTROY_CB].data;
					/*???DB.  Dangerous ? */
					data=(void *)(&callback);
				} break;
				case TRANSFORMATION_EDITOR_DIALOG_DATA:
				{
					data=(void *)(temp_transformation_editor_dialog->current_value);
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"transformation_editor_dialog_get_data.  Invalid data type");
					data=(void *)NULL;
				} break;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"transformation_editor_dialog_get_data.  Invalid dialog");
			data=NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"transformation_editor_dialog_get_data.  Missing dialog");
		data=NULL;
	}
	LEAVE;

	return (data);
} /* transformation_editor_dialog_get_data */

int bring_up_transformation_editor_dialog(
	Widget *transformation_editor_dialog_address,Widget parent,
	struct Cmgui_coordinate *init_data,struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 23 June 1996

DESCRIPTION :
If there is a transformation editor dialog in existence, then bring it to the
front, otherwise it creates a new one.
???DB.  Assumes we will only ever want one transformation editor at a time.
	This may be changed later.
==============================================================================*/
{
	int return_code;

	ENTER(bring_up_transformation_editor_dialog);
	/* check arguments */
	if (transformation_editor_dialog_address&&user_interface)
	{
		/* does it exist */
		if (*transformation_editor_dialog_address)
		{
			transformation_editor_dialog_set_data(
				*transformation_editor_dialog_address,TRANSFORMATION_EDITOR_DIALOG_DATA,
				init_data);
			XtPopup(*transformation_editor_dialog_address,XtGrabNone);
		}
		else
		{
			if (create_transformation_editor_dialog(
				transformation_editor_dialog_address,user_interface->application_shell,
				init_data,user_interface))
			{
				return_code=1;
			}
			else
			{
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"bring_up_transformation_editor_dialog.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* bring_up_transformation_editor_dialog */
