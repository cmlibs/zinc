/*******************************************************************************
FILE : poi.c

LAST MODIFIED : 13 December 1996

DESCRIPTION :
This module creates a free poi input device, using two dof3, two control and
one input widget.  The position is given relative to some coordinate system,
and the returned value is a global one.
==============================================================================*/
#include <math.h>
#include <stdio.h>
#include "dof3/dof3.h"
#include "dof3/dof3_control.h"
#if defined (EXT_INPUT)
#include "dof3/dof3_input.h"
#endif
#include "general/debug.h"
#include "view/coord.h"
#include "view/coord_trans.h"
#include "view/poi.h"
#include "view/poi.uidh"
#include "view/vector.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int poi_hierarchy_open=0;
static MrmHierarchy poi_hierarchy;
#endif /* defined (MOTIF) */
/* these two variables are set up so that we can force the eye position to be
	dependant upon the location of the point of interest, and orientation of the
	coordinate system that the point of interest is dependant upon */
/* ideally this should be set to NaN's to make sure it is never used */
struct Cmgui_coordinate poi_rel_coordinate=
	{"Relative",0,{{9999,9999,9999},{9999,9999,9999}}};
struct Cmgui_coordinate *poi_rel_coordinate_ptr= &poi_rel_coordinate;

/*
Module functions
----------------
*/
static void poi_update(struct Poi_struct *temp_poi)
/*******************************************************************************
LAST MODIFIED : 26 September 1995

DESCRIPTION :
Tells CMGUI about the current values.
==============================================================================*/
{
	ENTER(poi_update);
	if (temp_poi->callback_array[POI_UPDATE_CB].procedure)
	{
		/* now call the procedure with the user data and the position data */
		(temp_poi->callback_array[POI_UPDATE_CB].procedure)
			(temp_poi->widget,
				temp_poi->callback_array[POI_UPDATE_CB].data,
				&temp_poi->current_value);
	}
	LEAVE;
} /* poi_update */

static void get_poi_relative_local_position(struct Poi_struct *temp_poi,
	struct Dof3_data *local_values)
/*******************************************************************************
LAST MODIFIED : 26 September 1995

DESCRIPTION :
Works out the local position of the camera, based on the origin of the poi,
and the orientation of the up vector.
==============================================================================*/
{
	int i;
	Gmatrix orientation;
	POI_PRECISION temp;
	struct Dof3_data euler;

	ENTER(get_poi_relative_local_position);
	/* these formulae came from view/coord_trans.c routine get_local_position */
	for (i=0;i<3;i++)
	{
		local_values->data[i] = temp_poi->current_value.position.data[i]-
			temp_poi->current_value.poi.data[i];
	}
	/* now work out the euler angles of the up vector */
	euler.data[0] = 0.0; /* no azimuth as this is not a unique transformation */
	euler.data[1] = atan2(temp_poi->current_value.up_vector.data[0],
		temp_poi->current_value.up_vector.data[2]);
	temp = sin(euler.data[1]);
	if (temp)
	{
		euler.data[2] = atan2(-temp_poi->current_value.up_vector.data[1],
			temp_poi->current_value.up_vector.data[0]/temp)/PI_180;
	}
	else
	{
		euler.data[2] = atan2(-temp_poi->current_value.up_vector.data[1],
			temp_poi->current_value.up_vector.data[2]/cos(euler.data[1]))/PI_180;
	}
	euler.data[1] /= PI_180;
	/* get the orientation matrix */
	euler_matrix(&euler,&orientation);
	matrix_premult_vector(&(local_values->data[0]),&orientation);
	LEAVE;
} /* get_poi_relative_local_position */

static void get_poi_relative_global_position(struct Poi_struct *temp_poi,
	struct Dof3_data *local_values)
