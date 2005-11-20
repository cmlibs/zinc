/*******************************************************************************
FILE : input_module_dialog.c

LAST MODIFIED : 23 November 2001

DESCRIPTION :
Brings up a window which holds a data_grabber.  Allows the user to change what
data is accepted - pos,tangent,normal.
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
#include <Xm/List.h>
#include <Xm/ToggleBG.h>
#include <math.h>
#include <stdio.h>
#include "choose/choose_scene.h"
#include "choose/choose_graphical_material.h"
#include "dof3/dof3.h"
#include "general/compare.h"
#include "general/debug.h"
#include "general/list_private.h"
#include "general/manager.h"
#include "general/simple_list.h"
#include "graphics/graphics_object.h"
#include "graphics/scene.h"
#include "io_devices/input_module_dialog.h"
#if defined (EXT_INPUT)
#include "io_devices/input_module_dialog.uidh"
#endif /* defined (EXT_INPUT) */
#include "io_devices/input_module_widget.h"
#include "io_devices/input_module.h"
#include "io_devices/matrix.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"


/* SAB Trying to hide the guts of GT_object and its primitives,
	however the stream point stuff currently messes around in the guts
	of a pointset. */
#include "graphics/graphics_object_private.h"

#if defined (EXT_INPUT)

/*
UIL Identifiers
---------------
*/
#define input_module_chooser_form_ID (101)
#define input_module_data_rowcol_ID (102)
#define input_module_scale_frame_ID (103)
#define input_module_offset_frame_ID (104)
#define input_module_form_label_ID (105)
#define input_module_current_x_ID (106)
#define input_module_current_y_ID (107)
#define input_module_current_z_ID (108)
#define input_module_display_change (109)
#define input_module_scene_chooser_ID (110)
#define input_module_material_ID (111)
#define input_module_display_control_ID (112)
#define input_module_set_origin (113)
#define input_module_calibrate (114)
#define input_module_cross (115)
#define input_module_pixel (116)
#define input_module_sphere (117)
#define input_module_calib_control_ID (118)
#define input_module_calib_scroll_ID (119)
#define input_module_calib_cont_record_ID (120)
#define input_module_calib_cont_clear_ID (121)
#define input_module_calib_cont_solve_ID (122)
#define input_module_calib_cont_delete_ID (123)
#define input_module_faro_control_ID (124)
#define input_module_faro_scroll_ID (125)
#define input_module_faro_three_point_ID (126)

/*
Module types
------------
*/

enum Input_module_form_mode
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
The components of the position that are required by the user.
==============================================================================*/
{
	INPUT_MODULE_NO_FORMS = 0,
	INPUT_MODULE_DIALOG_POSITION=1,
	INPUT_MODULE_DIALOG_NORMAL=2,
	INPUT_MODULE_DIALOG_TANGENT=4
}; /* Input_module_form_mode */

enum Input_module_data_type
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Contains the different types of callbacks that are valid for the input_module_dialog
control widget.
==============================================================================*/
{
	INPUT_MODULE_DIALOG_UPDATE_CB,
	INPUT_MODULE_DIALOG_SELECT_CB,
	INPUT_MODULE_DIALOG_DATA,
	INPUT_MODULE_DIALOG_MODE
}; /* Input_module_data_type */

enum Input_module_scene_display
/*******************************************************************************
LAST MODIFIED : 24 February 1998

DESCRIPTION :
Describes how the current input device is represented on the selected scene
If the state is to be changed set the value to the MAKE_* value you want
and the input_module_dialog_redraw_scene function will update the scene
==============================================================================*/
{
	NOT_VISIBLE = 0,
	POINT_CROSS,
	POINT_PIXEL,
	SURFACE_SPHERE
}; /* Input_module_scene_display */


struct Input_module_dialog_data
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
Contains the offset for the position, tangent and normal vectors.
==============================================================================*/
{
	double current_value[3], scale[3], offset[3];
}; /* Input_module_dialog_data */

struct Input_module_calib_data
/*******************************************************************************
LAST MODIFIED : 27 February 1998

DESCRIPTION :
Contains data for one of the calibration points.
==============================================================================*/
{
	int list_number;
	Gmatrix direction;
	double position[3];
}; /* Input_module_calib_data */

DECLARE_LIST_TYPES(Input_module_calib_data);

struct Input_module_calib_struct
/*******************************************************************************
LAST MODIFIED : 27 February 1998

DESCRIPTION :
Contains information for each of the calibration windows that pop up.
==============================================================================*/
{
	int last_number;
	struct LIST(Input_module_calib_data) *data_list;
	double global[3], offset[3];
	Widget control,scroll,control_clear,control_delete,control_record,
		control_solve;
	Widget dialog,widget;
}; /* Input_module_calib_struct */

struct Input_module_faro_calib_struct
/*******************************************************************************
LAST MODIFIED : 27 February 1998

DESCRIPTION :
Contains information for each of the calibration windows that pop up.
==============================================================================*/
{
	int calibrating;
	Widget control,scroll,three_point;
	Widget dialog,widget;
}; /* Input_module_faro_calib_struct */

struct Input_module_dialog_struct
/*******************************************************************************
LAST MODIFIED : 9 November 1998

DESCRIPTION :
Contains the widgets for the input_module controller.
==============================================================================*/
{
	enum Input_module_scene_display scene_display, last_scene_display;
	enum Input_module_device device;
	int form_mode;
	Gmatrix last_direction;
	struct Input_module_calib_struct calib_struct[3];
	struct Input_module_faro_calib_struct faro_struct;
	struct Graphical_material *material;
	struct GT_object *graphics_object;
	struct GT_pointset *cross_pointset, *pixel_pointset;
	struct GT_surface *sphere_surface;

	/* SAB I'm trying to get it so that this widget does not keep
		copies of these values, they can be requested from the input module */
	struct Input_module_dialog_data position, tangent, normal;
	struct Scene *scene;
	Widget device_chooser_form, device_chooser_widget,
		*dialog_address,dialog,widget,dialog_parent,
		data_rowcol,data_widget[3],data_rc_form[3], scale_frame[3],
		offset_frame[3], dof3_scale_widget[3], dof3_offset_widget[3],
		current_x_widget[3], current_y_widget[3], current_z_widget[3],
		display_control_form, scene_chooser_widget, scene_chooser_form,
		material_chooser_widget, material_chooser_form;
}; /* Input_module_dialog_struct */

struct Input_module_calib_sum_position_struct
{
	struct Input_module_dialog_struct *input_module_dialog;
	int component;
	double sum[3];
};

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int input_module_dialog_hierarchy_open=0;
static MrmHierarchy input_module_dialog_hierarchy;
#endif /* defined (MOTIF) */

/*
Module functions
----------------
*/
DECLARE_SIMPLE_LIST_OBJECT_FUNCTIONS(Input_module_calib_data)
FULL_DECLARE_LIST_TYPE(Input_module_calib_data);
DECLARE_LIST_FUNCTIONS(Input_module_calib_data)
DECLARE_FIND_BY_IDENTIFIER_IN_LIST_FUNCTION(Input_module_calib_data,list_number,int,
	compare_int)

