/*******************************************************************************
FILE : delaunay.h

LAST MODIFIED : 18 April 2004

DESCRIPTION :
Specialized implementations of Delaunay triangulation for a cylinder and a
sphere.
==============================================================================*/
#if !defined (DELAUNAY_H)
#define DELAUNAY_H

/*
Global functions
----------------
*/
int cylinder_delaunay(int number_of_vertices,float *vertices,
	int *number_of_triangles_address,int **triangles_address);
/*******************************************************************************
LAST MODIFIED : 3 February 2002

DESCRIPTION :
Calculates the Delaunay triangulation of the <vertices> on a cylinder whose axis
is z.
<number_of_vertices> is the number of vertices to be triangulated
<vertices> is an array of length 3*<number_of_vertices> containing the x,y,z
	coordinates of the vertices
<*number_of_triangles_address> is the number of triangles calculated by the
	function
<*triangles_address> is an array of length 3*<*number_of_triangles_address>,
	allocated by the function, containing the vertex numbers for each triangle
==============================================================================*/

int sphere_delaunay(int number_of_vertices,float *vertices,
	int *number_of_triangles_address,int **triangles_address);
/*******************************************************************************
LAST MODIFIED : 3 February 2002

DESCRIPTION :
Calculates the Delaunay triangulation of the <vertices> on a sphere whose centre
is the origin.
<number_of_vertices> is the number of vertices to be triangulated
<vertices> is an array of length 3*<number_of_vertices> containing the x,y,z
	coordinates of the vertices
<*number_of_triangles_address> is the number of triangles calculated by the
	function
<*triangles_address> is an array of length 3*<*number_of_triangles_address>,
	allocated by the function, containing the vertex numbers for each triangle
==============================================================================*/

int plane_delaunay(int number_of_vertices,float *vertices,
	int *number_of_triangles_address,int **triangles_address);
/*******************************************************************************
LAST MODIFIED : 18 April 2004

DESCRIPTION :
Calculates the Delaunay triangulation of the <vertices> on a plane.
<number_of_vertices> is the number of vertices to be triangulated
<vertices> is an array of length 2*<number_of_vertices> containing the x,y
	coordinates of the vertices
<*number_of_triangles_address> is the number of triangles calculated by the
	function
<*triangles_address> is an array of length 3*<*number_of_triangles_address>,
	allocated by the function, containing the vertex numbers for each triangle
==============================================================================*/
#endif /* !defined (DELAUNAY_H) */
