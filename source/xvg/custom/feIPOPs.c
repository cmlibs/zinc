/***********************************************************************
*
*  Name:          feIPOPs.c
*
*  Author:        Paul Charette
*
*  Last Modified:
*                 - Paul Charette, 25 Sept 1995
*                   Added support for bicubic interpolation of the field
*                   (Note: element scaling factors not included,
*                   and estimates of derivative components of the field
*                   at the nodes w/r to element coordinates from the 
*                   bilinear model do not match the bicubic interpolation
*                   fitted values for some as yet unexplained reason).
*                 - Paul Charette, 5 Oct. 1995
*                   Added double pass linear then non-linear fitting
*
*
*  Purpose:       Utility routines for finite element computations
*                    BuildInterpolatedImage()
*                    ComputeMarkerPaths()
*                    DrawFEMarkers()
*                    FEComputeDisplacedNodes()
*                    FEComputeDispNodesXCorr()
*                    FEMeshOverlay()
*                    FEStrain()
*                    LoadFEMeshMenu()
*                    LoadMesh()
*                    MeshVideoLoop()
*                    PickMeshCoordinate()
*                    RegionMaskCreation()
*                    System()
*                    VAFCompute()
*                    WriteCMISSFieldSamplesFile()
*                    WriteCMISSForcesFile()
*                    WriteCMISSNodesFile()
*                    WriteCMISSPointsFile()
*
***********************************************************************/
#include <stdio.h>
#include "UxXt.h"
#include "XvgGlobals.h"
#include "ForceTransducerCalibration.h"
#ifdef FE_COMPUTE_DISPLACED_NODES_SPECKLE
#include "ForceModels.h"
#endif

/***********************************************************************/
/*                                                                     */
/*  Bicubic Hermite interpolation basis function macros                */
/*                                                                     */
/***********************************************************************/
#define H01(x) (1.0 - (x)*(3.0*(x) - 2.0*(x)*(x)))
#define H11(x) ((x)*((x) - 1.0)*((x) - 1.0))
#define H02(x) ((x)*(x)*(3.0 - 2.0*(x)))
#define H12(x) ((x)*(x)*((x) - 1.0))


/***********************************************************************/
/*                                                                     */
/*  Global FE variable declarations.                                   */
/*                                                                     */
/***********************************************************************/
#define PRECISION 1.0e-08

/* FE element type */
enum {R_TYPE, T_TYPE};
enum {BILINEAR_FIELD_MODEL, BICUBIC_FIELD_MODEL};

#define NNODES_MAX        76
#define NELEMENTS_MAX     60
#define NRELEMENTS_MAX    48

#define NRELEMENTS_MIN    24
#define NELEMENTS_MIN     32
#define NNODES_MIN        37

#define NCFMAX            50

typedef struct {
    short  element;
    double u;
    double v;
    double Phi[16];
    double Exx;
    double Exy;
    double Eyy;
} FESample;

typedef struct {
    int type;
    int nodes[4];
    int neighbours[8];
    int cx;
    int cy;
    int nsamples;
    int nsamples_x;
    int nsamples_y;
} MeshElement;

typedef struct {
    double xw;
    double yw;
    double xp;
    double yp;
} MeshNode;

/* definition of the fe mesh (list of four clockwise nodes for each element,*/
/* followed by the eight surrounding neighbouring elements.                 */
/* Clockwise list of contour nodes:                                         */
/*    1,2,7,14,21,28,33,36,35,34,29,22, 15,8,3,0                            */
MeshElement MeshElements[NELEMENTS_MAX] = {
  {R_TYPE, 4, 0, 1, 5,       25, -1, -1, -1, 1, 4, 3, 2,     0, 0}, /* el. 0 */
  {R_TYPE, 5, 1, 2, 6,        0, -1, -1, -1, 26, 5, 4, 3,    0, 0}, /* el. 1 */
  {R_TYPE, 9, 3, 4, 10,      24, -1, 25, 0, 3, 8, 7, 6,      0, 0}, /* el. 2 */
  {R_TYPE, 10, 4, 5, 11,      2, 25, 0, 1, 4, 9, 8, 7,       0, 0}, /* el. 3 */
  {R_TYPE, 11, 5, 6, 12,      3, 0, 1, 26, 5, 10, 9, 8,      0, 0}, /* el. 4 */
  {R_TYPE, 12, 6, 7, 13,      4, 1, 26, -1, 27, 11, 10, 9,   0, 0}, /* el. 5 */
  {R_TYPE, 15, 8, 9, 16,     -1, -1, 24, 2, 7, 13, 12, -1,   0, 0}, /* el. 6 */
  {R_TYPE, 16, 9, 10, 17,     6, 24, 2, 3, 8, 14, 13, 12,    0, 0}, /* el. 7 */
  {R_TYPE, 17, 10, 11, 18,    7, 2, 3, 4, 9, 15, 14, 13,     0, 0}, /* el. 8 */
  {R_TYPE, 18, 11, 12, 19,    8, 3, 4, 5, 10, 16, 15, 14,    0, 0}, /* el. 9 */
  {R_TYPE, 19, 12, 13, 20,    9, 4, 5, 27, 11, 17, 16, 15,   0, 0}, /* e. 10 */
  {R_TYPE, 20, 13, 14, 21,   10, 5, 27, -1, -1, -1, 17, 16,  0, 0}, /* e. 11 */
  {R_TYPE, 22, 15, 16, 23,   -1, -1, 6, 7, 13, 18, 31, -1,   0, 0}, /* e. 12 */
  {R_TYPE, 23, 16, 17, 24,   12, 6, 7, 8, 14, 19, 18, 31,    0, 0}, /* e. 13 */
  {R_TYPE, 24, 17, 18, 25,   13, 7, 8, 9, 15, 20, 19, 18,    0, 0}, /* e. 14 */
  {R_TYPE, 25, 18, 19, 26,   14, 8, 9, 10, 16, 21, 20, 19,   0, 0}, /* e. 15 */
  {R_TYPE, 26, 19, 20, 27,   15, 9, 10, 11, 17, 28, 21, 20,  0, 0}, /* e. 16 */
  {R_TYPE, 27, 20, 21, 28,   16, 10, 11, -1, -1, -1, 28, 21, 0, 0}, /* e. 17 */
  {R_TYPE, 29, 23, 24, 30,   31, 12, 13, 14, 19, 22, 30, -1, 0, 0}, /* e. 18 */
  {R_TYPE, 30, 24, 25, 31,   18, 13, 14, 15, 20, 23, 22, 30, 0, 0}, /* e. 19 */
  {R_TYPE, 31, 25, 26, 32,   19, 14, 15, 16, 21, 29, 23, 22, 0, 0}, /* e. 20 */
  {R_TYPE, 32, 26, 27, 33,   20, 15, 16, 17, 28, -1, 29, 23, 0, 0}, /* e. 21 */
  {R_TYPE, 34, 30, 31, 35,   30, 18, 19, 20, 23, -1, -1, -1, 0, 0}, /* e. 22 */
  {R_TYPE, 35, 31, 32, 36,   22, 19, 20, 21, 29, -1, -1, -1, 0, 0}, /* e. 23 */
#ifdef _FE_MODEL_HOLES
/*  {R_TYPE, 37,  7,  2, 38,   -1, -1, -1, -1, -1, -1, -1, -1, 0, 0},  e. 32 */
/*  {R_TYPE,  7, 37, 38, 39,   -1, -1, -1, -1, -1, -1, -1, -1, 0, 0},  e. 33 */
  {R_TYPE, 14,  7, 39, 40,   -1, -1, -1, -1, -1, -1, -1, -1, 0, 0}, /* e. 34 */
  {R_TYPE, 40, 39, 38, 41,   -1, -1, -1, -1, -1, -1, -1, -1, 0, 0}, /* e. 35 */
/*  {R_TYPE, 14, 40, 41, 42,   -1, -1, -1, -1, -1, -1, -1, -1, 0, 0},  e. 36 */
  {R_TYPE, 21, 14, 42, 43,   -1, -1, -1, -1, -1, -1, -1, -1, 0, 0}, /* e. 37 */
  {R_TYPE, 43, 42, 41, 44,   -1, -1, -1, -1, -1, -1, -1, -1, 0, 0}, /* e. 38 */
/*  {R_TYPE, 21, 43, 44, 45,   -1, -1, -1, -1, -1, -1, -1, -1, 0, 0},  e. 39 */
  {R_TYPE, 28, 21, 45, 46,   -1, -1, -1, -1, -1, -1, -1, -1, 0, 0}, /* e. 40 */
  {R_TYPE, 46, 45, 44, 47,   -1, -1, -1, -1, -1, -1, -1, -1, 0, 0}, /* e. 41 */
/*  {R_TYPE, 28, 46, 47, 48,   -1, -1, -1, -1, -1, -1, -1, -1, 0, 0},  e. 42 */
  {R_TYPE, 33, 28, 48, 49,   -1, -1, -1, -1, -1, -1, -1, -1, 0, 0}, /* e. 43 */
  {R_TYPE, 49, 48, 47, 50,   -1, -1, -1, -1, -1, -1, -1, -1, 0, 0}, /* e. 44 */
/*  {R_TYPE, 33, 49, 50, 51,   -1, -1, -1, -1, -1, -1, -1, -1, 0, 0},  e. 45 */
  {R_TYPE, 36, 33, 51, 52,   -1, -1, -1, -1, -1, -1, -1, -1, 0, 0}, /* e. 46 */
  {R_TYPE, 52, 51, 50, 53,   -1, -1, -1, -1, -1, -1, -1, -1, 0, 0}, /* e. 47 */
/*  {R_TYPE, 36, 52, 53, 54,   -1, -1, -1, -1, -1, -1, -1, -1, 0, 0},  e. 48 */
  {R_TYPE, 35, 36, 54, 55,   -1, -1, -1, -1, -1, -1, -1, -1, 0, 0}, /* e. 49 */
  {R_TYPE, 55, 54, 53, 56,   -1, -1, -1, -1, -1, -1, -1, -1, 0, 0}, /* e. 50 */
/*  {R_TYPE, 57, 35, 55, 56,   -1, -1, -1, -1, -1, -1, -1, -1, 0, 0},  e. 51 */
  {R_TYPE, 34, 35, 57, 58,   -1, -1, -1, -1, -1, -1, -1, -1, 0, 0}, /* e. 52 */
  {R_TYPE, 58, 57, 56, 59,   -1, -1, -1, -1, -1, -1, -1, -1, 0, 0}, /* e. 53 */
/*  {R_TYPE, 34, 58, 59, 60,   -1, -1, -1, -1, -1, -1, -1, -1, 0, 0},  e. 54 */
  {R_TYPE, 29, 34, 60, 61,   -1, -1, -1, -1, -1, -1, -1, -1, 0, 0}, /* e. 55 */
  {R_TYPE, 61, 60, 59, 62,   -1, -1, -1, -1, -1, -1, -1, -1, 0, 0}, /* e. 56 */
/*  {R_TYPE, 29, 61, 62, 63,   -1, -1, -1, -1, -1, -1, -1, -1, 0, 0},  e. 57 */
  {R_TYPE, 22, 29, 63, 64,   -1, -1, -1, -1, -1, -1, -1, -1, 0, 0}, /* e. 58 */
  {R_TYPE, 64, 63, 62, 65,   -1, -1, -1, -1, -1, -1, -1, -1, 0, 0}, /* e. 59 */
/*  {R_TYPE, 22, 64, 65, 66,   -1, -1, -1, -1, -1, -1, -1, -1, 0, 0},  e. 60 */
  {R_TYPE, 15, 22, 66, 67,   -1, -1, -1, -1, -1, -1, -1, -1, 0, 0}, /* e. 61 */
  {R_TYPE, 67, 66, 65, 68,   -1, -1, -1, -1, -1, -1, -1, -1, 0, 0}, /* e. 62 */
/*  {R_TYPE, 15, 67, 68, 69,   -1, -1, -1, -1, -1, -1, -1, -1, 0, 0},  e. 63 */
  {R_TYPE,  8, 15, 69, 70,   -1, -1, -1, -1, -1, -1, -1, -1, 0, 0}, /* e. 64 */
  {R_TYPE, 70, 69, 68, 71,   -1, -1, -1, -1, -1, -1, -1, -1, 0, 0}, /* e. 65 */
/*  {R_TYPE,  8, 70, 71, 72,   -1, -1, -1, -1, -1, -1, -1, -1, 0, 0},  e. 66 */
  {R_TYPE,  3,  8, 72, 73,   -1, -1, -1, -1, -1, -1, -1, -1, 0, 0}, /* e. 67 */
  {R_TYPE, 73, 72, 71, 74,   -1, -1, -1, -1, -1, -1, -1, -1, 0, 0}, /* e. 68 */
/*  {R_TYPE,  3, 73, 74, 75,   -1, -1, -1, -1, -1, -1, -1, -1, 0, 0},  e. 69 */
/*  {R_TYPE,  3, 75, 74,  0,   -1, -1, -1, -1, -1, -1, -1, -1, 0, 0},  e. 70 */
#endif
  {T_TYPE,  8,  3,  9, -1,    2,  7,  6, -1, -1, -1, -1, 25, 0, 0}, /* e. 24 */
  {T_TYPE,  3,  0,  4, -1,    0,  3,  2, 24, -1, -1, -1, -1, 0, 0}, /* e. 25 */
  {T_TYPE,  2,  7,  6, -1,    5,  4,  1, -1, -1, -1, -1, 27, 0, 0}, /* e. 26 */
  {T_TYPE,  7, 14, 13, -1,   11, 10,  5, 26, -1, -1, -1, -1, 0, 0}, /* e. 27 */
  {T_TYPE, 28, 33, 27, -1,   21, 16, 17, -1, -1, -1, -1, 29, 0, 0}, /* e. 28 */
  {T_TYPE, 33, 36, 32, -1,   23, 20, 21, 28, -1, -1, -1, -1, 0, 0}, /* e. 29 */
  {T_TYPE, 34, 29, 30, -1,   18, 19, 22, -1, -1, -1, -1, 31, 0, 0}, /* e. 30 */
  {T_TYPE, 29, 22, 23, -1,   12, 13, 18, 30, -1, -1, -1, -1, 0, 0}  /* e. 31 */
#ifdef _FE_MODEL_HOLES
 ,{T_TYPE, 75, 74,  0, -1,   -1, -1, -1, -1, -1, -1, -1, -1, 0, 0}, /* e.    */
  {T_TYPE,  3, 75,  0, -1,   -1, -1, -1, -1, -1, -1, -1, -1, 0, 0}, /* e.    */
  {T_TYPE,  7,  2, 37, -1,   -1, -1, -1, -1, -1, -1, -1, -1, 0, 0}, /* e.    */
  {T_TYPE, 37,  2, 38, -1,   -1, -1, -1, -1, -1, -1, -1, -1, 0, 0}  /* e.    */
#endif
};
MeshNode MeshNodes[NNODES_MAX];
static MeshNode MeshNodesFixed[NNODES_MAX];

/***********************************************************************/
/*                                                                     */
/*  Local FE variable declarations.                                    */
/*                                                                     */
/***********************************************************************/
#ifdef FE_COMPUTE_DISPLACED_NODES_SPECKLE
#define NGALVOS      13
static double forces_x[NGALVOS], forces_y[NGALVOS];
static int HookNodes[NGALVOS] = {7, 14, 21, 28, 33, 36, 35, 34, 29, 22, 15,
				   8, 3};
#endif

#define NGAUSSPOINTS  4
#define NFEMarkersMAX 100
#define NBOUNDARYNODES 16

/* Scaling factor to convert phase to distance            */
#define METERS_PER_RADIAN       154.473e-9

/* fixed point world coordinate location */
#define FIXEDPX    0.000708
#define FIXEDPY    0.008204;

extern Widget ProcessedImageTBG, ProcessedImageShell, XvgShell,
  ProcessedImageDrawingArea, UxTopLevel;

static double VAFsX[NELEMENTS_MAX], VAFsY[NELEMENTS_MAX], CCDRotationAngle;
static short *FEMeshPixels;
static FESample *FieldSamples, FEMarkers[NFEMarkersMAX];
static boolean_t FEMeshDefined;
static int NFEMarkers, CurrentMeshStep;
#ifdef FE_COMPUTE_DISPLACED_NODES_SPECKLE
static int BoundaryNodes[NBOUNDARYNODES] = {1,2,7,14,21,28,33,36,35,34,29,22,
					      15,8,3,0};
#else
static int BoundaryNodes[NBOUNDARYNODES] = {33,28,21,14,7,2,1,0,3,8,15,22,
					      29,34,35,36};
#endif
static int nrelements, nelements, nnodes;

static double ObjFunScale;

/* Initialize FE environment variables */
void Xvg_InitFE(void)
{
  NFEMarkers = 0;
  FEMeshPixels = NULL;
  FieldSamples = NULL;
  FEMeshDefined = B_FALSE;
#ifdef FE_COMPUTE_DISPLACED_NODES_SPECKLE
  CCDRotationAngle = 0;
#endif
  StrainElements = NULL;
#ifdef _FE_MODEL_HOLES
  nrelements = NRELEMENTS_MAX;
  nelements = NELEMENTS_MAX;
  nnodes = NNODES_MAX;
#else
  nrelements = NRELEMENTS_MIN;
  nelements = NELEMENTS_MIN;
  nnodes = NNODES_MIN;
#endif
}

/* load FE mesh image, push current image onto the stack */
void LoadFEMeshMenu(void)
{
  int i;

  /* push current image onto the stack */
  HGCursor(B_TRUE);
  if (XvgImgStack_Push(ProcessedPixels, ImgHeight, ImgWidth) == B_FALSE) {
    ErrorMsg("LoadFEMeshMenu(): Failed pushing image onto the stack");
    HGCursor(B_FALSE);
    return;
  }

  /* load the mesh data into the current buffer */
  for (i = 0; i < ImgHeight*ImgWidth; i++)
    ProcessedPixels[i] = FEMeshPixels[i];
  DynamicRange(ProcessedPixels, ImgHeight*ImgWidth, &ProcMax, &ProcMin);
  CreateProcImage();
  HGCursor(B_FALSE);
}


/* display FE mesh */
void FEMeshOverlay(Widget DrawingArea)
{
  FILE *fp;
  GC gc_local;
  XPoint points[5];
  int i;
  char s[256];

#ifdef _FE_WRITE_OVERLAY
  /* write out element boundaries to a file */
  if ((fp = fopen("FeMeshElements.dat", "w")) == NULL) {
    Abort = B_TRUE;
    sprintf(s, "Could not open the output file \"%s\" in FEMeshOverlay()",
	    "FeMeshElements.dat");
    draw_mesg(s);
    return;
  }
#endif
  
  /* draw black on white if monochrome */
  if (Monochrome)
    gc_local = gc_WhiteBlack;
  else
    gc_local = gc_YellowBlack;

  /* loop to draw the elements */
  for (i = 0; i < nelements; i++) {
    if (MeshElements[i].type == R_TYPE) {
      points[0].x = rint(MeshNodes[MeshElements[i].nodes[0]].xp
			    * ((double) ZoomFactor));
      points[0].y = rint(MeshNodes[MeshElements[i].nodes[0]].yp
			    * ((double) ZoomFactor));
      points[1].x = rint(MeshNodes[MeshElements[i].nodes[1]].xp
			    * ((double) ZoomFactor));
      points[1].y = rint(MeshNodes[MeshElements[i].nodes[1]].yp
			    * ((double) ZoomFactor));
      points[2].x = rint(MeshNodes[MeshElements[i].nodes[2]].xp
			    * ((double) ZoomFactor));
      points[2].y = rint(MeshNodes[MeshElements[i].nodes[2]].yp
			    * ((double) ZoomFactor));
      points[3].x = rint(MeshNodes[MeshElements[i].nodes[3]].xp
			    * ((double) ZoomFactor));
      points[3].y = rint(MeshNodes[MeshElements[i].nodes[3]].yp
			    * ((double) ZoomFactor));
      points[4].x = rint(MeshNodes[MeshElements[i].nodes[0]].xp
			    * ((double) ZoomFactor));
      points[4].y = rint(MeshNodes[MeshElements[i].nodes[0]].yp
			    * ((double) ZoomFactor));
      XDrawLines(UxDisplay, XtWindow(DrawingArea), gc_local,
		 points, 5, CoordModeOrigin);
#ifdef _FE_WRITE_OVERLAY
      fprintf(fp, "%d %d %d %d %d\n",
	      points[0].x, points[1].x, points[2].x, points[3].x, points[4].x);
      fprintf(fp, "%d %d %d %d %d\n",
	      points[0].y, points[1].y, points[2].y, points[3].y, points[4].y);
#endif
    }
    else {
      points[0].x = rint(MeshNodes[MeshElements[i].nodes[0]].xp
			    * ((double) ZoomFactor));
      points[0].y = rint(MeshNodes[MeshElements[i].nodes[0]].yp
			    * ((double) ZoomFactor));
      points[1].x = rint(MeshNodes[MeshElements[i].nodes[1]].xp
			    * ((double) ZoomFactor));
      points[1].y = rint(MeshNodes[MeshElements[i].nodes[1]].yp
			    * ((double) ZoomFactor));
      points[2].x = rint(MeshNodes[MeshElements[i].nodes[2]].xp
			    * ((double) ZoomFactor));
      points[2].y = rint(MeshNodes[MeshElements[i].nodes[2]].yp
			    * ((double) ZoomFactor));
      points[3].x = rint(MeshNodes[MeshElements[i].nodes[0]].xp
			    * ((double) ZoomFactor));
      points[3].y = rint(MeshNodes[MeshElements[i].nodes[0]].yp
			    * ((double) ZoomFactor));
      XDrawLines(UxDisplay, XtWindow(DrawingArea), gc_local,
		 points, 4, CoordModeOrigin);
#ifdef _FE_WRITE_OVERLAY
      fprintf(fp, "%d %d %d %d %d\n",
	      points[0].x, points[1].x, points[2].x, points[3].x, points[3].x);
      fprintf(fp, "%d %d %d %d %d\n",
	      points[0].y, points[1].y, points[2].y, points[3].y, points[3].y);
#endif
    }
  }
#ifdef _FE_WRITE_OVERLAY
  fclose(fp);
#endif
  
#ifdef _FE_MODEL_HOLES
  if (ZoomFactor > 1) {
#endif

#ifdef _FE_WRITE_OVERLAY
  /* write out node index coordinates */
  if ((fp = fopen("FeNodes.dat", "w")) == NULL) {
    Abort = B_TRUE;
    sprintf(s, "Could not open the output file \"%s\" in FEMeshOverlay()",
	    "FeNodes.dat");
    draw_mesg(s);
    return;
  }
#endif
  
    /* draw white on black if monochrome */
    if (Monochrome)
      gc_local = gc_WhiteBlack;
    else
      gc_local = gc_YellowBlack;
    
    /* draw node & element nunbers only if there is no strain overlay */
    for (i=0;(i<NOverlayFunctions)&&(OverlayFunctions[i] != StrainOverlay);
	 i++);
    if (i == NOverlayFunctions) {
      /* loop to draw the node points and index numbers */
      for (i = 0; i < nnodes; i++) {
	sprintf(s, "%d", i);
	XDrawString(UxDisplay, XtWindow(DrawingArea), gc_local,
		    (int) (MeshNodes[i].xp * ((double) ZoomFactor)) + 7,
		    (int) (MeshNodes[i].yp * ((double) ZoomFactor)) + 7 + 14,
		    s, strlen(s));
#ifdef _FE_WRITE_OVERLAY
	fprintf(fp, "%d %d %d\n", i,
		(int) (MeshNodes[i].xp * ((double) ZoomFactor)) + 7,
		(int) (MeshNodes[i].yp * ((double) ZoomFactor)) + 7 + 14);
#endif
      }
      /* loop to draw the element index numbers if required */
      if (!Monochrome) {
	for (i = 0; i < nelements; i++) {
	  sprintf(s, "%d", i);
	  XDrawString(UxDisplay,XtWindow(DrawingArea),
		      gc_RedBlack,
		      MeshElements[i].cx * ZoomFactor,
		      MeshElements[i].cy * ZoomFactor,
		      s, strlen(s));
	}
      }
    }
#ifdef _FE_MODEL_HOLES
  }
#endif

#ifdef _FE_WRITE_OVERLAY
  fclose(fp);
#endif
  /* force server to draw */
  XSync(UxDisplay, B_FALSE);
}

/************************ ComputeIntersection **************************/
/*                                                                     */
/* Routine which compute the intersection point of two lines, where :  */
/*      the first  line joins Node00 and Node01                        */
/*      the second line joins Node10 and Node11                        */
/*                                                                     */
/***********************************************************************/
static MeshNode ComputeIntersection(MeshNode n00, MeshNode n01,
				    MeshNode n10, MeshNode n11)
{
  double m0, m1, b0, b1;
  MeshNode n;

  /* check for parallel line error condition */
  if (((fabs(n01.xp - n00.xp) < 0.000001)
       && (fabs(n11.xp - n10.xp) < 0.000001)) 
      || ((fabs(n01.yp - n00.yp) < 0.000001) &&
	  (fabs(n11.yp - n10.yp) < 0.000001))) {
    ErrorMsg("ComputeIntersection() : Parallel lines error!");
    return(n00);
  }
  
  /* check for perpendicular lines degenerate case */
  if ((fabs(n01.xp - n00.xp) < 0.000001)
      && (fabs(n11.yp - n10.yp) < 0.000001)) {
    n.xp = (n01.xp + n00.xp)/2.0;
    n.yp = (n11.yp + n10.yp)/2.0;
  }
  else if ((fabs(n01.yp - n00.yp) < 0.000001)
	   && (fabs(n11.xp - n10.xp) < 0.000001)) {
    n.xp = (n11.xp + n10.xp)/2.0;
    n.yp = (n01.yp + n00.yp)/2.0;
  }
  /* check one vertical line case */
  else if (fabs(n01.xp - n00.xp) < 0.000001) {
    n.xp = (n01.xp + n00.xp)/2.0;
    n.yp = ((n11.yp - n10.yp)*n.xp
	    + (n11.xp*n10.yp - n10.xp*n11.yp))/(n11.xp - n10.xp);
  }
  else if (fabs(n11.xp - n10.xp) < 0.000001) {
    n.xp = (n11.xp + n10.xp)/2.0;
    n.yp = ((n01.yp - n00.yp)*n.xp
	    + (n01.xp*n00.yp - n00.xp*n01.yp))/(n01.xp - n00.xp);
  }
  /* general case, including horizontal lines */
  else {
    m0 = (n01.yp - n00.yp) / (n01.xp - n00.xp);
    b0 = (n01.xp*n00.yp - n00.xp*n01.yp) / (n01.xp - n00.xp);
    m1 = (n11.yp - n10.yp) / (n11.xp - n10.xp);
    b1 = (n11.xp*n10.yp - n10.xp*n11.yp) / (n11.xp - n10.xp);
    n.xp = (b0 - b1)/(m1 - m0);
    n.yp = (m1*b0 - m0*b1)/(m1 - m0);
  }
  return(n);
}

/*************************     GenerateMesh     ************************/
/*                                                                     */
/* Routine which generates an template of elements where each element  */
/* is assigned a unique value, and initializes the element info.       */
/*                                                                     */
/***********************************************************************/
static void GenerateMesh(boolean_t NewMesh)
{
#if (!defined (CMGUI) || !defined (NO_XVGMAIN))
  extern swidget XvgInitX(void);
#endif
  Pixmap pmap;
  XImage *image;
  XPoint points[5];
  MeshNode cn;
  boolean_t VAL_FOUND;
  double deltax, deltay, max;
  int i, j, k, l, m, n, r;

  /* check that FEMeshPixels is defined */
  if (FEMeshPixels == NULL) {
    Abort = B_TRUE;
    ErrorMsg("Abort: undefined FEMeshPixels in GenerateMesh()");
    return;
  }

  /* verbose */
  if (VerboseFlag)
    printf("GenerateMesh(%d)...\n", NewMesh);
  
  /* create local graphics context, if required */
  if (InteractiveFlag == B_FALSE) {
#if (!defined (CMGUI) || !defined (NO_XVGMAIN))
    XvgInitX();
#endif
    xgc.foreground = 0;
    xgc.function = GXcopy;
    gc = XCreateGC(UxDisplay, RootWindow(UxDisplay, UxScreen),
			GCFunction | GCForeground, &xgc);
  }
  
  /* create in-memory pixamp for drawing */
#ifndef SGI
  printf("GenerateMesh() : No in-memory pixmaps on IBM X! Program stop.\n");
  exit(-1);
#endif
  pmap = XCreatePixmap(UxDisplay, RootWindow(UxDisplay, UxScreen),
		       ImgWidth, ImgHeight, 8);

  /* clear pixmap */
  xgc.foreground = 0;
  XChangeGC(UxDisplay, gc, GCForeground, &xgc);
  XFillRectangle(UxDisplay, pmap, gc, 0, 0, ImgWidth, ImgHeight);

  /* loop for all elements */
  for (n = 0; n < nelements; n++) {

    /* rectangular element ? */
    if (MeshElements[n].type == R_TYPE) {
      /* set color in the graphics context and draw the polygon */
      if (NewMesh) {
	xgc.foreground = 1 + n;
	XChangeGC(UxDisplay, gc, GCForeground, &xgc);
	points[0].x = rint(MeshNodes[MeshElements[n].nodes[0]].xp);
	points[0].y = rint(MeshNodes[MeshElements[n].nodes[0]].yp);
	points[1].x = rint(MeshNodes[MeshElements[n].nodes[1]].xp);
	points[1].y = rint(MeshNodes[MeshElements[n].nodes[1]].yp);
	points[2].x = rint(MeshNodes[MeshElements[n].nodes[2]].xp);
	points[2].y = rint(MeshNodes[MeshElements[n].nodes[2]].yp);
	points[3].x = rint(MeshNodes[MeshElements[n].nodes[3]].xp);
	points[3].y = rint(MeshNodes[MeshElements[n].nodes[3]].yp);
	points[4].x = rint(MeshNodes[MeshElements[n].nodes[0]].xp);
	points[4].y = rint(MeshNodes[MeshElements[n].nodes[0]].yp);
	XFillPolygon(UxDisplay, pmap,
		     gc, points, 5, Nonconvex, CoordModeOrigin);
      }
      
      /* init sample counters */
      MeshElements[n].nsamples = 0;
      MeshElements[n].nsamples_x = 0;
      MeshElements[n].nsamples_y = 0;
      
      /* compute the center pixel coordinates */
      cn = ComputeIntersection(MeshNodes[MeshElements[n].nodes[0]], 
			       MeshNodes[MeshElements[n].nodes[2]], 
			       MeshNodes[MeshElements[n].nodes[1]], 
			       MeshNodes[MeshElements[n].nodes[3]]);
      MeshElements[n].cx = cn.xp;
      MeshElements[n].cy = cn.yp;
    }
    
    /* triangular element */
    else {
      /* set color in the graphics context and draw the polygon */
      if (NewMesh) {
	xgc.foreground = 1 + n;
	XChangeGC(UxDisplay, gc, GCForeground, &xgc);
	points[0].x = rint(MeshNodes[MeshElements[n].nodes[0]].xp);
	points[0].y = rint(MeshNodes[MeshElements[n].nodes[0]].yp);
	points[1].x = rint(MeshNodes[MeshElements[n].nodes[1]].xp);
	points[1].y = rint(MeshNodes[MeshElements[n].nodes[1]].yp);
	points[2].x = rint(MeshNodes[MeshElements[n].nodes[2]].xp);
	points[2].y = rint(MeshNodes[MeshElements[n].nodes[2]].yp);
	points[3].x = rint(MeshNodes[MeshElements[n].nodes[0]].xp);
	points[3].y = rint(MeshNodes[MeshElements[n].nodes[0]].yp);
	XFillPolygon(UxDisplay, pmap,
		     gc, points, 4, Nonconvex, CoordModeOrigin);
      }
      
      /* init sample counters */
      MeshElements[n].nsamples = 0;
      MeshElements[n].nsamples_x = 0;
      MeshElements[n].nsamples_y = 0;
      
      /* compute the center pixel coordinates */
      MeshElements[n].cx = (MeshNodes[MeshElements[n].nodes[0]].xp
			    + MeshNodes[MeshElements[n].nodes[1]].xp)/2.0;
      MeshElements[n].cy = (MeshNodes[MeshElements[n].nodes[0]].yp
			    + MeshNodes[MeshElements[n].nodes[1]].yp)/2.0;
    }
  }

  if (NewMesh) {
    /* grab the mesh image and copy the data to the mesh pixels buffer */
    /* and get rid of isolated "0" values resulting for drawing roundoff */
    image = XGetImage(UxDisplay, pmap, 0, 0,
		      ImgWidth, ImgHeight, AllPlanes, ZPixmap);
    for (j=0, k=0; j < ImgHeight; j++)
      for (i=0; i < ImgWidth; i++, k++)
	FEMeshPixels[k] = (image->data)[j*(image->bytes_per_line) + i];

    /* loop to convert "colors" to element indicies by decrementing */
    for (k = 0; k < ImgWidth*ImgHeight; k++) {
      FEMeshPixels[k] = FEMeshPixels[k] - 1;
    }
    
    /* loop to attempt to convert "-1" valued pixels to element indicies */
    for (j=0, k=0; j < ImgHeight; j++) {
      for (i=0; i < ImgWidth; i++, k++) {
	if ((j > 0) && (j < ImgHeight-1) && (i > 0) && (i < ImgWidth-1)) {
	  if (FEMeshPixels[k] < 0) {
	    /* so look at its 8-neighbours for a non-zero value */
	    for (m=j-1, VAL_FOUND=B_FALSE; (m <= j+1)&&(VAL_FOUND==B_FALSE); 
		 m++) {
	      for (l=i-1; (l <= i+1) && (VAL_FOUND==B_FALSE); l++) {
		if ((image->data)[m*(image->bytes_per_line) + l] > 0) {
		  FEMeshPixels[k] =
		    (image->data)[m*(image->bytes_per_line) + l] - 1;
		  VAL_FOUND = B_TRUE;
		}
	      }
	    }
	  }
	}
      }
    }

    /* destroy image */
    XDestroyImage(image);
    /* free in-memory drawing pixmap */
    XFreePixmap(UxDisplay, pmap);
  }
}

