/***********************************************************************
*
*  Name:          unima.h
*
*  Author:        Paul Charette
*
*  Last Modified: 3 March 1997
*
*  Purpose:       function prototyping
*
***********************************************************************/
#ifndef unima_externs_h
#define unima_externs_h
extern double LCRTempSample(void);
extern void LCRValOut(int v);
extern void LightSourceSelect(int v);
extern void UnimaInit(void);
extern void UnimaOff(void);
extern void UnimaPSGalvosOut(int NewVal);
extern int  UnimaSample(void);
extern void UnimaVOut(void);
#endif






















