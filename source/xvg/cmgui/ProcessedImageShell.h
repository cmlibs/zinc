
/*******************************************************************************
       ProcessedImageShell.h
       This header file is included by ProcessedImageShell.c

*******************************************************************************/

#ifndef	_PROCESSEDIMAGESHELL_INCLUDED
#define	_PROCESSEDIMAGESHELL_INCLUDED


#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <Mrm/MrmPublic.h>
#include <Xm/MenuShell.h>
#include "UxXt.h"

#include <Xm/LabelG.h>
#include <Xm/Frame.h>
#include <Xm/SeparatoG.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/DrawingA.h>
#include <Xm/ScrolledW.h>
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
	Widget	Uxform9;
	Widget	UxProcessedImageScrolledWindow;
	Widget	Uxmenu4;
	Widget	Uxmenu4_p1_b1;
	Widget	Uxmenu4_p1_b10;
	Widget	Uxmenu4_p1_b13;
	Widget	Uxmenu4_p1_b12;
	Widget	Uxmenu4_p1_b14;
	Widget	Uxmenu4_p1_b9;
	Widget	Uxmenu4_p1_b2;
	Widget	Uxmenu4_p1_b3;
	Widget	Uxmenu4_p1_b5;
	Widget	Uxmenu4_p1_b4;
	Widget	Uxmenu4_p1_b7;
	Widget	Uxmenu4_p1_b6;
	Widget	Uxmenu4_p1_b8;
	Widget	Uxmenu4_p1_b25;
	Widget	Uxmenu4_p1_b16;
	Widget	Uxmenu4_p1_b15;
	Widget	Uxframe7;
	Widget	UxProcessedXSliceDA;
	Widget	Uxframe8;
	Widget	UxProcessedYSliceDA;
	Widget	Uxframe9;
	Widget	UxrowColumn8;
	Widget	UxPXLG;
	Widget	UxPYLG;
	Widget	UxPVLG;
	Widget	UxProcessedVsbar;
	Widget	UxProcessedHsbar;
} _UxCProcessedImageShell;

#ifdef CONTEXT_MACRO_ACCESS
static _UxCProcessedImageShell *UxProcessedImageShellContext;
#define form9                   UxProcessedImageShellContext->Uxform9
#define ProcessedImageScrolledWindow UxProcessedImageShellContext->UxProcessedImageScrolledWindow
#define menu4                   UxProcessedImageShellContext->Uxmenu4
#define menu4_p1_b1             UxProcessedImageShellContext->Uxmenu4_p1_b1
#define menu4_p1_b2             UxProcessedImageShellContext->Uxmenu4_p1_b2
#define menu4_p1_b3             UxProcessedImageShellContext->Uxmenu4_p1_b3
#define menu4_p1_b4             UxProcessedImageShellContext->Uxmenu4_p1_b4
#define menu4_p1_b5             UxProcessedImageShellContext->Uxmenu4_p1_b5
#define menu4_p1_b10            UxProcessedImageShellContext->Uxmenu4_p1_b10
#define menu4_p1_b13            UxProcessedImageShellContext->Uxmenu4_p1_b13
#define menu4_p1_b12            UxProcessedImageShellContext->Uxmenu4_p1_b12
#define menu4_p1_b14            UxProcessedImageShellContext->Uxmenu4_p1_b14
#define menu4_p1_b9             UxProcessedImageShellContext->Uxmenu4_p1_b9
#define menu4_p1_b7             UxProcessedImageShellContext->Uxmenu4_p1_b7
#define menu4_p1_b6             UxProcessedImageShellContext->Uxmenu4_p1_b6
#define menu4_p1_b8             UxProcessedImageShellContext->Uxmenu4_p1_b8
#define menu4_p1_b25            UxProcessedImageShellContext->Uxmenu4_p1_b25
#define menu4_p1_b16            UxProcessedImageShellContext->Uxmenu4_p1_b16
#define menu4_p1_b15            UxProcessedImageShellContext->Uxmenu4_p1_b15
#define frame7                  UxProcessedImageShellContext->Uxframe7
#define ProcessedXSliceDA       UxProcessedImageShellContext->UxProcessedXSliceDA
#define frame8                  UxProcessedImageShellContext->Uxframe8
#define ProcessedYSliceDA       UxProcessedImageShellContext->UxProcessedYSliceDA
#define frame9                  UxProcessedImageShellContext->Uxframe9
#define rowColumn8              UxProcessedImageShellContext->UxrowColumn8
#define PXLG                    UxProcessedImageShellContext->UxPXLG
#define PYLG                    UxProcessedImageShellContext->UxPYLG
#define PVLG                    UxProcessedImageShellContext->UxPVLG
#define ProcessedVsbar          UxProcessedImageShellContext->UxProcessedVsbar
#define ProcessedHsbar          UxProcessedImageShellContext->UxProcessedHsbar

#endif /* CONTEXT_MACRO_ACCESS */

extern Widget	ProcessedImageShell;
extern Widget	ProcessedImageDrawingArea;

/*******************************************************************************
       Declarations of global functions.
*******************************************************************************/

Widget	create_ProcessedImageShell( void );

#endif	/* _PROCESSEDIMAGESHELL_INCLUDED */
