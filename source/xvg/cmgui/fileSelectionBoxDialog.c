
/*******************************************************************************
	fileSelectionBoxDialog.c

       Associated Header file: fileSelectionBoxDialog.h
       Associated Resource file: fileSelectionBoxDialog.rf
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
#include <Xm/FileSB.h>

/*******************************************************************************
       Includes, Defines, and Global variables from the Declarations Editor:
*******************************************************************************/

#include "include/XvgGlobals.h"

/* externs */
extern swidget XvgShell, ProcessedImageDrawingArea, RawImageDrawingArea,
	RawImageShell, ProcessedImageShell, RawImageTBG, ProcessedImageTBG,
	MovieNameLG, MovieControlsShell;

/* globals */
static int OutFileFormat;
static Matrix *e;

/*******************************************************************************
       The following header file defines the context structure.
*******************************************************************************/

#define CONTEXT_MACRO_ACCESS 1
#include "fileSelectionBoxDialog.h"
#undef CONTEXT_MACRO_ACCESS

Widget	fileSelectionBoxDialog;
Widget	FileFormatOM;

/*******************************************************************************
       The following are callback functions.
*******************************************************************************/

static	void	cancelCB_fileSelectionBoxDialog(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCfileSelectionBoxDialog *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxFileSelectionBoxDialogContext;
	UxFileSelectionBoxDialogContext = UxContext =
			(_UxCfileSelectionBoxDialog *) UxGetContext( UxWidget );
	{
	UxPopdownInterface(fileSelectionBoxDialog);
	XtSetSensitive(UxGetWidget(FileFormatOM), B_FALSE);
	
	
	}
	UxFileSelectionBoxDialogContext = UxSaveCtx;
}

