/************************************************************************************
   FILE: computed_field_sterology_measures.c

   LAST MODIFIED: 25 June 2004

   DESCRIPTION: Perform bone parameters extraction on Computed field.
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
#include "image_processing/computed_field_sterology_measures.h"
#include "image_processing/computed_field_median_filter.h"
#include "image_processing/morphology_functions.h"

#define my_Min(x,y) ((x) <= (y) ? (x) : (y))
#define my_Max(x,y) ((x) <= (y) ? (y) : (x))

#undef  A
#define A(X,Y)  *(a + (X)*dim + (Y))
#undef  IN
#define IN(X,Y)  *(in + (X)*dim + (Y))
#define BIGNUM  1.0E15
#define DISTANCE sqrt((x - xctr) * (x - xctr) + (y - yctr) * (y - yctr) + (z - zctr) * (z - zctr))
#define DISTANCE1 sqrt((x - xctr) * (x - xctr) + (y - yctr) * (y - yctr))


struct Computed_field_sterology_measures_package
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

struct Computed_field_sterology_measures_type_specific_data
{
	int number_of_dirs;
	int radius;
	double pixel_size;
	int dimension;
	int *input_sizes;
	int *output_sizes;
	float cached_time;
	int element_dimension;
	struct Set_names_from_list_data results;
	struct Cmiss_region *region;
	struct Graphics_buffer_package *graphics_buffer_package;
	struct Image_cache *image;
	struct MANAGER(Computed_field) *computed_field_manager;
	void *computed_field_manager_callback_id;
};

static char computed_field_sterology_measures_type_string[] = "sterology_measures";

int Computed_field_is_type_sterology_measures(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 10 April 2004

DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Computed_field_is_type_sterology_measures);
	if (field)
	{
		return_code =
		  (field->type_string == computed_field_sterology_measures_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_sterology_measures.  Missing field");
		return_code = 0;
	}

	return (return_code);
} /* Computed_field_is_type_sterology_measures */

static void Computed_field_sterology_measures_field_change(
	struct MANAGER_MESSAGE(Computed_field) *message, void *field_void)
/*******************************************************************************
LAST MODIFIED : 5 April 2004

DESCRIPTION :
Manager function called back when either of the source fields change so that
we know to invalidate the image cache.
==============================================================================*/
{
	struct Computed_field *field;
	struct Computed_field_sterology_measures_type_specific_data *data;

	ENTER(Computed_field_sterology_measures_source_field_change);
	if (message && (field = (struct Computed_field *)field_void) && (data =
		(struct Computed_field_sterology_measures_type_specific_data *)
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
			"Computed_field_sterology_measures_source_field_change.  "
			"Invalid arguments.");
	}
	LEAVE;
} /* Computed_field_sterology_measures_source_field_change */

