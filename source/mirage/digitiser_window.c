/*******************************************************************************
FILE : digitiser_window.c

LAST MODIFIED : 21 June 1999

DESCRIPTION:
Routines for generating and managing digitiser_windows. The digitiser window
utilises a Scene_viewer for display of a 3-D Scene with a 2-D Texture shown in
the background. A CUSTOM projection and ABSOLUTE viewport mode are used to
align the 3-D view with the texture image it is digitised from.
==============================================================================*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <Xm/Protocols.h>
#include <Xm/Xm.h>
#include <Xm/DialogS.h>
#include <Xm/MwmUtil.h>
#include <Xm/ToggleB.h>
#include <Mrm/MrmPublic.h>
#include <Mrm/MrmDecls.h>

#include "graphics/graphics_object.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_to_streamlines.h"
#include "general/compare.h"
#include "general/debug.h"
#include "general/indexed_list_private.h"
#include "general/manager_private.h"
#include "general/matrix_vector.h"
#include "general/object.h"
#include "general/photogrammetry.h"
#include "graphics/colour.h"
#include "mirage/digitiser_window.h"
#include "mirage/digitiser_window.uidh"
#include "graphics/light.h"
#include "graphics/light_model.h"
#include "graphics/scene.h"
#include "graphics/scene_viewer.h"
#include "graphics/texture.h"
#include "mirage/movie.h"
#include "user_interface/gui_dialog_macros.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module types
------------
*/
struct Digitiser_window
/*******************************************************************************
LAST MODIFIED : 18 November 1998

DESCRIPTION :
Contains information for a digitiser window.
==============================================================================*/
{
	/* identifier for uniquely specifying window: */
	char *name;
	/* need to keep digitiser window manager so window can be destroyed by self */
	struct MANAGER(Digitiser_window) *digitiser_window_manager;
	/* widgets on the Digitiser_window */
	Widget viewing_area,views_rowcolumn,scale_half_button,scale_one_button,
		scale_double_button,clone_button;
	Widget window_shell,main_window;

	/* scene_viewers and their parameters: */
	enum Digitiser_window_buffer_mode buffer_mode;
	struct Scene_viewer *scene_viewer;

	/* Mirage movie and view: */
	struct Mirage_movie *movie;
	int view_no;

	struct MANAGER(Light) *light_manager;
	struct MANAGER(Light_model) *light_model_manager;
	struct MANAGER(Scene) *scene_manager;
	struct MANAGER(Texture) *texture_manager;
	struct User_interface *user_interface;
	/* the number of objects accessing this window. The window cannot be removed
		from manager unless it is 1 (ie. only the manager is accessing it) */
	int access_count;
}; /* struct Digitiser_window */

FULL_DECLARE_INDEXED_LIST_TYPE(Digitiser_window);

FULL_DECLARE_MANAGER_TYPE(Digitiser_window);

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int digitiser_window_hierarchy_open=0;
static MrmHierarchy digitiser_window_hierarchy;
#endif /* defined (MOTIF) */

/*
Module functions
----------------
*/
DECLARE_INDEXED_LIST_MODULE_FUNCTIONS(Digitiser_window, \
	name,char *,strcmp)
DECLARE_LOCAL_MANAGER_FUNCTIONS(Digitiser_window)

DECLARE_DIALOG_IDENTIFY_FUNCTION(digitiser_window, \
	Digitiser_window,viewing_area)
DECLARE_DIALOG_IDENTIFY_FUNCTION(digitiser_window, \
	Digitiser_window,views_rowcolumn)