/**************************** InitNodes ********************************/
/*                                                                     */
/* Routine which computes the coordinates of the internal mesh nodes.  */
/* The boundary nodes are assumed to have been previously set.         */
/*                                                                     */
/***********************************************************************/
static void InitNodes(void)
{
  int i;

  MeshNodes[4] = ComputeIntersection(MeshNodes[3], MeshNodes[7], 
				     MeshNodes[0], MeshNodes[34]);
  MeshNodes[5] = ComputeIntersection(MeshNodes[3], MeshNodes[7], 
				     MeshNodes[1], MeshNodes[35]);
  MeshNodes[6] = ComputeIntersection(MeshNodes[3], MeshNodes[7], 
				     MeshNodes[2], MeshNodes[36]);
  MeshNodes[9] = ComputeIntersection(MeshNodes[8], MeshNodes[14], 
				     MeshNodes[3], MeshNodes[29]);
  MeshNodes[10] = ComputeIntersection(MeshNodes[8], MeshNodes[14], 
				     MeshNodes[0], MeshNodes[34]);
  MeshNodes[11] = ComputeIntersection(MeshNodes[8], MeshNodes[14], 
				     MeshNodes[1], MeshNodes[35]);
  MeshNodes[12] = ComputeIntersection(MeshNodes[8], MeshNodes[14], 
				     MeshNodes[2], MeshNodes[36]);
  MeshNodes[13] = ComputeIntersection(MeshNodes[8], MeshNodes[14], 
				     MeshNodes[7], MeshNodes[33]);
  MeshNodes[16] = ComputeIntersection(MeshNodes[15], MeshNodes[21], 
				     MeshNodes[3], MeshNodes[29]);
  MeshNodes[17] = ComputeIntersection(MeshNodes[15], MeshNodes[21], 
				     MeshNodes[0], MeshNodes[34]);
  MeshNodes[18] = ComputeIntersection(MeshNodes[15], MeshNodes[21], 
				     MeshNodes[1], MeshNodes[35]);
  MeshNodes[19] = ComputeIntersection(MeshNodes[15], MeshNodes[21], 
				     MeshNodes[2], MeshNodes[36]);
  MeshNodes[20] = ComputeIntersection(MeshNodes[15], MeshNodes[21], 
				     MeshNodes[7], MeshNodes[33]);
  MeshNodes[23] = ComputeIntersection(MeshNodes[22], MeshNodes[28], 
				     MeshNodes[3], MeshNodes[29]);
  MeshNodes[24] = ComputeIntersection(MeshNodes[22], MeshNodes[28], 
				     MeshNodes[0], MeshNodes[34]);
  MeshNodes[25] = ComputeIntersection(MeshNodes[22], MeshNodes[28], 
				     MeshNodes[1], MeshNodes[35]);
  MeshNodes[26] = ComputeIntersection(MeshNodes[22], MeshNodes[28], 
				     MeshNodes[2], MeshNodes[36]);
  MeshNodes[27] = ComputeIntersection(MeshNodes[22], MeshNodes[28], 
				     MeshNodes[7], MeshNodes[33]);
  MeshNodes[30] = ComputeIntersection(MeshNodes[29], MeshNodes[33], 
				     MeshNodes[0], MeshNodes[34]);
  MeshNodes[31] = ComputeIntersection(MeshNodes[29], MeshNodes[33], 
				     MeshNodes[1], MeshNodes[35]);
  MeshNodes[32] = ComputeIntersection(MeshNodes[29], MeshNodes[33], 
				     MeshNodes[2], MeshNodes[36]);

  /* initialize the nodal coordinates in meter units */
  for (i = 0; i < nnodes; i++) {
    MeshNodes[i].xw = MeshNodes[i].xp * PixelSpacing;
    MeshNodes[i].yw = MeshNodes[i].yp * PixelSpacing;
  }
}

/********************** ComputeBicubicDerivatives **********************/
/*                                                                     */
/* Compute the bicubic derivative parameters from a bilinear model     */
/*                                                                     */
/***********************************************************************/
static void ComputeBicubicDerivatives(double *LN_SOLN, double *CB_SOLN)
{
  MeshElement ee, *left, *top, *right, *bot, *bleft, *tleft, *bright, *tright;
  double ddrvs[NRELEMENTS_MAX];
  int i, nn, node0, node1, node2, node3, ndone[NNODES_MAX];

  /* transfer the field values at the nodes to the bicubic soln vector */
  /* and mark nodes as not done */
  for (i = 0; i < nnodes; i++) {
    CB_SOLN[4*i] = LN_SOLN[i];
    CB_SOLN[4*i + 1] = 0;
    CB_SOLN[4*i + 2] = 0;
    CB_SOLN[4*i + 3] = 0;
    ndone[i] = 0;
  }

  /* calculate the second derivative terms for all rectangular elements */
  for (i = 0; i < nrelements; i++) {
    ee = MeshElements[i];
    ddrvs[i] =  (LN_SOLN[ee.nodes[0]] - LN_SOLN[ee.nodes[1]]
		 + LN_SOLN[ee.nodes[2]] - LN_SOLN[ee.nodes[3]]);
  }

  /* loop for all rectangular elements */
  for (i = 0; i < nrelements; i++) {
    /* fetch element info */
    ee = MeshElements[i];

    /* fetch element nodes indicies */
    node0 = ee.nodes[0];
    node1 = ee.nodes[1];
    node2 = ee.nodes[2];
    node3 = ee.nodes[3];

    /* fetch left neighbouring element info */
    if (ee.neighbours[0] != -1) {
      if (MeshElements[ee.neighbours[0]].type == R_TYPE) {
	left  = &(MeshElements[ee.neighbours[0]]);
      }
      else {
	left = NULL;
      }
    }
    else {
      left = NULL;
    }

    /* fetch top left neighbouring element info */
    if (ee.neighbours[1] != -1) {
      if (MeshElements[ee.neighbours[1]].type == R_TYPE) {
	tleft  = &(MeshElements[ee.neighbours[1]]);
      }
      else {
	tleft = NULL;
      }
    }
    else {
      tleft = NULL;
    }

    /* fetch top neighbouring element info */
    if (ee.neighbours[2] != -1) {
      if (MeshElements[ee.neighbours[2]].type == R_TYPE) {
	top  = &(MeshElements[ee.neighbours[2]]);
      }
      else {
	top = NULL;
      }
    }
    else {
      top = NULL;
    }

    /* fetch top right neighbouring element info */
    if (ee.neighbours[3] != -1) {
      if (MeshElements[ee.neighbours[3]].type == R_TYPE) {
	tright  = &(MeshElements[ee.neighbours[3]]);
      }
      else {
	tright = NULL;
      }
    }
    else {
      tright = NULL;
    }

    /* fetch right neighbouring element info */
    if (ee.neighbours[4] != -1) {
      if (MeshElements[ee.neighbours[4]].type == R_TYPE) {
	right  = &(MeshElements[ee.neighbours[4]]);
      }
      else {
	right = NULL;
      }
    }
    else {
      right = NULL;
    }

    /* fetch bottom right neighbouring element info */
    if (ee.neighbours[5] != -1) {
      if (MeshElements[ee.neighbours[5]].type == R_TYPE) {
	bright  = &(MeshElements[ee.neighbours[5]]);
      }
      else {
	bright = NULL;
      }
    }
    else {
      bright = NULL;
    }

    /* fetch bottom neighbouring element info */
    if (ee.neighbours[6] != -1) {
      if (MeshElements[ee.neighbours[6]].type == R_TYPE) {
	bot  = &(MeshElements[ee.neighbours[6]]);
      }
      else {
	bot = NULL;
      }
    }
    else {
      bot = NULL;
    }

    /* fetch left neighbouring element info */
    if (ee.neighbours[7] != -1) {
      if (MeshElements[ee.neighbours[7]].type == R_TYPE) {
	bleft  = &(MeshElements[ee.neighbours[7]]);
      }
      else {
	bleft = NULL;
      }
    }
    else {
      bleft = NULL;
    }

    /* bottom left node (node 0) */
    if (ndone[node0] == 0) {
      
      /* calculate dU/dxi1, at xi2 = 0 */
      if (left)
	CB_SOLN[node0*4 + 1] = (LN_SOLN[node3] - LN_SOLN[left->nodes[0]])/2.0;
      else if (bleft)
	CB_SOLN[node0*4 + 1] = (LN_SOLN[node3] - LN_SOLN[bleft->nodes[1]])/2.0;
      else 
	CB_SOLN[node0*4 + 1] = LN_SOLN[node3] - LN_SOLN[node0];

      /* calculate dU/dxi2, at xi1 = 0 */
      if (bot)
	CB_SOLN[node0*4 + 2] = (LN_SOLN[node1] - LN_SOLN[bot->nodes[0]])/2.0;
      else if (bleft)
	CB_SOLN[node0*4 + 2] = (LN_SOLN[node1] - LN_SOLN[bleft->nodes[3]])/2.0;
      else
	CB_SOLN[node0*4 + 2] = LN_SOLN[node1] - LN_SOLN[node0];

      /* calculate dUdU/dxi1dxi2 */
      CB_SOLN[node0*4 + 3] = ddrvs[i];
      nn = 1;
      if (bot) {
	CB_SOLN[node0*4 + 3] += ddrvs[ee.neighbours[6]];
	nn++;
      }
      if (bleft) {
	CB_SOLN[node0*4 + 3] += ddrvs[ee.neighbours[7]];
	nn++;
      }
      if (left) {
	CB_SOLN[node0*4 + 3] += ddrvs[ee.neighbours[0]];
	nn++;
      }
      CB_SOLN[node0*4 + 3] /= ((double) nn);

      /* mark the node as done */
      ndone[node0] = 1;
    }

    /* top left node (node 1) */
    if (ndone[node1] == 0) {
      
      /* calculate dU/dxi1, at xi2 = 0 */
      if (left)
	CB_SOLN[node1*4 + 1] = (LN_SOLN[node2] - LN_SOLN[left->nodes[1]])/2.0;
      else if (tleft)
	CB_SOLN[node1*4 + 1] = (LN_SOLN[node2] - LN_SOLN[tleft->nodes[0]])/2.0;
      else
	CB_SOLN[node1*4 + 1] = LN_SOLN[node2] - LN_SOLN[node1];

      /* calculate dU/dxi2, at xi1 = 0 */
      if (top)
	CB_SOLN[node1*4 + 2] = (LN_SOLN[top->nodes[1]] - LN_SOLN[node0])/2.0;
      else if (tleft)
	CB_SOLN[node1*4 + 2] = (LN_SOLN[tleft->nodes[2]] - LN_SOLN[node0])/2.0;
      else
	CB_SOLN[node1*4 + 2] = LN_SOLN[node1] - LN_SOLN[node0];

      /* calculate dUdU/dxi1dxi2 */
      CB_SOLN[node1*4 + 3] = ddrvs[i];
      nn = 1;
      if (left) {
	CB_SOLN[node1*4 + 3] += ddrvs[ee.neighbours[0]];
	nn++;
      }
      if (tleft) {
	CB_SOLN[node1*4 + 3] += ddrvs[ee.neighbours[1]];
	nn++;
      }
      if (top) {
	CB_SOLN[node1*4 + 3] += ddrvs[ee.neighbours[2]];
	nn++;
      }
      CB_SOLN[node1*4 + 3] /= ((double) nn);

      /* mark the node as done */
      ndone[node1] = 1;
    }

    /* top right node (node 1) */
    if (ndone[node2] == 0) {
      
      /* calculate dU/dxi1, at xi2 = 0 */
      if (right)
	CB_SOLN[node2*4 + 1] = (LN_SOLN[right->nodes[2]] - LN_SOLN[node1])/2.0;
      else if (tright)
	CB_SOLN[node2*4 + 1]= (LN_SOLN[tright->nodes[3]] - LN_SOLN[node1])/2.0;
      else
	CB_SOLN[node2*4 + 1] = LN_SOLN[node2] - LN_SOLN[node1];

      /* calculate dU/dxi2, at xi1 = 0 */
      if (top)
	CB_SOLN[node2*4 + 2] = (LN_SOLN[top->nodes[2]] - LN_SOLN[node3])/2.0;
      else if (tright)
	CB_SOLN[node2*4 + 2]= (LN_SOLN[tright->nodes[1]] - LN_SOLN[node3])/2.0;
      else
	CB_SOLN[node2*4 + 2] = LN_SOLN[node2] - LN_SOLN[node3];

      /* calculate dUdU/dxi1dxi2 */
      CB_SOLN[node2*4 + 3] = ddrvs[i];
      nn = 1;
      if (top) {
	CB_SOLN[node2*4 + 3] += ddrvs[ee.neighbours[2]];
	nn++;
      }
      if (tright) {
	CB_SOLN[node2*4 + 3] += ddrvs[ee.neighbours[3]];
	nn++;
      }
      if (right) {
	CB_SOLN[node2*4 + 3] += ddrvs[ee.neighbours[4]];
	nn++;
      }
      CB_SOLN[node2*4 + 3] /= ((double) nn);

      /* mark the node as done */
      ndone[node2] = 1;
    }

    /* bottom right node (node 1) */
    if (ndone[node3] == 0) {
      
      /* calculate dU/dxi1, at xi2 = 0 */
      if (right)
	CB_SOLN[node3*4 + 1] = (LN_SOLN[right->nodes[3]] - LN_SOLN[node0])/2.0;
      else if (bright)
	CB_SOLN[node3*4 + 1]= (LN_SOLN[bright->nodes[2]] - LN_SOLN[node0])/2.0;
      else
	CB_SOLN[node3*4 + 1] = LN_SOLN[node3] - LN_SOLN[node0];

      /* calculate dU/dxi2, at xi2 = 1 */
      if (bot)
	CB_SOLN[node3*4 + 2] = (LN_SOLN[node2] - LN_SOLN[bot->nodes[3]])/2.0;
      else if (bright)
	CB_SOLN[node3*4 + 2]= (LN_SOLN[node2] - LN_SOLN[bright->nodes[0]])/2.0;
      else
	CB_SOLN[node3*4 + 2] = LN_SOLN[node2] - LN_SOLN[node3];

      /* calculate dUdU/dxi1dxi2 */
      CB_SOLN[node3*4 + 3] = ddrvs[i];
      nn = 1;
      if (right) {
	CB_SOLN[node3*4 + 3] += ddrvs[ee.neighbours[4]];
	nn++;
      }
      if (bright) {
	CB_SOLN[node3*4 + 3] += ddrvs[ee.neighbours[5]];
	nn++;
      }
      if (bot) {
	CB_SOLN[node3*4 + 3] += ddrvs[ee.neighbours[6]];
	nn++;
      }
      CB_SOLN[node3*4 + 3] /= ((double) nn);

      /* mark the node as done */
      ndone[node3] = 1;
    }
  }
}
  
/************************** ComputeJacobianIverse **********************/
/*                                                                     */
/* Compute the jacobian inverse elements for a sample                  */
/*                                                                     */
/***********************************************************************/
static void ComputeJacobianInverse(FESample *s, MeshNode nodes[],
				   double *dxi1dX, double *dxi2dX,
				   double *dxi1dY, double *dxi2dY)
{
  MeshElement e;
  double X1, X2, X3, X4, Y1, Y2, Y3, Y4;
  double dXdxi1, dXdxi2, dYdxi1, dYdxi2, detJInv;

  /* load the element fixed nodal coordinates */
  e = MeshElements[s->element];
  X1 = nodes[e.nodes[0]].xp;
  Y1 = nodes[e.nodes[0]].yp;
  X2 = nodes[e.nodes[1]].xp;
  Y2 = nodes[e.nodes[1]].yp;
  X3 = nodes[e.nodes[2]].xp;
  Y3 = nodes[e.nodes[2]].yp;
  if (e.type == R_TYPE) {
    X4 = nodes[e.nodes[3]].xp;
    Y4 = nodes[e.nodes[3]].yp;
  }

  /* compute partial derivatives w/r to the normalized coords */
  if (e.type == R_TYPE) {
    dXdxi1 = s->v * (X1 - X2 + X3 - X4) - X1 + X4;
    dXdxi2 = s->u * (X1 - X2 + X3 - X4) - X1 + X2;
    dYdxi1 = s->v * (Y1 - Y2 + Y3 - Y4) - Y1 + Y4;
    dYdxi2 = s->u * (Y1 - Y2 + Y3 - Y4) - Y1 + Y2;
  }
  else {
    dXdxi1 = X1 - X3;
    dXdxi2 = X2 - X3;
    dYdxi1 = Y1 - Y3;
    dYdxi2 = Y2 - Y3;
  }
    
  /* compute the Jacobian inverse */
  detJInv = 1.0 / (dXdxi1 * dYdxi2 - dXdxi2 * dYdxi1);
  *dxi1dX =  dYdxi2 * detJInv;
  *dxi1dY = -dXdxi2 * detJInv;
  *dxi2dX = -dYdxi1 * detJInv;
  *dxi2dY =  dXdxi1 * detJInv;
}
  
/******************** ComputePixelMaterialCoords ***********************/
/*                                                                     */
/* Routine which computes the linear interpolated coords for a sample  */
/*                                                                     */
/***********************************************************************/
static void ComputePixelMaterialCoords(FESample s, boolean_t vflag,
				       double *xh, double *yh)
{
  double X1, X2, X3, X4, Y1, Y2, Y3, Y4;

  /* compute the coordinate estimates */
  X1 = MeshNodes[MeshElements[s.element].nodes[0]].xp;
  Y1 = MeshNodes[MeshElements[s.element].nodes[0]].yp;
  X2 = MeshNodes[MeshElements[s.element].nodes[1]].xp;
  Y2 = MeshNodes[MeshElements[s.element].nodes[1]].yp;
  X3 = MeshNodes[MeshElements[s.element].nodes[2]].xp;
  Y3 = MeshNodes[MeshElements[s.element].nodes[2]].yp;
  if (MeshElements[s.element].type == R_TYPE) {
    X4 = MeshNodes[MeshElements[s.element].nodes[3]].xp;
    Y4 = MeshNodes[MeshElements[s.element].nodes[3]].yp;
    *xh =X1*(1-s.u)*(1-s.v) +X2*(1-s.u)*s.v +X3*s.u*s.v +X4*s.u*(1-s.v);
    *yh =Y1*(1-s.u)*(1-s.v) +Y2*(1-s.u)*s.v +Y3*s.u*s.v +Y4*s.u*(1-s.v);
    if (vflag) {
      printf("  Nodes                : (%.2f,%.2f),(%.2f,%.2f),(%.2f,%.2f),(%.2f,%.2f)\n", X1, Y1, X2, Y2, X3, Y3, X4, Y4);
      printf("  Element coordinates  : %f, %f\n", s.u, s.v);
    }
  }
  else {
    *xh = X1*((double) s.Phi[0]) + X2*((double) s.Phi[1])
      + X3*((double) s.Phi[2]);
    *yh = Y1*((double) s.Phi[0]) + Y2*((double) s.Phi[1])
      + Y3*((double) s.Phi[2]);
    if (vflag) {
      printf("  Nodes                : (%.2f,%.2f),(%.2f,%.2f),(%.2f,%.2f)\n",
	     X1, Y1, X2, Y2, X3, Y3);
      printf("  Element coordinates  : %f, %f\n", s.Phi[0], s.Phi[1]);
    }
  }
/*
#ifdef DEBUG
  printf("ComputePixelMaterialCoords() - element %d (%.2f,%.2f) at (%.2f,%.2f)\n",
	 s.element, s.u, s.v, *xh, *yh);
#endif
*/
}

/********************* ComputePixelXiCoords ****************************/
/*                                                                     */
/* Routine which compute the element coordinates of a pixel location.  */
/* and the associated matrix system parameter coefficients.            */
/*                                                                     */
/***********************************************************************/
static boolean_t ComputePixelXiCoords(FESample *s, int x, int y,
				      boolean_t *ColdStart,
				      int field_model_type)
{
  static MeshElement e;
  static double A, B, C, D, E, F, G, H;
  static int element;
  double X1, X2, X3, X4, Y1, Y2, Y3, Y4, a, b, c, T1, T2, u1, v1, u2, v2;
  double dxi1dX, dxi2dX, dxi1dY, dxi2dY,
    H01_U, H01_V, H02_U, H02_V, H11_U, H11_V, H12_U, H12_V;
  int i;

  /* fetch element nodal coordinates if required */
  if ((s->element != element) || (*ColdStart)) {
    element = s->element;
    e = MeshElements[element];
    if (e.type == R_TYPE) {                           /* rectangular element */
      X1 = MeshNodes[((e.nodes)[0])].xp; Y1 = MeshNodes[((e.nodes)[0])].yp;
      X2 = MeshNodes[((e.nodes)[1])].xp; Y2 = MeshNodes[((e.nodes)[1])].yp;
      X3 = MeshNodes[((e.nodes)[2])].xp; Y3 = MeshNodes[((e.nodes)[2])].yp;
      X4 = MeshNodes[((e.nodes)[3])].xp; Y4 = MeshNodes[((e.nodes)[3])].yp;
      A =  X1;
      B = -X1 + X4;
      C = -X1 + X2;
      D =  X1 - X2 + X3 -X4;
      E =  Y1;
      F = -Y1 + Y4;
      G = -Y1 + Y2;
      H =  Y1 - Y2 + Y3 - Y4;
    }
    else {                                             /* triangular element */
      X1 = MeshNodes[e.nodes[0]].xp; Y1 = MeshNodes[e.nodes[0]].yp;
      X2 = MeshNodes[e.nodes[1]].xp; Y2 = MeshNodes[e.nodes[1]].yp;
      X3 = MeshNodes[e.nodes[2]].xp; Y3 = MeshNodes[e.nodes[2]].yp;
      A = (X2*Y3 - X3*Y2);
      B = Y2 - Y3;
      C = X3 - X2;
      D = 1.0/(A + X1*B + Y1*C);
      E = (X3*Y1 - X1*Y3);
      F = Y3 - Y1;
      G = X1 - X3;
    }
    *ColdStart = B_FALSE;
  }

  if (e.type == R_TYPE) {                             /* rectangular element */
    /* compute the Xi equation roots */
    if (((fabs(X1 - X2) < PRECISION) && (fabs(X3 - X4) < PRECISION))
	|| ((fabs(Y1 - Y4) < PRECISION) && (fabs(Y2 - Y3) < PRECISION))) {
      u1 = (((double) x) - A)/B;
      v1 = (((double) y) - E)/G;
    }
    else {
      a = D*F - B*H;
      b = H*(((double) x) - A) - D*(((double) y) - E) + C*F - G*B;
      c = G*(((double) x) - A) - C*(((double) y) - E);
      T1 = -b / (2.0 * a);
      T2 = sqrt(b*b - 4.0*a*c)/(2.0 * a);
      u1 = T1 - T2;
      v1 = (((double) x) - A - u1 * B) / (C + u1 * D);
      u2 = T1 + T2;
      v2 = (((double) x) - A - u2 * B) / (C + u2 * D);
    }
    
    /* check the roots to find the coordinates in the range 0..1 */
    if ((u1 >= 0.0) && (u1 <= 1.0) && (v1 >= 0.0) && (v1 <= 1.0)) {
      s->u = u1;
      s->v = v1;
    }
    else if ((u2 >= 0.0) && (u2 <= 1.0) && (v2 >= 0.0) && (v2 <= 1.0)) {
      s->u = u2;
      s->v = v2;
    }
    else {
      return(B_FALSE);
    }
    
    /* bicubic field interpolation */
    if (field_model_type == BICUBIC_FIELD_MODEL) {
      /* precompute bicubic hermite basis function values */
      H01_U = H01(s->u);
      H02_U = H02(s->u);
      H11_U = H11(s->u);
      H12_U = H12(s->u);
      H01_V = H01(s->v);
      H02_V = H02(s->v);
      H11_V = H11(s->v);
      H12_V = H12(s->v);
      
      s->Phi[0]  = H01_U * H01_V;  /* u1 (U1 in CMISS) */
      s->Phi[1]  = H11_U * H01_V;  /* du1/dxi1 */
      s->Phi[2]  = H01_U * H11_V;  /* du1/dxi2 */
      s->Phi[3]  = H11_U * H11_V;  /* du1du1/dxi1dxi2 */
      
      s->Phi[4]  = H01_U * H02_V;  /* u2 (U3 in CMISS) */
      s->Phi[5]  = H11_U * H02_V;  /* du2/dxi1 */
      s->Phi[6]  = H01_U * H12_V;  /* du2/dxi2 */
      s->Phi[7]  = H11_U * H12_V;  /* du2du2/dxi1dxi2 */
      
      s->Phi[8]  = H02_U * H02_V;  /* u3 (U4 in CMISS) */
      s->Phi[9]  = H12_U * H02_V;  /* du3/dxi1 */
      s->Phi[10] = H02_U * H12_V;  /* du3/dxi2 */
      s->Phi[11] = H12_U * H12_V;  /* du3du3/dxi1dxi2 */
      
      s->Phi[12] = H02_U * H01_V;  /* u4 (U2 in CMISS) */
      s->Phi[13] = H12_U * H01_V;  /* du4/dxi1 */
      s->Phi[14] = H02_U * H11_V;  /* du4/dxi2 */
      s->Phi[15] = H12_U * H11_V;  /* du4du4/dxi1dxi2 */
    }

    /* bilinear field interpolation */
    else {
      /* precompute bilinear basis function values */
      s->Phi[0]  = (1 - s->u)*(1 - s->v);
      s->Phi[1]  = (1 - s->u) * s->v;
      s->Phi[2]  = s->u * s->v;
      s->Phi[3]  = s->u * (1 - s->v);
    }
  }
  else {                                             /* triangular element */
    /* compute the L1, L2, L3 linear triangular coordinates */
    s->Phi[0]  = (A + B*((double) x) + C*((double) y))*D;    /*L1            */
    s->Phi[1]  = (E + F*((double) x) + G*((double) y))*D;    /*L2            */
    s->Phi[2]  = 1.0 - s->Phi[0] - s->Phi[1];                /*1- L1 - L2    */
    s->Phi[3]  = 0;
    if (field_model_type == BICUBIC_FIELD_MODEL) {
      for (i = 4; i < 16; i++)
	s->Phi[i]  = 0;
    }

    /* check to make sure they are in the range 0..1 */
    if (!((s->Phi[0] >= 0.0) && (s->Phi[0] <= 1.0))
	|| !((s->Phi[1] >= 0.0) && (s->Phi[1] <= 1.0))
	|| !((s->Phi[2] >= 0.0) && (s->Phi[2] <= 1.0)))
      return(B_FALSE);
  }
  
  /* increment the sample count in the element structure */
  MeshElements[element].nsamples++;
  
  /* return success */
  return(B_TRUE);
}  

/*********************** ComputeAllPixelsXiCoords **********************/
/*                                                                     */
/* Routine which compute the element coordinates of each pixel within  */
/* the mesh.                                                           */
/*                                                                     */
/***********************************************************************/
static void ComputeAllPixelsXiCoords(boolean_t ErrorCheck,
				     boolean_t GenMesh,
				     int field_model_type)
{
  FESample s;
  int x, y, k, n, m, i, BoundarySamplesCancelled,
  SamplesReparented, SamplesCancelled;
  double rx, ry, t, xh, yh;
  boolean_t Dummy, rc, BoundaryElement;
  

  /* generate the mesh element template */
  if (GenMesh) {
    if (VerboseFlag)
      printf("ComputeAllPixelsXiCoords(): Computing element indicies...\n");
    GenerateMesh(B_TRUE);
    if (Abort) {
      return;
    }
  }
  
  /* show message */
  if (VerboseFlag)
    printf("ComputeAllPixelsXiCoords(): Computing element coordinates...\n");

  /* loop for all pixel positions whithin the mesh boundary, not on */
  /* an element boundary                                            */
  for (y = 0, k = 0, n = 0, m = 0, Dummy=B_TRUE, SamplesReparented = 0,
       BoundarySamplesCancelled=0, SamplesCancelled = 0; y < ImgHeight; y++)
    for (x = 0; x < ImgWidth; x++, k++)
      if (FEMeshPixels[k] >= 0) {
	/* load this sample into local storage (else it crashes if optimized)*/
	s = FieldSamples[k];
	
	/* compute the element coordinates & stuff of this sample */
	s.element = FEMeshPixels[k];
	rc = ComputePixelXiCoords(&s, x, y, &Dummy, field_model_type);
	n++;
	
	/* check if the Xi coordinates are outside the 0..1 range */
	/* if so, search the neighbours for the correct element   */
	if (rc == B_FALSE) {
	  for (i = 0, BoundaryElement=B_FALSE; (i < 8) && (rc == B_FALSE);
	       i++) {
	    s.element = MeshElements[(FEMeshPixels[k])].neighbours[i];
	    if (s.element != -1)
	      rc = ComputePixelXiCoords(&s, x, y, &Dummy, field_model_type);
	    else
	      BoundaryElement = B_TRUE;
	  }
	  if (i < 8) {
	    FEMeshPixels[k] = s.element;
	    SamplesReparented++;
	    m++;
	  }
	  else {
	    FEMeshPixels[k] = -1;
	    SamplesCancelled++;
	    if (BoundaryElement)
	      BoundarySamplesCancelled++;
	  }
	}
	else {
	  m++;
	}
	
	/* write the updated sample info */
	FieldSamples[k] = s;
	
	/* check that the computation was successfull if required */
	if ((ErrorCheck) && (FEMeshPixels[k] != -1)) {
	  /* compute coordinate estimates */
	  ComputePixelMaterialCoords(s, B_FALSE, &xh, &yh);
	  
	  /* compute residuals */
	  rx = xh - ((double) x);
	  ry = yh - ((double) y);
	  if ((fabs(rx) > 0.5) || (fabs(ry) > 0.5)) {
	    printf("ComputeAllPixelsXiCoords() residual error:\n");
	    printf("  Element              : %d\n", s.element);
	    printf("  Element neighbors    : %d, %d, %d, %d, %d, %d, %d, %d\n",
		   MeshElements[s.element].neighbours[0],
		   MeshElements[s.element].neighbours[1],
		   MeshElements[s.element].neighbours[2],
		   MeshElements[s.element].neighbours[3],
		   MeshElements[s.element].neighbours[4],
		   MeshElements[s.element].neighbours[5],
		   MeshElements[s.element].neighbours[6],
		   MeshElements[s.element].neighbours[7]);
	    printf("  Sample number        : %d\n", n);
	    printf("  Sample coordinates   : %d, %d\n", x, y);
	    printf("  Coordinate estimates : %e, %e\n", xh, yh);
	    printf("  Residuals            : %e, %e\n", rx, ry);
	    if (MeshElements[s.element].type == R_TYPE) {
	      printf("  Nodes                : (%.2f,%.2f), (%.2f,%.2f), (%.2f,%.2f), (%.2f,%.2f)\n",
		     MeshNodes[MeshElements[s.element].nodes[0]].xp,
		     MeshNodes[MeshElements[s.element].nodes[0]].yp,
		     MeshNodes[MeshElements[s.element].nodes[1]].xp,
		     MeshNodes[MeshElements[s.element].nodes[1]].yp,
		     MeshNodes[MeshElements[s.element].nodes[2]].xp,
		     MeshNodes[MeshElements[s.element].nodes[2]].yp,
		     MeshNodes[MeshElements[s.element].nodes[3]].xp,
		     MeshNodes[MeshElements[s.element].nodes[3]].yp);
	      printf("  Element coordinates  : %e, %e\n", s.u, s.v);
	    }
	    else {
	      printf("  Nodes                : (%.2f,%.2f), (%.2f,%.2f), (%.2f,%.2f)\n",
		     MeshNodes[MeshElements[s.element].nodes[0]].xp,
		     MeshNodes[MeshElements[s.element].nodes[0]].yp,
		     MeshNodes[MeshElements[s.element].nodes[1]].xp,
		     MeshNodes[MeshElements[s.element].nodes[1]].yp,
		     MeshNodes[MeshElements[s.element].nodes[2]].xp,
		     MeshNodes[MeshElements[s.element].nodes[3]].yp);
	      printf("  Element coordinates  : %e, %e\n", s.Phi[0], s.Phi[1]);
	    }
	  }
	}
      }

  /* print out results */
  if (VerboseFlag)
    printf("ComputeAllPixelsXiCoords(): %d/%d samples, reparented: %d, rejected: %d (%d in bndry el)\n",
	   m,n, SamplesReparented, SamplesCancelled, BoundarySamplesCancelled);
}

