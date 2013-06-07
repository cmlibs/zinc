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

#include <algorithm>

#include "zinc/status.h"
#include "zinc/graphicsfilter.h"
#include "zinc/graphicsmodule.h"
#include "zinc/scene.h"
#include "finite_element/finite_element_region.h"
#include "general/debug.h"
#include "general/object.h"
#include "graphics/graphics_library.h"
#include "graphics/render_gl.h"
#include "graphics/scene.h"
#include "graphics/scene_picker.hpp"
#include "graphics/scene_viewer.h"
#include "graphics/rendition.h"
#include "region/cmiss_region.h"

#define SELECT_BUFFER_SIZE_INCREMENT 10000

Cmiss_scene_picker::Cmiss_scene_picker(Cmiss_graphics_module_id graphics_module_in) :
	interaction_volume(0),
	scene(0),
	scene_viewer(0),
	centre_x(0), centre_y(0),
	size_x(0), size_y(0),
	coordinate_system(CMISS_GRAPHICS_COORDINATE_SYSTEM_WINDOW_PIXEL_TOP_LEFT),
	filter(0),
	select_buffer(0),
	select_buffer_size(10000),
	number_of_hits(0),
	graphics_module(Cmiss_graphics_module_access(graphics_module_in)),
	access_count(1)
{
}

Cmiss_scene_picker::~Cmiss_scene_picker()
{
	if (interaction_volume)
		DEACCESS(Interaction_volume)(&interaction_volume);
	if (scene_viewer)
		Cmiss_scene_viewer_destroy(&scene_viewer);
	if (scene)
		Cmiss_scene_destroy(&scene);
	if (filter)
		Cmiss_graphics_filter_destroy(&filter);
	if (select_buffer)
		DEALLOCATE(select_buffer);
	if (graphics_module)
		Cmiss_graphics_module_destroy(&graphics_module);
}

void Cmiss_scene_picker::updateViewerRectangle()
{
	if (scene_viewer)
	{
		if (interaction_volume)
			DEACCESS(Interaction_volume)(&interaction_volume);
		GLdouble temp_modelview_matrix[16], temp_projection_matrix[16];
		double viewport_bottom,viewport_height, viewport_left,viewport_width,
			viewport_pixels_per_unit_x, viewport_pixels_per_unit_y;
		int i, j, width, height;
		for (i=0;i<4;i++)
		{
			for (j=0;j<4;j++)
			{
				temp_modelview_matrix[i*4+j] =
					scene_viewer->modelview_matrix[j*4+i];
				temp_projection_matrix[i*4+j] =
					scene_viewer->window_projection_matrix[j*4+i];
			}
		}

		Scene_viewer_get_viewport_info(scene_viewer,
			&viewport_left, &viewport_bottom, &viewport_pixels_per_unit_x,
			&viewport_pixels_per_unit_y);
		Scene_viewer_get_viewport_size(scene_viewer,	&width, &height);
		viewport_width  = ((double)width)/viewport_pixels_per_unit_x;
		viewport_height = ((double)height)/viewport_pixels_per_unit_y;
		/* recalculate the centre of y as windows coordinate is different from the
		 * GL coordinate */
		double new_centre_y = viewport_height - (double)(centre_y) - 1.0;
		interaction_volume = create_Interaction_volume_ray_frustum(
			temp_modelview_matrix, temp_projection_matrix,
			viewport_left, viewport_bottom, viewport_width, viewport_height,
			centre_x, new_centre_y, size_x, size_y);
	}
}

