
/*******************************************************************************
	HistogramShell.c

       Associated Header file: HistogramShell.h
       Associated Resource file: HistogramShell.rf
*******************************************************************************/

#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <Mrm/MrmPublic.h>
#include <Xm/MenuShell.h>
#include "UxXt.h"

#include <Xm/Scale.h>
#include <Xm/SeparatoG.h>
#include <Xm/PushBG.h>
#include <Xm/LabelG.h>
#include <Xm/DrawingA.h>
#include <Xm/Frame.h>
#include <Xm/Form.h>
#include <X11/Shell.h>

/*******************************************************************************
       Includes, Defines, and Global variables from the Declarations Editor:
*******************************************************************************/

#include "include/XvgGlobals.h"

/* globals */
static XImage *RawColorBarImageXObj, *ProcessedColorBarImageXObj;
static char RawColorBarData[30*512], ProcessedColorBarData[30*512];

/* externals */
extern swidget HistogramTBG, ProcessedImageDrawingArea;


/*******************************************************************************
       The following header file defines the context structure.
*******************************************************************************/

#define CONTEXT_MACRO_ACCESS 1
#include "HistogramShell.h"
#undef CONTEXT_MACRO_ACCESS

Widget	HistogramShell;
Widget	RawColorBarDA;
Widget	ProcessedColorBarDA;

/*******************************************************************************
Auxiliary code from the Declarations Editor:
*******************************************************************************/

void LoadProcessedColorBar(void)
{
  int i, j, low, high;

  /* compute color bar boundaries */
  low = 512.0 * ((double) (UxGetValue(MinScaleW) - HIST_SCALE_MIN)) / ((double) (HIST_SCALE_MAX - HIST_SCALE_MIN));
  high = 512.0 * ((double) (UxGetValue(MaxScaleW) - HIST_SCALE_MIN)) / ((double) (HIST_SCALE_MAX - HIST_SCALE_MIN));
  for (i = 0; i < 30; i++) {
    for (j = 0; j < low; j++)
  	ProcessedColorBarData[i*512 + j] = CurrentCells[0].pixel;
    for (; j < high; j++)
	ProcessedColorBarData[i*512 + j] = CurrentCells[(int) (((double) (j-low)) / ((double) (high-low)) * 127.0)].pixel;
    for (; j < 512; j++)
	ProcessedColorBarData[i*512 + j] = CurrentCells[127].pixel;
  }
}


static void SetMinScaleLabel(void)
{
char b[512];

sprintf(b, "Processed image LUT dynamic range minimum data value : %.2e  ",
	ProcMin + (((double) (UxGetValue(MinScaleW) - HIST_SCALE_MIN)) * (ProcMax - ProcMin)
	/ ((double) (HIST_SCALE_MAX - HIST_SCALE_MIN)) ));
UxPutTitleString(MinScaleW, b);
}

static void SetMaxScaleLabel(void)
{
char b[512];

sprintf(b, "Processed image LUT dynamic range maximum data value : %.2e  ",
	ProcMin + (((double) (UxGetValue(MaxScaleW) - HIST_SCALE_MIN)) * (ProcMax - ProcMin)
	/ ((double) (HIST_SCALE_MAX - HIST_SCALE_MIN)) ));
UxPutTitleString(MaxScaleW, b);
}

