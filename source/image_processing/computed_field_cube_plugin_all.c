/************************************************************************************
   FILE: computed_field_cube_plugin_all.c

   LAST MODIFIED: 25 June 2004

   DESCRIPTION: Perform morphological parameters extraction on Computed field.
===================================================================================*/
#include <math.h>
#include <time.h>
#include "general/random.h"
#include <stdlib.h>
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_find_xi.h"
#include "computed_field/computed_field_private.h"
#include "computed_field/computed_field_set.h"
#include "image_processing/image_cache.h"
#include "general/image_utilities.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "user_interface/message.h"
#include "image_processing/computed_field_cube_plugin_all.h"
#include "image_processing/computed_field_median_filter.h"
#include "image_processing/morphology_functions.h"

#define my_Min(x,y) ((x) <= (y) ? (x) : (y))
#define my_Max(x,y) ((x) <= (y) ? (y) : (x))
#define DISTANCE sqrt((x - xctr) * (x - xctr) + (y - yctr) * (y - yctr) + (z - zctr) * (z - zctr))
#define DISTANCE1 sqrt((x - xctr) * (x - xctr) + (y - yctr) * (y - yctr))

struct Computed_field_cube_plugin_all_package
/*******************************************************************************
LAST MODIFIED : 17 February 2004

DESCRIPTION :
A container for objects required to define fields in this module.
==============================================================================*/
{
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Cmiss_region *root_region;
	struct Graphics_buffer_package *graphics_buffer_package;
};

struct Computed_field_cube_plugin_all_type_specific_data
{
	int number_of_dirs; /* the number of directions for checking*/
	int radius; /* the radius of region of interest */
	double pixel_size; /* resolution */
	int object_dimension; /* 3D or 2D parameters */
	int dimension;
	int *input_sizes;
	int *output_sizes;
	char *output_file_name;
	float cached_time;
	int element_dimension;
	struct Cmiss_region *region;
	struct Graphics_buffer_package *graphics_buffer_package;
	struct Image_cache *image;
	struct MANAGER(Computed_field) *computed_field_manager;
	void *computed_field_manager_callback_id;
};

static char computed_field_cube_plugin_all_type_string[] = "cube_plugin_all";

int Computed_field_is_type_cube_plugin_all(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 10 April 2004

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_cube_plugin_all);
	if (field)
	{
		return_code =
		  (field->type_string == computed_field_cube_plugin_all_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_cube_plugin_all.  Missing field");
		return_code = 0;
	}

	return (return_code);
} /* Computed_field_is_type_cube_plugin_all */

static void Computed_field_cube_plugin_all_field_change(
	struct MANAGER_MESSAGE(Computed_field) *message, void *field_void)
/*******************************************************************************
LAST MODIFIED : 5 April 2004

DESCRIPTION :
Manager function called back when either of the source fields change so that
we know to invalidate the image cache.
==============================================================================*/
{
	struct Computed_field *field;
	struct Computed_field_cube_plugin_all_type_specific_data *data;

	ENTER(Computed_field_cube_plugin_all_source_field_change);
	if (message && (field = (struct Computed_field *)field_void) && (data =
		(struct Computed_field_cube_plugin_all_type_specific_data *)
		field->type_specific_data))
	{
		switch (message->change)
		{
			case MANAGER_CHANGE_OBJECT_NOT_IDENTIFIER(Computed_field):
			case MANAGER_CHANGE_OBJECT(Computed_field):
			{
				if (Computed_field_depends_on_Computed_field_in_list(
					field->source_fields[0], message->changed_object_list) ||
					Computed_field_depends_on_Computed_field_in_list(
					field->source_fields[1], message->changed_object_list))
				{
					if (data->image)
					{
						data->image->valid = 0;
					}
				}
			} break;
			case MANAGER_CHANGE_ADD(Computed_field):
			case MANAGER_CHANGE_REMOVE(Computed_field):
			case MANAGER_CHANGE_IDENTIFIER(Computed_field):
			{
				/* do nothing */
			} break;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_cube_plugin_all_source_field_change.  "
			"Invalid arguments.");
	}
	LEAVE;
} /* Computed_field_cube_plugin_all_source_field_change */