int Cmiss_scene_picker::pickObjects()
{
	double modelview_matrix[16],projection_matrix[16];
	GLdouble opengl_modelview_matrix[16],opengl_projection_matrix[16];
	int i, j, return_code = CMISS_ERROR_GENERAL;
	updateViewerRectangle();
	if (select_buffer != NULL)
		return CMISS_OK;
	if (!has_current_context())
		return CMISS_ERROR_GENERAL;
	if (scene&&interaction_volume)
	{
		Render_graphics_opengl *renderer = Render_graphics_opengl_create_glbeginend_renderer();
		renderer->picking = 1;
		Cmiss_graphics_filter_id scene_filter = Cmiss_scene_get_filter(scene);
		Cmiss_graphics_filter_id combined_filter = scene_filter;
		if (filter)
		{
			combined_filter = Cmiss_graphics_module_create_filter_operator_and(
				graphics_module);
			Cmiss_graphics_filter_operator_id and_filter = Cmiss_graphics_filter_cast_operator(
				combined_filter);
			if (and_filter)
			{
				Cmiss_graphics_filter_operator_append_operand(and_filter, scene_filter);
				Cmiss_graphics_filter_operator_append_operand(and_filter, filter);
				Cmiss_graphics_filter_operator_destroy(&and_filter);
			}
		}
		if (renderer->Scene_compile(scene, combined_filter))
		{
			number_of_hits=-1;
			while (0>number_of_hits)
			{
				if (ALLOCATE(select_buffer,GLuint,select_buffer_size))
				{
					Interaction_volume_get_modelview_matrix(interaction_volume,
						modelview_matrix);
					Interaction_volume_get_projection_matrix(interaction_volume,
						projection_matrix);
					/* transpose projection matrix for OpenGL */
					for (i=0;i<4;i++)
					{
						for (j=0;j<4;j++)
						{
							opengl_modelview_matrix[j*4+i] = modelview_matrix[i*4+j];
							opengl_projection_matrix[j*4+i] = projection_matrix[i*4+j];
						}
					}
					renderer->set_world_view_matrix(opengl_modelview_matrix);

					glSelectBuffer(select_buffer_size,select_buffer);
					glRenderMode(GL_SELECT);
					glMatrixMode(GL_PROJECTION);
					glLoadIdentity();
					glMultMatrixd(opengl_projection_matrix);
					glMatrixMode(GL_MODELVIEW);
					glLoadIdentity();
					glMultMatrixd(opengl_modelview_matrix);
					/* set an arbitrary viewport - not really needed
						   SAB 22 July 2004 This is causing the view frustrums
						   to not match when picking, so instead I am not changing the
						   viewport, so presumably the last rendered viewport is OK. */
					/* glViewport(0,0,1024,1024); */
					glDepthRange((GLclampd)0,(GLclampd)1);
					{
						do
						{
							return_code = renderer->Scene_execute(scene);
						}
						while (return_code && renderer->next_layer());
					}
					glFlush();
					number_of_hits=glRenderMode(GL_RENDER);
					if (0<=number_of_hits)
					{
						return_code=CMISS_OK;
					}
					else
					{
						/* select buffer overflow; enlarge and repeat */
						select_buffer_size += SELECT_BUFFER_SIZE_INCREMENT;
						DEALLOCATE(select_buffer);
					}
				}
			}
		}
		if (combined_filter && (combined_filter !=scene_filter))
		{
			Cmiss_graphics_filter_destroy(&combined_filter);
		}
		if (scene_filter)
		{
			Cmiss_graphics_filter_destroy(&scene_filter);
		}

		delete renderer;
	}
	return return_code;
}

Region_node_map Cmiss_scene_picker::getPickedRegionSortedNodes(
	enum Cmiss_scene_picker_object_type type)
{
	Region_node_map node_map;

