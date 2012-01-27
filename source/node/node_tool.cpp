/*******************************************************************************
FILE : node_tool.c

LAST MODIFIED : 28 October 2004

Functions for mouse controlled node position and vector editing based on
Scene input.
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

extern "C" {
#include <math.h>
#include "api/cmiss_rendition.h"
#include "time/time_keeper.h"
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_composite.h"
#include "computed_field/computed_field_finite_element.h"
#include "computed_field/computed_field_group.h"
#include "computed_field/computed_field_set.h"
#include "computed_field/computed_field_wrappers.h"
#include "api/cmiss_field_subobject_group.h"
#include "general/debug.h"
#include "general/matrix_vector.h"
#include "general/mystring.h"
#include "graphics/graphics_object.h"
#include "interaction/interaction_graphics.h"
#include "interaction/interaction_volume.h"
#include "interaction/interactive_event.h"
#include "node/node_operations.h"
#include "node/node_tool.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_region.h"
#include "graphics/rendition.h"
#include "graphics/graphic.h"
#include "graphics/scene.h"
#include "region/cmiss_region.h"
#include "user_interface/message.h"
}
#if defined (WX_USER_INTERFACE)
#include "wx/wx.h"
#include <wx/tglbtn.h>
#include "wx/xrc/xmlres.h"
#include "choose/choose_manager_class.hpp"
#include "graphics/graphics_window_private.hpp"
#include "node/node_tool.xrch"
#include "region/cmiss_region_chooser_wx.hpp"
#endif /* defined (WX_USER_INTERFACE)*/
#include <map>
typedef std::multimap<Cmiss_region *, Cmiss_node_id> Region_node_map;

/*
Module variables
----------------
*/

static char Interactive_tool_node_type_string[] = "node_tool";

/*
Module types
------------
*/

#if defined (WX_USER_INTERFACE)
class wxNodeTool;
#endif /* defined (WX_USER_INTERFACE) */


static int Node_tool_set_region(struct Node_tool *node_tool,
	struct Cmiss_region *region, Cmiss_field_group_id group);

struct Node_tool
/*******************************************************************************
LAST MODIFIED : 17 May 2003

DESCRIPTION :
Object storing all the parameters for converting scene input messages into
changes in node position and derivatives etc.
==============================================================================*/
{
	struct Execute_command *execute_command;
	struct MANAGER(Interactive_tool) *interactive_tool_manager;
	struct Interactive_tool *interactive_tool;
	/* flag indicating that the above manager is actually the data manager */
	int use_data;
	/* The root region */
	struct Cmiss_region *root_region;
	/* The region we are working in */
	struct Cmiss_region *region;
	Cmiss_field_group_id group_field;
	struct FE_region *fe_region;
	/* needed for destroy button */
	struct MANAGER(FE_element) *element_manager;
	struct Graphical_material *rubber_band_material;
	struct Cmiss_time_keeper *time_keeper;
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
	/* if create is enabled this option will force the nodes to be created on a surface
		rather than between near and far */
	int constrain_to_surface;
	enum Node_tool_edit_mode edit_mode;
	struct Computed_field *coordinate_field, *command_field, *element_xi_field;
	/* information about picked nodes the editor knows about */
	struct Scene_picked_object *scene_picked_object;
	struct FE_node *last_picked_node;
	int picked_node_was_unselected;
	int motion_detected;

	struct Cmiss_rendition *rendition;
	struct Cmiss_graphic *graphic;
	void *computed_field_manager_callback_id;
	struct Interaction_volume *last_interaction_volume;
	struct GT_object *rubber_band;
	struct FE_field *FE_coordinate_field;
	/* maintain a template node for creating new nodes */
	/* the dimension of the elements being created - user settable */
	int element_dimension;
	/* maintain template element for creating new elements */
	struct FE_element *template_element;
	/* indicates whether elements are created in response to node selections */
	int element_create_enabled;
	/* the element being created */
	struct FE_element *element;
	/* number of nodes that have been set in the element being created */
	int number_of_clicked_nodes;
	struct MANAGER(Computed_field) *computed_field_manager;
#if defined (WX_USER_INTERFACE)
	wxNodeTool *wx_node_tool;
	 wxPoint tool_position;
#endif /* defined (WX_USER_INTERFACE) */
}; /* struct Node_tool */

struct FE_node_edit_information
/*******************************************************************************
LAST MODIFIED : 19 February 2008

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
	/* editing nodes in this region */
	struct FE_region *fe_region;
	/* information for undoing scene object transformations - only needs to be
		 done if transformation_required is set */
	int transformation_required,LU_indx[4];
	double transformation_matrix[16],LU_transformation_matrix[16];
	/* The last_picked node is used to calculate the delta change and so when
		the whole active group is looped over this node is ignored */
	struct FE_node *last_picked_node;
	/* Fields that allow constrain_to_surface to work correctly,
		only the last picked node will be updated as this is the only one
		we know the element for */
	int constrain_to_surface;
	struct FE_element *nearest_element;
	struct Computed_field *nearest_element_coordinate_field;
	struct Computed_field *element_xi_field;
}; /* struct FE_node_edit_information */

struct Node_tool_element_constraint_function_data
{
	struct FE_element *element, *found_element;
	FE_value xi[MAXIMUM_ELEMENT_XI_DIMENSIONS];
	struct Computed_field *coordinate_field;
}; /* struct Node_tool_element_constraint_function_data */

/*
Module functions
----------------
*/

/* Prototype */
static int Node_tool_end_element_creation(
	 struct Node_tool *node_tool);
int Node_tool_set_element_create_enabled(struct Node_tool  *node_tool,
	 int create_enabled);
#if defined (WX_USER_INTERFACE)
int Node_tool_set_element_dimension(
	 struct Node_tool *node_tool,int element_dimension);
#endif /*defined (WX_USER_INTERFACE)*/
#if defined (WX_USER_INTERFACE)
static int Node_tool_refresh_element_dimension_text(
	 struct Node_tool *node_tool);
#endif /*defined (WX_USER_INTERFACE)*/

static int Cmiss_field_group_destroy_all_nodes(Cmiss_field_group_id group, void *use_data_address_void)
{
	int return_code = 0;
	int *use_data_address = (int *)use_data_address_void;
	if (group && use_data_address)
	{
		Cmiss_field_module_id field_module = Cmiss_field_get_field_module(Cmiss_field_group_base_cast(group));
		Cmiss_nodeset_id master_nodeset = Cmiss_field_module_find_nodeset_by_name(
			field_module, (*use_data_address) ? "cmiss_data" : "cmiss_nodes");
		Cmiss_field_node_group_id node_group = Cmiss_field_group_get_node_group(group, master_nodeset);
		Cmiss_nodeset_destroy(&master_nodeset);
		if (node_group)
		{
			Cmiss_nodeset_group_id nodeset_group = Cmiss_field_node_group_get_nodeset(node_group);
			Cmiss_nodeset_destroy_all_nodes(Cmiss_nodeset_group_base_cast(nodeset_group));
			Cmiss_nodeset_group_destroy(&nodeset_group);
			Cmiss_field_node_group_destroy(&node_group);
		}
		Cmiss_field_module_destroy(&field_module);
		return_code = 1;
	}
	return return_code;
}

