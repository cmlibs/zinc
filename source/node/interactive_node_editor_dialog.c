/*******************************************************************************
FILE : interactive_node_editor_dialog.c

LAST MODIFIED : 13 May 1997

DESCRIPTION :
This module creates a free interactive_node_editor_dialog input device, using
two dof3, two control and one input widget.  The position is given relative to
some coordinate system, and the returned value is a global one.
==============================================================================*/
#include <stdio.h>
#include "general/debug.h"
#include "node/interactive_node_editor.h"
#include "node/interactive_node_editor_dialog.h"
#include "node/interactive_node_editor_dialog.uidh"
#include "select/select_finite_element.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int in_editor_dialog_hierarchy_open=0;
static MrmHierarchy in_editor_dialog_hierarchy;
#endif /* defined (MOTIF) */

/*
Module functions
----------------
*/
static void in_editor_dialog_update(
	struct Interactive_node_editor_dialog_struct *temp_interactive_node_editor_dialog)
/*******************************************************************************
LAST MODIFIED : 26 September 1995

DESCRIPTION :
Tells CMGUI about the current values. Returns a pointer to the material.
==============================================================================*/
{
	ENTER(in_editor_dialog_update);
	if (temp_interactive_node_editor_dialog->
		callback_array[IN_EDITOR_DIALOG_UPDATE_CB].
		procedure)
	{
		/* now call the procedure with the user data and the position data */
		(temp_interactive_node_editor_dialog->
			callback_array[IN_EDITOR_DIALOG_UPDATE_CB].
			procedure)(temp_interactive_node_editor_dialog->widget,
				temp_interactive_node_editor_dialog->
				callback_array[IN_EDITOR_DIALOG_UPDATE_CB].
				data,temp_interactive_node_editor_dialog->current_value);
	}
	LEAVE;
} /* in_editor_dialog_update */

static void in_editor_dialog_update_selection(Widget select_widget,
	void *user_data,void *temp_nod)
/*******************************************************************************
LAST MODIFIED : 26 September 1995

DESCRIPTION :
Finds which material is selected, and informs the editor widget.
???DB.  Not checking arguments ?
???GMH. The new FE_node may be NULL.  Should really check userdata.
==============================================================================*/
{
	struct Interactive_node_editor_dialog_struct *temp_interactive_node_editor_dialog=user_data;
	struct FE_node *temp_node=temp_nod;

	ENTER(in_editor_dialog_update_selection);
	if(temp_interactive_node_editor_dialog)
	{
		temp_interactive_node_editor_dialog->current_value=temp_node;
		interactive_node_editor_set_data(temp_interactive_node_editor_dialog->editor_widget,
			INTERACTIVE_NODE_EDITOR_DATA,temp_interactive_node_editor_dialog->current_value);
	}
	else
	{
		display_message(WARNING_MESSAGE,
			"in_editor_dialog_update_selection.  Invalid dialog.");
	}
	LEAVE;
} /* in_editor_dialog_update_selection */

static void in_editor_dialog_update_object(Widget select_widget,
	void *user_data,void *temp_nod)
/*******************************************************************************
LAST MODIFIED : 26 September 1995

DESCRIPTION :
Executes a command when the node is changed.
==============================================================================*/
{
	struct Interactive_node_editor_dialog_struct *temp_interactive_node_editor_dialog=
		user_data;

	ENTER(in_editor_dialog_update_object);
	if(temp_interactive_node_editor_dialog)
	{
		/* no matter what, we have to do an update */
		interactive_node_editor_modify_global(temp_interactive_node_editor_dialog->editor_widget);
		if(temp_interactive_node_editor_dialog->command)
		{
			(*(temp_interactive_node_editor_dialog->execute_command->function))(
				temp_interactive_node_editor_dialog->command,
				temp_interactive_node_editor_dialog->execute_command->data);
		}
		in_editor_dialog_update(temp_interactive_node_editor_dialog);
	}
	else
	{
		display_message(WARNING_MESSAGE,
			"in_editor_dialog_update_object.  Invalid dialog.");
	}
	LEAVE;
} /* in_editor_dialog_update_object */

