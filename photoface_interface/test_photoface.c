/*******************************************************************************
FILE : test_photoface.c

LAST MODIFIED : 31 January 2001

DESCRIPTION :
Tests the photoface interface.
==============================================================================*/

#include <stdio.h>
#include "photoface_cmiss.h"
#include "photoface_cmiss_utilities.h"

struct Obj
{ 
  int number_of_vertices;
  float *vertex_3d_locations;
  int number_of_texture_vertices;
  float *texture_vertex_3d_locations;
  int number_of_triangles;
  int *triangle_vertices;
  int *triangle_texture_vertices;
};

#define IMAGE_WIDTH (421)
#define IMAGE_HEIGHT (301)
#define TEXTURE_WIDTH (1024)
#define TEXTURE_HEIGHT (1024)

#if defined (MANUAL_CMISS)
static int display_message_function(char *message, void *user_data)
/*******************************************************************************
LAST MODIFIED : 13 February 2001

DESCRIPTION :
Runs a job through the photoface interface.
==============================================================================*/
{
  printf("%s\n", message);
}

static int display_message_and_wait_function(char *message, void *user_data)
/*******************************************************************************
LAST MODIFIED : 13 February 2001

DESCRIPTION :
Runs a job through the photoface interface.
==============================================================================*/
{
  printf("%s", message);
	printf("Press return to continue\n");
  scanf("%*c");
}
#endif /* defined (MANUAL_CMISS) */

