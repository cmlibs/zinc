/***************************************************************************
*
*  Name:          XvgGlobals.h
*
*  Author:        Paul Charette
*
*  Last Modified: 18 April 1998
*
*  Purpose:       Global or extern declarations for all files.
*                 If the compile time variable MAIN_PROGRAM is defined,
*                 this indicates that the variables are to be declared 
*                 as global and initialized if required.
*
****************************************************************************/
#ifndef XvgGlobals_h
#define XvgGlobals__h

#include <stdlib.h>
#include <math.h>
#include <values.h>
#include <X11/XWDFile.h>
#include <Mrm/MrmPublic.h>
#if defined (SGI)
#include <vca_xvg.h>
#endif /* defined (SGI) */
#ifdef ALL_INTERFACES
#include "UDACCalibrationData.h"
#endif

/* SGI compatibility */
#ifndef SGI
typedef int boolean_t;
#define B_TRUE  1
#define B_FALSE 0
#endif

/***************************************************************************/
/*                                                                         */
/*                           Magic Numbers                                 */
/*              (fixed for a given optical configuration)                  */
/*                                                                         */
/***************************************************************************/

/* Distance in meters between pixels on the membrane      */
#define PIXEL_SPACING           40.611e-6

/* VCAA dynamic range for the AD when left justifed in a char */
#define VCAA_RED_DR             255

/***************************************************************************/
/*                                                                         */
/*                                defines                                  */
/*                                                                         */
/***************************************************************************/

/* macros */
#ifdef DEBUG
  #define DEBUGS(str)     fprintf(stderr, "%s\n", str);
  #define DEBUGSIV(str,v) fprintf(stderr, "%s %d\n", str, v);
  #define DEBUGSFV(str,v) fprintf(stderr, "%s %g\n", str, v)
#else
  #define DEBUGS(str)
  #define DEBUGSIV(str,v)
  #define DEBUGSFV(str,v)
#endif

/* SGI nag defs */
#ifdef SGI

#define Int32   unsigned long
#define Float64 double
#define Float32 float

#define c06fuf c06fuf_
#define c06gcf c06gcf_
#define e02aef e02aef_
#define e02caf e02caf_
#define e02cbf e02cbf_
#define e02daf e02daf_
#define e02dcf e02dcf_
#define e02ddf e02ddf_
#define e02dff e02dff_
#define e02zaf e02zaf_
#define e04bbf e04bbf_
#define e04gef e04gef_
#define e04ucf e04ucf_
#define e04uef e04uef_
#define e04udm e04udm_
#define f04amf f04amf_
#define f04jaf f04jaf_
#define f04atf f04atf_
#define f04faf f04faf_
#define lsfun2 lsfun2_

#define strcmp XvgStrcmp

typedef enum {
    GLXcolorIndexSingleBuffer,
    GLXcolorIndexDoubleBuffer,
    GLXrgbSingleBuffer,
    GLXrgbDoubleBuffer
} GLXWindowType;
#endif

/* general system symbols */
#define CBUF_SIZE               1024
#define IPOPNPARMS              4
#define N640X480                307200
#define PIOVERTWO               1.570796327
#define TWOPI                   6.283185308
#define SCROLL_BAR_THICKNESS    35
#define SERVER_LUT_LEN          96
#define SLICE_WINDOW_WIDTH      80
#define IMAGESTACKSIZE          10
#define IPOPREGISTEREDLISTMAX   500
#define REGIONLISTSIZE          1000
#define MAXNEIGHBOURS           1000
#define OVERLAYFUNCTIONSMAX     10
#define NPAGEDLUTCELLS          192
#define COLORBARWIDTH           25
#define COLORBARHEIGHT          256
#define CONTOURNMAX             (640*480)
#define STARTING_IMAGE_HEIGHT   200
#define STARTING_IMAGE_WIDTH    300
#define GLMOVIEFRAMESNMAX       1000
#define MAXELEMENTS             100
#define HISTOGRAM_BINS          256
#define HISTOGRAM_HEIGHT        100
#define PPORT_DATA              0x0078
#define TEXT_OVERLAY_WIDTH      175
#define TEXT_OVERLAY_HEIGHT     35

#define HIST_SCALE_MIN          0
#define HIST_SCALE_MAX          100
#define INSTRINGSIZEMAX         1024

/* macros */
#define IsTrue(p) (strcmp((char *) p, "true") == 0 ? B_TRUE : B_FALSE)

