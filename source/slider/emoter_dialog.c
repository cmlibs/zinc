/*******************************************************************************
FILE : emoter_dialog.c

LAST MODIFIED : 30 May 2001

DESCRIPTION :
This module creates a emoter_slider input device.  An emoter slider is
used to control modes shapes for the EMOTER update positions of a
group of nodes
==============================================================================*/
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <Xm/PushB.h>
#include <Xm/ScrollBar.h>
#include <Xm/Text.h>
#include <Xm/ToggleB.h>
#include "command/command.h"
	/*???DB.  For Execute_command */
#include "command/parser.h"
#include "finite_element/import_finite_element.h"
#include "general/debug.h"
#include "general/geometry.h"
#include "general/manager.h"
#include "graphics/graphics_library.h"
#include "graphics/graphics_window.h"
#include "graphics/movie_graphics.h"
#include "graphics/scene.h"
#include "graphics/scene_viewer.h"
#include "mirage/em_cmgui.h"
	/*???DB.  For EM analysis - move out of mirage ? */
#include "slider/emoter_dialog.h"
#include "slider/emoter_dialog.uidh"
#include "three_d_drawing/movie_extensions.h"
#include "user_interface/confirmation.h"
#include "user_interface/message.h"
#include "user_interface/gui_dialog_macros.h"
#include "curve/control_curve.h"
#include "curve/control_curve_editor_dialog.h"

/*
Module constants
----------------
*/
#define SLIDER_RESOLUTION (1000)
#define SOLID_BODY_MODES (6)

/*
Module variables
----------------
*/
#if defined (MOTIF)
static int emoter_dialog_hierarchy_open=0;
static MrmHierarchy emoter_dialog_hierarchy;
#endif /* defined (MOTIF) */

/*
Module types
------------
*/
struct Shared_emoter_slider_data
/*******************************************************************************
LAST MODIFIED : 6 September 1999

DESCRIPTION :
==============================================================================*/
{
	char *input_sequence;
	double *weights;
	int number_of_modes, number_of_sliders, mode_limit, show_solid_body_motion,
		movie_playing;
	float time;
	struct Colour viewer_background_colour;
	struct Emoter_slider *active_slider, **sliders;
	struct Execute_command *execute_command;
	struct Light *viewer_light;
	struct Light_model *viewer_light_model;
	struct MANAGER(FE_node) *node_manager;
	struct MANAGER(FE_element) *element_manager;
	struct MANAGER(FE_field) *fe_field_manager;
	struct MANAGER(Graphics_window) *graphics_window_manager;
	struct MANAGER(GROUP(FE_element)) *element_group_manager;
	struct MANAGER(GROUP(FE_node)) *node_group_manager;
	struct MANAGER(GROUP(FE_node)) *data_group_manager;
	struct MANAGER(Control_curve) *control_curve_manager;
	struct EM_Object *em_object;
	struct Scene *viewer_scene;
	struct Scene_object *transformation_scene_object;
	struct User_interface *user_interface;
	Widget top_level, *control_curve_editor_dialog_address;
}; /* struct Shared_emoter_slider_data */

struct Emoter_combine_slider
/*******************************************************************************
LAST MODIFIED : 10 April 1998

DESCRIPTION :
==============================================================================*/
{
	float combine_time;
	struct Emoter_slider *slider;
	struct Control_curve *curve, *timebase_curve;
}; /* struct Emoter_combine_slider */

struct Emoter_marker
/*******************************************************************************
LAST MODIFIED : 16 April 1998

DESCRIPTION :
==============================================================================*/
{
	char *name;
	int index, element_index, node_index;
	float value;
	struct Emoter_slider *slider_parent;
	struct Shared_emoter_slider_data *shared;
	Widget widget, value_text;
}; /* struct Emoter_marker */

struct Emoter_slider
/*******************************************************************************
LAST MODIFIED : 16 April 1998

DESCRIPTION :
==============================================================================*/
{
	char *name, *sequence_filename;
	float maximum,minimum, value;
	int index, slider_position, selected,
		private_update_face_flag, number_of_combine_sliders,
		animated, number_of_emoter_markers,
		number_of_timebase_curves, solid_body_motion;
	struct Emoter_combine_slider **combine_sliders;
	struct Emoter_marker **emoter_markers;
	struct Shared_emoter_slider_data *shared;
	struct Scene_viewer *scene_viewer;
	struct Control_curve *mode_curve, **timebase_curves;
	Widget slider, widget, animated_pixmap,
		toggle_button, marker_rowcol,
		value_text, select_button;
}; /* struct Emoter_slider */

struct Emoter_dialog
/*******************************************************************************
LAST MODIFIED : 6 April 1998

DESCRIPTION :
==============================================================================*/
{
	int play_slider_position, movie_loop, movie_every_frame;
	float time_minimum, time_maximum;
	struct Shared_emoter_slider_data *shared;
	void *control_curve_manager_callback_id;
	Widget *dialog_address, shell, widget,
		play_slider, slider_form, play_value_text,
		play_max_text, play_min_text, mode_subform,
		mode_text, solid_motion_button, movie_control_form,
		play_button, movie_framerate_text, input_sequence_button;
	struct GROUP(FE_node) *minimum_nodegroup;
	struct GROUP(FE_element) *minimum_elementgroup;
	struct Shell_list_item *shell_list_item;
	struct Movie_graphics *movie;
}; /* struct Emoter_dialog */

struct Update_node_data
/*******************************************************************************
LAST MODIFIED : 2 June 1997

DESCRIPTION :
==============================================================================*/
{
	float new_slider_value,old_slider_value;
	struct MANAGER(FE_node) *node_manager;
	struct Emoter_slider *slider;
}; /* struct Update_node_data */

/*
Module functions
----------------
*/

struct Emoter_slider *create_emoter_slider(char *sequence_filename,
	char *name, Widget parent, float value,
	struct Shared_emoter_slider_data *shared_data,
	int index, struct Control_curve *existing_mode_curve,
	struct Emoter_dialog *emoter_dialog, int no_confirm);
/*******************************************************************************
LAST MODIFIED : 9 September 1999

DESCRIPTION :
Both or either of <sequence_filename> or <existing_mode_curve> can be NULL.
Declared here because of circular recursive function calling.
==============================================================================*/


DECLARE_DIALOG_IDENTIFY_FUNCTION(emoter_dialog, \
	Emoter_dialog, slider_form)

DECLARE_DIALOG_IDENTIFY_FUNCTION(emoter_dialog, \
	Emoter_dialog, play_value_text)

DECLARE_DIALOG_IDENTIFY_FUNCTION(emoter_dialog, \
	Emoter_dialog, play_max_text)

DECLARE_DIALOG_IDENTIFY_FUNCTION(emoter_dialog, \
	Emoter_dialog, play_min_text)

DECLARE_DIALOG_IDENTIFY_FUNCTION(emoter_dialog, \
	Emoter_dialog, play_button)

DECLARE_DIALOG_IDENTIFY_FUNCTION(emoter_dialog, \
	Emoter_dialog, input_sequence_button)

DECLARE_DIALOG_IDENTIFY_FUNCTION(emoter_dialog, \
	Emoter_dialog, mode_subform)

DECLARE_DIALOG_IDENTIFY_FUNCTION(emoter_dialog, \
	Emoter_dialog, mode_text)

DECLARE_DIALOG_IDENTIFY_FUNCTION(emoter_dialog, \
	Emoter_dialog, solid_motion_button)

DECLARE_DIALOG_IDENTIFY_FUNCTION(emoter_dialog, \
	Emoter_dialog, movie_control_form)

DECLARE_DIALOG_IDENTIFY_FUNCTION(emoter_dialog, \
	Emoter_dialog, movie_framerate_text)

DECLARE_DIALOG_IDENTIFY_FUNCTION(emoter_slider, \
	Emoter_slider, toggle_button)

DECLARE_DIALOG_IDENTIFY_FUNCTION(emoter_slider, \
	Emoter_slider, select_button)

DECLARE_DIALOG_IDENTIFY_FUNCTION(emoter_slider, \
	Emoter_slider, value_text)

DECLARE_DIALOG_IDENTIFY_FUNCTION(emoter_slider, \
	Emoter_slider, animated_pixmap)

DECLARE_DIALOG_IDENTIFY_FUNCTION(emoter_slider, \
	Emoter_slider, marker_rowcol)

DECLARE_DIALOG_IDENTIFY_FUNCTION(emoter_marker, \
	Emoter_marker, value_text)

static int emoter_update_nodes(struct Shared_emoter_slider_data *shared_data,
	int solid_body_motion )
