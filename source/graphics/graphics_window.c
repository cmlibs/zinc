/*******************************************************************************
FILE : graphics_window.c

LAST MODIFIED : 22 January 2002

DESCRIPTION:
Code for opening, closing and working a CMISS 3D display window.

Have get/set routines for parameters specific to window and/or which have
widgets that are automatically updated if you set them. Use these functions
if supplied, otherwise use Graphics_window_get_Scene_viewer() for the pane_no of
interest and set scene_viewer values directly.
==============================================================================*/
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <Xm/Protocols.h>
#include <Xm/PushBG.h>
#include <Xm/DialogS.h>
#include <Xm/MwmUtil.h>
#include <Xm/ToggleB.h>
#include <Mrm/MrmPublic.h>
#include <Mrm/MrmDecls.h>

#include "choose/choose_enumerator.h"
#include "colour/edit_var.h"
#include "command/parser.h"
#include "general/debug.h"
#include "general/geometry.h"
#include "general/indexed_list_private.h"
#include "general/manager_private.h"
#include "general/matrix_vector.h"
#include "general/mystring.h"
#include "general/object.h"
#include "general/photogrammetry.h"
#include "graphics/colour.h"
#include "graphics/graphics_window.h"
#include "graphics/graphics_window.uidh"
#include "graphics/light.h"
#include "graphics/light_model.h"
#include "graphics/scene.h"
#include "graphics/scene_viewer.h"
#include "graphics/texture.h"
#include "graphics/transform_tool.h"
#include "interaction/interactive_toolbar_widget.h"
#include "user_interface/gui_dialog_macros.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"
/* for writing bitmap to file: */
#include "general/image_utilities.h"
#include "three_d_drawing/dm_interface.h"
#include "time/time_keeper.h"
#include "user_interface/confirmation.h"
#include "user_interface/printer.h"

/*
Module constants
----------------
*/
/*???RC.  Would need some work to change to more than 4 panes, since they are
explicitly laid out in the UIL file. Perhaps we should bring this inside code */
#define GRAPHICS_WINDOW_MAX_NUMBER_OF_PANES 4

#define TIME_STEP 0.1
static char *axis_name[7]={"??","x","y","z","-x","-y","-z"};

/*
Module types
------------
*/
struct Graphics_window
/*******************************************************************************
LAST MODIFIED : 8 September 2000

DESCRIPTION :
Contains information for a graphics window.
==============================================================================*/
{
	/* identifier for uniquely specifying window: */
	char *name;
	/* need to keep graphics window manager so window can be destroyed by self */
	struct MANAGER(Graphics_window) *graphics_window_manager;
	/* widgets on the Graphics_window */
	Widget viewing_form,viewing_area1,viewing_area2,viewing_area3,viewing_area4,
		view_all_button,print_button,time_edit_form,time_edit_widget,
		perspective_button,layout_mode_form,layout_mode_widget,
		orthographic_form,ortho_up_option,
		ortho_up_menu,ortho_front_button,
		interactive_toolbar_form,interactive_toolbar_widget;
	Widget window_shell,main_window;
	/* scene_viewers and their parameters: */
	enum Graphics_window_layout_mode layout_mode;
	struct Scene_viewer *scene_viewer[GRAPHICS_WINDOW_MAX_NUMBER_OF_PANES];
	/* The viewing_width and viewing_height are the size of the viewing area when
		 the graphics window has only one pane. When multiple panes are used, they
		 are separated by 2 pixel borders within the viewing area.
		 These defaults are read in by Graphics_window_read_defaults. */
	int default_viewing_height,default_viewing_width;
	/* number_of_panes is a function of layout_mode, but stored for convenience */
	int number_of_panes;
	/* angle of view in degrees set by set_std_view_angle function */
	double std_view_angle;
	/* distance between eyes for 3-D viewing */
	double eye_spacing;
	/* for speeding/slowing translate, tumble and zoom */
	double default_translate_rate,default_tumble_rate,default_zoom_rate;
	int ortho_up_axis,ortho_front_axis;
	/* current pane for commands affecting view. First pane is 0 internally, but
		 user should be presented with this value+1, so the first pane is 1 */
	int current_pane;
	int antialias_mode;
	int perturb_lines;
	enum Scene_viewer_input_mode input_mode;
	/*???DB.  Do these belong here ? */
	/* time parameters for animation */
	/* current time for frame */
	double time_value;
	/* time at node */
	double animation_time;
	/* maximum animation time */
	double time_max;
	/* time step between frames */
	double time_step;
	/* not sure if graphics window should keep pointer to scene - could just get
		 it from scene_viewers; could even be different in each scene_viewer */
	struct Scene *scene;
	/* graphics window does not need to keep managers now that changes handled
		 by scene_viewer */
	struct MANAGER(Light) *light_manager;
	struct MANAGER(Light_model) *light_model_manager;
	struct MANAGER(Scene) *scene_manager;
	struct MANAGER(Texture) *texture_manager;
	/* interaction */
	struct MANAGER(Interactive_tool) *interactive_tool_manager;
	/* Note: interactive_tool is NOT accessed by graphics_window; the chooser
		 will update it if the current one is destroyed */
	struct Interactive_tool *interactive_tool;
	/* the time_slider is attached to the default_time_keeper of the scene,
		the reference is kept so that the callbacks can be undone */
	struct Time_keeper *time_keeper;
	struct User_interface *user_interface;
	/* the number of objects accessing this window. The window cannot be removed
		from manager unless it is 1 (ie. only the manager is accessing it) */
	int access_count;
}; /* struct Graphics_window */

FULL_DECLARE_INDEXED_LIST_TYPE(Graphics_window);

FULL_DECLARE_MANAGER_TYPE(Graphics_window);

struct Graphics_window_ortho_axes
{
	int up,front;
}; /* struct Graphics_window_ortho_axes */

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int graphics_window_hierarchy_open=0;
static MrmHierarchy graphics_window_hierarchy;
#endif /* defined (MOTIF) */

/*
Module functions
----------------
*/
DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Graphics_window, \
        name,char *,strcmp)
DECLARE_LOCAL_MANAGER_FUNCTIONS(Graphics_window)

DECLARE_DIALOG_IDENTIFY_FUNCTION(graphics_window, \
        Graphics_window,viewing_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphics_window, \
        Graphics_window,viewing_area1)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphics_window, \
        Graphics_window,viewing_area2)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphics_window, \
        Graphics_window,viewing_area3)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphics_window, \
        Graphics_window,viewing_area4)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphics_window, \
        Graphics_window,view_all_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphics_window, \
        Graphics_window,print_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphics_window, \
        Graphics_window,time_edit_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphics_window, \
        Graphics_window,perspective_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphics_window, \
        Graphics_window,layout_mode_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphics_window, \
        Graphics_window,orthographic_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphics_window, \
        Graphics_window,ortho_up_option)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphics_window, \
        Graphics_window,ortho_up_menu)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphics_window, \
        Graphics_window,ortho_front_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(graphics_window, \
        Graphics_window,interactive_toolbar_form)

static int axis_name_to_axis_number(char *axis_name)
/*******************************************************************************
LAST MODIFIED : 16 December 1997

DESCRIPTION :
Axes are numbered from 1 to 6 in the order X,Y,Z,-X,-Y,-Z. This function
returns the axis_number for the given axis_name. Eg. "-Y" -> 5.
A return value of 0 indicates an invalid name or other error.
==============================================================================*/
{
	int axis_number;

	ENTER(axis_name_to_axis_number);
	if (axis_name&&(0<strlen(axis_name)))
	{
		if (strchr(axis_name,'x')||strchr(axis_name,'X'))
		{
			axis_number=1;
		}
		else
		{
			if (strchr(axis_name,'y')||strchr(axis_name,'Y'))
			{
				axis_number=2;
			}
			else
			{
				if (strchr(axis_name,'z')||strchr(axis_name,'Z'))
				{
					axis_number=3;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Invalid orthographic axis: %s",axis_name);
					axis_number=0;
				}
			}
		}
		if (axis_number&&('-'==axis_name[0]))
		{
			axis_number += 3;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"axis_name_to_axis_number.  Missing axis_name");
		axis_number=0;
	}
	LEAVE;

	return (axis_number);
} /* axis_name_to_axis_number */

static int axis_number_to_axis_vector(int axis_number,double axis[3])
/*******************************************************************************
LAST MODIFIED : 16 December 1997

DESCRIPTION :
Axes are numbered from 1 to 6 in the order X,Y,Z,-X,-Y,-Z. This function
converts the axis number to a three component [float] vector. eg 1 -> (1,0,0).
==============================================================================*/
{
	int return_code;

	ENTER(axis_number_to_axis_vector);
	return_code=1;
	switch (axis_number)
	{
		case 1: /* +X */
		{
			axis[0]= 1.0;
			axis[1]= 0.0;
			axis[2]= 0.0;
		} break;
		case 2: /* +Y */
		{
			axis[0]= 0.0;
			axis[1]= 1.0;
			axis[2]= 0.0;
		} break;
		case 3: /* +Z */
		{
			axis[0]= 0.0;
			axis[1]= 0.0;
			axis[2]= 1.0;
		} break;
		case 4: /* -X */
		{
			axis[0]=-1.0;
			axis[1]= 0.0;
			axis[2]= 0.0;
		} break;
		case 5: /* -Y */
		{
			axis[0]= 0.0;
			axis[1]=-1.0;
			axis[2]= 0.0;
		} break;
		case 6: /* -Z */
		{
			axis[0]= 0.0;
			axis[1]= 0.0;
			axis[2]=-1.0;
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"axis_number_to_axis_vector.  Invalid axis_number");
			return_code=0;
		} break;
	}
	LEAVE;

	return (return_code);
} /* axis_number_to_axis_vector */

static int Graphics_window_read_defaults(
	struct Graphics_window *graphics_window)
/*******************************************************************************
LAST MODIFIED : 15 October 1998

DESCRIPTION :
Reads in the default window size and border thickness and definitions of the
orthographic up and front directions from the Xdefaults file.
==============================================================================*/
{
#define XmNgraphicsWindowOrthoUpAxis "graphicsWindowOrthoUpAxis"
#define XmCGraphicsWindowOrthoUpAxis "GraphicsWindowOrthoUpAxis"
#define XmNgraphicsWindowOrthoFrontAxis "graphicsWindowOrthoFrontAxis"
#define XmCGraphicsWindowOrthoFrontAxis "GraphicsWindowOrthoFrontAxis"
#define XmNgraphicsWindowViewingHeight "graphicsWindowViewingHeight"
#define XmCGraphicsWindowViewingHeight "GraphicsWindowViewingHeight"
#define XmNgraphicsWindowViewingWidth "graphicsWindowViewingWidth"
#define XmCGraphicsWindowViewingWidth "GraphicsWindowViewingWidth"
#define XmNgraphicsWindowTranslateRate "graphicsWindowTranslateRate"
#define XmCGraphicsWindowTranslateRate "GraphicsWindowTranslateRate"
#define XmNgraphicsWindowTumbleRate "graphicsWindowTumbleRate"
#define XmCGraphicsWindowTumbleRate "GraphicsWindowTumbleRate"
#define XmNgraphicsWindowZoomRate "graphicsWindowZoomRate"
#define XmCGraphicsWindowZoomRate "GraphicsWindowZoomRate"
	int return_code,up_axis,front_axis;
	struct Graphics_window_defaults
	{
		char *up_axis_name,*front_axis_name;
		float translate_rate,tumble_rate,zoom_rate;
		int viewing_height,viewing_width;
	} graphics_window_defaults;
	static XtResource resources[]=
	{
		{
			XmNgraphicsWindowOrthoUpAxis,
			XmCGraphicsWindowOrthoUpAxis,
			XmRString,
			sizeof(char *),
			XtOffsetOf(struct Graphics_window_defaults,up_axis_name),
			XmRString,
			"Z"
		},
		{
			XmNgraphicsWindowOrthoFrontAxis,
			XmCGraphicsWindowOrthoFrontAxis,
			XmRString,
			sizeof(char *),
			XtOffsetOf(struct Graphics_window_defaults,front_axis_name),
			XmRString,
			"-Y"
		},
		{
			XmNgraphicsWindowViewingHeight,
			XmCGraphicsWindowViewingHeight,
			XmRInt,
			sizeof(int),
			XtOffsetOf(struct Graphics_window_defaults,viewing_height),
			XmRString,
			"512"
		},
		{
			XmNgraphicsWindowViewingWidth,
			XmCGraphicsWindowViewingWidth,
			XmRInt,
			sizeof(int),
			XtOffsetOf(struct Graphics_window_defaults,viewing_width),
			XmRString,
			"512"
		},
		{
			XmNgraphicsWindowTranslateRate,
			XmCGraphicsWindowTranslateRate,
			XmRFloat,
			sizeof(float),
			XtOffsetOf(struct Graphics_window_defaults,translate_rate),
			XmRString,
			"1.0"
		},
		{
			XmNgraphicsWindowTumbleRate,
			XmCGraphicsWindowTumbleRate,
			XmRFloat,
			sizeof(float),
			XtOffsetOf(struct Graphics_window_defaults,tumble_rate),
			XmRString,
			"1.5"
		},
		{
			XmNgraphicsWindowZoomRate,
			XmCGraphicsWindowZoomRate,
			XmRFloat,
			sizeof(float),
			XtOffsetOf(struct Graphics_window_defaults,zoom_rate),
			XmRString,
			"1.0"
		}
	};

	ENTER(Graphics_window_read_defaults);
	if (graphics_window)
	{
		up_axis=0;
		front_axis=0;
		graphics_window_defaults.up_axis_name=(char *)NULL;
		graphics_window_defaults.front_axis_name=(char *)NULL;
		XtVaGetApplicationResources(
			graphics_window->user_interface->application_shell,
			&graphics_window_defaults,resources,XtNumber(resources),NULL);
		if (graphics_window_defaults.up_axis_name)
		{
			up_axis=axis_name_to_axis_number(graphics_window_defaults.up_axis_name);
		}
		if (graphics_window_defaults.front_axis_name)
		{
			front_axis=axis_name_to_axis_number(
				graphics_window_defaults.front_axis_name);
		}
		graphics_window->ortho_up_axis=up_axis;
		graphics_window->ortho_front_axis=front_axis;
		graphics_window->default_viewing_height=
			graphics_window_defaults.viewing_height;
		graphics_window->default_viewing_width=
			graphics_window_defaults.viewing_width;
		graphics_window->default_translate_rate=
			graphics_window_defaults.translate_rate;
		graphics_window->default_tumble_rate=
			graphics_window_defaults.tumble_rate;
		graphics_window->default_zoom_rate=
			graphics_window_defaults.zoom_rate;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_read_defaults.  Missing graphics_window");
		return_code=0;
	}
	LEAVE;

	return return_code;
} /* Graphics_window_read_defaults */

/*
Widget Callback Module functions
--------------------------------
*/

static void Graphics_window_close_CB(Widget caller,
	void *graphics_window_void,void *cbs)
