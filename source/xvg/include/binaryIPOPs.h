/***********************************************************************
*
*  Name:          binaryIPOPs.h
*
*  Author:        Paul Charette
*
*  Last Modified: 26 Nov 1994
*
*  Purpose:       function prototyping
*
***********************************************************************/
#ifndef binaryIPOPs_h
#define binaryIPOPs_h
extern void OR(double *inpixels, double *outpixels,
	       int height, int width, void *ParmPtrs[]);
extern void Skeleton(double *inpixels, double *outpixels,
		     int height, int width, void *ParmPtrs[]);
#endif

