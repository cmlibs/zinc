/*******************************************************************************
FILE : node_viewer.c

LAST MODIFIED : 20 March 2003

DESCRIPTION :
Dialog for selecting nodes and viewing and/or editing field values. Works with
FE_node_selection to display the last selected node, or set it if entered in
this dialog.
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
#include <Xm/Protocols.h>
#include <Xm/MwmUtil.h>
#include "choose/text_choose_fe_node.h"
#include "general/debug.h"
#include "node/node_viewer_widget.h"
#include "node/node_viewer.h"
#include "node/node_viewer.uidh"
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
	struct Cmiss_region *region;
	struct FE_region *fe_region;
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
LAST MODIFIED : 30 April 2003

DESCRIPTION :
Gets the current node from the select widget, makes a copy of it if not NULL,
and passes it to the node_viewer.
==============================================================================*/
{
	int return_code;
	struct FE_node *node, *node_copy;

	ENTER(Node_viewer_set_viewer_node);
	if (node_viewer)
	{
		if (node = TEXT_CHOOSE_FROM_FE_REGION_GET_OBJECT(FE_node)(
			node_viewer->select_widget))
		{
			node_copy = ACCESS(FE_node)(CREATE(FE_node)(get_FE_node_identifier(node),
				(struct FE_region *)NULL, node));
		}
		else
		{
			node_copy = (struct FE_node *)NULL;
		}
		REACCESS(FE_node)(&(node_viewer->node_copy), node_copy);
		node_viewer_widget_set_node(node_viewer->viewer_widget,
			node_viewer->fe_region, node_copy);
		if (node_copy)
		{
			DEACCESS(FE_node)(&node_copy);
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
		select_node = TEXT_CHOOSE_FROM_FE_REGION_GET_OBJECT(FE_node)(node_viewer->select_widget);
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
				TEXT_CHOOSE_FROM_FE_REGION_SET_OBJECT(FE_node)(node_viewer->select_widget,
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

static void Node_viewer_FE_region_change(struct FE_region *fe_region,
	struct FE_region_changes *changes, void *node_viewer_void)
/*******************************************************************************
LAST MODIFIED : 24 January 2003

DESCRIPTION :
Note that we do not have to handle add, delete and identifier change messages
here as the select widget does this for us. Only changes to the content of the
object cause updates.
==============================================================================*/
{
	enum CHANGE_LOG_CHANGE(FE_node) change;
	struct Node_viewer *node_viewer;

	ENTER(Node_viewer_Cmiss_region_change);
	if (fe_region && changes && 
		(node_viewer = (struct Node_viewer *)node_viewer_void))
	{
		if (CHANGE_LOG_QUERY(FE_node)(changes->fe_node_changes,
			TEXT_CHOOSE_FROM_FE_REGION_GET_OBJECT(FE_node)(node_viewer->select_widget),
			&change))
		{
			if (change | CHANGE_LOG_OBJECT_CHANGED(FE_node))
			{
				Node_viewer_set_viewer_node(node_viewer);
			}
		}		
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_viewer_Cmiss_region_change.  Invalid argument(s)");
	}
	LEAVE;
} /* Node_viewer_Cmiss_region_change */

static int Node_viewer_apply_changes(struct Node_viewer *node_viewer)
/*******************************************************************************
LAST MODIFIED : 27 March 2003

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
			if (FE_region_merge_FE_node(node_viewer->fe_region,
				node_viewer->node_copy))
			{
				return_code = 1;
			}
			else
			{
				return_code = 0;
			}
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
	struct Cmiss_region *region,
	struct FE_node_selection *node_selection,
	struct Computed_field_package *computed_field_package,
	struct Time_object *time_object, struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 30 April 2003

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
	if (node_viewer_address && dialog_title && region && node_selection &&
		computed_field_package && user_interface)
	{
		if (MrmOpenHierarchy_base64_string(node_viewer_uidh,
			&node_viewer_hierarchy,&node_viewer_hierarchy_open))
		{
			/* allocate memory */
			if (ALLOCATE(node_viewer,struct Node_viewer,1))
			{
				node_viewer->node_copy = (struct FE_node *)NULL;
				node_viewer->node_viewer_address = node_viewer_address;
				node_viewer->region = region;
				node_viewer->fe_region = Cmiss_region_get_FE_region(region);
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
				if (!initial_node)
				{
					if (!(initial_node = FIRST_OBJECT_IN_LIST_THAT(FE_node)(
						(LIST_CONDITIONAL_FUNCTION(FE_node) *)NULL,(void *)NULL,
						FE_node_selection_get_node_list(node_selection))))
					{
						if (initial_node = FE_region_get_first_FE_node_that(
							node_viewer->fe_region,
							(LIST_CONDITIONAL_FUNCTION(FE_node) *)NULL, (void *)NULL))
						{
							/* select the node to be displayed in dialog; note this is ok
								 here as we are not receiving selection callbacks yet */
							FE_node_selection_select_node(node_selection,initial_node);
						}
					}
				}
				/* get callbacks from global node selection */
				FE_node_selection_add_callback(node_selection,
					Node_viewer_node_selection_change,(void *)node_viewer);
				/* make the dialog shell */
				if (node_viewer->window_shell=XtVaCreatePopupShell(dialog_title,
					topLevelShellWidgetClass,
					User_interface_get_application_shell(user_interface),
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
								if (!(node_viewer->select_widget = 
									CREATE_TEXT_CHOOSE_FROM_FE_REGION_WIDGET(FE_node)(
								   node_viewer->select_form, initial_node,
									node_viewer->fe_region,
									(LIST_CONDITIONAL_FUNCTION(FE_node) *)NULL, NULL,
									FE_node_to_node_string, FE_region_node_string_to_FE_node)))
								{
									init_widgets=0;
								}
								if (!create_node_viewer_widget(
									&(node_viewer->viewer_widget),
									node_viewer->viewer_form,
									computed_field_package,
									node_viewer->fe_region, (struct FE_node *)NULL,
									time_object, user_interface))
								{
									init_widgets=0;
								}
								if (init_widgets)
								{
									/* get callbacks from the node selector */
									callback.procedure = Node_viewer_update_select_widget;
									callback.data = node_viewer;
									TEXT_CHOOSE_FROM_FE_REGION_SET_CALLBACK(FE_node)(
										node_viewer->select_widget, &callback);
									/* get callbacks from fe_region */
									FE_region_add_callback(node_viewer->fe_region,
										Node_viewer_FE_region_change, (void *)node_viewer);
									Node_viewer_set_viewer_node(node_viewer);
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
		/* end callback from region */
		FE_region_remove_callback(node_viewer->fe_region,
			Node_viewer_FE_region_change, (void *)node_viewer);
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
		node=TEXT_CHOOSE_FROM_FE_REGION_GET_OBJECT(FE_node)(node_viewer->select_widget);
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
			TEXT_CHOOSE_FROM_FE_REGION_SET_OBJECT(FE_node)(node_viewer->select_widget,node)&&
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
