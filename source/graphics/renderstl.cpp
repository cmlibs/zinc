/*******************************************************************************
FILE : renderstl.cpp

LAST MODIFIED : 8 July 2008

DESCRIPTION :
Renders gtObjects to STL stereolithography file.
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
 * Portions created by the Initial Developer are Copyright (C) 2008
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

#include <stack>
extern "C" {
#include <stdio.h>
#include "general/debug.h"
#include "general/matrix_vector.h"
#include "general/mystring.h"
#include "graphics/graphics_object.h"
#include "graphics/renderstl.h"
#include "graphics/scene.h"
#include "user_interface/message.h"
#include "graphics/graphics_object_private.h"
}

namespace {

/*
Module types
------------
*/

class Transformation_matrix
{
private:
	double entry[4][4];

public:
	Transformation_matrix(const gtMatrix& gt_matrix)
	{
		for (int i=0; i<4; i++)
		{
			for (int j=0; j<4; j++)
			{
				entry[i][j] = (double)gt_matrix[j][i];
			}
		}
	}

	Transformation_matrix(
		double m11, double m12, double m13, double m14,
		double m21, double m22, double m23, double m24,
		double m31, double m32, double m33, double m34,
		double m41, double m42, double m43, double m44)
	{
		entry[0][0] = m11;
		entry[0][1] = m12;
		entry[0][2] = m13;
		entry[0][3] = m14;

		entry[1][0] = m21;
		entry[1][1] = m22;
		entry[1][2] = m23;
		entry[1][3] = m24;

		entry[2][0] = m31;
		entry[2][1] = m32;
		entry[2][2] = m33;
		entry[2][3] = m34;

		entry[3][0] = m41;
		entry[3][1] = m42;
		entry[3][2] = m43;
		entry[3][3] = m44;
	}

	void post_multiply(const Transformation_matrix& matrix)
	{
		Transformation_matrix prev = *this;
		for (int i=0; i<4; i++)
		{
			for (int j=0; j<4; j++)
			{
				entry[i][j] =
					prev.entry[i][0]*matrix.entry[0][j] +
					prev.entry[i][1]*matrix.entry[1][j] +
					prev.entry[i][2]*matrix.entry[2][j] +
					prev.entry[i][3]*matrix.entry[3][j];
			}
		}
	}

	void transform(double* tv) const
	{
		double x = tv[0];
		double y = tv[1];
		double z = tv[2];
		tv[0] = entry[0][0]*x + entry[0][1]*y + entry[0][2]*z + entry[0][3];
		tv[1] = entry[1][0]*x + entry[1][1]*y + entry[1][2]*z + entry[1][3];
		tv[2] = entry[2][0]*x + entry[2][1]*y + entry[2][2]*z + entry[2][3];
	}
};

class Stl_context
{
private:
	FILE *stl_file;
	char *solid_name;
	/* Future: add binary option */
	std::stack<Transformation_matrix> transformation_stack;

public:
	Stl_context(const char *file_name, const char *solid_name) :
		stl_file(fopen(file_name, "w")),
		solid_name(duplicate_string(solid_name))
	{
		/* ASCII STL header */
		fprintf(stl_file,"solid %s\n",solid_name);
	}

	~Stl_context()
	{
		fprintf(stl_file,"endsolid %s\n",solid_name);
		DEALLOCATE(solid_name);
		fclose(stl_file);
	}

	/***************************************************************************//**
	 * Confirms STL file is correctly opened.
	 * 
	 * @return true if context is correctly constructed, false if not.
	 */
	bool is_valid() const
	{
		return (stl_file != (FILE *)NULL) && (solid_name != (char *)NULL);
	}

/***************************************************************************//**
 * Pushes another item onto the transformation stack equal to the previous top
 * item multiplied by the incoming matrix. 
 * 
 * @matrix the matrix to post multiply by
 */
	void push_multiply_transformation(const Transformation_matrix& matrix)
	{
		if (transformation_stack.empty())
		{
			transformation_stack.push(matrix);
		}
		else
		{
			transformation_stack.push(transformation_stack.top());
			transformation_stack.top().post_multiply(matrix);
		}
	}

	void pop_transformation(void)
	{
		if (transformation_stack.empty())
		{
			display_message(ERROR_MESSAGE,
				"Stl_context::pop_transformation.  Transformation stack is empty");
		}
		else
		{
			transformation_stack.pop();
		}
	}