/***************************************************************************/
/*                                                                         */
/* Select global or extern qualifier for the variables declarations        */
/* and assign initial values only in the case of a global declaration.     */
/*                                                                         */
/***************************************************************************/
#ifdef MAIN_PROGRAM
    #define DTYPE
#else
    #define DTYPE extern
#endif


/***************************************************************************/
/*                                                                         */
/*                                Data types                               */
/*                                                                         */
/***************************************************************************/
/* GL object types */
enum {SHADEDGEOMETRYOBJ, OVERLAYOBJ};

/* Movie types */
enum {XMOVIE, GLMOVIE};

/* LCR states */
enum {LCROTHER, LCRHALFWAVE, LCR0WAVE};

/* FFT identifiers */
enum {HANNINGWINDOW, TRIANGULARWINDOW, HALFSIZETRIANGULARWINDOW, NOWINDOW};
enum {FFTMAG, FFTLOGMAG, FFTPHASE};

/* grabbing identifiers */
enum {GRAYLEVELGRAB, REDGRAB, COLORGRAB, THREESTEPGRAB, FOURSTEPGRAB,
	FOUR_ASTEPGRAB, FIVESTEPGRAB, DITHEREDCOLORGRAB, NTSC, RGB,
	GREENGRAB};

/* IPOps parm data types */
enum {DOUBLE_DT, INTEGER_DT, STRING_DT, DOUBLE_ARRAY_DT, NONE_DT};

/* file selection dialog action definitions */
enum {LOADFILEACTION, LOADIPOPSACTION, LOADMOVIEACTION, SAVEDATAACTION,
	SAVEIPOPSACTION, SAVEHISTOGRAMACTION, LOADGLMOVIEACTION,
	SAVEGLMOVIEACTION, SAVEGLIMAGEACTION, GENGLMOVIEACTION,
        SAVEGLFRAMEACTION};
enum {MATLAB_FF, TIFFCOLOR_FF, TIFFGREYLEVEL_FF, IGS_FF, PHASESTEPRAW_FF,
	BSPLINE_FF, PX_FF, XWD_FF, LASERSCAN_FF, PX8_FF, PX16_FF, PX32_FF,
	MATLAB_16BIT_FF};

/* prompt dialog actions definitions */
enum {VCRIMAGESPERSECACTION, VCRMOVIELOOPACTION,
	VCR_SINGLE_IMAGESPERSECACTION, GALVOS_CYCLE_ACTION,
	REBUILDATANLUTACTION, MPERPIXELSACTION, LOADPSTEPIMAGEACTION,
	SELECTMOVIEFILEINDEXACTION, DEFINEMESHACTION,
	NOACTION};

/* IPOperations menu category names */
/* "BuildEditingMenu(void)" in the file "IPOperationsShellAux.c"      */
enum {
  FILEIOOPS,
  STACKOPS,
  GRABBINGOPS,
  DYNRANGEOPS,
  RPNOPS,
  BINARYOPS,
  MORPHOLOGICALOPS,
  FFTOPS,
  FILTEROPS,
  FITOPS,
  STEREOOPS,
  WRAPOPS,
  MISCOPS,
  VIDEOOPS,
  IPOPSMAX};


/***************************************************************************/
/*                                                                         */
/*                                Structures                               */
/*                                                                         */
/***************************************************************************/
typedef struct {
  double angle;
  double radius;
  double error;
} ATAN_LUT_ELEMENT;

typedef struct {
    int n;
    double *data;
} DOUBLE_ARRAY;

typedef struct {
    void (*f)(double *inpixels, double *outpixels,
	      int height, int width, void *ParmPtrs[]);
    char OpName[255];
    void *ParmPtrs[IPOPNPARMS];
    int ParmDataTypes[IPOPNPARMS];
    char ParmNames[IPOPNPARMS][255];
    int reps;
    boolean_t RepsAllowed, EdgeFixFlag;
    void *next;
    void *prev;
} IPOpListItem;

#ifndef NOINTRINSICS
typedef struct {
    char *name;
    Widget menuID;
} IPCategoryItem;
#endif
  
typedef struct {
    char OpName[255];
    int IPOpCategoryID;
    void (*f)(double *inpixels, double *outpixels,
	      int height, int width, void *ParmPtrs[]);
    int NParms;
    int ParmDataTypes[IPOPNPARMS];
    char ParmNames[IPOPNPARMS][255];
    boolean_t RepsAllowed, EdgeFixFlag;
} IPRegisteredOpListItem;
 
