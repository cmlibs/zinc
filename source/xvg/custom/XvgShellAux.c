/*****************************************************************************
*
*    File:      XvgShellAux.c
*    Author:    Paul Charette
*    Modified:  25 nov 1994
*
*    Purpose:   various initialisation & utility routines
*                   ClearStackMenu()
*                   CMOverlay()
*                   ContourOverlay()
*                   CreateProcImage()
*                   CreateRawImage()
*                   DrawBluePoint()
*                   DrawKnots()
*                   DrawLine()
*                   DrawRedPoint()
*                   DrawRegionLabels()
*                   DrawRegionNormals()
*                   DumpImage()
*                   ErrorMsg()
*                   GrabbingState()
*                   HGCursor()
*                   InfoMsg()
*                   InfoMsgPopdownOKOn()
*                   InfoMsgPopupOKOff()
*                   InfoMsgUpdate()
*                   InitProcBuffers()
*                   init_state()
*                   LoadArctanradiusImgMenu()
*                   LoadBRYGMBVWCells()
*                   LoadFile()
*                   LoadGammaGreyLevelCells()
*                   LoadGreyLevelCells()
*                   LoadIPOps()
*                   LoadMaskMenu()
*                   LoadOverlayFunction()
*                   LoadPlaneAngleMenu()
*                   LoadPStepImgMenu()
*                   LoadUserCells()
*                   NullFunction()
*                   parse_args()
*                   PeekNoInteractive()
*                   PopImageMenu()
*                   PushImageMenu()
*                   RemoveOverlayFunction()
*                   ResizeProcessedImageWidgets()
*                   ResizeRawImageWidgets()
*                   RotateImage()
*                   SaveIPOps()
*                   ScreenSaverOff()
*                   ScreenSaverOn()
*                   ShowImage()
*                   StrainOverlay()
*                   SwapMenu()
*                   TransposeImage()
*                   UpdateStackLabel()
*
*****************************************************************************/
static void XvgDeallocateResources(XtPointer client_data, XtIntervalId *_id )
{
  int i;

  /* deallocate color cells */
  for (i = 0; i < NOverlayCells; i++)
    CCTable[i] = OverlayCells[i].pixel;
  XFreeColors(UxDisplay, cmap, CCTable, NOverlayCells, 0);
  if (PagedCmap)
    XFreeColors(UxDisplay, cmap, LutCells, NPAGEDLUTCELLS, 0);
  
  /* free allocated memory */
  XvgMM_FreeAllBlocks();
  
  /* destroy context */
  if (client_data != NULL)
    XtFree((char *) client_data);

  /* show done */
  if (VerboseFlag)
    printf("XvgDeallocateResources() : xvg is no more...\n");
}
	  
void HGCursor(boolean_t state)
{
  if (InteractiveFlag == B_FALSE)
    return;

#ifdef DEBUG
  printf("HGCursor()\n");
#endif

  if (state) {
    if (XvgShell)
      if (XtIsRealized(XvgShell))
	XDefineCursor(UxDisplay, XtWindow(XvgShell),
		      hourglass);
/*
    if (fileSelectionBoxDialog)
      if (XtIsRealized(fileSelectionBoxDialog))
	XDefineCursor(UxDisplay,
		      XtWindow(fileSelectionBoxDialog),
		      hourglass);
#ifndef CMGUI
    if (errorDialog)
      if (XtIsRealized(errorDialog))
	XDefineCursor(UxDisplay, XtWindow(errorDialog),
		      hourglass);
#endif
    if (InfoDialog)
      if (XtIsRealized(InfoDialog))
	XDefineCursor(UxDisplay, XtWindow(InfoDialog),
		      hourglass);
    if (promptDialog)
      if (XtIsRealized(promptDialog))
	XDefineCursor(UxDisplay, XtWindow(promptDialog),
		      hourglass);
*/
    if (MovieControlsShellCreated)
      if (MovieControlsShell)
	if (XtIsRealized(MovieControlsShell))
	  XDefineCursor(UxDisplay, XtWindow(MovieControlsShell),
			hourglass);
    if (LUTShellCreated)
      if (LUTShell)
	if (XtIsRealized(LUTShell))
	  XDefineCursor(UxDisplay, XtWindow(LUTShell),
			hourglass);
#ifdef ALL_INTERFACES
    if (DAShellCreated)
      if (DAShell)
	if (XtIsRealized(DAShell))
	  XDefineCursor(UxDisplay, XtWindow(DAShell),
			hourglass);
    if (ADShellCreated)
      if (ADShell)
	if (XtIsRealized(ADShell))
	  XDefineCursor(UxDisplay, XtWindow(ADShell),
			hourglass);
#endif
    if (RawImageShellCreated)
      if (RawImageShell)
	if (XtIsRealized(RawImageShell))
	  XDefineCursor(UxDisplay, XtWindow(RawImageShell),
			hourglass);
    if (ProcessedImageShellCreated)
      if (ProcessedImageShell)
	if (XtIsRealized(ProcessedImageShell))
	  XDefineCursor(UxDisplay, XtWindow(ProcessedImageShell),
			hourglass);
    if (HistogramShellCreated)
      if (HistogramShell)
	if (XtIsRealized(HistogramShell))
	  XDefineCursor(UxDisplay, XtWindow(HistogramShell),
			hourglass);
#ifdef ALL_INTERFACES
    if (GalvoControlShellCreated)
      if (GalvoControlShell)
	if (XtIsRealized(GalvoControlShell))
	  XDefineCursor(UxDisplay,
			XtWindow(GalvoControlShell),
			hourglass);
#endif
    if (IPOperationsShellCreated)
      if (IPOperationsShell)
	if (XtIsRealized(IPOperationsShell))
	  XDefineCursor(UxDisplay, XtWindow(IPOperationsShell),
			hourglass);
#ifdef ALL_INTERFACES
    if (LCRShellCreated)
      if (LCRShell)
	if (XtIsRealized(LCRShell))
	  XDefineCursor(UxDisplay, XtWindow(LCRShell),
			hourglass);
#endif
    if (GLShellCreated)
      if (GLShell)
	if (XtIsRealized(GLShell))
	  XDefineCursor(UxDisplay, XtWindow(GLShell),
			hourglass);
    XmUpdateDisplay(UxTopLevel);
  }
  else {
    if (XvgShell)
      if (XtIsRealized(XvgShell))
	XUndefineCursor(UxDisplay, XtWindow(XvgShell));
/*
    if (fileSelectionBoxDialog)
      if (XtIsRealized(fileSelectionBoxDialog))
	XUndefineCursor(UxDisplay,
			XtWindow(fileSelectionBoxDialog));
#ifndef CMGUI
    if (errorDialog)
      if (XtIsRealized(errorDialog))
	XUndefineCursor(UxDisplay, XtWindow(errorDialog));
#endif
    if (InfoDialog)
      if (XtIsRealized(InfoDialog))
	XUndefineCursor(UxDisplay, XtWindow(InfoDialog));
    if (promptDialog)
      if (XtIsRealized(promptDialog))
	XUndefineCursor(UxDisplay, XtWindow(promptDialog));
*/
    if (MovieControlsShellCreated)
      if (MovieControlsShell)
	if (XtIsRealized(MovieControlsShell))
	  XUndefineCursor(UxDisplay,XtWindow(MovieControlsShell));
    if (LUTShellCreated)
      if (LUTShell)
	if (XtIsRealized(LUTShell))
	  XUndefineCursor(UxDisplay, XtWindow(LUTShell));
#ifdef ALL_INTERFACES
    if (DAShellCreated)
      if (DAShell)
	if (XtIsRealized(DAShell))
	  XUndefineCursor(UxDisplay, XtWindow(DAShell));
    if (ADShellCreated)
      if (ADShell)
	if (XtIsRealized(ADShell))
	  XUndefineCursor(UxDisplay, XtWindow(ADShell));
#endif
    if (RawImageShellCreated)
      if (RawImageShell)
	if (XtIsRealized(RawImageShell))
	  XDefineCursor(UxDisplay, XtWindow(RawImageShell),
		      RawImageCursor);
    if (ProcessedImageShellCreated)
      if (ProcessedImageShell)
	if (XtIsRealized(ProcessedImageShell))
	  XDefineCursor(UxDisplay, XtWindow(ProcessedImageShell),
			ProcessedImageCursor);
    if (HistogramShellCreated)
      if (HistogramShell)
	if (XtIsRealized(HistogramShell))
	  XDefineCursor(UxDisplay, XtWindow(HistogramShell),
			HistogramCursor);
#ifdef ALL_INTERFACES
    if (GalvoControlShellCreated)
      if (GalvoControlShell)
	if (XtIsRealized(GalvoControlShell))
	  XUndefineCursor(UxDisplay,
			  XtWindow(GalvoControlShell));
#endif
    if (IPOperationsShellCreated)
      if (IPOperationsShell)
	if (XtIsRealized(IPOperationsShell))
	  XUndefineCursor(UxDisplay, XtWindow(IPOperationsShell));
#ifdef ALL_INTERFACES
    if (LCRShellCreated)
      if (LCRShell)
	if (XtIsRealized(LCRShell))
	  XUndefineCursor(UxDisplay, XtWindow(LCRShell));
#endif
    if (GLShellCreated)
      if (GLShell)
	if (XtIsRealized(GLShell))
	  XUndefineCursor(UxDisplay, XtWindow(GLShell));
    XmUpdateDisplay(UxTopLevel);
  }
}

/* update stack label gadget */
void UpdateStackLabel(int x)
{
  char s[100];

  if (InteractiveFlag && (ChildProcess == B_FALSE)) {  
    sprintf(s, "Stack depth : %d", x);
    UxPutLabelString(StackLG, s);
    XSync(UxDisplay, B_FALSE);
  }
}

/*  clear image stack */
void ClearStackMenu(void)
{
  XvgImgStack_Clear();
}

/* pop image off the stack */
void SwapMenu(void)
{
  HGCursor(B_TRUE);
  if (XvgImgStack_Swap(ProcessedPixels)) {
    DynamicRange(ProcessedPixels, ImgHeight*ImgWidth, &ProcMax, &ProcMin);
    CreateProcImage();
  }
  else {
    ErrorMsg("SwapMenu() : Error swapping images (mismatched size or stack empty");
  }
  HGCursor(B_FALSE);
}

/* pop image off the stack */
void PopImageMenu(void)
{
  HGCursor(B_TRUE);
  if (XvgImgStack_Pop() == B_FALSE)
    ErrorMsg("PopImageMenu() : Error popping image off the stack");
  HGCursor(B_FALSE);
}

/* push image onto the stack */
void PushImageMenu(void)
{
  HGCursor(B_TRUE);
  if (XvgImgStack_Push(ProcessedPixels, ImgHeight, ImgWidth) == B_FALSE) {
    ErrorMsg("PushImageMenu() : Failed pushing image");
  }
  HGCursor(B_FALSE);
}


/* load radius image, push current image onto the stack */
void LoadArctanradiusImgMenu(void)
{
  int i;

  /* check that image is complex */
  if (RadiusImgData == NULL) {
    ErrorMsg("No radius data");
    return;
  }

  /* push current image onto the stack */
  HGCursor(B_TRUE);
  if (XvgImgStack_Push(ProcessedPixels, ImgHeight, ImgWidth) == B_FALSE) {
    ErrorMsg("LoadArctanradiusImgMenu(): Failed pushing image onto the stack");
    HGCursor(B_FALSE);
    return;
  }

  /* load the arctan radius data into the current buffer */
  for (i = 0; i < ImgHeight*ImgWidth; i++)
    ProcessedPixels[i] = RadiusImgData[i];
  DynamicRange(ProcessedPixels, ImgHeight*ImgWidth, &ProcMax, &ProcMin);
  CreateProcImage();
  HGCursor(B_FALSE);
}

/* load phase stepping raw image, push current image onto the stack */
void LoadPStepImgMenu(int indx)
{
  unsigned short *p;
  int i, RedMask;
  
  /* push current image onto the stack */
  HGCursor(B_TRUE);
  if (XvgImgStack_Push(ProcessedPixels, ImgHeight, ImgWidth) == B_FALSE) {
    ErrorMsg("LoadPStepImgMenu(): Failed pushing image onto the stack");
    HGCursor(B_FALSE);
    return;
  }

#ifdef VCAA_CODE
  /* mask out all but red gun bits */
  if (VCAAFormat == VCA_FORMAT_664)
    RedMask = 0xfc;
  else
#endif
    RedMask = 0xf8;
  
  /* load the PStep data into the current buffer */
  for (i = 0, p = &(RawPsBufsX[ImgHeight * ImgWidth * indx]);
       i < ImgHeight*ImgWidth; i++)
    ProcessedPixels[i] = p[i] & RedMask;
  DynamicRange(ProcessedPixels, ImgHeight*ImgWidth, &ProcMax, &ProcMin);
  CreateProcImage();
  HGCursor(B_FALSE);
}


/* load mask image, push current image onto the stack */
void LoadMaskMenu(void)
{
  int i;

  /* push current image onto the stack */
  HGCursor(B_TRUE);
  if (XvgImgStack_Push(ProcessedPixels, ImgHeight, ImgWidth) == B_FALSE) {
    ErrorMsg("LoadMaskMenu(): Failed pushing image onto the stack");
    HGCursor(B_FALSE);
    return;
  }

  /* load the mask data into the current buffer */
  for (i = 0; i < ImgHeight*ImgWidth; i++)
    ProcessedPixels[i] = MaskPixels[i];
  DynamicRange(ProcessedPixels, ImgHeight*ImgWidth, &ProcMax, &ProcMin);
  CreateProcImage();
  HGCursor(B_FALSE);
}


/* display center of mass */
void StrainOverlay(Widget DrawingArea)
{
  FILE *fp;
  XPoint points[5];
  int i;
  char s[256];

#ifdef _FE_WRITE_OVERLAY
  /* write out element boundaries to a file */
  if ((fp = fopen("FeStrain.dat", "w")) == NULL) {
    Abort = B_TRUE;
    sprintf(s, "Could not open the output file \"%s\" in StrainOverlay()",
	    "FeStrain.dat");
    draw_mesg(s);
    return;
  }
#endif
  
  /* principal strains */
  for (i = 0; i < NStrainElements; i++) {
    /* maximum strain */
    XDrawLine(UxDisplay, XtWindow(DrawingArea),
	      gc_YellowBlack,
	      (int) (StrainElements[i].x - StrainElements[i].dxmax),
	      (int) (StrainElements[i].y - StrainElements[i].dymax),
	      (int) (StrainElements[i].x + StrainElements[i].dxmax),
	      (int) (StrainElements[i].y + StrainElements[i].dymax));

    /* minimum strain */
    XDrawLine(UxDisplay, XtWindow(DrawingArea),
	      gc_YellowBlack,
	      (int) (StrainElements[i].x - StrainElements[i].dxmin),
	      (int) (StrainElements[i].y - StrainElements[i].dymin),
	      (int) (StrainElements[i].x + StrainElements[i].dxmin),
	      (int) (StrainElements[i].y + StrainElements[i].dymin));
#ifdef _FE_WRITE_OVERLAY
    fprintf(fp, "%d %d\n%d %d\n%d %d\n%d %d\n",
	    (int) (StrainElements[i].x - StrainElements[i].dxmax),
	    (int) (StrainElements[i].x + StrainElements[i].dxmax),
	    (int) (StrainElements[i].y - StrainElements[i].dymax),
	    (int) (StrainElements[i].y + StrainElements[i].dymax),
	    (int) (StrainElements[i].x - StrainElements[i].dxmin),
	    (int) (StrainElements[i].x + StrainElements[i].dxmin),
	    (int) (StrainElements[i].y - StrainElements[i].dymin),
	    (int) (StrainElements[i].y + StrainElements[i].dymin));
#endif
  }
#ifdef _FE_WRITE_OVERLAY
  fclose(fp);
#endif

  XSync(UxDisplay, B_FALSE);
}