/*******************************************************************************
LAST MODIFIED : 26 September 1995

DESCRIPTION :
Works out the local position of the camera, based on the origin of the poi,
and the orientation of the up vector.
==============================================================================*/
{
	int i;
	Gmatrix orientation;
	POI_PRECISION temp;
	struct Dof3_data euler;

	ENTER(get_poi_relative_global_position);
	/* copy the values */
	for (i=0;i<3;i++)
	{
		temp_poi->current_value.position.data[i] = local_values->data[i];
	}
	/* now work out the orientation of the up vector */
	euler.data[0] = 0.0; /* no azimuth as this is not a unique transformation */
	euler.data[1] = atan2(temp_poi->current_value.up_vector.data[0],
		temp_poi->current_value.up_vector.data[2]);
	temp = sin(euler.data[1]);
	if (temp)
	{
		euler.data[2] = atan2(-temp_poi->current_value.up_vector.data[1],
			temp_poi->current_value.up_vector.data[0]/temp)/PI_180;
	}
	else
	{
		euler.data[2] = atan2(-temp_poi->current_value.up_vector.data[1],
			temp_poi->current_value.up_vector.data[2]/cos(euler.data[1]))/PI_180;
	}
	euler.data[1] /= PI_180;
	/* get the orientation matrix */
	euler_matrix(&euler,&orientation);
	matrix_postmult_vector(&(temp_poi->current_value.position.data[0]),
		&orientation);
	/* now add to these the position of the poi */
	for (i=0;i<3;i++)
	{
		temp_poi->current_value.position.data[i] +=
			temp_poi->current_value.poi.data[i];
	}
	LEAVE;
} /* get_poi_relative_global_position */

static void poi_update_position_coord(Widget coord_widget,void *user_data,
	void *temp_coordinate)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Tells CMGUI about the current values.
==============================================================================*/
{
	struct Poi_struct *temp_poi = user_data;
	struct Cmgui_coordinate *coordinate = temp_coordinate;
	struct Dof3_data new_dof3;

	ENTER(poi_update_position_coord);
	temp_poi->current_position_coordinate = coordinate;
	if (coordinate!=poi_rel_coordinate_ptr)
	{
		temp_poi->relative = 0;
		get_local_position(&(temp_poi->current_value.position),
			coordinate,&new_dof3);
	}
	else
	{
		temp_poi->relative = 1;
		get_poi_relative_local_position(temp_poi,&new_dof3);
	}
	dof3_set_data(temp_poi->position_widget,DOF3_DATA,&new_dof3);
	LEAVE;
} /* poi_update_position_coord */

static void poi_update_poi_coord(Widget coord_widget,void *user_data,
	void *temp_coordinate)
/*******************************************************************************
LAST MODIFIED : 26 September 1995

DESCRIPTION :
Tells CMGUI about the current values.
==============================================================================*/
{
	Gmatrix origin;
	int i,j;
	struct Poi_struct *temp_poi = user_data;
	struct Cmgui_coordinate *coordinate = temp_coordinate;
	struct Dof3_data new_dof3,*temp_pointerest,*temp_up_vector;

	ENTER(poi_update_poi_coord);
	temp_poi->current_poi_coordinate = coordinate;
	get_local_position(&(temp_poi->current_value.poi),
		coordinate,&new_dof3);
	dof3_set_data(temp_poi->poi_widget,DOF3_DATA,&new_dof3);
	/* update the global up vector, and therefore also the relative position */
	temp_up_vector = dof3_get_data(temp_poi->position_widget,DOF3_DATA);
	euler_matrix(&temp_poi->current_poi_coordinate->origin.direction,
		&origin);
	for (i=0;i<3;i++)
	{
		temp_poi->current_value.up_vector.data[i] = 0.0;
		for (j=0;j<3;j++)
		{
			temp_poi->current_value.up_vector.data[i] += temp_up_vector->data[j]*
				origin.data[j][i];
		}
	}
	if (temp_poi->relative)
	{
		/* poll the position dof3 widget */
		temp_pointerest = dof3_get_data(temp_poi->position_widget,DOF3_DATA);
		get_poi_relative_global_position(temp_poi,temp_pointerest);
	}
#if defined (CODE_FRAGMENTS)
	if (temp_poi->relative)
	{
		get_poi_relative_local_position(temp_poi,&new_dof3);
#if defined (OLD_CODE)
		/* we need to base the origin on the position of the poi,
		but base the orientation about up vector of the poi */
		/* these formulae came from view/coord_trans.c routine get_local_position */
		for (i=0;i<3;i++)
		{
			new_dof3.data[i] = temp_poi->current_value.position.data[i]-
				temp_poi->current_value.poi.data[i];
		}
		/* these relative values are in relation to the global axes, so they must be
		changed to reflect the orientation of the local axes */
		euler_matrix(&temp_poi->current_poi_coordinate->origin.direction,&work);
		matrix_premult_vector(&(new_dof3.data[0]),&work);
#endif
		dof3_set_data(temp_poi->position_widget,DOF3_DATA,&new_dof3);
	}
#endif
	poi_update(temp_poi);
	LEAVE;
} /* poi_update_poi_coord */