	if ((CMISS_OK == pickObjects()) && select_buffer)
	{
		int hit_no, number_of_names;
		GLuint *select_buffer_ptr = 0, *next_select_buffer = select_buffer;
		Cmiss_rendition_id rendition = 0, existing_rendition = 0;
		Cmiss_graphic_id graphic = 0;
		Cmiss_nodeset_id nodeset = 0;
		for (hit_no=0;(hit_no<number_of_hits);hit_no++)
		{
			/* select_buffer[0] = number_of_names
			 * select_buffer[1] = nearest
			 * select_buffer[2] = furthest
			 * select_buffer[3] = rendition
			 * select_buffer[4] = graphic position
			 * select_buffer[5] = element number
			 * select_buffer[6] = point number
			 */
			select_buffer_ptr = next_select_buffer;
			number_of_names=(int)(select_buffer_ptr[0]);
			next_select_buffer = select_buffer_ptr + number_of_names + 3;
			if (number_of_names >= 4)
			{
				if ((getRenditionAndGraphic(select_buffer_ptr,
						&rendition, &graphic) && (0 != rendition) && (0 != graphic)))
				{
					if (((type == CMISS_SCENE_PICKER_OBJECT_DATA) &&
							(CMISS_GRAPHIC_DATA_POINTS == Cmiss_graphic_get_graphic_type(graphic))) ||
						((type == CMISS_SCENE_PICKER_OBJECT_NODE) &&
							(CMISS_GRAPHIC_NODE_POINTS == Cmiss_graphic_get_graphic_type(graphic))))
					{
						if (rendition)
						{
							Cmiss_region *region = Cmiss_rendition_get_region(rendition);
							if (existing_rendition != rendition)
							{
								existing_rendition = rendition;
								Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
								if (nodeset)
									Cmiss_nodeset_destroy(&nodeset);
								if (type == CMISS_SCENE_PICKER_OBJECT_DATA)
								{
									nodeset = Cmiss_field_module_find_nodeset_by_name(
										field_module, "cmiss_data");
								}
								else
								{
									nodeset = Cmiss_field_module_find_nodeset_by_name(
										field_module, "cmiss_nodes");
								}
								Cmiss_field_module_destroy(&field_module);
							}
							Cmiss_node_id node = Cmiss_nodeset_find_node_by_identifier(nodeset,
								(int)(select_buffer_ptr[6]));
							if (node)
							{
								node_map.insert(std::make_pair(region, node));
								Cmiss_node_destroy(&node);
							}
						}
					}
					Cmiss_graphic_destroy(&graphic);
				}
			}
		}
		if (nodeset)
			Cmiss_nodeset_destroy(&nodeset);
	}
	return node_map;
}

Region_element_map Cmiss_scene_picker::getPickedRegionSortedElements()
{
	Region_element_map element_map;

	if ((CMISS_OK == pickObjects()) && select_buffer)
	{
		int hit_no, number_of_names;
		GLuint *select_buffer_ptr = 0, *next_select_buffer = select_buffer;
		Cmiss_rendition_id rendition = 0, existing_rendition = 0;
		Cmiss_graphic_id graphic = 0;
		Cmiss_mesh_id mesh = 0;
		int current_element_type = 0;
		for (hit_no=0;(hit_no<number_of_hits);hit_no++)
		{
			/* select_buffer[0] = number_of_names
			 * select_buffer[1] = nearest
			 * select_buffer[2] = furthest
			 * select_buffer[3] = rendition
			 * select_buffer[4] = graphic position
			 * select_buffer[5] = element number
			 * select_buffer[6] = point number
			 */
			select_buffer_ptr = next_select_buffer;
			number_of_names=(int)(select_buffer_ptr[0]);
			next_select_buffer = select_buffer_ptr + number_of_names + 3;
			if (number_of_names >= 3)
			{
				if ((getRenditionAndGraphic(select_buffer_ptr,
					&rendition, &graphic) && (0 != rendition) && (0 != graphic)))
				{
					if (Cmiss_graphic_selects_elements(graphic))
					{
						if (rendition)
						{
							Cmiss_region *region = Cmiss_rendition_get_region(rendition);
							struct FE_region *fe_region = Cmiss_region_get_FE_region(region);
							int element_type = Cmiss_graphic_get_dimension(graphic, fe_region);
							if ((existing_rendition != rendition) ||
								(current_element_type != element_type))
							{
								existing_rendition = rendition;
								current_element_type = element_type;
								Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
								if (mesh)
									Cmiss_mesh_destroy(&mesh);
								mesh = Cmiss_field_module_find_mesh_by_dimension(field_module, current_element_type);
								Cmiss_field_module_destroy(&field_module);
							}
							Cmiss_element_id element = Cmiss_mesh_find_element_by_identifier(mesh,
								(int)(select_buffer_ptr[5]));
							if (element)
							{
								element_map.insert(std::make_pair(region, element));
								Cmiss_element_destroy(&element);
							}
						}
					}
					Cmiss_graphic_destroy(&graphic);
				}
			}
		}
		Cmiss_mesh_destroy(&mesh);
	}
	return element_map;
}