/* display displacement field lines */
void DispFieldOverlay(Widget DrawingArea)
{
  int i;
  
  for (i = 0; i < NDispFieldElements; i++)
    XDrawLine(UxDisplay, XtWindow(DrawingArea),
	      gc_YellowBlack,
	      DispFieldElements[i].xmin, DispFieldElements[i].ymin,
	      DispFieldElements[i].xmax, DispFieldElements[i].ymax);
  
  XSync(UxDisplay, B_FALSE);
}

/* load overlay function into array */
void LoadOverlayFunction(void (f)(Widget))
{
  int i;
  
  /* check if function already loaded */
  for (i = 0; (i < NOverlayFunctions) && (OverlayFunctions[i] != f); i++);

  /* load function if required */
  if ((NOverlayFunctions < OVERLAYFUNCTIONSMAX) && (i == NOverlayFunctions))
    OverlayFunctions[NOverlayFunctions++] = f;

#ifdef DEBUG
  printf("LoadOverlayFunction() : added overlay %d\n", (int) f);
#endif
}

/* load overlay function into array */
static void NullFunction(Widget dummy) {}
void RemoveOverlayFunction(void (f)(Widget))
{
  int i;
  
  /* check if function already loaded */
  for (i = 0; (i < NOverlayFunctions) && (OverlayFunctions[i] != f); i++);

  /* load function if required */
  if (i != NOverlayFunctions)
    OverlayFunctions[i] = NullFunction;

#ifdef DEBUG
  printf("RemoveOverlayFunction() : removeded overlay %d\n", (int) f);
#endif
}

/* display center of mass */
void CMOverlay(Widget DrawingArea)
{
  XDrawArc(UxDisplay, XtWindow(DrawingArea),
	   gc_BlueBlack, CMX-5, CMY-5, 10, 10, 0, 360*64 -1);
  XSync(UxDisplay, B_FALSE);
}

/* display contour overlay */
void ContourOverlay(Widget DrawingArea)
{
  int i;

  if (ZoomFactor == 1)
    XDrawLines(UxDisplay, XtWindow(DrawingArea), gc_BlueBlack,
	       ContourPoints, NContourPoints, CoordModeOrigin);
  else {
    for (i = 0; i < (NContourPoints-1); i++)
      XDrawLine(UxDisplay, XtWindow(DrawingArea), gc_BlueBlack,
		ContourPoints[i].x * ZoomFactor,
		ContourPoints[i].y * ZoomFactor,
		ContourPoints[i+1].x * ZoomFactor,
		ContourPoints[i+1].y * ZoomFactor);
  }
  
  XSync(UxDisplay, B_FALSE);
#ifdef DEBUG
  printf("DrawContourOverlay() : Done\n");
#endif
}


/* display BSpline knots overlay */
void DrawKnots(Widget DrawingArea)
{
  XDrawPoints(UxDisplay, XtWindow(DrawingArea),
	      gc_RedBlack, KnotPoints, NKnotPoints, CoordModeOrigin);
  XSync(UxDisplay, B_FALSE);
}


/* draw region labels */
void DrawRegionLabels(Widget DrawingArea)
{ 
  int i, j, k;
  char s[128];

  /* region contours */
  if (GcLineWidth <= 1) {
    for (i = 0; (i < NRegions) && (OverlayLevel > 0); i++)
      for (j = 0; j < RegionList[i].NContourElements; j++)
	XDrawPoint(UxDisplay, XtWindow(DrawingArea),
		   gc_WhiteBlack, RegionList[i].ContourElements[j].x,
		   RegionList[i].ContourElements[j].y);
  }
  else {
    for (i = 0; (i < NRegions) && (OverlayLevel > 0); i++)
      for (j = 0; j < RegionList[i].NContourElements; j++)
    	XDrawArc(UxDisplay, XtWindow(DrawingArea),
		 gc_WhiteBlack,
		 RegionList[i].ContourElements[j].x - GcLineWidth/2,
		 RegionList[i].ContourElements[j].y - GcLineWidth/2,
		 GcLineWidth, GcLineWidth, 0, 360*64 -1);
  }
  
  /* region labels */
/*
  for (i = 0; (i < NRegions) && (OverlayLevel > 1); i++) {
    sprintf(s, "%.0f", RegionList[i].index);
    XDrawString(UxDisplay, XtWindow(DrawingArea),
		gc_RedBlack, RegionList[i].CentroidX,
		RegionList[i].CentroidY, s, strlen(s));
  }
*/
#ifdef DEBUG
  printf("DrawRegionLabels() : Done\n");
#endif
}
  
void DrawRegionNormals(Widget DrawingArea)
{
  char *marked;
  int i, j, k;

#ifdef BROKEN_STUFF
  /* check to see is this overlay is disabled */
  if (NoNormalsOverlay)
    return;

  /* build matrix system to track fringes that have been marked */
  /* so as not to draw normals from both sides of the boundary  */
  if ((marked = (char *) XvgMM_Alloc(NULL, NRegions * NRegions)) == NULL) {
    ErrorMsg("Could not allocate in DrawRegionNormals()");
    return;
  }
  for (i = 0; i < NRegions*NRegions; i++)
    marked[i] = 0;

  /* contour normals */
  for (i = 0; (i < NRegions) && (OverlayLevel > 1); i++)
    for (j = 0; j < RegionList[i].NNeighbours; j++)
      {
	/* only draw normals along this boundry if they have not been */
	/* drawn as part of another region boundary, else it clutters */
	/* up the display */
	if ((marked[i*NRegions + RegionList[i].NeighbourIndicies[j]] == 0)
	    && (marked[RegionList[i].NeighbourIndicies[j]*NRegions + i] == 0)){
	  /* mark this combination */
	  marked[RegionList[i].NeighbourIndicies[j]*NRegions + i] = 1;
	  marked[i*NRegions + RegionList[i].NeighbourIndicies[j]] = 1;
	  /* loop to draw normals */
	  for (k = 0; k < RegionList[i].NNormalElements[j]; k++)
	    XDrawLine(UxDisplay, XtWindow(DrawingArea),
		      gc_WhiteBlack, RegionList[i].NormalElements[j][k].LineX0,
		      RegionList[i].NormalElements[j][k].LineY0,
		      RegionList[i].NormalElements[j][k].LineX1,
		      RegionList[i].NormalElements[j][k].LineY1);
	}
      }
  
  /* contour normal inner endpoints */
/*
  for (i = 0; (i < NRegions) && (OverlayLevel > 1); i++)
    for (j = 0; j < RegionList[i].NNeighbours; j++)
      for (k = 0; k < RegionList[i].NNormalElements[j]; k++)
	XDrawArc(UxDisplay, XtWindow(DrawingArea),
		 gc_BlueBlack, RegionList[i].NormalElements[j][k].LineX0-2,
		 RegionList[i].NormalElements[j][k].LineY0-2, 5, 5,
		 0, 360*64 -1);
*/
  XvgMM_Free(marked);
  XSync(UxDisplay, B_FALSE);
#ifdef DEBUG
  printf("DrawRegionNormals() : Done\n");
#endif
#endif
}

/* parse input arguments */
static void usage(void)
{
#ifndef CMGUI
  printf("\aUsage: xvg [flags] [IP Ops]\n");
  printf("  flags:\n");
  printf("    -cmapalloc                    Reserve cmap cells (default)\n");
  printf("    -colorbar                     Enable colorbar overlay\n");
  printf("    -colorlut                     Load color lut\n");
  printf("    -markercircleradius           Marker circle overlay radius\n");
  printf("    -notiffrgbtogray              Do not convert RGB tiff to graylevel\n");
  printf("    -halfwidth                    For .sgi images, use half width\n");
  printf("    -displayonly                  No interactive, display image\n");
  printf("    -gammalut        gcfactor     Load gammma lut\n");
  printf("    -inputimage      infilename   Input image (MATLAB or TIFF file)\n");
  printf("    -inputIPOps      infilename   Input IP Ops file\n");
  printf("    -linewidth       width        Overlay line drawing width\n");
  printf("    -memorycheck                  Enable memory check scan\n");
  printf("    -monochrome                   Monochrome overlays\n");
  printf("    -nocmapalloc                  Do not reserve cmap cells\n");
  printf("    -nointeractive                Batch processing\n");
  printf("    -nonormals                    Don't draw region normals\n");
  printf("    -norecompress                 Don't recompress .Z files\n");
  printf("    -overlay         level        Overlay level [0..2]\n");
  printf("    -radius          value        Atan lut radius threshold\n");
  printf("    -usage                        List flags & command line opts\n");
  printf("    -VCRtty          tty          VCR serial port device\n");
  printf("    -verbose                      Diagnostics\n");
#endif
}

