
/*******************************************************************************
	errorDialog.c

       Associated Header file: errorDialog.h
       Associated Resource file: errorDialog.rf
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
extern swidget XvgShell;


/*******************************************************************************
       The following header file defines the context structure.
*******************************************************************************/

#define CONTEXT_MACRO_ACCESS 1
#include "errorDialog.h"
#undef CONTEXT_MACRO_ACCESS

Widget	errorDialog;

/*******************************************************************************
       The following are callback functions.
*******************************************************************************/

static	void	okCallback_errorDialog(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCerrorDialog         *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxErrorDialogContext;
	UxErrorDialogContext = UxContext =
			(_UxCerrorDialog *) UxGetContext( UxWidget );
	{
	UxPopdownInterface(errorDialog);
	}
	UxErrorDialogContext = UxSaveCtx;
}

static	void	createCB_errorDialog(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCerrorDialog         *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxErrorDialogContext;
	UxContext = UxErrorDialogContext;
	{
	ErrorMsgWindowCreated = B_TRUE;
	}
	UxErrorDialogContext = UxSaveCtx;
}

/*******************************************************************************
       The 'build_' function creates all the widgets
       using the resource values specified in the Property Editor.
*******************************************************************************/

static Widget	_Uxbuild_errorDialog(void)
{
	Widget		_UxParent;
	MrmType		_UxDummyClass;
	Cardinal	_UxStatus;

	_UxParent = XvgShell;
	if ( _UxParent == NULL )
	{
		_UxParent = UxTopLevel;
	}

	errorDialog = NULL;
	_UxStatus = MrmFetchWidget( hierarchy,
					"errorDialog",
					_UxParent,
					&errorDialog,
					&_UxDummyClass );
	if ( _UxStatus != MrmSUCCESS )
	{
		UxMrmFetchError( hierarchy, "errorDialog",
				_UxParent, _UxStatus );
		return( (Widget) NULL );
	}


	UxPutContext( errorDialog, (char *) UxErrorDialogContext );

	XtAddCallback( errorDialog, XmNdestroyCallback,
		(XtCallbackProc) UxDestroyContextCB,
		(XtPointer) UxErrorDialogContext);

	return ( errorDialog );
}

/*******************************************************************************
       The following is the 'Interface function' which is the
       external entry point for creating this interface.
       This function should be called from your application or from
       a callback function.
*******************************************************************************/

Widget	create_errorDialog(void)
{
	Widget                  rtrn;
	_UxCerrorDialog         *UxContext;
	static int		_Uxinit = 0;

	UxErrorDialogContext = UxContext =
		(_UxCerrorDialog *) UxNewContext( sizeof(_UxCerrorDialog), B_FALSE );


	if ( ! _Uxinit )
	{
		static MrmRegisterArg	_UxMrmNames[] = {
			{ "okCallback_errorDialog", 
				(XtPointer) okCallback_errorDialog },
			{ "createCB_errorDialog", 
				(XtPointer) createCB_errorDialog }};

		MrmRegisterNamesInHierarchy( hierarchy,
					_UxMrmNames,
					XtNumber(_UxMrmNames) );

		UxLoadResources( "errorDialog.rf" );
		_Uxinit = 1;
	}

	rtrn = _Uxbuild_errorDialog();

	/* zap the cancel and help buttons */
	XtUnmanageChild(XmMessageBoxGetChild(UxGetWidget(rtrn), XmDIALOG_CANCEL_BUTTON));
	XtUnmanageChild(XmMessageBoxGetChild(UxGetWidget(rtrn), XmDIALOG_HELP_BUTTON));
	return(rtrn);
}

/*******************************************************************************
       END OF FILE
*******************************************************************************/

