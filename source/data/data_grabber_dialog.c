/*******************************************************************************
FILE : data_grabber_dialog.c

LAST MODIFIED : 1 September 1999

DESCRIPTION :
Brings up a window which holds a data_grabber.  Allows the user to change what
data is accepted - pos,tangent,normal.
==============================================================================*/
#include <stdio.h>
#include <math.h>
#include "choose/choose_node_group.h"
#include "data/data_grabber_dialog.h"
#include "data/data_grabber_dialog.uid64"
#include "data/data_grabber.h"
#include "finite_element/finite_element.h"
#include "general/debug.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

#define BACK_END_CREATE_NODE

/*
Module constants
----------------
*/
#define COORDINATES_3D_FIELD_NAME "coordinates"

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int data_grabber_hierarchy_open=0;
static MrmHierarchy data_grabber_hierarchy;
#endif /* defined (MOTIF) */

/*
Module functions
----------------
*/
static void dg_dialog_update(struct DG_dialog_struct *data_grabber_dialog)
/*******************************************************************************
LAST MODIFIED : 30 August 1999

DESCRIPTION :
Tells CMGUI about the current values. Returns a pointer to the material.
==============================================================================*/
#define PRECISION_STRING "10.5" DATA_GRABBER_PRECISION_STRING
{
	/*???GMH.  These constants should be removed... */
	char command_string[DATA_GRABBER_STRING_SIZE],
		temp_string[DATA_GRABBER_STRING_SIZE];
	char *component_names[3]=
	{
		"x","y","z"
	},
	*fibre_component_name[3]=
	{
		"angle"
	};
	enum FE_nodal_value_type *components_nodal_value_types[3]=
	{
		{
			FE_NODAL_VALUE
		},
		{
			FE_NODAL_VALUE
		},
		{
			FE_NODAL_VALUE
		}
	};
	int components_number_of_derivatives[3]={0,0,0},
		components_number_of_versions[3]={1,1,1};
	DATA_GRABBER_PRECISION fibre_angle,xy_proj;
	FE_value temp_values[9];
	int node_number, temp_index;
	struct DG_data *current_data;
	struct FE_field *coordinate_field, *normal_field, *tangent_field;
	struct FE_node *node,*template_node;
	struct MANAGER(FE_node) *manager;
	struct Coordinate_system rect_cart_coords;
	struct CM_field_information field_info;

	rect_cart_coords.type =  RECTANGULAR_CARTESIAN;

	ENTER(dg_dialog_update);
	if (data_grabber_dialog->current_mode)
	{
		current_data=data_grabber_get_data(
			data_grabber_dialog->grabber_widget,DATA_GRABBER_DATA);

#if defined (BACK_END_CREATE_NODE)
		/* First create the node in the back end */
		sprintf(command_string,"fem define");
		switch (data_grabber_dialog->type_mode)
		{
			case 0:
			{
				sprintf(temp_string," node");
				strcat(command_string,temp_string);
			} break;
			case 1:
			{
				sprintf(temp_string," data");
				strcat(command_string,temp_string);
			} break;
			default:
			{
				display_message(WARNING_MESSAGE,"dg_dialog_update.  Invalid type mode");
			} break;
		}
		if (data_grabber_dialog->current_mode&DATA_GRABBER_POSITION)
		{
			sprintf(temp_string," x=%"PRECISION_STRING" y=%"PRECISION_STRING
				" z=%"PRECISION_STRING,current_data->position.data[0],
				current_data->position.data[1],current_data->position.data[2]);
			strcat(command_string,temp_string);
		}
		if (data_grabber_dialog->current_mode&DATA_GRABBER_TANGENT)
		{
			switch (data_grabber_dialog->tangent_mode)
			{
				case 0:
				{
					sprintf(temp_string," tangent_x=%"PRECISION_STRING" tangent_y=%"
						PRECISION_STRING" tangent_z=%"PRECISION_STRING,
						current_data->tangent.data[0],current_data->tangent.data[1],
						current_data->tangent.data[2]);
					strcat(command_string,temp_string);
				} break;
				case 1:
				{
					/* they want fibre angles wrt the xy plane */
					/* find the magnitude of the projection on the xy plane */
					xy_proj=
						sqrt(current_data->tangent.data[0]*current_data->tangent.data[0]+
						current_data->tangent.data[1]*current_data->tangent.data[1]);
					/* angle is tan-1 (z/proj) */
					fibre_angle=atan2(current_data->tangent.data[2],xy_proj)/PI_180;
					sprintf(temp_string," fibre_angle=%"PRECISION_STRING,fibre_angle);
					strcat(command_string,temp_string);
				} break;
				default:
				{
					display_message(WARNING_MESSAGE,
						"dg_dialog_update.  Invalid tangent mode");
				} break;
			}
		}
		if (data_grabber_dialog->current_mode&DATA_GRABBER_NORMAL)
		{
			sprintf(temp_string," normal_x=%"PRECISION_STRING" normal_y=%"
				PRECISION_STRING" normal_z=%"PRECISION_STRING,
				current_data->normal.data[0],current_data->normal.data[1],
				current_data->normal.data[2]);
			strcat(command_string,temp_string);
		}
		(*(data_grabber_dialog->execute_command->function))(command_string,
			data_grabber_dialog->execute_command->data);
#endif /* defined (BACK_END_CREATE_NODE) */

		/* Create the node in CMGUI */
		switch(data_grabber_dialog->type_mode)
		{
			case 0:
			{
				manager = data_grabber_dialog->node_manager;
			} break;
			case 1:
			{
				manager = data_grabber_dialog->data_manager;
			} break;
			default:
			{
				display_message(WARNING_MESSAGE,
					"dg_dialog_update.  Unknown type");
			} break;
		}
		temp_index = 0;
		temp_values[temp_index++] = current_data->position.data[0];
		temp_values[temp_index++] = current_data->position.data[1];
		temp_values[temp_index++] = current_data->position.data[2];
		node_number=get_next_FE_node_number(manager, 1);

		set_CM_field_information(&field_info,CM_COORDINATE_FIELD,(int *)NULL);
		if (coordinate_field=get_FE_field_manager_matched_field(
			data_grabber_dialog->fe_field_manager,COORDINATES_3D_FIELD_NAME,
			GENERAL_FE_FIELD,/*indexer_field*/(struct FE_field *)NULL,
			/*number_of_indexed_values*/0,&field_info,
			&rect_cart_coords,FE_VALUE_VALUE,
			/*number_of_components*/3,component_names,
			/*number_of_times*/0,/*time_value_type*/UNKNOWN_VALUE))
		{
			if ((template_node=CREATE(FE_node)(0,(struct FE_node *)NULL))&&
				define_FE_field_at_node(template_node,
				coordinate_field,components_number_of_derivatives,
				components_number_of_versions,components_nodal_value_types))
			{
				if (data_grabber_dialog->current_mode&DATA_GRABBER_TANGENT)
				{
					switch (data_grabber_dialog->tangent_mode)
					{
						case 0:
						{
							set_CM_field_information(&field_info,CM_FIELD,(int *)NULL);
							if (tangent_field=get_FE_field_manager_matched_field(
								data_grabber_dialog->fe_field_manager,"tangent",
								GENERAL_FE_FIELD,/*indexer_field*/(struct FE_field *)NULL,
								/*number_of_indexed_values*/0,&field_info,
								&rect_cart_coords,FE_VALUE_VALUE,
								/*number_of_components*/3,component_names,
								/*number_of_times*/0,/*time_value_type*/UNKNOWN_VALUE))
							{
								if (define_FE_field_at_node(template_node,tangent_field,
									components_number_of_derivatives,
									components_number_of_versions,components_nodal_value_types))
								{
									temp_values[temp_index++] = current_data->tangent.data[0];
									temp_values[temp_index++] = current_data->tangent.data[1];
									temp_values[temp_index++] = current_data->tangent.data[2];
								}
								else
								{
									display_message(ERROR_MESSAGE,
									"dg_dialog_update.  Could not define tangent field at node");
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"dg_dialog_update.  Could not create tangent FE_field");
							}		
						} break;
						case 1:
						{
							/* fibre angle calculated above */
							set_CM_field_information(&field_info,CM_ANATOMICAL_FIELD,
								(int *)NULL);
							if (tangent_field=get_FE_field_manager_matched_field(
								data_grabber_dialog->fe_field_manager,"fibre angle",
								GENERAL_FE_FIELD,/*indexer_field*/(struct FE_field *)NULL,
								/*number_of_indexed_values*/0,&field_info,
								&rect_cart_coords,FE_VALUE_VALUE,
								/*number_of_components*/1,fibre_component_name,
								/*number_of_times*/0,/*time_value_type*/UNKNOWN_VALUE))
							{
								if (define_FE_field_at_node(template_node,tangent_field,
									components_number_of_derivatives,
									components_number_of_versions,components_nodal_value_types))
								{
									/* find the magnitude of the projection on the xy plane */
									xy_proj=
										sqrt(current_data->tangent.data[0]*current_data->tangent.data[0]+
											current_data->tangent.data[1]*current_data->tangent.data[1]);
									/* angle is tan-1 (z/proj) */
									fibre_angle=atan2(current_data->tangent.data[2],xy_proj)/PI_180;
									temp_values[temp_index++] = fibre_angle;
								}
								else
								{
									display_message(ERROR_MESSAGE,
									"dg_dialog_update.  Could not define tangent field at node");
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"dg_dialog_update.  Could not create tangent FE_field");
							}		
						} break;
						default:
						{
							display_message(WARNING_MESSAGE,
								"dg_dialog_update.  Invalid tangent mode part 2");
						} break;
					}
				}
				if (data_grabber_dialog->current_mode&DATA_GRABBER_NORMAL)
				{
					set_CM_field_information(&field_info,CM_FIELD,(int *)NULL);
					if (normal_field=get_FE_field_manager_matched_field(
						data_grabber_dialog->fe_field_manager,"normal",
						GENERAL_FE_FIELD,/*indexer_field*/(struct FE_field *)NULL,
						/*number_of_indexed_values*/0,&field_info,
						&rect_cart_coords,FE_VALUE_VALUE,
						/*number_of_components*/3,component_names,
						/*number_of_times*/0,/*time_value_type*/UNKNOWN_VALUE))
					{
						if (define_FE_field_at_node(template_node,normal_field,
							components_number_of_derivatives,
							components_number_of_versions,components_nodal_value_types))
						{
							temp_values[temp_index++] = current_data->normal.data[0];
							temp_values[temp_index++] = current_data->normal.data[1];
							temp_values[temp_index++] = current_data->normal.data[2];
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"dg_dialog_update.  Could not create normal FE_node_field");
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"dg_dialog_update.  Could not create normal FE_field");
					}		
				}
				if (node=CREATE(FE_node)(node_number,template_node))
				{					
					int length;
					set_FE_nodal_field_FE_value_values(coordinate_field,node,temp_values,
						&length);
					temp_index = length;
					if (data_grabber_dialog->current_mode&DATA_GRABBER_TANGENT)
					{
						switch (data_grabber_dialog->tangent_mode)
						{
							case 0:
							{
								set_FE_nodal_field_FE_value_values(tangent_field,node,
									temp_values+temp_index,&length);
								temp_index += length;
							} break;
							case 1:
							{
								set_FE_nodal_field_FE_value_values(tangent_field,node,
									temp_values+temp_index,&length);
								temp_index += length;
							} break;
						}
					}
					if (data_grabber_dialog->current_mode&DATA_GRABBER_NORMAL)
					{
						set_FE_nodal_field_FE_value_values(normal_field,node,
							temp_values+temp_index,&length);
						temp_index += length;
					}

					if (ADD_OBJECT_TO_MANAGER(FE_node)(node, manager))
					{
						if (data_grabber_dialog->node_group)
						{
							ADD_OBJECT_TO_GROUP(FE_node)(node,
								data_grabber_dialog->node_group);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"dg_dialog_update.  Could not add FE_node to manager");
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"dg_dialog_update.  Could not create FE_node");
				}
				DESTROY(FE_node)(&template_node);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"dg_dialog_update.  Could not create template_node");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"dg_dialog_update.  Could not create FE_field");
		}		
	}
	LEAVE;
} /* dg_dialog_update */