void Cmiss_scene_picker::reset()
{
	if (select_buffer)
	{
		DEALLOCATE(select_buffer);
		select_buffer = NULL;
	}
	select_buffer_size = 10000;
	number_of_hits = 0;
}

/*provide a select buffer pointer and return the rendition and graphic */
int Cmiss_scene_picker::getRenditionAndGraphic(GLuint *select_buffer_ptr,
	Cmiss_rendition_id *rendition, Cmiss_graphic_id *graphic)
{
	if (scene && select_buffer_ptr)
	{
		*rendition = Scene_get_rendition_of_position(scene, (int)(select_buffer_ptr[3]));
		*graphic = Cmiss_rendition_get_graphic_at_position(*rendition,
			(int)(select_buffer_ptr[4]));
		return 1;
	}
	else
	{
		*rendition = 0;
		*graphic = 0;
		return 0;
	}
}

int Cmiss_scene_picker::setGraphicsFilter(Cmiss_graphics_filter_id filter_in)
{
	if (filter_in != filter)
	{
		reset();
		if (filter)
			Cmiss_graphics_filter_destroy(&filter);
		if (filter_in)
			filter = Cmiss_graphics_filter_access(filter_in);
	}
	return CMISS_OK;
}


int Cmiss_scene_picker::setScene(Cmiss_scene_id scene_in)
{
	if (scene_in)
	{
		if (scene_in != scene)
		{
			reset();
			if (scene)
				Cmiss_scene_destroy(&scene);
			scene = Cmiss_scene_access(scene_in);
		}
		return CMISS_OK;
	}
	return CMISS_ERROR_ARGUMENT;
}

int Cmiss_scene_picker::setSceneViewerRectangle(Cmiss_scene_viewer_id scene_viewer_in,
	enum Cmiss_graphics_coordinate_system coordinate_system_in, double x1,
	double y1, double x2, double y2)
{
	reset();
	if (scene_viewer_in)
	{
		if (scene_viewer)
			Cmiss_scene_viewer_destroy(&scene_viewer);
		coordinate_system = coordinate_system_in;
		scene_viewer = Cmiss_scene_viewer_access(scene_viewer_in);
		size_x = x2 - x1;
		size_y = y2 - y1;
		centre_x = x1 + size_x/2;
		centre_y = y1 + size_y/2;
		return CMISS_OK;
	}
	return CMISS_ERROR_ARGUMENT;
}

int Cmiss_scene_picker::setInteractionVolume(struct Interaction_volume *interaction_volume_in)
{
	reset();
	if (scene_viewer)
		Cmiss_scene_viewer_destroy(&scene_viewer);
	size_x = 0;
	size_y = 0;
	centre_x = 0;
	centre_y = 0;
	REACCESS(Interaction_volume)(&interaction_volume, interaction_volume_in);
	return 1;
}

