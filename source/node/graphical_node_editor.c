/*******************************************************************************
FILE : graphical_node_editor.c

LAST MODIFIED : 29 February 2000

DESCRIPTION :
Functions for mouse controlled node position and vector editing based on
Scene input.
==============================================================================*/
#include <math.h>
#include "command/command.h"
/*#include "finite_element/finite_element_to_graphics_object.h"*/
#include "general/debug.h"
/*#include "general/geometry.h"*/
#include "general/matrix_vector.h"
#include "graphics/element_group_settings.h"
#include "graphics/graphical_element.h"
#include "graphics/graphics_object.h"
#include "graphics/scene.h"
#include "node/graphical_node_editor.h"
#include "user_interface/message.h"
/*#include "user_interface/user_interface.h"*/

/*
Module types
------------
*/
struct Graphical_node_editor
/*******************************************************************************
LAST MODIFIED : 21 February 2000

DESCRIPTION :
Object storing all the parameters for converting scene input messages into
changes in node position and derivatives etc.
==============================================================================*/
{
	struct Scene *scene;
	struct MANAGER(FE_node) *node_manager;
	/* information about picked nodes the editor knows about */
	struct Scene_picked_object *scene_picked_object;
	struct FE_node *last_picked_node;
	int last_picked_node_just_added;
	int motion_detected;
	struct GT_element_group *gt_element_group;
	struct GT_element_settings *gt_element_settings;
	double last_farx,last_fary,last_farz,last_nearx,last_neary,last_nearz;
	/* the element and node groups objects are movied in */
	struct GROUP(FE_element) *element_group;
	struct GROUP(FE_node) *node_group;
}; /* struct Graphical_node_editor */

