
/*******************************************************************************
       ScrolledWindowDialog.h
       This header file is included by ScrolledWindowDialog.c

*******************************************************************************/

#ifndef	_SCROLLEDWINDOWDIALOG_INCLUDED
#define	_SCROLLEDWINDOWDIALOG_INCLUDED


#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <Mrm/MrmPublic.h>
#include <Xm/DialogS.h>
#include <Xm/MenuShell.h>
#include "UxXt.h"

#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/List.h>
#include <Xm/ScrolledW.h>
#include <Xm/Form.h>

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
	Widget	UxScrolledWindowDialogSWW;
	Widget	UxScrolledWindowDialogSLW;
	Widget	UxrowColumn5;
	swidget	UxUxParent;
} _UxCScrolledWindowDialog;

#ifdef CONTEXT_MACRO_ACCESS
static _UxCScrolledWindowDialog *UxScrolledWindowDialogContext;
#define ScrolledWindowDialogSWW UxScrolledWindowDialogContext->UxScrolledWindowDialogSWW
#define ScrolledWindowDialogSLW UxScrolledWindowDialogContext->UxScrolledWindowDialogSLW
#define rowColumn5              UxScrolledWindowDialogContext->UxrowColumn5
#define UxParent                UxScrolledWindowDialogContext->UxUxParent

#endif /* CONTEXT_MACRO_ACCESS */

extern Widget	ScrolledWindowDialog;
extern Widget	ScrolledWindowOkPBG;
extern Widget	ScrolledWindowScanPBG;
extern Widget	ScrolledWindowCancelPBG;

/*******************************************************************************
       Declarations of global functions.
*******************************************************************************/

Widget	create_ScrolledWindowDialog( swidget _UxUxParent );

#endif	/* _SCROLLEDWINDOWDIALOG_INCLUDED */