	void transform(const Triple& v, double* tv) const
	{
		tv[0] = static_cast<double>(v[0]);
		tv[1] = static_cast<double>(v[1]);
		tv[2] = static_cast<double>(v[2]);
		if (!transformation_stack.empty())
		{
			transformation_stack.top().transform(tv);
		}
	}

	/***************************************************************************//**
	 * Writes a single STL triangle to file.
	 * 
	 * @param v1 coordinates of first vertex
	 * @param v2 coordinates of second vertex
	 * @param v3 coordinates of third vertex
	 */
	void Stl_context::write_triangle(
		const Triple& v1, const Triple& v2, const Triple& v3)
	{
		double tv1[3], tv2[3], tv3[3];
		transform(v1, tv1);
		transform(v2, tv2);
		transform(v3, tv3);

		double tangent1[3], tangent2[3], normal[3];
		tangent1[0] = tv2[0] - tv1[0];
		tangent1[1] = tv2[1] - tv1[1];
		tangent1[2] = tv2[2] - tv1[2];
		tangent2[0] = tv3[0] - tv1[0];
		tangent2[1] = tv3[1] - tv1[1];
		tangent2[2] = tv3[2] - tv1[2];
		cross_product3(tangent1, tangent2, normal);
		if (0.0 < normalize3(normal))
		{
			fprintf(stl_file, "facet normal %f %f %f\n", (float)normal[0], (float)normal[1], (float)normal[2]);
			fprintf(stl_file, " outer loop\n");		
			fprintf(stl_file, "  vertex %g %g %g\n", (float)tv1[0], (float)tv1[1], (float)tv1[2]);		
			fprintf(stl_file, "  vertex %g %g %g\n", (float)tv2[0], (float)tv2[1], (float)tv2[2]);
			fprintf(stl_file, "  vertex %g %g %g\n", (float)tv3[0], (float)tv3[1], (float)tv3[2]);
			fprintf(stl_file, " endloop\n");
		 	fprintf(stl_file, "endfacet\n");
		}
	} /* write_triangle_stl */

}; /* class Stl_context */

/*
Module functions
----------------
*/

int makestl(Stl_context& stl_context, gtObject *graphics_object, float time);

/***************************************************************************//**
 * Writes a glyph set to STL file.
 * Only surface glyphs can be output; other primitives are silently ignored.
 * Transformations are flattened.
 *
 * @param stl_context output STL file and context data
 * @param number_of_points number of points at which glyphs are drawn
 * @param point_list array of glyph point centres
 * @param axis1_list array of vectors which scale and orient first glyph axis
 * @param axis2_list array of vectors which scale and orient second glyph axis
 * @param axis3_list array of vectors which scale and orient third glyph axis
 * @param glyph the graphics object drawn at each point
 * @param time the time at which the graphics are output
 * @param labels ignored
 * @param number_of_data_components ignored
 * @param data ignored
 * @param material ignored
 * @param spectrum ignored
 * @return 1 on success, 0 on failure
 */
int draw_glyph_set_stl(Stl_context& stl_context, int number_of_points,
	Triple *point_list, Triple *axis1_list, Triple *axis2_list,
	Triple *axis3_list, Triple *scale_list,
	struct GT_object *glyph,char **labels,
	int number_of_data_components, GTDATA *data,
	struct Graphical_material *material, struct Spectrum *spectrum, float time)
{
	int return_code;
	struct GT_object *temp_glyph;
	Triple *axis1, *axis2, *axis3, *point, *scale, temp_axis1, temp_axis2,
		temp_axis3, temp_point;

