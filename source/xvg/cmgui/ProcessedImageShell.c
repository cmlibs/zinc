
/*******************************************************************************
	ProcessedImageShell.c

       Associated Header file: ProcessedImageShell.h
       Associated Resource file: ProcessedImageShell.rf
*******************************************************************************/

#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <Mrm/MrmPublic.h>
#include <Xm/MenuShell.h>
#include "UxXt.h"

#include <Xm/LabelG.h>
#include <Xm/Frame.h>
#include <Xm/SeparatoG.h>
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

#if defined (CMGUI)
/* for doing node editing and creation */
#include "finite_element/finite_element.h"
#include "general/list_private.h"
#include "data/data_2d_dialog.h"
#include "data/data_2d.h"

#include "general/geometry.h"

#define Node_2d_type float
struct Node_2d
/*******************************************************************************
LAST MODIFIED : 1 December 1997

DESCRIPTION :
This structure contains information necessary for XVG to draw a node on the
processed window.
==============================================================================*/
{
	char *identifier;
	Node_2d_type x,y;
	int selected;
	int access_count;
}; /* struct Node_2d */
#endif /* defined (CMGUI) */

/* externals */
extern Widget promptDialog, HistogramTBG, ProcessedImageTBG, XYSlicesTBG;

/* forwards */
#if defined (CMGUI)
struct Node_2d *CREATE(Node_2d)(char *identifier,Node_2d_type x,Node_2d_type y);
int DESTROY(Node_2d)(struct Node_2d **node_2d_address);
#else /* defined (CMGUI) */
static int  CreateNode(int x, int y);
static void Draw_FE_nodes(Widget DrawingArea);
static int DeleteNode(struct FE_node *ptr,void *dummy);
static void DeleteAllNodes(void);
#endif /* defined (CMGUI) */
static void DrawImgPointFrame(int cx, int cy);
static int  FE_node_get_pixel_coord(struct FE_node *node, int flag);
static void FE_node_change_real_coords(struct FE_node *node, int x, int y,
				       double z);
static void exposeCB_ProcImageDrawingArea(Widget wgt, XtPointer cd,
					  XtPointer cb);

/* globals */
enum {MODENULL, COMPDIST, DFNNODES, DELNODE, MVNODE, DRAWCLIPRECT};
enum {XPIXELCOORD, YPIXELCOORD};
#define NPOLYGONVERTICIESMAX 5000
#define NODERADIUS 5
static boolean_t DrawClipOn, DistanceFirstPointDone, ComputeDistanceOn;
static int OverlayLevelSave, npv, NFringeRegions, node_index,
ProcActionMode, PreviousXpos, PreviousYpos,
PreviousCursorPosX, PreviousCursorPosY, StartCursorPosX, StartCursorPosY;

static XPoint PolygonVerticies[NPOLYGONVERTICIESMAX];

#if defined (CMGUI)
static struct LIST(Node_2d) *processed_node_list=(struct LIST(Node_2d) *)NULL;
static struct Node_2d *Highlighted_node_2d=(struct Node_2d *)NULL;
Widget data_2d_dialog=(Widget)NULL;
#else /* defined (CMGUI) */
static struct FE_node *Highlighted_FE_node, *NodeToBeChanged;
static struct FE_field *field;
static struct FE_node_field *node_field;
static struct FE_node_field_info *node_field_info;
static struct FE_node_field_list *coordinate_node_field_list;
#endif /* defined (CMGUI) */

/*******************************************************************************
       The following header file defines the context structure.
*******************************************************************************/

#define CONTEXT_MACRO_ACCESS 1
#include "ProcessedImageShell.h"
#undef CONTEXT_MACRO_ACCESS

#if defined (CMGUI)
DECLARE_LIST_TYPES(Node_2d);
FULL_DECLARE_LIST_TYPE(Node_2d);
DECLARE_OBJECT_FUNCTIONS(Node_2d)
DECLARE_LIST_FUNCTIONS(Node_2d)
#endif /* defined (CMGUI) */

/*******************************************************************************
       The following function is an event-handler for posting menus.
*******************************************************************************/