Cmiss_element_id Cmiss_scene_picker::getNearestElement()
{
	Cmiss_element_id nearest_element = 0;
	if ((CMISS_OK == pickObjects()) && select_buffer)
	{
		int hit_no, number_of_names, current_element_type = 0;
		GLuint *select_buffer_ptr = 0, *next_select_buffer = select_buffer;
		double current_nearest = 0, nearest = 0;
		Cmiss_rendition_id rendition = 0, existing_rendition = 0;
		Cmiss_graphic_id graphic = 0;
		Cmiss_mesh_id mesh = 0;
		for (hit_no=0;(hit_no<number_of_hits);hit_no++)
		{
			/* select_buffer[0] = number_of_names
			 * select_buffer[1] = nearest
			 * select_buffer[2] = furthest
			 * select_buffer[3] = rendition
			 * select_buffer[4] = graphic position
			 * select_buffer[5] = element number
			 * select_buffer[6] = point number
			 */
			select_buffer_ptr = next_select_buffer;
			number_of_names=(int)(select_buffer_ptr[0]);
			next_select_buffer = select_buffer_ptr + number_of_names + 3;
			if (number_of_names >= 3)
			{
				/* get range of depth of picked object */
				/*???RC OpenGL Programming Guide p361 says depth values are
					made into integers from 0 to 2^32-1. Just convert to
						doubles for now */
				nearest = (double)(select_buffer_ptr[1]);
				if ((nearest_element == NULL) || (nearest < current_nearest))
				{
					if ((getRenditionAndGraphic(select_buffer_ptr,
						&rendition, &graphic) && (0 != rendition) && (0 != graphic)))
					{
						if (Cmiss_graphic_selects_elements(graphic))
						{
							if (rendition)
							{
								Cmiss_region *region = Cmiss_rendition_get_region(rendition);
								struct FE_region *fe_region = Cmiss_region_get_FE_region(region);
								int element_type = Cmiss_graphic_get_dimension(graphic, fe_region);
								if ((existing_rendition != rendition) ||
									(current_element_type != element_type))
								{
									existing_rendition = rendition;
									current_element_type = element_type;
									Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
									if (mesh)
										Cmiss_mesh_destroy(&mesh);
									mesh = Cmiss_field_module_find_mesh_by_dimension(field_module, current_element_type);
									Cmiss_field_module_destroy(&field_module);
								}

								Cmiss_element_id element = Cmiss_mesh_find_element_by_identifier(mesh,
									(int)(select_buffer_ptr[5]));
								if (element)
								{
									if (nearest_element)
										Cmiss_element_destroy(&nearest_element);
									nearest_element = element;
									current_nearest = nearest;
								}
							}
						}
						Cmiss_graphic_destroy(&graphic);
					}
				}
			}
		}
		if (mesh)
			Cmiss_mesh_destroy(&mesh);
	}
	return nearest_element;
}

Cmiss_node_id Cmiss_scene_picker::getNearestNode(enum Cmiss_scene_picker_object_type type)
{
	Cmiss_node_id nearest_node = 0;
	if ((CMISS_OK == pickObjects()) && select_buffer)
	{
		int hit_no, number_of_names;
		GLuint *select_buffer_ptr = 0, *next_select_buffer = select_buffer;
		double current_nearest = 0, nearest = 0;
		Cmiss_rendition_id rendition = 0, existing_rendition = 0;
		Cmiss_graphic_id graphic = 0;
		Cmiss_nodeset_id nodeset = 0;
		for (hit_no=0;(hit_no<number_of_hits);hit_no++)
		{
			/* select_buffer[0] = number_of_names
			 * select_buffer[1] = nearest
			 * select_buffer[2] = furthest
			 * select_buffer[3] = rendition
			 * select_buffer[4] = graphic position
			 * select_buffer[5] = element number
			 * select_buffer[6] = point number
			 */
			select_buffer_ptr = next_select_buffer;
			number_of_names=(int)(select_buffer_ptr[0]);
			next_select_buffer = select_buffer_ptr + number_of_names + 3;
			if (number_of_names >= 4)
			{
				/* get range of depth of picked object */
				/*???RC OpenGL Programming Guide p361 says depth values are
					made into integers from 0 to 2^32-1. Just convert to
						doubles for now */
				nearest = (double)(select_buffer_ptr[1]);
				if ((nearest_node == NULL) || (nearest < current_nearest))
				{
					if ((getRenditionAndGraphic(select_buffer_ptr,
						&rendition, &graphic) && (0 != rendition) && (0 != graphic)))
					{
						if (((type == CMISS_SCENE_PICKER_OBJECT_DATA) &&
								(CMISS_GRAPHIC_DATA_POINTS == Cmiss_graphic_get_graphic_type(graphic))) ||
							((type == CMISS_SCENE_PICKER_OBJECT_NODE) &&
								(CMISS_GRAPHIC_NODE_POINTS == Cmiss_graphic_get_graphic_type(graphic))))
						{
							if (rendition)
							{
								if (existing_rendition != rendition)
								{
									existing_rendition = rendition;
									Cmiss_region *region = Cmiss_rendition_get_region(rendition);
									Cmiss_field_module_id field_module = Cmiss_region_get_field_module(region);
									if (nodeset)
										Cmiss_nodeset_destroy(&nodeset);
									if (type == CMISS_SCENE_PICKER_OBJECT_DATA)
									{
										nodeset = Cmiss_field_module_find_nodeset_by_name(
											field_module, "cmiss_data");
									}
									else
									{
										nodeset = Cmiss_field_module_find_nodeset_by_name(
											field_module, "cmiss_nodes");
									}
									Cmiss_field_module_destroy(&field_module);
								}
								Cmiss_node_id node = Cmiss_nodeset_find_node_by_identifier(nodeset,
									(int)(select_buffer_ptr[6]));
								if (node)
								{
									if (nearest_node)
										Cmiss_node_destroy(&nearest_node);
									nearest_node = node;
									current_nearest = nearest;
								}
							}
						}
						Cmiss_graphic_destroy(&graphic);
					}
				}
			}
		}
		if (nodeset)
			Cmiss_nodeset_destroy(&nodeset);
	}
	return nearest_node;
}