static void dg_dialog_dg_update(Widget data_grabber_widget,void *user_data,
	void *new_data_grabber_data)
/*******************************************************************************
LAST MODIFIED : 25 September 1995

DESCRIPTION :
The button has been pressed, so get the data and output it.
==============================================================================*/
{
	struct DG_dialog_struct *data_grabber_dialog=user_data;

	ENTER(dg_dialog_dg_update);
	USE_PARAMETER(new_data_grabber_data);
	USE_PARAMETER(data_grabber_widget);
	dg_dialog_update(data_grabber_dialog);
	LEAVE;
} /* dg_dialog_dg_update */

static void dg_dialog_identify_button(Widget w,int button_num,
	unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 18 April 1994

DESCRIPTION :
Finds the id of the buttons on the data_grabber_dialog widget.
==============================================================================*/
{
	struct DG_dialog_struct *data_grabber_dialog;

	ENTER(dg_dialog_identify_button);
	USE_PARAMETER(reason);
	/* find out which data_grabber_dialog widget we are in */
	XtVaGetValues(w,XmNuserData,&data_grabber_dialog,NULL);
	switch (button_num)
	{
		case data_grabber_dialog_grabber_form_ID:
		{
			data_grabber_dialog->grabber_form=w;
		} break;
		case data_grabber_dialog_toggle_form_ID:
		{
			data_grabber_dialog->toggle_form=w;
		} break;
		case data_grabber_dialog_mode_form_ID:
		{
			data_grabber_dialog->mode_form=w;
		} break;
		case data_grabber_dialog_tangent_form_ID:
		{
			data_grabber_dialog->tangent_form=w;
		} break;
		case data_grabber_dialog_record_ID:
		{
			data_grabber_dialog->record_button=w;
		} break;
		case data_grabber_dialog_ok_ID:
		{
			data_grabber_dialog->ok_button=w;
		} break;
		case data_grabber_dialog_type_form_ID:
		{
			data_grabber_dialog->type_form = w;
		} break;
		case data_grabber_dialog_node_group_form_ID:
		{
			data_grabber_dialog->node_group_form = w;
		} break;
		default:
		{
			display_message(WARNING_MESSAGE,
				"dg_dialog_identify_button.  Invalid button number");
		} break;
	}
	LEAVE;
} /* dg_dialog_identify_button */

static void dg_dialog_control_CB(Widget w,int button_num,unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 25 September 1994

DESCRIPTION :
The mode has been changed etc.
==============================================================================*/
{
	struct DG_dialog_struct *data_grabber_dialog;

	ENTER(dg_dialog_control_CB);
	USE_PARAMETER(reason);
	/* find out which data_grabber_dialog widget we are in */
	XtVaGetValues(w,XmNuserData,&data_grabber_dialog,NULL);
	switch (button_num)
	{
		case data_grabber_dialog_record_ID:
		{
			dg_dialog_update(data_grabber_dialog);
		} break;
		case data_grabber_dialog_ok_ID:
		{
			XtDestroyWidget(data_grabber_dialog->dialog);
		} break;
		default:
		{
			display_message(WARNING_MESSAGE,
				"dg_dialog_control_CB.  Invalid button number");
		} break;
	}
	LEAVE;
} /* dg_dialog_control_CB */

static void dg_dialog_mode_CB(Widget w,int *tag,
	XmToggleButtonCallbackStruct *reason)
/*******************************************************************************
LAST MODIFIED : 25 September 1994

DESCRIPTION :
The mode has been changed etc.
==============================================================================*/
{
	int button_num=*tag;
	struct DG_dialog_struct *data_grabber_dialog;

	ENTER(dg_dialog_mode_CB);
	/* find out which data_grabber_dialog widget we are in */
	XtVaGetValues(w,XmNuserData,&data_grabber_dialog,NULL);
	switch (button_num)
	{
		case 0:
		{
			if (reason->set)
			{
				data_grabber_dialog->current_mode=
					data_grabber_dialog->current_mode|DATA_GRABBER_POSITION;
			}
			else
			{
				data_grabber_dialog->current_mode=
					data_grabber_dialog->current_mode&(~DATA_GRABBER_POSITION);
			}
			data_grabber_set_data(data_grabber_dialog->grabber_widget,
				DATA_GRABBER_MODE,&data_grabber_dialog->current_mode);
		} break;
		case 1:
		{
			if (reason->set)
			{
				data_grabber_dialog->current_mode=
					data_grabber_dialog->current_mode|DATA_GRABBER_TANGENT;
				XtManageChild(data_grabber_dialog->tangent_form);
			}
			else
			{
				data_grabber_dialog->current_mode=
					data_grabber_dialog->current_mode&(~DATA_GRABBER_TANGENT);
				XtUnmanageChild(data_grabber_dialog->tangent_form);
			}
			data_grabber_set_data(data_grabber_dialog->grabber_widget,
				DATA_GRABBER_MODE,&data_grabber_dialog->current_mode);
		} break;
		case 2:
		{
			if (reason->set)
			{
				data_grabber_dialog->current_mode=
					data_grabber_dialog->current_mode|DATA_GRABBER_NORMAL;
			}
			else
			{
				data_grabber_dialog->current_mode=
					data_grabber_dialog->current_mode&(~DATA_GRABBER_NORMAL);
			}
			data_grabber_set_data(data_grabber_dialog->grabber_widget,
				DATA_GRABBER_MODE,&data_grabber_dialog->current_mode);
		} break;
		default:
		{
			display_message(WARNING_MESSAGE,
				"dg_dialog_mode_CB.  Invalid button number");
		} break;
	}
	LEAVE;
} /* dg_dialog_mode_CB */

static void data_grabber_dialog_set_node_group(
	Widget widget, void *data_grabber_dialog_void, void *node_group_void)
/*******************************************************************************
LAST MODIFIED : 27 October 1998

DESCRIPTION :
Called when choosen node_group is changed.
==============================================================================*/
{
	struct DG_dialog_struct *data_grabber_dialog;

	ENTER(data_grabber_dialog_set_node_group);
	if (data_grabber_dialog = (struct DG_dialog_struct *)data_grabber_dialog_void)
	{
		if((widget == data_grabber_dialog->node_group_chooser) &&
			(data_grabber_dialog->type_mode == 0))
		{
			data_grabber_dialog->node_group = (struct GROUP(FE_node) *)node_group_void;
		}
		if((widget == data_grabber_dialog->data_group_chooser) &&
			(data_grabber_dialog->type_mode == 1))
		{
			data_grabber_dialog->node_group = (struct GROUP(FE_node) *)node_group_void;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"data_grabber_dialog_set_node_group.  Invalid argument(s)");
	}
	LEAVE;
} /* data_grabber_dialog_set_node_group */

static void dg_dialog_tangent_CB(Widget w,int *tag,
	XmToggleButtonCallbackStruct *reason)
/*******************************************************************************
LAST MODIFIED : 25 September 1994

DESCRIPTION :
The mode has been changed etc.
==============================================================================*/
{
	int button_num=*tag;
	struct DG_dialog_struct *data_grabber_dialog;

	ENTER(dg_dialog_tangent_CB);
	/* find out which data_grabber_dialog widget we are in */
	XtVaGetValues(w,XmNuserData,&data_grabber_dialog,NULL);
	if (reason->set)
	{
		data_grabber_dialog->tangent_mode=button_num;
	}
	LEAVE;
} /* dg_dialog_tangent_CB */

static void dg_dialog_type_CB(Widget w,int *tag,
	XmToggleButtonCallbackStruct *reason)
/*******************************************************************************
LAST MODIFIED : 25 September 1994

DESCRIPTION :
The mode has been changed etc.
==============================================================================*/
{
	int button_num= *tag;
	struct DG_dialog_struct *data_grabber_dialog;

	ENTER(dg_dialog_type_CB);
	/* find out which data_grabber_dialog widget we are in */
	XtVaGetValues(w,XmNuserData,&data_grabber_dialog,NULL);
	if (reason->set)
	{
		data_grabber_dialog->type_mode=button_num;
		switch(button_num)
		{
			case 0:
			{
				XtManageChild(data_grabber_dialog->node_group_chooser);
				XtUnmanageChild(data_grabber_dialog->data_group_chooser);
				data_grabber_dialog->node_group =
					CHOOSE_OBJECT_GET_OBJECT(GROUP(FE_node))
					(data_grabber_dialog->node_group_chooser);
			} break;
			case 1:
			{
				XtManageChild(data_grabber_dialog->data_group_chooser);
				XtUnmanageChild(data_grabber_dialog->node_group_chooser);
				data_grabber_dialog->node_group =
					CHOOSE_OBJECT_GET_OBJECT(GROUP(FE_node))
					(data_grabber_dialog->data_group_chooser);
			} break;
			default:
			{
				display_message(WARNING_MESSAGE,
					"dg_dialog_type_CB.  Unknown type");
			} break;
		}
	}
	LEAVE;
} /* dg_dialog_type_CB */

static void dg_dialog_add_toggles(
	struct DG_dialog_struct *data_grabber_dialog)
/*******************************************************************************
LAST MODIFIED : 29 January 1996

DESCRIPTION :
Adds any toggles to the top portion of the dialog.
==============================================================================*/
{
	MrmType data_grabber_dialog_toggle_class;
	Widget widget;

	ENTER(dg_dialog_add_toggles);
	if (data_grabber_hierarchy_open)
	{
		if (MrmSUCCESS==MrmFetchWidget(data_grabber_hierarchy,"data_grabber_d_mode",
			data_grabber_dialog->toggle_form,&widget,
			&data_grabber_dialog_toggle_class))
		{
			XtManageChild(widget);
		}
		else
		{
			display_message(WARNING_MESSAGE,
				"dg_dialog_add_toggles.  Could not fetch mode widget");
		}
		if (MrmSUCCESS==MrmFetchWidget(data_grabber_hierarchy,"data_grabber_d_type",
			data_grabber_dialog->toggle_form,&widget,
			&data_grabber_dialog_toggle_class))
		{
			XtManageChild(widget);
		}
		else
		{
			display_message(WARNING_MESSAGE,
				"dg_dialog_add_toggles.  Could not fetch type widget");
		}
		if (MrmSUCCESS==MrmFetchWidget(data_grabber_hierarchy,
			"data_grabber_d_tangent",data_grabber_dialog->toggle_form,
			&widget,&data_grabber_dialog_toggle_class))
		{
			XtManageChild(widget);
		}
		else
		{
			display_message(WARNING_MESSAGE,
				"dg_dialog_add_toggles.  Could not fetch tangent widget");
		}
		if (!(data_grabber_dialog->current_mode&DATA_GRABBER_TANGENT))
		{
			XtUnmanageChild(data_grabber_dialog->tangent_form);
		}
	}
	else
	{
		display_message(WARNING_MESSAGE,
			"dg_dialog_add_toggles.  Data grabber hierarchy is not open");
	}
	LEAVE;
} /* dg_dialog_add_toggles */

static void dg_dialog_destroy_CB(Widget w,int *tag,unsigned long *reason)
/*******************************************************************************
LAST MODIFIED : 2 November 1995

DESCRIPTION :
Callback for the data_grabber_dialog dialog - tidies up all details - mem etc
==============================================================================*/
{
	struct DG_dialog_struct *data_grabber_dialog;

	ENTER(dg_dialog_destroy_CB);
	USE_PARAMETER(tag);
	USE_PARAMETER(reason);
	/* Get the pointer to the data for the data_grabber_dialog widget */
	XtVaGetValues(w,XmNuserData,&data_grabber_dialog,NULL);
	/* deaccess the data_grabber_dialog */
	*(data_grabber_dialog->dialog_address)=(Widget)NULL;
	/* deallocate the memory for the user data */
	DEALLOCATE(data_grabber_dialog);
	LEAVE;
} /* dg_dialog_destroy_CB */

static Widget create_data_grabber_dialog(Widget *data_grabber_dialog_widget,
	Widget parent,struct Execute_command *execute_command,
	struct User_interface *user_interface,
	struct MANAGER(FE_field) *fe_field_manager,
	struct MANAGER(FE_node) *node_manager,
	struct MANAGER(FE_node) *data_manager, 
	struct MANAGER(GROUP(FE_node)) *node_group_manager,
	struct MANAGER(GROUP(FE_node)) *data_group_manager)
/*******************************************************************************
LAST MODIFIED : 29 December 1999

DESCRIPTION :
Brings up a data_grabber that allows the user to get three dimensional data
points.
==============================================================================*/
{
	int init_widgets;
	MrmType data_grabber_dialog_dialog_class;
	struct Callback_data callback;
	struct DG_dialog_struct *data_grabber_dialog=NULL;
	static MrmRegisterArg callback_list[]=
	{
		{"data_grabber_d_identify_button",(XtPointer)dg_dialog_identify_button},
		{"data_grabber_d_destroy_CB",(XtPointer)dg_dialog_destroy_CB},
		{"data_grabber_d_control_CB",(XtPointer)dg_dialog_control_CB},
		{"data_grabber_d_mode_CB",(XtPointer)dg_dialog_mode_CB},
		{"data_grabber_d_tangent_CB",(XtPointer)dg_dialog_tangent_CB},
		{"data_grabber_d_type_CB",(XtPointer)dg_dialog_type_CB},
	};
	static MrmRegisterArg identifier_list[]=
	{
		{"Data_grabber_d_structure",(XtPointer)NULL},
		{"data_grabber_d_grabber_form_ID",
		(XtPointer)data_grabber_dialog_grabber_form_ID},
		{"data_grabber_d_record_ID",
		(XtPointer)data_grabber_dialog_record_ID},
		{"data_grabber_d_ok_ID",
		(XtPointer)data_grabber_dialog_ok_ID},
		{"data_grabber_d_toggle_form_ID",
		(XtPointer)data_grabber_dialog_toggle_form_ID},
		{"data_grabber_d_mode_form_ID",
		(XtPointer)data_grabber_dialog_mode_form_ID},
		{"data_grabber_d_tangent_form_ID",
		(XtPointer)data_grabber_dialog_tangent_form_ID},
		{"data_grabber_d_type_form_ID",
		(XtPointer)data_grabber_dialog_type_form_ID},
		{"data_grabber_d_node_g_form_ID",
		(XtPointer)data_grabber_dialog_node_group_form_ID},
	};
	Widget return_widget;

	ENTER(create_data_grabber_dialog);
	return_widget=(Widget)NULL;
	/* check arguments */
	if (data_grabber_dialog_widget&&parent&&execute_command&&
		(execute_command->function))
	{
		if (MrmOpenHierarchy_base64_string(data_grabber_dialog_uid64,
			&data_grabber_hierarchy,&data_grabber_hierarchy_open))
		{
			/* allocate memory */
			if (ALLOCATE(data_grabber_dialog,struct DG_dialog_struct,1))
			{
				/* initialise the structure */
				data_grabber_dialog->dialog_parent=parent;
				data_grabber_dialog->dialog_address=data_grabber_dialog_widget;
				data_grabber_dialog->execute_command=execute_command;
				data_grabber_dialog->grabber_form=(Widget)NULL;
				data_grabber_dialog->toggle_form=(Widget)NULL;
				data_grabber_dialog->mode_form=(Widget)NULL;
				data_grabber_dialog->node_group_form=(Widget)NULL;
				data_grabber_dialog->tangent_form=(Widget)NULL;
				data_grabber_dialog->type_form=(Widget)NULL;
				data_grabber_dialog->ok_button=(Widget)NULL;
				data_grabber_dialog->record_button=(Widget)NULL;
				data_grabber_dialog->current_mode=DATA_GRABBER_POSITION;
				data_grabber_dialog->tangent_mode=0;
				data_grabber_dialog->type_mode=0;
				data_grabber_dialog->fe_field_manager = fe_field_manager;
				data_grabber_dialog->node_manager = node_manager;
				data_grabber_dialog->data_manager = data_manager;
				data_grabber_dialog->node_group_manager = node_group_manager;
				data_grabber_dialog->data_group_manager = data_group_manager;
				/* make the dialog shell */
				if (data_grabber_dialog->dialog=XtVaCreatePopupShell(
					"Data Grabber",topLevelShellWidgetClass,parent,XmNallowShellResize,
					TRUE,NULL))
				{
					/* register the callbacks */
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(data_grabber_hierarchy,
						callback_list,XtNumber(callback_list)))
					{
						/* assign and register the identifiers */
						identifier_list[0].value=(XtPointer)data_grabber_dialog;
						if (MrmSUCCESS==MrmRegisterNamesInHierarchy(data_grabber_hierarchy,
							identifier_list,XtNumber(identifier_list)))
						{
							/* fetch position window widget */
							if (MrmSUCCESS==MrmFetchWidget(data_grabber_hierarchy,
								"data_grabber_dialog_widget",data_grabber_dialog->dialog,
								&(data_grabber_dialog->widget),
								&data_grabber_dialog_dialog_class))
							{
								XtManageChild(data_grabber_dialog->widget);
								/* set the mode toggle to the correct position */
								init_widgets=1;
								if (!(create_data_grabber_widget(
									&data_grabber_dialog->grabber_widget,
									data_grabber_dialog->grabber_form,
									data_grabber_dialog->current_mode)))
								{
									display_message(ERROR_MESSAGE,
								"create_data_grabber_dialog.  Could not create grabber widget");
									init_widgets=0;
								}
								if (init_widgets &&
									(data_grabber_dialog->node_group_chooser =
									CREATE_CHOOSE_OBJECT_WIDGET(GROUP(FE_node))(
										data_grabber_dialog->node_group_form,
										(struct GROUP(FE_node) *)NULL, node_group_manager,
										(MANAGER_CONDITIONAL_FUNCTION(GROUP(FE_node)) *)NULL)))
								{
									data_grabber_dialog->node_group =
										CHOOSE_OBJECT_GET_OBJECT(GROUP(FE_node))
										(data_grabber_dialog->node_group_chooser);
								}
								else
								{
									init_widgets = 0;
								}
								if (init_widgets &&
									!(data_grabber_dialog->data_group_chooser =
									CREATE_CHOOSE_OBJECT_WIDGET(GROUP(FE_node))(
										data_grabber_dialog->node_group_form,
										(struct GROUP(FE_node) *)NULL, data_group_manager,
										(MANAGER_CONDITIONAL_FUNCTION(GROUP(FE_node)) *)NULL)))
								{
									init_widgets = 0;
								}
								if (init_widgets)
								{
									callback.data=(void *)data_grabber_dialog;
									callback.procedure=
										data_grabber_dialog_set_node_group;
									CHOOSE_OBJECT_SET_CALLBACK(GROUP(FE_node))(
										data_grabber_dialog->node_group_chooser, &callback);
									callback.data=(void *)data_grabber_dialog;
									callback.procedure=
										data_grabber_dialog_set_node_group;
									CHOOSE_OBJECT_SET_CALLBACK(GROUP(FE_node))(
										data_grabber_dialog->data_group_chooser, &callback);
									XtUnmanageChild(data_grabber_dialog->data_group_chooser);
									callback.procedure=dg_dialog_dg_update;
									callback.data=data_grabber_dialog;
									data_grabber_set_data(
										data_grabber_dialog->grabber_widget,
										DATA_GRABBER_SELECT_CB,&callback);
									dg_dialog_add_toggles(data_grabber_dialog);
									XtRealizeWidget(data_grabber_dialog->dialog);
									XtPopup(data_grabber_dialog->dialog,XtGrabNone);
#if defined (EXT_INPUT)
									input_module_add_input_window(
										data_grabber_dialog->dialog,user_interface);
#endif
									return_widget=data_grabber_dialog->dialog;
								}
								else
								{
									DEALLOCATE(data_grabber_dialog);
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
		"create_data_grabber_dialog.  Could not fetch data_grabber_dialog dialog");
								DEALLOCATE(data_grabber_dialog);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"create_data_grabber_dialog.  Could not register identifiers");
							DEALLOCATE(data_grabber_dialog);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_data_grabber_dialog.  Could not register callbacks");
						DEALLOCATE(data_grabber_dialog);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_data_grabber_dialog.  Could not create popup shell.");
					DEALLOCATE(data_grabber_dialog);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
"create_data_grabber_dialog.  Could not allocate data_grabber_dialog structure"
					);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_data_grabber_dialog.  Could not open hierarchy");
		}
		*data_grabber_dialog_widget=return_widget;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_data_grabber_dialog.  Invalid argument(s)");
		return_widget=(Widget)NULL;
	}
	LEAVE;

	return (return_widget);
} /* create_data_grabber_dialog */

/*
Global Functions
----------------
*/
int bring_up_data_grabber_dialog(Widget *data_grabber_dialog_address,
	Widget parent,struct Execute_command *execute_command,
	struct User_interface *user_interface,
	struct MANAGER(FE_field) *fe_field_manager,
	struct MANAGER(FE_node) *node_manager,
	struct MANAGER(FE_node) *data_manager, 
	struct MANAGER(GROUP(FE_node)) *node_group_manager,
	struct MANAGER(GROUP(FE_node)) *data_group_manager)
/*******************************************************************************
LAST MODIFIED : 29 January 1999

DESCRIPTION :
If there is a data_grabber dialog in existence, then bring it to the front,
else create a new one.
==============================================================================*/
{
	int return_code;

	ENTER(bring_up_data_grabber_dialog);
	if (data_grabber_dialog_address)
	{
		if (*data_grabber_dialog_address)
		{
			XtPopup(*data_grabber_dialog_address,XtGrabNone);
			return_code=1;
		}
		else
		{
			if (create_data_grabber_dialog(data_grabber_dialog_address,parent,
				execute_command,user_interface, fe_field_manager, node_manager,
				data_manager, node_group_manager, data_group_manager))
			{
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"bring_up_data_grabber_dialog.  Error creating dialog");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"bring_up_data_grabber_dialog.  Invalid argument");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* bring_up_data_grabber_dialog */

#if defined (OLD_CODE)
int data_grabber_dialog_set_data(Widget data_grabber_dialog_widget,
	enum data_grabber_dialog_data_type data_type,void *data)
/*******************************************************************************
LAST MODIFIED : 2 November 1995

DESCRIPTION :
Changes a data item of the data_grabber_dialog widget.
==============================================================================*/
{
	int num_children,return_code;
	struct DG_dialog_struct *data_grabber_dialog;
	Widget *child_list;

	ENTER(data_grabber_dialog_set_data);
	/* Get the pointer to the data for the data_grabber_dialog dialog */
	XtVaGetValues(data_grabber_dialog_widget,XmNchildren,&child_list,
		XmNnumChildren,&num_children,NULL);
	if (1==num_children)
	{
		XtVaGetValues(child_list[0],XmNuserData,&data_grabber_dialog,
			NULL);
		switch (data_type)
		{
			case DATA_GRABBER_DIALOG_MODE:
			{
				data_grabber_dialog->current_mode=*((int *)data);
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"data_grabber_dialog_set_data.  Invalid data type.");
				return_code=0;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"data_grabber_dialog_set_data.  Invalid dialog.");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* data_grabber_dialog_set_data */

void *data_grabber_dialog_get_data(Widget data_grabber_dialog_widget,
	enum data_grabber_dialog_data_type data_type)
/*******************************************************************************
LAST MODIFIED : 2 November 1995

DESCRIPTION :
Changes a data item of the data_grabber_dialog widget.
==============================================================================*/
{
	int num_children;
	void *return_code;
	struct DG_dialog_struct *data_grabber_dialog;
	static int dat_mode;
	Widget *child_list;

	ENTER(data_grabber_dialog_register_callback);
	/* Get the pointer to the data for the data_grabber_dialog dialog */
	XtVaGetValues(data_grabber_dialog_widget,
		XmNchildren,&child_list,
		XmNnumChildren,&num_children,
		NULL);
	if (1==num_children)
	{
		XtVaGetValues(child_list[0],XmNuserData,&data_grabber_dialog,
			NULL);
		switch (data_type)
		{
			case DATA_GRABBER_DIALOG_MODE:
			{
				dat_mode=data_grabber_dialog->current_mode;
				return_code= &dat_mode;
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"data_grabber_dialog_get_data.  Invalid data type.");
				return_code=NULL;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"data_grabber_dialog_get_data.  Invalid dialog.");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* data_grabber_dialog_get_data */
#endif /* defined (OLD_CODE) */