/* draw the FE markers */
void DrawFEMarkers(Widget DrawingArea)
{
  int i;
  double xh, yh;

  for (i = 0; i < NFEMarkers; i++) {
    ComputePixelMaterialCoords(FEMarkers[i], B_FALSE, &xh, &yh);
    xh *= ZoomFactor;
    yh *= ZoomFactor;
    XDrawArc(UxDisplay, XtWindow(DrawingArea), gc_YellowBlack,
	     ((int) xh) -2, ((int) yh)-2, 5, 5, 0, 360*64 -1);
  }
  XSync(UxDisplay, B_FALSE);
}

/*********************** PickMeshCoordinates ***************************/
/*                                                                     */
/* Routine which compute the element coordinates of a pixel at the     */
/* current mouse position.                                             */
/*                                                                     */
/***********************************************************************/
void PickMeshCoordinate(int x, int y)
{
  FESample s;
  boolean_t rc, Dummy;
  double xh, yh;

  if (FEMeshPixels[y*ImgWidth + x] >= 0) {
    /* compute the element coordinates of this sample */
    s = FieldSamples[y*ImgWidth + x];
    s.element = FEMeshPixels[y*ImgWidth + x];
    rc = ComputePixelXiCoords(&s, x, y, &Dummy, BILINEAR_FIELD_MODEL);

    /* print out header */
    printf("FE Sample info:\n");
    printf("  Element              : %d\n", s.element);

    /* compute interpolated coordinates */
    ComputePixelMaterialCoords(s, B_TRUE, &xh, &yh);

    /* print the rest and check for element coordinate computation error */
    printf("  Sample coordinates   : %d, %d\n", x, y);
    printf("  Coordinate estimates : %f, %f\n", xh, yh);
    if (rc == B_FALSE)
      printf("\aWARNING : Element coordinates are outside the range 0..1\n");

    /* record this marker */
    if (NFEMarkers < NFEMarkersMAX)
      FEMarkers[NFEMarkers++] = s;
  }
  else
    printf("FE Sample info: Sample is not in any element!\n");
}


/************************** ComputeSamplePartialSums *******************/
/*                                                                     */
/* Routine which computes the partial sums for a particular sample     */
/* and adds them to the current sums in the matrix system.             */
/*                                                                     */
/***********************************************************************/
static void ComputeSamplePartialSums(FESample s, double f, double w,
				     double *A, double *B,
				     int nparms, int nfringes,
				     int fringe_order, double offset,
				     int model_type)
{
  MeshElement e;
  int i, j, k, n, m, l, r, extent;

  /* A matrix width (it's square) */
  extent = nparms + nfringes;
  e = MeshElements[s.element];

  /* bicubic model for the field fit */
  if (model_type == BICUBIC_FIELD_MODEL) {
    /* add the partial sums to the running sums in the stiffness matrix */
    /* for the mesh node parameters */
    if (e.type == R_TYPE) {
      /* rectangular element, bicubic hermite basis functions */
      for (n=0, l=0; n < 4; n++) {           /* loop for the 4 corner nodes */
	for (m=0; m < 4; m++, l++) {  /* loop for the 4 parameters per node */
	  /* column index in A for parameter "l" */
	  r = 4*(e.nodes[n]) + m;
	  /* loop to multiply all 16 basis functions with basis function "l" */
	  for (j=0, k=0; j < 4; j++) {
	    for (i=4*e.nodes[j]*extent + r; i < ((4*e.nodes[j] + 4)*extent +r);
		 i+=extent, k++) {
	      A[i] += (s.Phi[k] * s.Phi[l] * w);
	    }
	  }
	  /* if the model include a fringe order, include this term */
	  if (fringe_order >= 0) {
	    A[(nparms + fringe_order)*extent + r] -= (s.Phi[l] * TWOPI * w);
	  }
	  /* right column vector term */
	  B[r] += (s.Phi[l] * (f + offset) * w);
	}
      }
      
      /* add the partial sums to the running sums in the stiffness matrix */
      /* for the region fringe order parameters, if required */
      if (fringe_order >= 0) {
	r = nparms + fringe_order;
	/* loop to multiply all 16 basis functions by 2PI */
	for (j=0, k=0; j < 4; j++) {
	  for (i=4*e.nodes[j]*extent + r; i < ((4*e.nodes[j] + 4)*extent + r);
	       i+=extent, k++) {
	    A[i] += (s.Phi[k] * w);
	  }
	}
	/* include the fringe order term */
	A[(nparms + fringe_order)*extent + r] -= (TWOPI * w);
	/* right column vector term */
	B[r] += (f * w);
      }
    }
    else {
      /* triangular element, linear triangle elements */
      for (j = 0; j < 3; j++) {
	r = 4*(e.nodes[j]);
	for (i = 0; i < 3; i++) {
	  A[4*(e.nodes[i])*extent + r] += (s.Phi[j] * s.Phi[i] * w);
	}
	if (fringe_order >= 0)
	  A[(nparms + fringe_order)*extent + r] -= (s.Phi[j] * TWOPI * w);
	B[r] += (s.Phi[j] * (f + offset) * w);
      }
      
      /* add the partial sums to the running sums in the stiffness matrix */
      /* for the region fringe order parameters, if required */
      if (fringe_order >= 0) {
	r = nparms + fringe_order;
	for (i = 0; i < 3; i++) {
	  A[4*(e.nodes[i])*extent + r] += (s.Phi[i] * w);
	}
	A[(nparms + fringe_order)*extent + r] -= (TWOPI * w);
	B[r] += (f * w);
      }
    }
  }
  
  /* else bilinear model */
  else {
    /* add the partial sums to the running sums in the stiffness matrix */
    /* for the mesh node parameters */
    for (j = 0; j < (e.type == R_TYPE ? 4 : 3); j++) {
      r = e.nodes[j];
      for (i = 0; i < (e.type == R_TYPE ? 4 : 3); i++)
	A[e.nodes[i] * extent + r] += (s.Phi[j] * s.Phi[i] * w);
      if (fringe_order >= 0)
	A[(nparms + fringe_order)*extent + r] -= (s.Phi[j] * TWOPI * w);
      B[r] += (s.Phi[j] * (f + offset) * w);
    }
    
    /* add the partial sums to the running sums in the stiffness matrix */
    /* for the region fringe order parameters, if required */
    if (fringe_order >= 0) {
      r = nparms + fringe_order;
      for (i = 0; i < (e.type == R_TYPE ? 4 : 3); i++)
	A[e.nodes[i] * extent + r] += (s.Phi[i] * w);
      A[(nparms + fringe_order)*extent + r] -= (TWOPI * w);
      B[r] += (f * w);
    }
  }
}

/*************************** ModelEval *********************************/
/*                                                                     */
/* Evaluate the fit at a point.                                        */
/*                                                                     */
/***********************************************************************/
static double ModelEval(FESample s, double *SOLN, int model_type)
{
  MeshElement e;
  double estimate;
  int i, j, k;
  
  e = MeshElements[s.element];

  /* bicubic model of the field */
  if (model_type == BICUBIC_FIELD_MODEL) {
    if (e.type == R_TYPE) {
      for (j=0, k=0, estimate=0; j < 4; j++)
	for (i = 4*e.nodes[j]; i < (4*e.nodes[j] + 4); i++, k++)
	  estimate += (SOLN[i] * ((double) s.Phi[k]));
    }
    else {
      for (i=0, estimate=0; i < 3; i++)
	estimate += (SOLN[4*(e.nodes[i])] * ((double) s.Phi[i]));
    }
  }

  /* bilinear model of the field */
  else {
    for (i=0, estimate=0; i < (e.type == R_TYPE ? 4 : 3); i++)
      estimate += (SOLN[e.nodes[i]] * ((double) s.Phi[i]));
  }

  return(estimate);
}

/*************************** OBJFUN ************************************/
/*                                                                     */
/* User supplied function for the e04ucf NAG call which evaluates the  */
/* objective function and its gradient at a point.                     */
/*                                                                     */
/* Note: IUSER[0] contains the number of samples                       */
/*       IUSER[1] contains the field interpolation type                */
/*       IUSER[2] contains the weights data pointer is any             */
/*        USER    points to the field data                             */
/*                                                                     */
/***********************************************************************/
static void OBJFUN(int *MODE, int *N, double *X, double *OBJ, double *OBJGRD, 
		   int *NSTATE, int *IUSER, double *USER)
{
  MeshElement e;
  FESample s;
  double residual, residual_x2, estimate, ObjFun, v, *weights, w;
  int i, j, k, p;

  /* init weights pointer */
  weights = (double *) IUSER[2];

  /* null gradient components if required */
  if ((*MODE == 1) || (*MODE == 2))
    {
      for (k = 0; k < *N; k++)
	{
	  OBJGRD[k] = 0;
	}
    }
  
  /* Compute the objective function and gradient at point X */
  for (k = 0, ObjFun = 0; (k < IUSER[0]) && (Abort == B_FALSE); k++)
    {
      if (FEMeshPixels[k] >= 0)
	{
	  /* load sample into local storage */
	  s = FieldSamples[k];

	  /* load the weight is any */
	  w = (weights ? (weights[k]*weights[k]) : 1.0);
	  
	  /* compute the unwrapped model estimate at field location (x,y) */
	  estimate = ModelEval(s, X, IUSER[1]);
	  
	  /* compute the wrapped weighted residual */
	  residual = estimate - USER[k];
	  residual = (residual - rint(residual/TWOPI)*TWOPI);
	  
	  /* add to residual squared to the running objective function sum */
	  ObjFun += (residual * residual * w);
	  
	  /* add to gradient components running sums */
	  if ((*MODE == 1) || (*MODE == 2))
	    {
	      e = MeshElements[s.element];
	      residual_x2 = residual * 2.0 * w;
	      
	      /* bicubic field m model */
	      if (IUSER[1] == BICUBIC_FIELD_MODEL) {
		if (e.type == R_TYPE) {
		  for (j = 0, p =0; j < 4; j++) {
		    for (i = 4*e.nodes[j]; i < (4*e.nodes[j] + 4); i++, p++)
		      OBJGRD[i] += (residual_x2 * s.Phi[p]);
		  }
		}
		else {
		  for (i = 0; i < 3; i++)
		    OBJGRD[4*e.nodes[i]] += (residual_x2 * s.Phi[i]);
		}
	      }
	      
	      /* bilinear field model */
	      else {
		OBJGRD[e.nodes[0]] += (residual_x2 * s.Phi[0]);
		OBJGRD[e.nodes[1]] += (residual_x2 * s.Phi[1]);
		OBJGRD[e.nodes[2]] += (residual_x2 * s.Phi[2]);
		OBJGRD[e.nodes[3]] += (residual_x2 * s.Phi[3]);
	      }
	    }
	}
    }
  
  /* store initial scaling factor if objective function is > 1.0 */
  /* fitting works much better if objective function is scaled   */
  /* to a reasonable value */
  if (*NSTATE == 1)
    {
      if (fabs(ObjFun) > 1.0)
	{
	  ObjFunScale = 1.0 / ObjFun;
	}
      else
	{
	  ObjFunScale = 1.0;
	}
      if (VerboseFlag) {
	printf("\nOBJFUN() => Initial objective function value = %.8e\n",
	       ObjFun);
	fflush(stdout);
      }
    }
  
  /* return the scaled objective function if required */
  if ((*MODE == 0) || (*MODE == 2))
    {
      *OBJ = ObjFun * ObjFunScale;
    }

  /* return the scaled objective gradient components if required */
  if ((*MODE == 1) || (*MODE == 2))
    {
      for (k = 0; k < *N; k++)
	{
	  OBJGRD[k] *= ObjFunScale;
	}
    }
  
  /* check for user abort */
  if (Abort)
    {
      *MODE = -1;
    }
}

/************************* FECompute (utilities) ***********************/
/*                                                                     */
/*                                                        .            */
/*                                                                     */
/***********************************************************************/
static void ComputeSumOfSquaredResiduals(double *field, double *weights,
					 int width, int height,
					 double *SOLN, int FieldModelType,
					 double *ssr, double *erms,
					 boolean_t WrapFlag)
				  
{
  double r, n, v, w;
  int k;
  
  for (k = 0, v=0, n=0; k < width*height; k++)
    if (FEMeshPixels[k] >= 0) {
      if (MeshElements[FieldSamples[k].element].type == R_TYPE) {
	w = (weights ? (weights[k]*weights[k]) : 1.0);
	r = ModelEval(FieldSamples[k], SOLN, FieldModelType) - field[k];
	if (WrapFlag)
	  r = r - rint(r/TWOPI)*TWOPI;
	/* add to running sums for VAF computation */
	v += (r*r*w);
	/* increment probability counter */
	n += w;
      }
    }
  *ssr = v;
  *erms = sqrt(v/n);
}


static boolean_t NonLinearFieldFit(int width, int height, int NPARMS,
				 double *xfield, double *yfield,
				 double *xweights, double *yweights,
				 FILE *LogFileFp, int FieldModelType,
				 double *XSOLN, double *YSOLN)
{
#ifdef _E04UCF_OPTIONAL_PARAMETERS
#ifdef SGI
#define e04uef_defaults         e04uef_defaults_
#define e04uef_func_prec        e04uef_func_prec_
#define e04uef_verify           e04uef_verify_
#define e04uef_max_iters_2000   e04uef_max_iters_2000_
#define e04uef_max_iters_10     e04uef_max_iters_10_
#endif
  extern void e04uef_defaults(void);
  extern void e04uef_func_prec(void);
  extern void e04uef_verify(void);
  extern void e04uef_max_iters_2000(void);
#endif

  static double *BL=NULL, *BU=NULL, *X=NULL, *Y=NULL, *G=NULL, *W=NULL,
  *CLAMBDA=NULL, *WORK=NULL, *R=NULL;
  static int *ISTATE=NULL, *IWORK=NULL;
  FILE *fp;
  double C[1], CJAC[1][1], g, F;
  int IUSER[3], ZERO, ONE, LWORK, IW[2], LIWORK, i, ITER, IFAIL;

  /* allocate storage */
  X  = (double *) XvgMM_Alloc((void *) X,  NPARMS * sizeof(double));
  Y  = (double *) XvgMM_Alloc((void *) Y,  NPARMS * sizeof(double));
  BL = (double *) XvgMM_Alloc((void *) BL, NPARMS * sizeof(double));
  BU = (double *) XvgMM_Alloc((void *) BU, NPARMS * sizeof(double));
  G  = (double *) XvgMM_Alloc((void *) G,  NPARMS * sizeof(double));
  W  = (double *) XvgMM_Alloc((void *) W,
			      7*NPARMS + NPARMS*(NPARMS-1)/2
			      * sizeof(double));
  R  = (double *) XvgMM_Alloc((void *) R,    NPARMS * NPARMS * sizeof(double));
  CLAMBDA = (double *) XvgMM_Alloc((void *) CLAMBDA,
				   NPARMS * sizeof(double));
  LWORK = 20*NPARMS;
  WORK = (double *) XvgMM_Alloc((void *) WORK, LWORK * sizeof(double));
  ISTATE = (int *) XvgMM_Alloc((void *) ISTATE, NPARMS * sizeof(int));
  LIWORK = 3*NPARMS;
  IWORK = (int *) XvgMM_Alloc((void *) IWORK, LIWORK * sizeof(int));
  if ((BL == NULL) || (BU == NULL) || (X == NULL) || (Y == NULL)
      || (G == NULL) || (W == NULL) || (CLAMBDA == NULL) || (WORK == NULL)
      || (ISTATE == NULL) || (IWORK == NULL)) {
    Abort = B_TRUE;
    draw_mesg("Abort : can't allocate for NL buffers in NonLinearFieldFit()");
    return(B_FALSE);
  }
  ZERO        = 0;
  ONE         = 1;

#ifdef _E04UCF_OPTIONAL_PARAMETERS
  /* set e04ucf optional parameters if required */
  switch (OptParms)
    {
    case 2:
      e04uef_verify();
      e04uef_func_prec();
      break;
    case 1:
      e04uef_verify();
      break;
    case 0:
    default:
      e04uef_defaults();
      break;
    }
  e04uef_defaults();
  e04uef_max_iters_2000();
#endif

  /* make a local copy of the initial guess data */
  for (i = 0; i < NPARMS; i++) {
    X[i] = XSOLN[i];
    Y[i] = YSOLN[i];
  }
  
  /* set infinite upper/lower bounds */
  for (i = 0; i < NPARMS; i++) {
    BL[i] = -MAXDOUBLE;
    BU[i] = MAXDOUBLE;
  }
    
  /* solve the non-linear equation system for X */
  draw_mesg("NonLinearFieldFit() : Solving non-linear LS system for X");
  fflush(stdout);
  IUSER[0] = width*height;
  IUSER[1] = FieldModelType;
  IUSER[2] = (int) xweights;
  IFAIL = 1;
  e04ucf(&NPARMS, &ZERO, &ZERO, &ONE, &ONE, &NPARMS, NULL, BL, BU,
	 (void *) e04udm, (void *) OBJFUN, &ITER, ISTATE, C,
	 (double *) CJAC, CLAMBDA, &F, G, R, X,
	 IWORK, &LIWORK, WORK, &LWORK, IUSER, xfield, &IFAIL);
  fflush(stdout);
  
  /* compute the norm of the objective function gradient at the solution */
  for (i = 0, g = 0; i < NPARMS; i++)
    g += (G[i]*G[i]);
  g = sqrt(g);
  if (LogFileFp)
    fprintf(LogFileFp,
	    "\nNL X fit in %d iters (IFAIL:%d, OBJF:%.8e, NOBJF:%.3f, Norm(Grad):%.2e)\n",
	    ITER, IFAIL, F/ObjFunScale, F, g);
  if (VerboseFlag)
    printf("\nNL X fit in %d iters (IFAIL:%d, OBJF:%.8e, NOBJF:%.3f, Norm(Grad):%.2e)\n",
	   ITER, IFAIL, F/ObjFunScale, F, g);
  
  /* solve the non-linear equation system for Y */
  draw_mesg("NonLinearFieldFit() : Solving non-linear LS system for Y");
  fflush(stdout);
  IUSER[0] = width*height;
  IUSER[1] = FieldModelType;
  IUSER[2] = (int) yweights;
  IFAIL = 1;
  e04ucf(&NPARMS, &ZERO, &ZERO, &ONE, &ONE, &NPARMS, NULL, BL, BU,
	 (void *) e04udm, (void *) OBJFUN, &ITER, ISTATE, C,
	 (double *) CJAC, CLAMBDA, &F, G, R, Y,
	 IWORK, &LIWORK, WORK, &LWORK, IUSER, yfield, &IFAIL);
  fflush(stdout);
  
  /* compute the norm of the objective function gradient at the solution */
  for (i = 0, g = 0; i < NPARMS; i++)
    g += (G[i]*G[i]);
  g = sqrt(g);
  if (LogFileFp)
    fprintf(LogFileFp,
	    "NL Y fit in %d iters (IFAIL:%d, OBJF:%.8e, NOBJF:%.3f, Norm(Grad):%.2e)\n",
	    ITER, IFAIL, F/ObjFunScale, F, g);
  if (VerboseFlag)
    printf("NL X fit in %d iters (IFAIL:%d, OBJ:%.8e, NOBJF:%.3f, Norm(Grad):%.2e)\n",
	   ITER, IFAIL, F/ObjFunScale, F, g);
  
  /* compare initial guess and solution for X */
  for (i = 0, printf("\n"); i < NPARMS; i++) {
#ifdef _COMPARE_INITIAL_GUESS
    printf("GUESS_X[%2d]= %8.3f, SOLN_X[%2d]= %8.3f, %%Diff= %.2f\n",
	   i, XSOLN[i], i, X[i], 100.0*(XSOLN[i] - X[i])/XSOLN[i]);
    fflush(stdout);
#endif
    XSOLN[i] = X[i];
  }
    
  /* compare initial guess and solution for Y */
  for (i = 0, printf("\n"); i < NPARMS; i++) {
#ifdef _COMPARE_INITIAL_GUESS
    printf("GUESS_Y[%2d]= %8.3f, SOLN_Y[%2d]= %8.3f, %%Diff= %.2f\n",
	   i, YSOLN[i], i, Y[i], 100.0*(YSOLN[i] - Y[i])/YSOLN[i]);
    fflush(stdout);
#endif
    YSOLN[i] = Y[i];
  }
  
  /* free storage */
  XvgMM_Free(BL);
  XvgMM_Free(BU);
  XvgMM_Free(X);
  XvgMM_Free(Y);
  XvgMM_Free(G);
  XvgMM_Free(W);
  XvgMM_Free(CLAMBDA);
  XvgMM_Free(WORK);
  XvgMM_Free(ISTATE);
  XvgMM_Free(IWORK);

  /* return success */
  return(B_TRUE);
}

static void ComputeElementVAFs(int width, int height, FILE *LogFileFp,
			       double *xmodel, double *ymodel,
			       double *xfield, double *yfield,
			       double *xweights, double *yweights,
			       double *XSOLN, double *YSOLN,
			       int FieldModelType, boolean_t WrapFlag)
{
  double SumsXXp2[NELEMENTS_MAX], SumsX2[NELEMENTS_MAX], res_x, res_y, v, nx,
  ny, SumsYYp2[NELEMENTS_MAX], SumsY2[NELEMENTS_MAX], SSRX, SSRY, MinVAFX,
  MinVAFY, ERMSX, ERMSY, wx, wy;
  int i, k, ne, MaxVAFXe, MaxVAFYe, MinVAFXe, MinVAFYe;

  /* evaluate model at all non-zero points within the mesh to compute VAF */
  /* for each individual element (between wrapped data and wrapped model  */
  for (i = 0; i < nelements; i++) {
    SumsXXp2[i] = 0;
    SumsX2[i] = 0;
    SumsYYp2[i] = 0;
    SumsY2[i] = 0;
  }
  for (k=0, nx=0, ny=0; k < width*height; k++)
    if (FEMeshPixels[k] >= 0) {
      /* fetch weights */
      wx = (xweights ? (xweights[k]*xweights[k]) : 1.0);
      wy = (yweights ? (yweights[k]*yweights[k]) : 1.0);
      /* compute model values */
      xmodel[k] = ModelEval(FieldSamples[k], XSOLN, FieldModelType);
      ymodel[k] = ModelEval(FieldSamples[k], YSOLN, FieldModelType);
      /* compute wrapped model values */
      res_x = xmodel[k] - xfield[k];
      res_y = ymodel[k] - yfield[k];
      if (WrapFlag) {
	res_x = res_x - rint(res_x/TWOPI)*TWOPI;
	res_y = res_y - rint(res_y/TWOPI)*TWOPI;
      }
      /* add to running sums for VAF computation */
      ne = FieldSamples[k].element;
      SumsX2[ne] += (xfield[k] * xfield[k] * wx);
      SumsY2[ne] += (yfield[k] * yfield[k] * wy);
      SumsXXp2[ne] += (res_x * res_x * wx);
      SumsYYp2[ne] += (res_y * res_y * wy);
      /* increment prob. counters, if this sample is in a rectangle element */
      if (MeshElements[FieldSamples[k].element].type == R_TYPE) {
	nx += wx;
	ny += wy;
      }
    }
    else {
      xmodel[k] = 0;
      ymodel[k] = 0;
    }
  
  /* compute VAFs and global sum of squared residuals */
  if (LogFileFp)
    fprintf(LogFileFp, "\n%VAF for all elements:\n");
  if (VerboseFlag)
    printf("\n");
  for (i = 0, MinVAFX = MinVAFY = MAXDOUBLE, cbuf[0] = '\0', SSRX=0, SSRY=0;
       i < nelements; i++) {
    /* update sum of squared residuals for rectangle elements only*/
    if (MeshElements[i].type == R_TYPE) {
      SSRX += SumsXXp2[i];
      SSRY += SumsYYp2[i];
    }

    /* update element VAFS */
    v = 100.0*(1.0 - SumsXXp2[i]/SumsX2[i]);
    if (finite(v) && (v > 0))
      VAFsX[i] = v;
    else
      VAFsX[i] = 0;
    v = 100.0*(1.0 - SumsYYp2[i]/SumsY2[i]);
    if (finite(v) && (v > 0))
      VAFsY[i] = v;
    else
      VAFsY[i] = 0;

    /* printf out VAF results */
    if (LogFileFp)
      fprintf(LogFileFp, "  Element %02d: VAFX=%05.1f%%, VAFY=%05.1f%%\n",
	      i, VAFsX[i], VAFsY[i]);
    if (VerboseFlag) {
      printf("E%02d: VAFX=%.1f%%, VAFY=%.1f%% -- ", i, VAFsX[i], VAFsY[i]);
      fflush(stdout);
    }
    
    /* update VAF Mins and Maxs */
    if (VAFsX[i] < MinVAFX) {
      MinVAFX = VAFsX[i];
      MinVAFXe = i;
    }
    if (VAFsY[i] < MinVAFY) {
      MinVAFY = VAFsY[i];
      MinVAFYe = i;
    }
  }
  if (VerboseFlag)
    printf("\n");

  /* erms calcs */
  ERMSX = sqrt(SSRX/nx);
  ERMSY = sqrt(SSRY/ny);

  /* print out sum of squred residuals at the solution, if required */
  if (LogFileFp) {
    fprintf(LogFileFp,
	    "\nX SOLN : sum of squared residuals=%.8e, rms error=%.4f rads\n",
	    SSRX, ERMSX);
    fprintf(LogFileFp,
	    "Y SOLN : sum of squared residuals=%.8e, rms error=%.4f rads\n",
	    SSRY, ERMSY);
  }
  if (VerboseFlag) {
    if (WrapFlag) {
      printf("\nX SOLN : sum of squared residuals=%.8e, rms error=%.4f rads\n",
	     SSRX, ERMSX);
      printf("Y SOLN : sum of squared residuals=%.8e, rms error=%.4f rads\n",
	     SSRY, ERMSY);
    }
    else {
      printf("\nX SOLN : sum of squared residuals=%.8e, rms error=%.4f\n",
	     SSRX, ERMSX);
      printf("Y SOLN : sum of squared residuals=%.8e, rms error=%.4f\n",
	     SSRY, ERMSY);
    }
  }
}

static boolean_t LinearFieldFit(int NX, int NY, int NPARMS,
			      double *XSOLN, double *YSOLN,
			      int N_X_FRINGES_UC, int N_Y_FRINGES_UC,
			      int N_X_FRINGES, int N_Y_FRINGES,
			      int height, int width,
			      double *xmask, double *ymask,
			      double *xfield, double *yfield,
			      double *xweights, double *yweights,
			      boolean_t *ConstrainedFringesX,
			      boolean_t *ConstrainedFringesY,
			      double *ConstraintValuesX,
			      double *ConstraintValuesY,
			      int *UnConstrainedFringeIndxX,
			      int *UnConstrainedFringeIndxY,
			      int FieldModelType, FILE *LogFileFp,
			      boolean_t ComputeVAFs, boolean_t WrapFlag)
{
  static double *AX=NULL, *BX=NULL, *AY=NULL, *BY=NULL, *QR=NULL, *ALPHA=NULL,
  *E=NULL, *Y=NULL, *Z=NULL, *R=NULL;
  static int *IPIV=NULL;
  FESample s;
  double EPS, XCoverage[NELEMENTS_MAX], YCoverage[NELEMENTS_MAX],
  SSRX, SSRY, ERMSX, ERMSY, w;
  int i, k, fo, ONE, IFAIL, NMAX;

  /* allocate storage for linear fitting */
  NMAX = (NX > NY ? NX : NY);
  AX    = (double *) XvgMM_Alloc((void *) AX,    NX * NX * sizeof(double));
  BX    = (double *) XvgMM_Alloc((void *) BX,    NX * sizeof(double));
  AY    = (double *) XvgMM_Alloc((void *) AY,    NY * NY * sizeof(double));
  BY    = (double *) XvgMM_Alloc((void *) BY,    NY * sizeof(double));
  QR    = (double *) XvgMM_Alloc((void *) QR,    NMAX * NMAX * sizeof(double));
  ALPHA = (double *) XvgMM_Alloc((void *) ALPHA, NMAX * sizeof(double));
  IPIV  = (int *)    XvgMM_Alloc((void *) IPIV,  NMAX * sizeof(int));
  E     = (double *) XvgMM_Alloc((void *) E,     NMAX * sizeof(double));
  Y     = (double *) XvgMM_Alloc((void *) Y,     NMAX * sizeof(double));
  Z     = (double *) XvgMM_Alloc((void *) Z,     NMAX * sizeof(double));
  R     = (double *) XvgMM_Alloc((void *) R,     NMAX * sizeof(double));
  if ((AX == NULL) || (BX == NULL) || (XSOLN == NULL)
      || (AY == NULL) || (BY == NULL) || (YSOLN == NULL)
      || (QR == NULL) || (IPIV == NULL)|| (E == NULL) || (Y == NULL)
      || (Z == NULL)|| (R == NULL) || (ALPHA == NULL)) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort : can't allocate for buffers in LinearFieldFit()");
    draw_mesg(cbuf);
    return(B_FALSE);
  }

  /* init constants */
  EPS = MINDOUBLE;
  ONE = 1;

  /* initiliaze the stiffness matrix and forcing vector */
  for (i = 0; i < NX*NX; i++)
    AX[i] = 0;
  for (i = 0; i < NY*NY; i++)
    AY[i] = 0;
  for (i = 0; i < NX; i++)
    BX[i] = 0;
  for (i = 0; i < NY; i++)
    BY[i] = 0;
      
  /* build the linear solution matrix for X and Y, enforcing offsets */
  draw_mesg("LinearFieldFit(): Computing normal equations matrix systems...");
  if (VerboseFlag) {
    printf("LinearFieldFit(): Computing normal equations matrix systems...");
    fflush(stdout);
  }
  for (k = 0; (k < height*width) && (Abort == B_FALSE); k++) {
    /* look at pixels inside the FE mesh only */
    if (FEMeshPixels[k] >= 0) {
      /* load the field sample at "k" */
      s = FieldSamples[k];
	  
      /* if x data point inside a fringe, include it in the fit */
      if (xmask[k] >= 0.0) {
	/* fetch weight if any */
	w = (xweights ? (xweights[k]*xweights[k]) : 1.0);
	if (ConstrainedFringesX) {
	  fo = xmask[k];
	  if (ConstrainedFringesX[fo]) {
	    ComputeSamplePartialSums(s, xfield[k], w, AX, BX, NPARMS,
				     N_X_FRINGES_UC, -1,
				     ConstraintValuesX[fo] * TWOPI,
				     FieldModelType);
	  }
	  else {
	    ComputeSamplePartialSums(s, xfield[k], w, AX, BX, NPARMS,
				     N_X_FRINGES_UC,
				     UnConstrainedFringeIndxX[fo],
				     0,
				     FieldModelType);
	  }
	}
	else {
	  ComputeSamplePartialSums(s, xfield[k], w, AX, BX, NPARMS,
				   0, -1, 0, FieldModelType);
	}
	MeshElements[s.element].nsamples_x++;
      }
      /* if y data point inside a fringe, include it in the fit */
      if (ymask[k] >= 0.0) {
	/* fetch weight if any */
	w = (yweights ? (yweights[k]*yweights[k]) : 1.0);
	if (ConstrainedFringesY) {
	  fo = ymask[k];
	  if (ConstrainedFringesY[fo]) {
	    ComputeSamplePartialSums(s, yfield[k], w, AY, BY, NPARMS,
				     N_Y_FRINGES_UC, -1,
				     ConstraintValuesY[fo] * TWOPI,
				     FieldModelType);
	  }
	  else {
	    ComputeSamplePartialSums(s, yfield[k], w, AY, BY, NPARMS,
				     N_Y_FRINGES_UC,
				     UnConstrainedFringeIndxY[fo],
				     0,
				     FieldModelType);
	  }
	}
	else {
	  ComputeSamplePartialSums(s, yfield[k], w, AY, BY, NPARMS,
				   0, -1, 0, FieldModelType);
	}
	MeshElements[s.element].nsamples_y++;
      }
    }
  }
  if (VerboseFlag)
    printf("Done.\n");
  if (Abort == B_TRUE) {
    draw_mesg("Abort:while calculating normal equations in LinearFieldFit(x)");
    return(B_FALSE);
  }
  
  /* re-solve the matrix systems to compute the displaced X nodal coords */
  draw_mesg("LinearFieldFit() : Solving linear LS matrix equations...");
  if (VerboseFlag) {
    printf("LinearFieldFit(): Solving normal equations matrix systems...");
    fflush(stdout);
  }
  IFAIL = 1;
  f04amf(AX, &NX, XSOLN, &NX, BX, &NX, &NX, &NX, &ONE, &EPS,
	 QR, &NX, ALPHA, E, Y, Z, R, IPIV, &IFAIL);
  if (IFAIL != 0) {
    Abort = B_TRUE;
    sprintf(cbuf,"Abort:IFAIL=%d returning from f04amf() in LinearFieldFit(x)",
	    IFAIL);
    draw_mesg(cbuf);
    if (VerboseFlag)
      printf("\n%s\n", cbuf);
    return(B_FALSE);
  }
  
  /* re-solve the matrix systems to compute the displaced Y nodal coords */
  IFAIL = 1;
  f04amf(AY, &NY, YSOLN, &NY, BY, &NY, &NY, &NY, &ONE, &EPS,
	 QR, &NY, ALPHA, E, Y, Z, R, IPIV, &IFAIL);
  if (IFAIL != 0) {
    Abort = B_TRUE;
    sprintf(cbuf,"Abort:IFAIL=%d returning from f04amf() in LinearFieldFit(y)",
	    IFAIL);
    draw_mesg(cbuf);
    if (VerboseFlag)
      printf("\n%s\n", cbuf);
    return(B_FALSE);
  }
  if (VerboseFlag)
    printf("Done.\n");
  
  /* show constrained calculated fringe offsets */
  if (N_X_FRINGES) {
    if (LogFileFp)
      fprintf(LogFileFp, "\nFitted X fringe orders:\n");
    if (VerboseFlag)
      printf("\nFitted X fringe orders:\n");
    for (i = 0, k = 0; i < N_X_FRINGES; i++)
      if (ConstrainedFringesX[i]) {
	if (LogFileFp)
	  fprintf(LogFileFp, "  %5.2f (Constrained)\n", ConstraintValuesX[i]);
	if (VerboseFlag)
	  printf("  %5.2f (Constrained)\n", ConstraintValuesX[i]);
      }
      else {
	if (LogFileFp)
	  fprintf(LogFileFp, "  %5.2f (f%d)\n",
		  XSOLN[NPARMS + UnConstrainedFringeIndxX[i]],
		  UnConstrainedFringeIndxX[i]);
	if (VerboseFlag)
	  printf("  %5.2f (f%d)\n",
		 XSOLN[NPARMS + UnConstrainedFringeIndxX[i]],
		 UnConstrainedFringeIndxX[i]);
      }
  }
  if (N_Y_FRINGES) {
    if (LogFileFp)
      fprintf(LogFileFp, "\nFitted Y fringe orders:\n");
    if (VerboseFlag)
      printf("\nFitted Y fringe orders:\n");
    for (i = 0; i < N_Y_FRINGES; i++)
      if (ConstrainedFringesY[i]) {
	if (LogFileFp)
	  fprintf(LogFileFp, "  %5.2f (Constrained)\n",ConstraintValuesY[i]);
	if (VerboseFlag)
	  printf("  %5.2f (Constrained)\n", ConstraintValuesY[i]);
      }
      else {
	if (LogFileFp)
	  fprintf(LogFileFp, "  %5.2f (f%d)\n",
		  YSOLN[NPARMS + UnConstrainedFringeIndxY[i]],
		  UnConstrainedFringeIndxY[i]);
	if (VerboseFlag)
	  printf("  %5.2f (f%d)\n",
		 YSOLN[NPARMS + UnConstrainedFringeIndxY[i]],
		 UnConstrainedFringeIndxY[i]);
      }
  }

  /* compute sum of squared residuals, if required */
  if (ComputeVAFs) {
    draw_mesg("\nLinearFieldFit() : Computing sum of squared residuals...");
    if (VerboseFlag) {
      printf("LinearFieldFit(): Computing sum of squared residuals...");
      fflush(stdout);
    }
    ComputeSumOfSquaredResiduals(xfield, xweights, width, height,
				 XSOLN, FieldModelType, &SSRX, &ERMSX,
				 WrapFlag);
    ComputeSumOfSquaredResiduals(yfield, yweights, width, height,
				 YSOLN, FieldModelType, &SSRY, &ERMSY,
				 WrapFlag);
    if (LogFileFp){
      fprintf(LogFileFp,
	      "\nX SOLN: sum of squared residuals=%.8e, rms error=%.4f rads\n",
	      SSRX, ERMSX);
      fprintf(LogFileFp,
	      "Y SOLN : sum of squared residuals=%.8e, rms error=%.4f rads\n",
	      SSRY, ERMSY);
    }
    if (VerboseFlag) {
      if (WrapFlag) {
	printf("Done.\n");
	printf("\nXSOLN: sum of squared residuals=%.4e, rms error=%.4f rads\n",
	       SSRX, ERMSX);
	printf("YSOLN: sum of squared residuals=%.8e, rms error=%.4f rads\n",
	       SSRY, ERMSY);
      }
      else {
	printf("Done.\n");
	printf("\nXSOLN: sum of squared residuals=%.4e, rms error=%.4f\n",
	       SSRX, ERMSX);
	printf("YSOLN: sum of squared residuals=%.8e, rms error=%.4f\n",
	       SSRY, ERMSY);
      }
    }
  }
  
  /* return success */
  return(B_TRUE);
}

