/***********************************************************************
*
*  Name:          RawImageShell.h
*
*  Author:        Paul Charette
*
*  Last Modified: 26 Nov 1994
*
*  Purpose:       function prototyping
*
***********************************************************************/
#ifndef RawImageShell_h
#define RawImageShell_h
extern swidget create_RawImageShell(void);
extern void HSBCallback(Widget w, XtPointer p1, XtPointer p2);
extern void RawExposeCB(Widget w, XtPointer p1, XtPointer p2);
extern void RawImageXYSlices(boolean_t ToggleFlag, boolean_t NewFlag);
extern void RawSetSizeTo640x480(void);
extern void UpdateRawWinParms(void);
extern void VSBCallback(Widget w, XtPointer p1, XtPointer p2);
#endif
