
/*******************************************************************************
	MovieControlsShell.c

       Associated Header file: MovieControlsShell.h
*******************************************************************************/

#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <Mrm/MrmPublic.h>
#include <Xm/MenuShell.h>
#include "UxXt.h"

#include <Xm/Text.h>
#include <Xm/LabelG.h>
#include <Xm/SeparatoG.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/Form.h>
#include <X11/Shell.h>

/*******************************************************************************
       Includes, Defines, and Global variables from the Declarations Editor:
*******************************************************************************/

#include "include/XvgGlobals.h"

extern swidget XvgShell, MovieControlsTBG, fileSelectionBoxDialog, promptDialog;
#ifdef ALL_INTERFACES
extern swidget GLShell;
#endif


/*******************************************************************************
       The following header file defines the context structure.
*******************************************************************************/

#define CONTEXT_MACRO_ACCESS 1
#include "MovieControlsShell.h"
#undef CONTEXT_MACRO_ACCESS

Widget	MovieControlsShell;
Widget	MovieNameLG;
Widget	MovieDelayTW;

static FILE *fp;
static void *parms[4];
static int i, vi;
static char fname[512];

/*******************************************************************************
Auxiliary code from the Declarations Editor:
*******************************************************************************/

void LoadMovieFrame(void)
{
  HGCursor(B_TRUE);
  switch (MovieType) {
  case XMOVIE:
    /* if a node file exists for this image, load it in */
    sprintf(fname, "%s", MovieFileNames[MovieCurrentFile]);
    for (i = strlen(fname) - 1; (i > 0) && (fname[i] != '.'); i--);
    fname[i+1] = '2';
    fname[i+2] = 'd';
    fname[i+3] = '\0';
    if ((fp = fopen(fname, "r"))) {
      fclose(fp);
      parms[0] = fname;
      parms[1] = &vi;
      vi = 0;
      OpticalFlowLoadNodes(RawPixels, RawPixels,
			   ImgHeight/ZoomFactor, ImgWidth/ZoomFactor,
			   parms);
      if (VerboseFlag)
	printf("LoadMovieFrame() : Nodes loaded from file %s...\n", fname);
    }
    LoadFile(MovieFileNames[MovieCurrentFile]);
    if (VerboseFlag)
      printf("LoadMovieFrame() : Image loaded from file %s...\n",
	     MovieFileNames[MovieCurrentFile]);
  break;
  case GLMOVIE:
#ifdef ALL_INTERFACES
    GLShowFrame(MovieCurrentFile);
#endif
  break;
  default:
  break;
  }
  HGCursor(B_FALSE);
}

boolean_t LoadMovie(char *fname)
{
  FILE *fpo;
  int i, j;
  char buf[512];

  /* de-allocate movie filenames if required */
  if (MovieFileNames) {
    for (i = 0; i < MovieFilesN; i++)
      XvgMM_Free(MovieFileNames[i]);
  }
  
  /* open input file */
  fpo = fopen(fname, "r");
  if (fpo == NULL) {
    sprintf(cbuf, "Could not open movie file: %s", fname);
    ErrorMsg(cbuf);
    return(B_FALSE);
  }

  /* count number of images in the file */
  for (MovieFilesN=0; (fgets(buf, 512, fpo) != NULL) && (buf[0] != '\n');
       MovieFilesN++);

  /* allocate space for file name array */
  if ((MovieFileNames = (char **) XvgMM_Alloc((void *) MovieFileNames,
					      MovieFilesN * sizeof(char *)))
      == NULL) {
    ErrorMsg("Error allocating in LoadMovie()");
    return(B_FALSE);
  }
  
  /* load the filenames into an array */
  for (j = 0, fseek(fpo, 0, 0); j < MovieFilesN; j++) {
    /* get next line in the file */
    fgets(buf, 512, fpo);

    /* strip carriage return if there is one */
    if (buf[strlen(buf)-1] == '\n')
      buf[strlen(buf)-1] = '\0';

    /* add directory path if required and load filename into array */
    for (i = strlen(buf)-1; (i > 0) && (buf[i] != '/'); i--);
    if (buf[i] != '/') {
      
      /* allocate space for file name */
      if ((MovieFileNames[j] =
	   (char *) XvgMM_Alloc(NULL,
				strlen(UxGetDirectory(fileSelectionBoxDialog))
				+ strlen(buf) + 1)) == NULL) {
	sprintf(cbuf, "Error allocating for filename %s%s in LoadMovie()", 
		UxGetDirectory(fileSelectionBoxDialog), buf);
	ErrorMsg(cbuf);
	return(B_FALSE);
      }
      
      /* load the filename in the name array */
      sprintf(MovieFileNames[j], "%s%s",
	      UxGetDirectory(fileSelectionBoxDialog), buf);
    }
    else {
      /* allocate space for file name */
      if ((MovieFileNames[j] =
	   (char *) XvgMM_Alloc(NULL, strlen(buf) + 1)) == NULL) {
	sprintf(cbuf, "Error allocating for filename %s in LoadMovie()", buf);
	ErrorMsg(cbuf);
	return(B_FALSE);
      }
      sprintf(MovieFileNames[j], "%s", buf);
    }
  }
  MovieCurrentFile = 0;
  
  /* close file */
  fclose(fpo);
  return(B_TRUE);
}

