/****************************************************************************
*
*    File:      IPOperationsShellAux.c
*    Author:    Paul Charette
*    Modified:  26 Nov 1994
*
*    Purpose:   various initialisation & utility routines
*             AddNewOpAfterCallback()
*             draw_mesg()
*             ExecuteIPOpList()
*             LoadFTRealImgMenu()
*             LoadFTImImgMenu()
*             LoadFTRealImg2Menu()
*             LoadFTImImg2Menu()
*             MallocScratchDouble()
*             RegisterIPOp()
*
*****************************************************************************/
/* load FFT Real buffer 1 image, push current image onto the stack */
static void LoadFTRealImgMenu(void)
{
  int i;

  /* check that the buffer exists */
  if (FFTRealBuffer == NULL) {
    ErrorMsg("LoadFTRealImgMenu() : Buffer is empty");
    return;
  }

  /* push current image on the stack */
  HGCursor(B_TRUE);
  if (XvgImgStack_Push(ProcessedPixels, ImgHeight, ImgWidth) == B_FALSE) {
    ErrorMsg("LoadFTRealImgMenu() : Failed pushing current image");
    HGCursor(B_FALSE);
    return;
  }

  /* reset current buffer size */
  if ((ImgHeight != FFTHeight) || (ImgWidth != FFTWidth)) {
    ImgHeight = FFTHeight;
    ImgWidth = FFTWidth;
    InitProcBuffers();
  }

  /* load the data into the current buffer */
  for (i = 0; i < ImgHeight*ImgWidth; i++)
    ProcessedPixels[i] = FFTRealBuffer[i];
  DynamicRange(ProcessedPixels, ImgHeight*ImgWidth, &ProcMax, &ProcMin);
  ResizeProcessedImageWidgets(B_FALSE);
  CreateProcImage();
  HGCursor(B_FALSE);
}

/* load FFT Imaginary buffer 1 image, push current image onto the stack */
static void LoadFTImImgMenu(void)
{
  int i;

  /* check that the buffer exists */
  if (FFTImBuffer == NULL) {
    ErrorMsg("LoadFTImImgMenu() : Buffer is empty");
    return;
  }

  /* push current image on the stack */
  HGCursor(B_TRUE);
  if (XvgImgStack_Push(ProcessedPixels, ImgHeight, ImgWidth) == B_FALSE) {
    ErrorMsg("LoadFTImImgMenu() : Failed pushing current image");
    HGCursor(B_FALSE);
    return;
  }

 /* reset current buffer size */
  if ((ImgHeight != FFTHeight) || (ImgWidth != FFTWidth)) {
    ImgHeight = FFTHeight;
    ImgWidth = FFTWidth;
    InitProcBuffers();
  }

   /* load the data into the current buffer */
  for (i = 0; i < ImgHeight*ImgWidth; i++)
    ProcessedPixels[i] = FFTImBuffer[i];
  DynamicRange(ProcessedPixels, ImgHeight*ImgWidth, &ProcMax, &ProcMin);
  CreateProcImage();
  HGCursor(B_FALSE);
}

/* load FFT Real buffer  image, push current image onto the stack */
static void LoadFTRealImg2Menu(void)
{
  int i;

  /* check that the buffer exists */
  if (FFTRealBuffer2 == NULL) {
    ErrorMsg("LoadFTRealImg2Menu() : Buffer is empty");
    return;
  }

  /* push current image on the stack */
  HGCursor(B_TRUE);
  if (XvgImgStack_Push(ProcessedPixels, ImgHeight, ImgWidth) == B_FALSE) {
    ErrorMsg("LoadFTRealImg2Menu() : Failed pushing current image");
    HGCursor(B_FALSE);
    return;
  }

 /* reset current buffer size */
  if ((ImgHeight != FFTHeight) || (ImgWidth != FFTWidth)) {
    ImgHeight = FFTHeight;
    ImgWidth = FFTWidth;
    InitProcBuffers();
  }

   /* load the data into the current buffer */
  for (i = 0; i < ImgHeight*ImgWidth; i++)
    ProcessedPixels[i] = FFTRealBuffer2[i];
  DynamicRange(ProcessedPixels, ImgHeight*ImgWidth, &ProcMax, &ProcMin);
  CreateProcImage();
  HGCursor(B_FALSE);
}

/* load FFT Imaginary buffer 2 image, push current image onto the stack */
static void LoadFTImImg2Menu(void)
{
  int i;

  /* check that the buffer exists */
  if (FFTImBuffer2 == NULL) {
    ErrorMsg("LoadFTImImg2Menu() : Buffer is empty");
    return;
  }

  /* push current image on the stack */
  HGCursor(B_TRUE);
  if (XvgImgStack_Push(ProcessedPixels, ImgHeight, ImgWidth) == B_FALSE) {
    ErrorMsg("LoadFTImImg2Menu() : Failed pushing current image");
    HGCursor(B_FALSE);
    return;
  }

 /* reset current buffer size */
  if ((ImgHeight != FFTHeight) || (ImgWidth != FFTWidth)) {
    ImgHeight = FFTHeight;
    ImgWidth = FFTWidth;
    InitProcBuffers();
  }

   /* load the data into the current buffer */
  for (i = 0; i < ImgHeight*ImgWidth; i++)
    ProcessedPixels[i] = FFTImBuffer2[i];
  DynamicRange(ProcessedPixels, ImgHeight*ImgWidth, &ProcMax, &ProcMin);
  CreateProcImage();
  HGCursor(B_FALSE);
}