void DrawHistograms(double RawMin, double RawMax, double ProcMin, double ProcMax)
{
    char s[512];

    /* Drawing crashes if window not crated */
    if (HistogramShellCreated != B_TRUE)
	return;

    /* fill the labels */
    SetMinScaleLabel();
    SetMaxScaleLabel();
    sprintf(s, "%e", RawMin);
    UxPutLabelString(RawMinLG, s);
    sprintf(s, "%e", RawMax);
    UxPutLabelString(RawMaxLG, s);
    sprintf(s, "Mean : %e", RawMean);
    UxPutLabelString(RawMeanLG, s);
    sprintf(s, "SD : %e", RawSD);
    UxPutLabelString(RawSDLG, s);

    sprintf(s, "%e", ProcMin);
    UxPutLabelString(ProcMinLG, s);
    sprintf(s, "%e", ProcMax);
    UxPutLabelString(ProcMaxLG, s);
    sprintf(s, "Mean : %e", ProcMean);
    UxPutLabelString(ProcMeanLG, s);
    sprintf(s, "SD : %e", ProcSD);
    UxPutLabelString(ProcSDLG, s);

    /* draw the histograms */
    XFillRectangle(UxDisplay, XtWindow(UxGetWidget(RawHistogramDA)), gc_BlackWhite, 0, 0, 512, 100);
    XFillRectangle(UxDisplay, XtWindow(UxGetWidget(ProcessedHistogramDA)), gc_BlackWhite, 0, 0, 512, 100);
    XDrawRectangles(UxDisplay, XtWindow(UxGetWidget(RawHistogramDA)), gc_RedBlack, RawHistogramRectangles, 256);
    XDrawRectangles(UxDisplay, XtWindow(UxGetWidget(ProcessedHistogramDA)), gc_RedBlack, ProcessedHistogramRectangles, 256);

    /* draw the color bars */
    XPutImage(UxDisplay, XtWindow(UxGetWidget(RawColorBarDA)), gc, RawColorBarImageXObj, 0, 0, 0, 0, 512, 30);
    XPutImage(UxDisplay, XtWindow(UxGetWidget(ProcessedColorBarDA)), gc, ProcessedColorBarImageXObj, 0, 0, 0, 0, 512, 30);
}

/*******************************************************************************
       The following are Action functions.
*******************************************************************************/

static	void	action_HistogramInfoDraw(
			Widget wgt, 
			XEvent *ev, 
			String *parm, 
			Cardinal *p_UxNumParams)
{
	Cardinal		UxNumParams = *p_UxNumParams;
	_UxCHistogramShell      *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XEvent                  *UxEvent = ev;
	String                  *UxParams = parm;

	UxSaveCtx = UxHistogramShellContext;
	UxHistogramShellContext = UxContext =
			(_UxCHistogramShell *) UxGetContext( UxWidget );
	{
	    char b[512];
	
	    sprintf(b, "Raw data : %6d / %6d pixels in [%.3e , %.3e[",
		RawHistogram[(UxEvent->xmotion.x)/2], ImgWidth*ImgHeight,
		(((double)(UxEvent->xmotion.x/2))/255.0*(RawMax - RawMin)) + RawMin,
	 	(((double)(UxEvent->xmotion.x/2 + 1))/255.0*(RawMax - RawMin)) + RawMin);
	    UxPutLabelString(RawInfoLG, b);
	    sprintf(b, "Processed Data : %6d / %6d pixels in [%.3e , %.3e[",
		ProcessedHistogram[(UxEvent->xmotion.x)/2], ImgWidth*ImgHeight,
		(((double)(UxEvent->xmotion.x/2))/255.0*(ProcMax - ProcMin)) + ProcMin,
	 	(((double)(UxEvent->xmotion.x/2 + 1))/255.0*(ProcMax - ProcMin)) + ProcMin);
	    UxPutLabelString(ProcInfoLG, b);
	}
	UxHistogramShellContext = UxSaveCtx;
}

/*******************************************************************************
       The following are callback functions.
*******************************************************************************/

static	void	destroyCB_HistogramShell(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCHistogramShell      *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxHistogramShellContext;
	UxHistogramShellContext = UxContext =
			(_UxCHistogramShell *) UxGetContext( UxWidget );
	{
	  if (XvgUp)
	    UxPutSet(HistogramTBG, "false");
	  HistogramShellCreated = B_FALSE;
	  if (VerboseFlag)
	    printf("XVG HistogramShell destroyed.\n");
	}
	UxHistogramShellContext = UxSaveCtx;
}

