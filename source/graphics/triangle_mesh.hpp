/***************************************************************************//**
 * FILE : triangle_mesh.hpp
 *
 * Class for representing a simple linear triangular mesh with facilities for
 * merging vertices whose coordinates are within a tolerance.
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

#if !defined (TRIANGLE_MESH)
#define TRIANGLE_MESH

#include <list>
#include <set>

class Triangle_vertex
{
private:
	float coordinates[3];
	int identifier;

	friend class Triangle_vertex_compare;
	friend class Mesh_triangle;
	friend class Triangle_mesh;

public:
	Triangle_vertex(const float *in_coordinates) :
		identifier(0)
	{
		coordinates[0] = in_coordinates[0];
		coordinates[1] = in_coordinates[1];
		coordinates[2] = in_coordinates[2];
	}
	
	~Triangle_vertex()
	{
	}

	void get_coordinates(float *coord1, float *coord2, float *coord3) const;

	void set_identifier(int in_identifier)
	{
		identifier = in_identifier;
	}
	
	int get_identifier() const
	{
		return identifier;
	}
	
	void list() const;
};

/***************************************************************************//**
 * Functor for comparing/sorting Triangle vertex based on coordinates but with
 * an allowed tolerance.
 */
class Triangle_vertex_compare
{
private:
	float tolerance;

public:
	Triangle_vertex_compare(float in_tolerance) :
		tolerance(in_tolerance)
	{	
	}

	bool operator() (const Triangle_vertex *v1, const Triangle_vertex *v2);
};

typedef std::set<Triangle_vertex*,Triangle_vertex_compare> Triangle_vertex_set;
typedef std::set<Triangle_vertex*,Triangle_vertex_compare>::iterator Triangle_vertex_set_iterator;
typedef std::set<Triangle_vertex*,Triangle_vertex_compare>::const_iterator Triangle_vertex_set_const_iterator;

class Mesh_triangle
{
private:
	const Triangle_vertex *trivertex1;
	const Triangle_vertex *trivertex2;
	const Triangle_vertex *trivertex3;
	
public:
	Mesh_triangle(const Triangle_vertex *in_vertex1,
		const Triangle_vertex *in_vertex2, const Triangle_vertex *in_vertex3) :
		trivertex1(in_vertex1), trivertex2(in_vertex2), trivertex3(in_vertex3)
	{
	}
	
	void get_vertexes(const Triangle_vertex **vertex1,
		const Triangle_vertex **vertex2, const Triangle_vertex **vertex3) const;
	
	void list() const;
};

typedef std::list<Mesh_triangle*> Mesh_triangle_list;
typedef std::list<Mesh_triangle*>::iterator Mesh_triangle_list_iterator;
typedef std::list<Mesh_triangle*>::const_iterator Mesh_triangle_list_const_iterator;

class Triangle_mesh
{
private:
	Triangle_vertex_compare compare;
	Triangle_vertex_set vertex_set;
	Mesh_triangle_list triangle_list;
	
public:
	Triangle_mesh(float tolerance) :
		compare(tolerance),
		vertex_set(compare),
		triangle_list()
	{
	}

	~Triangle_mesh();
	
	/***************************************************************************//**
	 * Either finds an existing vertex within the tolerance of the supplied
	 * coordinates, or creates one.
	 *
	 * @param coordinates  Pointer to 3 float values giving x, y, z coordinates. 
	 * @return  Pointer to const vertex with supplied coordinates or within mesh
	 * tolerance thereof.
	 */
	const Triangle_vertex *add_vertex(const float *coordinates);

	/***************************************************************************//**
	 * Adds a triangle to the mesh. Degenerate triangles - with repeared vertices
	 * are ignored. Triangle vertices must have anticlockwise winding for normal out
	 * of 2-D screen/page:
	 *   3
	 *   | \
	 *   1--2
	 *   
	 * @return  On success, const pointer to new triangle, or NULL on failure or
	 * degenerate triangle.
	 */
	const Mesh_triangle *add_triangle(const Triangle_vertex *v1,
		const Triangle_vertex *v2, const Triangle_vertex *v3);
	
	void add_triangle_coordinates(const float *c1, const float *c2, const float *c3)
	{
		add_triangle(add_vertex(c1), add_vertex(c2), add_vertex(c3));
	}

	/***************************************************************************//**
	 * Adds a quadrilateral split into two triangles along the shortest diagonal
	 * (assuming coordinates are rectangular cartesian).
	 * @param v1,v2,v3,v4  Vertices in following order (for normal towards reader):
	 *   3--4
	 *   |  |
	 *   1--2
	 */
	void add_quadrilateral(const Triangle_vertex *v1, const Triangle_vertex *v2,
		const Triangle_vertex *v3, const Triangle_vertex *v4);

	void add_quadrilateral_coordinates(const float *c1, const float *c2,
		const float *c3, const float *c4)
	{
		add_quadrilateral(add_vertex(c1), add_vertex(c2), add_vertex(c3), add_vertex(c4));
	}

	void set_vertex_identifiers(int first_identifier);
	
	const Triangle_vertex_set& get_vertex_set() const
	{
		return vertex_set;
	}

	const Mesh_triangle_list& get_triangle_list() const
	{
		return triangle_list;
	}
	
	void list() const;

private:
	// declared but not defined so illegal:
	Triangle_mesh(const Triangle_mesh& in_triangle_mesh);

	// declared but not defined so illegal:
	void operator=(const Triangle_mesh& in_triangle_mesh);
};

#endif /* !defined (TRIANGLE_MESH) */
