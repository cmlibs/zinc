/***********************************************************************
*
*  Name:          imgproc_externs.h
*
*  Author:        Paul Charette
*
*  Last Modified: 26 Nov 1994
*
*  Purpose:       function prototyping
*
***********************************************************************/
#ifndef imgproc_externs_h
#define imgproc_externs_h

extern void Acos(double *inpixels, double *outpixels,
		 int height, int width, void *parms[]);
extern void Asin(double *inpixels, double *outpixels,
		 int height, int width, void *parms[]);
extern void Atan(double *inpixels, double *outpixels,
		 int height, int width, void *ParmPtrs[]);
extern void Atan2(double *inpixels, double *outpixels,
		  int height, int width, void *ParmPtrs[]);
extern void SpeckleAtan(double *inpixels, double *outpixels,
			int height, int width, void *ParmPtrs[]);
extern void Binarize(double *inpixels, double *outpixels,
		     int height, int width, void *ParmPtrs[]);
extern void BracketSD(double *inpixels, double *outpixels,
		      int height, int width, void *ParmPtrs[]);
extern void BandZero(double *inpixels, double *outpixels,
		     int height, int width, void *ParmPtrs[]);
extern void Center(double *inpixels, double *outpixels, int height, int width,
		   void *ParmPtrs[]);
extern void CharToDouble(char *pin, double *pout, int n);
extern void StackClear(double *inpixels, double *outpixels, int height, int width,
		       void *ParmPtrs[]);
extern void Cos(double *inpixels, double *outpixels, int height, int width,
		void *ParmPtrs[]);
extern void Crop(double *inpixels, double *outpixels, int height, int width,
		 void *ParmPtrs[]);
extern void Divide(double *inpixels, double *outpixels, int height, int width,
		   void *ParmPtrs[]);
extern void DoublesToBytesZoom(double *inpixels, char *outpixels,
			       int height, int width, int ZoomFactor);
extern void DynamicRange(double *inpixels, int n, double *max, double *min);
extern void Exp(double *inpixels, double *outpixels, int height, int width,
		void *ParmPtrs[]);
extern void FillEdges(double *inpixels, double *outpixels,
		      int height, int width, int len);
extern void FreeDynamicRange(double *inpixels, double *outpixels,
			     int height, int width, void *parms[],
			     double max, double min);
extern void FreezeDynamicRange(double *inpixels, double *outpixels,
			       int height, int width, void *parms[],
			       double max, double min);
extern void FixEdges(double *inpixels, double *outpixels,
		     int height, int width,
		     void *parms[], double max, double min);
extern void Histogram(double *pixels, int xs, int ys, int w, int h,
		      int xextent, double max, double min, int *hist,
		      XRectangle *XRects, double *meanp, double *SDp);
extern void Log(double *inpixels, double *outpixels, int height, int width,
		void *ParmPtrs[]);
extern void MaskOut(double *inpixels, double *outpixels, int height, int width,
		    void *ParmPtrs[]);
extern void Mix(double *inpixels, double *outpixels, int height, int width,
		void *ParmPtrs[]);
extern void MixEm(double *inpixels1, double *inpixels2, double *outpixels,
		  int height, int width, double s);
extern void MoveDoubles(double *pin, double *pout, int n);
extern void PopImage(double *inpixels, double *outpixels,
		     int height, int width, void *ParmPtrs[]);
extern void Normalize(double *inpixels, double *outpixels,
		      int height, int width, void *ParmPtrs[]);
extern void Plane(double *inpixels, double *outpixels, int height, int width,
	      void *ParmPtrs[]);
extern void PlaneFit(double *inpixels, double *outpixels,
		     int height, int width, void *ParmPtrs[]);
extern void PushImage(double *inpixels, double *outpixels,
		      int height, int width, void *ParmPtrs[]);
extern void RealtoX(double *inpixels, char *outpixels, int height, int width,
		    double max, double min, int maxscale, int minscale,
		    int ZoomFactor);
extern int  RegionTraceContour(double *pixels, double *buffer,
			      int height, int width,
			      int xs, int ys, double rc,
			      CONTOUR_ELEMENT *contour, int nmax);
extern void RPNAdd(double *inpixels, double *outpixels, int height, int width,
		   void *ParmPtrs[]);
extern void RPNAddConstant(double *inpixels, double *outpixels,
			   int height, int width, void *ParmPtrs[]);
extern void RPNModConstant(double *inpixels, double *outpixels, int height,
			   int width, void *ParmPtrs[]);
extern void RPNMultiply(double *inpixels, double *outpixels,
			int height, int width, void *ParmPtrs[]);
extern void RPNMultiplyConstant(double *inpixels, double *outpixels,
				int height, int width, void *ParmPtrs[]);
extern void RPNRint(double *inpixels, double *outpixels, int height, int width,
	     void *ParmPtrs[]);
extern void RPNSubtract(double *inpixels, double *outpixels,
			int height, int width, void *ParmPtrs[]);
extern void RPNXInverse(double *inpixels, double *outpixels,
			int height, int width, void *ParmPtrs[]);
extern void Second(double *inpixels, double *outpixels, int height, int width,
		   void *parms[], double max, double min);
extern void SetDynamicRange(double *inpixels, double *outpixels,
			    int height, int width, void *parms[],
			    double max, double min);
extern void SetPixelValue(double *inpixels, double *outpixels,
			  int height, int width, void *parms[]);
extern void Sharpen(double *inpixels, double *outpixels, int height, int width,
		    void *parms[]);
extern void ShiftImage(double *inpixels, double *outpixels, int height,
		int width, void *ParmPtrs[]);
extern void Sin(double *inpixels, double *outpixels,
		int height, int width, void *parms[]);
extern void Sqr(double *inpixels, double *outpixels,
		int height, int width, void *parms[]);
extern void Sqrt(double *inpixels, double *outpixels,
		 int height, int width, void *parms[]);
extern void StackExchangePos(double *inpixels, double *outpixels,
			     int height, int width, void *ParmPtrs[]);
extern void StackRotateDown(double *inpixels, double *outpixels,
			    int height, int width, void *ParmPtrs[]);
extern void StackRotateUp(double *inpixels, double *outpixels,
			  int height, int width, void *ParmPtrs[]);
extern void Stats(double *inpixels, double *outpixels, int height, int width,
		  void *parms[], double max, double min);
extern void SubtractMean(double *inpixels, double *outpixels,
			 int height, int width, void *ParmPtrs[]);
extern void SubtractMin(double *inpixels, double *outpixels,
			int height, int width, void *ParmPtrs[]);
extern void SubtractPixVal(double *inpixels, double *outpixels,
			   int height, int width, void *ParmPtrs[]);
extern void SwapXY(double *inpixels, double *outpixels,
		   int height, int width, void *ParmPtrs[]);
extern void Threshold(double *inpixels, double *outpixels,
		      int height, int width, void *ParmPtrs[]);
extern void ZeroCenter(double *inpixels, double *outpixels, int height,
		       int width, void *ParmPtrs[]);
#endif












