
/*******************************************************************************
       RawImageShell.h
       This header file is included by RawImageShell.c

*******************************************************************************/

#ifndef	_RAWIMAGESHELL_INCLUDED
#define	_RAWIMAGESHELL_INCLUDED


#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <Mrm/MrmPublic.h>
#include <Xm/MenuShell.h>
#include "UxXt.h"

#include <Xm/LabelG.h>
#include <Xm/Frame.h>
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
	Widget	Uxform2;
	Widget	UxRawImageScrolledWindow;
	Widget	Uxmenu12;
	Widget	Uxmenu12_p1_b1;
	Widget	Uxmenu12_p1_b2;
	Widget	Uxframe1;
	Widget	UxRawXSliceDA;
	Widget	Uxframe2;
	Widget	UxRawYSliceDA;
	Widget	Uxframe3;
	Widget	UxrowColumn6;
	Widget	UxRXLG;
	Widget	UxRYLG;
	Widget	UxRVLG;
	Widget	UxRawVsbar;
	Widget	UxRawHsbar;
} _UxCRawImageShell;

#ifdef CONTEXT_MACRO_ACCESS
static _UxCRawImageShell       *UxRawImageShellContext;
#define form2                   UxRawImageShellContext->Uxform2
#define RawImageScrolledWindow  UxRawImageShellContext->UxRawImageScrolledWindow
#define menu12                  UxRawImageShellContext->Uxmenu12
#define menu12_p1_b1            UxRawImageShellContext->Uxmenu12_p1_b1
#define menu12_p1_b2            UxRawImageShellContext->Uxmenu12_p1_b2
#define frame1                  UxRawImageShellContext->Uxframe1
#define RawXSliceDA             UxRawImageShellContext->UxRawXSliceDA
#define frame2                  UxRawImageShellContext->Uxframe2
#define RawYSliceDA             UxRawImageShellContext->UxRawYSliceDA
#define frame3                  UxRawImageShellContext->Uxframe3
#define rowColumn6              UxRawImageShellContext->UxrowColumn6
#define RXLG                    UxRawImageShellContext->UxRXLG
#define RYLG                    UxRawImageShellContext->UxRYLG
#define RVLG                    UxRawImageShellContext->UxRVLG
#define RawVsbar                UxRawImageShellContext->UxRawVsbar
#define RawHsbar                UxRawImageShellContext->UxRawHsbar

#endif /* CONTEXT_MACRO_ACCESS */

extern Widget	RawImageShell;
extern Widget	RawImageDrawingArea;

/*******************************************************************************
       Declarations of global functions.
*******************************************************************************/

Widget	create_RawImageShell( void );

#endif	/* _RAWIMAGESHELL_INCLUDED */
