
/*******************************************************************************
	XvgShell.c

       Associated Header file: XvgShell.h
       Associated Resource file: XvgShell.rf
*******************************************************************************/

#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <Mrm/MrmPublic.h>
#include <Xm/MenuShell.h>
#include "UxXt.h"

#include <Xm/Scale.h>
#include <Xm/PushBG.h>
#include <Xm/SeparatoG.h>
#include <Xm/ToggleBG.h>
#include <Xm/RowColumn.h>
#include <Xm/LabelG.h>
#include <Xm/Form.h>
#include <Xm/ScrolledW.h>
#include <X11/Shell.h>

/*******************************************************************************
       Includes, Defines, and Global variables from the Declarations Editor:
*******************************************************************************/

#include "Xm/MessageB.h"
#include <values.h>
#include <signal.h>
#ifdef VCAA_CODE
#include <vca_xvg.h>
#endif
#include "include/hourglass.h"
#include "include/hourglassM.h"
#if defined (CMGUI)
#include "data/data_2d_dialog.h"
#endif /* defined (CMGUI) */

#define MAIN_PROGRAM
#include "include/XvgGlobals.h"

/* external declarations */
extern swidget fileSelectionBoxDialog,
#ifndef CMGUI
errorDialog,
#endif
  RawImageShell, LUTShell,
  ProcessedImageShell, HistogramShell, IPOperationsShell, InfoDialog,
  MovieControlsShell, promptDialog, ScrolledWindowDialog, InputDialog,
  RawImageDrawingArea, ProcessedImageDrawingArea, GLShell,
#ifdef ALL_INTERFACES
  ADShell, DAShell, LCRShell, GalvoControlShell, VCAADialog,
#endif
  ColorControlPtTW, FileFormatOM, StackLG, FNameLG, RepsTW;

/* locals that must be not ne declared in the context */
static XImage *LogoImage;
static int LogoWidth, LogoHeight;
#if defined (CMGUI)
/* necessary so we can pick up zoom changes */
extern Widget data_2d_dialog;
#endif /* defined (CMGUI) */

/*******************************************************************************
       The following header file defines the context structure.
*******************************************************************************/

#define CONTEXT_MACRO_ACCESS 1
#include "XvgShell.h"
#undef CONTEXT_MACRO_ACCESS

Widget	XvgShell;
Widget	RawImageTBG;
Widget	ProcessedImageTBG;
Widget	IPOperationsTBG;
Widget	HistogramTBG;
Widget	XYSlicesTBG;
Widget	LUTShellTBG;
Widget	MovieControlsTBG;
Widget	GLShellTBG;
Widget	FilenameLG;
Widget	SizeLG;
Widget	GammaScale;
Widget	LUTMenuOMW;

/*******************************************************************************
Auxiliary code from the Declarations Editor:
*******************************************************************************/

#include "custom/XvgShellAux.c"

/*******************************************************************************
       The following are callback functions.
*******************************************************************************/

static	void	destroyCB_XvgShell(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCXvgShell            *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxXvgShellContext;
	UxXvgShellContext = UxContext =
			(_UxCXvgShell *) UxGetContext( UxWidget );
	{
	  int i;

	  /* if standalone, exit */
#if !defined (NO_XVGMAIN)
	  exit(EXIT_SUCCESS);
#endif

	  /* flag killing in progress */
	  XvgUp = B_FALSE;

	  /* kill all other interfaces */
	  UxDestroyInterface(fileSelectionBoxDialog);
	  UxDestroyInterface(ScrolledWindowDialog);
	  UxDestroyInterface(InfoDialog);
	  UxDestroyInterface(promptDialog);
	  UxDestroyInterface(InputDialog);
#ifndef CMGUI
	  UxDestroyInterface(errorDialog);
#endif
	  if (MovieControlsShellCreated)
	    UxDestroyInterface(MovieControlsShell);
	  if (GLShellCreated)
	    UxDestroyInterface(GLShell);
	  if (LUTShellCreated)
	    UxDestroyInterface(LUTShell);
	  if (RawImageShellCreated)
	    UxDestroyInterface(RawImageShell);
	  if (ProcessedImageShellCreated)
	    UxDestroyInterface(ProcessedImageShell);
	  if (HistogramShellCreated)
	    UxDestroyInterface(HistogramShell);
	  if (IPOperationsShellCreated)
	    UxDestroyInterface(IPOperationsShell);
#ifdef ALL_INTERFACES
	  if (DAShellCreated)
	    UxDestroyInterface(DAShell);
	  if (ADShellCreated)
	    UxDestroyInterface(ADShell);
	  if (GalvoControlShellCreated)
	    UxDestroyInterface(GalvoControlShell);
	  if (LCRShellCreated)
	    UxDestroyInterface(LCRShell);
	  if (UxGetWidget(VCAADialog))
	    UxDestroyInterface(VCAADialog);
#endif

	  /* delayed resource allocation */
	  XtAppAddTimeOut(UxAppContext, 0, XvgDeallocateResources, cd);
	}
	UxXvgShellContext = UxSaveCtx;
}

