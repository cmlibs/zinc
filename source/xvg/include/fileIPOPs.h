/***********************************************************************
*
*  Name:          fileIPOPs.h
*
*  Author:        Paul Charette
*
*  Last Modified: 26 Nov 1994
*
*  Purpose:       function prototyping
*
***********************************************************************/
#ifndef fileIPOPs_externs_h
#define fileIPOPs_externs_h
extern void LoadArctanErrorImg(double *inpixels, double *outpixels,
			       int height, int width, void *ParmPtrs[]);
extern void LoadArctanRadiusImg(double *inpixels, double *outpixels,
				int height, int width, void *ParmPtrs[]);
extern void LoadCosImg(double *inpixels, double *outpixels,
		       int height, int width, void *ParmPtrs[]);
extern void LoadImageFile(double *inpixels, double *outpixels,
			  int height, int width, void *ParmPtrs[]);
extern void LoadMask(double *inpixels, double *outpixels,
		     int height, int width, void *ParmPtrs[]);
extern void LoadMaskFile(double *inpixels, double *outpixels,
			 int height, int width, void *ParmPtrs[]);
extern void LoadSinImg(double *inpixels, double *outpixels,
		       int height, int width, void *ParmPtrs[]);
extern void Rotate(double *pixels, int height, int width);
extern void SaveBSpline(double *inpixels, double *outpixels,
			int height, int width, void *ParmPtrs[]);
extern void SaveDataSet(double *inpixels, double *outpixels,
			int height, int width, void *ParmPtrs[]);
extern void SaveDataSetByte(double *inpixels, double *outpixels,
			    int height, int width, void *ParmPtrs[]);
extern void SaveDataSetShort(double *inpixels, double *outpixels,
			     int height, int width, void *ParmPtrs[]);
extern void SaveDataSetReal(double *inpixels, double *outpixels,
			    int height, int width, void *ParmPtrs[]);
extern void SaveTIFFGreyLevelFile(double *inpixels, double *outpixels,
				  int height, int width, void *ParmPtrs[]);
extern void SaveTIFFGreyLevelFixedByteDRFile(double *inpixels,
					     double *outpixels,
					     int height, int width,
					     void *ParmPtrs[]);
extern void SaveTIFFColorFile(double *inpixels, double *outpixels,
			      int height, int width, void *ParmPtrs[]);
extern void Transpose(double *pixels, int height, int width);
#endif