static int Computed_field_sterology_measures_clear_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 10 April 2004

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	int return_code;
	struct Computed_field_sterology_measures_type_specific_data *data;

	ENTER(Computed_field_sterology_measures_clear_type_specific);
	if (field && (data =
		(struct Computed_field_sterology_measures_type_specific_data *)
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
			"Computed_field_sterology_measures_clear_type_specific.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_sterology_measures_clear_type_specific */

static void *Computed_field_sterology_measures_copy_type_specific(
	struct Computed_field *source_field, struct Computed_field *destination_field)
/*******************************************************************************
LAST MODIFIED : 10 April 2004

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	struct Computed_field_sterology_measures_type_specific_data *destination,
		*source;
	int i;

	ENTER(Computed_field_sterology_measures_copy_type_specific);
	if (source_field && destination_field && (source =
		(struct Computed_field_sterology_measures_type_specific_data *)
		source_field->type_specific_data))
	{
		if (ALLOCATE(destination,
			struct Computed_field_sterology_measures_type_specific_data, 1))
		{
			destination->number_of_dirs = source->number_of_dirs;
			destination->radius = source->radius;
			destination->pixel_size = source->pixel_size;
			destination->dimension = source->dimension;
			destination->results = source->results;
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
				Computed_field_sterology_measures_field_change, (void *)destination_field,
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
				"Computed_field_sterology_measures_copy_type_specific.  "
				"Unable to allocate memory.");
			destination = NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_sterology_measures_copy_type_specific.  "
			"Invalid arguments.");
		destination = NULL;
	}
	LEAVE;

	return (destination);
} /* Computed_field_sterology_measures_copy_type_specific */

int Computed_field_sterology_measures_clear_cache_type_specific
   (struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 10 April 2004

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Computed_field_sterology_measures_type_specific_data *data;

	ENTER(Computed_field_sterology_measures_clear_type_specific);
	if (field && (data =
		(struct Computed_field_sterology_measures_type_specific_data *)
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
			"Computed_field_sterology_measures_clear_type_specific.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_sterology_measures_clear_type_specific */

static int Computed_field_sterology_measures_type_specific_contents_match(
	struct Computed_field *field, struct Computed_field *other_computed_field)
/*******************************************************************************
LAST MODIFIED : 10 April 2004

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int return_code;
	struct Computed_field_sterology_measures_type_specific_data *data,
		*other_data;

	ENTER(Computed_field_sterology_measures_type_specific_contents_match);
	if (field && other_computed_field && (data =
		(struct Computed_field_sterology_measures_type_specific_data *)
		field->type_specific_data) && (other_data =
		(struct Computed_field_sterology_measures_type_specific_data *)
		other_computed_field->type_specific_data))
	{
		if ((data->number_of_dirs == other_data->number_of_dirs) &&
		        (data->radius == other_data->radius)&&
			(data->pixel_size == other_data->pixel_size)&&
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
} /* Computed_field_sterology_measures_type_specific_contents_match */

#define Computed_field_sterology_measures_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 10 April 2004

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_sterology_measures_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 10 April 2004

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_sterology_measures_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 10 April 2004

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_sterology_measures_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 10 April 2004

DESCRIPTION :
No special criteria.
==============================================================================*/


static int Image_cache_sterology_measures(struct Image_cache *image, int number_of_dirs,
         int radius, double pixel_size, int dimension, int *output_sizes,
	 struct Set_names_from_list_data results)
/*******************************************************************************
LAST MODIFIED : 24 April 2004

DESCRIPTION :
Perform MIL analysis on the image cache.
==============================================================================*/
{
	char *storage;
	FE_value *data_index, *result_index;
	int  return_code,  storage_size, out_storage_size, result_depth;
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

        int  xsize, ysize, zsize;	        /* size of data_index1			*/

        int  xctr, yctr, zctr;	        /* center of test sphere */

	FE_value  pixsize;		/* size of one side of voxel		*/
        FE_value  tline_ln;		/* length of all test lines in mm	*/
        int num_tlines;		/* number of test lines in grid		*/
        int tintrsctn;		/* total # of intersections		*/
        FE_value 	*rot1;		/* random theta rotations		*/
        FE_value	*rot2;		/* random phi rotations			*/

        FE_value 	tv, bvtv;		/* solid volume in mm^3 and BV/TV	*/
        FE_value	*mil;		/* magnitude of MIL for ith rotation	*/
        FE_value	bsbv, tbth, tbn, tbsp;	/* BS/BV Tb.Th, Tb.N and Tb.Sp		*/

        FE_value euler_number;
	FE_value mean_curvature;
	FE_value SMI; /* structure model index */

        unsigned char seed;			/* seed for random number gen.	*/
        FE_value	dradius;		/* test sphere radius (FE_value)	*/
        int	lmarg, rmarg;			/* x limits of analysis region	*/
        int	tlay, blay;			/* y limits of analysis region	*/
        int 	top, bottom;			/* z limits of analysis region	*/
	FE_value bonev, totalv;

        int	ix, iy, iz;			/* integer grid position	*/
        int	iline_sp;			/* spacing of test lines (int)	*/
        int	irot;				/* rotation loop counter	*/
        int	i;				/* loop counter			*/
        FE_value	*elvector;		/* vector of ellipsoid coeff.(6) and goodness(4)	*/
	FE_value  *eigvals, *eigvecs;
	FE_value  prin1,prin2,prin3;	/* principal MIL vectors		*/
        FE_value  deg1,deg2,deg3;		/* degrees of anisotropy		*/
        FE_value T;
	FE_value sd, vf;
	int *h;
	int levels; /* number of grey levels */

	ENTER(Image_cache_sterology_measures);
	if (image && (dimension == image->dimension) && (image->depth > 0))
	{
	        return_code = 1;
		euler_number = 0.0;
		SMI = 0.0;
		bonev = totalv = bvtv = 0.0;
		tline_ln = 0.0;
		tintrsctn = num_tlines = 0;
		prin1 = prin2 = prin3 = 0.0;
		deg1 = deg2 = deg3 = 0.0;
		iline_sp = 0;

		levels = 256;
		result_depth = 1;
		for (i = 0; i < results.number_of_tokens; i++)
		{
			if (strcmp(results.tokens[i].string,"mil") == 0)
			{
			        result_depth *= 12;
			}
			else
			{
			        result_depth *= 1;
			}
		}
		result_depth += results.number_of_tokens - 1;
                storage_size = image->depth;
		out_storage_size = result_depth;

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

	                for(irot = 0; irot < number_of_dirs; irot++)
			{
        	        	rot1[irot] =  CMGUI_RANDOM(double) * M_PI;
        	        	rot2[irot] =  CMGUI_RANDOM(double) * M_PI;
        	        }
			#elif defined (WIN32_SYSTEM)
			seed = 1;
			CMGUI_SEED_RANDOM(seed);
			for(irot = 0; irot < number_of_dirs; irot++)
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

			result_index = (FE_value *)storage;
			for (i = 0; i < out_storage_size; i++)
			{
			        *result_index = 0.0;
				result_index++;
			}
			result_index = (FE_value *)storage;
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

						Spatial_gray_tone_histogram(data_index1, radius, xctr, yctr, zctr,
                                                         xsize, ysize, zsize, levels, h);
						Spatial_total_volume(h, &tv);
						Spatial_volume_fraction(h, &vf);
						Spatial_surface_density(pixsize, h, &sd);
						tv *= pixsize * pixsize * pixsize;

						bsbv = sd / vf;	/* BS/BV, [mm^2 mm^-3] */
						tbth = (5.0 /3.0)*vf / sd;	/* Tb.Th, [mm] */
						tbn = vf / tbth;		/* Tb.N, [mm^-1] */
						tbsp = (1.0/tbn) - tbth;	/* Tb.Sp, [mm] */
						for (i = 0; i < results.number_of_tokens; i++)
						{
						        if (strcmp(results.tokens[i].string, "euler") == 0)
							{
							        Spatial_Euler_number(pixsize, h, &euler_number);
							}
							else if (strcmp(results.tokens[i].string, "smi") == 0)
							{
							        Spatial_mean_curvature(pixsize,h, &mean_curvature);
								SMI = 2.025 * 12.0 * mean_curvature *vf / (sd * sd) ;
							}
							else if ((strcmp(results.tokens[i].string, "mil") == 0) && (number_of_dirs > 0))
							{
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
							}
						}
						for (i = 0; i < results.number_of_tokens; i++)
						{
						        if (strcmp(results.tokens[i].string, "bvtv") == 0)
							{
							        *result_index = vf;
								result_index++;
							}
							else if (strcmp(results.tokens[i].string, "bsbv") == 0)
							{
							        *result_index = bsbv;
								result_index++;
							}
							else if (strcmp(results.tokens[i].string, "tbth") == 0)
							{
							        *result_index = tbth;
								result_index++;
							}
							else if (strcmp(results.tokens[i].string, "tbsp") == 0)
							{
							        *result_index = tbsp;
								result_index++;
							}
							else if (strcmp(results.tokens[i].string, "tbn") == 0)
							{
							        *result_index = tbn;
								result_index++;
							}
							else if (strcmp(results.tokens[i].string, "euler") == 0)
							{
							        *result_index = euler_number;
								result_index++;
							}
							else if (strcmp(results.tokens[i].string, "smi") == 0)
							{
							        *result_index = SMI;
								result_index++;
							}
							else if (strcmp(results.tokens[i].string, "mil") == 0)
							{
							        *result_index = eigvecs[0];
								result_index++;
								*result_index = eigvecs[3];
								result_index++;
								*result_index = eigvecs[6];
								result_index++;
								*result_index = eigvecs[1];
								result_index++;
								*result_index = eigvecs[4];
								result_index++;
								*result_index = eigvecs[7];
								result_index++;
								*result_index = eigvecs[2];
								result_index++;
								*result_index = eigvecs[5];
								result_index++;
								*result_index = eigvecs[8];
								result_index++;
								*result_index = prin1;
								result_index++;
								*result_index = prin2;
								result_index++;
								*result_index = prin3;
								result_index++;
							}
						}
					}
				}
			}
			DEALLOCATE(data_index1);
			DEALLOCATE(elvector);
        		DEALLOCATE(mil);
			DEALLOCATE(eigvals);
			DEALLOCATE(eigvecs);
			DEALLOCATE(h);
			DEALLOCATE(image->data);
			image->data = storage;
			for (i = 0 ; i < dimension ; i++)
			{
				image->sizes[i] = output_sizes[i];
			}
			image->depth = result_depth;
			image->valid = 1;

			DEALLOCATE(rot1);
        		DEALLOCATE(rot2);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Image_cache_sterology_measures.  Not enough memory");
			return_code = 0;
		}
       	}
	else
	{
		display_message(ERROR_MESSAGE, "Image_cache_sterology_measures.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Image_cache_sterology_measures */

static int Computed_field_sterology_measures_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 10 April 2004

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int return_code;
	struct Computed_field_sterology_measures_type_specific_data *data;

	ENTER(Computed_field_sterology_measures_evaluate_cache_at_node);
	if (field && node &&
		(data = (struct Computed_field_sterology_measures_type_specific_data *)field->type_specific_data))
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

			return_code = Image_cache_sterology_measures(data->image, data->number_of_dirs,
			        data->radius, data->pixel_size,
				data->dimension, data->output_sizes, data->results);
		}
		/* 3. Evaluate texture coordinates and copy image to field */
		Computed_field_evaluate_cache_at_node(field->source_fields[1],
			node, time);
		Image_cache_evaluate_field(data->image,field);

	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_sterology_measures_evaluate_cache_at_node.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_sterology_measures_evaluate_cache_at_node */

static int Computed_field_sterology_measures_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 10 April 2004

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int return_code;
	struct Computed_field_sterology_measures_type_specific_data *data;

	ENTER(Computed_field_sterology_measures_evaluate_cache_in_element);
	USE_PARAMETER(calculate_derivatives);
	if (field && element && xi && (field->number_of_source_fields > 0) &&
		(data = (struct Computed_field_sterology_measures_type_specific_data *) field->type_specific_data) &&
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
			return_code = Image_cache_sterology_measures(data->image, data->number_of_dirs,
			        data->radius, data->pixel_size,
				data->dimension, data->output_sizes, data->results);
		}
		/* 3. Evaluate texture coordinates and copy image to field */
		Computed_field_evaluate_cache_in_element(field->source_fields[1],
			element, xi, time, top_level_element, /*calculate_derivatives*/0);
		Image_cache_evaluate_field(data->image,field);

	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_sterology_measures_evaluate_cache_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_sterology_measures_evaluate_cache_in_element */

#define Computed_field_sterology_measures_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 10 April 2004

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_sterology_measures_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 10 April 2004

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_sterology_measures_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 10 April 2004

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_sterology_measures_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 10 April 2004

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_sterology_measures_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 10 April 2004

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_sterology_measures_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 10 April 2004

DESCRIPTION :
Not implemented yet.
==============================================================================*/

static int list_Computed_field_sterology_measures(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 4 April 2004

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Computed_field_sterology_measures_type_specific_data *data;

	ENTER(List_Computed_field_sterology_measures);
	if (field && (field->type_string==computed_field_sterology_measures_type_string)
		&& (data = (struct Computed_field_sterology_measures_type_specific_data *)
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
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_sterology_measures.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_sterology_measures */

static char *Computed_field_sterology_measures_get_command_string(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 4 April 2004

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name, temp_string[40];
	int error;
	struct Computed_field_sterology_measures_type_specific_data *data;

	ENTER(Computed_field_sterology_measures_get_command_string);
	command_string = (char *)NULL;
	if (field && (field->type_string==computed_field_sterology_measures_type_string)
		&& (data = (struct Computed_field_sterology_measures_type_specific_data *)
		field->type_specific_data) )
	{
		error = 0;
		append_string(&command_string,
			computed_field_sterology_measures_type_string, &error);
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

		sprintf(temp_string, " input_sizes %d %d %d",
		                    data->input_sizes[0],data->input_sizes[1],data->input_sizes[2]);
		append_string(&command_string, temp_string, &error);

		sprintf(temp_string, " output_sizes %d %d %d",
		                    data->output_sizes[0],data->output_sizes[1],data->output_sizes[2]);
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
			"Computed_field_sterology_measures_get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_sterology_measures_get_command_string */

#define Computed_field_sterology_measures_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 4 April 2004

DESCRIPTION :
Works out whether time influences the field.
==============================================================================*/

int Computed_field_set_type_sterology_measures(struct Computed_field *field,
	struct Computed_field *source_field,
	struct Computed_field *texture_coordinate_field,
	int dimension, int number_of_dirs, int radius, double pixel_size,
	int bvtv_index, int bsbv_index, int tbth_index, int tbsp_index,
	int tbn_index, int euler_index, int smi_index, int mil_index,
	int *input_sizes, int *output_sizes, FE_value *minimums, FE_value *maximums,
	int element_dimension, struct MANAGER(Computed_field) *computed_field_manager,
	struct Cmiss_region *region, struct Graphics_buffer_package *graphics_buffer_package)
/*******************************************************************************
LAST MODIFIED : 17 April 2004

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_sterology_measures with the supplied
fields, <source_field> and <texture_coordinate_field>.  The <number_of_dirs> specifies
the quantization level of colour range.  The <dimension> is the
size of the <sizes>, <minimums> and <maximums> vectors and should be less than
or equal to the number of components in the <texture_coordinate_field>.
If function fails, field is guaranteed to be unchanged from its original state,
although its cache may be lost.
==============================================================================*/
{
	int depth, number_of_source_fields, return_code, i, result_depth, number_of_results;
	struct Computed_field **source_fields;
	struct Computed_field_sterology_measures_type_specific_data *data;

	ENTER(Computed_field_set_type_sterology_measures);
	if (field && source_field && texture_coordinate_field &&
		(number_of_dirs > 0) && (depth = source_field->number_of_components) &&
		(dimension <= texture_coordinate_field->number_of_components) &&
		region && graphics_buffer_package)
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=2;
		data = (struct Computed_field_sterology_measures_type_specific_data *)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, number_of_source_fields) &&
			ALLOCATE(data, struct Computed_field_sterology_measures_type_specific_data, 1) &&
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
			field->type_string = computed_field_sterology_measures_type_string;
			/* 4. calculate the number of results and make dynamic allocation for data*/
			number_of_results = 0;
			number_of_results = my_Max(number_of_results, bvtv_index);
			number_of_results = my_Max(number_of_results, bsbv_index);
			number_of_results = my_Max(number_of_results, tbth_index);
			number_of_results = my_Max(number_of_results, tbsp_index);
			number_of_results = my_Max(number_of_results, tbn_index);
			number_of_results = my_Max(number_of_results, euler_index);
			number_of_results = my_Max(number_of_results, smi_index);
			number_of_results = my_Max(number_of_results, mil_index);
			data->results.number_of_tokens = number_of_results;

			ALLOCATE(data->results.tokens, struct Set_names_from_list_token, number_of_results);

			/* 5. Evaluate the results data and calculate the number_of_components for field */
			for (i = 0; i < number_of_results; i++)
			{
			        if (bvtv_index == i + 1)
				{
				        data->results.tokens[i].string = "bvtv";
					data->results.tokens[i].index = bvtv_index;
				}
				else if (bsbv_index == i + 1)
				{
				        data->results.tokens[i].string = "bsbv";
					data->results.tokens[i].index = bsbv_index;
				}
				else if (tbth_index == i + 1)
				{
				        data->results.tokens[i].string = "tbth";
					data->results.tokens[i].index = tbth_index;
				}
				else if (tbsp_index == i + 1)
				{
				        data->results.tokens[i].string = "tbsp";
					data->results.tokens[i].index = tbsp_index;
				}
				else if (tbn_index == i + 1)
				{
				        data->results.tokens[i].string = "tbn";
					data->results.tokens[i].index = tbn_index;
				}
				else if (euler_index == i + 1)
				{
				        data->results.tokens[i].string = "euler";
					data->results.tokens[i].index = euler_index;
				}
				else if (smi_index == i + 1)
				{
				        data->results.tokens[i].string = "smi";
					data->results.tokens[i].index = smi_index;
				}
				else if (mil_index == i + 1)
				{
				        data->results.tokens[i].string = "mil";
					data->results.tokens[i].index = mil_index;
				}
			}
			if (mil_index > 0)
			{
			        result_depth = number_of_results + 11;
			}
			else
			{
			        result_depth = number_of_results;
			}
			field->number_of_components = result_depth;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			source_fields[1]=ACCESS(Computed_field)(texture_coordinate_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;
			data->number_of_dirs = number_of_dirs;
			data->radius = radius;
			data->pixel_size = pixel_size;
			data->dimension = dimension;
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
				Computed_field_sterology_measures_field_change, (void *)field,
				computed_field_manager);

			field->type_specific_data = data;

			/* Set all the methods */
			COMPUTED_FIELD_ESTABLISH_METHODS(sterology_measures);
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
			"Computed_field_set_type_sterology_measures.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_sterology_measures */

int Computed_field_get_type_sterology_measures(struct Computed_field *field,
	struct Computed_field **source_field,
	struct Computed_field **texture_coordinate_field,
	int *dimension, int *number_of_dirs, int *radius, double *pixel_size,
	int *bvtv_index, int *bsbv_index, int *tbth_index, int *tbsp_index,
	int *tbn_index, int *euler_index, int *smi_index, int *mil_index,
	int **input_sizes, int **output_sizes, FE_value **minimums,
	FE_value **maximums, int *element_dimension)
/*******************************************************************************
LAST MODIFIED : 17 April 2004

DESCRIPTION :
If the field is of type COMPUTED_FIELD_sterology_measures, the
parameters defining it are returned.
==============================================================================*/
{
	int i, return_code;
	struct Computed_field_sterology_measures_type_specific_data *data;

	ENTER(Computed_field_get_type_sterology_measures);
	if (field && (field->type_string==computed_field_sterology_measures_type_string)
		&& (data = (struct Computed_field_sterology_measures_type_specific_data *)
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
			for (i = 0 ; i < *dimension ; i++)
			{
				(*input_sizes)[i] = data->input_sizes[i];
				(*output_sizes)[i] = data->output_sizes[i];
				(*minimums)[i] = data->image->minimums[i];
				(*maximums)[i] = data->image->maximums[i];
			}
			*bvtv_index = *bsbv_index = *tbth_index = *tbsp_index = 0;
			*tbn_index = *euler_index = *smi_index = *mil_index = 0;
			for (i = 0; i < data->results.number_of_tokens; i++)
			{
			        if (strcmp(data->results.tokens[i].string, "bvtv") == 0)
				{
				        *bvtv_index = i + 1;
				}
				else if (strcmp(data->results.tokens[i].string, "bsbv") == 0)
				{
				        *bsbv_index = i + 1;
				}
				else if (strcmp(data->results.tokens[i].string, "tbth") == 0)
				{
				        *tbth_index = i + 1;
				}
				else if (strcmp(data->results.tokens[i].string, "tbsp") == 0)
				{
				        *tbsp_index = i + 1;
				}
				else if (strcmp(data->results.tokens[i].string, "tbn") == 0)
				{
				        *tbn_index = i + 1;
				}
				else if (strcmp(data->results.tokens[i].string, "euler") == 0)
				{
				        *euler_index = i + 1;
				}
				else if (strcmp(data->results.tokens[i].string, "smi") == 0)
				{
				        *smi_index = i + 1;
				}
				else if (strcmp(data->results.tokens[i].string, "mil") == 0)
				{
				        *mil_index = i + 1;
				}
			}
			*element_dimension = data->element_dimension;
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_get_type_sterology_measures.  Unable to allocate vectors.");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_sterology_measures.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_sterology_measures */

static int define_Computed_field_type_sterology_measures(struct Parse_state *state,
	void *field_void, void *computed_field_sterology_measures_package_void)
/*******************************************************************************
LAST MODIFIED : 18 April 2004

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_sterology_measures (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	char bvtv_string[] = "bvtv", *current_token, mil_string[] = "mil";
	char bsbv_string[] = "bsbv", tbth_string[] = "tbth", tbsp_string[] = "tbsp";
	char tbn_string[] = "tbn", euler_string[] = "euler", smi_string[] = "smi";
	FE_value *minimums, *maximums;
	int dimension, element_dimension, number_of_dirs,
		radius, return_code, *input_sizes, *output_sizes;
	double pixel_size;
	struct Computed_field *field, *source_field, *texture_coordinate_field;
	struct Computed_field_sterology_measures_package
		*computed_field_sterology_measures_package;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data,
		set_texture_coordinate_field_data;
	struct Set_names_from_list_data results;

	ENTER(define_Computed_field_type_sterology_measures);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_sterology_measures_package=
		(struct Computed_field_sterology_measures_package *)
		computed_field_sterology_measures_package_void))
	{
		return_code=1;
		source_field = (struct Computed_field *)NULL;
		texture_coordinate_field = (struct Computed_field *)NULL;
		dimension = 0;
		input_sizes = (int *)NULL;
		output_sizes = (int *)NULL;
		minimums = (FE_value *)NULL;
		maximums = (FE_value *)NULL;
		element_dimension = 0;
		number_of_dirs = 0;
		radius = 0;
		pixel_size = 0.0;
		/* field */
		set_source_field_data.computed_field_manager =
			computed_field_sterology_measures_package->computed_field_manager;
		set_source_field_data.conditional_function =
			Computed_field_has_numerical_components;
		set_source_field_data.conditional_function_user_data = (void *)NULL;
		/* results */
		results.number_of_tokens = 8;
		ALLOCATE(results.tokens, struct Set_names_from_list_token, 8);
		results.tokens[0].string = bvtv_string;
		results.tokens[0].index = 0;
		results.tokens[1].string = bsbv_string;
		results.tokens[1].index = 0;
		results.tokens[2].string = tbth_string;
		results.tokens[2].index = 0;
		results.tokens[3].string = tbsp_string;
		results.tokens[3].index = 0;
		results.tokens[4].string = tbn_string;
		results.tokens[4].index = 0;
		results.tokens[5].string = euler_string;
		results.tokens[5].index = 0;
		results.tokens[6].string = smi_string;
		results.tokens[6].index = 0;
		results.tokens[7].string = mil_string;
		results.tokens[7].index = 0;
		/* texture_coordinate_field */
		set_texture_coordinate_field_data.computed_field_manager =
			computed_field_sterology_measures_package->computed_field_manager;
		set_texture_coordinate_field_data.conditional_function =
			Computed_field_has_numerical_components;
		set_texture_coordinate_field_data.conditional_function_user_data = (void *)NULL;

		if (computed_field_sterology_measures_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code = Computed_field_get_type_sterology_measures(field,
				&source_field, &texture_coordinate_field, &dimension, &number_of_dirs,
				&radius, &pixel_size, &results.tokens[0].index, &results.tokens[1].index,
				&results.tokens[2].index, &results.tokens[3].index, &results.tokens[4].index,
				&results.tokens[5].index, &results.tokens[6].index, &results.tokens[7].index,
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
				/* results */
				Option_table_add_set_names_from_list_entry(option_table,
					"results", &results);
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
				Option_table_add_int_non_negative_entry(option_table,
					"number_of_dirs", &number_of_dirs);
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
				/* results */
				Option_table_add_set_names_from_list_entry(option_table,
					"results", &results);
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
				Option_table_add_int_non_negative_entry(option_table,
					"number_of_dirs", &number_of_dirs);
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
				return_code = Computed_field_set_type_sterology_measures(field,
					source_field, texture_coordinate_field, dimension, number_of_dirs,
					radius, pixel_size, results.tokens[0].index, results.tokens[1].index,
				        results.tokens[2].index, results.tokens[3].index, results.tokens[4].index,
				        results.tokens[5].index, results.tokens[6].index, results.tokens[7].index,
					input_sizes, output_sizes, minimums, maximums, element_dimension,
					computed_field_sterology_measures_package->computed_field_manager,
					computed_field_sterology_measures_package->root_region,
					computed_field_sterology_measures_package->graphics_buffer_package);
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_sterology_measures.  Failed");
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
			"define_Computed_field_type_sterology_measures.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_sterology_measures */

int Computed_field_register_types_sterology_measures(
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *root_region, struct Graphics_buffer_package *graphics_buffer_package)
/*******************************************************************************
LAST MODIFIED : 12 April 2004

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	static struct Computed_field_sterology_measures_package
		computed_field_sterology_measures_package;

	ENTER(Computed_field_register_types_sterology_measures);
	if (computed_field_package)
	{
		computed_field_sterology_measures_package.computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				computed_field_package);
		computed_field_sterology_measures_package.root_region = root_region;
		computed_field_sterology_measures_package.graphics_buffer_package = graphics_buffer_package;
		return_code = Computed_field_package_add_type(computed_field_package,
			            computed_field_sterology_measures_type_string,
			            define_Computed_field_type_sterology_measures,
			            &computed_field_sterology_measures_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_sterology_measures.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_sterology_measures */

