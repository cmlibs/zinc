/*******************************************************************************
FILE : computed_field_wiener_filter.h

LAST MODIFIED : 2 June 2004

DESCRIPTION :
Implements image Wiener filtering on computed fields.
==============================================================================*/
#if !defined (COMPUTED_FIELD_WIENER_FILTER_H)
#define COMPUTED_FIELD_WIENER_FILTER_H

#define SWAP(a,b) tempr=(a);(a)=(b);(b)=tempr;


int fft1d(FE_value *Xr, FE_value *Xi, FE_value *Yr, FE_value *Yi,
         char *inverse, int data_size);
/****************************************************************************
      LAST MODIFIED: 2 June 2004

      DESCRIPTION: Implement 1D fast Fourier transform
============================================================================*/
int fft2d(FE_value *in_re, FE_value *in_im, FE_value *out_re, FE_value *out_im,
         char *i_flag, int xsize, int ysize);
/****************************************************************************
      LAST MODIFIED: 2 June 2004

      DESCRIPTION: Implement 2D fast Fourier transform
============================================================================*/
void gaussian_kernel(FE_value *kernel,double g, int xsize, int ysize);
/***********************************************************************
   LAST MODIFIED: 1 June 2004

   DESCRIPTION: Build a Gaussian kernel with std g in Fourier domain
========================================================================*/
int wiener(FE_value *data_index, FE_value *result_index, double g, double lambda,
          int xsize, int ysize);
/************************************************************************
    LAST MODIFIED: 1 June 2004

    DESCRIPTION: Implement Wiener filtering
=========================================================================*/
int Computed_field_register_types_wiener_filter(
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *root_region, struct Graphics_buffer_package *graphics_buffer_package);
/*******************************************************************************
LAST MODIFIED : 18 March 2004

DESCRIPTION :
==============================================================================*/

#endif /* !defined (COMPUTED_FIELD_WIENER_FILTER_H) */
