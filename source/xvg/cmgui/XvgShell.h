
/*******************************************************************************
       XvgShell.h
       This header file is included by XvgShell.c

*******************************************************************************/

#ifndef	_XVGSHELL_INCLUDED
#define	_XVGSHELL_INCLUDED


#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <Mrm/MrmPublic.h>
#include <Xm/MenuShell.h>
#include "UxXt.h"

#include <Xm/Scale.h>
#include <Xm/PushBG.h>
#include <Xm/SeparatoG.h>
#include <Xm/ToggleBG.h>
#include <Xm/RowColumn.h>
#include <Xm/LabelG.h>
#include <Xm/Form.h>
#include <Xm/ScrolledW.h>
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
	Widget	UxscrolledWindow1;
	Widget	Uxform1;
	Widget	LogoDAW;
	Widget	UxlabelGadget1;
	Widget	UxrowColumn1;
	Widget	UxrowColumn4;
	Widget	UxseparatorGadget1;
	Widget	UxseparatorGadget9;
	Widget	UxlabelGadget2;
	Widget	UxrowColumn2;
	Widget	UxResize640x480PBG;
	Widget	UxLoadDataSetPBG;
	Widget	UxLoadIPOpsPBG;
	Widget	UxSaveHistogramPBG;
	Widget	UxSaveDataSetPBG;
	Widget	UxSaveIPOpsPBG;
	Widget	UxseparatorGadget2;
	Widget	UxlabelGadget3;
	Widget	UxrowColumn3;
	Widget	Uxmenu1_p4;
	Widget	Uxmenu1_p4_b1;
	Widget	Uxmenu1_p4_b8;
	Widget	Uxmenu1_p4_b16;
	Widget	Uxmenu1_p4_b17;
	Widget	Uxmenu1_p4_b6;
	Widget	Uxmenu1_p4_b7;
	Widget	Uxmenu1_p4_b5;
	Widget	Uxmenu1;
	Widget	Uxmenu1_p1;
	Widget	Uxmenu1_p1_b6;
	Widget	Uxmenu1_p1_b4;
	Widget	Uxmenu1_p1_b7;
	Widget	Uxmenu1_p1_b5;
	Widget	Uxmenu1_p1_b8;
	Widget	Uxmenu1_p1_b9;
	Widget	Uxmenu1_p1_b10;
	Widget	Uxmenu13;
	Widget	Uxmenu1_p3;
	Widget	Uxmenu1_p1_b1;
	Widget	Uxmenu1_p1_b12;
	int	Uxargc;
	char	**Uxargv;
} _UxCXvgShell;

#ifdef CONTEXT_MACRO_ACCESS
static _UxCXvgShell            *UxXvgShellContext;
#define scrolledWindow1         UxXvgShellContext->UxscrolledWindow1
#define form1                   UxXvgShellContext->Uxform1
#define LogoDAW                 UxXvgShellContext->LogoDAW
#define labelGadget1            UxXvgShellContext->UxlabelGadget1
#define rowColumn1              UxXvgShellContext->UxrowColumn1
#define rowColumn4              UxXvgShellContext->UxrowColumn4
#define separatorGadget1        UxXvgShellContext->UxseparatorGadget1
#define separatorGadget9        UxXvgShellContext->UxseparatorGadget9
#define labelGadget2            UxXvgShellContext->UxlabelGadget2
#define rowColumn2              UxXvgShellContext->UxrowColumn2
#define Resize640x480PBG        UxXvgShellContext->UxResize640x480PBG
#define LoadDataSetPBG          UxXvgShellContext->UxLoadDataSetPBG
#define LoadIPOpsPBG            UxXvgShellContext->UxLoadIPOpsPBG
#define SaveHistogramPBG        UxXvgShellContext->UxSaveHistogramPBG
#define SaveDataSetPBG          UxXvgShellContext->UxSaveDataSetPBG
#define SaveIPOpsPBG            UxXvgShellContext->UxSaveIPOpsPBG
#define separatorGadget2        UxXvgShellContext->UxseparatorGadget2
#define labelGadget3            UxXvgShellContext->UxlabelGadget3
#define rowColumn3              UxXvgShellContext->UxrowColumn3
#define menu1_p4                UxXvgShellContext->Uxmenu1_p4
#define menu1_p4_b1             UxXvgShellContext->Uxmenu1_p4_b1
#define menu1_p4_b8             UxXvgShellContext->Uxmenu1_p4_b8
#define menu1_p4_b16            UxXvgShellContext->Uxmenu1_p4_b16
#define menu1_p4_b17            UxXvgShellContext->Uxmenu1_p4_b17
#define menu1_p4_b6             UxXvgShellContext->Uxmenu1_p4_b6
#define menu1_p4_b7             UxXvgShellContext->Uxmenu1_p4_b7
#define menu1_p4_b5             UxXvgShellContext->Uxmenu1_p4_b5
#define menu1                   UxXvgShellContext->Uxmenu1
#define menu1_p1                UxXvgShellContext->Uxmenu1_p1
#define menu1_p1_b6             UxXvgShellContext->Uxmenu1_p1_b6
#define menu1_p1_b4             UxXvgShellContext->Uxmenu1_p1_b4
#define menu1_p1_b7             UxXvgShellContext->Uxmenu1_p1_b7
#define menu1_p1_b5             UxXvgShellContext->Uxmenu1_p1_b5
#define menu1_p1_b8             UxXvgShellContext->Uxmenu1_p1_b8
#define menu1_p1_b9             UxXvgShellContext->Uxmenu1_p1_b9
#define menu1_p1_b10            UxXvgShellContext->Uxmenu1_p1_b10
#define menu13                  UxXvgShellContext->Uxmenu13
#define menu1_p3                UxXvgShellContext->Uxmenu1_p3
#define menu1_p1_b1             UxXvgShellContext->Uxmenu1_p1_b1
#define menu1_p1_b12            UxXvgShellContext->Uxmenu1_p1_b12
#define argc                    UxXvgShellContext->Uxargc
#define argv                    UxXvgShellContext->Uxargv

#endif /* CONTEXT_MACRO_ACCESS */

extern Widget	XvgShell;
extern Widget	RawImageTBG;
extern Widget	ProcessedImageTBG;
extern Widget	IPOperationsTBG;
extern Widget	HistogramTBG;
extern Widget	XYSlicesTBG;
extern Widget	LUTShellTBG;
extern Widget	MovieControlsTBG;
extern Widget	FilenameLG;
extern Widget	SizeLG;
extern Widget	GammaScale;
extern Widget	LUTMenuOMW;

/*******************************************************************************
       Declarations of global functions.
*******************************************************************************/

Widget	create_XvgShell( int _Uxargc, char **_Uxargv );

#endif	/* _XVGSHELL_INCLUDED */
