
/*******************************************************************************
	LUTShell.c

       Associated Header file: LUTShell.h
*******************************************************************************/

#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <Mrm/MrmPublic.h>
#include <Xm/MenuShell.h>
#include "UxXt.h"

#include <Xm/RowColumn.h>
#include <Xm/SeparatoG.h>
#include <Xm/PushBG.h>
#include <Xm/LabelG.h>
#include <Xm/Text.h>
#include <Xm/Form.h>
#include <X11/Shell.h>

/*******************************************************************************
       Includes, Defines, and Global variables from the Declarations Editor:
*******************************************************************************/

#include "include/XvgGlobals.h"

/* externals */
extern swidget RawImageDrawingArea, ProcessedImageDrawingArea;
extern swidget LUTShellTBG, GammaScale, RawImageTBG, ProcessedImageTBG;

/* globals */
static char ColorBarData[COLORBARWIDTH * COLORBARHEIGHT];
static int ColorTableType=0;


/*******************************************************************************
       The following header file defines the context structure.
*******************************************************************************/

#define CONTEXT_MACRO_ACCESS 1
#include "LUTShell.h"
#undef CONTEXT_MACRO_ACCESS

Widget	LUTShell;
Widget	ColorControlPtTW;

/*******************************************************************************
Auxiliary code from the Declarations Editor:
*******************************************************************************/

void UpdateColorLUT(void)
{
  XColor verticies[16];
  int i, j, k, nverticies, NColorsPerEdge;
  double RedStep, GreenStep, BlueStep, scale;
  char b[168][32], s[512];

  /* load color list and remove any commas */
  nverticies = sscanf((char *) UxGetText(ColorControlPtTW), "%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s",
	&(b[0][0]), &(b[1][0]), &(b[2][0]), &(b[3][0]),
	&(b[4][0]), &(b[5][0]), &(b[6][0]), &(b[7][0]),
	&(b[8][0]), &(b[9][0]), &(b[10][0]), &(b[11][0]),
	&(b[12][0]), &(b[13][0]), &(b[14][0]), &(b[15][0]));
  nverticies = (nverticies == -1 ? 0 : nverticies);
  for (i = 0; i < nverticies; i++)
	if (b[i][strlen(&(b[i][0]))-1] == ',')
		b[i][strlen(&(b[i][0]))-1] = '\0';

  /* check input vertex number */
  if ((nverticies < 2) || (nverticies > 17)) {
	ErrorMsg("Invalid number of colors!");
	return;
  }

  /* load verticies array */
  for (i = 0; i < nverticies; i++)
	if (XParseColor(UxDisplay, cmap, (char *) &(b[i][0]), (XColor *) &(verticies[i])) == 0) {
		sprintf(s, "Unknown color \"%s\"!", &(b[i][0]));
		ErrorMsg(s);
		return;
	}

  /* load the LUT with the new mapping */
  scale = ((double) UxGetValue(GammaScale)) / (((double) UxGetMaximum(GammaScale)) - ((double) UxGetMinimum(GammaScale)));
  if (ColorTableType == 0) {
  for (i = 1, k = 0, NColorsPerEdge = 128/(nverticies -1); i < nverticies; i++) {
	RedStep = (((double) verticies[i].red) - ((double) verticies[i-1].red))*scale/((double) (NColorsPerEdge - 1));
	GreenStep = (((double) verticies[i].green) - ((double) verticies[i-1].green))*scale/((double) (NColorsPerEdge - 1));
	BlueStep = (((double) verticies[i].blue) - ((double) verticies[i-1].blue))*scale/((double) (NColorsPerEdge - 1));
	for (j = 0; j < NColorsPerEdge; j++, k++) {
		BRYGMBVMCells[k].red = ((double) verticies[i-1].red)*scale + ((double) j)*RedStep;
		BRYGMBVMCells[k].green = ((double) verticies[i-1].green)*scale + ((double) j)*GreenStep;
		BRYGMBVMCells[k].blue = ((double) verticies[i-1].blue)*scale + ((double) j)*BlueStep;
	}
  }	
  }
  else {
    for (i = 0, k = 0; i < nverticies; i++) {
      for (j = 0; j < (128/nverticies); j++, k++) {
        BRYGMBVMCells[k].red = ((double) verticies[i].red) * scale;
	BRYGMBVMCells[k].green = ((double) verticies[i].green) * scale;
	BRYGMBVMCells[k].blue = ((double) verticies[i].blue) * scale;
      }
    }
  }
  for (; k < 128; k++) {
	BRYGMBVMCells[k].red = BRYGMBVMCells[k-1].red;
	BRYGMBVMCells[k].green = BRYGMBVMCells[k-1].green;
	BRYGMBVMCells[k].blue = BRYGMBVMCells[k-1].blue;
  }

  /* load the lut */
  XStoreColors(UxDisplay, cmap, BRYGMBVMCells, NPAGEDLUTCELLS);
  CurrentCells = BRYGMBVMCells;
}

