/*******************************************************************************
FILE : emoter_dialog.cpp

LAST MODIFIED : 4 May 2004

DESCRIPTION :
This module creates a emoter_slider input device.  An emoter slider is
used to control modes shapes for the EMOTER update positions of a
group of nodes
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
#if defined (BUILD_WITH_CMAKE)
#include "configure/cmgui_configure.h"
#endif /* defined (BUILD_WITH_CMAKE) */
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
extern "C" {
#include "api/cmiss_element.h"
#include "api/cmiss_field_module.h"
#include "api/cmiss_field_subobject_group.h"
#include "command/command.h"
	/*???DB.  For Execute_command */
#include "command/parser.h"
#include "emoter/em_cmgui.h"
#include "finite_element/import_finite_element.h"
#include "general/debug.h"
#include "general/geometry.h"
#include "general/manager.h"
#include "general/mystring.h"
#include "graphics/graphics_library.h"
#include "graphics/graphics_window.h"
#include "graphics/scene.h"
#include "graphics/scene_viewer.h"
#include "graphics/rendition.h"
#include "user_interface/confirmation.h"
#include "user_interface/event_dispatcher.h"
#include "user_interface/message.h"
#include "curve/curve.h"
#include "emoter/emoter_dialog.h"
#include "region/cmiss_region.h"
}

/*
Module constants
----------------
*/
#define SLIDER_RESOLUTION (1000)
#define SOLID_BODY_MODES (6)

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
	FE_value time;
	struct Colour viewer_background_colour;
	struct Cmiss_region *region;
	struct Emoter_slider *active_slider, **sliders;
	struct Execute_command *execute_command;
	struct Graphics_buffer_package *graphics_buffer_package;
	struct IO_stream_package *io_stream_package;
	struct Light *viewer_light;
	struct Light_model *viewer_light_model;
	struct MANAGER(FE_basis) *basis_manager;
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
	struct MANAGER(Graphics_window) *graphics_window_manager;
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
	struct MANAGER(Curve) *curve_manager;
	struct EM_Object *em_object;
	int transform_graphics;
	struct Scene *viewer_scene;
	struct User_interface *user_interface;
}; /* struct Shared_emoter_slider_data */

struct Emoter_combine_slider
/*******************************************************************************
LAST MODIFIED : 10 April 1998

DESCRIPTION :
==============================================================================*/
{
	FE_value combine_time;
	struct Emoter_slider *slider;
	struct Curve *curve, *timebase_curve;
}; /* struct Emoter_combine_slider */

struct Emoter_marker
/*******************************************************************************
LAST MODIFIED : 16 April 1998

DESCRIPTION :
==============================================================================*/
{
	char *name;
	int index, element_index, node_index;
	FE_value value;
	struct Emoter_slider *slider_parent;
	struct Shared_emoter_slider_data *shared;
}; /* struct Emoter_marker */

struct Emoter_slider
/*******************************************************************************
LAST MODIFIED : 16 April 1998

DESCRIPTION :
==============================================================================*/
{
	char *name, *sequence_filename;
	FE_value maximum,minimum, value;
	int index, slider_position, selected,
		private_update_face_flag, number_of_combine_sliders,
		animated, number_of_emoter_markers,
		number_of_timebase_curves, solid_body_motion;
	struct Emoter_combine_slider **combine_sliders;
	struct Emoter_marker **emoter_markers;
	struct Shared_emoter_slider_data *shared;
	struct Scene_viewer *scene_viewer;
	struct Curve *mode_curve, **timebase_curves;
}; /* struct Emoter_slider */

struct Emoter_dialog
/*******************************************************************************
LAST MODIFIED : 6 April 1998

DESCRIPTION :
==============================================================================*/
{
	int play_slider_position, movie_loop, movie_every_frame;
	FE_value time_minimum, time_maximum;
	struct Shared_emoter_slider_data *shared;
	void *curve_manager_callback_id;
	Cmiss_nodeset_group_id minimum_nodeset_group;
	struct Shell_list_item *shell_list_item;
	struct Movie_graphics *movie;
	struct Emoter_dialog **dialog_address;
	struct Event_dispatcher_timeout_callback *autoplay_timeout;
}; /* struct Emoter_dialog */

/*
Module functions
----------------
*/

static struct Emoter_slider *create_emoter_slider(const char *sequence_filename,
	const char *name,
	FE_value value,
	struct Shared_emoter_slider_data *shared_data,
	int index, struct Curve *existing_mode_curve,
	struct Emoter_dialog *emoter_dialog, int no_confirm);
