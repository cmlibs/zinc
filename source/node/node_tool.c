/*******************************************************************************
FILE : node_tool.c

LAST MODIFIED : 12 June 2000

DESCRIPTION :
Functions for mouse controlled node position and vector editing based on
Scene input.
==============================================================================*/
#include <math.h>
#include "general/debug.h"
#include "general/matrix_vector.h"
#include "graphics/element_group_settings.h"
#include "graphics/graphical_element.h"
#include "graphics/graphics_object.h"
#include "interaction/interaction_volume.h"
#include "interaction/interactive_event.h"
#include "node/node_tool.h"
#include "node/node_tool.uidh"
#include "user_interface/message.h"

/*
Module variables
----------------
*/

#if defined (MOTIF)
static int node_tool_hierarchy_open=0;
static MrmHierarchy node_tool_hierarchy;
#endif /* defined (MOTIF) */

/*
Module types
------------
*/

struct Node_tool
/*******************************************************************************
LAST MODIFIED : 15 May 2000

DESCRIPTION :
Object storing all the parameters for converting scene input messages into
changes in node position and derivatives etc.
==============================================================================*/
{
	struct MANAGER(Interactive_tool) *interactive_tool_manager;
	struct Interactive_tool *interactive_tool;
	struct MANAGER(FE_node) *node_manager;
	/* flag indicating that the above manager is actually the data manager */
	int data_manager;
	struct FE_node_selection *node_selection;
	struct Computed_field_package *computed_field_package;
	/* user-settable flags */
	/* indicates whether node edits can occur with motion_notify events: slower */
	int motion_update_enabled;
	/* indicates whether existing nodes can be selected */
	int select_enabled;
	/* indicates whether selected nodes can be edited */
	int edit_enabled;
	/* indicates whether new nodes will can be created */
	int create_enabled;
	enum Node_tool_edit_mode edit_mode;
	struct GROUP(FE_node) *node_group;
	struct FE_field *coordinate_field;
	struct FE_node *template_node;
	/* information about picked nodes the editor knows about */
	struct Scene_picked_object *scene_picked_object;
	struct FE_node *last_picked_node;
	int picked_node_was_unselected;
	int motion_detected;
	struct GT_element_group *gt_element_group;
	struct GT_element_settings *gt_element_settings;
	struct Interaction_volume *last_interaction_volume;
}; /* struct Node_tool */

struct FE_node_edit_information
/*******************************************************************************
LAST MODIFIED : 27 April 2000

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
	/* the gesture indicated by the mouse is given by initial and final
		 interaction volumes */
	struct Interaction_volume *final_interaction_volume,
		*initial_interaction_volume;
	/* the coordinate field to change in the translation */
	struct Computed_field *coordinate_field;
	/* The same field wrapped to get RC coordinates */
	struct Computed_field *rc_coordinate_field;
	/* following required for EDIT_VECTOR only */
	struct Computed_field *orientation_scale_field,
		*wrapper_orientation_scale_field;
	Triple glyph_centre,glyph_scale_factors,glyph_size;
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

