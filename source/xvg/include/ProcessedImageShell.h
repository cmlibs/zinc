/***********************************************************************
*
*  Name:          ProcessedImageShell.h
*
*  Author:        Paul Charette
*
*  Last Modified: 23 Jan 1995
*
*  Purpose:       function prototyping
*
***********************************************************************/
#ifndef ProcessedImageShell_h
#define ProcessedImageShell_h
extern swidget create_ProcessedImageShell(void);
extern void PHSBCallback(Widget w, XtPointer p1, XtPointer p2);
extern void ProcessedImageXYSlices(boolean_t ToggleFlag, boolean_t NewFlag);
extern void ProcExposeCB(Widget w, XtPointer p1, XtPointer p2);
extern void ProcSetSizeTo640x480(void);
extern void PVSBCallback(Widget w, XtPointer p1, XtPointer p2);
extern void RedrawProcDrawingArea(void);
extern void UpdateProcessedWinParms(void);
#endif
