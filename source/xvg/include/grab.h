/***********************************************************************
*
*  Name:          grab.h
*
*  Author:        Paul Charette
*
*  Last Modified: 26 Nov 1994
*
*  Purpose:       function prototyping
*
***********************************************************************/
#ifndef grab_externs_h
#define grab_externs_h
extern void    DisplayOverlay(char *s0, char *s1, char *s2, char *s3);
extern void    GalvoOpticalDelay(int NewVal);
extern void    Grab(double *pixels, unsigned short *sbufs,
		    boolean_t NewFrame, boolean_t AtanConvert);
extern void    OverlayString(char *s, int x, int y);
extern boolean_t VCAWriteData(XImage *image, XColor *Cells);
extern void    VCAWriteGLImage(int *data, int xextent, int yextent);
#endif
