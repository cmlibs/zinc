/*******************************************************************************
FILE : node_editor.c

LAST MODIFIED : 21 September 1999

DESCRIPTION :
This module creates a node editor widget.  A node editor widget consists of
multiple node field widgets.  Buttons allow fields to be created and destroyed
in the node.
==============================================================================*/
#include <math.h>
#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xm/ToggleBG.h>
#include <Mrm/MrmPublic.h>
#include "general/debug.h"
#include "node/node_field_editor.h"
#include "node/node_editor.h"
#include "node/node_editor.uidh"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int node_editor_hierarchy_open=0;
static MrmHierarchy node_editor_hierarchy;
#endif /* defined (MOTIF) */

/*
Module functions
----------------
*/
static void node_editor_update(struct Node_editor_struct *temp_node_editor)
/*******************************************************************************
LAST MODIFIED : 26 September 1995

DESCRIPTION :
Tells CMGUI about the current values. Returns a pointer to the node.
==============================================================================*/
{
	ENTER(node_editor_update);
	if (temp_node_editor->callback_array[NODE_EDITOR_UPDATE_CB].procedure)
	{
		if (temp_node_editor->global_value==temp_node_editor->default_value)
		{
			/* now call the procedure with the user data and a NULL pointer */
			(temp_node_editor->callback_array[NODE_EDITOR_UPDATE_CB].procedure)
				(temp_node_editor->widget,
				temp_node_editor->callback_array[NODE_EDITOR_UPDATE_CB].data,
				NULL);
		}
		else
		{
			/* now call the procedure with the user data and a pointer to the node */
			(((temp_node_editor->callback_array)[NODE_EDITOR_UPDATE_CB]).
				procedure)(temp_node_editor->widget,
				temp_node_editor->callback_array[NODE_EDITOR_UPDATE_CB].data,
				temp_node_editor->current_value);
		}
	}
	LEAVE;
} /* node_editor_update */