static int input_module_dialog_redraw_scene( struct Input_module_dialog_struct *input_module_dialog )
/*******************************************************************************
LAST MODIFIED : 20 March 2000

DESCRIPTION :
Redraws the current representation of the input devices position in the selected scene
==============================================================================*/
{
#if defined (OLD_CODE)
static gtMatrix transform_matrix={
	1,0,0,0,
	0,1,0,0,
	0,0,1,0,
	0,0,0,1};
#endif /* defined (OLD_CODE) */

	char graphics_object_name[50];
	double PI2 = 2.0 * PI,
		surface_radius = 1.0, x, y, z;
	FE_value point_size;
	float time;
	gtMarkerType marker_type;
	int i, j, npts, surface_discretise_xi1 = 6, surface_discretise_xi2 = 11,
		return_code;
	struct GT_pointset **pointset_address;
	Triple *points, *normalpoints;

	ENTER(input_module_dialog_redraw_scene);

	if ( input_module_dialog)
	{
		if ( input_module_dialog->scene && input_module_dialog->scene_display )
		{
			/* First clean up any left over graphic objects */
			if ( input_module_dialog->cross_pointset && input_module_dialog->scene_display != POINT_CROSS )
			{
				if ( input_module_dialog->graphics_object )
				{
					printf("input_module_dialog_redraw_scene.  Removing pointcross\n");
					Scene_remove_graphics_object( input_module_dialog->scene,
						input_module_dialog->graphics_object );
					DEACCESS(GT_object)(&input_module_dialog->graphics_object);
					input_module_dialog->graphics_object = (struct GT_object *)NULL;
				}
				input_module_dialog->cross_pointset = (struct GT_pointset *)NULL;
			}
			if ( input_module_dialog->pixel_pointset && input_module_dialog->scene_display != POINT_PIXEL )
			{
				if ( input_module_dialog->graphics_object )
				{
					printf("input_module_dialog_redraw_scene.  Removing pointpixel\n");
					Scene_remove_graphics_object( input_module_dialog->scene,
						input_module_dialog->graphics_object );
					DEACCESS(GT_object)(&input_module_dialog->graphics_object);
					input_module_dialog->graphics_object = (struct GT_object *)NULL;
				}
				input_module_dialog->pixel_pointset = (struct GT_pointset *)NULL;
			}
			if ( input_module_dialog->sphere_surface && input_module_dialog->scene_display != SURFACE_SPHERE )
			{
				if ( input_module_dialog->graphics_object )
				{
					printf("input_module_dialog_redraw_scene.  Removing sphere\n");
					Scene_remove_graphics_object( input_module_dialog->scene,
						input_module_dialog->graphics_object );
					DEACCESS(GT_object)(&input_module_dialog->graphics_object);
					input_module_dialog->graphics_object = (struct GT_object *)NULL;
				}
				input_module_dialog->sphere_surface = (struct GT_surface *)NULL;
			}
			switch ( input_module_dialog->scene_display )
			{
				case POINT_CROSS:
				case POINT_PIXEL:
				{
					if ( input_module_dialog->graphics_object )
					{
						/* Just update the position */
						if ( POINT_CROSS == input_module_dialog->scene_display )
						{
							points = input_module_dialog->cross_pointset->pointlist;
						}
						else
						{
							points = input_module_dialog->pixel_pointset->pointlist;
						}
						points[0][0] = input_module_dialog->position.current_value[0];
						points[0][1] = input_module_dialog->position.current_value[1];
						points[0][2] = input_module_dialog->position.current_value[2];
						points[1][0] = input_module_dialog->position.current_value[0]+1.0;
						points[1][1] = input_module_dialog->position.current_value[1]+2.0;
						points[1][2] = input_module_dialog->position.current_value[2]+3.0;
						points[2][0] = input_module_dialog->position.current_value[0]+5.0;
						points[2][1] = input_module_dialog->position.current_value[1]+6.0;
						points[2][2] = input_module_dialog->position.current_value[2]+8.0;
						GT_object_changed(input_module_dialog->graphics_object);
#if defined (OLD_CODE)
						/* Don't need to recompile scene, only the graphics object */
						/*???RC compilation should be initiated by client of the scene
							(if any) since it owns the GLXContext */
						scene_time=Scene_time(input_module_dialog->scene);
						compile_GT_object( input_module_dialog->graphics_object,
							(void *)&scene_time);
#endif /* defined (OLD_CODE) */

						return_code = 1;
					}
					else
					{
						if ( POINT_CROSS == input_module_dialog->scene_display )
						{
							marker_type = g_PLUS_MARKER;
							pointset_address = &(input_module_dialog->cross_pointset);
						}
						else
						{
							marker_type = g_POINT_MARKER;
							pointset_address = &(input_module_dialog->pixel_pointset);
						}
						strcpy( graphics_object_name, "input pointset");
						printf("input_module_dialog_redraw_scene.  Adding pointset...");
						point_size = 1;
						time = 0;
						if ( ALLOCATE( points, Triple, 3))
						{
							points[0][0] = input_module_dialog->position.current_value[0];
							points[0][1] = input_module_dialog->position.current_value[1];
							points[0][2] = input_module_dialog->position.current_value[2];
							points[1][0] = input_module_dialog->position.current_value[0]+1.0;
							points[1][1] = input_module_dialog->position.current_value[1]+2.0;
							points[1][2] = input_module_dialog->position.current_value[2]+3.0;
							points[2][0] = input_module_dialog->position.current_value[0]+5.0;
							points[2][1] = input_module_dialog->position.current_value[1]+6.0;
							points[2][2] = input_module_dialog->position.current_value[2]+8.0;
							/* Create a new graphics object */
							if ((input_module_dialog->graphics_object =
								CREATE(GT_object)(graphics_object_name, g_POINTSET,
								input_module_dialog->material)))
							{
								ACCESS(GT_object)(input_module_dialog->graphics_object);
								/* Create a new pointset */
								if (*pointset_address =
									CREATE(GT_pointset)(3, points, (char **)NULL,
									marker_type, point_size, g_NO_DATA, (GTDATA *)NULL,
									(int *)NULL), (struct Graphics_font *)NULL)
								{
									GT_OBJECT_ADD(GT_pointset)(
										input_module_dialog->graphics_object, time,
										*pointset_address);
									printf(" to scene\n");
									return_code=
										Scene_add_graphics_object(input_module_dialog->scene,
											input_module_dialog->graphics_object, 0,
											input_module_dialog->graphics_object->name,
											/*fast_changing*/0);
								}
							}
						}
						else
						{
							return_code = 0;
							display_message(ERROR_MESSAGE,
									"input_module_dialog_redraw_scene. Unable to allocate points");
						}
					}
				} break;
				case SURFACE_SPHERE:
				{
					if ( input_module_dialog->graphics_object )
					{
						x = input_module_dialog->position.current_value[0];
						y = input_module_dialog->position.current_value[1];
						z = input_module_dialog->position.current_value[2];
						/* Just update the position */
						points = input_module_dialog->sphere_surface->pointlist;
						normalpoints = input_module_dialog->sphere_surface->normallist;
						npts = surface_discretise_xi1 * surface_discretise_xi2;
						for ( i = 0 ; i < surface_discretise_xi1 ; i++ )
						{
							for ( j = 0 ; j < surface_discretise_xi2 ; j++ )
							{
								points[i * surface_discretise_xi2 + j][0] = x + surface_radius *
									sin( PI2 * (double)j / (double)(surface_discretise_xi2 - 1) )*
									sin ( PI * (double)i / (double)(surface_discretise_xi1 - 1) );
								points[i * surface_discretise_xi2 + j][1] = y + surface_radius *
									cos( PI2 * (double)j / (double)(surface_discretise_xi2 - 1) )*
									sin ( PI * (double)i / (double)(surface_discretise_xi1 - 1) );
								points[i * surface_discretise_xi2 + j][2] = z + surface_radius *
									cos( PI * (double)i / (double)(surface_discretise_xi1 - 1));
								/* Normals */
								normalpoints[i * surface_discretise_xi2 + j][0] =
									sin( PI2 * (double)j / (double)(surface_discretise_xi2 - 1) )*
									sin ( PI * (double)i / (double)(surface_discretise_xi1 - 1) );
								normalpoints[i * surface_discretise_xi2 + j][1] =
									cos( PI2 * (double)j / (double)(surface_discretise_xi2 - 1) )*
									sin ( PI * (double)i / (double)(surface_discretise_xi1 - 1) );
								normalpoints[i * surface_discretise_xi2 + j][2] =
									cos( PI * (double)i / (double)(surface_discretise_xi1 - 1));
							}
						}
						GT_object_changed(input_module_dialog->graphics_object);
#if defined (OLD_CODE)
						/*???RC compilation should be initiated by client of the scene
							(if any) since it owns the GLXContext */
						scene_time=Scene_time(input_module_dialog->scene);
						compile_GT_object( input_module_dialog->graphics_object,
							(void *)&scene_time);
#endif /* defined (OLD_CODE) */
						return_code = 1;
					}
					else
					{
						strcpy( graphics_object_name, "input surface");
						printf("input_module_dialog_redraw_scene.  Adding surface...");
						point_size = 1;
						time = 0;
						x = input_module_dialog->position.current_value[0];
						y = input_module_dialog->position.current_value[1];
						z = input_module_dialog->position.current_value[2];
						npts=surface_discretise_xi1*surface_discretise_xi2;
						if ( ALLOCATE( points, Triple, npts)
							&& ALLOCATE( normalpoints, Triple, npts))
						{
							for ( i = 0 ; i < surface_discretise_xi1 ; i++ )
							{
								for ( j = 0 ; j < surface_discretise_xi2 ; j++ )
								{
									points[i * surface_discretise_xi2 + j][0] = x+surface_radius*
										sin( PI2 * (double)j / (double)(surface_discretise_xi2-1))*
										sin ( PI * (double)i / (double)(surface_discretise_xi1-1));
									points[i * surface_discretise_xi2 + j][1] = y+surface_radius*
										cos( PI2 * (double)j / (double)(surface_discretise_xi2-1))*
										sin ( PI * (double)i / (double)(surface_discretise_xi1-1));
									points[i * surface_discretise_xi2 + j][2] = z+surface_radius*
										cos(PI * (double)i / (double)(surface_discretise_xi1 - 1));
									/* Normals */
									normalpoints[i * surface_discretise_xi2 + j + npts][0] =
										sin( PI2 * (double)j / (double)(surface_discretise_xi2-1))*
										sin ( PI * (double)i / (double)(surface_discretise_xi1-1));
									normalpoints[i * surface_discretise_xi2 + j + npts][1] =
										cos( PI2 * (double)j / (double)(surface_discretise_xi2-1))*
										sin ( PI * (double)i / (double)(surface_discretise_xi1-1));
									normalpoints[i * surface_discretise_xi2 + j + npts][2] =
										cos( PI * (double)i / (double)(surface_discretise_xi1 - 1));
								}
							}
							/* Create a new graphics object */
							if ((input_module_dialog->graphics_object =
								CREATE(GT_object)(graphics_object_name, g_SURFACE,
								input_module_dialog->material)))
							{
								ACCESS(GT_object)(input_module_dialog->graphics_object);
								if ((input_module_dialog->sphere_surface=CREATE(GT_surface)(
									g_SHADED_TEXMAP, g_QUADRILATERAL,
									surface_discretise_xi2, surface_discretise_xi1,
									points, normalpoints, /*tangentpoints*/(Triple *)NULL,
									/*texturepoints*/(Triple *)NULL,
									g_NO_DATA, (GTDATA *)NULL)))
								{
									GT_OBJECT_ADD(GT_surface)(
										input_module_dialog->graphics_object, time,
										input_module_dialog->sphere_surface);
									printf(" to scene\n");
									return_code=
										Scene_add_graphics_object(input_module_dialog->scene,
											input_module_dialog->graphics_object, 0,
											input_module_dialog->graphics_object->name,
											/*fast_changing*/0);
								}
								else
								{
									DEALLOCATE( points );
									return_code = 0;
									display_message(ERROR_MESSAGE,
												"input_module_dialog_redraw_scene. Unable to create surface");
								}
							}
							else
							{
								DEALLOCATE( points );
								return_code = 0;
								display_message(ERROR_MESSAGE,
											"input_module_dialog_redraw_scene. Unable to create graphics_object");
							}
						}
						else
						{
							DEALLOCATE( points );
							return_code = 0;
							display_message(ERROR_MESSAGE,
									"input_module_dialog_redraw_scene. Unable to allocate points");
						}
					}
				} break;
			}
		}
	}
	else
	{
		return_code = 0;
		display_message(ERROR_MESSAGE,
			"input_module_dialog_redraw_scene. Invalid arguments");
	}
	LEAVE;
	return ( return_code );
} /* input_module_dialog_redraw_scene */

