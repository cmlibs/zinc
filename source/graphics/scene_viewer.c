/*******************************************************************************
FILE : scene_viewer.c

LAST MODIFIED : 17 February 2005

DESCRIPTION :
Three_D_drawing derivative for viewing a Scene from an arbitrary position.
The scene viewer has the following modes for handling user input:
SCENE_VIEWER_NO_INPUT ignores any input, leaving it up to the owner of the
scene viewer to set viewing parameters.
SCENE_VIEWER_SELECT performs OpenGL picking and returns the picked
objects to the scene via a callback, along with mouse button press and motion
information in a view-independent format.
SCENE_VIEWER_TRANSFORM allows the view of the scene to be changed by tumbling,
translating and zooming with mouse button and press and motion events.

HISTORY :
November 97 Created from rendering part of Drawing.
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
#include <stdio.h>
#include <math.h>
#include "general/compare.h"
#include "three_d_drawing/movie_extensions.h"
#include "general/callback_private.h"
#include "general/debug.h"
#include "general/enumerator_private.h"
#include "general/geometry.h"
#include "general/image_utilities.h"
#include "general/list.h"
#include "general/list_private.h"
#include "general/indexed_list_private.h"
#include "general/matrix_vector.h"
#include "general/object.h"
#include "graphics/colour.h"
#include "graphics/graphics_library.h"
#include "graphics/light.h"
#include "graphics/light_model.h"
#include "graphics/scene.h"
#include "graphics/scene_viewer.h"
#include "graphics/texture.h"
#include "graphics/transform_tool.h"
#include "three_d_drawing/graphics_buffer.h"
#include "user_interface/event_dispatcher.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

#define USE_LAYERZ
#if defined (USE_LAYERZ)
#include "graphics/order_independent_transparency.h"
#endif /* defined (USE_LAYERZ) */

/*
Module constants
----------------
*/
#define SCENE_VIEWER_PICK_SIZE 7.0
#define MAX_CLIP_PLANES (6)

/*
Module types
------------
*/

FULL_DECLARE_CMISS_CALLBACK_TYPES(Cmiss_scene_viewer_package_callback, \
	struct Cmiss_scene_viewer_package *, void *);

FULL_DECLARE_CMISS_CALLBACK_TYPES(Scene_viewer_callback, \
	struct Scene_viewer *, void *);

FULL_DECLARE_CMISS_CALLBACK_TYPES(Scene_viewer_input_callback, \
	struct Scene_viewer *, struct Graphics_buffer_input *);

/*
Module functions
----------------
*/

enum Scene_viewer_drag_mode
{
	SV_DRAG_NOTHING,
	SV_DRAG_TUMBLE,
	SV_DRAG_TRANSLATE,
	SV_DRAG_ZOOM
};

struct Cmiss_scene_viewer_package
/*******************************************************************************
LAST MODIFIED : 19 January 2007

DESCRIPTION:
The default data used to create Cmiss_scene_viewers.
==============================================================================*/
{
	struct Graphics_buffer_package *graphics_buffer_package;
	struct Colour *background_colour;
 	struct MANAGER(Interactive_tool) *interactive_tool_manager;
	struct Interactive_tool *default_interactive_tool;
	struct MANAGER(Light) *light_manager;
	struct Light *default_light;
	struct MANAGER(Light_model) *light_model_manager;
	struct Light_model *default_light_model;
	struct MANAGER(Scene) *scene_manager;
	struct Scene *scene;
	struct MANAGER(Texture) *texture_manager;
	struct User_interface *user_interface;
	/* List of scene_viewers created with this package,
		generally all scene_viewers that are not in graphics windows */
	struct LIST(Scene_viewer) *scene_viewer_list;
	struct LIST(CMISS_CALLBACK_ITEM(Cmiss_scene_viewer_package_callback))
		*destroy_callback_list;
};

struct Scene_viewer
/*******************************************************************************
LAST MODIFIED : 12 July 2000

DESCRIPTION :
==============================================================================*/
{
	/* The buffer into which this scene viewer is rendering */
	struct Graphics_buffer *graphics_buffer;
	enum Scene_viewer_input_mode input_mode;
	/* following flag forces the scene_viewer temporarily into transform mode
		 when the control key is held down */
	int temporary_transform_mode;
	/* scene to be viewed */
	struct Scene *scene;
	/* overlay scene for showing spectrum scale, annotation, etc. This is always
		 set up with a parallel projection fitting the largest cube in the window
		 with coordinates ranging from -1 to +1 from left to right, bottom to top
		 and far to near_plane. If the window is non-square, coordinates outside -1 to +1
		 are visible in the longer dimension, while 0,0 is the centre of the screen.
		 The overlay_scene is NULL by default (ie. no overlay). */
	struct Scene *overlay_scene;
	/* The projection mode. PARALLEL and PERSPECTIVE projections get their
		 modelview matrix using gluLookat, and their projection matrix from the
		 viewing volume. CUSTOM projection requires both matrices to be read-in */
	enum Scene_viewer_projection_mode projection_mode;
	/* Viewing transformation defined by eye pos, look-at point and up-vector */
	double eyex,eyey,eyez;
	double lookatx,lookaty,lookatz;
	double upx,upy,upz;
	/* Viewing volume for PARALLEL and PERSPECTIVE projections. */
	double left,right,bottom,top,near_plane,far_plane;
	/* Scale factors for controlling how rate of translate, tumble and zoom
		 transformations in relation to mouse movements. Setting a value to
		 zero turns off that transform capability. */
	double translate_rate,tumble_rate,zoom_rate;
	/* For CUSTOM projection only: 4X4 projection and modelview matrices.
		These are now stored internally in OpenGL format.
		ie. m0 m4 m8 m12 would be the first row, m1 m5 m9 m13 the second, etc. */
	double projection_matrix[16],modelview_matrix[16],
		window_projection_matrix[16];
	/* The projection matrix, whether set directly for CUSTOM projection or
		 calculated for PARALLEL and PERSPECTIVE projections using the viewing
		 volume, converts 3-D positions into Normalized Device Coordinates (NDCs) in
		 a cube from -1 to +1 in each direction.  In the z (depth) direction the
		 values from -1 (=near_plane plane) to +1 (=far plane) are already where we want
		 and need no further processing.  In general, however, the real x,y size and
		 origin in user coordinates are needed to display the image in an
		 undistorted manner. The following NDC_ variables are used for this purpose.
		 In a CUSTOM projection they must be read-in. PARALLEL and PERSPECTIVE
		 projections calculate them from the viewing volume.  Note that these values
		 must be given in user coordinates. */
	double NDC_left,NDC_top,NDC_width,NDC_height;
	/* The viewport mode specifies whether the NDCs, adjusted to the aspect
		 ratio from NDC_width/NDC_height are made as large as possible in the
		 physical viewport (RELATIVE_VIEWPORT), or whether an exact mapping from
		 user coordinates to pixels is used (ABSOLUTE_VIEWPORT), or whether the
		 aspect ratio from NDC_width/NDC_height is ignored and the NDCs are made
		 as large as possible(DISTORTING_RELATIVE_VIEWPORT).
	*/
	enum Scene_viewer_viewport_mode viewport_mode;
	/* Specifies the offset and scale of user coordinates in the physical
		 viewport, by supplying the user coordinate of the top,left position in
		 and the number of pixels plotted for a change of 1 in user units. Note
		 that these are in no way restricted to integer values.
		 ???RC.  Write how to handle y increasing down the screen? */
	double user_viewport_left,user_viewport_top,user_viewport_pixels_per_unit_x,
		user_viewport_pixels_per_unit_y;
	/* specifies the quality of transparency rendering */
	enum Scene_viewer_transparency_mode transparency_mode;
	/* number of layers used in layered transparency mode */
	int transparency_layers;
	/* A background texture may also be displayed behind the projected image */
	struct Texture *background_texture;
	/* When an ABSOLUTE_VIEWPORT is used the following values specify the
		 position and scale of the image relative to user coordinates. In the
		 RELATIVE_VIEWPORT and DISTORTING_RELATIVE_VIEWPORT modes, these values
		 are ignored and the image is
		 drawn behind the normalized device coordinate range.
		 ???RC.  Allow texture to be cropped as well? */
	double bk_texture_left,bk_texture_top,bk_texture_width,
		bk_texture_height,bk_texture_max_pixels_per_polygon;
	int bk_texture_undistort_on;
	/* Callbacks that are told about input (mouse clicks etc.) into the scene_viewer */
	struct LIST(CMISS_CALLBACK_ITEM(Scene_viewer_input_callback)) *input_callback_list;
	/* list of callbacks requested by other objects when view changes */
	struct LIST(CMISS_CALLBACK_ITEM(Scene_viewer_callback)) *sync_callback_list;
	struct LIST(CMISS_CALLBACK_ITEM(Scene_viewer_callback)) *transform_callback_list;
	/* list of callbacks requested by other objects when scene viewer destroyed */
	struct LIST(CMISS_CALLBACK_ITEM(Scene_viewer_callback)) *destroy_callback_list;
	/* When working in windowless mode we must only redraw as requested
		by the host application, the host should register for these callbacks
		and respond with a full repaint. */
	struct LIST(CMISS_CALLBACK_ITEM(Scene_viewer_callback)) *repaint_required_callback_list;
	/* the scene_viewer must always have a light model */
	struct Light_model *light_model;
	/* lights in this list are oriented relative to the viewer */
	struct LIST(Light) *list_of_lights;
	struct Event_dispatcher_idle_callback *idle_update_callback_id;
	/* managers and callback IDs for automatic updates */
	struct MANAGER(Light) *light_manager;
	void *light_manager_callback_id;
	struct MANAGER(Light_model) *light_model_manager;
	void *light_model_manager_callback_id;
	struct MANAGER(Scene) *scene_manager;
	void *scene_manager_callback_id;
	struct MANAGER(Texture) *texture_manager;
	void *texture_manager_callback_id;
	/* For interpreting mouse events */
        enum Scene_viewer_interact_mode interact_mode;
	enum Scene_viewer_drag_mode drag_mode;
	int previous_pointer_x, previous_pointer_y;
	/* interaction */
	/* Note: interactive_tool is NOT accessed by Scene_viewer; up to dialog
		 owning it to clear it if it is destroyed. This is usually ensured by having
		 a tool chooser in the parent dialog */
	struct Interactive_tool *interactive_tool;
 	struct MANAGER(Interactive_tool) *interactive_tool_manager;
	/* kept tumble axis and angle for spinning scene viewer */
	double tumble_axis[3], tumble_angle;
	int tumble_active;
	/* background */
	struct Colour background_colour;
	enum Scene_viewer_buffering_mode buffering_mode;
	enum Scene_viewer_stereo_mode stereo_mode;
	int pixel_height,pixel_width,update_pixel_image;
	void *pixel_data;
	int antialias;
	int perturb_lines;
	enum Scene_viewer_blending_mode blending_mode;
	double depth_of_field;  /* depth_of_field, 0 == infinite */
	double focal_depth;
	/* set between fast changing operations since the first fast-change will
		 copy from the front buffer to the back; subsequent changes will copy
		 saved buffer from back to front. */
	int first_fast_change;
	/* flag is cleared as soon as a change to the scene is not fast_changing */
	int fast_changing;
	/* flag indicating that the viewer should swap buffers at the next
		 appropriate point */
	int swap_buffers;
	/* Flag that indicates the update includes a change of the projection matrices */
	int transform_flag;
	/* Clip planes */
	char clip_planes_enable[MAX_CLIP_PLANES];
	double clip_planes[MAX_CLIP_PLANES * 4];
	/* The distance between the two stereo views in world space */
	double stereo_eye_spacing;
	/* Special persistent data for order independent transparency */
	struct Scene_viewer_order_independent_transparency_data
	   *order_independent_transparency_data;
	/* The connection to the systems user interface system */
	struct User_interface *user_interface;
#if defined (WIN32_SYSTEM)
	/* Clear twice, if set then the glClear in the background will be called
		twice, which appears to work around a rendering bug on ATI windows driver 6.14.0010.6706 */
	int clear_twice_flag;
#endif /* defined (WIN32_SYSTEM) */
	/* Keeps a counter of the frame redraws */
	unsigned int frame_count;
}; /* struct Scene_viewer */

DECLARE_LIST_TYPES(Scene_viewer);

struct Scene_viewer_rendering_data
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
Temporary data that only lasts for a single rendering.  Used by all the 
rendering functions.
==============================================================================*/
{
	/* The main scene_viewer */
	struct Scene_viewer *scene_viewer;

	/* The render callstack which we are processing */
	struct LIST(Scene_viewer_render_object) *render_callstack;

	/* Information for the rendering */
	int override_antialias;
	int override_transparency_layers;
	/* Width and height of actual viewport used internally for rendering */
	int viewport_left;
	int viewport_bottom;
	int viewport_width;
	int viewport_height;
	/* A flag to indicate to the rendering routines if the current render is
		actually being buffered, it may differ from teh buffering_mode if it
		the scene is being rendered offscreen */
	int rendering_double_buffered;
	/* Stencil buffer depth */
	GLint stencil_depth;
}; /* struct Scene_viewer_rendering_data */

struct Scene_viewer_render_object;
DECLARE_LIST_TYPES(Scene_viewer_render_object);

typedef int (Scene_viewer_render_function)(
	struct Scene_viewer_rendering_data *rendering_data);
/*******************************************************************************
LAST MODIFIED : 14 April 2003

DESCRIPTION :
A rendering function that exists in the rendering callstack.  See the 
struct Scene_viewer_render_object.
==============================================================================*/

struct Scene_viewer_render_object
/*******************************************************************************
LAST MODIFIED : 4 April 2003

DESCRIPTION :
This object is used to implement a callstack of render functions.  Each render
function may need to call those lower down multiple times and this provides
a facility for building a calltree of such functions.  This enables us to break
up the rendering into more sensible components.
In addition to the <rendering_data> which is private to this module some rendering
functions may have their own user data.
==============================================================================*/
{
	Scene_viewer_render_function *render_function;
	/* A flag used when descending down the call stack to indicate which 
		items have already been processed */
	int already_processed;
	int access_count;
}; /* struct Scene_viewer_render_object */

FULL_DECLARE_LIST_TYPE(Scene_viewer);

/* We need to maintain the order, so we do not want an indexed list */
FULL_DECLARE_LIST_TYPE(Scene_viewer_render_object);

/*
Module functions
----------------
*/

PROTOTYPE_LIST_FUNCTIONS(Scene_viewer);

PROTOTYPE_LIST_FUNCTIONS(Scene_viewer_render_object);

static struct Scene_viewer_render_object *CREATE(Scene_viewer_render_object)(
	Scene_viewer_render_function *render_function)
/*******************************************************************************
LAST MODIFIED : 14 April 2003

DESCRIPTION :
A rendering pass that can be incoporated into the rendering callstack.  The
render_function should implement the component of the rendering, calling
Scene_viewer_call_next_renderer to execute the rest of the renderer.
==============================================================================*/
{
	struct Scene_viewer_render_object *render_object;

	ENTER(CREATE(Scene_viewer_render_object));
	if (render_function)
	{
		if (ALLOCATE(render_object,struct Scene_viewer_render_object,1))
		{
 			render_object->render_function = render_function;
			render_object->already_processed = 0;
			render_object->access_count = 0;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Scene_viewer_render_object).  Could not allocate memory for node field");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Scene_viewer_render_object).  Invalid argument(s)");
		render_object = (struct Scene_viewer_render_object *)NULL;
	}
	LEAVE;

	return (render_object);
} /* CREATE(Scene_viewer_render_object) */

static int Scene_viewer_render_object_has_not_been_processed(
	struct Scene_viewer_render_object *render_object, void *dummy_void)