static	void	exposeCB_RawHistogramDA(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCHistogramShell      *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxHistogramShellContext;
	UxHistogramShellContext = UxContext =
			(_UxCHistogramShell *) UxGetContext( UxWidget );
	{DrawHistograms(RawMin, RawMax, ProcMin, ProcMax);}
	UxHistogramShellContext = UxSaveCtx;
}

static	void	exposeCB_ProcessedHistogramDA(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCHistogramShell      *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxHistogramShellContext;
	UxHistogramShellContext = UxContext =
			(_UxCHistogramShell *) UxGetContext( UxWidget );
	{DrawHistograms(RawMin, RawMax, ProcMin, ProcMax);}
	UxHistogramShellContext = UxSaveCtx;
}

static	void	exposeCB_RawColorBarDA(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCHistogramShell      *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxHistogramShellContext;
	UxHistogramShellContext = UxContext =
			(_UxCHistogramShell *) UxGetContext( UxWidget );
	{
	XPutImage(UxDisplay, XtWindow(UxGetWidget(RawColorBarDA)), gc, RawColorBarImageXObj, 0, 0, 0, 0, 512, 30);
	
	}
	UxHistogramShellContext = UxSaveCtx;
}

static	void	exposeCB_ProcessedColorBarDA(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCHistogramShell      *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxHistogramShellContext;
	UxHistogramShellContext = UxContext =
			(_UxCHistogramShell *) UxGetContext( UxWidget );
	XPutImage(UxDisplay, XtWindow(UxGetWidget(ProcessedColorBarDA)), gc, ProcessedColorBarImageXObj, 0, 0, 0, 0, 512, 30);
	UxHistogramShellContext = UxSaveCtx;
}

static	void	activateCB_pushButtonGadget5(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCHistogramShell      *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxHistogramShellContext;
	UxHistogramShellContext = UxContext =
			(_UxCHistogramShell *) UxGetContext( UxWidget );
	{
	Histogram(RawPixels, RawWinHOffset, RawWinVOffset, RawWinWidth,
		RawWinHeight, ImgWidth, RawMax, RawMin, RawHistogram,
		RawHistogramRectangles, &RawMean, &RawSD);
	Histogram(ProcessedPixels, ProcessedWinHOffset, ProcessedWinVOffset,
		ProcessedWinWidth, ProcessedWinHeight, ImgWidth, ProcMax,
		ProcMin, ProcessedHistogram, ProcessedHistogramRectangles,
		&ProcMean, &ProcSD);
	DrawHistograms(RawMin, RawMax, ProcMin, ProcMax);
	
	}
	UxHistogramShellContext = UxSaveCtx;
}

static	void	valueChangedCB_MinScaleW(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCHistogramShell      *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxHistogramShellContext;
	UxHistogramShellContext = UxContext =
			(_UxCHistogramShell *) UxGetContext( UxWidget );
	{
	/* make ceryain that new values have been loaded */
	XSync(UxDisplay, B_FALSE);
	
	/* check new value against Maximum */
	if (UxGetValue(MinScaleW) >= MaxScaleVal) {
	    UxPutValue(MinScaleW, HIST_SCALE_MIN);
	    return;
	}
	
	/* update the color bar */
	LoadProcessedColorBar();
	XPutImage(UxDisplay, XtWindow(UxGetWidget(ProcessedColorBarDA)), gc, ProcessedColorBarImageXObj, 0, 0, 0, 0, 512, 30);
	
	/* check for initial conditions */
	if (ProcessedPixelsX == NULL)
	    return;
	
	/* Check Ok, update new value of MinScaleVal and change image to new mapping */
	HGCursor(B_TRUE);
	SetMinScaleLabel();
	MinScaleVal = UxGetValue(MinScaleW);
	RealtoX(ProcessedPixels, ProcessedPixelsX, ImgHeight, ImgWidth,
		ProcMax, ProcMin, MaxScaleVal, MinScaleVal, ZoomFactor);
	RedrawProcDrawingArea();
	HGCursor(B_FALSE);
	
	
	}
	UxHistogramShellContext = UxSaveCtx;
}