static	void	valueChangedCB_RawImageTBG(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
  _UxCXvgShell            *UxSaveCtx, *UxContext;
  Widget                  UxWidget = wgt;
  XtPointer               UxClientData = cd;
  XtPointer               UxCallbackArg = cb;
  
  UxSaveCtx = UxXvgShellContext;
  UxXvgShellContext = UxContext =
    (_UxCXvgShell *) UxGetContext( UxWidget );
  {
    if (IsTrue(UxGetSet(RawImageTBG))) {
      if (RawImageShellCreated == B_FALSE) {
	RawImageShell = create_RawImageShell();
      }
      UxPopupInterface(RawImageShell, no_grab);
      UpdateRawWinParms();
    }
    else {
      UxPopdownInterface(RawImageShell);
    }
  }
  UxXvgShellContext = UxSaveCtx;
}

	
static	void	valueChangedCB_ProcImageTBG(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCXvgShell            *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxXvgShellContext;
	UxXvgShellContext = UxContext =
			(_UxCXvgShell *) UxGetContext( UxWidget );
	{
	if (IsTrue(UxGetSet(ProcessedImageTBG))) {
		if (ProcessedImageShellCreated == B_FALSE)
			ProcessedImageShell = create_ProcessedImageShell();
		UxPopupInterface(ProcessedImageShell, no_grab);
		UpdateProcessedWinParms();
		}
	else
		UxPopdownInterface(ProcessedImageShell);
	}
	UxXvgShellContext = UxSaveCtx;
}

static	void	valueChangedCB_IPOperationsTBG(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCXvgShell            *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxXvgShellContext;
	UxXvgShellContext = UxContext =
			(_UxCXvgShell *) UxGetContext( UxWidget );
	{
	if (IsTrue(UxGetSet(IPOperationsTBG))) {
		if (IPOperationsShellCreated == B_FALSE)
			IPOperationsShell = create_IPOperationsShell();
		UxPopupInterface(IPOperationsShell, no_grab);
		}
	else
		UxPopdownInterface(IPOperationsShell);
	
	}
	UxXvgShellContext = UxSaveCtx;
}

static	void	valueChangedCB_HistogramTBG(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCXvgShell            *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxXvgShellContext;
	UxXvgShellContext = UxContext =
			(_UxCXvgShell *) UxGetContext( UxWidget );
	{
	if (IsTrue(UxGetSet(HistogramTBG))) {
		if (HistogramShellCreated == B_FALSE)
			HistogramShell = create_HistogramShell();
		else {
		    Histogram(RawPixels, RawWinHOffset, RawWinVOffset, RawWinWidth,
			RawWinHeight, ImgWidth, RawMax, RawMin, RawHistogram,
			RawHistogramRectangles, &RawMean, &RawSD);
		    Histogram(ProcessedPixels, ProcessedWinHOffset, ProcessedWinVOffset,
			ProcessedWinWidth, ProcessedWinHeight, ImgWidth, ProcMax,
			ProcMin, ProcessedHistogram, ProcessedHistogramRectangles,
			&ProcMean, &ProcSD);
		    DrawHistograms(RawMin, RawMax, ProcMin, ProcMax);
		}
		UxPopupInterface(HistogramShell, no_grab);
		HistogramCursor = xhair;
		XDefineCursor(UxDisplay, XtWindow(UxGetWidget(HistogramShell)), HistogramCursor);
	
	}
	else
		UxPopdownInterface(HistogramShell);
	
	}
	UxXvgShellContext = UxSaveCtx;
}

static	void	valueChangedCB_XYSlicesTBG(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCXvgShell            *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxXvgShellContext;
	UxXvgShellContext = UxContext =
			(_UxCXvgShell *) UxGetContext( UxWidget );
	{
	if (IsTrue(UxGetSet(XYSlicesTBG))) {
	    /* check Raw Image window */
	    if (UxGetWidget(RawImageShell))
		if (XtIsRealized(UxGetWidget(RawImageShell)))
		    RawImageXYSlices(B_TRUE, B_FALSE);
	    /* check Processed Image window */
	    if (UxGetWidget(ProcessedImageShell))
		if (XtIsRealized(UxGetWidget(ProcessedImageShell)))
		    ProcessedImageXYSlices(B_TRUE, B_FALSE);
	}
	else {
	    /* check Raw Image window */
	    if (UxGetWidget(RawImageShell))
		if (XtIsRealized(UxGetWidget(RawImageShell))) {
		    RawImageXYSlices(B_FALSE, B_FALSE);
	        }
	    /* check Processed Image window */
	    if (UxGetWidget(ProcessedImageShell))
		if (XtIsRealized(UxGetWidget(ProcessedImageShell))) {
		    ProcessedImageXYSlices(B_FALSE, B_FALSE);
	            RedrawProcDrawingArea();
	        }
	}
	
	
	}
	UxXvgShellContext = UxSaveCtx;
}