	ENTER(draw_glyph_set_stl);
	/* Keep a similar interface to all the other render implementations */
	USE_PARAMETER(labels);
	USE_PARAMETER(number_of_data_components);
	USE_PARAMETER(data);
	USE_PARAMETER(material);
	USE_PARAMETER(spectrum);
	return_code=0;
	if ((0<number_of_points) && point_list && axis1_list && axis2_list &&
		axis3_list && scale_list && glyph)
	{
		axis1 = axis1_list;
		axis2 = axis2_list;
		axis3 = axis3_list;
		point = point_list;
		scale = scale_list;
		for (int i=0; i<number_of_points; i++)
		{
			int mirror_mode = GT_object_get_glyph_mirror_mode(glyph);
			int number_of_glyphs = mirror_mode ? 2 : 1;
			for (int j=0; j<number_of_glyphs; j++)
			{
				resolve_glyph_axes(*point, *axis1, *axis2, *axis3, *scale,
					/*mirror*/j, /*reverse*/mirror_mode,
					temp_point, temp_axis1, temp_axis2, temp_axis3);
				Transformation_matrix matrix(
					(double)temp_axis1[0], (double)temp_axis2[0], (double)temp_axis3[0], (double)temp_point[0],
					(double)temp_axis1[1], (double)temp_axis2[1], (double)temp_axis3[1], (double)temp_point[1],
					(double)temp_axis1[2], (double)temp_axis2[2], (double)temp_axis3[2], (double)temp_point[2],
					0.0, 0.0, 0.0, 1.0);
				stl_context.push_multiply_transformation(matrix);
				if (mirror_mode)
				{
					/* ignore first glyph since just a wrapper for the second */
					temp_glyph = GT_object_get_next_object(glyph);
				}
				else
				{
					temp_glyph = glyph;
				}
				/* call the glyph display lists of the linked-list of glyphs */
				while (temp_glyph)
				{
					makestl(stl_context, temp_glyph, time);
					temp_glyph = GT_object_get_next_object(temp_glyph);
				}
				stl_context.pop_transformation();
			}
			point++;
			axis1++;
			axis2++;
			axis3++;
			scale++;
		}
		return_code=1;
	}
	else
	{
		if (0 == number_of_points)
		{
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,"draw_glyph_set_stl. Invalid argument(s)");
			return_code=0;
		}
	}
	LEAVE;

	return (return_code);
} /* draw_glyph_set_stl */

/***************************************************************************//**
 * Writes quadrilateral and triangle surface strips to STL file as
 * individual triangles.
 * 
 * @param stl_context output STL file and context data
 * @param surfpts array of points making up the surface
 * @param npts1 number of surface points in first direction
 * @param pnts2 number of surface points in second direction; same as npts1 for
 *   g_TRIANGLE polygon_type.
 * @param surface_type any of enum GT_surface_type 
 * @param polygon_type g_QUADRILATERAL, g_TRIANGLE or g_GENERAL_POLYGON
 * @param normalpts ignored
 * @param texturepts ignored
 * @param number_of_data_components ignored
 * @param data ignored
 * @param material ignored
 * @param spectrum ignored
 * @return 1 on success, 0 on failure
 */
int draw_surface_stl(Stl_context& stl_context, Triple *surfpts,
	Triple *normalpts, Triple *texturepts, int npts1,int npts2,
	enum GT_surface_type surface_type, gtPolygonType polygon_type,
	int number_of_data_components, GTDATA *data,
	struct Graphical_material *material, struct Spectrum *spectrum)
{
	int i, j, return_code;
	Triple *point;

	ENTER(draw_surface_stl);
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
				switch (polygon_type)
				{
					case g_QUADRILATERAL:
					{
						for (j=npts2-1; j>0; j--)
						{
							for (i=npts1-1; i>0; i--)
							{
								stl_context.write_triangle(point[0], point[1], point[npts1]);
								stl_context.write_triangle(point[1], point[npts1+1], point[npts1]);
								point++;
							}
							point++;
						}
						return_code=1;
					} break;
					case g_TRIANGLE:
					{
						/* expecting npts2 == npts1 */
						for (j=npts2-1; j>0; j--)
						{
							for (i=j-1; i>0; i--)
							{
								stl_context.write_triangle(point[0], point[1], point[j+1]);
								stl_context.write_triangle(point[1], point[j+2], point[j+1]);
								point++;
							}
							stl_context.write_triangle(point[0], point[1], point[j+1]);
							point++;
							point++;
						}
						return_code=1;
					} break;
				}
			} break;
			case g_SH_DISCONTINUOUS:
			case g_SH_DISCONTINUOUS_STRIP:
			case g_SH_DISCONTINUOUS_TEXMAP:
			case g_SH_DISCONTINUOUS_STRIP_TEXMAP:
			{
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
									stl_context.write_triangle(point[j], point[j+1], point[j+2]);
									stl_context.write_triangle(point[j+1], point[j+3], point[j+2]);
								}
							}
							else
							{
								stl_context.write_triangle(point[0], point[1], point[2]);
								stl_context.write_triangle(point[0], point[2], point[3]);
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
										stl_context.write_triangle(point[j+1], point[j], point[j+2]);
									}
									else
									{
										stl_context.write_triangle(point[j], point[j+1], point[j+2]);
									}
								}
							}
							else
							{
								stl_context.write_triangle(point[0], point[1], point[2]);
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
								stl_context.write_triangle(point[j], point[(j+1) % npts2], centre);
							}
						}
					}
					point += npts2;
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
 * Writes voltex triangles to STL file.
 * 
 * @param stl_context output STL file and context data
 * @param surfpts array of points making up the surface
 * @param number_of_vertices number of vertices in structure
 * @param vertex_list array of vertices
 * @param number_of_triangles number of triangles to output
 * @param triangle_list array of triangles indexing the vertex_list
 * @param number_of_data_components ignored
 * @param material ignored
 * @param spectrum ignored
 * @return 1 on success, 0 on failure
 */
