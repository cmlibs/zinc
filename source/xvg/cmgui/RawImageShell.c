
/*******************************************************************************
	RawImageShell.c

       Associated Header file: RawImageShell.h
       Associated Resource file: RawImageShell.rf
*******************************************************************************/

#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <Mrm/MrmPublic.h>
#include <Xm/MenuShell.h>
#include "UxXt.h"

#include <Xm/LabelG.h>
#include <Xm/Frame.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/DrawingA.h>
#include <Xm/ScrolledW.h>
#include <Xm/Form.h>
#include <X11/Shell.h>

/*******************************************************************************
       Includes, Defines, and Global variables from the Declarations Editor:
*******************************************************************************/

#include "include/XvgGlobals.h"

/* externals */
extern swidget RawImageTBG, XYSlicesTBG;

/* forward declarations */
static void	exposeCB_RawImageDrawingArea(Widget wgt, XtPointer cd, XtPointer cb);


/*******************************************************************************
       The following header file defines the context structure.
*******************************************************************************/

#define CONTEXT_MACRO_ACCESS 1
#include "RawImageShell.h"
#undef CONTEXT_MACRO_ACCESS

/*******************************************************************************
       The following function is an event-handler for posting menus.
*******************************************************************************/

static void	_UxRawImageShellMenuPost(
			Widget wgt, 
			XtPointer client_data, 
			XEvent *event)
{
	Widget	menu = (Widget) client_data;
	int 	which_button;

	XtVaGetValues( menu, XmNwhichButton, &which_button, NULL );

	if ( event->xbutton.button == which_button )
	{
		XmMenuPosition( menu, (XButtonPressedEvent *) event );
		XtManageChild( menu );
	}
}

Widget	RawImageShell;
Widget	RawImageDrawingArea;

/*******************************************************************************
Auxiliary code from the Declarations Editor:
*******************************************************************************/

/* indirect call back calling sequence */
void RawExposeCB(Widget w, XtPointer p1, XtPointer p2)
{
    exposeCB_RawImageDrawingArea(w, p1, p2);
}

/* scrollbar callbacks */
void VSBCallback(Widget w, XtPointer p1, XtPointer p2)
{
    XtVaGetValues(RawVsbar, XmNvalue, &RawWinVOffset, NULL);
}
void HSBCallback(Widget w, XtPointer p1, XtPointer p2)
{
    XtVaGetValues(RawHsbar, XmNvalue, &RawWinHOffset, NULL);
}

/* set window size to 640x480 */
void RawSetSizeTo640x480(void)
{
  UxPutHeight(RawImageShell, 480 + SCROLL_BAR_THICKNESS);
  UxPutWidth(RawImageShell, 640 + SCROLL_BAR_THICKNESS);
}

/* Update window size parameters */
void UpdateRawWinParms(void)
{
  if (IsTrue(UxGetSet(RawImageTBG))) {
    XtVaGetValues(RawVsbar, XmNvalue, &RawWinVOffset,
		  XmNsliderSize, &RawWinHeight, NULL);
    XtVaGetValues(RawHsbar, XmNvalue, &RawWinHOffset,
		  XmNsliderSize, &RawWinWidth, NULL);
    RawVVisFrac = ((double) RawWinHeight) / ((double) (ImgHeight*ZoomFactor));
    RawHVisFrac = ((double) RawWinWidth) / ((double) (ImgWidth*ZoomFactor));
  }
}