DECLARE_DIALOG_IDENTIFY_FUNCTION(digitiser_window, \
	Digitiser_window,scale_half_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(digitiser_window, \
	Digitiser_window,scale_one_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(digitiser_window, \
	Digitiser_window,scale_double_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(digitiser_window, \
	Digitiser_window,clone_button)

int Digitiser_window_set_view(struct Digitiser_window *digitiser_window,
	int view_no)
/*******************************************************************************
LAST MODIFIED : 19 March 1998

DESCRIPTION :
==============================================================================*/
{
	double modelview_matrix[16],projection_matrix[16],eye[3],lookat[3],up[3],
		viewport_left,viewport_top,viewport_pixels_per_unit_x,
		viewport_pixels_per_unit_y;
	int return_code,width_texels,height_texels;
	struct Mirage_view *view,*old_view;

	ENTER(Digitiser_window_set_view);
	if (digitiser_window&&digitiser_window->movie&&(0<=view_no)&&
		(view_no<digitiser_window->movie->number_of_views)&&
		(old_view=digitiser_window->movie->views[digitiser_window->view_no])&&
		(view=digitiser_window->movie->views[view_no]))
	{
		if (return_code=photogrammetry_to_graphics_projection(
			view->transformation43,
			view->near_clipping_plane,view->far_clipping_plane,
			view->NDC_left,view->NDC_bottom,
			view->NDC_width,view->NDC_height,
			modelview_matrix,projection_matrix,eye,lookat,up))
		{
			digitiser_window->view_no=view_no;
			Scene_viewer_set_projection_mode(digitiser_window->scene_viewer,
				SCENE_VIEWER_CUSTOM);
			Scene_viewer_set_viewport_mode(digitiser_window->scene_viewer,
				SCENE_VIEWER_ABSOLUTE_VIEWPORT);
			Scene_viewer_set_modelview_matrix(digitiser_window->scene_viewer,
				modelview_matrix);
			Scene_viewer_set_lookat_parameters(digitiser_window->scene_viewer,
				eye[0],eye[1],eye[2],lookat[0],lookat[1],lookat[2],up[0],up[1],up[2]);
			Scene_viewer_set_NDC_info(digitiser_window->scene_viewer,
				view->NDC_left,view->NDC_bottom+view->NDC_height,
				view->NDC_width,view->NDC_height);
			Scene_viewer_set_projection_matrix(
				digitiser_window->scene_viewer,projection_matrix);
			Scene_viewer_set_scene(digitiser_window->scene_viewer,view->scene);
			if (0==(digitiser_window->movie->current_frame_no % 2))
			{
				/* even frame numbers */
				Scene_viewer_set_background_texture_info(
					digitiser_window->scene_viewer,
					view->image0_left,view->image0_bottom+view->image0_height,
					view->image0_width,view->image0_height,
					/*undistort_on*/1,/*max_pixels_per_polygon*/32);
			}
			else
			{
				/* odd frame numbers */
				Scene_viewer_set_background_texture_info(
					digitiser_window->scene_viewer,
					view->image1_left,view->image1_bottom+view->image1_height,
					view->image1_width,view->image1_height,
					/*undistort_on*/1,/*max_pixels_per_polygon*/32);
			}
			Scene_viewer_set_background_texture(digitiser_window->scene_viewer,
				view->texture);

			/* want same offset relative to NDC_top,NDC_left */
			if (Scene_viewer_get_viewport_info(digitiser_window->scene_viewer,
				&viewport_left,&viewport_top,
				&viewport_pixels_per_unit_x,&viewport_pixels_per_unit_y))
			{
				Scene_viewer_set_viewport_info(digitiser_window->scene_viewer,
					viewport_left+view->NDC_left  -old_view->NDC_left,
					viewport_top +view->NDC_bottom-old_view->NDC_bottom,
					viewport_pixels_per_unit_x,viewport_pixels_per_unit_y);
			}
			Digitiser_window_update(digitiser_window);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Digitiser_window_set_view.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Digitiser_window_set_view */

static int Digitiser_window_make_view_buttons(
	struct Digitiser_window *digitiser_window)
/*******************************************************************************
LAST MODIFIED : 14 February 1998

DESCRIPTION :
==============================================================================*/
{
	char *full_view_name;
	int return_code,view_no;
	struct Mirage_view *view;
	XmString new_string;
	Arg args[4];
	Widget temp_widget;

	ENTER(Digitiser_window_make_view_buttons);
	/* check arguments */
	if (digitiser_window&&digitiser_window->movie)
	{
		return_code=1;
		for (view_no=0;return_code&&
			(view_no<digitiser_window->movie->number_of_views);view_no++)
		{
			if ((view=digitiser_window->movie->views[view_no])&&
				ALLOCATE(full_view_name,char,8+strlen(view->name)))
			{
				sprintf(full_view_name,"View %s",view->name);
				XtSetArg(args[0],XmNuserData,(XtPointer)view_no);
				new_string=XmStringCreateSimple(full_view_name);
				XtSetArg(args[1],XmNlabelString,(XtPointer)new_string);
				if (view_no==digitiser_window->view_no)
				{
					XtSetArg(args[2],XmNset,(XtPointer)True);
				}
				else
				{
					XtSetArg(args[2],XmNset,(XtPointer)False);
				}
				XtSetArg(args[3],XmNindicatorOn,(XtPointer)False);
				if (temp_widget=XmCreateToggleButton(
					digitiser_window->views_rowcolumn,full_view_name,args,4))
				{
					/* make the settings_string the name of this item */
					/*new_string=XmStringCreateSimple(settings_string);
					XtVaSetValues(temp_widget,XmNlabelString,new_string,NULL);
					XmStringFree(new_string);*/
					XtManageChild(temp_widget);
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Digitiser_window_make_view_buttons.  "
						"Could not create view button");
					return_code=0;
				}
				XmStringFree(new_string);
				DEALLOCATE(full_view_name);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Digitiser_window_make_view_buttons.  Missing view");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Digitiser_window_make_view_buttons.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Digitiser_window_make_view_buttons */

#if defined (OLD_CODE)
static int Digitiser_window_read_defaults(
	struct Digitiser_window *digitiser_window)
/*******************************************************************************
LAST MODIFIED : 16 December 1997

DESCRIPTION :
==============================================================================*/
{
#define XmNdigitiserWindowOrthoUpAxis "digitiserWindowOrthoUpAxis"
#define XmCDigitiserWindowOrthoUpAxis "DigitiserWindowOrthoUpAxis"
#define XmNdigitiserWindowOrthoFrontAxis "digitiserWindowOrthoFrontAxis"
#define XmCDigitiserWindowOrthoFrontAxis "DigitiserWindowOrthoFrontAxis"
	int return_code,up_axis,front_axis;
	struct Digitiser_window_defaults
	{
		char *up_axis_name,*front_axis_name;
	} digitiser_window_defaults;
	static XtResource resources[]=
	{
		{
			XmNdigitiserWindowOrthoUpAxis,
			XmCDigitiserWindowOrthoUpAxis,
			XmRString,
			sizeof(char *),
			XtOffsetOf(struct Digitiser_window_defaults,up_axis_name),
			XmRString,
			"Z"
		},
		{
			XmNdigitiserWindowOrthoFrontAxis,
			XmCDigitiserWindowOrthoFrontAxis,
			XmRString,
			sizeof(char *),
			XtOffsetOf(struct Digitiser_window_defaults,front_axis_name),
			XmRString,
			"-Y"
		}
	};

	ENTER(Digitiser_window_read_defaults);
	if (digitiser_window)
	{
		up_axis=0;
		front_axis=0;
		digitiser_window_defaults.up_axis_name=(char *)NULL;
		digitiser_window_defaults.front_axis_name=(char *)NULL;
		XtVaGetApplicationResources(
			digitiser_window->user_interface->application_shell,
			&digitiser_window_defaults,resources,XtNumber(resources),NULL);
		if (digitiser_window_defaults.up_axis_name)
		{
			up_axis=axis_name_to_axis_number(digitiser_window_defaults.up_axis_name);
		}
		if (digitiser_window_defaults.front_axis_name)
		{
			front_axis=axis_name_to_axis_number(
				digitiser_window_defaults.front_axis_name);
		}
		digitiser_window->ortho_up_axis=up_axis;
		digitiser_window->ortho_front_axis=front_axis;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Digitiser_window_read_defaults.  Missing digitiser_window");
		return_code=0;
	}
	LEAVE;

	return return_code;
} /* Digitiser_window_read_defaults */
#endif /* defined (OLD_CODE) */

static void Digitiser_window_destroy_CB(Widget caller,
	XtPointer user_data,XtPointer caller_data)
/*******************************************************************************
LAST MODIFIED : 21 June 1999

DESCRIPTION :
Callback for when the window is destroyed. Does not do anything since the
Digitiser_window object is now destroyed by its DESTROY function.
Keep in case a use is found for it.
==============================================================================*/
{
	ENTER(Digitiser_window_destroy_CB);
	USE_PARAMETER(caller);
	USE_PARAMETER(user_data);
	USE_PARAMETER(caller_data);
	LEAVE;
} /* Digitiser_window_destroy_CB */

static void Digitiser_window_close_CB(Widget caller,
	void *digitiser_window_void,void *cbs)
/*******************************************************************************
LAST MODIFIED : 21 June 1999

DESCRIPTION :
Called when "close" is selected from the window menu, or it is double clicked.
How this is made to occur is as follows. The digitiser_window dialog has its
XmNdeleteResponse == XmDO_NOTHING, and a window manager protocol callback for
WM_DELETE_WINDOW has been set up with XmAddWMProtocolCallback to call this
function in response to the close command. See CREATE(Digitiser_window) for
more details.
Since the Digitiser_window is a managed object, we simply remove it from its
manager. If this is successful it will be DESTROYed automatically.
If it is not managed, can't destroy it here.
==============================================================================*/
{
	struct Digitiser_window *digitiser_window;

	ENTER(Digitiser_window_close_CB);
	USE_PARAMETER(caller);
	USE_PARAMETER(cbs);
	if (digitiser_window=(struct Digitiser_window *)digitiser_window_void)
	{
		if (digitiser_window->digitiser_window_manager)
		{
			REMOVE_OBJECT_FROM_MANAGER(Digitiser_window)(digitiser_window,
				digitiser_window->digitiser_window_manager);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Digitiser_window_close_CB.  Missing Digitiser_window");
	}
	LEAVE;
} /* Digitiser_window_close_CB */

static void Digitiser_window_change_view_CB(Widget widget,
	XtPointer client_data,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 16 February 1998

DESCRIPTION :
Called when a view button is selected.
==============================================================================*/
{
	int view_no;
	struct Digitiser_window *digitiser_window;
	Widget view_button;
	XmRowColumnCallbackStruct *rowcolumn;

	ENTER(Digitiser_window_change_view_CB);
	if (widget&&(digitiser_window=(struct Digitiser_window *)client_data)&&
		(rowcolumn=(XmRowColumnCallbackStruct *)call_data))
	{
		/* get the widget from the call data */
		if (view_button=rowcolumn->widget)
		{
			XtVaGetValues(view_button,XmNuserData,&view_no,NULL);
			if (((XmToggleButtonCallbackStruct *)(rowcolumn->callbackstruct))->set)
			{
				Digitiser_window_set_view(digitiser_window,view_no);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Digitiser_window_change_view_CB.  Missing view button");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Digitiser_window_change_view_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Digitiser_window_change_view_CB */

static void Digitiser_window_scale_half_button_CB(Widget caller,
	XtPointer *digitiser_window_void,XmAnyCallbackStruct *caller_data)
/*******************************************************************************
LAST MODIFIED : 21 June 1999

DESCRIPTION :
Halves the scale of the image while keeping the same centre point.
==============================================================================*/
{
	struct Digitiser_window *digitiser_window;

	ENTER(Digitiser_window_scale_half_button_CB);
	USE_PARAMETER(caller);
	USE_PARAMETER(caller_data);
	if ((digitiser_window=(struct Digitiser_window *)digitiser_window_void)&&
		digitiser_window->movie)
	{
		if (digitiser_window->movie->views[digitiser_window->view_no])
		{
			Scene_viewer_viewport_zoom(digitiser_window->scene_viewer,0.5);
			Digitiser_window_update(digitiser_window);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Digitiser_window_scale_half_button_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Digitiser_window_scale_half_button_CB */

static void Digitiser_window_scale_one_button_CB(Widget caller,
	XtPointer *digitiser_window_void,XmAnyCallbackStruct *caller_data)
/*******************************************************************************
LAST MODIFIED : 21 June 1999

DESCRIPTION :
Callback for the 1:1 scale button. Adjusts user coordinates in the scene
viewer so that the image is shown at 1:1 scale.
Also makes sure viewport_left and viewport_top are whole numbers of units from
the origin of the NDC area.
==============================================================================*/
{
	Dimension width,height;
	double viewport_left,viewport_top,viewport_pixels_per_unit_x,
		viewport_pixels_per_unit_y,NDC_left,NDC_top,NDC_width,NDC_height;
	struct Digitiser_window *digitiser_window;

	ENTER(Digitiser_window_scale_one_button_CB);
	USE_PARAMETER(caller);
	USE_PARAMETER(caller_data);
	if ((digitiser_window=(struct Digitiser_window *)digitiser_window_void)&&
		digitiser_window->movie)
	{
		if (digitiser_window->movie->views[digitiser_window->view_no])
		{
			if (Scene_viewer_get_viewport_info(digitiser_window->scene_viewer,
				&viewport_left,&viewport_top,&viewport_pixels_per_unit_x,
				&viewport_pixels_per_unit_y)&&
				Scene_viewer_get_viewport_size(digitiser_window->scene_viewer,
					&width,&height)&&
				Scene_viewer_get_NDC_info(digitiser_window->scene_viewer,
					&NDC_left,&NDC_top,&NDC_width,&NDC_height))
			{
				viewport_left=NDC_left+floor(viewport_left+
					0.5*(double)width*(1.0/viewport_pixels_per_unit_x-1.0)-NDC_left);
				viewport_top=NDC_top+floor(viewport_top+
					0.5*(double)height*(1.0-1.0/viewport_pixels_per_unit_y)-NDC_top);
				Scene_viewer_set_viewport_info(digitiser_window->scene_viewer,
					viewport_left,viewport_top,1,1);
				Digitiser_window_update(digitiser_window);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Digitiser_window_scale_one_button_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Digitiser_window_scale_one_button_CB */

static void Digitiser_window_scale_double_button_CB(Widget caller,
	XtPointer *digitiser_window_void,XmAnyCallbackStruct *caller_data)
/*******************************************************************************
LAST MODIFIED : 21 June 1999

DESCRIPTION :
==============================================================================*/
{
	struct Digitiser_window *digitiser_window;

	ENTER(Digitiser_window_scale_double_button_CB);
	USE_PARAMETER(caller);
	USE_PARAMETER(caller_data);
	if ((digitiser_window=(struct Digitiser_window *)digitiser_window_void)&&
		digitiser_window->movie)
	{
		if (digitiser_window->movie->views[digitiser_window->view_no])
		{
			Scene_viewer_viewport_zoom(digitiser_window->scene_viewer,2.0);
			Digitiser_window_update(digitiser_window);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Digitiser_window_scale_double_button_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Digitiser_window_scale_double_button_CB */

static int light_to_digitiser_window(struct Light *light,
	void *digitiser_window_void)
/*******************************************************************************
LAST MODIFIED : 15 February 1998

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Digitiser_window *digitiser_window;

	ENTER(light_to_digitiser_window);
	if (light&&
		(digitiser_window=(struct Digitiser_window *)digitiser_window_void))
	{
		return_code=Digitiser_window_add_light(digitiser_window,(void *)light);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"light_to_digitiser_window.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* light_to_digitiser_window */

static void Digitiser_window_clone_button_CB(Widget caller,
	XtPointer *digitiser_window_void,XmAnyCallbackStruct *caller_data)
/*******************************************************************************
LAST MODIFIED : 21 June 1999

DESCRIPTION :
==============================================================================*/
{
	char *digitiser_window_name;
	struct Digitiser_window *digitiser_window,*new_digitiser_window;
	struct Colour background_colour;

	ENTER(Digitiser_window_clone_button_CB);
	USE_PARAMETER(caller);
	USE_PARAMETER(caller_data);
	if ((digitiser_window=(struct Digitiser_window *)digitiser_window_void)&&
		digitiser_window->movie)
	{
		if (digitiser_window->digitiser_window_manager)
		{
			if (Scene_viewer_get_background_colour(digitiser_window->scene_viewer,
				&background_colour))
			{
				if (digitiser_window_name=Digitiser_window_manager_get_new_name(
					digitiser_window->digitiser_window_manager))
				{
					if (new_digitiser_window=CREATE(Digitiser_window)(
						digitiser_window_name,
						digitiser_window->movie,digitiser_window->view_no,
						digitiser_window->buffer_mode,
						&background_colour,
						digitiser_window->light_manager,
						(struct Light *)NULL,
						digitiser_window->light_model_manager,
						Scene_viewer_get_light_model(digitiser_window->scene_viewer),
						digitiser_window->scene_manager,
						digitiser_window->texture_manager,
						digitiser_window->user_interface))
					{
						for_each_Light_in_Scene_viewer(digitiser_window->scene_viewer,
							light_to_digitiser_window,(void *)new_digitiser_window);
						ADD_OBJECT_TO_MANAGER(Digitiser_window)(
							new_digitiser_window,digitiser_window->digitiser_window_manager);
					}
					DEALLOCATE(digitiser_window_name);
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Digitiser_window_clone__button_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Digitiser_window_clone_button_CB */

/*
Global functions
----------------
*/
struct Digitiser_window *CREATE(Digitiser_window)(char *name,
	struct Mirage_movie *movie,int view_no,
	enum Digitiser_window_buffer_mode buffer_mode,
	struct Colour *background_colour,
	struct MANAGER(Light) *light_manager,
	struct Light *default_light,
	struct MANAGER(Light_model) *light_model_manager,
	struct Light_model *default_light_model,
	struct MANAGER(Scene) *scene_manager,
	struct MANAGER(Texture) *texture_manager,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 21 June 1999

DESCRIPTION:
Creates a Digitiser_window object, window shell and widgets. Returns a pointer
to the newly created object. The Digitiser_window maintains a pointer to the
manager it is to live in, since users will want to close windows with the
window manager widgets.
Each window has a unique <name> that can be used to identify it, and which
will be printed on the windows title bar.
==============================================================================*/
{
	Atom WM_DELETE_WINDOW;
	char *window_title;
	enum Scene_viewer_buffer_mode scene_viewer_buffer_mode;
	MrmType digitiser_window_dialog_class;
	static MrmRegisterArg callbacks[] =
	{
		{"digwin_id_viewing_area",(XtPointer)
			DIALOG_IDENTIFY(digitiser_window,viewing_area)},
		{"digwin_id_views_rowcol",(XtPointer)
			DIALOG_IDENTIFY(digitiser_window,views_rowcolumn)},
		{"digwin_id_scale_half_btn",(XtPointer)
			DIALOG_IDENTIFY(digitiser_window,scale_half_button)},
		{"digwin_id_scale_one_btn",(XtPointer)
			DIALOG_IDENTIFY(digitiser_window,scale_one_button)},
		{"digwin_id_scale_double_btn",(XtPointer)
			DIALOG_IDENTIFY(digitiser_window,scale_double_button)},
		{"digwin_id_clone_btn",(XtPointer)
			DIALOG_IDENTIFY(digitiser_window,clone_button)},
		{"digwin_destroy_CB",
			(XtPointer)Digitiser_window_destroy_CB},
		{"digwin_change_view_CB",
			(XtPointer)Digitiser_window_change_view_CB},
		{"digwin_scale_half_btn_CB",
			(XtPointer)Digitiser_window_scale_half_button_CB},
		{"digwin_scale_one_btn_CB",
			(XtPointer)Digitiser_window_scale_one_button_CB},
		{"digwin_scale_double_btn_CB",
			(XtPointer)Digitiser_window_scale_double_button_CB},
		{"digwin_clone_btn_CB",
			(XtPointer)Digitiser_window_clone_button_CB}
	};
	static MrmRegisterArg identifiers[] =
	{
		{"digwin_structure",(XtPointer)NULL}
	};
	struct Digitiser_window *digitiser_window=NULL;
	struct Mirage_view *view;

	ENTER(create_digitiser_window);
	/* check arguments */
	if (name&&movie&&(0<=view_no)&&(view_no<movie->number_of_views)&&
		(view=movie->views[view_no])&&
		((DIGITISER_WINDOW_SINGLE_BUFFER==buffer_mode)||
		(DIGITISER_WINDOW_DOUBLE_BUFFER==buffer_mode))&&background_colour&&
		light_manager&&light_model_manager&&default_light_model&&
		scene_manager&&texture_manager&&user_interface)
	{
		if (MrmOpenHierarchy_base64_string(digitiser_window_uidh,
			&digitiser_window_hierarchy,&digitiser_window_hierarchy_open))
		{
			/* Try to allocate space for the window structure */
			if (ALLOCATE(digitiser_window,struct Digitiser_window,1)&&
				ALLOCATE(digitiser_window->name,char,strlen(name)+1))
			{
				strcpy(digitiser_window->name,name);
				/* initialize the fields of the window structure */
				digitiser_window->access_count=0;
				digitiser_window->buffer_mode=buffer_mode;
				digitiser_window->movie=movie;
				digitiser_window->view_no=view_no;
				digitiser_window->digitiser_window_manager=
					(struct MANAGER(Digitiser_window) *)NULL;
				digitiser_window->light_manager=light_manager;
				digitiser_window->light_model_manager=light_model_manager;
				digitiser_window->scene_manager=scene_manager;
				digitiser_window->texture_manager=texture_manager;
				digitiser_window->user_interface=user_interface;
				digitiser_window->scene_viewer=(struct Scene_viewer *)NULL;
				if (ALLOCATE(window_title,char,50+strlen(name)+strlen(movie->name)+1))
				{
					sprintf(window_title,"CMGUI Digitiser window %s: %s",
						name,movie->name);
				}
				/* clear widgets yet to be read in and created */
				digitiser_window->window_shell=(Widget)NULL;
				digitiser_window->main_window=(Widget)NULL;
				digitiser_window->viewing_area=(Widget)NULL;
				digitiser_window->views_rowcolumn=(Widget)NULL;
				digitiser_window->scale_half_button=(Widget)NULL;
				digitiser_window->scale_one_button=(Widget)NULL;
				digitiser_window->scale_double_button=(Widget)NULL;
				digitiser_window->clone_button=(Widget)NULL;
				/* create a shell for the window */
				if (!(digitiser_window->window_shell=XtVaCreatePopupShell(
					"digitiser_shell",xmDialogShellWidgetClass,
					user_interface->application_shell,
					XmNdeleteResponse,XmDO_NOTHING,
					XmNmwmDecorations,MWM_DECOR_ALL,
					XmNmwmFunctions,MWM_FUNC_ALL,
					/*XmNtransient,FALSE,*/
					XmNallowShellResize,False,
					/* needs to be False to stop the window resizing when
					set_view_panel_from_toggles is called (due to changing the labels) */
					XmNtitle,window_title,
					NULL)))
				{
					DEALLOCATE(digitiser_window);
					display_message(ERROR_MESSAGE,
						"CREATE(digitiser_window).  Could not create a shell");
				}
				else
				{
					/* Register the shell for the busy cursor */
					create_Shell_list_item(&(digitiser_window->window_shell),
						user_interface);

					/* Set up window manager callback for close window message */
					WM_DELETE_WINDOW=XmInternAtom(
						XtDisplay(digitiser_window->window_shell),"WM_DELETE_WINDOW",False);
					XmAddWMProtocolCallback(digitiser_window->window_shell,
						WM_DELETE_WINDOW,Digitiser_window_close_CB,digitiser_window);
					/* register callbacks */
					if (MrmSUCCESS!=MrmRegisterNamesInHierarchy(
						digitiser_window_hierarchy,callbacks,XtNumber(callbacks)))
					{
						XtDestroyWidget(digitiser_window->window_shell);
						DEALLOCATE(digitiser_window);
						display_message(ERROR_MESSAGE,
							"CREATE(digitiser_window).  Could not register the callbacks");
					}
					else
					{
						/* register identifiers */
						identifiers[0].value=(XtPointer)digitiser_window;
						if (MrmSUCCESS != MrmRegisterNamesInHierarchy(
							digitiser_window_hierarchy,identifiers,XtNumber(identifiers)))
						{
							XtDestroyWidget(digitiser_window->window_shell);
							DEALLOCATE(digitiser_window);
							display_message(ERROR_MESSAGE,"CREATE(digitiser_window).  "
								"Could not register the identifiers");
						}
						else
						{
							/* Get the digitiser window from the uid file */
							if (MrmSUCCESS != MrmFetchWidget(digitiser_window_hierarchy,
								"digitiser_window",digitiser_window->window_shell,
								&digitiser_window->main_window,&digitiser_window_dialog_class))
							{
								XtDestroyWidget(digitiser_window->window_shell);
								DEALLOCATE(digitiser_window);
								display_message(ERROR_MESSAGE,"CREATE(digitiser_window).  "
									"Could not retrieve the 3D window");
							}
							else
							{
								/* create the Scene_viewer */
								if (DIGITISER_WINDOW_SINGLE_BUFFER==buffer_mode)
								{
									scene_viewer_buffer_mode=SCENE_VIEWER_SINGLE_BUFFER;
								}
								else
								{
									scene_viewer_buffer_mode=SCENE_VIEWER_DOUBLE_BUFFER;
								}
								if (!(digitiser_window->scene_viewer=CREATE(Scene_viewer)(
									digitiser_window->viewing_area,
									background_colour,scene_viewer_buffer_mode,
									digitiser_window->light_manager,default_light,
									digitiser_window->light_model_manager,default_light_model,
									digitiser_window->scene_manager,view->scene,
									digitiser_window->texture_manager,
									digitiser_window->user_interface)))
								{
									XtDestroyWidget(digitiser_window->window_shell);
									DEALLOCATE(digitiser_window);
									display_message(ERROR_MESSAGE,"CREATE(digitiser_window).  "
										"Could not create Scene_viewer");
								}
								else
								{
									Scene_viewer_set_input_mode(digitiser_window->scene_viewer,
										SCENE_VIEWER_SELECT);
									Scene_viewer_set_viewport_info(digitiser_window->scene_viewer,
										view->NDC_left,view->NDC_bottom+view->NDC_height,1,1);
									/*view->image_pixel_size_x,view->image_pixel_size_y);*/
									Digitiser_window_set_view(digitiser_window,view_no);
									Digitiser_window_make_view_buttons(digitiser_window);
									/* deferred from above for OpenGL */
									XtManageChild(digitiser_window->main_window);
									/*XtRealizeWidget(digitiser_window->window_shell);*/
									XtPopup(digitiser_window->window_shell,XtGrabNone);
								}
							}
						}
					}
				}
				if (window_title)
				{
					DEALLOCATE(window_title);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"CREATE(digitiser_window).  "
					"Insufficient memory for digitiser window");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(digitiser_window).  Could not open hierarchy");
			digitiser_window=(struct Digitiser_window *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(digitiser_window).  Invalid argument(s)");
		digitiser_window=(struct Digitiser_window *)NULL;
	}
	LEAVE;

	return (digitiser_window);
} /* CREATE(digitiser_window) */

int DESTROY(Digitiser_window)(
	struct Digitiser_window **digitiser_window_address)
/*******************************************************************************
LAST MODIFIED : 18 November 1998

DESCRIPTION:
Frees the contents of the Digitiser_window structure and then the object itself,
then closes down the window shell and widgets it uses. Note that responsibility
for removing the digitiser_window from a global list of windows is left with the
calling routine. See also Digitiser_window_close_CB and
Digitiser_window_destroy_CB.
==============================================================================*/
{
	int return_code;
	struct Digitiser_window *digitiser_window;

	ENTER(DESTROY(digitiser_window));
	if (digitiser_window_address&&(digitiser_window= *digitiser_window_address))
	{
		DESTROY(Scene_viewer)(&(digitiser_window->scene_viewer));
		DEALLOCATE(digitiser_window->name);
		destroy_Shell_list_item_from_shell (&(digitiser_window->window_shell),
			digitiser_window->user_interface );
		DEALLOCATE(*digitiser_window_address);
		/* destroy the digitiser window widget */
		XtDestroyWidget(digitiser_window->window_shell);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(digitiser_window).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(digitiser_window) */

DECLARE_OBJECT_FUNCTIONS(Digitiser_window)
DECLARE_DEFAULT_GET_OBJECT_NAME_FUNCTION(Digitiser_window)

DECLARE_INDEXED_LIST_FUNCTIONS(Digitiser_window)
DECLARE_FIND_BY_IDENTIFIER_IN_INDEXED_LIST_FUNCTION(Digitiser_window, \
	name,char *,strcmp)

PROTOTYPE_MANAGER_COPY_WITH_IDENTIFIER_FUNCTION(Digitiser_window,name)
{
	char *name;
	int return_code;

	ENTER(MANAGER_COPY_WITH_IDENTIFIER(Digitiser_window,name));
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
					"MANAGER_COPY_WITH_IDENTIFIER(Digitiser_window,name).  "
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
			if (return_code = MANAGER_COPY_WITHOUT_IDENTIFIER(Digitiser_window,name)(
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
					"MANAGER_COPY_WITH_IDENTIFIER(Digitiser_window,name).  "
					"Could not copy without identifier");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_WITH_IDENTIFIER(Digitiser_window,name).  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITH_IDENTIFIER(Digitiser_window,name) */

PROTOTYPE_MANAGER_COPY_WITHOUT_IDENTIFIER_FUNCTION(Digitiser_window,name)
{
	int return_code;

	ENTER(MANAGER_COPY_WITHOUT_IDENTIFIER(Digitiser_window,name));
	if (source&&destination)
	{
		/*???RC have problems with copying scene_manager? messages? */
		printf("MANAGER_COPY_WITHOUT_IDENTIFIER(Digitiser_window,name).  "
			"Not used\n");
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_WITHOUT_IDENTIFIER(Digitiser_window,name).  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_WITHOUT_IDENTIFIER(Digitiser_window,name) */

PROTOTYPE_MANAGER_COPY_IDENTIFIER_FUNCTION(Digitiser_window,name,char *)
{
	char *destination_name;
	int return_code;

	ENTER(MANAGER_COPY_IDENTIFIER(Digitiser_window,name));
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
				"MANAGER_COPY_IDENTIFIER(Digitiser_window,name).  Insufficient memory");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"MANAGER_COPY_IDENTIFIER(Digitiser_window,name).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* MANAGER_COPY_IDENTIFIER(Digitiser_window,name) */

/* NOTE: Using special ADD_OBJECT_TO_MANAGER function so that object keeps */
/*       pointer to its manager while it is managed. */
DECLARE_MANAGER_FUNCTIONS(Digitiser_window)
DECLARE_OBJECT_WITH_MANAGER_MANAGER_IDENTIFIER_FUNCTIONS( \
	Digitiser_window,name,char *,digitiser_window_manager)

char *Digitiser_window_manager_get_new_name(
	struct MANAGER(Digitiser_window) *digitiser_window_manager)
/*******************************************************************************
LAST MODIFIED : 16 October 1998

DESCRIPTION :
Makes up a default name string for a digitiser window, based on numbers and
starting at "1". Up to the calling routine to deallocate the returned string.
==============================================================================*/
{
	char *return_name,temp_name[10];
	int number;

	ENTER(Digitiser_window_manager_get_new_name);
	if (digitiser_window_manager)
	{
		number=1;
		sprintf(temp_name,"%d",number);
		while (FIND_BY_IDENTIFIER_IN_MANAGER(Digitiser_window,name)(temp_name,
			digitiser_window_manager))
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
		"Digitiser_window_manager_get_new_name.  Missing digitiser_window_manager");
		return_name=(char *)NULL;
	}
	LEAVE;

	return (return_name);
} /* Digitiser_window_manager_get_new_name */

int Digitiser_window_add_light(struct Digitiser_window *digitiser_window,
	void *light_void)
/*******************************************************************************
LAST MODIFIED : 11 December 1997

DESCRIPTION :
Iterator function for adding a light to <digitiser_window>.
==============================================================================*/
{
	int return_code;
	struct Light *light;

	ENTER(Digitiser_window_add_light);
	if (digitiser_window&&(light=(struct Light *)light_void))
	{
		return_code=Scene_viewer_add_light(digitiser_window->scene_viewer,light);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Digitiser_window_add_light.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Digitiser_window_add_light */

int Digitiser_window_remove_light(struct Digitiser_window *digitiser_window,
	void *light_void)
/*******************************************************************************
LAST MODIFIED : 11 December 1997

DESCRIPTION :
Iterator function for removing a light from the <digitiser_window>.
==============================================================================*/
{
	int return_code;
	struct Light *light;

	ENTER(Digitiser_window_remove_light);
	if (digitiser_window&&(light=(struct Light *)light_void))
	{
		return_code=Scene_viewer_remove_light(digitiser_window->scene_viewer,light);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Digitiser_window_remove_light.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Digitiser_window_remove_light */

int Digitiser_window_set_light_model(struct Digitiser_window *digitiser_window,
	void *light_model_void)
/*******************************************************************************
LAST MODIFIED : 13 December 1997

DESCRIPTION :
Iterator function for setting the light_model for the <digitiser_window>.
==============================================================================*/
{
	int return_code;
	struct Light_model *light_model;

	ENTER(Digitiser_window_set_light_model);
	if (digitiser_window&&(light_model=(struct Light_model *)light_model_void))
	{
		return_code=
			Scene_viewer_set_light_model(digitiser_window->scene_viewer,light_model);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Digitiser_window_set_light_model.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Digitiser_window_set_light_model */

int Digitiser_window_update(struct Digitiser_window *digitiser_window)
/*******************************************************************************
LAST MODIFIED : 14 February 1998

DESCRIPTION :
Iterator function for forcing a redraw on <digitiser_window>.
==============================================================================*/
{
	int return_code;

	ENTER(Digitiser_window_update);
	if (digitiser_window)
	{
		Scene_viewer_redraw(digitiser_window->scene_viewer);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Digitiser_window_update.  Missing digitiser window");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Digitiser_window_update */

int Digitiser_window_update_now(struct Digitiser_window *digitiser_window)
/*******************************************************************************
LAST MODIFIED : 14 February 1998

DESCRIPTION :
Iterator function for forcing a redraw on <digitiser_window>.
==============================================================================*/
{
	int return_code;

	ENTER(Digitiser_window_update_now);
	if (digitiser_window)
	{
		Scene_viewer_redraw_now(digitiser_window->scene_viewer);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Digitiser_window_update_now.  Missing digitiser window");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Digitiser_window_update_now */

int Digitiser_window_update_view(struct Digitiser_window *digitiser_window,
	void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 21 June 1999

DESCRIPTION :
Iterator function for forcing a redraw on <digitiser_window> AFTER ensuring the
current view is properly re-established - must call this for all digitiser
windows after changing or re-reading frames.
==============================================================================*/
{
	int return_code;

	ENTER(Digitiser_window_update_view);
	USE_PARAMETER(dummy_void);
	if (digitiser_window)
	{
		return_code=
			Digitiser_window_set_view(digitiser_window,digitiser_window->view_no);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Digitiser_window_update_view.  Missing digitiser window");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Digitiser_window_update_view */