/* load radius 2ICos image,  push current image onto the stack */
void LoadCosImgMenu(void)
{
  int i;

  /* check that a Cos image exists */
  if (CosImgData == NULL) {
    ErrorMsg("LoadCosImgMenu() : No Cos image");
    return;
  }

  /* push current image on the stack */
  HGCursor(B_TRUE);
  if (XvgImgStack_Push(ProcessedPixels, ImgHeight, ImgWidth) == B_FALSE) {
    ErrorMsg("LoadCosImgMenu() : Failed pushing current image");
    HGCursor(B_FALSE);
    return;
  }

  /* load the 2ICos data into the current buffer */
  for (i = 0; i < ImgHeight*ImgWidth; i++)
    ProcessedPixels[i] = CosImgData[i];
  DynamicRange(ProcessedPixels, ImgHeight*ImgWidth, &ProcMax, &ProcMin);
  CreateProcImage();
  HGCursor(B_FALSE);
}

/* load radius 2ISin image,  push current image onto the stack */
void LoadSinImgMenu(void)
{
  int i;

  /* check that Sin image exists */
  if (SinImgData == NULL) {
    ErrorMsg("LoadSinImgMenu() : No Sin image");
    return;
  }

  /* push current image on the stack */
  HGCursor(B_TRUE);
  if (XvgImgStack_Push(ProcessedPixels, ImgHeight, ImgWidth) == B_FALSE) {
    ErrorMsg("LoadSinImgMenu() : Failed pushing current image");
    HGCursor(B_FALSE);
    return;
  }

  /* load the 2ISin data into the current buffer */
  for (i = 0; i < ImgHeight*ImgWidth; i++)
    ProcessedPixels[i] = SinImgData[i];
  DynamicRange(ProcessedPixels, ImgHeight*ImgWidth, &ProcMax, &ProcMin);
  CreateProcImage();
  HGCursor(B_FALSE);
}

boolean_t MallocScratchDouble(void)
{
  if ((ScratchDouble = (double *) XvgMM_Alloc((void *) ScratchDouble,
					      ImgHeight*ImgWidth
					      *sizeof(double)))
      == NULL)
    return(B_FALSE);
  else
    return(B_TRUE);
}

/* load a registered IP function in the editing window         */
/* called from a pulldown menu selection in the editing window */
static void EditIpOp(char *OpName)
{
  int i, j;
  
  /* traverse registered IP OP list to find location of this function */
  for (i = 0;(i < NIPRegisteredOps)
       && (strcmp(OpName, IPRegisteredOpList[i].OpName) != 0); i++);
  if (i == NIPRegisteredOps) {
    sprintf(cbuf, "IP Operation %s is not registered!", OpName);
    ErrorMsg(cbuf);
    return;
  }
  
  /* load the parameters into the current IP structure parms */
  CurrentIPOp.f = IPRegisteredOpList[i].f;
  CurrentIPOp.RepsAllowed = IPRegisteredOpList[i].RepsAllowed;
  CurrentIPOp.EdgeFixFlag = IPRegisteredOpList[i].EdgeFixFlag;
  (CurrentIPOp.ParmDataTypes)[0] = IPRegisteredOpList[i].ParmDataTypes[0];
  (CurrentIPOp.ParmDataTypes)[1] = IPRegisteredOpList[i].ParmDataTypes[1];
  (CurrentIPOp.ParmDataTypes)[2] = IPRegisteredOpList[i].ParmDataTypes[2];
  (CurrentIPOp.ParmDataTypes)[3] = IPRegisteredOpList[i].ParmDataTypes[3];
  
  /* load the parameters into the editing window xgetwidgetswidgets and map */
  UxPutLabelString(FNameLG, OpName);
  UxPutText(RepsTW, "1");
  for (j = 0; j < IPRegisteredOpList[i].NParms; j++) {
    UxMap(ParmLabels[j]);
    UxMap(ParmTexts[j]);
    UxPutLabelString(ParmLabels[j], IPRegisteredOpList[i].ParmNames[j]);
    switch (IPRegisteredOpList[i].ParmDataTypes[j]) {
    case INTEGER_DT:
      (CurrentIPOp.ParmPtrs)[j]
	= (void *) XvgMM_Alloc((void *)((CurrentIPOp.ParmPtrs)[j]),
			       sizeof(int));
      break;
    case DOUBLE_DT:
      (CurrentIPOp.ParmPtrs)[j]
	= (void *) XvgMM_Alloc((void *) ((CurrentIPOp.ParmPtrs)[j]),
			       sizeof(double));
      break;
    case DOUBLE_ARRAY_DT:
      (CurrentIPOp.ParmPtrs)[j]
	= (void *) XvgMM_Alloc((void *) ((CurrentIPOp.ParmPtrs)[j]),
			       sizeof(DOUBLE_ARRAY));
      ((DOUBLE_ARRAY *) ((CurrentIPOp.ParmPtrs)[j]))->data = NULL;
      break;
    case STRING_DT:
      if (((CurrentIPOp.ParmPtrs)[j]
	   = (void *) XvgMM_Alloc((void *) ((CurrentIPOp.ParmPtrs)[j]),
				  INSTRINGSIZEMAX)) == NULL) {
	ErrorMsg("EditIpOp() : Could not allocate space");
	return;
      }
      break;
    default:
      (CurrentIPOp.ParmPtrs)[j] = NULL;
      break;
    }
  }
  
  /* unmap unused fields */
  for (; j < IPOPNPARMS; j++) {
    (CurrentIPOp.ParmPtrs)[j] = NULL;
    UxUnmap(ParmLabels[j]);
    UxUnmap(ParmTexts[j]);
  }
  
  /* map reps swidgets if required */
  if (CurrentIPOp.RepsAllowed) {
    UxMap(RepsLG);
    UxMap(RepsTW);
  }
  else {
    UxUnmap(RepsLG);
    UxUnmap(RepsTW);
  }
}

