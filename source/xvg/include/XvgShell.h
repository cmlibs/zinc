/***********************************************************************
*
*  Name:          XvgShell.h
*
*  Author:        Paul Charette
*
*  Last Modified: 26 Nov 1994
*
*  Purpose:       function prototyping
*
***********************************************************************/
#ifndef XvgShell__h
#define XvgShell__h
extern void    ClearStackMenu(void);
extern void    CMOverlay(swidget DrawingArea);
extern void    ContourOverlay(swidget DrawingArea);
extern void    DispFieldOverlay(swidget DrawingArea);
extern void    CreateProcImage(void);
extern void    CreateRawImage(void);
extern swidget create_XvgShell(int _Uxargc, char **_Uxargv);
extern void    DrawBluePoint(int x, int y);
extern void    DrawKnots(swidget DrawingArea);
extern void    DrawLine(int xs, int y, int xe);
extern void    DrawRedPoint(int x, int y);
extern void    DrawRegionLabels(swidget DrawingArea);
extern void    DrawRegionNormals(swidget DrawingArea);
extern boolean_t DumpImage(void);
extern void    ErrorMsg(char *p);
extern void    GrabbingState(boolean_t flag);
extern void    HGCursor(boolean_t state);
extern void    InfoMsg(char *p);
extern void    InfoMsgPopdownOKOn(void);
extern void    InfoMsgPopupOKOff(void);
extern void    InfoMsgUpdate(char *p);
extern void    InitHierarchy(void);
extern void    InitProcBuffers(void);
extern void    init_state(swidget MainInterfaceSwidget,
		int Argc, char **Argv);
extern void    LoadArctanradiusImgMenu(void);
extern void    LoadBRYGMBVWCells(void);
extern boolean_t LoadFile(char *name);
extern void    LoadGammaGreyLevelCells(void);
extern void    LoadGreyLevelCells(int wfactor);
extern void    LoadIPOps(char *fname);
extern void    LoadMaskMenu(void);
extern void    LoadOverlayFunction(void (f)());
extern void    LoadPlaneAngleMenu(void);
extern void    LoadPStepImgMenu(int indx);
extern boolean_t parse_args(int nargs, char *args[]);
extern void    PeekNoInteractive(int ac, char **av);
extern void    PopImageMenu(void);
extern void    PushImageMenu(void);
extern void    RemoveOverlayFunction(void (f)());
extern void    ResizeProcessedImageWidgets(boolean_t force);
extern void    ResizeRawImageWidgets(boolean_t force);
extern void    RotateImage(void);
extern void    SaveIPOps(char *fname);
extern void    ScreenSaverOff(void);
extern void    ScreenSaverOn(void);
extern void    ShowImage(double *p);
extern void    StrainOverlay(swidget DrawingArea);
extern void    SwapMenu(void);
extern void    TransposeImage(void);
extern void    UpdateStackLabel(int x);
extern void    XvgInitVariables(void);
#endif