Cmiss_graphic_id Cmiss_scene_picker::getNearestGraphic(enum Cmiss_scene_picker_object_type type)
{
	if ((CMISS_OK == pickObjects()) && select_buffer)
	{
		int hit_no, number_of_names;
		GLuint *select_buffer_ptr = 0, *next_select_buffer = select_buffer;
		double current_nearest = 0, nearest = 0;
		Cmiss_rendition_id rendition = 0;
		Cmiss_graphic_id graphic = 0, nearest_graphic = 0;
		for (hit_no=0;(hit_no<number_of_hits);hit_no++)
		{
			/* select_buffer[0] = number_of_names
			 * select_buffer[1] = nearest
			 * select_buffer[2] = furthest
			 * select_buffer[3] = rendition
			 * select_buffer[4] = graphic position
			 * select_buffer[5] = element number
			 * select_buffer[6] = point number
			 */
			select_buffer_ptr = next_select_buffer;
			number_of_names=(int)(select_buffer_ptr[0]);
			next_select_buffer = select_buffer_ptr + number_of_names + 3;
			if (number_of_names >= 2)
			{
				/* get range of depth of picked object */
				/*???RC OpenGL Programming Guide p361 says depth values are
					made into integers from 0 to 2^32-1. Just convert to
						doubles for now */
				nearest = (double)(select_buffer_ptr[1]);
				if ((nearest_graphic == NULL) || (nearest < current_nearest))
				{
					if ((getRenditionAndGraphic(select_buffer_ptr,
						&rendition, &graphic) && (0 != rendition) && (0 != graphic)))
					{
						if ((type == CMISS_SCENE_PICKER_OBJECT_ANY) ||
							((type == CMISS_SCENE_PICKER_OBJECT_ELEMENT) &&
								(Cmiss_graphic_selects_elements(graphic))) ||
							((type == CMISS_SCENE_PICKER_OBJECT_DATA) &&
								(CMISS_GRAPHIC_DATA_POINTS == Cmiss_graphic_get_graphic_type(graphic))) ||
							((type == CMISS_SCENE_PICKER_OBJECT_NODE) &&
								(CMISS_GRAPHIC_NODE_POINTS == Cmiss_graphic_get_graphic_type(graphic))))
						{
							current_nearest = nearest;
							if (graphic != nearest_graphic)
								REACCESS(Cmiss_graphic)(&nearest_graphic, graphic);
						}
						Cmiss_graphic_destroy(&graphic);
					}
				}
			}
		}
		return nearest_graphic;
	}
	return 0;
}

