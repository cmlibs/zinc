
/*******************************************************************************
	InputDialog.c

       Associated Header file: InputDialog.h
       Associated Resource file: InputDialog.rf
*******************************************************************************/

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
       Includes, Defines, and Global variables from the Declarations Editor:
*******************************************************************************/

#include "include/XvgGlobals.h"


/*******************************************************************************
       The following header file defines the context structure.
*******************************************************************************/

#define CONTEXT_MACRO_ACCESS 1
#include "InputDialog.h"
#undef CONTEXT_MACRO_ACCESS

Widget	InputDialog;

/*******************************************************************************
Auxiliary code from the Declarations Editor:
*******************************************************************************/

/* indirect callbacks */
void InputDialogLoadParms(void)
{
  char c[256];
  sprintf(c, "%d", Image_pixel_org_X);
  UxPutText(UxGetWidget(XOrgTW), c);
  sprintf(c, "%d", Image_pixel_org_Y);
  UxPutText(UxGetWidget(YOrgTW), c);
  sprintf(c, "%f", Image_m_per_pixel_X);
  UxPutText(UxGetWidget(XScaleTW), c);
  sprintf(c, "%f", Image_m_per_pixel_Y);
  UxPutText(UxGetWidget(YScaleTW), c);
  sprintf(c, "%f", Image_Z_plane);
  UxPutText(UxGetWidget(ZPlaneTW), c);
}

/*******************************************************************************
       The following are callback functions.
*******************************************************************************/

static	void	activateCB_pushButtonGadget2(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCInputDialog         *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxInputDialogContext;
	UxInputDialogContext = UxContext =
			(_UxCInputDialog *) UxGetContext( UxWidget );
	{
	  Image_pixel_org_X = atoi(UxGetText(UxGetWidget(XOrgTW)));
	  Image_pixel_org_Y = atoi(UxGetText(UxGetWidget(YOrgTW)));
	  Image_m_per_pixel_X = atof(UxGetText(UxGetWidget(XScaleTW)));
	  Image_m_per_pixel_Y = atof(UxGetText(UxGetWidget(YScaleTW)));
	  Image_pixels_per_m_X = 1.0 / Image_m_per_pixel_X;
	  Image_pixels_per_m_Y = 1.0 / Image_m_per_pixel_Y;
	  Image_Z_plane = atof(UxGetText(UxGetWidget(ZPlaneTW)));
	  if (Image_m_per_pixel_X == Image_m_per_pixel_Y)
	    PixelSpacing = Image_m_per_pixel_X;
	  else
	    printf("\aWARNING: dx != dy, PixelSpacing not set\n");
	  if (VerboseFlag) {
	    printf("Image_pixel_org_X    : %d\n", Image_pixel_org_X);
	    printf("Image_pixel_org_Y    : %d\n", Image_pixel_org_Y);
	    printf("Image_m_per_pixel_X  : %f\n", Image_m_per_pixel_X);
	    printf("Image_m_per_pixel_Y  : %f\n", Image_m_per_pixel_Y);
	    printf("Image_pixels_per_m_X : %f\n", Image_pixels_per_m_X);
	    printf("Image_pixels_per_m_Y : %f\n", Image_pixels_per_m_Y);
	    printf("Image_Z_plane        : %f\n", Image_Z_plane);
	  }
	}
	UxInputDialogContext = UxSaveCtx;
}

static	void	activateCB_pushButtonGadget3(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCInputDialog         *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxInputDialogContext;
	UxInputDialogContext = UxContext =
			(_UxCInputDialog *) UxGetContext( UxWidget );
	{
	}
	UxInputDialogContext = UxSaveCtx;
}

/*******************************************************************************
       The 'build_' function creates all the widgets
       using the resource values specified in the Property Editor.
*******************************************************************************/

