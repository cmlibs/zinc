/***********************************************************************
*
*  Name:          GLUtils.h
*
*  Author:        Paul Charette
*
*  Last Modified: 26 Nov 1994
*
*  Purpose:       function prototyping
*
***********************************************************************/
#ifndef GLUtils_h
#define GLUtils_h
extern void    Build3DConeObject(double *inpixels, double *outpixels,
				   int height, int width, void *ParmPtrs[]);
extern void    Build3DHemisphere(double *inpixels, double *outpixels,
		       int height, int width, void *ParmPtrs[]);
extern void    Build3DPlaneObject(double *inpixels, double *outpixels,
				   int height, int width, void *ParmPtrs[]);
extern void    BuildNurbsObject(double MMPerPixel);
extern void    GLBuildNurbs(int PixelTolerance, int X, int Y, int Z,
			    char *startvals, char *incvals,int frames, 
			    char *SpecularS, char *DiffuseS, char *ShininessS,
			    double PixelsPerMM);
extern void    GLBuildPoly(int decimation,int X, int Y, int Z, int Zoom,
			   char *startvals, char *incvals, int frames, 
			   char *SpecularS, char *DiffuseS, char *ShininessS,
			   double PixelsPerMM);
extern void    GLDraw(int decimation, int X, int Y, int Z, int Zoom,
		      char *startvals, char *incvals,int frames, 
		      char *SpecularS, char *DiffuseS, char *ShininessS,
		      double PixelsPerMM);
extern void    GLMovieGenerate(int decimation, int X, int Y, int Z, int Zoom,
			       char *startvals, char *incvals, int frames, 
			       char *SpecularS, char *DiffuseS,
			       char *ShininessS, double PixelsPerMM,
			       char *fname);
extern void    GLMovieSeqGenerate(int decimation,
				  int X, int Y, int Z, int Zoom,
				  char *startvals, char *incvals, int frames, 
				  char *SpecularS, char *DiffuseS,
				  char *ShininessS, double PixelsPerMM);
extern void    GLReadMovie(char *fname);
extern boolean_t GLRecFrame(int decimation, int X, int Y, int Z, int Zoom,
			  char *startvals, char *incvals, int frames, 
			  char *SpecularS, char *DiffuseS, char *ShininessS,
			  double PixelsPerMM);
extern void    GLResetMovie(int decimation, int X, int Y, int Z,
			    char *startvals, char *incvals, int frames, 
			    char *SpecularS, char *DiffuseS, char *ShininessS,
			    double PixelsPerMM);
extern void    GLSetObjectType(int t);
extern void    GLShowFrame(int n);
extern void    GLWriteMovie(char *fname);
extern void    GL_SGI_OpenWindow(void);
extern void    InitXVGGLGaphics(void);
extern void    LoadGLField(void);
extern void    LoadGLGeometry(double);
extern void    SaveGLFrame(char *fname);
extern void    WriteGLImageToVCAA(void);
#endif

