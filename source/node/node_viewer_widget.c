/*******************************************************************************
FILE : node_viewer_widget.c

LAST MODIFIED : 30 May 2001

DESCRIPTION :
Widget for editing the contents of a node with multiple text fields, visible
one field at a time. Any computed field may be viewed. Most can be modified too.
Note the node passed to this widget should be a non-managed local copy.
==============================================================================*/
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Mrm/MrmPublic.h>
#include "choose/choose_computed_field.h"
#include "general/debug.h"
#include "node/node_field_viewer_widget.h"
#include "node/node_viewer_widget.h"
#include "node/node_viewer_widget.uidh"
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

/*
Module types
------------
*/

struct Node_viewer_widget_struct
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Contains all the information carried by the node_viewer widget.
Note that we just hold a pointer to the node_viewer, and must access and
deaccess it.
==============================================================================*/
{
	struct Computed_field_package *computed_field_package;
	void *computed_field_manager_callback_id;
	struct FE_node *current_node,*template_node;
	struct Callback_data update_callback;
	Widget choose_field_form,choose_field_widget,field_viewer_form,
		field_viewer_widget;
	Widget widget,*widget_address,widget_parent;
}; /* Node_viewer_widget_struct */

/*
Module functions
----------------
*/

DECLARE_DIALOG_IDENTIFY_FUNCTION(node_viewer_widget, \
	Node_viewer_widget_struct,choose_field_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(node_viewer_widget, \
	Node_viewer_widget_struct,field_viewer_form)

static void node_viewer_widget_update(
	struct Node_viewer_widget_struct *node_viewer)
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Tells the parent dialog the current_node has changed; sends a pointer to the
current node.
==============================================================================*/
{
	ENTER(node_viewer_widget_update);
	if (node_viewer)
	{
		if (node_viewer->update_callback.procedure)
		{
			/* now call the procedure with the user data and a NULL pointer */
			(node_viewer->update_callback.procedure)(node_viewer->widget,
				node_viewer->update_callback.data,node_viewer->current_node);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"node_viewer_widget_update.  Invalid argument(s)");
	}
	LEAVE;
} /* node_viewer_widget_update */

static void node_viewer_widget_destroy_CB(Widget widget,void *node_viewer_void,
	void *call_data)
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Callback for when the node_viewer widget is destroyed. Tidies up all dynamic
memory allocations and pointers.
==============================================================================*/
{
	struct Node_viewer_widget_struct *node_viewer;

	ENTER(node_viewer_widget_destroy_CB);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (node_viewer=(struct Node_viewer_widget_struct *)node_viewer_void)
	{
		REACCESS(FE_node)(&(node_viewer->template_node),(struct FE_node *)NULL);
		if (node_viewer->computed_field_manager_callback_id)
		{
			MANAGER_DEREGISTER(Computed_field)(
				node_viewer->computed_field_manager_callback_id,
				Computed_field_package_get_computed_field_manager(
					node_viewer->computed_field_package));
		}
		*(node_viewer->widget_address)=(Widget)NULL;
		DEALLOCATE(node_viewer);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"node_viewer_widget_destroy_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* node_viewer_widget_destroy_CB */

static void node_viewer_widget_update_choose_field(Widget widget,
	void *node_viewer_void,void *field_void)
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Callback for change of iso_scalar field.
==============================================================================*/
{
	struct Computed_field *field;
	struct Node_viewer_widget_struct *node_viewer;

	ENTER(node_viewer_widget_update_choose_field);
	USE_PARAMETER(widget);
	if ((node_viewer=(struct Node_viewer_widget_struct *)node_viewer_void)&&
		(field=(struct Computed_field *)field_void))
	{
		node_field_viewer_widget_set_node_field(node_viewer->field_viewer_widget,
			node_viewer->current_node,field);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"node_viewer_widget_update_choose_field.  Invalid argument(s)");
	}
	LEAVE;
} /* node_viewer_widget_update_choose_field */

static void node_viewer_widget_computed_field_change(
	struct MANAGER_MESSAGE(Computed_field) *message,void *node_viewer_void)
/*******************************************************************************
LAST MODIFIED : 30 May 2001

DESCRIPTION :
One or more of the computed_fields have changed in the manager. If the
current_field in the <node_viewer> has changed, re-send it to the
node_field_viewer to update widgets/values etc.
Note that delete/add messages are handled by the field chooser.
==============================================================================*/
{
	struct Computed_field *field;
	struct Node_viewer_widget_struct *node_viewer;

	ENTER(node_viewer_widget_computed_field_change);
	if (message &&
		(node_viewer = (struct Node_viewer_widget_struct *)node_viewer_void))
	{
		switch (message->change)
		{
			case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Computed_field):
			case MANAGER_CHANGE_OBJECT(Computed_field):
			{
				field = CHOOSE_OBJECT_GET_OBJECT(Computed_field)(
					node_viewer->choose_field_widget);
				if (IS_OBJECT_IN_LIST(Computed_field)(field,
					message->changed_object_list))
				{
					node_field_viewer_widget_set_node_field(
						node_viewer->field_viewer_widget, node_viewer->current_node, field);
				}
			} break;
			case MANAGER_CHANGE_ADD(Computed_field):
			case MANAGER_CHANGE_REMOVE(Computed_field):
			case MANAGER_CHANGE_IDENTIFIER(Computed_field):
			{
				/* do nothing */
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"node_viewer_widget_computed_field_change.  Invalid argument(s)");
	}
	LEAVE;
} /* node_viewer_widget_computed_field_change */

static void node_viewer_widget_update_field_viewer(
	Widget node_field_viewer_widget,void *node_viewer_void,void *call_data)
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Callback for change of values in the <node_field_viewer_widget>. Notify parent
dialog of change to node.
==============================================================================*/
{
	struct Node_viewer_widget_struct *node_viewer;

	ENTER(node_viewer_widget_update_field_viewer);
	USE_PARAMETER(node_field_viewer_widget);
	USE_PARAMETER(call_data);
	if (node_viewer=(struct Node_viewer_widget_struct *)node_viewer_void)
	{
		node_viewer_widget_update(node_viewer);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"node_viewer_widget_update_field_viewer.  Invalid argument(s)");
	}
	LEAVE;
} /* node_viewer_widget_update_field_viewer */

/*
Global functions
----------------
*/

Widget create_node_viewer_widget(Widget *node_viewer_widget_address,
	Widget parent,struct Computed_field_package *computed_field_package,
	struct FE_node *initial_node)
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Creates a widget for displaying and editing the contents of <initial_node>. Can
also pass a NULL node here and use the Node_viewer_widget_set_node function
instead. <initial_node> should be a local copy of a global node; up to the
parent dialog to make changes global.
==============================================================================*/
{
	int init_widgets;
	MANAGER_CONDITIONAL_FUNCTION(Computed_field)
		*choose_field_conditional_function;
	MrmType node_viewer_dialog_class;
	struct Computed_field *initial_field;
	struct Callback_data callback;
	struct Node_viewer_widget_struct *node_viewer=NULL;
	static MrmRegisterArg callback_list[]=
	{
		{"node_vw_id_choose_field_form",(XtPointer)
			DIALOG_IDENTIFY(node_viewer_widget,choose_field_form)},
		{"node_vw_id_field_viewer_form",(XtPointer)
			DIALOG_IDENTIFY(node_viewer_widget,field_viewer_form)},
		{"node_vw_destroy_CB",(XtPointer)node_viewer_widget_destroy_CB}
	};
	static MrmRegisterArg identifier_list[]=
	{
		{"node_vw_structure",(XtPointer)NULL}
	};
	Widget return_widget;

	ENTER(create_node_viewer_widget);
	return_widget=(Widget)NULL;
	if (node_viewer_widget_address&&parent&&computed_field_package)
	{
		if (MrmOpenHierarchy_base64_string(node_viewer_widget_uidh,
			&node_viewer_hierarchy,&node_viewer_hierarchy_open))
		{
			/* allocate memory */
			if (ALLOCATE(node_viewer,struct Node_viewer_widget_struct,1))
			{
				/* initialise the structure */
				if (initial_node)
				{
					choose_field_conditional_function=
						Computed_field_is_defined_at_node_conditional;
				}
				else
				{
					choose_field_conditional_function=
						(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
				}
				initial_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
					choose_field_conditional_function,(void *)initial_node,
					Computed_field_package_get_computed_field_manager(
						computed_field_package));
				node_viewer->computed_field_package=computed_field_package;
				node_viewer->computed_field_manager_callback_id=(void *)NULL;
				node_viewer->current_node=initial_node;
				if (initial_node)
				{
					node_viewer->template_node=ACCESS(FE_node)(
						CREATE(FE_node)(0,initial_node));
				}
				else
				{
					node_viewer->template_node=(struct FE_node *)NULL;
				}
				node_viewer->update_callback.procedure=(Callback_procedure *)NULL;
				node_viewer->update_callback.data=NULL;
				/* initialise the widgets */
				node_viewer->widget=(Widget)NULL;
				node_viewer->widget_address=node_viewer_widget_address;
				node_viewer->widget_parent=parent;
				node_viewer->choose_field_form=(Widget)NULL;
				node_viewer->field_viewer_form=(Widget)NULL;
				node_viewer->field_viewer_form=(Widget)NULL;
				node_viewer->field_viewer_widget=(Widget)NULL;
				/* register the callbacks */
				if (MrmSUCCESS==MrmRegisterNamesInHierarchy(node_viewer_hierarchy,
					callback_list,XtNumber(callback_list)))
				{
					/* assign and register the identifiers */
					identifier_list[0].value=(XtPointer)node_viewer;
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(node_viewer_hierarchy,
						identifier_list,XtNumber(identifier_list)))
					{
						/* fetch position window widget */
						if (MrmSUCCESS==MrmFetchWidget(node_viewer_hierarchy,
							"node_viewer_widget",node_viewer->widget_parent,
							&(node_viewer->widget),&node_viewer_dialog_class))
						{
							XtManageChild(node_viewer->widget);
							/* create subwidgets */
							init_widgets=1;
							if (!(node_viewer->choose_field_widget=
								CREATE_CHOOSE_OBJECT_WIDGET(Computed_field)(
								node_viewer->choose_field_form,initial_field,
								Computed_field_package_get_computed_field_manager(
									computed_field_package),
								choose_field_conditional_function,
								(void *)(node_viewer->template_node))))
							{
								init_widgets=0;
							}
							if (!(create_node_field_viewer_widget(
								&(node_viewer->field_viewer_widget),
								node_viewer->field_viewer_form,
								initial_node,initial_field)))
							{
								init_widgets=0;
							}
							if (init_widgets)
							{
								callback.procedure=node_viewer_widget_update_field_viewer;
								callback.data=node_viewer;
								node_field_viewer_widget_set_callback(
									node_viewer->field_viewer_widget,&callback);
								callback.procedure=node_viewer_widget_update_choose_field;
								CHOOSE_OBJECT_SET_CALLBACK(Computed_field)(
									node_viewer->choose_field_widget,&callback);
								node_viewer->computed_field_manager_callback_id=
									MANAGER_REGISTER(Computed_field)(
										node_viewer_widget_computed_field_change,
										(void *)node_viewer,
										Computed_field_package_get_computed_field_manager(
											node_viewer->computed_field_package));
								return_widget=node_viewer->widget;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"create_node_viewer_widget.  Could not create subwidgets");
								DEALLOCATE(node_viewer);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"create_node_viewer_widget.  "
								"Could not fetch node_viewer dialog");
							DEALLOCATE(node_viewer);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"create_node_viewer_widget.  "
							"Could not register identifiers");
						DEALLOCATE(node_viewer);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_node_viewer_widget.  Could not register callbacks");
					DEALLOCATE(node_viewer);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"create_node_viewer_widget.  "
					"Could not allocate node_viewer widget structure");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_node_viewer_widget.  Could not open hierarchy");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_node_viewer_widget.  Invalid argument(s)");
	}
	if (node_viewer_widget_address)
	{
		*node_viewer_widget_address=return_widget;
	}
	LEAVE;

	return (return_widget);
} /* create_node_viewer_widget */

int node_viewer_widget_set_callback(Widget node_viewer_widget,
	struct Callback_data *callback)
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Sets the callback for updates when the contents of the node in the viewer have
changed.
==============================================================================*/
{
	int return_code;
	struct Node_viewer_widget_struct *node_viewer;

	ENTER(node_viewer_widget_set_callback);
	if (node_viewer_widget&&callback)
	{
		node_viewer=(struct Node_viewer_widget_struct *)NULL;
		/* get the node_viewer structure from the widget */
		XtVaGetValues(node_viewer_widget,XmNuserData,&node_viewer,NULL);
		if (node_viewer)
		{
			node_viewer->update_callback.procedure=callback->procedure;
			node_viewer->update_callback.data=callback->data;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"node_viewer_widget_set_callback.  Missing widget data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"node_viewer_widget_set_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* node_viewer_widget_set_callback */

struct FE_node *node_viewer_widget_get_node(Widget node_viewer_widget)
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Returns the node being edited in the <node_viewer_widget>.
==============================================================================*/
{
	struct FE_node *node;
	struct Node_viewer_widget_struct *node_viewer;

	ENTER(node_viewer_widget_get_node);
	if (node_viewer_widget)
	{
		node_viewer=(struct Node_viewer_widget_struct *)NULL;
		/* get the node_viewer structure from the widget */
		XtVaGetValues(node_viewer_widget,XmNuserData,&node_viewer,NULL);
		if (node_viewer)
		{
			node=node_viewer->current_node;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"node_viewer_widget_get_node.  Missing widget data");
			node=(struct FE_node *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"node_viewer_widget_get_node.  Invalid argument(s)");
		node=(struct FE_node *)NULL;
	}
	LEAVE;

	return (node);
} /* node_viewer_widget_get_node */

int node_viewer_widget_set_node(Widget node_viewer_widget,struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Sets the node being edited in the <node_viewer_widget>. Note that the viewer
works on the node itself, not a local copy. Hence, only pass unmanaged nodes to
this widget.
==============================================================================*/
{
	int change_conditional_function,return_code;
	MANAGER_CONDITIONAL_FUNCTION(Computed_field)
		*choose_field_conditional_function;
	struct Computed_field *field;
	struct FE_node *template_node;
	struct Node_viewer_widget_struct *node_viewer;

	ENTER(node_viewer_widget_set_node);
	if (node_viewer_widget)
	{
		node_viewer=(struct Node_viewer_widget_struct *)NULL;
		/* get the node_viewer structure from the widget */
		XtVaGetValues(node_viewer_widget,XmNuserData,&node_viewer,NULL);
		if (node_viewer)
		{
			change_conditional_function=0;
			if (node)
			{
				field=CHOOSE_OBJECT_GET_OBJECT(Computed_field)(
					node_viewer->choose_field_widget);
				if (!(node_viewer->template_node)||
					(!equivalent_computed_fields_at_nodes(node,
						node_viewer->template_node)))
				{
					choose_field_conditional_function=
						Computed_field_is_defined_at_node_conditional;
					change_conditional_function=1;
					if ((!field)||(!Computed_field_is_defined_at_node(field,node)))
					{
						field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
							choose_field_conditional_function,(void *)node,
							Computed_field_package_get_computed_field_manager(
								node_viewer->computed_field_package));
					}
					template_node=CREATE(FE_node)(0,node);
				}
			}
			else
			{
				field=(struct Computed_field *)NULL;
				if (node_viewer->current_node)
				{
					choose_field_conditional_function=
						(MANAGER_CONDITIONAL_FUNCTION(Computed_field) *)NULL;
					change_conditional_function=1;
					template_node=(struct FE_node *)NULL;
				}
			}
			node_viewer->current_node=node;
			if (change_conditional_function)
			{
				REACCESS(FE_node)(&(node_viewer->template_node),template_node);
				CHOOSE_OBJECT_CHANGE_CONDITIONAL_FUNCTION(Computed_field)(
					node_viewer->choose_field_widget,
					choose_field_conditional_function,
					(void *)node_viewer->template_node,field);
			}
			node_field_viewer_widget_set_node_field(
				node_viewer->field_viewer_widget,node,field);
			if (node)
			{
				XtManageChild(node_viewer->widget);
			}
			else
			{
				XtUnmanageChild(node_viewer->widget);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"node_viewer_widget_set_node.  Missing widget data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"node_viewer_widget_set_node.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* node_viewer_widget_set_node */