static int Node_tool_create_template_node(struct Node_tool *node_tool)
/*******************************************************************************
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Ensures there is a template node defined with the coordinate_field in the
<node_tool> for creating new nodes as copies of.
==============================================================================*/
{
	enum FE_nodal_value_type *components_nodal_value_types[3]=
	{
		{FE_NODAL_VALUE},
		{FE_NODAL_VALUE},
		{FE_NODAL_VALUE}
	};
	int components_number_of_derivatives[3]={0,0,0},
		components_number_of_versions[3]={1,1,1},return_code;

	ENTER(Node_tool_create_template_node);
	if (node_tool)
	{
		if (node_tool->coordinate_field)
		{
			if (node_tool->node_group)
			{
				return_code=1;
				if (!node_tool->template_node)
				{
					if (node_tool->template_node=CREATE(FE_node)(
						/*node_number*/0,(struct FE_node *)NULL))
					{
						/* template_node is accessed by node_tool but not managed */
						ACCESS(FE_node)(node_tool->template_node);
						if (!define_FE_field_at_node(
							node_tool->template_node,
							node_tool->coordinate_field,
							components_number_of_derivatives,
							components_number_of_versions,
							components_nodal_value_types))
						{
							display_message(ERROR_MESSAGE,"Node_tool_create_template_node.  "
								"Could not define coordinate field in template_node");
							DEACCESS(FE_node)(&(node_tool->template_node));
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
				"Node_tool_create_template_node.  No coordinate_field specified");
			return_code=0;
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
			node,coordinates))
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
						edit_info->coordinate_field,node,initial_coordinates)&&
						Computed_field_set_values_at_managed_node(
							edit_info->rc_coordinate_field,node,coordinates,
							edit_info->node_manager)&&
						Computed_field_evaluate_at_node(edit_info->coordinate_field,
							node,final_coordinates);
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
LAST MODIFIED : 22 March 2000

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
			if (Computed_field_evaluate_at_node(edit_info->coordinate_field,
				node,coordinates))
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
			else
			{
				return_code=0;
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
LAST MODIFIED : 27 April 2000

DESCRIPTION :
Moves the end of the vector to exactly under the mouse, in the plane normal to the view direction at its current depth. Hence, this function should only be
called for a single node.
Note that you must supply an orientation_scale field, while glyph_size[0] and
glyph_centre[0] must be 0.0, and glyph_scale_factors[0] must be non-zero.
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
		(0.0 != (scale_factor=edit_info->glyph_scale_factors[0])))
	{
		return_code=1;
		/* clear coordinates in case less than 3 dimensions */
		coordinates[0]=0.0;
		coordinates[1]=0.0;
		coordinates[2]=0.0;
		if (Computed_field_evaluate_at_node(
			edit_info->wrapper_orientation_scale_field,node,orientation_scale)&&
			Computed_field_evaluate_at_node(edit_info->rc_coordinate_field,
				node,coordinates)&&
			make_glyph_orientation_scale_axes(number_of_orientation_scale_components,
				orientation_scale,a,b,c,size))
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
						edit_info->orientation_scale_field,node,orientation_scale))
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
				edit_info->orientation_scale_field,node,orientation_scale))
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

static struct FE_node *Node_tool_create_node(struct Node_tool *node_tool,
	struct Scene *scene,struct Interaction_volume *interaction_volume)
/*******************************************************************************
LAST MODIFIED : 15 May 2000

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

	ENTER(Node_tool_create_node);
	node=(struct FE_node *)NULL;
	scene_picked_object=(struct Scene_picked_object *)NULL;
	gt_element_group=(struct GT_element_group *)NULL;
	gt_element_settings=(struct GT_element_settings *)NULL;
	if (node_tool&&interaction_volume)
	{
		if (Node_tool_create_template_node(node_tool))
		{
			if (node_tool->data_manager)
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
				if (gt_element_settings=first_settings_in_GT_element_group_that(
					gt_element_group,GT_element_settings_type_matches,
					(void *)gt_element_settings_type))
				{
					coordinate_field=
						GT_element_settings_get_coordinate_field(gt_element_settings);
				}
				else
				{
					coordinate_field=(struct Computed_field *)NULL;
				}
				if (!coordinate_field)
				{
					coordinate_field=
						GT_element_group_get_default_coordinate_field(gt_element_group);
				}
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
				coordinate_field=FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
					Computed_field_is_read_only_with_fe_field,
					(void *)node_tool->coordinate_field,
					Computed_field_package_get_computed_field_manager(
						node_tool->computed_field_package));
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
						"Node_tool_create_node.  Could not create node");
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
				"Node_tool_create_node.  Could not create template node");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_create_node.  Invalid argument(s)");
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
} /* Node_tool_create_node */

static void Node_tool_interactive_event_handler(void *device_id,
	struct Interactive_event *event,void *node_tool_void)
