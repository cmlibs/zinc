/*******************************************************************************
FILE : node_group_slider_dialog.c

LAST MODIFIED : 18 November 1999

DESCRIPTION :
This module creates a node_group_slider input device.  A node group slider is
used to scale the distance between a fixed node and a group of nodes.
???DB.  To be extended
==============================================================================*/
#include <math.h>
#include <stdio.h>
#include <Xm/DialogS.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/ScrollBar.h>
#include <Xm/Separator.h>
#include "command/command.h"
	/*???DB.  For Execute_command */
#include "command/parser.h"
#include "general/debug.h"
#include "general/geometry.h"
#include "mirage/em_cmgui.h"
	/*???DB.  For EM analysis - move out of mirage ? */
#include "slider/node_group_slider_dialog.h"
#include "user_interface/message.h"
#include "curve/control_curve.h"

/*
Module constants
----------------
*/
#define SLIDER_RESOLUTION 1000

/*
Module types
------------
*/
struct Pivot_line
/*******************************************************************************
LAST MODIFIED : 2 June 1997

DESCRIPTION :
Two points on the pivot line.
==============================================================================*/
{
	float x1,x2,y1,y2,z1,z2;
}; /* struct Pivot_line */

enum Node_group_slider_type
/*******************************************************************************
LAST MODIFIED : 24 February 1998

DESCRIPTION :
==============================================================================*/
{
	MUSCLE_SLIDER,
	PIVOT_SLIDER,
	EM_SLIDER
}; /* enum Node_group_slider_type */

struct Shared_em_slider_data
/*******************************************************************************
LAST MODIFIED : 27 February 1998

DESCRIPTION :
==============================================================================*/
{
	double *weights;
	int count,number_of_sliders;
	struct EM_Object *em_object;
	struct Control_curve *em_variable;
}; /* struct Shared_em_slider_data */

struct Node_group_slider_type_data
/*******************************************************************************
LAST MODIFIED : 24 February 1998

DESCRIPTION :
Specifies the slider type and type specific data.
==============================================================================*/
{
	enum Node_group_slider_type type;
	union
	{
		struct
		{
			struct FE_field_component coefficient;
			struct FE_node *fixed_node;
		} muscle;
		struct Pivot_line pivot;
		struct
		{
			int index;
			struct Shared_em_slider_data *shared;
		} em;
	} data;
}; /* struct Node_group_slider_type_data */

struct Node_group_slider
/*******************************************************************************
LAST MODIFIED : 2 June 1997

DESCRIPTION :
==============================================================================*/
{
	char *name;
	struct Node_group_slider_type_data type_data;
	float initial_value,maximum,minimum,value;
	struct Execute_command *execute_command;
	struct GROUP(FE_node) *node_group;
	struct MANAGER(FE_node) *node_manager;
	Widget label,slider;
}; /* struct Node_group_slider */

struct Node_group_slider_dialog
/*******************************************************************************
LAST MODIFIED : 31 May 1997

DESCRIPTION :
==============================================================================*/
{
	int number_of_sliders;
	struct Execute_command *execute_command;
	struct User_interface *user_interface;
	Widget outer_form,reset_button,separator,*sliders;
	Widget variable_slider_form, variable_label, variable_slider;
}; /* struct Node_group_slider_dialog */

struct Update_node_data
/*******************************************************************************
LAST MODIFIED : 2 June 1997

DESCRIPTION :
==============================================================================*/
{
	float new_slider_value,old_slider_value;
	struct MANAGER(FE_node) *node_manager;
	struct Node_group_slider_type_data *type_data;
}; /* struct Update_node_data */

struct Update_em_slider_data
/*******************************************************************************
LAST MODIFIED : 2 June 1997

DESCRIPTION :
==============================================================================*/
{
	struct Node_group_slider_dialog *node_group_slider_dialog;
	struct Shared_em_slider_data *shared_data;
}; /* struct Update_em_slider_data */