void LoopMovie(void)
{
    XmString xs;
    int i;
    char *p;

    /* check input parms */
    if (VCRFramesPerImage >= 500) {
	ErrorMsg("Maximum number of frames/image exceeded (499)!");
        return;
    }

    /* fetch loop count */
    XtVaGetValues(UxGetWidget(promptDialog), XmNtextString, &xs, NULL);
    XmStringGetLtoR(xs, XmSTRING_DEFAULT_CHARSET, &p);
    MovieLoopCount = atoi(p);
    if (p)
      free(p);

    /* desensitise all buttons except for stop */
    XtSetSensitive(UxGetWidget(LoopPBG), B_FALSE);
    XtSetSensitive(UxGetWidget(ForwardPBG), B_FALSE);
    XtSetSensitive(UxGetWidget(BackwardPBG), B_FALSE);
    XtSetSensitive(UxGetWidget(ResetPBG), B_FALSE);
    XtSetSensitive(UxGetWidget(XvgShell), B_FALSE);

    HGCursor(B_TRUE);
    ScreenSaverOff();
    for (Abort=B_FALSE, i = 0; (Abort == B_FALSE) && (i < MovieLoopCount);) {
      switch(MovieType) {
      case XMOVIE:
	/* if a node file exists for this image, load it in */
	sprintf(fname, "%s", MovieFileNames[MovieCurrentFile]);
	for (i = strlen(fname) - 1; (i > 0) && (fname[i] != '.'); i--);
	fname[i+1] = '2';
	fname[i+2] = 'd';
	fname[i+3] = '\0';
	if ((fp = fopen(fname, "r"))) {
	  fclose(fp);
	  parms[0] = fname;
	  parms[1] = &vi;
	  vi = 0;
	  OpticalFlowLoadNodes(RawPixels, RawPixels,
			       ImgHeight/ZoomFactor, ImgWidth/ZoomFactor,
			       parms);
	}
	/* load the image */
        LoadFile(MovieFileNames[MovieCurrentFile++]);
      break;
      case GLMOVIE:
#ifdef ALL_INTERFACES
        GLShowFrame(MovieCurrentFile++);
#endif
      break;
      default:
      break;
      }
      if (MovieCurrentFile == MovieFilesN) {
        MovieCurrentFile = 0;
	i++;
      }
      sleep_us(atoi(UxGetText(MovieDelayTW))*1000);
    }
    Abort = B_FALSE;
    ScreenSaverOn();
    HGCursor(B_FALSE);

    /* resensitise all buttons except for stop */
    XtSetSensitive(UxGetWidget(LoopPBG), B_TRUE);
    XtSetSensitive(UxGetWidget(XvgShell), B_TRUE);
    XtSetSensitive(UxGetWidget(ForwardPBG), B_TRUE);
    XtSetSensitive(UxGetWidget(BackwardPBG), B_TRUE);
    XtSetSensitive(UxGetWidget(ResetPBG), B_TRUE);
}

