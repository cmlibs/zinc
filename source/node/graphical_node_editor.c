/*******************************************************************************
FILE : graphical_node_editor.c

LAST MODIFIED : 30 August 1999

DESCRIPTION :
Functions mouse controlled node editing to a Scene.
???DB.  Need a DESTROY
???DB.  Ignore outside clipping planes ?
==============================================================================*/
#include <math.h>
#include "command/command.h"
#include "finite_element/finite_element_to_graphics_object.h"
#include "general/debug.h"
#include "general/geometry.h"
#include "general/matrix_vector.h"
#include "graphics/element_group_settings.h"
#include "graphics/graphical_element.h"
#include "graphics/graphics_object.h"
#include "graphics/scene.h"
#include "node/graphical_node_editor.h"
#include "three_d_drawing/ThreeDDraw.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"

/*
Module constants
----------------
*/
double edit_scale=0.5,pointer_tolerance=0.01;
	/*???DB.  #define ?  setting ? */

/*
Module types
------------
*/
struct Graphical_node_editor
/*******************************************************************************
LAST MODIFIED : 21 July 1999

DESCRIPTION :
Object storing all the parameters for converting scene input messages into
changes in node position and derivatives etc.
==============================================================================*/
{
	struct Scene *scene;
	/*struct Execute_command *execute_command;*/
	struct MANAGER(FE_node) *node_manager;
	/* information about picked nodes the editor knows about */
	struct Scene_picked_object *scene_picked_object;
	struct FE_node *last_picked_node;
	int last_picked_node_just_added;
	int motion_detected;
	struct GROUP(FE_element) *active_element_group;
	struct GROUP(FE_node) *active_data_group,*active_node_group;
	struct GT_element_group *gt_element_group;
	struct GT_element_settings *gt_element_settings;
	double last_farx,last_fary,last_farz,last_nearx,last_neary,last_nearz;
}; /* struct Graphical_node_editor */

struct FE_node_edit_information
/*******************************************************************************
LAST MODIFIED : 14 September 1999

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
	/* whether editing EDIT_POSITION, EDIT_SELECT or EDIT_VECTOR */
	enum Glyph_edit_mode glyph_edit_mode;
	/* the coordinate field to change in the translation */
	struct Computed_field *coordinate_field;
	/* The same field wrapped to get RC coordinates */
	struct Computed_field *rc_coordinate_field;
	/* following required for EDIT_VECTOR only */
	struct Computed_field *wrapper_orientation_scale_field;
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

static int FE_node_calculate_delta(struct FE_node *node,void *edit_info_void)
/*******************************************************************************
LAST MODIFIED : 20 August 1999

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
		(GLYPH_EDIT_POSITION==edit_info->glyph_edit_mode)&&
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
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,"FE_node_calculate_delta.  Failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_calculate_delta.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_calculate_delta */

static int FE_node_edit_position(struct FE_node *node,void *edit_info_void)
/*******************************************************************************
LAST MODIFIED : 20 August 1999

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
		(GLYPH_EDIT_POSITION==edit_info->glyph_edit_mode)&&
		edit_info->node_manager&&edit_info->rc_coordinate_field&&
		(3>=Computed_field_get_number_of_components(
			edit_info->rc_coordinate_field))&&
		(0.0<edit_info->near_to_far))
	{
		return_code=1;
		if (node != edit_info->last_picked_node)
		{
			/* the last_picked_node was already updated in FE_node_calculate_delta */
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
			Computed_field_clear_cache(edit_info->rc_coordinate_field);
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

static int FE_node_edit_vector(struct FE_node *node,void *edit_info_void)
/*******************************************************************************
LAST MODIFIED : 23 July 1999

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

	ENTER(FE_node_edit_vector);
	if (node&&(edit_info=(struct FE_node_edit_information *)edit_info_void)&&
		(GLYPH_EDIT_VECTOR==edit_info->glyph_edit_mode)&&
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
							orientation_scale[0]=a[0];
						} break;
						case 2:
						case 4:
						{
							/* 1 or 2 2-D vectors */
							orientation_scale[0]=a[0];
							orientation_scale[1]=a[1];
						} break;
						case 3:
						case 6:
						case 9:
						{
							/* 1,2 or 3, 3-D vectors */
							orientation_scale[0]=a[0];
							orientation_scale[1]=a[1];
							orientation_scale[2]=a[2];
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
							edit_info->wrapper_orientation_scale_field,node,orientation_scale,
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
		Computed_field_clear_cache(edit_info->wrapper_orientation_scale_field);
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,"FE_node_edit_vector.  Failed");
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

