/***********************************************************************
*
*  Name:          HistogramShell.h
*
*  Author:        Paul Charette
*
*  Last Modified: 26 Nov 1994
*
*  Purpose:       function prototyping
*
***********************************************************************/
#ifndef HistogramShell_h
#define HistogramShell_h
extern swidget	create_HistogramShell(void);
extern void DrawHistograms(double RawMin, double RawMax,
			   double ProcMin, double ProcMax);
extern void LoadProcessedColorBar(void);
#endif
