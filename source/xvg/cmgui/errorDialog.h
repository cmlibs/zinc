
/*******************************************************************************
       errorDialog.h
       This header file is included by errorDialog.c

*******************************************************************************/

#ifndef	_ERRORDIALOG_INCLUDED
#define	_ERRORDIALOG_INCLUDED


#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <Mrm/MrmPublic.h>
#include <Xm/DialogS.h>
#include <Xm/MenuShell.h>
#include "UxXt.h"

#include <Xm/MessageB.h>

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
} _UxCerrorDialog;

#ifdef CONTEXT_MACRO_ACCESS
static _UxCerrorDialog         *UxErrorDialogContext;

#endif /* CONTEXT_MACRO_ACCESS */

extern Widget	errorDialog;

/*******************************************************************************
       Declarations of global functions.
*******************************************************************************/

Widget	create_errorDialog( void );

#endif	/* _ERRORDIALOG_INCLUDED */