static	void	okCallback_fileSelBoxDialog(
					    Widget wgt, 
					    XtPointer cd, 
					    XtPointer cb)
{
  _UxCfileSelectionBoxDialog *UxSaveCtx, *UxContext;
  Widget                  UxWidget = wgt;
  XtPointer               UxClientData = cd;
  XtPointer               UxCallbackArg = cb;
  
  UxSaveCtx = UxFileSelectionBoxDialogContext;
  UxFileSelectionBoxDialogContext = UxContext =
    (_UxCfileSelectionBoxDialog *) UxGetContext( UxWidget );
  {
    XImage *image;
    XColor cells[256];
    XmString xs;
    MATFile *DataFp;
    Matrix *m;
    int i;
    char s[256], *fname;
    double buf[HISTOGRAM_BINS];
    
    /* get rid of the dialog box */
    UxPopdownInterface(fileSelectionBoxDialog);
    XmUpdateDisplay(UxGetWidget(fileSelectionBoxDialog));
    XmUpdateDisplay(UxGetWidget(XvgShell));
    
    XtVaGetValues(UxGetWidget(fileSelectionBoxDialog), XmNtextString,
		  &xs, NULL);
    fname = NULL;
    XmStringGetLtoR(xs, XmSTRING_DEFAULT_CHARSET, &fname);
    if (VerboseFlag)
      printf("okCallback_fileSelBoxDialog(%s, length:%d)\n",
	     fname, strlen(fname));
    
    /* fetch window parameters */
    UpdateRawWinParms();
    UpdateProcessedWinParms();
    
    /* action */
    HGCursor(B_TRUE);
    switch (FileSelectionBoxAction) {
    case SAVEGLFRAMEACTION:
      SaveGLFrame(fname);
      break;
      
    case GENGLMOVIEACTION:
      GenGLMovie(fname);
      break;
      
    case SAVEDATAACTION:
      XtSetSensitive(UxGetWidget(FileFormatOM), B_FALSE);
      switch (OutFileFormat) {
      case PX8_FF:
	if (ProcessedPixels != NULL)
	  WritePxFile(ImgHeight, ImgWidth, ProcessedPixels, fname, 8);
	else
	  WritePxFile(ImgHeight, ImgWidth, RawPixels, fname, 8);
	break;
	
      case PX16_FF:
	if (ProcessedPixels != NULL)
	  WritePxFile(ImgHeight, ImgWidth, ProcessedPixels, fname, 16);
	else
	  WritePxFile(ImgHeight, ImgWidth, RawPixels, fname, 8);
	break;
	
      case PX32_FF:
	if (ProcessedPixels != NULL)
	  WritePxFile(ImgHeight, ImgWidth, ProcessedPixels, fname, 32);
	else
	  WritePxFile(ImgHeight, ImgWidth, RawPixels, fname, 8);
	break;
	
      case MATLAB_FF:
	if (ProcessedPixels != NULL)
	  WriteMatlabFile(ImgHeight, ImgWidth, "xvgimage",
			  ProcessedPixels, fname);
	else
	  WriteMatlabFile(ImgHeight, ImgWidth, "xvgimage", RawPixels, fname);
	break;

      case MATLAB_16BIT_FF:
	if (ProcessedPixels != NULL)
	  WriteMatlabUShortFile(ImgHeight, ImgWidth, "xvgimage",
				ProcessedPixels, fname);
	else
	  WriteMatlabUShortFile(ImgHeight, ImgWidth, "xvgimage",
				RawPixels, fname);
	break;
	
      case IGS_FF:
	if (ProcessedPixels != NULL)
	  WriteIGSFile(ImgHeight, ImgWidth, ProcessedPixels, fname);
	else
	  WriteIGSFile(ImgHeight, ImgWidth, RawPixels, fname);
	break;

      case TIFFGREYLEVEL_FF:
	if (ProcessedPixels != NULL)
	  WriteTiffFileGreyLevel(ProcessedPixels, ProcessedWinHOffset,
				 ProcessedWinVOffset,
				 ProcessedWinHeight, ProcessedWinWidth,
				 fname, NULL, B_FALSE);
	else
	  WriteTiffFileGreyLevel(RawPixels, RawWinHOffset, RawWinVOffset,
				 RawWinHeight, RawWinWidth, fname,
				 NULL, B_FALSE);
	break;
      case TIFFCOLOR_FF:
	/* fetch the current color cell definitions */
	for (i = 0; i < 256; i++)
	  cells[i].pixel = i;
	XQueryColors(UxDisplay, cmap, cells, 256);
	
	/* fetch drawing area contents */	
	if (IsTrue(UxGetSet(ProcessedImageTBG))) {
	  image = XGetImage(UxDisplay,
			    XtWindow(UxGetWidget(ProcessedImageDrawingArea)),
			    ProcessedWinHOffset, ProcessedWinVOffset,
			    ProcessedWinWidth, ProcessedWinHeight,
			    (unsigned long) 0xffffffff,
			    ZPixmap);
	  WriteTiffFileColor(image, 0, 0, ProcessedWinHeight,
			     ProcessedWinWidth, fname, NULL, cells,
			     DecimationFactor);
	}
	else if (IsTrue(UxGetSet(RawImageTBG))){
	  image = XGetImage(UxDisplay,
			    XtWindow(UxGetWidget(RawImageDrawingArea)),
			    RawWinHOffset, RawWinVOffset,
			    RawWinWidth, RawWinHeight,
			    (unsigned long) 0xffffffff,
			    ZPixmap);
	  WriteTiffFileColor(image, 0, 0,
			     RawWinHeight, RawWinWidth, fname, NULL, cells,
			     DecimationFactor);
	}
	else {
	  ErrorMsg("No image window popped up for saving!");
	  HGCursor(B_FALSE);
	  return;
	}
	
	/* destroy the image */
	XDestroyImage(image);
	break;
      default:
	break;
      }
      break;
      
    case SAVEHISTOGRAMACTION:
      /* open the MATLAB file */
      if ((DataFp=matOpen(fname, "w")) == NULL) {
	sprintf(s, "Could not open histogram output file %s", fname);
	ErrorMsg(s);
	return;
      }
      m = mxCreateFull(HISTOGRAM_BINS, 1, REAL);
      
      /* save Raw Histogram */
      for (i = 0; i < HISTOGRAM_BINS; i++)
	buf[i] = RawHistogram[i];
      memcpy(mxGetPr(m), buf, HISTOGRAM_BINS*sizeof(double));
      mxSetName(m, "RawHistogram");
      matPutMatrix(DataFp, m);
      
      memcpy(mxGetPr(e), &RawMean, sizeof(double));
      mxSetName(e, "RawMean");
      matPutMatrix(DataFp, e);
      
      memcpy(mxGetPr(e), &RawSD, sizeof(double));
      mxSetName(e, "RawSD");
      matPutMatrix(DataFp, e);
      
      memcpy(mxGetPr(e), &RawMin, sizeof(double));
      mxSetName(e, "RawMin");
      matPutMatrix(DataFp, e);
      
      memcpy(mxGetPr(e), &RawMax, sizeof(double));
      mxSetName(e, "RawMax");
      matPutMatrix(DataFp, e);
      
      /* save Processed Histogram */
      for (i = 0; i < HISTOGRAM_BINS; i++)
	buf[i] = ProcessedHistogram[i];
      memcpy(mxGetPr(m), buf, HISTOGRAM_BINS*sizeof(double));
      mxSetName(m, "ProcHistogram");
      matPutMatrix(DataFp, m);
      
      memcpy(mxGetPr(e), &ProcMean, sizeof(double));
      mxSetName(e, "ProcMean");
      matPutMatrix(DataFp, e);
      
      memcpy(mxGetPr(e), &ProcSD, sizeof(double));
      mxSetName(e, "ProcSD");
      matPutMatrix(DataFp, e);
      
      memcpy(mxGetPr(e), &ProcMin, sizeof(double));
      mxSetName(e, "ProcMin");
      matPutMatrix(DataFp, e);
      
      memcpy(mxGetPr(e), &ProcMax, sizeof(double));
      mxSetName(e, "ProcMax");
      matPutMatrix(DataFp, e);
      
      /* close the file & free storage */
      matClose(DataFp);
      mxFreeMatrix(m);
      break;
      
    case SAVEIPOPSACTION:
      SaveIPOps(fname);
      break;
    case SAVEGLMOVIEACTION:
      GLWriteMovie(fname);
      break;
    case LOADGLMOVIEACTION:
      MovieType = GLMOVIE;
      sprintf(s, "GL movie file : %s", fname);
      UxPutLabelString(MovieNameLG, s);
      XSync(UxDisplay, B_FALSE);
      GLReadMovie(fname);
      break;
    case LOADMOVIEACTION:
      MovieType = XMOVIE;
      sprintf(s, "X movie file : %s", fname);
      UxPutLabelString(MovieNameLG, s);
      XSync(UxDisplay, B_FALSE);
      UxFree(MovieFileName);
      if ((MovieFileName =
	   (char *) XvgMM_Alloc(MovieFileName, strlen(fname) + 1))
	  == NULL) {
	sprintf(cbuf,
		"Error allocating for %s in okCallback_fileSelBoxDialog()",
		buf);
	ErrorMsg(cbuf);
	return;
      }
      sprintf(MovieFileName, "%s", fname);
      LoadMovie(MovieFileName);
      break;
    case LOADIPOPSACTION:
      LoadIPOps(fname);
      break;
    case LOADFILEACTION:
    default:
      InFileName = (char *) XvgMM_Alloc((void *) InFileName,
					strlen(fname) + 1);
      sprintf(InFileName, "%s", fname);
      LoadFile(InFileName);
      break;
    }
    HGCursor(B_FALSE);
    if (fname)
      free(fname);
  }
  UxFileSelectionBoxDialogContext = UxSaveCtx;
}

