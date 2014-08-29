/*******************************************************************************
FILE : renderstl.cpp

LAST MODIFIED : 8 July 2008

DESCRIPTION :
Renders gtObjects to STL stereolithography file.
==============================================================================*/
/* OpenCMISS-Zinc Library
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <stack>
#include <stdio.h>
#include "general/debug.h"
#include "general/matrix_vector.h"
#include "general/mystring.h"
#include "graphics/glyph.hpp"
#include "graphics/graphics_object.h"
#include "graphics/render_stl.h"
#include "graphics/scene.h"
#include "general/message.h"
#include "graphics/graphics_object_private.hpp"
#include "graphics/scene.hpp"

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
	void write_triangle(
		const Triple& v1, const Triple& v2, const Triple& v3)
	{
		ZnReal tv1[3], tv2[3], tv3[3];
		transform(v1, tv1);
		transform(v2, tv2);
		transform(v3, tv3);

		ZnReal tangent1[3], tangent2[3], normal[3];
		tangent1[0] = tv2[0] - tv1[0];
		tangent1[1] = tv2[1] - tv1[1];
		tangent1[2] = tv2[2] - tv1[2];
		tangent2[0] = tv3[0] - tv1[0];
		tangent2[1] = tv3[1] - tv1[1];
		tangent2[2] = tv3[2] - tv1[2];
		cross_product3(tangent1, tangent2, normal);
		if (0.0 < normalize3(normal))
		{
			fprintf(stl_file, "facet normal %f %f %f\n", (ZnReal)normal[0], (ZnReal)normal[1], (ZnReal)normal[2]);
			fprintf(stl_file, " outer loop\n");		
			fprintf(stl_file, "  vertex %g %g %g\n", (ZnReal)tv1[0], (ZnReal)tv1[1], (ZnReal)tv1[2]);		
			fprintf(stl_file, "  vertex %g %g %g\n", (ZnReal)tv2[0], (ZnReal)tv2[1], (ZnReal)tv2[2]);
			fprintf(stl_file, "  vertex %g %g %g\n", (ZnReal)tv3[0], (ZnReal)tv3[1], (ZnReal)tv3[2]);
			fprintf(stl_file, " endloop\n");
		 	fprintf(stl_file, "endfacet\n");
		}
	} /* write_triangle_stl */

}; /* class Stl_context */

/*
Module functions
----------------
*/

int makestl(Stl_context& stl_context, gtObject *graphics_object, ZnReal time);

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
	int number_of_data_components, GLfloat *data,
	struct Graphical_material *material, struct Spectrum *spectrum)
{
	int i, j, return_code = 0;
	Triple *point = NULL;

	ENTER(draw_surface_stl);
	/* Keep a similar interface to all the other render implementations */
	USE_PARAMETER(normalpts);
	USE_PARAMETER(texturepts);
	USE_PARAMETER(number_of_data_components);
	USE_PARAMETER(data);
	USE_PARAMETER(material);
	USE_PARAMETER(spectrum);
	bool continuous = (surface_type == g_SHADED) || (surface_type == g_SHADED_TEXMAP);
	if (surfpts && ((continuous && (1 < npts1) && (1 < npts2)) ||
		(!continuous && (0 < npts1) && (2 < npts2))))
	{
		point=surfpts;
		switch (surface_type)
		{
			case g_SHADED:
			case g_SHADED_TEXMAP:
			{
				switch (polygon_type)
				{
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
					default:
					{
						/* Do nothing */
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
							centre[0] /= (ZnReal)npts2;
							centre[1] /= (ZnReal)npts2;
							centre[2] /= (ZnReal)npts2;
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
 * Writes surface primitives in graphics object to STL file.
 * 
 * @param stl_context output STL file and context data
 * @param object graphics object to output
 * @param time time at which graphics are output
 * @return 1 on success, 0 on failure
 */
int makestl(Stl_context& stl_context, gtObject *object, ZnReal time)
{
	ZnReal proportion = 0.0f, *times = NULL;
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

int write_scene_stl(Stl_context& stl_context, cmzn_scene_id scene,
	cmzn_scenefilter_id filter);

/**************************************************************************//**
 * Renders the visible parts of a scene object to STL.
 * 
 * @param stl_context output STL file and context data
 * @param graphics_object the graphics object to output
 * @return 1 on success, 0 on failure
 */
int write_graphics_object_stl(Stl_context& stl_context,
	struct GT_object *graphics_object, double time)
{
	int return_code;

	ENTER(write_graphics_object_stl)
	if (graphics_object)
	{
		return_code = makestl(stl_context,graphics_object, time);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"write_graphics_object_stl.  Missing graphics_object");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* write_graphics_object_stl */

int Graphcis_object_to_stl(struct GT_object *graphics_object, double time,
	void *stl_context_void)
{
	int return_code;

	ENTER(Graphcis_object_to_stl);
	if (graphics_object && stl_context_void)
	{
		return_code = 1;
		Stl_context& stl_context = *(static_cast<Stl_context*>(stl_context_void));
		return_code = write_graphics_object_stl(stl_context, graphics_object, time);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Graphcis_object_to_stl.  Invalid argument(s)");
		return_code=0;
	}

	LEAVE;

	return (return_code);
} /* Graphcis_object_to_stl */

/**
 * Renders the visible objects in a scene to STL.
 * 
 * @param stl_context output STL file and context data
 * @param scene the scene to output
 * @param filter the filter to filter scenes
 * @return 1 on success, 0 on failure
 */
int write_scene_stl(Stl_context& stl_context, cmzn_scene_id scene,
	cmzn_scenefilter_id filter)
{
	int return_code;
	
	if (scene)
	{
		return_code=for_each_graphics_object_in_scene_tree(scene, filter,
			Graphcis_object_to_stl,(void *)&stl_context);
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

int export_to_stl(char *file_name, cmzn_scene_id scene, cmzn_scenefilter_id filter)
{
	int return_code;

	if (file_name && scene)
	{
		build_Scene(scene, filter);
		char *solid_name = cmzn_region_get_name(cmzn_scene_get_region_internal(scene));
		Stl_context stl_context(file_name, solid_name);
		if (stl_context.is_valid())
		{
			return_code = write_scene_stl(stl_context, scene, filter);
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

	return( return_code);
} /* export_to_stl */