/* slice window routines */
void RawImageXYSlices(boolean_t ToggleFlag, boolean_t NewFlag)
{
  int height, width;

  /* compute current window size */
  height = ((double) (ImgHeight*ZoomFactor))*RawVVisFrac;
  width = ((double) (ImgWidth*ZoomFactor))*RawHVisFrac;

  /* delay the updates */
  UxDelayUpdate(RawImageShell);

  if (ToggleFlag) {
    /* enable RawImage slice windows */
    UxPutMaxHeight(RawImageShell,
		   (ImgHeight*ZoomFactor)
		   + SCROLL_BAR_THICKNESS + SLICE_WINDOW_WIDTH);
    UxPutMaxWidth(RawImageShell,
		  (ImgWidth*ZoomFactor)
		  + SCROLL_BAR_THICKNESS + SLICE_WINDOW_WIDTH);
    XSync(UxDisplay, B_FALSE);
    if (VerboseFlag)
      printf("RawImageXYSlices() : Reset Max values...\n");
    if (frame1 && XtIsRealized(frame1)
	&& frame2 && XtIsRealized(frame2)
	&& frame3 && XtIsRealized(frame3)) {
      UxMap(frame1);
      UxMap(frame2);
      UxMap(frame3);
    }
    XSync(UxDisplay, B_FALSE);
    if (VerboseFlag)
      printf("RawImageXYSlices() : Mapped frames...\n");
    UxPutBottomOffset(RawImageScrolledWindow, SLICE_WINDOW_WIDTH + 2);
    UxPutRightOffset(RawImageScrolledWindow, SLICE_WINDOW_WIDTH + 2);
    if (NewFlag) {
	UxPutHeight(RawImageShell,
		    height + SCROLL_BAR_THICKNESS + SLICE_WINDOW_WIDTH);
	UxPutWidth(RawImageShell,
		   width + SCROLL_BAR_THICKNESS + SLICE_WINDOW_WIDTH);
      }
    else {
	UxPutHeight(RawImageShell,
		    UxGetHeight(RawImageShell) + SLICE_WINDOW_WIDTH);
	UxPutWidth(RawImageShell,
		   UxGetWidth(RawImageShell) + SLICE_WINDOW_WIDTH);
    }
    XSync(UxDisplay, B_FALSE);
    if (VerboseFlag)
      printf("RawImageXYSlices() : Changed window size...\n");
    /* set cursors to crosshairs */
    if (RawImageShell)
      if (XtIsRealized(RawImageShell)) {
	XDefineCursor(UxDisplay, XtWindow(RawImageShell), xhair);
	RawImageCursor = xhair;
      }
    XSync(UxDisplay, B_FALSE);
    if (VerboseFlag)
      printf("RawImageXYSlices() : Defined cursor...\n");
  }
  else {
    /* disable RawImage slice windows */
    UxPutBottomOffset(RawImageScrolledWindow, 2);
    UxPutRightOffset(RawImageScrolledWindow, 2);
    UxUnmap(frame1);
    UxUnmap(frame2);
    UxUnmap(frame3);
    if (NewFlag) {
	UxPutHeight(RawImageShell, height + SCROLL_BAR_THICKNESS);
	UxPutWidth(RawImageShell, width + SCROLL_BAR_THICKNESS);
    }
    else {
	UxPutHeight(RawImageShell, UxGetHeight(RawImageShell) - SLICE_WINDOW_WIDTH);
	UxPutWidth(RawImageShell, UxGetWidth(RawImageShell) - SLICE_WINDOW_WIDTH);
    }
    UxPutMaxHeight(RawImageShell, (ImgHeight*ZoomFactor) + SCROLL_BAR_THICKNESS);
    UxPutMaxWidth(RawImageShell, (ImgWidth*ZoomFactor) + SCROLL_BAR_THICKNESS);
    /* set cursors to pointers */
    if (UxGetWidget(RawImageShell))
      if (XtIsRealized(UxGetWidget(RawImageShell))) {
	XDefineCursor(UxDisplay, XtWindow(UxGetWidget(RawImageShell)),
		      leftpointer);
	RawImageCursor = leftpointer;
      }
  }
  /* updates */
  UxUpdate(RawImageShell);
}

/*******************************************************************************
       The following are Action functions.
*******************************************************************************/

static	void	action_UpdateWinsize(
			Widget wgt, 
			XEvent *ev, 
			String *parm, 
			Cardinal *p_UxNumParams)
{
	Cardinal		UxNumParams = *p_UxNumParams;
	_UxCRawImageShell       *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XEvent                  *UxEvent = ev;
	String                  *UxParams = parm;

	UxSaveCtx = UxRawImageShellContext;
	UxRawImageShellContext = UxContext =
			(_UxCRawImageShell *) UxGetContext( UxWidget );
	{
	UpdateRawWinParms();
	}
	UxRawImageShellContext = UxSaveCtx;
}

