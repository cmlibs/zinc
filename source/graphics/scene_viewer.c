/*******************************************************************************
FILE : scene_viewer.c

LAST MODIFIED : 13 June 2000

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
#include <stdio.h>
#include <math.h>
#if defined (USING_3DFX_MESA)
/* code to take advantage of the 3Dfx cards "full screen mode" */
/* uses dynamic loading so that can still use versions before Mesa 2.6 */
#include <dlfcn.h>
#include <GL/xmesa.h>
#endif /* defined (USING_3DFX_MESA) */
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <GL/glx.h>
#include "three_d_drawing/dm_interface.h"
#include "three_d_drawing/ThreeDDraw.h"
#include "three_d_drawing/movie_extensions.h"
#include "general/callback_private.h"
#include "general/debug.h"
#include "general/geometry.h"
#include "general/image_utilities.h"
#include "general/matrix_vector.h"
#include "general/object.h"
#include "graphics/colour.h"
#include "graphics/graphics_library.h"
#include "graphics/light.h"
#include "graphics/light_model.h"
#include "graphics/scene.h"
#include "graphics/scene_viewer.h"
#include "graphics/texture.h"
/* #include "graphics/write_to_video.h" */
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module constants
----------------
*/
#define SCENE_VIEWER_PICK_SIZE 7.0

/*
Module types
------------
*/

FULL_DECLARE_CALLBACK_TYPES(Scene_viewer_transform, \
	struct Scene_viewer *, void *);

struct Scene_viewer
/*******************************************************************************
LAST MODIFIED : 11 April 2000

DESCRIPTION :
==============================================================================*/
{
	enum Scene_viewer_input_mode input_mode;
	/* following flag forces the scene_viewer temporarily into transform mode
		 when the control key is held down */
	int temporary_transform_mode;
	/* scene to be viewed */
	struct Scene *scene;
	/* overlay scene for showing spectrum scale, annotation, etc. This is always
		 set up with a parallel projection fitting the largest cube in the window
		 with coordinates ranging from -1 to +1 from left to right, bottom to top
		 and far to near. If the window is non-square, coordinates outside -1 to +1
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
	double left,right,bottom,top,near,far;
	/* Scale factors for controlling how rate of translate, tumble and zoom
		 transformations in relation to mouse movements. Setting a value to
		 zero turns off that transform capability. */
	double translate_rate,tumble_rate,zoom_rate;
	/* For CUSTOM projection only: 4X4 projection and modelview matrices. In
		 contrast to OpenGL these are stored in rows first ie. m0 m1 m2 m3 would be
		 the first row, m4 m5 m6 m7 the second, etc. */
	double projection_matrix[16],modelview_matrix[16],
		window_projection_matrix[16];
	/* The projection matrix, whether set directly for CUSTOM projection or
		 calculated for PARALLEL and PERSPECTIVE projections using the viewing
		 volume, converts 3-D positions into Normalized Device Coordinates (NDCs) in
		 a cube from -1 to +1 in each direction.  In the z (depth) direction the
		 values from -1 (=near plane) to +1 (=far plane) are already where we want
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
		 user coordinates to pixels is used (ABSOLUTE_VIEWPORT). */
	enum Scene_viewer_viewport_mode viewport_mode;
	/* Specifies the offset and scale of user coordinates in the physical
		 viewport, by supplying the user coordinate of the top,left position in
		 and the number of pixels plotted for a change of 1 in user units. Note
		 that these are in no way restricted to integer values.
		 ???RC.  Write how to handle y increasing down the screen? */
	double viewport_left,viewport_top,viewport_pixels_per_unit_x,
		viewport_pixels_per_unit_y;
	/* specifies the quality of transparency rendering */
	enum Scene_viewer_transparency_mode transparency_mode;
	/* number of layers used in layered transparency mode */
	int transparency_layers;
	/* A background texture may also be displayed behind the projected image */
	struct Texture *background_texture;
	/* When an ABSOLUTE_VIEWPORT is used the following values specify the
		 position and scale of the image relative to user coordinates. In
		 RELATIVE_VIEWPORT mode, these values are ignored and the image is
		 drawn behind the normalized device coordinate range.
		 ???RC.  Allow texture to be cropped as well? */
	double bk_texture_left,bk_texture_top,bk_texture_width,
		bk_texture_height,bk_texture_max_pixels_per_polygon;
	int bk_texture_undistort_on;
	struct Callback_data input_callback;
	/* list of callbacks requested by other objects when view changes */
	struct LIST(CALLBACK_ITEM(Scene_viewer_transform)) *sync_callback_list;
	struct LIST(CALLBACK_ITEM(Scene_viewer_transform)) *transform_callback_list;
	/* the scene_viewer must always have a light model */
	struct Light_model *light_model;
	/* lights in this list are oriented relative to the viewer */
	struct LIST(Light) *list_of_lights;
	Widget drawing_widget,parent;
	XtWorkProcId idle_update_proc;
	/* managers and callback IDs for automatic updates */
	struct MANAGER(Light) *light_manager;
	void *light_manager_callback_id;
	struct MANAGER(Light_model) *light_model_manager;
	void *light_model_manager_callback_id;
	struct MANAGER(Scene) *scene_manager;
	void *scene_manager_callback_id;
	struct MANAGER(Texture) *texture_manager;
	void *texture_manager_callback_id;
	/* interaction */
	/* Note: interactive_tool is NOT accessed by Scene_viewer; up to dialog
		 owning it to clear it if it is destroyed. This is usually ensured by having
		 a tool chooser in the parent dialog */
	struct Interactive_tool *interactive_tool;
	/* background */
	struct Colour background_colour;
	enum Scene_viewer_buffer_mode buffer_mode;
	int pixel_height,pixel_width,update_pixel_image;
	void *pixel_data;
	int antialias;
	int perturb_lines;
	struct User_interface *user_interface;
}; /* struct Scene_viewer */

/*
Module variables
----------------
*/

/*
Module functions
----------------
*/
DEFINE_CALLBACK_MODULE_FUNCTIONS(Scene_viewer_transform)

DEFINE_CALLBACK_FUNCTIONS(Scene_viewer_transform, \
	struct Scene_viewer *,void *)

static int Scene_viewer_render_background_texture(
	struct Scene_viewer *scene_viewer,int viewport_width,int viewport_height)