static	void	dragCB_MinScaleW(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCHistogramShell      *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxHistogramShellContext;
	UxHistogramShellContext = UxContext =
			(_UxCHistogramShell *) UxGetContext( UxWidget );
	{
	SetMinScaleLabel();
	}
	UxHistogramShellContext = UxSaveCtx;
}

static	void	valueChangedCB_MaxScaleW(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCHistogramShell      *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxHistogramShellContext;
	UxHistogramShellContext = UxContext =
			(_UxCHistogramShell *) UxGetContext( UxWidget );
	{
	/* make ceryain that new values have been loaded */
	XSync(UxDisplay, B_FALSE);
	
	/* check new value against Minimum */
	if (UxGetValue(MaxScaleW) <= MinScaleVal) {
	    UxPutValue(MaxScaleW, HIST_SCALE_MAX);
	    return;
	}
	
	/* update the color bar */
	LoadProcessedColorBar();
	XPutImage(UxDisplay, XtWindow(UxGetWidget(ProcessedColorBarDA)), gc, ProcessedColorBarImageXObj, 0, 0, 0, 0, 512, 30);
	
	/* check for initial conditions */
	if (ProcessedPixelsX == NULL)
	    return;
	
	/* Check Ok, update new value of MaxScaleVal and change image to new mapping */
	HGCursor(B_TRUE);
	SetMaxScaleLabel();
	MaxScaleVal = UxGetValue(MaxScaleW);
	RealtoX(ProcessedPixels, ProcessedPixelsX, ImgHeight, ImgWidth,
		ProcMax, ProcMin, MaxScaleVal, MinScaleVal, ZoomFactor);
	RedrawProcDrawingArea();
	HGCursor(B_FALSE);
	
	
	}
	UxHistogramShellContext = UxSaveCtx;
}

static	void	dragCB_MaxScaleW(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCHistogramShell      *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxHistogramShellContext;
	UxHistogramShellContext = UxContext =
			(_UxCHistogramShell *) UxGetContext( UxWidget );
	{
	SetMaxScaleLabel();
	}
	UxHistogramShellContext = UxSaveCtx;
}

/*******************************************************************************
       The 'build_' function creates all the widgets
       using the resource values specified in the Property Editor.
*******************************************************************************/