/*******************************************************************************
       The following are callback functions.
*******************************************************************************/

static	void	destroyCB_MovieControlsShell(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCMovieControlsShell  *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxMovieControlsShellContext;
	UxMovieControlsShellContext = UxContext =
			(_UxCMovieControlsShell *) UxGetContext( UxWidget );
	{
	  if (XvgUp)
	    UxPutSet(MovieControlsTBG, "false");
	  MovieControlsShellCreated = B_FALSE;
	  if (VerboseFlag)
	    printf("XVG MovieControlsShell destroyed.\n");
	}
	UxMovieControlsShellContext = UxSaveCtx;
}

static	void	popdownCB_MovieControlsShell(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCMovieControlsShell  *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxMovieControlsShellContext;
	UxMovieControlsShellContext = UxContext =
			(_UxCMovieControlsShell *) UxGetContext( UxWidget );
	{
	UxPutSet(MovieControlsTBG, "false");
	}
	UxMovieControlsShellContext = UxSaveCtx;
}

static	void	popupCB_MovieControlsShell(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCMovieControlsShell  *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxMovieControlsShellContext;
	UxMovieControlsShellContext = UxContext =
			(_UxCMovieControlsShell *) UxGetContext( UxWidget );
	{
	UxPutSet(MovieControlsTBG, "true");
	
	}
	UxMovieControlsShellContext = UxSaveCtx;
}

static	void	activateCB_LoopPBG(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCMovieControlsShell  *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxMovieControlsShellContext;
	UxMovieControlsShellContext = UxContext =
			(_UxCMovieControlsShell *) UxGetContext( UxWidget );
	{
	char b[256];
	
	/* fetch loop count */
	PromptDialogAction = VCRMOVIELOOPACTION;
	MovieLoopDelay = atoi(UxGetText(MovieDelayTW));
	UxPutSelectionLabelString(promptDialog, "Loop count :");
	MovieLoopCount = (MovieLoopCount <= 0 ? 1 : MovieLoopCount);
	sprintf(b, "%d", MovieLoopCount);
	UxPutTextString(promptDialog, b);
	UxPopupInterface(promptDialog, no_grab);
	
	}
	UxMovieControlsShellContext = UxSaveCtx;
}

static	void	activateCB_ResetPBG(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCMovieControlsShell  *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxMovieControlsShellContext;
	UxMovieControlsShellContext = UxContext =
			(_UxCMovieControlsShell *) UxGetContext( UxWidget );
	{
	Abort = B_FALSE;
	if (VCRFramesPerImage >= 500) {
	    ErrorMsg("Maximum number of frames/image exceeded (499)!");
	    return;
	}
	MovieCurrentFile = 0;
	
	HGCursor(B_TRUE);
	switch (MovieType) {
	case XMOVIE:
	  /* if a node file exists for this image, load it in */
	  sprintf(fname, "%s", MovieFileNames[MovieCurrentFile]);
	  for (i = strlen(fname) - 1; (i > 0) && (fname[i] != '.'); i--);
	  fname[i+1] = '2';
	  fname[i+2] = 'd';
	  fname[i+3] = '\0';
	  if ((fp = fopen(fname, "r"))) {
	    fclose(fp);
	    parms[0] = fname;
	    parms[1] = &vi;
	    vi = 0;
	    OpticalFlowLoadNodes(RawPixels, RawPixels,
				 ImgHeight/ZoomFactor, ImgWidth/ZoomFactor,
				 parms);
	  }
	  LoadFile(MovieFileNames[MovieCurrentFile]);
	break;
	case GLMOVIE:
	  break;
	default:
	break;
	}
	HGCursor(B_FALSE);
	
	}
	UxMovieControlsShellContext = UxSaveCtx;
}

