/*******************************************************************************
FILE : photoface_cmiss.h

LAST MODIFIED : 21 June 2001

DESCRIPTION :
The functions that interface Photoface to cmiss.  All functions have an integer
return code - zero is success, non-zero is failure.
==============================================================================*/
#if !defined (PHOTOFACE_CMISS_H)
#define PHOTOFACE_CMISS_H

#define LINUX_CMISS

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
Global constants
----------------
*/
/*******************************************************************************
LAST MODIFIED : 13 February 2001

DESCRIPTION :
Return codes for the interface functions.
==============================================================================*/
#define PF_SUCCESS_RC ((int)0)
#define PF_GENERAL_FAILURE_RC ((int)-1)
#define PF_ALLOCATE_FAILURE_RC ((int)-2)
#define PF_FIND_FILE_FAILURE_RC ((int)-3)
#define PF_SYSTEM_COMMAND_FAILURE_RC ((int)-4)
#define PF_INVALID_ARGUMENTS_FAILURE_RC ((int)-5)
#define PF_READ_FILE_FAILURE_RC ((int)-6)
#define PF_WRITE_FILE_FAILURE_RC ((int)-7)
#define PF_OPEN_FILE_FAILURE_RC ((int)-8)

/*
Global types
------------
*/
enum PF_image_format
/*******************************************************************************
LAST MODIFIED : 16 January 2001

DESCRIPTION :
The image formats that the interface supports
==============================================================================*/
{
  PF_RGB_IMAGE
}; /* enum PF_image_format */

/*
Global functions
----------------
*/
#if defined (LINUX_CMISS)
CMISSDECLSPEC int pf_specify_paths(char *photoface_main_windows_path,
	char *photoface_main_linux_path);
/*******************************************************************************
LAST MODIFIED : 13 February 2001

DESCRIPTION :
Specifies the main directory under which the model, working and cmiss directory
are expected to be located under Windows and Linux.  The paths should end in a
delimiting /.  This function will be obsolete once a windows only version of the
Photoface cmiss library is built.

If either path is NULL then the internal storage for that path is free'd.
==============================================================================*/
#endif /* defined (LINUX_CMISS) */

CMISSDECLSPEC int pf_setup(char *model_name,char *state,int *pf_job_id);
/*******************************************************************************
LAST MODIFIED : 9 May 2001

DESCRIPTION :
<model_name> is the name of head model.  cmiss will have a directory containing
the files (generic obj etc) need for each model.  <state> is additional
information about the face in the image, such as "smiling", which may allow
adjustment of the generic head.  On success, the <pf_job_id> is set.
==============================================================================*/

CMISSDECLSPEC int pf_close(int pf_job_id);
/*******************************************************************************
LAST MODIFIED : 9 May 2001

DESCRIPTION :
Closes the photoface job, freeing any internal memory and stopping any
internal processes associated with the job.
==============================================================================*/

CMISSDECLSPEC int pf_define_markers(int pf_job_id,int number_of_markers,char **marker_names,
  float *marker_3d_generic_positions);
/*******************************************************************************
LAST MODIFIED : 9 May 2001

DESCRIPTION :
Define <number_of_markers> using
<marker_names> an array of <number_of_markers> marker names
<marker_3d_generic_positions> an array of 3*<number_of_markers> floats giving
  the locations for the markers in the generic model (marker number varying
  slowest).
If a marker already exists for the model then its position in the generic head
model will be updated, other wise a new marker will be created.
Please note that
1.  These are the positions on the generic head not the one being created.  This
  is like in the pipeline where you click on the image and the corresponding
  point on the generic model.  Its the points on the generic head that would be
  passed to pf_define_markers.
2.  pf_define_markers involves cmiss doing an optimization to get the material
  points.
==============================================================================*/

CMISSDECLSPEC int pf_specify_markers(int pf_job_id,int number_of_markers,char **marker_names,
  float *marker_2d_positions,float *marker_confidences);
/*******************************************************************************
LAST MODIFIED : 9 May 2001

DESCRIPTION :
Specify <number_of_markers> using
<marker_names> an array of <number_of_markers> marker names.  The marker names
  will come from Ben's XML specification or MPEG4.  A particular model may not
  have material positions for all markers, in which case the markers it doesn't
  know about will be ignored.
<marker_2d_positions> an array of 2*<number_of_markers> floats giving the
  measured locations for the markers on the particular image (marker number
  varying slowest).
<marker_confidences> an array of <number_of_markers> positive floats giving
  the relative confidences for the marker positions.  For the two markers the
  one with the larger confidence has a better position estimate.
==============================================================================*/

CMISSDECLSPEC int pf_get_marker_fitted_positions(int pf_job_id,int number_of_markers,
	char **marker_names,float *marker_fitted_3d_positions);
/*******************************************************************************
LAST MODIFIED : 9 May 2001

DESCRIPTION :
Returns the fitted positions of the markers in <marker_fitted_3d_positions>
which is assumed to be allocated large enough for 3*<number_of_markers> floats
(marker number varying slowest).
==============================================================================*/

CMISSDECLSPEC int pf_view_align(int pf_job_id,float *error_measure);
/*******************************************************************************
LAST MODIFIED : 9 May 2001

DESCRIPTION :
Calculates the view that aligns the model to the specified markers and returns
an <error_measure>.
==============================================================================*/