int Cmiss_scene_picker::addPickedElementsToGroup(Cmiss_field_group_id group)
{
	if (group)
	{
		Region_element_map element_map = getPickedRegionSortedElements();
		if (!element_map.empty())
		{
			Cmiss_region *sub_region = NULL;
			Cmiss_field_group_id selection_group = NULL;
			Region_element_map::iterator pos;
			Cmiss_mesh_group_id mesh_group[MAXIMUM_ELEMENT_XI_DIMENSIONS];
			int iter = 0;
			for (iter = 0; iter < MAXIMUM_ELEMENT_XI_DIMENSIONS; iter++)
			{
				mesh_group[iter] = 0;
			}
			for (pos = element_map.begin(); pos != element_map.end(); ++pos)
			{
				if (pos->first != sub_region)
				{
					if (selection_group)
					{
						Cmiss_field_group_destroy(&selection_group);
					}
					for (iter = 0; iter < MAXIMUM_ELEMENT_XI_DIMENSIONS; iter++)
					{
						if (mesh_group[iter])
						{
							Cmiss_mesh_group_destroy(&(mesh_group[iter]));
						}
					}
					sub_region = pos->first;
					if (sub_region)
					{
						selection_group = Cmiss_field_group_get_subregion_group(group, sub_region);
						if (!selection_group)
						{
							selection_group = Cmiss_field_group_create_subregion_group(group, sub_region);
						}
					}
				}
				if (sub_region && selection_group)
				{
					Cmiss_field_module_id field_module = Cmiss_region_get_field_module(sub_region);
					int dimension = Cmiss_element_get_dimension(pos->second);
					if (dimension <= MAXIMUM_ELEMENT_XI_DIMENSIONS)
					{
						if (!mesh_group[dimension - 1])
						{
							Cmiss_mesh_id temp_mesh =
								Cmiss_field_module_find_mesh_by_dimension(field_module, dimension);
							Cmiss_field_element_group_id element_group =
								Cmiss_field_group_get_element_group(selection_group, temp_mesh);
							if (!element_group)
							{
								element_group = Cmiss_field_group_create_element_group(selection_group, temp_mesh);
							}
							mesh_group[dimension - 1] = Cmiss_field_element_group_get_mesh(element_group);
							Cmiss_field_element_group_destroy(&element_group);
							Cmiss_mesh_destroy(&temp_mesh);
						}
						Cmiss_mesh_group_add_element(mesh_group[dimension - 1], pos->second);
					}
					Cmiss_field_module_destroy(&field_module);
				}
			}
			if (selection_group)
			{
				Cmiss_field_group_destroy(&selection_group);
			}
			for (iter = 0; iter < MAXIMUM_ELEMENT_XI_DIMENSIONS; iter++)
			{
				if (mesh_group[iter])
				{
					Cmiss_mesh_group_destroy(&(mesh_group[iter]));
				}
			}
		}
	}
	else
	{
		return CMISS_ERROR_ARGUMENT;
	}
	return CMISS_OK;
}

