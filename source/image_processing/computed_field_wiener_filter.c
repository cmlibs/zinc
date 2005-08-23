/******************************************************************
  FILE: computed_field_wiener_filter.c

  LAST MODIFIED: 11 June 2004

  DESCRIPTION:Implement image decovolution in the basis of Wiener filter
==================================================================*/
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
 * Portions created by the Initial Developer are Copyright (C) 2005
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
#include <math.h>
#include "computed_field/computed_field.h"
#include "computed_field/computed_field_find_xi.h"
#include "computed_field/computed_field_private.h"
#include "computed_field/computed_field_set.h"
#include "image_processing/image_cache.h"
#include "general/image_utilities.h"
#include "general/debug.h"
#include "general/mystring.h"
#include "user_interface/message.h"
#include "image_processing/computed_field_wiener_filter.h"


struct Computed_field_wiener_filter_package
/*******************************************************************************
LAST MODIFIED : 17 March 2004

DESCRIPTION :
A container for objects required to define fields in this module.
==============================================================================*/
{
	struct MANAGER(Computed_field) *computed_field_manager;
	struct Cmiss_region *root_region;
	struct Graphics_buffer_package *graphics_buffer_package;
};

struct Computed_field_wiener_filter_type_specific_data
{
	double sigma;
	double lambda;
	float cached_time;
	/*int element_dimension;*/
	struct Cmiss_region *region;
	struct Graphics_buffer_package *graphics_buffer_package;
	struct Image_cache *image;
	struct MANAGER(Computed_field) *computed_field_manager;
	void *computed_field_manager_callback_id;
};

static char computed_field_wiener_filter_type_string[] = "wiener_filter";