static	void	valueChangedCB_LUTShellTBG(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCXvgShell            *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxXvgShellContext;
	UxXvgShellContext = UxContext =
			(_UxCXvgShell *) UxGetContext( UxWidget );
	{
	if (IsTrue(UxGetSet(LUTShellTBG))) {
	    if (LUTShellCreated == B_FALSE)
		LUTShell = create_LUTShell();
	    UxPopupInterface(LUTShell, no_grab);
	    }
	else
	    UxPopdownInterface(LUTShell);
	
	}
	UxXvgShellContext = UxSaveCtx;
}

static	void	valueChangedCB_MovieControlsTBG(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCXvgShell            *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxXvgShellContext;
	UxXvgShellContext = UxContext =
			(_UxCXvgShell *) UxGetContext( UxWidget );
	{
	if (IsTrue(UxGetSet(MovieControlsTBG))) {
	    if (MovieControlsShellCreated == B_FALSE)
		MovieControlsShell = create_MovieControlsShell();
	    UxPopupInterface(MovieControlsShell, no_grab);
	    }
	else
	    UxPopdownInterface(MovieControlsShell);
	
	}
	UxXvgShellContext = UxSaveCtx;
}

static	void	valueChangedCB_GLShellTBG(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCXvgShell            *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxXvgShellContext;
	UxXvgShellContext = UxContext =
			(_UxCXvgShell *) UxGetContext( UxWidget );
	{
	if (IsTrue(UxGetSet(GLShellTBG))) {
	    if (GLShellCreated == B_FALSE)
		GLShell = create_GLShell();
	    UxPopupInterface(GLShell, no_grab);
	    }
	else
	    UxPopdownInterface(GLShell);
	
	}
	UxXvgShellContext = UxSaveCtx;
}

static	void	activateCB_Resize640x480PBG(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCXvgShell            *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxXvgShellContext;
	UxXvgShellContext = UxContext =
			(_UxCXvgShell *) UxGetContext( UxWidget );
	{
	HGCursor(B_TRUE);
	LoadFile(NULL);
	HGCursor(B_FALSE);
	
	}
	UxXvgShellContext = UxSaveCtx;
}

static	void	exposeCB_LogoDAW(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCXvgShell            *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxXvgShellContext;
	UxXvgShellContext = UxContext =
			(_UxCXvgShell *) UxGetContext( UxWidget );
	{
	  if (LogoFound)
	    XPutImage(UxDisplay, XtWindow((Widget) UxGetWidget(LogoDAW)),
		      gc, LogoImage, 0,
		      0, 0, 0, LogoWidth, LogoHeight);
	}
	UxXvgShellContext = UxSaveCtx;
}

static	void	activateCB_LoadDataSetPBG(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCXvgShell            *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxXvgShellContext;
	UxXvgShellContext = UxContext =
			(_UxCXvgShell *) UxGetContext( UxWidget );
	{
	FileSelectionBoxAction = LOADFILEACTION;
	UxPutSelectionLabelString(fileSelectionBoxDialog, "Input file name:");
	UxPutPattern(fileSelectionBoxDialog, "*");
	UxPopupInterface(fileSelectionBoxDialog, no_grab);
	}
	UxXvgShellContext = UxSaveCtx;
}

static	void	activateCB_LoadIPOpsPBG(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCXvgShell            *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxXvgShellContext;
	UxXvgShellContext = UxContext =
			(_UxCXvgShell *) UxGetContext( UxWidget );
	{
	FileSelectionBoxAction = LOADIPOPSACTION;
	UxPutSelectionLabelString(fileSelectionBoxDialog, "IP Ops command file name:");
	UxPutPattern(fileSelectionBoxDialog, "*.*");
	UxPopupInterface(fileSelectionBoxDialog, no_grab);
	}
	UxXvgShellContext = UxSaveCtx;
}

static	void	activateCB_SaveHistogramPBG(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCXvgShell            *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxXvgShellContext;
	UxXvgShellContext = UxContext =
			(_UxCXvgShell *) UxGetContext( UxWidget );
	{
	FileSelectionBoxAction = SAVEHISTOGRAMACTION;
	UxPutSelectionLabelString(fileSelectionBoxDialog, "MATLAB output file name:");
	UxPutPattern(fileSelectionBoxDialog, "*");
	UxPopupInterface(fileSelectionBoxDialog, no_grab);
	}
	UxXvgShellContext = UxSaveCtx;
}

