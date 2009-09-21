/***************************************************************************//**
 * FILE : finite_element_to_iso_surfaces.cpp
 * 
 * Functions for creating graphical iso-surfaces from finite element fields.
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
 * Portions created by the Initial Developer are Copyright (C) 2009
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
#include <list>
#include <map>
extern "C" {
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_wrappers.h"
#include "finite_element/finite_element.h"
#include "finite_element/finite_element_to_iso_surfaces.h"
#include "general/debug.h"
#include "general/matrix_vector.h"
}
extern "C" {
#include "user_interface/message.h"
}

/*
Module types and functions
--------------------------
*/

namespace {

class Point_index
{
public:
	int i, j, k;

	Point_index() :
		i(0), j(0), k(0)
	{
	}

	Point_index(int i, int j, int k) :
		i(i), j(j), k(k)
	{
	}
	
	void set(int in_i, int in_j, int in_k)
	{
		i = in_i;
		j = in_j;
		k = in_k;
	}

	bool operator<(const Point_index& other) const
	{
		return ((i < other.i) || ((i == other.i) &&
			((j < other.j) || ((j == other.j) && (k < other.k)))));
	}
};

class Point_index_pair
{
public:
	Point_index pa;
	Point_index pb;

	Point_index_pair(const Point_index& in_pa, const Point_index& in_pb)
	{
		// ensure pa < pb
		if (in_pa < in_pb)
		{
			pa = in_pa;
			pb = in_pb;
		}
		else
		{
			pa = in_pb;
			pb = in_pa;
		}
	}
};

class Point_index_pair_compare
{
public:
	bool operator() (const Point_index_pair &pp1, const Point_index_pair &pp2) const
	{
		return ((pp1.pa < pp2.pa) || ((!(pp2.pa < pp1.pa)) && (pp1.pb < pp2.pb)));
	}
};

class Iso_vertex
{
public:
	FE_value xi[3];
	FE_value coordinates[3];
	FE_value normal[3];
	FE_value texture_coordinates[3];
	int number_of_data_components; // GRC point to shared object for this?
	GTDATA *data;

	Iso_vertex() :
		number_of_data_components(0),
		data(NULL)
	{
		xi[0] = 0.0;
		xi[1] = 0.0;
		xi[2] = 0.0;
		coordinates[0] = 0.0;
		coordinates[1] = 0.0;
		coordinates[2] = 0.0;
		normal[0] = 0.0;
		normal[1] = 0.0;
		normal[2] = 0.0;
		texture_coordinates[0] = 0.0;
		texture_coordinates[1] = 0.0;
		texture_coordinates[2] = 0.0;
	}

	Iso_vertex(const Iso_vertex& in_vertex) :
		data(NULL)
	{
		operator=(in_vertex);
	}
	
	~Iso_vertex()
	{
		delete[] data;
	}
	
	void operator=(const Iso_vertex& in_vertex)
	{
		xi[0] = in_vertex.xi[0];
		xi[1] = in_vertex.xi[1];
		xi[2] = in_vertex.xi[2];
		coordinates[0] = in_vertex.coordinates[0];
		coordinates[1] = in_vertex.coordinates[1];
		coordinates[2] = in_vertex.coordinates[2];
		normal[0] = in_vertex.normal[0];
		normal[1] = in_vertex.normal[1];
		normal[2] = in_vertex.normal[2];
		texture_coordinates[0] = in_vertex.texture_coordinates[0];
		texture_coordinates[1] = in_vertex.texture_coordinates[1];
		texture_coordinates[2] = in_vertex.texture_coordinates[2];
		number_of_data_components = in_vertex.number_of_data_components;
		delete data;
		if (NULL != in_vertex.data)
		{
			data = new GTDATA[number_of_data_components];
			for (int i = 0; i < number_of_data_components; i++)
			{
				data[i] = in_vertex.data[i];
			}
		}
		else
		{
			data = NULL;
		}
	}
};

inline FE_value square_distance3(const FE_value *c1, const FE_value *c2)
{
	FE_value offset0 = c2[0] - c1[0];
	FE_value offset1 = c2[1] - c1[1];
	FE_value offset2 = c2[2] - c1[2];
	return (offset0*offset0 + offset1*offset1 + offset2*offset2);
}

typedef std::map<Point_index_pair, Iso_vertex*, Point_index_pair_compare> Iso_vertex_map;
typedef std::map<Point_index_pair, Iso_vertex*, Point_index_pair_compare>::iterator Iso_vertex_map_iterator;
typedef std::map<Point_index_pair, Iso_vertex*, Point_index_pair_compare>::const_iterator Iso_vertex_map_const_iterator;

class Iso_triangle
{
public:
	const Iso_vertex *v1, *v2, *v3;
	
