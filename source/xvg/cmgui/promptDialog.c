
/*******************************************************************************
	promptDialog.c

       Associated Header file: promptDialog.h
*******************************************************************************/

#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <Mrm/MrmPublic.h>
#include <Xm/DialogS.h>
#include <Xm/MenuShell.h>
#include "UxXt.h"

#include <Xm/SelectioB.h>

/*******************************************************************************
       Includes, Defines, and Global variables from the Declarations Editor:
*******************************************************************************/

#include "include/XvgGlobals.h"


/*******************************************************************************
       The following header file defines the context structure.
*******************************************************************************/

#define CONTEXT_MACRO_ACCESS 1
#include "promptDialog.h"
#undef CONTEXT_MACRO_ACCESS

Widget	promptDialog;

/*******************************************************************************
       The following are callback functions.
*******************************************************************************/

static	void	okCallback_promptDialog(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCpromptDialog        *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxPromptDialogContext;
	UxPromptDialogContext = UxContext =
			(_UxCpromptDialog *) UxGetContext( UxWidget );
	{
		XmString xs;
		boolean_t tmp1, tmp2;
		char *p;
		
		/* get rid of the dialog box */
		UxPopdownInterface(promptDialog);
		XmUpdateDisplay(UxGetWidget(promptDialog));
		XSync(UxDisplay, B_FALSE);
		
		/* fetch dialog content */
		XtVaGetValues(UxGetWidget(promptDialog), XmNtextString, &xs, NULL);
		XmStringGetLtoR(xs, XmSTRING_DEFAULT_CHARSET, &p);
		
		/* action */
		switch (PromptDialogAction) {
		    case LOADPSTEPIMAGEACTION:
			HGCursor(B_TRUE);
			LoadPStepImgMenu(atoi(p));
			HGCursor(B_FALSE);
		    break;
		
		    case REBUILDATANLUTACTION:
			HGCursor(B_TRUE);
			CreateAtanLut((AtanRadiusThreshold = atof(p)));
			HGCursor(B_FALSE);
		    break;
		
		    case VCRIMAGESPERSECACTION:
			if ((VCRFramesPerImage = atoi(p)) >= 500)
				ErrorMsg("Maximum number of frames/image exceeded (499)!");
		    break;
		
		    case DEFINEMESHACTION:
		    case MPERPIXELSACTION:
			PixelSpacing = atof(p);
		    break;
		
		    case VCR_SINGLE_IMAGESPERSECACTION:
			if ((VCRFramesPerImage = atoi(p)) >= 500)
				ErrorMsg("Maximum number of frames/image exceeded (499)!");
			else {
				tmp1 = VCAWriteOn;
				tmp2 = VCRRecordOn;
				VCAWriteOn = B_TRUE;
				VCRRecordOn = B_TRUE;
				DumpImage();
				VCAWriteOn = tmp1;
				VCRRecordOn = tmp2;
			}
		    break;
		
		    case SELECTMOVIEFILEINDEXACTION:
			MovieCurrentFile = atoi(p);
			if (VerboseFlag)
			  printf("okCallback_promptDialog() : Index %d..\n", 
				 MovieCurrentFile);
			if (MovieCurrentFile > (MovieFilesN-1))
			  MovieCurrentFile = 0;
			LoadMovieFrame();
		    break;	
		
		    case VCRMOVIELOOPACTION:
			LoopMovie();
		    break;
			
		    case NOACTION:
		    default:
		    break;
		}
		if (p)
		  free(p);
		
	
	}
	UxPromptDialogContext = UxSaveCtx;
}

/*******************************************************************************
       The 'build_' function creates all the widgets
       using the resource values specified in the Property Editor.
*******************************************************************************/

static Widget	_Uxbuild_promptDialog(void)
{
	MrmType		_UxDummyClass;
	Cardinal	_UxStatus;

	promptDialog = NULL;
	_UxStatus = MrmFetchWidget( hierarchy,
					"promptDialog",
					UxTopLevel,
					&promptDialog,
					&_UxDummyClass );
	if ( _UxStatus != MrmSUCCESS )
	{
		UxMrmFetchError( hierarchy, "promptDialog",
				UxTopLevel, _UxStatus );
		return( (Widget) NULL );
	}


	UxPutContext( promptDialog, (char *) UxPromptDialogContext );

	XtAddCallback( promptDialog, XmNdestroyCallback,
		(XtCallbackProc) UxDestroyContextCB,
		(XtPointer) UxPromptDialogContext);

	return ( promptDialog );
}

/*******************************************************************************
       The following is the 'Interface function' which is the
       external entry point for creating this interface.
       This function should be called from your application or from
       a callback function.
*******************************************************************************/

Widget	create_promptDialog(void)
{
	Widget                  rtrn;
	_UxCpromptDialog        *UxContext;
	static int		_Uxinit = 0;

	UxPromptDialogContext = UxContext =
		(_UxCpromptDialog *) UxNewContext( sizeof(_UxCpromptDialog), B_FALSE );


	if ( ! _Uxinit )
	{
		static MrmRegisterArg	_UxMrmNames[] = {
			{ "okCallback_promptDialog", 
				(XtPointer) okCallback_promptDialog }};

		MrmRegisterNamesInHierarchy( hierarchy,
					_UxMrmNames,
					XtNumber(_UxMrmNames) );

		_Uxinit = 1;
	}

	rtrn = _Uxbuild_promptDialog();

	XtUnmanageChild(XmSelectionBoxGetChild(UxGetWidget(rtrn), XmDIALOG_HELP_BUTTON));
	return(rtrn);
}

/*******************************************************************************
       END OF FILE
*******************************************************************************/