static void	_UxProcessedImageShellMenuPost(
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

Widget	ProcessedImageShell;
Widget	ProcessedImageDrawingArea;

/*******************************************************************************
Auxiliary code from the Declarations Editor:
*******************************************************************************/

/* indirect callbacks */
#include "custom/ProcessedImageShellAux.c"

/*******************************************************************************
       The following are Action functions.
*******************************************************************************/

static	void	action_UpdateProcessedWinsize(
			Widget wgt, 
			XEvent *ev, 
			String *parm, 
			Cardinal *p_UxNumParams)
{
	Cardinal		UxNumParams = *p_UxNumParams;
	_UxCProcessedImageShell *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XEvent                  *UxEvent = ev;
	String                  *UxParams = parm;

	UxSaveCtx = UxProcessedImageShellContext;
	UxProcessedImageShellContext = UxContext =
			(_UxCProcessedImageShell *) UxGetContext( UxWidget );
	{
	    UpdateProcessedWinParms();
	}
	UxProcessedImageShellContext = UxSaveCtx;
}

static	void	action_ProcMotionNotify(
			Widget wgt, 
			XEvent *ev, 
			String *parm, 
			Cardinal *p_UxNumParams)
{
  Cardinal		UxNumParams = *p_UxNumParams;
  _UxCProcessedImageShell *UxSaveCtx, *UxContext;
  Widget                  UxWidget = wgt;
  XEvent                  *UxEvent = ev;
  String                  *UxParams = parm;
  
  UxSaveCtx = UxProcessedImageShellContext;
  UxProcessedImageShellContext = UxContext =
    (_UxCProcessedImageShell *) UxGetContext( UxWidget );
  {
    int i, LineStart;
    char b[256];
    
    /* draw any highlight around defined nodes */
    if ((ProcActionMode == MODENULL)
	|| (ProcActionMode == DELNODE)
	|| (ProcActionMode == MVNODE))
      DrawImgPointFrame(UxEvent->xmotion.x, UxEvent->xmotion.y);
    
    /* draw pixels if currently defining clipping contour or polygon */
    if (DrawClipOn) {
      XDrawLine(UxDisplay, XtWindow(ProcessedImageDrawingArea),
		gc_RedBlack, PreviousCursorPosX, PreviousCursorPosY, 
		UxEvent->xmotion.x, UxEvent->xmotion.y);
      PreviousCursorPosX      = UxEvent->xmotion.x;
      PreviousCursorPosY      = UxEvent->xmotion.y;
      PolygonVerticies[npv].x = UxEvent->xmotion.x / ZoomFactor;
      PolygonVerticies[npv].y = UxEvent->xmotion.y / ZoomFactor;
      npv++;
      if (npv == NPOLYGONVERTICIESMAX) {
	ProcActionMode = MODENULL;
	DrawClipOn = B_FALSE;
	ErrorMsg("ProcMotionNotify(): Max num of polygon verticies exceeded!");
      }
    }
    
    /* draw slice projections */
    if ((ProcessedPixels != NULL) && (IsTrue(UxGetSet(XYSlicesTBG)))) {
      /* mouse position stats */
      sprintf(b, "X: %d", (UxEvent->xmotion.x)/ZoomFactor);
      UxPutLabelString(PXLG, b);
      sprintf(b, "Y: %d", (UxEvent->xmotion.y)/ZoomFactor);
      UxPutLabelString(PYLG, b);
      sprintf(b, "%.1e",
	      ProcessedPixels[(UxEvent->xmotion.y/ZoomFactor)*ImgWidth
			      + UxEvent->xmotion.x/ZoomFactor]);
      UxPutLabelString(PVLG, b);
      
#ifndef RS6000
      /* horizontal and vertical lines line */
      if ((PreviousXpos >= 0) && (PreviousYpos >= 0))
	{
	  XDrawLine(UxDisplay,
		    XtWindow(ProcessedImageDrawingArea),
		    gc_xor,
		    PreviousXpos, 0,
		    PreviousXpos, ImgHeight*ZoomFactor -1);
	  XDrawLine(UxDisplay,
		    XtWindow(ProcessedImageDrawingArea),
		    gc_xor,
		    0, PreviousYpos,
		    ImgWidth*ZoomFactor -1, PreviousYpos);
	}
      XDrawLine(UxDisplay,
		XtWindow(ProcessedImageDrawingArea),
		gc_xor,
		UxEvent->xmotion.x, 0,
		UxEvent->xmotion.x, ImgHeight*ZoomFactor -1);
      XDrawLine(UxDisplay,
		XtWindow(ProcessedImageDrawingArea),
		gc_xor,
		0, UxEvent->xmotion.y,
		ImgWidth*ZoomFactor-1, UxEvent->xmotion.y);
      PreviousXpos = UxEvent->xmotion.x;
      PreviousYpos = UxEvent->xmotion.y;
#endif	
      /* horizontal slice */
      LineStart = (UxEvent->xmotion.y / ZoomFactor) * ImgWidth;
      if (ProcMax != ProcMin)
	for (i = 0; i < ProcessedWinWidth; i++) {
	  ProcessedPoints[i].x = i;
	  ProcessedPoints[i].y = SLICE_WINDOW_WIDTH -
	    (((ProcessedPixels[LineStart
			       + (ProcessedWinHOffset + i)/ZoomFactor]
	       - ProcMin)
	      * SLICE_WINDOW_WIDTH * 8 / 10 / (ProcMax-ProcMin))
	     + (SLICE_WINDOW_WIDTH / 10));
	}
      else
	for (i = 0; i < ProcessedWinWidth; i++) {
	  ProcessedPoints[i].x = i;
	  ProcessedPoints[i].y = SLICE_WINDOW_WIDTH/2;
	}
      XClearWindow(UxDisplay, XtWindow(ProcessedXSliceDA));
      XDrawPoints(UxDisplay, XtWindow(ProcessedXSliceDA), gc,
		  ProcessedPoints, ProcessedWinWidth, CoordModeOrigin);
      
      /* vertical slice */
      LineStart = UxEvent->xmotion.x / ZoomFactor;
      if (ProcMax != ProcMin)
	for (i = 0; i < ProcessedWinHeight; i++) {
	  ProcessedPoints[i].x =
	    ((ProcessedPixels[LineStart + ((ProcessedWinVOffset + i)
					   /ZoomFactor) * ImgWidth]
	      - ProcMin)
	     * SLICE_WINDOW_WIDTH * 8 / 10
	     / (ProcMax - ProcMin)) + (SLICE_WINDOW_WIDTH/10);
	  ProcessedPoints[i].y = i;
	}
      else
	for (i = 0; i < ProcessedWinWidth; i++) {
	  ProcessedPoints[i].x = SLICE_WINDOW_WIDTH/2;
	  ProcessedPoints[i].y = i;
	}
      XClearWindow(UxDisplay, XtWindow(ProcessedYSliceDA));
      XDrawPoints(UxDisplay, XtWindow(ProcessedYSliceDA), gc,
		  ProcessedPoints, ProcessedWinHeight, CoordModeOrigin);
    }
  }
  UxProcessedImageShellContext = UxSaveCtx;
}

static	void	action_ProcDABtn1Up(
				    Widget wgt, 
				    XEvent *ev, 
				    String *parm, 
				    Cardinal *p_UxNumParams)
{
  Cardinal		UxNumParams = *p_UxNumParams;
  _UxCProcessedImageShell *UxSaveCtx, *UxContext;
  Widget                  UxWidget = wgt;
  XEvent                  *UxEvent = ev;
  String                  *UxParams = parm;
  
  UxSaveCtx = UxProcessedImageShellContext;
  UxProcessedImageShellContext = UxContext =
    (_UxCProcessedImageShell *) UxGetContext( UxWidget );
  {
    CONTOUR_ELEMENT contour[CONTOURNMAX];
    XImage *image;
    int i, j, k, CentroidX, CentroidY;
#if defined (CMGUI)
		DATA_2D_PRECISION image_coord[2];
#endif /* defined (CMGUI) */

#if defined (CMGUI)
		if ((ProcActionMode==MODENULL)&&data_2d_dialog)
		{
			/* get image coords */
			image_coord[0] = (UxEvent->xmotion.x/ZoomFactor-Image_pixel_org_X)/
				Image_pixels_per_m_X;
			image_coord[1] = (UxEvent->xmotion.y/ZoomFactor-Image_pixel_org_Y)/
				Image_pixels_per_m_Y;
			data_2d_dialog_selection(data_2d_dialog,image_coord,FALSE,TRUE);
		}
#endif /* defined (CMGUI) */
    if (DrawClipOn) {
      /* draw a line from the last point to the first point */
      XDrawLine(UxDisplay,
		XtWindow(ProcessedImageDrawingArea), gc_RedBlack,
		UxEvent->xmotion.x, UxEvent->xmotion.y,
		StartCursorPosX, StartCursorPosY);
      
      /* capture mask contour data and convert data to double precison */
      if (VerboseFlag)
	printf("ProcDABtn1Up action : Generating clipping mask...\n");
      HGCursor(B_TRUE);
      XFillPolygon(UxDisplay, XtWindow(ProcessedImageDrawingArea),
		   gc_GreenBlack,
		   PolygonVerticies, npv, Nonconvex, CoordModeOrigin);
      image = XGetImage(UxDisplay, XtWindow(ProcessedImageDrawingArea),
			0, 0, ImgWidth, ImgHeight, AllPlanes, ZPixmap);
      for (i = 0, k = 0; i < ImgHeight; i++)
	for (j = 0; j < ImgWidth; j++, k++)
	  MaskPixels[k] = (image->data[i*image->bytes_per_line + j]
			   == green ? 1.0 : 0.0);
      XDestroyImage(image);
      
      /* trace clipping region contour */
      if (VerboseFlag)
	printf("ProcDABtn1Up action : Tracing mask contour...\n");
      NContourPoints = 0;
      for (i = 1; (i < ImgHeight) && (MaskPixels[i*ImgWidth + j] != 1.0);
	   i++)
	for (j = 1; (j < ImgWidth)&&(MaskPixels[i*ImgWidth + j] != 1.0);
	     j++);
      NContourPoints = RegionTraceContour(MaskPixels, DoubleBuffer,
					  ImgHeight, ImgWidth,
					  j-1, i-1, 1.0, contour,
					  CONTOURNMAX);

      if (NContourPoints > 0) {
	/* load the contour points overlay array */
	NContourPoints += 1;
	ContourPoints = (XPoint *) XvgMM_Alloc(ContourPoints,
					       NContourPoints
					       *sizeof(XPoint));
	for (i = 1, ContourPoints[0].x = contour[NContourPoints-2].x,
	     ContourPoints[0].y = contour[NContourPoints-2].y;
	     i < NContourPoints; i++) {
	  ContourPoints[i].x = contour[i-1].x;
	  ContourPoints[i].y = contour[i-1].y;
	}
	
	OverlayLevel = OverlayLevelSave;
	DisplayContourComputed = B_TRUE;
	LoadOverlayFunction(ContourOverlay);
	
	/* update histograms if required */
	if (IsTrue(UxGetSet(HistogramTBG))) {
	  Histogram(RawPixels, RawWinHOffset, RawWinVOffset, RawWinWidth,
		    RawWinHeight, ImgWidth, RawMax, RawMin, RawHistogram,
		    RawHistogramRectangles, &RawMean, &RawSD);
	  Histogram(ProcessedPixels, ProcessedWinHOffset, ProcessedWinVOffset,
		    ProcessedWinWidth, ProcessedWinHeight, ImgWidth, ProcMax,
		    ProcMin, ProcessedHistogram, ProcessedHistogramRectangles,
		    &ProcMean, &ProcSD);
	  DrawHistograms(RawMin, RawMax, ProcMin, ProcMax);
	}
      }
      
      /* redraw the processed window */
      RedrawProcDrawingArea();
      
      /* reset flags */
      DrawClipOn = B_FALSE;
      HGCursor(B_FALSE);
      ProcActionMode = MODENULL;
    }
  }
  UxProcessedImageShellContext = UxSaveCtx;
}

static	void	action_ProcDABtn1Down(
			Widget wgt, 
			XEvent *ev, 
			String *parm, 
			Cardinal *p_UxNumParams)
{
  Cardinal		UxNumParams = *p_UxNumParams;
  _UxCProcessedImageShell *UxSaveCtx, *UxContext;
  Widget                  UxWidget = wgt;
  XEvent                  *UxEvent = ev;
  String                  *UxParams = parm;
 
  UxSaveCtx = UxProcessedImageShellContext;
  UxProcessedImageShellContext = UxContext =
    (_UxCProcessedImageShell *) UxGetContext( UxWidget );
  {
#if defined (CMGUI)
		DATA_2D_PRECISION image_coord[2];
#endif /* defined (CMGUI) */
    double d, dx, dy;
    int i, node;
    char s[16];
    
    /* show current position if required */
    if (VerboseFlag)
      printf("%d %d\n",
	     ImgWidth  - UxEvent->xmotion.x - 1,
	     ImgHeight - UxEvent->xmotion.y - 1);

    switch (ProcActionMode) {
      /* move nodes */
#if defined (CMGUI)
	  case MODENULL:
    {
      if(data_2d_dialog)
      {
        /* get image coords */
        image_coord[0] = (UxEvent->xmotion.x/ZoomFactor-Image_pixel_org_X)/
          Image_pixels_per_m_X;
        image_coord[1] = (UxEvent->xmotion.y/ZoomFactor-Image_pixel_org_Y)/
          Image_pixels_per_m_Y;
        data_2d_dialog_selection(data_2d_dialog,image_coord,TRUE,TRUE);
      }
    }; break;
#else /* defined (CMGUI) */
    case MVNODE:
      if ((Highlighted_FE_node != NULL) && ((NodeToBeChanged == NULL))) {
	NodeToBeChanged = Highlighted_FE_node;
	XDrawRectangle(UxDisplay, 
		       XtWindow(ProcessedImageDrawingArea),
		       gc_xor,
		       FE_node_get_pixel_coord(Highlighted_FE_node,
					       XPIXELCOORD) - NODERADIUS,
		       FE_node_get_pixel_coord(Highlighted_FE_node,
					       YPIXELCOORD) - NODERADIUS,
		       2*NODERADIUS, 2*NODERADIUS);
	      XDrawArc(UxDisplay,
		       XtWindow(ProcessedImageDrawingArea),
		       gc_RedBlack,
		       FE_node_get_pixel_coord(Highlighted_FE_node,
					       XPIXELCOORD) - 1,
		       FE_node_get_pixel_coord(Highlighted_FE_node,
					       YPIXELCOORD) - 1,
		       2, 2, 0, 360*64);
	sprintf(s, "%d", Highlighted_FE_node->cm_node_identifier);
	XDrawString(UxDisplay,
		    XtWindow(ProcessedImageDrawingArea),
		    gc_RedBlack,
		    FE_node_get_pixel_coord(Highlighted_FE_node,
					    XPIXELCOORD) - 5,
		    FE_node_get_pixel_coord(Highlighted_FE_node,
					    YPIXELCOORD) - 5,
		    s, strlen(s));
	Highlighted_FE_node = NULL;
      }
      else if (NodeToBeChanged != NULL) {
	FE_node_change_real_coords(NodeToBeChanged,
				   UxEvent->xmotion.x, UxEvent->xmotion.y,
				   Image_Z_plane);
	NodeToBeChanged = NULL;
	ProcActionMode = MODENULL;
	RedrawProcDrawingArea();
      }
      break;
      /* define FE nodes */
    case DFNNODES:
      /* define a new node */
      if ((node = CreateNode(UxEvent->xmotion.x, UxEvent->xmotion.y))
	  > 0) {
	XDrawArc(UxDisplay,
		 XtWindow(ProcessedImageDrawingArea),
		 gc_YellowBlack,
		 UxEvent->xmotion.x - 1, UxEvent->xmotion.y - 1,
		 2, 2, 0, 360*64);
	sprintf(s, "%d", node);
	XDrawString(UxDisplay,
		    XtWindow(ProcessedImageDrawingArea),
		    gc_YellowBlack,
		    UxEvent->xmotion.x - 5, UxEvent->xmotion.y - 5,
		    s, strlen(s));
      }
      break;
      /* delete FE node */
    case DELNODE:
      if (Highlighted_FE_node != NULL) {
	HGCursor(B_TRUE);
	DeleteNode(Highlighted_FE_node,(void *)NULL);
	Highlighted_FE_node = NULL;
	RedrawProcDrawingArea();
	HGCursor(B_FALSE);
      }
      ProcActionMode = MODENULL;
      break;
#endif /* defined (CMGUI) */
    case COMPDIST:
      if (DistanceFirstPointDone == B_FALSE) {
	XFillArc(UxDisplay,
		 XtWindow(ProcessedImageDrawingArea),
		 gc_RedBlack,
		 UxEvent->xmotion.x - 2, UxEvent->xmotion.y - 2,
		 4, 4, 0, 360*64 - 1);
	XSync(UxDisplay, B_FALSE);
	DistanceFirstPointX = UxEvent->xmotion.x;
	DistanceFirstPointY = UxEvent->xmotion.y;
	DistanceFirstPointDone = B_TRUE;
      }
      else {
	XDefineCursor(UxDisplay,
		      XtWindow(ProcessedImageShell),
		      leftpointer);
	ProcessedImageCursor = leftpointer;
	XDrawLine(UxDisplay,
		  XtWindow(ProcessedImageDrawingArea),
		  gc_RedBlack, DistanceFirstPointX, DistanceFirstPointY,
		  UxEvent->xmotion.x, UxEvent->xmotion.y);
	XSync(UxDisplay, B_FALSE);
	/* compute and display the distance measure */
	dx = (UxEvent->xmotion.x - DistanceFirstPointX)/ZoomFactor;
	dy = (UxEvent->xmotion.y - DistanceFirstPointY)/ZoomFactor;
	d = sqrt(dx*dx + dy*dy);
	printf("Distance : %.2f pixels\a\n", d);
	/* redraw the processed window */
	RedrawProcDrawingArea();
	ProcActionMode = MODENULL;
      }
      break;
      
      /* Register the starting point of a manually drawn contour */
    case DRAWCLIPRECT:
      DrawClipOn = B_TRUE;
      /* set the start point */
      PreviousCursorPosX    = UxEvent->xmotion.x;
      PreviousCursorPosY    = UxEvent->xmotion.y;
      StartCursorPosX       = UxEvent->xmotion.x;
      StartCursorPosY       = UxEvent->xmotion.y;
      PolygonVerticies[0].x = UxEvent->xmotion.x / ZoomFactor;
      PolygonVerticies[0].y = UxEvent->xmotion.y / ZoomFactor;
      npv = 1;
      break;
    default:
      break;
    }
  }
  UxProcessedImageShellContext = UxSaveCtx;
}

/*******************************************************************************
       The following are callback functions.
*******************************************************************************/

static	void	destroyCB_ProcessedImageShell(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCProcessedImageShell *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxProcessedImageShellContext;
	UxProcessedImageShellContext = UxContext =
			(_UxCProcessedImageShell *) UxGetContext( UxWidget );
	{
	  if (XvgUp)
	    UxPutSet(ProcessedImageTBG, "false");
	  ProcessedImageShellCreated = B_FALSE;
	  if (VerboseFlag)
	    printf("XVG ProcessedImageShell destroyed.\n");
	}
	UxProcessedImageShellContext = UxSaveCtx;
}

static	void	popdownCB_ProcessedImageShell(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCProcessedImageShell *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxProcessedImageShellContext;
	UxProcessedImageShellContext = UxContext =
			(_UxCProcessedImageShell *) UxGetContext( UxWidget );
	{
	UxPutSet(ProcessedImageTBG, "false");
	}
	UxProcessedImageShellContext = UxSaveCtx;
}

static	void	popupCB_ProcessedImageShell(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCProcessedImageShell *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxProcessedImageShellContext;
	UxProcessedImageShellContext = UxContext =
			(_UxCProcessedImageShell *) UxGetContext( UxWidget );
	{
	UxPutSet(ProcessedImageTBG, "true");
	}
	UxProcessedImageShellContext = UxSaveCtx;
}

static	void	exposeCB_ProcImageDrawingArea(
					      Widget wgt, 
					      XtPointer cd, 
					      XtPointer cb)
{
  _UxCProcessedImageShell *UxSaveCtx, *UxContext;
  Widget                  UxWidget = wgt;
  XtPointer               UxClientData = cd;
  XtPointer               UxCallbackArg = cb;
  
  UxSaveCtx = UxProcessedImageShellContext;
  UxProcessedImageShellContext = UxContext =
    (_UxCProcessedImageShell *) UxGetContext( UxWidget );
  {
    XEvent *event;
    int i;
    
    if ((ProcessedImageShellCreated) && IsTrue(UxGetSet(ProcessedImageTBG)))
      {
	if ((ProcessedPixels != NULL) && (ProcessedPixelsX != NULL))
	  {
	    event = ((XmDrawingAreaCallbackStruct *) UxCallbackArg)->event;
	    XPutImage(UxDisplay,
		      XtWindow(ProcessedImageDrawingArea),
		      gc, ProcessedImageXObj,
		      event->xexpose.x, event->xexpose.y,
		      event->xexpose.x, event->xexpose.y,
		      event->xexpose.width, event->xexpose.height);
	  }
	
	/* reset cross hair cursor lines */
	PreviousXpos = -1;
	PreviousYpos = -1;
	
	/* reset points highlight flag */
#if defined (CMGUI)
	Highlighted_node_2d = NULL;
#else /* defined (CMGUI) */
	Highlighted_FE_node = NULL;
#endif /* defined (CMGUI) */
	
	/* execute the overlay functions if any */
	for (i = 0; (i < NOverlayFunctions) && (OverlayLevel > 0); i++)
	  (OverlayFunctions[i])(ProcessedImageDrawingArea);
	
	/* display colorbar */
	if (ColorBarOverlay)
	  DrawColorBar(ProcessedImageDrawingArea, B_TRUE);
      }
  }
  UxProcessedImageShellContext = UxSaveCtx;
}

static void activateCB_FFTShift(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCProcessedImageShell *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxProcessedImageShellContext;
	UxProcessedImageShellContext = UxContext =
			(_UxCProcessedImageShell *) UxGetContext( UxWidget );
	{
	  FFTShift(ProcessedPixels, ProcessedPixels, ImgHeight, ImgWidth);
	  ShowImage(ProcessedPixels);
	}
	UxProcessedImageShellContext = UxSaveCtx;
}

static	void	activateCB_menu4_p1_b1(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCProcessedImageShell *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxProcessedImageShellContext;
	UxProcessedImageShellContext = UxContext =
			(_UxCProcessedImageShell *) UxGetContext( UxWidget );
	{
	  if (ProcActionMode == MODENULL) {
	    ProcActionMode = DRAWCLIPRECT;
	    /* set flags */
	    Abort = B_FALSE;
	    DrawClipOn=B_FALSE;
	    DisplayContourComputed = B_FALSE;
	    /* refresh the processed window */
	    OverlayLevelSave = OverlayLevel;
	    OverlayLevel = 0;
	    RedrawProcDrawingArea();
	  }
	}
	UxProcessedImageShellContext = UxSaveCtx;
}

static	void	activateCB_menu4_p1_b2(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCProcessedImageShell *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxProcessedImageShellContext;
	UxProcessedImageShellContext = UxContext =
			(_UxCProcessedImageShell *) UxGetContext( UxWidget );
	{
	    RedrawProcDrawingArea();
            if (VerboseFlag)
              printf("activateCB_menu4_p1_b2() => OverlayLevel :%d, NOverlays:%d\n",
		OverlayLevel, NOverlayFunctions);
	}
	UxProcessedImageShellContext = UxSaveCtx;
}

static	void	activateCB_menu4_p1_b3(
				       Widget wgt, 
				       XtPointer cd, 
				       XtPointer cb)
{
  _UxCProcessedImageShell *UxSaveCtx, *UxContext;
  Widget                  UxWidget = wgt;
  XtPointer               UxClientData = cd;
  XtPointer               UxCallbackArg = cb;

  UxSaveCtx = UxProcessedImageShellContext;
  UxProcessedImageShellContext = UxContext =
    (_UxCProcessedImageShell *) UxGetContext( UxWidget );
  {
    OverlayLevel = 0;
    /*	    NOverlayFunctions = 0; */
    RedrawProcDrawingArea();
  }
  UxProcessedImageShellContext = UxSaveCtx;
}

static	void	activateCB_menu4_p1_b5(
				       Widget wgt, 
				       XtPointer cd, 
				       XtPointer cb)
{
  _UxCProcessedImageShell *UxSaveCtx, *UxContext;
  Widget                  UxWidget = wgt;
  XtPointer               UxClientData = cd;
  XtPointer               UxCallbackArg = cb;
  
  UxSaveCtx = UxProcessedImageShellContext;
  UxProcessedImageShellContext = UxContext =
    (_UxCProcessedImageShell *) UxGetContext( UxWidget );
  {
    OverlayLevel = 3;
    /*	    NOverlayFunctions = 0; */
    RedrawProcDrawingArea();
  }
  UxProcessedImageShellContext = UxSaveCtx;
}

static	void	activateCB_menu4_p1_b10(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCProcessedImageShell *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxProcessedImageShellContext;
	UxProcessedImageShellContext = UxContext =
			(_UxCProcessedImageShell *) UxGetContext( UxWidget );
	{
	  DisplayContourComputed = B_FALSE;
	  RemoveOverlayFunction(ContourOverlay);
	  RedrawProcDrawingArea();
	}
	UxProcessedImageShellContext = UxSaveCtx;
}

#if defined (CMGUI)
static int proc_window_add_node(struct FE_node *node,
	void *user_data)
{
	struct CM_field_information field_info;
	struct Coordinate_system rect_coord_system;
  struct Node_2d *temp_node;
  char temp_string[100];
	struct FE_field *coordinate_field;
	struct FE_field *digitised_field;
	char *component_names[2]=
	{
		"X","Y"
	};
	FE_value node_x,node_y,node_z;
	int return_code;
  
	return_code=0;
  if (node)
  {
		rect_coord_system.type = RECTANGULAR_CARTESIAN;


		set_CM_field_information(&field_info,CM_COORDINATE_FIELD,(int *)NULL);

		/* we would prefer the digitised(2d) field, but will settle for anything */
			/*???DB.  So that the back-end knows the field type.  Temporary ? */
		if (digitised_field=get_FE_field_manager_matched_field(
			xvg_fe_field_manager,"coordinates_2d_rc",
			GENERAL_FE_FIELD,/*indexer_field*/(struct FE_field *)NULL,
			/*number_of_indexed_values*/0,&field_info,
			&rect_coord_system,FE_VALUE_VALUE,
			/*number_of_components*/2,component_names,
			/*number_of_times*/0,/*time_value_type*/UNKNOWN_VALUE))
		{
			/* look for the digitised coordinates */
			if (for_FE_field_at_node(digitised_field,
				(FE_node_field_iterator_function *)NULL,(void *)NULL,node))
			{
				coordinate_field=digitised_field;
			}
			else
			{
				coordinate_field=get_FE_node_default_coordinate_field(node);
			}
			if (coordinate_field)
			{
				FE_node_get_position_cartesian(node,coordinate_field,&node_x,&node_y,
					&node_z,(FE_value *)NULL);
				sprintf(temp_string,"%i",get_FE_node_cm_node_identifier(node));
				if (temp_node = CREATE(Node_2d)(temp_string,node_x,node_y))
				{
					return_code=
						ADD_OBJECT_TO_LIST(Node_2d)(temp_node,processed_node_list);
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"proc_window_add_node.  Could not create node");
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"proc_window_add_node.  Invalid coordinate node field");
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"proc_window_add_node.  Could not create field");
		}
  }
  else
  {
    display_message(ERROR_MESSAGE,
      "proc_window_add_node.  Invalid node");
  }

	return (return_code);
} /* proc_window_add_node */