/* general second level popup menu callback */
static void IPOpEditCallback(Widget widget, char *OpName)
{
  EditIpOp(OpName);
}


/* load IP op into structure and build scrolled list string */
static void LoadIPOp(IPOpListItem *p, char *b)
{
  DOUBLE_ARRAY *da;
  XmString xs;
  int i, j, k, l, nn, num;
  char *s;
  
  /* fill in list element fields */
  XtVaGetValues(FNameLG, XmNlabelString, &xs, NULL);
  XmStringGetLtoR(xs, XmSTRING_DEFAULT_CHARSET, &s);
  sprintf(p->OpName, "%s", s);
  if (s)
    free(s);
  if (VerboseFlag)
    printf("LoadIPOp() : Building CurrentIPOp structure for %s\n", p->OpName);
  p->f = CurrentIPOp.f;
  p->RepsAllowed = CurrentIPOp.RepsAllowed; 
  p->EdgeFixFlag = CurrentIPOp.EdgeFixFlag;
  if (p->RepsAllowed)
    p->reps = atoi(UxGetText(RepsTW));
  else
    p->reps = 1;
  for (i = 0; i < IPOPNPARMS; i++) {
    if ((CurrentIPOp.ParmPtrs)[i] != NULL) {
      XtVaGetValues(ParmLabels[i], XmNlabelString, &xs, NULL);
      XmStringGetLtoR(xs, XmSTRING_DEFAULT_CHARSET, &s);
      sprintf(&((p->ParmNames)[i][0]), "%s", s);
      if (s)
	free(s);
      if ((p->ParmDataTypes)[i] == DOUBLE_ARRAY_DT) {
	XvgMM_Free(((DOUBLE_ARRAY *) ((p->ParmPtrs)[i]))->data);
	((DOUBLE_ARRAY *) ((p->ParmPtrs)[i]))->data = NULL;
      }
      switch ((CurrentIPOp.ParmDataTypes)[i]) {
      case INTEGER_DT:
	(p->ParmDataTypes)[i] = INTEGER_DT;
	(p->ParmPtrs)[i] = (void *) XvgMM_Alloc((void *) ((p->ParmPtrs)[i]),
						sizeof(int));
	*((int *) (p->ParmPtrs)[i]) = atoi(UxGetText(ParmTexts[i])); 
	if (VerboseFlag)
	  printf("  LoadIPOp() : INTEGER parameter : %d...\n",
		 *((int *) (p->ParmPtrs)[i]));
	break;
	
      case DOUBLE_DT:
	(p->ParmDataTypes)[i] = DOUBLE_DT;
	(p->ParmPtrs)[i] = (void *) XvgMM_Alloc((void *) ((p->ParmPtrs)[i]),
						sizeof(double));
	*((double *) (p->ParmPtrs)[i]) = atof(UxGetText(ParmTexts[i])); 
	if (VerboseFlag)
	  printf("  LoadIPOp() : DOUBLE parameter : %f...\n",
		 *((double *) (p->ParmPtrs)[i]));
	break;
	
      case DOUBLE_ARRAY_DT:
	da = NULL;
	if ((da = (DOUBLE_ARRAY *) XvgMM_Alloc((void *) da,
					       sizeof(DOUBLE_ARRAY)))
	    == NULL) {
	  ErrorMsg("LoadIPOp() : Could not allocate storage for DOUBLE_ARRAY struct");
	  return;
	}
	(p->ParmDataTypes)[i] = DOUBLE_ARRAY_DT;
	(p->ParmPtrs)[i] = da;
	
	/* determine number of elements in text string */
	s = (char *) UxGetText(ParmTexts[i]);
	for (num = 0, k = 0; s[k]  != '\0'; num++) {
	  for (; s[k] == ' '; k++);
	  for (; (s[k] != '\0') && (s[k] != ',') && (s[k] != ' '); k++);
	  if (s[k] != '\0')
	    k++;
	}
	if (VerboseFlag)
	  printf("LoadIPOp() : DOUBLE_ARRAY_DT of lenght %d (str: %s)\n",
		 num, s);

	/* init double_array structure fields */
	da->n = num;
        da->data = NULL;
	if ((da->data = (double *) XvgMM_Alloc((void *) (da->data),
					       sizeof(double) * num)) == NULL) {
	  sprintf(cbuf, "LoadIPOp() : Could not alloc storage (%d) for DOUBLE_ARRAY array",
		  num*sizeof(double));
	  ErrorMsg(cbuf);
	  return;
	}

	/* load elements into the array */
	for (nn = 0, k = 0; nn < num; nn++) {
	  for (; s[k] == ' '; k++);
	  for (l = 0, j = k; (s[k] != '\0') && (s[k] != ',') && (s[k] != ' ');
	       k++, l++)
	    cbuf[l] = s[k];
	  cbuf[l] = '\0';
	  (da->data)[nn] = atof(cbuf); 
	}

	if (VerboseFlag)
	  printf("  LoadIPOp() : DOUBLE_ARRAY parameter (%d elements)...\n",
		 num);
	break;
	
      case STRING_DT:
      default:
	(p->ParmDataTypes)[i] = STRING_DT;
	if (((p->ParmPtrs)[i] =
	     (void *) XvgMM_Alloc((void *) ((p->ParmPtrs)[i]),
				  strlen((char *) UxGetText(ParmTexts[i]))+1))
	    == NULL) {
	  ErrorMsg("LoadIPOp() : Could not allocate space");
	  return;
	}
	sprintf((p->ParmPtrs)[i], "%s", UxGetText(ParmTexts[i])); 
	if (VerboseFlag)
	  printf("  LoadIPOp() : STRING parameter : %s...\n",
		 (p->ParmPtrs)[i]);
	break;
      }
    }
    else
      (p->ParmPtrs)[i] = NULL;
  }
  
  /* build the string to insert into the scrolled list */
  sprintf(b, "%s (", p->OpName);
  for (i = 0; (i < IPOPNPARMS) && ((p->ParmPtrs)[i] != NULL); i++) {
    switch ((CurrentIPOp.ParmDataTypes)[i]) {
    case INTEGER_DT:
      if (i == 0)
	sprintf(b, "%s%d", b, atoi(UxGetText(ParmTexts[i])));
      else
	sprintf(b, "%s, %d", b, atoi(UxGetText(ParmTexts[i])));
      break;
      
    case DOUBLE_DT:
      if (i == 0)
	sprintf(b, "%s%.2e", b, atof(UxGetText(ParmTexts[i]))); 
      else
	sprintf(b, "%s, %.2e", b, atof(UxGetText(ParmTexts[i]))); 
      break;
      
    case DOUBLE_ARRAY_DT:
      da = (DOUBLE_ARRAY *) ((p->ParmPtrs)[i]);
      for (j = 0; j < da->n; j++)
	sprintf(b, "%s, %.4f", b, (da->data)[j]);
      break;
      
    case STRING_DT:
    default:
      if (i == 0)
	sprintf(b, "%s%s", b, UxGetText(ParmTexts[i])); 
      else
	sprintf(b, "%s, %s", b, UxGetText(ParmTexts[i])); 
      break;
    }
  }
  if (p->RepsAllowed) {
    sprintf(b, "%s) x%d", b, p->reps);
    if (VerboseFlag)
      printf("LoadIPOp() : REPS ALLOWED...\n");
  }
  else {
    sprintf(b, "%s)", b);
    if (VerboseFlag)
      printf("LoadIPOp() : NO REPS...\n");
  }
  
  if (VerboseFlag)
    printf("LoadIPOp() : string = %s\n", b);
}