	Iso_triangle(const Iso_vertex *v1, const Iso_vertex *v2, const Iso_vertex *v3) :
		v1(v1), v2(v2), v3(v3)
	{
	}
};

typedef std::list<Iso_triangle*> Iso_triangle_list;
typedef std::list<Iso_triangle*>::iterator Iso_triangle_list_iterator;
typedef std::list<Iso_triangle*>::const_iterator Iso_triangle_list_const_iterator;

class Iso_mesh
{
public:
	Iso_vertex_map vertex_map;
	Iso_triangle_list triangle_list;
	
	Iso_mesh() :
		vertex_map(Point_index_pair_compare()),
		triangle_list()
	{
	}

	~Iso_mesh()
	{
		for (Iso_triangle_list_iterator triangle_iter = triangle_list.begin(); triangle_iter != triangle_list.end(); triangle_iter++)
		{
			delete (*triangle_iter);
		}
		for (Iso_vertex_map_iterator vertex_iter = vertex_map.begin(); vertex_iter != vertex_map.end(); vertex_iter++)
		{
			Iso_vertex *vertex = vertex_iter->second;
			delete (vertex);
		}
	}
	
	const Iso_vertex *get_vertex(const Point_index_pair& pp) const
	{
		Iso_vertex_map_const_iterator iter = vertex_map.find(pp);
		if (iter != vertex_map.end())
		{
			if (NULL != iter->second)
			{
				return (iter->second);
			}
		}
		return NULL;
	}

	const Iso_vertex *add_vertex(const Point_index_pair& pp, const Iso_vertex& vertex)
	{
		Iso_vertex *new_vertex = new Iso_vertex(vertex);
		vertex_map[pp] = new_vertex;
		return new_vertex;
	}
	
	void add_triangle(const Iso_vertex *v1,	const Iso_vertex *v2, const Iso_vertex *v3)
	{
		// ignore degenerate triangles
		if ((0.0 == square_distance3(v1->coordinates, v2->coordinates)) ||
			(0.0 == square_distance3(v2->coordinates, v3->coordinates)) ||
			(0.0 == square_distance3(v3->coordinates, v1->coordinates)))
		{
			return;
		}
		triangle_list.push_back(new Iso_triangle(v1, v2, v3));
	}

private:
	// declared but not defined so illegal:
	Iso_mesh(const Iso_mesh& in_Iso_mesh);

	// declared but not defined so illegal:
	void operator=(const Iso_mesh& in_Iso_mesh);
};

class Isosurface_builder
{
private:
	FE_element *element;
	FE_value time;
	int number_in_xi1, number_in_xi2, number_in_xi3;
	FE_value delta_xi1, delta_xi2, delta_xi3;
	double iso_value;
	Computed_field *coordinate_field, *data_field, *scalar_field,
		*texture_coordinate_field;
	int plane_size;
	double *plane_scalars;
	bool cube, simplex12, simplex13, simplex23, tetrahedron;
public:
	Iso_mesh mesh;

public:
	Isosurface_builder(FE_element *element, FE_value time,
		int number_in_xi1_requested, int number_in_xi2_requested,
		int number_in_xi3_requested, double iso_value,
		Computed_field *coordinate_field, Computed_field *data_field,
		Computed_field *scalar_field, Computed_field *texture_coordinate_field);

	~Isosurface_builder();

	int do_it();

private:
	const Iso_vertex *compute_line_crossing(const Point_index_pair& pp);