static	void	activateCB_menu13_p1_b1(
					Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCfileSelectionBoxDialog *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxFileSelectionBoxDialogContext;
	UxFileSelectionBoxDialogContext = UxContext =
			(_UxCfileSelectionBoxDialog *) UxGetContext( UxWidget );
	{OutFileFormat = MATLAB_FF;}
	UxFileSelectionBoxDialogContext = UxSaveCtx;
}

static	void	activateCB_menu13_p1_b8(
					Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCfileSelectionBoxDialog *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxFileSelectionBoxDialogContext;
	UxFileSelectionBoxDialogContext = UxContext =
			(_UxCfileSelectionBoxDialog *) UxGetContext( UxWidget );
	{OutFileFormat = MATLAB_16BIT_FF;}
	UxFileSelectionBoxDialogContext = UxSaveCtx;
}

static	void	activateCB_menu13_p1_b3(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCfileSelectionBoxDialog *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxFileSelectionBoxDialogContext;
	UxFileSelectionBoxDialogContext = UxContext =
			(_UxCfileSelectionBoxDialog *) UxGetContext( UxWidget );
	{OutFileFormat = TIFFGREYLEVEL_FF;}
	UxFileSelectionBoxDialogContext = UxSaveCtx;
}

static	void	activateCB_menu13_p1_b7(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCfileSelectionBoxDialog *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxFileSelectionBoxDialogContext;
	UxFileSelectionBoxDialogContext = UxContext =
			(_UxCfileSelectionBoxDialog *) UxGetContext( UxWidget );
	{OutFileFormat = TIFFCOLOR_FF;}
	UxFileSelectionBoxDialogContext = UxSaveCtx;
}