void DrawColorBar(swidget DAW, boolean_t ShellCreatedFlag)
{
    char b[256];
    double max, min;

    if(ShellCreatedFlag) {
        XPutImage(UxDisplay, XtWindow(UxGetWidget(DAW)), gc, ColorBarImageXObj, 0, 0,
	    ImgWidth*ZoomFactor - (5*COLORBARWIDTH)/2, (ImgHeight*ZoomFactor - COLORBARHEIGHT)/2,
	    COLORBARWIDTH, COLORBARHEIGHT);
        XDrawRectangle(UxDisplay, XtWindow(UxGetWidget(DAW)), gc,
	    ImgWidth*ZoomFactor - (5*COLORBARWIDTH)/2 - 2,
	    (ImgHeight*ZoomFactor - COLORBARHEIGHT)/2 - 2, COLORBARWIDTH+4, COLORBARHEIGHT+4);
	if (UxGetWidget(DAW) == UxGetWidget(RawImageDrawingArea)) {
		max = RawMax;
		min = RawMin;
	}
	else {
		max = ProcMax;
		min = ProcMin;
	}	
	sprintf(b, "%.2e", max);
	XDrawString(UxDisplay, XtWindow(UxGetWidget(DAW)), gc, ImgWidth*ZoomFactor - (3*COLORBARWIDTH)/2 + 2
		- XTextWidth(LabelFont, b, strlen(b)), (ImgHeight*ZoomFactor - COLORBARHEIGHT)/2 - 2 - 10,
		b, strlen(b));
	sprintf(b, "%.2e", min);
	XDrawString(UxDisplay, XtWindow(UxGetWidget(DAW)), gc, ImgWidth*ZoomFactor - (3*COLORBARWIDTH)/2 + 2
		- XTextWidth(LabelFont, b, strlen(b)),
		(ImgHeight*ZoomFactor + COLORBARHEIGHT)/2 - 2 + 10 + LabelFont->max_bounds.ascent,
		b, strlen(b));
    }
}

/*******************************************************************************
       The following are callback functions.
*******************************************************************************/

static	void	destroyCB_LUTShell(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCLUTShell            *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxLUTShellContext;
	UxLUTShellContext = UxContext =
			(_UxCLUTShell *) UxGetContext( UxWidget );
	{
	  if (XvgUp)
	    UxPutSet(LUTShellTBG, "false");
	  LUTShellCreated = B_FALSE;
	  if (VerboseFlag)
	    printf("XVG LUTShell destroyed.\n");
	}
	UxLUTShellContext = UxSaveCtx;
}

static	void	popdownCB_LUTShell(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCLUTShell            *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxLUTShellContext;
	UxLUTShellContext = UxContext =
			(_UxCLUTShell *) UxGetContext( UxWidget );
	{
	UxPutSet(LUTShellTBG, "false");
	
	}
	UxLUTShellContext = UxSaveCtx;
}

static	void	popupCB_LUTShell(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCLUTShell            *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxLUTShellContext;
	UxLUTShellContext = UxContext =
			(_UxCLUTShell *) UxGetContext( UxWidget );
	{
	UxPutSet(LUTShellTBG, "true");
	
	}
	UxLUTShellContext = UxSaveCtx;
}

static	void	activateCB_pushButtonGadget1(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCLUTShell            *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxLUTShellContext;
	UxLUTShellContext = UxContext =
			(_UxCLUTShell *) UxGetContext( UxWidget );
	{
	UpdateColorLUT();
	}
	UxLUTShellContext = UxSaveCtx;
}

