/*******************************************************************************
FILE : node_editor_dialog.c

LAST MODIFIED : 13 May 1997

DESCRIPTION :
This module creates a free node_editor_dialog input device, using two dof3,
two control and one input widget.  The position is given relative to some
coordinate system, and the returned value is a global one.
==============================================================================*/
#include <stdio.h>
#include "general/debug.h"
#include "node/node_editor.h"
#include "node/node_editor_dialog.h"
#include "node/node_editor_dialog.uidh"
#include "select/select_finite_element.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int node_editor_dialog_hierarchy_open=0;
static MrmHierarchy node_editor_dialog_hierarchy;
#endif /* defined (MOTIF) */

/*
Module functions
----------------
*/
static void node_editor_dialog_update(
	struct Node_editor_dialog_struct *temp_node_editor_dialog)
/*******************************************************************************
LAST MODIFIED : 26 September 1995

DESCRIPTION :
Tells CMGUI about the current values. Returns a pointer to the material.
==============================================================================*/
{
	ENTER(node_editor_dialog_update);
	if (temp_node_editor_dialog->callback_array[NODE_EDITOR_DIALOG_UPDATE_CB].
		procedure)
	{
		/* now call the procedure with the user data and the position data */
		(temp_node_editor_dialog->callback_array[NODE_EDITOR_DIALOG_UPDATE_CB].
			procedure)(temp_node_editor_dialog->widget,
				temp_node_editor_dialog->callback_array[NODE_EDITOR_DIALOG_UPDATE_CB].
				data,temp_node_editor_dialog->current_value);
	}
	LEAVE;
} /* node_editor_dialog_update */

static void node_editor_dialog_update_selection(Widget select_widget,
	void *user_data,void *temp_nod)
/*******************************************************************************
LAST MODIFIED : 26 September 1995

DESCRIPTION :
Finds which material is selected, and informs the editor widget.
???DB.  Not checking arguments ?
???GMH. The new FE_node may be NULL.  Should really check userdata.
==============================================================================*/
{
	struct Node_editor_dialog_struct *temp_node_editor_dialog=user_data;
	struct FE_node *temp_node=temp_nod;

	ENTER(node_editor_dialog_update_selection);
	if(temp_node_editor_dialog)
	{
		temp_node_editor_dialog->current_value=temp_node;
		node_editor_set_data(temp_node_editor_dialog->editor_widget,
			NODE_EDITOR_DATA,temp_node_editor_dialog->current_value);
	}
	else
	{
		display_message(WARNING_MESSAGE,
			"node_editor_dialog_update_selection.  Invalid dialog.");
	}
	LEAVE;
} /* node_editor_dialog_update_selection */