int Node_tool_destroy_selected_nodes(struct Node_tool *node_tool)
{
	int return_code = 0;
	if (node_tool->region)
	{
		return_code = 1;
		Cmiss_rendition *root_rendition = Cmiss_region_get_rendition_internal(
			node_tool->root_region);
		Cmiss_field_group_id selection_group = Cmiss_rendition_get_selection_group(root_rendition);
		if (selection_group)
		{
			Cmiss_field_group_for_each_group_hierarchical(selection_group,
				Cmiss_field_group_destroy_all_nodes, (void *)&(node_tool->use_data));
			if (node_tool->use_data)
			{
				Cmiss_field_group_clear_region_tree_data(selection_group);
			}
			else
			{
				Cmiss_field_group_clear_region_tree_node(selection_group);
			}
			Cmiss_field_group_destroy(&selection_group);
		}
		Cmiss_rendition_flush_tree_selections(root_rendition);
		Cmiss_rendition_destroy(&root_rendition);
	}
	return return_code;
}

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
			Computed_field_get_defining_FE_field_list(node_tool->coordinate_field)))
		{
			if ((1==NUMBER_IN_LIST(FE_field)(fe_field_list))&&
				(fe_field=FIRST_OBJECT_IN_LIST_THAT(FE_field)(
				(LIST_CONDITIONAL_FUNCTION(FE_field) *)NULL,(void *)NULL,
				fe_field_list)) && (3 >= get_FE_field_number_of_components(
				fe_field)) && (FE_VALUE_VALUE == get_FE_field_value_type(fe_field)))
			{
				node_field_creator = CREATE(FE_node_field_creator)(/*number_of_components*/3);
				if (node_field_creator != 0)
				{
					if (define_FE_field_at_node(node,fe_field,
						(struct FE_time_sequence *)NULL,
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

static int FE_node_define_and_set_element_xi(struct FE_node *node,
		struct Computed_field *element_xi_field,
		struct FE_element *element, FE_value *xi)
/*******************************************************************************
LAST MODIFIED : 19 February 2008

DESCRIPTION :
Defines element_xi_field at node if not already defined. Sets value.
==============================================================================*/
{
	int return_code;
	struct FE_field *fe_field;
	struct FE_node_field_creator *node_field_creator;

	ENTER(FE_node_define_and_set_element_xi);
	return_code = 0;
	if (node && element_xi_field && element && xi)
	{
		fe_field = (struct FE_field *)NULL;
		if (Computed_field_get_type_finite_element(element_xi_field, &fe_field))
		{
			if (!FE_field_is_defined_at_node(fe_field,node))
			{
				node_field_creator =
					CREATE(FE_node_field_creator)(/*number_of_components*/1);
				if (node_field_creator != 0)
				{
					define_FE_field_at_node(node, fe_field,
						(struct FE_time_sequence *)NULL, node_field_creator);
					DESTROY(FE_node_field_creator)(&node_field_creator);
				}
			}
			return_code = set_FE_nodal_element_xi_value(node,fe_field,/*component_number*/0,
				/*version*/0,FE_NODAL_VALUE,element,xi);
		}
		if (!return_code)
		{
			display_message(ERROR_MESSAGE,
				"FE_node_define_and_set_element_xi.  Failed");
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"FE_node_define_and_set_element_xi.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* FE_node_define_and_set_element_xi */

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
			model_coordinates)&&(0.0 != (h=model_coordinates[3],/*singular_tolerance*/1.0e-12)))
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

static int Node_tool_element_constraint_function(FE_value *point,
	void *void_data)
/*******************************************************************************
LAST MODIFIED : 14 February 2008

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Node_tool_element_constraint_function_data *data;

	ENTER(Node_tool_element_constraint_function_data);
	if (point && (data = (struct Node_tool_element_constraint_function_data *)void_data))
	{
		data->found_element = data->element;
		return_code = Computed_field_find_element_xi(data->coordinate_field,
			point, /*number_of_values*/3, /*time*/0, &(data->found_element), 
			data->xi, (Cmiss_mesh_id)0, /*propagate_field*/0, /*find_nearest_location*/1);
		Computed_field_evaluate_in_element(data->coordinate_field,
			data->found_element, data->xi, /*time*/0.0, (struct FE_element *)NULL,
			point, (FE_value *)NULL);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_element_constraint_function.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_tool_element_constraint_function */

/***************************************************************************//**
 * Calculates the delta change in the coordinates due to the ray supplied in the
 * <edit_info>.  This change is set inside the <edit_info> and this can then be
 * applied to multiple nodes.
 */
static int FE_node_calculate_delta_position(struct FE_node *node,
	struct FE_node_edit_information *edit_info)
{
	double model_coordinates[3],normalised_coordinates[3],placement_coordinates[3];
	FE_value coordinates[3], initial_coordinates[3], final_coordinates[3];
	int i, return_code;
	struct Node_tool_element_constraint_function_data constraint_data;

	ENTER(FE_node_calculate_delta_position);
	if (node && edit_info &&
		edit_info->fe_region&&edit_info->rc_coordinate_field&&
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
				if (edit_info->constrain_to_surface && edit_info->nearest_element &&
					 edit_info->nearest_element_coordinate_field)
				{
					constraint_data.element = edit_info->nearest_element;
					constraint_data.found_element = edit_info->nearest_element;
					constraint_data.coordinate_field = edit_info->nearest_element_coordinate_field;
					for (i = 0; i < MAXIMUM_ELEMENT_XI_DIMENSIONS; i++)
					{
						constraint_data.xi[i] = 0.5;
					}
					Interaction_volume_get_placement_point(edit_info->final_interaction_volume,
						placement_coordinates, Node_tool_element_constraint_function,
						&constraint_data);
					coordinates[0] = placement_coordinates[0];
					coordinates[1] = placement_coordinates[1];
					coordinates[2] = placement_coordinates[2];		
				}
				else
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
						Computed_field_set_values_at_node(
							edit_info->rc_coordinate_field,node,edit_info->time,
							coordinates)&&
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
					return_code=Computed_field_set_values_at_node(
						edit_info->rc_coordinate_field,node,edit_info->time,
						coordinates);
				}
				/* may be some application for not editing element_xi field value */
				if (return_code && edit_info->nearest_element &&
					constraint_data.found_element && edit_info->element_xi_field)
				{
					return_code = FE_node_define_and_set_element_xi(node,
						edit_info->element_xi_field, constraint_data.found_element,
						constraint_data.xi);
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

/***************************************************************************//**
 * Translates the <rc_coordinate_field> of <node> according to the delta change
 * stored in the <edit_info>.
 */
static int FE_node_edit_position(struct FE_node *node,
	struct FE_node_edit_information *edit_info)
{
	FE_value coordinates[3];
	int return_code;

	ENTER(FE_node_edit_position);
	if (node && edit_info &&
		edit_info->fe_region&&edit_info->rc_coordinate_field&&
		(3>=Computed_field_get_number_of_components(
			edit_info->rc_coordinate_field)))
	{
		return_code=1;
		/* the last_picked_node was edited in FE_node_calculate_delta_position.
			 Also, don't edit unless in node_group, if supplied */
		if ((node != edit_info->last_picked_node)&&(
			FE_region_contains_FE_node(edit_info->fe_region, node)))
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
					coordinates[0] += edit_info->delta1;
					coordinates[1] += edit_info->delta2;
					coordinates[2] += edit_info->delta3;

					if (!Computed_field_set_values_at_node(edit_info->coordinate_field,
						node,edit_info->time, coordinates))
					{
						return_code=0;
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
	struct FE_node_edit_information *edit_info = NULL;
	FE_value_triple size;

	ENTER(FE_node_calculate_delta_vector);
	if (node&&(edit_info=(struct FE_node_edit_information *)edit_info_void)&&
		edit_info->fe_region&&edit_info->rc_coordinate_field&&
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
				if (!Computed_field_set_values_at_node(
					edit_info->wrapper_orientation_scale_field,node,edit_info->time,
					orientation_scale))
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
		if ((0.0 != edit_info->glyph_centre[0]) ||
			(0.0 != edit_info->glyph_size[0]) ||
			(0.0 == (scale_factor=edit_info->glyph_scale_factors[0])))
		{
			if (0.0 != edit_info->glyph_centre[0])
			{
				display_message(ERROR_MESSAGE,
					"To edit orientation vectors your main direction glyph centre must be zero.");
			}
			if (0.0 != edit_info->glyph_size[0])
			{
				display_message(ERROR_MESSAGE,
					"To edit orientation vectors your main direction base glyph size must be zero.");
			}
			if (0.0 == (scale_factor=edit_info->glyph_scale_factors[0]))
			{
				display_message(ERROR_MESSAGE,
					"To edit orientation vectors your main direction scale factor must not be zero.");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"FE_node_calculate_delta_vector.  Invalid argument(s)");
		}
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* FE_node_calculate_delta_vector */

/***************************************************************************//**
 * Translates the <rc_coordinate_field> of <node> according to the delta change
 * stored in the <edit_info>.
 */
static int FE_node_edit_vector(struct FE_node *node,
	struct FE_node_edit_information *edit_info)
{
	FE_value orientation_scale[9];
	int number_of_orientation_scale_components,return_code;

	ENTER(FE_node_edit_vector);
	if (node && edit_info &&
		edit_info->fe_region&&edit_info->orientation_scale_field&&
		(0<(number_of_orientation_scale_components=
			Computed_field_get_number_of_components(
				edit_info->orientation_scale_field)))&&
		(9>=number_of_orientation_scale_components))
	{
		return_code=1;
		/* the last_picked_node was edited in FE_node_calculate_delta_vector. */
		if ((node != edit_info->last_picked_node)&&(
			FE_region_contains_FE_node(edit_info->fe_region, node)))
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
					if (!Computed_field_set_values_at_node(
						edit_info->orientation_scale_field,node,edit_info->time,
						orientation_scale))
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

	ENTER(Node_tool_define_field_at_node_from_picked_coordinates);
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
		rc_coordinate_field=
			Computed_field_begin_wrap_coordinate_field(coordinate_field);
		if (rc_coordinate_field != 0)
		{
			picked_coordinate_field = Cmiss_graphic_get_coordinate_field(
				node_tool->graphic);
			rc_picked_coordinate_field = Computed_field_begin_wrap_coordinate_field(
				picked_coordinate_field);
			if (Computed_field_evaluate_at_node(rc_picked_coordinate_field,
				node,time,coordinates))
			{
				if (Node_tool_define_field_at_node(node_tool,node)&&
					Computed_field_set_values_at_node(rc_coordinate_field,
					node,time,coordinates))
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
	struct Interaction_volume *interaction_volume,
	struct FE_element *nearest_element, struct Computed_field *element_coordinate_field)
/*******************************************************************************
LAST MODIFIED : 14 February 2008

DESCRIPTION :
Returns a new node in the position indicated by <interaction_volume>, in the
<scene> if suppled. If any of the remaining address arguments
are not NULL, they are filled with the appropriate information pertaining to
the new node.
If <nearest_element> and <coordinate_field> are supplied then 
the interaction volume will be supplied a constraint function which will 
try to enforce that the node is created on that element.
==============================================================================*/
{
	double d,LU_transformation_matrix[16],node_coordinates[3];
	enum  Cmiss_graphic_type cmiss_graphic_type;
	FE_value coordinates[3];
	int i,LU_indx[4],node_number,transformation_required;
	struct Computed_field *rc_coordinate_field,*node_tool_coordinate_field;
	struct FE_node *merged_node, *node;
	struct Cmiss_rendition *rendition;
	struct Cmiss_graphic *graphic;
	struct Node_tool_element_constraint_function_data constraint_data;
	struct Scene_picked_object *scene_picked_object;

	ENTER(Node_tool_create_node_at_interaction_volume);
	USE_PARAMETER(scene);
	merged_node = (struct FE_node *)NULL;
	scene_picked_object=(struct Scene_picked_object *)NULL;
	rendition=(struct Cmiss_rendition *)NULL;
	graphic=(struct Cmiss_graphic *)NULL;
	if (!node_tool || !interaction_volume)
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_create_node_at_interaction_volume.  Invalid argument(s)");
	}
	else
	{
		if (!node_tool->coordinate_field)
		{
			display_message(ERROR_MESSAGE,
				"Node_tool_create_node_at_interaction_volume.  No coordinate field to define");
		}
		else if (!node_tool->fe_region)
		{
			display_message(ERROR_MESSAGE,
				"Node_tool_create_node_at_interaction_volume.  No region chosen for new node");
		}
		else
		{
			if (node_tool->use_data)
			{
				cmiss_graphic_type=CMISS_GRAPHIC_DATA_POINTS;
			}
			else
			{
				cmiss_graphic_type=CMISS_GRAPHIC_NODE_POINTS;
			}
			node_tool_coordinate_field=node_tool->coordinate_field;
			rendition = Cmiss_region_get_rendition_internal(node_tool->region);
			if (scene && rendition)
			{
				graphic = first_graphic_in_Cmiss_rendition_that(
					rendition, Cmiss_graphic_type_matches,	(void*)cmiss_graphic_type);
				scene_picked_object=CREATE(Scene_picked_object)(/*hit_no*/0);
				if (scene_picked_object != 0)
				{
					Scene_picked_object_add_rendition(scene_picked_object, rendition);
					REACCESS(Scene_picked_object)(&(node_tool->scene_picked_object),
						scene_picked_object);
				}
			}
			else
			{
				scene_picked_object=(struct Scene_picked_object *)NULL;
			}
			rc_coordinate_field=
				Computed_field_begin_wrap_coordinate_field(node_tool_coordinate_field);
			if (rc_coordinate_field != 0)
			{
				node_number = FE_region_get_next_FE_node_identifier(
					node_tool->fe_region, /*start*/1);

				/* get new node coordinates from interaction_volume */
				if (nearest_element && element_coordinate_field)
				{
					constraint_data.element = nearest_element;
					constraint_data.found_element = nearest_element;
					constraint_data.coordinate_field = element_coordinate_field;
					for (i = 0; i < MAXIMUM_ELEMENT_XI_DIMENSIONS; i++)
					{
						constraint_data.xi[i] = 0.5;
					}
					Interaction_volume_get_placement_point(interaction_volume,
						node_coordinates, Node_tool_element_constraint_function,
						&constraint_data);
				}
				else
				{
				Interaction_volume_get_placement_point(interaction_volume,
					node_coordinates, (Interation_volume_constraint_function)NULL,
					(void *)NULL);
				}

				for (i=0;i<3;i++)
				{
					coordinates[i]=(FE_value)node_coordinates[i];
				}
				if (scene_picked_object&&
					Scene_picked_object_get_total_transformation_matrix(
						node_tool->scene_picked_object,&transformation_required,
						LU_transformation_matrix)&&transformation_required&&
					LU_decompose(4,LU_transformation_matrix,LU_indx,&d,/*singular_tolerance*/1.0e-12))
				{
					world_to_model_coordinates(coordinates,
						LU_transformation_matrix,LU_indx);
				}
				node = CREATE(FE_node)(node_number, node_tool->fe_region, /*template*/(struct FE_node *)NULL);
				if (!node)
				{
					display_message(ERROR_MESSAGE,
						"Node_tool_create_node_at_interaction_volume.  Could not create node");
				}
				else
				{
					ACCESS(FE_node)(node);
					if (!Node_tool_define_field_at_node(node_tool,node) ||
						!Computed_field_set_values_at_node(rc_coordinate_field, node,
							/*time*/0, coordinates) ||
						(nearest_element && constraint_data.found_element && node_tool->element_xi_field &&
							!FE_node_define_and_set_element_xi(node, node_tool->element_xi_field,
								constraint_data.found_element, constraint_data.xi)) ||
						(NULL == (merged_node =
							FE_region_merge_FE_node(node_tool->fe_region, node))))
					{
						display_message(ERROR_MESSAGE,
							"Node_tool_create_node_at_interaction_volume.  Could not define and set fields");
					}
					else
					{
						if (node_tool->group_field)
						{
							Cmiss_field_module_id field_module = Cmiss_region_get_field_module(node_tool->region);
							Cmiss_nodeset_id master_nodeset = Cmiss_field_module_find_nodeset_by_name(field_module,
								node_tool->use_data ? "cmiss_data" : "cmiss_nodes");
							Cmiss_field_module_begin_change(field_module);
							Cmiss_field_node_group_id modify_node_group =
								Cmiss_field_group_get_node_group(node_tool->group_field, master_nodeset);
							if (!modify_node_group)
							{
								modify_node_group = Cmiss_field_group_create_node_group(node_tool->group_field, master_nodeset);
							}
							Cmiss_nodeset_group_id modify_nodeset_group = Cmiss_field_node_group_get_nodeset(modify_node_group);
							if (!Cmiss_nodeset_contains_node(Cmiss_nodeset_group_base_cast(modify_nodeset_group), merged_node))
							{
								if (!Cmiss_nodeset_group_add_node(modify_nodeset_group, merged_node))
								{
									display_message(ERROR_MESSAGE,
										"gfx modify ngroup:  Could not add node %d", Cmiss_node_get_identifier(node));
								}
							}
							Cmiss_field_module_end_change(field_module);
							Cmiss_nodeset_group_destroy(&modify_nodeset_group);
							Cmiss_field_node_group_destroy(&modify_node_group);
							Cmiss_nodeset_destroy(&master_nodeset);
							Cmiss_field_module_destroy(&field_module);
						}
					}
					DEACCESS(FE_node)(&node);
				}
				Computed_field_clear_cache(rc_coordinate_field);
				Computed_field_end_wrap(&rc_coordinate_field);
			}
		}
	}
	if (node_tool)
	{
		/* only need following if editing; in which case need all of them */
		if ((!merged_node) || (!scene_picked_object) || (!rendition) ||
			(!graphic))
		{
			scene_picked_object=(struct Scene_picked_object *)NULL;
			if (rendition)
			{
				Cmiss_rendition_destroy(&rendition);
			}
			graphic=(struct Cmiss_graphic *)NULL;
		}
		/* make sure node_tool point at following as found out above */
		REACCESS(Scene_picked_object)(&(node_tool->scene_picked_object),
			scene_picked_object);
		REACCESS(Cmiss_rendition)(&(node_tool->rendition),
			rendition);
		REACCESS(Cmiss_graphic)(&(node_tool->graphic),
			graphic);
	}
	if (rendition)
	{
		Cmiss_rendition_destroy(&rendition);
	}
	LEAVE;

	return (merged_node);
} /* Node_tool_create_node_at_interaction_volume */

static void Node_tool_reset(void *node_tool_void)
/*******************************************************************************
LAST MODIFIED : 25 February 2008

DESCRIPTION :
Resets current edit. Called on button release or when tool deactivated.
==============================================================================*/
{
	struct Node_tool *node_tool = (struct Node_tool *)node_tool_void;

	ENTER(Node_tool_reset);
	if (node_tool != 0)
	{
		REACCESS(FE_node)(&(node_tool->last_picked_node),
			(struct FE_node *)NULL);
		REACCESS(Interaction_volume)(
			&(node_tool->last_interaction_volume),
			(struct Interaction_volume *)NULL);
		REACCESS(Scene_picked_object)(&(node_tool->scene_picked_object),
			(struct Scene_picked_object *)NULL);
		REACCESS(Cmiss_rendition)(&(node_tool->rendition),
			(struct Cmiss_rendition *)NULL);
		REACCESS(Cmiss_graphic)(&(node_tool->graphic),
			(struct Cmiss_graphic *)NULL);
		REACCESS(Cmiss_rendition)(&(node_tool->rendition),
			(struct Cmiss_rendition *)NULL);
		if (node_tool->graphic)
			Cmiss_graphic_destroy(&node_tool->graphic);
	}
	else
	{
		display_message(ERROR_MESSAGE,"Node_tool_reset.  Invalid argument(s)");
	}
	LEAVE;
} /* Node_tool_reset */

int Node_tool_set_picked_node(struct Node_tool *node_tool, struct FE_node *picked_node)
{
	int return_code = 1;
	if (node_tool && picked_node)
	{
		if (node_tool->rendition)
		{
			Cmiss_field_module_id field_module = Cmiss_region_get_field_module(Cmiss_rendition_get_region(node_tool->rendition));
			Cmiss_field_module_begin_change(field_module);
			Cmiss_field_group_id selection_group = Cmiss_rendition_get_or_create_selection_group(node_tool->rendition);
			if (selection_group)
			{
				Cmiss_nodeset_id master_nodeset = Cmiss_field_module_find_nodeset_by_name(
					field_module, node_tool->use_data ? "cmiss_data" : "cmiss_nodes");
				if (master_nodeset)
				{
					Cmiss_field_node_group_id node_group = Cmiss_field_group_get_node_group(selection_group, master_nodeset);
					if (!node_group)
					{
						node_group = Cmiss_field_group_create_node_group(selection_group, master_nodeset);
					}
					Cmiss_nodeset_group_id nodeset_group = Cmiss_field_node_group_get_nodeset(node_group);
					Cmiss_nodeset_group_add_node(nodeset_group, picked_node);
					Cmiss_nodeset_group_destroy(&nodeset_group);
					Cmiss_field_node_group_destroy(&node_group);
					Cmiss_nodeset_destroy(&master_nodeset);
				}
			}
			Cmiss_field_group_destroy(&selection_group);
			Cmiss_field_module_end_change(field_module);
			Cmiss_field_module_destroy(&field_module);
		}
	}
	else
	{
		return_code = 0;
	}
	return return_code;
}

static void Node_tool_interactive_event_handler(void *device_id,
	struct Interactive_event *event,void *node_tool_void,
	struct Graphics_buffer *graphics_buffer)
/*******************************************************************************
LAST MODIFIED : 13 February 2008

DESCRIPTION :
Input handler for input from devices. <device_id> is a unique address enabling
the editor to handle input from more than one device at a time. The <event>
describes the type of event, button numbers and key modifiers, and the volume
of space affected by the interaction. Main events are button press, movement and
release.
==============================================================================*/
{
	char *command_string;
	double d;
	enum Graphic_glyph_scaling_mode glyph_scaling_mode;
	enum Interactive_event_type event_type;
	FE_value time;
	int clear_selection,input_modifier,return_code,shift_pressed;
	struct Computed_field *coordinate_field, *nearest_element_coordinate_field;
	struct FE_element *nearest_element;
	struct FE_node *picked_node;
	struct FE_node_edit_information edit_info;
	struct Cmiss_rendition *rendition = NULL, *rendition_element = NULL;
	struct Cmiss_graphic *graphic = NULL, *graphic_element = NULL;
	struct GT_object *glyph;
	struct Interaction_volume *interaction_volume,*temp_interaction_volume;
	struct LIST(Scene_picked_object) *scene_picked_object_list;
	struct Node_tool *node_tool;
	struct Scene *scene;
	struct Scene_picked_object *scene_picked_object, *scene_picked_object2;

	ENTER(Node_tool_interactive_event_handler);
	if (device_id&&event&&(node_tool=
		(struct Node_tool *)node_tool_void))
	{
		Cmiss_region_begin_hierarchical_change(node_tool->root_region);
		interaction_volume=Interactive_event_get_interaction_volume(event);
		scene = Interactive_event_get_scene(event);
		if (scene != 0)
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
						REACCESS(Interaction_volume)(&(node_tool->last_interaction_volume),
							interaction_volume);
						scene_picked_object_list=
							Scene_pick_objects(scene,interaction_volume,graphics_buffer);
						if (scene_picked_object_list != 0)
						{
							picked_node=(struct FE_node *)NULL;
							nearest_element = (struct FE_element *)NULL;
							if (node_tool->select_enabled)
							{
								picked_node=Scene_picked_object_list_get_nearest_node(
									scene_picked_object_list,node_tool->use_data,
									(struct Cmiss_region *)NULL,&scene_picked_object,
									&rendition,&graphic);
							}
							if (node_tool->constrain_to_surface)
							{
								nearest_element=Scene_picked_object_list_get_nearest_element(
									scene_picked_object_list,(struct Cmiss_region *)NULL,
									/*select_elements_enabled*/0, /*select_faces_enabled*/1, 
									/*select_lines_enabled*/0, &scene_picked_object2,
									&rendition_element,&graphic_element);
								/* Reject the previously picked node if the element is nearer */
								if (picked_node && nearest_element)
								{
									if (Scene_picked_object_get_nearest(scene_picked_object) >
										Scene_picked_object_get_nearest(scene_picked_object2))
									{
										picked_node = (struct FE_node *)NULL;
									}
								}				
							}
							if (picked_node)
							{
								node_tool->picked_node_was_unselected=1;
								Cmiss_field_group_id selection_group = Cmiss_rendition_get_selection_group(rendition);
								if (selection_group)
								{
									Cmiss_region_id temp_region = Cmiss_rendition_get_region(rendition);
									Cmiss_field_module_id field_module = Cmiss_region_get_field_module(temp_region);
									Cmiss_nodeset_id master_nodeset = Cmiss_field_module_find_nodeset_by_name(
										field_module, node_tool->use_data ? "cmiss_data" : "cmiss_nodes");
									Cmiss_field_node_group_id node_group = Cmiss_field_group_get_node_group(selection_group, master_nodeset);
									Cmiss_nodeset_destroy(&master_nodeset);
									if (node_group)
									{
										Cmiss_nodeset_group_id nodeset_group = Cmiss_field_node_group_get_nodeset(node_group);
										node_tool->picked_node_was_unselected =
											!Cmiss_nodeset_contains_node(Cmiss_nodeset_group_base_cast(nodeset_group), picked_node);
										Cmiss_nodeset_group_destroy(&nodeset_group);
										Cmiss_field_node_group_destroy(&node_group);
									}
									Cmiss_field_module_destroy(&field_module);
									Cmiss_field_group_destroy(&selection_group);
								}
								REACCESS(Scene_picked_object)(
									&(node_tool->scene_picked_object),scene_picked_object);
								REACCESS(Cmiss_rendition)(&(node_tool->rendition),
									rendition);
								REACCESS(Cmiss_graphic)(&(node_tool->graphic),graphic);
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
								if (node_tool->create_enabled)
								{
									/* Find the intersection of the element and the interaction volume */
									nearest_element_coordinate_field = (struct Computed_field *)NULL;
									if (nearest_element)
									{
										nearest_element_coordinate_field = Cmiss_graphic_get_coordinate_field(graphic_element);
									}
									/* If we are creating on elements and no element was selected then 
										don't create */
									if (!node_tool->constrain_to_surface || nearest_element)
									{
										picked_node=Node_tool_create_node_at_interaction_volume(
											node_tool,scene,interaction_volume,nearest_element,
											nearest_element_coordinate_field);
										if (picked_node != 0)
										{
											node_tool->picked_node_was_unselected=1;
										}
										else
										{
											node_tool->picked_node_was_unselected=0;
										}
									}
									else
									{
										node_tool->picked_node_was_unselected=0;
									}
								}
								else
								{
									node_tool->picked_node_was_unselected=0;
								}
							}
							REACCESS(FE_node)(&(node_tool->last_picked_node),picked_node);
							DESTROY(LIST(Scene_picked_object))(&(scene_picked_object_list));
							/*
							 * NOTE: make selection the last step so node_tool is in good state before
							 * receiving code gets to run: consider it capable of changing the current tool!
							 */
							clear_selection = !shift_pressed;
							if (clear_selection)
							{
								if (node_tool->root_region)
								{
									Cmiss_rendition *root_rendition = Cmiss_region_get_rendition_internal(
										node_tool->root_region);
									Cmiss_field_group_id root_group = Cmiss_rendition_get_selection_group(root_rendition);
									if (root_group)
									{
										if (!node_tool->use_data)
											Cmiss_field_group_clear_region_tree_node(root_group);
										else
											Cmiss_field_group_clear_region_tree_data(root_group);
										Cmiss_field_group_destroy(&root_group);
									}
									//Cmiss_rendition_flush_tree_selections(root_rendition);
									Cmiss_rendition_destroy(&root_rendition);
								}
							}
							if (picked_node)
							{
								Node_tool_set_picked_node(node_tool, picked_node);
							}
						}
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
						nearest_element = (struct FE_element *)NULL;
						nearest_element_coordinate_field = (struct Computed_field *)NULL;
						if (INTERACTIVE_EVENT_MOTION_NOTIFY==event_type)
						{
							node_tool->motion_detected=1;
						}
						if (node_tool->last_picked_node)
						{
							if (node_tool->constrain_to_surface)
							{
								scene_picked_object_list=
									Scene_pick_objects(scene,interaction_volume,graphics_buffer);
								if (scene_picked_object_list != 0)
								{
									if ( 0 != (nearest_element=Scene_picked_object_list_get_nearest_element(
										scene_picked_object_list,(struct Cmiss_region *)NULL,
										/*select_elements_enabled*/0, /*select_faces_enabled*/1, 
										/*select_lines_enabled*/0, &scene_picked_object2,
										&rendition_element,&graphic_element)))
									{
										nearest_element_coordinate_field = Cmiss_graphic_get_coordinate_field(graphic_element);
									}
									DESTROY(LIST(Scene_picked_object))(&(scene_picked_object_list));
								}
							}
							if (node_tool->create_enabled &&
								node_tool->streaming_create_enabled &&
								(INTERACTIVE_EVENT_MOTION_NOTIFY == event_type))
							{
								if (!node_tool->constrain_to_surface || nearest_element)
								{
									picked_node = Node_tool_create_node_at_interaction_volume(
											 node_tool, scene, interaction_volume, nearest_element,
											 nearest_element_coordinate_field);
									if (picked_node != 0)
									{
										Node_tool_set_picked_node(node_tool, picked_node);
										REACCESS(FE_node)(&(node_tool->last_picked_node),
											picked_node);
									}
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
								edit_info.fe_region=node_tool->fe_region;
								edit_info.time=Time_keeper_get_time(
									node_tool->time_keeper);
								edit_info.constrain_to_surface = node_tool->constrain_to_surface;
								edit_info.element_xi_field = node_tool->element_xi_field;
								edit_info.nearest_element = nearest_element;
								edit_info.nearest_element_coordinate_field = 
									nearest_element_coordinate_field;
								/* get coordinate field to edit */
								if (node_tool->define_enabled)
								{
									coordinate_field = node_tool->coordinate_field;
								}
								else
								{
									coordinate_field = Cmiss_graphic_get_coordinate_field(node_tool->graphic);
								}
								edit_info.coordinate_field=coordinate_field;
								/* get coordinate_field in RC coordinates */
								edit_info.rc_coordinate_field=
									Computed_field_begin_wrap_coordinate_field(coordinate_field);
								edit_info.orientation_scale_field=(struct Computed_field *)NULL;
								edit_info.wrapper_orientation_scale_field=
									(struct Computed_field *)NULL;
								if (!node_tool->graphic)
								{
									edit_info.glyph_centre[0] = 0.0;
									edit_info.glyph_centre[1] = 0.0;
									edit_info.glyph_centre[2] = 0.0;
									edit_info.glyph_size[0] = 1.0;
									edit_info.glyph_size[1] = 1.0;
									edit_info.glyph_size[2] = 1.0;
								}
								else if (Cmiss_graphic_get_glyph_parameters(
									node_tool->graphic, &glyph, &glyph_scaling_mode,
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
								if (!node_tool->scene_picked_object)
								{
									/* best we can do is use world coordinates;
									 * will look wrong if nodes drawn with a transformation */
									edit_info.transformation_required=0;
								}
								else if (!(Scene_picked_object_get_total_transformation_matrix(
									node_tool->scene_picked_object,
									&(edit_info.transformation_required),
									edit_info.transformation_matrix)&&
									copy_matrix(4,4,edit_info.transformation_matrix,
										edit_info.LU_transformation_matrix)&&
									((!edit_info.transformation_required)||
										LU_decompose(4,edit_info.LU_transformation_matrix,
											edit_info.LU_indx,&d,/*singular_tolerance*/1.0e-12))))
								{
									return_code=0;
								}
								if (return_code)
								{
									Cmiss_field_group_id selection_group = Cmiss_rendition_get_selection_group(node_tool->rendition);
									if (node_tool->rendition && selection_group)
									{
										Cmiss_region_id temp_region = Cmiss_rendition_get_region(node_tool->rendition);
										Cmiss_field_module_id field_module = Cmiss_region_get_field_module(temp_region);
										Cmiss_nodeset_id master_nodeset = Cmiss_field_module_find_nodeset_by_name(
											field_module, node_tool->use_data ? "cmiss_data" : "cmiss_nodes");
										Cmiss_field_module_destroy(&field_module);
										edit_info.fe_region=Cmiss_region_get_FE_region(temp_region);
										Cmiss_field_node_group_id node_group = Cmiss_field_group_get_node_group(selection_group, master_nodeset);
										Cmiss_nodeset_destroy(&master_nodeset);
										if (node_group)
										{
											Cmiss_nodeset_group_id nodeset_group = Cmiss_field_node_group_get_nodeset(node_group);
											FE_region_begin_change(edit_info.fe_region);
											/* edit vectors if non-constant orientation_scale field */
											if (((NODE_TOOL_EDIT_AUTOMATIC == node_tool->edit_mode)
													|| (NODE_TOOL_EDIT_VECTOR == node_tool->edit_mode))
													&& edit_info.wrapper_orientation_scale_field
													&& (!Computed_field_is_constant(
															edit_info.orientation_scale_field)))
											{

												/* edit vector */
												if (FE_node_calculate_delta_vector(
														node_tool->last_picked_node, (void *) &edit_info))
												{
													Cmiss_node_iterator_id iterator =
														Cmiss_nodeset_create_node_iterator(Cmiss_nodeset_group_base_cast(nodeset_group));
													Cmiss_node_id edit_node = 0;
													while (0 != (edit_node = Cmiss_node_iterator_next_non_access(iterator)))
													{
														FE_node_edit_vector(edit_node, &edit_info);
													}
													Cmiss_node_iterator_destroy(&iterator);
												}
											}
											else
											{
												if (NODE_TOOL_EDIT_VECTOR != node_tool->edit_mode)
												{
													/* edit position */
													if (FE_node_calculate_delta_position(node_tool->last_picked_node, &edit_info))
													{
														Cmiss_node_iterator_id iterator =
															Cmiss_nodeset_create_node_iterator(Cmiss_nodeset_group_base_cast(nodeset_group));
														Cmiss_node_id edit_node = 0;
														while (0 != (edit_node = Cmiss_node_iterator_next_non_access(iterator)))
														{
															FE_node_edit_position(edit_node, &edit_info);
														}
														Cmiss_node_iterator_destroy(&iterator);
													}
												}
												else
												{
													display_message(ERROR_MESSAGE, "Cannot edit vector: "
														"invalid orientation_scale field");
													return_code = 0;
												}
											}
											FE_region_end_change(edit_info.fe_region);
											Cmiss_field_node_group_destroy(&node_group);
											Cmiss_nodeset_group_destroy(&nodeset_group);
										}
										Cmiss_field_group_destroy(&selection_group);
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
									struct LIST(FE_node) *temp_node_list = CREATE(LIST(FE_node))();
									ADD_OBJECT_TO_LIST(FE_node)(node_tool->last_picked_node, temp_node_list);
									Cmiss_rendition_remove_selection_from_node_list(node_tool->rendition,
										temp_node_list, node_tool->use_data);
									DESTROY(LIST(FE_node))(&temp_node_list);
								}
							}
						}
						else if (node_tool->motion_detected)
						{
							/* rubber band select - make bounding box out of initial and
								 current interaction_volumes */
							temp_interaction_volume=
								create_Interaction_volume_bounding_box(
									node_tool->last_interaction_volume,interaction_volume);
							if (temp_interaction_volume != 0)
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
#if defined (USE_GRAPHICS_OBJECT)
										Scene_add_graphics_object(scene,node_tool->rubber_band,
											/*position*/0,"node_tool_rubber_band",
											/*fast_changing*/1);
#endif
									}
									Interaction_volume_make_polyline_extents(
										temp_interaction_volume,node_tool->rubber_band);
								}
								else
								{
#if defined (USE_GRAPHICS_OBJECT)
									Scene_remove_graphics_object(scene,node_tool->rubber_band);
#endif
									DEACCESS(GT_object)(&(node_tool->rubber_band));
								}
								if (INTERACTIVE_EVENT_BUTTON_RELEASE==event_type)
								{
									scene_picked_object_list=
										Scene_pick_objects(scene,temp_interaction_volume,graphics_buffer);
									if (scene_picked_object_list != 0)
									{
										Region_node_map *node_map = 
											(Region_node_map *)Scene_picked_object_list_get_picked_region_sorted_nodes(
												scene_picked_object_list, node_tool->use_data);
										if (node_map)
										{
											Cmiss_region *sub_region = NULL;
											Cmiss_field_group_id selection_group = NULL;
											Cmiss_rendition *region_rendition = NULL;
											Cmiss_nodeset_group_id nodeset_group = NULL;
											Region_node_map::iterator pos;
											for (pos = node_map->begin(); pos != node_map->end(); ++pos)
											{
												if (pos->first != sub_region)
												{
													if (sub_region && selection_group)
													{
														Cmiss_field_group_destroy(&selection_group);
													}
													if (nodeset_group)
													{
														Cmiss_nodeset_group_destroy(&nodeset_group);
													}
													if (region_rendition)
													{
														Cmiss_rendition_destroy(&region_rendition);
													}
													sub_region = pos->first;
													if (sub_region)
													{
														region_rendition= Cmiss_region_get_rendition_internal(sub_region);
														selection_group = Cmiss_rendition_get_or_create_selection_group(region_rendition);
													}
													if (selection_group)
													{
														Cmiss_field_module_id field_module = Cmiss_region_get_field_module(sub_region);
														Cmiss_nodeset_id master_nodeset = Cmiss_field_module_find_nodeset_by_name(
															field_module, node_tool->use_data ? "cmiss_data" : "cmiss_nodes");
														if (master_nodeset)
														{
															Cmiss_field_node_group_id node_group = Cmiss_field_group_get_node_group(selection_group, master_nodeset);
															if (!node_group)
															{
																node_group = Cmiss_field_group_create_node_group(selection_group, master_nodeset);
															}
															nodeset_group = Cmiss_field_node_group_get_nodeset(node_group);
															Cmiss_field_node_group_destroy(&node_group);
															Cmiss_nodeset_destroy(&master_nodeset);
														}
														Cmiss_field_module_destroy(&field_module);
													}
												}
												if (nodeset_group)
												{
													Cmiss_nodeset_group_add_node(nodeset_group, pos->second);
												}
											}
											if (selection_group)
											{
												Cmiss_field_group_destroy(&selection_group);
											}
											if (nodeset_group)
											{
												Cmiss_nodeset_group_destroy(&nodeset_group);
											}
											if (region_rendition)
											{
												Cmiss_rendition_destroy(&region_rendition);
											}
											delete node_map;
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
							/* Execute command_field of last_picked_node as last step of button release as code
							 * invoked by it may modify this tool, or change to another tool before returning */
							if (node_tool->last_picked_node && node_tool->command_field &&
								Computed_field_is_defined_at_node(node_tool->command_field,
									node_tool->last_picked_node))
							{
								if (node_tool->time_keeper)
								{
									time = Time_keeper_get_time(node_tool->time_keeper);
								}
								else
								{
									time = 0;
								}
								if (node_tool->execute_command &&
									(NULL != (command_string = Computed_field_evaluate_as_string_at_node(
									node_tool->command_field, /*component_number*/-1,
									node_tool->last_picked_node, time))))
								{
									Execute_command_execute_string(node_tool->execute_command,
										command_string);
									DEALLOCATE(command_string);
								}
							}
							Node_tool_reset((void *)node_tool);
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
		if (node_tool->root_region)
		{
			Cmiss_rendition *root_rendition = Cmiss_region_get_rendition_internal(
				node_tool->root_region);
			Cmiss_rendition_flush_tree_selections(root_rendition);
			Cmiss_rendition_destroy(&root_rendition);
		}
		Cmiss_region_end_hierarchical_change(node_tool->root_region);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_interactive_event_handler.  Invalid argument(s)");
	}
	LEAVE;
} /* Node_tool_interactive_event_handler */

static int Node_tool_destroy_node_tool(void **node_tool_void)
/*******************************************************************************
LAST MODIFIED : 6 July 2007

DESCRIPTION :
Function to call DESTROY
==============================================================================*/
{
	ENTER(Node_tool_destroy_node_tool);

	Node_tool *node_tool = (struct Node_tool *)*node_tool_void;
	int return_code = 0;

	if (node_tool != 0)
	{
		 return_code = DESTROY(Node_tool)(&node_tool);
	}

	LEAVE;
	return (return_code);
}

static int Node_tool_bring_up_interactive_tool_dialog(void *node_tool_void,
	 struct Graphics_window *graphics_window)
/*******************************************************************************
LAST MODIFIED : 20 June 2001

DESCRIPTION :
Brings up a dialog for editing settings of the Node_tool - in a standard format
for passing to an Interactive_toolbar.
==============================================================================*/
{
	int return_code;

	ENTER(Node_tool_bring_up_interactive_tool_dialog);
	return_code = Node_tool_pop_up_dialog((struct Node_tool *)node_tool_void,
		 graphics_window);
	LEAVE;

	return (return_code);
} /* Node_tool_bring_up_interactive_tool_dialog */

#if defined (WX_USER_INTERFACE)
class wxNodeTool : public wxPanel
{
	 Node_tool *node_tool;
	 FE_region *master_fe_region;
	 FE_field *fe_field;
	 wxCheckBox *button_select;
	 wxCheckBox *button_edit;
	 wxCheckBox *button_motion_update;
	 wxCheckBox *button_define;
	 wxCheckBox *button_create;
	 wxCheckBox *button_streaming;
	 wxCheckBox *button_constrain;
	 wxButton *button_destroy;
	 wxButton *button_undefine;
	 wxWindow *Title;
	 wxRegionChooser *region_chooser;
	 wxCheckBox *node_command_field_checkbox;
	 wxPanel *node_command_field_chooser_panel;
	 wxCheckBox *element_xi_field_checkbox;
	 wxPanel *element_xi_field_chooser_panel;
	 wxPanel *cmgui_node_tool;
	 wxButton *clear_button;
	 wxCheckBox *create_elements_checkbox;
	 wxTextCtrl *dimension_textctrl;
	 wxListBox *new_element_nodes_listbox;
	 wxStaticText *new_element_statictext, *dimension_statictext;
	 wxWindow *first_element_staticbox,*second_element_staticbox;
	 char temp_string[20];
	 DEFINE_MANAGER_CLASS(Computed_field);
	 Managed_object_chooser<Computed_field, MANAGER_CLASS(Computed_field)>
	 *computed_field_chooser;
	 Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
	 	*node_command_field_chooser;
	 Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
	 	*subgroup_field_chooser;
	 Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
	 	*element_xi_field_chooser;

public:

  wxNodeTool(Node_tool *node_tool, wxPanel *parent): 
    node_tool(node_tool)
  {
  	struct Computed_field *command_field, *element_xi_field;
	  { // Suppress the error message if we have already initialised this xrc
		  wxLogNull logNo;
		  wxXmlInit_node_tool();
	  } // ~wxLogNull called, old log restored

	  wxXmlResource::Get()->LoadPanel(this, parent, _T("CmguiNodeTool"));
		button_select = XRCCTRL(*this, "ButtonSelect", wxCheckBox);
		button_edit = XRCCTRL(*this, "ButtonEdit", wxCheckBox);
		button_motion_update=XRCCTRL(*this, "ButtonMotionUpdate", wxCheckBox);
		button_define = XRCCTRL(*this, "ButtonDefine", wxCheckBox);
		button_create = XRCCTRL(*this, "ButtonCreate", wxCheckBox);
		button_streaming = XRCCTRL(*this, "ButtonStreaming", wxCheckBox);  
		button_constrain = XRCCTRL(*this, "ButtonConstrain", wxCheckBox );
		button_select->SetValue(node_tool->select_enabled);
		button_edit->SetValue(node_tool->edit_enabled);
		button_motion_update->SetValue(node_tool->motion_update_enabled);
		button_define->SetValue(node_tool->define_enabled);
		button_create->SetValue(node_tool->create_enabled);
		button_streaming->SetValue(node_tool->streaming_create_enabled);
		button_constrain ->SetValue(node_tool->constrain_to_surface);
		create_elements_checkbox = XRCCTRL(*this, "CreateElementsCheckBox", wxCheckBox );
		dimension_textctrl = XRCCTRL(*this, "DimensionTextCtrl", wxTextCtrl);
		clear_button = XRCCTRL(*this, "ClearButton", wxButton);
		new_element_nodes_listbox = XRCCTRL(*this, "NewElementNodesListBox", wxListBox);
		new_element_statictext = XRCCTRL(*this, "NewElementStaticText",wxStaticText);
		dimension_statictext = XRCCTRL(*this, "DimensionStaticText",wxStaticText);
		first_element_staticbox = XRCCTRL(*this, "FirstElementStaticBox",wxWindow);
		second_element_staticbox = XRCCTRL(*this, "SecondElementStaticBox",wxWindow);

		Title = XRCCTRL(*this,"NodeSizer",wxWindow); 
		if (node_tool->use_data)
		{
			 Title->SetLabel("Data Tool");
			 create_elements_checkbox->Hide();
			 dimension_textctrl->Hide();
			 clear_button->Hide();
			 new_element_nodes_listbox->Hide();
			 new_element_statictext->Hide();
			 dimension_statictext->Hide();
			 first_element_staticbox->Hide();
			 second_element_staticbox->Hide();
		}
		else
		{
			 Title->SetLabel("Node Tool");
			 create_elements_checkbox->Show();
			 dimension_textctrl->Show();
			 clear_button->Show();
			 new_element_nodes_listbox->Show();
			 new_element_statictext->Show();
			 dimension_statictext->Show();
			 create_elements_checkbox->SetValue(node_tool->element_create_enabled);
			 sprintf(temp_string,"%d",node_tool->element_dimension);
			 dimension_textctrl->ChangeValue(temp_string);
			 first_element_staticbox->Show();
			 second_element_staticbox->Show();
		}

		wxStaticText *subgroup_field_text=XRCCTRL(*this,"SubgroupFieldText",wxStaticText);
		wxPanel *subgroup_field_chooser_panel = XRCCTRL(*this,"SubgroupFieldChooserPanel",wxPanel);

		Cmiss_field_id subgroup_field = Cmiss_field_group_base_cast(node_tool->group_field);
		subgroup_field_chooser =
			new Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
			(subgroup_field_chooser_panel, subgroup_field, node_tool->computed_field_manager,
				Cmiss_field_is_type_group, (void *)NULL,
				node_tool->user_interface);
		Callback_base< Computed_field* > *subgroup_field_callback =
			new Callback_member_callback< Computed_field*,
			wxNodeTool, int (wxNodeTool::*)(Computed_field *) >
			(this, &wxNodeTool::subgroup_field_callback);
		subgroup_field_chooser->set_callback(subgroup_field_callback);
		subgroup_field_chooser_panel->Fit();
		subgroup_field_chooser->include_null_item(true);
		subgroup_field_text->Show();
		subgroup_field_chooser->set_object(subgroup_field);
		subgroup_field_chooser_panel->Show();

	 wxPanel *coordinate_field_chooser_panel = 
		 XRCCTRL(*this, "CoordinateFieldChooserPanel", wxPanel);
	  computed_field_chooser = 
		  new Managed_object_chooser<Computed_field, MANAGER_CLASS(Computed_field)>
		  (coordinate_field_chooser_panel, node_tool->coordinate_field,
			  node_tool->computed_field_manager, 
			  Computed_field_has_up_to_3_numerical_components,
			  (void *)NULL, node_tool->user_interface);
	  Callback_base<Computed_field* > *coordinate_field_callback = 
		  new Callback_member_callback< Computed_field*, 
		  wxNodeTool, int (wxNodeTool::*)(Computed_field *) >
		  (this, &wxNodeTool::coordinate_field_callback);
	  computed_field_chooser->set_callback(coordinate_field_callback);

	  wxPanel *region_chooser_panel = 
		 XRCCTRL(*this, "RegionChooserPanel", wxPanel);
	  char *initial_path;
	  initial_path = Cmiss_region_get_root_region_path();
	  region_chooser = new wxRegionChooser(region_chooser_panel,
		  node_tool->root_region, initial_path); 
	  DEALLOCATE(initial_path);
	  Callback_base<Cmiss_region* > *Node_tool_wx_region_callback = 
		  new Callback_member_callback< Cmiss_region*, 
		  wxNodeTool, int (wxNodeTool::*)(Cmiss_region *) >
		  (this, &wxNodeTool::Node_tool_wx_region_callback);
	  region_chooser->set_callback(Node_tool_wx_region_callback);
	  if (node_tool->region != NULL)
	  {
			wx_Node_tool_set_region(node_tool->region);
	  }

		node_command_field_checkbox = XRCCTRL(*this, "NodeCommandFieldCheckBox",wxCheckBox);
		node_command_field_chooser_panel = XRCCTRL(*this, "NodeCommandFieldChooserPanel", wxPanel);
		command_field = Node_tool_get_command_field(node_tool);
		node_command_field_chooser =
			 new Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
			 (node_command_field_chooser_panel, command_field, node_tool->computed_field_manager,
					Computed_field_has_string_value_type, (void *)NULL, node_tool->user_interface);
		Callback_base< Computed_field* > *node_command_field_callback = 
			 new Callback_member_callback< Computed_field*, 
				wxNodeTool, int (wxNodeTool::*)(Computed_field *) >
			 (this, &wxNodeTool::node_command_field_callback);
		node_command_field_chooser->set_callback(node_command_field_callback);
		if (command_field == NULL)
		{
			node_command_field_checkbox->SetValue(0);
			node_command_field_chooser_panel->Disable();
		}
		else
		{
			node_command_field_checkbox->SetValue(1);
			node_command_field_chooser_panel->Enable();
		}

		element_xi_field_checkbox = XRCCTRL(*this, "ElementXiFieldCheckBox",wxCheckBox);
		element_xi_field_chooser_panel = XRCCTRL(*this, "ElementXiFieldChooserPanel", wxPanel);
		element_xi_field = Node_tool_get_element_xi_field(node_tool);
		element_xi_field_chooser =
			 new Managed_object_chooser<Computed_field,MANAGER_CLASS(Computed_field)>
			 (element_xi_field_chooser_panel, element_xi_field, node_tool->computed_field_manager,
					 Computed_field_has_element_xi_fe_field, (void *)NULL, node_tool->user_interface);
		Callback_base< Computed_field* > *element_xi_field_callback = 
			 new Callback_member_callback< Computed_field*, 
				wxNodeTool, int (wxNodeTool::*)(Computed_field *) >
			 (this, &wxNodeTool::element_xi_field_callback);
		element_xi_field_chooser->set_callback(element_xi_field_callback);
		if (element_xi_field == NULL)
		{
			element_xi_field_checkbox->SetValue(0);
			element_xi_field_chooser_panel->Disable();
		}
		else
		{
			element_xi_field_checkbox->SetValue(1);
			element_xi_field_chooser_panel->Enable();
		}

		XRCCTRL(*this, "DimensionTextCtrl", wxTextCtrl)->Connect(wxEVT_KILL_FOCUS,
			wxCommandEventHandler(wxNodeTool::OnDimensionEntered), NULL, this);

		cmgui_node_tool = XRCCTRL(*this, "CmguiNodeTool", wxPanel);
		cmgui_node_tool->SetScrollbar(wxVERTICAL,0,-1,-1);
		cmgui_node_tool -> Layout();
	}

  wxNodeTool()  /* Void constructor required for IMPLEMENT_DYNAMIC_CLASS */
  {
  }

  ~wxNodeTool()
  {
		 if (computed_field_chooser)
				delete computed_field_chooser;
		 if (subgroup_field_chooser)
				delete subgroup_field_chooser;
		 if (region_chooser)
				delete region_chooser;
		 if (node_command_field_chooser)
				delete node_command_field_chooser;
		 if (element_xi_field_chooser)
				delete element_xi_field_chooser;
  }

	int coordinate_field_callback(Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 9 February 2007

DESCRIPTION :
Callback from wxChooser<Computed_field> when choice is made.
==============================================================================*/
	{
		Node_tool_set_coordinate_field(node_tool, field);
		return 1;
	}

	int subgroup_field_callback(Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 9 February 2007

DESCRIPTION :
Callback from wxChooser<Computed_field> when choice is made.
==============================================================================*/
	{
		Cmiss_field_group_id subgroup_field = Cmiss_field_cast_group(field);
		Node_tool_set_region(node_tool, node_tool->region, subgroup_field);
		Cmiss_field_group_destroy(&subgroup_field);
		return 1;
	}

	int Node_tool_wx_region_callback(Cmiss_region *region)
/*******************************************************************************
LAST MODIFIED : 9 February 2007

DESCRIPTION :
Callback from wxChooser<Computed_field> when choice is made.
==============================================================================*/
	{
		Node_tool_set_region(node_tool, region, NULL);
		return 1;
	}

	int setCoordinateField(Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 9 February 2007

DESCRIPTION :
Set the selected option in the Coordinate Field chooser.
==============================================================================*/
	{
		computed_field_chooser->set_object(field);
		return 1;
	}

	 int node_command_field_callback(Computed_field *command_field)
	 {
			Node_tool_set_command_field(node_tool, command_field);
			return 1;
	 }

	int element_xi_field_callback(Computed_field *element_xi_field)
	{
		Node_tool_set_element_xi_field(node_tool, element_xi_field);
		return 1;
	}

	void OnButtonSelectpressed(wxCommandEvent& event)
	{    
	USE_PARAMETER(event);
		button_select = XRCCTRL(*this, "ButtonSelect", wxCheckBox);
		Node_tool_set_select_enabled(node_tool,button_select->IsChecked());
	}

	void OnButtonEditpressed(wxCommandEvent& event)
	{ 
	USE_PARAMETER(event);
		button_edit = XRCCTRL(*this, "ButtonEdit", wxCheckBox);
		Node_tool_set_edit_enabled(node_tool,button_edit->IsChecked());
	}

	void OnButtonMotionUpdatepressed(wxCommandEvent& event)
	{    
	USE_PARAMETER(event);
		 button_motion_update = XRCCTRL(*this, "ButtonMotionUpdate", wxCheckBox);
		 Node_tool_set_motion_update_enabled(node_tool,button_motion_update->IsChecked());
	}

	void OnButtonDefinepressed(wxCommandEvent& event)
	{    
	USE_PARAMETER(event);
		button_define = XRCCTRL(*this, "ButtonDefine", wxCheckBox);
		Node_tool_set_define_enabled(node_tool,button_define->IsChecked());
		if (!button_define->IsChecked())
		{
			button_create = XRCCTRL(*this, "ButtonCreate", wxCheckBox);
			button_create->SetValue(0);
			Node_tool_set_create_enabled(node_tool,button_create->IsChecked());
		} 
	}

	void OnButtonCreatepressed(wxCommandEvent& event)
	{    	 
	USE_PARAMETER(event);
		button_create = XRCCTRL(*this, "ButtonCreate", wxCheckBox);
		Node_tool_set_create_enabled(node_tool,button_create->IsChecked());

    if (button_create->IsChecked())
		{
	  	button_define = XRCCTRL(*this, "ButtonDefine", wxCheckBox);	
			button_define->SetValue(1);
		  Node_tool_set_define_enabled(node_tool,button_define->IsChecked());
		}
	}

	void OnButtonStreamingpressed(wxCommandEvent& event)
	{    
	USE_PARAMETER(event);
		button_streaming = XRCCTRL(*this, "ButtonStreaming", wxCheckBox);  
		Node_tool_set_streaming_create_enabled(node_tool,button_streaming->IsChecked());
	}

	void OnButtonConstrainpressed(wxCommandEvent& event)
	{    
	USE_PARAMETER(event);
		button_constrain = XRCCTRL(*this, "ButtonConstrain", wxCheckBox ); 	 
		Node_tool_set_constrain_to_surface(node_tool,button_constrain->IsChecked());
	}

	void OnButtonDestroypressed(wxCommandEvent& event)
	{    
		USE_PARAMETER(event);
		Node_tool_destroy_selected_nodes(node_tool);
	}

	void OnButtonUndefinepressed(wxCommandEvent& event)
	{    
	int number_in_elements;
	struct LIST(FE_field) *fe_field_list;
	struct LIST(FE_node) *node_list;
	button_undefine = XRCCTRL(*this, "ButtonUndefine", wxButton);  

	USE_PARAMETER(event);
	if (node_tool->coordinate_field && (fe_field_list=
			Computed_field_get_defining_FE_field_list(node_tool->coordinate_field)))
	{
		if ((1==NUMBER_IN_LIST(FE_field)(fe_field_list))&&
				(fe_field=FIRST_OBJECT_IN_LIST_THAT(FE_field)(
						(LIST_CONDITIONAL_FUNCTION(FE_field) *)NULL,(void *)NULL,
						fe_field_list)))
		{
			Cmiss_rendition *rendition = Cmiss_region_get_rendition_internal(node_tool->region);
			if (rendition)
			{
				Cmiss_field_group_id selection_group = Cmiss_rendition_get_selection_group(rendition);
				if (selection_group)
				{
					node_list = FE_node_list_from_region_and_selection_group(
						node_tool->region, NULL, Cmiss_field_group_base_cast(selection_group), NULL, 0, node_tool->use_data);
					FE_region_begin_change(node_tool->fe_region);
					FE_region_undefine_FE_field_in_FE_node_list(
						node_tool->fe_region, fe_field, node_list, &number_in_elements);
					display_message(WARNING_MESSAGE,
						"Field could not be undefined in %d node(s) "
						"because in-use by elements", number_in_elements);
					FE_region_end_change(node_tool->fe_region);
					Cmiss_field_group_destroy(&selection_group);
					DESTROY(LIST(FE_node))(&node_list);
				}
				Cmiss_rendition_destroy(&rendition);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
					"Node_tool_undefine_selected_CB.  Invalid field");
		}
		DESTROY(LIST(FE_field))(&fe_field_list);
	}
	}

void OnClearButtonpressed(wxCommandEvent &event)
{
	USE_PARAMETER(event);
	 if (node_tool)
	 {
			Node_tool_end_element_creation(node_tool);
	 }
	else
	{
		display_message(ERROR_MESSAGE,
			"Element_creator_abort_creation_CB.  Invalid argument(s)");
	}
}

void OnCreateElementsPressed(wxCommandEvent &event)
 {
	USE_PARAMETER(event);
		create_elements_checkbox = XRCCTRL(*this, "CreateElementsCheckBox", wxCheckBox );
		if (node_tool)
		{
			 Node_tool_set_element_create_enabled(node_tool,
					create_elements_checkbox->IsChecked());
		}
		else
		{
		display_message(ERROR_MESSAGE,
			 "Element_creator_create_button_CB.  Invalid argument(s)");
		}
 }

void OnDimensionEntered(wxCommandEvent &event)
 {
		char *value_string;
		int element_dimension;
		
	USE_PARAMETER(event);
		dimension_textctrl = XRCCTRL(*this, "DimensionTextCtrl", wxTextCtrl);
		if (node_tool)
		{
			 value_string=const_cast<char *>((dimension_textctrl->GetValue()).c_str());
			 if (value_string != 0)
			 {
					if (1==sscanf(value_string,"%d",&element_dimension))
					{
						 Node_tool_set_element_dimension(node_tool,
								element_dimension);
					}
			 }
			 /* always restore element_dimension_text to actual value stored */
			 Node_tool_refresh_element_dimension_text(node_tool);
		}
		else
		{
			 display_message(ERROR_MESSAGE,
					"Element_creator_element_dimension_text_CB.  Invalid argument(s)");
		}
 }


void NodeToolInterfaceRenew_element_xi_field(Node_tool *node_tool)
{
	struct Computed_field *element_xi_field = Node_tool_get_element_xi_field(node_tool);
	element_xi_field_checkbox->SetValue(element_xi_field != 0);
	if (element_xi_field)
	{
		element_xi_field_chooser->set_object(element_xi_field);
		element_xi_field_chooser_panel->Enable();
	}
	else
	{
		element_xi_field_chooser_panel->Disable();
	}
}

void NodeToolInterfaceRenew(Node_tool *destination_node_tool)
{
	button_select = XRCCTRL(*this, "ButtonSelect", wxCheckBox);
	button_edit = XRCCTRL(*this, "ButtonEdit", wxCheckBox);
	button_motion_update=XRCCTRL(*this, "ButtonMotionUpdate", wxCheckBox);
	button_define = XRCCTRL(*this, "ButtonDefine", wxCheckBox);
	button_create = XRCCTRL(*this, "ButtonCreate", wxCheckBox);
	button_streaming = XRCCTRL(*this, "ButtonStreaming", wxCheckBox);  
	button_constrain = XRCCTRL(*this, "ButtonConstrain", wxCheckBox );
	create_elements_checkbox = XRCCTRL(*this, "CreateElementsCheckBox", wxCheckBox );
	dimension_textctrl = XRCCTRL(*this, "DimensionTextCtrl", wxTextCtrl);
	button_select->SetValue(destination_node_tool->select_enabled);
	button_edit->SetValue(destination_node_tool->edit_enabled);
	button_motion_update->SetValue(destination_node_tool->motion_update_enabled);
	button_define->SetValue(destination_node_tool->define_enabled);
	button_create->SetValue(destination_node_tool->create_enabled);
	button_streaming->SetValue(destination_node_tool->streaming_create_enabled);
	button_constrain->SetValue(destination_node_tool->constrain_to_surface);
	NodeToolInterfaceRenew_element_xi_field(destination_node_tool);
	computed_field_chooser->set_object(destination_node_tool->coordinate_field);
	if (destination_node_tool->region != NULL)
	{
		wx_Node_tool_set_region(destination_node_tool->region);
	}
	Node_tool_set_element_dimension(destination_node_tool,destination_node_tool->element_dimension);
	create_elements_checkbox->SetValue(destination_node_tool->element_create_enabled);
}

void NodeCommandFieldChecked(wxCommandEvent &event)
{
	struct Computed_field *command_field = (struct Computed_field *)NULL;
	USE_PARAMETER(event);
	if (node_command_field_checkbox->IsChecked() &&
		(command_field = node_command_field_chooser->get_object()))
	{
		node_command_field_chooser_panel->Enable();
	}
	else
	{
		node_command_field_checkbox->SetValue(0);
		node_command_field_chooser_panel->Disable();
	}
	Node_tool_set_command_field(node_tool, command_field);
}

void ElementXiFieldChecked(wxCommandEvent &event)
{
	struct Computed_field *element_xi_field = (struct Computed_field *)NULL;
	USE_PARAMETER(event);
	if (element_xi_field_checkbox->IsChecked() &&
		(element_xi_field = element_xi_field_chooser->get_object()))
	{
		element_xi_field_chooser_panel->Enable();
	}
	else
	{
		element_xi_field_checkbox->SetValue(0);
		element_xi_field_chooser_panel->Disable();
	}
	Node_tool_set_element_xi_field(node_tool, element_xi_field);
}

void wx_Node_tool_set_region_path(char *path)
{
	if (path && (region_chooser != NULL))
	{
		/* path must start with '/' to match items in chooser */
		if (path[0] != '/')
		{
			char *temp = NULL;
			if (ALLOCATE(temp,char,strlen(path)+2))
			{
				strcpy(temp, "/");
				strcat(temp, path);
				region_chooser->set_path(temp);
				DEALLOCATE(temp);
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"wx_Node_tool_set_region_path.  Insufficient memory");
			}
		}
		else
		{
			region_chooser->set_path(path);
		}
	}
}

struct Cmiss_region *wx_Node_tool_get_region()
{
	 if (region_chooser)
	 {
			return (region_chooser->get_region());
	 }
	 else
	 {
			return (node_tool->root_region);
	 }
}

void wx_Node_tool_set_region(struct Cmiss_region *new_region)
{
	if (region_chooser != NULL)
	{
		region_chooser->set_region(new_region);
	}
}

void wx_Node_tool_set_field_chooser_manager()
{
	if (node_tool->computed_field_manager)
	{
		if (computed_field_chooser != NULL)
		{
			computed_field_chooser->set_manager(node_tool->computed_field_manager);
		}
		if (subgroup_field_chooser != NULL)
		{
			subgroup_field_chooser->set_manager(node_tool->computed_field_manager);
		}
		if (node_command_field_chooser != NULL)
		{
			node_command_field_chooser->set_manager(node_tool->computed_field_manager);
		}
		if (element_xi_field_chooser  != NULL)
		{
			element_xi_field_chooser->set_manager(node_tool->computed_field_manager);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "Node_tool_set_field_chooser_manager.  "
			"Could not update manager in choosers. Invalid field manager");
	}
}

void setSubgroup(Cmiss_field_id field)
{
	if (subgroup_field_chooser != NULL)
	{
		subgroup_field_chooser->set_object(field);
	}
}

  DECLARE_DYNAMIC_CLASS(wxNodeTool);
  DECLARE_EVENT_TABLE();
};

IMPLEMENT_DYNAMIC_CLASS(wxNodeTool, wxPanel)

BEGIN_EVENT_TABLE(wxNodeTool, wxPanel)
	 EVT_CHECKBOX(XRCID("ButtonSelect"),wxNodeTool::OnButtonSelectpressed)
	 EVT_CHECKBOX(XRCID("ButtonEdit"),wxNodeTool::OnButtonEditpressed)
	 EVT_CHECKBOX(XRCID("ButtonMotionUpdate"),wxNodeTool::OnButtonMotionUpdatepressed)
	 EVT_CHECKBOX(XRCID("ButtonDefine"),wxNodeTool::OnButtonDefinepressed)
	 EVT_CHECKBOX(XRCID("ButtonCreate"),wxNodeTool::OnButtonCreatepressed)
	 EVT_CHECKBOX(XRCID("ButtonStreaming"),wxNodeTool::OnButtonStreamingpressed)
	 EVT_CHECKBOX(XRCID("ButtonConstrain"),wxNodeTool::OnButtonConstrainpressed)
	 EVT_CHECKBOX(XRCID("ElementXiFieldCheckBox"),wxNodeTool::ElementXiFieldChecked)
	 EVT_CHECKBOX(XRCID("NodeCommandFieldCheckBox"),wxNodeTool::NodeCommandFieldChecked)
	 EVT_BUTTON(XRCID("ButtonDestroy"),wxNodeTool::OnButtonDestroypressed)
	 EVT_BUTTON(XRCID("ButtonUndefine"),wxNodeTool::OnButtonUndefinepressed)
	 EVT_BUTTON(XRCID("ClearButton"),wxNodeTool::OnClearButtonpressed)
	 EVT_CHECKBOX(XRCID("CreateElementsCheckBox"),wxNodeTool::OnCreateElementsPressed)
	 EVT_TEXT_ENTER(XRCID("DimensionTextCtrl"),wxNodeTool::OnDimensionEntered)
END_EVENT_TABLE()


#endif /* defined (WX_USER_INTERFACE) */

/*
Global functions
----------------
*/
#if defined (WX_USER_INTERFACE)
void Node_tool_set_wx_interface(void *node_tool_void)
/*******************************************************************************
LAST MODIFIED : 13 April 2007

DESCRIPTION :
Set the wx_interface for new settings.
==============================================================================*/	 
{
	 struct Node_tool *node_tool =(struct Node_tool *)node_tool_void;
	 if (node_tool != 0) 
	 {
			if (node_tool->wx_node_tool != (wxNodeTool *) NULL)
			{	
				 node_tool->wx_node_tool->NodeToolInterfaceRenew(node_tool);	
			}
	 }
}
#endif /*(WX_USER_INTERFACE)*/

static int Node_tool_copy_function(
	void *destination_tool_void, void *source_tool_void,
	struct MANAGER(Interactive_tool) *destination_tool_manager) 
/*******************************************************************************
LAST MODIFIED : 29 March 2007

DESCRIPTION :
Copies the state of one node tool to another.
==============================================================================*/
{
	int return_code;
	struct Node_tool *destination_node_tool, *source_node_tool;

	ENTER(Node_tool_copy_function);
	if ((destination_tool_void || destination_tool_manager) &&
		(source_node_tool=(struct Node_tool *)source_tool_void))
	{
		if (destination_tool_void)
		{
			destination_node_tool = (struct Node_tool *)destination_tool_void;
		}
		else
		{
			destination_node_tool = CREATE(Node_tool)(
				destination_tool_manager,
				source_node_tool->root_region, 
				source_node_tool->use_data,
				source_node_tool->rubber_band_material,
				source_node_tool->user_interface,
				source_node_tool->time_keeper);
			Node_tool_set_execute_command(destination_node_tool,
				source_node_tool->execute_command);
		}
		if (destination_node_tool)
		{
			destination_node_tool->coordinate_field = source_node_tool->coordinate_field;
			destination_node_tool->create_enabled = source_node_tool->create_enabled;
			destination_node_tool->define_enabled = source_node_tool->define_enabled;
			destination_node_tool->edit_enabled= source_node_tool->edit_enabled;
			destination_node_tool->motion_update_enabled= source_node_tool->motion_update_enabled;
			destination_node_tool->select_enabled = source_node_tool->select_enabled;
			destination_node_tool->streaming_create_enabled = source_node_tool->streaming_create_enabled;
			destination_node_tool->constrain_to_surface= source_node_tool->constrain_to_surface;
			destination_node_tool->command_field = source_node_tool->command_field;
			destination_node_tool->element_xi_field = source_node_tool->element_xi_field;
			destination_node_tool->element_create_enabled = source_node_tool->element_create_enabled;
			destination_node_tool->element_dimension = source_node_tool->element_dimension;
			Node_tool_set_region(destination_node_tool, source_node_tool->region, source_node_tool->group_field);
#if defined (WX_USER_INTERFACE)
			if (destination_node_tool->wx_node_tool != (wxNodeTool *) NULL)
			{	
				destination_node_tool->wx_node_tool->NodeToolInterfaceRenew(destination_node_tool);
			}
			return_code=1;
#endif /*(WX_USER_INTERFACE)*/
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Node_tool_copy_function.  Could not create copy.");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_copy_function.  Invalid argument(s)");
		return_code=0;
	}
	return (return_code);
} /* Node_tool_copy_function */

static int Node_tool_add_element(struct Node_tool *node_tool)
/*******************************************************************************
LAST MODIFIED : 11 April 2007

DESCRIPTION :
Adds the just created element to the fe_region, adding faces as necessary.
==============================================================================*/
{
	int return_code;

	ENTER(node_tool_add_element);
	if (node_tool && node_tool->fe_region && node_tool->element)
	{
		FE_region_begin_change(node_tool->fe_region);
		FE_region_begin_define_faces(node_tool->fe_region, /*all dimensions*/-1);
		return_code = FE_region_merge_FE_element_and_faces_and_nodes(
			node_tool->fe_region, node_tool->element);
		FE_region_end_define_faces(node_tool->fe_region);
		FE_region_end_change(node_tool->fe_region);
		Node_tool_end_element_creation(node_tool);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"node_tool_add_element.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* node_tool_add_element */


static void Node_tool_Computed_field_change(
	struct MANAGER_MESSAGE(Computed_field) *message,void *node_tool_void)
{
	struct CM_element_information element_identifier;
	struct Node_tool *node_tool;
#if defined (WX_USER_INTERFACE)
	wxString blank;
	wxListBox *new_element_nodes_list_box;
	char temp_string[50];
	int number;
#endif
	int number_of_nodes;
	struct FE_field *fe_field;
	struct LIST(FE_field) *fe_field_list;
	struct LIST(Computed_field) *changed_field_list=NULL;

	ENTER(Node_tool_Computed_field_change);
	node_tool =	(struct Node_tool *)node_tool_void;
	if (message && node_tool && node_tool->element_create_enabled)
	{
		Cmiss_rendition *rendition = Cmiss_region_get_rendition_internal(node_tool->region);
		if (rendition)
		{
			Cmiss_field_group_id selection_group = Cmiss_rendition_get_selection_group(rendition);
			changed_field_list =
				MANAGER_MESSAGE_GET_CHANGE_LIST(Computed_field)(message,
					MANAGER_CHANGE_RESULT(Computed_field));
			if (selection_group && changed_field_list && Computed_field_or_ancestor_satisfies_condition(
				Cmiss_field_group_base_cast(selection_group), Computed_field_is_in_list, (void *)changed_field_list))
			{
				Cmiss_field_module_id field_module = Cmiss_region_get_field_module(node_tool->region);
				Cmiss_field_node_group_id node_group = NULL;
				Cmiss_nodeset_id master_nodeset = Cmiss_field_module_find_nodeset_by_name(
					field_module, node_tool->use_data ? "cmiss_data" : "cmiss_nodes");
				Cmiss_field_module_destroy(&field_module);
				node_group = Cmiss_field_group_get_node_group(selection_group, master_nodeset);
				Cmiss_nodeset_destroy(&master_nodeset);
				Cmiss_node_id node = 0;
				if (node_group)
				{
					Cmiss_nodeset_group_id nodeset_group = Cmiss_field_node_group_get_nodeset(node_group);
					/* make sure there is only one node selected in group */
					if (1 == Cmiss_nodeset_get_size(Cmiss_nodeset_group_base_cast(nodeset_group)))
					{
						Cmiss_node_iterator_id iterator = Cmiss_nodeset_create_node_iterator(
							Cmiss_nodeset_group_base_cast(nodeset_group));
						node = Cmiss_node_iterator_next(iterator);
						Cmiss_node_iterator_destroy(&iterator);
					}
					Cmiss_nodeset_group_destroy(&nodeset_group);
					Cmiss_field_node_group_destroy(&node_group);
				}
				if (node)
				{
					/* get the last selected node and check it is in the FE_region */
					if (FE_region_contains_FE_node(node_tool->fe_region, node))
					{
						if (!node_tool->element)
						{
							/* get next unused element identifier from fe_region */
							element_identifier.type = CM_ELEMENT;
							element_identifier.number= FE_region_get_next_FE_element_identifier(
								node_tool->fe_region, node_tool->element_dimension, 1);
							if (node_tool->coordinate_field && (fe_field_list
									= Computed_field_get_defining_FE_field_list(
											node_tool->coordinate_field)))
							{
								if ((1 == NUMBER_IN_LIST(FE_field)(fe_field_list)) && (fe_field
										= FIRST_OBJECT_IN_LIST_THAT(FE_field)(
												(LIST_CONDITIONAL_FUNCTION(FE_field) *) NULL,
												(void *) NULL, fe_field_list)) && (3
														>= get_FE_field_number_of_components(fe_field))
														&& (FE_VALUE_VALUE == get_FE_field_value_type(fe_field)))
								{
									if (node_tool->template_element
											|| (((node_tool->template_element
													= create_FE_element_with_line_shape(
															/*identifier*/1, node_tool->fe_region,
															node_tool->element_dimension))
															&& FE_element_define_tensor_product_basis(
																	node_tool->template_element,
																	node_tool->element_dimension,/*basis_type*/
																	LINEAR_LAGRANGE, fe_field))
																	&& ACCESS(FE_element)(node_tool->template_element)))
									{
										node_tool->element = CREATE(FE_element)(
												&element_identifier, (struct FE_element_shape *) NULL,
												(struct FE_region *) NULL, node_tool->template_element);
										if (node_tool->element != 0) 
										{
											ACCESS(FE_element)(node_tool->element);
											node_tool->number_of_clicked_nodes = 0;
										}
										else
										{
											display_message(ERROR_MESSAGE,
													"Node_tool_Computed_field_change.  Could not create element");
										}
									}
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
										"Node_tool_Computed_field_change.  Could not create template element");
							}
						}
						if (node_tool->element)
						{
							/* When we make more than linear elements we will need
							 to check that the derivatives exist and are the correct ones */
							if (set_FE_element_node(node_tool->element,
									node_tool->number_of_clicked_nodes, node))
							{
#if defined (WX_USER_INTERFACE)
								if (node_tool->wx_node_tool != NULL)
								{
									sprintf(temp_string, "%d. Node %d",
											node_tool->number_of_clicked_nodes + 1,
											get_FE_node_identifier(node));
									wxString string(temp_string, wxConvUTF8);
									new_element_nodes_list_box = XRCCTRL(
											*node_tool->wx_node_tool, "NewElementNodesListBox",
											wxListBox);
									number = new_element_nodes_list_box->GetCount();
									if (number == 0)
									{
										new_element_nodes_list_box->InsertItems(1, &blank, number);
									}
									number = new_element_nodes_list_box->GetCount();
									new_element_nodes_list_box->InsertItems(1, &string, number
											- 1);
								}
#endif
								node_tool->number_of_clicked_nodes++;
								if (get_FE_element_number_of_nodes(node_tool->element,
										&number_of_nodes) && (node_tool->number_of_clicked_nodes
												== number_of_nodes))
								{
									Node_tool_add_element(node_tool);
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,
										"Node_tool_Computed_field_change.  Could not set node");
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,
									"Node_tool_Computed_field_change.  Could not create element");
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,
								"Element creator: Selected node not from current region");
					}
					Cmiss_node_destroy(&node);
				}
			}
			Cmiss_rendition_destroy(&rendition);
			if (selection_group)
			{
				Cmiss_field_group_destroy(&selection_group);
			}
		}
	}
	else
	{
		 display_message(ERROR_MESSAGE,
				"Node_tool_Computed_field_change.  Invalid argument(s)");
	}
	LEAVE;
}

int Node_tool_set_element_create_enabled(struct Node_tool *node_tool ,
	 int element_create_enabled)
/*******************************************************************************
LAST MODIFIED : 11 April 2007

DESCRIPTION :
Sets flag controlling whether elements are created in response to
node selection.
==============================================================================*/
{
	int return_code;
	ENTER(node_tool_set_create_enabled);
	if (node_tool )
	{
		/* make sure value of flag is 1 */
		if (element_create_enabled)
		{
			element_create_enabled=1;
		}
		if (element_create_enabled != node_tool ->element_create_enabled)
		{
			node_tool->element_create_enabled=element_create_enabled;
			if (element_create_enabled)
			{
				if (node_tool->computed_field_manager)
				{
					node_tool->computed_field_manager_callback_id=
						MANAGER_REGISTER(Computed_field)(
							Node_tool_Computed_field_change,(void *)node_tool,
							node_tool->computed_field_manager);
				}
			}
			else
			{
				Node_tool_end_element_creation(node_tool);
				if (node_tool->computed_field_manager_callback_id)
				{
					MANAGER_DEREGISTER(Computed_field)(
						node_tool->computed_field_manager_callback_id,
						node_tool->computed_field_manager);
					node_tool->computed_field_manager_callback_id=NULL;
				}
			}
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"node_tool _set_create_enabled.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* node_tool _set_create_enabled */

static int Node_tool_set_region(struct Node_tool *node_tool,
	struct Cmiss_region *region, Cmiss_field_group_id group)
/*******************************************************************************
LAST MODIFIED : 8 September 2008

DESCRIPTION :
Sets the <region> used by <node_tool> and update the chooser to include fields
in this region only.
==============================================================================*/
{
	int return_code;

	ENTER(Node_tool_set_region);
	if (node_tool)
	{
		return_code=1;
		if (region != node_tool->region)
		{
#if defined (WX_USER_INTERFACE)
			if (node_tool->wx_node_tool)
				node_tool->wx_node_tool->wx_Node_tool_set_region(region);
#endif
			Node_tool_end_element_creation(node_tool);
			node_tool->region = region;
			if (region)
			{
				node_tool->fe_region = Cmiss_region_get_FE_region(region);
				if (node_tool->use_data)
				{
					node_tool->fe_region = FE_region_get_data_FE_region(node_tool->fe_region);
				}
			}
			else
			{
				node_tool->fe_region = (struct FE_region *)NULL;
			}
			if (node_tool->computed_field_manager_callback_id)
			{
				MANAGER_DEREGISTER(Computed_field)(node_tool->computed_field_manager_callback_id,
					node_tool->computed_field_manager);
				node_tool->computed_field_manager_callback_id=NULL;
			}
			node_tool->computed_field_manager = Cmiss_region_get_Computed_field_manager(
				node_tool->region);
			if (node_tool->element_create_enabled && node_tool->computed_field_manager)
			{
				node_tool->computed_field_manager_callback_id=
					MANAGER_REGISTER(Computed_field)(Node_tool_Computed_field_change,(void *)node_tool,
						node_tool->computed_field_manager);
			}
#if defined (WX_USER_INTERFACE)
			if (node_tool->wx_node_tool)
			{
				node_tool->wx_node_tool->wx_Node_tool_set_field_chooser_manager();
			}
#endif
		}
		if (node_tool->group_field != group)
		{
			if (node_tool->group_field)
				Cmiss_field_group_destroy(&node_tool->group_field);
			if (group)
			{
				Cmiss_field_access(Cmiss_field_group_base_cast(group));
				node_tool->group_field = group;
			}
#if defined (WX_USER_INTERFACE)
			if (node_tool->wx_node_tool)
			{
				node_tool->wx_node_tool->setSubgroup(Cmiss_field_group_base_cast(node_tool->group_field));
			}
#endif
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_set_region.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_tool_set_region */

struct Node_tool *CREATE(Node_tool)(
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
	struct Cmiss_region *root_region, int use_data,
	struct Graphical_material *rubber_band_material,
	struct User_interface *user_interface,
	struct Time_keeper *time_keeper)
/*******************************************************************************
LAST MODIFIED : 17 May 2003

DESCRIPTION :
Creates a Node_tool for editing nodes/data in the <node_manager>.
The <use_data> flag indicates to use data, and that the selection
refers to data, not nodes; needed since different GT_element_settings types are
used to represent them. <element_manager> should be NULL if <use_data> is true.
==============================================================================*/
{
	char *initial_path;
	const char *tool_display_name,*tool_name;
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Node_tool *node_tool;

	ENTER(CREATE(Node_tool));
	node_tool=(struct Node_tool *)NULL;
	if (interactive_tool_manager&&root_region&&
		(NULL != (computed_field_manager=
			Cmiss_region_get_Computed_field_manager(root_region)))
		&&rubber_band_material&&user_interface)
	{
		initial_path = Cmiss_region_get_root_region_path();
		if (ALLOCATE(node_tool,struct Node_tool,1))
		{
			node_tool->execute_command=NULL;
			node_tool->interactive_tool_manager=interactive_tool_manager;
			node_tool->root_region=root_region;
			node_tool->region=(struct Cmiss_region *)NULL;
			node_tool->group_field = (Cmiss_field_group_id)NULL;
			node_tool->fe_region=(struct FE_region *)NULL;
			node_tool->use_data = use_data;
			node_tool->rubber_band_material=
				ACCESS(Graphical_material)(rubber_band_material);
			node_tool->user_interface=user_interface;
			node_tool->time_keeper = (struct Time_keeper *)NULL;
			node_tool->computed_field_manager_callback_id = NULL;
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
 			node_tool->constrain_to_surface=0;
			/* settings of the element creator */
			node_tool->FE_coordinate_field = (struct FE_field *)NULL;
			node_tool->element_dimension = 2;
			node_tool->template_element = (struct FE_element *)NULL;
			node_tool->element_create_enabled = 0;
			node_tool->element = (struct FE_element *)NULL;
			node_tool->number_of_clicked_nodes = 0;

			node_tool->edit_mode=NODE_TOOL_EDIT_AUTOMATIC;
			node_tool->coordinate_field =
				FIRST_OBJECT_IN_MANAGER_THAT(Computed_field)(
					Computed_field_has_up_to_3_numerical_components, (void *)NULL,
					computed_field_manager);
			node_tool->command_field = (struct Computed_field *)NULL;
			node_tool->element_xi_field = (struct Computed_field *)NULL;
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
#if defined (WX_USER_INTERFACE) /* switch (USER_INTERFACE) */
			node_tool->computed_field_manager=Cmiss_region_get_Computed_field_manager(root_region);
			node_tool->wx_node_tool = (wxNodeTool *)NULL;
			/* Set defaults until we have some sort of region chooser */
			node_tool->tool_position=wxPoint(0,0);
#endif /*defined (WX_USER_INTERFACE)*/
			Node_tool_set_region(node_tool, node_tool->root_region, NULL);
			node_tool->interactive_tool=CREATE(Interactive_tool)(
				tool_name,tool_display_name,
				Interactive_tool_node_type_string,
				Node_tool_interactive_event_handler,
				Node_tool_bring_up_interactive_tool_dialog,
				Node_tool_reset,
 				Node_tool_destroy_node_tool,
				Node_tool_copy_function,
				(void *)node_tool);
			ADD_OBJECT_TO_MANAGER(Interactive_tool)(
				node_tool->interactive_tool,
				node_tool->interactive_tool_manager);
			node_tool->scene_picked_object=(struct Scene_picked_object *)NULL;
			node_tool->last_picked_node=(struct FE_node *)NULL;

			node_tool->rendition=(struct Cmiss_rendition *)NULL;
			node_tool->graphic=(struct Cmiss_graphic *)NULL;

			node_tool->last_interaction_volume=(struct Interaction_volume *)NULL;
			node_tool->rubber_band=(struct GT_object *)NULL;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"CREATE(Node_tool).  Not enough memory");
			DEALLOCATE(node_tool);
		}
		DEALLOCATE(initial_path);
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
		Node_tool_reset((void *)node_tool);
		if (node_tool->computed_field_manager_callback_id)
		{
			MANAGER_DEREGISTER(Computed_field)(
				node_tool->computed_field_manager_callback_id,
				node_tool->computed_field_manager);
			node_tool->computed_field_manager_callback_id=NULL;
		}
		if (node_tool->template_element)
		{
			DEACCESS(FE_element)(&node_tool->template_element);
		}
		REACCESS(GT_object)(&(node_tool->rubber_band),(struct GT_object *)NULL);
		DEACCESS(Graphical_material)(&(node_tool->rubber_band_material));
		if (node_tool->time_keeper)
		{
			DEACCESS(Time_keeper)(&(node_tool->time_keeper));
		}
#if defined (WX_USER_INTERFACE)
		if (node_tool->wx_node_tool)
			 node_tool->wx_node_tool->Destroy();
#endif /*(WX_USER_INTERFACE)*/
		if (node_tool->group_field)
			Cmiss_field_group_destroy(&node_tool->group_field);
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

int Node_tool_pop_up_dialog(struct Node_tool *node_tool, struct Graphics_window *graphics_window)
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
#if defined (WX_USER_INTERFACE) /* switch (USER_INTERFACE) */
		wxPanel *pane;
		if (!node_tool->wx_node_tool)
		{
			 wxScrolledWindow *GeneralSettingPanel = Graphics_window_get_interactive_tool_panel(graphics_window);
			 node_tool->wx_node_tool = new wxNodeTool(node_tool,
					GeneralSettingPanel);
			 pane = XRCCTRL(*node_tool->wx_node_tool, "CmguiNodeTool", wxPanel);
			 node_tool->tool_position = pane->GetPosition();
			 node_tool->wx_node_tool->Show();
		}
		else
		{
			 pane = XRCCTRL(*node_tool->wx_node_tool, "CmguiNodeTool", wxPanel);
			 pane->SetPosition(node_tool->tool_position);
			 node_tool->wx_node_tool->Show();
		}
#else /* switch (USER_INTERFACE) */
		USE_PARAMETER(graphics_window);
		display_message(ERROR_MESSAGE, "Node_tool_pop_up_dialog.  "
			"No dialog implemented for this User Interface");
#endif /* switch (USER_INTERFACE) */
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
#if defined (WX_USER_INTERFACE)
			if (node_tool->wx_node_tool)
			{
				node_tool->wx_node_tool->setCoordinateField(coordinate_field);
			}
#endif /* defined (WX_USER_INTERFACE) */

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

	ENTER(Node_tool_set_create_enabled);
	if (node_tool)
	{
		return_code=1;
		if (create_enabled)
		{
			if (node_tool->region && node_tool->coordinate_field)
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
		/* make sure value of flag is 1 */
		if (motion_update_enabled)
		{
			motion_update_enabled=1;
		}
		if (motion_update_enabled != node_tool->motion_update_enabled)
		{
			node_tool->motion_update_enabled=motion_update_enabled;
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

int Node_tool_get_constrain_to_surface(struct Node_tool *node_tool)
/*******************************************************************************
LAST MODIFIED : 18 April 2005

DESCRIPTION :
Returns flag controlling, if create_enabled, whether new nodes will be created
on the closest surface element or just halfway between near and far.
==============================================================================*/
{
	int constrain_to_surface;

	ENTER(Node_tool_get_constrain_to_surface);
	if (node_tool)
	{
		constrain_to_surface = node_tool->constrain_to_surface;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_get_constrain_to_surface.  Invalid argument(s)");
		constrain_to_surface = 0;
	}
	LEAVE;

	return (constrain_to_surface);
} /* Node_tool_get_constrain_to_surface */

int Node_tool_set_constrain_to_surface(struct Node_tool *node_tool,
	int constrain_to_surface)
/*******************************************************************************
LAST MODIFIED : 18 April 2005

DESCRIPTION :
Sets flag controlling, if create_enabled, whether new nodes will be created
on the closest surface element or just halfway between near and far.
==============================================================================*/
{
	int return_code;

	ENTER(Node_tool_set_constrain_to_surface);
	if (node_tool)
	{
		if (constrain_to_surface)
		{
			/* make sure value of flag is exactly 1 */
			constrain_to_surface = 1;
		}
		if (constrain_to_surface != node_tool->constrain_to_surface)
		{
			node_tool->constrain_to_surface = constrain_to_surface;
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_set_constrain_to_surface.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Node_tool_set_constrain_to_surface */

struct Computed_field *Node_tool_get_element_xi_field(
	struct Node_tool *node_tool)
/*******************************************************************************
LAST MODIFIED : 18 February 2008

DESCRIPTION :
Returns the element_xi_field to define when the node is created in the <node_tool>
and node is constrained to surface.
==============================================================================*/
{
	struct Computed_field *element_xi_field;

	ENTER(Node_tool_get_element_xi_field);
	if (node_tool)
	{
		element_xi_field=node_tool->element_xi_field;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_get_element_xi_field.  Invalid argument(s)");
		element_xi_field=(struct Computed_field *)NULL;
	}
	LEAVE;

	return (element_xi_field);
} /* Node_tool_get_element_xi_field */

int Node_tool_set_element_xi_field(struct Node_tool *node_tool,
	struct Computed_field *element_xi_field)
/*******************************************************************************
LAST MODIFIED : 19 February 2008

DESCRIPTION :
Sets the element_xi_field to define when the node is created in the <node_tool>
and node is constrained to surface.
==============================================================================*/
{
	int return_code;

	ENTER(Node_tool_set_element_xi_field);
	if (node_tool && ((!element_xi_field) ||
		Computed_field_has_element_xi_fe_field(element_xi_field, (void *)NULL)))
	{
		if (element_xi_field != node_tool->element_xi_field)
		{
			node_tool->element_xi_field = element_xi_field;
		}	
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_set_element_xi_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_tool_set_element_xi_field */

struct Computed_field *Node_tool_get_command_field(
	struct Node_tool *node_tool)
/*******************************************************************************
LAST MODIFIED : 4 July 2002

DESCRIPTION :
Returns the command_field to be exectued when the node is clicked on in the <node_tool>.
==============================================================================*/
{
	struct Computed_field *command_field;

	ENTER(Node_tool_get_command_field);
	if (node_tool)
	{
		command_field=node_tool->command_field;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_get_command_field.  Invalid argument(s)");
		command_field=(struct Computed_field *)NULL;
	}
	LEAVE;

	return (command_field);
} /* Node_tool_get_command_field */

int Node_tool_set_command_field(struct Node_tool *node_tool,
	struct Computed_field *command_field)
/*******************************************************************************
LAST MODIFIED : 4 July 2002

DESCRIPTION :
Sets the command_field to be executed when the node is clicked on in the <node_tool>.
==============================================================================*/
{
	int return_code;

	ENTER(Node_tool_set_command_field);
	if (node_tool && ((!command_field) ||
		Computed_field_has_string_value_type(command_field, (void *)NULL)))
	{
		return_code = 1;
		if (command_field != node_tool->command_field)
		{
			node_tool->command_field = command_field;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_set_command_field.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Node_tool_set_command_field */

struct Interactive_tool *Node_tool_get_interactive_tool(
	struct Node_tool *node_tool)
/*******************************************************************************
LAST MODIFIED : 7 April 2007

DESCRIPTION :
Returns the generic interactive_tool the represents the <node_tool>.
==============================================================================*/
{
	struct Interactive_tool *interactive_tool;

	ENTER(Node_tool_get_interactive_tool);
	if (node_tool)
	{
		interactive_tool=node_tool->interactive_tool;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_get_interactive_tool.  Invalid argument(s)");
		interactive_tool=(struct Interactive_tool *)NULL;
	}
	LEAVE;

	return (interactive_tool);
} /* Node_tool_get_interactive_tool */

static int Node_tool_end_element_creation(
	struct Node_tool *node_tool)
/*******************************************************************************
LAST MODIFIED : 11 April 2007

DESCRIPTION :
DEACCESSes the element being created, if any, and if it is unmanaged, warns that
the creation was aborted. Also clears the node list.
Call this function whether element is successfully created or not.
==============================================================================*/
{
	int return_code;

	ENTER(node_tool_end_element_creation);
	if (node_tool)
	{
		if (node_tool->element)
		{
			if (!FE_region_contains_FE_element(node_tool->fe_region,
				node_tool->element))
			{
				display_message(WARNING_MESSAGE,
					"node_tool: destroying incomplete element");
			}
			else
			{
				if (node_tool->group_field)
				{
					Cmiss_field_module_id field_module = Cmiss_region_get_field_module(node_tool->region);
					Cmiss_mesh_id master_mesh = Cmiss_field_module_find_mesh_by_dimension(field_module,
						Cmiss_element_get_dimension(node_tool->element));
					Cmiss_field_module_begin_change(field_module);
					Cmiss_field_element_group_id modify_element_group =
						Cmiss_field_group_get_element_group(node_tool->group_field, master_mesh);
					if (!modify_element_group)
					{
						modify_element_group = Cmiss_field_group_create_element_group(node_tool->group_field, master_mesh);
					}
					Cmiss_mesh_group_id modify_mesh_group = Cmiss_field_element_group_get_mesh(modify_element_group);
					if (!Cmiss_mesh_contains_element(Cmiss_mesh_group_base_cast(modify_mesh_group), node_tool->element))
					{
						if (!Cmiss_mesh_group_add_element(modify_mesh_group, node_tool->element))
						{
							display_message(ERROR_MESSAGE,
								"Node_tool_end_element_creation:  Could not add element %d",
								Cmiss_element_get_identifier(node_tool->element));
						}
					}
					Cmiss_field_module_end_change(field_module);
					Cmiss_mesh_group_destroy(&modify_mesh_group);
					Cmiss_field_element_group_destroy(&modify_element_group);
					Cmiss_mesh_destroy(&master_mesh);
					Cmiss_field_module_destroy(&field_module);
				}
			}

			DEACCESS(FE_element)(&(node_tool->element));
#if defined (WX_USER_INTERFACE)
			wxListBox *new_element_nodes_list_box;
			if (node_tool->wx_node_tool != NULL)
			{
				 new_element_nodes_list_box = XRCCTRL(*node_tool->wx_node_tool, "NewElementNodesListBox", wxListBox);
				 new_element_nodes_list_box->Clear();
			}
#endif /* defined (WX_USER_INTERFACE)*/
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"node_tool_end_element_creation.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Element_creator_end_element_creation */


#if defined (WX_USER_INTERFACE)
int Node_tool_set_element_dimension(
	struct Node_tool *node_tool,int element_dimension)
/*******************************************************************************
LAST MODIFIED : 11 April 2007

DESCRIPTION :
Sets the <element_dimension> of elements to be created by <node_tool>.
==============================================================================*/
{
	int return_code;

	ENTER(Node_tool_set_element_dimension);
	if (node_tool)
	{
		if ((0<element_dimension)&&(element_dimension<=3))
		{
			return_code=1;
			if (node_tool->element_dimension != element_dimension)
			{
				node_tool->element_dimension=element_dimension;
				Node_tool_end_element_creation(node_tool);
				/* lose the current template element and node, if any */
				REACCESS(FE_element)(&(node_tool->template_element),
					(struct FE_element *)NULL);
			}
			if (node_tool->wx_node_tool != NULL)
			{
				 Node_tool_refresh_element_dimension_text(node_tool);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,"Dimension must be from 1 to 3");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_set_element_dimension.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* node_tool_set_element_dimension */
#endif /* defined (WX_USER_INTERFACE)*/

# if defined (WX_USER_INTERFACE)
static int Node_tool_refresh_element_dimension_text(
	struct Node_tool *node_tool)
/*******************************************************************************
LAST MODIFIED : 11 April 2007

DESCRIPTION :
Updates what is shown on the dimension text field.
==============================================================================*/
{
	 char temp_string[20],*value_string;
	 int return_code;
 	 wxTextCtrl *dimension_textctrl;
	 dimension_textctrl = XRCCTRL(*node_tool->wx_node_tool, "DimensionTextCtrl", wxTextCtrl);

	ENTER(Node_tool_refresh_element_dimension_text);
	if (node_tool)
	{
		return_code=1;
		value_string=const_cast<char *>((dimension_textctrl->GetValue()).c_str());
		if (value_string != 0)
		{
			 sprintf(temp_string,"%d",node_tool->element_dimension);
			 /* only set string if different from that shown */
			 if (strcmp(temp_string,value_string))
			 {
					wxString string(temp_string, wxConvUTF8);
					dimension_textctrl->ChangeValue(string);
			 }
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_refresh_element_dimension_text.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* node_tool_refresh_element_dimension_text */
#endif /* defined (WX_USER_INTERFACE)*/

#if defined (WX_USER_INTERFACE)
int Node_tool_get_element_create_enabled(struct Node_tool *node_tool)
/*******************************************************************************
LAST MODIFIED : 12 April 2007

DESCRIPTION :
Returns flag controlling whether node edits are updated during motion_notify
events, not just at the end of a mouse gesture.
==============================================================================*/
{
	int element_create_enabled;

	ENTER(Node_tool_get_element_create_enabled);
	if (node_tool)
	{
		element_create_enabled=node_tool->element_create_enabled;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_get_element_create_enabled.  Invalid argument(s)");
		element_create_enabled=0;
	}
	LEAVE;

	return (element_create_enabled);
} /* node_tool_get_create_enabled */
#endif /* defined (WX_USER_INTERFACE)*/

#if defined (WX_USER_INTERFACE)
int Node_tool_get_element_dimension(
	struct Node_tool *node_tool)
/*******************************************************************************
LAST MODIFIED : 12 April 2007

DESCRIPTION :
Returns the dimension of elements to be created by the <node_tool>.
==============================================================================*/
{
	int element_dimension;

	ENTER(Node_tool_get_element_dimension);
	if (node_tool)
	{
		element_dimension=node_tool->element_dimension;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_get_element_dimension.  Invalid argument(s)");
		element_dimension=0;
	}
	LEAVE;

	return (element_dimension);
} /* node_tool_get_element_dimension */
#endif /* defined (WX_USER_INTERFACE)*/

int Node_tool_set_execute_command(struct Node_tool *node_tool, 
	struct Execute_command *execute_command)
{
	int return_code = 0;
	if (node_tool)
	{
		node_tool->execute_command = execute_command;
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Node_tool_set_execute_command.  Invalid argument(s)");
	}
	
	return return_code;
}

int Node_tool_execute_command(struct Node_tool *node_tool, const char *command_string)
{
	int return_code = 0;
	if (command_string && node_tool)
	{
		struct Parse_state *state = create_Parse_state(command_string);
		return_code = Node_tool_execute_command_with_parse_state(node_tool, state);
		destroy_Parse_state(&state);
	}

	return return_code;
}

int Node_tool_execute_command_with_parse_state(struct Node_tool *node_tool, struct Parse_state *state)
/*******************************************************************************
LAST MODIFIED : 19 February 2008

DESCRIPTION :
Executes a GFX NODE_TOOL or GFX_DATA_TOOL command. If <data_tool_flag> is set,
then the <data_tool> is being modified, otherwise the <node_tool>.
Which tool that is being modified is passed in <node_tool_void>.
==============================================================================*/
{
	char *coordinate_field_name, *xi_field_name, *command_field_name;
	int create_enabled,define_enabled,edit_enabled,motion_update_enabled,
		return_code,select_enabled, streaming_create_enabled, constrain_to_surface;
#if defined (WX_USER_INTERFACE)
	int element_dimension, element_create_enabled;
#endif /*(WX_USER_INTERFACE)*/
	struct Computed_field *coordinate_field, *command_field, *element_xi_field;
	struct Option_table *option_table;
	Cmiss_field_group_id group;
	Cmiss_region_id region;

	ENTER(Node_tool_execute_command_with_parse_state);
	if (state)
	{
		/* initialize defaults */
		coordinate_field=(struct Computed_field *)NULL;
		create_enabled=0;
		define_enabled=0;
		edit_enabled=0;
		motion_update_enabled=0;
		select_enabled=1;
		streaming_create_enabled = 0;
		constrain_to_surface = 0;
		command_field=(struct Computed_field *)NULL;
		element_xi_field=(struct Computed_field *)NULL;
		coordinate_field_name = NULL;
		xi_field_name = NULL;
		command_field_name = NULL;
		region = NULL;
		group = NULL;
#if defined (WX_USER_INTERFACE)
		if (node_tool && !node_tool->use_data)
		{
			element_create_enabled = 0;
			element_dimension = 2;
		}
#endif /*(WX_USER_INTERFACE)*/
		if (node_tool)
		{
			coordinate_field=Node_tool_get_coordinate_field(node_tool);
			if (coordinate_field)
			{
				coordinate_field_name = Cmiss_field_get_name(coordinate_field);
			}
			create_enabled=Node_tool_get_create_enabled(node_tool);
			define_enabled=Node_tool_get_define_enabled(node_tool);
			edit_enabled=Node_tool_get_edit_enabled(node_tool);
			motion_update_enabled=Node_tool_get_motion_update_enabled(node_tool);
			select_enabled=Node_tool_get_select_enabled(node_tool);
			streaming_create_enabled =
				 Node_tool_get_streaming_create_enabled(node_tool);
			constrain_to_surface =
				 Node_tool_get_constrain_to_surface(node_tool);
			command_field=Node_tool_get_command_field(node_tool);
			if (command_field)
			{
				command_field_name = Cmiss_field_get_name(command_field);
			}
			element_xi_field=Node_tool_get_element_xi_field(node_tool);
			if (element_xi_field)
			{
				xi_field_name = Cmiss_field_get_name(element_xi_field);
			}
#if defined (WX_USER_INTERFACE)
		if (node_tool && !node_tool->use_data)
		{
			 element_create_enabled = Node_tool_get_element_create_enabled(node_tool);
			 element_dimension =
					Node_tool_get_element_dimension(node_tool);
		}
#endif /*(WX_USER_INTERFACE)*/
		}
		region = Cmiss_region_access(node_tool->root_region);
		option_table=CREATE(Option_table)();
		/* coordinate_field */
		Option_table_add_entry(option_table,"coordinate_field",&coordinate_field_name,
			NULL,set_name);
		/* constrain_to_surfaces/no_constrain_to_surfaces */
		Option_table_add_switch(option_table,"constrain_to_surfaces","no_constrain_to_surfaces",
			&constrain_to_surface);
		/* create/no_create */
		Option_table_add_switch(option_table,"create","no_create",&create_enabled);
		/* define/no_define */
		Option_table_add_switch(option_table,"define","no_define",&define_enabled);
#if defined (WX_USER_INTERFACE)
		if (node_tool && !node_tool->use_data)
		{
			 /* create/no_create */
			 Option_table_add_switch(option_table,"element_create","no_element_create",&element_create_enabled);
			 /* element_dimension*/
			 Option_table_add_entry(option_table,"element_dimension",
					&element_dimension,NULL,set_int_non_negative);
		}
#endif /*(WX_USER_INTERFACE)*/
		/* edit/no_edit */
		Option_table_add_switch(option_table,"edit","no_edit",&edit_enabled);
		/* element_xi_field */
		Option_table_add_entry(option_table,"element_xi_field",&xi_field_name,
			NULL,set_name);
		/* group */
		Option_table_add_region_or_group_entry(option_table, "group", &region, &group);
		/* motion_update/no_motion_update */
		Option_table_add_switch(option_table,"motion_update","no_motion_update",
			&motion_update_enabled);
		/* select/no_select */
		Option_table_add_switch(option_table,"select","no_select",&select_enabled);
		/* streaming_create/no_streaming_create */
		Option_table_add_switch(option_table, "streaming_create",
			"no_streaming_create", &streaming_create_enabled);
		/* command_field */
		Option_table_add_entry(option_table,"command_field", &command_field_name,
			NULL, set_name);
		return_code = Option_table_multi_parse(option_table,state);
		if (return_code)
		{
			if (node_tool)
			{
				Node_tool_set_region(node_tool,region , group);
				if (node_tool->computed_field_manager)
				{
					if (coordinate_field_name)
					{
						coordinate_field = FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field, name)(
							coordinate_field_name, node_tool->computed_field_manager);
						if (coordinate_field &&
							Computed_field_has_up_to_3_numerical_components(coordinate_field, NULL))
						{
							Node_tool_set_coordinate_field(node_tool,coordinate_field);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Node_tool_execute_command.  Missing or invalid coordinate field");
						}
					}
					if (xi_field_name)
					{
						element_xi_field = FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field, name)(
							xi_field_name, node_tool->computed_field_manager);
						if (element_xi_field &&
							Computed_field_has_element_xi_fe_field(element_xi_field, NULL))
						{
							Node_tool_set_element_xi_field(node_tool,element_xi_field);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Node_tool_execute_command.  Missing or invalid element xi field");
						}
					}
					if (command_field_name)
					{
						command_field = FIND_BY_IDENTIFIER_IN_MANAGER(Computed_field, name)(
							command_field_name, node_tool->computed_field_manager);
						if (command_field &&
							Computed_field_has_string_value_type(command_field, NULL))
						{
							Node_tool_set_command_field(node_tool,command_field);
						}
						else
						{
							display_message(ERROR_MESSAGE,
								"Node_tool_execute_command.  Missing or invalid command field");
						}
					}
				}
				Node_tool_set_streaming_create_enabled(node_tool,
					streaming_create_enabled);
				/* Set the state after setting the parameters as some of them
				   states rely on these parameters */
				Node_tool_set_edit_enabled(node_tool,edit_enabled);
				Node_tool_set_select_enabled(node_tool,select_enabled);
				Node_tool_set_define_enabled(node_tool,define_enabled);
				Node_tool_set_create_enabled(node_tool,create_enabled);
				Node_tool_set_constrain_to_surface(node_tool,constrain_to_surface);
				Node_tool_set_motion_update_enabled(node_tool,motion_update_enabled);
#if defined (WX_USER_INTERFACE)
				if (!node_tool->use_data)
				{
					Node_tool_set_element_dimension(node_tool,element_dimension);
					Node_tool_set_element_create_enabled(node_tool,element_create_enabled);
				}
#endif /*(WX_USER_INTERFACE)*/
			}
		} /* parse error,help */
		DESTROY(Option_table)(&option_table);
		Cmiss_region_destroy(&region);
		if (group)
		{
			Cmiss_field_group_destroy(&group);
		}
		if (coordinate_field_name)
			DEALLOCATE(coordinate_field_name);
		if (xi_field_name)
			DEALLOCATE(xi_field_name);
		if (command_field_name)
			DEALLOCATE(command_field_name);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"execute_command_gfx_node_tool.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* execute_command_gfx_node_tool */