static void ListDeselect(void)
{
  XmListDeselectAllItems(scrolledList1);
}

#define HOSTNAMELENGTH 256
void ExecuteIPOpList(void)
{
  IPOpListItem *p;
  int its, bufindx;
  double *char_bufs[2];
  char hostname[HOSTNAMELENGTH];

  /* check for correct initial state */
  if ((RawPixels == NULL) || (ProcessedPixels == NULL))
    return;
  
  /* fetch start time */
  XvgStartTimer();

  /* set up double buffer system for image processing */
  char_bufs[0] = ProcessedPixels;
  char_bufs[1] = ScratchDouble;
  bufindx = 0;
  
  /* load raw data into first buffer */
  MoveDoubles(RawPixels, char_bufs[bufindx], ImgHeight*ImgWidth);
  
  /* loop to call all image processing functions in the array */
  for (p = IPOpList, Abort=B_FALSE; (p != NULL) && (Abort == B_FALSE);
       p = p->next) {
    
    /* loop for repetitions */
    for (its = 0; (its < p->reps) && (Abort == B_FALSE); its++) {
      (* p->f)(char_bufs[bufindx],
	       char_bufs[(bufindx + 1) & 1], ImgHeight, ImgWidth,
	       p->ParmPtrs);
      if (Abort == B_FALSE)
	bufindx = (bufindx + 1) & 1;
    }
    
    /* fix edges */
    if ((p->EdgeFixFlag) && (Abort == B_FALSE)) {
      DynamicRange(char_bufs[bufindx], ImgHeight*ImgWidth, &ProcMax, &ProcMin);
      printf("Fixing edges with max:%f, min:%f\n", ProcMax, ProcMin);
      FixEdges(char_bufs[bufindx], char_bufs[bufindx], ImgHeight, ImgWidth,
	       p->ParmPtrs, ProcMax, ProcMin);
    }
  }
  
  /* compute and display elapsed time */
  XvgStopTimer();
  if (!InteractiveFlag && !DisplayOnlyFlag) {
    gethostname(hostname, HOSTNAMELENGTH);
    printf("Total execution time on %s %s\n",
	   hostname, ElapsedTime(""));
  }
}

