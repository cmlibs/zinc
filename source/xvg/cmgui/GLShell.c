
/*******************************************************************************
	GLShell.c

       Associated Header file: GLShell.h
*******************************************************************************/

#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <Mrm/MrmPublic.h>
#include <Xm/MenuShell.h>
#include "UxXt.h"

#include <Xm/RowColumn.h>
#include <Xm/ToggleBG.h>
#include <Xm/Scale.h>
#include <Xm/Text.h>
#include <Xm/PushBG.h>
#include <Xm/LabelG.h>
#include <Xm/SeparatoG.h>
#include <Xm/DrawingA.h>
#include <Xm/Frame.h>
#include <Xm/Form.h>
#include <X11/Shell.h>

#if defined (GL_API)
#include <gl/gl.h>
#include <gl/device.h>
#endif /* defined (GL_API) */

/*******************************************************************************
       Includes, Defines, and Global variables from the Declarations Editor:
*******************************************************************************/

#include "XvgGlobals.h"

extern Widget GLShellTBG, fileSelectionBoxDialog;

/*******************************************************************************
       The following header file defines the context structure.
*******************************************************************************/

#define CONTEXT_MACRO_ACCESS 1
#include "GLShell.h"
#undef CONTEXT_MACRO_ACCESS

Widget	GLShell;
Widget	PixelsPerMMTW;
Widget	SpecularTW;
Widget	DiffuseTW;
Widget	ShininessTW;
Widget	DecimationTW;
Widget	FramesTW;
Widget	StartXYZTW;
Widget	IncXYZTW;
Widget	ScaleX;
Widget	ScaleY;
Widget	ScaleZ;
Widget	ScaleZoom;

/*******************************************************************************
Auxiliary code from the Declarations Editor:
*******************************************************************************/

void GenGLMovie(char *fname)
{
#if defined (GL_API)
  HGCursor(B_TRUE);
  GLMovieGenerate(atoi(UxGetText(DecimationTW)),
		  UxGetValue(ScaleX), UxGetValue(ScaleY), UxGetValue(ScaleZ),
		  UxGetValue(ScaleZoom),
		  UxGetText(StartXYZTW), UxGetText(IncXYZTW),
		  atoi(UxGetText(FramesTW)), 
		  UxGetText(SpecularTW), UxGetText(DiffuseTW),
		  UxGetText(ShininessTW),
		  atof(UxGetText(PixelsPerMMTW)), fname);
  HGCursor(B_FALSE);
#endif /* defined (GL_API) */
}

/*******************************************************************************
       The following are callback functions.
*******************************************************************************/

#if defined (GL_API)
static	void	destroyCB_GLShell(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCGLShell             *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxGLShellContext;
	UxGLShellContext = UxContext =
			(_UxCGLShell *) UxGetContext( UxWidget );
	{
	UxPutSet(GLShellTBG, "false");
	GLShellCreated = B_FALSE;
	
	}
	UxGLShellContext = UxSaveCtx;
}

static	void	popdownCB_GLShell(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCGLShell             *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxGLShellContext;
	UxGLShellContext = UxContext =
			(_UxCGLShell *) UxGetContext( UxWidget );
	{
	UxPutSet(GLShellTBG, "false");
	
	}
	UxGLShellContext = UxSaveCtx;
}

static	void	popupCB_GLShell(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCGLShell             *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxGLShellContext;
	UxGLShellContext = UxContext =
			(_UxCGLShell *) UxGetContext( UxWidget );
	{
	UxPutSet(GLShellTBG, "true");
	
	}
	UxGLShellContext = UxSaveCtx;
}