static void node_editor_dialog_identify_button(Widget w,int button_num,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 18 April 1994

DESCRIPTION :
Finds the id of the buttons on the node_editor_dialog widget.
==============================================================================*/
{
	struct Node_editor_dialog_struct *temp_node_editor_dialog;

	ENTER(node_editor_dialog_identify_button);
	/* find out which node_editor_dialog widget we are in */
	XtVaGetValues(w,XmNuserData,&temp_node_editor_dialog,NULL);
	switch (button_num)
	{
		case node_editor_dialog_editor_form_ID:
		{
			temp_node_editor_dialog->editor_form=w;
		}; break;
		case node_editor_dialog_select_form_ID:
		{
			temp_node_editor_dialog->select_form=w;
		}; break;
		case node_editor_dialog_ok_ID:
		{
			temp_node_editor_dialog->ok_button=w;
		}; break;
		case node_editor_dialog_apply_ID:
		{
			temp_node_editor_dialog->apply_button=w;
		}; break;
		case node_editor_dialog_revert_ID:
		{
			temp_node_editor_dialog->revert_button=w;
		}; break;
		case node_editor_dialog_cancel_ID:
		{
			temp_node_editor_dialog->cancel_button=w;
		}; break;
		default:
		{
			display_message(WARNING_MESSAGE,
				"node_editor_dialog_identify_button.  Invalid button number");
		}; break;
	}
	LEAVE;
} /* node_editor_dialog_identify_button */

static void node_editor_dialog_control_CB(Widget w,int button_num,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 18 April 1994

DESCRIPTION :
Finds the id of the buttons on the node_editor_dialog widget.
==============================================================================*/
{
	struct Node_editor_dialog_struct *temp_node_editor_dialog;

	ENTER(node_editor_dialog_control_CB);
	/* find out which node_editor_dialog widget we are in */
	XtVaGetValues(w,XmNuserData,&temp_node_editor_dialog,NULL);
	if ((button_num==node_editor_dialog_cancel_ID)||
		(button_num==node_editor_dialog_revert_ID))
	{
		node_editor_set_data(temp_node_editor_dialog->editor_widget,
			NODE_EDITOR_DATA,temp_node_editor_dialog->current_value);
	}
	/* no matter what, we have to do an update */
	node_editor_modify_global(temp_node_editor_dialog->editor_widget);
	node_editor_dialog_update(temp_node_editor_dialog);
	/* if it is an OK or a cancel, we have to kill the dialog */
	if ((button_num==node_editor_dialog_ok_ID)||
		(button_num==node_editor_dialog_cancel_ID))
	{
		XtDestroyWidget(temp_node_editor_dialog->dialog);
	}
	LEAVE;
} /* node_editor_dialog_control_CB */

static void node_editor_dialog_destroy_CB(Widget w,int *tag,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Callback for the node_editor_dialog dialog - tidies up all details - mem etc
==============================================================================*/
{
	struct Node_editor_dialog_struct *temp_node_editor_dialog;

	ENTER(node_editor_dialog_destroy_CB);
	/* Get the pointer to the data for the node_editor_dialog widget */
	XtVaGetValues(w,XmNuserData,&temp_node_editor_dialog,NULL);
	/* deaccess the node_editor_dialog */
	*(temp_node_editor_dialog->dialog_address)=(Widget)NULL;
	/* deallocate the memory for the user data */
	DEALLOCATE(temp_node_editor_dialog);
	LEAVE;
} /* node_editor_dialog_destroy_CB */

static Widget create_node_editor_dialog(char *name,
	Widget *node_editor_dialog_widget,Widget parent,
	struct MANAGER(FE_node) *manager,struct FE_node *init_data)
/*******************************************************************************
LAST MODIFIED : 6 November 1998

DESCRIPTION :
Creates a dialog widget that allows the user to edit the properties of any of
the materials contained in the global list.
==============================================================================*/
{
	int i,init_widgets;
	MrmType node_editor_dialog_dialog_class;
	struct Callback_data callback;
	struct Node_editor_dialog_struct *temp_node_editor_dialog=NULL;
	static MrmRegisterArg callback_list[]=
	{
		{"node_editor_d_identify_button",
		(XtPointer)node_editor_dialog_identify_button},
		{"node_editor_d_destroy_CB",
		(XtPointer)node_editor_dialog_destroy_CB},
		{"node_editor_d_control_CB",
		(XtPointer)node_editor_dialog_control_CB}
	};
	static MrmRegisterArg identifier_list[]=
	{
		{"Node_editor_d_structure",(XtPointer)NULL},
		{"node_editor_d_editor_form_ID",
		(XtPointer)node_editor_dialog_editor_form_ID},
		{"node_editor_d_select_form_ID",
		(XtPointer)node_editor_dialog_select_form_ID},
		{"node_editor_d_ok_ID",(XtPointer)node_editor_dialog_ok_ID},
		{"node_editor_d_apply_ID",(XtPointer)node_editor_dialog_apply_ID},
		{"node_editor_d_revert_ID",(XtPointer)node_editor_dialog_revert_ID},
		{"node_editor_d_cancel_ID",(XtPointer)node_editor_dialog_cancel_ID}
	};
	Widget return_widget;

	ENTER(create_node_editor_dialog);
	return_widget=(Widget)NULL;
	/* check arguments */
	if (manager)
	{
		if (MrmOpenHierarchy_base64_string(node_editor_dialog_uidh,
			&node_editor_dialog_hierarchy,&node_editor_dialog_hierarchy_open))
		{
			/* allocate memory */
			if (ALLOCATE(temp_node_editor_dialog,
				struct Node_editor_dialog_struct,1))
			{
				/* initialise the structure */
				temp_node_editor_dialog->dialog_parent=parent;
				temp_node_editor_dialog->dialog_address=node_editor_dialog_widget;
				if (init_data)
				{
					temp_node_editor_dialog->current_value=init_data;
				}
				else
				{
					temp_node_editor_dialog->current_value=FIRST_OBJECT_IN_MANAGER_THAT(
						FE_node)((MANAGER_CONDITIONAL_FUNCTION(FE_node) *)NULL,(void *)NULL,
						manager);
				}
				temp_node_editor_dialog->manager=manager;
				temp_node_editor_dialog->widget=(Widget)NULL;
				temp_node_editor_dialog->dialog=(Widget)NULL;
				temp_node_editor_dialog->select_form=(Widget)NULL;
				temp_node_editor_dialog->select_widget=(Widget)NULL;
				temp_node_editor_dialog->editor_form=(Widget)NULL;
				temp_node_editor_dialog->editor_widget=(Widget)NULL;
				temp_node_editor_dialog->ok_button=(Widget)NULL;
				temp_node_editor_dialog->apply_button=(Widget)NULL;
				temp_node_editor_dialog->revert_button=(Widget)NULL;
				temp_node_editor_dialog->cancel_button=(Widget)NULL;
				for (i=0;i<NODE_EDITOR_DIALOG_NUM_CALLBACKS;i++)
				{
					temp_node_editor_dialog->callback_array[i].procedure=
						(Callback_procedure *)NULL;
					temp_node_editor_dialog->callback_array[i].data=NULL;
				}
				/* make the dialog shell */
				if (name)
				{
					temp_node_editor_dialog->dialog=XtVaCreatePopupShell(name,
						topLevelShellWidgetClass,parent,XmNallowShellResize,TRUE,NULL);
				}
				else
				{
					temp_node_editor_dialog->dialog=XtVaCreatePopupShell("Node Editor",
						topLevelShellWidgetClass,parent,XmNallowShellResize,TRUE,NULL);
				}
				if (temp_node_editor_dialog->dialog)
				{
					/* register the callbacks */
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
						node_editor_dialog_hierarchy,callback_list,XtNumber(callback_list)))
					{
						/* assign and register the identifiers */
						identifier_list[0].value=(XtPointer)temp_node_editor_dialog;
						if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
							node_editor_dialog_hierarchy,identifier_list,
							XtNumber(identifier_list)))
						{
							/* fetch position window widget */
							if (MrmSUCCESS==MrmFetchWidget(node_editor_dialog_hierarchy,
								"node_editor_dialog_widget",temp_node_editor_dialog->dialog,
								&(temp_node_editor_dialog->widget),
								&node_editor_dialog_dialog_class))
							{
								XtManageChild(temp_node_editor_dialog->widget);
								/* set the mode toggle to the correct position */
								init_widgets=1;
								if (!CREATE_SELECT_WIDGET(FE_node)(
									&temp_node_editor_dialog->select_widget,
									temp_node_editor_dialog->select_form,
									SELECT_TEXT,
									temp_node_editor_dialog->current_value,manager))
								{
									display_message(ERROR_MESSAGE,
								"create_node_editor_dialog.  Could not create select widget.");
									init_widgets=0;
								}
								if (!create_node_editor_widget(
									&temp_node_editor_dialog->editor_widget,
									temp_node_editor_dialog->editor_form,manager,
									temp_node_editor_dialog->current_value))
								{
									display_message(ERROR_MESSAGE,
								"create_node_editor_dialog.  Could not create editor widget.");
									init_widgets=0;
								}
								if (init_widgets)
								{
									callback.procedure=node_editor_dialog_update_selection;
									callback.data=temp_node_editor_dialog;
									SELECT_SET_UPDATE_CB(FE_node)(
										temp_node_editor_dialog->select_widget,&callback);
									XtRealizeWidget(temp_node_editor_dialog->dialog);
									XtPopup(temp_node_editor_dialog->dialog, XtGrabNone);
									return_widget=temp_node_editor_dialog->dialog;
								}
								else
								{
									DEALLOCATE(temp_node_editor_dialog);
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
							"create_node_editor_dialog.  Could not fetch node_editor_dialog");
								DEALLOCATE(temp_node_editor_dialog);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"create_node_editor_dialog.  Could not register identifiers");
							DEALLOCATE(temp_node_editor_dialog);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_node_editor_dialog.  Could not register callbacks");
						DEALLOCATE(temp_node_editor_dialog);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_node_editor_dialog.  Could not create popup shell.");
					DEALLOCATE(temp_node_editor_dialog);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
		"create_node_editor_dialog.  Could not allocate node_editor_dialog");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_node_editor_dialog.  Could not open hierarchy");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_node_editor_dialog.  Invalid argument(s)");
	}
	if (node_editor_dialog_widget&&return_widget)
	{
		*node_editor_dialog_widget=return_widget;
	}
	LEAVE;

	return (return_widget);
} /* create_node_editor_dialog */

/*
Global functions
----------------
*/
int node_editor_dialog_set_data(Widget node_editor_dialog_widget,
	enum Node_editor_dialog_data_type data_type,void *data)
/*******************************************************************************
LAST MODIFIED : 21 June 1996

DESCRIPTION :
If <node_editor_dialog_widget> is not NULL, then change the data item on
<node_editor_dialog widget>.  Otherwise, change the data item on
<node_editor_dialog>.
==============================================================================*/
{
	int num_children,return_code;
	struct Node_editor_dialog_struct *temp_node_editor_dialog;
	Widget *child_list;

	ENTER(node_editor_dialog_set_data);
	if (node_editor_dialog_widget)
	{
		/* get the pointer to the data for the dialog */
		XtVaGetValues(node_editor_dialog_widget,
			XmNchildren,&child_list,
			XmNnumChildren,&num_children,
			NULL);
		if (num_children==1)
		{
			XtVaGetValues(child_list[0],
				XmNuserData,&temp_node_editor_dialog,
				NULL);
			switch (data_type)
			{
				case NODE_EDITOR_DIALOG_UPDATE_CB:
				{
					temp_node_editor_dialog->
						callback_array[NODE_EDITOR_DIALOG_UPDATE_CB].procedure=
						((struct Callback_data *)data)->procedure;
					temp_node_editor_dialog->
						callback_array[NODE_EDITOR_DIALOG_UPDATE_CB].data=
						((struct Callback_data *)data)->data;
					return_code=1;
				} break;
				case NODE_EDITOR_DIALOG_DATA:
				{
					if (data)
					{
						temp_node_editor_dialog->current_value=
							(struct FE_node *)data;
						if (!IS_MANAGED(FE_node)(temp_node_editor_dialog->current_value,
							temp_node_editor_dialog->manager))
						{
							display_message(ERROR_MESSAGE,
								"node_editor_dialog_set_data.  Invalid FE_node.");
						}
					}
					else
					{
						temp_node_editor_dialog->current_value=
							FIRST_OBJECT_IN_MANAGER_THAT(FE_node)(
								(MANAGER_CONDITIONAL_FUNCTION(FE_node) *)NULL,(void *)NULL,
								temp_node_editor_dialog->manager);
					}
					SELECT_SET_SELECT_ITEM(FE_node)(
						temp_node_editor_dialog->select_widget,
						temp_node_editor_dialog->current_value);
					node_editor_set_data(temp_node_editor_dialog->editor_widget,
						NODE_EDITOR_DATA,temp_node_editor_dialog->current_value);
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"node_editor_dialog_set_data.  Invalid data type.");
					return_code=0;
				} break;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"node_editor_dialog_set_data.  Invalid dialog.");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"node_editor_dialog_set_data.  Missing dialog.");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* node_editor_dialog_set_data */

void *node_editor_dialog_get_data(Widget node_editor_dialog_widget,
	enum Node_editor_dialog_data_type data_type)
/*******************************************************************************
LAST MODIFIED : 21 June 1996

DESCRIPTION :
If <node_editor_dialog_widget> is not NULL, then get the data item from
<node_editor_dialog widget>.  Otherwise, get the data item from
<node_editor_dialog>.
==============================================================================*/
{
	int num_children;
	void *return_code;
	struct Node_editor_dialog_struct *temp_node_editor_dialog;
	static struct Callback_data dat_callback;
	static Widget dat_widget;
	Widget *child_list;

	ENTER(node_editor_dialog_get_data);
	if (node_editor_dialog_widget)
	{
		/* Get the pointer to the data for the node_editor_dialog dialog */
		XtVaGetValues(node_editor_dialog_widget,
			XmNchildren,&child_list,
			XmNnumChildren,&num_children,
			NULL);
		if (num_children==1)
		{
			XtVaGetValues(child_list[0],XmNuserData,&temp_node_editor_dialog,
				NULL);
			switch (data_type)
			{
				case NODE_EDITOR_DIALOG_UPDATE_CB:
				{
					dat_callback.procedure=temp_node_editor_dialog->callback_array
						[NODE_EDITOR_DIALOG_UPDATE_CB].procedure;
					dat_callback.data=temp_node_editor_dialog->callback_array
						[NODE_EDITOR_DIALOG_UPDATE_CB].data;
					return_code=&dat_callback;
				}; break;
				case NODE_EDITOR_DIALOG_DATA:
				{
					return_code=temp_node_editor_dialog->current_value;
				}; break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"node_editor_dialog_get_data.  Invalid data type.");
					return_code=NULL;
				}; break;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"node_editor_dialog_get_data.  Invalid dialog.");
			return_code=NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"node_editor_dialog_get_data.  Missing dialog.");
		return_code=NULL;
	}
	LEAVE;

	return (return_code);
} /* node_editor_dialog_get_data */

int bring_up_node_editor_dialog(Widget *node_editor_dialog_address,
	char *create_name,Widget parent,struct MANAGER(FE_node) *node_manager,
	struct FE_node *init_data)
/*******************************************************************************
LAST MODIFIED : 6 November 1998

DESCRIPTION :
If there is a material editor dialog in existence, then bring it to the front,
otherwise it creates a new one.  Assumes we will only ever want one material
editor at a time.  This implementation may be changed later.
???DB.  Should this be somewhere else ?
==============================================================================*/
{
	int return_code;

	ENTER(bring_up_node_editor_dialog);
	if (node_editor_dialog_address)
	{
		/* does it exist */
		if (*node_editor_dialog_address)
		{
			node_editor_dialog_set_data(*node_editor_dialog_address,
				NODE_EDITOR_DIALOG_DATA,init_data);
			XtPopup(*node_editor_dialog_address,XtGrabNone);
		}
		else
		{
			if (create_node_editor_dialog(create_name,node_editor_dialog_address,
				parent,node_manager,init_data))
			{
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"bring_up_node_editor_dialog.  Error creating dialog");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"bring_up_node_editor_dialog.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* bring_up_node_editor_dialog */