/* warning!! Flags are first parsed by Xt, so make sure names are distinct */
boolean_t parse_args(int nargs, char *args[])
{
  FILE *fp;
  IPOpListItem *ThisIPOp, *p;
  DOUBLE_ARRAY *da;
  int ac, i, j, k, nn;
  char *InOp, b[256];

  /* parse the arguments */
  for (ac = 0; ac < nargs;) {
    sprintf(b, "parse_args(): Input argument: %s", args[ac]);
    DEBUGS(b);

    /* input flags */
    if (args[ac][0]=='-') {
      if (strcmp(args[ac], "-inputimage") == 0) {        /* input image file */
	ac++;
	if (ac == nargs) {
	  ErrorMsg("Missing input image file name!");
	  return(B_FALSE);
	}
	InFileName = (char *) XvgMM_Alloc((void *) InFileName,
					  strlen(args[ac]) + 1);
	sprintf(InFileName, "%s", args[ac++]);
      }
      else if (strcmp(args[ac], "-inputIPOps") == 0) {  /* input IP Ops file */
	ac++;
	if (ac == nargs) {
	  ErrorMsg("Missing input IPOps file name!");
	  return(B_FALSE);
	}
	LoadIPOps(args[ac++]);
      }
      else if (strcmp(args[ac], "-cmapalloc") == 0) {
	PagedCmap = B_TRUE;
	ac++;
      }
      else if (strcmp(args[ac], "-display") == 0) {
	/* ignore it */
	ac+=2;
      }
      else if (strcmp(args[ac], "-memorycheck") == 0) {
	XvgMM_MemoryCheck_Flag = B_TRUE;
	ac++;
      }
      else if (strcmp(args[ac], "-colorbar") == 0) {
	ColorBarOverlay = B_TRUE;
	ac++;
      }
      else if (strcmp(args[ac], "-colorlut") == 0) {
	CurrentCells = BRYGMBVMCells;
	ac++;
      }
      else if (strcmp(args[ac], "-markercircleradius") == 0) {
	ac++;
	MarkerCircleRadius = atoi(args[ac]);
	ac++;
      }
      else if (strcmp(args[ac], "-notiffrgbtogray") == 0) {
	Convert_RGB_TIFF_To_Graylevel = B_FALSE;
	ac++;
      }
      else if (strcmp(args[ac], "-displayonly") == 0) {
	InteractiveFlag = B_FALSE;
	DisplayOnlyFlag = B_TRUE;
	ac++;
      }
      else if (strcmp(args[ac], "-gammalut") == 0) {
	ac++;
	if (ac == nargs) {
	  ErrorMsg("Missing gamma correction value on command line");
	  return(B_FALSE);
	}
	GcFactor = atof(args[ac]);
	ac++;
	CurrentCells = GammaGreyLevelCells;
      }
      else if (strcmp(args[ac], "-halfwidth") == 0) {
	SGIImageHalfWidth = B_TRUE;
	ac++;
      }
      else if (strcmp(args[ac], "-linewidth") == 0) {
	ac++;
	GcLineWidth = atoi(args[ac]);
	ac++;
      }
      else if (strcmp(args[ac], "-monochrome") == 0) {
	Monochrome = B_TRUE;
	ac++;
      }
      else if (strcmp(args[ac], "-nocmapalloc") == 0) {
	PagedCmap = B_FALSE;
	ac++;
      }
      else if (strcmp(args[ac], "-nonormals") == 0) {
	NoNormalsOverlay = B_TRUE;
	ac++;
      }
      else if (strcmp(args[ac], "-norecompress") == 0) {
	Recompress = B_FALSE;
	ac++;
      }
      else if (strcmp(args[ac], "-overlay") == 0) {
	ac++;
	if (ac == nargs) {
	  ErrorMsg("Missing overlay level on command line");
	  return(B_FALSE);
	}
	OverlayLevel = atoi(args[ac]);
	ac++;
      }
      else if (strcmp(args[ac], "-radius") == 0) {
	ac++;
	if (ac == nargs) {
	  ErrorMsg("Missing radius threshold value on command line");
	  return(B_FALSE);
	}
	AtanRadiusThreshold = atof(args[ac]);
	ac++;
      }
      else if (strcmp(args[ac], "-VCRtty") == 0) {
	ac++;
	if (ac == nargs) {
	  ErrorMsg("Missing tty device name on command line");
	  return(B_FALSE);
	}
	VCRtty = (char *) UxMalloc(strlen(args[ac]) + 1);
	sprintf(VCRtty, "%s", args[ac++]);
      }
      else if (strcmp(args[ac], "-usage") == 0) {
	usage();
	/* list IP Ops & parms */
	printf("\nIP Ops : (\"parameter name\"[data type], ..\n");
	for (i = 0; i < NIPRegisteredOps; i++) {
	  printf("    %s (", IPRegisteredOpList[i].OpName);
	  for (j = 0; j < IPRegisteredOpList[i].NParms; j++) {
	    if (j != 0)
	      printf(",");
	    switch (IPRegisteredOpList[i].ParmDataTypes[j]) {
	    case INTEGER_DT:
	      printf(" \"%s\"[integer]", IPRegisteredOpList[i].ParmNames[j]);
	      break;
	    case DOUBLE_DT:
	      printf(" \"%s\"[double]", IPRegisteredOpList[i].ParmNames[j]);
	      break;
	    case STRING_DT:
	      printf(" \"%s\"[string]", IPRegisteredOpList[i].ParmNames[j]);
	      break;
	    default:
	      break;
	    }
	  }
	  if (IPRegisteredOpList[i].RepsAllowed)
	    printf(", reps )\n");
	  else
	    printf(" )\n");
	}
	exit(EXIT_SUCCESS);
      }
      else if (strcmp(args[ac], "-verbose") == 0) {
	VerboseFlag = B_TRUE;
	ac++;
      }
      else {
	sprintf(b, "Unknown input flag : \"%s\"", args[ac]);
	ErrorMsg(b);
	return(B_FALSE);
      }
    }
    /* IP Ops */
    else {                                                          /* IP Op */
      /* create the ProcessedImage shell if its not done already */
      if ((ProcessedImageShellCreated == B_FALSE) && (InteractiveFlag)) {
	ProcessedImageShell = create_ProcessedImageShell();
	UxPopupInterface(ProcessedImageShell, no_grab);
      }
      
      /* create the IPOperations shell if its not done already */
      if ((IPOperationsShellCreated == B_FALSE) && (InteractiveFlag)) {
	IPOperationsShell = create_IPOperationsShell();
	UxPopupInterface(IPOperationsShell, no_grab);
      }
      
      /* find the IPOp in the linked list */
      for (i = 0, InOp = args[ac++];
	   (i < NIPRegisteredOps)
	   && (strcmp(IPRegisteredOpList[i].OpName, InOp) != 0); i++);

      /* if it doesn't exist, complain and exit */
      if ((i == NIPRegisteredOps) && (strcmp(InOp, "xvg") != 0)) {
	sprintf(b, "IP Op \"%s\" does not exist", InOp);
	ErrorMsg(b);
	printf("%s\n", b);
	return(B_FALSE);
      }
      /* else load in into the lists (ignore xvg command) */
      else if (strcmp(InOp, "xvg") != 0) {
	/* check for batch or interactive mode */
	if (InteractiveFlag)
	  ThisIPOp = &CurrentIPOp;
	else {
	  ThisIPOp = (IPOpListItem *) XvgMM_Alloc(NULL, sizeof(IPOpListItem));
	  (ThisIPOp->ParmPtrs)[0] = NULL;
	  (ThisIPOp->ParmPtrs)[1] = NULL;
	  (ThisIPOp->ParmPtrs)[2] = NULL;
	  (ThisIPOp->ParmPtrs)[3] = NULL;
	}

	/* load the parameters into the current IP structure parms */
	ThisIPOp->f = IPRegisteredOpList[i].f;
	ThisIPOp->RepsAllowed = IPRegisteredOpList[i].RepsAllowed;
	ThisIPOp->EdgeFixFlag = IPRegisteredOpList[i].EdgeFixFlag;
	(ThisIPOp->ParmDataTypes)[0]= IPRegisteredOpList[i].ParmDataTypes[0];
	(ThisIPOp->ParmDataTypes)[1]= IPRegisteredOpList[i].ParmDataTypes[1];
	(ThisIPOp->ParmDataTypes)[2]= IPRegisteredOpList[i].ParmDataTypes[2];
	(ThisIPOp->ParmDataTypes)[3]= IPRegisteredOpList[i].ParmDataTypes[3];
	
	/* load the parameters into the editing window Widgets and map */
	if (InteractiveFlag)
	  UxPutLabelString(FNameLG, InOp);
	for (j = 0; j < IPRegisteredOpList[i].NParms; j++) {
	  /* check to make sure there is another command line arg */
	  if (ac == nargs) {
	    sprintf(b, "Incorrect number of arguments for IP Op \"%s\"", InOp);
	    ErrorMsg(b);
	    return(B_FALSE);
	  }
	  if (InteractiveFlag) {
	    UxPutLabelString(ParmLabels[j],IPRegisteredOpList[i].ParmNames[j]);
	    if (IPRegisteredOpList[i].ParmDataTypes[j] != DOUBLE_ARRAY_DT) {
	      UxPutText(ParmTexts[j], args[ac++]);
	    }
	    else {
	      nn = atoi(args[ac++]);
	      for (k = 0, cbuf[0] = '\0'; k < nn; k++)
		sprintf(cbuf, "%s %s", cbuf, args[ac++]);
	      UxPutText(ParmTexts[j], cbuf);
	    }
	  }
	  switch (IPRegisteredOpList[i].ParmDataTypes[j]) {
	  case INTEGER_DT:
	    (ThisIPOp->ParmPtrs)[j] =
	      (void *) XvgMM_Alloc((void *) ((ThisIPOp->ParmPtrs)[j]),
				   sizeof(int));
	    if (!InteractiveFlag)
	      *((int *) ((ThisIPOp->ParmPtrs)[j])) = atoi(args[ac++]);
	    break;
	  case DOUBLE_DT:
	    (ThisIPOp->ParmPtrs)[j] =
	      (void *) XvgMM_Alloc((void *) ((ThisIPOp->ParmPtrs)[j]),
				   sizeof(double));
	    if (!InteractiveFlag)
	      *((double *) ((ThisIPOp->ParmPtrs)[j])) = atof(args[ac++]);
	    break;
	  case STRING_DT:
	    (ThisIPOp->ParmPtrs)[j] =
	      (void *) XvgMM_Alloc((void *) ((ThisIPOp->ParmPtrs)[j]),
				   INSTRINGSIZEMAX);
	    if (!InteractiveFlag)
	      sprintf((ThisIPOp->ParmPtrs)[j], "%s", args[ac++]);
	    break;
	  case DOUBLE_ARRAY_DT:
	    (ThisIPOp->ParmPtrs)[j] =
	      (void *) XvgMM_Alloc((void *) ((ThisIPOp->ParmPtrs)[j]),
				   sizeof(DOUBLE_ARRAY));
	    if (!InteractiveFlag) {
	      da = (DOUBLE_ARRAY *) ((ThisIPOp->ParmPtrs)[j]);
	      da->n = atoi(args[ac++]);

	      /* check that there are the required number of arguments left */
	      if ((nargs - ac) < da->n) {
		printf("parse_args(): %d args required for %s, %d supplied.\n",
		       da->n, InOp, nargs - ac);
		exit(-1);
	      }
	      
	      /* allocate storage for the array */
	      if ((da->data = (double *) XvgMM_Alloc((void *) (da->data),
						     sizeof(double) * da->n))
		  == NULL) {
		sprintf(cbuf, "parse_args() : Could not alloc storage (%d) for DOUBLE_ARRAY array",
			da->n * sizeof(double));
		ErrorMsg(cbuf);
		return(B_FALSE);
	      }
	      /* load in the elements */
	      for (k = 0; k < da->n; k++)
		(da->data)[k] = atof(args[ac++]);
	    }
	    break;
	  default:
	    (ThisIPOp->ParmPtrs)[j] = NULL;
	    break;
	  }
	}
	/* null the remaining arg pointers */
	for (; j < IPOPNPARMS; j++)
	  (ThisIPOp->ParmPtrs)[j] = NULL;
	
	/* if reps allowed, read in the count */
	if (ThisIPOp->RepsAllowed) {
	  /* check to make sure there is a rep count */
	  if (ac == nargs) {
	    sprintf(b, "Missing repetition count for IP Op \"%s\"", InOp);
	    ErrorMsg(b);
	    return(B_FALSE);
	  }
	  if (InteractiveFlag)
	    UxPutText(RepsTW, args[ac++]);
	  else
	    ThisIPOp->reps = atoi(args[ac++]);
	}
	else {
	  if (InteractiveFlag)
	    UxPutText(RepsTW, "1");
	  else
	    ThisIPOp->reps = 1;
	}
	
	/* load this op into the IPOp liked list */
	if (InteractiveFlag)
	  AddNewOpAfterCallback();
	else {
	  if (IPOpList == NULL) { /* empty list */
	    IPOpList = ThisIPOp;
	    ThisIPOp->prev = NULL;
	  }
	  else {                  /* insert at the end of the list */
	    for (p = IPOpList; p->next != NULL; p = (IPOpListItem *) p->next);
	    ThisIPOp->prev = (void *) p;
	    p->next = (void *) ThisIPOp;
	  }
	  ThisIPOp->next = NULL;
	}
      }
    }
  }
  /* return success */
  return(B_TRUE);
}


/*  Hourglass_pointer routines */
static void CreateCursors(void)
{
  XColor  color_defs[2];
  Pixmap  hourglass_pixmap;
  Pixmap  hourglass_mask_pixmap;

  /* create hourglass cursor */
  color_defs[0].pixel = black;
  color_defs[1].pixel = white;
  XQueryColors(UxDisplay, XDefaultColormap (UxDisplay, UxScreen),
	       color_defs, 2);
  
  hourglass_pixmap =
    XCreatePixmapFromBitmapData(UxDisplay, XDefaultRootWindow(UxDisplay),
				hourglass_bits, hourglass_width,
				hourglass_height,
				white,
				black, 1);
  hourglass_mask_pixmap =
    XCreatePixmapFromBitmapData(UxDisplay, XDefaultRootWindow (UxDisplay),
				hourglass_mask_bits, hourglass_mask_width,
				hourglass_mask_height,
				white,
				black, 1);
  hourglass =
    XCreatePixmapCursor(UxDisplay, hourglass_pixmap, hourglass_mask_pixmap,
			&color_defs[0], &color_defs[1], hourglass_x_hot,
			hourglass_y_hot);

  /* create crosshair cursor */
#ifdef RS6000
  xhair = XCreateCrossHairCursor(UxDisplay, 0, blue, 0);
#else
  xhair = XCreateFontCursor(UxDisplay, 68);
#endif
  if (VerboseFlag)
    printf("CreateCursors() : Created Xhair cursor...\n");
  XSync(UxDisplay, B_FALSE);
  
  /* pointer cursor */
  leftpointer = XCreateFontCursor(UxDisplay, 68);

  /* show message */
  if (VerboseFlag)
    printf("CreateCursors() : Created leftpointer cursor...\n");
  XSync(UxDisplay, B_FALSE);
}

/* display an error message and exit if in non-interactive mode */
void ErrorMsg(char *p)
{
  if (VerboseFlag || !InteractiveFlag) 
    printf("xvg error message: %s\n", p);

#ifdef CMGUI
  display_message(ERROR_MESSAGE, p);
#else
  if ((InteractiveFlag) && (ErrorMsgWindowCreated)) {
    UxPutMessageString(errorDialog, p);
    UxPopupInterface(errorDialog, no_grab);
  }
#ifdef ALL_INTERFACES
  else {
    if (UnimaInitialized)
      UnimaOff();
    printf("\axvg error : %s\n", p);
    exit(EXIT_FAILURE);
  }
#endif
#endif
}


/* display an Info message */
void InfoMsg(char *p)
{
  if (InteractiveFlag) {
    UxPutMessageString(InfoDialog, p);
    UxPopupInterface(InfoDialog, no_grab);
    XmUpdateDisplay(InfoDialog);
    XSync(UxDisplay, B_FALSE);
  }
  else
    printf("Info Message : %s\n", p);
}

void InfoMsgUpdate(char *p)
{
  if (InteractiveFlag) {
    UxPutMessageString(InfoDialog, p);
    XSync(UxDisplay, B_FALSE);
  }
}

void InfoMsgPopupOKOff(void)
{
  if (InteractiveFlag) {
    XtUnmanageChild((Widget) XmMessageBoxGetChild(InfoDialog,
					   XmDIALOG_OK_BUTTON));
    UxPopupInterface(InfoDialog, no_grab);
  }
}

void InfoMsgPopdownOKOn(void)
{
  if (InteractiveFlag) {
    UxPopdownInterface(InfoDialog);
    XtManageChild((Widget) XmMessageBoxGetChild(InfoDialog,
				       XmDIALOG_OK_BUTTON));
  }
}

void ScreenSaverOff(void)
{
  int TimeoutReturn, IntervalReturn;
  int PreferBlanckingReturn, AllowExposuresReturn;

  XSetScreenSaver(UxDisplay, 0, 0, DefaultBlanking, DefaultExposures);
  XGetScreenSaver(UxDisplay, &TimeoutReturn, &IntervalReturn,
		  &PreferBlanckingReturn, &AllowExposuresReturn);
  if (TimeoutReturn != 0)
    ErrorMsg("Unsuccessfull attempt to disable screen server");
  XResetScreenSaver(UxDisplay);
#ifdef DEBUG
  printf("ScreenSaverOff()\n");
#endif
}

void ScreenSaverOn(void)
{
  XSetScreenSaver(UxDisplay, -1, 0, DefaultBlanking, DefaultExposures);
#ifdef DEBUG
  printf("ScreenSaverOn()\n");
#endif
}

void ShowImage(double *p)
{
  extern int nprocessors;
  double max, min;

  if ((ChildProcess == B_FALSE) || (nprocessors == 1)) {
    DynamicRange(p, ImgHeight*ImgWidth, &max, &min);
    RealtoX(p, ProcessedPixelsX, ImgHeight, ImgWidth,
	    max, min, MaxScaleVal, MinScaleVal, ZoomFactor);
    XPutImage(UxDisplay, XtWindow(ProcessedImageDrawingArea), gc,
	      ProcessedImageXObj, 0, 0, 0, 0, ImgWidth*ZoomFactor,
	      ImgHeight*ZoomFactor);
    if (DisplayContourComputed)
      ContourOverlay(ProcessedImageDrawingArea);
    XmUpdateDisplay(UxTopLevel);
    XSync(UxDisplay, B_FALSE);
  }
}

void ResizeRawImageWidgets(boolean_t force)
{
  if ((OldRawImageWidgetHeight != (ImgHeight*ZoomFactor))
      || (OldRawImageWidgetWidth != (ImgWidth*ZoomFactor)) || force) {
    /* reset drawing area widget size */
    UxPutHeight(RawImageDrawingArea, (ImgHeight*ZoomFactor));
    UxPutWidth(RawImageDrawingArea, (ImgWidth*ZoomFactor));
    XSync(UxDisplay, B_FALSE);
    if (VerboseFlag)
      printf("ResizeRawImageWidgets() : Changed drawing area size...\n");

    /* reset shell window size */
    if (IsTrue(UxGetSet(XYSlicesTBG)))
      RawImageXYSlices(B_TRUE, B_TRUE);
    else
      RawImageXYSlices(B_FALSE, B_TRUE);
    XSync(UxDisplay, B_FALSE);
    if (VerboseFlag)
      printf("ResizeRawImageWidgets() : Changed slice area size...\n");

    /* reset local copies of height & width */
    OldRawImageWidgetHeight = (ImgHeight*ZoomFactor);
    OldRawImageWidgetWidth = (ImgWidth*ZoomFactor);
  }
  XSync(UxDisplay, B_FALSE);
  if (VerboseFlag)
    printf("ResizeRawImageWidgets() : Done\n");
}