/*******************************************************************************
LAST MODIFIED : 15 May 2000

DESCRIPTION :
Input handler for input from devices. <device_id> is a unique address enabling
the editor to handle input from more than one device at a time. The <event>
describes the type of event, button numbers and key modifiers, and the volume
of space affected by the interaction. Main events are button press, movement and
release.
==============================================================================*/
{
	double d;
	enum Interactive_event_type event_type;
	int clear_selection,input_modifier,number_of_selected_nodes,return_code,
		shift_pressed;
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
					if (scene_picked_object_list=
						Scene_pick_objects(scene,interaction_volume))
					{
						picked_node=(struct FE_node *)NULL;
						if (node_tool->select_enabled)
						{
							picked_node=Scene_picked_object_list_get_nearest_node(
								scene_picked_object_list,node_tool->node_manager,
								node_tool->data_manager,(struct GROUP(FE_node) *)NULL,
								&scene_picked_object,&gt_element_group,&gt_element_settings);
						}
						if (picked_node)
						{
							node_tool->picked_node_was_unselected=
								!FE_node_selection_is_node_selected(node_tool->node_selection,
									picked_node);
							REACCESS(Scene_picked_object)(&(node_tool->scene_picked_object),
								scene_picked_object);
							REACCESS(GT_element_group)(&(node_tool->gt_element_group),
								gt_element_group);
							REACCESS(GT_element_settings)(&(node_tool->gt_element_settings),
								gt_element_settings);
						}
						else
						{
							if (node_tool->create_enabled&&(picked_node=
								Node_tool_create_node(node_tool,scene,interaction_volume)))
							{
								node_tool->picked_node_was_unselected=1;
							}
							else
							{
								node_tool->picked_node_was_unselected=0;
							}
						}
						REACCESS(FE_node)(&(node_tool->last_picked_node),picked_node);
						if (clear_selection=((!shift_pressed)&&((!picked_node)||
							(node_tool->picked_node_was_unselected))))
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
						node_tool->motion_detected=0;
						DESTROY(LIST(Scene_picked_object))(&(scene_picked_object_list));
					}
				} break;
				case INTERACTIVE_EVENT_MOTION_NOTIFY:
				case INTERACTIVE_EVENT_BUTTON_RELEASE:
				{
					if (INTERACTIVE_EVENT_MOTION_NOTIFY==event_type)
					{
						node_tool->motion_detected=1;
					}
					if (node_tool->last_picked_node)
					{
						if ((node_tool->edit_enabled)&&node_tool->motion_detected &&
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
							if (node_tool->data_manager)
							{
								edit_info.node_group=GT_element_group_get_data_group(
									node_tool->gt_element_group);
							}
							else
							{
								edit_info.node_group=GT_element_group_get_node_group(
									node_tool->gt_element_group);
							}
							/* get coordinate field in RC coordinates */
							if (!(coordinate_field=GT_element_settings_get_coordinate_field(
								node_tool->gt_element_settings)))
							{
								coordinate_field=
									GT_element_group_get_default_coordinate_field(
										node_tool->gt_element_group);
							}
							edit_info.coordinate_field=coordinate_field;
							edit_info.rc_coordinate_field=
								Computed_field_begin_wrap_coordinate_field(coordinate_field);
							edit_info.orientation_scale_field=(struct Computed_field *)NULL;
							edit_info.wrapper_orientation_scale_field=
								(struct Computed_field *)NULL;
							if (GT_element_settings_get_glyph_parameters(
								node_tool->gt_element_settings,&glyph,
								edit_info.glyph_centre,edit_info.glyph_size,
								&edit_info.orientation_scale_field,
								edit_info.glyph_scale_factors))
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
									number_of_selected_nodes=NUMBER_IN_LIST(FE_node)(node_list);
									if (1<number_of_selected_nodes)
									{
										/* cache manager so only one change message produced */
										MANAGER_BEGIN_CACHE(FE_node)(node_tool->node_manager);
									}
									/* edit vectors if non-constant orientation_scale field */
									if (((NODE_TOOL_EDIT_AUTOMATIC == node_tool->edit_mode) ||
										(NODE_TOOL_EDIT_VECTOR == node_tool->edit_mode))&&
										edit_info.wrapper_orientation_scale_field&&
										(!(COMPUTED_FIELD_CONSTANT==Computed_field_get_type(
											edit_info.orientation_scale_field))))
									{
										/* edit vector */
										if (FE_node_calculate_delta_vector(
											node_tool->last_picked_node,(void *)&edit_info))
										{
											if (1<number_of_selected_nodes)
											{
												FOR_EACH_OBJECT_IN_LIST(FE_node)(FE_node_edit_vector,
													(void *)&edit_info,node_list);
											}
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
												if (1<number_of_selected_nodes)
												{
													FOR_EACH_OBJECT_IN_LIST(FE_node)(
														FE_node_edit_position,(void *)&edit_info,node_list);
												}
											}
										}
										else
										{
											display_message(ERROR_MESSAGE,
												"Cannot edit vector: invalid orientation_scale field");
											return_code=0;
										}
									}
									if (1<number_of_selected_nodes)
									{
										MANAGER_END_CACHE(FE_node)(node_tool->node_manager);
									}
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
					else if ((INTERACTIVE_EVENT_BUTTON_RELEASE==event_type)&&
						(node_tool->motion_detected))
					{
						/* rubber band select */
						if (temp_interaction_volume=create_Interaction_volume_bounding_box(
							node_tool->last_interaction_volume,interaction_volume))
						{
							if (scene_picked_object_list=
								Scene_pick_objects(scene,temp_interaction_volume))
							{
								if (node_list=Scene_picked_object_list_get_picked_nodes(
									scene_picked_object_list,node_tool->node_manager,
									node_tool->data_manager))
								{
									FE_node_selection_begin_cache(node_tool->node_selection);
									FOR_EACH_OBJECT_IN_LIST(FE_node)(
										FE_node_select_in_FE_node_selection,
										(void *)(node_tool->node_selection),node_list);
									FE_node_selection_end_cache(node_tool->node_selection);
									DESTROY(LIST(FE_node))(&node_list);
								}
								DESTROY(LIST(Scene_picked_object))(&(scene_picked_object_list));
							}
							DESTROY(Interaction_volume)(&temp_interaction_volume);
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
		if (INTERACTIVE_EVENT_BUTTON_RELEASE==event_type)
		{
			/* deaccess following as only kept for one input movement */
			REACCESS(FE_node)(&(node_tool->last_picked_node),(struct FE_node *)NULL);
			REACCESS(Interaction_volume)(&(node_tool->last_interaction_volume),
				(struct Interaction_volume *)NULL);
			REACCESS(Scene_picked_object)(&(node_tool->scene_picked_object),
				(struct Scene_picked_object *)NULL);
			REACCESS(GT_element_group)(&(node_tool->gt_element_group),
				(struct GT_element_group *)NULL);
			REACCESS(GT_element_settings)(&(node_tool->gt_element_settings),
				(struct GT_element_settings *)NULL);
		}
		else if ((INTERACTIVE_EVENT_BUTTON_PRESS==event_type)||
			(node_tool->last_picked_node&&node_tool->motion_update_enabled))
		{
			REACCESS(Interaction_volume)(&(node_tool->last_interaction_volume),
				interaction_volume);
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
LAST MODIFIED : 16 May 2000

DESCRIPTION :
Brings up a dialog for editing settings of the Node_tool - in a standard format
for passing to an Interactive_toolbar.
==============================================================================*/
{
	int return_code;
	struct Node_tool *node_tool;

	ENTER(Node_tool_bring_up_interactive_tool_dialog);
	if (node_tool=(struct Node_tool *)node_tool_void)
	{
		/* bring up the dialog */
		display_message(INFORMATION_MESSAGE,
			"Node_tool_bring_up_interactive_tool_dialog.  Not implemented\n");
		USE_PARAMETER(node_tool);
		return_code=0;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_bring_up_interactive_tool_dialog.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_tool_bring_up_interactive_tool_dialog */

static Widget Node_tool_make_interactive_tool_button(
	void *node_tool_void,Widget parent)
/*******************************************************************************
LAST MODIFIED : 16 May 2000

DESCRIPTION :
Fetches a ToggleButton with an appropriate icon for the interactive tool
and as a child of <parent>.
==============================================================================*/
{
	char *widget_name;
	MrmType node_tool_dialog_class;
	struct Node_tool *node_tool;
	Widget widget;

	ENTER(Node_tool_make_interactive_tool_button);
	widget=(Widget)NULL;
	if ((node_tool=(struct Node_tool *)node_tool_void)&&parent)
	{
		if (MrmOpenHierarchy_base64_string(node_tool_uidh,
			&node_tool_hierarchy,&node_tool_hierarchy_open))
		{
			if (node_tool->data_manager)
			{
				widget_name="data_tool_button";
			}
			else
			{
				widget_name="node_tool_button";
			}
			if (MrmSUCCESS == MrmFetchWidget(node_tool_hierarchy,
				widget_name,parent,&widget,&node_tool_dialog_class))
			{
				XtVaSetValues(widget,XmNuserData,node_tool->interactive_tool,NULL);
			}
			else
			{
				display_message(WARNING_MESSAGE,
					"Node_tool_make_interactive_tool_button.  Could not fetch widget");
			}
		}
		else
		{
			display_message(WARNING_MESSAGE,
				"Node_tool_make_interactive_tool_button.  Could not open heirarchy");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_make_interactive_tool_button.  Invalid argument(s)");
	}
	LEAVE;

	return (widget);
} /* Node_tool_make_interactive_tool_button */

/*
Global functions
----------------
*/

struct Node_tool *CREATE(Node_tool)(
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
	struct MANAGER(FE_node) *node_manager,int data_manager,
	struct FE_node_selection *node_selection,
	struct Computed_field_package *computed_field_package)
/*******************************************************************************
LAST MODIFIED : 15 May 2000

DESCRIPTION :
Creates a Node_tool for editing nodes/data in the <node_manager>,
using the <node_selection>.
The <data_manager> flag indicates that <node_manager> and <node_selection>
refer to data, not nodes, needed since different GT_element_settings types are
used to represent them. 
==============================================================================*/
{
	char *tool_display_name,*tool_name;
	struct Node_tool *node_tool;

	ENTER(CREATE(Node_tool));
	if (interactive_tool_manager&&node_manager&&node_selection&&
		computed_field_package)
	{
		if (ALLOCATE(node_tool,struct Node_tool,1))
		{
			node_tool->interactive_tool_manager=interactive_tool_manager;
			node_tool->node_manager=node_manager;
			node_tool->data_manager=data_manager;
			node_tool->node_selection=node_selection;
			node_tool->computed_field_package=computed_field_package;
			node_tool->scene_picked_object=(struct Scene_picked_object *)NULL;
			node_tool->last_picked_node=(struct FE_node *)NULL;
			node_tool->gt_element_group=(struct GT_element_group *)NULL;
			node_tool->gt_element_settings=(struct GT_element_settings *)NULL;
			/* user-settable flags */
			node_tool->motion_update_enabled=1;
			node_tool->select_enabled=1;
			node_tool->edit_enabled=0;
			node_tool->create_enabled=0;
			node_tool->edit_mode=NODE_TOOL_EDIT_AUTOMATIC;
			node_tool->node_group=(struct GROUP(FE_node) *)NULL;
			node_tool->coordinate_field=(struct FE_field *)NULL;
			node_tool->template_node=(struct FE_node *)NULL;
			/* interactive_tool */
			if (data_manager)
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
				Node_tool_interactive_event_handler,
				Node_tool_make_interactive_tool_button,
				Node_tool_bring_up_interactive_tool_dialog,
				(void *)node_tool);
			ADD_OBJECT_TO_MANAGER(Interactive_tool)(
				node_tool->interactive_tool,
				node_tool->interactive_tool_manager);
			node_tool->last_interaction_volume=(struct Interaction_volume *)NULL;
		}
		else
		{
			display_message(ERROR_MESSAGE,"CREATE(Node_tool).  Not enough memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Node_tool).  Invalid argument(s)");
		node_tool=(struct Node_tool *)NULL;
	}
	LEAVE;

	return (node_tool);
} /* CREATE(Node_tool) */

int DESTROY(Node_tool)(struct Node_tool **node_tool_address)
/*******************************************************************************
LAST MODIFIED : 11 May 2000

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
		REACCESS(FE_node)(&(node_tool->template_node),(struct FE_node *)NULL);
		REACCESS(FE_field)(&(node_tool->coordinate_field),(struct FE_field *)NULL);
		REACCESS(GROUP(FE_node))(&(node_tool->node_group),
			(struct GROUP(FE_node) *)NULL);
		DEALLOCATE(*node_tool_address);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"DESTROY(Node_tool).  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} /* DESTROY(Node_tool) */

struct FE_field *Node_tool_get_coordinate_field(struct Node_tool *node_tool)
/*******************************************************************************
LAST MODIFIED : 17 May 2000

DESCRIPTION :
Returns the coordinate field of nodes created by <node_tool>.
==============================================================================*/
{
	struct FE_field *coordinate_field;

	ENTER(Node_tool_get_coordinate_field);
	if (node_tool)
	{
		coordinate_field=node_tool->coordinate_field;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_get_coordinate_field.  Invalid argument(s)");
		coordinate_field=(struct FE_field *)NULL;
	}
	LEAVE;

	return (coordinate_field);
} /* Node_tool_get_coordinate_field */

int Node_tool_set_coordinate_field(struct Node_tool *node_tool,
	struct FE_field *coordinate_field)
/*******************************************************************************
LAST MODIFIED : 17 May 2000

DESCRIPTION :
Sets the coordinate field of nodes created by <node_tool>.
==============================================================================*/
{
	int return_code;

	ENTER(Node_tool_set_coordinate_field);
	if (node_tool)
	{
		return_code=1;
		if (coordinate_field != node_tool->coordinate_field)
		{
			if (coordinate_field)
			{
				if ((3<get_FE_field_number_of_components(coordinate_field))||
					(FE_VALUE_VALUE != get_FE_field_value_type(coordinate_field)))
				{
					display_message(ERROR_MESSAGE,
						"Node_tool_set_coordinate_field.  "
						"Invalid number of components or value type");
					return_code=0;
				}
			}
			if (return_code)
			{
				REACCESS(FE_field)(&(node_tool->coordinate_field),coordinate_field);
				/* lose the current template node, if any */
				REACCESS(FE_node)(&(node_tool->template_node),(struct FE_node *)NULL);
			}
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
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Returns flag controlling whether node edits are updated during motion_notify
events, not just at the end of a mouse gesture.
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
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Sets flag controlling whether node edits are updated during motion_notify
events, not just at the end of a mouse gesture.
==============================================================================*/
{
	int return_code;

	ENTER(Node_tool_set_create_enabled);
	if (node_tool)
	{
		if (create_enabled)
		{
			node_tool->create_enabled=1;
		}
		else
		{
			node_tool->create_enabled=0;
		}
		return_code=1;
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
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Sets flag controlling whether node edits are updated during motion_notify
events, not just at the end of a mouse gesture.
==============================================================================*/
{
	int return_code;

	ENTER(Node_tool_set_edit_enabled);
	if (node_tool)
	{
		if (edit_enabled)
		{
			node_tool->edit_enabled=1;
		}
		else
		{
			node_tool->edit_enabled=0;
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

	ENTER(Node_tool_set_motion_update_enabled);
	if (node_tool)
	{
		if (motion_update_enabled)
		{
			node_tool->motion_update_enabled=1;
		}
		else
		{
			node_tool->motion_update_enabled=0;
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
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Sets the node group new nodes are created in by <node_tool>.
==============================================================================*/
{
	int return_code;

	ENTER(Node_tool_set_node_group);
	if (node_tool)
	{
		REACCESS(GROUP(FE_node))(&(node_tool->node_group),node_group);
		return_code=1;
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
Returns flag controlling whether node edits are updated during motion_notify
events, not just at the end of a mouse gesture.
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
LAST MODIFIED : 11 May 2000

DESCRIPTION :
Sets flag controlling whether node edits are updated during motion_notify
events, not just at the end of a mouse gesture.
==============================================================================*/
{
	int return_code;

	ENTER(Node_tool_set_select_enabled);
	if (node_tool)
	{
		if (select_enabled)
		{
			node_tool->select_enabled=1;
		}
		else
		{
			node_tool->select_enabled=0;
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
