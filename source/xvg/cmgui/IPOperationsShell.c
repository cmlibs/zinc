
/*******************************************************************************
	IPOperationsShell.c

       Associated Header file: IPOperationsShell.h
       Associated Resource file: IPOperationsShell.rf
*******************************************************************************/

#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <Mrm/MrmPublic.h>
#include <Xm/MenuShell.h>
#include "UxXt.h"

#include <Xm/List.h>
#include <Xm/ScrolledW.h>
#include <Xm/SeparatoG.h>
#include <Xm/Text.h>
#include <Xm/ToggleBG.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/CascadeBG.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/Form.h>
#include <X11/Shell.h>

/*******************************************************************************
       Includes, Defines, and Global variables from the Declarations Editor:
*******************************************************************************/

#include <stdarg.h>
#include "include/XvgGlobals.h"

/* forward declarations */
static IPOpListItem *CreateNewIPOp(void);
static void BuildEditingMenu(void);

/* globals */
static double *ScratchDouble;
static Widget IPOpEditMenu;
static double *char_bufs[2];
static int    bufindx;

/* externals */
extern swidget RawImageDrawingArea, ProcessedImageDrawingArea, HistogramTBG,
  IPOperationsTBG, RawImageTBG, ProcessedImageTBG, ProcessedImageShell;


/*******************************************************************************
       The following header file defines the context structure.
*******************************************************************************/

#define CONTEXT_MACRO_ACCESS 1
#include "IPOperationsShell.h"
#undef CONTEXT_MACRO_ACCESS

void InitDoubleBuffering()
{
  char_bufs[0] = ProcessedPixels;
  char_bufs[1] = ScratchDouble;
  bufindx = 0;
}

double *GetCurrentOutputIPBuffer()
{
  return(char_bufs[(bufindx + 1) & 1]);
}

/*******************************************************************************
       The following function is an event-handler for posting menus.
*******************************************************************************/