static void proc_window_update_callback(Widget data_2d_dialog,
	void *user_data,void *update_callback_data)
{
  struct GROUP(FE_node) *data_group =
    (struct GROUP(FE_node) *)update_callback_data;

  DESTROY_LIST(Node_2d)(&processed_node_list);
  processed_node_list = CREATE_LIST(Node_2d)();
  if (data_group)
  {
		FOR_EACH_OBJECT_IN_GROUP(FE_node)(proc_window_add_node,NULL,data_group);
    /* no selected */
  }
#if defined (OLD_CODE)  
  else
  {
    display_message(ERROR_MESSAGE,
      "proc_window_update_callback.  Invalid data_group");
  }
#endif /* OLD_CODE */
  RedrawProcDrawingArea();
} /* proc_window_update_callback */

static int proc_window_check_selected(struct Node_2d *current_node,
  void *user_data)
{
  struct Data_2d_select_data *select_data =
    (struct Data_2d_select_data *)user_data;
  int found,i,node_number,return_code;
  
	return_code=0;
  if (select_data&&current_node)
  {
		return_code=1;
    /* is it a numbered node */
    if (sscanf(current_node->identifier,"%i",&node_number)==1)
    {
      /* check to see if this node is in the selected list - if not then
				unselect */
      found = FALSE;
      for (i=0;(i<select_data->num_selected)&&(!found);i++)
      {
        if (select_data->selected_nodes[i]==node_number)
        {
          found = TRUE;
        }
      }
      if (found)
      {
        current_node->selected = TRUE;
      }
      else
      {
        current_node->selected = FALSE;
      }
    }
    else
    {
      current_node->selected = FALSE;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,
      "proc_window_check_selected.  Invalid arguments");
  }

	return (return_code);
} /* proc_window_check_selected */