static	void	activateCB_menu13_p1_b2(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCfileSelectionBoxDialog *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxFileSelectionBoxDialogContext;
	UxFileSelectionBoxDialogContext = UxContext =
			(_UxCfileSelectionBoxDialog *) UxGetContext( UxWidget );
	{OutFileFormat = IGS_FF;}
	UxFileSelectionBoxDialogContext = UxSaveCtx;
}

static	void	activateCB_menu13_p1_b4(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCfileSelectionBoxDialog *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxFileSelectionBoxDialogContext;
	UxFileSelectionBoxDialogContext = UxContext =
			(_UxCfileSelectionBoxDialog *) UxGetContext( UxWidget );
	{OutFileFormat = PX8_FF;}
	UxFileSelectionBoxDialogContext = UxSaveCtx;
}

static	void	activateCB_menu13_p1_b5(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCfileSelectionBoxDialog *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxFileSelectionBoxDialogContext;
	UxFileSelectionBoxDialogContext = UxContext =
			(_UxCfileSelectionBoxDialog *) UxGetContext( UxWidget );
	{OutFileFormat = PX16_FF;}
	UxFileSelectionBoxDialogContext = UxSaveCtx;
}

static	void	activateCB_menu13_p1_b6(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCfileSelectionBoxDialog *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxFileSelectionBoxDialogContext;
	UxFileSelectionBoxDialogContext = UxContext =
			(_UxCfileSelectionBoxDialog *) UxGetContext( UxWidget );
	{OutFileFormat = PX32_FF;}
	UxFileSelectionBoxDialogContext = UxSaveCtx;
}

/*******************************************************************************
       The 'build_' function creates all the widgets
       using the resource values specified in the Property Editor.
*******************************************************************************/