static	void	activateCB_BuildPolyPBG(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCGLShell             *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxGLShellContext;
	UxGLShellContext = UxContext =
			(_UxCGLShell *) UxGetContext( UxWidget );
	{
	HGCursor(B_TRUE);
	GLBuildPoly(atoi(UxGetText(DecimationTW)),
		UxGetValue(ScaleX), UxGetValue(ScaleY), UxGetValue(ScaleZ), UxGetValue(ScaleZoom),
		UxGetText(StartXYZTW), UxGetText(IncXYZTW),
		atoi(UxGetText(FramesTW)), 
		UxGetText(SpecularTW), UxGetText(DiffuseTW), UxGetText(ShininessTW),
		atof(UxGetText(PixelsPerMMTW)));
	HGCursor(B_FALSE);
	}
	UxGLShellContext = UxSaveCtx;
}

static	void	activateCB_SavePBG(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCGLShell             *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxGLShellContext;
	UxGLShellContext = UxContext =
			(_UxCGLShell *) UxGetContext( UxWidget );
	{
	FileSelectionBoxAction = SAVEGLMOVIEACTION;
	UxPutSelectionLabelString(fileSelectionBoxDialog, "GL movie output file name:");
	UxPutPattern(fileSelectionBoxDialog, "*");
	UxPopupInterface(fileSelectionBoxDialog, no_grab);
	}
	UxGLShellContext = UxSaveCtx;
}

static	void	activateCB_SaveFramePBG(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCGLShell             *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxGLShellContext;
	UxGLShellContext = UxContext =
			(_UxCGLShell *) UxGetContext( UxWidget );
	{
	FileSelectionBoxAction = SAVEGLFRAMEACTION;
	UxPutSelectionLabelString(fileSelectionBoxDialog, "GL frame output file name:");
	UxPutPattern(fileSelectionBoxDialog, "*");
	UxPopupInterface(fileSelectionBoxDialog, no_grab);
	}
	UxGLShellContext = UxSaveCtx;
}

static	void	activateCB_GLResetPBG(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCGLShell             *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxGLShellContext;
	UxGLShellContext = UxContext =
			(_UxCGLShell *) UxGetContext( UxWidget );
	{
	HGCursor(B_TRUE);
	GLResetMovie(atoi(UxGetText(DecimationTW)),
		UxGetValue(ScaleX), UxGetValue(ScaleY), UxGetValue(ScaleZ),
		UxGetText(StartXYZTW), UxGetText(IncXYZTW),
		atoi(UxGetText(FramesTW)), 
		UxGetText(SpecularTW), UxGetText(DiffuseTW), UxGetText(ShininessTW),
		atof(UxGetText(PixelsPerMMTW)));
	HGCursor(B_FALSE);
	}
	UxGLShellContext = UxSaveCtx;
}

static	void	activateCB_GeneratePBG(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCGLShell             *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxGLShellContext;
	UxGLShellContext = UxContext =
			(_UxCGLShell *) UxGetContext( UxWidget );
	{
	HGCursor(B_TRUE);
	GLMovieSeqGenerate(atoi(UxGetText(DecimationTW)),
		UxGetValue(ScaleX), UxGetValue(ScaleY), UxGetValue(ScaleZ), UxGetValue(ScaleZoom), 
		UxGetText(StartXYZTW), UxGetText(IncXYZTW),
		atoi(UxGetText(FramesTW)), 
		UxGetText(SpecularTW), UxGetText(DiffuseTW), UxGetText(ShininessTW),
		atof(UxGetText(PixelsPerMMTW)));
	HGCursor(B_FALSE);
	}
	UxGLShellContext = UxSaveCtx;
}

static	void	activateCB_LoadFieldPBG(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCGLShell             *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxGLShellContext;
	UxGLShellContext = UxContext =
			(_UxCGLShell *) UxGetContext( UxWidget );
	{
	HGCursor(B_TRUE);
	LoadGLField();
	HGCursor(B_FALSE);
	}
	UxGLShellContext = UxSaveCtx;
}