void ResizeProcessedImageWidgets(boolean_t force)
{
	DATA_2D_PRECISION radius;

  if ((OldProcImageWidgetHeight != (ImgHeight*ZoomFactor))
      || (OldProcImageWidgetWidth != (ImgWidth*ZoomFactor)) || force) {
    XSync(UxDisplay, B_FALSE);
    if (VerboseFlag)
      printf("ResizeProcessedImageWidgets() About to change area size...\n");
    /* reset drawing area widget size */
    UxPutHeight(ProcessedImageDrawingArea, ImgHeight*ZoomFactor);
    UxPutWidth(ProcessedImageDrawingArea, ImgWidth*ZoomFactor);
    XSync(UxDisplay, B_FALSE);
    if (VerboseFlag)
      printf("ResizeProcessedImageWidgets() Drawing area size changed...\n");
    
    /* reset shell window size */
    if (IsTrue(UxGetSet(XYSlicesTBG)))
      ProcessedImageXYSlices(B_TRUE, B_TRUE);
    else
      ProcessedImageXYSlices(B_FALSE, B_TRUE);
    XSync(UxDisplay, B_FALSE);
    if (VerboseFlag)
      printf("ResizeProcessedImageWidgets() Slice size changed...\n");

    /* reset local copies of height & width */
    OldProcImageWidgetHeight = ImgHeight*ZoomFactor;
    OldProcImageWidgetWidth = ImgWidth*ZoomFactor;
		if (data_2d_dialog)
		{
			/* update the selection radius */
			radius = 5.0/ZoomFactor;
			data_2d_dialog_set_data(data_2d_dialog,DATA_2D_DIALOG_SELECT_RADIUS,
				&radius);
		}
    XSync(UxDisplay, B_FALSE);
    if (VerboseFlag)
      printf("ResizeProcessedImageWidgets() Done...\n");
  }
}

/* create and load processed image components */
void CreateRawImage(void)
{
  XEvent e;
  XmDrawingAreaCallbackStruct s;
  char buffer[512];

#ifdef DEBUG
    printf("CreateRawImage() : Entered\n");
#endif

  /* check for initial conditions */
  if (RawPixels == NULL)
    return;

  /* set window title */
  sprintf(buffer, "%s (Raw)", InFileNameStripped);
  UxPutTitle(RawImageShell, buffer);

  /* allocate storage  & create the image object if required  */
  if ((OldRawImgHeight != (ImgHeight*ZoomFactor))
      || (OldRawImgWidth != (ImgWidth*ZoomFactor))) {
#ifdef DEBUG
    printf("CreateRawImage() : Creating\n");
#endif

    /* image object */
    if ((RawPixelsX = (char *) XvgMM_Alloc((void *) RawPixelsX,
					   ImgHeight*ImgWidth
					   *ZoomFactor*ZoomFactor))
	== NULL) {
      ErrorMsg("CreateRawImage() : Error allocating for X image buffer");
      return;
    }
    if (RawImageXObj != NULL) {
      RawImageXObj->data = NULL;
      XDestroyImage(RawImageXObj);
    } 
    RawImageXObj = XCreateImage(UxDisplay, XDefaultVisual(UxDisplay, UxScreen),
				8, ZPixmap, 0, RawPixelsX, ImgWidth*ZoomFactor,
				ImgHeight*ZoomFactor, 8, 0);
    /* slice projections buffer */
    if ((RawPoints = (XPoint *) XvgMM_Alloc((void *) RawPoints,
					    sizeof(XPoint) * ZoomFactor 
					    * (ImgHeight > ImgWidth ?
					       ImgHeight : ImgWidth)))
	== NULL) {
      ErrorMsg("CreateRawImage() : Error allocating for projection buffer");
      return;
    }

    /* reset local copies of height & width */
    OldRawImgHeight = ImgHeight*ZoomFactor;
    OldRawImgWidth = ImgWidth*ZoomFactor;
  }

#ifdef DEBUG
    printf("CreateRawImage() : Updating drawing area\n");
#endif

  /* convert data to X pixel indicies */
  if (TrueColorImage)
    DoublesToBytesZoom(RawPixels, RawPixelsX, ImgHeight, ImgWidth, ZoomFactor);
  else
    RealtoX(RawPixels, RawPixelsX, ImgHeight, ImgWidth, RawMax,
	    RawMin, HIST_SCALE_MAX, HIST_SCALE_MIN, ZoomFactor);

  /* redraw the processed window */
  s.event = &e;
  s.event->xexpose.x = 0;
  s.event->xexpose.y = 0;
  s.event->xexpose.width = ImgWidth*ZoomFactor;
  s.event->xexpose.height = ImgHeight*ZoomFactor;
  RawExposeCB(RawImageShell, NULL, &s);

#ifdef DEBUG
    printf("CreateRawImage() : Done\n");
#endif
}

void CreateProcImage(void)
{
  char buffer[512];

  /* check for initial conditions */
  if (ProcessedPixels == NULL)
    return;

  /* set window title */
  sprintf(buffer, "%s (Processed)", InFileNameStripped);
  UxPutTitle(ProcessedImageShell, buffer);

  /* allocate storage  & create the image object if required  */
  if ((OldProcImgHeight != (ImgHeight*ZoomFactor))
      || (OldProcImgWidth != (ImgWidth*ZoomFactor))) {
    /* image and data buffer */
    if ((ProcessedPixelsX = (char *) XvgMM_Alloc((void *) ProcessedPixelsX,
						 ImgHeight*ImgWidth
						 *ZoomFactor*ZoomFactor))
	== NULL) {
      ErrorMsg("CreateProcImage() : Error allocating for X image buffer");
      return;
    }
    if (ProcessedImageXObj != NULL) {
      ProcessedImageXObj->data = NULL;
      XDestroyImage(ProcessedImageXObj);
    }
    ProcessedImageXObj = XCreateImage(UxDisplay,
				      XDefaultVisual(UxDisplay, UxScreen),
				      8, ZPixmap, 0, ProcessedPixelsX,
				      ImgWidth*ZoomFactor,ImgHeight*ZoomFactor,
				      8, 0);
    DEBUGS("CreateProcImage() : Created image");

    /* slice projections buffer */
    if ((ProcessedPoints = (XPoint *) XvgMM_Alloc((void *) ProcessedPoints,
						  sizeof(XPoint) * ZoomFactor 
						  * (ImgHeight > ImgWidth ?
						     ImgHeight : ImgWidth)))
	== NULL) {
      ErrorMsg("CreateProcImage() : Error allocating for projection buffer");
      return;
    }
    /* reset local copies of height & width */
    OldProcImgHeight = ImgHeight*ZoomFactor;
    OldProcImgWidth = ImgWidth*ZoomFactor;
  }
  RealtoX(ProcessedPixels, ProcessedPixelsX, ImgHeight, ImgWidth, ProcMax,
	  ProcMin, MaxScaleVal, MinScaleVal, ZoomFactor);
  XSync(UxDisplay, B_FALSE);
  if (VerboseFlag)
    printf("CreateProcImage() : Image created...\n");

  /* redraw the processed window */
  RedrawProcDrawingArea();
  XSync(UxDisplay, B_FALSE);
  if (VerboseFlag)
    printf("CreateProcImage() : Drawing area redrawn...\n");
}

void GrabbingState(boolean_t flag)
{
  char buffer[512];
  int i;

  if (flag) {
    /* init VCAA */
    if (InteractiveFlag) {
      if (VCAAConnectionOpen == B_FALSE)
	VCAAInit();
    }
    
    if (IsTrue(UxGetSet(RawImageTBG)) == B_FALSE) {
      RawWinHOffset = 0;
      RawWinVOffset = 0;
      RawWinWidth = FrameWidth;
      RawWinHeight = FrameHeight;
    }
    if (IsTrue(UxGetSet(ProcessedImageTBG)) == B_FALSE) {
      ProcessedWinHOffset = 0;
      ProcessedWinVOffset = 0;
      ProcessedWinWidth = FrameWidth;
      ProcessedWinHeight = FrameHeight;
    }

    /* set image window and buffer parameters */
    TrueColorImage = B_FALSE;
    if ((ImgHeight != FrameHeight) || (ImgWidth != FrameWidth)) {
      if (VerboseFlag)
	printf("GrabbingState() : Allocating storage for RAW image\n");
      ImgHeight = FrameHeight;
      ImgWidth = FrameWidth;
      if ((RawPixels = (double *) XvgMM_Alloc((void *) RawPixels,
					      FrameHeight*FrameWidth
					      *sizeof(double)))
	  == NULL) {
	sprintf(buffer,
		"GrabbingState(): Error allocating %d bytes for raw image",
		FrameHeight*FrameWidth*sizeof(double));
	ErrorMsg(buffer);
	return;
      }

      /* init buffers */
      for (i = 0; i < ImgHeight*ImgWidth; i++)
	RawPixels[i] = 0;

      /* create & resize the processed Image widgets */
      if (ProcessedImageShellCreated) {
	if (VerboseFlag)
	  printf("GrabbingState() : Allocating storage for PROC image\n");
	if ((ProcessedPixels = (double *) XvgMM_Alloc((void *) ProcessedPixels,
						      FrameHeight*FrameWidth
						      *sizeof(double)))
	    == NULL) {
	  sprintf(buffer,
		  "GrabbingState(): Error allocating %d bytes for proc image",
		  FrameHeight*FrameWidth*sizeof(double));
	  ErrorMsg(buffer);
	  return;
	}
	
	ResizeProcessedImageWidgets(B_FALSE);
	InitProcBuffers();
	MoveDoubles(RawPixels, ProcessedPixels, FrameHeight*FrameWidth);
	CreateProcImage();
      }
    }
    
    /* create the raw Image widgets if they don't exist already & resize 'em*/
    if (RawImageShellCreated == B_FALSE) {
      RawImageShell = create_RawImageShell();
      UxPopupInterface(RawImageShell, no_grab);
      UpdateRawWinParms();
    }
    else {
      ResizeRawImageWidgets(B_FALSE);
      CreateRawImage();
      if (VerboseFlag)
	printf("GrabbingState() : Resized RAW image window\n");
    }
    
    /* set image size to frame grabber size */
    if (VerboseFlag)
      printf("GrabbingState() : ImgHeight:%d, ImgWidth:%d\n",
	     ImgHeight, ImgWidth);
    
    /* set flag */
    GrabbingOn = B_TRUE;
  }
  else
    GrabbingOn = B_FALSE;
}

void InitProcBuffers(void)
{
  if (RawPixels != NULL) {
    MaskPixels = (double *) XvgMM_Alloc((void *) MaskPixels,
					 ImgHeight*ImgWidth*sizeof(double));
    DoubleBuffer = (double *) XvgMM_Alloc((void *) DoubleBuffer,
					   ImgHeight*ImgWidth*sizeof(double));
    ProcessedPixels = (double *) XvgMM_Alloc((void *) ProcessedPixels,
					      ImgHeight*ImgWidth
					      *sizeof(double));
    /* check if storage was successfull */
    if ((MaskPixels == NULL)
	|| (MallocScratchDouble() == B_FALSE)
	|| (DoubleBuffer == NULL) || (ProcessedPixels == NULL)) {
      sprintf(cbuf, "InitProcBuffers() : Could not allocate memory (%dx%d)",
	      ImgWidth, ImgHeight);
      ErrorMsg(cbuf);
      return;
    }
    InitDoubleBuffering();
  }
  if (VerboseFlag)
    printf("InitProcBuffers() : Done\n");
}