typedef struct {
    int height;
    int width;
    double *pixels;
} IMAGE_STACK_ELEMENT;

typedef struct {
    int x, y;
    double outval;
} CONTOUR_ELEMENT;

typedef struct {
    int x, y;
    int LineX0, LineY0;
    int LineX1, LineY1;
    double outval;
    double offset;
} NORMAL_ELEMENT;

typedef struct {
    double index;
    int NPixels;
    int CentroidX;
    int CentroidY;
    double offset;
    boolean_t OffsetSet;
    int NNeighbours;
    double *Neighbours;
    int *NeighbourIndicies;
    int NContourElements;
    CONTOUR_ELEMENT *ContourElements;
    int *NNormalElements;
    NORMAL_ELEMENT **NormalElements;
} REGION_LIST_ELEMENT;

typedef struct {
  int xmin, xmax, ymin, ymax;
} DISP_FIELD_ELEMENT;

typedef struct {
    int x, y;
    double ex, ey, exy;
    double emax, emin;
    double theta;
    double dxmax, dymax, ddxmax, ddymax;
    double dxmin, dymin, ddxmin, ddymin;
} STRAIN_ELEMENT;

typedef struct {
    CARD32 pixel B32;
    CARD16 red B16;
    CARD16 green B16;
    CARD16 blue B16;
    CARD8	flags;
    CARD8	pad;
} XWDColor_Def;


/***************************************************************************/
/*                                                                         */
/*                         X globals declarations                          */
/*                                                                         */
/***************************************************************************/
DTYPE Cursor HistogramCursor, hourglass, leftpointer, ProcessedImageCursor,
  RawImageCursor, xhair;
DTYPE GC gc, gc_BlueBlack, gc_GreenBlack, gc_RedBlack, gc_BlackWhite,
  gc_WhiteBlack, gc_Overlay, gc_YellowBlack, gc_Scratch, gc_xor, gc_Null;
DTYPE Colormap cmap;
DTYPE XFontStruct *LabelFont, *OverlayFont, *SmallFont;
DTYPE XColor GreyLevelCells[NPAGEDLUTCELLS],
  GammaGreyLevelCells[NPAGEDLUTCELLS], BRYGMBVMCells[NPAGEDLUTCELLS],
  UserCells[NPAGEDLUTCELLS], OverlayCells[128], *CurrentCells;

DTYPE XGCValues xgc;
DTYPE XImage *ColorBarImageXObj, *ProcessedImageXObj, *RawImageXObj,
  *ProcessedFTImageXObj, *RawFTImageXObj;
DTYPE XPoint *ProcessedPoints, *RawPoints, *KnotPoints, *ContourPoints;
DTYPE XRectangle ProcessedHistogramRectangles[HISTOGRAM_BINS],
  RawHistogramRectangles[HISTOGRAM_BINS];
DTYPE Widget GLWindowDAW;

/***************************************************************************/
/*                                                                         */
/*                        Data global declarations                         */
/*                                                                         */
/***************************************************************************/
#ifndef SGI
DTYPE struct vca_controls VcaControls;
#endif

DTYPE ATAN_LUT_ELEMENT atan_lut[2*(VCAA_RED_DR+1) - 1][2*(VCAA_RED_DR+1) - 1];

DTYPE IMAGE_STACK_ELEMENT ImageStack[IMAGESTACKSIZE];

DTYPE REGION_LIST_ELEMENT RegionList[REGIONLISTSIZE];

DTYPE STRAIN_ELEMENT *StrainElements;

DTYPE DISP_FIELD_ELEMENT *DispFieldElements;

DTYPE boolean_t Abort, AtanLutCreated, XvgUp, ColorBarOverlay, ComplexDataSet, 
  RedrawOnDrag, DisplayContourComputed, Monochrome, GLShellCreated,
  ImageUpdateFlag, SlicesProjectionsOn, ChildProcess, LogoFound, SGIImageHalfWidth,
#ifdef ALL_INTERFACES
  ADShellCreated, DAShellCreated, AddTwoPI, AddTwoPIOn, 
  DistanceFirstPointDone, DrawClip, DrawClipOn,
  ClipRegionGenerate, DrawPolygon, DrawPolygonOn, ComputeDistanceOn,
  PixelPickOn, GalvoControlShellCreated, SubTwoPI, SubTwoPIOn, 
  LCRShellCreated, UnimaInitialized,
