/***********************************************************************
*
*  Name:          grabIPOPs.h
*
*  Author:        Paul Charette
*
*  Last Modified: 26 Nov 1994
*
*  Purpose:       function prototyping
*
***********************************************************************/
#ifndef grabIPOPs_externs_h
#define grabIPOPs_externs_h
extern void DoubleGrabSubtract(double *inpixels, double *outpixels,
			int height, int width, void *ParmPtrs[]);
extern void GrabFrame(double *inpixels, double *outpixels,
	       int height, int width, void *ParmPtrs[]);
extern void GrabStep(double *inpixels, double *outpixels,
	      int height, int width, void *ParmPtrs[]);
extern void StepMinus(double *inpixels, double *outpixels,
	       int height, int width, void *ParmPtrs[]);
extern void StepPlus(double *inpixels, double *outpixels,
	      int height, int width, void *ParmPtrs[]);
#endif