/*******************************************************************************
LAST MODIFIED : 5 April 2000

DESCRIPTION :
==============================================================================*/
{
	double corner_x[4],corner_y[4],corr_x1,corr_x2,corr_y1,corr_y2,
		distortion_centre_x,distortion_centre_y,distortion_factor_k1,
		dist_x,dist_y1,dist_y2,min_x,max_x,min_y,max_y,tex_ratio_x,tex_ratio_y,
		viewport_texture_height,viewport_texture_width;
	float centre_x,centre_y,factor_k1,texture_width,texture_height;
	GLdouble viewport_left,viewport_right,viewport_bottom,viewport_top;
	int height_texels,i,j,k,min_i,max_i,min_j,max_j,return_code,
		texels_per_polygon_x,texels_per_polygon_y,width_texels;

	ENTER(Scene_viewer_render_background_texture);
	if (scene_viewer&&scene_viewer->background_texture)
	{
		/* get information about the texture */
		Texture_get_original_size(scene_viewer->background_texture,
			&width_texels,&height_texels);
		Texture_get_physical_size(scene_viewer->background_texture,
			&texture_width,&texture_height);
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
			(scene_viewer->viewport_left - scene_viewer->bk_texture_left);
		viewport_right = viewport_left +
			((double)viewport_width/scene_viewer->viewport_pixels_per_unit_x)*
			texture_width/viewport_texture_width;
		viewport_top = texture_height +
			texture_height/viewport_texture_height *
			(scene_viewer->viewport_top - scene_viewer->bk_texture_top);
		viewport_bottom=viewport_top -
			((double)viewport_height/scene_viewer->viewport_pixels_per_unit_y)*
			texture_height/viewport_texture_height;
#if defined (DEBUG)
		/*???debug */
		printf("viewport left=%f right=%f  top=%f bottom=%f\n",
			viewport_left,viewport_right,viewport_top,viewport_bottom);
#endif /* defined (DEBUG) */
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(viewport_left,viewport_right,viewport_bottom,viewport_top,
			-1.0,1.0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		compile_Texture(scene_viewer->background_texture,NULL);
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
			scene_viewer->viewport_pixels_per_unit_x*
			viewport_texture_width/width_texels <=
			scene_viewer->bk_texture_max_pixels_per_polygon)&&
			(texels_per_polygon_x < width_texels))
		{
			texels_per_polygon_x *= 2;
		}
		texels_per_polygon_y=1;
		while ((2*texels_per_polygon_y*
			scene_viewer->viewport_pixels_per_unit_y*
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

static int Scene_viewer_render_scene_private(struct Scene_viewer *scene_viewer,
	int picking_on, int left, int bottom, int right, int top)
/*******************************************************************************
LAST MODIFIED : 1 June 2000

DESCRIPTION :
Called to redraw the Scene_viewer scene after changes in the display lists or
viewing transformations.
If <picking_on> is set, then don't clear the projection matrix to identity so
that picking matrix may be set before calling this routine.
<left>, <right>, <top> and <bottom> define the viewport to draw into, if they
are all zero then the scene_viewer->widget size is used instead.
There are convienience functions, Scene_viewer_render_scene,
Scene_viewer_render_scene_with_picking, Scene_viewer_render_scene_in_viewport to
access this function.
==============================================================================*/
{
	Dimension xwidth, xheight;
	double dx,dy,dz,factor,max_x,max_y,pixel_offset_x,pixel_offset_y;
	GLboolean valid_raster;
	static GLint viewport[4]={0,0,1,1};
	GLint stencil_bits;
	GLdouble model_matrix[16],obj_x,obj_y,obj_z,projection_matrix[16],
		temp_matrix[16],temp_proj_matrix[16];
	int accumulation_count,antialias,do_render,layer, layers, return_code,height,
		i,j,scene_redraws,width;
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
	void *new_data;

	ENTER(Scene_viewer_render_scene);
	if (scene_viewer)
	{
		return_code=1;
		if ((!left) && (!bottom) && (!right) && (!top))
		{
			XtVaGetValues(scene_viewer->drawing_widget,
				XmNwidth,&xwidth,
				XmNheight,&xheight,NULL);
			left = 0;
			bottom = 0;
			right = xwidth;
			top = xheight;
		}
		width = right - left;
		height = top - bottom;
		/* only redraw if the drawing widget has area and neither it nor any of its
			 parents are unmanaged */
		do_render=(0<width)&&(0<height)&&XtIsManaged(scene_viewer->drawing_widget)&&
			XtIsManaged(scene_viewer->parent);
		if (do_render)
		{
			if (SCENE_VIEWER_NO_INPUT_OR_DRAW==scene_viewer->input_mode)
			{
				glClearColor(0.6,0.6,0.6,0.);
				glClearDepth(1.0);
				glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
				return_code=1;
			}
			else
			{
				if (!(scene_viewer->update_pixel_image)&&
					/*				(SCENE_VIEWER_NO_INPUT==scene_viewer->input_mode)&&*/
					(SCENE_VIEWER_PIXEL_BUFFER==scene_viewer->buffer_mode)&&
					(scene_viewer->pixel_width)&&(scene_viewer->pixel_height))
				{
					glClearColor((scene_viewer->background_colour).red,
						(scene_viewer->background_colour).green,
						(scene_viewer->background_colour).blue,0.);
					glClearDepth(1.0);
					glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
					glGetDoublev(GL_MODELVIEW_MATRIX,model_matrix);
					glGetDoublev(GL_PROJECTION_MATRIX,projection_matrix);
					/*				glGetIntegerv(GL_VIEWPORT,viewport);*/
					/* for OpenGL window z coordinates, 0.0=near, 1.0=far */
					if (GL_TRUE==gluUnProject(0.0001,0.0001,0.1,model_matrix,
						projection_matrix,viewport,&obj_x,&obj_y,&obj_z))
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
								"Scene_viewer_render_scene_private.  "
								"Culled raster position for redraw");
							/*???SAB.  If we return a zero return code the iterator functions
								calling this routine will stop and it won't attempt to update
								any other scene viewers */
						}
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Scene_viewer_render_scene_private.  Unable to unproject");
						return_code=0;
					}
				}
				else
				{
					/* It is also possible to test the extensions at compile time
						 by using #if defined GL_EXT_polygon_offset */
					if (query_gl_extension("GL_EXT_polygon_offset"))
					{
						if (scene_viewer->perturb_lines)
						{
							glPolygonOffsetEXT(1.5,0.000001);
							glEnable(GL_POLYGON_OFFSET_EXT);
						}
						else
						{
							glDisable(GL_POLYGON_OFFSET_EXT);
						}
					}
					if (compile_Scene(scene_viewer->scene)&&
						((!scene_viewer->overlay_scene)||
							compile_Scene(scene_viewer->overlay_scene)))
					{
						if (scene_viewer->overlay_scene)
						{
							/*???RC property functions should be obtained once, elsewhere */
							glGetIntegerv(GL_STENCIL_BITS,&stencil_bits);
						}
						else
						{
							stencil_bits=0;
						}
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
						glDepthRange((GLclampd)0,(GLclampd)1);
						glEnable(GL_DEPTH_TEST);
						/* glDepthFunc(GL_LESS); */
						/* Get size of alpha [blending] buffer. */
						/* glGetIntegerv(GL_ALPHA_BITS,&alpha_bits); */
						/* turn on alpha */
						glEnable(GL_BLEND);
						glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
						glViewport((GLint)left, (GLint)bottom, (GLint)width, (GLint)height);
						glScissor((GLint)left, (GLint)bottom, (GLint)width, (GLint)height);
						glEnable(GL_SCISSOR_TEST);
						/* glPushAttrib(GL_VIEWPORT_BIT); */
#if defined (OLD_CODE)
						/* clear the screen: colour buffer and depth buffer */
						glClearColor((scene_viewer->background_colour).red,
							(scene_viewer->background_colour).green,
							(scene_viewer->background_colour).blue,0.);
						glClearDepth(1.0);
						if (0<stencil_bits)
						{
							glClearStencil(0);
							glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
								GL_STENCIL_BUFFER_BIT);
						}
						else
						{
							glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
						}
#endif /* defined (OLD_CODE) */
						reset_Lights();

						/* light model */
						compile_Light_model(scene_viewer->light_model);
						execute_Light_model(scene_viewer->light_model);
						/* in picking mode the starting projection matrix is already
							 supplied */
						if (!picking_on)
						{
							/* set projection matrix/viewing volume */
							glMatrixMode(GL_PROJECTION);
							glLoadIdentity();
							if (SCENE_VIEWER_CUSTOM != scene_viewer->projection_mode)
							{
								/* store calculated projection matrix for later reference */
								if (SCENE_VIEWER_PARALLEL==scene_viewer->projection_mode)
								{
									glOrtho(scene_viewer->left,scene_viewer->right,
										scene_viewer->bottom,scene_viewer->top,
										scene_viewer->near,scene_viewer->far);
								}
								else
								{
									/* adjust left,right,bottom,top from lookat plane to near
										 plane */
									dx=scene_viewer->eyex-scene_viewer->lookatx;
									dy=scene_viewer->eyey-scene_viewer->lookaty;
									dz=scene_viewer->eyez-scene_viewer->lookatz;
									factor=scene_viewer->near/sqrt(dx*dx+dy*dy+dz*dz);
									/* perspective projection */
									glFrustum(
										scene_viewer->left*factor,
										scene_viewer->right*factor,
										scene_viewer->bottom*factor,
										scene_viewer->top*factor,
										scene_viewer->near,
										scene_viewer->far);
								}
								glGetDoublev(GL_PROJECTION_MATRIX,temp_matrix);
								/* convert from OpenGL to our matrix format (transpose) */
								for (i=0;i<4;i++)
								{
									for (j=0;j<4;j++)
									{
										scene_viewer->projection_matrix[i*4+j]=
											(double)temp_matrix[j*4+i];
									}
								}
							}
						}
						pixel_offset_x=0;
						pixel_offset_y=0;

						/* no antialiasing while picking */
						if (!picking_on)
						{
							antialias = scene_viewer->antialias;
						}
						else
						{
							antialias = 0;
						}
						if (antialias>1)
						{
							scene_redraws=antialias;
						}
						else
						{
							scene_redraws=1;
						}
						/* compile the viewer lights */
						FOR_EACH_OBJECT_IN_LIST(Light)(compile_Light,(void *)NULL,
							scene_viewer->list_of_lights);
						if (0<stencil_bits)
						{
							/* enable stencil buffer */
							glEnable(GL_STENCIL_TEST);
						}
						for (accumulation_count=0;accumulation_count<scene_redraws;
								 accumulation_count++)
						{
							/* clear the screen: colour buffer and depth buffer */
							glClearColor((scene_viewer->background_colour).red,
								(scene_viewer->background_colour).green,
								(scene_viewer->background_colour).blue,0.);
							glClearDepth(1.0);
							if (0<stencil_bits)
							{
								glClearStencil(0);
								glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
									GL_STENCIL_BUFFER_BIT);
							}
							else
							{
								glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
							}

							/********* RENDER BACKGROUND TEXTURE *********/
							if ((!picking_on)&&scene_viewer->background_texture)
							{
								glMatrixMode(GL_PROJECTION);
								glLoadIdentity();
								glDisable(GL_LIGHTING);
								glColor3f(1,1,1);
								Scene_viewer_render_background_texture(scene_viewer,
									width,height);
								glEnable(GL_LIGHTING);
							}

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
										"Scene_viewer_render_scene_private.  "
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
							for (i=0;i<16;i++)
							{
								if (0==(i%5))
								{
									temp_matrix[i]=1.0;
								}
								else
								{
									temp_matrix[i]=0.0;
								}
							}
							/* offsetting image by [sub]pixel distances for anti-aliasing.
								 offset_x is distance image is shifted to the right, offset_y is
								 distance image is shifted up. The actual offsets used are
								 fractions of half the viewport width or height,since normalized
								 device coordinates (NDCs) range from -1 to +1 */
							temp_matrix[12]=2.0*pixel_offset_x/width;
							temp_matrix[13]=2.0*pixel_offset_y/height;

							/********* RENDER OVERLAY SCENE *********/
							if (!picking_on&&scene_viewer->overlay_scene&&(0<width)&&
								(0<height))
							{
								/* set up parallel projection/modelview combination that gives
									 coordinates ranging from -1 to +1 from left to right and
									 bottom to top in the largest square that fits inside the
									 viewer. Also has -1 to +1 range from far to near */
								glMatrixMode(GL_PROJECTION);
								glLoadIdentity();
								if (antialias)
								{
									glMultMatrixd(temp_matrix);
								}
								if (width > height)
								{
									max_x = (double)width/(double)height;
									max_y = 1.0;
								}
								else
								{
									max_x = 1.0;
									max_y = (double)height/(double)width;
								}
								if (0<stencil_bits)
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
								if (picking_on)
								{
									/* Always execute simply for picking */
									execute_Scene(scene_viewer->overlay_scene);
								}
								else
								{
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
								}
							}

							/********* RENDER MAIN SCENE *********/
							if (!picking_on)
							{
								/* load identity matrix for rendering normal scene */
								glMatrixMode(GL_PROJECTION);
								glLoadIdentity();
							}
							if (antialias)
							{
								glMultMatrixd(temp_matrix);
							}
							temp_matrix[12]=0.0;
							temp_matrix[13]=0.0;
							if (SCENE_VIEWER_RELATIVE_VIEWPORT==scene_viewer->viewport_mode)
							{
								/* relative viewport: NDC volume is scaled to the largest size
									 that can fit in the viewport without distorting its shape. */
								if (scene_viewer->NDC_height/scene_viewer->NDC_width >
									(double)height/(double)width)
								{
									/* make NDC represent a wider viewing volume. */
									temp_matrix[0] *= (scene_viewer->NDC_width*height/
										(scene_viewer->NDC_height*width));
								}
								else
								{
									/* make NDC represent a taller viewing volume */
									temp_matrix[5] *= (scene_viewer->NDC_height*width/
										(scene_viewer->NDC_width*height));
								}
							}
							else
							{
								/* absolute viewport: NDC volume is placed in the position
									 described by the NDC_info relative to user viewport
									 coordinates - as with the background texture */
								temp_matrix[0] *= scene_viewer->NDC_width*
									scene_viewer->viewport_pixels_per_unit_x/width;
								temp_matrix[5] *= scene_viewer->NDC_height*
									scene_viewer->viewport_pixels_per_unit_y/height;
								temp_matrix[12]= -1.0+
									((scene_viewer->viewport_pixels_per_unit_x)/width)*
									((scene_viewer->NDC_width)+
										2.0*(scene_viewer->NDC_left-scene_viewer->viewport_left));
								temp_matrix[13]=1.0+
									((scene_viewer->viewport_pixels_per_unit_y)/height)*
									(-(scene_viewer->NDC_height)+
										2.0*(scene_viewer->NDC_top-scene_viewer->viewport_top));
							}
							glMultMatrixd(temp_matrix);
							/* copy projection_matrix to OpenGL format (ie. transpose) and
								 apply. Also save the actual projection matrix used to fill
								 window */
							for (i=0;i<4;i++)
							{
								for (j=0;j<4;j++)
								{
									temp_proj_matrix[j*4+i]=
										(GLdouble)(scene_viewer->projection_matrix)[i*4+j];
									scene_viewer->window_projection_matrix[i*4+j]=
										temp_matrix[   i]*scene_viewer->projection_matrix[   j]+
										temp_matrix[ 4+i]*scene_viewer->projection_matrix[ 4+j]+
										temp_matrix[ 8+i]*scene_viewer->projection_matrix[ 8+j]+
										temp_matrix[12+i]*scene_viewer->projection_matrix[12+j];
								}
							}
							glMultMatrixd(temp_proj_matrix);
							/* ModelView matrix */
							glMatrixMode(GL_MODELVIEW);
							glLoadIdentity();
#if defined (OLD_CODE)
							if (0==accumulation_count)
							{
#endif /* defined (OLD_CODE) */
								reset_Lights();
								/* turn on lights that are part of the Scene_viewer,
									 ie. headlamps */
								FOR_EACH_OBJECT_IN_LIST(Light)(execute_Light,(void *)NULL,
									scene_viewer->list_of_lights);
#if defined (OLD_CODE)
							}
#endif /* defined (OLD_CODE) */
							if (SCENE_VIEWER_CUSTOM==scene_viewer->projection_mode)
							{
								for (i=0;i<4;i++)
								{
									for (j=0;j<4;j++)
									{
										temp_matrix[j*4+i]=
											(GLdouble)scene_viewer->modelview_matrix[i*4+j];
									}
								}
								glMultMatrixd(temp_matrix);
							}
							else
							{
								gluLookAt(scene_viewer->eyex,scene_viewer->eyey,
									scene_viewer->eyez,scene_viewer->lookatx,
									scene_viewer->lookaty,scene_viewer->lookatz,
									scene_viewer->upx,scene_viewer->upy,scene_viewer->upz);
								glGetDoublev(GL_MODELVIEW_MATRIX,temp_matrix);
								/* store calculated modelview matrix for later reference */
								/* convert from OpenGL to our matrix format (transpose) */
								for (i=0;i<4;i++)
								{
									for (j=0;j<4;j++)
									{
										scene_viewer->modelview_matrix[i*4+j]=
											(double)temp_matrix[j*4+i];
									}
								}
							}
							/* turn on lights that are part of the Scene and fixed relative
								 to it. Note the scene will have compiled them already. */
							for_each_Light_in_Scene(scene_viewer->scene,execute_Light,
								(void *)NULL);
							if (0<stencil_bits)
							{
								/* use full z-buffer for overlay ranging from -1 to 1 in z */
								glStencilFunc(GL_NOTEQUAL,1,1);
								glStencilOp(GL_KEEP,GL_KEEP,GL_KEEP);
							}
							/* draw the model display list */
							if (picking_on)
							{
								/* Always execute simply for picking */
								execute_Scene(scene_viewer->scene);
							}
							else
							{
								switch (scene_viewer->transparency_mode)
								{
									default:
									{
										glAlphaFunc(GL_GREATER,0.0);
										execute_Scene(scene_viewer->scene);
									} break;
									case SCENE_VIEWER_SLOW_TRANSPARENCY:
									{
										glEnable(GL_ALPHA_TEST);
										/* render only fragments with alpha=1.0 & write depth */
										glDepthMask(GL_TRUE);
										glAlphaFunc(GL_EQUAL,1.0);
										execute_Scene(scene_viewer->scene);
										/* render only fragments with alpha != 1.0; do not write
											 into depth buffer */
										glDepthMask(GL_FALSE);
										glAlphaFunc(GL_NOTEQUAL,1.0);
										execute_Scene(scene_viewer->scene);
										glDepthMask(GL_TRUE);
										glDisable(GL_ALPHA_TEST);
									} break;
									case SCENE_VIEWER_LAYERED_TRANSPARENCY:
									{
										/* Draw each layer separately to help transparency */
										glMatrixMode(GL_PROJECTION);
										glGetDoublev(GL_PROJECTION_MATRIX,temp_matrix);
										layers = scene_viewer->transparency_layers;
										for (layer = 0 ; layer < layers ; layer++)
										{		
											glMatrixMode(GL_PROJECTION);
											glLoadIdentity();
											glTranslated(0.0, 0.0, (double)layer * 2.0
												- (double)(layers - 1));
											glScaled(1.0, 1.0, (double)layers);
											glMultMatrixd(temp_matrix);
											glDepthRange(
												(double)(layers - layer - 1) / (double)layers,
												(double)(layers - layer) / (double)layers);
											execute_Scene(scene_viewer->scene);
										}
									} break;
								}
							}
							if (antialias)
							{
								if (0==accumulation_count)
								{
									glAccum(GL_LOAD,1.0f/(GLfloat)antialias);
								}
								else
								{
									glAccum(GL_ACCUM,1.0f/(GLfloat)antialias);
								}
							}
						}
						if (antialias)
						{
							glAccum(GL_RETURN,1.0f);
							glFlush();
						}
						if (0<stencil_bits)
						{
							/* disable stencil buffer */
							glDisable(GL_STENCIL_TEST);
						}
						return_code=1;
					}
					if (SCENE_VIEWER_PIXEL_BUFFER==scene_viewer->buffer_mode)
					{
						if (REALLOCATE(new_data,scene_viewer->pixel_data,char,
							3*(width+1)*(height+1)))
						{
							scene_viewer->pixel_data=new_data;
							glReadPixels(0,0,width,height,GL_RGB,GL_BYTE,
								scene_viewer->pixel_data);
							scene_viewer->pixel_width=width;
							scene_viewer->pixel_height=height;
							scene_viewer->update_pixel_image=0;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Scene_viewer_render_scene.  "
								"Unable to reallocate pixel dataspace");
							scene_viewer->pixel_width=0;
							scene_viewer->pixel_height=0;
						}
					}
				}
			}
