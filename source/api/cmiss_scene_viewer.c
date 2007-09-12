/*******************************************************************************
FILE : cmiss_scene_viewer.c

LAST MODIFIED : 2 June 2004

DESCRIPTION :
The public interface to the Cmiss_scene_viewer object for rendering cmiss
scenes.
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
#include <stdarg.h>
#if defined (GTK_USER_INTERFACE)
#include <gtk/gtk.h>
#endif /* defined (GTK_USER_INTERFACE) */
#include "api/cmiss_scene_viewer.h"
#include "api/cmiss_scene_viewer_private.h"
#include "general/debug.h"
#include "graphics/scene_viewer.h"
#include "graphics/transform_tool.h"
#include "three_d_drawing/graphics_buffer.h"
#include "user_interface/message.h"

/*
Global functions
----------------
*/

#if defined (GTK_USER_INTERFACE)
Cmiss_scene_viewer_id create_Cmiss_scene_viewer_gtk(
	struct Cmiss_scene_viewer_package *cmiss_scene_viewer_package,
	GtkContainer *scene_viewer_widget,
	enum Cmiss_scene_viewer_buffering_mode buffer_mode,
	enum Cmiss_scene_viewer_stereo_mode stereo_mode,
	int minimum_colour_buffer_depth, int minimum_depth_buffer_depth,
	int minimum_accumulation_buffer_depth)
/*******************************************************************************
LAST MODIFIED : 19 January 2007

DESCRIPTION :
Creates a Cmiss_scene_viewer by creating a GtkGlArea inside the specified 
<scene_viewer_widget> container.
If <minimum_colour_buffer_depth>, <minimum_depth_buffer_depth> or 
<minimum_accumulation_buffer_depth> are not zero then they are used to filter
out the possible visuals selected for graphics_buffers.  If they are zero then 
the accumulation_buffer_depth are not tested and the maximum colour buffer depth is
chosen.
==============================================================================*/
{
	enum Graphics_buffer_buffering_mode graphics_buffer_buffering_mode;
	enum Graphics_buffer_stereo_mode graphics_buffer_stereo_mode;
	struct Graphics_buffer *graphics_buffer;
	struct Cmiss_scene_viewer *scene_viewer;

	ENTER(create_Cmiss_scene_viewer_gtk);
	/* Not implemented yet */
	USE_PARAMETER(minimum_colour_buffer_depth);
	USE_PARAMETER(minimum_accumulation_buffer_depth);
	USE_PARAMETER(minimum_depth_buffer_depth);
	if (cmiss_scene_viewer_package)
	{
		if (CMISS_SCENE_VIEWER_BUFFERING_ANY_MODE==buffer_mode)
		{
			graphics_buffer_buffering_mode = GRAPHICS_BUFFER_ANY_BUFFERING_MODE;
		}
		else if (CMISS_SCENE_VIEWER_BUFFERING_SINGLE==buffer_mode)
		{
			graphics_buffer_buffering_mode = GRAPHICS_BUFFER_SINGLE_BUFFERING;
		}
		else
		{
			graphics_buffer_buffering_mode = GRAPHICS_BUFFER_DOUBLE_BUFFERING;
		}
		if (CMISS_SCENE_VIEWER_STEREO_ANY_MODE==stereo_mode)
		{
			graphics_buffer_stereo_mode = GRAPHICS_BUFFER_ANY_STEREO_MODE;
		}
		else if (CMISS_SCENE_VIEWER_STEREO_STEREO==stereo_mode)
		{
			graphics_buffer_stereo_mode = GRAPHICS_BUFFER_STEREO;
		}
		else
		{
			graphics_buffer_stereo_mode = GRAPHICS_BUFFER_MONO;
		}
		graphics_buffer = create_Graphics_buffer_gtkgl(
			Cmiss_scene_viewer_package_get_graphics_buffer_package(cmiss_scene_viewer_package),
			scene_viewer_widget,
			graphics_buffer_buffering_mode, graphics_buffer_stereo_mode,
			minimum_colour_buffer_depth, minimum_depth_buffer_depth,
			minimum_accumulation_buffer_depth);
		scene_viewer = create_Scene_viewer_from_package(graphics_buffer,
			cmiss_scene_viewer_package,
			Cmiss_scene_viewer_package_get_default_scene(cmiss_scene_viewer_package));
	}
	else
	{
		display_message(ERROR_MESSAGE,"create_Cmiss_scene_viewer_gtk.  "
			"The Cmiss_scene_viewer data must be initialised before any scene "
			"viewers can be created.");
		scene_viewer=(struct Cmiss_scene_viewer *)NULL;
	}
	LEAVE;

	return (scene_viewer);
} /* create_Cmiss_scene_viewer_gtk */
#endif /* create_Cmiss_scene_viewer_gtk */

#if defined (CARBON_USER_INTERFACE)
Cmiss_scene_viewer_id create_Cmiss_scene_viewer_Carbon(
	struct Cmiss_scene_viewer_package *cmiss_scene_viewer_package,
	CGrafPtr port, int port_x, int port_y,
	enum Cmiss_scene_viewer_buffering_mode buffer_mode,
	enum Cmiss_scene_viewer_stereo_mode stereo_mode,
	int minimum_colour_buffer_depth, int minimum_depth_buffer_depth,
	int minimum_accumulation_buffer_depth)
