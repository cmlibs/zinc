/*******************************************************************************
FILE : delauney.h

LAST MODIFIED : 14 January 2002

DESCRIPTION :
Specialized implementations of Delauney triangulation for a cylinder and a
sphere.
==============================================================================*/
#if !defined (DELAUNEY_H)
#define DELAUNEY_H

/*
Global functions
----------------
*/
int cylinder_delauney(int number_of_vertices,float *vertices,
	int *number_of_triangles_address,int **triangles_address);
/*******************************************************************************
LAST MODIFIED : 7 October 2000

DESCRIPTION :
Calculates the Delauney triangulation of the <vertices> on a cylinder whose axis
is z.
<number_of_vertices> is the number of vertices to be triangulated
<vertices> is an array of length 3*<number_of_vertices>, allocated by the
	function, containing the x,y,z coordinates of the vertices
<*number_of_triangles_address> is the number of triangles calculated by the
	function
<*triangles_address> is an array of length 3*<*number_of_triangles_address>
	containing the vertex numbers for each triangle
==============================================================================*/

int sphere_delauney(int number_of_vertices,float *vertices,
	int *number_of_triangles_address,int **triangles_address);
/*******************************************************************************
LAST MODIFIED : 14 January 2002

DESCRIPTION :
Calculates the Delauney triangulation of the <vertices> on a sphere whose centre
is the origin.
<number_of_vertices> is the number of vertices to be triangulated
<vertices> is an array of length 3*<number_of_vertices>, allocated by the
	function, containing the x,y,z coordinates of the vertices
<*number_of_triangles_address> is the number of triangles calculated by the
	function
<*triangles_address> is an array of length 3*<*number_of_triangles_address>
	containing the vertex numbers for each triangle
==============================================================================*/
#endif /* !defined (DELAUNEY_H) */