static void CountFringes(double *xmask, double *ymask, int width, int height,
			 int *N_X_FRINGES, int *N_Y_FRINGES,
			 int *LARGEST_X_FRINGE, int *LARGEST_Y_FRINGE,
			 FILE *LogFileFp)
{
  int i, k, FringeTableX[NCFMAX], FringeTableY[NCFMAX], indx,
    FringePixCntX[NCFMAX], FringePixCntY[NCFMAX], xmax, ymax;

  /* if required, correct the mask images so that non-mesh pixels are */
  /* equal to "-1" and the fringes are numbered sequentially from "0" */
  for (i = 0; i < NCFMAX; i++) {
    FringeTableX[i] = -1;
    FringePixCntX[i] = 0;
    FringeTableY[i] = -1;
    FringePixCntY[i] = 0;
  }
  for (k = 0, *N_X_FRINGES = 0, *N_Y_FRINGES = 0;
       (k < width*height) && (Abort == B_FALSE); k++) {
    /* set non-mesh pixels to "-1" */
    if (FEMeshPixels[k] == -1) {
      xmask[k] = -1;
      ymask[k] = -1;
    }
    else {
      /* check  xmask values */
      indx = xmask[k];
      if (indx != -1) {
	if ((indx < 0) || (indx >= NCFMAX)) {
	  Abort = B_TRUE;
	}
	else {
	  if (FringeTableX[indx] == -1)
	    FringeTableX[indx] = (*N_X_FRINGES)++;
	  FringePixCntX[FringeTableX[indx]]++;
	}
      }
      /* check ymask values */
      indx = ymask[k];
      if (indx != -1) {
	if ((indx < 0) || (indx >= NCFMAX)) {
	  Abort = B_TRUE;
	}
	else {
	  if (FringeTableY[indx] == -1)
	    FringeTableY[indx] = (*N_Y_FRINGES)++;
	  FringePixCntY[FringeTableY[indx]]++;
	}
      }
    }
  }

  /* convert mask values to sequential numbers from 0 */
  for (k = 0; (k < height*width) && (Abort == B_FALSE); k++) {
    if (xmask[k] >= 0)
      xmask[k] = FringeTableX[(int) xmask[k]];
    if (ymask[k] >= 0)
      ymask[k] = FringeTableY[(int) ymask[k]];
  }

  /* find the largest fringe regions */
  for (i = 0, xmax = 0, *LARGEST_X_FRINGE = 0; i < *N_X_FRINGES; i++)
    if (FringePixCntX[i] > xmax) {
      xmax = FringePixCntX[i];
      *LARGEST_X_FRINGE = i;
    }
  for (i = 0, ymax = 0, *LARGEST_Y_FRINGE = 0; i < *N_Y_FRINGES; i++)
    if (FringePixCntY[i] > ymax) {
      ymax = FringePixCntY[i];
      *LARGEST_Y_FRINGE = i;
    }
    
  /* show number of fringes found */
  if (LogFileFp) {
    fprintf(LogFileFp, "\n%d \"X\" fringes (%d is largest with %d samples)\n",
	    *N_X_FRINGES, *LARGEST_X_FRINGE, xmax);
    fprintf(LogFileFp, "%d \"Y\" fringes (%d is largest with %d pixels)\n",
	    *N_Y_FRINGES, *LARGEST_Y_FRINGE, ymax);
  }
  if (VerboseFlag) {
    printf("CountFringes():%d \"X\" fringes (%d is largest with %d samples)\n",
	   *N_X_FRINGES, *LARGEST_X_FRINGE, xmax);
    printf("CountFringes():%d \"Y\" fringes (%d is largest with %d pixels)\n",
	   *N_Y_FRINGES, *LARGEST_Y_FRINGE, ymax);
  }
}

static void ScanFittedFringes(double *SOLN, int NPARMS, int N_FRINGES,
			      int *N_FRINGES_UC, int *N_FRINGES_C,
			      boolean_t *ConstrainedFringes,
			      int *UnConstrainedFringeIndx,
			      double *ConstraintValues,
			      double FOThreshold)
{
  int i;

  /* scan the fringes */
  for (i=0, *N_FRINGES_UC=0; i < N_FRINGES; i++) {
    /* if this fringe was a free one in the last fit, check to see if */
    /* it's fitted offset is now within the threshold. */
    if (ConstrainedFringes[i] == B_FALSE) {
      /* if the new fitted offset is within the threshold, constrain it */
      if (fabs(rint(SOLN[NPARMS+UnConstrainedFringeIndx[i]])
	       - SOLN[NPARMS+UnConstrainedFringeIndx[i]]) < FOThreshold) {
	ConstrainedFringes[i] = B_TRUE;
	ConstraintValues[i] = rint(SOLN[NPARMS+UnConstrainedFringeIndx[i]]);
	(*N_FRINGES_C)++;
      }
      /* else leave this fringe offset free for the next fit */
      else {
	UnConstrainedFringeIndx[i] = (*N_FRINGES_UC)++;
      }
    }
  }
}

void FECompute(double *inpixels, double *outpixels,
	       int height, int width, void *ParmPtrs[])
/***********************************************************************

FILE: feIPOPs.c

COMMAND: FECompute()

DESCRIPTION:
  This function takes a series of full-field deformation images and computes
  the corresponding fitted finite element mesh node displacements.

  The function uses the following paramaters:
    - inpixels  : pointer to the input image buffer
    - outpixels : pointer to the output image buffer
    - height    : image height
    - width     : image width
    - ParmPtrs  : array of function parameters, where
       ParmPtrs[0] : FitType
                       0, uniform  LS linear fitting only, no refinement.
                      -1, fitting is refined by non-linear uniform LS fitting.
                      -2, fitting is refined by non-linear weighted LS fitting.
                      99, uniform  LS linear fitting only, no refinement
                                          used by FEComputeDisplacedNodes().
       ParmPtrs[1] : FOThreshold, Max deviation from integer value for a fringe
                     order [0..1]. Used in fitting types 0, -1, -2.
       ParmPtrs[2] : Outfiles prefix.
       ParmPtrs[3] : FieldModelType, filename of initial guess for non-linear
                     fitting (fitting types -1, -2).

  Stack/buffer images:

  - FitType == 99
    outpixels                           : unwrapped model X data    (output)
    ImageStack[ImageStackTop -1].pixels : unwrapped model Y data    (output)
    ImageStack[ImageStackTop -2].pixels : dummy x&y mask data        (input)
    ImageStack[ImageStackTop -3].pixels : filtered unwrapped X data  (input)
    ImageStack[ImageStackTop -4].pixels : filtered unwrapped Y data  (input)

  - Others
    outpixels                           : unwrapped model X data    (output)
    ImageStack[ImageStackTop -1].pixels : unwrapped model Y data    (output)
    ImageStack[ImageStackTop -2].pixels : X mask data                (input)
                                generated by RegionFillSegmentation().
    ImageStack[ImageStackTop -3].pixels : Y mask data                (input)
                                generated by RegionFillSegmentation().
    ImageStack[ImageStackTop -4].pixels : filtered wrapped X data    (input)
    ImageStack[ImageStackTop -5].pixels : filtered wrapped Y data    (input)
    ImageStack[ImageStackTop -6].pixels : raw wrapped X data         (input)
    ImageStack[ImageStackTop -7].pixels : raw wrapped Y data         (input)
    ImageStack[ImageStackTop -8].pixels : X weight data [0..1]       (input)
    ImageStack[ImageStackTop -9].pixels : Y weight data [0..1]       (input)

RETURN: Void.

MODIFIED:
  - 5/March 1997, Paul Charette.

======================================================================*/
{
  double *xfield, *yfield, *xmodel, *ymodel, min, max, dx, dy, *xmask, *ymask,
    ConstraintValuesX[NCFMAX], ConstraintValuesY[NCFMAX], FOThreshold,
    *xfieldr, *yfieldr, *xweights, *yweights, *XSOLN, *YSOLN,
    *XSOLNB, *YSOLNB;
  int i, j, k, FitType, N_X_FRINGES, NX, NY, FieldModelType, N_Y_FRINGES, 
    NPARMS, N_X_FRINGES_C, N_Y_FRINGES_C, N_X_FRINGES_UC, N_Y_FRINGES_UC,
    fo, UnConstrainedFringeIndxX[NCFMAX], UnConstrainedFringeIndxY[NCFMAX],
    LARGEST_X_FRINGE, LARGEST_Y_FRINGE;
  char fname[256];
  boolean_t ConstrainedFringesX[NCFMAX], ConstrainedFringesY[NCFMAX];
  FILE *fp, *LogFileFp;

  /* load fitting type */
  FitType = *((int *) ParmPtrs[0]);

  /* Max deviation from integer value for a fringe order [0..1] */
  FOThreshold = *((double *) ParmPtrs[1]);

  /* load filename containing initial guess for the non-linear solution */
  FieldModelType = *((int *) ParmPtrs[3]);

  /* show input parameters */
  if (VerboseFlag)
    printf("FECompute(FitType:%d, FOThreshold:%f, FieldModelType:%d\n",
	   FitType, FOThreshold, FieldModelType);
  
  /* check that stack contains the correct number of images */
  /* for FEComputeDisplacedNodesCase */
  if (FitType == 99) {
    /* outpixels                           : unwrapped model X data (out) */
    /* ImageStack[ImageStackTop -1].pixels : unwrapped model Y data (out) */
    /* ImageStack[ImageStackTop -2].pixels : dummy x&y mask data          */
    /* ImageStack[ImageStackTop -3].pixels : filtered unwrapped X data    */
    /* ImageStack[ImageStackTop -4].pixels : filtered unwrapped Y data    */
    if (ImageStackTop < 4) {
      Abort = B_TRUE;
      draw_mesg("Abort : 4 images required on the stack in FECompute()");
      return;
    }
    else {
      /* initialize the data pointers */
      xmodel  = outpixels;
      ymodel  = ImageStack[ImageStackTop -1].pixels;
      xmask   = ImageStack[ImageStackTop -2].pixels;
      ymask   = ImageStack[ImageStackTop -2].pixels;
      xfield  = ImageStack[ImageStackTop -3].pixels;
      yfield  = ImageStack[ImageStackTop -4].pixels;
      xfieldr = NULL;
      yfieldr = NULL;
      xweights = NULL;
      yweights = NULL;
    }
  }
  /* else normal case */
  else {
    /* outpixels                           : unwrapped model X data (out) */
    /* ImageStack[ImageStackTop -1].pixels : unwrapped model Y data (out) */
    /* ImageStack[ImageStackTop -2].pixels : X mask data     (in)         */
    /*                             generated by RegionFillSegmentation(). */
    /* ImageStack[ImageStackTop -3].pixels : Y mask data     (in)         */
    /*                             generated by RegionFillSegmentation(). */
    /* ImageStack[ImageStackTop -4].pixels : filtered wrapped X data  (in)*/
    /* ImageStack[ImageStackTop -5].pixels : filtered wrapped Y data  (in)*/
    /* ImageStack[ImageStackTop -6].pixels : raw wrapped X data  (in)     */
    /* ImageStack[ImageStackTop -7].pixels : raw wrapped Y data  (in)     */
    /* ImageStack[ImageStackTop -8].pixels : X weight data [0..1] (in)    */
    /* ImageStack[ImageStackTop -9].pixels : Y weight data [0..1] (in)    */
    if (ImageStackTop < 9) {
      Abort = B_TRUE;
      draw_mesg("Abort : 9 images required on the stack in FECompute()");
      return;
    }
    else {
      /* initialize the data pointers */
      xmodel   = outpixels;
      ymodel   = ImageStack[ImageStackTop -1].pixels;
      xmask    = ImageStack[ImageStackTop -2].pixels;
      ymask    = ImageStack[ImageStackTop -3].pixels;
      xfield   = ImageStack[ImageStackTop -4].pixels;
      yfield   = ImageStack[ImageStackTop -5].pixels;
      xfieldr  = ImageStack[ImageStackTop -6].pixels;
      yfieldr  = ImageStack[ImageStackTop -7].pixels;
      xweights = ImageStack[ImageStackTop -8].pixels;
      yweights = ImageStack[ImageStackTop -9].pixels;
    }
  }    

  /* check that a mesh is loaded and defined */
  if (FEMeshDefined == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort : undefined mesh in FECompute()");
    return;
  }

  /* open log file and print out call parameters, if required */
  if (ParmPtrs[2]) {
    sprintf(fname, "%s.log", ParmPtrs[2]);
    if ((LogFileFp = fopen(fname, "w")) == NULL) {
      Abort = B_TRUE;
      sprintf(cbuf, "Could not open the output file \"%s\"", fname);
      draw_mesg(cbuf);
      return;
    }
    fprintf(LogFileFp, "FECompute() log file\n");
    fprintf(LogFileFp, "  FitType        = %d\n", FitType);
    fprintf(LogFileFp, "  FOThreshold    = %.2f\n", FOThreshold);
    fprintf(LogFileFp, "  FieldModelType = %d\n", FieldModelType);
  }
  else
    LogFileFp = NULL;

  /* check that weights are normalized, if required */
  if (FitType == -2) {
    DynamicRange(xweights, height*width, &max, &min);
    if ((max > 1.0) || (min < 0.0)) {
      Abort = B_TRUE;
      draw_mesg("Abort : non-normalized weights for X in FECompute()");
      if (LogFileFp)
	fclose(LogFileFp);
      return;
    }
    DynamicRange(yweights, height*width, &max, &min);
    if ((max > 1.0) || (min < 0.0)) {
      Abort = B_TRUE;
      draw_mesg("Abort : non-normalized weights for Y in FECompute()");
      if (LogFileFp)
	fclose(LogFileFp);
      return;
    }
  }

  /* allocate storage for the field samples if required */
  if ((FieldSamples = (FESample *) XvgMM_Alloc((void *) FieldSamples,
					       height*width
					       *sizeof(FESample))) == NULL) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort : can't allocate for field samples in FECompute()");
    draw_mesg(cbuf);
    if (LogFileFp)
      fclose(LogFileFp);
    return;
  }

  /* allocate storage for the mesh pixels if required and calc elm coords */
  draw_mesg("FECompute() : Computing element coordinates...");
  if ((FEMeshPixels == NULL)
      || (XvgMM_GetBlockSize(FEMeshPixels) != height*width*sizeof(short))) {
    if ((FEMeshPixels = (short *) XvgMM_Alloc((void *) FEMeshPixels,
					      height*width*sizeof(short)))
	== NULL) {
      Abort = B_TRUE;
      sprintf(cbuf, "Abort : can't allocate for mesh pixels in FECompute()");
      draw_mesg(cbuf);
      if (LogFileFp)
	fclose(LogFileFp);
      return;
    }
    ComputeAllPixelsXiCoords(B_TRUE, B_TRUE, FieldModelType);
  }
  /* else just calculate the element coordinates */
  else {
    ComputeAllPixelsXiCoords(B_TRUE, B_FALSE, FieldModelType);
  }
  if (Abort) {
    if (LogFileFp)
      fclose(LogFileFp);
    return;
  }
  
  /* determine the number of fringe areas in the mask images */
  CountFringes(xmask, ymask, width, height, &N_X_FRINGES, &N_Y_FRINGES,
	       &LARGEST_X_FRINGE, &LARGEST_Y_FRINGE, LogFileFp);
  if (Abort) {
    draw_mesg("Abort : Invalid mask data in FECompute()");
    if (LogFileFp)
      fclose(LogFileFp);
    return;
  }
  
  /* init local vals */
  if (FieldModelType == BICUBIC_FIELD_MODEL)
    NPARMS = 4*nnodes;
  else
    NPARMS = nnodes;
  NX = NPARMS + N_X_FRINGES - 1;
  NY = NPARMS + N_Y_FRINGES - 1;
  if (VerboseFlag)
    printf("FECompute() => %d model parameters, %d X fringes, %d Y fringes\n",
	   NPARMS, N_X_FRINGES, N_Y_FRINGES);

  /* allocate storage for the solution */
  XSOLN  = (double *) XvgMM_Alloc(NULL, NX * sizeof(double));
  YSOLN  = (double *) XvgMM_Alloc(NULL, NY * sizeof(double));
  XSOLNB = (double *) XvgMM_Alloc(NULL, NPARMS * 4 * sizeof(double));
  YSOLNB = (double *) XvgMM_Alloc(NULL, NPARMS * 4 * sizeof(double));
  if ((XSOLN == NULL)||(YSOLN == NULL)||(XSOLNB == NULL)||(YSOLNB == NULL)) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort : can't allocate for buffers in FECompute()");
    draw_mesg(cbuf);
    if (LogFileFp)
      fclose(LogFileFp);
    return;
  }

  /* initialize the fringe order constraint arrays by constraining the */
  /* largest fringe areas in both X and Y images to "0" offset         */
  /* and leaving the other fringes unconstrained */
  for (i=0, N_X_FRINGES_UC=0; i < N_X_FRINGES; i++)
    if (i != LARGEST_X_FRINGE) {
      ConstrainedFringesX[i] = B_FALSE;
      UnConstrainedFringeIndxX[i] = N_X_FRINGES_UC++;
    }
  ConstrainedFringesX[LARGEST_X_FRINGE] = B_TRUE;
  ConstraintValuesX[LARGEST_X_FRINGE] = 0;
  N_X_FRINGES_C = 1;
  for (i=0, N_Y_FRINGES_UC=0; i < N_Y_FRINGES; i++)
    if (i != LARGEST_Y_FRINGE) {
      ConstrainedFringesY[i] = B_FALSE;
      UnConstrainedFringeIndxY[i] = N_Y_FRINGES_UC++;
    }
  ConstrainedFringesY[LARGEST_Y_FRINGE] = B_TRUE;
  ConstraintValuesY[LARGEST_Y_FRINGE] = 0;
  N_Y_FRINGES_C = 1;
  
  /* first pass fitting to filtered wrapped data, with 1 fringe constrained */
  /* Because the linear fit does not use the B_TRUE wrapped error measure, the filtered */
  /* field gives a better fit and is not thrown off by wrapped error points */
  if ((LinearFieldFit(NX, NY, NPARMS, XSOLN, YSOLN,
		      N_X_FRINGES_UC, N_Y_FRINGES_UC,
		      N_X_FRINGES, N_Y_FRINGES, height, width,
		      xmask, ymask, xfield, yfield,  NULL, NULL,
		      ConstrainedFringesX, ConstrainedFringesY,
		      ConstraintValuesX, ConstraintValuesY,
		      UnConstrainedFringeIndxX, UnConstrainedFringeIndxY,
		      FieldModelType, LogFileFp, B_TRUE, B_TRUE) == B_FALSE)
      || (Abort)) {
    Abort = B_TRUE;
    draw_mesg("Abort : On return from LinearFieldFit() in FECompute()");
    if (LogFileFp)
      fclose(LogFileFp);
    return;
  }

  /* check for specical case of call with unwrapped data */
  if (FitType != 99) {
    /* decide which fringe orders are to be constrained for the next fit*/
    ScanFittedFringes(XSOLN, NPARMS, N_X_FRINGES, &N_X_FRINGES_UC,
		      &N_X_FRINGES_C, ConstrainedFringesX,
		      UnConstrainedFringeIndxX, ConstraintValuesX,
		      FOThreshold);
    ScanFittedFringes(YSOLN, NPARMS, N_Y_FRINGES, &N_Y_FRINGES_UC,
		      &N_Y_FRINGES_C, ConstrainedFringesY,
		      UnConstrainedFringeIndxY, ConstraintValuesY,
		      FOThreshold);
    
    /* check that all fringes are properly constrained, if not exit */
    if ((N_X_FRINGES_C != N_X_FRINGES) || (N_Y_FRINGES_C  != N_Y_FRINGES)) {
      Abort = B_TRUE;
      draw_mesg("Abort: Fringe constraining was unsuccessfull in FECompute()");
      if (LogFileFp)
	fclose(LogFileFp);
      return;
    }
    
    /* second pass linear fit to raw wrapped data and constrained fringes */
    /* Because the linear fit does not use the B_TRUE wrapped error measure, */
    /* the filtered */
    /* field gives a better fit and is not thrown off by wrapped error points*/
    if ((LinearFieldFit(NPARMS, NPARMS, NPARMS, XSOLN, YSOLN,
			N_X_FRINGES_UC, N_Y_FRINGES_UC,
			N_X_FRINGES, N_Y_FRINGES, height, width,
			 xmask, ymask, xfield, yfield, NULL, NULL,
			ConstrainedFringesX, ConstrainedFringesY,
			ConstraintValuesX, ConstraintValuesY,
			UnConstrainedFringeIndxX, UnConstrainedFringeIndxY,
			FieldModelType, LogFileFp,
			B_TRUE, B_TRUE) == B_FALSE)
	|| (Abort)) {
      Abort = B_TRUE;
      draw_mesg("Abort : On return from LinearFieldFit() in FECompute()");
      if (LogFileFp)
	fclose(LogFileFp);
      return;
    }
    
    /* write the linear fit solution in X out to a disk file */
    sprintf(fname, "%s_solnx_LN.doc", ParmPtrs[2]);
    if ((fp = fopen(fname, "w")) == NULL) {
      Abort = B_TRUE;
      sprintf(cbuf, "Could not open the output file \"%s\"", fname);
      draw_mesg(cbuf);
      if (LogFileFp)
	fclose(LogFileFp);
      return;
    }
    if (FieldModelType == BICUBIC_FIELD_MODEL) {
      for (i = 0; i < NPARMS; i++)
	
	fprintf(fp, "%f\n", XSOLN[i]);
    }
    else {
      ComputeBicubicDerivatives(XSOLN, XSOLNB);
      for (i = 0; i < 4*NPARMS; i++)
	fprintf(fp, "%f\n", XSOLNB[i]);
    }
    fclose(fp);
    
    
    /* write the linear fit solution in Y out to a disk file */
    sprintf(fname, "%s_solny_LN.doc", ParmPtrs[2]);
    if ((fp = fopen(fname, "w")) == NULL) {
      Abort = B_TRUE;
      sprintf(cbuf, "Could not open the output file \"%s\"", fname);
      draw_mesg(cbuf);
      if (LogFileFp)
	fclose(LogFileFp);
      return;
    }
    if (FieldModelType == BICUBIC_FIELD_MODEL) {
      for (i = 0; i < NPARMS; i++)
	fprintf(fp, "%f\n", YSOLN[i]);
    }
    else {
      ComputeBicubicDerivatives(YSOLN, YSOLNB);
      for (i = 0; i < 4*NPARMS; i++)
	fprintf(fp, "%f\n", YSOLNB[i]);
    }
    fclose(fp);
    
    /* NON-LINEAR fit, if required */
    if (FitType < 0) {
      if (NonLinearFieldFit(width, height, NPARMS, xfieldr, yfieldr,
			    (FitType == -1 ? NULL : xweights),
			    (FitType == -1 ? NULL : yweights),
			    LogFileFp,
			    FieldModelType, XSOLN, YSOLN)) {
	/* write the X solution */
	sprintf(fname, "%s_solnx_NL.doc", ParmPtrs[2]);
	if ((fp = fopen(fname, "w")) == NULL) {
	  Abort = B_TRUE;
	  sprintf(cbuf, "Could not open the output file \"%s\" in FECompute()",
		  fname);
	  draw_mesg(cbuf);
	  if (LogFileFp)
	    fclose(LogFileFp);
	  return;
	}
	for (i = 0; i < NPARMS; i++)
	  fprintf(fp, "%f\n", XSOLN[i]);
	fclose(fp);
	
	/* write the Y solution */
	sprintf(fname, "%s_solny_NL.doc", ParmPtrs[2]);
	if ((fp = fopen(fname, "w")) == NULL) {
	  Abort = B_TRUE;
	  sprintf(cbuf, "Could not open the output file \"%s\" in FECompute()",
		  fname);
	  draw_mesg(cbuf);
	  if (LogFileFp)
	    fclose(LogFileFp);
	  return;
	}
	for (i = 0; i < NPARMS; i++)
	  fprintf(fp, "%f\n", YSOLN[i]);
	fclose(fp);
      }
      else {
	Abort = B_TRUE;
	draw_mesg("Abort:Failure on return from NonLinearFit() in FECompte()");
	if (LogFileFp)
	  fclose(LogFileFp);
	return;
      }
    }
  }

  /* compute the %VAF by the model in each element w/r to the filtered data */
  if (Abort == B_FALSE) {
    draw_mesg("FECompute() : Computing element VAFs...");
    if (VerboseFlag) {
      printf("\nFECompute() : Computing element VAFs...");
      fflush(stdout);
    }
    ComputeElementVAFs(width, height, LogFileFp, xmodel, ymodel,
		       xfield, yfield, NULL, NULL,
		       XSOLN, YSOLN, FieldModelType, B_TRUE);
    if (VerboseFlag)
      printf("Done.\n");
  }

  /* update the nodal coordinates, subtract model values for the fixed node */
  if (FitType == 99) {
    draw_mesg("FECompute() : Updating field values at the nodes...");
    for (i = 0; (i < nnodes) && (Abort == B_FALSE); i++) {
      if (i != 1) {
	if (FieldModelType == BICUBIC_FIELD_MODEL) {
	  dx = (XSOLN[4*i] - XSOLN[4*1]) * METERS_PER_RADIAN;
	  dy = -(YSOLN[4*i] - YSOLN[4*1]) * METERS_PER_RADIAN;
	}
	else {
	  dx = (XSOLN[i] - XSOLN[1]) * METERS_PER_RADIAN;
	  dy = -(YSOLN[i] - YSOLN[1]) * METERS_PER_RADIAN;
	}
	MeshNodes[i].xw += (dx*cos(CCDRotationAngle)
			    - dy*sin(CCDRotationAngle));
	MeshNodes[i].yw += (dx*sin(CCDRotationAngle)
			    + dy*cos(CCDRotationAngle));
	MeshNodes[i].xp = MeshNodes[i].xw / PixelSpacing; 
	MeshNodes[i].yp = MeshNodes[i].yw / PixelSpacing;
      }
      if (VerboseFlag)
	printf("FECompute() : Node %2d at %.3f,%.3f moved by (%.2e,%.2e)\n",
	       i, MeshNodes[i].xp, MeshNodes[i].yp, dx, dy);
    }
  }

  /* free storage */
  XvgMM_Free(XSOLN);
  XvgMM_Free(YSOLN);
  XvgMM_Free(XSOLNB);
  XvgMM_Free(YSOLNB);

  /* close log file, if required */
  if (LogFileFp)
    fclose(LogFileFp);
}    

