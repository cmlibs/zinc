
/*******************************************************************************
       promptDialog.h
       This header file is included by promptDialog.c

*******************************************************************************/

#ifndef	_PROMPTDIALOG_INCLUDED
#define	_PROMPTDIALOG_INCLUDED


#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <Mrm/MrmPublic.h>
#include <Xm/DialogS.h>
#include <Xm/MenuShell.h>
#include "UxXt.h"

#include <Xm/SelectioB.h>

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
	int	mumble;
} _UxCpromptDialog;

#ifdef CONTEXT_MACRO_ACCESS
static _UxCpromptDialog        *UxPromptDialogContext;

#endif /* CONTEXT_MACRO_ACCESS */

extern Widget	promptDialog;

/*******************************************************************************
       Declarations of global functions.
*******************************************************************************/

Widget	create_promptDialog( void );

#endif	/* _PROMPTDIALOG_INCLUDED */