static	void	activateCB_RecFramePBG(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCGLShell             *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxGLShellContext;
	UxGLShellContext = UxContext =
			(_UxCGLShell *) UxGetContext( UxWidget );
	{
	HGCursor(B_TRUE);
	GLRecFrame(atoi(UxGetText(DecimationTW)),
		UxGetValue(ScaleX), UxGetValue(ScaleY), UxGetValue(ScaleZ), UxGetValue(ScaleZoom),
		UxGetText(StartXYZTW), UxGetText(IncXYZTW),
		atoi(UxGetText(FramesTW)), 
		UxGetText(SpecularTW), UxGetText(DiffuseTW), UxGetText(ShininessTW),
		atof(UxGetText(PixelsPerMMTW)));
	HGCursor(B_FALSE);
	}
	UxGLShellContext = UxSaveCtx;
}

static	void	activateCB_GenMoviePBG(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCGLShell             *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxGLShellContext;
	UxGLShellContext = UxContext =
			(_UxCGLShell *) UxGetContext( UxWidget );
	{
	FileSelectionBoxAction = GENGLMOVIEACTION;
	UxPutSelectionLabelString(fileSelectionBoxDialog, "Image name list input file name:");
	UxPutPattern(fileSelectionBoxDialog, "*");
	UxPopupInterface(fileSelectionBoxDialog, no_grab);
	
	}
	UxGLShellContext = UxSaveCtx;
}

static	void	dragCB_ScaleX(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCGLShell             *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxGLShellContext;
	UxGLShellContext = UxContext =
			(_UxCGLShell *) UxGetContext( UxWidget );
	{
	if (RedrawOnDrag)
	    GLDraw( atoi(UxGetText(DecimationTW)),
		UxGetValue(ScaleX), UxGetValue(ScaleY), UxGetValue(ScaleZ), UxGetValue(ScaleZoom),
		UxGetText(StartXYZTW), UxGetText(IncXYZTW),
		atoi(UxGetText(FramesTW)), 
		UxGetText(SpecularTW), UxGetText(DiffuseTW), UxGetText(ShininessTW),
		atof(UxGetText(PixelsPerMMTW)));
	
	}
	UxGLShellContext = UxSaveCtx;
}

static	void	valueChangedCB_ScaleX(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCGLShell             *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxGLShellContext;
	UxGLShellContext = UxContext =
			(_UxCGLShell *) UxGetContext( UxWidget );
	{
	if (RedrawOnDrag)
	    GLDraw( atoi(UxGetText(DecimationTW)),
		UxGetValue(ScaleX), UxGetValue(ScaleY), UxGetValue(ScaleZ), UxGetValue(ScaleZoom),
		UxGetText(StartXYZTW), UxGetText(IncXYZTW),
		atoi(UxGetText(FramesTW)), 
		UxGetText(SpecularTW), UxGetText(DiffuseTW), UxGetText(ShininessTW),
		atof(UxGetText(PixelsPerMMTW)));
	
	}
	UxGLShellContext = UxSaveCtx;
}

static	void	dragCB_ScaleY(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCGLShell             *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxGLShellContext;
	UxGLShellContext = UxContext =
			(_UxCGLShell *) UxGetContext( UxWidget );
	{
	if (RedrawOnDrag)
	    GLDraw( atoi(UxGetText(DecimationTW)),
		UxGetValue(ScaleX), UxGetValue(ScaleY), UxGetValue(ScaleZ), UxGetValue(ScaleZoom),
		UxGetText(StartXYZTW), UxGetText(IncXYZTW),
		atoi(UxGetText(FramesTW)), 
		UxGetText(SpecularTW), UxGetText(DiffuseTW), UxGetText(ShininessTW),
		atof(UxGetText(PixelsPerMMTW)));
	
	}
	UxGLShellContext = UxSaveCtx;
}

static	void	valueChangedCB_ScaleY(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCGLShell             *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxGLShellContext;
	UxGLShellContext = UxContext =
			(_UxCGLShell *) UxGetContext( UxWidget );
	{
	if (RedrawOnDrag)
	    GLDraw( atoi(UxGetText(DecimationTW)),
		UxGetValue(ScaleX), UxGetValue(ScaleY), UxGetValue(ScaleZ), UxGetValue(ScaleZoom),
		UxGetText(StartXYZTW), UxGetText(IncXYZTW),
		atoi(UxGetText(FramesTW)), 
		UxGetText(SpecularTW), UxGetText(DiffuseTW), UxGetText(ShininessTW),
		atof(UxGetText(PixelsPerMMTW)));
	
	}
	UxGLShellContext = UxSaveCtx;
}

