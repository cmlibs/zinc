/***********************************************************************
*
*  Name:          utils.h
*
*  Author:        Paul Charette
*
*  Last Modified: 3 March 1995
*
*  Purpose:       function prototyping
*
***********************************************************************/
#ifndef utils_h
#define utils_h
extern void ClearPPort(void);
extern void CreateAtanLut(double radius_threshold);
extern void CreateNoiseImage(double *inpixels, double *outpixels,
			int height, int width, void *ParmPtrs[]);
extern void CreateSpeckleImage(double *inpixels, double *outpixels,
			       int height, int width, void *ParmPtrs[]);
extern void FlushSPort(void);
extern void RandomDotStereogram(double *inpixels, double *outpixels,
			 int height, int width, void *ParmPtrs[]);
extern void RecordFrames(double *inpixels, double *outpixels,
			 int height,int width, void *ParmPtrs[]);
extern boolean_t SGICloseVideo(void);
extern boolean_t SGIGetVideoFBParms(void);
extern boolean_t SGILiveVideoOn(void);
extern boolean_t SGILoadFrame(void);
extern void sleep_us(int delay);
extern void VCAAInit(void);
extern boolean_t VCRFrameBackward(int nframes);
extern boolean_t VCRFrameForeward(int nframes);
extern boolean_t VCRInitVideo(void);
extern boolean_t VCRReset(void);
extern boolean_t VCRRecord(int nframes);
extern boolean_t XvgImgStack_Alloc(int height, int width);
extern void    XvgImgStack_Clear(void); 
extern void    XvgImgStack_Init(void);
extern boolean_t XvgImgStack_Pop(void);
extern boolean_t XvgImgStack_Push(double *pixels, int height, int width);
extern boolean_t XvgImgStack_Swap(double *pixels);
#endif






