static IPOpListItem *CreateNewIPOp(void)
{
  IPOpListItem *new;
  int i;

  if ((new = (IPOpListItem *) XvgMM_Alloc(NULL, sizeof(IPOpListItem)))
      == NULL) {
    ErrorMsg("CreateNewIPOp() : Could not allocate");
    return(NULL);
  }
  new->f = NULL;
  for (i = 0; i < IPOPNPARMS; i++)
    (new->ParmPtrs)[i] = NULL;
  new->next = NULL;
  new->prev = NULL;
  return(new);
}

void AddNewOpAfterCallback(void)
{
  int *s, cnt, i;
  IPOpListItem *new, *p;
  char b[INSTRINGSIZEMAX];

  /* check to make sure new item is defined */
  if (CurrentIPOp.f == NULL) {
    ErrorMsg("A new IP Op must first be defined in the editing window!\n");
    return;
  }

  /* allocate storage for new list element & initialize fields */
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
#ifdef DEBUG
    printf("AddNewOpAfterCallback() : adding item to the top of the Xmlist\n");
#endif
    XmListAddItem(scrolledList1, XmStringCreateSimple(b), 1);
    XmListSelectPos(scrolledList1, 1, B_FALSE);
  }
  /* else look for a selected item */
  else {
    XmListGetSelectedPos(scrolledList1, &s, &cnt);
#ifdef DEBUG
    printf("AddNewOpAfterCallback(XmListGetSelectedPos()) %d items, pos:%d\n",
	   cnt, *s);
#endif
    /* insert the new element in the linked list after the selected item */
    if ((cnt > 0) && (InteractiveFlag)) {
      /* check to see if more than one item selected */
      if (cnt > 1) {
	sprintf(cbuf,
		"AddNewOpAfterCallback() : More than one (%d) item selected!",
		cnt);
	ErrorMsg(cbuf);
	return;
      }
      /* add item to linked list */
      for (i = 1, p = IPOpList; i < *s; p = (IPOpListItem *) p->next, i++);
      new->next = p->next;
      new->prev = (void *) p;
      if (p->next != NULL)
	((IPOpListItem *) p->next)->prev = (void *) new;
      p->next = new;
      /* add item to scrolled list */
#ifdef DEBUG
    printf("AddNewOpAfterCallback() : adding item in the Xmlist at pos %d\n",
	   *s);
#endif
      XmListDeselectAllItems(scrolledList1);
      XmListAddItem(scrolledList1, XmStringCreateSimple(b),
		    s[cnt-1]+1);
      XmListSelectPos(scrolledList1, s[cnt-1]+1, B_FALSE);
      if (s)
	free(s);
    }
    /* no selected item so insert the new element at the end of the list */
    else {
      /* add item to linked list */
      for (p = IPOpList; p->next != NULL; p = (IPOpListItem *) p->next);
      new->next = NULL;
      new->prev = (void *) p;
      p->next = (void *) new;
      /* add item to scrolled list */
#ifdef DEBUG
    printf("AddNewOpAfterCallback() : adding item to the end of the Xmlist\n");
#endif
      XmListDeselectAllItems(scrolledList1);
      XmListAddItem(scrolledList1, XmStringCreateSimple(b),
		    UxGetItemCount(scrolledList1)+1);
      XmListSelectPos(scrolledList1,
		      UxGetItemCount(scrolledList1)+1, B_FALSE);
    }
  }
#ifdef DEBUG
    printf("AddNewOpAfterCallback() : Done\n");
#endif
}