static Widget	_Uxbuild_HistogramShell(void)
{
	MrmType		_UxDummyClass;
	Cardinal	_UxStatus;


	/* Creation of HistogramShell */
	HistogramShell = XtVaCreatePopupShell( "HistogramShell",
			topLevelShellWidgetClass,
			UxTopLevel,
			XmNx, 442,
			XmNy, 170,
			XmNwidth, 560,
			XmNheight, 690,
			XmNiconName, "Histograms",
			XmNtitle, "Histograms",
			NULL );
	XtAddCallback( HistogramShell, XmNdestroyCallback,
		(XtCallbackProc) destroyCB_HistogramShell,
		(XtPointer) UxHistogramShellContext );

	UxPutContext( HistogramShell, (char *) UxHistogramShellContext );

	form4 = NULL;
	_UxStatus = MrmFetchWidget( hierarchy,
					"form4",
					HistogramShell,
					&form4,
					&_UxDummyClass );
	if ( _UxStatus != MrmSUCCESS )
	{
		UxMrmFetchError( hierarchy, "form4",
				HistogramShell, _UxStatus );
		return( (Widget) NULL );
	}

	XtManageChild( form4 );

	UxPutContext( form4, (char *) UxHistogramShellContext );
	frame4 = XtNameToWidget( form4, "*frame4" );
	UxPutContext( frame4, (char *) UxHistogramShellContext );
	RawHistogramDA = XtNameToWidget( frame4, "*RawHistogramDA" );
	UxPutContext( RawHistogramDA, (char *) UxHistogramShellContext );
	frame5 = XtNameToWidget( form4, "*frame5" );
	UxPutContext( frame5, (char *) UxHistogramShellContext );
	ProcessedHistogramDA = XtNameToWidget( frame5, "*ProcessedHistogramDA" );
	UxPutContext( ProcessedHistogramDA, (char *) UxHistogramShellContext );
	labelGadget7 = XtNameToWidget( form4, "*labelGadget7" );
	UxPutContext( labelGadget7, (char *) UxHistogramShellContext );
	labelGadget8 = XtNameToWidget( form4, "*labelGadget8" );
	UxPutContext( labelGadget8, (char *) UxHistogramShellContext );
	ProcMaxLG = XtNameToWidget( form4, "*ProcMaxLG" );
	UxPutContext( ProcMaxLG, (char *) UxHistogramShellContext );
	ProcMinLG = XtNameToWidget( form4, "*ProcMinLG" );
	UxPutContext( ProcMinLG, (char *) UxHistogramShellContext );
	RawMaxLG = XtNameToWidget( form4, "*RawMaxLG" );
	UxPutContext( RawMaxLG, (char *) UxHistogramShellContext );
	RawMinLG = XtNameToWidget( form4, "*RawMinLG" );
	UxPutContext( RawMinLG, (char *) UxHistogramShellContext );
	frame6 = XtNameToWidget( form4, "*frame6" );
	UxPutContext( frame6, (char *) UxHistogramShellContext );
	RawColorBarDA = XtNameToWidget( frame6, "*RawColorBarDA" );
	UxPutContext( RawColorBarDA, (char *) UxHistogramShellContext );
	frame10 = XtNameToWidget( form4, "*frame10" );
	UxPutContext( frame10, (char *) UxHistogramShellContext );
	ProcessedColorBarDA = XtNameToWidget( frame10, "*ProcessedColorBarDA" );
	UxPutContext( ProcessedColorBarDA, (char *) UxHistogramShellContext );
	pushButtonGadget5 = XtNameToWidget( form4, "*pushButtonGadget5" );
	UxPutContext( pushButtonGadget5, (char *) UxHistogramShellContext );
	separatorGadget5 = XtNameToWidget( form4, "*separatorGadget5" );
	UxPutContext( separatorGadget5, (char *) UxHistogramShellContext );
	RawInfoLG = XtNameToWidget( form4, "*RawInfoLG" );
	UxPutContext( RawInfoLG, (char *) UxHistogramShellContext );
	ProcInfoLG = XtNameToWidget( form4, "*ProcInfoLG" );
	UxPutContext( ProcInfoLG, (char *) UxHistogramShellContext );
	separatorGadget6 = XtNameToWidget( form4, "*separatorGadget6" );
	UxPutContext( separatorGadget6, (char *) UxHistogramShellContext );
	MinScaleW = XtNameToWidget( form4, "*MinScaleW" );
	UxPutContext( MinScaleW, (char *) UxHistogramShellContext );
	MaxScaleW = XtNameToWidget( form4, "*MaxScaleW" );
	UxPutContext( MaxScaleW, (char *) UxHistogramShellContext );
	separatorGadget14 = XtNameToWidget( form4, "*separatorGadget14" );
	UxPutContext( separatorGadget14, (char *) UxHistogramShellContext );
	RawSDLG = XtNameToWidget( form4, "*RawSDLG" );
	UxPutContext( RawSDLG, (char *) UxHistogramShellContext );
	RawMeanLG = XtNameToWidget( form4, "*RawMeanLG" );
	UxPutContext( RawMeanLG, (char *) UxHistogramShellContext );
	ProcSDLG = XtNameToWidget( form4, "*ProcSDLG" );
	UxPutContext( ProcSDLG, (char *) UxHistogramShellContext );
	ProcMeanLG = XtNameToWidget( form4, "*ProcMeanLG" );
	UxPutContext( ProcMeanLG, (char *) UxHistogramShellContext );

	XtAddCallback( HistogramShell, XmNdestroyCallback,
		(XtCallbackProc) UxDestroyContextCB,
		(XtPointer) UxHistogramShellContext);

	return ( HistogramShell );
}

