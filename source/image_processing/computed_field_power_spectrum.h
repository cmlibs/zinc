/*******************************************************************************
FILE : computed_field_power_spectrum.h

LAST MODIFIED : 2 August 2004

DESCRIPTION :
Compute power spectrum on computed fields.
==============================================================================*/
#if !defined (COMPUTED_FIELD_POWER_SPECTRUM_H)
#define COMPUTED_FIELD_POWER_SPECTRUM_H

#define SWAP(a,b) tempr=(a);(a)=(b);(b)=tempr;
#define my_Min(x,y) ((x) <= (y) ? (x) : (y))
#define my_Max(x,y) ((x) <= (y) ? (y) : (x))

int FFT_1d(FE_value *Xr, FE_value *Xi, int dir, int data_size);
/****************************************************************************
      LAST MODIFIED: 2 August 2004

      DESCRIPTION: Implement 1D fast Fourier transform
============================================================================*/
int FFT_md(FE_value *in_re, FE_value *in_im, int dir, int dim, int *sizes);
/****************************************************************************
      LAST MODIFIED: 2 August 2004

      DESCRIPTION: Implement 2D fast Fourier transform
============================================================================*/

int Computed_field_register_types_power_spectrum(
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *root_region, struct Graphics_buffer_package *graphics_buffer_package);
/*******************************************************************************
LAST MODIFIED : 2 August 2004

DESCRIPTION :
==============================================================================*/

#endif /* !defined (COMPUTED_FIELD_POWER_SPECTRUM_H) */