/* first level popup menu callback */
static void LoadSelectedListItemCallback(Widget widget)
{
  int *s, cnt, i, j;
  IPOpListItem *p;
  char b[INSTRINGSIZEMAX];
  
  /* check to make sure an item is selected */
  XmListGetSelectedPos(scrolledList1, &s, &cnt);
  if (cnt < 1) {
    ErrorMsg("No list item selected!\n");
    return;
  }
  
  /* check to see if more than one item selected */
  if (cnt > 1) {
    ErrorMsg("LoadSelectedListItemCallback() : More than 1 item selected!\n");
    return;
  }
  
  /* fetch the information from the linked list */
  for (i = 1, p = IPOpList; i < *s; p = (IPOpListItem *) p->next, i++);
  CurrentIPOp.f = p->f;
  CurrentIPOp.RepsAllowed = p->RepsAllowed;
  CurrentIPOp.EdgeFixFlag = p->EdgeFixFlag;
  CurrentIPOp.reps = p->reps;
  
  /* unmap all parameters fields */
  for (i = 0; i < IPOPNPARMS; i++) {
    CurrentIPOp.ParmPtrs[i] = NULL;
    UxUnmap(ParmLabels[i]);
    UxUnmap(ParmTexts[i]);
  }
  UxUnmap(RepsLG);
  UxUnmap(RepsTW);
  
  /* load the parameters and map necessary widgets */
  UxPutLabelString(FNameLG, p->OpName);
  for (i = 0; (i < IPOPNPARMS) && ((p->ParmPtrs)[i] != NULL); i++) {
    CurrentIPOp.ParmPtrs[i] = (p->ParmPtrs)[i];
    CurrentIPOp.ParmDataTypes[i] = (p->ParmDataTypes)[i];
    switch (CurrentIPOp.ParmDataTypes[i]) {
    case INTEGER_DT:
      sprintf(b, "%d", *((int *) ((p->ParmPtrs)[i])));
      UxPutText(ParmTexts[i], b);
      UxPutLabelString(ParmLabels[i], &((p->ParmNames)[i][0]));
      break;
      
    case DOUBLE_DT:
      sprintf(b, "%e", *((double *) ((p->ParmPtrs)[i])));
      UxPutText(ParmTexts[i], b);
      UxPutLabelString(ParmLabels[i], &((p->ParmNames)[i][0]));
      break;
      
    case DOUBLE_ARRAY_DT:
      for (j = 0, sprintf(b, "");
	   j < (((DOUBLE_ARRAY *) ((p->ParmPtrs)[i]))->n - 1); j++)
	sprintf(b , "%s%.4f ",
		b, (((DOUBLE_ARRAY *) ((p->ParmPtrs)[i]))->data)[j]);
      sprintf(b , "%s%f",
	      b, (((DOUBLE_ARRAY *) ((p->ParmPtrs)[i]))->data)[j]);
      UxPutText(ParmTexts[i], b);
      UxPutLabelString(ParmLabels[i], &((p->ParmNames)[i][0]));
      break;
      
    case STRING_DT:
    default:
      sprintf(b, "%s", (char *) ((p->ParmPtrs)[i]));
      UxPutText(ParmTexts[i], b);
      UxPutLabelString(ParmLabels[i], &((p->ParmNames)[i][0]));
      break;
    }
    UxMap(ParmLabels[i]);
    UxMap(ParmTexts[i]);
  }
  
  /* map repetitions field if required */
  if (p->RepsAllowed == B_TRUE) {
    sprintf(b, "%d", p->reps);
    UxPutText(RepsTW, b);
    UxMap(RepsLG);
    UxMap(RepsTW);
  }
}

/* interface routine for loading pstepped raw images */
static void LoadPStepImg(void)
{
  extern swidget promptDialog;
  char b[256];

  PromptDialogAction = LOADPSTEPIMAGEACTION;
  UxPutSelectionLabelString(promptDialog, "Image index :");
  UxPutTextString(promptDialog, "0");
  UxPopupInterface(promptDialog, no_grab);
}

