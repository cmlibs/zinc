/*******************************************************************************
FILE : node_viewer.c

LAST MODIFIED : 21 November 2001

DESCRIPTION :
Dialog for selecting nodes and viewing and/or editing field values. Works with
FE_node_selection to display the last selected node, or set it if entered in
this dialog.
==============================================================================*/
#include <Xm/Protocols.h>
#include <Xm/MwmUtil.h>
#include "general/debug.h"
#include "node/node_viewer_widget.h"
#include "node/node_viewer.h"
#include "node/node_viewer.uidh"
#include "select/select_finite_element.h"
#include "user_interface/gui_dialog_macros.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module variables
----------------
*/

#if defined (MOTIF)
static int node_viewer_hierarchy_open=0;
static MrmHierarchy node_viewer_hierarchy;
#endif /* defined (MOTIF) */

struct Node_viewer
/*******************************************************************************
LAST MODIFIED : 6 June 2001

DESCRIPTION :
Contains all the information carried by the node_viewer widget.
==============================================================================*/
{
	struct Computed_field_package *computed_field_package;
	struct Node_viewer **node_viewer_address;
	struct FE_node *node_copy;
	struct MANAGER(FE_node) *this_node_manager;
	void *this_node_manager_callback_id;
	struct MANAGER(FE_node) *actual_node_manager;
	void *actual_node_manager_callback_id;
	struct MANAGER(FE_element) *actual_element_manager;
	void *actual_element_manager_callback_id;
	struct FE_node_selection *node_selection;
	struct User_interface *user_interface;
	Widget viewer_form,viewer_widget,select_form,select_widget;
	Widget widget,window_shell;
}; /* node_viewer_struct */

/*
Module functions
----------------
*/