static void proc_window_select_callback(Widget data_2d_dialog,void *user_data,
  void *select_callback_data)
{
  struct Data_2d_select_data *select_data =
    (struct Data_2d_select_data *)select_callback_data;

  if(select_data)
  {
    FOR_EACH_OBJECT_IN_LIST(Node_2d)(proc_window_check_selected,select_data,
      processed_node_list);
    /* redraw the nodes */
    Draw_Nodes_2d(NULL);
			/* THIS is BAD - Draw_Nodes_2D does not use parameter */
    /* should just have changed the colours of the nodes */
    /*    RedrawProcDrawingArea(); */
  }
  else
  {
    display_message(ERROR_MESSAGE,
      "proc_window_select_callback.  Invalid select data");
  }
} /* proc_window_select_callback */

static void proc_window_destroy_callback(Widget data_2d_dialog,void *user_data,
  void *destroy_callback_data)
{
  if(processed_node_list)
  {
    DESTROY_LIST(Node_2d)(&processed_node_list);
    RemoveOverlayFunction(Draw_Nodes_2d);
    /* redraw the processed window */
    RedrawProcDrawingArea();
  }
  else
  {
    display_message(WARNING_MESSAGE,
      "proc_window_destroy_callback.  List should be created");
  }
  RedrawProcDrawingArea();
} /* proc_window_destroy_callback */
#endif /* defined (CMGUI) */

