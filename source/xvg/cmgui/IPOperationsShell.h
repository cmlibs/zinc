
/*******************************************************************************
       IPOperationsShell.h
       This header file is included by IPOperationsShell.c

*******************************************************************************/

#ifndef	_IPOPERATIONSSHELL_INCLUDED
#define	_IPOPERATIONSSHELL_INCLUDED


#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <Mrm/MrmPublic.h>
#include <Xm/MenuShell.h>
#include "UxXt.h"

#include <Xm/List.h>
#include <Xm/ScrolledW.h>
#include <Xm/SeparatoG.h>
#include <Xm/Text.h>
#include <Xm/ToggleBG.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/CascadeBG.h>
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
	Widget	Uxform6;
	Widget	Uxmenu10;
	Widget	Uxmenu10_top_b1;
	Widget	Uxmenu10_p1;
	Widget	Uxmenu10_p1_b2;
	Widget	UxlabelGadget6;
	Widget	UxlabelGadget9;
	Widget	UxUpdateTBG;
	Widget	UxVerboseTBG;
	Widget	UxParm1TW;
	Widget	UxParm2TW;
	Widget	UxParm3TW;
	Widget	UxParm4TW;
	Widget	UxrowColumn7;
	Widget	UxFFTWindowMenu;
	Widget	Uxmenu1_p5;
	Widget	Uxmenu1_p4_b10;
	Widget	Uxmenu1_p4_b11;
	Widget	Uxmenu1_p5_b6;
	Widget	Uxmenu1_p5_b7;
	Widget	Uxmenu1_p4_b13;
	Widget	Uxmenu1_p4_b12;
	Widget	UxStatusLG;
	Widget	UxseparatorGadget4;
	Widget	UxlabelGadget4;
	Widget	UxscrolledWindow1;
	Widget	UxscrolledList1;
	Widget	Uxmenu8;
	Widget	Uxmenu8_p1_title1;
	Widget	Uxmenu8_p1_b1;
	Widget	Uxmenu8_p1_b2;
	Widget	Uxmenu8_p1_b3;
	Widget	Uxmenu8_p1_b4;
	Widget	Uxmenu8_p1_b5;
	Widget	UxExecSelectedOpPBG;
	Widget	UxExecAllPBG;
	Widget	UxReset;
	int	Uxparmcnt;
} _UxCIPOperationsShell;

#ifdef CONTEXT_MACRO_ACCESS
static _UxCIPOperationsShell   *UxIPOperationsShellContext;
#define form6                   UxIPOperationsShellContext->Uxform6
#define menu10                  UxIPOperationsShellContext->Uxmenu10
#define menu10_top_b1           UxIPOperationsShellContext->Uxmenu10_top_b1
#define menu10_p1               UxIPOperationsShellContext->Uxmenu10_p1
#define menu10_p1_b2            UxIPOperationsShellContext->Uxmenu10_p1_b2
#define labelGadget6            UxIPOperationsShellContext->UxlabelGadget6
#define labelGadget9            UxIPOperationsShellContext->UxlabelGadget9
#define UpdateTBG               UxIPOperationsShellContext->UxUpdateTBG
#define VerboseTBG              UxIPOperationsShellContext->UxVerboseTBG
#define Parm1TW                 UxIPOperationsShellContext->UxParm1TW
#define Parm2TW                 UxIPOperationsShellContext->UxParm2TW
#define Parm3TW                 UxIPOperationsShellContext->UxParm3TW
#define Parm4TW                 UxIPOperationsShellContext->UxParm4TW
#define rowColumn7              UxIPOperationsShellContext->UxrowColumn7
#define FFTWindowMenu           UxIPOperationsShellContext->UxFFTWindowMenu
#define menu1_p5                UxIPOperationsShellContext->Uxmenu1_p5
#define menu1_p4_b10            UxIPOperationsShellContext->Uxmenu1_p4_b10
#define menu1_p4_b11            UxIPOperationsShellContext->Uxmenu1_p4_b11
#define menu1_p5_b6             UxIPOperationsShellContext->Uxmenu1_p5_b6
#define menu1_p5_b7             UxIPOperationsShellContext->Uxmenu1_p5_b7
#define menu1_p4_b13            UxIPOperationsShellContext->Uxmenu1_p4_b13
#define menu1_p4_b12            UxIPOperationsShellContext->Uxmenu1_p4_b12
#define StatusLG                UxIPOperationsShellContext->UxStatusLG
#define separatorGadget4        UxIPOperationsShellContext->UxseparatorGadget4
#define labelGadget4            UxIPOperationsShellContext->UxlabelGadget4
#define scrolledWindow1         UxIPOperationsShellContext->UxscrolledWindow1
#define scrolledList1           UxIPOperationsShellContext->UxscrolledList1
#define menu8                   UxIPOperationsShellContext->Uxmenu8
#define menu8_p1_title1         UxIPOperationsShellContext->Uxmenu8_p1_title1
#define menu8_p1_b1             UxIPOperationsShellContext->Uxmenu8_p1_b1
#define menu8_p1_b2             UxIPOperationsShellContext->Uxmenu8_p1_b2
#define menu8_p1_b3             UxIPOperationsShellContext->Uxmenu8_p1_b3
#define menu8_p1_b4             UxIPOperationsShellContext->Uxmenu8_p1_b4
#define menu8_p1_b5             UxIPOperationsShellContext->Uxmenu8_p1_b5
#define ExecSelectedOpPBG       UxIPOperationsShellContext->UxExecSelectedOpPBG
#define ExecAllPBG              UxIPOperationsShellContext->UxExecAllPBG
#define Reset                   UxIPOperationsShellContext->UxReset
#define parmcnt                 UxIPOperationsShellContext->Uxparmcnt

#endif /* CONTEXT_MACRO_ACCESS */

extern Widget	IPOperationsShell;
extern Widget	IPEditingWindowFW;
extern Widget	RepsLG;
extern Widget	StackLG;
extern Widget	Parm4LG;
extern Widget	Parm3LG;
extern Widget	Parm2LG;
extern Widget	FNameLG;
extern Widget	Parm1LG;
extern Widget	RepsTW;

/*******************************************************************************
       Declarations of global functions.
*******************************************************************************/

Widget	create_IPOperationsShell( void );

#endif	/* _IPOPERATIONSSHELL_INCLUDED */
