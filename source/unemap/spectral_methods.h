/*******************************************************************************
FILE : spectral_methods.h

LAST MODIFIED : 8 March 1995

DESCRIPTION :
Prototypes of functions for analysing signals using spectral methods.
==============================================================================*/
#if !defined (SPECTRAL_METHODS_H)
#define SPECTRAL_METHODS_H

/*
Global types
------------
*/
enum Fourier_window_type
/*******************************************************************************
LAST MODIFIED : 7 March 1995

DESCRIPTION :
Specifies the data windowing to be used when calculating the Fourier transform.
==============================================================================*/
{
	HAMMING_WINDOW,
	PARZEN_WINDOW,
	SQUARE_WINDOW,
	WELCH_WINDOW
}; /* enum Data_window_type */

/*
Global functions
----------------
*/
int fourier_transform(enum Fourier_window_type window,
	struct Device *signal_real,struct Device *signal_imaginary,
	struct Device *transform_real,struct Device *transform_imaginary);
/*******************************************************************************
LAST MODIFIED : 6 September 1993

DESCRIPTION :
Calculates the Fourier transform of a complex signal.  If <signal_imaginary> is
NULL, only the non-negative half of the transform is calculated (the Fourier
transform of a real signal is even).  The real and imaginary parts of the signal
need to be stored in the same buffer  .The real and imaginary parts of the
transform need to be stored in the same buffer and they need to be the only
signals in the buffer.  Extra memory is allocated for the transform buffer if
required.
???DB.  Data windowing.
==============================================================================*/

int inverse_fourier_transform(struct Device *signal_real,
	struct Device *signal_imaginary,struct Device *transform_real,
	struct Device *transform_imaginary);
/*******************************************************************************
LAST MODIFIED : 8 March 1995

DESCRIPTION :
Calculates the inverse Fourier transform of a complex signal.  If
<transform_imaginary> is NULL, then the complex signal is assumed to be even
so that the inverse transform is real.  Extra memory is allocated for the
transform buffer if required.
==============================================================================*/
#endif