#if defined (REPORT_GL_ERRORS)
			{
				char message[200];
				GLenum error;
				while(GL_NO_ERROR!=(error = glGetError()))
				{
					strcpy(message,"Scene_viewer_render_scene_private: GL ERROR ");
					strcat(message, (char *)gluErrorString(error));
					display_message(ERROR_MESSAGE, message);
				}
			}
#endif /* defined (REPORT_GL_ERRORS) */
		}
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
LAST MODIFIED : 1 September 1999

DESCRIPTION :
Called to redraw the Scene_viewer scene after changes in the display lists or
viewing transformations.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_render_scene);
	if (scene_viewer)
	{
		return_code=Scene_viewer_render_scene_private(scene_viewer, /*picking_on*/0,
			/*left*/0, /*bottom*/0, /*right*/0, /*top*/0);
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

int Scene_viewer_render_scene_with_picking(struct Scene_viewer *scene_viewer)
/*******************************************************************************
LAST MODIFIED : 1 September 1999

DESCRIPTION :
Called to redraw the Scene_viewer scene after changes in the display lists or
viewing transformations.  Writes picking names with the primitives.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_render_scene_with_picking);
	if (scene_viewer)
	{
		return_code=Scene_viewer_render_scene_private(scene_viewer, /*picking_on*/1,
			/*left*/0, /*bottom*/0, /*right*/0, /*top*/0);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_render_scene_with_picking.  Missing scene_viewer");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_render_scene_with_picking */

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
		return_code=Scene_viewer_render_scene_private(scene_viewer, /*picking_on*/0,
			left, bottom, right, top);
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

static Boolean Scene_viewer_idle_update(XtPointer scene_viewer_void)
/*******************************************************************************
LAST MODIFIED : 25 July 1998

DESCRIPTION :
A WorkProc that updates the scene_viewer, and then returns TRUE so that it is
removed from WorkProc queue.
==============================================================================*/
{
	struct Scene_viewer *scene_viewer;

	ENTER(Scene_viewer_idle_update);
	if (scene_viewer=(struct Scene_viewer *)scene_viewer_void)
	{
		X3dThreeDDrawingMakeCurrent(scene_viewer->drawing_widget);
		Scene_viewer_render_scene(scene_viewer);
		X3dThreeDDrawingSwapBuffers();
		/* set workproc no longer pending */
		scene_viewer->idle_update_proc=(XtWorkProcId)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_idle_update.  Missing scene_viewer");
	}
	LEAVE;

	return (TRUE); /* so workproc finished */
} /* Scene_viewer_idle_update */