int main(int argc, char **argv)
/*******************************************************************************
LAST MODIFIED : 24 January 2001

DESCRIPTION :
Runs a job through the photoface interface.
==============================================================================*/
{
	char *marker_names[] = {"eye_left_exocanthion",
									"eye_left_endocanthion",
									"eye_right_exocanthion",
									"Eye_Right_Inner_Iris_Top",
									"outside_hair_vertex",
									"nose_tip",
									"nose_midpoint",
									"lips_upper_top_1",
									"jaw_gnathon"},
		image_array[3 * IMAGE_WIDTH * IMAGE_HEIGHT], *image_ptr,
		texture_array[3 * TEXTURE_WIDTH * TEXTURE_HEIGHT];
	char *eye_marker_names[] = {"eyeball_origin_left",
										 "eyeball_origin_right"};
	float marker_2d_positions[] = {25, 30,
											 45, 31,
											 75, 32,
											 67, 31,
											 55, 5,
											 55, 45,
											 55, 50,
											 45, 60,
											 57, 95},
		marker_confidences[] = {1.0,
										1.0,
										1.0,
										1.0,
										1.0,
										1.0,
										1.0,
										1.0,
										1.0},
		marker_fitted_3d_positions[3 * 9],
		eyeball_fitted_3d_positions[3 * 2],
		error, eye_point[3], interest_point[3], up_vector[3], 
		*vertex_3d_locations, view_angle;
	int i, j, number_of_markers = 9, number_of_modes, number_of_vertices,
		pf_job_id;
	struct Obj obj;

#if defined (MANUAL_CMISS)
	pf_set_display_message_function(PF_ERROR_MESSAGE,display_message_function,NULL);
	pf_set_display_message_function(PF_INFORMATION_MESSAGE,display_message_and_wait_function,NULL);
	pf_set_display_message_function(PF_WARNING_MESSAGE,display_message_function,NULL);
#endif /* defined (MANUAL_CMISS) */

	/* pf_specify_paths("/hosts/netapp/lifefx/data/photoface/", "/hosts/netapp/lifefx/data/photoface/"); */
	/* pf_specify_paths("/hosts/netapp/home/shane/photoface/", "/hosts/netapp/home/shane/photoface/"); */
	/* pf_specify_paths("/blackett/mirage/photoface/", "/blackett/mirage/photoface/"); */
	pf_specify_paths("/home/blackett/lifefx/photoface/", "/home/blackett/lifefx/photoface/");

	pf_setup("rachelv_r05m", "", &pf_job_id);
	
	printf("Completed pf_setup\n");

	pf_specify_markers(pf_job_id, number_of_markers, marker_names,
		marker_2d_positions, marker_confidences);
	printf("Completed pf_specify_markers.\n");

	pf_view_align(pf_job_id, &error);
	printf("Completed pf_view_align.\n");

	if (0 == pf_get_view(pf_job_id, eye_point, interest_point, up_vector, &view_angle))
	{
		printf("Got view parameters.\n");
		printf("   Eye point %f %f %f.\n", eye_point[0], eye_point[1], eye_point[2]);
		printf("   Interest point %f %f %f.\n", interest_point[0], interest_point[1],
			interest_point[2]);
		printf("   Up vector %f %f %f.\n", up_vector[0], up_vector[1], up_vector[2]);
		printf("   View angle %f.\n", view_angle);
	}

	pf_fit(pf_job_id, &error);
	printf("Completed pf_fit.\n");

	if (0 == pf_get_head_model(pf_job_id, &(obj.number_of_vertices), &(obj.vertex_3d_locations),
		&(obj.number_of_texture_vertices), &(obj.texture_vertex_3d_locations),
		&(obj.number_of_triangles), &(obj.triangle_vertices),
	  &(obj.triangle_texture_vertices)))
	{
		printf("Completed pf_get_head_model. vertices %d, %d, triangles %d\n",
			obj.number_of_vertices, obj.number_of_texture_vertices, obj.number_of_triangles);
	}

	/* Put something into the image array */
	image_ptr = image_array;
	for (i = 0 ; i < IMAGE_HEIGHT ; i++)
	{
		for (j = 0 ; j < IMAGE_WIDTH ; j++)
		{
			*image_ptr = 255.0 * (float)j / (float)IMAGE_WIDTH;
			image_ptr++;
			*image_ptr = 255.0 * (float)i /  (float)IMAGE_HEIGHT;
			image_ptr++;
			*image_ptr = 128.0;
			image_ptr++;
			printf("%ud %ud %ud\n", *(image_ptr - 3), *(image_ptr - 2), 
				*(image_ptr - 1)); 
		}
	}
	
	if (0 == pf_get_marker_fitted_positions(pf_job_id, number_of_markers, 
		marker_names, marker_fitted_3d_positions))
	{
		printf("Fitted positions:\n");
		for (i = 0 ; i < number_of_markers ; i++)
		{
			printf("   %s:  %f %f %f\n", marker_names[i],
				marker_fitted_3d_positions[3 * i], marker_fitted_3d_positions[3 * i + 1], 
				marker_fitted_3d_positions[3 * i + 2]);
		}
	}

	if (0 == pf_get_marker_fitted_positions(pf_job_id, 2, 
		eye_marker_names, eyeball_fitted_3d_positions))
	{
		printf("Eyeball Fitted positions:\n");
		for (i = 0 ; i < 2 ; i++)
		{
			printf("   %s:  %f %f %f\n", eye_marker_names[i],
				eyeball_fitted_3d_positions[3 * i], eyeball_fitted_3d_positions[3 * i + 1], 
				eyeball_fitted_3d_positions[3 * i + 2]);
		}
	}

	if (0 == pf_get_basis(pf_job_id, &number_of_modes, &number_of_vertices,
		&vertex_3d_locations))
	{
		printf("Basis:  Modes %d, vertices %d\n", number_of_modes,
			number_of_vertices);
		printf("Basis[10][10]:  %f\n", vertex_3d_locations[10 * 
			3 * number_of_vertices + 10]);
	}

	pf_specify_image(pf_job_id, IMAGE_WIDTH, IMAGE_HEIGHT, PF_RGB_IMAGE, image_array);

	if (0 == pf_get_texture(pf_job_id, TEXTURE_WIDTH, TEXTURE_HEIGHT, texture_array))
	{
		printf ("Texture[30][30]: %d %d %d\n", texture_array[3 * 30 * 100 + 3 * 30],
			texture_array[3 * 30 * 100 + 3 * 30 + 1], texture_array[3 * 30 * 100 + 3 * 30 + 2]);
		printf ("Texture[70][70]: %d %d %d\n", texture_array[3 * 70 * 100 + 3 * 70],
			texture_array[3 * 70 * 100 + 3 * 70 + 1], texture_array[3 * 70 * 100 + 3 * 70 + 2]);
	}

	/* Free up the path memory */
	pf_specify_paths(NULL, NULL);


	/* Write out the files */
	pf_write_head_model("test_photoface.obj", obj.number_of_vertices, obj.vertex_3d_locations,
		obj.number_of_texture_vertices, obj.texture_vertex_3d_locations,
		obj.number_of_triangles, obj.triangle_vertices,
		obj.triangle_texture_vertices);
	
	pf_write_basis("test_photoface.basis" ,number_of_modes,
		number_of_vertices, vertex_3d_locations);

	pf_write_texture("test_photoface.jpg", TEXTURE_WIDTH, TEXTURE_HEIGHT, texture_array);

	pf_write_scene_graph("test_photoface_scenegraph.txt",
		eye_point, interest_point, up_vector, view_angle,
		eyeball_fitted_3d_positions, eyeball_fitted_3d_positions + 3);
}