static	void	activateCB_SaveDataSetPBG(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCXvgShell            *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxXvgShellContext;
	UxXvgShellContext = UxContext =
			(_UxCXvgShell *) UxGetContext( UxWidget );
	{
	XtSetSensitive(UxGetWidget(FileFormatOM), B_TRUE);
	FileSelectionBoxAction = SAVEDATAACTION;
	UxPutSelectionLabelString(fileSelectionBoxDialog, "Output file name:");
	UxPutPattern(fileSelectionBoxDialog, "*");
	UxPopupInterface(fileSelectionBoxDialog, no_grab);
	}
	UxXvgShellContext = UxSaveCtx;
}

static	void	activateCB_SaveIPOpsPBG(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCXvgShell            *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxXvgShellContext;
	UxXvgShellContext = UxContext =
			(_UxCXvgShell *) UxGetContext( UxWidget );
	{
	FileSelectionBoxAction = SAVEIPOPSACTION;
	UxPutSelectionLabelString(fileSelectionBoxDialog, "IP Ops command file name:");
	UxPutPattern(fileSelectionBoxDialog, "*");
	UxPopupInterface(fileSelectionBoxDialog, no_grab);
	}
	UxXvgShellContext = UxSaveCtx;
}

static	void	valueChangedCB_GammaScale(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCXvgShell            *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxXvgShellContext;
	UxXvgShellContext = UxContext =
			(_UxCXvgShell *) UxGetContext( UxWidget );
	{
	if (CurrentCells == GreyLevelCells) {
	  GcFactor = ((double) UxGetValue(GammaScale))/100.0;
	  LoadGreyLevelCells((int) (GcFactor * 127.0));
	  XStoreColors(UxDisplay, cmap, GreyLevelCells, 128);
	}
	else if (CurrentCells == GammaGreyLevelCells) {
	  GcFactor = ((double) UxGetValue(GammaScale))/100.0;
	  LoadGammaGreyLevelCells();
	  XStoreColors(UxDisplay, cmap, GammaGreyLevelCells, 128);
	}
	else {
	  UpdateColorLUT();
	  XStoreColors(UxDisplay, cmap, BRYGMBVMCells, 128);
	}
	
	}
	UxXvgShellContext = UxSaveCtx;
}

static	void	dragCB_GammaScale(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCXvgShell            *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxXvgShellContext;
	UxXvgShellContext = UxContext =
			(_UxCXvgShell *) UxGetContext( UxWidget );
	{
	if (CurrentCells == GreyLevelCells) {
	  GcFactor = ((double) UxGetValue(GammaScale))/100.0;
	  LoadGreyLevelCells((int) (GcFactor*127.0));
	  XStoreColors(UxDisplay, cmap, GreyLevelCells, 128);
	}
	else if (CurrentCells == GammaGreyLevelCells) {
	  GcFactor = ((double) UxGetValue(GammaScale))/100.0;
	  LoadGammaGreyLevelCells();
	  XStoreColors(UxDisplay, cmap, GammaGreyLevelCells, 128);
	}
	else {
	  UpdateColorLUT();
	  XStoreColors(UxDisplay, cmap, BRYGMBVMCells, 128);
	}
	
	}
	UxXvgShellContext = UxSaveCtx;
}

static	void	activateCB_menu1_p4_b1(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCXvgShell            *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxXvgShellContext;
	UxXvgShellContext = UxContext =
			(_UxCXvgShell *) UxGetContext( UxWidget );
	{
	  LoadGreyLevelCells(0);
	  XStoreColors(UxDisplay, cmap, GreyLevelCells, NPAGEDLUTCELLS);
	  CurrentCells = GreyLevelCells;
	  XtSetSensitive(UxGetWidget(GammaScale), B_TRUE);
	  UxPutTitleString(GammaScale, "Wrapping factor");
	  UxPutMaximum(GammaScale, 100);
	  UxPutValue(GammaScale, 0);
	}
	UxXvgShellContext = UxSaveCtx;
}

static	void	activateCB_menu1_p4_b16(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCXvgShell            *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxXvgShellContext;
	UxXvgShellContext = UxContext =
			(_UxCXvgShell *) UxGetContext( UxWidget );
	{
	  XStoreColors(UxDisplay, cmap, GammaGreyLevelCells, NPAGEDLUTCELLS);
	  CurrentCells = GammaGreyLevelCells;
	  XtSetSensitive(UxGetWidget(GammaScale), B_TRUE);
	  UxPutTitleString(GammaScale, "Gamma correction factor");
	  UxPutMaximum(GammaScale, 200);
	  UxPutValue(GammaScale, 100);
	}
	UxXvgShellContext = UxSaveCtx;
}

