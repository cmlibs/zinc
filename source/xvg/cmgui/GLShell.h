
/*******************************************************************************
       GLShell.h
       This header file is included by GLShell.c

*******************************************************************************/

#ifndef	_GLSHELL_INCLUDED
#define	_GLSHELL_INCLUDED


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

/*******************************************************************************
       The definition of the context structure:
       If you create multiple copies of your interface, the context
       structure ensures that your callbacks use the variables for the
       correct copy.

       For each swidget in the interface, each argument to the Interface
       function, and each variable in the Interface Specific section of the
       Declarations Editor, there is an entry in the context structure.
       and a #define.  The #define makes the variable name refer to the
       corresponding entry in the context structure.
*******************************************************************************/

typedef	struct
{
	Widget	Uxform3;
	Widget	Uxframe11;
	Widget	UxdrawingArea1;
	Widget	UxseparatorGadget3;
	Widget	UxlabelGadget10;
	Widget	UxlabelGadget121;
	Widget	UxlabelGadget122;
	Widget	UxBuildPolyPBG;
	Widget	UxSavePBG;
	Widget	UxSaveFramePBG;
	Widget	UxGLResetPBG;
	Widget	UxseparatorGadget16;
	Widget	UxlabelGadget124;
	Widget	UxlabelGadget125;
	Widget	UxlabelGadget126;
	Widget	UxlabelGadget127;
	Widget	UxlabelGadget128;
	Widget	UxseparatorGadget17;
	Widget	UxlabelGadget129;
	Widget	UxseparatorGadget18;
	Widget	UxlabelGadget130;
	Widget	UxlabelGadget131;
	Widget	UxlabelGadget132;
	Widget	UxlabelGadget135;
	Widget	UxlabelGadget136;
	Widget	UxGeneratePBG;
	Widget	UxLoadFieldPBG;
	Widget	UxRecFramePBG;
	Widget	UxGenMoviePBG;
	Widget	UxlabelGadget60;
	Widget	UxseparatorGadget22;
	Widget	UxlabelGadget123;
	Widget	UxtoggleButtonGadget1;
	Widget	Uxmenu14;
	Widget	Uxmenu14_p1;
	Widget	Uxmenu14_p1_b1;
	Widget	Uxmenu14_p1_b2;
	Widget	UxLoadGeomPBG;
	Widget	UxRedrawPBG;
} _UxCGLShell;

#ifdef CONTEXT_MACRO_ACCESS
static _UxCGLShell             *UxGLShellContext;
#define form3                   UxGLShellContext->Uxform3
#define frame11                 UxGLShellContext->Uxframe11
#define drawingArea1            UxGLShellContext->UxdrawingArea1
#define separatorGadget3        UxGLShellContext->UxseparatorGadget3
#define labelGadget10           UxGLShellContext->UxlabelGadget10
#define labelGadget121          UxGLShellContext->UxlabelGadget121
#define labelGadget122          UxGLShellContext->UxlabelGadget122
#define BuildPolyPBG            UxGLShellContext->UxBuildPolyPBG
#define SavePBG                 UxGLShellContext->UxSavePBG
#define SaveFramePBG            UxGLShellContext->UxSaveFramePBG
#define GLResetPBG              UxGLShellContext->UxGLResetPBG
#define separatorGadget16       UxGLShellContext->UxseparatorGadget16
#define labelGadget124          UxGLShellContext->UxlabelGadget124
#define labelGadget125          UxGLShellContext->UxlabelGadget125
#define labelGadget126          UxGLShellContext->UxlabelGadget126
#define labelGadget127          UxGLShellContext->UxlabelGadget127
#define labelGadget128          UxGLShellContext->UxlabelGadget128
#define separatorGadget17       UxGLShellContext->UxseparatorGadget17
#define labelGadget129          UxGLShellContext->UxlabelGadget129
#define separatorGadget18       UxGLShellContext->UxseparatorGadget18
#define labelGadget130          UxGLShellContext->UxlabelGadget130
#define labelGadget131          UxGLShellContext->UxlabelGadget131
#define labelGadget132          UxGLShellContext->UxlabelGadget132
#define labelGadget135          UxGLShellContext->UxlabelGadget135
#define labelGadget136          UxGLShellContext->UxlabelGadget136
#define GeneratePBG             UxGLShellContext->UxGeneratePBG
#define LoadFieldPBG            UxGLShellContext->UxLoadFieldPBG
#define RecFramePBG             UxGLShellContext->UxRecFramePBG
#define GenMoviePBG             UxGLShellContext->UxGenMoviePBG
#define labelGadget60           UxGLShellContext->UxlabelGadget60
#define separatorGadget22       UxGLShellContext->UxseparatorGadget22
#define labelGadget123          UxGLShellContext->UxlabelGadget123
#define toggleButtonGadget1     UxGLShellContext->UxtoggleButtonGadget1
#define menu14                  UxGLShellContext->Uxmenu14
#define menu14_p1               UxGLShellContext->Uxmenu14_p1
#define menu14_p1_b1            UxGLShellContext->Uxmenu14_p1_b1
#define menu14_p1_b2            UxGLShellContext->Uxmenu14_p1_b2
#define LoadGeomPBG             UxGLShellContext->UxLoadGeomPBG
#define RedrawPBG               UxGLShellContext->UxRedrawPBG

#endif /* CONTEXT_MACRO_ACCESS */

extern Widget	GLShell;
extern Widget	PixelsPerMMTW;
extern Widget	SpecularTW;
extern Widget	DiffuseTW;
extern Widget	ShininessTW;
extern Widget	DecimationTW;
extern Widget	FramesTW;
extern Widget	StartXYZTW;
extern Widget	IncXYZTW;
extern Widget	ScaleX;
extern Widget	ScaleY;
extern Widget	ScaleZ;
extern Widget	ScaleZoom;

/*******************************************************************************
       Declarations of global functions.
*******************************************************************************/

Widget	create_GLShell( void );

#endif	/* _GLSHELL_INCLUDED */
