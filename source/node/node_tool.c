/*******************************************************************************
FILE : node_tool.c

LAST MODIFIED : 4 July 2002

DESCRIPTION :
Functions for mouse controlled node position and vector editing based on
Scene input.
==============================================================================*/
#include <math.h>
#if defined (MOTIF)
#include <Xm/Protocols.h>
#include <Xm/MwmUtil.h>
#include <Xm/Xm.h>
#include <Xm/ToggleBG.h>
#include "choose/choose_computed_field.h"
#include "choose/choose_node_group.h"
#endif /* defined (MOTIF) */
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_composite.h"
#include "computed_field/computed_field_finite_element.h"
#include "computed_field/computed_field_wrappers.h"
#include "general/debug.h"
#include "general/matrix_vector.h"
#include "graphics/element_group_settings.h"
#include "graphics/graphical_element.h"
#include "graphics/graphics_object.h"
#include "help/help_interface.h"
#include "interaction/interaction_graphics.h"
#include "interaction/interaction_volume.h"
#include "interaction/interactive_event.h"
#include "node/node_operations.h"
#include "node/node_tool.h"
#if defined (MOTIF)
#include "node/node_tool.uidh"
#include "motif/image_utilities.h"
#endif /* defined (MOTIF) */
#include "user_interface/gui_dialog_macros.h"
#include "user_interface/message.h"

/*
Module variables
----------------
*/

#if defined (MOTIF)
static int node_tool_hierarchy_open=0;
static MrmHierarchy node_tool_hierarchy;
#endif /* defined (MOTIF) */

static char Interactive_tool_node_type_string[] = "node_tool";

/*
Module types
------------
*/

struct Node_tool
/*******************************************************************************
LAST MODIFIED : 4 July 2002

DESCRIPTION :
Object storing all the parameters for converting scene input messages into
changes in node position and derivatives etc.
==============================================================================*/
{
	struct Execute_command *execute_command;
	struct MANAGER(Interactive_tool) *interactive_tool_manager;
	struct Interactive_tool *interactive_tool;
	struct MANAGER(FE_node) *node_manager;
	/* flag indicating that the above manager is actually the data manager */
	int use_data;
	struct MANAGER(GROUP(FE_node)) *node_group_manager;
	void *node_group_manager_callback_id;
	/* needed for destroy button */
	struct MANAGER(FE_element) *element_manager;
	struct FE_node_selection *node_selection;
	struct Computed_field_package *computed_field_package;
	struct Graphical_material *rubber_band_material;
	struct Time_keeper *time_keeper;
	struct User_interface *user_interface;
	/* user-settable flags */
	/* indicates whether node edits can occur with motion_notify events: slower */
	int motion_update_enabled;
	/* indicates whether existing nodes can be selected */
	int select_enabled;
	/* indicates whether selected nodes can be edited */
	int edit_enabled;
	/* indicates whether new fields can be defined at nodes */
	int define_enabled;
	/* indicates whether new nodes will can be created */
	int create_enabled;
	/* if create_enabled, controls if create will be streaming, ie. creating
		 a stream of nodes where the user swipes, rather than just 1 */
	int streaming_create_enabled;
	enum Node_tool_edit_mode edit_mode;
	struct GROUP(FE_node) *node_group;
	struct Computed_field *coordinate_field, *url_field;
	struct FE_node *template_node;
	/* information about picked nodes the editor knows about */
	struct Scene_picked_object *scene_picked_object;
	struct FE_node *last_picked_node;
	int picked_node_was_unselected;
	int motion_detected;
	struct GT_element_group *gt_element_group;
	struct GT_element_settings *gt_element_settings;
	struct Interaction_volume *last_interaction_volume;
	struct GT_object *rubber_band;

#if defined (MOTIF)
	Display *display;
	Widget coordinate_field_form,coordinate_field_widget,create_button,
		define_button,edit_button,motion_update_button,node_group_form,
		node_group_widget,select_button,streaming_create_button,
		url_field_button,url_field_form,url_field_widget;
	Widget widget,window_shell;
#endif /* defined (MOTIF) */
}; /* struct Node_tool */

struct FE_node_edit_information
/*******************************************************************************
LAST MODIFIED : 20 November 2000

DESCRIPTION :
Describes how to move a node in space. The node will move on the plane normal
to the viewing direction a distance proportional to the two starting and
finishing points on the near and far plane. The exact amount is in proportion
to its position between these two planes.
==============================================================================*/
{
	/* the actual coordinate change calculated from the drag at the last picked
		 node */
	double delta1,delta2,delta3;
	/* the current value of the time being used */
	FE_value time;
	/* the gesture indicated by the mouse is given by initial and final
		 interaction volumes */
	struct Interaction_volume *final_interaction_volume,
		*initial_interaction_volume;
	/* the field to translate */
	struct Computed_field *coordinate_field;
	/* The same field wrapped to get RC coordinates */
	struct Computed_field *rc_coordinate_field;
	/* following required for EDIT_VECTOR only */
	struct Computed_field *orientation_scale_field,
		*wrapper_orientation_scale_field, *variable_scale_field;
	Triple glyph_centre, glyph_scale_factors, glyph_size;
	/* need the node manager for global modify */
	struct MANAGER(FE_node) *node_manager;
	/* only edit the node if in the node_group, if supplied */
	struct GROUP(FE_node) *node_group;
	/* information for undoing scene object transformations - only needs to be
		 done if transformation_required is set */
	int transformation_required,LU_indx[4];
	double transformation_matrix[16],LU_transformation_matrix[16];
	/* The last_picked node is used to calculate the delta change and so when
		the whole active group is looped over this node is ignored */
	struct FE_node *last_picked_node;
}; /* struct FE_node_edit_information */

/*
Module functions
----------------
*/

