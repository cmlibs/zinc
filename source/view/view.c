/*******************************************************************************
FILE : view.c

LAST MODIFIED : 13 December 1996

DESCRIPTION :
This module creates a view widget that will return a global camera position
and orientation.  This widget contains a toggle to switch between camera and
point of interest modes.  For each mode, the widget contains a separate
widget which gets input from the user according to where the user wishes
to look.
The user may input their viewing position relative to the origin of a model
component (ie heart etc), and the view widget will interact with the models
coordinate system to return global values.
Output from this widget is used in the following way by OPENGL and GL to create
a viewing matrix.
if defined (GL_API)
		mmode(MVIEWING);
		start from scratch
		loadmatrix(idmat);
		This is the final (cosmetic) transformation that makes the z axis be
			'up', and look down the z direction
		rotate(-900,'z');
		rotate(900,'y');
		These are the orientation transformations
		rotate(-angle[2],'x');
		rotate(-angle[1],'y');
		rotate(-angle[0],'z');
		and translate back
		translate(-position[0],-position[1],-position[2]);
endif
if defined (OPENGL_API)
		glMatrixMode(GL_MODELVIEW);
		start from scratch
		glLoadIdentity();
		This is the final (cosmetic) transformation that makes the z axis be
			'up', and look down the z direction
		glRotated(-90,0,0,1);
		glRotated(90,0,1,0);
		These are the orientation transformations
		glRotated(-angle[2],1,0,0);
		glRotated(-angle[1],0,1,0);
		glRotated(-angle[0],0,0,1);
		and translate back
		glTranslated(-position[0],-position[1],-position[2]);
endif
==============================================================================*/
#include <math.h>
#include <stdio.h>
#include <Xm/ToggleBG.h>
#include "general/debug.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"
#include "view/coord.h"
#include "view/coord_trans.h"
#include "view/view.h"
#include "view/view.uid64"

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int view_hierarchy_open=0;
static MrmHierarchy view_hierarchy;
#endif /* defined (MOTIF) */

/*
Module functions
----------------
*/
void poi_to_camera(struct Poi_data *old_data,struct Camera_data *new_data)
/*******************************************************************************
LAST MODIFIED : 28 December 1995

DESCRIPTION :
Converts from point of interest data to the equivalent position and
orientation.
==============================================================================*/
{
	int i;
	struct Dof3_data x_vector,y_vector;
	VIEW_PRECISION work,temp,z_z,sum_squares_x;

	ENTER(poi_to_camera);
	poi_coordinate_ptr->origin.position.data[0]=old_data->poi.data[0];
	poi_coordinate_ptr->origin.position.data[1]=old_data->poi.data[1];
	poi_coordinate_ptr->origin.position.data[2]=old_data->poi.data[2];
	for(i=0;i<3;i++)
	{
		new_data->position.data[i] = old_data->position.data[i];
		x_vector.data[i] = old_data->poi.data[i]-old_data->position.data[i];
	}
	/* calculate the azimuth and elevation */
	temp = atan2(x_vector.data[1],
		x_vector.data[0]);
	new_data->direction.data[0] = temp/PI_180;
	work = cos(temp);
	if(work!=0)
	{
		new_data->direction.data[1] = atan2(-x_vector.data[2],
			x_vector.data[0]/work);
	}
	else
	{
		work = sin(temp);
		new_data->direction.data[1] = atan2(-x_vector.data[2],
			x_vector.data[1]/work);
	}
	/* work out the z component of the y and z axes */
	/* first, x cross up_vector = y axis */
	y_vector.data[0] = x_vector.data[1]*old_data->up_vector.data[2]-
		x_vector.data[2]*old_data->up_vector.data[1];
	y_vector.data[1] = x_vector.data[2]*old_data->up_vector.data[0]-
		x_vector.data[0]*old_data->up_vector.data[2];
	y_vector.data[2] = x_vector.data[0]*old_data->up_vector.data[1]-
		x_vector.data[1]*old_data->up_vector.data[0];
	/* make the y and x vectors have unit length */
	sum_squares_x = 0.0;
	for(i=0;i<3;i++)
	{
		sum_squares_x += x_vector.data[i]*x_vector.data[i];
	}
	sum_squares_x = sqrt(sum_squares_x);
	/* now x cross y = z axis */
	z_z = (x_vector.data[0]*y_vector.data[1]-
		x_vector.data[1]*y_vector.data[0]);
	/* an we know the roll is the atan of the two z components */
	new_data->direction.data[2] = atan2(y_vector.data[2],
		z_z/sum_squares_x)/PI_180;
	/* remember to scale the direction by PI_180 */
	new_data->direction.data[1] /= PI_180;
	LEAVE;
} /* poi_to_camera */