static	void	activateCB_menu7_p1_b2(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCLUTShell            *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxLUTShellContext;
	UxLUTShellContext = UxContext =
			(_UxCLUTShell *) UxGetContext( UxWidget );
	{
	    static XEvent e;
	    static XmDrawingAreaCallbackStruct s;
	
	    ColorBarOverlay = B_FALSE;
	
	    s.event = &e;
	    s.event->xexpose.x = 0;
	    s.event->xexpose.y = 0;
	    s.event->xexpose.width = ImgWidth*ZoomFactor;
	    s.event->xexpose.height = ImgHeight*ZoomFactor;
	    RawExposeCB(UxGetWidget(LUTShell), NULL, &s);
	    ProcExposeCB(UxGetWidget(LUTShell), NULL, &s);
	}
	UxLUTShellContext = UxSaveCtx;
}

static	void	activateCB_menu7_p1_b1(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCLUTShell            *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxLUTShellContext;
	UxLUTShellContext = UxContext =
			(_UxCLUTShell *) UxGetContext( UxWidget );
	{
	    ColorBarOverlay = B_TRUE;
	    if (IsTrue(UxGetSet(RawImageTBG)))
	      DrawColorBar(RawImageDrawingArea, RawImageShellCreated);
	    if (IsTrue(UxGetSet(ProcessedImageTBG)))
	      DrawColorBar(ProcessedImageDrawingArea, ProcessedImageShellCreated);
	}
	UxLUTShellContext = UxSaveCtx;
}

static	void	activateCB_menu8_p1_b2(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCLUTShell            *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxLUTShellContext;
	UxLUTShellContext = UxContext =
			(_UxCLUTShell *) UxGetContext( UxWidget );
	{
	  ColorTableType = 0;
	  UpdateColorLUT();
	}
	UxLUTShellContext = UxSaveCtx;
}

static	void	activateCB_menu8_p1_b1(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCLUTShell            *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxLUTShellContext;
	UxLUTShellContext = UxContext =
			(_UxCLUTShell *) UxGetContext( UxWidget );
	{
	  ColorTableType = 1;
	  UpdateColorLUT();
	}
	UxLUTShellContext = UxSaveCtx;
}

/*******************************************************************************
       The 'build_' function creates all the widgets
       using the resource values specified in the Property Editor.
*******************************************************************************/

static Widget	_Uxbuild_LUTShell(void)
{
	MrmType		_UxDummyClass;
	Cardinal	_UxStatus;


	/* Creation of LUTShell */
	LUTShell = XtVaCreatePopupShell( "LUTShell",
			applicationShellWidgetClass,
			UxTopLevel,
			XmNx, 106,
			XmNy, 449,
			XmNwidth, 614,
			XmNheight, 91,
			XmNallowShellResize, B_FALSE,
			XmNtitle, "Color LUT Control",
			NULL );
	XtAddCallback( LUTShell, XmNdestroyCallback,
		(XtCallbackProc) destroyCB_LUTShell,
		(XtPointer) UxLUTShellContext );
	XtAddCallback( LUTShell, XmNpopdownCallback,
		(XtCallbackProc) popdownCB_LUTShell,
		(XtPointer) UxLUTShellContext );
	XtAddCallback( LUTShell, XmNpopupCallback,
		(XtCallbackProc) popupCB_LUTShell,
		(XtPointer) UxLUTShellContext );

	UxPutContext( LUTShell, (char *) UxLUTShellContext );

	form12 = NULL;
	_UxStatus = MrmFetchWidget( hierarchy,
					"form12",
					LUTShell,
					&form12,
					&_UxDummyClass );
	if ( _UxStatus != MrmSUCCESS )
	{
		UxMrmFetchError( hierarchy, "form12",
				LUTShell, _UxStatus );
		return( (Widget) NULL );
	}

	XtManageChild( form12 );

	UxPutContext( form12, (char *) UxLUTShellContext );
	ColorControlPtTW = XtNameToWidget( form12, "*ColorControlPtTW" );
	UxPutContext( ColorControlPtTW, (char *) UxLUTShellContext );
	labelGadget66 = XtNameToWidget( form12, "*labelGadget66" );
	UxPutContext( labelGadget66, (char *) UxLUTShellContext );
	pushButtonGadget1 = XtNameToWidget( form12, "*pushButtonGadget1" );
	UxPutContext( pushButtonGadget1, (char *) UxLUTShellContext );
	separatorGadget11 = XtNameToWidget( form12, "*separatorGadget11" );
	UxPutContext( separatorGadget11, (char *) UxLUTShellContext );
	menu7 = XtNameToWidget( form12, "*menu7" );
	UxPutContext( menu7, (char *) UxLUTShellContext );
	menu7_p1 = XtNameToWidget( form12, "*menu7_p1" );
	UxPutContext( menu7_p1, (char *) UxLUTShellContext );
	menu7_p1_b2 = XtNameToWidget( menu7_p1, "*menu7_p1_b2" );
	UxPutContext( menu7_p1_b2, (char *) UxLUTShellContext );
	menu7_p1_b1 = XtNameToWidget( menu7_p1, "*menu7_p1_b1" );
	UxPutContext( menu7_p1_b1, (char *) UxLUTShellContext );
	menu8 = XtNameToWidget( form12, "*menu8" );
	UxPutContext( menu8, (char *) UxLUTShellContext );
	menu8_p1 = XtNameToWidget( form12, "*menu8_p1" );
	UxPutContext( menu8_p1, (char *) UxLUTShellContext );
	menu8_p1_b2 = XtNameToWidget( menu8_p1, "*menu8_p1_b2" );
	UxPutContext( menu8_p1_b2, (char *) UxLUTShellContext );
	menu8_p1_b1 = XtNameToWidget( menu8_p1, "*menu8_p1_b1" );
	UxPutContext( menu8_p1_b1, (char *) UxLUTShellContext );

	XtAddCallback( LUTShell, XmNdestroyCallback,
		(XtCallbackProc) UxDestroyContextCB,
		(XtPointer) UxLUTShellContext);

	return ( LUTShell );
}