/************************* GenerateSampleFile() ************************/
/*                                                                     */
/*                                                                     */
/***********************************************************************/
void GenerateSampleFile(double *inpixels, double *outpixels,
			int height, int width, void *ParmPtrs[])
{
  FILE *fp;
  double offsetx, offsety;
  int spacing, i, j, k, fpx, fpy;

  /* fetch decimation parameter */
  spacing = *((int *) ParmPtrs[1]);

  /* check that an FE mesh is defined */
  if (FEMeshPixels == NULL) {
    Abort = B_TRUE;
    draw_mesg("Abort : undefined FE mesh in GenerateSampleFile()");
    return;
  }

  /* check that stack contains the correct number of images */
  /* outpixels                           : X data  */
  /* ImageStack[ImageStackTop -1].pixels : Y data */
  if (ImageStackTop < 1) {
    Abort = B_TRUE;
    draw_mesg("Abort : 1 stack image required in GenerateSampleFile(()");
    return;
  }
  ImageStackTop--;
  UpdateStackLabel(ImageStackTop);

  /* open output file */
  if ((fp = fopen(ParmPtrs[0], "w")) == NULL) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort : Can't open file \"%s\" in GenerateSampleFile()",
	    ParmPtrs[0]);
    draw_mesg(cbuf);
    return;
  }
  fprintf(fp,
	  "SPECKLE DISPLACEMENT DATA (Format: N X Y DX DX WX WY WDX WDY)\n");

  /* fetch field offset at position of fixed clamp (FE node 1) */
  fpx = MeshNodes[1].xp;
  fpy = MeshNodes[1].yp;
  offsetx = inpixels[fpy*width + fpx];
  offsety = (ImageStack[ImageStackTop].pixels)[fpy*width + fpx];

  /* scan the data and write to the output file */
  for (j = 0, k = 1; j < height; j += spacing)
    for (i = 0; i < width; i += spacing)
      if (FEMeshPixels[j*width + i] >= 0)
	fprintf(fp, "%d %e %e %e %e 1.0 1.0 1.0 1.0\n",
		k++,
		((double) (i - fpx))*PixelSpacing + MeshNodes[1].xw, 
		((double) (j - fpy))*PixelSpacing + MeshNodes[1].yw, 
		(inpixels[j*width + i] - offsetx)*METERS_PER_RADIAN,
		(-((ImageStack[ImageStackTop].pixels)[j*width + i] - offsety))
		*METERS_PER_RADIAN);
  
  /* close the output file */
  fclose(fp);
  
  /* transfer the data */
  for (k = 0; k < height*width; k++)
    outpixels[k] = inpixels[k];
}

/********************* BuildInterpolatedImage() ************************/
/*                                                                     */
/*                                                                     */
/***********************************************************************/
void BuildInterpolatedImage(double *inpixels, double *outpixels,
			    int height, int width, void *ParmPtrs[])
{
  FILE *fp;
  double SOLN[NNODES_MAX*4];
  int i, FieldModelType, flag;

  /* fetch flag parameter */
  flag = *((int *) ParmPtrs[1]);
  
  /* check that a mesh is loaded and defined */
  if (FEMeshDefined == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort : undefined mesh in BuildInterpolatedImage()");
    return;
  }

  /* fetch the X solution guess data */
  if ((fp = fopen(ParmPtrs[0], "r")) == NULL) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort : Can't open file %s in BuildInterpolatedImage()",
	    ParmPtrs[0]);
    draw_mesg(cbuf);
    return;
  }
  /* load in solution vector */
  for (i = 0; (i < nnodes*4) && (fscanf(fp, "%lf", &(SOLN[i])) == 1); i++);
  
  /* close the input file */
  fclose(fp);

  /* decide what model this is */
  if (i == nnodes)
    FieldModelType = BILINEAR_FIELD_MODEL;
  else if (i == nnodes*4)
    FieldModelType = BICUBIC_FIELD_MODEL;
  else {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort : %d values read from %s in BuildInterpolatedImage()",
	    i, ParmPtrs[0]);
    draw_mesg(cbuf);
    return;
  }

  /* calculate element coordinates if required */
  if (flag) {
    /* allocate storage for the field samples if required */
    if ((FieldSamples = (FESample *) XvgMM_Alloc((void *) FieldSamples,
						 height*width
						 *sizeof(FESample))) == NULL) {
      Abort = B_TRUE;
      draw_mesg("Abort: can't alloc FieldSamples in BuildInterpolatedImage()");
      return;
    }
    
    /* allocate storage for the mesh pixels if required and calc elm coords */
    draw_mesg("BuildInterpolatedImage() : Computing element coordinates...");
    if ((FEMeshPixels == NULL)
	|| (XvgMM_GetBlockSize(FEMeshPixels) != height*width*sizeof(short))) {
      if ((FEMeshPixels = (short *) XvgMM_Alloc((void *) FEMeshPixels,
						height*width*sizeof(short)))
	  == NULL) {
	Abort = B_TRUE;
	draw_mesg("Abort:can't alloc FEMeshPixels in BuildInterpolatedImage");
	return;
      }
      ComputeAllPixelsXiCoords(B_TRUE, B_TRUE, FieldModelType);
    }
    /* else just calculate the element coordinates */
    else {
      ComputeAllPixelsXiCoords(B_TRUE, B_FALSE, FieldModelType);
    }
    if (Abort)
      return;
  }
  /* else at least check that valid pointers to mesh data exist */
  else if ((FieldSamples == NULL) || (FEMeshPixels == NULL)) {
      Abort = B_TRUE;
      draw_mesg("Abort: No element coordinates in BuildInterpolatedImage()");
      return;
  }
  
  /* build interpolated image */
  draw_mesg("BuildInterpolatedImage() : Generating model values...");
  for (i = 0; i < height*width; i++) {
    if (FEMeshPixels[i] >= 0)
      outpixels[i] = ModelEval(FieldSamples[i], SOLN, FieldModelType);
    else
      outpixels[i] = 0;
  }
}

/*********************** EvalForceVolts ********************************/
/*                                                                     */
/* Routine which computes galvo restoring force voltages               */
/*                                                                     */
/***********************************************************************/
static double EvalForceVolts(double X, double max, double min, double *A,
			     int NPLUS1)
{
  double XCAP, P;
  int IFAIL, OPLUS1;
  
  IFAIL = -1;
  OPLUS1 = NPLUS1;
  XCAP = ((X - min) - (max - X))/(max - min);
  e02aef(&OPLUS1, A, &XCAP, &P, &IFAIL);
  return(P);
}

#ifdef FE_COMPUTE_DISPLACED_NODES_SPECKLE
/******************** FEComputeDisplacedNodes **************************/
/*                                                                     */
/* Routine which computes the displaced nodal coordinates from two     */
/* displacement field images in x (current image) and y (stack image)  */
/* for a series of images.                                             */
/*                                                                     */
/* The files are assumed to be organized in subdirectories             */
/*     mf    : median filtered wrapped files                           */
/*     uw    : unwrapped files                                         */
/*     cntx  : context files                                           */
/*     fe    : resulting fe files                                      */
/*                                                                     */
/***********************************************************************/
void FEComputeDisplacedNodes(double *inpixels, double *outpixels,
			     int height, int width, void *ParmPtrs[])
{
#ifdef FE_COMPUTE_FORCES
  static int PosIndx[NGALVOS]   = {19,18,17,31,30,29,28,27,26,25,24,23,22};
  static int ForceIndx[NGALVOS] = { 3, 2, 1,15,14,13,12,11,10, 9, 8, 7, 6};
  static double NperV[NGALVOS] = {5.138, 4.922, 4.97, 5.152, 4.98, 3.11,
				    5.028, 5.143,
				    4.715, 3.281, 3.089, 3.0, 4.996};
  static int GalvoIndx[NGALVOS] = {2,  1,  0, 14, 13, 12, 11, 10,  9,  8, 
				     7, 6, 5};
#endif
  MATFile *DataFpo, *DataFpi;
  Matrix *a;
  FILE *fpo;
  double *pixels, lmax, lmin, t0, *xnodes, *ynodes, *temp, fpx, fpy, FOThreshold,
#ifdef FE_COMPUTE_FORCES
  xcontact, ycontact, Newtons, theta, *Volts, *inforces, *ImDataP, 
#endif
  scratch[NGALVOS], *xforces, *yforces, *vafx, *vafy, TmpDouble;
  int i, j, nfiles, lheight, lwidth, *parms[4], n, FitType, fx, fy, lr, li,
  indx, FieldModelType;
  char lbuf[512], fname[512], mant[512], dir[512];

  /* load parms */
  FieldModelType = *((int *) ParmPtrs[2]);
  CCDRotationAngle = (*((double *) ParmPtrs[3]))/180.0*PI;

  /* draw banner */
  draw_mesg("FEComputeDisplacedNodes() : Initializing...");

  /* check that the processed image window is popped up */
  if (IsTrue(UxGetSet(ProcessedImageTBG)) == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort: Processed image window not popped up in FEComputeDisplacedNodes()");
    return;
  }
  XRaiseWindow(UxDisplay, XtWindow(ProcessedImageShell));
  XSync(UxDisplay, B_FALSE);
  ScreenSaverOff();

  /* check that sample structures have been initialized */
  if (FEMeshDefined == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort : Mesh unitialized in FEComputeDisplacedNodes()");
    return;
  }

  /* allocate storage for new images */
  if (XvgImgStack_Alloc(height, width) == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("FEComputeDisplacedNodes() : Failed to get space for buf1");
    return;
  }
  if (XvgImgStack_Alloc(height, width) == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("FEComputeDisplacedNodes() : Failed to get space for buf2");
    return;
  }
  if (XvgImgStack_Alloc(height, width) == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("FEComputeDisplacedNodes() : Failed to get space for buf3");
    return;
  }
  if (XvgImgStack_Alloc(height, width) == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("FEComputeDisplacedNodes() : Failed to get space for buf4");
    return;
  }

  /* create the dummy mask image */
  for (i = 0; i < height*width; i++)
    ImageStack[ImageStackTop -2].pixels[i] = 0;

  /* open input list file */
  if ((fpo = fopen((char *) (ParmPtrs[0]), "r")) == NULL) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort : Could not open file %s FEComputeDisplacedNodes()", 
	    (char *) (ParmPtrs[0]));
    draw_mesg(cbuf);
    ImageStackTop -= 4;
    UpdateStackLabel(ImageStackTop);
    return;
  }

  /* open MATLAB output file */
  if ((DataFpo=matOpen((char *) (ParmPtrs[1]), "w")) == NULL) {
    sprintf(cbuf, "Abort : Could not open file %s FEComputeDisplacedNodes()",
	    (char *) (ParmPtrs[1]));
    draw_mesg(cbuf);
    ImageStackTop -= 4;
    UpdateStackLabel(ImageStackTop);
    return;
  }

  /* save the initial pixel location of the fixed hook */
  fpx = MeshNodes[1].xp;
  fpy = MeshNodes[1].yp;

  /* figure out number of lines in the file */
  for (nfiles=0; (fgets(lbuf, 512, fpo) != NULL) && (lbuf[0] != '\n');
       nfiles++);

  /* allocate storage for the local storage */
  xnodes = (double *) XvgMM_Alloc(NULL, nnodes * nfiles * sizeof(double));
  ynodes = (double *) XvgMM_Alloc(NULL, nnodes * nfiles * sizeof(double));
  vafx = (double *) XvgMM_Alloc(NULL, nelements * nfiles * sizeof(double));
  vafy = (double *) XvgMM_Alloc(NULL, nelements * nfiles * sizeof(double));
  xforces = (double *) XvgMM_Alloc(NULL, NGALVOS * nfiles * sizeof(double));
  yforces = (double *) XvgMM_Alloc(NULL, NGALVOS * nfiles * sizeof(double));
  if ((xnodes == NULL) || (ynodes == NULL) || (vafx == NULL) || (vafy == NULL)
      || (xforces == NULL) || (yforces == NULL)) {
    Abort = B_TRUE;
    draw_mesg("Abort : Can't allocate in FEComputeDisplacedNodes()");
    ImageStackTop -= 4;
    UpdateStackLabel(ImageStackTop);
    return;
  }

  /* loop to load four images onto memory and compute displaced nodes */
  for (n=0, t0 = second(), fseek(fpo, 0, 0);
       (n < nfiles) && (Abort == B_FALSE); n++) {
    /* read file name */
    fgets(lbuf, 512, fpo);

    /* parse the line to obtain the mantissa & root directory */
    for (i = (strlen(lbuf) - 1); (i >= 0) && (lbuf[i] != 'x'); i--);
    for (j = i; (j >= 0) && (lbuf[j] != '/'); j--);
    lbuf[i] = '\0';
    for (i = 0, j++; lbuf[j] != '\0'; i++, j++)
      mant[i] = lbuf[j];
    mant[i] = '\0';
    if (j < 0) {
      sprintf(dir, "");
    }
    else {
      for (j = (strlen(lbuf) - 1); (j >= 0) && (lbuf[j] != '/'); j--);
      lbuf[j-2] = '\0';
      sprintf(dir, "%s", lbuf);
      if ((i == 0) || (j < 3)) {
	Abort = B_TRUE;
	sprintf(cbuf, "Abort: Parsing \"%s\" in FEComputeDisplacedNodes()",
		lbuf);
	draw_mesg(cbuf);
	ImageStackTop -= 4;
	UpdateStackLabel(ImageStackTop);
	UxFree(xnodes);
	UxFree(ynodes);
	UxFree(xforces);
	UxFree(yforces);
	UxFree(vafx);
	UxFree(vafy); 
	ScreenSaverOn();
	return;
      }
    }

    /* change the title of the processed window to the current file name */
    sprintf(cbuf, "%s (Processed)", mant);
    UxPutTitle(ProcessedImageShell, cbuf);

    /* load the unwrapped y data file */
    sprintf(fname, "%suw/%sy_diff_uw.mat", dir, mant);
    if (GetMatlabFile(&lheight, &lwidth, &lmin, &lmax, &pixels, fname,
		      VerboseFlag) == B_FALSE) {
      Abort = B_TRUE;
      sprintf(cbuf, "Abort : Can't open file %s in FEComputeDisplacedNodes()",
	      fname);
      draw_mesg(cbuf);
      ImageStackTop -= 4;
      UpdateStackLabel(ImageStackTop);
      UxFree(xnodes);
      UxFree(ynodes);
      UxFree(xforces);
      UxFree(yforces);
      UxFree(vafx);
      UxFree(vafy); 
      ScreenSaverOn();
      return;
    }
    for (i = 0; i < height*width; i++)
      ImageStack[ImageStackTop -4].pixels[i] = pixels[i];
    
    /* load the unwrapped x data file */
    sprintf(fname, "%suw/%sx_diff_uw.mat", dir, mant);
    if (GetMatlabFile(&lheight, &lwidth, &lmin, &lmax, &pixels, fname,
		      VerboseFlag) == B_FALSE) {
      Abort = B_TRUE;
      sprintf(cbuf, "Abort : Can't open file %s in FEComputeDisplacedNodes()",
	      fname);
      draw_mesg(cbuf);
      ImageStackTop -= 4;
      UpdateStackLabel(ImageStackTop);
      UxFree(xnodes);
      UxFree(ynodes);
      UxFree(xforces);
      UxFree(yforces);
      UxFree(vafx);
      UxFree(vafy); 
      ScreenSaverOn();
      return;
    }
    for (i = 0; i < height*width; i++)
      ImageStack[ImageStackTop -3].pixels[i] = pixels[i];
    XvgMM_Free(pixels);

    /* calculate new node positions */
    FitType = 99;
    FOThreshold = 0.8;
    parms[0] = (int *) (&FitType);
    parms[1] = (int *) (&FOThreshold);
    parms[2] = (int *) NULL;
    parms[3] = (int *) (&FieldModelType);
    FECompute(inpixels, outpixels, height, width, (void **) parms);

    /* save the nodes in the local storage, center with respect to plate */
    for (i = 0; i < nnodes; i++) {
      xnodes[i*nfiles + n] = MeshNodes[i].xw - MeshNodes[1].xw + FIXEDPX;
      ynodes[i*nfiles + n] = MeshNodes[i].yw - MeshNodes[1].yw - FIXEDPY;
    }

    /* save VAFs in local storage */
    for (i = 0; i < nelements; i++) {
      vafx[i*nfiles + n] = VAFsX[i];
      vafy[i*nfiles + n] = VAFsY[i];
    }

#ifdef FE_COMPUTE_FORCES
    /* load the A/D samples from the MATLAB experimental context file */
    sprintf(fname, "%scntx/%scntx.mat", dir, mant);
    if ((DataFpi=matOpen(fname, "r")) == NULL) {
      Abort = B_TRUE;
      sprintf(cbuf, "Abort : Can't open file %s in FEComputeDisplacedNodes()",
	      fname);
      draw_mesg(cbuf);
      ImageStackTop -= 4;
      UpdateStackLabel(ImageStackTop);
      UxFree(xnodes);
      UxFree(ynodes);
      UxFree(xforces);
      UxFree(yforces);
      UxFree(vafx);
      UxFree(vafy); 
      ScreenSaverOn();
      return;
    }
    if (matGetFull(DataFpi, "VoltsMean", &lr, &li, &Volts, &ImDataP) != 0) {
      Abort = B_TRUE;
      sprintf(cbuf, "Abort : Can't load \"VoltsMean\" from file %s in FEComputeDisplacedNodes()", fname);
      draw_mesg(cbuf);
      ImageStackTop -= 4;
      UpdateStackLabel(ImageStackTop);
      UxFree(xnodes);
      UxFree(ynodes);
      UxFree(xforces);
      UxFree(yforces);
      UxFree(vafx);
      UxFree(vafy); 
      ScreenSaverOn();
      return;
    }
    matClose(DataFpi);
    
    /* loop to compute X & Y restoring force components */
    for (i = 0; i < NGALVOS; i++) {
      indx = HookNodes[i]*nfiles + n;
      Newtons = -(Volts[ForceIndx[i]] -
		  EvalForceVolts(Volts[PosIndx[i]], Maxs[i], Mins[i],
				 &(ChebyCoeffs[i*NPLUS1]), NPLUS1)) * NperV[i];
      Newtons = (Newtons >= 0 ? Newtons : 0);
      ComputeGalvoContacts(ynodes[indx], xnodes[indx], &ycontact, &xcontact, 
			   GalvoIndx[i]);
      theta = atan2(((ycontact*0.001) - ynodes[indx]),
		    ((xcontact*0.001) - xnodes[indx]));
      xforces[i*nfiles + n] = Newtons * cos(theta);
      yforces[i*nfiles + n] = Newtons * sin(theta);
    }
    XvgMM_Free(Volts); XvgMM_Free(ImDataP);
#endif

    /* print out info */
    if (VerboseFlag)
      printf("Processed files \"%s*\" (n, nfiles) in %.2fs\n",
	     mant, n, nfiles, second() - t0);
    t0 = second();
  }

  /* write the hook nodes indicies array */
  for (i = 0; i < NGALVOS; i++)
    scratch[i] = HookNodes[i];
  a = mxCreateFull(1, NGALVOS, REAL);
  memcpy(mxGetPr(a), scratch, NGALVOS * sizeof(double));
  mxSetName(a, "HookNodes");
  matPutMatrix(DataFpo, a);
  mxFreeMatrix(a);

  /* write the displaced nodes to the MATLAB array */
  a = mxCreateFull(nfiles, nnodes, REAL);
  memcpy(mxGetPr(a), xnodes, nnodes * nfiles * sizeof(double));
  mxSetName(a, "nodesx");
  matPutMatrix(DataFpo, a);
  memcpy(mxGetPr(a), ynodes, nnodes * nfiles * sizeof(double));
  mxSetName(a, "nodesy");
  matPutMatrix(DataFpo, a);
  mxFreeMatrix(a);

  /* write the VAF info to the MATLAB array */
  a = mxCreateFull(nfiles, nelements, REAL);
  memcpy(mxGetPr(a), vafx, nelements * nfiles * sizeof(double));
  mxSetName(a, "vafx");
  matPutMatrix(DataFpo, a);
  memcpy(mxGetPr(a), vafy, nelements * nfiles * sizeof(double));
  mxSetName(a, "vafy");
  matPutMatrix(DataFpo, a);
  mxFreeMatrix(a);

  /* write the force information to the MATLAB array */
  a = mxCreateFull(nfiles, NGALVOS, REAL);
  memcpy(mxGetPr(a), xforces, NGALVOS * nfiles * sizeof(double));
  mxSetName(a, "xforces");
  matPutMatrix(DataFpo, a);
  memcpy(mxGetPr(a), yforces, NGALVOS * nfiles * sizeof(double));
  mxSetName(a, "yforces");
  matPutMatrix(DataFpo, a);
  mxFreeMatrix(a);

  /* write the initial fixed hook location & pixel spacing constant */
  a = mxCreateFull(1, 1, REAL);
  memcpy(mxGetPr(a), &fpx, sizeof(double));
  mxSetName(a, "fpx");
  matPutMatrix(DataFpo, a);
  memcpy(mxGetPr(a), &fpy, sizeof(double));
  mxSetName(a, "fpy");
  matPutMatrix(DataFpo, a);
  memcpy(mxGetPr(a), &PixelSpacing, sizeof(double));
  mxSetName(a, "PixelSpacing");
  matPutMatrix(DataFpo, a);
  TmpDouble = METERS_PER_RADIAN;
  memcpy(mxGetPr(a), &TmpDouble, sizeof(double));
  mxSetName(a, "METERS_PER_RADIAN");
  matPutMatrix(DataFpo, a);
  mxFreeMatrix(a);

  /* close the MATLAB output file */
  matClose(DataFpo);

  /* free local storage & release stack space */
  ImageStackTop -= 4;
  UpdateStackLabel(ImageStackTop);
  UxFree(xnodes);
  UxFree(ynodes);
  UxFree(xforces);
  UxFree(yforces);
  UxFree(vafx);
  UxFree(vafy); 
  ScreenSaverOn();
}
#endif


static void FEComputeDispNodesXCorrSaveData(char *fname,
					    int nstates,
					    double *nodesx,
					    double *nodesy,
					    double *nodesx_mm,
					    double *nodesy_mm, 
					    int cx, int cy,
					    double MMPerPixel)
{
  MATFile *fpo;
  Matrix *a;
  double v;

  /* open MATLAB output file */
  if ((fpo = matOpen(fname, "w")) == NULL) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort : Could not open file %s FEComputeDispNodesXCorr()",
	    fname);
    draw_mesg(cbuf);
    return;
  }
  
  /* write the node coordinates to the output MATLAB file */
  a = mxCreateFull(nstates, nnodes, REAL);
  memcpy(mxGetPr(a), nodesx, nnodes * nstates * sizeof(double));
  mxSetName(a, "nodesx");
  matPutMatrix(fpo, a);
  memcpy(mxGetPr(a), nodesy, nnodes * nstates * sizeof(double));
  mxSetName(a, "nodesy");
  matPutMatrix(fpo, a);
  memcpy(mxGetPr(a), nodesx_mm, nnodes * nstates * sizeof(double));
  mxSetName(a, "nodesx_mm");
  matPutMatrix(fpo, a);
  memcpy(mxGetPr(a), nodesy_mm, nnodes * nstates * sizeof(double));
  mxSetName(a, "nodesy_mm");
  matPutMatrix(fpo, a);
  mxFreeMatrix(a);

  /* write the pixel to world coordinate parameters to the MATLAB file */
  a = mxCreateFull(1, 1, REAL);
  v = cx;
  memcpy(mxGetPr(a), &v, sizeof(double));
  mxSetName(a, "cx");
  matPutMatrix(fpo, a);
  v = cy;
  memcpy(mxGetPr(a), &v, sizeof(double));
  mxSetName(a, "cy");
  matPutMatrix(fpo, a);
  memcpy(mxGetPr(a), &MMPerPixel, sizeof(double));
  mxSetName(a, "PixelSpacing");
  matPutMatrix(fpo, a);
  mxFreeMatrix(a);

  /* close the files */
  matClose(fpo);
}

void FEComputeDispNodesXCorr(double *inpixels, double *outpixels,
			     int height, int width, void *ParmPtrs[])
