
/*******************************************************************************
	ScrolledWindowDialog.c

       Associated Header file: ScrolledWindowDialog.h
*******************************************************************************/

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
       Includes, Defines, and Global variables from the Declarations Editor:
*******************************************************************************/

#include "include/XvgGlobals.h"


/*******************************************************************************
       The following header file defines the context structure.
*******************************************************************************/

#define CONTEXT_MACRO_ACCESS 1
#include "ScrolledWindowDialog.h"
#undef CONTEXT_MACRO_ACCESS

Widget	ScrolledWindowDialog;
Widget	ScrolledWindowOkPBG;
Widget	ScrolledWindowScanPBG;
Widget	ScrolledWindowCancelPBG;

/*******************************************************************************
Auxiliary code from the Declarations Editor:
*******************************************************************************/

#include "custom/ScrolledWindowDialogAux.c"

/*******************************************************************************
       The following are callback functions.
*******************************************************************************/

static	void	activateCB_ScrolWinOkPBG(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCScrolledWindowDialog *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxScrolledWindowDialogContext;
	UxScrolledWindowDialogContext = UxContext =
			(_UxCScrolledWindowDialog *) UxGetContext( UxWidget );
	{ScrolledWindowOkCB();}
	UxScrolledWindowDialogContext = UxSaveCtx;
}

static	void	activateCB_ScrolWinScanPBG(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCScrolledWindowDialog *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxScrolledWindowDialogContext;
	UxScrolledWindowDialogContext = UxContext =
			(_UxCScrolledWindowDialog *) UxGetContext( UxWidget );
	{
	ScanAllFENodeGroups();
	}
	UxScrolledWindowDialogContext = UxSaveCtx;
}

static	void	activateCB_ScrolWinCancelPBG(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCScrolledWindowDialog *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxScrolledWindowDialogContext;
	UxScrolledWindowDialogContext = UxContext =
			(_UxCScrolledWindowDialog *) UxGetContext( UxWidget );
	{UxPopdownInterface(ScrolledWindowDialog);}
	UxScrolledWindowDialogContext = UxSaveCtx;
}

/*******************************************************************************
       The 'build_' function creates all the widgets
       using the resource values specified in the Property Editor.
*******************************************************************************/

static Widget	_Uxbuild_ScrolledWindowDialog(void)
{
	Widget		_UxParent;
	MrmType		_UxDummyClass;
	Cardinal	_UxStatus;

	_UxParent = UxParent;
	if ( _UxParent == NULL )
	{
		_UxParent = UxTopLevel;
	}

	ScrolledWindowDialog = NULL;
	_UxStatus = MrmFetchWidget( hierarchy,
					"ScrolledWindowDialog",
					_UxParent,
					&ScrolledWindowDialog,
					&_UxDummyClass );
	if ( _UxStatus != MrmSUCCESS )
	{
		UxMrmFetchError( hierarchy, "ScrolledWindowDialog",
				_UxParent, _UxStatus );
		return( (Widget) NULL );
	}


	UxPutContext( ScrolledWindowDialog, (char *) UxScrolledWindowDialogContext );
	ScrolledWindowDialogSWW = XtNameToWidget( ScrolledWindowDialog, "*ScrolledWindowDialogSWW" );
	UxPutContext( ScrolledWindowDialogSWW, (char *) UxScrolledWindowDialogContext );
	ScrolledWindowDialogSLW = XtNameToWidget( ScrolledWindowDialogSWW, "*ScrolledWindowDialogSLW" );
	UxPutContext( ScrolledWindowDialogSLW, (char *) UxScrolledWindowDialogContext );
	rowColumn5 = XtNameToWidget( ScrolledWindowDialog, "*rowColumn5" );
	UxPutContext( rowColumn5, (char *) UxScrolledWindowDialogContext );
	ScrolledWindowOkPBG = XtNameToWidget( rowColumn5, "*ScrolledWindowOkPBG" );
	UxPutContext( ScrolledWindowOkPBG, (char *) UxScrolledWindowDialogContext );
	ScrolledWindowScanPBG = XtNameToWidget( rowColumn5, "*ScrolledWindowScanPBG" );
	UxPutContext( ScrolledWindowScanPBG, (char *) UxScrolledWindowDialogContext );
	ScrolledWindowCancelPBG = XtNameToWidget( rowColumn5, "*ScrolledWindowCancelPBG" );
	UxPutContext( ScrolledWindowCancelPBG, (char *) UxScrolledWindowDialogContext );

	XtAddCallback( ScrolledWindowDialog, XmNdestroyCallback,
		(XtCallbackProc) UxDestroyContextCB,
		(XtPointer) UxScrolledWindowDialogContext);

	return ( ScrolledWindowDialog );
}

/*******************************************************************************
       The following is the "convenience function" for creating a
       copy of the interface.  It is called from UIL code that uses
       this interface as a component.
*******************************************************************************/

Widget	conv_create_ScrolledWindowDialog( Widget _Uxparent, String _UxName,
				Arg *_Uxargs, Cardinal _Uxnum_args )
{
	Widget	_Uxwgt;
	String	_Uxname = _UxName;

	_Uxwgt = create_ScrolledWindowDialog( _Uxparent );

	if ( _Uxnum_args > 0 )
		XtSetValues( _Uxwgt, _Uxargs, _Uxnum_args );

	return ( _Uxwgt );
}

/*******************************************************************************
       The following is the 'Interface function' which is the
       external entry point for creating this interface.
       This function should be called from your application or from
       a callback function.
*******************************************************************************/

Widget	create_ScrolledWindowDialog( swidget _UxUxParent )
{
	Widget                  rtrn;
	_UxCScrolledWindowDialog *UxContext;
	static int		_Uxinit = 0;

	UxScrolledWindowDialogContext = UxContext =
		(_UxCScrolledWindowDialog *) UxNewContext( sizeof(_UxCScrolledWindowDialog), B_FALSE );

	UxParent = _UxUxParent;

	if ( ! _Uxinit )
	{
		static MrmRegisterArg	_UxMrmNames[] = {
			{ "activateCB_ScrolWinOkPBG", 
				(XtPointer) activateCB_ScrolWinOkPBG },
			{ "activateCB_ScrolWinScanPBG", 
				(XtPointer) activateCB_ScrolWinScanPBG },
			{ "activateCB_ScrolWinCancelPBG", 
				(XtPointer) activateCB_ScrolWinCancelPBG }};

		MrmRegisterNamesInHierarchy( hierarchy,
					_UxMrmNames,
					XtNumber(_UxMrmNames) );

		_Uxinit = 1;
	}

	rtrn = _Uxbuild_ScrolledWindowDialog();

	return(rtrn);
}

/*******************************************************************************
       END OF FILE
*******************************************************************************/