/*
Module functions
----------------
*/
#if defined (OLD_CODE)
static int set_slider_type_muscle(struct Parse_state *state,
	void *slider_type_void,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 31 May 1997

DESCRIPTION :
Sets the slider type to muscle.
==============================================================================*/
{
	enum Node_group_slider_type *slider_type;
	int return_code;

	ENTER(set_slider_type_muscle);
	if (state)
	{
		if (slider_type=(enum Node_group_slider_type *)slider_type_void)
		{
			*slider_type=MUSCLE_SLIDER;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_slider_type_muscle.  Missing slider_type");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_slider_type_muscle.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_slider_type_muscle */
#endif /* defined (OLD_CODE) */

#if defined (OLD_CODE)
static int set_slider_type_pivot(struct Parse_state *state,
	void *slider_type_void,void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 31 May 1997

DESCRIPTION :
Sets the slider type to pivot.
==============================================================================*/
{
	enum Node_group_slider_type *slider_type;
	int return_code;

	ENTER(set_slider_type_pivot);
	if (state)
	{
		if (slider_type=(enum Node_group_slider_type *)slider_type_void)
		{
			*slider_type=PIVOT_SLIDER;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_slider_type_pivot.  Missing slider_type");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_slider_type_pivot.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_slider_type_pivot */
#endif /* defined (OLD_CODE) */

static int set_pivot_line(struct Parse_state *state,void *pivot_line_void,
	void *dummy_user_data)
/*******************************************************************************
LAST MODIFIED : 2 June 1997

DESCRIPTION :
Sets the points for a pivot.
==============================================================================*/
{
	char *current_token;
	float x1,x2,y1,y2,z1,z2;
	int return_code;
	struct Pivot_line *pivot_line;

	ENTER(set_pivot_line);
	USE_PARAMETER(dummy_user_data);
	if (state)
	{
		if (current_token=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,current_token)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))
			{
				if (pivot_line=(struct Pivot_line *)pivot_line_void)
				{
					if ((1==sscanf(current_token," %f ",&x1))&&
						shift_Parse_state(state,1)&&
						(1==sscanf(state->current_token," %f ",&y1))&&
						shift_Parse_state(state,1)&&
						(1==sscanf(state->current_token," %f ",&z1))&&
						shift_Parse_state(state,1)&&
						(1==sscanf(state->current_token," %f ",&x2))&&
						shift_Parse_state(state,1)&&
						(1==sscanf(state->current_token," %f ",&y2))&&
						shift_Parse_state(state,1)&&
						(1==sscanf(state->current_token," %f ",&z2)))
					{
						if ((x1==x2)&&(y1==y2)&&(z1==z2))
						{
							display_message(ERROR_MESSAGE,"Pivot line points must differ");
							return_code=0;
						}
						else
						{
							pivot_line->x1=x1;
							pivot_line->y1=y1;
							pivot_line->z1=z1;
							pivot_line->x2=x2;
							pivot_line->y2=y2;
							pivot_line->z2=z2;
							return_code=shift_Parse_state(state,1);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Missing coordinate(s) for pivot line");
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"set_pivot_line.  Missing pivot_line");
					return_code=0;
				}
			}
			else
			{
				if (pivot_line=(struct Pivot_line *)pivot_line_void)
				{
					display_message(INFORMATION_MESSAGE,
						" X1#[%g] Y1#[%g] Z1#[%g] X2#[%g] Y2#[%g] Z2#[%g]",pivot_line->x1,
						pivot_line->y1,pivot_line->z1,pivot_line->x2,pivot_line->y2,
						pivot_line->z2);
				}
				else
				{
					display_message(INFORMATION_MESSAGE," X1# Y1# Z1# X2# Y2# Z2#");
				}
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing pivot line");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_pivot_line.  Missing state");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_pivot_line */

static int update_node(struct FE_node *node,void *update_node_data_void)
/*******************************************************************************
LAST MODIFIED : 3 November 1998

DESCRIPTION :
Updates the <node> location
==============================================================================*/
{
	FE_value coefficient,fixed_x,fixed_y,fixed_z,node_x,node_y,node_z,
		r1,r2;
	float axis1_x,axis2_x,axis3_x,axis1_y,axis2_y,axis3_y,axis1_z,axis2_z,axis3_z,
		cosine,length,sine,temp;
	int return_code;
	struct FE_field *coordinate_field;
	struct FE_node *temp_node;
	struct Update_node_data *update_node_data;

	ENTER(update_node);
	if (node&&(update_node_data=(struct Update_node_data *)update_node_data_void))
	{
		if ((temp_node=CREATE(FE_node)(0,(struct FE_node *)NULL))&&
			COPY(FE_node)(temp_node,node))
		{
			switch (update_node_data->type_data->type)
			{
				case MUSCLE_SLIDER:
				{
					if (coordinate_field=FE_node_get_position_cartesian(
						(update_node_data->type_data->data).muscle.fixed_node,
						(struct FE_field *)NULL,&fixed_x,&fixed_y,&fixed_z,
						(FE_value *)NULL))
					{
						if (FE_node_get_position_cartesian(temp_node,coordinate_field,
							&node_x,&node_y,&node_z,(FE_value *)NULL))
						{
							coefficient=1;
							if ((update_node_data->type_data->data).muscle.coefficient.field)
							{
								get_FE_nodal_FE_value_value(temp_node,
									&((update_node_data->type_data->data).muscle.coefficient),0,
									FE_NODAL_VALUE,&coefficient);
							}
							r1=coefficient*((update_node_data->old_slider_value)-1);
							r2=coefficient*((update_node_data->new_slider_value)-1);
							/*???DB.  Restrict ranges, otherwise get "trapped" */
							if (r1< -0.999)
							{
								r1= -0.999;
							}
							else
							{
								if (r1>0.999)
								{
									r1=0.999;
								}
							}
							if (r2< -0.999)
							{
								r2= -0.999;
							}
							else
							{
								if (r2>0.999)
								{
									r2=0.999;
								}
							}
							node_x=(node_x+r1*fixed_x+r2*(node_x-fixed_x))/(1+r1);
							node_y=(node_y+r1*fixed_y+r2*(node_y-fixed_y))/(1+r1);
							node_z=(node_z+r1*fixed_z+r2*(node_z-fixed_z))/(1+r1);
							return_code=FE_node_set_position_cartesian(temp_node,
								coordinate_field,node_x,node_y,node_z);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"update_node.  Coordinate fields don't match");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"update_node.  No coordinate field for fixed node");
						return_code=0;
					}
				} break;
				case PIVOT_SLIDER:
				{
					if (coordinate_field=FE_node_get_position_cartesian(temp_node,
						(struct FE_field *)NULL,&axis2_x,&axis2_y,&axis2_z,
						(FE_value *)NULL))
					{
						axis1_x=((update_node_data->type_data->data).pivot.x2)-
							((update_node_data->type_data->data).pivot.x1);
						axis1_y=((update_node_data->type_data->data).pivot.y2)-
							((update_node_data->type_data->data).pivot.y1);
						axis1_z=((update_node_data->type_data->data).pivot.z2)-
							((update_node_data->type_data->data).pivot.z1);
						axis2_x -= ((update_node_data->type_data->data).pivot.x1);
						axis2_y -= ((update_node_data->type_data->data).pivot.y1);
						axis2_z -= ((update_node_data->type_data->data).pivot.z1);
						length=axis1_x*axis1_x+axis1_y*axis1_y+axis1_z*axis1_z;
						temp=(axis1_x*axis2_x+axis1_y*axis2_y+axis1_z*axis2_z)/length;
						length=sqrt(length);
						axis2_x -= axis1_x*temp;
						axis2_y -= axis1_y*temp;
						axis2_z -= axis1_z*temp;
						axis3_x=axis1_y*axis2_z-axis1_z*axis2_y;
						axis3_y=axis1_z*axis2_x-axis1_x*axis2_z;
						axis3_z=axis1_x*axis2_y-axis1_y*axis2_x;
						axis1_x *= temp;
						axis1_y *= temp;
						axis1_z *= temp;
						axis3_x /= length;
						axis3_y /= length;
						axis3_z /= length;
						temp=PI*((update_node_data->new_slider_value)-
							(update_node_data->old_slider_value));
						cosine=cos(temp);
						sine=sin(temp);
						node_x=((update_node_data->type_data->data).pivot.x1)+
							axis1_x+cosine*axis2_x+sine*axis3_x;
						node_y=((update_node_data->type_data->data).pivot.y1)+
							axis1_y+cosine*axis2_y+sine*axis3_y;
						node_z=((update_node_data->type_data->data).pivot.z1)+
							axis1_z+cosine*axis2_z+sine*axis3_z;
						return_code=FE_node_set_position_cartesian(temp_node,
							coordinate_field,node_x,node_y,node_z);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"update_node.  No coordinate field for node");
						return_code=0;
					}
				} break;
				case EM_SLIDER:
				{
					display_message(WARNING_MESSAGE,
						"update_node.  EM_SLIDER not yet implemented");
					return_code=1;
				} break;
			}
			if (return_code)
			{
				return_code=MANAGER_MODIFY_NOT_IDENTIFIER(FE_node,cm_node_identifier)(
					node,temp_node,update_node_data->node_manager);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"update_node.  Could not create temp_node");
			return_code=0;
		}
		if (temp_node)
		{
			DESTROY(FE_node)(&temp_node);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"update_node.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* update_node */

static int update_node_group_slider(struct Node_group_slider *node_group_slider)
/*******************************************************************************
LAST MODIFIED : 3 November 1998

DESCRIPTION :
Updates the node locations for the <node_group_slider>
==============================================================================*/
{
	double temp_double,*u,*weights;
	FE_value node_x,node_y,node_z;
	float new_value;
	int i,j,k,offset,return_code,slider_value;
	struct FE_field_component coordinate_field_component;
	struct FE_node *node,*temp_node;
	struct Shared_em_slider_data *shared_em_data;
	struct EM_Object *em_object;
	struct Update_node_data update_node_data;

	ENTER(update_node_group_slider);
	if (node_group_slider)
	{
		XtVaGetValues(node_group_slider->slider,XmNvalue,&slider_value,NULL);
		new_value=
			((node_group_slider->minimum)*(float)(SLIDER_RESOLUTION-slider_value)+
			(node_group_slider->maximum)*(float)(slider_value))/
			(float)SLIDER_RESOLUTION;
		switch ((node_group_slider->type_data).type)
		{
			case EM_SLIDER:
			{
				return_code=1;
				shared_em_data=(node_group_slider->type_data).data.em.shared;
				(shared_em_data->weights)[(node_group_slider->type_data).data.em.
					index]=new_value;
				em_object=shared_em_data->em_object;
				if ((shared_em_data->count>=shared_em_data->number_of_sliders)&&
					(0<em_object->n_nodes))
				{
					if ((node=FIND_BY_IDENTIFIER_IN_MANAGER(FE_node,cm_node_identifier)(
						(em_object->index)[0],node_group_slider->node_manager))&&
						(coordinate_field_component.field=FE_node_get_position_cartesian(
						node,(struct FE_field *)NULL,&node_x,&node_y,&node_z,
						(FE_value *)NULL)))
					{
						if (temp_node=CREATE(FE_node)(0,(struct FE_node *)NULL))
						{
							/* perform EM reconstruction */
							MANAGER_BEGIN_CACHE(FE_node)(node_group_slider->node_manager);
							i=0;
							offset=em_object->m;
							while (return_code&&(i<em_object->n_nodes))
							{
								if (node=FIND_BY_IDENTIFIER_IN_MANAGER(FE_node,cm_node_identifier)(
									(em_object->index)[i],node_group_slider->node_manager))
								{
									if (COPY(FE_node)(temp_node,node))
									{
										coordinate_field_component.number=0;
										j=0;
										while (return_code&&(j<3))
										{
											temp_double=0;
											u=(em_object->u)+(3*i+j);
											weights=shared_em_data->weights;
											for (k=em_object->n_modes;k>0;k--)
											{
												temp_double += (*u)*(*weights);
												u += offset;
												weights++;
											}
											return_code=set_FE_nodal_FE_value_value(temp_node,
												&coordinate_field_component,0,FE_NODAL_VALUE,
												(FE_value)temp_double);
											(coordinate_field_component.number)++;
											j++;
										}
										if (return_code)
										{
											return_code=MANAGER_MODIFY_NOT_IDENTIFIER(FE_node,
												cm_node_identifier)(node,temp_node,
												node_group_slider->node_manager);
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"update_node_group_slider.  Could not copy node");
										return_code=0;
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"update_node_group_slider.  Unknown node %d",
										(em_object->index)[i]);
									return_code=0;
								}
								i++;
							}
							MANAGER_END_CACHE(FE_node)(node_group_slider->node_manager);
							DESTROY(FE_node)(&temp_node);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"update_node_group_slider.  Could not create temp_node");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"update_node_group_slider.  Could not find coordinate_field");
						return_code=0;
					}
				}
				node_group_slider->value=new_value;
			} break;
			default:
			{
				if (new_value!=node_group_slider->value)
				{
					update_node_data.new_slider_value=new_value;
					update_node_data.old_slider_value=node_group_slider->value;
					update_node_data.type_data= &(node_group_slider->type_data);
					update_node_data.node_manager=node_group_slider->node_manager;
					MANAGER_BEGIN_CACHE(FE_node)(node_group_slider->node_manager);
					if (node_group_slider->node_group)
					{
						return_code=FOR_EACH_OBJECT_IN_GROUP(FE_node)(update_node,
							&update_node_data,node_group_slider->node_group);
					}
					else
					{
						return_code=FOR_EACH_OBJECT_IN_MANAGER(FE_node)(update_node,
							&update_node_data,node_group_slider->node_manager);
					}
					MANAGER_END_CACHE(FE_node)(node_group_slider->node_manager);
					node_group_slider->value=new_value;
				}
				else
				{
					return_code=1;
				}
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"update_node_group_slider.  Missing node_group_slider");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* update_node_group_slider */

static void update_node_group_slider_callback(Widget widget,
	XtPointer node_group_slider_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 29 April 1997

DESCRIPTION :
Slider scrollbar callback.  Updates the node locations and then executes
"open comfile redraw ex"
==============================================================================*/
{
	struct Node_group_slider *node_group_slider;

	ENTER(update_node_group_slider_callback);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (node_group_slider=(struct Node_group_slider *)node_group_slider_structure)
	{
		update_node_group_slider(node_group_slider);
		(*(node_group_slider->execute_command->function))(
			"open comfile redraw execute",node_group_slider->execute_command->data);
	}
	else
	{
		display_message(ERROR_MESSAGE,
		"update_node_group_slider_callback.  Missing node_group_slider_structure");
	}
	LEAVE;
} /* update_node_group_slider_callback */

static void update_variable_slider_callback(Widget widget,
	XtPointer Update_em_slider_data_void, XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 18 November 1999

DESCRIPTION :
Variable slider scrollbar callback.  Updates all the other variables using
the Control_curve from the EM
==============================================================================*/
{
	int i, slider_value;
	float end_time, new_value, *shape_vector, start_time;
	struct Update_em_slider_data *update_em_slider_data;
	struct Control_curve *em_variable;

	ENTER(update_variable_slider_callback);
	USE_PARAMETER(call_data);
	if (update_em_slider_data=(struct Update_em_slider_data *)Update_em_slider_data_void)
	{
		em_variable = update_em_slider_data->shared_data->em_variable;
		XtVaGetValues(widget, XmNvalue, &slider_value, NULL);
		Control_curve_get_parameter_range(em_variable, &start_time, &end_time);
		new_value=
			((start_time)*(float)(SLIDER_RESOLUTION-slider_value)+
			(end_time)*(float)(slider_value))/
			(float)SLIDER_RESOLUTION;
		if (ALLOCATE(shape_vector,FE_value,
			Control_curve_get_number_of_components(em_variable)))
		{
			if (Control_curve_get_values_at_parameter( em_variable, new_value,
				shape_vector, /*derivatives*/(FE_value *)NULL))
			{
				update_em_slider_data->shared_data->count=1;
				for ( i = 0 ; i < update_em_slider_data->node_group_slider_dialog->number_of_sliders ; i++ )
				{
					set_node_group_slider_function ( update_em_slider_data->node_group_slider_dialog->sliders[i], shape_vector[i] );
					update_em_slider_data->shared_data->count++;
				}
				(*(update_em_slider_data->node_group_slider_dialog->execute_command->function))(
					"open comfile redraw execute",
					update_em_slider_data->node_group_slider_dialog->execute_command->data);
			}
			DEALLOCATE( shape_vector );
		}

	}
	else
	{
		display_message(ERROR_MESSAGE,
		"update_variable_slider_callback.  Missing update_variable_slider_structure");
	}
	LEAVE;
} /* update_variable_slider_callback */

static void destroy_variable_slider_callback(Widget widget,
	XtPointer Update_em_slider_data_void, XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 3 April 1998

DESCRIPTION :
Destroy Variable slider scrollbar callback.  Deallocates the structure for the
slider
==============================================================================*/
{
	struct Update_em_slider_data *update_em_slider_data;

	ENTER(destroy_variable_slider_callback);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (update_em_slider_data=(struct Update_em_slider_data *)Update_em_slider_data_void)
	{
		DEALLOCATE( update_em_slider_data );
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"destroy_variable_slider_callback.  Missing update_variable_slider_structure");
	}
	LEAVE;
} /* destroy_variable_slider_callback */

static void reset_node_group_sliders_callback(Widget widget,
	XtPointer node_group_slider_dialog_structure,XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 2 March 1998

DESCRIPTION :
Reset button callback.  Updates the node locations and then executes
"open comfile redraw ex"
==============================================================================*/
{
	int first_em,i;
	struct Node_group_slider *node_group_slider;
	struct Node_group_slider_dialog *node_group_slider_dialog;
	Widget *slider;

	ENTER(reset_node_group_sliders_callback);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (node_group_slider_dialog=
		(struct Node_group_slider_dialog *)node_group_slider_dialog_structure)
	{
		slider=node_group_slider_dialog->sliders;
		first_em=1;
		for (i=node_group_slider_dialog->number_of_sliders;i>0;i--)
		{
			XtVaGetValues(*slider,XmNuserData,&node_group_slider,NULL);
			if (node_group_slider)
			{
				if (EM_SLIDER==(node_group_slider->type_data).type)
				{
					if (first_em)
					{
						first_em=0;
						((node_group_slider->type_data).data.em.shared)->count=1;
					}
					else
					{
						(((node_group_slider->type_data).data.em.shared)->count)++;
					}
				}
				XtVaSetValues(node_group_slider->slider,XmNvalue,
					(int)((float)SLIDER_RESOLUTION*((node_group_slider->initial_value)-
					(node_group_slider->minimum))/((node_group_slider->maximum)-
					(node_group_slider->minimum))),NULL);
				update_node_group_slider(node_group_slider);
			}
			slider++;
		}
		(*(node_group_slider_dialog->execute_command->function))(
			"open comfile redraw execute",
			node_group_slider_dialog->execute_command->data);
	}
	else
	{
		display_message(ERROR_MESSAGE,
"reset_node_group_sliders_callback.  Missing node_group_slider_dialog_structure");
	}
	LEAVE;
} /* reset_node_group_sliders_callback */

static Widget create_Node_group_slider(char *name,Widget parent,float minimum,
	float value,float maximum,struct Node_group_slider_type_data *type_data,
	struct GROUP(FE_node) *node_group,struct MANAGER(FE_node) *node_manager,
	struct Execute_command *execute_command,struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 3 November 1998

DESCRIPTION :
If there is a node_group_slider dialog in existence, then bring it to the front,
else create a new one.  If there is not a slider for the <fixed_node> and
<node_group> then create one.
==============================================================================*/
{
	char *group_name,*label;
	struct Node_group_slider *node_group_slider;
	Widget node_group_slider_widget;
	XmString label_string;

	ENTER(create_Node_group_slider);
	/* check arguments */
	if (name&&parent&&(minimum<maximum)&&(minimum<=value)&&(value<=maximum)&&
		type_data&&execute_command&&user_interface)
	{
		if (node_group_slider_widget=XtVaCreateManagedWidget("slider_form",
			xmFormWidgetClass,parent,
			XmNleftAttachment,XmATTACH_FORM,
			XmNleftOffset,user_interface->widget_spacing,
			XmNrightAttachment,XmATTACH_FORM,
			XmNrightOffset,user_interface->widget_spacing,
			XmNtopAttachment,XmATTACH_WIDGET,
			XmNtopOffset,user_interface->widget_spacing,
			XmNbottomAttachment,XmATTACH_NONE,NULL))
		{
			if (ALLOCATE(node_group_slider,struct Node_group_slider,1)&&
				ALLOCATE(node_group_slider->name,char,strlen(name)+1))
			{
				strcpy(node_group_slider->name,name);
				if (node_group)
				{
					if (GET_NAME(GROUP(FE_node))(node_group,&group_name))
					{
						switch (type_data->type)
						{
							case MUSCLE_SLIDER:
							{
								if (ALLOCATE(label,char,strlen(name)+strlen(group_name)+22+
									(int)(2+log10(fabs((double)get_FE_node_cm_node_identifier(
									(type_data->data).muscle.fixed_node))))))
								{
									sprintf(label,"%s.  Muscle.  Fixed %d.  %s",name,
										get_FE_node_cm_node_identifier((type_data->data).muscle.
										fixed_node),group_name);
									label_string=XmStringCreateSimple(label);
									DEALLOCATE(label);
								}
								else
								{
									label_string=(XmString)NULL;
								}
							} break;
							case PIVOT_SLIDER:
							{
								if (ALLOCATE(label,char,strlen(name)+strlen(group_name)+12))
								{
									sprintf(label,"%s.  Pivot.  %s",name,group_name);
									label_string=XmStringCreateSimple(label);
									DEALLOCATE(label);
								}
								else
								{
									label_string=(XmString)NULL;
								}
							} break;
							case EM_SLIDER:
							{
								/*???DB.  More to do */
								if (ALLOCATE(label,char,strlen(name)+strlen(group_name)+10))
								{
									sprintf(label,"%s.  EM.  %s",name,group_name);
									label_string=XmStringCreateSimple(label);
									DEALLOCATE(label);
								}
								else
								{
									label_string=(XmString)NULL;
								}
							} break;
						}
						if (!label_string)
						{
							display_message(ERROR_MESSAGE,
								"create_Node_group_slider.  Could not allocate label string");
						}
						DEALLOCATE(group_name);
					}
				}
				else
				{
					switch (type_data->type)
					{
						case MUSCLE_SLIDER:
						{
							if (ALLOCATE(label,char,strlen(name)+31+
								(int)(2+log10(fabs((double)get_FE_node_cm_node_identifier(
								(type_data->data).muscle.fixed_node))))))
							{
								sprintf(label,"%s.  Muscle.  Fixed %d.  All nodes",name,
									get_FE_node_cm_node_identifier((type_data->data).muscle.
									fixed_node));
								label_string=XmStringCreateSimple(label);
								DEALLOCATE(label);
							}
							else
							{
								label_string=(XmString)NULL;
							}
						} break;
						case PIVOT_SLIDER:
						{
							if (ALLOCATE(label,char,strlen(name)+21))
							{
								sprintf(label,"%s.  Pivot.  All nodes",name);
								label_string=XmStringCreateSimple(label);
								DEALLOCATE(label);
							}
							else
							{
								label_string=(XmString)NULL;
							}
						} break;
						case EM_SLIDER:
						{
							/*???DB.  More to do */
							if (ALLOCATE(label,char,strlen(name)+19))
							{
								sprintf(label,"%s.  EM.  All nodes",name);
								label_string=XmStringCreateSimple(label);
								DEALLOCATE(label);
							}
							else
							{
								label_string=(XmString)NULL;
							}
						} break;
					}
					if (!label_string)
					{
						display_message(ERROR_MESSAGE,
							"create_Node_group_slider.  Could not allocate label string");
					}
				}
				if ((node_group_slider->label=XtVaCreateManagedWidget("label",
						xmLabelWidgetClass,node_group_slider_widget,
						XmNlabelString,label_string,
						XmNrightAttachment,XmATTACH_FORM,
						XmNrightOffset,0,
						XmNleftAttachment,XmATTACH_NONE,
						XmNtopAttachment,XmATTACH_FORM,
						XmNtopOffset,0,
						XmNbottomAttachment,XmATTACH_FORM,
						XmNbottomOffset,0,NULL))&&
					(node_group_slider->slider=XtVaCreateManagedWidget("slider",
						xmScrollBarWidgetClass,node_group_slider_widget,
						XmNvalue,
						(int)((float)SLIDER_RESOLUTION*(value-minimum)/(maximum-minimum)),
						XmNminimum,0,
						XmNmaximum,(int)(1.05*(float)SLIDER_RESOLUTION),
						XmNpageIncrement,(int)(0.05*(float)SLIDER_RESOLUTION),
						XmNsliderSize,(int)(0.05*(float)SLIDER_RESOLUTION),
						XmNorientation,XmHORIZONTAL,
						XmNleftAttachment,XmATTACH_FORM,
						XmNleftOffset,0,
						XmNrightAttachment,XmATTACH_WIDGET,
						XmNrightOffset,user_interface->widget_spacing,
						XmNrightWidget,node_group_slider->label,
						XmNtopAttachment,XmATTACH_FORM,
						XmNtopOffset,0,
						XmNbottomAttachment,XmATTACH_FORM,
						XmNbottomOffset,0,NULL)))
				{
					XtAddCallback(node_group_slider->slider,XmNvalueChangedCallback,
						update_node_group_slider_callback,node_group_slider);
					XtVaSetValues(node_group_slider_widget,XmNuserData,node_group_slider,
						NULL);
					node_group_slider->initial_value=value;
					node_group_slider->minimum=minimum;
					node_group_slider->maximum=maximum;
					node_group_slider->node_group=node_group;
					node_group_slider->node_manager=node_manager;
					node_group_slider->execute_command=execute_command;
					(node_group_slider->type_data).type=type_data->type;
					switch (type_data->type)
					{
						case MUSCLE_SLIDER:
						{
							node_group_slider->value=1;
							(node_group_slider->type_data).data.muscle.fixed_node=
								(type_data->data).muscle.fixed_node;
							(node_group_slider->type_data).data.muscle.coefficient.field=
								(type_data->data).muscle.coefficient.field;
							(node_group_slider->type_data).data.muscle.coefficient.number=
								(type_data->data).muscle.coefficient.number;
						} break;
						case PIVOT_SLIDER:
						{
							node_group_slider->value=0;
							(node_group_slider->type_data).data.pivot.x1=
								(type_data->data).pivot.x1;
							(node_group_slider->type_data).data.pivot.y1=
								(type_data->data).pivot.y1;
							(node_group_slider->type_data).data.pivot.z1=
								(type_data->data).pivot.z1;
							(node_group_slider->type_data).data.pivot.x2=
								(type_data->data).pivot.x2;
							(node_group_slider->type_data).data.pivot.y2=
								(type_data->data).pivot.y2;
							(node_group_slider->type_data).data.pivot.z2=
								(type_data->data).pivot.z2;
						} break;
						case EM_SLIDER:
						{
							/*???DB.  More to do */
							node_group_slider->value=2*minimum-maximum-1;
							(node_group_slider->type_data).data.em.index=
								(type_data->data).em.index;
							(node_group_slider->type_data).data.em.shared=
								(type_data->data).em.shared;
							(((type_data->data).em.shared)->count)++;
						} break;
					}
					update_node_group_slider(node_group_slider);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_Node_group_slider.  Could not create widgets");
					XtDestroyWidget(node_group_slider_widget);
					node_group_slider_widget=(Widget)NULL;
				}
				if (label_string)
				{
					XmStringFree(label_string);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_Node_group_slider.  Could not create structure");
				DEALLOCATE(node_group_slider);
				XtDestroyWidget(node_group_slider_widget);
				node_group_slider_widget=(Widget)NULL;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_Node_group_slider.  Could not create form");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_Node_group_slider.  Invalid argument(s)");
		node_group_slider_widget=(Widget)NULL;
	}
	LEAVE;

	return (node_group_slider_widget);
} /* create_Node_group_slider */

static int bring_up_node_group_slider_dialog(char *name,float minimum,
	float value,float maximum,struct Node_group_slider_type_data *type_data,
	struct GROUP(FE_node) *node_group,Widget *node_group_slider_dialog_address,
	Widget parent,struct MANAGER(FE_node) *node_manager,
	struct Execute_command *execute_command,struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 2 June 1997

DESCRIPTION :
If there is a node_group_slider dialog in existence, then bring it to the front,
else create a new one.  If there is not a slider for the <fixed_node> and
<node_group> then create one.
==============================================================================*/
{
	int found,i,number_of_children,return_code;
	struct Node_group_slider *node_group_slider;
	struct Node_group_slider_dialog *node_group_slider_dialog;
	struct Update_em_slider_data *update_em_slider_data;
	Widget *child_list,node_group_slider_dialog_widget,node_group_slider_widget,
		*slider;
	XmString label_string;

	ENTER(bring_up_node_group_slider_dialog);
	/* check arguments */
	if (name&&node_group_slider_dialog_address&&type_data&&
		(minimum<maximum)&&(minimum<=value)&&(value<=maximum)&&execute_command&&
		user_interface)
	{
		if (node_group_slider_dialog_widget= *node_group_slider_dialog_address)
		{
			/* check if there is an existing slider with the same name */
			XtVaGetValues(node_group_slider_dialog_widget,
				XmNchildren,&child_list,
				XmNnumChildren,&number_of_children,
				NULL);
			if (1==number_of_children)
			{
				XtVaGetValues(*child_list,
					XmNuserData,&node_group_slider_dialog,
					NULL);
				if (node_group_slider_dialog)
				{
					slider=node_group_slider_dialog->sliders;
					i=node_group_slider_dialog->number_of_sliders;
					return_code=1;
					found=0;
					while ((i>0)&&(!found)&&return_code)
					{
						XtVaGetValues(*slider,
							XmNuserData,&node_group_slider,
							NULL);
						if (node_group_slider)
						{
							if (0==strcmp(name,node_group_slider->name))
							{
								display_message(ERROR_MESSAGE,"Slider %s already exists",
									name);
								found=1;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
							"bring_up_node_group_slider_dialog.  Missing node_group_slider");
							return_code=0;
						}
						slider++;
						i--;
					}
					if (return_code&&!found)
					{
						if (node_group_slider_widget=create_Node_group_slider(name,
							node_group_slider_dialog->outer_form,minimum,value,maximum,
							type_data,node_group,node_manager,
							node_group_slider_dialog->execute_command,
							node_group_slider_dialog->user_interface))
						{
							if (REALLOCATE(slider,node_group_slider_dialog->sliders,
								Widget,(node_group_slider_dialog->number_of_sliders)+1))
							{
								node_group_slider_dialog->sliders=slider;
								(node_group_slider_dialog->sliders)[
									node_group_slider_dialog->number_of_sliders]=
									node_group_slider_widget;
								(node_group_slider_dialog->number_of_sliders)++;
								XtVaSetValues(node_group_slider_widget,
									XmNtopWidget,(node_group_slider_dialog->sliders)[
									node_group_slider_dialog->number_of_sliders-2],
									NULL);
								XtVaSetValues(node_group_slider_dialog->separator,
									XmNtopWidget,node_group_slider_widget,
									NULL);
							}
							else
							{
								display_message(ERROR_MESSAGE,
						"bring_up_node_group_slider_dialog.  Could not reallocate sliders");
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"bring_up_node_group_slider_dialog.  Could not create slider");
							return_code=0;
						}
					}
					if (return_code)
					{
						XtPopup(node_group_slider_dialog_widget,XtGrabNone);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
				"bring_up_node_group_slider_dialog.  Missing node_group_slider_dialog");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"bring_up_node_group_slider_dialog.  Invalid dialog");
				return_code=0;
			}
		}
		else
		{
			if (node_group_slider_dialog_widget=XtVaCreatePopupShell(
				"node_group_slider_dialog",topLevelShellWidgetClass,parent,
				XmNtitle,"Sliders",
				XmNallowShellResize,True,
				NULL))
			{
				if (ALLOCATE(node_group_slider_dialog,struct Node_group_slider_dialog,
					1)&&ALLOCATE(node_group_slider_dialog->sliders,Widget,1))
				{
					label_string=XmStringCreateSimple("Reset");
					if ((node_group_slider_dialog->outer_form=XtVaCreateWidget(
						"outer_form",xmFormWidgetClass,node_group_slider_dialog_widget,
						NULL))&&
						(node_group_slider_dialog->reset_button=XtVaCreateManagedWidget(
							"reset_button",xmPushButtonWidgetClass,
							node_group_slider_dialog->outer_form,
							XmNlabelString,label_string,
							XmNleftAttachment,XmATTACH_FORM,
							XmNleftOffset,user_interface->widget_spacing,
							XmNrightAttachment,XmATTACH_NONE,
							XmNtopAttachment,XmATTACH_NONE,
							XmNbottomAttachment,XmATTACH_FORM,
							XmNbottomOffset,user_interface->widget_spacing,NULL))&&
						(node_group_slider_dialog->separator=XtVaCreateManagedWidget(
							"separator",xmSeparatorWidgetClass,
							node_group_slider_dialog->outer_form,
							XmNseparatorType,XmNO_LINE,
							XmNleftAttachment,XmATTACH_FORM,
							XmNleftOffset,user_interface->widget_spacing,
							XmNrightAttachment,XmATTACH_FORM,
							XmNrightOffset,user_interface->widget_spacing,
							XmNbottomAttachment,XmATTACH_WIDGET,
							XmNbottomOffset,user_interface->widget_spacing,
							XmNbottomWidget,node_group_slider_dialog->reset_button,
							XmNtopAttachment,XmATTACH_WIDGET,
							XmNtopOffset,user_interface->widget_spacing,NULL)))
					{
						XtVaSetValues(node_group_slider_dialog->outer_form,
							XmNuserData,node_group_slider_dialog,
							NULL);
						XtAddCallback(node_group_slider_dialog->reset_button,
							XmNactivateCallback,reset_node_group_sliders_callback,
							node_group_slider_dialog);
						if (node_group_slider_widget=create_Node_group_slider(name,
							node_group_slider_dialog->outer_form,minimum,value,maximum,
							type_data,node_group,node_manager,execute_command,user_interface))
						{
							XtManageChild(node_group_slider_dialog->outer_form);
							*(node_group_slider_dialog->sliders)=node_group_slider_widget;
							node_group_slider_dialog->number_of_sliders=1;
							node_group_slider_dialog->user_interface=user_interface;
							node_group_slider_dialog->execute_command=execute_command;
							XtVaSetValues(node_group_slider_widget,
								XmNtopAttachment,XmATTACH_FORM,
								NULL);
							XtVaSetValues(node_group_slider_dialog->separator,
								XmNtopWidget,node_group_slider_widget,
								NULL);

							if ( EM_SLIDER == type_data->type )
							{
								/* SAB Add an extra slider to control frame number */
								if (node_group_slider_dialog->variable_slider_form =
									XtVaCreateManagedWidget("variable_slider_form",
									xmFormWidgetClass, node_group_slider_dialog->outer_form,
									XmNleftAttachment,XmATTACH_FORM,
									XmNleftOffset,user_interface->widget_spacing,
									XmNrightAttachment,XmATTACH_FORM,
									XmNrightOffset,user_interface->widget_spacing,
									XmNtopAttachment,XmATTACH_NONE,
									XmNbottomAttachment,XmATTACH_WIDGET,
									XmNbottomOffset,user_interface->widget_spacing,
									XmNbottomWidget,node_group_slider_dialog->reset_button,
									NULL))
								{
									XtVaSetValues(node_group_slider_dialog->separator,
											XmNbottomWidget,node_group_slider_dialog->variable_slider_form,
											NULL);


									if ((node_group_slider_dialog->variable_label=XtVaCreateManagedWidget("EM Frame Control",
										xmLabelWidgetClass,node_group_slider_dialog->variable_slider_form,
										XmNrightAttachment,XmATTACH_FORM,
										XmNrightOffset,0,
										XmNleftAttachment,XmATTACH_NONE,
										XmNtopAttachment,XmATTACH_FORM,
										XmNtopOffset,0,
										XmNbottomAttachment,XmATTACH_FORM,
										XmNbottomOffset,0,NULL))&&
										(node_group_slider_dialog->variable_slider=XtVaCreateManagedWidget("em_slider",
										xmScrollBarWidgetClass,node_group_slider_dialog->variable_slider_form,
										XmNvalue,
										(int)((float)SLIDER_RESOLUTION*(value-minimum)/(maximum-minimum)),
										XmNminimum,0,
										XmNmaximum,(int)(1.05*(float)SLIDER_RESOLUTION),
										XmNpageIncrement,(int)(0.05*(float)SLIDER_RESOLUTION),
										XmNsliderSize,(int)(0.05*(float)SLIDER_RESOLUTION),
										XmNorientation,XmHORIZONTAL,
										XmNleftAttachment,XmATTACH_FORM,
										XmNleftOffset,0,
										XmNrightAttachment,XmATTACH_WIDGET,
										XmNrightOffset,user_interface->widget_spacing,
										XmNrightWidget,node_group_slider_dialog->variable_label,
										XmNtopAttachment,XmATTACH_FORM,
										XmNtopOffset,0,
										XmNbottomAttachment,XmATTACH_FORM,
										XmNbottomOffset,0,NULL)))
									{
										if ( ALLOCATE( update_em_slider_data,
											struct Update_em_slider_data, 1))
										{
											update_em_slider_data->node_group_slider_dialog
												= node_group_slider_dialog;
											update_em_slider_data->shared_data
												= type_data->data.em.shared;
											XtAddCallback(node_group_slider_dialog->variable_slider,XmNvalueChangedCallback,
													update_variable_slider_callback, update_em_slider_data);
											XtAddCallback(node_group_slider_dialog->variable_slider,XmNdragCallback,
													update_variable_slider_callback, update_em_slider_data);
											XtAddCallback(node_group_slider_dialog->variable_slider,XmNdestroyCallback,
													destroy_variable_slider_callback, update_em_slider_data);
											XtVaSetValues(node_group_slider_dialog->variable_slider,
												XmNuserData,update_em_slider_data,
												NULL);
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"bring_up_node_group_slider_dialog.  Unable to allocate update_em_slider_data");
											return_code=0;
										}
									}
								}
							}
							XtRealizeWidget(node_group_slider_dialog_widget);
							XtPopup(node_group_slider_dialog_widget,XtGrabNone);
							*node_group_slider_dialog_address=node_group_slider_dialog_widget;
							return_code=1;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"bring_up_node_group_slider_dialog.  Could not create slider");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
"bring_up_node_group_slider_dialog.  Could not create node_group_slider_dialog widgets");
						DEALLOCATE(node_group_slider_dialog);
						XtDestroyWidget(node_group_slider_dialog_widget);
							/*???DB.  Will destroy any children created */
						return_code=0;
					}
					XmStringFree(label_string);
				}
				else
				{
					display_message(ERROR_MESSAGE,
"bring_up_node_group_slider_dialog.  Could not allocate node_group_slider_dialog");
					DEALLOCATE(node_group_slider_dialog);
					XtDestroyWidget(node_group_slider_dialog_widget);
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"bring_up_node_group_slider_dialog.  Could not create shell");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"bring_up_node_group_slider_dialog.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* bring_up_node_group_slider_dialog */

static int create_em_Control_curve( struct Control_curve **em_variable_addr,
	struct MANAGER(Control_curve) *control_curve_manager,
	struct EM_Object *em_object )
/*******************************************************************************
LAST MODIFIED : 18 November 1999

DESCRIPTION :
Creates a time variable which corresponds to the V and W matricies in the EM
decomposition.
==============================================================================*/
{
	FE_value time;
	float *shape_vector;
	int i, j, return_code;
	struct Control_curve *em_variable;

	ENTER(create_em_Control_curve);

	if ( em_variable_addr && control_curve_manager && em_object )
	{
		if ( ALLOCATE( shape_vector, float, em_object->n_modes ))
		{
			if (em_variable = CREATE(Control_curve)("em_variable",
				LINEAR_LAGRANGE, em_object->n_modes ))
			{
				if (ADD_OBJECT_TO_MANAGER(Control_curve)(
					em_variable, control_curve_manager))
				{
					/* SAB Do the first and second modes as special case
						as we need to create the first element, position both
						nodes and then from then on the current elements can
						just be appended to */
					Control_curve_add_element( em_variable, 1 );
					time = 1.0;
					Control_curve_set_parameter( em_variable,/*element_no*/1,
						/*local_node_no*/0,time);
					time += 1.0;
					Control_curve_set_parameter( em_variable,/*element_no*/1,
						/*local_node_no*/1,time);
					for ( j = 0 ; j < em_object->n_modes ; j++ )
					{
						shape_vector[j] = em_object->v[j] * em_object->w[j];
					}
					Control_curve_set_node_values( em_variable,
							1, 0, shape_vector);

					for ( j = 0 ; j < em_object->n_modes ; j++ )
					{
						shape_vector[j] = em_object->v[em_object->n_modes + j]
							* em_object->w[j];
					}
					Control_curve_set_node_values( em_variable,
						1, 1, shape_vector);

					for ( i = 2 ; i < em_object->n_modes ; i++ )
					{
						Control_curve_add_element( em_variable, i );
						time += 1.0;
						Control_curve_set_parameter( em_variable,/*element_no*/i,
							/*local_node_no*/1,time);
						for ( j = 0 ; j < em_object->n_modes ; j++ )
						{
							shape_vector[j] = em_object->v[i * em_object->n_modes + j]
								* em_object->w[j];
						}
						Control_curve_set_node_values( em_variable,
							i, 1, shape_vector);
					}
					*em_variable_addr = em_variable;
					return_code = 1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
							"create_em_Control_curve.  Unable to add time variable to manager");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_em_Control_curve.  Unable to create time variable");
				return_code=0;
			}
			DEALLOCATE (shape_vector);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_em_Control_curve.  Unable to allocate shape vector");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_em_Control_curve.  Invalid argument(s)");
		return_code=0;
	}

	LEAVE;

	return (return_code);
} /* create_em_Control_curve */



/*
Global functions
----------------
*/
int create_muscle_slider(struct Parse_state *state,void *dummy_to_be_modified,
	void *create_node_group_slider_data_void)
/*******************************************************************************
LAST MODIFIED : 29 January 1999

DESCRIPTION :
Executes a GFX CREATE MUSCLE_SLIDER command.  If there is a node group slider
dialog in existence, then bring it to the front, otherwise create new one.  If
the fixed node and node group don't have a slider in the slider dialog then add
a new slider.
???DB.  Temporary command ?
==============================================================================*/
{
	char *slider_name;
	float maximum,minimum,value;
	int return_code;
	auto struct Modifier_entry
		help_option_table[]=
		{
			{"SLIDER_NAME",NULL,NULL,create_muscle_slider},
			{NULL,NULL,NULL,NULL}
		},
		option_table[]=
		{
			{"coefficient",NULL,NULL,set_FE_field_component},
			{"fixed_node",NULL,NULL,set_FE_node},
			{"maximum",NULL,NULL,set_float},
			{"minimum",NULL,NULL,set_float},
			{"ngroup",NULL,NULL,set_FE_node_group},
			{"value",NULL,NULL,set_float},
			{NULL,NULL,NULL,NULL}
		};
	struct Create_node_group_slider_data *create_node_group_slider_data;
	struct GROUP(FE_node) *node_group;
	struct Node_group_slider_type_data type_data;

	ENTER(create_muscle_slider);
	if (state&&(create_node_group_slider_data=
		(struct Create_node_group_slider_data *)
		create_node_group_slider_data_void)&&
		(create_node_group_slider_data->node_group_slider_dialog_address)&&
		(create_node_group_slider_data->fe_field_manager)&&
		(create_node_group_slider_data->node_manager)&&
		(create_node_group_slider_data->node_group_manager)&&
		(create_node_group_slider_data->execute_command))
	{
		if (slider_name=state->current_token)
		{
			return_code=1;
			/* initialise defaults */
			type_data.type=MUSCLE_SLIDER;
			type_data.data.muscle.coefficient.field=(struct FE_field *)NULL;
			type_data.data.muscle.coefficient.number=0;
			(option_table[0]).to_be_modified= &(type_data.data.muscle.coefficient);
			(option_table[0]).user_data=
				create_node_group_slider_data->fe_field_manager;
			type_data.data.muscle.fixed_node=FIRST_OBJECT_IN_MANAGER_THAT(FE_node)(
				(MANAGER_CONDITIONAL_FUNCTION(FE_node) *)NULL,(void *)NULL,
				create_node_group_slider_data->node_manager);
			if (type_data.data.muscle.fixed_node)
			{
				ACCESS(FE_node)(type_data.data.muscle.fixed_node);
			}
			(option_table[1]).to_be_modified= &(type_data.data.muscle.fixed_node);
			(option_table[1]).user_data=create_node_group_slider_data->node_manager;
			maximum=2;
			(option_table[2]).to_be_modified= &maximum;
			minimum=0;
			(option_table[3]).to_be_modified= &minimum;
			node_group=(struct GROUP(FE_node) *)NULL;
			(option_table[4]).to_be_modified= &node_group;
			(option_table[4]).user_data=
				create_node_group_slider_data->node_group_manager;
			value=1;
			(option_table[5]).to_be_modified= &value;
			if (strcmp(PARSER_HELP_STRING,slider_name)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,slider_name))
			{
				if (type_data.data.muscle.fixed_node)
				{
					return_code=shift_Parse_state(state,1);
				}
				else
				{
					display_message(ERROR_MESSAGE,"No nodes defined");
					return_code=0;
				}
			}
			else
			{
				if (!dummy_to_be_modified)
				{
					help_option_table[0].to_be_modified=(void *)1;
					help_option_table[0].user_data=create_node_group_slider_data_void;
					process_option(state,help_option_table);
					return_code=0;
				}
			}
			if (return_code)
			{
				if ((create_node_group_slider_data->parent)&&
					create_node_group_slider_data->user_interface)
				{
					return_code=process_multiple_options(state,option_table);
					/* no errors, not asking for help */
					if (return_code)
					{
						if ((minimum<maximum)&&(minimum<=value)&&(value<=maximum))
						{
							return_code=bring_up_node_group_slider_dialog(slider_name,minimum,
								value,maximum,&type_data,node_group,
								create_node_group_slider_data->node_group_slider_dialog_address,
								create_node_group_slider_data->parent,
								create_node_group_slider_data->node_manager,
								create_node_group_slider_data->execute_command,
								create_node_group_slider_data->user_interface);
						}
						else
						{
							display_message(ERROR_MESSAGE,"Invalid range/value %g %g %g",
								minimum,value,maximum);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"create_muscle_slider."
							"  Missing user_interface or parent widget.");
						return_code=0;
					}
				} /* parse error, help */
			}
			if (type_data.data.muscle.fixed_node)
			{
				DEACCESS(FE_node)(&(type_data.data.muscle.fixed_node));
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,"Missing muscle slider name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"create_muscle_slider.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* create_muscle_slider */

int create_pivot_slider(struct Parse_state *state,void *dummy_to_be_modified,
	void *create_node_group_slider_data_void)
/*******************************************************************************
LAST MODIFIED : 2 June 1997

DESCRIPTION :
Executes a GFX CREATE PIVOT_SLIDER command.  If there is a node group slider
dialog in existence, then bring it to the front, otherwise create new one.
???DB.  Temporary command ?
==============================================================================*/
{
	char *slider_name;
	float maximum,minimum,value;
	int return_code;
	auto struct Modifier_entry
		help_option_table[]=
		{
			{"SLIDER_NAME",NULL,NULL,create_pivot_slider},
			{NULL,NULL,NULL,NULL}
		},
		option_table[]=
		{
			{"line",NULL,NULL,set_pivot_line},
			{"maximum",NULL,NULL,set_float},
			{"minimum",NULL,NULL,set_float},
			{"ngroup",NULL,NULL,set_FE_node_group},
			{"value",NULL,NULL,set_float},
			{NULL,NULL,NULL,NULL}
		};
	struct Create_node_group_slider_data *create_node_group_slider_data;
	struct GROUP(FE_node) *node_group;
	struct Node_group_slider_type_data type_data;

	ENTER(create_pivot_slider);
	if (state&&(create_node_group_slider_data=
		(struct Create_node_group_slider_data *)
		create_node_group_slider_data_void)&&
		(create_node_group_slider_data->node_group_slider_dialog_address)&&
		(create_node_group_slider_data->node_manager)&&
		(create_node_group_slider_data->node_group_manager)&&
		(create_node_group_slider_data->execute_command))
	{
		if (slider_name=state->current_token)
		{
			return_code=1;
			/* initialise defaults */
			type_data.type=PIVOT_SLIDER;
			type_data.data.pivot.x1= -1;
			type_data.data.pivot.y1=0;
			type_data.data.pivot.z1=0;
			type_data.data.pivot.x2=1;
			type_data.data.pivot.y2=0;
			type_data.data.pivot.z2=0;
			(option_table[0]).to_be_modified= &(type_data.data.pivot);
			maximum=1;
			(option_table[1]).to_be_modified= &maximum;
			minimum= -1;
			(option_table[2]).to_be_modified= &minimum;
			node_group=(struct GROUP(FE_node) *)NULL;
			(option_table[3]).to_be_modified= &node_group;
			(option_table[3]).user_data=
				create_node_group_slider_data->node_group_manager;
			value=0;
			(option_table[4]).to_be_modified= &value;
			if (strcmp(PARSER_HELP_STRING,slider_name)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,slider_name))
			{
				return_code=shift_Parse_state(state,1);
			}
			else
			{
				if (!dummy_to_be_modified)
				{
					help_option_table[0].to_be_modified=(void *)1;
					help_option_table[0].user_data=create_node_group_slider_data_void;
					process_option(state,help_option_table);
					return_code=0;
				}
			}
			if (return_code)
			{
				if ((create_node_group_slider_data->parent)&&
					create_node_group_slider_data->user_interface)
				{
					return_code=process_multiple_options(state,option_table);
					/* no errors, not asking for help */
					if (return_code)
					{
						if ((minimum<maximum)&&(minimum<=value)&&(value<=maximum))
						{
							return_code=bring_up_node_group_slider_dialog(slider_name,minimum,
								value,maximum,&type_data,node_group,
								create_node_group_slider_data->node_group_slider_dialog_address,
								create_node_group_slider_data->parent,
								create_node_group_slider_data->node_manager,
								create_node_group_slider_data->execute_command,
								create_node_group_slider_data->user_interface);
						}
						else
						{
							display_message(ERROR_MESSAGE,"Invalid range/value %g %g %g",
								minimum,value,maximum);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"create_pivot_slider."
							"  Missing user_interface or parent widget.");
						return_code=0;
					}
				} /* parse error, help */
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,"Missing pivot slider name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"create_pivot_slider.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* create_pivot_slider */

int create_em_sliders(struct Parse_state *state,void *dummy_to_be_modified,
	void *create_node_group_slider_data_void)
/*******************************************************************************
LAST MODIFIED : 29 July 1998

DESCRIPTION :
Executes a GFX CREATE EM_SLIDER command.  If there is a node group slider
dialog in existence, then bring it to the front, otherwise create new one.
???DB.  Temporary command ?
???DB.  Not yet implemented.  Will probably create several at once
==============================================================================*/
{
	float maximum,minimum,temp_float,value;
	int i,j,number_of_modes,number_of_nodes,return_code;
	char *basis_file_name;
	struct Create_node_group_slider_data *create_node_group_slider_data;
	struct MANAGER(FE_node) *node_manager;
	struct Node_group_slider_type_data type_data;
	struct Shared_em_slider_data *shared_em_slider_data;
	struct EM_Object *em_object;

	ENTER(create_em_sliders);
	USE_PARAMETER(dummy_to_be_modified);
	return_code=0;
	/* check arguments */
	if (state&&(create_node_group_slider_data=
		(struct Create_node_group_slider_data *)
		create_node_group_slider_data_void)&&
		(create_node_group_slider_data->node_group_slider_dialog_address)&&
		(node_manager=create_node_group_slider_data->node_manager)&&
		(create_node_group_slider_data->node_group_manager)&&
		(create_node_group_slider_data->control_curve_manager)&&
		(create_node_group_slider_data->execute_command))
	{
		if (basis_file_name=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,basis_file_name)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,basis_file_name))
			{
				if((create_node_group_slider_data->parent)&&
					(create_node_group_slider_data->user_interface))
				{
#if defined (MIRAGE)
					/* read EM basis */
					em_object=(struct EM_Object *)NULL;
					if (EM_read_basis(basis_file_name,&em_object))
					{
						number_of_nodes=EM_number_of_nodes(em_object);
						number_of_modes=EM_number_of_modes(em_object);
						if ((0<number_of_nodes)&&(0<number_of_modes))
						{
							/* check that all the nodes exist */
							i=0;
							while ((i<number_of_nodes)&&FIND_BY_IDENTIFIER_IN_MANAGER(FE_node,
								cm_node_identifier)((em_object->index)[i],node_manager))
							{
								i++;
							}
							if (i==number_of_nodes)
							{
								/* create the shared EM data */
								if (ALLOCATE(shared_em_slider_data,struct Shared_em_slider_data,
									1)&&ALLOCATE(shared_em_slider_data->weights,double,
										number_of_modes))
								{
									shared_em_slider_data->em_object=em_object;
									shared_em_slider_data->number_of_sliders=number_of_modes;
									shared_em_slider_data->count=0;
									/* SAB Create a time variable with the data from VW */
									create_em_Control_curve( &shared_em_slider_data->em_variable,
										create_node_group_slider_data->control_curve_manager,
										em_object );
									type_data.type=EM_SLIDER;
									type_data.data.em.index=0;
									type_data.data.em.shared=shared_em_slider_data;
									/* create the sliders */
									i=0;
									return_code=1;
									while (return_code&&(i<number_of_modes))
									{
										/* start with the first mode */
										maximum=(float)((em_object->v)[i]);
										minimum=maximum;
										for (j=1;j<number_of_modes;j++)
										{
											value=(float)((em_object->v)[i+j*number_of_modes]);
											if (value>maximum)
											{
												maximum=value;
											}
											else
											{
												if (value<minimum)
												{
													minimum=value;
												}
											}
										}
										value=(float)(em_object->w)[i];
										maximum *= value;
										minimum *= value;
										/* increase range so that can "exagerate expressions" */
										temp_float=maximum-minimum;
										maximum += temp_float;
										minimum -= temp_float;
										value *= (float)(em_object->v)[i];
										i++;
										sprintf(global_temp_string,"mode_%d",i);
										return_code=bring_up_node_group_slider_dialog(
											global_temp_string,minimum,value,maximum,&type_data,
											(struct GROUP(FE_node) *)NULL,
											create_node_group_slider_data->
											node_group_slider_dialog_address,
											create_node_group_slider_data->parent,
											create_node_group_slider_data->node_manager,
											create_node_group_slider_data->execute_command,
											create_node_group_slider_data->user_interface);
										(type_data.data.em.index)++;
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"create_em_sliders.  Could not allocate memory for shared data");
									DEALLOCATE(shared_em_slider_data);
									destroy_EM_Object(&em_object);
									return_code=0;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,"Node %d does not exist",
									(em_object->index)[i]);
								destroy_EM_Object(&em_object);
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"Invalid nodes (%d) or modes (%d)",
								number_of_nodes,number_of_modes);
							destroy_EM_Object(&em_object);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Could not read %s",basis_file_name);
						return_code=0;
					}
#else /* defined (MIRAGE) */
					display_message(ERROR_MESSAGE,"Emoter is not supported");
					return_code=1;
#endif /* defined (MIRAGE) */
				}
				else
				{
					display_message(INFORMATION_MESSAGE,"Missing user_interface or parent widget");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE,"\n        EM_BASIS_FILE_NAME");
				return_code=1;
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,"Missing EM basis file name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"create_em_sliders.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* create_em_sliders */

int set_node_group_slider_value(struct Parse_state *state,
	void *dummy_to_be_modified,void *node_group_slider_dialog_widget_void)
/*******************************************************************************
LAST MODIFIED : 1 June 1997

DESCRIPTION :
==============================================================================*/
{
	char *slider_name;
	float value;
	int found,i,number_of_children,return_code;
	struct Node_group_slider *node_group_slider;
	struct Node_group_slider_dialog *node_group_slider_dialog;
	Widget *child_list,node_group_slider_dialog_widget,*slider;

	ENTER(set_node_group_slider_value);
	USE_PARAMETER(dummy_to_be_modified);
	/* check arguments */
	if (state)
	{
		if (slider_name=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,slider_name)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,slider_name))
			{
				if((node_group_slider_dialog_widget=
					(Widget)node_group_slider_dialog_widget_void))
				{
					if (shift_Parse_state(state,1)&&(state->current_token)&&
						(1==sscanf(state->current_token,"%f",&value)))
					{
						/* check if there is a slider with the given name */
						XtVaGetValues(node_group_slider_dialog_widget,
							XmNchildren,&child_list,
							XmNnumChildren,&number_of_children,
							NULL);
						if (1==number_of_children)
						{
							XtVaGetValues(*child_list,
								XmNuserData,&node_group_slider_dialog,
								NULL);
							if (node_group_slider_dialog)
							{
								slider=node_group_slider_dialog->sliders;
								i=node_group_slider_dialog->number_of_sliders;
								return_code=1;
								found=0;
								while ((i>0)&&(!found)&&return_code)
								{
									XtVaGetValues(*slider,
										XmNuserData,&node_group_slider,
										NULL);
									if (node_group_slider)
									{
										if (0==strcmp(slider_name,node_group_slider->name))
										{
											found=1;
											if (value<node_group_slider->minimum)
											{
												value=node_group_slider->minimum;
											}
											else
											{
												if (value>node_group_slider->maximum)
												{
													value=node_group_slider->maximum;
												}
											}
											XtVaSetValues(node_group_slider->slider,XmNvalue,
												(int)((float)SLIDER_RESOLUTION*(value-
													(node_group_slider->minimum))/
													((node_group_slider->maximum)-
														(node_group_slider->minimum))),NULL);
											update_node_group_slider(node_group_slider);
											(*(node_group_slider_dialog->execute_command->function))(
												"open comfile redraw execute",
												node_group_slider_dialog->execute_command->data);
										}
										else
										{
											slider++;
											i--;
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"set_node_group_slider_value.  Missing node_group_slider");
										return_code=0;
									}
								}
								if (return_code&&!found)
								{
									display_message(ERROR_MESSAGE,"Unknown slider %s",slider_name);
									return_code=0;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"set_node_group_slider_value.  Missing node_group_slider_dialog");
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"set_node_group_slider_value.  Invalid dialog");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Missing/invalid slider value");
						display_parse_state_location(state);
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,"Missing node group slider widget");
					return_code=0;
				}

			}
			else
			{
				display_message(INFORMATION_MESSAGE," SLIDER_NAME #");
				return_code=1;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Missing slider name");
			display_parse_state_location(state);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_node_group_slider_value.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_node_group_slider_value */

int set_node_group_slider_function(Widget slider, float value)
/*******************************************************************************
LAST MODIFIED : 2 April 1998

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Node_group_slider *node_group_slider;

	ENTER(set_node_group_slider_function);
	/* check arguments */
	if (slider)
	{
		XtVaGetValues(slider,
			XmNuserData,&node_group_slider,
			NULL);
		if (node_group_slider)
		{
			if (value<node_group_slider->minimum)
			{
				value=node_group_slider->minimum;
			}
			else
			{
				if (value>node_group_slider->maximum)
				{
					value=node_group_slider->maximum;
				}
			}
			XtVaSetValues(node_group_slider->slider,XmNvalue,
				(int)((float)SLIDER_RESOLUTION*(value-
				(node_group_slider->minimum))/
				((node_group_slider->maximum)-
				(node_group_slider->minimum))),NULL);
			update_node_group_slider(node_group_slider);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"set_node_group_slider_function.  Missing node_group_slider");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"set_node_group_slider_function.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_node_group_slider_function */

int list_node_group_slider(struct Parse_state *state,
	void *dummy_to_be_modified,void *node_group_slider_dialog_widget_void)
/*******************************************************************************
LAST MODIFIED : 2 March 1998

DESCRIPTION :
==============================================================================*/
{
	char *slider_name;
	int found,i,number_of_children,return_code;
	struct Node_group_slider *node_group_slider;
	struct Node_group_slider_dialog *node_group_slider_dialog;
	Widget *child_list,node_group_slider_dialog_widget,*slider;

	ENTER(list_node_group_slider);
	USE_PARAMETER(dummy_to_be_modified);
	/* check arguments */
	if (state)
	{
		if (slider_name=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,slider_name)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,slider_name))
			{
				if (node_group_slider_dialog_widget=
					(Widget)node_group_slider_dialog_widget_void)
				{
					/* check if there is a slider with the given name */
					XtVaGetValues(node_group_slider_dialog_widget,
						XmNchildren,&child_list,
						XmNnumChildren,&number_of_children,
						NULL);
					if (1==number_of_children)
					{
						XtVaGetValues(*child_list,
							XmNuserData,&node_group_slider_dialog,
							NULL);
						if (node_group_slider_dialog)
						{
							slider=node_group_slider_dialog->sliders;
							i=node_group_slider_dialog->number_of_sliders;
							return_code=1;
							found=0;
							while ((i>0)&&(!found)&&return_code)
							{
								XtVaGetValues(*slider,
									XmNuserData,&node_group_slider,
									NULL);
								if (node_group_slider)
								{
									if (0==strcmp(slider_name,node_group_slider->name))
									{
										found=1;
										display_message(INFORMATION_MESSAGE,
											"%s.  min=%g, val=%g, max=%g\n",
											node_group_slider->name,node_group_slider->minimum,
											node_group_slider->value,node_group_slider->maximum);
									}
									else
									{
										slider++;
										i--;
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"list_node_group_slider.  Missing node_group_slider");
									return_code=0;
								}
							}
							if (return_code&&!found)
							{
								display_message(ERROR_MESSAGE,"Unknown slider %s",slider_name);
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"list_node_group_slider.  Missing node_group_slider_dialog");
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"list_node_group_slider.  Invalid dialog");
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"list_node_group_slider.  Invalid slider widget");
					return_code=0;
				}
			}
			else
			{
				display_message(INFORMATION_MESSAGE," SLIDER_NAME");
				return_code=1;
			}
		}
		else
		{
			if (0<number_of_children)
			{
				XtVaGetValues(*child_list,
					XmNuserData,&node_group_slider_dialog,
					NULL);
				slider=node_group_slider_dialog->sliders;
				i=node_group_slider_dialog->number_of_sliders;
				while (i>0)
				{
					XtVaGetValues(*slider,
						XmNuserData,&node_group_slider,
						NULL);
					if (node_group_slider)
					{
						display_message(INFORMATION_MESSAGE,"%s.  min=%g, val=%g, max=%g\n",
							node_group_slider->name,node_group_slider->minimum,
							node_group_slider->value,node_group_slider->maximum);
					}
					i--;
					slider++;
				}
			}
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_node_group_slider.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* list_node_group_slider */