static int Computed_field_cube_plugin_all_clear_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 10 April 2004

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	int return_code;
	struct Computed_field_cube_plugin_all_type_specific_data *data;

	ENTER(Computed_field_cube_plugin_all_clear_type_specific);
	if (field && (data =
		(struct Computed_field_cube_plugin_all_type_specific_data *)
		field->type_specific_data))
	{
		if (data->region)
		{
			DEACCESS(Cmiss_region)(&data->region);
		}
		if (data->image)
		{
			DEACCESS(Image_cache)(&data->image);
		}
		if (data->computed_field_manager && data->computed_field_manager_callback_id)
		{
			MANAGER_DEREGISTER(Computed_field)(
				data->computed_field_manager_callback_id,
				data->computed_field_manager);
		}

		if (data->input_sizes)
		{
			DEALLOCATE(data->input_sizes);
		}
		if (data->output_sizes)
		{
			DEALLOCATE(data->output_sizes);
		}
		DEALLOCATE(field->type_specific_data);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_cube_plugin_all_clear_type_specific.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_cube_plugin_all_clear_type_specific */

static void *Computed_field_cube_plugin_all_copy_type_specific(
	struct Computed_field *source_field, struct Computed_field *destination_field)
/*******************************************************************************
LAST MODIFIED : 10 April 2004

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	struct Computed_field_cube_plugin_all_type_specific_data *destination,
		*source;
	int i;

	ENTER(Computed_field_cube_plugin_all_copy_type_specific);
	if (source_field && destination_field && (source =
		(struct Computed_field_cube_plugin_all_type_specific_data *)
		source_field->type_specific_data))
	{
		if (ALLOCATE(destination,
			struct Computed_field_cube_plugin_all_type_specific_data, 1))
		{
			destination->number_of_dirs = source->number_of_dirs;
			destination->radius = source->radius;
			destination->pixel_size = source->pixel_size;
			destination->object_dimension = source->object_dimension;
			destination->dimension = source->dimension;
			destination->output_file_name = source->output_file_name;
			for (i = 0; i < source->dimension; i++)
			{
				destination->input_sizes[i] = source->input_sizes[i];
				destination->output_sizes[i] = source->output_sizes[i];
			}
			destination->cached_time = source->cached_time;
			destination->region = ACCESS(Cmiss_region)(source->region);
			destination->element_dimension = source->element_dimension;
			destination->graphics_buffer_package = source->graphics_buffer_package;
			destination->computed_field_manager = source->computed_field_manager;
			destination->computed_field_manager_callback_id =
				MANAGER_REGISTER(Computed_field)(
				Computed_field_cube_plugin_all_field_change, (void *)destination_field,
				destination->computed_field_manager);
			if (source->image)
			{
				destination->image = ACCESS(Image_cache)(CREATE(Image_cache)());
				Image_cache_update_dimension(destination->image,
					source->image->dimension, source->image->depth,
					source->input_sizes, source->image->minimums,
					source->image->maximums);
			}
			else
			{
				destination->image = (struct Image_cache *)NULL;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_cube_plugin_all_copy_type_specific.  "
				"Unable to allocate memory.");
			destination = NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_cube_plugin_all_copy_type_specific.  "
			"Invalid arguments.");
		destination = NULL;
	}
	LEAVE;

	return (destination);
} /* Computed_field_cube_plugin_all_copy_type_specific */

int Computed_field_cube_plugin_all_clear_cache_type_specific
   (struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 10 April 2004

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Computed_field_cube_plugin_all_type_specific_data *data;

	ENTER(Computed_field_cube_plugin_all_clear_type_specific);
	if (field && (data =
		(struct Computed_field_cube_plugin_all_type_specific_data *)
		field->type_specific_data))
	{
		if (data->image)
		{
			/* data->image->valid = 0; */
		}
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_cube_plugin_all_clear_type_specific.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_cube_plugin_all_clear_type_specific */

static int Computed_field_cube_plugin_all_type_specific_contents_match(
	struct Computed_field *field, struct Computed_field *other_computed_field)
/*******************************************************************************
LAST MODIFIED : 10 April 2004

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int return_code;
	struct Computed_field_cube_plugin_all_type_specific_data *data,
		*other_data;

	ENTER(Computed_field_cube_plugin_all_type_specific_contents_match);
	if (field && other_computed_field && (data =
		(struct Computed_field_cube_plugin_all_type_specific_data *)
		field->type_specific_data) && (other_data =
		(struct Computed_field_cube_plugin_all_type_specific_data *)
		other_computed_field->type_specific_data))
	{
		if ((data->number_of_dirs == other_data->number_of_dirs) &&
		        (data->radius == other_data->radius)&&
			(data->pixel_size == other_data->pixel_size)&&
			(data->object_dimension == other_data->object_dimension)&&
			data->image && other_data->image &&
			(data->image->dimension == other_data->image->dimension) &&
			(data->image->depth == other_data->image->depth))
		{
			/* Check sizes and minimums and maximums */
			return_code = 1;
		}
		else
		{
			return_code = 0;
		}
	}
	else
	{
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_cube_plugin_all_type_specific_contents_match */

#define Computed_field_cube_plugin_all_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 10 April 2004

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_cube_plugin_all_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 10 April 2004

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_cube_plugin_all_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 10 April 2004

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_cube_plugin_all_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 10 April 2004

DESCRIPTION :
No special criteria.
==============================================================================*/


static int Image_cache_structure_indices(struct Image_cache *image, int number_of_dirs,
         int radius, double pixel_size,
	 int object_dimension, int dimension, int *output_sizes, char *output_file_name)
/*******************************************************************************
LAST MODIFIED : 24 April 2004

DESCRIPTION :
Perform morphologic parameters extraction on the image cache.
==============================================================================*/
{
	char *storage;

	char filename[80];
	FILE *fp;

	FE_value *data_index, *result_index;
	int  return_code,  storage_size, out_storage_size;

	FE_value *data_index1;

        /*  Variables required to generate random numbers  */
	#if defined(UNIX)
        static long state1[32] = {
        	3,
        	0x9a319039, 0x32d9c024, 0x9b663182, 0x5da1f342,
        	0x7449e56b, 0xbeb1dbb0, 0xab5c5918, 0x946554fd,
        	0x8c2e680f, 0xeb3d799f, 0xb11ee0b7, 0x2d436b86,
        	0xda672e2a, 0x1588ca88, 0xe369735d, 0x904f35f7,
        	0xd7158fd6, 0x6fa6f051, 0x616e6b96, 0xac94efdc,
        	0xde3b81e0, 0xdf0a6fb5, 0xf103bc02, 0x48f340fb,
        	0x36413f93, 0xc622c298, 0xf5a42ab8, 0x8a88d77b,
        	0xf5ad9d0e, 0x8999220b, 0x27fb47b9
        	};
	int	n;				/* for random number gen.	*/
	#endif

        int	xsize, ysize, zsize;	        /* size of data_index1			*/

        int 	xctr, yctr, zctr;	        /* center of test sphere */

	FE_value 	pixsize;		/* size of one side of voxel		*/
        FE_value	tline_ln;		/* length of all test lines in mm	*/
        int	num_tlines;		/* number of test lines in grid		*/
        int	tintrsctn;		/* total # of intersections		*/
        FE_value 	*rot1;		/* random theta rotations		*/
        FE_value	*rot2;		/* random phi rotations			*/

        FE_value 	bv, bvtv;		/* solid volume in mm^3 and BV/TV	*/
        FE_value 	pl;			/* #intrsctns/unit test line length	*/
        FE_value	*mil;		/* magnitude of MIL for ith rotation	*/
        FE_value	bs, sv, tb, tpd, tps;	/* BS/TV, Tb.Th, Tb.N and Tb.Sp		*/

        FE_value euler_number;
	FE_value mean_curvature;
	FE_value SMI; /* structure model index */

        unsigned char seed;			/* seed for random number gen.	*/

        FE_value	dradius;		/* test sphere radius (FE_value)	*/
        int	lmarg, rmarg;			/* x limits of analysis region	*/
        int	tlay, blay;			/* y limits of analysis region	*/
        int 	top, bottom;			/* z limits of analysis region	*/
        FE_value 	tv;				/* total number of voxels	*/
	FE_value bonev, totalv;

        FE_value	phi;			/* orientation angles		*/

        FE_value	sin_phi, cos_phi;		/* more trig calculations	*/

        FE_value 	r[3], norm_r[3];		/* test line direction vectors	*/
        FE_value 	x, y;			/* position on test line	*/
        FE_value 	x_off, y_off;		/* offset to get to start pos.	*/
        FE_value 	dx, dy;			/* grid position (FE_value)	*/
        int	ix, iy, iz;			/* integer grid position	*/
        int	iline_sp;			/* spacing of test lines (int)	*/
        int 	intrsctn;			/* number of intersections	*/
        int 	cur_pt;				/* current location in data_index1	*/
        int  preval, curval;			/* previous voxel value		*/

        int	irot;				/* rotation loop counter	*/
        int	i, k;				/* loop counter			*/

        FE_value	*elvector;		/* vector of ellipsoid coeff.(6) and goodness(4)	*/

	FE_value  *eigvals, *eigvecs;
	FE_value  prin1,prin2,prin3;	/* principal MIL vectors		*/

        FE_value  deg1,deg2,deg3;		/* degrees of anisotropy		*/
        FE_value T;
	FE_value A1, A2;
	FE_value P1, P2, tbpf;
	int M1, N1;
	FE_value *Im1, *Im2, *Im3, *Im4;
	FE_value sd, vf;
	int *h;
	int levels; /* number of grey levels */

	ENTER(Image_cache_structure_indices);
	if (image && (dimension == image->dimension) && (image->depth > 0))
	{
	        return_code = 1;
		A1 = A2 = 0.0;
		P1 = P2 = tbpf = 0.0;
		bs = 0.0;
		pl = 0.0;
		levels = 256;
		for (i = 0; i < 80; i++)
		{
		        filename[i] = *output_file_name;
			output_file_name++;
		}

                storage_size = image->depth;
		out_storage_size = 12;
		for (i = 0 ; i < dimension ; i++)
		{
			storage_size *= image->sizes[i];
			out_storage_size *= output_sizes[i];
		}
		if (ALLOCATE(data_index1, FE_value, (storage_size/image->depth))&&
                                ALLOCATE(rot1, FE_value, number_of_dirs)&&
		                ALLOCATE(rot2, FE_value, number_of_dirs)&&
		                ALLOCATE(mil, FE_value, number_of_dirs) &&
		                ALLOCATE(elvector, FE_value, 10)&&
				ALLOCATE(eigvals, FE_value, 9)&&
				ALLOCATE(eigvecs, FE_value, 9) &&
				ALLOCATE(h, int, 256) &&
				ALLOCATE(storage,char, out_storage_size * sizeof(FE_value)))
		{
                        return_code = 1;

			data_index = (FE_value *)image->data;
			/* Threshold for re_segmentint image */

			/* Get the single component data for gray level image */
			T = 0.68;
			for (i = 0; i < storage_size/image->depth; i++)
		        {
			        if (*data_index < T)
				{
                                         data_index1[i] = 1.0;
				}
				else
				{
		        	         data_index1[i] = 0.0;
				}
				data_index += image->depth;
		        }
			pixsize = (FE_value)pixel_size; /* (20/1024) (mm)*/
			/* maxrand = 2147483647, the max value generated by the random*/
				/* number routine is (2**31 - 1) 	*/

                        /*   Generate the random rotations */
			#if defined(UNIX)
                	seed = 1;
                	n = 256;
                	initstate(seed, (char *) state1, n);
                	setstate((char *) state1);

	                for(irot=0;irot < number_of_dirs;irot++)
			{
        	        	rot1[irot] =  CMGUI_RANDOM(double) * M_PI;
        	        	rot2[irot] =  CMGUI_RANDOM(double) * M_PI;
        	        }
			#elif defined (WIN32_SYSTEM)
			seed = 1;
			CMGUI_SEED_RANDOM(seed);
			for(irot=0;irot < number_of_dirs;irot++)
			{
        	        	rot1[irot] =  CMGUI_RANDOM(double) * M_PI;
        	        	rot2[irot] =  CMGUI_RANDOM(double) * M_PI;
        	        }
			#endif
                       /*  Calculate the test line spacing in pixels in x-y plane using a 120.722	*/
                       /*	micron spacing							*/

                	iline_sp = (int)(0.120722 / pixsize);
			if (iline_sp == 0)
			{
			        iline_sp = 1;
			}
		        dradius = (FE_value)radius;
		        xsize = image->sizes[0];
		        ysize = image->sizes[1];
		        zsize = image->sizes[2];
			xctr = image->sizes[0] / 2;
			yctr = image->sizes[1] / 2;
			zctr = image->sizes[2] / 2;

			lmarg = xctr - radius;
			if (lmarg < 0) lmarg = 0;
			rmarg = xctr + radius;
			if (rmarg > (xsize - 1)) rmarg = xsize - 1;
			tlay = yctr - radius;
			if (tlay < 0) tlay = 0;
			blay = yctr + radius;
			if (blay > (ysize - 1)) blay = ysize - 1;
			top = zctr - radius;

			M1 = rmarg - lmarg + 1;
			N1 = blay - tlay + 1;

			result_index = (FE_value *)storage;
			for (i = 0; i < out_storage_size / 12; i++)
			{
				result_index[i] = 0.0;
				result_index++;
			}
			result_index = (FE_value *)storage;
			fp = fopen(filename,"wa");
			fprintf(fp, "Bone Image Analysis Results (3D)\n");
			fprintf(fp,"\n");
			fprintf(fp,"-----------------------------------------------\n");
			fprintf(fp, "Bone Parameters:\n");
			fprintf(fp,"\n");
			for (iz = 0; iz < output_sizes[2]; iz++)
			{
			        for(iy = 0; iy < output_sizes[1]; iy++)
				{
				        for(ix = 0; ix < output_sizes[0]; ix++)
					{
					        xctr = (int)((((FE_value)ix + 0.5) * ((FE_value)image->sizes[0] - 1.0) / ((FE_value)output_sizes[0])));
						yctr = (int)((((FE_value)iy + 0.5) * ((FE_value)image->sizes[1] - 1.0) / ((FE_value)output_sizes[1])));
						zctr = (int)((((FE_value)iz + 0.5) * ((FE_value)image->sizes[2] - 1.0) / ((FE_value)output_sizes[2])));
						lmarg = xctr - radius;
						if (lmarg < 0) lmarg = 0;
						rmarg = xctr + radius;
						if (rmarg > (xsize - 1)) rmarg = xsize - 1;
						tlay = yctr - radius;
						if (tlay < 0) tlay = 0;
						blay = yctr + radius;
						if (blay > (ysize - 1)) blay = ysize - 1;
						top = zctr - radius;
						if (top < 0) top = 0;
						bottom = zctr + radius;
						if (bottom > (zsize - 1)) bottom = zsize - 1;

						/*Spatial_gray_tone_histogram_cube(data_index1,
                                                         xsize, ysize, zsize, levels, h);*/
						Spatial_gray_tone_histogram(data_index1, radius, xctr, yctr, zctr,
                                                         xsize, ysize, zsize, levels, h);
						Spatial_Euler_number(pixsize, h, &euler_number);
						Spatial_mean_curvature(pixsize,h, &mean_curvature);
						Spatial_total_volume(h, &tv);
						Spatial_volume_fraction(h, &vf);
						Spatial_surface_density(pixsize, h, &sd);

						tv *= pixsize * pixsize * pixsize;

						bv = vf * tv;  /* Scale voxels -> mm^3 */
						bs = sd * tv;
						sv = sd / vf;	/* BS/BV, [mm^2 mm^-3] */
						tb = (5.0 /3.0)*vf / sd;	/* Tb.Th, [mm] */
						tpd = vf / tb;		/* Tb.N, [mm^-1] */
						tps = (1.0/tpd) - tb;	/* Tb.Sp, [mm] */
						SMI = 2.025 * 12.0 * mean_curvature *vf / (sd * sd) ;
						vf *= 100.0;
						fprintf(fp,"Position (x, y, z): %d, %d, %d\n",xctr,yctr,zctr);
						fprintf(fp,"BV(mm^3)         : %f\n",bv);
						fprintf(fp,"TV(mm^3)         : %f\n",tv);
						fprintf(fp,"BS(mm^2)         : %f\n",bs);
						fprintf(fp,"BV/TV(percentage): %f\n",vf);
						fprintf(fp,"BS/BV(mm^2 mm^-3): %f\n",sv);
						fprintf(fp,"Tb.Th(mm)        : %f\n",tb);
						fprintf(fp,"Tb.N(mm^-1)      : %f\n",tpd);
						fprintf(fp,"Tb.Sp(mm)        : %f\n",tps);
						fprintf(fp,"Pixel size(mm)   : %f X %f X %f\n",pixsize, pixsize, pixsize);
						fprintf(fp,"\n");
						fprintf(fp,"Euler number: %f\n", euler_number);
						fprintf(fp,"SMI: %1f\n", SMI);
						tintrsctn = 0;
						num_tlines = 0;
						Volume_fraction_and_length_of_test_lines(data_index1, dradius,
                                                     xctr, yctr, zctr, iline_sp,
	                                                xsize, ysize,
	                                             lmarg, rmarg, tlay, blay, top, bottom,
	                                             &bonev, &totalv, &tline_ln);
        	        			tline_ln *= pixsize;  /* Convert from pixels -> mm */
						bvtv = bonev/totalv;
						MIL_vector(data_index1, number_of_dirs, dradius,
                                                     rot1, rot2, tline_ln, iline_sp, bvtv,
						     xsize, ysize, xctr, yctr, zctr,
                                                     lmarg, rmarg, tlay, blay, top, bottom,
                                                     mil, &tintrsctn, &num_tlines);

						Mil_ellipsoid_tensor(mil, rot1, rot2, number_of_dirs,elvector);

						Principal_orientations(elvector,
	                                             &prin1, &prin2, &prin3,
	                                             &deg1, &deg2, &deg3, eigvals, eigvecs);
						Sort_eigenvalue(eigvals, eigvecs,
	                                              &prin1, &prin2, &prin3,
	                                              &deg1, &deg2, &deg3);
						fprintf(fp,"Anisotropy Tensor:\n");
						fprintf(fp,"ae: %.8f\n", elvector[0]);
						fprintf(fp,"be: %.8f\n", elvector[1]);
						fprintf(fp,"ce: %.8f\n", elvector[2]);
						fprintf(fp,"de: %.8f\n", elvector[3]);
						fprintf(fp,"ee: %.8f\n", elvector[4]);
						fprintf(fp,"fe: %.8f\n", elvector[5]);
						fprintf(fp,"\n");
						fprintf(fp, "principle eig values: %1f, %1f, %1f\n", prin1,prin2,prin3);
						fprintf(fp,"\n");
						fprintf(fp,"Orientation 1 (x,y,z): %1f, %1f, %1f\n", eigvecs[0],eigvecs[3],eigvecs[6]);
						fprintf(fp,"Orientation 2 (x,y,z): %1f, %1f, %1f\n", eigvecs[1],eigvecs[4],eigvecs[7]);
						fprintf(fp,"Orientation 3 (x,y,z): %1f, %1f, %1f\n", eigvecs[2],eigvecs[5],eigvecs[8]);
						fprintf(fp,"degrees of nanisotropy: %lf, %1f, %1f\n", deg1,deg2,deg3);
						fprintf(fp,"=======================================\n");
						result_index[0] = eigvecs[0];
						result_index[1] = eigvecs[3];
						result_index[2] = eigvecs[6];
						result_index[3] = eigvecs[1];
						result_index[4] = eigvecs[4];
						result_index[5] = eigvecs[7];
						result_index[6] = eigvecs[2];
						result_index[7] = eigvecs[5];
						result_index[8] = eigvecs[8];
						result_index[9] = prin1;
						result_index[10] = prin2;
						result_index[11] = prin3;
						result_index += 12;
					}
				}
			}
			DEALLOCATE(elvector);
        		DEALLOCATE(mil);
			DEALLOCATE(eigvals);
			DEALLOCATE(eigvecs);
			DEALLOCATE(h);

			ALLOCATE(Im1, FE_value, M1*N1);
			ALLOCATE(Im2, FE_value, M1*N1);
			ALLOCATE(Im3, FE_value, M1*N1);
			ALLOCATE(Im4, FE_value, M1*N1);

			if (object_dimension == 3)
			{
			        fclose(fp);
				DEALLOCATE(data_index1);
				DEALLOCATE(Im1);
				DEALLOCATE(Im2);
				DEALLOCATE(Im3);
				DEALLOCATE(Im4);
			}
			else if (object_dimension == 2)
			{
				fprintf(fp, "Plugin: Bone parameters per cross section (2D)\n");
				fprintf(fp,"\n");

                       /*  Calculate the total test line length (same for all rotations).  	*/
                       /*	For any (x,y) point on the test grid, the test line length is 	*/
                       /*	twice the perpendicular distance from the x-y plane to the edge */
                       /*	of the test sphere.		  				*/

        	                tline_ln = 0.0;
				for(ix = lmarg; ix <= rmarg; ix += iline_sp)
				{
                	                dx = (FE_value)ix - (FE_value)xctr;
					if (dx < 0.0) dx = -dx;
					if (dx <= dradius)
					{
						dy = sqrt(dradius * dradius - dx*dx);
						tline_ln += 2.0 * dy;
					}
				}
				tline_ln *= pixsize;  /* Convert from pixels -> mm */
				for (iz = 0; iz < output_sizes[2]; iz++) /* slice_by_slice */
				{

					zctr = (int)((((FE_value)iz + 0.5) / ((FE_value)output_sizes[2])) * ((FE_value)image->sizes[2] - 1.0));
					fprintf(fp,"Slice %d\n",zctr + 1);
					fprintf(fp,"Results: BV, TV, BV/TV, BS/BV, Tb.Th, Tb.N, Tb.Sp, TBPf\n");
					tintrsctn = 0;
				        bv = tv = 0.00;
					i = 0;
					for(iy = tlay; iy <= blay; iy++)
					{
					        for (ix = lmarg; ix <= rmarg; ix++)
						{
							cur_pt = (zctr * ysize + iy) * xsize + ix;
							Im1[i] = data_index1[cur_pt];
							i++;
						}
					}

					for(iy = 0; iy < ysize; iy++)
					{
					        for (ix = 0; ix < xsize; ix++)
						{
							cur_pt = (zctr * ysize + iy) * xsize + ix;

        		        			tv++;
							if (data_index1[cur_pt] == 1.0)
							{
								bv++;
							}
							else
							{
								data_index1[cur_pt] = 0.0;
							}
						}
					}
					bvtv = bv / tv;	 /* Volume fraction, BV/TV */
					bv *= pixsize * pixsize;  /* Scale voxels -> mm^2 */
					tv *= pixsize * pixsize;

                        /*   Scan the image for each of the randomly selected rotations  */
			                for( irot = 0; irot < number_of_dirs; irot++)
					{
					        phi = rot2[irot];
						sin_phi = sin(phi);
						cos_phi = cos(phi);

                       /*   Define direction vector for this rotation 	*/
		                                r[0] = cos_phi;
						r[1] = sin_phi;

                      /*   Normalize direction vector to +1 for the maximum component */

                        	                if (fabs(r[0]) >= fabs(r[1]))
						{
						        norm_r[1] = r[1] / r[0];
							norm_r[0] = 1.0;
						}
						else
						{
						        norm_r[0] = r[0] / r[1];
							norm_r[1] = 1.0;
						}

                                /*   Make sure signs are correct */
				                for (i=0; i<2; i++)
						{
						        if (r[i] > 0)
							{
							        norm_r[i] = fabs(norm_r[i]);
							}
							else
							{
							        norm_r[i] = (-fabs(norm_r[i]));
							}
						}
						intrsctn = 0;

                        /*  Loop over the test grid which is equally spaced in x and y */
			                        for(ix = lmarg; ix < rmarg; ix += iline_sp)
						{
							dx = (FE_value)ix - (FE_value)xctr;
							if (dx < 0.0)
							{
							        dx = -dx;
							}
							if (dx <= dradius)
							{
							        dy = sqrt(dradius * dradius - dx * dx);
								x_off = dy * r[0];
								y_off = dy * r[1];
								x = dx*cos_phi - dy*sin_phi + (FE_value)xctr;
								y = dx*sin_phi + dy*cos_phi + (FE_value)yctr;
								x -= x_off;
								y -= y_off;
								cur_pt = ((int)(zctr)*ysize + (int)(y))*xsize + (int)(x);
								if ((int)(y) >= tlay && (int)(y) <= blay)
								{
									preval = *(data_index1 + cur_pt);
								}
								else
								{
								        preval = 0.0;
								}
								x += norm_r[0];
								y += norm_r[1];
								while (DISTANCE1 <= dradius)
								{
								        cur_pt = ((int)(zctr) * ysize +
        			                			                      (int)(y)) * xsize + (int)(x);
									if ((int)(y) >= tlay && (int)(y) <= blay)
									{
									        curval = *(data_index1 + cur_pt);
									}
									else
									{
									        curval = 0.0;
									}
									if(curval != preval)
									{
	                			        		        intrsctn += 1;
										preval = curval;
									}
									x += norm_r[0];
									y += norm_r[1];
								}
								num_tlines++;
							} /* end loop over y axis */
						} /* end loop over x axis */
						tintrsctn += intrsctn;
					}   /* end loop over number_of_dirs rotations */

                       /* Calculate morphology parameters: BS/BV, Tb.Th, Tb.N, Tb.Sp, using 	*/
                      /*	parallel plate model (Parfitt, et al. JCI 72:1396-1409, 1983)	*/
		                        pl = (FE_value)tintrsctn / (tline_ln * number_of_dirs);
					sv = 2 * pl / bvtv;	/* BS/BV, [mm^2 mm^-3] */
					tb = bvtv / pl;	/* Tb.Th, [mm] */
					tpd = pl;		/* Tb.N, [mm^-1] */
					tps = (1.0-bvtv)/pl;	/* Tb.Sp, [mm] */
					A1 = 0.0;
					A2 = 0.0;
					P1 = 0.0;
					P2 = 0.0;

					for (k = 0; k < N1; k++)
					{
					        for(i = 0; i < M1; i++)
						{
						        if ((k-1) >= 0 && (i-1) >= 0 && (k+1) < N1 && (i+1)< M1)
							{
							        Im2[k*M1 + i] = Im1[k*M1 + i] * Im1[(k-1)*M1 + i] *
							            Im1[(k+1)*M1 + i] * Im1[k*M1 + i-1] * Im1[k*M1 + i+1];
								if (Im1[k*M1 + i] == 1.0)
								{
								        A1 += 1.0;
								}
							}
							else
							{
							        Im2[k*M1 + i] = 0.0;
							}
							Im3[k*M1 + i] = Im1[k*M1 + i] - Im2[k*M1 + i];
						}
					}
					for (k = 0; k < N1; k++)
					{
					        for(i = 0; i < M1; i++)
						{
						        if ((k-1) >= 0 && (i-1) >= 0 && (k+1) < N1 && (i+1)< M1)
							{
							        Im2[k*M1 + i] = Im1[(k-1)*M1 + i] +
							            Im1[(k+1)*M1 + i] + Im1[k*M1 + i-1] + Im1[k*M1 + i+1];
								if (Im2[k*M1 + i] >= 1.0)
								{
								        A2 += 1.0;
									Im2[k*M1+i] = 1.0;
								}
							}
							else
							{
							        Im2[k*M1 + i] = 0.0;
							}
						}
					}
					for (k = 0; k < N1; k++)
					{
					        for(i = 0; i < M1; i++)
						{
						        if ((k-1) >= 0 && (i-1) >= 0 && (k+1) < N1 && (i+1)< M1)
							{
							        Im1[k*M1 + i] = Im2[k*M1 + i] * Im2[(k-1)*M1 + i] *
							            Im2[(k+1)*M1 + i] * Im2[k*M1 + i-1] * Im2[k*M1 + i+1];

							}
							else
							{
							        Im1[k*M1 + i] = 0.0;
							}
							Im4[k*M1 + i] = Im2[k*M1 + i] - Im1[k*M1 + i];
						}
					}
					for (k = 0; k < N1; k++)
					{
					        for(i = 0; i < M1; i++)
						{
						        if ((k-1) >= 0 && (i-1) >= 0 && (k+1) < N1 && (i+1)< M1)
							{
							        Im1[k*M1 + i] = 10.0*Im3[(k-1)*M1+i-1]+ 2.0*Im3[(k-1)*M1+i] + 10.0*Im3[(k-1)*M1 + i+1] +
							            2.0 *Im3[k*M1 + i -1] + Im3[k*M1 + i] + 2.0*Im3[k*M1 + i +1] +
								    10.0*Im3[(k+1)*M1+i-1]+ 2.0*Im3[(k+1)*M1+i] + 10.0*Im3[(k+1)*M1 + i+1];
								Im2[k*M1 + i] = 10.0*Im4[(k-1)*M1+i-1]+ 2.0*Im4[(k-1)*M1+i] + 10.0*Im4[(k-1)*M1 + i+1] +
							            2.0 *Im4[k*M1 + i -1] + Im4[k*M1 + i] + 2.0*Im4[k*M1 + i +1] +
								    10.0*Im4[(k+1)*M1+i-1]+ 2.0*Im4[(k+1)*M1+i] + 10.0*Im4[(k+1)*M1 + i+1];
							}
							else
							{
							        Im1[k*M1 + i] = Im2[k*M1 + i] = 0.0;
							}
							if ((Im1[k*M1 + i] == 5.0) ||(Im1[k*M1 + i] == 15.0)||(Im1[k*M1 + i] == 7.0)
							          ||(Im1[k*M1 + i] == 25.0)||(Im1[k*M1 + i] == 27.0) ||(Im1[k*M1 + i] == 17.0))
							{
							        P1 += 1.0;
							}
							else if ((Im1[k*M1 + i] == 13.0) || (Im1[k*M1 + i] == 23.0))
							{
							        P1 += 1.207;
							}
							else if ((Im1[k*M1 + i] == 21.0) ||(Im1[k*M1 + i] == 33.0))
							{
							        P1 += 1.414;
							}
							if ((Im2[k*M1 + i] == 5.0) ||(Im2[k*M1 + i] == 15.0)||(Im2[k*M1 + i] == 7.0)
							          ||(Im2[k*M1 + i] == 25.0)||(Im2[k*M1 + i] == 27.0) ||(Im2[k*M1 + i] == 17.0))
							{
							        P2 += 1.0;
							}
							else if ((Im2[k*M1 + i] == 13.0) ||(Im2[k*M1 + i] == 23.0))
							{
							        P2 += 1.207;
							}
							else if ((Im2[k*M1 + i] == 21.0) ||(Im2[k*M1 + i] == 33.0))
							{
							        P2 += 1.414;
							}
						}
					}

					tbpf = (P2 - P1)/(A2 - A1);

					fprintf(fp,"%f %f %f %f %f %f %f %f\n",bv,tv, bvtv * 100.0, sv, tb, tpd, tps, tbpf);
					fprintf(fp,"----------------------------------------------------------\n");
				}
				fclose(fp);
				DEALLOCATE(data_index1);
				DEALLOCATE(Im1);
				DEALLOCATE(Im2);
				DEALLOCATE(Im3);
				DEALLOCATE(Im4);
			}
			else
			{
			        display_message(ERROR_MESSAGE,
				"Image_cache_cube_plugin_all.  Invalid arguments.");
			        return_code = 0;
			}

			DEALLOCATE(image->data);
			image->data = storage;
			for (i = 0 ; i < dimension ; i++)
			{
				image->sizes[i] = output_sizes[i];
			}
			image->depth = 12;
			image->valid = 1;

			DEALLOCATE(rot1);
        		DEALLOCATE(rot2);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Image_cache_cube_plugin_all.  Not enough memory");
			return_code = 0;
		}
       	}
	else
	{
		display_message(ERROR_MESSAGE, "Image_cache_cube_plugin_all.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Image_cache_structure_indices */

static int Computed_field_cube_plugin_all_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 10 April 2004

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int return_code;
	struct Computed_field_cube_plugin_all_type_specific_data *data;

	ENTER(Computed_field_cube_plugin_all_evaluate_cache_at_node);
	if (field && node &&
		(data = (struct Computed_field_cube_plugin_all_type_specific_data *)field->type_specific_data))
	{
		return_code = 1;
		/* 1. Precalculate the Image_cache */
		if (!data->image->valid)
		{
		        Image_cache_update_dimension(data->image,
				data->dimension, data->image->depth,
				data->input_sizes, data->image->minimums,
				data->image->maximums);
			return_code = Image_cache_update_from_fields(data->image, field->source_fields[0],
				field->source_fields[1], data->element_dimension, data->region,
				data->graphics_buffer_package);
			/* 2. Perform image processing operation */

			return_code = Image_cache_structure_indices(data->image, data->number_of_dirs,
			        data->radius, data->pixel_size,
				data->object_dimension, data->dimension, data->output_sizes, data->output_file_name);
		}
		/* 3. Evaluate texture coordinates and copy image to field */
		Computed_field_evaluate_cache_at_node(field->source_fields[1],
			node, time);
		Image_cache_evaluate_field(data->image,field);

	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_cube_plugin_all_evaluate_cache_at_node.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_cube_plugin_all_evaluate_cache_at_node */

static int Computed_field_cube_plugin_all_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 10 April 2004

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int return_code;
	struct Computed_field_cube_plugin_all_type_specific_data *data;

	ENTER(Computed_field_cube_plugin_all_evaluate_cache_in_element);
	USE_PARAMETER(calculate_derivatives);
	if (field && element && xi && (field->number_of_source_fields > 0) &&
		(data = (struct Computed_field_cube_plugin_all_type_specific_data *) field->type_specific_data) &&
		data->image )
	{
		return_code = 1;
		/* 1. Precalculate the Image_cache */
		if (!data->image->valid)
		{
			return_code = Image_cache_update_from_fields(data->image, field->source_fields[0],
				field->source_fields[1], data->element_dimension, data->region,
				data->graphics_buffer_package);
			/* 2. Perform image processing operation */
			return_code = Image_cache_structure_indices(data->image, data->number_of_dirs,
			        data->radius, data->pixel_size,
				data->object_dimension, data->dimension, data->output_sizes, data->output_file_name);
		}
		/* 3. Evaluate texture coordinates and copy image to field */
		Computed_field_evaluate_cache_in_element(field->source_fields[1],
			element, xi, time, top_level_element, /*calculate_derivatives*/0);
		Image_cache_evaluate_field(data->image,field);

	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_cube_plugin_all_evaluate_cache_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_cube_plugin_all_evaluate_cache_in_element */

#define Computed_field_cube_plugin_all_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 10 April 2004

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_cube_plugin_all_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 10 April 2004

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_cube_plugin_all_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 10 April 2004

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_cube_plugin_all_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 10 April 2004

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_cube_plugin_all_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 10 April 2004

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_cube_plugin_all_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 10 April 2004

DESCRIPTION :
Not implemented yet.
==============================================================================*/

static int list_Computed_field_cube_plugin_all(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 4 April 2004

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Computed_field_cube_plugin_all_type_specific_data *data;

	ENTER(List_Computed_field_cube_plugin_all);
	if (field && (field->type_string==computed_field_cube_plugin_all_type_string)
		&& (data = (struct Computed_field_cube_plugin_all_type_specific_data *)
		field->type_specific_data))
	{
		display_message(INFORMATION_MESSAGE,
			"    source field : %s\n",field->source_fields[0]->name);
		display_message(INFORMATION_MESSAGE,
			"    texture coordinate field : %s\n",field->source_fields[1]->name);
		display_message(INFORMATION_MESSAGE,
			"    number_of_dirs : %d\n", data->number_of_dirs);
		display_message(INFORMATION_MESSAGE,
			"    radius : %d\n", data->radius);
		display_message(INFORMATION_MESSAGE,
			"    pixel_size : %f\n", data->pixel_size);
		display_message(INFORMATION_MESSAGE,
			"    object_dimension : %d\n", data->object_dimension);
		display_message(INFORMATION_MESSAGE,
			"    output_file_name : %s\n", data->output_file_name);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_cube_plugin_all.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_cube_plugin_all */

static char *Computed_field_cube_plugin_all_get_command_string(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 4 April 2004

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name, temp_string[40];
	int error;
	struct Computed_field_cube_plugin_all_type_specific_data *data;

	ENTER(Computed_field_cube_plugin_all_get_command_string);
	command_string = (char *)NULL;
	if (field && (field->type_string==computed_field_cube_plugin_all_type_string)
		&& (data = (struct Computed_field_cube_plugin_all_type_specific_data *)
		field->type_specific_data) )
	{
		error = 0;
		append_string(&command_string,
			computed_field_cube_plugin_all_type_string, &error);
		append_string(&command_string, " field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[0], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		append_string(&command_string, " texture_coordinate_field ", &error);
		if (GET_NAME(Computed_field)(field->source_fields[1], &field_name))
		{
			make_valid_token(&field_name);
			append_string(&command_string, field_name, &error);
			DEALLOCATE(field_name);
		}
		sprintf(temp_string, " dimension %d ", data->dimension);
		append_string(&command_string, temp_string, &error);

		sprintf(temp_string, "number_of_dirs %d ", data->number_of_dirs);
		append_string(&command_string, temp_string, &error);

		sprintf(temp_string, "radius %d ", data->radius);
		append_string(&command_string, temp_string, &error);

		sprintf(temp_string, "pixel_size %f ", data->pixel_size);
		append_string(&command_string, temp_string, &error);

		sprintf(temp_string, "object_dimension %d ", data->object_dimension);
		append_string(&command_string, temp_string, &error);

		sprintf(temp_string, " input_sizes %d %d %d",
		                    data->input_sizes[0],data->input_sizes[1],data->input_sizes[2]);
		append_string(&command_string, temp_string, &error);

		sprintf(temp_string, " output_sizes %d %d %d",
		                    data->output_sizes[0],data->output_sizes[1],data->output_sizes[2]);
		append_string(&command_string, temp_string, &error);

		sprintf(temp_string, " output_file_name %s ",
		                  data->output_file_name);
		append_string(&command_string, temp_string, &error);

		sprintf(temp_string, " minimums %f %f %f ",
		                    data->image->minimums[0],data->image->minimums[1], data->image->minimums[2]);
		append_string(&command_string, temp_string, &error);

		sprintf(temp_string, " maximums %f %f %f ",
		                    data->image->maximums[0],data->image->maximums[1],data->image->maximums[2]);
		append_string(&command_string, temp_string, &error);

	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_cube_plugin_all_get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_cube_plugin_all_get_command_string */

#define Computed_field_cube_plugin_all_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 4 April 2004

DESCRIPTION :
Works out whether time influences the field.
==============================================================================*/

int Computed_field_set_type_cube_plugin_all(struct Computed_field *field,
	struct Computed_field *source_field,
	struct Computed_field *texture_coordinate_field,
	int dimension, int number_of_dirs, int radius, double pixel_size,
	int object_dimension, char *output_file_name,
	int *input_sizes, int *output_sizes, FE_value *minimums, FE_value *maximums,
	int element_dimension, struct MANAGER(Computed_field) *computed_field_manager,
	struct Cmiss_region *region, struct Graphics_buffer_package *graphics_buffer_package)
/*******************************************************************************
LAST MODIFIED : 17 April 2004

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_cube_plugin_all with the supplied
fields, <source_field> and <texture_coordinate_field>.  The <number_of_dirs> specifies
the quantization level of colour range.  The <dimension> is the
size of the <sizes>, <minimums> and <maximums> vectors and should be less than
or equal to the number of components in the <texture_coordinate_field>.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int depth, number_of_source_fields, return_code, i;
	struct Computed_field **source_fields;
	struct Computed_field_cube_plugin_all_type_specific_data *data;

	ENTER(Computed_field_set_type_cube_plugin_all);
	if (field && source_field && texture_coordinate_field && (object_dimension > 0) &&
		(number_of_dirs > 0) && (depth = source_field->number_of_components) &&
		(dimension <= texture_coordinate_field->number_of_components) &&
		region && graphics_buffer_package)
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=2;
		data = (struct Computed_field_cube_plugin_all_type_specific_data *)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, number_of_source_fields) &&
			ALLOCATE(data, struct Computed_field_cube_plugin_all_type_specific_data, 1) &&
			ALLOCATE(data->input_sizes, int, dimension) &&
			ALLOCATE(data->output_sizes, int, dimension) &&
			(data->image = ACCESS(Image_cache)(CREATE(Image_cache)())) &&
			Image_cache_update_dimension(
			data->image, dimension, depth, input_sizes, minimums, maximums) &&
			Image_cache_update_data_storage(data->image))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type_string = computed_field_cube_plugin_all_type_string;
			field->number_of_components = 12;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			source_fields[1]=ACCESS(Computed_field)(texture_coordinate_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;
			data->number_of_dirs = number_of_dirs;
			data->radius = radius;
			data->pixel_size = pixel_size;
			data->object_dimension = object_dimension;
			data->dimension = dimension;
			data->output_file_name = output_file_name;
			for (i = 0; i < dimension; i++)
			{
				data->input_sizes[i] = input_sizes[i];
				data->output_sizes[i] = output_sizes[i];
			}
			data->element_dimension = element_dimension;
			data->region = ACCESS(Cmiss_region)(region);
			data->graphics_buffer_package = graphics_buffer_package;
			data->computed_field_manager = computed_field_manager;
			data->computed_field_manager_callback_id =
				MANAGER_REGISTER(Computed_field)(
				Computed_field_cube_plugin_all_field_change, (void *)field,
				computed_field_manager);

			field->type_specific_data = data;

			/* Set all the methods */
			COMPUTED_FIELD_ESTABLISH_METHODS(cube_plugin_all);
		}
		else
		{
			DEALLOCATE(source_fields);
			if (data)
			{
				if (data->image)
				{
					DESTROY(Image_cache)(&data->image);
				}
				DEALLOCATE(data);
			}
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_set_type_cube_plugin_all.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_cube_plugin_all */

int Computed_field_get_type_cube_plugin_all(struct Computed_field *field,
	struct Computed_field **source_field,
	struct Computed_field **texture_coordinate_field,
	int *dimension, int *number_of_dirs, int *radius, double *pixel_size,
        int *object_dimension, char **output_file_name,
	int **input_sizes, int **output_sizes, FE_value **minimums,
	FE_value **maximums, int *element_dimension)
/*******************************************************************************
LAST MODIFIED : 17 April 2004

DESCRIPTION :
If the field is of type COMPUTED_FIELD_cube_plugin_all, the
parameters defining it are returned.
==============================================================================*/
{
	int i, return_code;
	struct Computed_field_cube_plugin_all_type_specific_data *data;

	ENTER(Computed_field_get_type_cube_plugin_all);
	if (field && (field->type_string==computed_field_cube_plugin_all_type_string)
		&& (data = (struct Computed_field_cube_plugin_all_type_specific_data *)
		field->type_specific_data) && data->image)
	{
		*dimension = data->image->dimension;
		if (ALLOCATE(*input_sizes, int, *dimension)
			&& ALLOCATE(*output_sizes, int, *dimension)
			&& ALLOCATE(*minimums, FE_value, *dimension)
			&& ALLOCATE(*maximums, FE_value, *dimension))
		{
			*source_field = field->source_fields[0];
			*texture_coordinate_field = field->source_fields[1];
			*number_of_dirs = data->number_of_dirs;
			*radius = data->radius;
			*pixel_size = data->pixel_size;
			*object_dimension = data->object_dimension;
			*output_file_name = data->output_file_name;
			for (i = 0 ; i < *dimension ; i++)
			{
				(*input_sizes)[i] = data->input_sizes[i];
				(*output_sizes)[i] = data->output_sizes[i];
				(*minimums)[i] = data->image->minimums[i];
				(*maximums)[i] = data->image->maximums[i];
			}
			*element_dimension = data->element_dimension;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_get_type_cube_plugin_all.  Unable to allocate vectors.");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_cube_plugin_all.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_cube_plugin_all */

static int define_Computed_field_type_cube_plugin_all(struct Parse_state *state,
	void *field_void, void *computed_field_cube_plugin_all_package_void)
/*******************************************************************************
LAST MODIFIED : 18 April 2004

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_cube_plugin_all (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	char *current_token, *output_file_name;
	FE_value *minimums, *maximums;
	int dimension, element_dimension, number_of_dirs, return_code, *input_sizes, *output_sizes;
	int radius, object_dimension;
	double pixel_size;
	struct Computed_field *field, *source_field, *texture_coordinate_field;
	struct Computed_field_cube_plugin_all_package
		*computed_field_cube_plugin_all_package;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data,
		set_texture_coordinate_field_data;

	ENTER(define_Computed_field_type_cube_plugin_all);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_cube_plugin_all_package=
		(struct Computed_field_cube_plugin_all_package *)
		computed_field_cube_plugin_all_package_void))
	{
		return_code=1;
		source_field = (struct Computed_field *)NULL;
		texture_coordinate_field = (struct Computed_field *)NULL;
		dimension = 0;
		input_sizes = (int *)NULL;
		output_sizes = (int *)NULL;
		output_file_name = (char *)NULL;
		minimums = (FE_value *)NULL;
		maximums = (FE_value *)NULL;
		element_dimension = 0;
		number_of_dirs = 0;
		radius = 0;
		pixel_size = 0.0;
		object_dimension = 0;
		/* field */
		set_source_field_data.computed_field_manager =
			computed_field_cube_plugin_all_package->computed_field_manager;
		set_source_field_data.conditional_function =
			Computed_field_has_numerical_components;
		set_source_field_data.conditional_function_user_data = (void *)NULL;
		/* texture_coordinate_field */
		set_texture_coordinate_field_data.computed_field_manager =
			computed_field_cube_plugin_all_package->computed_field_manager;
		set_texture_coordinate_field_data.conditional_function =
			Computed_field_has_numerical_components;
		set_texture_coordinate_field_data.conditional_function_user_data = (void *)NULL;

		if (computed_field_cube_plugin_all_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code = Computed_field_get_type_cube_plugin_all(field,
				&source_field, &texture_coordinate_field, &dimension, &number_of_dirs,
				&radius, &pixel_size, &object_dimension, &output_file_name,
				&input_sizes, &output_sizes, &minimums, &maximums, &element_dimension);
		}
		if (return_code)
		{
			/* must access objects for set functions */
			if (source_field)
			{
				ACCESS(Computed_field)(source_field);
			}
			if (texture_coordinate_field)
			{
				ACCESS(Computed_field)(texture_coordinate_field);
			}

			if ((current_token=state->current_token) &&
				(!(strcmp(PARSER_HELP_STRING,current_token)&&
					strcmp(PARSER_RECURSIVE_HELP_STRING,current_token))))
			{
				option_table = CREATE(Option_table)();
				/* dimension */
				Option_table_add_int_positive_entry(option_table, "dimension",
					&dimension);
				/* element_dimension */
				Option_table_add_int_non_negative_entry(option_table, "element_dimension",
					&element_dimension);
				/* field */
				Option_table_add_Computed_field_conditional_entry(option_table,
					"field", &source_field, &set_source_field_data);
				/* output_file_name */
				Option_table_add_name_entry(option_table,
					"output_file_name", &output_file_name);
				/* input_sizes */
				Option_table_add_int_vector_entry(option_table,
					"input_sizes", input_sizes, &dimension);
				/* maximums */
				Option_table_add_FE_value_vector_entry(option_table,
					"maximums", maximums, &dimension);
				/* minimums */
				Option_table_add_FE_value_vector_entry(option_table,
					"minimums", minimums, &dimension);
				/* number_of_dirs */
				Option_table_add_int_positive_entry(option_table,
					"number_of_dirs", &number_of_dirs);
				/* object_dimension */
				Option_table_add_int_positive_entry(option_table,
					"object_dimension", &object_dimension);
				/* output_sizes */
				Option_table_add_int_vector_entry(option_table,
					"output_sizes", output_sizes, &dimension);
				/* pixel_size */
				Option_table_add_double_entry(option_table,
					"pixel_size", &pixel_size);
				/* radius */
				Option_table_add_int_positive_entry(option_table,
					"radius", &radius);
				/* texture_coordinate_field */
				Option_table_add_Computed_field_conditional_entry(option_table,
					"texture_coordinate_field", &texture_coordinate_field,
					&set_texture_coordinate_field_data);
				return_code=Option_table_multi_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
			}
			/* parse the dimension ... */
			if (return_code && (current_token = state->current_token))
			{
				/* ... only if the "dimension" token is next */
				if (fuzzy_string_compare(current_token, "dimension"))
				{
					option_table = CREATE(Option_table)();
					/* dimension */
					Option_table_add_int_positive_entry(option_table, "dimension",
						&dimension);
					if (return_code = Option_table_parse(option_table, state))
					{
						if (!(REALLOCATE(input_sizes, input_sizes, int, dimension) &&
							REALLOCATE(output_sizes, output_sizes, int, dimension) &&
							REALLOCATE(minimums, minimums, FE_value, dimension) &&
							REALLOCATE(maximums, maximums, FE_value, dimension)))
						{
							return_code = 0;
						}
					}
					DESTROY(Option_table)(&option_table);
				}
			}
			if (return_code && (dimension < 1))
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_scale.  Must specify a dimension first.");
				return_code = 0;
			}
			/* parse the rest of the table */
			if (return_code&&state->current_token)
			{
				option_table = CREATE(Option_table)();
				/* element_dimension */
				Option_table_add_int_non_negative_entry(option_table, "element_dimension",
					&element_dimension);
				/* field */
				Option_table_add_Computed_field_conditional_entry(option_table,
					"field", &source_field, &set_source_field_data);
				/* output_file_name */
				Option_table_add_name_entry(option_table,
					"output_file_name", &output_file_name);
				/* input_sizes */
				Option_table_add_int_vector_entry(option_table,
					"input_sizes", input_sizes, &dimension);
				/* maximums */
				Option_table_add_FE_value_vector_entry(option_table,
					"maximums", maximums, &dimension);
				/* minimums */
				Option_table_add_FE_value_vector_entry(option_table,
					"minimums", minimums, &dimension);
				/* number_of_dirs */
				Option_table_add_int_positive_entry(option_table,
					"number_of_dirs", &number_of_dirs);
				/* object_dimension */
				Option_table_add_int_positive_entry(option_table,
					"object_dimension", &object_dimension);
				/* output_sizes */
				Option_table_add_int_vector_entry(option_table,
					"output_sizes", output_sizes, &dimension);
				/* pixel_size */
				Option_table_add_double_entry(option_table,
					"pixel_size", &pixel_size);
				/* radius */
				Option_table_add_int_positive_entry(option_table,
					"radius", &radius);
				/* texture_coordinate_field */
				Option_table_add_Computed_field_conditional_entry(option_table,
					"texture_coordinate_field", &texture_coordinate_field,
					&set_texture_coordinate_field_data);
				return_code=Option_table_multi_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
			}
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code = Computed_field_set_type_cube_plugin_all(field,
					source_field, texture_coordinate_field, dimension, number_of_dirs,
					radius, pixel_size, object_dimension, output_file_name,
					input_sizes, output_sizes, minimums, maximums, element_dimension,
					computed_field_cube_plugin_all_package->computed_field_manager,
					computed_field_cube_plugin_all_package->root_region,
					computed_field_cube_plugin_all_package->graphics_buffer_package);
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_cube_plugin_all.  Failed");
				}
			}
			if (source_field)
			{
				DEACCESS(Computed_field)(&source_field);
			}
			if (texture_coordinate_field)
			{
				DEACCESS(Computed_field)(&texture_coordinate_field);
			}
			if (input_sizes)
			{
				DEALLOCATE(input_sizes);
			}
			if (output_sizes)
			{
				DEALLOCATE(output_sizes);
			}
			if (minimums)
			{
				DEALLOCATE(minimums);
			}
			if (maximums)
			{
				DEALLOCATE(maximums);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_cube_plugin_all.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_cube_plugin_all */

int Computed_field_register_types_cube_plugin_all(
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *root_region, struct Graphics_buffer_package *graphics_buffer_package)
/*******************************************************************************
LAST MODIFIED : 12 April 2004

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	static struct Computed_field_cube_plugin_all_package
		computed_field_cube_plugin_all_package;

	ENTER(Computed_field_register_types_cube_plugin_all);
	if (computed_field_package)
	{
		computed_field_cube_plugin_all_package.computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				computed_field_package);
		computed_field_cube_plugin_all_package.root_region = root_region;
		computed_field_cube_plugin_all_package.graphics_buffer_package = graphics_buffer_package;
		return_code = Computed_field_package_add_type(computed_field_package,
			            computed_field_cube_plugin_all_type_string,
			            define_Computed_field_type_cube_plugin_all,
			            &computed_field_cube_plugin_all_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_cube_plugin_all.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_cube_plugin_all */