static	void	action_DrawSlices(
			Widget wgt, 
			XEvent *ev, 
			String *parm, 
			Cardinal *p_UxNumParams)
{
	Cardinal		UxNumParams = *p_UxNumParams;
	_UxCRawImageShell       *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XEvent                  *UxEvent = ev;
	String                  *UxParams = parm;

	UxSaveCtx = UxRawImageShellContext;
	UxRawImageShellContext = UxContext =
			(_UxCRawImageShell *) UxGetContext( UxWidget );
	{
	  int i, LineStart;
	  char b[256];
	  
	  if ((RawPixels != NULL) && (IsTrue(UxGetSet(XYSlicesTBG)))) {
	    /* mouse position stats */
	    sprintf(b, "X: %d", (UxEvent->xmotion.x)/ZoomFactor);
	    UxPutLabelString(RXLG, b);
	    sprintf(b, "Y: %d", (UxEvent->xmotion.y)/ZoomFactor);
	    UxPutLabelString(RYLG, b);
	    sprintf(b, "V: %.1e", RawPixels[
		(UxEvent->xmotion.y/ZoomFactor)*ImgWidth + UxEvent->xmotion.x/ZoomFactor
						  ]);
	    UxPutLabelString(RVLG, b);
	    
	    /* horizontal slice */
	    LineStart = (UxEvent->xmotion.y / ZoomFactor) * ImgWidth;
	    if (RawMax != RawMin)
	      for (i = 0; i < RawWinWidth; i++) {
	        RawPoints[i].x = i;
	        RawPoints[i].y = SLICE_WINDOW_WIDTH -
		  (((RawPixels[LineStart + (RawWinHOffset + i)/ZoomFactor]
		     - RawMin)
		    * SLICE_WINDOW_WIDTH * 8 / 10 / (RawMax-RawMin))
		   + (SLICE_WINDOW_WIDTH / 10));
	      }
	    else
	      for (i = 0; i < RawWinWidth; i++) {
	        RawPoints[i].x = i;
	        RawPoints[i].y = SLICE_WINDOW_WIDTH/2;
	      }
	    XClearWindow(UxDisplay, XtWindow(UxGetWidget(RawXSliceDA)));
	    XDrawPoints(UxDisplay, XtWindow(UxGetWidget(RawXSliceDA)), gc,
			RawPoints, RawWinWidth, CoordModeOrigin);
	    
	    /* vertical slice */
	    LineStart = UxEvent->xmotion.x / ZoomFactor;
	    if (RawMax != RawMin)
	      for (i = 0; i < RawWinHeight; i++) {
	        RawPoints[i].x = ((
	            RawPixels[LineStart + ((RawWinVOffset + i)/ZoomFactor) * ImgWidth]
	  			       - RawMin)
	  			      * SLICE_WINDOW_WIDTH * 8 / 10
	    			      / (RawMax - RawMin)) + (SLICE_WINDOW_WIDTH/10);
	        RawPoints[i].y = i;
	      }
	    else
	      for (i = 0; i < RawWinWidth; i++) {
	        RawPoints[i].x = SLICE_WINDOW_WIDTH/2;
	        RawPoints[i].y = i;
	      }
	    XClearWindow(UxDisplay, XtWindow(UxGetWidget(RawYSliceDA)));
	    XDrawPoints(UxDisplay, XtWindow(UxGetWidget(RawYSliceDA)), gc,
			RawPoints, RawWinHeight, CoordModeOrigin);
	  }
	}
	UxRawImageShellContext = UxSaveCtx;
}

/*******************************************************************************
       The following are callback functions.
*******************************************************************************/

