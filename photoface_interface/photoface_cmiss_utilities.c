/*******************************************************************************
FILE : photoface_cmiss_utilities.c

LAST MODIFIED : 10 June 2001

DESCRIPTION :
File writing routines that may be needed by Photoface.  All functions have an
integer return code - zero is success, non-zero is failure.
==============================================================================*/
#ifdef _AFXDLL
#include "stdafx.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include "jpeglib.h"
#include "photoface_cmiss_utilities.h"

/*
Macros
------
*/
#define ALLOCATE( result , type , number ) \
( result = ( type *) malloc( ( number ) * sizeof( type ) ) )

#define DEALLOCATE( ptr ) \
{ free((char *) ptr ); ( ptr )=NULL;}

#define ENTER( function_name )

#define LEAVE

#define REALLOCATE( final , initial , type , number ) \
( final = ( type *) realloc( (void *)( initial ) , \
	( number ) * sizeof( type ) ) )

/*
Module functions
----------------
*/
static void write_basis(const char *filename,int m,int n,float *a,
	int basis_version,int verbose)
/*******************************************************************************
LAST MODIFIED : 28 January 1998

DESCRIPTION :
Create a basis data file.
Copied from filt/lib/emlib.c except for restructuring the storage of a to suit
and removing unused basis version 1 paramters.
==============================================================================*/
{
	int i, j;
	char magic[]  = "em basis data";
	char magic2[] = "em basis 2.0\n";
	FILE* file;

	if (verbose)
	{
		fprintf(stderr,"writing out the basis file \"%s\" version %d....",filename,
			basis_version);
	}
	if (file = fopen(filename,"w"))
	{
		
		switch (basis_version)
		{
			case 1:
			{
				fprintf(stderr, "write_basis():lib/emlib.c",
					"Unable to write basis file version %d\n", basis_version);
			} break;
			case 2:
			{
				fprintf(file, magic2);

				/* Comment/title line */
				fprintf(file, "\n");
				fprintf(file, "%d %d\n", m, n);

				/* Only write A as floats */
				for (i=0;i<n;i++)
				{
					for (j=0;j<m;j++)
					{
						fprintf(file, "%f ", a[i * m + j]);
					}
					fprintf(file, "\n");
				}
			} break;
			default:
			{
				fprintf(stderr, "write_basis():lib/emlib.c",
					"Unknown basis file version %d\n", basis_version);
			} break;
		}
  
		fclose(file);
		if (verbose) fprintf(stderr,"done!\n");
	}
}

/*
Global functions
----------------
*/
CMISSDECLSPEC int pf_write_head_model(const char *obj_file_name,
	int number_of_vertices,int number_of_dynamic_vertices,
	float *vertex_3d_locations,int number_of_texture_vertices,
	float *texture_vertex_3d_locations,int number_of_triangles,
	int *triangle_vertices,int *triangle_texture_vertices)
/*******************************************************************************
LAST MODIFIED : 12 June 2001

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
{
	FILE *file;
	float *vertex;
	int i, return_code;

	ENTER(pf_write_head_model);
	if (file = fopen(obj_file_name,"w"))
	{
		if ((number_of_dynamic_vertices > 0) && 
			(number_of_dynamic_vertices <= number_of_vertices))
		{
			fprintf(file, "# DYNOBJ %d %d\n", number_of_dynamic_vertices,
				(number_of_vertices - number_of_dynamic_vertices));
			fprintf(file, "# dynamic\n");
		}
		vertex = vertex_3d_locations;
		for ( i = 0 ; i < number_of_vertices ; i++)
		{
			if (i + 1 == number_of_dynamic_vertices)
			{
				fprintf(file, "# static\n");
			}
			fprintf(file, "v %f %f %f\n", vertex[0], vertex[1], vertex[2]);
			vertex += 3;
		}
		vertex = texture_vertex_3d_locations;
		for ( i = 0 ; i < number_of_texture_vertices ; i++)
		{
			fprintf(file, "vt %f %f %f\n", vertex[0], vertex[1], vertex[2]);
			vertex += 3;
		}
		for ( i = 0 ; i < number_of_triangles ; i++)
		{
			fprintf(file, "f %d/%d %d/%d %d/%d\n",
				triangle_vertices[i * 3], triangle_texture_vertices[i * 3],
				triangle_vertices[i * 3 + 1], triangle_texture_vertices[i * 3 + 1],
				triangle_vertices[i * 3 + 2], triangle_texture_vertices[i * 3 + 2]);
		}		
		fclose(file);
		return_code=0;
	}
	else
	{
		return_code=1;
	}
	LEAVE;

	return (return_code);
} /* pf_write_head_model */

CMISSDECLSPEC int pf_write_basis(const char *basis_file_name,int number_of_modes,
	int number_of_vertices,float *vertex_3d_locations_or_offsets)
