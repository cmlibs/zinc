/***********************************************************************
*
*  Name:          XvgSystem.h
*
*  Author:        Paul Charette
*
*  Last Modified: 3 March 1997
*
*  Purpose:       function prototyping
*
***********************************************************************/
#ifndef XvgSystem_h
#define XvgSystem_h

extern double XvgStartTimer(void);
extern double XvgStopTimer(void);
extern double XvgGetTime(void);
extern char *ElapsedTime(char *s);
extern void XvgTrapSignal(void);

#endif
