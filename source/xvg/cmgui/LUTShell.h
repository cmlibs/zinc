
/*******************************************************************************
       LUTShell.h
       This header file is included by LUTShell.c

*******************************************************************************/

#ifndef	_LUTSHELL_INCLUDED
#define	_LUTSHELL_INCLUDED


#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <Mrm/MrmPublic.h>
#include <Xm/MenuShell.h>
#include "UxXt.h"

#include <Xm/RowColumn.h>
#include <Xm/SeparatoG.h>
#include <Xm/PushBG.h>
#include <Xm/LabelG.h>
#include <Xm/Text.h>
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
	Widget	Uxform12;
	Widget	UxlabelGadget66;
	Widget	UxpushButtonGadget1;
	Widget	UxseparatorGadget11;
	Widget	Uxmenu7;
	Widget	Uxmenu7_p1;
	Widget	Uxmenu7_p1_b2;
	Widget	Uxmenu7_p1_b1;
	Widget	Uxmenu8;
	Widget	Uxmenu8_p1;
	Widget	Uxmenu8_p1_b2;
	Widget	Uxmenu8_p1_b1;
	int	Uxii;
	int	Uxjj;
	int	Uxkk;
} _UxCLUTShell;

#ifdef CONTEXT_MACRO_ACCESS
static _UxCLUTShell            *UxLUTShellContext;
#define form12                  UxLUTShellContext->Uxform12
#define labelGadget66           UxLUTShellContext->UxlabelGadget66
#define pushButtonGadget1       UxLUTShellContext->UxpushButtonGadget1
#define separatorGadget11       UxLUTShellContext->UxseparatorGadget11
#define menu7                   UxLUTShellContext->Uxmenu7
#define menu7_p1                UxLUTShellContext->Uxmenu7_p1
#define menu7_p1_b2             UxLUTShellContext->Uxmenu7_p1_b2
#define menu7_p1_b1             UxLUTShellContext->Uxmenu7_p1_b1
#define menu8                   UxLUTShellContext->Uxmenu8
#define menu8_p1                UxLUTShellContext->Uxmenu8_p1
#define menu8_p1_b2             UxLUTShellContext->Uxmenu8_p1_b2
#define menu8_p1_b1             UxLUTShellContext->Uxmenu8_p1_b1
#define ii                      UxLUTShellContext->Uxii
#define jj                      UxLUTShellContext->Uxjj
#define kk                      UxLUTShellContext->Uxkk

#endif /* CONTEXT_MACRO_ACCESS */

extern Widget	LUTShell;
extern Widget	ColorControlPtTW;

/*******************************************************************************
       Declarations of global functions.
*******************************************************************************/

Widget	create_LUTShell( void );

#endif	/* _LUTSHELL_INCLUDED */