static	void	activateCB_menu1_p4_b6(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCXvgShell            *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxXvgShellContext;
	UxXvgShellContext = UxContext =
			(_UxCXvgShell *) UxGetContext( UxWidget );
	{
	  CurrentCells = BRYGMBVMCells;
	  XtSetSensitive(UxGetWidget(GammaScale), B_TRUE);
	  UxPutTitleString(GammaScale, "Intensity scale factor");
	  UxPutMaximum(GammaScale, 100);
	  UxPutValue(GammaScale, 100);
	  UpdateColorLUT();
	  XStoreColors(UxDisplay, cmap, BRYGMBVMCells, NPAGEDLUTCELLS);
	}
	UxXvgShellContext = UxSaveCtx;
}

static	void	activateCB_menu1_p4_b5(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCXvgShell            *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxXvgShellContext;
	UxXvgShellContext = UxContext =
			(_UxCXvgShell *) UxGetContext( UxWidget );
	{
	  XStoreColors(UxDisplay, cmap, UserCells, NPAGEDLUTCELLS);
	  CurrentCells = UserCells;
	  XtSetSensitive(UxGetWidget(GammaScale), B_FALSE);
	}
	UxXvgShellContext = UxSaveCtx;
}

static	void	activateCB_menu1_p1_b6(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCXvgShell            *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxXvgShellContext;
	UxXvgShellContext = UxContext =
			(_UxCXvgShell *) UxGetContext( UxWidget );
	{
	int i;
	
	HGCursor(B_TRUE);
	ZoomFactor = 1;
	if (RawImageShellCreated) {
	    ResizeRawImageWidgets(B_FALSE);
	    CreateRawImage();
	}
	if (ProcessedImageShellCreated) {
	    ResizeProcessedImageWidgets(B_FALSE);
	    CreateProcImage();
	}
	HGCursor(B_FALSE);
	
	/* set decimation factor */
	DecimationFactor = 1;
	}
	UxXvgShellContext = UxSaveCtx;
}

static	void	activateCB_menu1_p1_b7(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCXvgShell            *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxXvgShellContext;
	UxXvgShellContext = UxContext =
			(_UxCXvgShell *) UxGetContext( UxWidget );
	{
	HGCursor(B_TRUE);
	ZoomFactor = 2;
	if (RawImageShellCreated) {
	    ResizeRawImageWidgets(B_FALSE);
	    CreateRawImage();
	    }
	if (ProcessedImageShellCreated) {
	    ResizeProcessedImageWidgets(B_FALSE);
	    CreateProcImage();
	    }
	HGCursor(B_FALSE);
	
	/* set decimation factor */
	DecimationFactor = 1;
	}
	UxXvgShellContext = UxSaveCtx;
}

static	void	activateCB_menu1_p1_b8(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCXvgShell            *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxXvgShellContext;
	UxXvgShellContext = UxContext =
			(_UxCXvgShell *) UxGetContext( UxWidget );
	{
	HGCursor(B_TRUE);
	ZoomFactor = 4;
	if (RawImageShellCreated) {
	    ResizeRawImageWidgets(B_FALSE);
	    CreateRawImage();
	    }
	if (ProcessedImageShellCreated) {
	    ResizeProcessedImageWidgets(B_FALSE);
	    CreateProcImage();
	    }
	HGCursor(B_FALSE);
	
	/* set decimation factor */
	DecimationFactor = 1;
	}
	UxXvgShellContext = UxSaveCtx;
}

static	void	activateCB_menu1_p1_b10(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCXvgShell            *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxXvgShellContext;
	UxXvgShellContext = UxContext =
			(_UxCXvgShell *) UxGetContext( UxWidget );
	{
	HGCursor(B_TRUE);
	ZoomFactor = 8;
	if (RawImageShellCreated) {
	    ResizeRawImageWidgets(B_FALSE);
	    CreateRawImage();
	    }
	if (ProcessedImageShellCreated) {
	    ResizeProcessedImageWidgets(B_FALSE);
	    CreateProcImage();
	    }
	HGCursor(B_FALSE);
	
	/* set decimation factor */
	DecimationFactor = 1;
	}
	UxXvgShellContext = UxSaveCtx;
}

static	void	activateCB_menu1_p1_b1(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCXvgShell            *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxXvgShellContext;
	UxXvgShellContext = UxContext =
			(_UxCXvgShell *) UxGetContext( UxWidget );
	{VerboseFlag = B_FALSE;}
	UxXvgShellContext = UxSaveCtx;
}

static	void	activateCB_menu1_p1_b12(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCXvgShell            *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxXvgShellContext;
	UxXvgShellContext = UxContext =
			(_UxCXvgShell *) UxGetContext( UxWidget );
	{VerboseFlag = B_TRUE;}
	UxXvgShellContext = UxSaveCtx;
}

/*******************************************************************************
       The 'build_' function creates all the widgets
       using the resource values specified in the Property Editor.
*******************************************************************************/

