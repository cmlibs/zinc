/***********************************************************************
*
*  Name:          wrappingIPOPs_externs.h
*
*  Author:        Paul Charette
*
*  Last Modified: 16 Nov 1994
*
*  Purpose:       function prototyping
*
***********************************************************************/
#ifndef wrappingIPOPs_externs_h
#define wrappingIPOPs_externs_h

extern void FixHook(double *inpixels, double *outpixels, int height, int width,
		    void *ParmPtrs[]);
extern void NonLinearLSPhaseFit(double *inpixels, double *outpixels,
				int height, int width, void *ParmPtrs[]);
extern void RegionBoundaryNormals(double *inpixels, double *outpixels,
				  int height, int width, void *ParmPtrs[]);
extern void RegionExpansion(double *inpixels, double *outpixels, int height,
			    int width, void *ParmPtrs[]);
extern int RegionFill(double *pixels, int height, int width, int ys, int xs,
		      double MarkVal, double NewVal, CONTOUR_ELEMENT *contour,
		      int *pcnt, int *CentroidX, int *CentroidY);
extern void RegionFillIPOp(double *inpixels, double *outpixels, int height,
			   int width, void *ParmPtrs[]);
extern void RegionFillSegmentation(double *inpixels, double *outpixels,
				   int height, int width, void *ParmPtrs[]);
extern void Residues(double *inpixels, double *outpixels,
		     int height, int width, void *ParmPtrs[]);
extern void Unwrap(double *inpixels, double *outpixels, int height,
		   int width, void *ParmPtrs[]);
extern void UnwrapManual(void);
extern void Wrap(double *inpixels, double *outpixels, int height,
		 int width, void *ParmPtrs[]);
#endif