static void poi_update_position(Widget position_widget,void *user_data,
	void *temp_dof3)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Tells CMGUI about the current values.
==============================================================================*/
{
	struct Poi_struct *temp_poi = user_data;
	struct Dof3_data *temp_position = temp_dof3;

	ENTER(poi_update_position);
	if (temp_poi->relative)
	{
		get_poi_relative_global_position(temp_poi,temp_position);
	}
	else
	{
		get_global_position(temp_position,temp_poi->current_position_coordinate,
			&(temp_poi->current_value.position));
	}
	poi_update(temp_poi);
	LEAVE;
} /* poi_update_position */

static void poi_update_poi(Widget direction_widget,void *user_data,
	void *temp_dof3)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Tells CMGUI about the current values.
==============================================================================*/
{
	struct Poi_struct *temp_poi = user_data;
	struct Dof3_data *temp_pointerest = temp_dof3;

	ENTER(poi_update_poi);
	get_global_position(temp_pointerest,temp_poi->current_poi_coordinate,
		&(temp_poi->current_value.poi));
	if (temp_poi->relative)
	{
		/* poll the position dof3 widget */
		temp_pointerest = dof3_get_data(temp_poi->position_widget,DOF3_DATA);
		get_poi_relative_global_position(temp_poi,temp_pointerest);
	}
	poi_update(temp_poi);
	LEAVE;
} /* poi_update_poi */

static void poi_update_up_vector(Widget direction_widget,void *user_data,
	void *temp_dof3)
/*******************************************************************************
LAST MODIFIED : 26 September 1995

DESCRIPTION :
Tells CMGUI about the current values.
==============================================================================*/
{
	int i,j;
	struct Poi_struct *temp_poi = user_data;
	struct Dof3_data *temp_up_vector = temp_dof3,new_dof3;
	Gmatrix origin;

	ENTER(poi_update_up_vector);
	/* only interested in an up vector if the vector has non-zero magnitude */
	if ((fabs(temp_up_vector->data[0])+fabs(temp_up_vector->data[1])+
		fabs(temp_up_vector->data[2]))>0)
	{
		euler_matrix(&temp_poi->current_poi_coordinate->origin.direction,
			&origin);
		for (i=0;i<3;i++)
		{
			temp_poi->current_value.up_vector.data[i] = 0.0;
			for (j=0;j<3;j++)
			{
				temp_poi->current_value.up_vector.data[i] += temp_up_vector->data[j]*
					origin.data[j][i];
			}
		}
		if (temp_poi->relative)
		{
			get_poi_relative_local_position(temp_poi,&new_dof3);
			dof3_set_data(temp_poi->position_widget,DOF3_DATA,&new_dof3);
		}
		poi_update(temp_poi);
	}
	LEAVE;
} /* poi_update_up_vector */

