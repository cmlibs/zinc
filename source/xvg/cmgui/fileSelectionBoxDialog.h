
/*******************************************************************************
       fileSelectionBoxDialog.h
       This header file is included by fileSelectionBoxDialog.c

*******************************************************************************/

#ifndef	_FILESELECTIONBOXDIALOG_INCLUDED
#define	_FILESELECTIONBOXDIALOG_INCLUDED


#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <Mrm/MrmPublic.h>
#include <Xm/DialogS.h>
#include <Xm/MenuShell.h>
#include "UxXt.h"

#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/FileSB.h>

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
	Widget	Uxmenu13_p1;
	Widget	Uxmenu13_p1_b1;
	Widget	Uxmenu13_p1_b8;
	Widget	Uxmenu13_p1_b3;
	Widget	Uxmenu13_p1_b7;
	Widget	Uxmenu13_p1_b2;
	Widget	Uxmenu13_p1_b4;
	Widget	Uxmenu13_p1_b5;
	Widget	Uxmenu13_p1_b6;
} _UxCfileSelectionBoxDialog;

#ifdef CONTEXT_MACRO_ACCESS
static _UxCfileSelectionBoxDialog *UxFileSelectionBoxDialogContext;
#define menu13_p1               UxFileSelectionBoxDialogContext->Uxmenu13_p1
#define menu13_p1_b1            UxFileSelectionBoxDialogContext->Uxmenu13_p1_b1
#define menu13_p1_b8            UxFileSelectionBoxDialogContext->Uxmenu13_p1_b8
#define menu13_p1_b3            UxFileSelectionBoxDialogContext->Uxmenu13_p1_b3
#define menu13_p1_b7            UxFileSelectionBoxDialogContext->Uxmenu13_p1_b7
#define menu13_p1_b2            UxFileSelectionBoxDialogContext->Uxmenu13_p1_b2
#define menu13_p1_b4            UxFileSelectionBoxDialogContext->Uxmenu13_p1_b4
#define menu13_p1_b5            UxFileSelectionBoxDialogContext->Uxmenu13_p1_b5
#define menu13_p1_b6            UxFileSelectionBoxDialogContext->Uxmenu13_p1_b6

#endif /* CONTEXT_MACRO_ACCESS */

extern Widget	fileSelectionBoxDialog;
extern Widget	FileFormatOM;

/*******************************************************************************
       Declarations of global functions.
*******************************************************************************/

Widget	create_fileSelectionBoxDialog( void );

#endif	/* _FILESELECTIONBOXDIALOG_INCLUDED */