/* load image file from disk */
boolean_t LoadFile(char *name)
{
  Pixmap RawPixmap;
  boolean_t Compressed, rc;
  int i, j, ccnt, datatype, background, bg;
  char vname[512], buffer[512], fname[512], *q;
  double LocalMin, LocalMax, noise_norm;

  /* speack out if required */
  if (VerboseFlag)
    printf("LoadFile(%s)...\n", name);

  /* cancel grabbing */
  GrabbingOn = B_FALSE;

  /* set flags */
  rc = B_TRUE;
  Compressed = B_FALSE;

  /* if no filename specified, define 640x480 buffers & windows */
  if (name == NULL) {
    if ((ImgHeight != 480) || (ImgWidth != 640)) {
      ImgHeight = 480;
      ImgWidth = 640;
      if ((RawPixels = (double *) XvgMM_Alloc((void *) RawPixels,
					      ImgHeight*ImgWidth
					      *sizeof(double))) == NULL) {
	sprintf(buffer,
		"LoadFile(): Error allocating %d bytes for raw image",
		ImgHeight*ImgWidth*sizeof(double));
	ErrorMsg(buffer);
	return(B_FALSE);
      }
      
      /* init buffer with noise */
      RawMin = 0;
      RawMax = 1;
      noise_norm = 1.0 / ((double) MAXINT);
      for (i = 0; i < ImgHeight*ImgWidth; i++)
	RawPixels[i] = ((double) random()) * noise_norm;
    }
  }
  else {
    /* set current filename if required */
    if (strcmp(name, InFileName) != 0) {
      InFileName = (char *) XvgMM_Alloc((void *) InFileName, strlen(name) + 1);
      sprintf(InFileName, "%s", name);
    }

    /* parse file name */
    sprintf(fname, "%s", name);
    for (j = (strlen(fname) - 1); (j >= 0) && (fname[j] != '/'); j--);
    for (i = 0, j = (j == 0 ? 0 : j+1); fname[j] != '\0'; i++, j++)
      InFileNameStripped[i] = fname[j];
    InFileNameStripped[i] = '\0';
    for (j = (strlen(fname) - 1); (j >= 0) && (fname[j] != '.'); j--);
    for (i = 0; fname[j] != '\0'; i++, j++)
      InFileNameExt[i] = fname[j];
    InFileNameExt[i] = '\0';
    
    /* check for compressed file */
    if (strcmp(".Z", InFileNameExt) == 0) {
      sprintf(buffer, "uncompress %s", fname);
      system(buffer);
      fname[j-2] = '\0';
      /* get new file extension */
      for (j = (strlen(fname) - 1); (j >= 0) && (fname[j] != '.'); j--);
      for (i = 0; fname[j] != '\0'; i++, j++)
	InFileNameExt[i] = fname[j];
      InFileNameExt[i] = '\0';
      Compressed = B_TRUE;
      if (VerboseFlag)
	printf("LoadFile() : Uncompressed file %s\n", fname);
    }
    else if (strcmp(".gz", InFileNameExt) == 0) {
      sprintf(buffer, "gunzip %s", fname);
      system(buffer);
      fname[j-3] = '\0';
      /* get new file extension */
      for (j = (strlen(fname) - 1); (j >= 0) && (fname[j] != '.'); j--);
      for (i = 0; fname[j] != '\0'; i++, j++)
	InFileNameExt[i] = fname[j];
      InFileNameExt[i] = '\0';
      Compressed = B_TRUE;
      if (VerboseFlag)
	printf("LoadFile() : Uncompressed file %s\n", fname);
    }
    else {
      Compressed = B_FALSE;
    }
    
    /* TIFF file */
    if ((strcmp(".tiff", InFileNameExt) == 0) 
	|| (strcmp(".tif", InFileNameExt) == 0)) {
#ifdef DEBUG
      printf("LoadFile(%s) : Loading tiff file\n", name);
#endif
      TrueColorImage = B_FALSE;
      if ((rc = GetTiffFile(fname,&RawPixels,&ImgHeight, &ImgWidth, &LocalMin,
			    &LocalMax, cmap, UserCells, &TrueColorImage,
			    VerboseFlag)) && (FixDynamicRange == B_FALSE)) {
#ifdef DEBUG
	printf("LoadFile() : return from GetTiffFile() OK\n");
#endif
	RawMin = LocalMin;
	RawMax = LocalMax;
      }
      else {
#ifdef DEBUG
	printf("LoadFile() : return from GetTiffFile FAILED()\n");
#endif
      }
    }
    /* MATLAB file */
    else if (strcmp(".mat", InFileNameExt) == 0) {
      TrueColorImage = B_FALSE;
      if ((rc = GetMatlabFile(&ImgHeight, &ImgWidth, &LocalMin, &LocalMax,
			      &RawPixels, fname, VerboseFlag))
	  && (FixDynamicRange == B_FALSE)) {
	RawMin = LocalMin;
	RawMax = LocalMax;
      }
    }
    /* short data file */
    else if (strcmp(".dat", InFileNameExt) == 0) {
      TrueColorImage = B_FALSE;
      if ((rc = GetDataFile(&ImgHeight, &ImgWidth, &LocalMin, &LocalMax,
			    &RawPixels, fname, VerboseFlag))
	  && (FixDynamicRange == B_FALSE)) {
	RawMin = LocalMin;
	RawMax = LocalMax;
      }
    }
    /* SGI rgb file */
    else if (strcmp(".rgb", InFileNameExt) == 0) {
      TrueColorImage = B_FALSE;
      if ((rc = GetSGIRGBFile(fname, &RawPixels, &ImgHeight, &ImgWidth,
			      &LocalMin, &LocalMax))
	  && (FixDynamicRange == B_FALSE)) {
	RawMin = LocalMin;
	RawMax = LocalMax;
      }
    }
    /* SGI interlaced rgb file */
    else if (strcmp(".sgi", InFileNameExt) == 0) {
      TrueColorImage = B_FALSE;
      if ((rc = GetFieldFromSGIRGBFile(fname, 1, &RawPixels,
                                       &ImgHeight, &ImgWidth,
			               &LocalMin, &LocalMax))
	  && (FixDynamicRange == B_FALSE)) {
	RawMin = LocalMin;
	RawMax = LocalMax;
      }
    }
    /* MATLAB raw step image file */
    else if (strcmp(".smat", InFileNameExt) == 0) {
      TrueColorImage = B_FALSE;
      if ((rc = DeCompReadMatlabImages(&ImgHeight, &ImgWidth, &LocalMin,
				       &LocalMax, &RawPixels, fname,
				       VerboseFlag))
	  && (FixDynamicRange == B_FALSE)) {
	RawMin = LocalMin;
	RawMax = LocalMax;
      }
    }
    /* B-spline file */
    else if (strcmp(".bspl", InFileNameExt) == 0) {
      TrueColorImage = B_FALSE;
      if ((rc = GetBSplineFile(fname, &ImgHeight, &ImgWidth, &LocalMin,
			       &LocalMax, &RawPixels, VerboseFlag))
	  && (FixDynamicRange == B_FALSE)) {
	RawMin = LocalMin;
	RawMax = LocalMax;
      }
    }
    /* laser scan ASCII file */
    else if (strcmp(".scan", InFileNameExt) == 0) {
      TrueColorImage = B_FALSE;
      if ((rc = GetScanImage(fname, &ImgHeight, &ImgWidth, &LocalMin,
			     &LocalMax, &RawPixels, VerboseFlag))
	  && (FixDynamicRange == B_FALSE)) {
	RawMin = LocalMin;
	RawMax = LocalMax;
      }
    }
    /* ".px" file */
    else if (strcmp(".px", InFileNameExt) == 0) {
      TrueColorImage = B_TRUE;
      if ((q = LoadPxData(fname, cmap, white, B_FALSE,
			  UserCells, &ImgWidth, &ImgHeight)) == NULL)
	rc = B_FALSE;
      else {
	if ((RawPixels = (double *) XvgMM_Alloc((void *) RawPixels,
						ImgHeight*ImgWidth
						*sizeof(double)))
	    == NULL) {
	  sprintf(buffer,
		  "LoadFile(): Error allocating %d bytes for raw image",
		  ImgHeight*ImgWidth*sizeof(double));
	  ErrorMsg(buffer);
	  return(B_FALSE);
	}
	CharToDouble(q, RawPixels, ImgHeight*ImgWidth);
	XvgMM_Free(q);
	RawMin = 0;
	RawMax = 255;
	if ((CurrentCells == UserCells) && (PagedCmap))
	  XStoreColors(UxDisplay, cmap, UserCells, NPAGEDLUTCELLS);
      }
    }
    /* ".xwd" file */
    else if (strcmp(".xwd", InFileNameExt) == 0) {
      TrueColorImage = B_TRUE;
      if (rc = LoadXWDFile(fname, &RawPixels,&ImgHeight, &ImgWidth, cmap,
			   UserCells, VerboseFlag)) {
	RawMin = 0;
	RawMax = 255;
	if ((CurrentCells == UserCells) && (PagedCmap))
	  XStoreColors(UxDisplay, cmap, UserCells, NPAGEDLUTCELLS);
      }
    }
    /* unsupported file type */
    else {
      sprintf(buffer, "File \"%s\" has an unsupported file extension!", fname);
      ErrorMsg(buffer);
      rc = B_FALSE;
    }
  }

  /* kludge to set window parameters right in case window not popped up */
  if (InteractiveFlag) {
    if (IsTrue(UxGetSet(RawImageTBG)) == B_FALSE) {
      RawWinHOffset = 0;
      RawWinVOffset = 0;
      RawWinWidth = ImgWidth;
      RawWinHeight = ImgHeight;
    }
    if (IsTrue(UxGetSet(ProcessedImageTBG)) == B_FALSE) {
      ProcessedWinHOffset = 0;
      ProcessedWinVOffset = 0;
      ProcessedWinWidth = ImgWidth;
      ProcessedWinHeight = ImgHeight;
    }
  }

  /* show filename & size on xvg window */
  if (InteractiveFlag && rc) {
    /* display filename and size in main window */
    if (name != NULL) {
      sprintf(buffer, "File: %s", InFileNameStripped);
      UxPutLabelString(FilenameLG, buffer);
      if (VerboseFlag)
	printf("LoadFile() : filename label loaded...\n");
    }
    sprintf(buffer, "Size: %d x %d", ImgWidth, ImgHeight);
    UxPutLabelString(SizeLG, buffer);
  }
    
  if ((InteractiveFlag || DisplayOnlyFlag) && rc) {
    /* create the raw Image widgets if they don't exist already, resize them */
    if (RawImageShellCreated == B_FALSE)
      RawImageShell = create_RawImageShell();
    else {
      ResizeRawImageWidgets(B_FALSE);
      CreateRawImage();
    }
  }

  if (InteractiveFlag && rc) {
    /* create & resize the processed Image widgets */
    if ((ProcessedImageShellCreated) && (TrueColorImage == B_FALSE)
	&& (XvgMM_GetBlockSize((void *) ProcessedPixels)
	    != (ImgHeight*ImgWidth*sizeof(double)))) {
      InitProcBuffers();
      ProcMax = RawMax;
      ProcMin = RawMin;
      MoveDoubles(RawPixels, ProcessedPixels, ImgHeight*ImgWidth);
      ResizeProcessedImageWidgets(B_FALSE);
      CreateProcImage();
      if (IsTrue(UxGetSet(HistogramTBG)))
	Histogram(ProcessedPixels, ProcessedWinHOffset, ProcessedWinVOffset,
		  ProcessedWinWidth, ProcessedWinHeight, ImgWidth, ProcMax,
		  ProcMin, ProcessedHistogram, ProcessedHistogramRectangles,
		  &ProcMean, &ProcSD);
    }
    
    /* compute and display the histograms if required */
    if (IsTrue(UxGetSet(HistogramTBG))) {
      Histogram(RawPixels, RawWinHOffset, RawWinVOffset, RawWinWidth,
		RawWinHeight, ImgWidth, RawMax, RawMin, RawHistogram,
		RawHistogramRectangles, &RawMean, &RawSD);
      DrawHistograms(RawMin, RawMax, ProcMin, ProcMax);
    }
  }

  /* recompress file if required */
/*
  if (Compressed && Recompress) {
    sprintf(buffer, "compress %s", fname);
    system(buffer);
  }
*/

  /* return success or failure */
  XvgMM_MemoryCheck("Loadfile");
  return(rc);  
}

static void LoadUserCells(void)
{
  int i;

/*
  for (i = 0; i < 64; i++) {
    UserCells[i].red = i << 10;
    UserCells[i].green = i << 10;
    UserCells[i].blue = i << 10;
    UserCells[i].flags = DoRed | DoGreen | DoBlue;
    UserCells[i].pixel = LutCells[i];
  }
  for (; i < 128; i++) {
    UserCells[127-i].red = i << 10;
    UserCells[127-i].green = i << 10;
    UserCells[127-i].blue = i << 10;
    UserCells[127-i].flags = DoRed | DoGreen | DoBlue;
    UserCells[127-i].pixel = LutCells[i];
  }
  for (; i < NPAGEDLUTCELLS; i++)
    UserCells[i].pixel = LutCells[i];
*/
  for (i = 0; i < NPAGEDLUTCELLS; i++) {
    UserCells[i].red = i << 10;
    UserCells[i].green = i << 10;
    UserCells[i].blue = i << 10;
    UserCells[i].pixel = LutCells[i];
    UserCells[i].flags = DoRed | DoGreen | DoBlue;
  }
}

void LoadGreyLevelCells(int wfactor)
{
  int i, v;
  
  for (i = 0; i < 128; i++) {
    v = ((i + wfactor) > 127 ? (i + wfactor - 128) : (i + wfactor));
    GreyLevelCells[i].red = v << 9;
    GreyLevelCells[i].green = v << 9;
    GreyLevelCells[i].blue = v << 9;
    GreyLevelCells[i].flags = DoRed | DoGreen | DoBlue;
    GreyLevelCells[i].pixel = LutCells[i];
  }
  for (; i < NPAGEDLUTCELLS; i++)
    GreyLevelCells[i].pixel = LutCells[i];
}

void LoadGammaGreyLevelCells(void)
{
  int i;

  for (i = 0; i < 128; i++) {
    GammaGreyLevelCells[i].red = ((int)(sqrt((2*i)/255.0)*255.0*GcFactor
					+ 2*i*(1.0-GcFactor))) << 8;
    GammaGreyLevelCells[i].green = GammaGreyLevelCells[i].red;
    GammaGreyLevelCells[i].blue = GammaGreyLevelCells[i].red;
    GammaGreyLevelCells[i].flags = DoRed | DoGreen | DoBlue;
    GammaGreyLevelCells[i].pixel = LutCells[i];
  }
  for (; i < NPAGEDLUTCELLS; i++)
    GammaGreyLevelCells[i].pixel = LutCells[i];
}

void LoadBRYGMBVWCells(void)
{
  int i;

  for (i = 0; i < 128; i++) {
    BRYGMBVMCells[i].flags = DoRed | DoGreen | DoBlue;
    BRYGMBVMCells[i].pixel = LutCells[i];
  }
  for (; i < NPAGEDLUTCELLS; i++)
    BRYGMBVMCells[i].pixel = LutCells[i];
}

