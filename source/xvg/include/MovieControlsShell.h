/***********************************************************************
*
*  Name:          MovieControlsShell.h
*
*  Author:        Paul Charette
*
*  Last Modified: 26 Nov 1994
*
*  Purpose:       function prototyping
*
***********************************************************************/
#ifndef MovieControlsShell_h
#define MovieControlsShell_h
extern swidget create_MovieControlsShell(void);
extern boolean_t LoadMovie(char *fname);
extern void LoadMovieFrame(void);
extern void LoopMovie(void);
#endif