/*******************************************************************************
LAST MODIFIED : 10 June 2001

DESCRIPTION :
Writes the basis

<vertex_3d_locations_or_offsets> which is a 1-D array of
	3*<number_of_modes>*<number_of_vertices> floats with x,y,z varying fastest and
	mode number slowest

to the specified <basis_file>.

Copied from lib/emlib.c
==============================================================================*/
{
	int return_code;

	ENTER(pf_write_basis);
	write_basis(basis_file_name, number_of_vertices * 3, number_of_modes,
		vertex_3d_locations_or_offsets, /*basis_version*/2, /*verbose*/0 );
	return_code=0;
	LEAVE;

	return (return_code);
} /* pf_write_basis */

CMISSDECLSPEC int pf_write_texture(const char *jpeg_file_name,int width,int height,
	char *texture)
/*******************************************************************************
LAST MODIFIED : 10 June 2001

DESCRIPTION :
Writes the <texture> to the <jpeg_file>.

???DB.  From libjpeg.doc
==============================================================================*/
{
	char *texture_ptr;
	FILE *jpeg_file;
	int return_code,row_stride;
	JSAMPROW row_pointer[1];
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;

	ENTER(pf_write_texture);
	return_code=0;
	if (jpeg_file_name&&(0<width)&&(0<height)&&texture)
	{
		if (jpeg_file=fopen(jpeg_file_name,"wb"))
		{
			return_code=0;
			/* 1. Allocate and initialize a JPEG compression object */
			cinfo.err=jpeg_std_error(&jerr);
			jpeg_create_compress(&cinfo);
			/* 2. Specify the destination for the compressed data (eg, a file) */
			jpeg_stdio_dest(&cinfo,jpeg_file);
			/* 3. Set parameters for compression, including image size & colorspace */
			cinfo.image_width=width;
			cinfo.image_height=height;
			cinfo.input_components=3;
			cinfo.in_color_space=JCS_RGB;
			jpeg_set_defaults(&cinfo);
			/* 4. jpeg_start_compress(...) */
			jpeg_start_compress(&cinfo, TRUE);
			/* 5. while (scan lines remain to be written) */
			row_stride=width*3;
			texture_ptr = texture + row_stride * (height - 1);
			while (cinfo.next_scanline<cinfo.image_height)
			{
				row_pointer[0]= texture_ptr;
				jpeg_write_scanlines(&cinfo,row_pointer,1);
				texture_ptr -= row_stride;
			}
			/* 6. jpeg_finish_compress(...) */
			jpeg_finish_compress(&cinfo);
			/* 7. Release the JPEG compression object */
			jpeg_destroy_compress(&cinfo);
			fclose(jpeg_file);
		}
	}
	LEAVE;

	return (return_code);
} /* pf_write_texture */

CMISSDECLSPEC int pf_write_scene_graph(const char *scene_graph_file_name,
	float *eye_point,float *interest_point,float *up_vector,float view_angle,
	float *left_eye,float *right_eye)
/*******************************************************************************
LAST MODIFIED : 10 June 2001

DESCRIPTION :
Writes the <scene_graph_file> from the information returned by <pf_get_view>
and <pf_get_marker_fitted_positions>.
==============================================================================*/
{
	FILE *file;
	int return_code;

	ENTER(pf_write_scene_graph);
	return_code=0;

	if (file = fopen(scene_graph_file_name, "w"))
	{
		fprintf(file, "#LifeF/X Scene Graph V1.0\n\n");
		fprintf(file, "#Default is 0 0 100\n");
		fprintf(file, "eyepoint %g %g %g\n",eye_point[0],eye_point[1],eye_point[2]);
		fprintf(file, "#Default is 0 50 0\n");
		fprintf(file, "interestpoint %g %g %g\n",interest_point[0],interest_point[1],
			interest_point[2]);
		fprintf(file, "#Default is 18, smaller numbers is same as 'zooming' in.\n");
		fprintf(file, "fieldofview %g\n", view_angle);
		fprintf(file, "\n");

		fprintf(file, "	pushmatrix \"Head Movement\"\n");
		fprintf(file, "\n");
		fprintf(file, "   pushTexture unimap_rachel_web  #set texture\n");
		fprintf(file, "      Geoset face # render geoset\n");
		fprintf(file, "\n");
		fprintf(file, "      pushmatrix \"Left Eye\" translate %g %g %g\n",
			left_eye[0], left_eye[1], left_eye[2]);
		fprintf(file, "         Geoset 'eye' # render\n");
		fprintf(file, "      popmatrix       # pop matrix\n");
		fprintf(file, "      \n");
		fprintf(file, "      pushmatrix \"Right Eye\"  translate %g %g %g\n",
			right_eye[0], right_eye[1], right_eye[2]);
		fprintf(file, "         geoset \"eye\" # render geoset\n");
		fprintf(file, "      popmatrix     # pop matrix\n");
		fprintf(file, "\n");
		fprintf(file, "    poptexture\n");
		fprintf(file, "\n");
		fprintf(file, "\n");
		fprintf(file, "#   pushTexture rachel_webhair\n");
		fprintf(file, "#     Geoset hairgeometry\n");
		fprintf(file, "#   poptexture\n");
		fprintf(file, "\n");
		fprintf(file, "popmatrix\n");
		fprintf(file, "\n");

		fclose(file);
	}
	else
	{
		return_code = 1;
	}
	LEAVE;

	return (return_code);
} /* pf_write_scene_graph */
