/*******************************************************************************
FILE : computed_field_wavelet_decomp.h

LAST MODIFIED : 3 March 2004

DESCRIPTION :
Implements image wavelet decomposition on computed fields.
==============================================================================*/
#if !defined (COMPUTED_FIELD_WAVELET_DECOMP_H)
#define COMPUTED_FIELD_WAVELET_DECOMP_H

int Computed_field_register_types_wavelet_decomp(
	struct Computed_field_package *computed_field_package,
	struct Cmiss_region *root_region, struct Graphics_buffer_package *graphics_buffer_package);
/*******************************************************************************
LAST MODIFIED : 4 March 2004

DESCRIPTION :
==============================================================================*/

#endif /* !defined (COMPUTED_FIELD_WAVELET_DECOMP_H) */
