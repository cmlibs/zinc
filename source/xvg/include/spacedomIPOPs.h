/***********************************************************************
*
*  Name:          spacedomIPOPs_externs.h
*
*  Author:        Paul Charette
*
*  Last Modified: 26 Nov 1994
*
*  Purpose:       function prototyping
*
***********************************************************************/
#ifndef spacedomIPOPs_externs_h
#define spacedomIPOPs_externs_h

extern void AveragingFilter(double *inpixels, double *outpixels,
			    int height, int width, void *ParmPtrs[]);
extern void Correlate(double *inpixels, double *outpixels,
		      int height, int width, void *ParmPtrs[]);
extern void Detrend(double *inpixels, double *outpixels, int height, int width,
		    void *ParmPtrs[]);
extern void DetrendXYZ(double *inpixels, double *outpixels, int height, int width,
	        void *ParmPtrs[]);
extern void Gradient(double *inpixels, double *outpixels,
		     int height, int width, void *ParmPtrs[]);
extern void Laplacian(double *inpixels, double *outpixels, int height, int width,
	       void *ParmPtrs[]);
extern void MedianFilter(double *inpixels, double *outpixels,
			 int height, int width, void *ParmPtrs[]);
extern void MedianFilterByte(double *inpixels, double *outpixels,
			     int height, int width, void *ParmPtrs[]);
extern void RegionAveragingFilter(double *inpixels, double *outpixels,
				  int height, int width, void *ParmPtrs[]);
extern void WeightedAveragingFilter(double *inpixels, double *outpixels,
				    int height, int width, void *ParmPtrs[]);
extern void ZeroEdges(double *inpixels, double *outpixels,
		      int height, int width, void *parms[]);
#endif