void InitHierarchy(void)
{
  FILE *fp;

#ifdef _RS6000_UID_PATH
  static char *uid_filenames[] = {
    "/u/paul/xvg/cmgui/XvgShell.uid",
    "/u/paul/xvg/cmgui/fileSelectionBoxDialog.uid",
    "/u/paul/xvg/cmgui/RawImageShell.uid",
    "/u/paul/xvg/cmgui/HistogramShell.uid",
    "/u/paul/xvg/cmgui/InfoDialog.uid",
    "/u/paul/xvg/cmgui/ProcessedImageShell.uid",
    "/u/paul/xvg/cmgui/IPOperationsShell.uid",
    "/u/paul/xvg/cmgui/LUTShell.uid",
    "/u/paul/xvg/cmgui/promptDialog.uid",
    "/u/paul/xvg/cmgui/errorDialog.uid",
    "/u/paul/xvg/cmgui/MovieControlsShell.uid",
    "/u/paul/xvg/cmgui/InputDialog.uid",
    "/u/paul/xvg/cmgui/ScrolledWindowDialog.uid",
    "/u/paul/xvg/cmgui/GLShell.uid"
  };
#else
#if defined (SGI) && !defined (CMGUI)
  static char *uid_filenames[] = {
    "/usr/people/paulc/xvg/cmgui/XvgShell.uid",
    "/usr/people/paulc/xvg/cmgui/fileSelectionBoxDialog.uid",
    "/usr/people/paulc/xvg/cmgui/RawImageShell.uid",
    "/usr/people/paulc/xvg/cmgui/HistogramShell.uid",
    "/usr/people/paulc/xvg/cmgui/InfoDialog.uid",
    "/usr/people/paulc/xvg/cmgui/ProcessedImageShell.uid",
    "/usr/people/paulc/xvg/cmgui/IPOperationsShell.uid",
    "/usr/people/paulc/xvg/cmgui/LUTShell.uid",
    "/usr/people/paulc/xvg/cmgui/promptDialog.uid",
    "/usr/people/paulc/xvg/cmgui/errorDialog.uid",
    "/usr/people/paulc/xvg/cmgui/MovieControlsShell.uid",
    "/usr/people/paulc/xvg/cmgui/InputDialog.uid",
    "/usr/people/paulc/xvg/cmgui/ScrolledWindowDialog.uid",
    "/usr/people/paulc/xvg/cmgui/GLShell.uid"
  };
  static char *uid_filenames_blob[] = {
    "/usr/people/paul/xvg/cmgui/XvgShell.uid",
    "/usr/people/paul/xvg/cmgui/fileSelectionBoxDialog.uid",
    "/usr/people/paul/xvg/cmgui/RawImageShell.uid",
    "/usr/people/paul/xvg/cmgui/HistogramShell.uid",
    "/usr/people/paul/xvg/cmgui/InfoDialog.uid",
    "/usr/people/paul/xvg/cmgui/ProcessedImageShell.uid",
    "/usr/people/paul/xvg/cmgui/IPOperationsShell.uid",
    "/usr/people/paul/xvg/cmgui/LUTShell.uid",
    "/usr/people/paul/xvg/cmgui/promptDialog.uid",
    "/usr/people/paul/xvg/cmgui/errorDialog.uid",
    "/usr/people/paul/xvg/cmgui/MovieControlsShell.uid",
    "/usr/people/paul/xvg/cmgui/InputDialog.uid",
    "/usr/people/paul/xvg/cmgui/ScrolledWindowDialog.uid",
    "/usr/people/paul/xvg/cmgui/GLShell.uid"
  };
  static char *uid_filenames_hpc1[] = {
    "/xlv1/eng_sci/charette/xvg/XvgShell.uid",
    "/xlv1/eng_sci/charette/xvg/fileSelectionBoxDialog.uid",
    "/xlv1/eng_sci/charette/xvg/RawImageShell.uid",
    "/xlv1/eng_sci/charette/xvg/HistogramShell.uid",
    "/xlv1/eng_sci/charette/xvg/InfoDialog.uid",
    "/xlv1/eng_sci/charette/xvg/ProcessedImageShell.uid",
    "/xlv1/eng_sci/charette/xvg/IPOperationsShell.uid",
    "/xlv1/eng_sci/charette/xvg/LUTShell.uid",
    "/xlv1/eng_sci/charette/xvg/promptDialog.uid",
    "/xlv1/eng_sci/charette/xvg/errorDialog.uid",
    "/xlv1/eng_sci/charette/xvg/MovieControlsShell.uid",
    "/xlv1/eng_sci/charette/xvg/InputDialog.uid",
    "/xlv1/eng_sci/charette/xvg/ScrolledWindowDialog.uid",
    "/xlv1/eng_sci/charette/xvg/GLShell.uid"
  };
  static char *uid_filenames_local[] = {
    "XvgShell.uid",
    "fileSelectionBoxDialog.uid",
    "RawImageShell.uid",
    "HistogramShell.uid",
    "InfoDialog.uid",
    "ProcessedImageShell.uid",
    "IPOperationsShell.uid",
    "LUTShell.uid",
    "promptDialog.uid",
    "errorDialog.uid",
    "MovieControlsShell.uid",
    "InputDialog.uid",
    "ScrolledWindowDialog.uid",
    "GLShell.uid"
  };
#else
#include "xvg/cmgui/XvgShell.uid64"
#include "xvg/cmgui/fileSelectionBoxDialog.uid64"
#include "xvg/cmgui/RawImageShell.uid64"
#include "xvg/cmgui/HistogramShell.uid64"
#include "xvg/cmgui/InfoDialog.uid64"
#include "xvg/cmgui/ProcessedImageShell.uid64"
#include "xvg/cmgui/IPOperationsShell.uid64"
#include "xvg/cmgui/LUTShell.uid64"
#include "xvg/cmgui/promptDialog.uid64"
#include "xvg/cmgui/MovieControlsShell.uid64"
#include "xvg/cmgui/InputDialog.uid64"
#include "xvg/cmgui/ScrolledWindowDialog.uid64"
#if defined (CMGUI)
  static char *uid_strings[] = {
    XvgShell_uid64,
    fileSelectionBoxDialog_uid64,
    RawImageShell_uid64,
    HistogramShell_uid64,
    InfoDialog_uid64,
    ProcessedImageShell_uid64,
    IPOperationsShell_uid64,
    LUTShell_uid64,
    promptDialog_uid64,
    MovieControlsShell_uid64,
    InputDialog_uid64,
    ScrolledWindowDialog_uid64
  };
#else /* defined (CMGUI) */
  static char *uid_filenames[] = {
    "XvgShell.uid",
    "fileSelectionBoxDialog.uid",
    "RawImageShell.uid",
    "HistogramShell.uid",
    "InfoDialog.uid",
    "ProcessedImageShell.uid",
    "IPOperationsShell.uid",
    "LUTShell.uid",
    "promptDialog.uid",
    "errorDialog.uid",
    "MovieControlsShell.uid",
    "InputDialog.uid",
    "ScrolledWindowDialog.uid",
    "GLShell.uid"
  };
#endif /* defined (CMGUI) */
#endif
#endif
  int number_of_uid_files;
#if defined (CMGUI)
  int number_of_uid_strings;
	int hierarchy_open;
#endif /* defined (CMGUI) */

  
  number_of_uid_strings = XtNumber(uid_strings);

#if defined (SGI) && !defined(CMGUI)
  if (fp = fopen(uid_filenames[0], "r")) {
    fclose(fp);
    if (MrmSUCCESS != MrmOpenHierarchy(number_of_uid_files,
				       uid_filenames, NULL, &hierarchy))
      {
	printf("Could not open hierarchy\n");
	exit(EXIT_FAILURE);
      }
  }
  else if (fp = fopen(uid_filenames_blob[0], "r")) {
    fclose(fp);
    if (MrmSUCCESS != MrmOpenHierarchy(number_of_uid_files,
				       uid_filenames_blob, NULL, &hierarchy))
      {
	printf("Could not open hierarchy\n");
	exit(EXIT_FAILURE);
      }
  }
  else if (fp = fopen(uid_filenames_hpc1[0], "r")) {
    fclose(fp);
    if (MrmSUCCESS != MrmOpenHierarchy(number_of_uid_files,
				       uid_filenames_hpc1, NULL, &hierarchy))
      {
	printf("Could not open hierarchy\n");
	exit(EXIT_FAILURE);
      }
  }
  else {
    if (MrmSUCCESS != MrmOpenHierarchy(number_of_uid_files,
				       uid_filenames_local, NULL, &hierarchy))
      {
	printf("Could not open hierarchy\n");
	exit(EXIT_FAILURE);
      }
  }
#else
#if defined (CMGUI)
	hierarchy_open=0;
	if (!MrmOpenHierarchy_base64_multiple_strings(number_of_uid_strings,uid_strings,
		&hierarchy,&hierarchy_open))
#else /* defined (CMGUI) */
  if (MrmSUCCESS != MrmOpenHierarchy(number_of_uid_files,
				     uid_filenames, NULL, &hierarchy))
#endif /* defined (CMGUI) */
    {
      printf("Could not open hierarchy\n");
      exit(EXIT_FAILURE);
    }
#endif
}

void XvgInitVariables(void)
{
  FILE *fp;
  int i;

  /* initialize all variables */
  for (i = 0; i < REGIONLISTSIZE; i++) {
    RegionList[i].Neighbours = NULL;
    RegionList[i].NeighbourIndicies = NULL;
    RegionList[i].ContourElements = NULL;
    RegionList[i].NormalElements = NULL;
  }

  (CurrentIPOp.ParmPtrs)[0] = NULL;
  (CurrentIPOp.ParmPtrs)[1] = NULL;
  (CurrentIPOp.ParmPtrs)[2] = NULL;
  (CurrentIPOp.ParmPtrs)[3] = NULL;
  CurrentCells = NULL;
  ProcessedImageXObj  =NULL;
  RawImageXObj =NULL;
  ProcessedFTImageXObj =NULL;
  RawFTImageXObj =NULL;
  ProcessedPoints =NULL;
  RawPoints =NULL;
  KnotPoints =NULL;
  ContourPoints =NULL;
  StrainElements = NULL;
  DispFieldElements = NULL;
  NDispFieldElements = 0;
  Abort =B_FALSE;
  ImageUpdateFlag = B_FALSE;
  AtanLutCreated = B_FALSE;
  GLShellCreated = B_FALSE;
  GLWindowMapped = B_FALSE;
  ColorBarOverlay = B_FALSE;
  ComplexDataSet = B_FALSE;
  DisplayContourComputed =B_FALSE;
  Monochrome =B_FALSE;
  Convert_RGB_TIFF_To_Graylevel = B_TRUE;
  DisplayOnlyFlag = B_FALSE;
  ErrorMsgWindowCreated = B_FALSE;
  FixDynamicRange = B_FALSE;
  EraserOn = B_FALSE;
  GrabbingOn = B_FALSE;
  HistogramShellCreated =B_FALSE;
  InteractiveFlag = B_TRUE;
  InternalChecks = B_TRUE;
  IPOperationsShellCreated =B_FALSE;
  IPOpExecVerbose =B_FALSE;
  LUTShellCreated = B_FALSE;
  BSplineComputed = B_FALSE;
  XMovieLoaded = B_FALSE;
  MovieControlsShellCreated = B_FALSE;
#ifdef SGI
  PagedCmap = B_FALSE;
#else
  PagedCmap = B_TRUE;
#endif
  promptDialogUp = B_FALSE;
  ProcessedImageShellCreated =B_FALSE;
  RawImageShellCreated =B_FALSE;
  NoNormalsOverlay = B_FALSE;
  Recompress = B_TRUE;
  SGIImageHalfWidth = B_FALSE;
  SGIRGBFullWidth = -1;
  SGIRGBFullHeight = -1;
  TrueColorImage = B_FALSE;
  SGIFBInitialized = B_FALSE;
  VCRInitDone = B_FALSE;
  VCAWriteOn = B_FALSE;
  VCRRecordOn = B_FALSE;
  VerboseFlag = B_FALSE;
/*  XvgMM_MemoryCheck_Flag = B_FALSE; */
  XvgMM_MemoryCheck_Flag = B_TRUE;
  WhiteLightImaging = B_FALSE;
  DrawMesgEnabled = B_TRUE;
  InFileName =NULL;
  OldRawImageWidgetHeight = 0;
  OldRawImageWidgetWidth = 0;
  OldRawImgHeight = 0;
  OldRawImgWidth = 0;
  OldProcImageWidgetHeight = 0;
  OldProcImageWidgetWidth = 0;
  OldProcImgHeight = 0;
  OldProcImgWidth = 0;
#ifdef SGI
  if (fp = fopen("/usr/people/paulc/xvg/cmgui/catoon.px", "r")) {
    fclose(fp);
    LogoName = "/usr/people/paulc/xvg/cmgui/catoon.px";
  }
  else if (fp = fopen("/usr/people/paul/xvg/cmgui/catoon.px", "r")) {
    fclose(fp);
    LogoName = "/usr/people/paul/xvg/cmgui/catoon.px";
  }
  else if (fp = fopen("/xlv1/eng_sci/charette/xvg/catoon.px", "r")) {
    fclose(fp);
    LogoName = "/xlv1/eng_sci/charette/xvg/catoon.px";
  }
  else {
    LogoName ="catoon.px";
  }
#else
  LogoName ="/u/paul/xvg/cmgui/catoon.px";
#endif
  MovieFileNames = NULL;
  MovieFileName = NULL;
  PPortOutVal = 0;
  RawPixelsX =NULL;
  ProcessedPixelsX =NULL;
  VCAAToGrayLevelLUT565 =NULL;
  VCAAToGrayLevelLUT664 =NULL;
  VCAA565ToColorLUT = NULL;
#ifdef SGI
  VCRtty = "/dev/ttyd2";
#else
  VCRtty = "/dev/tty0";
#endif
  VCAA664ToColorLUT =NULL;
  AtanRadiusThreshold = 1.0;
  BSplineKi = NULL;
  BSplineKj = NULL;
  BSplineCij = NULL;
  DoubleBuffer = NULL;
  GcFactor = 1.0;
  Image_pixels_per_m_X = 1.0;
  Image_m_per_pixel_X = 1.0;
  Image_pixels_per_m_Y = 1.0;
  Image_m_per_pixel_Y = 1.0;
  Image_Z_plane = 0.0;
  MaskPixels = NULL;
  PixelSpacing = PIXEL_SPACING;
  PlaneAngle = NULL;
  ProcessedPixels = NULL;
  ProcMean = 0;
  ProcSD = 0;
  ProcMax = 1.0;
  ProcMin = 0.0;
  RadiusImgData = NULL;
  SinImgData = NULL;
  CosImgData = NULL;
  RawMax = 1.0;
  RawMin = 0.0;
  RawPixels = NULL;
  RawMean = 0;
  RawSD = 0;
  RawHVisFrac = 1.0;
  RawVVisFrac = 1.0;
  ProcHVisFrac = 1.0;
  ProcVVisFrac = 1.0;
  residues =NULL;
  ADIndex = 0;
  AllocatedCmapCells = 0;
  BSplineNKi = 0;
  BSplineNKj = 0;
  DecimationFactor = 1;
  FrameX = 0;
  FrameY = 0;
  FrameWidth = 640;
  FrameHeight = 480;
  GcLineWidth = 1;
  MarkerCircleRadius = 0;
  FileSelectionBoxAction = LOADFILEACTION;
  GalvoCycles = 1;
  GLMovieNFrames = 0;
  InitXVGGLGaphics();
  GrabType =GRAYLEVELGRAB;
  GrabbingControl = -1;
  Image_pixel_org_X = 0;
  Image_pixel_org_Y = 0;
  ImgHeight =STARTING_IMAGE_HEIGHT;
  ImageStackTop =0;
  ImgWidth =STARTING_IMAGE_WIDTH;
  MovieFilesN = 0;
  MovieCurrentFile = 0;
  MovieLoopCount = 0;
  MovieLoopDelay = 0;
  LCRState = -1;
  LCR_NoRet = 15500;
  LCR_HalfWaveRet = 3850;
  NMaskPixels = 0;
  MaxScaleVal =HIST_SCALE_MAX;
  MinScaleVal =HIST_SCALE_MIN;
  MovieType = XMOVIE;
  MeshStatesN = 0;
  NContourPoints =0;
  NRegions =0;
  NOverlayCells = 0;
  NKnotPoints =0;
  NOverlayFunctions =0;
  NIPRegisteredOps =0;
  OverlayLevel = 3;
  PromptDialogAction = VCRIMAGESPERSECACTION;
  VCAAFd =0;
  VCRFramesPerImage = 1;
  xhairnum = 0;
  ZoomFactor =1;
  SGIFrameBuffer = NULL;
  SGIRgbFullSizeBuffer = NULL;
  IPOpList =NULL;
  CurrentFENodeGroup = NULL;
  SlicesProjectionsOn = B_FALSE;
  VCAAConnectionOpen = B_FALSE;
  StereoMMPerPixel = -1;
  StereoFocalLength = -1;
  StereoCameraSeparation = -1;
  ChildProcess = B_FALSE;
  LogoFound = B_FALSE;
#if (!defined (CMGUI) || !defined (NO_XVGMAIN))
  all_FE_node_group = NULL;
#endif
#ifdef ALL_INTERFACES
  ADShellCreated = B_FALSE;
  DAShellCreated = B_FALSE;
  AddTwoPI = B_FALSE;
  AddTwoPIOn = B_FALSE;
  DistanceFirstPointDone = B_FALSE;
  DrawClip = B_FALSE;
  DrawClipOn = B_FALSE;
  ClipRegionGenerate = B_FALSE;
  DrawPolygon = B_FALSE;
  DrawPolygonOn = B_FALSE;
  ComputeDistanceOn = B_FALSE;
  PixelPickOn = B_FALSE;
  GalvoControlShellCreated = B_FALSE;
  SubTwoPI = B_FALSE;
  SubTwoPIOn = B_FALSE;
  LCRShellCreated =B_FALSE;
  UnimaInitialized = B_FALSE;
#endif
  for (i = 0; i < IPOPSMAX; i++)
    IPOpCategories[i].name = NULL;
  ClearFFTState();
  Xvg_InitFE();
  InitStereoParameters();
/*  NRC_FFT_Init(); */
}

