/***************************************************************************//**
 * render_triangularisation.cpp
 * Rendering calls - Non API specific.
 */
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

extern "C" {
#include "general/debug.h"
#include "graphics/auxiliary_graphics_types.h"
#include "graphics/graphical_element.h"
#include "graphics/graphics_object.h"
#include "graphics/scene.h"
}
#include "graphics/triangle_mesh.hpp"
#include "graphics/graphics_object_private.hpp"
#include "graphics/scene.hpp"
#include "graphics/render_triangularisation.hpp"

#include <math.h>

int write_scene_triangle_mesh(Triangle_mesh& trimesh, struct Scene *scene);

int draw_voltex_triangle_mesh(Triangle_mesh& trimesh,
	int number_of_vertices, struct VT_iso_vertex **vertex_list,
	int number_of_triangles, struct VT_iso_triangle **triangle_list,
	int number_of_data_components,
	struct Graphical_material *material, struct Spectrum *spectrum)
{
	int i, index1, index2, index3, return_code;
	Triple v1, v2, v3;

	ENTER(draw_voltex_stl);
	/* Keep a similar interface to all the other render implementations */
	USE_PARAMETER(number_of_vertices);
	USE_PARAMETER(number_of_data_components);
	USE_PARAMETER(material);
	USE_PARAMETER(spectrum);
	return_code=0;
	if (triangle_list && vertex_list && (0<number_of_triangles))
	{
		for (i=0;i<number_of_triangles;i++)
		{
			index1 = triangle_list[i]->vertices[0]->index;
			index2 = triangle_list[i]->vertices[1]->index;
			index3 = triangle_list[i]->vertices[2]->index;
			v1[0] = vertex_list[index1]->coordinates[0];
			v1[1] = vertex_list[index1]->coordinates[1];
			v1[2] = vertex_list[index1]->coordinates[2];
			v2[0] = vertex_list[index2]->coordinates[0];
			v2[1] = vertex_list[index2]->coordinates[1];
			v2[2] = vertex_list[index2]->coordinates[2];
			v3[0] = vertex_list[index3]->coordinates[0];
			v3[1] = vertex_list[index3]->coordinates[1];
			v3[2] = vertex_list[index3]->coordinates[2];
			trimesh.add_triangle_coordinates(v1, v2, v3);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"draw_voltex_trimesh.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* draw_voltex_trimesh */

int draw_surface_triangle_mesh(Triangle_mesh& trimesh, Triple *surfpts,
	Triple *normalpts, Triple *texturepts, int npts1,int npts2,
	enum GT_surface_type surface_type, gtPolygonType polygon_type,
	int number_of_data_components, GTDATA *data,
	struct Graphical_material *material, struct Spectrum *spectrum)
{
	int return_code = 0;
	Triple *point = NULL;

	ENTER(draw_surface_triangle_mesh);
	/* Keep a similar interface to all the other render implementations */
	USE_PARAMETER(normalpts);
	USE_PARAMETER(texturepts);
	USE_PARAMETER(number_of_data_components);
	USE_PARAMETER(data);
	USE_PARAMETER(material);
	USE_PARAMETER(spectrum);
	if (surfpts && (1<npts1) && (1<npts2))
	{
		point=surfpts;
		switch (surface_type)
		{
			case g_SHADED:
			case g_SHADED_TEXMAP:
			case g_WIREFRAME_SHADED_TEXMAP:
			{
				int number_of_points = npts1*npts2;
				if (polygon_type == g_TRIANGLE)
				{
					number_of_points = npts2*(npts2+1)/2;
				}
				const Triangle_vertex **tri_vertex = new const Triangle_vertex*[number_of_points];
				for (int i = 0; i < number_of_points; i++)
				{
					tri_vertex[i] = trimesh.add_vertex(*point);
					point++;
				}
				point = surfpts;
				switch (polygon_type)
				{
					case g_QUADRILATERAL:
					{
						for (int j = 0; j < npts2-1; j++)
						{
							for (int i = 0; i < npts1-1; i++)
							{
								trimesh.add_quadrilateral(
									tri_vertex[j*npts1+i],
									tri_vertex[j*npts1+i+1],
									tri_vertex[(j+1)*npts1+i],
									tri_vertex[(j+1)*npts1+i+1]);
							}
						}
						return_code=1;
					} break;
					case g_TRIANGLE:
					{
						for (int j = 0; j < npts2-1; j++)
						{
							for (int i = 0; i < j-1; i++)
							{
								trimesh.add_triangle(
									tri_vertex[j*npts2+i],
									tri_vertex[j*npts2+i+1],
									tri_vertex[(j+1)*npts2+i]);
								trimesh.add_triangle(
									tri_vertex[j*npts2+i+1],
									tri_vertex[(j+1)*npts2+i+1],
									tri_vertex[(j+1)*npts2+i]);
							}
						}
						return_code=1;
					} break;
					default:
					{
						return_code = 1;
					}
				}
				delete[] tri_vertex;
			} break;
			case g_SH_DISCONTINUOUS:
			case g_SH_DISCONTINUOUS_STRIP:
			case g_SH_DISCONTINUOUS_TEXMAP:
			case g_SH_DISCONTINUOUS_STRIP_TEXMAP:
			{
				int number_of_points = npts1*npts2;
				if (polygon_type == g_TRIANGLE)
				{
					number_of_points = npts2*(npts2+1)/2;
				}
				const Triangle_vertex **tri_vertex = new const Triangle_vertex*[number_of_points];
				for (int i = 0; i < number_of_points; i++)
				{
					tri_vertex[i] = trimesh.add_vertex(*point);
					point++;
				}
				int i, j;
				int strip=((g_SH_DISCONTINUOUS_STRIP_TEXMAP==surface_type)
					||(g_SH_DISCONTINUOUS_STRIP==surface_type));
				for (i=0; i<npts1; i++)
				{
					switch (polygon_type)
					{
						case g_QUADRILATERAL:
						{
							if (strip)
							{
								for (j=0; j<npts2-2; j+=2)
								{
									trimesh.add_quadrilateral(
										tri_vertex[j+i*npts2],
										tri_vertex[j+i*npts2+1],
										tri_vertex[j+i*npts2+2],
										tri_vertex[j+i*npts2+3]);
								}
							}
							else
							{
								trimesh.add_quadrilateral(
									tri_vertex[i*npts2],
									tri_vertex[i*npts2+1],
									tri_vertex[i*npts2+2],
									tri_vertex[i*npts2+3]);
							}
						} break;
						case g_TRIANGLE:
						{
							if (strip)
							{
								for (j=0; j<npts2-2; j++)
								{
									if (j & 1)
									{
										trimesh.add_triangle(tri_vertex[j+i*npts2+1], tri_vertex[j+i*npts2], tri_vertex[j+i*npts2+2]);
									}
									else
									{
										trimesh.add_triangle(tri_vertex[j+i*npts2], tri_vertex[j+i*npts2+1], tri_vertex[j+i*npts2+2]);
									}
								}
							}
							else
							{
								trimesh.add_triangle(tri_vertex[i*npts2], tri_vertex[i*npts2+1], tri_vertex[i*npts2+2]);
							}
						} break;
						default:
						{
							/* find polygon centre = average of points */
							Triple centre = { 0.0f, 0.0f, 0.0f };
							Triple *temp_point = point;
							for (j=0; j<npts2; j++)
							{
								centre[0] += (*temp_point)[0];
								centre[1] += (*temp_point)[1];
								centre[2] += (*temp_point)[2];
								temp_point++;
							}
							centre[0] /= (float)npts2;
							centre[1] /= (float)npts2;
							centre[2] /= (float)npts2;
							for (j=0; j<npts2; j++)
							{
								trimesh.add_triangle_coordinates(point[j+i*npts2], point[(j+i*npts2+1) % npts2], centre);
							}
						}
					}

				}
			} break;
			default:
			{
				display_message(ERROR_MESSAGE,
					"draw_surface_stl.  Invalid surface type");
				return_code=0;
			} break;
		}
	}
	else
	{
		if ((1<npts1) && (1<npts2))
		{
			display_message(ERROR_MESSAGE,
				"draw_surface_stl.  Invalid argument(s)");
			return_code=0;
		}
		else
		{
			return_code=1;
		}
	}
	LEAVE;

	return (return_code);
} /* draw_surface_stl */

/***************************************************************************//**
 * Writes surface primitives in graphics object to STL file.
 * 
 * @param trimesh output STL file and context data
 * @param object graphics object to output
 * @param time time at which graphics are output
 * @return 1 on success, 0 on failure
 */
int maketriangle_mesh(Triangle_mesh& trimesh, gtObject *object, float time)
{
	float proportion = 0.0f, *times = NULL;
	int itime, number_of_times, return_code = 0;
	union GT_primitive_list *primitive_list1 = NULL, *primitive_list2 = NULL;

	ENTER(makestl);
	return_code = 1;
	if (object)
	{
		number_of_times = object->number_of_times;
		if (0 < number_of_times)
		{
			itime = number_of_times;
			if ((itime>1)&&(times=object->times))
			{
				itime--;
				times += itime;
				if (time>= *times)
				{
					proportion=0;
				}
				else
				{
					while ((itime>0)&&(time< *times))
					{
						itime--;
						times--;
					}
					if (time< *times)
					{
						proportion=0;
					}
					else
					{
						proportion=times[1]-times[0];
						if (proportion>0)
						{
							proportion=time-times[0]/proportion;
						}
						else
						{
							proportion=0;
						}
					}
				}
			}
			else
			{
				itime=0;
				proportion=0;
			}
			if (object->primitive_lists &&
				(primitive_list1 = object->primitive_lists + itime))
			{
				if (proportion > 0)
				{
					if (!(primitive_list2 = object->primitive_lists + itime + 1))
					{
						display_message(ERROR_MESSAGE,
							"makestl.  Invalid primitive_list");
						return_code = 0;
					}
				}
				else
				{
					primitive_list2 = (union GT_primitive_list *)NULL;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"makestl.  Invalid primitive_lists");
				return_code = 0;
			}
		}
		if ((0 < number_of_times) && return_code)
		{
			switch (object->object_type)
			{
				case g_GLYPH_SET:
				{
					/* not relevant to triangle mesh: ignore */
					return_code=1;
				} break;
				case g_VOLTEX:
				{
					struct GT_voltex *voltex;

					if (voltex = primitive_list1->gt_voltex.first)
					{
						while (voltex)
						{
							draw_voltex_triangle_mesh(trimesh,
								voltex->number_of_vertices, voltex->vertex_list,
								voltex->number_of_triangles, voltex->triangle_list,
								voltex->n_data_components, 
								object->default_material, object->spectrum);
							voltex=voltex->ptrnext;
						}
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,"makestl.  Missing voltex");
						return_code=0;
					}
				} break;
				case g_POINTSET:
				case g_POLYLINE:					
				{
					/* not relevant to STL: ignore */
					return_code=1;
				} break;
				case g_SURFACE:
				{
					struct GT_surface *interpolate_surface,*surface,*surface_2;

					if (surface = primitive_list1->gt_surface.first)
					{
						if (proportion>0)
						{
							surface_2 = primitive_list2->gt_surface.first;
							while (surface&&surface_2)
							{
								if (interpolate_surface=morph_GT_surface(proportion,
									surface,surface_2))
								{
									draw_surface_triangle_mesh(trimesh,
										interpolate_surface->pointlist,
										interpolate_surface->normallist,
										interpolate_surface->texturelist,
										interpolate_surface->n_pts1,
										interpolate_surface->n_pts2,
										interpolate_surface->surface_type,
										interpolate_surface->polygon,
										interpolate_surface->n_data_components,
										interpolate_surface->data,
										object->default_material, object->spectrum);
									DESTROY(GT_surface)(&interpolate_surface);
								}
								surface=surface->ptrnext;
								surface_2=surface_2->ptrnext;
							}
						}
						else
						{
							while (surface)
							{
								draw_surface_triangle_mesh(trimesh,
									surface->pointlist,surface->normallist,
									surface->texturelist,
									surface->n_pts1, surface->n_pts2,
									surface->surface_type, surface->polygon,
									surface->n_data_components,
									surface->data,object->default_material,
									object->spectrum);
								surface=surface->ptrnext;
							}
						}
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,"maketriangle_mesh.  Missing surface");
						return_code=0;
					}
				} break;
				case g_NURBS:
				{
					display_message(WARNING_MESSAGE,"maketriangle_mesh.  nurbs not supported yet");
					return_code=1;
				} break;
				case g_USERDEF:
				{
				} break;
				default:
				{
					display_message(WARNING_MESSAGE,"maketriangle_mesh.  Invalid object type");
					return_code=1;
				} break;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"maketriangle_mesh.  Missing object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* makestl */

int GT_element_settings_graphics_to_triangle_mesh(struct GT_element_settings *settings,
	void *trimesh_void)
{
	int return_code;

	ENTER(GT_element_settings_graphics_to_triangle_mesh);
	if (settings && trimesh_void)
	{
		return_code = 1;
		Triangle_mesh& trimesh = *(static_cast<Triangle_mesh*>(trimesh_void));
		struct GT_object *graphics_object;
		if (GT_element_settings_get_visibility(settings) &&
			(graphics_object = GT_element_settings_get_graphics_object(
			settings)))
		{
			return_code = maketriangle_mesh(trimesh, graphics_object, /*time*/0);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_graphics_to_triangle_mesh.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_graphics_to_stl */

int write_scene_object_triangle_mesh(Triangle_mesh& trimesh,
	struct Scene_object *scene_object)
{
	int return_code;

	ENTER(write_scene_object_triangle_mesh)
	if (scene_object)
	{
		return_code = 1;
		if (g_VISIBLE == Scene_object_get_visibility(scene_object))
		{
			double time;
			if (Scene_object_has_time(scene_object))
			{
				time = Scene_object_get_time(scene_object);
			}
			else
			{
				time = 0.0;
			}

			if (Scene_object_has_gt_object(scene_object,
				(struct GT_object *)NULL))
			{
				return_code = maketriangle_mesh(trimesh,
					Scene_object_get_gt_object(scene_object), time);
			}
			else if (Scene_object_has_graphical_element_group(scene_object,
				(void *)NULL))
			{
				struct GT_element_group *graphical_element_group =
					Scene_object_get_graphical_element_group(scene_object);
				return_code = for_each_settings_in_GT_element_group(
					graphical_element_group, GT_element_settings_graphics_to_triangle_mesh,
					static_cast<void*>(&trimesh));
			}
			else if (Scene_object_has_child_scene(scene_object, (void *)NULL))
			{
				return_code = write_scene_triangle_mesh(trimesh,
					Scene_object_get_child_scene(scene_object));
			}
			else
			{
				return_code = 0;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_scene_object_triangle_mesh.  Missing scene_object");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* write_scene_object_triangle_mesh */

int Scene_object_to_triangle_mesh(struct Scene_object *scene_object,
	void *trimesh_void)
{
	int return_code;

	ENTER(Scene_object_to_triangle_mesh);
	if (scene_object && trimesh_void)
	{
		return_code = 1;
		Triangle_mesh& trimesh = *(static_cast<Triangle_mesh*>(trimesh_void));
		return_code = write_scene_object_triangle_mesh(trimesh, scene_object);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_object_to_triangle_mesh.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_object_to_triangle_mesh */

int write_scene_triangle_mesh(Triangle_mesh& trimesh, struct Scene *scene)
{
	int return_code;

	ENTER(write_scene_triangle_mesh)
	if (scene)
	{

		return_code = 1;
		return_code = for_each_Scene_object_in_Scene(scene, Scene_object_to_triangle_mesh,
			static_cast<void*>(&trimesh));
	}
	else
	{
		display_message(ERROR_MESSAGE,"write_scene_triangle_mesh.  Missing scene");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* write_scene_triangle_mesh */

int render_scene_triangularisation(struct Scene *scene, Triangle_mesh *trimesh)
{
// 	double centre_x, centre_y, centre_z, size_x, size_y, size_z;
// 	Scene_get_graphics_range(scene, 
// 		&centre_x, &centre_y, &centre_z, &size_x, &size_y, &size_z);
// 	if (size_x !=0 && size_y!=0 && size_z!=0)
// 	{
// 		tolerance = tolerance * (float)sqrt(
// 			size_x*size_x + size_y*size_y + size_z*size_z);
// 	}
// 	float coordinates[][3] =
// 	{
// 		{ 0.0, 0.0, 0.0 },
// 		{ 1.0, 0.0, 0.0 },
// 		{ 1.1, 0.0, 0.0 },
// 		{ 1.01, 0.0, 0.0 },
// 		{ 1.001, 0.0, 0.0 },
// 		{ 1.0001, 0.0, 0.0 },
// 		{ 1.000099, 0.0, 0.0 },
// 		{ 1.00001, 0.0, 0.0 },
// 		{ 37.2, 0.0, -5.0 },
// 		{ 37.2, 0.0, -5.0 },
// 		{ 37.2, 1.0, -5.0 }
// 	};

// 	const int number_of_points = sizeof(coordinates) / sizeof(float[3]);
// 	for (int i = 0; i < number_of_points; i++)
// 	{
// 		trimesh.add_vertex(coordinates[i]);
// 	}
	

	//Triangle_mesh trimesh(tolerance);
	write_scene_triangle_mesh(*trimesh, scene);
//	trimesh.list();

	return 1;
}

int render_scene_object_triangularisation(struct Scene_object *scene_object)
{
	float tolerance = 0.000001;
	Triangle_mesh trimesh(tolerance);
	Scene_object_to_triangle_mesh(scene_object,
		static_cast<void*>(&trimesh));
	///trimesh.list();
	return 1;
}

int render_gt_element_group_triangularisation(GT_element_group *graphical_element_group)
{
	float tolerance = 0.000001;
	Triangle_mesh trimesh(tolerance);
	int return_code = for_each_settings_in_GT_element_group(
		graphical_element_group, GT_element_settings_graphics_to_triangle_mesh,
		static_cast<void*>(&trimesh));
	//trimesh.list();
	return return_code;
}

int Render_graphics_triangularisation::Scene_execute(Scene *scene)
{
	USE_PARAMETER(scene);
// 	printf("1");
 	render_scene_triangularisation(scene, trimesh);
// 	//trimesh.list();
	return 1;
}

Triangle_mesh *Render_graphics_triangularisation::get_triangle_mesh()
{
	return trimesh;
}

// int Render_graphics_triangularisation::Scene_object_compile(Scene_object *scene_object)
// {
// 	printf("2");
// 	return render_scene_object_triangularisation(scene_object);
// }

// int Render_graphics_triangularisation::Graphical_element_group_compile(GT_element_group *graphical_element_group)
// {
// 	printf("3");
// 	return render_gt_element_group_triangularisation(graphical_element_group);
// }