static Widget	_Uxbuild_XvgShell(void)
{
	MrmType		_UxDummyClass;
	Cardinal	_UxStatus;


	/* Creation of XvgShell */
	XvgShell = XtVaCreatePopupShell( "XvgShell",
			topLevelShellWidgetClass,
			UxTopLevel,
			XmNallowShellResize,B_TRUE,
#if defined (CMGUI)
			XmNiconName, "IP",
			XmNtitle, "Image Processing",
#else
			XmNiconName, "Xvg",
			XmNtitle, "Xvg",
#endif
/*
			XmNmaxWidth, 400,
			XmNmaxHeight, 750,
*/
			NULL );
	UxPutContext( XvgShell, (char *) UxXvgShellContext );
	XSync(UxDisplay, B_FALSE);

	scrolledWindow1 = NULL;
	_UxStatus = MrmFetchWidget( hierarchy,
					"scrolledWindow1",
					XvgShell,
					&scrolledWindow1,
					&_UxDummyClass );
	if ( _UxStatus != MrmSUCCESS )
	{
		UxMrmFetchError( hierarchy, "scrolledWindow1",
				XvgShell, _UxStatus );
		return( (Widget) NULL );
	}
	XSync(UxDisplay, B_FALSE);

	XtManageChild( scrolledWindow1 );

	UxPutContext( scrolledWindow1, (char *) UxXvgShellContext );
	form1 = XtNameToWidget( scrolledWindow1, "*form1" );
	UxPutContext( form1, (char *) UxXvgShellContext );
	LogoDAW = XtNameToWidget( form1, "*LogoDAW" );
	UxPutContext( LogoDAW, (char *) UxXvgShellContext );
	labelGadget1 = XtNameToWidget( form1, "*labelGadget1" );
	UxPutContext( labelGadget1, (char *) UxXvgShellContext );
	rowColumn1 = XtNameToWidget( form1, "*rowColumn1" );
	UxPutContext( rowColumn1, (char *) UxXvgShellContext );
	RawImageTBG = XtNameToWidget( rowColumn1, "*RawImageTBG" );
	UxPutContext( RawImageTBG, (char *) UxXvgShellContext );
	ProcessedImageTBG = XtNameToWidget( rowColumn1, "*ProcessedImageTBG" );
	UxPutContext( ProcessedImageTBG, (char *) UxXvgShellContext );
	IPOperationsTBG = XtNameToWidget( rowColumn1, "*IPOperationsTBG" );
	UxPutContext( IPOperationsTBG, (char *) UxXvgShellContext );
	rowColumn4 = XtNameToWidget( form1, "*rowColumn4" );
	UxPutContext( rowColumn4, (char *) UxXvgShellContext );
	HistogramTBG = XtNameToWidget( rowColumn4, "*HistogramTBG" );
	UxPutContext( HistogramTBG, (char *) UxXvgShellContext );
	XYSlicesTBG = XtNameToWidget( rowColumn4, "*XYSlicesTBG" );
	UxPutContext( XYSlicesTBG, (char *) UxXvgShellContext );
	LUTShellTBG = XtNameToWidget( rowColumn4, "*LUTShellTBG" );
	UxPutContext( LUTShellTBG, (char *) UxXvgShellContext );
	MovieControlsTBG = XtNameToWidget( rowColumn4, "*MovieControlsTBG" );
	UxPutContext( MovieControlsTBG, (char *) UxXvgShellContext );
	GLShellTBG = XtNameToWidget( rowColumn4, "*GLShellTBG" );
	UxPutContext( GLShellTBG, (char *) UxXvgShellContext );
	separatorGadget1 = XtNameToWidget( form1, "*separatorGadget1" );
	UxPutContext( separatorGadget1, (char *) UxXvgShellContext );
	FilenameLG = XtNameToWidget( form1, "*FilenameLG" );
	UxPutContext( FilenameLG, (char *) UxXvgShellContext );
	SizeLG = XtNameToWidget( form1, "*SizeLG" );
	UxPutContext( SizeLG, (char *) UxXvgShellContext );
	separatorGadget9 = XtNameToWidget( form1, "*separatorGadget9" );
	UxPutContext( separatorGadget9, (char *) UxXvgShellContext );
	labelGadget2 = XtNameToWidget( form1, "*labelGadget2" );
	UxPutContext( labelGadget2, (char *) UxXvgShellContext );
	rowColumn2 = XtNameToWidget( form1, "*rowColumn2" );
	UxPutContext( rowColumn2, (char *) UxXvgShellContext );
	Resize640x480PBG = XtNameToWidget( rowColumn2, "*Resize640x480PBG" );
	UxPutContext( Resize640x480PBG, (char *) UxXvgShellContext );
	LoadDataSetPBG = XtNameToWidget( rowColumn2, "*LoadDataSetPBG" );
	UxPutContext( LoadDataSetPBG, (char *) UxXvgShellContext );
	LoadIPOpsPBG = XtNameToWidget( rowColumn2, "*LoadIPOpsPBG" );
	UxPutContext( LoadIPOpsPBG, (char *) UxXvgShellContext );
	SaveHistogramPBG = XtNameToWidget( rowColumn2, "*SaveHistogramPBG" );
	UxPutContext( SaveHistogramPBG, (char *) UxXvgShellContext );
	SaveDataSetPBG = XtNameToWidget( rowColumn2, "*SaveDataSetPBG" );
	UxPutContext( SaveDataSetPBG, (char *) UxXvgShellContext );
	SaveIPOpsPBG = XtNameToWidget( rowColumn2, "*SaveIPOpsPBG" );
	UxPutContext( SaveIPOpsPBG, (char *) UxXvgShellContext );
	separatorGadget2 = XtNameToWidget( form1, "*separatorGadget2" );
	UxPutContext( separatorGadget2, (char *) UxXvgShellContext );
	labelGadget3 = XtNameToWidget( form1, "*labelGadget3" );
	UxPutContext( labelGadget3, (char *) UxXvgShellContext );
	GammaScale = XtNameToWidget( form1, "*GammaScale" );
	UxPutContext( GammaScale, (char *) UxXvgShellContext );
	rowColumn3 = XtNameToWidget( form1, "*rowColumn3" );
	UxPutContext( rowColumn3, (char *) UxXvgShellContext );
	LUTMenuOMW = XtNameToWidget( rowColumn3, "*LUTMenuOMW" );
	UxPutContext( LUTMenuOMW, (char *) UxXvgShellContext );
	menu1_p4 = XtNameToWidget( rowColumn3, "*menu1_p4" );
	UxPutContext( menu1_p4, (char *) UxXvgShellContext );
	menu1_p4_b1 = XtNameToWidget( menu1_p4, "*menu1_p4_b1" );
	UxPutContext( menu1_p4_b1, (char *) UxXvgShellContext );
	menu1_p4_b8 = XtNameToWidget( menu1_p4, "*menu1_p4_b8" );
	UxPutContext( menu1_p4_b8, (char *) UxXvgShellContext );
	menu1_p4_b16 = XtNameToWidget( menu1_p4, "*menu1_p4_b16" );
	UxPutContext( menu1_p4_b16, (char *) UxXvgShellContext );
	menu1_p4_b17 = XtNameToWidget( menu1_p4, "*menu1_p4_b17" );
	UxPutContext( menu1_p4_b17, (char *) UxXvgShellContext );
	menu1_p4_b6 = XtNameToWidget( menu1_p4, "*menu1_p4_b6" );
	UxPutContext( menu1_p4_b6, (char *) UxXvgShellContext );
	menu1_p4_b7 = XtNameToWidget( menu1_p4, "*menu1_p4_b7" );
	UxPutContext( menu1_p4_b7, (char *) UxXvgShellContext );
	menu1_p4_b5 = XtNameToWidget( menu1_p4, "*menu1_p4_b5" );
	UxPutContext( menu1_p4_b5, (char *) UxXvgShellContext );
	menu1 = XtNameToWidget( rowColumn3, "*menu1" );
	UxPutContext( menu1, (char *) UxXvgShellContext );
	menu1_p1 = XtNameToWidget( rowColumn3, "*menu1_p1" );
	UxPutContext( menu1_p1, (char *) UxXvgShellContext );
	menu1_p1_b6 = XtNameToWidget( menu1_p1, "*menu1_p1_b6" );
	UxPutContext( menu1_p1_b6, (char *) UxXvgShellContext );
	menu1_p1_b4 = XtNameToWidget( menu1_p1, "*menu1_p1_b4" );
	UxPutContext( menu1_p1_b4, (char *) UxXvgShellContext );
	menu1_p1_b7 = XtNameToWidget( menu1_p1, "*menu1_p1_b7" );
	UxPutContext( menu1_p1_b7, (char *) UxXvgShellContext );
	menu1_p1_b5 = XtNameToWidget( menu1_p1, "*menu1_p1_b5" );
	UxPutContext( menu1_p1_b5, (char *) UxXvgShellContext );
	menu1_p1_b8 = XtNameToWidget( menu1_p1, "*menu1_p1_b8" );
	UxPutContext( menu1_p1_b8, (char *) UxXvgShellContext );
	menu1_p1_b9 = XtNameToWidget( menu1_p1, "*menu1_p1_b9" );
	UxPutContext( menu1_p1_b9, (char *) UxXvgShellContext );
	menu1_p1_b10 = XtNameToWidget( menu1_p1, "*menu1_p1_b10" );
	UxPutContext( menu1_p1_b10, (char *) UxXvgShellContext );
	menu13 = XtNameToWidget( rowColumn3, "*menu13" );
	UxPutContext( menu13, (char *) UxXvgShellContext );
	menu1_p3 = XtNameToWidget( rowColumn3, "*menu1_p3" );
	UxPutContext( menu1_p3, (char *) UxXvgShellContext );
	menu1_p1_b1 = XtNameToWidget( menu1_p3, "*menu1_p1_b1" );
	UxPutContext( menu1_p1_b1, (char *) UxXvgShellContext );
	menu1_p1_b12 = XtNameToWidget( menu1_p3, "*menu1_p1_b12" );
	UxPutContext( menu1_p1_b12, (char *) UxXvgShellContext );
	XSync(UxDisplay, B_FALSE);

	XtAddCallback( XvgShell, XmNdestroyCallback,
		      (XtCallbackProc) destroyCB_XvgShell,
		      (XtPointer) UxXvgShellContext);
	
	return ( XvgShell );
}