void camera_to_poi(struct Camera_data *old_data,struct Poi_data *new_data)
/*******************************************************************************
LAST MODIFIED : 20 January 1995

DESCRIPTION :
Converts from camera position and orientation to a eye position, point of
interest and an up vector.
Note: this is not a unqiue transformation as the poi may be anywhere along
the line of sight.  The up vector should be taken as the -z axis of the
orientation matrix, but we will take it as 0,0,1 so that it works ok with the
vector widget.
==============================================================================*/
{
	int i,max_num;
	VIEW_PRECISION max_val;
	Gmatrix work;

	ENTER(camera_to_poi);
	euler_matrix(&old_data->direction,&work);
	for(i=0;i<3;i++)
	{
		new_data->position.data[i] = old_data->position.data[i];
		/* the x axis is the view direction, so make the poi just
			this vector away from the eye position */
		new_data->poi.data[i] = old_data->position.data[i]+work.data[0][i];
		new_data->up_vector.data[i] = 0.0;
	}
	/* find out in which direction the up vector points predominantly */
	max_val = -1.0;
	max_num = 0;
	for(i=0;i<3;i++)
	{
		if (fabs(work.data[2][i])>max_val)
		{
			max_val = fabs(work.data[2][i]);
			max_num = i;
		}
	}
	if (work.data[2][max_num]<0.0) /* set it to the -ve of this value */
	{
		new_data->up_vector.data[max_num] = 1.0; /* make it in the z dir */
	}
	else
	{
		new_data->up_vector.data[max_num] = -1.0; /* make it in the z dir */
	}
	LEAVE;
} /* camera_to_poi */

void view_update(struct View_struct *temp_view)
/*******************************************************************************
LAST MODIFIED : 8 September 1994

DESCRIPTION :
Tells CMGUI about the current values.
==============================================================================*/
{
	ENTER(view_update);
	if(temp_view->callback_array[VIEW_UPDATE_CB].procedure)
	{
		/* now call the procedure with the user data and the position data */
		(temp_view->callback_array[VIEW_UPDATE_CB].procedure)
			(temp_view->widget,
			temp_view->callback_array[VIEW_UPDATE_CB].data,&temp_view->current_value);
	}
	LEAVE;
} /* view_update */

void view_update_relative(Widget camera_widget,void *user_data,void *temp_cam)
/*******************************************************************************
LAST MODIFIED : 8 September 1994

DESCRIPTION :
Receives an update from the camera dialog, and converts it to the 'standard'
format, then updates any clients of the view widget.
==============================================================================*/
{
	int i;
	struct View_struct *temp_view = user_data;
	struct Camera_data *temp_camera = temp_cam;
	ENTER(view_update_relative);
	if(temp_view->mode==VIEW_RELATIVE_MODE)
	{
		/* convert it to standard - no conversion */
		for(i=0;i<3;i++)
		{
			temp_view->current_value.position.data[i] = temp_camera->position.data[i];
			temp_view->current_value.direction.data[i] = temp_camera->direction.data[i];
		}
		view_update(temp_view);
	}
	LEAVE;
} /* view_update_relative */

void view_update_camera(Widget camera_widget,void *user_data,void *temp_cam)
/*******************************************************************************
LAST MODIFIED : 8 September 1994

DESCRIPTION :
Receives an update from the camera dialog, and converts it to the 'standard'
format, then updates any clients of the view widget.
==============================================================================*/
{
	int i;
	struct View_struct *temp_view = user_data;
	struct Camera_data *temp_camera = temp_cam;

	ENTER(view_update_camera);
	if(temp_view->mode==VIEW_CAMERA_MODE)
	{
		/* convert it to standard - no conversion */
		for(i=0;i<3;i++)
		{
			temp_view->current_value.position.data[i] = temp_camera->position.data[i];
			temp_view->current_value.direction.data[i] = temp_camera->direction.data[i];
		}
		view_update(temp_view);
	}
	LEAVE;
} /* view_update_camera */

void view_update_poi(Widget camera_widget,void *user_data,void *temp_pointerest)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Receives an update from the camera dialog, and converts it to the 'standard'
format, then updates any clients of the view widget.
==============================================================================*/
{
	struct View_struct *temp_view = user_data;
	struct Poi_data *temp_poi = temp_pointerest;

	ENTER(view_update_poi);
	if(temp_view->mode==VIEW_POI_MODE)
	{
		poi_to_camera(temp_poi,&temp_view->current_value);
		view_update(temp_view);
	}
	LEAVE;
} /* view_update_poi */