/***********************************************************************

FILE: feIPOPs.c

COMMAND: FEComputeDispNodesXCorr

DESCRIPTION:
  This function takes a series of full-field deformation images and computes
  the corresponding fitted finite element mesh node displacements.

  The function uses the following paramaters:
    - inpixels  : pointer to the input image buffer
    - outpixels : pointer to the output image buffer
    - height    : image height
    - width     : image width
    - ParmPtrs  : array of function parameters, where
       ParmPtrs[0] : Name of the mesh definition file (text), which lists
                     the boundary node pixel coordinates. The positions of the
                     remaining internal nodes are calculated automatically.
		     The format of the file is:
                        00 x00 y00
                        01 x01 y02
                           .
                           .
                           .
                        15 x15 y15
                     where the nodes are defined clockwise starting from the 
                     top/center node with reference to the digitized images.
       ParmPtrs[1] : Name of the text file listing the n input image files.
       ParmPtrs[2] : Name of the output file containing the fitted mesh data.
       ParmPtrs[3] : List of extra parameters:
              - parameters[0] : Pixel spacing in mm (ex: 0.075 for an image
                                with a horizontal width of 50mm)
	      - parameters[1] : Maximum number of lags to scan (ex: 4 or 5)
	      - parameters[2] : FT kernel size (ex: 16, 32 or 64)
	      - parameters[3] : World coordinate origin x pixel coordinate
                                (For the biaxial testing rig, this should
                                be the center of the rig, determined from
				an image of the rig).
	      - parameters[4] : World coordinate origin y pixel coordinate
                                (For the biaxial testing rig, this should
                                be the center of the rig, determined from
				an image of the rig).
	      - parameters[5] : Number of parallel processes in X
	      - parameters[6] : Number of parallel processes in Y
	      - parameters[7] : GenCMISSFIlesOnly
	                         -1 : Calculate the X and Y displacements
				      fields only, no fitting or CMISS files
				      generations.
	                          0 : Calculate the X and Y displacements
				      fields with cross-correlation, and
				      generate CMISS fields, nodes, and forces
				      files.
	                          1 : Generate CMISS fields, nodes, and forces
				      files only, using previously calculated
				      field with "GenCMISSFIlesOnly = 0 or -1".

RETURN: Void.

MODIFIED:
  - 5/March 1997, Paul Charette.

======================================================================*/
{
  DOUBLE_ARRAY *parameters, CMISSXparms, PatchXCorrByAreaParallel_Xparms;
  FILE *fpi;
  void *parms[4], *Dilationparms[4], *CMISSparms[4],
  *PatchXCorrByAreaParallel_parms[4];
  double *XSOLN, *YSOLN, *nodesx, *nodesy, *weights, *xfield, *yfield,
  MMPerPixel, CMISSXparmsData[4], *zeros, mV, lmin, lmax, *nodesx_mm,
  *nodesy_mm, *InitialForcesV, *InitialForceOffsetsV, *forcesV, *forceOffsetsV,
  InitialForces[NBOUNDARYNODES], InitialForcesScaledOnly[NBOUNDARYNODES],
  PatchXCorrByAreaParallel_XparmsData[5];
  int n, nstates, i, k, x, y, h, w, lheight, lwidth, lags, kernw, fifty,
  decimation, radius, cx, cy, GenCMISSFilesOnly, ngridx, ngridy;
  char inbuf[512], fname[512], ofname[512];

  /* verify input parameters */
  parameters = (DOUBLE_ARRAY *) ParmPtrs[3];
  if (parameters->n < 8) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort : 8 extra parms in FEComputeDispNodesXCorr()");
    draw_mesg(cbuf);
    return;
  }
  MMPerPixel         = (parameters->data)[0];
  lags               = (parameters->data)[1];
  kernw              = (parameters->data)[2];
  cx                 = (parameters->data)[3];
  cy                 = (parameters->data)[4];
  ngridx             = (parameters->data)[5];
  ngridy             = (parameters->data)[6];
  GenCMISSFilesOnly  = (parameters->data)[7];
  radius             = (kernw * 10)/32;

  /* Load the initial boundary node positions, if required */
  if (GenCMISSFilesOnly != -1) {
    draw_mesg("FEComputeDispNodesXCorr() : Loading initial mesh defintion...");
    if ((fpi = fopen((char *) (ParmPtrs[0]), "r")) == NULL) {
      Abort = B_TRUE;
      sprintf(cbuf, "Abort : Could not open file %s FEComputeDispNodesXCorr()", 
	      (char *) (ParmPtrs[0]));
      draw_mesg(cbuf);
      return;
    }
    for (i = 0; (i < 16) && (Abort == FALSE); i++) {
      if (fscanf(fpi, "%d %d %d\n", &k, &x, &y) != 3) {
	Abort = B_TRUE;
      }
      else {
	MeshNodes[(BoundaryNodes[i])].xp = x;
	MeshNodes[(BoundaryNodes[i])].yp = y;
      }
    }
    fclose(fpi);
    if (Abort) {
      Abort = B_TRUE;
      sprintf(cbuf,
	      "Abort:Loading nodes (line %d) in FEComputeDispNodesXCorr()",
	      i);
      draw_mesg(cbuf);
      return;
    }
    
    /* automatically calculate the starting internal node positions */
    InitNodes();
  }

  /* open input data list file */
  if ((fpi = fopen((char *) (ParmPtrs[1]), "r")) == NULL) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort : Could not open file %s FEComputeDispNodesXCorr()", 
	    (char *) (ParmPtrs[1]));
    draw_mesg(cbuf);
    return;
  }
  
  /* figure out number of lines in the input file */
  for (nstates=0; (fgets(inbuf, 512, fpi) != NULL) && (inbuf[0] != '\n');
       nstates++);
  fseek(fpi, 0, 0);
  if (VerboseFlag)
    printf("FEComputeDispNodesXCorr() : %d states to process.\n", nstates);

  /* allocate storage */
  draw_mesg("FEComputeDispNodesXCorr() : Allocating storage...");
  FEMeshPixels = (short *) XvgMM_Alloc((void *) FEMeshPixels,
				       height*width*sizeof(short));
  FieldSamples = (FESample *) XvgMM_Alloc((void *) FieldSamples,
					  height*width*sizeof(FESample));
  zeros  = (double *) XvgMM_Alloc(NULL, height*width*sizeof(double));
  XSOLN  = (double *) XvgMM_Alloc(NULL, nnodes * sizeof(double));
  YSOLN  = (double *) XvgMM_Alloc(NULL, nnodes * sizeof(double));
  nodesx = (double *) XvgMM_Alloc(NULL, nnodes * nstates * sizeof(double));
  nodesy = (double *) XvgMM_Alloc(NULL, nnodes * nstates * sizeof(double));
  nodesx_mm = (double *) XvgMM_Alloc(NULL, nnodes * nstates * sizeof(double));
  nodesy_mm = (double *) XvgMM_Alloc(NULL, nnodes * nstates * sizeof(double));
  if ((FEMeshPixels == NULL) || (FieldSamples == NULL) || (zeros == NULL)
      || (XSOLN == NULL) || (YSOLN == NULL)
      || (nodesx_mm == NULL) || (nodesy_mm == NULL)
      || (nodesx == NULL) || (nodesy == NULL)) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort : can't allocate in FEComputeDispNodesXCorr()");
    draw_mesg(cbuf);
    fclose(fpi);
    return;
  }
  
  /* set parameter values & addresses for subsequent calls to IP functions */
  parms[0] = (void *) inbuf;
  parms[1] = (void *) &lags;
  parms[2] = (void *) &kernw;
  parms[3] = (void *) &radius;
  fifty = 50;
  Dilationparms[0] = &fifty;
  decimation = 4;

  /* initialize internal fitting state and write CMISS files, if required */
  if (GenCMISSFilesOnly != -1) {
    /* pre-load parameters for calls to WriteCMISSFieldSamplesFile() */
    CMISSparms[0] = (void *) inbuf;
    CMISSparms[1] = (void *) &decimation;
    CMISSparms[2] = (void *) &MMPerPixel;
    CMISSparms[3] = (void *) &CMISSXparms;
    CMISSXparms.n = 2;
    CMISSXparms.data = CMISSXparmsData;
    CMISSXparms.data[0] = cx;
    CMISSXparms.data[1] = cy;

    /* store the initial mesh node coordinates locally */
    for (i = 0; i < nnodes; i++) {
      MeshNodes[i].xw = ((double) (MeshNodes[i].xp - cx))*MMPerPixel;
      MeshNodes[i].yw = ((double) (MeshNodes[i].yp - cy))*MMPerPixel;;
      nodesx[i*nstates] = MeshNodes[i].xp;
      nodesy[i*nstates] = MeshNodes[i].yp;
      nodesx_mm[i*nstates] = MeshNodes[i].xw;
      nodesy_mm[i*nstates] = MeshNodes[i].yw;
    }
    LoadOverlayFunction(FEMeshOverlay);

    /* zeros for use as dummy mask in LinearFieldFit() */
    for (i = 0; i < height*width; i++)
      zeros[i] = 0;

    /* write starting node positions CMISS file, if required */
    fscanf(fpi, "%s\n", inbuf);
    for (i = (strlen(inbuf) - 1); (i >= 0) && (inbuf[i] != '.'); i--);
    inbuf[i] = '\0';
    WriteCMISSNodesFile(inpixels, inpixels, height, width, parms);

    /* read in the initial boundary force and offset voltages, convert to N */
    forcesV = NULL;
    forceOffsetsV = NULL;
    InitialForcesV = NULL;
    InitialForceOffsetsV = NULL;
    sprintf(fname, "%s_forces.mat", inbuf);
    if (GetArrayFromMatlabFile(&InitialForcesV, &h, &w, fname, "forces")
	== FALSE) {
      Abort = B_TRUE;
      sprintf(cbuf,
	      "Abort : Error reading from %s in FEComputeDispNodesXCorr()",
	      fname);
      draw_mesg(cbuf);
      fclose(fpi);
      return;
    }
    sprintf(ofname, "%sffsetXX_forces.mat", inbuf);
    ofname[strlen(inbuf)+6] = inbuf[strlen(inbuf)-1];
    ofname[strlen(inbuf)+5] = inbuf[strlen(inbuf)-2];
    ofname[strlen(inbuf)-2] = '_';
    ofname[strlen(inbuf)-1] = 'o';
    if (GetArrayFromMatlabFile(&InitialForceOffsetsV, &h, &w, ofname, "forces")
	== FALSE) {
      Abort = B_TRUE;
      sprintf(cbuf,
	      "Abort : Error reading from %s in FEComputeDispNodesXCorr()",
	      ofname);
      draw_mesg(cbuf);
      fclose(fpi);
      return;
    }
    for (i = 0; i < NBOUNDARYNODES; i++) {
      mV = (InitialForcesV[i] - InitialForceOffsetsV[i])*1000.0
	+ NoForceOffsets_mV[i];
      InitialForcesScaledOnly[i] = (mV - NoForceOffsets_mV[i])*mV_to_mN_a1[i];
      InitialForces[i] =
	mV_to_mN_a0[i] + mV*(mV_to_mN_a1[i] + mV*mV_to_mN_a2[i]);
    }

    /* write the forces to CMISS and MATLAB files */
    sprintf(fname, "%s_initial", inbuf);
    WriteCMISSForcesFile(zeros, zeros, InitialForcesV, InitialForceOffsetsV,
			 cx, cy, fname);
    
    /* free storage */
    XvgMM_Free(InitialForcesV);
    XvgMM_Free(InitialForceOffsetsV);
  }
    
  /* required initializations for correlation calcultaions */
  if ((GenCMISSFilesOnly == 0) || (GenCMISSFilesOnly == -1)) {
    /* pre-load calling parameters for PatchXCorrByAreaParallel_parms() */
    PatchXCorrByAreaParallel_parms[0] = (void *) inbuf;
    PatchXCorrByAreaParallel_parms[1] = (void *) &ngridx;
    PatchXCorrByAreaParallel_parms[2] = (void *) &ngridy;
    PatchXCorrByAreaParallel_parms[3] =
      (void *) &PatchXCorrByAreaParallel_Xparms;
    PatchXCorrByAreaParallel_Xparms.n = 5;
    PatchXCorrByAreaParallel_Xparms.data = PatchXCorrByAreaParallel_XparmsData;
    PatchXCorrByAreaParallel_Xparms.data[0] = lags;
    PatchXCorrByAreaParallel_Xparms.data[1] = kernw;
    PatchXCorrByAreaParallel_Xparms.data[2] = radius;
    PatchXCorrByAreaParallel_Xparms.data[3] = 0;
    PatchXCorrByAreaParallel_Xparms.data[4] = 0;

    /* load the first image into the inpixels buffer */
    fseek(fpi, 0, 0);
    fscanf(fpi, "%s\n", inbuf);
    sprintf(cbuf, "FEComputeDispNodesXCorr() : Reading file %s...", inbuf);
    draw_mesg(cbuf);
    LoadImageFile(inpixels, inpixels, height, width, parms);
    if (Abort) {
      fclose(fpi);
      return;
    }
  }

  /* loop to fit x & y displacement fields and compute node displacements */
  for (n = 1; (n < nstates) && (Abort == B_FALSE); n++) {
    /* read in the next filename */
    fscanf(fpi, "%s\n", inbuf);

    /* calculate the displacement field values, if required */
    if ((GenCMISSFilesOnly == 0) || (GenCMISSFilesOnly == -1)) {
      /* get rid of the older image on the stack */
      ImageStackTop--;
      UpdateStackLabel(ImageStackTop);
      
      /* push the current image and load the next image in the list */
      sprintf(cbuf, "FEComputeDispNodesXCorr() : Reading file %s...", inbuf);
      draw_mesg(cbuf);
      LoadImageFile(inpixels, inpixels, height, width, parms);
      if (Abort) {
	FEComputeDispNodesXCorrSaveData(ParmPtrs[2], nstates, nodesx, nodesy,
					nodesx_mm, nodesy_mm, cx, cy,
					MMPerPixel);
	fclose(fpi);
	return;
      }
      
      /* extract the filename mantissa */
      for (i = (strlen(inbuf) - 1); (i >= 0) && (inbuf[i] != '.'); i--);
      inbuf[i] = '\0';
      
      /* calculate the shift between the two images and write the results */
      PatchXCorrByAreaParallel(inpixels, outpixels, height, width,
			       PatchXCorrByAreaParallel_parms);
      if (Abort) {
	FEComputeDispNodesXCorrSaveData(ParmPtrs[2], nstates, nodesx, nodesy,
					nodesx_mm, nodesy_mm, cx, cy,
					MMPerPixel);
	fclose(fpi);
	return;
      }
    }
    /* else just extract the filename mantissa */
    else {
      for (i = (strlen(inbuf) - 1); (i >= 0) && (inbuf[i] != '.'); i--);
      inbuf[i] = '\0';
    }

    /* fit the displacement fields and write CMISS files if required */
    if (GenCMISSFilesOnly != -1) {
      /* read the x field data */
      sprintf(fname, "%s_shiftsX.mat", inbuf);
      sprintf(cbuf, "FEComputeDispNodesXCorr() : Reading file %s...", fname);
      draw_mesg(cbuf);
      if (GetMatlabFile(&lheight, &lwidth, &lmin, &lmax, &xfield, fname,
			VerboseFlag) == B_FALSE) {
	Abort = B_TRUE;
	sprintf(cbuf, "Abort : Can't open %s in FEComputeDispNodesXCorr()",
		fname);
	draw_mesg(cbuf);
	FEComputeDispNodesXCorrSaveData(ParmPtrs[2], nstates, nodesx, nodesy,
					nodesx_mm, nodesy_mm, cx, cy,
					MMPerPixel);
	fclose(fpi);
	return;
      }
      
      /* read the y field data */
      sprintf(fname, "%s_shiftsY.mat", inbuf);
      sprintf(cbuf, "FEComputeDispNodesXCorr() : Reading file %s...", fname);
      draw_mesg(cbuf);
      if (GetMatlabFile(&lheight, &lwidth, &lmin, &lmax, &yfield, fname,
			VerboseFlag) == B_FALSE) {
	Abort = B_TRUE;
	sprintf(cbuf, "Abort : Can't open %s in FEComputeDispNodesXCorr()",
		fname);
	draw_mesg(cbuf);
	FEComputeDispNodesXCorrSaveData(ParmPtrs[2], nstates, nodesx, nodesy,
					nodesx_mm, nodesy_mm, cx, cy,
					MMPerPixel);
	fclose(fpi);
	return;
      }
      
      /* read the %VAF data, scale to 0..1, use as weights */
      sprintf(cbuf, "FEComputeDispNodesXCorr() : Reading file %s...", fname);
      draw_mesg(cbuf);
      sprintf(fname, "%s_VAFs.mat", inbuf);
      if (GetMatlabFile(&lheight, &lwidth, &lmin, &lmax, &weights, fname,
			VerboseFlag) == B_FALSE) {
	Abort = B_TRUE;
	sprintf(cbuf, "Abort : Can't open %s in FEComputeDispNodesXCorr()",
		fname);
	draw_mesg(cbuf);
	FEComputeDispNodesXCorrSaveData(ParmPtrs[2], nstates, nodesx, nodesy,
					nodesx_mm, nodesy_mm, cx, cy,
					MMPerPixel);
	fclose(fpi);
	return;
      }
      for (i = 0; i < height*width; i++) {
	if (weights[i] < 0.0)
	  weights[i] = 0.0;
	weights[i] /= 100.0;
      }
      
      /* calculate the element coordinates of all field samples */
      draw_mesg("FEComputeDispNodesXCorr() : Computing element coordinates..");
      ComputeAllPixelsXiCoords(B_TRUE, B_TRUE, BILINEAR_FIELD_MODEL);
      if (Abort) {
	FEComputeDispNodesXCorrSaveData(ParmPtrs[2], nstates, nodesx, nodesy,
					nodesx_mm, nodesy_mm, cx, cy,
					MMPerPixel);
	fclose(fpi);
	return;
      }
      
      /* load the pixel mask with valid mesh pixels only */
      for (i = 0; i < height*width; i++) {
	if (FEMeshPixels[i] >= 0)
	  MaskPixels[i] = 1.0;
	else
	  MaskPixels[i] = 0.0;
      }
      DisplayContourComputed = B_TRUE;
      
      /* dilate the mask by 50 pixels to include samples around the border */
      draw_mesg("FEComputeDispNodesXCorr() : Dilating mask...");
#if defined (NAG)
      DilationFFT(MaskPixels, MaskPixels, height, width, Dilationparms);
#else /* defined (NAG) */
			Abort=B_TRUE;
#endif /* defined (NAG) */
      if (Abort) {
	FEComputeDispNodesXCorrSaveData(ParmPtrs[2], nstates, nodesx, nodesy,
					nodesx_mm, nodesy_mm, cx, cy,
					MMPerPixel);
	fclose(fpi);
	return;
      }
      
#ifdef DEBUG
      /* write mask file */
      draw_mesg("FEComputeDispNodesXCorr() : Writing mask to disk...");
      sprintf(fname, "%s_mask.mat", inbuf);
      WriteMatlabFile(height, width, "mask", MaskPixels, fname);
      if (Abort) {
	FEComputeDispNodesXCorrSaveData(ParmPtrs[2], nstates, nodesx, nodesy,
					nodesx_mm, nodesy_mm, cx, cy,
					MMPerPixel);
	fclose(fpi);
	return;
      }
#endif
      
      /* write the X & Y displacement fields to the CMISS file */
      XvgImgStack_Push(yfield, height, width);
      XvgImgStack_Push(xfield, height, width);
      WriteCMISSFieldSamplesFile(weights, weights, height, width, CMISSparms);
      ImageStackTop -= 2;
      UpdateStackLabel(ImageStackTop);
      if (Abort) {
	FEComputeDispNodesXCorrSaveData(ParmPtrs[2], nstates, nodesx, nodesy,
					nodesx_mm, nodesy_mm, cx, cy,
					MMPerPixel);
	fclose(fpi);
	return;
      }
      
      /* Fit the nodal field parameters to the data */
      draw_mesg("FEComputeDispNodesXCorr() : Fitting...");
      if (LinearFieldFit(nnodes, nnodes, nnodes, XSOLN, YSOLN, 0, 0, 0, 0,
			 height, width, zeros, zeros, xfield, yfield, 
			 weights, weights, NULL, NULL, NULL, NULL, NULL, NULL,
			 BILINEAR_FIELD_MODEL, NULL, B_TRUE, B_FALSE)
	  == B_FALSE) {
	Abort = B_TRUE;
	FEComputeDispNodesXCorrSaveData(ParmPtrs[2], nstates, nodesx, nodesy,
					nodesx_mm, nodesy_mm, cx, cy,
					MMPerPixel);
	fclose(fpi);
	return;
      }
      
      /* update the node coordinates with the fitted field values */
      for (i = 0; i < nnodes; i++) {
	MeshNodes[i].xp += XSOLN[i];
	MeshNodes[i].yp += YSOLN[i];
	MeshNodes[i].xw = ((double) (MeshNodes[i].xp - cx))*MMPerPixel;
	MeshNodes[i].yw = ((double) (MeshNodes[i].yp - cy))*MMPerPixel;;
	nodesx[i*nstates + n] = MeshNodes[i].xp;
	nodesy[i*nstates + n] = MeshNodes[i].yp;
	nodesx_mm[i*nstates + n] = MeshNodes[i].xw;
	nodesy_mm[i*nstates + n] = MeshNodes[i].yw;
      }
      
      /* write the CMISS nodes file */
      WriteCMISSNodesFile(inpixels, inpixels, height, width, parms);
      
      /* read in the boundary forces */
      sprintf(fname, "%s_forces.mat", inbuf);
      if (GetArrayFromMatlabFile(&forcesV, &h, &w, fname, "forces")
	  == FALSE) {
	Abort = B_TRUE;
	sprintf(cbuf,
		"Abort : Error reading from %s in FEComputeDispNodesXCorr()",
		fname);
	draw_mesg(cbuf);
	fclose(fpi);
	return;
      }
      sprintf(ofname, "%sffsetXX_forces.mat", inbuf);
      ofname[strlen(inbuf)+6] = inbuf[strlen(inbuf)-1];
      ofname[strlen(inbuf)+5] = inbuf[strlen(inbuf)-2];
      ofname[strlen(inbuf)-2] = '_';
      ofname[strlen(inbuf)-1] = 'o';
      if (GetArrayFromMatlabFile(&forceOffsetsV, &h, &w, ofname, "forces")
	  == FALSE) {
	Abort = B_TRUE;
	sprintf(cbuf,
		"Abort : Error reading from %s in FEComputeDispNodesXCorr()",
		ofname);
	draw_mesg(cbuf);
	fclose(fpi);
	return;
      }
      
      /* convert the forces to a CMISS file */
      WriteCMISSForcesFile(InitialForces, InitialForcesScaledOnly,
			   forcesV, forceOffsetsV, cx, cy, inbuf);
      
      /* debug */
      XvgMM_MemoryCheck("in FEComputeDispNodesXCorr()");
    }
  }
  
  /* get rid of the older image on the stack, if required */
  if ((GenCMISSFilesOnly == 0) || (GenCMISSFilesOnly == -1)) {
    ImageStackTop--;
    UpdateStackLabel(ImageStackTop);
  }
  
  /* save the mesh data in the MATLAB file */
  if (GenCMISSFilesOnly != -1)
    FEComputeDispNodesXCorrSaveData(ParmPtrs[2], nstates, nodesx, nodesy,
				    nodesx_mm, nodesy_mm, cx, cy,
				    MMPerPixel);
  
  /* close the files */
  fclose(fpi);
  
  /* restore input image */
  for (i = 0; i < height*width; i++)
    outpixels[i] = inpixels[i];
  
  /* free local storage & release stack space */
  UxFree(forcesV);
  UxFree(forceOffsetsV);
  UxFree(zeros);
  UxFree(nodesx);
  UxFree(nodesy);
  UxFree(XSOLN);
  UxFree(YSOLN);
}

/***************************    System   *******************************/
/*                                                                     */
/* Routine executes a shell command                                    */
/*                                                                     */
/***********************************************************************/
void System(double *inpixels, double *outpixels,
	    int height, int width, void *ParmPtrs[])
{
  int i;

  system(ParmPtrs[0]);

  for (i = 0; i < height*width; i++)
    outpixels[i] = inpixels[i];
}


/********************* ReadCMISSFieldSamplesFile ***********************/
/*                                                                     */
/* Read field samples in CMISS file.                                   */
/*                                                                     */
/* Parameters:                                                         */
/*   ParmPtrs[0] : Output file name.                                   */
/*                                                                     */
/*                                                                     */
/***********************************************************************/
void ReadCMISSFieldSamplesFile(double *inpixels, double *outpixels,
				int height, int width, void *ParmPtrs[])
{
  FILE *fp;
  double MMPerPixel, x, y, fx, fy, wx, wy, wfx, wfy;
  int indx, i, j, n, cx, cy, xp, yp, decimation, ftype, iflag, heightd, widthd,
  IFAIL, xi, yi, k;
  char inbuf[512];

  /* get parameters */
  ftype = *((int *) ParmPtrs[1]);
  iflag = *((int *) ParmPtrs[2]);

  /* open the input file */
  if ((fp = fopen(ParmPtrs[0], "r")) == NULL) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "ReadCMISSFieldSamplesFile():Could not open file \"%s\"",
	    ParmPtrs[0]);
    draw_mesg(cbuf);
    fclose(fp);
    return;
  }
  
  /* read in the file header */
  fgets(inbuf, 512, fp);
  if (sscanf(inbuf, "Biaxial displacement field samples: x y fx fy wx wy wfx wfy (cx:%d, cy:%d, MMPerPixel:%le, decimation:%d)\n",
	     &cx, &cy, &MMPerPixel, &decimation) != 4) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "ReadCMISSFieldSamplesFile(): Error parsing header (%s) in \"%s\"",
	    inbuf, ParmPtrs[0]);
    draw_mesg(cbuf);
    return;
  }
  if (VerboseFlag) 
    printf("ReadCMISSFieldSamplesFile(cx:%d, cy:%d, MMPerPixel:%f, dec:%d)\n",
	   cx, cy, MMPerPixel, decimation);
  
  /* zero to the output buffer */
  for (i = 0; i < height*width; i++)
    outpixels[i] = -100;

  /* set up internal parameters */
  widthd = width/decimation;
  heightd = height/decimation;

  /* setup FFT buffers if required */
  if (iflag) {
    draw_mesg("ReadCMISSFieldSamplesFile() : Initializing FFT buffers...");
    if (FFTInit(height, width) == B_FALSE) {
      Abort = B_TRUE;
      draw_mesg("Abort : FFTInit() failed in ReadCMISSFieldSamplesFile()");
      return;
    }
    if (FFTInit2(heightd, widthd) == B_FALSE) {
      Abort = B_TRUE;
      draw_mesg("Abort : FFTInit2() failed in ReadCMISSFieldSamplesFile()");
      return;
    }
    for (i = 0; i < height*width; i++) {
	FFTRealBuffer[i] = 0;
	FFTImBuffer[i] = 0;
    }
    for (i = 0; i < heightd*widthd; i++) {
      FFTRealBuffer2[i] = 0;
      FFTImBuffer2[i] = 0;
    }
  }

  /* loop to read the points */
  draw_mesg("ReadCMISSFieldSamplesFile() : Reading from the CMISS file...");
  i = 0;
  while ((fscanf(fp, "%d %le %le %le %le %le %le %le %le\n",
		 &indx, &x, &y, &fx, &fy, &wx, &wy, &wfx, &wfy) == 9)
	 && (Abort == B_FALSE)) {
    /* load the filed sample in the output buffer if required */
    if (iflag == 0) {
      xp = rint(x/MMPerPixel) + cx;
      yp = rint(y/MMPerPixel) + cy;
      if ((xp >= 0) && (xp < width) && (yp >= 0) && (yp < height)) {
	if (ftype == 0)
	  outpixels[yp*width + xp] = fx;
	else
	  outpixels[yp*width + xp] = fy;
	i++;
      }
      else {
	Abort = B_TRUE;
	sprintf(cbuf,
		"ReadCMISSFieldSamplesFile():coords (%d, %d) out of range",
		xp, yp);
	draw_mesg(cbuf);
      }
    }
    /* else load the field sample in the FFT buffer */
    else {
      xp = rint((x/MMPerPixel + ((double) cx))/((double) decimation));
      yp = rint((y/MMPerPixel + ((double) cy))/((double) decimation));
      if ((xp >= 0) && (xp < widthd) && (yp >= 0) && (yp < heightd)) {
	if (ftype == 0)
	  FFTRealBuffer2[yp*widthd + xp] = fx/MMPerPixel;
	else
	  FFTRealBuffer2[yp*widthd + xp] = fy/MMPerPixel;
	i++;
      }
      else {
	Abort = B_TRUE;
	sprintf(cbuf,
		"ReadCMISSFieldSamplesFile():coords (%d, %d) out of range",
		xp, yp);
	draw_mesg(cbuf);
      }
    }
  }
  if (VerboseFlag) 
    printf("ReadCMISSFieldSamplesFile() : read in %d points\n", i);

  /* close the file */
  fclose(fp);

  /* build interpolated image if required */
  if (iflag && (Abort == B_FALSE)) {
    /* calculate FFT of the decimated image */
    draw_mesg("ReadCMISSFieldSamplesFile() : Calculating image FFT...");
    IFAIL = 1;
    c06fuf(&widthd, &heightd, FFTRealBuffer2, FFTImBuffer2, "Initial",
	   FFTTrigm2, FFTTrign2, FFTWork2, &IFAIL);
    if (IFAIL != 0) {
      Abort = B_TRUE;
      sprintf(cbuf,"Abort:IFAIL=%d c06fuf(1) in ReadCMISSFieldSamplesFile()",
	      IFAIL);
      draw_mesg(cbuf);
      return;
    }
    
    /* zero pad the FT of the decimated image to the full image size */
    draw_mesg("ReadCMISSFieldSamplesFile() : Zero padding image FFT...");
    FFTShift(FFTRealBuffer2, FFTRealBuffer2, heightd, widthd);
    FFTShift(FFTImBuffer2, FFTImBuffer2, heightd, widthd);
    for (j = 0, k = 0; j < heightd; j++) {
      yi = (height*(decimation - 1))/(2*decimation) + j;
      xi = (width *(decimation - 1))/(2*decimation);
      indx = yi*width + xi;
      for (i = 0; i < widthd; i++, k++, indx++) {
	FFTRealBuffer[indx] = FFTRealBuffer2[k];
	FFTImBuffer[indx] = FFTImBuffer2[k];
      }
    }
    FFTShift(FFTRealBuffer, FFTRealBuffer, height, width);
    FFTShift(FFTImBuffer, FFTImBuffer, height, width);

    /* calculate inverse FFT of interpolated image */
    draw_mesg("ReadCMISSFieldSamplesFile() : Calculating inverse FFT...");
    n = heightd*widthd;
    IFAIL = 1;
    c06gcf(FFTImBuffer, &n, &IFAIL);
    if (IFAIL != 0) {
      Abort = B_TRUE;
      sprintf(cbuf,
	      "Abort:IFAIL=%d from c06gcf() in ReadCMISSFieldSamplesFile()",
	      IFAIL);
      draw_mesg(cbuf);
      return;
    }
    IFAIL = 1;
    c06fuf(&width, &height, FFTRealBuffer, FFTImBuffer,
	   "Initial", FFTTrigm, FFTTrign, FFTWork, &IFAIL);
    if (IFAIL != 0) {
      Abort = B_TRUE;
      sprintf(cbuf,
	      "Abort:IFAIL=%d from c06fuf(2) in ReadCMISSFieldSamplesFile()",
	      IFAIL);
      draw_mesg(cbuf);
      return;
    }

    /* transfer data to output buffer */
    for (i = 0; i < height*width; i++)
      outpixels[i] = FFTRealBuffer[i];
  }
}


/******************** WriteCMISSFieldSamplesFile ***********************/
/*                                                                     */
/* Write field samples in CMISS file. The units of the input x and y   */
/* fields are assumed to be pixels. Hence, they are multipled by the   */
/* input parameter "MMPerPixel" to scale to physical units.            */
/*                                                                     */
/* Parameters:                                                         */
/*   ParmPtrs[0] : Output file name mantissa.                          */
/*   ParmPtrs[1] : Decimation factor.                                  */
/*   ParmPtrs[2] : Pixel spacing in mm.                                */
/*   ParmPtrs[3] : Extra parameters:                                   */
/*         - parameters[0] : X pixel coordinate of center point        */
/*         - parameters[1] : Y pixel coordinate of center point        */
/*                                                                     */
/* Stack images:                                                       */
/*   ImageStack[ImageStackTop-2] : Y displacement field                */
/*   ImageStack[ImageStackTop-1] : X displacement field                */
/*   Current image               : weights                             */
/*                                                                     */
/*                                                                     */
/***********************************************************************/
void WriteCMISSFieldSamplesFile(double *inpixels, double *outpixels,
				int height, int width, void *ParmPtrs[])
{
  DOUBLE_ARRAY *parameters;
  FILE *fp;
  double MMPerPixel;
  int i, j, n, decimation, cx, cy;
  char fname[256];

  /* check that the stack contains the correct number of images */
  if (ImageStackTop < 2) {
    Abort = B_TRUE;
    draw_mesg("Abort: need 2 images on stack in WriteCMISSFieldSamplesFile()");
    return;
  }
  
  /* check that a mask image is defined */
  if (DisplayContourComputed == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort : undefined mask in WriteCMISSFieldSamplesFile()");
    return;
  }
    
  /* load input parameters */
  decimation = *((int *) ParmPtrs[1]);
  MMPerPixel = *((double *) ParmPtrs[2]);
  parameters = (DOUBLE_ARRAY *) ParmPtrs[3];
  if (parameters->n < 2) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort : 2 extra parms in WriteCMISSFieldSamplesFile()");
    draw_mesg(cbuf);
    return;
  }
  cx = (parameters->data)[0];
  cy = (parameters->data)[1];

  /* open the nodes .ipnode output file */
  sprintf(fname, "%s.ipdata", ParmPtrs[0]);
  if ((fp = fopen(fname, "w")) == NULL) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "WriteCMISSFieldSamplesFile():Could not open output file \"%s\"",
	    fname);
    draw_mesg(cbuf);
    return;
  }
  
  /* write out the file header */
  fprintf(fp, "Biaxial displacement field samples: x y fx fy wx wy wfx wfy (cx:%d, cy:%d, MMPerPixel:%f, decimation:%d)\n",
	  cx, cy, MMPerPixel, decimation);
  
  /* loop to write the points */
  draw_mesg("WriteCMISSFieldSamplesFile() : Writing to data file...");
  for (j = 0, n = 1; j < height; j+=decimation) {
    for (i = 0; i < width; i+=decimation) {
      if ((MaskPixels[j*width + i] > 0) 
	  && !(((ImageStack[ImageStackTop -1].pixels)[j*width + i] == 0.0)
	       && ((ImageStack[ImageStackTop -2].pixels)[j*width + i] == 0.0)))
	{
	  fprintf(fp, "%6d %12f %12f %12f %12f %12f %12f %12f %12f\n",
		  n++,
		  ((double) (i - cx))*MMPerPixel,
		  ((double) (j - cy))*MMPerPixel,
		  ((ImageStack[ImageStackTop -1].pixels)[j*width + i])
		  *MMPerPixel,
		  ((ImageStack[ImageStackTop -2].pixels)[j*width + i])
		  *MMPerPixel,
		  1.0, 1.0,
		  inpixels[j*width + i],
		  inpixels[j*width + i]);
	}
    }
  }
    
  /* close the nodes .ipnode file */
  fclose(fp);
  if (VerboseFlag)
    printf("WriteCMISSFieldSamplesFile() : %d FieldSamples written\n", n);
  
  /* transfer data to the output buffer */
  for (i = 0; i < height*width; i++)
    outpixels[i] = inpixels[i];
}


/********************** WriteCMISSPointsFile ***************************/
/*                                                                     */
/* Write the current mesh out as CMISS description files               */
/*                                                                     */
/* Parameters:                                                         */
/*   ParmPtrs[0] : Output file name mantissa.                          */
/*   ParmPtrs[1] : Data type (0:XYZW data, 1:Z only)                   */
/*   ParmPtrs[2] : Decimation factor.                                  */
/*   ParmPtrs[3] : Extra parameters:                                   */
/*         - parameters[0] : Weights threshold  (type 0 only)          */
/*         - parameters[1] : Pixel spacing (mm) (type 1 only)          */
/*         - parameters[2] : X pixel coordinate of the origin (t1 only)*/
/*         - parameters[3] : Y pixel coordinate of the origin (t1 only)*/
/*                                                                     */
/* Stack images:                                                       */
/*   ImageStack[ImageStackTop-3] : Weights (Optional)                  */
/*   ImageStack[ImageStackTop-2] : X       (Optional)                  */
/*   ImageStack[ImageStackTop-1] : Y       (Optional)                  */
/*   Current image               : Z                                   */
/*                                                                     */
/*                                                                     */
/***********************************************************************/
void WriteCMISSPointsFile(double *inpixels, double *outpixels,
			  int height, int width, void *ParmPtrs[])
{
  DOUBLE_ARRAY *parameters;
  FILE *fp;
  double MMPerPixel, threshold, max, min;
  int i, j, index, n, decimation, DataType, cx, cy;
  char fname[256];

  /* load input parameters */
  DataType = *((int *) ParmPtrs[1]);
  decimation = *((int *) ParmPtrs[2]);
  parameters = (DOUBLE_ARRAY *) ParmPtrs[3];
  if (parameters->n < 4) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort : 4 extra parms in WriteCMISSPointsFile()");
    draw_mesg(cbuf);
    return;
  }
  threshold  = (parameters->data)[0];
  MMPerPixel = (parameters->data)[1];
  cx         = (parameters->data)[2];
  cy         = (parameters->data)[3];

  /* check that a mask image is defined */
  if (DisplayContourComputed == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort : undefined mask in WriteCMISSPointsFile()");
    return;
  }
    
  /* XYZW data type */
  if (DataType == 0) {
    /* check that there are two images on the stack */
    if (ImageStackTop < 3) {
      Abort = B_TRUE;
      sprintf(cbuf,
	      "Abort: 3 images required on stack in WriteCMISSPointsFile()");
      draw_mesg(cbuf);
      return;
    }
    
    /* compute replacement value for nulled pixels */
    DynamicRange(inpixels, height*width, &max, &min);
    
    /* count points */
    draw_mesg("WriteCMISSPointsFile() : Counting valid data points...");
    for (j = 0, n = 0; j < height; j+=decimation) {
      for (i = 0; i < width; i+=decimation) {
	if (MaskPixels[j*width + i]
	    && (ImageStack[ImageStackTop-3].pixels[j*width + i] > threshold))
	  n++;
      }
    }
    
    /* open the nodes .ipnode output file */
    sprintf(fname, "%s.ipdata", ParmPtrs[0]);
    if ((fp = fopen(fname, "w")) == NULL) {
      Abort = B_TRUE;
      sprintf(cbuf,
	      "WriteCMISSPointsFile():Could not open the output file \"%s\"",
	      fname);
      draw_mesg(cbuf);
      return;
    }
    
    /* print out the nodes .ipnode file header */
    fprintf(fp, "Geometry data file (%d points)\n", n);
    
    /* loop to write the points */
    sprintf(cbuf,
	    "WriteCMISSPointsFile() : Writing %d pts to data file...", n);
    draw_mesg(cbuf);
    for (j = 0, n = 0; j < height; j+=decimation) {
      for (i = 0; i < width; i+=decimation) {
	index = j*width + i;
	if (MaskPixels[index]
	    && (ImageStack[ImageStackTop-3].pixels[index] > threshold))
	  fprintf(fp, "%6d %12f %12f %12f %12f %12f %12f\n",
		  n++,
		  ImageStack[ImageStackTop-2].pixels[index],
		  ImageStack[ImageStackTop-1].pixels[index],
		  inpixels[index],
		  ImageStack[ImageStackTop-3].pixels[index],
		  ImageStack[ImageStackTop-3].pixels[index],
		  ImageStack[ImageStackTop-3].pixels[index]);
      }
    }
    
    /* close the nodes .ipnode file */
    fclose(fp);
    if (VerboseFlag)
      printf("WriteCMISSPointsFile() : %d points inside the contour\n", n);
  }
  
  /* Z only data type */
  else {
    /* open the nodes .ipnode output file */
    sprintf(fname, "%s.ipdata", ParmPtrs[0]);
    if ((fp = fopen(fname, "w")) == NULL) {
      Abort = B_TRUE;
      sprintf(cbuf,
	      "WriteCMISSPointsFile():Could not open the output file \"%s\"",
	      fname);
      draw_mesg(cbuf);
      return;
    }
    
    /* print out the nodes .ipnode file header */
    fprintf(fp, "Geometry data file\n");
    
    /* loop to write the points */
    sprintf(cbuf, "WriteCMISSPointsFile() : Writing %d pts to data file...",
	    (height/decimation)*(width/decimation));
    draw_mesg(cbuf);
    for (j = 0, n = 0; j < height; j+=decimation) {
      for (i = 0; i < width; i+=decimation) {
	if (MaskPixels[j*width + i] > 0) {
	  fprintf(fp, "%6d %12f %12f %12f 1.0 1.0 1.0\n",
		  n++,
		  ((double) (j - cy))*MMPerPixel,
		  ((double) (i - cx))*MMPerPixel,
		  inpixels[j*width + i]);
	}
      }
    }
      
    /* close the nodes .ipnode file */
    fclose(fp);
    if (VerboseFlag)
      printf("WriteCMISSPointsFile() : %d points written\n", n);
  }
    
  /* transfer data to the output buffer */
  for (i = 0; i < height*width; i++)
    outpixels[i] = inpixels[i];
}