/*******************************************************************************
       The following is the 'Interface function' which is the
       external entry point for creating this interface.
       This function should be called from your application or from
       a callback function.
*******************************************************************************/

Widget	create_LUTShell(void)
{
	Widget                  rtrn;
	_UxCLUTShell            *UxContext;
	static int		_Uxinit = 0;

	UxLUTShellContext = UxContext =
		(_UxCLUTShell *) UxNewContext( sizeof(_UxCLUTShell), B_FALSE );


	if ( ! _Uxinit )
	{
		static MrmRegisterArg	_UxMrmNames[] = {
			{ "destroyCB_LUTShell", 
				(XtPointer) destroyCB_LUTShell },
			{ "popdownCB_LUTShell", 
				(XtPointer) popdownCB_LUTShell },
			{ "popupCB_LUTShell", 
				(XtPointer) popupCB_LUTShell },
			{ "activateCB_pushButtonGadget1", 
				(XtPointer) activateCB_pushButtonGadget1 },
			{ "activateCB_menu7_p1_b2", 
				(XtPointer) activateCB_menu7_p1_b2 },
			{ "activateCB_menu7_p1_b1", 
				(XtPointer) activateCB_menu7_p1_b1 },
			{ "activateCB_menu8_p1_b2", 
				(XtPointer) activateCB_menu8_p1_b2 },
			{ "activateCB_menu8_p1_b1", 
				(XtPointer) activateCB_menu8_p1_b1 }};

		MrmRegisterNamesInHierarchy( hierarchy,
					_UxMrmNames,
					XtNumber(_UxMrmNames) );

		_Uxinit = 1;
	}

	rtrn = _Uxbuild_LUTShell();

	LUTShellCreated = B_TRUE;
	
	/* create color bar pattern */
	ColorBarImageXObj = XCreateImage(UxDisplay, XDefaultVisual(UxDisplay, UxScreen), 8,
		ZPixmap, 0, ColorBarData, COLORBARWIDTH, COLORBARHEIGHT, 8, 0);
	
	for (ii = 127; ii >= 0; ii--)
	  for (jj = COLORBARHEIGHT/128 - 1; jj >= 0; jj--)
	    for (kk = 0; kk < COLORBARWIDTH; kk++)
		ColorBarData[((ii*(COLORBARHEIGHT/128))+jj)*COLORBARWIDTH + kk] = BRYGMBVMCells[127-ii].pixel;
	
	ColorTableType = 0;
	return(rtrn);
}

/*******************************************************************************
       END OF FILE
*******************************************************************************/