#if defined (OLD_CODE)
struct GT_object_point_picked_data
/*******************************************************************************
LAST MODIFIED : 22 September 1997

DESCRIPTION :
==============================================================================*/
{
	double cursor_x,cursor_y;
	float time,nearest_time,pointer_distance;
	GLdouble	*model_matrix,*projection_matrix,window_x,window_y,window_z;
	GLint *default_viewport;
	struct GT_object *graphics_object;
	struct FE_node **node;
	struct MANAGER(FE_node) *node_manager;
	Triple point;
}; /* struct GT_object_point_picked_data */

static int Scene_object_get_point_picked(
	struct Scene_object *scene_object,void *point_picked_data_void)
/*******************************************************************************
LAST MODIFIED : 19 October 1998

DESCRIPTION :
Returns
==============================================================================*/
{
	float pointer_distance,temp;
	GLdouble window_x,window_y,window_z;
	int found_node,i,return_code;
	struct Scene *scene;
	struct FE_node **node;
	struct GT_object *graphics_object;
	struct GT_object_point_picked_data *point_picked_data;
	struct GT_pointset *point_set;
	Triple *point;

	ENTER(Scene_object_get_point_picked);
	if (scene_object&&(point_picked_data=
		(struct GT_object_point_picked_data *)point_picked_data_void)&&
		point_picked_data->model_matrix&&point_picked_data->projection_matrix&&
		point_picked_data->default_viewport&&point_picked_data->node_manager)
	{
		return_code=1;
		if (g_VISIBLE==Scene_object_get_visibility(scene_object))
		{
			for (graphics_object=Scene_object_get_gt_object(scene_object);
				return_code&&(graphics_object != NULL);
				graphics_object=graphics_object->nextobject)
			{
				if (g_POINTSET==graphics_object->object_type)
				{
					/*???DB.  Skipping "proportions" at present */
					point_picked_data->nearest_time=GT_object_get_nearest_time(
						graphics_object,point_picked_data->time);
					if (point_set=GT_OBJECT_GET_PRIMITIVE_LIST(GT_pointset)(
						graphics_object,point_picked_data->nearest_time))
					{
						point=point_set->pointlist;
						if (node=point_set->nodes)
						{
							for (i=point_set->n_pts;i>0;i--)
							{
#if defined (OPENGL_API)
								if (GL_TRUE==gluProject((*point)[0],(*point)[1],(*point)[2],
									point_picked_data->model_matrix,
									point_picked_data->projection_matrix,
									point_picked_data->default_viewport,
									&window_x,&window_y,&window_z))
								{
									pointer_distance=fabs(point_picked_data->cursor_x-window_x);
									temp=fabs(point_picked_data->cursor_y-window_y);
									if (temp>pointer_distance)
									{
										pointer_distance=temp;
									}
									if ((pointer_distance<pointer_tolerance)&&
										IS_MANAGED(FE_node)(*node,point_picked_data->node_manager))
									{
/*???DB.  Temporary to write out all hits */
printf("%d  %g %g %g\n",(*node)->cm_node_identifier,(*point)[0],(*point)[1],
	(*point)[2]);
										if (!(point_picked_data->graphics_object)||
											(pointer_distance<point_picked_data->pointer_distance))
										{
											point_picked_data->pointer_distance=pointer_distance;
											point_picked_data->graphics_object=graphics_object;
											point_picked_data->point[0]=(*point)[0];
											point_picked_data->point[1]=(*point)[1];
											point_picked_data->point[2]=(*point)[2];
											point_picked_data->window_x=window_x;
											point_picked_data->window_y=window_y;
											point_picked_data->window_z=window_z;
											point_picked_data->node=node;
										}
									}
								}
								point++;
								node++;
#endif /* defined (OPENGL_API) */
							}
						}
					}
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_object_get_point_picked.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_object_get_point_picked */
#endif /* defined (OLD_CODE) */

#if defined (OLD_CODE)
struct Select_node_data
{
	int derivative_number;
	struct FE_node *derivative_node;
	struct GROUP(FE_node) *active_node_group;
	struct LIST(FE_node) *nodes_for_this_pick;
	struct MANAGER(FE_node) *node_manager;
}; /* Select_node_data */

static int Scene_picked_object_select_node(
	struct Scene_picked_object *picked_object,void *select_node_data_void)
/*******************************************************************************
LAST MODIFIED : 18 May 1998

DESCRIPTION :
Adds picked nodes to the active_node_group group, or removes them if they are
already in there.
==============================================================================*/
{
	int derivative_number,node_number,number_of_subobjects,return_code;
	struct FE_node *picked_node;
	struct Select_node_data *select_node_data;

	ENTER(Scene_picked_object_select_node);
	if (picked_object&&(picked_object->graphics_object)&&
		(select_node_data=(struct Select_node_data *)select_node_data_void))
	{
		return_code=1;
		if ((g_POINTSET==picked_object->graphics_object->object_type)&&
			(1<=(number_of_subobjects=picked_object->number_of_subobjects))&&
			(-1!=(node_number= *(picked_object->subobjects))))
		{
			if (picked_node=FIND_BY_IDENTIFIER_IN_MANAGER(FE_node,cm_node_identifier)(
				node_number,select_node_data->node_manager))
			{
				if (1==number_of_subobjects)
				{
#if defined (DEBUG)
					/*???debug */
					printf("derivative_node %p\n",select_node_data->derivative_node);
					/*???debug end */
#endif /* defined (DEBUG) */
					if (!(select_node_data->derivative_node))
					{
						if (!IS_OBJECT_IN_LIST(FE_node)(picked_node,
							select_node_data->nodes_for_this_pick))
						{
							if (IS_OBJECT_IN_GROUP(FE_node)(picked_node,
								select_node_data->active_node_group))
							{
#if defined (DEBUG)
								/*???debug */
								printf("Removing node %d from selection\n",node_number);
								/*???debug end */
#endif /* defined (DEBUG) */
								return_code=REMOVE_OBJECT_FROM_GROUP(FE_node)(picked_node,
									select_node_data->active_node_group);
							}
							else
							{
#if defined (DEBUG)
								/*???debug */
								printf("Adding node %d to selection\n",node_number);
								/*???debug end */
#endif /* defined (DEBUG) */
								return_code=ADD_OBJECT_TO_GROUP(FE_node)(picked_node,
									select_node_data->active_node_group);
							}
							ADD_OBJECT_TO_LIST(FE_node)(picked_node,
								select_node_data->nodes_for_this_pick);
						}
					}
				}
				else
				{
					if ((2==number_of_subobjects)&&(1<=(derivative_number=
						(picked_object->subobjects)[1]))&&(derivative_number<=3))
					{
						if (!(select_node_data->derivative_node))
						{
							REMOVE_ALL_OBJECTS_FROM_GROUP(FE_node)(
								select_node_data->active_node_group);
							select_node_data->derivative_node=ACCESS(FE_node)(picked_node);
							select_node_data->derivative_number=derivative_number;
#if defined (DEBUG)
							/*???debug */
							printf("derivative_node %d %d\n",node_number,derivative_number);
							/*???debug end */
#endif /* defined (DEBUG) */
						}
					}
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Scene_picked_object_select_node.  Node number not in manager");
				return_code=0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_picked_object_select_node.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_picked_object_select_node */
#endif /* defined (OLD_CODE) */

struct Scene_picked_object_get_nearest_picked_node_data
{
	struct FE_node *picked_node;
	struct Scene_picked_object *scene_picked_object;
	struct GT_element_group *gt_element_group;
	struct GT_element_settings *gt_element_settings;
	struct MANAGER(FE_node) *node_manager;
	/* "nearest" value from Scene_picked_object for picked_node */
	unsigned int nearest;
};

static int Scene_picked_object_get_nearest_picked_node(
	struct Scene_picked_object *scene_picked_object,void *picked_node_data_void)
/*******************************************************************************
LAST MODIFIED : 19 August 1999

DESCRIPTION :
If the <scene_picked_object> refers to a node, the "nearest" value is compared
with that for the currently picked_node in the <picked_node_data>. I there was
no currently picked_node or the new node is nearer, it becomes the picked node
and its "nearest" value is stored in the picked_node_data.
==============================================================================*/
{
	int node_number,return_code;
	struct FE_node *picked_node;
	struct Scene_object *scene_object;
	struct Scene_picked_object_get_nearest_picked_node_data *picked_node_data;
	struct GT_element_group *gt_element_group;
	struct GT_element_settings *gt_element_settings;

	ENTER(Scene_picked_object_get_nearest_picked_node);
	if (scene_picked_object&&
		(picked_node_data=(struct Scene_picked_object_get_nearest_picked_node_data
			*)picked_node_data_void))
	{
		return_code=1;
		/* proceed only if there is no picked_node or object is nearer */
		if (((struct FE_node *)NULL==picked_node_data->picked_node)||
			(Scene_picked_object_get_nearest(scene_picked_object) <
				picked_node_data->nearest))
		{
			/* is the last scene_object a Graphical_element wrapper, and does the
				 settings for the graphic refer to node_glyphs? */
			if ((scene_object=Scene_picked_object_get_Scene_object(
				scene_picked_object,
				Scene_picked_object_get_number_of_scene_objects(scene_picked_object)-1))
				&&(SCENE_OBJECT_GRAPHICAL_ELEMENT_GROUP==
					Scene_object_get_type(scene_object))&&(gt_element_group=
						Scene_object_get_graphical_element_group(scene_object))&&
				(3==Scene_picked_object_get_number_of_subobjects(scene_picked_object))&&
				(gt_element_settings=get_settings_at_position_in_GT_element_group(
					gt_element_group,
					Scene_picked_object_get_subobject(scene_picked_object,0)))&&
				(GT_ELEMENT_SETTINGS_NODE_POINTS==
					GT_element_settings_get_settings_type(gt_element_settings)))
			{
				node_number=Scene_picked_object_get_subobject(scene_picked_object,2);
				if (picked_node=FIND_BY_IDENTIFIER_IN_MANAGER(FE_node,
					cm_node_identifier)(node_number,picked_node_data->node_manager))
				{
					picked_node_data->picked_node=picked_node;
					picked_node_data->scene_picked_object=scene_picked_object;
					picked_node_data->gt_element_group=gt_element_group;
					picked_node_data->gt_element_settings=gt_element_settings;
					picked_node_data->nearest=
						Scene_picked_object_get_nearest(scene_picked_object);
				}
				else
				{
					display_message(WARNING_MESSAGE,
						"Scene_picked_object_get_nearest_picked_node.  "
						"Node number %d not in manager",node_number);
					/*return_code=0;*/
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_picked_object_get_nearest_picked_node.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_picked_object_get_nearest_picked_node */

static void GNE_scene_input_callback(struct Scene *scene,
	void *graphical_node_editor_void,
	struct Scene_input_callback_data *scene_input_callback_data)
/*******************************************************************************
LAST MODIFIED : 23 July 1999

DESCRIPTION :
Receives mouse button press, motion and release events from <scene>, and
processes them into node movements as necessary.
==============================================================================*/
{
	double d;
	enum Glyph_edit_mode glyph_edit_mode,glyph_edit_mode2;
	int node_is_active,number_of_active_nodes,return_code,shift_pressed;
	struct Computed_field *coordinate_field,*orientation_scale_field;
	struct FE_node_edit_information edit_info;
	struct Graphical_node_editor *node_editor;
	struct GT_object *glyph;
	struct Scene_picked_object_get_nearest_picked_node_data picking_data;

	ENTER(GNE_scene_input_callback);
	if (scene&&(node_editor=(struct Graphical_node_editor *)
		graphical_node_editor_void)&&scene_input_callback_data)
	{
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
				picking_data.picked_node=(struct FE_node *)NULL;
				picking_data.scene_picked_object=(struct Scene_picked_object *)NULL;
				picking_data.gt_element_group=(struct GT_element_group *)NULL;
				picking_data.gt_element_settings=(struct GT_element_settings *)NULL;
				picking_data.nearest=0;
				picking_data.node_manager=node_editor->node_manager;
				FOR_EACH_OBJECT_IN_LIST(Scene_picked_object)(
					Scene_picked_object_get_nearest_picked_node,(void *)&picking_data,
					scene_input_callback_data->picked_object_list);
				/* need to know whether picked node is in the active_nodes_group */
				if (picking_data.picked_node)
				{
					node_is_active=((struct FE_node *)NULL !=
						FIND_BY_IDENTIFIER_IN_GROUP(FE_node,cm_node_identifier)(
							get_FE_node_cm_node_identifier(picking_data.picked_node),
							node_editor->active_node_group));
				}
				number_of_active_nodes=
					NUMBER_IN_GROUP(FE_node)(node_editor->active_node_group);
				/* if the node_editor has a scene_picked_object, then it will have
					 nodes in the selected group */
				if (node_editor->scene_picked_object)
				{
					/* if picked_node is not compatible with existing selected nodes,
						 ignore it with a warning - if shift is not pressed then new node
						 will replace active group, so ignore check */
					if (picking_data.picked_node&&shift_pressed&&(
						(!Scene_picked_objects_have_same_transformation(
							picking_data.scene_picked_object,
							node_editor->scene_picked_object))||
						(GT_element_settings_get_coordinate_field(
							node_editor->gt_element_settings) !=
							GT_element_settings_get_coordinate_field(
								picking_data.gt_element_settings))||
						(GT_element_settings_get_glyph_edit_mode(
							node_editor->gt_element_settings,&glyph_edit_mode)&&
							GT_element_settings_get_glyph_edit_mode(
								picking_data.gt_element_settings,&glyph_edit_mode2)&&
							(glyph_edit_mode != glyph_edit_mode2))||
						((GLYPH_EDIT_VECTOR==glyph_edit_mode)&&(!node_is_active))))
					{
						if ((GLYPH_EDIT_VECTOR==glyph_edit_mode)&&
							(GLYPH_EDIT_VECTOR==glyph_edit_mode2))
						{
							display_message(WARNING_MESSAGE,
								"Cannot edit vectors for more than one node");
						}
						else
						{
							display_message(WARNING_MESSAGE,
								"Node %d does not match current selection - ignoring",
								get_FE_node_cm_node_identifier(picking_data.picked_node));
						}
						picking_data.picked_node=(struct FE_node *)NULL;
						picking_data.scene_picked_object=(struct Scene_picked_object *)NULL;
						picking_data.gt_element_group=(struct GT_element_group *)NULL;
						picking_data.gt_element_settings=(struct GT_element_settings *)NULL;
					}
				}
				REACCESS(FE_node)(&(node_editor->last_picked_node),
					picking_data.picked_node);
				node_editor->last_picked_node_just_added=0;
				if (picking_data.picked_node)
				{
					REACCESS(Scene_picked_object)(&(node_editor->scene_picked_object),
						picking_data.scene_picked_object);
					REACCESS(GT_element_group)(&(node_editor->gt_element_group),
						picking_data.gt_element_group);
					REACCESS(GT_element_settings)(&(node_editor->gt_element_settings),
						picking_data.gt_element_settings);
				}
				/*if ((picking_data.picked_node&&(!node_is_active))||
					((0<number_of_active_nodes)&&(!shift_pressed)&&
						(!((1==number_of_active_nodes)&&picking_data.picked_node&&
						node_is_active))))*/
				if ((picking_data.picked_node&&(!node_is_active))||
					((!shift_pressed)&&(0<number_of_active_nodes)&&
						!(picking_data.picked_node&&node_is_active)))
				{
					MANAGED_GROUP_BEGIN_CACHE(FE_node)(node_editor->active_node_group);
					if ((0<number_of_active_nodes)&&(!shift_pressed))
					{
						REMOVE_ALL_OBJECTS_FROM_GROUP(FE_node)(
							node_editor->active_node_group);
						node_is_active=0;
					}
					if (picking_data.picked_node&&(!node_is_active))
					{
						if (ADD_OBJECT_TO_GROUP(FE_node)(node_editor->last_picked_node,
							node_editor->active_node_group))
						{
							node_editor->last_picked_node_just_added=1;
						}
					}
					MANAGED_GROUP_END_CACHE(FE_node)(node_editor->active_node_group);
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
					if (GT_element_settings_get_glyph_parameters(
						node_editor->gt_element_settings,&glyph,
						edit_info.glyph_centre,edit_info.glyph_size,
						&orientation_scale_field,edit_info.glyph_scale_factors))
					{
						if (orientation_scale_field)
						{
							edit_info.wrapper_orientation_scale_field=
								Computed_field_begin_wrap_orientation_scale_field(
									orientation_scale_field,edit_info.rc_coordinate_field);
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
					GT_element_settings_get_glyph_edit_mode(
						node_editor->gt_element_settings,&glyph_edit_mode);
					edit_info.glyph_edit_mode=glyph_edit_mode;
					if (return_code)
					{
						switch (glyph_edit_mode)
						{
							case GLYPH_EDIT_POSITION:
							{
								if (node_editor->motion_detected)
								{
									if (FE_node_calculate_delta(node_editor->last_picked_node,
										(void *)&edit_info))
									{
										if (1<NUMBER_IN_GROUP(FE_node)(
											node_editor->active_node_group))
										{
											/* the FE_node_calculate_delta already shifted this node */
											edit_info.last_picked_node = node_editor->last_picked_node;
											/* cache manager so only one change message produced */
											MANAGER_BEGIN_CACHE(FE_node)(node_editor->node_manager);
											FOR_EACH_OBJECT_IN_GROUP(FE_node)(FE_node_edit_position,
												(void *)&edit_info,node_editor->active_node_group);
											MANAGER_END_CACHE(FE_node)(node_editor->node_manager);
										}
									}
								}
								else
								{
									/* toggle last_picked_node in active_node_group group */
									if (shift_pressed&&
										(!(node_editor->last_picked_node_just_added)))
									{
										REMOVE_OBJECT_FROM_GROUP(FE_node)(
											node_editor->last_picked_node,
											node_editor->active_node_group);
									}
								}
							} break;
							case GLYPH_EDIT_SELECT:
							{
								/* toggle last_picked_node in active_node_group group */
								if (shift_pressed&&
									(!(node_editor->last_picked_node_just_added)))
								{
									REMOVE_OBJECT_FROM_GROUP(FE_node)(
										node_editor->last_picked_node,
										node_editor->active_node_group);
								}
							} break;
							case GLYPH_EDIT_VECTOR:
							{
								/*if (edit_info->wrapper_orientation_scale_field&&
									(0.0 != edit_info.glyph_scale_factors[0]))
									{*/
								FE_node_edit_vector(node_editor->last_picked_node,
									(void *)&edit_info);
								/*}
									else
									{
									display_message(ERROR_MESSAGE,
									"Must have and orientation field and non-zero scale factor "
									"to edit vector");
									}*/
							} break;
							default:
							{
								display_message(ERROR_MESSAGE,
									"GNE_scene_input_callback.  Unknown glyph edit mode");
							} break;
						}
					}
					if (orientation_scale_field)
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


#if defined (OLD_CODE)
				if (node=node_editor->derivative_node)
				{
					double fact;
					enum FE_nodal_value_type derivative_type;
					FE_value deriv_x,deriv_y,deriv_z,node_x,node_y,node_z;
					int return_code;
					struct FE_field_component field_component;
					struct FE_node *copy_node;

#if defined (DEBUG)
					/*???debug */
					printf("derivative_node\n");
					/*???debug end */
#endif /* defined (DEBUG) */
					if ((edit_info.node_manager)&&(0.0<edit_info.near_to_far))
					{
#if defined (DEBUG)
						/*???debug */
						printf("arguments OK\n");
						/*???debug end */
#endif /* defined (DEBUG) */
						if (field_component.field=FE_node_get_position_cartesian(node,
							(struct FE_field *)NULL,&node_x,&node_y,&node_z,(FE_value *)NULL))
						{
							if (RECTANGULAR_CARTESIAN==get_coordinate_system_type(
								get_FE_field_coordinate_system(field_component.field)))
							{
								field_component.number=0;
								switch (node_editor->derivative_number)
								{
									case 1:
									{
										derivative_type=FE_NODAL_D_DS1;
										return_code=1;
									} break;
									case 2:
									{
										derivative_type=FE_NODAL_D_DS2;
										return_code=1;
									} break;
									case 3:
									{
										derivative_type=FE_NODAL_D_DS3;
										return_code=1;
									} break;
									default:
									{
										return_code=0;
									} break;
								}
#if defined (DEBUG)
								/*???debug */
								printf("derivative_type %d\n",return_code);
								/*???debug end */
#endif /* defined (DEBUG) */
								if (return_code&&(return_code=get_FE_nodal_FE_value_value(node,
									&field_component,0,derivative_type,&deriv_x)))
								{
									(field_component.number)++;
									if (return_code=get_FE_nodal_FE_value_value(node,&field_component,0,
										derivative_type,&deriv_y))
									{
										(field_component.number)++;
										return_code=get_FE_nodal_FE_value_value(node,&field_component,0,
											derivative_type,&deriv_z);
									}
								}
#if defined (DEBUG)
								/*???debug */
								printf("%d %g %g %g\n",return_code,node_x,node_y,node_z);
								printf("  %g %g %g\n",deriv_x,deriv_y,deriv_z);
								/*???debug end */
#endif /* defined (DEBUG) */
								if (return_code)
								{
									/* translate the node */
									/* need the ratio of the distance from the near plane to
										<node> over the distance between the near and far planes,
										along the view direction: */
									fact=((node_x - edit_info.near1x)*edit_info.viewx+
										(node_y - edit_info.near1y)*edit_info.viewy+
										(node_z - edit_info.near1z)*edit_info.viewz) /
										edit_info.near_to_far;
									/* interpolate near and far displacements to displacement at
										node */
									deriv_x += fact*(edit_info.far2x-edit_info.far1x) +
										(1.0-fact)*(edit_info.near2x-edit_info.near1x);
									deriv_y += fact*(edit_info.far2y-edit_info.far1y) +
										(1.0-fact)*(edit_info.near2y-edit_info.near1y);
									deriv_z += fact*(edit_info.far2z-edit_info.far1z) +
										(1.0-fact)*(edit_info.near2z-edit_info.near1z);
									/* create a copy of the node: */
									field_component.number=0;
									if ((copy_node=CREATE(FE_node)(0,(struct FE_node *)NULL))
										&&COPY(FE_node)(copy_node,node))
									{
										set_FE_nodal_FE_value_value(copy_node,&field_component,0,
											derivative_type,deriv_x);
										(field_component.number)++;
										set_FE_nodal_FE_value_value(copy_node,&field_component,0,
											derivative_type,deriv_y);
										(field_component.number)++;
										set_FE_nodal_FE_value_value(copy_node,&field_component,0,
											derivative_type,deriv_z);
										MANAGER_MODIFY_NOT_IDENTIFIER(FE_node,cm_node_identifier)(
											node,copy_node,edit_info.node_manager);
									}
									else
									{
										display_message(ERROR_MESSAGE,
											"GNE_scene_input_callback.  Could not make copy of node");
										return_code=0;
									}
									DESTROY(FE_node)(&copy_node);
								}
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
						"GNE_scene_input_callback.  Could not calculate coordinate field");
							return_code=0;
						}
					}
					DEACCESS(FE_node)(&(node_editor->derivative_node));
					node_editor->derivative_node=(struct FE_node *)NULL;
					node_editor->derivative_number=0;
				}
				else
				{
					/*???RC should cache manager if more than one node modified */
					FOR_EACH_OBJECT_IN_GROUP(FE_node)(FE_node_edit_position,
						(void *)&edit_info,node_editor->active_node_group);
				}
#endif /* defined (OLD_CODE) */
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"GNE_scene_input_callback.  Invalid input_type");
			} break;
		}
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
	struct Scene *scene,struct GROUP(FE_element) *active_element_group,
	struct GROUP(FE_node) *active_data_group,
	struct GROUP(FE_node) *active_node_group,
	struct MANAGER(FE_node) *node_manager)
/*******************************************************************************
LAST MODIFIED : 20 July 1999

DESCRIPTION :
Creates the structure that needs to be passed to the GNE_scene_input_callback.
==============================================================================*/
{
	struct Graphical_node_editor *graphical_node_editor;
	struct Scene_input_callback input_callback;

	ENTER(CREATE(Graphical_node_editor));
	if (scene&&active_element_group&&active_data_group&&active_node_group&&
		node_manager)
	{
		if (ALLOCATE(graphical_node_editor,struct Graphical_node_editor,1))
		{
			graphical_node_editor->scene=ACCESS(Scene)(scene);
			graphical_node_editor->active_element_group=
				ACCESS(GROUP(FE_element))(active_element_group);
			graphical_node_editor->active_data_group=
				ACCESS(GROUP(FE_node))(active_data_group);
			graphical_node_editor->active_node_group=
				ACCESS(GROUP(FE_node))(active_node_group);
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
LAST MODIFIED : 20 July 1999

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
		DEACCESS(GROUP(FE_element))(&(node_editor->active_element_group));
		DEACCESS(GROUP(FE_node))(&(node_editor->active_data_group));
		DEACCESS(GROUP(FE_node))(&(node_editor->active_node_group));
		if (node_editor->last_picked_node)
		{
			DEACCESS(FE_node)(&(node_editor->last_picked_node));
		}
		if (node_editor->scene_picked_object)
		{
			DEACCESS(Scene_picked_object)(&(node_editor->scene_picked_object));
			DEACCESS(GT_element_group)(&(node_editor->gt_element_group));
			DEACCESS(GT_element_settings)(&(node_editor->gt_element_settings));
		}
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