static void view_identify_button(Widget w,int button_num,unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Finds the id of the buttons on the view widget.
==============================================================================*/
{
	struct View_struct *temp_view;

	ENTER(view_identify_button);
	/* find out which view widget we are in */
	XtVaGetValues(w,XmNuserData,&temp_view,NULL);
	switch (button_num)
	{
		case view_cam_form_ID:
		{
			temp_view->cam_form = w;
		}; break;
		case view_toggle_camera_button_ID:
		{
			temp_view->toggle[VIEW_CAMERA_MODE] = w;
		}; break;
		case view_toggle_poi_button_ID:
		{
			temp_view->toggle[VIEW_POI_MODE] = w;
		}; break;
		case view_toggle_relative_button_ID:
		{
			temp_view->toggle[VIEW_RELATIVE_MODE] = w;
		}; break;
		default:
		{
		display_message(WARNING_MESSAGE,
			"view_identify_button.  Invalid button number");
		} break;
	}
	LEAVE;
} /* view_identify_button */

static void view_destroy_CB(Widget w, int *tag, unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Callback for the viewment dialog - tidies up all details - mem etc
==============================================================================*/
{
	struct View_struct *temp_view;

	ENTER(view_destroy_CB);
	/* Get the pointer to the data for the view widget */
	XtVaGetValues(w,XmNuserData,&temp_view,NULL);
	/* deallocate the memory for the user data */
	printf("De-allocating memory.\n");
	DEALLOCATE(temp_view);
	LEAVE;
} /* view_destroy_CB */

static void view_toggle_CB(Widget w, int *tag,
	XmToggleButtonCallbackStruct *reason)
/*******************************************************************************
LAST MODIFIED : 20 March 1994

DESCRIPTION :
Callback for the view widget.  Processes any changes that have occurred to the
slider.
==============================================================================*/
{
	int button_num = *tag;
	struct View_struct *temp_view;
	struct Poi_data temp_poi_value;

	ENTER(view_toggle_CB);
	/* Get the pointer to the data for the view widget */
	XtVaGetValues(w,XmNuserData,&temp_view,NULL);
	if ((button_num>=0)&&(button_num<VIEW_NUM_MODES))
	{
		if (reason->set)
		{
			switch (button_num)
			{
				case VIEW_CAMERA_MODE:
				{
					camera_set_data(temp_view->sub_widget[VIEW_CAMERA_MODE],
						CAMERA_DATA,&temp_view->current_value);
				}; break;
				case VIEW_POI_MODE:
				{
					camera_to_poi(&temp_view->current_value,&temp_poi_value);
					/* update current value with the poi version of the view */
					poi_to_camera(&temp_poi_value,&temp_view->current_value);
					poi_set_data(temp_view->sub_widget[VIEW_POI_MODE],
						POI_DATA,&temp_poi_value);
				}; break;
				case VIEW_RELATIVE_MODE:
				{
					camera_set_data(temp_view->sub_widget[VIEW_RELATIVE_MODE],
						CAMERA_DATA,&temp_view->current_value);
				}; break;
			}
			XtManageChild(temp_view->sub_widget[button_num]);
			temp_view->mode = (enum View_mode)button_num;
			view_update(temp_view);
		}
		else
		{
			XtUnmanageChild(temp_view->sub_widget[button_num]);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"view_toggle_CB.  Incorrect button number.");
	}
	LEAVE;
} /* view_toggle_CB */


/*
Global functions
----------------
*/
Widget create_view_widget(Widget parent,enum View_mode mode)
/*******************************************************************************
LAST MODIFIED : 13 December 1996

DESCRIPTION :
Creates a view widget that will either be in camera or poi mode, and will
return a global position and orientation of the camera.
==============================================================================*/
{
	int i,init_widgets;
	MrmType view_dialog_class;
	struct Callback_data callback;
	struct View_struct *temp_view = NULL;
	struct Poi_data temp_poi_value;
	static MrmRegisterArg callback_list[]=
	{
		{"view_identify_button",(XtPointer)view_identify_button},
		{"view_destroy_CB",(XtPointer)view_destroy_CB},
		{"view_toggle_CB",(XtPointer)view_toggle_CB}
	};
	static MrmRegisterArg identifier_list[] =
	{
		{"View_structure",(XtPointer)NULL},
		{"view_toggle_camera_button_ID",(XtPointer)view_toggle_camera_button_ID},
		{"view_toggle_poi_button_ID",(XtPointer)view_toggle_poi_button_ID},
		{"view_toggle_relative_button_ID",
			(XtPointer)view_toggle_relative_button_ID},
		{"view_cam_form_ID",(XtPointer)view_cam_form_ID}
	};
	Widget return_widget;

	ENTER(create_view_widget);
	return_widget = (Widget)NULL;
	if (MrmOpenHierarchy_base64_string(view_uid64,
		&view_hierarchy,&view_hierarchy_open))
	{
		/* allocate memory */
		if (ALLOCATE(temp_view,struct View_struct,1))
		{
			/* initialise the structure */
			temp_view->widget_parent = parent;
			temp_view->widget = (Widget)NULL;
			temp_view->mode = mode;
			for(i=0;i<VIEW_NUM_MODES;i++)
			{
				temp_view->sub_widget[i] = (Widget)NULL;
				temp_view->toggle[i] = (Widget)NULL;
			}
			temp_view->cam_form = (Widget)NULL;
			for(i=0;i<3;i++)
			{
				temp_view->current_value.position.data[i] = 0.0;
				temp_view->current_value.direction.data[i] = 0.0;
				temp_poi_value.position.data[i] =
					temp_view->current_value.position.data[i];
				temp_poi_value.poi.data[i] =
					temp_view->current_value.direction.data[i];
				temp_poi_value.up_vector.data[i] = 0.0;
			}
			temp_poi_value.up_vector.data[2] = 1.0;
			for(i=0;i<VIEW_NUM_CALLBACKS;i++)
			{
				temp_view->callback_array[i].procedure = (Callback_procedure *)NULL;
				temp_view->callback_array[i].data = NULL;
			}
			/* register the callbacks */
			if (MrmSUCCESS==MrmRegisterNamesInHierarchy(view_hierarchy,callback_list,
				XtNumber(callback_list)))
			{
				/* assign and register the identifiers */
				identifier_list[0].value=(XtPointer)temp_view;
				if (MrmSUCCESS==MrmRegisterNamesInHierarchy(view_hierarchy,
					identifier_list,XtNumber(identifier_list)))
				{
					/* fetch position window widget */
					if (MrmSUCCESS==MrmFetchWidget(view_hierarchy,"view_widget",
						temp_view->widget_parent,&(temp_view->widget),&view_dialog_class))
					{
						XtManageChild(temp_view->widget);
						/* set the mode toggle to the correct position */
						XmToggleButtonGadgetSetState(temp_view->toggle[temp_view->mode],
							TRUE,FALSE);
						init_widgets = 1;
						if (!create_camera_widget(
							&(temp_view->sub_widget[VIEW_CAMERA_MODE]),temp_view->cam_form,
							&temp_view->current_value,CAMERA_ABSOLUTE))
						{
							display_message(ERROR_MESSAGE,
								"create_view_widget.  Could not create camera widget.");
							init_widgets = 0;
						}
						if (!create_poi_widget(&(temp_view->sub_widget[VIEW_POI_MODE]),
							temp_view->cam_form,&temp_poi_value))
						{
							display_message(ERROR_MESSAGE,
								"create_view_widget.  Could not create poi widget.");
							init_widgets = 0;
						}
						if (!create_camera_widget(
							&(temp_view->sub_widget[VIEW_RELATIVE_MODE]),
							temp_view->cam_form,&temp_view->current_value,CAMERA_RELATIVE))
						{
							display_message(ERROR_MESSAGE,
								"create_view_widget.  Could not create relative widget.");
							init_widgets = 0;
						}
						if(init_widgets)
						{
							/* set up the callbacks for each of the widgets */
							callback.procedure = view_update_camera;
							callback.data = temp_view;
							camera_set_data(temp_view->sub_widget[VIEW_CAMERA_MODE],
								CAMERA_UPDATE_CB,&callback);
							callback.procedure = view_update_poi;
							callback.data = temp_view;
							poi_set_data(temp_view->sub_widget[VIEW_POI_MODE],
								POI_UPDATE_CB,&callback);
							callback.procedure = view_update_relative;
							callback.data = temp_view;
							camera_set_data(temp_view->sub_widget[VIEW_RELATIVE_MODE],
								CAMERA_UPDATE_CB,&callback);
							/* unmanage those that arent current */
#if defined (CODE_FRAGMENTS)
							for(i=0;i<VIEW_NUM_MODES;i++)
							{
								if (i!=temp_view->mode)
								{
									XtUnmanageChild(temp_view->sub_widget[i]);
								}
								else
								{
									XtManageChild(temp_view->sub_widget[i]);
								}
							}
#endif
							for (i=0;i<VIEW_NUM_MODES;i++)
							{
								XtUnmanageChild(temp_view->sub_widget[i]);
							}
							XtManageChild(temp_view->sub_widget[temp_view->mode]);
							return_widget = temp_view->widget;
						}
						else
						{
							DEALLOCATE(temp_view);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_view_widget.  Could not fetch view dialog");
						DEALLOCATE(temp_view);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_view_widget.  Could not register identifiers");
					DEALLOCATE(temp_view);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_view_widget.  Could not register callbacks");
				DEALLOCATE(temp_view);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_view_widget.  Could not allocate view widget structure");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_view_widget.  Could not open hierarchy");
	}
	LEAVE;

	return (return_widget);
} /* create_view_widget */

int view_set_data(Widget view_widget,enum View_data_type data_type,void *data)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Changes a data item of the view widget.
==============================================================================*/
{
	int i,return_code;
	struct View_struct *temp_view;
	struct Camera_data temp_camera;
	struct Poi_data temp_poi_value;

	ENTER(view_set_data);
/*???debug */
printf("enter view_set_data\n");
	/* Get the pointer to the data for the view dialog */
	XtVaGetValues(view_widget,XmNuserData,&temp_view,NULL);
	switch(data_type)
	{
		case VIEW_UPDATE_CB:
		{
			temp_view->callback_array[VIEW_UPDATE_CB].procedure =
				((struct Callback_data *)data)->procedure;
			temp_view->callback_array[VIEW_UPDATE_CB].data =
				((struct Callback_data *)data)->data;
			return_code = 1;
		}; break;
		case VIEW_CAMERA_DATA:
		{
			camera_to_poi((struct Camera_data *)data,&temp_poi_value);
			for(i=0;i<3;i++)
			{
				temp_view->current_value.position.data[i] =
					((struct Camera_data *)data)->position.data[i];
				temp_view->current_value.direction.data[i] =
					((struct Camera_data *)data)->direction.data[i];
			}
			camera_set_data(temp_view->sub_widget[VIEW_CAMERA_MODE],CAMERA_DATA,
				&temp_view->current_value);
			camera_set_data(temp_view->sub_widget[VIEW_RELATIVE_MODE],CAMERA_DATA,
				&temp_view->current_value);
			poi_set_data(temp_view->sub_widget[VIEW_POI_MODE],POI_DATA,
				&temp_poi_value);
			view_update(temp_view);
			return_code = 1;
		}; break;
		case VIEW_POI_DATA:
		{
			poi_to_camera((struct Poi_data *)data,&temp_camera);
			for(i=0;i<3;i++)
			{
				temp_view->current_value.position.data[i] =
					temp_camera.position.data[i];
				temp_view->current_value.direction.data[i] =
					temp_camera.direction.data[i];
			}
			camera_set_data(temp_view->sub_widget[VIEW_CAMERA_MODE],CAMERA_DATA,
				&temp_view->current_value);
			camera_set_data(temp_view->sub_widget[VIEW_RELATIVE_MODE],CAMERA_DATA,
				&temp_view->current_value);
			poi_set_data(temp_view->sub_widget[VIEW_POI_MODE],POI_DATA,
				(struct Poi_data *)data);
			view_update(temp_view);
			return_code = 1;
		}; break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"view_set_data.  Invalid data type.");
			return_code = 0;
		}; break;
	}
/*???debug */
printf("leave view_set_data\n");
	LEAVE;

	return (return_code);
} /* view_set_data */

void *view_get_data(Widget view_widget,enum View_data_type data_type)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Changes a data item of the view widget.
==============================================================================*/
{
	void *return_code;
	struct View_struct *temp_view;
	static struct Callback_data dat_callback;
	static struct Poi_data temp_poi_value;
	static Widget dat_widget;

	ENTER(view_get_data);
	/* Get the pointer to the data for the view dialog */
	XtVaGetValues(view_widget,XmNuserData,&temp_view,NULL);
	switch(data_type)
	{
		case VIEW_UPDATE_CB:
		{
			dat_callback.procedure =
				temp_view->callback_array[VIEW_UPDATE_CB].procedure;
			dat_callback.data = temp_view->callback_array[VIEW_UPDATE_CB].data;
			return_code = &dat_callback;
		}; break;
		case VIEW_CAMERA_DATA:
		{
			return_code = &temp_view->current_value;
		}; break;
		case VIEW_POI_DATA:
		{
			camera_to_poi(&temp_view->current_value,&temp_poi_value);
			return_code = &temp_poi_value;
		}; break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"view_get_data.  Invalid data type.");
			return_code = NULL;
		}; break;
	}
	LEAVE;

	return (return_code);
} /* view_get_data */