/***********************************************************************/
/*                                                                     */
/* Build the colormaps, where the first 64 entries are for the server, */
/* the next 64 colors are for usefull colors (red, green, blue, etc)   */
/* and the logo, and the last 128 entries are for the image luts       */
/* (User loaded, gray level or HSV).                                   */
/*                                                                     */
/***********************************************************************/
void init_state(Widget MainInterfaceWidget,
		int Argc, char **Argv)
{
  XColor color, *TmpLut, unused;
  FILE *fp;
  int i, rc, bg, return_code, cnt;
  char *LogoPixels, **FontList;
  unsigned long PMasks[128];
  boolean_t LoadFileRc, FontsLoaded;

  /* trap control-C signals */
  XvgTrapSignal();
  DEBUGS("init_state(): Installed ctrl-C trap");

  /* init the image stack */
  XvgImgStack_Init();

  /* register the image processing function */
  RegisterIPOps();
  DEBUGS("init_state(): Registered IPOps");

  /* parse input arguments */
  CurrentCells = GreyLevelCells;
  parse_args(Argc-1, (char **) &Argv[1]);
  if (VerboseFlag)
    printf("init_state(): Parsed input args\n");
  
  /* clear the parallel port */
#ifdef PPORT_CODE
  ClearPPort();
#else
#ifdef ALL_INTERFACES
  if (InteractiveFlag)
    XtSetSensitive(menu15, B_FALSE);
#endif
#endif

  /* create dialog interfaces */
  if (InteractiveFlag) {
    fileSelectionBoxDialog = create_fileSelectionBoxDialog();
    InfoDialog = create_InfoDialog();
    InputDialog = create_InputDialog(XvgShell);
#ifndef CMGUI
    errorDialog = create_errorDialog();
#endif
    promptDialog = create_promptDialog();
    ScrolledWindowDialog = create_ScrolledWindowDialog(XvgShell);
#ifdef ALL_INTERFACES
    VCAADialog = create_VCAADialog();
#endif
    if (VerboseFlag)
      printf("init_state(): Created dialog interfaces\n");
  }

  if (InteractiveFlag || DisplayOnlyFlag) {
    /* get default color map */
    cmap = XDefaultColormap(UxDisplay, UxScreen);
    if (VerboseFlag)
      printf("init_state(): Created default color rmap\n");
    
    /* load small font */
    FontList = XListFonts(UxDisplay, "*adobe*-8-*-75-75*", 1, &cnt);
    if (cnt == 0) {
      FontList = XListFonts(UxDisplay, "*adobe*", 1, &cnt);
      ErrorMsg("init_state() : Could not load small font.");
    }

    /* load fonts */
    FontsLoaded = B_TRUE;
    if (((LabelFont = XLoadQueryFont(UxDisplay, "-adobe-helvetica-medium-r-normal--14-100-100-100-p-76-iso8859-1")) == NULL) ||
	((OverlayFont = XLoadQueryFont(UxDisplay, "-adobe-helvetica-bold-r-normal--25-180-100-100-p-138-iso8859-1")) == NULL) ||
	((SmallFont = XLoadQueryFont(UxDisplay, FontList[0])) == NULL))
      FontsLoaded = B_FALSE;
    if (VerboseFlag)
      printf("init_state() : Loaded label and overlay fonts...\n");
    XSync(UxDisplay, B_FALSE);

    /* allocate usefull colors */
    XAllocNamedColor(UxDisplay, XDefaultColormap(UxDisplay, UxScreen),
		     "black", &color, &unused);
    black = color.pixel;
    XAllocNamedColor(UxDisplay, XDefaultColormap(UxDisplay, UxScreen),
		     "white", &color, &unused);
    white = color.pixel;

#ifdef HEXCOLORS
    XAllocNamedColor(UxDisplay, XDefaultColormap(UxDisplay, UxScreen),
		     "#ff000f", &color, &unused);
#else
    if (XAllocNamedColor(UxDisplay, XDefaultColormap(UxDisplay, UxScreen),
			 "red", &color, &unused) == 0) {
      ErrorMsg("init_state() : Error allocating \"red\" entry in LUT");
    }
#endif
    red = color.pixel;
    CCTable[color.pixel] = NOverlayCells;
    OverlayCells[NOverlayCells++] = color;

#ifdef HEXCOLORS
    XAllocNamedColor(UxDisplay, XDefaultColormap(UxDisplay, UxScreen),
		     "#00ff00", &color, &unused);
#else
    if (XAllocNamedColor(UxDisplay, XDefaultColormap(UxDisplay, UxScreen),
			 "green", &color, &unused) == 0) {
      ErrorMsg("init_state() : Error allocating \"green\" entry in LUT");
    }
#endif
    green = color.pixel;
    CCTable[color.pixel] = NOverlayCells;
    OverlayCells[NOverlayCells++] = color;

#ifdef HEXCOLORS
    XAllocNamedColor(UxDisplay, XDefaultColormap(UxDisplay, UxScreen),
		     "#0000ff", &color, &unused);
#else
    if (XAllocNamedColor(UxDisplay, XDefaultColormap(UxDisplay, UxScreen),
			 "blue", &color, &unused) == 0) {
      ErrorMsg("init_state() : Error allocating \"blue\" entry in LUT");
    }
#endif
    blue = color.pixel;
    CCTable[color.pixel] = NOverlayCells;
    OverlayCells[NOverlayCells++] = color;

#ifdef HEXCOLORS
    XAllocNamedColor(UxDisplay, XDefaultColormap(UxDisplay, UxScreen),
		     "#00ffff", &color, &unused);
#else
    if (XAllocNamedColor(UxDisplay, XDefaultColormap(UxDisplay, UxScreen),
			 "cyan", &color, &unused) == 0) {
      ErrorMsg("init_state() : Error allocating \"cyan\" entry in LUT");
    }
#endif
    cyan = color.pixel;
    CCTable[color.pixel] = NOverlayCells;
    OverlayCells[NOverlayCells++] = color;

#ifdef HEXCOLORS
    XAllocNamedColor(UxDisplay, XDefaultColormap(UxDisplay, UxScreen),
		     "#ff00ff", &color, &unused);
#else
    if (XAllocNamedColor(UxDisplay, XDefaultColormap(UxDisplay, UxScreen),
			 "magenta", &color, &unused) == 0) {
      ErrorMsg("init_state() : Error allocating \"magenta\" entry in LUT");
    }
#endif
    magenta = color.pixel;
    CCTable[color.pixel] = NOverlayCells;
    OverlayCells[NOverlayCells++] = color;

#ifdef HEXCOLORS
    XAllocNamedColor(UxDisplay, XDefaultColormap(UxDisplay, UxScreen),
		     "#ffff0f", &color, &unused);
#else
    if (XAllocNamedColor(UxDisplay, XDefaultColormap(UxDisplay, UxScreen),
			 "yellow", &color, &unused) == 0) {
      ErrorMsg("init_state() : Error allocating \"yellow\" entry in LUT");
    }
#endif
    yellow = color.pixel;
    CCTable[color.pixel] = NOverlayCells;
    OverlayCells[NOverlayCells++] = color;

#ifdef ALL_INTERFACES
    XAllocNamedColor(UxDisplay, XDefaultColormap(UxDisplay, UxScreen),
		     "BlueViolet", &color, &unused);
    CCTable[color.pixel] = NOverlayCells;
    OverlayCells[NOverlayCells++] = color;

    XAllocNamedColor(UxDisplay, XDefaultColormap(UxDisplay, UxScreen),
		     "DarkOliveGreen", &color, &unused);
    CCTable[color.pixel] = NOverlayCells;
    OverlayCells[NOverlayCells++] = color;

    XAllocNamedColor(UxDisplay, XDefaultColormap(UxDisplay, UxScreen),
		     "IndianRed", &color, &unused);
    CCTable[color.pixel] = NOverlayCells;
    OverlayCells[NOverlayCells++] = color;

    XAllocNamedColor(UxDisplay, XDefaultColormap(UxDisplay, UxScreen),
		     "HotPink", &color, &unused);
    CCTable[color.pixel] = NOverlayCells;
    OverlayCells[NOverlayCells++] = color;

    XAllocNamedColor(UxDisplay, XDefaultColormap(UxDisplay, UxScreen),
		     "DeepSkyBlue4", &color, &unused);
    CCTable[color.pixel] = NOverlayCells;
    OverlayCells[NOverlayCells++] = color;
#endif
    if (VerboseFlag)
      printf("init_state(): Alloctated standard colors\n");
  }
  
#ifndef CMGUI
  /* load logo image to fetch the required colors and the size */
  if (InteractiveFlag) {
    XtVaGetValues(XvgShell, XmNbackground, &bg, NULL);
    if ((LogoPixels = LoadPxData(LogoName, cmap, bg, B_TRUE, NULL, &LogoWidth,
				 &LogoHeight)) == NULL) {
      sprintf(cbuf, "Could not find logo image file %s", LogoName);
      ErrorMsg(cbuf);
    }
    else
      LogoFound = B_TRUE;
    if (VerboseFlag)
      printf("init_state(): Loaded logo image\n");
  }
#endif
  
  if (InteractiveFlag || DisplayOnlyFlag) {
    /* internal check */
    if ((NPAGEDLUTCELLS + NOverlayCells) > 255) {
      printf("NPAGEDLUTCELLS + NOverlayCells > 255 in init_state()\n");
      exit(EXIT_FAILURE);
    }
    
    /* create the color cell arrays */
    if (PagedCmap) {
      if (XAllocColorCells(UxDisplay, cmap, B_FALSE, PMasks, (unsigned int) 0,
      	LutCells, (unsigned int) NPAGEDLUTCELLS) == 0) {
	ErrorMsg("Warning : Insufficient colormap entries available for swapped colormaps");
	PagedCmap = B_FALSE;
	DEBUGS("init_state(): Allocation of LUT color cells failed");
      }
      else {
	DEBUGS("init_state(): Allocated LUT color cells");
      }
    }
    else {
      DEBUGS("init_state(): Hard non-paged cmap");
    }
    
    LoadGreyLevelCells(0);
    LoadGammaGreyLevelCells();
    LoadBRYGMBVWCells();
    LoadUserCells();
    if ((InteractiveFlag) && (PagedCmap)) {
      LUTShell = create_LUTShell();
      TmpLut = CurrentCells;
      UpdateColorLUT();
      CurrentCells = TmpLut;
      if (VerboseFlag)
	printf("init_state(): Initialized swapped color maps\n");
    }
    
    /* install the gray level cmap as the startup else allocate it */
    if (PagedCmap) {
      XStoreColors(UxDisplay, cmap, CurrentCells, NPAGEDLUTCELLS);
      if (VerboseFlag)
	printf("init_state(): Stored gray level cmap into LUT\n");
    }
    else {
      i = 0;
      do 
	{
	  color = CurrentCells[i];
	  return_code = XAllocColor(UxDisplay, cmap, &color);
	  CurrentCells[i] = color;
	  i++;
	} while ((i < 128) && (return_code));
      if (return_code == 0)
	{
	  sprintf(cbuf, "init_state() : Failed allocating colors at index %d (close other applications which use many colors)",
		  i-1);
	  ErrorMsg(cbuf);
	}
      if (VerboseFlag)
	printf("init_state(): Allocated gray level cmap\n");
    }
    
    /* create white/black graphics context for overlays with large font */
    xgc.foreground = white;
    xgc.background = black;
    xgc.function = GXcopy;
    xgc.line_width = GcLineWidth;
    if (FontsLoaded)
      xgc.font = OverlayFont->fid;
    gc_Overlay = (FontsLoaded ?
		  XCreateGC(UxDisplay, RootWindow(UxDisplay, UxScreen),
			    GCFunction | GCForeground | GCBackground | GCFont
			    | GCLineWidth, &xgc)
		  : XCreateGC(UxDisplay, RootWindow(UxDisplay, UxScreen),
			      GCFunction | GCForeground | GCBackground
			      | GCLineWidth, &xgc));
    if (VerboseFlag)
      printf("init_state(): gc_Overlay created\n");
    
    /* create white/black graphics context */
    if (FontsLoaded)
      xgc.font = LabelFont->fid;
    gc = (FontsLoaded ?
	  XCreateGC(UxDisplay, RootWindow(UxDisplay, UxScreen),
		    GCFunction | GCForeground | GCBackground | GCFont
		    | GCLineWidth, &xgc)
	  : XCreateGC(UxDisplay, RootWindow(UxDisplay, UxScreen),
		      GCFunction | GCForeground | GCBackground
		      | GCLineWidth, &xgc));
    if (VerboseFlag)
      printf("init_state(): gc created\n");
    
    /* create xor graphics context */
    xgc.foreground = 0xffff;
    xgc.function = GXxor;
    gc_xor = XCreateGC(UxDisplay, RootWindow(UxDisplay, UxScreen),
		       GCFunction | GCForeground, &xgc);
    if (VerboseFlag)
      printf("init_state(): gc_xor created\n");
    
    /* create black/white graphics context */
    xgc.foreground = black;
    xgc.background = white;
    xgc.cap_style = CapRound;
    xgc.function = GXcopy;
    xgc.line_width = GcLineWidth;
    gc_BlackWhite = (FontsLoaded ?
		     XCreateGC(UxDisplay, RootWindow(UxDisplay, UxScreen),
			       GCFunction | GCForeground | GCBackground
			       | GCFont | GCLineWidth | GCCapStyle, &xgc)
		     : XCreateGC(UxDisplay, RootWindow(UxDisplay, UxScreen),
				 GCFunction | GCForeground | GCBackground
				 | GCLineWidth | GCCapStyle, &xgc));
    if (VerboseFlag)
      printf("init_state(): gc_BlackWhite created\n");
    
    /* create white/black graphics context */
    xgc.foreground = white;
    xgc.background = black;
    xgc.function = GXcopy;
    xgc.line_width = GcLineWidth;
    gc_WhiteBlack = (FontsLoaded ?
		     XCreateGC(UxDisplay, RootWindow(UxDisplay, UxScreen),
			       GCFunction | GCForeground | GCBackground
			       | GCLineWidth, &xgc)
		     : XCreateGC(UxDisplay, RootWindow(UxDisplay, UxScreen),
				 GCFunction | GCForeground | GCBackground
				 | GCLineWidth, &xgc));
    if (VerboseFlag)
      printf("init_state(): gc_WhiteBlack created\n");
    
    /* create blue/black graphics context */
    if (Monochrome)
      xgc.foreground = white;
    else
      xgc.foreground = blue;
    xgc.background = black;
    xgc.function = GXcopy;
    xgc.line_width = GcLineWidth;
    gc_BlueBlack = (FontsLoaded ?
		    XCreateGC(UxDisplay, RootWindow(UxDisplay, UxScreen),
			      GCFunction | GCForeground | GCBackground
			      | GCFont | GCLineWidth, &xgc)
		    : XCreateGC(UxDisplay, RootWindow(UxDisplay, UxScreen),
				GCFunction | GCForeground | GCBackground
				| GCLineWidth, &xgc));
    if (VerboseFlag)
      printf("init_state(): gc_BlueBlack created\n");
    
    /* create red/black graphics context */
    if (Monochrome)
      xgc.foreground = white;
    else
      xgc.foreground = red;
    xgc.background = black;
    xgc.function = GXcopy;
    gc_RedBlack = (FontsLoaded ?
		   XCreateGC(UxDisplay, RootWindow(UxDisplay, UxScreen),
			     GCFunction | GCForeground | GCBackground | GCFont
			     | GCLineWidth, &xgc)
		   : XCreateGC(UxDisplay, RootWindow(UxDisplay, UxScreen),
			       GCFunction | GCForeground | GCBackground
			       | GCLineWidth, &xgc));
    if (VerboseFlag)
      printf("init_state(): gc_RedBlack created\n");
    
    /* create Green/black graphics context */
    if (Monochrome)
      xgc.foreground = white;
    else
      xgc.foreground = green;
    xgc.background = black;
    xgc.function = GXcopy;
    gc_GreenBlack = (FontsLoaded ?
		     XCreateGC(UxDisplay, RootWindow(UxDisplay, UxScreen),
			       GCFunction | GCForeground | GCBackground
			       | GCFont | GCLineWidth, &xgc)
		     : XCreateGC(UxDisplay, RootWindow(UxDisplay, UxScreen),
				 GCFunction | GCForeground | GCBackground
				 | GCLineWidth, &xgc));
    if (VerboseFlag)
      printf("init_state(): gc_GreenBlack created\n");
    
    /* create yellow/black graphics context */
    if (Monochrome)
      xgc.foreground = white;
    else
      xgc.foreground = yellow;
    xgc.background = black;
    xgc.function = GXcopy;
#if !defined (CMGUI)
/*???DB.  Has to match with gc_RedBlack */
    if (FontsLoaded)
      xgc.font = SmallFont->fid;
#endif /* !defined (CMGUI) */
    gc_YellowBlack = (FontsLoaded ?
		      XCreateGC(UxDisplay, RootWindow(UxDisplay, UxScreen),
				GCFunction | GCForeground | GCBackground
				| GCFont | GCLineWidth, &xgc)
		      : XCreateGC(UxDisplay, RootWindow(UxDisplay, UxScreen),
				  GCFunction | GCForeground | GCBackground
				  | GCLineWidth, &xgc));
    if (VerboseFlag)
      printf("init_state(): gc_YellowBlack created\n");
    
    /* create general usage gc */
    if (Monochrome)
      xgc.foreground = white;
    else
      xgc.foreground = green;
    xgc.background = black;
    gc_Scratch = XCreateGC(UxDisplay, RootWindow(UxDisplay, UxScreen),
			   GCForeground | GCBackground | GCLineWidth, &xgc);
    if (VerboseFlag)
      printf("init_state(): gc_Scratch created\n");
    
    /* create graphics context with 0 as a foreground color */
    xgc.foreground = 0;
    xgc.function = GXcopy;
    gc_Null = XCreateGC(UxDisplay, RootWindow(UxDisplay, UxScreen),
			GCFunction | GCForeground, &xgc);
    if (VerboseFlag)
      printf("init_state(): gc_Null created\n");
    
    /* create cursors */
    CreateCursors();
    if (VerboseFlag)
      printf("init_state(): Cursors created\n");
  }
    
  /* popup the main interface if required */
  if (InteractiveFlag) {
    /* if no paged cmap, desensitize options menus */
    if (PagedCmap == B_FALSE) {
      XtSetSensitive(LUTShellTBG, B_FALSE);
      XtSetSensitive(LUTMenuOMW, B_FALSE);
    }
    
    /* popup main interface */
    UxPopupInterface(MainInterfaceWidget, no_grab);
    if (VerboseFlag)
      printf("init_state(): Main interface popped up\n");

#ifndef CMGUI
    /* build and display the logo image */
    if (LogoFound) {
      LogoImage = XCreateImage(UxDisplay, DefaultVisual(UxDisplay, UxScreen),
			       8, ZPixmap, 0, LogoPixels, LogoWidth,
			       LogoHeight, 8, 0);
      XPutImage(UxDisplay, XtWindow((Widget) LogoDAW),
		gc, LogoImage, 0,
		0, 0, 0, LogoWidth, LogoHeight);
    if (VerboseFlag)
      printf("init_state(): Logo image built\n");
    }
#endif

    /* write default size statistics */
    sprintf(cbuf, "Size: %d x %d", ImgHeight, ImgWidth);
    UxPutLabelString(SizeLG, cbuf);
  }

  /* if a filename was specified on the command line, open it */
  LoadFileRc = LoadFile(InFileName);

  /* exit if in non-interactive mode and no file was loaded */
  if ((!InteractiveFlag && !LoadFileRc) || (DisplayOnlyFlag && !LoadFileRc)) {
    printf("init_state() : xvg execution aborted\n");
    exit(EXIT_FAILURE);
  }

  /* popup raw image window only */
  if (DisplayOnlyFlag) {
    UxPopupInterface(RawImageShell, no_grab);
    UpdateRawWinParms();
  }

  /* execute IPOp linked list if in nointeractive mode */
  if (!InteractiveFlag && !DisplayOnlyFlag) {
    InitProcBuffers();
    ExecuteIPOpList();
    exit(EXIT_SUCCESS);
  }
  
  /* flag XVG up and running */
  XvgUp = B_TRUE;
  if (VerboseFlag)
    printf("init_state() done.\n");
}