/*******************************************************************************
       The following is the 'Interface function' which is the
       external entry point for creating this interface.
       This function should be called from your application or from
       a callback function.
*******************************************************************************/

Widget	create_XvgShell( int _Uxargc, char **_Uxargv )
{
	Widget                  rtrn;
	_UxCXvgShell            *UxContext;
	static int		_Uxinit = 0;

	UxXvgShellContext = UxContext =
		(_UxCXvgShell *) UxNewContext( sizeof(_UxCXvgShell), B_FALSE );

	argc = _Uxargc;
	argv = _Uxargv;

	if ( ! _Uxinit )
	{
		static MrmRegisterArg	_UxMrmNames[] = {
			{ "valueChangedCB_RawImageTBG", 
				(XtPointer) valueChangedCB_RawImageTBG },
			{ "valueChangedCB_ProcImageTBG", 
				(XtPointer) valueChangedCB_ProcImageTBG },
			{ "valueChangedCB_IPOperationsTBG", 
				(XtPointer) valueChangedCB_IPOperationsTBG },
			{ "valueChangedCB_HistogramTBG", 
				(XtPointer) valueChangedCB_HistogramTBG },
			{ "valueChangedCB_XYSlicesTBG", 
				(XtPointer) valueChangedCB_XYSlicesTBG },
			{ "valueChangedCB_LUTShellTBG", 
				(XtPointer) valueChangedCB_LUTShellTBG },
			{ "valueChangedCB_MovieControlsTBG", 
				(XtPointer) valueChangedCB_MovieControlsTBG },
			{ "valueChangedCB_GLShellTBG", 
				(XtPointer) valueChangedCB_GLShellTBG },
			{ "activateCB_Resize640x480PBG", 
				(XtPointer) activateCB_Resize640x480PBG },
			{ "exposeCB_LogoDAW", 
				(XtPointer) exposeCB_LogoDAW },
			{ "activateCB_LoadDataSetPBG", 
				(XtPointer) activateCB_LoadDataSetPBG },
			{ "activateCB_LoadIPOpsPBG", 
				(XtPointer) activateCB_LoadIPOpsPBG },
			{ "activateCB_SaveHistogramPBG", 
				(XtPointer) activateCB_SaveHistogramPBG },
			{ "activateCB_SaveDataSetPBG", 
				(XtPointer) activateCB_SaveDataSetPBG },
			{ "activateCB_SaveIPOpsPBG", 
				(XtPointer) activateCB_SaveIPOpsPBG },
			{ "valueChangedCB_GammaScale", 
				(XtPointer) valueChangedCB_GammaScale },
			{ "dragCB_GammaScale", 
				(XtPointer) dragCB_GammaScale },
			{ "activateCB_menu1_p4_b1", 
				(XtPointer) activateCB_menu1_p4_b1 },
			{ "activateCB_menu1_p4_b16", 
				(XtPointer) activateCB_menu1_p4_b16 },
			{ "activateCB_menu1_p4_b6", 
				(XtPointer) activateCB_menu1_p4_b6 },
			{ "activateCB_menu1_p4_b5", 
				(XtPointer) activateCB_menu1_p4_b5 },
			{ "activateCB_menu1_p1_b6", 
				(XtPointer) activateCB_menu1_p1_b6 },
			{ "activateCB_menu1_p1_b7", 
				(XtPointer) activateCB_menu1_p1_b7 },
			{ "activateCB_menu1_p1_b8", 
				(XtPointer) activateCB_menu1_p1_b8 },
			{ "activateCB_menu1_p1_b10", 
				(XtPointer) activateCB_menu1_p1_b10 },
			{ "activateCB_menu1_p1_b1", 
				(XtPointer) activateCB_menu1_p1_b1 },
			{ "activateCB_menu1_p1_b12", 
				(XtPointer) activateCB_menu1_p1_b12 }};

		MrmRegisterNamesInHierarchy( hierarchy,
					_UxMrmNames,
					XtNumber(_UxMrmNames) );

		_Uxinit = 1;
	}

	rtrn = _Uxbuild_XvgShell();
	if (InteractiveFlag)
	  init_state(rtrn, argc, argv);
	return(rtrn);
}

/*******************************************************************************
       END OF FILE
*******************************************************************************/