static	void	activateCB_menu4_p1_b6(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
#if defined (CMGUI)
  DATA_2D_PRECISION radius;
  struct Callback_data callback;
	_UxCProcessedImageShell *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxProcessedImageShellContext;
	UxProcessedImageShellContext = UxContext =
    (_UxCProcessedImageShell *) UxGetContext( UxWidget );
	{
    if(data_2d_dialog)
    {
      XtDestroyWidget(data_2d_dialog);
    }
    else
    {
      if(create_data_2d_dialog(&data_2d_dialog,ProcessedImageShell,2,
        xvg_fe_field_manager,xvg_node_manager,xvg_node_group_manager,
				(struct GROUP(FE_node) *)NULL,5.0))
      {
        if(!processed_node_list)
        {
          processed_node_list = CREATE_LIST(Node_2d)();
        }
        else
        {
          display_message(WARNING_MESSAGE,
            "activateCB_menu4_p1_b2.  List should not be created");
        }
        /* register the callbacks */
        callback.procedure=proc_window_update_callback; 
        callback.data=NULL; 
        data_2d_dialog_set_data(data_2d_dialog,DATA_2D_DIALOG_UPDATE_CB,
					&callback); 
        callback.procedure=proc_window_select_callback; 
        callback.data=NULL; 
        data_2d_dialog_set_data(data_2d_dialog,DATA_2D_DIALOG_SELECT_CB,
					&callback); 
        callback.procedure=proc_window_destroy_callback; 
        callback.data=NULL; 
        data_2d_dialog_set_data(data_2d_dialog,DATA_2D_DIALOG_DESTROY_CB,
          &callback);
        /* update the selection radius */
        radius = 5.0/ZoomFactor;
        data_2d_dialog_set_data(data_2d_dialog,DATA_2D_DIALOG_SELECT_RADIUS,
          &radius);
        /* force an update */
        data_2d_dialog_set_data(data_2d_dialog,DATA_2D_DIALOG_DATA,NULL); 
        LoadOverlayFunction(Draw_Nodes_2d);
        /* redraw the processed window */
        RedrawProcDrawingArea();
      }
      else
      {
        display_message(ERROR_MESSAGE,
          "activateCB_menu4_p1_b6.  Could not create data_2d_dialog");
      }
    }
	}
	UxProcessedImageShellContext = UxSaveCtx;
#endif /* defined (CMGUI) */
} /* activateCB_menu4_p1_b6 */