static	void	dragCB_ScaleZ(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCGLShell             *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxGLShellContext;
	UxGLShellContext = UxContext =
			(_UxCGLShell *) UxGetContext( UxWidget );
	{
	if (RedrawOnDrag)
	    GLDraw( atoi(UxGetText(DecimationTW)),
		UxGetValue(ScaleX), UxGetValue(ScaleY), UxGetValue(ScaleZ), UxGetValue(ScaleZoom),
		UxGetText(StartXYZTW), UxGetText(IncXYZTW),
		atoi(UxGetText(FramesTW)), 
		UxGetText(SpecularTW), UxGetText(DiffuseTW), UxGetText(ShininessTW),
		atof(UxGetText(PixelsPerMMTW)));
	
	}
	UxGLShellContext = UxSaveCtx;
}

static	void	valueChangedCB_ScaleZ(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCGLShell             *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxGLShellContext;
	UxGLShellContext = UxContext =
			(_UxCGLShell *) UxGetContext( UxWidget );
	{
	if (RedrawOnDrag)
	    GLDraw( atoi(UxGetText(DecimationTW)),
		UxGetValue(ScaleX), UxGetValue(ScaleY), UxGetValue(ScaleZ), UxGetValue(ScaleZoom),
		UxGetText(StartXYZTW), UxGetText(IncXYZTW),
		atoi(UxGetText(FramesTW)), 
		UxGetText(SpecularTW), UxGetText(DiffuseTW), UxGetText(ShininessTW),
		atof(UxGetText(PixelsPerMMTW)));
	
	}
	UxGLShellContext = UxSaveCtx;
}

static	void	dragCB_ScaleZoom(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCGLShell             *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxGLShellContext;
	UxGLShellContext = UxContext =
			(_UxCGLShell *) UxGetContext( UxWidget );
	{
	if (RedrawOnDrag)
	    GLDraw( atoi(UxGetText(DecimationTW)),
		UxGetValue(ScaleX), UxGetValue(ScaleY), UxGetValue(ScaleZ), UxGetValue(ScaleZoom),
		UxGetText(StartXYZTW), UxGetText(IncXYZTW),
		atoi(UxGetText(FramesTW)), 
		UxGetText(SpecularTW), UxGetText(DiffuseTW), UxGetText(ShininessTW),
		atof(UxGetText(PixelsPerMMTW)));
	
	}
	UxGLShellContext = UxSaveCtx;
}

static	void	valueChangedCB_ScaleZoom(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCGLShell             *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxGLShellContext;
	UxGLShellContext = UxContext =
			(_UxCGLShell *) UxGetContext( UxWidget );
	{
	if (RedrawOnDrag)
	    GLDraw( atoi(UxGetText(DecimationTW)),
		UxGetValue(ScaleX), UxGetValue(ScaleY), UxGetValue(ScaleZ), UxGetValue(ScaleZoom),
		UxGetText(StartXYZTW), UxGetText(IncXYZTW),
		atoi(UxGetText(FramesTW)), 
		UxGetText(SpecularTW), UxGetText(DiffuseTW), UxGetText(ShininessTW),
		atof(UxGetText(PixelsPerMMTW)));
	
	}
	UxGLShellContext = UxSaveCtx;
}

static	void	valueChangedCB_TBG1(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCGLShell             *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxGLShellContext;
	UxGLShellContext = UxContext =
			(_UxCGLShell *) UxGetContext( UxWidget );
	{
	if (IsTrue(UxGetSet(UxThisWidget)))
		RedrawOnDrag = B_TRUE;
	else
		RedrawOnDrag = B_FALSE;
	
	}
	UxGLShellContext = UxSaveCtx;
}

