/***********************************************************************
*
*  Name:          XWDUtils.h
*
*  Author:        Paul Charette
*
*  Last Modified: 26 Nov 1994
*
*  Purpose:       function prototyping
*
***********************************************************************/
#ifndef XWDUtils_h
#define XWDUtils_h
extern boolean_t GrabWindow(double **pixels, int *heightp, int *widthp,
			  Colormap WinCMap, XColor *Cells, boolean_t verbose);
#endif