/********************** WriteCMISSNodesFile ****************************/
/*                                                                     */
/* Write the current nodes out as CMISS description files              */
/*                                                                     */
/* Parameters:                                                         */
/*     ParmPtrs[0] : File name mantissa                                */
/*                                                                     */
/***********************************************************************/
void WriteCMISSNodesFile(double *inpixels, double *outpixels,
			 int height, int width, void *ParmPtrs[])
{
  FILE *fp;
  int n, i;
  char fname[256];

  /* create the nodes .ipnode file name */
  sprintf(fname, "%s_nodes.ipnode", ParmPtrs[0]);
  
  /* open the nodes .ipnode output file */
  if ((fp = fopen(fname, "w")) == NULL) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "WriteCMISSNodeFile() : Could not open the output file \"%s\"",
	    fname);
    draw_mesg(cbuf);
    return;
  }
  
  /* print out the nodes .ipnode file header */
  fprintf(fp, "CMISS Version 1.21 ipnode File Version  2\n");
  fprintf(fp, "Heading:  Bilinear mesh nodes\n\n");   
  fprintf(fp, "$The number of nodes is [    1]:   %d\n", nnodes);
  fprintf(fp, "$Number of coordinates [2]: 2\n");
  fprintf(fp,
	  "$Do you want prompting for different versions of nj=1 [N]? N\n");
  fprintf(fp,
	  "$Do you want prompting for different versions of nj=2 [N]? N\n");
  fprintf(fp, "$The number of derivatives for coordinate 1 is [0]: 0\n");
  fprintf(fp, "$The number of derivatives for coordinate 2 is [0]: 0\n\n");
  
  /* loop to write the node description */
  for (n = 0; n < nnodes; n++) {
    fprintf(fp, "$Node number [   1]:    %d\n", n+1);
    fprintf(fp, "$The Xj(1) coordinate is [ 0.00000E+00]: %e\n",
	    MeshNodes[n].xw);
    fprintf(fp, "$The Xj(2) coordinate is [ 0.00000E+00]: %e\n\n",
	    MeshNodes[n].yw);
  }
  
  /* close the nodes .ipnode file */
  fclose(fp);
    
  /* transfer data */
  for (i = 0; i < height*width; i++)
    outpixels[i] = inpixels[i];
}


/********************** WriteCMISSForcesFile ***************************/
/*                                                                     */
/* Write the current forces as CMISS description files                 */
/*                                                                     */
/* Parameters:                                                         */
/*     ParmPtrs[0] : File name mantissa                                */
/*                                                                     */
/***********************************************************************/
static boolean_t WriteCMISSForcesFile(double *InitialForces,
				      double *InitialForcesScaledOnly,
				      double *forcesV, double *forceOffsetsV,
				      int cx, int cy, char *name)
{
  FILE *fp;
  double angle, mV;
  int n, i;
  double BoundaryForces_Scaled_Relative[NBOUNDARYNODES],
  BoundaryForces[NBOUNDARYNODES];
  char fname[256];
  
  /* create the forces .ipnode file name */
  sprintf(fname, "%s_forces.ipnode", name);
  
  /* open the forces .ipnode output file */
  if ((fp = fopen(fname, "w")) == NULL) {
    Abort = B_TRUE;
    sprintf(cbuf, "WriteCMISSForcesFile() : Could not open output file \"%s\"",
	    fname);
    draw_mesg(cbuf);
    return(B_FALSE);
  }
  
  /* print out the forces .ipnode file header */
  fprintf(fp, "CMISS Version 1.21 ipnode File Version  2\n");
  fprintf(fp, "Heading:  applied nodal forces\n\n");   
  fprintf(fp, "$The number of nodes is [    1]:   %d\n", nnodes);
  fprintf(fp, "$Number of coordinates [2]: 2\n");
  fprintf(fp,
	  "$Do you want prompting for different versions of nj=1 [N]? N\n");
  fprintf(fp,
	  "$Do you want prompting for different versions of nj=2 [N]? N\n");
  fprintf(fp, "$The number of derivatives for coordinate 1 is [0]: 0\n");
  fprintf(fp, "$The number of derivatives for coordinate 2 is [0]: 0\n\n");
  
  /* loop to write the node description */
  for (n = 0; n < nnodes; n++) {
    /* look to see if this node index is in the HookNodes array */
    for (i = 0; (i < NBOUNDARYNODES) && (n != BoundaryNodes[i]); i++);
    
    /* if this is a HookNode, write out the forces */
    if (i < NBOUNDARYNODES) {
      angle = atan2(MeshNodes[n].yw, MeshNodes[n].xw);
      mV = (forcesV[i] - forceOffsetsV[i])*1000.0 + NoForceOffsets_mV[i];
      BoundaryForces_Scaled_Relative[i] =
	(mV - NoForceOffsets_mV[i]) * mV_to_mN_a1[i]
	  - InitialForcesScaledOnly[i];
      BoundaryForces[i] =
	(mV_to_mN_a0[i] + mV*(mV_to_mN_a1[i] + mV*mV_to_mN_a2[i]))
	  - InitialForces[i];
      fprintf(fp, "$Node number [   1]:    %d\n", n+1);
      fprintf(fp, "$The Xj(1) coordinate is [ 0.00000E+00]: %e\n",
	      BoundaryForces[i] * cos(angle));
      fprintf(fp, "$The Xj(2) coordinate is [ 0.00000E+00]: %e\n\n",
	      BoundaryForces[i] * sin(angle));
    }
    /* else, its an internal node, so no force applied */
    else {
      fprintf(fp, "$Node number [   1]:    %d\n", n+1);
      fprintf(fp, "$The Xj(1) coordinate is [ 0.00000E+00]: %e\n", 0.0);
      fprintf(fp, "$The Xj(2) coordinate is [ 0.00000E+00]: %e\n\n", 0.0);
    }
  }
  
  /* close the forces .ipnode file */
  fclose(fp);

  /* write out the forces file in Matlab format */
  sprintf(fname, "%s_forces_mN.mat", name);
  if (WriteArrayToMatlabFile(16, 1, "forces", BoundaryForces,
			     fname) == B_FALSE) {
    Abort = B_TRUE;
    sprintf(cbuf, "WriteCMISSForcesFile() : Could not write to file \"%s\"",
	    fname);
    draw_mesg(cbuf);
    return(B_FALSE);
  }

  /* write out the "scaled relative" forces file in Matlab format */
  sprintf(fname, "%s_scaled_relative_forces_mN.mat", name);
  if (WriteArrayToMatlabFile(16, 1, "forces_scaled",
			     BoundaryForces_Scaled_Relative,
			     fname) == B_FALSE) {
    Abort = B_TRUE;
    sprintf(cbuf, "WriteCMISSForcesFile() : Could not write to file \"%s\"",
	    fname);
    draw_mesg(cbuf);
    return(B_FALSE);
  }

  /* return success */
  return(B_TRUE);
}


/***************************  LoadMesh   *******************************/
/*                                                                     */
/* Load a FE mesh definition from a file                               */
/*                                                                     */
/***********************************************************************/
void LoadMesh(double *inpixels, double *outpixels,
	      int height, int width, void *ParmPtrs[])
{
  MATFile *DataFp;
  boolean_t GenMesh;
  double *nodesx, *nodesy, *xforces, *yforces, *ImDataP, *fpx, *fpy,
  *LocalPS, *buf, *dcx, *dcy, *nodesx_mm, *nodesy_mm;
  int i, n, nmax, xc, yc, flag, lwidth, lheight, LoadElementTemplate;
  boolean_t SpeckleFEMesh, Cx_Cy_Loaded, NodesMM_Loaded;

  /* local inits */
  ImDataP = NULL;

  /* load parms */
  n = *((int *) ParmPtrs[1]);
  CurrentMeshStep = n;
  flag = *((int *) ParmPtrs[2]);
  LoadElementTemplate = *((int *) ParmPtrs[3]);
  SpeckleFEMesh = B_TRUE;

  /* allocate storage for the mesh pixels if required */
  if ((FEMeshPixels = (short *) XvgMM_Alloc((void *) FEMeshPixels,
					    height*width*sizeof(short)))
      == NULL) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort : can't allocate for mesh pixels in LoadMesh()");
    draw_mesg(cbuf);
    return;
  }
  
  /* get the nodal coordinates data */
  if ((DataFp=matOpen(ParmPtrs[0], "r")) == NULL) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort : Can't open file %s in LoadMesh()", ParmPtrs[0]);
    draw_mesg(cbuf);
    return;
  }
  nodesx=NULL;
  nodesy=NULL;
  xforces=NULL;
  yforces=NULL;
  ImDataP=NULL;
  fpx=NULL;
  fpy=NULL;
  LocalPS=NULL;
  buf=NULL;
  if (matGetFull(DataFp, "nodesx", &MeshStatesN, &nmax, &nodesx, &ImDataP)
      != 0) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort : Can't load \"nodesx\" in LoadMesh()", ParmPtrs[0]);
    ErrorMsg(cbuf);
    return;
  }
  if (matGetFull(DataFp, "nodesy", &MeshStatesN, &nmax, &nodesy, &ImDataP)
      != 0) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort : Can't load \"nodesy\" in LoadMesh()", ParmPtrs[0]);
    ErrorMsg(cbuf);
    return;
  }

  /* load fixed point coordinates */
  if (matGetFull(DataFp, "fpx", &xc, &yc, &fpx, &ImDataP) != 0) 
    SpeckleFEMesh = B_FALSE;
  if (matGetFull(DataFp, "fpy", &xc, &yc, &fpy, &ImDataP) != 0)
    SpeckleFEMesh = B_FALSE;

  /* load PixelSpacing value from the file */
  if (matGetFull(DataFp, "PixelSpacing", &xc, &yc, &LocalPS, &ImDataP) != 0)
    SpeckleFEMesh = B_FALSE;

  /* load the x force information */
  if (matGetFull(DataFp, "xforces", &xc, &yc, &xforces, &ImDataP) != 0)
    SpeckleFEMesh = B_FALSE;

  /* load the y force information */
  if (matGetFull(DataFp, "yforces", &xc, &yc, &yforces, &ImDataP) != 0)
    SpeckleFEMesh = B_FALSE;

  /* load the hooknode information */
  if (matGetFull(DataFp, "HookNodes", &xc, &yc, &buf, &ImDataP) != 0)
    SpeckleFEMesh = B_FALSE;

  /* load center point coordinates if possible */
  if ((matGetFull(DataFp, "cx", &xc, &yc, &dcx, &ImDataP) == 0) &&
      (matGetFull(DataFp, "cy", &xc, &yc, &dcy, &ImDataP) == 0)) {
    Cx_Cy_Loaded = B_TRUE;
    if (VerboseFlag)
      printf("LoadMesh() : Loaded cx and cy...\n");
  }
  else
    Cx_Cy_Loaded = B_FALSE;

  /* load center point coordinates if possible */
  if ((matGetFull(DataFp, "nodesx_mm", &xc, &yc, &nodesx_mm, &ImDataP) == 0) &&
      (matGetFull(DataFp, "nodesy_mm", &xc, &yc, &nodesy_mm, &ImDataP) == 0)) {
    NodesMM_Loaded = B_TRUE;
    if (VerboseFlag)
      printf("LoadMesh() : Loaded nodesx_mm and nodesy_mm...\n");
  }
  else
    NodesMM_Loaded = B_FALSE;

  /* load the element index template if there is one and it's required */
  if (LoadElementTemplate) {
    if (matGetFull(DataFp, "ElementTemplate", &lwidth, &lheight,
		   &DoubleBuffer, &ImDataP) == 0) {
      if ((lwidth != width) || (lheight != height)) {
	Abort = B_TRUE;
	sprintf(cbuf, "Abort : \"Element template\" size mismatch (%d,%d) in LoadMesh()",
		lwidth, lheight);
	ErrorMsg(cbuf);
	return;
      }
      else {
	for (i = 0; i < height*width; i++)
	  FEMeshPixels[i] = DoubleBuffer[i];
	GenMesh = B_FALSE;
      }
    }
    else {
      GenMesh = B_TRUE;
    }
  }
  else {
    GenMesh = B_FALSE;
  }
  
  /* close the input file */
  matClose(DataFp);

  /* look for wrong mesh definition */
  if (nmax != nnodes) {
    Abort = B_TRUE;
    sprintf(cbuf,"Abort : Wrong mesh defintion (%d vs %d nodes) in LoadMesh(). Consider the \"_FE_MODEL_HOLES\" flag in \"feIPOPs.c\".",
	    nmax, nnodes);
    ErrorMsg(cbuf);
    return;
  }
  
  /* Load "PixelSpacing" with file data if required */
  if (flag != 0)
    PixelSpacing = *LocalPS;

  /* check that requested index does not exceed data file length */
  if (n >= MeshStatesN) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort : Index (%d) exceeds max (%d) in LoadMesh()",
	    n, MeshStatesN);
    ErrorMsg(cbuf);
    return;
  }

  /* load the requested nodal coordinates */
  if (SpeckleFEMesh) {
    for (i = 0; i < nnodes; i++) {
      /* nodal coordinates in pixels */
      MeshNodes[i].xp = (nodesx[i*MeshStatesN + n]
			 - nodesx[MeshStatesN])/(*LocalPS) + *fpx; 
      MeshNodes[i].yp = (nodesy[i*MeshStatesN + n]
			 - nodesy[MeshStatesN])/(*LocalPS) + *fpy;
      
      /* initialize the nodal coordinates in meter units */
      if (flag != 0) {
	MeshNodes[i].xw = (nodesx[i*MeshStatesN + n] + (PixelSpacing * *fpx))
	  - nodesx[MeshStatesN];
	MeshNodes[i].yw = (nodesy[i*MeshStatesN + n] + (PixelSpacing * *fpy))
	  - nodesy[MeshStatesN];
      }
      else {
	MeshNodes[i].xw = MeshNodes[i].xp * PixelSpacing;
	MeshNodes[i].yw = MeshNodes[i].yp * PixelSpacing;
      }
    }
    
#ifdef FE_COMPUTE_DISPLACED_NODES_SPECKLE
    /* load force and hooknode information into local storage */
    for (i = 0; i < NGALVOS; i++) {
      forces_x[i] = xforces[i*MeshStatesN + n];
      forces_y[i] = yforces[i*MeshStatesN + n];
      HookNodes[i] = buf[i];
    }
#endif
    
    /* show outer node cordinates */
    if (VerboseFlag) {
      printf("LoadMesh() => Number of mesh states: %d\n", MeshStatesN);
      printf("LoadMesh() => Fixed point pixel coordinates are (%d,%d)\n",
	     (int) (*fpx), (int) (*fpy));
      printf("LoadMesh()=> Fixed point world coordinates are (%.9fm, %.9fm)\n",
	     MeshNodes[1].xw, MeshNodes[1].yw);
      printf("LoadMesh() => World coordinates of (0,0) are (%.9fm, %.9fm)\n",
	     MeshNodes[1].xw - (*fpx)*PixelSpacing, 
	     MeshNodes[1].yw - (*fpy)*PixelSpacing);
      printf("LoadMesh() => Pixel spacing : %.9fm\n", PixelSpacing);
    }
  }

  /* load correlation data - type FE mesh */
  else {
    sprintf(cbuf, "Nodes coordinates loaded only from \'%s\'", ParmPtrs[0]);
    InfoMsg(cbuf);
    /* convert from mm to pixels */
    if ((flag != 0) && (Cx_Cy_Loaded) && (NodesMM_Loaded)) {
      for (i = 0; i < nnodes; i++) {
	MeshNodes[i].xp = (nodesx_mm[i*MeshStatesN + n] / PixelSpacing) + *dcx;
	MeshNodes[i].yp = (nodesy_mm[i*MeshStatesN + n] / PixelSpacing) + *dcy;
      }
      if (VerboseFlag)
	printf("LoadMesh() : Converted from mm to pix (ps:%f, cx:%f, cy:%f\n",
	       PixelSpacing, *dcx, *dcy);
    }
    /* else load pixel infomration directly */
    else {
      for (i = 0; i < nnodes; i++) {
	MeshNodes[i].xp = nodesx[i*MeshStatesN + n];
	MeshNodes[i].yp = nodesy[i*MeshStatesN + n];
      }
    }
  }
    
  /* compute element centre coordinates and flag the initialized mesh */
  if (GenMesh)
    GenerateMesh(B_TRUE);
  FEMeshDefined = B_TRUE;
  LoadOverlayFunction(FEMeshOverlay);
  LoadOverlayFunction(DrawFEMarkers);

  /* free storage */
  XvgMM_Free(nodesx);
  XvgMM_Free(nodesy);
  XvgMM_Free(xforces);
  XvgMM_Free(yforces);
  XvgMM_Free(ImDataP);
  XvgMM_Free(fpx);
  XvgMM_Free(fpy);
  XvgMM_Free(LocalPS);
  XvgMM_Free(buf);

  /* copy the data to the output buffer */
  for (i = 0; i < height*width; i++)
    outpixels[i] = inpixels[i];
  return;
}

/************************** ComputeStrain ******************************/
/*                                                                     */
/* Compute the Lagrangian strain for a sample                          */
/*                                                                     */
/***********************************************************************/
static void ComputeStrain(FESample *s, boolean_t *ColdStart)
{
  static double x1, x2, x3, x4, y1, y2, y3, y4;
  static int element;
  static MeshElement e;
  double dxdX, dxdY, dydX, dydY, dxdxi1, dxdxi2, dydxi1, dydxi2,
  dxi1dX, dxi2dX, dxi1dY, dxi2dY;

  /* load the element fixed and displaced nodal coordinates, if required */
  if ((s->element != element) || (*ColdStart)) {
    element = s->element;
    e = MeshElements[element];
    x1 = MeshNodes[e.nodes[0]].xp;
    y1 = MeshNodes[e.nodes[0]].yp;
    x2 = MeshNodes[e.nodes[1]].xp;
    y2 = MeshNodes[e.nodes[1]].yp;
    x3 = MeshNodes[e.nodes[2]].xp;
    y3 = MeshNodes[e.nodes[2]].yp;
    if (e.type == R_TYPE) {
      x4 = MeshNodes[e.nodes[3]].xp;
      y4 = MeshNodes[e.nodes[3]].yp;
    }
    *ColdStart = B_FALSE;
  }
  
  /* compute partial derivatives w/r to the normalized coords */
  if (e.type == R_TYPE) {
    dxdxi1 = s->v * (x1 - x2 + x3 - x4) - x1 + x4;
    dxdxi2 = s->u * (x1 - x2 + x3 - x4) - x1 + x2;
    dydxi1 = s->v * (y1 - y2 + y3 - y4) - y1 + y4;
    dydxi2 = s->u * (y1 - y2 + y3 - y4) - y1 + y2;
  }
  else {
    dxdxi1 = x1 - x3;
    dxdxi2 = x2 - x3;
    dydxi1 = y1 - y3;
    dydxi2 = y2 - y3;
  }
    
  /* compute the Jacobian inverse */
  ComputeJacobianInverse(s, MeshNodesFixed,
			 &dxi1dX, &dxi2dX, &dxi1dY, &dxi2dY);
  
  /* compute the partial dervs of the displaced nodes w/r to the fixed */
  dxdX = dxdxi1*dxi1dX + dxdxi2*dxi2dX;
  dxdY = dxdxi1*dxi1dY + dxdxi2*dxi2dY;
  dydX = dydxi1*dxi1dX + dydxi2*dxi2dX;
  dydY = dydxi1*dxi1dY + dydxi2*dxi2dY;
  
  /* compute the strain components */
  s->Exx = (dxdX*dxdX + dydX*dydX - 1.0)*0.5;
  s->Exy = (dxdX*dxdY + dydX*dydY)*0.5;
  s->Eyy = (dxdY*dxdY + dydY*dydY - 1.0)*0.5;
}

/************************ Capture overlay ******************************/
/*                                                                     */
/* Convert the overlay pixels to data and junk the underlying image    */
/*                                                                     */
/***********************************************************************/
void CaptureOverlay(double *inpixels, double *outpixels,
		    int height, int width, void *ParmPtrs[])
{
  XImage *image;
  int i, j, k, blk;

  /* check that the processed image window is popped up */
  if (IsTrue(UxGetSet(ProcessedImageTBG)) == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort : Processed window not popped up in CaptureOverlay(()");
    return;
  }
  
  /* blank out the image data */
  for (k = 0; k < width*height; k++)
    ProcessedPixels[k] = 0;
  RealtoX(ProcessedPixels, ProcessedPixelsX, ImgHeight, ImgWidth,
	  1.0, 0.0, MaxScaleVal, MinScaleVal, ZoomFactor);
  RedrawProcDrawingArea();
  XRaiseWindow(UxDisplay, XtWindow(ProcessedImageShell));
  XSync(UxDisplay, B_FALSE);
  
  /* grab the image and covert overlay pixels to black and others to white */
  blk = CurrentCells[0].pixel;
  image = XGetImage(UxDisplay,
		    XtWindow(ProcessedImageDrawingArea), 0, 0,
		    ImgWidth, ImgHeight, AllPlanes, ZPixmap);
  for (j=0, k=0; j < height; j++)
    for (i=0; i < width; i++, k++) {
      if ((image->data)[j*(image->bytes_per_line) + i] == blk)
	outpixels[k] = 1;
      else
	outpixels[k] = 0;
    }
  XDestroyImage(image);
  
  /* remove overlays */
  NOverlayFunctions = 0;
}


/************************ CompDispField  *******************************/
/*                                                                     */
/* Compute the incremental displacement field from a, x,y data set pair*/
/*                                                                     */
/***********************************************************************/
void CompDispField(double *inpixels, double *outpixels,
		   int height, int width, void *ParmPtrs[])
{
  FILE *fp;
  double scale, dx, dy, dfx, dfy;
  int spacing, i, j, k, fx, fy;

  /* fetch decimation parameter */
  spacing = *((int *) ParmPtrs[0]);
  scale = *((double *) ParmPtrs[1]);
  fx = *((int *) ParmPtrs[2]);
  fy = *((int *) ParmPtrs[3]);

  /* check that a mask image is defined */
  if (MaskPixels == NULL) {
    Abort = B_TRUE;
    draw_mesg("Abort : undefined mask in CompDispField()");
    return;
  }

  /* check that stack contains the correct number of images */
  /* outpixels                           : unwrapped model X data  */
  /* ImageStack[ImageStackTop -1].pixels : unwrapped model Y data  */
  if (ImageStackTop < 1) {
    Abort = B_TRUE;
    draw_mesg("Abort : 1 imaged required on the stack in CompDispField()");
    return;
  }

  /* count the number of required field elements */
  for (j = 0, NDispFieldElements = 0; j < height; j+=spacing)
    for (i = 0; i < width; i+=spacing)
      if (MaskPixels[j*width + i] != 0)
	NDispFieldElements++;
  
  /* check that the mask contained valid data */
  if (NDispFieldElements == 0) {
    Abort = B_TRUE;
    draw_mesg("Abort : null mask data in CompDispField()");
    return;
  }

  /* allocate storage for the displacement field elements */
  if ((DispFieldElements = (DISP_FIELD_ELEMENT *)
       XvgMM_Alloc((void *) DispFieldElements,
		   NDispFieldElements*sizeof(DISP_FIELD_ELEMENT))) == NULL) {
    Abort = B_TRUE;
    draw_mesg("Abort : can't allocate for samples in CompDispField()");
    return;
  }

  /* loop through image to create lines */
  dfx = inpixels[fy*width + fx];
  dfy = (ImageStack[ImageStackTop-1].pixels)[fy*width + fx];
  for (j = 0, k = 0; j < height; j+=spacing)
    for (i = 0; i < width; i+=spacing)
      if (MaskPixels[j*width + i] != 0) {
	dx = (inpixels[j*width + i] - dfx)*scale;
	dy = ((ImageStack[ImageStackTop-1].pixels)[j*width + i] - dfy)*scale;
	DispFieldElements[k].xmin = ((double) i) - dx;
	DispFieldElements[k].xmax = ((double) i) + dx;
	DispFieldElements[k].ymin = ((double) j) - dy;
	DispFieldElements[k].ymax = ((double) j) + dy;
	k++;
      }

  /* write out principal strain coordinates */
  if ((fp = fopen("DispField.dat", "w")) == NULL) {
    Abort = B_TRUE;
    sprintf(cbuf, "Could not open the output file \"%s\" in DispStrainField()",
	    "DispField.dat");
    draw_mesg(cbuf);
    return;
  }
  for (i = 0; i < k; i++)
    fprintf(fp, "%d %d\n%d %d\n",
	    DispFieldElements[i].xmin, DispFieldElements[i].ymin,
	    DispFieldElements[i].xmax, DispFieldElements[i].ymax);
  fclose(fp);
  
  /* transfer data */
  for (k = 0; k < width*height; k++)
    outpixels[k] = inpixels[k];

  /* display the field overlay function */
  LoadOverlayFunction(DispFieldOverlay);
}

/************************ CompStrainField  *****************************/
/*                                                                     */
/* Compute the infinitesimal strain field from a two phase images      */
/*                                                                     */
/***********************************************************************/
void CompStrainField(double *inpixels, double *outpixels,
		     int height, int width, void *ParmPtrs[])
{
  FILE *fp;
  double scale, dx, dy, Exx, Exy, Eyy, alpha, Emax, Emin, CosA, SinA;
  int spacing, i, j, k, step;

  /* fetch decimation parameter */
  spacing = *((int *) ParmPtrs[0]);
  scale = *((double *) ParmPtrs[1]);
  step = *((int *) ParmPtrs[2]);

  /* check that a mask image is defined */
  if (MaskPixels == NULL) {
    Abort = B_TRUE;
    draw_mesg("Abort : undefined mask in CompStrainField()");
    return;
  }

  /* check that stack contains the correct number of images */
  /* outpixels                           : unwrapped model X data  */
  /* ImageStack[ImageStackTop -1].pixels : unwrapped model Y data  */
  if (ImageStackTop < 1) {
    Abort = B_TRUE;
    draw_mesg("Abort : 1 imaged required on the stack in CompStrainField()");
    return;
  }

  /* count the number of required field elements */
  for (j = 0, NDispFieldElements = 0; j < height; j+=spacing)
    for (i = 0; i < width; i+=spacing)
      if ((j > step) && (i > step) && (j < height-step) && (i < width - step)
	  && (MaskPixels[j*width + (i-step)] != 0)
	  && (MaskPixels[j*width + (i+step)] != 0)
	  && (MaskPixels[(j-step)*width + i] != 0)
	  && (MaskPixels[(j+step)*width + i] != 0)) {
	NDispFieldElements+=2;
      }
  
  /* check that the mask contained valid data */
  if (NDispFieldElements == 0) {
    Abort = B_TRUE;
    draw_mesg("Abort : null mask data in CompStrainField()");
    return;
  }

  /* allocate storage for the displacement field elements */
  if ((DispFieldElements = (DISP_FIELD_ELEMENT *)
       XvgMM_Alloc((void *) DispFieldElements,
		   NDispFieldElements*sizeof(DISP_FIELD_ELEMENT))) == NULL) {
    Abort = B_TRUE;
    draw_mesg("Abort : can't allocate for samples in CompStrainField()");
    return;
  }

  /* loop through image to create lines */
  for (j = 0, k = 0; j < height; j++)
    for (i = 0; i < width; i++)
      if ((j > step) && (i > step) && (j < height-step) && (i < width - step)
	  && (MaskPixels[j*width + (i-step)] != 0)
	  && (MaskPixels[j*width + (i+step)] != 0)
	  && (MaskPixels[(j-step)*width + i] != 0)
	  && (MaskPixels[(j+step)*width + i] != 0)
	  && (k < NDispFieldElements)) {
	/* calculate strain components */
	Exx = ((inpixels[j*width + (i+step)] - inpixels[j*width + (i-step)])
	       *METERS_PER_RADIAN)
	  /(((double) (2*step)) * 0.000034316);
	Eyy = (((ImageStack[ImageStackTop-1].pixels)[(j+step)*width + i]
		-(ImageStack[ImageStackTop-1].pixels)[(j-step)*width + i])
	       *METERS_PER_RADIAN)
	  /(((double) (2*step)) * 0.000034316);
	Exy = (((inpixels[(j+step)*width + i] - inpixels[(j-step)*width + i])
		+ ((ImageStack[ImageStackTop-1].pixels)[j*width + (i+step)]
		   - (ImageStack[ImageStackTop-1].pixels)[j*width + (i-step)]))
	       *METERS_PER_RADIAN)
	  /(((double) (4*step)) * 0.000034316);
	/* calculate principal strains and rotation angle */
	Emax = 0.5*(Exx + Eyy) + sqrt(0.25*(Exx - Eyy)*(Exx - Eyy) + Exy*Exy);
	Emin = 0.5*(Exx + Eyy) - sqrt(0.25*(Exx - Eyy)*(Exx - Eyy) + Exy*Exy);

	/* write % difference between principal strains */
	if (fabs(Emax) > 0){
	  outpixels[j*width + i] = 100.0*(fabs(Emax) - fabs(Emin))/fabs(Emax);
	  if (outpixels[j*width + i] < 0)
	    outpixels[j*width + i] = 0;
	}
	else
	  outpixels[j*width + i] = -1;

	/* load principal strain line coordinates if required */
	if ((fmod((double) j, (double) spacing) == 0.0)
	    && (fmod((double) i, (double) spacing) == 0.0)) {
	  /* calculate principal strain rotation angle */
	  alpha = 0.5*atan2(2.0*Exy, Exx - Eyy);
	  CosA = cos(alpha);
	  SinA = sin(alpha);
	  
	  /* calculate maximum principal strain line */
	  dx = scale*fabs(Emax)/2.0*CosA;
	  dy = scale*fabs(Emax)/2.0*SinA;
	  DispFieldElements[k].xmin = ((double) i) - dx;
	  DispFieldElements[k].xmax = ((double) i) + dx;
	  DispFieldElements[k].ymin = ((double) j) - dy;
	  DispFieldElements[k].ymax = ((double) j) + dy;
	  k++;
	  
	  /* calculate maximum principal strain line */
	  dx = scale*fabs(Emin)/2.0*(-SinA);
	  dy = scale*fabs(Emin)/2.0*CosA;
	  DispFieldElements[k].xmin = ((double) i) - dx;
	  DispFieldElements[k].xmax = ((double) i) + dx;
	  DispFieldElements[k].ymin = ((double) j) - dy;
	  DispFieldElements[k].ymax = ((double) j) + dy;
	  k++;
	}
      }
      else {
	outpixels[j*width + i] = -1;
      }

  /* write out principal strain coordinates */
  if ((fp = fopen(ParmPtrs[3], "w")) == NULL) {
    Abort = B_TRUE;
    sprintf(cbuf, "Could not open the output file \"%s\" in CompStrainField()",
	    ParmPtrs[3]);
    draw_mesg(cbuf);
    return;
  }
  for (i = 0; i < k; i++)
    fprintf(fp, "%d %d\n%d %d\n",
	    DispFieldElements[i].xmin, DispFieldElements[i].ymin,
	    DispFieldElements[i].xmax, DispFieldElements[i].ymax);
  fclose(fp);
  
  /* display the field overlay function */
  LoadOverlayFunction(DispFieldOverlay);
}

/***************************  FEStrain   *******************************/
/*                                                                     */
/* Compute the Lagrangian strain                                       */
/*                                                                     */
/***********************************************************************/
void FEStrain(double *inpixels, double *outpixels,
	      int height, int width, void *ParmPtrs[])
{
  static double GaussPointsX[NGAUSSPOINTS] = {0.21132, 0.78868,
						0.21132, 0.78868};
  static double GaussPointsY[NGAUSSPOINTS] = {0.21132, 0.21132,
						0.78868, 0.78868};
  static int One=1, Zero=0;
  FESample s;
  boolean_t GaussPtStrainOnly, dummy;
  double T1, T2, emax, emin, Scale, theta, CosTheta, SinTheta, xh, yh;
  int i, j, x, y, k, n, *parms[4];

  /* load parms */
  n = *((int *) ParmPtrs[1]);
  Scale = *((double *) ParmPtrs[2]);
  if (*((int *) ParmPtrs[3]) == 0)
    GaussPtStrainOnly = B_FALSE;
  else
    GaussPtStrainOnly = B_TRUE;

  /* allocate storage for the field samples if required */
  if ((FieldSamples = (FESample *) XvgMM_Alloc((void *) FieldSamples,
					       height*width
					       *sizeof(FESample))) == NULL) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort : can't allocate for field samples in FEStrain()");
    draw_mesg(cbuf);
    return;
  }

  /* allocate storage for the mesh pixels if required */
  if ((FEMeshPixels = (short *) XvgMM_Alloc((void *) FEMeshPixels,
					    height*width*sizeof(short)))
      == NULL) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort : can't allocate for mesh pixels in FEStrain()");
    draw_mesg(cbuf);
    return;
  }
  
  /* allocate storage for principal strain lines if required */
  if ((StrainElements = (STRAIN_ELEMENT *)
       XvgMM_Alloc((void *) StrainElements,
		   nelements * NGAUSSPOINTS * sizeof(STRAIN_ELEMENT))) == NULL) {
    Abort = B_TRUE;
    draw_mesg("Can't allocate in FESTrain()");
    return;
  }
  NStrainElements = 0;
  
  /* load the initial mesh definition */
  draw_mesg("FEStrain() : Loading initial node coordinates...");
  parms[0] = ParmPtrs[0];
  parms[1] = &Zero;
  parms[2] = &One;
  parms[3] = &One;
  LoadMesh(inpixels, outpixels, height, width, (void **) parms);
  if (Abort)
    return;

  /* save undisplaced material coordinates */
  for (i = 0; i < nnodes; i++)
    MeshNodesFixed[i] = MeshNodes[i];

  /* load the displaced mesh definition */
  draw_mesg("FEStrain() : Loading displaced node coordinates...");
  parms[1] = &n;
  parms[3] = &Zero;
  LoadMesh(inpixels, outpixels, height, width, (void **) parms);
  if (Abort)
    return;
  
  /* loop to compute strain at each sample within the mesh */
  if (GaussPtStrainOnly == B_FALSE) {
    /* compute element coordinates */
    draw_mesg("FEStrain() : Computing element coordinates...");
    ComputeAllPixelsXiCoords(B_TRUE, B_TRUE, BILINEAR_FIELD_MODEL);
    if (Abort)
      return;

    /* compute strain */
    draw_mesg("FEStrain() : Computing strain...");
    for (i = 0, k = 0, dummy = B_TRUE; (i < height) && (Abort == B_FALSE); i++)
      for (j = 0; (j < width) && (Abort == B_FALSE); j++, k++)
	if (FEMeshPixels[k] >= 0) {
	  /* compute the strain tensor and load components in local vars */
	  ComputeStrain(&(FieldSamples[k]), &dummy);
	  outpixels[k] = (FieldSamples[k].Exx + FieldSamples[k].Eyy)*0.5
	    + sqrt((FieldSamples[k].Exx - FieldSamples[k].Eyy)
		   *(FieldSamples[k].Exx
		     - FieldSamples[k].Eyy)*0.25
		   + FieldSamples[k].Exy
		   * FieldSamples[k].Exy);
	}
	else
	  outpixels[k] = 0;
#ifdef DEBUG
    printf("FEStrain() : Computed strain tensor at each point\n");
#endif
  }
  
  /* loop to compute the principal strains at the Gauss points */
  draw_mesg("FEStrain() : Computing principal strains at the Gauss pts");
  for (i = 0, dummy = B_TRUE; i < nelements; i++) {
    for (j = 0; j < (MeshElements[i].type == R_TYPE ? 4 : 1); j++) {
      /* compute the x,y pixel coordinates and the field sample index */
      s.element = i;
      if (MeshElements[i].type == R_TYPE) {
	s.u = GaussPointsX[j];
	s.v = GaussPointsY[j];
      }
      else {
	s.Phi[0] = s.Phi[1] = s.Phi[2] = 0.33;
      }
      ComputePixelMaterialCoords(s, B_FALSE, &xh, &yh);
      x = xh; y = yh;
      k = ImgWidth*y + x;

      /* compute strain tensor if required */
      if (GaussPtStrainOnly) {
	FieldSamples[k] = s;
	ComputeStrain(&(FieldSamples[k]), &dummy);
      }

      /* compute the principal strains */
      T1 = (FieldSamples[k].Exx + FieldSamples[k].Eyy)*0.5;
      T2 = sqrt((FieldSamples[k].Exx - FieldSamples[k].Eyy)
		*(FieldSamples[k].Exx - FieldSamples[k].Eyy)*0.25
		+ FieldSamples[k].Exy * FieldSamples[k].Exy);
      emax = T1 + T2;
      emin = T1 - T2;
      theta = atan2(2.0*FieldSamples[k].Exy,
		    FieldSamples[k].Exx-FieldSamples[k].Eyy)/2.0;
      CosTheta = cos(theta);
      SinTheta = sin(theta);
      
      /* load strain components into the global array */
      StrainElements[NStrainElements].x = x;
      StrainElements[NStrainElements].y = y;
      StrainElements[NStrainElements].ex = FieldSamples[k].Exx;
      StrainElements[NStrainElements].ey = FieldSamples[k].Eyy;
      StrainElements[NStrainElements].exy = FieldSamples[k].Exy;
      StrainElements[NStrainElements].emax = emax;
      StrainElements[NStrainElements].emin = emin;
      StrainElements[NStrainElements].theta = theta;
      
      /* compute and load strain overlay info */
/*
      StrainElements[NStrainElements]..xwax =
	Scale * fabs(emax)/2.0 * CosTheta;
      StrainElements[NStrainElements]..ywax =
	Scale * fabs(emax)/2.0 * SinTheta;
      StrainElements[NStrainElements]..xwin =
	Scale * fabs(emin)/2.0 * -SinTheta;
      StrainElements[NStrainElements]..ywin =
	Scale * fabs(emin)/2.0 * CosTheta;
      StrainElements[NStrainElements].d.xwax =
	(Scale * fabs(emax)/2.0 + 1.0) * CosTheta;
      StrainElements[NStrainElements].d.ywax =
	(Scale * fabs(emax)/2.0 + 1.0) * SinTheta;
      StrainElements[NStrainElements].d.xwin =
	(Scale * fabs(emin)/2.0 + 1.0) * -SinTheta;
      StrainElements[NStrainElements].d.ywin =
	(Scale * fabs(emin)/2.0 + 1.0) * CosTheta;
*/
      NStrainElements++;
    }
  }
