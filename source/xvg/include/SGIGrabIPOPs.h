/***********************************************************************
*
*  Name:          SGIGrabIPOPs.h
*
*  Author:        Paul Charette
*
*  Last Modified: 1 March 1997
*
*  Purpose:       function prototyping
*
***********************************************************************/
#ifndef SGIGrabIPOPs_h
#define SGIGrabIPOPs_h
extern void NetworkGrab(double *inpixels, double *outpixels,
			int height, int width, void *ParmPtrs[]);
extern void FrameGrab(double *inpixels, double *outpixels,
		      int height, int width, void *ParmPtrs[]);
#endif