static	void	activateCB_menu4_p1_b16(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCProcessedImageShell *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxProcessedImageShellContext;
	UxProcessedImageShellContext = UxContext =
			(_UxCProcessedImageShell *) UxGetContext( UxWidget );
	{
	  extern Widget InputDialog;
	  InputDialogLoadParms();
	  UxPopupInterface(InputDialog, no_grab);
	}
	UxProcessedImageShellContext = UxSaveCtx;
}

static	void	activateCB_menu4_p1_b15(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCProcessedImageShell *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxProcessedImageShellContext;
	UxProcessedImageShellContext = UxContext =
			(_UxCProcessedImageShell *) UxGetContext( UxWidget );
	{
	  /* check that not in another mode, if not set to distance computation */
	  if (ProcActionMode == MODENULL) {
	    ProcActionMode = COMPDIST;
	    XDefineCursor(UxDisplay, XtWindow(ProcessedImageShell),
	      xhair);
	    ProcessedImageCursor = xhair;
	    ComputeDistanceOn = B_TRUE;
	    DistanceFirstPointDone = B_FALSE;
	  }
	}
	UxProcessedImageShellContext = UxSaveCtx;
}

/*******************************************************************************
       The 'build_' function creates all the widgets
       using the resource values specified in the Property Editor.
*******************************************************************************/