DECLARE_DIALOG_IDENTIFY_FUNCTION(node_viewer,Node_viewer,select_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(node_viewer,Node_viewer,viewer_form)

static int Node_viewer_set_viewer_node(struct Node_viewer *node_viewer)
/*******************************************************************************
LAST MODIFIED : 5 December 2000

DESCRIPTION :
Gets the current node from the select widget, makes a copy of it if not NULL,
and passes it to the node_viewer.
==============================================================================*/
{
	int return_code;
	struct FE_node *node, *node_copy, *temp_node;

	ENTER(Node_viewer_set_viewer_node);
	if (node_viewer)
	{
		if (node = SELECT_GET_SELECT_ITEM(FE_node)(node_viewer->select_widget))
		{
			node_copy = CREATE(FE_node)(get_FE_node_cm_node_identifier(node), node);
		}
		else
		{
			node_copy = (struct FE_node *)NULL;
		}
		/* make sure previous node is around while setting new one */
		if (node_viewer->node_copy)
		{
			temp_node = ACCESS(FE_node)(node_viewer->node_copy);
		}
		else
		{
			temp_node = (struct FE_node *)NULL;
		}
		REACCESS(FE_node)(&(node_viewer->node_copy), node_copy);
		node_viewer_widget_set_node(node_viewer->viewer_widget, node_copy);
		if (temp_node)
		{
			DEACCESS(FE_node)(&temp_node);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_viewer_set_viewer_node.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_viewer_set_viewer_node */

static void Node_viewer_update_select_widget(Widget select_widget,
	void *node_viewer_void, void *node_void)
/*******************************************************************************
LAST MODIFIED : 5 December 2000

DESCRIPTION :
Callback for change of selected node from select widget. Clears the global node
selection and selects this node; Node_viewer_node_selection_change
then passes it to the viewer.
==============================================================================*/
{
	struct FE_node *node;
	struct Node_viewer *node_viewer;

	ENTER(Node_viewer_update_select_widget);
	USE_PARAMETER(select_widget);
	if (node_viewer = (struct Node_viewer *)node_viewer_void)
	{
		FE_node_selection_begin_cache(node_viewer->node_selection);
		FE_node_selection_clear(node_viewer->node_selection);
		if (node = (struct FE_node *)node_void)
		{
			FE_node_selection_select_node(node_viewer->node_selection, node);
		}
		else
		{
			Node_viewer_set_viewer_node(node_viewer);
		}
		FE_node_selection_end_cache(node_viewer->node_selection);
	}
	else
	{
		display_message(WARNING_MESSAGE,
			"Node_viewer_update_select_widget.  Invalid argument(s)");
	}
	LEAVE;
} /* Node_viewer_update_select_widget */

static void Node_viewer_node_selection_change(
	struct FE_node_selection *node_selection,
	struct FE_node_selection_changes *changes,void *node_viewer_void)
/*******************************************************************************
LAST MODIFIED : 5 December 2000

DESCRIPTION :
Callback for change in the global node selection.
==============================================================================*/
{
	struct FE_node *last_selected_node, *select_node;
	struct Node_viewer *node_viewer;

	ENTER(Node_viewer_node_selection_change);
	if (node_selection&&changes&&
		(node_viewer=(struct Node_viewer *)node_viewer_void))
	{
		/* get the last selected node and put it in the viewer */
		select_node = SELECT_GET_SELECT_ITEM(FE_node)(node_viewer->select_widget);
		if ((last_selected_node = FIRST_OBJECT_IN_LIST_THAT(FE_node)(
			(LIST_CONDITIONAL_FUNCTION(FE_node) *)NULL, (void *)NULL,
			changes->newly_selected_node_list)) ||
			((last_selected_node = select_node) && IS_OBJECT_IN_LIST(FE_node)(
				select_node, FE_node_selection_get_node_list(node_selection))) ||
			(last_selected_node = FIRST_OBJECT_IN_LIST_THAT(FE_node)(
				(LIST_CONDITIONAL_FUNCTION(FE_node) *)NULL, (void *)NULL,
				FE_node_selection_get_node_list(node_selection))))
		{
			if (last_selected_node != select_node)
			{
				SELECT_SET_SELECT_ITEM(FE_node)(node_viewer->select_widget,
					last_selected_node);
			}
			Node_viewer_set_viewer_node(node_viewer);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_viewer_node_selection_change.  Invalid argument(s)");
	}
	LEAVE;
} /* Node_viewer_node_selection_change */

static void Node_viewer_this_node_change(
	struct MANAGER_MESSAGE(FE_node) *message, void *node_viewer_void)
/*******************************************************************************
LAST MODIFIED : 6 June 2001

DESCRIPTION :
Callback from this_node_manager for changes to nodes. If the currently selected
node has changed, re-send to viewer.
Note that we do not have to handle add, delete and identifier change messages
here as the select widget does this for us. Only changes to the content of the
object cause updates.
==============================================================================*/
{
	struct Node_viewer *node_viewer;

	ENTER(Node_viewer_this_node_change);
	if (message && (node_viewer = (struct Node_viewer *)node_viewer_void))
	{
		switch (message->change)
		{
			case MANAGER_CHANGE_IDENTIFIER(FE_node):
			case MANAGER_CHANGE_OBJECT(FE_node):
			case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(FE_node):
			{
				if (IS_OBJECT_IN_LIST(FE_node)(
					SELECT_GET_SELECT_ITEM(FE_node)(node_viewer->select_widget),
					message->changed_object_list))
				{
					Node_viewer_set_viewer_node(node_viewer);
				}
			} break;
			case MANAGER_CHANGE_ADD(FE_node):
			case MANAGER_CHANGE_REMOVE(FE_node):
			{
				/* do nothing */
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_viewer_this_node_change.  Invalid argument(s)");
	}
	LEAVE;
} /* Node_viewer_this_node_change */

static void Node_viewer_actual_node_change(
	struct MANAGER_MESSAGE(FE_node) *message, void *node_viewer_void)
/*******************************************************************************
LAST MODIFIED : 6 June 2001

DESCRIPTION :
Callback from the actual_node_manager for changes to nodes. If the node is
embedded in any elements using nodes that have changed, updates node in editor.
==============================================================================*/
{
	struct FE_node_is_embedded_in_changed_element_data embedded_data;
	struct Node_viewer *node_viewer;

	ENTER(Node_viewer_actual_node_change);
	if (message && (node_viewer = (struct Node_viewer *)node_viewer_void))
	{
		switch (message->change)
		{
			case MANAGER_CHANGE_OBJECT(FE_node):
			case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(FE_node):
			{
				embedded_data.changed_element_list = (struct LIST(FE_element) *)NULL;
				embedded_data.changed_node_list = message->changed_object_list;
				if (FE_node_is_embedded_in_changed_element(
					SELECT_GET_SELECT_ITEM(FE_node)(node_viewer->select_widget),
					(void *)&embedded_data))
				{
					Node_viewer_set_viewer_node(node_viewer);
				}
			} break;
			case MANAGER_CHANGE_ADD(FE_node):
			case MANAGER_CHANGE_REMOVE(FE_node):
			case MANAGER_CHANGE_IDENTIFIER(FE_node):
			{
				/* do nothing */
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_viewer_actual_node_change.  Invalid argument(s)");
	}
	LEAVE;
} /* Node_viewer_actual_node_change */

static void Node_viewer_actual_element_change(
	struct MANAGER_MESSAGE(FE_element) *message, void *node_viewer_void)
/*******************************************************************************
LAST MODIFIED : 6 June 2001

DESCRIPTION :
Callback from the actual_element_manager for changes to elements. If the node is
embedded in any elements that have changed, updates node in editor.
==============================================================================*/
{
	struct FE_node_is_embedded_in_changed_element_data embedded_data;
	struct Node_viewer *node_viewer;

	ENTER(Node_viewer_actual_element_change);
	if (message && (node_viewer = (struct Node_viewer *)node_viewer_void))
	{
		switch (message->change)
		{
			case MANAGER_CHANGE_OBJECT(FE_element):
			case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(FE_element):
			{
				embedded_data.changed_element_list = message->changed_object_list;
				embedded_data.changed_node_list = (struct LIST(FE_node) *)NULL;
				if (FE_node_is_embedded_in_changed_element(
					SELECT_GET_SELECT_ITEM(FE_node)(node_viewer->select_widget),
					(void *)&embedded_data))
				{
					Node_viewer_set_viewer_node(node_viewer);
				}
			} break;
			case MANAGER_CHANGE_ADD(FE_element):
			case MANAGER_CHANGE_REMOVE(FE_element):
			case MANAGER_CHANGE_IDENTIFIER(FE_element):
			{
				/* do nothing */
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_viewer_actual_element_change.  Invalid argument(s)");
	}
	LEAVE;
} /* Node_viewer_actual_element_change */

static int Node_viewer_apply_changes(struct Node_viewer *node_viewer)
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Makes the node change global.
==============================================================================*/
{
	int return_code;

	ENTER(Node_viewer_apply_changes);
	if (node_viewer)
	{
		if (node_viewer->node_copy)
		{
			return_code=MANAGER_MODIFY_NOT_IDENTIFIER(FE_node,cm_node_identifier)(
				SELECT_GET_SELECT_ITEM(FE_node)(node_viewer->select_widget),
				node_viewer->node_copy,node_viewer->this_node_manager);
		}
		else
		{
			return_code=1;
		}
	}
	else
	{
		display_message(WARNING_MESSAGE,
			"Node_viewer_apply_changes.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_viewer_apply_changes */

static void Node_viewer_ok_CB(Widget widget,void *node_viewer_void,
	void *call_data)
/*******************************************************************************
LAST MODIFIED : 16 June 2000

DESCRIPTION :
Callback for OK button. Applies changes to the node in the editor, and if this
is successful, closes the dialog.
==============================================================================*/
{
	struct Node_viewer *node_viewer;

	ENTER(Node_viewer_ok_CB);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (node_viewer=(struct Node_viewer *)node_viewer_void)
	{
		if (Node_viewer_apply_changes(node_viewer))
		{
			DESTROY(Node_viewer)(node_viewer->node_viewer_address);
		}
	}
	else
	{
		display_message(WARNING_MESSAGE,
			"Node_viewer_ok_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Node_viewer_ok_CB */

static void Node_viewer_apply_CB(Widget widget,void *node_viewer_void,
	void *call_data)
/*******************************************************************************
LAST MODIFIED : 16 June 2000

DESCRIPTION :
Callback for Apply button. Applies changes to the node in the editor.
==============================================================================*/
{
	struct Node_viewer *node_viewer;

	ENTER(Node_viewer_apply_CB);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (node_viewer=(struct Node_viewer *)node_viewer_void)
	{
		Node_viewer_apply_changes(node_viewer);
	}
	else
	{
		display_message(WARNING_MESSAGE,
			"Node_viewer_apply_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Node_viewer_apply_CB */

static void Node_viewer_revert_CB(Widget widget,void *node_viewer_void,
	void *call_data)
/*******************************************************************************
LAST MODIFIED : 16 June 2000

DESCRIPTION :
Callback for Revert button. Sends global node values back to the
editor widget, undoing any modifications.
==============================================================================*/
{
	struct Node_viewer *node_viewer;

	ENTER(Node_viewer_revert_CB);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (node_viewer=(struct Node_viewer *)node_viewer_void)
	{
		Node_viewer_set_viewer_node(node_viewer);
	}
	else
	{
		display_message(WARNING_MESSAGE,
			"Node_viewer_revert_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Node_viewer_revert_CB */

static void Node_viewer_cancel_CB(Widget widget,void *node_viewer_void,
	void *call_data)
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Callback from the Cancel button.
Also called when "close" is selected from the window menu, or it is double
clicked. How this is made to occur is as follows. The dialog has its
XmNdeleteResponse == XmDO_NOTHING, and a window manager protocol callback for
WM_DELETE_WINDOW has been set up with XmAddWMProtocolCallback to call this
function in response to the close command. See CREATE for more details.
==============================================================================*/
{
	struct Node_viewer *node_viewer;

	ENTER(Node_viewer_cancel_CB);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (node_viewer=(struct Node_viewer *)node_viewer_void)
	{
		DESTROY(Node_viewer)(node_viewer->node_viewer_address);
	}
	else
	{
		display_message(WARNING_MESSAGE,
			"Node_viewer_cancel_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Node_viewer_cancel_CB */

/*
Global functions
----------------
*/

struct Node_viewer *CREATE(Node_viewer)(
	struct Node_viewer **node_viewer_address,
	char *dialog_title,
	struct FE_node *initial_node,
	struct MANAGER(FE_node) *this_node_manager,
	struct MANAGER(FE_node) *actual_node_manager,
	struct MANAGER(FE_element) *actual_element_manager,
	struct FE_node_selection *node_selection,
	struct Computed_field_package *computed_field_package,
	struct Time_object *time_object, struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 22 November 2001

DESCRIPTION :
Creates a dialog for choosing nodes and displaying and editing their fields.
Nodes, starting with <initial_node> may be chosen from <this_node_manager>.
Pass <initial_data> and <data_manager> in these parameters for data viewer.
Since both nodes and data can depend on embedded fields, the
<actual_node_manager> and <actual_element_manager> also need to be passed.
==============================================================================*/
{
	Atom WM_DELETE_WINDOW;
	int init_widgets;
	MrmType node_viewer_dialog_class;
	struct Callback_data callback;
	struct Node_viewer *node_viewer;
	static MrmRegisterArg callback_list[]=
	{
		{"node_viewer_id_select_form",(XtPointer)
			DIALOG_IDENTIFY(node_viewer,select_form)},
		{"node_viewer_id_viewer_form",(XtPointer)
			DIALOG_IDENTIFY(node_viewer,viewer_form)},
		{"node_viewer_ok_CB",(XtPointer)Node_viewer_ok_CB},
		{"node_viewer_apply_CB",(XtPointer)Node_viewer_apply_CB},
		{"node_viewer_revert_CB",(XtPointer)Node_viewer_revert_CB},
		{"node_viewer_cancel_CB",(XtPointer)Node_viewer_cancel_CB}
	};
	static MrmRegisterArg identifier_list[]=
	{
		{"node_viewer_structure",(XtPointer)NULL}
	};

	ENTER(CREATE(Node_viewer));
	node_viewer = (struct Node_viewer *)NULL;
	if (node_viewer_address && dialog_title && this_node_manager &&
		actual_node_manager && actual_element_manager && node_selection &&
		computed_field_package && user_interface)
	{
		if (MrmOpenHierarchy_base64_string(node_viewer_uidh,
			&node_viewer_hierarchy,&node_viewer_hierarchy_open))
		{
			/* allocate memory */
			if (ALLOCATE(node_viewer,struct Node_viewer,1))
			{
				/* initialise the structure */
				if ((!initial_node) ||
					(!IS_MANAGED(FE_node)(initial_node, this_node_manager)))
				{
					if (!(initial_node=FIRST_OBJECT_IN_LIST_THAT(FE_node)(
						(LIST_CONDITIONAL_FUNCTION(FE_node) *)NULL,(void *)NULL,
						FE_node_selection_get_node_list(node_selection))))
					{
						if (initial_node=FIRST_OBJECT_IN_MANAGER_THAT(FE_node)(
							(MANAGER_CONDITIONAL_FUNCTION(FE_node) *)NULL,(void *)NULL,
							this_node_manager))
						{
							/* select the node to be displayed in dialog; note this is ok
								 here as we are not receiving selection callbacks yet */
							FE_node_selection_select_node(node_selection,initial_node);
						}
					}
				}
				if (initial_node)
				{
					node_viewer->node_copy = ACCESS(FE_node)(
						CREATE(FE_node)(get_FE_node_cm_node_identifier(initial_node),
							initial_node));
				}
				else
				{
					node_viewer->node_copy = (struct FE_node *)NULL;
				}
				node_viewer->node_viewer_address = node_viewer_address;
				node_viewer->this_node_manager = this_node_manager;
				node_viewer->this_node_manager_callback_id = (void *)NULL;
				node_viewer->actual_node_manager = actual_node_manager;
				node_viewer->actual_node_manager_callback_id = (void *)NULL;
				node_viewer->actual_element_manager = actual_element_manager;
				node_viewer->actual_element_manager_callback_id = (void *)NULL;
				node_viewer->node_selection=node_selection;
				node_viewer->computed_field_package=computed_field_package;
				node_viewer->user_interface=user_interface;
				/* initialise widgets */
				node_viewer->viewer_form=(Widget)NULL;
				node_viewer->viewer_widget=(Widget)NULL;
				node_viewer->select_form=(Widget)NULL;
				node_viewer->select_widget=(Widget)NULL;
				node_viewer->widget=(Widget)NULL;
				node_viewer->window_shell=(Widget)NULL;
				/* get callbacks from global node selection */
				FE_node_selection_add_callback(node_selection,
					Node_viewer_node_selection_change,(void *)node_viewer);
				/* make the dialog shell */
				if (node_viewer->window_shell=XtVaCreatePopupShell(dialog_title,
					topLevelShellWidgetClass,
					user_interface->application_shell,
					XmNdeleteResponse,XmDO_NOTHING,
					XmNmwmDecorations,MWM_DECOR_ALL,
					XmNmwmFunctions,MWM_FUNC_ALL,
					/*XmNtransient,FALSE,*/
					XmNallowShellResize,False,
					XmNtitle,dialog_title,
					NULL))
				{
					/* Set up window manager callback for close window message */
					WM_DELETE_WINDOW=
						XmInternAtom(XtDisplay(node_viewer->window_shell),
							"WM_DELETE_WINDOW",False);
					XmAddWMProtocolCallback(node_viewer->window_shell,
						WM_DELETE_WINDOW,Node_viewer_cancel_CB,node_viewer);
					/* Register the shell with the busy signal list */
					create_Shell_list_item(&(node_viewer->window_shell),
						user_interface);
					/* register the callbacks */
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
						node_viewer_hierarchy,callback_list,XtNumber(callback_list)))
					{
						/* assign and register the identifiers */
						identifier_list[0].value=(XtPointer)node_viewer;
						if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
							node_viewer_hierarchy,identifier_list,
							XtNumber(identifier_list)))
						{
							/* fetch position window widget */
							if (MrmSUCCESS==MrmFetchWidget(node_viewer_hierarchy,
								"node_viewer",node_viewer->window_shell,
								&(node_viewer->widget),&node_viewer_dialog_class))
							{
								XtManageChild(node_viewer->widget);
								init_widgets=1;
								if (!CREATE_SELECT_WIDGET(FE_node)(
									&node_viewer->select_widget,
									node_viewer->select_form,SELECT_TEXT,
									initial_node,this_node_manager))
								{
									init_widgets=0;
								}
								if (!create_node_viewer_widget(
									&(node_viewer->viewer_widget),
									node_viewer->viewer_form,
									computed_field_package,
									node_viewer->node_copy, time_object, user_interface))
								{
									init_widgets=0;
								}
								if (init_widgets)
								{
									/* get callbacks from the node selector */
									callback.procedure = Node_viewer_update_select_widget;
									callback.data = node_viewer;
									SELECT_SET_UPDATE_CB(FE_node)(
										node_viewer->select_widget, &callback);
									/* get callbacks from managers */
									node_viewer->this_node_manager_callback_id =
										MANAGER_REGISTER(FE_node)(Node_viewer_this_node_change,
											node_viewer, node_viewer->this_node_manager);
									node_viewer->actual_node_manager_callback_id =
										MANAGER_REGISTER(FE_node)(Node_viewer_actual_node_change,
											node_viewer, node_viewer->actual_node_manager);
									node_viewer->actual_element_manager_callback_id =
										MANAGER_REGISTER(FE_element)(
											Node_viewer_actual_element_change, node_viewer,
											node_viewer->actual_element_manager);
									/* bring up the widget */
									XtRealizeWidget(node_viewer->window_shell);
									XtPopup(node_viewer->window_shell,XtGrabNone);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"CREATE(Node_viewer).  Could not init widgets");
									DESTROY(Node_viewer)(&node_viewer);
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,"CREATE(Node_viewer).  "
									"Could not fetch node_viewer");
								DESTROY(Node_viewer)(&node_viewer);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"CREATE(Node_viewer).  Could not register identifiers");
							DESTROY(Node_viewer)(&node_viewer);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"CREATE(Node_viewer).  Could not register callbacks");
						DESTROY(Node_viewer)(&node_viewer);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"CREATE(Node_viewer).  Could not create popup shell.");
					DESTROY(Node_viewer)(&node_viewer);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"CREATE(Node_viewer).  Could not allocate node_viewer");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Node_viewer).  Could not open hierarchy");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Node_viewer).  Invalid argument(s)");
	}
	if (node_viewer_address)
	{
		*node_viewer_address=node_viewer;
	}
	LEAVE;

	return (node_viewer);
} /* CREATE(Node_viewer) */

int DESTROY(Node_viewer)(struct Node_viewer **node_viewer_address)
/*******************************************************************************
LAST MODIFIED : 6 June 2001

DESCRIPTION:
Destroys the Node_viewer. See also Node_viewer_close_CB.
==============================================================================*/
{
	int return_code;
	struct Node_viewer *node_viewer;

	ENTER(DESTROY(node_viewer));
	if (node_viewer_address&&
		(node_viewer= *node_viewer_address))
	{
		/* end callbacks from managers */
		if (node_viewer->this_node_manager_callback_id)
		{
			MANAGER_DEREGISTER(FE_node)(node_viewer->this_node_manager_callback_id,
				node_viewer->this_node_manager);
		}
		if (node_viewer->actual_node_manager_callback_id)
		{
			MANAGER_DEREGISTER(FE_node)(node_viewer->actual_node_manager_callback_id,
				node_viewer->actual_node_manager);
		}
		if (node_viewer->actual_element_manager_callback_id)
		{
			MANAGER_DEREGISTER(FE_element)(
				node_viewer->actual_element_manager_callback_id,
				node_viewer->actual_element_manager);
		}
		/* end callbacks from global node selection */
		FE_node_selection_remove_callback(node_viewer->node_selection,
			Node_viewer_node_selection_change,(void *)node_viewer);
		/* deaccess the local node_copy */
		REACCESS(FE_node)(&(node_viewer->node_copy),(struct FE_node *)NULL);
		if (node_viewer->window_shell)
		{
			destroy_Shell_list_item_from_shell(&(node_viewer->window_shell),
				node_viewer->user_interface);
			XtDestroyWidget(node_viewer->window_shell);
		}
		DEALLOCATE(*node_viewer_address);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Node_viewer).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Node_viewer) */

struct FE_node *Node_viewer_get_node(struct Node_viewer *node_viewer)
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Returns the node currently in the <node_viewer>.
==============================================================================*/
{
	struct FE_node *node;

	ENTER(node_viewer_get_node);
	if (node_viewer)
	{
		node=SELECT_GET_SELECT_ITEM(FE_node)(node_viewer->select_widget);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"node_viewer_get_node.  Missing dialog.");
		node=(struct FE_node *)NULL;
	}
	LEAVE;

	return (node);
} /* node_viewer_get_node */

int Node_viewer_set_node(struct Node_viewer *node_viewer,struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Makes <node> the current_node in <node_viewer>.
==============================================================================*/
{
	int return_code;

	ENTER(Node_viewer_set_node);
	if (node_viewer&&node)
	{
		return_code=
			SELECT_SET_SELECT_ITEM(FE_node)(node_viewer->select_widget,node)&&
			Node_viewer_set_viewer_node(node_viewer);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_viewer_set_node.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_viewer_set_node */

int Node_viewer_bring_window_to_front(struct Node_viewer *node_viewer)
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Pops the window for <node_viewer> to the front of those visible.
??? De-iconify as well?
==============================================================================*/
{
	int return_code;

	ENTER(Node_viewer_bring_window_to_front);
	if (node_viewer)
	{
		XtPopup(node_viewer->window_shell,XtGrabNone);
		XtVaSetValues(node_viewer->window_shell,XmNiconic,False,NULL);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_viewer_bring_window_to_front.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_viewer_bring_window_to_front */