/* editing window cascade menu building routine */
static void BuildEditingMenu(void)
{
  Widget submenu, widget, cascade;
  XmString str;
  XColor color, unused;
  Pixel fg;
  Arg args[1];
  int i, j;

  /* create main popup menu */
  XAllocNamedColor(UxDisplay, XDefaultColormap(UxDisplay, UxScreen),
		   "white", &color, &unused);
  fg = color.pixel;
  XtSetArg(args[0], XmNforeground, fg);

  IPOpEditMenu = XmCreatePopupMenu(IPEditingWindowFW, "_popup",
				   args, 1);

  /* create and load main menu items */
  widget = XtVaCreateManagedWidget("Editing Actions", xmLabelGadgetClass,
				   IPOpEditMenu, NULL);
  widget = XtVaCreateManagedWidget(NULL, xmSeparatorGadgetClass,
				   IPOpEditMenu, NULL);
  widget = XtVaCreateManagedWidget("Load Selected List Item",
				   xmPushButtonGadgetClass, IPOpEditMenu,
				   NULL);
  XtAddCallback(widget, XmNactivateCallback,
		(XtCallbackProc) LoadSelectedListItemCallback,
		NULL);
  submenu = XmCreatePulldownMenu(IPEditingWindowFW, "_pulldown",
				 args, 1);
  str = XmStringCreateSimple("Load New Ip Op");
  widget = XtVaCreateManagedWidget("New Ip Operations",
				   xmCascadeButtonGadgetClass, IPOpEditMenu,
				   XmNsubMenuId, submenu, XmNlabelString, str,
				   NULL);
#ifndef CMGUI
  widget = XtVaCreateManagedWidget("Load FFT Real 1 data",
				   xmPushButtonGadgetClass, IPOpEditMenu,
				   NULL);
  XtAddCallback(widget, XmNactivateCallback, (XtCallbackProc) LoadFTRealImgMenu, NULL);
  widget = XtVaCreateManagedWidget("Load FFT Imaginary 1 data",
				   xmPushButtonGadgetClass, IPOpEditMenu,
				   NULL);
  XtAddCallback(widget, XmNactivateCallback, (XtCallbackProc) LoadFTImImgMenu, NULL);
  widget = XtVaCreateManagedWidget("Load FFT Real 2 data",
				   xmPushButtonGadgetClass, IPOpEditMenu,
				   NULL);
  XtAddCallback(widget, XmNactivateCallback, (XtCallbackProc) LoadFTRealImg2Menu, NULL);
  widget = XtVaCreateManagedWidget("Load FFT Imaginary 2 data",
				   xmPushButtonGadgetClass, IPOpEditMenu,
				   NULL);
  XtAddCallback(widget, XmNactivateCallback, (XtCallbackProc) LoadFTImImg2Menu, NULL);
  widget = XtVaCreateManagedWidget("Load Arctan Radius",
				   xmPushButtonGadgetClass, IPOpEditMenu,
				   NULL);
  XtAddCallback(widget, XmNactivateCallback, (XtCallbackProc) LoadArctanradiusImgMenu, NULL);
  widget = XtVaCreateManagedWidget("Load 2ISin data",
				   xmPushButtonGadgetClass, IPOpEditMenu,
				   NULL);
  XtAddCallback(widget, XmNactivateCallback, (XtCallbackProc) LoadSinImgMenu, NULL);
  widget = XtVaCreateManagedWidget("Load 2ICos data",
				   xmPushButtonGadgetClass, IPOpEditMenu,
				   NULL);
  XtAddCallback(widget, XmNactivateCallback, (XtCallbackProc) LoadCosImgMenu, NULL);
  widget = XtVaCreateManagedWidget("Load FE Mesh",
				   xmPushButtonGadgetClass, IPOpEditMenu,
				   NULL);
  XtAddCallback(widget, XmNactivateCallback, (XtCallbackProc) LoadFEMeshMenu, NULL);
#endif
  widget = XtVaCreateManagedWidget("Load Mask",
				   xmPushButtonGadgetClass, IPOpEditMenu,
				   NULL);
  XtAddCallback(widget, XmNactivateCallback, (XtCallbackProc) LoadMaskMenu, NULL);
#ifdef ALL_INTERFACES
  widget = XtVaCreateManagedWidget("Load Plane Angle",
				   xmPushButtonGadgetClass, IPOpEditMenu,
				   NULL);
  XtAddCallback(widget, XmNactivateCallback, (XtCallbackProc) LoadPlaneAngleMenu, NULL);
  widget = XtVaCreateManagedWidget("Load PStep image",
				   xmPushButtonGadgetClass, IPOpEditMenu,
				   NULL);
  XtAddCallback(widget, XmNactivateCallback, (XtCallbackProc) LoadPStepImg, NULL);
#endif
  widget = XtVaCreateManagedWidget("Push Image",
				   xmPushButtonGadgetClass, IPOpEditMenu,
				   NULL);
  XtAddCallback(widget, XmNactivateCallback, (XtCallbackProc) PushImageMenu, NULL);
  widget = XtVaCreateManagedWidget("Pop Image",
				   xmPushButtonGadgetClass, IPOpEditMenu,
				   NULL);
  XtAddCallback(widget, XmNactivateCallback, (XtCallbackProc) PopImageMenu, NULL);
  widget = XtVaCreateManagedWidget("Swap XY",
				   xmPushButtonGadgetClass, IPOpEditMenu,
				   NULL);
  XtAddCallback(widget, XmNactivateCallback, (XtCallbackProc) SwapMenu, NULL);
  widget = XtVaCreateManagedWidget("Clear Stack",
				   xmPushButtonGadgetClass, IPOpEditMenu,
				   NULL);
  XtAddCallback(widget, XmNactivateCallback, (XtCallbackProc) ClearStackMenu, NULL);
  XmStringFree(str);

  /* init IPOps submenu array */
  IPOpCategories[FILEIOOPS].name = "Load/Save ops"; 
  IPOpCategories[STACKOPS].name = "Stack ops"; 
#ifndef CMGUI
#if (defined (VCAA_CODE) || defined (SGI))
  IPOpCategories[GRABBINGOPS].name = "Grabbing ops"; 
#endif /* VCAA_CODE */
  IPOpCategories[DYNRANGEOPS].name = "Dynamic range ops"; 
#endif /* CMGUI */
  IPOpCategories[RPNOPS].name = "RPN ops"; 
  IPOpCategories[BINARYOPS].name = "Binary image ops"; 
  IPOpCategories[MORPHOLOGICALOPS].name = "Morphological ops"; 
#if defined (NAG)
  IPOpCategories[FFTOPS].name = "Frequency domain filters"; 
#endif /* NAG */
  IPOpCategories[FILTEROPS].name = "Space domain filters"; 
#ifndef CMGUI
  IPOpCategories[FITOPS].name = "Fitting"; 
  IPOpCategories[STEREOOPS].name = "Stereo"; 
  IPOpCategories[WRAPOPS].name = "Wrapping / Unwrapping"; 
#endif /* CMGUI */
  IPOpCategories[MISCOPS].name = "Misc"; 
  IPOpCategories[VIDEOOPS].name = "Video recording";
  
  /* create NEW IPOps submenu cascade buttons */
  for (i = 0; i < IPOPSMAX; i++) {
    if (IPOpCategories[i].name) {
      IPOpCategories[i].menuID
	= XmCreatePulldownMenu(IPEditingWindowFW, "_pulldown",
			       args, 1);
      widget = XtVaCreateManagedWidget(IPOpCategories[i].name, 
				       xmCascadeButtonGadgetClass, submenu,
				       XmNsubMenuId, IPOpCategories[i].menuID,
				       XmNlabelString,
				       XmStringCreateSimple
				       (IPOpCategories[i].name),
				       NULL);
    }
  }
  
  /* load IPOps into the appropriate sub-sub-menus */
  for (i = 0; i < NIPRegisteredOps; i++) {
    if (IPOpCategories[IPRegisteredOpList[i].IPOpCategoryID].name) {
      widget = XtVaCreateManagedWidget(IPRegisteredOpList[i].OpName,
				       xmPushButtonGadgetClass,
				       IPOpCategories[IPRegisteredOpList[i]
						      .IPOpCategoryID].menuID,
				       NULL);
      XtAddCallback(widget, XmNactivateCallback,
		    (XtCallbackProc) IPOpEditCallback,
		    IPRegisteredOpList[i].OpName);
    }
    else {
      sprintf(cbuf, "BuildEditingMenu():Parent for item \"%s\" not found",
	      IPRegisteredOpList[i].OpName);
#ifdef CMGUI
      ErrorMsg(cbuf);
      return;
#else
      printf("\a%s\n", cbuf);
      exit(EXIT_FAILURE);
#endif
    }
  }
}

