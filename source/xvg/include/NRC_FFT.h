/***********************************************************************
*
*  Name:          NRC_FFT.h
*
*  Author:        Paul Charette
*
*  Last Modified: 26 Nov 1997
*
*  Purpose:       function prototyping
*
***********************************************************************/
#ifndef NRC_FFT_externs_h
#define NRC_FFT_externs_h
extern NRC_FFT(double *RealPixels, double *ImPixels,
	       int height, int width, int isign);
extern void NRC_FFT_Init();
#endif
