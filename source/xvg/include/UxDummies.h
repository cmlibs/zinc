/***********************************************************************
*
*  Name:          UxDummies.h
*
*  Author:        Paul Charette
*
*  Last Modified: 26 Nov 1994
*
*  Purpose:       function prototyping
*
***********************************************************************/
#ifndef UxDummies_h
#define UxDummies_h
extern void *UxCalloc(size_t NumberOfElements, size_t ElementSize);
extern void  UxFree(void *p);
extern char *UxGetDirectory(swidget sw);
extern int   UxGetHeight(swidget sw);
extern int   UxGetItemCount(swidget sw);
extern char *UxGetLabelString(swidget sw);
extern int   UxGetMaximum(swidget sw);
extern int   UxGetMinimum(swidget sw);
extern char *UxGetSet(swidget sw);
extern char *UxGetText(swidget sw);
extern int   UxGetValue(swidget sw);
extern int   UxGetWidth(swidget sw);
extern void  UxDelayUpdate(swidget sw);
extern void  UxMap(swidget sw);
extern void *UxMalloc(size_t size);
extern void  UxPutBottomOffset(swidget sw, int d);
extern void  UxPutLabelString(swidget sw, char *s);
extern void  UxPutHeight(swidget sw, int d);
extern void  UxPutMaxHeight(swidget sw, int d);
extern void  UxPutMaximum(swidget sw, int d);
extern void  UxPutMaxWidth(swidget sw, int d);
extern void  UxPutMessageString(swidget sw, char *s);
extern void  UxPutPattern(swidget sw, char *s);
extern void  UxPutRightOffset(swidget sw, int d);
extern void  UxPutSelectionLabelString(swidget sw, char *s);
extern void  UxPutSet(swidget sw, char *s);
extern void  UxPutText(swidget sw, char *s);
extern void  UxPutTextString(swidget sw, char *s);
extern void  UxPutTitle(swidget sw, char *s);
extern void  UxPutTitleString(swidget sw, char *s);
extern void  UxPutValue(swidget sw, int d);
extern void  UxPutWidth(swidget sw, int d);
extern void  UxPutX(swidget sw, int d);
extern void *UxRealloc(void *ptr, size_t size);
extern void  UxUnmap(swidget sw);
extern void  UxUpdate(swidget sw);
#endif