static void node_editor_identify_button(Widget w,int button_num,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 21 September 1999

DESCRIPTION :
Finds the id of the buttons on the node_editor widget.
==============================================================================*/
{
	struct Node_editor_struct *temp_node_editor;

	ENTER(node_editor_identify_button);
	USE_PARAMETER(reason);
	/* find out which node_editor widget we are in */
	XtVaGetValues(w,XmNuserData,&temp_node_editor,NULL);
	switch (button_num)
	{
#if defined (NODE_EDITOR_NAME)
		case node_editor_name_ID:
		{
			temp_node_editor->name=w;
		}; break;
#endif
		case node_editor_field_form_ID:
		{
			temp_node_editor->field_form=w;
		}; break;
		case node_editor_toggle_position_ID:
		{
			temp_node_editor->toggle_position=w;
		}; break;
		case node_editor_toggle_rowcol_ID:
		{
			temp_node_editor->toggle_rowcol=w;
		}; break;
		default:
		{
			display_message(WARNING_MESSAGE,
				"node_editor_identify_button.  Invalid button number");
		}; break;
	}
	LEAVE;
} /* node_editor_identify_button */

static void node_editor_destroy_CB(Widget widget,int *tag,unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 21 September 1999

DESCRIPTION :
Callback for the node_editor dialog - tidies up all details - mem etc
==============================================================================*/
{
	struct Node_editor_struct *temp_node_editor;

	ENTER(node_editor_destroy_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	if (widget)
	{
		/* Get the pointer to the data for the node_editor widget */
		XtVaGetValues(widget,XmNuserData,&temp_node_editor,NULL);
		if (temp_node_editor)
		{
			*(temp_node_editor->widget_address)=(Widget)NULL;
			MANAGER_DEREGISTER(FE_node)(temp_node_editor->manager_callback_id,
				temp_node_editor->manager);
			/* deallocate the memory for the user data */
			if (temp_node_editor->current_value!=temp_node_editor->default_value)
			{
				DESTROY(FE_node)(&(temp_node_editor->current_value));
			}
			DESTROY(FE_node)(&(temp_node_editor->default_value));
			DEALLOCATE(temp_node_editor);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"node_editor_destroy_CB.  Missing node_editor");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"node_editor_destroy_CB.  Missing widget");
	}
	LEAVE;
} /* node_editor_destroy_CB */

#if defined (NODE_EDITOR_NAME)
static void node_editor_set_name(struct Node_editor_struct *temp_node_editor)
/*******************************************************************************
LAST MODIFIED : 28 September 1995

DESCRIPTION :
Writes the correct name on the label.
==============================================================================*/
{
	XmString temp_label;

	ENTER(node_editor_set_name);
	temp_label=XmStringCreateSimple((temp_node_editor->current_value)->name);
	XtVaSetValues(temp_node_editor->name,XmNlabelString,temp_label,NULL);
	XmStringFree(temp_label);
	LEAVE;
} /* node_editor_set_name */
#endif

static void node_editor_global_object_change(
	struct MANAGER_MESSAGE(FE_node) *message,void *data)
/*******************************************************************************
LAST MODIFIED : 28 September 1995

DESCRIPTION :
Something has changed globally about the objects this widget uses, so refresh.
==============================================================================*/
{
	struct Node_editor_struct *temp_node_editor=data;

	ENTER(node_editor_global_object_change);
	/* ???GMH.  I wrote this a while ago, and remember thinking a long time over
		what actions were necessary with what messages.  Surely if we get a modify
		message we have to update the node_editor of the node? */
#if defined (OLD_CODE)
	/*???DB.  Is type needed */
	/*???GMH.  Probably not.  Maybe I was being over-cautious, but we do have void *
	pointers floating around. */
	if (message->type==MANAGER_FE_node)
	{
#endif
		switch (message->change)
		{
			case MANAGER_CHANGE_ALL(FE_node):
			{
				if (IS_MANAGED(FE_node)(temp_node_editor->global_value,
					temp_node_editor->manager))
				{
					node_editor_set_data(temp_node_editor->widget,NODE_EDITOR_DATA,
						temp_node_editor->global_value);
				}
				else
				{
					node_editor_set_data(temp_node_editor->widget,
						NODE_EDITOR_DATA,FIRST_OBJECT_IN_MANAGER_THAT(FE_node)(
						(MANAGER_CONDITIONAL_FUNCTION(FE_node) *)NULL,(void *)NULL,
						temp_node_editor->manager));
				}
			}; break;
			case MANAGER_CHANGE_DELETE(FE_node):
			{
				if (message->object_changed==temp_node_editor->global_value)
				{
					node_editor_set_data(temp_node_editor->widget,
						NODE_EDITOR_DATA,FIRST_OBJECT_IN_MANAGER_THAT(FE_node)(
						(MANAGER_CONDITIONAL_FUNCTION(FE_node) *)NULL,(void *)NULL,
						temp_node_editor->manager));
				}
			}; break;
			case MANAGER_CHANGE_ADD(FE_node):
			{
				if (NULL==temp_node_editor->global_value)
				{
					node_editor_set_data(temp_node_editor->widget,NODE_EDITOR_DATA,
						message->object_changed);
				}
			}; break;
			case MANAGER_CHANGE_IDENTIFIER(FE_node):
			case MANAGER_CHANGE_OBJECT(FE_node):
			case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(FE_node):
			{
				if (message->object_changed==temp_node_editor->global_value)
				{
					node_editor_set_data(temp_node_editor->widget,NODE_EDITOR_DATA,
						message->object_changed);
				}
			} break;
		}
#if defined (OLD_CODE)
	}
#endif
	LEAVE;
} /* node_editor_global_object_change */

static void node_editor_toggle_CB(Widget w, int *tag,
	XmToggleButtonCallbackStruct *reason)
/*******************************************************************************
LAST MODIFIED : 21 September 1999

DESCRIPTION :
Makes sure that the correct field editor is managed
==============================================================================*/
{
	struct FE_field *field;
	struct Node_editor_struct *temp_node_editor;

	ENTER(node_editor_toggle_CB);
	USE_PARAMETER(tag);
	/* Get the pointer to the data for the node_editor widget */
	XtVaGetValues(XtParent(w),XmNuserData,&temp_node_editor,NULL);
	XtVaGetValues(w,XmNuserData,&field,NULL);
	if (temp_node_editor)
	{
		if (reason->set)
		{
			node_field_editor_set_data(temp_node_editor->field_widget,
				NODE_FIELD_EDITOR_DATA,field);
			/* setup the values in the field widget */
			node_field_editor_set_data(temp_node_editor->field_widget,
				NODE_FIELD_EDITOR_VALUE,temp_node_editor->current_value);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"node_editor_toggle_CB.  Invalid field widget.");
	}
	LEAVE;
} /* node_editor_toggle_CB */

static void node_editor_update_field(Widget node_field_widget,void *user_data,
	void *temp_null)
/*******************************************************************************
LAST MODIFIED : 21 September 1999

DESCRIPTION :
Notifier that some portion of the data vector has changed, and so the widget
should notify all its clients.
The temp_null field contains no information.
==============================================================================*/
{
	struct Node_editor_struct *temp_node_editor=user_data;

	ENTER(node_editor_update_field);
	USE_PARAMETER(node_field_widget);
	USE_PARAMETER(temp_null);
	node_editor_update(temp_node_editor);
	LEAVE;
} /* node_editor_update_field */

static int node_editor_setup_a_field(struct FE_node *node,
	struct FE_field *field,void *user_data)
/*******************************************************************************
LAST MODIFIED : 21 September 1999

DESCRIPTION :
Creates one toggle button and one field editor.
*first is used to tell it which button should be set.
==============================================================================*/
{
	Arg override_arg;
	char *name;
	int return_code;
	MrmType node_editor_class;
	struct Node_editor_struct *temp_node_editor=user_data;
	Widget temp_widget;
	XmString new_string;

	ENTER(node_editor_setup_a_field);
	if (node&&field)
	{
		if (node_editor_hierarchy_open)
		{
			/* create the toggle button */
			XtSetArg(override_arg,XmNuserData,field);
			/* ie the data is the corresponding field widget */
			temp_widget=(Widget)NULL;
			if (MrmSUCCESS==MrmFetchWidgetOverride(node_editor_hierarchy,
				"node_editor_toggle_item",temp_node_editor->toggle_rowcol,NULL,
				&override_arg,1,&temp_widget,&node_editor_class))
			{
				return_code=1;
				XtManageChild(temp_widget);
				name=(char *)NULL;
				if (GET_NAME(FE_field)(field,&name))
				{
					new_string=XmStringCreateSimple(name);
					XtVaSetValues(temp_widget,XmNlabelString,new_string,NULL);
					XmStringFree(new_string);
					if (!temp_node_editor->first_toggle)
					{
						temp_node_editor->first_toggle=temp_widget;
					}
					DEALLOCATE(name);
				}
				else
				{
					display_message(WARNING_MESSAGE,
						"node_editor_setup_a_field.  Could not get field name");
					return_code=0;
				}
			}
			else
			{
				display_message(WARNING_MESSAGE,
					"node_editor_setup_a_field.  Could not fetch toggle item widget");
				return_code=0;
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,
				"node_editor_setup_a_field.  Hierarchy is not open");
			return_code=0;
		}
	}
	else
	{
		display_message(WARNING_MESSAGE,
			"node_editor_setup_a_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return return_code;
} /* node_editor_setup_a_field */

static void node_editor_setup_fields(
	struct Node_editor_struct *temp_node_editor)
/*******************************************************************************
LAST MODIFIED : 2 November 1998

DESCRIPTION :
Creates as many field editor widgets are required, and puts them all in the
same widget.  Only one is managed at one time.
==============================================================================*/
{
	struct FE_field *field;
	Arg override_arg;
	MrmType node_editor_class;
	Widget temp_widget;

	ENTER(node_editor_setup_fields);
	if (node_editor_hierarchy_open)
	{
		/* destroy the main form */
		if (temp_node_editor->toggle_rowcol)
		{
			XtDestroyWidget(temp_node_editor->toggle_rowcol);
		}
		/* now create another */
		XtSetArg(override_arg,XmNuserData,temp_node_editor);
		temp_widget=(Widget)NULL;
		if (MrmSUCCESS==MrmFetchWidgetOverride(node_editor_hierarchy,
			"node_editor_toggle_rowcol",temp_node_editor->toggle_position,NULL,
			&override_arg,1,&temp_widget,&node_editor_class))
		{
			XtManageChild(temp_widget);
		}
		else
		{
			display_message(WARNING_MESSAGE,
				"node_editor_setup_fields.  Could not fetch toggle rowcol widget.");
		}
		/* now create the items in the widgets */
		temp_node_editor->first_toggle=(Widget)NULL;
		for_each_FE_field_at_node(node_editor_setup_a_field,temp_node_editor,
			temp_node_editor->current_value);
		/* if we have a field, we must display the first field */
		if (temp_node_editor->first_toggle)
		{
			XmToggleButtonGadgetSetState(temp_node_editor->first_toggle,TRUE,FALSE);
			/* now set the field widget */
			XtVaGetValues(temp_node_editor->first_toggle,XmNuserData,&field,NULL);
			node_field_editor_set_data(temp_node_editor->field_widget,
				NODE_FIELD_EDITOR_DATA,field);
		}
		else
		{
			node_field_editor_set_data(temp_node_editor->field_widget,
				NODE_FIELD_EDITOR_DATA,NULL);
		}
	}
	else
	{
		display_message(WARNING_MESSAGE,
			"node_editor_setup_fields.  Hierarchy is not open");
	}
	LEAVE;
} /* node_editor_setup_fields */

/*
Global functions
----------------
*/
Widget create_node_editor_widget(Widget *node_editor_widget,Widget parent,
	struct MANAGER(FE_node) *manager,struct FE_node *init_data)
/*******************************************************************************
LAST MODIFIED : 2 November 1998

DESCRIPTION :
Creates a node_editor widget that gets a position and orientation from the
user.
==============================================================================*/
{
	int i,init_widgets;
	MrmType node_editor_dialog_class;
	struct Node_editor_struct *temp_node_editor=NULL;
	struct Callback_data callback;
	static MrmRegisterArg callback_list[]=
	{
		{"node_editor_identify_button",
		(XtPointer)node_editor_identify_button},
		{"node_editor_toggle_CB",(XtPointer)node_editor_toggle_CB},
		{"node_editor_destroy_CB",(XtPointer)node_editor_destroy_CB},
	};
	static MrmRegisterArg identifier_list[]=
	{
		{"Node_editor_structure",(XtPointer)NULL},
#if defined (NODE_EDITOR_NAME)
		{"node_editor_name_ID",(XtPointer)node_editor_name_ID},
#endif
		{"node_editor_field_form_ID",(XtPointer)node_editor_field_form_ID},
		{"node_editor_toggle_position_ID",
			(XtPointer)node_editor_toggle_position_ID},
		{"node_editor_toggle_rowcol_ID",(XtPointer)node_editor_toggle_rowcol_ID},
	};
	Widget return_widget;

	ENTER(create_node_editor_widget);
	/* check arguments */
	return_widget=(Widget)NULL;
	if (manager&&node_editor_widget&&parent)
	{
		if (MrmOpenHierarchy_base64_string(node_editor_uidh,
			&node_editor_hierarchy,&node_editor_hierarchy_open))
		{
			/* allocate memory */
			if (ALLOCATE(temp_node_editor,struct Node_editor_struct,1)&&
				(temp_node_editor->default_value=
				CREATE(FE_node)(0,(struct FE_node *)NULL))&&
				(temp_node_editor->current_value=
				CREATE(FE_node)(0,(struct FE_node *)NULL)))
			{
				/* initialise the structure */
				temp_node_editor->manager=manager;
				temp_node_editor->manager_callback_id=(void *)NULL;
				temp_node_editor->widget_parent=parent;
				temp_node_editor->widget_address=node_editor_widget;
				temp_node_editor->widget=(Widget)NULL;
				temp_node_editor->field_form=(Widget)NULL;
				temp_node_editor->field_widget=(Widget)NULL;
				temp_node_editor->toggle_position=(Widget)NULL;
				temp_node_editor->toggle_rowcol=(Widget)NULL;
#if defined (NODE_EDITOR_NAME)
				temp_node_editor->name=(Widget)NULL;
#endif
				if (init_data)
				{
					temp_node_editor->global_value=init_data;
					if (!IS_MANAGED(FE_node)(temp_node_editor->global_value,
						temp_node_editor->manager))
					{
						display_message(ERROR_MESSAGE,
							"create_node_editor_widget.  Invalid FE_node.");
					}
				}
				else
				{
					temp_node_editor->global_value=FIRST_OBJECT_IN_MANAGER_THAT(FE_node)(
						(MANAGER_CONDITIONAL_FUNCTION(FE_node) *)NULL,(void *)NULL,
						temp_node_editor->manager);
					if (!temp_node_editor->global_value)
					{
						temp_node_editor->global_value=
							temp_node_editor->default_value;
					}
				}
				/* copy to the working node */
				overwrite_FE_node_with_cm_node_identifier(temp_node_editor->current_value,
					temp_node_editor->global_value);
				for(i=0;i<NODE_EDITOR_NUM_CALLBACKS;i++)
				{
					temp_node_editor->callback_array[i].procedure=
						(Callback_procedure *)NULL;
					temp_node_editor->callback_array[i].data=NULL;
				}
				/* register the callbacks */
				if (MrmSUCCESS==MrmRegisterNamesInHierarchy(node_editor_hierarchy,
					callback_list,XtNumber(callback_list)))
				{
					/* assign and register the identifiers */
					identifier_list[0].value=(XtPointer)temp_node_editor;
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(node_editor_hierarchy,
						identifier_list,XtNumber(identifier_list)))
					{
						/* fetch position window widget */
						if (MrmSUCCESS==MrmFetchWidget(node_editor_hierarchy,
							"node_editor_widget",temp_node_editor->widget_parent,
							&(temp_node_editor->widget),&node_editor_dialog_class))
						{
							XtManageChild(temp_node_editor->widget);
							/* set the mode toggle to the correct position */
							init_widgets=1;
							if (!(create_node_field_editor_widget(
								&temp_node_editor->field_widget,
								temp_node_editor->field_form,NULL,
								temp_node_editor->current_value)))
							{
								display_message(ERROR_MESSAGE,
							"create_node_editor_widget.  Could not create node_field widget");
								init_widgets=0;
							}
							if (init_widgets)
							{
								callback.procedure=node_editor_update_field;
								callback.data=temp_node_editor;
								node_field_editor_set_data(temp_node_editor->field_widget,
									NODE_FIELD_EDITOR_UPDATE_CB,&callback);
#if defined (NODE_EDITOR_SET_NAME)
								/* set the name of the node_editor */
								node_editor_set_name(temp_node_editor);
#endif
								/* create the field widgets */
								node_editor_setup_fields(temp_node_editor);
								/* setup the values in the field widget */
								node_field_editor_set_data(temp_node_editor->field_widget,
									NODE_FIELD_EDITOR_VALUE,
									temp_node_editor->current_value);
								/* register for any changes */
								temp_node_editor->manager_callback_id=MANAGER_REGISTER(FE_node)(
									node_editor_global_object_change,temp_node_editor,
									temp_node_editor->manager);
								if (temp_node_editor->global_value==
									temp_node_editor->default_value)
								{
									XtUnmanageChild(temp_node_editor->widget);
								}
								return_widget=temp_node_editor->widget;
							}
							else
							{
								DESTROY(FE_node)(&(temp_node_editor->default_value));
								DESTROY(FE_node)(&(temp_node_editor->current_value));
								DEALLOCATE(temp_node_editor);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
							"create_node_editor_widget.  Could not fetch node_editor dialog");
							DESTROY(FE_node)(&(temp_node_editor->default_value));
							DESTROY(FE_node)(&(temp_node_editor->current_value));
							DEALLOCATE(temp_node_editor);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_node_editor_widget.  Could not register identifiers");
						DESTROY(FE_node)(&(temp_node_editor->default_value));
						DESTROY(FE_node)(&(temp_node_editor->current_value));
						DEALLOCATE(temp_node_editor);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_node_editor_widget.  Could not register callbacks");
					DESTROY(FE_node)(&(temp_node_editor->default_value));
					DESTROY(FE_node)(&(temp_node_editor->current_value));
					DEALLOCATE(temp_node_editor);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
"create_node_editor_widget.  Could not allocate node_editor widget structure");
				if (temp_node_editor)
				{
					if (temp_node_editor->default_value)
					{
						DESTROY(FE_node)(&(temp_node_editor->default_value));
					}
					DEALLOCATE(temp_node_editor);
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_node_editor_widget.  Could not open hierarchy");
		}
		*node_editor_widget=return_widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_node_editor_widget.  Invalid argument(s)");
	}
	LEAVE;

	return (return_widget);
} /* create_node_editor_widget */

int node_editor_set_data(Widget node_editor_widget,
	enum Node_editor_data_type data_type,void *data)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Changes a data item of the node_editor widget.
==============================================================================*/
{
	int return_code;
	struct Node_editor_struct *temp_node_editor;

	ENTER(node_editor_set_data);
	/* Get the pointer to the data for the node_editor dialog */
	XtVaGetValues(node_editor_widget,XmNuserData,&temp_node_editor,NULL);
	switch(data_type)
	{
		case NODE_EDITOR_UPDATE_CB:
		{
			temp_node_editor->callback_array[NODE_EDITOR_UPDATE_CB].procedure=
				((struct Callback_data *)data)->procedure;
			temp_node_editor->callback_array[NODE_EDITOR_UPDATE_CB].data=
				((struct Callback_data *)data)->data;
			return_code=1;
		}; break;
		case NODE_EDITOR_DATA:
		{
			if (temp_node_editor->global_value==
				temp_node_editor->default_value)
			{
				XtManageChild(temp_node_editor->widget);
			}
			if (data)
			{
				temp_node_editor->global_value=(struct FE_node *)data;
				if (!IS_MANAGED(FE_node)(temp_node_editor->global_value,
					temp_node_editor->manager))
				{
					display_message(ERROR_MESSAGE,
						"node_editor_set_data.  Invalid FE_node.");
				}
			}
			else
			{
				temp_node_editor->global_value=FIRST_OBJECT_IN_MANAGER_THAT(FE_node)(
					(MANAGER_CONDITIONAL_FUNCTION(FE_node) *)NULL,(void *)NULL,
					temp_node_editor->manager);
				if (!temp_node_editor->global_value)
				{
					temp_node_editor->global_value=
						temp_node_editor->default_value;
					XtUnmanageChild(temp_node_editor->widget);
				}
			}
			/* copy to the working node */
			overwrite_FE_node_with_cm_node_identifier(temp_node_editor->current_value,
				temp_node_editor->global_value);
#if defined (NODE_EDITOR_NAME)
			/* set the name of the node_editor */
			node_editor_set_name(temp_node_editor);
#endif
			/* create the field widgets */
			node_editor_setup_fields(temp_node_editor);
			/* setup the values in the field widget */
			node_field_editor_set_data(temp_node_editor->field_widget,
				NODE_FIELD_EDITOR_VALUE,temp_node_editor->current_value);
		}; break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"node_editor_set_data.  Invalid data type.");
			return_code=0;
		}; break;
	}
	LEAVE;

	return (return_code);
} /* node_editor_set_data */

void *node_editor_get_data(Widget node_editor_widget,
	enum Node_editor_data_type data_type)
/*******************************************************************************
LAST MODIFIED : 21 September 1999

DESCRIPTION :
Changes a data item of the node_editor widget.
==============================================================================*/
{
	void *return_code;
	struct Node_editor_struct *temp_node_editor;
	static struct Callback_data dat_callback;

	ENTER(node_editor_set_data);
	/* Get the pointer to the data for the node_editor dialog */
	XtVaGetValues(node_editor_widget,XmNuserData,&temp_node_editor,NULL);
	switch(data_type)
	{
		case NODE_EDITOR_UPDATE_CB:
		{
			dat_callback.procedure=temp_node_editor->
				callback_array[NODE_EDITOR_UPDATE_CB].procedure;
			dat_callback.data=temp_node_editor->
				callback_array[NODE_EDITOR_UPDATE_CB].data;
			return_code= &dat_callback;
		}; break;
		case NODE_EDITOR_DATA:
		{
			return_code=temp_node_editor->current_value;
		}; break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"node_editor_get_data.  Invalid data type.");
			return_code=NULL;
		}; break;
	}
	LEAVE;

	return (return_code);
} /* node_editor_get_data */

void node_editor_modify_global(Widget node_editor_widget)
/*******************************************************************************
LAST MODIFIED : 28 September 1995

DESCRIPTION :
Makes the node change global.
==============================================================================*/
{
	struct Node_editor_struct *temp_node_editor;

	ENTER(node_editor_modify_global);
	/* Get the pointer to the data for the node_editor dialog */
	XtVaGetValues(node_editor_widget,XmNuserData,&temp_node_editor,NULL);
	if (temp_node_editor->global_value!=temp_node_editor->default_value)
	{
		MANAGER_MODIFY_NOT_IDENTIFIER(FE_node,cm_node_identifier)(temp_node_editor->
			global_value,temp_node_editor->current_value,
			temp_node_editor->manager);
	}
	LEAVE;
} /* node_editor_modify_global */