static void	_UxIPOperationsShellMenuPost(
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

Widget	IPOperationsShell;
Widget	IPEditingWindowFW;
Widget	RepsLG;
Widget	StackLG;
Widget	Parm4LG;
Widget	Parm3LG;
Widget	Parm2LG;
Widget	FNameLG;
Widget	Parm1LG;
Widget	RepsTW;

/*******************************************************************************
Auxiliary code from the Declarations Editor:
*******************************************************************************/

#include "custom/IPOperationsShellAux.c"

/*******************************************************************************
       The following are Action functions.
*******************************************************************************/

static	void	action_IPOpEditMenuPullDown(
			Widget wgt, 
			XEvent *ev, 
			String *parm, 
			Cardinal *p_UxNumParams)
{
	Cardinal		UxNumParams = *p_UxNumParams;
	_UxCIPOperationsShell   *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XEvent                  *UxEvent = ev;
	String                  *UxParams = parm;

	UxSaveCtx = UxIPOperationsShellContext;
	UxIPOperationsShellContext = UxContext =
			(_UxCIPOperationsShell *) UxGetContext( UxWidget );
	{
		XmMenuPosition(IPOpEditMenu, (XButtonPressedEvent *) UxEvent);	
		XtManageChild(IPOpEditMenu);
	}
	UxIPOperationsShellContext = UxSaveCtx;
}

/*******************************************************************************
       The following are callback functions.
*******************************************************************************/

static	void	destroyCB_IPOperationsShell(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCIPOperationsShell   *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxIPOperationsShellContext;
	UxIPOperationsShellContext = UxContext =
			(_UxCIPOperationsShell *) UxGetContext( UxWidget );
	{
	  if (XvgUp)
	    UxPutSet(IPOperationsTBG, "false");
	  IPOperationsShellCreated = B_FALSE;
	  if (VerboseFlag)
	    printf("XVG IPOperationsShell destroyed.\n");
	}
	UxIPOperationsShellContext = UxSaveCtx;
}

static	void	popdownCB_IPOperationsShell(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCIPOperationsShell   *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxIPOperationsShellContext;
	UxIPOperationsShellContext = UxContext =
			(_UxCIPOperationsShell *) UxGetContext( UxWidget );
	{
	UxPutSet(IPOperationsTBG, "false");
	
	}
	UxIPOperationsShellContext = UxSaveCtx;
}

static	void	popupCB_IPOperationsShell(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCIPOperationsShell   *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxIPOperationsShellContext;
	UxIPOperationsShellContext = UxContext =
			(_UxCIPOperationsShell *) UxGetContext( UxWidget );
	{
	UxPutSet(IPOperationsTBG, "true");
	
	}
	UxIPOperationsShellContext = UxSaveCtx;
}

static	void	activateCB_menu10_p1_b2(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCIPOperationsShell   *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxIPOperationsShellContext;
	UxIPOperationsShellContext = UxContext =
			(_UxCIPOperationsShell *) UxGetContext( UxWidget );
	{
	    InfoMsg("Use the mouse menu button (rightmost) to activate popup menus");
	}
	UxIPOperationsShellContext = UxSaveCtx;
}

static	void	valueChangedCB_UpdateTBG(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCIPOperationsShell   *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxIPOperationsShellContext;
	UxIPOperationsShellContext = UxContext =
			(_UxCIPOperationsShell *) UxGetContext( UxWidget );
	{
	if (IsTrue(UxGetSet(UpdateTBG)))
	  ImageUpdateFlag = B_TRUE;
	else
	  ImageUpdateFlag = B_FALSE;
	
	}
	UxIPOperationsShellContext = UxSaveCtx;
}

static	void	valueChangedCB_VerboseTBG(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCIPOperationsShell   *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxIPOperationsShellContext;
	UxIPOperationsShellContext = UxContext =
			(_UxCIPOperationsShell *) UxGetContext( UxWidget );
	{
	if (IsTrue(UxGetSet(VerboseTBG)))
	  IPOpExecVerbose = B_TRUE;
	else
	  IPOpExecVerbose = B_FALSE;
	
	}
	UxIPOperationsShellContext = UxSaveCtx;
}

static	void	activateCB_menu1_p4_b10(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCIPOperationsShell   *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxIPOperationsShellContext;
	UxIPOperationsShellContext = UxContext =
			(_UxCIPOperationsShell *) UxGetContext( UxWidget );
	{FFTWindow = HANNINGWINDOW;}
	UxIPOperationsShellContext = UxSaveCtx;
}

static	void	activateCB_menu1_p5_b6(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCIPOperationsShell   *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxIPOperationsShellContext;
	UxIPOperationsShellContext = UxContext =
			(_UxCIPOperationsShell *) UxGetContext( UxWidget );
	{FFTWindow = TRIANGULARWINDOW;}
	UxIPOperationsShellContext = UxSaveCtx;
}

static	void	activateCB_menu1_p5_b7(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCIPOperationsShell   *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxIPOperationsShellContext;
	UxIPOperationsShellContext = UxContext =
			(_UxCIPOperationsShell *) UxGetContext( UxWidget );
	{FFTWindow = HALFSIZETRIANGULARWINDOW;}
	UxIPOperationsShellContext = UxSaveCtx;
}

static	void	activateCB_menu1_p4_b12(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCIPOperationsShell   *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxIPOperationsShellContext;
	UxIPOperationsShellContext = UxContext =
			(_UxCIPOperationsShell *) UxGetContext( UxWidget );
	{FFTWindow = NOWINDOW;}
	UxIPOperationsShellContext = UxSaveCtx;
}

static	void	activateCB_menu8_p1_b2(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCIPOperationsShell   *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxIPOperationsShellContext;
	UxIPOperationsShellContext = UxContext =
			(_UxCIPOperationsShell *) UxGetContext( UxWidget );
	{AddNewOpAfterCallback();}
	UxIPOperationsShellContext = UxSaveCtx;
}

static	void	activateCB_menu8_p1_b3(
				       Widget wgt, 
				       XtPointer cd, 
				       XtPointer cb)
{
  _UxCIPOperationsShell   *UxSaveCtx, *UxContext;
  Widget                  UxWidget = wgt;
  XtPointer               UxClientData = cd;
  XtPointer               UxCallbackArg = cb;
  
  UxSaveCtx = UxIPOperationsShellContext;
  UxIPOperationsShellContext = UxContext =
    (_UxCIPOperationsShell *) UxGetContext( UxWidget );
  {
    /* AddNewOpBefore callback */
    int *s, cnt, i;
    IPOpListItem *p, *new;
    char b[256];
    
    /* check to make sure new item is defined */
    if (CurrentIPOp.f == NULL) {
      ErrorMsg("A new IP Op must first be defined in the editing window!\n");
      return;
    }
    
    /* allocate storage for new list element */
    new = CreateNewIPOp();
    
    /* fill in list element fields */
    LoadIPOp(new, b);
    
    /* check for an empty list, if so initialize it */
    if (UxGetItemCount(scrolledList1) == 0) {
      /* add item to linked list */
      IPOpList = new;
      new->next = NULL;
      new->prev = NULL;
      /* add item to scrolled list */
      XmListAddItem(UxGetWidget(scrolledList1),
		    XmStringCreate(b, XmSTRING_DEFAULT_CHARSET), 1);
    }
    /* insert the new element in the linked list before the selected item */
    else {
      XmListGetSelectedPos(UxGetWidget(scrolledList1), &s, &cnt);
      if (cnt > 0) {
	/* check to see if more than one item selected */
	if (cnt > 1) {
	  ErrorMsg("More than one item selected!\n");
	  return;
	}
	/* add item to linked list */
	for (i = 1, p = IPOpList; i < *s; p = (IPOpListItem *) p->next, i++);
	new->next = (void *) p;
	new->prev = p->prev;
	if (p->prev != NULL)
	  ((IPOpListItem *) p->prev)->next = (void *) new;
	p->prev = (void *) new;
	
	/* check to see if the element was added to the top of the list */
	if (IPOpList == p)
	  IPOpList = new;
	
	/* add item to scrolled list */
	XmListAddItemUnselected(UxGetWidget(scrolledList1),
				XmStringCreate(b, XmSTRING_DEFAULT_CHARSET),
				*s);
	if (s)
	  free(s);
      }
      /* no selected item so insert new element at the beginning of the list */
      else {
	/* add item to linked list */
	new->next = IPOpList;
	new->prev = NULL;
	IPOpList->prev = (void *) new;
	IPOpList = (void *) new;
	/* add item to scrolled list */
	XmListAddItem(UxGetWidget(scrolledList1),
		      XmStringCreate(b, XmSTRING_DEFAULT_CHARSET), 1);
      }
    }
  }
  UxIPOperationsShellContext = UxSaveCtx;
}

static	void	activateCB_menu8_p1_b4(
				       Widget wgt, 
				       XtPointer cd, 
				       XtPointer cb)
{
  _UxCIPOperationsShell   *UxSaveCtx, *UxContext;
  Widget                  UxWidget = wgt;
  XtPointer               UxClientData = cd;
  XtPointer               UxCallbackArg = cb;
  
  UxSaveCtx = UxIPOperationsShellContext;
  UxIPOperationsShellContext = UxContext =
    (_UxCIPOperationsShell *) UxGetContext( UxWidget );
  {
    /* ReplaceSelectedItemWithNewOp callback */
    int *s, cnt, i;
    IPOpListItem *p;
    char b[256];
    XmString xms;
    
    /* check to make sure new item is defined */
    if (CurrentIPOp.f == NULL) {
      ErrorMsg("A new IP Op must first be defined in the editing window!\n");
      return;
    }
    
    /* check to make sure an item is selected */
    XmListGetSelectedPos(UxGetWidget(scrolledList1), &s, &cnt);
    if (cnt < 1) {
      ErrorMsg("No item selected for replacement in the IP Op list!\n");
      if (s)
	free(s);
      return;
    }
    
    /* check for more than one item selected */
    if (cnt > 1) {
      ErrorMsg("More than one item selected for replacement in IP Op list!\n");
      if (s)
	free(s);
      return;
    }
    
    /* fill in list element fields */
    for (i = 1, p = IPOpList; i < *s; p = (IPOpListItem *) p->next, i++);
    LoadIPOp(p, b);
    
    /* replace item in the scrolled list */
    xms = XmStringCreate(b, XmSTRING_DEFAULT_CHARSET);
    XmListReplaceItemsPos(UxGetWidget(scrolledList1), &xms, 1, *s);
    
    /* print out current liked list contents */
    XmStringFree(xms);
    if (s)
      free(s);
  }
  UxIPOperationsShellContext = UxSaveCtx;
}

static	void	activateCB_menu8_p1_b5(
				       Widget wgt, 
				       XtPointer cd, 
				       XtPointer cb)
{
  _UxCIPOperationsShell   *UxSaveCtx, *UxContext;
  Widget                  UxWidget = wgt;
  XtPointer               UxClientData = cd;
  XtPointer               UxCallbackArg = cb;
  
  UxSaveCtx = UxIPOperationsShellContext;
  UxIPOperationsShellContext = UxContext =
    (_UxCIPOperationsShell *) UxGetContext( UxWidget );
  {
    int *s, cnt, i, j;
    IPOpListItem *p;
    
    /* check to make sure an item is selected */
    XmListGetSelectedPos(UxGetWidget(scrolledList1), &s, &cnt);
    if (cnt < 1) {
      ErrorMsg("No item selected for removal in the IP Op list!\n");
      if (s)
	free(s);
      return;
    }
    
    /* find the position of the first element to be deleted */
    for (i = 1, p = IPOpList; i < *s; p = (IPOpListItem *) p->next, i++);
    
    /* loop to delete selected items */
    for (j = 0; j < cnt; p = (IPOpListItem *) p->next, j++) {
      /* remove elements from linked list */
      if (p->prev != NULL)
	((IPOpListItem *) p->prev)->next = (void *) p->next;
      if (p->next != NULL)
	((IPOpListItem *) p->next)->prev = (void *) p->prev;
      if (p == IPOpList)
	IPOpList = p->next;
    }
    
    /* delete item from the scrolled list */
    XmListDeleteItemsPos(UxGetWidget(scrolledList1), cnt, *s);
    
    /* print out current liked list contents */
    if (s)
      free(s);
  }
  UxIPOperationsShellContext = UxSaveCtx;
}

static	void	activateCB_ExecSelectedOpPBG(
					     Widget wgt, 
					     XtPointer cd, 
					     XtPointer cb)
{
  _UxCIPOperationsShell   *UxSaveCtx, *UxContext;
  Widget                  UxWidget = wgt;
  XtPointer               UxClientData = cd;
  XtPointer               UxCallbackArg = cb;
  
  UxSaveCtx = UxIPOperationsShellContext;
  UxIPOperationsShellContext = UxContext =
    (_UxCIPOperationsShell *) UxGetContext( UxWidget );
  {
    /* EXEC OPs button CALLBACK */
    
    XmStringTable List;
    XmString ListItem;
    IPOpListItem *p, *last;
    int i, j, its, ListItemCount, go, lcnt, *l;
    char *s, cbuf[512];
    
    /* check for correct initial state */
    if ((RawPixels == NULL) || (ProcessedPixels == NULL))
      return;
    
    /* check to make sure at least one item is selected */
    XmListGetSelectedPos(UxGetWidget(scrolledList1), &l, &lcnt);
    if (lcnt < 1) {
      ErrorMsg("No items selected for execution in the IP Op list!\n");
      if (l)
	free(l);
      return;
    }
    
    /* set up double buffer system for image processing */
    InitDoubleBuffering();
    
    /* put up hourglass cursor */
    HGCursor(B_TRUE);
    
    /* fetch the current scrolled list items in a table */
    XtVaGetValues(UxGetWidget(scrolledList1), XmNitems, &List, NULL);
    
    /* check for consistency */
    XtVaGetValues(scrolledList1, XmNitemCount, &ListItemCount, NULL);
    for (i = 0, p = IPOpList; p != NULL; p = p->next, i++);
    if (i != ListItemCount) {
      sprintf(cbuf,
	      "Abort : Scrolled list has length %d != IPOp list length (%d)",
	      ListItemCount, i);
      draw_mesg(cbuf);
      go = B_FALSE;
    }
    else
      go = B_TRUE;
    
    /* loop to call all image processing functions selected */
    XmListDeselectAllItems(UxGetWidget(scrolledList1));
    for (j = 0, Abort=B_FALSE; (j < lcnt) && (Abort == B_FALSE) && go; j++) {
      /* fetch start time */
      XvgStartTimer();
      
      /* loop to find the position of this IP function */
      for (i = 1, p = IPOpList; i < l[j]; p = (IPOpListItem *) p->next, i++);
      last = p;
      
      /* scroll list and select highlight current operation in the list */
      XmListDeselectAllItems(UxGetWidget(scrolledList1));
      XmListSelectPos(UxGetWidget(scrolledList1), i, B_FALSE);
      XmListSetBottomPos(UxGetWidget(scrolledList1), i);
      XmUpdateDisplay(UxGetWidget(scrolledList1));
      
      /* loop for repetitions */
      for (its = 0; (its < p->reps) && (Abort == B_FALSE); its++) {
	(* p->f)(char_bufs[bufindx],
		 char_bufs[(bufindx + 1) & 1], ImgHeight, ImgWidth,
		 p->ParmPtrs);
	
	if (Abort == B_FALSE) {
	  bufindx = (bufindx + 1) & 1;
	  
	  if (ImageUpdateFlag) {
	    DynamicRange(char_bufs[bufindx], ImgHeight*ImgWidth,
			 &ProcMax, &ProcMin);
	    RealtoX(char_bufs[bufindx], ProcessedPixelsX, ImgHeight, ImgWidth,
		    ProcMax, ProcMin, MaxScaleVal, MinScaleVal, ZoomFactor);
	    if (IsTrue(UxGetSet(ProcessedImageTBG))) 
	      XPutImage(UxDisplay, XtWindow(ProcessedImageDrawingArea), gc,
		        ProcessedImageXObj, 0, 0, 0, 0, ImgWidth*ZoomFactor,
			ImgHeight*ZoomFactor);
	    XSync(UxDisplay, B_FALSE);
	  }
	}
      }
      
      /* fix edges */
      if ((p->EdgeFixFlag) && (Abort == B_FALSE)){
	DynamicRange(char_bufs[bufindx],ImgHeight*ImgWidth,&ProcMax,&ProcMin);
	FixEdges(char_bufs[bufindx], char_bufs[bufindx], ImgHeight, ImgWidth,
		 p->ParmPtrs, ProcMax, ProcMin);
      }
      
      if (Abort == B_FALSE) {
	/* fetch stop time, compute elapsed time, display in the list */
	XvgStopTimer();
	if (XmStringGetLtoR(List[i-1], XmSTRING_DEFAULT_CHARSET, &s)
	    == B_FALSE) {
	  Abort = B_TRUE;
	  draw_mesg("Abort : Error getting string in ExecOps callback!");
	  return;
	}
	ListItem = XmStringConcat(XmStringCreate(s,XmSTRING_DEFAULT_CHARSET),
				  XmStringCreate(ElapsedTime(s),
						 XmSTRING_DEFAULT_CHARSET));
	XmListReplaceItemsPos(UxGetWidget(scrolledList1), &ListItem, 1, i);
	XmUpdateDisplay(UxGetWidget(scrolledList1));
	
	/* free storage */
	XmStringFree(ListItem);
	XtFree(s);
	
	/* record last IP Op */
	last = p;
      }
    }
    
    if (Abort == B_FALSE) {
      draw_mesg("Ok");
      if ((i+1) <= ListItemCount) {
	XmListSelectPos(UxGetWidget(scrolledList1), i+1, B_FALSE);
	XmListSetBottomPos(UxGetWidget(scrolledList1), i);
	XmUpdateDisplay(UxGetWidget(scrolledList1));
      }
    }
    
    /* move pixels to output buffer and convert to X indicies */
    if (last->EdgeFixFlag == B_FALSE)
      DynamicRange(char_bufs[bufindx], ImgHeight*ImgWidth, &ProcMax,&ProcMin);
    if (char_bufs[bufindx] != ProcessedPixels)
      MoveDoubles(char_bufs[bufindx], ProcessedPixels, ImgHeight*ImgWidth);
    RealtoX(ProcessedPixels, ProcessedPixelsX, ImgHeight, ImgWidth,
	    ProcMax, ProcMin, MaxScaleVal, MinScaleVal, ZoomFactor);
    
    /* load image into drawing area */
    if (IsTrue(UxGetSet(ProcessedImageTBG))) 
      XPutImage(UxDisplay, XtWindow(ProcessedImageDrawingArea), gc,
		ProcessedImageXObj, 0, 0, 0, 0, ImgWidth*ZoomFactor,
		ImgHeight*ZoomFactor);
    
    /* execute the overlay functions if any */
    for (i = 0; (i < NOverlayFunctions) && (Abort == B_FALSE)
	 && (OverlayLevel > 0); i++) {
      if (IsTrue(UxGetSet(RawImageTBG))) 
	(OverlayFunctions[i])(RawImageDrawingArea);
      if (IsTrue(UxGetSet(ProcessedImageTBG))) {
#ifdef DEBUG
	printf("activateCB_ExecSelectedOpPBG() : drawing overlay %d\n",
	       (int) (OverlayFunctions[i]));
#endif
	(OverlayFunctions[i])(ProcessedImageDrawingArea);
      }
    }
    
    /* colorbar overlay */
    if (ColorBarOverlay) {
      DrawColorBar(RawImageDrawingArea, RawImageShellCreated);
      DrawColorBar(ProcessedImageDrawingArea, ProcessedImageShellCreated);
    }
    
    /* compute and display the histograms if required */
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
    
    
    /* restore regular cursor and internal state flags */
    printf("\a");
    fflush(stdout);
    HGCursor(B_FALSE);
    
  }
  UxIPOperationsShellContext = UxSaveCtx;
}

static	void	activateCB_ExecAllPBG(
				      Widget wgt, 
				      XtPointer cd, 
				      XtPointer cb)
{
  _UxCIPOperationsShell   *UxSaveCtx, *UxContext;
  Widget                  UxWidget = wgt;
  XtPointer               UxClientData = cd;
  XtPointer               UxCallbackArg = cb;
  
  UxSaveCtx = UxIPOperationsShellContext;
  UxIPOperationsShellContext = UxContext =
    (_UxCIPOperationsShell *) UxGetContext( UxWidget );
  {
    /* EXEC ALL button CALLBACK */
    
    XmStringTable List;
    XmString ListItem;
    IPOpListItem *p, *last;
    int i, j, its, ListItemCount, go, lcnt, *l;
    char *s, cbuf[512];
    
    /* check for correct initial state */
    if ((RawPixels == NULL) || (ProcessedPixels == NULL))
      return;
    
    /* set up double buffer system for image processing */
    InitDoubleBuffering();
    
    /* put up hourglass cursor */
    HGCursor(B_TRUE);
    
    /* load raw data into first buffer */
    MoveDoubles(RawPixels, char_bufs[bufindx], ImgHeight*ImgWidth);
    
    /* fetch the current scrolled list items in a table */
    XtVaGetValues(UxGetWidget(scrolledList1), XmNitems, &List, NULL);
    
    /* check for consistency */
    XtVaGetValues(UxGetWidget(scrolledList1), XmNitemCount,
		  &ListItemCount, NULL);
    for (i = 0, p = IPOpList; p != NULL; p = p->next, i++);
    if (i != ListItemCount) {
      sprintf(cbuf,
	      "Abort : Scrolled list has length %d != IPOp list length (%d)",
	      ListItemCount, i);
      draw_mesg(cbuf);
      go = B_FALSE;
    }
    else
      go = B_TRUE;
    
    /* loop to call all image processing functions in the array */
    XmListDeselectAllItems(UxGetWidget(scrolledList1));
    for (i= 1, p = IPOpList, Abort=B_FALSE, last = IPOpList;
	 (p != NULL) && (Abort == B_FALSE) && go; p = p->next, i++) {
      /* fetch start time */
      XvgStartTimer();
      
      /* scroll list and select highlight current operation in the list */
      XmListDeselectAllItems(UxGetWidget(scrolledList1));
      XmListSelectPos(UxGetWidget(scrolledList1), i, B_FALSE);
      XmListSetBottomPos(UxGetWidget(scrolledList1), i);
      XmUpdateDisplay(UxGetWidget(scrolledList1));
      
      /* loop for repetitions */
      for (its = 0; (its < p->reps) && (Abort == B_FALSE); its++) {
	(* p->f)(char_bufs[bufindx],
		 char_bufs[(bufindx + 1) & 1], ImgHeight, ImgWidth,
		 p->ParmPtrs);
	
	if (Abort == B_FALSE) {
	  bufindx = (bufindx + 1) & 1;
	  
	  if (ImageUpdateFlag) {
	    DynamicRange(char_bufs[bufindx], ImgHeight*ImgWidth,
			 &ProcMax,&ProcMin);
	    RealtoX(char_bufs[bufindx], ProcessedPixelsX,
		    ImgHeight, ImgWidth,
		    ProcMax, ProcMin, MaxScaleVal, MinScaleVal,
		    ZoomFactor);
	    if (IsTrue(UxGetSet(ProcessedImageTBG))) 
	      XPutImage(UxDisplay,
			XtWindow(UxGetWidget(ProcessedImageDrawingArea)),
			gc, ProcessedImageXObj, 0, 0, 0, 0,
			ImgWidth*ZoomFactor, ImgHeight*ZoomFactor);
	    XSync(UxDisplay, B_FALSE);
	  }
	}
      }
      
      /* fix edges */
      if ((p->EdgeFixFlag) && (Abort == B_FALSE)){
	DynamicRange(char_bufs[bufindx],ImgHeight*ImgWidth,
		     &ProcMax,&ProcMin);
	FixEdges(char_bufs[bufindx], char_bufs[bufindx],
		 ImgHeight, ImgWidth,
		 p->ParmPtrs, ProcMax, ProcMin);
      }
      
      if (Abort == B_FALSE) {
	/* fetch stop time, compute elapsed time, display in the list */
	XvgStopTimer();
	if (XmStringGetLtoR(List[i-1], XmSTRING_DEFAULT_CHARSET, &s)
	    == B_FALSE) {
	  Abort = B_TRUE;
	  draw_mesg("Abort : Error getting string in ExecAll callback!");
	  return;
	}
	ListItem=XmStringConcat(XmStringCreate(s,XmSTRING_DEFAULT_CHARSET),
				XmStringCreate(ElapsedTime(s),
					       XmSTRING_DEFAULT_CHARSET));
	XmListReplaceItemsPos(UxGetWidget(scrolledList1), &ListItem, 1, i);
	XmUpdateDisplay(UxGetWidget(scrolledList1));
	
	/* free storage */
	XmStringFree(ListItem);
	XtFree(s);
	
	/* record last IP Op */
	last = p;
      }
    }
    
    if (Abort == B_FALSE) {
      /* reset list display to initial state */
      XmListDeselectPos(UxGetWidget(scrolledList1), i-1);
      XmListSetPos(UxGetWidget(scrolledList1), 1);
      XmUpdateDisplay(UxGetWidget(scrolledList1));
      draw_mesg("Ok");
    }
    
    /* move pixels to output buffer and convert to X indicies */
    if (last->EdgeFixFlag == B_FALSE)
      DynamicRange(char_bufs[bufindx], ImgHeight*ImgWidth,
		   &ProcMax,&ProcMin);
    if (char_bufs[bufindx] != ProcessedPixels)
      MoveDoubles(char_bufs[bufindx], ProcessedPixels, ImgHeight*ImgWidth);
    RealtoX(ProcessedPixels, ProcessedPixelsX, ImgHeight, ImgWidth,
	    ProcMax, ProcMin, MaxScaleVal, MinScaleVal, ZoomFactor);
    
    /* load image into drawing area */
    if (IsTrue(UxGetSet(ProcessedImageTBG))) 
      XPutImage(UxDisplay,
		XtWindow(UxGetWidget(ProcessedImageDrawingArea)), gc,
		ProcessedImageXObj, 0, 0, 0, 0,
		ImgWidth*ZoomFactor, ImgHeight*ZoomFactor);
    
    /* execute the overlay functions if any */
    for (i = 0; (i < NOverlayFunctions) && (Abort == B_FALSE)
	 && (OverlayLevel > 0); i++) {
      if (IsTrue(UxGetSet(RawImageTBG))) 
	(OverlayFunctions[i])(RawImageDrawingArea);
      if (IsTrue(UxGetSet(ProcessedImageTBG))) 
	(OverlayFunctions[i])(ProcessedImageDrawingArea);
    }
    
    /* colorbar overlay */
    if (ColorBarOverlay) {
      DrawColorBar(RawImageDrawingArea, RawImageShellCreated);
      DrawColorBar(ProcessedImageDrawingArea, ProcessedImageShellCreated);
    }
    
    /* compute and display the histograms if required */
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
    
    
    /* restore regular cursor and internal state flags */
    printf("\a");
    fflush(stdout);
    HGCursor(B_FALSE);
    
  }
  UxIPOperationsShellContext = UxSaveCtx;
}

static	void	activateCB_Reset(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
  _UxCIPOperationsShell   *UxSaveCtx, *UxContext;
  Widget                  UxWidget = wgt;
  XtPointer               UxClientData = cd;
  XtPointer               UxCallbackArg = cb;
  
  UxSaveCtx = UxIPOperationsShellContext;
  UxIPOperationsShellContext = UxContext =
    (_UxCIPOperationsShell *) UxGetContext( UxWidget );
  {
    /* RESET button CALLBACK */
    
    IPOpListItem *p;
    
    /* check for correct initial state */
    if ((RawPixels == NULL) || (ProcessedPixels == NULL))
      return;
    
    /* put up hourglass cursor */
    HGCursor(B_TRUE);
    
    /* reset list display to initial state */
    XmListDeselectAllItems(UxGetWidget(scrolledList1));
    XmListSelectPos(UxGetWidget(scrolledList1), 1, B_FALSE);
    XmListSetPos(UxGetWidget(scrolledList1), 1);
    
    /* set flags */
    BSplineComputed=B_FALSE;
	
    /* load raw data into processed buffer & load image into drawing area*/
    MoveDoubles(RawPixels, ProcessedPixels, ImgHeight*ImgWidth);
    ProcMax = RawMax;
    ProcMin = RawMin;
    RealtoX(RawPixels, RawPixelsX, ImgHeight, ImgWidth, RawMax,
	    RawMin, HIST_SCALE_MAX, HIST_SCALE_MIN, ZoomFactor);
    RealtoX(ProcessedPixels, ProcessedPixelsX, ImgHeight, ImgWidth,
	    ProcMax, ProcMin, MaxScaleVal, MinScaleVal, ZoomFactor);
    if (IsTrue(UxGetSet(RawImageTBG))) 
      XPutImage(UxDisplay, XtWindow(UxGetWidget(RawImageDrawingArea)), gc,
		RawImageXObj, 0, 0, 0, 0, ImgWidth*ZoomFactor,
		ImgHeight*ZoomFactor);
    if (IsTrue(UxGetSet(ProcessedImageTBG))) 
      XPutImage(UxDisplay, XtWindow(ProcessedImageDrawingArea), gc,
		ProcessedImageXObj, 0, 0, 0, 0, ImgWidth*ZoomFactor,
		ImgHeight*ZoomFactor);
    
    /* colorbar overlay */
    if (ColorBarOverlay) {
      DrawColorBar(RawImageDrawingArea, RawImageShellCreated);
      DrawColorBar(ProcessedImageDrawingArea, ProcessedImageShellCreated);
    }
    
    /* compute and display the histograms if required */
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
    
    /* reset internal state */
    Abort = B_FALSE;
    draw_mesg("Ok");
    
    /* restore regular cursor */
    HGCursor(B_FALSE);
    
  }
  UxIPOperationsShellContext = UxSaveCtx;
}

/*******************************************************************************
       The 'build_' function creates all the widgets
       using the resource values specified in the Property Editor.
*******************************************************************************/

static Widget	_Uxbuild_IPOperationsShell(void)
{
	MrmType		_UxDummyClass;
	Cardinal	_UxStatus;


	/* Creation of IPOperationsShell */
	IPOperationsShell = XtVaCreatePopupShell( "IPOperationsShell",
			topLevelShellWidgetClass,
			UxTopLevel,
			XmNwidth, 750,
			XmNheight, 440,
			XmNiconName, "IP Operations",
			XmNtitle, "IP Operations",
			XmNmaxWidth, -1,
			XmNminHeight, 300,
			XmNminWidth, 450,
			NULL );
	XtAddCallback( IPOperationsShell, XmNdestroyCallback,
		(XtCallbackProc) destroyCB_IPOperationsShell,
		(XtPointer) UxIPOperationsShellContext );
	XtAddCallback( IPOperationsShell, XmNpopdownCallback,
		(XtCallbackProc) popdownCB_IPOperationsShell,
		(XtPointer) UxIPOperationsShellContext );
	XtAddCallback( IPOperationsShell, XmNpopupCallback,
		(XtCallbackProc) popupCB_IPOperationsShell,
		(XtPointer) UxIPOperationsShellContext );

	UxPutContext( IPOperationsShell, (char *) UxIPOperationsShellContext );

	form6 = NULL;
	_UxStatus = MrmFetchWidget( hierarchy,
					"form6",
					IPOperationsShell,
					&form6,
					&_UxDummyClass );
	if ( _UxStatus != MrmSUCCESS )
	{
		UxMrmFetchError( hierarchy, "form6",
				IPOperationsShell, _UxStatus );
		return( (Widget) NULL );
	}

	XtManageChild( form6 );

	UxPutContext( form6, (char *) UxIPOperationsShellContext );
	menu10 = XtNameToWidget( form6, "*menu10" );
	UxPutContext( menu10, (char *) UxIPOperationsShellContext );
	menu10_top_b1 = XtNameToWidget( menu10, "*menu10_top_b1" );
	UxPutContext( menu10_top_b1, (char *) UxIPOperationsShellContext );
	menu10_p1 = XtNameToWidget( menu10, "*menu10_p1" );
	UxPutContext( menu10_p1, (char *) UxIPOperationsShellContext );
	menu10_p1_b2 = XtNameToWidget( menu10_p1, "*menu10_p1_b2" );
	UxPutContext( menu10_p1_b2, (char *) UxIPOperationsShellContext );
	IPEditingWindowFW = XtNameToWidget( form6, "*IPEditingWindowFW" );
	UxPutContext( IPEditingWindowFW, (char *) UxIPOperationsShellContext );
	labelGadget6 = XtNameToWidget( IPEditingWindowFW, "*labelGadget6" );
	UxPutContext( labelGadget6, (char *) UxIPOperationsShellContext );
	labelGadget9 = XtNameToWidget( IPEditingWindowFW, "*labelGadget9" );
	UxPutContext( labelGadget9, (char *) UxIPOperationsShellContext );
	RepsLG = XtNameToWidget( IPEditingWindowFW, "*RepsLG" );
	UxPutContext( RepsLG, (char *) UxIPOperationsShellContext );
	UpdateTBG = XtNameToWidget( IPEditingWindowFW, "*UpdateTBG" );
	UxPutContext( UpdateTBG, (char *) UxIPOperationsShellContext );
	VerboseTBG = XtNameToWidget( IPEditingWindowFW, "*VerboseTBG" );
	UxPutContext( VerboseTBG, (char *) UxIPOperationsShellContext );
	StackLG = XtNameToWidget( IPEditingWindowFW, "*StackLG" );
	UxPutContext( StackLG, (char *) UxIPOperationsShellContext );
	Parm4LG = XtNameToWidget( IPEditingWindowFW, "*Parm4LG" );
	UxPutContext( Parm4LG, (char *) UxIPOperationsShellContext );
	Parm3LG = XtNameToWidget( IPEditingWindowFW, "*Parm3LG" );
	UxPutContext( Parm3LG, (char *) UxIPOperationsShellContext );
	Parm2LG = XtNameToWidget( IPEditingWindowFW, "*Parm2LG" );
	UxPutContext( Parm2LG, (char *) UxIPOperationsShellContext );
	FNameLG = XtNameToWidget( IPEditingWindowFW, "*FNameLG" );
	UxPutContext( FNameLG, (char *) UxIPOperationsShellContext );
	Parm1LG = XtNameToWidget( IPEditingWindowFW, "*Parm1LG" );
	UxPutContext( Parm1LG, (char *) UxIPOperationsShellContext );
	Parm1TW = XtNameToWidget( IPEditingWindowFW, "*Parm1TW" );
	UxPutContext( Parm1TW, (char *) UxIPOperationsShellContext );
	Parm2TW = XtNameToWidget( IPEditingWindowFW, "*Parm2TW" );
	UxPutContext( Parm2TW, (char *) UxIPOperationsShellContext );
	Parm3TW = XtNameToWidget( IPEditingWindowFW, "*Parm3TW" );
	UxPutContext( Parm3TW, (char *) UxIPOperationsShellContext );
	Parm4TW = XtNameToWidget( IPEditingWindowFW, "*Parm4TW" );
	UxPutContext( Parm4TW, (char *) UxIPOperationsShellContext );
	RepsTW = XtNameToWidget( IPEditingWindowFW, "*RepsTW" );
	UxPutContext( RepsTW, (char *) UxIPOperationsShellContext );
	rowColumn7 = XtNameToWidget( IPEditingWindowFW, "*rowColumn7" );
	UxPutContext( rowColumn7, (char *) UxIPOperationsShellContext );
	FFTWindowMenu = XtNameToWidget( rowColumn7, "*FFTWindowMenu" );
	UxPutContext( FFTWindowMenu, (char *) UxIPOperationsShellContext );
	menu1_p5 = XtNameToWidget( rowColumn7, "*menu1_p5" );
	UxPutContext( menu1_p5, (char *) UxIPOperationsShellContext );
	menu1_p4_b10 = XtNameToWidget( menu1_p5, "*menu1_p4_b10" );
	UxPutContext( menu1_p4_b10, (char *) UxIPOperationsShellContext );
	menu1_p4_b11 = XtNameToWidget( menu1_p5, "*menu1_p4_b11" );
	UxPutContext( menu1_p4_b11, (char *) UxIPOperationsShellContext );
	menu1_p5_b6 = XtNameToWidget( menu1_p5, "*menu1_p5_b6" );
	UxPutContext( menu1_p5_b6, (char *) UxIPOperationsShellContext );
	menu1_p5_b7 = XtNameToWidget( menu1_p5, "*menu1_p5_b7" );
	UxPutContext( menu1_p5_b7, (char *) UxIPOperationsShellContext );
	menu1_p4_b13 = XtNameToWidget( menu1_p5, "*menu1_p4_b13" );
	UxPutContext( menu1_p4_b13, (char *) UxIPOperationsShellContext );
	menu1_p4_b12 = XtNameToWidget( menu1_p5, "*menu1_p4_b12" );
	UxPutContext( menu1_p4_b12, (char *) UxIPOperationsShellContext );
	StatusLG = XtNameToWidget( IPEditingWindowFW, "*StatusLG" );
	UxPutContext( StatusLG, (char *) UxIPOperationsShellContext );
	separatorGadget4 = XtNameToWidget( form6, "*separatorGadget4" );
	UxPutContext( separatorGadget4, (char *) UxIPOperationsShellContext );
	labelGadget4 = XtNameToWidget( form6, "*labelGadget4" );
	UxPutContext( labelGadget4, (char *) UxIPOperationsShellContext );
	scrolledWindow1 = XtNameToWidget( form6, "*scrolledWindow1" );
	UxPutContext( scrolledWindow1, (char *) UxIPOperationsShellContext );
	scrolledList1 = XtNameToWidget( scrolledWindow1, "*scrolledList1" );
	UxPutContext( scrolledList1, (char *) UxIPOperationsShellContext );
	menu8 = XtNameToWidget( scrolledList1, "*menu8" );
	UxPutContext( menu8, (char *) UxIPOperationsShellContext );
	menu8_p1_title1 = XtNameToWidget( menu8, "*menu8_p1_title1" );
	UxPutContext( menu8_p1_title1, (char *) UxIPOperationsShellContext );
	menu8_p1_b1 = XtNameToWidget( menu8, "*menu8_p1_b1" );
	UxPutContext( menu8_p1_b1, (char *) UxIPOperationsShellContext );
	menu8_p1_b2 = XtNameToWidget( menu8, "*menu8_p1_b2" );
	UxPutContext( menu8_p1_b2, (char *) UxIPOperationsShellContext );
	menu8_p1_b3 = XtNameToWidget( menu8, "*menu8_p1_b3" );
	UxPutContext( menu8_p1_b3, (char *) UxIPOperationsShellContext );
	menu8_p1_b4 = XtNameToWidget( menu8, "*menu8_p1_b4" );
	UxPutContext( menu8_p1_b4, (char *) UxIPOperationsShellContext );
	menu8_p1_b5 = XtNameToWidget( menu8, "*menu8_p1_b5" );
	UxPutContext( menu8_p1_b5, (char *) UxIPOperationsShellContext );
	ExecSelectedOpPBG = XtNameToWidget( form6, "*ExecSelectedOpPBG" );
	UxPutContext( ExecSelectedOpPBG, (char *) UxIPOperationsShellContext );
	ExecAllPBG = XtNameToWidget( form6, "*ExecAllPBG" );
	UxPutContext( ExecAllPBG, (char *) UxIPOperationsShellContext );
	Reset = XtNameToWidget( form6, "*Reset" );
	UxPutContext( Reset, (char *) UxIPOperationsShellContext );

	XtAddCallback( IPOperationsShell, XmNdestroyCallback,
		(XtCallbackProc) UxDestroyContextCB,
		(XtPointer) UxIPOperationsShellContext);

	XtAddEventHandler(scrolledList1, ButtonPressMask,
			B_FALSE, (XtEventHandler) _UxIPOperationsShellMenuPost, (XtPointer) menu8 );

	return ( IPOperationsShell );
}

/*******************************************************************************
       The following is the 'Interface function' which is the
       external entry point for creating this interface.
       This function should be called from your application or from
       a callback function.
*******************************************************************************/

Widget	create_IPOperationsShell(void)
{
	Widget                  rtrn;
	_UxCIPOperationsShell   *UxContext;
	static int		_Uxinit = 0;

	UxIPOperationsShellContext = UxContext =
		(_UxCIPOperationsShell *) UxNewContext( sizeof(_UxCIPOperationsShell), B_FALSE );


	if ( ! _Uxinit )
	{
		static MrmRegisterArg	_UxMrmNames[] = {
			{ "destroyCB_IPOperationsShell", 
				(XtPointer) destroyCB_IPOperationsShell },
			{ "popdownCB_IPOperationsShell", 
				(XtPointer) popdownCB_IPOperationsShell },
			{ "popupCB_IPOperationsShell", 
				(XtPointer) popupCB_IPOperationsShell },
			{ "activateCB_menu10_p1_b2", 
				(XtPointer) activateCB_menu10_p1_b2 },
			{ "valueChangedCB_UpdateTBG", 
				(XtPointer) valueChangedCB_UpdateTBG },
			{ "valueChangedCB_VerboseTBG", 
				(XtPointer) valueChangedCB_VerboseTBG },
			{ "activateCB_menu1_p4_b10", 
				(XtPointer) activateCB_menu1_p4_b10 },
			{ "activateCB_menu1_p5_b6", 
				(XtPointer) activateCB_menu1_p5_b6 },
			{ "activateCB_menu1_p5_b7", 
				(XtPointer) activateCB_menu1_p5_b7 },
			{ "activateCB_menu1_p4_b12", 
				(XtPointer) activateCB_menu1_p4_b12 },
			{ "activateCB_menu8_p1_b2", 
				(XtPointer) activateCB_menu8_p1_b2 },
			{ "activateCB_menu8_p1_b3", 
				(XtPointer) activateCB_menu8_p1_b3 },
			{ "activateCB_menu8_p1_b4", 
				(XtPointer) activateCB_menu8_p1_b4 },
			{ "activateCB_menu8_p1_b5", 
				(XtPointer) activateCB_menu8_p1_b5 },
			{ "activateCB_ExecSelectedOpPBG", 
				(XtPointer) activateCB_ExecSelectedOpPBG },
			{ "activateCB_ExecAllPBG", 
				(XtPointer) activateCB_ExecAllPBG },
			{ "activateCB_Reset", 
				(XtPointer) activateCB_Reset }};

		static XtActionsRec	_Uxactions[] = {
			{ "IPOpEditMenuPullDown", (XtActionProc) action_IPOpEditMenuPullDown }};

		XtAppAddActions( UxAppContext,
				_Uxactions,
				XtNumber(_Uxactions) );

		MrmRegisterNamesInHierarchy( hierarchy,
					_UxMrmNames,
					XtNumber(_UxMrmNames) );

		_Uxinit = 1;
	}

	rtrn = _Uxbuild_IPOperationsShell();

	/* init global vars */
	IPOperationsShellCreated = B_TRUE;
	CurrentIPOp.f = NULL;
	UxPutLabelString(StackLG, "Stack depth : 0  ");
	sprintf(Msg, "");
	
	/* init local vars */
	ParmLabels[0] = Parm1LG;
	ParmLabels[1] = Parm2LG;
	ParmLabels[2] = Parm3LG;
	ParmLabels[3] = Parm4LG;
	ParmTexts[0] = Parm1TW;
	ParmTexts[1] = Parm2TW;
	ParmTexts[2] = Parm3TW;
	ParmTexts[3] = Parm4TW;
	
	/* unmap all parameters fields */
	for (parmcnt = 0; parmcnt < IPOPNPARMS; parmcnt++) {
	    UxUnmap(ParmLabels[parmcnt]);
	    UxUnmap(ParmTexts[parmcnt]);
	}
	UxUnmap(RepsLG);
	UxUnmap(RepsTW);
	
	/* create Processed image widgets if not done already */
	if (ProcessedImageShellCreated == B_FALSE)
	    ProcessedImageShell = create_ProcessedImageShell();
	
	/* build popup menu */
	BuildEditingMenu();
	
	/* return */
	return(rtrn);
}

/*******************************************************************************
       END OF FILE
*******************************************************************************/