struct FE_node_edit_information
/*******************************************************************************
LAST MODIFIED : 22 February 2000

DESCRIPTION :
Describes how to move a node in space. The node will move on the plane normal
to the viewing direction a distance proportional to the two starting and
finishing points on the near and far plane. The exact amount is in proportion
to its position between these two planes.
==============================================================================*/
{
	/* the actual coordinate change calculated from the drag at the last picked node */
	double delta1,delta2,delta3;
	/* the direction we are viewing along */
	double viewx,viewy,viewz;
	/* the distance between near and far planes along the view direction */
	double near_to_far;
	/* starting points on the near and far planes */
	double near1x,near1y,near1z,far1x,far1y,far1z;
	/* ending points on the near and far planes */
	double near2x,near2y,near2z,far2x,far2y,far2z;
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

static int FE_node_calculate_delta_position(struct FE_node *node,void *edit_info_void)
/*******************************************************************************
LAST MODIFIED : 29 February 2000

DESCRIPTION :
Calculates the delta change in the coordinates due to the ray supplied in the
<edit_info>.  This change is set inside the <edit_info> and this can then be 
applied to multiple nodes.
==============================================================================*/
{
	double fact;
	FE_value coordinates[3], initial_coordinates[3], final_coordinates[3];
	int return_code;
	struct FE_node_edit_information *edit_info;

	ENTER(FE_node_edit_position);
	if (node&&(edit_info=(struct FE_node_edit_information *)edit_info_void)&&
		edit_info->node_manager&&edit_info->rc_coordinate_field&&
		(3>=Computed_field_get_number_of_components(
			edit_info->rc_coordinate_field))&&
		(0.0<edit_info->near_to_far))
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
				/* translate the node */
				/* need ratio of distance from the near plane to <node> over distance
					 between near and far planes, along the view direction: */
				fact=((coordinates[0] - edit_info->near1x)*edit_info->viewx+
					(coordinates[1] - edit_info->near1y)*edit_info->viewy+
					(coordinates[2] - edit_info->near1z)*edit_info->viewz) /
					edit_info->near_to_far;
				/* interpolate near and far displacements to displacement at node */
				coordinates[0] += fact*(edit_info->far2x-edit_info->far1x) +
					(1.0-fact)*(edit_info->near2x-edit_info->near1x);
				coordinates[1] += fact*(edit_info->far2y-edit_info->far1y) +
					(1.0-fact)*(edit_info->near2y-edit_info->near1y);
				coordinates[2] += fact*(edit_info->far2z-edit_info->far1z) +
					(1.0-fact)*(edit_info->near2z-edit_info->near1z);
				if (edit_info->transformation_required)
				{
					return_code=world_to_model_coordinates(coordinates,
						edit_info->LU_transformation_matrix,edit_info->LU_indx);
				}
				if (return_code)
				{
					edit_info->last_picked_node = node;
					if (edit_info->coordinate_field != edit_info->rc_coordinate_field)
					{
						if (Computed_field_evaluate_at_node(edit_info->coordinate_field,
							node, initial_coordinates))
						{
							if (Computed_field_set_values_at_node(
								edit_info->rc_coordinate_field,node,coordinates,
								edit_info->node_manager))
							{
								if (Computed_field_evaluate_at_node(edit_info->coordinate_field,
									node, final_coordinates))
								{
									edit_info->delta1 = final_coordinates[0] - initial_coordinates[0];
									edit_info->delta2 = final_coordinates[1] - initial_coordinates[1];
									edit_info->delta3 = final_coordinates[2] - initial_coordinates[2];
								}
								else
								{
									return_code = 0;
								}
							}
							else
							{
								return_code = 0;
							}
						}
						else
						{
							return_code = 0;
						}
					}
					else
					{
						edit_info->delta1 = coordinates[0] - initial_coordinates[0];
						edit_info->delta2 = coordinates[1] - initial_coordinates[1];
						edit_info->delta3 = coordinates[2] - initial_coordinates[2];
						if (!Computed_field_set_values_at_node(
							edit_info->rc_coordinate_field,node,coordinates,
							edit_info->node_manager))
						{
							return_code=0;
						}
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
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,"FE_node_calculate_delta_position.  Failed");
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
LAST MODIFIED : 29 February 2000

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
		if (node != edit_info->last_picked_node)
		{
			/* the last_picked_node was already updated in
				 FE_node_calculate_delta_position */
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
						if (!Computed_field_set_values_at_node(
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
LAST MODIFIED : 22 February 2000

DESCRIPTION :
Moves the end of the vector to exactly under the mouse, in the plane normal to the view direction at its current depth. Hence, this function should only be
called for a single node.
Note that you must supply an orientation_scale field, while glyph_size[0] and
glyph_centre[0] must be 0.0, and glyph_scale_factors[0] must be non-zero.
==============================================================================*/
{
	double fact;
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
		(0.0 != (scale_factor=edit_info->glyph_scale_factors[0]))&&
		(0.0 < edit_info->near_to_far))
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
				edit_info->last_picked_node = node;
				/* move the end_coordinates to under the mouse, in the plane normal
					 to the view direction */
				/* need ratio of distance from the near plane to <node> over distance
					 between near and far planes, along the view direction: */
				fact=((end_coordinates[0] - edit_info->near1x)*edit_info->viewx+
					(end_coordinates[1] - edit_info->near1y)*edit_info->viewy+
					(end_coordinates[2] - edit_info->near1z)*edit_info->viewz) /
					edit_info->near_to_far;
				/* interpolate near and far displacements to displacement at node */
				end_coordinates[0]=fact*edit_info->far2x+(1.0-fact)*edit_info->near2x;
				end_coordinates[1]=fact*edit_info->far2y+(1.0-fact)*edit_info->near2y;
				end_coordinates[2]=fact*edit_info->far2z+(1.0-fact)*edit_info->near2z;
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
					if (return_code)
					{
						if (!Computed_field_set_values_at_node(
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
							}
							else
							{
								return_code=0;
							}
						}
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
LAST MODIFIED : 22 February 2000

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
		if (node != edit_info->last_picked_node)
		{
			/* the last_picked_node was already updated in
				 FE_node_calculate_delta_vector */
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
					if (!Computed_field_set_values_at_node(
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

static void GNE_scene_input_callback(struct Scene *scene,
	void *graphical_node_editor_void,
	struct Scene_input_callback_data *scene_input_callback_data)
/*******************************************************************************
LAST MODIFIED : 22 February 2000

DESCRIPTION :
Receives mouse button press, motion and release events from <scene>, and
processes them into node movements as necessary.
==============================================================================*/
{
	double d;
	int number_of_selected_nodes,return_code,shift_pressed;
	struct Computed_field *coordinate_field;
	struct FE_node *picked_node;
	struct FE_node_edit_information edit_info;
	struct Graphical_node_editor *node_editor;
	struct GT_element_group *gt_element_group;
	struct GT_element_settings *gt_element_settings;
	struct GT_object *glyph;
	struct LIST(FE_node) *node_list;
	struct Scene_picked_object *scene_picked_object;

	ENTER(GNE_scene_input_callback);
	if (scene&&(node_editor=(struct Graphical_node_editor *)
		graphical_node_editor_void)&&scene_input_callback_data)
	{
		node_list=CREATE(LIST(FE_node))();
		shift_pressed=
			SCENE_INPUT_MODIFY_SHIFT & scene_input_callback_data->input_modifier;
		switch (scene_input_callback_data->input_type)
		{
			case SCENE_BUTTON_PRESS:
			{
#if defined (DEBUG)
				/*???debug */
				printf("GNE: button press!\n");
				/*???debug end */
#endif /* defined (DEBUG) */
				picked_node=Scene_picked_object_list_get_nearest_node(
					scene_input_callback_data->picked_object_list,
					node_editor->node_manager,node_editor->node_group,
					&scene_picked_object,&gt_element_group,&gt_element_settings);
				REACCESS(FE_node)(&(node_editor->last_picked_node),picked_node);
				node_editor->last_picked_node_just_added=0;
				/* need to know whether picked_node is currently selected */
				if (picked_node)
				{
					if (!(node_editor->node_group))
					{
						REACCESS(GROUP(FE_element))(&(node_editor->element_group),
							GT_element_group_get_element_group(gt_element_group));
						REACCESS(GROUP(FE_node))(&(node_editor->node_group),
							GT_element_group_get_node_group(gt_element_group));
					}
					REACCESS(Scene_picked_object)(&(node_editor->scene_picked_object),
						scene_picked_object);
					REACCESS(GT_element_group)(&(node_editor->gt_element_group),
						gt_element_group);
					REACCESS(GT_element_settings)(&(node_editor->gt_element_settings),
						gt_element_settings);
					if (!GT_element_group_is_node_selected(gt_element_group,picked_node))
					{
						ADD_OBJECT_TO_LIST(FE_node)(picked_node,node_list);
						if (shift_pressed)
						{
							GT_element_group_modify_selected_node_list(gt_element_group,
								GT_ELEMENT_GROUP_SELECT_ADD,node_list);
						}
						else
						{
							GT_element_group_modify_selected_node_list(gt_element_group,
								GT_ELEMENT_GROUP_SELECT_REPLACE,node_list);
						}
						node_editor->last_picked_node_just_added=1;
					}
				}
				else
				{
					if (!shift_pressed&&(node_editor->node_group))
					{
						GT_element_group_clear_selected(node_editor->gt_element_group);
						DEACCESS(GROUP(FE_element))(&(node_editor->element_group));
						DEACCESS(GROUP(FE_node))(&(node_editor->node_group));
						DEACCESS(Scene_picked_object)(&(node_editor->scene_picked_object));
						DEACCESS(GT_element_group)(&(node_editor->gt_element_group));
						DEACCESS(GT_element_settings)(&(node_editor->gt_element_settings));
					}
				}
				node_editor->last_nearx=scene_input_callback_data->nearx;
				node_editor->last_neary=scene_input_callback_data->neary;
				node_editor->last_nearz=scene_input_callback_data->nearz;
				node_editor->last_farx=scene_input_callback_data->farx;
				node_editor->last_fary=scene_input_callback_data->fary;
				node_editor->last_farz=scene_input_callback_data->farz;
				node_editor->motion_detected=0;
			} break;
			case SCENE_MOTION_NOTIFY:
			{
				node_editor->motion_detected=1;
#if defined (DEBUG)
				/*???debug */
				/*printf("GNE: motion notify!\n");*/
				/*???debug end */
#endif /* defined (DEBUG) */
			} break;
			case SCENE_BUTTON_RELEASE:
			{
#if defined (DEBUG)
				/*???debug */
				printf("GNE: button release!\n");
				/*???debug end */
#endif /* defined (DEBUG) */
				if (node_editor->last_picked_node)
				{
					return_code=1;
					/* establish edit_info */
					edit_info.last_picked_node = (struct FE_node *)NULL;
					edit_info.delta1=0.0;
					edit_info.delta2=0.0;
					edit_info.delta3=0.0;
					/* view direction must be a unit vector */
					edit_info.viewx=scene_input_callback_data->viewx;
					edit_info.viewy=scene_input_callback_data->viewy;
					edit_info.viewz=scene_input_callback_data->viewz;
					/* calc distance from near to far planes along view direction */
					edit_info.near_to_far=
						scene_input_callback_data->viewx*
						(scene_input_callback_data->farx-scene_input_callback_data->nearx)+
						scene_input_callback_data->viewy*
						(scene_input_callback_data->fary-scene_input_callback_data->neary)+
						scene_input_callback_data->viewz*
						(scene_input_callback_data->farz-scene_input_callback_data->nearz);
					edit_info.near1x=node_editor->last_nearx;
					edit_info.near1y=node_editor->last_neary;
					edit_info.near1z=node_editor->last_nearz;
					edit_info.far1x=node_editor->last_farx;
					edit_info.far1y=node_editor->last_fary;
					edit_info.far1z=node_editor->last_farz;
					edit_info.near2x=scene_input_callback_data->nearx;
					edit_info.near2y=scene_input_callback_data->neary;
					edit_info.near2z=scene_input_callback_data->nearz;
					edit_info.far2x=scene_input_callback_data->farx;
					edit_info.far2y=scene_input_callback_data->fary;
					edit_info.far2z=scene_input_callback_data->farz;
					edit_info.node_manager=node_editor->node_manager;
					/* get coordinate field in RC coordinates */
					if (!(coordinate_field=GT_element_settings_get_coordinate_field(
						node_editor->gt_element_settings)))
					{
						coordinate_field=
							GT_element_group_get_default_coordinate_field(
								node_editor->gt_element_group);
					}
					edit_info.coordinate_field=coordinate_field;
					edit_info.rc_coordinate_field=
						Computed_field_begin_wrap_coordinate_field(coordinate_field);
					edit_info.orientation_scale_field=(struct Computed_field *)NULL;
					edit_info.wrapper_orientation_scale_field=
						(struct Computed_field *)NULL;
					if (GT_element_settings_get_glyph_parameters(
						node_editor->gt_element_settings,&glyph,
						edit_info.glyph_centre,edit_info.glyph_size,
						&edit_info.orientation_scale_field,edit_info.glyph_scale_factors))
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
						node_editor->scene_picked_object,
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
						if (node_editor->motion_detected)
						{
							if (GT_element_group_get_selected_node_list(
								node_editor->gt_element_group,node_list))
							{
								number_of_selected_nodes=NUMBER_IN_LIST(FE_node)(node_list);
								if (1<number_of_selected_nodes)
								{
									/* cache manager so only one change message produced */
									MANAGER_BEGIN_CACHE(FE_node)(node_editor->node_manager);
								}
								/* edit vectors if non-constant orientation_scale field */
								if (edit_info.wrapper_orientation_scale_field&&
									(!(COMPUTED_FIELD_CONSTANT==Computed_field_get_type(
										edit_info.orientation_scale_field))))
								{
									/* edit vector */
									if (FE_node_calculate_delta_vector(
										node_editor->last_picked_node,(void *)&edit_info))
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
									/* edit position */
									if (FE_node_calculate_delta_position(
										node_editor->last_picked_node,(void *)&edit_info))
									{
										if (1<number_of_selected_nodes)
										{
											FOR_EACH_OBJECT_IN_LIST(FE_node)(FE_node_edit_position,
												(void *)&edit_info,node_list);
										}
									}
								}
								if (1<number_of_selected_nodes)
								{
									MANAGER_END_CACHE(FE_node)(node_editor->node_manager);
								}
							}
							else
							{
								return_code=0;
							}
						}
						else
						{
							/* unselect of last_picked_node if not just added */
							if (shift_pressed&&(!(node_editor->last_picked_node_just_added)))
							{
								ADD_OBJECT_TO_LIST(FE_node)(
									node_editor->last_picked_node,node_list);
								GT_element_group_modify_selected_node_list(
									node_editor->gt_element_group,
									GT_ELEMENT_GROUP_SELECT_REMOVE,node_list);
							}
						}
					}
					if (edit_info.orientation_scale_field)
					{
						Computed_field_end_wrap(
							&(edit_info.wrapper_orientation_scale_field));
					}
					Computed_field_end_wrap(&(edit_info.rc_coordinate_field));
				}
				/* deaccess last_picked_node since only kept for one input movement */
				if (node_editor->last_picked_node)
				{
					DEACCESS(FE_node)(&(node_editor->last_picked_node));
				}
#if defined (OLD_CODE)
				/* if active_node_group group is empty, deaccess objects not needed */
				if (!(FIRST_OBJECT_IN_GROUP_THAT(FE_node)(
					(GROUP_CONDITIONAL_FUNCTION(FE_node) *)NULL,(void *)NULL,
					node_editor->active_node_group)))
				{
					if (node_editor->scene_picked_object)
					{
						DEACCESS(Scene_picked_object)(&(node_editor->scene_picked_object));
						DEACCESS(GT_element_group)(&(node_editor->gt_element_group));
						DEACCESS(GT_element_settings)(&(node_editor->gt_element_settings));
					}
				}
#endif /* defined (OLD_CODE) */
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"GNE_scene_input_callback.  Invalid input_type");
			} break;
		}
		DESTROY(LIST(FE_node))(&node_list);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GNE_scene_input_callback.  Invalid argument(s)");
	}
	LEAVE;
} /* GNE_scene_input_callback */

/*
Global functions
----------------
*/

struct Graphical_node_editor *CREATE(Graphical_node_editor)(
	struct Scene *scene,struct MANAGER(FE_node) *node_manager)
/*******************************************************************************
LAST MODIFIED : 21 February 2000

DESCRIPTION :
Creates the structure that needs to be passed to the GNE_scene_input_callback.
==============================================================================*/
{
	struct Graphical_node_editor *graphical_node_editor;
	struct Scene_input_callback input_callback;

	ENTER(CREATE(Graphical_node_editor));
	if (scene&&node_manager)
	{
		if (ALLOCATE(graphical_node_editor,struct Graphical_node_editor,1))
		{
			graphical_node_editor->scene=ACCESS(Scene)(scene);
			graphical_node_editor->node_manager=node_manager;

			input_callback.procedure=GNE_scene_input_callback;
			input_callback.data=(void *)graphical_node_editor;
			Scene_set_input_callback(scene,&input_callback);
			graphical_node_editor->scene_picked_object=
				(struct Scene_picked_object *)NULL;
			graphical_node_editor->last_picked_node=(struct FE_node *)NULL;
			graphical_node_editor->gt_element_group=(struct GT_element_group *)NULL;
			graphical_node_editor->gt_element_settings=
				(struct GT_element_settings *)NULL;
			graphical_node_editor->element_group=(struct GROUP(FE_element) *)NULL;
			graphical_node_editor->node_group=(struct GROUP(FE_node) *)NULL;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Graphical_node_editor).  Not enough memory");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"CREATE(Graphical_node_editor).  Invalid argument(s)");
		graphical_node_editor=(struct Graphical_node_editor *)NULL;
	}
	LEAVE;

	return (graphical_node_editor);
} /* CREATE(Graphical_node_editor) */

int DESTROY(Graphical_node_editor)(
	struct Graphical_node_editor **node_editor_address)
/*******************************************************************************
LAST MODIFIED : 22 February 2000

DESCRIPTION :
Frees and deaccesses objects in the <node_editor> and deallocates the
structure itself.
==============================================================================*/
{
	struct Graphical_node_editor *node_editor;
	int return_code;

	ENTER(DESTROY(Graphical_node_editor));
	if (node_editor_address&&(node_editor= *node_editor_address))
	{
		DEACCESS(Scene)(&(node_editor->scene));
		if (node_editor->last_picked_node)
		{
			DEACCESS(FE_node)(&(node_editor->last_picked_node));
		}
		if (node_editor->scene_picked_object)
		{
			DEACCESS(Scene_picked_object)(&(node_editor->scene_picked_object));
			DEACCESS(GT_element_group)(&(node_editor->gt_element_group));
			DEACCESS(GT_element_settings)(&(node_editor->gt_element_settings));
			if (node_editor->element_group)
			{
				DEACCESS(GROUP(FE_element))(&(node_editor->element_group));
			}
			if (node_editor->node_group)
			{
				DEACCESS(GROUP(FE_node))(&(node_editor->node_group));
			}
		}
		DEALLOCATE(*node_editor_address);
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Graphical_node_editor).  Invalid argument(s)");
		return_code=0;
	}

	return (return_code);
} /* DESTROY(Graphical_node_editor) */