#ifdef DEBUG
  printf("FEStrain() : Computed principal strains at the Gauss points\n");
#endif

  /* if principal strains only, copy input data to output buffer */
  if (GaussPtStrainOnly)
    for (i = 0; i < height*width; i++)
      outpixels[i] = inpixels[i];
  
  /* display the strain overlay function */
  LoadOverlayFunction(StrainOverlay);
}


/*********************** ComputeMarkerPaths ****************************/
/*                                                                     */
/* Compute the interpolated marker trajectory                          */
/*                                                                     */
/***********************************************************************/
void ComputeMarkerPath(double *inpixels, double *outpixels,
		       int height, int width, void *ParmPtrs[])
{
  MATFile *DataFp;
  Matrix *a;
  double *nodesx, *nodesy, *ImDataP, *fpx, *fpy, *LocalPS, *xp, *yp;
  boolean_t Dummy;
  int i, n, nmax, xc, yc, x0, y0, node;

  /* load parms */
  x0 = *((int *) ParmPtrs[0]);
  y0 = *((int *) ParmPtrs[1]);

  /* get the nodal coordinates data */
  if ((DataFp=matOpen(ParmPtrs[2], "rb")) == NULL) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort : Can't open input file %s in ComputeMarkerPath()",
	    ParmPtrs[2]);
    draw_mesg(cbuf);
    return;
  }
  nodesx=NULL;
  nodesy=NULL;
  ImDataP=NULL;
  fpx=NULL;
  fpy=NULL;
  LocalPS=NULL;
  if (matGetFull(DataFp, "nodesx", &MeshStatesN, &nmax, &nodesx, &ImDataP) != 0) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort : Can't load \"nodesx\" in ComputeMarkerPath()");
    ErrorMsg(cbuf);
    return;
  }
  if (matGetFull(DataFp, "nodesy", &MeshStatesN, &nmax, &nodesy, &ImDataP) != 0) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort : Can't load \"nodesy\" in ComputeMarkerPath()");
    ErrorMsg(cbuf);
    return;
  }
  if (matGetFull(DataFp, "fpx", &xc, &yc, &fpx, &ImDataP) != 0) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort : Can't load \"fpx\" in ComputeMarkerPath()");
    ErrorMsg(cbuf);
    return;
  }
  if (matGetFull(DataFp, "fpy", &xc, &yc, &fpy, &ImDataP) != 0) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort : Can't load \"fpy\" in ComputeMarkerPath()");
    ErrorMsg(cbuf);
    return;
  }
  /* load PixelSpacing value from the file */
  if (matGetFull(DataFp, "PixelSpacing", &xc, &yc, &LocalPS, &ImDataP) != 0) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort: Can't load \"PixelSpacing\" in ComputeMarkerPath()");
    ErrorMsg(cbuf);
    return;
  }
  matClose(DataFp);

  /* load for wrong mesh definition */
  if (nmax != nnodes) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort : Wrong mesh defintion (%d vs %d nodes) in ComputeMarkerPath()",
	    nmax, nnodes);
    ErrorMsg(cbuf);
    return;
  }
  
  /* Load "PixelSpacing" with file data */
  PixelSpacing = *LocalPS;

  /* load mesh node coordinates for reference state */
  for (i = 0; i < nnodes; i++) {
    /* nodal coordinates in pixels */
    MeshNodes[i].xp = (nodesx[i*MeshStatesN] - nodesx[MeshStatesN])/(*LocalPS) + *fpx; 
    MeshNodes[i].yp = (nodesy[i*MeshStatesN] - nodesy[MeshStatesN])/(*LocalPS) + *fpy;
    
    /* initialize the nodal coordinates in meter units */
    MeshNodes[i].xw = (nodesx[i*MeshStatesN] + (PixelSpacing * *fpx))
      - nodesx[MeshStatesN];
    MeshNodes[i].yw = (nodesy[i*MeshStatesN] + (PixelSpacing * *fpy))
      - nodesy[MeshStatesN];
  }
  
  /* allocate storage for the field samples if required */
  if ((FieldSamples = (FESample *) XvgMM_Alloc((void *) FieldSamples,
					       height*width
					       *sizeof(FESample))) == NULL) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort : can't allocate for field samples in ComputeMarkerPath()");
    draw_mesg(cbuf);
    return;
  }

  /* create the mesh and compute element coordinates */
  draw_mesg("ComputeMarkerPath() : Computing element coordinates...");
  if ((FEMeshPixels == NULL)
      || (XvgMM_GetBlockSize(FEMeshPixels) != height*width*sizeof(short))) {
    if ((FEMeshPixels = (short *) XvgMM_Alloc((void *) FEMeshPixels,
					      height*width*sizeof(short)))
	== NULL) {
      Abort = B_TRUE;
      sprintf(cbuf, "Abort : can't allocate for mesh pixels in ComputeMarkerPath()");
      draw_mesg(cbuf);
      return;
    }
    ComputeAllPixelsXiCoords(B_TRUE, B_TRUE, BILINEAR_FIELD_MODEL);
  }
  /* else just calculate the element coordinates */
  else {
    ComputeAllPixelsXiCoords(B_TRUE, B_FALSE, BILINEAR_FIELD_MODEL);
  }
  if (Abort)
    return;
#ifdef DEBUG
  printf("ComputeMarkerPath() : Computed element coordinates...\n");
#endif
  
  /* store this marker */
  NFEMarkers = 1;
  FEMarkers[0] = FieldSamples[y0*width + x0];
  LoadOverlayFunction(DrawFEMarkers);

  /* allocate storage for the marker coordinates */
  if ((xp = (double *) XvgMM_Alloc(NULL, MeshStatesN * sizeof(double)))
      == NULL) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort : Can't allocate for xp in ComputeMarkerPath()");
    ErrorMsg(cbuf);
    return;
  }
  if ((yp = (double *) XvgMM_Alloc(NULL, MeshStatesN * sizeof(double)))
      == NULL) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort : Can't allocate for yp in ComputeMarkerPath()");
    ErrorMsg(cbuf);
    return;
  }
#ifdef DEBUG
  printf("ComputeMarkerPath() : Storage allocated...\n");
#endif

  /* loop for all mesh states */
  for (n = 0; n < MeshStatesN; n++) {
    for (i = 0; i < nnodes; i++) {
      /* nodal coordinates in pixels */
      MeshNodes[i].xp = (nodesx[i*MeshStatesN + n]
			 - nodesx[MeshStatesN])/(*LocalPS) + *fpx; 
      MeshNodes[i].yp = (nodesy[i*MeshStatesN + n]
			 - nodesy[MeshStatesN])/(*LocalPS) + *fpy;
      
      /* initialize the nodal coordinates in meter units */
      MeshNodes[i].xw = (nodesx[i*MeshStatesN + n] + (PixelSpacing * *fpx))
	- nodesx[MeshStatesN];
      MeshNodes[i].yw = (nodesy[i*MeshStatesN + n] + (PixelSpacing * *fpy))
	- nodesy[MeshStatesN];
    }
    /* compute the interpolated marker position */
    ComputePixelMaterialCoords(FieldSamples[y0*width + x0],
			       B_FALSE, &(xp[n]), &(yp[n]));
  }
#ifdef DEBUG
  printf("ComputeMarkerPath() : Computed interpolated coordinates...\n");
#endif

  /* check that reference point jibes */
  if ((fabs(((double) x0) - xp[0]) > .1) || (fabs(((double) y0) - yp[0]) > .1)) {
    printf("ERROR, x0=%d y0=%d xp[0]=%e yp[0]=%e\n", x0, y0, xp[0], yp[0]);
    Abort = B_TRUE;
    return;
  }
#ifdef DEBUG
  printf("ComputeMarkerPath() : Checked 0,0 point...\n");
#endif
    
  /* open output file */
  if ((DataFp=matOpen(ParmPtrs[3], "wb")) == NULL) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort : Can't open output file %s in ComputeMarkerPath()",
	    ParmPtrs[1]);
    draw_mesg(cbuf);
    return;
  }

  /* write marker positions */
  a = mxCreateFull(1, MeshStatesN, REAL);
  memcpy(mxGetPr(a), xp, MeshStatesN * sizeof(double));
  mxSetName(a, "MarkersX");
  matPutMatrix(DataFp, a);
  memcpy(mxGetPr(a), yp, MeshStatesN * sizeof(double));
  mxSetName(a, "MarkersY");
  matPutMatrix(DataFp, a);
  mxFreeMatrix(a);
  matClose(DataFp);
#ifdef DEBUG
  printf("ComputeMarkerPath() : Wrote data to the output file...\n");
#endif

  /* free storage */
  XvgMM_Free(nodesx);
  XvgMM_Free(nodesy);
  XvgMM_Free(ImDataP);
  XvgMM_Free(fpx);
  XvgMM_Free(fpy);
  XvgMM_Free(LocalPS);
  XvgMM_Free(xp);
  XvgMM_Free(yp);
#ifdef DEBUG
  printf("ComputeMarkerPath() : Freed storage...\n");
#endif
  
  /* reload mesh node coordinates for reference state */
  for (i = 0; i < nnodes; i++) {
    /* nodal coordinates in pixels */
    MeshNodes[i].xp = (nodesx[i*MeshStatesN] - nodesx[MeshStatesN])/(*LocalPS) + *fpx; 
    MeshNodes[i].yp = (nodesy[i*MeshStatesN] - nodesy[MeshStatesN])/(*LocalPS) + *fpy;
    
    /* initialize the nodal coordinates in meter units */
    MeshNodes[i].xw = (nodesx[i*MeshStatesN] + (PixelSpacing * *fpx))
      - nodesx[MeshStatesN];
    MeshNodes[i].yw = (nodesy[i*MeshStatesN] + (PixelSpacing * *fpy))
      - nodesy[MeshStatesN];
  }
  
  /* copy the data to the output buffer */
  for (i = 0; i < height*width; i++)
    outpixels[i] = inpixels[i];
#ifdef DEBUG
  printf("ComputeMarkerPath() : Transfered data...\n");
#endif
}


void MeshVideoLoop(double *inpixels, double *outpixels,
		   int height, int width, void *ParmPtrs[])
/***********************************************************************

FILE: feIPOPs.c

COMMAND: MeshVideoLoop

DESCRIPTION:
  This function creates a sequence of images showing the deformation
  of a membrane during an experiment, with the finite element mesh shown as
  an overlay. The images may be output to a series of TIFF files or
  sent directly to a VCR to create an animation sequence of the deformation.
  The function takes as input a file describing the kinematics of
  the membrane during the deformation experiment as well as a number
  of image files of the membrane captured using a camera during the
  experiment. The function creates intermediate images of the membrane
  by using weighted averages of the input images, as required.

  The function uses the following paramaters:
    - inpixels  : pointer to the input image buffer
    - outpixels : pointer to the output image buffer
    - height    : image height
    - width     : image width
    - ParmPtrs  : array of function parameters, where
       ParmPtrs[0] : Name of the input data file which contains the
                     finite element based kinematic specifications,
		     in the format described in LoadMesh().
       ParmPtrs[1] : Number of frames per cycle, where each cycle
                     corresponds to a complete deformation from min to max.
                     If the number is negative, the images will be send to
		     the VCR. If the number is positive, the images will be
                     written to disk as "img***.tiff".
       ParmPtrs[2] : Scale of the principal strains (0 indicates mesh
                     drawing only, without strains).
       ParmPtrs[3] : Name of the input text file listing the membrane images.

RETURN: Void.

MODIFIED:
  - 25/August 1995, Paul Charette, Added support for writing to files.

======================================================================*/
{
  boolean_t BoolTmp1, BoolTmp2, VideoOut;
  double scale, tmp, delta, lmin, lmax, *lpixels;
  int i, j, k, *LocalParms[4], ONE=1, ZERO=0, period, n, nf, lheight,
  lwidth, MeshStateInc, indx, FramesPerCycle, *SavImgFileParms[4];
  void (*f)(double *inpixels, double *outpixels, int height, int width,
	    void **LocalParms);

  /* load input parameters */
  ONE = 1;
  ZERO = 0;
  FramesPerCycle = *((int *) ParmPtrs[1]);
  if (FramesPerCycle == 0) {
    sprintf(cbuf, "Abort : FramesPerCycle = 0 in MeshVideoLoop()");
    ErrorMsg(cbuf);
    Abort = B_TRUE;
    return;
  }
  else if (FramesPerCycle > 0) {
    VideoOut = B_FALSE;
  }
  else {
    VideoOut = B_TRUE;
    FramesPerCycle = -FramesPerCycle;
  }
  scale = *((double *) ParmPtrs[2]);
  if (scale < 0) {
    sprintf(cbuf, "Abort : scale = %f in MeshVideoLoop()", scale);
    ErrorMsg(cbuf);
    Abort = B_TRUE;
    return;
  }
  
  /* load background image names into array */
  if (LoadMovie(ParmPtrs[3]) == B_FALSE) {
    sprintf(cbuf, "Abort : error loading movie file \"%s\" in MeshVideoLoop()",
	    ParmPtrs[3]);
    ErrorMsg(cbuf);
    Abort = B_TRUE;
    return;
  }
  sprintf(cbuf, "MeshVideoLoop() - %d background file names loaded...",
	  MovieFilesN);
  draw_mesg(cbuf);
#ifdef DEBUG
    printf("MeshVideoLoop() : %d background filenames loaded\n", MovieFilesN);
#endif

  /* fetch number of mesh states from the mesh definition file */
  LocalParms[0] = ParmPtrs[0];
  LocalParms[1] = &ZERO;
  LocalParms[2] = &ONE;
  LocalParms[3] = &ONE;
  LoadMesh(inpixels, outpixels, height, width, (void **) LocalParms);
  if (MeshStatesN <= 0) {
    sprintf(cbuf, "Abort : MeshSatesN = %d in MeshVideoLoop()", MeshStatesN);
    ErrorMsg(cbuf);
    Abort = B_TRUE;
    return;
  }
  sprintf(cbuf, "MeshVideoLoop() - %d state mesh loaded...", MeshStatesN);
  draw_mesg(cbuf);
#ifdef DEBUG
  printf("MeshVideoLoop() - Loaded initial mesh definition\n");
#endif

  /* compute looping parameters */
  if (FramesPerCycle > MeshStatesN) {
    FramesPerCycle = MeshStatesN;
    MeshStateInc = 1;
  }
  else
    MeshStateInc = MeshStatesN / FramesPerCycle;

  /* save state variables to local storage if required */
  if (VideoOut) {
    BoolTmp1 = VCRRecordOn;
    BoolTmp2 = VCAWriteOn;
    VCRRecordOn = B_TRUE;
    VCAWriteOn = B_TRUE;
  }
  else
    SavImgFileParms[0] = (int *) cbuf;

  /* compute background image period parms */
  period = FramesPerCycle / (MovieFilesN - 1);
  if (period < 1) {
    sprintf(cbuf, "Abort : period = %d in MeshVideoLoop()", period);
    ErrorMsg(cbuf);
    Abort = B_TRUE;
    return;
  }
#ifdef DEBUG
  printf("MeshVideoLoop() - Morphing period : %d\n", period);
#endif

  /* disable screen saver */
  ScreenSaverOff();

  /* select drawing routine "mesh" or "strain & mesh" */
  if (scale == 0.0) {
    f = LoadMesh;
    LocalParms[2] = &ONE;
    LocalParms[3] = &ZERO;
  }
  else {
    f = FEStrain;
    LocalParms[2] = (int *) (&scale);
    LocalParms[3] = &ONE;
  }

  /* load the first background image */
  if (GetTiffFile(MovieFileNames[0], &lpixels, &lheight, &lwidth, &lmin, &lmax,
		  cmap, UserCells, &TrueColorImage, VerboseFlag) == B_FALSE) {
    Abort = B_TRUE;
    sprintf(cbuf, "Error reading TIFF file %s in MeshVideoLoop()",
	    MovieFileNames[0]);
    draw_mesg(cbuf);
  }
  if (((height != lheight) || (width != lwidth)) && (Abort == B_FALSE)) {
    Abort = B_TRUE;
    sprintf(cbuf, "File %s has wrong size (%dx%d) in MeshVideoLoop()",
	    MovieFileNames[1], lheight, lwidth);
    draw_mesg(cbuf);
  }
  for (i = 0; (i < height*width) && (Abort == B_FALSE); i++)
    DoubleBuffer[i] = lpixels[i];


  /* load the second background image */
  if (GetTiffFile(MovieFileNames[1], &lpixels, &lheight, &lwidth, &lmin, &lmax,
		  cmap, UserCells, &TrueColorImage, VerboseFlag) == B_FALSE) {
    Abort = B_TRUE;
    sprintf(cbuf, "Error reading TIFF file %s in MeshVideoLoop()",
	    MovieFileNames[1]);
    draw_mesg(cbuf);
  }
  if (((height != lheight) || (width != lwidth)) && (Abort == B_FALSE)) {
    Abort = B_TRUE;
    sprintf(cbuf, "File %s has wrong size (%dx%d) in MeshVideoLoop()",
	    MovieFileNames[1], lheight, lwidth);
    draw_mesg(cbuf);
  }
#ifdef DEBUG
  printf("MeshVideoLoop() - Loaded first two background images\n");
#endif

  /* loop for increasing states */
  for (i = 0, k = 0, n = 0, nf=2, LocalParms[1] = &i, indx=0, delta=period;
       (i < MeshStatesN) && (Abort == B_FALSE); i+= MeshStateInc, k++, n++) { 
    /* print banner */
    sprintf(cbuf, "MeshVideoLoop() - Forward looping step %d/%d\n",
	    i/MeshStateInc + 1, MeshStatesN/MeshStateInc);
    draw_mesg(cbuf);
    /* check if at new period, if so, load image pair info */
    if ((n == period) && (nf < MovieFilesN)) {
      /* transfer the second file to the buffer */
      for (j = 0; (j < height*width) && (Abort == B_FALSE); j++)
	DoubleBuffer[j] = lpixels[j];
      /* load the next file */
      if (GetTiffFile(MovieFileNames[nf], &lpixels, &lheight, &lwidth, &lmin,
		      &lmax, cmap, UserCells, &TrueColorImage,
		      VerboseFlag) == B_FALSE) {
	Abort = B_TRUE;
	sprintf(cbuf, "Error reading TIFF file %s in MeshVideoLoop()",
		MovieFileNames[nf]);
	draw_mesg(cbuf);
      }
      if (((height != lheight) || (width != lwidth)) && (Abort == B_FALSE)) {
	Abort = B_TRUE;
	sprintf(cbuf, "File %s has wrong size (%dx%d) in MeshVideoLoop()",
		MovieFileNames[nf], lheight, lwidth);
	draw_mesg(cbuf);
      }
      /* update counters */
      nf++;
      n = 0;

      /* if this is the final image pair, set the period to take up */
      /* the slack caused by roundoff error */
      if (nf == MovieFilesN)
	delta = (MeshStatesN - i)/MeshStateInc;
#ifdef DEBUG
      printf("MeshVideoLoop() - Loaded background image %s\n",
	     MovieFileNames[nf-1]);
#endif
    }

    /* mix background files */
    printf("Delta = %f, n = %d, i = %d, \n", delta, n, i);
    MixEm(DoubleBuffer, lpixels, outpixels, height, width,
	  1.0 - ((double) n) / delta);

    /* execute the mesh loading and/or strain computation */
    (f)(outpixels, outpixels, height, width, (void **) LocalParms);

    /* output frames to VCR */
    if (VideoOut) {
      /* display image in the window */
      ShowImage(outpixels);
      /* draw frame number */
      sprintf(cbuf, "%03d", k);
      XDrawString(UxDisplay, XtWindow(ProcessedImageDrawingArea),
		  gc_Overlay, 5, TEXT_OVERLAY_HEIGHT+5, cbuf, strlen(cbuf));
      /* execute overlay function */
      for (j = 0; j < NOverlayFunctions; j++)
	(OverlayFunctions[j])(ProcessedImageDrawingArea);
      /* dump image to VCR */
/*
      if (DumpImage() == B_FALSE) {
	Abort = B_TRUE;
	sprintf(cbuf, "Abort : Error dumping frame %d/%d in MeshVideoLoop()",
		i/MeshStateInc + 1, MeshStatesN/MeshStateInc);
	draw_mesg(cbuf);
      }
*/
    }
    /* or send output to a disk file */
    else {
      sprintf(cbuf, "img%03d.tiff", indx++);
      SaveTIFFGreyLevelFile(outpixels, outpixels, height, width,
			    (void **) SavImgFileParms);
    }
  }
  
  /* loop for decreasing states if sending output to the VCR */
  if (VideoOut) {
    
    /* swap buffers */
    for (j = 0; (j < height*width) && (Abort == B_FALSE); j++) {
      tmp = DoubleBuffer[j];
      DoubleBuffer[j] = lpixels[j];
      lpixels[j] = tmp;
    }
    
    /* loop for decreasing states */
    for (i-= MeshStateInc, n--, nf-=3; (i >= 0) && (Abort == B_FALSE);
	 i-= MeshStateInc, k++) {
      /* print banner */
      sprintf(cbuf, "MeshVideoLoop() - Backward looping step %d/%d\n",
	      MeshStatesN/MeshStateInc - i/MeshStateInc,
	      MeshStatesN/MeshStateInc);
      draw_mesg(cbuf);
      /* check if at new cycle, if so, load first image into */
      if ((n < 0) && (nf >= 0)) {
	/* transfer the second file to the buffer */
	for (j = 0; (j < height*width) && (Abort == B_FALSE); j++)
	  DoubleBuffer[j] = lpixels[j];
	/* load the next file */
	if (GetTiffFile(MovieFileNames[nf], &lpixels, &lheight, &lwidth, &lmin,
			&lmax, cmap, UserCells, &TrueColorImage,
			VerboseFlag) == B_FALSE) {
	  Abort = B_TRUE;
	  sprintf(cbuf, "Error reading TIFF file %s in MeshVideoLoop()",
		  MovieFileNames[nf]);
	  draw_mesg(cbuf);
	}
	/* update counters */
	nf--;
	n = period-1;
	delta = period;
#ifdef DEBUG
	printf("MeshVideoLoop() - Loaded background image %s\n",
	       MovieFileNames[nf+1]);
#endif
      }
      /* mix background files */
      if (n >= 0)
	MixEm(DoubleBuffer, lpixels, outpixels, height, width,
	      ((double) n) / delta);
      /* execute the mesh loading or strain computation */
      (f)(outpixels, outpixels, height, width, (void **) LocalParms);
      /* display image in the window */
      ShowImage(outpixels);
      /* execute overlay function */
      for (j = 0; j < NOverlayFunctions; j++)
	(OverlayFunctions[j])(ProcessedImageDrawingArea);
      /* draw frame number */
      sprintf(cbuf, "%03d", k);
      XDrawString(UxDisplay, XtWindow(ProcessedImageDrawingArea),
		  gc_Overlay, 5, TEXT_OVERLAY_HEIGHT+5, cbuf, strlen(cbuf));
      /* output frames to VCR */
      if (VideoOut)
/*
	if (DumpImage() == B_FALSE) {
	  Abort = B_TRUE;
	  sprintf(cbuf, "Abort : Error dumping frame %d/%d in MeshVideoLoop()",
		  MeshStatesN/MeshStateInc - i/MeshStateInc,
		  MeshStatesN/MeshStateInc);
	  draw_mesg(cbuf);
	}
*/
      /* update mixing factor */
      if (n >= 0)
	n--;
    }
    
    /* show initial image again if required */
    if (n > 0) {
      /* display image in the window */
      ShowImage(lpixels);
      /* execute overlay function */
      for (j = 0; j < NOverlayFunctions; j++)
	(OverlayFunctions[j])(ProcessedImageDrawingArea);
    }
    
    /* restore system state variables */
    VCRRecordOn = BoolTmp1;
    VCAWriteOn = BoolTmp2;
  }
  
  /* reenable screen saver */
  ScreenSaverOn();
  
  /* free up the storage */
  XvgMM_Free(lpixels);
}

/************************ RegionMaskCreation *******************************/
/*                                                                         */
/*  function: seed fill disconnected regions with distinct values          */
/*                                                                         */
/*            Mark the largest region with an index of +1 all others to -1 */
/*                                                                         */
/***************************************************************************/
void RegionMaskCreation(double *inpixels, double *outpixels, int height,
			int width, void *ParmPtrs[])
{
  int i, j, k, count, CentroidX, CentroidY, maxcnt, MinSize;
  double c, maxc;

  /* load parameters */
  MinSize = *((int *) ParmPtrs[0]);

  /* make sure contour is defined */
  if (DisplayContourComputed == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort : Undefined contour in RegionMaskCreation()");
    return;
  }

  /* show banner */
  draw_mesg("Region mask creation...");

  /* copy data to outpixels */
  for (i = 0; i < height*width; i++)
      outpixels[i] = inpixels[i];

  /* seed fill disjointed regions to segment image */
  for (i=0, k=0, c=2.0, maxcnt=0; (i < height) && (Abort == B_FALSE); i++)
    for (j = 0; (j < width) && (Abort == B_FALSE); j++, k++)
      if (outpixels[k] == 1.0) {
	/* seed fill this region */
	count = RegionFill(outpixels, height, width, i, j, 1.0, c, NULL, NULL,
			   &CentroidX, &CentroidY); 

	/* if this region contains fewer pixels than MinSize, fill it with 0 */
	if (count < MinSize) {
	  RegionFill(outpixels, height, width, i, j, c, 0.0, NULL, NULL,
		     &CentroidX, &CentroidY);
	}
	
	/* store this color index if its the largest region so far */
	if (count > maxcnt)
	  {
	    maxcnt = count;
	    maxc = c;
	  }
	
	/* increment color counter */
	c++;
      }
  
  /* change the color of the largest region to 1 and other regions to -1 */
  for (i = 0; i < height*width; i++)
    if (outpixels[i] == maxc)
      outpixels[i] = 1;
    else if (outpixels[i] != 0)
      outpixels[i] = -1;
}


/*************************** VAFCompute ************************************/
/*                                                                         */
/*  function: compute the VAF between two images within the contour        */
/*                                                                         */
/***************************************************************************/
void VAFCompute(double *inpixels, double *outpixels, int height,
		int width, void *ParmPtrs[])
{
  double SumZ2, SumZZp2, Z, Zp;
  int x, y, VAF, index;

  /* check that stack is not empty, then decrement stack pointer  */
  if (ImageStackTop < 1) {
    Abort = B_TRUE;
    draw_mesg("Abort : No image on the stack for VAF()");
    return;
  }
  
  /* make sure contour is defined */
  if (DisplayContourComputed == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort : Undefined contour in VAF()");
    return;
  }

  /* compute the VAF */
  draw_mesg("Computing VAF...");
  for (y=0, SumZ2=0, SumZZp2=0; y < height; y++)
    for (x=0; x < width; x++) {
      index = y*width +x;
      if (MaskPixels[index] != 0) {
	Zp = inpixels[index];
	Z = ImageStack[ImageStackTop-1].pixels[index];
	outpixels[index] = Z - Zp;
	/* compute sums for VAF computation */
	SumZZp2 += ((Z - Zp) * (Z - Zp));
	SumZ2 += (Z * Z);
      }
      else
	outpixels[index] = 0;
    }
  VAF = 100.0*(1.0 - SumZZp2/SumZ2);

  /* print out the VAF if required */
  if (!InteractiveFlag && !DisplayOnlyFlag)
    sprintf(Msg, "VAF = %d%% for file %s ", VAF, InFileNameStripped);
  else
    sprintf(Msg, " = %d%% ", VAF);
  if (VerboseFlag)
    printf("VAFCompute() : VAF = %d%%\n", VAF);
}





