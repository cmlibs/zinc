/*******************************************************************************
FILE : transform_tool.c

LAST MODIFIED : 13 June 2000

DESCRIPTION :
Icon/tool representing the transform function on a graphics window.
Eventually use to store parameters for the transform function.
==============================================================================*/
#include "general/debug.h"
#include "interaction/interaction_volume.h"
#include "interaction/interactive_event.h"
#include "interaction/interactive_tool.h"
#include "interaction/interactive_tool_private.h"
#include "graphics/transform_tool.h"
#include "graphics/transform_tool.uidh"
#include "user_interface/message.h"

/*
Module variables
----------------
*/

#if defined (MOTIF)
static int transform_tool_hierarchy_open=0;
static MrmHierarchy transform_tool_hierarchy;
#endif /* defined (MOTIF) */

/*
Module types
------------
*/

static char Interactive_tool_transform_type_string[] = "Transform_tool";

struct Transform_tool
/*******************************************************************************
LAST MODIFIED : 12 June 2000

DESCRIPTION :
==============================================================================*/
{
	struct Interactive_tool *interactive_tool;
	int free_spin_flag;
}; /* struct Transform_tool */

struct Transform_tool_defaults
{
	Boolean free_spin;
};

/*
Module functions
----------------
*/

static Widget Transform_tool_make_interactive_tool_button(
	void *transform_tool_void,Widget parent)
/*******************************************************************************
LAST MODIFIED : 12 June 2000

DESCRIPTION :
Fetches a ToggleButton with an appropriate icon for the interactive tool
and as a child of <parent>.
==============================================================================*/
{
	MrmType transform_tool_dialog_class;
	struct Transform_tool *transform_tool;
	Widget widget;

	ENTER(Transform_tool_make_interactive_tool_button);
	widget=(Widget)NULL;
	if ((transform_tool=(struct Transform_tool *)transform_tool_void)&&parent)
	{
		if (MrmOpenHierarchy_base64_string(transform_tool_uidh,
			&transform_tool_hierarchy,&transform_tool_hierarchy_open))
		{
			if (MrmSUCCESS == MrmFetchWidget(transform_tool_hierarchy,
				"transform_tool_button",parent,&widget,&transform_tool_dialog_class))
			{
				XtVaSetValues(widget,XmNuserData,transform_tool->interactive_tool,NULL);
			}
			else
			{
				display_message(WARNING_MESSAGE,
					"Transform_tool_make_interactive_tool_button.  "
					"Could not fetch widget");
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,
				"Transform_tool_make_interactive_tool_button.  "
				"Could not open heirarchy");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Transform_tool_make_interactive_tool_button.  Invalid argument(s)");
	}
	LEAVE;

	return (widget);
} /* Transform_tool_make_interactive_tool_button */

static int destroy_Interactive_tool_transform_tool_data(
	void **interactive_tool_data_address)
/*******************************************************************************
LAST MODIFIED : 9 October 2000

DESCRIPTION :
Destroys the tool_data associated with a transform tool.
==============================================================================*/
{
	int return_code;

	ENTER(destroy_Interactive_tool_transform_tool_data);
	if (interactive_tool_data_address)
	{
		DEALLOCATE(*interactive_tool_data_address);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"destroy_Interactive_tool_transform_tool_data.  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} /* destroy_Interactive_tool_transform_tool_data */

/*
Global functions
----------------
*/

int Interactive_tool_is_Transform_tool(struct Interactive_tool *interactive_tool)
/*******************************************************************************
LAST MODIFIED : 6 October 2000

DESCRIPTION :
Identifies whether an Interactive_tool is a Transform_tool.
==============================================================================*/
{
	int return_code;

	ENTER(Interative_tool_is_Transform_tool);
	if (interactive_tool)
	{
		return_code = (Interactive_tool_transform_type_string == 
			Interactive_tool_get_tool_type_name(interactive_tool));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Interative_tool_is_Transform_tool.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Interative_tool_is_Transform_tool */

int Interactive_tool_transform_get_free_spin(
	struct Interactive_tool *interactive_tool)
/*******************************************************************************
LAST MODIFIED : 9 October 2000

DESCRIPTION :
If the interactive tool is of type Transform this function specifies whether 
the window should spin freely when tumbling.
==============================================================================*/
{
	int return_code;
	struct Transform_tool *transform_tool;

	ENTER(Interactive_tool_transform_get_free_spin);
	if (interactive_tool && Interactive_tool_is_Transform_tool(interactive_tool)
		 && (transform_tool = (struct Transform_tool *)
		 Interactive_tool_get_tool_data(interactive_tool)))
	{
		return_code = transform_tool->free_spin_flag;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Interactive_tool_transform_get_free_spin.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Interactive_tool_transform_get_free_spin */

int Interactive_tool_transform_set_free_spin(struct Interactive_tool *interactive_tool,
	int free_spin)
/*******************************************************************************
LAST MODIFIED : 9 October 2000

DESCRIPTION :
If the interactive tool is of type Transform this function controls whether 
the window will spin freely when tumbling.
==============================================================================*/
{
	int return_code;
	struct Transform_tool *transform_tool;

	ENTER(Interactive_tool_transform_set_free_spin);
	if (interactive_tool && Interactive_tool_is_Transform_tool(interactive_tool)
		&& (transform_tool = (struct Transform_tool *)
		Interactive_tool_get_tool_data(interactive_tool)))
	{
		transform_tool->free_spin_flag = free_spin;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Interactive_tool_transform_set_free_spin.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Interactive_tool_transform_set_free_spin */

struct Interactive_tool *create_Interactive_tool_transform(
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 9 October 2000

DESCRIPTION :
Creates a transform type Interactive_tool which control the transformation of
scene_viewers.
==============================================================================*/
{
#define XmNtransformFreeSpin "transformFreeSpin"
#define XmCtransformFreeSpin "TransformFreeSpin"
	struct Transform_tool_defaults transform_tool_defaults;
	static XtResource resources[]=
	{
		{
			XmNtransformFreeSpin,
			XmCtransformFreeSpin,
			XmRBoolean,
			sizeof(Boolean),
			XtOffsetOf(struct Transform_tool_defaults,free_spin),
			XmRString,
			"false"
		}
	};
	struct Interactive_tool *interactive_tool;
	struct Transform_tool *transform_tool;

	ENTER(create_Interactive_tool_transform);
	if (user_interface)
	{
		if (ALLOCATE(transform_tool,struct Transform_tool,1))
		{
			transform_tool_defaults.free_spin = False;
			XtVaGetApplicationResources(User_interface_get_application_shell(user_interface),
				&transform_tool_defaults,resources,XtNumber(resources),NULL);
			if (transform_tool_defaults.free_spin)
			{
				transform_tool->free_spin_flag = 1;
			}
			else
			{
				transform_tool->free_spin_flag = 0;
			}
			interactive_tool=CREATE(Interactive_tool)(
				"transform_tool","Transform tool",
				Interactive_tool_transform_type_string,
				(Interactive_event_handler *)NULL,
				Transform_tool_make_interactive_tool_button,
				(Interactive_tool_bring_up_dialog_function *)NULL,
				destroy_Interactive_tool_transform_tool_data,
				(void *)transform_tool);
			transform_tool->interactive_tool = interactive_tool;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_Interactive_tool_transform.  Not enough memory");
			interactive_tool=(struct Interactive_tool *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_Interactive_tool_transform.  Invalid argument(s)");
		interactive_tool=(struct Interactive_tool *)NULL;
	}
	LEAVE;

	return (interactive_tool);
} /* create_Interactive_tool_transform */


