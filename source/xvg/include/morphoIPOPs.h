/***********************************************************************
*
*  Name:          morphoIPOPs_externs.h
*
*  Author:        Paul Charette
*
*  Last Modified: 26 Nov 1994
*
*  Purpose:       function prototyping
*
***********************************************************************/
#ifndef morphoIPOPs_externs_h
#define morphoIPOPs_externs_h
extern void Contour(double *inpixels, double *outpixels,
		    int height, int width, void *ParmPtrs[]);
extern void Dilation(double *inpixels, double *outpixels,
		     int height, int width, void *ParmPtrs[]);
extern void DilationFFT(double *inpixels, double *outpixels,
			int height, int width, void *ParmPtrs[]);
extern void Disk(double *inpixels, double *outpixels, int height, int width,
		 void *ParmPtrs[]);
extern void Erosion(double *inpixels, double *outpixels, int height, int width,
		    void *ParmPtrs[]);
extern void ErosionFFT(double *inpixels, double *outpixels,
		       int height, int width, void *ParmPtrs[]);
#endif
