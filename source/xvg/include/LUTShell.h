/***********************************************************************
*
*  Name:          LUTShell.h
*
*  Author:        Paul Charette
*
*  Last Modified: 26 Nov 1994
*
*  Purpose:       function prototyping
*
***********************************************************************/
#ifndef LUTShell_h
#define LUTShell_h
extern swidget create_LUTShell(void);
extern void DrawColorBar(swidget DAW, boolean_t ShellCreatedFlag);
extern void UpdateColorLUT(void);
#endif

