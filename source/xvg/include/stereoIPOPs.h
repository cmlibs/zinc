/***********************************************************************
*
*  Name:          stereoIPOPs.h
*
*  Author:        Paul Charette
*
*  Last Modified: 26 Nov 1996
*
*  Purpose:       function prototyping
*
***********************************************************************/
#ifndef stereoIPOPs_externs_h
#define stereoIPOPs_externs_h
#define PROCESSORS_NMAX  16                  /* maximum number of processors */
extern void GlobalPlaneFit(double *inpixels, double *outpixels,
			   int height, int width, void *ParmPtrs[]);
extern void InitStereoParameters(void);
extern void RemoveSemaphores(void);
extern void StopChildren(void);
extern void OpticalFlowLoadNodes(double *inpixels, double *outpixels,
				 int height, int width, void *ParmPtrs[]);
extern void MeanUVW(double *inpixels, double *outpixels, int height, int width,
		    void *ParmPtrs[]);
extern void OpticalFlow(double *inpixels, double *outpixels,
			int height, int width, void *ParmPtrs[]);
extern void OpticalFlowFieldFilter(double *inpixels, double *outpixels,
				   int height, int width, void *ParmPtrs[]);
extern void OpticalFlowLoadNodes(double *inpixels, double *outpixels,
				 int height, int width, void *ParmPtrs[]);
extern void OpticalFlowMakeMovie(double *inpixels, double *outpixels,
				 int height, int width, void *ParmPtrs[]);
extern void OpticalFlowNodeCMFilter(double *inpixels, double *outpixels,
				    int height, int width, void *ParmPtrs[]);
extern void PatchXCorr(double *inpixels, double *outpixels,
		       int height, int width, void *ParmPtrs[]);
extern void PatchXCorrByLine(double *inpixels, double *outpixels,
			     int height, int width, void *ParmPtrs[]);
extern void PatchXCorrByHorizSliceParallel(double *inpixels, double *outpixels,
					   int height, int width,
					   void *ParmPtrs[]);
extern void PatchXCorrByAreaParallel(double *inpixels, double *outpixels,
				     int height, int width, void *ParmPtrs[]);
extern void PatchXCorrByAreaHierarchical(double *inpixels, double *outpixels,
			      int height, int width, void *ParmPtrs[]);
extern void SequenceTrackPoints(double *inpixels, double *outpixels,
		                int height, int width, void *ParmPtrs[]);
extern void InterpolateSequence(double *inpixels, double *outpixels,
		                int height, int width, void *ParmPtrs[]);
extern void ConvertExNodeFileToMatlab(double *inpixels, double *outpixels,
			                          int height, int width, void *ParmPtrs[]);
#endif
