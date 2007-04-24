/*******************************************************************************
FILE : transform_tool.c

LAST MODIFIED : 13 June 2000

DESCRIPTION :
Icon/tool representing the transform function on a graphics window.
Eventually use to store parameters for the transform function.
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

extern "C"{
#include "general/debug.h"
#if defined (MOTIF)
#include "motif/image_utilities.h"
#endif /* defined (MOTIF) */
#include "interaction/interaction_volume.h"
#include "interaction/interactive_event.h"
#include "interaction/interactive_tool.h"
#include "interaction/interactive_tool_private.h"
#include "graphics/transform_tool.h"
#if defined (MOTIF)
static char transform_tool_uidh[] =
#include "graphics/transform_tool.uidh"
	;
#endif /* defined (MOTIF) */
#include "user_interface/message.h"
}

#if defined (WX_USER_INTERFACE)
#include "wx/wx.h"
#include <wx/tglbtn.h>
#include "wx/xrc/xmlres.h"
#include "graphics/graphics_window.h"
#include "graphics/transform_tool.xrch"
#include "graphics/graphics_window_private.hpp"
#endif /* defined (WX_USER_INTERFACE)*/

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

#if defined (WX_USER_INTERFACE)
class wxTransformTool;

#endif /* defined (WX_USER_INTERFACE) */

static char Interactive_tool_transform_type_string[] = "Transform_tool";

#if defined (MOTIF)
struct Transform_tool_defaults
{
	Boolean free_spin;
};
#endif /* defined (MOTIF) */

struct Transform_tool
/*******************************************************************************
LAST MODIFIED : 12 June 2000

DESCRIPTION :
==============================================================================*/
{
  struct Interactive_tool *interactive_tool;

	struct User_interface *user_interface;
	int free_spin_flag;
#if defined (MOTIF)
	Display *display;
#endif /* defined (MOTIF) */

#if defined (WX_USER_INTERFACE)
  wxTransformTool *wx_transform_tool;
#endif /* defined (WX_USER_INTERFACE) */
}; /* struct Transform_tool */
/*
Module functions
----------------
*/
int Transform_tool_transform_set_free_spin(struct Transform_tool *transform_tool,
	int free_spin)
