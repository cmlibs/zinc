
/*******************************************************************************
       InputDialog.h
       This header file is included by InputDialog.c

*******************************************************************************/

#ifndef	_INPUTDIALOG_INCLUDED
#define	_INPUTDIALOG_INCLUDED


#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <Mrm/MrmPublic.h>
#include <Xm/DialogS.h>
#include <Xm/MenuShell.h>
#include "UxXt.h"

#include <Xm/LabelG.h>
#include <Xm/PushBG.h>
#include <Xm/Label.h>
#include <Xm/Text.h>
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
	Widget	UxXScaleTW;
	Widget	Uxlabel1;
	Widget	UxpushButtonGadget2;
	Widget	UxpushButtonGadget3;
	Widget	UxlabelGadget5;
	Widget	UxYScaleTW;
	Widget	Uxlabel2;
	Widget	UxZPlaneTW;
	Widget	Uxlabel3;
	Widget	UxXOrgTW;
	Widget	Uxlabel4;
	Widget	UxYOrgTW;
	Widget	Uxlabel5;
	swidget	UxUxParent;
} _UxCInputDialog;

#ifdef CONTEXT_MACRO_ACCESS
static _UxCInputDialog         *UxInputDialogContext;
#define XScaleTW                UxInputDialogContext->UxXScaleTW
#define label1                  UxInputDialogContext->Uxlabel1
#define pushButtonGadget2       UxInputDialogContext->UxpushButtonGadget2
#define pushButtonGadget3       UxInputDialogContext->UxpushButtonGadget3
#define labelGadget5            UxInputDialogContext->UxlabelGadget5
#define YScaleTW                UxInputDialogContext->UxYScaleTW
#define label2                  UxInputDialogContext->Uxlabel2
#define ZPlaneTW                UxInputDialogContext->UxZPlaneTW
#define label3                  UxInputDialogContext->Uxlabel3
#define XOrgTW                  UxInputDialogContext->UxXOrgTW
#define label4                  UxInputDialogContext->Uxlabel4
#define YOrgTW                  UxInputDialogContext->UxYOrgTW
#define label5                  UxInputDialogContext->Uxlabel5
#define UxParent                UxInputDialogContext->UxUxParent

#endif /* CONTEXT_MACRO_ACCESS */

extern Widget	InputDialog;

/*******************************************************************************
       Declarations of global functions.
*******************************************************************************/

Widget	create_InputDialog( swidget _UxUxParent );

#endif	/* _INPUTDIALOG_INCLUDED */