static void in_editor_dialog_identify_button(Widget w,int button_num,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 18 April 1994

DESCRIPTION :
Finds the id of the buttons on the interactive_node_editor_dialog widget.
==============================================================================*/
{
	struct Interactive_node_editor_dialog_struct *temp_interactive_node_editor_dialog;

	ENTER(in_editor_dialog_identify_button);
	/* find out which interactive_node_editor_dialog widget we are in */
	XtVaGetValues(w,XmNuserData,&temp_interactive_node_editor_dialog,NULL);
	switch (button_num)
	{
		case in_editor_dialog_editor_form_ID:
		{
			temp_interactive_node_editor_dialog->editor_form=w;
		}; break;
		case in_editor_dialog_select_form_ID:
		{
			temp_interactive_node_editor_dialog->select_form=w;
		}; break;
		case in_editor_dialog_comfile_label_ID:
		{
			temp_interactive_node_editor_dialog->comfile_label=w;
		}; break;
		case in_editor_dialog_comfile_text_ID:
		{
			temp_interactive_node_editor_dialog->comfile_text=w;
		}; break;
		default:
		{
			display_message(WARNING_MESSAGE,
				"in_editor_dialog_identify_button.  Invalid button number");
		}; break;
	}
	LEAVE;
} /* in_editor_dialog_identify_button */

static void in_editor_dialog_destroy_CB(Widget w,int *tag,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Callback for the interactive_node_editor_dialog dialog - tidies up all details - mem etc
==============================================================================*/
{
	struct Interactive_node_editor_dialog_struct *temp_interactive_node_editor_dialog;

	ENTER(in_editor_dialog_destroy_CB);
	/* Get the pointer to the data for the interactive_node_editor_dialog widget */
	XtVaGetValues(w,XmNuserData,&temp_interactive_node_editor_dialog,NULL);
	/* deaccess the interactive_node_editor_dialog */
	*(temp_interactive_node_editor_dialog->dialog_address)=(Widget)NULL;
	/* deallocate the command */
	DEALLOCATE(temp_interactive_node_editor_dialog->command);
	/* deallocate the memory for the user data */
	DEALLOCATE(temp_interactive_node_editor_dialog);
	LEAVE;
} /* in_editor_dialog_destroy_CB */

static void in_editor_dialog_comfile_text_CB(Widget w, int button_num,
	XmAnyCallbackStruct *reason)
/*******************************************************************************
LAST MODIFIED : 20 February 1996

DESCRIPTION :
Takes the name from the text field widget, and makes it the name of the current
object.
==============================================================================*/
{
	char *new_name;
	struct Interactive_node_editor_dialog_struct *temp_interactive_node_editor_dialog;

	ENTER(in_editor_dialog_comfile_text_CB);
	XtVaGetValues(w,XmNuserData,&temp_interactive_node_editor_dialog,XmNvalue,&new_name,NULL);
	if (temp_interactive_node_editor_dialog)
	{
		DEALLOCATE(temp_interactive_node_editor_dialog->command);
		/* only create a command if there is a name in the box */
		if(new_name&&strlen(new_name))
		{
			if(ALLOCATE(temp_interactive_node_editor_dialog->command,char,
				4+1+7+1+strlen(new_name)+1+7+1))
			{
				strcpy(temp_interactive_node_editor_dialog->command,"open comfile ");
				strcat(temp_interactive_node_editor_dialog->command,new_name);
					strcat(temp_interactive_node_editor_dialog->command," execute");
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"in_editor_dialog_comfile_text_CB.  Could not create command.");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"in_editor_dialog_comfile_text_CB.  Invalid dialog.");
	}
	XtFree(new_name);
	LEAVE;
} /* in_editor_dialog_comfile_text_CB */

static Widget create_interactive_node_editor_dialog(
	Widget *in_editor_dialog_widget,Widget parent,
	struct MANAGER(FE_node) *manager,struct Execute_command *execute_command,
	struct FE_node *init_data)
/*******************************************************************************
LAST MODIFIED : 13 December 1996

DESCRIPTION :
Creates a dialog widget that allows the user to edit the properties of any of
the materials contained in the global list.
==============================================================================*/
{
	int i,init_widgets;
	MrmType in_editor_dialog_dialog_class;
	struct Callback_data callback;
	struct Interactive_node_editor_dialog_struct *temp_interactive_node_editor_dialog=NULL;
	static MrmRegisterArg callback_list[]=
	{
		{"i_node_editor_d_identify_button",
		(XtPointer)in_editor_dialog_identify_button},
		{"i_node_editor_d_destroy_CB",
		(XtPointer)in_editor_dialog_destroy_CB},
		{"i_node_editor_d_comfile_text_CB",
		(XtPointer)in_editor_dialog_comfile_text_CB}
	};
	static MrmRegisterArg identifier_list[]=
	{
		{"I_node_editor_d_structure",(XtPointer)NULL},
		{"i_node_editor_d_editor_form_ID",
		(XtPointer)in_editor_dialog_editor_form_ID},
		{"i_node_editor_d_select_form_ID",
		(XtPointer)in_editor_dialog_select_form_ID},
		{"i_node_editor_d_com_label_ID",
		(XtPointer)in_editor_dialog_comfile_label_ID},
		{"i_node_editor_d_com_text_ID",
		(XtPointer)in_editor_dialog_comfile_text_ID},
	};
	Widget return_widget;

	ENTER(create_interactive_node_editor_dialog);
	return_widget=(Widget)NULL;
	/* check arguments */
	if (in_editor_dialog_widget&&parent&&manager&&execute_command&&
		(execute_command->function))
	{
		if (MrmOpenHierarchy_base64_string(interactive_node_editor_dialog_uidh,
			&in_editor_dialog_hierarchy,
			&in_editor_dialog_hierarchy_open))
		{
			/* allocate memory */
			if (ALLOCATE(temp_interactive_node_editor_dialog,
				struct Interactive_node_editor_dialog_struct,1))
			{
				/* initialise the structure */
				temp_interactive_node_editor_dialog->dialog_parent=parent;
				temp_interactive_node_editor_dialog->dialog_address=in_editor_dialog_widget;
				temp_interactive_node_editor_dialog->execute_command=execute_command;
				if (init_data)
				{
					temp_interactive_node_editor_dialog->current_value=init_data;
				}
				else
				{
					temp_interactive_node_editor_dialog->current_value=FIRST_OBJECT_IN_MANAGER_THAT(
						FE_node)((MANAGER_CONDITIONAL_FUNCTION(FE_node) *)NULL,(void *)NULL,
						manager);
				}
				temp_interactive_node_editor_dialog->manager=manager;
				temp_interactive_node_editor_dialog->widget=(Widget)NULL;
				temp_interactive_node_editor_dialog->dialog=(Widget)NULL;
				temp_interactive_node_editor_dialog->select_form=(Widget)NULL;
				temp_interactive_node_editor_dialog->select_widget=(Widget)NULL;
				temp_interactive_node_editor_dialog->editor_form=(Widget)NULL;
				temp_interactive_node_editor_dialog->editor_widget=(Widget)NULL;
				temp_interactive_node_editor_dialog->comfile_label=(Widget)NULL;
				temp_interactive_node_editor_dialog->comfile_text=(Widget)NULL;
				temp_interactive_node_editor_dialog->command=(char *)NULL;
				for (i=0;i<IN_EDITOR_DIALOG_NUM_CALLBACKS;i++)
				{
					temp_interactive_node_editor_dialog->callback_array[i].procedure=
						(Callback_procedure *)NULL;
					temp_interactive_node_editor_dialog->callback_array[i].data=NULL;
				}
				/* make the dialog shell */
				if (temp_interactive_node_editor_dialog->dialog=XtVaCreatePopupShell(
					"Node Editor",topLevelShellWidgetClass,parent,XmNallowShellResize,
					True,NULL))
				{
					/* register the callbacks */
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
						in_editor_dialog_hierarchy,callback_list,XtNumber(callback_list)))
					{
						/* assign and register the identifiers */
						identifier_list[0].value=(XtPointer)temp_interactive_node_editor_dialog;
						if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
							in_editor_dialog_hierarchy,identifier_list,
							XtNumber(identifier_list)))
						{
							/* fetch position window widget */
							if (MrmSUCCESS==MrmFetchWidget(in_editor_dialog_hierarchy,
								"i_node_editor_dialog_widget",temp_interactive_node_editor_dialog->dialog,
								&(temp_interactive_node_editor_dialog->widget),
								&in_editor_dialog_dialog_class))
							{
								XtManageChild(temp_interactive_node_editor_dialog->widget);
								/* set the mode toggle to the correct position */
								init_widgets=1;
								if (!CREATE_SELECT_WIDGET(FE_node)(
									&temp_interactive_node_editor_dialog->select_widget,
									temp_interactive_node_editor_dialog->select_form,
									SELECT_TEXT,
									temp_interactive_node_editor_dialog->current_value,manager))
								{
									display_message(ERROR_MESSAGE,
								"create_interactive_node_editor_dialog.  Could not create select widget.");
									init_widgets=0;
								}
								if (!create_interactive_node_editor_widget(
									&temp_interactive_node_editor_dialog->editor_widget,
									temp_interactive_node_editor_dialog->editor_form,manager,
									temp_interactive_node_editor_dialog->current_value))
								{
									display_message(ERROR_MESSAGE,
								"create_interactive_node_editor_dialog.  Could not create editor widget.");
									init_widgets=0;
								}
								if (init_widgets)
								{
									callback.procedure=in_editor_dialog_update_selection;
									callback.data=temp_interactive_node_editor_dialog;
									SELECT_SET_UPDATE_CB(FE_node)(
										temp_interactive_node_editor_dialog->select_widget,
										&callback);
									callback.procedure=in_editor_dialog_update_object;
									callback.data=temp_interactive_node_editor_dialog;
									interactive_node_editor_set_data(
										temp_interactive_node_editor_dialog->editor_widget,
										INTERACTIVE_NODE_EDITOR_UPDATE_CB,&callback);
									XtRealizeWidget(temp_interactive_node_editor_dialog->dialog);
									XtPopup(temp_interactive_node_editor_dialog->dialog, XtGrabNone);
									return_widget=temp_interactive_node_editor_dialog->dialog;
								}
								else
								{
									DEALLOCATE(temp_interactive_node_editor_dialog);
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
							"create_interactive_node_editor_dialog.  Could not fetch interactive_node_editor_dialog");
								DEALLOCATE(temp_interactive_node_editor_dialog);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"create_interactive_node_editor_dialog.  Could not register identifiers");
							DEALLOCATE(temp_interactive_node_editor_dialog);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_interactive_node_editor_dialog.  Could not register callbacks");
						DEALLOCATE(temp_interactive_node_editor_dialog);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_interactive_node_editor_dialog.  Could not create popup shell.");
					DEALLOCATE(temp_interactive_node_editor_dialog);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
		"create_interactive_node_editor_dialog.  Could not allocate interactive_node_editor_dialog");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_interactive_node_editor_dialog.  Could not open hierarchy");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_interactive_node_editor_dialog.  Invalid argument(s)");
	}
	if (in_editor_dialog_widget&&return_widget)
	{
		*in_editor_dialog_widget=return_widget;
	}
	LEAVE;

	return (return_widget);
} /* create_interactive_node_editor_dialog */

/*
Global Functions
----------------
*/
int in_editor_dialog_set_data(Widget in_editor_dialog_widget,
	enum Interactive_node_editor_dialog_data_type data_type,void *data)
/*******************************************************************************
LAST MODIFIED : 21 July 1996

DESCRIPTION :
If <in_editor_dialog_widget> is not NULL, then change the data item on
<interactive_node_editor_dialog widget>.  Otherwise, change the data item on
<interactive_node_editor_dialog>.
==============================================================================*/
{
	int num_children,return_code;
	struct Interactive_node_editor_dialog_struct *temp_interactive_node_editor_dialog;
	Widget *child_list;

	ENTER(in_editor_dialog_set_data);
	if (in_editor_dialog_widget)
	{
		/* get the pointer to the data for the dialog */
		XtVaGetValues(in_editor_dialog_widget,
			XmNchildren,&child_list,
			XmNnumChildren,&num_children,
			NULL);
		if (num_children==1)
		{
			XtVaGetValues(child_list[0],
				XmNuserData,&temp_interactive_node_editor_dialog,
				NULL);
			switch (data_type)
			{
				case IN_EDITOR_DIALOG_UPDATE_CB:
				{
					temp_interactive_node_editor_dialog->
						callback_array[IN_EDITOR_DIALOG_UPDATE_CB].procedure=
						((struct Callback_data *)data)->procedure;
					temp_interactive_node_editor_dialog->
						callback_array[IN_EDITOR_DIALOG_UPDATE_CB].data=
						((struct Callback_data *)data)->data;
					return_code=1;
				} break;
				case IN_EDITOR_DIALOG_DATA:
				{
					if (data)
					{
						temp_interactive_node_editor_dialog->current_value=
							(struct FE_node *)data;
						if (!IS_MANAGED(FE_node)(temp_interactive_node_editor_dialog->current_value,
							temp_interactive_node_editor_dialog->manager))
						{
							display_message(ERROR_MESSAGE,
								"in_editor_dialog_set_data.  Invalid FE_node.");
						}
					}
					else
					{
						temp_interactive_node_editor_dialog->current_value=
							FIRST_OBJECT_IN_MANAGER_THAT(FE_node)(
								(MANAGER_CONDITIONAL_FUNCTION(FE_node) *)NULL,(void *)NULL,
								temp_interactive_node_editor_dialog->manager);
					}
					SELECT_SET_SELECT_ITEM(FE_node)(
						temp_interactive_node_editor_dialog->select_widget,
						temp_interactive_node_editor_dialog->current_value);
					interactive_node_editor_set_data(
						temp_interactive_node_editor_dialog->editor_widget,
						INTERACTIVE_NODE_EDITOR_DATA,
						temp_interactive_node_editor_dialog->current_value);
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"in_editor_dialog_set_data.  Invalid data type.");
					return_code=0;
				} break;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"in_editor_dialog_set_data.  Invalid dialog.");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"in_editor_dialog_set_data.  Missing dialog.");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* in_editor_dialog_set_data */

void *in_editor_dialog_get_data(Widget in_editor_dialog_widget,
	enum Interactive_node_editor_dialog_data_type data_type)
/*******************************************************************************
LAST MODIFIED : 21 July 1996

DESCRIPTION :
If <in_editor_dialog_widget> is not NULL, then get the data item from
<interactive_node_editor_dialog widget>.  Otherwise, get the data item from
<interactive_node_editor_dialog>.
==============================================================================*/
{
	int num_children;
	void *return_code;
	struct Interactive_node_editor_dialog_struct *temp_interactive_node_editor_dialog;
	static struct Callback_data dat_callback;
	static Widget dat_widget;
	Widget *child_list;

	ENTER(in_editor_dialog_get_data);
	if (in_editor_dialog_widget)
	{
		/* Get the pointer to the data for the interactive_node_editor_dialog dialog */
		XtVaGetValues(in_editor_dialog_widget,
			XmNchildren,&child_list,
			XmNnumChildren,&num_children,
			NULL);
		if (num_children==1)
		{
			XtVaGetValues(child_list[0],XmNuserData,&temp_interactive_node_editor_dialog,
				NULL);
			switch (data_type)
			{
				case IN_EDITOR_DIALOG_UPDATE_CB:
				{
					dat_callback.procedure=temp_interactive_node_editor_dialog->callback_array
						[IN_EDITOR_DIALOG_UPDATE_CB].procedure;
					dat_callback.data=temp_interactive_node_editor_dialog->callback_array
						[IN_EDITOR_DIALOG_UPDATE_CB].data;
					return_code=&dat_callback;
				}; break;
				case IN_EDITOR_DIALOG_DATA:
				{
					return_code=temp_interactive_node_editor_dialog->current_value;
				}; break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"in_editor_dialog_get_data.  Invalid data type.");
					return_code=NULL;
				}; break;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"in_editor_dialog_get_data.  Invalid dialog.");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"in_editor_dialog_get_data.  Missing dialog.");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* in_editor_dialog_get_data */

int bring_up_interactive_node_editor_dialog(
	Widget *in_editor_dialog_address,Widget parent,
	struct MANAGER(FE_node) *node_manager,struct Execute_command *execute_command,
	struct FE_node *init_data)
/*******************************************************************************
LAST MODIFIED : 21 July 1996

DESCRIPTION :
If there is a material editor dialog in existence, then bring it to the front,
otherwise it creates a new one.  Assumes we will only ever want one material
editor at a time.  This implementation may be changed later.
???DB.  Should this be somewhere else ?
==============================================================================*/
{
	int return_code;

	ENTER(bring_up_interactive_node_editor_dialog);
	/* does it exist */
	if (in_editor_dialog_address)
	{
		/* does it exist */
		if (*in_editor_dialog_address)
		{
			in_editor_dialog_set_data(
				*in_editor_dialog_address,
				IN_EDITOR_DIALOG_DATA,init_data);
			XtPopup(*in_editor_dialog_address,XtGrabNone);
		}
		else
		{
			if (create_interactive_node_editor_dialog(
				in_editor_dialog_address,parent,node_manager,
				execute_command,init_data))
			{
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"bring_up_interactive_node_editor_dialog.  Error creating dialog");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"bring_up_interactive_node_editor_dialog.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* bring_up_interactive_node_editor_dialog */