/*******************************************************************************
LAST MODIFIED : 30 January 2007

DESCRIPTION :
If the interactive tool is of type Transform this function controls whether 
the window will spin freely when tumbling.
==============================================================================*/
{
	int return_code;

	ENTER(Transform_tool_transform_set_free_spin);
	if (transform_tool)
	{
		return_code=1;
		if (free_spin)
		{
			transform_tool->free_spin_flag = free_spin;
		}
		else
		{
			transform_tool->free_spin_flag = 0;
    }
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

#if defined (WX_USER_INTERFACE)
class wxTransformTool : public wxPanel
{																								
	Transform_tool *transform_tool;
 	wxCheckBox *button_free_spin;

public:

  wxTransformTool(Transform_tool *transform_tool, wxPanel *parent): 
    transform_tool(transform_tool)
  {	 
		{
			wxXmlInit_transform_tool();
		}
 		wxXmlResource::Get()->LoadPanel(this,parent,_T("CmguiTransformTool"));	
		button_free_spin = XRCCTRL(*this, "ButtonFreeSpin", wxCheckBox);
		Transform_tool_transform_set_free_spin(transform_tool,button_free_spin->IsChecked());
  };

  wxTransformTool()
  {
  };

 	void OnButtonFreeSpin(wxCommandEvent& event)
 	{    
		button_free_spin = XRCCTRL(*this, "ButtonFreeSpin", wxCheckBox);
    Transform_tool_transform_set_free_spin(transform_tool,button_free_spin->IsChecked());
 	}

	void TransformToolInterafaceRenew(Transform_tool *destination_transform_tool)
	{
		button_free_spin = XRCCTRL(*this, "ButtonFreeSpin", wxCheckBox);
		button_free_spin->SetValue(destination_transform_tool->free_spin_flag);
	}


  DECLARE_DYNAMIC_CLASS(wxTransformTool);
  DECLARE_EVENT_TABLE();
};

IMPLEMENT_DYNAMIC_CLASS(wxTransformTool, wxPanel)

BEGIN_EVENT_TABLE(wxTransformTool, wxPanel)
 	EVT_CHECKBOX(XRCID("ButtonFreeSpin"),wxTransformTool::OnButtonFreeSpin)
END_EVENT_TABLE()

#endif /* defined (WX_USER_INTERFACE) */


	static int Transform_tool_pop_up_dialog(struct Transform_tool *transform_tool, struct Graphics_window *graphics_window)
/*******************************************************************************
LAST MODIFIED : 26 January 2007

DESCRIPTION :
Pops up a dialog for editing settings of the Transform_tool.
==============================================================================*/
{
	int return_code;

	ENTER(Transform_tool_pop_up_dialog);
	if (transform_tool)
	{
#if defined (WX_USER_INTERFACE) /* (WX_USER_INTERFACE) */

		if (!transform_tool->wx_transform_tool)
			{
				transform_tool->wx_transform_tool = new wxTransformTool(transform_tool,
					Graphics_window_get_interactive_tool_panel(graphics_window));
			}
		transform_tool->wx_transform_tool->Show();
		
#else /* (WX_USER_INTERFACE) */
		display_message(ERROR_MESSAGE, "Transform_tool_pop_up_dialog.  "
			"No dialog implemented for this User Interface");
#endif /* defined (WX_USER_INTERFACE) */
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_point_tool_pop_up_dialog.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;
	return (return_code);
} /* Transform_tool_pop_up_dialog */




static int Transform_tool_bring_up_interactive_tool_dialog(void *transform_tool_void,struct Graphics_window *graphics_window)
/*******************************************************************************
LAST MODIFIED : 26 January 2007

DESCRIPTION :
Brings up a dialog for editing settings of the Transform_tool - in a standard
format for passing to an Interactive_toolbar.
==============================================================================*/
{
	int return_code;
	

	ENTER(Transform_tool_bring_up_interactive_tool_dialog);
	return_code = Transform_tool_pop_up_dialog((struct Transform_tool *)transform_tool_void, graphics_window);
	LEAVE;

 	return (return_code);
} /* Transform_tool_bring_up_interactive_tool_dialog */



static struct Cmgui_image *Transform_tool_get_icon(struct Colour *foreground, 
	struct Colour *background, void *transform_tool_void)
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Fetches the appropriate icon for the interactive tool.
==============================================================================*/
{
#if defined (MOTIF)
	Display *display;
	Pixel background_pixel, foreground_pixel;
	Pixmap pixmap;
#endif /* defined (MOTIF) */
	struct Cmgui_image *image;
	struct Transform_tool *transform_tool;

	ENTER(Transform_tool_get_icon);
	if ((transform_tool=(struct Transform_tool *)transform_tool_void))
	{
#if defined (MOTIF)
		if (MrmOpenHierarchy_binary_string(transform_tool_uidh,sizeof(transform_tool_uidh),
			&transform_tool_hierarchy,&transform_tool_hierarchy_open))
		{
			display = transform_tool->display;
			convert_Colour_to_Pixel(display, foreground, &foreground_pixel);
			convert_Colour_to_Pixel(display, background, &background_pixel);
			if (MrmSUCCESS == MrmFetchIconLiteral(transform_tool_hierarchy,
				"transform_tool_icon",DefaultScreenOfDisplay(display),display,
				foreground_pixel, background_pixel, &pixmap))
			{ 
				image = create_Cmgui_image_from_Pixmap(display, pixmap);
			}
			else
			{
				display_message(WARNING_MESSAGE, "Transform_tool_get_icon.  "
					"Could not fetch widget");
				image = (struct Cmgui_image *)NULL;
			}			
		}
		else
		{
			display_message(WARNING_MESSAGE, "Transform_tool_get_icon.  "
				"Could not open heirarchy");
			image = (struct Cmgui_image *)NULL;
		}
#else /* defined (MOTIF) */
		USE_PARAMETER(foreground);
		USE_PARAMETER(background);
		display_message(WARNING_MESSAGE, "Transform_tool_get_icon.  "
			"Not implemented for this version.");
		image = (struct Cmgui_image *)NULL;
#endif /* defined (MOTIF) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Transform_tool_get_icon.  Invalid argument(s)");
		image = (struct Cmgui_image *)NULL;
	}
	LEAVE;

	return (image);
} /* Transform_tool_get_icon */

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

static int Transform_tool_copy_function(
	void *destination_tool_void, void *source_tool_void,
	struct MANAGER(Interactive_tool) *destination_tool_manager) 
/*******************************************************************************
LAST MODIFIED : 29 March 2007

DESCRIPTION :
Copies the state of one transform tool to another.
==============================================================================*/
{
	int return_code;
	struct Transform_tool *destination_transform_tool, *source_transform_tool;

	ENTER(Transform_tool_copy_function);
	if ((destination_tool_void || destination_tool_manager) &&
		(source_transform_tool=(struct Transform_tool *)source_tool_void))
	{
		if (destination_tool_void)
		{
			destination_transform_tool = (struct Transform_tool *)destination_tool_void;
		}
		else
		{
			Interactive_tool *new_transform_tool = 
				create_Interactive_tool_transform(source_transform_tool->user_interface);
			ADD_OBJECT_TO_MANAGER(Interactive_tool)(
				new_transform_tool, destination_tool_manager);
			destination_transform_tool = (struct Transform_tool *)
				Interactive_tool_get_tool_data(new_transform_tool);
		}
		if (destination_transform_tool)
		{
			destination_transform_tool->free_spin_flag = source_transform_tool->free_spin_flag;
#if defined (WX_USER_INTERFACE)
			if (destination_transform_tool->wx_transform_tool != (wxTransformTool *) NULL)
			{	
				destination_transform_tool->wx_transform_tool->TransformToolInterafaceRenew(destination_transform_tool);
			}
#endif /*defined (WX_USER_INTERFACE)*/
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"transform_tool_copy_function.  Could not create copy.");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Transform_tool_copy_function.  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} /* Transform_tool_copy_function */

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
	struct  Transform_tool *transform_tool;;

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
#if defined (MOTIF)
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
			const_cast<char*>("false")
		}
	};
#endif /* defined (MOTIF) */
	struct Interactive_tool *interactive_tool;
	struct Transform_tool *transform_tool;

	ENTER(create_Interactive_tool_transform);
	if (user_interface)
	{
		if (ALLOCATE(transform_tool,struct Transform_tool,1))
		{			
#if defined (MOTIF)
			transform_tool_defaults.free_spin = False;
			XtVaGetApplicationResources(User_interface_get_application_shell(user_interface),
				&transform_tool_defaults,resources,XtNumber(resources),NULL);
			transform_tool->display = User_interface_get_display(user_interface);
			if (transform_tool_defaults.free_spin)
			{
				transform_tool->free_spin_flag = 1;
			}
			else
			{
				transform_tool->free_spin_flag = 0;
			}
#elif defined (WX_USER_INTERFACE) /* (WX_USER_INTERFACE) */

			transform_tool->user_interface = user_interface;
			transform_tool->wx_transform_tool = (wxTransformTool *)NULL; 

#else /* switch (USER_INTERFACE) */
			transform_tool->free_spin_flag = 0;
#endif /* switch (USER_INTERFACE) */
			interactive_tool=CREATE(Interactive_tool)(
				"transform_tool","Transform tool",
				Interactive_tool_transform_type_string,
				(Interactive_event_handler *)NULL,
				Transform_tool_get_icon,
				//(Interactive_tool_bring_up_dialog_function *)NULL,
				Transform_tool_bring_up_interactive_tool_dialog,
				destroy_Interactive_tool_transform_tool_data,
				Transform_tool_copy_function,
				(void *)transform_tool);
			transform_tool->interactive_tool = interactive_tool;
		}       //Transform_tool_bring_up_interactive_tool_dialog,
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