static void Scene_viewer_initialize_callback(Widget scene_viewer_widget,
	XtPointer scene_viewer_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 November 1997

DESCRIPTION :
This is the configuration callback for the GL widget.
Sets the initial viewing transform for the scene_viewer.
???RC Needed at all?
???RC Move functionality elsewhere?
???RC Need MakeCurrent?
==============================================================================*/
{
	struct Scene_viewer *scene_viewer;

	ENTER(Scene_viewer_initialize_callback);
	USE_PARAMETER(call_data);
	USE_PARAMETER(scene_viewer_widget);
	if (scene_viewer=(struct Scene_viewer *)scene_viewer_structure)
	{
		/* initialize graphics library to load XFont */
		initialize_graphics_library(scene_viewer->user_interface);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_initialize_callback.  Missing scene_viewer");
	}
	LEAVE;
} /* Scene_viewer_initialize_callback */

static void Scene_viewer_resize_callback(Widget scene_viewer_widget,
	XtPointer scene_viewer_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 25 July 1998

DESCRIPTION :
Called when part of the Scene_viewer window is resized. All it does is notify
callbacks interested in the scene_viewers transformations.
==============================================================================*/
{
	struct Scene_viewer *scene_viewer;
	X3dThreeDDrawCallbackStruct *resize_callback_data;

	ENTER(Scene_viewer_resize_callback);
	if (scene_viewer_widget&&
		(scene_viewer=(struct Scene_viewer *)scene_viewer_structure)&&
		(resize_callback_data=(X3dThreeDDrawCallbackStruct *)call_data)&&
		(X3dCR_RESIZE==resize_callback_data->reason))
	{
		CALLBACK_LIST_CALL(Scene_viewer_transform)(
			scene_viewer->transform_callback_list,scene_viewer,NULL);		
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_resize_callback.  Invalid argument(s)");
	}
	LEAVE;
} /* Scene_viewer_resize_callback */

static void Scene_viewer_expose_callback(Widget scene_viewer_widget,
	XtPointer scene_viewer_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 25 July 1998

DESCRIPTION :
Called when part of the Scene_viewer window is exposed. Does not attempt to
redraw just the exposed area. Instead, it redraws the whole picture, but only
if there are no more expose events pending.
==============================================================================*/
{
	struct Scene_viewer *scene_viewer;
	XExposeEvent *expose_event;
	X3dThreeDDrawCallbackStruct *expose_callback_data;

	ENTER(Scene_viewer_expose_callback);
	if (scene_viewer_widget&&
		(scene_viewer=(struct Scene_viewer *)scene_viewer_structure)&&
		(expose_callback_data=(X3dThreeDDrawCallbackStruct *)call_data)&&
		(X3dCR_EXPOSE==expose_callback_data->reason)&&
		(expose_event=(XExposeEvent *)(expose_callback_data->event)))
	{
		/* if no more expose events in series */
		if (0==expose_event->count)
		{
			Scene_viewer_render_scene(scene_viewer);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_expose_callback.  Invalid argument(s)");
	}
	LEAVE;
} /* Scene_viewer_expose_callback */

static int Scene_viewer_unproject(int pointer_x,int pointer_y,
	double *near_x,double *near_y,double *near_z,
	double *far_x,double *far_y,double *far_z)
/*******************************************************************************
LAST MODIFIED : 28 February 1998

DESCRIPTION :
Converts the pointer location into locations on the near and far planes in
world space.
==============================================================================*/
{
	int return_code;
	GLdouble model_matrix[16],projection_matrix[16],obj_x,obj_y,obj_z,win_x,win_y;
	GLint viewport[4];

	ENTER(Scene_viewer_unproject);
	if (near_x&&near_y&&near_z&&far_x&&far_y&&far_z)
	{
		glGetDoublev(GL_MODELVIEW_MATRIX,model_matrix);
		glGetDoublev(GL_PROJECTION_MATRIX,projection_matrix);
		glGetIntegerv(GL_VIEWPORT,viewport);
		return_code=0;
		win_x=(GLdouble)pointer_x;
		win_y=(GLdouble)(viewport[3]-pointer_y);
		/* for OpenGL window z coordinates, 0.0=near, 1.0=far */
		if (GL_TRUE==gluUnProject(win_x,win_y,0.0,
			model_matrix,projection_matrix,viewport,&obj_x,&obj_y,&obj_z))
		{
			*near_x=(double)obj_x;
			*near_y=(double)obj_y;
			*near_z=(double)obj_z;
			if (GL_TRUE==gluUnProject(win_x,win_y,1.0,
				model_matrix,projection_matrix,viewport,&obj_x,&obj_y,&obj_z))
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

static int Scene_viewer_input_select_old(struct Scene_viewer *scene_viewer,
	XEvent *event)
/*******************************************************************************
LAST MODIFIED : 18 May 1998

DESCRIPTION :
Converts mouse button-press and motion events into viewing transformations in
<scene_viewer>.
==============================================================================*/
{
	double near_x,near_y,near_z,far_x,far_y,far_z,view[3];
	int return_code,pointer_x,pointer_y,scene_input_modifier;
	XButtonEvent *button_event;
#if defined (DEBUG)
	XKeyEvent *key_event;
#endif /* defined (DEBUG) */
	XMotionEvent *motion_event;
	GLuint *picking_buffer;
	GLint viewport[4],hits;

	ENTER(Scene_viewer_input_select_old);
	if (scene_viewer&&event)
	{
		return_code=1;
		/* get direction we are viewing along */
		view[0]=scene_viewer->lookatx-scene_viewer->eyex;
		view[1]=scene_viewer->lookaty-scene_viewer->eyey;
		view[2]=scene_viewer->lookatz-scene_viewer->eyez;
		normalize3(view);
		switch (event->type)
		{
			case ButtonPress:
			{
				button_event=&(event->xbutton);
				pointer_x=button_event->x;
				pointer_y=button_event->y;
				/*printf("SELECT: button %d press at %d %d\n",button_event->button,
					pointer_x,pointer_y);*/
				if (Scene_viewer_unproject(pointer_x,pointer_y,
					&near_x,&near_y,&near_z,&far_x,&far_y,&far_z))
				{
					/*printf("  Press near: %8.4f %8.4f %8.4f  Far: %8.4f %8.4f %8.4f\n",
						near_x,near_y,near_z,far_x,far_y,far_z);*/
					/* allocate space for picking buffer */
					/*???RC picking buffer size should be read from default file */
					if (ALLOCATE(picking_buffer,GLuint,5000))
					{
						glSelectBuffer(5000,picking_buffer);
						glRenderMode(GL_SELECT);
						glMatrixMode(GL_PROJECTION);
						glPushMatrix();
						glLoadIdentity();
						glGetIntegerv(GL_VIEWPORT,viewport);
						/*???RC Picking sensitivity should not be hardcoded - read from
							defaults file and/or set from text command */
						gluPickMatrix((GLdouble)pointer_x,(GLdouble)(viewport[3]-pointer_y),
							SCENE_VIEWER_PICK_SIZE,SCENE_VIEWER_PICK_SIZE,viewport);
						Scene_viewer_render_scene_with_picking(scene_viewer);
						glMatrixMode(GL_PROJECTION);
						glPopMatrix();
						glFlush();
						hits=glRenderMode(GL_RENDER);
						if (0>hits)
						{
							display_message(ERROR_MESSAGE,
								"Scene_viewer_input_select_old.  picking_buffer overflow");
						}
						scene_input_modifier=0;
						if (ShiftMask&(button_event->state))
						{
							scene_input_modifier += SCENE_INPUT_MODIFY_SHIFT;
						}
						/* note that control key currently overrides to transform mode */
						if (ControlMask&(button_event->state))
						{
							scene_input_modifier += SCENE_INPUT_MODIFY_CONTROL;
						}
						if (Mod1Mask&(button_event->state))
						{
							scene_input_modifier += SCENE_INPUT_MODIFY_ALT;
						}
						Scene_input(scene_viewer->scene,SCENE_BUTTON_PRESS,
							button_event->button,scene_input_modifier,view[0],view[1],view[2],
							near_x,near_y,near_z,far_x,far_y,far_z,hits,picking_buffer);
						DEALLOCATE(picking_buffer);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Scene_viewer_input_select_old.  Could not allocate picking_buffer");
						return_code=0;
					}
				}
			} break;
			case MotionNotify:
			{
				motion_event= &(event->xmotion);
				pointer_x=motion_event->x;
				pointer_y=motion_event->y;
#if defined (DEBUG)
				/*???debug */
				printf("mouse move to %d %d\n",pointer_x,pointer_y);
#endif /* defined (DEBUG) */
				if (Scene_viewer_unproject(pointer_x,pointer_y,
					&near_x,&near_y,&near_z,&far_x,&far_y,&far_z))
				{
					scene_input_modifier=0;
					if (ShiftMask&(motion_event->state))
					{
						scene_input_modifier += SCENE_INPUT_MODIFY_SHIFT;
					}
					/* note that control key currently overrides to transform mode */
					if (ControlMask&(motion_event->state))
					{
						scene_input_modifier += SCENE_INPUT_MODIFY_CONTROL;
					}
					if (Mod1Mask&(motion_event->state))
					{
						scene_input_modifier += SCENE_INPUT_MODIFY_ALT;
					}
					Scene_input(scene_viewer->scene,SCENE_MOTION_NOTIFY,
						-1/*button_number*/,scene_input_modifier,view[0],view[1],view[2],
						near_x,near_y,near_z,far_x,far_y,far_z,0/*hits*/,(GLuint *)NULL);
				}
			} break;
			case ButtonRelease:
			{
				button_event=&(event->xbutton);
				pointer_x=button_event->x;
				pointer_y=button_event->y;
#if defined (DEBUG)
				/*???debug */
				printf("mouse release at %d %d\n",pointer_x,pointer_y);
#endif /* defined (DEBUG) */
				if (Scene_viewer_unproject(pointer_x,pointer_y,
					&near_x,&near_y,&near_z,&far_x,&far_y,&far_z))
				{
#if defined (DEBUG)
					/*???debug */
					printf("Release near: %8.4f %8.4f %8.4f  Far: %8.4f %8.4f %8.4f\n",
						near_x,near_y,near_z,far_x,far_y,far_z);
#endif /* defined (DEBUG) */
					scene_input_modifier=0;
					if (ShiftMask&(button_event->state))
					{
						scene_input_modifier += SCENE_INPUT_MODIFY_SHIFT;
					}
					/* note that control key currently overrides to transform mode */
					if (ControlMask&(button_event->state))
					{
						scene_input_modifier += SCENE_INPUT_MODIFY_CONTROL;
					}
					if (Mod1Mask&(button_event->state))
					{
						scene_input_modifier += SCENE_INPUT_MODIFY_ALT;
					}
					Scene_input(scene_viewer->scene,SCENE_BUTTON_RELEASE,
						button_event->button,scene_input_modifier,
						view[0],view[1],view[2],near_x,near_y,near_z,far_x,far_y,far_z,
						0/*hits*/,(GLuint *)NULL);
				}
#if defined (DEBUG)
				/*???debug */
				printf("button %d release at %d %d\n",button_event->button,
					button_event->x,button_event->y);
#endif /* defined (DEBUG) */
			} break;
			case KeyPress:
			{
#if defined (DEBUG)
				key_event= &(event->xkey);
				printf("key %u press at %d %d\n",key_event->keycode,key_event->x,
					key_event->y);
#endif /* defined (DEBUG) */
#if defined (OLD_CODE)
				charcount=XLookupString(key_event,buffer,bufsize,&keysym,&compose);
				switch (keysym)
				{
					case XK_Delete:
					{
#if defined (DEBUG)
						printf("* Delete pressed!\n");
#endif /* defined (DEBUG) */
					} break;
				}
#endif /* defined (OLD_CODE) */
			} break;
			case KeyRelease:
			{
#if defined (DEBUG)
				key_event= &(event->xkey);
				printf("key %u release at %d %d\n",key_event->keycode,key_event->x,
					key_event->y);
#endif /* defined (DEBUG) */
			} break;
			default:
			{
				printf("Scene_viewer_input_select_old.  Invalid X event");
				return_code=0;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_input_select_old.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_input_select_old */

static int Scene_viewer_input_select(struct Scene_viewer *scene_viewer,
	XEvent *event)
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
	int button_number,input_modifier,modifier_state,mouse_event,return_code;
	XButtonEvent *button_event;
	XMotionEvent *motion_event;
	GLint viewport[4];
	struct Interactive_event *interactive_event;
	struct Interaction_volume *interaction_volume;

	ENTER(Scene_viewer_input_select);
	if (scene_viewer&&scene_viewer->interactive_tool&&event)
	{
		return_code=1;
		mouse_event=0;
		glGetIntegerv(GL_VIEWPORT,viewport);
		viewport_left   = (double)(viewport[0]);
		viewport_bottom = (double)(viewport[1]);
		viewport_width  = (double)(viewport[2]);
		viewport_height = (double)(viewport[3]);
		switch (event->type)
		{
			case ButtonPress:
			{
				interactive_event_type=INTERACTIVE_EVENT_BUTTON_PRESS;
				button_event=&(event->xbutton);
				centre_x=(double)(button_event->x);
				/* flip y as x event has y=0 at top of window, increasing down */
				centre_y=viewport_height-(double)(button_event->y)-1.0;
				button_number=button_event->button;
				modifier_state=button_event->state;
				mouse_event=1;
			} break;
			case MotionNotify:
			{
				interactive_event_type=INTERACTIVE_EVENT_MOTION_NOTIFY;
				motion_event= &(event->xmotion);
				centre_x=(double)(motion_event->x);
				/* flip y as x event has y=0 at top of window, increasing down */
				centre_y=viewport_height-(double)(motion_event->y)-1.0;
				button_number=-1;
				modifier_state=motion_event->state;
				mouse_event=1;
			} break;
			case ButtonRelease:
			{
				interactive_event_type=INTERACTIVE_EVENT_BUTTON_RELEASE;
				button_event=&(event->xbutton);
				centre_x=(double)(button_event->x);
				/* flip y as x event has y=0 at top of window, increasing down */
				centre_y=viewport_height-(double)(button_event->y)-1.0;
				button_number=button_event->button;
				modifier_state=button_event->state;
				mouse_event=1;
			}
			case KeyPress:
			{
#if defined (DEBUG)
	XKeyEvent *key_event;
#endif /* defined (DEBUG) */
#if defined (DEBUG)
				key_event= &(event->xkey);
				printf("key %u press at %d %d\n",key_event->keycode,key_event->x,
					key_event->y);
#endif /* defined (DEBUG) */
#if defined (OLD_CODE)
				charcount=XLookupString(key_event,buffer,bufsize,&keysym,&compose);
				switch (keysym)
				{
					case XK_Delete:
					{
#if defined (DEBUG)
						printf("* Delete pressed!\n");
#endif /* defined (DEBUG) */
					} break;
				}
#endif /* defined (OLD_CODE) */
			} break;
			case KeyRelease:
			{
#if defined (DEBUG)
				key_event= &(event->xkey);
				printf("key %u release at %d %d\n",key_event->keycode,key_event->x,
					key_event->y);
#endif /* defined (DEBUG) */
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
			if (ShiftMask&modifier_state)
			{
				input_modifier += INTERACTIVE_EVENT_MODIFIER_SHIFT;
			}
			/* note that control key currently overrides to transform mode */
			if (ControlMask&modifier_state)
			{
				input_modifier += INTERACTIVE_EVENT_MODIFIER_CONTROL;
			}
			if (Mod1Mask&modifier_state)
			{
				input_modifier += INTERACTIVE_EVENT_MODIFIER_ALT;
			}
			interaction_volume=create_Interaction_volume_ray_frustum(
				scene_viewer->modelview_matrix,scene_viewer->window_projection_matrix,
				viewport_left,viewport_bottom,viewport_width,viewport_height,
				centre_x,centre_y,size_x,size_y);
			ACCESS(Interaction_volume)(interaction_volume);
			interactive_event=CREATE(Interactive_event)(interactive_event_type,
				button_number,input_modifier,interaction_volume,scene_viewer->scene);
			ACCESS(Interactive_event)(interactive_event);
			return_code=Interactive_tool_handle_interactive_event(
				scene_viewer->interactive_tool,(void *)scene_viewer,interactive_event);
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

enum Scene_viewer_drag_mode
{
	SV_DRAG_NOTHING,
	SV_DRAG_TUMBLE,
	SV_DRAG_TRANSLATE,
	SV_DRAG_ZOOM
};

static int Scene_viewer_input_transform(struct Scene_viewer *scene_viewer,
	XEvent *event)
/*******************************************************************************
LAST MODIFIED : 15 October 1998

DESCRIPTION :
Converts mouse button-press and motion events into viewing transformations in
<scene_viewer>.
==============================================================================*/
{
	Dimension width,height;
	double near_x,near_y,near_z,far_x,far_y,far_z,dx,dy,dz;
	double old_near_x,old_near_y,old_near_z,old_far_x,old_far_y,old_far_z;
	double radius,fact,a[3],b[3],c[3],e[3],eye_distance,tangent_dist,d,phi,
		axis[3],angle;
	static enum Scene_viewer_drag_mode drag_mode=SV_DRAG_NOTHING;
	int return_code,pointer_x,pointer_y,i,delta_x,delta_y,view_changed;
	static int old_pointer_x,old_pointer_y;
	XButtonEvent *button_event;
	XKeyEvent *key_event;
	XMotionEvent *motion_event;
	/*
	int charcount;
	*/

	ENTER(Scene_viewer_input_transform);
	if (scene_viewer&&event)
	{
		return_code=1;
		switch (event->type)
		{
			case ButtonPress:
			{
				X3dThreeDDrawingMakeCurrent(scene_viewer->drawing_widget);
				button_event=&(event->xbutton);
				pointer_x=button_event->x;
				pointer_y=button_event->y;
				/* printf("button %d press at %d %d\n",button_event->button,
					pointer_x,pointer_y); */
				if (Scene_viewer_unproject(pointer_x,pointer_y,
					&near_x,&near_y,&near_z,&far_x,&far_y,&far_z))
				{
					/*printf("PRESS  Near: %8.4f %8.4f %8.4f  Far: %8.4f %8.4f %8.4f\n",
						near_x,near_y,near_z,far_x,far_y,far_z);*/
					switch (button_event->button)
					{
						case Button1:
						{
							if (0.0 != scene_viewer->tumble_rate)
							{
								drag_mode=SV_DRAG_TUMBLE;
							}
						} break;
						case Button2:
						{
							if (0.0 != scene_viewer->translate_rate)
							{
								drag_mode=SV_DRAG_TRANSLATE;
							}
						} break;
						case Button3:
						{
							if (0.0 != scene_viewer->zoom_rate)
							{
								drag_mode=SV_DRAG_ZOOM;
							}
						} break;
						default:
						{
						} break;
					}
					old_pointer_x=pointer_x;
					old_pointer_y=pointer_y;
				}
			} break;
			case MotionNotify:
			{
				motion_event= &(event->xmotion);
				pointer_x=motion_event->x;
				pointer_y=motion_event->y;
				/* printf("mouse move to %d %d\n",pointer_x,pointer_y); */
				if (Scene_viewer_unproject(pointer_x,pointer_y,
					&near_x,&near_y,&near_z,&far_x,&far_y,&far_z)&&
					Scene_viewer_unproject(old_pointer_x,old_pointer_y,
					&old_near_x,&old_near_y,&old_near_z,&old_far_x,&old_far_y,&old_far_z))
				{
					view_changed=0;
					switch (drag_mode)
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
							XtVaGetValues(scene_viewer->drawing_widget,
								XmNwidth,&width,XmNheight,&height,NULL);
							if ((0<width)&&(0<height))
							{
								/* get the radius of the ball */
								radius=0.25*(width+height);
								delta_x=pointer_x-old_pointer_x;
								delta_y=old_pointer_y-pointer_y;
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
									normalize3(c);
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
							if ((scene_viewer->far > scene_viewer->near)&&
								(eye_distance >= scene_viewer->near)&&
								(eye_distance <= scene_viewer->far))
							{
								fact = (eye_distance-scene_viewer->near)/
									(scene_viewer->far-scene_viewer->near);
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
							while (i>old_pointer_y)
							{
								radius /= fact;
								i--;
							}
							while (i<old_pointer_y)
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
						CALLBACK_LIST_CALL(Scene_viewer_transform)(
							scene_viewer->transform_callback_list,scene_viewer,NULL);
						CALLBACK_LIST_CALL(Scene_viewer_transform)(
							scene_viewer->sync_callback_list,scene_viewer,NULL);
					}
					old_pointer_x=pointer_x;
					old_pointer_y=pointer_y;
				}
			} break;
			case ButtonRelease:
			{
				button_event=&(event->xbutton);
				/* printf("button %d release at %d %d\n",button_event->button,
					button_event->x,button_event->y); */
				pointer_x=button_event->x;
				pointer_y=button_event->y;
				if (Scene_viewer_unproject(pointer_x,pointer_y,
					&near_x,&near_y,&near_z,&far_x,&far_y,&far_z))
				{
					/*printf("RELEASENear: %8.4f %8.4f %8.4f  Far: %8.4f %8.4f %8.4f\n",
						near_x,near_y,near_z,far_x,far_y,far_z);*/
				}
				drag_mode=SV_DRAG_NOTHING;
			} break;
			case KeyPress:
			{
				key_event= &(event->xkey);
				printf("key %u press at %d %d\n",key_event->keycode,key_event->x,
					key_event->y);
#if defined (OLD_CODE)
				charcount=XLookupString(key_event,buffer,bufsize,&keysym,&compose);
				switch (keysym)
				{
					case XK_Delete:
					{
						printf("* Delete pressed!\n");
					} break;
				}
#endif /* defined (OLD_CODE) */
			} break;
			case KeyRelease:
			{
				key_event= &(event->xkey);
				printf("key %u release at %d %d\n",key_event->keycode,key_event->x,
					key_event->y);
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
	struct Scene_viewer *scene_viewer,XEvent *event)
/*******************************************************************************
LAST MODIFIED : 19 December 1997

DESCRIPTION :
Converts mouse button-press and motion events into viewport zoom and translate
transformations.
==============================================================================*/
{
	double dx,dy,zoom_ratio,fact;
	static enum Scene_viewer_drag_mode drag_mode=SV_DRAG_NOTHING;
	int return_code,pointer_x,pointer_y,i;
	static int old_pointer_x,old_pointer_y;
	XButtonEvent *button_event;
	XKeyEvent *key_event;
	XMotionEvent *motion_event;
	/*
	int charcount;
	*/

	ENTER(Scene_viewer_input_viewport_transform);
	if (scene_viewer&&event)
	{
		return_code=1;
		switch (event->type)
		{
			case ButtonPress:
			{
				X3dThreeDDrawingMakeCurrent(scene_viewer->drawing_widget);
				button_event=&(event->xbutton);
				pointer_x=button_event->x;
				pointer_y=button_event->y;
				switch (button_event->button)
				{
					case Button2:
					{
						drag_mode=SV_DRAG_TRANSLATE;
					} break;
					case Button3:
					{
						drag_mode=SV_DRAG_ZOOM;
					} break;
					default:
					{
					} break;
				}
				old_pointer_x=pointer_x;
				old_pointer_y=pointer_y;
			} break;
			case MotionNotify:
			{
				motion_event= &(event->xmotion);
				pointer_x=motion_event->x;
				pointer_y=motion_event->y;
				switch (drag_mode)
				{
					case SV_DRAG_NOTHING:
					{
					} break;
					case SV_DRAG_TRANSLATE:
					{
						dx=(pointer_x-old_pointer_x)/
							scene_viewer->viewport_pixels_per_unit_x;
						dy=(old_pointer_y-pointer_y)/
							scene_viewer->viewport_pixels_per_unit_y;
						scene_viewer->viewport_left -= dx;
						scene_viewer->viewport_top -= dy;
						Scene_viewer_redraw(scene_viewer);
					} break;
					case SV_DRAG_ZOOM:
					{
						zoom_ratio=1.0;
						fact=1.01;
						i=pointer_y;
						while (i>old_pointer_y)
						{
							zoom_ratio *= fact;
							i--;
						}
						while (i<old_pointer_y)
						{
							zoom_ratio /= fact;
							i++;
						}
						Scene_viewer_viewport_zoom(scene_viewer,zoom_ratio);
						Scene_viewer_redraw(scene_viewer);
					} break;
					default:
					{
					} break;
				}
				old_pointer_x=pointer_x;
				old_pointer_y=pointer_y;
			} break;
			case ButtonRelease:
			{
				button_event=&(event->xbutton);
				/* printf("button %d release at %d %d\n",button_event->button,
					button_event->x,button_event->y); */
				pointer_x=button_event->x;
				pointer_y=button_event->y;
				drag_mode=SV_DRAG_NOTHING;
			} break;
			case KeyPress:
			{
				key_event= &(event->xkey);
				printf("key %u press at %d %d\n",key_event->keycode,key_event->x,
					key_event->y);
#if defined (OLD_CODE)
				charcount=XLookupString(key_event,buffer,bufsize,&keysym,&compose);
				switch (keysym)
				{
					case XK_Delete:
					{
						printf("* Delete pressed!\n");
					} break;
				}
#endif /* defined (OLD_CODE) */
			} break;
			case KeyRelease:
			{
				key_event= &(event->xkey);
				printf("key %u release at %d %d\n",key_event->keycode,key_event->x,
					key_event->y);
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

static void Scene_viewer_input_callback(Widget scene_viewer_widget,
	XtPointer scene_viewer_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 16 October 1998

DESCRIPTION :
The callback for mouse or keyboard input in the Scene_viewer window. The
resulting behaviour depends on the <scene_viewer> input_mode. In Transform mode
mouse clicks and drags are converted to transformation; in Select mode OpenGL
picking is performed with picked objects and mouse click and drag information
returned to the scene.
==============================================================================*/
{
	struct Scene_viewer *scene_viewer;
	XButtonEvent *button_event;
	XEvent *event;
	X3dThreeDDrawCallbackStruct *input_callback_data;

	ENTER(Scene_viewer_input_callback);
	if ((scene_viewer=(struct Scene_viewer *)scene_viewer_structure)&&
		(input_callback_data=(X3dThreeDDrawCallbackStruct *)call_data)&&
		(X3dCR_INPUT==input_callback_data->reason)&&
		(event=(XEvent *)(input_callback_data->event)))
	{
		X3dThreeDDrawingMakeCurrent(scene_viewer->drawing_widget);
#if defined (USING_3DFX_MESA)
#if !defined (XMESA_FX_WINDOW)
#define XMESA_FX_WINDOW 1
#endif /* !defined (XMESA_FX_WINDOW) */
#if !defined (XMESA_FX_FULLSCREEN)
#define XMESA_FX_FULLSCREEN 2
#endif /* !defined (XMESA_FX_FULLSCREEN) */
#if defined (DEBUG)
		/*???debug */
		printf("valid arguments\n");
#endif /* defined (DEBUG) */
		/* code to take advantage of the 3Dfx cards "full screen mode" */
		{
			typedef GLboolean (*XMesaSetFXmode_function)(GLint);
			static XMesaSetFXmode_function XMesaSetFXmode;
			static int first=1,full_screen=0;
			static KeyCode f_key_code=0,F_key_code=0;
			static void *library_handle;
			XKeyEvent *key_press;

			/* hot key is f/F */
			if (KeyPress==event->type)
			{
#if defined (DEBUG)
				/*???debug */
				printf("KeyPress\n");
#endif /* defined (DEBUG) */
				key_press=(XKeyEvent *)event;
				if (0==f_key_code)
				{
					f_key_code=XKeysymToKeycode(scene_viewer->user_interface->display,
						102);
					F_key_code=XKeysymToKeycode(scene_viewer->user_interface->display,
						70);
				}
				if ((f_key_code==key_press->keycode)||(f_key_code==key_press->keycode))
				{
					/*???debug */
					printf("Hot key\n");
					if (first)
					{
						/* check for required Mesa function */
						if (library_handle=dlopen("/usr/lib/libGL.so",RTLD_LAZY))
						{
							if (!(XMesaSetFXmode=(XMesaSetFXmode_function)dlsym(
								library_handle,"XMesaSetFXmode")))
							{
								display_message(ERROR_MESSAGE,
					"Scene_viewer_input_callback.  Could not locate XMesaSetFXmode .  %s",
									dlerror());
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Scene_viewer_input_callback.  Could not load libGL.so .  %s",
								dlerror());
						}
						first=0;
					}
					/*???debug */
					printf("XMesaSetFXmode %p, full_screen %d, XMESA_FX_WINDOW %d, XMESA_FX_WINDOW %d\n",
						XMesaSetFXmode,full_screen,XMESA_FX_WINDOW,XMESA_FX_FULLSCREEN);
					if (XMesaSetFXmode)
					{
						if (full_screen)
						{
							(*XMesaSetFXmode)(XMESA_FX_WINDOW);
							full_screen=0;
						}
						else
						{
							(*XMesaSetFXmode)(XMESA_FX_FULLSCREEN);
							full_screen=1;
						}
					}
				}
			}
		}
#endif /* defined (USING_3DFX_MESA) */
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
				if (ButtonPress==event->type)
				{
					button_event=&(event->xbutton);
					if (((ControlMask & button_event->state)&&
						(SCENE_VIEWER_ABSOLUTE_VIEWPORT != scene_viewer->viewport_mode))
						||((SCENE_VIEWER_ABSOLUTE_VIEWPORT == scene_viewer->viewport_mode)&&
						!((Button1==button_event->button)||
							(Button1Mask & button_event->state))))
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
					if (SCENE_VIEWER_RELATIVE_VIEWPORT == scene_viewer->viewport_mode)
					{
						if (SCENE_VIEWER_CUSTOM != scene_viewer->projection_mode)
						{
							Scene_viewer_input_transform(scene_viewer,event);
						}
					}
					else
					{
						Scene_viewer_input_viewport_transform(scene_viewer,event);
					}
				}
				else
				{
					/*???RC temporary until all tools are Interactive_tools */
					if (scene_viewer->interactive_tool)
					{
						Scene_viewer_input_select(scene_viewer,event);
					}
					else
					{
						Scene_viewer_input_select_old(scene_viewer,event);
					}
				}
			} break;
			case SCENE_VIEWER_UPDATE_ON_CLICK:
			case SCENE_VIEWER_TRANSFORM:
			{
				if (SCENE_VIEWER_UPDATE_ON_CLICK==scene_viewer->input_mode)
				{
					if (ButtonPress==event->type)
					{
						button_event=&(event->xbutton);
						if (button_event->state&ControlMask)
						{
							Scene_viewer_view_all(scene_viewer);
						}
					}
					scene_viewer->update_pixel_image=1;
					Scene_viewer_redraw(scene_viewer);
				}
				if (SCENE_VIEWER_RELATIVE_VIEWPORT==scene_viewer->viewport_mode)
				{
					if (SCENE_VIEWER_CUSTOM != scene_viewer->projection_mode)
					{
						Scene_viewer_input_transform(scene_viewer,event);
					}
				}
				else
				{
					Scene_viewer_input_viewport_transform(scene_viewer,event);
				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"Scene_viewer_input_callback.  Invalid input mode");
			} break;
		}
		if (scene_viewer->input_callback.procedure)
		{
			(scene_viewer->input_callback.procedure)(scene_viewer_widget,
				scene_viewer->input_callback.data,call_data);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_input_callback.  Invalid argument(s)");
	}
	LEAVE;
} /* Scene_viewer_input_callback */

/*
Manager Callback Module functions
---------------------------------
*/

static void Scene_viewer_light_change(
	struct MANAGER_MESSAGE(Light) *message,void *scene_viewer_void)
/*******************************************************************************
LAST MODIFIED : 18 November 1998

DESCRIPTION :
Something has changed globally in the light manager. If the modified light(s)
are in the scene, overlay_scene or the scene_viewer, then redraw.
==============================================================================*/
{
	struct Scene_viewer *scene_viewer;

	ENTER(Scene_viewer_light_change);
	if (message&&(scene_viewer=(struct Scene_viewer *)scene_viewer_void))
	{
		switch (message->change)
		{
			case MANAGER_CHANGE_ALL(Light):
			case MANAGER_CHANGE_OBJECT(Light):
			case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Light):
			{
				if (Scene_viewer_has_light(scene_viewer,message->object_changed)||
					Scene_has_light(scene_viewer->scene,message->object_changed)||
					(scene_viewer->overlay_scene&&
					Scene_has_light(scene_viewer->overlay_scene,message->object_changed)))
				{
					Scene_viewer_redraw(scene_viewer);
				}
			} break;
			case MANAGER_CHANGE_ADD(Light):
			case MANAGER_CHANGE_DELETE(Light):
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
	struct MANAGER_MESSAGE(Light_model) *message,void *scene_viewer_void)
/*******************************************************************************
LAST MODIFIED : 18 November 1998

DESCRIPTION :
Something has changed globally in the light_model manager. If the modified
light_model is used in the scene_viewer, then redraw.
==============================================================================*/
{
	struct Scene_viewer *scene_viewer;

	ENTER(Scene_viewer_light_model_change);
	if (message&&(scene_viewer=(struct Scene_viewer *)scene_viewer_void))
	{
		switch (message->change)
		{
			case MANAGER_CHANGE_ALL(Light_model):
			{
				if (scene_viewer->light_model)
				{
					Scene_viewer_redraw(scene_viewer);
				}
			} break;
			case MANAGER_CHANGE_OBJECT(Light_model):
			case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Light_model):
			{
				if (message->object_changed == scene_viewer->light_model)
				{
					Scene_viewer_redraw(scene_viewer);
				}
			} break;
			case MANAGER_CHANGE_ADD(Light_model):
			case MANAGER_CHANGE_DELETE(Light_model):
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
	struct MANAGER_MESSAGE(Scene) *message,void *scene_viewer_void)
/*******************************************************************************
LAST MODIFIED : 18 November 1998

DESCRIPTION :
Something has changed globally in the scene manager. If the contents of the
scene or overlay_scene in this scene_viewer are modified, then redraw.
==============================================================================*/
{
	struct Scene_viewer *scene_viewer;

	ENTER(Scene_viewer_scene_change);
	if (message&&(scene_viewer=(struct Scene_viewer *)scene_viewer_void))
	{
		switch (message->change)
		{
			case MANAGER_CHANGE_ALL(Scene):
			{
				Scene_viewer_redraw(scene_viewer);
			} break;
			case MANAGER_CHANGE_OBJECT(Scene):
			case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Scene):
			{
				if ((message->object_changed==scene_viewer->scene)||
					(message->object_changed==scene_viewer->overlay_scene))
				{
					Scene_viewer_redraw(scene_viewer);
				}
			} break;
			case MANAGER_CHANGE_ADD(Scene):
			case MANAGER_CHANGE_DELETE(Scene):
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
LAST MODIFIED : 18 November 1998

DESCRIPTION :
Something has changed globally in the texture manager. If a modified texture is
used in this scene_viewer, then redraw.
==============================================================================*/
{
	struct Scene_viewer *scene_viewer;

	ENTER(Scene_viewer_texture_change);
	if (message&&(scene_viewer=(struct Scene_viewer *)scene_viewer_void))
	{
		switch (message->change)
		{
			case MANAGER_CHANGE_ALL(Texture):
			{
				if (scene_viewer->background_texture)
				{
					Scene_viewer_redraw(scene_viewer);
				}
			} break;
			case MANAGER_CHANGE_OBJECT(Texture):
			case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Texture):
			{
				if (message->object_changed == scene_viewer->background_texture)
				{
					Scene_viewer_redraw(scene_viewer);
				}
			} break;
			case MANAGER_CHANGE_ADD(Texture):
			case MANAGER_CHANGE_DELETE(Texture):
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
struct Scene_viewer *CREATE(Scene_viewer)(Widget parent,
	struct Colour *background_colour,enum Scene_viewer_buffer_mode buffer_mode,
	struct MANAGER(Light) *light_manager,struct Light *default_light,
	struct MANAGER(Light_model) *light_model_manager,
	struct Light_model *default_light_model,
	struct MANAGER(Scene) *scene_manager,struct Scene *scene,
	struct MANAGER(Texture) *texture_manager,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 11 April 2000

DESCRIPTION :
Creates a Scene_viewer in the widget <parent> to display <scene>.
Note: the parent must be an XmForm since form constraints will be applied.
If any of light_manager, light_model_manager, scene_manager or texture_manager
are supplied, the scene_viewer will automatically redraw in response to changes
of objects from these managers that are in use by the scene_viewer. Redraws are
performed in idle time so that multiple redraws are avoided.
==============================================================================*/
{
	int return_code,i;
	struct Scene_viewer *scene_viewer;
	Widget drawing_widget;
	X3dBufferingMode x3d_buffering_mode;

	ENTER(CREATE(Scene_viewer));
	if (background_colour&&default_light_model&&scene&&user_interface)
	{
		return_code=1;
		switch (buffer_mode)
		{
			case SCENE_VIEWER_SINGLE_BUFFER:
			{
				x3d_buffering_mode=X3dSINGLE_BUFFERING;
			} break;
			case SCENE_VIEWER_PIXEL_BUFFER:
			case SCENE_VIEWER_DOUBLE_BUFFER:
			{
				x3d_buffering_mode=X3dDOUBLE_BUFFERING;
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"CREATE(Scene_viewer).  Invalid buffering mode");
				return_code=0;
			}
		}
		if (return_code)
		{
			/* allocate memory for the scene_viewer structure */
			if (ALLOCATE(scene_viewer,struct Scene_viewer,1)&&
				(scene_viewer->list_of_lights=CREATE(LIST(Light)())))
			{
				/* create the Three_D_drawing widget */
				if (drawing_widget=XtVaCreateWidget("cm_scene_viewer_area",
					threeDDrawingWidgetClass,parent,
					X3dNbufferColourMode,X3dCOLOUR_RGB_MODE,
					X3dNbufferingMode,x3d_buffering_mode,
					XmNleftAttachment,XmATTACH_FORM,
					XmNrightAttachment,XmATTACH_FORM,
					XmNbottomAttachment,XmATTACH_FORM,
					XmNtopAttachment,XmATTACH_FORM,
					XmNborderWidth,0,
					XmNleftOffset,0,
					XmNrightOffset,0,
					XmNbottomOffset,0,
					XmNtopOffset,0,
					NULL))
				{
					/* access the scene, since don't want it to disappear */
					scene_viewer->scene=ACCESS(Scene)(scene);
					scene_viewer->overlay_scene=(struct Scene *)NULL;
					scene_viewer->input_mode=SCENE_VIEWER_TRANSFORM;
					scene_viewer->temporary_transform_mode=0;
					scene_viewer->parent=parent;
					scene_viewer->user_interface=user_interface;
					scene_viewer->idle_update_proc=(XtWorkProcId)NULL;
					(scene_viewer->background_colour).red=background_colour->red;
					(scene_viewer->background_colour).green=background_colour->green;
					(scene_viewer->background_colour).blue=background_colour->blue;
					scene_viewer->background_texture=(struct Texture *)NULL;
					scene_viewer->drawing_widget=drawing_widget;
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
					scene_viewer->near=0.1;
					scene_viewer->far=1000.0;
					scene_viewer->projection_mode=SCENE_VIEWER_PARALLEL;
					scene_viewer->translate_rate=1.0;
					scene_viewer->tumble_rate=1.5;
					scene_viewer->zoom_rate=1.0;
					scene_viewer->light_model=ACCESS(Light_model)(default_light_model);
					scene_viewer->antialias=0;
					scene_viewer->perturb_lines=0;
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
					scene_viewer->viewport_top=0.0;
					scene_viewer->viewport_left=0.0;
					scene_viewer->viewport_pixels_per_unit_x=1.0;
					scene_viewer->viewport_pixels_per_unit_y=1.0;
					scene_viewer->background_texture=(struct Texture *)NULL;
					scene_viewer->bk_texture_top=0.0;
					scene_viewer->bk_texture_left=0.0;
					scene_viewer->bk_texture_width=1.0;
					scene_viewer->bk_texture_height=1.0;
					/* by default, use undistort stuff on textures */
					scene_viewer->bk_texture_undistort_on=1;
					scene_viewer->bk_texture_max_pixels_per_polygon=16.0;
					scene_viewer->buffer_mode=buffer_mode;
					scene_viewer->transparency_mode=SCENE_VIEWER_FAST_TRANSPARENCY;
					scene_viewer->input_callback.procedure=(Callback_procedure *)NULL;
					scene_viewer->input_callback.data=NULL;
					scene_viewer->sync_callback_list=
					  CREATE(LIST(CALLBACK_ITEM(Scene_viewer_transform)))();
					scene_viewer->transform_callback_list=
					  CREATE(LIST(CALLBACK_ITEM(Scene_viewer_transform)))();
					scene_viewer->pixel_width=0;
					scene_viewer->pixel_height=0;
					scene_viewer->update_pixel_image=0;
					if (SCENE_VIEWER_PIXEL_BUFFER==buffer_mode )
					{
						ALLOCATE(scene_viewer->pixel_data,char,1);
					}
					/* add callbacks to the drawing widget */
					XtAddCallback(drawing_widget,X3dNinitializeCallback,
						Scene_viewer_initialize_callback,scene_viewer);
					XtAddCallback(drawing_widget,X3dNresizeCallback,
						Scene_viewer_resize_callback,scene_viewer);
					XtAddCallback(drawing_widget,X3dNexposeCallback,
						Scene_viewer_expose_callback,scene_viewer);
					XtAddCallback(drawing_widget,X3dNinputCallback,
						Scene_viewer_input_callback,scene_viewer);
					/* register for lighting changes */
					if (light_manager)
					{
						scene_viewer->light_manager_callback_id=
							MANAGER_REGISTER(Light)(Scene_viewer_light_change,
								(void *)scene_viewer,scene_viewer->light_manager);
					}
					if (light_model_manager)
					{
						scene_viewer->light_model_manager_callback_id=
						MANAGER_REGISTER(Light_model)(Scene_viewer_light_model_change,
							(void *)scene_viewer,scene_viewer->light_model_manager);
					}
					/* register for any scene changes */
					if (scene_manager)
					{
						scene_viewer->scene_manager_callback_id=
							MANAGER_REGISTER(Scene)(Scene_viewer_scene_change,
								(void *)scene_viewer,scene_viewer->scene_manager);
					}
					/* register for any texture changes */
					if (texture_manager)
					{
						scene_viewer->texture_manager_callback_id=
							MANAGER_REGISTER(Texture)(Scene_viewer_texture_change,
								(void *)scene_viewer,scene_viewer->texture_manager);
					}
					XtManageChild(drawing_widget);
				}
				else
				{
					DESTROY(LIST(Light))(&(scene_viewer->list_of_lights));
					DEALLOCATE(scene_viewer);
					display_message(ERROR_MESSAGE,"CREATE(Scene_viewer).  "
						"Could not create the scene_viewer area widget");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"CREATE(Scene_viewer).  Not enought memory for scene_viewer");
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
LAST MODIFIED : 18 November 1998

DESCRIPTION :
Closes the scene_viewer and disposes of the scene_viewer data structure.
==============================================================================*/
{
	int return_code;
	struct Scene_viewer *scene_viewer;

	ENTER(DESTROY(Scene_viewer));
	if (scene_viewer_address&&(scene_viewer= *scene_viewer_address))
	{
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
		/* if there's an update pending, then remove the workproc from the queue */
		/*???DB.  Is a workproc really necessary ? */
		if (scene_viewer->idle_update_proc)
		{
			XtRemoveWorkProc(scene_viewer->idle_update_proc);
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
			DESTROY(LIST(CALLBACK_ITEM(Scene_viewer_transform)))(
				&scene_viewer->sync_callback_list);
		}
		if (scene_viewer->transform_callback_list)
		{
			DESTROY(LIST(CALLBACK_ITEM(Scene_viewer_transform)))(
				&scene_viewer->transform_callback_list);
		}
		/* must destroy the widget */
		XtDestroyWidget(scene_viewer->drawing_widget);				
		if (SCENE_VIEWER_PIXEL_BUFFER==scene_viewer->buffer_mode)
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

#if defined (OLD_CODE)
int Scene_viewer_get_aspect(struct Scene_viewer *scene_viewer,double *aspect)
/*******************************************************************************
LAST MODIFIED : 21 November 1997

DESCRIPTION :
Returns the Scene_viewer aspect; see struct Scene_viewer for definition.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_get_aspect);
	if (scene_viewer&&aspect)
	{
		*aspect=scene_viewer->aspect;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_aspect.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_get_aspect */

int Scene_viewer_set_aspect(struct Scene_viewer *scene_viewer,double aspect)
/*******************************************************************************
LAST MODIFIED : 21 November 1997

DESCRIPTION :
Sets the Scene_viewer aspect; see struct Scene_viewer for definition.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_set_aspect);
	if (scene_viewer&&(0.0<aspect))
	{
		scene_viewer->aspect=aspect;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_set_aspect.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_set_aspect */
#endif /* defined (OLD_CODE) */

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
	Dimension border_width;

	ENTER(Scene_viewer_get_border_width);
	if (scene_viewer)
	{
		XtVaGetValues(scene_viewer->drawing_widget,
			XmNborderWidth,&border_width,NULL);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_border_width.  Missing scene_viewer");
		border_width=0;
	}
	LEAVE;

	return ((int)border_width);
} /* Scene_viewer_get_border_width */

int Scene_viewer_set_border_width(struct Scene_viewer *scene_viewer,
	int border_width)
/*******************************************************************************
LAST MODIFIED : 12 October 1998

DESCRIPTION :
Sets the border_width of the scene_viewer widget. Note that the border is
only shown on the bottom and right of each viewer in the graphics window.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_set_border_width);
	if (scene_viewer&&(0<=border_width))
	{
		/*XtVaSetValues(scene_viewer->drawing_widget,
			XmNborderWidth,(Dimension)border_width,NULL);*/
		XtVaSetValues(scene_viewer->drawing_widget,
			XmNleftOffset,(Dimension)border_width,
			XmNrightOffset,(Dimension)border_width,
			XmNbottomOffset,(Dimension)border_width,
			XmNtopOffset,(Dimension)border_width,NULL);
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

enum Scene_viewer_buffer_mode Scene_viewer_get_buffer_mode(
	struct Scene_viewer *scene_viewer)
/*******************************************************************************
LAST MODIFIED : 15 October 1998

DESCRIPTION :
Returns the buffer mode - single_buffer/double_buffer - of the Scene_viewer.
==============================================================================*/
{
	enum Scene_viewer_buffer_mode buffer_mode;

	ENTER(Scene_viewer_get_buffer_mode);
	if (scene_viewer)
	{
		buffer_mode=scene_viewer->buffer_mode;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_buffer_mode.  Invalid argument(s)");
		/* return any valid mode */
		buffer_mode=SCENE_VIEWER_DOUBLE_BUFFER;
	}
	LEAVE;

	return (buffer_mode);
} /* Scene_viewer_get_buffer_mode */

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
LAST MODIFIED : 25 July 1998

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
Special version of Scene_viewer_set_lookat_parameters that ensures the up vector
is orthogonal to the view direction - so prejection is not skew.
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
	int return_code,i;

	ENTER(Scene_viewer_get_modelview_matrix);
	if (scene_viewer&&modelview_matrix)
	{
		for (i=0;i<16;i++)
		{
			modelview_matrix[i]=scene_viewer->modelview_matrix[i];
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
	int return_code,i;

	ENTER(Scene_viewer_set_modelview_matrix);
	if (scene_viewer&&modelview_matrix)
	{
		if (SCENE_VIEWER_CUSTOM==scene_viewer->projection_mode)
		{
			for (i=0;i<16;i++)
			{
				scene_viewer->modelview_matrix[i]=modelview_matrix[i];
			}
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

enum Scene_viewer_projection_mode Scene_viewer_get_projection_mode(
	struct Scene_viewer *scene_viewer)
/*******************************************************************************
LAST MODIFIED : 5 October 1998

DESCRIPTION :
Returns the projection mode - parallel/perspective - of the Scene_viewer.
==============================================================================*/
{
	enum Scene_viewer_projection_mode projection_mode;

	ENTER(Scene_viewer_get_projection_mode);
	if (scene_viewer)
	{
		projection_mode=scene_viewer->projection_mode;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_projection_mode.  Invalid argument(s)");
		projection_mode=SCENE_VIEWER_PARALLEL;
	}
	LEAVE;

	return (projection_mode);
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
	int return_code,i;

	ENTER(Scene_viewer_get_projection_matrix);
	if (scene_viewer&&projection_matrix)
	{
		for (i=0;i<16;i++)
		{
			projection_matrix[i]=scene_viewer->projection_matrix[i];
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
	int return_code,i;

	ENTER(Scene_viewer_set_projection_matrix);
	if (scene_viewer&&projection_matrix)
	{
		if (SCENE_VIEWER_CUSTOM==scene_viewer->projection_mode)
		{
			for (i=0;i<16;i++)
			{
				scene_viewer->projection_matrix[i]=projection_matrix[i];
			}
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

int Scene_viewer_get_transform_rate(struct Scene_viewer *scene_viewer,
	double *translate_rate,double *tumble_rate,double *zoom_rate)
/*******************************************************************************
LAST MODIFIED : 15 October 1998

DESCRIPTION :
Returns the rate of translate, tumble and zoom transformations in
relation to mouse movements. Values of around 1.0 could be considered normal.
A value of zero turns off that transform capability.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_get_transform_rate);
	if (scene_viewer&&translate_rate&&tumble_rate&&zoom_rate)
	{
		*translate_rate=scene_viewer->translate_rate;
		*tumble_rate=scene_viewer->tumble_rate;
		*zoom_rate=scene_viewer->zoom_rate;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_transform_rate.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_get_transform_rate */

int Scene_viewer_set_transform_rate(struct Scene_viewer *scene_viewer,
	double translate_rate,double tumble_rate,double zoom_rate)
/*******************************************************************************
LAST MODIFIED : 15 October 1998

DESCRIPTION :
Sets the rate of translate, tumble and zoom transformations in
relation to mouse movements. Values of around 1.0 could be considered normal,
although I use 1.5 for the default tumble_rate.
A value of zero turns off that transform capability.
Negative values reverse the effects of mouse movement.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_set_transform_rate);
	if (scene_viewer)
	{
		scene_viewer->translate_rate=translate_rate;
		scene_viewer->tumble_rate=tumble_rate;
		scene_viewer->zoom_rate=zoom_rate;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_set_transform_rate.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_set_transform_rate */

enum Scene_viewer_transparency_mode Scene_viewer_get_transparency_mode(
	struct Scene_viewer *scene_viewer)
/*******************************************************************************
LAST MODIFIED : 23 November 1998

DESCRIPTION :
See Scene_viewer_set_transparency_mode for explanation.
==============================================================================*/
{
	enum Scene_viewer_transparency_mode transparency_mode;

	ENTER(Scene_viewer_get_transparency_mode);
	if (scene_viewer)
	{
		transparency_mode=scene_viewer->transparency_mode;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_transparency_mode.  Invalid argument(s)");
		/* return any valid value */
		transparency_mode=SCENE_VIEWER_FAST_TRANSPARENCY;
	}
	LEAVE;

	return (transparency_mode);
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
		(SCENE_VIEWER_LAYERED_TRANSPARENCY==transparency_mode)))
	{
		scene_viewer->transparency_mode=transparency_mode;
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

int Scene_viewer_get_transparency_layers(struct Scene_viewer *scene_viewer)
/*******************************************************************************
LAST MODIFIED : 9 October 1999

DESCRIPTION :
See Scene_viewer_set_transparency_layers for explanation.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_get_transparency_layers);
	if (scene_viewer)
	{
		return_code=scene_viewer->transparency_layers;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_transparency_layers.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_get_transparency_layers */

int Scene_viewer_set_transparency_layers(struct Scene_viewer *scene_viewer,
	int layers)
/*******************************************************************************
LAST MODIFIED : 9 October 1999

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
		scene_viewer->transparency_layers=layers;
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
LAST MODIFIED : 13 October 1998

DESCRIPTION :
Gets the diagonal view angle, in radians, of the <scene_viewer>.
For PARALLEL and PERSPECTIVE projection modes only.
==============================================================================*/
{
	double diagonal,eye_distance,view[3];
	int return_code;

	ENTER(Scene_viewer_get_view_angle);
	if (scene_viewer&&view_angle&&(
		(SCENE_VIEWER_PARALLEL==scene_viewer->projection_mode)||
		(SCENE_VIEWER_PERSPECTIVE==scene_viewer->projection_mode)))
	{
		diagonal=sqrt((scene_viewer->right-scene_viewer->left)*
			(scene_viewer->right-scene_viewer->left)+
			(scene_viewer->top-scene_viewer->bottom)*
			(scene_viewer->top-scene_viewer->bottom));
		view[0]=scene_viewer->eyex - scene_viewer->lookatx;
		view[1]=scene_viewer->eyey - scene_viewer->lookaty;
		view[2]=scene_viewer->eyez - scene_viewer->lookatz;
		eye_distance=normalize3(view);
		*view_angle=2.0*atan(diagonal/(2.0*eye_distance));
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_get_view_angle.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_get_view_angle */

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
		scene_viewer->far=eye_distance+clip_distance;
		if (clip_distance>=eye_distance)
		{
			scene_viewer->near=0.01*eye_distance;
		}
		else
		{
			scene_viewer->near=eye_distance-clip_distance;
		}
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
	double *left,double *right,double *bottom,double *top,double *near,
	double *far)
/*******************************************************************************
LAST MODIFIED : 21 November 1997

DESCRIPTION :
Gets the viewing volume of the Scene_viewer.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_get_viewing_volume);
	if (scene_viewer&&left&&right&&bottom&&top&&near&&far)
	{
		*left=scene_viewer->left;
		*right=scene_viewer->right;
		*bottom=scene_viewer->bottom;
		*top=scene_viewer->top;
		*near=scene_viewer->near;
		*far=scene_viewer->far;
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
	double left,double right,double bottom,double top,double near,double far)
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
		if ((right>left)&&(top>bottom)&&(0<near)&&(near<far))
		{
			scene_viewer->left=left;
			scene_viewer->right=right;
			scene_viewer->bottom=bottom;
			scene_viewer->top=top;
			scene_viewer->near=near;
			scene_viewer->far=far;
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
		*viewport_left=scene_viewer->viewport_left;
		*viewport_top=scene_viewer->viewport_top;
		*viewport_pixels_per_unit_x=scene_viewer->viewport_pixels_per_unit_x;
		*viewport_pixels_per_unit_y=scene_viewer->viewport_pixels_per_unit_y;
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
		scene_viewer->viewport_left=viewport_left;
		scene_viewer->viewport_top=viewport_top;
		scene_viewer->viewport_pixels_per_unit_x=viewport_pixels_per_unit_x;
		scene_viewer->viewport_pixels_per_unit_y=viewport_pixels_per_unit_y;
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
	int *antialias)
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
	int antialias_mode)
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
LAST MODIFIED : 13 June 2000

DESCRIPTION :
When the line draw mode is turned on (set to one) the lines are raised in the
z direction when the GL_EXT_polygon_offset extension is available from the X
Server.  This means that the lines appear solid rather than interfering with a
surface in the same space.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_set_perturb_lines);
	if (scene_viewer)
	{
		if (perturb_lines)
		{
			if (query_gl_extension("GL_EXT_polygon_offset"))
			{
				scene_viewer->perturb_lines=1;
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,"Scene_viewer_set_perturb_lines.  "
					"GL_EXT_polygon_offset extension not available on this display");
				return_code=0;
			}
		}
		else
		{
			scene_viewer->perturb_lines=0;
			return_code=1;
		}
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
LAST MODIFIED : 14 October 1998

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
		(SCENE_VIEWER_ABSOLUTE_VIEWPORT==viewport_mode)))
	{
		scene_viewer->viewport_mode=viewport_mode;
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
	Dimension *width,Dimension *height)
/*******************************************************************************
LAST MODIFIED : 21 January 1998

DESCRIPTION :
Returns the width and height of the Scene_viewers drawing area.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_get_viewport_size);
	if (scene_viewer&&width&&height)
	{
		XtVaGetValues(scene_viewer->drawing_widget,
			XmNwidth,width,
			XmNheight,height,NULL);
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
	Dimension width,Dimension height)
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
		XtVaSetValues(scene_viewer->drawing_widget,
			XmNwidth,width,
			XmNheight,height,NULL);
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

int Scene_viewer_get_window_projection_matrix(struct Scene_viewer *scene_viewer,
	double window_projection_matrix[16])
/*******************************************************************************
LAST MODIFIED : 18 May 1998

DESCRIPTION :
Returns the actual projection matrix applied to fill the window.
==============================================================================*/
{
	int return_code,i;

	ENTER(Scene_viewer_get_window_projection_matrix);
	if (scene_viewer&&window_projection_matrix)
	{
		for (i=0;i<16;i++)
		{
			window_projection_matrix[i]=scene_viewer->window_projection_matrix[i];
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
LAST MODIFIED : 16 October 1998

DESCRIPTION :
Call this after changing viewing parameters or display lists to have the
Scene_viewer redraw the image. Does this by putting a WorkProc on the queue
(if not already done for this Scene_viewer) which will force a redraw at the
next idle moment. If the scene_viewer is changed again before it is updated,
a new WorkProc will not be put in the queue, but the old one will update the
window to the new state.
???RC Why not draw it directly?
???RC Maybe have a direct redraw and an idle redraw.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_redraw);
	if (scene_viewer)
	{
		if (!scene_viewer->idle_update_proc)
		{
			scene_viewer->idle_update_proc=XtAppAddWorkProc(
				scene_viewer->user_interface->application_context,
				Scene_viewer_idle_update,scene_viewer);
		}
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
LAST MODIFIED : 16 October 1998

DESCRIPTION :
Forces a redraw of the given scene viewer to take place immediately - ie.
not just at the next idle moment as Scene_viewer_redraw does.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_redraw_now);
	if (scene_viewer)
	{
		/* remove idle update workproc if pending */
		if (scene_viewer->idle_update_proc)
		{
			XtRemoveWorkProc(scene_viewer->idle_update_proc);
			scene_viewer->idle_update_proc=(XtWorkProcId)NULL;
		}
		X3dThreeDDrawingMakeCurrent(scene_viewer->drawing_widget);
		return_code=Scene_viewer_render_scene(scene_viewer);
		X3dThreeDDrawingSwapBuffers();
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
extensions can get the undated frame from the backbuffer.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_redraw_now_without_swapbuffers);
	if (scene_viewer)
	{
		X3dThreeDDrawingMakeCurrent(scene_viewer->drawing_widget);
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
	if (scene_viewer&&(SCENE_VIEWER_PIXEL_BUFFER==scene_viewer->buffer_mode))
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
	if (scene_viewer&&(SCENE_VIEWER_PIXEL_BUFFER==scene_viewer->buffer_mode)&&
		height&&width&&data)
	{
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
LAST MODIFIED : 19 October 1998

DESCRIPTION :
Finds the x, y and z ranges from the scene and sets the view parameters so
that everything can be seen, and with window's std_view_angle. Also adjusts
near and far clipping planes; if specific values are required, should follow
with commands for setting these.
==============================================================================*/
{
	double centre_x,centre_y,centre_z,clip_factor,radius,
		size_x,size_y,size_z,width_factor;
	int return_code;

	ENTER(Scene_viewer_view_all);
	if (scene_viewer)
	{	
		if (Scene_get_graphics_range(Scene_viewer_get_scene(scene_viewer),
			&centre_x,&centre_y,&centre_z,&size_x,&size_y,&size_z)&&			
			(radius=0.5*sqrt(size_x*size_x + size_y*size_y + size_z*size_z)))
		{		
			/* enlarge radius to keep image within edge of window */
			/*???RC width_factor should be read in from defaults file */
			width_factor=1.05;
			radius *= width_factor;
			/*???RC clip_factor should be read in from defaults file: */
			clip_factor = 10.0;		
			return_code=Scene_viewer_set_view_simple(scene_viewer
				,centre_x,centre_y,centre_z,
				radius,40,clip_factor*radius);		
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Scene_viewer_view_all.  No valid range obtained from scene");
			return_code=0;
		}
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
	Dimension height,width; /* X widget dimensions */
	int return_code;

	ENTER(Scene_viewer_viewport_zoom);
	if (scene_viewer&&(0.0<zoom_ratio))
	{
		scene_viewer->viewport_pixels_per_unit_x *= zoom_ratio;
		scene_viewer->viewport_pixels_per_unit_y *= zoom_ratio;
		/* adjust top,left so that zoom comes from centre of viewport */
		XtVaGetValues(scene_viewer->drawing_widget,
			XmNwidth,&width,XmNheight,&height,NULL);
		scene_viewer->viewport_left += 0.5*(zoom_ratio-1.0)*
			(width/scene_viewer->viewport_pixels_per_unit_x);
		scene_viewer->viewport_top -= 0.5*(zoom_ratio-1.0)*
			(height/scene_viewer->viewport_pixels_per_unit_y);
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
LAST MODIFIED : 26 April 2000

DESCRIPTION :
Sets the interactive tool that will receive input if the Scene_viewer is in
SCENE_VIEWER_SELECT mode. A NULL value indicates no tool.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_set_interactive_tool);
	if (scene_viewer)
	{
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

int Scene_viewer_set_input_callback(struct Scene_viewer *scene_viewer,
	struct Callback_data *callback)
/*******************************************************************************
LAST MODIFIED : 25 July 1998

DESCRIPTION :
Sets a callback that will be activated each time input is received by the 
scene_viewer.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_set_input_callback);
	if (scene_viewer&&callback)
	{
		scene_viewer->input_callback.procedure=callback->procedure;
		scene_viewer->input_callback.data=callback->data;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_set_input_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_set_input_callback */

int Scene_viewer_sync_add_callback(struct Scene_viewer *scene_viewer,
	CALLBACK_FUNCTION(Scene_viewer_transform) *function,void *user_data)
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
Adds a callback to <element_selection> so that when it changes <function> is
called with <user_data>. <function> has 3 arguments, a
struct Scene_viewer *, a void* and the void *user_data.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_sync_add_callback);
	if (scene_viewer&&function)
	{
		if (CALLBACK_LIST_ADD_CALLBACK(Scene_viewer_transform)(
			scene_viewer->sync_callback_list,function,user_data))
		{
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Scene_viewer_sync_add_callback.  Could not add callback");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_sync_add_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_sync_add_callback */

int Scene_viewer_sync_remove_callback(struct Scene_viewer *scene_viewer,
	CALLBACK_FUNCTION(Scene_viewer_transform) *function,void *user_data)
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
Removes the callback calling <function> with <user_data> from
<scene_viewer>.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_sync_remove_callback);
	if (scene_viewer&&function)
	{
		if (CALLBACK_LIST_REMOVE_CALLBACK(Scene_viewer_transform)(
			scene_viewer->sync_callback_list,function,user_data))
		{
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Scene_viewer_sync_remove_callback.  Could not remove callback");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_sync_remove_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_sync_remove_callback */

int Scene_viewer_transform_add_callback(struct Scene_viewer *scene_viewer,
	CALLBACK_FUNCTION(Scene_viewer_transform) *function,void *user_data)
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
Adds a callback to <element_selection> so that when it changes <function> is
called with <user_data>. <function> has 3 arguments, a
struct Scene_viewer *, a void* and the void *user_data.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_transform_add_callback);
	if (scene_viewer&&function)
	{
		if (CALLBACK_LIST_ADD_CALLBACK(Scene_viewer_transform)(
			scene_viewer->transform_callback_list,function,user_data))
		{
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Scene_viewer_transform_add_callback.  Could not add callback");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_transform_add_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_transform_add_callback */

int Scene_viewer_transform_remove_callback(struct Scene_viewer *scene_viewer,
	CALLBACK_FUNCTION(Scene_viewer_transform) *function,void *user_data)
/*******************************************************************************
LAST MODIFIED : 5 July 2000

DESCRIPTION :
Removes the callback calling <function> with <user_data> from
<scene_viewer>.
==============================================================================*/
{
	int return_code;

	ENTER(Scene_viewer_transform_remove_callback);
	if (scene_viewer&&function)
	{
		if (CALLBACK_LIST_REMOVE_CALLBACK(Scene_viewer_transform)(
			scene_viewer->transform_callback_list,function,user_data))
		{
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Scene_viewer_transform_remove_callback.  Could not remove callback");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_viewer_transform_remove_callback.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_viewer_transform_remove_callback */

char *Scene_viewer_buffer_mode_string(
	enum Scene_viewer_buffer_mode buffer_mode)
/*******************************************************************************
LAST MODIFIED : 14 October 1998

DESCRIPTION :
Returns a string label for the <buffer_mode>.
NOTE: Calling function must not deallocate returned string.
==============================================================================*/
{
	char *return_string;

	ENTER(Scene_viewer_buffer_mode_string);
	switch (buffer_mode)
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
				"Scene_viewer_buffer_mode_string.  Unknown buffer mode");
			return_string=(char *)NULL;
		}
	}
	LEAVE;

	return (return_string);
} /* Scene_viewer_buffer_mode_string */

char *Scene_viewer_input_mode_string(
	enum Scene_viewer_input_mode input_mode)
/*******************************************************************************
LAST MODIFIED : 14 October 1998

DESCRIPTION :
Returns a string label for the <input_mode>.
NOTE: Calling function must not deallocate returned string.
==============================================================================*/
{
	char *return_string;

	ENTER(Scene_viewer_input_mode_string);
	switch (input_mode)
	{
		case SCENE_VIEWER_NO_INPUT_OR_DRAW:
		{
			return_string="no_input_or_draw";
		} break;
		case SCENE_VIEWER_UPDATE_ON_CLICK:
		{
			return_string="update_on_click";
		} break;
		case SCENE_VIEWER_NO_INPUT:
		{
			return_string="no_input";
		} break;
		case SCENE_VIEWER_SELECT:
		{
			return_string="select";
		} break;
		case SCENE_VIEWER_TRANSFORM:
		{
			return_string="transform";
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"Scene_viewer_input_mode_string.  Unknown input mode");
			return_string=(char *)NULL;
		}
	}
	LEAVE;

	return (return_string);
} /* Scene_viewer_input_mode_string */

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
LAST MODIFIED : 23 November 1998

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
LAST MODIFIED : 14 October 1998

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
