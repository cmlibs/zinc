/***********************************************************************
*
*  Name:          fitIPOPs.h
*
*  Author:        Paul Charette
*
*  Last Modified: 26 Nov 1994
*
*  Purpose:       function prototyping
*
***********************************************************************/
#ifndef fitIPOPs_externs_h
#define fitIPOPs_externs_h
extern void BSplines(double *inpixels, double *outpixels,
		     int height, int width, void *ParmPtrs[]);
extern void BSplinesAuto(double *inpixels, double *outpixels,
			 int height, int width, void *ParmPtrs[]);
extern void ChebyshevSurfaceFit(double *inpixels, double *outpixels,
				int height, int width, void *ParmPtrs[]);
extern void PolynomialFit(double *inpixels, double *outpixels,
			  int height, int width, void *ParmPtrs[]);
extern int PolynomialSurfaceFit(double *inpixels, double *outpixels,
			 int height, int width, int order, double *p);
extern void PowerSeriesModelEval(double *inpixels, double *outpixels,
				 int height, int width, void *ParmPtrs[]);
extern void SpeckleAutoCorrelationFit(double *inpixels, double *outpixels,
				      int height, int width, void *ParmPtrs[]);
extern void SpeckleAutoCorrelationTest(double *inpixels, double *outpixels,
				      int height, int width, void *ParmPtrs[]);
#endif