/*******************************************************************************
LAST MODIFIED : 10 December 1997

DESCRIPTION :
Called when "close" is selected from the window menu, or it is double clicked.
How this is made to occur is as follows. The graphics_window dialog has its
XmNdeleteResponse == XmDO_NOTHING, and a window manager protocol callback for
WM_DELETE_WINDOW has been set up with XmAddWMProtocolCallback to call this
function in response to the close command. See CREATE(Graphics_window) for more
details.
Since the Graphics_window is a managed object, we simply remove it from its
manager. If this is successful it will be DESTROYed automatically.
If it is not managed, can't destroy it here.
==============================================================================*/
{
	struct Graphics_window *graphics_window;

	ENTER(Graphics_window_close_CB);
	USE_PARAMETER(caller);
	USE_PARAMETER(cbs);
	if (graphics_window=(struct Graphics_window *)graphics_window_void)
	{
		if (graphics_window->graphics_window_manager)
		{
			REMOVE_OBJECT_FROM_MANAGER(Graphics_window)(graphics_window,
				graphics_window->graphics_window_manager);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_close_CB.  Missing Graphics_window");
	}
	LEAVE;
} /* Graphics_window_close_CB */

static void Graphics_window_destroy_CB(Widget caller,
	XtPointer user_data,XtPointer caller_data)
/*******************************************************************************
LAST MODIFIED : 24 November 1997

DESCRIPTION :
Callback for when the window is destroyed. Does not do anything since the
Graphics_window object is now destroyed by its DESTROY function.
Keep in case a use is found for it.
==============================================================================*/
{
	ENTER(Graphics_window_destroy_CB);
	USE_PARAMETER(caller);
	USE_PARAMETER(user_data);
	USE_PARAMETER(caller_data);
	LEAVE;
} /* Graphics_window_destroy_CB */

static int Graphics_window_set_interactive_tool(
	struct Graphics_window *graphics_window,
	struct Interactive_tool *interactive_tool)
/*******************************************************************************
LAST MODIFIED : 13 June 2000

DESCRIPTION :
Sets the <interactive_tool> in use in the <graphics_window>. Updates the
toolbar to match the selection.
==============================================================================*/
{
	int pane_no,return_code;

	ENTER(Graphics_window_set_interactive_tool);
	if (graphics_window)
	{
		if (interactive_toolbar_widget_set_current_interactive_tool(
			graphics_window->interactive_toolbar_widget,interactive_tool))
		{
			graphics_window->interactive_tool=interactive_tool;
			if (Interactive_tool_is_Transform_tool(interactive_tool))
			{
				Graphics_window_set_input_mode(graphics_window,SCENE_VIEWER_TRANSFORM);
			}
			else
			{
				Graphics_window_set_input_mode(graphics_window,SCENE_VIEWER_SELECT);
			}
			for (pane_no=0;(pane_no<GRAPHICS_WINDOW_MAX_NUMBER_OF_PANES);pane_no++)
			{
				Scene_viewer_set_interactive_tool(
					graphics_window->scene_viewer[pane_no],interactive_tool);
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Graphics_window_set_interactive_tool.  Could not update toolbar");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_set_interactive_tool.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_window_set_interactive_tool */

static void Graphics_window_update_interactive_tool(Widget widget,
	void *graphics_window_void,void *interactive_tool_void)
/*******************************************************************************
LAST MODIFIED : 13 June 2000

DESCRIPTION :
Called when a new tool is chosen in the interactive_toolbar_widget.
==============================================================================*/
{
	struct Graphics_window *graphics_window;

	ENTER(Graphics_window_update_interactive_tool);
	USE_PARAMETER(widget);
	if (graphics_window=(struct Graphics_window *)graphics_window_void)
	{
		Graphics_window_set_interactive_tool(graphics_window,
			(struct Interactive_tool *)interactive_tool_void);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_update_interactive_tool.  Invalid argument(s)");
	}
	LEAVE;
} /* Graphics_window_update_interactive_tool */

static void Graphics_window_layout_mode_CB(Widget widget,
	void *graphics_window_void,void *layout_mode_string_void)
/*******************************************************************************
LAST MODIFIED : 28 June 2000

DESCRIPTION :
Callback for change of layout_mode from the layout_mode_widget.
==============================================================================*/
{
	struct Graphics_window *graphics_window;

	ENTER(Graphics_window_layout_mode_CB);
	USE_PARAMETER(widget);
	if (graphics_window=(struct Graphics_window *)graphics_window_void)
	{
		Graphics_window_set_layout_mode(graphics_window,
			Graphics_window_layout_mode_from_string((char *)layout_mode_string_void));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_layout_mode_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Graphics_window_layout_mode_CB */

static void Graphics_window_ortho_front_button_CB(Widget widget,
	XtPointer window_void,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 8 October 1998

DESCRIPTION :
Callback for a change of front_axis defining the orthographic views. This
function cycles to the next orthogonal front axis for the current ortho_up_axis.
==============================================================================*/
{
	int ortho_front_axis,ortho_up_axis;
	struct Graphics_window *window;

	ENTER(Graphics_window_ortho_front_button_CB);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (window=(struct Graphics_window *)window_void)
	{
		/* cycle through the possible front axes for the current up axis */
		ortho_up_axis=window->ortho_up_axis;
		ortho_front_axis=window->ortho_front_axis;
		do
		{
			ortho_front_axis=(ortho_front_axis % 6)+1;
		} while ((ortho_up_axis % 3) == (ortho_front_axis % 3));
		Graphics_window_set_orthographic_axes(window,ortho_up_axis,
			ortho_front_axis);
		/* display the panes in the new orientation */
		Graphics_window_set_layout_mode(window,window->layout_mode);
		Graphics_window_update(window);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_ortho_front_button_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Graphics_window_ortho_front_button_CB */

static void Graphics_window_ortho_up_menu_CB(Widget widget,
	XtPointer window_void,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 8 October 1998

DESCRIPTION :
Callback for a change of up_axis defining the orthographic views.
==============================================================================*/
{
	struct Graphics_window *window;
	int up_axis;
	Widget menu_item_widget;

	ENTER(Graphics_window_ortho_up_menu_CB);
	if (widget&&(window=(struct Graphics_window *)window_void)&&call_data)
	{
		/* get the widget from the call data */
		if (menu_item_widget=((XmRowColumnCallbackStruct *)call_data)->widget)
		{
			/* Get the axis this menu item represents and make it current */
			XtVaGetValues(menu_item_widget,XmNuserData,&up_axis,NULL);
			if ((0<up_axis)&&(6 >= up_axis))
			{
				Graphics_window_set_orthographic_axes(window,
					up_axis,window->ortho_front_axis);
				Graphics_window_set_layout_mode(window,window->layout_mode);
				Graphics_window_update(window);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Graphics_window_ortho_up_menu_CB.  Invalid axis");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Graphics_window_ortho_up_menu_CB.  Missing menu item widget");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_ortho_up_menu_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Graphics_window_ortho_up_menu_CB */

static void Graphics_window_perspective_button_CB(Widget caller,
	XtPointer *window_void,XmAnyCallbackStruct *caller_data)
/*******************************************************************************
LAST MODIFIED : 15 October 1998

DESCRIPTION :
Callback for when the perspective toggle button is pressed.
==============================================================================*/
{
	enum Scene_viewer_projection_mode projection_mode;
	struct Graphics_window *window;

	ENTER(Graphics_window_perspective_button_CB);
	USE_PARAMETER(caller);
	USE_PARAMETER(caller_data);
	if (window=(struct Graphics_window *)window_void)
	{
		if (XmToggleButtonGetState(window->perspective_button))
		{
			projection_mode=SCENE_VIEWER_PERSPECTIVE;
		}
		else
		{
			projection_mode=SCENE_VIEWER_PARALLEL;
		}
		if (Graphics_window_set_projection_mode(window,window->current_pane,
			projection_mode))
		{
			/* update the affected scene_viewer and any tied to it */
			Scene_viewer_redraw(window->scene_viewer[window->current_pane]);
			Graphics_window_view_changed(window,window->current_pane);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_perspective_button_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Graphics_window_perspective_button_CB */

static void Graphics_window_time_edit_CB(Widget edit_var,void *window_void,
	void *current_value)
/*******************************************************************************
LAST MODIFIED : 18 October 1998

DESCRIPTION :
Callback for change to the time.
==============================================================================*/
{
	struct Graphics_window *window;

	ENTER(Graphics_window_time_edit_CB);
	/* check arguments */
	if (edit_var&&(window=(struct Graphics_window *)window_void)&&
		(window->scene)&&current_value)
	{
		if(window->time_keeper)
		{
			Time_keeper_request_new_time(window->time_keeper,
				*((EDIT_VAR_PRECISION *)current_value));
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_time_edit_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Graphics_window_time_edit_CB */

static int Graphics_window_time_keeper_callback(struct Time_keeper *time_keeper,
	enum Time_keeper_event event, void *graphics_window_void)
/*******************************************************************************
LAST MODIFIED : 18 February 1999

DESCRIPTION :
Updates the time display of the time_slider
==============================================================================*/
{
	EDIT_VAR_PRECISION time;
	int return_code;
	struct Graphics_window *window;

	ENTER(Graphics_window_time_keeper_callback);
	if (time_keeper && (window = (struct Graphics_window *)graphics_window_void))
	{
		switch(event)
		{
			case TIME_KEEPER_NEW_TIME:
			{
				time = Time_keeper_get_time(window->time_keeper);
				edit_var_set_data(window->time_edit_widget,EDIT_VAR_VALUE,time);				
#if defined (DEBUG)
				printf("Graphics_window_time_keeper_callback.  time \n"
					EDIT_VAR_NUM_FORMAT, time);
#endif /* defined (DEBUG) */
				return_code = 1;
			} break;
			case TIME_KEEPER_NEW_MINIMUM:
			{
				time = Time_keeper_get_minimum(window->time_keeper);
				edit_var_set_data(window->time_edit_widget,EDIT_VAR_LOW_LIMIT,time);				
#if defined (DEBUG)
				printf("Graphics_window_time_keeper_callback.  time \n"
					EDIT_VAR_NUM_FORMAT, time);
#endif /* defined (DEBUG) */
				return_code = 1;
			} break;
			case TIME_KEEPER_NEW_MAXIMUM:
			{
				time = Time_keeper_get_maximum(window->time_keeper);
				edit_var_set_data(window->time_edit_widget,EDIT_VAR_HIGH_LIMIT,time);				
#if defined (DEBUG)
				printf("Graphics_window_time_keeper_callback.  time \n"
					EDIT_VAR_NUM_FORMAT, time);
#endif /* defined (DEBUG) */
				return_code = 1;
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Graphics_window_time_keeper_callback.  Unknown time_keeper event");
				return_code = 0;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_time_keeper_callback.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_window_time_keeper_callback */

static void Graphics_window_view_all_button_CB(Widget caller,
	XtPointer *graphics_window_void,XmAnyCallbackStruct *caller_data)
/*******************************************************************************
LAST MODIFIED : 6 October 1998

DESCRIPTION :
Callback for when the view_all_button is pressed.  Finds the x, y and z
ranges and sets the view parameters so that everything can be seen.
==============================================================================*/
{
	struct Graphics_window *graphics_window;

	ENTER(Graphics_window_view_all_button_CB);
	USE_PARAMETER(caller);
	USE_PARAMETER(caller_data);
	if (graphics_window=(struct Graphics_window *)graphics_window_void)
	{
		if (Graphics_window_view_all(graphics_window))
		{
			Graphics_window_update(graphics_window);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_view_all_button_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Graphics_window_view_all_button_CB */

#if defined (IMAGEMAGICK)
static void Graphics_window_print_button_CB(Widget caller,
	XtPointer *graphics_window_void,XmAnyCallbackStruct *caller_data)
/*******************************************************************************
LAST MODIFIED : 10 May 2001

DESCRIPTION :
Callback for when the print_button is pressed.  Finds the x, y and z
ranges and sets the view parameters so that everything can be seen.
==============================================================================*/
{
	char *file_name;
	int force_onscreen, height, width;
	struct Graphics_window *graphics_window;

	ENTER(Graphics_window_print_button_CB);
	USE_PARAMETER(caller);
	USE_PARAMETER(caller_data);
	if (graphics_window = (struct Graphics_window *)graphics_window_void)
	{
		if (file_name = confirmation_get_write_filename((char *)NULL,
			graphics_window->user_interface))
		{
			force_onscreen = 0;
			width = 0;
			height = 0;
			write_Graphics_window_to_file(file_name, graphics_window, force_onscreen,
				width, height);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_print_button_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Graphics_window_print_button_CB */
#else /* defined (IMAGEMAGICK) */
static void Graphics_window_print_button_CB(Widget caller,
	XtPointer *graphics_window_void,XmAnyCallbackStruct *caller_data)
/*******************************************************************************
LAST MODIFIED : 10 May 2001

DESCRIPTION :
Callback for when the print_button is pressed.  Finds the x, y and z
ranges and sets the view parameters so that everything can be seen.
==============================================================================*/
{
	char *file_name;
	enum Image_file_format image_file_format;
	int force_onscreen, height, width;
	struct Graphics_window *graphics_window;

	ENTER(Graphics_window_print_button_CB);
	USE_PARAMETER(caller);
	USE_PARAMETER(caller_data);
	if (graphics_window = (struct Graphics_window *)graphics_window_void)
	{
		if (file_name = confirmation_get_write_filename((char *)NULL,
			graphics_window->user_interface))
		{
			if (Image_file_format_from_file_name(file_name, &image_file_format))
			{
				force_onscreen = 0;
				width = 0;
				height = 0;
				write_Graphics_window_to_file(file_name, graphics_window,
					image_file_format, force_onscreen, width, height);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Print window:  Unknown image file format for file '%s'", file_name);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_print_button_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Graphics_window_print_button_CB */
#endif /* defined (IMAGEMAGICK) */

/*
Manager Callback Module functions
---------------------------------
*/

static void Graphics_window_Scene_viewer_view_changed(struct Scene_viewer *scene_viewer,
	void *dummy_void, void *window_void)
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
Called whenever the view changes in parallel and perspective mode by mouse
movements in the scene_viewer. This function works out which pane has changed
and calls Graphics_window_view_changed to synchronise views in certain
layout_modes, eg. GRAPHICS_WINDOW_LAYOUT_ORTHOGRAPHIC.
==============================================================================*/
{
	int changed_pane,pane_no;
	struct Graphics_window *window;

	ENTER(Graphics_window_Scene_viewer_view_changed);
	USE_PARAMETER(dummy_void);
	if ((window=(struct Graphics_window *)window_void)&&scene_viewer)
	{
		changed_pane=-1;
		for (pane_no=0;pane_no<window->number_of_panes;pane_no++)
		{
			if (scene_viewer == window->scene_viewer[pane_no])
			{
				changed_pane=pane_no;
			}
		}
		if (-1 != changed_pane)
		{
			Graphics_window_view_changed(window,changed_pane);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Graphics_window_Scene_viewer_view_changed.  Unknown Scene_viewer");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_Scene_viewer_view_changed.  Invalid argument(s)");
	}
	LEAVE;
} /* Graphics_window_Scene_viewer_view_changed */

/*
Command Parsing Module functions
--------------------------------
*/

static int set_Graphics_window_ortho_axes(struct Parse_state *state,
	void *ortho_axes_void,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 8 October 1998

DESCRIPTION :
Modifier function to set the up and front directions for defining the
orthographic axes.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Graphics_window_ortho_axes *ortho_axes;

	ENTER(set_Graphics_window_ortho_axes);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		if (ortho_axes=(struct Graphics_window_ortho_axes *)ortho_axes_void)
		{
			if (current_token=state->current_token)
			{
				if (strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
				{
					if (ortho_axes->up=axis_name_to_axis_number(current_token))
					{
						if (return_code=shift_Parse_state(state,1))
						{
							if (current_token=state->current_token)
							{
								if (ortho_axes->front=axis_name_to_axis_number(current_token))
								{
									return_code=shift_Parse_state(state,1);
								}
								else
								{
									return_code=0;
								}
							}
						}
					}
					else
					{
						return_code=0;
					}
				}
				else
				{
					display_message(INFORMATION_MESSAGE,
						" ORTHO_UP{X/Y/Z/-X/-Y/-Z} ORTHO_FRONT{X/Y/Z/-X/-Y/-Z}");
					return_code=1;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"Missing ORTHO_UP and ORTHO_FRONT");
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_Graphics_window_ortho_axes.  Missing ortho_axes structure");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_Graphics_window_ortho_axes.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Graphics_window_ortho_axes */

static int modify_Graphics_window_background(struct Parse_state *state,
	void *window_void,void *modify_graphics_window_data_void)
/*******************************************************************************
LAST MODIFIED : 28 September 1999

DESCRIPTION :
Parser commands for modifying the background colours and textures of the
current pane of the <window>. Note that the <all_panes> options signifies that
the changes are to be applied to all panes.
==============================================================================*/
{
	char all_panes_flag,no_undistort_flag,undistort_flag;
	double max_pixels_per_polygon,texture_placement[4];
	int first_pane,i,last_pane,pane_no,pixelsx,pixelsy,return_code,undistort_on;
	struct Colour background_colour;
	struct Graphics_window *window;
	struct Modify_graphics_window_data *modify_graphics_window_data;
	struct Scene_viewer *scene_viewer;
	struct Texture *background_texture;
	static struct Modifier_entry
		undistort_option_table[]=
		{
			{"undistort_texture",NULL,NULL,set_char_flag},
			{"no_undistort_texture",NULL,NULL,set_char_flag},
			{NULL,NULL,NULL,NULL}
		},
		option_table[]=
		{
			{"all_panes",NULL,NULL,set_char_flag},
			{"colour",NULL,NULL,set_Colour},
			{"max_pixels_per_polygon",NULL,NULL,set_double},
			{"texture",NULL,NULL,set_Texture},
			{"tex_placement",NULL,NULL,set_double_vector_with_help},
			{NULL,NULL,NULL,NULL},
			{NULL,NULL,NULL,NULL}
		};
	/* do not make the following static as 'set' flag must start at 0 */
	struct Set_vector_with_help_data texture_placement_data=
		{4," TEXTURE_LEFT TEXTURE_TOP TEXTURE_WIDTH TEXTURE_HEIGHT",0};

	ENTER(modify_Graphics_window_background);
	if (state)
	{
		if (modify_graphics_window_data=(struct Modify_graphics_window_data *)
			modify_graphics_window_data_void)
		{
			if (state->current_token)
			{
				/* get defaults from scene_viewer for first pane of window */
				if ((window=(struct Graphics_window *)window_void)&&
					(scene_viewer=window->scene_viewer[window->current_pane]))
				{
					Scene_viewer_get_background_colour(scene_viewer,&background_colour);
					if (background_texture=
						Scene_viewer_get_background_texture(scene_viewer))
					{
						ACCESS(Texture)(background_texture);
					}
					Scene_viewer_get_background_texture_info(scene_viewer,
						&(texture_placement[0]),&(texture_placement[1]),
						&(texture_placement[2]),&(texture_placement[3]),
						&undistort_on,&max_pixels_per_polygon);
				}
				else
				{
					scene_viewer=(struct Scene_viewer *)NULL;
					background_colour.red=0.0;
					background_colour.green=0.0;
					background_colour.blue=0.0;
					background_texture=(struct Texture *)NULL;
					texture_placement[0]=texture_placement[1]=
						texture_placement[2]=texture_placement[3]=0.0;
					undistort_on=0;
					max_pixels_per_polygon=0.0;
				}
				undistort_flag=0;
				no_undistort_flag=0;
				all_panes_flag=0;
				i=0;
				/* all_panes */
				(option_table[i]).to_be_modified= &all_panes_flag;
				i++;
				/* colour */
				(option_table[i]).to_be_modified= &background_colour;
				i++;
				/* max_pixels_per_polygon */
				(option_table[i]).to_be_modified= (void *)&max_pixels_per_polygon;
				i++;
				/* texture */
				(option_table[i]).to_be_modified= &background_texture;
				(option_table[i]).user_data=
					modify_graphics_window_data->texture_manager;
				i++;
				/* tex_placement */
				(option_table[i]).to_be_modified= (void *)texture_placement;
				(option_table[i]).user_data= &texture_placement_data;
				i++;
				/* undistort_texture/no_undistort_texture */
				(undistort_option_table[0]).to_be_modified = &undistort_flag;
				(undistort_option_table[1]).to_be_modified = &no_undistort_flag;
				(option_table[i]).user_data=undistort_option_table;
				i++;
				if (return_code=process_multiple_options(state,option_table))
				{
					if (scene_viewer)
					{
						return_code=1;
						if (all_panes_flag)
						{
							first_pane=0;
							last_pane=GRAPHICS_WINDOW_MAX_NUMBER_OF_PANES-1;
						}
						else
						{
							first_pane=last_pane=window->current_pane;
						}
						if (undistort_flag&&no_undistort_flag)
						{
							display_message(ERROR_MESSAGE,
								"modify_Graphics_window_background. "
								"Only one of undistort_texture|no_undistort_texture");
						}
						else
						{
							if (undistort_flag)
							{
								undistort_on=1;
							}
							else if (no_undistort_flag)
							{
								undistort_on=0;
							}
						}
						for (pane_no=first_pane;pane_no <= last_pane;pane_no++)
						{
							scene_viewer=window->scene_viewer[pane_no];
							Scene_viewer_set_background_colour(scene_viewer,
								&background_colour);
							Scene_viewer_set_background_texture(scene_viewer,
								background_texture);
							if ((texture_placement[2] == 0.0) && (texture_placement[3] == 0.0))
							{
								if (background_texture)
								{
									/* Get the default size from the texture itself */
									Texture_get_original_size(background_texture,
										&pixelsx, &pixelsy);
									texture_placement[2] = pixelsx;
									texture_placement[3] = pixelsy;
									Scene_viewer_set_background_texture_info(scene_viewer,
										texture_placement[0],texture_placement[1],
										texture_placement[2],texture_placement[3],
										undistort_on,max_pixels_per_polygon);
								}
							}
							else
							{
								Scene_viewer_set_background_texture_info(scene_viewer,
									texture_placement[0],texture_placement[1],
									texture_placement[2],texture_placement[3],
									undistort_on,max_pixels_per_polygon);
							}
						}
						Graphics_window_update_now(window);
					}
					else
					{
						display_message(ERROR_MESSAGE,"modify_Graphics_window_background. "
							"Missing or invalid scene_viewer");
						display_parse_state_location(state);
						return_code=0;
					}
				}
				if (background_texture)
				{
					DEACCESS(Texture)(&background_texture);
				}
			}
			else
			{
				display_message(WARNING_MESSAGE,
					"Missing window background modifications");
				display_parse_state_location(state);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"modify_Graphics_window_background.  "
				"Missing modify_graphics_window_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"modify_Graphics_window_background.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* modify_Graphics_window_background */

static int modify_Graphics_window_image(struct Parse_state *state,
	void *window_void,void *modify_graphics_window_data_void)
/*******************************************************************************
LAST MODIFIED : 18 November 1998

DESCRIPTION :
Parser commands for setting the scene and how it is displayed (time, light model
etc.) in all panes of the <window>.
???RC Remove add_light and remove_light once we have an overlay scene? ie.
overlay scene contains headlights - lights stationary w.r.t. the viewer.
???RC Probably not. The model we will probably end up with is conceptually like
a spaceship/submarine, where:
- Lights in the overlay scene light only the "interior" controls, ie. graphics
  objects in the overlay scene.
- Lights in the window are attached to the ship and thus positioned relative to
  it, but they illuminate only the outside scene.
- Lights in the scene illuminate the scene and are positioned relative to it.
==============================================================================*/
{
	char update_flag,view_all_flag;
	double rotate_angle,rotate_axis[3],rotate_data[4];
	EDIT_VAR_PRECISION time_value;
	int i,pane_no,return_code;
	struct Graphics_window *window;
	struct Light *light_to_add,*light_to_remove;
	struct Light_model *light_model;
	struct Modify_graphics_window_data *modify_graphics_window_data;
	struct Scene *scene;
	struct Scene_viewer *scene_viewer;
	struct Time_keeper *new_time_keeper;
	static struct Modifier_entry option_table[]=
	{
		{"add_light",NULL,NULL,set_Light},
		{"light_model",NULL,NULL,set_Light_model},
		{"remove_light",NULL,NULL,set_Light},
		{"rotate",NULL,NULL,set_double_vector_with_help},
		{"scene",NULL,NULL,set_Scene},
		{"update",NULL,NULL,set_char_flag},
		{"view_all",NULL,NULL,set_char_flag},
		{NULL,NULL,NULL,NULL}
	};
	static struct Set_vector_with_help_data
		rotate_command_data={4," AXIS_X AXIS_Y AXIS_Z ANGLE",0};

	ENTER(modify_Graphics_window_image);
	if (state)
	{
		if (modify_graphics_window_data=(struct Modify_graphics_window_data *)
			modify_graphics_window_data_void)
		{
			if (state->current_token)
			{
				/* get defaults from scene_viewer for first pane of window */
				if ((window=(struct Graphics_window *)window_void)&&
					(scene_viewer=window->scene_viewer[0]))
				{
					if (light_model=Scene_viewer_get_light_model(scene_viewer))
					{
						ACCESS(Light_model)(light_model);
					}
					scene=ACCESS(Scene)(window->scene);
				}
				else
				{
					scene=(struct Scene *)NULL;
					scene_viewer=(struct Scene_viewer *)NULL;
					light_model=(struct Light_model *)NULL;
				}
				light_to_add=(struct Light *)NULL;
				light_to_remove=(struct Light *)NULL;
				rotate_command_data.set=0;
				rotate_angle=0.0;
				update_flag=0;
				view_all_flag=0;
				i=0;
				/* add_light */
				(option_table[i]).to_be_modified= &light_to_add;
				(option_table[i]).user_data=modify_graphics_window_data->light_manager;
				i++;
				/* light_model */
				(option_table[i]).to_be_modified= &light_model;
				(option_table[i]).user_data=
					modify_graphics_window_data->light_model_manager;
				i++;
				/* remove_light */
				(option_table[i]).to_be_modified= &light_to_remove;
				(option_table[i]).user_data=modify_graphics_window_data->light_manager;
				i++;
				/* rotate */
				(option_table[i]).to_be_modified= rotate_data;
				(option_table[i]).user_data= &rotate_command_data;
				i++;
				/* scene */
				(option_table[i]).to_be_modified= &scene;
				(option_table[i]).user_data=modify_graphics_window_data->scene_manager;
				i++;
				/* update */
				(option_table[i]).to_be_modified= &update_flag;
				i++;
				/* view_all */
				(option_table[i]).to_be_modified= &view_all_flag;
				i++;
				if (return_code=process_multiple_options(state,option_table))
				{
					if (scene_viewer)
					{
						return_code=1;
						if (light_to_add&&Scene_viewer_has_light(scene_viewer,light_to_add))
						{
							/*display_message(WARNING_MESSAGE,"Light is already in window");*/
							DEACCESS(Light)(&light_to_add);
							light_to_add=(struct Light *)NULL;
						}
						if (light_to_remove&&
							!Scene_viewer_has_light(scene_viewer,light_to_remove))
						{
							display_message(WARNING_MESSAGE,
								"Cannot remove light that is not in scene");
							DEACCESS(Light)(&light_to_remove);
							light_to_remove=(struct Light *)NULL;
						}
						if (rotate_command_data.set)
						{
							rotate_axis[0]=rotate_data[0];
							rotate_axis[1]=rotate_data[1];
							rotate_axis[2]=rotate_data[2];
							rotate_angle=rotate_data[3]*(PI/180.0);
						}
						/* set values for all panes */
						for (pane_no=0;pane_no<GRAPHICS_WINDOW_MAX_NUMBER_OF_PANES;
							pane_no++)
						{
							scene_viewer=window->scene_viewer[pane_no];
							if (light_model)
							{
								Scene_viewer_set_light_model(scene_viewer,light_model);
							}
							if (light_to_add)
							{
								Scene_viewer_add_light(scene_viewer,light_to_add);
							}
							if (light_to_remove)
							{
								Scene_viewer_remove_light(scene_viewer,light_to_remove);
							}
							if (rotate_command_data.set)
							{
								Scene_viewer_rotate_about_lookat_point(scene_viewer,
									rotate_axis,rotate_angle);
							}
							if (scene)
							{
								Scene_viewer_set_scene(scene_viewer,scene);
							}
						}
						if (scene)
						{
							/* maintain pointer to scene in graphics_window */
							ACCESS(Scene)(scene);
							DEACCESS(Scene)(&(window->scene));
							window->scene=scene;
							/* if the default_time_keeper of the new scene is different
								than the old_scene change the callback */
							new_time_keeper = Scene_get_default_time_keeper(scene);
							if(new_time_keeper != window->time_keeper)
							{
								if(window->time_keeper)
								{
									Time_keeper_remove_callback(window->time_keeper,
										Graphics_window_time_keeper_callback, (void *)window);
									DEACCESS(Time_keeper)(&(window->time_keeper));
								}
								if(new_time_keeper)
								{
									window->time_keeper = ACCESS(Time_keeper)(new_time_keeper);
									Time_keeper_add_callback(window->time_keeper,
										Graphics_window_time_keeper_callback, (void *)window,
										(enum Time_keeper_event)(TIME_KEEPER_NEW_TIME | 
										TIME_KEEPER_NEW_MINIMUM | TIME_KEEPER_NEW_MAXIMUM ));
									time_value = Time_keeper_get_minimum(window->time_keeper);
									edit_var_set_data(window->time_edit_widget,EDIT_VAR_LOW_LIMIT,
										time_value);
									time_value = Time_keeper_get_maximum(window->time_keeper);
									edit_var_set_data(window->time_edit_widget,EDIT_VAR_HIGH_LIMIT,
										time_value);
								}
								else
								{
									window->time_keeper = (struct Time_keeper *)NULL;
								}
							}
						}
						if (view_all_flag)
						{
							Graphics_window_view_all(window);
						}
						/* redraw all active scene_viewers */
						Graphics_window_update_now(window);
					}
					else
					{
						display_message(ERROR_MESSAGE,"modify_Graphics_window_image. "
							"Missing or invalid scene_viewer");
						display_parse_state_location(state);
						return_code=0;
					}
				}
				if (scene)
				{
					DEACCESS(Scene)(&scene);
				}
				if (light_model)
				{
					DEACCESS(Light_model)(&light_model);
				}
				if (light_to_add)
				{
					DEACCESS(Light)(&light_to_add);
				}
				if (light_to_remove)
				{
					DEACCESS(Light)(&light_to_remove);
				}
			}
			else
			{
				display_message(WARNING_MESSAGE,"Missing window image modifications");
				display_parse_state_location(state);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"modify_Graphics_window_image.  "
				"Missing modify_graphics_window_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"modify_Graphics_window_image.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* modify_Graphics_window_image */

static int modify_Graphics_window_layout(struct Parse_state *state,
	void *graphics_window_void,void *user_data_void)
/*******************************************************************************
LAST MODIFIED : 28 June 2000

DESCRIPTION :
Parser commands for setting the scene and how it is displayed (time, light model
etc.) in all panes of the <window>.
==============================================================================*/
{
	char *layout_mode_string,**valid_strings;
	double eye_spacing;
	enum Graphics_window_layout_mode layout_mode,old_layout_mode;
	int height,number_of_valid_strings,old_height,old_width,return_code,width;
	struct Graphics_window *graphics_window;
	struct Graphics_window_ortho_axes ortho_axes;
	struct Option_table *option_table;

	ENTER(modify_Graphics_window_layout);
	USE_PARAMETER(user_data_void);
	if (state)
	{
		if (state->current_token)
		{
			/* get defaults from scene_viewer for first pane of window */
			if (graphics_window=(struct Graphics_window *)graphics_window_void)
			{
				layout_mode=Graphics_window_get_layout_mode(graphics_window);
				Graphics_window_get_viewing_area_size(graphics_window,&width,&height);
				ortho_axes.up=graphics_window->ortho_up_axis;
				ortho_axes.front=graphics_window->ortho_front_axis;
				eye_spacing=graphics_window->eye_spacing;
			}
			else
			{
				layout_mode=GRAPHICS_WINDOW_LAYOUT_SIMPLE;
				width=0;
				height=0;
				ortho_axes.up=0;
				ortho_axes.front=0;
				eye_spacing=0.0;
			}
			old_width=width;
			old_height=height;
			old_layout_mode=layout_mode;

			option_table=CREATE(Option_table)();
			/* layout_mode */
			layout_mode_string=Graphics_window_layout_mode_string(layout_mode);
			valid_strings=Graphics_window_layout_mode_get_valid_strings(
				&number_of_valid_strings);
			Option_table_add_enumerator(option_table,number_of_valid_strings,
				valid_strings,&layout_mode_string);
			DEALLOCATE(valid_strings);
			/* eye_spacing */
			Option_table_add_entry(option_table,"eye_spacing",&eye_spacing,
				NULL,set_double);
			/* height */
			Option_table_add_entry(option_table,"height",&height,
				NULL,set_int_non_negative);
			/* ortho_axes */
			Option_table_add_entry(option_table,"ortho_axes",&ortho_axes,
				NULL,set_Graphics_window_ortho_axes);
			/* width */
			Option_table_add_entry(option_table,"width",&width,
				NULL,set_int_non_negative);
			if (return_code=Option_table_multi_parse(option_table,state))
			{
				if (graphics_window)
				{
					if ((ortho_axes.up != graphics_window->ortho_up_axis)||
						(ortho_axes.front != graphics_window->ortho_front_axis))
					{
						Graphics_window_set_orthographic_axes(graphics_window,
							ortho_axes.up,ortho_axes.front);
						/* always force layout to be reset */
						old_layout_mode=GRAPHICS_WINDOW_LAYOUT_MODE_INVALID;
					}
					if (eye_spacing != graphics_window->eye_spacing)
					{
						Graphics_window_set_eye_spacing(graphics_window,eye_spacing);
						/* always force layout to be reset */
						old_layout_mode=GRAPHICS_WINDOW_LAYOUT_MODE_INVALID;
					}
					layout_mode=
						Graphics_window_layout_mode_from_string(layout_mode_string);
					if (layout_mode != old_layout_mode)
					{
						Graphics_window_set_layout_mode(graphics_window,layout_mode);
					}
					if ((width != old_width) || (height != old_height))
					{
						Graphics_window_set_viewing_area_size(graphics_window,
							width,height);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"modify_Graphics_window_layout.  Missing graphics_window");
					return_code=0;
				}
			}
			DESTROY(Option_table)(&option_table);
		}
		else
		{
			display_message(WARNING_MESSAGE,"Missing window layout modifications");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"modify_Graphics_window_layout.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* modify_Graphics_window_layout */

static int modify_Graphics_window_overlay(struct Parse_state *state,
	void *window_void,void *modify_graphics_window_data_void)
/*******************************************************************************
LAST MODIFIED : 18 November 1998

DESCRIPTION :
Parser commands for modifying the overlay scene of the current pane of the
<window>.
==============================================================================*/
{
	int i,return_code;
	struct Graphics_window *window;
	struct Modify_graphics_window_data *modify_graphics_window_data;
	struct Scene *overlay_scene,*previous_overlay_scene;
	struct Scene_viewer *scene_viewer;
	static struct Modifier_entry option_table[]=
	{
		{"scene",NULL,NULL,set_Scene},
		{NULL,NULL,NULL,NULL}
	};

	ENTER(modify_Graphics_window_overlay);
	if (state)
	{
		if (modify_graphics_window_data=(struct Modify_graphics_window_data *)
			modify_graphics_window_data_void)
		{
			if (state->current_token)
			{
				/* get defaults from scene_viewer for first pane of window */
				if ((window=(struct Graphics_window *)window_void)&&
					(scene_viewer=window->scene_viewer[window->current_pane]))
				{
					if (overlay_scene=Scene_viewer_get_overlay_scene(scene_viewer))
					{
						ACCESS(Scene)(overlay_scene);
					}
				}
				else
				{
					scene_viewer=(struct Scene_viewer *)NULL;
					overlay_scene=(struct Scene *)NULL;
				}
				previous_overlay_scene=overlay_scene;
				i=0;
				/* scene */
				(option_table[i]).to_be_modified= &overlay_scene;
				(option_table[i]).user_data=modify_graphics_window_data->scene_manager;
				i++;
				if (return_code=process_multiple_options(state,option_table))
				{
					if (scene_viewer)
					{
						return_code=1;
						if (overlay_scene != previous_overlay_scene)
						{
							return_code=
								Scene_viewer_set_overlay_scene(scene_viewer,overlay_scene);
							Scene_viewer_redraw_now(scene_viewer);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"modify_Graphics_window_overlay. "
							"Missing or invalid scene_viewer");
						display_parse_state_location(state);
						return_code=0;
					}
				}
				if (overlay_scene)
				{
					DEACCESS(Scene)(&overlay_scene);
				}
			}
			else
			{
				display_message(WARNING_MESSAGE,
					"Missing window overlay modifications");
				display_parse_state_location(state);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"modify_Graphics_window_overlay.  "
				"Missing modify_graphics_window_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"modify_Graphics_window_overlay.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* modify_Graphics_window_overlay */

int Graphics_window_set_antialias_mode(struct Graphics_window *graphics_window,
	int antialias_mode)
/*******************************************************************************
LAST MODIFIED : 13 June 2000

DESCRIPTION :
Sets the number of times the images is oversampled to antialias the image. Only
certain values are supported 0/1 = off, 2, 4 & 8 are on.
==============================================================================*/
{
	int pane_no,return_code;

	ENTER(Graphics_window_set_antialias_mode);
	if (graphics_window)
	{
		if (1==antialias_mode)
		{
			antialias_mode=0;
		}
		return_code=1;
		for (pane_no=0;(pane_no<GRAPHICS_WINDOW_MAX_NUMBER_OF_PANES)&&return_code;
			pane_no++)
		{
			return_code=Scene_viewer_set_antialias_mode(
				graphics_window->scene_viewer[pane_no],antialias_mode);
		}
		if (return_code)
		{
			graphics_window->antialias_mode=antialias_mode;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_set_antialias_mode.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* Graphics_window_set_antialias_mode */

int Graphics_window_set_perturb_lines(struct Graphics_window *graphics_window,
	int perturb_lines)
/*******************************************************************************
LAST MODIFIED : 13 June 2000

DESCRIPTION :
Sets if the <graphics_window> perturbs lines or not, using <perturb_lines>
(1==TRUE,0==FALSE)
==============================================================================*/
{
	int pane_no,return_code;

	ENTER(Graphics_window_set_perturb_lines);
	if (graphics_window)
	{
		return_code=1;
		for (pane_no=0;(pane_no<GRAPHICS_WINDOW_MAX_NUMBER_OF_PANES)&&return_code;
			pane_no++)
		{
			return_code=Scene_viewer_set_perturb_lines(
				graphics_window->scene_viewer[pane_no],perturb_lines);
		}
		if (return_code)
		{
			graphics_window->perturb_lines=perturb_lines;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_set_perturb_lines.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* Graphics_window_set_perturb_lines */

static int modify_Graphics_window_set(struct Parse_state *state,
	void *window_void,void *modify_graphics_window_data_void)
/*******************************************************************************
LAST MODIFIED : 13 June 2000

DESCRIPTION :
Parser commands for setting simple parameters applicable to the whole <window>.
==============================================================================*/
{
	char fast_transparency_flag,slow_transparency_flag,*tool_name,**tool_names;
	double std_view_angle;
	enum Scene_viewer_transparency_mode transparency_mode;
	int antialias_mode,current_pane,i,layered_transparency,number_of_tools,
		pane_no,perturb_lines,redraw,return_code;
	struct Graphics_window *graphics_window;
	struct Interactive_tool *interactive_tool;
	struct Modify_graphics_window_data *modify_graphics_window_data;
	struct Option_table *antialias_option_table,*option_table,
		*transparency_option_table;

	ENTER(modify_Graphics_window_set);
	if (state)
	{
		if (modify_graphics_window_data=(struct Modify_graphics_window_data *)
			modify_graphics_window_data_void)
		{
			/* Keep the handle in case we need it sometime */
			USE_PARAMETER(modify_graphics_window_data);
			if (state->current_token)
			{
				/* get defaults from scene_viewer for first pane of window */
				if (graphics_window=(struct Graphics_window *)window_void)
				{
					current_pane=graphics_window->current_pane+1;
					std_view_angle=graphics_window->std_view_angle;
					interactive_tool=graphics_window->interactive_tool;
					antialias_mode=graphics_window->antialias_mode;
					perturb_lines=graphics_window->perturb_lines;
				}
				else
				{
					current_pane=1;
					std_view_angle=40.0;
					interactive_tool=(struct Interactive_tool *)NULL;
					antialias_mode=0;
					perturb_lines=0;
				}
				fast_transparency_flag=0;
				slow_transparency_flag=0;
				layered_transparency = 0;

				option_table=CREATE(Option_table)();
				/* antialias/no_antialias */
				antialias_option_table=CREATE(Option_table)();
				Option_table_add_entry(antialias_option_table,"antialias",
					&antialias_mode,(void *)NULL,set_int_positive);
				Option_table_add_entry(antialias_option_table,"no_antialias",
					&antialias_mode,(void *)NULL,unset_int_switch);
				Option_table_add_suboption_table(option_table,antialias_option_table);
				/* current_pane */
				Option_table_add_entry(option_table,"current_pane",
					&current_pane,(void *)NULL,set_int);
				/* transform|other tools. tool_names not deallocated until later */
				tool_name=(char *)NULL;
				if (tool_names=interactive_tool_manager_get_tool_names(
					modify_graphics_window_data->interactive_tool_manager,
					&number_of_tools,interactive_tool,&tool_name))
				{
					Option_table_add_enumerator(option_table,number_of_tools,
						tool_names,&tool_name);
				}
				/* perturb_lines|normal_lines */
				Option_table_add_switch(option_table,"perturb_lines","normal_lines",
					&perturb_lines);
				/* std_view_angle */
				Option_table_add_entry(option_table,"std_view_angle",
					&std_view_angle,(void *)NULL,set_double);
				/* fast_transparency|slow_transparency */
				transparency_option_table=CREATE(Option_table)();
				Option_table_add_entry(transparency_option_table,"fast_transparency",
					&fast_transparency_flag,(void *)NULL,set_char_flag);
				Option_table_add_entry(transparency_option_table,
					"layered_transparency",&layered_transparency,(void *)NULL,
					set_int_positive);
				Option_table_add_entry(transparency_option_table,"slow_transparency",
					&slow_transparency_flag,(void *)NULL,set_char_flag);
				Option_table_add_suboption_table(option_table,
					transparency_option_table);
				if (return_code=Option_table_multi_parse(option_table,state))
				{
					if ((1>current_pane)||(current_pane>graphics_window->number_of_panes))
					{
						display_message(WARNING_MESSAGE,"current_pane may be from 1 to %d",
							graphics_window->number_of_panes);
						current_pane=graphics_window->current_pane+1;
					}
					if (fast_transparency_flag&&slow_transparency_flag&&
						 layered_transparency)
					{
						display_message(ERROR_MESSAGE,"Only one of "
							"fast_transparency/slow_transparency/layered_transparency");
						fast_transparency_flag=0;
						slow_transparency_flag=0;
						layered_transparency=0;
					}
					if (graphics_window)
					{
						redraw=0;
						/* user deals with pane numbers one higher than internally */
						current_pane -= 1;
						if (current_pane != graphics_window->current_pane)
						{
							Graphics_window_set_current_pane(graphics_window,current_pane);
						}
						if (interactive_tool=
							FIND_BY_IDENTIFIER_IN_MANAGER(Interactive_tool,name)(
								tool_name,graphics_window->interactive_tool_manager))
						{
							Graphics_window_set_interactive_tool(graphics_window,
								interactive_tool);
						}
						if (std_view_angle != graphics_window->std_view_angle)
						{
							Graphics_window_set_std_view_angle(graphics_window,
								std_view_angle);
						}
						if (antialias_mode != graphics_window->antialias_mode)
						{
							Graphics_window_set_antialias_mode(graphics_window,
								antialias_mode);
							redraw=1;
						}
						if (perturb_lines != graphics_window->perturb_lines)
						{
							Graphics_window_set_perturb_lines(graphics_window,perturb_lines);
							redraw=1;
						}
						if (fast_transparency_flag||slow_transparency_flag||
							layered_transparency)
						{
							if (fast_transparency_flag)
							{
								transparency_mode=SCENE_VIEWER_FAST_TRANSPARENCY;
							}
							else if (layered_transparency)
							{
								transparency_mode=SCENE_VIEWER_LAYERED_TRANSPARENCY;
							}
							else
							{
								transparency_mode=SCENE_VIEWER_SLOW_TRANSPARENCY;
							}
							for (pane_no=0;pane_no<GRAPHICS_WINDOW_MAX_NUMBER_OF_PANES;
								pane_no++)
							{
								Scene_viewer_set_transparency_mode(
									graphics_window->scene_viewer[pane_no],transparency_mode);
								if (layered_transparency)
								{
									Scene_viewer_set_transparency_layers(
										graphics_window->scene_viewer[pane_no],
										layered_transparency);
								}
							}
							redraw=1;
						}
						if (redraw)
						{
							Graphics_window_update_now(graphics_window);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"modify_Graphics_window_set.  Missing window");
						return_code=0;
					}
				}
				if (tool_names)
				{
					for (i=0;i<number_of_tools;i++)
					{
						DEALLOCATE(tool_names[i]);
					}
					DEALLOCATE(tool_names);
				}
				DESTROY(Option_table)(&option_table);
			}
			else
			{
				display_message(WARNING_MESSAGE,"Missing window settings");
				display_parse_state_location(state);
				return_code=0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"modify_Graphics_window_set.  "
				"Missing modify_graphics_window_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"modify_Graphics_window_set.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* modify_Graphics_window_set */

static int modify_Graphics_window_view(struct Parse_state *state,
	void *window_void,void *modify_graphics_window_data_void)
/*******************************************************************************
LAST MODIFIED : 26 November 2001

DESCRIPTION :
Parser commands for modifying the view in the current pane of <window>,
view angle, interest point etc.
==============================================================================*/
{
	char allow_skew_flag,absolute_viewport_flag,relative_viewport_flag,
		custom_projection_flag,parallel_projection_flag,perspective_projection_flag;
	double bottom,eye[3],clip_plane_add[4],clip_plane_remove[4],far,left,lookat[3],
		modelview_matrix[16],
		ndc_placement[4],near,photogrammetry_matrix[12],projection_matrix[16],right,
		top,up[3],view_angle,viewport_coordinates[4];
	enum Scene_viewer_projection_mode projection_mode;
	int i,number_of_components,return_code;
	struct Graphics_window *window;
	struct Scene_viewer *scene_viewer;
	static struct Modifier_entry
		projection_mode_option_table[]=
		{
			{"custom",NULL,NULL,set_char_flag},
			{"parallel",NULL,NULL,set_char_flag},
			{"perspective",NULL,NULL,set_char_flag},
			{NULL,NULL,NULL,NULL}
		},
		viewport_mode_option_table[]=
		{
			{"absolute_viewport",NULL,NULL,set_char_flag},
			{"relative_viewport",NULL,NULL,set_char_flag},
			{NULL,NULL,NULL,NULL}
		},
		option_table[]=
		{
			{"allow_skew",NULL,NULL,set_char_flag},
			{"clip_plane_add",NULL,NULL,set_double_vector_with_help},
			{"clip_plane_remove",NULL,NULL,set_double_vector_with_help},
			{"eye_point",NULL,NULL,set_double_vector},
			{"far_clipping_plane",NULL,NULL,set_double},
			{"interest_point",NULL,NULL,set_double_vector},
			{"modelview_matrix",NULL,NULL,set_double_vector_with_help},
			{"ndc_placement",NULL,NULL,set_double_vector_with_help},
			{"near_clipping_plane",NULL,NULL,set_double},
			{"photogrammetry_matrix",NULL,NULL,set_double_vector_with_help},
			{"projection_matrix",NULL,NULL,set_double_vector_with_help},
			{NULL,NULL,NULL,NULL},
			{"up_vector",NULL,NULL,set_double_vector},
			{"viewport_coordinates",NULL,NULL,set_double_vector_with_help},
			{"view_angle",NULL,NULL,set_double},
			{NULL,NULL,NULL,NULL},
			{NULL,NULL,NULL,NULL}
		};
	static struct Set_vector_with_help_data
		clip_plane_add_data=
			{4," A B C D",0},
		clip_plane_remove_data=
			{4," A B C D",0},
		modelview_matrix_data=
			{16," M11 M12 M13 M14 M21 M22 M23 M24 M31 M32 M33 M34 M41 M42 M43 M44",0},
		ndc_placement_data=
			{4," NDC_LEFT NDC_TOP NDC_WIDTH NDC_HEIGHT",0},
		photogrammetry_matrix_data=
			{12," T11 T12 T13 T21 T22 T23 T31 T32 T33 T41 T42 T43",0},
		projection_matrix_data=
			{16," M11 M12 M13 M14 M21 M22 M23 M24 M31 M32 M33 M34 M41 M42 M43 M44",0},
		viewport_coordinates_data=
			{4," VIEWPORT_LEFT VIEWPORT_TOP PIXELS_PER_UNIT_X PIXELS_PER_UNIT_Y",0};

	ENTER(modify_Graphics_window_view);
	USE_PARAMETER(modify_graphics_window_data_void);
	if (state)
	{
		if (state->current_token)
		{
			/* get defaults from scene_viewer of current pane of window */
			if ((window=(struct Graphics_window *)window_void)&&
				(scene_viewer=window->scene_viewer[window->current_pane])&&
				Scene_viewer_get_lookat_parameters(scene_viewer,
					&(eye[0]),&(eye[1]),&(eye[2]),
					&(lookat[0]),&(lookat[1]),&(lookat[2]),&(up[0]),&(up[1]),&(up[2]))&&
				Scene_viewer_get_viewing_volume(scene_viewer,&left,&right,
					&bottom,&top,&near,&far))
			{
				if (SCENE_VIEWER_CUSTOM==
					Scene_viewer_get_projection_mode(scene_viewer))
				{
					view_angle=0.0;
				}
				else
				{
					Scene_viewer_get_view_angle(scene_viewer,&view_angle);
				}
				view_angle *= (180.0/PI);
			}
			else
			{
				eye[0]=eye[1]=eye[2]=0.0;
				lookat[0]=lookat[1]=lookat[2]=0.0;
				up[0]=up[1]=up[2]=0.0;
				left=right=bottom=top=0.0;
				near=far=0.0;
				view_angle=0.0;
				scene_viewer=(struct Scene_viewer *)NULL;
			}
			allow_skew_flag=0;
			absolute_viewport_flag=0;
			relative_viewport_flag=0;
			custom_projection_flag=0;
			parallel_projection_flag=0;
			perspective_projection_flag=0;
			number_of_components=3;
			modelview_matrix_data.set=0;
			ndc_placement_data.set=0;
			photogrammetry_matrix_data.set=0;
			projection_matrix_data.set=0;
			viewport_coordinates_data.set=0;
			i=0;
			/* allow_skew */
			(option_table[i]).to_be_modified= &allow_skew_flag;
			i++;
			/* clip_plane_add */
			(option_table[i]).to_be_modified= (void *)clip_plane_add;
			(option_table[i]).user_data= &clip_plane_add_data;
			i++;
			/* clip_plane_remove */
			(option_table[i]).to_be_modified= (void *)clip_plane_remove;
			(option_table[i]).user_data= &clip_plane_remove_data;
			i++;
			/* eye_point */
			(option_table[i]).to_be_modified= eye;
			(option_table[i]).user_data= (void *)&number_of_components;
			i++;
			/* far_clipping_plane */
			(option_table[i]).to_be_modified= &far;
			i++;
			/* interest_point */
			(option_table[i]).to_be_modified= lookat;
			(option_table[i]).user_data= (void *)&number_of_components;
			i++;
			/* modelview_matrix */
			(option_table[i]).to_be_modified= (void *)modelview_matrix;
			(option_table[i]).user_data= &modelview_matrix_data;
			i++;
			/* ndc_placement (normalised device coordinate placement) */
			(option_table[i]).to_be_modified= (void *)ndc_placement;
			(option_table[i]).user_data= &ndc_placement_data;
			i++;
			/* near_clipping_plane */
			(option_table[i]).to_be_modified= &near;
			i++;
			/* photogrammetry_matrix */
			(option_table[i]).to_be_modified= (void *)photogrammetry_matrix;
			(option_table[i]).user_data= &photogrammetry_matrix_data;
			i++;
			/* projection_matrix */
			(option_table[i]).to_be_modified= (void *)projection_matrix;
			(option_table[i]).user_data= &projection_matrix_data;
			i++;
			/* projection mode: custom/parallel/perspective */
			(projection_mode_option_table[0]).to_be_modified=
				&custom_projection_flag;
			(projection_mode_option_table[1]).to_be_modified=
				&parallel_projection_flag;
			(projection_mode_option_table[2]).to_be_modified=
				&perspective_projection_flag;
			(option_table[i]).user_data=projection_mode_option_table;
			i++;
			/* up_vector */
			(option_table[i]).to_be_modified= up;
			(option_table[i]).user_data= (void *)&number_of_components;
			i++;
			/* viewport_coordinates */
			(option_table[i]).to_be_modified= (void *)viewport_coordinates;
			(option_table[i]).user_data= &viewport_coordinates_data;
			i++;
			/* view_angle */
			(option_table[i]).to_be_modified= &view_angle;
			i++;
			/* viewport mode: absolute_viewport/relative_viewport */
			(viewport_mode_option_table[0]).to_be_modified= &absolute_viewport_flag;
			(viewport_mode_option_table[1]).to_be_modified= &relative_viewport_flag;
			(option_table[i]).user_data=viewport_mode_option_table;
			i++;
			if (return_code=process_multiple_options(state,option_table))
			{
				if (absolute_viewport_flag&&relative_viewport_flag)
				{
					display_message(WARNING_MESSAGE,
						"Only one of absolute_viewport/relative_viewport");
					absolute_viewport_flag=0;
					relative_viewport_flag=0;
				}
				if (1<(custom_projection_flag+parallel_projection_flag+
					perspective_projection_flag))
				{
					display_message(WARNING_MESSAGE,
						"Only one of parallel/perspective/custom");
					custom_projection_flag=0;
					parallel_projection_flag=0;
					perspective_projection_flag=0;
				}
				if (photogrammetry_matrix_data.set&&
					(modelview_matrix_data.set || projection_matrix_data.set))
				{
					display_message(WARNING_MESSAGE,"photogrammetry_matrix "
						"may not be used with modelview_matrix or projection_matrix");
					photogrammetry_matrix_data.set=0;
				}
				if (scene_viewer)
				{
					if (parallel_projection_flag)
					{
						Graphics_window_set_projection_mode(window,window->current_pane,
							SCENE_VIEWER_PARALLEL);
					}
					if (perspective_projection_flag)
					{
						Graphics_window_set_projection_mode(window,window->current_pane,
							SCENE_VIEWER_PERSPECTIVE);
					}
					if (custom_projection_flag)
					{
						Graphics_window_set_projection_mode(window,window->current_pane,
							SCENE_VIEWER_CUSTOM);
					}
					projection_mode=
						Graphics_window_get_projection_mode(window,window->current_pane);
					/* must set viewing volume before view_angle otherwise view_angle
						 is overwritten */
					Scene_viewer_set_viewing_volume(scene_viewer,left,right,bottom,top,
						near,far);
					if (allow_skew_flag)
					{
						Scene_viewer_set_lookat_parameters(scene_viewer,
							eye[0],eye[1],eye[2],lookat[0],lookat[1],lookat[2],
							up[0],up[1],up[2]);
					}
					else
					{
						Scene_viewer_set_lookat_parameters_non_skew(scene_viewer,
							eye[0],eye[1],eye[2],lookat[0],lookat[1],lookat[2],
							up[0],up[1],up[2]);
					}
					/* must set view angle after lookat parameters */
					if ((0.0<view_angle)&&(view_angle<180.0))
					{
						Scene_viewer_set_view_angle(scene_viewer,view_angle*(PI/180.0));
					}
					else
					{
						display_message(WARNING_MESSAGE,
							"View angle should be between 0 and 180 degrees.");
					}
					if (absolute_viewport_flag)
					{
						Scene_viewer_set_viewport_mode(scene_viewer,
							SCENE_VIEWER_ABSOLUTE_VIEWPORT);
					}
					if (relative_viewport_flag)
					{
						Scene_viewer_set_viewport_mode(scene_viewer,
							SCENE_VIEWER_RELATIVE_VIEWPORT);
					}
					/*???RC should have checks on whether you can set these for the
						current layout_mode */
					if (modelview_matrix_data.set)
					{
						if (SCENE_VIEWER_CUSTOM==projection_mode)
						{
							Scene_viewer_set_modelview_matrix(scene_viewer,
								modelview_matrix);
						}
						else
						{
							display_message(WARNING_MESSAGE,
								"modelview_matrix may only be used with CUSTOM projection");
						}
					}
					/* must set ndc_placement before photogrammetry_matrix */
					if (ndc_placement_data.set)
					{
						Scene_viewer_set_NDC_info(scene_viewer,ndc_placement[0],
							ndc_placement[1],ndc_placement[2],ndc_placement[3]);
					}
					if (projection_matrix_data.set)
					{
						if (SCENE_VIEWER_CUSTOM==projection_mode)
						{
							Scene_viewer_set_projection_matrix(scene_viewer,
								projection_matrix);
						}
						else
						{
							display_message(WARNING_MESSAGE,
								"projection_matrix may only be used with CUSTOM projection");
						}
					}
					if (photogrammetry_matrix_data.set)
					{
						if (SCENE_VIEWER_CUSTOM==projection_mode)
						{
#if defined (MIRAGE)
							double NDC_left,NDC_bottom,NDC_top,NDC_width,NDC_height;

							/* photogrammetry_matrix must be after ndc_placement viewing
								 volume since it uses the latest NDC, near and far values */
							if (Scene_viewer_get_viewing_volume(scene_viewer,&left,&right,
								&bottom,&top,&near,&far)&&Scene_viewer_get_NDC_info(
									scene_viewer,&NDC_left,&NDC_top,&NDC_width,&NDC_height))
							{
								NDC_bottom=NDC_top-NDC_height;
								photogrammetry_to_graphics_projection(photogrammetry_matrix,
									near,far,NDC_left,NDC_bottom,NDC_width,NDC_height,
									modelview_matrix,projection_matrix,eye,lookat,up);
								Scene_viewer_set_modelview_matrix(scene_viewer,
									modelview_matrix);
								Scene_viewer_set_lookat_parameters(scene_viewer,
									eye[0],eye[1],eye[2],lookat[0],lookat[1],lookat[2],
									up[0],up[1],up[2]);
								Scene_viewer_set_NDC_info(scene_viewer,
									NDC_left,NDC_top,NDC_width,NDC_height);
								Scene_viewer_set_projection_matrix(scene_viewer,
									projection_matrix);
							}
							else
							{
								display_message(ERROR_MESSAGE,"modify_Graphics_window_view.  "
									"Could not apply photogrammetry_matrix");
								return_code=0;
							}
#else /* defined (MIRAGE) */
							display_message(INFORMATION_MESSAGE,
								"photogrammetry_matrix is not available\n");
#endif /* defined (MIRAGE) */
						}
						else
						{
							display_message(WARNING_MESSAGE,
								"photogrammetry_matrix may only be used with CUSTOM projection");
						}
					}
					if (viewport_coordinates_data.set)
					{
						Scene_viewer_set_viewport_info(scene_viewer,
							viewport_coordinates[0],viewport_coordinates[1],
							viewport_coordinates[2],viewport_coordinates[3]);
					}
					if (clip_plane_remove_data.set)
					{
						Scene_viewer_remove_clip_plane(scene_viewer,
							clip_plane_remove[0], clip_plane_remove[1], clip_plane_remove[2],
							clip_plane_remove[3]);
					}
					if (clip_plane_add_data.set)
					{
						Scene_viewer_add_clip_plane(scene_viewer,
							clip_plane_add[0], clip_plane_add[1], clip_plane_add[2],
							clip_plane_add[3]);
					}
					/* redraw the Scene_viewer */
					Scene_viewer_redraw_now(scene_viewer);
					/* allow changes to flow to tied panes */
					Graphics_window_view_changed(window,window->current_pane);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"modify_Graphics_window_view.  Missing or invalid scene_viewer");
					display_parse_state_location(state);
					return_code=0;
				}
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,"Missing window view modifications");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"modify_Graphics_window_view.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* modify_Graphics_window_view */

/*
Global functions
----------------
*/

struct Graphics_window *CREATE(Graphics_window)(char *name,
	enum Scene_viewer_buffer_mode buffer_mode,
	struct Colour *background_colour,
	struct MANAGER(Light) *light_manager,
	struct Light *default_light,
	struct MANAGER(Light_model) *light_model_manager,
	struct Light_model *default_light_model,
	struct MANAGER(Scene) *scene_manager,struct Scene *scene,
	struct MANAGER(Texture) *texture_manager,
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 2 November 2000

DESCRIPTION:
Creates a Graphics_window object, window shell and widgets. Returns a pointer
to the newly created object. The Graphics_window maintains a pointer to the
manager it is to live in, since users will want to close windows with the
window manager widgets.
Each window has a unique <name> that can be used to identify it, and which
will be printed on the windows title bar.
==============================================================================*/
{
	Atom WM_DELETE_WINDOW;
	char **valid_strings,*window_title;
	double eye[3],eye_distance,front[3],lookat[3],up[3],view[3];
	EDIT_VAR_PRECISION time_value;
	int init_widgets,number_of_valid_strings,ortho_front_axis,ortho_up_axis,
		pane_no,return_code;
	MrmType graphics_window_dialog_class;
	static MrmRegisterArg callbacks[] =
	{
		{"gwin_id_viewing_form",(XtPointer)
			DIALOG_IDENTIFY(graphics_window,viewing_form)},
		{"gwin_id_viewing_area1",(XtPointer)
			DIALOG_IDENTIFY(graphics_window,viewing_area1)},
		{"gwin_id_viewing_area2",(XtPointer)
			DIALOG_IDENTIFY(graphics_window,viewing_area2)},
		{"gwin_id_viewing_area3",(XtPointer)
			DIALOG_IDENTIFY(graphics_window,viewing_area3)},
		{"gwin_id_viewing_area4",(XtPointer)
			DIALOG_IDENTIFY(graphics_window,viewing_area4)},
		{"gwin_id_view_all_btn",(XtPointer)
			DIALOG_IDENTIFY(graphics_window,view_all_button)},
		{"gwin_id_print_btn",(XtPointer)
			DIALOG_IDENTIFY(graphics_window,print_button)},
		{"gwin_id_time_edit_form",(XtPointer)
			DIALOG_IDENTIFY(graphics_window,time_edit_form)},
		{"gwin_id_perspective_btn",(XtPointer)
			DIALOG_IDENTIFY(graphics_window,perspective_button)},
		{"gwin_id_layout_mode_form",(XtPointer)
			DIALOG_IDENTIFY(graphics_window,layout_mode_form)},
		{"gwin_id_orthographic_form",(XtPointer)
			DIALOG_IDENTIFY(graphics_window,orthographic_form)},
		{"gwin_id_ortho_up_option",(XtPointer)
			DIALOG_IDENTIFY(graphics_window,ortho_up_option)},
		{"gwin_id_ortho_up_menu",(XtPointer)
			DIALOG_IDENTIFY(graphics_window,ortho_up_menu)},
		{"gwin_id_ortho_front_btn",(XtPointer)
			DIALOG_IDENTIFY(graphics_window,ortho_front_button)},
		{"gwin_id_interactive_tool_form",(XtPointer)
			DIALOG_IDENTIFY(graphics_window,interactive_toolbar_form)},
		{"gwin_destroy_CB",(XtPointer)Graphics_window_destroy_CB},
		{"gwin_view_all_btn_CB",
			(XtPointer)Graphics_window_view_all_button_CB},
		{"gwin_print_btn_CB",
			(XtPointer)Graphics_window_print_button_CB},
		{"gwin_perspective_btn_CB",
			(XtPointer)Graphics_window_perspective_button_CB},
		{"gwin_ortho_up_menu_CB",
			(XtPointer)Graphics_window_ortho_up_menu_CB},
		{"gwin_ortho_front_btn_CB",
			(XtPointer)Graphics_window_ortho_front_button_CB}
	};
	static MrmRegisterArg identifiers[] =
	{
		{"gwin_structure",(XtPointer)NULL}
	};
	struct Callback_data callback;
	struct Graphics_window *graphics_window=NULL;
	Widget viewing_area[4];

	ENTER(create_graphics_window);
	if (name&&((SCENE_VIEWER_SINGLE_BUFFER==buffer_mode)||
		(SCENE_VIEWER_DOUBLE_BUFFER==buffer_mode))&&background_colour&&
		light_manager&&light_model_manager&&default_light_model&&
		scene_manager&&scene&&texture_manager&&interactive_tool_manager&&
		user_interface)
	{
		if (MrmOpenHierarchy_base64_string(graphics_window_uidh,
			&graphics_window_hierarchy,&graphics_window_hierarchy_open))
		{
			/* Try to allocate space for the window structure */
			if (ALLOCATE(graphics_window,struct Graphics_window,1)&&
				ALLOCATE(graphics_window->name,char,strlen(name)+1))
			{
				strcpy(graphics_window->name,name);
				/* initialize the fields of the window structure */
				graphics_window->access_count=0;
				graphics_window->eye_spacing=0.25;
				graphics_window->std_view_angle=40.0;
					/*???RC should be read in from defaults file */
				graphics_window->graphics_window_manager=
					(struct MANAGER(Graphics_window) *)NULL;
				graphics_window->light_manager=light_manager;
				graphics_window->light_model_manager=light_model_manager;
				graphics_window->scene_manager=scene_manager;
				graphics_window->texture_manager=texture_manager;
				graphics_window->scene=ACCESS(Scene)(scene);
				/* I have accessed the time_keeper further down */
				graphics_window->time_keeper = Scene_get_default_time_keeper(scene);
				graphics_window->interactive_tool_manager=interactive_tool_manager;
				graphics_window->interactive_tool=
					FIND_BY_IDENTIFIER_IN_MANAGER(Interactive_tool,name)(
						"transform_tool",graphics_window->interactive_tool_manager);
				graphics_window->user_interface=user_interface;
				graphics_window->default_viewing_height=512;
				graphics_window->default_viewing_width=512;
				graphics_window->default_translate_rate=1.0;
				graphics_window->default_tumble_rate=1.5;
				graphics_window->default_zoom_rate=1.0;
				graphics_window->layout_mode=GRAPHICS_WINDOW_LAYOUT_ORTHOGRAPHIC;
				graphics_window->number_of_panes=GRAPHICS_WINDOW_MAX_NUMBER_OF_PANES;
				for (pane_no=0;pane_no<GRAPHICS_WINDOW_MAX_NUMBER_OF_PANES;pane_no++)
				{
					graphics_window->scene_viewer[pane_no]=(struct Scene_viewer *)NULL;
				}
				graphics_window->current_pane=0;
				graphics_window->antialias_mode=0;
				graphics_window->perturb_lines=0;
				/* the input_mode set here is changed below */
				graphics_window->input_mode=SCENE_VIEWER_NO_INPUT;
				/* read default settings from Cmgui defaults file */
				Graphics_window_read_defaults(graphics_window);
				if (ALLOCATE(window_title,char,50+strlen(name)))
				{
					sprintf(window_title,"CMGUI Graphics window %s",name);
				}
				/* clear widgets yet to be read in and created */
				graphics_window->window_shell=(Widget)NULL;
				graphics_window->main_window=(Widget)NULL;
				graphics_window->viewing_form=(Widget)NULL;
				graphics_window->viewing_area1=(Widget)NULL;
				graphics_window->viewing_area2=(Widget)NULL;
				graphics_window->viewing_area3=(Widget)NULL;
				graphics_window->viewing_area4=(Widget)NULL;
				graphics_window->view_all_button=(Widget)NULL;
				graphics_window->print_button=(Widget)NULL;
				graphics_window->time_edit_form=(Widget)NULL;
				graphics_window->time_edit_widget=(Widget)NULL;
				graphics_window->perspective_button=(Widget)NULL;
				graphics_window->layout_mode_form=(Widget)NULL;
				graphics_window->layout_mode_widget=(Widget)NULL;
				graphics_window->orthographic_form=(Widget)NULL;
				graphics_window->ortho_up_option=(Widget)NULL;
				graphics_window->ortho_up_menu=(Widget)NULL;
				graphics_window->ortho_front_button=(Widget)NULL;
				graphics_window->interactive_toolbar_form=(Widget)NULL;
				graphics_window->interactive_toolbar_widget=(Widget)NULL;
				graphics_window->ortho_up_axis=0;
				graphics_window->ortho_front_axis=0;
				/* create a shell for the window */
				if (graphics_window->window_shell=XtVaCreatePopupShell(
					"graphics_shell",xmDialogShellWidgetClass,
					user_interface->application_shell,
					XmNdeleteResponse,XmDO_NOTHING,
					XmNmwmDecorations,MWM_DECOR_ALL,
					XmNmwmFunctions,MWM_FUNC_ALL,
					/*XmNtransient,FALSE,*/
					XmNallowShellResize,False,
					/* needs to be False to stop the window resizing when
					set_view_panel_from_toggles is called (due to changing the labels) */
					XmNtitle,window_title,
					NULL))
				{
					/* Register the shell with the busy signal list */
					create_Shell_list_item(&(graphics_window->window_shell),
						user_interface);
					/* Set up window manager callback for close window message */
					WM_DELETE_WINDOW=XmInternAtom(
						XtDisplay(graphics_window->window_shell),"WM_DELETE_WINDOW",False);
					XmAddWMProtocolCallback(graphics_window->window_shell,
						WM_DELETE_WINDOW,Graphics_window_close_CB,graphics_window);
					/* register callbacks */
					if (MrmSUCCESS!=MrmRegisterNamesInHierarchy(graphics_window_hierarchy,
						callbacks,XtNumber(callbacks)))
					{
						destroy_Shell_list_item_from_shell(&(graphics_window->window_shell),
							graphics_window->user_interface);
						XtDestroyWidget(graphics_window->window_shell);
						DEACCESS(Scene)(&(graphics_window->scene));
						DEALLOCATE(graphics_window->name);
						DEALLOCATE(graphics_window);
						display_message(ERROR_MESSAGE,
							"CREATE(graphics_window).  Could not register the callbacks");
					}
					else
					{
						/* register identifiers */
						identifiers[0].value=(XtPointer)graphics_window;
						if (MrmSUCCESS != MrmRegisterNamesInHierarchy(
							graphics_window_hierarchy,identifiers,XtNumber(identifiers)))
						{
							destroy_Shell_list_item_from_shell(
								&(graphics_window->window_shell),
								graphics_window->user_interface);
							XtDestroyWidget(graphics_window->window_shell);
							DEACCESS(Scene)(&(graphics_window->scene));
							DEALLOCATE(graphics_window->name);
							DEALLOCATE(graphics_window);
							display_message(ERROR_MESSAGE,
								"CREATE(graphics_window).  Could not register the identifiers");
						}
						else
						{
							/* Get the graphics window from the uid file */
							if (MrmSUCCESS != MrmFetchWidget(graphics_window_hierarchy,
								"graphics_window",graphics_window->window_shell,
								&graphics_window->main_window,&graphics_window_dialog_class))
							{
								destroy_Shell_list_item_from_shell(
									&(graphics_window->window_shell),
									graphics_window->user_interface);
								XtDestroyWidget(graphics_window->window_shell);
								DEACCESS(Scene)(&(graphics_window->scene));
								DEALLOCATE(graphics_window->name);
								DEALLOCATE(graphics_window);
								display_message(ERROR_MESSAGE,
									"CREATE(graphics_window).  Could not retrieve the 3D window");
							}
							else
							{
								init_widgets=1;
								/* create the subwidgets with default values */
								if (graphics_window->interactive_toolbar_widget=
									create_interactive_toolbar_widget(
										graphics_window->interactive_toolbar_form,
										interactive_tool_manager,INTERACTIVE_TOOLBAR_VERTICAL))
								{
									FOR_EACH_OBJECT_IN_MANAGER(Interactive_tool)(
										add_interactive_tool_to_interactive_toolbar_widget,
										(void *)graphics_window->interactive_toolbar_widget,
										interactive_tool_manager);
								}
								else
								{
									init_widgets=0;
								}
								/* create the time editing widget */
								if (!(graphics_window->time_edit_widget=create_edit_var_widget(
									graphics_window->time_edit_form,"Time",0.0,0.0,1.0)))
								{
									init_widgets=0;
								}
								valid_strings=Graphics_window_layout_mode_get_valid_strings(
									&number_of_valid_strings);
								if (graphics_window->layout_mode_widget=
									create_choose_enumerator_widget(
										graphics_window->layout_mode_form,
										number_of_valid_strings,valid_strings,
										Graphics_window_layout_mode_string(
											graphics_window->layout_mode), user_interface))
								{
									/* get callbacks for change of layout mode */
									callback.data=(void *)graphics_window;
									callback.procedure=Graphics_window_layout_mode_CB;
									choose_enumerator_set_callback(
										graphics_window->layout_mode_widget,&callback);
								}
								else
								{
									init_widgets=0;
								}
								DEALLOCATE(valid_strings);
								/* create the time editing widget */
								if (!init_widgets)
								{
									destroy_Shell_list_item_from_shell(
										&(graphics_window->window_shell),
										graphics_window->user_interface);
									XtDestroyWidget(graphics_window->window_shell);
									DEACCESS(Scene)(&(graphics_window->scene));
									DEALLOCATE(graphics_window->name);
									DEALLOCATE(graphics_window);
									display_message(ERROR_MESSAGE,
										"CREATE(graphics_window).  Could not create subwidgets");
								}
								else
								{
									install_accelerators(graphics_window->window_shell,
										graphics_window->window_shell);
									/* turn on callbacks */
									callback.procedure=Graphics_window_time_edit_CB;
									callback.data=graphics_window;
									edit_var_set_callback(graphics_window->time_edit_widget,
										&callback);
									callback.procedure=Graphics_window_update_interactive_tool;
									interactive_toolbar_widget_set_callback(
										graphics_window->interactive_toolbar_widget,&callback);
									/* create four Scene_viewers */
									viewing_area[0]=graphics_window->viewing_area1;
									viewing_area[1]=graphics_window->viewing_area2;
									viewing_area[2]=graphics_window->viewing_area3;
									viewing_area[3]=graphics_window->viewing_area4;
									return_code=1;
									for (pane_no=0;return_code&&
										(pane_no<GRAPHICS_WINDOW_MAX_NUMBER_OF_PANES);pane_no++)
									{
										if (graphics_window->scene_viewer[pane_no]=
											CREATE(Scene_viewer)(viewing_area[pane_no],
											background_colour,buffer_mode,
											light_manager,default_light,
											light_model_manager,default_light_model,
											scene_manager,graphics_window->scene,
											texture_manager,graphics_window->user_interface))
										{
											Scene_viewer_set_interactive_tool(
												graphics_window->scene_viewer[pane_no],
												graphics_window->interactive_tool);
											/* get scene_viewer transform callbacks to allow
												 synchronising of views in multiple panes */
											Scene_viewer_sync_add_callback(
												graphics_window->scene_viewer[pane_no],
												Graphics_window_Scene_viewer_view_changed,
												graphics_window);
											/*???RC temporary */
											Scene_viewer_set_transform_rate(
												graphics_window->scene_viewer[pane_no],2.0,1.5,2.0);
										}
										else
										{
											while (0<pane_no)
											{
												pane_no--;
												DESTROY(Scene_viewer)(
													&(graphics_window->scene_viewer[pane_no]));
											}
											return_code=0;
										}
									}
									if (!return_code)
									{
										destroy_Shell_list_item_from_shell(
											&(graphics_window->window_shell),
											graphics_window->user_interface);
										XtDestroyWidget(graphics_window->window_shell);
										DEACCESS(Scene)(&(graphics_window->scene));
										DEALLOCATE(graphics_window->name);
										DEALLOCATE(graphics_window);
										display_message(ERROR_MESSAGE,"CREATE(graphics_window).  "
											"Could not create Scene_viewer");
									}
									else
									{
										for (pane_no=0;
											(pane_no<GRAPHICS_WINDOW_MAX_NUMBER_OF_PANES);pane_no++)
										{
											Scene_viewer_set_transform_rate(
												graphics_window->scene_viewer[pane_no],
												graphics_window->default_translate_rate,
												graphics_window->default_tumble_rate,
												graphics_window->default_zoom_rate);
										}
										Graphics_window_set_interactive_tool(
											graphics_window, graphics_window->interactive_tool);
										/* set and update the orthographic axes */
										ortho_up_axis=graphics_window->ortho_up_axis;
										ortho_front_axis=graphics_window->ortho_front_axis;
										graphics_window->ortho_up_axis=0;
										graphics_window->ortho_front_axis=0;
										Graphics_window_set_orthographic_axes(graphics_window,
											ortho_up_axis,ortho_front_axis);
										/* The time_slider receives messages from the
											default_time_keeper of the scene */
										ACCESS(Time_keeper)(graphics_window->time_keeper);
										Time_keeper_add_callback(graphics_window->time_keeper,
											Graphics_window_time_keeper_callback,
											(void *)graphics_window,
											(enum Time_keeper_event) (TIME_KEEPER_NEW_TIME | 
											TIME_KEEPER_NEW_MINIMUM | TIME_KEEPER_NEW_MAXIMUM ));
										time_value = Time_keeper_get_minimum(graphics_window->time_keeper);
										edit_var_set_data(graphics_window->time_edit_widget,
											EDIT_VAR_LOW_LIMIT, time_value );
										time_value = Time_keeper_get_maximum(graphics_window->time_keeper);
										edit_var_set_data(graphics_window->time_edit_widget,
											EDIT_VAR_HIGH_LIMIT, time_value );
										/* initial view is of all of the current scene */
										Graphics_window_view_all(graphics_window);
										/* deferred from above for OpenGL */
										XtManageChild(graphics_window->main_window);
										/*XtRealizeWidget(graphics_window->window_shell);*/
										XtPopup(graphics_window->window_shell,XtGrabNone);
										/* make sure the first scene_viewer shows the front view */
										if (Scene_viewer_get_lookat_parameters(
											graphics_window->scene_viewer[0],
											&(eye[0]),&(eye[1]),&(eye[2]),
											&(lookat[0]),&(lookat[1]),&(lookat[2]),
											&(up[0]),&(up[1]),&(up[2]))&&
											axis_number_to_axis_vector(
												graphics_window->ortho_up_axis,up)&&
											axis_number_to_axis_vector(
												graphics_window->ortho_front_axis,front))
										{
											view[0]=eye[0]-lookat[0];
											view[1]=eye[1]-lookat[1];
											view[2]=eye[2]-lookat[2];
											eye_distance=normalize3(view);
											Scene_viewer_set_lookat_parameters(
												graphics_window->scene_viewer[0],
												lookat[0]+eye_distance*front[0],
												lookat[1]+eye_distance*front[1],
												lookat[2]+eye_distance*front[2],
												lookat[0],lookat[1],lookat[2],up[0],up[1],up[2]);
									/*Scene_viewer_redraw_now(graphics_window->scene_viewer[0]);*/
										}
										/* set the initial layout */
										Graphics_window_set_layout_mode(graphics_window,
											GRAPHICS_WINDOW_LAYOUT_SIMPLE);
										/* give the window its default size */
										Graphics_window_set_viewing_area_size(graphics_window,
											graphics_window->default_viewing_width,
											graphics_window->default_viewing_height);
									}
								}
							}
						}
					}
				}
				else
				{
					DEACCESS(Scene)(&(graphics_window->scene));
					DEALLOCATE(graphics_window->name);
					DEALLOCATE(graphics_window);
					display_message(ERROR_MESSAGE,
						"CREATE(graphics_window).  Could not create a shell");
				}
				if (window_title)
				{
					DEALLOCATE(window_title);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"CREATE(graphics_window).  Insufficient memory for graphics window");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(graphics_window).  Could not open hierarchy");
			graphics_window=(struct Graphics_window *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(graphics_window).  Invalid argument(s)");
		graphics_window=(struct Graphics_window *)NULL;
	}
	LEAVE;

	return (graphics_window);
} /* CREATE(graphics_window) */

int DESTROY(Graphics_window)(struct Graphics_window **graphics_window_address)
/*******************************************************************************
LAST MODIFIED : 9 May 2000

DESCRIPTION:
Frees the contents of the Graphics_window structure and then the object itself,
then closes down the window shell and widgets it uses. Note that responsibility
for removing the graphics_window from a global list of windows is left with the
calling routine. See also Graphics_window_close_CB and
Graphics_window_destroy_CB.
==============================================================================*/
{
	int return_code,pane_no;
	struct Graphics_window *graphics_window;

	ENTER(DESTROY(graphics_window));
	if (graphics_window_address&&(graphics_window= *graphics_window_address))
	{
		/* close the Scene_viewer(s) */
		for (pane_no=0;pane_no<GRAPHICS_WINDOW_MAX_NUMBER_OF_PANES;pane_no++)
		{
			DESTROY(Scene_viewer)(&(graphics_window->scene_viewer[pane_no]));
		}
		destroy_Shell_list_item_from_shell(&(graphics_window->window_shell),
			graphics_window->user_interface);
		/* destroy the graphics window widget */
		XtDestroyWidget(graphics_window->window_shell);
		/* no longer accessing scene */
		DEACCESS(Scene)(&(graphics_window->scene));
		if(graphics_window->time_keeper)
		{
			Time_keeper_remove_callback(graphics_window->time_keeper,
				Graphics_window_time_keeper_callback, (void *)graphics_window);
			DEACCESS(Time_keeper)(&(graphics_window->time_keeper));
		}
		DEALLOCATE(graphics_window->name);
		DEALLOCATE(*graphics_window_address);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(graphics_window).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(graphics_window) */

DECLARE_OBJECT_FUNCTIONS(Graphics_window)
DECLARE_DEFAULT_GET_OBJECT_NAME_FUNCTION(Graphics_window)

DECLARE_INDEXED_LIST_FUNCTIONS(Graphics_window)
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Graphics_window, \
	name,char *,strcmp)
DECLARE_INDEXED_LIST_IDENTIFIER_CHANGE_FUNCTIONS(Graphics_window,name)

PROTOTYPE_MANAGER_COPY_WITH_IDENTIFIER_FUNCTION(Graphics_window,name)
{
	char *name;
	int return_code;

	ENTER(MANAGER_COPY_WITH_IDENTIFIER(Graphics_window,name));
	/* check arguments */
	if (source&&destination)
	{
		if (source->name)
		{
			if (ALLOCATE(name,char,strlen(source->name)+1))
			{
				strcpy(name,source->name);
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"MANAGER_COPY_WITH_IDENTIFIER(Graphics_window,name).  "
					"Insufficient memory");
				return_code=0;
			}
		}
		else
		{
			name=(char *)NULL;
			return_code=1;
		}
		if (return_code)
		{
			if (return_code = MANAGER_COPY_WITHOUT_IDENTIFIER(Graphics_window,name)(
				destination,source))
			{
				/* copy values */
				DEALLOCATE(destination->name);
				destination->name=name;
			}
			else
			{
				DEALLOCATE(name);
				display_message(ERROR_MESSAGE,
					"MANAGER_COPY_WITH_IDENTIFIER(Graphics_window,name).  "
					"Could not copy without identifier");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_WITH_IDENTIFIER(Graphics_window,name).  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITH_IDENTIFIER(Graphics_window,name) */

PROTOTYPE_MANAGER_COPY_WITHOUT_IDENTIFIER_FUNCTION(Graphics_window,name)
{
	int return_code;

	ENTER(MANAGER_COPY_WITHOUT_IDENTIFIER(Graphics_window,name));
	if (source&&destination)
	{
		/*???RC have problems with copying scene_manager? messages? */
		printf("MANAGER_COPY_WITHOUT_IDENTIFIER(Graphics_window,name).  "
			"Not used\n");
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_WITHOUT_IDENTIFIER(Graphics_window,name).  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITHOUT_IDENTIFIER(Graphics_window,name) */

PROTOTYPE_MANAGER_COPY_IDENTIFIER_FUNCTION(Graphics_window,name,char *)
{
	char *destination_name;
	int return_code;

	ENTER(MANAGER_COPY_IDENTIFIER(Graphics_window,name));
	if (name&&destination)
	{
		if (ALLOCATE(destination_name,char,strlen(name)+1))
		{
			strcpy(destination_name,name);
			if (destination->name)
			{
				DEALLOCATE(destination->name);
			}
			destination->name=destination_name;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"MANAGER_COPY_IDENTIFIER(Graphics_window,name).  Insufficient memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_IDENTIFIER(Graphics_window,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_IDENTIFIER(Graphics_window,name) */

/* NOTE: Using special ADD_OBJECT_TO_MANAGER function so that object keeps
	pointer to its manager while it is managed. */
DECLARE_MANAGER_FUNCTIONS(Graphics_window)

DECLARE_DEFAULT_MANAGED_OBJECT_NOT_IN_USE_FUNCTION(Graphics_window)

DECLARE_OBJECT_WITH_MANAGER_MANAGER_IDENTIFIER_FUNCTIONS(Graphics_window,name, \
	char *,graphics_window_manager)

char *Graphics_window_manager_get_new_name(
	struct MANAGER(Graphics_window) *graphics_window_manager)
/*******************************************************************************
LAST MODIFIED : 13 October 1998

DESCRIPTION :
Makes up a default name string for a graphics window, based on numbers and
starting at "1". Up to the calling routine to deallocate the returned string.
==============================================================================*/
{
	char *return_name,temp_name[10];
	int number;

	ENTER(Graphics_window_manager_get_new_name);
	if (graphics_window_manager)
	{
		number=1;
		sprintf(temp_name,"%d",number);
		while (FIND_BY_IDENTIFIER_IN_MANAGER(Graphics_window,name)(temp_name,
			graphics_window_manager))
		{
			number++;
			sprintf(temp_name,"%d",number);
		}
		if (ALLOCATE(return_name,char,strlen(temp_name)+1))
		{
			strcpy(return_name,temp_name);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_manager_get_new_name.  Missing graphics_window_manager");
		return_name=(char *)NULL;
	}
	LEAVE;

	return (return_name);
} /* Graphics_window_manager_get_new_name */

int Graphics_window_get_current_pane(struct Graphics_window *window)
/*******************************************************************************
LAST MODIFIED : 6 October 1998

DESCRIPTION :
Returns the current_pane of the <window>, from 0 to number_of_panes-1.
==============================================================================*/
{
	int current_pane;

	ENTER(Graphics_window_get_current_pane);
	if (window)
	{
		current_pane=window->current_pane;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_get_current_pane.  Invalid argument(s)");
		current_pane=0;
	}
	LEAVE;

	return (current_pane);
} /* Graphics_window_get_current_pane */

int Graphics_window_set_current_pane(struct Graphics_window *window,
	int pane_no)
/*******************************************************************************
LAST MODIFIED : 15 October 1998

DESCRIPTION :
Sets the current_pane of the <window> to <pane_no>, from 0 to number_of_panes-1.
==============================================================================*/
{
	int return_code;

	ENTER(Graphics_window_set_current_pane);
	if (window&&(0<=pane_no)&&(pane_no<window->number_of_panes))
	{
		window->current_pane=pane_no;
		/* make sure the parallel/perspective button is set up for pane */
		Graphics_window_set_projection_mode(window,pane_no,
			Scene_viewer_get_projection_mode(window->scene_viewer[pane_no]));

		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_set_current_pane.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_window_set_current_pane */

double Graphics_window_get_eye_spacing(struct Graphics_window *graphics_window)
/*******************************************************************************
LAST MODIFIED : 15 October 1998

DESCRIPTION :
Returns the eye_spacing from the <graphics_window> used for 3-D viewing.
==============================================================================*/
{
	double eye_spacing;

	ENTER(Graphics_window_get_eye_spacing);
	if (graphics_window)
	{
		eye_spacing=graphics_window->eye_spacing;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_get_eye_spacing.  Missing graphics window");
		eye_spacing=0.0;
	}
	LEAVE;

	return (eye_spacing);
} /* Graphics_window_get_eye_spacing */

int Graphics_window_set_eye_spacing(struct Graphics_window *graphics_window,
	double eye_spacing)
/*******************************************************************************
LAST MODIFIED : 13 November 1998

DESCRIPTION :
Sets the <eye_spacing> for the <graphics_window> used for 3-D viewing.
==============================================================================*/
{
	int return_code;

	ENTER(Graphics_window_set_eye_spacing);
	if (graphics_window)
	{
		graphics_window->eye_spacing=eye_spacing;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_set_eye_spacing.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_window_set_eye_spacing */

enum Scene_viewer_input_mode Graphics_window_get_input_mode(
	struct Graphics_window *window)
/*******************************************************************************
LAST MODIFIED : 6 October 1998

DESCRIPTION :
Returns the current input mode of the graphics window. Valid return values are
SCENE_VIEWER_NO_INPUT, SCENE_VIEWER_SELECT and SCENE_VIEWER_TRANSFORM.
==============================================================================*/
{
	enum Scene_viewer_input_mode input_mode;

	ENTER(Graphics_window_get_input_mode);
	if (window)
	{
		input_mode=window->input_mode;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_get_input_mode.  Invalid argument(s)");
		/* return any old value */
		input_mode=SCENE_VIEWER_NO_INPUT;
	}
	LEAVE;

	return (input_mode);
} /* Graphics_window_get_input_mode */

int Graphics_window_set_input_mode(struct Graphics_window *window,
	enum Scene_viewer_input_mode input_mode)
/*******************************************************************************
LAST MODIFIED : 6 October 1998

DESCRIPTION :
Sets the current input mode of the <window> to <input_mode>. Valid input_modes
are SCENE_VIEWER_NO_INPUT, SCENE_VIEWER_SELECT and SCENE_VIEWER_TRANSFORM.
==============================================================================*/
{
	int pane_no,return_code;

	ENTER(Graphics_window_set_input_mode);
	if (window&&((SCENE_VIEWER_NO_INPUT==input_mode)||
		(SCENE_VIEWER_SELECT==input_mode)||(SCENE_VIEWER_TRANSFORM==input_mode)))
	{
		return_code=1;
		for (pane_no=0;pane_no<GRAPHICS_WINDOW_MAX_NUMBER_OF_PANES;pane_no++)
		{
			Scene_viewer_set_input_mode(window->scene_viewer[pane_no],input_mode);
		}
		if (input_mode != window->input_mode)
		{
			window->input_mode=input_mode;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_set_input_mode.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_window_set_input_mode */

enum Graphics_window_layout_mode Graphics_window_get_layout_mode(
	struct Graphics_window *window)
/*******************************************************************************
LAST MODIFIED : 28 June 2000

DESCRIPTION :
Returns the layout mode in effect on the <window>.
==============================================================================*/
{
	enum Graphics_window_layout_mode layout_mode;

	ENTER(Graphics_window_get_layout_mode);
	if (window)
	{
		layout_mode=window->layout_mode;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_get_layout_mode.  Invalid argument(s)");
		layout_mode=GRAPHICS_WINDOW_LAYOUT_MODE_INVALID;
	}
	LEAVE;

	return (layout_mode);
} /* Graphics_window_get_layout_mode */

int Graphics_window_set_layout_mode(struct Graphics_window *window,
	enum Graphics_window_layout_mode layout_mode)
/*******************************************************************************
LAST MODIFIED : 29 September 2000

DESCRIPTION :
Sets the layout mode in effect on the <window>.
==============================================================================*/
{
	double eye[3],eye_distance,front[3],lookat[3],up[3],view[3];
	enum Scene_viewer_projection_mode projection_mode;
	int new_layout,pane_no,return_code;

	ENTER(Graphics_window_set_layout_mode);
	if (window)
	{
		return_code=1;
		if (new_layout=(layout_mode != window->layout_mode))
		{
			window->layout_mode=layout_mode;
			/* get the number of panes for the new layout */
			window->number_of_panes=
				Graphics_window_layout_mode_get_number_of_panes(layout_mode);
			/* make sure the current layout mode is displayed on the chooser */
			choose_enumerator_set_string(window->layout_mode_widget,
				Graphics_window_layout_mode_string(layout_mode));
			/* awaken scene_viewers in panes to be used; put others to sleep */
			for	(pane_no=0;pane_no<GRAPHICS_WINDOW_MAX_NUMBER_OF_PANES;pane_no++)
			{
				if (pane_no < window->number_of_panes)
				{
					Scene_viewer_awaken(window->scene_viewer[pane_no]);
				}
				else
				{
					Scene_viewer_sleep(window->scene_viewer[pane_no]);
				}
			}
		}
		/* get projection mode of pane 0, using parallel for custom */
		if (SCENE_VIEWER_PERSPECTIVE==
			Graphics_window_get_projection_mode(window,0))
		{
			projection_mode=SCENE_VIEWER_PERSPECTIVE;
		}
		else
		{
			projection_mode=SCENE_VIEWER_PARALLEL;
		}
		switch (layout_mode)
		{
			case GRAPHICS_WINDOW_LAYOUT_SIMPLE:
			{
				if (new_layout)
				{
					/* one single, fully customisable scene viewer */
					XtUnmanageChild(window->viewing_area2);
					XtUnmanageChild(window->viewing_area3);
					XtUnmanageChild(window->viewing_area4);
					/* re-enable tumbling in main scene viewer */
					Scene_viewer_set_transform_rate(window->scene_viewer[0],
						window->default_translate_rate,window->default_tumble_rate,
						window->default_zoom_rate);
					/*???RC temporary*/
					XtVaSetValues(window->viewing_area1,
						XmNrightPosition,2,XmNbottomPosition,2,NULL);
					/* grey-out orthographic view controls */
					XtSetSensitive(window->orthographic_form,False);
				}
			} break;
			case GRAPHICS_WINDOW_LAYOUT_2D:
			{
				if (new_layout)
				{
					/* disable tumbling in main scene viewer */
					Scene_viewer_set_transform_rate(window->scene_viewer[0],
						window->default_translate_rate,/*tumble_rate*/0.0,
						window->default_zoom_rate);
					/* single scene viewer with no tumbling, orientation controlled
						 by orthographic axes */
					XtUnmanageChild(window->viewing_area2);
					XtUnmanageChild(window->viewing_area3);
					XtUnmanageChild(window->viewing_area4);
					/*???RC temporary*/
					XtVaSetValues(window->viewing_area1,
						XmNrightPosition,2,XmNbottomPosition,2,NULL);
					/* un-grey orthographic view controls */
					XtSetSensitive(window->orthographic_form,True);
				}
				/* set the plan view in pane 0 */
				if (Scene_viewer_get_lookat_parameters(window->scene_viewer[0],
					&(eye[0]),&(eye[1]),&(eye[2]),
					&(lookat[0]),&(lookat[1]),&(lookat[2]),&(up[0]),&(up[1]),&(up[2]))&&
					axis_number_to_axis_vector(window->ortho_up_axis,up)&&
					axis_number_to_axis_vector(window->ortho_front_axis,front))
				{
					view[0]=eye[0]-lookat[0];
					view[1]=eye[1]-lookat[1];
					view[2]=eye[2]-lookat[2];
					eye_distance=normalize3(view);
					Scene_viewer_set_lookat_parameters(window->scene_viewer[0],
						lookat[0]+eye_distance*up[0],lookat[1]+eye_distance*up[1],
						lookat[2]+eye_distance*up[2],lookat[0],lookat[1],lookat[2],
						-front[0],-front[1],-front[2]);
					Scene_viewer_redraw_now(window->scene_viewer[0]);
				}
			} break;
			case GRAPHICS_WINDOW_LAYOUT_ORTHOGRAPHIC:
			case GRAPHICS_WINDOW_LAYOUT_FREE_ORTHO:
			{
				if (new_layout)
				{
					/* make sure pane 0 is not using a custom projection */
					Graphics_window_set_projection_mode(window,0,projection_mode);
					/* re-enable tumbling in main scene viewer */
					Scene_viewer_set_transform_rate(window->scene_viewer[0],
						window->default_translate_rate,window->default_tumble_rate,
						window->default_zoom_rate);
					/* make sure panes 1-3 use parallel projection and disable tumble */
					for (pane_no=1;pane_no<4;pane_no++)
					{
						Graphics_window_set_projection_mode(window,pane_no,
							SCENE_VIEWER_PARALLEL);
						Scene_viewer_set_transform_rate(window->scene_viewer[pane_no],
							window->default_translate_rate,/*tumble_rate*/0.0,
							window->default_zoom_rate);
					}
					XtVaSetValues(window->viewing_area1,
						XmNrightPosition,1,XmNbottomPosition,1,NULL);
					XtVaSetValues(window->viewing_area2,
						XmNbottomPosition,1,NULL);
					XtManageChild(window->viewing_area2);
					XtManageChild(window->viewing_area3);
					XtManageChild(window->viewing_area4);
					/* un-grey orthographic view controls */
					XtSetSensitive(window->orthographic_form,True);
				}
				/* four views, 3 tied together as front, side and plan views */
				/* set the plan view in pane 1 */
				if (Scene_viewer_get_lookat_parameters(window->scene_viewer[1],
						&(eye[0]),&(eye[1]),&(eye[2]),
						&(lookat[0]),&(lookat[1]),&(lookat[2]),&(up[0]),&(up[1]),&(up[2]))&&
					axis_number_to_axis_vector(window->ortho_up_axis,up)&&
					axis_number_to_axis_vector(window->ortho_front_axis,front))
				{
					view[0]=eye[0]-lookat[0];
					view[1]=eye[1]-lookat[1];
					view[2]=eye[2]-lookat[2];
					eye_distance=normalize3(view);
					Scene_viewer_set_lookat_parameters(window->scene_viewer[1],
						lookat[0]+eye_distance*up[0],lookat[1]+eye_distance*up[1],
						lookat[2]+eye_distance*up[2],lookat[0],lookat[1],lookat[2],
						-front[0],-front[1],-front[2]);
				}
				/* put tied views in proper relationship to plan view in pane 1 */
				Graphics_window_view_changed(window,1);
				/* Graphics_window_view_changed redraws the scene viewers that are
					 tied to pane 1, but not pane 1 itself, hence: */
				Scene_viewer_redraw_now(window->scene_viewer[1]);
			} break;
			case GRAPHICS_WINDOW_LAYOUT_FRONT_BACK:
			case GRAPHICS_WINDOW_LAYOUT_FRONT_SIDE:
			case GRAPHICS_WINDOW_LAYOUT_PSEUDO_3D:
			{
				if (new_layout)
				{
					/* make sure panes 0 and 1 use same projection, and enable tumble */
					for (pane_no=0;pane_no<2;pane_no++)
					{
						Graphics_window_set_projection_mode(window,pane_no,projection_mode);
						Scene_viewer_set_transform_rate(window->scene_viewer[pane_no],
							window->default_translate_rate,window->default_tumble_rate,
							window->default_zoom_rate);
					}
					XtVaSetValues(window->viewing_area1,
						XmNrightPosition,1,XmNbottomPosition,2,NULL);
					XtVaSetValues(window->viewing_area2,
						XmNbottomPosition,2,NULL);
					XtManageChild(window->viewing_area2);
					XtUnmanageChild(window->viewing_area3);
					XtUnmanageChild(window->viewing_area4);
					/* un-grey orthographic view controls */
					XtSetSensitive(window->orthographic_form,True);
				}
				if ((GRAPHICS_WINDOW_LAYOUT_FRONT_BACK==layout_mode)||
					(GRAPHICS_WINDOW_LAYOUT_FRONT_SIDE==layout_mode))
				{
					/* set the front view in pane 0 */
					if (Scene_viewer_get_lookat_parameters(window->scene_viewer[0],
						&(eye[0]),&(eye[1]),&(eye[2]),
						&(lookat[0]),&(lookat[1]),&(lookat[2]),&(up[0]),&(up[1]),&(up[2]))&&
						axis_number_to_axis_vector(window->ortho_up_axis,up)&&
						axis_number_to_axis_vector(window->ortho_front_axis,front))
					{
						view[0]=eye[0]-lookat[0];
						view[1]=eye[1]-lookat[1];
						view[2]=eye[2]-lookat[2];
						eye_distance=normalize3(view);
						Scene_viewer_set_lookat_parameters(window->scene_viewer[0],
							lookat[0]+eye_distance*front[0],lookat[1]+eye_distance*front[1],
							lookat[2]+eye_distance*front[2],lookat[0],lookat[1],lookat[2],
							up[0],up[1],up[2]);
					}
					Scene_viewer_redraw_now(window->scene_viewer[0]);
				}
				/* put tied views in proper relationship to front view in pane 0 */
				Graphics_window_view_changed(window,0);
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Graphics_window_set_layout_mode.  Unknown layout_mode");
				return_code=0;
			} break;
		}
		/* Always set current_pane to the pane 0 when layout re-established */
		Graphics_window_set_current_pane(window,0);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_set_layout_mode.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_window_set_layout_mode */

int Graphics_window_get_orthographic_axes(struct Graphics_window *window,
	int *ortho_up_axis,int *ortho_front_axis)
/*******************************************************************************
LAST MODIFIED : 13 October 1998

DESCRIPTION :
Returns the "up" and "front" axes of the graphics window.
Axis numbers are from 1 to 6, where 1=x, 2=y, 3=z, 4=-x, 5=-y and 6=-z.
==============================================================================*/
{
	int return_code;

	ENTER(Graphics_window_get_orthographic_axes);
	if (window&&ortho_up_axis&&ortho_front_axis)
	{
		*ortho_up_axis = window->ortho_up_axis;
		*ortho_front_axis = window->ortho_front_axis;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_get_orthographic_axes.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_window_get_orthographic_axes */

int Graphics_window_set_orthographic_axes(struct Graphics_window *window,
	int ortho_up_axis,int ortho_front_axis)
/*******************************************************************************
LAST MODIFIED : 8 October 1998

DESCRIPTION :
Sets the "up" and "front" axes of the graphics window. Used for layout_modes
such as GRAPHICS_WINDOW_LAYOUT_ORTHOGRAPHIC.
Axis numbers are from 1 to 6, where 1=x, 2=y, 3=z, 4=-x, 5=-y and 6=-z.
==============================================================================*/
{
	int return_code,new_up_axis,new_front_axis,num_children;
	XmString temp_string;
	Widget *child_list;

	ENTER(Graphics_window_set_orthographic_axes);
	if (window)
	{
		new_up_axis=0;
		new_front_axis=0;
		if (window->ortho_up_axis != ortho_up_axis)
		{
			window->ortho_up_axis=ortho_up_axis;
			new_up_axis=1;
		}
		if (window->ortho_front_axis != ortho_front_axis)
		{
			window->ortho_front_axis=ortho_front_axis;
			new_front_axis=1;
		}
		/* Ensure the up and front axes are legal... */
		if (!window->ortho_up_axis)
		{
			window->ortho_up_axis=3;
			new_up_axis=1;
		}
		if (!window->ortho_front_axis)
		{
			window->ortho_front_axis=5;
			new_front_axis=1;
		}
		/* ...and orthogonal */
		if ((window->ortho_up_axis % 3)==(window->ortho_front_axis % 3))
		{
			window->ortho_front_axis=(window->ortho_front_axis % 6)+1;
			new_front_axis=1;
		}
		/* update the widgets if values have changed in this function */
		if (new_up_axis)
		{
			/* get children of the menu so that one may be selected */
			XtVaGetValues(window->ortho_up_menu,XmNnumChildren,&num_children,
				XmNchildren,&child_list,NULL);
			if ((1 <= window->ortho_up_axis)&&(num_children >= window->ortho_up_axis))
			{
				XtVaSetValues(window->ortho_up_option,
					XmNmenuHistory,child_list[window->ortho_up_axis-1],NULL);
			}
		}
		if (new_front_axis)
		{
			temp_string=
				XmStringCreateSimple(axis_name[window->ortho_front_axis]);
			XtVaSetValues(window->ortho_front_button,
				XmNlabelString,temp_string,NULL);
			XmStringFree(temp_string);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_set_orthographic_axes.  Missing graphics_window");
		return_code=0;
	}
	LEAVE;

	return return_code;
} /* Graphics_window_set_orthographic_axes */

enum Scene_viewer_projection_mode Graphics_window_get_projection_mode(
	struct Graphics_window *window,int pane_no)
/*******************************************************************************
LAST MODIFIED : 5 October 1998

DESCRIPTION :
Returns the projection mode used by pane <pane_no> of <window>.
==============================================================================*/
{
	enum Scene_viewer_projection_mode projection_mode;

	ENTER(Graphics_window_get_projection_mode);
	if (window&&(0<=pane_no)&&(pane_no<window->number_of_panes)&&
		(window->scene_viewer[pane_no]))
	{
		projection_mode=Scene_viewer_get_projection_mode(
			window->scene_viewer[pane_no]);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_get_projection_mode.  Invalid argument(s)");
		projection_mode=SCENE_VIEWER_PARALLEL;
	}
	LEAVE;

	return (projection_mode);
} /* Graphics_window_get_projection_mode */

int Graphics_window_set_projection_mode(struct Graphics_window *window,
	int pane_no,enum Scene_viewer_projection_mode projection_mode)
/*******************************************************************************
LAST MODIFIED : 28 June 2000

DESCRIPTION :
Sets the <projection_mode> used by pane <pane_no> of <window>. Allowable values
are SCENE_VIEWER_PARALLEL,	SCENE_VIEWER_PERSPECTIVE and SCENE_VIEWER_CUSTOM.
Whether you can set this for a pane depends on current layout_mode of window.
Must call Graphics_window_view_changed after changing tied pane.
==============================================================================*/
{
	int return_code;

	ENTER(Graphics_window_set_projection_mode);
	if (window&&(0<=pane_no)&&(pane_no<window->number_of_panes)&&
		(window->scene_viewer[pane_no]))
	{
		if (Graphics_window_layout_mode_is_projection_mode_valid_for_pane(
			window->layout_mode,pane_no,projection_mode))
		{
			if (return_code=Scene_viewer_set_projection_mode(
				window->scene_viewer[pane_no],projection_mode))
			{
				/* update perspective button widget if current_pane changed */
				if (pane_no == window->current_pane)
				{
					/* set perspective widgets */
					if (SCENE_VIEWER_PERSPECTIVE == projection_mode)
					{
						XtVaSetValues(window->perspective_button,XmNset,True,NULL);
					}
					else
					{
						XtVaSetValues(window->perspective_button,XmNset,False,NULL);
					}
					if ((!Graphics_window_layout_mode_is_projection_mode_valid_for_pane(
						window->layout_mode,pane_no,SCENE_VIEWER_PERSPECTIVE))||
						(SCENE_VIEWER_CUSTOM==projection_mode))
					{
						XtSetSensitive(window->perspective_button,False);
					}
					else
					{
						XtSetSensitive(window->perspective_button,True);
					}
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Graphics_window_set_projection_mode.  "
				"Projection mode not valid for pane");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_set_projection_mode.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_window_set_projection_mode */

struct Scene *Graphics_window_get_Scene(struct Graphics_window *window)
/*******************************************************************************
LAST MODIFIED : 6 October 1998

DESCRIPTION :
Returns the Scene for the <graphics_window>.
???DB.  Used for setting the scene time.  Could alternatively have
	Graphics_window_set_time
==============================================================================*/
{
	struct Scene *scene;

	ENTER(Graphics_window_get_Scene);
	if (window)
	{
		scene=window->scene;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Graphics_window_get_Scene.  Missing window");
		scene=(struct Scene *)NULL;
	}
	LEAVE;

	return (scene);
} /* Graphics_window_get_Scene */

struct Scene_viewer *Graphics_window_get_Scene_viewer(
	struct Graphics_window *window,int pane_no)
/*******************************************************************************
LAST MODIFIED : 8 October 1998

DESCRIPTION :
Returns the Scene_viewer in pane <pane_no> of <window>. Calling function can
then set view and other parameters for the scene_viewer directly.
???RC  May not need this in the longer term. Better if not allowed to get at.
==============================================================================*/
{
	struct Scene_viewer *scene_viewer;

	ENTER(Graphics_window_get_Scene_viewer);
	if (window&&(0<=pane_no)&&(pane_no<window->number_of_panes))
	{
		scene_viewer=(window->scene_viewer)[pane_no];
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_get_Scene_viewer.  Invalid argument(s)");
		scene_viewer=(struct Scene_viewer *)NULL;
	}
	LEAVE;

	return (scene_viewer);
} /* Graphics_window_get_Scene_viewer */

double Graphics_window_get_std_view_angle(
	struct Graphics_window *graphics_window)
/*******************************************************************************
LAST MODIFIED : 16 December 1997

DESCRIPTION :
Returns the std_view_angle from the <graphics_window>.
==============================================================================*/
{
	double return_angle;

	ENTER(Graphics_window_get_std_view_angle);
	if (graphics_window)
	{
		return_angle=graphics_window->std_view_angle;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_get_std_view_angle.  Missing graphics window");
		return_angle=0.0;
	}
	LEAVE;

	return (return_angle);
} /* Graphics_window_get_std_view_angle */

int Graphics_window_set_std_view_angle(struct Graphics_window *graphics_window,
	double std_view_angle)
/*******************************************************************************
LAST MODIFIED : 6 October 1998

DESCRIPTION :
Sets the <std_view_angle> for the <graphics_window>. The std_view_angle (in
degrees) is used by the Graphics_window_view_all function which positions the
viewer the correct distance away to see the currently visible scene at that
angle.
==============================================================================*/
{
	int return_code;

	ENTER(Graphics_window_set_std_view_angle);
	if (graphics_window)
	{
		if ((1.0 <= std_view_angle)&&(179.0 >= std_view_angle))
		{
			graphics_window->std_view_angle=std_view_angle;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Graphics_window_set_std_view_angle.  std_view_angle out of range");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_set_std_view_angle.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_window_set_std_view_angle */

int Graphics_window_get_viewing_area_size(struct Graphics_window *window,
	int *viewing_width,int *viewing_height)
/*******************************************************************************
LAST MODIFIED : 14 October 1998

DESCRIPTION :
The viewing_width and viewing_height are the size of the viewing area when the
graphics window has only one pane. When multiple panes are used, they are
separated by 2 pixel borders within the viewing area.
==============================================================================*/
{
	Dimension height,width;
	int return_code;

	ENTER(Graphics_window_get_viewing_area_size);
	if (window&&viewing_width&&viewing_height)
	{
		XtVaGetValues(window->viewing_form,
			XmNwidth,&width,
			XmNheight,&height,NULL);
		/* subtract 2 pixel border */
		*viewing_width=((int)width-2);
		*viewing_height=((int)height-2);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_get_viewing_area_size.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_window_get_viewing_area_size */

int Graphics_window_set_viewing_area_size(struct Graphics_window *window,
	int viewing_width,int viewing_height)
/*******************************************************************************
LAST MODIFIED : 14 October 1998

DESCRIPTION :
The viewing_width and viewing_height are the size of the viewing area when the
graphics window has only one pane. When multiple panes are used, they are
separated by 2 pixel borders within the viewing area.
==============================================================================*/
{
	Dimension old_viewing_height,old_viewing_width,shell_height,shell_width;
	int pane_no,return_code;

	ENTER(Graphics_window_set_viewing_area_size);
	if (window&&(0<=viewing_width)&&(0<=viewing_height))
	{
		/* add allowance for two pixel border on scene_viewer */
		viewing_width += 2;
		viewing_height += 2;
		XtVaGetValues(window->viewing_form,
			XmNwidth,&old_viewing_width,
			XmNheight,&old_viewing_height,NULL);
		XtVaGetValues(window->main_window,
			XmNwidth,&shell_width,
			XmNheight,&shell_height,NULL);
		shell_width += (viewing_width-old_viewing_width);
		shell_height += (viewing_height-old_viewing_height);
		XtVaSetValues(window->window_shell,
			XmNwidth,shell_width,
			XmNheight,shell_height,NULL);
		/*XtVaSetValues(window->viewing_form,
			XmNheight,viewing_height,
			XmNwidth,viewing_width,NULL);*/
		for (pane_no=0;pane_no<GRAPHICS_WINDOW_MAX_NUMBER_OF_PANES;pane_no++)
		{
			Scene_viewer_set_border_width(window->scene_viewer[pane_no],1);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_set_viewing_area_size.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_window_set_viewing_area_size */

int Graphics_window_update(struct Graphics_window *graphics_window)
/*******************************************************************************
LAST MODIFIED : 6 October 1998

DESCRIPTION :
Forces a redraw on <graphics_window> at the next idle moment.
In future want a flag in the graphics window which if on delays the redraw
until an explicit "gfx update" is entered.
==============================================================================*/
{
	int return_code,pane_no;

	ENTER(Graphics_window_update);
	if (graphics_window)
	{
		for (pane_no=0;pane_no<graphics_window->number_of_panes;pane_no++)
		{
			Scene_viewer_redraw(graphics_window->scene_viewer[pane_no]);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_update.  Missing graphics window");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_window_update */

int Graphics_window_update_now(struct Graphics_window *graphics_window)
/*******************************************************************************
LAST MODIFIED : 6 October 1998

DESCRIPTION :
Forces a redraw on <graphics_window>.
In future want a flag in the graphics window which if on delays the redraw
until an explicit "gfx update" is entered.
==============================================================================*/
{
	int return_code,pane_no;

	ENTER(Graphics_window_update_now);
	if (graphics_window)
	{
#if defined (NEW_CODE)
		/* Handle all the X events so that the window resizing gets done */
		while (XtAppPending(graphics_window->user_interface->application_context))
		{
			application_main_step(graphics_window->user_interface);
		}
#endif /* defined (NEW_CODE) */
		for (pane_no=0;pane_no<graphics_window->number_of_panes;pane_no++)
		{
			Scene_viewer_redraw_now(graphics_window->scene_viewer[pane_no]);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_update_now.  Missing graphics window");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_window_update_now */

int Graphics_window_update_now_iterator(struct Graphics_window *graphics_window,
	void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 6 October 1998

DESCRIPTION :
Iterator function for forcing a redraw on <graphics_window>.
In future want a flag in the graphics window which if on delays the redraw
until an explicit "gfx update" is entered.
==============================================================================*/
{
	int return_code,pane_no;

	ENTER(Graphics_window_update_now_iterator);
	USE_PARAMETER(dummy_void);
	if (graphics_window)
	{
		for (pane_no=0;pane_no<graphics_window->number_of_panes;pane_no++)
		{
			Scene_viewer_redraw_now(graphics_window->scene_viewer[pane_no]);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_update_now_iterator.  Missing graphics window");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_window_update_now_iterator */

int Graphics_window_update_now_without_swapbuffers(
	struct Graphics_window *graphics_window)
/*******************************************************************************
LAST MODIFIED : 6 October 1998

DESCRIPTION :
Forces a redraw on <graphics_window>.  Allows the window to be updated but kept
in the backbuffer so that utility functions such as the movie extensions can get
the pixels out of the backbuffer before the frames are swapped.
==============================================================================*/
{
	int return_code,pane_no;

	ENTER(Graphics_window_update_now_without_swapbuffers);
	if (graphics_window)
	{
		for (pane_no=0;pane_no<graphics_window->number_of_panes;pane_no++)
		{
			Scene_viewer_redraw_now_without_swapbuffers
				(graphics_window->scene_viewer[pane_no]);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_update_now_without_swapbuffers.  "
			"Missing graphics window");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_window_update_now_without_swapbuffers */

static int Graphics_window_read_pixels(char *frame_data, int width, int height,
	enum Texture_storage_type storage)
/*******************************************************************************
LAST MODIFIED : 23 July 1999

DESCRIPTION :
Read pixels into <frame_data> of size <width> and <height> according to the 
storage type.
==============================================================================*/
{
#if defined (OPENGL_API)
	GLenum read_buffer;
#endif /* defined (OPENGL_API) */
	int return_code;

	ENTER(Graphics_window_read_pixels);
	if (frame_data && width && height)
	{
#if defined (OPENGL_API)
		printf("Graphics_window_read_pixels:  Reading width %d, height %d\n", width, height);
		/* Make sure we get it from the front for a double buffer,
			has no effect on a single buffer, keep the old read
			buffer so we can set it back after reading */
		glGetIntegerv(GL_READ_BUFFER,(GLint *)(&read_buffer));
		glReadBuffer(GL_FRONT);
		switch(storage)
		{
			case TEXTURE_ABGR:
			{
				glReadPixels(0, 0, width, height, GL_ABGR_EXT,
					GL_UNSIGNED_BYTE,frame_data);
				return_code=1;
			} break;
			case TEXTURE_RGBA:
			{
				glReadPixels(0, 0, width, height, GL_RGBA,
					GL_UNSIGNED_BYTE,frame_data);
				return_code=1;
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Graphics_window_read_pixels.  Unsupported or unknown storage type");
				return_code=0;
			} break;
		}
		glReadBuffer(read_buffer);
#else /* defined (OPENGL_API) */
		return_code=0;
#endif /* defined (OPENGL_API) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_read_pixels.  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_window_read_pixels */

int Graphics_window_get_frame_pixels(struct Graphics_window *window,
	enum Texture_storage_type storage, int *width, int *height, char **frame_data,
	int force_onscreen)
/*******************************************************************************
LAST MODIFIED : 10 July 2000

DESCRIPTION :
Returns the contents of the graphics window as pixels.  <width> and <height>
will be respected if the window is drawn offscreen and they are non zero,
otherwise they are set in accordance with current size of the graphics window.
If <force_onscreen> is non zero then the pixels will always be grabbed from the
graphics window on screen.
==============================================================================*/
{
	int components, return_code;
	struct Dm_buffer *dmbuffer;

	ENTER(Graphics_window_get_frame_pixels);
	if (window && width && height)
	{
		if ((!*width) || (!*height))
		{
			/* Only use the window size if either dimension is zero */
			Graphics_window_get_viewing_area_size(window, width, height);
		}
		/* Update the window or offscreen area if possible */
		if ((!force_onscreen) && (dmbuffer = CREATE(Dm_buffer)(*width, *height, 
			/*depth_buffer_flag*/1, /*shared_display_buffer_flag*/1,
			window->user_interface)))
		{
			Dm_buffer_glx_make_current(dmbuffer);
			switch (window->layout_mode)
			{
				case GRAPHICS_WINDOW_LAYOUT_SIMPLE:
				case GRAPHICS_WINDOW_LAYOUT_2D:
				{
					/* Only one pane */
					Scene_viewer_render_scene_in_viewport(
						Graphics_window_get_Scene_viewer(window,/*pane_no*/0),
						/*left*/0, /*bottom*/0, /*right*/*width, /*top*/*height);
					return_code=1;
				} break;
				case GRAPHICS_WINDOW_LAYOUT_ORTHOGRAPHIC:
				case GRAPHICS_WINDOW_LAYOUT_FREE_ORTHO:
				{
#if defined (OPENGL_API)
					glClearColor(0.0,0.0,0.0,0.);
					glClear(GL_COLOR_BUFFER_BIT);
#endif /* defined (OPENGL_API) */
					/* Four panes */
					Scene_viewer_render_scene_in_viewport(
						Graphics_window_get_Scene_viewer(window,/*pane_no*/0),
						/*left*/0, /*bottom*/(*height / 2 + 1),
						/*right*/(*width / 2 - 1), /*top*/*height);
					Scene_viewer_render_scene_in_viewport(
						Graphics_window_get_Scene_viewer(window,/*pane_no*/1),
						/*left*/(*width / 2 + 1), /*bottom*/(*height / 2 + 1),
						/*right*/*width, /*top*/*height);
					Scene_viewer_render_scene_in_viewport(
						Graphics_window_get_Scene_viewer(window,/*pane_no*/2),
						/*left*/0, /*bottom*/0, /*right*/(*width / 2 - 1), 
						/*top*/(*height / 2 - 1));
					Scene_viewer_render_scene_in_viewport(
						Graphics_window_get_Scene_viewer(window,/*pane_no*/3),
						/*left*/(*width / 2 + 1), /*bottom*/0, /*right*/*width, 
						/*top*/(*height / 2 - 1));
					return_code=1;
				} break;
				case GRAPHICS_WINDOW_LAYOUT_FRONT_BACK:
				case GRAPHICS_WINDOW_LAYOUT_FRONT_SIDE:
				case GRAPHICS_WINDOW_LAYOUT_PSEUDO_3D:
				{
#if defined (OPENGL_API)
					glClearColor(0.0,0.0,0.0,0.);
					glClear(GL_COLOR_BUFFER_BIT);
#endif /* defined (OPENGL_API) */
					/* Two panes, side by side */
					Scene_viewer_render_scene_in_viewport(
						Graphics_window_get_Scene_viewer(window,/*pane_no*/0),
						/*left*/0, /*bottom*/0, /*right*/(*width / 2 - 1), /*top*/*height);
					Scene_viewer_render_scene_in_viewport(
						Graphics_window_get_Scene_viewer(window,/*pane_no*/1),
						/*left*/(*width / 2 + 1), /*bottom*/0,
						/*right*/*width, /*top*/*height);
					return_code=1;
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"Graphics_window_get_frame_pixels.  Unknown layout_mode");
					return_code=0;
				} break;
			}
			if (return_code)
			{
				components=Texture_get_number_of_components_from_storage_type(storage);
				if (ALLOCATE(*frame_data, char, components * *width * *height))
				{
					if (!(return_code=Graphics_window_read_pixels(*frame_data, *width,
						*height, storage)))
					{
						DEALLOCATE(*frame_data);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Graphics_window_get_frame_pixels.  Unable to allocate pixels");
					return_code=0;
				}
			}
			DESTROY(Dm_buffer)(&dmbuffer);
		}
		else
		{
			/* bring the graphics window to the front so image is not obscured */
			XRaiseWindow(XtDisplay(window->window_shell),
				XtWindow(window->window_shell));
			/* Always use the window size if grabbing from screen */
			Graphics_window_get_viewing_area_size(window, width, height);
			Graphics_window_update_now(window);
			components = Texture_get_number_of_components_from_storage_type(storage);
			if (ALLOCATE(*frame_data, char, components * *width * *height))
			{
				switch (window->layout_mode)
				{
					case GRAPHICS_WINDOW_LAYOUT_SIMPLE:
					case GRAPHICS_WINDOW_LAYOUT_2D:
					{
						/* Only one pane */
						if (!(return_code=Graphics_window_read_pixels(*frame_data, *width,
							*height, storage)))
						{
							DEALLOCATE(*frame_data);
						}
					} break;
					case GRAPHICS_WINDOW_LAYOUT_ORTHOGRAPHIC:
					case GRAPHICS_WINDOW_LAYOUT_FREE_ORTHO:
					{
						/* Four panes */
						display_message(ERROR_MESSAGE,"Graphics_window_get_frame_pixels.  "
							"Can only grab single pane windows onscreen currently");
						return_code=1;
					} break;
					case GRAPHICS_WINDOW_LAYOUT_FRONT_BACK:
					case GRAPHICS_WINDOW_LAYOUT_FRONT_SIDE:
					case GRAPHICS_WINDOW_LAYOUT_PSEUDO_3D:
					{
						/* Two panes, side by side */
						display_message(ERROR_MESSAGE,"Graphics_window_get_frame_pixels.  "
							"Can only grab single pane windows onscreen currently");
						return_code=1;
					} break;
					default:
					{
						display_message(ERROR_MESSAGE,
							"Graphics_window_get_frame_pixels.  Unknown layout_mode");
						DEALLOCATE(*frame_data);
						return_code=0;
					} break;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Graphics_window_get_frame_pixels.  Unable to allocate pixels");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_get_frame_pixels.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return return_code;
} /* Graphics_window_get_frame_pixels */

int Graphics_window_view_all(struct Graphics_window *window)
/*******************************************************************************
LAST MODIFIED : 16 October 2001

DESCRIPTION :
Finds the x, y and z ranges from the scene and sets the view parameters so
that everything can be seen, and with window's std_view_angle. Also adjusts
near and far clipping planes; if specific values are required, should follow
with commands for setting these.
==============================================================================*/
{
	double centre_x,centre_y,centre_z,clip_factor,radius,
		size_x,size_y,size_z,width_factor;
	int pane_no,return_code;
	double left, right, bottom, top, near, far;

	ENTER(Graphics_window_view_all);
	if (window)
	{
		return_code = 1;

		Scene_get_graphics_range(window->scene,
			&centre_x, &centre_y, &centre_z, &size_x, &size_y, &size_z);
		radius = 0.5*sqrt(size_x*size_x + size_y*size_y + size_z*size_z);
		if (0 == radius)
		{
			/* get current "radius" from first scene viewer */
			Scene_viewer_get_viewing_volume(window->scene_viewer[0],
				&left, &right, &bottom, &top, &near, &far);
			radius = 0.5*(right - left);
		}
		else
		{
			/*???RC width_factor should be read in from defaults file */
			width_factor = 1.05;
			/* enlarge radius to keep image within edge of window */
			radius *= width_factor;
		}

		/*???RC clip_factor should be read in from defaults file: */
		clip_factor = 10.0;
		for (pane_no = 0; (pane_no < GRAPHICS_WINDOW_MAX_NUMBER_OF_PANES) &&
			return_code; pane_no++)
		{
			return_code = Scene_viewer_set_view_simple(
				window->scene_viewer[pane_no], centre_x, centre_y, centre_z,
				radius, window->std_view_angle, clip_factor*radius);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_view_all.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Graphics_window_view_all */

int Graphics_window_view_changed(struct Graphics_window *window,
	int changed_pane)
/*******************************************************************************
LAST MODIFIED : 29 September 2000

DESCRIPTION :
Call this function whenever the view in a pane has changed. Depending on the
current layout_mode, the function adjusts the view in all the panes tied to
<changed_pane> to maintain the relationship expected for it.
==============================================================================*/
{
	double angle,bottom,eye[3],eye_distance,extra_axis[3],
		far[GRAPHICS_WINDOW_MAX_NUMBER_OF_PANES],left,lookat[3],
		near[GRAPHICS_WINDOW_MAX_NUMBER_OF_PANES],right,top,up[3],view[3],
		view_left[3];
	enum Scene_viewer_projection_mode projection_mode;
	int pane_no,return_code;

	ENTER(Graphics_window_view_changed);
	if (window&&(0<=changed_pane)&&(changed_pane<window->number_of_panes))
	{
		/* 1. get lookat parameters and viewing volume of changed_pane. Also need
			 separate near and far clipping planes for each pane_no */
		for (pane_no=0;pane_no<window->number_of_panes;pane_no++)
		{
			if (pane_no != changed_pane)
			{
				Scene_viewer_stop_animations(window->scene_viewer[pane_no]);
			}
			Scene_viewer_get_viewing_volume(window->scene_viewer[pane_no],
				&left,&right,&bottom,&top,&(near[pane_no]),&(far[pane_no]));
		}
		if (return_code=(Scene_viewer_get_lookat_parameters(
			window->scene_viewer[changed_pane],&(eye[0]),&(eye[1]),&(eye[2]),
			&(lookat[0]),&(lookat[1]),&(lookat[2]),&(up[0]),&(up[1]),&(up[2]))&&
			Scene_viewer_get_viewing_volume(window->scene_viewer[changed_pane],
				&left,&right,&bottom,&top,&(near[changed_pane]),&(far[changed_pane]))))
		{
			projection_mode=Graphics_window_get_projection_mode(window,changed_pane);
			/* get orthogonal view, up and left directions in changed_pane */
			view[0]=eye[0]-lookat[0];
			view[1]=eye[1]-lookat[1];
			view[2]=eye[2]-lookat[2];
			eye_distance=normalize3(view);
			cross_product3(view,up,view_left);
			normalize3(view_left);
			cross_product3(view_left,view,up);
			switch (window->layout_mode)
			{
				case GRAPHICS_WINDOW_LAYOUT_SIMPLE:
				case GRAPHICS_WINDOW_LAYOUT_2D:
				{
					/* nothing to do: only one pane */
				} break;
				case GRAPHICS_WINDOW_LAYOUT_ORTHOGRAPHIC:
				{
					/* panes 2, 3 and 4 tied in third angle orthographic projection. Pane
						 0 shows the iso_metric view in the octant of the other views */
					/* first give each tied scene viewer the same lookat parameters and
						 viewing volume (except for near and far) */
					for (pane_no=0;pane_no<4;pane_no++)
					{
						Scene_viewer_set_lookat_parameters(window->scene_viewer[pane_no],
							eye[0],eye[1],eye[2],lookat[0],lookat[1],lookat[2],
							up[0],up[1],up[2]);
						Scene_viewer_set_viewing_volume(window->scene_viewer[pane_no],
							left,right,bottom,top,near[pane_no],far[pane_no]);
					}
					/* now rotate all tied panes but the changed_pane to get the proper
						 relationship between them */
					switch (changed_pane)
					{
						case 0:
						{
							/* extra_axis = front,up */
							extra_axis[0]=view[0]+up[0];
							extra_axis[1]=view[1]+up[1];
							extra_axis[2]=view[2]+up[2];
							normalize3(extra_axis);
							Scene_viewer_rotate_about_lookat_point(window->scene_viewer[1],
								view_left,0.25*PI);
							Scene_viewer_rotate_about_lookat_point(window->scene_viewer[1],
								extra_axis,0.25*PI);
							Scene_viewer_rotate_about_lookat_point(window->scene_viewer[2],
								view_left,-0.25*PI);
							Scene_viewer_rotate_about_lookat_point(window->scene_viewer[2],
								extra_axis,-0.25*PI);
							Scene_viewer_rotate_about_lookat_point(window->scene_viewer[3],
								view_left,-0.25*PI);
							Scene_viewer_rotate_about_lookat_point(window->scene_viewer[3],
								extra_axis,0.25*PI);
							Scene_viewer_redraw_now(window->scene_viewer[1]);
							Scene_viewer_redraw_now(window->scene_viewer[2]);
							Scene_viewer_redraw_now(window->scene_viewer[3]);
						} break;
						case 1:
						{
							Scene_viewer_rotate_about_lookat_point(window->scene_viewer[0],
								view_left,-0.25*PI);
							Scene_viewer_rotate_about_lookat_point(window->scene_viewer[0],
								view,-0.25*PI);
							Scene_viewer_rotate_about_lookat_point(window->scene_viewer[3],
								view_left,-0.5*PI);
							Scene_viewer_rotate_about_lookat_point(window->scene_viewer[2],
								view_left,-0.5*PI);
							Scene_viewer_rotate_about_lookat_point(window->scene_viewer[2],
								view,-0.5*PI);
							Scene_viewer_redraw_now(window->scene_viewer[0]);
							Scene_viewer_redraw_now(window->scene_viewer[2]);
							Scene_viewer_redraw_now(window->scene_viewer[3]);
						} break;
						case 2:
						{
							/* extra_axis = left, front */
							extra_axis[0]=view_left[0]+view[0];
							extra_axis[1]=view_left[1]+view[1];
							extra_axis[2]=view_left[2]+view[2];
							normalize3(extra_axis);
							Scene_viewer_rotate_about_lookat_point(window->scene_viewer[0],
								up,0.25*PI);
							Scene_viewer_rotate_about_lookat_point(window->scene_viewer[0],
								extra_axis,0.25*PI);
							Scene_viewer_rotate_about_lookat_point(window->scene_viewer[3],
								up,0.5*PI);
							Scene_viewer_rotate_about_lookat_point(window->scene_viewer[1],
								up,0.5*PI);
							Scene_viewer_rotate_about_lookat_point(window->scene_viewer[1],
								view,0.5*PI);
							Scene_viewer_redraw_now(window->scene_viewer[0]);
							Scene_viewer_redraw_now(window->scene_viewer[1]);
							Scene_viewer_redraw_now(window->scene_viewer[3]);
						} break;
						case 3:
						{
							/* extra_axis = left, rear */
							extra_axis[0]=view_left[0]-view[0];
							extra_axis[1]=view_left[1]-view[1];
							extra_axis[2]=view_left[2]-view[2];
							normalize3(extra_axis);
							Scene_viewer_rotate_about_lookat_point(window->scene_viewer[0],
								up,-0.25*PI);
							Scene_viewer_rotate_about_lookat_point(window->scene_viewer[0],
								extra_axis,0.25*PI);
							Scene_viewer_rotate_about_lookat_point(window->scene_viewer[1],
								view_left,0.5*PI);
							Scene_viewer_rotate_about_lookat_point(window->scene_viewer[2],
								up,-0.5*PI);
							Scene_viewer_redraw_now(window->scene_viewer[0]);
							Scene_viewer_redraw_now(window->scene_viewer[1]);
							Scene_viewer_redraw_now(window->scene_viewer[2]);
						} break;
					}
				} break;
				case GRAPHICS_WINDOW_LAYOUT_FREE_ORTHO:
				{
					/* pane 0 is not tied at present */
					if (0 != changed_pane)
					{
					/* panes 2, 3 and 4 tied in third angle orthographic projection */
					/* first give each tied scene viewer the same lookat parameters and
						 viewing volume (except for near and far) */
					for (pane_no=1;pane_no<4;pane_no++)
					{
						Scene_viewer_set_lookat_parameters(window->scene_viewer[pane_no],
							eye[0],eye[1],eye[2],lookat[0],lookat[1],lookat[2],
							up[0],up[1],up[2]);
						Scene_viewer_set_viewing_volume(window->scene_viewer[pane_no],
							left,right,bottom,top,near[pane_no],far[pane_no]);
					}
					/* now rotate all tied panes but the changed_pane to get the proper
						 relationship between them */
					switch (changed_pane)
					{
						case 1:
						{
							Scene_viewer_rotate_about_lookat_point(window->scene_viewer[3],
								view_left,-0.5*PI);
							Scene_viewer_rotate_about_lookat_point(window->scene_viewer[2],
								view_left,-0.5*PI);
							Scene_viewer_rotate_about_lookat_point(window->scene_viewer[2],
								view,-0.5*PI);
							Scene_viewer_redraw_now(window->scene_viewer[2]);
							Scene_viewer_redraw_now(window->scene_viewer[3]);
						} break;
						case 2:
						{
							Scene_viewer_rotate_about_lookat_point(window->scene_viewer[3],
								up,0.5*PI);
							Scene_viewer_rotate_about_lookat_point(window->scene_viewer[1],
								up,0.5*PI);
							Scene_viewer_rotate_about_lookat_point(window->scene_viewer[1],
								view,0.5*PI);
							Scene_viewer_redraw_now(window->scene_viewer[1]);
							Scene_viewer_redraw_now(window->scene_viewer[3]);
						} break;
						case 3:
						{
							Scene_viewer_rotate_about_lookat_point(window->scene_viewer[1],
								view_left,0.5*PI);
							Scene_viewer_rotate_about_lookat_point(window->scene_viewer[2],
								up,-0.5*PI);
							Scene_viewer_redraw_now(window->scene_viewer[1]);
							Scene_viewer_redraw_now(window->scene_viewer[2]);
						} break;
					}
					}
				} break;
				case GRAPHICS_WINDOW_LAYOUT_FRONT_BACK:
				case GRAPHICS_WINDOW_LAYOUT_FRONT_SIDE:
				{
					if (GRAPHICS_WINDOW_LAYOUT_FRONT_BACK==window->layout_mode)
					{
						/* panes 0 and 1 tied as front and back */
						angle=PI;
					}
					else
					{
						/* panes 0 and 1 tied as front and side (third angle) */
						angle=PI/2.0;
					}
					/* first give each tied scene viewer the same lookat parameters,
						 viewing volume (except for near and far) and projection_mode */
					for (pane_no=0;pane_no<window->number_of_panes;pane_no++)
					{
						Graphics_window_set_projection_mode(window,pane_no,projection_mode);
						Scene_viewer_set_lookat_parameters(window->scene_viewer[pane_no],
							eye[0],eye[1],eye[2],lookat[0],lookat[1],lookat[2],
							up[0],up[1],up[2]);
						Scene_viewer_set_viewing_volume(window->scene_viewer[pane_no],
							left,right,bottom,top,near[pane_no],far[pane_no]);
					}
					/* now rotate all tied panes but the changed_pane to get the proper
						 relationship between them */
					switch (changed_pane)
					{
						case 0:
						{
							Scene_viewer_rotate_about_lookat_point(window->scene_viewer[1],
								up,angle);
							Scene_viewer_redraw_now(window->scene_viewer[1]);
						} break;
						case 1:
						{
							Scene_viewer_rotate_about_lookat_point(window->scene_viewer[0],
								up,-angle);
							Scene_viewer_redraw_now(window->scene_viewer[0]);
						} break;
					}
				} break;
				case GRAPHICS_WINDOW_LAYOUT_PSEUDO_3D:
				{
					if (0.0<eye_distance)
					{
						/* panes 0 and 1 tied as right and left eye views, respectively */
						/* first give each tied scene viewer the same lookat parameters,
							 viewing volume (except for near and far) and projection_mode */
						for (pane_no=0;pane_no<window->number_of_panes;pane_no++)
						{
							Graphics_window_set_projection_mode(window,pane_no,
								projection_mode);
							Scene_viewer_set_lookat_parameters(window->scene_viewer[pane_no],
								eye[0],eye[1],eye[2],lookat[0],lookat[1],lookat[2],
								up[0],up[1],up[2]);
							Scene_viewer_set_viewing_volume(window->scene_viewer[pane_no],
								left,right,bottom,top,near[pane_no],far[pane_no]);
						}
						/* now rotate all tied panes but the changed_pane to get the proper
						 relationship between them */
						/* 2 panes show views from eyes separated by eye_spacing, and
							 looking at the same lookat point. Hence, can calculate the angle
							 difference between the views using the eye_distance. */
						angle=2.0*atan(0.5*window->eye_spacing/eye_distance);
						switch (changed_pane)
						{
							case 0:
							{
								Scene_viewer_rotate_about_lookat_point(window->scene_viewer[1],
									up,-angle);
								Scene_viewer_redraw_now(window->scene_viewer[1]);
							} break;
							case 1:
							{
								Scene_viewer_rotate_about_lookat_point(window->scene_viewer[0],
									up,angle);
								Scene_viewer_redraw_now(window->scene_viewer[0]);
							} break;
						}
					}
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
					"Graphics_window_view_changed.  Unknown layout_mode");
					return_code=0;
				} break;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Graphics_window_view_changed.  Invalid scene_viewer");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_view_changed.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return return_code;
} /* Graphics_window_view_changed */

int view_and_up_vectors_to_xyz_rotations(double *view, double *up,
	double *rotations)
/*******************************************************************************
LAST MODIFIED : 6 April 2001

DESCRIPTION :
Deduces three <rotations> about the global x, global y and global z axes that
rotate the initial view (looking in the -z direction with y up) to look in the
<view> direction upright in the <up>-direction. The final up-vector will be
orthogonal with the view direction even if not initially; <up> may not be
co-linear with <view>.
Rotations are returned in radians. Note the results are non-unique.
These rotations are used in the MAYA viewing model.
==============================================================================*/
{
  int return_code;
	double c, m[9], norm_right[3], norm_view[3], ry[9], rz[9], s, x_view[3];

	ENTER(view_and_up_vectors_to_xyz_rotations);
	if (view && up && rotations)
	{
		norm_view[0] = view[0];
		norm_view[1] = view[1];
		norm_view[2] = view[2];
		/* establish right vector at view x up and normalise it and view */
		if ((0.0 != normalize3(norm_view)) &&
			cross_product3(norm_view, up, norm_right) &&
			(0.0 != normalize3(norm_right)))
		{
			/* Calculate the z and y rotations */
			rotations[2] = atan2(norm_right[1], norm_right[0]);
			rotations[1] = atan2(-norm_right[2],
				sqrt(norm_right[0]*norm_right[0] + norm_right[1]*norm_right[1]));

			/* create matrix ry that undoes the y rotation */
			identity_matrix(3, ry);
			s = sin(-rotations[1]);
			c = cos(-rotations[1]);
			ry[0] = c;
			ry[2] = s;
			ry[6] = -s;
			ry[8] = c;
			/* create matrix rz that undoes the z rotation */
			identity_matrix(3, rz);
			s = sin(-rotations[2]);
			c = cos(-rotations[2]);
			rz[0] = c;
			rz[1] = -s;
			rz[3] = s;
			rz[4] = c;
			/* multiply to get matrix m that undoes the z and y rotations */
			multiply_matrix(3, 3, 3, ry, rz, m);
			/* multiply norm_view by m to get initial view rotated just by x angle */
			multiply_matrix(3, 3, 1, m, norm_view, x_view);

			/* calculate the x-rotation */
			rotations[0] = atan2(x_view[1], -x_view[2]);

			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"view_and_up_vectors_to_xyz_rotations.  "
				"View and/or up vectors zero or not orthogonal");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"view_and_up_vectors_to_xyz_rotations.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* view_and_up_vectors_to_xyz_rotations */

int list_Graphics_window(struct Graphics_window *window,void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 6 April 2001

DESCRIPTION :
Writes the properties of the <window> to the command window.
==============================================================================*/
{
	char line[80],*name;
	double bottom, eye[3], far, horizontal_view_angle, left, lookat[3],
		max_pixels_per_polygon, modelview_matrix[16],
		NDC_height, NDC_left, NDC_top, NDC_width,
		near, projection_matrix[16], right, rotations[3],
		texture_height, texture_width, top,
		up[3], view[3], view_angle, viewport_left, viewport_top,
		viewport_pixels_per_unit_x, viewport_pixels_per_unit_y;
	enum Scene_viewer_buffer_mode buffer_mode;
	enum Scene_viewer_projection_mode projection_mode;
	enum Scene_viewer_transparency_mode transparency_mode;
	enum Scene_viewer_viewport_mode viewport_mode;
	int antialias,height,pane_no,perturb_lines,return_code,width,undistort_on;
	struct Colour colour;
	struct Scene *overlay_scene;
	struct Scene_viewer *scene_viewer;
	struct Texture *texture;

	ENTER(list_Graphics_window);
	USE_PARAMETER(dummy_void);
	if (window)
	{
		display_message(INFORMATION_MESSAGE,"Graphics window : %s\n",window->name);
		if (buffer_mode=Scene_viewer_get_buffer_mode(window->scene_viewer[0]))
		{
			display_message(INFORMATION_MESSAGE,"  %s\n",
				Scene_viewer_buffer_mode_string(buffer_mode));
		}
		/* image */
		if (GET_NAME(Scene)(window->scene,&name))
		{
			display_message(INFORMATION_MESSAGE,"  scene: %s\n",name);
			DEALLOCATE(name);
		}
		if (GET_NAME(Light_model)(
			Scene_viewer_get_light_model(window->scene_viewer[0]),&name))
		{
			display_message(INFORMATION_MESSAGE,"  light model: %s\n",name);
			DEALLOCATE(name);
		}
		display_message(INFORMATION_MESSAGE,"  lights:\n");
		for_each_Light_in_Scene_viewer(window->scene_viewer[0],list_Light_name,
			(void *)"    ");
		/* layout */
		display_message(INFORMATION_MESSAGE,"  layout: %s\n",
			Graphics_window_layout_mode_string(window->layout_mode));
		display_message(INFORMATION_MESSAGE,
			"  orthographic up and front axes: %s %s\n",
			axis_name[window->ortho_up_axis],axis_name[window->ortho_front_axis]);
		display_message(INFORMATION_MESSAGE,
			"  eye_spacing: %g\n",window->eye_spacing);
		Graphics_window_get_viewing_area_size(window,&width,&height);
		display_message(INFORMATION_MESSAGE,"  viewing width x height: %d x %d\n",
			width,height);
		/* background and view in each active pane */
		for (pane_no=0;pane_no<window->number_of_panes;pane_no++)
		{
			scene_viewer=window->scene_viewer[pane_no];
			display_message(INFORMATION_MESSAGE,"  pane: %d\n",pane_no+1);
			/* background */
			Scene_viewer_get_background_colour(scene_viewer,&colour);
			display_message(INFORMATION_MESSAGE,
				"    background colour R G B: %g %g %g\n",
				colour.red,colour.green,colour.blue);
			if ((texture=Scene_viewer_get_background_texture(scene_viewer))&&
				Scene_viewer_get_background_texture_info(scene_viewer,
					&left,&top,&texture_width,&texture_height,&undistort_on,
					&max_pixels_per_polygon)&&
				GET_NAME(Texture)(texture,&name))
			{
				display_message(INFORMATION_MESSAGE,
					"    background texture %s\n",name);
				display_message(INFORMATION_MESSAGE,
					"      (placement: left=%g top=%g width=%g height=%g\n",
					left,top,texture_width,texture_height);
				display_message(INFORMATION_MESSAGE,"      (undistort: ");
				if (undistort_on)
				{
					display_message(INFORMATION_MESSAGE,"on");
				}
				else
				{
					display_message(INFORMATION_MESSAGE,"off");
				}
				display_message(INFORMATION_MESSAGE,
					", max pixels/polygon=%g)\n",max_pixels_per_polygon);
				DEALLOCATE(name);
			}
			/* view */
			projection_mode=Scene_viewer_get_projection_mode(scene_viewer);
			display_message(INFORMATION_MESSAGE,"    projection: %s\n",
				Scene_viewer_projection_mode_string(projection_mode));
			if (SCENE_VIEWER_CUSTOM==projection_mode)
			{
				Scene_viewer_get_modelview_matrix(scene_viewer,modelview_matrix);
				Scene_viewer_get_projection_matrix(scene_viewer,projection_matrix);
				display_message(INFORMATION_MESSAGE,
					"    modelview matrix  = | %13.6e %13.6e %13.6e %13.6e |\n",
					modelview_matrix[0],modelview_matrix[1],
					modelview_matrix[2],modelview_matrix[3]);
				display_message(INFORMATION_MESSAGE,
					"                      = | %13.6e %13.6e %13.6e %13.6e |\n",
					modelview_matrix[4],modelview_matrix[5],
					modelview_matrix[6],modelview_matrix[7]);
				display_message(INFORMATION_MESSAGE,
					"                      = | %13.6e %13.6e %13.6e %13.6e |\n",
					modelview_matrix[8],modelview_matrix[9],
					modelview_matrix[10],modelview_matrix[11]);
				display_message(INFORMATION_MESSAGE,
					"                      = | %13.6e %13.6e %13.6e %13.6e |\n",
					modelview_matrix[12],modelview_matrix[13],
					modelview_matrix[14],modelview_matrix[15]);
				display_message(INFORMATION_MESSAGE,
					"    projection matrix = | %13.6e %13.6e %13.6e %13.6e |\n",
					projection_matrix[0],projection_matrix[1],
					projection_matrix[2],projection_matrix[3]);
				display_message(INFORMATION_MESSAGE,
					"                      = | %13.6e %13.6e %13.6e %13.6e |\n",
					projection_matrix[4],projection_matrix[5],
					projection_matrix[6],projection_matrix[7]);
				display_message(INFORMATION_MESSAGE,
					"                      = | %13.6e %13.6e %13.6e %13.6e |\n",
					projection_matrix[8],projection_matrix[9],
					projection_matrix[10],projection_matrix[11]);
				display_message(INFORMATION_MESSAGE,
					"                      = | %13.6e %13.6e %13.6e %13.6e |\n",
					projection_matrix[12],projection_matrix[13],
					projection_matrix[14],projection_matrix[15]);
			}
			else
			{
				/* parallel/perspective: write eye and interest points and up-vector */
				Scene_viewer_get_lookat_parameters(scene_viewer,
					&(eye[0]),&(eye[1]),&(eye[2]),&(lookat[0]),&(lookat[1]),&(lookat[2]),
					&(up[0]),&(up[1]),&(up[2]));
				view[0] = lookat[0] - eye[0];
				view[1] = lookat[1] - eye[1];
				view[2] = lookat[2] - eye[2];
				Scene_viewer_get_viewing_volume(scene_viewer,&left,&right,
					&bottom,&top,&near,&far);
				Scene_viewer_get_view_angle(scene_viewer,&view_angle);
				Scene_viewer_get_horizontal_view_angle(scene_viewer,
					&horizontal_view_angle);
				display_message(INFORMATION_MESSAGE,
					"    eye point: %g %g %g\n",eye[0],eye[1],eye[2]);
				display_message(INFORMATION_MESSAGE,
					"    interest point: %g %g %g\n",lookat[0],lookat[1],lookat[2]);
				display_message(INFORMATION_MESSAGE,
					"    up vector: %g %g %g\n",up[0],up[1],up[2]);
				display_message(INFORMATION_MESSAGE,
					"    view angle (across NDC, degrees) diagonal, horizontal : %g %g\n",
					view_angle*(180/PI), horizontal_view_angle*(180/PI));

				display_message(INFORMATION_MESSAGE,
					"    near and far clipping planes: %g %g\n",near,far);
				/* work out if view is skew = up not orthogonal to view direction */
				eye[0] -= lookat[0];
				eye[1] -= lookat[1];
				eye[2] -= lookat[2];
				normalize3(eye);
				normalize3(up);
				if (0.00001<fabs(dot_product3(up,eye)))
				{
					display_message(INFORMATION_MESSAGE,
						"    view is skew (up-vector not orthogonal to view direction)\n");
				}
				else
				{
					if (view_and_up_vectors_to_xyz_rotations(view, up, rotations))
					{
						display_message(INFORMATION_MESSAGE,
							"    view rotations about x, y and z axes (degrees): %g %g %g\n",
							rotations[0]*(180.0/PI), rotations[1]*(180.0/PI),
							rotations[2]*(180.0/PI));
					}
				}
			}
			viewport_mode=Scene_viewer_get_viewport_mode(scene_viewer);
			display_message(INFORMATION_MESSAGE,"    %s\n",
				Scene_viewer_viewport_mode_string(viewport_mode));
			Scene_viewer_get_NDC_info(scene_viewer,
				&NDC_left,&NDC_top,&NDC_width,&NDC_height);
			display_message(INFORMATION_MESSAGE,
				"    NDC placement: left=%g top=%g width=%g height=%g\n",
				NDC_left,NDC_top,NDC_width,NDC_height);
			Scene_viewer_get_viewport_info(scene_viewer,
				&viewport_left,&viewport_top,&viewport_pixels_per_unit_x,
				&viewport_pixels_per_unit_y);
			display_message(INFORMATION_MESSAGE,
				"    Viewport coordinates: left=%g top=%g pixels/unit x=%g y=%g\n",
				viewport_left,viewport_top,viewport_pixels_per_unit_x,
				viewport_pixels_per_unit_y);
			/* overlay */
			display_message(INFORMATION_MESSAGE,"    overlay scene: ");
			if (overlay_scene=Scene_viewer_get_overlay_scene(scene_viewer))
			{
				if (GET_NAME(Scene)(overlay_scene,&name))
				{
					display_message(INFORMATION_MESSAGE,"%s\n",name);
					DEALLOCATE(name);
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE,"none\n");
			}
		}
		/* settings */
		if (GET_NAME(Interactive_tool)(window->interactive_tool,&name))
		{
			display_message(INFORMATION_MESSAGE,"  Interactive tool: %s\n",name);
			DEALLOCATE(name);
		}
		transparency_mode = Scene_viewer_get_transparency_mode(scene_viewer);
		display_message(INFORMATION_MESSAGE,
			"  Transparency mode: %s\n",Scene_viewer_transparency_mode_string(
				transparency_mode));
		if (transparency_mode == SCENE_VIEWER_LAYERED_TRANSPARENCY)
		{
			display_message(INFORMATION_MESSAGE,"    transparency_layers: %d\n", 
				Scene_viewer_get_transparency_layers(scene_viewer));
		}
		display_message(INFORMATION_MESSAGE,
			"  Current pane: %d\n",window->current_pane+1);
		display_message(INFORMATION_MESSAGE,
			"  Standard view angle: %g degrees\n",window->std_view_angle);
		Scene_viewer_get_perturb_lines(window->scene_viewer[0],&perturb_lines);
		if (perturb_lines)
		{
			display_message(INFORMATION_MESSAGE,"  perturbed lines: on\n");
		}
		else
		{
			display_message(INFORMATION_MESSAGE,"  perturbed lines: off\n");
		}
		Scene_viewer_get_antialias_mode(window->scene_viewer[0],&antialias);
		if (antialias)
		{
			display_message(INFORMATION_MESSAGE,"  anti-aliasing at %d\n",antialias);
		}
		else
		{
			display_message(INFORMATION_MESSAGE,"  no anti-aliasing\n");
		}
		sprintf(line,"  access count = %i\n",window->access_count);
		display_message(INFORMATION_MESSAGE,line);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Graphics_window.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* list_Graphics_window */

int list_Graphics_window_commands(struct Graphics_window *window,
	void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 22 January 2002

DESCRIPTION :
Writes the commands for creating the <window> and establishing the views in it
to the command window.
==============================================================================*/
{
	char *name,*prefix;
	double bottom,
		eye[3],far,left,lookat[3],max_pixels_per_polygon,modelview_matrix[16],
		NDC_height,NDC_left,NDC_top,NDC_width,near,projection_matrix[16],right,
		texture_height,texture_width,top,up[3],view_angle,viewport_left,
		viewport_top,viewport_pixels_per_unit_x,viewport_pixels_per_unit_y;
	enum Scene_viewer_buffer_mode buffer_mode;
	enum Scene_viewer_projection_mode projection_mode;
	enum Scene_viewer_transparency_mode transparency_mode;
	enum Scene_viewer_viewport_mode viewport_mode;
	int antialias,height,i,pane_no,perturb_lines,return_code,width,undistort_on;
	struct Colour colour;
	struct Scene *overlay_scene;
	struct Scene_viewer *scene_viewer;
	struct Texture *texture;

	ENTER(list_Graphics_window);
	USE_PARAMETER(dummy_void);
	if (window&&ALLOCATE(prefix,char,50+strlen(window->name)))
	{
		if (name=duplicate_string(window->name))
		{
			/* put quotes around name if it contains special characters */
			make_valid_token(&name);
			display_message(INFORMATION_MESSAGE,"gfx create window %s",name);
			DEALLOCATE(name);
		}
		if (buffer_mode=Scene_viewer_get_buffer_mode(window->scene_viewer[0]))
		{
			display_message(INFORMATION_MESSAGE," %s",
				Scene_viewer_buffer_mode_string(buffer_mode));
		}
		display_message(INFORMATION_MESSAGE,";\n");
		/* image */
		display_message(INFORMATION_MESSAGE,"gfx modify window %s image",
			window->name);
		if (GET_NAME(Scene)(window->scene,&name))
		{
			/* put quotes around name if it contains special characters */
			make_valid_token(&name);
			display_message(INFORMATION_MESSAGE," scene %s",name);
			DEALLOCATE(name);
		}
		if (GET_NAME(Light_model)(
			Scene_viewer_get_light_model(window->scene_viewer[0]),&name))
		{
			/* put quotes around name if it contains special characters */
			make_valid_token(&name);
			display_message(INFORMATION_MESSAGE," light_model %s",name);
			DEALLOCATE(name);
		}
		display_message(INFORMATION_MESSAGE,";\n");
		sprintf(prefix,"gfx modify window %s image add_light ",window->name);
		for_each_Light_in_Scene_viewer(window->scene_viewer[0],
			list_Light_name_command, (void *)prefix);
		/* layout */
		Graphics_window_get_viewing_area_size(window,&width,&height);
		display_message(INFORMATION_MESSAGE,
			"gfx modify window %s layout %s ortho_axes %s %s eye_spacing %g"
			" width %d height %d;\n",window->name,
			Graphics_window_layout_mode_string(window->layout_mode),
			axis_name[window->ortho_up_axis],axis_name[window->ortho_front_axis],
			window->eye_spacing,width,height);
		/* background, view and overlay in each active pane */
		for (pane_no=0;pane_no<window->number_of_panes;pane_no++)
		{
			scene_viewer=window->scene_viewer[pane_no];
			display_message(INFORMATION_MESSAGE,
				"gfx modify window %s set current_pane %d;\n",
				window->name,pane_no+1);
			/* background */
			display_message(INFORMATION_MESSAGE,
				"gfx modify window %s background",window->name);
			Scene_viewer_get_background_colour(scene_viewer,&colour);
			display_message(INFORMATION_MESSAGE,
				" colour %g %g %g",colour.red,colour.green,colour.blue);
			if ((texture=Scene_viewer_get_background_texture(scene_viewer))&&
				Scene_viewer_get_background_texture_info(scene_viewer,
					&left,&top,&texture_width,&texture_height,&undistort_on,
					&max_pixels_per_polygon)&&
				GET_NAME(Texture)(texture,&name))
			{
				/* put quotes around name if it contains special characters */
				make_valid_token(&name);
				display_message(INFORMATION_MESSAGE," texture %s",name);
				display_message(INFORMATION_MESSAGE," tex_placement %g %g %g %g",
					left,top,texture_width,texture_height);
				if (undistort_on)
				{
					display_message(INFORMATION_MESSAGE," undistort_texture");
				}
				else
				{
					display_message(INFORMATION_MESSAGE," no_undistort_texture");
				}
				display_message(INFORMATION_MESSAGE," max_pixels_per_polygon %g",
					max_pixels_per_polygon);
				DEALLOCATE(name);
			}
			else
			{
				display_message(INFORMATION_MESSAGE," texture none");
			}
			display_message(INFORMATION_MESSAGE,";\n");
			/* view */
			display_message(INFORMATION_MESSAGE,
				"gfx modify window %s view",window->name);
			projection_mode=Scene_viewer_get_projection_mode(scene_viewer);
			display_message(INFORMATION_MESSAGE," %s",
				Scene_viewer_projection_mode_string(projection_mode));
			if (SCENE_VIEWER_CUSTOM==projection_mode)
			{
				Scene_viewer_get_modelview_matrix(scene_viewer,modelview_matrix);
				Scene_viewer_get_projection_matrix(scene_viewer,projection_matrix);
				display_message(INFORMATION_MESSAGE," modelview matrix");
				for (i=0;i<16;i++)
				{
					display_message(INFORMATION_MESSAGE," %13.6e",modelview_matrix[i]);
				}
				display_message(INFORMATION_MESSAGE," projection matrix");
				for (i=0;i<16;i++)
				{
					display_message(INFORMATION_MESSAGE," %13.6e",projection_matrix[i]);
				}
			}
			else
			{
				/* parallel/perspective: write eye and interest points and up-vector */
				Scene_viewer_get_lookat_parameters(scene_viewer,
					&(eye[0]),&(eye[1]),&(eye[2]),&(lookat[0]),&(lookat[1]),&(lookat[2]),
					&(up[0]),&(up[1]),&(up[2]));
				Scene_viewer_get_viewing_volume(scene_viewer,&left,&right,
					&bottom,&top,&near,&far);
				Scene_viewer_get_view_angle(scene_viewer,&view_angle);
				display_message(INFORMATION_MESSAGE,
					" eye_point %g %g %g",eye[0],eye[1],eye[2]);
				display_message(INFORMATION_MESSAGE,
					" interest_point %g %g %g",lookat[0],lookat[1],lookat[2]);
				display_message(INFORMATION_MESSAGE,
					" up_vector %g %g %g",up[0],up[1],up[2]);
				display_message(INFORMATION_MESSAGE,
					" view_angle %g",view_angle*180/PI);
				display_message(INFORMATION_MESSAGE,
					" near_clipping_plane %g far_clipping_plane %g",near,far);
				/* work out if view is skew = up not orthogonal to view direction */
				eye[0] -= lookat[0];
				eye[1] -= lookat[1];
				eye[2] -= lookat[2];
				normalize3(eye);
				normalize3(up);
				if (0.00001<fabs(dot_product3(up,eye)))
				{
					display_message(INFORMATION_MESSAGE," allow_skew");
				}
			}
			viewport_mode=Scene_viewer_get_viewport_mode(scene_viewer);
			display_message(INFORMATION_MESSAGE," %s",
				Scene_viewer_viewport_mode_string(viewport_mode));


			Scene_viewer_get_NDC_info(scene_viewer,
				&NDC_left,&NDC_top,&NDC_width,&NDC_height);
			display_message(INFORMATION_MESSAGE," ndc_placement %g %g %g %g",
				NDC_left,NDC_top,NDC_width,NDC_height);
			Scene_viewer_get_viewport_info(scene_viewer,
				&viewport_left,&viewport_top,&viewport_pixels_per_unit_x,
				&viewport_pixels_per_unit_y);
			display_message(INFORMATION_MESSAGE,
				" viewport_coordinates %g %g %g %g",viewport_left,viewport_top,
				viewport_pixels_per_unit_x,viewport_pixels_per_unit_y);
			display_message(INFORMATION_MESSAGE,";\n");
			/* overlay */
			display_message(INFORMATION_MESSAGE,
				"gfx modify window %s overlay",window->name);
			if (overlay_scene=Scene_viewer_get_overlay_scene(scene_viewer))
			{
				if (GET_NAME(Scene)(overlay_scene,&name))
				{
					/* put quotes around name if it contains special characters */
					make_valid_token(&name);
					display_message(INFORMATION_MESSAGE," scene %s",name);
					DEALLOCATE(name);
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," scene none");
			}
			display_message(INFORMATION_MESSAGE,";\n");
		}
		/* settings */
		display_message(INFORMATION_MESSAGE,
			"gfx modify window %s set",window->name);
		if (GET_NAME(Interactive_tool)(window->interactive_tool,&name))
		{
			display_message(INFORMATION_MESSAGE," %s",name);
			DEALLOCATE(name);
		}
		display_message(INFORMATION_MESSAGE,
			" current_pane %d",window->current_pane+1);
		display_message(INFORMATION_MESSAGE,
			" std_view_angle %g",window->std_view_angle);
		Scene_viewer_get_perturb_lines(window->scene_viewer[0],&perturb_lines);
		if (perturb_lines)
		{
			display_message(INFORMATION_MESSAGE," perturb_lines");
		}
		else
		{
			display_message(INFORMATION_MESSAGE," normal_lines");
		}
		Scene_viewer_get_antialias_mode(window->scene_viewer[0],&antialias);
		if (antialias)
		{
			display_message(INFORMATION_MESSAGE," antialias %d",antialias);
		}
		else
		{
			display_message(INFORMATION_MESSAGE," no_antialias");
		}
		transparency_mode = Scene_viewer_get_transparency_mode(scene_viewer);
		display_message(INFORMATION_MESSAGE," %s",
			Scene_viewer_transparency_mode_string(transparency_mode));
		if (transparency_mode == SCENE_VIEWER_LAYERED_TRANSPARENCY)
		{
			display_message(INFORMATION_MESSAGE,
				" %d",Scene_viewer_get_transparency_layers(scene_viewer));
		}
		display_message(INFORMATION_MESSAGE,";\n");
		DEALLOCATE(prefix);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Graphics_window.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* list_Graphics_window_commands */

int modify_Graphics_window(struct Parse_state *state,void *help_mode,
	void *modify_graphics_window_data_void)
/*******************************************************************************
LAST MODIFIED : 18 November 1998

DESCRIPTION :
Parser commands for modifying graphics windows - views, lighting, etc.
See comments with struct Modify_graphics_window_data;
Parameter <help_mode> should be NULL when calling this function.
==============================================================================*/
{
	int return_code;
	struct Graphics_window *window;
	struct Modify_graphics_window_data *modify_graphics_window_data;
	static struct Modifier_entry
		help_option_table[]=
		{
			{"GRAPHICS_WINDOW_NAME",NULL,NULL,modify_Graphics_window},
			{NULL,NULL,NULL,NULL}
		},
		option_table[]=
		{
			{NULL,NULL,NULL,set_Graphics_window}
		},
		valid_window_option_table[]=
		{
			{"background",NULL,NULL,modify_Graphics_window_background},
			{"image",NULL,NULL,modify_Graphics_window_image},
			{"layout",NULL,NULL,modify_Graphics_window_layout},
			{"overlay",NULL,NULL,modify_Graphics_window_overlay},
			{"set",NULL,NULL,modify_Graphics_window_set},
			{"view",NULL,NULL,modify_Graphics_window_view},
			{NULL,NULL,NULL,NULL}
		};

	ENTER(modify_Graphics_window);
	if (state)
	{
		if (modify_graphics_window_data=(struct Modify_graphics_window_data *)
			modify_graphics_window_data_void)
		{
			return_code=1;
			/* initialize defaults */
			window=(struct Graphics_window *)NULL;
			if (!help_mode)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* read element group */
					(option_table[0]).to_be_modified= (void *)&window;
					(option_table[0]).user_data=
						(void *)(modify_graphics_window_data->graphics_window_manager);
					return_code=process_option(state,option_table);
				}
				else
				{
					/* write help - use help_mode flag to get correct behaviour */
					(help_option_table[0]).to_be_modified= (void *)1;
					(help_option_table[0]).user_data=modify_graphics_window_data_void;
					return_code=process_option(state,help_option_table);
				}
			}
			if (return_code)
			{
				(valid_window_option_table[0]).to_be_modified=(void *)window;
				(valid_window_option_table[0]).user_data=
					modify_graphics_window_data_void;
				(valid_window_option_table[1]).to_be_modified=(void *)window;
				(valid_window_option_table[1]).user_data=
					modify_graphics_window_data_void;
				(valid_window_option_table[2]).to_be_modified=(void *)window;
				(valid_window_option_table[2]).user_data=
					modify_graphics_window_data_void;
				(valid_window_option_table[3]).to_be_modified=(void *)window;
				(valid_window_option_table[3]).user_data=
					modify_graphics_window_data_void;
				(valid_window_option_table[4]).to_be_modified=(void *)window;
				(valid_window_option_table[4]).user_data=
					modify_graphics_window_data_void;
				(valid_window_option_table[5]).to_be_modified=(void *)window;
				(valid_window_option_table[5]).user_data=
					modify_graphics_window_data_void;
				return_code=process_option(state,valid_window_option_table);
			}
			if (window)
			{
				DEACCESS(Graphics_window)(&window);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"modify_Graphics_window.  Missing modify_graphics_window_data");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"modify_Graphics_window.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* modify_Graphics_window */

int set_Graphics_window(struct Parse_state *state,void *window_address_void,
	void *graphics_window_manager_void)
/*******************************************************************************
LAST MODIFIED : 10 December 1997

DESCRIPTION :
Used in command parsing to translate a window name into a Graphics_window.
NOTE: Calling function must remember to ACCESS any window passed to this
function, and DEACCESS any returned window.
???RC set_Object routines could become a macro.
==============================================================================*/
{
	char *current_token;
	int return_code;
	struct Graphics_window *window,**window_address;
	struct MANAGER(Graphics_window) *graphics_window_manager;

	ENTER(set_Graphics_window);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if ((window_address=(struct Graphics_window **)window_address_void)&&
					(graphics_window_manager=
					(struct MANAGER(Graphics_window) *)graphics_window_manager_void))
				{
					if (window=FIND_BY_IDENTIFIER_IN_MANAGER(Graphics_window,name)(
						current_token,graphics_window_manager))
					{
						ACCESS(Graphics_window)(window);
						if (*window_address)
						{
							DEACCESS(Graphics_window)(window_address);
						}
						*window_address=window;
						return_code=shift_Parse_state(state,1);
					}
					else
					{
						display_message(WARNING_MESSAGE,"Unknown window: %s",current_token);
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"set_Graphics_window.  Invalid argument(s)");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," WINDOW_NUMBER");
				if ((window_address=(struct Graphics_window **)window_address_void)&&
					(window= *window_address))
				{
					display_message(INFORMATION_MESSAGE,"[%s]",window->name);
				}
				return_code=1;
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,"Missing window number");
			display_parse_state_location(state);
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_Graphics_window.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_Graphics_window */

#if defined (IMAGEMAGICK)
int write_Graphics_window_to_file(char *file_name,
	struct Graphics_window *window,
	int force_onscreen, int preferred_width, int preferred_height)	  
/*******************************************************************************
LAST MODIFIED : 10 May 2001

DESCRIPTION :
This writes the first scene viewer of the graphics window to a file.
When using IMAGEMAGICK the file format is determined according to the file
extension or a filename prefix, i.e. sgi:bob.rgb writes an sgi formatted file
with name bob.rgb.
If <force_onscreen> is set then the pixels are grabbed directly from the window
display and the <preferred_width> and <preferred_height> are ignored.
==============================================================================*/
{
	char *frame_data;
	int height, return_code, width;

	ENTER(write_Graphics_window_to_file);
	return_code=0;
	/* check arguments */
	if (file_name&&window&&(window->scene_viewer))
	{
		width = preferred_width;
		height = preferred_height;
		if (Graphics_window_get_frame_pixels(window, TEXTURE_RGBA,
			&width, &height, &frame_data, force_onscreen))
		{
			return_code = write_image_file(file_name,/* number_of_components */4,
				/* bytes_per_component */1, height, width, /*padding*/0, 
				(unsigned long *)frame_data);
			DEALLOCATE(frame_data);
		}
		else
		{
			display_message(ERROR_MESSAGE,"write_Graphics_window_to_file.  "
				"Could not allocate array for frame_data");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_Graphics_window_to_file.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* write_Graphics_window_to_file */
#else /* defined (IMAGEMAGICK) */
int write_Graphics_window_to_file(char *file_name,
	struct Graphics_window *window, 	enum Image_file_format image_file_format,
	int force_onscreen, int preferred_width, int preferred_height)	  
/*******************************************************************************
LAST MODIFIED : 10 May 2001

DESCRIPTION :
This writes the first scene viewer of the graphics window to a file.
When using IMAGEMAGICK the file format is determined according to the file
extension or a filename prefix, i.e. sgi:bob.rgb writes an sgi formatted file
with name bob.rgb.
If <force_onscreen> is set then the pixels are grabbed directly from the window
display and the <preferred_width> and <preferred_height> are ignored.
==============================================================================*/
{
	char *frame_data;
	enum Image_orientation image_orientation = PORTRAIT_ORIENTATION;
	float pixel_aspect_ratio;
	int height, return_code, width;
	Screen *screen;
	struct Printer printer;

	ENTER(write_Graphics_window_to_file);
	return_code=0;
	/* check arguments */
	if (file_name&&window&&(window->scene_viewer))
	{
		width = preferred_width;
		height = preferred_height;
		if (Graphics_window_get_frame_pixels(window, TEXTURE_RGBA,
			&width, &height, &frame_data, force_onscreen))
		{
			switch (image_file_format)
			{
				case POSTSCRIPT_FILE_FORMAT:
				{
					if (return_code=open_printer(&printer,window->user_interface))
					{
						screen=DefaultScreenOfDisplay(window->user_interface->display);
						pixel_aspect_ratio=
							((float)HeightMMOfScreen(screen)/(float)HeightOfScreen(screen))/
							((float)WidthMMOfScreen(screen)/(float)WidthOfScreen(screen));
						return_code=write_postscript_image_file(file_name,4,
							/* bytes_per_component */1, height,width, 0,
							pixel_aspect_ratio,(unsigned long *)frame_data,
							image_orientation,&printer);
						close_printer(&printer);
					}
				} break;
				case RGB_FILE_FORMAT:
				{
					return_code=write_rgb_image_file(file_name,4,
						/* bytes_per_component */1, height,width, 0, 
						(unsigned long *)frame_data);
				} break;
				case TIFF_FILE_FORMAT:
				{
					return_code=write_tiff_image_file(file_name,4,
						/* bytes_per_component */1, height,width, 0,
						TIFF_PACK_BITS_COMPRESSION, (unsigned long *)frame_data);
				} break;
				case YUV_FILE_FORMAT:
				{
					display_message(ERROR_MESSAGE,"Print graphics window: "
						"Cannot write YUV file '%s'; file format not supported for writing",
						file_name);
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,"Print graphics window: "
						"Unknown image file format for '%s'",file_name);
				} break;
			}
			DEALLOCATE(frame_data);
		}
		else
		{
			display_message(ERROR_MESSAGE,"write_Graphics_window_to_file.  "
				"Could not allocate array for frame_data");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_Graphics_window_to_file.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* write_Graphics_window_to_file */
#endif /* defined (IMAGEMAGICK) */

char *Graphics_window_layout_mode_string(
	enum Graphics_window_layout_mode layout_mode)
/*******************************************************************************
LAST MODIFIED : 28 June 2000

DESCRIPTION :
Returns a string label for the <layout_mode>, used in widgets and parsing.
NOTE: Calling function must not deallocate returned string.
==============================================================================*/
{
	char *return_string;

	ENTER(Graphics_window_layout_mode_string);
	switch (layout_mode)
	{
		case GRAPHICS_WINDOW_LAYOUT_2D:
		{
			return_string="2d";
		} break;
		case GRAPHICS_WINDOW_LAYOUT_FREE_ORTHO:
		{
			return_string="free_ortho";
		} break;
		case GRAPHICS_WINDOW_LAYOUT_FRONT_BACK:
		{
			return_string="front_back";
		} break;
		case GRAPHICS_WINDOW_LAYOUT_FRONT_SIDE:
		{
			return_string="front_side";
		} break;
		case GRAPHICS_WINDOW_LAYOUT_ORTHOGRAPHIC:
		{
			return_string="orthographic";
		} break;
		case GRAPHICS_WINDOW_LAYOUT_PSEUDO_3D:
		{
			return_string="pseudo_3d";
		} break;
		case GRAPHICS_WINDOW_LAYOUT_SIMPLE:
		{
			return_string="simple";
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"Graphics_window_layout_mode_string.  Unknown layout mode");
			return_string=(char *)NULL;
		}
	}
	LEAVE;

	return (return_string);
} /* Graphics_window_layout_mode_string */

char **Graphics_window_layout_mode_get_valid_strings(
	int *number_of_valid_strings)
/*******************************************************************************
LAST MODIFIED : 28 June 2000

DESCRIPTION :
Returns an allocated array of pointers to all static strings for valid
Graphics_window_layout_modes - obtained from function
Graphics_window_layout_mode_string.
Up to calling function to deallocate returned array - but not the strings in it!
==============================================================================*/
{
	char **valid_strings;
	enum Graphics_window_layout_mode layout_mode;
	int i;

	ENTER(Graphics_window_layout_mode_get_valid_strings);
	if (number_of_valid_strings)
	{
		*number_of_valid_strings=0;
		layout_mode=GRAPHICS_WINDOW_LAYOUT_MODE_BEFORE_FIRST;
		layout_mode++;
		while (layout_mode<GRAPHICS_WINDOW_LAYOUT_MODE_AFTER_LAST)
		{
			(*number_of_valid_strings)++;
			layout_mode++;
		}
		if (ALLOCATE(valid_strings,char *,*number_of_valid_strings))
		{
			layout_mode=GRAPHICS_WINDOW_LAYOUT_MODE_BEFORE_FIRST;
			layout_mode++;
			i=0;
			while (layout_mode<GRAPHICS_WINDOW_LAYOUT_MODE_AFTER_LAST)
			{
				valid_strings[i]=Graphics_window_layout_mode_string(layout_mode);
				i++;
				layout_mode++;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Graphics_window_layout_mode_get_valid_strings.  Not enough memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_layout_mode_get_valid_strings.  Invalid argument");
		valid_strings=(char **)NULL;
	}
	LEAVE;

	return (valid_strings);
} /* Graphics_window_layout_mode_get_valid_strings */

enum Graphics_window_layout_mode Graphics_window_layout_mode_from_string(
	char *layout_mode_string)
/*******************************************************************************
LAST MODIFIED : 28 June 2000

DESCRIPTION :
Returns the <Graphics_window_layout_mode> described by <layout_mode_string>,
or GRAPHICS_WINDOW_LAYOUT_MODE_INVALID if not recognized.
==============================================================================*/
{
	enum Graphics_window_layout_mode layout_mode;

	ENTER(Graphics_window_layout_mode_from_string);
	if (layout_mode_string)
	{
		layout_mode=GRAPHICS_WINDOW_LAYOUT_MODE_BEFORE_FIRST;
		layout_mode++;
		while ((layout_mode<GRAPHICS_WINDOW_LAYOUT_MODE_AFTER_LAST)&&
			(!fuzzy_string_compare_same_length(layout_mode_string,
				Graphics_window_layout_mode_string(layout_mode))))
		{
			layout_mode++;
		}
		if (GRAPHICS_WINDOW_LAYOUT_MODE_AFTER_LAST==layout_mode)
		{
			layout_mode=GRAPHICS_WINDOW_LAYOUT_MODE_INVALID;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphics_window_layout_mode_from_string.  Invalid argument");
		layout_mode=GRAPHICS_WINDOW_LAYOUT_MODE_INVALID;
	}
	LEAVE;

	return (layout_mode);
} /* Graphics_window_layout_mode_from_string */






int Graphics_window_layout_mode_get_number_of_panes(
	enum Graphics_window_layout_mode layout_mode)
/*******************************************************************************
LAST MODIFIED : 28 June 2000

DESCRIPTION :
Returns the number of panes in a graphics window with the given <layout_mode>.
==============================================================================*/
{
	int number_of_panes;

	ENTER(Graphics_window_layout_mode_get_number_of_panes);
	switch (layout_mode)
	{
		case GRAPHICS_WINDOW_LAYOUT_SIMPLE:
		case GRAPHICS_WINDOW_LAYOUT_2D:
		{
			number_of_panes=1;
		} break;
		case GRAPHICS_WINDOW_LAYOUT_ORTHOGRAPHIC:
		case GRAPHICS_WINDOW_LAYOUT_FREE_ORTHO:
		{
			number_of_panes=4;
		} break;
		case GRAPHICS_WINDOW_LAYOUT_FRONT_BACK:
		case GRAPHICS_WINDOW_LAYOUT_FRONT_SIDE:
		case GRAPHICS_WINDOW_LAYOUT_PSEUDO_3D:
		{
			number_of_panes=2;
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"Graphics_window_layout_mode_get_number_of_panes.  Unknown layout mode");
			number_of_panes=0;
		}
	}
	LEAVE;

	return (number_of_panes);
} /* Graphics_window_layout_mode_get_number_of_panes */

int Graphics_window_layout_mode_is_projection_mode_valid_for_pane(
	enum Graphics_window_layout_mode layout_mode,int pane_no,
	enum Scene_viewer_projection_mode projection_mode)
/*******************************************************************************
LAST MODIFIED : 28 June 2000

DESCRIPTION :
Returns true if the <projection_mode> can be used with pane <pane_no> of a
graphics window with the given <layout_mode>.
==============================================================================*/
{
	int return_code;

	ENTER(Graphics_window_layout_mode_is_projection_mode_valid_for_pane);
	switch (layout_mode)
	{
		case GRAPHICS_WINDOW_LAYOUT_SIMPLE:
		{
			return_code=((0==pane_no)&&(
				(SCENE_VIEWER_PARALLEL==projection_mode)||
				(SCENE_VIEWER_PERSPECTIVE==projection_mode)||
				(SCENE_VIEWER_CUSTOM==projection_mode)));
		} break;
		case GRAPHICS_WINDOW_LAYOUT_2D:
		{
			return_code=((0==pane_no)&&(
				(SCENE_VIEWER_PARALLEL==projection_mode)||
				(SCENE_VIEWER_PERSPECTIVE==projection_mode)));
		} break;
		case GRAPHICS_WINDOW_LAYOUT_ORTHOGRAPHIC:
		case GRAPHICS_WINDOW_LAYOUT_FREE_ORTHO:
		{
			return_code=((0==pane_no)&&(
				(SCENE_VIEWER_PARALLEL==projection_mode)||
				(SCENE_VIEWER_PERSPECTIVE==projection_mode)))||
				((1<=pane_no)&&(4>pane_no)&&(SCENE_VIEWER_PARALLEL==projection_mode));
		} break;
		case GRAPHICS_WINDOW_LAYOUT_FRONT_BACK:
		case GRAPHICS_WINDOW_LAYOUT_FRONT_SIDE:
		case GRAPHICS_WINDOW_LAYOUT_PSEUDO_3D:
		{
			return_code=(0<=pane_no)&&(2>pane_no)&&(
				(SCENE_VIEWER_PARALLEL==projection_mode)||
				(SCENE_VIEWER_PERSPECTIVE==projection_mode));
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"Graphics_window_layout_mode_is_projection_mode_valid_for_pane.  "
				"Unknown layout mode");
			return_code=0;
		}
	}
	LEAVE;

	return (return_code);
} /* Graphics_window_layout_mode_is_projection_mode_valid_for_pane */