static	void	activateCB_BackwardPBG(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCMovieControlsShell  *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxMovieControlsShellContext;
	UxMovieControlsShellContext = UxContext =
			(_UxCMovieControlsShell *) UxGetContext( UxWidget );
	{
	if (VCRFramesPerImage >= 500) {
	  ErrorMsg("Maximum number of frames/image exceeded (499)!");
	  return;
	}
	if (MovieCurrentFile == 0)
	  MovieCurrentFile = MovieFilesN-1;
	else
	  MovieCurrentFile--;
	
	LoadMovieFrame();
	}
	UxMovieControlsShellContext = UxSaveCtx;
}

static	void	activateCB_ForwardPBG(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCMovieControlsShell  *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxMovieControlsShellContext;
	UxMovieControlsShellContext = UxContext =
			(_UxCMovieControlsShell *) UxGetContext( UxWidget );
	{
	if (VCRFramesPerImage >= 500) {
	  ErrorMsg("Maximum number of frames/image exceeded (499)!");
	   return;
	}
	
	if (MovieCurrentFile < (MovieFilesN-1))
	  MovieCurrentFile++;
	else
	  MovieCurrentFile = 0;
	
	LoadMovieFrame();
	}
	UxMovieControlsShellContext = UxSaveCtx;
}

static	void	activateCB_IndexPBG(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCMovieControlsShell  *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxMovieControlsShellContext;
	UxMovieControlsShellContext = UxContext =
			(_UxCMovieControlsShell *) UxGetContext( UxWidget );
	{
	char b[256];
	
	/* fetch loop count */
	PromptDialogAction = SELECTMOVIEFILEINDEXACTION;
	UxPutSelectionLabelString(promptDialog, "Image index :");
	sprintf(b, "%d", MovieCurrentFile);
	UxPutTextString(promptDialog, b);
	UxPopupInterface(promptDialog, no_grab);
	
	}
	UxMovieControlsShellContext = UxSaveCtx;
}

static	void	activateCB_LoadMFPBG(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCMovieControlsShell  *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxMovieControlsShellContext;
	UxMovieControlsShellContext = UxContext =
			(_UxCMovieControlsShell *) UxGetContext( UxWidget );
	{
	extern swidget ProcessedImageShell;
	XEvent e;
	XmDrawingAreaCallbackStruct s;
	char cbuf[512], fname[512];
	int i, *parms[1];
	
	HGCursor(B_TRUE);
	sprintf(cbuf, "%s", MovieFileNames[MovieCurrentFile]);
	for (i = (strlen(cbuf) - 1); (i >= 0) && (cbuf[i] != 'y') && (cbuf[i] != 'x'); i--);
	cbuf[i+1] = '\0';
	for (; (i >= 0) && (cbuf[i] != '/'); i--);
	if (i < 2) {
	  sprintf(cbuf, "Error parsing file \"%s\"", MovieFileNames[MovieCurrentFile]);
	  ErrorMsg(cbuf);
	  return;
	}
	cbuf[i-2] = '\0';
	sprintf(fname, "%smf/%s_diff_mf.mat", cbuf, &(cbuf[i+1]));
	if (GetMatlabFile(&ImgHeight, &ImgWidth, &ProcMin, &ProcMax,
		      &ProcessedPixels, fname, VerboseFlag)) {
	  RealtoX(ProcessedPixels, ProcessedPixelsX, ImgHeight, ImgWidth,
		ProcMax, ProcMin, MaxScaleVal, MinScaleVal, ZoomFactor);
	  s.event = &e;
	  s.event->xexpose.x = 0;
	  s.event->xexpose.y = 0;
	  s.event->xexpose.width = ImgWidth*ZoomFactor;
	  s.event->xexpose.height = ImgHeight*ZoomFactor;
	  ProcExposeCB(UxGetWidget(ProcessedImageShell), NULL, &s);
	  HGCursor(B_FALSE);
	}
	}
	UxMovieControlsShellContext = UxSaveCtx;
}

static	void	activateCB_menu3_p2_b2(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCMovieControlsShell  *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxMovieControlsShellContext;
	UxMovieControlsShellContext = UxContext =
			(_UxCMovieControlsShell *) UxGetContext( UxWidget );
	{VCRRecordOn = VCAWriteOn = B_FALSE;}
	UxMovieControlsShellContext = UxSaveCtx;
}