/*******************************************************************************
LAST MODIFIED : 14 February 2007

DESCRIPTION :
Creates a Cmiss_scene_viewer by creating a graphics buffer on the specified 
<port> window handle.
If <minimum_colour_buffer_depth>, <minimum_depth_buffer_depth> or 
<minimum_accumulation_buffer_depth> are not zero then they are used to filter
out the possible visuals selected for graphics_buffers.  If they are zero then 
the accumulation_buffer_depth are not tested and the maximum colour buffer depth is
chosen.
==============================================================================*/
{
	enum Graphics_buffer_buffering_mode graphics_buffer_buffering_mode;
	enum Graphics_buffer_stereo_mode graphics_buffer_stereo_mode;
	struct Graphics_buffer *graphics_buffer;
	struct Cmiss_scene_viewer *scene_viewer;

	ENTER(create_Cmiss_scene_viewer_Carbon);
	/* Not implemented yet */
	USE_PARAMETER(minimum_colour_buffer_depth);
	USE_PARAMETER(minimum_accumulation_buffer_depth);
	USE_PARAMETER(minimum_depth_buffer_depth);
	if (cmiss_scene_viewer_package)
	{
		if (CMISS_SCENE_VIEWER_BUFFERING_ANY_MODE==buffer_mode)
		{
			graphics_buffer_buffering_mode = GRAPHICS_BUFFER_ANY_BUFFERING_MODE;
		}
		else if (CMISS_SCENE_VIEWER_BUFFERING_SINGLE==buffer_mode)
		{
			graphics_buffer_buffering_mode = GRAPHICS_BUFFER_SINGLE_BUFFERING;
		}
		else
		{
			graphics_buffer_buffering_mode = GRAPHICS_BUFFER_DOUBLE_BUFFERING;
		}
		if (CMISS_SCENE_VIEWER_STEREO_ANY_MODE==stereo_mode)
		{
			graphics_buffer_stereo_mode = GRAPHICS_BUFFER_ANY_STEREO_MODE;
		}
		else if (CMISS_SCENE_VIEWER_STEREO_STEREO==stereo_mode)
		{
			graphics_buffer_stereo_mode = GRAPHICS_BUFFER_STEREO;
		}
		else
		{
			graphics_buffer_stereo_mode = GRAPHICS_BUFFER_MONO;
		}
		graphics_buffer = create_Graphics_buffer_Carbon(
			Cmiss_scene_viewer_package_get_graphics_buffer_package(cmiss_scene_viewer_package),
			port, port_x, port_y,
			graphics_buffer_buffering_mode, graphics_buffer_stereo_mode,
			minimum_colour_buffer_depth, minimum_depth_buffer_depth,
			minimum_accumulation_buffer_depth);
		scene_viewer = create_Scene_viewer_from_package(graphics_buffer,
			cmiss_scene_viewer_package,
			Cmiss_scene_viewer_package_get_default_scene(cmiss_scene_viewer_package));
	}
	else
	{
		display_message(ERROR_MESSAGE,"create_Cmiss_scene_viewer_Carbon.  "
			"The Cmiss_scene_viewer data must be initialised before any scene "
			"viewers can be created.");
		scene_viewer=(struct Cmiss_scene_viewer *)NULL;
	}
	LEAVE;

	return (scene_viewer);
}
#endif /* defined (CARBON_USER_INTERFACE) */

#if defined (WIN32_USER_INTERFACE)
Cmiss_scene_viewer_id create_Cmiss_scene_viewer_win32(
	struct Cmiss_scene_viewer_package *cmiss_scene_viewer_package,
	HWND hWnd, HDC hDC,
	enum Cmiss_scene_viewer_buffering_mode buffer_mode,
	enum Cmiss_scene_viewer_stereo_mode stereo_mode,
	int minimum_colour_buffer_depth, int minimum_depth_buffer_depth,
	int minimum_accumulation_buffer_depth)
/*******************************************************************************
LAST MODIFIED : 1 June 2007

DESCRIPTION :
Creates a Cmiss_scene_viewer by creating a graphics buffer on the specified 
<hWnd> window handle.
If <minimum_colour_buffer_depth>, <minimum_depth_buffer_depth> or 
<minimum_accumulation_buffer_depth> are not zero then they are used to filter
out the possible visuals selected for graphics_buffers.  If they are zero then 
the accumulation_buffer_depth are not tested and the maximum colour buffer depth is
chosen.
==============================================================================*/
{
	enum Graphics_buffer_buffering_mode graphics_buffer_buffering_mode;
	enum Graphics_buffer_stereo_mode graphics_buffer_stereo_mode;
	struct Graphics_buffer *graphics_buffer;
	struct Cmiss_scene_viewer *scene_viewer;

	ENTER(create_Cmiss_scene_viewer_win32);
	if (cmiss_scene_viewer_package)
	{
		if (CMISS_SCENE_VIEWER_BUFFERING_ANY_MODE==buffer_mode)
		{
			graphics_buffer_buffering_mode = GRAPHICS_BUFFER_ANY_BUFFERING_MODE;
		}
		else if (CMISS_SCENE_VIEWER_BUFFERING_SINGLE==buffer_mode)
		{
			graphics_buffer_buffering_mode = GRAPHICS_BUFFER_SINGLE_BUFFERING;
		}
		else
		{
			graphics_buffer_buffering_mode = GRAPHICS_BUFFER_DOUBLE_BUFFERING;
		}
		if (CMISS_SCENE_VIEWER_STEREO_ANY_MODE==stereo_mode)
		{
			graphics_buffer_stereo_mode = GRAPHICS_BUFFER_ANY_STEREO_MODE;
		}
		else if (CMISS_SCENE_VIEWER_STEREO_STEREO==stereo_mode)
		{
			graphics_buffer_stereo_mode = GRAPHICS_BUFFER_STEREO;
		}
		else
		{
			graphics_buffer_stereo_mode = GRAPHICS_BUFFER_MONO;
		}
		graphics_buffer = create_Graphics_buffer_win32(
			Cmiss_scene_viewer_package_get_graphics_buffer_package(cmiss_scene_viewer_package),
			hWnd, hDC,
			graphics_buffer_buffering_mode, graphics_buffer_stereo_mode,
			minimum_colour_buffer_depth, minimum_depth_buffer_depth,
			minimum_accumulation_buffer_depth);
		scene_viewer = create_Scene_viewer_from_package(graphics_buffer,
			cmiss_scene_viewer_package,
			Cmiss_scene_viewer_package_get_default_scene(cmiss_scene_viewer_package));
	}
	else
	{
		display_message(ERROR_MESSAGE,"create_Cmiss_scene_viewer_win32.  "
			"The Cmiss_scene_viewer data must be initialised before any scene "
			"viewers can be created.");
		scene_viewer=(struct Cmiss_scene_viewer *)NULL;
	}
	LEAVE;

	return (scene_viewer);
}
#endif /* defined (WIN32_USER_INTERFACE) */