static void input_module_dialog_identify(Widget w, int object_num,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 24 July 1995

DESCRIPTION :
Finds the id of the buttons on the input_module_dialog widget.
==============================================================================*/
{
	int number;
	struct Input_module_dialog_struct *input_module_dialog;
	XmString new_string;

	ENTER(input_module_dialog_identify);
	USE_PARAMETER(reason);
	/* find out which input_module_dialog widget we are in */
	XtVaGetValues(w,XmNuserData,&input_module_dialog,NULL);

	switch (object_num)
	{
		case input_module_chooser_form_ID:
		{
			input_module_dialog->device_chooser_form = w;
		} break;
		case input_module_data_rowcol_ID:
		{
			input_module_dialog->data_rowcol = w;
		} break;
		case input_module_scale_frame_ID:
		{
			XtVaGetValues(XtParent(XtParent(w)),XmNuserData,&number,NULL);
			input_module_dialog->scale_frame[number] = w;
		} break;
		case input_module_offset_frame_ID:
		{
			XtVaGetValues(XtParent(XtParent(w)),XmNuserData,&number,NULL);
			input_module_dialog->offset_frame[number] = w;
		} break;
		case input_module_form_label_ID:
		{
			XtVaGetValues(XtParent(w),XmNuserData,&number,NULL);
			input_module_dialog->scale_frame[number] = w;
			switch( number )
			{
				case 0:
				{
					new_string=XmStringCreateSimple("Position");
				} break;
				case 1:
				{
					new_string=XmStringCreateSimple("Tangent");
				} break;
				case 2:
				{
					new_string=XmStringCreateSimple("Normal");
				} break;
			}
			XtVaSetValues(w,XmNlabelString,new_string,NULL);
			XmStringFree(new_string);
		} break;
		case input_module_current_x_ID:
		{
			XtVaGetValues(XtParent(XtParent(w)),XmNuserData,&number,NULL);
			input_module_dialog->current_x_widget[number] = w;
		} break;
		case input_module_current_y_ID:
		{
			XtVaGetValues(XtParent(XtParent(w)),XmNuserData,&number,NULL);
			input_module_dialog->current_y_widget[number] = w;
		} break;
		case input_module_current_z_ID:
		{
			XtVaGetValues(XtParent(XtParent(w)),XmNuserData,&number,NULL);
			input_module_dialog->current_z_widget[number] = w;
		} break;
		case input_module_scene_chooser_ID:
		{
			input_module_dialog->scene_chooser_form = w;
		} break;
		case input_module_material_ID:
		{
			input_module_dialog->material_chooser_form = w;
		} break;
		case input_module_display_control_ID:
		{
			input_module_dialog->display_control_form = w;
		} break;
	default:
		{
			display_message(WARNING_MESSAGE,
				"input_module_dialog_identify_button.  Invalid button number");
		}; break;
	}
	LEAVE;
} /* input_module_dialog_identify_button */

static void input_module_dialog_destroy_CB(Widget w,int *tag,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 20 March 2000

DESCRIPTION :
Callback for the input_module_dialog dialog - tidies up all details - mem etc
==============================================================================*/
{
	struct Input_module_dialog_struct *input_module_dialog;

	ENTER(input_module_dialog_destroy_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	/* Get the pointer to the data for the input_module_dialog widget */
	XtVaGetValues(w,XmNuserData,&input_module_dialog,NULL);

	/* Release the callback from the selected device */
	if ( input_module_dialog->device != IM_DEVICE_NONE )
	{
		input_module_deregister(input_module_dialog->device, input_module_dialog);
	}

	*(input_module_dialog->dialog_address) = (Widget)NULL;
	/* deallocate the memory for the user data */
	/*printf("input_module_dialog_destroy_CB. Not De-allocating memory for input_module_dialog %p.\n", input_module_dialog);*/
	DEALLOCATE(input_module_dialog);
	input_module_dialog = (struct Input_module_dialog_struct *) NULL;
	LEAVE;
} /* input_module_dialog_destroy_CB */

static void input_module_dialog_update_scene(Widget widget,
	void *input_module_dialog_void,void *scene_void)
/*******************************************************************************
LAST MODIFIED : 20 March 2000

DESCRIPTION :
Called when scene is changed.
==============================================================================*/
{
	struct Input_module_dialog_struct *input_module_dialog;

	ENTER(input_module_dialog_update_scene);
	USE_PARAMETER(widget);
	if ( input_module_dialog_void && scene_void )
	{
		input_module_dialog =
		(struct Input_module_dialog_struct *)input_module_dialog_void;

		if ( input_module_dialog->graphics_object && input_module_dialog->scene )
		{
			if ( Scene_has_graphics_object ( input_module_dialog->scene,
				input_module_dialog->graphics_object ))
			{
				Scene_remove_graphics_object( input_module_dialog->scene,
				input_module_dialog->graphics_object );
			}
		}
		input_module_dialog->scene = (struct Scene *)scene_void;
		if ( input_module_dialog->graphics_object )
		{
			Scene_add_graphics_object(input_module_dialog->scene,
				input_module_dialog->graphics_object, 0,
				input_module_dialog->graphics_object->name,
				/*fast_changing*/0);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"input_module_dialog_update_scene.  Invalid argument(s)");
	}
	LEAVE;
} /* input_module_dialog_update_scene */

static void input_module_dialog_update_material(Widget widget,
	void *input_module_dialog_void, void *material_void)
/*******************************************************************************
LAST MODIFIED : 26 February 1998

DESCRIPTION :
Called when material is changed.
==============================================================================*/
{
	struct Input_module_dialog_struct *input_module_dialog;

	ENTER(input_module_dialog_update_material);
	USE_PARAMETER(widget);
	if ( input_module_dialog_void && material_void )
	{
		input_module_dialog =
		(struct Input_module_dialog_struct *)input_module_dialog_void;
		input_module_dialog->material = (struct Graphical_material *)material_void;

		if ( input_module_dialog->graphics_object )
		{
			if ( input_module_dialog->graphics_object->default_material )
			{
				DEACCESS(Graphical_material)(&(input_module_dialog->graphics_object->default_material));
			}
			input_module_dialog->graphics_object->default_material =
				ACCESS(Graphical_material)( input_module_dialog->material );
			GT_object_changed(input_module_dialog->graphics_object);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"input_module_dialog_update_material.  Invalid argument(s)");
	}
	LEAVE;
} /* input_module_dialog_update_material */

#if defined (OLD_CODE)
static void input_module_dialog_dof3_update(Widget dof3_widget,
	void *user_data, void *new_dof3_data)
/*******************************************************************************
LAST MODIFIED : 16 February 1998

DESCRIPTION :
Receives a pointer to a dof3_struct, and the new data for it.
==============================================================================*/
{
	double *data;
	int i;
	struct Dof3_data *new_data;

	ENTER(dg_dof3_data_no_update);

	if ( user_data && dof3_widget && new_dof3_data )
	{
		data = (double *)user_data;
		new_data = (struct Dof3_data *)new_dof3_data;
		for (i=0;i<3;i++)
		{
			data[i] = new_data->data[i];
		}
	}
	else
	{
		display_message(WARNING_MESSAGE,"input_module_dialog_dof3_update. Invalid arguments");
	}

	LEAVE;
} /* input_module_dialog_dof3_update */
#endif /* defined (OLD_CODE) */

static void input_module_dialog_dof3_update_position(Widget dof3_widget,
	void *user_data, void *new_dof3_data)
/*******************************************************************************
LAST MODIFIED : 16 February 1998

DESCRIPTION :
Receives a pointer to a dof3_struct, and the new data for it.
==============================================================================*/
{
	enum Input_module_device device;
	enum Input_module_data_types mode;
	struct Dof3_data *new_data;

	ENTER(input_module_dialog_dof3_update_position);

	if ( user_data && dof3_widget && new_dof3_data )
	{
		device = (enum Input_module_device)(((int)user_data) & 0xff);
		mode = (enum Input_module_data_types)(((int)user_data & 0xff00 ) >> 8);
		new_data = (struct Dof3_data *)new_dof3_data;
		input_module_set( device, mode, new_data->data );
	}
	else
	{
		display_message(WARNING_MESSAGE,"input_module_dialog_dof3_update. Invalid arguments" );
	}

	LEAVE;
} /* input_module_dialog_dof3_update_position */

static void input_module_dialog_update_form(struct Input_module_dialog_struct *input_module_dialog, int new_mode)
/*******************************************************************************
LAST MODIFIED : 2 November 1995

DESCRIPTION :
Adds however many forms are required to the calibration and data rowcol's.
Deletes any forms that are now not required, and adds any new forms to the
bottom of the rowcol.
==============================================================================*/
{
	double data[3];
	int new_forms,old_forms;
	Arg override_arg;
	struct Dof3_data dof3_data;
	MrmType input_module_dialog_data_form_class;
	struct Callback_data callback;

	ENTER(input_module_dialog_add_forms);
	if (input_module_dialog_hierarchy_open)
	{
			/*???debug */
			printf("old_mode %i new_mode %i\n",input_module_dialog->form_mode,new_mode);
		/* kill any widgets that are there already */
		new_forms=new_mode&(~input_module_dialog->form_mode);
			/*???debug */
			printf("new forms %i\n",new_forms);
		old_forms=input_module_dialog->form_mode&(~new_mode);
			/*???debug */
			printf("old forms %i\n",old_forms);
		if (old_forms&INPUT_MODULE_DIALOG_POSITION)
		{
			XtDestroyWidget(input_module_dialog->data_rc_form[0]);
		}
		if (old_forms&INPUT_MODULE_DIALOG_TANGENT)
		{
			XtDestroyWidget(input_module_dialog->data_rc_form[1]);
		}
		if (old_forms&INPUT_MODULE_DIALOG_NORMAL)
		{
			XtDestroyWidget(input_module_dialog->data_rc_form[2]);
		}
		if (new_forms&INPUT_MODULE_DIALOG_POSITION)
		{
			XtSetArg(override_arg,XmNuserData,0);
			if (MrmSUCCESS==MrmFetchWidgetOverride(input_module_dialog_hierarchy,
				"input_module_dialog_data_form",input_module_dialog->data_rowcol,NULL,&override_arg,1,
				&input_module_dialog->data_rc_form[0],&input_module_dialog_data_form_class))
			{
				XtManageChild(input_module_dialog->data_rc_form[0]);
				input_module_get( input_module_dialog->device,
					IM_POSITION_SCALE, dof3_data.data );
				if (!create_dof3_widget(&input_module_dialog->dof3_scale_widget[0],
					input_module_dialog->scale_frame[0],DOF3_POSITION,DOF3_ABSOLUTE,
					CONV_RECTANGULAR_CARTESIAN, &dof3_data))
				{
					display_message(ERROR_MESSAGE,
						"input_module_dialog_add_forms.  Could not create position scale dof3 widget");
				}
				callback.procedure = input_module_dialog_dof3_update_position;
				callback.data = (void *)((input_module_dialog->device & 0xff ) + (( IM_POSITION_SCALE & 0xff ) << 8));
				dof3_set_data(input_module_dialog->dof3_scale_widget[0],
					DOF3_UPDATE_CB,&callback);

				input_module_get( input_module_dialog->device,
					IM_POSITION_OFFSET, dof3_data.data );
				if (!create_dof3_widget(&input_module_dialog->dof3_offset_widget[0],
					input_module_dialog->offset_frame[0],DOF3_POSITION,DOF3_ABSOLUTE,
					CONV_RECTANGULAR_CARTESIAN,
					&dof3_data))
				{
					display_message(ERROR_MESSAGE,
						"input_module_dialog_add_forms.  Could not create position offset dof3 widget");
				}
				callback.procedure = input_module_dialog_dof3_update_position;
				callback.data = (void *)((input_module_dialog->device & 0xff ) + ((IM_POSITION_OFFSET & 0xff ) << 8));
				dof3_set_data(input_module_dialog->dof3_offset_widget[0],
					DOF3_UPDATE_CB,&callback);
			}
			else
			{
				display_message(WARNING_MESSAGE,
					"input_module_dialog_add_forms.  Could not fetch data form widget");
			}
		}
		if ( new_mode & INPUT_MODULE_DIALOG_POSITION)
		{
			input_module_get( input_module_dialog->device, IM_POSITION_SCALE, data );
			dof3_set_data( input_module_dialog->dof3_scale_widget[0],
				DOF3_DATA, data);
			input_module_get( input_module_dialog->device, IM_POSITION_OFFSET, data );
			dof3_set_data( input_module_dialog->dof3_offset_widget[0],
				DOF3_DATA, data);
		}
		if (new_forms&INPUT_MODULE_DIALOG_TANGENT)
		{
			XtSetArg(override_arg,XmNuserData,1);
		}
		if (new_forms&INPUT_MODULE_DIALOG_NORMAL)
		{
			XtSetArg(override_arg,XmNuserData,2);
		}
		input_module_dialog->form_mode=new_mode;
	}
	else
	{
		display_message(WARNING_MESSAGE,"input_module_dialog_add_forms.  Hierarchy not open");
	}
	LEAVE;
} /* input_module_dialog_add_forms */

static int input_module_dialog_newdata_CB(void *identifier,
	Input_module_message message)
/*******************************************************************************
LAST MODIFIED : 20 March 2000

DESCRIPTION :
Accepts input from one of the devices.
==============================================================================*/
{
	char char_string[100];
	int return_code;
	struct Input_module_dialog_struct *input_module_dialog;
	XmString new_string;

	ENTER(input_module_dialog_newdata_CB);

	if ( identifier && message )
	{
		input_module_dialog = identifier;
		switch (message->type)
		{
			case IM_TYPE_MOTION:
			{
				switch (message->source)
				{
					case IM_SOURCE_SPACEBALL:
					case IM_SOURCE_HAPTIC:
					case IM_SOURCE_FARO:
					{
						input_module_dialog->position.current_value[0] = message->data[0];
						input_module_dialog->position.current_value[1] = message->data[1];
						input_module_dialog->position.current_value[2] = message->data[2];

						input_module_dialog_redraw_scene( input_module_dialog );

						sprintf(char_string, "%10.5f", message->data[0]);
						new_string=XmStringCreateSimple(char_string);
						XtVaSetValues ( input_module_dialog->current_x_widget[0],
							XmNlabelString, new_string, NULL );
						XmStringFree( new_string );

						sprintf(char_string, "%10.5f", message->data[1]);
						new_string=XmStringCreateSimple(char_string);
						XtVaSetValues ( input_module_dialog->current_y_widget[0],
							XmNlabelString, new_string, NULL );
						XmStringFree( new_string );

						sprintf(char_string, "%10.5f", message->data[2]);
						new_string=XmStringCreateSimple(char_string);
						XtVaSetValues ( input_module_dialog->current_z_widget[0],
							XmNlabelString, new_string, NULL );
						XmStringFree( new_string );
					} break;
				}
			}
		}
		return_code = 1;
	}
	else
	{
		return_code = 0;
		display_message(ERROR_MESSAGE,"input_module_dialog_newdata_CB. Invalid arguments");
	}

	return ( return_code );

	LEAVE;
} /* input_module_dialog_newdata_CB */

static void input_module_dialog_change_device(Widget widget,
	struct Input_module_widget_data *device_change,void *input_module_dialog_void)
/*******************************************************************************
LAST MODIFIED : 20 March 2000

DESCRIPTION :
The selected device has been changed
==============================================================================*/
{
	struct Input_module_dialog_struct *input_module_dialog;

	ENTER(input_module_dialog_change_device);
	USE_PARAMETER(widget);
	if (device_change&&(input_module_dialog=
		(struct Input_module_dialog_struct *)input_module_dialog_void))
	{
		if (device_change->status)
		{
			input_module_dialog->device = device_change->device;
			input_module_register(device_change->device, input_module_dialog,
				input_module_dialog->widget, input_module_dialog_newdata_CB);
			input_module_dialog_update_form(input_module_dialog,
				INPUT_MODULE_DIALOG_POSITION);
		}
		else
		{
			input_module_deregister(device_change->device,input_module_dialog);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"input_module_dialog_change_device.  Invalid argument(s)");
	}

	LEAVE;
} /* input_module_dialog_change_device */

static void input_module_create_calib_dialog(struct Input_module_dialog_struct *input_module_dialog,
	int component)
/*******************************************************************************
LAST MODIFIED : 27 February 1998

DESCRIPTION :
Creates a new calibration dialog.  If one exists, bring it up to the front.
==============================================================================*/
{
	Arg override_arg;
	MrmType input_module_calibration_class;

	ENTER(input_module_create_calib_dialog);
	if (input_module_dialog_hierarchy_open)
	{
		if (input_module_dialog->calib_struct[component].dialog=XtVaCreatePopupShell(
			"Calibrate",topLevelShellWidgetClass,input_module_dialog->widget,
			/* XmNallowShellResize,TRUE, */NULL))
		{
			XtSetArg(override_arg,XmNuserData,component);
			if (MrmSUCCESS==MrmFetchWidgetOverride(input_module_dialog_hierarchy,
				"input_calib_widget",input_module_dialog->calib_struct[component].dialog,
				NULL,&override_arg,1,&input_module_dialog->calib_struct[component].widget,
				&input_module_calibration_class))
			{
				input_module_dialog->calib_struct[component].data_list = CREATE_LIST(Input_module_calib_data)();
				XtManageChild(input_module_dialog->calib_struct[component].widget);
				XtRealizeWidget(input_module_dialog->calib_struct[component].dialog);
				XtPopup(input_module_dialog->calib_struct[component].dialog, XtGrabNone);
			}
			else
			{
				display_message(WARNING_MESSAGE,
					"input_module_create_calib_dialog.  Could not fetch calibration widget");
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,
				"input_module_create_calib_dialog.  Could not create shell widget");
		}
	}
	else
	{
		display_message(WARNING_MESSAGE,
			"input_module_create_calib_dialog.  Hierarchy not open");
	}
	LEAVE;
} /* input_module_create_calib_dialog */

static void input_module_create_faro_calib_dialog(struct Input_module_dialog_struct *input_module_dialog)
/*******************************************************************************
LAST MODIFIED : 27 February 1998

DESCRIPTION :
Creates a new calibration dialog.  If one exists, bring it up to the front.
==============================================================================*/
{
	MrmType input_module_calibration_class;

	ENTER(input_module_create_faro_calib_dialog);
	if (input_module_dialog_hierarchy_open)
	{
		if (input_module_dialog->faro_struct.dialog=XtVaCreatePopupShell(
			"Calibrate Faro Arm",topLevelShellWidgetClass,input_module_dialog->widget,
			/* XmNallowShellResize,TRUE, */NULL))
		{
			if (MrmSUCCESS==MrmFetchWidgetOverride(input_module_dialog_hierarchy,
				"input_faro_widget",input_module_dialog->faro_struct.dialog,
				NULL,NULL,0,&input_module_dialog->faro_struct.widget,
				&input_module_calibration_class))
			{
				input_module_dialog->faro_struct.calibrating = 0;
				XtManageChild(input_module_dialog->faro_struct.widget);
				XtRealizeWidget(input_module_dialog->faro_struct.dialog);
				XtPopup(input_module_dialog->faro_struct.dialog, XtGrabNone);
			}
			else
			{
				display_message(WARNING_MESSAGE,
					"input_module_create_faro_calib_dialog.  Could not fetch calibration widget");
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,
				"input_module_create_faro_calib_dialog.  Could not create shell widget");
		}
	}
	else
	{
		display_message(WARNING_MESSAGE,
			"input_module_create_faro_calib_dialog.  Hierarchy not open");
	}
	LEAVE;
} /* input_module_create_faro_calib_dialog */

static void input_module_bring_up_calib(struct Input_module_dialog_struct *input_module_dialog, int component)
/*******************************************************************************
LAST MODIFIED : 27 February 1998

DESCRIPTION :
Creates a new calibration dialog.  If one exists, bring it up to the front.
==============================================================================*/
{
	ENTER(input_module_bring_up_calib);
	if (input_module_dialog->calib_struct[component].dialog)
	{
		XtPopup(input_module_dialog->calib_struct[component].dialog,XtGrabNone);
	}
	else
	{
		switch(input_module_dialog->device)
		{
			case IM_DEVICE_FARO:
			{
				input_module_create_faro_calib_dialog(input_module_dialog);
			} break;
			case IM_DEVICE_POLHEMUS1:
			case IM_DEVICE_POLHEMUS2:
			case IM_DEVICE_POLHEMUS3:
			case IM_DEVICE_POLHEMUS4:
			{
				input_module_create_calib_dialog(input_module_dialog, component);
			} break;
			default:
			{
				display_message(WARNING_MESSAGE,
					"input_module_bring_up_calib.  No calibration dialog for selected device");
			} break;
		}
	}
	LEAVE;
} /* input_module_bring_up_calib */

#if defined (OLD_CODE)
static void input_module_get_calib_data(struct Input_module_dialog_struct *input_module_dialog)
/*******************************************************************************
LAST MODIFIED : 27 February 1998

DESCRIPTION :
Initialises the calibration values.
==============================================================================*/
{
	int i;

	ENTER(input_module_get_calib_data);
	/* this should get the calibration data from a file. */
	for (i=0;i<3;i++)
	{
		input_module_dialog->position.offset[i]=0.0;
		input_module_dialog->tangent.offset[i]=0.0;
		input_module_dialog->normal.offset[i]=0.0;
	}
	input_module_dialog->tangent.offset[0]=1.0;
	input_module_dialog->normal.offset[0]=1.0;
	LEAVE;
} /* input_module_get_calib_data */
#endif /* defined (OLD_CODE) */

static double input_module_calib_get_error(
	struct Input_module_dialog_struct *input_module_dialog, int component,
	struct Input_module_calib_data *new_data)
/*******************************************************************************
LAST MODIFIED : 27 February 1998

DESCRIPTION :
Gets the L2 norm of the distance between the global and the estimate coords.
==============================================================================*/
{
	double return_value, temp[3];
	int i;

	ENTER(input_module_calib_get_error);
	for (i=0;i<3;i++)
	{
		temp[i] = input_module_dialog->calib_struct[component].offset[i];
	}
	matrix_postmult_vector(temp,&new_data->direction);
	return_value=0.0;
	for (i=0;i<3;i++)
	{
		return_value += (temp[i]+new_data->position[i]-
			input_module_dialog->calib_struct[component].global[i])*
			(temp[i]+new_data->position[i]-
			input_module_dialog->calib_struct[component].global[i]);
	}
	return_value=sqrt(return_value);
	LEAVE;

	return return_value;
} /* input_module_calib_get_error */

static int input_module_calib_sum_positions(struct Input_module_calib_data *object, void *user_data)
/*******************************************************************************
LAST MODIFIED : 27 February 1998

DESCRIPTION :
Calculates the average position of all the data points according to the current
transformation.
==============================================================================*/
{
	double temp[3];
	int i,return_code;
	struct Input_module_calib_sum_position_struct *sum_position = user_data;

	ENTER(input_module_calib_sum_positions);

	return_code=1;
	for (i=0;i<3;i++)
	{
		temp[i] = sum_position->input_module_dialog->
			calib_struct[sum_position->component].offset[i];
	}

	matrix_postmult_vector(temp, &object->direction);
	for (i=0;i<3;i++)
	{
		temp[i] += object->position[i];
		sum_position->sum[i] += temp[i];
	}
	LEAVE;

	return (return_code);
} /* input_module_calib_sum_positions */

static int input_module_calib_sum_difference(struct Input_module_calib_data *object, void *user_data)
/*******************************************************************************
LAST MODIFIED : 27 February 1998

DESCRIPTION :
Calculates the average difference between the global position and the data
point positions.
==============================================================================*/
{
	double temp[3];
	int i,return_code;
	struct Input_module_calib_sum_position_struct *sum_position = user_data;

	ENTER(input_module_calib_sum_difference);
	return_code=1;
	for (i=0;i<3;i++)
	{
		temp[i] = sum_position->input_module_dialog->
			calib_struct[sum_position->component].global[i]-object->
			position[i];
	}
	/* this may be in any direction from the data point, now transform it */
	matrix_premult_vector(temp,
		&object->direction);
	for (i=0;i<3;i++)
	{
		sum_position->sum[i] += temp[i];
	}
	LEAVE;

	return (return_code);
} /* input_module_calib_sum_difference */

static int input_module_calib_display_error(struct Input_module_calib_data *object, void *user_data)
/*******************************************************************************
LAST MODIFIED : 27 February 1998

DESCRIPTION :
Calculates the average difference between the global position and the data
point positions.
==============================================================================*/
{
	char new_string[150];
	int return_code;
	struct Input_module_calib_sum_position_struct *sum_position = user_data;
	XmString list_item;

	ENTER(input_module_calib_display_error);
	return_code = 1;
	object->list_number = sum_position->input_module_dialog->
		calib_struct[sum_position->component].last_number++;
	sprintf(new_string, "Calib point %2i.  Error %6.3lf",
		object->list_number,
		input_module_calib_get_error(sum_position->input_module_dialog,
		sum_position->component,object));
	list_item=XmStringCreateSimple(new_string);
	XmListAddItemUnselected(sum_position->input_module_dialog->
		calib_struct[sum_position->component].scroll,list_item,0);
	XmStringFree(list_item);
	LEAVE;

	return (return_code);
} /* input_module_calib_display_error */

static int input_module_calib_add_display(struct Input_module_calib_data *object, void *user_data)
/*******************************************************************************
LAST MODIFIED : 27 February 1998

DESCRIPTION :
Calculates the average difference between the global position and the data
point positions.
==============================================================================*/
{
	char new_string[150];
	int return_code;
	struct Input_module_calib_sum_position_struct *sum_position = user_data;
	XmString list_item;

	ENTER(input_module_calib_add_display);
	return_code=1;
	object->list_number=(sum_position->input_module_dialog->calib_struct)
		[sum_position->component].last_number++;
	sprintf(new_string,"Calib point %2i.  Error %6.3lf",
		object->list_number,
		input_module_calib_get_error(sum_position->input_module_dialog,
		sum_position->component,object));
	list_item=XmStringCreateSimple(new_string);
	XmListAddItemUnselected((sum_position->input_module_dialog->calib_struct)
		[sum_position->component].scroll,list_item,0);
	XmStringFree(list_item);
	LEAVE;

	return (return_code);
} /* input_module_calib_add_display */

static void input_module_calib_calibrate(struct Input_module_dialog_struct *input_module_dialog,
	int component)
/*******************************************************************************
LAST MODIFIED : 27 February 1998

DESCRIPTION :
Minimises the position of the global coordinates so that the difference between
the estimates and the global posn is minimised.  Must be passed with at least
one data point.
==============================================================================*/
{
	int i,num_items;
	struct Input_module_calib_sum_position_struct sum_position;

	ENTER(input_module_calib_calibrate);
	num_items=NUMBER_IN_LIST(Input_module_calib_data)(
		input_module_dialog->calib_struct[component].data_list);
	sum_position.component=component;
	sum_position.input_module_dialog=input_module_dialog;
	/* get the average position of the data points */
	for (i=0;i<3;i++)
	{
		sum_position.sum[i]=0.0;
	}
	FOR_EACH_OBJECT_IN_LIST(Input_module_calib_data)(input_module_calib_sum_positions,&sum_position,
		input_module_dialog->calib_struct[component].data_list);
	/* we have the sum, now average */
	for (i=0;i<3;i++)
	{
		input_module_dialog->calib_struct[component].global[i]=
			sum_position.sum[i]/num_items;
	}
	/* get the average difference (corrected by transformation) */
	for (i=0;i<3;i++)
	{
		sum_position.sum[i]=0.0;
	}
	FOR_EACH_OBJECT_IN_LIST(Input_module_calib_data)(input_module_calib_sum_difference,&sum_position,
		input_module_dialog->calib_struct[component].data_list);
	/* we have the sum, now average */
	for (i=0;i<3;i++)
	{
		input_module_dialog->calib_struct[component].offset[i]=
			sum_position.sum[i]/num_items;
	}
	/* now update the tangent and normal vectors */
	switch (component)
	{
		case 0:
		{
			printf("input_module_calib_calibrate %lf %lf %lf\n",
				input_module_dialog->calib_struct[0].offset[i],
				input_module_dialog->calib_struct[1].offset[i],
				input_module_dialog->calib_struct[2].offset[i] );
			input_module_set( input_module_dialog->device,
				IM_POSITION_OFFSET, input_module_dialog->calib_struct[0].offset);
			/* SAB Would be better to enable the input module so that the dof3 can
				register for changes with the input module function so the
				dof3 gets updated even if someone else changes it */
			dof3_set_data(input_module_dialog->dof3_offset_widget[0],DOF3_DATA,
				&input_module_dialog->calib_struct[0].offset);
		} break;
		case 1:
		{
			for (i=0;i<3;i++)
			{
				input_module_dialog->tangent.offset[i]=
					input_module_dialog->calib_struct[1].offset[i]-
					input_module_dialog->calib_struct[0].offset[i];
			}
			/* make it a unit vector */
			matrix_vector_unit(&input_module_dialog->tangent.offset[0]);
			dof3_set_data(input_module_dialog->dof3_offset_widget[1],DOF3_DATA,
				&input_module_dialog->tangent.offset);
		} break;
		case 2:
		{
			for (i=0;i<3;i++)
			{
				input_module_dialog->normal.offset[i]=
					input_module_dialog->calib_struct[2].offset[i]-
					input_module_dialog->calib_struct[0].offset[i];
			}
			/* make it a unit vector */
			matrix_vector_unit(&input_module_dialog->normal.offset[0]);
			dof3_set_data(input_module_dialog->dof3_offset_widget[2],DOF3_DATA,
				&input_module_dialog->normal.offset);
		} break;
	}
	/* Update the view */
	XmListDeleteAllItems(input_module_dialog->calib_struct[component].scroll);
	input_module_dialog->calib_struct[component].last_number=0;
	/* go through and display all the errors for the data points */
	FOR_EACH_OBJECT_IN_LIST(Input_module_calib_data)(input_module_calib_display_error,&sum_position,
		input_module_dialog->calib_struct[component].data_list);
	LEAVE;
} /* input_module_calib_calibrate */

static void input_module_calib_identify_button(Widget w,int button_num,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 20 March 2000

DESCRIPTION :
Finds the id of the buttons on the data_grabber_calib widget.
==============================================================================*/
{
	int component;
	struct Input_module_dialog_struct *input_module_dialog;

	ENTER(input_module_calib_identify_button);
	USE_PARAMETER(reason);
	XtVaGetValues(w,XmNuserData,&input_module_dialog,NULL);
	if (button_num==input_module_calib_control_ID)
	{
		XtVaGetValues(XtParent(w),XmNuserData,&component,NULL);
		switch (button_num)
		{
			case input_module_calib_control_ID:
			{
				input_module_dialog->calib_struct[component].control=w;
			} break;
			default:
			{
				display_message(WARNING_MESSAGE,
					"input_module_calib_identify_button.  Invalid button number");
			} break;
		}
	}
	else
	{
		XtVaGetValues(XtParent(XtParent(w)),XmNuserData,&component,NULL);
		switch (button_num)
		{
			case input_module_calib_scroll_ID:
			{
				input_module_dialog->calib_struct[component].scroll=w;
			} break;
			case input_module_calib_cont_record_ID:
			{
				input_module_dialog->calib_struct[component].control_record=w;
			} break;
			case input_module_calib_cont_clear_ID:
			{
				input_module_dialog->calib_struct[component].control_clear=w;
			} break;
			case input_module_calib_cont_solve_ID:
			{
				input_module_dialog->calib_struct[component].control_solve=w;
			} break;
			case input_module_calib_cont_delete_ID:
			{
				input_module_dialog->calib_struct[component].control_delete=w;
			} break;
			default:
			{
				display_message(WARNING_MESSAGE,
					"input_module_calib_identify_button.  Invalid button number");
			} break;
		}
	}
	LEAVE;
} /* input_module_identify_button */

static void input_module_calib_control_CB(Widget w,int button_num,
	XmAnyCallbackStruct *reason)
/*******************************************************************************
LAST MODIFIED : 20 March 2000

DESCRIPTION :
Performs actions depending upon the button pressed.
==============================================================================*/
{
	char new_string[150];
	int i,component,*select_list,num_selected;
	struct Input_module_dialog_struct *input_module_dialog;
	struct Input_module_calib_data *new_data;
	struct Input_module_calib_sum_position_struct sum_position;
	XmString list_item;

	ENTER(input_module_calib_identify_button);
	USE_PARAMETER(reason);
	XtVaGetValues(w,XmNuserData,&input_module_dialog,NULL);
	XtVaGetValues(XtParent(XtParent(w)),XmNuserData,&component,NULL);
	switch (button_num)
	{
		case input_module_calib_cont_record_ID:
		{
			if (ALLOCATE(new_data,struct Input_module_calib_data,1))
			{
				new_data->list_number=
					input_module_dialog->calib_struct[component].last_number++;
				for (i=0;i<3;i++)
				{
					new_data->position[i]=input_module_dialog->position.current_value[i];
				}
				matrix_copy(&new_data->direction,&input_module_dialog->last_direction);
				ADD_OBJECT_TO_LIST(Input_module_calib_data)(new_data,
					input_module_dialog->calib_struct[component].data_list);
				sprintf(new_string,"Calib point %2i.  Error %6.3lf",
					new_data->list_number,
					input_module_calib_get_error(input_module_dialog,component,new_data));
				list_item=XmStringCreateSimple(new_string);
				XmListAddItemUnselected(input_module_dialog->calib_struct[component].
					scroll,list_item,0);
				XmStringFree(list_item);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"input_module_calib_control_CB.  Could not allocate memory");
			}
		} break;
		case input_module_calib_cont_solve_ID:
		{
			if (0<NUMBER_IN_LIST(Input_module_calib_data)(
				input_module_dialog->calib_struct[component].data_list))
			{
				input_module_calib_calibrate(input_module_dialog,component);
			}
			else
			{
				display_message(WARNING_MESSAGE,
			"input_module_calib_control_CB.  Must have at least one data point before solving");
			}
		} break;
		case input_module_calib_cont_delete_ID:
		{
			if (XmListGetSelectedPos(
				input_module_dialog->calib_struct[component].scroll,&select_list,
				&num_selected))
			{
				for (i=0;i<num_selected;i++)
				{
					if (new_data=FIND_BY_IDENTIFIER_IN_LIST(
						Input_module_calib_data,list_number)(select_list[i]-1,
						input_module_dialog->calib_struct[component].data_list))
					{
						REMOVE_OBJECT_FROM_LIST(Input_module_calib_data)(new_data,
							input_module_dialog->calib_struct[component].data_list);
					}
					else
					{
						display_message(WARNING_MESSAGE,
							"input_module_calib_control_CB.  Could not find calibration point");
					}
				}
				XmListDeleteAllItems(input_module_dialog->calib_struct[component].scroll);
				input_module_dialog->calib_struct[component].last_number=0;
				sum_position.component=component;
				sum_position.input_module_dialog=input_module_dialog;
				FOR_EACH_OBJECT_IN_LIST(Input_module_calib_data)(input_module_calib_add_display,
					&sum_position,input_module_dialog->calib_struct[component].data_list);
			}
		} break;
		case input_module_calib_cont_clear_ID:
		{
			for (i=0;i<3;i++)
			{
				input_module_dialog->calib_struct[component].offset[i]=0.0;
			}
			REMOVE_ALL_OBJECTS_FROM_LIST(Input_module_calib_data)(
				input_module_dialog->calib_struct[component].data_list);
			XmListDeleteAllItems(input_module_dialog->calib_struct[component].scroll);
			input_module_dialog->calib_struct[component].last_number=0;
		} break;
		default:
		{
			display_message(WARNING_MESSAGE,
				"input_module_calib_control_CB.  Invalid button number");
		} break;
	}
	LEAVE;
} /* input_module_calib_control_CB */

static void input_module_calib_destroy_CB(Widget w,int *tag,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 20 March 2000

DESCRIPTION :
Callback for the calib dialog - tidies up all memory allocation
==============================================================================*/
{
	int component,num_children;
	struct Input_module_dialog_struct *input_module_dialog;
	Widget *child_list;

	ENTER(input_module_calib_destroy_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	/* Get the pointer to the data for the input_module dialog */
	XtVaGetValues(w,
		XmNuserData,&component,
		XmNnumChildren,&num_children,
		XmNchildren,&child_list,
		NULL);
	/* get a child */
	if (2==num_children)
	{
		XtVaGetValues(child_list[0],XmNuserData,&input_module_dialog,NULL);
		input_module_dialog->calib_struct[component].dialog=(Widget)NULL;
		DESTROY_LIST(Input_module_calib_data)(
			&input_module_dialog->calib_struct[component].data_list);
		input_module_dialog->calib_struct[component].last_number=0;
	}
	else
	{
		display_message(WARNING_MESSAGE,"input_module_calib_destroy_CB.  Invalid widget");
	}
	LEAVE;
} /* input_module_calib_destroy_CB */

static void input_module_faro_identify(Widget w,int button_num,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 20 March 2000

DESCRIPTION :
Sets the id of widgets in the faro_calibration dialog.
==============================================================================*/
{
	struct Input_module_dialog_struct *input_module_dialog;

	ENTER(input_module_faro_identify);
	USE_PARAMETER(reason);
	XtVaGetValues(w,XmNuserData,&input_module_dialog,NULL);
	switch (button_num)
	{
		case input_module_faro_control_ID:
		{
			input_module_dialog->faro_struct.control=w;
		} break;
		case input_module_faro_scroll_ID:
		{
			input_module_dialog->faro_struct.scroll=w;
		} break;
		case input_module_faro_three_point_ID:
		{
			input_module_dialog->faro_struct.three_point=w;
		} break;
		default:
		{
			display_message(WARNING_MESSAGE,
				"input_module_faro_identify.  Invalid id number");
		} break;
	}
	LEAVE;
} /* input_module_faro_identify */

static void input_module_faro_control_CB(Widget w,int button_num,
	XmAnyCallbackStruct *reason)
/*******************************************************************************
LAST MODIFIED : 20 March 2000

DESCRIPTION :
Performs actions depending upon the button pressed.
==============================================================================*/
{
	struct Input_module_dialog_struct *input_module_dialog;
	XmString list_item;

	ENTER(input_module_faro_identify_button);
	USE_PARAMETER(reason);
	XtVaGetValues(w,XmNuserData,&input_module_dialog,NULL);
	switch (button_num)
	{
		case input_module_faro_three_point_ID:
		{
			if(input_module_dialog->faro_struct.calibrating)
			{
				input_module_faro_calibrate();
				input_module_dialog->faro_struct.calibrating = 0;
			}
			else
			{
				input_module_dialog->faro_struct.calibrating = 1;
				list_item=XmStringCreateSimple("Define your three points and then click this button again\n");
				XmListAddItemUnselected(input_module_dialog->faro_struct.scroll,
					list_item,0);
				XmStringFree(list_item);
			}
		} break;
		default:
		{
			display_message(WARNING_MESSAGE,
				"input_module_faro_control_CB.  Invalid button number");
		} break;
	}
	LEAVE;
} /* input_module_faro_control_CB */

static void input_module_faro_destroy_CB(Widget w,int *tag,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 20 March 2000

DESCRIPTION :
Callback for the faro dialog - tidies up all memory allocation
==============================================================================*/
{
	int component,num_children;
	struct Input_module_dialog_struct *input_module_dialog;
	Widget *child_list;

	ENTER(input_module_faro_destroy_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	/* Get the pointer to the data for the input_module dialog */
	XtVaGetValues(w,
		XmNuserData,&component,
		XmNnumChildren,&num_children,
		XmNchildren,&child_list,
		NULL);
	/* get a child */
	if (2==num_children)
	{
		XtVaGetValues(child_list[0],XmNuserData,&input_module_dialog,NULL);
		input_module_dialog->faro_struct.dialog=(Widget)NULL;
	}
	else
	{
		display_message(WARNING_MESSAGE,"input_module_faro_destroy_CB.  Invalid widget");
	}
	LEAVE;
} /* input_module_faro_destroy_CB */


static void input_module_dialog_control_CB(Widget w, int user_data,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 20 March 2000

DESCRIPTION :
The mode has been changed etc.
==============================================================================*/
{
	double data[3];
	struct Input_module_dialog_struct *input_module_dialog;

	ENTER(input_module_dialog_control_CB);
	USE_PARAMETER(reason);
	/* find out which input_module_dialog widget we are in */
	XtVaGetValues(w,XmNuserData,&input_module_dialog,NULL);
	switch (user_data)
	{
		case input_module_display_change:
		{
			if ( XmToggleButtonGadgetGetState ( w ))
			{
				input_module_dialog->scene_display = input_module_dialog->last_scene_display;
				XtSetSensitive( input_module_dialog->display_control_form,
					True );
				input_module_dialog_redraw_scene( input_module_dialog );
			}
			else
			{
				input_module_dialog->scene_display = NOT_VISIBLE;
				XtSetSensitive( input_module_dialog->display_control_form,
					False );
				input_module_dialog_redraw_scene( input_module_dialog );
			}
		} break;
		case input_module_set_origin:
		{
			if ( input_module_dialog->device != IM_DEVICE_NONE )
			{
				input_module_set ( input_module_dialog->device, IM_SET_CURRENT_AS_ORIGIN, NULL );
				input_module_get ( input_module_dialog->device, IM_POSITION_OFFSET, data );
				dof3_set_data( input_module_dialog->dof3_offset_widget[0],
					DOF3_DATA, data );
			}
		} break;
		case input_module_calibrate:
		{
			/* The zero indicates the component it applies to and so should
				not necessarily be zero */
			input_module_bring_up_calib(input_module_dialog, 0);
		} break;
		case input_module_cross:
		{
			input_module_dialog->last_scene_display = POINT_CROSS;
			input_module_dialog->scene_display = POINT_CROSS;
		} break;
		case input_module_pixel:
		{
			input_module_dialog->scene_display = POINT_PIXEL;
			input_module_dialog->last_scene_display = POINT_PIXEL;
		} break;
		case input_module_sphere:
		{
			input_module_dialog->scene_display = SURFACE_SPHERE;
			input_module_dialog->last_scene_display = SURFACE_SPHERE;
		} break;
		default:
		{
			display_message(WARNING_MESSAGE,
				"input_module_dialog_control_CB.  Invalid button number");
		} break;
	}
	LEAVE;
} /* input_module_dialog_control_CB */

static Widget create_input_module_dialog(Widget *input_module_dialog_widget,
	Widget parent,struct Graphical_material *material,
	struct MANAGER(Graphical_material) *material_manager,struct Scene *scene,
	struct MANAGER(Scene) *scene_manager, struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 21 November 2001

DESCRIPTION :
Allows the user to control the input_module.
==============================================================================*/
{
	int i, j;
	MrmType input_module_dialog_dialog_class;
	struct Callback_data callback;
	struct Input_module_dialog_struct *input_module_dialog = NULL;
	static MrmRegisterArg callback_list[]=
	{
		{"input_module_d_identify", (XtPointer)input_module_dialog_identify},
		{"input_module_d_destroy_CB", (XtPointer)input_module_dialog_destroy_CB},
		{"input_module_d_control_CB", (XtPointer)input_module_dialog_control_CB},
		{"input_calib_identify_button", (XtPointer)input_module_calib_identify_button},
		{"input_calib_destroy_CB", (XtPointer)input_module_calib_destroy_CB},
		{"input_calib_control_CB", (XtPointer)input_module_calib_control_CB},
		{"input_faro_identify", (XtPointer)input_module_faro_identify},
		{"input_faro_destroy_CB", (XtPointer)input_module_faro_destroy_CB},
		{"input_faro_control_CB", (XtPointer)input_module_faro_control_CB},
	};
	static MrmRegisterArg identifier_list[] =
	{
		{"input_module_d_structure", (XtPointer)NULL},
		{"input_module_chooser_form_ID", (XtPointer)input_module_chooser_form_ID},
		{"input_module_data_rowcol_ID", (XtPointer)input_module_data_rowcol_ID},
			{"input_module_scale_frame_ID", (XtPointer)input_module_scale_frame_ID},
		{"input_module_offset_frame_ID", (XtPointer)input_module_offset_frame_ID},
		{"input_module_form_label_ID", (XtPointer)input_module_form_label_ID},
		{"input_module_current_x_ID", (XtPointer)input_module_current_x_ID},
		{"input_module_current_y_ID", (XtPointer)input_module_current_y_ID},
		{"input_module_current_z_ID", (XtPointer)input_module_current_z_ID},
		{"input_module_display_change", (XtPointer)input_module_display_change},
		{"input_module_scene_chooser_ID", (XtPointer)input_module_scene_chooser_ID},
		{"input_module_material_ID", (XtPointer)input_module_material_ID},
		{"input_module_display_control_ID", (XtPointer)input_module_display_control_ID},
		{"input_module_set_origin", (XtPointer)input_module_set_origin},
		{"input_module_calibrate", (XtPointer)input_module_calibrate},
		{"input_module_cross", (XtPointer)input_module_cross},
		{"input_module_pixel", (XtPointer)input_module_pixel},
		{"input_module_sphere", (XtPointer)input_module_sphere},
			/* Calibration dialog stuff */
		{"input_calib_control_ID", (XtPointer)input_module_calib_control_ID},
		{"input_calib_scroll_ID", (XtPointer)input_module_calib_scroll_ID},
		{"input_calib_cont_record_ID", (XtPointer)input_module_calib_cont_record_ID},
		{"input_calib_cont_clear_ID", (XtPointer)input_module_calib_cont_clear_ID},
		{"input_calib_cont_solve_ID", (XtPointer)input_module_calib_cont_solve_ID},
		{"input_calib_cont_delete_ID", (XtPointer)input_module_calib_cont_delete_ID},
			/* Calibration dialog stuff */
		{"input_faro_control_ID", (XtPointer)input_module_faro_control_ID},
		{"input_faro_scroll_ID", (XtPointer)input_module_faro_scroll_ID},
		{"input_faro_three_point_ID", (XtPointer)input_module_faro_three_point_ID},
	};
	Widget return_widget;

	ENTER(create_input_module_dialog);
	return_widget = (Widget)NULL;
	if (MrmOpenHierarchy_base64_string(input_module_dialog_uidh,
		&input_module_dialog_hierarchy,&input_module_dialog_hierarchy_open))
	{
		/* allocate memory */
		if (ALLOCATE(input_module_dialog,struct Input_module_dialog_struct,1))
		{
			/* initialise the structure */
			input_module_dialog->device = IM_DEVICE_NONE;
			input_module_dialog->form_mode = INPUT_MODULE_NO_FORMS;
			input_module_dialog->dialog_address = input_module_dialog_widget;
			input_module_dialog->device_chooser_form = (Widget)NULL;
			input_module_dialog->device_chooser_widget = (Widget)NULL;
			input_module_dialog->widget = (Widget)NULL;
			input_module_dialog->dialog_parent = (Widget)NULL;
			input_module_dialog->data_rowcol = (Widget)NULL;
			matrix_I(&input_module_dialog->last_direction);
			for ( i = 0 ; i < 3 ; i++ )
			{
				input_module_dialog->data_widget[i] = (Widget)NULL;
				input_module_dialog->data_rc_form[i] = (Widget)NULL;
				input_module_dialog->scale_frame[i] = (Widget)NULL;
				input_module_dialog->offset_frame[i] = (Widget)NULL;
				input_module_dialog->dof3_scale_widget[i] = (Widget)NULL;
				input_module_dialog->dof3_offset_widget[i] = (Widget)NULL;
				input_module_dialog->current_x_widget[i] = (Widget)NULL;
				input_module_dialog->current_y_widget[i] = (Widget)NULL;
				input_module_dialog->current_z_widget[i] = (Widget)NULL;
			}
			input_module_dialog->display_control_form = (Widget)NULL;
			input_module_dialog->scene_chooser_widget = (Widget)NULL;
			input_module_dialog->scene_chooser_form = (Widget)NULL;
			input_module_dialog->material_chooser_widget = (Widget)NULL;
			input_module_dialog->material_chooser_form = (Widget)NULL;
			input_module_dialog->scene = scene;
			input_module_dialog->scene_display = NOT_VISIBLE;
			input_module_dialog->last_scene_display = POINT_CROSS;
			input_module_dialog->material = material;

			input_module_dialog->cross_pointset = (struct GT_pointset *)NULL;
			input_module_dialog->pixel_pointset = (struct GT_pointset *)NULL;
			input_module_dialog->sphere_surface = (struct GT_surface *)NULL;
			input_module_dialog->graphics_object = (struct GT_object *)NULL;

			for ( i = 0 ; i < 3 ; i++ )
			{
				input_module_dialog->calib_struct[i].dialog=(Widget)NULL;
				input_module_dialog->calib_struct[i].widget=(Widget)NULL;
				input_module_dialog->calib_struct[i].control=(Widget)NULL;
				input_module_dialog->calib_struct[i].scroll=(Widget)NULL;
				input_module_dialog->calib_struct[i].control_record=(Widget)NULL;
				input_module_dialog->calib_struct[i].control_solve=(Widget)NULL;
				input_module_dialog->calib_struct[i].control_delete=(Widget)NULL;
				input_module_dialog->calib_struct[i].control_clear=(Widget)NULL;
				input_module_dialog->calib_struct[i].last_number=0;
				input_module_dialog->calib_struct[i].data_list =
					(struct LIST(Input_module_calib_data) *)NULL;
				for ( j = 0 ; j < 3 ; j++)
				{
					input_module_dialog->calib_struct[i].global[j]=0.0;
					input_module_dialog->calib_struct[i].offset[j]=0.0;
				}
			}

			/* make the dialog shell */
			if (input_module_dialog->dialog = XtVaCreatePopupShell(
				"Input Module Controller",topLevelShellWidgetClass,parent,
				XmNallowShellResize,TRUE,NULL))
			{
				/* register the callbacks */
				if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
					input_module_dialog_hierarchy,callback_list,XtNumber(callback_list)))
				{
					/* assign and register the identifiers */
					identifier_list[0].value=(XtPointer)input_module_dialog;
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
						input_module_dialog_hierarchy,identifier_list,
						XtNumber(identifier_list)))
					{
						/* fetch position window widget */
						if (MrmSUCCESS==MrmFetchWidget(input_module_dialog_hierarchy,
							"input_module_dialog_widget",input_module_dialog->dialog,
							&(input_module_dialog->widget),
							&input_module_dialog_dialog_class))
						{
							XtManageChild(input_module_dialog->widget);

							/* add the input menu */
							if (create_input_module_widget(
								&(input_module_dialog->device_chooser_widget),
								input_module_dialog->device_chooser_form ))
							{
								Input_module_add_device_change_callback(
									input_module_dialog->device_chooser_widget,
									input_module_dialog_change_device,
									(void *)input_module_dialog);
								if (input_module_dialog->scene_chooser_widget =
									CREATE_CHOOSE_OBJECT_WIDGET(Scene)(
									input_module_dialog->scene_chooser_form,
									input_module_dialog->scene,scene_manager,
									(MANAGER_CONDITIONAL_FUNCTION(Scene) *)NULL, (void *)NULL,
									user_interface))
								{
									callback.data = (void *)input_module_dialog;
									callback.procedure=
										input_module_dialog_update_scene;
									CHOOSE_OBJECT_SET_CALLBACK(Scene)(
										input_module_dialog->scene_chooser_widget,&callback);
									if (input_module_dialog->material_chooser_widget =
										CREATE_CHOOSE_OBJECT_WIDGET(Graphical_material)(
										input_module_dialog->material_chooser_form,
										input_module_dialog->material,material_manager,
										(MANAGER_CONDITIONAL_FUNCTION(Graphical_material) *)NULL,
										(void *)NULL, user_interface))
									{
										callback.data = (void *)input_module_dialog;
										callback.procedure=
											input_module_dialog_update_material;
										CHOOSE_OBJECT_SET_CALLBACK(Graphical_material)(
											input_module_dialog->material_chooser_widget,&callback);

										XtRealizeWidget(input_module_dialog->dialog);
										XtPopup(input_module_dialog->dialog, XtGrabNone);

										/* SAB I want this sensitivity to be false by default
											but if I set it that way sooner then the chooser
											crashes when clicked on */
										XtSetSensitive( input_module_dialog->display_control_form, False );

										return_widget = input_module_dialog->dialog;
									}
									else
									{
										display_message(ERROR_MESSAGE,
												"create_input_module_dialog.  Could not create material chooser");
										DEALLOCATE(input_module_dialog);
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"create_input_module_dialog.  Could not create scene chooser");
									DEALLOCATE(input_module_dialog);
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
										"create_input_module_dialog.  Could not create input module chooser widget");
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
		"create_input_module_dialog.  Could not fetch input_module_dialog dialog");
							DEALLOCATE(input_module_dialog);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_input_module_dialog.  Could not register identifiers");
						DEALLOCATE(input_module_dialog);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_input_module_dialog.  Could not register callbacks");
					DEALLOCATE(input_module_dialog);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_input_module_dialog.  Could not create popup shell.");
				DEALLOCATE(input_module_dialog);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
"create_input_module_dialog.  Could not allocate input_module_dialog widget structure");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_input_module_dialog.  Could not open hierarchy");
	}
	*input_module_dialog_widget = return_widget;
	LEAVE;

	return (return_widget);
} /* create_input_module_dialog */

/*
Global functions
----------------
*/

int bring_up_input_module_dialog(Widget *input_module_dialog_address,
	Widget parent,struct Graphical_material *material,
	struct MANAGER(Graphical_material) *material_manager,struct Scene *scene,
	struct MANAGER(Scene) *scene_manager, struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 21 November 2001

DESCRIPTION :
If there is a input_module dialog in existence, then bring it to the front, else
create a new one.  Assumes we will only ever want one input_module controller at
a time.  This implementation may be changed later.
==============================================================================*/
{
	int return_code;

	ENTER(bring_up_input_module_dialog);
	if (input_module_dialog_address)
	{
		/* does it exist */
		if (*input_module_dialog_address)
		{
			XtPopup(*input_module_dialog_address,XtGrabNone);
			return_code=1;
		}
		else
		{
			if (create_input_module_dialog(input_module_dialog_address,parent,
				material,material_manager,scene,scene_manager,user_interface))
			{
				return_code=1;
			}
			else
			{
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"bring_up_input_module_dialog.  Could not open hierarchy");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* bring_up_input_module_dialog */

#endif /* EXT_INPUT */
