
/*******************************************************************************
	InfoDialog.c

       Associated Header file: InfoDialog.h
*******************************************************************************/

#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <Mrm/MrmPublic.h>
#include <Xm/DialogS.h>
#include <Xm/MenuShell.h>
#include "UxXt.h"

#include <Xm/MessageB.h>

/*******************************************************************************
       Includes, Defines, and Global variables from the Declarations Editor:
*******************************************************************************/

#include "include/XvgGlobals.h"


/*******************************************************************************
       The following header file defines the context structure.
*******************************************************************************/

#define CONTEXT_MACRO_ACCESS 1
#include "InfoDialog.h"
#undef CONTEXT_MACRO_ACCESS

Widget	InfoDialog;

/*******************************************************************************
       The following are callback functions.
*******************************************************************************/

/*******************************************************************************
       The 'build_' function creates all the widgets
       using the resource values specified in the Property Editor.
*******************************************************************************/

static Widget	_Uxbuild_InfoDialog(void)
{
	MrmType		_UxDummyClass;
	Cardinal	_UxStatus;

	InfoDialog = NULL;
	_UxStatus = MrmFetchWidget( hierarchy,
					"InfoDialog",
					UxTopLevel,
					&InfoDialog,
					&_UxDummyClass );
	if ( _UxStatus != MrmSUCCESS )
	{
		UxMrmFetchError( hierarchy, "InfoDialog",
				UxTopLevel, _UxStatus );
		return( (Widget) NULL );
	}


	UxPutContext( InfoDialog, (char *) UxInfoDialogContext );

	XtAddCallback( InfoDialog, XmNdestroyCallback,
		(XtCallbackProc) UxDestroyContextCB,
		(XtPointer) UxInfoDialogContext);

	return ( InfoDialog );
}

/*******************************************************************************
       The following is the 'Interface function' which is the
       external entry point for creating this interface.
       This function should be called from your application or from
       a callback function.
*******************************************************************************/

Widget	create_InfoDialog(void)
{
	Widget                  rtrn;
	_UxCInfoDialog          *UxContext;
	static int		_Uxinit = 0;

	UxInfoDialogContext = UxContext =
		(_UxCInfoDialog *) UxNewContext( sizeof(_UxCInfoDialog), B_FALSE );


	if ( ! _Uxinit )
	{
		_Uxinit = 1;
	}

	rtrn = _Uxbuild_InfoDialog();

	/* zap the cancel and help buttons */
	XtUnmanageChild(XmMessageBoxGetChild(UxGetWidget(rtrn), XmDIALOG_CANCEL_BUTTON));
	XtUnmanageChild(XmMessageBoxGetChild(UxGetWidget(rtrn), XmDIALOG_HELP_BUTTON));
	return(rtrn);
}

/*******************************************************************************
       END OF FILE
*******************************************************************************/