static	void	activateCB_menu3_p2_b5(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCMovieControlsShell  *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxMovieControlsShellContext;
	UxMovieControlsShellContext = UxContext =
			(_UxCMovieControlsShell *) UxGetContext( UxWidget );
	{
	    VCRRecordOn = B_FALSE;
	    VCAWriteOn = B_TRUE;
	}
	UxMovieControlsShellContext = UxSaveCtx;
}

static	void	activateCB_menu3_p2_b1(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCMovieControlsShell  *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxMovieControlsShellContext;
	UxMovieControlsShellContext = UxContext =
			(_UxCMovieControlsShell *) UxGetContext( UxWidget );
	{
	    char b[16];
	
	    PromptDialogAction = VCRIMAGESPERSECACTION;
	    UxPutSelectionLabelString(promptDialog, "VCR 30 Hz frames/image :");
	    sprintf(b, "%d", VCRFramesPerImage);
	    UxPutTextString(promptDialog, b);
	    UxPopupInterface(promptDialog, no_grab);
	    VCRRecordOn = B_TRUE;
	    VCAWriteOn = B_TRUE;
	}
	UxMovieControlsShellContext = UxSaveCtx;
}

static	void	activateCB_menu3_p2_b3(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCMovieControlsShell  *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxMovieControlsShellContext;
	UxMovieControlsShellContext = UxContext =
			(_UxCMovieControlsShell *) UxGetContext( UxWidget );
	{FixDynamicRange = B_FALSE;}
	UxMovieControlsShellContext = UxSaveCtx;
}

static	void	activateCB_menu3_p2_b4(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCMovieControlsShell  *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxMovieControlsShellContext;
	UxMovieControlsShellContext = UxContext =
			(_UxCMovieControlsShell *) UxGetContext( UxWidget );
	{FixDynamicRange = B_TRUE;}
	UxMovieControlsShellContext = UxSaveCtx;
}

static	void	activateCB_LoadXMoviePBG(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCMovieControlsShell  *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxMovieControlsShellContext;
	UxMovieControlsShellContext = UxContext =
			(_UxCMovieControlsShell *) UxGetContext( UxWidget );
	{
	FileSelectionBoxAction = LOADMOVIEACTION;
	UxPutSelectionLabelString(fileSelectionBoxDialog, "X movie file name:");
	UxPutPattern(fileSelectionBoxDialog, "*movie*.*");
	UxPopupInterface(fileSelectionBoxDialog, no_grab);
	}
	UxMovieControlsShellContext = UxSaveCtx;
}

static	void	activateCB_LoadGLMoviePBG(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCMovieControlsShell  *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxMovieControlsShellContext;
	UxMovieControlsShellContext = UxContext =
			(_UxCMovieControlsShell *) UxGetContext( UxWidget );
	{
#ifdef ALL_INTERFACES
	if (GLShellCreated == B_FALSE) {
	  GLShell = create_GLShell();
	  UxPopupInterface(GLShell, no_grab);
	}
	FileSelectionBoxAction = LOADGLMOVIEACTION;
	UxPutSelectionLabelString(fileSelectionBoxDialog, "X movie file name:");
	UxPutPattern(fileSelectionBoxDialog, "*.GL");
	UxPopupInterface(fileSelectionBoxDialog, no_grab);
#endif
	}
	UxMovieControlsShellContext = UxSaveCtx;
}

/*******************************************************************************
       The 'build_' function creates all the widgets
       using the resource values specified in the Property Editor.
*******************************************************************************/