static	void	destroyCB_RawImageShell(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCRawImageShell       *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxRawImageShellContext;
	UxRawImageShellContext = UxContext =
			(_UxCRawImageShell *) UxGetContext( UxWidget );
	{
	  if (XvgUp)
	    UxPutSet(RawImageTBG, "false");
	  RawImageShellCreated = B_FALSE;
	  if (VerboseFlag)
	    printf("XVG RawImageShell destroyed.\n");
	}
	UxRawImageShellContext = UxSaveCtx;
}

static	void	popdownCB_RawImageShell(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCRawImageShell       *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxRawImageShellContext;
	UxRawImageShellContext = UxContext =
			(_UxCRawImageShell *) UxGetContext( UxWidget );
	{
	UxPutSet(RawImageTBG, "false");
	}
	UxRawImageShellContext = UxSaveCtx;
}

static	void	popupCB_RawImageShell(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCRawImageShell       *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxRawImageShellContext;
	UxRawImageShellContext = UxContext =
			(_UxCRawImageShell *) UxGetContext( UxWidget );
	{
	UxPutSet(RawImageTBG, "true");
	}
	UxRawImageShellContext = UxSaveCtx;
}

static	void	exposeCB_RawImageDrawingArea(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCRawImageShell       *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxRawImageShellContext;
	UxRawImageShellContext = UxContext =
			(_UxCRawImageShell *) UxGetContext( UxWidget );
	{
	XEvent *event;
	int i;
	
	if ((RawPixels == NULL) || (RawImageShellCreated == B_FALSE) || (IsTrue(UxGetSet(RawImageTBG)) == B_FALSE))
	  return;
	
	event = ((XmDrawingAreaCallbackStruct *) UxCallbackArg)->event;
	XPutImage(UxDisplay, XtWindow(UxGetWidget(RawImageDrawingArea)),
		  gc, RawImageXObj, event->xexpose.x, event->xexpose.y,
		  event->xexpose.x, event->xexpose.y,
		  event->xexpose.width, event->xexpose.height);
	
	/* execute the overlay functions if any */
	for (i = 0; (i < NOverlayFunctions) && (OverlayLevel > 0); i++)
		(OverlayFunctions[i])(RawImageDrawingArea);
	
	/* overlay the colorbar if required */
	if (ColorBarOverlay)
	    DrawColorBar(RawImageDrawingArea, B_TRUE);
	}
	UxRawImageShellContext = UxSaveCtx;
}

static	void	activateCB_menu12_p1_b1(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCRawImageShell       *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxRawImageShellContext;
	UxRawImageShellContext = UxContext =
			(_UxCRawImageShell *) UxGetContext( UxWidget );
	{TransposeImage();}
	UxRawImageShellContext = UxSaveCtx;
}

static	void	activateCB_menu12_p1_b2(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCRawImageShell       *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxRawImageShellContext;
	UxRawImageShellContext = UxContext =
			(_UxCRawImageShell *) UxGetContext( UxWidget );
	{RotateImage();}
	UxRawImageShellContext = UxSaveCtx;
}

/*******************************************************************************
       The 'build_' function creates all the widgets
       using the resource values specified in the Property Editor.
*******************************************************************************/