CMISSDECLSPEC int pf_get_view(int pf_job_id,float *eye_point,float *interest_point,
	float *up_vector,float *view_angle);
/*******************************************************************************
LAST MODIFIED : 9 May 2001

DESCRIPTION :
Returns the current view as an <eye_point> (3 component vector), an
<interest_point> (3 component vector), an <up_vector> (3 component vector) and a
<view_angle> (scalar).  Assumes that all storage has been assigned large enough.
==============================================================================*/

CMISSDECLSPEC int pf_specify_view(int pf_job_id,float *eye_point,float *interest_point,
	float *up_vector,float view_angle);
/*******************************************************************************
LAST MODIFIED : 9 May 2001

DESCRIPTION :
Sets the current view as an <eye_point> (3 component vector), an
<interest_point> (3 component vector), an <up_vector> (3 component vector) and a
<view_angle> (scalar).  It is an alternative/override for pf_view_align.
==============================================================================*/

CMISSDECLSPEC int pf_fit(int pf_job_id,float *error_measure);
/*******************************************************************************
LAST MODIFIED : 9 May 2001

DESCRIPTION :
Fits the model to the specified markers, using the current transformation
matrix, and returns an <error_measure>.
==============================================================================*/

CMISSDECLSPEC int pf_get_head_model(int pf_job_id,int *number_of_vertices,
	float **vertex_3d_locations,int *number_of_texture_vertices,
	float **texture_vertex_3d_locations,int *number_of_triangles,
	int **triangle_vertices,int **triangle_texture_vertices);
/*******************************************************************************
LAST MODIFIED : 9 May 2001

DESCRIPTION :
Returns the current transformed generic head as
<vertex_3d_locations> a 1-D array of 3*<number_of_vertices> floats specifying
  the world locations of the vertices (vertex number varying slowest)
<texture_vertex_3d_locations> a 1-D array of 3*<number_of_texture_vertices>
  floats specifying the texture locations of the texture vertices (vertex number
  varying slowest)
<triangle_vertices> a 1-D array of 3*<number_of_triangles> ints giving the
  vertex numbers for each triangle
<triangle_texture_vertices> a 1-D array of 3*<number_of_triangles> ints giving
  the texture vertex numbers for each triangle
==============================================================================*/

CMISSDECLSPEC int pf_get_basis(int pf_job_id,int *number_of_modes,int *number_of_vertices,
  float **vertex_3d_locations_or_offsets);
/*******************************************************************************
LAST MODIFIED : 9 May 2001

DESCRIPTION :
Returns the basis for the current transformed model in
<vertex_3d_locations_or_offsets> which is a 1-D array of
3*<number_of_modes>*<number_of_vertices> floats with x,y,z varying fastest and
mode number fastest.
==============================================================================*/

CMISSDECLSPEC int pf_specify_image(int pf_job_id,int width,int height,
	enum PF_image_format image_format,char *image);
/*******************************************************************************
LAST MODIFIED : 9 May 2001

DESCRIPTION :
Used to specify the image to be texture mapped onto the model.
==============================================================================*/

CMISSDECLSPEC int pf_get_texture(int pf_job_id,int width,int height,char *texture);
/*******************************************************************************
LAST MODIFIED : 9 May 2001

DESCRIPTION :
The caller specifies the texture size and provides the storage.  The <texture>
is filled in based on the current model.
==============================================================================*/

CMISSDECLSPEC int pf_get_hair_model(int pf_job_id,int *number_of_vertices,
	float **vertex_3d_locations,int *number_of_texture_vertices,
	float **texture_vertex_3d_locations,int *number_of_triangles,
	int **triangle_vertices,int **triangle_texture_vertices);
/*******************************************************************************
LAST MODIFIED : 21 June 2001

DESCRIPTION :
Returns the current transformed generic head as
<vertex_3d_locations> a 1-D array of 3*<number_of_vertices> floats specifying
  the world locations of the vertices (vertex number varying slowest)
<texture_vertex_3d_locations> a 1-D array of 3*<number_of_texture_vertices>
  floats specifying the texture locations of the texture vertices (vertex number
  varying slowest)
<triangle_vertices> a 1-D array of 3*<number_of_triangles> ints giving the
  vertex numbers for each triangle
<triangle_texture_vertices> a 1-D array of 3*<number_of_triangles> ints giving
  the texture vertex numbers for each triangle
==============================================================================*/

CMISSDECLSPEC int pf_specify_hair_mask(int pf_job_id,int width,int height,
	enum PF_image_format image_format,char *image);
/*******************************************************************************
LAST MODIFIED : 21 June 2001

DESCRIPTION :
Used to specify the image to be texture mapped onto the model.
==============================================================================*/

CMISSDECLSPEC int pf_get_hair_texture(int pf_job_id,int width,int height,
	char *texture);
/*******************************************************************************
LAST MODIFIED : 21 June 2001

DESCRIPTION :
The caller specifies the texture size and provides the storage.  The <texture>
is filled in based on the current model.
==============================================================================*/

#if defined (__cplusplus)
}
#endif /* defined (__cplusplus) */

#endif /* !defined (PHOTOFACE_CMISS_H) */