int Cmiss_scene_picker::addPickedNodesToGroup(Cmiss_field_group_id group,
	enum Cmiss_scene_picker_object_type type)
{
	if (group)
	{
		Region_node_map node_map = getPickedRegionSortedNodes(type);
		if (!node_map.empty())
		{
			Cmiss_region *sub_region = NULL;
			Cmiss_field_group_id selection_group = NULL;
			Cmiss_nodeset_group_id nodeset_group = NULL;
			Region_node_map::iterator pos;
			for (pos = node_map.begin(); pos != node_map.end(); ++pos)
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
					sub_region = pos->first;
					if (sub_region)
					{
						selection_group = Cmiss_field_group_get_subregion_group(group, sub_region);
						if (!selection_group)
						{
							selection_group = Cmiss_field_group_create_subregion_group(group, sub_region);
						}
					}
					if (selection_group)
					{
						Cmiss_field_module_id field_module = Cmiss_region_get_field_module(sub_region);
						Cmiss_nodeset_id master_nodeset = 0;
						if (type == CMISS_SCENE_PICKER_OBJECT_DATA)
						{
							master_nodeset = Cmiss_field_module_find_nodeset_by_name(
								field_module, "cmiss_data");
						}
						else
						{
							master_nodeset = Cmiss_field_module_find_nodeset_by_name(
								field_module, "cmiss_nodes");
						}
						if (master_nodeset)
						{
							Cmiss_field_node_group_id node_group = Cmiss_field_group_get_node_group(
								selection_group, master_nodeset);
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
		}
	}
	else
	{
		return CMISS_ERROR_ARGUMENT;
	}
	return CMISS_OK;
}

int DESTROY(Cmiss_scene_picker)(struct Cmiss_scene_picker **scene_picker_address)
{
	if (scene_picker_address && (*scene_picker_address))
	{
		delete *scene_picker_address;
		*scene_picker_address = NULL;
		return 1;
	}
	else
	{
		return 0;
	}
}

Cmiss_scene_picker_id Cmiss_scene_picker_create(Cmiss_graphics_module *graphics_module)
{
	return new Cmiss_scene_picker(graphics_module);
}

DECLARE_OBJECT_FUNCTIONS(Cmiss_scene_picker);

Cmiss_scene_picker_id Cmiss_scene_picker_access(
	Cmiss_scene_picker_id scene_picker)
{
	if (scene_picker)
		return scene_picker->access();
	return 0;
}

int Cmiss_scene_picker_destroy(Cmiss_scene_picker_id *scene_picker_address)
{
	return DEACCESS(Cmiss_scene_picker)(scene_picker_address);
}

int Cmiss_scene_picker_set_scene(Cmiss_scene_picker_id scene_picker,
	Cmiss_scene_id scene)
{
	return scene_picker->setScene(scene);
}

int Cmiss_scene_picker_set_graphics_filter(Cmiss_scene_picker_id scene_picker,
	Cmiss_graphics_filter_id filter_in)
{
	return scene_picker->setGraphicsFilter(filter_in);
}

int Cmiss_scene_picker_set_scene_viewer_rectangle(
	Cmiss_scene_picker_id scene_picker, Cmiss_scene_viewer_id scene_viewer_in,
	enum Cmiss_graphics_coordinate_system coordinate_system_in, double x1,
		double y1, double x2, double y2)
{
	return scene_picker->setSceneViewerRectangle(scene_viewer_in,
		coordinate_system_in, x1, y1, x2, y2);
}

Cmiss_element_id Cmiss_scene_picker_get_nearest_element(Cmiss_scene_picker_id scene_picker)
{
	return scene_picker->getNearestElement();
}

Cmiss_node_id Cmiss_scene_picker_get_nearest_data(Cmiss_scene_picker_id scene_picker)
{
	return scene_picker->getNearestNode(CMISS_SCENE_PICKER_OBJECT_DATA);
}

Cmiss_node_id Cmiss_scene_picker_get_nearest_node(Cmiss_scene_picker_id scene_picker)
{
	return scene_picker->getNearestNode(CMISS_SCENE_PICKER_OBJECT_NODE);
}

Cmiss_graphic_id Cmiss_scene_picker_get_nearest_graphic(Cmiss_scene_picker_id scene_picker)
{
	return scene_picker->getNearestGraphic(CMISS_SCENE_PICKER_OBJECT_ANY);
}

Cmiss_graphic_id Cmiss_scene_picker_get_nearest_data_graphic(Cmiss_scene_picker_id scene_picker)
{
	return scene_picker->getNearestGraphic(CMISS_SCENE_PICKER_OBJECT_DATA);
}

Cmiss_graphic_id Cmiss_scene_picker_get_nearest_element_graphic(Cmiss_scene_picker_id scene_picker)
{
	return scene_picker->getNearestGraphic(CMISS_SCENE_PICKER_OBJECT_ELEMENT);
}

Cmiss_graphic_id Cmiss_scene_picker_get_nearest_node_graphic(Cmiss_scene_picker_id scene_picker)
{
	return scene_picker->getNearestGraphic(CMISS_SCENE_PICKER_OBJECT_NODE);
}

int Cmiss_scene_picker_add_picked_data_to_group(Cmiss_scene_picker_id scene_picker,
	Cmiss_field_group_id group)
{
	return scene_picker->addPickedNodesToGroup(group, CMISS_SCENE_PICKER_OBJECT_DATA);
}

int Cmiss_scene_picker_add_picked_elements_to_group(Cmiss_scene_picker_id scene_picker,
	Cmiss_field_group_id group)
{
	return scene_picker->addPickedElementsToGroup(group);
}

int Cmiss_scene_picker_add_picked_nodes_to_group(Cmiss_scene_picker_id scene_picker,
	Cmiss_field_group_id group)
{
	return scene_picker->addPickedNodesToGroup(group, CMISS_SCENE_PICKER_OBJECT_NODE);
}

int Cmiss_scene_picker_set_interaction_volume(Cmiss_scene_picker_id scene_picker,
	struct Interaction_volume *interaction_volume)
{
	return scene_picker->setInteractionVolume(interaction_volume);
}