static Widget	_Uxbuild_MovieControlsShell(void)
{
	MrmType		_UxDummyClass;
	Cardinal	_UxStatus;


	/* Creation of MovieControlsShell */
	MovieControlsShell = XtVaCreatePopupShell( "MovieControlsShell",
			applicationShellWidgetClass,
			UxTopLevel,
			XmNx, 510,
			XmNy, 238,
			XmNwidth, 442,
			XmNheight, 202,
			XmNiconName, "Movie",
			XmNtitle, "Movie Controls",
			XmNallowShellResize, B_FALSE,
			NULL );
	XtAddCallback( MovieControlsShell, XmNdestroyCallback,
		(XtCallbackProc) destroyCB_MovieControlsShell,
		(XtPointer) UxMovieControlsShellContext );
	XtAddCallback( MovieControlsShell, XmNpopdownCallback,
		(XtCallbackProc) popdownCB_MovieControlsShell,
		(XtPointer) UxMovieControlsShellContext );
	XtAddCallback( MovieControlsShell, XmNpopupCallback,
		(XtCallbackProc) popupCB_MovieControlsShell,
		(XtPointer) UxMovieControlsShellContext );

	UxPutContext( MovieControlsShell, (char *) UxMovieControlsShellContext );

	form5 = NULL;
	_UxStatus = MrmFetchWidget( hierarchy,
					"form5",
					MovieControlsShell,
					&form5,
					&_UxDummyClass );
	if ( _UxStatus != MrmSUCCESS )
	{
		UxMrmFetchError( hierarchy, "form5",
				MovieControlsShell, _UxStatus );
		return( (Widget) NULL );
	}

	XtManageChild( form5 );

	UxPutContext( form5, (char *) UxMovieControlsShellContext );
	rowColumn9 = XtNameToWidget( form5, "*rowColumn9" );
	UxPutContext( rowColumn9, (char *) UxMovieControlsShellContext );
	LoopPBG = XtNameToWidget( rowColumn9, "*LoopPBG" );
	UxPutContext( LoopPBG, (char *) UxMovieControlsShellContext );
	ResetPBG = XtNameToWidget( rowColumn9, "*ResetPBG" );
	UxPutContext( ResetPBG, (char *) UxMovieControlsShellContext );
	BackwardPBG = XtNameToWidget( rowColumn9, "*BackwardPBG" );
	UxPutContext( BackwardPBG, (char *) UxMovieControlsShellContext );
	ForwardPBG = XtNameToWidget( rowColumn9, "*ForwardPBG" );
	UxPutContext( ForwardPBG, (char *) UxMovieControlsShellContext );
	IndexPBG = XtNameToWidget( rowColumn9, "*IndexPBG" );
	UxPutContext( IndexPBG, (char *) UxMovieControlsShellContext );
	LoadMFPBG = XtNameToWidget( rowColumn9, "*LoadMFPBG" );
	UxPutContext( LoadMFPBG, (char *) UxMovieControlsShellContext );
	menu3 = XtNameToWidget( form5, "*menu3" );
	UxPutContext( menu3, (char *) UxMovieControlsShellContext );
	menu3_p2 = XtNameToWidget( form5, "*menu3_p2" );
	UxPutContext( menu3_p2, (char *) UxMovieControlsShellContext );
	menu3_p2_b2 = XtNameToWidget( menu3_p2, "*menu3_p2_b2" );
	UxPutContext( menu3_p2_b2, (char *) UxMovieControlsShellContext );
	menu3_p2_b5 = XtNameToWidget( menu3_p2, "*menu3_p2_b5" );
	UxPutContext( menu3_p2_b5, (char *) UxMovieControlsShellContext );
	menu3_p2_b1 = XtNameToWidget( menu3_p2, "*menu3_p2_b1" );
	UxPutContext( menu3_p2_b1, (char *) UxMovieControlsShellContext );
	menu6 = XtNameToWidget( form5, "*menu6" );
	UxPutContext( menu6, (char *) UxMovieControlsShellContext );
	menu3_p3 = XtNameToWidget( form5, "*menu3_p3" );
	UxPutContext( menu3_p3, (char *) UxMovieControlsShellContext );
	menu3_p2_b3 = XtNameToWidget( menu3_p3, "*menu3_p2_b3" );
	UxPutContext( menu3_p2_b3, (char *) UxMovieControlsShellContext );
	menu3_p2_b4 = XtNameToWidget( menu3_p3, "*menu3_p2_b4" );
	UxPutContext( menu3_p2_b4, (char *) UxMovieControlsShellContext );
	separatorGadget19 = XtNameToWidget( form5, "*separatorGadget19" );
	UxPutContext( separatorGadget19, (char *) UxMovieControlsShellContext );
	LoadXMoviePBG = XtNameToWidget( form5, "*LoadXMoviePBG" );
	UxPutContext( LoadXMoviePBG, (char *) UxMovieControlsShellContext );
	MovieNameLG = XtNameToWidget( form5, "*MovieNameLG" );
	UxPutContext( MovieNameLG, (char *) UxMovieControlsShellContext );
	LoadGLMoviePBG = XtNameToWidget( form5, "*LoadGLMoviePBG" );
	UxPutContext( LoadGLMoviePBG, (char *) UxMovieControlsShellContext );
	separatorGadget21 = XtNameToWidget( form5, "*separatorGadget21" );
	UxPutContext( separatorGadget21, (char *) UxMovieControlsShellContext );
	labelGadget134 = XtNameToWidget( form5, "*labelGadget134" );
	UxPutContext( labelGadget134, (char *) UxMovieControlsShellContext );
	MovieDelayTW = XtNameToWidget( form5, "*MovieDelayTW" );
	UxPutContext( MovieDelayTW, (char *) UxMovieControlsShellContext );

	XtAddCallback( MovieControlsShell, XmNdestroyCallback,
		(XtCallbackProc) UxDestroyContextCB,
		(XtPointer) UxMovieControlsShellContext);

	return ( MovieControlsShell );
}