static	void	activateCB_menu14_p1_b1(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCGLShell             *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxGLShellContext;
	UxGLShellContext = UxContext =
			(_UxCGLShell *) UxGetContext( UxWidget );
	{GLSetObjectType(SHADEDGEOMETRYOBJ);}
	UxGLShellContext = UxSaveCtx;
}

static	void	activateCB_menu14_p1_b2(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCGLShell             *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxGLShellContext;
	UxGLShellContext = UxContext =
			(_UxCGLShell *) UxGetContext( UxWidget );
	{GLSetObjectType(OVERLAYOBJ);}
	UxGLShellContext = UxSaveCtx;
}

static	void	activateCB_LoadGeomPBG(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCGLShell             *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxGLShellContext;
	UxGLShellContext = UxContext =
			(_UxCGLShell *) UxGetContext( UxWidget );
	{
	HGCursor(B_TRUE);
	LoadGLGeometry(atof(UxGetText(PixelsPerMMTW)));
	HGCursor(B_FALSE);
	}
	UxGLShellContext = UxSaveCtx;
}

static	void	activateCB_RedrawPBG(
			Widget wgt, 
			XtPointer cd, 
			XtPointer cb)
{
	_UxCGLShell             *UxSaveCtx, *UxContext;
	Widget                  UxWidget = wgt;
	XtPointer               UxClientData = cd;
	XtPointer               UxCallbackArg = cb;

	UxSaveCtx = UxGLShellContext;
	UxGLShellContext = UxContext =
			(_UxCGLShell *) UxGetContext( UxWidget );
	{
	HGCursor(B_TRUE);
	GLDraw( atoi(UxGetText(DecimationTW)),
		UxGetValue(ScaleX), UxGetValue(ScaleY), UxGetValue(ScaleZ), UxGetValue(ScaleZoom),
		UxGetText(StartXYZTW), UxGetText(IncXYZTW),
		atoi(UxGetText(FramesTW)), 
		UxGetText(SpecularTW), UxGetText(DiffuseTW), UxGetText(ShininessTW),
		atof(UxGetText(PixelsPerMMTW)));
	HGCursor(B_FALSE);
	
	}
	UxGLShellContext = UxSaveCtx;
}

/*******************************************************************************
       The 'build_' function creates all the widgets
       using the resource values specified in the Property Editor.
*******************************************************************************/

