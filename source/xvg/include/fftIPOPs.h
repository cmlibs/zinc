/***********************************************************************
*
*  Name:          fftIPOPs.h
*
*  Author:        Paul Charette
*
*  Last Modified: 26 Nov 1994
*
*  Purpose:       function prototyping
*
***********************************************************************/
#ifndef fftIPOPs_h
#define fftIPOPs_h
extern void AutoCorrelation(double *inpixels, double *outpixels,
			    int height, int width, void *ParmPtrs[]);
extern void ClearFFTState(void);
extern void CrossCorrelation(double *inpixels, double *outpixels,
			     int height, int width, void *ParmPtrs[]);
extern void CustomFilter(double *inpixels, double *outpixels,
			 int height, int width, void *ParmPtrs[]);
extern void DiskCorrelation(double *inpixels, double *outpixels,
			    int height, int width, void *ParmPtrs[]);
extern void FFT(double *inpixels, double *outpixels,
		int height, int width, void *ParmPtrs[]);
extern void FFTShift(double *inpixels, double *outpixels,
		     int height, int width);
extern void FFTApplyWindow(double *inpixels, double *RealBuffer,
			   double *ImBuffer);
extern void FFTApplyInverseWindow(double *RealBuffer, double *outpixels);
extern boolean_t FFTInit(int height, int width);
extern boolean_t FFTInit2(int height, int width);
extern void FFTInverse(double *inpixels, double *outpixels,
		       int height, int width, void *ParmPtrs[]);
extern void HighpassFilter(double *inpixels, double *outpixels,
			   int height, int width, void *ParmPtrs[]);
extern void InterpolateFFT(double *inpixels, double *outpixels,
			   int height, int width, void *ParmPtrs[]);
extern void LowpassFilter(double *inpixels, double *outpixels,
			  int height, int width, void *ParmPtrs[]);
extern void NotchFilter(double *inpixels, double *outpixels,
		 int height, int width, void *ParmPtrs[]);
extern void ShiftImageFFT(double *inpixels, double *outpixels,
			  int height, int width, void *ParmPtrs[]);
extern void WaveletTransform(double *inpixels, double *outpixels,
			     int height, int width, void *ParmPtrs[]);
extern void ApplyWindow(double *inpixels, double *outpixels,
			int height, int width, void *ParmPtrs[]);
extern void ScaleImageFFT(double *inpixels, double *outpixels,
		   int height, int width, void *ParmPtrs[]);
#endif