	const Iso_vertex *get_line_crossing(const Point_index_pair& pp)
	{
		const Iso_vertex *vertex = mesh.get_vertex(pp);
		if (NULL == vertex)
		{
			vertex = compute_line_crossing(pp);
		}
		return (vertex);
	}

	double get_scalar(const Point_index& p) const
	{
		return plane_scalars[(p.k & 1)*plane_size + p.j*(number_in_xi1 + 1) + p.i];
	}

	void set_scalar(const Point_index& p, double scalar_value)
	{
		plane_scalars[(p.k & 1)*plane_size + p.j*(number_in_xi1 + 1) + p.i] = scalar_value;
	}

	void get_xi(const Point_index& p, FE_value *xi) const
	{
		xi[0] = p.i * delta_xi1;
		xi[1] = p.j * delta_xi2;
		xi[2] = p.k * delta_xi3;
	}

	void cross_tetrahedron(const Point_index& p1,
		const Point_index& p2, const Point_index& p3, const Point_index& p4);

	int cross_cube(int i, int j, int k);

	int cross_octahedron(int i, int j, int k);
};

Isosurface_builder::Isosurface_builder(FE_element *element, FE_value time,
	int number_in_xi1_requested, int number_in_xi2_requested,
	int number_in_xi3_requested, double iso_value,
	Computed_field *coordinate_field, Computed_field *data_field,
	Computed_field *scalar_field, Computed_field *texture_coordinate_field) :
		element(element), time(time),
		number_in_xi1(number_in_xi1_requested),
		number_in_xi2(number_in_xi2_requested),
		number_in_xi3(number_in_xi3_requested),
		iso_value(iso_value),
		coordinate_field(coordinate_field),
		data_field(data_field),
		scalar_field(scalar_field),
		texture_coordinate_field(texture_coordinate_field)
{
	enum FE_element_shape_type shape_type1, shape_type2, shape_type3;
	FE_element_shape *element_shape = NULL;
	get_FE_element_shape(element, &element_shape);
	get_FE_element_shape_xi_shape_type(element_shape, /*xi_number*/0, &shape_type1);
	get_FE_element_shape_xi_shape_type(element_shape, /*xi_number*/1, &shape_type2);
	get_FE_element_shape_xi_shape_type(element_shape, /*xi_number*/2, &shape_type3);
	
	// GRC add support for polygon shape
	/* Check for simplices */
	simplex12 = (SIMPLEX_SHAPE == shape_type1) && (SIMPLEX_SHAPE == shape_type2);
	simplex23 = (SIMPLEX_SHAPE == shape_type2) && (SIMPLEX_SHAPE == shape_type3);
	simplex13 = (SIMPLEX_SHAPE == shape_type3) && (SIMPLEX_SHAPE == shape_type1);
	tetrahedron = simplex12 && simplex23;
	cube = (!simplex12) && (!simplex23) && (!simplex13);

	if (simplex12)
	{
		number_in_xi2 = number_in_xi1;
	}
	if (simplex13)
	{
		number_in_xi3 = number_in_xi1;
	}
	if (simplex23)
	{
		number_in_xi3 = number_in_xi2;
	}
	// precalculate scalings from index to xi
	delta_xi1 = 1.0 / number_in_xi1;
	delta_xi2 = 1.0 / number_in_xi2;
	delta_xi3 = 1.0 / number_in_xi3;
	plane_size = (number_in_xi1 + 1)*(number_in_xi2 + 1);
	plane_scalars = new double[2*plane_size];
}

Isosurface_builder::~Isosurface_builder()
{
	ENTER(Isosurface_builder::~Isosurface_builder);
	delete[] plane_scalars;
	Computed_field_clear_cache(coordinate_field);
	Computed_field_clear_cache(scalar_field);
	if (data_field)
	{
		Computed_field_clear_cache(data_field);
	}
	if (texture_coordinate_field)
	{
		Computed_field_clear_cache(texture_coordinate_field);
	}
	LEAVE;
}

const Iso_vertex *Isosurface_builder::compute_line_crossing(
	const Point_index_pair& pp)
{
	ENTER(Isosurface_builder::compute_line_crossing);
	Iso_vertex vertex;
	double scalar_a = get_scalar(pp.pa);
	double scalar_b = get_scalar(pp.pb);
	double r = (iso_value - scalar_a) / (scalar_b - scalar_a);
	double inverse_r = 1.0 - r;
	FE_value xi_a[3], xi_b[3];
	get_xi(pp.pa, xi_a);
	get_xi(pp.pb, xi_b);
	vertex.xi[0] = xi_a[0]*inverse_r + xi_b[0]*r;
	vertex.xi[1] = xi_a[1]*inverse_r + xi_b[1]*r;
	vertex.xi[2] = xi_a[2]*inverse_r + xi_b[2]*r;
	// future: option to iterate to get exact xi crossing for non-linear fields
	// likewise should eventually recalculate coordinates at exact xi; linearly interpolate for now
	//crossing_coords[0] = (double)point_coords[mp1][0]*inverse_r + (double)point_coords[mp2][0]*r;
	//crossing_coords[1] = (double)point_coords[mp1][1]*inverse_r + (double)point_coords[mp2][1]*r;
	//crossing_coords[2] = (double)point_coords[mp1][2]*inverse_r + (double)point_coords[mp2][2]*r;
	// temp: evaluate coordinates at the found xi:
	Computed_field_evaluate_in_element(coordinate_field, element, vertex.xi,
		time, /*top_level_element*/(struct FE_element *)NULL, vertex.coordinates,
		/*derivatives*/(FE_value *)NULL);
	const Iso_vertex* new_vertex = mesh.add_vertex(pp, vertex);
	LEAVE;
	
	return (new_vertex);
}

/***************************************************************************//**
 *
 * @param p0..p3  Point index locations of tetrahedral vertices with winding: 
 * 
 *       4---3
 *      / \.'|
 *     / .'\ |
 *    /.'   \|
 *   1-------2
 * 
 * @return  1 on success, 0 on failure
 */
void Isosurface_builder::cross_tetrahedron(const Point_index& p1,
	const Point_index& p2, const Point_index& p3, const Point_index& p4)
{
	// first column is final case = number of points > iso_value, or 0 for no crossings
	// columns 1-4 are indexes of initial vertices in order to rotate to final case:
	// 0 : ignore
	// 1 : p1 > iso_value
	// 2 : p1, p2 > iso_value
	// 3 : p1, p2, p3 > iso_value
	static const int tet_case[16][5] =
	{
		{ 0, 0, 0, 0, 0 }, // [nothing]   > iso_value
		{ 1, 0, 1, 2, 3 }, // p1          > iso_value
		{ 1, 1, 0, 3, 2 }, //    p2       > iso_value
		{ 2, 0, 1, 2, 3 }, // p1 p2       > iso_value
		{ 1, 2, 0, 1, 3 }, //       p3    > iso_value
		{ 2, 0, 2, 3, 1 }, // p1    p3    > iso_value
		{ 2, 1, 2, 0, 3 }, //    p2 p3    > iso_value
		{ 3, 0, 1, 2, 3 }, // p1 p2 p3    > iso_value
		{ 1, 3, 0, 2, 1 }, //          p4 > iso_value
		{ 2, 0, 3, 1, 2 }, // p1       p4 > iso_value
		{ 2, 1, 3, 2, 0 }, //    p2    p4 > iso_value
		{ 3, 0, 3, 1, 2 }, // p1 p2    p4 > iso_value
		{ 2, 2, 3, 0, 1 }, //       p3 p4 > iso_value
		{ 3, 0, 2, 3, 1 }, // p1    p3 p4 > iso_value
		{ 3, 1, 3, 2, 0 }, //    p2 p3 p4 > iso_value
		{ 0, 0, 0, 0, 0 }, // p1 p2 p3 p4 > iso_value
	}
	
	ENTER(Isosurface_builder::cross_tetrahedron);
	int unrotated_case =	
		((get_scalar(p1) > iso_value) ? 1 : 0) +
		((get_scalar(p2) > iso_value) ? 2 : 0) +
		((get_scalar(p3) > iso_value) ? 4 : 0) +
		((get_scalar(p4) > iso_value) ? 8 : 0);
	int final_case = tet_case[unrotated_case][0];
	if (0 < final_case)
	{
		Point_index mp[4] = { p1, p2, p3, p4 };
		Point_index mp1 = mp[tet_case[unrotated_case][1]];
		Point_index mp2 = mp[tet_case[unrotated_case][2]];
		Point_index mp3 = mp[tet_case[unrotated_case][3]];
		Point_index mp4 = mp[tet_case[unrotated_case][4]];
		const Iso_vertex *v1, *v2, *v3, *v4;
		if (final_case == 1)
		{
			v1 = get_line_crossing(Point_index_pair(mp1, mp2));
			v2 = get_line_crossing(Point_index_pair(mp1, mp3));
			v3 = get_line_crossing(Point_index_pair(mp1, mp4));
			mesh.add_triangle(v1,v3,v2);
		}
		else if (final_case == 2)
		{
			v1 = get_line_crossing(Point_index_pair(mp1, mp3));
			v2 = get_line_crossing(Point_index_pair(mp1, mp4));
			v3 = get_line_crossing(Point_index_pair(mp2, mp3));
			v4 = get_line_crossing(Point_index_pair(mp2, mp4));
			// cut quadrilateral across the shortest diagonal
			if (square_distance3(v1->coordinates, v4->coordinates) <
				square_distance3(v2->coordinates, v3->coordinates))
			{
				mesh.add_triangle(v1,v4,v2);
				mesh.add_triangle(v1,v3,v4);
			}
			else
			{
				mesh.add_triangle(v1,v3,v2);
				mesh.add_triangle(v2,v3,v4);
			}
		}
		else if (final_case == 3)
		{
			v1 = get_line_crossing(Point_index_pair(mp1, mp4));
			v2 = get_line_crossing(Point_index_pair(mp2, mp4));
			v3 = get_line_crossing(Point_index_pair(mp3, mp4));
			mesh.add_triangle(v1,v3,v2);
		}
	}
	LEAVE;
}

int Isosurface_builder::cross_cube(int i, int j, int k)
{
	ENTER(Isosurface_builder::cross_cube);
	int return_code = 1;
	Point_index p[8];
	p[0].set(i    , j    , k    );
	p[1].set(i + 1, j    , k    );
	p[2].set(i    , j + 1, k    );
	p[3].set(i + 1, j + 1, k    );
	p[4].set(i    , j    , k + 1);
	p[5].set(i + 1, j    , k + 1);
	p[6].set(i    , j + 1, k + 1);
	p[7].set(i + 1, j + 1, k + 1);
	bool first_above = (get_scalar(p[0]) > iso_value);
	int n;
	for (n = 1; n < 8; n++)
	{
		if ((get_scalar(p[n]) > iso_value) != first_above)
		{
			break;
		}
	}
	if (n < 8)
	{
		// GRC temp: subdivide into 6 tetrahedra. Note: non-symmetric
		// Future: divide into pyramids with centre point sample
		cross_tetrahedron(p[0], p[1], p[2], p[4]);
		cross_tetrahedron(p[1], p[6], p[2], p[4]);
		cross_tetrahedron(p[1], p[6], p[3], p[2]);
		cross_tetrahedron(p[1], p[6], p[5], p[3]);
		cross_tetrahedron(p[1], p[6], p[4], p[5]);
		cross_tetrahedron(p[3], p[5], p[7], p[6]);
	}
	LEAVE;

	return (return_code);
}

int Isosurface_builder::cross_octahedron(int i, int j, int k)
{
	ENTER(Isosurface_builder::cross_octahedron);
	int return_code = 1;
	Point_index p[6];
	p[0].set(i + 1, j    , k    );
	p[1].set(i    , j + 1, k    );
	p[2].set(i + 1, j + 1, k    );
	p[3].set(i    , j    , k + 1);
	p[4].set(i + 1, j    , k + 1);
	p[5].set(i    , j + 1, k + 1);
	bool first_above = (get_scalar(p[0]) > iso_value);
	int n;
	for (n = 1; n < 6; n++)
	{
		if ((get_scalar(p[n]) > iso_value) != first_above)
		{
			break;
		}
	}
	if (n < 6)
	{
		// GRC temp: subdivide into 4 tetrahedra. Note: non-symmetric
		// Future: handle as special case
		cross_tetrahedron(p[0], p[5], p[1], p[3]);
		cross_tetrahedron(p[0], p[5], p[2], p[1]);
		cross_tetrahedron(p[0], p[5], p[4], p[2]);
		cross_tetrahedron(p[0], p[5], p[3], p[4]);
	}
	LEAVE;

	return (return_code);
}

int Isosurface_builder::do_it()
{
	ENTER(Isosurface_builder::do_it);
	int return_code = 1;
	double scalar_value;
	FE_value scalar_FE_value, xi[3];
	// optimisation: don't check individual cell crossings until first scalar
	// on other side of iso_value
	// (further optimisation: reset to value of last plane)
	bool iso_surface_crosses_element = false;
	bool first_point = true;
	bool first_scalar_over;

	for (int k = number_in_xi3; (k >= 0) && return_code; k--)
	{
		xi[2] = k*delta_xi3;
		int mod_number_in_xi2 = number_in_xi2;
		if (simplex23)
		{
			mod_number_in_xi2 -= k; // assume number_in_xi equal on linked axes
		}

		for (int j = mod_number_in_xi2; (j >= 0) && return_code; j--)
		{
			xi[1] = j*delta_xi2;
			int mod_number_in_xi1 = number_in_xi1;
			if (simplex12)
			{
				mod_number_in_xi1 -= j;
			}
			if (simplex13)
			{
				mod_number_in_xi1 -= k;
			}

			for (int i = mod_number_in_xi1; (i >= 0) && return_code; i--)
			{
				xi[0] = i*delta_xi1; 
				if (Computed_field_evaluate_in_element(scalar_field, element, xi,
					time, /*top_level_element*/(struct FE_element *)NULL, &scalar_FE_value,
					/*derivatives*/(FE_value *)NULL))
				{
					scalar_value = static_cast<double>(scalar_FE_value);
					set_scalar(Point_index(i, j, k), scalar_value);
					if (first_point)
					{
						first_scalar_over = scalar_value > iso_value;
						first_point = false;
					}
					else if ((scalar_value > iso_value) != first_scalar_over)
					{
						iso_surface_crosses_element = true;
					}

					if (iso_surface_crosses_element && (i < mod_number_in_xi1) &&
						(j < mod_number_in_xi2) && (k < number_in_xi3))
					{
						if (cube)
						{
							return_code = cross_cube(i, j, k);
						}
						else if (tetrahedron)
						{
							if (i < (mod_number_in_xi1 - 1))
							{
								if (i < (mod_number_in_xi1 - 2))
								{
									cross_tetrahedron(
										Point_index(i + 1, j + 1, k    ),
										Point_index(i    , j + 1, k + 1),
										Point_index(i + 1, j    , k + 1),
										Point_index(i + 1, j + 1, k + 1));
								}
								cross_octahedron(i, j, k);
							}
							cross_tetrahedron(
								Point_index(i, j, k),
								Point_index(i + 1, j, k),
								Point_index(i, j + 1, k),
								Point_index(i, j, k + 1));
						}
						else if (simplex12)
						{
							// not implemented
						}
						else if (simplex23)
						{
							// not implemented
						}
						else if (simplex13)
						{
							// not implemented
						}
					}
				}
				else
				{
					return_code = 0;
				}
			}
		}
	}
	LEAVE;
	
	return (return_code);
}

} // anonymous namespace