/*******************************************************************************
LAST MODIFIED : 9 September 1999

DESCRIPTION :
Both or either of <sequence_filename> or <existing_mode_curve> can be NULL.
Declared here because of circular recursive function calling.
==============================================================================*/

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
	float euler_angles[3];
	gtMatrix transformation; /* 4 x 4 */
	int i,j,k,offset,return_code,versions;
	struct Cmiss_region *input_sequence;
	struct FE_field *field;
	struct FE_node *node;
	struct FE_region *fe_region;
	struct EM_Object *em_object;
	struct IO_stream *input_file;

	ENTER(emoter_update_nodes);
	if (shared_data)
	{
		return_code=1;
		em_object=shared_data->em_object;
		if (0<em_object->n_nodes)
		{
			if ((fe_region=Cmiss_region_get_FE_region(shared_data->region))&&
				(node=FE_region_get_FE_node_from_identifier(fe_region,
				(em_object->index)[0]))&&(field=get_FE_node_default_coordinate_field(node)))
			{
				/* Read from an input sequence which the emoter is overriding */
				if (shared_data->input_sequence)
				{
					sprintf(input_filename,shared_data->input_sequence,
						shared_data->time);
					if ((input_file = CREATE(IO_stream)(shared_data->io_stream_package))
						&& (IO_stream_open_for_read(input_file, input_filename)))
					{
						input_sequence = Cmiss_region_create_region(shared_data->region);
						if (read_exregion_file(input_sequence, input_file,
							(struct FE_import_time_index *)NULL))
						{
							if (Cmiss_region_can_merge(shared_data->region, input_sequence))
							{
								if (!Cmiss_region_merge(shared_data->region, input_sequence))
								{
									display_message(ERROR_MESSAGE,
										"Error merging elements from file: %s", input_filename);
									return_code = 0;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"Contents of file %s not compatible with global objects",
									input_filename);
								return_code = 0;
							}
						}
						else
						{
							display_message(WARNING_MESSAGE,
								"Unable to parse node file %s", input_filename);
							return_code=0;							
						}
						DEACCESS(Cmiss_region)(&input_sequence);
						IO_stream_close(input_file);
						DESTROY(IO_stream)(&input_file);
					}
					else
					{
						display_message(WARNING_MESSAGE,
							"emoter_update_nodes.  Unable to import node file %s", input_filename);
						return_code=0;							
					}
				}

				FE_region_begin_change(fe_region);
				
				/* perform EM reconstruction */
				if ( solid_body_motion && shared_data->transform_graphics)
				{
					euler_angles[0] = shared_data->weights[5];
					euler_angles[1] = shared_data->weights[4];
					euler_angles[2] = shared_data->weights[3];
					euler_to_gtMatrix(euler_angles, transformation);
					transformation[3][0] = shared_data->weights[0];
					transformation[3][1] = shared_data->weights[1];
					transformation[3][2] = shared_data->weights[2];
					Cmiss_rendition *rendition = Cmiss_region_get_rendition_internal(shared_data->region);
					Cmiss_rendition_set_transformation(rendition, &transformation);
					Cmiss_rendition_destroy(&rendition);
				}
				i=0;
				offset=em_object->m;
				while (return_code&&(i<em_object->n_nodes))
				{
					if ((node=FE_region_get_FE_node_from_identifier(fe_region,
						(em_object->index)[i])))
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
							if (!shared_data->transform_graphics)
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
							versions = get_FE_node_field_component_number_of_versions(
								node, field, k);
							for (j = 0 ; j < versions ; j++)
							{
								return_code=set_FE_nodal_FE_value_value(node,
									field, /*component_number*/k, j, FE_NODAL_VALUE,
									/*time*/0, (FE_value)position[k]);
							}
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
				FE_region_end_change(fe_region);
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

static int emoter_set_mode_value(int mode_number, FE_value new_value,
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
	FE_value *total_shape_vector, struct Shared_emoter_slider_data *shared,
	FE_value time)
/*******************************************************************************
LAST MODIFIED : 17 November 1999

DESCRIPTION :
==============================================================================*/
{
	int i;
	FE_value data_start_frame, data_end_frame, marker_start_time, marker_end_time, 
		*shape_vector, slider_time;

	ENTER(emoter_add_slider_modes);

	slider->private_update_face_flag = 1;
	if ( slider->value != 0.0 && slider->mode_curve )
	{
		marker_start_time = slider->emoter_markers[0]->value;
		marker_end_time = 
			slider->emoter_markers[slider->number_of_emoter_markers-1]->value;
		Curve_get_parameter_range( slider->mode_curve, &data_start_frame,
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
			Curve_get_number_of_components(slider->mode_curve)))
		{
			if (Curve_get_values_at_parameter( slider->mode_curve, 
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

static FE_value *emoter_sum_all_modes( struct Shared_emoter_slider_data *shared)
/*******************************************************************************
LAST MODIFIED : 18 April 1998

DESCRIPTION :
Evaluates the modes of all the sliders without updating the nodes.
Returns the vector of mode shapes.
==============================================================================*/
{
	int i, j;
	FE_value *total_shape_vector;
	struct Emoter_slider *slider;

	ENTER(emoter_sum_all_modes);

#if defined (DEBUG_CODE)
	printf("emoter_sum_all_modes\n");
#endif /* defined (DEBUG_CODE) */


	/* First reset all the flags (so no slider is added in twice)
		The flag is set for a slider when emoter_add_slider_modes is
		called */
	for ( j = 0 ; j < shared->number_of_sliders ; j++ )
	{
		(shared->sliders[j])->private_update_face_flag = 0;
	}

	if ( ALLOCATE( total_shape_vector, FE_value, SOLID_BODY_MODES + shared->number_of_modes ))
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
		total_shape_vector = (FE_value *)NULL;
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
	FE_value *total_shape_vector;

	ENTER(emoter_update_face);

#if defined (DEBUG_CODE)
	printf("emoter_update_face\n");
#endif /* defined (DEBUG_CODE) */

	if ( !shared->movie_playing )
	{
		total_shape_vector = emoter_sum_all_modes(shared);
		if (total_shape_vector)
		{
			for ( i = 0; i < shared->mode_limit + SOLID_BODY_MODES ; i++ )
			{
				emoter_set_mode_value ( i, total_shape_vector[i], shared );
			}

			emoter_update_nodes( shared, shared->show_solid_body_motion );
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
			struct Graphics_window *graphics_window;
			/*			(*(emoter_dialog->shared->execute_command->function))(
						"open comfile redraw execute",
						emoter_dialog->shared->execute_command->data);*/
			/* SAB This would be better implemented by adding a special
				manager message to the Scene Manager and then calling that. */
			if ((graphics_window=FIRST_OBJECT_IN_MANAGER_THAT(Graphics_window)(
				(MANAGER_CONDITIONAL_FUNCTION(Graphics_window) *)NULL,
				(void *)NULL,shared->graphics_window_manager)))
			{
				Graphics_window_update_now(graphics_window);
			}
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
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
	FE_value new_value )
/*******************************************************************************
LAST MODIFIED : 9 April 1998

DESCRIPTION :
Sets the emoter slider according to the <new_value> given.
Updates all the basis modes from the active
emoter slider's Curve
==============================================================================*/
{
	int slider_position;

	ENTER(emoter_set_slider_value);
	if ( emoter_slider && emoter_slider->shared )
	{
		emoter_slider->value = new_value;

		/* Check slider position */
		slider_position =
			(int)((FE_value)SLIDER_RESOLUTION*((emoter_slider->value)-
			(emoter_slider->minimum))/((emoter_slider->maximum)-
			(emoter_slider->minimum)));
		if (slider_position < 0)
		{
			slider_position = 0;
		}
		else if (slider_position > SLIDER_RESOLUTION)
		{
			slider_position = SLIDER_RESOLUTION;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
		"emoter_set_slider_value.  Invalid arguments");
	}
	LEAVE;
} /* emoter_set_slider_value */

static void emoter_set_marker_value( struct Emoter_marker *emoter_marker,
	FE_value new_value )
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
		active = emoter_marker->shared->active_slider;
		if (active)
		{
			if ( active == emoter_marker->slider_parent )
			{
				emoter_marker->value = new_value;
				/* Change the independent value in every timebase curve referring to this
					slider */
				for ( i = 0 ; i < active->number_of_timebase_curves ; i++ )
				{
					if ( 1 == (Curve_get_number_of_components
						(active->timebase_curves[i])) )
					{
						Curve_set_node_values(
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
						Curve_set_parameter(
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

static int DESTROY(Emoter_marker)(struct Emoter_marker **emoter_marker_address)
/*******************************************************************************
LAST MODIFIED : 10 December 2003

DESCRIPTION :
Destroy Emoter marker scrollbar callback.  Deallocates the structure for the
marker
==============================================================================*/
{
	int return_code;
	struct Emoter_marker *emoter_marker;

	ENTER(DESTROY(Emoter_marker));
	emoter_marker = *emoter_marker_address;
	if (emoter_marker)
	{
		if ( emoter_marker->name )
		{
			DEALLOCATE( emoter_marker->name );
		}
		DEALLOCATE( *emoter_marker_address );
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Emoter_marker).  Missing emoter_marker structure");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Emoter_marker) */

static int DESTROY(Emoter_slider)(struct Emoter_slider **emoter_slider_address)
/*******************************************************************************
LAST MODIFIED : 9 December 2003

DESCRIPTION :
Destroy Emoter slider scrollbar callback.  Deallocates the structure for the
slider
==============================================================================*/
{
	int i, j, return_code;
	struct Emoter_slider *emoter_slider, *combine_slider;
	struct Curve *temp_var;

	ENTER(DESTROY(Emoter_slider));
	emoter_slider = *emoter_slider_address;
	if (emoter_slider)
	{
		return_code = 1;
		for (i = 0 ; i < emoter_slider->number_of_emoter_markers ; i++)
		{
			DESTROY(Emoter_marker)(emoter_slider->emoter_markers + i);
		}
		emoter_slider->number_of_emoter_markers = 0;
		if ( emoter_slider->mode_curve )
		{
			temp_var = emoter_slider->mode_curve;
			DEACCESS(Curve)(&(emoter_slider->mode_curve));
			REMOVE_OBJECT_FROM_MANAGER(Curve)
				(temp_var, emoter_slider->shared->curve_manager);
		}
		for ( j = 0 ; j < emoter_slider->number_of_combine_sliders ; j++ )
		{
			temp_var = (emoter_slider->combine_sliders[j])->curve;
			DEACCESS(Curve)( &((emoter_slider->combine_sliders[j])->curve) );
			REMOVE_OBJECT_FROM_MANAGER(Curve)
				(temp_var, emoter_slider->shared->curve_manager);

			/* To complete this destruction the reference to the Curve
				in the combine slider also needs to be removed */
			combine_slider = (emoter_slider->combine_sliders[j])->slider;
			temp_var = (emoter_slider->combine_sliders[j])->timebase_curve;
			if (temp_var)
			{
				/* Check the combine_slider is still active.
				   Would be preferable to have removed this reference to an invalid slider
 				   but this is quite expensive. The present fix does not stop the combine
               slider from attempts to evaluate it. */
				i = 0;
				while ((i < emoter_slider->shared->number_of_sliders) &&
					(combine_slider != emoter_slider->shared->sliders[i]))
				{
					i++;
				}
				if ((i < emoter_slider->shared->number_of_sliders) &&
					(combine_slider->timebase_curves))
				{
					/* This pointer still exists in the global list so is valid */
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
							"DESTROY(Emoter_slider).  Unable to find timebase curve reference in combine curve");
						return_code = 0;
					}
				}
				if ( (emoter_slider->combine_sliders[j])->timebase_curve )
				{
					temp_var = (emoter_slider->combine_sliders[j])->timebase_curve;
					DEACCESS(Curve)( &((emoter_slider->combine_sliders[j])->timebase_curve) );
					REMOVE_OBJECT_FROM_MANAGER(Curve)
						(temp_var, emoter_slider->shared->curve_manager);
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
		DEALLOCATE( *emoter_slider_address );
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Emoter_slider).  Missing emoter_slider structure");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Emoter_slider) */

int DESTROY(Emoter_dialog)(struct Emoter_dialog **emoter_dialog_address)
/*******************************************************************************
LAST MODIFIED : 9 December 2003

DESCRIPTION :
Callback for the emoter dialog - tidies up all details - mem etc
==============================================================================*/
{
	int i, return_code;
	struct Emoter_dialog *emoter_dialog;

	ENTER(DESTROY(Emoter_dialog));
	emoter_dialog = *emoter_dialog_address;
	if (emoter_dialog)
	{
		return_code = 1;
		for (i = 0 ; i < emoter_dialog->shared->number_of_sliders ; i++)
		{
			DESTROY(Emoter_slider)(emoter_dialog->shared->sliders + i);
		}
		emoter_dialog->shared->number_of_sliders = 0;

		if (emoter_dialog->autoplay_timeout)
		{
			Event_dispatcher_remove_timeout_callback(
				User_interface_get_event_dispatcher(emoter_dialog->shared->user_interface),
				emoter_dialog->autoplay_timeout);
		}

		if ( emoter_dialog->curve_manager_callback_id )
		{
			MANAGER_DEREGISTER(Curve)(
				emoter_dialog->curve_manager_callback_id,
				emoter_dialog->shared->curve_manager);
			emoter_dialog->curve_manager_callback_id=(void *)NULL;
		}

		/* Destroy shared slider data */
		DEALLOCATE(emoter_dialog->shared->weights);
		DEALLOCATE(emoter_dialog->shared->sliders);
		DEACCESS(Cmiss_region)(&emoter_dialog->shared->region);
		destroy_EM_Object(&(emoter_dialog->shared->em_object));
		Cmiss_nodeset_group_destroy(&emoter_dialog->minimum_nodeset_group);
		destroy_Shell_list_item(&(emoter_dialog->shell_list_item));

		DEALLOCATE(emoter_dialog->shared);
		
		*(emoter_dialog->dialog_address) = (struct Emoter_dialog *)NULL;

		DEALLOCATE(emoter_dialog);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Emoter_dialog).  Could not get emoter_dialog");
		return_code = 0;
	}

	LEAVE;
	return (return_code);
} /* DESTROY(Emoter_dialog) */

static void emoter_update_combine_sliders(struct Emoter_slider *slider)
/*******************************************************************************
LAST MODIFIED : 4 November 1998

DESCRIPTION :
Callback for the play slider which sets the min and max and stores the widget
==============================================================================*/
{
	int i;
	FE_value *combine_vector;
	struct Emoter_slider *combine_slider;
	struct Curve *timebase_curve,*curve;

	ENTER(emoter_update_combine_sliders);

	if ( slider )
	{
		if ( slider->number_of_combine_sliders )
		{
			for ( i = 0 ; i < slider->number_of_combine_sliders ; i++ )
			{
				curve=(slider->combine_sliders[i])->curve;
				combine_slider = (slider->combine_sliders[i])->slider;
				if (curve && combine_slider)
				{
					if (ALLOCATE(combine_vector,FE_value,
						Curve_get_number_of_components(curve)))
					{
						if (Curve_get_values_at_parameter(curve,
							slider->shared->time,	combine_vector, (FE_value *)NULL))
						{
							emoter_set_slider_value( combine_slider, combine_vector[0] );
							emoter_slider_select( combine_slider, 0 );
						}
						DEALLOCATE ( combine_vector );
					}
					(slider->combine_sliders[i])->combine_time = slider->shared->time;
					timebase_curve=(slider->combine_sliders[i])->timebase_curve;
					if (timebase_curve)
					{
						if (ALLOCATE(combine_vector,FE_value,
							Curve_get_number_of_components(timebase_curve)))
						{
							if (Curve_get_values_at_parameter(timebase_curve,
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

static void emoter_set_play_slider_value( struct Emoter_dialog *emoter_dialog,
	FE_value new_value )
/*******************************************************************************
LAST MODIFIED : 8 April 1998

DESCRIPTION :
Sets the play slider according to the <new_value> given.
Updates all the basis modes from the active
emoter slider's Curve
==============================================================================*/
{
	int slider_position;
	struct Emoter_slider *active;

	ENTER(emoter_set_play_slider_value);
	if ( emoter_dialog && emoter_dialog->shared )
	{
		emoter_dialog->shared->time = new_value;

		/* Check slider position */
		slider_position = (int)new_value;

		/* Update the 'combine' sliders from the active slider */
		active = emoter_dialog->shared->active_slider;
		if (active)
		{
			emoter_update_combine_sliders(active);
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
	FE_value min, FE_value max )
/*******************************************************************************
LAST MODIFIED : 15 April 1998

DESCRIPTION :
Sets the slider range of the play slider
==============================================================================*/
{
	ENTER(emoter_set_play_slider_range);

	if ( emoter_dialog )
	{
		emoter_dialog->time_minimum = min;
		emoter_dialog->time_maximum = max;
	}
	else
	{
		display_message(ERROR_MESSAGE,
		"emoter_set_play_slider_range.  Invalid arguments");
	}
	LEAVE;
} /* emoter_set_play_slider_range */

static int emoter_slider_make_active(struct Emoter_slider *slider)
/*******************************************************************************
LAST MODIFIED : 7 April 1998

DESCRIPTION :
Sets the active slider to point to this <slider> and updates the
togglebuttons accordingly.
==============================================================================*/
{
	int return_code;

	ENTER(emoter_slider_make_active);
	if ( slider )
	{
		slider->shared->active_slider = slider;
		return_code = 1;
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

struct read_file_data
/*******************************************************************************
LAST MODIFIED : 7 April 1998

DESCRIPTION :
==============================================================================*/
{
	struct IO_stream *stream;
	char current_token[300];
};

static int read_file_next_token (struct read_file_data *file_data)
/*******************************************************************************
LAST MODIFIED : 7 April 1998

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(read_file_FE_value);

	IO_stream_scan(file_data->stream, "%s", file_data->current_token );
	while ('#' == file_data->current_token[0])
	{
		while (!IO_stream_end_of_stream(file_data->stream)
			&& 10 != IO_stream_getc(file_data->stream))
		{
		}
		IO_stream_scan(file_data->stream, "%s", file_data->current_token );
	}

	return_code = 1;

	LEAVE;

	return (return_code);
} /* read_file_next_token */

static int read_file_FE_value (struct read_file_data *file_data,
	FE_value *data )
/*******************************************************************************
LAST MODIFIED : 7 April 1998

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(read_file_FE_value);

	sscanf(file_data->current_token, FE_VALUE_INPUT_STRING, data);
	read_file_next_token(file_data);

	return_code = 1;

	LEAVE;

	return (return_code);
} /* read_file_FE_value */

static int read_file_int (struct read_file_data *file_data,
	int *data )
/*******************************************************************************
LAST MODIFIED : 7 April 1998

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(read_file_FE_value);

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
	const char *filename, struct IO_stream_package *io_stream_package)
/*******************************************************************************
LAST MODIFIED : 7 April 1998

DESCRIPTION :
==============================================================================*/
{
	struct read_file_data *data;

	ENTER(read_file_open);

	if ( ALLOCATE(data, struct read_file_data, 1))
	{
		if ((data->stream = CREATE(IO_stream)(io_stream_package))
			&& IO_stream_open_for_read(data->stream, filename))
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

	IO_stream_close(file_data->stream);
	DESTROY(IO_stream)(&file_data->stream);
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

	return_code = IO_stream_end_of_stream(file_data->stream);

	LEAVE;

	return (return_code);
} /* read_file_eof */

static int read_file_eof_marker(struct read_file_data *file_data, const char *marker)
/*******************************************************************************
LAST MODIFIED : 19 April 1998

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(read_file_eof_marker);

	if (IO_stream_end_of_stream(file_data->stream))
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

static int read_file_marker(struct read_file_data *file_data, const char *marker)
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
	int basis_modes, int total_values, FE_value *weights )
/*******************************************************************************
LAST MODIFIED : 6 March 2000

DESCRIPTION :
Reads a weight vector from the <file>.
==============================================================================*/
{
	FE_value weight;
	int j;

	ENTER(read_weights);

	for ( j = 0 ; j < total_values ; j++ )
	{
		read_file_FE_value(file_data, &weight );
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
static int read_emoter_mode_Curve( struct Curve **emoter_curve_addr,
	struct MANAGER(Curve) *curve_manager,
	char *filename, struct Shared_emoter_slider_data *shared,
	FE_value *number_of_frames)
/*******************************************************************************
LAST MODIFIED : 17 November 1999

DESCRIPTION :
Creates a control curve which is read from file values.
==============================================================================*/
{
	char warning[300], *name;
	const char *filename_base;
	FE_value time;
	FE_value *shape_vector, weight;
	int i, j, k, n_modes, return_code;
	struct read_file_data *file_data;
	struct Curve *emoter_curve;

	ENTER(read_emoter_mode_Curve);

	if ( emoter_curve_addr && curve_manager && filename )
	{
		if ( file_data = read_file_open( filename ))
		{
			if ( filename_base = strrchr( filename, '/'))
			{
				filename_base++;
			}
			else
			{
				filename_base = filename;
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
				if ( ALLOCATE( name, char, strlen(filename_base)+1))
				{
					strcpy(name, filename_base);
					if ( ALLOCATE( shape_vector, FE_value, shared->number_of_modes ))
					{
						while (FIND_BY_IDENTIFIER_IN_MANAGER(Curve,name)(
							name, curve_manager))
						{
							REALLOCATE(name, name, char, strlen(name)+2);
							strcat(name,"+");
						}
						if (emoter_curve = CREATE(Curve)(name,
							LINEAR_LAGRANGE, shared->number_of_modes ))
						{
							if (ADD_OBJECT_TO_MANAGER(Curve)(
								emoter_curve, curve_manager))
							{
								/* Set the type after the object is added to the
									manager so that ADD message is ignored */
								Curve_set_type(emoter_curve,
									CURVE_TYPE_EMOTER_MODES );
								ACCESS(Curve)(emoter_curve);
								/* SAB Do the first and second modes as special case
									as we need to create the first element, position both
									nodes and then from then on the current elements can
									just be appended to */
								Curve_add_element( emoter_curve, 1 );
								time = 1.0;
								Curve_set_parameter( emoter_curve,/*element_no*/1,
									/*local_node_no*/0,time);
								time += 1.0;
								Curve_set_parameter( emoter_curve,/*element_no*/1,
									/*local_node_no*/1,time);
								read_weights( n_modes, file_data, shared->number_of_modes, shape_vector );
								Curve_set_node_values( emoter_curve,
									1, 0, shape_vector);
								if ( !read_file_eof_marker(file_data, "END_MODE_DATA"))
								{
									read_weights( n_modes, file_data, shared->number_of_modes, shape_vector );
								}
								/* else do nothing and make a single constant element */
								Curve_set_node_values( emoter_curve,
									1, 1, shape_vector);

								i = 2;
								while (!read_file_eof_marker(file_data, "END_MODE_DATA"))
								{
									Curve_add_element( emoter_curve, i );
									time += 1.0;
									Curve_set_parameter(emoter_curve,/*element_no*/i,
										/*local_node_no*/1,time);
									read_weights( n_modes, file_data, shared->number_of_modes, shape_vector );
									Curve_set_node_values( emoter_curve,
										i, 1, shape_vector);
									i++;
								}
								*emoter_curve_addr = emoter_curve;
								*number_of_frames = i;
								return_code = 1;
							}
							else
							{
								DESTROY(Curve)(&emoter_curve);
								display_message(ERROR_MESSAGE,
									"read_emoter_mode_Curve.  Unable to add control curve to manager");
								return_code=0;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"read_emoter_mode_Curve.  Unable to create control curve");
							return_code=0;
						}
						DEALLOCATE (shape_vector);
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"read_emoter_mode_Curve.  Unable to allocate shape vector");
						return_code=0;
					}
					DEALLOCATE(name);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"read_emoter_mode_Curve.  Unable to allocate name string");
					return_code=0;
				}
			}
			read_file_close( file_data );
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"read_emoter_mode_Curve.  Unable to open file %s", filename);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"read_emoter_mode_Curve.  Invalid argument(s)");
		return_code=0;
	}

	LEAVE;

	return (return_code);
} /* read_emoter_mode_Curve */
#endif /* OLD_CODE */

static struct Curve *read_emoter_curve( struct read_file_data *file_data,
	const char *base_name, struct Shared_emoter_slider_data *shared)
/*******************************************************************************
LAST MODIFIED : 17 November 1999

DESCRIPTION :
Reads a control curve from a file.
==============================================================================*/
{
	char *name;
	enum FE_basis_type fe_basis_type;
	int j, local_node_no = 0, nodes_per_element, elements, derivatives,
		return_code;
	FE_value time, total_time, value;
	struct Curve *curve;

	ENTER(read_emoter_curve);

	curve = (struct Curve *)NULL;

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
			while (FIND_BY_IDENTIFIER_IN_MANAGER(Curve,name)(
				name, shared->curve_manager))
			{
				REALLOCATE(name, name, char, strlen(name)+2);
				strcat(name,"+");
			}
			curve = CREATE(Curve)(name, fe_basis_type, 1);
			if (curve)
			{
				Curve_set_type(curve,
					CURVE_TYPE_EMOTER_COMBINE );
				if (ADD_OBJECT_TO_MANAGER(Curve)(
					curve, shared->curve_manager))
				{
					ACCESS(Curve)(curve);
					for ( j = 0 ; j < elements ; j ++ )
					{
						Curve_add_element( curve, j + 1 );
						if (0==j)
						{
							read_file_FE_value( file_data, &total_time );
							Curve_set_parameter( curve,/*element_no*/1,
								/*local_node_no*/0,	total_time );
						}
						read_file_FE_value( file_data, &time );
						total_time += time;
						Curve_set_parameter( curve,/*element_no*/j+1,
							/*local_node_no*/1,	total_time );
						local_node_no = 0;
						for (local_node_no = 0 ;
								local_node_no < nodes_per_element - 1
									; local_node_no++ )
						{
							read_file_FE_value( file_data, &value );
							Curve_set_node_values ( curve,
								j + 1, local_node_no, &value );
							if ( derivatives )
							{
								read_file_FE_value( file_data, &value );
								Curve_set_node_derivatives ( curve,
									j + 1, local_node_no, &value );
							}
							return_code = 1;
						}
					}
					read_file_FE_value( file_data, &value );
					Curve_set_node_values ( curve,
						elements, nodes_per_element - 1, &value );
					if ( derivatives )
					{
						read_file_FE_value( file_data, &value );
						Curve_set_node_derivatives ( curve,
							elements, local_node_no, &value );
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"emoter_keyframe_CB.  Unable to add combine curve to manager");
					DESTROY(Curve)(&curve);
					curve = (struct Curve *)NULL;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"emoter_keyframe_CB.  Could not create combine slider curve");
				curve = (struct Curve *)NULL;
			}
			DEALLOCATE( name );
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"emoter_keyframe_CB.  Could not allocate combine curve name");
			curve = (struct Curve *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"read_emoter_slider_file.  Unsupported combination of nodes per element and derivatives");
		curve = (struct Curve *)NULL;
	}

	LEAVE;

	return (curve);
} /* read_emoter_curve */

static struct Emoter_marker *create_emoter_marker(const char *name,
	FE_value value,
	int index,
	int element_index, int node_index, int end_marker,
	struct Emoter_slider *slider_parent,
	struct Shared_emoter_slider_data *shared_data)
/*******************************************************************************
LAST MODIFIED : 16 April 1998

DESCRIPTION :
==============================================================================*/
{
	struct Emoter_marker *emoter_marker;

	ENTER(create_emoter_marker);
	/* check arguments */
	if (name && shared_data)
	{
		if ( ALLOCATE ( emoter_marker, struct Emoter_marker, 1) &&
			ALLOCATE( emoter_marker->name, char, strlen(name) + 1))
		{
			strcpy( emoter_marker->name, name );
			emoter_marker->shared = shared_data;
			emoter_marker->slider_parent = slider_parent;
			emoter_marker->value = value;
			emoter_marker->element_index = element_index;
			emoter_marker->node_index = node_index;
			emoter_marker->index = index;
			USE_PARAMETER(end_marker);
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
			"create_emoter_marker.  Invalid argument(s)");
		emoter_marker=(struct Emoter_marker *)NULL;
	}
	LEAVE;

	return (emoter_marker);
} /* create_emoter_marker */

static int read_emoter_slider_file( struct Emoter_slider *slider,
	const char *filename, FE_value *start_time, FE_value *end_time, int *marker_count,
	struct Emoter_dialog *emoter_dialog, void **icon_data,
	int *icon_width, int *icon_height, int no_confirm )
/*******************************************************************************
LAST MODIFIED : 17 November 1999

DESCRIPTION :
Reads stuff from a file.
==============================================================================*/
{
	char *basename, warning[300], *name, *temp_filename, temp_string[300],
		*char_data;
	const char *constname;
	FE_value *shape_vector,total_time;
	int face_index, face_values, header, i, index, j, n_modes, return_code,
		solid_body_index = 0, values;
	unsigned int integer_data;
	struct read_file_data *file_data;
	struct Emoter_combine_slider *combine_slider, **new_combine_sliders;
	struct Emoter_marker **new_markers;
	struct Emoter_slider *slider_to_combine;
	struct Shared_emoter_slider_data *shared;
	struct Curve *emoter_curve, *curve, **new_timebase_curves;

	ENTER(read_emoter_slider_file);

	if ( slider && filename )
	{
		shared = slider->shared;
		file_data = read_file_open(filename, shared->io_stream_package);
		if (file_data)
		{
			constname = strrchr( filename, '/');
			if (constname)
			{
				constname++;
				basename = duplicate_string(filename);
				basename[constname - filename] = 0;
				name = duplicate_string(constname);
			}
			else
			{
				name = duplicate_string(filename);
				basename = duplicate_string("");
			}
			return_code = 1;
			while ( return_code && !read_file_eof(file_data))
			{
				if ( read_file_marker(file_data, "COMBINE"))
				{
					read_file_string(file_data, temp_string);

					return_code = 0;
					slider_to_combine = (struct Emoter_slider *)NULL;
#if defined (DEBUG_CODE)
					printf(" %d\n", shared->number_of_sliders);
#endif /* defined (DEBUG_CODE) */
					for ( i = 0 ; i < shared->number_of_sliders ; i++ )
					{
						if ( (shared->sliders[i])->sequence_filename )
						{
#if defined (DEBUG_CODE)
							printf("%s   %s\n", temp_string, (shared->sliders[i])->sequence_filename);
#endif /* defined (DEBUG_CODE) */
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
							warning,
									shared->user_interface
#if defined (WX_USER_INTERFACE)
, shared->execute_command
#endif /* defined (WX_USER_INTERFACE) */
 ))
						{
							temp_filename = strrchr( temp_string, '/');
							if (temp_filename)
							{
								temp_filename = duplicate_string(temp_filename + 1);
							}
							else
							{
								int error = 0;
								temp_filename = duplicate_string(basename);
								append_string(&temp_filename, temp_string, &error);
							}
							slider_to_combine = create_emoter_slider(temp_filename,
								temp_string,
								1, shared, shared->number_of_sliders,
								(struct Curve *)NULL, emoter_dialog, no_confirm );
							if (slider_to_combine)
							{
								return_code = 1;
							}
							else
							{
								return_code = 0;
							}
							DEALLOCATE(temp_filename);
						}
					}

					if ( return_code )
					{
						if ( REALLOCATE( name, name, char, strlen( slider->name ) +
							strlen( slider_to_combine->name ) + 20 /* Allocate enough for timebase name too */ ))
						{
							sprintf(name, "%s in %s", slider_to_combine->name, slider->name );
							curve = read_emoter_curve(file_data, name, shared);
							if (curve)
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
										combine_slider->combine_time = shared->time;
										if ( read_file_marker(file_data, "TIMEBASE"))
										{
											strcat( name, " timebase" );
											combine_slider->timebase_curve = read_emoter_curve(file_data, name, shared);
											if (combine_slider->timebase_curve)
											{
												if ( REALLOCATE( new_timebase_curves, slider_to_combine->timebase_curves,
													struct Curve *, slider_to_combine->number_of_timebase_curves + 1))
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
											combine_slider->timebase_curve = (struct Curve *)NULL;
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
							DEALLOCATE(name);
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
					IO_stream_scan(file_data->stream, "%d", icon_height );
					if ( ALLOCATE( *icon_data, char, 3 * *icon_width * *icon_height ))
					{
						char_data = static_cast<char *>(*icon_data);
						for ( j = 0 ; j < *icon_height ; j++ )
						{
							for ( i = 0 ; i < *icon_width ; i++ )
							{
								IO_stream_scan(file_data->stream, "%6x", &integer_data);
								*char_data = ( integer_data >> 16 ) & 0xff;
								char_data++;
								*char_data = ( integer_data >> 8 ) & 0xff;
								char_data++;
								*char_data = integer_data & 0xff;
								char_data++;
							}
							IO_stream_scan(file_data->stream, "\n");
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
							IO_stream_scan(file_data->stream, "%*[^\n]");
							IO_stream_scan(file_data->stream, "%*[\n]");
							
							/* Comment/title line */
							IO_stream_scan(file_data->stream, "%*[^\n]");
							IO_stream_scan(file_data->stream, "%*[\n]");
							
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
								warning, 
								 shared->user_interface
#if defined (WX_USER_INTERFACE)
, shared->execute_command
#endif /* defined (WX_USER_INTERFACE) */
																													 );
						}
					}
					if ( return_code )
					{
						if (name)
						{
							if ( ALLOCATE( shape_vector, FE_value, shared->number_of_modes + slider->solid_body_motion ))
							{
								while (FIND_BY_IDENTIFIER_IN_MANAGER(Curve,name)(
									name, shared->curve_manager))
								{
									REALLOCATE(name, name, char, strlen(name)+2);
									strcat(name,"+");
								}
								emoter_curve = CREATE(Curve)(name, LINEAR_LAGRANGE, shared->number_of_modes + slider->solid_body_motion);
								if (emoter_curve)
								{
									if (ADD_OBJECT_TO_MANAGER(Curve)(
										emoter_curve, shared->curve_manager))
									{
										/* Set the type after the object is added to the
											manager so that ADD message is ignored */
										Curve_set_type(emoter_curve,
											CURVE_TYPE_EMOTER_MODES );
										ACCESS(Curve)(emoter_curve);
										/* SAB Do the first and second modes as special case
											as we need to create the first element, position both
											nodes and then from then on the current elements can
											just be appended to */
										Curve_add_element( emoter_curve, 1 );
										*start_time = 1.0;
										if ( read_file_marker(file_data, "MARKER"))
										{
											/* Discard a start marker name, it is already counted and the name will be start */
											read_file_string(file_data, temp_string);
											read_file_FE_value(file_data, start_time);
										}
										total_time= *start_time;
										*end_time = *start_time;
										Curve_set_parameter( emoter_curve,
											/*element_no*/1,/*local_node_no*/0,	*start_time);
										read_weights( face_index, n_modes, solid_body_index, 
											slider->solid_body_motion, file_data, 
											shared->number_of_modes, values, shape_vector);
										Curve_set_node_values( emoter_curve,
											1, 0, shape_vector);
										if ( !read_file_eof_marker(file_data, "END_MODE_DATA"))
										{
											total_time += 1.0;
											if ( read_file_marker(file_data, "MARKER"))
											{
												/* Get the next marker */
												read_file_string(file_data, temp_string);
												read_file_FE_value(file_data, end_time);
											}
											read_weights( face_index, n_modes, solid_body_index, 
												slider->solid_body_motion, file_data, 
												shared->number_of_modes, values, shape_vector);
										}
										Curve_set_parameter( emoter_curve,
											/*element_no*/1,/*local_node_no*/1,	total_time );

										/* else do nothing and make a single constant element */
										Curve_set_node_values( emoter_curve,
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
																*end_time, 
																*marker_count - 1,
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
												read_file_FE_value(file_data, end_time);
											}
											Curve_add_element( emoter_curve, i );
											total_time += 1.0;
											Curve_set_parameter( emoter_curve,
												/*element_no*/i,/*local_node_no*/1,	total_time );
											read_weights( face_index, n_modes, solid_body_index, 
												slider->solid_body_motion, file_data, 
												shared->number_of_modes, values, shape_vector);
											Curve_set_node_values( emoter_curve,
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
										DESTROY(Curve)(&emoter_curve);
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
							DEALLOCATE(name);
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
			DEALLOCATE(basename);
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

static struct Emoter_slider *create_emoter_slider(const char *sequence_filename,
	const char *name,
	FE_value value,
	struct Shared_emoter_slider_data *shared_data,
	int index, struct Curve *existing_mode_curve,
	struct Emoter_dialog *emoter_dialog, int no_confirm)
/*******************************************************************************
LAST MODIFIED : 9 September 1999

DESCRIPTION :
Both or either of <sequence_filename> or <existing_mode_curve> can be NULL.
==============================================================================*/
{
	int marker_count, return_code;
	FE_value start_time, end_time;
	struct Emoter_slider *emoter_slider;
	void *icon_data;
	int icon_width, icon_height;

	ENTER(create_emoter_slider);
	/* check arguments */

	if (name && shared_data)
	{
		if ( ALLOCATE ( emoter_slider, struct Emoter_slider, 1)&&
			ALLOCATE(emoter_slider->name, char, strlen(name) + 1) &&
			ALLOCATE(emoter_slider->combine_sliders, struct Emoter_combine_slider *, 1)&&
			ALLOCATE(emoter_slider->emoter_markers, struct Emoter_marker *, 2) &&
			ALLOCATE(emoter_slider->timebase_curves, struct Curve *, 1))
		{
			emoter_slider->index = index;
			strcpy(emoter_slider->name, name);
			emoter_slider->shared = shared_data;
			emoter_slider->minimum = -2.5;
			emoter_slider->maximum = 2.5;
			emoter_slider->mode_curve = (struct Curve *)NULL;
			emoter_slider->number_of_combine_sliders = 0;
			emoter_slider->value = value;
			emoter_slider->selected = 0;
			emoter_slider->animated = 0;
			emoter_slider->sequence_filename = (char *)NULL;
			emoter_slider->number_of_emoter_markers = 0;
			emoter_slider->number_of_timebase_curves = 0;
			emoter_slider->solid_body_motion = 0;
			emoter_slider->slider_position = -1;
			start_time = 0;
			end_time = 0;
			marker_count = 1;
			icon_data = NULL;
			icon_width = 0;
			icon_height = 0;

			return_code = 1;
			if ( return_code )
			{
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
							ACCESS(Curve)(existing_mode_curve);
					}
					return_code = 1;
				}

				emoter_slider->scene_viewer = (struct Scene_viewer *)NULL;
				emoter_slider->number_of_emoter_markers = marker_count + 1;
				emoter_slider->emoter_markers[0] = create_emoter_marker (
					"Start", start_time,
					0, 1, 0, 0,
					emoter_slider, shared_data );
				emoter_slider->emoter_markers[marker_count] = create_emoter_marker (
					"End", end_time,
					marker_count, marker_count, 1, 1,
					emoter_slider, shared_data );
				if ( REALLOCATE( shared_data->sliders,
					shared_data->sliders, struct Emoter_slider *,
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
			"create_emoter_slider.  Invalid argument(s)");
		emoter_slider=(struct Emoter_slider *)NULL;
	}
	LEAVE;

	return (emoter_slider);
} /* create_emoter_slider */

static void emoter_save_slider(struct Emoter_slider *slider,
	char *filename, struct Emoter_dialog *emoter_dialog)
/*******************************************************************************
LAST MODIFIED : 17 November 1999

DESCRIPTION :

==============================================================================*/
{
	char *char_data;
	FILE *file;
	FE_value delta_time, dS_dxi, end_time, start_time, frame_time, value,
		derivative, *temp_data;
	int i, j, k, frame, nodes_per_element, elements, derivatives,
		number_of_frames, width, height, marker_no, number_of_components;
	struct Curve *curve;
	time_t current_time;
	void *data;

	ENTER(emoter_save_slider);


	if ( slider && filename )
	{
		file = fopen(filename, "w");
		if (file)
		{
			/* Write header comment */
			fprintf(file, "#Emoter emotion file '%s' written ", slider->name);
			time(&current_time);
			fprintf(file, "%s", ctime(&current_time));

			number_of_frames = (int)(slider->emoter_markers
				[slider->number_of_emoter_markers-1]->value
				- slider->emoter_markers[0]->value + 1);
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
					temp_data = (FE_value *)NULL;

					if ( frame_time == slider->emoter_markers[marker_no]->value )
					{
						fprintf(file, "MARKER %s %f\n", slider->emoter_markers[marker_no]->name,
							slider->emoter_markers[marker_no]->value );
						marker_no++;
					}
					if ( ALLOCATE( temp_data, FE_value, SOLID_BODY_MODES + emoter_dialog->shared->number_of_modes ))
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
					number_of_components=Curve_get_number_of_components(curve);
					if ( 1 == number_of_components )
					{
						fprintf(file, "COMBINE\n");
						fprintf(file, "%s\n", (slider->combine_sliders[i])->slider->sequence_filename );
						nodes_per_element = Curve_get_nodes_per_element
							(curve);
						elements = Curve_get_number_of_elements
							(curve);
						derivatives = Curve_get_derivatives_per_node(curve);
						Curve_get_parameter_range( curve, &start_time , &end_time);

						fprintf(file, "%d #nodes per element\n", nodes_per_element);
						fprintf(file, "%d #elements\n", elements);
						fprintf(file, "%d #derivatives\n", derivatives);
						fprintf(file, "%f #start time\n", start_time);
						for ( j = 1 ; j <= elements ; j++ )
						{
							Curve_get_element_parameter_change(
								curve,
								j, &delta_time);
							fprintf(file, "%f",delta_time );
							for ( k = 0 ; k < nodes_per_element - 1 ; k++ )
							{
								if (Curve_get_node_values(curve, j, k, &value))
								{
									fprintf(file, " %f", value);
									if ( derivatives )
									{
										if (Curve_get_node_derivatives(
											curve, j, k, &derivative) &&
											Curve_get_node_scale_factor_dparameter(
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
						if (Curve_get_node_values(curve, elements,
							nodes_per_element-1, &value))
						{
							fprintf(file, " %f", value);
							if ( derivatives )
							{
								if (Curve_get_node_derivatives(curve,
									elements, nodes_per_element-1, &derivative) &&
									Curve_get_node_scale_factor_dparameter(
										curve, elements, nodes_per_element-1, &dS_dxi))
								{
									fprintf(file, " %f", derivative * dS_dxi );
								}
							}
							fprintf(file, "\n");
						}
						curve = (slider->combine_sliders[i])->timebase_curve;
						if (curve)
						{
							fprintf(file, "TIMEBASE\n");
							nodes_per_element = Curve_get_nodes_per_element
								(curve);
							elements = Curve_get_number_of_elements
								(curve);
							derivatives = Curve_get_derivatives_per_node(curve);
							Curve_get_parameter_range( curve,
								&start_time , &end_time);

							fprintf(file, "%d #nodes per element\n", nodes_per_element);
							fprintf(file, "%d #elements\n", elements);
							fprintf(file, "%d #derivatives\n", derivatives);
							fprintf(file, "%f #start time\n", start_time);
							for ( j = 1 ; j <= elements ; j++ )
							{
								Curve_get_element_parameter_change(
									curve,
									j, &delta_time);
								fprintf(file, "%f",delta_time );
								for ( k = 0 ; k < nodes_per_element - 1 ; k++ )
								{
									if (Curve_get_node_values(curve, j, k, &value))
									{
										fprintf(file, " %f", value);
										if ( derivatives )
										{
											if (Curve_get_node_derivatives(
												curve, j, k, &derivative) &&
												Curve_get_node_scale_factor_dparameter(
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
							if (Curve_get_node_values(curve, elements,
								nodes_per_element-1, &value))
							{
								fprintf(file, " %f", value);
								if ( derivatives )
								{
									if (Curve_get_node_derivatives(
										curve, elements, nodes_per_element-1, &derivative) &&
										Curve_get_node_scale_factor_dparameter(
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

static int emoter_export_nodes(struct Emoter_dialog *emoter_dialog,
	char *filename)
/*******************************************************************************
LAST MODIFIED : 9 September 1999

DESCRIPTION :

==============================================================================*/
{
	char *command;
	int frame, number_of_frames, return_code;
	FE_value time;
	struct Shared_emoter_slider_data *shared;

	ENTER(emoter_saveslider_CB);
	if ( emoter_dialog && filename)
	{
		return_code = 1;
		shared = emoter_dialog->shared;
		if ( ALLOCATE( command, char, strlen(filename)+100 ))
		{
			number_of_frames = (int)floor( emoter_dialog->time_maximum -
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

static int emoter_autoplay_timeout(void *emoter_dialog_void)
/*******************************************************************************
LAST MODIFIED : 17 August 2006

DESCRIPTION :
Sets the <emoter_dialog> to autoplay if <play> is true or to stop if <play> is false.
==============================================================================*/
{
	FE_value time;
	int return_code;
	struct Emoter_dialog *emoter_dialog;

	ENTER(emoter_autoplay);
	emoter_dialog = (struct Emoter_dialog *)emoter_dialog_void;
	if (emoter_dialog)
	{
		time = emoter_dialog->shared->time + 1;
		if ( time > emoter_dialog->time_maximum )
		{
			time = emoter_dialog->time_minimum;
		}
		emoter_set_play_slider_value ( emoter_dialog, time );
		emoter_update_face( emoter_dialog->shared );

	  	emoter_dialog->autoplay_timeout = 
			Event_dispatcher_add_timeout_callback(
				User_interface_get_event_dispatcher(
					emoter_dialog->shared->user_interface), 0, /*ns*/50000000,
				emoter_autoplay_timeout, emoter_dialog);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"emoter_autoplay.  Invalid emoter_dialog.");
		return_code = 0;
	}

	LEAVE;
	return(return_code);
} /* emoter_autoplay_CB */

static int emoter_autoplay(struct Emoter_dialog *emoter_dialog,
	int play)
/*******************************************************************************
LAST MODIFIED : 17 August 2006

DESCRIPTION :
Sets the <emoter_dialog> to autoplay if <play> is true or to stop if <play> is false.
==============================================================================*/
{
	int return_code;

	ENTER(emoter_autoplay);
	if (emoter_dialog)
	{
		if (play)
		{
			if (!emoter_dialog->autoplay_timeout)
			{
				emoter_dialog->autoplay_timeout = 
					Event_dispatcher_add_timeout_callback(
						User_interface_get_event_dispatcher(
							emoter_dialog->shared->user_interface), 0, 1000,
						emoter_autoplay_timeout, emoter_dialog);
			}
		}
		else
		{
			if (emoter_dialog->autoplay_timeout)
			{
				Event_dispatcher_remove_timeout_callback(
					User_interface_get_event_dispatcher(
						emoter_dialog->shared->user_interface),
					emoter_dialog->autoplay_timeout);
				emoter_dialog->autoplay_timeout =
					(struct Event_dispatcher_timeout_callback *)NULL;
			}
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"emoter_autoplay.  Invalid emoter_dialog.");
		return_code = 0;
	}

	LEAVE;
	return(return_code);
} /* emoter_autoplay_CB */

static int emoter_set_mode_limit(struct Emoter_dialog *emoter_dialog,
	int new_value )
/*******************************************************************************
LAST MODIFIED : 21 April 1998

DESCRIPTION :

==============================================================================*/
{
	int i, *node_numbers, number_of_nodes, return_code;

	ENTER(emoter_set_mode_limit);

	if ( emoter_dialog )
	{
		return_code = 1;
		if ( new_value > emoter_dialog->shared->number_of_modes )
		{
			new_value = emoter_dialog->shared->number_of_modes;
		}
		if ( new_value < 1 )
		{
			new_value = 1;
		}
		emoter_dialog->shared->mode_limit = new_value;
		node_numbers = emoter_dialog->shared->em_object->minimum_nodes;
		if (node_numbers)
		{
			number_of_nodes = new_value;
			Cmiss_field_module_id field_module = Cmiss_region_get_field_module(emoter_dialog->shared->region);
			Cmiss_field_module_begin_change(field_module);
			Cmiss_nodeset_id master_nodeset = Cmiss_field_module_find_nodeset_by_name(field_module, "cmiss_nodes");
			if (emoter_dialog->minimum_nodeset_group)
			{
				Cmiss_nodeset_group_remove_all_nodes(emoter_dialog->minimum_nodeset_group);
			}
			else
			{
				Cmiss_field_id minimum_node_group_field = Cmiss_field_module_create_node_group(field_module, master_nodeset);
				Cmiss_field_node_group_id minimum_node_group = Cmiss_field_cast_node_group(minimum_node_group_field);
				emoter_dialog->minimum_nodeset_group = Cmiss_field_node_group_get_nodeset(minimum_node_group);
				Cmiss_field_node_group_destroy(&minimum_node_group);
				Cmiss_field_destroy(&minimum_node_group_field);
				if (!emoter_dialog->minimum_nodeset_group)
				{
					display_message(ERROR_MESSAGE,
						"emoter_set_mode_limit.  Unable to create minimum node set.");
					return_code = 0;
				}
			}
			if (return_code)
			{
				for ( i = 0 ; i < number_of_nodes ; i++ )
				{
					if ( node_numbers[i] != -1 )
					{
						Cmiss_node_id node = Cmiss_nodeset_find_node_by_identifier(master_nodeset, node_numbers[i]);
						if ((0 == node) || !Cmiss_nodeset_group_add_node(emoter_dialog->minimum_nodeset_group, node))
						{
							display_message(ERROR_MESSAGE,
								"emoter_set_mode_limit.  Unable to add node %d", i);
						}
						Cmiss_node_destroy(&node);
					}
				}
			}
			Cmiss_nodeset_destroy(&master_nodeset);
			Cmiss_field_module_end_change(field_module);
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

static int emoter_convert_active_slider(struct Emoter_dialog *emoter_dialog)
/*******************************************************************************
LAST MODIFIED : 9 September 1999

DESCRIPTION :

==============================================================================*/
{
	char *var_name;
	FE_value time, **temp_data;
	int i, j, frame, number_of_frames, solid_body_modes, mode_offset, return_code;
	struct Emoter_slider *active, *combine_slider;
	struct Shared_emoter_slider_data *shared;
	struct Curve *temp_var;

	ENTER(emoter_convert_active_slider);

	if ( emoter_dialog  && emoter_dialog->shared &&
			emoter_dialog->shared->active_slider )
	{
		return_code = 1;
		shared = emoter_dialog->shared;
		active = shared->active_slider;

		number_of_frames = (int)floor( emoter_dialog->time_maximum -
			emoter_dialog->time_minimum + 1);
		if ( ALLOCATE( temp_data, FE_value *, number_of_frames ))
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
				GET_NAME(Curve)(active->mode_curve,&var_name );
				temp_var = active->mode_curve;
				DEACCESS(Curve)(&active->mode_curve);
				REMOVE_OBJECT_FROM_MANAGER(Curve)
				(temp_var, active->shared->curve_manager);
			}
			else
			{
				ALLOCATE(var_name, char, strlen(active->name) + 1);
				strcpy( var_name, active->name );
			}
			while (FIND_BY_IDENTIFIER_IN_MANAGER(Curve,name)(
				var_name, shared->curve_manager))
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
			active->mode_curve = CREATE(Curve)(var_name,
				LINEAR_LAGRANGE, shared->number_of_modes + solid_body_modes);
			if (active->mode_curve)
			{
				if (ADD_OBJECT_TO_MANAGER(Curve)(
					active->mode_curve, shared->curve_manager))
				{
					/* Set the type after the object is added to the
							manager so that ADD message is ignored */
					Curve_set_type(active->mode_curve,
						CURVE_TYPE_EMOTER_MODES );
					ACCESS(Curve)(active->mode_curve);
					/* SAB Do the first and second nodes as special case
							as we need to create the first element, position both
							nodes and then from then on the current elements can
							just be appended to */
					Curve_add_element( active->mode_curve, 1 );
					Curve_set_parameter( active->mode_curve,
						/*element_no*/1,/*local_node_no*/0,	time);
					time += 1.0;
					Curve_set_parameter( active->mode_curve,
						/*element_no*/1,/*local_node_no*/1,	time);
					if ( temp_data[frame] )
					{
						Curve_set_node_values( active->mode_curve,
							1, 0, temp_data[frame] + mode_offset);
					}
					if ( number_of_frames > 1 )
					{
						frame++;
					}

					if ( temp_data[frame] )
					{
						Curve_set_node_values( active->mode_curve,
							1, 1, temp_data[frame] + mode_offset);
					}

					frame = 2;
					while (frame < number_of_frames)
					{
						Curve_add_element( active->mode_curve, frame );
						time += 1.0;
						Curve_set_parameter( active->mode_curve,
							/*element_no*/frame,/*local_node_no*/1,	time);
						if ( temp_data[frame] )
						{
							Curve_set_node_values( active->mode_curve,
								frame, 1, temp_data[frame] + mode_offset);
						}
						frame++;
					}
				}
				else
				{
					DESTROY(Curve)(&active->mode_curve);
					display_message(ERROR_MESSAGE,
						"read_emoter_mode_Curve.  Unable to add control curve to manager");
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
				DEACCESS(Curve)( &((active->combine_sliders[i])->curve));
				REMOVE_OBJECT_FROM_MANAGER(Curve)
				(temp_var, shared->curve_manager);

				combine_slider = (active->combine_sliders[i])->slider;
				temp_var = (active->combine_sliders[i])->timebase_curve;
				if (temp_var)
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
				DEACCESS(Curve)( &((active->combine_sliders[i])->timebase_curve));
				REMOVE_OBJECT_FROM_MANAGER(Curve)
				(temp_var, shared->curve_manager);
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

static int Curve_update_emoter_sliders(
	struct Curve *curve, void *emoter_dialog_void)
/*******************************************************************************
LAST MODIFIED : 30 May 2001

DESCRIPTION :
Updates existing control curves combine sliders.
==============================================================================*/
{
	enum Curve_type type;
	int return_code;
	struct Emoter_dialog *emoter_dialog;

	ENTER(Curve_update_emoter_sliders);
	if (curve &&
		(emoter_dialog = (struct Emoter_dialog *)emoter_dialog_void))
	{
		Curve_get_type( curve, &type );
		if (type == CURVE_TYPE_EMOTER_COMBINE)
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
			"Curve_update_emoter_sliders.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Curve_update_emoter_sliders */

static int Curve_add_emoter_sliders(
	struct Curve *curve, void *emoter_dialog_void)
/*******************************************************************************
LAST MODIFIED : 30 May 2001

DESCRIPTION :
Adds new control curves combine sliders.
==============================================================================*/
{
	char *name;
	enum Curve_type type;
	int return_code;
	struct Emoter_dialog *emoter_dialog;
	struct Shared_emoter_slider_data *shared;

	ENTER(Curve_add_emoter_sliders);
	if (curve &&
		(emoter_dialog = (struct Emoter_dialog *)emoter_dialog_void))
	{
		shared = emoter_dialog->shared;

		Curve_get_type( curve, &type );
		if ( type == CURVE_TYPE_EMOTER_MODES )
		{
			if (GET_NAME(Curve)( curve, &name ))
			{
				create_emoter_slider((char *)NULL,
					name,
					1.0, shared,
					shared->number_of_sliders,
					curve, emoter_dialog, /*no_confirm*/0);
				DEALLOCATE( name );
			}
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Curve_add_emoter_sliders.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Curve_add_emoter_sliders */

static void emoter_curve_manager_message(
	struct MANAGER_MESSAGE(Curve) *message, void *emoter_dialog_void)
/*******************************************************************************
LAST MODIFIED : 30 May 2001

DESCRIPTION :
Something has changed globally in the control curve manager.
==============================================================================*/
{
	ENTER(emoter_curve_manager_message);
	if (message && emoter_dialog_void)
	{
		struct LIST(Curve) *changed_curve_list =
			MANAGER_MESSAGE_GET_CHANGE_LIST(Curve)(message,
				MANAGER_CHANGE_RESULT(Curve));
		if (changed_curve_list)
		{
			FOR_EACH_OBJECT_IN_LIST(Curve)(
				Curve_update_emoter_sliders, emoter_dialog_void,
				changed_curve_list);
			DESTROY_LIST(Curve)(&changed_curve_list);
		}
		changed_curve_list =MANAGER_MESSAGE_GET_CHANGE_LIST(Curve)(message,
			MANAGER_CHANGE_ADD(Curve));
		if (changed_curve_list)
		{
			FOR_EACH_OBJECT_IN_LIST(Curve)(
				Curve_add_emoter_sliders, emoter_dialog_void,
				changed_curve_list);
			DESTROY_LIST(Curve)(&changed_curve_list);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"emoter_curve_manager_message.  Invalid argument(s)");
	}
	LEAVE;
} /* emoter_curve_manager_message */

static int create_emoter_dialog(struct Emoter_dialog **emoter_dialog_address,
	struct Shared_emoter_slider_data *shared_data)
/*******************************************************************************
LAST MODIFIED : 15 June 1998

DESCRIPTION :
Create emoter controls.
==============================================================================*/
{
	int return_code;
	struct Emoter_dialog *emoter_dialog = NULL;

	ENTER(create_emoter_dialog);

	return_code = 0;
	/* allocate memory */
	if (ALLOCATE(emoter_dialog, struct Emoter_dialog, 1))
	{
		/* initialise the structure */
		emoter_dialog->shared = shared_data;
		emoter_dialog->dialog_address = emoter_dialog_address;
		emoter_dialog->time_minimum = 1;
		emoter_dialog->time_maximum = 400;
		emoter_dialog->play_slider_position = -1;
		emoter_dialog->curve_manager_callback_id = (void *)NULL;
		emoter_dialog->movie = (struct Movie_graphics *)NULL;
		emoter_dialog->movie_loop = 1;
		emoter_dialog->movie_every_frame = 1;
		emoter_dialog->minimum_nodeset_group = (Cmiss_nodeset_group_id)0;
		emoter_dialog->shell_list_item = (struct Shell_list_item *)NULL;
		emoter_dialog->autoplay_timeout =
			(struct Event_dispatcher_timeout_callback *)NULL;


		emoter_set_play_slider_range( emoter_dialog,
			emoter_dialog->time_minimum,
			emoter_dialog->time_maximum );
		emoter_set_play_slider_value( emoter_dialog,
			shared_data->time );
		emoter_set_mode_limit( emoter_dialog,
			shared_data->mode_limit );
		emoter_dialog->curve_manager_callback_id=
			MANAGER_REGISTER(Curve)(
				emoter_curve_manager_message,
				(void *)emoter_dialog,
				emoter_dialog->shared->curve_manager);
		*(emoter_dialog->dialog_address) = emoter_dialog;
		return_code = 1;

	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_emoter_dialog.  Could not allocate emoter_dialog structure");
	}

	LEAVE;

	return (return_code);
} /* create_emoter_dialog */

int bring_up_emoter_dialog(struct Emoter_dialog **emoter_dialog_address,
	struct Shared_emoter_slider_data *shared_data)
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
			return_code=1;
		}
		else
		{
			return_code = create_emoter_dialog(emoter_dialog_address,
				shared_data);
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
				get_FE_node_identifier(node);
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

/***************************************************************************//**
 * Executes a GFX CREATE EMOTER command.
 */
int gfx_create_emoter(struct Parse_state *state,void *dummy_to_be_modified,
	void *create_emoter_slider_data_void)
{
	char *basis_file_name, minimum_nodeset_flag;
	int i,*index_nodes,number_of_modes,number_of_nodes,number_of_index_nodes,
		return_code;
	struct Create_emoter_slider_data *create_emoter_slider_data;
	struct FE_region *fe_region;
	struct Index_list_data index_list_data;
	struct Option_table *option_table;
	struct Shared_emoter_slider_data *shared_emoter_slider_data;
	struct EM_Object *em_object;

	ENTER(gfx_create_emoter);
	USE_PARAMETER(dummy_to_be_modified);
	return_code=0;
	/* check arguments */
	if (state&&(create_emoter_slider_data=
		(struct Create_emoter_slider_data *)
		create_emoter_slider_data_void)&&
		(create_emoter_slider_data->emoter_dialog_address))
	{
		struct Cmiss_region *region =
			ACCESS(Cmiss_region)(create_emoter_slider_data->root_region);
		index_nodes = (int *)NULL;
		number_of_index_nodes = 0;
		minimum_nodeset_flag = 0;
		basis_file_name = (char *)NULL;
		int transform_graphics = 0;

		option_table = CREATE(Option_table)();
		Option_table_add_string_entry(option_table, "basis", &basis_file_name,
			" BASIS_FILE_NAME");
		Option_table_add_char_flag_entry(option_table, "minimum_nodeset",
			&minimum_nodeset_flag);
		Option_table_add_set_Cmiss_region(option_table, "region",
			create_emoter_slider_data->root_region, &region);
		Option_table_add_switch(option_table, "transform_graphics",
			"transform_nodes", &transform_graphics);
		return_code = Option_table_multi_parse(option_table, state);
		DESTROY(Option_table)(&option_table);

		if (return_code)
		{
			if (!create_emoter_slider_data->user_interface)
			{
				display_message(ERROR_MESSAGE,
					"gfx_create_emoter.  Missing user_interface");
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
				em_object=(struct EM_Object *)NULL;
				fe_region = Cmiss_region_get_FE_region(region);
				if (fe_region)
				{
					number_of_index_nodes = FE_region_get_number_of_FE_nodes(fe_region);
					if (ALLOCATE(index_nodes, int, number_of_index_nodes))
					{
						index_list_data.number_of_index_nodes = 0;
						index_list_data.maximum_index_nodes = number_of_index_nodes;
						index_list_data.index_nodes = index_nodes;
						FE_region_for_each_FE_node(fe_region, add_FE_node_number_to_list,
							(void *)&index_list_data);
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
				else
				{
					display_message(ERROR_MESSAGE,
						"gfx_create_emoter.  Missing fe_region.");
					return_code = 0;
				}
			}
			if (return_code)
			{
				if (EM_read_basis(basis_file_name,create_emoter_slider_data->io_stream_package,
					&em_object,index_nodes,number_of_index_nodes))
				{
					number_of_nodes=EM_number_of_nodes(em_object);
					number_of_modes=EM_number_of_modes(em_object);
					if ((0<number_of_nodes)&&(0<number_of_modes))
					{
						/* check that all the nodes exist */
						i=0;
						while ((i<number_of_nodes)&&FE_region_get_FE_node_from_identifier(
						   fe_region,(em_object->index)[i]))
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
								shared_emoter_slider_data->region = ACCESS(Cmiss_region)(region);
								shared_emoter_slider_data->show_solid_body_motion = 1;
								shared_emoter_slider_data->curve_manager
									= create_emoter_slider_data->curve_manager;
								shared_emoter_slider_data->execute_command =
									create_emoter_slider_data->execute_command;
								shared_emoter_slider_data->em_object = em_object;
								shared_emoter_slider_data->active_slider =
									(struct Emoter_slider *)NULL;
								shared_emoter_slider_data->time = 1;
								shared_emoter_slider_data->graphics_buffer_package
									= create_emoter_slider_data->graphics_buffer_package;
								shared_emoter_slider_data->user_interface
									= create_emoter_slider_data->user_interface;
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
								shared_emoter_slider_data->graphics_window_manager
									= create_emoter_slider_data->graphics_window_manager;
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
								shared_emoter_slider_data->viewer_scene
									= create_emoter_slider_data->viewer_scene;
								shared_emoter_slider_data->viewer_background_colour
									= create_emoter_slider_data->viewer_background_colour;
								shared_emoter_slider_data->viewer_light
									= create_emoter_slider_data->viewer_light;
								shared_emoter_slider_data->viewer_light_model
									= create_emoter_slider_data->viewer_light_model;
								shared_emoter_slider_data->transform_graphics = transform_graphics;
								shared_emoter_slider_data->io_stream_package
									= create_emoter_slider_data->io_stream_package;
								return_code=bring_up_emoter_dialog(
									create_emoter_slider_data->emoter_dialog_address,
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
		if (basis_file_name)
		{
			DEALLOCATE(basis_file_name);
		}
		DEACCESS(Cmiss_region)(&region);
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
	void *dummy_to_be_modified, void *emoter_dialog_void)
/*******************************************************************************
LAST MODIFIED : 7 September 1999

DESCRIPTION :
Executes a GFX MODIFY EMOTER command.
==============================================================================*/
{
	char activate, convert_data, *export_filename, *filename, 
		*input_sequence, keyframe, maximum_time_flag, minimum_time_flag, 
		*movie_filename, new_flag, no_rigid_body_motion, play, rigid_body_motion,
		*save_filename, *slidername, stop, *temp_filename, time_flag, value_flag;
	double maximum_time, minimum_time, time, value;
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
		{"set_maximum_time",NULL,NULL,set_double_and_char_flag},
		{"set_minimum_time",NULL,NULL,set_double_and_char_flag},
		{"set_time",NULL,NULL,set_double_and_char_flag},
		{"set_value",NULL,NULL,set_double_and_char_flag},
		{"slider",NULL,(void *)1,set_name},
		{"stop",NULL,NULL,set_char_flag},
		{NULL,NULL,NULL,NULL}
	};
	struct Emoter_dialog *emoter_dialog;
	struct Emoter_slider *emoter_slider;
	struct Shared_emoter_slider_data *shared;

	ENTER(gfx_modify_emoter);
	USE_PARAMETER(dummy_to_be_modified);
	if (state)
	{
		emoter_dialog = static_cast<Emoter_dialog *>(emoter_dialog_void);
		if (emoter_dialog)
		{
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
		new_flag = 0;
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
		(option_table[7]).to_be_modified=&new_flag;
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
						}
						else
						{
							emoter_dialog->shared->input_sequence = (char *)NULL;
						}
						face_changed = 1;
					}
					if (time_flag)
					{
						integer_time = (int)time;
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
						emoter_autoplay(emoter_dialog, /*play*/1);
					}
					if (stop)
					{
						emoter_autoplay(emoter_dialog, /*play*/0);
					}
					if (rigid_body_motion)
					{
						emoter_dialog->shared->show_solid_body_motion = 1;
						face_changed = 1;
					}
					if (no_rigid_body_motion)
					{
						emoter_dialog->shared->show_solid_body_motion = 0;
						face_changed = 1;
					}
					if (new_flag)
					{
						create_emoter_slider((char *)NULL,
							"new", 
							1, emoter_dialog->shared,
							emoter_dialog->shared->number_of_sliders,
							(struct Curve *)NULL,
							emoter_dialog, /*no_confirm*/1 ); 
					}
					if (filename)
					{
						temp_filename = strrchr(filename, '/');
						if (temp_filename)
						{
							temp_filename++;
						}
						else
						{
							temp_filename = filename;
						}
						create_emoter_slider(filename,
							temp_filename, 
							1, emoter_dialog->shared,
							emoter_dialog->shared->number_of_sliders,
							(struct Curve *)NULL,
							emoter_dialog, /*no_confirm*/1 ); 
						DEALLOCATE(filename);
					}
					if (save_filename)
					{
						emoter_save_slider(emoter_dialog->shared->active_slider,
							save_filename, emoter_dialog);
						DEALLOCATE(save_filename);
					}
					if (face_changed)
					{
						emoter_update_face (emoter_dialog->shared);
#if defined (USE_CMGUI_GRAPHICS_WINDOW)
						struct Graphics_window *graphics_window;
						/* SAB This would be better implemented by adding a special
							manager message to the Scene Manager and then calling that. */
						graphics_window=FIRST_OBJECT_IN_MANAGER_THAT(Graphics_window)(
							(MANAGER_CONDITIONAL_FUNCTION(Graphics_window) *)NULL,
							(void *)NULL,emoter_dialog->shared->graphics_window_manager);
						if (graphics_window)
						{
							Graphics_window_update_now(graphics_window);
						}
#endif /* defined (USE_CMGUI_GRAPHICS_WINDOW) */
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
