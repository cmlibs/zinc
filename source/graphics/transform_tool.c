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

struct Transform_tool
/*******************************************************************************
LAST MODIFIED : 12 June 2000

DESCRIPTION :
==============================================================================*/
{
	struct MANAGER(Interactive_tool) *interactive_tool_manager;
	struct Interactive_tool *interactive_tool;
}; /* struct Transform_tool */

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

/*
Global functions
----------------
*/

struct Transform_tool *CREATE(Transform_tool)(
	struct MANAGER(Interactive_tool) *interactive_tool_manager)
/*******************************************************************************
LAST MODIFIED : 13 June 2000

DESCRIPTION :
Creates a Transform_tool which works like a placeholder for a graphics windows
own transform mode - so it appears like any other tool in a toolbar.
==============================================================================*/
{
	struct Transform_tool *transform_tool;

	ENTER(CREATE(Transform_tool));
	if (interactive_tool_manager)
	{
		if (ALLOCATE(transform_tool,struct Transform_tool,1))
		{
			transform_tool->interactive_tool_manager=interactive_tool_manager;
			/* interactive_tool */
			transform_tool->interactive_tool=CREATE(Interactive_tool)(
				"transform_tool","Transform tool",(Interactive_event_handler *)NULL,
				Transform_tool_make_interactive_tool_button,
				(Interactive_tool_bring_up_dialog_function *)NULL,
				(void *)transform_tool);
			ADD_OBJECT_TO_MANAGER(Interactive_tool)(transform_tool->interactive_tool,
				transform_tool->interactive_tool_manager);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Transform_tool).  Not enough memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Transform_tool).  Invalid argument(s)");
		transform_tool=(struct Transform_tool *)NULL;
	}
	LEAVE;

	return (transform_tool);
} /* CREATE(Transform_tool) */

int DESTROY(Transform_tool)(struct Transform_tool **transform_tool_address)
/*******************************************************************************
LAST MODIFIED : 12 June 2000

DESCRIPTION :
Frees and deaccesses objects in the <transform_tool> and deallocates the
structure itself.
==============================================================================*/
{
	struct Transform_tool *transform_tool;
	int return_code;

	ENTER(DESTROY(Transform_tool));
	if (transform_tool_address&&(transform_tool= *transform_tool_address))
	{
		REMOVE_OBJECT_FROM_MANAGER(Interactive_tool)(
			transform_tool->interactive_tool,
			transform_tool->interactive_tool_manager);
		DEALLOCATE(*transform_tool_address);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Transform_tool).  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} /* DESTROY(Transform_tool) */