/*
Global functions
----------------
*/

int create_iso_surfaces_from_FE_element_new(struct FE_element *element,
	FE_value time, int *number_in_xi, double iso_value, 
	struct Computed_field *coordinate_field, struct Computed_field *data_field,
	struct Computed_field *scalar_field,
	struct Computed_field *texture_coordinate_field,
	struct GT_object *graphics_object, enum Render_type render_type)
{
	ENTER(create_iso_surfaces_from_FE_element2);
	int return_code = 0;
	if ((NULL != element) && (3 == get_FE_element_dimension(element)) &&
		Computed_field_has_3_components(coordinate_field, NULL) &&
		(NULL != number_in_xi) && (0 < number_in_xi[0]) && (0 < number_in_xi[1]) && (0 < number_in_xi[2])
		&& (1 == Computed_field_get_number_of_components(scalar_field) &&
		((NULL == texture_coordinate_field) ||
			(3 >= Computed_field_get_number_of_components(texture_coordinate_field))))
		&& graphics_object)
	{
		Isosurface_builder iso_builder(element, time, number_in_xi[0],
			number_in_xi[1], number_in_xi[2], iso_value, 
			coordinate_field, data_field, scalar_field, texture_coordinate_field);
		return_code = iso_builder.do_it();
		if (return_code)
		{
			const Iso_triangle_list& triangle_list = iso_builder.mesh.triangle_list;
			const int number_of_triangles = triangle_list.size();
			if (0 < number_of_triangles)
			{
				Triple *normalpoints, *points;
				ALLOCATE(points, Triple, number_of_triangles*3);
				ALLOCATE(normalpoints, Triple, number_of_triangles*3);
				Triple *tangentpoints = NULL;
				Triple *texturepoints = NULL;
				int n_data_components = 0;
				GTDATA *data = NULL;
				if ((NULL != points) && (NULL != normalpoints))
				{
					Triple *point = points;
					Triple *normal = normalpoints;
					// GRC fill points and calc normals
					for (Iso_triangle_list_const_iterator triangle_iter = triangle_list.begin(); triangle_iter != triangle_list.end(); triangle_iter++)
					{
						const Iso_triangle *triangle = *triangle_iter;
						const Iso_vertex* v1 = triangle->v1;
						const Iso_vertex* v2 = triangle->v2;
						const Iso_vertex* v3 = triangle->v3;
						// calculate normal from vertices:
						FE_value axis1[3], axis2[3], facet_normal[3];
						axis1[0] = v2->coordinates[0] - v1->coordinates[0];
						axis1[1] = v2->coordinates[1] - v1->coordinates[1];
						axis1[2] = v2->coordinates[2] - v1->coordinates[2];
						axis2[0] = v3->coordinates[0] - v1->coordinates[0];
						axis2[1] = v3->coordinates[1] - v1->coordinates[1];
						axis2[2] = v3->coordinates[2] - v1->coordinates[2];
						cross_product_FE_value_vector3(axis1, axis2, facet_normal);
						normalize_FE_value3(facet_normal);
						(*point)[0] = (float)v1->coordinates[0]; 
						(*point)[1] = (float)v1->coordinates[1]; 
						(*point)[2] = (float)v1->coordinates[2];
						point++;
						(*point)[0] = (float)v2->coordinates[0]; 
						(*point)[1] = (float)v2->coordinates[1]; 
						(*point)[2] = (float)v2->coordinates[2];
						point++;
						(*point)[0] = (float)v3->coordinates[0]; 
						(*point)[1] = (float)v3->coordinates[1]; 
						(*point)[2] = (float)v3->coordinates[2];
						point++;
						for (int i = 0; i < 3; i++)
						{
							(*normal)[0] = (float)facet_normal[0];
							(*normal)[1] = (float)facet_normal[1];
							(*normal)[2] = (float)facet_normal[2];
							normal++;
						}
					}
				}
				else
				{
					return_code = 0;
				}
				const GT_surface_type surface_type = (render_type == RENDER_TYPE_WIREFRAME) ?
					g_WIREFRAME_SH_DISCONTINUOUS_TEXMAP : g_SH_DISCONTINUOUS_TEXMAP;
				if (return_code)
				{
					GT_surface *surface = CREATE(GT_surface)(surface_type, /*polygon_type*/g_TRIANGLE,
						/*number_of_points_in_xi1*/number_of_triangles, /*number_of_points_in_xi2*/3, points,
						normalpoints, tangentpoints, texturepoints, n_data_components, data);
					if (!GT_OBJECT_ADD(GT_surface)(graphics_object, time, surface))
					{
						DESTROY(GT_surface)(&surface);
						return_code = 0;
					}
				}
				else
				{
					DEALLOCATE(points);
					DEALLOCATE(normalpoints);
				}
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"create_iso_surfaces_from_FE_element2.  Invalid argument(s)");
	}
	LEAVE;

	return (return_code);
} /* create_iso_surfaces_from_FE_element2 */