static void poi_identify_button(Widget w,int button_num,unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Finds the id of the buttons on the poi widget.
==============================================================================*/
{
	struct Poi_struct *temp_poi;

	ENTER(poi_identify_button);
	/* find out which poi widget we are in */
	XtVaGetValues(w,XmNuserData,&temp_poi,NULL);
	switch (button_num)
	{
		case poi_menu_position_ID:
		{
			temp_poi->menu_position = w;
		} break;
		case poi_menu_poi_ID:
		{
			temp_poi->menu_poi = w;
		} break;
		case poi_position_form_ID:
		{
			temp_poi->position_form = w;
		} break;
		case poi_poi_form_ID:
		{
			temp_poi->poi_form = w;
		} break;
		case poi_up_vector_form_ID:
		{
			temp_poi->up_vector_form = w;
		} break;
		case poi_coord_position_ID:
		{
			temp_poi->position_coord_form = w;
		} break;
		case poi_coord_poi_ID:
		{
			temp_poi->poi_coord_form = w;
		} break;
		default:
		{
			display_message(WARNING_MESSAGE,
				"poi_identify_button.  Invalid button number");
		} break;
	}
	LEAVE;
} /* poi_identify_button */

static void poi_destroy_CB(Widget w,int *tag,unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Callback for the poiment dialog - tidies up all details - mem etc
==============================================================================*/
{
	struct Poi_struct *temp_poi;

	ENTER(poi_destroy_CB);
	/* Get the pointer to the data for the poi widget */
	XtVaGetValues(w,XmNuserData,&temp_poi,NULL);
	*(temp_poi->widget_address) = (Widget)NULL;
	/* deallocate the memory for the user data */
	printf("De-allocating memory.\n");
	DEALLOCATE(temp_poi);
	LEAVE;
} /* poi_destroy_CB */

/*
Global Functions
----------------
*/
Widget create_poi_widget(Widget *poi_widget,Widget parent,
	struct Poi_data *init_data)
/*******************************************************************************
LAST MODIFIED : 13 December 1996

DESCRIPTION :
Creates a poi widget that gets a position and orientation from the user.
==============================================================================*/
{
	int i,init_widgets;
	MrmType poi_dialog_class;
	struct Callback_data callback;
	struct Poi_struct *temp_poi = NULL;
	static MrmRegisterArg callback_list[]=
	{
		{"poi_identify_button",(XtPointer)poi_identify_button},
		{"poi_destroy_CB",(XtPointer)poi_destroy_CB}
	};
	static MrmRegisterArg identifier_list[] =
	{
		{"Poi_structure",(XtPointer)NULL},
		{"poi_menu_position_ID",(XtPointer)poi_menu_position_ID},
		{"poi_menu_poi_ID",(XtPointer)poi_menu_poi_ID},
		{"poi_position_form_ID",(XtPointer)poi_position_form_ID},
		{"poi_poi_form_ID",(XtPointer)poi_poi_form_ID},
		{"poi_up_vector_form_ID",(XtPointer)poi_up_vector_form_ID},
		{"poi_coord_position_ID",(XtPointer)poi_coord_position_ID},
		{"poi_coord_poi_ID",(XtPointer)poi_coord_poi_ID},
	};
	Widget return_widget;

	ENTER(create_poi_widget);
	return_widget = (Widget)NULL;
	if (MrmOpenHierarchy_base64_string(poi_uidh,
		&poi_hierarchy,&poi_hierarchy_open))
	{
		/* allocate memory */
		if (ALLOCATE(temp_poi,struct Poi_struct,1))
		{
			/* initialise the structure */
			temp_poi->widget_parent = parent;
			temp_poi->widget = (Widget)NULL;
			temp_poi->widget_address = poi_widget;
			temp_poi->relative = 0;
			temp_poi->menu_position = (Widget)NULL;
			temp_poi->menu_poi = (Widget)NULL;
			temp_poi->position_form = (Widget)NULL;
			temp_poi->poi_form = (Widget)NULL;
			temp_poi->position_coord_form = (Widget)NULL;
			temp_poi->poi_coord_form = (Widget)NULL;
			temp_poi->up_vector_form = (Widget)NULL;
			temp_poi->position_widget = (Widget)NULL;
			temp_poi->poi_widget = (Widget)NULL;
			temp_poi->up_vector_widget = (Widget)NULL;
			temp_poi->positionctrl_widget = (Widget)NULL;
			temp_poi->poictrl_widget = (Widget)NULL;
			temp_poi->input_position_widget = (Widget)NULL;
			temp_poi->input_poi_widget = (Widget)NULL;
			temp_poi->coord_position_widget = (Widget)NULL;
			temp_poi->coord_poi_widget = (Widget)NULL;
			temp_poi->current_position_coordinate = (struct Cmgui_coordinate *)NULL;
			temp_poi->current_poi_coordinate = (struct Cmgui_coordinate *)NULL;
			for (i=0;i<3;i++)
			{
				temp_poi->current_value.position.data[i] = init_data->position.data[i];
				temp_poi->current_value.poi.data[i] = init_data->poi.data[i];
				temp_poi->current_value.up_vector.data[i]=init_data->up_vector.data[i];
			}
			for (i=0;i<POI_NUM_CALLBACKS;i++)
			{
				temp_poi->callback_array[i].procedure = (Callback_procedure *)NULL;
				temp_poi->callback_array[i].data = NULL;
			}
			/* register the callbacks */
			if (MrmSUCCESS==MrmRegisterNamesInHierarchy(poi_hierarchy,callback_list,
				XtNumber(callback_list)))
			{
				/* assign and register the identifiers */
				identifier_list[0].value=(XtPointer)temp_poi;
				if (MrmSUCCESS==MrmRegisterNamesInHierarchy(poi_hierarchy,
					identifier_list,XtNumber(identifier_list)))
				{
					/* fetch position window widget */
					if (MrmSUCCESS==MrmFetchWidget(poi_hierarchy,"poi_widget",
						temp_poi->widget_parent,&(temp_poi->widget),&poi_dialog_class))
					{
						XtManageChild(temp_poi->widget);
						/* set the mode toggle to the correct position */
						init_widgets = 1;
						if (!create_dof3_widget(&(temp_poi->position_widget),
							temp_poi->position_form,DOF3_POSITION,DOF3_ABSOLUTE,
							CONV_RECTANGULAR_CARTESIAN,&temp_poi->current_value.position))
						{
							display_message(ERROR_MESSAGE,
								"create_poi_widget.  Could not create position dof3 widget.");
							init_widgets = 0;
						}
						if (!create_dof3_widget(&(temp_poi->poi_widget),
							temp_poi->poi_form,DOF3_POSITION,DOF3_ABSOLUTE,
							CONV_RECTANGULAR_CARTESIAN,&temp_poi->current_value.poi))
						{
							display_message(ERROR_MESSAGE,
								"create_poi_widget.  Could not create poi dof3 widget.");
							init_widgets = 0;
						}
						if (!(temp_poi->up_vector_widget = create_vector_widget(
							temp_poi->up_vector_form)))
						{
							display_message(ERROR_MESSAGE,
								"create_poi_widget.  Could not create up_vector dof3 widget");
							init_widgets = 0;
						}
#if defined (EXT_INPUT)
						if (!(temp_poi->input_position_widget = create_input_widget(
							temp_poi->menu_position,temp_poi->position_form)))
						{
							display_message(ERROR_MESSAGE,
								"create_poi_widget.  Could not create input position widget");
							init_widgets = 0;
						}
						if (!(temp_poi->input_poi_widget = create_input_widget(
							temp_poi->menu_poi,temp_poi->poi_form)))
						{
							display_message(ERROR_MESSAGE,
								"create_poi_widget.  Could not create input poi widget");
							init_widgets = 0;
						}
#endif
						if (!(temp_poi->positionctrl_widget = create_control_widget(
							temp_poi->menu_position,"Control")))
						{
							display_message(ERROR_MESSAGE,
								"create_poi_widget.  Could not create position control widget");
							init_widgets = 0;
						}
						if (!(temp_poi->poictrl_widget = create_control_widget(
							temp_poi->menu_poi,"Control")))
						{
							display_message(ERROR_MESSAGE,
								"create_poi_widget.  Could not create poi control widget.");
							init_widgets = 0;
						}
						if (!(temp_poi->coord_position_widget = create_coord_widget(
							temp_poi->position_coord_form)))
						{
							display_message(ERROR_MESSAGE,
								"create_poi_widget.  Could not create coord position widget.");
							init_widgets = 0;
						}
						if (!(temp_poi->coord_poi_widget = create_coord_widget(
							temp_poi->poi_coord_form)))
						{
							display_message(ERROR_MESSAGE,
								"create_poi_widget.  Could not create coord poi widget.");
							init_widgets = 0;
						}
						if (init_widgets)
						{
#if defined (EXT_INPUT)
							/* now link all the widgets together */
							input_set_data(temp_poi->input_position_widget,
								INPUT_POSITION_WIDGET,temp_poi->position_widget);
							input_set_data(temp_poi->input_position_widget,
								INPUT_POSCTRL_WIDGET,temp_poi->positionctrl_widget);
#endif
							control_set_data(temp_poi->positionctrl_widget,
								CONTROL_DOF3_WIDGET,temp_poi->position_widget);
#if defined (EXT_INPUT)
							/* now link all the widgets together */
							input_set_data(temp_poi->input_poi_widget,INPUT_POSITION_WIDGET,
								temp_poi->poi_widget);
							input_set_data(temp_poi->input_poi_widget,INPUT_POSCTRL_WIDGET,
								temp_poi->poictrl_widget);
#endif
							control_set_data(temp_poi->poictrl_widget,CONTROL_DOF3_WIDGET,
								temp_poi->poi_widget);
							vector_set_data(temp_poi->up_vector_widget,VECTOR_DATA,
								&temp_poi->current_value.up_vector);
							/* get the global coordinate system */
								/*???GMH.  A bit of a hack at the moment */
							temp_poi->current_position_coordinate = poi_rel_coordinate_ptr;
							coord_add_coordinate(temp_poi->coord_position_widget,
								poi_rel_coordinate_ptr);
							coord_set_data(temp_poi->coord_position_widget,COORD_COORD_DATA,
								poi_rel_coordinate_ptr);
							temp_poi->relative = 1;
							temp_poi->current_poi_coordinate = global_coordinate_ptr;
							coord_set_data(temp_poi->coord_poi_widget,COORD_COORD_DATA,
								global_coordinate_ptr);
							/* set coord callbacks */
							callback.procedure = poi_update_position_coord;
							callback.data = temp_poi;
							coord_set_data(temp_poi->coord_position_widget,COORD_UPDATE_CB,
								&callback);
							callback.procedure = poi_update_poi_coord;
							callback.data = temp_poi;
							coord_set_data(temp_poi->coord_poi_widget,COORD_UPDATE_CB,
								&callback);
							/* set dof3 callbacks */
							callback.procedure = poi_update_position;
							callback.data = temp_poi;
							dof3_set_data(temp_poi->position_widget,DOF3_UPDATE_CB,
								&callback);
							callback.procedure = poi_update_poi;
							callback.data = temp_poi;
							dof3_set_data(temp_poi->poi_widget,DOF3_UPDATE_CB,
								&callback);
							callback.procedure = poi_update_up_vector;
							callback.data = temp_poi;
							vector_set_data(temp_poi->up_vector_widget,VECTOR_UPDATE_CB,
								&callback);
							return_widget = temp_poi->widget;
						}
						else
						{
							DEALLOCATE(temp_poi);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_poi_widget.  Could not fetch poi dialog");
						DEALLOCATE(temp_poi);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_poi_widget.  Could not register identifiers");
					DEALLOCATE(temp_poi);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_poi_widget.  Could not register callbacks");
				DEALLOCATE(temp_poi);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_poi_widget.  Could not allocate poi widget structure");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_poi_widget.  Could not open hierarchy");
	}
	if (poi_widget)
	{
		*poi_widget = return_widget;
	}
	LEAVE;

	return (return_widget);
} /* create_poi_widget */

int poi_set_data(Widget poi_widget,
	enum Poi_data_type data_type,void *data)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Changes a data item of the poi widget.
==============================================================================*/
{
	int i,return_code;
	struct Poi_struct *temp_poi;

	ENTER(poi_set_data);
	/* Get the pointer to the data for the poi dialog */
	XtVaGetValues(poi_widget,XmNuserData,&temp_poi,NULL);
	switch (data_type)
	{
		case POI_UPDATE_CB:
		{
			temp_poi->callback_array[POI_UPDATE_CB].procedure =
				((struct Callback_data *)data)->procedure;
			temp_poi->callback_array[POI_UPDATE_CB].data =
				((struct Callback_data *)data)->data;
			return_code = 1;
		} break;
		case POI_DATA:
		{
			for (i=0;i<3;i++)
			{
				temp_poi->current_value.position.data[i] =
					((struct Poi_data *)data)->position.data[i];
				temp_poi->current_value.poi.data[i] =
					((struct Poi_data *)data)->poi.data[i];
				temp_poi->current_value.up_vector.data[i] =
					((struct Poi_data *)data)->up_vector.data[i];
			}
			dof3_set_data(temp_poi->position_widget,DOF3_DATA,
				&temp_poi->current_value.position);
			dof3_set_data(temp_poi->poi_widget,DOF3_DATA,
				&temp_poi->current_value.poi);
			vector_set_data(temp_poi->up_vector_widget,VECTOR_DATA,
				&temp_poi->current_value.up_vector);
			/* we must change to the global coord system */
			temp_poi->relative = 0;
			temp_poi->current_position_coordinate = global_coordinate_ptr;
			coord_set_data(temp_poi->coord_position_widget,COORD_COORD_DATA,
				temp_poi->current_position_coordinate);
			temp_poi->current_poi_coordinate = global_coordinate_ptr;
			coord_set_data(temp_poi->coord_poi_widget,COORD_COORD_DATA,
				temp_poi->current_poi_coordinate);
			return_code = 1;
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"poi_set_data.  Invalid data type.");
			return_code = 0;
		} break;
	}
	LEAVE;

	return (return_code);
} /* poi_set_data */

void *poi_get_data(Widget poi_widget,
	enum Poi_data_type data_type)
/*******************************************************************************
LAST MODIFIED : 4 November 1995

DESCRIPTION :
Changes a data item of the poi widget.
==============================================================================*/
{
	void *return_code;
	struct Poi_struct *temp_poi;
	static struct Callback_data dat_callback;
	static Widget dat_widget;
	static struct Poi_data dat_data;

	ENTER(poi_get_data);
	/* Get the pointer to the data for the poi dialog */
	XtVaGetValues(poi_widget,XmNuserData,&temp_poi,NULL);
	switch (data_type)
	{
		case POI_UPDATE_CB:
		{
			dat_callback.procedure =
				temp_poi->callback_array[POI_UPDATE_CB].procedure;
			dat_callback.data = temp_poi->callback_array[POI_UPDATE_CB].data;
			return_code = &dat_callback;
		} break;
		case POI_DATA:
		{
			return_code = &temp_poi->current_value;
		} break;
		default:
		{
			display_message(ERROR_MESSAGE,
				"poi_get_data.  Invalid data type.");
			return_code = NULL;
		} break;
	}
	LEAVE;

	return (return_code);
} /* poi_get_data */