static Widget	_Uxbuild_ProcessedImageShell(void)
{
	MrmType		_UxDummyClass;
	Cardinal	_UxStatus;


	/* Creation of ProcessedImageShell */
	ProcessedImageShell = XtVaCreatePopupShell( "ProcessedImageShell",
			topLevelShellWidgetClass,
			UxTopLevel,
			XmNwidth, 340,
			XmNheight, 280,
			XmNiconName, "PrcImg",
			XmNtitle, "Processed Image",
			XmNmaxHeight, 200,
			XmNmaxWidth, 300,
			RES_CONVERT( XmNtranslations, "" ),
			XmNallowShellResize, B_FALSE,
			NULL );
	XtAddCallback( ProcessedImageShell, XmNdestroyCallback,
		(XtCallbackProc) destroyCB_ProcessedImageShell,
		(XtPointer) UxProcessedImageShellContext );
	XtAddCallback( ProcessedImageShell, XmNpopdownCallback,
		(XtCallbackProc) popdownCB_ProcessedImageShell,
		(XtPointer) UxProcessedImageShellContext );
	XtAddCallback( ProcessedImageShell, XmNpopupCallback,
		(XtCallbackProc) popupCB_ProcessedImageShell,
		(XtPointer) UxProcessedImageShellContext );

	UxPutContext( ProcessedImageShell, (char *) UxProcessedImageShellContext );

	form9 = NULL;
	_UxStatus = MrmFetchWidget( hierarchy,
					"form9",
					ProcessedImageShell,
					&form9,
					&_UxDummyClass );
	if ( _UxStatus != MrmSUCCESS )
	{
		UxMrmFetchError( hierarchy, "form9",
				ProcessedImageShell, _UxStatus );
		return( (Widget) NULL );
	}

	XtManageChild( form9 );

	UxPutContext( form9, (char *) UxProcessedImageShellContext );
	ProcessedImageScrolledWindow =
	  XtNameToWidget( form9, "*ProcessedImageScrolledWindow" );
	UxPutContext( ProcessedImageScrolledWindow,
		     (char *) UxProcessedImageShellContext );
	ProcessedImageDrawingArea =
	  XtNameToWidget( ProcessedImageScrolledWindow,
			 "*ProcessedImageDrawingArea" );
	UxPutContext( ProcessedImageDrawingArea,
		     (char *) UxProcessedImageShellContext );
	menu4 = XtNameToWidget( ProcessedImageDrawingArea, "*menu4" );
	UxPutContext( menu4, (char *) UxProcessedImageShellContext );
	menu4_p1_b1 = XtNameToWidget( menu4, "*menu4_p1_b1" );
	UxPutContext( menu4_p1_b1, (char *) UxProcessedImageShellContext );
	menu4_p1_b2 = XtNameToWidget( menu4, "*menu4_p1_b2" );
	UxPutContext( menu4_p1_b2, (char *) UxProcessedImageShellContext );
	menu4_p1_b3 = XtNameToWidget( menu4, "*menu4_p1_b3" );
	UxPutContext( menu4_p1_b3, (char *) UxProcessedImageShellContext );
	menu4_p1_b5 = XtNameToWidget( menu4, "*menu4_p1_b5" );
	UxPutContext( menu4_p1_b5, (char *) UxProcessedImageShellContext );
	menu4_p1_b4 = XtNameToWidget( menu4, "*menu4_p1_b4" );
	UxPutContext( menu4_p1_b4, (char *) UxProcessedImageShellContext );
	menu4_p1_b6 = XtNameToWidget( menu4, "*menu4_p1_b6" );
	UxPutContext( menu4_p1_b6, (char *) UxProcessedImageShellContext );
	menu4_p1_b10 = XtNameToWidget( menu4, "*menu4_p1_b10" );
	UxPutContext( menu4_p1_b10, (char *) UxProcessedImageShellContext );
	menu4_p1_b13 = XtNameToWidget( menu4, "*menu4_p1_b13" );
	UxPutContext( menu4_p1_b13, (char *) UxProcessedImageShellContext );
	menu4_p1_b12 = XtNameToWidget( menu4, "*menu4_p1_b12" );
	UxPutContext( menu4_p1_b12, (char *) UxProcessedImageShellContext );
	menu4_p1_b25 = XtNameToWidget( menu4, "*menu4_p1_b25" );
	UxPutContext( menu4_p1_b25, (char *) UxProcessedImageShellContext );
	menu4_p1_b16 = XtNameToWidget( menu4, "*menu4_p1_b16" );
	UxPutContext( menu4_p1_b16, (char *) UxProcessedImageShellContext );
	menu4_p1_b15 = XtNameToWidget( menu4, "*menu4_p1_b15" );
	UxPutContext( menu4_p1_b15, (char *) UxProcessedImageShellContext );
	frame7 = XtNameToWidget( form9, "*frame7" );
	UxPutContext( frame7, (char *) UxProcessedImageShellContext );
	ProcessedXSliceDA = XtNameToWidget( frame7, "*ProcessedXSliceDA" );
	UxPutContext(ProcessedXSliceDA, (char *) UxProcessedImageShellContext);
	frame8 = XtNameToWidget( form9, "*frame8" );
	UxPutContext( frame8, (char *) UxProcessedImageShellContext );
	ProcessedYSliceDA = XtNameToWidget( frame8, "*ProcessedYSliceDA" );
	UxPutContext(ProcessedYSliceDA, (char *) UxProcessedImageShellContext);
	frame9 = XtNameToWidget( form9, "*frame9" );
	UxPutContext( frame9, (char *) UxProcessedImageShellContext );
	rowColumn8 = XtNameToWidget( frame9, "*rowColumn8" );
	UxPutContext( rowColumn8, (char *) UxProcessedImageShellContext );
	PXLG = XtNameToWidget( rowColumn8, "*PXLG" );
	UxPutContext( PXLG, (char *) UxProcessedImageShellContext );
	PYLG = XtNameToWidget( rowColumn8, "*PYLG" );
	UxPutContext( PYLG, (char *) UxProcessedImageShellContext );
	PVLG = XtNameToWidget( rowColumn8, "*PVLG" );
	UxPutContext( PVLG, (char *) UxProcessedImageShellContext );

	XtAddCallback( ProcessedImageShell, XmNdestroyCallback,
		(XtCallbackProc) UxDestroyContextCB,
		(XtPointer) UxProcessedImageShellContext);

	XtAddEventHandler(ProcessedImageDrawingArea, ButtonPressMask,
			  B_FALSE,
			  (XtEventHandler) _UxProcessedImageShellMenuPost,
			  (XtPointer) menu4 );

	return ( ProcessedImageShell );
}