/*******************************************************************************
       The following is the 'Interface function' which is the
       external entry point for creating this interface.
       This function should be called from your application or from
       a callback function.
*******************************************************************************/

Widget	create_MovieControlsShell(void)
{
	Widget                  rtrn;
	_UxCMovieControlsShell  *UxContext;
	static int		_Uxinit = 0;

	UxMovieControlsShellContext = UxContext =
		(_UxCMovieControlsShell *) UxNewContext( sizeof(_UxCMovieControlsShell), B_FALSE );


	if ( ! _Uxinit )
	{
		static MrmRegisterArg	_UxMrmNames[] = {
			{ "destroyCB_MovieControlsShell", 
				(XtPointer) destroyCB_MovieControlsShell },
			{ "popdownCB_MovieControlsShell", 
				(XtPointer) popdownCB_MovieControlsShell },
			{ "popupCB_MovieControlsShell", 
				(XtPointer) popupCB_MovieControlsShell },
			{ "activateCB_LoopPBG", 
				(XtPointer) activateCB_LoopPBG },
			{ "activateCB_ResetPBG", 
				(XtPointer) activateCB_ResetPBG },
			{ "activateCB_BackwardPBG", 
				(XtPointer) activateCB_BackwardPBG },
			{ "activateCB_ForwardPBG", 
				(XtPointer) activateCB_ForwardPBG },
			{ "activateCB_IndexPBG", 
				(XtPointer) activateCB_IndexPBG },
			{ "activateCB_LoadMFPBG", 
				(XtPointer) activateCB_LoadMFPBG },
			{ "activateCB_menu3_p2_b2", 
				(XtPointer) activateCB_menu3_p2_b2 },
			{ "activateCB_menu3_p2_b5", 
				(XtPointer) activateCB_menu3_p2_b5 },
			{ "activateCB_menu3_p2_b1", 
				(XtPointer) activateCB_menu3_p2_b1 },
			{ "activateCB_menu3_p2_b3", 
				(XtPointer) activateCB_menu3_p2_b3 },
			{ "activateCB_menu3_p2_b4", 
				(XtPointer) activateCB_menu3_p2_b4 },
			{ "activateCB_LoadXMoviePBG", 
				(XtPointer) activateCB_LoadXMoviePBG },
			{ "activateCB_LoadGLMoviePBG", 
				(XtPointer) activateCB_LoadGLMoviePBG }};

		MrmRegisterNamesInHierarchy( hierarchy,
					_UxMrmNames,
					XtNumber(_UxMrmNames) );

		_Uxinit = 1;
	}

	rtrn = _Uxbuild_MovieControlsShell();

	MovieControlsShellCreated = B_TRUE;
	return(rtrn);
}

/*******************************************************************************
       END OF FILE
*******************************************************************************/