int draw_voltex_stl(Stl_context& stl_context,
	int number_of_vertices, struct VT_iso_vertex **vertex_list,
	int number_of_triangles, struct VT_iso_triangle **triangle_list,
	int number_of_data_components,
	struct Graphical_material *material, struct Spectrum *spectrum)
{
	int i, index1, index2, index3, return_code;
	Triple v1, v2, v3;

	ENTER(draw_voltex_stl);
	/* Keep a similar interface to all the other render implementations */
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
			stl_context.write_triangle(v1, v2, v3);
		}
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"draw_voltex_stl.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* draw_voltex_stl */

/***************************************************************************//**
 * Writes surface primitives in graphics object to STL file.
 * 
 * @param stl_context output STL file and context data
 * @param object graphics object to output
 * @param time time at which graphics are output
 * @return 1 on success, 0 on failure
 */
int makestl(Stl_context& stl_context, gtObject *object, float time)
{
	float proportion, *times;
	int itime, number_of_times, return_code;
	union GT_primitive_list *primitive_list1, *primitive_list2;

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
					struct GT_glyph_set *interpolate_glyph_set,*glyph_set,*glyph_set_2;

					if (glyph_set = primitive_list1->gt_glyph_set.first)
					{
						if (proportion>0)
						{
							glyph_set_2 = primitive_list2->gt_glyph_set.first;
							while (glyph_set&&glyph_set_2)
							{
								if (interpolate_glyph_set=morph_GT_glyph_set(proportion,
									glyph_set,glyph_set_2))
								{
									draw_glyph_set_stl(stl_context,
										interpolate_glyph_set->number_of_points,
										interpolate_glyph_set->point_list,
										interpolate_glyph_set->axis1_list,
										interpolate_glyph_set->axis2_list,
										interpolate_glyph_set->axis3_list,
										interpolate_glyph_set->scale_list,
										interpolate_glyph_set->glyph,
										interpolate_glyph_set->labels,
										interpolate_glyph_set->n_data_components,
										interpolate_glyph_set->data,
										object->default_material,object->spectrum,
									  time);
									DESTROY(GT_glyph_set)(&interpolate_glyph_set);
								}
								glyph_set=glyph_set->ptrnext;
								glyph_set_2=glyph_set_2->ptrnext;
							}
						}
						else
						{
							while (glyph_set)
							{
								draw_glyph_set_stl(stl_context,
									glyph_set->number_of_points,
									glyph_set->point_list,glyph_set->axis1_list,
									glyph_set->axis2_list,glyph_set->axis3_list,
									glyph_set->scale_list,glyph_set->glyph,
									glyph_set->labels,glyph_set->n_data_components,
									glyph_set->data,object->default_material,object->spectrum,
									time);
								glyph_set=glyph_set->ptrnext;
							}
						}
						return_code=1;
					}
					else
					{
						display_message(ERROR_MESSAGE,"makestl.  Missing glyph_set");
						return_code=0;
					}
				} break;
				case g_VOLTEX:
				{
					struct GT_voltex *voltex;

					if (voltex = primitive_list1->gt_voltex.first)
					{
						while (voltex)
						{
							draw_voltex_stl(stl_context,
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
									draw_surface_stl(stl_context,
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
								draw_surface_stl(stl_context,
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
						display_message(ERROR_MESSAGE,"makestl.  Missing surface");
						return_code=0;
					}
				} break;
				case g_NURBS:
				{
					display_message(WARNING_MESSAGE,"makestl.  nurbs not supported yet");
					return_code=1;
				} break;
				case g_USERDEF:
				{
				} break;
				default:
				{
					display_message(ERROR_MESSAGE,"makestl.  Invalid object type");
					return_code=0;
				} break;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"makestl.  Missing object");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* makestl */

int GT_element_settings_graphics_to_stl(struct GT_element_settings *settings,
	void *stl_context_void)
{
	int return_code;

	ENTER(GT_element_settings_graphics_to_stl);
	if (settings && stl_context_void)
	{
		return_code = 1;
		Stl_context& stl_context = *(static_cast<Stl_context*>(stl_context_void));
		struct GT_object *graphics_object;
		if (GT_element_settings_get_visibility(settings) &&
			(graphics_object = GT_element_settings_get_graphics_object(
			settings)))
		{
			return_code = makestl(stl_context, graphics_object, /*time*/0);
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"GT_element_settings_graphics_to_stl.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* GT_element_settings_graphics_to_stl */

int write_scene_stl(Stl_context& stl_context, struct Scene *scene);

/**************************************************************************//**
 * Renders the visible parts of a scene object to STL.
 * 
 * @param stl_context output STL file and context data
 * @param scene_object the scene object to output
 * @return 1 on success, 0 on failure
 */
int write_scene_object_stl(Stl_context& stl_context,
	struct Scene_object *scene_object)
{
	int return_code;

	ENTER(write_scene_object_stl)
	if (scene_object)
	{
		return_code = 1;
		if (g_VISIBLE == Scene_object_get_visibility(scene_object))
		{
			int has_transform = Scene_object_has_transformation(scene_object);
			if (has_transform)
			{
				gtMatrix transformation;
				Scene_object_get_transformation(scene_object, &transformation);
				stl_context.push_multiply_transformation(
					Transformation_matrix(transformation));
			}

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
				return_code = makestl(stl_context,
					Scene_object_get_gt_object(scene_object), time);
			}
			else if (Scene_object_has_graphical_element_group(scene_object,
				(void *)NULL))
			{
				struct GT_element_group *graphical_element_group =
					Scene_object_get_graphical_element_group(scene_object);
				return_code = for_each_settings_in_GT_element_group(
					graphical_element_group, GT_element_settings_graphics_to_stl,
					static_cast<void*>(&stl_context));
			}
			else if (Scene_object_has_child_scene(scene_object, (void *)NULL))
			{
				return_code = write_scene_stl(stl_context,
					Scene_object_get_child_scene(scene_object));
			}
			else
			{
				return_code = 0;
			}

			if (has_transform)
			{
				stl_context.pop_transformation();
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_scene_object_stl.  Missing scene_object");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* write_scene_object_stl */

int Scene_object_to_stl(struct Scene_object *scene_object,
	void *stl_context_void)
{
	int return_code;

	ENTER(Scene_object_to_stl);
	if (scene_object && stl_context_void)
	{
		return_code = 1;
		Stl_context& stl_context = *(static_cast<Stl_context*>(stl_context_void));
		return_code = write_scene_object_stl(stl_context, scene_object);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Scene_object_to_stl.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Scene_object_to_stl */

/**************************************************************************//**
 * Renders the visible objects in a scene to STL.
 * 
 * @param stl_context output STL file and context data
 * @param scene the scene to output
 * @return 1 on success, 0 on failure
 */
int write_scene_stl(Stl_context& stl_context, struct Scene *scene)
{
	int return_code;

	ENTER(write_scene_stl)
	if (scene)
	{
		return_code = for_each_Scene_object_in_Scene(scene, Scene_object_to_stl,
			static_cast<void*>(&stl_context));
	}
	else
	{
		display_message(ERROR_MESSAGE,"write_scene_stl.  Missing scene");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* write_scene_stl */

} // anonymous namespace

/*
Global functions
----------------
*/

int export_to_stl(char *file_name, struct Scene *scene,
	struct Scene_object *scene_object)
{
	int return_code;

	ENTER(export_to_stl);
	if (file_name && scene)
	{
		build_Scene(scene);
		char *solid_name = NULL;
		if (scene_object)
		{
			GET_NAME(Scene_object)(scene_object, &solid_name);
		}
		else
		{
			GET_NAME(Scene)(scene, &solid_name);
		}
		Stl_context stl_context(file_name, solid_name);
		if (stl_context.is_valid())
		{
			if (scene_object)
			{
				return_code = write_scene_object_stl(stl_context, scene_object);
			}
			else
			{
				return_code = write_scene_stl(stl_context, scene);
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"export_to_stl.  Could not open stl file %s", file_name);
			return_code = 0;
		}
		DEALLOCATE(solid_name);
	}
	else
	{
		display_message(ERROR_MESSAGE,"export_to_stl.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return( return_code);
} /* export_to_stl */
