/*******************************************************************************
FILE : photoface_cmiss_utilities.h

LAST MODIFIED : 10 June 2001

DESCRIPTION :
File writing routines that may be needed by Photoface.  All functions have an
integer return code - zero is success, non-zero is failure.
==============================================================================*/
#if !defined (PHOTOFACE_CMISS_UTILITIES_H)
#define PHOTOFACE_CMISS_UTILITIES_H

#if defined (WIN32) && defined (PHOTOFACE_CMISS_EXPORTS)
#if defined (CMISSDLLEXPORT)
#define CMISSDECLSPEC __declspec( dllexport )
#else /* defined (CMISSDLLEXPORT)*/
#define CMISSDECLSPEC __declspec( dllimport )
#endif /* defined (CMISSDLLEXPORT)*/
#else /* defined (WIN32) && defined (PHOTOFACE_CMISS_EXPORTS) */
#define CMISSDECLSPEC
#endif /* defined (WIN32) && defined (PHOTOFACE_CMISS_EXPORTS) */

#if defined (__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */

/*
Global functions
----------------
*/
CMISSDECLSPEC int pf_write_head_model(char *obj_file_name,
	int number_of_vertices,float *vertex_3d_locations,
	int number_of_texture_vertices,float *texture_vertex_3d_locations,
	int number_of_triangles,int *triangle_vertices,
	int *triangle_texture_vertices);
/*******************************************************************************
LAST MODIFIED : 10 June 2001

DESCRIPTION :
Writes the head model

<vertex_3d_locations> a 1-D array of 3*<number_of_vertices> floats specifying
  the world locations of the vertices (vertex number varying slowest)
<texture_vertex_3d_locations> a 1-D array of 3*<number_of_texture_vertices>
  floats specifying the texture locations of the texture vertices (vertex number
  varying slowest)
<triangle_vertices> a 1-D array of 3*<number_of_triangles> ints giving the
  vertex numbers for each triangle
<triangle_texture_vertices> a 1-D array of 3*<number_of_triangles> ints giving
  the texture vertex numbers for each triangle

to the specified <obj_file>.
==============================================================================*/

CMISSDECLSPEC int pf_write_basis(char *basis_file_name,int number_of_modes,
	int number_of_vertices,float *vertex_3d_locations_or_offsets);
/*******************************************************************************
LAST MODIFIED : 10 June 2001

DESCRIPTION :
Writes the basis 

<vertex_3d_locations_or_offsets> which is a 1-D array of 
3*<number_of_modes>*<number_of_vertices> floats with x,y,z varying fastest and
mode number slowest 

to the specified <basis_file>.
==============================================================================*/

CMISSDECLSPEC int pf_write_texture(char *jpeg_file_name,int width,int height,
	char *texture);
/*******************************************************************************
LAST MODIFIED : 10 June 2001

DESCRIPTION :
Writes the <texture> to the <jpeg_file>.
==============================================================================*/

CMISSDECLSPEC int pf_write_scene_graph(char *scene_graph_file_name,
	float *eye_point,float *interest_point,float *up_vector,float view_angle,
	float *left_eye,float *right_eye);
/*******************************************************************************
LAST MODIFIED : 10 June 2001

DESCRIPTION :
Writes the <scene_graph_file> from the information returned by <pf_get_view>
and <pf_get_marker_fitted_positions>.
==============================================================================*/

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

#endif /* !defined (PHOTOFACE_CMISS_UTILITIES_H) */
