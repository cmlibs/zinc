/*****************************************************************************
*
*    File:      ProcessedImageShellAux.c
*    Author:    Paul Charette
*    Modified:  30 July 1995
*
*    Purpose:   various utility routines
*                 CreateNode()
*                 create_ProcessedImageShell()
*                 FE_node_get_pixel_coord()
*                 FE_node_change_real_coords()
*                 PHSBCallback()
*                 ProcessedImageXYSlices()
*                 ProcExposeCB()
*                 ProcSetSizeTo640x480()
*                 PVSBCallback()
*                 RedrawProcDrawingArea()
*                 Reset_FE_display_state()
*                 Setup_FE_node_defintion()
*                 Terminate_FE_node_definition()
*                 UpdateProcessedWinParms()
*
*****************************************************************************/

/* whether to draw the number of the data point when digitising nodes */
int data_draw_number = 1;

struct Node_2d *CREATE(Node_2d)(char *identifier,Node_2d_type x,Node_2d_type y)
/*******************************************************************************
LAST MODIFIED : 2 November 1995

DESCRIPTION :
Creates a 2d node and makes it not-selected.
==============================================================================*/
{
  struct Node_2d *temp_Node_2d;
  
  ENTER(CREATE(Node_2d));
  temp_Node_2d = (struct Node_2d *)NULL;
  if (ALLOCATE(temp_Node_2d,struct Node_2d,1))
  {
    temp_Node_2d->x = x;
    temp_Node_2d->y = y;
    temp_Node_2d->selected = FALSE;
    if (identifier)
    {
      if (ALLOCATE(temp_Node_2d->identifier,char,strlen(identifier)+1))
      {
        strcpy(temp_Node_2d->identifier,identifier);
      }
      else
      {
        display_message(ERROR_MESSAGE,
          "CREATE(Node_2d).  Could not allocate memory for node identifier");
        DEALLOCATE(temp_Node_2d);
      }
    }
    else
    {
      temp_Node_2d->identifier = (char *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,
      "CREATE(Node_2d).  Could not allocate memory for node");
  }
  LEAVE;

  return temp_Node_2d;
} /* CREATE(Node_2d) */

int DESTROY(Node_2d)(struct Node_2d **node_2d_address)
/*******************************************************************************
LAST MODIFIED : 2 November 1995

DESCRIPTION :
Destroys the given node.
==============================================================================*/
{
  int return_code;
  struct Node_2d *temp_Node_2d;

  ENTER(CREATE(Node_2d));
	if ((node_2d_address)&&(temp_Node_2d= *node_2d_address))
	{
    if (temp_Node_2d->identifier)
    {
			DEALLOCATE(temp_Node_2d->identifier);
    }
    DEALLOCATE(*node_2d_address);
		return_code=1;
	}
	else
	{
		return_code=0;
	}
  LEAVE;

  return return_code;
} /* DESTROY(Node_2d) */

void get_window_position(Node_2d_type x,Node_2d_type y,int *window_x,int *window_y)
/* Returns the window positions for a given pixel position */
{
    *window_x = ((x * Image_pixels_per_m_X) + Image_pixel_org_X)*ZoomFactor;
    *window_y = ((y * Image_pixels_per_m_Y) + Image_pixel_org_Y)*ZoomFactor;
}

static int draw_Node_2d(struct Node_2d *node,void *dummy)
/*******************************************************************************
LAST MODIFIED : 2 November 1995

DESCRIPTION :
Draws a node on the processed window - either selected or not.
==============================================================================*/
{
  int return_code,window_x,window_y;
  GC colour;

  /* init return code */
  return_code = B_FALSE;

#if defined (CMGUI)
	if (node)
	{
    if(node->selected)
    {
      colour = gc_RedBlack;
    }
    else
    {
      colour = gc_YellowBlack;
    }
    get_window_position(node->x,node->y,&window_x,&window_y);
		XDrawArc(UxDisplay, XtWindow(UxGetWidget(ProcessedImageDrawingArea)),
			colour,window_x-1,window_y-1,2,2,0,360*64);
    if(node->identifier&&data_draw_number)
    {
      XDrawString(UxDisplay, XtWindow(UxGetWidget(ProcessedImageDrawingArea)),
        colour,window_x-5,window_y-5,node->identifier, strlen(node->identifier));
    }
		return_code=1;
	}
	else
	{
		return_code=0;
	}
#endif
	return (return_code);
} /* Draw_FE_node */

static void Draw_Nodes_2d(swidget DrawingArea)
/*******************************************************************************
LAST MODIFIED : 2 November 1995

DESCRIPTION :
Loops through the list of 2d Nodes and draws them.
==============================================================================*/
{
#ifdef CMGUI
  if(processed_node_list)
  {
    FOR_EACH_OBJECT_IN_LIST(Node_2d)(draw_Node_2d,(void *)NULL,
      processed_node_list);
  }
#endif
} /* Draw_FE_nodes */


#if defined (OLD_CODE)
/* create an FE node and add it to the liked list of the current group */
/* return values: 0 if unsuccessfull or cmiss_number otherwise         */
static int CreateNode(int x, int y)
{
  return(0);
}

/* delete an FE node */
static int DeleteNode(struct FE_node *ptr,void *dummy)
{
  return (1);
} /* DeleteNode */

static void DeleteAllNodes(void)
{
}

/* compute real coordinates for an FE node */
static void FE_node_change_real_coords(struct FE_node *node, int x, int y,
				       double z)
{
  char c[256];

  /* change fields within FE node structure */
  (node->values)[0] = (FE_value) (Image_m_per_pixel_X
				  * ((double) (x - Image_pixel_org_X)));
  (node->values)[1] = (FE_value) (Image_m_per_pixel_Y\
				  * ((double) (y - Image_pixel_org_Y)));
  (node->values)[2] = (FE_value) (Image_Z_plane);
}

/* compute the pixel coordinate (X or Y) of an FE node */
static int FE_node_get_pixel_coord(struct FE_node *node, int flag)
{
  int pix_coord;
  
  switch (flag) {
  case XPIXELCOORD:
    pix_coord = itrunc((node->values)[0] * Image_pixels_per_m_X)
      + Image_pixel_org_X;
    break;
  default:
  case YPIXELCOORD:
    pix_coord = itrunc((node->values)[1] * Image_pixels_per_m_Y)
      + Image_pixel_org_Y;
    break;
  }
  return(pix_coord);
}

static int Draw_FE_node(struct FE_node *node,void *dummy)
{
  char s[16];
  int return_code;

  /* init return code */
  return_code = B_FALSE;

#if defined (CMGUI)
  if (node) {
    XDrawArc(UxDisplay, XtWindow(ProcessedImageDrawingArea),
	     gc_YellowBlack,
	     FE_node_get_pixel_coord(node, XPIXELCOORD) - 1,\
	     FE_node_get_pixel_coord(node, YPIXELCOORD) - 1,\
	     2, 2, 0, 360*64);
    sprintf(s, "%d", node->cmiss_number);
    XDrawString(UxDisplay, XtWindow(ProcessedImageDrawingArea),
		gc_YellowBlack,
		FE_node_get_pixel_coord(node, XPIXELCOORD) - 5,\
		FE_node_get_pixel_coord(node, YPIXELCOORD) - 5,\
		s, strlen(s));
    return_code=1;
  }
  else {
    return_code=0;
  }
#endif
  return (return_code);
} /* Draw_FE_node */

/* draw nodes in the processed image drawing area */
static void Draw_FE_nodes(Widget DrawingArea)
{
}
#endif /* defined (OLD_CODE) */

struct Mouse_position
{
	int x,y;
}; /* struct Mouse_position */

static int pick_Node_2d(struct Node_2d *node,void *void_mouse_position)
{
  int window_x,window_y,return_code;
	struct Mouse_position *mouse_position;

  /* init return code */
  return_code = B_FALSE;

#ifdef CMGUI
	if (node&&(mouse_position=(struct Mouse_position *)void_mouse_position))
	{
		/* calculate node coordinates in the window */
    get_window_position(node->x,node->y,&window_x,&window_y);
		/* check if cursor within bounding box */
		if ((mouse_position->x >= (window_x - NODERADIUS)) &&
			(mouse_position->x <= (window_x + NODERADIUS)) &&
			(mouse_position->y >= (window_y - NODERADIUS)) &&
			(mouse_position->y <= (window_y + NODERADIUS)))
		{
			return_code=1;
			/* if this point is not already highlighted, then highlight it and print
				out the node world coordinates */
			if (node != Highlighted_node_2d)
			{
				XDrawRectangle(UxDisplay, 
					XtWindow(UxGetWidget(ProcessedImageDrawingArea)),gc_xor,
					window_x - NODERADIUS, window_y - NODERADIUS,2*NODERADIUS, 2*NODERADIUS);
			}
		}
		else
		{
			return_code=0;
		}
	}
	else
	{
		return_code=0;
	}

	return (return_code);
#endif
} /* pick_Node_2d */

#if defined (OLD_CODE)
static int pick_FE_node(struct FE_node *node,void *void_mouse_position)
{
  char s[512];
  int nodex,nodey,return_code;
	struct Mouse_position *mouse_position;

  /* init return code */
  return_code = B_FALSE;

#ifdef CMGUI
  if (node&&(mouse_position=(struct Mouse_position *)void_mouse_position)) {
    /* calculate node coordinates in the window */
    nodex = FE_node_get_pixel_coord(node, XPIXELCOORD);
    nodey = FE_node_get_pixel_coord(node, YPIXELCOORD);
    /* check if cursor within bounding box */
    if ((mouse_position->x >= (nodex - NODERADIUS)) &&
	(mouse_position->x <= (nodex + NODERADIUS)) &&
	(mouse_position->y >= (nodey - NODERADIUS)) &&
	(mouse_position->y <= (nodey + NODERADIUS))) {
      return_code=1;
      /* if this point is not already highlighted, then highlight it and print
	 out the node world coordinates */
      if (node != Highlighted_FE_node) {
	XDrawRectangle(UxDisplay, 
		       XtWindow(ProcessedImageDrawingArea),gc_xor,
		       nodex - NODERADIUS, nodey - NODERADIUS,2*NODERADIUS, 2*NODERADIUS);
	sprintf(s, "Fe node %d : WC(x=%f,y=%f,z=%f) SC(x=%d,y=%d)\n",
		node->cmiss_number,(node->values)[0],(node->values)[1],
		(node->values)[2],nodex,nodey);
	write_output(1, s);
      }
    }
    else {
      return_code=0;
    }
  }
  else {
    return_code=0;
  }
  
#endif
  return (return_code);
} /* pick_FE_node */
#endif /* defined (OLD_CODE) */

static void DrawImgPointFrame(int cx, int cy)
{
  int window_x,window_y;
  struct Node_2d *current_node;
	struct Mouse_position mouse_position;

#if defined (CMGUI)
  /* check for no current group */
  if (processed_node_list)
	{
    /* look for first drawable frame */
		mouse_position.x=cx;
		mouse_position.y=cy;
		current_node=FIRST_OBJECT_IN_LIST_THAT(Node_2d)(pick_Node_2d,
			(void *)&mouse_position,processed_node_list);
    /* set the new current highlighted point */
    if (current_node != Highlighted_node_2d)
		{
			/* unhighlight any previous point if required */
			if (Highlighted_node_2d != NULL)
			{
        get_window_position(Highlighted_node_2d->x,Highlighted_node_2d->y,
          &window_x,&window_y);
				XDrawRectangle(UxDisplay,
					 XtWindow(UxGetWidget(ProcessedImageDrawingArea)),
					 gc_xor,window_x-NODERADIUS,window_y-NODERADIUS,
					 2*NODERADIUS, 2*NODERADIUS);
			}
      Highlighted_node_2d= current_node;
    }
    /* sync the server */
    XmUpdateDisplay(UxGetWidget(ProcessedImageDrawingArea));
    XSync(UxDisplay, B_FALSE);
  }
#endif
}
  
#if defined (OLD_CODE)
static void DrawImgPointFrame(int cx, int cy)
{
  struct FE_node *highlighted_node;
  struct Mouse_position mouse_position;
  
#if defined (CMGUI)
  /* check for no current group */
  if (CurrentFENodeGroup != NULL) {
    /* look for first drawable frame */
    mouse_position.x=cx;
    mouse_position.y=cy;
    highlighted_node=first_FE_node_in_group_that(pick_FE_node,
						 (void *) &mouse_position,
						 CurrentFENodeGroup);
    /* set the new current highlighted point */
    if (highlighted_node != Highlighted_FE_node) {
      /* unhighlight any previous point if required */
      if (Highlighted_FE_node != NULL) {
	XDrawRectangle(UxDisplay,
		       XtWindow(ProcessedImageDrawingArea),
		       gc_xor,
		       FE_node_get_pixel_coord(Highlighted_FE_node,
					       XPIXELCOORD)-NODERADIUS,
		       FE_node_get_pixel_coord(Highlighted_FE_node,
					       YPIXELCOORD)-NODERADIUS,
		       2*NODERADIUS, 2*NODERADIUS);
      }
      Highlighted_FE_node = highlighted_node;
    }
    /* sync the server */
    XmUpdateDisplay(ProcessedImageDrawingArea);
    XSync(UxDisplay, B_FALSE);
  }
#endif
}
#endif /* defined (OLD_CODE) */
  
/* indirect callbacks */
void ProcExposeCB(Widget w, XtPointer p1, XtPointer p2)
{
  exposeCB_ProcImageDrawingArea(w, p1, p2);
}

void RedrawProcDrawingArea(void)
{
  XEvent e;
  XmDrawingAreaCallbackStruct s;

  s.event = &e;
  s.event->xexpose.x = 0;
  s.event->xexpose.y = 0;
  s.event->xexpose.width = ImgWidth*ZoomFactor;
  s.event->xexpose.height = ImgHeight*ZoomFactor;
  ProcExposeCB(ProcessedImageShell, NULL, &s);
}

void RedrawProcSubDrawingArea(int x, int y, int width, int height)
{
  XEvent e;
  XmDrawingAreaCallbackStruct s;

  s.event = &e;
  s.event->xexpose.x = x;
  s.event->xexpose.y = y;
  s.event->xexpose.width = width;
  s.event->xexpose.height = height;
  ProcExposeCB(ProcessedImageShell, NULL, &s);
}

/* scrollbar callbacks */
void PVSBCallback(Widget w, XtPointer p1, XtPointer p2)
{
    XtVaGetValues(ProcessedVsbar, XmNvalue, &ProcessedWinVOffset, NULL);
}
void PHSBCallback(Widget w, XtPointer p1, XtPointer p2)
{
    XtVaGetValues(ProcessedHsbar, XmNvalue, &ProcessedWinHOffset, NULL);
}


/* set window size to 640x480 */
void ProcSetSizeTo640x480(void)
{
  UxPutHeight(ProcessedImageShell, 480 + SCROLL_BAR_THICKNESS);
  UxPutWidth(ProcessedImageShell, 640 + SCROLL_BAR_THICKNESS);
}

/* Update window size parameters */
void UpdateProcessedWinParms(void)
{
  if (IsTrue(UxGetSet(ProcessedImageTBG))) {
    XtVaGetValues(ProcessedVsbar, XmNvalue, &ProcessedWinVOffset,
		  XmNsliderSize, &ProcessedWinHeight, NULL);
    XtVaGetValues(ProcessedHsbar, XmNvalue, &ProcessedWinHOffset,
		  XmNsliderSize, &ProcessedWinWidth, NULL);
    ProcVVisFrac = ((double) ProcessedWinHeight)
      / ((double) (ImgHeight*ZoomFactor));
    ProcHVisFrac = ((double) ProcessedWinWidth)
      / ((double) (ImgWidth*ZoomFactor));
  }
}

/* slice window routines */
void ProcessedImageXYSlices(boolean_t ToggleFlag, boolean_t NewFlag)
{
  /* delay the updates */
  UxDelayUpdate(ProcessedImageShell);

  /* reset cross hair cursor lines */
  PreviousXpos = -1;
  PreviousYpos = -1;

  if (ToggleFlag) {
    /* enable ProcessedImage slice windows */
    UxPutMaxHeight(ProcessedImageShell,
	(ImgHeight*ZoomFactor) + SCROLL_BAR_THICKNESS + SLICE_WINDOW_WIDTH);
    UxPutMaxWidth(ProcessedImageShell,
	(ImgWidth*ZoomFactor) + SCROLL_BAR_THICKNESS + SLICE_WINDOW_WIDTH);
    XSync(UxDisplay, B_FALSE);
    if (VerboseFlag)
      printf("ProcessedImageXYSlices() : Reset maximum size limits...\n");
    if (frame7 && XtIsRealized(frame7)
	&& frame8 && XtIsRealized(frame8)
	&& frame9 && XtIsRealized(frame9)) {
      UxMap(frame7);
      UxMap(frame8);
      UxMap(frame9);
    }
    XSync(UxDisplay, B_FALSE);
    if (VerboseFlag)
      printf("ProcessedImageXYSlices() : Mapped frames...\n");
    UxPutBottomOffset(ProcessedImageScrolledWindow, SLICE_WINDOW_WIDTH + 2);
    UxPutRightOffset(ProcessedImageScrolledWindow, SLICE_WINDOW_WIDTH + 2);
    if (NewFlag) {
	UxPutHeight(ProcessedImageShell,
		    ImgHeight*ZoomFactor + SCROLL_BAR_THICKNESS
		    + SLICE_WINDOW_WIDTH);
	UxPutWidth(ProcessedImageShell,
		   ImgWidth*ZoomFactor + SCROLL_BAR_THICKNESS
		   + SLICE_WINDOW_WIDTH);
    }
    else {
	UxPutHeight(ProcessedImageShell,
		    UxGetHeight(ProcessedImageShell) + SLICE_WINDOW_WIDTH);
	UxPutWidth(ProcessedImageShell,
		   UxGetWidth(ProcessedImageShell) + SLICE_WINDOW_WIDTH);
    }
    /* set cursors to crosshairs */
    if (ProcessedImageShell)
      if (XtIsRealized(ProcessedImageShell)) {
	XDefineCursor(UxDisplay, XtWindow(ProcessedImageShell),
		      xhair);
	ProcessedImageCursor = xhair;
      }
    /* flag this condition */
    SlicesProjectionsOn = B_TRUE;
  }
  else {
    /* disable ProcessedImage slice windows */
    UxPutBottomOffset(ProcessedImageScrolledWindow, 2);
    UxPutRightOffset(ProcessedImageScrolledWindow, 2);
    UxUnmap(frame7);
    UxUnmap(frame8);
    UxUnmap(frame9);
    XSync(UxDisplay, B_FALSE);
    if (VerboseFlag)
      printf("ProcessedImageXYSlices() : Unmapped frames...\n");
    if (NewFlag) {
	UxPutHeight(ProcessedImageShell,
		    ImgHeight*ZoomFactor + SCROLL_BAR_THICKNESS);
	UxPutWidth(ProcessedImageShell,
		   ImgWidth*ZoomFactor + SCROLL_BAR_THICKNESS);
    }
    else {
	UxPutHeight(ProcessedImageShell,
		    UxGetHeight(ProcessedImageShell) - SLICE_WINDOW_WIDTH);
	UxPutWidth(ProcessedImageShell,
		   UxGetWidth(ProcessedImageShell) - SLICE_WINDOW_WIDTH);
    }
    UxPutMaxHeight(ProcessedImageShell,
		   (ImgHeight*ZoomFactor) + SCROLL_BAR_THICKNESS);
    UxPutMaxWidth(ProcessedImageShell,
		  (ImgWidth*ZoomFactor) + SCROLL_BAR_THICKNESS);
    /* set cursors to pointers */
    if (ProcessedImageShell)
      if (XtIsRealized(ProcessedImageShell)) {
	XDefineCursor(UxDisplay, XtWindow(ProcessedImageShell),
		      leftpointer);
	ProcessedImageCursor = leftpointer;
      }
    /* flag this condition */
    SlicesProjectionsOn = B_FALSE;
   }
  /* updates */
  UxUpdate(ProcessedImageShell);
}