/*******************************************************************************
LAST MODIFIED : 7 April 2003

DESCRIPTION :
Returns NOT the state of the already_processed flag.
==============================================================================*/
{
	int return_code;
  
	ENTER(Scene_viewer_render_object_has_not_been_processed);
	USE_PARAMETER(dummy_void);
	if (render_object)
	{
		return_code = ! render_object->already_processed;
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_render_object_has_not_been_processed */

static int DESTROY(Scene_viewer_render_object)(
	struct Scene_viewer_render_object **render_object_address)
/*******************************************************************************
LAST MODIFIED : 4 April 2003

DESCRIPTION :
Frees the memory for the render_object and sets <*render_object_address> to NULL.
==============================================================================*/
{
	int return_code;

	ENTER(DESTROY(Scene_viewer_render_object));
	if (render_object_address)
	{
		if (0 == (*render_object_address)->access_count)
		{
			DEALLOCATE(*render_object_address);
		}
		else
		{
			*render_object_address = 
				(struct Scene_viewer_render_object *)NULL;
		}
		return_code = 1;
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Scene_viewer_render_object) */

DECLARE_OBJECT_FUNCTIONS(Scene_viewer_render_object)
DECLARE_LIST_FUNCTIONS(Scene_viewer_render_object)

DEFINE_CMISS_CALLBACK_MODULE_FUNCTIONS(Cmiss_scene_viewer_package_callback, void)

DEFINE_CMISS_CALLBACK_FUNCTIONS(Cmiss_scene_viewer_package_callback, \
	struct Cmiss_scene_viewer_package *,void *)

	DEFINE_CMISS_CALLBACK_MODULE_FUNCTIONS(Scene_viewer_callback, void)

DEFINE_CMISS_CALLBACK_FUNCTIONS(Scene_viewer_callback, \
	struct Scene_viewer *,void *)

DEFINE_CMISS_CALLBACK_MODULE_FUNCTIONS(Scene_viewer_input_callback, int)

DEFINE_CMISS_CALLBACK_FUNCTIONS(Scene_viewer_input_callback, \
	struct Scene_viewer *,struct Graphics_buffer_input *)

static int Scene_viewer_render_background_texture(
	struct Scene_viewer *scene_viewer,int viewport_width,int viewport_height)
/*******************************************************************************
LAST MODIFIED : 11 February 2002

DESCRIPTION :
==============================================================================*/
{
	double corner_x[4],corner_y[4],corr_x1,corr_x2,corr_y1,corr_y2,
		distortion_centre_x,distortion_centre_y,distortion_factor_k1,
		dist_x,dist_y1,dist_y2,min_x,max_x,min_y,max_y,tex_ratio_x,tex_ratio_y,
		viewport_texture_height,viewport_texture_width;
	float centre_x,centre_y,factor_k1,texture_width,texture_height,texture_depth;
	GLdouble viewport_left,viewport_right,viewport_bottom,viewport_top;
	int depth_texels, height_texels,i,j,k,min_i,max_i,min_j,max_j,return_code,
		texels_per_polygon_x,texels_per_polygon_y,width_texels;

	ENTER(Scene_viewer_render_background_texture);
	if (scene_viewer&&scene_viewer->background_texture)
	{
		/* get information about the texture */
		Texture_get_original_size(scene_viewer->background_texture,
			&width_texels, &height_texels, &depth_texels);
		Texture_get_physical_size(scene_viewer->background_texture,
			&texture_width, &texture_height, &texture_depth);
		tex_ratio_x=texture_width/width_texels;
		tex_ratio_y=texture_height/height_texels;
		/* note the texture stores radial distortion parameters in terms
			 of its physical space from 0,0 to texture_width,texture_height.
			 We want them in terms of user viewport coordinates */
		Texture_get_distortion_info(scene_viewer->background_texture,
			&centre_x,&centre_y,&factor_k1);
		distortion_centre_x=(double)centre_x;
		distortion_centre_y=(double)centre_y;
		if (scene_viewer->bk_texture_undistort_on)
		{
			distortion_factor_k1=(double)factor_k1;
		}
		else
		{
			distortion_factor_k1=0.0;
		}
		/* set up orthographic projection to match physical/model
			 coordinates of background texture */
		viewport_texture_width=scene_viewer->bk_texture_width;
		if (0.0==viewport_texture_width)
		{
			/* to avoid division by zero */
			viewport_texture_width=1.0;
		}
		viewport_texture_height=scene_viewer->bk_texture_height;
		if (0.0==viewport_texture_height)
		{
			/* to avoid division by zero */
			viewport_texture_height=1.0;
		}
		viewport_left = texture_width/viewport_texture_width*
			(scene_viewer->user_viewport_left - scene_viewer->bk_texture_left);
		viewport_right = viewport_left +
			((double)viewport_width/scene_viewer->user_viewport_pixels_per_unit_x)*
			texture_width/viewport_texture_width;
		viewport_top = texture_height +
			texture_height/viewport_texture_height *
			(scene_viewer->user_viewport_top - scene_viewer->bk_texture_top);
		viewport_bottom=viewport_top -
			((double)viewport_height/scene_viewer->user_viewport_pixels_per_unit_y)*
			texture_height/viewport_texture_height;
#if defined (DEBUG)
		/*???debug */
		printf("viewport left=%f right=%f  top=%f bottom=%f\n",
			viewport_left,viewport_right,viewport_top,viewport_bottom);
#endif /* defined (DEBUG) */
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(viewport_left,viewport_right,viewport_bottom,viewport_top,
			-1.0,1.0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		compile_Texture(scene_viewer->background_texture,
			scene_viewer->graphics_buffer, (struct Texture_tiling **)NULL);
		execute_Texture(scene_viewer->background_texture);

#if defined (OLD_CODE)
		/* simple, un-corrected texture */
		glBegin(GL_QUADS);
		glTexCoord2d(0.0,(double)texture_height);
		glVertex3d(0.0,height_texels,-0.999);
		glTexCoord2d((double)texture_width,(double)texture_height);
		glVertex3d(width_texels,height_texels,-0.999);
		glTexCoord2d((double)texture_width,0.0);
		glVertex3d(width_texels,0,-0.999);
		glTexCoord2d(0.0,0.0);
		glVertex3d(0,0,-0.999);
		glEnd();
#endif /* defined (OLD_CODE) */

		/* get texels per polygon */
		texels_per_polygon_x=1;
		while ((2*texels_per_polygon_x*
			scene_viewer->user_viewport_pixels_per_unit_x*
			viewport_texture_width/width_texels <=
			scene_viewer->bk_texture_max_pixels_per_polygon)&&
			(texels_per_polygon_x < width_texels))
		{
			texels_per_polygon_x *= 2;
		}
		texels_per_polygon_y=1;
		while ((2*texels_per_polygon_y*
			scene_viewer->user_viewport_pixels_per_unit_y*
			viewport_texture_height/height_texels <=
			scene_viewer->bk_texture_max_pixels_per_polygon)&&
			(texels_per_polygon_y < height_texels))
		{
			texels_per_polygon_y *= 2;
		}
#if defined (DEBUG)
		/*???debug */
		printf("texels per polygon: x=%i y=%i\n",texels_per_polygon_x,
			texels_per_polygon_y);
#endif /* defined (DEBUG) */
		/* get range of physical texture coordinates across viewport */
		corner_x[0]=viewport_left;
		corner_x[1]=viewport_right;
		corner_x[2]=viewport_right;
		corner_x[3]=viewport_left;
		corner_y[0]=viewport_top;
		corner_y[1]=viewport_top;
		corner_y[2]=viewport_bottom;
		corner_y[3]=viewport_bottom;
		min_x=max_x=viewport_left;
		min_y=max_y=viewport_top;
		for (k=0;k<4;k++)
		{
			if (corner_x[k]<min_x)
			{
				min_x=corner_x[k];
			}
			if (corner_x[k]>max_x)
			{
				max_x=corner_x[k];
			}
			if (corner_y[k]<min_y)
			{
				min_y=corner_y[k];
			}
			if (corner_y[k]>max_y)
			{
				max_y=corner_y[k];
			}
			if (0!=distortion_factor_k1)
			{
				get_radial_distortion_distorted_coordinates(corner_x[k],
					corner_y[k],distortion_centre_x,distortion_centre_y,
					distortion_factor_k1,/*tolerance*/0.001,&(corner_x[k]),
					&(corner_y[k]));
				if (corner_x[k]<min_x)
				{
					min_x=corner_x[k];
				}
				if (corner_x[k]>max_x)
				{
					max_x=corner_x[k];
				}
				if (corner_y[k]<min_y)
				{
					min_y=corner_y[k];
				}
				if (corner_y[k]>max_y)
				{
					max_y=corner_y[k];
				}
			}
		}
		/* ensure inside actual range of image */
		if (min_x<0)
		{
			min_x=0;
		}
		if (max_x>texture_width)
		{
			max_x=texture_width;
		}
		if (min_y<0)
		{
			min_y=0;
		}
		if (max_y>texture_height)
		{
			max_y=texture_height;
		}
		/* get max_x, max_y in terms of texels */
		min_x /= tex_ratio_x;
		max_x /= tex_ratio_x;
		min_y /= tex_ratio_y;
		max_y /= tex_ratio_y;
#if defined (DEBUG)
		/*???debug */
		printf("min_x=%f max_x=%f  min_y=%f max_y=%f\n",min_x,max_x,min_y,
			max_y);
#endif /* defined (DEBUG) */
		min_i = min_x/texels_per_polygon_x;
		max_i = ceil(0.999999*max_x/texels_per_polygon_x);
		min_j = min_y/texels_per_polygon_y;
		max_j = ceil(0.999999*max_y/texels_per_polygon_y);
#if defined (DEBUG)
		/*???debug */
		printf("min_i=%i max_i=%i  min_j=%i max_j=%i\n",min_i,max_i,min_j,
			max_j);
#endif /* defined (DEBUG) */
		tex_ratio_x *= texels_per_polygon_x;
		tex_ratio_y *= texels_per_polygon_y;
		/* draw the array of polygons */
		for (j=min_j;j<max_j;j++)
		{
			dist_y1=j*tex_ratio_y;
			dist_y2=(j+1)*tex_ratio_y;
			if (dist_y2>texture_height)
			{
				dist_y2=texture_height;
			}
			glBegin(GL_QUAD_STRIP);
			for (i=min_i;i<=max_i;i++)
			{
				dist_x=i*tex_ratio_x;
				if (dist_x>texture_width)
				{
					dist_x=texture_width;
				}
				get_radial_distortion_corrected_coordinates(dist_x,dist_y1,
					distortion_centre_x,distortion_centre_y,distortion_factor_k1,
					&corr_x1,&corr_y1);
				get_radial_distortion_corrected_coordinates(dist_x,dist_y2,
					distortion_centre_x,distortion_centre_y,distortion_factor_k1,
					&corr_x2,&corr_y2);
				glTexCoord2d(dist_x,dist_y1);
				glVertex3d(corr_x1,corr_y1,-0.999);
				glTexCoord2d(dist_x,dist_y2);
				glVertex3d(corr_x2,corr_y2,-0.999);
			}
			glEnd();
		}
		execute_Texture((struct Texture *)NULL);
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_render_background_texture.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_render_background_texture */

static int Scene_viewer_calculate_transformation(
	struct Scene_viewer *scene_viewer, int viewport_width, int viewport_height)
/*******************************************************************************
LAST MODIFIED : 04 February 2005

DESCRIPTION :
Calculates the projection_matrix, window_projection_matrix and modelview_materix
for the <scene_viewer> onto a given <viewport_width>.<viewport_height> in
pixels, both of which must be positive.
The <left>, <right>, <top> and <bottom> are used to specify the viewing frustrum,
by default these are stored in the scene_viewer but are supplied as parameters
to this function so that they can be overridden.
In CUSTOM projections, the projection_matrix and modelview_matrix are supplied
and hence these are only calculated for other projection modes. In all cases
the projection_matrix is understood to project space onto the NDC_info; In
absolute_viewport mode this is some fixed region relative to the user
viewport coordinates. In relative_viewport mode, the NDC_width and NDC_height
are made as large as can fit in the viewport size without a shape change.
As a result, the projection_matrix in both relative and absolute viewport modes
is not the projection that will fill the entire viewport/window - this function
calculates the window_projection_matrix for this purpose.
Note that this function makes changes to the OpenGL rendering matrices in some
modes, so push/pop them if you want them preserved.
==============================================================================*/
{
	double dx,dy,dz,postmultiply_matrix[16],factor;
	int return_code,i;

	ENTER(Scene_viewer_calculate_transformation);
	if (scene_viewer&&(0<viewport_width)&&(0<viewport_height))
	{
		return_code=1;

		/* 1. calculate and store projection_matrix - no need in CUSTOM mode */
		if (SCENE_VIEWER_CUSTOM != scene_viewer->projection_mode)
		{
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			switch (scene_viewer->projection_mode)
			{
				case SCENE_VIEWER_PARALLEL:
				{
					glOrtho(scene_viewer->left, scene_viewer->right,
						scene_viewer->bottom, scene_viewer->top,
						scene_viewer->near_plane, scene_viewer->far_plane);
				} break;
				case SCENE_VIEWER_PERSPECTIVE:
				{
					/* adjust left, right, bottom, top from lookat plane to near plane */
					dx = scene_viewer->eyex-scene_viewer->lookatx;
					dy = scene_viewer->eyey-scene_viewer->lookaty;
					dz = scene_viewer->eyez-scene_viewer->lookatz;
					factor = scene_viewer->near_plane/sqrt(dx*dx+dy*dy+dz*dz);
					/* perspective projection */
					glFrustum(scene_viewer->left*factor, scene_viewer->right*factor,
						scene_viewer->bottom*factor, scene_viewer->top*factor,
						scene_viewer->near_plane, scene_viewer->far_plane);
				} break;
			}
			glGetDoublev(GL_PROJECTION_MATRIX,scene_viewer->projection_matrix);
		}

		/* 2. calculate and store window_projection_matrix - all modes */
		/* the projection matrix converts the viewing volume into the Normalised
			 Device Coordinates (NDCs) ranging from -1 to +1 in each coordinate
			 direction. Need to scale this range to fit the viewport/window by
			 postmultiplying with a matrix. First start with identity: Note that
			 numbers go down columns first in OpenGL matrices */
		for (i=1;i<15;i++)
		{
			postmultiply_matrix[i] = 0.0;
		}
		postmultiply_matrix[ 0] = 1.0;
		postmultiply_matrix[ 5] = 1.0;
		postmultiply_matrix[10] = 1.0;
		postmultiply_matrix[15] = 1.0;
		switch (scene_viewer->viewport_mode)
		{
			case SCENE_VIEWER_ABSOLUTE_VIEWPORT:
			{
				/* absolute viewport: NDC volume is placed in the position
					 described by the NDC_info relative to user viewport
					 coordinates - as with the background texture */
				postmultiply_matrix[0] *= scene_viewer->NDC_width*
					scene_viewer->user_viewport_pixels_per_unit_x/viewport_width;
				postmultiply_matrix[5] *= scene_viewer->NDC_height*
					scene_viewer->user_viewport_pixels_per_unit_y/viewport_height;
				postmultiply_matrix[12] = -1.0+
					((scene_viewer->user_viewport_pixels_per_unit_x)/viewport_width)*
					((scene_viewer->NDC_width)+
						2.0*(scene_viewer->NDC_left-scene_viewer->user_viewport_left));
				postmultiply_matrix[13] =1.0+
					((scene_viewer->user_viewport_pixels_per_unit_y)/viewport_height)*
					(-(scene_viewer->NDC_height)+
						2.0*(scene_viewer->NDC_top-scene_viewer->user_viewport_top));
			} break;
			case SCENE_VIEWER_RELATIVE_VIEWPORT:
			{
				/* relative viewport: NDC volume is scaled to the largest size
					 that can fit in the viewport without distorting its shape. Note that
					 the NDC_height and NDC_width are all that is needed to characterise
					 the size/shape of the NDC volume in relative mode */
				if (scene_viewer->NDC_height/scene_viewer->NDC_width >
					(double)viewport_height/(double)viewport_width)
				{
					/* make NDC represent a wider viewing volume. */
					postmultiply_matrix[0] *= (scene_viewer->NDC_width*viewport_height/
						(scene_viewer->NDC_height*viewport_width));
				}
				else
				{
					/* make NDC represent a taller viewing volume */
					postmultiply_matrix[5] *= (scene_viewer->NDC_height*viewport_width/
						(scene_viewer->NDC_width*viewport_height));
				}
			} break;
			case SCENE_VIEWER_DISTORTING_RELATIVE_VIEWPORT:
			{
				/* distorting relative viewport: NDC volume is scaled to the largest size
					 that can fit in the viewport. Note that
					 the NDC_height and NDC_width are all that is needed to characterise
					 the size/shape of the NDC volume in relative mode
					 This is a simple no-op, as the identity matrix is sufficient to achieve this.
				*/
			} break;
		}
		multiply_matrix(4,4,4,scene_viewer->projection_matrix,postmultiply_matrix,
			scene_viewer->window_projection_matrix);

		/* 3. Calculate and store modelview_matrix - no need in CUSTOM mode */
		if (SCENE_VIEWER_CUSTOM != scene_viewer->projection_mode)
		{
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			gluLookAt(scene_viewer->eyex,scene_viewer->eyey,
				scene_viewer->eyez,scene_viewer->lookatx,
				scene_viewer->lookaty,scene_viewer->lookatz,
				scene_viewer->upx,scene_viewer->upy,scene_viewer->upz);
			glGetDoublev(GL_MODELVIEW_MATRIX,scene_viewer->modelview_matrix);
		}
#if defined (REPORT_GL_ERRORS)
		{
			char message[200];
			GLenum error;
			int max_error = 200;
			while((max_error--) && (GL_NO_ERROR!=(error = glGetError())))
			{
				strcpy(message,"Scene_viewer_calculate_transformation: GL ERROR ");
				strcat(message, (char *)gluErrorString(error));
				display_message(ERROR_MESSAGE, message);
			}
		}
#endif /* defined (REPORT_GL_ERRORS) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_calculate_transformation.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_calculate_transformation */

int Scene_viewer_call_next_renderer(
	struct Scene_viewer_rendering_data *rendering_data)
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
This function calls the next renderer that isn't already being processed and
handles the flags that mark render_objects as processed.
==============================================================================*/
{
	int return_code;
	struct Scene_viewer_render_object *render_object;
	render_object= NULL;
	ENTER(Scene_viewer_call_next_renderer);
	if (rendering_data)
	{
		render_object = FIRST_OBJECT_IN_LIST_THAT(Scene_viewer_render_object)
			(Scene_viewer_render_object_has_not_been_processed, NULL,
			rendering_data->render_callstack);
		render_object->already_processed = 1;
		return_code = (*(render_object->render_function))(
			 rendering_data);
		/* The flag must be set back as we may call parts of the tree
			many times. */
		render_object->already_processed = 0;
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);	
} /* Scene_viewer_call_next_renderer */

static int Scene_viewer_execute_scene_non_fastchanging(
	struct Scene_viewer_rendering_data *rendering_data)
/*******************************************************************************
LAST MODIFIED : 4 April 2003

DESCRIPTION :
This is the last function in the render callstack that actually executes the
scene.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_execute_scene_non_fastchanging);
	if (rendering_data)
	{
		return_code = 1;
		
		execute_Scene_non_fast_changing(rendering_data->scene_viewer->scene);
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);	
} /* Scene_viewer_execute_scene_non_fastchanging */

static int Scene_viewer_apply_projection_matrix(
	struct Scene_viewer_rendering_data *rendering_data)
/*******************************************************************************
LAST MODIFIED : 8 April 2003

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_apply_projection_matrix);
	if (rendering_data)
	{
		return_code = 1;

		glMatrixMode(GL_PROJECTION);

		glPushMatrix();

		glMultMatrixd(rendering_data->scene_viewer->window_projection_matrix);
		
		Scene_viewer_call_next_renderer(rendering_data);

		glMatrixMode(GL_PROJECTION);

		glPopMatrix();
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);	
} /* Scene_viewer_apply_projection_matrix */

static int Scene_viewer_use_pixel_buffer(
	struct Scene_viewer_rendering_data *rendering_data)
/*******************************************************************************
LAST MODIFIED : 8 April 2003

DESCRIPTION :
Keeps a copy of the scene in a pixel buffer and only updates that image
when the scene_viewer->update_pixel_image flag is set.  This is used by the 
emoter to make icons representing the current scene.
==============================================================================*/
{
	GLboolean valid_raster;
	GLdouble obj_x,obj_y,obj_z;
	static GLint viewport[4]={0,0,1,1};
	int return_code;
	struct Scene_viewer *scene_viewer;
	void *new_data;

	ENTER(Scene_viewer_use_pixel_buffer);
	if (rendering_data && (scene_viewer = rendering_data->scene_viewer))
	{
		return_code = 1;
		if (scene_viewer->update_pixel_image)
		{
			/* Draw the scene first */
			Scene_viewer_call_next_renderer(rendering_data);

			/* Copy the image from the scene viewer */
			if (REALLOCATE(new_data,rendering_data->scene_viewer->pixel_data,char,
				3*(rendering_data->viewport_width+1)*(rendering_data->viewport_height+1)))
			{
				scene_viewer->pixel_data=new_data;
				glReadPixels(0,0,rendering_data->viewport_width,
					rendering_data->viewport_height,GL_RGB,GL_BYTE,
					scene_viewer->pixel_data);
				scene_viewer->pixel_width=rendering_data->viewport_width;
				scene_viewer->pixel_height=rendering_data->viewport_height;
				scene_viewer->update_pixel_image=0;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Scene_viewer_use_pixel_buffer.  "
					"Unable to reallocate pixel dataspace");
				scene_viewer->pixel_width=0;
				scene_viewer->pixel_height=0;
			}
		}
		else
		{
			/* Just draw the pixel buffer back into the window */
			glClearColor((scene_viewer->background_colour).red,
				(scene_viewer->background_colour).green,
				(scene_viewer->background_colour).blue,0.);
			glClearDepth(1.0);
			glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
			glGetDoublev(GL_MODELVIEW_MATRIX,scene_viewer->modelview_matrix);
			glGetDoublev(GL_PROJECTION_MATRIX,scene_viewer->projection_matrix);
			/*				glGetIntegerv(GL_VIEWPORT,viewport);*/
			/* for OpenGL window z coordinates, 0.0=near_plane, 1.0=far */
			if (GL_TRUE==gluUnProject(0.0001,0.0001,0.1,
				scene_viewer->modelview_matrix,scene_viewer->projection_matrix,
				viewport,&obj_x,&obj_y,&obj_z))
			{
				glRasterPos3d(obj_x,obj_y,obj_z);
				glGetBooleanv(GL_CURRENT_RASTER_POSITION_VALID,&valid_raster);
				if (valid_raster)
				{
					glDrawPixels(scene_viewer->pixel_width,scene_viewer->pixel_height,
						GL_RGB,GL_BYTE,scene_viewer->pixel_data);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Scene_viewer_use_pixel_buffer.  "
						"Culled raster position for redraw");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Scene_viewer_use_pixel_buffer.  Unable to unproject");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "Scene_viewer_use_pixel_buffer.  "
			"Invalid arguments");
		return_code = 0;
	}
	LEAVE;

	return (return_code);	
} /* Scene_viewer_use_pixel_buffer */

static int Scene_viewer_render_overlay_scene(
	struct Scene_viewer_rendering_data *rendering_data)
/*******************************************************************************
LAST MODIFIED : 8 April 2003

DESCRIPTION :
==============================================================================*/
{
	double max_x, max_y;
	int return_code;
	struct Scene_viewer *scene_viewer;

	ENTER(Scene_viewer_render_overlay_scene);
	if (rendering_data && (scene_viewer = rendering_data->scene_viewer))
	{
		return_code = 1;
		/* set up parallel projection/modelview combination that gives
			coordinates ranging from -1 to +1 from left to right and
			bottom to top in the largest square that fits inside the
			viewer. Also has -1 to +1 range from far to near_plane */
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		if (rendering_data->viewport_width > rendering_data->viewport_height)
		{
			max_x = (double)rendering_data->viewport_width/
				(double)rendering_data->viewport_height;
			max_y = 1.0;
		}
		else
		{
			max_x = 1.0;
			max_y = (double)rendering_data->viewport_height/
				(double)rendering_data->viewport_width;
		}
		if (0 < rendering_data->stencil_depth)
		{
			/* use full z-buffer for overlay ranging from -1 to 1 in z */
			glOrtho(-max_x,max_x,-max_y,max_y,1.0,3.0);
			glStencilFunc(GL_ALWAYS,1,1);
			glStencilOp(GL_REPLACE,GL_REPLACE,GL_REPLACE);
		}
		else
		{
			/* overlay ranges from -99 to 1 in z, hence it is at front of
				z-buffer */
			glOrtho(-max_x,max_x,-max_y,max_y,1.0,101.0);
		}
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(/*eye*/0.0,0.0,2.0, /*lookat*/0.0,0.0,0.0,
			/*up*/0.0,1.0,0.0);
		/* set up lights in overlay_scene */
		/* only lights in overlay_scene apply to it */
		reset_Lights();
		for_each_Light_in_Scene(scene_viewer->overlay_scene,
			execute_Light,(void *)NULL);
		/* render overlay_scene */
		switch (scene_viewer->transparency_mode)
		{
			default:
			{
				glAlphaFunc(GL_GREATER,0.0);
				execute_Scene(scene_viewer->overlay_scene);
			} break;
			case SCENE_VIEWER_SLOW_TRANSPARENCY:
			{
				glEnable(GL_ALPHA_TEST);
				/* render only fragments with alpha = 1.0, write depth */
				glDepthMask(GL_TRUE);
				glAlphaFunc(GL_EQUAL,1.0);
				execute_Scene(scene_viewer->overlay_scene);
				/* render fragments with alpha != 1.0; do not write into
					depth buffer */
				glDepthMask(GL_FALSE);
				glAlphaFunc(GL_NOTEQUAL,1.0);
				execute_Scene(scene_viewer->overlay_scene);
				glDepthMask(GL_TRUE);
				glDisable(GL_ALPHA_TEST);
			} break;
		}

 		if (0 < rendering_data->stencil_depth)
		{
			/* use full z-buffer for overlay ranging from -1 to 1 in z */
			glStencilFunc(GL_NOTEQUAL,1,1);
			glStencilOp(GL_KEEP,GL_KEEP,GL_KEEP);
		}

		glMatrixMode(GL_PROJECTION);
		glPopMatrix();

		/* Render everything else */
		Scene_viewer_call_next_renderer(rendering_data);

 		if (0 < rendering_data->stencil_depth)
		{
			/* disable stencil buffer */
			glDisable(GL_STENCIL_TEST);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "Scene_viewer_render_overlay_scene.  "
			"Invalid arguments");
		return_code = 0;
	}
	LEAVE;

	return (return_code);	
} /* Scene_viewer_render_overlay_scene */

static int Scene_viewer_initialise_matrices_and_swap_buffers(
		struct Scene_viewer_rendering_data *rendering_data)
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Scene_viewer *scene_viewer;

	ENTER(Scene_viewer_initialise_matrices_and_swap_buffers);
	if (rendering_data && (scene_viewer = rendering_data->scene_viewer))
	{
		return_code = 1;

		/* load identity matrix for rendering normal scene */
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
							
		Scene_viewer_call_next_renderer(rendering_data);

		if (rendering_data->rendering_double_buffered)
		{
			scene_viewer->swap_buffers=1;
		}
		else
		{
			scene_viewer->swap_buffers=0;
		}

		/* SAB  Reapply the projection matrix which was cleared by the 
			last glPopMatrix (Apply projection is further down the stack) 
			so that unproject gets the full transformation */
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glLoadMatrixd(scene_viewer->window_projection_matrix);

	}
	else
	{
		display_message(ERROR_MESSAGE, "Scene_viewer_initialise_matrices_and_swap_buffers.  "
			"Invalid arguments");
		return_code = 0;
	}
	LEAVE;

	return (return_code);	
} /* Scene_viewer_initialise_matrices_and_swap_buffers */

static int Scene_viewer_apply_modelview_lights_and_clip_planes(
		struct Scene_viewer_rendering_data *rendering_data)
/*******************************************************************************
LAST MODIFIED : 9 April 2003

DESCRIPTION :
==============================================================================*/
{
	int i, return_code;
	struct Scene_viewer *scene_viewer;

	ENTER(Scene_viewer_apply_modelview_turn_on_lights_and_clip_planes);
	if (rendering_data && (scene_viewer = rendering_data->scene_viewer))
	{
		return_code = 1;

		/* ModelView matrix */
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		reset_Lights();
		/* turn on lights that are part of the Scene_viewer,
			ie. headlamps */
		FOR_EACH_OBJECT_IN_LIST(Light)(execute_Light,(void *)NULL,
			scene_viewer->list_of_lights);
		
		glMultMatrixd(scene_viewer->modelview_matrix);
		/* turn on lights that are part of the Scene and fixed relative
			to it. Note the scene will have compiled them already. */
		for_each_Light_in_Scene(scene_viewer->scene,execute_Light,
			(void *)NULL);

		/* Clip planes */
		for (i = 0 ; i < MAX_CLIP_PLANES ; i++)
		{
			if (scene_viewer->clip_planes_enable[i])
			{
				switch(i)
				{
					case 0:
					{
						glEnable(GL_CLIP_PLANE0);
						glClipPlane(GL_CLIP_PLANE0,
							&(scene_viewer->clip_planes[i * 4]));
					} break;
					case 1:
					{
						glEnable(GL_CLIP_PLANE1);
						glClipPlane(GL_CLIP_PLANE1,
							&(scene_viewer->clip_planes[i * 4]));
					} break;
					case 2:
					{
						glEnable(GL_CLIP_PLANE2);
						glClipPlane(GL_CLIP_PLANE2,
							&(scene_viewer->clip_planes[i * 4]));
					} break;
					case 3:
					{
						glEnable(GL_CLIP_PLANE3);
						glClipPlane(GL_CLIP_PLANE3,
							&(scene_viewer->clip_planes[i * 4]));
					} break;
					case 4:
					{
						glEnable(GL_CLIP_PLANE4);
						glClipPlane(GL_CLIP_PLANE4,
							&(scene_viewer->clip_planes[i * 4]));
					} break;
					case 5:
					{
						glEnable(GL_CLIP_PLANE5);
						glClipPlane(GL_CLIP_PLANE5,
							&(scene_viewer->clip_planes[i * 4]));
					} break;
				}
			}
			else
			{
				switch(i)
				{
					case 0:
					{
						glDisable(GL_CLIP_PLANE0);
					} break;
					case 1:
					{
						glDisable(GL_CLIP_PLANE1);
					} break;
					case 2:
					{
						glDisable(GL_CLIP_PLANE2);
					} break;
					case 3:
					{
						glDisable(GL_CLIP_PLANE3);
					} break;
					case 4:
					{
						glDisable(GL_CLIP_PLANE4);
					} break;
					case 5:
					{
						glDisable(GL_CLIP_PLANE5);
					} break;
				}
			}
		}

		Scene_viewer_call_next_renderer(rendering_data);

	}
	else
	{
		display_message(ERROR_MESSAGE, "Scene_viewer_apply_modelview_lights_and_clip_planes.  "
			"Invalid arguments");
		return_code = 0;
	}
	LEAVE;

	return (return_code);	
} /* Scene_viewer_apply_modelview_lights_and_clip_planes */

static int Scene_viewer_handle_fastchanging(
	struct Scene_viewer_rendering_data *rendering_data)
/*******************************************************************************
LAST MODIFIED : 8 April 2003

DESCRIPTION :
This function determines whether to draw the main scene or whether to just
update the fastchanging objects.
==============================================================================*/
{
#if defined (OLD_CODE)
	GLdouble obj_x,obj_y,obj_z;
#endif /* defined (OLD_CODE) */
	int return_code;
#if defined (OLD_CODE)
	static GLint viewport[4]={0,0,1,1};
#endif /* defined (OLD_CODE) */
	struct Scene_viewer *scene_viewer;

	ENTER(Scene_viewer_handle_fastchanging);
	if (rendering_data && (scene_viewer = rendering_data->scene_viewer))
	{
		return_code = 1;
		if ((!rendering_data->rendering_double_buffered) || (!scene_viewer->fast_changing))
		{
			Scene_viewer_call_next_renderer(rendering_data);
		}

		if (scene_viewer->fast_changing ||
			Scene_has_fast_changing_objects(scene_viewer->scene))
		{
			scene_viewer->swap_buffers=0;
			/* Set up projection */
			glMatrixMode(GL_PROJECTION);
			glLoadMatrixd(scene_viewer->window_projection_matrix);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			reset_Lights();
			/* turn on lights that are part of the Scene_viewer,
				ie. headlamps */
			FOR_EACH_OBJECT_IN_LIST(Light)(execute_Light,(void *)NULL,
				scene_viewer->list_of_lights);
			glLoadMatrixd(scene_viewer->modelview_matrix);
			/* turn on lights that are part of the Scene and fixed relative
				to it. Note the scene will have compiled them already. */
			for_each_Light_in_Scene(scene_viewer->scene,execute_Light,
				(void *)NULL);

			glViewport((GLint)rendering_data->viewport_left,
				(GLint)rendering_data->viewport_bottom,
				(GLint)rendering_data->viewport_width,
				(GLint)rendering_data->viewport_height);
			/* do not write into the depth buffer */
			glDepthMask(GL_FALSE);

#if defined (OLD_CODE)
			if (rendering_data->rendering_double_buffered)
			{
				/* for OpenGL window z coordinates, 0.0=near_plane, 1.0=far */
				if (GL_TRUE==gluUnProject(0.0001,0.0001,0.0001,
						 scene_viewer->modelview_matrix,scene_viewer->window_projection_matrix,viewport,
						 &obj_x,&obj_y,&obj_z))
				{
					if (scene_viewer->first_fast_change &&
						scene_viewer->fast_changing)
					{
						/* copy front buffer to the back buffer */
						glReadBuffer(GL_FRONT);
						glDrawBuffer(GL_BACK);
					}
					else
					{
						/* copy back buffer to the front buffer */
						glReadBuffer(GL_BACK);
						glDrawBuffer(GL_FRONT);
					}
					/* Copy all the pixels irrespective of their alpha values */
					glDisable(GL_BLEND);
					glRasterPos3d(obj_x,obj_y,obj_z);
					glCopyPixels(0,0,rendering_data->viewport_width,
						rendering_data->viewport_height,GL_COLOR);
				}
				glDrawBuffer(GL_FRONT);
				glEnable(GL_BLEND);
			}
#endif /* defined (OLD_CODE) */
			execute_Scene_fast_changing(scene_viewer->scene);
			glFlush();
			scene_viewer->first_fast_change=0;
		}
		else
		{
			scene_viewer->first_fast_change=1;
		}
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);	
} /* Scene_viewer_handle_fastchanging */

static int Scene_viewer_render_background(
	struct Scene_viewer_rendering_data *rendering_data)
/*******************************************************************************
LAST MODIFIED : 8 April 2003

DESCRIPTION :
Renders the background into the scene.
==============================================================================*/
{
	int return_code;
	struct Scene_viewer *scene_viewer;

	ENTER(Scene_viewer_render_background);
	if (rendering_data && (scene_viewer = rendering_data->scene_viewer))
	{
		return_code = 1;

		/* clear the screen: colour buffer and depth buffer */
		glClearColor((scene_viewer->background_colour).red,
			(scene_viewer->background_colour).green,
			(scene_viewer->background_colour).blue,0.0);
		glClearDepth(1.0);
#if defined (WIN32_SYSTEM)
		/* Clear twice, if set then the glClear in the background will be called
			twice, which appears to work around a rendering bug on ATI windows driver 6.14.0010.6706 */
		if (1 == scene_viewer->clear_twice_flag)
		{
			const char *vendor_string = (const char *)glGetString(GL_VENDOR);
			/* Now that we have a current context check to see what the vendor is */ 
			if (vendor_string && !strcmp("ATI Technologies Inc.", vendor_string))
			{
				/* Don't check again */
				scene_viewer->clear_twice_flag = 2;
			}
			else
			{
				/* Don't need work around */
				scene_viewer->clear_twice_flag = 0;
			}
		}
		if (scene_viewer->clear_twice_flag)
		{
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}
#endif /* defined (WIN32_SYSTEM) */
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (scene_viewer->background_texture)
		{
			glDisable(GL_LIGHTING);
			glColor3f((scene_viewer->background_colour).red,
				(scene_viewer->background_colour).green,
				(scene_viewer->background_colour).blue);
			Scene_viewer_render_background_texture(scene_viewer,
				rendering_data->viewport_width,rendering_data->viewport_height);
			glEnable(GL_LIGHTING);
		}	

		Scene_viewer_call_next_renderer(rendering_data);
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);	
} /* Scene_viewer_render_background */

static int Scene_viewer_slow_transparency(
	struct Scene_viewer_rendering_data *rendering_data)
/*******************************************************************************
LAST MODIFIED : 4 April 2003

DESCRIPTION :
Render the scene twice.  Once with opaque objects filling the depth buffer
and then again with only semi transparent objects not changing the depth buffer.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_slow_transparency);
	if (rendering_data)
	{
		return_code = 1;
		glEnable(GL_ALPHA_TEST);
		/* render only fragments with alpha = 1.0, write depth */
		glDepthMask(GL_TRUE);
		glAlphaFunc(GL_EQUAL,1.0);
		Scene_viewer_call_next_renderer(rendering_data);

		/* render fragments with alpha != 1.0; do not write into
			depth buffer */
		glDepthMask(GL_FALSE);
		glAlphaFunc(GL_NOTEQUAL,1.0);
		Scene_viewer_call_next_renderer(rendering_data);

		glDepthMask(GL_TRUE);
		glDisable(GL_ALPHA_TEST);
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);	
} /* Scene_viewer_slow_transparency */

static int Scene_viewer_layered_transparency(
	struct Scene_viewer_rendering_data *rendering_data)
/*******************************************************************************
LAST MODIFIED : 4 April 2003

DESCRIPTION :
Render the scene twice.  Once with opaque objects filling the depth buffer
and then again with only semi transparent objects not changing the depth buffer.
==============================================================================*/
{
	GLdouble temp_matrix[16];
	int layer,layers,return_code;

	ENTER(Scene_viewer_layered_transparency);
	if (rendering_data)
	{
		return_code = 1;

		glMatrixMode(GL_PROJECTION);
		glGetDoublev(GL_PROJECTION_MATRIX,temp_matrix);

		/* Draw each layer separately to help transparency */
		layers = rendering_data->override_transparency_layers;
		for (layer = 0 ; layer < layers ; layer++)
		{		
			glMatrixMode(GL_PROJECTION);

			glLoadIdentity();

			glTranslated(0.0, 0.0, (double)layer * 2.0
				- (double)(layers - 1));
			glScaled(1.0, 1.0, (double)layers);

			glMultMatrixd(temp_matrix);

			glDepthRange((double)(layers - layer - 1) / (double)layers,
				(double)(layers - layer) / (double)layers);

			Scene_viewer_call_next_renderer(rendering_data);

			glDepthRange((GLclampd)0.0,(GLclampd)1.0);
		}
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_layered_transparency */

static int Scene_viewer_antialias(
	struct Scene_viewer_rendering_data *rendering_data)
/*******************************************************************************
LAST MODIFIED : 7 April 2003

DESCRIPTION :
Render the scene multiple times perturbing the viewing frustrum to antialias
the entire scene.
==============================================================================*/
{
	double pixel_offset_x, pixel_offset_y;
	GLdouble temp_matrix[16];
	int accumulation_count, antialias, return_code;
	float j2[2][2]=
		{
			{0.25,0.75},
			{0.75,0.25}
		};
	float j4[4][2]=
		{
			{0.375,0.25},
			{0.125,0.75},
			{0.875,0.25},
			{0.625,0.75}
		};
	float j8[8][2]=
		{
			{0.5625,0.4375},
			{0.0625,0.9375},
			{0.3125,0.6875},
			{0.6875,0.8125},
			{0.8125,0.1875},
			{0.9375,0.5625},
			{0.4375,0.0625},
			{0.1875,0.3125}
		};

	ENTER(Scene_viewer_antialias);
	if (rendering_data)
	{
		return_code = 1;
		antialias = rendering_data->override_antialias;
		for (accumulation_count = 0 ; accumulation_count < antialias ;
			  accumulation_count++)
		{
			glMatrixMode(GL_PROJECTION);

			/* SAB This should more robustly be a glPushMatrix and glPopMatrix
				pair but the SGI implementations sometimes have only one
				level on the Projection matrix stack and so knowing that this
				matrix is the identity so far I can just reload that */
			glLoadIdentity();

			/********* CALCULATE ANTIALIAS OFFSET MATRIX *********/
			switch(antialias)
			{
				case 0:
				case 1:
				{
					/* Do nothing */
				} break;
				case 2:
				{
					pixel_offset_x=j2[accumulation_count][0]-0.5;
					pixel_offset_y=j2[accumulation_count][1]-0.5;
				} break;
				case 4:
				{
					pixel_offset_x=j4[accumulation_count][0]-0.5;
					pixel_offset_y=j4[accumulation_count][1]-0.5;
				} break;
				case 8:
				{
					pixel_offset_x=j8[accumulation_count][0]-0.5;
					pixel_offset_y=j8[accumulation_count][1]-0.5;
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"Scene_viewer_antialias.  "
						"Invalid antialias number");
					return_code=0;
				} break;
			}
			/* the projection matrix converts the viewing volume into the
				Normalised Device Coordinates ranging from -1 to +1 in each
				coordinate direction. Need to scale this range to fit the
				viewport by premultiplying with a matrix. First start with
				identity: Note that numbers go down columns first in OpenGL
				matrices */
			temp_matrix[0] = 1.0;
			temp_matrix[1] = 0.0;
			temp_matrix[2] = 0.0;
			temp_matrix[3] = 0.0;
		
			temp_matrix[4] = 0.0;
			temp_matrix[5] = 1.0;
			temp_matrix[6] = 0.0;
			temp_matrix[7] = 0.0;
		
			temp_matrix[8] = 0.0;
			temp_matrix[9] = 0.0;
			temp_matrix[10] = 1.0;
			temp_matrix[11] = 0.0;
			/* offsetting image by [sub]pixel distances for anti-aliasing.
				offset_x is distance image is shifted to the right, offset_y is
				distance image is shifted up. The actual offsets used are
				fractions of half the viewport width or height,since normalized
				device coordinates (NDCs) range from -1 to +1 */
			temp_matrix[12] = 2.0*pixel_offset_x/rendering_data->viewport_width;
			temp_matrix[13] = 2.0*pixel_offset_y/rendering_data->viewport_height;
			temp_matrix[14] = 0.0;
			temp_matrix[15] = 1.0;
			
			glMultMatrixd(temp_matrix);

			Scene_viewer_call_next_renderer(rendering_data);
			
			if (0==accumulation_count)
			{
				glAccum(GL_LOAD,1.0f/((GLfloat)antialias));
			}
			else
			{
				glAccum(GL_ACCUM,1.0f/((GLfloat)antialias));
			}
			
		} /* for (antialias_count) */

		/* We want to ensure that we return white when we accumulate a white
			background */
		glAccum(GL_RETURN,1.001f);
		glFlush();
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);	
} /* Scene_viewer_antialias */

static int Scene_viewer_depth_of_field(
	struct Scene_viewer_rendering_data *rendering_data)
/*******************************************************************************
LAST MODIFIED : 5 December 2006

DESCRIPTION :
Render the scene multiple times perturbing the viewing frustrum to create a
depth of field effect.
==============================================================================*/
{
	double depth_of_field, dx, dy, focal_depth, pixel_offset_x, pixel_offset_y;
	GLdouble temp_matrix[16];
	int accumulation_count, return_code;
	float j8[8][2]=
		{
			{0.5625,0.4375},
			{0.0625,0.9375},
			{0.3125,0.6875},
			{0.6875,0.8125},
			{0.8125,0.1875},
			{0.9375,0.5625},
			{0.4375,0.0625},
			{0.1875,0.3125}
		};

	ENTER(Scene_viewer_antialias);
	if (rendering_data)
	{
		return_code = 1;
		for (accumulation_count = 0 ; accumulation_count < 8 ;
			  accumulation_count++)
		{
			glMatrixMode(GL_PROJECTION);

			/* SAB This should more robustly be a glPushMatrix and glPopMatrix
				pair but the SGI implementations sometimes have only one
				level on the Projection matrix stack and so knowing that this
				matrix is the identity so far I can just reload that */
			glLoadIdentity();

			pixel_offset_x=j8[accumulation_count][0];
			pixel_offset_y=j8[accumulation_count][1];

			focal_depth = rendering_data->scene_viewer->focal_depth;
			depth_of_field = rendering_data->scene_viewer->depth_of_field;

			dx = pixel_offset_x/(depth_of_field*rendering_data->viewport_width);
			dy = pixel_offset_y/(depth_of_field*rendering_data->viewport_height);

			/* the projection matrix converts the viewing volume into the
				Normalised Device Coordinates ranging from -1 to +1 in each
				coordinate direction. Need to scale this range to fit the
				viewport by premultiplying with a matrix. First start with
				identity: Note that numbers go down columns first in OpenGL
				matrices */
			temp_matrix[0] = 1.0;
			temp_matrix[1] = 0.0;
			temp_matrix[2] = 0.0;
			temp_matrix[3] = 0.0;
		
			temp_matrix[4] = 0.0;
			temp_matrix[5] = 1.0;
			temp_matrix[6] = 0.0;
			temp_matrix[7] = 0.0;
		
			temp_matrix[8] = dx / (1.0 - focal_depth);
			temp_matrix[9] = dy / (1.0 - focal_depth);
			temp_matrix[10] = 1.0;
			temp_matrix[11] = 0.0;
			/* offsetting image by [sub]pixel distances for anti-aliasing.
				offset_x is distance image is shifted to the right, offset_y is
				distance image is shifted up. The actual offsets used are
				fractions of half the viewport width or height,since normalized
				device coordinates (NDCs) range from -1 to +1 */
			temp_matrix[12] = -dx * focal_depth;
			temp_matrix[13] = -dy * focal_depth;
			temp_matrix[14] = 0.0;
			temp_matrix[15] = 1.0;
			
			glMultMatrixd(temp_matrix);

			Scene_viewer_call_next_renderer(rendering_data);
			
			if (0==accumulation_count)
			{
				glAccum(GL_LOAD,1.0f/(8.0f));
			}
			else
			{
				glAccum(GL_ACCUM,1.0f/(8.0f));
			}
			
		} /* for (antialias_count) */

		/* We want to ensure that we return white when we accumulate a white
			background */
		glAccum(GL_RETURN,1.001f);
		glFlush();
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);	
} /* Scene_viewer_depth_of_field */

static int Scene_viewer_stereo(
	struct Scene_viewer_rendering_data *rendering_data)
/*******************************************************************************
LAST MODIFIED : 8 April 2003

DESCRIPTION :
Render the scene into the LEFT and RIGHT buffers perturbed slightly to generate
a stereo perspective.
==============================================================================*/
{
	double eye_distance,stereo_angle,stereo_cos,stereo_sin,view[3];
	int return_code;
	GLdouble stereo_matrix[16];
	struct Scene_viewer *scene_viewer;

	ENTER(Scene_viewer_stereo);
	if (rendering_data && (scene_viewer = rendering_data->scene_viewer))
	{
		return_code = 1;
		
		/* Calculate the angle */
		view[0] = scene_viewer->eyex - scene_viewer->lookatx;
		view[1] = scene_viewer->eyey - scene_viewer->lookaty;
		view[2] = scene_viewer->eyez - scene_viewer->lookatz;
		eye_distance = norm3(view);

		stereo_angle = 2.0*atan2(0.5*scene_viewer->stereo_eye_spacing,
			eye_distance);
		stereo_sin = sin(stereo_angle);
		stereo_cos = cos(stereo_angle);

		glMatrixMode(GL_PROJECTION);
			
		glPushMatrix();

		if (rendering_data->rendering_double_buffered)
		{
			glDrawBuffer(GL_BACK_LEFT);
		}
		else
		{
			glDrawBuffer(GL_FRONT_LEFT);
		}

		/* the projection matrix converts the viewing volume into the
			Normalised Device Coordinates ranging from -1 to +1 in each
			coordinate direction. Need to scale this range to fit the
			viewport by premultiplying with a matrix. First start with
			identity: Note that numbers go down columns first in OpenGL
			matrices */
		stereo_matrix[0] = stereo_cos;
		stereo_matrix[1] = 0.0;
		stereo_matrix[2] = stereo_sin;
		stereo_matrix[3] = 0.0;

		stereo_matrix[4] = 0.0;
		stereo_matrix[5] = 1.0;
		stereo_matrix[6] = 0.0;
		stereo_matrix[7] = 0.0;

		stereo_matrix[8] = -stereo_sin;
		stereo_matrix[9] = 0.0;
		stereo_matrix[10] = stereo_cos;
		stereo_matrix[11] = 0.0;

		stereo_matrix[12] = 0.0;
		stereo_matrix[13] = 0.0;
		stereo_matrix[14] = 0.0;
		stereo_matrix[15] = 1.0;

		glMultMatrixd(stereo_matrix);

		Scene_viewer_call_next_renderer(rendering_data);

		glMatrixMode(GL_PROJECTION);

		glPopMatrix();

		if (rendering_data->rendering_double_buffered)
		{
			glDrawBuffer(GL_BACK_RIGHT);
		}
		else
		{
			glDrawBuffer(GL_FRONT_RIGHT);
		}

		/* the projection matrix converts the viewing volume into the
			Normalised Device Coordinates ranging from -1 to +1 in each
			coordinate direction. Need to scale this range to fit the
			viewport by premultiplying with a matrix. First start with
			identity: Note that numbers go down columns first in OpenGL
			matrices */
		stereo_matrix[0] = stereo_cos;
		stereo_matrix[1] = 0.0;
		stereo_matrix[2] = -stereo_sin;
		stereo_matrix[3] = 0.0;

		stereo_matrix[4] = 0.0;
		stereo_matrix[5] = 1.0;
		stereo_matrix[6] = 0.0;
		stereo_matrix[7] = 0.0;

		stereo_matrix[8] = stereo_sin;
		stereo_matrix[9] = 0.0;
		stereo_matrix[10] = stereo_cos;
		stereo_matrix[11] = 0.0;

		stereo_matrix[12] = 0.0;
		stereo_matrix[13] = 0.0;
		stereo_matrix[14] = 0.0;
		stereo_matrix[15] = 1.0;
			
		glMultMatrixd(stereo_matrix);

		Scene_viewer_call_next_renderer(rendering_data);
	}
	else
	{
		return_code = 0;
	}
	LEAVE;
	
	return (return_code);	
} /* Scene_viewer_stereo */

#if defined (USE_PER_PIXEL_LIGHTING)
static int Scene_viewer_per_pixel_lighting(
	struct Scene_viewer_rendering_data *rendering_data)
/*******************************************************************************
LAST MODIFIED : 4 April 2003

DESCRIPTION :
Render the scene twice.  Once with opaque objects filling the depth buffer
and then again with only semi transparent objects not changing the depth buffer.
==============================================================================*/
{
	int return_code;
	static GLuint vertex_program;
	GLuint texture0_index;
	char vertex_program_code[] = 
		"!!VP1.0 # This is the vertex program\n"
		"# Constant memory data:\n"
		"# c[0]...c[3]      contains the modelview projection composite matrix\n"
		"# c[4]...c[7]    contains the inverse modelview matrix\n"
		"# c[8]            contains the light direction.\n"
		"# c[9]            contains useful constants ( 0.5, 1.0, 2.0, 0.0 )\n"
		"#\n"
		"# Per-Vertex Attribute data:\n"
		"# v[OPOS] contains the per-vertex position\n"
		"# v[TEX0] contains the per-vertex texture coordinates for texture units 0.\n"
		"# v[NRML] contains the per-vertex normal\n"
		"# v[1]    contains the tangent\n"
		"# v[3]    contains the binormal\n"
		"  \n"
		"# Transform vertices to homogeneous clip space.\n"
		"# Transform the vertex by the MODELVIEW and PROJECTION matrix\n"
		"# and store the result in the vertex output register.\n"
		"DP4   o[HPOS].x, c[0], v[OPOS];\n"
		"DP4   o[HPOS].y, c[1], v[OPOS];\n"
		"DP4   o[HPOS].z, c[2], v[OPOS];\n"
		"DP4   o[HPOS].w, c[3], v[OPOS];\n"
		"\n"
		"# Put the normal into the primary colour.\n"
		"MOV   R2,   v[NRML];\n"
		"MOV   R2.w,   c[9].y    ; # w = 1\n"
		"# Range compress since COL0 will clamp(0,1)\n"
		"MAD   o[COL0],  R2, c[9].x, c[9].x;\n"
		"\n"
		"# Put the light direction into the secondary colour"
		"# Range compress since COL1 will clamp(0,1)\n"
		"MAD   o[COL1],  c[8], c[9].x, c[9].x;\n"
		"\n"
		"# Assign texture coordinates for texture unit 0.\n"
		"MOV   o[TEX0], v[TEX0];\n"
		"MOV   o[TEX1], v[TEX0];\n"
		"\n"
		"END\n";
	char vertex_program_code4[] = 
		"!!VP1.0 # This is the vertex program\n"
		"# Constant memory data:\n"
		"# c[0]...c[3]      contains the modelview projection composite matrix\n"
		"# c[4]...c[7]    contains the inverse modelview matrix\n"
		"# c[8]            contains the eye-space light position.\n"
		"# c[9]            contains useful constants ( 0.5, 1.0, 2.0, 0.0 )\n"
		"#\n"
		"# Per-Vertex Attribute data:\n"
		"# v[OPOS] contains the per-vertex position\n"
		"# v[TEX0] contains the per-vertex texture coordinates for texture units 0.\n"
		"# v[NRML] contains the per-vertex normal\n"
		"# v[1]    contains the tangent\n"
		"# v[3]    contains the binormal\n"
		"  \n"
		"# Transform vertices to homogeneous clip space.\n"
		"# Transform the vertex by the MODELVIEW and PROJECTION matrix\n"
		"# and store the result in the vertex output register.\n"
		"DP4   o[HPOS].x, c[0], v[OPOS];\n"
		"DP4   o[HPOS].y, c[1], v[OPOS];\n"
		"DP4   o[HPOS].z, c[2], v[OPOS];\n"
		"DP4   o[HPOS].w, c[3], v[OPOS];\n"
		"\n"
		"# Compute the light position in model space.\n"
		"# Transform the light in register c[16] from eye-space to model space\n"
		"# using the inverse modelview matrix.\n"
		"MOV   R0, c[8];\n"
		"DP4   R1.x, c[4], R0;\n"
		"DP4   R1.y, c[5], R0;\n"
		"DP4   R1.z, c[6], R0;\n"
		"\n"
		"# Compute L = light position - vertex position (in model space)\n"
		"ADD   R2.xyz, R1, -v[OPOS];\n"
		"DP3   R2.w, R2, R2;\n"
		"RSQ   R2.w, R2.w;\n"
		"MUL   R2.xyz, R2, R2.w;\n"
		"\n"
		"# Compute H = halfangle vector (in model space)\n"
		"MOV   R0, c[9].wwwy;\n"
		"DP4   R3.x, c[4], R0;\n"
		"DP4   R3.y, c[5], R0;\n"
		"DP4   R3.z, c[6], R0;\n"
		"ADD   R3.xyz, R3, -v[OPOS];\n"
		"DP3   R3.w, R3, R3;\n"
		"RSQ   R3.w, R3.w;\n"
		"MUL   R3.xyz, R3, R3.w;\n"
		"\n"
		"# H = normalized(V+L)\n"
		"ADD   R4.xyz, R2, R3;\n"
		"DP3   R4.w, R4, R4;\n"
		"RSQ   R4.w, R4.w;\n"
		"MUL   R4.xyz, R4, R4.w;\n"
		"\n"
		/*
		"# Transform L to local tangent space.\n"
		"# Store the local tangent space light vector in primary\n"
		"# color (col0) so that in the register combiners we can\n"
		"# use the z-component for self-shadowing.\n"
		"DP3   R3.x, v[1], R2;\n"
		"DP3   R3.y, v[3], R2;\n"
		"DP3   R3.z, v[NRML], R2;\n"
		*/

		"MOV   R2.w,   c[9].y    ; # w = 1\n"
		"# Range compress since COL0 will clamp(0,1)\n"
		"MAD   o[COL0],  R2, c[9].x, c[9].x;\n"
		"\n"

		/*
		"# Transform H to local tangent space.\n"
		"DP3   R5.x, v[1], R4;\n"
		"DP3   R5.y, v[3], R4;\n"
		"DP3   R5.z, v[NRML], R4;\n"

		"# Range compress since COL1 will clamp(0,1)\n"
		"MAD   o[COL1],  R4, c[9].x, c[9].x;\n"
		"\n"
		*/
		"# Assign texture coordinates for texture unit 0.\n"
		"MOV   o[TEX0], v[TEX0];\n"
		"MOV   o[TEX1], v[TEX0];\n"
		"\n"
		"END\n";
	char vertex_program_code_old[] = 
		"!!VP1.0\n"
		"\n"
		"# First we multiply the vertex with the modelview and projection matrices.\n"
		"# These are stored in in c[0] to c[3].\n"
		"DP4 R6.x, v[OPOS], c[0];\n"
		"DP4 R6.y, v[OPOS], c[1];\n"
		"DP4 R6.z, v[OPOS], c[2];\n"
		"DP4 R6.w, v[OPOS], c[3];\n"
		"MOV o[HPOS], R6;\n"
		"# Now we calculate a vector from the light source (c[4]) to this vertex, and\n"
		"# normalize it. The result is stored in the output's primary color.\n"
		"ADD R0, -c[4], R6;\n"
		"# Vector length squared:\n"
		"DP3 R1, R0, R0;\n"
		"# Reciprocal square root of length:\n"
		"RSQ R2, R1.x;\n"
		"# Normalize:\n"
		"MUL R3, R0, R2;\n"
		"# Scale and bias into [0, 1] range:\n"
		"MUL R4, R3, c[5].yyyy;\n"
		"ADD R5, R4, c[5].yyyy;\n"
		"\n"
		"MOV o[COL0], R5;\n"
		"MOV o[TEX0], v[TEX0];\n"
		"MOV o[TEX1], v[TEX1];\n"
		"\n"
		"END\n";
	char vertex_program_code3[] = 
		"!!VP1.0\n"
		"\n"
		"# First we multiply the vertex with the modelview and projection matrices.\n"
		"# These are stored in in c[0] to c[3].\n"
		"DP4 o[HPOS].x, v[OPOS], c[0];\n"
		"DP4 o[HPOS].y, v[OPOS], c[1];\n"
		"DP4 o[HPOS].z, v[OPOS], c[2];\n"
		"DP4 o[HPOS].w, v[OPOS], c[3];\n"
		"# R1 = normalize eye-space normal\n"
		"DP3   R1.x,   c[4], v[NRML] ;\n"
		"DP3   R1.y,   c[5], v[NRML] ;\n"
		"DP3   R1.z,   c[6], v[NRML] ;\n"
		"\n"
		"DP3   R1.z,   c[8], R1 ;\n"
		"\n"
		"DP3   R1.w,   R1, R1     ;\n"
		"RSQ   R1.w,   R1.w       ;\n"
		"MUL   R1.xyz, R1, R1.w   ;\n"
		"\n"
		"MOV   R1.w,   c[9].z    ; # w = 1\n"
		"\n"
		"MOV o[COL0], R1;\n"
		"MOV o[TEX0], v[TEX0];\n"
		"MOV o[TEX1], v[TEX1];\n"
		"\n"
		"END\n";
	char vertex_program_code_extra[] = 
      "!!VP1.0\n"
	  "                                                        \n"
	  "# set the position                                      \n"
	  "DP4   o[HPOS].x, c[0], v[OPOS] ;                        \n"
	  "DP4   o[HPOS].y, c[1], v[OPOS] ;                        \n"
	  "DP4   o[HPOS].z, c[2], v[OPOS] ;                        \n"
	  "DP4   o[HPOS].w, c[3], v[OPOS] ;                        \n"
	  "                                                        \n"
	  "# c[5] == light_direction                               \n"
	  "# o[TEX1].x = dp3(light_direction, v[1])                \n"
	  "# o[TEX1].y = dp3(light_direction, v[2])                \n"
	  "# o[TEX1].z = dp3(light_direction, v[3])                \n"
      "DP3   o[TEX1].x,   c[5], v[1]       ;                   \n"
      "DP3   o[TEX1].y,   c[5], v[2]       ;                   \n"
      "DP3   o[TEX1].z,   c[5], v[3]       ;                   \n"
	  "                                                        \n"
	  "# c[6] == halfangle_direction    *                       \n"
	  "# o[TEX2].x = dp3(halfangle_direction, v[1])            \n"
	  "# o[TEX2].y = dp3(halfangle_direction, v[2])            \n"
	  "# o[TEX2].z = dp3(halfangle_direction, v[3])            \n"
      "DP3   o[TEX2].x,   c[6], v[1]       ;                   \n"
      "DP3   o[TEX2].y,   c[6], v[2]       ;                   \n"
      "DP3   o[TEX2].z,   c[6], v[3]       ;                   \n"
	  "                                                        \n"
	  "# pass the same texture coordinates through             \n"
	  "# to both tex0 and tex1                                 \n"
	  "MOV o[TEX0], v[TEX0]         ;                          \n"
	  "MOV o[TEX3], v[TEX0]         ;                          \n"
	  "                                                        \n"
	  "END";
	/* GLfloat light_position[4] = {0, -250, -250, 0}; */
	GLfloat light_position[4] = {0, 0.75, 0.75, 1.0};
	GLfloat vertex_program_constants[4] = {0.5, 1, 2, 0};

	ENTER(Scene_viewer_per_pixel_lighting);
	if (scene_viewer)
	{
		return_code = 1;

		glEnable(GL_VERTEX_PROGRAM_NV);
		if (!vertex_program)
		{
			glGenProgramsNV(1, &vertex_program);
			glBindProgramNV(GL_VERTEX_PROGRAM_NV, vertex_program);
			glLoadProgramNV(GL_VERTEX_PROGRAM_NV, vertex_program,
				strlen(vertex_program_code), vertex_program_code);
						
			glTrackMatrixNV(GL_VERTEX_PROGRAM_NV, 0, GL_MODELVIEW_PROJECTION_NV, GL_IDENTITY_NV);
			glTrackMatrixNV(GL_VERTEX_PROGRAM_NV, 4, GL_MODELVIEW, GL_INVERSE_TRANSPOSE_NV);
						
			glProgramParameter4fvNV(GL_VERTEX_PROGRAM_NV, 8, light_position);
			glProgramParameter4fvNV(GL_VERTEX_PROGRAM_NV, 9, vertex_program_constants);
		}
		else
		{
			glBindProgramNV(GL_VERTEX_PROGRAM_NV, vertex_program);
		}

		{
			glEnable(GL_REGISTER_COMBINERS_NV);
				
			glCombinerParameteriNV(GL_NUM_GENERAL_COMBINERS_NV, 2);
			glCombinerInputNV(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_A_NV, GL_TEXTURE0_ARB, GL_EXPAND_NORMAL_NV, GL_RGB);
			glCombinerInputNV(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_B_NV, GL_PRIMARY_COLOR_NV, GL_EXPAND_NORMAL_NV, GL_RGB);
			glCombinerInputNV(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_C_NV, GL_TEXTURE0_ARB, GL_EXPAND_NORMAL_NV, GL_RGB);
			glCombinerInputNV(GL_COMBINER0_NV, GL_RGB, GL_VARIABLE_D_NV, GL_SECONDARY_COLOR_NV, GL_EXPAND_NORMAL_NV, GL_RGB);
			glCombinerOutputNV(GL_COMBINER0_NV, GL_RGB, GL_SPARE0_NV, GL_SPARE1_NV, GL_DISCARD_NV, GL_NONE, GL_NONE, GL_TRUE, GL_TRUE, GL_FALSE);
				
				
			glCombinerInputNV(GL_COMBINER1_NV, GL_RGB, GL_VARIABLE_A_NV, GL_SPARE1_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
			glCombinerInputNV(GL_COMBINER1_NV, GL_RGB, GL_VARIABLE_B_NV, GL_SPARE1_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
			glCombinerOutputNV(GL_COMBINER1_NV, GL_RGB, GL_SPARE1_NV, GL_DISCARD_NV, GL_DISCARD_NV, GL_NONE, GL_NONE, GL_FALSE, GL_FALSE, GL_FALSE);
				
			glFinalCombinerInputNV(GL_VARIABLE_A_NV, GL_E_TIMES_F_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
			glFinalCombinerInputNV(GL_VARIABLE_B_NV, GL_E_TIMES_F_NV, GL_UNSIGNED_INVERT_NV, GL_RGB);
			glFinalCombinerInputNV(GL_VARIABLE_C_NV, GL_ZERO, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
			glFinalCombinerInputNV(GL_VARIABLE_D_NV, GL_SPARE0_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
			glFinalCombinerInputNV(GL_VARIABLE_E_NV, GL_SPARE1_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
			glFinalCombinerInputNV(GL_VARIABLE_F_NV, GL_SPARE1_NV, GL_UNSIGNED_IDENTITY_NV, GL_RGB);
			glFinalCombinerInputNV(GL_VARIABLE_G_NV, GL_ZERO, GL_UNSIGNED_IDENTITY_NV, GL_ALPHA);
		}

		/* Now just render scene simply */
		Scene_viewer_call_next_renderer(rendering_data);
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);	
} /* Scene_viewer_per_pixel_lighting */
#endif /* defined (USE_PER_PIXEL_LIGHTING) */

static int Scene_viewer_initialise_order_independent_transparency(
	struct Scene_viewer_rendering_data *rendering_data)
/*******************************************************************************
LAST MODIFIED : 14 April 2003

DESCRIPTION :
Render the scene twice.  Once with opaque objects filling the depth buffer
and then again with only semi transparent objects not changing the depth buffer.
==============================================================================*/
{
	int return_code;
	struct Scene_viewer *scene_viewer;

	ENTER(Scene_viewer_initialise_order_independent_transparency);
	if (rendering_data && (scene_viewer = rendering_data->scene_viewer))
	{
		if (!scene_viewer->order_independent_transparency_data)
		{
			scene_viewer->order_independent_transparency_data = 
				order_independent_initialise(scene_viewer);
		}

		if (scene_viewer->order_independent_transparency_data)
		{
			order_independent_reshape(scene_viewer->order_independent_transparency_data,
				rendering_data->viewport_width, rendering_data->viewport_height,
				rendering_data->override_transparency_layers,
				(rendering_data->stencil_depth > 0));
			return_code = 1;
		}
		else
		{
			return_code = 0;
		}
		
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);	
} /* Scene_viewer_initialise_order_independent_transparency */

static int Scene_viewer_order_independent_transparency(
	struct Scene_viewer_rendering_data *rendering_data)
/*******************************************************************************
LAST MODIFIED : 7 April 2003

DESCRIPTION :
Render the scene twice.  Once with opaque objects filling the depth buffer
and then again with only semi transparent objects not changing the depth buffer.
==============================================================================*/
{
	int return_code;
	struct Scene_viewer *scene_viewer;

	ENTER(Scene_viewer_order_independent_transparency);
	if (rendering_data && (scene_viewer = rendering_data->scene_viewer))
	{
		return_code = 1;

		if (scene_viewer->order_independent_transparency_data)
		{
			order_independent_display(rendering_data, 
				scene_viewer->order_independent_transparency_data,
				scene_viewer->window_projection_matrix,
				scene_viewer->modelview_matrix, scene_viewer->blending_mode);
			
			if (rendering_data->rendering_double_buffered)
			{
				scene_viewer->swap_buffers=1;
			}
			else
			{
				scene_viewer->swap_buffers=0;
			}
		}
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);	
} /* Scene_viewer_order_independent_transparency */

static int Scene_viewer_render_scene_private(struct Scene_viewer *scene_viewer,
	int left, int bottom, int right, int top,
	int override_antialias, int override_transparency_layers)
/*******************************************************************************
LAST MODIFIED : 17 September 2002

DESCRIPTION :
Called to redraw the Scene_viewer scene after changes in the display lists or
viewing transformations.
<left>, <right>, <top> and <bottom> define the viewport to draw into, if they
are all zero then the scene_viewer->widget size is used instead.
If <override_antialias> or <override_transparency_layers> are non zero 
then they override the default values for just this call.
There are convienience functions, Scene_viewer_render_scene,
Scene_viewer_render_scene_with_picking, Scene_viewer_render_scene_in_viewport to
access this function.
==============================================================================*/
{
	GLboolean double_buffer;
	int do_render,return_code;
	struct Scene_viewer_rendering_data rendering_data;
	struct Scene_viewer_render_object *render_object;

	ENTER(Scene_viewer_render_scene_private);
	if (scene_viewer)
	{
		return_code=1;
		if ((!left) && (!bottom) && (!right) && (!top))
		{
			rendering_data.viewport_left = Graphics_buffer_get_origin_x(scene_viewer->graphics_buffer);
			rendering_data.viewport_bottom = Graphics_buffer_get_origin_y(scene_viewer->graphics_buffer);
			rendering_data.viewport_width = Graphics_buffer_get_width(scene_viewer->graphics_buffer);
			rendering_data.viewport_height = Graphics_buffer_get_height(scene_viewer->graphics_buffer);
		}
		else
		{
			rendering_data.viewport_left = left;
			rendering_data.viewport_bottom = bottom;
			rendering_data.viewport_width = right - left;
			rendering_data.viewport_height = top - bottom;
		}

#if defined (DEBUG)
		printf ("Viewport data %d,%d %d,%d\n",
			rendering_data.viewport_left, rendering_data.viewport_bottom,
			rendering_data.viewport_width, rendering_data.viewport_height);
#endif /* defined (DEBUG) */


		rendering_data.scene_viewer = scene_viewer;
		rendering_data.render_callstack = 
			(struct LIST(Scene_viewer_render_object) *)NULL;
		if (override_antialias > 0)
		{
			rendering_data.override_antialias = override_antialias;
		}
		else
		{
			rendering_data.override_antialias = scene_viewer->antialias;
		}
		if (override_transparency_layers > 0)
		{
			rendering_data.override_transparency_layers = override_transparency_layers;
		}
		else
		{
			rendering_data.override_transparency_layers = scene_viewer->transparency_layers;
		}		
		/* Set further down */
		rendering_data.rendering_double_buffered = 0;
		rendering_data.stencil_depth = 0;
		
		/* only redraw if the drawing widget has area and neither it nor any of its
			 parents are unmanaged */
		do_render=(0<rendering_data.viewport_width)&&(0<rendering_data.viewport_height)&&
			Graphics_buffer_is_visible(scene_viewer->graphics_buffer);
		if (do_render)
		{
			/* Calculate the transformations before doing the callback list */
			Scene_viewer_calculate_transformation(scene_viewer,
				rendering_data.viewport_width,rendering_data.viewport_height);

			/* Send the transform callback before compiling scene if the flag is set */
			if (scene_viewer->transform_flag)
			{
				CMISS_CALLBACK_LIST_CALL(Scene_viewer_callback)(
					scene_viewer->transform_callback_list,scene_viewer,NULL);
				scene_viewer->transform_flag = 0;
			}

			/* work out if the rendering is double buffered. Do not just look at
				the buffer_mode flag as it is overridden in cases such as printing
				the window. */				
			glGetBooleanv(GL_DOUBLEBUFFER,&double_buffer);
			/* Make this visible to the rendering routines */
			rendering_data.rendering_double_buffered = double_buffer;

			build_Scene(scene_viewer->scene);
			compile_Scene(scene_viewer->scene, scene_viewer->graphics_buffer);

			if (scene_viewer->overlay_scene)
			{
				build_Scene(scene_viewer->overlay_scene);
				compile_Scene(scene_viewer->overlay_scene, scene_viewer->graphics_buffer);
			}

			rendering_data.render_callstack = CREATE(LIST(Scene_viewer_render_object))();
			/* Add functionality to the render callstack */
			
			if (SCENE_VIEWER_NO_INPUT_OR_DRAW==scene_viewer->input_mode)
			{
				glClearColor(0.6,0.6,0.6,0.);
				glClearDepth(1.0);
				glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
				return_code=1;
			}
			else
			{
				if (SCENE_VIEWER_PIXEL_BUFFER==scene_viewer->buffering_mode)
				{
					render_object = CREATE(Scene_viewer_render_object)(
						Scene_viewer_use_pixel_buffer);
					ADD_OBJECT_TO_LIST(Scene_viewer_render_object)(render_object,
						rendering_data.render_callstack);
				}
	
				if (rendering_data.rendering_double_buffered)
				{
					render_object = CREATE(Scene_viewer_render_object)(
						Scene_viewer_handle_fastchanging);
					ADD_OBJECT_TO_LIST(Scene_viewer_render_object)(render_object,
						rendering_data.render_callstack);
				}

				/* Initialise the matricies and handle the double buffer flag */
				render_object = CREATE(Scene_viewer_render_object)(
					Scene_viewer_initialise_matrices_and_swap_buffers);
				ADD_OBJECT_TO_LIST(Scene_viewer_render_object)(render_object,
					rendering_data.render_callstack);

				if (rendering_data.override_antialias > 1)
				{
					render_object = CREATE(Scene_viewer_render_object)(
						Scene_viewer_antialias);
					ADD_OBJECT_TO_LIST(Scene_viewer_render_object)(render_object,
						rendering_data.render_callstack);
				}

				if (scene_viewer->depth_of_field > 0.0)
				{
					render_object = CREATE(Scene_viewer_render_object)(
						Scene_viewer_depth_of_field);
					ADD_OBJECT_TO_LIST(Scene_viewer_render_object)(render_object,
						rendering_data.render_callstack);
				}

				/* Render the background */
				render_object = CREATE(Scene_viewer_render_object)(
					Scene_viewer_render_background);
				ADD_OBJECT_TO_LIST(Scene_viewer_render_object)(render_object,
					rendering_data.render_callstack);

				if (scene_viewer->overlay_scene)
				{
					/*???RC property functions should be obtained once, elsewhere */
					glGetIntegerv(GL_STENCIL_BITS,&rendering_data.stencil_depth);
					if (0<rendering_data.stencil_depth)
					{
						glEnable(GL_STENCIL_TEST);
						glClearStencil(0);
						glClear(GL_STENCIL_BUFFER_BIT);
					}

					render_object = CREATE(Scene_viewer_render_object)(
						Scene_viewer_render_overlay_scene);
					ADD_OBJECT_TO_LIST(Scene_viewer_render_object)(render_object,
						rendering_data.render_callstack);
				}

				/* Apply the modelview matrix, lights and clip planes */
				render_object = CREATE(Scene_viewer_render_object)(
					Scene_viewer_apply_modelview_lights_and_clip_planes);
				ADD_OBJECT_TO_LIST(Scene_viewer_render_object)(render_object,
					rendering_data.render_callstack);

				if (SCENE_VIEWER_STEREO == scene_viewer->stereo_mode)
				{
					render_object = CREATE(Scene_viewer_render_object)(
						Scene_viewer_stereo);
					ADD_OBJECT_TO_LIST(Scene_viewer_render_object)(render_object,
						rendering_data.render_callstack);
				}

				switch (scene_viewer->transparency_mode)
				{
					case SCENE_VIEWER_SLOW_TRANSPARENCY:
					{
						render_object = CREATE(Scene_viewer_render_object)(
							Scene_viewer_slow_transparency);
						ADD_OBJECT_TO_LIST(Scene_viewer_render_object)(render_object,
							rendering_data.render_callstack);
					} break;
					case SCENE_VIEWER_LAYERED_TRANSPARENCY:
					{
						render_object = CREATE(Scene_viewer_render_object)(
							Scene_viewer_layered_transparency);
						ADD_OBJECT_TO_LIST(Scene_viewer_render_object)(render_object,
							rendering_data.render_callstack);
					} break;
					case SCENE_VIEWER_ORDER_INDEPENDENT_TRANSPARENCY:
					{
						Scene_viewer_initialise_order_independent_transparency(&rendering_data);

						render_object = CREATE(Scene_viewer_render_object)(
							Scene_viewer_order_independent_transparency);
						ADD_OBJECT_TO_LIST(Scene_viewer_render_object)(render_object,
							rendering_data.render_callstack);						
					}
				}

				render_object = CREATE(Scene_viewer_render_object)(
					Scene_viewer_apply_projection_matrix);
				ADD_OBJECT_TO_LIST(Scene_viewer_render_object)(render_object,
					rendering_data.render_callstack);

				/* Always add this to the stack last */
				render_object = CREATE(Scene_viewer_render_object)(
					Scene_viewer_execute_scene_non_fastchanging);
				ADD_OBJECT_TO_LIST(Scene_viewer_render_object)(render_object,
					rendering_data.render_callstack);

				return_code=1;

				if (scene_viewer->perturb_lines)
				{
					glPolygonOffset(1.5,0.000001);
					glEnable(GL_POLYGON_OFFSET_FILL);
				}
				else
				{
					glDisable(GL_POLYGON_OFFSET_FILL);
				}

#if defined (DEBUG)
				/*???debug*/
				printf("Scene_viewer: build scene and redraw\n");
#endif /* defined (DEBUG) */

				/*???RC. Is this the best place to set line width and point size? */
				glLineWidth((GLfloat)global_line_width);
				glPointSize((GLfloat)global_point_size);
				/*???RC temporary: turn on point and line antialiasing */
				/*glEnable(GL_POINT_SMOOTH);
				  glEnable(GL_LINE_SMOOTH);*/
				/*???RC test */
				glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
				/* depth tests are against a normalised z coordinate (i.e. [0..1])
					so the following sets this up and turns on the test */
				if (SCENE_VIEWER_STEREO != scene_viewer->stereo_mode)
				{
					if (double_buffer)
					{
						glDrawBuffer(GL_BACK);
						/* Multipass rendering types need to have the correct read buffer */
						glReadBuffer(GL_BACK);
					}
					else	
					{
						glDrawBuffer(GL_FRONT);
						/* Multipass rendering types need to have the correct read buffer */
						glReadBuffer(GL_FRONT);
					}
				}

				glDepthRange((GLclampd)0.0,(GLclampd)1.0);
				glDepthMask(GL_TRUE);
				glEnable(GL_DEPTH_TEST);
				glDepthFunc(GL_LESS);
				/* Get size of alpha [blending] buffer. */
				/* glGetIntegerv(GL_ALPHA_BITS,&alpha_bits); */
				/* turn on alpha */
				switch(scene_viewer->blending_mode)
				{
					default:
					case SCENE_VIEWER_BLEND_NORMAL:
					{
						glEnable(GL_BLEND);
						glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
					} break;
					case SCENE_VIEWER_BLEND_NONE:
					{
						glDisable(GL_BLEND);
					} break;
#if defined GL_VERSION_1_4
					case SCENE_VIEWER_BLEND_TRUE_ALPHA:
					{
						/* This function is protected at runtime by testing in the set 
							blending mode function */
						glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
							GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
						glEnable(GL_BLEND);
					} break;
#endif /* defined GL_VERSION_1_4 */
				}

				glViewport((GLint)rendering_data.viewport_left,
					(GLint)rendering_data.viewport_bottom,
					(GLint)rendering_data.viewport_width,
					(GLint)rendering_data.viewport_height);

				//glScissor((GLint)rendering_data.viewport_left,
				//	(GLint)rendering_data.viewport_bottom,
				//	(GLint)rendering_data.viewport_width,
				//	(GLint)rendering_data.viewport_height);
				//glEnable(GL_SCISSOR_TEST);

				/* glPushAttrib(GL_VIEWPORT_BIT); */
				reset_Lights();

				/* light model */
				compile_Light_model(scene_viewer->light_model);
				execute_Light_model(scene_viewer->light_model);

				/* compile the viewer lights */
				FOR_EACH_OBJECT_IN_LIST(Light)(compile_Light,(void *)NULL,
					scene_viewer->list_of_lights);

				/********* CALL THE RENDERING CALLSTACK **********/
				Scene_viewer_call_next_renderer(&rendering_data);

				glFlush();
			}
#if defined (REPORT_GL_ERRORS)
			{
				char message[200];
				GLenum error;
				int max_error = 200;
				while((max_error--) && (GL_NO_ERROR!=(error = glGetError())))
				{
					strcpy(message,"Scene_viewer_render_scene_private: GL ERROR ");
					strcat(message, (char *)gluErrorString(error));
					display_message(ERROR_MESSAGE, message);
				}
			}
#endif /* defined (REPORT_GL_ERRORS) */
			DESTROY(LIST(Scene_viewer_render_object))(&rendering_data.render_callstack);
		}
		scene_viewer->fast_changing=1;
		scene_viewer->frame_count++;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_render_scene_private.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_render_scene_private */

int Scene_viewer_render_scene(struct Scene_viewer *scene_viewer)
/*******************************************************************************
LAST MODIFIED : 12 July 2000

DESCRIPTION :
Called to redraw the Scene_viewer scene after changes in the display lists or
viewing transformations.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_render_scene);
	if (scene_viewer)
	{
		return_code=Scene_viewer_render_scene_private(scene_viewer, 
			/*left*/0, /*bottom*/0, /*right*/0, /*top*/0, /*override_antialias*/0, 
			/*override_transparency_layers*/0);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_render_scene.  Missing scene_viewer");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_render_scene */

int Scene_viewer_render_scene_in_viewport(struct Scene_viewer *scene_viewer,
	int left, int bottom, int right, int top)
/*******************************************************************************
LAST MODIFIED : 1 September 1999

DESCRIPTION :
Called to redraw the Scene_viewer scene after changes in the display lists or
viewing transformations.  Uses the specified viewport to draw into (unless
all the dimensions are zero).
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_render_scene_in_viewport);
	if (scene_viewer)
	{
		return_code=Scene_viewer_render_scene_private(scene_viewer,
			left, bottom, right, top, /*override_antialias*/0, 
			/*override_transparency_layers*/0);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_render_scene_in_viewport.  Missing scene_viewer");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_render_scene_in_viewport */

int Scene_viewer_render_scene_in_viewport_with_overrides(
	struct Scene_viewer *scene_viewer, int left, int bottom, int right, int top,
	int antialias, int transparency_layers, int drawing_offscreen)
/*******************************************************************************
LAST MODIFIED : 11 December 2002

DESCRIPTION :
Called to redraw the Scene_viewer scene after changes in the display lists or
viewing transformations.  Uses the specified viewport to draw into (unless
all the dimensions are zero).  If non_zero then the supplied <antialias> and
<transparency_layers> are used for just this render.
The <drawing_offscreen> flag is used by offscreen buffers to force the scene
viewer to do a full Scene execute despite the current fast_changing state.
The previous fast_changing state is kept so that the onscreen graphics are
kept in a sensible state.
==============================================================================*/
{
	int keep_fast_changing_state, return_code;

	ENTER(Scene_viewer_render_scene_in_viewport);
	if (scene_viewer)
	{
		if (drawing_offscreen)
		{
			keep_fast_changing_state = scene_viewer->fast_changing;
			scene_viewer->fast_changing = 0;
		}
		return_code=Scene_viewer_render_scene_private(scene_viewer,
			left, bottom, right, top, antialias, transparency_layers);
		if (drawing_offscreen)
		{
			scene_viewer->fast_changing = keep_fast_changing_state;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_render_scene_in_viewport.  Missing scene_viewer");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_render_scene_in_viewport */

static int Scene_viewer_automatic_tumble(struct Scene_viewer *scene_viewer)
/*******************************************************************************
LAST MODIFIED : 28 September 2000

DESCRIPTION :
Rotates the scene_viewer when the tumble is active.
==============================================================================*/
{
	double centre_x,centre_y,size_x,size_y,viewport_bottom,viewport_height,
		viewport_left,viewport_width;
	enum Interactive_event_type interactive_event_type;
	int i,j,return_code;
	GLdouble temp_modelview_matrix[16], temp_projection_matrix[16];
	GLint viewport[4];
	struct Interactive_event *interactive_event;
	struct Interaction_volume *interaction_volume;

	ENTER(Scene_viewer_automatic_tumble);
	if (scene_viewer)
	{
		if (scene_viewer->tumble_active)
		{
			Scene_viewer_rotate_about_lookat_point(scene_viewer,
				scene_viewer->tumble_axis,
				scene_viewer->tumble_angle);
			CMISS_CALLBACK_LIST_CALL(Scene_viewer_callback)(
				scene_viewer->sync_callback_list,scene_viewer,NULL);

			if (scene_viewer->interactive_tool)
			{
				glGetIntegerv(GL_VIEWPORT,viewport);
				viewport_left   = (double)(viewport[0]);
				viewport_bottom = (double)(viewport[1]);
				viewport_width  = (double)(viewport[2]);
				viewport_height = (double)(viewport[3]);

				/*???RC*//*Scene_viewer_calculate_transformation(scene_viewer,
					viewport_width,viewport_height);*/

				size_x = SCENE_VIEWER_PICK_SIZE;
				size_y = SCENE_VIEWER_PICK_SIZE;
				
				centre_x=(double)(scene_viewer->previous_pointer_x);
				/* flip y as x event has y=0 at top of window, increasing down */
				centre_y=viewport_height-(double)(scene_viewer->previous_pointer_y)-1.0;
				
				/* Update the interaction volume */
				interactive_event_type=INTERACTIVE_EVENT_MOTION_NOTIFY;
				for (i=0;i<4;i++)
				{
					for (j=0;j<4;j++)
					{
						temp_modelview_matrix[i*4+j] =
							scene_viewer->modelview_matrix[j*4+i];
						temp_projection_matrix[i*4+j] =
							scene_viewer->window_projection_matrix[j*4+i];
					}
				}
				interaction_volume=create_Interaction_volume_ray_frustum(
					temp_modelview_matrix,temp_projection_matrix,
					viewport_left,viewport_bottom,viewport_width,viewport_height,
					centre_x,centre_y,size_x,size_y);
				ACCESS(Interaction_volume)(interaction_volume);
				interactive_event=CREATE(Interactive_event)(interactive_event_type,
					/*button_number*/-1,/*input_modifier*/0,interaction_volume,
					scene_viewer->scene);
				ACCESS(Interactive_event)(interactive_event);
				Interactive_tool_handle_interactive_event(
					scene_viewer->interactive_tool,(void *)scene_viewer,
					interactive_event, scene_viewer->graphics_buffer);
				DEACCESS(Interactive_event)(&interactive_event);
				DEACCESS(Interaction_volume)(&interaction_volume);
			}
		}
		else
		{
			scene_viewer->tumble_angle = 0.0;
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_automatic_tumble.  Missing scene_viewer");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_automatic_tumble */

static int Scene_viewer_idle_update_callback(void *scene_viewer_void)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Updates the scene_viewer.
==============================================================================*/
{
	int return_code;
	struct Scene_viewer *scene_viewer;

	ENTER(Scene_viewer_idle_update_callback);
	if (scene_viewer=(struct Scene_viewer *)scene_viewer_void)
	{
		/* set workproc no longer pending */
		scene_viewer->idle_update_callback_id = (struct Event_dispatcher_idle_callback *)NULL;
		if (scene_viewer->tumble_active &&
				(!Interactive_tool_is_Transform_tool(scene_viewer->interactive_tool) ||
				Interactive_tool_transform_get_free_spin(scene_viewer->interactive_tool)))
		{
			scene_viewer->fast_changing = 0;
			Scene_viewer_automatic_tumble(scene_viewer);
			/* Repost the idle callback */
			if(!scene_viewer->idle_update_callback_id)
			{
				scene_viewer->idle_update_callback_id = Event_dispatcher_add_idle_callback(
					User_interface_get_event_dispatcher(scene_viewer->user_interface),
					Scene_viewer_idle_update_callback, (void *)scene_viewer,
					EVENT_DISPATCHER_IDLE_UPDATE_SCENE_VIEWER_PRIORITY);			
			}
		}
		else
		{
			scene_viewer->tumble_angle = 0.0;
		}
		Graphics_buffer_make_current(scene_viewer->graphics_buffer);
		Scene_viewer_render_scene(scene_viewer);
		if (scene_viewer->swap_buffers)
		{
			Graphics_buffer_swap_buffers(scene_viewer->graphics_buffer);
		}
		CMISS_CALLBACK_LIST_CALL(Scene_viewer_callback)(
			scene_viewer->repaint_required_callback_list, scene_viewer, NULL);
		return_code = 0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_idle_update_callback.  Missing scene_viewer");
		return_code = 0;
	}
	LEAVE;

	return (return_code); /* so workproc finished */
} /* Scene_viewer_idle_update_callback */

static int Scene_viewer_redraw_in_idle_time(struct Scene_viewer *scene_viewer)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Sets up a callback to Scene_viewer_idle_update for an update in idle time.
Does this by putting a WorkProc on the queue - if not already done for this
Scene_viewer - which will force a redraw at the next idle moment. If the
scene_viewer is changed again before it is updated, a new WorkProc will not be
put in the queue, but the old one will update the window to the new state.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_redraw_in_idle_time);
	if (scene_viewer)
	{
		if (!scene_viewer->idle_update_callback_id)
		{
			scene_viewer->idle_update_callback_id = Event_dispatcher_add_idle_callback(
				User_interface_get_event_dispatcher(scene_viewer->user_interface),
				Scene_viewer_idle_update_callback, (void *)scene_viewer,
				EVENT_DISPATCHER_IDLE_UPDATE_SCENE_VIEWER_PRIORITY);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_redraw_in_idle_time.  Missing scene_viewer");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_redraw_in_idle_time */

static void Scene_viewer_initialise_callback(struct Graphics_buffer *graphics_buffer,
	void *dummy_void, void *scene_viewer_void)
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Scene_viewer *scene_viewer;

	ENTER(Scene_viewer_initialise_callback);
	USE_PARAMETER(dummy_void);
	if (scene_viewer=(struct Scene_viewer *)scene_viewer_void)
	{
		Graphics_buffer_make_current(graphics_buffer);
		/* initialise graphics library to load XFont */
		return_code = initialize_graphics_library(scene_viewer->user_interface);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_initialise_callback.  Missing scene_viewer");
		return_code = 0;
	}
	LEAVE;
	USE_PARAMETER(return_code);
} /* Scene_viewer_initialise_callback */

static void Scene_viewer_resize_callback(struct Graphics_buffer *graphics_buffer,
	void *dummy_void, void *scene_viewer_void)
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Called when part of the Scene_viewer window is resized. All it does is notify
callbacks interested in the scene_viewers transformations.
==============================================================================*/
{
	int return_code;
	struct Scene_viewer *scene_viewer;

	ENTER(Scene_viewer_resize_callback);
	USE_PARAMETER(graphics_buffer);
	USE_PARAMETER(dummy_void);
	if (scene_viewer=(struct Scene_viewer *)scene_viewer_void)
	{
		scene_viewer->transform_flag = 1;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_resize_callback.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;
	USE_PARAMETER(return_code);
} /* Scene_viewer_resize_callback */

static void Scene_viewer_expose_callback(struct Graphics_buffer *graphics_buffer,
	void *expose_data_void, void *scene_viewer_void)
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Called when part of the Scene_viewer window is exposed. Does not attempt to
redraw just the exposed area. Instead, it redraws the whole picture, but only
if there are no more expose events pending.
==============================================================================*/
{
	int return_code;
	struct Graphics_buffer_expose_data *expose_data;
	struct Scene_viewer *scene_viewer;

	ENTER(Scene_viewer_expose_callback);
	USE_PARAMETER(graphics_buffer);
	if (scene_viewer=(struct Scene_viewer *)scene_viewer_void)
	{
		if (!(expose_data = (struct Graphics_buffer_expose_data *)expose_data_void))
		{
			/* The redraw everything in idle time default */
			return_code = Scene_viewer_redraw(scene_viewer);
		}
		else
		{
			/* We are not currently using the fields of this data */
			USE_PARAMETER(expose_data);
			/*Redraw now if idle callback is pending
			(otherwise assume we are up to date already) */
			if (scene_viewer->idle_update_callback_id)
			{
				Scene_viewer_redraw_now(scene_viewer);
				/* Redraw_now clears the idle_update_callback */
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_expose_callback.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;
	USE_PARAMETER(return_code);
} /* Scene_viewer_expose_callback */

static int Scene_viewer_unproject(int pointer_x,int pointer_y,
	double *near_x,double *near_y,double *near_z,
	double *far_x,double *far_y,double *far_z)
/*******************************************************************************
LAST MODIFIED : 13 July 2000

DESCRIPTION :
Converts the pointer location into locations on the near_plane and far planes in
world space.
==============================================================================*/
{
	int return_code;
	GLdouble modelview_matrix[16],projection_matrix[16],obj_x,obj_y,obj_z,
		win_x,win_y;
	GLint viewport[4];

	ENTER(Scene_viewer_unproject);
	if (near_x&&near_y&&near_z&&far_x&&far_y&&far_z)
	{
		glGetDoublev(GL_MODELVIEW_MATRIX,modelview_matrix);
		glGetDoublev(GL_PROJECTION_MATRIX,projection_matrix);
		glGetIntegerv(GL_VIEWPORT,viewport);
		return_code=0;
		win_x=(GLdouble)pointer_x;
		win_y=(GLdouble)(viewport[3]-pointer_y);
		/* for OpenGL window z coordinates, 0.0=near_plane, 1.0=far */
		if (GL_TRUE==gluUnProject(win_x,win_y,0.0,
			modelview_matrix,projection_matrix,viewport,&obj_x,&obj_y,&obj_z))
		{
			*near_x=(double)obj_x;
			*near_y=(double)obj_y;
			*near_z=(double)obj_z;
			if (GL_TRUE==gluUnProject(win_x,win_y,1.0,
				modelview_matrix,projection_matrix,viewport,&obj_x,&obj_y,&obj_z))
			{
				*far_x=(double)obj_x;
				*far_y=(double)obj_y;
				*far_z=(double)obj_z;
				return_code=1;
			}
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"Scene_viewer_unproject.  Unable to unproject");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_unproject.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_unproject */

static int Scene_viewer_input_select(struct Scene_viewer *scene_viewer,
	struct Graphics_buffer_input *input)
/*******************************************************************************
LAST MODIFIED : 27 April 2000

DESCRIPTION :
Creates abstract interactive events relating to the mouse input to the
<scene_viewer> <event> and sends them to the current interactive_tool for the
scene_viewer.
==============================================================================*/
{
	double centre_x,centre_y,size_x,size_y,viewport_bottom,viewport_height,
		viewport_left,viewport_width;
	enum Interactive_event_type interactive_event_type;
	int button_number,i,input_modifier,j,modifier_state,mouse_event,return_code;
	GLdouble temp_modelview_matrix[16], temp_projection_matrix[16];
	GLint viewport[4];
	struct Interactive_event *interactive_event;
	struct Interaction_volume *interaction_volume;

	ENTER(Scene_viewer_input_select);
	if (scene_viewer && scene_viewer->interactive_tool && input)
	{
		return_code=1;
		mouse_event=0;
		glGetIntegerv(GL_VIEWPORT,viewport);
		viewport_left   = (double)(viewport[0]);
		viewport_bottom = (double)(viewport[1]);
		viewport_width  = (double)(viewport[2]);
		viewport_height = (double)(viewport[3]);
		switch (input->type)
		{
			case GRAPHICS_BUFFER_BUTTON_PRESS:
			{
				interactive_event_type=INTERACTIVE_EVENT_BUTTON_PRESS;
				centre_x=(double)(input->position_x);
				/* flip y as x event has y=0 at top of window, increasing down */
				centre_y=viewport_height-(double)(input->position_y)-1.0;
				button_number=input->button_number;
				/* Keep position for automatic tumbling update */
				scene_viewer->previous_pointer_x = input->position_x;
				scene_viewer->previous_pointer_y = input->position_y;
				modifier_state=input->input_modifier;
				mouse_event=1;
			} break;
			case GRAPHICS_BUFFER_MOTION_NOTIFY:
			{
				interactive_event_type=INTERACTIVE_EVENT_MOTION_NOTIFY;
				centre_x=(double)(input->position_x);
				/* flip y as x event has y=0 at top of window, increasing down */
				centre_y=viewport_height-(double)(input->position_y)-1.0;
				button_number=-1;
				/* Keep position for automatic tumbling update */
				scene_viewer->previous_pointer_x = input->position_x;
				scene_viewer->previous_pointer_y = input->position_y;
				modifier_state=input->input_modifier;
				mouse_event=1;
			} break;
			case GRAPHICS_BUFFER_BUTTON_RELEASE:
			{
				interactive_event_type=INTERACTIVE_EVENT_BUTTON_RELEASE;
				centre_x=(double)(input->position_x);
				/* flip y as x event has y=0 at top of window, increasing down */
				centre_y=viewport_height-(double)(input->position_y)-1.0;
				button_number=input->button_number;
				modifier_state=input->input_modifier;
				mouse_event=1;
			}
			case GRAPHICS_BUFFER_KEY_PRESS:
			{
			} break;
			case GRAPHICS_BUFFER_KEY_RELEASE:
			{
			} break;
			default:
			{
				printf("Scene_viewer_input_select.  Invalid X event");
				return_code=0;
			} break;
		}
		if (return_code&&mouse_event)
		{
			/*???RC Picking sensitivity should not be hardcoded - read from
				defaults file and/or set from text command */
			size_x = SCENE_VIEWER_PICK_SIZE;
			size_y = SCENE_VIEWER_PICK_SIZE;
			input_modifier=0;
			if (GRAPHICS_BUFFER_INPUT_MODIFIER_SHIFT&modifier_state)
			{
				input_modifier += INTERACTIVE_EVENT_MODIFIER_SHIFT;
			}
			/* note that control key currently overrides to transform mode */
			if (GRAPHICS_BUFFER_INPUT_MODIFIER_CONTROL&modifier_state)
			{
				input_modifier += INTERACTIVE_EVENT_MODIFIER_CONTROL;
			}
			if (GRAPHICS_BUFFER_INPUT_MODIFIER_ALT&modifier_state)
			{
				input_modifier += INTERACTIVE_EVENT_MODIFIER_ALT;
			}
			for (i=0;i<4;i++)
			{
				for (j=0;j<4;j++)
				{
					temp_modelview_matrix[i*4+j] =
						scene_viewer->modelview_matrix[j*4+i];
					temp_projection_matrix[i*4+j] =
						scene_viewer->window_projection_matrix[j*4+i];
				}
			}

			interaction_volume=create_Interaction_volume_ray_frustum(
				temp_modelview_matrix,temp_projection_matrix,
				viewport_left,viewport_bottom,viewport_width,viewport_height,
				centre_x,centre_y,size_x,size_y);
			ACCESS(Interaction_volume)(interaction_volume);
			interactive_event=CREATE(Interactive_event)(interactive_event_type,
				button_number,input_modifier,interaction_volume,scene_viewer->scene);
			ACCESS(Interactive_event)(interactive_event);
			return_code=Interactive_tool_handle_interactive_event(
				scene_viewer->interactive_tool,(void *)scene_viewer,interactive_event,
				scene_viewer->graphics_buffer);
			DEACCESS(Interactive_event)(&interactive_event);
			DEACCESS(Interaction_volume)(&interaction_volume);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_input_select.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_input_select */

static int Scene_viewer_input_transform(struct Scene_viewer *scene_viewer,
	struct Graphics_buffer_input *input)
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Converts mouse button-press and motion events into viewing transformations in
<scene_viewer>.
==============================================================================*/
{
	int width,height;
	double near_x,near_y,near_z,far_x,far_y,far_z,dx,dy,dz;
	double old_near_x,old_near_y,old_near_z,old_far_x,old_far_y,old_far_z;
	double radius,fact,a[3],b[3],c[3],e[3],eye_distance,tangent_dist,d,phi,
		axis[3],angle;
	int return_code,pointer_x,pointer_y,i,delta_x,delta_y,view_changed;

	ENTER(Scene_viewer_input_transform);
	if (scene_viewer && input)
	{
		return_code=1;
		switch (input->type)
		{
			case GRAPHICS_BUFFER_BUTTON_PRESS:
			{
				Graphics_buffer_make_current(scene_viewer->graphics_buffer);
				pointer_x=input->position_x;
				pointer_y=input->position_y;
				/* printf("button %d press at %d %d\n",input->button,
					pointer_x,pointer_y); */
				if (Scene_viewer_unproject(pointer_x,pointer_y,
					&near_x,&near_y,&near_z,&far_x,&far_y,&far_z))
				{
					/*printf("PRESS  Near: %8.4f %8.4f %8.4f  Far: %8.4f %8.4f %8.4f\n",
						near_x,near_y,near_z,far_x,far_y,far_z);*/
					switch (input->button_number)
					{
						case 1:
						{
							scene_viewer->tumble_angle = 0;
							scene_viewer->tumble_active = 0;
							switch (scene_viewer->interact_mode)
							{
							       case SCENE_VIEWER_INTERACT_STANDARD:
							       {
							               if (0.0 != scene_viewer->tumble_rate)
							               {
								               scene_viewer->drag_mode=SV_DRAG_TUMBLE;
							               }
							       }break;
							       case SCENE_VIEWER_INTERACT_2D:
							       {
							               if (0.0 != scene_viewer->translate_rate)
							               {
								               scene_viewer->drag_mode=SV_DRAG_TRANSLATE;
                                                                       }
							       }break;
							}
						} break;
						case 2:
						{
							switch (scene_viewer->interact_mode)
							{
							       case SCENE_VIEWER_INTERACT_STANDARD:
							       {
							               if (0.0 != scene_viewer->translate_rate)
							               {
							                       scene_viewer->drag_mode=SV_DRAG_TRANSLATE;
								       }
							       }break;
							       case SCENE_VIEWER_INTERACT_2D:
							       {
							               if (0.0 != scene_viewer->tumble_rate)
							               {
							                       scene_viewer->drag_mode=SV_DRAG_TUMBLE;
								       }
							       }break;
							}
						} break;
						case 3:
						{
							if (0.0 != scene_viewer->zoom_rate)
							{
								scene_viewer->drag_mode=SV_DRAG_ZOOM;
							}
						} break;
						default:
						{
						} break;
					}
					scene_viewer->previous_pointer_x=pointer_x;
					scene_viewer->previous_pointer_y=pointer_y;
				}
			} break;
			case GRAPHICS_BUFFER_MOTION_NOTIFY:
			{
				pointer_x=input->position_x;
				pointer_y=input->position_y;
#if defined (DEBUG)
				printf("mouse move to %d %d\n",pointer_x,pointer_y);
#endif /* defined (DEBUG) */
				if (Scene_viewer_unproject(pointer_x,pointer_y,
					&near_x,&near_y,&near_z,&far_x,&far_y,&far_z)&&
					Scene_viewer_unproject(scene_viewer->previous_pointer_x,
						scene_viewer->previous_pointer_y, &old_near_x,&old_near_y,
						&old_near_z,&old_far_x,&old_far_y,&old_far_z))
				{
					view_changed=0;
					switch (scene_viewer->drag_mode)
					{
						case SV_DRAG_NOTHING:
						{
						} break;
						case SV_DRAG_TUMBLE:
						{
							/* Tumble works like you are pulling string off a ball placed in
								the middle of the window. The line of the string is in the
								direction of dragging. If you follow back to the tangent point
								on the front of the ball, then the axis of rotation is the
								cross product of the vector from the centre of the ball to this
								tangent point with the direction you are pulling the string.
								As the distance from line you drag along to the centre of the
								window increases, the tumbling increasingly turns to twisting
								about the view direction. */
							width = Graphics_buffer_get_width(scene_viewer->graphics_buffer);
							height = Graphics_buffer_get_height(scene_viewer->graphics_buffer);
 							if ((0<width)&&(0<height))
							{
								/* get the radius of the ball */
								radius=0.25*(width+height);
								delta_x=pointer_x-scene_viewer->previous_pointer_x;
								delta_y=scene_viewer->previous_pointer_y-pointer_y;
								if (0<(tangent_dist=sqrt(delta_x*delta_x+delta_y*delta_y)))
								{
									/* get unit vector dx,dy normal to drag line */
									dx=-(double)delta_y/tangent_dist;
									dy= (double)delta_x/tangent_dist;
									/* get shortest distance to centre along drag line normal */
									d=dx*(pointer_x-0.5*(width-1))+dy*(0.5*(height-1)-pointer_y);
									/* limit d to radius so twists about view direction */
									if (d > radius)
									{
										d = radius;
									}
									else
									{
										if (d < -radius)
										{
											d = -radius;
										}
									}
									/* phi ranges from 0 pointing out of the screen to +/- PI/2
										 in the plane of the window */
									phi=acos(d/radius)-0.5*PI;
									/* apply the tumble_rate to slow/hasten tumble */
									angle=scene_viewer->tumble_rate*tangent_dist/radius;
									/* get axis to rotate about */
									/* a = vector towards viewer = angle to rotate about as a
										 right hand screw when phi = -PI/2 */
									a[0]=scene_viewer->eyex-scene_viewer->lookatx;
									a[1]=scene_viewer->eyey-scene_viewer->lookaty;
									a[2]=scene_viewer->eyez-scene_viewer->lookatz;
									normalize3(a);
									/* b = up vector */
									b[0]=scene_viewer->upx;
									b[1]=scene_viewer->upy;
									b[2]=scene_viewer->upz;
									normalize3(b);
									/* c = b (x) a = vector to the right */
									cross_product3(b,a,c);
									normalize3(c);
									/* e = angle to rotate about if phi = 0 */
									e[0] = dx*c[0] + dy*b[0];
									e[1] = dx*c[1] + dy*b[1];
									e[2] = dx*c[2] + dy*b[2];
									/* get actual angle to rotate by from a, e and phi */
									axis[0]=sin(phi)*a[0]+cos(phi)*e[0];
									axis[1]=sin(phi)*a[1]+cos(phi)*e[1];
									axis[2]=sin(phi)*a[2]+cos(phi)*e[2];
									if (Scene_viewer_rotate_about_lookat_point(scene_viewer,axis,
										-angle))
									{
										if (Interactive_tool_is_Transform_tool(
											scene_viewer->interactive_tool) &&
											Interactive_tool_transform_get_free_spin(
												scene_viewer->interactive_tool))
										{
											/* Store axis and angle so that we can make the
												 scene viewer spin if left alone. */
											scene_viewer->tumble_axis[0] = axis[0];
											scene_viewer->tumble_axis[1] = axis[1];
											scene_viewer->tumble_axis[2] = axis[2];
											scene_viewer->tumble_angle = -angle;
											scene_viewer->tumble_active = 0;
										}
										else
										{
											scene_viewer->tumble_angle = 0;
											scene_viewer->tumble_active = 0;											
										}
										view_changed=1;
									}
								}
							}
						} break;
						case SV_DRAG_TRANSLATE:
						{
							/* a = vector towards viewer */
							a[0]=scene_viewer->eyex-scene_viewer->lookatx;
							a[1]=scene_viewer->eyey-scene_viewer->lookaty;
							a[2]=scene_viewer->eyez-scene_viewer->lookatz;
							eye_distance=normalize3(a);
							/* translate at lookat point; proportion from near to far */
							if ((scene_viewer->far_plane > scene_viewer->near_plane)&&
								(eye_distance >= scene_viewer->near_plane)&&
								(eye_distance <= scene_viewer->far_plane))
							{
								fact = (eye_distance-scene_viewer->near_plane)/
									(scene_viewer->far_plane-scene_viewer->near_plane);
							}
							else
							{
								fact = 0.0;
							}
							/* get translation at eye distance between near and far */
							/* apply the translate_rate to slow/hasten translate */
							dx=scene_viewer->translate_rate*
								((1.0-fact)*(near_x-old_near_x) + fact*(far_x-old_far_x));
							dy=scene_viewer->translate_rate*
								((1.0-fact)*(near_y-old_near_y) + fact*(far_y-old_far_y));
							dz=scene_viewer->translate_rate*
								((1.0-fact)*(near_z-old_near_z) + fact*(far_z-old_far_z));
							scene_viewer->eyex -= dx;
							scene_viewer->eyey -= dy;
							scene_viewer->eyez -= dz;
							scene_viewer->lookatx -= dx;
							scene_viewer->lookaty -= dy;
							scene_viewer->lookatz -= dz;
							view_changed=1;
						} break;
						case SV_DRAG_ZOOM:
						{
							/*??? Handles only symmetric viewing volume */
							radius=0.25*(scene_viewer->right-scene_viewer->left+
								scene_viewer->top-scene_viewer->bottom);
							/* apply the zoom_rate to slow/hasten zoom */
							fact=1.0 + 0.01*scene_viewer->zoom_rate;
							i=pointer_y;
							while (i>scene_viewer->previous_pointer_y)
							{
								radius /= fact;
								i--;
							}
							while (i<scene_viewer->previous_pointer_y)
							{
								radius *= fact;
								i++;
							}
							scene_viewer->left=-radius;
							scene_viewer->right=radius;
							scene_viewer->bottom=-radius;
							scene_viewer->top=radius;
							view_changed=1;
						} break;
						default:
						{
						} break;
					}
					if (view_changed)
					{
						Scene_viewer_redraw_now(scene_viewer);
						/* send the callbacks */
						CMISS_CALLBACK_LIST_CALL(Scene_viewer_callback)(
							scene_viewer->sync_callback_list,scene_viewer,NULL);
						scene_viewer->transform_flag = 1;
					}
					scene_viewer->previous_pointer_x=pointer_x;
					scene_viewer->previous_pointer_y=pointer_y;
				}
			} break;
			case GRAPHICS_BUFFER_BUTTON_RELEASE:
			{
				if ((scene_viewer->drag_mode == SV_DRAG_TUMBLE) && scene_viewer->tumble_angle)
				{
					scene_viewer->tumble_active = 1;
					if (!scene_viewer->idle_update_callback_id)
					{
						scene_viewer->idle_update_callback_id = 
							Event_dispatcher_add_idle_callback(
								User_interface_get_event_dispatcher(scene_viewer->user_interface),
								Scene_viewer_idle_update_callback, (void *)scene_viewer,
								EVENT_DISPATCHER_IDLE_UPDATE_SCENE_VIEWER_PRIORITY);			
					}
				}
#if defined (DEBUG)
				printf("button %d release at %d %d\n",input->button_number,
					input->position_x,input->position_y);
				pointer_x=input->position_x;
				pointer_y=input->position_y;
				if (Scene_viewer_unproject(pointer_x,pointer_y,
					&near_x,&near_y,&near_z,&far_x,&far_y,&far_z))
				{
					printf("RELEASENear: %8.4f %8.4f %8.4f  Far: %8.4f %8.4f %8.4f\n",
						near_x,near_y,near_z,far_x,far_y,far_z);
				}
#endif /* defined (DEBUG) */
				scene_viewer->drag_mode=SV_DRAG_NOTHING;
			} break;
			case GRAPHICS_BUFFER_KEY_PRESS:
			{
#if defined (DEBUG)
				printf("key %d press at %d %d\n",input->key_code,input->position_x,
					input->position_y);
#endif /* defined (DEBUG) */
			} break;
			case GRAPHICS_BUFFER_KEY_RELEASE:
			{
#if defined (DEBUG)
				printf("key %d release at %d %d\n",input->key_code,input->position_x,
					input->position_y);
#endif /* defined (DEBUG) */
			} break;
			default:
			{
				printf("Scene_viewer_input_transform.  Invalid X event");
				return_code=0;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_input_transform.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_input_transform */

static int Scene_viewer_input_viewport_transform(
	struct Scene_viewer *scene_viewer, struct Graphics_buffer_input *input)
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Converts mouse button-press and motion events into viewport zoom and translate
transformations.
==============================================================================*/
{
	double dx,dy,zoom_ratio,fact;
	int return_code,pointer_x,pointer_y,i;

	ENTER(Scene_viewer_input_viewport_transform);
	if (scene_viewer&&input)
	{
		return_code=1;
		switch (input->type)
		{
			case GRAPHICS_BUFFER_BUTTON_PRESS:
			{
				Graphics_buffer_make_current(scene_viewer->graphics_buffer);
				pointer_x=input->position_x;
				pointer_y=input->position_y;
				switch (input->button_number)
				{
					case 1:
					{
						switch (scene_viewer->interact_mode)
						{
							case SCENE_VIEWER_INTERACT_STANDARD:
							{
							        scene_viewer->drag_mode=SV_DRAG_TUMBLE;
							}break;
						        case SCENE_VIEWER_INTERACT_2D:
							{
							  scene_viewer->drag_mode=SV_DRAG_TRANSLATE;
							}break;
						}
					} break;
					case 2:
					{
						switch (scene_viewer->interact_mode)
						{
							case SCENE_VIEWER_INTERACT_STANDARD:
							{
							        scene_viewer->drag_mode=SV_DRAG_TRANSLATE;
							}break;
						        case SCENE_VIEWER_INTERACT_2D:
							{
							  scene_viewer->drag_mode=SV_DRAG_TUMBLE;
							}break;
						}
					} break;
					case 3:
					{
						scene_viewer->drag_mode=SV_DRAG_ZOOM;
					} break;
					default:
					{
					} break;
				}
				scene_viewer->previous_pointer_x=pointer_x;
				scene_viewer->previous_pointer_y=pointer_y;
			} break;
			case GRAPHICS_BUFFER_MOTION_NOTIFY:
			{
				pointer_x=input->position_x;
				pointer_y=input->position_y;
				switch (scene_viewer->drag_mode)
				{
					case SV_DRAG_NOTHING:
					{
					} break;
					case SV_DRAG_TRANSLATE:
					{
						dx=(pointer_x-scene_viewer->previous_pointer_x)/
							scene_viewer->user_viewport_pixels_per_unit_x;
						dy=(scene_viewer->previous_pointer_y-pointer_y)/
							scene_viewer->user_viewport_pixels_per_unit_y;
						scene_viewer->user_viewport_left -= dx;
						scene_viewer->user_viewport_top -= dy;
						scene_viewer->transform_flag = 1;
						Scene_viewer_redraw_now(scene_viewer);
					} break;
					case SV_DRAG_ZOOM:
					{
						zoom_ratio=1.0;
						fact=1.01;
						i=pointer_y;
						while (i>scene_viewer->previous_pointer_y)
						{
							zoom_ratio *= fact;
							i--;
						}
						while (i<scene_viewer->previous_pointer_y)
						{
							zoom_ratio /= fact;
							i++;
						}
						Scene_viewer_viewport_zoom(scene_viewer,zoom_ratio);
						Scene_viewer_redraw_now(scene_viewer);
					} break;
					default:
					{
					} break;
				}
				scene_viewer->previous_pointer_x=pointer_x;
				scene_viewer->previous_pointer_y=pointer_y;
			} break;
			case GRAPHICS_BUFFER_BUTTON_RELEASE:
			{
				pointer_x=input->position_x;
				pointer_y=input->position_y;
				scene_viewer->drag_mode=SV_DRAG_NOTHING;
			} break;
			case GRAPHICS_BUFFER_KEY_PRESS:
			{
				printf("key %d press at %d %d\n",input->key_code,input->position_x,
					input->position_y);
			} break;
			case GRAPHICS_BUFFER_KEY_RELEASE:
			{
				printf("key %d release at %d %d\n",input->key_code,input->position_x,
					input->position_y);
			} break;
			default:
			{
				printf("Scene_viewer_input_viewport_transform.  Invalid X event");
				return_code=0;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_input_viewport_transform.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_input_viewport_transform */

int Scene_viewer_default_input_callback(struct Scene_viewer *scene_viewer,
	struct Graphics_buffer_input *input, void *dummy_void)
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
	int return_code;

	ENTER(Scene_viewer_default_input_callback);
	USE_PARAMETER(dummy_void);
	if (scene_viewer)
	{
		Graphics_buffer_make_current(scene_viewer->graphics_buffer);
		switch (scene_viewer->input_mode)
		{
			case SCENE_VIEWER_NO_INPUT_OR_DRAW:
			case SCENE_VIEWER_NO_INPUT:
			{
				/* do nothing */
			} break;
			case SCENE_VIEWER_SELECT:
			{
				/* can override select mode by holding down control key */
				if (GRAPHICS_BUFFER_BUTTON_PRESS==input->type)
				{
					if (((GRAPHICS_BUFFER_INPUT_MODIFIER_CONTROL & input->input_modifier)&&
						(SCENE_VIEWER_ABSOLUTE_VIEWPORT != scene_viewer->viewport_mode))
						||((SCENE_VIEWER_ABSOLUTE_VIEWPORT == scene_viewer->viewport_mode)&&
						!((1==input->button_number)||
					   (GRAPHICS_BUFFER_INPUT_MODIFIER_BUTTON1 & input->input_modifier))))
					{
						scene_viewer->temporary_transform_mode=1;
					}
					else
					{
						scene_viewer->temporary_transform_mode=0;
					}
				}
				if (scene_viewer->temporary_transform_mode)
				{
					if (SCENE_VIEWER_RELATIVE_VIEWPORT == scene_viewer->viewport_mode ||
						SCENE_VIEWER_DISTORTING_RELATIVE_VIEWPORT == scene_viewer->viewport_mode)
					{
						if (SCENE_VIEWER_CUSTOM != scene_viewer->projection_mode)
						{
							Scene_viewer_input_transform(scene_viewer, input);
						}
					}
					else
					{
						Scene_viewer_input_viewport_transform(scene_viewer, input);
					}
				}
				else
				{
					/*???RC temporary until all tools are Interactive_tools */
					if (scene_viewer->interactive_tool)
					{
						Scene_viewer_input_select(scene_viewer, input);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Scene_viewer_default_input_callback.  Always need an interactive tool");
					}
				}
			} break;
			case SCENE_VIEWER_UPDATE_ON_CLICK:
			case SCENE_VIEWER_TRANSFORM:
			{
				if (SCENE_VIEWER_UPDATE_ON_CLICK==scene_viewer->input_mode)
				{
					if (GRAPHICS_BUFFER_BUTTON_PRESS==input->type)
					{
						if (input->input_modifier&GRAPHICS_BUFFER_INPUT_MODIFIER_CONTROL)
						{
							Scene_viewer_view_all(scene_viewer);
						}
					}
					scene_viewer->update_pixel_image=1;
					Scene_viewer_redraw(scene_viewer);
				}
				if (SCENE_VIEWER_RELATIVE_VIEWPORT==scene_viewer->viewport_mode ||
					SCENE_VIEWER_DISTORTING_RELATIVE_VIEWPORT==scene_viewer->viewport_mode)
				{
					if (SCENE_VIEWER_CUSTOM != scene_viewer->projection_mode)
					{
						Scene_viewer_input_transform(scene_viewer, input);
					}
				}
				else
				{
					Scene_viewer_input_viewport_transform(scene_viewer, input);
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Scene_viewer_default_input_callback.  Invalid input mode");
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_default_input_callback.  Invalid argument(s)");
	}
	LEAVE;
	/* We want to call any other callbacks even if this fails. */
	return_code = 1;

	return(return_code);
} /* Scene_viewer_default_input_callback */

static void Scene_viewer_graphics_buffer_input_callback(
	struct Graphics_buffer *graphics_buffer,
	struct Graphics_buffer_input *input, void *scene_viewer_void)
/*******************************************************************************
LAST MODIFIED : 04 February 2005

DESCRIPTION :
The callback for mouse or keyboard input in the Scene_viewer window. The
resulting behaviour depends on the <scene_viewer> input_mode. In Transform mode
mouse clicks and drags are converted to transformation; in Select mode OpenGL
picking is performed with picked objects and mouse click and drag information
returned to the scene.
==============================================================================*/
{
	struct Scene_viewer *scene_viewer;

	ENTER(Scene_viewer_graphics_buffer_input_callback);
	USE_PARAMETER(graphics_buffer);
	if (scene_viewer=(struct Scene_viewer *)scene_viewer_void)
	{
		CMISS_CALLBACK_LIST_CALL(Scene_viewer_input_callback)(
			scene_viewer->input_callback_list,scene_viewer,input);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_graphics_buffer_input_callback.  "
			"Invalid argument(s)");
	}
	LEAVE;
} /* Scene_viewer_input_callback */

/*
Manager Callback Module functions
---------------------------------
*/

static void Scene_viewer_light_change(
	struct MANAGER_MESSAGE(Light) *message, void *scene_viewer_void)
/*******************************************************************************
LAST MODIFIED : 30 May 2001

DESCRIPTION :
Something has changed globally in the light manager. If the modified light(s)
are in the scene, overlay_scene or the scene_viewer, then redraw.
???RC Review Manager Messages Here
==============================================================================*/
{
	struct Scene_viewer *scene_viewer;

	ENTER(Scene_viewer_light_change);
	if (message && (scene_viewer = (struct Scene_viewer *)scene_viewer_void))
	{
		switch (message->change)
		{
			case MANAGER_CHANGE_OBJECT(Light):
			case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Light):
			{
				if (Scene_viewer_has_light_in_list(scene_viewer,
					message->changed_object_list) ||
					Scene_has_light_in_list(scene_viewer->scene,
						message->changed_object_list) ||
					(scene_viewer->overlay_scene &&
						Scene_has_light_in_list(scene_viewer->overlay_scene,
							message->changed_object_list)))
				{
					Scene_viewer_redraw(scene_viewer);
				}
			} break;
			case MANAGER_CHANGE_ADD(Light):
			case MANAGER_CHANGE_REMOVE(Light):
			case MANAGER_CHANGE_IDENTIFIER(Light):
			{
				/* do nothing */
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_light_change.  Invalid argument(s)");
	}
	LEAVE;
} /* Scene_viewer_light_change */

static void Scene_viewer_light_model_change(
	struct MANAGER_MESSAGE(Light_model) *message, void *scene_viewer_void)
/*******************************************************************************
LAST MODIFIED : 30 May 2001

DESCRIPTION :
Something has changed globally in the light_model manager. If the light_model
in use by the scene_viewer is one of the changed light_models, then redraw.
???RC Review Manager Messages Here
==============================================================================*/
{
	struct Scene_viewer *scene_viewer;

	ENTER(Scene_viewer_light_model_change);
	if (message && (scene_viewer = (struct Scene_viewer *)scene_viewer_void))
	{
		switch (message->change)
		{
			case MANAGER_CHANGE_OBJECT(Light_model):
			case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Light_model):
			{
				if (scene_viewer->light_model &&
					IS_OBJECT_IN_LIST(Light_model)(scene_viewer->light_model,
						message->changed_object_list))
				{
					Scene_viewer_redraw(scene_viewer);
				}
			} break;
			case MANAGER_CHANGE_ADD(Light_model):
			case MANAGER_CHANGE_REMOVE(Light_model):
			case MANAGER_CHANGE_IDENTIFIER(Light_model):
			{
				/* do nothing */
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_light_model_change.  Invalid argument(s)");
	}
	LEAVE;
} /* Scene_viewer_light_model_change */

static void Scene_viewer_scene_change(
	struct MANAGER_MESSAGE(Scene) *message, void *scene_viewer_void)
/*******************************************************************************
LAST MODIFIED : 30 May 2001

DESCRIPTION :
Something has changed globally in the scene manager. If either the scene or
overlay_scene in this scene_viewer have been modified, then redraw.
???RC Review Manager Messages Here
==============================================================================*/
{
	struct Scene_viewer *scene_viewer;

	ENTER(Scene_viewer_scene_change);
	if (message && (scene_viewer = (struct Scene_viewer *)scene_viewer_void))
	{
		switch (message->change)
		{
			case MANAGER_CHANGE_OBJECT(Scene):
			case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Scene):
			{
				if (IS_OBJECT_IN_LIST(Scene)(scene_viewer->scene,
					message->changed_object_list) ||
					(scene_viewer->overlay_scene &&
						IS_OBJECT_IN_LIST(Scene)(scene_viewer->overlay_scene,
							message->changed_object_list)))
				{
					if (SCENE_FAST_CHANGE == Scene_get_change_status(scene_viewer->scene))
					{
						Scene_viewer_redraw_in_idle_time(scene_viewer);
					}
					else
					{
						Scene_viewer_redraw(scene_viewer);
					}
				}
			} break;
			case MANAGER_CHANGE_ADD(Scene):
			case MANAGER_CHANGE_REMOVE(Scene):
			case MANAGER_CHANGE_IDENTIFIER(Scene):
			{
				/* do nothing */
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_scene_change.  Invalid argument(s)");
	}
	LEAVE;
} /* Scene_viewer_scene_change */

static void Scene_viewer_texture_change(
	struct MANAGER_MESSAGE(Texture) *message,void *scene_viewer_void)
/*******************************************************************************
LAST MODIFIED : 30 May 2001

DESCRIPTION :
Something has changed globally in the texture manager. If the scene has a
background_texture and it is in the changed_object_list, then redraw.
???RC Review Manager Messages Here
==============================================================================*/
{
	struct Scene_viewer *scene_viewer;

	ENTER(Scene_viewer_texture_change);
	if (message && (scene_viewer = (struct Scene_viewer *)scene_viewer_void))
	{
		switch (message->change)
		{
			case MANAGER_CHANGE_OBJECT(Texture):
			case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Texture):
			{
				if (scene_viewer->background_texture &&
					IS_OBJECT_IN_LIST(Texture)(scene_viewer->background_texture,
						message->changed_object_list))
				{
					Scene_viewer_redraw(scene_viewer);
				}
			} break;
			case MANAGER_CHANGE_ADD(Texture):
			case MANAGER_CHANGE_REMOVE(Texture):
			case MANAGER_CHANGE_IDENTIFIER(Texture):
			{
				/* do nothing */
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_texture_change.  Invalid argument(s)");
	}
	LEAVE;
} /* Scene_viewer_texture_change */

/*
Global functions
----------------
*/

struct Cmiss_scene_viewer_package *CREATE(Cmiss_scene_viewer_package)(
	struct Graphics_buffer_package *graphics_buffer_package,
	struct Colour *background_colour,
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
	struct Interactive_tool *default_interactive_tool, 
	struct MANAGER(Light) *light_manager,struct Light *default_light,
	struct MANAGER(Light_model) *light_model_manager,
	struct Light_model *default_light_model,
	struct MANAGER(Scene) *scene_manager,struct Scene *scene,
	struct MANAGER(Texture) *texture_manager,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 19 January 2007

DESCRIPTION :
Creates a Cmiss_scene_viewer_package.
==============================================================================*/
{
	struct Cmiss_scene_viewer_package *scene_viewer_package;

	ENTER(CREATE(Scene_viewer_package));
	if (graphics_buffer_package&&background_colour&&default_light_model&&scene&&
		user_interface&&graphics_buffer_package&&interactive_tool_manager)
	{
		/* allocate memory for the scene_viewer structure */
		if (ALLOCATE(scene_viewer_package,struct Cmiss_scene_viewer_package,1))
		{
			scene_viewer_package->graphics_buffer_package = graphics_buffer_package;
			scene_viewer_package->background_colour = background_colour;
			scene_viewer_package->interactive_tool_manager = interactive_tool_manager;
			scene_viewer_package->default_interactive_tool = ACCESS(Interactive_tool)
				(default_interactive_tool);
			scene_viewer_package->light_manager = light_manager;
			scene_viewer_package->default_light = ACCESS(Light)(default_light);
			scene_viewer_package->light_model_manager = light_model_manager;
			scene_viewer_package->default_light_model = ACCESS(Light_model)(default_light_model);
			scene_viewer_package->scene_manager = scene_manager;
			scene_viewer_package->scene = ACCESS(Scene)(scene);
			scene_viewer_package->texture_manager = texture_manager;
			scene_viewer_package->user_interface = user_interface;
			scene_viewer_package->scene_viewer_list = CREATE(LIST(Scene_viewer))();
			scene_viewer_package->destroy_callback_list=
					CREATE(LIST(CMISS_CALLBACK_ITEM(Cmiss_scene_viewer_package_callback)))();
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Scene_viewer_package).  Not enough memory for scene_viewer");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Scene_viewer_package).  Invalid argument(s)");
		scene_viewer_package=(struct Cmiss_scene_viewer_package *)NULL;
	}
	LEAVE;

	return (scene_viewer_package);
} /* CREATE(Cmiss_scene_viewer_package) */

static void Scene_viewer_destroy_remove_from_package(
	struct Scene_viewer *scene_viewer,
	void *dummy_void, void *package_void)
/*******************************************************************************
LAST MODIFIED : 19 April 2007

DESCRIPTION :
==============================================================================*/
{
	struct Cmiss_scene_viewer_package *package;

	ENTER(create_Scene_viewer_from_package);
	USE_PARAMETER(dummy_void);
	if (scene_viewer && (package = (struct Cmiss_scene_viewer_package *)package_void))
	{
		REMOVE_OBJECT_FROM_LIST(Scene_viewer)(scene_viewer,
			package->scene_viewer_list);
	}
}

static int Scene_viewer_destroy_from_package(
	struct Scene_viewer *scene_viewer, void *package_void)
/*******************************************************************************
LAST MODIFIED : 19 April 2007

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Cmiss_scene_viewer_package *package;

	ENTER(Scene_viewer_destroy_from_package);
	if (scene_viewer && (package = (struct Cmiss_scene_viewer_package *)package_void))
	{
		Scene_viewer_remove_destroy_callback(scene_viewer,
			Scene_viewer_destroy_remove_from_package, package);
		DESTROY(Scene_viewer)(&scene_viewer);
	}
	return_code = 1;

	return (return_code);
}

int DESTROY(Cmiss_scene_viewer_package)(
	struct Cmiss_scene_viewer_package **scene_viewer_package_address)
/*******************************************************************************
LAST MODIFIED : 19 January 2007

DESCRIPTION :
Destroys the scene_viewer_package.
==============================================================================*/
{
	int return_code;
	struct Cmiss_scene_viewer_package *scene_viewer_package;

	ENTER(DESTROY(Cmiss_scene_viewer_package));
	if (scene_viewer_package = *scene_viewer_package_address)
	{
		/* Call the destroy callbacks */
		CMISS_CALLBACK_LIST_CALL(Cmiss_scene_viewer_package_callback)(
			scene_viewer_package->destroy_callback_list,scene_viewer_package,NULL);
		DESTROY(LIST(CMISS_CALLBACK_ITEM(Cmiss_scene_viewer_package_callback)))
			(&scene_viewer_package->destroy_callback_list);

		/* Destroy the scene viewers in the list as they are not accessed
			or deaccessed by the list (so not destroyed when delisted).
			The owners of these scene viewers
			should by virtue of having destroyed the command data and
			therefore this package no longer reference these scene viewers or
			they should register for destroy callbacks. */
		FOR_EACH_OBJECT_IN_LIST(Scene_viewer)(Scene_viewer_destroy_from_package,
			scene_viewer_package, scene_viewer_package->scene_viewer_list);
		DESTROY(LIST(Scene_viewer))(&scene_viewer_package->scene_viewer_list);

		DEACCESS(Interactive_tool)(&scene_viewer_package->default_interactive_tool);
		DEACCESS(Light)(&scene_viewer_package->default_light);
		DEACCESS(Light_model)(&scene_viewer_package->default_light_model);
		DEACCESS(Scene)(&scene_viewer_package->scene);
		DEALLOCATE(*scene_viewer_package_address);
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(Cmiss_scene_viewer_package).  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Cmiss_scene_viewer_package) */

int Cmiss_scene_viewer_package_add_destroy_callback(
	struct Cmiss_scene_viewer_package *scene_viewer_package,
	CMISS_CALLBACK_FUNCTION(Cmiss_scene_viewer_package_callback) *function,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 24 January 2007

DESCRIPTION :
Adds a callback to the <scene_viewer_package> that is called back before the scene
viewer is destroyed.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_scene_viewer_package_add_destroy_callback);
	if (scene_viewer_package&&function)
	{
		if (CMISS_CALLBACK_LIST_ADD_CALLBACK(Cmiss_scene_viewer_package_callback)(
			scene_viewer_package->destroy_callback_list,function,user_data))
		{
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_scene_viewer_package_add_destroy_callback.  Could not add callback");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_scene_viewer_package_add_destroy_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_scene_viewer_package_add_destroy_callback */

int Cmiss_scene_viewer_package_remove_destroy_callback(
	struct Cmiss_scene_viewer_package *scene_viewer_package,
	CMISS_CALLBACK_FUNCTION(Cmiss_scene_viewer_package_callback) *function,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 24 January 2007

DESCRIPTION :
Removes the callback calling <function> with <user_data> from
<scene_viewer_package>.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_scene_viewer_package_remove_destroy_callback);
	if (scene_viewer_package&&function)
	{
		if (CMISS_CALLBACK_LIST_REMOVE_CALLBACK(Cmiss_scene_viewer_package_callback)(
			scene_viewer_package->destroy_callback_list,function,user_data))
		{
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Cmiss_scene_viewer_package_remove_destroy_callback.  Could not remove callback");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_scene_viewer_package_remove_destroy_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_scene_viewer_package_remove_destroy_callback */

struct Graphics_buffer_package *Cmiss_scene_viewer_package_get_graphics_buffer_package(
	struct Cmiss_scene_viewer_package *cmiss_scene_viewer_package)
/*******************************************************************************
LAST MODIFIED : 19 January 2007

DESCRIPTION :
==============================================================================*/
{
	struct Graphics_buffer_package *graphics_buffer_package;

	ENTER(Scene_viewer_get_graphics_buffer_package);
	if (cmiss_scene_viewer_package)
	{
		graphics_buffer_package = 
			cmiss_scene_viewer_package->graphics_buffer_package;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_graphics_buffer_package.  Missing scene_viewer");
		graphics_buffer_package=(struct Graphics_buffer_package *)NULL;
	}
	LEAVE;

	return (graphics_buffer_package);
} /* Scene_viewer_get_graphics_buffer_package */

struct Scene *Cmiss_scene_viewer_package_get_default_scene(
	struct Cmiss_scene_viewer_package *cmiss_scene_viewer_package)
/*******************************************************************************
LAST MODIFIED : 19 January 2007

DESCRIPTION :
==============================================================================*/
{
	struct Scene *default_scene;

	ENTER(Scene_viewer_get_default_scene);
	if (cmiss_scene_viewer_package)
	{
		default_scene = cmiss_scene_viewer_package->scene;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_default_scene.  Missing scene_viewer");
		default_scene=(struct Scene *)NULL;
	}
	LEAVE;

	return (default_scene);
} /* Scene_viewer_get_default_scene */

static int Scene_viewer_update_Interactive_tool(
	struct Scene_viewer *scene_viewer, void *interactive_tool_void)
/*******************************************************************************
LAST MODIFIED : 26 April 2007

DESCRIPTION :
Updates the interactive_tool that matches the type of <interactive_tool_void>
to have the same settings as <interactive_tool_void> overwriting the 
settings the individual tool has.  Used to provide compatibility with the old
global tools.  The scene_viewers in a graphics_window are updated separately
from this.
==============================================================================*/
{
	char *tool_name;
	int return_code;
	struct Interactive_tool *global_interactive_tool;
	struct Interactive_tool *scene_viewer_interactive_tool;
	global_interactive_tool = (struct Interactive_tool *)interactive_tool_void;
	
	if (GET_NAME(Interactive_tool)(global_interactive_tool,&tool_name)
		&& (scene_viewer_interactive_tool=
		FIND_BY_IDENTIFIER_IN_MANAGER(Interactive_tool,name)(
		(char *)tool_name,scene_viewer->interactive_tool_manager)))
	{
		Interactive_tool_copy(scene_viewer_interactive_tool,
			global_interactive_tool, (struct MANAGER(Interactive_tool) *)NULL);
	}
	return_code = 1;
	DEALLOCATE(tool_name);
	return (return_code);
}

int Cmiss_scene_viewer_package_update_Interactive_tool(
	struct Cmiss_scene_viewer_package *cmiss_scene_viewer_package,
	struct Interactive_tool *interactive_tool)
/*******************************************************************************
LAST MODIFIED : 26 April 2007

DESCRIPTION :
Updates the interactive tools in each of the scene_viewers created with the
<cmiss_scene_viewer_package> to have the same settings as the <interactive_tool>.
This enables the old global commands to continue to work for all scene_viewers,
however new code should probably modify the particular tools for the 
particular scene_viewer intended.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_scene_viewer_package_update_Interactive_tool);
	if (cmiss_scene_viewer_package)
	{
		return_code = FOR_EACH_OBJECT_IN_LIST(Scene_viewer)(
			Scene_viewer_update_Interactive_tool, (void *)interactive_tool,
			cmiss_scene_viewer_package->scene_viewer_list);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Cmiss_scene_viewer_package_update_Interactive_tool.  Missing scene_viewer");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_scene_viewer_package_update_Interactive_tool */

struct Scene_viewer *CREATE(Scene_viewer)(struct Graphics_buffer *graphics_buffer,
	struct Colour *background_colour,
	struct MANAGER(Light) *light_manager,struct Light *default_light,
	struct MANAGER(Light_model) *light_model_manager,
	struct Light_model *default_light_model,
	struct MANAGER(Scene) *scene_manager,struct Scene *scene,
	struct MANAGER(Texture) *texture_manager,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 19 September 2002

DESCRIPTION :
Creates a Scene_viewer in the widget <parent> to display <scene>.
Note: the parent must be an XmForm since form constraints will be applied.
If any of light_manager, light_model_manager, scene_manager or texture_manager
are supplied, the scene_viewer will automatically redraw in response to changes
of objects from these managers that are in use by the scene_viewer. Redraws are
performed in idle time so that multiple redraws are avoided.
==============================================================================*/
{
 	enum Graphics_buffer_buffering_mode graphics_buffer_buffering_mode;
	enum Graphics_buffer_stereo_mode graphics_buffer_stereo_mode;
 	enum Scene_viewer_buffering_mode buffering_mode;
	enum Scene_viewer_stereo_mode stereo_mode;
	int return_code,i;
	struct Scene_viewer *scene_viewer;

	ENTER(CREATE(Scene_viewer));
	if (graphics_buffer&&background_colour&&default_light_model&&scene&&
		user_interface&&Graphics_buffer_get_buffering_mode(graphics_buffer,
		&graphics_buffer_buffering_mode)&&Graphics_buffer_get_stereo_mode(
		graphics_buffer,&graphics_buffer_stereo_mode))
	{
		return_code=1;
		switch(graphics_buffer_buffering_mode)
		{
			case GRAPHICS_BUFFER_SINGLE_BUFFERING:
			{
				buffering_mode = SCENE_VIEWER_SINGLE_BUFFER;
			} break;
			case GRAPHICS_BUFFER_DOUBLE_BUFFERING:
			{
				buffering_mode = SCENE_VIEWER_DOUBLE_BUFFER;
			} break;
			default:
			{
				display_message(ERROR_MESSAGE, "CREATE(Scene_viewer).  "
					"Invalid Graphics_buffer_buffering_mode.");
				return_code = 0;
			} break;
		}
		switch(graphics_buffer_stereo_mode)
		{
			case GRAPHICS_BUFFER_MONO:
			{
				stereo_mode = SCENE_VIEWER_MONO;
			} break;
			case GRAPHICS_BUFFER_STEREO:
			{
				stereo_mode = SCENE_VIEWER_STEREO;
			} break;
			default:
			{
				display_message(ERROR_MESSAGE, "CREATE(Scene_viewer).  "
					"Invalid Graphics_buffer_stereo_mode.");
				return_code = 0;
			} break;
		}
		if (return_code)
		{
			/* allocate memory for the scene_viewer structure */
			if (ALLOCATE(scene_viewer,struct Scene_viewer,1)&&
				(scene_viewer->list_of_lights=CREATE(LIST(Light)())))
			{
				scene_viewer->graphics_buffer=ACCESS(Graphics_buffer)(graphics_buffer);
				/* access the scene, since don't want it to disappear */
				scene_viewer->scene=ACCESS(Scene)(scene);
				scene_viewer->overlay_scene=(struct Scene *)NULL;
				scene_viewer->input_mode=SCENE_VIEWER_TRANSFORM;
				scene_viewer->temporary_transform_mode=0;
				scene_viewer->user_interface=user_interface;
				scene_viewer->buffering_mode = buffering_mode;
				scene_viewer->stereo_mode = stereo_mode;
				scene_viewer->idle_update_callback_id = (struct Event_dispatcher_idle_callback *)NULL;
				(scene_viewer->background_colour).red=background_colour->red;
				(scene_viewer->background_colour).green=background_colour->green;
				(scene_viewer->background_colour).blue=background_colour->blue;
				scene_viewer->background_texture=(struct Texture *)NULL;
				/* set viewing transformation eye pos, look at point and up-vector */
				/* initially view the x,y plane */
				scene_viewer->eyex=0.0;
				scene_viewer->eyey=0.0;
				scene_viewer->eyez=2.0;
				scene_viewer->lookatx=0.0;
				scene_viewer->lookaty=0.0;
				scene_viewer->lookatz=0.0;
				scene_viewer->upx=0.0;
				scene_viewer->upy=1.0;
				scene_viewer->upz=0.0;
				/* Projection specified by viewing volume and perspective flag */
				/* viewing volume initially a unit cube */
				scene_viewer->left=-1.0;
				scene_viewer->right=1.0;
				scene_viewer->bottom=-1.0;
				scene_viewer->top=1.0;
				scene_viewer->near_plane=0.1;
				scene_viewer->far_plane=1000.0;
				scene_viewer->projection_mode=SCENE_VIEWER_PARALLEL;
				scene_viewer->translate_rate=1.0;
				scene_viewer->tumble_rate=1.5;
				scene_viewer->zoom_rate=1.0;
				scene_viewer->light_model=ACCESS(Light_model)(default_light_model);
				scene_viewer->antialias=0;
				scene_viewer->perturb_lines=0;
				scene_viewer->blending_mode=SCENE_VIEWER_BLEND_NORMAL;
				scene_viewer->depth_of_field=0.0;  /* default 0==infinite */
				scene_viewer->focal_depth=0.0;
				scene_viewer->blending_mode=SCENE_VIEWER_BLEND_NORMAL;
 				scene_viewer->transform_flag=0;
				scene_viewer->first_fast_change=1;
				scene_viewer->fast_changing=0;
				scene_viewer->stereo_eye_spacing=0.25;
				scene_viewer->swap_buffers=0;
				if (default_light)
				{
					ADD_OBJECT_TO_LIST(Light)(default_light,
						scene_viewer->list_of_lights);
				}
				/* managers and callback IDs for automatic updates */
				scene_viewer->light_manager=light_manager;
				scene_viewer->light_manager_callback_id=(void *)NULL;
				scene_viewer->light_model_manager=light_model_manager;
				scene_viewer->light_model_manager_callback_id=(void *)NULL;
				scene_viewer->scene_manager=scene_manager;
				scene_viewer->scene_manager_callback_id=(void *)NULL;
				scene_viewer->texture_manager=texture_manager;
				scene_viewer->texture_manager_callback_id=(void *)NULL;
				/* no current interactive_tool */
				scene_viewer->interactive_tool=(struct Interactive_tool *)NULL;
				/* Currently only set when created from a Cmiss_scene_viewer_package
					to avoid changing the interface */
				scene_viewer->interactive_tool_manager=
					(struct MANAGER(Interactive_tool) *)NULL;
				scene_viewer->order_independent_transparency_data = 
					(struct Scene_viewer_order_independent_transparency_data *)NULL;

				/* set projection matrices to identity */
				for (i=0;i<16;i++)
				{
					if (0==(i % 5))
					{
						scene_viewer->projection_matrix[i]=1.0;
						scene_viewer->modelview_matrix[i]=1.0;
					}
					else
					{
						scene_viewer->projection_matrix[i]=0.0;
						scene_viewer->modelview_matrix[i]=0.0;
					}
				}
				scene_viewer->NDC_width=scene_viewer->right-scene_viewer->left;
				scene_viewer->NDC_height=scene_viewer->top-scene_viewer->bottom;
				scene_viewer->NDC_top=scene_viewer->top;
				scene_viewer->NDC_left=scene_viewer->left;
				scene_viewer->viewport_mode=SCENE_VIEWER_RELATIVE_VIEWPORT;
				scene_viewer->user_viewport_top=0.0;
				scene_viewer->user_viewport_left=0.0;
				scene_viewer->user_viewport_pixels_per_unit_x=1.0;
				scene_viewer->user_viewport_pixels_per_unit_y=1.0;
				scene_viewer->background_texture=(struct Texture *)NULL;
				scene_viewer->bk_texture_top=0.0;
				scene_viewer->bk_texture_left=0.0;
				scene_viewer->bk_texture_width=0.0;
				scene_viewer->bk_texture_height=0.0;
				scene_viewer->interact_mode=SCENE_VIEWER_INTERACT_STANDARD;
				scene_viewer->drag_mode=SV_DRAG_NOTHING;
				scene_viewer->previous_pointer_x = 0;
				scene_viewer->previous_pointer_y = 0;
				/* automatic tumble */
				scene_viewer->tumble_axis[0] = 1.0;
				scene_viewer->tumble_axis[1] = 0.0;
				scene_viewer->tumble_axis[2] = 0.0;
				scene_viewer->tumble_angle = 0;
				scene_viewer->tumble_active = 0;
				/* by default, use undistort stuff on textures */
				scene_viewer->bk_texture_undistort_on=1;
				scene_viewer->bk_texture_max_pixels_per_polygon=16.0;
				scene_viewer->transparency_mode=SCENE_VIEWER_FAST_TRANSPARENCY;
				scene_viewer->transparency_layers=1;
				scene_viewer->input_callback_list=
					CREATE(LIST(CMISS_CALLBACK_ITEM(Scene_viewer_input_callback)))();
				/* Add the default callback */
				CMISS_CALLBACK_LIST_ADD_CALLBACK(Scene_viewer_input_callback)(
					scene_viewer->input_callback_list,
					Scene_viewer_default_input_callback,NULL);
				scene_viewer->sync_callback_list=
					CREATE(LIST(CMISS_CALLBACK_ITEM(Scene_viewer_callback)))();
				scene_viewer->transform_callback_list=
					CREATE(LIST(CMISS_CALLBACK_ITEM(Scene_viewer_callback)))();
				scene_viewer->destroy_callback_list=
					CREATE(LIST(CMISS_CALLBACK_ITEM(Scene_viewer_callback)))();
				scene_viewer->repaint_required_callback_list=
					CREATE(LIST(CMISS_CALLBACK_ITEM(Scene_viewer_callback)))();
				scene_viewer->pixel_width=0;
				scene_viewer->pixel_height=0;
				scene_viewer->update_pixel_image=0;
				scene_viewer->pixel_data = (char *)NULL;
				for (i = 0 ; i < MAX_CLIP_PLANES ; i++)
				{
					scene_viewer->clip_planes_enable[i] = 0;
					scene_viewer->clip_planes[i * 4] = 0.0;
					scene_viewer->clip_planes[i * 4 + 1] = 0.0;
					scene_viewer->clip_planes[i * 4 + 2] = 0.0;
					scene_viewer->clip_planes[i * 4 + 3] = 0.0;
				}
#if defined (WIN32_SYSTEM)
				/* Clear twice, if set then the glClear in the background will be called
					twice, which appears to work around a rendering bug on ATI windows driver 6.14.0010.6706 */
				scene_viewer->clear_twice_flag = 1;
#endif /* defined (WIN32_SYSTEM) */
				scene_viewer->frame_count = 0;
				
				/* add callbacks to the graphics buffer */
				Graphics_buffer_add_initialise_callback(graphics_buffer,
					Scene_viewer_initialise_callback, scene_viewer);
				Graphics_buffer_add_resize_callback(graphics_buffer,
					Scene_viewer_resize_callback, scene_viewer);
				Graphics_buffer_add_expose_callback(graphics_buffer,
					Scene_viewer_expose_callback, scene_viewer);

				Graphics_buffer_add_input_callback(graphics_buffer,
					 Scene_viewer_graphics_buffer_input_callback, scene_viewer);

				Scene_viewer_awaken(scene_viewer);
				Graphics_buffer_awaken(scene_viewer->graphics_buffer);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"CREATE(Scene_viewer).  Not enough memory for scene_viewer");
			}
		}
		else
		{
			scene_viewer=(struct Scene_viewer *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Scene_viewer).  Invalid argument(s)");
		scene_viewer=(struct Scene_viewer *)NULL;
	}
	LEAVE;

	return (scene_viewer);
} /* CREATE(Scene_viewer) */

int DESTROY(Scene_viewer)(struct Scene_viewer **scene_viewer_address)
/*******************************************************************************
LAST MODIFIED : 29 September 2000

DESCRIPTION :
Closes the scene_viewer and disposes of the scene_viewer data structure.
==============================================================================*/
{
	int return_code;
	struct Scene_viewer *scene_viewer;

	ENTER(DESTROY(Scene_viewer));
	if (scene_viewer_address&&(scene_viewer= *scene_viewer_address))
	{
		Scene_viewer_sleep(scene_viewer);
		/* send the destroy callbacks */
		if (scene_viewer->destroy_callback_list)
		{
			CMISS_CALLBACK_LIST_CALL(Scene_viewer_callback)(
				scene_viewer->destroy_callback_list,scene_viewer,NULL);
			DESTROY( LIST(CMISS_CALLBACK_ITEM(Scene_viewer_callback)))(
				&scene_viewer->destroy_callback_list);
		}
		/* dispose of our data structure */
		if (scene_viewer->background_texture)
		{
			DEACCESS(Texture)(&(scene_viewer->background_texture));
		}
		DEACCESS(Scene)(&(scene_viewer->scene));
		if (scene_viewer->overlay_scene)
		{
			DEACCESS(Scene)(&(scene_viewer->overlay_scene));
		}
		DEACCESS(Light_model)(&(scene_viewer->light_model));
		DESTROY(LIST(Light))(&(scene_viewer->list_of_lights));
		if (scene_viewer->sync_callback_list)
		{
			DESTROY(LIST(CMISS_CALLBACK_ITEM(Scene_viewer_callback)))(
				&scene_viewer->sync_callback_list);
		}
		if (scene_viewer->transform_callback_list)
		{
			DESTROY(LIST(CMISS_CALLBACK_ITEM(Scene_viewer_callback)))(
				&scene_viewer->transform_callback_list);
		}
		if (scene_viewer->input_callback_list)
		{
			DESTROY(LIST(CMISS_CALLBACK_ITEM(Scene_viewer_input_callback)))(
				&scene_viewer->input_callback_list);
		}
		if (scene_viewer->repaint_required_callback_list)
		{
			DESTROY(LIST(CMISS_CALLBACK_ITEM(Scene_viewer_callback)))(
				&scene_viewer->repaint_required_callback_list);
		}
		if (scene_viewer->order_independent_transparency_data)
		{
			order_independent_finalise(
				&scene_viewer->order_independent_transparency_data);
		}

		/* must destroy the widget */
		DEACCESS(Graphics_buffer)(&scene_viewer->graphics_buffer);				
		if (scene_viewer->pixel_data)
		{
			DEALLOCATE(scene_viewer->pixel_data);
		}
		DEALLOCATE(*scene_viewer_address);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Scene_viewer).  Missing scene_viewer");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Scene_viewer) */

struct Scene_viewer *ACCESS(Scene_viewer)(struct Scene_viewer *scene_viewer)
/*******************************************************************************
LAST MODIFIED : 19 January 2007

DESCRIPTION :
==============================================================================*/
{
	//Do nothing as the scene viewer removes itself from the package list
	return(scene_viewer);
}

int DEACCESS(Scene_viewer)(struct Scene_viewer **scene_viewer_address)
/*******************************************************************************
LAST MODIFIED : 19 January 2007

DESCRIPTION :
==============================================================================*/
{
	//Do nothing as the scene viewer removes itself from the package list
	*scene_viewer_address = (struct Scene_viewer *)NULL;
	return(1);
}
DECLARE_LIST_FUNCTIONS(Scene_viewer)

struct Scene_viewer *create_Scene_viewer_from_package(
	struct Graphics_buffer *graphics_buffer,
	struct Cmiss_scene_viewer_package *cmiss_scene_viewer_package,
	struct Scene *scene)
/*******************************************************************************
LAST MODIFIED : 19 January 2007

DESCRIPTION :
==============================================================================*/
{
	struct MANAGER(Interactive_tool) *new_interactive_tool_manager;
	struct Scene_viewer *scene_viewer;

	ENTER(create_Scene_viewer_from_package);
	if (graphics_buffer && cmiss_scene_viewer_package && scene)
	{
		scene_viewer = CREATE(Scene_viewer)(graphics_buffer,
			cmiss_scene_viewer_package->background_colour,
			cmiss_scene_viewer_package->light_manager,
			cmiss_scene_viewer_package->default_light,
			cmiss_scene_viewer_package->light_model_manager,
			cmiss_scene_viewer_package->default_light_model,
			cmiss_scene_viewer_package->scene_manager,
			scene,
			cmiss_scene_viewer_package->texture_manager,
			cmiss_scene_viewer_package->user_interface);
		
		new_interactive_tool_manager = CREATE(MANAGER(Interactive_tool))();
		FOR_EACH_OBJECT_IN_MANAGER(Interactive_tool)(
			Interactive_tool_create_copy_iterator, new_interactive_tool_manager,
			cmiss_scene_viewer_package->interactive_tool_manager);

		Scene_viewer_set_interactive_tool(scene_viewer,
			FIND_BY_IDENTIFIER_IN_MANAGER(Interactive_tool,name)
			("transform_tool", new_interactive_tool_manager));
		scene_viewer->interactive_tool_manager = new_interactive_tool_manager;

		/* Add this scene_viewer to the package list */
		ADD_OBJECT_TO_LIST(Scene_viewer)(scene_viewer,
			cmiss_scene_viewer_package->scene_viewer_list);

		/* Register a callback so that if the scene_viewer is destroyed
			then it is removed from the list */
		Scene_viewer_add_destroy_callback(scene_viewer,
			Scene_viewer_destroy_remove_from_package, 
			cmiss_scene_viewer_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,"create_Scene_viewer_from_package.  Invalid argument(s)");
		scene_viewer = (struct Scene_viewer *)NULL;
	}
	LEAVE;

	return (scene_viewer);
} /* create_Scene_viewer_from_package */

int Scene_viewer_awaken(struct Scene_viewer *scene_viewer)
/*******************************************************************************
LAST MODIFIED : 29 September 2000

DESCRIPTION :
Restores manager callbacks of previously inactive scene_viewer. Must call after
Scene_viewer_sleep to restore normal activity.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_awaken);
	if (scene_viewer)
	{
		/* register for lighting changes */
		if (scene_viewer->light_manager &&
			(!scene_viewer->light_manager_callback_id))
		{
			scene_viewer->light_manager_callback_id=
				MANAGER_REGISTER(Light)(Scene_viewer_light_change,
					(void *)scene_viewer,scene_viewer->light_manager);
		}
		if (scene_viewer->light_model_manager &&
			(!scene_viewer->light_model_manager_callback_id))
		{
			scene_viewer->light_model_manager_callback_id=
				MANAGER_REGISTER(Light_model)(Scene_viewer_light_model_change,
					(void *)scene_viewer,scene_viewer->light_model_manager);
		}
		/* register for any scene changes */
		if (scene_viewer->scene_manager &&
			(!scene_viewer->scene_manager_callback_id))
		{
			scene_viewer->scene_manager_callback_id=
				MANAGER_REGISTER(Scene)(Scene_viewer_scene_change,
					(void *)scene_viewer,scene_viewer->scene_manager);
		}
		/* register for any texture changes */
		if (scene_viewer->texture_manager &&
			(!scene_viewer->texture_manager_callback_id))
		{
			scene_viewer->texture_manager_callback_id=
				MANAGER_REGISTER(Texture)(Scene_viewer_texture_change,
					(void *)scene_viewer,scene_viewer->texture_manager);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Scene_viewer_awaken.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_awaken */

int Scene_viewer_get_freespin_tumble_angle(struct Scene_viewer *scene_viewer,
	double *tumble_angle)
/*******************************************************************************
LAST MODIFIED : 9 October 2003

DESCRIPTION :
Gets the <scene_viewer> tumble angle.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_get_freespin_tumble_angle);
	if (scene_viewer && tumble_angle)
	{
		*tumble_angle = scene_viewer->tumble_angle;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_freespin_tumble_angle.  Missing scene_viewer");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_get_freespin_tumble_angle */

int Scene_viewer_set_freespin_tumble_angle(struct Scene_viewer *scene_viewer,
	double tumble_angle)
/*******************************************************************************
LAST MODIFIED : 17 February 2005

DESCRIPTION :
Sets the <scene_viewer> tumble angle.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_get_freespin_tumble_angle);
	if (scene_viewer)
	{
		scene_viewer->tumble_angle = tumble_angle;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_set_freespin_tumble_angle.  Missing scene_viewer");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_set_freespin_tumble_angle */

int Scene_viewer_get_freespin_tumble_axis(struct Scene_viewer *scene_viewer,
	float *tumble_axis)
/*******************************************************************************
LAST MODIFIED : 9 October 2003

DESCRIPTION :
Gets the <scene_viewer> tumble axis.  The <tumble_axis> is the vector
about which the scene is turning relative to its lookat point.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_get_freespin_tumble_axis);
	if (scene_viewer && tumble_axis)
	{
		tumble_axis[0] = scene_viewer->tumble_axis[0];
		tumble_axis[1] = scene_viewer->tumble_axis[1];
		tumble_axis[2] = scene_viewer->tumble_axis[2];
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_freespin_tumble_axis.  Missing scene_viewer");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_get_freespin_tumble_axis */

int Scene_viewer_start_freespin(struct Scene_viewer *scene_viewer,
	float *tumble_axis, double tumble_angle)
/*******************************************************************************
LAST MODIFIED : 10 September 2003

DESCRIPTION :
Sets the <scene_viewer> spinning in idle time.  The <tumble_axis> is the vector
about which the scene is turning relative to its lookat point and the
<tumble_angle> controls how much it turns on each redraw.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_start_freespin);
	if (scene_viewer && tumble_axis)
	{
		scene_viewer->tumble_active = 1;
		scene_viewer->tumble_axis[0] = tumble_axis[0];
		scene_viewer->tumble_axis[1] = tumble_axis[1];
		scene_viewer->tumble_axis[2] = tumble_axis[2];
		scene_viewer->tumble_angle = tumble_angle;
		scene_viewer->fast_changing = 0;
		/* Repost the idle callback */
		if(!scene_viewer->idle_update_callback_id)
		{
			scene_viewer->idle_update_callback_id = Event_dispatcher_add_idle_callback(
				User_interface_get_event_dispatcher(scene_viewer->user_interface),
				Scene_viewer_idle_update_callback, (void *)scene_viewer,
				EVENT_DISPATCHER_IDLE_UPDATE_SCENE_VIEWER_PRIORITY);			
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_start_freespin.  Missing scene_viewer");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_start_freespin */

int Scene_viewer_stop_animations(struct Scene_viewer *scene_viewer)
/*******************************************************************************
LAST MODIFIED : 29 September 2000

DESCRIPTION :
Tells the <scene_viewer> to stop all automatic informations that it produces,
eg. automatic tumble.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_stop_animations);
	if (scene_viewer)
	{
		scene_viewer->tumble_active = 0;
		scene_viewer->tumble_angle = 0.0;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_stop_animations.  Missing scene_viewer");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_stop_animations */

int Scene_viewer_sleep(struct Scene_viewer *scene_viewer)
/*******************************************************************************
LAST MODIFIED : 29 September 2000

DESCRIPTION :
Turns off any pending automatic tumbles or redraws in idle time, and removes
any manager callbacks to minimise impact of inactive scene_viewer on rest of
program. Must call Scene_viewer_awaken to restore manager callbacks.
Must call this in DESTROY function.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_sleep);
	if (scene_viewer)
	{
		if (scene_viewer->idle_update_callback_id)
		{
			Event_dispatcher_remove_idle_callback(
				User_interface_get_event_dispatcher(scene_viewer->user_interface),
				scene_viewer->idle_update_callback_id);
			scene_viewer->idle_update_callback_id=(struct Event_dispatcher_idle_callback *)NULL;
		}
		scene_viewer->tumble_active = 0;
		scene_viewer->tumble_angle = 0.0;
		/* turn off manager messages */
		if (scene_viewer->light_manager_callback_id)
		{
			MANAGER_DEREGISTER(Light)(
				scene_viewer->light_manager_callback_id,
				scene_viewer->light_manager);
			scene_viewer->light_manager_callback_id=(void *)NULL;
		}
		if (scene_viewer->light_model_manager_callback_id)
		{
			MANAGER_DEREGISTER(Light_model)(
				scene_viewer->light_model_manager_callback_id,
				scene_viewer->light_model_manager);
			scene_viewer->light_model_manager_callback_id=(void *)NULL;
		}
		if (scene_viewer->scene_manager_callback_id)
		{
			MANAGER_DEREGISTER(Scene)(
				scene_viewer->scene_manager_callback_id,
				scene_viewer->scene_manager);
			scene_viewer->scene_manager_callback_id=(void *)NULL;
		}
		if (scene_viewer->texture_manager_callback_id)
		{
			MANAGER_DEREGISTER(Texture)(scene_viewer->texture_manager_callback_id,
				scene_viewer->texture_manager);
			scene_viewer->texture_manager_callback_id=(void *)NULL;
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Scene_viewer_sleep.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_sleep */

int Scene_viewer_get_background_colour(struct Scene_viewer *scene_viewer,
	struct Colour *background_colour)
/*******************************************************************************
LAST MODIFIED : 21 January 1998

DESCRIPTION :
Returns the background_colour of the scene_viewer.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_get_background_colour);
	if (scene_viewer&&background_colour)
	{
		background_colour->red=scene_viewer->background_colour.red;
		background_colour->green=scene_viewer->background_colour.green;
		background_colour->blue=scene_viewer->background_colour.blue;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_background_colour.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_get_background_colour */

int Scene_viewer_set_background_colour(struct Scene_viewer *scene_viewer,
	struct Colour *background_colour)
/*******************************************************************************
LAST MODIFIED : 21 January 1998

DESCRIPTION :
Sets the background_colour of the scene_viewer.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_set_background_colour);
	if (scene_viewer&&background_colour)
	{
		scene_viewer->background_colour.red=background_colour->red;
		scene_viewer->background_colour.green=background_colour->green;
		scene_viewer->background_colour.blue=background_colour->blue;
		Scene_viewer_redraw(scene_viewer);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_set_background_colour.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_set_background_colour */

struct Texture *Scene_viewer_get_background_texture(
	struct Scene_viewer *scene_viewer)
/*******************************************************************************
LAST MODIFIED : 21 January 1998

DESCRIPTION :
Retrieves the Scene_viewer's background_texture. Note that NULL is the valid
return if there is no background texture.
==============================================================================*/
{
	struct Texture *return_texture;

	ENTER(Scene_viewer_get_background_texture);
	if (scene_viewer)
	{
		return_texture=scene_viewer->background_texture;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_background_texture.  Missing scene_viewer");
		return_texture=(struct Texture *)NULL;
	}
	LEAVE;

	return (return_texture);
} /* Scene_viewer_get_background_texture */

int Scene_viewer_set_background_texture(struct Scene_viewer *scene_viewer,
	struct Texture *background_texture)
/*******************************************************************************
LAST MODIFIED : 21 January 1998

DESCRIPTION :
Sets the background_texture to be displayed in the Scene_viewer. Information
on how it will be displayed is set in Scene_viewer_set_background_texture_info.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_set_background_texture);
	if (scene_viewer)
	{
		if (background_texture)
		{
			ACCESS(Texture)(background_texture);
		}
		if (scene_viewer->background_texture)
		{
			DEACCESS(Texture)(&(scene_viewer->background_texture));
		}
		scene_viewer->background_texture=background_texture;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_set_background_texture.  Missing scene_viewer");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_set_background_texture */

int Scene_viewer_set_background_texture_by_name(struct Scene_viewer *scene_viewer,
	const char *name)
/*******************************************************************************
LAST MODIFIED : 19 January 2007

DESCRIPTION :
Sets the Scene_viewer scene from names in the scene manager.
==============================================================================*/
{
	int return_code;
	struct Texture *texture;

	ENTER(Scene_viewer_set_background_texture_by_name);
	if (scene_viewer&&name)
	{
		if (texture=FIND_BY_IDENTIFIER_IN_MANAGER(Texture,name)(
			(char *)name, scene_viewer->texture_manager))
		{
			return_code = Scene_viewer_set_background_texture(scene_viewer, texture);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Scene_viewer_set_background_texture_by_name.  "
				"Unable to find a texture named %s.", name);
			return_code = 0;
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_set_background_texture_by_name.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_set_background_texture_by_name */

int Scene_viewer_get_background_texture_info(struct Scene_viewer *scene_viewer,
	double *bk_texture_left,double *bk_texture_top,
	double *bk_texture_width,double *bk_texture_height,
	int *bk_texture_undistort_on,double *bk_texture_max_pixels_per_polygon)
/*******************************************************************************
LAST MODIFIED : 28 September 1999

DESCRIPTION :
See Scene_viewer_set_background_texture_info for meaning of return values.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_get_background_texture_info);
	if (scene_viewer&&bk_texture_left&&bk_texture_top&&
		bk_texture_width&&bk_texture_height&&
		bk_texture_undistort_on&&bk_texture_max_pixels_per_polygon)
	{
		*bk_texture_left=scene_viewer->bk_texture_left;
		*bk_texture_top=scene_viewer->bk_texture_top;
		*bk_texture_width=scene_viewer->bk_texture_width;
		*bk_texture_height=scene_viewer->bk_texture_height;
		*bk_texture_undistort_on=scene_viewer->bk_texture_undistort_on;
		*bk_texture_max_pixels_per_polygon=
			scene_viewer->bk_texture_max_pixels_per_polygon;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_background_texture_info.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_get_background_texture_info */

int Scene_viewer_set_background_texture_info(struct Scene_viewer *scene_viewer,
	double bk_texture_left,double bk_texture_top,
	double bk_texture_width,double bk_texture_height,
	int bk_texture_undistort_on,double bk_texture_max_pixels_per_polygon)
/*******************************************************************************
LAST MODIFIED : 28 September 1999

DESCRIPTION :
If there is a background_texture in the scene_viewer, these values specify the
top,left corner, in user coordinates, where it will be displayed, while the
next two parameters specify the size it will have in these coordinates.
If the bk_texture_undistort_on flag is set, radial distortion parameters from the background texture are un-distorted when the
texture is displayed. It does this by drawing it as a collection of polygons;
the last parameter controls the size of polygons used to do this.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_set_background_texture_info);
	if (scene_viewer&&(0.0 != bk_texture_width)&&
		(0.0 != bk_texture_height)&&
		(0.0 < bk_texture_max_pixels_per_polygon))
	{
		scene_viewer->bk_texture_left=bk_texture_left;
		scene_viewer->bk_texture_top=bk_texture_top;
		scene_viewer->bk_texture_width=bk_texture_width;
		scene_viewer->bk_texture_height=bk_texture_height;
		scene_viewer->bk_texture_undistort_on=bk_texture_undistort_on;
		scene_viewer->bk_texture_max_pixels_per_polygon=
			bk_texture_max_pixels_per_polygon;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_set_background_texture_info.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_set_background_texture_info */

int Scene_viewer_get_border_width(struct Scene_viewer *scene_viewer)
/*******************************************************************************
LAST MODIFIED : 12 October 1998

DESCRIPTION :
Returns the border_width of the scene_viewer widget. Note that the border is
only shown on the bottom and right of each viewer in the graphics window.
==============================================================================*/
{
	int border_width;

	ENTER(Scene_viewer_get_border_width);
	if (scene_viewer)
	{
		border_width = Graphics_buffer_get_border_width(
			scene_viewer->graphics_buffer);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_border_width.  Missing scene_viewer");
		border_width=0;
	}
	LEAVE;

	return (border_width);
} /* Scene_viewer_get_border_width */

int Scene_viewer_set_border_width(struct Scene_viewer *scene_viewer,
	int border_width)
/*******************************************************************************
LAST MODIFIED : 2 July 2002

DESCRIPTION :
Sets the border_width of the scene_viewer widget. Note that the border is
only shown on the bottom and right of each viewer in the graphics window.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_set_border_width);
	if (scene_viewer&&(0<=border_width))
	{
		Graphics_buffer_set_border_width(scene_viewer->graphics_buffer,
			border_width);
		scene_viewer->transform_flag = 1;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_set_border_width.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_set_border_width */

enum Scene_viewer_buffering_mode Scene_viewer_get_buffering_mode(
	struct Scene_viewer *scene_viewer)
/*******************************************************************************
LAST MODIFIED : 15 October 1998

DESCRIPTION :
Returns the buffer mode - single_buffer/double_buffer - of the Scene_viewer.
==============================================================================*/
{
	enum Scene_viewer_buffering_mode buffering_mode;

	ENTER(Scene_viewer_get_buffering_mode);
	if (scene_viewer)
	{
		buffering_mode=scene_viewer->buffering_mode;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_buffering_mode.  Invalid argument(s)");
		/* return any valid mode */
		buffering_mode=SCENE_VIEWER_DOUBLE_BUFFER;
	}
	LEAVE;

	return (buffering_mode);
} /* Scene_viewer_get_buffering_mode */

enum Scene_viewer_stereo_mode Scene_viewer_get_stereo_mode(
	struct Scene_viewer *scene_viewer)
/*******************************************************************************
LAST MODIFIED : 16 September 2002

DESCRIPTION :
Returns the stereo mode - mono/stereo - of the Scene_viewer.
==============================================================================*/
{
	enum Scene_viewer_stereo_mode stereo_mode;

	ENTER(Scene_viewer_get_stereo_mode);
	if (scene_viewer)
	{
		stereo_mode=scene_viewer->stereo_mode;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_stereo_mode.  Invalid argument(s)");
		/* return any valid mode */
		stereo_mode=SCENE_VIEWER_MONO;
	}
	LEAVE;

	return (stereo_mode);
} /* Scene_viewer_get_stereo_mode */

enum Scene_viewer_input_mode Scene_viewer_get_input_mode(
	struct Scene_viewer *scene_viewer)
/*******************************************************************************
LAST MODIFIED : 6 October 1998

DESCRIPTION :
Returns the input_mode of the Scene_viewer.
==============================================================================*/
{
	enum Scene_viewer_input_mode input_mode;

	ENTER(Scene_viewer_get_input_mode);
	if (scene_viewer)
	{
		input_mode=scene_viewer->input_mode;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_input_mode.  Invalid argument(s)");
		input_mode=SCENE_VIEWER_NO_INPUT;
	}
	LEAVE;

	return (input_mode);
} /* Scene_viewer_get_input_mode */

int Scene_viewer_set_input_mode(struct Scene_viewer *scene_viewer,
	enum Scene_viewer_input_mode input_mode)
/*******************************************************************************
LAST MODIFIED : 6 October 2000

DESCRIPTION :
Sets the input_mode of the Scene_viewer.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_set_input_mode);
	if (scene_viewer&&((SCENE_VIEWER_NO_INPUT==input_mode)||
		(SCENE_VIEWER_NO_INPUT_OR_DRAW==input_mode)||
		(SCENE_VIEWER_UPDATE_ON_CLICK==input_mode)||
		(SCENE_VIEWER_SELECT==input_mode)||
		(SCENE_VIEWER_TRANSFORM==input_mode)))
	{
		/* clear automatic tumble since cannot make successful input while on */
		scene_viewer->tumble_active = 0;
		scene_viewer->tumble_angle = 0.0;
		scene_viewer->input_mode=input_mode;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_set_input_mode.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_set_input_mode */

int Scene_viewer_add_light(struct Scene_viewer *scene_viewer,
	struct Light *light)
/*******************************************************************************
LAST MODIFIED : 3 December 1997

DESCRIPTION :
Adds a light to the Scene_viewer list_of_lights.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_add_light);
	if (scene_viewer&&light)
	{
		if (!IS_OBJECT_IN_LIST(Light)(light,scene_viewer->list_of_lights))
		{
			return_code=ADD_OBJECT_TO_LIST(Light)(light,scene_viewer->list_of_lights);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Scene_viewer_add_light.  Light already in list");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_add_light.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_add_light */

int Scene_viewer_has_light(struct Scene_viewer *scene_viewer,
	struct Light *light)
/*******************************************************************************
LAST MODIFIED : 12 December 1997

DESCRIPTION :
Returns true if <Scene_viewer> has <light> in its list_of_lights, OR if <light>
is NULL, returns true if <scene_viewer> has any lights.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_has_light);
	if (scene_viewer)
	{
		if (light)
		{
			return_code=IS_OBJECT_IN_LIST(Light)(light,scene_viewer->list_of_lights);
		}
		else
		{
			return_code=NUMBER_IN_LIST(Light)(scene_viewer->list_of_lights);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_has_light.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_has_light */

int Scene_viewer_has_light_in_list(struct Scene_viewer *scene_viewer,
	struct LIST(Light) *light_list)
/*******************************************************************************
LAST MODIFIED : 30 May 2001

DESCRIPTION :
Returns true if the list_of_lights in <Scene> intersects <light_list>.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_has_light_in_list);
	if (scene_viewer && light_list)
	{
		if (FIRST_OBJECT_IN_LIST_THAT(Light)(Light_is_in_list,
			(void *)light_list, scene_viewer->list_of_lights))
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
		display_message(ERROR_MESSAGE,
			"Scene_viewer_has_light_in_list.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_has_light_in_list */

int Scene_viewer_remove_light(struct Scene_viewer *scene_viewer,
	struct Light *light)
/*******************************************************************************
LAST MODIFIED : 3 December 1997

DESCRIPTION :
Removes a light from the Scene_viewer list_of_lights.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_remove_light);
	if (scene_viewer&&light)
	{
		if (IS_OBJECT_IN_LIST(Light)(light,scene_viewer->list_of_lights))
		{
			return_code=REMOVE_OBJECT_FROM_LIST(Light)(light,
				scene_viewer->list_of_lights);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Scene_viewer_remove_light.  Light not in list");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_remove_light.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_remove_light */

int Scene_viewer_add_clip_plane(struct Scene_viewer *scene_viewer,
	double A, double B, double C, double D)
/*******************************************************************************
LAST MODIFIED : 12 December 2000

DESCRIPTION :
Sets a clip plane that defines a plane in Modelview space, (Ax+By+Cz=D).
==============================================================================*/
{
	int i, index, return_code;

	ENTER(Scene_viewer_add_clip_plane);
	if (scene_viewer)
	{
		return_code=1;
		index = -1;
		for (i = 0 ; return_code && (i < MAX_CLIP_PLANES) ; i++)
		{
			if (!scene_viewer->clip_planes_enable[i])
			{
				if (index == -1)
				{
					index = i;
				}
			}
			else
			{
				/* Check it doesn't already exist */
				if ((A == scene_viewer->clip_planes[i * 4]) &&
					(B == scene_viewer->clip_planes[i * 4 + 1])  &&
					(C == scene_viewer->clip_planes[i * 4 + 2])  &&
					(D == scene_viewer->clip_planes[i * 4 + 3]))
				{
					display_message(ERROR_MESSAGE, "Scene_viewer_add_clip_plane.  "
						"Clip plane %fx+%fy+%fz=%f already exists", A, B, C, D);
					return_code=0;
				}
			}
		}
		if ((index != -1) && return_code)
		{
			scene_viewer->clip_planes_enable[index] = 1;
			scene_viewer->clip_planes[index * 4] = A;
			scene_viewer->clip_planes[index * 4 + 1] = B;
			scene_viewer->clip_planes[index * 4 + 2] = C;
			scene_viewer->clip_planes[index * 4 + 3] = D;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_add_clip_plane.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_add_clip_plane */

int Scene_viewer_remove_clip_plane(struct Scene_viewer *scene_viewer,
	double A, double B, double C, double D)
/*******************************************************************************
LAST MODIFIED : 12 December 2000

DESCRIPTION :
Removes a clip plane that defines a plane in Modelview space, fails if the
exact plane isn't defined as a clip plane.
==============================================================================*/
{
	int i, return_code;

	ENTER(Scene_viewer_remove_clip_plane);
	if (scene_viewer)
	{
		return_code = 0;
		for (i = 0 ; i < MAX_CLIP_PLANES ; i++)
		{
			if (scene_viewer->clip_planes_enable[i])
			{
				if ((A == scene_viewer->clip_planes[i * 4]) &&
					(B == scene_viewer->clip_planes[i * 4 + 1])  &&
					(C == scene_viewer->clip_planes[i * 4 + 2])  &&
					(D == scene_viewer->clip_planes[i * 4 + 3]))
				{
					scene_viewer->clip_planes_enable[i] = 0;
					scene_viewer->clip_planes[i * 4] = 0.0;
					scene_viewer->clip_planes[i * 4 + 1] = 0.0;
					scene_viewer->clip_planes[i * 4 + 2] = 0.0;
					scene_viewer->clip_planes[i * 4 + 3] = 0.0;
					return_code = 1;
				}
			}
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE, "Scene_viewer_remove_clip_plane.  "
				"Clip plane %fx+%fy+%fz=%f not found.", A, B, C, D);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_remove_clip_plane.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_remove_clip_plane */

int Scene_viewer_get_interact_mode(struct Scene_viewer *scene_viewer,
	enum Scene_viewer_interact_mode *interact_mode)
/*******************************************************************************
LAST MODIFIED : 2 November 2006

DESCRIPTION :
Get the mouse and keyboard interaction configuration 
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_get_interact_mode);
	if (scene_viewer)
	{
		*interact_mode = scene_viewer->interact_mode;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_interact_mode.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_get_interact_mode */

int Scene_viewer_set_interact_mode(struct Scene_viewer *scene_viewer,
	enum Scene_viewer_interact_mode interact_mode)
/*******************************************************************************
LAST MODIFIED : 2 November 2006

DESCRIPTION :
Set the mouse and keyboard interaction configuration 
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_set_interact_mode);
	if (scene_viewer&&((SCENE_VIEWER_INTERACT_STANDARD==interact_mode)||
		(SCENE_VIEWER_INTERACT_2D==interact_mode)))
	{
		return_code=1;
		scene_viewer->interact_mode=interact_mode;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_set_interact_mode.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_set_interact_mode */

struct Light_model *Scene_viewer_get_light_model(
	struct Scene_viewer *scene_viewer)
/*******************************************************************************
LAST MODIFIED : 3 December 1997

DESCRIPTION :
Returns the Scene_viewer light_model.
==============================================================================*/
{
	struct Light_model *return_light_model;

	ENTER(Scene_viewer_get_light_model);
	if (scene_viewer)
	{
		return_light_model=scene_viewer->light_model;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_light_model.  Invalid argument(s)");
		return_light_model=(struct Light_model *)NULL;
	}
	LEAVE;

	return (return_light_model);
} /* Scene_viewer_get_light_model */

int Scene_viewer_set_light_model(struct Scene_viewer *scene_viewer,
	struct Light_model *light_model)
/*******************************************************************************
LAST MODIFIED : 13 December 1997

DESCRIPTION :
Sets the Scene_viewer light_model.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_set_light_model);
	if (scene_viewer&&light_model)
	{
		if (light_model != scene_viewer->light_model)
		{
			DEACCESS(Light_model)(&(scene_viewer->light_model));
			scene_viewer->light_model=ACCESS(Light_model)(light_model);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_set_light_model.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_set_light_model */

int Scene_viewer_get_lookat_parameters(struct Scene_viewer *scene_viewer,
	double *eyex,double *eyey,double *eyez,
	double *lookatx,double *lookaty,double *lookatz,
	double *upx,double *upy,double *upz)
/*******************************************************************************
LAST MODIFIED : 21 November 1997

DESCRIPTION :
Gets the view direction and orientation of the Scene_viewer.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_get_lookat_parameters);
	if (scene_viewer&&eyex&&eyey&&eyez&&lookatx&&lookaty&&lookatz&&upx&&upy&&upz)
	{
		*eyex=scene_viewer->eyex;
		*eyey=scene_viewer->eyey;
		*eyez=scene_viewer->eyez;
		*lookatx=scene_viewer->lookatx;
		*lookaty=scene_viewer->lookaty;
		*lookatz=scene_viewer->lookatz;
		*upx=scene_viewer->upx;
		*upy=scene_viewer->upy;
		*upz=scene_viewer->upz;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_lookat_parameters.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_get_lookat_parameters */

int Scene_viewer_set_lookat_parameters(struct Scene_viewer *scene_viewer,
	double eyex,double eyey,double eyez,
	double lookatx,double lookaty,double lookatz,
	double upx,double upy,double upz)
/*******************************************************************************
LAST MODIFIED : 11 August 1998

DESCRIPTION :
Sets the view direction and orientation of the Scene_viewer.
==============================================================================*/
{
	int return_code;
	double upv[3],viewv[3];

	ENTER(Scene_viewer_set_lookat_parameters);
	if (scene_viewer)
	{
		upv[0]=upx;
		upv[1]=upy;
		upv[2]=upz;
		viewv[0]=lookatx-eyex;
		viewv[1]=lookaty-eyey;
		viewv[2]=lookatz-eyez;
		if ((0.0<normalize3(upv))&&(0.0<normalize3(viewv))&&
			(fabs(dot_product3(upv,viewv))<0.999))
		{
			scene_viewer->eyex=eyex;
			scene_viewer->eyey=eyey;
			scene_viewer->eyez=eyez;
			scene_viewer->lookatx=lookatx;
			scene_viewer->lookaty=lookaty;
			scene_viewer->lookatz=lookatz;
			/* set only unit up-vector */
			scene_viewer->upx=upv[0];
			scene_viewer->upy=upv[1];
			scene_viewer->upz=upv[2];
			scene_viewer->transform_flag = 1;
			Scene_viewer_redraw(scene_viewer);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Scene_viewer_set_lookat_parameters.  "
				"Up and view directions zero or colinear");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_set_lookat_parameters.  Missing scene_viewer");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_set_lookat_parameters */

int Scene_viewer_set_lookat_parameters_non_skew(
	struct Scene_viewer *scene_viewer,double eyex,double eyey,double eyez,
	double lookatx,double lookaty,double lookatz,
	double upx,double upy,double upz)
/*******************************************************************************
LAST MODIFIED : 7 October 1998

DESCRIPTION :
Normal function for controlling Scene_viewer_set_lookat_parameters that ensures
the up vector is orthogonal to the view direction - so projection is not skew.
==============================================================================*/
{
	int return_code;
	double tempv[3],upv[3],viewv[3];

	ENTER(Scene_viewer_set_lookat_parameters_non_skew);
	if (scene_viewer)
	{
		upv[0]=upx;
		upv[1]=upy;
		upv[2]=upz;
		viewv[0]=lookatx-eyex;
		viewv[1]=lookaty-eyey;
		viewv[2]=lookatz-eyez;
		if ((0.0<normalize3(upv))&&(0.0<normalize3(viewv))&&
			(fabs(dot_product3(upv,viewv))<0.999))
		{
			scene_viewer->eyex=eyex;
			scene_viewer->eyey=eyey;
			scene_viewer->eyez=eyez;
			scene_viewer->lookatx=lookatx;
			scene_viewer->lookaty=lookaty;
			scene_viewer->lookatz=lookatz;
			/* set only unit up-vector */
			/* make sure up vector is orthogonal to view direction */
			cross_product3(upv,viewv,tempv);
			cross_product3(viewv,tempv,upv);
			normalize3(upv);
			scene_viewer->upx=upv[0];
			scene_viewer->upy=upv[1];
			scene_viewer->upz=upv[2];
			scene_viewer->transform_flag = 1;
			Scene_viewer_redraw(scene_viewer);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Scene_viewer_set_lookat_parameters_non_skew.  "
				"Up and view directions zero or colinear");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_set_lookat_parameters_non_skew.  Missing scene_viewer");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_set_lookat_parameters_non_skew */

double Scene_viewer_get_stereo_eye_spacing(struct Scene_viewer *scene_viewer)
/*******************************************************************************
LAST MODIFIED : 13 August 2002

DESCRIPTION :
Returns the Scene_viewer stereo_eye_spacing.
==============================================================================*/
{
	double return_spacing;

	ENTER(Scene_viewer_get_stereo_eye_spacing);
	if (scene_viewer)
	{
		return_spacing=scene_viewer->stereo_eye_spacing;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_stereo_eye_spacing.  Invalid argument(s)");
		return_spacing=0;
	}
	LEAVE;

	return (return_spacing);
} /* Scene_viewer_get_stereo_eye_spacing */

int Scene_viewer_set_stereo_eye_spacing(struct Scene_viewer *scene_viewer,
	double stereo_eye_spacing)
/*******************************************************************************
LAST MODIFIED : 13 August 2002

DESCRIPTION :
Sets the Scene_viewer stereo_eye_spacing.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_set_stereo_eye_spacing);
	if (scene_viewer && stereo_eye_spacing)
	{
		scene_viewer->stereo_eye_spacing = stereo_eye_spacing;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_set_stereo_eye_spacing.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_set_stereo_eye_spacing */

int Scene_viewer_get_modelview_matrix(struct Scene_viewer *scene_viewer,
	double modelview_matrix[16])
/*******************************************************************************
LAST MODIFIED : 21 January 1998

DESCRIPTION :
Reads the modelview matrix in effect.  For custom projections this is set by
Scene_viewer_set_modelview_matrix otherwise it is updated each time the window
is rendered.
The format of the matrix is as in Scene_viewer_set_modelview_matrix.
==============================================================================*/
{
	int return_code,i,j;

	ENTER(Scene_viewer_get_modelview_matrix);
	if (scene_viewer&&modelview_matrix)
	{
		for (i=0;i<4;i++)
		{
			for (j=0;j<4;j++)
			{
				modelview_matrix[i*4+j] = 
					scene_viewer->modelview_matrix[j*4+i];					
			}
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_modelview_matrix.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_get_modelview_matrix */

int Scene_viewer_set_modelview_matrix(struct Scene_viewer *scene_viewer,
	double modelview_matrix[16])
/*******************************************************************************
LAST MODIFIED : 21 January 1998

DESCRIPTION :
Sets the modelview matrix used if the SCENE_VIEWER_CUSTOM projection is in
effect. The 4X4 matrix is stored in an array of 16 double values, with values
consecutive across rows, eg:
[x'] = |  m0  m1  m2  m3 |.[x]
[y']   |  m4  m5  m6  m7 | [y]
[z']   |  m8  m9 m10 m11 | [z]
[w']   | m12 m13 m14 m15 | [w]
==============================================================================*/
{
	int return_code,i,j;

	ENTER(Scene_viewer_set_modelview_matrix);
	if (scene_viewer&&modelview_matrix)
	{
		if (SCENE_VIEWER_CUSTOM==scene_viewer->projection_mode)
		{
			for (i=0;i<4;i++)
			{
				for (j=0;j<4;j++)
				{
					scene_viewer->modelview_matrix[j*4+i]=
						modelview_matrix[i*4+j];
				}
			}
			scene_viewer->transform_flag = 1;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,"Scene_viewer_set_modelview_matrix.  "
				"Must be in CUSTOM projection mode");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_set_modelview_matrix.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_set_modelview_matrix */

int Scene_viewer_get_NDC_info(struct Scene_viewer *scene_viewer,
	double *NDC_left,double *NDC_top,double *NDC_width,double *NDC_height)
/*******************************************************************************
LAST MODIFIED : 21 January 1998

DESCRIPTION :
Returns the NDC_info from the scene_viewer - see Scene_viewer_set_NDC_info.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_get_NDC_info);
	if (scene_viewer&&NDC_left&&NDC_top&&NDC_width&&NDC_height)
	{
		*NDC_left=scene_viewer->NDC_left;
		*NDC_top=scene_viewer->NDC_top;
		*NDC_width=scene_viewer->NDC_width;
		*NDC_height=scene_viewer->NDC_height;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_NDC_info.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_get_NDC_info */

int Scene_viewer_set_NDC_info(struct Scene_viewer *scene_viewer,
	double NDC_left,double NDC_top,double NDC_width,double NDC_height)
/*******************************************************************************
LAST MODIFIED : 14 October 1998

DESCRIPTION :
The projection matrix converts the viewing volume into Normalised Device
Coordinates - NDCs - which range from -1.0 to +1.0 in each axis. However, the
shape of this area in the x,y plane of the screen will not be square in the
general case. The NDC_width, NDC_height, NDC_top and NDC_left values describe
the physical dimensions of the NDC cube, used to draw the image on the screen
without distortion. In RELATIVE viewport_mode, only the ratio of NDC_width to
NDC_height is important. In ABSOLUTE viewport_mode, the top and left values
are used to position the intended viewing volume in user coordinates.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_set_NDC_info);
	if (scene_viewer)
	{
		if ((0.0 != NDC_width)&&(0.0 != NDC_height))
		{
			scene_viewer->NDC_left=NDC_left;
			scene_viewer->NDC_top=NDC_top;
			scene_viewer->NDC_width=NDC_width;
			scene_viewer->NDC_height=NDC_height;
			scene_viewer->transform_flag = 1;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,"Scene_viewer_set_NDC_info.  "
				"NDC_width or NDC_height zero or negative");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_set_NDC_info.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_set_NDC_info */

struct Scene *Scene_viewer_get_overlay_scene(struct Scene_viewer *scene_viewer)
/*******************************************************************************
LAST MODIFIED : 18 November 1998

DESCRIPTION :
Returns the overlay_scene used by the Scene_viewer.
The overlay_scene may be NULL, indicating that no overlay is in use.
==============================================================================*/
{
	struct Scene *overlay_scene;

	ENTER(Scene_viewer_get_overlay_scene);
	if (scene_viewer)
	{
		overlay_scene=scene_viewer->overlay_scene;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_overlay_scene.  Invalid argument(s)");
		overlay_scene=(struct Scene *)NULL;
	}
	LEAVE;

	return (overlay_scene);
} /* Scene_viewer_get_overlay_scene */

int Scene_viewer_set_overlay_scene(struct Scene_viewer *scene_viewer,
	struct Scene *overlay_scene)
/*******************************************************************************
LAST MODIFIED : 18 November 1998

DESCRIPTION :
Sets the overlay_scene displayed in addition to the scene as a fixed parallel
projection with coordinates ranging from -1 to +1 in the largest square fitting
in the scene_viewer window, and -1 to +1 from far to near.
The overlay_scene may be NULL, indicating that no overlay is in use.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_set_overlay_scene);
	if (scene_viewer)
	{
		if (overlay_scene)
		{
			ACCESS(Scene)(overlay_scene);
		}
		if (scene_viewer->overlay_scene)
		{
			DEACCESS(Scene)(&(scene_viewer->overlay_scene));
		}
		scene_viewer->overlay_scene=overlay_scene;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_set_overlay_scene.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_set_overlay_scene */

int Scene_viewer_set_overlay_scene_by_name(struct Scene_viewer *scene_viewer,
	const char *name)
/*******************************************************************************
LAST MODIFIED : 19 January 2007

DESCRIPTION :
Sets the Scene_viewer overlay scene from names in the scene manager.
==============================================================================*/
{
	int return_code;
	struct Scene *scene;

	ENTER(Scene_viewer_set_overlay_scene_by_name);
	if (scene_viewer&&name)
	{
		if (scene=FIND_BY_IDENTIFIER_IN_MANAGER(Scene,name)(
			(char *)name, scene_viewer->scene_manager))
		{
			return_code = Scene_viewer_set_overlay_scene(scene_viewer, scene);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Scene_viewer_set_overlay_scene_by_name.  "
				"Unable to find a scene named %s.", name);
			return_code = 0;
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_set_overlay_scene_by_name.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_set_overlay_scene_by_name */

int Scene_viewer_get_projection_mode(struct Scene_viewer *scene_viewer,
	enum Scene_viewer_projection_mode *projection_mode)
/*******************************************************************************
LAST MODIFIED : 17 September 2002

DESCRIPTION :
Returns the projection mode - parallel/perspective - of the Scene_viewer.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_get_projection_mode);
	if (scene_viewer)
	{
		*projection_mode=scene_viewer->projection_mode;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_projection_mode.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_get_projection_mode */

int Scene_viewer_set_projection_mode(struct Scene_viewer *scene_viewer,
	enum Scene_viewer_projection_mode projection_mode)
/*******************************************************************************
LAST MODIFIED : 21 January 1998

DESCRIPTION :
Sets the projection mode - parallel/perspective/custom - of the Scene_viewer.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_set_projection_mode);
	if (scene_viewer&&((SCENE_VIEWER_PARALLEL==projection_mode)||
		(SCENE_VIEWER_PERSPECTIVE==projection_mode)||
		(SCENE_VIEWER_CUSTOM==projection_mode)))
	{
		scene_viewer->projection_mode=projection_mode;
		scene_viewer->transform_flag = 1;
		Scene_viewer_redraw(scene_viewer);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_set_projection_mode.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_set_projection_mode */

int Scene_viewer_get_projection_matrix(struct Scene_viewer *scene_viewer,
	double projection_matrix[16])
/*******************************************************************************
LAST MODIFIED : 25 January 1998

DESCRIPTION :
Reads the projection matrix in effect.  For custom projections this is set by
Scene_viewer_set_modelview_matrix otherwise it is updated each time the window
is rendered.
The format of the matrix is as in Scene_viewer_set_projection_matrix.
==============================================================================*/
{
	int return_code,i,j;

	ENTER(Scene_viewer_get_projection_matrix);
	if (scene_viewer&&projection_matrix)
	{
		for (i=0;i<4;i++)
		{
			for (j=0;j<4;j++)
			{
				projection_matrix[j*4+i]=
					(GLdouble)(scene_viewer->projection_matrix)[i*4+j];
			}
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_projection_matrix.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_get_projection_matrix */

int Scene_viewer_set_projection_matrix(struct Scene_viewer *scene_viewer,
	double projection_matrix[16])
/*******************************************************************************
LAST MODIFIED : 21 January 1998

DESCRIPTION :
Sets the projection matrix used if the SCENE_VIEWER_CUSTOM projection is in
effect. The 4X4 matrix is stored in an array of 16 double values, with values
consecutive across rows, eg:
[x'] = |  m0  m1  m2  m3 |.[x]
[y']   |  m4  m5  m6  m7 | [y]
[z']   |  m8  m9 m10 m11 | [z]
[w']   | m12 m13 m14 m15 | [w]
==============================================================================*/
{
	int return_code,i,j;

	ENTER(Scene_viewer_set_projection_matrix);
	if (scene_viewer&&projection_matrix)
	{
		if (SCENE_VIEWER_CUSTOM==scene_viewer->projection_mode)
		{
			for (i=0;i<4;i++)
			{
				for (j=0;j<4;j++)
				{
					scene_viewer->projection_matrix[i*4+j] = projection_matrix[j*4+i];
				}
			}
			scene_viewer->transform_flag = 1;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,"Scene_viewer_set_projection_matrix.  "
				"Must be in CUSTOM projection mode");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_set_projection_matrix.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_set_projection_matrix */

struct Scene *Scene_viewer_get_scene(struct Scene_viewer *scene_viewer)
/*******************************************************************************
LAST MODIFIED : 18 November 1998

DESCRIPTION :
Returns the Scene_viewer scene.
==============================================================================*/
{
	struct Scene *scene;

	ENTER(Scene_viewer_get_scene);
	if (scene_viewer)
	{
		scene=scene_viewer->scene;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_scene.  Invalid argument(s)");
		scene=(struct Scene *)NULL;
	}
	LEAVE;

	return (scene);
} /* Scene_viewer_get_scene */

int Scene_viewer_set_scene(struct Scene_viewer *scene_viewer,
	struct Scene *scene)
/*******************************************************************************
LAST MODIFIED : 14 February 1998

DESCRIPTION :
Sets the Scene_viewer scene.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_set_scene);
	if (scene_viewer&&scene)
	{
		if (scene != scene_viewer->scene)
		{
			DEACCESS(Scene)(&(scene_viewer->scene));
			scene_viewer->scene=ACCESS(Scene)(scene);
			Scene_viewer_redraw(scene_viewer);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_set_scene.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_set_scene */

int Scene_viewer_set_scene_by_name(struct Scene_viewer *scene_viewer,
	const char *name)
/*******************************************************************************
LAST MODIFIED : 19 January 2007

DESCRIPTION :
Sets the Scene_viewer scene from names in the scene manager.
==============================================================================*/
{
	int return_code;
	struct Scene *scene;

	ENTER(Scene_viewer_set_scene_by_name);
	if (scene_viewer&&name)
	{
		if (scene=FIND_BY_IDENTIFIER_IN_MANAGER(Scene,name)(
			(char *)name, scene_viewer->scene_manager))
		{
			return_code = Scene_viewer_set_scene(scene_viewer, scene);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Scene_viewer_set_scene_by_name.  "
				"Unable to find a scene named %s.", name);
			return_code = 0;
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_set_scene_by_name.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_set_scene_by_name */

int Scene_viewer_get_translation_rate(struct Scene_viewer *scene_viewer,
	double *translation_rate)
/*******************************************************************************
LAST MODIFIED : 4 February 2005

DESCRIPTION :
Gets the scene viewer translation rate.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_get_translation_rate);
	if (scene_viewer)
	{
		*translation_rate = scene_viewer->translate_rate;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Scene_viewer_get_translation_rate.  "
			"Missing scene_viewer parameter.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_get_translation_rate */

int Scene_viewer_set_translation_rate(struct Scene_viewer *scene_viewer,
	double translation_rate)
/*******************************************************************************
LAST MODIFIED : 4 February 2005

DESCRIPTION :
Sets the scene viewer translation rate.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_set_translation_rate);
	if (scene_viewer)
	{
		scene_viewer->translate_rate = translation_rate;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Scene_viewer_set_translation_rate.  "
			"Missing scene_viewer parameter.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_set_translation_rate */

int Scene_viewer_get_tumble_rate(struct Scene_viewer *scene_viewer,
	double *tumble_rate)
/*******************************************************************************
LAST MODIFIED : 4 February 2005

DESCRIPTION :
Gets the scene viewer tumble rate.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_get_translation_rate);
	if (scene_viewer)
	{
		*tumble_rate = scene_viewer->tumble_rate;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Scene_viewer_get_tumble_rate.  "
			"Missing scene_viewer parameter.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_get_tumble_rate */

int Scene_viewer_set_tumble_rate(struct Scene_viewer *scene_viewer,
	double tumble_rate)
/*******************************************************************************
LAST MODIFIED : 4 February 2005

DESCRIPTION :
Sets the scene viewer tumble rate.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_set_tumble_rate);
	if (scene_viewer)
	{
		scene_viewer->tumble_rate = tumble_rate;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Scene_viewer_set_tumble_rate.  "
			"Missing scene_viewer parameter.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_set_tumble_rate */

int Scene_viewer_get_zoom_rate(struct Scene_viewer *scene_viewer,
	double *zoom_rate)
/*******************************************************************************
LAST MODIFIED : 4 February 2005

DESCRIPTION :
Gets the scene viewer zoom rate.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_get_zoom_rate);
	if (scene_viewer)
	{
		*zoom_rate = scene_viewer->zoom_rate;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Scene_viewer_get_zoom_rate.  "
			"Missing scene_viewer parameter.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_get_zoom_rate */

int Scene_viewer_set_zoom_rate(struct Scene_viewer *scene_viewer,
	double zoom_rate)
/*******************************************************************************
LAST MODIFIED : 4 February 2005

DESCRIPTION :
Sets the scene viewer zoom rate.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_set_zoom_rate);
	if (scene_viewer)
	{
		scene_viewer->zoom_rate = zoom_rate;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"Scene_viewer_set_zoom_rate.  "
			"Missing scene_viewer parameter.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_set_tumble_rate */

int Scene_viewer_get_transparency_mode(struct Scene_viewer *scene_viewer,
	enum Scene_viewer_transparency_mode *transparency_mode)
/*******************************************************************************
LAST MODIFIED : 17 September 2002

DESCRIPTION :
See Scene_viewer_set_transparency_mode for explanation.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_get_transparency_mode);
	if (scene_viewer)
	{
		*transparency_mode = scene_viewer->transparency_mode;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_transparency_mode.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_get_transparency_mode */

int Scene_viewer_set_transparency_mode(struct Scene_viewer *scene_viewer,
	enum Scene_viewer_transparency_mode transparency_mode)
/*******************************************************************************
LAST MODIFIED : 23 November 1998

DESCRIPTION :
Sets the transparency_mode of the Scene_viewer. In fast transparency mode,
the scene is drawn as is, with depth buffer writing even for semi-transparent
objects. In slow transparency mode, opaque objects are rendered first, then
semi-transparent objects are rendered without writing the depth buffer. Hence,
you can even see through the first semi-transparent surface drawn.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_set_transparency_mode);
	if (scene_viewer&&((SCENE_VIEWER_FAST_TRANSPARENCY==transparency_mode)||
		(SCENE_VIEWER_SLOW_TRANSPARENCY==transparency_mode)||
		(SCENE_VIEWER_LAYERED_TRANSPARENCY==transparency_mode)||
		(SCENE_VIEWER_ORDER_INDEPENDENT_TRANSPARENCY==transparency_mode)))
	{
		return_code=1;
		if (SCENE_VIEWER_ORDER_INDEPENDENT_TRANSPARENCY==transparency_mode)
		{
			if (!order_independent_capable())
			{
				/* If we can't do it don't change */
				return_code=0;
			}
		}
		if (return_code)
		{
			scene_viewer->transparency_mode=transparency_mode;
			Scene_viewer_redraw(scene_viewer);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_set_transparency_mode.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_set_transparency_mode */

int Scene_viewer_get_transparency_layers(struct Scene_viewer *scene_viewer,
	unsigned int *transparency_layers)
/*******************************************************************************
LAST MODIFIED : 17 September 2002

DESCRIPTION :
See Scene_viewer_set_transparency_layers for explanation.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_get_transparency_layers);
	if (scene_viewer)
	{
		*transparency_layers = scene_viewer->transparency_layers;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_transparency_layers.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_get_transparency_layers */

int Scene_viewer_set_transparency_layers(struct Scene_viewer *scene_viewer,
	unsigned int transparency_layers)
/*******************************************************************************
LAST MODIFIED : 17 September 2002

DESCRIPTION :
When the transparency_mode of the Scene_viewer is layered_transparency then
the z depth is divided into <layers> slices.  From back to front for each layer
the clip planes are set to clip all other layers and then the entire scene is 
drawn.  This is very expensive but can get great results for transparent
surfaces.  Best use of the slices is made if the near and far clip planes are
tight around the objects in the scene.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_set_transparency_mode);
	if (scene_viewer)
	{
		scene_viewer->transparency_layers = transparency_layers;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_set_transparency_mode.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_set_transparency_mode */

int Scene_viewer_get_view_angle(struct Scene_viewer *scene_viewer,
	double *view_angle)
/*******************************************************************************
LAST MODIFIED : 6 April 2001

DESCRIPTION :
Gets the diagonal view angle, in radians, of the <scene_viewer>.
View angle is measured across the normalized device coordinates - NDCs.
For PARALLEL and PERSPECTIVE projection modes only.
==============================================================================*/
{
	double diagonal, eye_distance, view[3];
	int return_code;

	ENTER(Scene_viewer_get_view_angle);
	if (scene_viewer && view_angle)
	{
		diagonal = sqrt((scene_viewer->right - scene_viewer->left)*
			(scene_viewer->right - scene_viewer->left) +
			(scene_viewer->top - scene_viewer->bottom)*
			(scene_viewer->top - scene_viewer->bottom));
		view[0] = scene_viewer->eyex - scene_viewer->lookatx;
		view[1] = scene_viewer->eyey - scene_viewer->lookaty;
		view[2] = scene_viewer->eyez - scene_viewer->lookatz;
		eye_distance = norm3(view);
		*view_angle = 2.0*atan(diagonal/(2.0*eye_distance));
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_view_angle.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_get_view_angle */

int Scene_viewer_get_horizontal_view_angle(struct Scene_viewer *scene_viewer,
	double *horizontal_view_angle)
/*******************************************************************************
LAST MODIFIED : 6 April 2001

DESCRIPTION :
Gets the horizontal view angle, in radians, of the <scene_viewer>.
View angle is measured across the normalized device coordinates - NDCs.
For PARALLEL and PERSPECTIVE projection modes only.
==============================================================================*/
{
	double eye_distance, view[3];
	int return_code;

	ENTER(Scene_viewer_get_horizontal_view_angle);
	if (scene_viewer && horizontal_view_angle && (
		(SCENE_VIEWER_PARALLEL == scene_viewer->projection_mode) ||
		(SCENE_VIEWER_PERSPECTIVE == scene_viewer->projection_mode)))
	{
		view[0] = scene_viewer->eyex - scene_viewer->lookatx;
		view[1] = scene_viewer->eyey - scene_viewer->lookaty;
		view[2] = scene_viewer->eyez - scene_viewer->lookatz;
		eye_distance = norm3(view);
		*horizontal_view_angle =
			2.0*atan((scene_viewer->right - scene_viewer->left)/(2.0*eye_distance));
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_horizontal_view_angle.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_get_horizontal_view_angle */

int Scene_viewer_get_vertical_view_angle(struct Scene_viewer *scene_viewer,
	double *vertical_view_angle)
/*******************************************************************************
LAST MODIFIED : 6 April 2001

DESCRIPTION :
Gets the vertical view angle, in radians, of the <scene_viewer>.
View angle is measured across the normalized device coordinates - NDCs.
For PARALLEL and PERSPECTIVE projection modes only.
==============================================================================*/
{
	double eye_distance, view[3];
	int return_code;

	ENTER(Scene_viewer_get_vertical_view_angle);
	if (scene_viewer && vertical_view_angle && (
		(SCENE_VIEWER_PARALLEL == scene_viewer->projection_mode) ||
		(SCENE_VIEWER_PERSPECTIVE == scene_viewer->projection_mode)))
	{
		view[0] = scene_viewer->eyex - scene_viewer->lookatx;
		view[1] = scene_viewer->eyey - scene_viewer->lookaty;
		view[2] = scene_viewer->eyez - scene_viewer->lookatz;
		eye_distance = norm3(view);
		*vertical_view_angle =
			2.0*atan((scene_viewer->top - scene_viewer->bottom)/(2.0*eye_distance));
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_vertical_view_angle.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_get_vertical_view_angle */

int Scene_viewer_set_view_angle(struct Scene_viewer *scene_viewer,
	double view_angle)
/*******************************************************************************
LAST MODIFIED : 13 October 1998

DESCRIPTION :
Sets the diagonal view angle, in radians, of the <scene_viewer>.
For PARALLEL and PERSPECTIVE projection modes only.
==============================================================================*/
{
	double centre_x,centre_y,diagonal,eye_distance,height,size_ratio,view[3],
		width;
	int return_code;

	ENTER(Scene_viewer_set_view_angle);
	if (scene_viewer&&(0<view_angle)&&(view_angle<PI)&&(
		(SCENE_VIEWER_PARALLEL==scene_viewer->projection_mode)||
		(SCENE_VIEWER_PERSPECTIVE==scene_viewer->projection_mode)))
	{
		width=fabs(scene_viewer->right-scene_viewer->left);
		height=fabs(scene_viewer->top-scene_viewer->bottom);
		centre_x=(scene_viewer->right+scene_viewer->left)/2.0;
		centre_y=(scene_viewer->top+scene_viewer->bottom)/2.0;
		diagonal=sqrt(width*width+height*height);
		view[0]=scene_viewer->eyex - scene_viewer->lookatx;
		view[1]=scene_viewer->eyey - scene_viewer->lookaty;
		view[2]=scene_viewer->eyez - scene_viewer->lookatz;
		eye_distance=normalize3(view);
		size_ratio=tan(view_angle/2.0)*eye_distance/diagonal;
		scene_viewer->left   = centre_x - width *size_ratio;
		scene_viewer->right  = centre_x + width *size_ratio;
		scene_viewer->bottom = centre_y - height*size_ratio;
		scene_viewer->top    = centre_y + height*size_ratio;
		scene_viewer->transform_flag = 1;
		Scene_viewer_redraw(scene_viewer);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_set_view_angle.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_set_view_angle */

int Scene_viewer_set_view_simple(struct Scene_viewer *scene_viewer,
	double centre_x,double centre_y,double centre_z,double radius,
	double view_angle,double clip_distance)
/*******************************************************************************
LAST MODIFIED : 13 October 1998

DESCRIPTION :
Adjusts the viewing parameters of <scene_viewer> so that it is looking at the
<centre_pt> of a sphere of the given <radius> with the given <view_angle>.
The function also adjusts the far clipping plane to be clip_distance behind
the interest point, and the near plane to by the minimum of clip_distance or
eye_distance*0.99 in front of it.
==============================================================================*/
{
	double eyex,eyey,eyez,fact,eye_distance;
	int return_code;

	ENTER(Scene_viewer_set_view_simple);
	if (scene_viewer&&(0.0<radius)&&(1.0 <= view_angle)&&(179.0 >= view_angle))
	{
		/* turn eyex into unit vector in direction back to eye */
		eyex = scene_viewer->eyex-scene_viewer->lookatx;
		eyey = scene_viewer->eyey-scene_viewer->lookaty;
		eyez = scene_viewer->eyez-scene_viewer->lookatz;
		fact = 1.0/sqrt(eyex*eyex+eyey*eyey+eyez*eyez);
		eyex *= fact;
		eyey *= fact;
		eyez *= fact;
		/* look at the centre of the sphere */
		scene_viewer->lookatx=centre_x;
		scene_viewer->lookaty=centre_y;
		scene_viewer->lookatz=centre_z;
		/* shift the eye position to achieve the desired view_angle */
		eye_distance=sqrt(2.0)*radius/tan(view_angle*PI/360.0);
		scene_viewer->eyex = centre_x + eyex*eye_distance;
		scene_viewer->eyey = centre_y + eyey*eye_distance;
		scene_viewer->eyez = centre_z + eyez*eye_distance;
		scene_viewer->left= -radius;
		scene_viewer->right= radius;
		scene_viewer->bottom= -radius;
		scene_viewer->top= radius;
		scene_viewer->far_plane=eye_distance+clip_distance;
		if (clip_distance>=eye_distance)
		{
			scene_viewer->near_plane=0.01*eye_distance;
		}
		else
		{
			scene_viewer->near_plane=eye_distance-clip_distance;
		}
		scene_viewer->transform_flag = 1;
		Scene_viewer_redraw(scene_viewer);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_set_view_simple.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_set_view_simple */

int Scene_viewer_get_viewing_volume(struct Scene_viewer *scene_viewer,
	double *left,double *right,double *bottom,double *top,
	double *near_plane, double *far_plane)
/*******************************************************************************
LAST MODIFIED : 21 November 1997

DESCRIPTION :
Gets the viewing volume of the Scene_viewer.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_get_viewing_volume);
	if (scene_viewer&&left&&right&&bottom&&top&&near_plane&&far_plane)
	{
		*left=scene_viewer->left;
		*right=scene_viewer->right;
		*bottom=scene_viewer->bottom;
		*top=scene_viewer->top;
		*near_plane=scene_viewer->near_plane;
		*far_plane=scene_viewer->far_plane;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_viewing_volume.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_get_viewing_volume */

int Scene_viewer_set_viewing_volume(struct Scene_viewer *scene_viewer,
	double left,double right,double bottom,double top,
	double near_plane,double far_plane)
/*******************************************************************************
LAST MODIFIED : 15 December 1997

DESCRIPTION :
Sets the viewing volume of the Scene_viewer. Unless the viewing volume is the
same shape as the window, taking into account the aspect, the Scene_viewer will
enlarge it to maintain the desired aspect ratio. Hence, the values specified
represent the minimum viewing volume. The left, right, bottom and top values
are at the lookat point, not on the near plane as OpenGL assumes. This gives a
similar sized viewing_volume for both parallel and perspective projections.
The viewing volume can be made unsymmetric to create special effects such as
rendering a higher resolution image in parts.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_set_viewing_volume);
	if (scene_viewer)
	{
		if ((right>left)&&(top>bottom)&&(0<near_plane)&&
		   (near_plane<far_plane))
		{
			scene_viewer->left=left;
			scene_viewer->right=right;
			scene_viewer->bottom=bottom;
			scene_viewer->top=top;
			scene_viewer->near_plane=near_plane;
			scene_viewer->far_plane=far_plane;
			scene_viewer->transform_flag = 1;
			Scene_viewer_redraw(scene_viewer);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Scene_viewer_set_viewing_volume.  Invalid viewing volume");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_set_viewing_volume.  Missing scene_viewer");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_set_viewing_volume */

int Scene_viewer_get_viewport_info(struct Scene_viewer *scene_viewer,
	double *viewport_left,double *viewport_top,double *viewport_pixels_per_unit_x,
	double *viewport_pixels_per_unit_y)
/*******************************************************************************
LAST MODIFIED : 21 January 1998

DESCRIPTION :
See Scene_viewer_set_viewport_info for explanation of the values returned.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_get_viewport_info);
	if (scene_viewer&&viewport_left&&viewport_top&&viewport_pixels_per_unit_x&&
		viewport_pixels_per_unit_y)
	{
		*viewport_left=scene_viewer->user_viewport_left;
		*viewport_top=scene_viewer->user_viewport_top;
		*viewport_pixels_per_unit_x=scene_viewer->user_viewport_pixels_per_unit_x;
		*viewport_pixels_per_unit_y=scene_viewer->user_viewport_pixels_per_unit_y;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_viewport_info.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_get_viewport_info */

int Scene_viewer_set_viewport_info(struct Scene_viewer *scene_viewer,
	double viewport_left,double viewport_top,double viewport_pixels_per_unit_x,
	double viewport_pixels_per_unit_y)
/*******************************************************************************
LAST MODIFIED : 21 January 1998

DESCRIPTION :
The 4 parameters of this routine define the user coordinate system in the
physical viewport. <viewport_left> and <viewport_top> define the exact value
of the user's x and y coordinate at these edges of the viewport.
Note that these are real numbers, and that they refer to the location at the
top-left of the top-left pixel.
???RC Check the above!
The remaining two values specify the scale in pixels per 1 unit of user
coordinates in the x and y direction.
The parameters to the Scene_viewer_set_background_texture_info and
Scene_viewer_set_NDC_info routines are to be given in the user coordinate
system established here. Furthermore, by adjusting the viewport_left and
viewport_top, the image can be translated, while changing the number of
pixels per unit enables zooming to be achieved.
???RC Later: send user coordinates with mouse events.
???RC How to handle y axis pointing down?
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_set_viewport_info);
	if (scene_viewer&&(0.0 != viewport_pixels_per_unit_x)&&
		(0.0 != viewport_pixels_per_unit_y))
	{
		scene_viewer->user_viewport_left=viewport_left;
		scene_viewer->user_viewport_top=viewport_top;
		scene_viewer->user_viewport_pixels_per_unit_x=viewport_pixels_per_unit_x;
		scene_viewer->user_viewport_pixels_per_unit_y=viewport_pixels_per_unit_y;
		scene_viewer->transform_flag = 1;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_set_viewport_info.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_set_viewport_info */

int Scene_viewer_get_antialias_mode(struct Scene_viewer *scene_viewer,
	unsigned int *antialias)
/*******************************************************************************
LAST MODIFIED : 15 October 1998

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_get_antialias_mode);
	if (scene_viewer&&antialias)
	{
		*antialias=scene_viewer->antialias;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_antialias_mode.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_get_antialias_mode */

int Scene_viewer_set_antialias_mode(struct Scene_viewer *scene_viewer,
	unsigned int antialias_mode)
/*******************************************************************************
LAST MODIFIED : 15 October 1998

DESCRIPTION :
Sets the number of jitter samples used to antialias the scene_viewer.gfx
Zero turns antialiasing off.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_set_antialias_mode);
	if (scene_viewer)
	{
		/* Could also check to see if an accumulation buffer is available */
		if ((8==antialias_mode)||(4==antialias_mode)||(2==antialias_mode))
		{
			scene_viewer->antialias=antialias_mode;
			return_code=1;
		}
		else
		{
			if ((1==antialias_mode)||(0==antialias_mode))
			{
				/* Turn antialias off */
				scene_viewer->antialias=0;
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Scene_viewer_set_antialias_mode.  Only 8, 4 or 2 point jitter supported at the moment");
				return_code=0;
			}
		}
		if (return_code)
		{
			Scene_viewer_redraw(scene_viewer);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_set_antialias_mode.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_set_antialias_mode */

int Scene_viewer_get_depth_of_field(struct Scene_viewer *scene_viewer,
	double *depth_of_field, double *focal_depth)
/*******************************************************************************
LAST MODIFIED : 5 December 2006

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_get_depth_of_field);
	if (scene_viewer && depth_of_field && focal_depth)
	{
		*depth_of_field = scene_viewer->depth_of_field;
		*focal_depth = scene_viewer->focal_depth;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_depth_of_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_get_depth_of_field */

int Scene_viewer_set_depth_of_field(struct Scene_viewer *scene_viewer,
	double depth_of_field, double focal_depth)
/*******************************************************************************
LAST MODIFIED : 5 December 2006

DESCRIPTION :
depth of field 0 == infinite.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_set_depth_of_field);
	if (scene_viewer)
	{
		scene_viewer->depth_of_field = depth_of_field;
		scene_viewer->focal_depth = focal_depth;
		return_code = Scene_viewer_redraw(scene_viewer);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_set_depth_of_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_set_depth_of_field */

int Scene_viewer_get_blending_mode(struct Scene_viewer *scene_viewer,
	enum Scene_viewer_blending_mode *blending_mode)
/*******************************************************************************
LAST MODIFIED : 16 April 2003

DESCRIPTION :
See Scene_viewer_set_blending_mode.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_get_blending_mode);
	if (scene_viewer&&blending_mode)
	{
		*blending_mode=scene_viewer->blending_mode;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_blending_mode.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_get_blending_mode */

int Scene_viewer_set_blending_mode(struct Scene_viewer *scene_viewer,
	enum Scene_viewer_blending_mode blending_mode)
/*******************************************************************************
LAST MODIFIED : 16 April 2003

DESCRIPTION :
Sets the blending mode for the scene draw.
SCENE_VIEWER_BLEND_NORMAL is src=GL_SRC_ALPHA and dest=GL_ONE_MINUS_SRC_ALPHA
SCENE_VIEWER_BLEND_TRUE_ALPHA is src=GL_SRC_ALPHA and dest=GL_ONE_MINUS_SRC_ALPHA
  for rgb and src=GL_ONE and dest=GL_ONE_MINUS_SRC_ALPHA for alpha.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_set_blending_mode);
	if (scene_viewer &&
		/* Check that this represents a valid mode */
		(ENUMERATOR_STRING(Scene_viewer_blending_mode)(blending_mode)))
	{
		return_code = 1;
		if (blending_mode == SCENE_VIEWER_BLEND_TRUE_ALPHA)
		{
#if defined (GL_VERSION_1_4)
			if (!Graphics_library_check_extension(GL_VERSION_1_4))
			{
#endif /* defined (GL_VERSION_1_4) */
				display_message(ERROR_MESSAGE, "Scene_viewer_set_blending_mode.  "
					"Blend_true_alpha (glBlendFuncSeparate) is not available on this display.");
				return_code=0;
#if defined (GL_VERSION_1_4)
			}
#endif /* defined (GL_VERSION_1_4) */
		}
		if (return_code)
		{
			scene_viewer->blending_mode = blending_mode;
			Scene_viewer_redraw(scene_viewer);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_set_blending_mode.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_set_blending_mode */

int Scene_viewer_get_perturb_lines(struct Scene_viewer *scene_viewer,
	int *perturb_lines)
/*******************************************************************************
LAST MODIFIED : 13 June 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_get_perturb_lines);
	if (scene_viewer&&perturb_lines)
	{
		*perturb_lines=scene_viewer->perturb_lines;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_perturb_lines.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_get_perturb_lines */

int Scene_viewer_set_perturb_lines(struct Scene_viewer *scene_viewer,
	int perturb_lines)
/*******************************************************************************
LAST MODIFIED : 4 December 2001

DESCRIPTION :
When the line draw mode is turned on (set to one) the lines are raised in the
z direction.  This means that the lines appear solid rather than interfering with a
surface in the same space.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_set_perturb_lines);
	if (scene_viewer)
	{
		if (perturb_lines)
		{
			scene_viewer->perturb_lines=1;
			return_code=1;
		}
		else
		{
			scene_viewer->perturb_lines=0;
			return_code=1;
		}
		Scene_viewer_redraw(scene_viewer);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_set_perturb_lines.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_set_perturb_lines */

enum Scene_viewer_viewport_mode Scene_viewer_get_viewport_mode(
	struct Scene_viewer *scene_viewer)
/*******************************************************************************
LAST MODIFIED : 14 October 1998

DESCRIPTION :
See Scene_viewer_set_viewport_mode for explanation.
==============================================================================*/
{
	enum Scene_viewer_viewport_mode viewport_mode;

	ENTER(Scene_viewer_get_viewport_mode);
	if (scene_viewer)
	{
		viewport_mode=scene_viewer->viewport_mode;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_viewport_mode.  Invalid argument(s)");
		/* return any valid value */
		viewport_mode=SCENE_VIEWER_RELATIVE_VIEWPORT;
	}
	LEAVE;

	return (viewport_mode);
} /* Scene_viewer_get_viewport_mode */

int Scene_viewer_set_viewport_mode(struct Scene_viewer *scene_viewer,
	enum Scene_viewer_viewport_mode viewport_mode)
/*******************************************************************************
LAST MODIFIED : 04 February 2005

DESCRIPTION :
Sets the viewport_mode of the Scene_viewer. A relative viewport scales the NDC
viewing volume to the maximum size that can fit in the window. An absolute
viewport uses the NDC_information to map the NDC viewing volume onto the
viewport coordinates, which are specified relative to the window.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_set_viewport_mode);
	if (scene_viewer&&((SCENE_VIEWER_RELATIVE_VIEWPORT==viewport_mode)||
		(SCENE_VIEWER_ABSOLUTE_VIEWPORT==viewport_mode) ||
		(SCENE_VIEWER_DISTORTING_RELATIVE_VIEWPORT==viewport_mode)))
	{
		scene_viewer->viewport_mode=viewport_mode;
		scene_viewer->transform_flag = 1;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_set_viewport_mode.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_set_viewport_mode */

int Scene_viewer_get_viewport_size(struct Scene_viewer *scene_viewer,
	int *width, int *height)
/*******************************************************************************
LAST MODIFIED : 2 July 2002

DESCRIPTION :
Returns the width and height of the Scene_viewers drawing area.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_get_viewport_size);
	if (scene_viewer&&width&&height)
	{
		*width = Graphics_buffer_get_width(scene_viewer->graphics_buffer);
		*height = Graphics_buffer_get_height(scene_viewer->graphics_buffer);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_viewport_size.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_get_viewport_size */

int Scene_viewer_set_viewport_size(struct Scene_viewer *scene_viewer,
	int width, int height)
/*******************************************************************************
LAST MODIFIED : 21 January 1998

DESCRIPTION :
Sets the width and height of the Scene_viewers drawing area.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_set_viewport_size);
	if (scene_viewer&&(0<width)&&(0<height))
	{
		Graphics_buffer_set_width(scene_viewer->graphics_buffer, width);
		Graphics_buffer_set_height(scene_viewer->graphics_buffer, height);
		scene_viewer->transform_flag = 1;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_set_viewport_size.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_set_viewport_size */

int Scene_viewer_get_opengl_information(struct Scene_viewer *scene_viewer,
	char **opengl_version, char **opengl_vendor, char **opengl_extensions,
	int *visual_id, int *colour_buffer_depth, int *depth_buffer_depth,
	int *accumulation_buffer_depth)
/*******************************************************************************
LAST MODIFIED : 19 September 2002

DESCRIPTION :
Returns the OpenGL state information.  The <opengl_version>, <opengl_vendor> and
<opengl_extensions> strings are static pointers supplied from the driver and
so should not be modified or deallocated.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_get_opengl_information);
	if (scene_viewer)
	{
		Graphics_buffer_make_current(scene_viewer->graphics_buffer);
		*opengl_version=(char *)glGetString(GL_VERSION);
		*opengl_vendor=(char *)glGetString(GL_VENDOR);
		*opengl_extensions=(char *)glGetString(GL_EXTENSIONS);

#if defined (DEBUG)
		printf("%s\n", *opengl_extensions);
#endif /* defined (DEBUG) */

		Graphics_buffer_get_visual_id(scene_viewer->graphics_buffer, visual_id);
		Graphics_buffer_get_colour_buffer_depth(scene_viewer->graphics_buffer,
			colour_buffer_depth);
		Graphics_buffer_get_depth_buffer_depth(scene_viewer->graphics_buffer,
			depth_buffer_depth);
		Graphics_buffer_get_accumulation_buffer_depth(scene_viewer->graphics_buffer,
			accumulation_buffer_depth);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_viewport_info.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_get_viewport_info */

int Scene_viewer_get_window_projection_matrix(struct Scene_viewer *scene_viewer,
	double window_projection_matrix[16])
/*******************************************************************************
LAST MODIFIED : 7 April 2003

DESCRIPTION :
Returns the actual projection matrix applied to fill the window.
==============================================================================*/
{
	int return_code,i,j;

	ENTER(Scene_viewer_get_window_projection_matrix);
	if (scene_viewer&&window_projection_matrix)
	{
		for (i=0;i<4;i++)
		{
			for (j=0;j<4;j++)
			{
				window_projection_matrix[j*4+i]=
					(GLdouble)(scene_viewer->window_projection_matrix)[i*4+j];
			}
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_window_projection_matrix.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_get_window_projection_matrix */

int Scene_viewer_rotate_about_lookat_point(struct Scene_viewer *scene_viewer,
	double a[3],double angle)
/*******************************************************************************
LAST MODIFIED : 26 November 1997

DESCRIPTION :
Rotates the eye <angle> radians about unit vector axis <a> stemming from the
<scene_viewer> lookat point. Up vector is also reoriented to remain normal to
the eye-to-lookat direction. Rotation is in a clockwise sense. Also, if <a> is
not already a unit vector, it will be made one by this function.
==============================================================================*/
{
	double b[3],c[3],v[3],rel_eyex,rel_eyey,rel_eyez,rel_eyea,rel_eyeb,rel_eyec,
		upa,upb,upc,new_b[3],new_c[3],cos_angle,sin_angle;
	int return_code;

	ENTER(Scene_viewer_rotate_about_lookat_point);
	if (scene_viewer&&a&&(0<normalize3(a)))
	{
		/* get coordinate system moving with rotation, consisting of the axis a */
		/* and two othogonal vectors b and c in the plane normal to a. */
		/* v = vector towards viewer */
		v[0]=rel_eyex=scene_viewer->eyex-scene_viewer->lookatx;
		v[1]=rel_eyey=scene_viewer->eyey-scene_viewer->lookaty;
		v[2]=rel_eyez=scene_viewer->eyez-scene_viewer->lookatz;
		normalize3(v);
		/* check v is not too closely in line with a */
		if (0.8 < fabs(v[0]*a[0]+v[1]*a[1]+v[2]*a[2]))
		{
			/* use up-vector instead */
			v[0]=scene_viewer->upx;
			v[1]=scene_viewer->upy;
			v[2]=scene_viewer->upz;
		}
		/* b = axis (x) a, a vector in plane of rotation */
		b[0]=a[1]*v[2]-a[2]*v[1];
		b[1]=a[2]*v[0]-a[0]*v[2];
		b[2]=a[0]*v[1]-a[1]*v[0];
		normalize3(b);
		/* c = b (x) axis, another unit vector in plane of rotation */
		c[0]=a[1]*b[2]-a[2]*b[1];
		c[1]=a[2]*b[0]-a[0]*b[2];
		c[2]=a[0]*b[1]-a[1]*b[0];
		/* define eye position and up vector relative to a, b and c */
		rel_eyea=a[0]*rel_eyex+a[1]*rel_eyey+a[2]*rel_eyez;
		rel_eyeb=b[0]*rel_eyex+b[1]*rel_eyey+b[2]*rel_eyez;
		rel_eyec=c[0]*rel_eyex+c[1]*rel_eyey+c[2]*rel_eyez;
		upa=a[0]*scene_viewer->upx+a[1]*scene_viewer->upy+a[2]*scene_viewer->upz;
		upb=b[0]*scene_viewer->upx+b[1]*scene_viewer->upy+b[2]*scene_viewer->upz;
		upc=c[0]*scene_viewer->upx+c[1]*scene_viewer->upy+c[2]*scene_viewer->upz;
		/* get new b and c from clockwise rotation by <angle> radians about a */
		cos_angle=cos(angle);
		sin_angle=sin(angle);
		new_b[0]=cos_angle*b[0]+sin_angle*c[0];
		new_b[1]=cos_angle*b[1]+sin_angle*c[1];
		new_b[2]=cos_angle*b[2]+sin_angle*c[2];
		new_c[0]=cos_angle*c[0]-sin_angle*b[0];
		new_c[1]=cos_angle*c[1]-sin_angle*b[1];
		new_c[2]=cos_angle*c[2]-sin_angle*b[2];
		/* get eye position and up vector back in world coordinates */
		scene_viewer->eyex=scene_viewer->lookatx+
			a[0]*rel_eyea+new_b[0]*rel_eyeb+new_c[0]*rel_eyec;
		scene_viewer->eyey=scene_viewer->lookaty+
			a[1]*rel_eyea+new_b[1]*rel_eyeb+new_c[1]*rel_eyec;
		scene_viewer->eyez=scene_viewer->lookatz+
			a[2]*rel_eyea+new_b[2]*rel_eyeb+new_c[2]*rel_eyec;
		scene_viewer->upx=a[0]*upa+new_b[0]*upb+new_c[0]*upc;
		scene_viewer->upy=a[1]*upa+new_b[1]*upb+new_c[1]*upc;
		scene_viewer->upz=a[2]*upa+new_b[2]*upb+new_c[2]*upc;
		scene_viewer->transform_flag = 1;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_rotate_about_lookat_point.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_rotate_about_lookat_point */

int for_each_Light_in_Scene_viewer(struct Scene_viewer *scene_viewer,
	LIST_ITERATOR_FUNCTION(Light) *iterator_function,void *user_data)
/*******************************************************************************
LAST MODIFIED : 18 December 1997

DESCRIPTION :
Allows clients of the <scene_viewer> to perform functions with the lights in it.
The most common task will to list the lights in the scene with show_Light.
==============================================================================*/
{
	int return_code;

	ENTER(for_each_Light_in_Scene_viewer);
	if (scene_viewer)
	{
		return_code=FOR_EACH_OBJECT_IN_LIST(Light)(iterator_function,user_data,
			scene_viewer->list_of_lights);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"for_each_Light_in_Scene_viewer.  Missing scene_viewer");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* for_each_Light_in_Scene_viewer */

int Scene_viewer_redraw(struct Scene_viewer *scene_viewer)
/*******************************************************************************
LAST MODIFIED : 14 July 2000

DESCRIPTION :
Requests a full redraw in idle time.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_redraw);
	if (scene_viewer)
	{
		scene_viewer->fast_changing=0;
		Scene_viewer_redraw_in_idle_time(scene_viewer);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_redraw.  Missing scene_viewer");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_redraw */

int Scene_viewer_redraw_now(struct Scene_viewer *scene_viewer)
/*******************************************************************************
LAST MODIFIED : 29 September 2000

DESCRIPTION :
Requests a full redraw immediately.
==============================================================================*/
{
	int return_code;
	struct Event_dispatcher *event_dispatcher;

	ENTER(Scene_viewer_redraw_now);
	if (scene_viewer)
	{
		/* remove idle update workproc if pending */
		event_dispatcher = User_interface_get_event_dispatcher(
			scene_viewer->user_interface);
		if (scene_viewer->idle_update_callback_id)
		{
			Event_dispatcher_remove_idle_callback(
				event_dispatcher, scene_viewer->idle_update_callback_id);
			scene_viewer->idle_update_callback_id = (struct Event_dispatcher_idle_callback *)NULL;
		}
		if (scene_viewer->tumble_active)
		{
			Scene_viewer_automatic_tumble(scene_viewer);
			if(!scene_viewer->idle_update_callback_id)
			{
				scene_viewer->idle_update_callback_id = Event_dispatcher_add_idle_callback(
					event_dispatcher, Scene_viewer_idle_update_callback, (void *)scene_viewer,
					EVENT_DISPATCHER_IDLE_UPDATE_SCENE_VIEWER_PRIORITY);
			}
		}
		Graphics_buffer_make_current(scene_viewer->graphics_buffer);
		/* always do a full redraw */
		scene_viewer->fast_changing=0;
		return_code=Scene_viewer_render_scene(scene_viewer);
		if (scene_viewer->swap_buffers)
		{
			Graphics_buffer_swap_buffers(scene_viewer->graphics_buffer);
		}
		CMISS_CALLBACK_LIST_CALL(Scene_viewer_callback)(
			scene_viewer->repaint_required_callback_list, scene_viewer, NULL);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_redraw_now.  Missing scene_viewer");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_redraw_now */

int Scene_viewer_redraw_now_with_overrides(struct Scene_viewer *scene_viewer,
	int antialias, int transparency_layers)
/*******************************************************************************
LAST MODIFIED : 23 September 2002

DESCRIPTION :
Requests a full redraw immediately.  If non_zero then the supplied <antialias>
and <transparency_layers> are used for just this render.
==============================================================================*/
{
	int return_code;
	struct Event_dispatcher *event_dispatcher;

	ENTER(Scene_viewer_redraw_now);
	if (scene_viewer)
	{
		/* remove idle update workproc if pending */
		event_dispatcher = User_interface_get_event_dispatcher(
			scene_viewer->user_interface);
		if (scene_viewer->idle_update_callback_id)
		{
			Event_dispatcher_remove_idle_callback(
				event_dispatcher, scene_viewer->idle_update_callback_id);
			scene_viewer->idle_update_callback_id = (struct Event_dispatcher_idle_callback *)NULL;
		}
		if (scene_viewer->tumble_active)
		{
			Scene_viewer_automatic_tumble(scene_viewer);
			if(!scene_viewer->idle_update_callback_id)
			{
				scene_viewer->idle_update_callback_id = Event_dispatcher_add_idle_callback(
					event_dispatcher, Scene_viewer_idle_update_callback, (void *)scene_viewer,
					EVENT_DISPATCHER_IDLE_UPDATE_SCENE_VIEWER_PRIORITY);
			}
		}
		Graphics_buffer_make_current(scene_viewer->graphics_buffer);
		/* always do a full redraw */
		scene_viewer->fast_changing=0;
		return_code=Scene_viewer_render_scene_in_viewport_with_overrides(
			scene_viewer, /*left*/0, /*bottom*/0, /*right*/0, /*top*/0,
			antialias, transparency_layers, /*drawing_offscreen*/0);
		if (scene_viewer->swap_buffers)
		{
			Graphics_buffer_swap_buffers(scene_viewer->graphics_buffer);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_redraw_now.  Missing scene_viewer");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_redraw_now */

int Scene_viewer_redraw_now_without_swapbuffers(
	struct Scene_viewer *scene_viewer)
/*******************************************************************************
LAST MODIFIED : 25 July 1998

DESCRIPTION :
Forces a redraw of the given scene viewer to take place immediately but does
not swap the back and front buffers so that utilities such as the movie
extensions can get the updated frame from the backbuffer.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_redraw_now_without_swapbuffers);
	if (scene_viewer)
	{
		Graphics_buffer_make_current(scene_viewer->graphics_buffer);
		/* always do a full redraw */
		scene_viewer->fast_changing=0;
		return_code=Scene_viewer_render_scene(scene_viewer);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_redraw_now_without_swapbuffers.  Missing scene_viewer");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_redraw_now_without_swapbuffers */

int Scene_viewer_get_frame_pixels(struct Scene_viewer *scene_viewer,
	enum Texture_storage_type storage, int *width, int *height,
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
	int number_of_components, return_code;
#if defined (MOTIF) || defined (GTK_USER_INTERFACE)
	struct Graphics_buffer *offscreen_buffer;
#endif /* defined (MOTIF) || defined (GTK_USER_INTERFACE) */

	ENTER(Scene_viewer_get_frame_pixels);

#if ! defined (MOTIF) && ! defined (GTK_USER_INTERFACE)
	USE_PARAMETER(force_onscreen);
#endif /* ! defined (MOTIF) && ! defined (GTK_USER_INTERFACE) */
	if (scene_viewer && width && height)
	{
#if defined (MOTIF) || defined (GTK_USER_INTERFACE)
		if ((!*width) || (!*height))
		{
			/* Only use the scene viewer size if either dimension is zero */
			*width = Graphics_buffer_get_width(scene_viewer->graphics_buffer);
			*height = Graphics_buffer_get_height(scene_viewer->graphics_buffer);
		}
		/* If working offscreen try and allocate as large an area as possible */
		if (!force_onscreen && (offscreen_buffer = create_Graphics_buffer_offscreen_from_buffer(
			*width, *height, Scene_viewer_get_graphics_buffer(
			scene_viewer))))
		{
			Graphics_buffer_make_current(offscreen_buffer);
			Scene_viewer_render_scene_in_viewport_with_overrides(scene_viewer,
				/*left*/0, /*bottom*/0, /*right*/*width, /*top*/*height,
				preferred_antialias, preferred_transparency_layers,
				/*drawing_offscreen*/1);
			number_of_components =
				Texture_storage_type_get_number_of_components(storage);
			if (ALLOCATE(*frame_data, unsigned char,
				number_of_components * (*width) * (*height)))
			{
				if (!(return_code=Graphics_library_read_pixels(*frame_data, *width,
					*height, storage, /*front_buffer*/0)))
				{
					DEALLOCATE(*frame_data);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Scene_viewer_get_frame_pixels.  Unable to allocate pixels");
				return_code=0;
			}
			DESTROY(Graphics_buffer)(&offscreen_buffer);
		}
		else
		{
#endif /* defined (MOTIF) || defined (GTK_USER_INTERFACE) */
			/* Always use the window size if grabbing from screen */
			*width = Graphics_buffer_get_width(scene_viewer->graphics_buffer);
			*height = Graphics_buffer_get_height(scene_viewer->graphics_buffer);
			Scene_viewer_render_scene_in_viewport_with_overrides(scene_viewer,
				/*left*/0, /*bottom*/0, /*right*/*width, /*top*/*height,
				preferred_antialias, preferred_transparency_layers,
				/*drawing_offscreen*/0);
			number_of_components =
				Texture_storage_type_get_number_of_components(storage);
			if (ALLOCATE(*frame_data, unsigned char,
				number_of_components * (*width) * (*height)))
			{
				if (!(return_code=Graphics_library_read_pixels(*frame_data, *width,
					*height, storage, /*front_buffer*/0)))
				{
					DEALLOCATE(*frame_data);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Scene_viewer_get_frame_pixels.  Unable to allocate pixels");
				return_code=0;
			}
#if defined (MOTIF) || defined (GTK_USER_INTERFACE)
		}
#endif /* defined (MOTIF) || defined (GTK_USER_INTERFACE) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_frame_pixels.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return return_code;
} /* Scene_viewer_get_frame_pixels */

struct Cmgui_image *Scene_viewer_get_image(struct Scene_viewer *scene_viewer,
	int force_onscreen, int preferred_width, int preferred_height,
	int preferred_antialias, int preferred_transparency_layers,
	enum Texture_storage_type storage)
/*******************************************************************************
LAST MODIFIED : 18 September 2002

DESCRIPTION :
Creates and returns a Cmgui_image from the image in <scene_viewer>, usually for
writing. The image has a single depth plane and is in RGBA format.
Up to the calling function to DESTROY the returned Cmgui_image.
If <preferred_width>, <preferred_height>, <preferred_antialias> or
<preferred_transparency_layers> are non zero then they attempt to override the
default values for just this call.
If <force_onscreen> is set then the pixels are grabbed directly from the window
display and the <preferred_width> and <preferred_height> are ignored.
Currently limited to 1 byte per component -- may want to improve for HPC.
==============================================================================*/
{
	unsigned char *frame_data;
	int bytes_per_pixel, height, number_of_bytes_per_component,
		number_of_components, width;
	struct Cmgui_image *cmgui_image;

	ENTER(Scene_viewer_get_image);
	cmgui_image = (struct Cmgui_image *)NULL;
	if (scene_viewer)
	{
		number_of_components =
			Texture_storage_type_get_number_of_components(storage);
		number_of_bytes_per_component = 1;
		bytes_per_pixel = number_of_components*number_of_bytes_per_component;
		width = preferred_width;
		height = preferred_height;
		if (Scene_viewer_get_frame_pixels(scene_viewer, storage,
			&width, &height, preferred_antialias, preferred_transparency_layers,
			&frame_data, force_onscreen))
		{
			cmgui_image = Cmgui_image_constitute(width, height,
				number_of_components, number_of_bytes_per_component,
				width*bytes_per_pixel, frame_data);
			if (!cmgui_image)
			{
				display_message(ERROR_MESSAGE,
					"Scene_viewer_get_image.  Could not constitute image");
			}
			DEALLOCATE(frame_data);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Scene_viewer_get_image.  Could not get frame pixels");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_image.  Missing window");
	}
	LEAVE;

	return (cmgui_image);
} /* Scene_viewer_get_image */

int Scene_viewer_set_update_pixel_image(struct Scene_viewer *scene_viewer)
/*******************************************************************************
LAST MODIFIED : 18 May 1998

DESCRIPTION :
Sets a flag so that the redraw will necessarily fully render the scene in
pixel buffer mode
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_set_update_pixel_image);
	if (scene_viewer)
	{
		scene_viewer->buffering_mode = SCENE_VIEWER_PIXEL_BUFFER;
		scene_viewer->update_pixel_image=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_set_update_pixel_image.  Missing scene_viewer");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_set_update_pixel_image */

int Scene_viewer_get_pixel_image(struct Scene_viewer *scene_viewer,
	int *width, int *height, void **data)
/*******************************************************************************
LAST MODIFIED : 18 May 1998

DESCRIPTION :
Returns RGB data grabbed from the scene viewer when buffer type
is SCENE_VIEWER_PIXEL_BUFFER.  The data is handed directly so it should
be used immediately and not DEALLOCATED.  It is expected to be byte sized
values for each of Red Green and Blue only.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_set_update_pixel_image);
	if (scene_viewer&&(SCENE_VIEWER_PIXEL_BUFFER==scene_viewer->buffering_mode))
	{
		*width=scene_viewer->pixel_width;
		*height=scene_viewer->pixel_height;
		*data=scene_viewer->pixel_data;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_pixel_image.  Missing scene_viewer");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_get_pixel_image */

int Scene_viewer_set_pixel_image(struct Scene_viewer *scene_viewer,int width,
	int height,void *data)
/*******************************************************************************
LAST MODIFIED : 18 May 1998

DESCRIPTION :
Sets the RGB data in a scene viewer when buffer type
is SCENE_VIEWER_PIXEL_BUFFER.  The data is copied into the internal buffer.
It is expected to be byte sized values for each of Red Green and Blue only.
==============================================================================*/
{
	int return_code;
	void *new_data;

	ENTER(Scene_viewer_set_update_pixel_image);
	if (scene_viewer&&height&&width&&data)
	{
		scene_viewer->buffering_mode = SCENE_VIEWER_PIXEL_BUFFER;
		if (REALLOCATE(new_data,scene_viewer->pixel_data,char,3*width*height))
		{
			scene_viewer->pixel_width=width;
			scene_viewer->pixel_height=height;
			memcpy(new_data,data,3*width*height);
			scene_viewer->pixel_data=new_data;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Scene_viewer_set_pixel_image. Unable to reallocate pixel image");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_set_pixel_image.  Missing scene_viewer");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_set_pixel_image */

int Scene_viewer_view_all(struct Scene_viewer *scene_viewer)
/*******************************************************************************
LAST MODIFIED : 16 October 2001

DESCRIPTION :
Finds the x, y and z ranges from the scene and sets the view parameters so
that everything can be seen, and with window's std_view_angle. Also adjusts
near and far clipping planes; if specific values are required, should follow
with commands for setting these.
==============================================================================*/
{
	double centre_x, centre_y, centre_z, clip_factor, radius,
		size_x, size_y, size_z, width_factor;
	int return_code;

	ENTER(Scene_viewer_view_all);
	if (scene_viewer)
	{	
		Scene_get_graphics_range(Scene_viewer_get_scene(scene_viewer),
			&centre_x,&centre_y,&centre_z,&size_x,&size_y,&size_z);
		radius = 0.5*sqrt(size_x*size_x + size_y*size_y + size_z*size_z);
		if (0 == radius)
		{
			radius = 0.5*(scene_viewer->right - scene_viewer->left);
		}
		else
		{		
			/* enlarge radius to keep image within edge of window */
			/*???RC width_factor should be read in from defaults file */
			width_factor = 1.05;
			radius *= width_factor;
		}
		/*???RC clip_factor should be read in from defaults file: */
		clip_factor = 10.0;		
		return_code = Scene_viewer_set_view_simple(scene_viewer, centre_x, centre_y,
			centre_z, radius, 40, clip_factor*radius);		
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_view_all.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_view_all */

int Scene_viewer_viewport_zoom(struct Scene_viewer *scene_viewer,
	double zoom_ratio)
/*******************************************************************************
LAST MODIFIED : 16 February 1998

DESCRIPTION :
Scales of the absolute image while keeping the same centre point.
==============================================================================*/
{
	int height,width; /* X widget dimensions */
	int return_code;

	ENTER(Scene_viewer_viewport_zoom);
	if (scene_viewer&&(0.0<zoom_ratio))
	{
		scene_viewer->user_viewport_pixels_per_unit_x *= zoom_ratio;
		scene_viewer->user_viewport_pixels_per_unit_y *= zoom_ratio;
		/* adjust top,left so that zoom comes from centre of viewport */
		width = Graphics_buffer_get_width(scene_viewer->graphics_buffer);
		height = Graphics_buffer_get_height(scene_viewer->graphics_buffer);
		scene_viewer->user_viewport_left += 0.5*(zoom_ratio-1.0)*
			(width/scene_viewer->user_viewport_pixels_per_unit_x);
		scene_viewer->user_viewport_top -= 0.5*(zoom_ratio-1.0)*
			(height/scene_viewer->user_viewport_pixels_per_unit_y);
		scene_viewer->transform_flag = 1;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_viewport_zoom.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_viewport_zoom */

struct Interactive_tool *Scene_viewer_get_interactive_tool(
	struct Scene_viewer *scene_viewer)
/*******************************************************************************
LAST MODIFIED : 11 April 2000

DESCRIPTION :
Returns the interactive_tool used by the Scene_viewer.
The interactive_tool may be NULL, indicating that no overlay is in use.
==============================================================================*/
{
	struct Interactive_tool *interactive_tool;

	ENTER(Scene_viewer_get_interactive_tool);
	if (scene_viewer)
	{
		interactive_tool=scene_viewer->interactive_tool;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_interactive_tool.  Invalid argument(s)");
		interactive_tool=(struct Interactive_tool *)NULL;
	}
	LEAVE;

	return (interactive_tool);
} /* Scene_viewer_get_interactive_tool */

int Scene_viewer_set_interactive_tool(struct Scene_viewer *scene_viewer,
	struct Interactive_tool *interactive_tool)
/*******************************************************************************
LAST MODIFIED : 25 February 2008

DESCRIPTION :
Sets the interactive tool that will receive input if the Scene_viewer is in
SCENE_VIEWER_SELECT mode. A NULL value indicates no tool.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_set_interactive_tool);
	if (scene_viewer)
	{
		if (scene_viewer->interactive_tool &&
			(scene_viewer->interactive_tool != interactive_tool))
		{
			Interactive_tool_reset(scene_viewer->interactive_tool);
		}
		scene_viewer->interactive_tool=interactive_tool;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_set_interactive_tool.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_set_interactive_tool */

int Scene_viewer_set_interactive_tool_by_name(
	struct Scene_viewer *scene_viewer, const char *tool_name)
/*******************************************************************************
LAST MODIFIED : 19 January 2007

DESCRIPTION :
==============================================================================*/
{
	struct Interactive_tool *interactive_tool;
	int return_code;

	ENTER(Cmiss_scene_viewer_set_interactive_tool_by_name);
	if (scene_viewer && scene_viewer->interactive_tool_manager)
	{
		if (interactive_tool=
			FIND_BY_IDENTIFIER_IN_MANAGER(Interactive_tool,name)(
				(char *)tool_name,scene_viewer->interactive_tool_manager))
		{
			if (Interactive_tool_is_Transform_tool(interactive_tool))
			{
				Scene_viewer_set_input_mode(scene_viewer,SCENE_VIEWER_TRANSFORM);
			}
			else
			{
				Scene_viewer_set_input_mode(scene_viewer,SCENE_VIEWER_SELECT);
			}
			return_code = Scene_viewer_set_interactive_tool(scene_viewer,
				interactive_tool);
		}
		else
		{
			display_message(ERROR_MESSAGE,"Cmiss_scene_viewer_set_interactive_tool_by_name.  "
				"Unable to find an interactive tool named %s.", tool_name);
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_scene_viewer_set_interactive_tool_by_name.  "
			"The Cmiss_scene_viewer data must be initialised before using "
			"the scene_viewer api.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_scene_viewer_set_interactive_tool_by_name */

int Scene_viewer_add_input_callback(struct Scene_viewer *scene_viewer,
	CMISS_CALLBACK_FUNCTION(Scene_viewer_input_callback) *function,
	void *user_data, int add_first)
/*******************************************************************************
LAST MODIFIED : 11 September 2007

DESCRIPTION :
Adds callback that will be activated each time input is received by the 
scene_viewer.
If <add_first> is true (non zero) then this callback will be added to the 
front of the list.
When a callback event is generated the list is processed as long as each
callback function returns true, so to stop processing and not call any more
of the callbacks registered after your handler then return false.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_add_input_callback);
	if (scene_viewer&&function)
	{
		if (add_first)
		{
			return_code = 
				CMISS_CALLBACK_LIST_ADD_CALLBACK_TO_FRONT(Scene_viewer_input_callback)(
				scene_viewer->input_callback_list,function,user_data);
		}
		else
		{
			return_code = 
				CMISS_CALLBACK_LIST_ADD_CALLBACK(Scene_viewer_input_callback)(
				scene_viewer->input_callback_list,function,user_data);
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"Scene_viewer_add_input_callback.  Could not add callback");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_add_input_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_add_input_callback */

int Scene_viewer_remove_input_callback(struct Scene_viewer *scene_viewer,
	CMISS_CALLBACK_FUNCTION(Scene_viewer_input_callback) *function,
	void *user_data)
/*******************************************************************************
LAST MODIFIED : 2 July 2002

DESCRIPTION :
Removes the callback calling <function> with <user_data> from
<scene_viewer>.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_remove_input_callback);
	if (scene_viewer&&function)
	{
		if (CMISS_CALLBACK_LIST_REMOVE_CALLBACK(Scene_viewer_input_callback)(
			scene_viewer->input_callback_list,function,user_data))
		{
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Scene_viewer_remove_input_callback.  Could not remove callback");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_remove_input_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_remove_input_callback */

int Scene_viewer_add_sync_callback(struct Scene_viewer *scene_viewer,
	CMISS_CALLBACK_FUNCTION(Scene_viewer_callback) *function,void *user_data)
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_add_sync_callback);
	if (scene_viewer&&function)
	{
		if (CMISS_CALLBACK_LIST_ADD_CALLBACK(Scene_viewer_callback)(
			scene_viewer->sync_callback_list,function,user_data))
		{
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Scene_viewer_add_sync_callback.  Could not add callback");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_add_sync_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_add_sync_callback */

int Scene_viewer_remove_sync_callback(struct Scene_viewer *scene_viewer,
	CMISS_CALLBACK_FUNCTION(Scene_viewer_callback) *function,void *user_data)
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
Removes the callback calling <function> with <user_data> from
<scene_viewer>.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_remove_sync_callback);
	if (scene_viewer&&function)
	{
		if (CMISS_CALLBACK_LIST_REMOVE_CALLBACK(Scene_viewer_callback)(
			scene_viewer->sync_callback_list,function,user_data))
		{
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Scene_viewer_remove_sync_callback.  Could not remove callback");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_remove_sync_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_remove_sync_callback */

int Scene_viewer_add_transform_callback(struct Scene_viewer *scene_viewer,
	CMISS_CALLBACK_FUNCTION(Scene_viewer_callback) *function,void *user_data)
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_add_transform_callback);
	if (scene_viewer&&function)
	{
		if (CMISS_CALLBACK_LIST_ADD_CALLBACK(Scene_viewer_callback)(
			scene_viewer->transform_callback_list,function,user_data))
		{
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Scene_viewer_add_transform_callback.  Could not add callback");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_add_transform_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_add_transform_callback */

int Scene_viewer_remove_transform_callback(struct Scene_viewer *scene_viewer,
	CMISS_CALLBACK_FUNCTION(Scene_viewer_callback) *function,void *user_data)
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
Removes the callback calling <function> with <user_data> from
<scene_viewer>.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_remove_transform_callback);
	if (scene_viewer&&function)
	{
		if (CMISS_CALLBACK_LIST_REMOVE_CALLBACK(Scene_viewer_callback)(
			scene_viewer->transform_callback_list,function,user_data))
		{
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Scene_viewer_remove_transform_callback.  Could not remove callback");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_remove_transform_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_remove_transform_callback */

int Scene_viewer_add_destroy_callback(struct Scene_viewer *scene_viewer,
	CMISS_CALLBACK_FUNCTION(Scene_viewer_callback) *function,void *user_data)
/*******************************************************************************
LAST MODIFIED : 19 February 2002

DESCRIPTION :
Adds a callback to the <scene_viewer> that is called back before the scene
viewer is destroyed.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_add_destroy_callback);
	if (scene_viewer&&function)
	{
		if (CMISS_CALLBACK_LIST_ADD_CALLBACK(Scene_viewer_callback)(
			scene_viewer->destroy_callback_list,function,user_data))
		{
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Scene_viewer_add_destroy_callback.  Could not add callback");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_add_destroy_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_add_destroy_callback */

int Scene_viewer_remove_destroy_callback(struct Scene_viewer *scene_viewer,
	CMISS_CALLBACK_FUNCTION(Scene_viewer_callback) *function,void *user_data)
/*******************************************************************************
LAST MODIFIED : 19 February 2002

DESCRIPTION :
Removes the callback calling <function> with <user_data> from
<scene_viewer>.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_remove_destroy_callback);
	if (scene_viewer&&function)
	{
		if (CMISS_CALLBACK_LIST_REMOVE_CALLBACK(Scene_viewer_callback)(
			scene_viewer->destroy_callback_list,function,user_data))
		{
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Scene_viewer_remove_destroy_callback.  Could not remove callback");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_remove_destroy_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_remove_destroy_callback */

int Scene_viewer_add_repaint_required_callback(struct Scene_viewer *scene_viewer,
	CMISS_CALLBACK_FUNCTION(Scene_viewer_callback) *function, void *user_data)
/*******************************************************************************
LAST MODIFIED : 20 September 2007

DESCRIPTION :
This callback will be notified when a repaint is required by a windowless mode
scene_viewer, so that the host application can do the redraw.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_add_repaint_required_callback);
	if (scene_viewer&&function)
	{
		return_code = 
			CMISS_CALLBACK_LIST_ADD_CALLBACK(Scene_viewer_callback)(
				scene_viewer->repaint_required_callback_list,function,user_data);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_add_repaint_required_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_add_repaint_required_callback */

int Scene_viewer_remove_repaint_required_callback(struct Scene_viewer *scene_viewer,
	CMISS_CALLBACK_FUNCTION(Scene_viewer_callback) *function, void *user_data)
/*******************************************************************************
LAST MODIFIED : 20 September 2007

DESCRIPTION :
Removes the callback calling <function> with <user_data> from
<scene_viewer>.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_remove_repaint_required_callback);
	if (scene_viewer&&function)
	{
		if (CMISS_CALLBACK_LIST_REMOVE_CALLBACK(Scene_viewer_callback)(
			scene_viewer->repaint_required_callback_list,function,user_data))
		{
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Scene_viewer_remove_repaint_required_callback.  Could not remove callback");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_remove_repaint_required_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_remove_repaint_required_callback */

PROTOTYPE_ENUMERATOR_STRING_FUNCTION(Scene_viewer_blending_mode)
/*******************************************************************************
LAST MODIFIED : 16 April 2003

DESCRIPTION :
Returns a string label for the <blending_mode>.
NOTE: Calling function must not deallocate returned string.
==============================================================================*/
{
	const char *enumerator_string;

	ENTER(ENUMERATOR_STRING(Scene_viewer_blending_mode));
	switch (enumerator_value)
	{
		case SCENE_VIEWER_BLEND_NORMAL:
		{
			enumerator_string="blend_normal";
		} break;
		case SCENE_VIEWER_BLEND_NONE:
		{
			enumerator_string="blend_none";
		} break;
		case SCENE_VIEWER_BLEND_TRUE_ALPHA:
		{
			enumerator_string="blend_true_alpha";
		} break;
		default:
		{
			enumerator_string=(const char *)NULL;
		} break;
	}
	LEAVE;

	return (enumerator_string);
} /* ENUMERATOR_STRING(Scene_viewer_blending_mode) */

DEFINE_DEFAULT_ENUMERATOR_FUNCTIONS(Scene_viewer_blending_mode)

char *Scene_viewer_buffering_mode_string(
	enum Scene_viewer_buffering_mode buffering_mode)
/*******************************************************************************
LAST MODIFIED : 14 October 1998

DESCRIPTION :
Returns a string label for the <buffering_mode>.
NOTE: Calling function must not deallocate returned string.
==============================================================================*/
{
	char *return_string;

	ENTER(Scene_viewer_buffering_mode_string);
	switch (buffering_mode)
	{
		case SCENE_VIEWER_PIXEL_BUFFER:
		{
			return_string="pixel_buffer";
		} break;
		case SCENE_VIEWER_SINGLE_BUFFER:
		{
			return_string="single_buffer";
		} break;
		case SCENE_VIEWER_DOUBLE_BUFFER:
		{
			return_string="double_buffer";
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"Scene_viewer_buffering_mode_string.  Unknown buffer mode");
			return_string=(char *)NULL;
		}
	}
	LEAVE;

	return (return_string);
} /* Scene_viewer_buffering_mode_string */

char *Scene_viewer_stereo_mode_string(
	enum Scene_viewer_stereo_mode stereo_mode)
/*******************************************************************************
LAST MODIFIED : 16 September 2002

DESCRIPTION :
Returns a string label for the <stereo_mode>.
NOTE: Calling function must not deallocate returned string.
==============================================================================*/
{
	char *return_string;

	ENTER(Scene_viewer_stereo_mode_string);
	switch (stereo_mode)
	{
		case SCENE_VIEWER_MONO:
		{
			return_string="mono";
		} break;
		case SCENE_VIEWER_STEREO:
		{
			return_string="stereo";
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"Scene_viewer_stereo_mode_string.  Unknown stereo mode");
			return_string=(char *)NULL;
		}
	}
	LEAVE;

	return (return_string);
} /* Scene_viewer_stereo_mode_string */

char *Scene_viewer_projection_mode_string(
	enum Scene_viewer_projection_mode projection_mode)
/*******************************************************************************
LAST MODIFIED : 14 October 1998

DESCRIPTION :
Returns a string label for the <projection_mode>.
NOTE: Calling function must not deallocate returned string.
==============================================================================*/
{
	char *return_string;

	ENTER(Scene_viewer_projection_mode_string);
	switch (projection_mode)
	{
		case SCENE_VIEWER_PARALLEL:
		{
			return_string="parallel";
		} break;
		case SCENE_VIEWER_PERSPECTIVE:
		{
			return_string="perspective";
		} break;
		case SCENE_VIEWER_CUSTOM:
		{
			return_string="custom";
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"Scene_viewer_projection_mode_string.  Unknown projection mode");
			return_string=(char *)NULL;
		}
	}
	LEAVE;

	return (return_string);
} /* Scene_viewer_projection_mode_string */

char *Scene_viewer_transparency_mode_string(
	enum Scene_viewer_transparency_mode transparency_mode)
/*******************************************************************************
LAST MODIFIED : 26 June 2003

DESCRIPTION :
Returns a string label for the <transparency_mode>.
NOTE: Calling function must not deallocate returned string.
==============================================================================*/
{
	char *return_string;

	ENTER(Scene_viewer_transparency_mode_string);
	switch (transparency_mode)
	{
		case SCENE_VIEWER_FAST_TRANSPARENCY:
		{
			return_string="fast_transparency";
		} break;
		case SCENE_VIEWER_SLOW_TRANSPARENCY:
		{
			return_string="slow_transparency";
		} break;
		case SCENE_VIEWER_LAYERED_TRANSPARENCY:
		{
			return_string="layered_transparency";
		} break;
		case SCENE_VIEWER_ORDER_INDEPENDENT_TRANSPARENCY:
		{
			return_string="order_independent_transparency";
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"Scene_viewer_transparency_mode_string.  Unknown transparency mode");
			return_string=(char *)NULL;
		}
	}
	LEAVE;

	return (return_string);
} /* Scene_viewer_transparency_mode_string */

char *Scene_viewer_viewport_mode_string(
	enum Scene_viewer_viewport_mode viewport_mode)
/*******************************************************************************
LAST MODIFIED : 04 February 2005

DESCRIPTION :
Returns a string label for the <viewport_mode>.
NOTE: Calling function must not deallocate returned string.
==============================================================================*/
{
	char *return_string;

	ENTER(Scene_viewer_viewport_mode_string);
	switch (viewport_mode)
	{
		case SCENE_VIEWER_RELATIVE_VIEWPORT:
		{
			return_string="relative_viewport";
		} break;
		case SCENE_VIEWER_DISTORTING_RELATIVE_VIEWPORT:
		{
			return_string="distorting_relative_viewport";
		} break;
		case SCENE_VIEWER_ABSOLUTE_VIEWPORT:
		{
			return_string="absolute_viewport";
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"Scene_viewer_viewport_mode_string.  Unknown viewport mode");
			return_string=(char *)NULL;
		}
	}
	LEAVE;

	return (return_string);
} /* Scene_viewer_viewport_mode_string */

struct Graphics_buffer *Scene_viewer_get_graphics_buffer(
	struct Scene_viewer *scene_viewer)
/*******************************************************************************
LAST MODIFIED : 12 May 2004

DESCRIPTION :
Gets the <graphics_buffer> used for 3D graphics in the scene_viewer.
==============================================================================*/
{
	struct Graphics_buffer *graphics_buffer;

	ENTER(Scene_viewer_get_graphics_buffer);
	if (scene_viewer)
	{
		graphics_buffer = scene_viewer->graphics_buffer;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_graphics_buffer.  Missing scene_viewer");
		graphics_buffer = (struct Graphics_buffer *)NULL;
	}
	LEAVE;

	return (graphics_buffer);
} /* Scene_viewer_get_graphics_buffer */

#if defined (CARBON_USER_INTERFACE)
int Scene_viewer_carbon_set_window_size(struct Scene_viewer *scene_viewer,
	int width, int height, int portx, int porty, int clip_width, int clip_height)
/*******************************************************************************
LAST MODIFIED : 16 February 2007

DESCRIPTION :
Sets the coordinates within the graphics port which the scene_viewer should
respect.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_scene_viewer_get_near_and_far_plane);
	if (scene_viewer)
	{
		return_code = Graphics_buffer_carbon_set_window_size(scene_viewer->graphics_buffer,
			width, height, portx, porty, clip_width, clip_height);
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_scene_viewer_carbon_set_window_size.  "
			"Missing scene_viewer parameter.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_scene_viewer_carbon_set_window_size */
#endif /* defined (CARBON_USER_INTERFACE) */

#if defined (WIN32_USER_INTERFACE)
int Scene_viewer_win32_set_window_size(struct Scene_viewer *scene_viewer,
	int width, int height, int x, int y)
/*******************************************************************************
LAST MODIFIED : 14 September 2007

DESCRIPTION :
Sets the maximum extent of the graphics window within which individual paints 
will be requested with handle_windows_event.
==============================================================================*/
{
	int return_code;

	ENTER(Cmiss_scene_viewer_get_near_and_far_plane);
	if (scene_viewer)
	{
		return_code = Graphics_buffer_win32_set_window_size(scene_viewer->graphics_buffer,
			width, height, x, y);
	}
	else
	{
		display_message(ERROR_MESSAGE,"Cmiss_scene_viewer_win32_set_window_size.  "
			"Missing scene_viewer parameter.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Cmiss_scene_viewer_win32_set_window_size */
#endif /* defined (WIN32_USER_INTERFACE) */

unsigned int Scene_viewer_get_frame_count(struct Scene_viewer *scene_viewer)
/*******************************************************************************
LAST MODIFIED : 24 April 2008

DESCRIPTION :
Returns a count of the number of scene viewer redraws.
==============================================================================*/
{
	unsigned int frame_count;

	ENTER(Scene_viewer_get_frame_count);
	if (scene_viewer)
	{
		frame_count = scene_viewer->frame_count;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_frame_count.  Invalid argument(s)");
		frame_count = 0;
	}
	LEAVE;

	return (frame_count);
} /* Scene_viewer_get_frame_count */