int Computed_field_is_type_wiener_filter(struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 6 January 2004

DESCRIPTION :
==============================================================================*/
{
	int return_code;


	ENTER(Computed_field_is_type_wiener_filter);
	if (field)
	{
		return_code =
		  (field->type_string == computed_field_wiener_filter_type_string);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_is_type_wiener_filter.  Missing field");
		return_code = 0;
	}

	return (return_code);
} /* Computed_field_is_type_wiener_filter */

static void Computed_field_wiener_filter_field_change(
	struct MANAGER_MESSAGE(Computed_field) *message, void *field_void)
/*******************************************************************************
LAST MODIFIED : 5 December 2003

DESCRIPTION :
Manager function called back when either of the source fields change so that
we know to invalidate the image cache.
==============================================================================*/
{
	struct Computed_field *field;
	struct Computed_field_wiener_filter_type_specific_data *data;

	ENTER(Computed_field_wiener_filter_source_field_change);
	if (message && (field = (struct Computed_field *)field_void) && (data =
		(struct Computed_field_wiener_filter_type_specific_data *)
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
			"Computed_field_wiener_filter_source_field_change.  "
			"Invalid arguments.");
	}
	LEAVE;
} /* Computed_field_wiener_filter_source_field_change */

static int Computed_field_wiener_filter_clear_type_specific(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 6 January 2004

DESCRIPTION :
Clear the type specific data used by this type.
==============================================================================*/
{
	int return_code;
	struct Computed_field_wiener_filter_type_specific_data *data;

	ENTER(Computed_field_wiener_filter_clear_type_specific);
	if (field && (data =
		(struct Computed_field_wiener_filter_type_specific_data *)
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
		DEALLOCATE(field->type_specific_data);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_wiener_filter_clear_type_specific.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_wiener_filter_clear_type_specific */

static void *Computed_field_wiener_filter_copy_type_specific(
	struct Computed_field *source_field, struct Computed_field *destination_field)
/*******************************************************************************
LAST MODIFIED : 6 January 2004

DESCRIPTION :
Copy the type specific data used by this type.
==============================================================================*/
{
	struct Computed_field_wiener_filter_type_specific_data *destination,
		*source;

	ENTER(Computed_field_wiener_filter_copy_type_specific);
	if (source_field && destination_field && (source =
		(struct Computed_field_wiener_filter_type_specific_data *)
		source_field->type_specific_data))
	{
		if (ALLOCATE(destination,
			struct Computed_field_wiener_filter_type_specific_data, 1))
		{
			destination->sigma = source->sigma;
			destination->lambda = source->lambda;
			destination->cached_time = source->cached_time;
			destination->region = ACCESS(Cmiss_region)(source->region);
			/*destination->element_dimension = source->element_dimension;*/
			destination->graphics_buffer_package = source->graphics_buffer_package;
			destination->computed_field_manager = source->computed_field_manager;
			destination->computed_field_manager_callback_id =
				MANAGER_REGISTER(Computed_field)(
				Computed_field_wiener_filter_field_change, (void *)destination_field,
				destination->computed_field_manager);
			if (source->image)
			{
				destination->image = ACCESS(Image_cache)(CREATE(Image_cache)());
				Image_cache_update_dimension(destination->image,
					source->image->dimension, source->image->depth,
					source->image->sizes);
			}
			else
			{
				destination->image = (struct Image_cache *)NULL;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_wiener_filter_copy_type_specific.  "
				"Unable to allocate memory.");
			destination = NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_wiener_filter_copy_type_specific.  "
			"Invalid arguments.");
		destination = NULL;
	}
	LEAVE;

	return (destination);
} /* Computed_field_wiener_filter_copy_type_specific */

int Computed_field_wiener_filter_clear_cache_type_specific
   (struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 6 January 2004

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Computed_field_wiener_filter_type_specific_data *data;

	ENTER(Computed_field_wiener_filter_clear_type_specific);
	if (field && (data =
		(struct Computed_field_wiener_filter_type_specific_data *)
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
			"Computed_field_wiener_filter_clear_type_specific.  "
			"Invalid arguments.");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_wiener_filter_clear_type_specific */

static int Computed_field_wiener_filter_type_specific_contents_match(
	struct Computed_field *field, struct Computed_field *other_computed_field)
/*******************************************************************************
LAST MODIFIED : 6 January 2004

DESCRIPTION :
Compare the type specific data
==============================================================================*/
{
	int return_code;
	struct Computed_field_wiener_filter_type_specific_data *data,
		*other_data;

	ENTER(Computed_field_wiener_filter_type_specific_contents_match);
	if (field && other_computed_field && (data =
		(struct Computed_field_wiener_filter_type_specific_data *)
		field->type_specific_data) && (other_data =
		(struct Computed_field_wiener_filter_type_specific_data *)
		other_computed_field->type_specific_data))
	{
		if ((data->sigma == other_data->sigma) &&
		        (data->lambda == other_data->lambda) &&
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
} /* Computed_field_wiener_filter_type_specific_contents_match */

#define Computed_field_wiener_filter_is_defined_in_element \
	Computed_field_default_is_defined_in_element
/*******************************************************************************
LAST MODIFIED : 6 January 2004

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_wiener_filter_is_defined_at_node \
	Computed_field_default_is_defined_at_node
/*******************************************************************************
LAST MODIFIED : 6 January 2004

DESCRIPTION :
Check the source fields using the default.
==============================================================================*/

#define Computed_field_wiener_filter_has_numerical_components \
	Computed_field_default_has_numerical_components
/*******************************************************************************
LAST MODIFIED : 6 January 2004

DESCRIPTION :
Window projection does have numerical components.
==============================================================================*/

#define Computed_field_wiener_filter_not_in_use \
	(Computed_field_not_in_use_function)NULL
/*******************************************************************************
LAST MODIFIED : 6 January 2004

DESCRIPTION :
No special criteria.
==============================================================================*/


int This_fft1d(FE_value *Xr, FE_value *Xi, FE_value *Yr, FE_value *Yi,
         char *inverse, int data_size)
/****************************************************************************
      LAST MODIFIED: 2 June 2004

      DESCRIPTION: Implement 1D fast Fourier transform. The input <Xr> is the real part,
      <Xi> is the image part. <Yr> and <Yi> are the real and image parts of the output,
      respectively.
============================================================================*/
{
        int size,n,mmax,m,j,istep,i,isign;
	FE_value wtemp,wr,wpr,wpi,wi,theta;
	FE_value tempr,tempi;
	FE_value  *data;
	int return_code;
	ENTER(This_fft1d);
	size = data_size;
	if (!inverse)
	{
	       isign = 1;
	}
	else
	{
	       isign = -1;
	}
	if (ALLOCATE(data, FE_value, 2 * size))
	{
	        return_code = 1;
		for (i=0;i<size;i++)
		{
		        data[2*i] = Xr[i];

			data[2*i+1] = Xi[i];
		}
		n = size << 1;
		j = 1;
		for (i = 1; i < n; i += 2)
		{
		        if (j > i)
			{
			        SWAP(data[j-1],data[i-1]);
				SWAP(data[j],data[i]);
			}
			m = n >> 1;
			while (m >=2 && j>m)
			{
			        j = j-m;
				m >>= 1;
			}
			j = j+m;
		}
		mmax = 2;
		while (n > mmax)
		{
		        istep = 2*mmax;
			theta = 2*M_PI/(isign*mmax);
			wtemp = sin(0.5*theta);
			wpr = -2.0*wtemp*wtemp;
			wpi = sin(theta);
			wr = 1.0;
			wi = 0.0;
			for (m = 1; m < mmax; m += 2)
			{
			        for (i = m-1; i <= n-1; i += istep)
				{
				        j = i+mmax;
					tempr = wr*data[j] - wi*data[j+1];
					tempi = wr*data[j+1] + wi*data[j];
					data[j] = data[i] - tempr;
					data[j+1] = data[i+1] - tempi;
					data[i] = data[i] + tempr;
					data[i+1] = data[i+1] + tempi;
				}
				wr = (wtemp = wr)*wpr - wi*wpi + wr;
				wi = wi*wpr + wtemp*wpi + wi;
			}
			mmax = istep;
		}
		if (inverse)
		{
		        for (i=0;i<size;i++)
			{
			        Yr[i] = data[2*i]/(FE_value)size;
			}
		}
		else
		{
		        for (i=0;i<size;i++)
			{
			        Yr[i] = data[2*i];
			}
		}
		if (inverse)
		{
		        for (i=0;i<size;i++)
			{
			        Yi[i] = data[2*i+1]/(FE_value)size;
			}
		}
		else
		{
		        for (i=0;i<size;i++)
			{
			         Yi[i] = data[2*i+1];
			}
		}
		DEALLOCATE(data);
	}
	else
	{
	        display_message(ERROR_MESSAGE,
				"In function fft1d.  "
				"Unable to allocate memory.");
		return_code = 0;
	}
	LEAVE;
	return(return_code);
}/* This_fft1d */

int This_fft2d(FE_value *in_re, FE_value *in_im, FE_value *out_re, FE_value *out_im,
         char *i_flag, int xsize, int ysize)
/****************************************************************************
      LAST MODIFIED: 2 June 2004

      DESCRIPTION: Implement 2D fast Fourier transform
============================================================================*/
{
        int      i,j,n,p;
	int return_code;
	FE_value  *f1_re, *f1_im, *f2_re, *f2_im;
	FE_value  *f3_re, *f3_im, *f4_re, *f4_im;
	FE_value  *tmp_re, *tmp_im;
	ENTER(This_fft2d);
	p = xsize;
	n = ysize;
	if (ALLOCATE(f1_re, FE_value, p) &&
	     ALLOCATE(f1_im, FE_value, p) &&
	     ALLOCATE(f2_re, FE_value, p) &&
	     ALLOCATE(f2_im, FE_value, p) &&
	     ALLOCATE(tmp_re, FE_value, xsize*ysize) &&
	     ALLOCATE(tmp_im, FE_value, xsize*ysize))
	{
	        return_code = 1;

		for (i=0;i<n;i++)
		{
		        for (j=0;j<p;j++)
			{
			        f1_re[j] = in_re[p*i+j];
				f1_im[j] = in_im[p*i+j];
			}
			This_fft1d(f1_re,f1_im,f2_re,f2_im,i_flag, p);
			for (j=0;j<p;j++)
			{
			        tmp_re[p*i+j] = f2_re[j];
				tmp_im[p*i+j] = f2_im[j];
			}
		}
		if (ALLOCATE(f3_re, FE_value, n) &&
	                 ALLOCATE(f3_im, FE_value, n) &&
			 ALLOCATE(f4_re, FE_value, n) &&
			 ALLOCATE(f4_im, FE_value, n))
		{
		        for (j=0;j<p;j++)
			{
			        for (i=0;i<n;i++)
				{
				        f3_re[i] = tmp_re[p*i+j];
					f3_im[i] = tmp_im[p*i+j];
				}
				This_fft1d(f3_re,f3_im,f4_re,f4_im,i_flag, n);
				for (i=0;i<n;i++)
				{
				        out_re[p*i+j] = f4_re[i];
					out_im[p*i+j] = f4_im[i];
				}
			}
			DEALLOCATE(f3_re);
			DEALLOCATE(f3_im);
			DEALLOCATE(f4_re);
			DEALLOCATE(f4_im);
		}
		else
		{
		        display_message(ERROR_MESSAGE,
				"In function fft2d.  "
				"Unable to allocate memory.");
			return_code = 0;
		}
		DEALLOCATE(f1_re);
		DEALLOCATE(f1_im);
		DEALLOCATE(f2_re);
		DEALLOCATE(f2_im);
		DEALLOCATE(tmp_re);
		DEALLOCATE(tmp_im);
	}
	else
	{
	        display_message(ERROR_MESSAGE,
				"In function fft2d.  "
				"Unable to allocate memory.");
		return_code = 0;
	}
	LEAVE;
	return(return_code);
}/* This_fft2d */


int This_fft3d(FE_value *in_re, FE_value *in_im, FE_value *out_re, FE_value *out_im,
         char *i_flag, int xsize, int ysize, int zsize)
/****************************************************************************
      LAST MODIFIED: 17 February 2005

      DESCRIPTION: Implement 3D fast Fourier transform
============================================================================*/
{
        int      i, j, k, n, p, q;
	int return_code;
	FE_value  *f1_re, *f1_im, *f2_re, *f2_im;
	FE_value  *f3_re, *f3_im, *f4_re, *f4_im;
	FE_value  *f5_re, *f5_im, *f6_re, *f6_im;
	FE_value  *tmp1_re, *tmp1_im;
	FE_value  *tmp2_re, *tmp2_im;
	ENTER(This_fft3d);
	p = xsize;
	n = ysize;
	q = zsize;
	if (ALLOCATE(f1_re, FE_value, p) &&
	     ALLOCATE(f1_im, FE_value, p) &&
	     ALLOCATE(f2_re, FE_value, p) &&
	     ALLOCATE(f2_im, FE_value, p) &&
	     ALLOCATE(tmp1_re, FE_value, xsize*ysize*zsize) &&
	     ALLOCATE(tmp1_im, FE_value, xsize*ysize*zsize) &&
	     ALLOCATE(tmp2_re, FE_value, xsize*ysize*zsize) &&
	     ALLOCATE(tmp2_im, FE_value, xsize*ysize*zsize) &&
	     ALLOCATE(f3_re, FE_value, n) &&
	     ALLOCATE(f3_im, FE_value, n) &&
	     ALLOCATE(f4_re, FE_value, n) &&
	     ALLOCATE(f4_im, FE_value, n) &&
	     ALLOCATE(f5_re, FE_value, q) &&
	     ALLOCATE(f5_im, FE_value, q) &&
	     ALLOCATE(f6_re, FE_value, q) &&
	     ALLOCATE(f6_im, FE_value, q))
	{
	        return_code = 1;

		for (i=0;i<q;i++)
		{
		        for (j=0;j<n;j++)
			{
			        for (k=0;k<p;k++)
				{
			                f1_re[k] = in_re[p*n*i+p*j+k];
					f1_im[k] = in_im[p*n*i+p*j+k];
				}
				This_fft1d(f1_re,f1_im,f2_re,f2_im,i_flag, p);
				for (k=0;k<p;k++)
				{
			        	tmp1_re[p*n*i+p*j+k] = f2_re[k];
					tmp1_im[p*n*i+p*j+k] = f2_im[k];
				}
			}
		}
		DEALLOCATE(f1_re);
		DEALLOCATE(f1_im);
		DEALLOCATE(f2_re);
		DEALLOCATE(f2_im);
		for (i=0;i<q;i++)
		{
			for (k=0;k<p;k++)
			{
			        for (j=0;j<n;j++)
				{
				        f3_re[j] = tmp1_re[p*n*i+p*j+k];
				        f3_im[j] = tmp1_im[p*n*i+p*j+k];
				}
				This_fft1d(f3_re,f3_im,f4_re,f4_im,i_flag, n);
				for (j=0;j<n;j++)
				{
					tmp2_re[p*n*i+p*j+k] = f4_re[j];
					tmp2_im[p*n*i+p*j+k] = f4_im[j];
				}
			}
		}
		DEALLOCATE(f3_re);
		DEALLOCATE(f3_im);
		DEALLOCATE(f4_re);
		DEALLOCATE(f4_im);
		for (j=0;j<n;j++)
		{
			for (k=0;k<p;k++)
			{
			        for (i=0;i<q;i++)
				{
				        f5_re[i] = tmp2_re[p*n*i+p*j+k];
				        f5_im[i] = tmp2_im[p*n*i+p*j+k];
				}
				This_fft1d(f5_re,f5_im,f6_re,f6_im,i_flag, q);
				for (i=0;i<q;i++)
				{
					out_re[p*n*i+p*j+k] = f6_re[i];
					out_im[p*n*i+p*j+k] = f6_im[i];
				}
			}
		}
		DEALLOCATE(f5_re);
		DEALLOCATE(f5_im);
		DEALLOCATE(f6_re);
		DEALLOCATE(f6_im);
		DEALLOCATE(tmp1_re);
		DEALLOCATE(tmp1_im);
		DEALLOCATE(tmp2_re);
		DEALLOCATE(tmp2_im);
	}
	else
	{
	        display_message(ERROR_MESSAGE,
				"In function fft3d.  "
				"Unable to allocate memory.");
		return_code = 0;
	}
	LEAVE;
	return(return_code);
}/* This_fft3d */


void This_gaussian_kernel2d(FE_value *kernel,double g, int xsize, int ysize)
/***********************************************************************
   LAST MODIFIED: 1 June 2004

   DESCRIPTION: Build a Gaussian kernel with std g in Fourier domain
========================================================================*/
{
        int nx,ny,x,y,adr;
	FE_value cx,cy;
	nx = xsize;
	ny = ysize;
	cx = (FE_value)(g*g)*2.*M_PI*M_PI/(FE_value)(nx*nx);
	cy = (FE_value)(g*g)*2.*M_PI*M_PI/(FE_value)(ny*ny);
	for (x=-nx/2;x<nx/2;x++)
	{
	        for (y=-ny/2;y<ny/2;y++)
		{
		        adr = (y<0?ny+y:y)*nx+(x<0?nx+x:x);
			kernel[adr] = (FE_value)exp(-cx*(FE_value)(x*x)-cy*(FE_value)(y*y));
		}
	}
}/* This_gaussian_kernel2d */

void This_gaussian_kernel3d(FE_value *kernel,double g, int xsize, int ysize, int zsize)
/***********************************************************************
   LAST MODIFIED: 1 June 2004

   DESCRIPTION: Build a Gaussian kernel with std g in Fourier domain
========================================================================*/
{
        int nx,ny,nz,x,y,z,adr;
	FE_value cx,cy,cz;
	nx = xsize;
	ny = ysize;
	nz = zsize;
	cx = (FE_value)(g*g)*2.*M_PI*M_PI/(FE_value)(nx*nx);
	cy = (FE_value)(g*g)*2.*M_PI*M_PI/(FE_value)(ny*ny);
	cz = (FE_value)(g*g)*2.*M_PI*M_PI/(FE_value)(nz*nz);
	for (z=-nz/2;z<nz/2;z++)
	{
		for (y=-ny/2;y<ny/2;y++)
		{
			for (x=-nx/2;x<nx/2;x++)
			{
			         adr = (z<0?nz+z:z)*nx*ny+(y<0?ny+y:y)*nx+(x<0?nx+x:x);
				 kernel[adr] = (FE_value)exp(-cx*(FE_value)(x*x)-cy*(FE_value)(y*y)-cz*(FE_value)(z*z));
			}
		}
	}
}/* This_gaussian_kernel3d */

int This_wiener2d(FE_value *data_index, FE_value *result_index, double g, double lambda,
          int xsize, int ysize)
/************************************************************************
    LAST MODIFIED: 1 June 2004

    DESCRIPTION: Implement Wiener filtering
=========================================================================*/
{
        FE_value *re, *im, *kernel, *nothing;
	int x,y,nx,ny,adr;
	FE_value c,k,rho2,cx,cy;
	int storage_size;
	int return_code;
	int i;
	ENTER(This_wiener2d);
	storage_size = xsize * ysize;

        if(ALLOCATE(re, FE_value, storage_size) &&
	         ALLOCATE(im, FE_value, storage_size) &&
		 ALLOCATE(nothing, FE_value, storage_size) &&
		 ALLOCATE(kernel, FE_value, storage_size))
	{
	        return_code = 1;
		nx = xsize;
		ny = ysize;
		for (i = 0; i < storage_size; i++)
		{
		        nothing[i] = 0.0;
		}

		This_fft2d(data_index, nothing, re, im, (char *)0, nx, ny);

		This_gaussian_kernel2d(kernel, g, xsize, ysize);

		cx = M_PI*M_PI/(FE_value)(nx*nx);
		cy = M_PI*M_PI/(FE_value)(ny*ny);
		for (x=-nx/2;x<nx/2;x++)
		{
		        for (y=-ny/2;y<ny/2;y++)
			{
			         adr = (y<0?ny+y:y)*nx+(x<0?nx+x:x);
				 rho2 = cx*(FE_value)(x*x)+cy*(FE_value)(y*y);
				 k = kernel[adr];
				 c = lambda * k/(lambda*k*k+rho2);
				 re[adr] *= c;
				 im[adr] *= c;
			}
		}
		This_fft2d(re, im, result_index, nothing, (char *)1, nx,ny);
		DEALLOCATE(re);
		DEALLOCATE(im);
		DEALLOCATE(nothing);
		DEALLOCATE(kernel);
	}
	else
	{
	        display_message(ERROR_MESSAGE,
				"In function wiener.  "
				"Unable to allocate memory.");
		return_code = 0;
	}
	LEAVE;
	return(return_code);
}/* This_wiener2d */

int This_wiener3d(FE_value *data_index, FE_value *result_index, double g, double lambda,
          int xsize, int ysize, int zsize)
/************************************************************************
    LAST MODIFIED: 17 February 2005

    DESCRIPTION: Implement 3D Wiener filtering
=========================================================================*/
{
        FE_value *re, *im, *kernel, *nothing;
	int x,y,z, nx,ny,nz,adr;
	FE_value c,k,rho2,cx,cy,cz;
	int storage_size;
	int return_code;
	int i;
	ENTER(This_wiener3d);
	storage_size = xsize * ysize * zsize;

        if(ALLOCATE(re, FE_value, storage_size) &&
	         ALLOCATE(im, FE_value, storage_size) &&
		 ALLOCATE(nothing, FE_value, storage_size) &&
		 ALLOCATE(kernel, FE_value, storage_size))
	{
	        return_code = 1;
		nx = xsize;
		ny = ysize;
		nz = zsize;
		for (i = 0; i < storage_size; i++)
		{
		        nothing[i] = 0.0;
		}

		This_fft3d(data_index, nothing, re, im, (char *)0, nx, ny, nz);

		This_gaussian_kernel3d(kernel, g, xsize, ysize, zsize);

		cx = M_PI*M_PI/(FE_value)(nx*nx);
		cy = M_PI*M_PI/(FE_value)(ny*ny);
		cz = M_PI*M_PI/(FE_value)(nz*nz);
		for (z=-nz/2;z<nz/2;z++)
		{
		        for (y=-ny/2;y<ny/2;y++)
			{
			         for (x=-nx/2;x<nx/2;x++)
				 {
			                 adr = (z<0?nz+z:z)*nx*ny+(y<0?ny+y:y)*nx+(x<0?nx+x:x);
				         rho2 = cx*(FE_value)(x*x)+cy*(FE_value)(y*y)+cz*(FE_value)(z*z);
				         k = kernel[adr];
				         c = lambda * k/(lambda*k*k+rho2);
				         re[adr] *= c;
				         im[adr] *= c;
				 }
			}
		}
		This_fft3d(re, im, result_index, nothing, (char *)1, nx,ny,nz);
		DEALLOCATE(re);
		DEALLOCATE(im);
		DEALLOCATE(nothing);
		DEALLOCATE(kernel);
	}
	else
	{
	        display_message(ERROR_MESSAGE,
				"In function wiener.  "
				"Unable to allocate memory.");
		return_code = 0;
	}
	LEAVE;
	return(return_code);
}/* This_wiener3d */

static int Image_cache_wiener_filter(struct Image_cache *image, double sigma, double lambda)
/*******************************************************************************
LAST MODIFIED : 17 March 2004

DESCRIPTION : Implement deconvolution with Wiener filter.

==============================================================================*/
{
	char *storage;
	FE_value *data_index, *result_index, *data_index1, *result_index1;
	int i, return_code, storage_size, k;

	ENTER(Image_cache_wiener_filter);
	if (image && (image->dimension == 2 ||image->dimension==3 ) && (image->depth > 0))
	{
		return_code = 1;

		storage_size = image->depth;
		for (i = 0 ; i < image->dimension ; i++)
		{
			storage_size *= image->sizes[i];
		}
		if (ALLOCATE(storage, char, storage_size * sizeof(FE_value))&&
		         ALLOCATE(data_index1, FE_value, storage_size/image->depth) &&
			 ALLOCATE(result_index1, FE_value, storage_size/image->depth))
		{
		        return_code = 1;
			result_index = (FE_value *)storage;
			for (i = 0 ; i < storage_size ; i++)
			{
				*result_index = 0.0;
				result_index++;
			}
			data_index = (FE_value *)image->data;
			result_index = (FE_value *)storage;
			for (k = 0; k < image->depth; k++)
			{
			        for (i = 0; i < storage_size/image->depth; i++)
			        {
			                 data_index1[i] = *(data_index + k);
					 data_index += image->depth;
					 result_index += image->depth;
			        }
				if (image->dimension == 2)
				{
			                 This_wiener2d(data_index1, result_index1, sigma, lambda,
			                       image->sizes[0], image->sizes[1]);
			        }
				else if (image->dimension == 3)
				{
				         This_wiener3d(data_index1, result_index1, sigma, lambda,
			                       image->sizes[0], image->sizes[1], image->sizes[2]);
				}
				for (i = (storage_size/image->depth)-1; i >= 0; i--)
			        {
				         data_index -= image->depth;
					 result_index -= image->depth;
			                 result_index[k] = result_index1[i];
			        }
			}
			if (return_code)
			{
				DEALLOCATE(image->data);
				image->data = storage;
				image->valid = 1;
			}
			else
			{
				DEALLOCATE(storage);
			}
			DEALLOCATE(data_index1);
			DEALLOCATE(result_index1);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Image_cache_wiener_filter.  Not enough memory");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE, "Image_cache_wiener_filter.  "
			"Invalid arguments.");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Image_cache_wiener_filter */

static int Computed_field_wiener_filter_evaluate_cache_at_node(
	struct Computed_field *field, struct FE_node *node, FE_value time)
/*******************************************************************************
LAST MODIFIED : 6 January 2004

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int return_code;
	struct Computed_field_wiener_filter_type_specific_data *data;

	ENTER(Computed_field_wiener_filter_evaluate_cache_at_node);
	if (field && node &&
		(data = (struct Computed_field_wiener_filter_type_specific_data *)field->type_specific_data))
	{
		return_code = 1;
		/* 1. Precalculate the Image_cache */
		if (!data->image->valid)
		{
			return_code = Image_cache_update_from_fields(data->image, field->source_fields[0],
				field->source_fields[1],  data->region,
				data->graphics_buffer_package);
			/* 2. Perform image processing operation */
			return_code = Image_cache_wiener_filter(data->image, data->sigma, data->lambda);
		}
		/* 3. Evaluate texture coordinates and copy image to field */
		Computed_field_evaluate_cache_at_node(field->source_fields[1],
			node, time);
		Image_cache_evaluate_field(data->image,field);

	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_wiener_filter_evaluate_cache_at_node.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_wiener_filter_evaluate_cache_at_node */

static int Computed_field_wiener_filter_evaluate_cache_in_element(
	struct Computed_field *field, struct FE_element *element, FE_value *xi,
	FE_value time, struct FE_element *top_level_element,int calculate_derivatives)
/*******************************************************************************
LAST MODIFIED : 6 January 2004

DESCRIPTION :
Evaluate the fields cache at the node.
==============================================================================*/
{
	int return_code;
	struct Computed_field_wiener_filter_type_specific_data *data;

	ENTER(Computed_field_wiener_filter_evaluate_cache_in_element);
	USE_PARAMETER(calculate_derivatives);
	if (field && element && xi && (field->number_of_source_fields > 0) &&
		(field->number_of_components == field->source_fields[0]->number_of_components) &&
		(data = (struct Computed_field_wiener_filter_type_specific_data *) field->type_specific_data) &&
		data->image && (field->number_of_components == data->image->depth))
	{
		return_code = 1;
		/* 1. Precalculate the Image_cache */
		if (!data->image->valid)
		{
			return_code = Image_cache_update_from_fields(data->image, field->source_fields[0],
				field->source_fields[1],  data->region,
				data->graphics_buffer_package);
			/* 2. Perform image processing operation */
			return_code = Image_cache_wiener_filter(data->image, data->sigma, data->lambda);
		}
		/* 3. Evaluate texture coordinates and copy image to field */
		Computed_field_evaluate_cache_in_element(field->source_fields[1],
			element, xi, time, top_level_element, /*calculate_derivatives*/0);
		Image_cache_evaluate_field(data->image,field);

	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_wiener_filter_evaluate_cache_in_element.  "
			"Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_wiener_filter_evaluate_cache_in_element */

#define Computed_field_wiener_filter_evaluate_as_string_at_node \
	Computed_field_default_evaluate_as_string_at_node
/*******************************************************************************
LAST MODIFIED : 6 January 2004

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_wiener_filter_evaluate_as_string_in_element \
	Computed_field_default_evaluate_as_string_in_element
/*******************************************************************************
LAST MODIFIED : 6 January 2004

DESCRIPTION :
Print the values calculated in the cache.
==============================================================================*/

#define Computed_field_wiener_filter_set_values_at_node \
   (Computed_field_set_values_at_node_function)NULL
/*******************************************************************************
LAST MODIFIED : 6 January 2004

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_wiener_filter_set_values_in_element \
   (Computed_field_set_values_in_element_function)NULL
/*******************************************************************************
LAST MODIFIED : 6 January 2004

DESCRIPTION :
Not implemented yet.
==============================================================================*/

#define Computed_field_wiener_filter_get_native_discretization_in_element \
	Computed_field_default_get_native_discretization_in_element
/*******************************************************************************
LAST MODIFIED : 6 January 2004

DESCRIPTION :
Inherit result from first source field.
==============================================================================*/

#define Computed_field_wiener_filter_find_element_xi \
   (Computed_field_find_element_xi_function)NULL
/*******************************************************************************
LAST MODIFIED : 6 January 2004

DESCRIPTION :
Not implemented yet.
==============================================================================*/

int Computed_field_wiener_filter_get_native_resolution(struct Computed_field *field,
        int *dimension, int **sizes, 
	struct Computed_field **texture_coordinate_field)
/*******************************************************************************
LAST MODIFIED : 4 February 2005

DESCRIPTION :
Gets the <dimension>, <sizes>, <minimums>, <maximums> and <texture_coordinate_field> from
the <field>. These parameters will be used in image processing.

==============================================================================*/
{       
        int return_code;
	struct Computed_field_wiener_filter_type_specific_data *data;
	
	ENTER(Computed_field_wiener_filter_get_native_resolution);
	if (field && (data =
		(struct Computed_field_wiener_filter_type_specific_data *)
		field->type_specific_data) && data->image)
	{
	        return_code = 1;
		Image_cache_get_native_resolution(data->image,
			dimension, sizes);
		/* Texture_coordinate_field from source fields */
		if (*texture_coordinate_field)
		{
		        REACCESS(Computed_field)(&(*texture_coordinate_field), field->source_fields[0]); 
		}
		else
		{
		        *texture_coordinate_field = ACCESS(Computed_field)(field->source_fields[1]);
		}	 
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_wiener_filter_get_native_resolution.  Missing field");
		return_code=0;
	}

	return (return_code);
} /* Computed_field_wiener_filter_get_native_resolution */

static int list_Computed_field_wiener_filter(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 4 December 2003

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	struct Computed_field_wiener_filter_type_specific_data *data;

	ENTER(List_Computed_field_wiener_filter);
	if (field && (field->type_string==computed_field_wiener_filter_type_string)
		&& (data = (struct Computed_field_wiener_filter_type_specific_data *)
		field->type_specific_data))
	{
		display_message(INFORMATION_MESSAGE,
			"    source field : %s\n",field->source_fields[0]->name);
		display_message(INFORMATION_MESSAGE,
			"    texture coordinate field : %s\n",field->source_fields[1]->name);
		display_message(INFORMATION_MESSAGE,
			"    filter sigma : %f\n", data->sigma);
		display_message(INFORMATION_MESSAGE,
			"    filter lambda : %f\n", data->lambda);
		return_code = 1;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"list_Computed_field_wiener_filter.  Invalid field");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* list_Computed_field_wiener_filter */

static char *Computed_field_wiener_filter_get_command_string(
	struct Computed_field *field)
/*******************************************************************************
LAST MODIFIED : 4 December 2003

DESCRIPTION :
Returns allocated command string for reproducing field. Includes type.
==============================================================================*/
{
	char *command_string, *field_name, temp_string[40];
	int error;
	struct Computed_field_wiener_filter_type_specific_data *data;

	ENTER(Computed_field_wiener_filter_get_command_string);
	command_string = (char *)NULL;
	if (field&& (field->type_string==computed_field_wiener_filter_type_string)
		&& (data = (struct Computed_field_wiener_filter_type_specific_data *)
		field->type_specific_data) )
	{
		error = 0;
		append_string(&command_string,
			computed_field_wiener_filter_type_string, &error);
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
		sprintf(temp_string, " dimension %d", data->image->dimension);
		append_string(&command_string, temp_string, &error);

		sprintf(temp_string, " lambda %f", data->lambda);
		append_string(&command_string, temp_string, &error);

		sprintf(temp_string, " sigma %f", data->sigma);
		append_string(&command_string, temp_string, &error);

		sprintf(temp_string, " sizes %d %d",
		                    data->image->sizes[0],data->image->sizes[1]);
		append_string(&command_string, temp_string, &error);

	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_wiener_filter_get_command_string.  Invalid field");
	}
	LEAVE;

	return (command_string);
} /* Computed_field_wiener_filter_get_command_string */

#define Computed_field_wiener_filter_has_multiple_times \
	Computed_field_default_has_multiple_times
/*******************************************************************************
LAST MODIFIED : 4 December 2003

DESCRIPTION :
Works out whether time influences the field.
==============================================================================*/

int Computed_field_set_type_wiener_filter(struct Computed_field *field,
	struct Computed_field *source_field,
	struct Computed_field *texture_coordinate_field,
	double sigma, double lambda, int dimension, int *sizes, 
	struct MANAGER(Computed_field) *computed_field_manager,
	struct Cmiss_region *region, struct Graphics_buffer_package *graphics_buffer_package)
/*******************************************************************************
LAST MODIFIED : Mar 18 2004

DESCRIPTION :
Converts <field> to type COMPUTED_FIELD_wiener_filter with the supplied
fields, <source_field> and <texture_coordinate_field>.  The <sigma> specifies
half the width and height of the filter window.  The <dimension> is the
size of the <sizes>.
==============================================================================*/
{
	int depth, number_of_source_fields, return_code;
	struct Computed_field **source_fields;
	struct Computed_field_wiener_filter_type_specific_data *data;

	ENTER(Computed_field_set_type_wiener_filter);
	if (field && source_field && texture_coordinate_field && (lambda > 0) &&
		(sigma > 0) && (depth = source_field->number_of_components) &&
		(dimension <= texture_coordinate_field->number_of_components) &&
		region && graphics_buffer_package)
	{
		return_code=1;
		/* 1. make dynamic allocations for any new type-specific data */
		number_of_source_fields=2;
		data = (struct Computed_field_wiener_filter_type_specific_data *)NULL;
		if (ALLOCATE(source_fields, struct Computed_field *, number_of_source_fields) &&
			ALLOCATE(data, struct Computed_field_wiener_filter_type_specific_data, 1) &&
			(data->image = ACCESS(Image_cache)(CREATE(Image_cache)())) &&
			Image_cache_update_dimension(
			data->image, dimension, depth, sizes) &&
			Image_cache_update_data_storage(data->image))
		{
			/* 2. free current type-specific data */
			Computed_field_clear_type(field);
			/* 3. establish the new type */
			field->type_string = computed_field_wiener_filter_type_string;
			field->number_of_components = source_field->number_of_components;
			source_fields[0]=ACCESS(Computed_field)(source_field);
			source_fields[1]=ACCESS(Computed_field)(texture_coordinate_field);
			field->source_fields=source_fields;
			field->number_of_source_fields=number_of_source_fields;
			data->sigma = sigma;
			data->lambda = lambda;
			/*data->element_dimension = element_dimension;*/
			data->region = ACCESS(Cmiss_region)(region);
			data->graphics_buffer_package = graphics_buffer_package;
			data->computed_field_manager = computed_field_manager;
			data->computed_field_manager_callback_id =
				MANAGER_REGISTER(Computed_field)(
				Computed_field_wiener_filter_field_change, (void *)field,
				computed_field_manager);

			field->type_specific_data = data;

			/* Set all the methods */
			COMPUTED_FIELD_ESTABLISH_METHODS(wiener_filter);
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
			"Computed_field_set_type_wiener_filter.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_set_type_wiener_filter */

int Computed_field_get_type_wiener_filter(struct Computed_field *field,
	struct Computed_field **source_field,
	struct Computed_field **texture_coordinate_field,
	double *sigma, double *lambda, int *dimension, int **sizes)
/*******************************************************************************
LAST MODIFIED : 17 December 2003

DESCRIPTION :
If the field is of type COMPUTED_FIELD_wiener_filter, the
parameters defining it are returned.
==============================================================================*/
{
	int i, return_code;
	struct Computed_field_wiener_filter_type_specific_data *data;

	ENTER(Computed_field_get_type_wiener_filter);
	if (field && (field->type_string==computed_field_wiener_filter_type_string)
		&& (data = (struct Computed_field_wiener_filter_type_specific_data *)
		field->type_specific_data) && data->image)
	{
		*dimension = data->image->dimension;
		if (ALLOCATE(*sizes, int, *dimension))
		{
			*source_field = field->source_fields[0];
			*texture_coordinate_field = field->source_fields[1];
			*sigma = data->sigma;
			*lambda = data->lambda;
			for (i = 0 ; i < *dimension ; i++)
			{
				(*sizes)[i] = data->image->sizes[i];
			}
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Computed_field_get_type_wiener_filter.  Unable to allocate vectors.");
			return_code = 0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_get_type_wiener_filter.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_get_type_wiener_filter */

static int define_Computed_field_type_wiener_filter(struct Parse_state *state,
	void *field_void, void *computed_field_wiener_filter_package_void)
/*******************************************************************************
LAST MODIFIED : 4 December 2003

DESCRIPTION :
Converts <field> into type COMPUTED_FIELD_wiener_filter (if it is not
already) and allows its contents to be modified.
==============================================================================*/
{
	char *current_token;
	double sigma, lambda;
	int dimension, return_code, *sizes;
	struct Computed_field *field, *source_field, *texture_coordinate_field;
	struct Computed_field_wiener_filter_package
		*computed_field_wiener_filter_package;
	struct Option_table *option_table;
	struct Set_Computed_field_conditional_data set_source_field_data,
		set_texture_coordinate_field_data;

	ENTER(define_Computed_field_type_wiener_filter);
	if (state&&(field=(struct Computed_field *)field_void)&&
		(computed_field_wiener_filter_package=
		(struct Computed_field_wiener_filter_package *)
		computed_field_wiener_filter_package_void))
	{
		return_code=1;
		source_field = (struct Computed_field *)NULL;
		texture_coordinate_field = (struct Computed_field *)NULL;
		dimension = 0;
		sizes = (int *)NULL;
		sigma = 1.0;
		lambda = 1.0;
		/* field */
		set_source_field_data.computed_field_manager =
			computed_field_wiener_filter_package->computed_field_manager;
		set_source_field_data.conditional_function =
			Computed_field_has_numerical_components;
		set_source_field_data.conditional_function_user_data = (void *)NULL;
		/* texture_coordinate_field */
		set_texture_coordinate_field_data.computed_field_manager =
			computed_field_wiener_filter_package->computed_field_manager;
		set_texture_coordinate_field_data.conditional_function =
			Computed_field_has_numerical_components;
		set_texture_coordinate_field_data.conditional_function_user_data = (void *)NULL;

		if (computed_field_wiener_filter_type_string ==
			Computed_field_get_type_string(field))
		{
			return_code = Computed_field_get_type_wiener_filter(field,
				&source_field, &texture_coordinate_field, &sigma, &lambda,
				&dimension, &sizes);
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
				/* field */
				Option_table_add_Computed_field_conditional_entry(option_table,
					"field", &source_field, &set_source_field_data);
				/* lambda */
				Option_table_add_double_entry(option_table,
					"lambda", &lambda);
				/* sigma */
				Option_table_add_double_entry(option_table,
					"sigma", &sigma);
				/* sizes */
				Option_table_add_int_vector_entry(option_table,
					"sizes", sizes, &dimension);
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
						if (!(REALLOCATE(sizes, sizes, int, dimension)))
						{
							return_code = 0;
						}
					}
					DESTROY(Option_table)(&option_table);
				}
			}
			/*if (return_code && (dimension < 1))
			{
				display_message(ERROR_MESSAGE,
					"define_Computed_field_type_scale.  Must specify a dimension first.");
				return_code = 0;
			}*/
			/* parse the rest of the table */
			if (return_code&&state->current_token)
			{
				option_table = CREATE(Option_table)();
				/* field */
				Option_table_add_Computed_field_conditional_entry(option_table,
					"field", &source_field, &set_source_field_data);
				/* lambda */
				Option_table_add_double_entry(option_table,
					"lambda", &lambda);
				/* sigma */
				Option_table_add_double_entry(option_table,
					"sigma", &sigma);
				/* sizes */
				Option_table_add_int_vector_entry(option_table,
					"sizes", sizes, &dimension);
				/* texture_coordinate_field */
				Option_table_add_Computed_field_conditional_entry(option_table,
					"texture_coordinate_field", &texture_coordinate_field,
					&set_texture_coordinate_field_data);
				return_code=Option_table_multi_parse(option_table,state);
				DESTROY(Option_table)(&option_table);
			}
			if ((dimension < 1) && source_field)
			{
			        return_code = Computed_field_get_native_resolution(source_field,
				     &dimension,&sizes,&texture_coordinate_field);
			}
			/* no errors,not asking for help */
			if (return_code)
			{
				return_code = Computed_field_set_type_wiener_filter(field,
					source_field, texture_coordinate_field, sigma, lambda, dimension, sizes, 
					computed_field_wiener_filter_package->computed_field_manager,
					computed_field_wiener_filter_package->root_region,
					computed_field_wiener_filter_package->graphics_buffer_package);
			}
			if (!return_code)
			{
				if ((!state->current_token)||
					(strcmp(PARSER_HELP_STRING,state->current_token)&&
						strcmp(PARSER_RECURSIVE_HELP_STRING,state->current_token)))
				{
					/* error */
					display_message(ERROR_MESSAGE,
						"define_Computed_field_type_wiener_filter.  Failed");
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
			if (sizes)
			{
				DEALLOCATE(sizes);
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"define_Computed_field_type_wiener_filter.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* define_Computed_field_type_wiener_filter */

int Computed_field_register_types_wiener_filter(
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *root_region, struct Graphics_buffer_package *graphics_buffer_package)
/*******************************************************************************
LAST MODIFIED : 12 December 2003

DESCRIPTION :
==============================================================================*/
{
	int return_code;
	static struct Computed_field_wiener_filter_package
		computed_field_wiener_filter_package;

	ENTER(Computed_field_register_types_wiener_filter);
	if (computed_field_package)
	{
		computed_field_wiener_filter_package.computed_field_manager =
			Computed_field_package_get_computed_field_manager(
				computed_field_package);
		computed_field_wiener_filter_package.root_region = root_region;
		computed_field_wiener_filter_package.graphics_buffer_package = graphics_buffer_package;
		return_code = Computed_field_package_add_type(computed_field_package,
			            computed_field_wiener_filter_type_string,
			            define_Computed_field_type_wiener_filter,
			            &computed_field_wiener_filter_package);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Computed_field_register_types_wiener_filter.  Invalid argument(s)");
		return_code = 0;
	}
	LEAVE;

	return (return_code);
} /* Computed_field_register_types_wiener_filter */