/*******************************************************************************
LAST MODIFIED : 6 September 1998

DESCRIPTION :
Updates the node locations for the <emoter_slider>
==============================================================================*/
{
	char input_filename[200];
	double position[3], temp_two, temp_cos, temp_sin, *u,*weights;
	FILE *input_file;
	float euler_angles[3];
	gtMatrix transformation; /* 4 x 4 */
	int i,j,k,offset,return_code,versions;
	struct FE_field_component coordinate_field_component;
	struct FE_node *node,*temp_node;
	struct EM_Object *em_object;

	ENTER(emoter_update_nodes);
	if (shared_data)
	{
		return_code=1;
		em_object=shared_data->em_object;
		if (0<em_object->n_nodes)
		{
			if ((node=FIND_BY_IDENTIFIER_IN_MANAGER(FE_node,cm_node_identifier)(
				(em_object->index)[0],shared_data->node_manager))&&
				(coordinate_field_component.field=get_FE_node_default_coordinate_field(
				node)))
			{
				if (temp_node=CREATE(FE_node)(0,(struct FE_node *)NULL))
				{
					/* Read from an input sequence which the emoter is overriding */
					if (shared_data->input_sequence)
					{
						sprintf(input_filename,shared_data->input_sequence,
							shared_data->time);
						if (input_file = fopen(input_filename, "r"))
						{
							read_FE_node_group(input_file,shared_data->fe_field_manager,
								(struct FE_time *)NULL,
								shared_data->node_manager,
								shared_data->element_manager,
								shared_data->node_group_manager,
								shared_data->data_group_manager,
								shared_data->element_group_manager);
							fclose(input_file);
						}
						else
						{
							display_message(WARNING_MESSAGE,
				"emoter_update_nodes.  Unable to import node file %s", input_filename);
							return_code=0;							
						}
					}

					MANAGER_BEGIN_CACHE(FE_node)(shared_data->node_manager);

					/* perform EM reconstruction */
					i=0;
					offset=em_object->m;
					while (return_code&&(i<em_object->n_nodes))
					{
						if (node=FIND_BY_IDENTIFIER_IN_MANAGER(FE_node,cm_node_identifier)(
							(em_object->index)[i],shared_data->node_manager))
						{
							if (COPY(FE_node)(temp_node,node))
							{
								position[0] = 0;
 								position[1] = 0;
 								position[2] = 0;
								u=(em_object->u)+(3*i);
								weights=shared_data->weights + SOLID_BODY_MODES;
								for (k=shared_data->mode_limit;k>0;k--)
								{
									position[0] += u[0]*(*weights);
									position[1] += u[1]*(*weights);
									position[2] += u[2]*(*weights);
									u += offset;
									weights++;
								}

								if ( solid_body_motion )
								{
									if (shared_data->transformation_scene_object)
									{
										euler_angles[0] = shared_data->weights[5];
										euler_angles[1] = shared_data->weights[4];
										euler_angles[2] = shared_data->weights[3];
										euler_to_gtMatrix(euler_angles, transformation);
										transformation[3][0] = shared_data->weights[0];
										transformation[3][1] = shared_data->weights[1];
										transformation[3][2] = shared_data->weights[2];
										Scene_object_set_transformation(
											shared_data->transformation_scene_object,
											&transformation);
									}
									else
									{
										/* Need to apply rotations in reverse order */
										weights = shared_data->weights + 5;
										temp_sin = sin( -*weights );
										temp_cos = cos( -*weights );
										temp_two = temp_cos * position[0] - temp_sin * position[1];
										position[1] = temp_sin * position[0] + temp_cos * position[1];
										position[0] = temp_two;
										weights--;
										
										temp_sin = sin( -*weights );
										temp_cos = cos( -*weights );
										temp_two = temp_cos * position[2] - temp_sin * position[0];
										position[0] = temp_sin * position[2] + temp_cos * position[0];
										position[2] = temp_two;
										weights--;
										
										temp_sin = sin( -*weights );
										temp_cos = cos( -*weights );
										temp_two = temp_cos * position[1] - temp_sin * position[2];
										position[2] = temp_sin * position[1] + temp_cos * position[2];
										position[1] = temp_two;
										weights--;
										
										position[2] -= *weights;
										weights--;
										position[1] -= *weights;
										weights--;
										position[0] -= *weights;
									}
								}

								for (k = 0 ; k < 3 ; k++)
								{
									coordinate_field_component.number = k;
									versions = get_FE_node_field_component_number_of_versions(
										temp_node, coordinate_field_component.field, k);
									for (j = 0 ; j < versions ; j++)
									{
										return_code=set_FE_nodal_FE_value_value(temp_node,
											&coordinate_field_component, j, FE_NODAL_VALUE,
											/*time*/0, (FE_value)position[k]);
									}
								}

								if (return_code)
								{
									return_code=MANAGER_MODIFY_NOT_IDENTIFIER(FE_node,
										cm_node_identifier)(node,temp_node,
										shared_data->node_manager);
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"emoter_update_nodes.  Could not copy node");
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"emoter_update_nodes.  Unknown node %d",
								(em_object->index)[i]);
							return_code=0;
						}
						i++;
					}
					MANAGER_END_CACHE(FE_node)(shared_data->node_manager);
					DESTROY(FE_node)(&temp_node);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"emoter_update_nodes.  Could not create temp_node");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"emoter_update_nodes.  Could not find coordinate_field");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"emoter_update_nodes.  Missing shared_data");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* emoter_update_nodes */

static int emoter_set_mode_value(int mode_number, float new_value,
	struct Shared_emoter_slider_data *shared_data)
/*******************************************************************************
LAST MODIFIED : 22 April 1998

DESCRIPTION :
Updates the node locations for the <emoter_slider>
==============================================================================*/
{
	int return_code;

	ENTER(emoter_set_mode_value);
	if (shared_data)
	{
		(shared_data->weights)[mode_number]=new_value;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"emoter_set_mode_value.  Missing shared_data");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* emoter_set_mode_value */

static void emoter_add_slider_modes( struct Emoter_slider *slider,
	float *total_shape_vector, struct Shared_emoter_slider_data *shared,
	float time)
/*******************************************************************************
LAST MODIFIED : 17 November 1999

DESCRIPTION :
==============================================================================*/
{
	int i;
	float data_start_frame, data_end_frame, marker_start_time, marker_end_time, 
		*shape_vector, slider_time;

	ENTER(emoter_add_slider_modes);

	slider->private_update_face_flag = 1;
	if ( slider->value != 0.0 && slider->mode_curve )
	{
		marker_start_time = slider->emoter_markers[0]->value;
		marker_end_time = 
			slider->emoter_markers[slider->number_of_emoter_markers-1]->value;
		Control_curve_get_parameter_range( slider->mode_curve, &data_start_frame,
			&data_end_frame );
		if ( marker_end_time != marker_start_time )
		{
			slider_time = data_start_frame + (data_end_frame - data_start_frame)
				* (time - marker_start_time) / (marker_end_time - marker_start_time);
		}
		else
		{
			slider_time = data_start_frame;
		}
		if (ALLOCATE(shape_vector,FE_value,
			Control_curve_get_number_of_components(slider->mode_curve)))
		{
			if (Control_curve_get_values_at_parameter( slider->mode_curve, 
				slider_time, shape_vector, (FE_value *)NULL))
			{
				if ( slider->solid_body_motion )
				{
					shape_vector[slider->solid_body_motion] -= EM_standard_mode_one( shared->em_object );
					for ( i = 0 ; i < slider->solid_body_motion ; i++ )
					{
						total_shape_vector[i] += shape_vector[i] * slider->value;
					}
					for ( ; i < slider->solid_body_motion + shared->mode_limit ; i++ )
					{
						total_shape_vector[i] += shape_vector[i] * slider->value;
					}
				}
				else
				{
					shape_vector[0] -= EM_standard_mode_one( shared->em_object );
					for ( i = 0 ; i < shared->mode_limit ; i++ )
					{
						total_shape_vector[i + SOLID_BODY_MODES] += shape_vector[i] * slider->value;
					}
				}
			}
			DEALLOCATE( shape_vector );
		}
	}

	LEAVE;

} /* emoter_add_slider_modes */

static float *emoter_sum_all_modes( struct Shared_emoter_slider_data *shared)
/*******************************************************************************
LAST MODIFIED : 18 April 1998

DESCRIPTION :
Evaluates the modes of all the sliders without updating the nodes.
Returns the vector of mode shapes.
==============================================================================*/
{
	int i, j;
	float *total_shape_vector;
	struct Emoter_slider *slider;

	ENTER(emoter_sum_all_modes);

#if defined (DEBUG)
	printf("emoter_sum_all_modes\n");
#endif /* defined (DEBUG) */


	/* First reset all the flags (so no slider is added in twice)
		The flag is set for a slider when emoter_add_slider_modes is
		called */
	for ( j = 0 ; j < shared->number_of_sliders ; j++ )
	{
		(shared->sliders[j])->private_update_face_flag = 0;
	}

	if ( ALLOCATE( total_shape_vector, float, SOLID_BODY_MODES + shared->number_of_modes ))
	{
		for ( i = 0 ; i < SOLID_BODY_MODES + shared->mode_limit ; i++ )
		{
			total_shape_vector[i] = 0;
		}
		total_shape_vector[SOLID_BODY_MODES] += EM_standard_mode_one( shared->em_object );

		/* Add in the active slider */
		if ( shared->active_slider )
		{
			emoter_add_slider_modes ( shared->active_slider,
				total_shape_vector, shared, shared->time );

			/* Add in the active slider's combine_sliders */
			for ( j = 0 ; j < shared->active_slider->number_of_combine_sliders ; j++ )
			{
				slider = (shared->active_slider->combine_sliders[j])->slider;

				if ( !slider->private_update_face_flag )
				{
					emoter_add_slider_modes( slider, total_shape_vector, shared,
						(shared->active_slider->combine_sliders[j])->combine_time );
				}
			}
		}

		/* Add in any other selected sliders */
		for ( j = 0 ; j < shared->number_of_sliders ; j++ )
		{
			slider = shared->sliders[j];

			if ( slider->selected && !slider->private_update_face_flag )
			{
				emoter_add_slider_modes( slider, total_shape_vector, shared, shared->time );
			}
		}

	}
	else
	{
		display_message(ERROR_MESSAGE,
			"emoter_sum_all_modes.  Unable to allocate total shape vector");
		total_shape_vector = (float *)NULL;
	}

	LEAVE;

	return( total_shape_vector );
} /* emoter_sum_all_modes */

static void emoter_update_face( struct Shared_emoter_slider_data *shared )
/*******************************************************************************
LAST MODIFIED : 18 April 1998

DESCRIPTION :
Sets the nodes according to all the values of the sliders
==============================================================================*/
{
	int i;
	float *total_shape_vector;

	ENTER(emoter_update_face);

#if defined (DEBUG)
	printf("emoter_update_face\n");
#endif /* defined (DEBUG) */

	if ( !shared->movie_playing )
	{
		if ( total_shape_vector = emoter_sum_all_modes( shared ))
		{
			for ( i = 0; i < shared->mode_limit + SOLID_BODY_MODES ; i++ )
			{
				emoter_set_mode_value ( i, total_shape_vector[i], shared );
			}

			emoter_update_nodes( shared, shared->show_solid_body_motion );
			/*			(*(emoter_dialog->shared->execute_command->function))(
						"open comfile redraw execute",
						emoter_dialog->shared->execute_command->data);*/
			DEALLOCATE( total_shape_vector );
		}
	}
	LEAVE;
} /* emoter_update_face */

static int emoter_slider_select(struct Emoter_slider *slider, int state)
/*******************************************************************************
LAST MODIFIED : 7 April 1998

DESCRIPTION :
Sets the select state of this <slider> to the <state>
==============================================================================*/
{
	int return_code;

	ENTER(emoter_slider_select);

	if ( slider )
	{
		slider->selected = state;
		if ( slider->select_button )
		{
			if ( state )
			{
				XmToggleButtonSetState( slider->select_button,
					True, False );
			}
			else
			{
				if ( slider != slider->shared->active_slider )
				{
					XmToggleButtonSetState( slider->select_button,
						False, False );
				}
				else
				{
					/* If slider is active it must be selected */
					slider->selected = 1;
					XmToggleButtonSetState( slider->select_button,
						True, False );
				}

			}
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"emoter_slider_select.  Invalid slider");
		return_code = 0;
	}


	LEAVE;
	return ( return_code );
} /* emoter_slider_select */

static int emoter_slider_animated(struct Emoter_slider *slider, int state)
/*******************************************************************************
LAST MODIFIED : 7 April 1998

DESCRIPTION :
Sets the animated state of this <slider> to the <state>
==============================================================================*/
{
	int return_code;

	ENTER(emoter_slider_animated);

	if ( slider )
	{
		if ( slider->animated != state )
		{
			slider->animated = state;
			if ( slider->animated_pixmap )
			{
				if ( state )
				{
					Scene_viewer_set_input_mode( slider->scene_viewer,
						SCENE_VIEWER_NO_INPUT );
					Scene_viewer_redraw(slider->scene_viewer);
				}
				else
				{
					Scene_viewer_set_input_mode( slider->scene_viewer,
						SCENE_VIEWER_NO_INPUT_OR_DRAW );
					Scene_viewer_redraw(slider->scene_viewer);
				}
			}
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"emoter_slider_animated.  Invalid slider");
		return_code = 0;
	}

	LEAVE;
	return ( return_code );
} /* emoter_slider_animated */

static void emoter_set_slider_value( struct Emoter_slider *emoter_slider,
	float new_value )
/*******************************************************************************
LAST MODIFIED : 9 April 1998

DESCRIPTION :
Sets the emoter slider according to the <new_value> given.
Updates all the basis modes from the active
emoter slider's Control_curve
==============================================================================*/
{
	char value_text_string[30];
	int slider_position;

	ENTER(emoter_set_slider_value);
	if ( emoter_slider && emoter_slider->shared )
	{
		emoter_slider->value = new_value;

		/* Check slider position */
		slider_position =
			(int)((float)SLIDER_RESOLUTION*((emoter_slider->value)-
			(emoter_slider->minimum))/((emoter_slider->maximum)-
			(emoter_slider->minimum)));
		if ( slider_position != emoter_slider->slider_position &&
			emoter_slider->slider )
		{
			emoter_slider->slider_position = slider_position;
			XtVaSetValues(emoter_slider->slider,
				XmNvalue,slider_position,
				NULL);
		}

		/* Update the text box */
		if ( emoter_slider->value_text )
		{
			sprintf(value_text_string, "%6.2f", new_value );
			XmTextSetString(emoter_slider->value_text, value_text_string);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
		"emoter_set_slider_value.  Invalid arguments");
	}
	LEAVE;
} /* emoter_set_slider_value */

static void emoter_show_marker_value( struct Emoter_marker *emoter_marker,
	float new_value )
/*******************************************************************************
LAST MODIFIED : 17 April 1998

DESCRIPTION :
Shows the <new_value> given in the emoter marker textbox without updating
any control curves.
==============================================================================*/
{
	char value_text_string[30];

	ENTER(emoter_show_marker_value);
	if ( emoter_marker )
	{
		/* Update the text box */
		if ( emoter_marker->value_text )
		{
			sprintf(value_text_string, "%10.1f", new_value );
			XmTextSetString(emoter_marker->value_text, value_text_string);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
		"emoter_show_marker_value.  Invalid arguments");
	}
	LEAVE;
} /* emoter_show_marker_value */

static void emoter_set_marker_value( struct Emoter_marker *emoter_marker,
	float new_value )
/*******************************************************************************
LAST MODIFIED : 17 April 1998

DESCRIPTION :
Sets the emoter marker according to the <new_value> given.
==============================================================================*/
{
	int i;
	struct Emoter_slider *active;

	ENTER(emoter_set_marker_value);
	if ( emoter_marker && emoter_marker->shared )
	{
		emoter_show_marker_value( emoter_marker, new_value );

		if ( active = emoter_marker->shared->active_slider)
		{
			if ( active == emoter_marker->slider_parent )
			{
				emoter_marker->value = new_value;
				/* Change the independent value in every timebase curve referring to this
					slider */
				for ( i = 0 ; i < active->number_of_timebase_curves ; i++ )
				{
					if ( 1 == (Control_curve_get_number_of_components
						(active->timebase_curves[i])) )
					{
						Control_curve_set_node_values(
							(active->timebase_curves[i]),
							emoter_marker->element_index, emoter_marker->node_index,
							&new_value );
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"emoter_set_marker_value.  Incorrect number of components in timebase curve");
					}
				}
			}
			else
			{
				/* Find this curve in the active slider and change its dependent value */
				for ( i = 0 ; i < active->number_of_combine_sliders ; i++ )
				{
					if ( emoter_marker->slider_parent == (active->combine_sliders[i]->slider))
					{
						Control_curve_set_parameter(
							((active->combine_sliders[i])->timebase_curve),
							emoter_marker->element_index, emoter_marker->node_index,
							new_value );
					}
				}
			}

		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
		"emoter_set_marker_value.  Invalid arguments");
	}
	LEAVE;
} /* emoter_set_marker_value */

static void emoter_slider_CB(Widget widget,
	XtPointer emoter_slider_void, XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 6 April 1998

DESCRIPTION :
Emoter slider scrollbar callback.  Sets the value from the slider
==============================================================================*/
{
	int slider_position;
	float new_value;
	struct Emoter_slider *emoter_slider;

	ENTER(update_emoter_slider_callback);
	USE_PARAMETER(call_data);
	if (emoter_slider = (struct Emoter_slider *)emoter_slider_void)
	{
		XtVaGetValues(widget, XmNvalue, &slider_position, NULL);

		emoter_slider_select( emoter_slider, 1 );

		if ( emoter_slider->slider_position != slider_position )
		{
			emoter_slider->slider_position = slider_position;
			new_value=
				((emoter_slider->minimum)*
				(float)(SLIDER_RESOLUTION-slider_position)+
				(emoter_slider->maximum)*
				(float)(slider_position))/
				(float)SLIDER_RESOLUTION;
			emoter_set_slider_value( emoter_slider, new_value );
			emoter_update_face( emoter_slider->shared );
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
		"update_emoter_slider_callback.  Missing emoter_slider structure");
	}
	LEAVE;
} /* update_emoter_slider_callback */

static void destroy_emoter_marker_callback(Widget widget,
	XtPointer emoter_marker_void, XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 6 April 1998

DESCRIPTION :
Destroy Emoter marker scrollbar callback.  Deallocates the structure for the
marker
==============================================================================*/
{
	struct Emoter_marker *emoter_marker;

	ENTER(destroy_emoter_marker_callback);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (emoter_marker = (struct Emoter_marker *)emoter_marker_void)
	{
		if ( emoter_marker->name )
		{
			DEALLOCATE( emoter_marker->name );
		}
		DEALLOCATE( emoter_marker );
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"destroy_emoter_marker_callback.  Missing emoter_marker structure");
	}
	LEAVE;
} /* destroy_emoter_marker_callback */

static void destroy_emoter_slider_callback(Widget widget,
	XtPointer emoter_slider_void, XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 6 April 1998

DESCRIPTION :
Destroy Emoter slider scrollbar callback.  Deallocates the structure for the
slider
==============================================================================*/
{
	int i, j;
	struct Emoter_slider *emoter_slider, *combine_slider;
	struct Control_curve *temp_var;

	ENTER(destroy_emoter_slider_callback);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (emoter_slider = (struct Emoter_slider *)emoter_slider_void)
	{
		if ( emoter_slider->mode_curve )
		{
			temp_var = emoter_slider->mode_curve;
			DEACCESS(Control_curve)(&(emoter_slider->mode_curve));
			REMOVE_OBJECT_FROM_MANAGER(Control_curve)
				(temp_var, emoter_slider->shared->control_curve_manager);
		}
		for ( j = 0 ; j < emoter_slider->number_of_combine_sliders ; j++ )
		{
			temp_var = (emoter_slider->combine_sliders[j])->curve;
			DEACCESS(Control_curve)( &((emoter_slider->combine_sliders[j])->curve) );
			REMOVE_OBJECT_FROM_MANAGER(Control_curve)
				(temp_var, emoter_slider->shared->control_curve_manager);

			/* To complete this destruction the reference to the Control_curve
				in the combine slider also needs to be removed */
			combine_slider = (emoter_slider->combine_sliders[j])->slider;
			if ( temp_var = (emoter_slider->combine_sliders[j])->timebase_curve)
			{
				i = 0;
				while ( i < combine_slider->number_of_timebase_curves
					&& temp_var != combine_slider->timebase_curves[i])
				{
					i++;
				}
				if ( i < combine_slider->number_of_timebase_curves )
				{
					combine_slider->number_of_timebase_curves--;
					while ( i < combine_slider->number_of_timebase_curves )
					{
						combine_slider->timebase_curves[i] =
							combine_slider->timebase_curves[i + 1];
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"destroy_emoter_slider_callback.  Unable to find timebase curve reference in combine curve");
				}

				if ( (emoter_slider->combine_sliders[j])->timebase_curve )
				{
					temp_var = (emoter_slider->combine_sliders[j])->timebase_curve;
					DEACCESS(Control_curve)( &((emoter_slider->combine_sliders[j])->timebase_curve) );
					REMOVE_OBJECT_FROM_MANAGER(Control_curve)
						(temp_var, emoter_slider->shared->control_curve_manager);
				}
			}

			DEALLOCATE( emoter_slider->combine_sliders[j] );
		}
		DEALLOCATE( emoter_slider->combine_sliders );
		DEALLOCATE( emoter_slider->emoter_markers );
		DEALLOCATE( emoter_slider->timebase_curves );

		if ( emoter_slider->sequence_filename )
		{
			DEALLOCATE(  emoter_slider->sequence_filename );
		}
		DEALLOCATE( emoter_slider->name );
		DEALLOCATE( emoter_slider );
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"destroy_emoter_slider_callback.  Missing emoter_slider structure");
	}
	LEAVE;
} /* destroy_emoter_slider_callback */

static void emoter_dialog_destroy_CB(Widget w,XtPointer user_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 4 June 1999

DESCRIPTION :
Callback for the emoter dialog - tidies up all details - mem etc
==============================================================================*/
{
	struct Emoter_dialog *emoter_dialog;
	struct GROUP(FE_element) *temp_fe_element_group;
	struct GROUP(FE_node) *temp_fe_node_group;

	ENTER(emoter_dialog_destroy_CB);
	USE_PARAMETER(user_data);
	USE_PARAMETER(call_data);
	/* Get the pointer to the emoter_dialog structure */
	XtVaGetValues(w,XmNuserData,&emoter_dialog,NULL);

	if ( emoter_dialog )
	{
		/* The sliders DEALLOCATE their own data */

		if ( emoter_dialog->control_curve_manager_callback_id )
		{
			MANAGER_DEREGISTER(Control_curve)(
				emoter_dialog->control_curve_manager_callback_id,
				emoter_dialog->shared->control_curve_manager);
			emoter_dialog->control_curve_manager_callback_id=(void *)NULL;
		}

		/* Destroy shared slider data */
		DEALLOCATE(emoter_dialog->shared->weights);
		DEALLOCATE(emoter_dialog->shared->sliders);
		destroy_EM_Object(&(emoter_dialog->shared->em_object));

		if( emoter_dialog->minimum_elementgroup )
		{
			temp_fe_element_group = emoter_dialog->minimum_elementgroup;
			DEACCESS(GROUP(FE_element))(&(emoter_dialog->minimum_elementgroup));
			REMOVE_OBJECT_FROM_MANAGER(GROUP(FE_element))(temp_fe_element_group,
				emoter_dialog->shared->element_group_manager);
		}
		if ( emoter_dialog->minimum_nodegroup )
		{
			temp_fe_node_group = emoter_dialog->minimum_nodegroup;
			DEACCESS(GROUP(FE_node))(&(emoter_dialog->minimum_nodegroup));
			REMOVE_OBJECT_FROM_MANAGER(GROUP(FE_node))(temp_fe_node_group,
				emoter_dialog->shared->node_group_manager);
		}

		destroy_Shell_list_item(&(emoter_dialog->shell_list_item));

		DEALLOCATE(emoter_dialog->shared);

		*(emoter_dialog->dialog_address) = (Widget)NULL;
		/* deallocate the memory for the user data */
		DEALLOCATE(emoter_dialog);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_dialog_destroy_CB.  Could not get emoter_dialog");
	}

	LEAVE;
} /* emoter_dialog_destroy_CB */

static void emoter_id_play_slider(Widget widget,XtPointer user_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 14 April 1998

DESCRIPTION :
Callback for the play slider which sets the min and max and stores the widget
==============================================================================*/
{
	struct Emoter_dialog *emoter_dialog;

	ENTER(emoter_id_play_slider);
	USE_PARAMETER(user_data);
	USE_PARAMETER(call_data);
	/* Get the pointer to the emoter_dialog structure */
	XtVaGetValues(widget,
		XmNuserData, &emoter_dialog,
		NULL);

	if ( emoter_dialog )
	{
		emoter_dialog->play_slider = widget;
		XtVaSetValues(widget,
			XmNminimum, (int)emoter_dialog->time_minimum,
			XmNmaximum, (int)(emoter_dialog->time_maximum + 1),
			XmNsliderSize, 1,
			NULL);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_id_play_slider.  Could not get emoter_dialog");
	}

	LEAVE;
} /* emoter_id_play_slider */

static void emoter_update_combine_sliders(struct Emoter_slider *slider)
/*******************************************************************************
LAST MODIFIED : 4 November 1998

DESCRIPTION :
Callback for the play slider which sets the min and max and stores the widget
==============================================================================*/
{
	int i;
	float *combine_vector;
	struct Emoter_slider *combine_slider;
	struct Control_curve *timebase_curve,*curve;

	ENTER(emoter_update_combine_sliders);

	if ( slider )
	{
		if ( slider->number_of_combine_sliders )
		{
			for ( i = 0 ; i < slider->number_of_combine_sliders ; i++ )
			{
				if ((curve=(slider->combine_sliders[i])->curve) &&
					(combine_slider = (slider->combine_sliders[i])->slider))
				{
					if (ALLOCATE(combine_vector,FE_value,
						Control_curve_get_number_of_components(curve)))
					{
						if (Control_curve_get_values_at_parameter(curve,
							slider->shared->time,	combine_vector, (FE_value *)NULL))
						{
							emoter_set_slider_value( combine_slider, combine_vector[0] );
							emoter_slider_select( combine_slider, 0 );
						}
						DEALLOCATE ( combine_vector );
					}
					(slider->combine_sliders[i])->combine_time = slider->shared->time;
					if (timebase_curve=(slider->combine_sliders[i])->timebase_curve)
					{
						if (ALLOCATE(combine_vector,FE_value,
							Control_curve_get_number_of_components(timebase_curve)))
						{
							if (Control_curve_get_values_at_parameter(timebase_curve,
								slider->shared->time,	combine_vector, (FE_value *)NULL))
							{
								(slider->combine_sliders[i])->combine_time = combine_vector[0];
							}
							DEALLOCATE( combine_vector );
						}
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"emoter_update_combine_sliders.  Unable to set combine slider");
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_update_combine_sliders.  Could not get emoter_dialog");
	}

	LEAVE;
} /* emoter_update_combine_sliders */

static void emoter_update_markers(struct Emoter_slider *slider)
/*******************************************************************************
LAST MODIFIED : 17 April 1998

DESCRIPTION :
Callback for the play slider which sets the min and max and stores the widget
==============================================================================*/
{
	int i, j;
	float value;
	struct Emoter_slider *combine_slider;
	struct Control_curve *timebase_curve;

	ENTER(emoter_update_markers);

	if ( slider )
	{
		/* Set the markers for the slider */
		for ( j = 0 ; j < slider->number_of_emoter_markers ; j++)
		{
			emoter_show_marker_value( slider->emoter_markers[j],
				(slider->emoter_markers[j])->value );
		}
		/* Set the markers combine sliders */
		for ( i = 0 ; i < slider->number_of_combine_sliders ; i++ )
		{
			combine_slider = (slider->combine_sliders[i])->slider;
			timebase_curve = (slider->combine_sliders[i])->timebase_curve;
			if ( timebase_curve &&
				(1 == Control_curve_get_number_of_components(
					timebase_curve)) &&
				(combine_slider->number_of_emoter_markers <=
					Control_curve_get_number_of_elements(
				timebase_curve) + 1))
			{
				for ( j = 0 ; j < combine_slider->number_of_emoter_markers - 1 ; j++)
				{
					if ( Control_curve_get_parameter(
						timebase_curve, j+1, 0, &value ))
					{
						emoter_show_marker_value( combine_slider->emoter_markers[j],
							value );
					}
				}
				if ( Control_curve_get_parameter(
					timebase_curve, j, 1, &value))
				{
					emoter_show_marker_value( combine_slider->emoter_markers[j],
						value );
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_update_markers.  Could not get emoter_dialog");
	}

	LEAVE;
} /* emoter_update_markers */

static void emoter_set_play_slider_value( struct Emoter_dialog *emoter_dialog,
	float new_value )
/*******************************************************************************
LAST MODIFIED : 8 April 1998

DESCRIPTION :
Sets the play slider according to the <new_value> given.
Updates all the basis modes from the active
emoter slider's Control_curve
==============================================================================*/
{
	char value_text_string[30];
	int slider_position;
	struct Emoter_slider *active;

	ENTER(emoter_set_play_slider_value);
	if ( emoter_dialog && emoter_dialog->shared )
	{
		emoter_dialog->shared->time = new_value;

		/* Check slider position */
		slider_position = (int)new_value;
		if ( slider_position != emoter_dialog->play_slider_position
			&& emoter_dialog->play_slider )
		{
			emoter_dialog->play_slider_position = slider_position;
			XtVaSetValues(emoter_dialog->play_slider,
				XmNvalue,slider_position,
				NULL);
		}

		/* Update the text box */
		if ( emoter_dialog->play_value_text )
		{
			sprintf(value_text_string, "%8.2f", new_value );
			XmTextSetString(emoter_dialog->play_value_text, value_text_string);
		}

		/* Update the 'combine' sliders from the active slider */
		if ( active = emoter_dialog->shared->active_slider )
		{
			emoter_update_combine_sliders(active);
		}

		/* Set the cursor bar in the curve editor if it exists */
		if ( *(emoter_dialog->shared->control_curve_editor_dialog_address) )
		{
			control_curve_editor_dialog_set_cursor_parameter(
				*(emoter_dialog->shared->control_curve_editor_dialog_address),
				new_value );
		}

	}
	else
	{
		display_message(ERROR_MESSAGE,
		"emoter_set_play_slider_value.  Invalid arguments");
	}
	LEAVE;
} /* emoter_set_play_slider_value */

static void emoter_set_play_slider_range( struct Emoter_dialog *emoter_dialog,
	float min, float max )
/*******************************************************************************
LAST MODIFIED : 15 April 1998

DESCRIPTION :
Sets the slider range of the play slider
==============================================================================*/
{
	char text_string[30];

	ENTER(emoter_set_play_slider_range);

	if ( emoter_dialog )
	{
		emoter_dialog->time_minimum = min;
		emoter_dialog->time_maximum = max;

		/* Update the text boxes */
		if ( emoter_dialog->play_min_text )
		{
			sprintf(text_string, "%8.2f", min );
			XmTextSetString(emoter_dialog->play_min_text, text_string);
		}
		if ( emoter_dialog->play_max_text )
		{
			sprintf(text_string, "%8.2f", max );
			XmTextSetString(emoter_dialog->play_max_text, text_string);
		}

		/* Set the range on the play slider */
		if ( emoter_dialog->play_slider )
		{
			XtVaSetValues( emoter_dialog->play_slider,
				XmNminimum, ((int)min),
				XmNmaximum, ((int)max + 1),
				XmNsliderSize, 1,
				NULL );
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
		"emoter_set_play_slider_range.  Invalid arguments");
	}
	LEAVE;
} /* emoter_set_play_slider_range */

static void emoter_play_slider_CB(Widget widget,XtPointer user_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 16 October 1998

DESCRIPTION :
Callback for the play slider which sets the min and max and stores the widget
==============================================================================*/
{
	int slider_position;
	struct Emoter_dialog *emoter_dialog;
	struct Graphics_window *graphics_window;

	ENTER(emoter_id_play_slider);
	USE_PARAMETER(user_data);
	USE_PARAMETER(call_data);
	/* Get the pointer to the emoter_dialog structure */
	XtVaGetValues(widget,
		XmNuserData, &emoter_dialog,
		NULL);

	if ( emoter_dialog )
	{
		XtVaGetValues(widget,
			XmNvalue, &slider_position,
			NULL);
		if ( slider_position != emoter_dialog->play_slider_position)
		{
			emoter_dialog->play_slider_position = slider_position;
			emoter_set_play_slider_value(emoter_dialog,
				slider_position);
			emoter_update_face( emoter_dialog->shared );
		}
		/* SAB This would be better implemented by adding a special
			manager message to the Scene Manager and then calling that. */
		if (graphics_window=FIRST_OBJECT_IN_MANAGER_THAT(Graphics_window)(
			(MANAGER_CONDITIONAL_FUNCTION(Graphics_window) *)NULL,
			(void *)NULL,emoter_dialog->shared->graphics_window_manager))
		{
			Graphics_window_update_now(graphics_window);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_play_slider_CB.  Could not get emoter_dialog");
	}

	LEAVE;
} /* emoter_play_slider_CB */

static void emoter_id_emoter_slider(Widget widget,XtPointer user_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 7 April 1998

DESCRIPTION :
Callback for the emoter slider which sets the min and max and stores the widget
==============================================================================*/
{
	struct Emoter_slider *emoter_slider;

	ENTER(emoter_id_emoter_slider);
	USE_PARAMETER(user_data);
	USE_PARAMETER(call_data);
	/* Get the pointer to the emoter_dialog structure */
	XtVaGetValues(widget,
		XmNuserData, &emoter_slider,
		NULL);

	if ( emoter_slider )
	{
		emoter_slider->slider = widget;
		XtVaSetValues(widget,
			XmNminimum, 0,
			XmNmaximum, SLIDER_RESOLUTION + 1,
			XmNsliderSize, 1,
			NULL);
		emoter_set_slider_value( emoter_slider, emoter_slider->value );
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"emoter_id_emoter_slider.  Could not get emoter_slider");
	}

	LEAVE;
} /* emoter_id_emoter_slider */

static void emoter_slider_select_activate(Widget widget,XtPointer user_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 7 April 1998

DESCRIPTION :
Callback for the emoter slider which sets the min and max and stores the widget
==============================================================================*/
{
	Boolean set;
	struct Emoter_slider *emoter_slider;

	ENTER(emoter_slider_select_activate);
	USE_PARAMETER(user_data);
	USE_PARAMETER(call_data);
	/* Get the pointer to the emoter_dialog structure */
	XtVaGetValues(widget,
		XmNuserData, &emoter_slider,
		XmNset, &set,
		NULL);

	if ( emoter_slider )
	{
		emoter_slider_select( emoter_slider, set );
		emoter_update_face( emoter_slider->shared );
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"emoter_slider_select_activate.  Could not get emoter_slider");
	}

	LEAVE;
} /* emoter_slider_select_activate */

static int emoter_slider_make_active(struct Emoter_slider *slider)
/*******************************************************************************
LAST MODIFIED : 7 April 1998

DESCRIPTION :
Sets the active slider to point to this <slider> and updates the
togglebuttons accordingly.
==============================================================================*/
{
	int j, return_code;
	struct Emoter_slider *active;

	ENTER(emoter_slider_make_active);
	if ( slider )
	{
		if ( (active = slider->shared->active_slider)
			&& active->toggle_button )
		{
			/* Have to make it not the active slider otherwise it
				cannot be deselected */
			slider->shared->active_slider = (struct Emoter_slider *)NULL;
			XtVaSetValues( active->toggle_button,
				XmNset, False,
				NULL );
			emoter_slider_select( active, 0 );
			/* Reset the animated icons for combine sliders */
			for ( j = 0 ; j < active->number_of_combine_sliders ; j++ )
			{
				emoter_slider_animated( active->combine_sliders[j]->slider, 0 );
			}
			emoter_slider_animated( active, 0 );
		}

		slider->shared->active_slider = slider;

		if ( slider->toggle_button )
		{
			XtVaSetValues( slider->toggle_button,
				XmNset, True,
				NULL );
			emoter_slider_select( slider, 1 );
			emoter_slider_animated( slider, 1 );
			emoter_set_slider_value( slider, 1.0 );
			emoter_update_combine_sliders( slider );
			emoter_update_markers(slider);
			/* Set the animated icons for combine sliders */
			for ( j = 0 ; j < slider->number_of_combine_sliders ; j++ )
			{
				emoter_slider_animated( slider->combine_sliders[j]->slider, 1 );
			}
			emoter_update_face( slider->shared );
			if ( slider->scene_viewer )
			{
				Scene_viewer_set_input_mode(slider->scene_viewer, SCENE_VIEWER_UPDATE_ON_CLICK );
				Scene_viewer_redraw(slider->scene_viewer);
			}
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"emoter_slider_make_active.  Togglebutton widget not set");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"emoter_slider_make_active.  Could not get emoter_slider");
		return_code = 0;
	}

	LEAVE;
	return ( return_code );
} /* emoter_slider_make_active */

static void emoter_slider_toggle_activate(Widget widget,XtPointer user_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 7 April 1998

DESCRIPTION :
Callback for the emoter slider which sets the min and max and stores the widget
==============================================================================*/
{
	Boolean set;
	struct Emoter_slider *emoter_slider;

	ENTER(emoter_slider_toggle_activate);
	USE_PARAMETER(user_data);
	USE_PARAMETER(call_data);
	/* Get the pointer to the emoter_dialog structure */
	XtVaGetValues(widget,
		XmNuserData, &emoter_slider,
		XmNset, &set,
		NULL);

	if ( emoter_slider )
	{
		emoter_slider_make_active( emoter_slider );
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"emoter_slider_toggle_activate.  Could not get emoter_slider");
	}

	LEAVE;
} /* emoter_slider_toggle_activate */

static void emoter_slider_value_text_CB(Widget widget,XtPointer user_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 7 April 1998

DESCRIPTION :
Callback for the emoter slider which responds to updates in the text box.
==============================================================================*/
{
	char *text_string;
	float value;
	struct Emoter_slider *emoter_slider;

	ENTER(emoter_slider_value_text_CB);
	USE_PARAMETER(user_data);
	USE_PARAMETER(call_data);
	/* Get the pointer to the emoter_dialog structure */
	XtVaGetValues(widget,
		XmNuserData, &emoter_slider,
		NULL);

	if ( emoter_slider )
	{
		emoter_slider_select( emoter_slider, 1 );

		if ( text_string = XmTextGetString(widget))
		{
			sscanf(text_string, "%f", &value );
			emoter_set_slider_value( emoter_slider, value );
			emoter_update_face( emoter_slider->shared );
			XtFree ( text_string );
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"emoter_slider_value_text_CB.  Could not get emoter_slider");
	}

	LEAVE;
} /* emoter_slider_value_text_CB */

static void emoter_marker_value_text_CB(Widget widget,XtPointer user_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 16 April 1998

DESCRIPTION :
Callback for the emoter marker which responds to updates in the text box.
==============================================================================*/
{
	char *text_string;
	float value;
	struct Emoter_marker *emoter_marker;

	ENTER(emoter_marker_value_text_CB);
	USE_PARAMETER(user_data);
	USE_PARAMETER(call_data);
	/* Get the pointer to the emoter_dialog structure */
	XtVaGetValues(widget,
		XmNuserData, &emoter_marker,
		NULL);

	if ( emoter_marker )
	{
		if ( text_string = XmTextGetString(widget))
		{
			sscanf(text_string, "%f", &value );
			emoter_set_marker_value( emoter_marker, value );
			emoter_update_face( emoter_marker->shared );
			XtFree ( text_string );
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"emoter_marker_value_text_CB.  Could not get emoter_marker");
	}

	LEAVE;
} /* emoter_marker_value_text_CB */

struct read_file_data
/*******************************************************************************
LAST MODIFIED : 7 April 1998

DESCRIPTION :
==============================================================================*/
{
	FILE *file;
	char current_token[300];
};

static int read_file_next_token (struct read_file_data *file_data)
/*******************************************************************************
LAST MODIFIED : 7 April 1998

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(read_file_float);

	fscanf(file_data->file, "%s", file_data->current_token );
	while ('#' == file_data->current_token[0])
	{
		while (!feof(file_data->file)
			&& 10 != fgetc( file_data->file ))
		{
		}
		fscanf(file_data->file, "%s", file_data->current_token );
	}

	return_code = 1;

	LEAVE;

	return (return_code);
} /* read_file_next_token */

static int read_file_float (struct read_file_data *file_data,
	float *data )
/*******************************************************************************
LAST MODIFIED : 7 April 1998

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(read_file_float);

	sscanf(file_data->current_token, "%f", data);
	read_file_next_token(file_data);

	return_code = 1;

	LEAVE;

	return (return_code);
} /* read_file_float */

static int read_file_int (struct read_file_data *file_data,
	int *data )
/*******************************************************************************
LAST MODIFIED : 7 April 1998

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(read_file_float);

	sscanf(file_data->current_token, "%d", data);
	read_file_next_token(file_data);

	return_code = 1;

	LEAVE;

	return (return_code);
} /* read_file_int */

static int read_file_string (struct read_file_data *file_data,
	char *data )
/*******************************************************************************
LAST MODIFIED : 17 April 1998

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(read_file_string);

	strcpy( data, file_data->current_token);
	read_file_next_token(file_data);

	return_code = 1;

	LEAVE;

	return (return_code);
} /* read_file_string */

static struct read_file_data *read_file_open (
	char *filename )
/*******************************************************************************
LAST MODIFIED : 7 April 1998

DESCRIPTION :
==============================================================================*/
{
	struct read_file_data *data;

	ENTER(read_file_open);

	if ( ALLOCATE(data, struct read_file_data, 1))
	{
		if ((data->file = fopen (filename, "r")))
		{
			read_file_next_token(data);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"read_file_data.  Unable to open file %s", filename);
			DEALLOCATE( data );
			data = (struct read_file_data *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"read_file_data.  Unable to allocate file data");
		data = (struct read_file_data *)NULL;
	}

	LEAVE;

	return (data);
} /* read_file_open */

static int read_file_close (struct read_file_data *file_data)
/*******************************************************************************
LAST MODIFIED : 7 April 1998

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(read_file_open);

	fclose(file_data->file);
	DEALLOCATE(file_data);

	return_code = 1;

	LEAVE;

	return (return_code);
} /* read_file_close */

static int read_file_eof(struct read_file_data *file_data)
/*******************************************************************************
LAST MODIFIED : 7 April 1998

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(read_file_eof);

	return_code = feof( file_data->file );

	LEAVE;

	return (return_code);
} /* read_file_eof */

static int read_file_eof_marker(struct read_file_data *file_data, char *marker)
/*******************************************************************************
LAST MODIFIED : 19 April 1998

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(read_file_eof_marker);

	if ( feof( file_data->file ))
	{
		return_code = 1;
	}
	else
	{
		if ( !strcmp( marker, file_data->current_token ))
		{
			read_file_next_token(file_data);
			return_code = 1;
		}
		else
		{
			return_code = 0;
		}
	}

	LEAVE;

	return (return_code);
} /* read_file_eof */

static int read_file_marker(struct read_file_data *file_data, char *marker)
/*******************************************************************************
LAST MODIFIED : 17 April 1998

DESCRIPTION :
Checks the current token for the specified marker.  If it is found then
the marker is incremented.
==============================================================================*/
{
	int return_code;

	ENTER(read_file_marker);

	if ( !strcmp( marker, file_data->current_token ))
	{
		read_file_next_token(file_data);
		return_code = 1;
	}
	else
	{
		return_code = 0;
	}

	LEAVE;

	return (return_code);
} /* read_file_marker */

static int read_weights( int file_modes_index, int file_modes,
	int solid_body_index, int solid_body_modes,
	struct read_file_data *file_data,
	int basis_modes, int total_values, float *weights )
/*******************************************************************************
LAST MODIFIED : 6 March 2000

DESCRIPTION :
Reads a weight vector from the <file>.
==============================================================================*/
{
	float weight;
	int j;

	ENTER(read_weights);

	for ( j = 0 ; j < total_values ; j++ )
	{
		read_file_float(file_data, &weight );
		if (solid_body_modes && (j >= solid_body_index) &&
			(j < solid_body_index + solid_body_modes))
		{
			weights[j - solid_body_index] = weight;
		}
		if (file_modes && (j >= file_modes_index) &&
			(j < file_modes_index + file_modes) && 
			(j < file_modes_index + basis_modes))
		{
			weights[j - file_modes_index + solid_body_modes] = weight;
		}
	}
	j = file_modes;
	while (j < basis_modes)
	{
		weights[j + solid_body_modes] = 0.0;
		j++;
	}
	LEAVE;

	return( 1 );
} /* read_weights */

#ifdef OLD_CODE
static int read_emoter_mode_Control_curve( struct Control_curve **emoter_curve_addr,
	struct MANAGER(Control_curve) *control_curve_manager,
	char *filename, struct Shared_emoter_slider_data *shared,
	float *number_of_frames)
/*******************************************************************************
LAST MODIFIED : 17 November 1999

DESCRIPTION :
Creates a control curve which is read from file values.
==============================================================================*/
{
	char warning[300], *name, *temp_filename;
	FE_value time;
	float *shape_vector, weight;
	int i, j, k, n_modes, return_code;
	struct read_file_data *file_data;
	struct Control_curve *emoter_curve;

	ENTER(read_emoter_mode_Control_curve);

	if ( emoter_curve_addr && control_curve_manager && filename )
	{
		if ( file_data = read_file_open( filename ))
		{
			if ( name = strrchr( filename, '/'))
			{
				name++;
			}
			else
			{
				name = filename;
			}
			/* This reads the marker if it is there and continues even
				if it isn't */
			read_file_marker( file_data, "MODE_DATA");
			read_file_int(file_data, &n_modes);
			if ( shared->number_of_modes == n_modes )
			{
				return_code = 1;
			}
			else
			{
				sprintf(warning, "The number of modes in file %s is %d which is not the \nsame as the basis file which has %d modes.  Do you want to load this file anyway?",
					filename, n_modes, shared->number_of_modes );
				return_code = confirmation_warning_ok_cancel("Emotionator warning",
					warning, shared->application_shell,
					shared->user_interface );
			}
			if ( return_code )
			{
				if ( ALLOCATE( temp_filename, char, strlen(name)+1))
				{
					strcpy(temp_filename, name);
					name = temp_filename;
					if ( ALLOCATE( shape_vector, float, shared->number_of_modes ))
					{
						while (FIND_BY_IDENTIFIER_IN_MANAGER(Control_curve,name)(
							name, control_curve_manager))
						{
							REALLOCATE(name, name, char, strlen(name)+2);
							strcat(name,"+");
						}
						if (emoter_curve = CREATE(Control_curve)(name,
							LINEAR_LAGRANGE, shared->number_of_modes ))
						{
							if (ADD_OBJECT_TO_MANAGER(Control_curve)(
								emoter_curve, control_curve_manager))
							{
								/* Set the type after the object is added to the
									manager so that ADD message is ignored */
								Control_curve_set_type(emoter_curve,
									CONTROL_CURVE_TYPE_EMOTER_MODES );
								ACCESS(Control_curve)(emoter_curve);
								/* SAB Do the first and second modes as special case
									as we need to create the first element, position both
									nodes and then from then on the current elements can
									just be appended to */
								Control_curve_add_element( emoter_curve, 1 );
								time = 1.0;
								Control_curve_set_parameter( emoter_curve,/*element_no*/1,
									/*local_node_no*/0,time);
								time += 1.0;
								Control_curve_set_parameter( emoter_curve,/*element_no*/1,
									/*local_node_no*/1,time);
								read_weights( n_modes, file_data, shared->number_of_modes, shape_vector );
								Control_curve_set_node_values( emoter_curve,
									1, 0, shape_vector);
								if ( !read_file_eof_marker(file_data, "END_MODE_DATA"))
								{
									read_weights( n_modes, file_data, shared->number_of_modes, shape_vector );
								}
								/* else do nothing and make a single constant element */
								Control_curve_set_node_values( emoter_curve,
									1, 1, shape_vector);

								i = 2;
								while (!read_file_eof_marker(file_data, "END_MODE_DATA"))
								{
									Control_curve_add_element( emoter_curve, i );
									time += 1.0;
									Control_curve_set_parameter( emoter_curve,/*element_no*/i,
										/*local_node_no*/1,time);
									read_weights( n_modes, file_data, shared->number_of_modes, shape_vector );
									Control_curve_set_node_values( emoter_curve,
										i, 1, shape_vector);
									i++;
								}
								*emoter_curve_addr = emoter_curve;
								*number_of_frames = i;
								return_code = 1;
							}
							else
							{
								DESTROY(Control_curve)(&emoter_curve);
								display_message(ERROR_MESSAGE,
									"read_emoter_mode_Control_curve.  Unable to add control curve to manager");
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"read_emoter_mode_Control_curve.  Unable to create control curve");
							return_code=0;
						}
						DEALLOCATE (shape_vector);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"read_emoter_mode_Control_curve.  Unable to allocate shape vector");
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"read_emoter_mode_Control_curve.  Unable to allocate name string");
					return_code=0;
				}
			}
			read_file_close( file_data );
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"read_emoter_mode_Control_curve.  Unable to open file %s", filename);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"read_emoter_mode_Control_curve.  Invalid argument(s)");
		return_code=0;
	}

	LEAVE;

	return (return_code);
} /* read_emoter_mode_Control_curve */
#endif /* OLD_CODE */

static struct Control_curve *read_emoter_control_curve( struct read_file_data *file_data,
	char *base_name, struct Shared_emoter_slider_data *shared)
/*******************************************************************************
LAST MODIFIED : 17 November 1999

DESCRIPTION :
Reads a control curve from a file.
==============================================================================*/
{
	char *name;
	enum FE_basis_type fe_basis_type;
	int j, local_node_no, nodes_per_element, elements, derivatives,
		return_code;
	float time, total_time, value;
	struct Control_curve *curve;

	ENTER(read_emoter_control_curve);

	curve = (struct Control_curve *)NULL;

	read_file_int(file_data, &nodes_per_element);
	read_file_int(file_data, &elements);
	read_file_int(file_data, &derivatives);
	return_code = 0;
	switch( nodes_per_element )
	{
		case 2:
		{
			switch ( derivatives )
			{
				case 0:
				{
					fe_basis_type = LINEAR_LAGRANGE;
					return_code = 1;
				} break;
				case 1:
				{
					fe_basis_type = CUBIC_HERMITE;
					return_code = 1;
				} break;
			}
		} break;
		case 3:
		{
			switch ( derivatives )
			{
				case 0:
				{
					fe_basis_type = QUADRATIC_LAGRANGE;
					return_code = 1;
				} break;
			}

		} break;
		case 4:
		{
			switch ( derivatives )
			{
				case 0:
				{
					fe_basis_type = CUBIC_LAGRANGE;
					return_code = 1;
				} break;
			}

		} break;
		default:
		{
			return_code = 0;
		} break;
	}

	if ( return_code )
	{
		if ( ALLOCATE( name, char, strlen(base_name)+1))
		{
			strcpy( name, base_name );
			while (FIND_BY_IDENTIFIER_IN_MANAGER(Control_curve,name)(
				name, shared->control_curve_manager))
			{
				REALLOCATE(name, name, char, strlen(name)+2);
				strcat(name,"+");
			}
			if ( curve = CREATE(Control_curve)(
				name, fe_basis_type, 1 ))
			{
				Control_curve_set_type(curve,
					CONTROL_CURVE_TYPE_EMOTER_COMBINE );
				if (ADD_OBJECT_TO_MANAGER(Control_curve)(
					curve, shared->control_curve_manager))
				{
					ACCESS(Control_curve)(curve);
					for ( j = 0 ; j < elements ; j ++ )
					{
						Control_curve_add_element( curve, j + 1 );
						if (0==j)
						{
							read_file_float( file_data, &total_time );
							Control_curve_set_parameter( curve,/*element_no*/1,
								/*local_node_no*/0,	total_time );
						}
						read_file_float( file_data, &time );
						total_time += time;
						Control_curve_set_parameter( curve,/*element_no*/j+1,
							/*local_node_no*/1,	total_time );
						local_node_no = 0;
						for (local_node_no = 0 ;
								local_node_no < nodes_per_element - 1
									; local_node_no++ )
						{
							read_file_float( file_data, &value );
							Control_curve_set_node_values ( curve,
								j + 1, local_node_no, &value );
							if ( derivatives )
							{
								read_file_float( file_data, &value );
								Control_curve_set_node_derivatives ( curve,
									j + 1, local_node_no, &value );
							}
							return_code = 1;
						}
					}
					read_file_float( file_data, &value );
					Control_curve_set_node_values ( curve,
						elements, nodes_per_element - 1, &value );
					if ( derivatives )
					{
						read_file_float( file_data, &value );
						Control_curve_set_node_derivatives ( curve,
							elements, local_node_no, &value );
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"emoter_keyframe_CB.  Unable to add combine curve to manager");
					DESTROY(Control_curve)(&curve);
					curve = (struct Control_curve *)NULL;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"emoter_keyframe_CB.  Could not create combine slider curve");
				curve = (struct Control_curve *)NULL;
			}
			DEALLOCATE( name );
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"emoter_keyframe_CB.  Could not allocate combine curve name");
			curve = (struct Control_curve *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"read_emoter_slider_file.  Unsupported combination of nodes per element and derivatives");
		curve = (struct Control_curve *)NULL;
	}

	LEAVE;

	return (curve);
} /* read_emoter_control_curve */

struct Emoter_marker *create_emoter_marker(char *name,
	float value, Widget parent, int index,
	int element_index, int node_index, int end_marker,
	struct Emoter_slider *slider_parent,
	struct Shared_emoter_slider_data *shared_data)
/*******************************************************************************
LAST MODIFIED : 16 April 1998

DESCRIPTION :
==============================================================================*/
{
	Arg override_arg[2];
	MrmType emoter_marker_class;
	static MrmRegisterArg callback_list[]=
	{
		{"emoter_marker_destroy_CB",
			(XtPointer)destroy_emoter_marker_callback},
		{"emoter_id_marker_value_text",(XtPointer)
			DIALOG_IDENTIFY(emoter_marker, value_text)},
		{"emoter_marker_value_text_CB",
			(XtPointer)emoter_marker_value_text_CB}
	};
	static MrmRegisterArg identifier_list[] =
	{
		{"emoter_marker_structure",(XtPointer)NULL},
		{"emoter_marker_name",(XtPointer)NULL}
	};
	struct Emoter_marker *emoter_marker;
	XmString label_string;

	ENTER(create_emoter_marker);
	/* check arguments */
	if (name && shared_data)
	{
		/* register the callbacks */
		if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
			emoter_dialog_hierarchy,callback_list,XtNumber(callback_list)))
		{
			if ( ALLOCATE ( emoter_marker, struct Emoter_marker, 1) &&
					ALLOCATE( emoter_marker->name, char, strlen(name) + 1))
			{
				strcpy( emoter_marker->name, name );
				emoter_marker->widget = (Widget)NULL;
				emoter_marker->shared = shared_data;
				emoter_marker->slider_parent = slider_parent;
				emoter_marker->value = value;
				emoter_marker->value_text = (Widget)NULL;
				emoter_marker->element_index = element_index;
				emoter_marker->node_index = node_index;
				emoter_marker->index = index;

				label_string=XmStringCreateSimple(name);
				identifier_list[0].value=(XtPointer)emoter_marker;
				identifier_list[1].value=(XtPointer)label_string;
				if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
					emoter_dialog_hierarchy,identifier_list,
					XtNumber(identifier_list)))
				{
					XtSetArg(override_arg[0],XmNuserData,emoter_marker);
					XtSetArg(override_arg[1],XmNpositionIndex,index);
					if (MrmSUCCESS==MrmFetchWidgetOverride(emoter_dialog_hierarchy,
						end_marker ? "emoter_marker_end" : "emoter_marker",
						parent,NULL,override_arg,2,
						&emoter_marker->widget, &emoter_marker_class))
					{
						XtManageChild(emoter_marker->widget);
						emoter_show_marker_value( emoter_marker, value );
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_emoter_marker.  Could not create emoter marker widgets");
						DEALLOCATE ( emoter_marker );
						emoter_marker=(struct Emoter_marker *)NULL;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_emoter_marker.  Could not register identifiers");
					DEALLOCATE(emoter_marker);
					emoter_marker=(struct Emoter_marker *)NULL;
				}
				XmStringFree(label_string);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_emoter_marker.  Could not allocate emoter marker structure");
				emoter_marker=(struct Emoter_marker *)NULL;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_emoter_dialog.  Could not register callbacks");
			emoter_marker=(struct Emoter_marker *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_emoter_marker.  Invalid argument(s)");
		emoter_marker=(struct Emoter_marker *)NULL;
	}
	LEAVE;

	return (emoter_marker);
} /* create_emoter_marker */

static int read_emoter_slider_file( struct Emoter_slider *slider,
	char *filename, float *start_time, float *end_time, int *marker_count,
	struct Emoter_dialog *emoter_dialog, void **icon_data,
	int *icon_width, int *icon_height, int no_confirm )
/*******************************************************************************
LAST MODIFIED : 17 November 1999

DESCRIPTION :
Reads stuff from a file.
==============================================================================*/
{
	char warning[300], *name, *temp_filename, temp_string[300], *char_data;
	float *shape_vector,total_time;
	int face_index, face_values, header, i, index, j, n_modes, return_code,
		solid_body_index, values;
	unsigned int integer_data;
	struct read_file_data *file_data;
	struct Emoter_combine_slider *combine_slider, **new_combine_sliders;
	struct Emoter_marker **new_markers;
	struct Emoter_slider *slider_to_combine;
	struct Shared_emoter_slider_data *shared;
	struct Control_curve *emoter_curve, *curve, **new_timebase_curves;

	ENTER(read_emoter_slider_file);

	if ( slider && filename )
	{
		shared = slider->shared;
		if ( file_data = read_file_open( filename ))
		{
			if ( name = strrchr( filename, '/'))
			{
				name++;
			}
			else
			{
				name = filename;
			}
			return_code = 1;
			while ( return_code && !read_file_eof(file_data))
			{
				if ( read_file_marker(file_data, "COMBINE"))
				{
					read_file_string(file_data, temp_string);

					return_code = 0;
					slider_to_combine = (struct Emoter_slider *)NULL;
					printf(" %d\n", shared->number_of_sliders);
					for ( i = 0 ; i < shared->number_of_sliders ; i++ )
					{
						if ( (shared->sliders[i])->sequence_filename )
						{
							printf("%s   %s\n", temp_string, (shared->sliders[i])->sequence_filename);
							if ( !strcmp( temp_string, (shared->sliders[i])->sequence_filename ))
							{
								slider_to_combine = shared->sliders[i];
								return_code = 1;
							}
						}
					}
					if ( !return_code )
					{
						sprintf(warning,  "Slider %s depends on %s which is not loaded.  Do you want to try and load it now?",
							slider->name, temp_string );
						if ( no_confirm || confirmation_warning_ok_cancel( "Emotionator dependency",
							warning, emoter_dialog->shell,
							shared->user_interface ))
						{
							if ( temp_filename = strrchr( temp_string, '/'))
							{
								temp_filename++;
							}
							else
							{
								temp_filename = temp_string;
							}
							slider_to_combine = create_emoter_slider(temp_string,
								temp_filename, emoter_dialog->slider_form,
								1, shared,
								shared->number_of_sliders,
								(struct Control_curve *)NULL,
								emoter_dialog, no_confirm );
							return_code = 1;
						}
					}

					if ( return_code )
					{
						if ( ALLOCATE( name, char, strlen( slider->name ) +
							strlen( slider_to_combine->name ) + 20 /* Allocate enough for timebase name too */ ))
						{
							sprintf(name, "%s in %s", slider_to_combine->name, slider->name );
							if ( curve = read_emoter_control_curve(file_data, name, shared) )
							{
								/* Need to add new combine slider */
								if ( REALLOCATE( new_combine_sliders, slider->combine_sliders,
									struct Emoter_combine_slider *, slider->number_of_combine_sliders + 1))
								{
									return_code = 0;
									slider->combine_sliders = new_combine_sliders;
									slider->number_of_combine_sliders++;
									if ( ALLOCATE ( combine_slider, struct Emoter_combine_slider, 1))
									{
										slider->combine_sliders[slider->number_of_combine_sliders-1]
											= combine_slider;
										combine_slider->slider = slider_to_combine;
										combine_slider->curve = curve;
										if ( read_file_marker(file_data, "TIMEBASE"))
										{
											strcat( name, " timebase" );
											if ( combine_slider->timebase_curve =
												read_emoter_control_curve(file_data, name, shared))
											{
												if ( REALLOCATE( new_timebase_curves, slider_to_combine->timebase_curves,
													struct Control_curve *, slider_to_combine->number_of_timebase_curves + 1))
												{
													new_timebase_curves[slider_to_combine->number_of_timebase_curves]
														= combine_slider->timebase_curve;
													slider_to_combine->timebase_curves = new_timebase_curves;
													slider_to_combine->number_of_timebase_curves++;
												}
												else
												{
													display_message(ERROR_MESSAGE,
														"read_emoter_slider_file.  Unable to reallocate the combined slider's timebase curves array");
													return_code = 0;
												}
											}
										}
										else
										{
											combine_slider->timebase_curve = (struct Control_curve *)NULL;
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"read_emoter_slider_file.  Unable to allocate combine slider structure");
										return_code = 0;
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"read_emoter_slider_file.  Unable to reallocate combine sliders array");
									return_code = 0;
								}

							}
							else
							{
								display_message(ERROR_MESSAGE,
									"read_emoter_slider_file.  Unable to read emoter control curve");
								return_code = 0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"read_emoter_slider_file.  Could not allocate combine curve name");
							return_code = 0;
						}
					}
					/* Tidy up to marker */
					while(!read_file_eof_marker(file_data, "END_COMBINE"))
					{
						read_file_string(file_data, temp_string );
					}
					return_code = 1;
				}
				else if ( read_file_marker(file_data, "ICON"))
				{
					/* Don't want to tokenise the next bit so just normal read */
					sscanf( file_data->current_token, "%d", icon_width );
					fscanf( file_data->file, "%d", icon_height );
					if ( ALLOCATE( *icon_data, char, 3 * *icon_width * *icon_height ))
					{
						char_data = *icon_data;
						for ( j = 0 ; j < *icon_height ; j++ )
						{
							for ( i = 0 ; i < *icon_width ; i++ )
							{
								fscanf(file_data->file, "%6x", &integer_data);
								*char_data = ( integer_data >> 16 ) & 0xff;
								char_data++;
								*char_data = ( integer_data >> 8 ) & 0xff;
								char_data++;
								*char_data = integer_data & 0xff;
								char_data++;
							}
							fscanf(file_data->file, "\n");
						}
					}
					else
					{
						*icon_data = NULL;
						display_message(ERROR_MESSAGE,
							"read_emoter_slider_file.  Unable to allocate icon memory");
						return_code = 0;
					}

					read_file_next_token(file_data);

					/* Tidy up to marker */
					while(!read_file_eof_marker(file_data, "END_ICON"))
					{
						read_file_string(file_data, temp_string );
					}
					return_code = 1;
				}
				else 
				{
					/* Try version 2 */
					if (read_file_marker(file_data, "em"))
					{
						if (read_file_marker(file_data, "sequence") &&
							!strcmp(file_data->current_token, "2.0"))
						{
							/* Read \n off previous line */
							fscanf(file_data->file, "%*[^\n]");
							fscanf(file_data->file, "%*[\n]");
							
							/* Comment/title line */
							fscanf(file_data->file, "%*[^\n]");
							fscanf(file_data->file, "%*[\n]");
							
							read_file_next_token(file_data);
							header = 1;
							index = 0;
							face_index = 0;
							face_values = 0;
							while (header)
							{
								if (read_file_marker(file_data, "face"))
								{
									face_index = index;
									read_file_int(file_data, &face_values);
									index = index + face_values;
								}
								else if (read_file_marker(file_data, "head"))
								{
									solid_body_index = index;
									read_file_int(file_data, &values);
									slider->solid_body_motion = values;
									index = index + values;
								}
								else if (read_file_marker(file_data, "eyes")
									|| read_file_marker(file_data, "jaw"))
								{
									read_file_int(file_data, &values);
									index = index + values;
								}
								else
								{
									header = 0;
								}
							}
							values = index;
							n_modes = face_values;
							return_code = 1;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"read_emoter_slider_file.  Unknown em file type");
							return_code = 0;						
						}
					}
					else
					{
						/* Drop the marker if it is there, otherwise assume mode data anyway */
						read_file_marker(file_data, "MODE_DATA");
						if ( read_file_marker(file_data, "SOLID_BODY_MOTION"))
						{
							slider->solid_body_motion = SOLID_BODY_MODES;
							solid_body_index = 0;
							read_file_int(file_data, &n_modes);
							face_values = n_modes;
							face_index = SOLID_BODY_MODES;
							values = SOLID_BODY_MODES + n_modes;
						}
						else
						{
							solid_body_index = 0;
							slider->solid_body_motion = 0;
							read_file_int(file_data, &n_modes);
							/* All the values are the face modes */
							face_values = n_modes;
							face_index = 0;
							values = n_modes;
						}
						return_code = 1;
					}
					if (return_code)
					{
						if (no_confirm || (shared->number_of_modes == n_modes))
						{
							return_code = 1;
						}
						else
						{
							sprintf(warning, "The number of modes in file %s is %d which is not the \nsame as the basis file which has %d modes.  Do you want to load this file anyway?",
								filename, n_modes, shared->number_of_modes );
							return_code = confirmation_warning_ok_cancel("Emotionator warning",
								warning, emoter_dialog->shell, shared->user_interface );
						}
					}
					if ( return_code )
					{
						if ( ALLOCATE( temp_filename, char, strlen(name)+1))
						{
							strcpy(temp_filename, name);
							name = temp_filename;
							if ( ALLOCATE( shape_vector, float, shared->number_of_modes + slider->solid_body_motion ))
							{
								while (FIND_BY_IDENTIFIER_IN_MANAGER(Control_curve,name)(
									name, shared->control_curve_manager))
								{
									REALLOCATE(name, name, char, strlen(name)+2);
									strcat(name,"+");
								}
								if (emoter_curve = CREATE(Control_curve)(name,
									LINEAR_LAGRANGE, shared->number_of_modes + slider->solid_body_motion))
								{
									if (ADD_OBJECT_TO_MANAGER(Control_curve)(
										emoter_curve, shared->control_curve_manager))
									{
										/* Set the type after the object is added to the
											manager so that ADD message is ignored */
										Control_curve_set_type(emoter_curve,
											CONTROL_CURVE_TYPE_EMOTER_MODES );
										ACCESS(Control_curve)(emoter_curve);
										/* SAB Do the first and second modes as special case
											as we need to create the first element, position both
											nodes and then from then on the current elements can
											just be appended to */
										Control_curve_add_element( emoter_curve, 1 );
										*start_time = 1.0;
										if ( read_file_marker(file_data, "MARKER"))
										{
											/* Discard a start marker name, it is already counted and the name will be start */
											read_file_string(file_data, temp_string);
											read_file_float(file_data, start_time);
										}
										total_time= *start_time;
										*end_time = *start_time;
										Control_curve_set_parameter( emoter_curve,
											/*element_no*/1,/*local_node_no*/0,	*start_time);
										read_weights( face_index, n_modes, solid_body_index, 
											slider->solid_body_motion, file_data, 
											shared->number_of_modes, values, shape_vector);
										Control_curve_set_node_values( emoter_curve,
											1, 0, shape_vector);
										if ( !read_file_eof_marker(file_data, "END_MODE_DATA"))
										{
											total_time += 1.0;
											if ( read_file_marker(file_data, "MARKER"))
											{
												/* Get the next marker */
												read_file_string(file_data, temp_string);
												read_file_float(file_data, end_time);
											}
											read_weights( face_index, n_modes, solid_body_index, 
												slider->solid_body_motion, file_data, 
												shared->number_of_modes, values, shape_vector);
										}
										Control_curve_set_parameter( emoter_curve,
											/*element_no*/1,/*local_node_no*/1,	total_time );

										/* else do nothing and make a single constant element */
										Control_curve_set_node_values( emoter_curve,
											1, 1, shape_vector);

										i = 2;
										while (!read_file_eof_marker(file_data, "END_MODE_DATA"))
										{
											if ( read_file_marker(file_data, "MARKER"))
											{
												/* If more than just an end time add marker */
												if ( *start_time != *end_time )
												{
													/* Make a marker with previous values */
													if ( REALLOCATE( new_markers, slider->emoter_markers,
														struct Emoter_marker *, *marker_count + 2))
													{
														slider->emoter_markers = new_markers;
														slider->emoter_markers[*marker_count]
															= create_emoter_marker(temp_string,
																*end_time, slider->marker_rowcol, *marker_count - 1,
																*marker_count + 1, 0, 0,
																slider, slider->shared);
														(*marker_count)++;
													}
													else
													{
														display_message(ERROR_MESSAGE,
															"read_emoter_slider_file.  Unable to reallocate marker array");
														return_code=0;
													}
												}
												read_file_string(file_data, temp_string);
												read_file_float(file_data, end_time);
											}
											Control_curve_add_element( emoter_curve, i );
											total_time += 1.0;
											Control_curve_set_parameter( emoter_curve,
												/*element_no*/i,/*local_node_no*/1,	total_time );
											read_weights( face_index, n_modes, solid_body_index, 
												slider->solid_body_motion, file_data, 
												shared->number_of_modes, values, shape_vector);
											Control_curve_set_node_values( emoter_curve,
												i, 1, shape_vector);
											i++;
										}
										if ( *start_time == *end_time )
										{
											*end_time = i;
										}
										slider->mode_curve = emoter_curve;
										return_code = 1;
									}
									else
									{
										DESTROY(Control_curve)(&emoter_curve);
										display_message(ERROR_MESSAGE,
											"read_emoter_slider_file.  Unable to add control curve to manager");
										return_code=0;
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"read_emoter_slider_file.  Unable to create control curve");
									return_code=0;
								}
								DEALLOCATE (shape_vector);
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"read_emoter_slider_file.  Unable to allocate shape vector");
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"read_emoter_slider_file.  Unable to allocate name string");
							return_code=0;
						}
					}
				}
			}
			read_file_close( file_data );
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"read_emoter_slider_file.  Unable to open file %s", filename);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"read_emoter_slider_file.  Invalid argument(s)");
		return_code=0;
	}

	LEAVE;

	return (return_code);
} /* read_emoter_slider_file */

struct Emoter_slider *create_emoter_slider(char *sequence_filename,
	char *name, Widget parent, float value,
	struct Shared_emoter_slider_data *shared_data,
	int index, struct Control_curve *existing_mode_curve,
	struct Emoter_dialog *emoter_dialog, int no_confirm)
/*******************************************************************************
LAST MODIFIED : 9 September 1999

DESCRIPTION :
Both or either of <sequence_filename> or <existing_mode_curve> can be NULL.
==============================================================================*/
{
	Arg override_arg;
	int marker_count, return_code;
	float start_time, end_time;
	MrmType emoter_slider_class;
	static MrmRegisterArg callback_list[]=
	{
		{"emoter_slider_destroy_CB",
			(XtPointer)destroy_emoter_slider_callback},
		{"emoter_id_emoter_slider",(XtPointer)emoter_id_emoter_slider},
		{"emoter_slider_CB",(XtPointer)emoter_slider_CB},
		{"emoter_id_slider_togglebutton",(XtPointer)
			DIALOG_IDENTIFY(emoter_slider, toggle_button)},
		{"emoter_id_slider_selectbutton",(XtPointer)
			DIALOG_IDENTIFY(emoter_slider, select_button)},
		{"emoter_slider_toggle_activate",
			(XtPointer)emoter_slider_toggle_activate},
		{"emoter_slider_select_activate",
			(XtPointer)emoter_slider_select_activate},
		{"emoter_id_slider_value_text",(XtPointer)
			DIALOG_IDENTIFY(emoter_slider, value_text)},
		{"emoter_slider_value_text_CB",
			(XtPointer)emoter_slider_value_text_CB},
		{"emoter_id_slider_anim_pixmap",(XtPointer)
			DIALOG_IDENTIFY(emoter_slider, animated_pixmap)},
		{"emoter_id_slider_markerrowcol",(XtPointer)
			DIALOG_IDENTIFY(emoter_slider, marker_rowcol)}
		};
	static MrmRegisterArg identifier_list[] =
	{
		{"emoter_slider_structure",(XtPointer)NULL},
		{"emoter_slider_name",(XtPointer)NULL},
	};
	struct Emoter_slider *emoter_slider;
	struct Graphics_buffer *graphics_buffer;
	XmString label_string;
	void *icon_data;
	int icon_width, icon_height;

	ENTER(create_emoter_slider);
	/* check arguments */

	if (name && parent && shared_data)
	{
		/* register the callbacks */
		if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
			emoter_dialog_hierarchy,callback_list,XtNumber(callback_list)))
		{
			if ( ALLOCATE ( emoter_slider, struct Emoter_slider, 1)&&
				ALLOCATE(emoter_slider->name, char, strlen(name) + 1) &&
				ALLOCATE(emoter_slider->combine_sliders, struct Emoter_combine_slider *, 1)&&
				ALLOCATE(emoter_slider->emoter_markers, struct Emoter_marker *, 2) &&
				ALLOCATE(emoter_slider->timebase_curves, struct Control_curve *, 1))
			{
				emoter_slider->index = index;
				strcpy(emoter_slider->name, name);
				emoter_slider->slider = (Widget)NULL;
				emoter_slider->widget = (Widget)NULL;
				emoter_slider->shared = shared_data;
				emoter_slider->minimum = -2.5;
				emoter_slider->maximum = 2.5;
				emoter_slider->mode_curve = (struct Control_curve *)NULL;
				emoter_slider->number_of_combine_sliders = 0;
				emoter_slider->value = value;
				emoter_slider->toggle_button = (Widget)NULL;
				emoter_slider->select_button = (Widget)NULL;
				emoter_slider->value_text = (Widget)NULL;
				emoter_slider->selected = 0;
				emoter_slider->animated_pixmap = (Widget)NULL;
				emoter_slider->animated = 0;
				emoter_slider->sequence_filename = (char *)NULL;
				emoter_slider->marker_rowcol = (Widget)NULL;
				emoter_slider->number_of_emoter_markers = 0;
				emoter_slider->number_of_timebase_curves = 0;
				emoter_slider->solid_body_motion = 0;

				start_time = 0;
				end_time = 0;
				marker_count = 1;
				icon_data = NULL;
				icon_width = 0;
				icon_height = 0;

				return_code = 1;
				if ( return_code )
				{
					/* assign and register the identifiers */
					label_string=XmStringCreateSimple(name);
					identifier_list[0].value=(XtPointer)emoter_slider;
					identifier_list[1].value=(XtPointer)label_string;
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
						emoter_dialog_hierarchy,identifier_list,
						XtNumber(identifier_list)))
					{
						XtSetArg(override_arg,XmNuserData,emoter_slider);
						if (MrmSUCCESS==MrmFetchWidgetOverride(emoter_dialog_hierarchy,
							"emoter_slider", parent,NULL,&override_arg,1,
							&emoter_slider->widget, &emoter_slider_class))
						{
							XtManageChild(emoter_slider->widget);
							if ( sequence_filename )
							{
								if ( ALLOCATE( emoter_slider->sequence_filename, char,
									strlen( sequence_filename ) + 1))
								{
									strcpy( emoter_slider->sequence_filename, sequence_filename );
									return_code = read_emoter_slider_file(
										emoter_slider, sequence_filename,
										&start_time, &end_time, &marker_count,
										emoter_dialog, &icon_data,
										&icon_width, &icon_height, no_confirm);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"create_emoter_slider.  Could not allocated sequence filename string");
								}
							}
							else
							{
								if (existing_mode_curve)
								{
									emoter_slider->mode_curve =
										ACCESS(Control_curve)(existing_mode_curve);
								}
								return_code = 1;
							}
							graphics_buffer = create_Graphics_buffer_X3d(
								emoter_slider->animated_pixmap,
								GRAPHICS_BUFFER_SINGLE_BUFFERING, GRAPHICS_BUFFER_MONO,
 								/*minimum_colour_buffer_depth*/0,
 								/*minimum_depth_buffer_depth*/0,
								/*minimum_accumulation_buffer_depth*/0,
								User_interface_get_specified_visual_id(
								shared_data->user_interface));
							emoter_slider->scene_viewer = CREATE(Scene_viewer)(
								graphics_buffer,
								&(shared_data->viewer_background_colour),
								(struct MANAGER(Light) *)NULL,shared_data->viewer_light,
								(struct MANAGER(Light_model) *)NULL,
								shared_data->viewer_light_model,
								(struct MANAGER(Scene) *)NULL,
								shared_data->viewer_scene,
								(struct MANAGER(Texture) *)NULL,
								shared_data->user_interface );
							Scene_viewer_set_viewport_size(emoter_slider->scene_viewer,
								50, 50);
							if ( !icon_data )
							{
								Scene_viewer_view_all(emoter_slider->scene_viewer);
								Scene_viewer_redraw(emoter_slider->scene_viewer);
							}
							else
							{
								Scene_viewer_view_all(emoter_slider->scene_viewer);
								Scene_viewer_set_input_mode(emoter_slider->scene_viewer, SCENE_VIEWER_UPDATE_ON_CLICK );
								Scene_viewer_set_pixel_image( emoter_slider->scene_viewer,
									icon_width, icon_height, icon_data );
								Scene_viewer_redraw(emoter_slider->scene_viewer);
							}
							emoter_slider->number_of_emoter_markers = marker_count + 1;
							emoter_slider->emoter_markers[0] = create_emoter_marker (
								"Start", start_time, emoter_slider->marker_rowcol,
								0, 1, 0, 0,
								emoter_slider, shared_data );
							emoter_slider->emoter_markers[marker_count] = create_emoter_marker (
								"End", end_time, emoter_slider->marker_rowcol,
								marker_count, marker_count, 1, 1,
								emoter_slider, shared_data );
							if ( REALLOCATE( shared_data->sliders,
								shared_data->sliders, struct Emoter_slider*,
								shared_data->number_of_sliders + 1))
							{
								shared_data->sliders
									[shared_data->number_of_sliders]
									= emoter_slider;
								shared_data->number_of_sliders++;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"create_emoter_slider.  Unable to Reallocate slider array");
							}
							emoter_slider_make_active( emoter_slider );
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"create_emoter_slider.  Could not create emoter slider widgets");
							DEALLOCATE ( emoter_slider );
							emoter_slider=(struct Emoter_slider *)NULL;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_emoter_slider.  Could not register identifiers");
						DEALLOCATE(emoter_slider);
						emoter_slider=(struct Emoter_slider *)NULL;
					}
					XmStringFree(label_string);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_emoter_slider.  Could not read control curve");
					DEALLOCATE(emoter_slider);
					emoter_slider=(struct Emoter_slider *)NULL;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_emoter_slider.  Could not allocate emoter slider structure");
				emoter_slider=(struct Emoter_slider *)NULL;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_emoter_dialog.  Could not register callbacks");
			emoter_slider=(struct Emoter_slider *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_emoter_slider.  Invalid argument(s)");
		emoter_slider=(struct Emoter_slider *)NULL;
	}
	LEAVE;

	return (emoter_slider);
} /* create_emoter_slider */

static void emoter_loadslider_CB(Widget widget,XtPointer user_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 7 April 1998

DESCRIPTION :

==============================================================================*/
{
	char *filename, *temp_filename;
	struct Emoter_dialog *emoter_dialog;
	struct Shared_emoter_slider_data *shared;

	ENTER(emoter_loadslider_CB);
	USE_PARAMETER(call_data);
	USE_PARAMETER(user_data);
	/* Get the pointer to the emoter_dialog structure */
	XtVaGetValues(widget,
		XmNuserData, &emoter_dialog,
		NULL);

	if ( emoter_dialog )
	{
		shared = emoter_dialog->shared;

		if ( temp_filename = confirmation_get_read_filename(".em", shared->user_interface))
		{
			if ( filename = strrchr( temp_filename, '/'))
			{
				filename++;
			}
			else
			{
				filename = temp_filename;
			}
			create_emoter_slider(temp_filename,
				filename, emoter_dialog->slider_form,
				1, shared,
				shared->number_of_sliders,
				(struct Control_curve *)NULL,
				emoter_dialog, /*no_confirm*/0 );
			DEALLOCATE( temp_filename );
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"emoter_loadslider_CB.  Could not get emoter_dialog");
	}

	LEAVE;
} /* emoter_loadslider_CB */

static void emoter_save_slider(struct Emoter_slider *slider,
	char *filename, struct Emoter_dialog *emoter_dialog)
/*******************************************************************************
LAST MODIFIED : 17 November 1999

DESCRIPTION :

==============================================================================*/
{
	char *char_data;
	FILE *file;
	float delta_time, dS_dxi, end_time, start_time, frame_time, value,
		derivative, *temp_data;
	int i, j, k, frame, nodes_per_element, elements, derivatives,
		number_of_frames, width, height, marker_no, number_of_components;
	struct Control_curve *curve;
	time_t current_time;
	void *data;

	ENTER(emoter_save_slider);


	if ( slider && filename )
	{
		if ( file = fopen( filename, "w" ) )
		{
			/* Write header comment */
			fprintf(file, "#Emoter emotion file '%s' written ", slider->name);
			time(&current_time);
			fprintf(file, ctime(&current_time));

			number_of_frames = slider->emoter_markers
				[slider->number_of_emoter_markers-1]->value
				- slider->emoter_markers[0]->value + 1;
			if ( number_of_frames > 0 && slider->mode_curve )
			{
				marker_no = 0;
				fprintf(file, "MODE_DATA\n");
				if ( slider->solid_body_motion )
				{
					fprintf(file, "SOLID_BODY_MOTION\n" );
				}
				fprintf(file, "%d\n", slider->shared->number_of_modes );

				/* Set up the temporary array */
				for ( frame = 0 ; frame < number_of_frames ; frame++ )
				{
					frame_time = slider->emoter_markers[0]->value + frame;
					temp_data = (float *)NULL;

					if ( frame_time == slider->emoter_markers[marker_no]->value )
					{
						fprintf(file, "MARKER %s %f\n", slider->emoter_markers[marker_no]->name,
							slider->emoter_markers[marker_no]->value );
						marker_no++;
					}
					if ( ALLOCATE( temp_data, float, SOLID_BODY_MODES + emoter_dialog->shared->number_of_modes ))
					{
						for ( i = 0 ; i < SOLID_BODY_MODES + emoter_dialog->shared->number_of_modes ; i++ )
						{
							temp_data[i] = 0;
						}
						temp_data[SOLID_BODY_MODES] += EM_standard_mode_one( emoter_dialog->shared->em_object );

						emoter_add_slider_modes ( slider,
							temp_data, emoter_dialog->shared, frame_time );
						if ( slider->solid_body_motion )
						{
							for ( i = 0 ; i < SOLID_BODY_MODES ; i++ )
							{
								fprintf(file, "%e ", temp_data[i]);
							}
						}
						for ( i = SOLID_BODY_MODES ; i < emoter_dialog->shared->number_of_modes
									+ SOLID_BODY_MODES ; i++ )
						{
							fprintf(file, "%e ", temp_data[i]);
						}
						fprintf(file, "\n");
						DEALLOCATE( temp_data );
					}
				}
				fprintf(file, "END_MODE_DATA\n");
			}
			for ( i = 0 ; i < slider->number_of_combine_sliders ; i++ )
			{
				if ( (slider->combine_sliders[i])->slider->sequence_filename )
				{
					curve = (slider->combine_sliders[i])->curve;
					number_of_components=Control_curve_get_number_of_components(curve);
					if ( 1 == number_of_components )
					{
						fprintf(file, "COMBINE\n");
						fprintf(file, "%s\n", (slider->combine_sliders[i])->slider->sequence_filename );
						nodes_per_element = Control_curve_get_nodes_per_element
							(curve);
						elements = Control_curve_get_number_of_elements
							(curve);
						derivatives = Control_curve_get_derivatives_per_node(curve);
						Control_curve_get_parameter_range( curve, &start_time , &end_time);

						fprintf(file, "%d #nodes per element\n", nodes_per_element);
						fprintf(file, "%d #elements\n", elements);
						fprintf(file, "%d #derivatives\n", derivatives);
						fprintf(file, "%f #start time\n", start_time);
						for ( j = 1 ; j <= elements ; j++ )
						{
							Control_curve_get_element_parameter_change(
								curve,
								j, &delta_time);
							fprintf(file, "%f",delta_time );
							for ( k = 0 ; k < nodes_per_element - 1 ; k++ )
							{
								if (Control_curve_get_node_values(curve, j, k, &value))
								{
									fprintf(file, " %f", value);
									if ( derivatives )
									{
										if (Control_curve_get_node_derivatives(
											curve, j, k, &derivative) &&
											Control_curve_get_node_scale_factor_dparameter(
												curve, j, k, &dS_dxi))
										{
											fprintf(file, " %f", derivative * dS_dxi );
										}
									}
								}
							}
							fprintf(file, "\n");
						}
						/* Last node as a special case */
						if (Control_curve_get_node_values(curve, elements,
							nodes_per_element-1, &value))
						{
							fprintf(file, " %f", value);
							if ( derivatives )
							{
								if (Control_curve_get_node_derivatives(curve,
									elements, nodes_per_element-1, &derivative) &&
									Control_curve_get_node_scale_factor_dparameter(
										curve, elements, nodes_per_element-1, &dS_dxi))
								{
									fprintf(file, " %f", derivative * dS_dxi );
								}
							}
							fprintf(file, "\n");
						}
						if ( curve = (slider->combine_sliders[i])->timebase_curve)
						{
							fprintf(file, "TIMEBASE\n");
							nodes_per_element = Control_curve_get_nodes_per_element
								(curve);
							elements = Control_curve_get_number_of_elements
								(curve);
							derivatives = Control_curve_get_derivatives_per_node(curve);
							Control_curve_get_parameter_range( curve,
								&start_time , &end_time);

							fprintf(file, "%d #nodes per element\n", nodes_per_element);
							fprintf(file, "%d #elements\n", elements);
							fprintf(file, "%d #derivatives\n", derivatives);
							fprintf(file, "%f #start time\n", start_time);
							for ( j = 1 ; j <= elements ; j++ )
							{
								Control_curve_get_element_parameter_change(
									curve,
									j, &delta_time);
								fprintf(file, "%f",delta_time );
								for ( k = 0 ; k < nodes_per_element - 1 ; k++ )
								{
									if (Control_curve_get_node_values(curve, j, k, &value))
									{
										fprintf(file, " %f", value);
										if ( derivatives )
										{
											if (Control_curve_get_node_derivatives(
												curve, j, k, &derivative) &&
												Control_curve_get_node_scale_factor_dparameter(
													curve, j, k, &dS_dxi))
											{
												fprintf(file, " %f", derivative * dS_dxi );
											}
										}
									}
								}
								fprintf(file, "\n");
							}
							/* Last node as a special case */
							if (Control_curve_get_node_values(curve, elements,
								nodes_per_element-1, &value))
							{
								fprintf(file, " %f", value);
								if ( derivatives )
								{
									if (Control_curve_get_node_derivatives(
										curve, elements, nodes_per_element-1, &derivative) &&
										Control_curve_get_node_scale_factor_dparameter(
											curve, elements, nodes_per_element-1, &dS_dxi))
									{
										fprintf(file, " %f", derivative * dS_dxi );
									}
								}
								fprintf(file, "\n");
							}
						}
						fprintf(file, "END_COMBINE\n");
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"emoter_saveslider_CB. Incorrect number of components in control curve");
					}
				}
				else
				{
						display_message(INFORMATION_MESSAGE,
							"emoter_save_slider. Slider %s depends on %s which has not been saved.\n  To keep this dependency, save it first and then this slider\n",
							slider->name, (slider->combine_sliders[i])->slider->name);

				}
			}
			if ( Scene_viewer_get_pixel_image ( slider->scene_viewer,
				&width, &height, &data ))
			{
				char_data = (char *)data;
				fprintf(file, "ICON #RGB data\n");
				fprintf(file, "%d %d\n", width, height );
				for ( j = 0 ; j < height ; j++ )
				{
					for ( i = 0 ; i < width ; i++ )
					{
						for ( k = 0 ; k < 3 ; k++ )
						{
							fprintf(file, "%02x", *char_data);
							char_data++;
						}
					}
					fprintf(file, "\n");
				}
				fprintf(file, "END_ICON\n");
			}
			fclose(file);
			if ( REALLOCATE( slider->sequence_filename, slider->sequence_filename,
				char, strlen( filename ) + 1))
			{
				strcpy ( slider->sequence_filename, filename );
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"emoter_save_slider.  Unable to open file %s", filename);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"emoter_save_slider.  Could not get emoter_dialog");
	}

	LEAVE;
} /* emoter_save_slider */

static void emoter_saveslider_CB(Widget widget,XtPointer user_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 15 April 1998

DESCRIPTION :

==============================================================================*/
{
	char *filename, *temp_filename;
	struct Emoter_dialog *emoter_dialog;
	struct Shared_emoter_slider_data *shared;
	XmString label_string;

	ENTER(emoter_saveslider_CB);
	USE_PARAMETER(call_data);
	USE_PARAMETER(user_data);
	/* Get the pointer to the emoter_dialog structure */
	XtVaGetValues(widget,
		XmNuserData, &emoter_dialog,
		NULL);

	if ( emoter_dialog )
	{
		shared = emoter_dialog->shared;

		if ( filename = confirmation_get_write_filename("", shared->user_interface))
		{
			emoter_save_slider(shared->active_slider, filename,
				emoter_dialog);
			if ( shared->active_slider->select_button )
			{
				if ( temp_filename = strrchr( filename, '/'))
				{
					temp_filename++;
				}
				else
				{
					temp_filename = filename;
				}
				label_string=XmStringCreateSimple(temp_filename);

				XtVaSetValues( shared->active_slider->select_button,
					XmNlabelString, label_string,
					NULL );

				XtVaSetValues( shared->active_slider->select_button,
					XmNwidth, 150,
					NULL );

				XmStringFree(label_string);
			}
			DEALLOCATE(filename);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"emoter_saveslider_CB.  Could not get emoter_dialog");
	}

	LEAVE;
} /* emoter_saveslider_CB */

static int emoter_export_nodes(struct Emoter_dialog *emoter_dialog,
	char *filename)
/*******************************************************************************
LAST MODIFIED : 9 September 1999

DESCRIPTION :

==============================================================================*/
{
	char *command;
	int frame, number_of_frames, return_code;
	float time;
	struct Shared_emoter_slider_data *shared;

	ENTER(emoter_saveslider_CB);
	if ( emoter_dialog && filename)
	{
		return_code = 1;
		shared = emoter_dialog->shared;
		if ( ALLOCATE( command, char, strlen(filename)+100 ))
		{
			number_of_frames = floor( emoter_dialog->time_maximum -
				emoter_dialog->time_minimum + 1);
			
			for ( frame = 0 ; frame < number_of_frames ; frame++ )
			{
				time = emoter_dialog->time_minimum + frame;
				emoter_set_play_slider_value ( emoter_dialog, time );
				emoter_update_face( shared );
				sprintf(command, "gfx write nodes %s.%05d", filename, (int)time );
				Execute_command_execute_string(shared->execute_command,command);
			}
			DEALLOCATE(command);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"emoter_export_nodes.  Invalid arguments");
		return_code = 0;
	}

	return (return_code);

	LEAVE;
} /* emoter_export_nodes */

static void emoter_exportslider_CB(Widget widget,XtPointer user_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 April 1998

DESCRIPTION :

==============================================================================*/
{
	char *filename;
	struct Emoter_dialog *emoter_dialog;

	ENTER(emoter_saveslider_CB);
	USE_PARAMETER(call_data);
	USE_PARAMETER(user_data);
	/* Get the pointer to the emoter_dialog structure */
	XtVaGetValues(widget,
		XmNuserData, &emoter_dialog,
		NULL);

	if ( emoter_dialog )
	{
		if ( filename = confirmation_get_write_filename("#####.exnode", 
			emoter_dialog->shared->user_interface))
		{
			*strstr(filename, "#####.exnode") = 0;
			emoter_export_nodes(emoter_dialog, filename);
			DEALLOCATE(filename);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"emoter_exportslider_CB.  Could not get emoter_dialog");
	}

	LEAVE;
} /* emoter_exportslider_CB */

static void emoter_autoplay_CB(XtPointer user_data,
	XtIntervalId *id)
/*******************************************************************************
LAST MODIFIED : 21 April 1998

DESCRIPTION :

==============================================================================*/
{
	Boolean button_set;
	float time;
	struct Emoter_dialog *emoter_dialog;
	Widget widget;

	ENTER(emoter_playbutton_CB);
	USE_PARAMETER(id);
	if ( widget = (Widget)user_data )
	{
		/* Get the pointer to the emoter_dialog structure */
		XtVaGetValues(widget,
			XmNuserData, &emoter_dialog,
			XmNset, &button_set,
			NULL);

		if ( emoter_dialog )
		{
			if ( button_set )
			{
				time = emoter_dialog->shared->time + 1;
				if ( time > emoter_dialog->time_maximum )
				{
					time = emoter_dialog->time_minimum;
				}
				emoter_set_play_slider_value ( emoter_dialog, time );
				emoter_update_face( emoter_dialog->shared );
				XtAppAddTimeOut( XtWidgetToApplicationContext(widget),
					1, emoter_autoplay_CB, (void *)widget );
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"emoter_autoplay_CB.  Could not get emoter_dialog");
		}

	}
	else
	{
		display_message(ERROR_MESSAGE,
			"emoter_autoplay_CB.  Invalid user_data");
	}

	LEAVE;
} /* emoter_autoplay_CB */

static void emoter_playbutton_CB(Widget widget,XtPointer user_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 April 1998

DESCRIPTION :

==============================================================================*/
{
	Boolean button_set;
	float time;
	struct Emoter_dialog *emoter_dialog;

	ENTER(emoter_playbutton_CB);
	USE_PARAMETER(call_data);
	USE_PARAMETER(user_data);
	/* Get the pointer to the emoter_dialog structure */
	XtVaGetValues(widget,
		XmNuserData, &emoter_dialog,
		XmNset, &button_set,
		NULL);

	if ( emoter_dialog )
	{
		if ( emoter_dialog->shared->movie_playing )
		{
			if ( emoter_dialog->movie )
			{
				if ( button_set )
				{
					X3d_movie_play(Movie_graphics_get_X3d_movie(emoter_dialog->movie));
				}
				else
				{
					X3d_movie_stop(Movie_graphics_get_X3d_movie(emoter_dialog->movie));
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"emoter_playbutton_CB.  Movie scene_viewer not found");
			}
		}
		else
		{
			if ( button_set )
			{
				time = emoter_dialog->shared->time + 1;
				if ( time > emoter_dialog->time_maximum )
				{
					time = emoter_dialog->time_minimum;
				}
				emoter_set_play_slider_value ( emoter_dialog, time );
				emoter_update_face( emoter_dialog->shared );
				XtAppAddTimeOut( XtWidgetToApplicationContext(widget),
					1, emoter_autoplay_CB, (void *)widget );
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"emoter_playbutton_CB.  Could not get emoter_dialog");
	}

	LEAVE;
} /* emoter_playbutton_CB */

static void emoter_movie_loop_CB(Widget widget,XtPointer user_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 April 1998

DESCRIPTION :

==============================================================================*/
{
	Boolean button_set;
	struct Emoter_dialog *emoter_dialog;

	ENTER(emoter_movie_loop_CB);
	USE_PARAMETER(call_data);
	USE_PARAMETER(user_data);
	/* Get the pointer to the emoter_dialog structure */
	XtVaGetValues(widget,
		XmNuserData, &emoter_dialog,
		XmNset, &button_set,
		NULL);

	if ( emoter_dialog )
	{
		if ( button_set )
		{
			emoter_dialog->movie_loop = 1;
		}
		else
		{
			emoter_dialog->movie_loop = 0;
		}
		if ( emoter_dialog->movie )
		{
			X3d_movie_set_play_loop(
				Movie_graphics_get_X3d_movie(emoter_dialog->movie),
				emoter_dialog->movie_loop );
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"emoter_movie_loop_CB.  Invalid emoter_dialog");
	}

	LEAVE;
} /* emoter_movie_loop_CB */

static void emoter_movie_every_frame_CB(Widget widget,XtPointer user_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 April 1998

DESCRIPTION :

==============================================================================*/
{
	Boolean button_set;
	struct Emoter_dialog *emoter_dialog;

	ENTER(emoter_movie_every_frame_CB);
	USE_PARAMETER(call_data);
	USE_PARAMETER(user_data);
	/* Get the pointer to the emoter_dialog structure */
	XtVaGetValues(widget,
		XmNuserData, &emoter_dialog,
		XmNset, &button_set,
		NULL);

	if ( emoter_dialog )
	{
		if ( button_set )
		{
			emoter_dialog->movie_every_frame = 1;
		}
		else
		{
			emoter_dialog->movie_every_frame = 0;
		}
		if ( emoter_dialog->movie )
		{
			X3d_movie_set_play_every_frame(
				Movie_graphics_get_X3d_movie(emoter_dialog->movie),
				emoter_dialog->movie_every_frame );
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"emoter_movie_every_frame_CB.  Invalid emoter_dialog");
	}

	LEAVE;
} /* emoter_movie_every_frame_CB */

static void emoter_movie_framerate_CB(Widget widget,XtPointer user_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 23 April 1998

DESCRIPTION :

==============================================================================*/
{
	char *string, temp_string[40];
	double frame_rate;
	struct Emoter_dialog *emoter_dialog;

	ENTER(emoter_movie_framerate_CB);
	USE_PARAMETER(call_data);
	USE_PARAMETER(user_data);
	/* Get the pointer to the emoter_dialog structure */
	XtVaGetValues(widget,
		XmNuserData, &emoter_dialog,
		NULL);

	if ( emoter_dialog && emoter_dialog->movie )
	{
		string = XmTextGetString( widget );
		sscanf( string, "%lf", &frame_rate );
		XtFree( string );
		X3d_movie_set_play_speed(
			Movie_graphics_get_X3d_movie(emoter_dialog->movie), frame_rate );
		sprintf( temp_string, "%6.2lf", frame_rate );
		XtVaSetValues( widget,
			XmNvalue, temp_string,
			NULL );
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"emoter_movie_framerate_CB.  Invalid emoter_dialog");
	}

	LEAVE;
} /* emoter_movie_framerate_CB */

static int emoter_create_movie(struct Emoter_dialog *emoter_dialog,
	char *filename)
/*******************************************************************************
LAST MODIFIED : 3 February 2000

DESCRIPTION :
==============================================================================*/
{
	int frame, number_of_frames, return_code;
	float time;
	struct Graphics_window *graphics_window;

	ENTER(emoter_create_movie);

	if ( emoter_dialog )
	{
		return_code = 1;
		busy_cursor_on((Widget)NULL, emoter_dialog->shared->user_interface );
		if (graphics_window=FIRST_OBJECT_IN_MANAGER_THAT(Graphics_window)(
			(MANAGER_CONDITIONAL_FUNCTION(Graphics_window) *)NULL,
			(void *)NULL,emoter_dialog->shared->graphics_window_manager))
		{
			if ((emoter_dialog->movie=CREATE(Movie_graphics)("emoter_movie",filename, 
				X3D_MOVIE_CREATE_FILE_RLE24_SGI_MOVIE3))&&
				(Movie_graphics_set_Graphics_window(emoter_dialog->movie,
					graphics_window)))
			{
				number_of_frames = floor( emoter_dialog->time_maximum -
					emoter_dialog->time_minimum + 1);
				for ( frame = 0 ; frame < number_of_frames ; frame++ )
				{
					time = emoter_dialog->time_minimum + frame;
					emoter_set_play_slider_value ( emoter_dialog, time );
					emoter_update_face( emoter_dialog->shared );
					Movie_graphics_add_frame_to_movie(emoter_dialog->movie,
						/*width*/0, /*heigth*/0, /*force_onscreen*/0);
				}
#if defined (OLD_CODE)
				/* The movieplay override functions are not operating
					so the widgets are not made visible for the moment,
					instead.... */
				X3d_movie_set_play_loop(
					Movie_graphics_get_X3d_movie(emoter_dialog->movie),
					emoter_dialog->movie_loop );
				X3d_movie_set_play_every_frame(
					Movie_graphics_get_X3d_movie(emoter_dialog->movie),
					emoter_dialog->movie_every_frame );
				X3d_movie_play(Movie_graphics_get_X3d_movie(emoter_dialog->movie));
				emoter_dialog->shared->movie_playing = 1;
				XmToggleButtonSetState( emoter_dialog->play_button,
					True, False );
				XtManageChild( emoter_dialog->movie_control_form );
#endif /* defined (OLD_CODE) */
				/* ....we just close the movie off and reset the button */
				DESTROY(Movie_graphics)( &emoter_dialog->movie );
				emoter_dialog->movie = (struct Movie_graphics *)NULL;
				emoter_dialog->shared->movie_playing = 0;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"emoter_movie_play_CB.  could not create movie %s", filename);
				return_code = 0;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"emoter_create_movie.  could not get graphics window");
			return_code = 0;
		}
		busy_cursor_off((Widget)NULL, emoter_dialog->shared->user_interface );
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"emoter_create_movie.  Missing emoter dialog");
		return_code = 0;
	}
 
	LEAVE;

	return (return_code);
} /* emoter_create_movie */

static void emoter_movie_play_CB(Widget widget,XtPointer user_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 16 October 1998

DESCRIPTION :
==============================================================================*/
{
	Boolean button_set;
	char *filename;
	struct Emoter_dialog *emoter_dialog;

	ENTER(emoter_movie_play_CB);
	USE_PARAMETER(call_data);
	USE_PARAMETER(user_data);
	/* Get the pointer to the emoter_dialog structure */
	XtVaGetValues(widget,
		XmNuserData, &emoter_dialog,
		XmNset, &button_set,
		NULL);

	if ( emoter_dialog )
	{
		if ( button_set )
		{
			if ( !emoter_dialog->movie )
			{
				if ( filename = confirmation_get_write_filename("", emoter_dialog->shared->user_interface ))
				{
					emoter_create_movie(emoter_dialog, filename);
					/* Normally only turn off the button for an error but the 
						movie play stuff isn't enabled currently and so we always
						set the button back to off */
					XmToggleButtonSetState( widget,
						False, False );
					DEALLOCATE( filename );
				}
				else
				{
					/* Cancel so unset togglebutton */
					XmToggleButtonSetState( widget,
						False, False );
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"emoter_movie_play_CB.  scene_viewer already selected");
			}
		}
		else
		{
			if ( emoter_dialog->movie )
			{
				DESTROY(Movie_graphics)( &emoter_dialog->movie );
				emoter_dialog->movie = (struct Movie_graphics *)NULL;
				emoter_dialog->shared->movie_playing = 0;
				emoter_update_face( emoter_dialog->shared );
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"emoter_movie_play_CB.  scene_viewer for movie not found");
			}
			XtUnmanageChild( emoter_dialog->movie_control_form );
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"emoter_movie_play_CB.  Could not get emoter_dialog");
	}

	LEAVE;
} /* emoter_movie_play_CB */

static int emoter_set_mode_limit(struct Emoter_dialog *emoter_dialog,
	int new_value )
/*******************************************************************************
LAST MODIFIED : 21 April 1998

DESCRIPTION :

==============================================================================*/
{
	char value_text_string[20];
	int i, *node_numbers, number_of_nodes, return_code;
	struct FE_node *node;

	ENTER(emoter_set_mode_limit);

	if ( emoter_dialog )
	{
		if ( new_value > emoter_dialog->shared->number_of_modes )
		{
			new_value = emoter_dialog->shared->number_of_modes;
		}
		if ( new_value < 1 )
		{
			new_value = 1;
		}
		emoter_dialog->shared->mode_limit = new_value;
		if ( emoter_dialog->mode_text )
		{
			sprintf(value_text_string, "%5d", new_value );
			XmTextSetString(emoter_dialog->mode_text, value_text_string);
		}
		if ( node_numbers = emoter_dialog->shared->em_object->minimum_nodes )
		{
			number_of_nodes = new_value;
			if ( emoter_dialog->minimum_nodegroup )
			{
				MANAGED_GROUP_BEGIN_CACHE(FE_node)(emoter_dialog->minimum_nodegroup);
				REMOVE_ALL_OBJECTS_FROM_GROUP(FE_node)(emoter_dialog->minimum_nodegroup);

				return_code = 0;
				for ( i = 0 ; i < number_of_nodes ; i++ )
				{
					if ( ( node_numbers[i] != -1 ) &&
						!FIND_BY_IDENTIFIER_IN_GROUP(FE_node,cm_node_identifier)(
						node_numbers[i],emoter_dialog->minimum_nodegroup))
					{
						if ( (node=FIND_BY_IDENTIFIER_IN_MANAGER(FE_node,cm_node_identifier)(
							node_numbers[i],emoter_dialog->shared->node_manager))&&
							ADD_OBJECT_TO_GROUP(FE_node)(node,emoter_dialog->minimum_nodegroup))
						{
							return_code = 1;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"emoter_set_mode_limit.  Unable to add node %d", i);
						}
					}
				}

				MANAGED_GROUP_END_CACHE(FE_node)(emoter_dialog->minimum_nodegroup);
			}
			else
			{
				if ( emoter_dialog->minimum_nodegroup
					= CREATE_GROUP(FE_node)("minimum_set"))
				{
					return_code = 0;
					for ( i = 0 ; i < number_of_nodes ; i++ )
					{
						if ( ( node_numbers[i] != -1 ) &&
							!FIND_BY_IDENTIFIER_IN_GROUP(FE_node,cm_node_identifier)(
							node_numbers[i],emoter_dialog->minimum_nodegroup))
						{
							if ( (node=FIND_BY_IDENTIFIER_IN_MANAGER(FE_node,cm_node_identifier)(
								node_numbers[i],emoter_dialog->shared->node_manager))&&
								ADD_OBJECT_TO_GROUP(FE_node)(node,emoter_dialog->minimum_nodegroup))
							{
								return_code = 1;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"emoter_set_mode_limit.  Unable to add node %d", i);
							}
						}
					}
					if ( return_code )
					{
						ACCESS(GROUP(FE_node))( emoter_dialog->minimum_nodegroup );
						if (ADD_OBJECT_TO_MANAGER(GROUP(FE_node))(emoter_dialog->minimum_nodegroup,
							emoter_dialog->shared->node_group_manager))
						{
							if ( emoter_dialog->minimum_elementgroup
								= CREATE_GROUP(FE_element)("minimum_set"))
							{
								ACCESS(GROUP(FE_element))( emoter_dialog->minimum_elementgroup );
								if (ADD_OBJECT_TO_MANAGER(GROUP(FE_element))(emoter_dialog->minimum_elementgroup,
									emoter_dialog->shared->element_group_manager))
								{
									return_code = 1;
								}
								else
								{
									DEACCESS(GROUP(FE_element))( &(emoter_dialog->minimum_elementgroup) );
									display_message(ERROR_MESSAGE,
										"emoter_set_mode_limit.  Could not add element group to manager");
									return_code=0;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"emoter_set_mode_limit.  Could not create element group");
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"emoter_set_mode_limit.  Could not add group to manager");
							DEACCESS(GROUP(FE_node))( &(emoter_dialog->minimum_nodegroup) );
							return_code=0;
						}
					}
					else
					{
					display_message(ERROR_MESSAGE,
						"emoter_set_mode_limit.  No nodes in group");
					return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"emoter_set_mode_limit.  Could not create node group");
					return_code = 0;
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"emoter_set_mode_limit.  Could not get emoter_dialog");
		return_code = 0;
	}

	LEAVE;

	return( return_code );
} /* emoter_set_mode_limit */

static void emoter_mode_text_CB(Widget widget,XtPointer user_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 April 1998

DESCRIPTION :

==============================================================================*/
{
	char *text_string;
	int value;
	struct Emoter_dialog *emoter_dialog;

	ENTER(emoter_mode_text_CB);
	USE_PARAMETER(call_data);
	USE_PARAMETER(user_data);
	/* Get the pointer to the emoter_dialog structure */
	XtVaGetValues(widget,
		XmNuserData, &emoter_dialog,
		NULL);

	if ( emoter_dialog && emoter_dialog->mode_subform)
	{
		if ( text_string = XmTextGetString(widget))
		{
			sscanf(text_string, "%d", &value );
			emoter_set_mode_limit( emoter_dialog, value );
			emoter_update_face( emoter_dialog->shared );
			XtFree ( text_string );
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"emoter_mode_text_CB.  Invalid arguments");
	}

	LEAVE;
} /* emoter_mode_text_CB */

static void emoter_mode_up_CB(Widget widget,XtPointer user_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 April 1998

DESCRIPTION :

==============================================================================*/
{
	struct Emoter_dialog *emoter_dialog;

	ENTER(emoter_mode_up_CB);
	USE_PARAMETER(call_data);
	USE_PARAMETER(user_data);
	/* Get the pointer to the emoter_dialog structure */
	XtVaGetValues(widget,
		XmNuserData, &emoter_dialog,
		NULL);

	if ( emoter_dialog )
	{
		emoter_set_mode_limit( emoter_dialog, emoter_dialog->shared->mode_limit + 1 );
		emoter_update_face( emoter_dialog->shared );
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"emoter_mode_up_CB.  Invalid arguments");
	}

	LEAVE;
} /* emoter_mode_up_CB */

static void emoter_mode_down_CB(Widget widget,XtPointer user_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 April 1998

DESCRIPTION :

==============================================================================*/
{
	struct Emoter_dialog *emoter_dialog;

	ENTER(emoter_mode_down_CB);
	USE_PARAMETER(call_data);
	USE_PARAMETER(user_data);
	/* Get the pointer to the emoter_dialog structure */
	XtVaGetValues(widget,
		XmNuserData, &emoter_dialog,
		NULL);

	if ( emoter_dialog )
	{
		emoter_set_mode_limit( emoter_dialog, emoter_dialog->shared->mode_limit - 1 );
		emoter_update_face( emoter_dialog->shared );
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"emoter_mode_down_CB.  Invalid arguments");
	}

	LEAVE;
} /* emoter_mode_down_CB */

static void emoter_mode_show_CB(Widget widget,XtPointer user_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 April 1998

DESCRIPTION :

==============================================================================*/
{
	Boolean button_set;
	struct Emoter_dialog *emoter_dialog;

	ENTER(emoter_mode_show_CB);
	USE_PARAMETER(call_data);
	USE_PARAMETER(user_data);
	/* Get the pointer to the emoter_dialog structure */
	XtVaGetValues(widget,
		XmNuserData, &emoter_dialog,
		XmNset, &button_set,
		NULL);

	if ( emoter_dialog && emoter_dialog->mode_subform)
	{
		if ( button_set )
		{
			XtManageChild( emoter_dialog->mode_subform );
		}
		else
		{
			XtUnmanageChild( emoter_dialog->mode_subform );
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"emoter_mode_show_CB.  Invalid arguments");
	}

	LEAVE;
} /* emoter_mode_show_CB */

static void emoter_solid_motion_CB(Widget widget,XtPointer user_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 21 April 1998

DESCRIPTION :

==============================================================================*/
{
	Boolean button_set;
	struct Emoter_dialog *emoter_dialog;

	ENTER(emoter_solid_motion_CB);
	USE_PARAMETER(call_data);
	USE_PARAMETER(user_data);
	/* Get the pointer to the emoter_dialog structure */
	XtVaGetValues(widget,
		XmNuserData, &emoter_dialog,
		XmNset, &button_set,
		NULL);

	if ( emoter_dialog )
	{
		if ( button_set )
		{
			emoter_dialog->shared->show_solid_body_motion = 1;
		}
		else
		{
			emoter_dialog->shared->show_solid_body_motion = 0;
		}
		emoter_update_face( emoter_dialog->shared );
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"emoter_solid_motion_CB.  Invalid arguments");
	}

	LEAVE;
} /* emoter_solid_motion_CB */

static void emoter_input_sequence_CB(Widget widget,XtPointer user_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 8 January 1999

DESCRIPTION :
==============================================================================*/
{
	Boolean button_set;
	struct Emoter_dialog *emoter_dialog;

	ENTER(emoter_input_sequence_CB);
	USE_PARAMETER(call_data);
	USE_PARAMETER(user_data);
	/* Get the pointer to the emoter_dialog structure */
	XtVaGetValues(widget,
		XmNuserData, &emoter_dialog,
		XmNset, &button_set,
		NULL);

	if ( emoter_dialog )
	{
		if ( button_set )
		{
			if (!(emoter_dialog->shared->input_sequence = 
				confirmation_get_string("Input sequence template",
				"Enter a printf string (time is a float parameter)\ni.e. taketest.%05.0f.exnode",
				(char *)NULL,emoter_dialog->shell,
				emoter_dialog->shared->user_interface)))
			{
				XmToggleButtonSetState( widget, False, False );				
			}
		}
		else
		{
			if ( emoter_dialog->shared->input_sequence)
			{
				DEALLOCATE(emoter_dialog->shared->input_sequence);
				emoter_dialog->shared->input_sequence = (char *)NULL;
			}
		}
		emoter_update_face( emoter_dialog->shared );
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"emoter_input_sequence_CB.  Invalid arguments");
	}

	LEAVE;
} /* emoter_input_sequence_CB */

static int emoter_convert_active_slider(struct Emoter_dialog *emoter_dialog)
/*******************************************************************************
LAST MODIFIED : 9 September 1999

DESCRIPTION :

==============================================================================*/
{
	char *var_name;
	float time, **temp_data;
	int i, j, frame, number_of_frames, solid_body_modes, mode_offset, return_code;
	struct Emoter_slider *active, *combine_slider;
	struct Shared_emoter_slider_data *shared;
	struct Control_curve *temp_var;

	ENTER(emoter_convert_active_slider);

	if ( emoter_dialog  && emoter_dialog->shared &&
			emoter_dialog->shared->active_slider )
	{
		return_code = 1;
		shared = emoter_dialog->shared;
		active = shared->active_slider;

		busy_cursor_on((Widget)NULL, emoter_dialog->shared->user_interface );
		number_of_frames = floor( emoter_dialog->time_maximum -
			emoter_dialog->time_minimum + 1);
		if ( ALLOCATE( temp_data, float *, number_of_frames ))
		{
			
			/* Set up the temporary array */
			for ( frame = 0 ; frame < number_of_frames ; frame++ )
			{
				time = emoter_dialog->time_minimum + frame;
					emoter_set_play_slider_value ( emoter_dialog, time );
					if (!( temp_data[frame] = emoter_sum_all_modes(
						emoter_dialog->shared )))
					{
						display_message(ERROR_MESSAGE,
							"emoter_convert_raw_CB.  Could not get mode vector for frame %d", frame);
					}
				}

				/* Destroy the current mode curve or create a new name*/
				if ( active->mode_curve )
				{
					GET_NAME(Control_curve)(active->mode_curve,&var_name );
					temp_var = active->mode_curve;
					DEACCESS(Control_curve)(&active->mode_curve);
					REMOVE_OBJECT_FROM_MANAGER(Control_curve)
						(temp_var, active->shared->control_curve_manager);
				}
				else
				{
					ALLOCATE(var_name, char, strlen(active->name) + 1);
					strcpy( var_name, active->name );
				}
				while (FIND_BY_IDENTIFIER_IN_MANAGER(Control_curve,name)(
					var_name, shared->control_curve_manager))
				{
					REALLOCATE(var_name, var_name, char, strlen(var_name)+2);
					strcat(var_name,"+");
				}

				/* Create new mode curve */
				frame = 0;
				time = emoter_dialog->time_minimum;
				if ( shared->show_solid_body_motion )
				{
					active->solid_body_motion = SOLID_BODY_MODES;
					solid_body_modes = active->solid_body_motion;
					mode_offset = 0;
				}
				else
				{
					solid_body_modes = 0;
					mode_offset = SOLID_BODY_MODES;
				}

				if (active->mode_curve = CREATE(Control_curve)(var_name,
						LINEAR_LAGRANGE, shared->number_of_modes + solid_body_modes ))
				{
					if (ADD_OBJECT_TO_MANAGER(Control_curve)(
						active->mode_curve, shared->control_curve_manager))
					{
						/* Set the type after the object is added to the
							manager so that ADD message is ignored */
						Control_curve_set_type(active->mode_curve,
							CONTROL_CURVE_TYPE_EMOTER_MODES );
						ACCESS(Control_curve)(active->mode_curve);
						/* SAB Do the first and second nodes as special case
							as we need to create the first element, position both
							nodes and then from then on the current elements can
							just be appended to */
						Control_curve_add_element( active->mode_curve, 1 );
						Control_curve_set_parameter( active->mode_curve,
							/*element_no*/1,/*local_node_no*/0,	time);
						time += 1.0;
						Control_curve_set_parameter( active->mode_curve,
							/*element_no*/1,/*local_node_no*/1,	time);
						if ( temp_data[frame] )
						{
							Control_curve_set_node_values( active->mode_curve,
								1, 0, temp_data[frame] + mode_offset);
						}
						if ( number_of_frames > 1 )
						{
							frame++;
						}

						if ( temp_data[frame] )
						{
							Control_curve_set_node_values( active->mode_curve,
								1, 1, temp_data[frame] + mode_offset);
						}

						frame = 2;
						while (frame < number_of_frames)
						{
							Control_curve_add_element( active->mode_curve, frame );
							time += 1.0;
							Control_curve_set_parameter( active->mode_curve,
								/*element_no*/frame,/*local_node_no*/1,	time);
							if ( temp_data[frame] )
							{
								Control_curve_set_node_values( active->mode_curve,
									frame, 1, temp_data[frame] + mode_offset);
							}
							frame++;
						}
					}
					else
					{
						DESTROY(Control_curve)(&active->mode_curve);
						display_message(ERROR_MESSAGE,
							"read_emoter_mode_Control_curve.  Unable to add control curve to manager");
						return_code = 0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
								"emoter_convert_active_slider.  Unable to create control curve");
					return_code = 0;
				}

				/* DEALLOCATE temp_data */
				for ( frame = 0 ; frame < number_of_frames ; frame++ )
				{
					if ( temp_data[frame] )
					{
						DEALLOCATE( temp_data[frame] );
					}
				}
				DEALLOCATE( temp_data );
				DEALLOCATE( var_name );

				/* Remove all combine sliders and set active slider to 1.0 */
				emoter_set_slider_value ( active, 1.0 );
				for ( i = 0 ; i < active->number_of_combine_sliders ; i++ )
				{
					emoter_slider_animated( (active->combine_sliders[i])->slider, 0 );
					temp_var = (active->combine_sliders[i])->curve;
					DEACCESS(Control_curve)( &((active->combine_sliders[i])->curve));
					REMOVE_OBJECT_FROM_MANAGER(Control_curve)
						(temp_var, shared->control_curve_manager);

					combine_slider = (active->combine_sliders[i])->slider;
					if ( temp_var = (active->combine_sliders[i])->timebase_curve)
					{
						j = 0;
						while ( j < combine_slider->number_of_timebase_curves
							&& temp_var != combine_slider->timebase_curves[j])
						{
							j++;
						}
						if ( j < combine_slider->number_of_timebase_curves )
						{
							combine_slider->number_of_timebase_curves--;
							while ( j < combine_slider->number_of_timebase_curves )
							{
								combine_slider->timebase_curves[j] =
									combine_slider->timebase_curves[j + 1];
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"emoter_convert_active_slider.  Unable to find timebase curve reference in combine curve");
						}
					}
					temp_var = (active->combine_sliders[i])->timebase_curve;
					DEACCESS(Control_curve)( &((active->combine_sliders[i])->timebase_curve));
					REMOVE_OBJECT_FROM_MANAGER(Control_curve)
						(temp_var, shared->control_curve_manager);
					DEALLOCATE( active->combine_sliders[i] );
				}
				active->number_of_combine_sliders = 0;
				emoter_set_marker_value(active->emoter_markers[0], emoter_dialog->time_minimum );
				emoter_set_marker_value(active->emoter_markers[active->number_of_emoter_markers-1], emoter_dialog->time_maximum );
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"emoter_convert_active_slider.  Unable to allocate raw data array");
				return_code = 0;
			}
			busy_cursor_off((Widget)NULL, emoter_dialog->shared->user_interface );
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"emoter_convert_active_slider.  Could not get emoter_dialog");
		return_code = 0;
	}

	return(return_code);
	LEAVE;
} /* emoter_convert_active_slider */


static void emoter_convert_raw_CB(Widget widget,XtPointer user_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 15 April 1998

DESCRIPTION :

==============================================================================*/
{
	char warning[100];
	struct Emoter_dialog *emoter_dialog;

	ENTER(emoter_convert_raw_CB);
	USE_PARAMETER(call_data);
	USE_PARAMETER(user_data);
	/* Get the pointer to the emoter_dialog structure */
	XtVaGetValues(widget,
		XmNuserData, &emoter_dialog,
		NULL);

	if ( emoter_dialog && emoter_dialog->shared && emoter_dialog->shared->active_slider)
	{
		sprintf( warning, "Converting slider %s to raw data %6.1f:%6.1f. All history will be lost",
			emoter_dialog->shared->active_slider->name,
			emoter_dialog->time_minimum, emoter_dialog->time_maximum );
		if ( confirmation_warning_ok_cancel("Convert to Raw Data",
			warning, emoter_dialog->shell, emoter_dialog->shared->user_interface))
		{
			emoter_convert_active_slider(emoter_dialog);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"emoter_convert_raw_CB.  Could not get emoter_dialog");
	}

	LEAVE;
} /* emoter_convert_raw_CB */

static void emoter_newslider_CB(Widget widget,XtPointer user_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 14 April 1998

DESCRIPTION :

==============================================================================*/
{
	struct Emoter_dialog *emoter_dialog;
	struct Shared_emoter_slider_data *shared;

	ENTER(emoter_newslider_CB);
	USE_PARAMETER(call_data);
	USE_PARAMETER(user_data);
	/* Get the pointer to the emoter_dialog structure */
	XtVaGetValues(widget,
		XmNuserData, &emoter_dialog,
		NULL);

	if ( emoter_dialog )
	{
		shared = emoter_dialog->shared;

		create_emoter_slider((char *)NULL,
			"new", emoter_dialog->slider_form,
			1, shared,
			shared->number_of_sliders,
			(struct Control_curve *)NULL,
			emoter_dialog, /*no_confirm*/0);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"emoter_newslider_CB.  Could not get emoter_dialog");
	}

	LEAVE;
} /* emoter_newsslider_CB */

static void emoter_add_marker_CB(Widget widget,XtPointer user_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 8 January 1999

DESCRIPTION :
==============================================================================*/
{
	char *name;
	int i;
	struct Emoter_marker *marker, *new_marker;
	struct Emoter_slider *slider;
	struct Shared_emoter_slider_data *shared;

	ENTER(emoter_add_marker_CB);
	USE_PARAMETER(call_data);
	USE_PARAMETER(user_data);
	/* Get the pointer to the emoter_dialog structure */
	XtVaGetValues(widget,
		XmNuserData, &marker,
		NULL);

	if ( marker && marker->shared &&
		marker->slider_parent &&
		marker->shared->active_slider )
	{
		shared = marker->shared;
		slider = marker->slider_parent;


		if ( name = confirmation_get_string( "Add Marker",
			"Specify name for new marker",(char *)NULL,
			User_interface_get_application_shell(marker->shared->user_interface),
			marker->shared->user_interface ))
		{
			new_marker = create_emoter_marker(name,
				marker->value, slider->marker_rowcol,
				marker->index + 1, marker->element_index+1, 0, 0,
				slider, shared );

			DEALLOCATE( name );

			for ( i = 0 ; i < slider->number_of_emoter_markers ; i++ )
			{
				if ( (slider->emoter_markers[i])->index > marker->index )
				{
					(slider->emoter_markers[i])->index++;
					(slider->emoter_markers[i])->element_index++;
				}
			}

			/* Add the node to any existing control curves */
			for ( i = 0 ; i < slider->number_of_timebase_curves ; i++ )
			{
				Control_curve_subdivide_element(
					(slider->timebase_curves[i]),
					marker->element_index, 0);
			}

			if ( REALLOCATE( slider->emoter_markers,
				slider->emoter_markers, struct Emoter_marker *,
				slider->number_of_emoter_markers + 1))
			{
				for ( i = marker->index + 1 ; i < slider->number_of_emoter_markers ; i++)
				{
					slider->emoter_markers[i+1]
						= slider->emoter_markers[i];
				}

				slider->emoter_markers[marker->index+1]
					= new_marker;
				slider->number_of_emoter_markers++;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"emoter_add_marker_CB.  Unable to Reallocate marker array");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"emoter_add_marker_CB.  Could not get emoter_dialog");
	}

	LEAVE;
} /* emoter_newsslider_CB */

static void emoter_keyframe_CB(Widget widget,XtPointer user_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 17 November 1999

DESCRIPTION :

==============================================================================*/
{
	char *name;
	int i, combine_index, element, local_node_no, number_of_elements, return_code;
	float end_time, start_time, time, time_change, value, xi;
	struct Emoter_combine_slider *combine_slider, **new_combine_sliders;
	struct Emoter_dialog *emoter_dialog;
	struct Emoter_slider *active, *slider;
	struct Shared_emoter_slider_data *shared;
	struct Control_curve *curve, **new_timebase_curves;

	ENTER(emoter_keyframe_CB);
	USE_PARAMETER(call_data);
	USE_PARAMETER(user_data);
	/* Get the pointer to the emoter_dialog structure */
	XtVaGetValues(widget,
		XmNuserData, &emoter_dialog,
		NULL);

	if ( emoter_dialog && emoter_dialog->shared )
	{
		shared = emoter_dialog->shared;
		time = shared->time;

		if ( active = emoter_dialog->shared->active_slider )
		{
			for (i = 0 ; i < shared->number_of_sliders ; i++ )
			{
				slider = shared->sliders[i];
				/* Is this slider selected for addition of keyframes */
				if ( active != slider && slider->selected )
				{
					/* Deselect this slider */
					emoter_slider_select( slider, 0 );

					/* Show animated icon if not already */
					emoter_slider_animated( slider, 1 );

					/* Check each selected slider to see if it is
						included in the combine sliders set */
					combine_index = 0;
					while ( combine_index < active->number_of_combine_sliders &&
						active->combine_sliders[combine_index]->slider != slider )
					{
						combine_index++;
					}
					if (combine_index < active->number_of_combine_sliders)
					{
						/* Slider is already in combine sliders so set or add keyframe */
						if ( curve = active->combine_sliders[combine_index]->curve )
						{
							if ( 1 == Control_curve_get_number_of_components(curve))
							{
								value = slider->value;
								Control_curve_get_parameter_range(curve, &start_time ,
									&end_time);
								/* Assume first element was made when curve was created */
								if ( time >= start_time)
								{
									if ( time <= end_time)
									{
										/* If time is within already defined values adjust existing
											node or subdivide a current element */
										if ( Control_curve_find_node_at_parameter( curve, time,
											&element, &local_node_no ))
										{
											Control_curve_set_node_values( curve,
												element, local_node_no, &value );
										}
										else
										{
											if ( Control_curve_find_element_at_parameter( curve, time,
												&element, &xi ))
											{
												Control_curve_subdivide_element( curve,
													element, xi );
												Control_curve_set_node_values( curve,
													element + 1, 0, &value );
											}
											else
											{
												display_message(ERROR_MESSAGE,
													"emoter_keyframe_CB.  Unable to subdivide element");
											}
										}
									}
									else
									{
										/* If there is only one element at a single point shift
											the second node, otherwise add an element at end */
										number_of_elements = Control_curve_get_number_of_elements( curve );
										Control_curve_get_element_parameter_change( curve,
											1, &time_change );
										if ( number_of_elements == 1 && time_change == 0.0 )
										{
											local_node_no = Control_curve_get_nodes_per_element (curve) - 1;
											Control_curve_set_parameter( curve,
												1, local_node_no, time );
											Control_curve_set_node_values ( curve,
												1, local_node_no, &value );
										}
										else
										{
											Control_curve_add_element( curve, number_of_elements + 1 );
											local_node_no = Control_curve_get_nodes_per_element (curve) - 1;
											Control_curve_set_parameter( curve,
												number_of_elements + 1, local_node_no, time );
											Control_curve_set_node_values ( curve,
												number_of_elements + 1, local_node_no, &value );
										}
									}
								}
								else
								{
									/* If there is only one element at a single point shift
										the first node, otherwise add an element at start */
									number_of_elements = Control_curve_get_number_of_elements( curve );
									Control_curve_get_element_parameter_change( curve,
										1, &time_change );
									if ( number_of_elements == 1 && time_change == 0.0 )
									{
										local_node_no = 0;
										Control_curve_set_parameter( curve,
											1, local_node_no, time );
										Control_curve_set_node_values ( curve,
											1, local_node_no, &value );
									}
									else
									{
										Control_curve_add_element( curve, 1 );
										local_node_no = 0;
										Control_curve_set_parameter( curve,
											1, local_node_no, time );
										Control_curve_set_node_values ( curve,
											1, local_node_no, &value );
									}
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"emoter_keyframe_CB.  Combine curve has more than one component");
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"emoter_keyframe_CB.  Combine curve not found when expected");
						}
					}
					else
					{
						/* Need to add new combine slider */
						if ( REALLOCATE( new_combine_sliders, active->combine_sliders,
							struct Emoter_combine_slider *, active->number_of_combine_sliders + 1))
						{
							return_code = 0;
							active->combine_sliders = new_combine_sliders;
							active->number_of_combine_sliders++;
							if ( ALLOCATE ( combine_slider, struct Emoter_combine_slider, 1))
							{
								active->combine_sliders[active->number_of_combine_sliders-1]
									= combine_slider;
								combine_slider->slider = slider;

								if ( ALLOCATE( name, char, strlen(active->name) +
									strlen(slider->name) + 10))
								{
									sprintf(name, "%s in %s", slider->name, active->name );
									while (FIND_BY_IDENTIFIER_IN_MANAGER(Control_curve,name)(
										name, shared->control_curve_manager))
									{
										REALLOCATE(name, name, char, strlen(name)+2);
										strcat(name,"+");
									}
									if ( curve = CREATE(Control_curve)(
										name, CUBIC_HERMITE, 1 ))
									{
										Control_curve_set_type(curve,
											CONTROL_CURVE_TYPE_EMOTER_COMBINE );
										combine_slider->curve = curve;
										if (ADD_OBJECT_TO_MANAGER(Control_curve)(
											curve, shared->control_curve_manager))
										{
											ACCESS(Control_curve)(curve);
											Control_curve_add_element( curve, 1 );
											local_node_no = 0;
											Control_curve_set_parameter( curve,
												/*element_no*/1,/*local_node_no*/0,	time);
											Control_curve_set_parameter( curve,
												/*element_no*/1,/*local_node_no*/
												Control_curve_get_nodes_per_element(curve)-1,	time);

											value = slider->value;
											for (local_node_no = 0 ;
													local_node_no < Control_curve_get_nodes_per_element (curve)
													; local_node_no++ )
											{
												Control_curve_set_node_values ( curve,
													1, local_node_no, &value );
												return_code = 1;
											}
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"emoter_keyframe_CB.  Unable to add combine curve to manager");
											DESTROY(Control_curve)(&curve);
											active->number_of_combine_sliders--;
											DEALLOCATE( combine_slider );
										}
										DEALLOCATE( name );
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"emoter_keyframe_CB.  Could not create combine slider curve");
										active->number_of_combine_sliders--;
										DEALLOCATE( combine_slider );
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"emoter_keyframe_CB.  Could not allocate combine curve name");
									active->number_of_combine_sliders--;
									DEALLOCATE( combine_slider );
								}
								if ( return_code )
								{
									/* Create the timebase curve */
									if ( REALLOCATE( new_timebase_curves, slider->timebase_curves,
										struct Control_curve *, slider->number_of_timebase_curves + 1))
									{
										slider->timebase_curves = new_timebase_curves;
										slider->number_of_timebase_curves++;
										if ( ALLOCATE( name, char, strlen(slider->name) +
											strlen(active->name) + 15))
										{
											sprintf(name, "%s in %s timebase", slider->name, active->name );
											while (FIND_BY_IDENTIFIER_IN_MANAGER(Control_curve,name)(
												name, shared->control_curve_manager))
											{
												REALLOCATE(name, name, char, strlen(name)+2);
												strcat(name,"+");
											}
											if ( curve = CREATE(Control_curve)(
												name, LINEAR_LAGRANGE, 1 ))
											{
												Control_curve_set_type(curve,
													CONTROL_CURVE_TYPE_EMOTER_TIMEBASE );
												combine_slider->timebase_curve = curve;
												slider->timebase_curves[slider->number_of_timebase_curves-1]
													= curve;
												if (ADD_OBJECT_TO_MANAGER(Control_curve)(
													curve, shared->control_curve_manager))
												{
													ACCESS(Control_curve)(curve);
													if ( slider->number_of_emoter_markers > 1)
													{
#if defined (OLD_CODE)
														Control_curve_set_start_time( curve,
															(slider->emoter_markers[0])->value );
#endif /* defined (OLD_CODE) */
														for (element = 0 ;
															element < slider->number_of_emoter_markers -1
															; element++ )
														{
															value = (slider->emoter_markers[element])->value;
															Control_curve_add_element( curve, element + 1 );

															Control_curve_set_parameter( curve, element + 1,
																1, value );
															Control_curve_set_parameter( curve, element + 1,
																0, value );
															Control_curve_set_node_values ( curve,
																element + 1, 0, &value );
															printf("  element %d, value %f\n", element, value);
														}
														value = (slider->emoter_markers[element])->value;
														Control_curve_set_parameter( curve, element,
															1, value );
														Control_curve_set_node_values ( curve,
															element, 1, &value );
														printf("  element %d, value %f\n", element, value);
													}
												}
												else
												{
													display_message(ERROR_MESSAGE,
														"emoter_keyframe_CB.  Unable to add timebase curve to manager");
													DESTROY(Control_curve)(&curve);
													slider->number_of_timebase_curves--;
												}
												DEALLOCATE( name );
											}
											else
											{
												display_message(ERROR_MESSAGE,
													"emoter_keyframe_CB.  Could not create timebase curve");
												slider->number_of_timebase_curves--;
											}
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"emoter_keyframe_CB.  Could not allocate timebase curve name");
											slider->number_of_timebase_curves--;
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"emoter_keyframe_CB.  Could not reallocate timebase curve pointer array");
									}
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"emoter_keyframe_CB.  Could not allocate combine slider structure");
								active->number_of_combine_sliders--;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"emoter_keyframe_CB.  Could not reallocate combine sliders pointer array");
						}
					}
				}
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"emoter_keyframe_CB.  Cannot set keyframes when no slider is active");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"emoter_keyframe_CB.  Could not get emoter_dialog");
	}

	LEAVE;
} /* emoter_keyframe_CB */

static void emoter_play_value_text_CB(Widget widget,XtPointer user_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 14 April 1998

DESCRIPTION :
Handles input from the text widget which displays the current frame timecode
==============================================================================*/
{
	char *text_string;
	float value;
	struct Emoter_dialog *emoter_dialog;

	ENTER(emoter_play_value_text_CB);
	USE_PARAMETER(call_data);
	USE_PARAMETER(user_data);
	/* Get the pointer to the emoter_dialog structure */
	XtVaGetValues(widget,
		XmNuserData, &emoter_dialog,
		NULL);

	if ( emoter_dialog )
	{
		if ( text_string = XmTextGetString(widget))
		{
			sscanf(text_string, "%f", &value );
			emoter_set_play_slider_value( emoter_dialog, value );
			emoter_update_face( emoter_dialog->shared );
			XtFree ( text_string );
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"emoter_play_value_text_CB.  Could not get emoter_dialog");
	}

	LEAVE;
} /* emoter_play_value_text_CB */

static void emoter_play_range_text_CB(Widget widget,XtPointer user_data,
	XtPointer call_data)
/*******************************************************************************
LAST MODIFIED : 14 April 1998

DESCRIPTION :
Handles input from the text widget which displays the current frame timecode
==============================================================================*/
{
	char *text_string_min, *text_string_max;
	float max, min;
	struct Emoter_dialog *emoter_dialog;

	ENTER(emoter_play_max_text_CB);
	USE_PARAMETER(call_data);
	USE_PARAMETER(user_data);
	/* Get the pointer to the emoter_dialog structure */
	XtVaGetValues(widget,
		XmNuserData, &emoter_dialog,
		NULL);

	if ( emoter_dialog )
	{
		if ( emoter_dialog->play_max_text && emoter_dialog->play_min_text )
		{
			if ( text_string_max = XmTextGetString(emoter_dialog->play_max_text))
			{
				sscanf(text_string_max, "%f", &max );
				if ( text_string_min = XmTextGetString(emoter_dialog->play_min_text))
				{
					sscanf(text_string_min, "%f", &min );
					emoter_set_play_slider_range( emoter_dialog, min, max );
					emoter_update_face( emoter_dialog->shared );
					XtFree ( text_string_min );
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"emoter_play_range_text_CB.  Unable to get min text string");
				}
				XtFree ( text_string_max );
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"emoter_play_range_text_CB.  Unable to get max text string");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"emoter_play_max_text_CB.  Could not get emoter_dialog");
	}

	LEAVE;
} /* emoter_play_max_text_CB */

static int Control_curve_update_emoter_sliders(
	struct Control_curve *control_curve, void *emoter_dialog_void)
/*******************************************************************************
LAST MODIFIED : 30 May 2001

DESCRIPTION :
Updates existing control curves combine sliders.
==============================================================================*/
{
	enum Control_curve_type type;
	int return_code;
	struct Emoter_dialog *emoter_dialog;

	ENTER(Control_curve_update_emoter_sliders);
	if (control_curve &&
		(emoter_dialog = (struct Emoter_dialog *)emoter_dialog_void))
	{
		Control_curve_get_type( control_curve, &type );
		if (type == CONTROL_CURVE_TYPE_EMOTER_COMBINE)
		{
			/* Setting the time slider to the time it already has
				 updates all the combine sliders */
			emoter_update_combine_sliders( emoter_dialog->shared->active_slider );
		}
		else
		{
			emoter_update_face( emoter_dialog->shared );
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Control_curve_update_emoter_sliders.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Control_curve_update_emoter_sliders */

static int Control_curve_add_emoter_sliders(
	struct Control_curve *control_curve, void *emoter_dialog_void)
/*******************************************************************************
LAST MODIFIED : 30 May 2001

DESCRIPTION :
Adds new control curves combine sliders.
==============================================================================*/
{
	char *name;
	enum Control_curve_type type;
	int return_code;
	struct Emoter_dialog *emoter_dialog;
	struct Shared_emoter_slider_data *shared;

	ENTER(Control_curve_add_emoter_sliders);
	if (control_curve &&
		(emoter_dialog = (struct Emoter_dialog *)emoter_dialog_void))
	{
		shared = emoter_dialog->shared;

		Control_curve_get_type( control_curve, &type );
		if ( type == CONTROL_CURVE_TYPE_EMOTER_MODES )
		{
			if (GET_NAME(Control_curve)( control_curve, &name ))
			{
				create_emoter_slider((char *)NULL,
					name, emoter_dialog->slider_form,
					1.0, shared,
					shared->number_of_sliders,
					control_curve, emoter_dialog, /*no_confirm*/0);
				DEALLOCATE( name );
			}
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Control_curve_add_emoter_sliders.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Control_curve_add_emoter_sliders */

static void emoter_control_curve_manager_message(
	struct MANAGER_MESSAGE(Control_curve) *message, void *emoter_dialog_void)
/*******************************************************************************
LAST MODIFIED : 30 May 2001

DESCRIPTION :
Something has changed globally in the control curve manager.
==============================================================================*/
{
	ENTER(emoter_control_curve_manager_message);
	if (message && emoter_dialog_void)
	{
		switch (message->change)
		{
			case MANAGER_CHANGE_OBJECT(Control_curve):
			case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Control_curve):
			{
				FOR_EACH_OBJECT_IN_LIST(Control_curve)(
					Control_curve_update_emoter_sliders, emoter_dialog_void,
					message->changed_object_list);
			} break;
			case MANAGER_CHANGE_ADD(Control_curve):
			{
				FOR_EACH_OBJECT_IN_LIST(Control_curve)(
					Control_curve_add_emoter_sliders, emoter_dialog_void,
					message->changed_object_list);
			} break;
			case MANAGER_CHANGE_REMOVE(Control_curve):
			case MANAGER_CHANGE_IDENTIFIER(Control_curve):
			{
				/* do nothing */
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"emoter_control_curve_manager_message.  Invalid argument(s)");
	}
	LEAVE;
} /* emoter_control_curve_manager_message */

static Widget create_emoter_dialog(Widget *emoter_dialog_address,
	Widget parent, struct Shared_emoter_slider_data *shared_data)
/*******************************************************************************
LAST MODIFIED : 15 June 1998

DESCRIPTION :
Create emoter controls.
==============================================================================*/
{
	MrmType emoter_dialog_class;
	struct Emoter_dialog *emoter_dialog = NULL;
	static MrmRegisterArg callback_list[]=
	{
		{"emoter_dialog_destroy_CB",(XtPointer)emoter_dialog_destroy_CB},
		{"emoter_id_play_slider",(XtPointer)emoter_id_play_slider},
		{"emoter_play_slider_CB",(XtPointer)emoter_play_slider_CB},
		{"emoter_loadslider_CB",(XtPointer)emoter_loadslider_CB},
		{"emoter_saveslider_CB",(XtPointer)emoter_saveslider_CB},
		{"emoter_exportslider_CB",(XtPointer)emoter_exportslider_CB},
		{"emoter_newslider_CB",(XtPointer)emoter_newslider_CB},
		{"emoter_add_marker_CB",(XtPointer)emoter_add_marker_CB},
		{"emoter_keyframe_CB",(XtPointer)emoter_keyframe_CB},
		{"emoter_id_slider_form",(XtPointer)
			DIALOG_IDENTIFY(emoter_dialog, slider_form)},
		{"emoter_id_play_value_text",(XtPointer)
			DIALOG_IDENTIFY(emoter_dialog, play_value_text)},
		{"emoter_play_value_text_CB",(XtPointer)
			emoter_play_value_text_CB},
		{"emoter_id_play_max_text",(XtPointer)
			DIALOG_IDENTIFY(emoter_dialog, play_max_text)},
		{"emoter_id_play_min_text",(XtPointer)
			DIALOG_IDENTIFY(emoter_dialog, play_min_text)},
		{"emoter_id_play_button",(XtPointer)
			DIALOG_IDENTIFY(emoter_dialog, play_button)},
		{"emoter_id_input_seq_button",(XtPointer)
			DIALOG_IDENTIFY(emoter_dialog, input_sequence_button)},
		{"emoter_id_mode_subform",(XtPointer)
			DIALOG_IDENTIFY(emoter_dialog, mode_subform)},
		{"emoter_id_mode_text",(XtPointer)
			DIALOG_IDENTIFY(emoter_dialog, mode_text)},
		{"emoter_id_solid_motion_button",(XtPointer)
			DIALOG_IDENTIFY(emoter_dialog, solid_motion_button)},
		{"emoter_id_movie_control_form",(XtPointer)
			DIALOG_IDENTIFY(emoter_dialog, movie_control_form)},
		{"emoter_id_movie_framerate_text",(XtPointer)
			DIALOG_IDENTIFY(emoter_dialog, movie_framerate_text)},
		{"emoter_play_range_text_CB",(XtPointer)
			emoter_play_range_text_CB},
		{"emoter_playbutton_CB",(XtPointer)emoter_playbutton_CB},
		{"emoter_mode_show_CB",(XtPointer)emoter_mode_show_CB},
		{"emoter_mode_text_CB",(XtPointer)emoter_mode_text_CB},
		{"emoter_mode_up_CB",(XtPointer)emoter_mode_up_CB},
		{"emoter_mode_down_CB",(XtPointer)emoter_mode_down_CB},
		{"emoter_solid_motion_CB",(XtPointer)emoter_solid_motion_CB},
		{"emoter_input_sequence_CB",(XtPointer)emoter_input_sequence_CB},
		{"emoter_movie_play_CB",(XtPointer)emoter_movie_play_CB},
		{"emoter_movie_loop_CB",(XtPointer)emoter_movie_loop_CB},
		{"emoter_movie_every_frame_CB",(XtPointer)emoter_movie_every_frame_CB},
		{"emoter_movie_framerate_CB",(XtPointer)emoter_movie_framerate_CB},
		{"emoter_convert_raw_CB",(XtPointer)emoter_convert_raw_CB}
	};
	static MrmRegisterArg identifier_list[] =
	{
		{"emoter_dialog_structure",(XtPointer)NULL},
	};
	Widget return_widget;

	ENTER(create_emoter_dialog);

	return_widget = (Widget)NULL;
	if (MrmOpenHierarchy_base64_string(emoter_dialog_uidh,
		&emoter_dialog_hierarchy,&emoter_dialog_hierarchy_open))
	{
		/* allocate memory */
		if (ALLOCATE(emoter_dialog, struct Emoter_dialog, 1))
		{
			/* initialise the structure */
			emoter_dialog->shared = shared_data;
			emoter_dialog->dialog_address = emoter_dialog_address;
			emoter_dialog->widget = NULL;
			emoter_dialog->time_minimum = 1;
			emoter_dialog->time_maximum = 400;
			emoter_dialog->play_slider = (Widget)NULL;
			emoter_dialog->slider_form = (Widget)NULL;
			emoter_dialog->mode_subform = (Widget)NULL;
			emoter_dialog->mode_text = (Widget)NULL;
			emoter_dialog->solid_motion_button = (Widget)NULL;
			emoter_dialog->play_min_text = (Widget)NULL;
			emoter_dialog->play_max_text = (Widget)NULL;
			emoter_dialog->play_button = (Widget)NULL;
			emoter_dialog->input_sequence_button = (Widget)NULL;
			emoter_dialog->play_slider_position = -1;
			emoter_dialog->control_curve_manager_callback_id = (void *)NULL;
			emoter_dialog->movie = (struct Movie_graphics *)NULL;
			emoter_dialog->movie_control_form = (Widget)NULL;
			emoter_dialog->movie_framerate_text = (Widget)NULL;
			emoter_dialog->movie_loop = 1;
			emoter_dialog->movie_every_frame = 1;
			emoter_dialog->minimum_nodegroup = (struct GROUP(FE_node) *)NULL;
			emoter_dialog->minimum_elementgroup = (struct GROUP(FE_element) *)NULL;
			emoter_dialog->shell_list_item = (struct Shell_list_item *)NULL;

			/* make the dialog shell */
			if (emoter_dialog->shell = XtVaCreatePopupShell(
				"The Emotionator",topLevelShellWidgetClass,parent,
				XmNallowShellResize,TRUE,NULL))
			{

				emoter_dialog->shell_list_item=
					create_Shell_list_item(&(emoter_dialog->shell),
					shared_data->user_interface);

				/* register the callbacks */
				if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
					emoter_dialog_hierarchy,callback_list,XtNumber(callback_list)))
				{
					/* assign and register the identifiers */
					identifier_list[0].value=(XtPointer)emoter_dialog;
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
						emoter_dialog_hierarchy,identifier_list,
						XtNumber(identifier_list)))
					{
						/* fetch position window widget */
						if (MrmSUCCESS==MrmFetchWidget(emoter_dialog_hierarchy,
							"emoter_dialog_widget", emoter_dialog->shell,
							&(emoter_dialog->widget),
							&emoter_dialog_class))
						{
							emoter_dialog->control_curve_manager_callback_id=
								MANAGER_REGISTER(Control_curve)(
								emoter_control_curve_manager_message,
								(void *)emoter_dialog,
								emoter_dialog->shared->control_curve_manager);

							XtManageChild(emoter_dialog->widget);

							emoter_set_play_slider_range( emoter_dialog,
								emoter_dialog->time_minimum,
								emoter_dialog->time_maximum );
							emoter_set_play_slider_value( emoter_dialog,
								shared_data->time );
							emoter_set_mode_limit( emoter_dialog,
								shared_data->mode_limit );

							XtRealizeWidget(emoter_dialog->shell);

							if ( emoter_dialog->mode_subform )
							{
								XtUnmanageChild( emoter_dialog->mode_subform );
							}
							if ( emoter_dialog->movie_control_form )
							{
								XtUnmanageChild( emoter_dialog->movie_control_form );
							}
							XtPopup(emoter_dialog->shell, XtGrabNone);

							*(emoter_dialog->dialog_address) = emoter_dialog->widget;
							return_widget = emoter_dialog->widget;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"create_emoter_dialog.  Could not fetch emoter_dialog dialog");
							DEALLOCATE(emoter_dialog);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"create_emoter_dialog.  Could not register identifiers");
						DEALLOCATE(emoter_dialog);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"create_emoter_dialog.  Could not register callbacks");
					DEALLOCATE(emoter_dialog);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"create_emoter_dialog.  Could not create popup shell.");
				DEALLOCATE(emoter_dialog);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"create_emoter_dialog.  Could not allocate emoter_dialog structure");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_emoter_dialog.  Could not open hierarchy");
	}
	*emoter_dialog_address = return_widget;
	LEAVE;

	return (return_widget);
} /* create_emoter_dialog */

int bring_up_emoter_dialog(Widget *emoter_dialog_address,
	Widget parent, struct Shared_emoter_slider_data *shared_data)
/*******************************************************************************
LAST MODIFIED : 6 April 1998

DESCRIPTION :
If there is a emoter dialog in existence, then bring it to the front, else
create a new one.  Assumes we will only ever want one emoter controller at
a time.  This implementation may be changed later.
==============================================================================*/
{
	int return_code;

	ENTER(bring_up_emoter_dialog);
	if (emoter_dialog_address)
	{
		/* does it exist */
		if (*emoter_dialog_address)
		{
			XtPopup(*emoter_dialog_address,XtGrabNone);
			return_code=1;
		}
		else
		{
			if (create_emoter_dialog(emoter_dialog_address,
				parent, shared_data))
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
			"bring_up_emoter_dialog.  Invalid arguments");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* bring_up_emoter_dialog */

struct Index_list_data
{
	int number_of_index_nodes;
	int maximum_index_nodes;
	int *index_nodes;
}; /* struct Index_list_data */

static int add_FE_node_number_to_list(struct FE_node *node,
	void *index_list_data_void)
/*******************************************************************************
LAST MODIFIED : 6 March 2000

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Index_list_data *index_list_data;

	ENTER(add_FE_node_number_to_list);
	if (node && (index_list_data = 
		(struct Index_list_data *)index_list_data_void))
	{
		if (index_list_data->number_of_index_nodes < 
			index_list_data->maximum_index_nodes)
		{
			index_list_data->index_nodes[index_list_data->number_of_index_nodes] =
				get_FE_node_cm_node_identifier(node);
			index_list_data->number_of_index_nodes++;
			return_code = 1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"add_FE_node_number_to_list.  Too many nodes for memory allocated.");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"add_FE_node_number_to_list.  Invalid arguments");
		return_code=0;
	}
	
	LEAVE;

	return (return_code);
} /* add_FE_node_number_to_list */
/*
Global functions
----------------
*/
int gfx_create_emoter(struct Parse_state *state,void *dummy_to_be_modified,
	void *create_emoter_slider_data_void)
/*******************************************************************************
LAST MODIFIED : 6 March 2000

DESCRIPTION :
Executes a GFX CREATE EMOTER command.  If there is a emoter dialog
in existence, then bring it to the front, otherwise create new one.
==============================================================================*/
{
	char *basis_file_name, minimum_nodeset_flag,
		*transformation_graphics_object_name;
	int i,*index_nodes,number_of_modes,number_of_nodes,number_of_index_nodes,
		return_code;
	struct Create_emoter_slider_data *create_emoter_slider_data;
	struct GROUP(FE_node) *node_group;
	struct Index_list_data index_list_data;
	struct MANAGER(FE_node) *node_manager;
	struct Scene_object *transformation_scene_object;
	struct Scene *transformation_scene;
	struct Shared_emoter_slider_data *shared_emoter_slider_data;
	struct EM_Object *em_object;
	static struct Modifier_entry option_table[]=
	{
		{"minimum_nodeset",NULL,NULL,set_char_flag},
		{"basis",NULL,(void *)1,set_name},
		{"ngroup",NULL,NULL,set_FE_node_group},
		{"transformation_graphics_object",NULL,(void *)1,set_name},
		{"transformation_scene",NULL,NULL,set_Scene},
		{NULL,NULL,NULL,set_name}
	};

	ENTER(gfx_create_emoter);
	USE_PARAMETER(dummy_to_be_modified);
	return_code=0;
	/* check arguments */
	if (state&&(create_emoter_slider_data=
		(struct Create_emoter_slider_data *)
		create_emoter_slider_data_void)&&
		(create_emoter_slider_data->emoter_slider_dialog_address)&&		
		(node_manager=create_emoter_slider_data->node_manager)&&
		(create_emoter_slider_data->element_manager)&&
		(create_emoter_slider_data->node_group_manager)&&
		(create_emoter_slider_data->control_curve_manager)&&
		(create_emoter_slider_data->execute_command))
	{
		index_nodes = (int *)NULL;
		number_of_index_nodes = 0;
		minimum_nodeset_flag = 0;
		basis_file_name = (char *)NULL;
		node_group = (struct GROUP(FE_node) *) NULL;
		transformation_graphics_object_name=(char *)NULL;
		transformation_scene=ACCESS(Scene)(create_emoter_slider_data->viewer_scene);
		transformation_scene_object = (struct Scene_object *)NULL;
		(option_table[0]).to_be_modified = &minimum_nodeset_flag;
		(option_table[1]).to_be_modified = &basis_file_name;
		(option_table[2]).to_be_modified = &node_group;
		(option_table[2]).user_data=
			create_emoter_slider_data->node_group_manager;
		(option_table[3]).to_be_modified = &transformation_graphics_object_name;
		(option_table[4]).to_be_modified = &transformation_scene;
		(option_table[4]).user_data=create_emoter_slider_data->scene_manager;
		(option_table[5]).to_be_modified = &basis_file_name;
		return_code=process_multiple_options(state,option_table);
		if (return_code)
		{
			if ((!create_emoter_slider_data->parent)||
				(!create_emoter_slider_data->user_interface))
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_emoter.  Missing user_interface or parent widget");
				return_code = 0;
			}
			if (!basis_file_name)
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_emoter.  Must specify basis_file_name");
				return_code = 0;
			}
			if (return_code)
			{
				if (transformation_scene && transformation_graphics_object_name)
				{
					if (!(transformation_scene_object = Scene_get_Scene_object_by_name(
						transformation_scene, transformation_graphics_object_name)))
					{
						display_message(ERROR_MESSAGE,
							"gfx_create_emoter.  Unable to find object %s in scene",
							transformation_graphics_object_name);
						return_code = 0;
					}
				}
				em_object=(struct EM_Object *)NULL;
				if (node_group)
				{
					number_of_index_nodes = NUMBER_IN_GROUP(FE_node)(node_group);
					if (ALLOCATE(index_nodes, int, number_of_index_nodes))
					{
						index_list_data.number_of_index_nodes = 0;
						index_list_data.maximum_index_nodes = number_of_index_nodes;
						index_list_data.index_nodes = index_nodes;
						FOR_EACH_OBJECT_IN_GROUP(FE_node)(add_FE_node_number_to_list,
							(void *)&index_list_data, node_group);
						if (index_list_data.number_of_index_nodes != 
							number_of_index_nodes)
						{
							display_message(ERROR_MESSAGE,
								"gfx_create_emoter.  Inconsistent index node group counts");
							number_of_index_nodes = index_list_data.number_of_index_nodes;
						}
					}
					else
					{
						number_of_index_nodes = 0;
					}
				}
				if (EM_read_basis(basis_file_name,&em_object,index_nodes,
					number_of_index_nodes))
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
							if (minimum_nodeset_flag)
							{
								EM_calculate_minimum_nodeset(em_object, number_of_modes);
							}
							/* create the shared EMOTER data */
							if (ALLOCATE(shared_emoter_slider_data,struct Shared_emoter_slider_data,
								1)&&ALLOCATE(shared_emoter_slider_data->weights,double,
								number_of_modes + SOLID_BODY_MODES)
								&&ALLOCATE(shared_emoter_slider_data->sliders,
								struct Emoter_slider *,1))
							{
								shared_emoter_slider_data->input_sequence = (char *)NULL;
								shared_emoter_slider_data->movie_playing = 0;
								shared_emoter_slider_data->number_of_sliders = 0;
								shared_emoter_slider_data->number_of_modes=number_of_modes;
								shared_emoter_slider_data->mode_limit = number_of_modes;
								shared_emoter_slider_data->show_solid_body_motion = 1;
								shared_emoter_slider_data->fe_field_manager = 
									create_emoter_slider_data->fe_field_manager;
								shared_emoter_slider_data->node_manager = node_manager;
								shared_emoter_slider_data->element_manager =
									create_emoter_slider_data->element_manager;
								shared_emoter_slider_data->control_curve_manager
									= create_emoter_slider_data->control_curve_manager;
								shared_emoter_slider_data->execute_command =
									create_emoter_slider_data->execute_command;
								shared_emoter_slider_data->em_object = em_object;
								shared_emoter_slider_data->active_slider =
									(struct Emoter_slider *)NULL;
								shared_emoter_slider_data->control_curve_editor_dialog_address =
									create_emoter_slider_data->control_curve_editor_dialog_address;
								shared_emoter_slider_data->top_level
									= create_emoter_slider_data->parent;
								shared_emoter_slider_data->time = 1;
								shared_emoter_slider_data->user_interface
									= create_emoter_slider_data->user_interface;
								shared_emoter_slider_data->element_group_manager
									= create_emoter_slider_data->element_group_manager;
								shared_emoter_slider_data->node_group_manager
									= create_emoter_slider_data->node_group_manager;
								shared_emoter_slider_data->data_group_manager
									= create_emoter_slider_data->data_group_manager;
								shared_emoter_slider_data->graphics_window_manager
									= create_emoter_slider_data->graphics_window_manager;
								shared_emoter_slider_data->viewer_scene
									= create_emoter_slider_data->viewer_scene;
								shared_emoter_slider_data->viewer_background_colour
									= create_emoter_slider_data->viewer_background_colour;
								shared_emoter_slider_data->viewer_light
									= create_emoter_slider_data->viewer_light;
								shared_emoter_slider_data->viewer_light_model
									= create_emoter_slider_data->viewer_light_model;
								shared_emoter_slider_data->transformation_scene_object
									= transformation_scene_object;
								return_code=bring_up_emoter_dialog(
									create_emoter_slider_data->emoter_slider_dialog_address,
									create_emoter_slider_data->parent,
									shared_emoter_slider_data);
								return_code=1;
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"create_emoter_sliders.  Could not allocate memory for shared data");
								DEALLOCATE(shared_emoter_slider_data);
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
				if (index_nodes)
				{
					DEALLOCATE(index_nodes);
				}
			}
		}
		if (transformation_scene)
		{
			DEACCESS(Scene)(&transformation_scene);
		}
		if (node_group)
		{
			DEACCESS(GROUP(FE_node))(&node_group);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"create_emoter_sliders.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* create_emoter */

int gfx_modify_emoter(struct Parse_state *state,
	void *dummy_to_be_modified, void *emoter_dialog_widget_void)
/*******************************************************************************
LAST MODIFIED : 7 September 1999

DESCRIPTION :
Executes a GFX MODIFY EMOTER command.
==============================================================================*/
{
	char activate, convert_data, *export_filename, *filename, 
		*input_sequence, keyframe, maximum_time_flag, minimum_time_flag, 
		*movie_filename, new, no_rigid_body_motion, play, rigid_body_motion,
		*save_filename, *slidername, stop, *temp_filename, time_flag, value_flag;
	float maximum_time, minimum_time, time, value;
	int face_changed, i, integer_time, modes, return_code;
	static struct Modifier_entry option_table[]=
	{
		{"activate",NULL,NULL,set_char_flag},
		{"convert_data",NULL,NULL,set_char_flag},
		{"create_movie",NULL,(void *)1,set_name},
		{"export_nodes",NULL,(void *)1,set_name},
		{"input_sequence",NULL,(void *)1,set_name},
		{"keyframe",NULL,NULL,set_char_flag},
		{"load",NULL,(void *)1,set_name},
		{"new",NULL,NULL,set_char_flag},
		{"no_rigid_body_motion",NULL,NULL,set_char_flag},
		{"number_of_modes",NULL,NULL,set_int_positive},
		{"play",NULL,NULL,set_char_flag},
		{"rigid_body_motion",NULL,NULL,set_char_flag},
		{"save",NULL,(void *)1,set_name},
		{"set_maximum_time",NULL,NULL,set_float_and_char_flag},
		{"set_minimum_time",NULL,NULL,set_float_and_char_flag},
		{"set_time",NULL,NULL,set_float_and_char_flag},
		{"set_value",NULL,NULL,set_float_and_char_flag},
		{"slider",NULL,(void *)1,set_name},
		{"stop",NULL,NULL,set_char_flag},
		{NULL,NULL,NULL,NULL}
	};
	struct Emoter_dialog *emoter_dialog;
	struct Emoter_slider *emoter_slider;
	struct Graphics_window *graphics_window;
	struct Shared_emoter_slider_data *shared;
	Widget emoter_dialog_widget;

	ENTER(gfx_modify_emoter);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		if (emoter_dialog_widget = (Widget)emoter_dialog_widget_void)
		{
			/* Get the pointer to the emoter_dialog structure */
			XtVaGetValues(emoter_dialog_widget,
				XmNuserData, &emoter_dialog,
				NULL);
			maximum_time = emoter_dialog->time_maximum;
			minimum_time = emoter_dialog->time_minimum;
			shared = emoter_dialog->shared;
			modes = emoter_dialog->shared->mode_limit;
		}
		else
		{
			emoter_dialog = (struct Emoter_dialog *)NULL;
			maximum_time = 0.0;
			minimum_time = 0.0;
			shared = (struct Shared_emoter_slider_data *)NULL;
			modes = 1;
		} 

		/* initialise defaults */
		activate = 0;
		convert_data = 0;
		export_filename = (char *)NULL;
		face_changed = 0;
		filename = (char *)NULL;
		input_sequence = (char *)NULL;
		keyframe = 0;
		maximum_time_flag = 0;
		minimum_time_flag = 0;
		movie_filename = (char *)NULL;
		new = 0;
		no_rigid_body_motion = 0;
		play = 0;
		rigid_body_motion = 0;
		save_filename = (char *)NULL;
		slidername = (char *)NULL;
		stop = 0;
		time = 0.0;
		time_flag = 0;
		value = 1.0;
		value_flag = 0;
		(option_table[0]).to_be_modified=&activate;
		(option_table[1]).to_be_modified=&convert_data;
		(option_table[2]).to_be_modified=&movie_filename;
		(option_table[3]).to_be_modified=&export_filename;
		(option_table[4]).to_be_modified=&input_sequence;
		(option_table[5]).to_be_modified=&keyframe;
		(option_table[6]).to_be_modified=&filename;
		(option_table[7]).to_be_modified=&new;
		(option_table[8]).to_be_modified=&no_rigid_body_motion;
		(option_table[9]).to_be_modified=&modes;
		(option_table[10]).to_be_modified=&play;
		(option_table[11]).to_be_modified=&rigid_body_motion;
		(option_table[12]).to_be_modified=&save_filename;
		(option_table[13]).to_be_modified=&maximum_time;
		(option_table[13]).user_data=&maximum_time_flag;
		(option_table[14]).to_be_modified=&minimum_time;
		(option_table[14]).user_data=&minimum_time_flag;
		(option_table[15]).to_be_modified=&time;
		(option_table[15]).user_data=&time_flag;
		(option_table[16]).to_be_modified=&value;
		(option_table[16]).user_data=&value_flag;
		(option_table[17]).to_be_modified=&slidername;
		(option_table[18]).to_be_modified=&stop;
		return_code=process_multiple_options(state,option_table);
		/* no errors, not asking for help */
		if (return_code)
		{
			if (emoter_dialog)
			{
				if (play && stop)
				{
					display_message(ERROR_MESSAGE,
						"gfx_modify_emoter.  Specify only one of play or stop");
					return_code=0;
				}
				if (rigid_body_motion && no_rigid_body_motion)
				{
					display_message(ERROR_MESSAGE,
						"gfx_modify_emoter.  Specify only one of rigid_body_motion or no_rigid_body_motion");
					return_code=0;
				}
				if ((activate || value_flag) && (!slidername))
				{
					display_message(ERROR_MESSAGE,
						"gfx_modify_emoter.  Slider operations (activate|set_value) require a slider to be named.");
					return_code=0;					
				}
				if (return_code)
				{
					if (slidername)
					{
						emoter_slider = (struct Emoter_slider *)NULL;
						i = 0;
						while ((!emoter_slider) && (i < shared->number_of_sliders))
						{
							if (!strcmp(shared->sliders[i]->name, slidername))
							{
								emoter_slider = shared->sliders[i];
							}
							i++;
						}
						if (emoter_slider)
						{
							if (activate)
							{
								emoter_slider_make_active(emoter_slider);
							}
							if (value_flag)
							{
								emoter_slider_select(emoter_slider, 1);
								emoter_set_slider_value(emoter_slider, value);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"gfx_modify_emoter.  Invalid slider name %s",
								slidername);
							return_code=0;
						}
						DEALLOCATE(slidername);
					}
					if (convert_data)
					{
						emoter_convert_active_slider(emoter_dialog);
					}
					if (export_filename)
					{
						emoter_export_nodes(emoter_dialog, export_filename);
						DEALLOCATE(export_filename);
					}
					if (input_sequence)
					{
						if (shared->input_sequence)
						{
							DEALLOCATE(shared->input_sequence);
						}
						if (strcmp(input_sequence, "none"))
						{
							emoter_dialog->shared->input_sequence = input_sequence;
							XmToggleButtonSetState(emoter_dialog->input_sequence_button, True, False );
						}
						else
						{
							emoter_dialog->shared->input_sequence = (char *)NULL;
							XmToggleButtonSetState(emoter_dialog->input_sequence_button, False, False );
						}
						face_changed = 1;
					}
					if (time_flag)
					{
						integer_time = time;
						emoter_set_play_slider_value(emoter_dialog,
							integer_time);
						face_changed = 1;
					}
					if (maximum_time_flag || minimum_time_flag)
					{
						emoter_set_play_slider_range(emoter_dialog,
							minimum_time, maximum_time);
					}
					if (modes != shared->number_of_modes)
					{
						emoter_set_mode_limit(emoter_dialog, modes);
						face_changed = 1;
					}
					if (play)
					{
						XmToggleButtonSetState( emoter_dialog->play_button,
							True, False );
						emoter_autoplay_CB(emoter_dialog->play_button,
							(XtIntervalId *)NULL);
					}
					if (stop)
					{
						XmToggleButtonSetState( emoter_dialog->play_button,
							False, False );
					}
					if (rigid_body_motion)
					{
						emoter_dialog->shared->show_solid_body_motion = 1;
						XmToggleButtonSetState( emoter_dialog->solid_motion_button,
							True, False );
						face_changed = 1;
					}
					if (no_rigid_body_motion)
					{
						emoter_dialog->shared->show_solid_body_motion = 0;
						XmToggleButtonSetState( emoter_dialog->solid_motion_button,
							False, False );
						face_changed = 1;
					}
					if (new)
					{
						create_emoter_slider((char *)NULL,
							"new", emoter_dialog->slider_form,
							1, emoter_dialog->shared,
							emoter_dialog->shared->number_of_sliders,
							(struct Control_curve *)NULL,
							emoter_dialog, /*no_confirm*/1 ); 
					}
					if (filename)
					{
						if ( temp_filename = strrchr(filename, '/'))
						{
							temp_filename++;
						}
						else
						{
							temp_filename = filename;
						}
						create_emoter_slider(filename,
							temp_filename, emoter_dialog->slider_form,
							1, emoter_dialog->shared,
							emoter_dialog->shared->number_of_sliders,
							(struct Control_curve *)NULL,
							emoter_dialog, /*no_confirm*/1 ); 
						DEALLOCATE(filename);
					}
					if (save_filename)
					{
						emoter_save_slider(emoter_dialog->shared->active_slider,
							save_filename, emoter_dialog);
						DEALLOCATE(save_filename);
					}
					if (movie_filename)
					{
						emoter_create_movie(emoter_dialog, movie_filename);
						DEALLOCATE(movie_filename);
					}
					if (face_changed)
					{
						emoter_update_face (emoter_dialog->shared);
						/* SAB This would be better implemented by adding a special
							manager message to the Scene Manager and then calling that. */
						if (graphics_window=FIRST_OBJECT_IN_MANAGER_THAT(Graphics_window)(
							(MANAGER_CONDITIONAL_FUNCTION(Graphics_window) *)NULL,
							(void *)NULL,emoter_dialog->shared->graphics_window_manager))
						{
							Graphics_window_update_now(graphics_window);
						}
					}
					if (keyframe)
					{
						emoter_keyframe_CB(emoter_dialog_widget, NULL, NULL);
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,"gfx_modify_emoter."
					"  Missing dialog widget.");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"gfx_modify_emoter.  Missing command_data");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* gfx_modify_emoter */

#ifdef OLD_CODE
int set_emoter_slider_value(struct Parse_state *state,
	void *dummy_to_be_modified,void *emoter_dialog_widget_void)
/*******************************************************************************
LAST MODIFIED : 7 April 1998

DESCRIPTION :
==============================================================================*/
{
	char *slider_name;
	float value;
	int found,i,number_of_children,return_code;
	struct Emoter_slider *emoter_slider;
	struct Emoter_dialog *emoter_dialog;
	Widget *child_list,emoter_dialog_widget,emoter_slider_widget,
		*slider;

	ENTER(set_emoter_slider_value);
	return_code = 0;
	/* check arguments */
	if (state&&(emoter_dialog_widget=
		(Widget)emoter_dialog_widget_void))
	{
		if (slider_name=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,slider_name)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,slider_name))
			{
				if (shift_Parse_state(state,1)&&(state->current_token)&&
					(1==sscanf(state->current_token,"%f",&value)))
				{
					/* check if there is a slider with the given name */
					XtVaGetValues(emoter_dialog_widget,
						XmNuserData, &emoter_dialog,
						NULL);
					if (emoter_dialog)
					{
						i = 0;
						while( !return_code && i<emoter_dialog->shared->number_of_sliders )
						{
							emoter_slider = emoter_dialog->shared->sliders[i];
							if ( !strcmp ( emoter_slider->name, slider_name ))
							{
								emoter_set_slider_value( emoter_slider, value );
								return_code = 1;
							}
							i++;
						}
						if ( !return_code )
						{
							display_message(ERROR_MESSAGE,
								"set_emoter_slider_value.  Emoter slider %s not found", slider_name);
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"set_emoter_slider_value.  Missing emoter_dialog");
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
			"set_emoter_slider_value.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* set_emoter_slider_value */
#endif /* OLD_CODE */

#ifdef OLD_CODE
int list_emoter_slider(struct Parse_state *state,
	void *dummy_to_be_modified,void *emoter_dialog_widget_void)
/*******************************************************************************
LAST MODIFIED : 2 March 1998

DESCRIPTION :
==============================================================================*/
{
	char *slider_name;
	int found,i,number_of_children,return_code;
	struct Emoter_slider *emoter_slider;
	struct Emoter_dialog *emoter_dialog;
	Widget *child_list,emoter_dialog_widget,*slider;

	ENTER(list_emoter_slider);
	/* check arguments */
	if (state&&(emoter_dialog_widget=
		(Widget)emoter_dialog_widget_void))
	{
		/* check if there is a slider with the given name */
		XtVaGetValues(emoter_dialog_widget,
			XmNchildren,&child_list,
			XmNnumChildren,&number_of_children,
			NULL);
		if (slider_name=state->current_token)
		{
			if (strcmp(PARSER_HELP_STRING,slider_name)&&
				strcmp(PARSER_RECURSIVE_HELP_STRING,slider_name))
			{
				if (1==number_of_children)
				{
					XtVaGetValues(*child_list,
						XmNuserData,&emoter_dialog,
						NULL);
					if (emoter_dialog)
					{
						slider=emoter_dialog->sliders;
						i=emoter_dialog->number_of_sliders;
						return_code=1;
						found=0;
						while ((i>0)&&(!found)&&return_code)
						{
							XtVaGetValues(*slider,
								XmNuserData,&emoter_slider,
								NULL);
							if (emoter_slider)
							{
								if (0==strcmp(slider_name,emoter_slider->name))
								{
									found=1;
									display_message(INFORMATION_MESSAGE,
										"%s.  min=%g, val=%g, max=%g\n",
										emoter_slider->name,emoter_slider->minimum,
										emoter_slider->value,emoter_slider->maximum);
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
									"list_emoter_slider.  Missing emoter_slider");
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
							"list_emoter_slider.  Missing emoter_dialog");
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"list_emoter_slider.  Invalid dialog");
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
					XmNuserData,&emoter_dialog,
					NULL);
				slider=emoter_dialog->sliders;
				i=emoter_dialog->number_of_sliders;
				while (i>0)
				{
					XtVaGetValues(*slider,
						XmNuserData,&emoter_slider,
						NULL);
					if (emoter_slider)
					{
						display_message(INFORMATION_MESSAGE,"%s.  min=%g, val=%g, max=%g\n",
							emoter_slider->name,emoter_slider->minimum,
							emoter_slider->value,emoter_slider->maximum);
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
			"list_emoter_slider.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* list_emoter_slider */
#endif /* OLD_CODE */
