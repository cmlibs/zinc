
/*******************************************************************************
       MovieControlsShell.h
       This header file is included by MovieControlsShell.c

*******************************************************************************/

#ifndef	_MOVIECONTROLSSHELL_INCLUDED
#define	_MOVIECONTROLSSHELL_INCLUDED


#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <Mrm/MrmPublic.h>
#include <Xm/MenuShell.h>
#include "UxXt.h"

#include <Xm/Text.h>
#include <Xm/LabelG.h>
#include <Xm/SeparatoG.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/Form.h>
#include <X11/Shell.h>

/*******************************************************************************
       The definition of the context structure:
       If you create multiple copies of your interface, the context
       structure ensures that your callbacks use the variables for the
       correct copy.

       For each swidget in the interface, each argument to the Interface
       function, and each variable in the Interface Specific section of the
       Declarations Editor, there is an entry in the context structure.
       and a #define.  The #define makes the variable name refer to the
       corresponding entry in the context structure.
*******************************************************************************/

typedef	struct
{
	Widget	Uxform5;
	Widget	UxrowColumn9;
	Widget	UxLoopPBG;
	Widget	UxResetPBG;
	Widget	UxBackwardPBG;
	Widget	UxForwardPBG;
	Widget	UxIndexPBG;
	Widget	UxLoadMFPBG;
	Widget	Uxmenu3;
	Widget	Uxmenu3_p2;
	Widget	Uxmenu3_p2_b2;
	Widget	Uxmenu3_p2_b5;
	Widget	Uxmenu3_p2_b1;
	Widget	Uxmenu6;
	Widget	Uxmenu3_p3;
	Widget	Uxmenu3_p2_b3;
	Widget	Uxmenu3_p2_b4;
	Widget	UxseparatorGadget19;
	Widget	UxLoadXMoviePBG;
	Widget	UxLoadGLMoviePBG;
	Widget	UxseparatorGadget21;
	Widget	UxlabelGadget134;
} _UxCMovieControlsShell;

#ifdef CONTEXT_MACRO_ACCESS
static _UxCMovieControlsShell  *UxMovieControlsShellContext;
#define form5                   UxMovieControlsShellContext->Uxform5
#define rowColumn9              UxMovieControlsShellContext->UxrowColumn9
#define LoopPBG                 UxMovieControlsShellContext->UxLoopPBG
#define ResetPBG                UxMovieControlsShellContext->UxResetPBG
#define BackwardPBG             UxMovieControlsShellContext->UxBackwardPBG
#define ForwardPBG              UxMovieControlsShellContext->UxForwardPBG
#define IndexPBG                UxMovieControlsShellContext->UxIndexPBG
#define LoadMFPBG               UxMovieControlsShellContext->UxLoadMFPBG
#define menu3                   UxMovieControlsShellContext->Uxmenu3
#define menu3_p2                UxMovieControlsShellContext->Uxmenu3_p2
#define menu3_p2_b2             UxMovieControlsShellContext->Uxmenu3_p2_b2
#define menu3_p2_b5             UxMovieControlsShellContext->Uxmenu3_p2_b5
#define menu3_p2_b1             UxMovieControlsShellContext->Uxmenu3_p2_b1
#define menu6                   UxMovieControlsShellContext->Uxmenu6
#define menu3_p3                UxMovieControlsShellContext->Uxmenu3_p3
#define menu3_p2_b3             UxMovieControlsShellContext->Uxmenu3_p2_b3
#define menu3_p2_b4             UxMovieControlsShellContext->Uxmenu3_p2_b4
#define separatorGadget19       UxMovieControlsShellContext->UxseparatorGadget19
#define LoadXMoviePBG           UxMovieControlsShellContext->UxLoadXMoviePBG
#define LoadGLMoviePBG          UxMovieControlsShellContext->UxLoadGLMoviePBG
#define separatorGadget21       UxMovieControlsShellContext->UxseparatorGadget21
#define labelGadget134          UxMovieControlsShellContext->UxlabelGadget134

#endif /* CONTEXT_MACRO_ACCESS */

extern Widget	MovieControlsShell;
extern Widget	MovieNameLG;
extern Widget	MovieDelayTW;

/*******************************************************************************
       Declarations of global functions.
*******************************************************************************/

Widget	create_MovieControlsShell( void );

#endif	/* _MOVIECONTROLSSHELL_INCLUDED */
