
/*******************************************************************************
       HistogramShell.h
       This header file is included by HistogramShell.c

*******************************************************************************/

#ifndef	_HISTOGRAMSHELL_INCLUDED
#define	_HISTOGRAMSHELL_INCLUDED


#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <Mrm/MrmPublic.h>
#include <Xm/MenuShell.h>
#include "UxXt.h"

#include <Xm/Scale.h>
#include <Xm/SeparatoG.h>
#include <Xm/PushBG.h>
#include <Xm/LabelG.h>
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
	Widget	Uxform4;
	Widget	Uxframe4;
	Widget	UxRawHistogramDA;
	Widget	Uxframe5;
	Widget	UxProcessedHistogramDA;
	Widget	UxlabelGadget7;
	Widget	UxlabelGadget8;
	Widget	UxProcMaxLG;
	Widget	UxProcMinLG;
	Widget	UxRawMaxLG;
	Widget	UxRawMinLG;
	Widget	Uxframe6;
	Widget	Uxframe10;
	Widget	UxpushButtonGadget5;
	Widget	UxseparatorGadget5;
	Widget	UxRawInfoLG;
	Widget	UxProcInfoLG;
	Widget	UxseparatorGadget6;
	Widget	UxMinScaleW;
	Widget	UxMaxScaleW;
	Widget	UxseparatorGadget14;
	Widget	UxRawSDLG;
	Widget	UxRawMeanLG;
	Widget	UxProcSDLG;
	Widget	UxProcMeanLG;
	int	Uxhi;
	int	Uxhj;
} _UxCHistogramShell;

#ifdef CONTEXT_MACRO_ACCESS
static _UxCHistogramShell      *UxHistogramShellContext;
#define form4                   UxHistogramShellContext->Uxform4
#define frame4                  UxHistogramShellContext->Uxframe4
#define RawHistogramDA          UxHistogramShellContext->UxRawHistogramDA
#define frame5                  UxHistogramShellContext->Uxframe5
#define ProcessedHistogramDA    UxHistogramShellContext->UxProcessedHistogramDA
#define labelGadget7            UxHistogramShellContext->UxlabelGadget7
#define labelGadget8            UxHistogramShellContext->UxlabelGadget8
#define ProcMaxLG               UxHistogramShellContext->UxProcMaxLG
#define ProcMinLG               UxHistogramShellContext->UxProcMinLG
#define RawMaxLG                UxHistogramShellContext->UxRawMaxLG
#define RawMinLG                UxHistogramShellContext->UxRawMinLG
#define frame6                  UxHistogramShellContext->Uxframe6
#define frame10                 UxHistogramShellContext->Uxframe10
#define pushButtonGadget5       UxHistogramShellContext->UxpushButtonGadget5
#define separatorGadget5        UxHistogramShellContext->UxseparatorGadget5
#define RawInfoLG               UxHistogramShellContext->UxRawInfoLG
#define ProcInfoLG              UxHistogramShellContext->UxProcInfoLG
#define separatorGadget6        UxHistogramShellContext->UxseparatorGadget6
#define MinScaleW               UxHistogramShellContext->UxMinScaleW
#define MaxScaleW               UxHistogramShellContext->UxMaxScaleW
#define separatorGadget14       UxHistogramShellContext->UxseparatorGadget14
#define RawSDLG                 UxHistogramShellContext->UxRawSDLG
#define RawMeanLG               UxHistogramShellContext->UxRawMeanLG
#define ProcSDLG                UxHistogramShellContext->UxProcSDLG
#define ProcMeanLG              UxHistogramShellContext->UxProcMeanLG
#define hi                      UxHistogramShellContext->Uxhi
#define hj                      UxHistogramShellContext->Uxhj

#endif /* CONTEXT_MACRO_ACCESS */

extern Widget	HistogramShell;
extern Widget	RawColorBarDA;
extern Widget	ProcessedColorBarDA;

/*******************************************************************************
       Declarations of global functions.
*******************************************************************************/

Widget	create_HistogramShell( void );

#endif	/* _HISTOGRAMSHELL_INCLUDED */