static Widget	_Uxbuild_InputDialog(void)
{
	Widget		_UxParent;
	MrmType		_UxDummyClass;
	Cardinal	_UxStatus;

	_UxParent = UxParent;
	if ( _UxParent == NULL )
	{
		_UxParent = UxTopLevel;
	}

	InputDialog = NULL;
	_UxStatus = MrmFetchWidget( hierarchy,
					"InputDialog",
					_UxParent,
					&InputDialog,
					&_UxDummyClass );
	if ( _UxStatus != MrmSUCCESS )
	{
		UxMrmFetchError( hierarchy, "InputDialog",
				_UxParent, _UxStatus );
		return( (Widget) NULL );
	}


	UxPutContext( InputDialog, (char *) UxInputDialogContext );
	XScaleTW = XtNameToWidget( InputDialog, "*XScaleTW" );
	UxPutContext( XScaleTW, (char *) UxInputDialogContext );
	label1 = XtNameToWidget( InputDialog, "*label1" );
	UxPutContext( label1, (char *) UxInputDialogContext );
	pushButtonGadget2 = XtNameToWidget( InputDialog, "*pushButtonGadget2" );
	UxPutContext( pushButtonGadget2, (char *) UxInputDialogContext );
	pushButtonGadget3 = XtNameToWidget( InputDialog, "*pushButtonGadget3" );
	UxPutContext( pushButtonGadget3, (char *) UxInputDialogContext );
	labelGadget5 = XtNameToWidget( InputDialog, "*labelGadget5" );
	UxPutContext( labelGadget5, (char *) UxInputDialogContext );
	YScaleTW = XtNameToWidget( InputDialog, "*YScaleTW" );
	UxPutContext( YScaleTW, (char *) UxInputDialogContext );
	label2 = XtNameToWidget( InputDialog, "*label2" );
	UxPutContext( label2, (char *) UxInputDialogContext );
	ZPlaneTW = XtNameToWidget( InputDialog, "*ZPlaneTW" );
	UxPutContext( ZPlaneTW, (char *) UxInputDialogContext );
	label3 = XtNameToWidget( InputDialog, "*label3" );
	UxPutContext( label3, (char *) UxInputDialogContext );
	XOrgTW = XtNameToWidget( InputDialog, "*XOrgTW" );
	UxPutContext( XOrgTW, (char *) UxInputDialogContext );
	label4 = XtNameToWidget( InputDialog, "*label4" );
	UxPutContext( label4, (char *) UxInputDialogContext );
	YOrgTW = XtNameToWidget( InputDialog, "*YOrgTW" );
	UxPutContext( YOrgTW, (char *) UxInputDialogContext );
	label5 = XtNameToWidget( InputDialog, "*label5" );
	UxPutContext( label5, (char *) UxInputDialogContext );

	XtAddCallback( InputDialog, XmNdestroyCallback,
		(XtCallbackProc) UxDestroyContextCB,
		(XtPointer) UxInputDialogContext);

	return ( InputDialog );
}

/*******************************************************************************
       The following is the "convenience function" for creating a
       copy of the interface.  It is called from UIL code that uses
       this interface as a component.
*******************************************************************************/

Widget	conv_create_InputDialog( Widget _Uxparent, String _UxName,
				Arg *_Uxargs, Cardinal _Uxnum_args )
{
	Widget	_Uxwgt;
	String	_Uxname = _UxName;

	_Uxwgt = create_InputDialog( _Uxparent );

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

Widget	create_InputDialog( swidget _UxUxParent )
{
	Widget                  rtrn;
	_UxCInputDialog         *UxContext;
	static int		_Uxinit = 0;

	UxInputDialogContext = UxContext =
		(_UxCInputDialog *) UxNewContext( sizeof(_UxCInputDialog), B_FALSE );

	UxParent = _UxUxParent;

	if ( ! _Uxinit )
	{
		static MrmRegisterArg	_UxMrmNames[] = {
			{ "activateCB_pushButtonGadget2", 
				(XtPointer) activateCB_pushButtonGadget2 },
			{ "activateCB_pushButtonGadget3", 
				(XtPointer) activateCB_pushButtonGadget3 }};

		MrmRegisterNamesInHierarchy( hierarchy,
					_UxMrmNames,
					XtNumber(_UxMrmNames) );

		_Uxinit = 1;
	}

	rtrn = _Uxbuild_InputDialog();

	return(rtrn);
}

/*******************************************************************************
       END OF FILE
*******************************************************************************/

