/***********************************************************************
*
*  Name:          fileio.h
*
*  Author:        Paul Charette
*
*  Last Modified: 26 Jan 1995
*
*  Purpose:       function prototyping
*
***********************************************************************/
#ifndef fileio_externs_h
#define fileio_externs_h
extern boolean_t CompWriteMatlabImages(char *fname_mant,
				     unsigned short *sbufs, char *suffix);
extern boolean_t DeCompReadMatlabImages(int *height, int *width,
				      double *min, double *max,
				      double **pixels,
				      char *fname, boolean_t verbose);
extern boolean_t GetArrayFromMatlabFile(double **pixels,
					int *heightp, int *widthp,
					char *fname, char *vname);
extern boolean_t GetBSplineFile(char *fname, int *heightp, int *widthp,
				double *min, double *max, double **pixels,
				boolean_t verbose);
extern boolean_t GetDataFile(int *heightp, int *widthp, double *min,
			     double *max,
			     double **pixels, char *fname, boolean_t verbose);
extern boolean_t GetMatlabFile(int *heightp, int *widthp,
			       double *min, double *max,
			       double **pixels, char *fname,
			       boolean_t verbose);
extern boolean_t GetScanImage(char *fname, int *heightp, int *widthp,
			      double *min, double *max, double **pixels,
			      boolean_t verbose);
extern boolean_t GetSGIRGBFile(char *fname, double **pixels,
			       int *heightp, int *widthp,
			       double *min, double *max);
extern boolean_t GetFieldFromSGIRGBFile(char *fname, int field, double **pixels,
			                int *heightp, int *widthp,
			                double *min, double *max);
extern boolean_t GetTiffFile(char *fname, double **pixels,
			     int *heightp, int *widthp,
			     double *min, double *max,
			     Colormap cmap, XColor *Cells,
			     boolean_t *TrueColorFlag, boolean_t verbose);
extern char   *LoadPxData(char *fname, Colormap cmap, int background,
			  int AllocFlag, XColor Cells[],
			  int *width, int *height);
extern boolean_t LoadXWDFile(char *fname, double **pixels,
			     int *heightp, int *widthp,
			     Colormap cmap, XColor *Cells, boolean_t verbose);
extern void    PeekImage(char *fname, int *height, int *width, int *ConvItems);
extern boolean_t WriteArrayToMatlabFile(int height, int width, char *vname,
					double *pixels, char *fname);
extern boolean_t WriteGLGrayLevelTiffFile(char *fname, int width, int height,
					  int *pixels, char *buffer);
extern boolean_t WriteGLTiffFile(char *fname, int width, int height,
				 int *pixels, int *buffer);
extern boolean_t WriteIGSFile(int height, int width, double *pixels,
			      char *fname);
extern boolean_t WriteMatlabFile(int height, int width, char *vname,
				 double *pixels, char *fname);
extern boolean_t WriteMatlabUShortFile(int height, int width, char *vname,
				       double *pixels, char *fname);
extern void WriteMatlabUShortField(double *inpixels, double *outpixels,
                            int height, int width, void *parms[]);
extern boolean_t WritePxFile(int height, int width, double *pixels,
			     char *fname, int ncolors);
extern boolean_t WriteTiffFileColor(XImage *image, int xstart, int ystart,
				    int height, int width, char *fname,
				    char *info, XColor *Cells, int dfactor);
extern boolean_t WriteTiffFileGreyLevel(double *pixels, int xstart, int ystart,
					int height, int width, char *fname,
					char *info,
					boolean_t FixedDynamicRange);
#endif