static Widget	_Uxbuild_GLShell(void)
{
	MrmType		_UxDummyClass;
	Cardinal	_UxStatus;


	/* Creation of GLShell */
	GLShell = XtVaCreatePopupShell( "GLShell",
			applicationShellWidgetClass,
			UxTopLevel,
			XmNx, 40,
			XmNy, 68,
			XmNwidth, 1200,
			XmNheight, 520,
			XmNallowShellResize, B_FALSE,
			RES_CONVERT( XmNbackground, "BlueViolet" ),
			XmNdefaultFontList, UxConvertFontList( "-adobe-helvetica-bold-r-normal--14-100-100-100-p-82-iso8859-1" ),
			XmNiconName, "GL",
			XmNtitle, "GL Window",
			NULL );
	XtAddCallback( GLShell, XmNdestroyCallback,
		(XtCallbackProc) destroyCB_GLShell,
		(XtPointer) UxGLShellContext );
	XtAddCallback( GLShell, XmNpopdownCallback,
		(XtCallbackProc) popdownCB_GLShell,
		(XtPointer) UxGLShellContext );
	XtAddCallback( GLShell, XmNpopupCallback,
		(XtCallbackProc) popupCB_GLShell,
		(XtPointer) UxGLShellContext );

	UxPutContext( GLShell, (char *) UxGLShellContext );

	form3 = NULL;
	_UxStatus = MrmFetchWidget( hierarchy,
					"form3",
					GLShell,
					&form3,
					&_UxDummyClass );
	if ( _UxStatus != MrmSUCCESS )
	{
		UxMrmFetchError( hierarchy, "form3",
				GLShell, _UxStatus );
		return( (Widget) NULL );
	}

	XtManageChild( form3 );

	UxPutContext( form3, (char *) UxGLShellContext );
	frame11 = XtNameToWidget( form3, "*frame11" );
	UxPutContext( frame11, (char *) UxGLShellContext );
	drawingArea1 = XtNameToWidget( frame11, "*drawingArea1" );
	UxPutContext( drawingArea1, (char *) UxGLShellContext );
	separatorGadget3 = XtNameToWidget( form3, "*separatorGadget3" );
	UxPutContext( separatorGadget3, (char *) UxGLShellContext );
	labelGadget10 = XtNameToWidget( form3, "*labelGadget10" );
	UxPutContext( labelGadget10, (char *) UxGLShellContext );
	labelGadget121 = XtNameToWidget( form3, "*labelGadget121" );
	UxPutContext( labelGadget121, (char *) UxGLShellContext );
	labelGadget122 = XtNameToWidget( form3, "*labelGadget122" );
	UxPutContext( labelGadget122, (char *) UxGLShellContext );
	BuildPolyPBG = XtNameToWidget( form3, "*BuildPolyPBG" );
	UxPutContext( BuildPolyPBG, (char *) UxGLShellContext );
	SavePBG = XtNameToWidget( form3, "*SavePBG" );
	UxPutContext( SavePBG, (char *) UxGLShellContext );
	GLResetPBG = XtNameToWidget( form3, "*GLResetPBG" );
	UxPutContext( GLResetPBG, (char *) UxGLShellContext );
	separatorGadget16 = XtNameToWidget( form3, "*separatorGadget16" );
	UxPutContext( separatorGadget16, (char *) UxGLShellContext );
	labelGadget124 = XtNameToWidget( form3, "*labelGadget124" );
	UxPutContext( labelGadget124, (char *) UxGLShellContext );
	PixelsPerMMTW = XtNameToWidget( form3, "*PixelsPerMMTW" );
	UxPutContext( PixelsPerMMTW, (char *) UxGLShellContext );
	labelGadget125 = XtNameToWidget( form3, "*labelGadget125" );
	UxPutContext( labelGadget125, (char *) UxGLShellContext );
	SpecularTW = XtNameToWidget( form3, "*SpecularTW" );
	UxPutContext( SpecularTW, (char *) UxGLShellContext );
	labelGadget126 = XtNameToWidget( form3, "*labelGadget126" );
	UxPutContext( labelGadget126, (char *) UxGLShellContext );
	DiffuseTW = XtNameToWidget( form3, "*DiffuseTW" );
	UxPutContext( DiffuseTW, (char *) UxGLShellContext );
	labelGadget127 = XtNameToWidget( form3, "*labelGadget127" );
	UxPutContext( labelGadget127, (char *) UxGLShellContext );
	ShininessTW = XtNameToWidget( form3, "*ShininessTW" );
	UxPutContext( ShininessTW, (char *) UxGLShellContext );
	DecimationTW = XtNameToWidget( form3, "*DecimationTW" );
	UxPutContext( DecimationTW, (char *) UxGLShellContext );
	labelGadget128 = XtNameToWidget( form3, "*labelGadget128" );
	UxPutContext( labelGadget128, (char *) UxGLShellContext );
	separatorGadget17 = XtNameToWidget( form3, "*separatorGadget17" );
	UxPutContext( separatorGadget17, (char *) UxGLShellContext );
	FramesTW = XtNameToWidget( form3, "*FramesTW" );
	UxPutContext( FramesTW, (char *) UxGLShellContext );
	labelGadget129 = XtNameToWidget( form3, "*labelGadget129" );
	UxPutContext( labelGadget129, (char *) UxGLShellContext );
	separatorGadget18 = XtNameToWidget( form3, "*separatorGadget18" );
	UxPutContext( separatorGadget18, (char *) UxGLShellContext );
	labelGadget130 = XtNameToWidget( form3, "*labelGadget130" );
	UxPutContext( labelGadget130, (char *) UxGLShellContext );
	labelGadget131 = XtNameToWidget( form3, "*labelGadget131" );
	UxPutContext( labelGadget131, (char *) UxGLShellContext );
	labelGadget132 = XtNameToWidget( form3, "*labelGadget132" );
	UxPutContext( labelGadget132, (char *) UxGLShellContext );
	labelGadget135 = XtNameToWidget( form3, "*labelGadget135" );
	UxPutContext( labelGadget135, (char *) UxGLShellContext );
	StartXYZTW = XtNameToWidget( form3, "*StartXYZTW" );
	UxPutContext( StartXYZTW, (char *) UxGLShellContext );
	labelGadget136 = XtNameToWidget( form3, "*labelGadget136" );
	UxPutContext( labelGadget136, (char *) UxGLShellContext );
	IncXYZTW = XtNameToWidget( form3, "*IncXYZTW" );
	UxPutContext( IncXYZTW, (char *) UxGLShellContext );
	GeneratePBG = XtNameToWidget( form3, "*GeneratePBG" );
	UxPutContext( GeneratePBG, (char *) UxGLShellContext );
	LoadFieldPBG = XtNameToWidget( form3, "*LoadFieldPBG" );
	UxPutContext( LoadFieldPBG, (char *) UxGLShellContext );
	RecFramePBG = XtNameToWidget( form3, "*RecFramePBG" );
	UxPutContext( RecFramePBG, (char *) UxGLShellContext );
	GenMoviePBG = XtNameToWidget( form3, "*GenMoviePBG" );
	UxPutContext( GenMoviePBG, (char *) UxGLShellContext );
	labelGadget60 = XtNameToWidget( form3, "*labelGadget60" );
	UxPutContext( labelGadget60, (char *) UxGLShellContext );
	separatorGadget22 = XtNameToWidget( form3, "*separatorGadget22" );
	UxPutContext( separatorGadget22, (char *) UxGLShellContext );
	ScaleX = XtNameToWidget( form3, "*ScaleX" );
	UxPutContext( ScaleX, (char *) UxGLShellContext );
	ScaleY = XtNameToWidget( form3, "*ScaleY" );
	UxPutContext( ScaleY, (char *) UxGLShellContext );
	ScaleZ = XtNameToWidget( form3, "*ScaleZ" );
	UxPutContext( ScaleZ, (char *) UxGLShellContext );
	labelGadget123 = XtNameToWidget( form3, "*labelGadget123" );
	UxPutContext( labelGadget123, (char *) UxGLShellContext );
	ScaleZoom = XtNameToWidget( form3, "*ScaleZoom" );
	UxPutContext( ScaleZoom, (char *) UxGLShellContext );
	toggleButtonGadget1 = XtNameToWidget( form3, "*toggleButtonGadget1" );
	UxPutContext( toggleButtonGadget1, (char *) UxGLShellContext );
	menu14 = XtNameToWidget( form3, "*menu14" );
	UxPutContext( menu14, (char *) UxGLShellContext );
	menu14_p1 = XtNameToWidget( form3, "*menu14_p1" );
	UxPutContext( menu14_p1, (char *) UxGLShellContext );
	menu14_p1_b1 = XtNameToWidget( menu14_p1, "*menu14_p1_b1" );
	UxPutContext( menu14_p1_b1, (char *) UxGLShellContext );
	menu14_p1_b2 = XtNameToWidget( menu14_p1, "*menu14_p1_b2" );
	UxPutContext( menu14_p1_b2, (char *) UxGLShellContext );
	LoadGeomPBG = XtNameToWidget( form3, "*LoadGeomPBG" );
	UxPutContext( LoadGeomPBG, (char *) UxGLShellContext );
	RedrawPBG = XtNameToWidget( form3, "*RedrawPBG" );
	UxPutContext( RedrawPBG, (char *) UxGLShellContext );

	XtAddCallback( GLShell, XmNdestroyCallback,
		(XtCallbackProc) UxDestroyContextCB,
		(XtPointer) UxGLShellContext);

	return ( GLShell );
}
#endif /* defined (GL_API) */