static Widget	_Uxbuild_RawImageShell(void)
{
	MrmType		_UxDummyClass;
	Cardinal	_UxStatus;


	/* Creation of RawImageShell */
	RawImageShell = XtVaCreatePopupShell( "RawImageShell",
			topLevelShellWidgetClass,
			UxTopLevel,
			XmNx, 270,
			XmNy, 200,
			XmNwidth, 330,
			XmNheight, 250,
			XmNiconName, "RawImg",
			XmNtitle, "Raw Image Window",
			XmNmaxHeight, 200,
			XmNmaxWidth, 300,
			RES_CONVERT( XmNtranslations, "" ),
			XmNallowShellResize, B_FALSE,
			NULL );
	XtAddCallback( RawImageShell, XmNdestroyCallback,
		(XtCallbackProc) destroyCB_RawImageShell,
		(XtPointer) UxRawImageShellContext );
	XtAddCallback( RawImageShell, XmNpopdownCallback,
		(XtCallbackProc) popdownCB_RawImageShell,
		(XtPointer) UxRawImageShellContext );
	XtAddCallback( RawImageShell, XmNpopupCallback,
		(XtCallbackProc) popupCB_RawImageShell,
		(XtPointer) UxRawImageShellContext );

	UxPutContext( RawImageShell, (char *) UxRawImageShellContext );

	form2 = NULL;
	_UxStatus = MrmFetchWidget( hierarchy,
					"form2",
					RawImageShell,
					&form2,
					&_UxDummyClass );
	if ( _UxStatus != MrmSUCCESS )
	{
		UxMrmFetchError( hierarchy, "form2",
				RawImageShell, _UxStatus );
		return( (Widget) NULL );
	}

	XtManageChild( form2 );

	UxPutContext( form2, (char *) UxRawImageShellContext );
	RawImageScrolledWindow = XtNameToWidget( form2, "*RawImageScrolledWindow" );
	UxPutContext( RawImageScrolledWindow, (char *) UxRawImageShellContext );
	RawImageDrawingArea = XtNameToWidget( RawImageScrolledWindow, "*RawImageDrawingArea" );
	UxPutContext( RawImageDrawingArea, (char *) UxRawImageShellContext );
	menu12 = XtNameToWidget( RawImageDrawingArea, "*menu12" );
	UxPutContext( menu12, (char *) UxRawImageShellContext );
	menu12_p1_b1 = XtNameToWidget( menu12, "*menu12_p1_b1" );
	UxPutContext( menu12_p1_b1, (char *) UxRawImageShellContext );
	menu12_p1_b2 = XtNameToWidget( menu12, "*menu12_p1_b2" );
	UxPutContext( menu12_p1_b2, (char *) UxRawImageShellContext );
	frame1 = XtNameToWidget( form2, "*frame1" );
	UxPutContext( frame1, (char *) UxRawImageShellContext );
	RawXSliceDA = XtNameToWidget( frame1, "*RawXSliceDA" );
	UxPutContext( RawXSliceDA, (char *) UxRawImageShellContext );
	frame2 = XtNameToWidget( form2, "*frame2" );
	UxPutContext( frame2, (char *) UxRawImageShellContext );
	RawYSliceDA = XtNameToWidget( frame2, "*RawYSliceDA" );
	UxPutContext( RawYSliceDA, (char *) UxRawImageShellContext );
	frame3 = XtNameToWidget( form2, "*frame3" );
	UxPutContext( frame3, (char *) UxRawImageShellContext );
	rowColumn6 = XtNameToWidget( frame3, "*rowColumn6" );
	UxPutContext( rowColumn6, (char *) UxRawImageShellContext );
	RXLG = XtNameToWidget( rowColumn6, "*RXLG" );
	UxPutContext( RXLG, (char *) UxRawImageShellContext );
	RYLG = XtNameToWidget( rowColumn6, "*RYLG" );
	UxPutContext( RYLG, (char *) UxRawImageShellContext );
	RVLG = XtNameToWidget( rowColumn6, "*RVLG" );
	UxPutContext( RVLG, (char *) UxRawImageShellContext );

	XtAddCallback( RawImageShell, XmNdestroyCallback,
		(XtCallbackProc) UxDestroyContextCB,
		(XtPointer) UxRawImageShellContext);

	XtAddEventHandler(RawImageDrawingArea, ButtonPressMask,
			B_FALSE, (XtEventHandler) _UxRawImageShellMenuPost, (XtPointer) menu12 );

	return ( RawImageShell );
}

/*******************************************************************************
       The following is the 'Interface function' which is the
       external entry point for creating this interface.
       This function should be called from your application or from
       a callback function.
*******************************************************************************/