/*******************************************************************************
       The following is the 'Interface function' which is the
       external entry point for creating this interface.
       This function should be called from your application or from
       a callback function.
*******************************************************************************/

Widget	create_ProcessedImageShell(void)
{
	Widget                  rtrn;
	_UxCProcessedImageShell *UxContext;
	static int		_Uxinit = 0;

	UxProcessedImageShellContext = UxContext =
		(_UxCProcessedImageShell *) UxNewContext( sizeof(_UxCProcessedImageShell), B_FALSE );


	if ( ! _Uxinit )
	{
		static MrmRegisterArg	_UxMrmNames[] = {
			{ "destroyCB_ProcessedImageShell", 
				(XtPointer) destroyCB_ProcessedImageShell },
			{ "popdownCB_ProcessedImageShell", 
				(XtPointer) popdownCB_ProcessedImageShell },
			{ "popupCB_ProcessedImageShell", 
				(XtPointer) popupCB_ProcessedImageShell },
			{ "exposeCB_ProcImageDrawingArea", 
				(XtPointer) exposeCB_ProcImageDrawingArea },
			{ "activateCB_menu4_p1_b1", 
				(XtPointer) activateCB_menu4_p1_b1 },
			{ "activateCB_menu4_p1_b2", 
				(XtPointer) activateCB_menu4_p1_b2 },
			{ "activateCB_menu4_p1_b3", 
				(XtPointer) activateCB_menu4_p1_b3 },
			{ "activateCB_menu4_p1_b5", 
				(XtPointer) activateCB_menu4_p1_b5 },
			{ "activateCB_menu4_p1_b6", 
				(XtPointer) activateCB_menu4_p1_b6 },
			{ "activateCB_menu4_p1_b10", 
				(XtPointer) activateCB_menu4_p1_b10 },
			{ "activateCB_FFTShift", 
				(XtPointer) activateCB_FFTShift },
			{ "activateCB_menu4_p1_b16", 
				(XtPointer) activateCB_menu4_p1_b16 },
			{ "activateCB_menu4_p1_b15", 
				(XtPointer) activateCB_menu4_p1_b15 }};

		static XtActionsRec	_Uxactions[] = {
			{ "UpdateProcessedWinsize", (XtActionProc) action_UpdateProcessedWinsize },
			{ "ProcMotionNotify", (XtActionProc) action_ProcMotionNotify },
			{ "ProcDABtn1Up", (XtActionProc) action_ProcDABtn1Up },
			{ "ProcDABtn1Down", (XtActionProc) action_ProcDABtn1Down }};

		XtAppAddActions( UxAppContext,
				_Uxactions,
				XtNumber(_Uxactions) );

		MrmRegisterNamesInHierarchy( hierarchy,
					_UxMrmNames,
					XtNumber(_UxMrmNames) );

		_Uxinit = 1;
	}

	rtrn = _Uxbuild_ProcessedImageShell();

	/* reset widget sizes and create image */
	DrawClipOn = B_FALSE;
	DistanceFirstPointDone = B_FALSE;
	ComputeDistanceOn = B_FALSE;
	OverlayLevelSave=0;
	npv=0;
	NFringeRegions=0;
	node_index = 1;
#if defined (CMGUI)
	Highlighted_node_2d=NULL;
#else /* defined (CMGUI) */
	LoadOverlayFunction(Draw_FE_nodes);
	Highlighted_FE_node=NULL;
	NodeToBeChanged=NULL;
	field=NULL;
	node_field=NULL;
	node_field_info=NULL;
	coordinate_node_field_list=NULL;
#endif /* defined (CMGUI) */
	ProcActionMode=MODENULL;
	ProcMax = RawMax;
	ProcMin = RawMin;
	ResizeProcessedImageWidgets(B_TRUE);
	InitProcBuffers();
	if (ProcessedPixels != NULL)
	  MoveDoubles(RawPixels, ProcessedPixels, ImgHeight*ImgWidth);
	CreateProcImage();

	/* check for histograms */
	if (IsTrue(UxGetSet(HistogramTBG))) {
	    Histogram(ProcessedPixels, ProcessedWinHOffset, ProcessedWinVOffset,
		ProcessedWinWidth, ProcessedWinHeight, ImgWidth, ProcMax,
		ProcMin, ProcessedHistogram, ProcessedHistogramRectangles,
		&ProcMean, &ProcSD);
	    DrawHistograms(RawMin, RawMax, ProcMin, ProcMax);
	}
	
	/* set vertical scrollbar callbacks */
	XtVaGetValues(ProcessedImageScrolledWindow, XmNverticalScrollBar, &ProcessedVsbar, NULL);
	XtAddCallback(ProcessedVsbar, XmNincrementCallback, PVSBCallback, NULL);
	XtAddCallback(ProcessedVsbar, XmNdecrementCallback, PVSBCallback, NULL);
	XtAddCallback(ProcessedVsbar, XmNdragCallback, PVSBCallback, NULL);
	XtAddCallback(ProcessedVsbar, XmNtoTopCallback, PVSBCallback, NULL);
	XtAddCallback(ProcessedVsbar, XmNtoBottomCallback, PVSBCallback, NULL);
	XtAddCallback(ProcessedVsbar, XmNpageIncrementCallback, PVSBCallback, NULL);
	XtAddCallback(ProcessedVsbar, XmNpageDecrementCallback, PVSBCallback, NULL);
	
	/* set horizontal scrollbar callbacks */
	XtVaGetValues(ProcessedImageScrolledWindow, XmNhorizontalScrollBar, &ProcessedHsbar, NULL);
	XtAddCallback(ProcessedHsbar, XmNincrementCallback, PHSBCallback, NULL);
	XtAddCallback(ProcessedHsbar, XmNdecrementCallback, PHSBCallback, NULL);
	XtAddCallback(ProcessedHsbar, XmNdragCallback, PHSBCallback, NULL);
	XtAddCallback(ProcessedHsbar, XmNtoTopCallback, PHSBCallback, NULL);
	XtAddCallback(ProcessedHsbar, XmNtoBottomCallback, PHSBCallback, NULL);
	XtAddCallback(ProcessedHsbar, XmNpageIncrementCallback, PHSBCallback, NULL);
	XtAddCallback(ProcessedHsbar, XmNpageDecrementCallback, PHSBCallback, NULL);
	
	/* set created flag */
	ProcessedImageShellCreated = B_TRUE;
	
	/* return */
	return(rtrn);
}

/*******************************************************************************
       END OF FILE
*******************************************************************************/