/*******************************************************************************
       The following is the 'Interface function' which is the
       external entry point for creating this interface.
       This function should be called from your application or from
       a callback function.
*******************************************************************************/

Widget	create_HistogramShell(void)
{
	Widget                  rtrn;
	_UxCHistogramShell      *UxContext;
	static int		_Uxinit = 0;

	UxHistogramShellContext = UxContext =
		(_UxCHistogramShell *) UxNewContext( sizeof(_UxCHistogramShell), B_FALSE );


	if ( ! _Uxinit )
	{
		static MrmRegisterArg	_UxMrmNames[] = {
			{ "destroyCB_HistogramShell", 
				(XtPointer) destroyCB_HistogramShell },
			{ "exposeCB_RawHistogramDA", 
				(XtPointer) exposeCB_RawHistogramDA },
			{ "exposeCB_ProcessedHistogramDA", 
				(XtPointer) exposeCB_ProcessedHistogramDA },
			{ "exposeCB_RawColorBarDA", 
				(XtPointer) exposeCB_RawColorBarDA },
			{ "exposeCB_ProcessedColorBarDA", 
				(XtPointer) exposeCB_ProcessedColorBarDA },
			{ "activateCB_pushButtonGadget5", 
				(XtPointer) activateCB_pushButtonGadget5 },
			{ "valueChangedCB_MinScaleW", 
				(XtPointer) valueChangedCB_MinScaleW },
			{ "dragCB_MinScaleW", 
				(XtPointer) dragCB_MinScaleW },
			{ "valueChangedCB_MaxScaleW", 
				(XtPointer) valueChangedCB_MaxScaleW },
			{ "dragCB_MaxScaleW", 
				(XtPointer) dragCB_MaxScaleW }};

		static XtActionsRec	_Uxactions[] = {
			{ "HistogramInfoDraw", (XtActionProc) action_HistogramInfoDraw }};

		XtAppAddActions( UxAppContext,
				_Uxactions,
				XtNumber(_Uxactions) );

		MrmRegisterNamesInHierarchy( hierarchy,
					_UxMrmNames,
					XtNumber(_UxMrmNames) );

		_Uxinit = 1;
	}

	rtrn = _Uxbuild_HistogramShell();

	/* create and fill the colorbar images */
	RawColorBarImageXObj = XCreateImage(UxDisplay, XDefaultVisual(UxDisplay, UxScreen), 8,
		ZPixmap, 0, RawColorBarData, 512, 30, 8, 0);
	ProcessedColorBarImageXObj = XCreateImage(UxDisplay, XDefaultVisual(UxDisplay, UxScreen), 8,
		ZPixmap, 0, ProcessedColorBarData, 512, 30, 8, 0);
	for (hi = 0; hi < 30; hi++)
	  for (hj = 0; hj < 512; hj++) {
		RawColorBarData[hi*512 + hj] = CurrentCells[hj >> 2].pixel;
		ProcessedColorBarData[hi*512 + hj] = CurrentCells[hj >> 2].pixel;
	  }
	
	/* Compute and draw the histograms */
	Histogram(RawPixels, RawWinHOffset, RawWinVOffset, RawWinWidth,
		RawWinHeight, ImgWidth, RawMax, RawMin, RawHistogram,
		RawHistogramRectangles, &RawMean, &RawSD);
	Histogram(ProcessedPixels, ProcessedWinHOffset, ProcessedWinVOffset,
		ProcessedWinWidth, ProcessedWinHeight, ImgWidth, ProcMax,
		ProcMin, ProcessedHistogram, ProcessedHistogramRectangles,
		&ProcMean, &ProcSD);
	DrawHistograms(RawMin, RawMax, ProcMin, ProcMax);
	
	/* initialisations */
	HistogramShellCreated = B_TRUE;
	
	/* return */
	return(rtrn);
}

/*******************************************************************************
       END OF FILE
*******************************************************************************/