#endif
  DisplayOnlyFlag, ErrorMsgWindowCreated, FixDynamicRange, EraserOn,
  GrabbingOn, HistogramShellCreated, GLWindowMapped, VCAAConnectionOpen,
  InteractiveFlag, InternalChecks, IPOperationsShellCreated, IPOpExecVerbose,
  LUTShellCreated, BSplineComputed, XMovieLoaded, MovieControlsShellCreated,
  PagedCmap, promptDialogUp, ProcessedImageShellCreated,XvgMM_MemoryCheck_Flag,
  RawImageShellCreated, NoNormalsOverlay, Recompress, TrueColorImage,
  SGIFBInitialized, VCRInitDone, VCAWriteOn, VCRRecordOn, VerboseFlag,
  WhiteLightImaging, DrawMesgEnabled, Convert_RGB_TIFF_To_Graylevel;

DTYPE char *InFileName, InFileNameStripped[512],
  InFileNameExt[32], *LogoName, **MovieFileNames,
  Msg[1024], *MovieFileName, PPortOutVal, *RawPixelsX, *ProcessedPixelsX,
  *VCAAToGrayLevelLUT565, *VCAAToGrayLevelLUT664, *VCAA565ToColorLUT,
  *VCRtty, *VCAA664ToColorLUT, cbuf[CBUF_SIZE];

DTYPE double AtanRadiusThreshold, *BSplineKi, *BSplineKj, *BSplineCij,
  *DoubleBuffer, DAValuesOut[16], DASlopes[16], DAOffsets[16], DAMax[16],
  DAMin[16], ErrorImgData[N640X480], FixedDynamicRangeMin,
  FixedDynamicRangeMax, GcFactor, Image_pixels_per_m_X, Image_m_per_pixel_X, 
  Image_pixels_per_m_Y, Image_m_per_pixel_Y, Image_Z_plane,
  *MaskPixels, PixelSpacing, *PlaneAngle, StereoCameraSeparation,
  *ProcessedPixels, ProcMean, ProcSD, ProcMax, ProcMin, *RadiusImgData,
  *SinImgData, *CosImgData, RawMax, RawMin, StereoMMPerPixel,StereoFocalLength,
  *RawPixels, RawMean, RawSD, RawHVisFrac, RawVVisFrac, *SGIRgbFullSizeBuffer,
  ProcHVisFrac, ProcVVisFrac, *residues, *FFTTrigm, *FFTTrigm2, *FFTTrign,
  *FFTTrign2, *FFTWork, *FFTWork2, *FFTMask, *FFTRealBuffer, *FFTRealBuffer2,
  *FFTImBuffer, *FFTImBuffer2, SysTimeSeconds, *FFTWindowBuffer;

DTYPE int ADIndex, AllocatedCmapCells, BasePixel, black, blue, BlueShades,
  CMX, CMY, BSplineNKi, BSplineNKj, cyan, OldRawImageWidgetHeight,
  DistanceFirstPointX, DistanceFirstPointY, OldRawImageWidgetWidth,
  DA0CurrentVal, DA4CurrentVal, DACurrentVals[16],DecimationFactor,
  FrameX, FrameY, FrameWidth, FrameHeight, FFTWindow, NDispFieldElements,
  GcLineWidth, FileSelectionBoxAction, GalvoCycles, MarkerCircleRadius,
  *GLMovieFrames[GLMOVIEFRAMESNMAX], GLMovieNFrames, GrabType, 
  green, GreenShades, GrabbingControl, Image_pixel_org_X, Image_pixel_org_Y,
  ImgHeight, ImageStackTop, ImgWidth, MovieFilesN, MovieCurrentFile,
  MovieLoopCount,MovieLoopDelay, LCRState, magenta, LCR_NoRet, LCR_HalfWaveRet,
  NMaskPixels, MaxScaleVal, MinScaleVal, MovieType, MeshStatesN,
  NContourPoints, NRegions, NOverlayCells, NKnotPoints, NOverlayFunctions,
  NIPRegisteredOps, OverlayLevel, NStrainElements, FFTPreviousWindow,
  ProcessedHistogram[HISTOGRAM_BINS], OldProcImageWidgetHeight,
  OldProcImageWidgetWidth, OldProcImgWidth, OldProcImgHeight, SGIRGBFullHeight,
  OldRawImgWidth, OldRawImgHeight, FFTHeight, FFTWidth, SGIRGBFullWidth,
  ProcessedWinWidth, ProcessedWinHeight, ProcessedWinHOffset,
  ProcessedWinVOffset, PromptDialogAction, SGIFBWidth, SGIFBHeight,
  RawHistogram[HISTOGRAM_BINS], RawWinWidth, RawWinHeight, RawWinHOffset,
  RawWinVOffset, red, RedShades, VCAAFd, VCAASource, VCAAMode,
  VCAAFormat, yellow, white, VCRFramesPerImage, xhairnum, ZoomFactor;