#if defined (MOTIF)
DECLARE_DIALOG_IDENTIFY_FUNCTION(node_tool,Node_tool,coordinate_field_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(node_tool,Node_tool,create_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(node_tool,Node_tool,define_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(node_tool,Node_tool,edit_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(node_tool,Node_tool,motion_update_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(node_tool,Node_tool,node_group_form)
DECLARE_DIALOG_IDENTIFY_FUNCTION(node_tool,Node_tool,select_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(node_tool,Node_tool,streaming_create_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(node_tool,Node_tool,url_field_button)
DECLARE_DIALOG_IDENTIFY_FUNCTION(node_tool,Node_tool,url_field_form)
#endif /* defined (MOTIF) */

static int Node_tool_define_field_at_node(struct Node_tool *node_tool,
	struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 16 October 2001

DESCRIPTION :
Defines the appropriate FE_field upon which the <coordinate_field> depends in
<node>. The field is defined with no versions or derivatives.
==============================================================================*/
{
	int return_code;
	struct FE_field *fe_field;
	struct FE_node_field_creator *node_field_creator;
	struct LIST(FE_field) *fe_field_list;

	ENTER(Node_tool_define_field_at_node);
	if (node_tool && node)
	{
		if (node_tool->coordinate_field && (fe_field_list=
			Computed_field_get_defining_FE_field_list(node_tool->coordinate_field,
				Computed_field_package_get_computed_field_manager(
					node_tool->computed_field_package))))
		{
			if ((1==NUMBER_IN_LIST(FE_field)(fe_field_list))&&
				(fe_field=FIRST_OBJECT_IN_LIST_THAT(FE_field)(
				(LIST_CONDITIONAL_FUNCTION(FE_field) *)NULL,(void *)NULL,
				fe_field_list)) && (3 >= get_FE_field_number_of_components(
				fe_field)) && (FE_VALUE_VALUE == get_FE_field_value_type(fe_field)))
			{
				if (node_field_creator = CREATE(FE_node_field_creator)(
					/*number_of_components*/3))
				{
					if (define_FE_field_at_node(node,fe_field,
						(struct FE_time_version *)NULL,
						node_field_creator))
					{
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"Node_tool_define_field_at_node.  Failed");
						return_code=0;
					}
					DESTROY(FE_node_field_creator)(&node_field_creator);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Node_tool_define_field_at_node.  Unable to make creator.");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Node_tool_define_field_at_node.  Invalid field");
				return_code=0;
			}
			DESTROY(LIST(FE_field))(&fe_field_list);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Node_tool_define_field_at_node.  No field to define");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_define_field_at_node.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_tool_define_field_at_node */

static int Node_tool_create_template_node(struct Node_tool *node_tool)
/*******************************************************************************
LAST MODIFIED : 12 September 2000

DESCRIPTION :
Ensures there is a template node defined with the coordinate_field in the
<node_tool> for creating new nodes as copies of.
==============================================================================*/
{
	int return_code;

	ENTER(Node_tool_create_template_node);
	if (node_tool)
	{
		if (!node_tool->template_node)
		{
			if (node_tool->coordinate_field)
			{
				if (node_tool->node_group)
				{
					if (node_tool->template_node=CREATE(FE_node)(
						/*node_number*/0,(struct FE_node *)NULL))
					{
						if (Node_tool_define_field_at_node(node_tool,
							node_tool->template_node))
						{
							ACCESS(FE_node)(node_tool->template_node);
							return_code=1;
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Node_tool_create_template_node.  "
								"Could not define coordinate field in template_node");
							DESTROY(FE_node)(&(node_tool->template_node));
							return_code=0;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"Node_tool_create_template_node.  "
							"Could not create template node");
						return_code=0;
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Node_tool_create_template_node.  Must specify a node group first");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Node_tool_create_template_node.  No field to define");
				return_code=0;
			}
		}
		else
		{
			/* already have a template node */
			return_code=1;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_create_template_node.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_tool_create_template_node */

static int model_to_world_coordinates(FE_value coordinates[3],
	double *transformation_matrix)
/*******************************************************************************
LAST MODIFIED : 23 July 1999

DESCRIPTION :
Makes a homogoneous coordinate [x,y,z,1] out of <coordinates> and premultiplies
it by the 16 value 4x4 <transformation_matrix> to give [x',y',z',h']. If h' is
non-zero the <coordinates> are modified to [x'/h',y'/h',z'/h'], otherwise an
error is reported.
==============================================================================*/
{
	double h,model_coordinates[4],world_coordinates[4];
	int return_code;

	ENTER(model_to_world_coordinates);
	if (coordinates&&transformation_matrix)
	{
		model_coordinates[0]=(double)coordinates[0];
		model_coordinates[1]=(double)coordinates[1];
		model_coordinates[2]=(double)coordinates[2];
		model_coordinates[3]=1.0;
		if (multiply_matrix(4,4,1,transformation_matrix,
			model_coordinates,world_coordinates)&&
			(0.0 != (h=world_coordinates[3])))
		{
			coordinates[0]=(FE_value)(world_coordinates[0] / h);
			coordinates[1]=(FE_value)(world_coordinates[1] / h);
			coordinates[2]=(FE_value)(world_coordinates[2] / h);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"model_to_world_coordinates.  Invalid transformation");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"model_to_world_coordinates.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* model_to_world_coordinates */

static int world_to_model_coordinates(FE_value coordinates[3],
	double *LU_transformation_matrix,int *LU_indx)
/*******************************************************************************
LAST MODIFIED : 23 July 1999

DESCRIPTION :
Makes a homogoneous coordinate [x',y',z',1] out of <coordinates> and solves for
the homogeneous model_coordinates [x,y,z,h] using the already-decomposed
16 value 4x4 <LU_transformation_matrix> and associated 4 value <LU_indx> vector.
If h is non-zero the <coordinates> are modified to [x/h,y/h,z/h], otherwise an
error is reported.
==============================================================================*/
{
	double h,model_coordinates[4];
	int return_code;

	ENTER(world_to_model_coordinates);
	if (coordinates&&LU_transformation_matrix)
	{
		model_coordinates[0]=(double)coordinates[0];
		model_coordinates[1]=(double)coordinates[1];
		model_coordinates[2]=(double)coordinates[2];
		model_coordinates[3]=1.0;
		if (LU_backsubstitute(4,LU_transformation_matrix,LU_indx,
			model_coordinates)&&(0.0 != (h=model_coordinates[3])))
		{
			coordinates[0]=(FE_value)(model_coordinates[0] / h);
			coordinates[1]=(FE_value)(model_coordinates[1] / h);
			coordinates[2]=(FE_value)(model_coordinates[2] / h);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"world_to_model_coordinates.  Invalid transformation");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"world_to_model_coordinates.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* world_to_model_coordinates */

static int FE_node_calculate_delta_position(struct FE_node *node,
	void *edit_info_void)
/*******************************************************************************
LAST MODIFIED : 27 April 2000

DESCRIPTION :
Calculates the delta change in the coordinates due to the ray supplied in the
<edit_info>.  This change is set inside the <edit_info> and this can then be 
applied to multiple nodes.
==============================================================================*/
{
	double model_coordinates[3],normalised_coordinates[3];
	FE_value coordinates[3], initial_coordinates[3], final_coordinates[3];
	int return_code;
	struct FE_node_edit_information *edit_info;

	ENTER(FE_node_calculate_delta_position);
	if (node&&(edit_info=(struct FE_node_edit_information *)edit_info_void)&&
		edit_info->node_manager&&edit_info->rc_coordinate_field&&
		(3>=Computed_field_get_number_of_components(
			edit_info->rc_coordinate_field)))
	{
		return_code=1;
		/* clear coordinates in case less than 3 dimensions */
		coordinates[0]=0.0;
		coordinates[1]=0.0;
		coordinates[2]=0.0;
		if (Computed_field_evaluate_at_node(edit_info->rc_coordinate_field,
			node,edit_info->time,coordinates))
		{
			initial_coordinates[0] = coordinates[0];
			initial_coordinates[1] = coordinates[1];
			initial_coordinates[2] = coordinates[2];
			if (edit_info->transformation_required)
			{
				return_code=model_to_world_coordinates(coordinates,
					edit_info->transformation_matrix);
			}
			if (return_code)
			{
				/* convert initial model coordinates into normalised coordinates in the
					 space of the initial_interaction_volume, and back into model
					 coordinates in the space of the final_interaction_volume to get
					 translation of node */
				model_coordinates[0]=(double)coordinates[0];
				model_coordinates[1]=(double)coordinates[1];
				model_coordinates[2]=(double)coordinates[2];
				return_code=Interaction_volume_model_to_normalised_coordinates(
					edit_info->initial_interaction_volume,model_coordinates,
					normalised_coordinates)&&
					Interaction_volume_normalised_to_model_coordinates(
						edit_info->final_interaction_volume,normalised_coordinates,
						model_coordinates);
				coordinates[0]=(FE_value)model_coordinates[0];
				coordinates[1]=(FE_value)model_coordinates[1];
				coordinates[2]=(FE_value)model_coordinates[2];
			}
			if (return_code&&edit_info->transformation_required)
			{
				return_code=world_to_model_coordinates(coordinates,
					edit_info->LU_transformation_matrix,edit_info->LU_indx);
			}
			if (return_code)
			{
				edit_info->last_picked_node = node;
				if (edit_info->coordinate_field != edit_info->rc_coordinate_field)
				{
					/* get delta of coordinate_field from change of rc_coordinate_field */
					return_code=Computed_field_evaluate_at_node(
						edit_info->coordinate_field,node,edit_info->time,
						initial_coordinates)&&
						Computed_field_set_values_at_managed_node(
							edit_info->rc_coordinate_field,node,coordinates,
							edit_info->node_manager)&&
						Computed_field_evaluate_at_node(edit_info->coordinate_field,
							node,edit_info->time,final_coordinates);
					edit_info->delta1 = final_coordinates[0] - initial_coordinates[0];
					edit_info->delta2 = final_coordinates[1] - initial_coordinates[1];
					edit_info->delta3 = final_coordinates[2] - initial_coordinates[2];
				}
				else
				{
					edit_info->delta1 = coordinates[0] - initial_coordinates[0];
					edit_info->delta2 = coordinates[1] - initial_coordinates[1];
					edit_info->delta3 = coordinates[2] - initial_coordinates[2];
					return_code=Computed_field_set_values_at_managed_node(
						edit_info->rc_coordinate_field,node,coordinates,
						edit_info->node_manager);
				}
			}
		}
		else
		{
			return_code=0;
		}
		/* always clear caches of evaluated fields */
		Computed_field_clear_cache(edit_info->rc_coordinate_field);
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"FE_node_calculate_delta_position.  Failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_calculate_delta_position.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_calculate_delta_position */

static int FE_node_edit_position(struct FE_node *node,void *edit_info_void)
/*******************************************************************************
LAST MODIFIED : 15 September 2000

DESCRIPTION :
Translates the <rc_coordinate_field> of <node> according to the delta change
stored in the <edit_info>.
==============================================================================*/
{
	FE_value coordinates[3];
	int return_code;
	struct FE_node_edit_information *edit_info;

	ENTER(FE_node_edit_position);
	if (node&&(edit_info=(struct FE_node_edit_information *)edit_info_void)&&
		edit_info->node_manager&&edit_info->rc_coordinate_field&&
		(3>=Computed_field_get_number_of_components(
			edit_info->rc_coordinate_field)))
	{
		return_code=1;
		/* the last_picked_node was edited in FE_node_calculate_delta_position.
			 Also, don't edit unless in node_group, if supplied */
		if ((node != edit_info->last_picked_node)&&((!edit_info->node_group)||
			IS_OBJECT_IN_GROUP(FE_node)(node,edit_info->node_group)))
		{
			/* clear coordinates in case less than 3 dimensions */
			coordinates[0]=0.0;
			coordinates[1]=0.0;
			coordinates[2]=0.0;
			/* If the field we are changing isn't defined at this node then we
				don't complain and just do nothing */
			if (Computed_field_is_defined_at_node(edit_info->coordinate_field,node)&&
				Computed_field_evaluate_at_node(edit_info->coordinate_field,
					node,edit_info->time,coordinates))
			{
				if (return_code)
				{
					/* interpolate near and far displacements to displacement at node */
					coordinates[0] += edit_info->delta1;
					coordinates[1] += edit_info->delta2;
					coordinates[2] += edit_info->delta3;
					if (return_code)
					{
						if (!Computed_field_set_values_at_managed_node(
							edit_info->coordinate_field,node,coordinates,
							edit_info->node_manager))
						{
							return_code=0;
						}
					}
				}
			}
			/* always clear caches of evaluated fields */
			Computed_field_clear_cache(edit_info->coordinate_field);
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,"FE_node_edit_position.  Failed");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_edit_position.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_edit_position */

static int FE_node_calculate_delta_vector(struct FE_node *node,
	void *edit_info_void)
/*******************************************************************************
LAST MODIFIED : 20 November 2000

DESCRIPTION :
Moves the end of the vector to exactly under the mouse, in the plane normal to the view direction at its current depth. Hence, this function should only be
called for a single node.
Note that you must supply an orientation_scale field, while glyph_size[0] and
glyph_centre[0] must be 0.0, and glyph_scale_factors[0] must be non-zero.
NOTE: currently does not tolerate having a variable_scale_field.
==============================================================================*/
{
	double model_coordinates[3],normalised_coordinates[3];
	FE_value a[3],b[3],c[3],coordinates[3],end_coordinates[3],old_coordinates[3],
		orientation_scale[9],scale_factor;
	int number_of_orientation_scale_components,return_code;
	struct FE_node_edit_information *edit_info;
	Triple size;

	ENTER(FE_node_calculate_delta_vector);
	if (node&&(edit_info=(struct FE_node_edit_information *)edit_info_void)&&
		edit_info->node_manager&&edit_info->rc_coordinate_field&&
		(3>=Computed_field_get_number_of_components(
			edit_info->rc_coordinate_field))&&
		edit_info->wrapper_orientation_scale_field&&
		(0<(number_of_orientation_scale_components=
			Computed_field_get_number_of_components(
				edit_info->wrapper_orientation_scale_field)))&&
		(9>=number_of_orientation_scale_components)&&
		(0.0 == edit_info->glyph_centre[0])&&
		(0.0 == edit_info->glyph_size[0])&&
		(0.0 != (scale_factor=edit_info->glyph_scale_factors[0])) &&
		((struct Computed_field *)NULL == edit_info->variable_scale_field))
	{
		return_code=1;
		/* clear coordinates in case less than 3 dimensions */
		coordinates[0]=0.0;
		coordinates[1]=0.0;
		coordinates[2]=0.0;
		if (Computed_field_evaluate_at_node(
			edit_info->wrapper_orientation_scale_field,node,edit_info->time,
			orientation_scale)&&
			Computed_field_evaluate_at_node(edit_info->rc_coordinate_field,
				node,edit_info->time,coordinates)&&
			make_glyph_orientation_scale_axes(number_of_orientation_scale_components,
				orientation_scale, a, b, c, size))
		{
			/* save old coordinates since will not change when converted back to
				 model coordinates */
			old_coordinates[0]=coordinates[0];
			old_coordinates[1]=coordinates[1];
			old_coordinates[2]=coordinates[2];
			end_coordinates[0]=coordinates[0]+size[0]*scale_factor*a[0];
			end_coordinates[1]=coordinates[1]+size[0]*scale_factor*a[1];
			end_coordinates[2]=coordinates[2]+size[0]*scale_factor*a[2];
			if (edit_info->transformation_required)
			{
				return_code=model_to_world_coordinates(coordinates,
					edit_info->transformation_matrix)&&
					model_to_world_coordinates(end_coordinates,
						edit_info->transformation_matrix);
			}
			if (return_code)
			{
				/* convert end_coordinates into normalised coordinates in the
					 space of the initial_interaction_volume, and back into model
					 coordinates centred in the space of the final_interaction_volume to
					 get new end point */
				model_coordinates[0]=(double)end_coordinates[0];
				model_coordinates[1]=(double)end_coordinates[1];
				model_coordinates[2]=(double)end_coordinates[2];
				return_code=Interaction_volume_model_to_normalised_coordinates(
					edit_info->initial_interaction_volume,model_coordinates,
					normalised_coordinates)&&
					Interaction_volume_centred_normalised_to_model_coordinates(
						edit_info->final_interaction_volume,normalised_coordinates,
						model_coordinates);
				end_coordinates[0]=(FE_value)model_coordinates[0];
				end_coordinates[1]=(FE_value)model_coordinates[1];
				end_coordinates[2]=(FE_value)model_coordinates[2];
			}
			if (edit_info->transformation_required)
			{
				return_code=world_to_model_coordinates(end_coordinates,
					edit_info->LU_transformation_matrix,edit_info->LU_indx);
			}
			if (return_code)
			{
				/* note use of old_coordinates in model space */
				a[0]=(end_coordinates[0]-old_coordinates[0])/scale_factor;
				a[1]=(end_coordinates[1]-old_coordinates[1])/scale_factor;
				a[2]=(end_coordinates[2]-old_coordinates[2])/scale_factor;
				switch (number_of_orientation_scale_components)
				{
					case 1:
					{
						/* scalar */
						edit_info->delta1=orientation_scale[0]=a[0];
					} break;
					case 2:
					case 4:
					{
						/* 1 or 2 2-D vectors */
						edit_info->delta1=orientation_scale[0]=a[0];
						edit_info->delta2=orientation_scale[1]=a[1];
					} break;
					case 3:
					case 6:
					case 9:
					{
						/* 1,2 or 3, 3-D vectors */
						edit_info->delta1=orientation_scale[0]=a[0];
						edit_info->delta2=orientation_scale[1]=a[1];
						edit_info->delta3=orientation_scale[2]=a[2];
					} break;
					default:
					{
						display_message(ERROR_MESSAGE,"FE_node_calculate_delta_vector.  "
							"Invalid number of orientation scale components");
						return_code=0;
					} break;
				}
			}
			if (return_code)
			{
				if (!Computed_field_set_values_at_managed_node(
					edit_info->wrapper_orientation_scale_field,node,orientation_scale,
					edit_info->node_manager))
				{
					return_code=0;
				}
				if (edit_info->orientation_scale_field !=
					edit_info->wrapper_orientation_scale_field)
				{
					/* get delta values from the orientation_scale_field */
					if (Computed_field_evaluate_at_node(
						edit_info->orientation_scale_field,node,
						edit_info->time,orientation_scale))
					{
						number_of_orientation_scale_components=
							Computed_field_get_number_of_components(
								edit_info->orientation_scale_field);
						switch (number_of_orientation_scale_components)
						{
							case 1:
							{
								/* scalar */
								edit_info->delta1=orientation_scale[0];
							} break;
							case 2:
							case 4:
							{
								/* 1 or 2 2-D vectors */
								edit_info->delta1=orientation_scale[0];
								edit_info->delta2=orientation_scale[1];
							} break;
							case 3:
							case 6:
							case 9:
							{
								/* 1,2 or 3, 3-D vectors */
								edit_info->delta1=orientation_scale[0];
								edit_info->delta2=orientation_scale[1];
								edit_info->delta3=orientation_scale[2];
							} break;
							default:
							{
								display_message(ERROR_MESSAGE,
									"FE_node_calculate_delta_vector.  "
									"Invalid number of orientation scale components");
								return_code=0;
							} break;
						}
					}
					else
					{
						return_code=0;
					}
				}
			}
		}
		else
		{
			return_code=0;
		}
		/* always clear caches of evaluated fields */
		Computed_field_clear_cache(edit_info->rc_coordinate_field);
		Computed_field_clear_cache(edit_info->wrapper_orientation_scale_field);
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,"FE_node_calculate_delta_vector.  Failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_calculate_delta_vector.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_calculate_delta_vector */

static int FE_node_edit_vector(struct FE_node *node,void *edit_info_void)
/*******************************************************************************
LAST MODIFIED : 22 March 2000

DESCRIPTION :
Translates the <rc_coordinate_field> of <node> according to the delta change
stored in the <edit_info>.
==============================================================================*/
{
	FE_value orientation_scale[9];
	int number_of_orientation_scale_components,return_code;
	struct FE_node_edit_information *edit_info;

	ENTER(FE_node_edit_vector);
	if (node&&(edit_info=(struct FE_node_edit_information *)edit_info_void)&&
		edit_info->node_manager&&edit_info->orientation_scale_field&&
		(0<(number_of_orientation_scale_components=
			Computed_field_get_number_of_components(
				edit_info->orientation_scale_field)))&&
		(9>=number_of_orientation_scale_components))
	{
		return_code=1;
		/* the last_picked_node was edited in FE_node_calculate_delta_vector.
			 Also, don't edit unless in node_group, if supplied */
		if ((node != edit_info->last_picked_node)&&((!edit_info->node_group)||
			IS_OBJECT_IN_GROUP(FE_node)(node,edit_info->node_group)))
		{
			if (Computed_field_evaluate_at_node(
				edit_info->orientation_scale_field,node,edit_info->time,
				orientation_scale))
			{
				switch (number_of_orientation_scale_components)
				{
					case 1:
					{
						/* scalar */
						orientation_scale[0]=edit_info->delta1;
					} break;
					case 2:
					case 4:
					{
						/* 1 or 2 2-D vectors */
						orientation_scale[0]=edit_info->delta1;
						orientation_scale[1]=edit_info->delta2;
					} break;
					case 3:
					case 6:
					case 9:
					{
						/* 1,2 or 3, 3-D vectors */
						orientation_scale[0]=edit_info->delta1;
						orientation_scale[1]=edit_info->delta2;
						orientation_scale[2]=edit_info->delta3;
					} break;
					default:
					{
						display_message(ERROR_MESSAGE,"FE_node_edit_vector.  "
							"Invalid number of orientation scale components");
						return_code=0;
					} break;
				}
				if (return_code)
				{
					if (!Computed_field_set_values_at_managed_node(
						edit_info->orientation_scale_field,node,orientation_scale,
						edit_info->node_manager))
					{
						return_code=0;
					}
				}
			}
			else
			{
				return_code=0;
			}
			/* always clear caches of evaluated fields */
			Computed_field_clear_cache(edit_info->orientation_scale_field);
			if (!return_code)
			{
				display_message(ERROR_MESSAGE,"FE_node_edit_vector.  Failed");
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_edit_vector.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_edit_vector */

static int Node_tool_define_field_at_node_from_picked_coordinates(
	struct Node_tool *node_tool,struct FE_node *node)
/*******************************************************************************
LAST MODIFIED : 29 September 2000

DESCRIPTION :
Defines the coordinate_field at the node using the position of the picked
object's coordinate field.
==============================================================================*/
{
	FE_value coordinates[3], time;
	int return_code;
	struct Computed_field *coordinate_field, *picked_coordinate_field,
		*rc_coordinate_field, *rc_picked_coordinate_field;

	ENTER(Node_tool_define_field_at_node_at_interaction_volume);
	if (node_tool&&node)
	{
		coordinate_field=node_tool->coordinate_field;
		if (node_tool->time_keeper)
		{
			time = Time_keeper_get_time(node_tool->time_keeper);
		}
		else
		{
			time = 0;
		}
		if (rc_coordinate_field=
			Computed_field_begin_wrap_coordinate_field(coordinate_field))
		{
			if (!(picked_coordinate_field = GT_element_settings_get_coordinate_field(
				node_tool->gt_element_settings)))
			{
				picked_coordinate_field = GT_element_group_get_default_coordinate_field(
					node_tool->gt_element_group);
			}
			rc_picked_coordinate_field = Computed_field_begin_wrap_coordinate_field(
				picked_coordinate_field);
			if (Computed_field_evaluate_at_node(rc_picked_coordinate_field,
				node,time,coordinates))
			{
				if (Node_tool_define_field_at_node(node_tool,node)&&
					Computed_field_set_values_at_node(rc_coordinate_field,
						node,coordinates))
				{
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Node_tool_define_field_at_node_from_picked_coordinates.  Failed");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Node_tool_define_field_at_node_from_picked_coordinates.  "
					"Unable to evaluate picked position.");
				return_code=0;
			}
			Computed_field_clear_cache(rc_picked_coordinate_field);
			Computed_field_end_wrap(&rc_picked_coordinate_field);
			Computed_field_clear_cache(rc_coordinate_field);
			Computed_field_end_wrap(&rc_coordinate_field);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Node_tool_define_field_at_node_from_picked_coordinates.  "
				"Could not wrap coordinate field");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_define_field_at_node_from_picked_coordinates.  "
			"Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_tool_define_field_at_node_from_picked_coordinates */

static struct FE_node *Node_tool_create_node_at_interaction_volume(
	struct Node_tool *node_tool,struct Scene *scene,
	struct Interaction_volume *interaction_volume)
/*******************************************************************************
LAST MODIFIED : 12 September 2000

DESCRIPTION :
Returns a new node in the position indicated by <interaction_volume>, in the
<scene> if suppled. If any of the remaining address arguments
are not NULL, they are filled with the appropriate information pertaining to
the new node.
==============================================================================*/
{
	double d,LU_transformation_matrix[16],node_coordinates[3];
	enum GT_element_settings_type gt_element_settings_type;
	FE_value coordinates[3];
	int i,LU_indx[4],node_number,transformation_required;
	struct Computed_field *coordinate_field,*rc_coordinate_field;
	struct FE_node *node;
	struct GT_element_group *gt_element_group;
	struct GT_element_settings *gt_element_settings;
	struct Scene_object *scene_object;
	LIST_CONDITIONAL_FUNCTION(Scene_object) *scene_object_conditional_function;
	struct Scene_picked_object *scene_picked_object;

	ENTER(Node_tool_create_node_at_interaction_volume);
	node=(struct FE_node *)NULL;
	scene_picked_object=(struct Scene_picked_object *)NULL;
	gt_element_group=(struct GT_element_group *)NULL;
	gt_element_settings=(struct GT_element_settings *)NULL;
	if (node_tool&&interaction_volume)
	{
		if (Node_tool_create_template_node(node_tool))
		{
			coordinate_field=node_tool->coordinate_field;
			if (node_tool->use_data)
			{
				scene_object_conditional_function=Scene_object_has_data_group;
				gt_element_settings_type=GT_ELEMENT_SETTINGS_DATA_POINTS;
			}
			else
			{
				scene_object_conditional_function=Scene_object_has_node_group;
				gt_element_settings_type=GT_ELEMENT_SETTINGS_NODE_POINTS;
			}
			if (scene&&(scene_object=first_Scene_object_in_Scene_that(scene,
				scene_object_conditional_function,(void *)node_tool->node_group)))
			{
				gt_element_group=Scene_object_get_graphical_element_group(scene_object);
				gt_element_settings=first_settings_in_GT_element_group_that(
					gt_element_group,GT_element_settings_type_matches,
					(void *)gt_element_settings_type);
				/* return a scene_object with the correct transformation */
				if (scene_picked_object=CREATE(Scene_picked_object)(/*hit_no*/0))
				{
					Scene_picked_object_add_Scene_object(scene_picked_object,
						scene_object);
					REACCESS(Scene_picked_object)(&(node_tool->scene_picked_object),
						scene_picked_object);
				}
			}
			else
			{
				scene_picked_object=(struct Scene_picked_object *)NULL;
			}
			if (rc_coordinate_field=
				Computed_field_begin_wrap_coordinate_field(coordinate_field))
			{
				node_number=get_next_FE_node_number(node_tool->node_manager,1);
				/* get new node coordinates from interaction_volume */
				Interaction_volume_get_placement_point(interaction_volume,
					node_coordinates);
				for (i=0;i<3;i++)
				{
					coordinates[i]=(FE_value)node_coordinates[i];
				}
				if (scene_picked_object&&
					Scene_picked_object_get_total_transformation_matrix(
						node_tool->scene_picked_object,&transformation_required,
						LU_transformation_matrix)&&transformation_required&&
					LU_decompose(4,LU_transformation_matrix,LU_indx,&d))
				{
					world_to_model_coordinates(coordinates,
						LU_transformation_matrix,LU_indx);
				}
				if (!((node=CREATE(FE_node)(node_number,node_tool->template_node))&&
					Computed_field_set_values_at_node(rc_coordinate_field,
						node,coordinates)&&
					ADD_OBJECT_TO_MANAGER(FE_node)(node,node_tool->node_manager)&&
					ADD_OBJECT_TO_GROUP(FE_node)(node,node_tool->node_group)))
				{
					display_message(ERROR_MESSAGE,
						"Node_tool_create_node_at_interaction_volume.  "
						"Could not create node");
					if (IS_MANAGED(FE_node)(node,node_tool->node_manager))
					{
						REMOVE_OBJECT_FROM_MANAGER(FE_node)(node,
							node_tool->node_manager);
					}
					else
					{
						DESTROY(FE_node)(&node);
					}
				}
				Computed_field_clear_cache(rc_coordinate_field);
				Computed_field_end_wrap(&rc_coordinate_field);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Node_tool_create_node_at_interaction_volume.  "
				"Could not create template node");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_create_node_at_interaction_volume.  "
			"Invalid argument(s)");
	}
	if (node_tool)
	{
		/* only need following if editing; in which case need all of them */
		if ((!node)||(!scene_picked_object)||(!gt_element_group)||
			(!gt_element_settings))
		{
			scene_picked_object=(struct Scene_picked_object *)NULL;
			gt_element_group=(struct GT_element_group *)NULL;
			gt_element_settings=(struct GT_element_settings *)NULL;
		}
		/* make sure node_tool point at following as found out above */
		REACCESS(Scene_picked_object)(&(node_tool->scene_picked_object),
			scene_picked_object);
		REACCESS(GT_element_group)(&(node_tool->gt_element_group),
			gt_element_group);
		REACCESS(GT_element_settings)(&(node_tool->gt_element_settings),
			gt_element_settings);
	}
	LEAVE;

	return (node);
} /* Node_tool_create_node_at_interaction_volume */

#if defined (MOTIF)
static void Node_tool_close_CB(Widget widget,void *node_tool_void,
	void *call_data)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Callback when "close" is selected from the window menu, or it is double
clicked. How this is made to occur is as follows. The dialog has its
XmNdeleteResponse == XmDO_NOTHING, and a window manager protocol callback for
WM_DELETE_WINDOW has been set up with XmAddWMProtocolCallback to call this
function in response to the close command. See CREATE for more details.
Function pops down dialog as a response,
==============================================================================*/
{
	struct Node_tool *node_tool;

	ENTER(Node_tool_close_CB);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (node_tool=(struct Node_tool *)node_tool_void)
	{
		XtPopdown(node_tool->window_shell);
	}
	else
	{
		display_message(ERROR_MESSAGE,"Node_tool_close_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Node_tool_close_CB */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void Node_tool_create_button_CB(Widget widget,
	void *node_tool_void,void *call_data)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Callback from toggle button controlling whether nodes are created in
response to interactive events.
==============================================================================*/
{
	struct Node_tool *node_tool;

	ENTER(Node_tool_create_button_CB);
	USE_PARAMETER(call_data);
	if (node_tool=(struct Node_tool *)node_tool_void)
	{
		Node_tool_set_create_enabled(node_tool,
			XmToggleButtonGadgetGetState(widget));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_create_button_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Node_tool_create_button_CB */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void Node_tool_define_button_CB(Widget widget,
	void *node_tool_void,void *call_data)
/*******************************************************************************
LAST MODIFIED : 11 September 2000

DESCRIPTION :
Callback from toggle button controlling whether fields are defined at nodes in
response to interactive events.
==============================================================================*/
{
	struct Node_tool *node_tool;

	ENTER(Node_tool_define_button_CB);
	USE_PARAMETER(call_data);
	if (node_tool=(struct Node_tool *)node_tool_void)
	{
		Node_tool_set_define_enabled(node_tool,
			XmToggleButtonGadgetGetState(widget));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_define_button_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Node_tool_define_button_CB */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void Node_tool_edit_button_CB(Widget widget,
	void *node_tool_void,void *call_data)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Callback from toggle button controlling whether nodes are edited in
response to interactive events.
==============================================================================*/
{
	struct Node_tool *node_tool;

	ENTER(Node_tool_edit_button_CB);
	USE_PARAMETER(call_data);
	if (node_tool=(struct Node_tool *)node_tool_void)
	{
		Node_tool_set_edit_enabled(node_tool,
			XmToggleButtonGadgetGetState(widget));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_edit_button_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Node_tool_edit_button_CB */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void Node_tool_motion_update_button_CB(Widget widget,
	void *node_tool_void,void *call_data)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Callback from toggle button controlling whether editing motions are updated
during the edit - if off then updates only once at the end.
==============================================================================*/
{
	struct Node_tool *node_tool;

	ENTER(Node_tool_motion_update_button_CB);
	USE_PARAMETER(call_data);
	if (node_tool=(struct Node_tool *)node_tool_void)
	{
		Node_tool_set_motion_update_enabled(node_tool,
			XmToggleButtonGadgetGetState(widget));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_motion_update_button_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Node_tool_motion_update_button_CB */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void Node_tool_select_button_CB(Widget widget,
	void *node_tool_void,void *call_data)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Callback from toggle button controlling whether nodes are selected in
response to interactive events.
==============================================================================*/
{
	struct Node_tool *node_tool;

	ENTER(Node_tool_select_button_CB);
	USE_PARAMETER(call_data);
	if (node_tool=(struct Node_tool *)node_tool_void)
	{
		Node_tool_set_select_enabled(node_tool,
			XmToggleButtonGadgetGetState(widget));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_select_button_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Node_tool_select_button_CB */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void Node_tool_streaming_create_button_CB(Widget widget,
	void *node_tool_void,void *call_data)
/*******************************************************************************
LAST MODIFIED : 14 May 2001

DESCRIPTION :
Callback from toggle button controlling whether nodes are created continuously,
ie. streaming in response to interactive events = user drags.
==============================================================================*/
{
	struct Node_tool *node_tool;

	ENTER(Node_tool_streaming_create_button_CB);
	USE_PARAMETER(call_data);
	if (node_tool=(struct Node_tool *)node_tool_void)
	{
		Node_tool_set_streaming_create_enabled(node_tool,
			XmToggleButtonGadgetGetState(widget));
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_streaming_create_button_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Node_tool_streaming_create_button_CB */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void Node_tool_url_field_button_CB(Widget widget,
	void *node_tool_void,void *call_data)
/*******************************************************************************
LAST MODIFIED : 4 July 2002

DESCRIPTION :
Callback from toggle button enabling a url_field to be selected.
==============================================================================*/
{
	struct Computed_field *url_field;
	struct Node_tool *node_tool;

	ENTER(Node_tool_url_field_button_CB);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (node_tool=(struct Node_tool *)node_tool_void)
	{
		if (Node_tool_get_url_field(node_tool))
		{
			Node_tool_set_url_field(node_tool, (struct Computed_field *)NULL);
		}
		else
		{
			/* get label field from widget */
			url_field = CHOOSE_OBJECT_GET_OBJECT(Computed_field)(
				node_tool->url_field_widget);
			if (url_field)
			{
				Node_tool_set_url_field(node_tool, url_field);
			}
			else
			{
				XtVaSetValues(node_tool->url_field_button, XmNset, False, NULL);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_url_field_button_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Node_tool_url_field_button_CB */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void Node_tool_update_url_field(Widget widget,
	void *node_tool_void, void *url_field_void)
/*******************************************************************************
LAST MODIFIED : 4 July 2002

DESCRIPTION :
Callback for change of url_field.
==============================================================================*/
{
	struct Node_tool *node_tool;

	ENTER(Node_tool_update_url_field);
	USE_PARAMETER(widget);
	if (node_tool = (struct Node_tool *)node_tool_void)
	{
		/* skip messages from chooser if it is grayed out */
		if (XtIsSensitive(node_tool->url_field_widget))
		{
			Node_tool_set_url_field(node_tool,
				(struct Computed_field *)url_field_void);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_update_url_field.  Invalid argument(s)");
	}
	LEAVE;
} /* Node_tool_update_url_field */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void Node_tool_update_node_group(Widget widget,
	void *node_tool_void,void *node_group_void)
/*******************************************************************************
LAST MODIFIED : 26 June 2000

DESCRIPTION :
Callback for change of node group to put the new nodes in.
==============================================================================*/
{
	struct GROUP(FE_node) *node_group;
	struct Node_tool *node_tool;

	ENTER(Node_tool_update_node_group);
	USE_PARAMETER(widget);
	if (node_tool=(struct Node_tool *)node_tool_void)
	{
		node_group=(struct GROUP(FE_node) *)node_group_void;
		Node_tool_set_node_group(node_tool,node_group);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_update_node_group.  Invalid argument(s)");
	}
	LEAVE;
} /* Node_tool_update_node_group */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void Node_tool_update_coordinate_field(Widget widget,
	void *node_tool_void,void *coordinate_field_void)
/*******************************************************************************
LAST MODIFIED : 12 September 2000

DESCRIPTION :
Callback for change of coordinate_field.
==============================================================================*/
{
	struct Node_tool *node_tool;

	ENTER(Node_tool_update_coordinate_field);
	USE_PARAMETER(widget);
	if (node_tool=(struct Node_tool *)node_tool_void)
	{
		Node_tool_set_coordinate_field(node_tool,
			(struct Computed_field *)coordinate_field_void);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_update_coordinate_field.  Invalid argument(s)");
	}
	LEAVE;
} /* Node_tool_update_coordinate_field */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void Node_tool_destroy_selected_CB(Widget widget,
	void *node_tool_void,void *call_data)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Attempts to destroy all the nodes currently in the global selection.
==============================================================================*/
{
	struct LIST(FE_node) *destroy_node_list;
	struct Node_tool *node_tool;

	ENTER(Node_tool_destroy_selected_CB);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (node_tool=(struct Node_tool *)node_tool_void)
	{
		if (destroy_node_list=CREATE(LIST(FE_node))())
		{
			COPY_LIST(FE_node)(destroy_node_list,
				FE_node_selection_get_node_list(node_tool->node_selection));
			destroy_listed_nodes(destroy_node_list,
				node_tool->node_manager,node_tool->node_group_manager,
				node_tool->element_manager,node_tool->node_selection);
			DESTROY(LIST(FE_node))(&destroy_node_list);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_destroy_selected_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Node_tool_destroy_selected_CB */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
static void Node_tool_undefine_selected_CB(Widget widget,
	void *node_tool_void,void *call_data)
/*******************************************************************************
LAST MODIFIED : 15 September 2000

DESCRIPTION :
Attempts to undefine all the nodes currently in the global selection.
==============================================================================*/
{
	struct FE_field *fe_field;
	struct LIST(FE_field) *fe_field_list;
	struct Node_tool *node_tool;

	ENTER(Node_tool_undefine_selected_CB);
	USE_PARAMETER(widget);
	USE_PARAMETER(call_data);
	if (node_tool=(struct Node_tool *)node_tool_void)
	{
		if (node_tool->coordinate_field && (fe_field_list=
			Computed_field_get_defining_FE_field_list(node_tool->coordinate_field,
				Computed_field_package_get_computed_field_manager(
					node_tool->computed_field_package))))
		{
			if ((1==NUMBER_IN_LIST(FE_field)(fe_field_list))&&
				(fe_field=FIRST_OBJECT_IN_LIST_THAT(FE_field)(
					(LIST_CONDITIONAL_FUNCTION(FE_field) *)NULL,(void *)NULL,
					fe_field_list)))
			{
				undefine_field_at_listed_nodes(
					FE_node_selection_get_node_list(node_tool->node_selection),
					fe_field,node_tool->node_manager,node_tool->element_manager);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Node_tool_undefine_selected_CB.  Invalid field");
			}
			DESTROY(LIST(FE_field))(&fe_field_list);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Node_tool_undefine_selected_CB.  No field to undefine");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_undefine_selected_CB.  Invalid argument(s)");
	}
	LEAVE;
} /* Node_tool_undefine_selected_CB */
#endif /* defined (MOTIF) */

static void Node_tool_interactive_event_handler(void *device_id,
	struct Interactive_event *event,void *node_tool_void)
/*******************************************************************************
LAST MODIFIED : 4 July 2002

DESCRIPTION :
Input handler for input from devices. <device_id> is a unique address enabling
the editor to handle input from more than one device at a time. The <event>
describes the type of event, button numbers and key modifiers, and the volume
of space affected by the interaction. Main events are button press, movement and
release.
==============================================================================*/
{
	char *url_string;
	double d;
	enum Glyph_scaling_mode glyph_scaling_mode;
	enum Interactive_event_type event_type;
	FE_value time;
	int clear_selection,input_modifier,return_code,shift_pressed;
	struct Computed_field *coordinate_field;
	struct FE_node *picked_node;
	struct FE_node_edit_information edit_info;
	struct GT_element_group *gt_element_group;
	struct GT_element_settings *gt_element_settings;
	struct GT_object *glyph;
	struct Interaction_volume *interaction_volume,*temp_interaction_volume;
	struct LIST(FE_node) *node_list;
	struct LIST(Scene_picked_object) *scene_picked_object_list;
	struct Node_tool *node_tool;
	struct Scene *scene;
	struct Scene_picked_object *scene_picked_object;

	ENTER(Node_tool_interactive_event_handler);
	if (device_id&&event&&(node_tool=
		(struct Node_tool *)node_tool_void))
	{
		interaction_volume=Interactive_event_get_interaction_volume(event);
		if (scene=Interactive_event_get_scene(event))
		{
			event_type=Interactive_event_get_type(event);
			input_modifier=Interactive_event_get_input_modifier(event);
			shift_pressed=(INTERACTIVE_EVENT_MODIFIER_SHIFT & input_modifier);
			switch (event_type)
			{
				case INTERACTIVE_EVENT_BUTTON_PRESS:
				{
					/* interaction only works with first mouse button */
					if (1==Interactive_event_get_button_number(event))
					{
						if (scene_picked_object_list=
							Scene_pick_objects(scene,interaction_volume))
						{
							picked_node=(struct FE_node *)NULL;
							if (node_tool->select_enabled)
							{
								picked_node=Scene_picked_object_list_get_nearest_node(
									scene_picked_object_list,node_tool->use_data,
									(struct GROUP(FE_node) *)NULL,&scene_picked_object,
									&gt_element_group,&gt_element_settings);
							}
							if (picked_node)
							{
								/* Open url_field of picked_node in browser */
								if (node_tool->url_field)
								{
									if (Computed_field_is_defined_at_node(node_tool->url_field,
										picked_node))
									{
										if (node_tool->time_keeper)
										{
											time = Time_keeper_get_time(node_tool->time_keeper);
										}
										else
										{
											time = 0;
										}
										if (url_string = Computed_field_evaluate_as_string_at_node(
											node_tool->url_field, /*component_number*/-1,
											picked_node, time))
										{
											do_help(url_string,
												/*help_examples_directory*/(char *)NULL,
												node_tool->execute_command, node_tool->user_interface);
											DEALLOCATE(url_string);
										}
									}
								}
								node_tool->picked_node_was_unselected=
									!FE_node_selection_is_node_selected(node_tool->node_selection,
										picked_node);
								REACCESS(Scene_picked_object)(
									&(node_tool->scene_picked_object),scene_picked_object);
								REACCESS(GT_element_group)(&(node_tool->gt_element_group),
									gt_element_group);
								REACCESS(GT_element_settings)(
									&(node_tool->gt_element_settings),gt_element_settings);
								if (node_tool->define_enabled)
								{
									if (!Computed_field_is_defined_at_node(
										node_tool->coordinate_field,picked_node))
									{
										Node_tool_define_field_at_node_from_picked_coordinates(
											node_tool,picked_node);
									}
								}
							}
							else
							{
								if (node_tool->create_enabled&&(picked_node=
									Node_tool_create_node_at_interaction_volume(node_tool,
										scene,interaction_volume)))
								{
									node_tool->picked_node_was_unselected=1;
								}
								else
								{
									node_tool->picked_node_was_unselected=0;
								}
							}
							REACCESS(FE_node)(&(node_tool->last_picked_node),picked_node);
							if (clear_selection = !shift_pressed)
#if defined (OLD_CODE)
								&&((!picked_node)||(node_tool->picked_node_was_unselected))))
#endif /*defined (OLD_CODE) */
							{
								FE_node_selection_begin_cache(node_tool->node_selection);
								FE_node_selection_clear(node_tool->node_selection);
							}
							if (picked_node)
							{
								FE_node_selection_select_node(node_tool->node_selection,
									picked_node);
							}
							if (clear_selection)
							{
								FE_node_selection_end_cache(node_tool->node_selection);
							}
							DESTROY(LIST(Scene_picked_object))(&(scene_picked_object_list));
						}
						REACCESS(Interaction_volume)(&(node_tool->last_interaction_volume),
							interaction_volume);
					}
					node_tool->motion_detected=0;
				} break;
				case INTERACTIVE_EVENT_MOTION_NOTIFY:
				case INTERACTIVE_EVENT_BUTTON_RELEASE:
				{
					if (node_tool->last_interaction_volume&&
						((INTERACTIVE_EVENT_MOTION_NOTIFY==event_type) ||
							(1==Interactive_event_get_button_number(event))))
					{
						if (INTERACTIVE_EVENT_MOTION_NOTIFY==event_type)
						{
							node_tool->motion_detected=1;
						}
						if (node_tool->last_picked_node)
						{
							if (node_tool->create_enabled &&
								node_tool->streaming_create_enabled &&
								(INTERACTIVE_EVENT_MOTION_NOTIFY == event_type))
							{
								if (picked_node = Node_tool_create_node_at_interaction_volume(
									node_tool, scene, interaction_volume))
								{
									FE_node_selection_select_node(node_tool->node_selection,
										picked_node);
									REACCESS(FE_node)(&(node_tool->last_picked_node),
										picked_node);
								}
							}
							else if ((node_tool->edit_enabled)&&node_tool->motion_detected &&
								(((INTERACTIVE_EVENT_MOTION_NOTIFY==event_type)&&
									node_tool->motion_update_enabled)||
									((INTERACTIVE_EVENT_BUTTON_RELEASE==event_type)&&
										(!node_tool->motion_update_enabled))))
							{
								return_code=1;
								/* establish edit_info */
								edit_info.last_picked_node = (struct FE_node *)NULL;
								edit_info.delta1=0.0;
								edit_info.delta2=0.0;
								edit_info.delta3=0.0;
								edit_info.initial_interaction_volume=
									node_tool->last_interaction_volume;
								edit_info.final_interaction_volume=interaction_volume;
								edit_info.node_manager=node_tool->node_manager;
								edit_info.time=Time_keeper_get_time(
									node_tool->time_keeper);
								if (node_tool->use_data)
								{
									edit_info.node_group=GT_element_group_get_data_group(
										node_tool->gt_element_group);
								}
								else
								{
									edit_info.node_group=GT_element_group_get_node_group(
										node_tool->gt_element_group);
								}
								/* get coordinate field to edit */
								if (node_tool->define_enabled)
								{
									coordinate_field=node_tool->coordinate_field;
								}
								else if (!(coordinate_field=
									GT_element_settings_get_coordinate_field(
										node_tool->gt_element_settings)))
								{
									coordinate_field=
										GT_element_group_get_default_coordinate_field(
											node_tool->gt_element_group);
								}
								edit_info.coordinate_field=coordinate_field;
								/* get coordinate_field in RC coordinates */
								edit_info.rc_coordinate_field=
									Computed_field_begin_wrap_coordinate_field(coordinate_field);
								edit_info.orientation_scale_field=(struct Computed_field *)NULL;
								edit_info.wrapper_orientation_scale_field=
									(struct Computed_field *)NULL;
								if (GT_element_settings_get_glyph_parameters(
									node_tool->gt_element_settings, &glyph, &glyph_scaling_mode,
									edit_info.glyph_centre,edit_info.glyph_size,
									&(edit_info.orientation_scale_field),
									edit_info.glyph_scale_factors,
									&(edit_info.variable_scale_field)))
								{
									if (edit_info.orientation_scale_field)
									{
										edit_info.wrapper_orientation_scale_field=
											Computed_field_begin_wrap_orientation_scale_field(
												edit_info.orientation_scale_field,
												edit_info.rc_coordinate_field);
									}
								}
								else
								{
									return_code=0;
								}
								/* work out scene_object transformation information */
								if (!(Scene_picked_object_get_total_transformation_matrix(
									node_tool->scene_picked_object,
									&(edit_info.transformation_required),
									edit_info.transformation_matrix)&&
									copy_matrix(4,4,edit_info.transformation_matrix,
										edit_info.LU_transformation_matrix)&&
									((!edit_info.transformation_required)||
										LU_decompose(4,edit_info.LU_transformation_matrix,
											edit_info.LU_indx,&d))))
								{
									return_code=0;
								}
								if (return_code)
								{
									if (node_list=FE_node_selection_get_node_list(
										node_tool->node_selection))
									{
										/* cache manager so only one change message produced */
										MANAGER_BEGIN_CACHE(FE_node)(node_tool->node_manager);
										/* edit vectors if non-constant orientation_scale field */
										if (((NODE_TOOL_EDIT_AUTOMATIC == node_tool->edit_mode) ||
											(NODE_TOOL_EDIT_VECTOR == node_tool->edit_mode))&&
											edit_info.wrapper_orientation_scale_field&&
											(!Computed_field_is_constant(
												edit_info.orientation_scale_field)))
										{
											/* edit vector */
											if (FE_node_calculate_delta_vector(
												node_tool->last_picked_node,(void *)&edit_info))
											{
												FOR_EACH_OBJECT_IN_LIST(FE_node)(FE_node_edit_vector,
													(void *)&edit_info,node_list);
											}
										}
										else
										{
											if (NODE_TOOL_EDIT_VECTOR != node_tool->edit_mode)
											{
												/* edit position */
												if (FE_node_calculate_delta_position(
													node_tool->last_picked_node,(void *)&edit_info))
												{
													FOR_EACH_OBJECT_IN_LIST(FE_node)(
														FE_node_edit_position,(void *)&edit_info,node_list);
												}
											}
											else
											{
												display_message(ERROR_MESSAGE,"Cannot edit vector: "
													"invalid orientation_scale field");
												return_code=0;
											}
										}
										MANAGER_END_CACHE(FE_node)(node_tool->node_manager);
									}
									else
									{
										return_code=0;
									}
								}
								if (edit_info.orientation_scale_field)
								{
									Computed_field_end_wrap(
										&(edit_info.wrapper_orientation_scale_field));
								}
								Computed_field_end_wrap(&(edit_info.rc_coordinate_field));
							}
							else
							{
								/* unselect last_picked_node if not just added */
								if ((INTERACTIVE_EVENT_BUTTON_RELEASE==event_type)&&
									shift_pressed&&(!(node_tool->picked_node_was_unselected))&&
									(!(node_tool->edit_enabled && node_tool->motion_detected)))
								{
									FE_node_selection_unselect_node(node_tool->node_selection,
										node_tool->last_picked_node);
								}
							}
						}
						else if (node_tool->motion_detected)
						{
							/* rubber band select - make bounding box out of initial and
								 current interaction_volumes */
							if (temp_interaction_volume=
								create_Interaction_volume_bounding_box(
									node_tool->last_interaction_volume,interaction_volume))
							{
								if (INTERACTIVE_EVENT_MOTION_NOTIFY==event_type)
								{
									if (!node_tool->rubber_band)
									{
										/* create rubber_band object and put in scene */
										node_tool->rubber_band=CREATE(GT_object)(
											"node_tool_rubber_band",g_POLYLINE,
											node_tool->rubber_band_material);
										ACCESS(GT_object)(node_tool->rubber_band);
										Scene_add_graphics_object(scene,node_tool->rubber_band,
											/*position*/0,"node_tool_rubber_band",
											/*fast_changing*/1);
									}
									Interaction_volume_make_polyline_extents(
										temp_interaction_volume,node_tool->rubber_band);
								}
								else
								{
									Scene_remove_graphics_object(scene,node_tool->rubber_band);
									DEACCESS(GT_object)(&(node_tool->rubber_band));
								}
								if (INTERACTIVE_EVENT_BUTTON_RELEASE==event_type)
								{
									if (scene_picked_object_list=
										Scene_pick_objects(scene,temp_interaction_volume))
									{
										if (node_list=Scene_picked_object_list_get_picked_nodes(
											scene_picked_object_list,node_tool->use_data))
										{
											FE_node_selection_begin_cache(node_tool->node_selection);
											FOR_EACH_OBJECT_IN_LIST(FE_node)(
												FE_node_select_in_FE_node_selection,
												(void *)(node_tool->node_selection),node_list);
											FE_node_selection_end_cache(node_tool->node_selection);
											DESTROY(LIST(FE_node))(&node_list);
										}
										DESTROY(LIST(Scene_picked_object))(
											&(scene_picked_object_list));
									}
								}
								DESTROY(Interaction_volume)(&temp_interaction_volume);
							}
						}
						if (INTERACTIVE_EVENT_BUTTON_RELEASE==event_type)
						{
							/* deaccess following as only kept for one input movement */
							REACCESS(FE_node)(&(node_tool->last_picked_node),
								(struct FE_node *)NULL);
							REACCESS(Interaction_volume)(
								&(node_tool->last_interaction_volume),
								(struct Interaction_volume *)NULL);
							REACCESS(Scene_picked_object)(&(node_tool->scene_picked_object),
								(struct Scene_picked_object *)NULL);
							REACCESS(GT_element_group)(&(node_tool->gt_element_group),
								(struct GT_element_group *)NULL);
							REACCESS(GT_element_settings)(&(node_tool->gt_element_settings),
								(struct GT_element_settings *)NULL);
						}
						else if (node_tool->last_picked_node&&
							node_tool->motion_update_enabled)
						{
							REACCESS(Interaction_volume)(
								&(node_tool->last_interaction_volume),interaction_volume);
						}
					}
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,
						"Node_tool_interactive_event_handler.  "
						"Unknown event type");
				} break;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_interactive_event_handler.  Invalid argument(s)");
	}
	LEAVE;
} /* Node_tool_interactive_event_handler */

static int Node_tool_bring_up_interactive_tool_dialog(void *node_tool_void)
/*******************************************************************************
LAST MODIFIED : 20 June 2001

DESCRIPTION :
Brings up a dialog for editing settings of the Node_tool - in a standard format
for passing to an Interactive_toolbar.
==============================================================================*/
{
	int return_code;

	ENTER(Node_tool_bring_up_interactive_tool_dialog);
	return_code = Node_tool_pop_up_dialog((struct Node_tool *)node_tool_void);
	LEAVE;

	return (return_code);
} /* Node_tool_bring_up_interactive_tool_dialog */

static struct Cmgui_image *Node_tool_get_icon(struct Colour *foreground, 
	struct Colour *background, void *node_tool_void)
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Fetches the appropriate icon for the interactive tool.
==============================================================================*/
{
#if defined (MOTIF)
	char *icon_name;
	Display *display;
	Pixel background_pixel, foreground_pixel;
	Pixmap pixmap;
#endif /* defined (MOTIF) */
	struct Cmgui_image *image;
	struct Node_tool *node_tool;

	ENTER(node_tool_get_icon);
	if ((node_tool=(struct Node_tool *)node_tool_void))
	{
#if defined (MOTIF)
		if (MrmOpenHierarchy_base64_string(node_tool_uidh,
			&node_tool_hierarchy,&node_tool_hierarchy_open))
		{
			if (node_tool->use_data)
			{
				icon_name="data_tool_icon";
			}
			else
			{
				icon_name="node_tool_icon";
			}
			display = node_tool->display;
			convert_Colour_to_Pixel(display, foreground, &foreground_pixel);
			convert_Colour_to_Pixel(display, background, &background_pixel);
			if (MrmSUCCESS == MrmFetchIconLiteral(node_tool_hierarchy,
				icon_name,DefaultScreenOfDisplay(display),display,
				foreground_pixel, background_pixel, &pixmap))
			{ 
				image = create_Cmgui_image_from_Pixmap(display, pixmap);
			}
			else
			{
				display_message(WARNING_MESSAGE, "Node_tool_get_icon.  "
					"Could not fetch widget");
				image = (struct Cmgui_image *)NULL;
			}			
		}
		else
		{
			display_message(WARNING_MESSAGE, "Node_tool_get_icon.  "
				"Could not open heirarchy");
			image = (struct Cmgui_image *)NULL;
		}
#else /* defined (MOTIF) */
		USE_PARAMETER(foreground);
		USE_PARAMETER(background);
		display_message(WARNING_MESSAGE, "Node_tool_get_icon.  "
			"Not implemented for this user interface.");
#endif /* defined (MOTIF) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_get_icon.  Invalid argument(s)");
		image = (struct Cmgui_image *)NULL;
	}
	LEAVE;

	return (image);
} /* Node_tool_get_icon */

static void Node_tool_node_group_change(
	struct MANAGER_MESSAGE(GROUP(FE_node)) *message,void *node_tool_void)
/*******************************************************************************
LAST MODIFIED : 30 September 2002

DESCRIPTION :
Node group manager change callback.  Makes sure we don't hold onto a 
reference to an invalid node group.
==============================================================================*/
{
	int return_code;
	struct Node_tool *node_tool;

	ENTER(Node_tool_node_group_change);
	if (message && (node_tool = (struct Node_tool *)node_tool_void))
	{
		switch (message->change)
		{
			case MANAGER_CHANGE_REMOVE(GROUP(FE_node)):
			{
				/* If the node group we are creating into disappears then
					remove our reference to it and leave create mode */
				if (node_tool->node_group && 
					IS_OBJECT_IN_LIST(GROUP(FE_node))(node_tool->node_group,
					message->changed_object_list))
				{
					node_tool->node_group = (struct GROUP(FE_node) *)NULL;
					Node_tool_set_create_enabled(node_tool, /*false*/0);
				}
				return_code = 1;
			} break;
			case MANAGER_CHANGE_OBJECT(GROUP(FE_node)):
			case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(GROUP(FE_node)):
			case MANAGER_CHANGE_IDENTIFIER(GROUP(FE_node)):
			case MANAGER_CHANGE_ADD(GROUP(FE_node)):
			{
				return_code = 1;
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_node_group_change.  Invalid argument(s)");
	}
	LEAVE;
} /* Node_tool_node_group_change */

/*
Global functions
----------------
*/

struct Node_tool *CREATE(Node_tool)(
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
	struct MANAGER(FE_node) *node_manager,int use_data,
	struct MANAGER(GROUP(FE_node)) *node_group_manager,
	struct MANAGER(FE_element) *element_manager,
	struct FE_node_selection *node_selection,
	struct Computed_field_package *computed_field_package,
	struct Graphical_material *rubber_band_material,
	struct User_interface *user_interface,
	struct Time_keeper *time_keeper,
	struct Execute_command *execute_command)
/*******************************************************************************
LAST MODIFIED : 4 July 2002

DESCRIPTION :
Creates a Node_tool for editing nodes/data in the <node_manager>,
using the <node_selection>.
The <use_data> flag indicates that <node_manager> and <node_selection>
refer to data, not nodes, needed since different GT_element_settings types are
used to represent them. <element_manager> should be NULL if <use_data> is true.
==============================================================================*/
{
	char *tool_display_name,*tool_name;
#if defined (MOTIF)
	Atom WM_DELETE_WINDOW;
	int init_widgets;
	MrmType node_tool_dialog_class;
	static MrmRegisterArg callback_list[]=
	{
		{"node_tool_id_select_btn",(XtPointer)
			DIALOG_IDENTIFY(node_tool,select_button)},
		{"node_tool_id_edit_btn",(XtPointer)
			DIALOG_IDENTIFY(node_tool,edit_button)},
		{"node_tool_id_motion_update_btn",(XtPointer)
			DIALOG_IDENTIFY(node_tool,motion_update_button)},
		{"node_tool_id_define_btn",(XtPointer)
			DIALOG_IDENTIFY(node_tool,define_button)},
		{"node_tool_id_create_btn",(XtPointer)
			DIALOG_IDENTIFY(node_tool,create_button)},
		{"node_tool_id_streaming_btn",(XtPointer)
			DIALOG_IDENTIFY(node_tool,streaming_create_button)},
		{"node_tool_id_node_group_form",(XtPointer)
			DIALOG_IDENTIFY(node_tool,node_group_form)},
		{"node_tool_id_coord_field_form",(XtPointer)
			DIALOG_IDENTIFY(node_tool,coordinate_field_form)},
		{"node_tool_id_url_field_btn",(XtPointer)
			DIALOG_IDENTIFY(node_tool,url_field_button)},
		{"node_tool_id_url_field_form",(XtPointer)
			DIALOG_IDENTIFY(node_tool,url_field_form)},
		{"node_tool_select_btn_CB",
		 (XtPointer)Node_tool_select_button_CB},
		{"node_tool_edit_btn_CB",
		 (XtPointer)Node_tool_edit_button_CB},
		{"node_tool_motion_update_btn_CB",
		 (XtPointer)Node_tool_motion_update_button_CB},
		{"node_tool_define_btn_CB",
		 (XtPointer)Node_tool_define_button_CB},
		{"node_tool_create_btn_CB",
		 (XtPointer)Node_tool_create_button_CB},
		{"node_tool_streaming_btn_CB",
		 (XtPointer)Node_tool_streaming_create_button_CB},
		{"node_tool_url_field_btn_CB",
		 (XtPointer)Node_tool_url_field_button_CB},
		{"node_tool_destroy_selected_CB",
		 (XtPointer)Node_tool_destroy_selected_CB},
		{"node_tool_undefine_selected_CB",
		 (XtPointer)Node_tool_undefine_selected_CB}
	};
	static MrmRegisterArg identifier_list[]=
	{
		{"node_tool_structure",(XtPointer)NULL}
	};
	struct Callback_data callback;
#endif /* defined (MOTIF) */
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Node_tool *node_tool;

	ENTER(CREATE(Node_tool));
	node_tool=(struct Node_tool *)NULL;
	if (interactive_tool_manager&&node_manager&&
		node_group_manager&&(element_manager||use_data)&&node_selection&&
		computed_field_package&&(computed_field_manager=
			Computed_field_package_get_computed_field_manager(computed_field_package))
		&&rubber_band_material&&user_interface&&execute_command)
	{
		if (ALLOCATE(node_tool,struct Node_tool,1))
		{
			node_tool->execute_command=execute_command;
			node_tool->interactive_tool_manager=interactive_tool_manager;
			node_tool->node_manager=node_manager;
			node_tool->use_data=use_data;
			node_tool->node_group_manager=node_group_manager;
			node_tool->node_group_manager_callback_id=
				MANAGER_REGISTER(GROUP(FE_node))(Node_tool_node_group_change,
				(void *)node_tool, node_tool->node_group_manager);
			node_tool->element_manager=element_manager;
			node_tool->node_selection=node_selection;
			node_tool->computed_field_package=computed_field_package;
			node_tool->rubber_band_material=
				ACCESS(Graphical_material)(rubber_band_material);
			node_tool->user_interface=user_interface;
			node_tool->time_keeper = (struct Time_keeper *)NULL;
			if (time_keeper)
			{
				node_tool->time_keeper = ACCESS(Time_keeper)(time_keeper);
			}
			/* user-settable flags */
			node_tool->select_enabled=1;
			node_tool->edit_enabled=0;
			node_tool->motion_update_enabled=1;
			node_tool->define_enabled=0;
			node_tool->create_enabled=0;
			node_tool->streaming_create_enabled=0;
			node_tool->edit_mode=NODE_TOOL_EDIT_AUTOMATIC;
			node_tool->node_group=FIRST_OBJECT_IN_MANAGER_THAT(GROUP(FE_node))(
				(MANAGER_CONDITIONAL_FUNCTION(GROUP(FE_node)) *)NULL,
				(void *)NULL,node_group_manager);
			node_tool->coordinate_field =
				FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
					Computed_field_has_up_to_3_numerical_components, (void *)NULL,
					computed_field_manager);
			node_tool->url_field = (struct Computed_field *)NULL;
			node_tool->template_node=(struct FE_node *)NULL;
			/* interactive_tool */
			if (use_data)
			{
				tool_name="data_tool";
				tool_display_name="Data tool";
			}
			else
			{
				tool_name="node_tool";
				tool_display_name="Node tool";
			}
			node_tool->interactive_tool=CREATE(Interactive_tool)(
				tool_name,tool_display_name,
				Interactive_tool_node_type_string,
				Node_tool_interactive_event_handler,
				Node_tool_get_icon,
				Node_tool_bring_up_interactive_tool_dialog,
				(Interactive_tool_destroy_tool_data_function *)NULL,
				(void *)node_tool);
			ADD_OBJECT_TO_MANAGER(Interactive_tool)(
				node_tool->interactive_tool,
				node_tool->interactive_tool_manager);
			node_tool->scene_picked_object=(struct Scene_picked_object *)NULL;
			node_tool->last_picked_node=(struct FE_node *)NULL;
			node_tool->gt_element_group=(struct GT_element_group *)NULL;
			node_tool->gt_element_settings=(struct GT_element_settings *)NULL;
			node_tool->last_interaction_volume=(struct Interaction_volume *)NULL;
			node_tool->rubber_band=(struct GT_object *)NULL;
#if defined (MOTIF)
			/* initialise widgets */
			node_tool->coordinate_field_form=(Widget)NULL;
			node_tool->coordinate_field_widget=(Widget)NULL;
			node_tool->create_button=(Widget)NULL;
			node_tool->define_button=(Widget)NULL;
			node_tool->display = User_interface_get_display(user_interface);
			node_tool->edit_button=(Widget)NULL;
			node_tool->motion_update_button=(Widget)NULL;
			node_tool->node_group_form=(Widget)NULL;
			node_tool->node_group_widget=(Widget)NULL;
			node_tool->select_button=(Widget)NULL;
			node_tool->streaming_create_button=(Widget)NULL;
			node_tool->url_field_button=(Widget)NULL;
			node_tool->url_field_form=(Widget)NULL;
			node_tool->url_field_widget=(Widget)NULL;
			node_tool->widget=(Widget)NULL;
			node_tool->window_shell=(Widget)NULL;

			/* make the dialog shell */
			if (MrmOpenHierarchy_base64_string(node_tool_uidh,
				&node_tool_hierarchy,&node_tool_hierarchy_open))
			{
				if (node_tool->window_shell=
					XtVaCreatePopupShell(tool_display_name,
						topLevelShellWidgetClass,
						User_interface_get_application_shell(user_interface),
						XmNdeleteResponse,XmDO_NOTHING,
						XmNmwmDecorations,MWM_DECOR_ALL,
						XmNmwmFunctions,MWM_FUNC_ALL,
						/*XmNtransient,FALSE,*/
						XmNallowShellResize,False,
						XmNtitle,tool_display_name,
						NULL))
				{
					/* Set up window manager callback for close window message */
					WM_DELETE_WINDOW=XmInternAtom(XtDisplay(node_tool->window_shell),
						"WM_DELETE_WINDOW",False);
					XmAddWMProtocolCallback(node_tool->window_shell,
						WM_DELETE_WINDOW,Node_tool_close_CB,node_tool);
					/* Register the shell with the busy signal list */
					create_Shell_list_item(&(node_tool->window_shell),user_interface);
					/* register the callbacks */
					if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
						node_tool_hierarchy,callback_list,XtNumber(callback_list)))
					{
						/* assign and register the identifiers */
						identifier_list[0].value=(XtPointer)node_tool;
						if (MrmSUCCESS==MrmRegisterNamesInHierarchy(
							node_tool_hierarchy,identifier_list,XtNumber(identifier_list)))
						{
							/* fetch node tool widgets */
							if (MrmSUCCESS==MrmFetchWidget(node_tool_hierarchy,
								"node_tool",node_tool->window_shell,
								&(node_tool->widget),&node_tool_dialog_class))
							{
								init_widgets=1;
								if (node_tool->coordinate_field_widget=
									CREATE_CHOOSE_OBJECT_WIDGET(Computed_field)(
										node_tool->coordinate_field_form,
										node_tool->coordinate_field,computed_field_manager,
										Computed_field_has_up_to_3_numerical_components,
										(void *)NULL, user_interface))
								{
									callback.data=(void *)node_tool;
									callback.procedure=Node_tool_update_coordinate_field;
									CHOOSE_OBJECT_SET_CALLBACK(Computed_field)(
										node_tool->coordinate_field_widget,&callback);
								}
								else
								{
									init_widgets=0;
								}
								if (node_tool->url_field_widget =
									CREATE_CHOOSE_OBJECT_WIDGET(Computed_field)(
										node_tool->url_field_form,
										node_tool->url_field, computed_field_manager,
										Computed_field_has_string_value_type,
										(void *)NULL, user_interface))
								{
									callback.data = (void *)node_tool;
									callback.procedure = Node_tool_update_url_field;
									CHOOSE_OBJECT_SET_CALLBACK(Computed_field)(
										node_tool->url_field_widget, &callback);
								}
								else
								{
									init_widgets=0;
								}
								if (node_tool->node_group_widget=
									CREATE_CHOOSE_OBJECT_WIDGET(GROUP(FE_node))(
										node_tool->node_group_form,
										node_tool->node_group,node_group_manager,
										(MANAGER_CONDITIONAL_FUNCTION(GROUP(FE_node)) *)NULL,
										(void *)NULL, user_interface))
								{
									callback.data=(void *)node_tool;
									callback.procedure=Node_tool_update_node_group;
									CHOOSE_OBJECT_SET_CALLBACK(GROUP(FE_node))(
										node_tool->node_group_widget,&callback);
								}
								else
								{
									init_widgets=0;
								}
								if (init_widgets)
								{
									XmToggleButtonGadgetSetState(node_tool->create_button,
										/*state*/node_tool->create_enabled,/*notify*/False);
									XmToggleButtonGadgetSetState(node_tool->define_button,
										/*state*/node_tool->define_enabled,/*notify*/False);
									XmToggleButtonGadgetSetState(node_tool->edit_button,
										/*state*/node_tool->edit_enabled,/*notify*/False);
									XmToggleButtonGadgetSetState(node_tool->motion_update_button,
										/*state*/node_tool->motion_update_enabled,/*notify*/False);
									XmToggleButtonGadgetSetState(node_tool->select_button,
										/*state*/node_tool->select_enabled,/*notify*/False);
									XmToggleButtonGadgetSetState(
										node_tool->streaming_create_button,
										/*state*/node_tool->streaming_create_enabled,
										/*notify*/False);
									XmToggleButtonGadgetSetState(node_tool->url_field_button,
										/*state*/False, /*notify*/False);
									XtSetSensitive(node_tool->url_field_widget, /*state*/False);
									XtManageChild(node_tool->widget);
									XtRealizeWidget(node_tool->window_shell);
								}
								else
								{
									display_message(ERROR_MESSAGE,
										"CREATE(Node_tool).  Could not init widgets");
									DESTROY(Node_tool)(&node_tool);
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
									"CREATE(Node_tool).  Could not fetch node_tool");
								DESTROY(Node_tool)(&node_tool);
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"CREATE(Node_tool).  Could not register identifiers");
							DESTROY(Node_tool)(&node_tool);
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
							"CREATE(Node_tool).  Could not register callbacks");
						DESTROY(Node_tool)(&node_tool);
					}
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"CREATE(Node_tool).  Could not create Shell");
					DESTROY(Node_tool)(&node_tool);
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"CREATE(Node_tool).  Could not open hierarchy");
			}
#endif /* defined (MOTIF) */
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Node_tool).  Not enough memory");
			DEALLOCATE(node_tool);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Node_tool).  Invalid argument(s)");
	}
	LEAVE;

	return (node_tool);
} /* CREATE(Node_tool) */

int DESTROY(Node_tool)(struct Node_tool **node_tool_address)
/*******************************************************************************
LAST MODIFIED : 19 July 2000

DESCRIPTION :
Frees and deaccesses objects in the <node_tool> and deallocates the
structure itself.
==============================================================================*/
{
	struct Node_tool *node_tool;
	int return_code;

	ENTER(DESTROY(Node_tool));
	if (node_tool_address&&
		(node_tool= *node_tool_address))
	{
		REMOVE_OBJECT_FROM_MANAGER(Interactive_tool)(
			node_tool->interactive_tool,
			node_tool->interactive_tool_manager);
		if (node_tool->last_picked_node)
		{
			DEACCESS(FE_node)(&(node_tool->last_picked_node));
		}
		if (node_tool->scene_picked_object)
		{
			DEACCESS(Scene_picked_object)(
				&(node_tool->scene_picked_object));
			DEACCESS(GT_element_group)(&(node_tool->gt_element_group));
			DEACCESS(GT_element_settings)(
				&(node_tool->gt_element_settings));
		}
		REACCESS(Interaction_volume)(
			&(node_tool->last_interaction_volume),
			(struct Interaction_volume *)NULL);
		REACCESS(GT_object)(&(node_tool->rubber_band),(struct GT_object *)NULL);
		DEACCESS(Graphical_material)(&(node_tool->rubber_band_material));
		REACCESS(FE_node)(&(node_tool->template_node),(struct FE_node *)NULL);
		if (node_tool->node_group_manager_callback_id && node_tool->node_group_manager)
		{
			MANAGER_DEREGISTER(GROUP(FE_node))(
				node_tool->node_group_manager_callback_id,
				node_tool->node_group_manager);
			node_tool->node_group_manager_callback_id=(void *)NULL;
		}
		if (node_tool->time_keeper)
		{
			DEACCESS(Time_keeper)(&(node_tool->time_keeper));
		}
#if defined (MOTIF)
		if (node_tool->window_shell)
		{
			destroy_Shell_list_item_from_shell(&(node_tool->window_shell),
				node_tool->user_interface);
			XtDestroyWidget(node_tool->window_shell);
		}
#endif /* defined (MOTIF) */
		DEALLOCATE(*node_tool_address);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(Node_tool).  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Node_tool) */

int Node_tool_pop_up_dialog(struct Node_tool *node_tool)
/*******************************************************************************
LAST MODIFIED : 20 June 2001

DESCRIPTION :
Pops up a dialog for editing settings of the Node_tool.
==============================================================================*/
{
	int return_code;

	ENTER(Node_tool_pop_up_dialog);
	if (node_tool)
	{
#if defined (MOTIF)
		XtPopup(node_tool->window_shell, XtGrabNone);
		/* make sure in addition that it is not shown as an icon */
		XtVaSetValues(node_tool->window_shell, XmNiconic, False, NULL);
#else /* defined (MOTIF) */
		display_message(ERROR_MESSAGE, "Node_tool_pop_up_dialog.  "
			"No dialog implemented for this User Interface");
#endif /* defined (MOTIF) */
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_pop_up_dialog.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Node_tool_pop_up_dialog */

int Node_tool_pop_down_dialog(struct Node_tool *node_tool)
/*******************************************************************************
LAST MODIFIED : 20 June 2001

DESCRIPTION :
Hides the dialog for editing settings of the Node_tool.
==============================================================================*/
{
	int return_code;

	ENTER(Node_tool_pop_down_dialog);
	if (node_tool)
	{
#if defined (MOTIF)
		XtPopdown(node_tool->window_shell);
#else /* defined (MOTIF) */
		display_message(ERROR_MESSAGE, "Node_tool_pop_down_dialog.  "
			"No dialog implemented for this User Interface");
#endif /* defined (MOTIF) */
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_pop_down_dialog.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Node_tool_pop_down_dialog */

struct Computed_field *Node_tool_get_coordinate_field(
	struct Node_tool *node_tool)
/*******************************************************************************
LAST MODIFIED : 12 September 2000

DESCRIPTION :
Returns the coordinate_field used by the <node_tool> when create/define are on.
==============================================================================*/
{
	struct Computed_field *coordinate_field;

	ENTER(Node_tool_get_coordinate_field);
	if (node_tool)
	{
		coordinate_field=node_tool->coordinate_field;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_get_coordinate_field.  Invalid argument(s)");
		coordinate_field=(struct Computed_field *)NULL;
	}
	LEAVE;

	return (coordinate_field);
} /* Node_tool_get_coordinate_field */

int Node_tool_set_coordinate_field(struct Node_tool *node_tool,
	struct Computed_field *coordinate_field)
/*******************************************************************************
LAST MODIFIED : 12 September 2000

DESCRIPTION :
Sets the coordinate_field to be defined by the <node_tool> when create/define
are on.
==============================================================================*/
{
	int return_code;

	ENTER(Node_tool_set_coordinate_field);
	if (node_tool)
	{
		return_code=1;
		if (coordinate_field != node_tool->coordinate_field)
		{
			node_tool->coordinate_field=coordinate_field;
			/* lose the current template node, if any */
			REACCESS(FE_node)(&(node_tool->template_node),(struct FE_node *)NULL);
#if defined (MOTIF)
			/* make sure the current field is shown on the widget */
			CHOOSE_OBJECT_SET_OBJECT(Computed_field)(
				node_tool->coordinate_field_widget,node_tool->coordinate_field);
#endif /* defined (MOTIF) */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_set_coordinate_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_tool_set_coordinate_field */

int Node_tool_get_create_enabled(struct Node_tool *node_tool)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Returns flag controlling whether nodes can be created when none are selected
on a mouse button press.
==============================================================================*/
{
	int create_enabled;

	ENTER(Node_tool_get_create_enabled);
	if (node_tool)
	{
		create_enabled=node_tool->create_enabled;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_get_create_enabled.  Invalid argument(s)");
		create_enabled=0;
	}
	LEAVE;

	return (create_enabled);
} /* Node_tool_get_create_enabled */

int Node_tool_set_create_enabled(struct Node_tool *node_tool,
	int create_enabled)
/*******************************************************************************
LAST MODIFIED : 12 September 2000

DESCRIPTION :
Sets flag controlling whether nodes can be created when none are selected
on a mouse button press. Also ensures define is enabled if create is.
==============================================================================*/
{
	int return_code;
#if defined (MOTIF)
	int button_state;
#endif /* defined (MOTIF) */

	ENTER(Node_tool_set_create_enabled);
	if (node_tool)
	{
		return_code=1;
		if (create_enabled)
		{
			if (node_tool->node_group && node_tool->coordinate_field)
			{
				/* make sure value of flag is exactly 1 */
				create_enabled=1;
				/* must also enable define */
				Node_tool_set_define_enabled(node_tool,1);
			}
			else
			{
				display_message(WARNING_MESSAGE,
					"Node_tool must have a group and coordinate_field to create nodes");
				create_enabled=0;
				return_code=0;
			}
		}
		if (create_enabled != node_tool->create_enabled)
		{
			node_tool->create_enabled=create_enabled;
			/* make sure button shows current state */
#if defined (MOTIF)
			if (XmToggleButtonGadgetGetState(node_tool->create_button))
			{
				button_state=1;
			}
			else
			{
				button_state=0;
			}
			if (button_state != node_tool->create_enabled)
			{
				XmToggleButtonGadgetSetState(node_tool->create_button,
					/*state*/node_tool->create_enabled,/*notify*/False);
			}
#endif /* defined (MOTIF) */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_set_create_enabled.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_tool_set_create_enabled */

int Node_tool_get_define_enabled(struct Node_tool *node_tool)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Returns flag controlling whether nodes can be defined when none are selected
on a mouse button press.
==============================================================================*/
{
	int define_enabled;

	ENTER(Node_tool_get_define_enabled);
	if (node_tool)
	{
		define_enabled=node_tool->define_enabled;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_get_define_enabled.  Invalid argument(s)");
		define_enabled=0;
	}
	LEAVE;

	return (define_enabled);
} /* Node_tool_get_define_enabled */

int Node_tool_set_define_enabled(struct Node_tool *node_tool,
	int define_enabled)
/*******************************************************************************
LAST MODIFIED : 12 September 2000

DESCRIPTION :
Sets flag controlling whether the coordinate_field can be defined on any new
or individually selected existing nodes.
==============================================================================*/
{
	int return_code;
#if defined (MOTIF)
	int button_state;
#endif /* defined (MOTIF) */

	ENTER(Node_tool_set_define_enabled);
	if (node_tool)
	{
		return_code=1;
		if (define_enabled)
		{
			if (node_tool->coordinate_field)
			{
				/* make sure value of flag is exactly 1 */
				define_enabled=1;
			}
			else
			{
				display_message(WARNING_MESSAGE,
					"Cannot enable define in Node_tool without a field to define");
				define_enabled=0;
				return_code=0;
			}
		}
		else
		{
			/* make sure create is off */
			Node_tool_set_create_enabled(node_tool,0);
		}
		if (define_enabled != node_tool->define_enabled)
		{
			node_tool->define_enabled=define_enabled;
#if defined (MOTIF)
			/* make sure button shows current state */
			if (XmToggleButtonGadgetGetState(node_tool->define_button))
			{
				button_state=1;
			}
			else
			{
				button_state=0;
			}
			if (button_state != node_tool->define_enabled)
			{
				XmToggleButtonGadgetSetState(node_tool->define_button,
					/*state*/node_tool->define_enabled,/*notify*/False);
			}
#endif /* defined (MOTIF) */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_set_define_enabled.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_tool_set_define_enabled */

int Node_tool_get_edit_enabled(struct Node_tool *node_tool)
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Returns flag controlling whether node edits are updated during motion_notify
events, not just at the end of a mouse gesture.
==============================================================================*/
{
	int edit_enabled;

	ENTER(Node_tool_get_edit_enabled);
	if (node_tool)
	{
		edit_enabled=node_tool->edit_enabled;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_get_edit_enabled.  Invalid argument(s)");
		edit_enabled=0;
	}
	LEAVE;

	return (edit_enabled);
} /* Node_tool_get_edit_enabled */

int Node_tool_set_edit_enabled(struct Node_tool *node_tool,int edit_enabled)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Sets flag controlling whether node edits are updated during motion_notify
events, not just at the end of a mouse gesture.
==============================================================================*/
{
	int return_code;
#if defined (MOTIF)
	int button_state;
#endif /* defined (MOTIF) */

	ENTER(Node_tool_set_edit_enabled);
	if (node_tool)
	{
		/* make sure value of flag is 1 */
		if (edit_enabled)
		{
			edit_enabled=1;
		}
		if (edit_enabled != node_tool->edit_enabled)
		{
			node_tool->edit_enabled=edit_enabled;
#if defined (MOTIF)
			/* make sure button shows current state */
			if (XmToggleButtonGadgetGetState(node_tool->edit_button))
			{
				button_state=1;
			}
			else
			{
				button_state=0;
			}
			if (button_state != node_tool->edit_enabled)
			{
				XmToggleButtonGadgetSetState(node_tool->edit_button,
					/*state*/node_tool->edit_enabled,/*notify*/False);
			}
#endif /* defined (MOTIF) */
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_set_edit_enabled.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_tool_set_edit_enabled */

enum Node_tool_edit_mode Node_tool_get_edit_mode(struct Node_tool *node_tool)
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Returns the current edit mode of <node_tool>.
==============================================================================*/
{
	enum Node_tool_edit_mode edit_mode;

	ENTER(Node_tool_get_edit_mode);
	if (node_tool)
	{
		edit_mode=node_tool->edit_mode;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_get_edit_mode.  Invalid argument(s)");
		/* return anything for error condition */
		edit_mode=NODE_TOOL_EDIT_AUTOMATIC;
	}
	LEAVE;

	return (edit_mode);
} /* Node_tool_get_edit_mode */

int Node_tool_set_edit_mode(struct Node_tool *node_tool,
	enum Node_tool_edit_mode edit_mode)
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Sets the <edit_mode> of <node_tool> - controls whether the editor
can select or edit nodes, and whether the editing is restricted to position or
vector only.
==============================================================================*/
{
	int return_code;

	ENTER(Node_tool_set_edit_mode);
	if (node_tool)
	{
		node_tool->edit_mode=edit_mode;
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_set_edit_mode.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_tool_set_edit_mode */

int Node_tool_get_motion_update_enabled(struct Node_tool *node_tool)
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Returns flag controlling whether node edits are updated during motion_notify
events, not just at the end of a mouse gesture.
==============================================================================*/
{
	int motion_update_enabled;

	ENTER(Node_tool_get_motion_update_enabled);
	if (node_tool)
	{
		motion_update_enabled=node_tool->motion_update_enabled;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_get_motion_update_enabled.  Invalid argument(s)");
		motion_update_enabled=0;
	}
	LEAVE;

	return (motion_update_enabled);
} /* Node_tool_get_motion_update_enabled */

int Node_tool_set_motion_update_enabled(struct Node_tool *node_tool,
	int motion_update_enabled)
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Sets flag controlling whether node edits are updated during motion_notify
events, not just at the end of a mouse gesture.
==============================================================================*/
{
	int return_code;
#if defined (MOTIF)
	int button_state;
#endif /* defined (MOTIF) */

	ENTER(Node_tool_set_motion_update_enabled);
	if (node_tool)
	{
		/* make sure value of flag is 1 */
		if (motion_update_enabled)
		{
			motion_update_enabled=1;
		}
		if (motion_update_enabled != node_tool->motion_update_enabled)
		{
			node_tool->motion_update_enabled=motion_update_enabled;
#if defined (MOTIF)
			/* make sure button shows current state */
			if (XmToggleButtonGadgetGetState(node_tool->motion_update_button))
			{
				button_state=1;
			}
			else
			{
				button_state=0;
			}
			if (button_state != node_tool->motion_update_enabled)
			{
				XmToggleButtonGadgetSetState(node_tool->motion_update_button,
					/*state*/node_tool->motion_update_enabled,/*notify*/False);
			}
#endif /* defined (MOTIF) */
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_set_motion_update_enabled.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_tool_set_motion_update_enabled */

struct GROUP(FE_node) *Node_tool_get_node_group(struct Node_tool *node_tool)
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Returns the node group new nodes are created in by <node_tool>.
==============================================================================*/
{
	struct GROUP(FE_node) *node_group;

	ENTER(Node_tool_get_node_group);
	if (node_tool)
	{
		node_group=node_tool->node_group;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_get_node_group.  Invalid argument(s)");
		node_group=(struct GROUP(FE_node) *)NULL;
	}
	LEAVE;

	return (node_group);
} /* Node_tool_get_node_group */

int Node_tool_set_node_group(struct Node_tool *node_tool,
	struct GROUP(FE_node) *node_group)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Sets the node group new nodes are created in by <node_tool>.
==============================================================================*/
{
	int return_code;

	ENTER(Node_tool_set_node_group);
	if (node_tool)
	{
		return_code=1;
		if (node_group != node_tool->node_group)
		{
			node_tool->node_group=node_group;
#if defined (MOTIF)
			/* make sure the current group is shown */
			CHOOSE_OBJECT_SET_OBJECT(GROUP(FE_node))(
				node_tool->node_group_widget,node_tool->node_group);
#endif /* defined (MOTIF) */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_set_node_group.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_tool_set_node_group */

int Node_tool_get_select_enabled(struct Node_tool *node_tool)
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Returns flag controlling whether existing nodes can be selected.
==============================================================================*/
{
	int select_enabled;

	ENTER(Node_tool_get_select_enabled);
	if (node_tool)
	{
		select_enabled=node_tool->select_enabled;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_get_select_enabled.  Invalid argument(s)");
		select_enabled=0;
	}
	LEAVE;

	return (select_enabled);
} /* Node_tool_get_select_enabled */

int Node_tool_set_select_enabled(struct Node_tool *node_tool,
	int select_enabled)
/*******************************************************************************
LAST MODIFIED : 18 July 2000

DESCRIPTION :
Sets flag controlling whether existing nodes can be selected.
==============================================================================*/
{
	int return_code;
#if defined (MOTIF)
	int button_state;
#endif /* defined (MOTIF) */

	ENTER(Node_tool_set_select_enabled);
	if (node_tool)
	{
		/* make sure value of flag is 1 */
		if (select_enabled)
		{
			select_enabled=1;
		}
		if (select_enabled != node_tool->select_enabled)
		{
			node_tool->select_enabled=select_enabled;
#if defined (MOTIF)
			/* make sure button shows current state */
			if (XmToggleButtonGadgetGetState(node_tool->select_button))
			{
				button_state=1;
			}
			else
			{
				button_state=0;
			}
			if (button_state != node_tool->select_enabled)
			{
				XmToggleButtonGadgetSetState(node_tool->select_button,
					/*state*/node_tool->select_enabled,/*notify*/False);
			}
#endif /* defined (MOTIF) */
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_set_select_enabled.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_tool_set_select_enabled */

int Node_tool_get_streaming_create_enabled(struct Node_tool *node_tool)
/*******************************************************************************
LAST MODIFIED : 14 May 2001

DESCRIPTION :
Returns flag controlling, if create_enabled, whether a stream of nodes is
created as the user drags the mouse around.
==============================================================================*/
{
	int streaming_create_enabled;

	ENTER(Node_tool_get_streaming_create_enabled);
	if (node_tool)
	{
		streaming_create_enabled = node_tool->streaming_create_enabled;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_get_streaming_create_enabled.  Invalid argument(s)");
		streaming_create_enabled = 0;
	}
	LEAVE;

	return (streaming_create_enabled);
} /* Node_tool_get_streaming_create_enabled */

int Node_tool_set_streaming_create_enabled(struct Node_tool *node_tool,
	int streaming_create_enabled)
/*******************************************************************************
LAST MODIFIED : 14 May 2001

DESCRIPTION :
Sets flag controlling, if create_enabled, whether a stream of nodes is
created as the user drags the mouse around.
==============================================================================*/
{
	int return_code;
#if defined (MOTIF)
	int button_state;
#endif /* defined (MOTIF) */

	ENTER(Node_tool_set_streaming_create_enabled);
	if (node_tool)
	{
		if (streaming_create_enabled)
		{
			/* make sure value of flag is exactly 1 */
			streaming_create_enabled = 1;
		}
		if (streaming_create_enabled != node_tool->streaming_create_enabled)
		{
			node_tool->streaming_create_enabled = streaming_create_enabled;
#if defined (MOTIF)
			/* make sure button shows current state */
			if (XmToggleButtonGadgetGetState(node_tool->streaming_create_button))
			{
				button_state = 1;
			}
			else
			{
				button_state = 0;
			}
			if (button_state != node_tool->streaming_create_enabled)
			{
				XmToggleButtonGadgetSetState(node_tool->streaming_create_button,
					/*state*/node_tool->streaming_create_enabled, /*notify*/False);
			}
#endif /* defined (MOTIF) */
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_set_streaming_create_enabled.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Node_tool_set_streaming_create_enabled */

struct Computed_field *Node_tool_get_url_field(
	struct Node_tool *node_tool)
/*******************************************************************************
LAST MODIFIED : 4 July 2002

DESCRIPTION :
Returns the url_field to be looked up in a web browser when the node is clicked
on in the <node_tool>.
==============================================================================*/
{
	struct Computed_field *url_field;

	ENTER(Node_tool_get_url_field);
	if (node_tool)
	{
		url_field=node_tool->url_field;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_get_url_field.  Invalid argument(s)");
		url_field=(struct Computed_field *)NULL;
	}
	LEAVE;

	return (url_field);
} /* Node_tool_get_url_field */

int Node_tool_set_url_field(struct Node_tool *node_tool,
	struct Computed_field *url_field)
/*******************************************************************************
LAST MODIFIED : 4 July 2002

DESCRIPTION :
Sets the url_field to be looked up in a web browser when the node is clicked on
in the <node_tool>.
==============================================================================*/
{
	int field_set, return_code;

	ENTER(Node_tool_set_url_field);
	if (node_tool && ((!url_field) ||
		Computed_field_has_string_value_type(url_field, (void *)NULL)))
	{
		return_code = 1;
		if (url_field != node_tool->url_field)
		{
			node_tool->url_field = url_field;
#if defined (MOTIF)
			if (url_field)
			{
				CHOOSE_OBJECT_SET_OBJECT(Computed_field)(
					node_tool->url_field_widget,node_tool->url_field);
			}
#endif /* defined (MOTIF) */
			field_set = ((struct Computed_field *)NULL != url_field);
#if defined (MOTIF)
			XtVaSetValues(node_tool->url_field_button,
				XmNset, (XtPointer)field_set, NULL);
			XtSetSensitive(node_tool->url_field_widget, field_set);
#endif /* defined (MOTIF) */
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_set_url_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_tool_set_url_field */