#if defined (WIN32_USER_INTERFACE)
int Cmiss_scene_viewer_handle_windows_event(Cmiss_scene_viewer_id scene_viewer,
	UINT event,WPARAM first_message,LPARAM second_message)
/*******************************************************************************
LAST MODIFIED : 31 May 2007

DESCRIPTION:
Passes the supplied windows event on to the graphics buffer.
==============================================================================*/
{
   int return_code;
	struct Graphics_buffer *graphics_buffer;

	ENTER(create_Cmiss_scene_viewer_win32);
	if (scene_viewer)
	{
	  graphics_buffer = Scene_viewer_get_graphics_buffer(scene_viewer);
	  return_code = Graphics_buffer_handle_windows_event(graphics_buffer,
		 event, first_message, second_message);
	}
	else
	{
		display_message(ERROR_MESSAGE,"create_Cmiss_scene_viewer_win32.  "
			"Scene viewer required.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}
#endif /* defined (WIN32_USER_INTERFACE) */

#if defined (MOTIF)
Cmiss_scene_viewer_id create_Cmiss_scene_viewer_motif(
	struct Cmiss_scene_viewer_package *cmiss_scene_viewer_package,
	Widget parent,
	enum Cmiss_scene_viewer_buffering_mode buffer_mode,
	enum Cmiss_scene_viewer_stereo_mode stereo_mode,
	int minimum_colour_buffer_depth, int minimum_depth_buffer_depth,
	int minimum_accumulation_buffer_depth)
/*******************************************************************************
LAST MODIFIED : 25 January 2006

DESCRIPTION :
Creates a Cmiss_scene_viewer by creating a graphics buffer on the specified 
<parent> widget.
If <minimum_colour_buffer_depth>, <minimum_depth_buffer_depth> or 
<minimum_accumulation_buffer_depth> are not zero then they are used to filter
out the possible visuals selected for graphics_buffers.  If they are zero then 
the accumulation_buffer_depth are not tested and the maximum colour buffer depth is
chosen.
==============================================================================*/
{
	Dimension height,width;
	enum Graphics_buffer_buffering_mode graphics_buffer_buffering_mode;
	enum Graphics_buffer_stereo_mode graphics_buffer_stereo_mode;
	struct Graphics_buffer *graphics_buffer;
	struct Cmiss_scene_viewer *scene_viewer;

	ENTER(create_Cmiss_scene_viewer_x11);
	if (cmiss_scene_viewer_package)
	{
		if (CMISS_SCENE_VIEWER_BUFFERING_ANY_MODE==buffer_mode)
		{
			graphics_buffer_buffering_mode = GRAPHICS_BUFFER_ANY_BUFFERING_MODE;
		}
		else if (CMISS_SCENE_VIEWER_BUFFERING_SINGLE==buffer_mode)
		{
			graphics_buffer_buffering_mode = GRAPHICS_BUFFER_SINGLE_BUFFERING;
		}
		else
		{
			graphics_buffer_buffering_mode = GRAPHICS_BUFFER_DOUBLE_BUFFERING;
		}
		if (CMISS_SCENE_VIEWER_STEREO_ANY_MODE==stereo_mode)
		{
			graphics_buffer_stereo_mode = GRAPHICS_BUFFER_ANY_STEREO_MODE;
		}
		else if (CMISS_SCENE_VIEWER_STEREO_STEREO==stereo_mode)
		{
			graphics_buffer_stereo_mode = GRAPHICS_BUFFER_STEREO;
		}
		else
		{
			graphics_buffer_stereo_mode = GRAPHICS_BUFFER_MONO;
		}
		XtVaGetValues(parent, XmNwidth,&width, XmNheight,&height,NULL);
		graphics_buffer = create_Graphics_buffer_X3d(
			Cmiss_scene_viewer_package_get_graphics_buffer_package(cmiss_scene_viewer_package),
			parent, width, height,
			graphics_buffer_buffering_mode, graphics_buffer_stereo_mode,
			minimum_colour_buffer_depth, minimum_depth_buffer_depth,
			minimum_accumulation_buffer_depth);
		scene_viewer = create_Scene_viewer_from_package(graphics_buffer,
			cmiss_scene_viewer_package,
			Cmiss_scene_viewer_package_get_default_scene(cmiss_scene_viewer_package));
	}
	else
	{
		display_message(ERROR_MESSAGE,"create_Cmiss_scene_viewer_x11.  "
			"The Cmiss_scene_viewer data must be initialised before any scene "
			"viewers can be created.");
		scene_viewer=(struct Cmiss_scene_viewer *)NULL;
	}
	LEAVE;

	return (scene_viewer);
}
#endif /* defined (MOTIF) */

#if defined (NEW_CODE)
Cmiss_scene_viewer_id create_Cmiss_scene_viewer_X11(
	struct Cmiss_scene_viewer_package *cmiss_scene_viewer_package,
	Window window,
	enum Cmiss_scene_viewer_buffering_mode buffer_mode,
	enum Cmiss_scene_viewer_stereo_mode stereo_mode,
	int minimum_colour_buffer_depth, int minimum_depth_buffer_depth,
	int minimum_accumulation_buffer_depth)
/*******************************************************************************
LAST MODIFIED : 25 January 2006

DESCRIPTION :
Creates a Cmiss_scene_viewer by creating a graphics buffer on the specified 
<window>.
If <minimum_colour_buffer_depth>, <minimum_depth_buffer_depth> or 
<minimum_accumulation_buffer_depth> are not zero then they are used to filter
out the possible visuals selected for graphics_buffers.  If they are zero then 
the accumulation_buffer_depth are not tested and the maximum colour buffer depth is
chosen.
==============================================================================*/
{
	enum Graphics_buffer_buffering_mode graphics_buffer_buffering_mode;
	enum Graphics_buffer_stereo_mode graphics_buffer_stereo_mode;
	struct Graphics_buffer *graphics_buffer;
	struct Cmiss_scene_viewer *scene_viewer;
	Window root;
	int x, y;
	unsigned int height, width, border_width, depth;

	ENTER(create_Cmiss_scene_viewer_x11);
	if (cmiss_scene_viewer_package)
	{
		if (CMISS_SCENE_VIEWER_BUFFERING_ANY_MODE==buffer_mode)
		{
			graphics_buffer_buffering_mode = GRAPHICS_BUFFER_ANY_BUFFERING_MODE;
		}
		else if (CMISS_SCENE_VIEWER_BUFFERING_SINGLE==buffer_mode)
		{
			graphics_buffer_buffering_mode = GRAPHICS_BUFFER_SINGLE_BUFFERING;
		}
		else
		{
			graphics_buffer_buffering_mode = GRAPHICS_BUFFER_DOUBLE_BUFFERING;
		}
		if (CMISS_SCENE_VIEWER_STEREO_ANY_MODE==stereo_mode)
		{
			graphics_buffer_stereo_mode = GRAPHICS_BUFFER_ANY_STEREO_MODE;
		}
		else if (CMISS_SCENE_VIEWER_STEREO_STEREO==stereo_mode)
		{
			graphics_buffer_stereo_mode = GRAPHICS_BUFFER_STEREO;
		}
		else
		{
			graphics_buffer_stereo_mode = GRAPHICS_BUFFER_MONO;
		}
		XGetGeometry(User_interface_get_display(cmiss_scene_viewer_package->user_interface),
				window, &root, &x, &y, &width, &height, &border_width, &depth); 
		graphics_buffer = create_Graphics_buffer_X11(
			Cmiss_scene_viewer_package_get_graphics_buffer_package(cmiss_scene_viewer_package),
			window,
			width, height,
			graphics_buffer_buffering_mode, graphics_buffer_stereo_mode,
			minimum_colour_buffer_depth, minimum_depth_buffer_depth,
			minimum_accumulation_buffer_depth);
		scene_viewer = create_Scene_viewer_from_package(graphics_buffer,
			cmiss_scene_viewer_package,
			Cmiss_scene_viewer_package_get_default_scene(cmiss_scene_viewer_package));
	}
	else
	{
		display_message(ERROR_MESSAGE,"create_Cmiss_scene_viewer_x11.  "
			"The Cmiss_scene_viewer data must be initialised before any scene "
			"viewers can be created.");
		scene_viewer=(struct Cmiss_scene_viewer *)NULL;
	}
	LEAVE;

	return (scene_viewer);
}
#endif /* defined (NEW_CODE) */

int Cmiss_scene_viewer_destroy(Cmiss_scene_viewer_id *scene_viewer_id_address)
/*******************************************************************************
LAST MODIFIED : 4 September 2007

DESCRIPTION :
Closes the scene_viewer.
==============================================================================*/
{
	/* The normal destroy will call the Scene_viewer_package callback
		to remove it from the package */
	return (DESTROY(Scene_viewer)(scene_viewer_id_address));
}

int Cmiss_scene_viewer_get_near_and_far_plane(Cmiss_scene_viewer_id scene_viewer,
	double *near_plane, double *far_plane)
/*******************************************************************************
LAST MODIFIED : 13 September 2002

DESCRIPTION :
Gets the distance from the eye_point to the <near> clip plane and to the <far>
clip plane in the <scene_viewer>.
==============================================================================*/
{
	double left, right, bottom, top;
	int return_code;

	ENTER(Cmiss_scene_viewer_get_near_and_far_plane);
	if (scene_viewer)
	{
		return_code = Scene_viewer_get_viewing_volume(scene_viewer,
		  &left, &right, &bottom, &top, near_plane, far_plane);
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_scene_viewer_get_near_and_far_plane.  "
			"Missing scene_viewer parameter.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_scene_viewer_get_near_and_far_plane */

int Cmiss_scene_viewer_set_near_and_far_plane(Cmiss_scene_viewer_id scene_viewer,
	double near_plane, double far_plane)
/*******************************************************************************
LAST MODIFIED : 13 September 2002

DESCRIPTION :
Sets the distance from the eye_point to the <near> clip plane and to the <far>
clip plane in the <scene_viewer>.
==============================================================================*/
{
	double left, right, bottom, top, old_near, old_far;
	int return_code;

	ENTER(Cmiss_scene_viewer_set_near_and_far_plane);
	if (scene_viewer)
	{
		if (Scene_viewer_get_viewing_volume(scene_viewer,
			&left, &right, &bottom, &top, &old_near, &old_far))
		{
			return_code = Scene_viewer_set_viewing_volume(scene_viewer,
				left, right, bottom, top, near_plane, far_plane);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_scene_viewer_get_near_and_far_plane.  "
			"Missing scene_viewer parameter.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_scene_viewer_get_near_and_far_plane */

int Cmiss_scene_viewer_get_viewport_mode(Cmiss_scene_viewer_id scene_viewer,
	enum Cmiss_scene_viewer_viewport_mode *viewport_mode)
/*******************************************************************************
LAST MODIFIED : 04 February 2005

DESCRIPTION :
Gets the viewport mode(absolute/relative/distorting relative) for the
<scene_viewer>.
==============================================================================*/
{
	enum Scene_viewer_viewport_mode scene_viewer_viewport_mode;
	int return_code;

	ENTER(Cmiss_scene_viewer_get_viewport_mode);
	if (scene_viewer)
	{
		scene_viewer_viewport_mode = Scene_viewer_get_viewport_mode(scene_viewer);
		switch(scene_viewer_viewport_mode)
		{
			case SCENE_VIEWER_ABSOLUTE_VIEWPORT:
			{
				*viewport_mode = CMISS_SCENE_VIEWER_VIEWPORT_ABSOLUTE;
				return_code = 1;
			} break;
			case SCENE_VIEWER_RELATIVE_VIEWPORT:
			{
				*viewport_mode = CMISS_SCENE_VIEWER_VIEWPORT_RELATIVE;
				return_code = 1;
			} break;
			case SCENE_VIEWER_DISTORTING_RELATIVE_VIEWPORT:
			{
				*viewport_mode = CMISS_SCENE_VIEWER_VIEWPORT_DISTORTING_RELATIVE;
				return_code = 1;
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_scene_viewer_get_viewport_mode.  "
					"Viewport mode not supported in public interface.");
				return_code = 0;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_scene_viewer_get_viewport_mode.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_scene_viewer_get_viewport_mode */

int Cmiss_scene_viewer_set_viewport_mode(Cmiss_scene_viewer_id scene_viewer,
	enum Cmiss_scene_viewer_viewport_mode viewport_mode)
/*******************************************************************************
LAST MODIFIED : 04 February 2005

DESCRIPTION :
Sets the viewport mode(absolute/relative/distorting relative) for the
<scene_viewer>.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_scene_viewer_set_viewport_mode);
	if (scene_viewer)
	{
		switch(viewport_mode)
		{
			case CMISS_SCENE_VIEWER_VIEWPORT_ABSOLUTE:
			{
				return_code = Scene_viewer_set_viewport_mode(scene_viewer, 
					SCENE_VIEWER_ABSOLUTE_VIEWPORT);
			} break;
			case CMISS_SCENE_VIEWER_VIEWPORT_RELATIVE:
			{
				return_code = Scene_viewer_set_viewport_mode(scene_viewer, 
					SCENE_VIEWER_RELATIVE_VIEWPORT);
			} break;
			case CMISS_SCENE_VIEWER_VIEWPORT_DISTORTING_RELATIVE:
			{
				return_code = Scene_viewer_set_viewport_mode(scene_viewer, 
					SCENE_VIEWER_DISTORTING_RELATIVE_VIEWPORT);
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_scene_viewer_set_viewport_mode.  "
					"Unknown viewport mode.");
				return_code = 0;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_scene_viewer_set_viewport_mode.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_scene_viewer_set_viewport_mode */

int Cmiss_scene_viewer_get_projection_mode(Cmiss_scene_viewer_id scene_viewer,
	enum Cmiss_scene_viewer_projection_mode *projection_mode)
/*******************************************************************************
LAST MODIFIED : 17 September 2002

DESCRIPTION :
Returns the projection mode - parallel/perspective - of the Cmiss_scene_viewer.
==============================================================================*/
{
	int return_code;
	enum Scene_viewer_projection_mode scene_viewer_projection_mode;

	ENTER(Cmiss_scene_viewer_get_projection_mode);
	if (scene_viewer)
	{
		return_code = Scene_viewer_get_projection_mode(scene_viewer,
			&scene_viewer_projection_mode);
		if (return_code)
		{
			switch(scene_viewer_projection_mode)
			{
				case SCENE_VIEWER_PERSPECTIVE:
				{
					*projection_mode = CMISS_SCENE_VIEWER_PROJECTION_PERSPECTIVE;
					return_code = 1;
				} break;
				case SCENE_VIEWER_PARALLEL:
				{
					*projection_mode = CMISS_SCENE_VIEWER_PROJECTION_PARALLEL;
					return_code = 1;
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"Cmiss_scene_viewer_set_projection_mode.  "
						"Projection mode not supported in public interface.");
					return_code = 0;
				} break;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_scene_viewer_get_projection_mode.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_scene_viewer_get_projection_mode */

int Cmiss_scene_viewer_set_projection_mode(Cmiss_scene_viewer_id scene_viewer,
	enum Cmiss_scene_viewer_projection_mode projection_mode)
/*******************************************************************************
LAST MODIFIED : 17 September 2002

DESCRIPTION :
Sets the projection mode - parallel/perspective/custom - of the Cmiss_scene_viewer.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_scene_viewer_set_projection_mode);
	if (scene_viewer)
	{
		switch(projection_mode)
		{
			case CMISS_SCENE_VIEWER_PROJECTION_PERSPECTIVE:
			{
				return_code = Scene_viewer_set_projection_mode(scene_viewer, 
					SCENE_VIEWER_PERSPECTIVE);
			} break;
			case CMISS_SCENE_VIEWER_PROJECTION_PARALLEL:
			{
				return_code = Scene_viewer_set_projection_mode(scene_viewer, 
					SCENE_VIEWER_PARALLEL);
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_scene_viewer_set_projection_mode.  "
					"Unknown projection mode.");
				return_code = 0;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_scene_viewer_set_projection_mode.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_scene_viewer_set_projection_mode */

int Cmiss_scene_viewer_get_background_colour_r_g_b(
	Cmiss_scene_viewer_id scene_viewer, double *red, double *green, double *blue)
/*******************************************************************************
LAST MODIFIED : 15 January 2007

DESCRIPTION :
Returns the background_colour of the scene_viewer.
==============================================================================*/
{
	int return_code;
	struct Colour colour;

	ENTER(Cmiss_scene_viewer_set_background_colour_rgb);
	if (scene_viewer)
	{
		if (return_code = Scene_viewer_get_background_colour(scene_viewer, &colour))
		{
			*red = colour.red;
			*green = colour.green;
			*blue = colour.blue;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_scene_viewer_get_background_colour_rgb.  "
			"Missing scene_viewer parameter.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_scene_viewer_get_background_colour_rgb */

int Cmiss_scene_viewer_set_background_colour_r_g_b(
	Cmiss_scene_viewer_id scene_viewer, double red, double green, double blue)
/*******************************************************************************
LAST MODIFIED : 15 January 2007

DESCRIPTION :
Sets the background_colour of the scene_viewer.
==============================================================================*/
{
	int return_code;
	struct Colour colour;

	ENTER(Cmiss_scene_viewer_set_background_colour_rgb);
	if (scene_viewer)
	{
		colour.red = red;
		colour.green = green;
		colour.blue = blue;
		return_code = Scene_viewer_set_background_colour(scene_viewer, &colour);
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_scene_viewer_set_background_colour_rgb.  "
			"Missing scene_viewer parameter.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_scene_viewer_set_background_colour_rgb */

int Cmiss_scene_viewer_get_interactive_tool_name(
	Cmiss_scene_viewer_id scene_viewer, char **tool_name)
/*******************************************************************************
LAST MODIFIED : 11 September 2002

DESCRIPTION :
Returns an ALLOCATED string which specifies the name of the current
interactive_tool.  You should call Cmiss_deallocate with the returned
pointer when it is no longer required.
==============================================================================*/
{
	struct Interactive_tool *interactive_tool;
	int return_code;

	ENTER(Cmiss_scene_viewer_get_interactive_tool_name);
	if (scene_viewer)
	{
		if ((interactive_tool = Scene_viewer_get_interactive_tool(scene_viewer))
			&&(GET_NAME(Interactive_tool)(interactive_tool, tool_name)))
		{
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,"Cmiss_scene_viewer_get_interactive_tool_name.  "
				"Failed to get the tool or tool name.");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_scene_viewer_get_background_colour_rgb.  "
			"Missing scene_viewer parameter.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_scene_viewer_get_interactive_tool_name */

int Cmiss_scene_viewer_get_scene_name(
	Cmiss_scene_viewer_id scene_viewer, char **scene_name)
/*******************************************************************************
LAST MODIFIED : 13 September 2002

DESCRIPTION :
Returns an ALLOCATED string which identifies the scene currently rendered
by the <scene_viewer>.  You should call Cmiss_deallocate with the returned
pointer when it is no longer required.
==============================================================================*/
{
	struct Scene *scene;
	int return_code;

	ENTER(Cmiss_scene_viewer_get_scene_name);
	if (scene_viewer)
	{
		if ((scene = Scene_viewer_get_scene(scene_viewer))
			&&(GET_NAME(Scene)(scene, scene_name)))
		{
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,"Cmiss_scene_viewer_get_scene_name.  "
				"Failed to get the scene or scene name.");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_scene_viewer_get_background_colour_rgb.  "
			"Missing scene_viewer parameter.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_scene_viewer_get_scene_name */

int Cmiss_scene_viewer_write_image_to_file(Cmiss_scene_viewer_id scene_viewer,
	const char *file_name, int force_onscreen, int preferred_width, int preferred_height,
	int preferred_antialias, int preferred_transparency_layers)
/*******************************************************************************
LAST MODIFIED : 18 September 2002

DESCRIPTION :
Writes the view in the scene_viewer to the specified filename.
==============================================================================*/
{
	enum Texture_storage_type storage;
	int return_code;
	struct Cmgui_image *cmgui_image;
	struct Cmgui_image_information *cmgui_image_information;

	ENTER(Cmiss_scene_viewer_write_image_to_file);
	if (scene_viewer && file_name)
	{
		storage = TEXTURE_RGBA;
		if (cmgui_image = Scene_viewer_get_image(scene_viewer,
			force_onscreen, preferred_width, preferred_height, preferred_antialias,
			preferred_transparency_layers, storage))
		{
			cmgui_image_information = CREATE(Cmgui_image_information)();
			Cmgui_image_information_add_file_name(cmgui_image_information,
				(char *)file_name);
			Cmgui_image_write(cmgui_image, cmgui_image_information);
			DESTROY(Cmgui_image_information)(&cmgui_image_information);
			DESTROY(Cmgui_image)(&cmgui_image);
			return_code = 1;
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_scene_viewer_write_image_to_file.  "
			"Invalid scene_viewer or file name.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_scene_viewer_write_image_to_file */

int Cmiss_scene_viewer_get_NDC_info(Cmiss_scene_viewer_id scene_viewer,
	double *NDC_left,double *NDC_top,double *NDC_width,double *NDC_height)
/*******************************************************************************
LAST MODIFIED : 15 November 2005

DESCRIPTION :
Gets the NDC information.
==============================================================================*/
{
	return Scene_viewer_get_NDC_info(scene_viewer, NDC_left, NDC_top,
		NDC_width, NDC_height);
}

int Cmiss_scene_viewer_set_NDC_info(Cmiss_scene_viewer_id scene_viewer,
	double NDC_left,double NDC_top,double NDC_width,double NDC_height)
/*******************************************************************************
LAST MODIFIED : 15 November 2005

DESCRIPTION :
Gets the NDC information.
==============================================================================*/
{
	return Scene_viewer_set_NDC_info(scene_viewer, NDC_left, NDC_top,
		NDC_width, NDC_height);
}

int Cmiss_scene_viewer_get_frame_pixels(Cmiss_scene_viewer_id scene_viewer,
	enum Cmiss_texture_storage_type storage, int *width, int *height,
	int preferred_antialias, int preferred_transparency_layers,
	unsigned char **frame_data, int force_onscreen)
/*******************************************************************************
LAST MODIFIED : 18 September 2002

DESCRIPTION :
Returns the contents of the scene viewer as pixels.  <width> and <height>
will be respected if the window is drawn offscreen and they are non zero,
otherwise they are set in accordance with current size of the scene viewer.
If <preferred_antialias> or <preferred_transparency_layers> are non zero then they
attempt to override the default values for just this call.
If <force_onscreen> is non zero then the pixels will always be grabbed from the
scene viewer on screen.
==============================================================================*/
{
	enum Texture_storage_type internal_storage_type;
	int return_code;
	switch(storage)
	{
		case CMISS_TEXTURE_LUMINANCE:
		{
			internal_storage_type = TEXTURE_LUMINANCE;
		} break;
		case CMISS_TEXTURE_LUMINANCE_ALPHA:
		{
			internal_storage_type = TEXTURE_LUMINANCE_ALPHA;
		} break;
		case CMISS_TEXTURE_RGB:
		{
			internal_storage_type = TEXTURE_RGB;
		} break;
		case CMISS_TEXTURE_RGBA:
		{
			internal_storage_type = TEXTURE_RGBA;
		} break;
		case CMISS_TEXTURE_ABGR:
		{
			internal_storage_type = TEXTURE_ABGR;
		} break;
		case CMISS_TEXTURE_BGR:
		{
			internal_storage_type = TEXTURE_BGR;
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_scene_viewer_get_frame_pixels.  "
				"Unknown storage mode.");
			return_code = 0;
		} break;
	}
	if (return_code)
	{
		return_code = Scene_viewer_get_frame_pixels(
			scene_viewer,
			internal_storage_type, width, height, preferred_antialias,
			preferred_transparency_layers, frame_data, force_onscreen);
	}
	return (return_code);
}

int Cmiss_scene_viewer_default_input_callback(struct Scene_viewer *scene_viewer,
	struct Graphics_buffer_input *input)
/*******************************************************************************
LAST MODIFIED : 11 September 2007

DESCRIPTION :
The callback for mouse or keyboard input in the Scene_viewer window. The
resulting behaviour depends on the <scene_viewer> input_mode. In Transform mode
mouse clicks and drags are converted to transformation; in Select mode OpenGL
picking is performed with picked objects and mouse click and drag information
returned to the scene.
==============================================================================*/
{
	return Scene_viewer_default_input_callback(scene_viewer, input,
		/*dummy_void*/NULL);
}

int Cmiss_scene_viewer_input_get_event_type(
	Cmiss_scene_viewer_input_id input_data,
	enum Cmiss_scene_viewer_input_event_type *event_type)
/*******************************************************************************
LAST MODIFIED : 11 September 2007

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_scene_viewer_input_get_event_type);
	if (input_data)
	{
		switch(input_data->type)
		{
			case GRAPHICS_BUFFER_MOTION_NOTIFY:
			{
				*event_type = CMISS_SCENE_VIEWER_INPUT_MOTION_NOTIFY;
				return_code = 1;
			} break;
			case GRAPHICS_BUFFER_BUTTON_PRESS:
			{
				*event_type = CMISS_SCENE_VIEWER_INPUT_BUTTON_PRESS;
				return_code = 1;
			} break;
			case GRAPHICS_BUFFER_BUTTON_RELEASE:
			{
				*event_type = CMISS_SCENE_VIEWER_INPUT_BUTTON_RELEASE;
				return_code = 1;
			} break;
			case GRAPHICS_BUFFER_KEY_PRESS:
			{
				*event_type = CMISS_SCENE_VIEWER_INPUT_KEY_PRESS;
				return_code = 1;
			} break;
			case GRAPHICS_BUFFER_KEY_RELEASE:
			{
				*event_type = CMISS_SCENE_VIEWER_INPUT_KEY_RELEASE;
				return_code = 1;
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_scene_viewer_input_get_event_type.  "
					"Invalid type.");
				return_code = 0;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_scene_viewer_input_get_event_type.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

int Cmiss_scene_viewer_input_set_event_type(
	Cmiss_scene_viewer_input_id input_data,
	enum Cmiss_scene_viewer_input_event_type event_type)
/*******************************************************************************
LAST MODIFIED : 11 September 2007

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_scene_viewer_set_projection_mode);
	if (input_data)
	{
		switch(event_type)
		{
			case CMISS_SCENE_VIEWER_INPUT_MOTION_NOTIFY:
			{
				input_data->type = GRAPHICS_BUFFER_MOTION_NOTIFY;
			} break;
			case CMISS_SCENE_VIEWER_INPUT_BUTTON_PRESS:
			{
				input_data->type = GRAPHICS_BUFFER_BUTTON_PRESS;
			} break;
			case CMISS_SCENE_VIEWER_INPUT_BUTTON_RELEASE:
			{
				input_data->type = GRAPHICS_BUFFER_BUTTON_RELEASE;
			} break;
			case CMISS_SCENE_VIEWER_INPUT_KEY_PRESS:
			{
				input_data->type = GRAPHICS_BUFFER_KEY_PRESS;
			} break;
			case CMISS_SCENE_VIEWER_INPUT_KEY_RELEASE:
			{
				input_data->type = GRAPHICS_BUFFER_KEY_RELEASE;
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Cmiss_scene_viewer_set_event_type.  "
					"Unknown event type.");
				return_code = 0;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_scene_viewer_set_event_type.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_scene_viewer_set_projection_mode */

int Cmiss_scene_viewer_input_get_button_number(
	Cmiss_scene_viewer_input_id input_data)
/*******************************************************************************
LAST MODIFIED : 11 September 2007

DESCRIPTION :
Returns the button number that generated the event.
This will be 1 to 3 for a button event and 0 for a non button event.
The object is visible within cmiss but needs an interface to expose the
data through the API.
==============================================================================*/
{
	return (input_data->button_number);
}

int Cmiss_scene_viewer_input_set_button_number(
	Cmiss_scene_viewer_input_id input_data, int button_number)
/*******************************************************************************
LAST MODIFIED : 11 September 2007

DESCRIPTION :
Sets the button number that the event represents.
1 to 3 for a button event and 0 for a non button event.
==============================================================================*/
{
	if (input_data)
	{
		input_data->button_number = button_number;
	}
	return (1);
}

int Cmiss_scene_viewer_input_get_key_code(
	Cmiss_scene_viewer_input_id input_data)
/*******************************************************************************
LAST MODIFIED : 11 September 2007

DESCRIPTION :
Returns the button number that generated the event.
This will be 1 to 3 for a button event and 0 for a non button event.
The object is visible within cmiss but needs an interface to expose the
data through the API.
==============================================================================*/
{
	return (input_data->key_code);
}

int Cmiss_scene_viewer_input_set_key_code(
	Cmiss_scene_viewer_input_id input_data, int key_code)
/*******************************************************************************
LAST MODIFIED : 11 September 2007

DESCRIPTION :
Sets the button number that the event represents.
1 to 3 for a button event and 0 for a non button event.
==============================================================================*/
{
	if (input_data)
	{
		input_data->key_code = key_code;
	}
	return (1);
}

int Cmiss_scene_viewer_input_get_x_position(
	Cmiss_scene_viewer_input_id input_data)
/*******************************************************************************
LAST MODIFIED : 11 September 2007

DESCRIPTION :
Returns the x position of the mouse when the event occured in pixels from top left corner.
==============================================================================*/
{
	return (input_data->position_x);
}

int Cmiss_scene_viewer_input_set_x_position(
	Cmiss_scene_viewer_input_id input_data, int x_position)
/*******************************************************************************
LAST MODIFIED : 11 September 2007

DESCRIPTION :
Sets the button number that the event represents.
1 to 3 for a button event and 0 for a non button event.
==============================================================================*/
{
	if (input_data)
	{
		input_data->position_x = x_position;
	}
	return (1);
}

int Cmiss_scene_viewer_input_get_y_position(
	Cmiss_scene_viewer_input_id input_data)
/*******************************************************************************
LAST MODIFIED : 11 September 2007

DESCRIPTION :
Returns the y position of the mouse when the event occured in pixels from top left corner.
==============================================================================*/
{
	return (input_data->position_y);
}

int Cmiss_scene_viewer_input_set_y_position(
	Cmiss_scene_viewer_input_id input_data, int y_position)
/*******************************************************************************
LAST MODIFIED : 11 September 2007

DESCRIPTION :
Sets the button number that the event represents.
1 to 3 for a button event and 0 for a non button event.
==============================================================================*/
{
	if (input_data)
	{
		input_data->position_y = y_position;
	}
	return (1);
}

int Cmiss_scene_viewer_input_get_modifier_flags(
	Cmiss_scene_viewer_input_id input_data,
	enum Cmiss_scene_viewer_input_modifier_flags *modifier_flags)
/*******************************************************************************
LAST MODIFIED : 12 September 2007

DESCRIPTION :
Returns the set of bit flags showing the whether the modifier inputs
were active when the event was generated.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_scene_viewer_input_get_event_type);
	if (input_data)
	{
		*modifier_flags = 0;
		if (input_data->input_modifier &
			GRAPHICS_BUFFER_INPUT_MODIFIER_SHIFT)
		{
			*modifier_flags |= CMISS_SCENE_VIEWER_INPUT_MODIFIER_SHIFT;
		}
		if (input_data->input_modifier &
			GRAPHICS_BUFFER_INPUT_MODIFIER_CONTROL)
		{
			*modifier_flags |= CMISS_SCENE_VIEWER_INPUT_MODIFIER_CONTROL;
		}
		if (input_data->input_modifier &
			GRAPHICS_BUFFER_INPUT_MODIFIER_ALT)
		{
			*modifier_flags |= CMISS_SCENE_VIEWER_INPUT_MODIFIER_ALT;
		}
		if (input_data->input_modifier &
			GRAPHICS_BUFFER_INPUT_MODIFIER_BUTTON1)
		{
			*modifier_flags |= CMISS_SCENE_VIEWER_INPUT_MODIFIER_BUTTON1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_scene_viewer_input_get_event_type.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

int Cmiss_scene_viewer_input_set_modifier_flags(
	Cmiss_scene_viewer_input_id input_data,
	enum Cmiss_scene_viewer_input_modifier_flags modifier_flags)
/*******************************************************************************
LAST MODIFIED : 12 September 2007

DESCRIPTION :
Sets the set of bit flags showing the whether the modifier inputs
were active when the event was generated.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_scene_viewer_input_get_event_type);
	if (input_data)
	{
		input_data->input_modifier = 0;
		if (modifier_flags & CMISS_SCENE_VIEWER_INPUT_MODIFIER_SHIFT)
		{
			input_data->input_modifier |= 
				GRAPHICS_BUFFER_INPUT_MODIFIER_SHIFT;
		}
		if (modifier_flags & CMISS_SCENE_VIEWER_INPUT_MODIFIER_CONTROL)
		{
			input_data->input_modifier |= 
				GRAPHICS_BUFFER_INPUT_MODIFIER_CONTROL;
		}
		if (modifier_flags & CMISS_SCENE_VIEWER_INPUT_MODIFIER_ALT)
		{
			input_data->input_modifier |= 
				GRAPHICS_BUFFER_INPUT_MODIFIER_ALT;
		}
		if (modifier_flags & CMISS_SCENE_VIEWER_INPUT_MODIFIER_BUTTON1)
		{
			input_data->input_modifier |= 
				GRAPHICS_BUFFER_INPUT_MODIFIER_BUTTON1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_scene_viewer_input_get_event_type.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
}