static Widget	_Uxbuild_fileSelectionBoxDialog(void)
{
	Widget		_UxParent;
	MrmType		_UxDummyClass;
	Cardinal	_UxStatus;

	_UxParent = XvgShell;
	if ( _UxParent == NULL )
	{
		_UxParent = UxTopLevel;
	}

	fileSelectionBoxDialog = NULL;
	_UxStatus = MrmFetchWidget( hierarchy,
					"fileSelectionBoxDialog",
					_UxParent,
					&fileSelectionBoxDialog,
					&_UxDummyClass );
	if ( _UxStatus != MrmSUCCESS )
	{
		UxMrmFetchError( hierarchy, "fileSelectionBoxDialog",
				_UxParent, _UxStatus );
		return( (Widget) NULL );
	}


	UxPutContext( fileSelectionBoxDialog, (char *) UxFileSelectionBoxDialogContext );
	FileFormatOM = XtNameToWidget( fileSelectionBoxDialog, "*FileFormatOM" );
	UxPutContext( FileFormatOM, (char *) UxFileSelectionBoxDialogContext );
	menu13_p1 = XtNameToWidget( fileSelectionBoxDialog, "*menu13_p1" );
	UxPutContext( menu13_p1, (char *) UxFileSelectionBoxDialogContext );
	menu13_p1_b1 = XtNameToWidget( menu13_p1, "*menu13_p1_b1" );
	UxPutContext( menu13_p1_b1, (char *) UxFileSelectionBoxDialogContext );
	menu13_p1_b8 = XtNameToWidget( menu13_p1, "*menu13_p1_b8" );
	UxPutContext( menu13_p1_b8, (char *) UxFileSelectionBoxDialogContext );
	menu13_p1_b3 = XtNameToWidget( menu13_p1, "*menu13_p1_b3" );
	UxPutContext( menu13_p1_b3, (char *) UxFileSelectionBoxDialogContext );
	menu13_p1_b7 = XtNameToWidget( menu13_p1, "*menu13_p1_b7" );
	UxPutContext( menu13_p1_b7, (char *) UxFileSelectionBoxDialogContext );
	menu13_p1_b2 = XtNameToWidget( menu13_p1, "*menu13_p1_b2" );
	UxPutContext( menu13_p1_b2, (char *) UxFileSelectionBoxDialogContext );
	menu13_p1_b4 = XtNameToWidget( menu13_p1, "*menu13_p1_b4" );
	UxPutContext( menu13_p1_b4, (char *) UxFileSelectionBoxDialogContext );
	menu13_p1_b5 = XtNameToWidget( menu13_p1, "*menu13_p1_b5" );
	UxPutContext( menu13_p1_b5, (char *) UxFileSelectionBoxDialogContext );
	menu13_p1_b6 = XtNameToWidget( menu13_p1, "*menu13_p1_b6" );
	UxPutContext( menu13_p1_b6, (char *) UxFileSelectionBoxDialogContext );

	XtAddCallback( fileSelectionBoxDialog, XmNdestroyCallback,
		(XtCallbackProc) UxDestroyContextCB,
		(XtPointer) UxFileSelectionBoxDialogContext);

	return ( fileSelectionBoxDialog );
}

/*******************************************************************************
       The following is the 'Interface function' which is the
       external entry point for creating this interface.
       This function should be called from your application or from
       a callback function.
*******************************************************************************/

Widget	create_fileSelectionBoxDialog(void)
{
	Widget                  rtrn;
	_UxCfileSelectionBoxDialog *UxContext;
	static int		_Uxinit = 0;

	UxFileSelectionBoxDialogContext = UxContext =
		(_UxCfileSelectionBoxDialog *) UxNewContext( sizeof(_UxCfileSelectionBoxDialog), B_FALSE );


	if ( ! _Uxinit )
	{
		static MrmRegisterArg	_UxMrmNames[] = {
			{ "cancelCB_fileSelectionBoxDialog", 
				(XtPointer) cancelCB_fileSelectionBoxDialog },
			{ "okCallback_fileSelBoxDialog", 
				(XtPointer) okCallback_fileSelBoxDialog },
			{ "activateCB_menu13_p1_b1", 
				(XtPointer) activateCB_menu13_p1_b1 },
			{ "activateCB_menu13_p1_b8", 
				(XtPointer) activateCB_menu13_p1_b8 },
			{ "activateCB_menu13_p1_b3", 
				(XtPointer) activateCB_menu13_p1_b3 },
			{ "activateCB_menu13_p1_b7", 
				(XtPointer) activateCB_menu13_p1_b7 },
			{ "activateCB_menu13_p1_b2", 
				(XtPointer) activateCB_menu13_p1_b2 },
			{ "activateCB_menu13_p1_b4", 
				(XtPointer) activateCB_menu13_p1_b4 },
			{ "activateCB_menu13_p1_b5", 
				(XtPointer) activateCB_menu13_p1_b5 },
			{ "activateCB_menu13_p1_b6", 
				(XtPointer) activateCB_menu13_p1_b6 }};

		MrmRegisterNamesInHierarchy( hierarchy,
					_UxMrmNames,
					XtNumber(_UxMrmNames) );

		_Uxinit = 1;
	}

	rtrn = _Uxbuild_fileSelectionBoxDialog();

	/* set startup state */
	OutFileFormat = MATLAB_FF;
	
	/* create storage */
	e = mxCreateFull(1, 1, REAL);
	
	/* zap the cancel and help buttons */
	XtUnmanageChild(XmFileSelectionBoxGetChild(UxGetWidget(rtrn), XmDIALOG_HELP_BUTTON));
	return(rtrn);
}

/*******************************************************************************
       END OF FILE
*******************************************************************************/