/*******************************************************************************
       The following is the 'Interface function' which is the
       external entry point for creating this interface.
       This function should be called from your application or from
       a callback function.
*******************************************************************************/

Widget	create_GLShell(void)
{
	Widget                  rtrn;
#if defined (GL_API)
	_UxCGLShell             *UxContext;
	static int		_Uxinit = 0;
#endif /* defined (GL_API) */

	rtrn=(Widget)NULL;
#if defined (GL_API)
	UxGLShellContext = UxContext =
		(_UxCGLShell *) UxNewContext( sizeof(_UxCGLShell), B_FALSE );


	if ( ! _Uxinit )
	{
		static MrmRegisterArg	_UxMrmNames[] = {
			{ "destroyCB_GLShell", 
				(XtPointer) destroyCB_GLShell },
			{ "popdownCB_GLShell", 
				(XtPointer) popdownCB_GLShell },
			{ "popupCB_GLShell", 
				(XtPointer) popupCB_GLShell },
			{ "activateCB_BuildPolyPBG", 
				(XtPointer) activateCB_BuildPolyPBG },
			{ "activateCB_SavePBG", 
				(XtPointer) activateCB_SavePBG },
			{ "activateCB_SaveFramePBG", 
				(XtPointer) activateCB_SaveFramePBG },
			{ "activateCB_GLResetPBG", 
				(XtPointer) activateCB_GLResetPBG },
			{ "activateCB_GeneratePBG", 
				(XtPointer) activateCB_GeneratePBG },
			{ "activateCB_LoadFieldPBG", 
				(XtPointer) activateCB_LoadFieldPBG },
			{ "activateCB_RecFramePBG", 
				(XtPointer) activateCB_RecFramePBG },
			{ "activateCB_GenMoviePBG", 
				(XtPointer) activateCB_GenMoviePBG },
			{ "dragCB_ScaleX", 
				(XtPointer) dragCB_ScaleX },
			{ "valueChangedCB_ScaleX", 
				(XtPointer) valueChangedCB_ScaleX },
			{ "dragCB_ScaleY", 
				(XtPointer) dragCB_ScaleY },
			{ "valueChangedCB_ScaleY", 
				(XtPointer) valueChangedCB_ScaleY },
			{ "dragCB_ScaleZ", 
				(XtPointer) dragCB_ScaleZ },
			{ "valueChangedCB_ScaleZ", 
				(XtPointer) valueChangedCB_ScaleZ },
			{ "dragCB_ScaleZoom", 
				(XtPointer) dragCB_ScaleZoom },
			{ "valueChangedCB_ScaleZoom", 
				(XtPointer) valueChangedCB_ScaleZoom },
			{ "valueChangedCB_TBG1", 
				(XtPointer) valueChangedCB_TBG1 },
			{ "activateCB_menu14_p1_b1", 
				(XtPointer) activateCB_menu14_p1_b1 },
			{ "activateCB_menu14_p1_b2", 
				(XtPointer) activateCB_menu14_p1_b2 },
			{ "activateCB_LoadGeomPBG", 
				(XtPointer) activateCB_LoadGeomPBG },
			{ "activateCB_RedrawPBG", 
				(XtPointer) activateCB_RedrawPBG }};

		MrmRegisterNamesInHierarchy(hierarchy, _UxMrmNames,
					    XtNumber(_UxMrmNames) );

		_Uxinit = 1;
	}

	rtrn = _Uxbuild_GLShell();

	GLWindowDAW = drawingArea1;
	GLShellCreated = B_TRUE;
	RedrawOnDrag = B_FALSE;
#endif /* defined (GL_API) */
	
	/* return */
	return(rtrn);
}

/*******************************************************************************
       END OF FILE
*******************************************************************************/