DTYPE unsigned short RawPsBufsX[5*N640X480], RawPsBufsY[5*N640X480],
  TextOverlayBuffer[TEXT_OVERLAY_WIDTH * TEXT_OVERLAY_HEIGHT];

DTYPE unsigned long LutCells[NPAGEDLUTCELLS], CCTable[256], *SGIFrameBuffer;

DTYPE IPOpListItem CurrentIPOp, *IPOpList;

DTYPE IPRegisteredOpListItem IPRegisteredOpList[IPOPREGISTEREDLISTMAX];

#ifndef NOINTRINSICS
DTYPE IPCategoryItem IPOpCategories[IPOPSMAX];
#endif

DTYPE void (*OverlayFunctions[OVERLAYFUNCTIONSMAX])(swidget);

DTYPE Widget ParmLabels[IPOPNPARMS], ParmTexts[IPOPNPARMS];

DTYPE MrmHierarchy hierarchy;

/***************************************************************************/
/*                                                                         */
/*                        Function prototype                               */
/*                                                                         */
/***************************************************************************/
#include "UxXt.h"
#include "xvg_interface.h"
#include "nag.h"
#ifndef CMGUI
#include "errorDialog.h"
#endif
#include "fileSelectionBoxDialog.h"
#include "InfoDialog.h"
#include "InputDialog.h"
#include "promptDialog.h"
#include "HistogramShell.h"
#include "IPOperationsShell.h"
#include "LUTShell.h"
#include "MovieControlsShell.h"
#include "GLShell.h"
#include "GLUtils.h"
#include "RawImageShell.h"
#include "ProcessedImageShell.h"
#include "ScrolledWindowDialog.h"
#include "XvgShell.h"
#include "UxDummies.h"
#include "XWDUtils.h"
#include "XvgRegisterIPOps.h"
#include "binaryIPOPs.h"
#include "feIPOPs.h"
#include "fftIPOPs.h"
#include "fileIPOPs.h"
#include "grabIPOPs.h"
#include "fileio.h"
#include "fitIPOPs.h"
#ifndef CMGUI
#include "grab.h"
#endif
#include "imgproc.h"
#include "morphoIPOPs.h"
#ifdef SGI
#include "sgi.h"
#else
#include "second.h"
#endif
#include "spacedomIPOPs.h"
#include "stereoIPOPs.h"
#include "unima.h"
#include "utils.h"
#include "XvgMemoryManager.h"
#include "wrappingIPOPs.h"
#include "XvgSystem.h"
#include "SGIGrabIPOPs.h"
#include "NRC_SVD.h"
#include "NRC_FFT.h"
#ifndef _NO_MATLAB
#include "matlab.h"
#endif

/* CMGUI include files */
#include "command/command.h"
#include "command/command_window.h"
#include "general/debug.h"
#include "user_interface/message.h"
#include "user_interface/user_interface.h"
#include "finite_element/finite_element.h"

extern struct GROUP(FE_node) *CurrentFENodeGroup;

extern struct MANAGER(FE_field) *xvg_fe_field_manager;
extern struct MANAGER(FE_node) *xvg_node_manager;
extern struct MANAGER(GROUP(FE_node)) *xvg_node_group_manager;

/***************************************************************************/
/*                                                                         */
/*                  explicit function prototype                            */
/*                                                                         */
/***************************************************************************/
/*???DB.  In unistd.h */
/*extern int    gethostname(char *Name, int NameLength);*/
extern int    finite(double);
extern double rint(double);
#if defined (IRIX)
/* This definition conflicts on LINUX */
extern long   random(void);
#endif /* defined (IRIX) */
extern double trunc(double);

#endif