void DrawRedPoint(int x, int y)
{
  XDrawPoint(UxDisplay, XtWindow(ProcessedImageDrawingArea),
	     gc_RedBlack, x, y);
  XSync(UxDisplay, B_FALSE);
}

void DrawBluePoint(int x, int y)
{
  XDrawPoint(UxDisplay, XtWindow(ProcessedImageDrawingArea),
	     gc_BlueBlack, x, y);
  XSync(UxDisplay, B_FALSE);
}

void DrawLine(int xs, int y, int xe)
{
  XDrawLine(UxDisplay, XtWindow(ProcessedImageDrawingArea),
	     gc_RedBlack, xs, y, xe, y);
  XSync(UxDisplay, B_FALSE);
}


/* Load IPOp list from a file */
#define IPOPSINMAX 1000
void LoadIPOps(char *fname)
{
  char *InParms[IPOPSINMAX], inbuf[256];
  FILE *fp;
  int i, j;
  
  /* open the file */
  if ((fp = fopen(fname, "r")) == NULL) {
    sprintf(cbuf, "Could not open the file %s in LoadIPOps()", fname);
    ErrorMsg(cbuf);
    return;
  }

  /* read the data from the file */
  for (i = 0; (fscanf(fp, "%s", inbuf) != EOF) && (i < IPOPSINMAX); i++) {
    InParms[i] = (char *) XvgMM_Alloc(NULL, strlen(inbuf)+1);
    sprintf(InParms[i], "%s", inbuf);
  }
  if (i >= IPOPSINMAX) {
    ErrorMsg("Maximum input length execeeded in LoadIPOps()");
    return;
  }

  /* parse arguments and load into the IPOps list */
  sprintf(inbuf, "%s", InFileName);
  parse_args(i, InParms);
  InteractiveFlag = B_TRUE;
  
  /* load any input file if required */
  if (strcmp(inbuf, InFileName) != 0)
    LoadFile(InFileName);

  /* free storage */
  for (j = 0; j < i; j++)
    XvgMM_Free(InParms[j]);

  /* close the file */
  fclose(fp);
}

void SaveIPOps(char *fname)
{
  IPOpListItem *p;
  FILE *fp;
  DOUBLE_ARRAY *da;
  int i, j, k;
  
  /* open the file */
  if ((fp = fopen(fname, "w")) == NULL) {
    sprintf(cbuf, "Could not open the file %s in SaveIPOps()", fname);
    ErrorMsg(cbuf);
    return;
  }

  /* loop for all IPOps in the list */
  for (p = IPOpList; p != NULL; p = p->next) {
    
    /* find the IPOp in the linked list */
    for (i = 0; (i < NIPRegisteredOps) && (IPRegisteredOpList[i].f != p->f);
	 i++);

    /* consistency check */
    if (i == NIPRegisteredOps) {
      ErrorMsg("Could not find IPOp match in SaveIPOp()");
      return;
    }

    /* write the IPOp name to the file */
    fprintf(fp, "%s ", IPRegisteredOpList[i].OpName);
    
    /* loop to write the parameter values to the file */
    for (j = 0; j < IPRegisteredOpList[i].NParms; j++) {
      switch (IPRegisteredOpList[i].ParmDataTypes[j]) {
      case INTEGER_DT:
	fprintf(fp, "%d ", *((int *)((p->ParmPtrs)[j])));
	break;
      case DOUBLE_DT:
	fprintf(fp, "%e ", *((double *)((p->ParmPtrs)[j])));
	break;
      case STRING_DT:
	fprintf(fp, "%s ", (p->ParmPtrs)[j]);
	break;
      case DOUBLE_ARRAY_DT:
	da = (DOUBLE_ARRAY *) ((p->ParmPtrs)[j]);
	fprintf(fp, "%d ", da->n);
	for (k = 0; k < da->n; k++)
	  fprintf(fp, "%e ", da->data[k]);
	break;
      default:
	break;
      }
    }
    
    /* write repetions count if required */
    if (IPRegisteredOpList[i].RepsAllowed)
      fprintf(fp, "%d ", p->reps);
  }
  fprintf(fp,"\n");
  fclose(fp);
}

/* dump an image to the VCA and to the VCR if required */
extern int RawWinHOffset, RawWinVOffset, RawWinWidth, RawWinHeight,
  ProcessedWinHOffset, ProcessedWinVOffset, ProcessedWinWidth,
  ProcessedWinHeight;
boolean_t DumpImage(void)
{
  XImage *image;
  XColor cells[256];
  int i;

#ifdef ALL_INTERFACES
  if (VCAWriteOn == B_FALSE)
    return(B_FALSE);

  /* enable hourglass cursor */
  HGCursor(B_TRUE);

  /* fetch drawing area contents */	
  if (IsTrue(UxGetSet(ProcessedImageTBG))) {
    XRaiseWindow(UxDisplay, XtWindow(ProcessedImageShell));
    image = XGetImage(UxDisplay,
		      XtWindow(ProcessedImageDrawingArea),
		      ProcessedWinHOffset, ProcessedWinVOffset,
		      ProcessedWinWidth, ProcessedWinHeight,
		      (unsigned long) 0xffffffff, ZPixmap);
  }
  else if (IsTrue(UxGetSet(RawImageTBG))) {
    XRaiseWindow(UxDisplay, XtWindow(RawImageShell));
    image = XGetImage(UxDisplay, XtWindow(RawImageDrawingArea),
		      RawWinHOffset, RawWinVOffset,
		      RawWinWidth, RawWinHeight,
		      (unsigned long) 0xffffffff, ZPixmap);
  }
  else if (IsTrue(UxGetSet(GLShellTBG)) == B_FALSE) {
    HGCursor(B_FALSE);
    ErrorMsg("No image window popped up for writing to the VCAA!");
    return(B_FALSE);
  }

  /* X Windows dumps */
  if (IsTrue(UxGetSet(ProcessedImageTBG)) || IsTrue(UxGetSet(RawImageTBG))) {
    /* fetch the current color cell definitions */
    for (i = 0; i < 256; i++)
      cells[i].pixel = i;
    XQueryColors(UxDisplay, cmap, cells, 256);
    
    /* write the data to the VCA */
    VCAWriteData(image, cells);
    
    /* record the frame if required */
    if (VCRRecordOn)
      if (VCRRecord(VCRFramesPerImage) == B_FALSE) {
	XDestroyImage(image);
	HGCursor(B_FALSE);
	return(B_FALSE);
      }
    
    /* destroy the image */
    XDestroyImage(image);
  }
  /* GL Window dump */
  else {
    /* dump the image to the VCAA */
    WriteGLImageToVCAA();
    /* record the frame if required */
    if (VCRRecordOn)
      if (VCRRecord(VCRFramesPerImage) == B_FALSE) {
	XDestroyImage(image);
	return(B_FALSE);
      }
  }
  
  /* disable hourglass cursor */
  HGCursor(B_FALSE);
  return(B_TRUE);
#else
  return(B_FALSE);
#endif
}
  

void RotateImage(void)
{
  int tmp;
  char buffer[256];

  HGCursor(B_TRUE);
  Rotate(RawPixels, ImgHeight, ImgWidth);
  tmp = ImgHeight;
  ImgHeight = ImgWidth;
  ImgWidth = tmp;
  sprintf(buffer, "Size: %d x %d", ImgWidth, ImgHeight);
  UxPutLabelString(SizeLG, buffer);
  ResizeRawImageWidgets(B_FALSE);
  CreateRawImage();
  if (ProcessedImageShellCreated) {
    ResizeProcessedImageWidgets(B_FALSE);
    InitProcBuffers();
    MoveDoubles(RawPixels, ProcessedPixels, ImgHeight*ImgWidth);
    CreateProcImage();
  }
  HGCursor(B_FALSE);
}

void TransposeImage(void)
{
  int tmp;
  char buffer[256];

  HGCursor(B_TRUE);
  Transpose(RawPixels, ImgHeight, ImgWidth);
  tmp = ImgHeight;
  ImgHeight = ImgWidth;
  ImgWidth = tmp;
  sprintf(buffer, "Size: %d x %d", ImgWidth, ImgHeight);
  UxPutLabelString(SizeLG, buffer);
  ResizeRawImageWidgets(B_FALSE);
  CreateRawImage();
  if (ProcessedImageShellCreated) {
    ResizeProcessedImageWidgets(B_FALSE);
    InitProcBuffers();
    MoveDoubles(RawPixels, ProcessedPixels, ImgHeight*ImgWidth);
    CreateProcImage();
  }
  HGCursor(B_FALSE);
}

void PeekNoInteractive(int ac, char **av)
{
  int i, j;
  
  /* look for "-nointeractive flag" */
  for (i = 0; i < ac; i++)
    /* flag found, suppress it from the list, parse the args and execute */
    if (strcmp(av[i], "-nointeractive") == 0) {
      InteractiveFlag = B_FALSE;
      PagedCmap = B_FALSE;
      for (j = i; j < ac-1; j++)
	av[j] = av[j+1];
      init_state((Widget) 0, ac-1, av);
      exit(EXIT_SUCCESS);
    }
}