Widget	create_RawImageShell(void)
{
	Widget                  rtrn;
	_UxCRawImageShell       *UxContext;
	static int		_Uxinit = 0;

	UxRawImageShellContext = UxContext =
		(_UxCRawImageShell *) UxNewContext( sizeof(_UxCRawImageShell), B_FALSE );


	if ( ! _Uxinit )
	{
		static MrmRegisterArg	_UxMrmNames[] = {
			{ "destroyCB_RawImageShell", 
				(XtPointer) destroyCB_RawImageShell },
			{ "popdownCB_RawImageShell", 
				(XtPointer) popdownCB_RawImageShell },
			{ "popupCB_RawImageShell", 
				(XtPointer) popupCB_RawImageShell },
			{ "exposeCB_RawImageDrawingArea", 
				(XtPointer) exposeCB_RawImageDrawingArea },
			{ "activateCB_menu12_p1_b1", 
				(XtPointer) activateCB_menu12_p1_b1 },
			{ "activateCB_menu12_p1_b2", 
				(XtPointer) activateCB_menu12_p1_b2 }};

		static XtActionsRec	_Uxactions[] = {
			{ "UpdateWinsize", (XtActionProc) action_UpdateWinsize },
			{ "DrawSlices", (XtActionProc) action_DrawSlices }};

		XtAppAddActions( UxAppContext,
				_Uxactions,
				XtNumber(_Uxactions) );

		MrmRegisterNamesInHierarchy( hierarchy,
					_UxMrmNames,
					XtNumber(_UxMrmNames) );

		_Uxinit = 1;
	}

	rtrn = _Uxbuild_RawImageShell();
	XSync(UxDisplay, B_FALSE);
	if (VerboseFlag)
	  printf("create_RawImageShell() : Shell widgets built...\n");

	/* show slice windows if required */
	ResizeRawImageWidgets(B_TRUE);
	XSync(UxDisplay, B_FALSE);
	if (VerboseFlag)
	  printf("create_RawImageShell() : Shell widgets resized...\n");
	CreateRawImage();
	XSync(UxDisplay, B_FALSE);
	if (VerboseFlag)
	  printf("create_RawImageShell() : Shell image created...\n");
	
	/* set vertical scrollbar callbacks */
	XtVaGetValues(UxGetWidget(RawImageScrolledWindow), XmNverticalScrollBar, &RawVsbar, NULL);
	XtAddCallback(RawVsbar, XmNincrementCallback, VSBCallback, NULL);
	XtAddCallback(RawVsbar, XmNdecrementCallback, VSBCallback, NULL);
	XtAddCallback(RawVsbar, XmNdragCallback, VSBCallback, NULL);
	XtAddCallback(RawVsbar, XmNtoTopCallback, VSBCallback, NULL);
	XtAddCallback(RawVsbar, XmNtoBottomCallback, VSBCallback, NULL);
	XtAddCallback(RawVsbar, XmNpageIncrementCallback, VSBCallback, NULL);
	XtAddCallback(RawVsbar, XmNpageDecrementCallback, VSBCallback, NULL);
	
	/* set horizontal scrollbar callbacks */
	XtVaGetValues(UxGetWidget(RawImageScrolledWindow), XmNhorizontalScrollBar, &RawHsbar, NULL);
	XtAddCallback(RawHsbar, XmNincrementCallback, HSBCallback, NULL);
	XtAddCallback(RawHsbar, XmNdecrementCallback, HSBCallback, NULL);
	XtAddCallback(RawHsbar, XmNdragCallback, HSBCallback, NULL);
	XtAddCallback(RawHsbar, XmNtoTopCallback, HSBCallback, NULL);
	XtAddCallback(RawHsbar, XmNtoBottomCallback, HSBCallback, NULL);
	XtAddCallback(RawHsbar, XmNpageIncrementCallback, HSBCallback, NULL);
	XtAddCallback(RawHsbar, XmNpageDecrementCallback, HSBCallback, NULL);
	
	/* set created flag */
	RawImageShellCreated = B_TRUE;
	
	/* return */
	if (VerboseFlag)
	  printf("create_RawImageShell() : Done...\n");
	return(rtrn);
}

/*******************************************************************************
       END OF FILE
*******************************************************************************/