/* load an IP function definition in the defined function list */
/* very kludgy variable number of argument definition ...      */
void RegisterIPOp(char *OpName, void (*f)(), int IPOpSubmenuID, 
		  boolean_t RepsAllowed, boolean_t EdgeFixFlag,
		    ...)
{
  va_list args;
  int i, PDataType;
  char *PName;

  /* check to make sure list size not exceeded */
  if (NIPRegisteredOps >= IPOPREGISTEREDLISTMAX-1) {
    printf("\aIP operations list size exceeded in RegisterIPOp() at function \'%s\"\n", OpName);
    return;
  }
  
  /* initialize variable argument list retrieval */
  va_start(args, EdgeFixFlag);
  
  /* load the function description into the list */
  sprintf(IPRegisteredOpList[NIPRegisteredOps].OpName, "%s", OpName);
  IPRegisteredOpList[NIPRegisteredOps].f = f;
  IPRegisteredOpList[NIPRegisteredOps].IPOpCategoryID = IPOpSubmenuID;
  IPRegisteredOpList[NIPRegisteredOps].RepsAllowed = RepsAllowed;
  IPRegisteredOpList[NIPRegisteredOps].EdgeFixFlag = EdgeFixFlag;
  for (i = 0; ((PName = va_arg(args, char *)) != NULL) && (i < IPOPNPARMS);
       i++) {
    /* process function parameter pair information */
    switch (PDataType = va_arg(args, int)) {
    case INTEGER_DT:
#ifdef DEBUG
      printf("  Function (%s) : parameter (%s) has type INTEGER\n",
	     OpName, PName);
#endif
      IPRegisteredOpList[NIPRegisteredOps].ParmDataTypes[i] = PDataType;
      sprintf(IPRegisteredOpList[NIPRegisteredOps].ParmNames[i], "%s",
	      PName);
      break;
    case DOUBLE_DT:
#ifdef DEBUG
      printf("  Function (%s) : parameter (%s) has type DOUBLE\n",
	     OpName, PName);
#endif
      IPRegisteredOpList[NIPRegisteredOps].ParmDataTypes[i] = PDataType;
      sprintf(IPRegisteredOpList[NIPRegisteredOps].ParmNames[i], "%s",
	      PName);
      break;
    case DOUBLE_ARRAY_DT:
#ifdef DEBUG
      printf("  Function (%s) : parameter (%s) has type DOUBLE_ARRAY\n",
	     OpName, PName);
#endif
      IPRegisteredOpList[NIPRegisteredOps].ParmDataTypes[i] = PDataType;
      sprintf(IPRegisteredOpList[NIPRegisteredOps].ParmNames[i], "%s",
	      PName);
      break;
    case STRING_DT:
#ifdef DEBUG
      printf("  Function (%s) : parameter (%s) has type STRING\n",
	     OpName, PName);
#endif
      IPRegisteredOpList[NIPRegisteredOps].ParmDataTypes[i] = PDataType;
      sprintf(IPRegisteredOpList[NIPRegisteredOps].ParmNames[i], "%s",
	      PName);
      break;
    case NONE_DT:
    default:
      printf("\aInvalid datatype (%d) for parameter \"%s\" of IP Op \"%s\" in RegisterIPOp()\n", PDataType, PName, OpName);
      va_end(args);
      return;
/*      break; */
    }
  }
  IPRegisteredOpList[NIPRegisteredOps].NParms = i;
  NIPRegisteredOps++;

  /* end retrieval of varaibel length arguments */
  va_end(args);
}



/* draw message in the status area */
void draw_mesg(char *p)
{
  extern int nprocessors;
  char buf[1024];

  if ((DrawMesgEnabled == B_FALSE) || (ChildProcess && (nprocessors > 1))) {
    if (Abort)
      printf("\a%s (CHILD PROCESS)\n", p);
    return;
  }
  
  if (InteractiveFlag) {
    if (IPOperationsShellCreated == B_FALSE)
      return;
    if (strlen(p) > 1024)
      sprintf(buf, "Status - STRING TOO LONG (length = %d)", strlen(p));
    else
      sprintf(buf, "Status - %s", p);
    if (!((strcmp("Manual Abort!", (char *) UxGetLabelString(StatusLG))==0)
	  && Abort))
      UxPutLabelString(StatusLG, buf);
    XmUpdateDisplay(StatusLG);
    XSync(UxDisplay, B_FALSE);
  }
  else if (Abort)
    printf("%s\n", p);
}





