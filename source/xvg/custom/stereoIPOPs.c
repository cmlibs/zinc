/***********************************************************************
*
*  Name:          stereoIPOPs.c
*
*  Author:        Paul Charette
*
*  Last Modified: 26 Oct 1997
*
*  Purpose:       Utility routines for stereo image processing.
*
*                      OpticalFlow()
*                      OpticalFlowFieldFilter()
*                      OpticalFlowLoadNodes()
*                      OpticalFlowMakeMovie()
*                      OpticalFlowNodeCMFilter()
*                      PatchXCorr()
*                      PatchXCorrByAreaHierarchical()
*                      PatchXCorrByAreaParallel()
*                      PatchXCorrByLine()
*                      PatchXCorrByHorizSliceParallel()
*                      GlobalPlaneFit()
*                      MeanUVW()
*
***********************************************************************/
#include <stdio.h>
#include "UxXt.h"
#include <values.h>
#include <sys/types.h>
#ifdef SGI
#include <fcntl.h>
#include <sys/ipc.h> 
#else
#include <sys/mode.h> 
#endif
#include <sys/sem.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include "XvgGlobals.h"

#define VERSIONUM 26

/* maximum number of points too track (single sided) */
#define N_POINTS_MAX      308
#define N_POINTS_ACTUAL   282

/* numof times to loop to recover from co-registration errors */
#define N_COREG_ERROR_MAX   3

/* forward declarations */
static void OpticalFlowNodesOverlay(Widget DrawingArea);

/* largest allowed center of mass filter */
#define MAXCMRADIUS 50

/* calculation precision */
#define PRECISION  1.0e-10
#define MAXHPASSES 25

/* global definition for child processes */
int   nprocessors;
static pid_t pids[PROCESSORS_NMAX];
static int   ChildProcessNumber, SemID, npids;

/* global point tracking defintions */
typedef struct {
  int index;
  double x, y;
  int WeightType;
  int LorR;
  int GuessXOffset, GuessYOffset;
  boolean_t Done;
  double CMRadiusX, CMRadiusY;
  double FieldEstimateDx, FieldEstimateDy, ObjFunc, VAF;
} POINT_TRACK_REQUEST;

/* global stereo point definition */
typedef struct {
  int valid;
  double xl, yl, xr, yr;
  double ObjL, ObjR, VAFL, VAFR;
  double CMRadiusXL, CMRadiusYL, CMRadiusXR, CMRadiusYR;
} STEREO_POINT;

/* global FFT cache declarations */
#define FFT_CACHE_SIZE_MAX 1000
typedef struct {
  int index;
  double time;
  double *FFTReal, *FFTIm;
} FFT_CACHE_ITEM;
typedef struct {
  int x, y;
  boolean_t Done;
} FFT_CACHE_REQUEST;
FFT_CACHE_ITEM *FFTCache;
static double *FFTCacheBuffer;
static short *FFTCacheTable;
static int FFTCacheNItems, FFTCacheHitsNum, FFTCacheRequestsNum;
static FFT_CACHE_REQUEST *FFTCacheRequests;

/* local static storage */
static double meanU, meanV, CurrentObjFun, AverageShiftX, AverageShiftY,
*OpticalFlowNodesX, *OpticalFlowNodesY;
static boolean_t ShiftQuadrants = B_TRUE, InMemoryPixmapFlag;
static int NOpticalFlowNodes, OpticalFlowNodesWriteIndicies, *OpticalFlowNodesI;
static Pixmap InMemoryPixmap;

void InitStereoParameters(void)
{
  npids = 0;
  nprocessors = 0;
  OpticalFlowNodesX = NULL;
  OpticalFlowNodesX = NULL;
  OpticalFlowNodesI = NULL;
  NOpticalFlowNodes = 0;
  OpticalFlowNodesWriteIndicies = 0;
  FFTCacheBuffer = NULL;
  FFTCache = NULL;
  FFTCacheTable = NULL;
  FFTCacheRequests = NULL;
  InMemoryPixmapFlag = B_FALSE;
}

void RemoveSemaphores(void)
{
  semctl(SemID, 0, IPC_RMID, 0);
}

void StopChildren(void)
{
  int i;
  
  for (i = 0; i < npids; i++)
    kill(pids[i], SIGKILL);
  npids = 0;
}

/************************** FFTCacheLookup ***************************
*
*   function : FFTCacheLookup()
*
*	parameters:
*            FFTCacheRequest:    Cache request index
*            pixels:             Image data to cache from
*            Xextent:            Width of this image
*            Re:                 Address to pointer to real part of FFT
*            Im:                 Address to pointer to imaginary part of FFT
*
*       Return values:
*            1 : Cache miss
*            0 : Cache hit
*           -1 : Error
*
***********************************************************************/
static boolean_t InitFFTCache(int FFTCacheSize, int height, int width,
			      int kernw)
{
  int k;
  
  /* initialize counters */
  FFTCacheNItems = 0;
  FFTCacheHitsNum = 0;
  FFTCacheRequestsNum = 0;

  /* allocate storage */
  if (FFTCacheSize > 0) {
    FFTCacheTable = (short *) XvgMM_Alloc((void *) FFTCacheTable, 
					  height*width*sizeof(short));
    FFTCache = (FFT_CACHE_ITEM *)
      XvgMM_Alloc((void *) FFTCache, FFTCacheSize*sizeof(FFT_CACHE_ITEM));
  }
  else {
    FFTCacheTable = NULL;
    FFTCache = NULL;
  }
  FFTCacheBuffer = (double *) XvgMM_Alloc((void *) FFTCacheBuffer,
					  (kernw*kernw)*sizeof(double));

  /* check that storage succeeded */
  if ((FFTCacheBuffer == NULL)
      || ((FFTCacheSize > 0)
	  && ((FFTCacheTable == NULL) || (FFTCache == NULL)))) {
    Abort = B_TRUE;
    sprintf(cbuf, "InitFFTCache(kernw:%d) : alloc failed", kernw);
    draw_mesg(cbuf);
    return(B_FALSE);
  }

  /* init the FFT cache, if required */
  if (FFTCacheSize > 0) {
    for (k = 0; k < height*width; k++)
      FFTCacheTable[k] = -1;
    for (k = 0; k < FFTCacheSize; k++)
      FFTCache[k].index = -1;
  }

  /* return success */
  return(B_TRUE);
}

static void FreeFFTCache(int FFTCacheSize)
{
  int k;

  if (FFTCacheSize > 0) {
    for (k = 0; k < FFTCacheNItems; k++) {
      XvgMM_Free(FFTCache[k].FFTReal);
      XvgMM_Free(FFTCache[k].FFTIm);
    }
  }
}

static int FFTCacheLookup(FFT_CACHE_REQUEST FFTCacheRequest,
			  double *pixels, int Xextent,
			  int kernw, double **Re, double**Im, int FFTCacheSize)
{
  double OldestTime;
  int TableIndex, CacheIndex, rc, i, x, y, k, IFAIL;

  /* check that the cache is enabled */
  if (FFTCacheSize > 0) {
    /* calculate indicies */
    TableIndex = (FFTCacheRequest.y * Xextent) + FFTCacheRequest.x;
    CacheIndex = FFTCacheTable[TableIndex];
    
    /* find out if this item is already in the cache */
    if (CacheIndex != -1) {
      *Re = FFTCache[CacheIndex].FFTReal;
      *Im = FFTCache[CacheIndex].FFTIm;
      FFTCache[CacheIndex].time = XvgGetTime();
      return(1);
    }
    
    /* if cache not full, look for place in the cache to store the new FFT */
    if (FFTCacheNItems < FFTCacheSize) {
      CacheIndex = FFTCacheNItems++;
      FFTCache[CacheIndex].FFTReal =
	(double *) XvgMM_Alloc((void *) NULL, kernw*kernw*sizeof(double));
      FFTCache[CacheIndex].FFTIm =
	(double *) XvgMM_Alloc((void *) NULL, kernw*kernw*sizeof(double));
      if ((FFTCache[CacheIndex].FFTReal == NULL) ||
	  (FFTCache[CacheIndex].FFTIm == NULL)) {
	return(-1);
      }
      if (VerboseFlag && !ChildProcess)
	printf("FFTCacheLookup():New cache item defined at index %d (%d:%d)\n",
	       CacheIndex, FFTCacheRequest.x, FFTCacheRequest.y);
    }
    /* else look for the oldest cache item and replace it */
    else {
      for (i = 0, OldestTime = MAXDOUBLE; i < FFTCacheSize; i++) {
	if (FFTCache[i].time < OldestTime) {
	  OldestTime = FFTCache[i].time;
	  CacheIndex = i;
	}
      }
      FFTCacheTable[(FFTCache[CacheIndex].index)] = -1;
      if (VerboseFlag && !ChildProcess)
	printf("FFTCacheLookup() : Cache item replaced at index %d (%d:%d)\n",
	       CacheIndex, FFTCacheRequest.x, FFTCacheRequest.y);
    }
    
    /* set cache & table parameters, and return FT pointers */
    FFTCacheTable[TableIndex] = CacheIndex;
    FFTCache[CacheIndex].index = TableIndex;
    FFTCache[CacheIndex].time = XvgGetTime();
    *Re = FFTCache[CacheIndex].FFTReal;
    *Im = FFTCache[CacheIndex].FFTIm;
  }

  /* else, no cache, just calculate FFT */
  else {
      *Re = (double *) XvgMM_Alloc((void *) *Re, kernw*kernw*sizeof(double));
      *Im = (double *) XvgMM_Alloc((void *) *Im, kernw*kernw*sizeof(double));
  }
    
  /* Calculate the new FFT */
  for (y = FFTCacheRequest.y - kernw/2, k = 0;
       y < (FFTCacheRequest.y + kernw/2);
       y++) {
    for (x = FFTCacheRequest.x - kernw/2;
	 x < (FFTCacheRequest.x + kernw/2);
	 x++, k++) {
      FFTCacheBuffer[k] = pixels[y*Xextent + x];
    }
  }
/*  SubtractMean(FFTCacheBuffer, FFTCacheBuffer, kernw, kernw, NULL); */
  FFTApplyWindow(FFTCacheBuffer, *Re, *Im);

  IFAIL = 1;
  c06fuf(&kernw, &kernw, *Re, *Im,
	 "Initial", FFTTrigm2, FFTTrign2, FFTWork2, &IFAIL);
  if (IFAIL != 0) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "Abort:IFAIL=%d on return from c06fuf() in FFTCacheLookup()",
	    IFAIL);
    draw_mesg(cbuf);
    return(-1);
    }
  FFTShift(*Re, *Re, kernw, kernw);
  FFTShift(*Im, *Im, kernw, kernw);
  
  /* check memory */
  sprintf(cbuf, "FFTCacheLookup(Xextent:%d, kernw:%d)", Xextent, kernw);
  XvgMM_MemoryCheck(cbuf);
  
  /* return Cache miss */
  return(0);
}


/****************************** KernelScan() ***************************
*
*   function : KernelScan()
*
*	parameters:
*            FFTCacheRequests:   Array of cache requests                      
*            pixels:             Image from which FFTs are cached
*            mask:               Doughnut shaped mask of valid pixels
*            Xextent:            Width of the image
*            NRequests:          number of cache requests to scan
*            PatchCX:            reference image patch center X coordinate
*            PatchCY:            reference image patch center Y coordinate
*            kernw:              pixel dim of the square scanning kernels
*            offsetsX, offsetsY, dxs, dys, objs, VAFs:
*                                work buffers
*            dx, dy:             (OUTPUT) calculated incremental shifts
*            minShiftX, minShiftX:
*                                (OUTPUT) calculated total shifts
*            minobj:             (OUTPUT) minimum objective function
*            MaxVAF:             (OUTPUT) maximum VAF
*
*  Note: for some unknown reason, the pixel shifts are always underestimated
*        by a fixed scale factor. This error is estimated and corrected for
*        by looking at the neighbors with the lowest objective functions in the
*        X and Y directions.
*
***********************************************************************/
static int KernelScan(FFT_CACHE_REQUEST *FFTCacheRequests, int FFTCacheSize,
		      double *pixels, double *mask, int Xextent, int NRequests,
		      int PatchCX, int PatchCY, int kernw,
		      double *offsetsX, double *offsetsY,
		      double *dxs, double *dys, 
		      double *objs, double *VAFs,
		      double *dx, double *dy,
		      double *minShiftX, double *minShiftY,
		      double *minobj, double *MaxVAF)
{
  static double *Re=NULL, *Im=NULL;
  void *ParmPtrs[1];
  boolean_t OverflowFlag, BoolTmp;
  double SumX2, SumY2, SumXY, SumXZ, SumYZ, MaxObj, SumZZp2, ZZp, U, V, DX,
  MinObjVal, MinD, v, DY, denom, SumZ2, VAF, lVAFMax, scale, SumW2,
  MaxVAFVal, X, Y, Phase, CoherenceSquared;
  int i, k, x, y, MinObjIndx, MaxVAFIndx, hits, rc, left, right, top, bottom;
  boolean_t tmp;
  char fname[256];

  /* set internal parameters */
  MinObjVal = MAXDOUBLE;
  MaxVAFVal = -1;
  MaxObj = 0.0;
  lVAFMax = 0.0;
  hits = 0;

  /* loop to identify the phase shift */
  for (i = 0, MinObjIndx = 0, MaxVAFIndx = 0;
       (i < NRequests) && (Abort == B_FALSE);
       i++) {
    
    /* FFT cache lookup */
    tmp = VerboseFlag;
    VerboseFlag = B_FALSE;
    rc = FFTCacheLookup(FFTCacheRequests[i], pixels, Xextent,
			kernw, &Re, &Im, FFTCacheSize);
    VerboseFlag = tmp;
    if (rc < 0) {
      return(rc);
    }
    hits += rc;

    /* calculate the raw phase values from the cross spectrum function */
    SumX2 = 0; SumY2 = 0; SumXY = 0; SumXZ = 0; SumYZ = 0;
    for (y = 0, k = 0; y < kernw; y++) {
      /* plane model Y coordinate of this pixel */
      Y = y - kernw/2;
      for (x = 0; x < kernw; x++, k++) {
	/* plane model X coordinate of this pixel */
	X = x - kernw/2;
	/* check if sample is within the fitting radius, if so calculate */
	if (mask[k] != 0.0) {
	  /* cross spectrum terms */
	  FFTRealBuffer2[k] = (FFTRealBuffer[k]*Re[k] + FFTImBuffer[k]*Im[k]);
	  FFTImBuffer2[k]   = (FFTImBuffer[k]*Re[k] - FFTRealBuffer[k]*Im[k]);
	  Phase             = atan2(FFTImBuffer2[k], FFTRealBuffer2[k]);
	  
	  /* coherence squared function */
#ifdef _KERNEL_SCAN_COHERENCE_SQUARED
	  CoherenceSquared = (FFTRealBuffer2[k]*FFTRealBuffer2[k]
			+ FFTImBuffer2[k]*FFTImBuffer2[k])
	    /((FFTRealBuffer[k]*FFTRealBuffer[k] +
	       FFTImBuffer[k]*FFTImBuffer[k])
	      * (Re[k]*Re[k] + Im[k]*Im[k]));
#else
          /* NOTE: use magnitude squared instead */
	  CoherenceSquared = (FFTRealBuffer2[k]*FFTRealBuffer2[k]
			+ FFTImBuffer2[k]*FFTImBuffer2[k]);
#endif
	  FFTImBuffer2[k]   = Phase;
	  FFTRealBuffer2[k] = CoherenceSquared;

	  /* update weighted least squares fit running sums */
	  SumX2 += (X * X * CoherenceSquared);
	  SumY2 += (Y * Y * CoherenceSquared);
	  SumXY += (X * Y * CoherenceSquared);
	  SumXZ += (X * Phase * CoherenceSquared);
	  SumYZ += (Y * Phase * CoherenceSquared);
	}
	else {
	  FFTImBuffer2[k]   = 0;
	  FFTRealBuffer2[k] = 0.0;
	}
      }
    }
    
    /* calculate the plane model parameters */
    scale = ((double) kernw)/TWOPI;
    denom = SumX2*SumY2 - SumXY*SumXY;
    if (fabs(denom) > MINDOUBLE) {
      U = (SumY2*SumXZ - SumXY*SumYZ)/denom;
      V = (SumX2*SumYZ - SumXY*SumXZ)/denom;
      DX = -U*scale;
      DY = -V*scale;
      OverflowFlag = B_FALSE;
    }
    else {
      DX = -kernw/4;
      DY = -kernw/4;
      OverflowFlag = B_TRUE;
    }

    /* calculate the objective function (sum of squared residuals) */
    for (y = 0, k = 0, SumZ2 = 0, SumZZp2=0, SumW2=0; y < kernw; y++) {
      Y = y - kernw/2;
      for (x = 0; x < kernw; x++, k++) {
	X = x - kernw/2;
	/* check if sample is within the fitting radius, if so update sums */
	if (mask[k] != 0.0) {
	  SumW2   += FFTRealBuffer2[k];
	  ZZp      = (U*X + V*Y - FFTImBuffer2[k]);
	  SumZZp2 += (ZZp * ZZp * FFTRealBuffer2[k]);
	  SumZ2   += (FFTImBuffer2[k] * FFTImBuffer2[k]  * FFTRealBuffer2[k]);
	}
      }
    }

    /* flag over/underflow in the calculations with VAF = -2 */
    if (!finite(SumZ2) || OverflowFlag) {
      VAF = -2;
    }
    /* for sum of squared samples and errors less than PRECISION, VAF = 100% */
    else if ((SumZ2 < PRECISION) && (SumZZp2 < PRECISION)) {
      VAF = 100.0;
    }
    /* normal condition, compute %VAF. If %VAF < 0, flag this by %VAF = -1 */
    else {
      VAF = 100.0*(1.0 - SumZZp2/SumZ2);
      VAF = (VAF > 0.0 ? VAF : -1);
    }
    
    /* store the results in local buffers */
    dxs[i] = DX;
    dys[i] = DY;
    offsetsX[i] = PatchCX - FFTCacheRequests[i].x;
    offsetsY[i] = PatchCY - FFTCacheRequests[i].y;
    objs[i] = SumZZp2;
    VAFs[i] = VAF;
    
    /* information output */
    if ((VerboseFlag || ImageUpdateFlag)
	&& !(ChildProcess && (nprocessors > 1))) {
      if (ImageUpdateFlag) {
        BoolTmp = VerboseFlag;
        VerboseFlag = B_FALSE;
	ShowImage(FFTImBuffer2);
	sprintf(fname, "PhaseX%d_Y%d.mat",
		(int) offsetsX[i], (int) offsetsY[i]);
	WriteMatlabFile(kernw, kernw, "Phase", FFTImBuffer2, fname);
	sprintf(fname, "MagSqrdX%d_Y%d.mat",
		(int) offsetsX[i], (int) offsetsY[i]);
	WriteMatlabFile(kernw, kernw, "MagSqrd", FFTRealBuffer2, fname);
        VerboseFlag = BoolTmp;
      }
      printf("#%03d (OX:%3.f, OY:%3.f)->dx:%6.3f, dy:%6.3f, Obj:%8.2e, %%VAF:%4.1f%%, SW2:%7.1e, SZZp2:%7.1e, SZ2:%7.1e\n",
	     i, offsetsX[i], offsetsY[i], DX, DY, SumZZp2, VAF,
	     SumW2, SumZZp2, SumZ2);
    }
    
    /* store the smallest objective function */ 
    if (SumZZp2 < MinObjVal) {
      MinObjVal = SumZZp2;
      MinObjIndx = i;
    }
   
    /* store the largest %VAF */ 
    if (VAF > MaxVAFVal) {
      MaxVAFVal = VAF;
      MaxVAFIndx = i;
    }
   
    /* check for largest objective function */
    MaxObj = (SumZZp2 > MaxObj ? SumZZp2 : MaxObj);
  }

  /* find optimal solution left neighbour */
  for (i = 0, left = -1; (i < NRequests) && (left == -1); i++) {
    if ((FFTCacheRequests[i].x == (FFTCacheRequests[MinObjIndx].x - 1))
	&& (FFTCacheRequests[i].y == FFTCacheRequests[MinObjIndx].y))
      left = i;
  }
  /* find optimal solution right neighbour */
  for (i = 0, right = -1; (i < NRequests) && (right == -1); i++) {
    if ((FFTCacheRequests[i].x == (FFTCacheRequests[MinObjIndx].x + 1))
	&& (FFTCacheRequests[i].y == FFTCacheRequests[MinObjIndx].y))
      right = i;
  }
  /* find optimal solution top neighbour */
  for (i = 0, top = -1; (i < NRequests) && (top == -1); i++) {
    if ((FFTCacheRequests[i].x == FFTCacheRequests[MinObjIndx].x)
	&& (FFTCacheRequests[i].y == (FFTCacheRequests[MinObjIndx].y - 1)))
      top = i;
  }
  /* find optimal solution bottom neighbour */
  for (i = 0, bottom = -1; (i < NRequests) && (bottom == -1); i++) {
    if ((FFTCacheRequests[i].x == FFTCacheRequests[MinObjIndx].x)
	&& (FFTCacheRequests[i].y == (FFTCacheRequests[MinObjIndx].y + 1)))
      bottom = i;
  }
  
  /* assume an error of the form DELTA*DX (where DELTA is in  [0..1]  */
  /* and DELTA == 1.0 for no error). Average out this error using the */
  /* immediate neighbour with the lowest objective function */
  if ((left != -1) && (right != -1)) {
    if (objs[left] < objs[right]) {
      *minShiftX = ((dxs[MinObjIndx]*(offsetsX[left] - offsetsX[MinObjIndx]))
		    /(dxs[MinObjIndx] - dxs[left]))  + offsetsX[MinObjIndx];
    }
    else {
      *minShiftX = ((dxs[MinObjIndx]*(offsetsX[right] - offsetsX[MinObjIndx]))
		    /(dxs[MinObjIndx] - dxs[right])) + offsetsX[MinObjIndx];
    }
    *dx = *minShiftX - rint(*minShiftX);
  }
  else {
    *dx = dxs[MinObjIndx];
    *minShiftX = offsetsX[MinObjIndx] + *dx;
  }
  
  /* assume an error of the form DELTA*DY (where DELTA is in  [0..1]  */
  /* and DELTA == 1.0 for no error). Average out this error using the */
  /* immediate neighbour with the lowest objective function */
  if ((top != -1) && (bottom != -1)) {
    if (objs[top] < objs[bottom]) {
      *minShiftY = ((dys[MinObjIndx]*(offsetsY[top] - offsetsY[MinObjIndx]))
		    /(dys[MinObjIndx] - dys[top]))    + offsetsY[MinObjIndx];
    }
    else {
      *minShiftY = ((dys[MinObjIndx]*(offsetsY[bottom] - offsetsY[MinObjIndx]))
	     /(dys[MinObjIndx] - dys[bottom])) + offsetsY[MinObjIndx];
    }
    *dy = *minShiftY - rint(*minShiftY);
  }
  else {
    *dy = dys[MinObjIndx];
    *minShiftY = offsetsY[MinObjIndx] + *dy;
  }
  
  /* return results */
  if (OverflowFlag == B_FALSE) {
    *minobj = objs[MinObjIndx];
    *MaxVAF = VAFs[MinObjIndx];
  }
  else {
    *minobj = -1;
    *MaxVAF = -1;
  }

  /* print out info if` required */
  if ((VerboseFlag  || ImageUpdateFlag)
      && !(ChildProcess && (nprocessors > 1)))
    printf("KernelScan (X:%3d, Y:%3d): dx:%6.3f, dy:%6.3f, SX:%6.3f, SY:%6.3f, MinObj:%7.1e, MaxObj:%7.1e, %%VAF:%4.1f%%, MinObjIndx = %2d\n",
	   PatchCX, PatchCY, *dx, *dy, *minShiftX, *minShiftY, *minobj, MaxObj, *MaxVAF, MinObjIndx);
  
  /* check memory */
  sprintf(cbuf, "KernelScan(PatchCX:%d, PatchCY:%d)", PatchCX, PatchCY);
  XvgMM_MemoryCheck(cbuf);

  /* return number of cache hits */
  return(hits);
}


/***************************************************************************
*
*   function : PatchXCorr()
*
*   parameters (loaded):
*              ((char *) ParmPtrs[0])  = Output filename prefix
*              *((int *) ParmPtrs[1])  = X scan start in I1 (reference)
*              *((int *) ParmPtrs[2])  = Y scan start in I1 (reference)
*                       parameters[0]  = X scan starting offset in I2 (stack)
*                       parameters[1]  = Y scan starting offset in I2 (stack)
*                       parameters[2]  = X scanning extent
*                       parameters[3]  = Y scanning extent
*                       parameters[4]  = number of lags
*                       parameters[5]  = decimation (image sub-sampling)
*                       parameters[6]  = FT kernel size (power of 2)
*                       parameters[7]  = Fitting area radius max
*                       parameters[8]  = Fitting area radius min
*                       parameters[9] = Scan type (0:linear, 1:circular)
*
***********************************************************************/
static void RestoreImageSize(int zf, int width, int height, boolean_t TmpFlag,
			     int lFFTWindow,
			     double *inpixels, double *outpixels)
{
  int k;

  /* restore image size parameters */
  FFTWindow = lFFTWindow;
  ImgWidth = width;
  ImgHeight = height;
  
  /* restore input buffer */
  InitProcBuffers();
  for (k = 0; k < height*width; k++)
    outpixels[k] = inpixels[k];

  /* restore image size parameters */
  if (ImageUpdateFlag && !ChildProcess) {
    ZoomFactor = zf;
    DynamicRange(outpixels, ImgHeight*ImgWidth, &ProcMax, &ProcMin);
    ResizeProcessedImageWidgets(B_FALSE);
    CreateProcImage();
  }

  /* restore message banner drawing */
  DrawMesgEnabled = TmpFlag;
}
 
void PatchXCorr(double *inpixels, double *outpixels,
		int height, int width, void *ParmPtrs[])
{
  FILE *fp;
  DOUBLE_ARRAY *parameters;
  void *parms[4];
  boolean_t TmpFlag;
  double WindowRadiusMin, WindowRadiusMax, *ImgPatch, *objsfuncs, *VAFs,
  *shiftsX, *ReTmp, *ImTmp, *dxs, *dys, *buf1, *buf2, *buf3, *buf4, *buf5,
  *buf6, *X, *Y, *Z, CameraOffset, Multiplier, tm, tm0, ShiftXReturn, m,
  ObjFuncsReturn, VafsReturn, DxsReturn, DysReturn, ShiftYReturn, radius2,
  *shiftsY, val, max, min, scale, MinShiftX, MinShiftY, MaxShiftX, MaxShiftY,
  TotalWeights, *mask, RadiusSqrd, RMaxSqrd, RMinSqrd, XX, YY;
  int i1, i2, j1, j2, k, ii, jj, XStartI1, YStartI1, YStartI2, XStartI2, lags,
  kernw, Xextent, Yextent, sampling, XOffset, x, y, zf, zero, cc,
  YOffset, IFAIL, index, rc, ScanType, Nrequests, BufferSize,
  lFFTWindow, ResultsType, decimation, FFTCacheSize;
  char fname[256];
  
  /* check that stack is not empty */
  if (ImageStackTop <= 0) {
    Abort = B_TRUE;
    draw_mesg("Abort : One stack image required in StereoShiftScan()");
    return;
  }
  
  /* check that height and width match the current image size */
  if   ((ImageStack[ImageStackTop-1].height != height)
	|| (ImageStack[ImageStackTop-1].width != width)) {
    Abort = B_TRUE;
    draw_mesg("Abort: Stack image size mismatch in PatchXCorr(()");
    return;
  }
  
  /* load input parameters */
  XStartI1 = *((int *) ParmPtrs[1]);
  YStartI1 = *((int *) ParmPtrs[2]);
  parameters = (DOUBLE_ARRAY *) ParmPtrs[3];
  if (parameters->n < 11) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort : 11 extra parms in PatchXCorr()");
    draw_mesg(cbuf);
    return;
  }
  XOffset = (parameters->data)[0];
  YOffset = (parameters->data)[1];
  Xextent = (parameters->data)[2];
  Yextent = (parameters->data)[3];
  lags = (parameters->data)[4];
  decimation = (parameters->data)[5];
  kernw = (parameters->data)[6];
  WindowRadiusMax = (parameters->data)[7];
  WindowRadiusMin = (parameters->data)[8];
  ScanType = (parameters->data)[9];
  ResultsType = (parameters->data)[10];
  sampling = 1;
  FFTCacheNItems = 0;
  FFTCacheSize = FFT_CACHE_SIZE_MAX;
  FFTCacheHitsNum = 0;
  FFTCacheRequestsNum = 0;
  lFFTWindow = FFTWindow;
  m = -99.0;
  zf = ZoomFactor;

  /* check that stereo parameters have been initialized */
  if (((StereoMMPerPixel == -1) || (StereoFocalLength == -1)
       || (StereoCameraSeparation == -1)) && (ResultsType == 0)) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort : stereo unitiliazed in PatchXCorr()");
    draw_mesg(cbuf);
    return;
  }

  /* generate the scanning offsets, if required */
  if ((XOffset == -999) || (YOffset == -999)) {
    /* copy the area of interest to a temporary buffer */
    for (jj = YStartI1, k = 0; jj < (YStartI1 + Yextent); jj++) {
      for (ii = XStartI1; ii < (XStartI1 + Xextent); ii++, k++) {
	outpixels[k] = inpixels[jj*width + ii];
	DoubleBuffer[k] = (ImageStack[ImageStackTop-1].pixels)[jj*width + ii];
      }
    }
    
    /* determine gross image shift using cross-correlation and Laplacian */
    XvgImgStack_Push(DoubleBuffer, Yextent, Xextent);
    FFTWindow = NOWINDOW;
    zero = 0;
    parms[0] = (void *) &zero;
    CrossCorrelation(outpixels, DoubleBuffer, Yextent, Xextent, parms);
#ifdef DEBUG
    sprintf(fname, "%s_xc.mat", ParmPtrs[0]);
    WriteMatlabFile(Yextent, Xextent, "xc", DoubleBuffer, fname);
#endif
    Laplacian(DoubleBuffer, outpixels, Yextent, Xextent, NULL);
    if (Abort)
      return;

    /* find max in the Laplacian and use as estimate of gross shift */
    if ((Xextent > 8) && (Yextent > 8)) {
      for (jj = 4, m = -MAXDOUBLE; jj < (Yextent-4); jj++) {
	for (ii = 4; ii < (Xextent-4); ii++) {
	  if (outpixels[jj*Xextent + ii] > m) {
	    m = outpixels[jj*Xextent + ii];
	    x = ii;
	    y = jj;
	  }
	}
      }
    }
    else {
      x = 0;
      y = 0;
    }
    XOffset = rint(((double) Xextent)/2.0) - x;
    YOffset = rint(((double) Yextent)/2.0) - y;
    if (VerboseFlag)
      printf("PatchXCorr(): Shift estimate is (%d,%d) pixels\n",
	     XOffset, YOffset);
  }

  /* adjust input parameters for the reference image, if required */
  val = XStartI1 - (lags + kernw/2 + 2);
  if (val < 0) {
    XStartI1 += (-val);
    Xextent  -= (-val);
  }
  val = YStartI1 - (lags + kernw/2 + 2);
  if (val < 0) {
    YStartI1 += (-val);
    Yextent  -= (-val);
  }
  val = XStartI1 + (lags + kernw/2 + 2) + Xextent;
  if (val > (width - 1)) {
    Xextent -= (val - (width - 1));
  }
  val = YStartI1 + (lags + kernw/2 + 2) + Yextent;
  if (val > (height - 1)) {
    Yextent -= (val - (height - 1));
  }

  /* set remaining scanning bounds */
  XStartI2 = XStartI1 + XOffset;
  YStartI2 = YStartI1 + YOffset;

  /* adjust input parameters for the scanned image, if required */
  val = XStartI2 - (lags + kernw/2 + 2);
  if (val < 0) {
    XStartI1 += (-val);
    XStartI2 += (-val);
    Xextent  -= (-val);
  }
  val = YStartI2 - (lags + kernw/2 + 2);
  if (val < 0) {
    YStartI1 += (-val);
    YStartI2 += (-val);
    Yextent  -= (-val);
  }
  val = XStartI2 + (lags + kernw/2 + 2) + Xextent;
  if (val > (width - 1)) {
    Xextent -= (val - (width - 1));
  }
  val = YStartI2 + (lags + kernw/2 + 2) + Yextent;
  if (val > (height - 1)) {
    Yextent -= (val - (height - 1));
  }

  /* adjust stereo reconstruction parameters */
  if (XOffset > 0) {
    Multiplier = 1.0;
    CameraOffset = - StereoCameraSeparation/2.0;
  }
  else {
    Multiplier = -1.0;
    CameraOffset = StereoCameraSeparation/2.0;
  }

  /* show parameters, if required */
  if (VerboseFlag) {
    printf("PatchXCorr() :\n");
    printf("  InFileName = %s\n", InFileName);
    printf("  XStartI1 = %d\n", XStartI1);
    printf("  YStartI1 = %d\n", YStartI1);
    printf("  XStartI2 = %d\n", XStartI2);
    printf("  YStartI2 = %d\n", YStartI2);
    printf("  Xextent = %d\n", Xextent);
    printf("  Yextent = %d\n", Yextent);
    printf("  Xoffset = %d\n", XOffset);
    printf("  Yoffset = %d\n", YOffset);
    printf("  lags = %d\n", lags);
    printf("  sampling = %d\n", sampling);
    printf("  decimation = %d\n", decimation);
    printf("  kernw = %d\n", kernw);
    printf("  WindowRadiusMax = %e\n", WindowRadiusMax);
    printf("  WindowRadiusMin = %e\n", WindowRadiusMin);
    printf("  scan type = %d\n", ScanType);
    if (ResultsType == 0) {
      printf("  StereoMMPerPixel = %e\n", StereoMMPerPixel);
      printf("  StereoFocalLength = %e\n", StereoFocalLength);
      printf("  StereoCameraSeparation = %e\n", StereoCameraSeparation);
    }
    printf("  FFT kernel size %dx%d\n", kernw, kernw);
    printf("  results type = %d\n", ResultsType);
    printf("  Laplacian max = %e\n", m);
  }

  /* check input parameters */
  if (((XStartI1 - lags - kernw/2 - 1) < 0)
      || ((YStartI1 - lags - kernw/2 - 1) < 0)
      || !((XStartI1 + Xextent + lags + kernw/2 + 1) < width)
      || !((YStartI1 + Yextent + lags + kernw/2 + 1) < height)) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort: image size (%d, %d) exceeded for I1 X:[%d, %d], Y:[%d, %d] in StereoShiftImgScan()",
	    width, height,
	    XStartI1 - lags - kernw/2 - 1,
	    XStartI1 + Xextent + lags + kernw/2 + 1,
	    YStartI1 - lags - kernw/2 - 1,
	    YStartI1 + Yextent + lags + kernw/2 + 1);
    draw_mesg(cbuf);
    return;
  }
  if (((XStartI2 - lags - kernw/2 - 1) < 0)
      || ((YStartI2 - lags - kernw/2 - 1) < 0)
      || !((XStartI2 + Xextent + lags + kernw/2 + 1) < width)
      || !((YStartI2 + Yextent + lags + kernw/2 + 1) < height)) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort: image size (%d, %d) exceeded for I2 X:[%d, %d], Y:[%d, %d] in StereoShiftImgScan()",
	    width, height,
	    XStartI2 - lags - kernw/2 - 1,
	    XStartI2 + Xextent + lags + kernw/2 + 1,
	    YStartI2 - lags - kernw/2 - 1,
	    YStartI2 + Yextent + lags + kernw/2 + 1);
    draw_mesg(cbuf);
    return;
  }
  
  /* Normalize images. This improves performance slightly */
  draw_mesg("PatchXCorr() : Normalizing...");
  for (k = 0, max = -MAXDOUBLE, min = MAXDOUBLE; k < height*width; k++) {
    if (inpixels[k] < min)
      min = inpixels[k];
    if ((ImageStack[ImageStackTop-1].pixels)[k] < min)
      min = (ImageStack[ImageStackTop-1].pixels)[k];
    if (inpixels[k] > max)
      max = inpixels[k];
    if ((ImageStack[ImageStackTop-1].pixels)[k] > max)
      max = (ImageStack[ImageStackTop-1].pixels)[k];
  }
  scale = 1.0 / (max - min);
  for (k = 0; k < height*width; k++) {
    inpixels[k] = (inpixels[k] - min) * scale;
    outpixels[k] = ((ImageStack[ImageStackTop-1].pixels)[k] - min) * scale;
  }

  /* write out the text output file */
  sprintf(cbuf, "%s_README.doc", ParmPtrs[0]);
  if ((fp = fopen(cbuf, "w")) == NULL) {
    sprintf(cbuf, "PatchXCorr() : Could not open the file %s",
	    cbuf);
    draw_mesg(cbuf);
    return;
  }
  fprintf(fp, "InFileName = %s\n", InFileName);
  fprintf(fp, "XStartI1 = %d\n", XStartI1);
  fprintf(fp, "YStartI1 = %d\n", YStartI1);
  fprintf(fp, "XStartI2 = %d\n", XStartI2);
  fprintf(fp, "YStartI2 = %d\n", YStartI2);
  fprintf(fp, "Xoffset = %d\n", XOffset);
  fprintf(fp, "Yoffset = %d\n", YOffset);
  fprintf(fp, "Xextent = %d\n", Xextent);
  fprintf(fp, "Yextent = %d\n", Yextent);
  fprintf(fp, "lags = %d\n", lags);
  fprintf(fp, "sampling = %d\n", sampling);
  fprintf(fp, "decimation = %d\n", decimation);
  fprintf(fp, "kernw = %d\n", kernw);
  fprintf(fp, "WindowRadiusMax = %e\n", WindowRadiusMax);
  fprintf(fp, "WindowRadiusMin = %e\n", WindowRadiusMin);
  fprintf(fp, "ScanType = %d\n", ScanType);
  fprintf(fp, "ResultsType = %d\n", ResultsType);
  if (ResultsType == 0) {
    fprintf(fp, "StereoMMPerPixel = %e\n", StereoMMPerPixel);
    fprintf(fp, "StereoFocalLength = %e\n", StereoFocalLength);
    fprintf(fp, "StereoCameraSeparation = %e\n", StereoCameraSeparation);
  }
  fprintf(fp, "Laplacian max = %e\n", m);
  fprintf(fp, "FFT Cache size = %d items (%d Kbytes)\n",
	  FFTCacheSize, (2*kernw*kernw*sizeof(double)*FFTCacheSize)/1000);
  fprintf(fp, "Number of cache requests = %d\n", FFTCacheRequestsNum);
  fprintf(fp, "Number of cache hits = %d (%d%% hit ratio)\n",
	  FFTCacheHitsNum, 100*FFTCacheHitsNum/FFTCacheRequestsNum);
  fclose(fp);
  
  /* allocate storage */
  draw_mesg("PatchXCorr() : Allocating memory...");
  BufferSize = (2*lags +1)*(2*lags +1);
  mask = (double *) XvgMM_Alloc((void *) NULL, kernw*kernw*sizeof(double));
  FFTCacheTable = (short *) XvgMM_Alloc((void *) NULL, 
					height*width*sizeof(short));
  FFTCache = (FFT_CACHE_ITEM *)
    XvgMM_Alloc((void *) NULL, FFTCacheSize*sizeof(FFT_CACHE_ITEM));
  FFTCacheRequests =
    (FFT_CACHE_REQUEST *) XvgMM_Alloc((void *) NULL,
				      BufferSize*sizeof(FFT_CACHE_REQUEST));
  FFTCacheBuffer =
    (double *) XvgMM_Alloc((void *) NULL, (kernw*kernw)*sizeof(double));
  buf1 = (double *) XvgMM_Alloc((void *) NULL, BufferSize*sizeof(double));
  buf2 = (double *) XvgMM_Alloc((void *) NULL, BufferSize*sizeof(double)); 
  buf3 = (double *) XvgMM_Alloc((void *) NULL, BufferSize*sizeof(double));
  buf4 = (double *) XvgMM_Alloc((void *) NULL, BufferSize*sizeof(double));
  buf5 = (double *) XvgMM_Alloc((void *) NULL, BufferSize*sizeof(double));
  buf6 = (double *) XvgMM_Alloc((void *) NULL, BufferSize*sizeof(double));
  dxs  = (double *) XvgMM_Alloc((void *) NULL,Xextent*Yextent*sizeof(double));
  dys  = (double *) XvgMM_Alloc((void *) NULL,Xextent*Yextent*sizeof(double));
  objsfuncs = (double *) XvgMM_Alloc((void *) NULL,
				     Xextent*Yextent*sizeof(double));
  VAFs = (double *) XvgMM_Alloc((void *) NULL,Xextent*Yextent*sizeof(double));
  shiftsX = (double *) XvgMM_Alloc((void *) NULL,
				  Xextent*Yextent*sizeof(double));
  shiftsY = (double *) XvgMM_Alloc((void *) NULL,
				  Xextent*Yextent*sizeof(double));
  X = (double *) XvgMM_Alloc((void *) NULL, Xextent*Yextent*sizeof(double));
  Y = (double *) XvgMM_Alloc((void *) NULL, Xextent*Yextent*sizeof(double));
  Z = (double *) XvgMM_Alloc((void *) NULL, Xextent*Yextent*sizeof(double));
  ImgPatch = (double *) XvgMM_Alloc((void *) NULL, kernw*kernw*sizeof(double));
  if ((buf1 == NULL) || (buf2 == NULL) || (buf3 == NULL) || (buf4 == NULL)
      || (buf5 == NULL) || (buf6 == NULL) || (mask == NULL)
      || (dxs == NULL) || (dys == NULL) || (objsfuncs == NULL)
      || (VAFs == NULL) || (shiftsY == NULL) 
      || (shiftsX == NULL) || (X == NULL) || (Y == NULL) || (Z == NULL)
      || (ImgPatch == NULL) || (FFTCacheRequests == NULL)
      || (FFTCacheBuffer == NULL)
      || (FFTCacheTable == NULL)|| (FFTCache == NULL)) {
    Abort = B_TRUE;
    draw_mesg("Abort: can't allocate for buffers in StereoShiftImgScan()");
    return;
  }

  /* fill the valid pixel mask */
  RMaxSqrd=WindowRadiusMax*WindowRadiusMax;
  RMinSqrd=WindowRadiusMin*WindowRadiusMin;
  cc = kernw/2;
  for (y = 0, k = 0; y < kernw; y++) {
    YY = y - cc;
    for (x = 0; x < kernw; x++, k++) {
      XX = x - cc;
      RadiusSqrd = XX*XX + YY*YY;
      if ((RadiusSqrd < RMaxSqrd) && (RadiusSqrd > RMinSqrd) && (XX >= 0)) {
	mask[k] = 1.0;
      }
      else {
	mask[k] = 0.0;
      }
    }
  }
  
  /* init the FFT cache */
  for (k = 0; k < height*width; k++)
    FFTCacheTable[k] = -1;
  for (k = 0; k < FFTCacheSize; k++)
    FFTCache[k].index = -1;
  
  /* set temporary image size parameters */
  ImgWidth = kernw;
  ImgHeight = kernw;
  TmpFlag = DrawMesgEnabled;

  /* debugging */
  if (ImageUpdateFlag && !ChildProcess) {
    ZoomFactor = 8;
    ResizeProcessedImageWidgets(B_FALSE);
    CreateProcImage();
  }

  /* init buffers */
  if (FFTInit(kernw, kernw) == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort : FFTInit() failed in StereoShiftScan()");
    RestoreImageSize(zf, width, height, TmpFlag, lFFTWindow,
		     inpixels, outpixels);
    return;
  }
  if (FFTInit2(kernw, kernw) == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort : FFTInit2() failed in StereoShiftScan()");
    RestoreImageSize(zf, width, height, TmpFlag, lFFTWindow,
		     inpixels, outpixels);
    return;
  }

  /* set the FFT window type */
  FFTWindow = HANNINGWINDOW;

  /* loop to scan image by line */
  for (j1 = YStartI1, j2 = YStartI2, tm0 = XvgGetTime(),
       MinShiftX = MAXDOUBLE, MinShiftY = MAXDOUBLE, 
       MaxShiftX = -MAXDOUBLE, MaxShiftY = -MAXDOUBLE,
       AverageShiftX = 0, AverageShiftY = 0, TotalWeights = 0;
       (j1 < (YStartI1+Yextent)) && (Abort == B_FALSE);
       j1++, j2++) {

    /* check that this line is on the decimation grid, if so process it */
    if ((j1 & (decimation-1)) == 0) { 

      /* show progress */
      tm = XvgGetTime();
      sprintf(cbuf,"PatchXCorr() at line %d/%d (%.2f seconds/line)..",
	      j1, YStartI1+Yextent - 1, tm - tm0);
      draw_mesg(cbuf);
      TmpFlag = DrawMesgEnabled;
      DrawMesgEnabled = B_FALSE;
      tm0 = tm;
      
      /* loop for pixels */
      for (i1 = XStartI1, i2 = XStartI2;
	   (i1 < (XStartI1+Xextent)) && (Abort == B_FALSE); i1++, i2++) {
	
	/* check that this pixel is on the decimation grid, if so process it */
	if ((i1 & (decimation-1)) == 0) { 
	  
	  /* fetch the kernel from the reference image, centered at (i1,j1) */
	  for (y = j1 - kernw/2, k = 0; y < (j1 + kernw/2); y += sampling)
	    for (x = i1 - kernw/2; x < (i1 + kernw/2); x += sampling, k++)
	      ImgPatch[k] = inpixels[y*width + x];
	  
	  /* calculate the kernel's FT */
/*	  SubtractMean(ImgPatch, ImgPatch, kernw, kernw, NULL); */
	  FFTApplyWindow(ImgPatch, FFTRealBuffer, FFTImBuffer);

#ifdef _NRC
	  NRC_FFT(FFTRealBuffer, FFTImBuffer, kernw, kernw, 1);
#else
	  IFAIL = 1;
	  c06fuf(&kernw, &kernw, FFTRealBuffer, FFTImBuffer,
		 "Initial", FFTTrigm, FFTTrign, FFTWork, &IFAIL);
	  if (IFAIL != 0) {
	    Abort = B_TRUE;
	    sprintf(cbuf,
		    "Abort:IFAIL=%d on from c06fuf() in PatchXCorr()",
		    IFAIL);
	    draw_mesg(cbuf);
	    RestoreImageSize(zf, width, height, TmpFlag, lFFTWindow,
			     inpixels, outpixels);
	    return;
	  }
	  FFTShift(FFTRealBuffer, FFTRealBuffer, kernw, kernw);
	  FFTShift(FFTImBuffer, FFTImBuffer, kernw, kernw);
#endif
	  
	  /* load list of FFT cache requests for scanning around this pixel */
	  switch (ScanType) {
	  case 0:                      /* line scan +/- lags on either size */
	    for (ii = i2 - lags, Nrequests = 0; ii <= (i2 + lags);
		 ii++, Nrequests++) {
	      FFTCacheRequests[Nrequests].x = ii;
	      FFTCacheRequests[Nrequests].y = j2;
	    }
	    break;
	  case 1:             /* circular area scan around the center pixel */
	  default:
	    for (jj = j2 - lags, Nrequests = 0; jj <= (j2 + lags); jj++) {
	      for (ii = i2 - lags; ii <= (i2 + lags); ii++) {
		radius2 = (jj - j2)*(jj - j2) + (ii - i2)*(ii - i2);
		if (radius2 <= (lags*lags)) {
		  FFTCacheRequests[Nrequests].x = ii;
		  FFTCacheRequests[Nrequests].y = jj;
		  Nrequests++;
		}
	      }
	    }
	    break;
	  }
	  
	  /* find the pixel shift with the lowest objective function */
	  if ((rc = KernelScan(FFTCacheRequests, FFTCacheSize, outpixels,
			       mask, width, Nrequests, i1, j1, kernw, 
			       buf1, buf2, buf3, buf4, buf5, buf6,
			       &DxsReturn, &DysReturn,
			       &ShiftXReturn, &ShiftYReturn, 
			       &ObjFuncsReturn, &VafsReturn)) < 0) {
	    Abort = B_TRUE;
	    sprintf(cbuf,
		    "Abort:on retrn from KernelScan() in PatchXCorr()");
	    draw_mesg(cbuf);
	    RestoreImageSize(zf, width, height, TmpFlag, lFFTWindow,
			     inpixels, outpixels);
	    return;
	  }
	  FFTCacheRequestsNum += Nrequests;
	  FFTCacheHitsNum += rc;
	  
	  /* store results for this pixel in the output arrays */
	  index = (j1 - YStartI1)*Xextent + (i1 - XStartI1);
	  dxs[index] = DxsReturn;
	  dys[index] = DysReturn;
	  shiftsX[index] = ShiftXReturn;
	  shiftsY[index] = ShiftYReturn;
	  objsfuncs[index] = ObjFuncsReturn;
	  VAFs[index] = VafsReturn;
	  if (ResultsType == 0) {
	    if (finite(ShiftXReturn) && (ShiftXReturn != 0.0)) {
	      X[index] = (StereoCameraSeparation * ((double) (i1 - width/2)))
		/ ShiftXReturn * Multiplier + CameraOffset;
	      Y[index] = (StereoCameraSeparation * ((double) (j1 - height/2)))
		/ ShiftXReturn * Multiplier;
	      Z[index] = (StereoCameraSeparation * StereoFocalLength)
		/ (ShiftXReturn * StereoMMPerPixel) * Multiplier;
	    }
	    else {
	      X[index] = -1;
	      Y[index] = -1;
	      Z[index] = -1;
	    }
	  }
	  
	  /* update stats */
	  if (VafsReturn > 0.0) {
	    AverageShiftX += (ShiftXReturn * VafsReturn);
	    AverageShiftY += (ShiftYReturn * VafsReturn);
	    TotalWeights  += (VafsReturn);
	  }
	  if (ShiftXReturn < MinShiftX)
	    MinShiftX = ShiftXReturn;
	  if (ShiftXReturn > MaxShiftX)
	    MaxShiftX = ShiftXReturn;
	  if (ShiftYReturn < MinShiftY)
	    MinShiftY = ShiftYReturn;
	  if (ShiftYReturn > MaxShiftY)
	    MaxShiftY = ShiftYReturn;
	}
      } /* end loop for pixels */
      
#ifdef DEBUG
      /* write file to indicate progress */
      sprintf(fname, "%s_progress.doc", ParmPtrs[0]);
      if ((fp = fopen(fname, "w")) == NULL) {
	sprintf(cbuf, "PatchXCorr() : Could not open the file %s",
		fname);
	draw_mesg(cbuf);
	RestoreImageSize(zf, width, height, TmpFlag, lFFTWindow,
			 inpixels, outpixels);
	return;
      }
      fprintf(fp, "Processed line %d [%d,%d] (Execution time = %fs).\n",
	      j1, YStartI1, YStartI1+Yextent, XvgGetTime() - tm0);
      fprintf(fp, "MinShiftX = %e\n", MinShiftX);
      fprintf(fp, "MaxShiftX = %e\n", MaxShiftX);
      fprintf(fp, "MinShiftY = %e\n", MinShiftY);
      fprintf(fp, "MaxShiftY = %e\n", MaxShiftY);
      fclose(fp);
#endif
      
      /* re-enable text output */
      DrawMesgEnabled = TmpFlag;
    }
  } /* end loop for lines */
  
#ifdef DEBUG
  /* delete the progress text file */
  sprintf(fname, "%s_progress.doc", ParmPtrs[0]);
  unlink(fname);
#endif

  /* update the stats */
  AverageShiftX /= TotalWeights;
  AverageShiftY /= TotalWeights;

  /* write the output files */
  if (Abort == B_FALSE) {
    sprintf(fname, "%s_objsfuncs.mat", ParmPtrs[0]);
    if (WriteMatlabFile(Yextent, Xextent, "objsfuncs", objsfuncs, fname)
	== B_FALSE) {
      Abort = B_TRUE;
      sprintf(cbuf, "Abort: could not open \"%s\" in PatchXCorr()",
	      fname);
      draw_mesg(cbuf);
      RestoreImageSize(zf, width, height, TmpFlag, lFFTWindow,
		       inpixels, outpixels);
      return;
    }
    sprintf(fname, "%s_VAFs.mat", ParmPtrs[0]);
    if (WriteMatlabFile(Yextent, Xextent, "VAFs", VAFs, fname) == B_FALSE) {
      Abort = B_TRUE;
      sprintf(cbuf, "Abort: could not open \"%s\" in PatchXCorr()",
	      fname);
      draw_mesg(cbuf);
      RestoreImageSize(zf, width, height, TmpFlag, lFFTWindow,
		       inpixels, outpixels);
      return;
    }
    sprintf(fname, "%s_shiftsX.mat", ParmPtrs[0]);
    if (WriteMatlabFile(Yextent, Xextent, "shiftsX", shiftsX, fname)
	== B_FALSE) {
      Abort = B_TRUE;
      sprintf(cbuf, "Abort: could not open \"%s\" in PatchXCorr()",
	      fname);
      draw_mesg(cbuf);
      RestoreImageSize(zf, width, height, TmpFlag, lFFTWindow,
		       inpixels, outpixels);
      return;
    }
    sprintf(fname, "%s_shiftsY.mat", ParmPtrs[0]);
    if (WriteMatlabFile(Yextent, Xextent, "shiftsY", shiftsY, fname)
	== B_FALSE) {
      Abort = B_TRUE;
      sprintf(cbuf, "Abort: could not open \"%s\" in PatchXCorr()",
	      fname);
      draw_mesg(cbuf);
      RestoreImageSize(zf, width, height, TmpFlag, lFFTWindow,
		       inpixels, outpixels);
      return;
    }
    sprintf(fname, "%s_dxs.mat", ParmPtrs[0]);
    if (WriteMatlabFile(Yextent, Xextent, "dxs", dxs, fname) == B_FALSE) {
      Abort = B_TRUE;
      sprintf(cbuf, "Abort: could not open \"%s\" in PatchXCorr()",
	      fname);
      draw_mesg(cbuf);
      RestoreImageSize(zf, width, height, TmpFlag, lFFTWindow,
		       inpixels, outpixels);
      return;
    }
    sprintf(fname, "%s_dys.mat", ParmPtrs[0]);
    if (WriteMatlabFile(Yextent, Xextent, "dys", dys, fname) == B_FALSE) {
      Abort = B_TRUE;
      sprintf(cbuf, "Abort: could not open \"%s\" in PatchXCorr()",
	      fname);
      draw_mesg(cbuf);
      RestoreImageSize(zf, width, height, TmpFlag, lFFTWindow,
		       inpixels, outpixels);
      return;
    }

    /* save X/Y/Z calculations of required */
    if (ResultsType == 0) {
      sprintf(fname, "%s_X.mat", ParmPtrs[0]);
      if (WriteMatlabFile(Yextent, Xextent, "X", X, fname) == B_FALSE) {
	Abort = B_TRUE;
	sprintf(cbuf, "Abort: could not open \"%s\" in PatchXCorr()",
		fname);
	draw_mesg(cbuf);
	RestoreImageSize(zf, width, height, TmpFlag, lFFTWindow,
			 inpixels, outpixels);
	return;
      }
      sprintf(fname, "%s_Y.mat", ParmPtrs[0]);
      if (WriteMatlabFile(Yextent, Xextent, "Y", Y, fname) == B_FALSE) {
	Abort = B_TRUE;
	sprintf(cbuf, "Abort: could not open \"%s\" in PatchXCorr()",
		fname);
	draw_mesg(cbuf);
	RestoreImageSize(zf, width, height, TmpFlag, lFFTWindow,
			 inpixels, outpixels);
	return;
      }
      sprintf(fname, "%s_Z.mat", ParmPtrs[0]);
      if (WriteMatlabFile(Yextent, Xextent, "Z", Z, fname) == B_FALSE) {
	Abort = B_TRUE;
	sprintf(cbuf, "Abort: could not open \"%s\" in PatchXCorr()",
		fname);
	draw_mesg(cbuf);
	RestoreImageSize(zf, width, height, TmpFlag, lFFTWindow,
			 inpixels, outpixels);
	return;
      }
    }
      
    /* write out the text output file */
    sprintf(cbuf, "%s_README.doc", ParmPtrs[0]);
    if ((fp = fopen(cbuf, "w")) == NULL) {
      sprintf(cbuf, "PatchXCorr() : Could not open the file %s",
	      cbuf);
      draw_mesg(cbuf);
      RestoreImageSize(zf, width, height, TmpFlag, lFFTWindow,
		       inpixels, outpixels);
      return;
    }
    fprintf(fp, "InFileName = %s\n", InFileName);
    fprintf(fp, "XStartI1 = %d\n", XStartI1);
    fprintf(fp, "YStartI1 = %d\n", YStartI1);
    fprintf(fp, "XStartI2 = %d\n", XStartI2);
    fprintf(fp, "YStartI2 = %d\n", YStartI2);
    fprintf(fp, "Xoffset = %d\n", XOffset);
    fprintf(fp, "Yoffset = %d\n", YOffset);
    fprintf(fp, "Xextent = %d\n", Xextent);
    fprintf(fp, "Yextent = %d\n", Yextent);
    fprintf(fp, "lags = %d\n", lags);
    fprintf(fp, "sampling = %d\n", sampling);
    fprintf(fp, "decimation = %d\n", decimation);
    fprintf(fp, "kernw = %d\n", kernw);
    fprintf(fp, "WindowRadiusMax = %e\n", WindowRadiusMax);
    fprintf(fp, "WindowRadiusMin = %e\n", WindowRadiusMin);
    fprintf(fp, "ScanType = %d\n", ScanType);
    fprintf(fp, "ResultsType = %d\n", ResultsType);
    if (ResultsType == 0) {
      fprintf(fp, "StereoMMPerPixel = %e\n", StereoMMPerPixel);
      fprintf(fp, "StereoFocalLength = %e\n", StereoFocalLength);
      fprintf(fp, "StereoCameraSeparation = %e\n", StereoCameraSeparation);
    }
    fprintf(fp, "AverageShiftX = %f\n", AverageShiftX);
    fprintf(fp, "AverageShiftY = %f\n", AverageShiftY);
    fprintf(fp, "MinShiftX = %f\n", MinShiftX);
    fprintf(fp, "MaxShiftX = %f\n", MaxShiftX);
    fprintf(fp, "MinShiftY = %f\n", MinShiftY);
    fprintf(fp, "MaxShiftY = %f\n", MaxShiftY);
    fprintf(fp, "Laplacian max = %e\n", m);
    fprintf(fp, "FFT Cache size = %d items (%d Kbytes)\n",
	    FFTCacheSize, (2*kernw*kernw*sizeof(double)*FFTCacheSize)/1000);
    fprintf(fp, "Number of cache requests = %d\n", FFTCacheRequestsNum);
    fprintf(fp, "Number of cache hits = %d (%d%% hit ratio)\n",
	    FFTCacheHitsNum, 100*FFTCacheHitsNum/FFTCacheRequestsNum);
    fclose(fp);
  }
  
  /* restore image parameters */
  RestoreImageSize(zf, width, height, TmpFlag, lFFTWindow,
		   inpixels, outpixels);
  
  /* free storage */
  for (k = 0; k < FFTCacheNItems; k++) {
    XvgMM_Free(FFTCache[k].FFTReal);
    XvgMM_Free(FFTCache[k].FFTIm);
  }
  XvgMM_Free(FFTCache);
  XvgMM_Free(FFTCacheBuffer);
  XvgMM_Free(FFTCacheTable);
  XvgMM_Free(FFTCacheRequests);
  XvgMM_Free(X);
  XvgMM_Free(Y);
  XvgMM_Free(Z);
  XvgMM_Free(dxs);
  XvgMM_Free(dys);
  XvgMM_Free(ImgPatch);
  XvgMM_Free(VAFs);
  XvgMM_Free(shiftsX);
  XvgMM_Free(shiftsY);
  XvgMM_Free(objsfuncs);
  XvgMM_Free(buf1);
  XvgMM_Free(buf2);
  XvgMM_Free(buf3);
  XvgMM_Free(buf4);
  XvgMM_Free(buf5);
  XvgMM_Free(buf6);
}


/***************************************************************************
*
*   function : PatchXCorrHierarchical()
***************************************************************************/
static void DecrementParentProcessSemaphore()
{
  struct sembuf SemOperation;

  SemOperation.sem_num = nprocessors;
  SemOperation.sem_op  = -1;
  SemOperation.sem_flg = 0;
  if (semop(SemID, &SemOperation, 1) == -1)
    Abort = B_TRUE;
}

static void PatchXCorrHierarchical(double *inpixels, double *outpixels,
				   int height, int width, char *OutFname,
				   int XMin, int XMax, int YMin, int YMax,
				   int XOffset, int YOffset,
				   double WindowRadiusMin,
				   int *Decimations, int *LagsX, int *LagsY,
				   int *FTKernelWidths, int *SamplingIncs,
				   double *WindowRadiiMax, int *FFTCacheSizes,
				   int *WriteSecondaryFiles,
				   int HierarchicalPassesN)
{
  struct sembuf SemOperation;
  FILE *fp;
  boolean_t TmpFlag, FFTCacheInitialized;
  double *ImgPatch, *objsfuncs, *VAFs, *shiftsX, *dxs, *dys, X, Y,
  *buf1, *buf2, *buf3, *buf4, *buf5, *buf6, tm, tm0, ShiftXReturn, tstart,
  *PreviousShiftsY, *PreviousShiftsX, ObjFuncsReturn, VafsReturn, DxsReturn,
  DysReturn, ShiftYReturn, *shiftsY, val, lmin, lmax, ptime, *mask, RadiusSqrd,
  RMaxSqrd, RMinSqrd;
  int i1, i2, j1, j2, k, ii, jj, x, y, zero, IFAIL, index, rc, Nrequests,
  BufferSize, lFFTWindow, lheight, lwidth, HierarchicalPass, zf,
  XStartI1, YStartI1, XExtent, YExtent, NPixelsMatched;
  char fname[256];
  
  /* init internal parameters */
  zf = ZoomFactor;
  lFFTWindow = FFTWindow;
  buf1 = NULL;
  buf2 = NULL;
  buf3 = NULL;
  buf4 = NULL;
  buf5 = NULL;
  buf6 = NULL;
  ImgPatch = NULL;
  PreviousShiftsX = NULL;
  PreviousShiftsY = NULL;
  dxs = NULL;
  dys = NULL;
  objsfuncs = NULL;
  VAFs = NULL;
  shiftsX = NULL;
  shiftsY = NULL;
  mask = NULL;

  /* disable message drawing */
  TmpFlag = DrawMesgEnabled;

  /* set the FFT window type */
  FFTWindow = HANNINGWINDOW;

  /* loop for all hierarchical refinement passes */
  for (HierarchicalPass = 0; (HierarchicalPass < HierarchicalPassesN)
       && (Abort == B_FALSE); HierarchicalPass++) {

    /* wait on this child's semaphore */
    if (VerboseFlag)
      printf("PatchXCorrHierarchical() : Child process %d waiting...\n",
	     ChildProcessNumber);
    SemOperation.sem_num = ChildProcessNumber;
    SemOperation.sem_op  = 0;
    SemOperation.sem_flg = 0;
    if (semop(SemID, &SemOperation, 1) == -1) {
      Abort = B_TRUE;
      sprintf(cbuf, "Abort(%d): \"Error %d : %s\" waiting on sem in PatchXCorrHierarchical()",
	      ChildProcessNumber, errno, strerror(errno));
      draw_mesg(cbuf);
      DecrementParentProcessSemaphore();
      RestoreImageSize(zf, width, height, TmpFlag, lFFTWindow,
		       inpixels, outpixels);
      return;
    }
    if (VerboseFlag)
      printf("PatchXCorrHierarchical() : Child process %d running...\n",
	     ChildProcessNumber);

    /* reset this child's semaphore to 1 so that semaphore wait operation */
    /* above stalls this process until the parent says ist ok to go again */
    SemOperation.sem_num = ChildProcessNumber;
    SemOperation.sem_op  = 1;
    SemOperation.sem_flg = 0;
    if (semop(SemID, &SemOperation, 1) == -1) {
      Abort = B_TRUE;
      sprintf(cbuf,
	      "Abort: \"Error %d : %s\" for semaphore %d in PatchXCorrHierarchical()",
	      errno, strerror(errno), ChildProcessNumber);
      draw_mesg(cbuf);
      DecrementParentProcessSemaphore();
      RestoreImageSize(zf, width, height, TmpFlag, lFFTWindow,
		       inpixels, outpixels);
      return;
    }

    /* get starting time fot this pass */
    tstart = XvgGetTime();

    /* calculate scanned image patch size parameters (remove edges) */
    if ((XMin  - FTKernelWidths[HierarchicalPass]/2) < 0)
      XStartI1 = FTKernelWidths[HierarchicalPass]/2;
    else
      XStartI1 = XMin;
    if ((XMax + FTKernelWidths[HierarchicalPass]/2) > width)
      XExtent = width  - FTKernelWidths[HierarchicalPass]/2 - XStartI1 + 1;
    else
      XExtent = XMax - XStartI1 + 1;
    if ((YMin - FTKernelWidths[HierarchicalPass]/2) < 0)
      YStartI1 = FTKernelWidths[HierarchicalPass]/2;
    else
      YStartI1 = YMin;
    if ((YMax + FTKernelWidths[HierarchicalPass]/2) > height)
      YExtent = height - FTKernelWidths[HierarchicalPass]/2 - YStartI1 + 1;
    else
      YExtent = YMax - YStartI1 + 1;

    /* set temporary image size parameters */
    ImgWidth = FTKernelWidths[HierarchicalPass];
    ImgHeight = FTKernelWidths[HierarchicalPass];

    /* debugging */
    if (ImageUpdateFlag && !(ChildProcess && (nprocessors > 1))) {
      ZoomFactor = (height*ZoomFactor)/ImgWidth;
      if (ZoomFactor > 8)
	ZoomFactor = 8;
      ResizeProcessedImageWidgets(B_FALSE);
      CreateProcImage();
    }

    /* init FFT buffers */
    if (FFTInit(FTKernelWidths[HierarchicalPass],
		FTKernelWidths[HierarchicalPass]) == B_FALSE) {
      Abort = B_TRUE;
      draw_mesg("Abort : FFTInit() failed in PatchXCorrHierarchical()");
      DecrementParentProcessSemaphore();
      RestoreImageSize(zf, width, height, TmpFlag, lFFTWindow,
		       inpixels, outpixels);
      return;
    }
    if (FFTInit2(FTKernelWidths[HierarchicalPass],
		 FTKernelWidths[HierarchicalPass]) == B_FALSE) {
      Abort = B_TRUE;
      draw_mesg("Abort : FFTInit2() failed in PatchXCorrHierarchical()");
      DecrementParentProcessSemaphore();
      RestoreImageSize(zf, width, height, TmpFlag, lFFTWindow,
		       inpixels, outpixels);
      return;
    }

    /* initialize the FFT Cache, if this is the first pass */
    if (HierarchicalPass == 0) {
      FFTCacheRequests =
	(FFT_CACHE_REQUEST *) XvgMM_Alloc((void *) FFTCacheRequests,
					  (2*LagsX[HierarchicalPass] +1)*
					  (2*LagsY[HierarchicalPass] +1)*
					  sizeof(FFT_CACHE_REQUEST));
      if ((InitFFTCache(FFTCacheSizes[HierarchicalPass], height, width,
			FTKernelWidths[HierarchicalPass]) == B_FALSE)
	  || (FFTCacheRequests == NULL)) {
	Abort = B_TRUE;
	draw_mesg("Abort : InitFFTCache() failed in PatchXCorrHierarchical()");
	DecrementParentProcessSemaphore();
	RestoreImageSize(zf, width, height, TmpFlag, lFFTWindow,
			 inpixels, outpixels);
	return;
      }
      FFTCacheInitialized = B_TRUE;
    }
    
    /* else initialize the FFT Cache, if the parameters have changed */
    else {
      /* init the FFTCacheRequests buffer */
      if ((FFTCacheRequests =
	   (FFT_CACHE_REQUEST *) XvgMM_Alloc((void *) FFTCacheRequests,
					     (2*LagsX[HierarchicalPass] +1)*
					     (2*LagsY[HierarchicalPass] +1)*
					     sizeof(FFT_CACHE_REQUEST)))
	  == NULL) {
	Abort = B_TRUE;
	draw_mesg("Abort:Aloc FFTCacheRequests fail PatchXCorrHierarchical()");
	DecrementParentProcessSemaphore();
	RestoreImageSize(zf, width, height, TmpFlag, lFFTWindow,
			 inpixels, outpixels);
	return;
      }
      
      /* initialize the FFT Cache, if the parameters have changed */
      if ((FFTCacheSizes[HierarchicalPass] !=
	   FFTCacheSizes[HierarchicalPass-1]) ||
	  (FTKernelWidths[HierarchicalPass] !=
	   FTKernelWidths[HierarchicalPass-1])) {
	
	/* Free the FFT Cache storage */
	FreeFFTCache(FFTCacheSizes[HierarchicalPass]);
	
	/* init cache */
	if (InitFFTCache(FFTCacheSizes[HierarchicalPass], height, width,
			 FTKernelWidths[HierarchicalPass]) == B_FALSE) {
	  Abort = B_TRUE;
	  draw_mesg("Abort:InitFFTCache() failed in PatchXCorrHierarchical()");
	  DecrementParentProcessSemaphore();
	  RestoreImageSize(zf, width, height, TmpFlag, lFFTWindow,
			   inpixels, outpixels);
	  return;
	}
	FFTCacheInitialized = B_TRUE;
      }
      else {
	FFTCacheInitialized = B_FALSE;
      }
    }
    
    /* allocate memory */
    BufferSize = (2*LagsX[HierarchicalPass] +1)*(2*LagsY[HierarchicalPass] +1);
    buf1 = (double *) XvgMM_Alloc((void *) buf1, BufferSize*sizeof(double));
    buf2 = (double *) XvgMM_Alloc((void *) buf2, BufferSize*sizeof(double)); 
    buf3 = (double *) XvgMM_Alloc((void *) buf3, BufferSize*sizeof(double));
    buf4 = (double *) XvgMM_Alloc((void *) buf4, BufferSize*sizeof(double));
    buf5 = (double *) XvgMM_Alloc((void *) buf5, BufferSize*sizeof(double));
    buf6 = (double *) XvgMM_Alloc((void *) buf6, BufferSize*sizeof(double));
    ImgPatch = (double *) XvgMM_Alloc((void *) ImgPatch,
				      FTKernelWidths[HierarchicalPass]
				      *FTKernelWidths[HierarchicalPass]
				      *sizeof(double));
    dxs  = (double *) XvgMM_Alloc((void *) dxs,
				  XExtent*YExtent*sizeof(double));
    dys  = (double *) XvgMM_Alloc((void *) dys,
				  XExtent*YExtent*sizeof(double));
    objsfuncs = (double *) XvgMM_Alloc((void *) objsfuncs,
				       XExtent*YExtent*sizeof(double));
    VAFs = (double *) XvgMM_Alloc((void *) VAFs,
				  XExtent*YExtent*sizeof(double));
    shiftsX = (double *) XvgMM_Alloc((void *) shiftsX,
				     XExtent*YExtent*sizeof(double));
    shiftsY = (double *) XvgMM_Alloc((void *) shiftsY,
				     XExtent*YExtent*sizeof(double));
    mask = (double *) XvgMM_Alloc((void *) mask,
				  FTKernelWidths[HierarchicalPass]
				  *FTKernelWidths[HierarchicalPass]
				  *sizeof(double));
    if ((buf1 == NULL) || (buf2 == NULL) || (buf3 == NULL) || (buf4 == NULL)
	|| (buf5 == NULL) || (buf6 == NULL) || (ImgPatch == NULL)
	|| (dxs == NULL) || (dys == NULL) || (objsfuncs == NULL)
	|| (VAFs == NULL) || (shiftsY == NULL) || (shiftsX == NULL)
	|| (mask == NULL)) {
      Abort = B_TRUE;
      sprintf(cbuf, "Abort: can't allocate in PatchXCorrHierarchical(pass:%d)",
	      HierarchicalPass);
      draw_mesg(cbuf);
      DecrementParentProcessSemaphore();
      RestoreImageSize(zf, width, height, TmpFlag, lFFTWindow,
		       inpixels, outpixels);
      return;
    }

    /* fill the valid pixel mask, use X >= 0 quadrants only... */
    RMaxSqrd=WindowRadiiMax[HierarchicalPass]*WindowRadiiMax[HierarchicalPass];
    RMinSqrd=WindowRadiusMin*WindowRadiusMin;
    for (y = 0, k = 0; y < FTKernelWidths[HierarchicalPass]; y++) {
      Y = y - FTKernelWidths[HierarchicalPass]/2;
      for (x = 0; x < FTKernelWidths[HierarchicalPass]; x++, k++) {
	X = x - FTKernelWidths[HierarchicalPass]/2;
	RadiusSqrd = X*X + Y*Y;
	if ((RadiusSqrd < RMaxSqrd) && (RadiusSqrd > RMinSqrd)
	    && (X >= 0)) {
	  mask[k] = 1.0;
	}
	else {
	  mask[k] = 0.0;
	}
      }
    }
    
    /* load the previous passe's xy field data */
    if (HierarchicalPass > 0) {
      sprintf(fname, "%s_shiftsX_INTERPOLATED_d%02d.mat",
	      OutFname, Decimations[HierarchicalPass-1]);
      if (GetMatlabFile(&lheight, &lwidth, &lmin, &lmax, &PreviousShiftsX,
			fname, VerboseFlag) == B_FALSE) {
	Abort = B_TRUE;
	sprintf(cbuf, "Abort : Can't open %s in PatchXCorrHierarchical()",
		fname);
	draw_mesg(cbuf);
	DecrementParentProcessSemaphore();
	RestoreImageSize(zf, width, height, TmpFlag, lFFTWindow,
			 inpixels, outpixels);
	return;
      }

      sprintf(fname, "%s_shiftsY_INTERPOLATED_d%02d.mat",
	      OutFname, Decimations[HierarchicalPass-1]);
      if (GetMatlabFile(&lheight, &lwidth, &lmin, &lmax, &PreviousShiftsY,
			fname, VerboseFlag) == B_FALSE) {
	Abort = B_TRUE;
	sprintf(cbuf, "Abort : Can't open %s in PatchXCorrHierarchical()",
		fname);
	draw_mesg(cbuf);
	DecrementParentProcessSemaphore();
	RestoreImageSize(zf, width, height, TmpFlag, lFFTWindow,
			 inpixels, outpixels);
	return;
      }
    }
    
    /* loop to scan image by line */
    for (j1 = YStartI1, tm0 = XvgGetTime(), NPixelsMatched = 0;
	 (j1 < (YStartI1+YExtent)) && (Abort == B_FALSE); j1++) {
      
      /* check that this line is on the decimation grid, if so process it */
      if ((j1 & (Decimations[HierarchicalPass]-1)) == 0) { 
	
	/* show progress */
	tm = XvgGetTime();
	sprintf(cbuf,
		"PatchXCorrHierarchical() at line %d/%d (%.2f seconds/line)..",
		j1, YStartI1+YExtent - 1, tm - tm0);
	draw_mesg(cbuf);
	TmpFlag = DrawMesgEnabled;
	DrawMesgEnabled = B_FALSE;
	tm0 = tm;
	
	/* loop for pixels */
	for (i1 = XStartI1;
	     (i1 < (XStartI1+XExtent)) && (Abort == B_FALSE); i1++) {

	  /* check that pixel is on the decimation grid, if so process it */
	  if ((i1 & (Decimations[HierarchicalPass]-1)) == 0) { 

	    /* current pixel index */
	    index = (j1 - YStartI1)*XExtent + (i1 - XStartI1);

	    /* process this pixel is it is not masked */
	    if (MaskPixels[j1*width + i1] != 0.0) {

	      /* update pixel count */
	      NPixelsMatched++;

	      /* fetch the kernel from reference image, centered at (i1,j1) */
	      for (y = j1 - FTKernelWidths[HierarchicalPass]/2, k = 0;
		   y < (j1 + FTKernelWidths[HierarchicalPass]/2); y++)
		for (x = i1 - FTKernelWidths[HierarchicalPass]/2;
		     x < (i1 + FTKernelWidths[HierarchicalPass]/2); x++, k++)
		  ImgPatch[k] = inpixels[y*width + x];
	      
	      /* calculate the kernel's FT */
/*
	      SubtractMean(ImgPatch, ImgPatch,
			   FTKernelWidths[HierarchicalPass],
			   FTKernelWidths[HierarchicalPass], NULL);
*/
	      FFTApplyWindow(ImgPatch, FFTRealBuffer, FFTImBuffer);

#ifdef _NRC
	      NRC_FFT(FFTRealBuffer, FFTImBuffer, 
		      FTKernelWidths[HierarchicalPass],
		      FTKernelWidths[HierarchicalPass], 1);
#else
	      IFAIL = 1;
	      c06fuf(&(FTKernelWidths[HierarchicalPass]),
		     &(FTKernelWidths[HierarchicalPass]),
		     FFTRealBuffer, FFTImBuffer,
		     "Initial", FFTTrigm, FFTTrign, FFTWork, &IFAIL);
	      if (IFAIL != 0) {
		Abort = B_TRUE;
		sprintf(cbuf,
			"Abort:IFAIL=%d, c06fuf() in PatchXCorrHierarchical()",
			IFAIL);
		draw_mesg(cbuf);
		DecrementParentProcessSemaphore();
		RestoreImageSize(zf, width, height, TmpFlag, lFFTWindow,
				 inpixels, outpixels);
		return;
	      }
	      FFTShift(FFTRealBuffer, FFTRealBuffer,
		       FTKernelWidths[HierarchicalPass],
		       FTKernelWidths[HierarchicalPass]);
	      FFTShift(FFTImBuffer, FFTImBuffer,
		       FTKernelWidths[HierarchicalPass],
		       FTKernelWidths[HierarchicalPass]);
#endif
	      
	      /* set search center to previous interpolated result */
	      if (HierarchicalPass == 0) {
		j2 = j1 - YOffset;
		i2 = i1 - XOffset;
	      }
	      else {
		j2 = j1 - PreviousShiftsY[j1*width + i1];
		i2 = i1 - PreviousShiftsX[j1*width + i1];
	      }
	      
	      /* load list of FFT cache requests for scanning around pixel */
	      for (jj = j2 - LagsY[HierarchicalPass], Nrequests = 0;
		   jj <= (j2 + LagsY[HierarchicalPass]);
		   jj += SamplingIncs[HierarchicalPass]) {
		if ((jj > (FTKernelWidths[HierarchicalPass]/2))
		    && (jj < (height - FTKernelWidths[HierarchicalPass]/2))) {
		  for (ii = i2 - LagsX[HierarchicalPass];
		       ii <= (i2 + LagsX[HierarchicalPass]);
		       ii += SamplingIncs[HierarchicalPass]) {
		    if ((ii > (FTKernelWidths[HierarchicalPass]/2))
			&& (ii<(width - FTKernelWidths[HierarchicalPass]/2))) {
		      FFTCacheRequests[Nrequests].x = ii;
		      FFTCacheRequests[Nrequests].y = jj;
		      Nrequests++;
		    }
		  }
		}
	      }
	      
	      /* find the pixel shift with the lowest objective function */
	      if ((rc = KernelScan(FFTCacheRequests,
				   FFTCacheSizes[HierarchicalPass],
				   outpixels, mask, width, Nrequests,
				   i1, j1, FTKernelWidths[HierarchicalPass],
				   buf1, buf2, buf3, buf4, buf5, buf6,
				   &DxsReturn, &DysReturn,
				   &ShiftXReturn, &ShiftYReturn, 
				   &ObjFuncsReturn, &VafsReturn)) < 0) {
		Abort = B_TRUE;
		sprintf(cbuf,
			"Abort:from KernelScan() in PatchXCorrHierarchical()");
		draw_mesg(cbuf);
		DecrementParentProcessSemaphore();
		RestoreImageSize(zf, width, height, TmpFlag, lFFTWindow,
				 inpixels, outpixels);
		return;
	      }
	      FFTCacheRequestsNum += Nrequests;
	      FFTCacheHitsNum += rc;
	      
	      /* store results for this pixel in the output arrays */
	      dxs[index] = DxsReturn;
	      dys[index] = DysReturn;
	      shiftsX[index] = ShiftXReturn;
	      shiftsY[index] = ShiftYReturn;
	      objsfuncs[index] = ObjFuncsReturn;
	      VAFs[index] = VafsReturn;
	    } /* end "if" statement for masked pixels */
	    
	    /* pixel masked, so store dummy values */
	    else {
	      dxs[index] = 0;
	      dys[index] = 0;
	      shiftsX[index] = 0;
	      shiftsY[index] = 0;
	      objsfuncs[index] = 0;
	      VAFs[index] = 0;
	    }

	  } /* end "if" statement for X decimation */
	  
	} /* end loop for pixels */
	
	/* re-enable text output */
	DrawMesgEnabled = TmpFlag;

      } /* end "if" statement for Y decimation */

    } /* end loop for lines */
    
    /* fetch total processing time for this pass */
    ptime = XvgGetTime() - tstart;
      
    /* write out the text output file */
    sprintf(fname, "%s_%02d_README.doc", OutFname, ChildProcessNumber);
    if ((fp = fopen(fname, "w")) == NULL) {
      sprintf(cbuf, "PatchXCorrHierarchical() : Could not open the file %s",
	      fname);
      draw_mesg(cbuf);
      DecrementParentProcessSemaphore();
      RestoreImageSize(zf, width, height, TmpFlag, lFFTWindow,
		       inpixels, outpixels);
      return;
    }
    fprintf(fp, "XStartI1 = %d\n", XStartI1);
    fprintf(fp, "YStartI1 = %d\n", YStartI1);
    fprintf(fp, "XExtent  = %d\n", XExtent);
    fprintf(fp, "YExtent  = %d\n", YExtent);
    fprintf(fp, "Number of pixels matched = %d\n", NPixelsMatched);
    fprintf(fp, "Total memory allocated   = %d KBytes\n",
	    XvgMM_GetMemSize()/1000);
    if (FFTCacheInitialized)
      fprintf(fp, "FFT Cache initialized    = Yes\n");
    else
      fprintf(fp, "FFT Cache initialized    = No\n");
    fprintf(fp, "FFT Cache size           = %d items (%d Kbytes)\n",
	    FFTCacheSizes[HierarchicalPass],
	    (2*FTKernelWidths[HierarchicalPass]
	     *FTKernelWidths[HierarchicalPass]
	     *sizeof(double)*FFTCacheSizes[HierarchicalPass])/1000);
    fprintf(fp, "Number of cache requests = %d\n", FFTCacheRequestsNum);
    fprintf(fp, "Number of cache hits     = %d (%d%% hit ratio)\n",
	    FFTCacheHitsNum, 100*FFTCacheHitsNum/FFTCacheRequestsNum);
    fprintf(fp, "Processing time          = %.3f seconds (%.6fs / request, %.6fs / match)\n",
	    ptime,
	    ptime / ((double) FFTCacheRequestsNum),
	    ptime / ((double) NPixelsMatched));
    fprintf(fp, "DEC = %d, SMPL = %d, LAGSX = %d, LAGSY = %d, FTKW = %d, CS = %d\n",
	    Decimations[HierarchicalPass],
	    SamplingIncs[HierarchicalPass],
	    LagsX[HierarchicalPass],
	    LagsY[HierarchicalPass],
	    FTKernelWidths[HierarchicalPass],
	    FFTCacheSizes[HierarchicalPass]);
    fclose(fp);
  
    /* write the output data files */
    if (Abort == B_FALSE) {
      sprintf(fname, "%s_%02d_shiftsX.mat", OutFname, ChildProcessNumber);
      if (WriteMatlabFile(YExtent, XExtent, "shiftsX", shiftsX, fname)
	  == B_FALSE) {
	Abort = B_TRUE;
	sprintf(cbuf,
		"Abort: could not open \"%s\" in PatchXCorrHierarchical()",
		fname);
	draw_mesg(cbuf);
	DecrementParentProcessSemaphore();
	RestoreImageSize(zf, width, height, TmpFlag, lFFTWindow,
			 inpixels, outpixels);
	return;
      }
      sprintf(fname, "%s_%02d_shiftsY.mat", OutFname, ChildProcessNumber);
      if (WriteMatlabFile(YExtent, XExtent, "shiftsY", shiftsY, fname)
	  == B_FALSE) {
	Abort = B_TRUE;
	sprintf(cbuf,
		"Abort: could not open \"%s\" in PatchXCorrHierarchical()",
		fname);
	draw_mesg(cbuf);
	DecrementParentProcessSemaphore();
	RestoreImageSize(zf, width, height, TmpFlag, lFFTWindow,
			 inpixels, outpixels);
	return;
      }
      if (WriteSecondaryFiles[HierarchicalPass]) {
	sprintf(fname, "%s_%02d_objsfuncs.mat", OutFname, ChildProcessNumber);
	if (WriteMatlabFile(YExtent, XExtent, "objsfuncs", objsfuncs, fname)
	    == B_FALSE) {
	  Abort = B_TRUE;
	  sprintf(cbuf,
		  "Abort: could not open \"%s\" in PatchXCorrHierarchical()",
		  fname);
	  draw_mesg(cbuf);
	  DecrementParentProcessSemaphore();
	  RestoreImageSize(zf, width, height, TmpFlag, lFFTWindow,
			   inpixels, outpixels);
	  return;
	}
	sprintf(fname, "%s_%02d_VAFs.mat", OutFname, ChildProcessNumber);
	if (WriteMatlabFile(YExtent, XExtent, "VAFs", VAFs,fname) == B_FALSE) {
	  Abort = B_TRUE;
	  sprintf(cbuf,
		  "Abort: could not open \"%s\" in PatchXCorrHierarchical()",
		  fname);
	  draw_mesg(cbuf);
	  DecrementParentProcessSemaphore();
	  RestoreImageSize(zf, width, height, TmpFlag, lFFTWindow,
			   inpixels, outpixels);
	  return;
	}
	sprintf(fname, "%s_%02d_dxs.mat", OutFname, ChildProcessNumber);
	if (WriteMatlabFile(YExtent, XExtent, "dxs", dxs, fname) == B_FALSE) {
	  Abort = B_TRUE;
	  sprintf(cbuf,
		  "Abort: could not open \"%s\" in PatchXCorrHierarchical()",
		  fname);
	  draw_mesg(cbuf);
	  DecrementParentProcessSemaphore();
	  RestoreImageSize(zf, width, height, TmpFlag, lFFTWindow,
			   inpixels, outpixels);
	  return;
	}
	sprintf(fname, "%s_%02d_dys.mat", OutFname, ChildProcessNumber);
	if (WriteMatlabFile(YExtent, XExtent, "dys", dys, fname) == B_FALSE) {
	  Abort = B_TRUE;
	  sprintf(cbuf,
		  "Abort: could not open \"%s\" in PatchXCorrHierarchical()",
		  fname);
	  draw_mesg(cbuf);
	  DecrementParentProcessSemaphore();
	  RestoreImageSize(zf, width, height, TmpFlag, lFFTWindow,
			   inpixels, outpixels);
	  return;
	}
      }
    }
    
    /* check memory */
    sprintf(cbuf, "PatchXCorrHierarchical(decimation:%d)",
	    Decimations[HierarchicalPass]);
    XvgMM_MemoryCheck(cbuf);
    
    /* decrement parent's semaphore */
    DecrementParentProcessSemaphore();
  }
  
  /* free storage */
  XvgMM_Free(dxs);
  XvgMM_Free(dys);
  XvgMM_Free(ImgPatch);
  XvgMM_Free(VAFs);
  XvgMM_Free(shiftsX);
  XvgMM_Free(shiftsY);
  XvgMM_Free(objsfuncs);
  XvgMM_Free(buf1);
  XvgMM_Free(buf2);
  XvgMM_Free(buf3);
  XvgMM_Free(buf4);
  XvgMM_Free(buf5);
  XvgMM_Free(buf6);
  
  /* restore image size parameters */
  RestoreImageSize(zf, width, height, TmpFlag, lFFTWindow,
		   inpixels, outpixels);
}


/********************* PatchXCorrByLine  **********************
*
*   function : PatchXCorrByLine()
*              Version of PatchXCorr() which calculates the
*              parameters automatically to scan the complete images
*              for stereo shift, based on an initial estimate of the 
*              delay using the cross-correlation function.
*
*   parameters (loaded):
*              ((char *) ParmPtrs[0])  = Output filename prefix
*              *((int *) ParmPtrs[1])  = Number of lags to scan.
*              *((int *) ParmPtrs[2])  = YStart1 (if < 0, calculate)
*              *((int *) ParmPtrs[3])  = YExtent (if < 0, calculate)
*
***********************************************************************/
void PatchXCorrByLine(double *inpixels, double *outpixels,
		      int height, int width, void *ParmPtrs[])
{
  DOUBLE_ARRAY darray;
  void *parms[4];
  double darray_data[14], WindowRadiusMax, WindowRadiusMin, m;
  int i, j, k, sampling, kernw, lags, AvgShiftX, AvgShiftY,
  XStart1, YStart1, YExtent, ScanType, lFFTWindow, x, y, zero;

  /* check that stack is not empty */
  if (ImageStackTop <= 0) {
    Abort = B_TRUE;
    draw_mesg("Abort : One stack image required in StereoShiftScanAuto()");
    return;
  }
  
  /* load input parameters */
  lags = *((int *) ParmPtrs[1]);
  YStart1 = *((int *) ParmPtrs[2]);
  YExtent = *((int *) ParmPtrs[3]);

  /* init stereo parameters */
  StereoMMPerPixel = 200.0/((double) (height-1));
  StereoFocalLength = 500;
  StereoCameraSeparation = 50.0;
  sprintf(cbuf, "Camera seperation: %.2f mm, %.6f mm/pixel, distance:%.2f mm",
	  StereoCameraSeparation, StereoMMPerPixel, StereoFocalLength);
  InfoMsg(cbuf);

  /* set fixed parameters */
  sampling = 1;
  kernw = 32;
  WindowRadiusMax = 10;
  WindowRadiusMin = 1;
  ScanType = 0;
  
  /* cross-correlation */
  lFFTWindow = FFTWindow;
  FFTWindow = NOWINDOW;
  zero = 0;
  parms[0] = (void *) &zero;
  CrossCorrelation(inpixels, DoubleBuffer, height, width, parms);
  Laplacian(DoubleBuffer, outpixels, height, width, NULL);
  ImageStackTop++;
  UpdateStackLabel(ImageStackTop);
  FFTWindow = lFFTWindow;

  /* check for aborts */
  if (Abort)
    return;

  /* find max in the Laplacian and use as estimate of average shift */
  for (j = 0, k = 0, m = -MAXDOUBLE; j < height; j++) {
    for (i = 0; i < width; i++, k++) {
      if (outpixels[k] > m) {
	m = outpixels[k];
	x = i;
	y = j;
      }
    }
  }
  AvgShiftX = width/2 - x;
  AvgShiftY = height/2 - y;
  if (VerboseFlag)
    printf("PatchXCorrByLine():Shift estimate is (%d,%d) pix\n",
	   AvgShiftX, AvgShiftY);
  
  /* calculate stereo shift */
  parms[0] = (void *) ParmPtrs[0];
  XStart1 = lags + kernw/2 + 2;
  if (AvgShiftX < 0)
    XStart1 -= AvgShiftX;
  if (YStart1 < 0)
    YStart1 = lags + kernw/2 + 2;
  if (AvgShiftY < 0)
    YStart1 -= AvgShiftY;
  parms[1] = (void *) &XStart1;
  parms[2] = (void *) &YStart1;
  parms[3] = (void *) &darray;
  darray.n = 14;
  darray.data = darray_data;
  darray_data[0]  = AvgShiftX; /* X offset */
  darray_data[1]  = AvgShiftY; /* Y offset */
  if (AvgShiftX < 0)
    darray_data[2]  = width - (lags + kernw/2 + 2) - XStart1;
  else
    darray_data[2]  = width -darray_data[0] - (lags + kernw/2 + 2) - XStart1;
  if (YExtent < 0) {
    if (AvgShiftY < 0)
      darray_data[3] = height - (lags + kernw/2 + 2) - YStart1;
    else
      darray_data[3] = height - darray_data[1] -(lags + kernw/2 + 2) -YStart1;
  }
  else
    darray_data[3] = YExtent;
  darray_data[4]  = lags;
  darray_data[5]  = sampling;
  darray_data[6]  = kernw;
  darray_data[7]  = WindowRadiusMax;
  darray_data[8]  = WindowRadiusMin;
  darray_data[9]  = ScanType;
  darray_data[10] = 0;
  PatchXCorr(inpixels, outpixels, height, width, parms);

  /* restore FFT window */
  FFTWindow = lFFTWindow;

  /* restore output buffer */
  for (i = 0; i < height*width; i++)
    outpixels[i] = inpixels[i];
}


/****************** PatchXCorrByHorizSliceParallel  **********************
*
*   function : PatchXCorrByHorizSliceParallel()
*              Version of PatchXCorrByHorizSlice() which calculates the
*              parameters automatically to scan the complete images
*              for patch shift, based on an initial estimate of the 
*              delay using the cross-correlation function. This version
*              creates child processes which run in parallel on a multi
*              CPU machine.
*
*   parameters (loaded):
*              ((char *) ParmPtrs[0])  = Output filename prefix
*              *((int *) ParmPtrs[1])  = Number of lags to scan.
*
***********************************************************************/
static void CombineDataFiles(char *vname, char *fname_mantissa, double *buffer,
			     int nprocessors)
{
  double *inpixels, min, max;
  char fname[256];
  int i, n, k, hh, ww, count, TotalHeight;

  /* loop to load the partial data files */
  for (n = 0, count = 0, TotalHeight = 0;
       (n < nprocessors) && (Abort == B_FALSE); n++) {
    /* load the file */
    sprintf(fname, "%s_%02d_%s.mat", fname_mantissa, n, vname);
    if (GetMatlabFile(&hh, &ww, &min, &max,
		      &inpixels, fname, VerboseFlag) == B_FALSE) {
      Abort = B_TRUE;
      sprintf(cbuf, "Abort: Could not open \"%s\" in CombineDataFiles()",
	      fname);      
      draw_mesg(cbuf);
      return;
    }
    /* delete the file */
    unlink(fname);
    
    /* copy the data to the complete image buffer */
    for (i = 0, k = count; k < (count + hh*ww); k++, i++)
      buffer[k] = inpixels[i];

    /* update the running counts */
    count += (hh * ww);
    TotalHeight += hh;
    if (VerboseFlag)
      printf("CombineDataFiles() : Loaded %s (%dx%d), count:%d, T-Height:%d\n",
	     fname, ww, hh, count, TotalHeight);
  }
  XvgMM_Free(inpixels);
  
  /* write out the complete image buffer */
  sprintf(fname, "%s_%s.mat", fname_mantissa, vname);
  WriteMatlabFile(TotalHeight, ww, vname, buffer, fname);

  /* check memory */
  sprintf(cbuf, "CombineDataFiles(%s, %s, 0x%x)", vname, fname_mantissa,
	  (int) buffer);
  XvgMM_MemoryCheck(cbuf);
}



void PatchXCorrByHorizSliceParallel(double *inpixels, double *outpixels,
				    int height, int width, void *ParmPtrs[])
{
  FILE *fpo, *fpi;
  DOUBLE_ARRAY darray;
  void *parms[4];
  char fname[256];
  double m, darray_data[14], WindowRadiusMax, WindowRadiusMin, *buffer;
  int i, j, k, sampling, kernw, lags, lFFTWindow, zero,
  ScanType, AvgShiftX, AvgShiftY, x, y, XStart1, YStart1, Yextent,
  YextentTotal, Xextent, pp;

  /* check that stack is not empty */
  if (ImageStackTop <= 0) {
    Abort = B_TRUE;
    draw_mesg("Abort: One stack image required in StereoShiftScanForkAuto()");
    return;
  }
  
  /* load input parameters */
  lags = *((int *) ParmPtrs[1]);
  kernw = *((int *) ParmPtrs[2]);
  WindowRadiusMax = *((int *) ParmPtrs[3]);

  /* init stereo parameters */
  StereoMMPerPixel = 200.0/((double) (height-1));
  StereoFocalLength = 500;
  StereoCameraSeparation = 50.0;
  sprintf(cbuf, "Camera seperation: %.f mm, %f mm/pixel, distance:%f mm",
	  StereoCameraSeparation, StereoMMPerPixel, StereoFocalLength);
  InfoMsg(cbuf);

  /* set fixed parameters */
  sampling = 1;
  WindowRadiusMin = 1;
  ScanType = 0;
  nprocessors = PROCESSORS_NMAX;
  npids = nprocessors - 1;

  /* cross-correlation */
  lFFTWindow = FFTWindow;
  FFTWindow = NOWINDOW;
  zero = 0;
  parms[0] = (void *) &zero;
  CrossCorrelation(inpixels, DoubleBuffer, height, width, parms);
  Laplacian(DoubleBuffer, outpixels, height, width, NULL);
  ImageStackTop++;
  UpdateStackLabel(ImageStackTop);
  FFTWindow = lFFTWindow;

  /* check for aborts */
  if (Abort)
    return;

  /* find max in the Laplacian and use as estimate of average shift */
  for (j = 0, k = 0, m = -MAXDOUBLE; j < height; j++) {
    for (i = 0; i < width; i++, k++) {
      if (outpixels[k] > m) {
	m = outpixels[k];
	x = i;
	y = j;
      }
    }
  }
  AvgShiftX = width/2 - x;
  AvgShiftY = height/2 - y;
  if (VerboseFlag)
    printf("PatchXCorrByHorizSliceParallel():Shift estimate is (%d,%d) pix\n",
	   AvgShiftX, AvgShiftY);
  
  /* calculate stereo shift */
  parms[0] = (void *) fname;
  XStart1 = lags + kernw/2 + 2;
  if (AvgShiftX < 0)
    XStart1 -= AvgShiftX;
  YStart1 = lags + kernw/2 + 2;
  if (AvgShiftY < 0)
    YStart1 -= AvgShiftY;
  parms[1] = (void *) &XStart1;
  parms[2] = (void *) &YStart1;
  parms[3] = (void *) &darray;
  darray.n = 14;
  darray.data = darray_data;
  darray_data[0]  = AvgShiftX; /* X offset */
  darray_data[1]  = AvgShiftY; /* Y offset */
  if (AvgShiftX < 0)
    Xextent  = width - (lags + kernw/2 + 2) - XStart1;
  else
    Xextent  = width -darray_data[0] - (lags + kernw/2 + 2) - XStart1;
  darray_data[2] = Xextent;
  if (AvgShiftY < 0)
    YextentTotal = height - (lags + kernw/2 + 2) - YStart1;
  else
    YextentTotal = height - darray_data[1] -(lags + kernw/2 + 2) - YStart1;
  Yextent = YextentTotal/nprocessors;
  darray_data[3]  = Yextent;
  darray_data[4]  = lags;
  darray_data[5]  = sampling;
  darray_data[6]  = kernw;
  darray_data[7]  = WindowRadiusMax;
  darray_data[8]  = WindowRadiusMin;
  darray_data[9]  = ScanType;
  darray_data[10] = 0;
  FFTWindow = HANNINGWINDOW;

  /* loop to start parallel processes */
  for (ChildProcessNumber = 0; ChildProcessNumber < (nprocessors-1);
       ChildProcessNumber++) {
    sprintf(cbuf,"PatchXCorrByHorizSliceParallel(): Starting child process %d",
	    ChildProcessNumber);
    draw_mesg(cbuf);
    /* start child process */
    ChildProcess = B_TRUE;
    if ((pids[ChildProcessNumber] = fork()) == 0) {
      sprintf(fname, "%s_%02d", ParmPtrs[0], ChildProcessNumber);
      PatchXCorr(inpixels, outpixels, height, width, parms);
      exit(EXIT_SUCCESS);
    }
    /* update Y starting point */
    else {
      YStart1 += Yextent;
    }
    ChildProcess = B_FALSE;
  }
  draw_mesg("PatchXCorrByHorizSliceParallel() : Executing current process...");
  darray_data[3] = YextentTotal - (nprocessors-1)*Yextent;
  sprintf(fname, "%s_%d", ParmPtrs[0], nprocessors-1);
  PatchXCorr(inpixels, outpixels, height, width, parms);
  
  /* check for aborts */
  if (Abort)
    return;

  /* wait for the child processes to terminate */
  draw_mesg("PatchXCorrByHorizSliceParallel() : Waiting for child processes to terminate...");
  for (pp = 0; pp < (nprocessors-1); pp++)
    waitpid(pids[pp], NULL, 0);
  
  /* restore FFT window */
  FFTWindow = lFFTWindow;

  /* recombine child process output data files */
  draw_mesg("PatchXCorrByHorizSliceParallel() : Writing results to files...");
  if ((buffer = (double *) XvgMM_Alloc((void *) NULL,
				       Xextent*YextentTotal*sizeof(double)))
      == NULL) {
    Abort = B_TRUE;
    draw_mesg("Abort : Out of memory in StereoShiftScanForkAuto()");
    return;
  }
  CombineDataFiles("objsfuncs", ParmPtrs[0], buffer, nprocessors);
  CombineDataFiles("VAFs", ParmPtrs[0], buffer, nprocessors);
  CombineDataFiles("shiftsX", ParmPtrs[0], buffer, nprocessors);
  CombineDataFiles("shiftsY", ParmPtrs[0], buffer, nprocessors);
  CombineDataFiles("dxs", ParmPtrs[0], buffer, nprocessors);
  CombineDataFiles("dys", ParmPtrs[0], buffer, nprocessors);
  CombineDataFiles("X", ParmPtrs[0], buffer, nprocessors);
  CombineDataFiles("Y", ParmPtrs[0], buffer, nprocessors);
  CombineDataFiles("Z", ParmPtrs[0], buffer, nprocessors);
  XvgMM_Free(buffer);

  /* recombine README files */
  sprintf(fname, "%s_README.doc", ParmPtrs[0]);
  if ((fpo = fopen(fname, "w")) == NULL) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort: Could not open \"%s\" in StereoShiftScanForkAuto()",
	    fname);
    draw_mesg(cbuf);
    return;
  }
  fprintf(fpo, "PARALLEL PROCESSES: %d\n", nprocessors);
  fprintf(fpo, "YextentTotal: %d\n", YextentTotal);
  fprintf(fpo, "\n***********\n\n");
  for (pp = 0; pp < nprocessors; pp++) {
    /* load the file */
    sprintf(fname, "%s_%d_README.doc", ParmPtrs[0], pp);
    if ((fpi = fopen(fname, "r")) == NULL) {
      Abort = B_TRUE;
      sprintf(cbuf, "Abort:Could not open \"%s\" in StereoShiftScanForkAuto()",
	      fname);
      draw_mesg(cbuf);
      return;
    }
    /* copy the data */
    while (fgets(cbuf, 512, fpi) != NULL)
      fputs(cbuf, fpo);
    if (pp < (nprocessors -1))
      fprintf(fpo, "\n***********\n\n");
    /* close and delete the file */
    fclose(fpi);
    unlink(fname);
  }
  fclose(fpo);

  /* restore output buffer */
  for (i = 0; i < height*width; i++)
    outpixels[i] = inpixels[i];
}


/****************** PatchXCorrByAreaParallel  **********************
*
*   function : PatchXCorrByAreaParallel()
*              Version of PatchXCorr() which calculates the
*              parameters automatically to scan the complete images
*              for patch shift, based on an initial estimate of the 
*              delay using the cross-correlation function. This version
*              creates child processes which run in parallel on a multi
*              CPU machine. This routine assumes that the gross shift
*              between both images if near 0.
*
*   parameters (loaded):
*              ((char *) ParmPtrs[0])  = Output filename prefix
*              *((int *) ParmPtrs[1])  = Number of child processes in X
*              *((int *) ParmPtrs[2])  = Number of child processes in Y
*              *((int *) ParmPtrs[3])  = Extra parameters...
*                       parameters[0]  = Number of lags to scan.
*                       parameters[1]  = FFT kernel size (32 recommended)
*                       parameters[2]  = FFT filter radius (9 recommended)
*                       parameters[3]  = Decimation (log base 2),
*                                        i.e.: 0 means sub-sampling by 1 (none)
*                                              1 means sub-sampling by 2
*                                              2 means sub-sampling by 4
*                                              etc...
*                                        Note: this parameter is ignored
*                                              in the refinement pass,
*                                              where parameters[4] != 0.
*                       parameters[4]  = Estimation strategy
*                                         0: initial shift estimation for each
*                                            child process patch using
*                                            cross-correlation. Single pass
*                                            processing, no refinement of the
*                                            solution.
*                                            (recommended for biaxial strain
*                                            measurements).
*                                         X: Initial shift estimation for
*                                            each child process patch using
*                                            averaging on each patch
*                                            and decimated sampling
*                                            (i.e.: parameters[3] = 3 or 4,
*                                            and parameters[0] >> 1).
*                                            A second pass on the data is
*                                            made (refinement) using the value
*                                            of this parameter (X) instead
*                                            of "parameters[0]" and setting 
*                                            the decimation to "none".
*                                            (recommended for geometry from
*                                            stereo calculations).
*
***********************************************************************/
static void CombineAreaPatchDataFiles(char *vname, char *fname_mantissa,
				      int height, int width,
				      int *xs, int *ys, double *buffer,
				      int ngridx, int ngridy, int decimation)
{
  double *inpixels, min, max;
  char fname[256];
  int x, y, i, j, k, n, hh, ww;

  /* null output buffer */
  for (i = 0; i < height*width; i++)
    buffer[i] = 0;

  /* loop to load the partial data files */
  for (y = 0, n = 0; (y < ngridy) && (Abort == B_FALSE); y++) {
    for (x = 0; (x < ngridx) && (Abort == B_FALSE); x++, n++) {
      /* load the file */
      sprintf(fname, "%s_%02d_%s.mat", fname_mantissa, n, vname);
      if (GetMatlabFile(&hh, &ww, &min, &max,
			&inpixels, fname, VerboseFlag) == B_FALSE) {
	Abort = B_TRUE;
	sprintf(cbuf, "Abort: Could not open \"%s\" in CombineDataFiles()",
		fname);      
	draw_mesg(cbuf);
	return;
      }
      /* delete the file */
      unlink(fname);
      
      /* copy the data to the complete image buffer */
      for (j = ys[n], k = 0; j < (ys[n] + hh); j++) {
	for (i = xs[n]; i < (xs[n] + ww); i++, k++) {
	  if (((j & (decimation-1)) == 0) && ((i & (decimation-1)) == 0)) {
	    buffer[(j/decimation)*width + (i/decimation)] = inpixels[k];
	  }
	}
      }

      /* print progress of verbose */
      if (VerboseFlag)
	printf("CombineDataFiles() : Loaded %s (%dx%d)...\n", fname);
    }
  }
  XvgMM_Free(inpixels);
  
  /* write out the buffer */
  sprintf(fname, "%s_%s.mat", fname_mantissa, vname);
  WriteMatlabFile(height, width, vname, buffer, fname);

  /* check memory */
  sprintf(cbuf, "CombineDataFiles(%s, %s, 0x%x)", vname, fname_mantissa,
	  (int) buffer);
  XvgMM_MemoryCheck(cbuf);
}



void PatchXCorrByAreaParallel(double *inpixels, double *outpixels,
			      int height, int width, void *ParmPtrs[])
{
  FILE *fpo, *fpi;
  DOUBLE_ARRAY darray, *parameters;
  void *parms[4], *parms_xc[4];
  char fname[256];
  double m, darray_data[14], WindowRadiusMax, WindowRadiusMin, *buffer;
  int i, j, k, decimation, kernw, lags, lFFTWindow, zero, status,
  ScanType, pp, EdgeWidth, XStart, YStart, XInc, YInc, x, y,
  xs[PROCESSORS_NMAX], ys[PROCESSORS_NMAX], iv, ngridx, ngridy,
  EstimationStrategyFlag, decimation_log2;

  /* check that stack is not empty */
  if (ImageStackTop <= 0) {
    Abort = B_TRUE;
    draw_mesg("Abort: One stack image required in PatchXCorrByAreaParallel()");
    return;
  }

  /* load input parameters */
  ngridx = *((int *) ParmPtrs[1]);
  ngridy = *((int *) ParmPtrs[2]);
  parameters = (DOUBLE_ARRAY *) ParmPtrs[3];
  if (parameters->n < 5) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort : 5 extra parms in PatchXCorrByAreaParallel()");
    draw_mesg(cbuf);
    return;
  }
  lags = (parameters->data)[0];
  kernw = (parameters->data)[1];
  WindowRadiusMax = (parameters->data)[2];
  decimation_log2 = (parameters->data)[3];
  decimation = 0x01 << decimation_log2;
  EstimationStrategyFlag = (parameters->data)[4];

  /* set fixed parameters */
  WindowRadiusMin = 1;
  ScanType = 1;
  nprocessors = ngridx*ngridy;
  npids = nprocessors - 1;
  lFFTWindow = FFTWindow;

  /* check that we have 16 processors available */
  if (nprocessors > PROCESSORS_NMAX) {
    Abort = B_TRUE;
    draw_mesg("Abort: max processors exceeded in PatchXCorrByAreaParallel()");
    return;
  }
  
  /* set fixed function parameters */
  EdgeWidth = lags + kernw/2 + 2;
  XInc = (width - 2*EdgeWidth)/ngridx;
  YInc = (height - 2*EdgeWidth)/ngridy;
  parms[0] = (void *) fname;
  parms[1] = (void *) &XStart; /* X start in I1 */
  parms[2] = (void *) &YStart; /* Y start in I1 */
  parms[3] = (void *) &darray;
  darray.n = 14;
  darray.data = darray_data;
  darray_data[4]  = lags;
  darray_data[5]  = decimation;
  darray_data[6]  = kernw;
  darray_data[7]  = WindowRadiusMax;
  darray_data[8]  = WindowRadiusMin;
  darray_data[9]  = ScanType;
  darray_data[10] = 1;

  /* generate the scanning offsets, if required */
  if (EstimationStrategyFlag != 0) {
    /* determine gross image shift using cross-correlation and Laplacian */
    for (i = 0; i < height*width; i++)
      DoubleBuffer[i] = inpixels[i];
    XvgImgStack_Push(ImageStack[ImageStackTop-1].pixels, height, width);
    FFTWindow = NOWINDOW;
    zero = 0;
    parms_xc[0] = (void *) &zero;
    CrossCorrelation(DoubleBuffer, outpixels, height, width, parms_xc);
    Laplacian(outpixels, DoubleBuffer, height, width, NULL);
    if (Abort)
      return;

    /* find max in the Laplacian and use as estimate of gross shift */
    for (j = 4, m = -MAXDOUBLE; j < (height-4); j++) {
      for (i = 4; i < (width-4); i++) {
	if (DoubleBuffer[j*width + i] > m) {
	  m = DoubleBuffer[j*width + i];
	  x = i;
	  y = j;
	}
      }
    }
    darray_data[0] = rint(((double) width)/2.0) - x;  /* X offset */
    darray_data[1] = rint(((double) height)/2.0) - y; /* Y offset */
    if (VerboseFlag)
      printf("PatchXCorrByAreaParallel(): Shift estimate is (%d,%d) pixels\n",
	     width, height);
  }
  else {
    darray_data[0] = -999; /* X offset */
    darray_data[1] = -999; /* Y offset */
  }

  /* loop to start parallel processes */
  for (j = 0, ChildProcessNumber = 0; j < ngridy; j++) {
    /* calculate Y parameters for this patch */
    YStart = EdgeWidth + j*YInc;
    if (j < (ngridy-1))
      darray_data[3] = YInc;                           /* Yextent */
    else
      darray_data[3] = (height - 2*EdgeWidth) - (ngridy-1)*YInc; /* Yextent */
    for (i = 0; i < ngridx; i++, ChildProcessNumber++) {
      /* show banner */
      sprintf(cbuf,"PatchXCorrByAreaParallel(): Starting child process %d",
	      ChildProcessNumber);
      draw_mesg(cbuf);
      /* calculate X parameters for this patch */
      XStart = EdgeWidth + i*XInc;
      if (i < (ngridx-1))
	darray_data[2] = XInc;                           /* Xextent */
      else
	darray_data[2] = (width - 2*EdgeWidth) - (ngridx-1)*XInc; /* Xextent */
      /* if this the last patch, do the work in the current process */
      if ((i == (ngridx-1)) && (j == (ngridy-1))) {
	sprintf(fname, "%s_%02d", ParmPtrs[0], ChildProcessNumber);
	PatchXCorr(inpixels, outpixels, height, width, parms);
	/* refine the solution if required */
	if ((EstimationStrategyFlag != 0) && (Abort == B_FALSE)) {
	  sprintf(fname, "%s_REFINED_%02d", ParmPtrs[0], ChildProcessNumber);
	  darray_data[0] = AverageShiftX;            /* X offset */
	  darray_data[1] = AverageShiftY;            /* Y offset */
	  darray_data[4] = EstimationStrategyFlag;       /* lags */
	  darray_data[5] = 1;                      /* decimation */
	  PatchXCorr(inpixels, outpixels, height, width, parms);
	}
      }
      /* else, start child process */
      else {
	ChildProcess = B_TRUE;
	if ((pids[ChildProcessNumber] = fork()) == 0) {
	  sprintf(fname, "%s_%02d", ParmPtrs[0], ChildProcessNumber);
	  PatchXCorr(inpixels, outpixels, height, width, parms);
	  /* refine the solution if required */
	  if ((EstimationStrategyFlag != 0) && (Abort == B_FALSE)) {
	    sprintf(fname, "%s_REFINED_%02d", ParmPtrs[0], ChildProcessNumber);
	    darray_data[0] = AverageShiftX;            /* X offset */
	    darray_data[1] = AverageShiftY;            /* Y offset */
	    darray_data[4] = EstimationStrategyFlag;       /* lags */
	    darray_data[5] = 1;                      /* decimation */
	    PatchXCorr(inpixels, outpixels, height, width, parms);
	  }
	  if (Abort)
	    exit(EXIT_FAILURE);
	  else
	    exit(EXIT_SUCCESS);
	}
	ChildProcess = B_FALSE;
      }
    }
  }
  
  /* wait for the child processes to terminate */
  draw_mesg("PatchXCorrByHorizSliceParallel() : Waiting for child processes to terminate...");
  for (pp = 0; pp < npids; pp++) {
    waitpid(pids[pp], &status, 0);
    if (WIFEXITED(status)) {
      if (WEXITSTATUS(status) != EXIT_SUCCESS) {
	Abort = B_TRUE;
	printf("\aWARNING: Process %d terminated with failure in PatchXCorrByAreaParallel()\n",
	       pp);
      }
      else {
	if (VerboseFlag) {
	  printf("Process %d terminated normally in PatchXCorrByAreaParallel()\n",
	       pp);
	}
      }
    }
    else {
      printf("\aWARNING: Process %d terminated for unknown reason in PatchXCorrByAreaParallel()\n",
	     pp);
    }
  }
  
  /* restore FFT window */
  FFTWindow = lFFTWindow;

  /* recombine child process output data files */
  if (Abort == FALSE) {
    /* recombine README files */
    sprintf(fname, "%s_README.doc", ParmPtrs[0]);
    if ((fpo = fopen(fname, "w")) == NULL) {
      Abort = B_TRUE;
      sprintf(cbuf, "Abort:Couldn't open \"%s\" in PatchXCorrByAreaParallel()",
	      fname);
      draw_mesg(cbuf);
      return;
    }
    fprintf(fpo, "PARALLEL PROCESSES: %d (%d x %d)\n",
	    nprocessors, ngridx, ngridy);
    fprintf(fpo, "\n***********\n\n");
    for (pp = 0; pp < nprocessors; pp++) {
      /* show header */
      fprintf(fpo, "PROCESS #%d:\n", pp);

      /* load the file */
      sprintf(fname, "%s_%02d_README.doc", ParmPtrs[0], pp);
      if ((fpi = fopen(fname, "r")) == NULL) {
	Abort = B_TRUE;
	sprintf(cbuf, "Abort: Couldn't open %s in PatchXCorrByAreaParallel()",
		fname);
	draw_mesg(cbuf);
	return;
      }

      /* extract the XStartI1/YStartI1 fields */
      fgets(cbuf, 512, fpi);
      fgets(cbuf, 512, fpi);
      if (sscanf(cbuf, "XStartI1 = %d", &iv) != 1) {
	Abort = B_TRUE;
	sprintf(cbuf, "Abort: Couldn't load xs from file %s in PatchXCorrByAreaParallel()", fname);
	draw_mesg(cbuf);
	return;
      }
      xs[pp] = iv;
      fgets(cbuf, 512, fpi);
      if (sscanf(cbuf, "YStartI1 = %d", &iv) != 1) {
	Abort = B_TRUE;
	sprintf(cbuf, "Abort: Couldn't load ys from file %s in PatchXCorrByAreaParallel()", fname);
	draw_mesg(cbuf);
	return;
      }
      ys[pp] = iv;
      fseek(fpi, 0, 0);

      /* copy the data to the output file */
      while (fgets(cbuf, 512, fpi) != NULL)
	fputs(cbuf, fpo);
      if (pp < (nprocessors -1))
	fprintf(fpo, "\n***********\n\n");

      /* close and delete the file */
      fclose(fpi);
      unlink(fname);
    }
    fclose(fpo);

    /* recombine child process output data files */
    draw_mesg("PatchXCorrByHorizSliceParallel(): Writing results to files...");
    if ((buffer = (double *) XvgMM_Alloc((void *) NULL,
					 height*width/(decimation*decimation)
					 *sizeof(double)))
	== NULL) {
      Abort = B_TRUE;
      draw_mesg("Abort : Out of memory in PatchXCorrByAreaParallel()");
      return;
    }
    CombineAreaPatchDataFiles("objsfuncs", ParmPtrs[0],
			      height/decimation, width/decimation,
			      xs, ys, buffer, ngridx, ngridy, decimation);
    CombineAreaPatchDataFiles("VAFs", ParmPtrs[0],
			      height/decimation, width/decimation,
			      xs, ys, buffer, ngridx, ngridy, decimation);
    CombineAreaPatchDataFiles("shiftsX", ParmPtrs[0],
			      height/decimation, width/decimation,
			      xs, ys, buffer, ngridx, ngridy, decimation);
    CombineAreaPatchDataFiles("shiftsY", ParmPtrs[0],
			      height/decimation, width/decimation,
			      xs, ys, buffer, ngridx, ngridy, decimation);
    CombineAreaPatchDataFiles("dxs", ParmPtrs[0],
			      height/decimation, width/decimation,
			      xs, ys, buffer, ngridx, ngridy, decimation);
    CombineAreaPatchDataFiles("dys", ParmPtrs[0],
			      height/decimation, width/decimation,
			      xs, ys, buffer, ngridx, ngridy, decimation);

    /* recombined the refined data files if required */
    if (EstimationStrategyFlag != 0) {
      /* recombine README files */
      sprintf(fname, "%s_REFINED_README.doc", ParmPtrs[0]);
      if ((fpo = fopen(fname, "w")) == NULL) {
	Abort = B_TRUE;
	sprintf(cbuf,
		"Abort:Couldn't open \"%s\" in PatchXCorrByAreaParallel()",
		fname);
	draw_mesg(cbuf);
	return;
      }
      fprintf(fpo, "PARALLEL PROCESSES: %d (%d x %d)\n",
	      nprocessors, ngridx, ngridy);
      fprintf(fpo, "\n***********\n\n");
      for (pp = 0; pp < nprocessors; pp++) {
	/* show header */
	fprintf(fpo, "PROCESS #%d:\n", pp);
	
	/* load the file */
	sprintf(fname, "%s_REFINED_%02d_README.doc", ParmPtrs[0], pp);
	if ((fpi = fopen(fname, "r")) == NULL) {
	  Abort = B_TRUE;
	  sprintf(cbuf,
		  "Abort: Couldn't open %s in PatchXCorrByAreaParallel()",
		  fname);
	  draw_mesg(cbuf);
	  return;
	}
	
	/* extract the XStartI1/YStartI1 fields */
	fgets(cbuf, 512, fpi);
	fgets(cbuf, 512, fpi);
	if (sscanf(cbuf, "XStartI1 = %d", &iv) != 1) {
	  Abort = B_TRUE;
	  sprintf(cbuf, "Abort: Couldn't load xs from file %s in PatchXCorrByAreaParallel()", fname);
	  draw_mesg(cbuf);
	  return;
	}
	xs[pp] = iv;
	fgets(cbuf, 512, fpi);
	if (sscanf(cbuf, "YStartI1 = %d", &iv) != 1) {
	  Abort = B_TRUE;
	  sprintf(cbuf, "Abort: Couldn't load ys from file %s in PatchXCorrByAreaParallel()", fname);
	  draw_mesg(cbuf);
	  return;
	}
	ys[pp] = iv;
	fseek(fpi, 0, 0);
	
	/* copy the data to the output file */
	while (fgets(cbuf, 512, fpi) != NULL)
	  fputs(cbuf, fpo);
	if (pp < (nprocessors -1))
	  fprintf(fpo, "\n***********\n\n");
	
	/* close and delete the file */
	fclose(fpi);
	unlink(fname);
      }
      fclose(fpo);
      
      /* recombine the data files */
      if ((buffer = (double *) XvgMM_Alloc((void *) NULL,
					   height*width*sizeof(double)))
	  == NULL) {
	Abort = B_TRUE;
	draw_mesg("Abort : Out of memory in PatchXCorrByAreaParallel()");
	return;
      }
      sprintf(fname, "%s_REFINED", ParmPtrs[0]);
      CombineAreaPatchDataFiles("objsfuncs", fname, height, width,
				xs, ys, buffer, ngridx, ngridy, 1);
      CombineAreaPatchDataFiles("VAFs", fname, height, width,
				xs, ys, buffer, ngridx, ngridy, 1);
      CombineAreaPatchDataFiles("shiftsX", fname, height, width,
				xs, ys, buffer, ngridx, ngridy, 1);
      CombineAreaPatchDataFiles("shiftsY", fname, height, width,
				xs, ys, buffer, ngridx, ngridy, 1);
      CombineAreaPatchDataFiles("dxs", fname, height, width,
				xs, ys, buffer, ngridx, ngridy, 1);
      CombineAreaPatchDataFiles("dys", fname, height, width,
				xs, ys, buffer, ngridx, ngridy, 1);
    }

    /* free storage */
    XvgMM_Free(buffer);
  }

  /* restore output buffer */
  for (i = 0; i < height*width; i++)
    outpixels[i] = inpixels[i];
}


/******************     OpticalFlowFieldFilter     *********************
*
*   function : OpticalFlowFieldFilter()
*
*   parameters (loaded):
*              *((char *)    ParmPtrs[0])  = Input filename
*              *((int *)     ParmPtrs[1])  = Unused...
*              *((int *)     ParmPtrs[2])  = Write Intermediate files (0:no)
*              *((double *)  ParmPtrs[3])  = extra parameters
*                   (parameters->data)[0]  = Generate pixel mask (0:No)
*                   (parameters->data)[1]  = LaplacianThreshold
*                   (parameters->data)[2]  = Dynamic range threshold (ex:100.0)
*                   (parameters->data)[3]  = Node footprint radius   (ex: 25.0)
*                   (parameters->data)[4]  = Kernel filter area size (ex:3or5)
*
***********************************************************************/
void OpticalFlowFieldFilter(double *inpixels, double *outpixels,
		       int height, int width, void *ParmPtrs[])
{
  DOUBLE_ARRAY *parameters;
  boolean_t BooleanTmp;
  void *parms[4];
  FILE *fp;
  char StrippedMantissa[512], mantissa[512], fname[512], vname[512];
  double MaskRadius, NodeXL, NodeYL, NodeXR, NodeYR, v0, v1, v2, v3,
  LaplacianThreshold, T, X, Y, *p, min, max;
  int k, x, y, GenMask, WriteIntermediateIPFilesFlag, decimation, vi0, vi1,
  hh, ww, NNodes, NValidPixels, NLines, kernw;
  
  /* load input parameters */
  WriteIntermediateIPFilesFlag = *((int *)        ParmPtrs[2]);
  parameters                   = (DOUBLE_ARRAY *) ParmPtrs[3];
  if (parameters->n < 5) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort : 5 extra parms in OpticalFlowFieldFilter()");
    draw_mesg(cbuf);
    return;
  }
  GenMask            = (parameters->data)[0];
  LaplacianThreshold = (parameters->data)[1];
  T                  = (parameters->data)[2];
  MaskRadius         = (parameters->data)[3];
  kernw              = (parameters->data)[4];
  if (VerboseFlag)
    printf("OpticalFlowFieldFilter():Genmask=%d, LlpT = %f, T=%f, MaskRd=%f\n",
	   GenMask, LaplacianThreshold, T, MaskRadius);

  /* get input file */
  if (GetMatlabFile(&hh, &ww, &min, &max,
		    &outpixels, ParmPtrs[0], VerboseFlag) == B_FALSE) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort: Can't open \"%s\" in OpticalFlowFieldFilter()",
	    ParmPtrs[0]);      
    draw_mesg(cbuf);
    return;
  }

  /* check input file size against current size */
  if ((hh != height) || (ww != width)) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort:Image siz mismatch (%dx%d), OpticalFlowFieldFilter()",
	    ww, hh);      
    draw_mesg(cbuf);
    return;
  }

  /* fetch input filename matissa */
  sprintf(mantissa, "%s", ParmPtrs[0]);
  for (k = (strlen(mantissa) - 1); (k >= 0) && (mantissa[k] != '.'); k--);
  mantissa[k] = '\0';
  sprintf(StrippedMantissa, "%s", mantissa);
  StrippedMantissa[strlen(mantissa) - 12] = '\0';

  /* extract decimation parameter */
  cbuf[0] = mantissa[k-2];
  cbuf[1] = mantissa[k-1];
  cbuf[2] = '\0';
  decimation = atoi(cbuf);
  if ((decimation != 1) && (decimation != 2) && (decimation != 4)
      && (decimation != 8) && (decimation != 16) && (decimation != 32)) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort:invalid decimation (%d) in OpticalFlowFieldFilter()",
	    decimation);
    draw_mesg(cbuf);
    return;
  }
  if (VerboseFlag)
    printf("OpticalFlowFieldFilter() : decimation = %d\n", decimation);

  /* extract vname parameter */
  if (mantissa[k-5] == 'X') {
    sprintf(vname, "shiftsX");
  }
  else if (mantissa[k-5] == 'Y') {
    sprintf(vname, "shiftsY");
  }
  else {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort : invalid vname (%c) in OpticalFlowFieldFilter()",
	    mantissa[k-5]);
    draw_mesg(cbuf);
    return;
  }

  /* clear the stack */
  ImageStackTop = 0;
  
  /* generate the mask if required */
  if (GenMask) { 
    /* init mask values to 0 */
    for (k = 0; k < height*width; k++)
      MaskPixels[k] = 0;
    
    /* open the starting node coordinate file */
    sprintf(fname, "%s.2d", StrippedMantissa);
    if ((fp = fopen(fname, "r")) == NULL) {
      Abort = B_TRUE;
      sprintf(cbuf,
	      "OpticalFlowFieldFilter():Could not open node file \"%s\"",
	      fname);
      draw_mesg(cbuf);
      return;
    }

    /* loop to fill in mask pixels around nodes */
    NLines = 0;
    NNodes = 0;
    NValidPixels = 0;
    while (fgets(cbuf, CBUF_SIZE, fp) != NULL) {
      NLines++;
      if (fscanf(fp, "%le %le %le %le\n", &NodeXL, &NodeYL, &NodeXR, &NodeYR)
	  == 4) {
	/* convert nodes to X windows notation */
	NLines++;
	NNodes += 2;
	NodeYL = height*decimation - NodeYL - 1;
	NodeYR = height*decimation - NodeYR - 1;
	
	/* left node */
	for (y = rint((NodeYL - MaskRadius)/((double) decimation));
	     y <= rint((NodeYL + MaskRadius)/((double) decimation));
	     y++) {
	  if ((y >= 0) && (y < height)) {
	    Y = ((double) y) - (NodeYL/((double) decimation));
	    for (x = rint((NodeXL - MaskRadius)/((double) decimation));
		 x <= rint((NodeXL + MaskRadius)/((double) decimation));
		 x++) {
	      if ((x >= 0) && (x < width)) {
		X = ((double) x) - (NodeXL/((double) decimation));
		if (sqrt(X*X + Y*Y) <= (MaskRadius/((double) decimation))) {
		  MaskPixels[y*width + x] = 1.0;
		  NValidPixels++;
		}
	      }
	    }
	  }
	}
	
	/* right node */
	for (y = rint((NodeYR - MaskRadius)/((double) decimation));
	     y <= rint((NodeYR + MaskRadius)/((double) decimation));
	     y++) {
	  if ((y >= 0) && (y < height)) {
	    Y = ((double) y) - (NodeYR/((double) decimation));
	    for (x = rint((NodeXR - MaskRadius)/((double) decimation));
		 x <= rint((NodeXR + MaskRadius)/((double) decimation));
		 x++) {
	      if ((x >= 0) && (x < width)) {
		X = ((double) x) - (NodeXR/((double) decimation));
		if (sqrt(X*X + Y*Y) <= (MaskRadius/((double) decimation))) {
		  MaskPixels[y*width + x] = 1.0;
		  NValidPixels++;
		}
	      }
	    }
	  }
	}
      }
    }
    
    /* close node file */
    fclose(fp);
    
    /* flag mask definition */
    DisplayContourComputed = B_TRUE;
  }
  if (VerboseFlag)
    printf("OpticalFlowFieldFilter():%d lines, %d nodes, %d valid pixels...\n",
	   NLines, NNodes, NValidPixels);
  
  /* check that mask image is defined */
  if (DisplayContourComputed == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort: No pixel mask in OpticalFlowFieldFilter()");
    return;
  }
	
  /* write intermediate output if required */
  if (WriteIntermediateIPFilesFlag) {
    sprintf(fname,"%s_mask.mat", StrippedMantissa);
    WriteMatlabUShortFile(height, width, "mask", MaskPixels, fname);
  }
  
  /* threshold to eliminate outliers and compress dynamic range */
  parms[0] = (void *) &v0;
  parms[1] = (void *) &v1;
  parms[2] = (void *) &v2;
  parms[3] = (void *) &v3;
  v0 = -T;
  v1 =  T;
  v2 = -T;
  v3 =  T;
  Threshold(outpixels, outpixels, height, width, parms);
  
  /* push the decimated image onto the stack */
  XvgImgStack_Push(outpixels, height, width);
  
  /* generate the valid pixel mask using Laplacian */
  Laplacian(outpixels, inpixels, height, width, NULL);
  if (Abort)
    return;
  
  /* write intermediate output if required */
  if (WriteIntermediateIPFilesFlag) {
    sprintf(fname,"%s_00.mat", mantissa);
    WriteMatlabUShortFile(height, width, vname, inpixels, fname);
  }
  
  /* create valid pixel mask ... */
  v0 = LaplacianThreshold;
  parms[0] = (void *) &v0;
  BracketSD(inpixels, outpixels, height, width, parms);
  if (Abort)
    return;
  
  /* write intermediate output if required */
  if (WriteIntermediateIPFilesFlag) {
    sprintf(fname,"%s_01.mat", mantissa);
    WriteMatlabUShortFile(height, width, vname, outpixels, fname);
  }
  
  /* Erode pixel mask, catalog disjoint regions of connected pixels  */
  parms[0] = (void *) &vi0;
  parms[1] = (void *) &vi1;
  vi0 = 1;
  Erosion(outpixels, inpixels, height, width, parms);
  vi0 = -1;
  vi1 =  0;
  RegionFillSegmentation(inpixels, outpixels, height, width, parms);
  if (Abort)
    return;
  
  /* write intermediate output if required */
  if (WriteIntermediateIPFilesFlag) {
    sprintf(fname,"%s_02.mat", mantissa);
    WriteMatlabUShortFile(height, width, vname, outpixels, fname);
  }
  
  /* crate new pixel mask from largest regions, and dilate */
  parms[0] = (void *) &v0;
  v0 = -0.5;
  Binarize(outpixels, inpixels, height, width, parms);
  parms[0] = (void *) &vi0;
  vi0 = 1;
  Dilation(inpixels, outpixels, height, width, parms);
  if (Abort)
    return;
  
  /* write intermediate output if required */
  if (WriteIntermediateIPFilesFlag) {
    sprintf(fname,"%s_03.mat", mantissa);
    WriteMatlabUShortFile(height, width, vname, outpixels, fname);
  }
  
  /* multiply the mask by the original image ... */
  XvgImgStack_Push(outpixels, height, width);
  p = ImageStack[ImageStackTop-2].pixels;
  ImageStack[ImageStackTop-2].pixels =ImageStack[ImageStackTop-1].pixels;
  ImageStack[ImageStackTop-1].pixels = p;
  RPNMultiply(outpixels, inpixels, height, width, NULL);
  
  /* write intermediate output if required */
  if (WriteIntermediateIPFilesFlag) {
    sprintf(fname,"%s_04.mat", mantissa);
    WriteMatlabUShortFile(height, width, vname, inpixels, fname);
  }
  
  /* weighted averaging using valid pixels only */
  vi0 = kernw;
  WeightedAveragingFilter(inpixels, outpixels, height, width, parms);
  BooleanTmp = DisplayContourComputed;
  DisplayContourComputed = B_FALSE;
  RegionExpansion(outpixels, inpixels, height, width, NULL);
  DisplayContourComputed = BooleanTmp;
  
  /* write intermediate output if required */
  if (WriteIntermediateIPFilesFlag) {
    sprintf(fname,"%s_05.mat", mantissa);
    WriteMatlabUShortFile(height, width, vname, inpixels, fname);
  }
  
  /* median filter */
  vi0 = 3;
  MedianFilter(inpixels, outpixels, height, width, parms);
  if (Abort)
    return;
  
  /* final average smoothing */
  vi0 = kernw;
  AveragingFilter(outpixels, inpixels, height, width, parms);
  if (Abort)
    return;

  /* threshold to eliminate outliers and compress dynamic range */
  parms[0] = (void *) &v0;
  parms[1] = (void *) &v1;
  parms[2] = (void *) &v2;
  parms[3] = (void *) &v3;
  v0 = -T;
  v1 =  T;
  v2 = -T;
  v3 =  T;
  Threshold(inpixels, outpixels, height, width, parms);
  
  /* write out the filtered file */
  sprintf(fname, "%s_%s_d%02d_FILTERED.mat",
	  StrippedMantissa, vname, decimation);
  WriteMatlabUShortFile(height, width, vname, outpixels, fname);
}
  
/****************** PatchXCorrByAreaHierarchical  **********************
*
*   function : PatchXCorrByAreaHierarchical()
*
*   parameters (loaded):
*              *((char *) ParmPtrs[0])  = Outfilenames mantissa
*              *((int *)  ParmPtrs[1])  = Number of child processes in X
*              *((int *)  ParmPtrs[2])  = Number of child processes in Y
*              *((char *) ParmPtrs[0])  = Parameter filename
*
*   Parameter file format (8 columns)
*    Line 0: Comment (essential!)
*    Lines 1..n: "n" hierarchical refinement passes specifications...
*      Column 0  : Index (bogus number)
*      Column 1  : Decimation (MUST BE A POWER OF 2!)
*      Column 2  : Search lags in the X direction
*      Column 3  : Search lags in the Y direction
*      Column 4  : Search sampling increment (ex: 2 for FTKW=64, 1 otherwise)
*      Column 5  : FT kernel width                       (FTKW=32 or FTKW=64)
*      Column 6  : Max FT filter Radius   (10 for FTKW=32, or 20 for FTKW=64)
*      Column 7  : Laplacian filter SD multiplier     (0.5 for heavy, to 5.0)
*      Column 8  : Maximum number of cached FTs (0:No cache, else 1000 recom)
*      Column 8  : Enable filtering of hierarchical data estimates     (0:No)
*      Column11  : Write out intermediate filtering result files       (0:No)
*      Column12  : Write out dx/dy/VAF/ObjFunc files                   (0:No)
*
***********************************************************************/
boolean_t CombineHierarchicalFiles(char *vname, char *fname_mantissa,
				   int height, int width,
				   int *xs, int *ys, double *buffer,
				   int ngridx, int ngridy,
				   double InitVal, int HierarchicalPasses,
				   int decimation,
				   boolean_t InterpolateFlag,
				   boolean_t SaveCompressedFlag,
				   int FilterEnableFlag,
				   int WriteIntermediateIPFilesFlag,
				   double LaplacianThreshold,
				   double *scratch)
{
  boolean_t BooleanTmp;
  void *parms[4];
  double *inpixels, min, max, ScaleFactor, v0, v1, *p;
  char fname[256];
  int x, y, i, j, k, n, hh, ww, xmin, ymin, dheight, dwidth, vi0, vi1,
  xpad, ypad, r, iheight, iwidth, IFAIL;

  /* initialize output buffer */
  for (i = 0; i < height*width; i++)
    buffer[i] = InitVal;

  /* loop to load the partial data files */
  for (y = 0, n = 0; (y < ngridy) && (Abort == B_FALSE); y++) {
    for (x = 0; (x < ngridx) && (Abort == B_FALSE); x++, n++) {

      /* load the file */
      sprintf(fname, "%s_%02d_%s.mat", fname_mantissa, n, vname);
      if (GetMatlabFile(&hh, &ww, &min, &max,
			&inpixels, fname, VerboseFlag) == B_FALSE) {
	Abort = B_TRUE;
	sprintf(cbuf, "Abort: Can't open \"%s\" in CombineHierarchicalFiles()",
		fname);      
	draw_mesg(cbuf);
	return(B_FALSE);
      }

      /* delete the file */
      unlink(fname);
      
      /* copy the data to the complete image buffer */
      for (j = ys[n], k = 0; j < (ys[n] + hh); j++) {
	for (i = xs[n]; i < (xs[n] + ww); i++, k++) {
	  if (((j & (decimation-1)) == 0) && ((i & (decimation-1)) == 0)) {
	    buffer[(j/decimation)*(width/decimation) + (i/decimation)]
	      = inpixels[k];
	  }
	}
      }
    }
  }
  XvgMM_Free(inpixels);
  
  /* calculate the decimated image size parameters */
  dwidth = width/decimation;
  dheight = height/decimation;

  /* write out the file */
  sprintf(fname, "%s_%s_d%02d.mat", fname_mantissa, vname, decimation);
  if (SaveCompressedFlag)
    WriteMatlabUShortFile(dheight, dwidth, vname, buffer, fname);
  else
    WriteMatlabFile(dheight, dwidth, vname, buffer, fname);

  /* build interpolated image if required */
  if (InterpolateFlag && (Abort == B_FALSE)) {
    /* set FT parameters */
    FFTWidth = dwidth;
    FFTHeight = dheight;
    FFTWindow = HANNINGWINDOW;

    /* filter the data if required */
    if (FilterEnableFlag) {
      
      /* clear the stack */
      ImageStackTop = 0;

      /* scale the mask image to the current decimated size, if required */
      if (DisplayContourComputed) {
	/* push the mask image onto the stack */
	XvgImgStack_Push(MaskPixels, height, width);

	/* decimate the mask */
	for (y = 0, k = 0; y < height; y += decimation) {
	  for (x = 0; x < width; x += decimation, k++) {
	    scratch[k] = MaskPixels[y*width + x];
	  }
	}

	/* realod mask image with decimated data */
	for (k = 0; k < dheight*dwidth; k++) {
	  MaskPixels[k] = scratch[k];
	}

#ifdef NOTHIS
	/* write out the decimated mask if required */
	if (WriteIntermediateIPFilesFlag) {
	  sprintf(fname,"%s_%s_d%02d_mask.mat",
		  fname_mantissa, vname, decimation);
	  WriteMatlabUShortFile(dheight, dwidth, vname, MaskPixels, fname);
	}
#endif
      }
	
      /* push the decimated image onto the stack */
      MoveDoubles(buffer, scratch, dheight*dwidth);
      XvgImgStack_Push(scratch, dheight, dwidth);

      /* generate the valid pixel mask using Laplacian */
      Laplacian(scratch, buffer, dheight, dwidth, NULL);
      if (Abort)
	return(B_FALSE);
      
      /* write intermediate output if required */
      if (WriteIntermediateIPFilesFlag) {
	sprintf(fname,"%s_%s_d%02d_00.mat",
		fname_mantissa, vname, decimation);
	WriteMatlabUShortFile(dheight, dwidth, vname, buffer, fname);
      }
      
      /* create valid pixel mask ... */
      v0 = LaplacianThreshold;
      parms[0] = (void *) &v0;
      BracketSD(buffer, scratch, dheight, dwidth, parms);
      if (Abort)
	return(B_FALSE);
      
      /* write intermediate output if required */
      if (WriteIntermediateIPFilesFlag) {
	sprintf(fname,"%s_%s_d%02d_01.mat",
		fname_mantissa, vname, decimation);
	WriteMatlabUShortFile(dheight, dwidth, vname, scratch, fname);
      }
      
      /* Erode pixel mask, catalog disjoint regions of connected pixels  */
      parms[0] = (void *) &vi0;
      parms[1] = (void *) &vi1;
      vi0 = 1;
      Erosion(scratch, buffer, dheight, dwidth, parms);
      vi0 = -1;
      vi1 =  0;
      RegionFillSegmentation(buffer, scratch, dheight, dwidth, parms);
      if (Abort)
	return(B_FALSE);

      /* write intermediate output if required */
      if (WriteIntermediateIPFilesFlag) {
	sprintf(fname,"%s_%s_d%02d_02.mat",
		fname_mantissa, vname, decimation);
	WriteMatlabUShortFile(dheight, dwidth, vname, scratch, fname);
      }
      
      /* crate new pixel mask from largest regions, and dilate */
      parms[0] = (void *) &v0;
      v0 = -0.5;
      Binarize(scratch, buffer, dheight, dwidth, parms);
      parms[0] = (void *) &vi0;
      vi0 = 1;
      Dilation(buffer, scratch, dheight, dwidth, parms);
      if (Abort)
	return(B_FALSE);
      
      /* write intermediate output if required */
      if (WriteIntermediateIPFilesFlag) {
	sprintf(fname,"%s_%s_d%02d_03.mat",
		fname_mantissa, vname, decimation);
	WriteMatlabUShortFile(dheight, dwidth, vname, scratch, fname);
      }
    
      /* multiply the mask by the original image ... */
      XvgImgStack_Push(scratch, dheight, dwidth);
      p = ImageStack[ImageStackTop-2].pixels;
      ImageStack[ImageStackTop-2].pixels =ImageStack[ImageStackTop-1].pixels;
      ImageStack[ImageStackTop-1].pixels = p;
      RPNMultiply(scratch, buffer, dheight, dwidth, NULL);
      
      /* write intermediate output if required */
      if (WriteIntermediateIPFilesFlag) {
	sprintf(fname,"%s_%s_d%02d_04.mat", fname_mantissa, vname, decimation);
	WriteMatlabUShortFile(dheight, dwidth, vname, buffer, fname);
      }
      
      /* weighted averaging using valid pixels only */
      vi0 = 3;
      WeightedAveragingFilter(buffer, scratch, dheight, dwidth, parms);
      BooleanTmp = DisplayContourComputed;
      DisplayContourComputed = B_FALSE;
      RegionExpansion(scratch, buffer, dheight, dwidth, NULL);
      DisplayContourComputed = BooleanTmp;
      
      /* write intermediate output if required */
      if (WriteIntermediateIPFilesFlag) {
	sprintf(fname,"%s_%s_d%02d_05.mat", fname_mantissa, vname, decimation);
	WriteMatlabUShortFile(dheight, dwidth, vname, buffer, fname);
      }
      
      /* median filter */
      MedianFilter(buffer, scratch, dheight, dwidth, parms);
      if (Abort)
	return(B_FALSE);

      /* final average smoothing */
      AveragingFilter(scratch, buffer, dheight, dwidth, parms);
      if (Abort)
	return(B_FALSE);
    
      /* write out the filtered file */
      sprintf(fname, "%s_%s_d%02d_FILTERED.mat",
	      fname_mantissa, vname, decimation);
      WriteMatlabFile(dheight, dwidth, vname, buffer, fname);

      /* pop the mask image, if required */
      if (DisplayContourComputed) {
	ImageStackTop--;
	for (k = 0; k < height*width; k++)
	  MaskPixels[k] = ImageStack[0].pixels[k];
      }
    }
    
    /* check for Abort flagged */
    if (Abort)
      return(B_FALSE);

    /* scale the image using Fourier domain interpolation if required */
    if (decimation > 1)  {
      /* check for odd-sized images */
      if ((dwidth & 0x01) != 0) {
	xmin = 1;
	dwidth--;
      }
      else {
	xmin = 0;
      }
      if ((dheight & 0x01) != 0) {
	ymin = 1;
	dheight--;
      }
      else {
	ymin = 0;
      }
      
      /* set FT parameters */
      FFTWidth = dwidth;
      FFTHeight = dheight;
      FFTWindow = HANNINGWINDOW;
      
      /* calculate interpolated image size parameters */
      iwidth = dwidth * decimation;
      iheight = dheight * decimation;
      
      /* crop the valid data into the FFT buffers */
      for (j = ymin, k = 0; j < (ymin + dheight); j++)
	for (i = xmin; i < (xmin + dwidth); i++, k++) {
	  FFTRealBuffer[k] = buffer[j*(width/decimation) + i];
	  FFTImBuffer[k] = 0;
	}
      
      /* compute FT of the decimated image */
      IFAIL = 1;
      c06fuf(&dwidth, &dheight, FFTRealBuffer, FFTImBuffer,
	     "Initial", FFTTrigm, FFTTrign, FFTWork, &IFAIL);
      if (IFAIL != 0) {
	Abort = B_TRUE;
	sprintf(cbuf,
		"Abort:IFAIL=%d, c06fuf(%s) in CombineHierarchicalFiles()",
		IFAIL, vname);
	draw_mesg(cbuf);
	return(B_FALSE);
      }
      
      /* flip the quadrants of the decimated image FT */
      FFTShift(FFTRealBuffer, FFTRealBuffer, dheight, dwidth);
      FFTShift(FFTImBuffer, FFTImBuffer, dheight, dwidth);
      
      /* clear the interpolated FT */
      for (k = 0; k < iheight*iwidth; k++) {
	FFTRealBuffer2[k] = 0;
	FFTImBuffer2[k] = 0;
      }
      
      /* transfer the data to the interpolated FT buffers */
      ScaleFactor = sqrt(((double)(iheight*iwidth))
			 /((double)(dheight*dwidth)));
      xpad = (iwidth - dwidth)/2;
      ypad = (iheight - dheight)/2;
      for (y = ypad, k = 0; y < (iheight - ypad); y++) {
	for (x = xpad; x < (iwidth - xpad); x++, k++) {
	  FFTRealBuffer2[y*iwidth + x] = FFTRealBuffer[k]*ScaleFactor;
	  FFTImBuffer2[y*iwidth + x] = FFTImBuffer[k]*ScaleFactor;
	}
      }
      
      /* un-flip quadrants */
      FFTShift(FFTRealBuffer2, FFTRealBuffer2, iheight, iwidth);
      FFTShift(FFTImBuffer2, FFTImBuffer2, iheight, iwidth);
      
      /* inverse FFT */
      n = iheight*iwidth;
      IFAIL = 1;
      c06gcf(FFTImBuffer2, &n, &IFAIL);
      if (IFAIL != 0) {
	Abort = B_TRUE;
	sprintf(cbuf,
		"Abort : IFAIL = %d, c06gcf(%s) in CombineHierarchicalFiles()",
		IFAIL, vname);
	draw_mesg(cbuf);
	return(B_FALSE);
      }
      IFAIL = 1;
      c06fuf(&iwidth, &iheight, FFTRealBuffer2, FFTImBuffer2,
	     "Initial", FFTTrigm2, FFTTrign2, FFTWork2, &IFAIL);
      if (IFAIL != 0) {
	Abort = B_TRUE;
	sprintf(cbuf,
		"Abort:IFAIL=%d, c06fuf(%s, 2) in CombineHierarchicalFiles() ",
		IFAIL, vname);
	draw_mesg(cbuf);
	return(B_FALSE);
      }
      
      /* copy the interpolated data to the output buffer */
      for (k = 0; k < height*width; k++)
	FFTImBuffer2[k] = InitVal;
      xpad = (width - iwidth)/2;
      ypad = (height - iheight)/2;
      for (y = ypad + ymin*decimation, k = 0; y < (height - ypad); y++)
	for (x = xpad + xmin*decimation; x < (width - xpad); x++, k++)
	  FFTImBuffer2[y*width + x] = FFTRealBuffer2[k];
      
      /* save the interpoplated file */
      sprintf(fname, "%s_%s_INTERPOLATED_d%02d.mat",
	      fname_mantissa, vname, decimation);
      WriteMatlabFile(height, width, vname, FFTImBuffer2, fname);
    }
  }

  /* check memory */
  sprintf(cbuf, "CombineDataFiles(%s, %s, 0x%x)", vname, fname_mantissa,
	  (int) buffer);
  XvgMM_MemoryCheck(cbuf);

  /* return success */
  return(B_TRUE);
}

void PatchXCorrByAreaHierarchical(double *inpixels, double *outpixels,
				  int height, int width, void *ParmPtrs[])
{
  struct sembuf SemOperation;
  FILE *fpi, *fpo;
  void *parms_xc[4];
  char fname[256], OutFname[256];
  double WindowRadiiMax[MAXHPASSES], WindowRadiusMin, *RecombinationBuffer,
  t0, t1, m, max, min, scale, LaplacianThresholds[MAXHPASSES];
  int i, j, k, iv, lFFTWindow, zero, status, pp,
  XStart, YStart, XInc, YInc, x, y, ngridx, ngridy, ival,
  XOffset, YOffset, XExtent, YExtent, HierarchicalPasses, HierarchicalPassesN,
  xs[PROCESSORS_NMAX], ys[PROCESSORS_NMAX], index,
  Decimations[MAXHPASSES], FTKernelWidths[MAXHPASSES], LagsX[MAXHPASSES],
  SamplingIncs[MAXHPASSES], FFTCacheSizes[MAXHPASSES], LagsY[MAXHPASSES],
  WriteSecondaryFiles[MAXHPASSES], WriteIntermediateIPFiles[MAXHPASSES],
  FilterEnable[MAXHPASSES];

  /* check that pixel mask defined */
  if (DisplayContourComputed == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort: No pixel mask in PatchXCorrByAreaHierarchical()");
    return;
  }

  /* check that stack is not empty */
  if (ImageStackTop <= 0) {
    Abort = B_TRUE;
    draw_mesg("Abort: Stack image required in PatchXCorrByAreaHierarchical()");
    return;
  }

  /* load input parameters */
  sprintf(OutFname, "%s", ParmPtrs[0]);
  ngridx = *((int *) ParmPtrs[1]);
  ngridy = *((int *) ParmPtrs[2]);
  nprocessors = ngridx*ngridy;
  npids = nprocessors;

  /* check that we have 16 processors available */
  if (nprocessors > PROCESSORS_NMAX) {
    Abort = B_TRUE;
    draw_mesg("Abort: max procs exceeded in PatchXCorrByAreaHierarchical()");
    return;
  }
  
  /* open the parameter file */
  if ((fpi = fopen(ParmPtrs[3], "r")) == NULL) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "PatchXCorrByAreaHierarchical():Could not open file \"%s\"",
	    ParmPtrs[3]);
    draw_mesg(cbuf);
    fclose(fpi);
    return;
  }
  
  /* read in the parameter file lines */
  fgets(cbuf, CBUF_SIZE, fpi);
  HierarchicalPassesN = 0;
  while ((fscanf(fpi, "%d %d %d %d %d %d %le %le %d %d %d %d\n",
		 &index,
		 &(Decimations[HierarchicalPassesN]),
		 &(LagsX[HierarchicalPassesN]),
		 &(LagsY[HierarchicalPassesN]),
		 &(SamplingIncs[HierarchicalPassesN]),
		 &(FTKernelWidths[HierarchicalPassesN]),
		 &(WindowRadiiMax[HierarchicalPassesN]),
		 &(LaplacianThresholds[HierarchicalPassesN]),
		 &(FFTCacheSizes[HierarchicalPassesN]),
		 &(FilterEnable[HierarchicalPassesN]),
		 &(WriteIntermediateIPFiles[HierarchicalPassesN]),
		 &(WriteSecondaryFiles[HierarchicalPassesN])) == 12)
	 && (HierarchicalPassesN < MAXHPASSES)) {
    HierarchicalPassesN++;
  }
  
  /* close the parameter file */
  fclose(fpi);
  
  /* set fixed parameters */
  WindowRadiusMin = 1.0;
  lFFTWindow = FFTWindow;
  FFTCache = NULL;
  FFTCacheBuffer = NULL;
  FFTCacheTable = NULL;
  FFTCacheRequests = NULL;

#ifdef NOTHIS
  /* init FFT buffers */
  if (FFTInit(height, width) == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort : FFTInit() failed in PatchXCorrByAreaHierarchical()");
    return;
  }
  if (FFTInit2(height, width) == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort : FFTInit2() failed in PatchXCorrByAreaHierarchical()");
    return;
  }
#endif

  /* determine gross image shift using cross-correlation and Laplacian */
  FFTWindow = HANNINGWINDOW;
  zero = 0;
  parms_xc[0] = (void *) &zero;
#ifdef NOTHIS
  CrossCorrelation(inpixels, outpixels, height, width, parms_xc);
  ImageStackTop++;
  Laplacian(outpixels, DoubleBuffer, height, width, NULL);
  if (Abort)
    return;
  
  /* find max in the Laplacian and use as estimate of gross shift */
  for (j = 4, m = -MAXDOUBLE; j < (height-4); j++) {
    for (i = 4; i < (width-4); i++) {
      if (DoubleBuffer[j*width + i] > m) {
	m = DoubleBuffer[j*width + i];
	x = i;
	y = j;
      }
    }
  }
  XOffset = -(width/2  - x);  /* X offset */
  YOffset = -(height/2 - y);  /* Y offset */
#endif
  XOffset = 0;  /* X offset */
  YOffset = 0;  /* Y offset */

  /* write out the initial offset to a file */
  sprintf(cbuf, "%s_XYOffset.doc", OutFname);
  if ((fpo = fopen(cbuf, "w")) == NULL) {
    sprintf(cbuf, "PatchXCorrByAreaHierarchical(): Could not open the file %s",
	    cbuf);
    draw_mesg(cbuf);
    return;
  }
  fprintf(fpo, "XOffset:%d, YOffset:%d\n", XOffset, YOffset);
  fclose(fpo);

  /* Normalize images. This improves performance slightly */
  draw_mesg("PatchXCorrByAreaHierarchical() : Normalizing...");
  for (k = 0, max = -MAXDOUBLE, min = MAXDOUBLE; k < height*width; k++) {
    if (inpixels[k] < min)
      min = inpixels[k];
    if ((ImageStack[ImageStackTop-1].pixels)[k] < min)
      min = (ImageStack[ImageStackTop-1].pixels)[k];
    if (inpixels[k] > max)
      max = inpixels[k];
    if ((ImageStack[ImageStackTop-1].pixels)[k] > max)
      max = (ImageStack[ImageStackTop-1].pixels)[k];
  }
  scale = 1.0 / (max - min);
  for (k = 0; k < height*width; k++) {
    inpixels[k] = (inpixels[k] - min) * scale;
    outpixels[k] = ((ImageStack[ImageStackTop-1].pixels)[k] - min) * scale;
  }

  /* allocate storage space */
  if ((RecombinationBuffer = (double *) XvgMM_Alloc((void *) NULL,
						    height*width
						    *sizeof(double)))
      == NULL) {
    Abort = B_TRUE;
    draw_mesg("Abort : Out of memory in PatchXCorrByAreaHierarchical()");
    return;
  }

  /* create the child and parent process semaphores */
  if ((SemID = semget(IPC_PRIVATE, nprocessors+1, IPC_CREAT | S_IRWXU))
      == -1) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "Abort(semget %d): \"Error %d : %s\" in PatchXCorrByAreaHierarchical()",
	    nprocessors+1, errno, strerror(errno));
    draw_mesg(cbuf);
    semctl(SemID, 0, IPC_RMID, 0);
    return;
  }
  if (VerboseFlag)
    printf("PatchXCorrByAreaHierarchical() : Created semaphore %d (%d)...\n",
	   SemID, nprocessors+1);

  /* set all child semaphores to 1 so that they wait when created */
  for (k = 0; k < nprocessors; k++) {
    /* set semaphore values to 1 */
    SemOperation.sem_num = k;
    SemOperation.sem_op  = 1;
    SemOperation.sem_flg = 0;
    if (semop(SemID, &SemOperation, 1) == -1) {
      Abort = B_TRUE;
      sprintf(cbuf,
	      "Abort: \"Error %d : %s\" for semaphore %d in PatchXCorrByAreaHierarchical()",
	      errno, strerror(errno), k);
      draw_mesg(cbuf);
      semctl(SemID, 0, IPC_RMID, 0);
      return;
    }
    /* check semaphore value */
    if ((ival = semctl(SemID, k, GETVAL, 1)) != 1) {
      Abort = B_TRUE;
      sprintf(cbuf,
	      "Abort: semaphore %d = %d in PatchXCorrByAreaHierarchical()",
	      k, ival);
      draw_mesg(cbuf);
      semctl(SemID, 0, IPC_RMID, 0);
      return;
    }
  }
  
  /* loop to start the child processes in the vertical direction */
  for (j=0, ChildProcessNumber = 0, XInc = width/ngridx, YInc = height/ngridy;
       j < ngridy; j++) {

    /* calculate Y parameters for this patch */
    YStart = j*YInc;
    if (j < (ngridy-1))
      YExtent = YInc;
    else
      YExtent = height - (ngridy-1)*YInc; /* Yextent */

    /* loop for the horizontal direction */
    for (i = 0; i < ngridx; i++, ChildProcessNumber++) {

      /* show banner */
      sprintf(cbuf,"PatchXCorrByAreaHierarchical(): Starting child process %d",
	      ChildProcessNumber);
      draw_mesg(cbuf);

      /* calculate X parameters for this patch */
      XStart = i*XInc;
      if (i < (ngridx-1))
	XExtent = XInc;
      else
	XExtent = width - (ngridx-1)*XInc; /* Xextent */

      /* start the child process */
      ChildProcess = B_TRUE;
      if ((pids[ChildProcessNumber] = fork()) == 0) {
	PatchXCorrHierarchical(inpixels, outpixels, height, width, OutFname,
			       XStart, XStart + XExtent - 1,
			       YStart, YStart + YExtent - 1,
			       XOffset, YOffset, WindowRadiusMin,
			       Decimations, LagsX, LagsY, FTKernelWidths,
			       SamplingIncs, WindowRadiiMax,
			       FFTCacheSizes, WriteSecondaryFiles,
			       HierarchicalPassesN);
	if (Abort)
	  exit(EXIT_FAILURE);
	else
	  exit(EXIT_SUCCESS);
      }
      ChildProcess = B_FALSE;
    }
  }
  
  /* loop for hierarchical refinements */
  for (HierarchicalPasses = 0;
       (HierarchicalPasses < HierarchicalPassesN) && (Abort == B_FALSE);
       HierarchicalPasses++) {
    
    /* get start time */
    t0 = XvgGetTime();

    /* set the global semaphore value to the number of child processes */
    SemOperation.sem_num = nprocessors;
    SemOperation.sem_op  = 1;
    SemOperation.sem_flg = 0;
    for (k = 0; k < nprocessors; k++) {
      if (semop(SemID, &SemOperation, 1) == -1) {
	Abort = B_TRUE;
	sprintf(cbuf,
		"Abort: \"Error %d : %s\" for glbl semaphore in PatchXCorrByAreaHierarchical()",
		errno, strerror(errno));
	draw_mesg(cbuf);
	StopChildren();
	semctl(SemID, 0, IPC_RMID, 0);
	return;
      }
    }
    /* check semaphore value */
    if ((ival = semctl(SemID, nprocessors, GETVAL, 1)) != nprocessors) {
      Abort = B_TRUE;
      sprintf(cbuf,
	      "Abort: semaphore %d = %d in PatchXCorrByAreaHierarchical()",
	      nprocessors, ival);
      draw_mesg(cbuf);
      StopChildren();
      semctl(SemID, 0, IPC_RMID, 0);
      return;
    }
    if (VerboseFlag)
      printf("PatchXCorrByAreaHierarchical() : Global sem set to %d...\n",
	     nprocessors);

    /* show progress */
    printf("Executing pass %d/%d -> DEC = %2d, SMPL = %d, LAGSX = %2d, LAGSY = %2d, FTKW = %2d, FTFR = %.2f, LPLT = %.2e, CS = %d...\n",
	   HierarchicalPasses+1, HierarchicalPassesN,
	   Decimations[HierarchicalPasses],
	   SamplingIncs[HierarchicalPasses],
	   LagsX[HierarchicalPasses],
	   LagsY[HierarchicalPasses],
	   FTKernelWidths[HierarchicalPasses],
	   WindowRadiiMax[HierarchicalPasses],
	   LaplacianThresholds[HierarchicalPasses],
	   FFTCacheSizes[HierarchicalPasses]);
    
    /* show banner */
    sprintf(cbuf,"PatchXCorrByAreaHierarchical(): Looping on pass %d/%d",
	    HierarchicalPasses < HierarchicalPassesN);
    draw_mesg(cbuf);

    /* clear the child semaphores to fire up the child processes */
    for (k = 0; k < nprocessors; k++) {
      SemOperation.sem_num = k;
      SemOperation.sem_op  = -1;
      SemOperation.sem_flg = 0;
      if (semop(SemID, &SemOperation, 1) == -1) {
	Abort = B_TRUE;
	sprintf(cbuf,
		"Abort: \"Error %d : %s\" for semaphore %d in PatchXCorrByAreaHierarchical()",
		errno, strerror(errno), k);
	draw_mesg(cbuf);
	StopChildren();
	semctl(SemID, 0, IPC_RMID, 0);
	return;
      }
    }
    
    /* wait on the global semaphore until all child processes are idle */
    if (VerboseFlag)
      printf("PatchXCorrByAreaHierarchical() : Waiting on global sem...\n");
    SemOperation.sem_num = nprocessors;
    SemOperation.sem_op  = 0;
    SemOperation.sem_flg = 0;
    if (semop(SemID, &SemOperation, 1) == -1) {
      Abort = B_TRUE;
      sprintf(cbuf,
	      "Abort: \"Error %d : %s\" waiting for gblsem in PatchXCorrByAreaHierarchical()",
	      errno, strerror(errno));
      draw_mesg(cbuf);
      StopChildren();
      semctl(SemID, 0, IPC_RMID, 0);
      return;
    }

    /* remove the files from the previous pass, is required */
    if (HierarchicalPasses == 0) {
      if (WriteIntermediateIPFiles[HierarchicalPasses] == 0) {
	sprintf(fname, "%s_XYOffset.doc", OutFname);
	unlink(fname);
      }
    }
    else {
      if (WriteIntermediateIPFiles[HierarchicalPasses - 1] == 0) {
	sprintf(fname, "%s_README_d%02d.doc",
		OutFname, Decimations[HierarchicalPasses - 1]);
	unlink(fname);
	sprintf(fname, "%s_shiftsX_d%02d.mat",
		OutFname, Decimations[HierarchicalPasses - 1]);
	unlink(fname);
	sprintf(fname, "%s_shiftsX_d%02d_FILTERED.mat",
		OutFname, Decimations[HierarchicalPasses - 1]);
	unlink(fname);
	sprintf(fname, "%s_shiftsX_INTERPOLATED_d%02d.mat",
		OutFname, Decimations[HierarchicalPasses - 1]);
	unlink(fname);
	sprintf(fname, "%s_shiftsY_d%02d.mat",
		OutFname, Decimations[HierarchicalPasses - 1]);
	unlink(fname);
	sprintf(fname, "%s_shiftsY_d%02d_FILTERED.mat",
		OutFname, Decimations[HierarchicalPasses - 1]);
	unlink(fname);
	sprintf(fname, "%s_shiftsY_INTERPOLATED_d%02d.mat",
		OutFname, Decimations[HierarchicalPasses - 1]);
	unlink(fname);
      }
    }

    /* get stop time */
    t1 = XvgGetTime();

    /* recombine README files */
    if (Abort == FALSE) {
      sprintf(fname, "%s_README_d%02d.doc",
	      OutFname, Decimations[HierarchicalPasses]);
      if ((fpo = fopen(fname, "w")) == NULL) {
	Abort = B_TRUE;
	sprintf(cbuf,
		"Abort:Couldn't open \"%s\" in PatchXCorrByAreaHierarchical()",
		fname);
	draw_mesg(cbuf);
	StopChildren();
	semctl(SemID, 0, IPC_RMID, 0);
	return;
      }
      fprintf(fpo, "PARALLEL PROCESSES: %d (%d x %d)\n",
	      nprocessors, ngridx, ngridy);
      fprintf(fpo, "\nPass %2d/%2d processing time   = %.2f minutes\n",
	      HierarchicalPasses+1, HierarchicalPassesN, (t1 - t0)/60.0);
      fprintf(fpo, "InFileName                   = %s\n", InFileName);
      fprintf(fpo, "Decimation                   = %d\n",
	      Decimations[HierarchicalPasses]);
      fprintf(fpo, "Search lags X                = %d\n",
	      LagsX[HierarchicalPasses]);
      fprintf(fpo, "Search lags Y                = %d\n",
	      LagsY[HierarchicalPasses]);
      fprintf(fpo, "Search sampling increment    = %d\n",
	      SamplingIncs[HierarchicalPasses]);
      fprintf(fpo, "FT kernel width              = %d\n",
	      FTKernelWidths[HierarchicalPasses]);
      fprintf(fpo, "WindowRadiusMax              = %.2f\n",
	      WindowRadiiMax[HierarchicalPasses]);
      fprintf(fpo, "WindowRadiusMin              = %.2f\n", WindowRadiusMin);
      fprintf(fpo, "LaplacianThresholds          = %.2e\n",
	      LaplacianThresholds[HierarchicalPasses]);
      fprintf(fpo, "\n***********\n\n");
      for (pp = 0; pp < nprocessors; pp++) {
	/* show header */
	fprintf(fpo, "PROCESS #%d:\n", pp);
	
	/* load the file */
	sprintf(fname, "%s_%02d_README.doc", OutFname, pp);
	if ((fpi = fopen(fname, "r")) == NULL) {
	  Abort = B_TRUE;
	  sprintf(cbuf,
		  "Abort: Couldn't open %s in PatchXCorrByAreaHierarchical()",
		  fname);
	  draw_mesg(cbuf);
	  StopChildren();
	  semctl(SemID, 0, IPC_RMID, 0);
	  return;
	}
	
	/* extract the XStartI1/YStartI1 fields */
	fgets(cbuf, 512, fpi);
	if (sscanf(cbuf, "XStartI1 = %d", &iv) != 1) {
	  Abort = B_TRUE;
	  sprintf(cbuf,
		  "Abort: can't load xs from %s in PatchXCorrByAreaHierarchical()",
		  fname);
	  draw_mesg(cbuf);
	  StopChildren();
	  semctl(SemID, 0, IPC_RMID, 0);
	  return;
	}
	xs[pp] = iv;
	fgets(cbuf, 512, fpi);
	if (sscanf(cbuf, "YStartI1 = %d", &iv) != 1) {
	  Abort = B_TRUE;
	  sprintf(cbuf,
		  "Abort: Couldn't load ys from file %s in PatchXCorrByAreaHierarchical()",
		  fname);
	  draw_mesg(cbuf);
	  StopChildren();
	  semctl(SemID, 0, IPC_RMID, 0);
	  return;
	}
	ys[pp] = iv;
	fseek(fpi, 0, 0);
	
	/* copy the data to the output file */
	while (fgets(cbuf, 512, fpi) != NULL)
	  fputs(cbuf, fpo);
	if (pp < (nprocessors -1))
	  fprintf(fpo, "\n***********\n\n");
	
	/* close and delete the file */
	fclose(fpi);
	unlink(fname);
      }
      fclose(fpo);
    }

    /* recombine child process output data files */
    if (Abort == FALSE)
      CombineHierarchicalFiles("shiftsX", OutFname, height, width,
			       xs, ys, RecombinationBuffer, ngridx, ngridy,
			       0.0, HierarchicalPasses,
			       Decimations[HierarchicalPasses],
			       B_TRUE, B_FALSE,
			       FilterEnable[HierarchicalPasses],
			       WriteIntermediateIPFiles[HierarchicalPasses],
			       LaplacianThresholds[HierarchicalPasses],
			       outpixels);
    if (Abort == FALSE)
      CombineHierarchicalFiles("shiftsY", OutFname, height, width,
			       xs, ys, RecombinationBuffer, ngridx, ngridy,
			       0.0, HierarchicalPasses,
			       Decimations[HierarchicalPasses],
			       B_TRUE, B_FALSE,
			       FilterEnable[HierarchicalPasses],
			       WriteIntermediateIPFiles[HierarchicalPasses],
			       LaplacianThresholds[HierarchicalPasses],
			       outpixels);
    if (WriteSecondaryFiles[HierarchicalPasses]) {
      if (Abort == FALSE)
	CombineHierarchicalFiles("objsfuncs", OutFname, height, width,
				 xs, ys, RecombinationBuffer, ngridx, ngridy,
				 0.01, HierarchicalPasses,
				 Decimations[HierarchicalPasses],
				 B_FALSE, B_FALSE,
				 FilterEnable[HierarchicalPasses],
				 WriteIntermediateIPFiles[HierarchicalPasses],
				 LaplacianThresholds[HierarchicalPasses],
				 outpixels);
      if (Abort == FALSE)
	CombineHierarchicalFiles("VAFs", OutFname, height, width,
				 xs, ys, RecombinationBuffer, ngridx, ngridy,
				 0.0, HierarchicalPasses,
				 Decimations[HierarchicalPasses],
				 B_FALSE, B_TRUE,
				 FilterEnable[HierarchicalPasses],
				 WriteIntermediateIPFiles[HierarchicalPasses],
				 LaplacianThresholds[HierarchicalPasses],
				 outpixels);
      if (Abort == FALSE)
	CombineHierarchicalFiles("dxs", OutFname, height, width,
				 xs, ys, RecombinationBuffer, ngridx, ngridy,
				 0.0, HierarchicalPasses,
				 Decimations[HierarchicalPasses],
				 B_FALSE, B_TRUE,
				 FilterEnable[HierarchicalPasses],
				 WriteIntermediateIPFiles[HierarchicalPasses],
				 LaplacianThresholds[HierarchicalPasses],
				 outpixels);
      if (Abort == FALSE)
	CombineHierarchicalFiles("dys", OutFname, height, width,
				 xs, ys, RecombinationBuffer, ngridx, ngridy,
				 0.0, HierarchicalPasses,
				 Decimations[HierarchicalPasses],
				 B_FALSE, B_TRUE,
				 FilterEnable[HierarchicalPasses],
				 WriteIntermediateIPFiles[HierarchicalPasses],
				 LaplacianThresholds[HierarchicalPasses],
				 outpixels);
    }
    
    /* show elapsed time */
    printf("  Done. Elapsed time = %.2f minutes.\n", (t1 - t0)/60.0);
  }
  
  /* remove the interpolated ShiftX and ShiftY image files, if required */
  if (WriteIntermediateIPFiles[HierarchicalPasses - 1] == 0) {
    sprintf(fname, "%s_shiftsX_INTERPOLATED_d%02d.mat",
	    OutFname, Decimations[HierarchicalPasses - 1]);
    unlink(fname);
    sprintf(fname, "%s_shiftsY_INTERPOLATED_d%02d.mat",
	    OutFname, Decimations[HierarchicalPasses - 1]);
    unlink(fname);
  }

  /* restore FFT window */
  FFTWindow = lFFTWindow;

  /* show no more children running */
  if (Abort)
    StopChildren();

  /* clean up */
  semctl(SemID, 0, IPC_RMID, 0);
  npids = 0;
  XvgMM_Free(RecombinationBuffer);

  /* restore output buffer */
  for (i = 0; i < height*width; i++)
    outpixels[i] = inpixels[i];
}


/******************     OpticalFlowNodesOverlay    *********************
*
*   function : OpticalFlowNodesOverlay
*
***********************************************************************/
static void OpticalFlowNodesOverlay(Widget DrawingArea)
{
  double d;
  int i;

  /* set field/frame multiplier */
  d = 1.0;

  /* loop to draw the node points and index numbers */
  if (InMemoryPixmapFlag) {
    for (i = 0; i < NOpticalFlowNodes; i+=2) {
      /* left node */
      if (OpticalFlowNodesWriteIndicies) {
	sprintf(cbuf, "%d", OpticalFlowNodesI[i]);
        XDrawString(UxDisplay, InMemoryPixmap, gc_YellowBlack,
		    ((int) rint(OpticalFlowNodesX[i]/d *((double)ZoomFactor)))+2,
		    ((int) rint(OpticalFlowNodesY[i]/d *((double)ZoomFactor)))+8,
		    cbuf, strlen(cbuf));
      }
      if (MarkerCircleRadius) {
	XDrawArc(UxDisplay, InMemoryPixmap, gc_YellowBlack,
		 ((int) rint(OpticalFlowNodesX[i]/d * ((double) ZoomFactor)))
		 - MarkerCircleRadius,
		 ((int) rint(OpticalFlowNodesY[i]/d * ((double) ZoomFactor)))
		 - MarkerCircleRadius,
		 2*MarkerCircleRadius, 2*MarkerCircleRadius, 1, 360*64);
      }
      else {
	XDrawPoint(UxDisplay, InMemoryPixmap, gc_YellowBlack,
		   ((int) rint(OpticalFlowNodesX[i]/d * ((double) ZoomFactor))),
		   ((int) rint(OpticalFlowNodesY[i]/d * ((double) ZoomFactor))));
      }
      
      /* right node */
      if (OpticalFlowNodesWriteIndicies) {
	sprintf(cbuf, "%d", OpticalFlowNodesI[i]);
	XDrawString(UxDisplay, InMemoryPixmap, gc_YellowBlack,
		    ((int)rint(OpticalFlowNodesX[i+1]/d*((double)ZoomFactor)))+2,
		    ((int)rint(OpticalFlowNodesY[i+1]/d*((double)ZoomFactor)))+8,
		    cbuf, strlen(cbuf));
      }
      if (MarkerCircleRadius) {
	XDrawArc(UxDisplay, InMemoryPixmap, gc_YellowBlack,
		 ((int) rint(OpticalFlowNodesX[i+1]/d * ((double) ZoomFactor)))
		 - MarkerCircleRadius,
		 ((int) rint(OpticalFlowNodesY[i+1]/d * ((double) ZoomFactor)))
		 - MarkerCircleRadius,
		 2*MarkerCircleRadius, 2*MarkerCircleRadius,
		 1, 360*64);
      }
      else {
	XDrawPoint(UxDisplay, InMemoryPixmap, gc_YellowBlack,
		   ((int) rint(OpticalFlowNodesX[i+1]/d *((double)ZoomFactor))),
		   ((int) rint(OpticalFlowNodesY[i+1]/d *((double)ZoomFactor))));
      }
    }
  }
  else {
    for (i = 0; i < NOpticalFlowNodes; i+=2) {
      /* left node */
      if (OpticalFlowNodesWriteIndicies) {
	sprintf(cbuf, "%d", OpticalFlowNodesI[i]);
	XDrawString(UxDisplay, XtWindow(DrawingArea), gc_YellowBlack,
		    ((int) rint(OpticalFlowNodesX[i]/d *((double)ZoomFactor)))+2,
		    ((int) rint(OpticalFlowNodesY[i]/d *((double)ZoomFactor)))+8,
		    cbuf, strlen(cbuf));
      }
      if (MarkerCircleRadius) {
	XDrawArc(UxDisplay, XtWindow(DrawingArea), gc_YellowBlack,
		 ((int) rint(OpticalFlowNodesX[i]/d * ((double) ZoomFactor)))
		 - MarkerCircleRadius,
		 ((int) rint(OpticalFlowNodesY[i]/d * ((double) ZoomFactor)))
		 - MarkerCircleRadius,
		 2*MarkerCircleRadius, 2*MarkerCircleRadius, 1, 360*64);
      }
      else {
	XDrawPoint(UxDisplay, XtWindow(DrawingArea), gc_YellowBlack,
		   ((int) rint(OpticalFlowNodesX[i]/d * ((double) ZoomFactor))),
		   ((int) rint(OpticalFlowNodesY[i]/d * ((double) ZoomFactor))));
      }
      
      /* right node */
      if (OpticalFlowNodesWriteIndicies) {
	sprintf(cbuf, "%d", OpticalFlowNodesI[i]);
	XDrawString(UxDisplay, XtWindow(DrawingArea), gc_YellowBlack,
		    ((int)rint(OpticalFlowNodesX[i+1]/d*((double)ZoomFactor)))+2,
		    ((int)rint(OpticalFlowNodesY[i+1]/d*((double)ZoomFactor)))+8,
		    cbuf, strlen(cbuf));
      }
      if (MarkerCircleRadius) {
	XDrawArc(UxDisplay, XtWindow(DrawingArea), gc_YellowBlack,
		 ((int) rint(OpticalFlowNodesX[i+1]/d * ((double) ZoomFactor)))
		 - MarkerCircleRadius,
		 ((int) rint(OpticalFlowNodesY[i+1]/d * ((double) ZoomFactor)))
		 - MarkerCircleRadius,
		 2*MarkerCircleRadius, 2*MarkerCircleRadius,
		 1, 360*64);
      }
      else {
	XDrawPoint(UxDisplay, XtWindow(DrawingArea), gc_YellowBlack,
		   ((int) rint(OpticalFlowNodesX[i+1]/d *((double)ZoomFactor))),
		   ((int) rint(OpticalFlowNodesY[i+1]/d *((double)ZoomFactor))));
      }
    }
  }

  /* check memory */
  XvgMM_MemoryCheck("OpticalFlowNodesOverlay");
}

/*********************     OpticalFlowLoadNodes    *********************
*
*   function : OpticalFlowLoadNodes
*
***********************************************************************/
void OpticalFlowLoadNodes(double *inpixels, double *outpixels,
			  int height, int width, void *ParmPtrs[])
{
  FILE *fpn;
  int k;

  /* load input parameters */
  OpticalFlowNodesWriteIndicies = *((int *) ParmPtrs[1]);

  /* open the starting node coordinate file */
  if ((fpn = fopen(ParmPtrs[0], "r")) == NULL) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "OpticalFlowLoadNodes() : Could not open file \"%s\"",
	    ParmPtrs[0]);
    draw_mesg(cbuf);
    return;
  }

  /* count the number of lines (half the number of nodes) in the file */
  for (NOpticalFlowNodes = 0; fgets(cbuf, CBUF_SIZE, fpn) != NULL;
       NOpticalFlowNodes++);
  if (VerboseFlag)
    printf("OpticalFlowLoadNodes() : %d nodes to load...\n",
	   NOpticalFlowNodes/2);

  /* allocate storage for the nodes */
  OpticalFlowNodesX = (double *) XvgMM_Alloc((void *) OpticalFlowNodesX,
					     NOpticalFlowNodes*sizeof(double));
  OpticalFlowNodesY = (double *) XvgMM_Alloc((void *) OpticalFlowNodesY,
					     NOpticalFlowNodes*sizeof(double));
  OpticalFlowNodesI = (int *) XvgMM_Alloc((void *) OpticalFlowNodesI,
					     NOpticalFlowNodes*sizeof(int));
  if ((OpticalFlowNodesX == NULL) || (OpticalFlowNodesY == NULL) || (OpticalFlowNodesI == NULL)) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "OpticalFlowLoadNodes() : Could not allocate for %d nodes",
	    NOpticalFlowNodes);
    draw_mesg(cbuf);
    fclose(fpn);
    return;
  }

  /* read in the starting node coordinates */
  for (k = 0, fseek(fpn, 0, 0); k < NOpticalFlowNodes; k+=2) {
    fgets(cbuf, CBUF_SIZE, fpn);
/*
    if (VerboseFlag)
      printf("OpticalFlowLoadNodes() : line \"%s\"\n", cbuf);
*/
    if (sscanf(cbuf, "Node: %d", &(OpticalFlowNodesI[k])) != 1) {
      Abort = B_TRUE;
      sprintf(cbuf,
	      "OpticalFlowLoadNodes() : Error parsing index in file \"%s\" at line %d",
	      ParmPtrs[0], k);
      draw_mesg(cbuf);
      fclose(fpn);
      return;
    }
    if (fscanf(fpn, "%le %le %le %le\n",
	       &(OpticalFlowNodesX[k]), &(OpticalFlowNodesY[k]),
	       &(OpticalFlowNodesX[k+1]), &(OpticalFlowNodesY[k+1]))
	!= 4) {
      Abort = B_TRUE;
      sprintf(cbuf,
	      "OpticalFlowLoadNodes() : Error parsing file \"%s\" at line %d, node %d.",
	      ParmPtrs[0], k, OpticalFlowNodesI[k]);
      draw_mesg(cbuf);
      fclose(fpn);
      return;
    }

    /* correct for X axis, if required */
    if (SGIImageHalfWidth) {
      OpticalFlowNodesX[k]   /= 2.0;
      OpticalFlowNodesX[k+1] /= 2.0;
    }

    /* correct for Y axis differences between GL and X, and field display only */
    OpticalFlowNodesY[k]   = (((double) 2*height) - OpticalFlowNodesY[k]   - 1.0)/2.0;
    OpticalFlowNodesY[k+1] = (((double) 2*height) - OpticalFlowNodesY[k+1] - 1.0)/2.0;
  }
  fclose(fpn);

  /* load the funtion the list of overlays */
  LoadOverlayFunction(OpticalFlowNodesOverlay);

  /* transfer data to the output buffer */
  MoveDoubles(inpixels, outpixels, height*width);

  /* check memory */
  XvgMM_MemoryCheck("OpticalFlowLoadNodes");
}


/******************     OpticalFlowNodeCMFilter    *********************
*
*   function : OpticalFlowNodeCMFilter()
*
*   purpose :
*              Find the coordinates of the center of mass of the area
*              (kernel) around each specified node and write out the results
*              as a "corrected" node file.
*
*   parameters :
*              *((char *)   ParmPtrs[0])  = Input node file
*              *((char *)   ParmPtrs[1])  = Output node file
*              *((char *)   ParmPtrs[2])  = parameter filename
*
***********************************************************************/
static boolean_t ExtractNodeParametersFromString(char *s, double *RadiusL,
						 double *RadiusR)
{
  if (sscanf(s, "%le %le\n", RadiusL, RadiusR) == 2)
    return(B_TRUE);
  else
    return(B_FALSE);
}

static boolean_t CenterOfMass(double *inpixels, int height, int width,
                              double *buffer, int *xs, int *ys,
		              double NodeX, double NodeY, double Radius, int flag,
		              double *CMX, double *CMY)
{
  double v, cmx, cmy, w, median, mediann, XMult, MAX, MIN;
  int vi, i, j, swap, x, y, NPoints, xc, yc;

  /* check for half width images */
  if (SGIImageHalfWidth)
    XMult = 2.0;
  else
    XMult = 1.0;

  /* check radius */
  if (Radius >= MAXCMRADIUS) {
    *CMX = NodeX;
    *CMY = NodeY;
    return(B_FALSE);
  }

  /* if radius is 0.0, no filtering required */
  if (Radius <= 1.0) {
    *CMX = NodeX;
    *CMY = NodeY;
    return(B_TRUE);
  }

  /* bubble sort the left node neighbourhood (yeah, I know...) */
  for (y = floor(NodeY - Radius/2.0), NPoints = 0; y <= ceil(NodeY + Radius/2.0); y++) {
    for (x = floor(NodeX - Radius/XMult); x <= ceil(NodeX + Radius/XMult); x++) {
      if ((x > 0) && (x < width) && (y > 0) && (y < height)) {
	/* load the next pixel in the array */
	buffer[NPoints] = inpixels[y*width + x];
	xs[NPoints]     = x;
	ys[NPoints]     = y;
	/* shift the new value down to keep values ordered from min to max */
	for (i = NPoints, swap = 1; (i > 0) && (swap != 0); i--) {
	  if (buffer[i] < buffer[i-1]) {
            /* swap values */
	    v = buffer[i-1];
	    buffer[i-1] = buffer[i];
	    buffer[i] = v;
            /* swap x indicies */
	    vi = xs[i-1];
	    xs[i-1] = xs[i];
	    xs[i] = vi;
            /* swap y indicies */
	    vi = ys[i-1];
	    ys[i-1] = ys[i];
	    ys[i] = vi;
	  }
	  else {
	    swap = 0;
	  }
	}
	/* increment insertion index */
	NPoints++;
      }
    }
  }

  /* check the sort */
  for (i = 0; i < (NPoints-1); i++) {
    if (buffer[i] > buffer[i+1]) {
      Abort = B_TRUE;
      sprintf(cbuf,
	      "CenterOfMass() : Error in the sorting routine (%.2f, %.2f)",
	      NodeX, NodeY);
      draw_mesg(cbuf);
      return(B_FALSE);
    }
  }

  /* if required, return the coordinates of the minimum only */
  if (flag == 1) {
    *CMX = xs[0];
    *CMY = ys[0];
  }
  /* else find the center of mass of the local area about the minimum */
  else if ((buffer[NPoints-1] - buffer[0]) > 0.001) {
    /* data extrema */
    MIN = buffer[0];
    MAX = buffer[NPoints-1];

    /* normalized median */
    median  = buffer[NPoints/2];
    mediann = (buffer[NPoints/2] - MIN)/(MAX - MIN);

    /* loop to normalize */
    for (i = 0; i < NPoints; i++)
      buffer[i] = (buffer[i] - MIN)/(MAX - MIN);
 
    /* loop to calculate the CM */
    for (i = 0, w = 0, cmx = 0, cmy = 0; i < NPoints; i++) {
      buffer[i] = (buffer[i] < (mediann/2.0) ? buffer[i] : 1.0);
      v = (1.0 - buffer[i])*(1.0 - buffer[i]);
      cmx += (((double) xs[i])*v); 
      cmy += (((double) ys[i])*v);
      w   += v;
    } 
  
    /* return results */
    *CMX = cmx / w;
    *CMY = cmy / w;
  }
  else {
    *CMX = NodeX;
    *CMY = NodeY;
  }
  return(B_TRUE);
}

void OpticalFlowNodeCMFilter(double *inpixels, double *outpixels,
			     int height, int width, void *ParmPtrs[])
{
  FILE *fpi, *fpo;
  double NodeXL, NodeYL, NodeXR, NodeYR, cmxL, cmyL, cmxR, cmyR, XMult,
    RadiusL, RadiusR, buffer[(MAXCMRADIUS*2 + 1)*(MAXCMRADIUS*2 + 1)];
  int index, xs[(MAXCMRADIUS*2 + 1)*(MAXCMRADIUS*2 + 1)],
    ys[(MAXCMRADIUS*2 + 1)*(MAXCMRADIUS*2 + 1)], k, flag;
  char buf0[256];

  /* check for half width images */
  if (SGIImageHalfWidth)
    XMult = 2.0;
  else
    XMult = 1.0;

  /* input parameters */
  flag = *((int *) ParmPtrs[2]);
  if (VerboseFlag)
    printf("OpticalFlowNodeCMFilter() : flag = %d\n", flag);

  /* open the input node coordinate file */
  if ((fpi = fopen(ParmPtrs[0], "r")) == NULL) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "OpticalFlowNodeCMFilter() : Could not open input file \"%s\"",
	    ParmPtrs[0]);
    draw_mesg(cbuf);
    return;
  }

  /* open the output node coordinate file */
  if ((fpo = fopen(ParmPtrs[1], "w")) == NULL) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "OpticalFlowNodeCMFilter() : Could not open output file \"%s\"",
	    ParmPtrs[1]);
    draw_mesg(cbuf);
    fclose(fpi);
    return;
  }

  /* loop for all nodes */
  for (k = 0; fgets(buf0, CBUF_SIZE, fpi); k++) {
    /* read in the CMF parameters */
    if (sscanf(buf0, "Node: %d CMF %le %le NULL\n", &index, &RadiusL, &RadiusR) != 3) {
      Abort = B_TRUE;
      sprintf(cbuf,
	      "OpticalFlowNodeCMFilter() : Error parsing CM data in file %s at line %d",
	      ParmPtrs[0], k);
      draw_mesg(cbuf);
      fclose(fpi);
      fclose(fpo);
      return;
    }
    /* check radius values */
    if ((RadiusL >= MAXCMRADIUS) || (RadiusR >= MAXCMRADIUS)) {
      Abort = B_TRUE;
      sprintf(cbuf,
	      "OpticalFlowNodeCMFilter() : CM radius max exceeded (%.2f or %.2f) in file %s at line %d",
	      RadiusL, RadiusR, ParmPtrs[0], k);
      draw_mesg(cbuf);
      fclose(fpi);
      fclose(fpo);
      return;
    }
    /* read in the left/right node coordinates to filter */
    if (fscanf(fpi, "%le %le %le %le\n", &NodeXL, &NodeYL, &NodeXR, &NodeYR)
	!= 4) {
      Abort = B_TRUE;
      sprintf(cbuf,
	      "OpticalFlowNodeCMFilter() : Error parsing file %s at line %d",
	      ParmPtrs[0], k);
      draw_mesg(cbuf);
      fclose(fpi);
      fclose(fpo);
      return;
    }

    /* correct X coordinates for half sized images if required */
    NodeXL /= XMult;
    NodeXR /= XMult;

    /* correct node coordinates from GL to X notation, and de-interlace */
    NodeYL = (((double) 2*height) - NodeYL - 1.0)/2.0;
    NodeYR = (((double) 2*height) - NodeYR - 1.0)/2.0;

    /* verbose */
    if (VerboseFlag)
      printf("OpticalFlowNodeCMFilter() => Node %d (Radii: %.2f, %.2f) : %.2f, %.2f, %.2f, %.2f\n",
             index, RadiusL, RadiusR, NodeXL, NodeYL, NodeXR, NodeYR);

    /* calculate/write the center of mass of the left node */
    if (CenterOfMass(outpixels, height, width, buffer, xs, ys,
                     NodeXL, NodeYL, RadiusL, flag,
		     &cmxL, &cmyL) == B_FALSE) {
      Abort = B_TRUE;
      fclose(fpi);
      fclose(fpo);
      return;
    }

    /* calculate/write the center of mass of the right node */
    if (CenterOfMass(outpixels, height, width, buffer, xs, ys,
                     NodeXR, NodeYR, RadiusR, flag,
		     &cmxR, &cmyR) == B_FALSE) {
      Abort = B_TRUE;
      fclose(fpi);
      fclose(fpo);
      return;
    }

    /* correct node coordinates from GL to X notation, and re-interlace */
    NodeXL = cmxL*XMult;
    NodeXR = cmxR*XMult;
    NodeYL = ((double) 2*height) - 2.0*cmyL - 1.0;
    NodeYR = ((double) 2*height) - 2.0*cmyR - 1.0;

    /* write the data to the output file */
    fprintf(fpo, "%s", buf0);
    fprintf(fpo, "%f %f %f %f\n", NodeXL, NodeYL, NodeXR, NodeYR);
  }

  /* clean up */
  fclose(fpi);
  fclose(fpo);

  /* transfer data to the output buffer */
  MoveDoubles(inpixels, outpixels, height*width);
}


/******************     OpticalFlowFitFieldNR() **************************
*
*   function : OpticalFlowFitFieldNR(), utility used by OpticalFlow()
*
***********************************************************************/
static int OpticalFlowFitField_NR(double *pixels, int height, int width,
				  int decimation, double nodex, double nodey,
				  double Radius, int order, double *a,
				  double *c, double **u, double **v,
				  double *w, double *b, double *sd,
				  double *FieldEstimate)
{
  double X, Y, sigma, SumZ2, SumZZp2, Z, Zp, wmax, thresh;
  int x, y, nn, i, j, kk, rank, nsamples, NPARMS;

  /* calculate the number of samples within the fitting neighborhood */
  for (y = rint(nodey - Radius), nsamples = 0;
       y <= rint(nodey + Radius); y++) {
    if ((y >= 0) && (y < height) && ((y & (decimation-1)) == 0)) {
      Y = ((double) y) - nodey;
      for (x = rint(nodex - Radius); x <= rint(nodex + Radius);
	   x++) {
	if ((x >= 0) && (x < width) && ((x & (decimation-1)) == 0)) {
	  X = ((double) x) - nodex;
	  if (sqrt(X*X + Y*Y) <= Radius) {
	    nsamples++;
	  }
	}
      }
    }
  }      

  /* calculate the number of model parameters */
  NPARMS = ((order + 1) * (order + 1)) - 1;

  /* check that there are more samples than parameters */
  if (nsamples < NPARMS) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "OpticalFlowFitField_NR(): %d samples (NPARMS=%d) at [%.2f,%.2f]",
	    nsamples, NPARMS, nodex, nodey);
    ErrorMsg(cbuf);
    return(nsamples);
  }

  /* load u & b arrays (normal equations) */
  for (y = rint(nodey - Radius), nn = 1; y <= rint(nodey + Radius); y++) {
    if ((y >= 0) && (y < height) && ((y & (decimation-1)) == 0)) {
      Y = ((double) y) - nodey;
      for (x = rint(nodex - Radius); x <= rint(nodex + Radius); x++) {
	if ((x >= 0) && (x < width) && ((x & (decimation-1)) == 0)) {
	  X = ((double) x) - nodex;
	  if (sqrt(X*X + Y*Y) <= Radius) {
	    for (i = 0, kk = 1; i <= order; i++) {
	      for (j = 0; j <= order; j++) {
		if ((i != order) || (j != order)) {
		  u[nn][kk++] = pow((double) x, (double) i)
		    * pow((double) y, (double) j);
		}
	      }
	    }
	    b[nn++] = 
	      pixels[(y*width)/(decimation*decimation) + (x/decimation)];
	  }
	}
      }
    }
  }      

  /* compute least squares solution using Numerical Recipes SVD  */
  svdcmp(u, nsamples, NPARMS, w, v, c);
  for (j=1, wmax=0.0; j <= NPARMS; j++) {
    if (w[j] > wmax) {
      wmax = w[j];
    }
    thresh = TOL*wmax;
    for (j = 1; j <= NPARMS; j++) {
      if (w[j] < thresh) {
	w[j] = 0.0;
      }
      svbksb(u, w, v, nsamples, NPARMS, b, a, c);
    }
  }

  /* compute the rms error for the parametric representation of the data */
  for (y = rint(nodey - Radius), SumZZp2=0.0;
       y <= rint(nodey + Radius); y++) {
    if ((y >= 0) && (y < height) && ((y & (decimation-1)) == 0)) {
      Y = ((double) y) - nodey;
      for (x = rint(nodex - Radius); x <= rint(nodex + Radius); x++) {
	if ((x >= 0) && (x < width) && ((x & (decimation-1)) == 0)) {
	  X = ((double) x) - nodex;
	  if (sqrt(X*X + Y*Y) <= Radius) {
	    /* fetch sample */
	    Z = pixels[(y*width)/(decimation*decimation) + (x/decimation)];
	    /* compute estimate (Zp) */
	    for (i=0, kk=1, Zp=0; i <= order; i++) {
	      for (j=0; j <= order; j++) {
		if ((i != order) || (j != order)) {
		  Zp += (a[kk++] * pow((double) x, (double) i)
			 * pow((double) y, (double) j));
		}
	      }
	    }
	    /* update sums for VAF computation */
	    SumZZp2 += ((Z - Zp) * (Z - Zp));
	  }
	}
      }
    }
  }
  *sd = sqrt(SumZZp2/((double) nsamples));
  
  /* evaluate polynomial at the node position */
  for (i=0, kk=1, Zp=0; i <= order; i++) {
    for (j=0; j <= order; j++) {
      if ((i != order) || (j != order)) {
	Zp += (a[kk++] * pow(nodex, (double) i) * pow(nodey, (double) j));
      }
    }
  }
  *FieldEstimate = Zp;

  /* return number of samples in the domain */
  return(nsamples);
}

static int OpticalFlowFitField(double *pixels, int height, int width,
			       int decimation, double nodex, double nodey,
			       double Radius, int order, double TOLERANCE,
			       int NPARMS, double *A, double *B,
			       double *WORK, int WORKLEN,
			       double *sd, double *FieldEstimate)
{
  double X, Y, sigma, SumZ2, SumZZp2, Z, Zp;
  int x, y, n, i, j, k, rank, nsamples, IFAIL;

  /* calculate number of samples within node neighborhood, average objfunc */
  for (y = rint(nodey - Radius), nsamples = 0;
       y <= rint(nodey + Radius); y++) {
    if ((y >= 0) && (y < height) && ((y & (decimation-1)) == 0)) {
      Y = ((double) y) - nodey;
      for (x = rint(nodex - Radius); x <= rint(nodex + Radius); x++) {
	if ((x >= 0) && (x < width) && ((x & (decimation-1)) == 0)) {
	  X = ((double) x) - nodex;
	  if (sqrt(X*X + Y*Y) <= Radius) {
	    nsamples++;
	  }
	}
      }
    }
  }      

  /* load A & B arrays */
  for (y = rint(nodey - Radius), n = 0; y <= rint(nodey + Radius); y++) {
    if ((y >= 0) && (y < height) && ((y & (decimation-1)) == 0)) {
      Y = ((double) y) - nodey;
      for (x = rint(nodex - Radius); x <= rint(nodex + Radius); x++) {
	if ((x >= 0) && (x < width) && ((x & (decimation-1)) == 0)) {
	  X = ((double) x) - nodex;
	  if (sqrt(X*X + Y*Y) <= Radius) {
	    for (i = 0, k = 0; i <= order; i++) {
	      for (j = 0; j <= order; j++) {
		if ((i != order) || (j != order)) {
		  A[(k++)*nsamples + n] = pow((double) x, (double) i)
		    * pow((double) y, (double) j);
		}
	      }
	    }
	    B[n++] =pixels[(y*width)/(decimation*decimation) + (x/decimation)];
	  }
	}
      }
    }
  }      

  /* check memory */
  sprintf(cbuf, "OpticalFlowFitField(height:%d, width%d, nodex=%f, nodey=%f, decimation=%d, radius=%f, nsamples=%d, NPARMS=%d)",
	height, width, nodex, nodey, decimation, Radius, nsamples, NPARMS);
  XvgMM_MemoryCheck(cbuf);

  /* compute NAG least squares solution if parms are OK */
  IFAIL = 1;
  f04jaf(&nsamples, &NPARMS, A, &nsamples, B, &TOLERANCE, &sigma, &rank,
	 WORK, &WORKLEN, &IFAIL);
  if (IFAIL != 0) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "Abort:IFAIL=%d on return from f04jaf(nodex=%f, nodey=%f, decimation=%d, radius=%f, nsamples=%d, NPARMS=%d) in OpticalFlowFitField()",
	    IFAIL, nodex, nodey, decimation, Radius, nsamples, NPARMS);
    draw_mesg(cbuf);
    return(-1);
  }
  
  /* compute the rms error for the parametric representation of the data */
  for (y = rint(nodey - Radius), SumZZp2=0.0;
       y <= rint(nodey + Radius); y++) {
    if ((y >= 0) && (y < height) && ((y & (decimation-1)) == 0)) {
      Y = ((double) y) - nodey;
      for (x = rint(nodex - Radius); x <= rint(nodex + Radius); x++) {
	if ((x >= 0) && (x < width) && ((x & (decimation-1)) == 0)) {
	  X = ((double) x) - nodex;
	  if (sqrt(X*X + Y*Y) <= Radius) {
	    /* fetch sample */
	    Z = pixels[(y*width)/(decimation*decimation) + (x/decimation)];
	    /* compute estimate (Zp) */
	    for (i=0, k=0, Zp=0; i <= order; i++) {
	      for (j=0; j <= order; j++) {
		if ((i != order) || (j != order)) {
		  Zp += (B[k++] * pow((double) x, (double) i)
			 * pow((double) y, (double) j));
		}
	      }
	    }
	    /* update sums for VAF computation */
	    SumZZp2 += ((Z - Zp) * (Z - Zp));
	  }
	}
      }
    }
  }
  *sd = sqrt(SumZZp2/((double) nsamples));
  
  /* evaluate polynomial at the node position */
  for (i=0, k=0, Zp=0; i <= order; i++) {
    for (j=0; j <= order; j++) {
      if ((i != order) || (j != order)) {
	Zp += (B[k++] * pow(nodex, (double) i) * pow(nodey, (double) j));
      }
    }
  }
  *FieldEstimate = Zp;

  /* return number of samples in the domain */
  return(nsamples);
}

/********************     OpticalFlow()       *************************
*
*   function   : OpticalFlow()
*
*   parameters :
*              *((char *)   ParmPtrs[0])  = File listing the ".rgb" files
*                                           to process.
*              *((char *)   ParmPtrs[1])  = Node parameter file used by
*                                           OpticalFlowCMFilter()
*              *((char *)   ParmPtrs[2])  = Parameter filename used
*                                           by PatchXCorrByAreaHierarchical()
*              *((double *) ParmPtrs[3])  = extra parameters
*                       parameters[0]  = Processors used along the X dir used
*                                        by PatchXCorrByAreaHierarchical()
*                       parameters[1]  = Processors used along the Y dir used
*                                        by PatchXCorrByAreaHierarchical()
*                       parameters[2]  = index of start image in ParmPtrs[0]
*                       parameters[3]  = index of stop  image in ParmPtrs[0]
*                       parameters[4]  = Radius of the area around each node
*                                        used to fit a polynomial to the 
*                                        displacement field to estimate the
*                                        actual displacement at that node.
*                                        Suggested value : 3 to 5.
*                       parameters[5]  = Radius of the around each node
*                                        which is tracked using
*                                        PatchXCorrByAreaHierarchical(). A 
*                                        value of 25 results in roughly full
*                                        field calculations.
*                       parameters[6]  = Order of the polynomial model used
*                                        to calculate the displacement at a
*                                        node. Suggested value : 2.
*                       parameters[7]  = Calculate disp fields using
*                                        PatchXCorrByAreaHierarchical()
*                                        0: Load fields from disk files, do
*                                        NOT use PatchXCorrByAreaHierarchical()
*                                        1: Calculate new fields.
*
***********************************************************************/
static int ExtractIndex(char *fname)
{
  int k, index;
  
  if (fname) {
    for (k = (strlen(fname) - 1);
	 (k >= 0) && (fname[k] != '.'); k--);
    fname[k] = '\0';
    for (k = (strlen(fname) - 1);
	 (k >= 0) && ((fname[k] == '0') ||
		      (fname[k] == '1') ||
		      (fname[k] == '2') ||
		      (fname[k] == '3') ||
		      (fname[k] == '4') ||
		      (fname[k] == '5') ||
		      (fname[k] == '6') ||
		      (fname[k] == '7') ||
		      (fname[k] == '8') ||
		      (fname[k] == '9')); k--);
    index = atoi(&(fname[k+1]));
    fname[k+1] = '\0';
    return(index);
  }
  else {
    return(-1);
  }
}

void OpticalFlow(double *inpixels, double *outpixels,
		 int height, int width, void *ParmPtrs[])
{
  DOUBLE_ARRAY *parameters;
  FILE *fp, *fpn, *fpo, *fpo2, *fpc, *fpp;
  boolean_t NodeCorrection, StartImgIndexFound, StopImgIndexFound, rc, btmp;
  void *parms_PatchXCorrByAreaHierarchical[4], *parms_LoadImageFile[4];
  char outfname[512], infname[512], fname[512], FirstFileName[512],
  SecondFileName[512], outfname2[512], CorrectionFile[512], inbuf[CBUF_SIZE],
  NodeFileName[512], buffer[512], FileNameStripped[512];
  double *NodesX, *NodesY, FitRadius, X, Y, *xfield, *yfield, dx, dy, *C,
  lmin, lmax, *A, *B, TOLERANCE, *WORK, SD_Dy, FieldEstimateDy,
  SD_Dx, FieldEstimateDx, v, MaskRadius, NodeXL, NodeYL, NodeXR, NodeYR,
  MaxRadius, RadiusL, RadiusR, *dbuffer, **U, **V, *W;
  int cpus_x, cpus_y, k, x, y, NNodes, NImages, ngridx, ngridy,
  lheight, lwidth, NPARMS, WORKLEN, FitOrder, MaxSamples, index, nsamples,
  decimation, XCorrEnable, StartImgIndex, StopImgIndex, FirstIndex,
  SecondIndex, siz, NKeyFrameImages, NNodesCorrected, NNodesFiltered;

  /* fetch the input parameters */
  parameters = (DOUBLE_ARRAY *) ParmPtrs[3];
  if (parameters->n != 8) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort : 8 extra parms in OpticalFlow()");
    draw_mesg(cbuf);
    return;
  }
  ngridx        = (parameters->data)[0];
  ngridy        = (parameters->data)[1];
  StartImgIndex = (parameters->data)[2];
  StopImgIndex  = (parameters->data)[3];
  FitRadius     = (parameters->data)[4];
  MaskRadius    = (parameters->data)[5];
  FitOrder      = (parameters->data)[6];
  XCorrEnable   = (parameters->data)[7];

  /* check input parameters */
  if (StopImgIndex <= StartImgIndex) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "OpticalFlow() : Invalid start/stop indicies (%d, %d)",
	    StartImgIndex, StopImgIndex);
    draw_mesg(cbuf);
    return;
  }
  
  /* init variables */
  xfield = NULL;
  yfield = NULL;
  btmp = VerboseFlag;

  /* open the node parameter file */
  draw_mesg("Parsing node parameter file...");
  if (strcmp(ParmPtrs[1], "none") == 0) {
    fpp = NULL;
    if (VerboseFlag)
      printf("OpticalFLow() : No node parameter file loaded...\n");
  }
  else {
    /* open the file */
    if ((fpp = fopen(ParmPtrs[1], "r")) == NULL) {
      Abort = B_TRUE;
      sprintf(cbuf,
	      "OpticalFlowNodeCMFilter() : Could not open the file \"%s\"",
	      ParmPtrs[1]);
      draw_mesg(cbuf);
      VerboseFlag = btmp;
      return;
    }
  
    /* loop to find the largest kernel radius in the parameter file */
    MaxRadius = 0.0;
    rc = B_TRUE;
    while (fgets(cbuf, CBUF_SIZE, fpp) && rc) {
      fgets(cbuf, CBUF_SIZE, fpp);
      rc = ExtractNodeParametersFromString(cbuf, &RadiusL, &RadiusR);
      if (RadiusL > MaxRadius)
	MaxRadius = RadiusL;
      if (RadiusR > MaxRadius)
	MaxRadius = RadiusR;
    }
    if (VerboseFlag)
      printf("OpticalFlow() : Maximum CM filter radius = %.2f\n", MaxRadius);
    
    /* allocate storage for the sorting buffer */
    siz = rint((2.0*(MaxRadius + 4.0))*(2.0*(MaxRadius + 4.0)));
    if ((dbuffer = (double *) XvgMM_Alloc(NULL, siz*sizeof(double))) == NULL) {
      Abort = B_TRUE;
      sprintf(cbuf,
	      "OpticalFlow() : Could not allocate %d bytes",
	      siz*sizeof(double));
      draw_mesg(cbuf);
      VerboseFlag = btmp;
      return;
    }
  }

  /* open the PatchXCorrByAreaHierarchical() parameter file */
  draw_mesg("Parsing PatchXCorrByAreaHierarchical() parameter file...");
  if ((fp = fopen(ParmPtrs[2], "r")) == NULL) {
    Abort = B_TRUE;
    sprintf(cbuf, "OpticalFlow() : Could not open file \"%s\"", ParmPtrs[2]);
    draw_mesg(cbuf);
    fclose(fpp);
    VerboseFlag = btmp;
    return;
  }
  
  /* go to the last line and fetch the decimation parameter */
  for (fgets(cbuf, CBUF_SIZE, fp); fgets(cbuf, CBUF_SIZE, fp);)
    sscanf(cbuf, "%d %d %d %d %d %d %le %le %d %d %d %d %d\n",
	   &k, &decimation, &k, &k, &k, &k, &v, &v, &k, &k, &k, &k, &k);
  if (VerboseFlag)
    printf("OpticalFlow() : Decimation = %d\n", decimation);
  
  /* close the parameter file */
  fclose(fp);
  
  /* set fitting parameters */
#ifdef _NRC
  NPARMS = ((FitOrder + 1) * (FitOrder + 1)) - 1;
  MaxSamples = rint(2.0*FitRadius + 1.0) * rint(2.0*FitRadius + 1.0);
  A    = (double *) XvgMM_Alloc(NULL, (NPARMS+1)*sizeof(double));
  B    = (double *) XvgMM_Alloc(NULL, MaxSamples*sizeof(double));
  C    = (double *) XvgMM_Alloc(NULL, (NPARMS+1)*sizeof(double));
  W    = (double *) XvgMM_Alloc(NULL, (NPARMS+1)*sizeof(double));
  if ((U = (double **) XvgMM_Alloc(NULL, MaxSamples*sizeof(double *)))
      == NULL) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "OpticalFlow() : Could not allocate U (%d doubles)", MaxSamples);
    draw_mesg(cbuf);
    fclose(fpp);
    VerboseFlag = btmp;
    return;
  }
  for (k = 0; k < MaxSamples; k++) {
    if ((U[k] = (double *) XvgMM_Alloc(NULL, (NPARMS+1)*sizeof(double)))
	== NULL) {
      Abort = B_TRUE;
      sprintf(cbuf,
	      "OpticalFlow() : Could not allocate for U[%d] (%d doubles)",
	      k, NPARMS+1);
      draw_mesg(cbuf);
      fclose(fpp);
      VerboseFlag = btmp;
      return;
    }
  }
  if ((V = (double **) XvgMM_Alloc(NULL, (NPARMS+1)*sizeof(double *)))
      == NULL) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "OpticalFlow() : Could not allocate for V (%d doubles)",
	    NPARMS + 1);
    draw_mesg(cbuf);
    fclose(fpp);
    VerboseFlag = btmp;
    return;
  }
  for (k = 0; k <= NPARMS; k++) {
    if ((V[k] = (double *) XvgMM_Alloc(NULL, (NPARMS+1)*sizeof(double)))
	== NULL) {
      Abort = B_TRUE;
      sprintf(cbuf,
	      "OpticalFlow() : Could not allocate for v[%d] (%d doubles)",
	      k, NPARMS+1);
      draw_mesg(cbuf);
      fclose(fpp);
      VerboseFlag = btmp;
      return;
    }
  }
  if ((A == NULL) || (B == NULL) || (C == NULL) || (W == NULL)) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "OpticalFlow() : Could not allocate for %d samples or %d parms",
	    MaxSamples, NPARMS);
    draw_mesg(cbuf);
    fclose(fpp);
    VerboseFlag = btmp;
    return;
  }
#else
  NPARMS = ((FitOrder + 1) * (FitOrder + 1)) - 1;
  TOLERANCE = 1e-9;
  WORKLEN = 4 * NPARMS;
  MaxSamples = rint(2.0*FitRadius + 1.0) * rint(2.0*FitRadius + 1.0);
  A    = (double *) XvgMM_Alloc(NULL, MaxSamples*NPARMS*sizeof(double));
  B    = (double *) XvgMM_Alloc(NULL, MaxSamples*sizeof(double));
  WORK = (double *) XvgMM_Alloc(NULL, WORKLEN*sizeof(double));
  if ((A == NULL) || (B == NULL) || (WORK == NULL)) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "OpticalFlow() : Could not allocate for %d samples or %d parms",
	    MaxSamples, NPARMS);
    draw_mesg(cbuf);
    fclose(fpp);
    VerboseFlag = btmp;
    return;
  }
#endif

  /* open the image listing file */
  draw_mesg("Reading image listing file...");
  if ((fp = fopen(ParmPtrs[0], "r")) == NULL) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "OpticalFlow() : Could not open file \"%s\"", ParmPtrs[0]);
    draw_mesg(cbuf);
    fclose(fpp);
    VerboseFlag = btmp;
    return;
  }

  /* find the start/stop indicies in the image listing file */
  for (StartImgIndexFound = B_FALSE, StopImgIndexFound = B_FALSE,
       NKeyFrameImages = 0;
       (fscanf(fp, "%s\n", fname) == 1) && (StopImgIndexFound == B_FALSE);) {
    /* parse the filename to extract the index */
    sprintf(buffer, "%s", fname);
    index = ExtractIndex(buffer);

    /* compare to start index */
    if (index == StartImgIndex) {
      StartImgIndexFound = B_TRUE;
      if (StopImgIndexFound) {
	Abort = B_TRUE;
	sprintf(cbuf,
		"OpticalFlow() : Misordered filenames in \"%s\"", ParmPtrs[0]);
	draw_mesg(cbuf);
	fclose(fpp);
	VerboseFlag = btmp;
	return;
      }
      if (VerboseFlag)
	printf("OpticalFlow() : First file to process is %s\n", fname);
    }

    /* compare to stop index */
    if (index == StopImgIndex) {
      StopImgIndexFound = B_TRUE;
      if (VerboseFlag)
	printf("OpticalFlow() : Last file to process is %s\n", fname);
    }

    /* increment the keyframe counter, if require d*/
    if (StartImgIndexFound && !StopImgIndexFound)
      NKeyFrameImages++;
  }

  /* check that the starting image index was found */
  if ((StartImgIndexFound == B_FALSE) || (StopImgIndexFound == B_FALSE)) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "OpticalFlow():Can't find either %d or %d indicies in \"%s\"",
	    StartImgIndex, StopImgIndex, ParmPtrs[0]);
    draw_mesg(cbuf);
    fclose(fpp);
    VerboseFlag = btmp;
    return;
  }

  /* show info */
  NImages = StopImgIndex - StartImgIndex + 1;
  if (VerboseFlag)
    printf("OpticalFlow() : %d images to process (start=%d, stop=%d)...\n",
	   NKeyFrameImages, StartImgIndex, StopImgIndex);
  
  /* scroll back to the starting image filename */
  fseek(fp, 0, 0);
  do {
    fscanf(fp, "%s\n", fname);
    sprintf(buffer, "%s", fname);
    index = ExtractIndex(buffer);
  } while (index != StartImgIndex);

  /* extract the mantisaa and stripped mantissa (no directory path) */
  sprintf(FirstFileName, "%s", fname);
  for (k = (strlen(FirstFileName) - 1);
       (k >= 0) && (FirstFileName[k] != '.'); k--);
  FirstFileName[k] = '\0';
  for (k = (strlen(FirstFileName) - 1);
       (k >= 0) && (FirstFileName[k] != '/'); k--);
  if (FirstFileName[k] == '/')
    sprintf(FileNameStripped, "%s", &(FirstFileName[k+1]));
  else
    sprintf(FileNameStripped, "%s", FirstFileName);

  /* open the starting node coordinate file */
  draw_mesg("Loading starting node coordinates...");
  sprintf(NodeFileName, "%s.2d", FileNameStripped);
  if ((fpn = fopen(NodeFileName, "r")) == NULL) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "OpticalFlow() : Could not open starting node file \"%s\"",
	    NodeFileName);
    draw_mesg(cbuf);
    fclose(fp);
    fclose(fpp);
    VerboseFlag = btmp;
    return;
  }

  /* count the number of lines (half the number of nodes) in the file */
  for (NNodes = 0; fgets(cbuf, CBUF_SIZE, fpn) != NULL; NNodes++);
  if (VerboseFlag)
    printf("OpticalFlow() : %d nodes to process...\n", NNodes/2);

  /* allocate storage for the nodes */
  NodesX = (double *) XvgMM_Alloc((void *) NULL,
				  NImages*NNodes*sizeof(double));
  NodesY = (double *) XvgMM_Alloc((void *) NULL,
				  NImages*NNodes*sizeof(double));
  if ((NodesX == NULL) || (NodesY == NULL)) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "OpticalFlow() : Could not allocate for %d nodes, %d images",
	    NNodes, NImages);
    draw_mesg(cbuf);
    fclose(fp);
    fclose(fpn);
    fclose(fpp);
    VerboseFlag = btmp;
    return;
  }

  /* initialize node storage */
  for (k = 0; k < NImages*NNodes; k++) {
    NodesX[k] = 0;
    NodesY[k] = 0;
  }

  /* read in the starting node coordinates */
  for (k = 0, fseek(fpn, 0, 0); k < NNodes; k+=2) {
    fgets(cbuf, CBUF_SIZE, fpn);
    if (fscanf(fpn, "%le %le %le %le\n",
	       &(NodesX[k]), &(NodesY[k]), &(NodesX[k + 1]), &(NodesY[k + 1]))
	!= 4) {
      Abort = B_TRUE;
      sprintf(cbuf,
	      "OpticalFlow() : Error parsing file \"%s\" at line %d",
	      fname, k);
      draw_mesg(cbuf);
      fclose(fpn);
      fclose(fp);
      fclose(fpp);
      VerboseFlag = btmp;
      return;
    }

    /* correct for Y axis differences between GL and X */
    NodesY[k]     = height - NodesY[k]     - 1;
    NodesY[k + 1] = height - NodesY[k + 1] - 1;
  }
  fclose(fpn);

  /* loop to process the images */
  parms_LoadImageFile[0] = fname;
  parms_PatchXCorrByAreaHierarchical[0] = SecondFileName;
  parms_PatchXCorrByAreaHierarchical[1] = &ngridx;
  parms_PatchXCorrByAreaHierarchical[2] = &ngridy;
  parms_PatchXCorrByAreaHierarchical[3] = ParmPtrs[2];
  do {
    /* fetch the index of the first file */
    sprintf(buffer, "%s", fname);
    FirstIndex = ExtractIndex(buffer);

    /* load the first image */
    VerboseFlag = B_FALSE;
    LoadImageFile(inpixels, inpixels, height, width, parms_LoadImageFile);
    if (Abort) {
      fclose(fp);
      fclose(fpp);
      VerboseFlag = btmp;
      return;
    }
    VerboseFlag = btmp;
    if (VerboseFlag)
      printf("OpticalFlow() : Loaded image file %s\n", fname);
    
    /* clear the stack */
    ImageStackTop = 0;
    
    /* fetch first filename mantissa */
    sprintf(FirstFileName, "%s", fname);
    for (k = (strlen(fname) - 1); (k >= 0) && (fname[k] != '.'); k--);
    FirstFileName[k] = '\0';
    
    /* fetch the next filename */ 
    if (fscanf(fp, "%s\n", fname) != 1) {
      sprintf(cbuf,"OpticalFlow() :  failed to read filename from %s",
	      ParmPtrs[0]);
      draw_mesg(cbuf);
      Abort = B_TRUE;
      fclose(fp);
      fclose(fpp);
      VerboseFlag = btmp;
      return;
    }
    
    /* fetch the index of the second file */
    sprintf(buffer, "%s", fname);
    SecondIndex = ExtractIndex(buffer);

    /* load the next image */
    VerboseFlag = B_FALSE;
    LoadImageFile(inpixels, inpixels, height, width, parms_LoadImageFile);
    if (Abort) {
      fclose(fp);
      fclose(fpp);
      VerboseFlag = btmp;
      return;
    }
    VerboseFlag = btmp;
    if (VerboseFlag)
      printf("OpticalFlow() : Loaded image file %s\n", fname);
    
    /* fetch second filename mantissa and stripped mantissa (no dir path) */
    sprintf(SecondFileName, "%s", fname);
    for (k = (strlen(fname) - 1); (k >= 0) && (fname[k] != '.'); k--);
    SecondFileName[k] = '\0';
    for (k = (strlen(SecondFileName) - 1);
	 (k >= 0) && (SecondFileName[k] != '/'); k--);
    if (SecondFileName[k] == '/')
      sprintf(FileNameStripped, "%s", &(SecondFileName[k+1]));
    else
      sprintf(FileNameStripped, "%s", SecondFileName);
    
    /* show progress */
    if (VerboseFlag)
      printf("OpticalFlow() : Processing files %s.* (%d) and %s.* (%d)..\n",
	     FirstFileName, FirstIndex, SecondFileName, SecondIndex);
    
    /* calculate full-field displacements, if required */
    if (XCorrEnable == 1) {
      /* generate the mask by calculating neighborhoods around the nodes */
      draw_mesg("Calculating pixel mask...");
      DisplayContourComputed = B_TRUE;
      for (k = 0; k < height*width; k++)
	MaskPixels[k] = 0;
      for (k = 0; k < NNodes; k++) {
	index = (FirstIndex - StartImgIndex)*NNodes + k;
	for (y = rint(NodesY[index] - MaskRadius);
	     y <= rint(NodesY[index] + MaskRadius); y++) {
	  if ((y >= 0) && (y < height)) {
	    Y = ((double) y) - NodesY[index];
	    for (x = rint(NodesX[index] - MaskRadius);
		 x <= rint(NodesX[index] + MaskRadius); x++) {
	      if ((x >= 0) && (x < width)) {
		X = ((double) x) - NodesX[index];
		if (sqrt(X*X + Y*Y) <= MaskRadius) {
		  MaskPixels[y*width + x] = 1.0;
		}
	      }
	    }
	  }
	}      
      }

#ifdef NOTHIS      
      /* debug, write the mask */
      draw_mesg("Writing pixel mask file...");
      sprintf(outfname, "%s_mask.mat", FirstFileName);
      WriteMatlabUShortFile(height, width, "mask", MaskPixels, outfname);
#endif
    
      /* calculate the pixel shifts */
      PatchXCorrByAreaHierarchical(inpixels, outpixels, height, width,
				   parms_PatchXCorrByAreaHierarchical);
      if (Abort) {
	fclose(fp);
	fclose(fpp);
	VerboseFlag = btmp;
	return;
      }
    }

    /* read in the x field data */
    sprintf(infname, "%s_shiftsX_d%02d_FILTERED.mat",
	    SecondFileName, decimation);
    sprintf(cbuf, "OpticalFlow() : Reading field file %s...", infname);
    draw_mesg(cbuf);
    VerboseFlag = B_FALSE;
    if (GetMatlabFile(&lheight, &lwidth, &lmin, &lmax, &xfield, infname,
		      VerboseFlag) == B_FALSE) {
      VerboseFlag = btmp;
      Abort = B_TRUE;
      sprintf(cbuf, "Abort : Can't open %s in OpticalFlow()", infname);
      draw_mesg(cbuf);
      fclose(fp);
      fclose(fpp);
      return;
    }
    VerboseFlag = btmp;
    if (VerboseFlag)
      printf("OpticalFlow() : Loaded field data file %s\n", infname);
    
    /* read in the y field data */
    sprintf(infname, "%s_shiftsY_d%02d_FILTERED.mat",
	    SecondFileName, decimation);
    sprintf(cbuf, "OpticalFlow() : Reading field file %s...", infname);
    draw_mesg(cbuf);
    VerboseFlag = B_FALSE;
    if (GetMatlabFile(&lheight, &lwidth, &lmin, &lmax, &yfield, infname,
		      VerboseFlag) == B_FALSE) {
      VerboseFlag = btmp;
      Abort = B_TRUE;
      sprintf(cbuf, "Abort : Can't open %s in OpticalFlow()", infname);
      draw_mesg(cbuf);
      fclose(fp);
      fclose(fpp);
      return;
    }
    VerboseFlag = btmp;
    if (VerboseFlag)
      printf("OpticalFlow() : Loaded field data file %s\n", infname);

    /* open the node correction file for this image if there is one */
    sprintf(CorrectionFile, "%s.2d.correction", FileNameStripped);
    fpc = fopen(CorrectionFile, "r");
    if (fpc) {
      sprintf(cbuf, "Loaded correction file %s...", CorrectionFile);
      draw_mesg(cbuf);
      if (VerboseFlag)
	printf("OpticalFLow() : Loaded correction file %s...\n",
	       CorrectionFile);
    }
    
    /* open file to write new node positions */
    sprintf(NodeFileName, "%s.2d", FileNameStripped);
    if ((fpo = fopen(NodeFileName, "w")) == NULL) {
      Abort = B_TRUE;
      sprintf(cbuf,
	      "OpticalFlow() : Could not open file \"%s\"", NodeFileName);
      draw_mesg(cbuf);
      fclose(fp);
      fclose(fpp);
      VerboseFlag = btmp;
      return;
    }

    /* open file to write fit stats */
    sprintf(outfname2, "%s.stats", FileNameStripped);
    if ((fpo2 = fopen(outfname2, "w")) == NULL) {
      Abort = B_TRUE;
      sprintf(cbuf,
	      "OpticalFlow() : Could not open file \"%s\"", outfname2);
      draw_mesg(cbuf);
      fclose(fp);
      fclose(fpp);
      VerboseFlag = btmp;
      return;
    }

    /* loop to fit polynomial models to node neighborhoods, calc new pos */
    draw_mesg("Calculating updated node coordinates...");
    if (fpp) {
      fseek(fpp, 0, 0);
    }
    fprintf(fpo2,"******* %d Nodes *******\n\n", NNodes/2);
    for (k = 0, NNodesCorrected = 0, NNodesFiltered = 0;
	 (k < NNodes) && (Abort == B_FALSE);
	 k += 2) {
      /* read in the corrected node information if it exists */
      if (fpc) {
	/* load in comment line at this node */
	fgets(inbuf, CBUF_SIZE, fpc);

	/* load in node information, if it exists */
	if (fscanf(fpc, "%le %le %le %le\n",
		   &NodeXL, &NodeYL, &NodeXR, &NodeYR) == 4) {
	  /* flag this node if a correction is supplied */
	  if (inbuf[0] == 'C') {
	    NodeCorrection = B_TRUE;
	  }
	  /* no node correction supplied */
	  else {
	    NodeCorrection = B_FALSE;
	  }
	}
	else {
	  NodeCorrection = B_FALSE;
	}
      }
      /* no node correction supplied */
      else {
	NodeCorrection = B_FALSE;
      }

      /* read in CM filter radii from the parameter file, if required */
      if (fpp) {
	/* load in comment line at this node */
	fgets(cbuf, CBUF_SIZE, fpp);

	/* load in radius information, if it exists */
	if (fgets(cbuf, CBUF_SIZE, fpp)) {
	  if (ExtractNodeParametersFromString(cbuf, &RadiusL, &RadiusR)
	      == B_FALSE) {
	    Abort = B_TRUE;
	    sprintf(cbuf,
		    "OpticalFlow():Error reading CM radii info in %s, node %d",
		    ParmPtrs[1], k);
	    draw_mesg(cbuf);
	    fclose(fpo);
	    fclose(fpo2);
	    fclose(fpc);
	    fclose(fp);
	    fclose(fpp);
	    VerboseFlag = btmp;
	    return;
	  }
	}
	else {
	  RadiusL = 0.0;
	  RadiusR = 0.0;
	}
      }
      else {
	RadiusL = 0.0;
	RadiusR = 0.0;
      }
      
      /* calculate the left node displacements */
#ifdef _NRC
      nsamples =
	OpticalFlowFitField_NR(xfield, height, width, decimation,
			       NodesX[(FirstIndex - StartImgIndex)*NNodes + k],
			       NodesY[(FirstIndex - StartImgIndex)*NNodes + k],
			       FitRadius, FitOrder, A, C, U, V, W, B,
			       &SD_Dx, &FieldEstimateDx);
      nsamples =
	OpticalFlowFitField_NR(yfield, height, width, decimation,
			       NodesX[(FirstIndex - StartImgIndex)*NNodes + k],
			       NodesY[(FirstIndex - StartImgIndex)*NNodes + k],
			       FitRadius, FitOrder, A, C, U, V, W, B,
			       &SD_Dy, &FieldEstimateDy);
#else
      nsamples =
	    OpticalFlowFitField(xfield, height, width, decimation,
			    NodesX[(FirstIndex - StartImgIndex)*NNodes + k],
			    NodesY[(FirstIndex - StartImgIndex)*NNodes + k],
			    FitRadius, FitOrder,
			    TOLERANCE, NPARMS, A, B,
			    WORK, WORKLEN, &SD_Dx, &FieldEstimateDx);
      nsamples =
	OpticalFlowFitField(yfield, height, width, decimation,
			    NodesX[(FirstIndex - StartImgIndex)*NNodes + k],
			    NodesY[(FirstIndex - StartImgIndex)*NNodes + k],
			    FitRadius, FitOrder,
			    TOLERANCE, NPARMS, A, B,
			    WORK, WORKLEN, &SD_Dy, &FieldEstimateDy);
#endif

      /* save the updated left node coordinates in local storage */
      if (NodeCorrection) {
	NodesX[(SecondIndex - StartImgIndex)*NNodes + k] = NodeXL;
	NodesY[(SecondIndex - StartImgIndex)*NNodes + k] = height - NodeYL - 1;
	NNodesCorrected++;
      }
      else {
	/* center of mass filtering, if required */
	if (RadiusL > 0.0) {
#ifdef NOTHIS
	  CenterOfMass(inpixels, height, width, dbuffer,
		       NodesX[(FirstIndex - StartImgIndex)*NNodes + k]
		       + FieldEstimateDx,
		       NodesY[(FirstIndex - StartImgIndex)*NNodes + k]
		       + FieldEstimateDy,
		       RadiusL,
		       &(NodesX[(SecondIndex - StartImgIndex)*NNodes + k]),
		       &(NodesY[(SecondIndex - StartImgIndex)*NNodes + k]));
	  NNodesFiltered++;
#endif
	}
	/* simply update the node coordinates using the field values */
	else {
	  NodesX[(SecondIndex - StartImgIndex)*NNodes + k] =
	    NodesX[(FirstIndex - StartImgIndex)*NNodes + k]
	      + FieldEstimateDx;
	  NodesY[(SecondIndex - StartImgIndex)*NNodes + k] =
	    NodesY[(FirstIndex - StartImgIndex)*NNodes + k]
	      + FieldEstimateDy;
	}
      }

      /* write the left node coordinates to the output text file */
      fprintf(fpo, "Node: %03d\n", k/2);
      fprintf(fpo, "%8.3f %8.3f ",
	      NodesX[(SecondIndex - StartImgIndex)*NNodes + k],
	      height - NodesY[(SecondIndex - StartImgIndex)*NNodes + k] - 1);
      if (NodeCorrection)
	fprintf(fpo2,"C-Node: %03d\n", k/2);
      else
	fprintf(fpo2,"Node: %03d\n", k/2);
      fprintf(fpo2,"dxl=%07.3f (sd=%5.3f), dyl=%07.3f (sd=%5.3f), ",
	      FieldEstimateDx, SD_Dx, FieldEstimateDy, SD_Dy);
      
      /* calculate the right node displacements */
#ifdef _NRC
      nsamples =
	OpticalFlowFitField_NR(xfield, height, width, decimation,
			       NodesX[(FirstIndex -StartImgIndex)*NNodes +k+1],
			       NodesY[(FirstIndex -StartImgIndex)*NNodes +k+1],
			       FitRadius, FitOrder, A, C, U, V, W, B,
			       &SD_Dx, &FieldEstimateDx);
      nsamples =
	OpticalFlowFitField_NR(yfield, height, width, decimation,
			       NodesX[(FirstIndex -StartImgIndex)*NNodes +k+1],
			       NodesY[(FirstIndex -StartImgIndex)*NNodes +k+1],
			       FitRadius, FitOrder, A, C, U, V, W, B,
			       &SD_Dy, &FieldEstimateDy);
#else
      nsamples =
	OpticalFlowFitField(xfield, height, width, decimation,
			    NodesX[(FirstIndex - StartImgIndex)*NNodes + k +1],
			    NodesY[(FirstIndex - StartImgIndex)*NNodes + k +1],
			    FitRadius, FitOrder,
			    TOLERANCE, NPARMS, A, B,
			    WORK, WORKLEN, &SD_Dx, &FieldEstimateDx);
      nsamples =
	OpticalFlowFitField(yfield, height, width, decimation,
			    NodesX[(FirstIndex - StartImgIndex)*NNodes + k +1],
			    NodesY[(FirstIndex - StartImgIndex)*NNodes + k +1],
			    FitRadius, FitOrder,
			    TOLERANCE, NPARMS, A, B,
			    WORK, WORKLEN, &SD_Dy, &FieldEstimateDy);
#endif
      
      /* save the updated right node coordinates in local storage */
      if (NodeCorrection) {
	NodesX[(SecondIndex - StartImgIndex)*NNodes +k +1] = NodeXR;
	NodesY[(SecondIndex - StartImgIndex)*NNodes +k +1] = height -NodeYR -1;
      }
      else {
	/* center of mass filtering, if required */
	if (RadiusR > 0.0) {
#ifdef NOTHIS
	  CenterOfMass(inpixels, height, width, dbuffer,
		       NodesX[(FirstIndex - StartImgIndex)*NNodes + k +1]
		       + FieldEstimateDx,
		       NodesY[(FirstIndex - StartImgIndex)*NNodes + k +1]
		       + FieldEstimateDy,
		       RadiusR,
		       &(NodesX[(SecondIndex - StartImgIndex)*NNodes + k +1]),
		       &(NodesY[(SecondIndex - StartImgIndex)*NNodes + k +1]));
#endif
	}
	/* simply update the node coordinates using the field values */
	else {
	  NodesX[(SecondIndex - StartImgIndex)*NNodes + k + 1] =
	    NodesX[(FirstIndex - StartImgIndex)*NNodes + k + 1] +
	      FieldEstimateDx;
	  NodesY[(SecondIndex - StartImgIndex)*NNodes + k + 1] = 
	    NodesY[(FirstIndex - StartImgIndex)*NNodes + k + 1] +
	      FieldEstimateDy;
	}
      }

      /* write the right node coordinates to the output text file */
      fprintf(fpo,  "%8.3f %8.3f\n",
	      NodesX[(SecondIndex - StartImgIndex)*NNodes + k + 1],
	      height - NodesY[(SecondIndex - StartImgIndex)*NNodes + k +1] -1);
      fprintf(fpo2, "dxr=%07.3f (sd=%5.3f), dyr=%07.3f (sd=%5.3f)\n",
	      FieldEstimateDx, SD_Dx, FieldEstimateDy, SD_Dy);

      /* bound check on updated node coordinates */
      if ((NodesX[(SecondIndex - StartImgIndex)*NNodes + k] < 0.0) ||
	  (NodesX[(SecondIndex - StartImgIndex)*NNodes + k] >= width) ||
	  (NodesY[(SecondIndex - StartImgIndex)*NNodes + k] < 0.0) ||
	  (NodesY[(SecondIndex - StartImgIndex)*NNodes + k] >= height)) {
	Abort = B_TRUE;
	sprintf(cbuf,
		"OpticalFlow() : Node value out of bounds [%.2f,%.2f]",
		NodesX[(SecondIndex - StartImgIndex)*NNodes + k],
		NodesY[(SecondIndex - StartImgIndex)*NNodes + k]);
	draw_mesg(cbuf);
      }
      if ((NodesX[(SecondIndex - StartImgIndex)*NNodes + k + 1] < 0.0) ||
	  (NodesX[(SecondIndex - StartImgIndex)*NNodes + k + 1] >= width) ||
	  (NodesY[(SecondIndex - StartImgIndex)*NNodes + k + 1] < 0.0) ||
	  (NodesY[(SecondIndex - StartImgIndex)*NNodes + k + 1] >= height)) {
	Abort = B_TRUE;
	sprintf(cbuf,
		"OpticalFlow() : Node value out of bounds [%.2f,%.2f]",
		NodesX[(SecondIndex - StartImgIndex)*NNodes + k + 1],
		NodesY[(SecondIndex - StartImgIndex)*NNodes + k + 1]);
	draw_mesg(cbuf);
      }
    }

    /* close output files */
    fclose(fpo);
    fclose(fpc);
    fclose(fpo2);

    /* show information */
    if (VerboseFlag) {
      printf("OpticalFlow() : %d/%d nodes corrected, %d/%d nodes filtered..\n",
	     NNodesCorrected, NNodes/2, NNodesFiltered, NNodes/2);
      printf("OpticalFLow() : Wrote node file %s...\n", NodeFileName);
    }

    /* interpoate between FirstIndex and SecondIndex, if required */
    if ((SecondIndex - FirstIndex) != 1) {
      /* loop for interpolation files required */
      for (index = FirstIndex+1; (index < SecondIndex) && (Abort != B_TRUE);
	   index++) {
	/* open file to write new node positions */
	sprintf(buffer, "%s", FirstFileName);
	ExtractIndex(buffer);
	for (k = (strlen(buffer) - 1);
	     (k >= 0) && (buffer[k] != '/'); k--);
	if (buffer[k] == '/')
	  sprintf(FileNameStripped, "%s", &(buffer[k+1]));
	else
	  sprintf(FileNameStripped, "%s", buffer);
	sprintf(outfname, "%s%d.2d", FileNameStripped, index);
	if ((fpo = fopen(outfname, "w")) == NULL) {
	  Abort = B_TRUE;
	  sprintf(cbuf,
		  "OpticalFlow() : Could not open file \"%s\"", outfname);
	  draw_mesg(cbuf);
	  fclose(fp);
	  fclose(fpp);
	  VerboseFlag = btmp;
	  return;
	}

	/* show banner */
	sprintf(cbuf,
		"Calculating interpolated node coordinates for file %s...",
		outfname);
	draw_mesg(cbuf);

	/* loop to calculate/write the interpolated node values */
	for (k = 0; (k < NNodes) && (Abort != B_TRUE); k+=2) {
	  /* calculate interpolated values of the left node */
	  dx = (NodesX[(SecondIndex - StartImgIndex)*NNodes + k] -
		NodesX[(FirstIndex - StartImgIndex)*NNodes + k])
	    / ((double) (SecondIndex - FirstIndex));
	  dy = (NodesY[(SecondIndex - StartImgIndex)*NNodes + k] -
		NodesY[(FirstIndex - StartImgIndex)*NNodes + k])
	    / ((double) (SecondIndex - FirstIndex));
	  NodesX[(index - StartImgIndex)*NNodes + k] =
	    NodesX[(FirstIndex - StartImgIndex)*NNodes + k] +
	      (dx * ((double) (index - FirstIndex)));
	  NodesY[(index - StartImgIndex)*NNodes + k] =
	    NodesY[(FirstIndex - StartImgIndex)*NNodes + k] +
	      (dy * ((double) (index - FirstIndex)));
	  
	  /* write the left node coordinates to the output text file */
	  fprintf(fpo, "Node: %03d\n", k/2);
	  fprintf(fpo, "%8.3f %8.3f ",
		  NodesX[(index - StartImgIndex)*NNodes + k],
		  height - NodesY[(index - StartImgIndex)*NNodes + k] - 1);

	  /* calculate interpolated values of the right node */
	  dx = (NodesX[(SecondIndex - StartImgIndex)*NNodes + k + 1] -
		NodesX[(FirstIndex - StartImgIndex)*NNodes + k + 1])
	    / ((double) (SecondIndex - FirstIndex));
	  dy = (NodesY[(SecondIndex - StartImgIndex)*NNodes + k + 1] -
		NodesY[(FirstIndex - StartImgIndex)*NNodes + k + 1])
	    / ((double) (SecondIndex - FirstIndex));
	  NodesX[(index - StartImgIndex)*NNodes + k + 1] =
	    NodesX[(FirstIndex - StartImgIndex)*NNodes + k + 1] +
	      (dx * ((double) (index - FirstIndex)));
	  NodesY[(index - StartImgIndex)*NNodes + k + 1] =
	    NodesY[(FirstIndex - StartImgIndex)*NNodes + k + 1] +
	      (dy * ((double) (index - FirstIndex)));
	  
	  /* write the left node coordinates to the output text file */
	  fprintf(fpo, "%8.3f %8.3f\n",
		  NodesX[(index - StartImgIndex)*NNodes + k + 1],
		  height - NodesY[(index - StartImgIndex)*NNodes + k + 1] - 1);
	}

	/* close interpolated file */
	fclose(fpo);

	/* show information */
	if (VerboseFlag)
	  printf("OpticalFLow() : Wrote interpolated file %s...\n", outfname);
      }
    }
    
    /* write/update the "total" matlab output files */
    draw_mesg("Writing matlab output files...");
    VerboseFlag = B_FALSE;
    WriteMatlabFile(NImages, NNodes, "nodesx", NodesX, "nodesx.mat");
    WriteMatlabFile(NImages, NNodes, "nodesy", NodesY, "nodesy.mat");
    VerboseFlag = btmp;
    if (VerboseFlag)
      printf("OpticalFlow() : Wrote MATLAB node data files\n");

    /* check memory */
    sprintf(cbuf, "OpticalFlow(file #%d, Memory : %d KB)",
	    FirstIndex, XvgMM_GetMemSize()/1000);	   
    XvgMM_MemoryCheck(cbuf);
    
    /* show progress */
    if (VerboseFlag)
      printf("OpticalFlow() : Processed file #%d... (Memory : %d KB)\n",
	     FirstIndex, XvgMM_GetMemSize()/1000);	   
  } while ((SecondIndex != StopImgIndex) && (Abort == B_FALSE));

  /* verbose */
  if (VerboseFlag)
    printf("OpticalFlow() : Freeing memory...\n");

  /* close files */
  if (fpp) {
    fclose(fpp);
    XvgMM_Free(dbuffer);
  }
  fclose(fp);

  /* free storage */
  XvgMM_Free(xfield);
  XvgMM_Free(yfield);
#ifdef _NRC
  XvgMM_Free(A);
  XvgMM_Free(B);
  XvgMM_Free(C);
  XvgMM_Free(W);
  for (k = 0; k < MaxSamples; k++)
    XvgMM_Free(U[k]);
  for (k = 0; k <= NPARMS; k++)
    XvgMM_Free(V[k]);
  XvgMM_Free(U);
  XvgMM_Free(V);
#endif
  /* clean up */
  VerboseFlag = btmp;
  MoveDoubles(inpixels, outpixels, height*width);
}


/*************************** OBJFUN ************************************/
/*                                                                     */
/* User supplied function for the e04ucf NAG call which evaluates the  */
/* objective function and its gradient at a point.                     */
/*                                                                     */
/* Note: IUSER[0] width of the image                                   */
/*       IUSER[1] height of the image                                  */
/*       IUSER[2] fit type (see below)                                 */
/*        USER    points to the field data                             */
/*                                                                     */
/*         Plane equation => z = ux + vy + w                           */
/*                                                                     */
/*                                                                     */
/*         Since the phase is always 0 at DC, w = -Ux0 - Vy0           */
/*         where (x0, y0) are the coordinates of the DC component.     */
/*         x0 = 0, y0 = 0, hence w = 0.                                */
/*                                                                     */
/***********************************************************************/
static double ObjFunScale;
static void OBJFUN(int *MODE, int *N, double *X, double *OBJ, double *OBJGRD, 
		   int *NSTATE, int *IUSER, double *USER)
{
  double residual, estimate, ObjFun, w2, z;
  int i, j, k, cc;

  /* null gradient components if required */
  if ((*MODE == 1) || (*MODE == 2)) {
    OBJGRD[0] = 0;
    OBJGRD[1] = 0;
  }
  
  /* Compute the objective function and gradient at point X */
  for (j = 0, k = 0, ObjFun = 0, cc = IUSER[0]/2;
       (j < IUSER[1]) && (Abort == B_FALSE); j++) {
    for (i = 0; (i < IUSER[0]) && (Abort == B_FALSE); i++, k++) {
      /* sample and weight values */
      switch (IUSER[2]) {
      case 2:
	z = FFTImBuffer2[k];
	w2 = FFTRealBuffer2[k];
	break;
      case 1:
	z = FFTImBuffer2[k];
	w2 = 1.0;
	break;
      case 0:
	z = USER[k];
	w2 = 1.0;
      default:
	break;
      }
      
      /* compute the unwrapped model estimate at field location (x,y) */
      if (ShiftQuadrants)
	estimate = X[0]*((double) (i - cc)) + X[1]*((double) (j - cc));
      else
	estimate = X[0]*((double) i) + X[1]*((double) j);

      /* compute the wrapped weighted residual */
      residual = estimate - z;
      residual = residual - rint(residual/TWOPI)*TWOPI;
      
      /* add to residual squared to the running objective function sum */
      ObjFun += (residual * residual * w2);
      
      /* add to gradient components running sums */
      if ((*MODE == 1) || (*MODE == 2)) {
	if (ShiftQuadrants) {
	  OBJGRD[0] += (2.0 * residual * ((double) (i - cc)) * w2);
	  OBJGRD[1] += (2.0 * residual * ((double) (j - cc)) * w2);
	}
	else {
	  OBJGRD[0] += (2.0 * residual * ((double) i) * w2);
	  OBJGRD[1] += (2.0 * residual * ((double) j) * w2);
	}
      }
    }
  }
  
  /* store initial scaling factor if objective function is > 1.0 */
  /* fitting works much better if objective function is scaled   */
  /* to a reasonable value */
  if (*NSTATE == 1) {
    if (fabs(ObjFun) > 1.0) {
      ObjFunScale = 1.0 / ObjFun;
    }
    else {
      ObjFunScale = 1.0;
    }
    if (VerboseFlag) {
      printf("\nOBJFUN() => Initial objective function value = %.8e\n",
	     ObjFun);
      printf("\nOBJFUN() => U:%.6e, V:%.6e\n", X[0], X[1]);
      fflush(stdout);
    }
  }
  
  /* return the scaled objective function if required */
  if ((*MODE == 0) || (*MODE == 2)) {
    *OBJ = ObjFun * ObjFunScale;
  }
  
  /* return the scaled objective gradient components if required */
  if ((*MODE == 1) || (*MODE == 2)) {
    OBJGRD[0] *= ObjFunScale;
    OBJGRD[1] *= ObjFunScale;
  }
  
  /* check for user abort */
  if (Abort) {
    *MODE = -1;
  }
}


/**************************** GlobalPlaneFit ***************************
*
*   function : GlobalPlaneFit()
*
*   parameters:
*       *((double *) ParmPtrs[0]) = DX (fraction of the initial guess used to
*                                           constrain the solution, 0..1)
*       *((int *) ParmPtrs[1]) = 0: fit to the filtered phase
*                             1: fit to the unfiltered phase, uniform weights
*                             2: fit to the unfiltered phase, H(s) norm weights
*
***********************************************************************/
void GlobalPlaneFit(double *inpixels, double *outpixels, int height, int width,
		    void *ParmPtrs[])
{
  static double *BL=NULL, *BU=NULL, *X=NULL, *Y=NULL, *G=NULL, *W=NULL,
  *CLAMBDA=NULL, *WORK=NULL, *R=NULL;
  static int *ISTATE=NULL, *IWORK=NULL;
  double C[1], CJAC[1][1], g, F, DX;
  int IUSER[3], ZERO, ONE, LWORK, IW[2], LIWORK, i, j, k, ITER, IFAIL, NPARMS;

  /* load parameters */
  DX = *((double *) ParmPtrs[0]);
  IUSER[2] = *((int *) ParmPtrs[1]);

  /* allocate storage */
  NPARMS = 2;
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
    draw_mesg("Abort : can't allocate for NL buffers in GlobalPlaneFit()");
    return;
  }
  ZERO = 0;
  ONE = 1;

  /* load initial guess */
  X[0] = meanU;
  X[1] = meanV;

  /* set upper/lower bounds */
  for (i = 0; i < NPARMS; i++) {
    if (DX == 0) {
      BL[i] = -MAXDOUBLE;
      BU[i] = MAXDOUBLE;
    }
    else if (X[i] >= 0) {
      BL[i] = X[i]*(1.0 - DX);
      BU[i] = X[i]*(1.0 + DX);
    }
    else {
      BL[i] = X[i]*(1.0 + DX);
      BU[i] = X[i]*(1.0 - DX);
    }
  }
    
  /* solve the non-linear equation system for X */
  draw_mesg("GlobalPlaneFit() : Solving non-linear LS system for X");
  fflush(stdout);
  IUSER[0] = width;
  IUSER[1] = height;
  IFAIL = -1;
  e04ucf(&NPARMS, &ZERO, &ZERO, &ONE, &ONE, &NPARMS, NULL, BL, BU,
	 (void *) e04udm, (void *) OBJFUN, &ITER, ISTATE, C,
	 (double *) CJAC, CLAMBDA, &F, G, R, X,
	 IWORK, &LIWORK, WORK, &LWORK, IUSER, inpixels, &IFAIL);
  fflush(stdout);
  CurrentObjFun = F/ObjFunScale;
  
  /* show non-linear fitted results */
  printf("NL X shift: %.2f (%.2f) pix, Y shift: %.2f (%.2f) pix (objfun:%e)\n",
	 X[0]*((double) width)/TWOPI,
	 meanU*((double) width)/TWOPI,
	 X[1]*((double) height)/TWOPI,
	 meanV*((double) height)/TWOPI,
	 F/ObjFunScale);
  
  /* evaluate model */
  for (j = 0, k = 0; j < height; j++)
    for (i = 0; i < width; i++, k++)
      outpixels[k] = X[0]*((double) i) + X[1]*((double) j);
}



/****************************** MeanUV *************************************/
/*                                                                         */
/*  function: Mean values of U and V from a previous plane fit.            */
/*                                                                         */
/***************************************************************************/
void MeanUVW(double *inpixels, double *outpixels, int height, int width,
	    void *ParmPtrs[])
{
  double meanW, weights, w, sx, sy, *U, *V, *W, Threshold;
  int i;

  /* load binarization threshold */
  Threshold = *((double *) ParmPtrs[0]);

  /* check that stack is not empty, then decrement stack pointer  */
  if (ImageStackTop <= 2) {
    Abort = B_TRUE;
    draw_mesg("Abort : Three stack images required in WeightedMean()");
    return;
  }
  U = ImageStack[ImageStackTop-1].pixels;
  V = ImageStack[ImageStackTop-2].pixels;
  W = ImageStack[ImageStackTop-3].pixels;
  
  /* compute means */
  draw_mesg("MeanUV - Computing means...");
  for (i=0, meanU=0, meanV=0, meanW=0, weights=0; i < (height*width); i++) {
    /* binarize images according to weight */
    if (inpixels[i] > Threshold) {
      w = (inpixels[i] - Threshold)/(100.00 - Threshold);
      meanU  += U[i]*w;
      meanV  += V[i]*w;
      meanW  += W[i]*w;
      weights += w;
      U[i] = U[i]*((double) width)/TWOPI;
      V[i] = V[i]*((double) height)/TWOPI;
      outpixels[i] = inpixels[i];
    }
    else {
      U[i] = 0;
      V[i] = 0;
      outpixels[i] = Threshold;
    }
  }
  if (weights > 0) {
    meanU /= weights;
    meanV /= weights;
    meanW /= weights;
  }
  else {
    meanU = 0;
    meanV = 0;
    meanW = 0;
  }
  sx = meanU*((double) width)/TWOPI;
  sy = meanV*((double) height)/TWOPI;
  printf("MEANUVW() => X shift: %.2f pixels, Y shift: %.2f pixels\n", sx, sy);
}



/****************************** KernelScanBugout() ***************************
*
*   function : KernelScanBugout()
*
*	parameters:
*            FFTCacheRequests:   Array of cache requests                      
*            pixels:             Image from which FFTs are cached
*            mask:               Doughnut shaped mask of valid pixels
*            Xextent:            Width of the image
*            NRequests:          number of cache requests to scan
*            PatchCX:            reference image patch center X coordinate
*            PatchCY:            reference image patch center Y coordinate
*            kernw:              pixel dim of the square scanning kernels
*            offsetsX, offsetsY, dxs, dys, objs, VAFs:
*                                work buffers
*            dx, dy:             (OUTPUT) calculated incremental shifts
*            minShiftX, minShiftX:
*                                (OUTPUT) calculated total shifts
*            minobj:             (OUTPUT) minimum objective function
*            MaxVAF:             (OUTPUT) maximum VAF
*
*  Note: for some unknown reason, the pixel shifts are always underestimated
*        by a fixed scale factor. This error is estimated and corrected for
*        by looking at the neighbors with the lowest objective functions in the
*        X and Y directions.
*
***********************************************************************/
static boolean_t LSEstimateShiftFFT(double *pixels, int Xextent, FFT_CACHE_REQUEST *FFTCacheRequests, int n, int FFTCacheSize,
                                    int kernw, int WeightType, double *mask, int PatchCX, int PatchCY, 
                                    int index, double *offsetsX, double *offsetsY, double *MinObjVal, int *MinObjIndx, double *MaxObj,
                                    int *hits, boolean_t *OverflowFlag,
                                    double *DX, double *DY, double *ObjFunc, double *VAF, char *msg)
{
  static double *Re=NULL, *Im=NULL;
  boolean_t tmp, BoolTmp;
  double SumX2, SumY2, SumXY, SumXZ, SumYZ, SumZZp2, ZZp, U, V,
  scale, denom, SumZ2, SumW2, X, Y, Phase, CoherenceSquared;
  int x, y, k, rc;

  /* FFT cache lookup, if required  */
  tmp = VerboseFlag;
  VerboseFlag = B_FALSE;
  rc = FFTCacheLookup(FFTCacheRequests[n], pixels, Xextent, kernw, &Re, &Im, FFTCacheSize);
  VerboseFlag = tmp;
  if (rc < 0) {
    Abort = B_TRUE;
    return(B_FALSE);
  }
  *hits += rc;
    
  /* calculate the raw phase values from the cross spectrum function */
  SumX2 = 0; SumY2 = 0; SumXY = 0; SumXZ = 0; SumYZ = 0;
  for (y = 0, k = 0; y < kernw; y++) {
    /* plane model Y coordinate of this pixel */
    Y = y - kernw/2;
    for (x = 0; x < kernw; x++, k++) {
      /* plane model X coordinate of this pixel */
      X = x - kernw/2;
      /* check if sample is within the fitting radius, if so calculate */
      if (mask[k] != 0.0) {
        /* cross spectrum terms */
        FFTRealBuffer2[k] = (FFTRealBuffer[k]*Re[k] + FFTImBuffer[k]*Im[k]);
        FFTImBuffer2[k]   = (FFTImBuffer[k]*Re[k] - FFTRealBuffer[k]*Im[k]);
        Phase             = atan2(FFTImBuffer2[k], FFTRealBuffer2[k]);
	  
	    /* calculate the magnitude of the coherence squared function as a weight */
        if (WeightType == 0)
	      CoherenceSquared = (FFTRealBuffer2[k]*FFTRealBuffer2[k] + FFTImBuffer2[k]*FFTImBuffer2[k])
	                          /((FFTRealBuffer[k]*FFTRealBuffer[k] + FFTImBuffer[k]*FFTImBuffer[k]) * (Re[k]*Re[k] + Im[k]*Im[k]));
        /* else use the cross-spectrum magnitude squared instead */
        else
	      CoherenceSquared = (FFTRealBuffer2[k]*FFTRealBuffer2[k] + FFTImBuffer2[k]*FFTImBuffer2[k]);

        /* store the phase estimate and weight */
        FFTImBuffer2[k]   = Phase;
        FFTRealBuffer2[k] = CoherenceSquared;

        /* update weighted least squares fit running sums */
        SumX2 += (X * X * CoherenceSquared);
        SumY2 += (Y * Y * CoherenceSquared);
        SumXY += (X * Y * CoherenceSquared);
        SumXZ += (X * Phase * CoherenceSquared);
        SumYZ += (Y * Phase * CoherenceSquared);
      }
      else {
        FFTImBuffer2[k]   = 0;
        FFTRealBuffer2[k] = 0.0;
      }
    }
  }
    
  /* calculate the plane model parameters */
  scale = ((double) kernw)/TWOPI;
  denom = SumX2*SumY2 - SumXY*SumXY;
  if (fabs(denom) > MINDOUBLE) {
    U = (SumY2*SumXZ - SumXY*SumYZ)/denom;
    V = (SumX2*SumYZ - SumXY*SumXZ)/denom;
    *DX = -U*scale;
    *DY = -V*scale;
    *OverflowFlag = B_FALSE;
  }
  else {
    *DX = -kernw/4;
    *DY = -kernw/4;
    *OverflowFlag = B_TRUE;
  }

  /* calculate the objective function (sum of squared residuals) */
  for (y = 0, k = 0, SumZ2 = 0, SumZZp2=0, SumW2=0; y < kernw; y++) {
    Y = y - kernw/2;
    for (x = 0; x < kernw; x++, k++) {
      X = x - kernw/2;
      /* check if sample is within the fitting radius, if so update sums */
      if (mask[k] != 0.0) {
        SumW2   += FFTRealBuffer2[k];
        ZZp      = (U*X + V*Y - FFTImBuffer2[k]);
        SumZZp2 += (ZZp * ZZp * FFTRealBuffer2[k]);
        SumZ2   += (FFTImBuffer2[k] * FFTImBuffer2[k]  * FFTRealBuffer2[k]);
      }
    }
  }
  *ObjFunc = SumZZp2;

  /* flag over/underflow in the calculations with VAF = -2 */
  if (!finite(SumZ2) || *OverflowFlag) {
    *VAF = -2;
  }
  /* for sum of squared samples and errors less than PRECISION, VAF = 100% */
  else if ((SumZ2 < PRECISION) && (SumZZp2 < PRECISION)) {
    *VAF = 100.0;
  }
  /* normal condition, compute %VAF. If %VAF < 0, flag this by %VAF = -1 */
  else {
    *VAF = 100.0*(1.0 - SumZZp2/SumZ2);
    *VAF = (*VAF > 0.0 ? *VAF : -1);
  }

  /* return detailed results, if required */
  if (index != -1) {
    /* update the smallest objective function */ 
    if (*ObjFunc < *MinObjVal) {
      *MinObjVal  = *ObjFunc;
      *MinObjIndx = index;
    }
   
    /* update for largest objective function */
    *MaxObj = (*ObjFunc > *MaxObj ? *ObjFunc : *MaxObj);

    /* offsets */
    offsetsX[index] = PatchCX - FFTCacheRequests[n].x;
    offsetsY[index] = PatchCY - FFTCacheRequests[n].y;
  }

  /* verbose */
  if (VerboseFlag)
    printf("LSEstimateShiftFFT() -> #%03d (OX:%3.f, OY:%3.f)->dx:%6.3f, dy:%6.3f, Obj:%8.2e, %%VAF:%4.1f%%, WT:%d - %s\n",
	  n, (offsetsX ? offsetsX[index] : 0), (offsetsY ? offsetsY[index] : 0),
          *DX, *DY, *ObjFunc, *VAF, WeightType, msg);

  /* return success */   
  FFTCacheRequests[n].Done = B_TRUE;
  return(B_TRUE);
}

static int KernelScanBugout(POINT_TRACK_REQUEST *PointToTrack, double BugoutT, double ObjFunT,
                            FFT_CACHE_REQUEST *FFTCacheRequests, int FFTCacheSize,
		                    double *pixels, double *mask, int Xextent, int NRequests,
                            int PatchCX, int PatchCY, int kernw, int MRX, int MRY, int *IndexMap, int IndexMapN,
		                    double *offsetsX, double *offsetsY, double *dxs, double *dys, double *objs, double *VAFs,
		                    double *minShiftX, double *minShiftY)
{
  void *ParmPtrs[1];
  boolean_t BoolTmp, OverflowFlag;
  double MinObjVal, MaxObj, dx, dy, ObjFuncVal;
  int Indx, n, k, x, y, index, MinObjIndx, hits, rc, left, right, top, bottom;
  boolean_t tmp, Bugout;
  char fname[256];

  /* set internal parameters */
  MinObjVal = MAXDOUBLE;
  MaxObj = 0.0;
  hits = 0;

  /* loop to identify the phase shift */
  for (n = 0, MinObjIndx = 0, Bugout = B_FALSE;
      (n < NRequests) && (Abort == B_FALSE) && (Bugout == B_FALSE);
       n++) {
    /* calculate the index of this track request in the local index map */
    x = FFTCacheRequests[n].x - (PatchCX + PointToTrack->GuessXOffset);
    y = FFTCacheRequests[n].y - (PatchCY + PointToTrack->GuessYOffset);
    index = (y + MRY)*(2*MRX + 1) + (x + MRX);
    
    /* if all 4-connected neighbours are in "the zone", look them up */
    if ((abs(x) < MRX) && (abs(y) < MRY)) {
      /* process this request if it is a new one */
      if (FFTCacheRequests[n].Done == B_FALSE)
        LSEstimateShiftFFT(pixels, Xextent, FFTCacheRequests, n, FFTCacheSize, kernw, PointToTrack->WeightType, mask,
                           PatchCX, PatchCY, index, offsetsX, offsetsY, &MinObjVal, &MinObjIndx, &MaxObj,
                           &hits, &OverflowFlag, &(dxs[index]), &(dys[index]), &(objs[index]), &(VAFs[index]), "PRIMARY");

      /* calculate the 4-connected neighbour indicies */
      left   =     (y + MRY)*(2*MRX + 1) + (x - 1 + MRX);
      right  =     (y + MRY)*(2*MRX + 1) + (x + 1 + MRX);
      top    = (y + 1 + MRY)*(2*MRX + 1) + (x + MRX);
      bottom = (y - 1 + MRY)*(2*MRX + 1) + (x + MRX);

      /* doublecheck that indicies are not out of range */
      if ((left < 0)          || (right < 0)          || (top < 0)          || (bottom < 0) || 
          (left >= IndexMapN) || (right >= IndexMapN) || (top >= IndexMapN) || (bottom >= IndexMapN)) {
        Abort = B_TRUE;
        ErrorMsg("Index out of range in KernelScanBugout()");
        return(-1);
      }

      /* if the 4-connected neighbours are in the queue, continue... */
      if ((IndexMap[left] != -1) && (IndexMap[right] != -1) && (IndexMap[top] != -1) && (IndexMap[bottom] != -1)) {
        /* initialize local sorting vars */
        ObjFuncVal = objs[index];
        Indx       = index;

        /* calculate the 4-connected neighbours if required */
        if (FFTCacheRequests[(IndexMap[left])].Done == B_FALSE)
          LSEstimateShiftFFT(pixels, Xextent, FFTCacheRequests, IndexMap[left], FFTCacheSize, kernw, PointToTrack->WeightType, mask,
                             PatchCX, PatchCY, left, offsetsX, offsetsY, &MinObjVal, &MinObjIndx, &MaxObj,
                             &hits, &OverflowFlag, &(dxs[left]), &(dys[left]), &(objs[left]), &(VAFs[left]), "LEFT");
        if (objs[left] < ObjFuncVal) {
          ObjFuncVal = objs[left];
          Indx       = left;
        }
        if (FFTCacheRequests[(IndexMap[right])].Done == B_FALSE)
          LSEstimateShiftFFT(pixels, Xextent, FFTCacheRequests, IndexMap[right], FFTCacheSize, kernw, PointToTrack->WeightType, mask,
                             PatchCX, PatchCY, right, offsetsX, offsetsY, &MinObjVal, &MinObjIndx, &MaxObj,
                             &hits, &OverflowFlag, &(dxs[right]), &(dys[right]), &(objs[right]), &(VAFs[right]), "RIGHT");
        if (objs[right] < ObjFuncVal) {
          ObjFuncVal = objs[right];
          Indx       = right;
        }
        if (FFTCacheRequests[(IndexMap[top])].Done == B_FALSE)
          LSEstimateShiftFFT(pixels, Xextent, FFTCacheRequests, IndexMap[top], FFTCacheSize, kernw, PointToTrack->WeightType, mask,
                             PatchCX, PatchCY, top, offsetsX, offsetsY, &MinObjVal, &MinObjIndx, &MaxObj,
                             &hits, &OverflowFlag, &(dxs[top]), &(dys[top]), &(objs[top]), &(VAFs[top]), "TOP");
        if (objs[top] < ObjFuncVal) {
          ObjFuncVal = objs[top];
          Indx       = top;
        }
        if (FFTCacheRequests[(IndexMap[bottom])].Done == B_FALSE)
          LSEstimateShiftFFT(pixels, Xextent, FFTCacheRequests, IndexMap[bottom], FFTCacheSize, kernw, PointToTrack->WeightType, mask,
                             PatchCX, PatchCY, bottom, offsetsX, offsetsY, &MinObjVal, &MinObjIndx, &MaxObj,
                             &hits, &OverflowFlag, &(dxs[bottom]), &(dys[bottom]), &(objs[bottom]), &(VAFs[bottom]), "BOTTOM");
        if (objs[bottom] < ObjFuncVal) {
          ObjFuncVal = objs[bottom];
          Indx       = bottom;
        }

        if (VerboseFlag)
          printf("KernelScanBugout(%d,%d) : Index:%d, DL=%.2f, DRL=%.2f, DT=%.2f, DB=%.2f -> DX:%.3f, DY:%.3f, MRX:%d, MRY:%d, Obj:%.2e\n",
                 x, y, n, 
                 dxs[left]   - dxs[index], dxs[right]  - dxs[index],
                 dys[top]    - dys[index], dys[bottom] - dys[index],
                 offsetsX[index] + dxs[index], offsetsY[index] + dys[index],
                 MRX, MRY, objs[index]);

        /* is this the one ? */
        if (((fabs(1.0 + dxs[left] - dxs[index]) < BugoutT) || (fabs(1.0 - dxs[right]  + dxs[index]) < BugoutT)) &&
            ((fabs(1.0 - dys[top]  + dys[index]) < BugoutT) || (fabs(1.0 + dys[bottom] - dys[index]) < BugoutT)) &&
			!((objs[index] >= ObjFunT) && (PointToTrack->WeightType == 1))) {
          /* return the index of the smallest objective function within the neighborhood */
          MinObjIndx = Indx;
          Bugout = B_TRUE;
        }
      }
    }
  }
   
  /* return results */
  if (OverflowFlag == B_FALSE) {
    dx                    = dxs[MinObjIndx];
    dy                    = dys[MinObjIndx];
    *minShiftX            = offsetsX[MinObjIndx] + dx;
    *minShiftY            = offsetsY[MinObjIndx] + dy;
    PointToTrack->ObjFunc = objs[MinObjIndx];
    PointToTrack->VAF     = VAFs[MinObjIndx];
  }
  else {
    PointToTrack->ObjFunc = -1;
    PointToTrack->VAF     = -1;
  }

  /* print out info if required */
  if ((VerboseFlag  || ImageUpdateFlag)
      && !(ChildProcess && (nprocessors > 1)))
    printf("KernelScanBugout (X:%3d, Y:%3d): Bugout:%s, dx:%6.3f, dy:%6.3f, SX:%6.3f, SY:%6.3f, MinObj:%7.1e, MaxObj:%7.1e, %%VAF:%4.1f%%, MinObjIndx = %2d\n",
	   PatchCX, PatchCY, (Bugout ? "TRUE" : "FALSE"), dx, dy, *minShiftX, *minShiftY,
	   PointToTrack->ObjFunc, MaxObj, PointToTrack->VAF, IndexMap[MinObjIndx]);
  
  /* check memory */
  sprintf(cbuf, "KernelScanBugout(PatchCX:%d, PatchCY:%d)", PatchCX, PatchCY);
  XvgMM_MemoryCheck(cbuf);

  /* return number of cache hits */
  return(Bugout ? 1 : 0);
}
/***************************************************************************
*
*   function : IncrementalTrackPoints()
*
***************************************************************************/
static boolean_t CalculatePatchFFT(int xi, int yi, int FTKernelWidth, int width,
                                   double *ImgPatch, double *inpixels)
{
  int IFAIL, x, y, k;

  /* fetch the kernel from reference image, centered at xi, yi */
  for (y = yi - FTKernelWidth/2, k = 0; y < (yi + FTKernelWidth/2); y++)
    for (x = xi - FTKernelWidth/2; x < (xi + FTKernelWidth/2); x++, k++)
      ImgPatch[k] = inpixels[y*width + x];
	      
  /* calculate the kernel's FT */
  FFTApplyWindow(ImgPatch, FFTRealBuffer, FFTImBuffer);

  /* calculate image patch FFT */
  IFAIL = 1;
  c06fuf(&FTKernelWidth, &FTKernelWidth, FFTRealBuffer, FFTImBuffer,
         "Initial", FFTTrigm, FFTTrign, FFTWork, &IFAIL);
  if (IFAIL != 0) {
    Abort = B_TRUE;
    sprintf(cbuf,
            "Abort:IFAIL=%d, c06fuf() in CalculatePatchFFT()", IFAIL);
    draw_mesg(cbuf);
    return(B_FALSE);
  }
  FFTShift(FFTRealBuffer, FFTRealBuffer, FTKernelWidth, FTKernelWidth);
  FFTShift(FFTImBuffer, FFTImBuffer, FTKernelWidth, FTKernelWidth);

  /* return success */
  return(B_TRUE);	      
}


static void IncrementalTrackPoints(double *inpixels, double *outpixels, double *SmallMask,
                                   int height, int width, double BugoutT, double ObjFunT, 
                                   POINT_TRACK_REQUEST *PointsToTrack,
                                   int NPointsToTrack, int FTKernelWidth,
                                   double MaxMatchSearchRadiusX, double MaxMatchSearchRadiusY,
                                   int FFTCacheSize,
                                   double *buf1, double *buf2, double *buf3,
                                   double *buf4, double *buf5, double *buf6,
                                   double *ImgPatch, int *IndexMap, int IndexMapN, 
                                   double *CacheHitRatio)
{
  boolean_t TmpFlag, VbTmp, OverflowFlag, go;
  double DX, DY, ptime, tstart, DxsReturn, DysReturn, radius, ETA1, ETA2, dummy0, dummy1,
    FieldEstimateDx00, FieldEstimateDx10, FieldEstimateDx01, FieldEstimateDx11,
    FieldEstimateDy00, FieldEstimateDy10, FieldEstimateDy01, FieldEstimateDy11;
  int Nrequests, zf, lFFTWindow, i, j, k, x, y, x0, y0, x1, y1, TrackRequests,
    rc, RadiusX, RadiusY, index, MRX, MRY; 
  
  /* init internal parameters */
  VbTmp = VerboseFlag;
  zf = ZoomFactor;
  lFFTWindow = FFTWindow;
  MRX = floor(MaxMatchSearchRadiusX);
  MRY = floor(MaxMatchSearchRadiusY);

  /* disable message drawing */
  TmpFlag = DrawMesgEnabled;

  /* init FFT buffers */
  if (FFTInit(FTKernelWidth, FTKernelWidth) == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort : FFTInit() failed in IncrementalTrackPoints()");
    return;
  }
  if (FFTInit2(FTKernelWidth, FTKernelWidth) == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort : FFTInit2() failed in IncrementalTrackPoints()");
    return;
  }

  /* set the FFT window type */
  FFTWindow = HANNINGWINDOW;

  /* set temporary image size parameters */
  ImgWidth = FTKernelWidth;
  ImgHeight = FTKernelWidth;

  /* debugging */
  if (ImageUpdateFlag && !(ChildProcess && (nprocessors > 1))) {
    ZoomFactor = (height*ZoomFactor)/ImgWidth;
    if (ZoomFactor > 8)
	ZoomFactor = 8;
    ResizeProcessedImageWidgets(B_FALSE);
    CreateProcImage();
  }

  /* loop through match requests */
  for (TrackRequests = 0, FFTCacheRequestsNum = 0, FFTCacheHitsNum = 0;
       (TrackRequests < NPointsToTrack) && (Abort == B_FALSE); TrackRequests++) {
	/* check if this point needs to be tracked */
	if (!PointsToTrack[TrackRequests].Done) {
	  /* Verbose */
	  if (VerboseFlag)
	    printf("**** IncrementalTrackPoints() : Sample #%d (Point %d-%s) ****\n",
		       TrackRequests, PointsToTrack[TrackRequests].index,
			   (PointsToTrack[TrackRequests].LorR == -1 ? "LEFT" : "RIGHT"));
			   
      /* show progress */
      tstart = XvgGetTime();
	  
/*
	  if (PointsToTrack[TrackRequests].index == 194)
	    VerboseFlag = B_TRUE;
	  else
	    VerboseFlag = VbTmp;
		*/

      /* initialize the index map */
      for (k = 0; k < IndexMapN; k++)
        IndexMap[k] = -1;

      /* temporary flags */
      DrawMesgEnabled = B_FALSE;

      /* calculate the enclosing box coordinates */
      x0 = floor(PointsToTrack[TrackRequests].x);
      x1 = ceil(PointsToTrack[TrackRequests].x);
      if (x0 == x1)
        x1++;
      y0 = floor(PointsToTrack[TrackRequests].y);
      y1 = ceil(PointsToTrack[TrackRequests].y);
      if (y0 == y1)
        y1++;

      /* calculate the FFT for the point at corner 00 of the bounding box */
      if (CalculatePatchFFT(x0, y0, FTKernelWidth, width, ImgPatch, inpixels) == B_FALSE) {
        RestoreImageSize(zf, width, height, TmpFlag, lFFTWindow, inpixels, outpixels);
        return;
      }
	      
      /* load list of FFT cache requests for scanning around the 00 corner pixel */
      RadiusX = 1;
      RadiusY = 1;
      Nrequests = 0;
      go = B_TRUE;
      do {
        /* check loop variable */
        if ((RadiusX == MRX) && (RadiusY == MRY))
          go = B_FALSE;
        /* loop for y */
        for (y = -RadiusY; y <= RadiusY; y++) {
          /* loop for x */
          for (x = -RadiusX; x <= RadiusX; x++) {
            /* calculate pixel index */
            index = (y + MRY)*(2*MRX + 1) + (x + MRX);
            /* if this pixel location is unmarked, calculate the distance from the center */
            if (IndexMap[index] == -1) {
              /* if the pixel is inside the ellipse, mark it! */
              if (((((double) (x*x))/((double) (RadiusX*RadiusX))) +
                 (((double) (y*y))/((double) (RadiusY*RadiusY)))) <= 1.0) {
                IndexMap[index] = Nrequests;
                FFTCacheRequests[Nrequests].x    = (x0 + PointsToTrack[TrackRequests].GuessXOffset) + x;
                FFTCacheRequests[Nrequests].y    = (y0 + PointsToTrack[TrackRequests].GuessYOffset) + y;
                FFTCacheRequests[Nrequests].Done = B_FALSE;
                Nrequests++;
              }
            }
          }
        }
        /* update incrementing radii */
        if (RadiusX < MRX)
          RadiusX++;
        if (RadiusY < MRY)
          RadiusY++;
      } while (go);                                 

      /* debug */
      if (IndexMap[IndexMapN-1] != -1) {
        Abort = B_TRUE;
        sprintf(cbuf, "Abort : IndexMap corrupted in IncrementalTrackPoints()");
        draw_mesg(cbuf);
        RestoreImageSize(zf, width, height, TmpFlag, lFFTWindow,
                        inpixels, outpixels);
        DrawMesgEnabled = TmpFlag;
        VerboseFlag = VbTmp;
        return;
      }

      /* find the pixel shift with the lowest objective function */
      if ((rc = KernelScanBugout(&(PointsToTrack[TrackRequests]), BugoutT, ObjFunT,  FFTCacheRequests, FFTCacheSize,
                                 outpixels, SmallMask, width, Nrequests, x0, y0, FTKernelWidth,
                                 MRX, MRX, IndexMap, IndexMapN, buf1, buf2, buf3, buf4, buf5, buf6, 
                                 &FieldEstimateDx00, &FieldEstimateDy00)) < 0) {
        Abort = B_TRUE;
        sprintf(cbuf, "Abort:from KernelScan() in IncrementalTrackPoints()");
        draw_mesg(cbuf);
        RestoreImageSize(zf, width, height, TmpFlag, lFFTWindow,
                         inpixels, outpixels);
        DrawMesgEnabled = TmpFlag;
        VerboseFlag = VbTmp;
        return;
      }
      FFTCacheRequestsNum += Nrequests;
      FFTCacheHitsNum += rc;
      if (VerboseFlag)
        printf("IncrementalTrackPoints() : x:%d, y:%d, SX:%.3f, SY:%.3f...\n",
                x0, y0, FieldEstimateDx00, FieldEstimateDy00);
  
      /* find the pixel shift for the point at location 01 of the bounding box */
      if (CalculatePatchFFT(x0, y1, FTKernelWidth, width, ImgPatch, inpixels) == B_FALSE) {
        RestoreImageSize(zf, width, height, TmpFlag, lFFTWindow, inpixels, outpixels);
        return;
      }
      FFTCacheRequests[0].x = x0 - rint(FieldEstimateDx00);
      FFTCacheRequests[0].y = y1 - rint(FieldEstimateDy00);
      if (LSEstimateShiftFFT(outpixels, width, FFTCacheRequests, 0, 0, FTKernelWidth,
                             PointsToTrack[TrackRequests].WeightType, SmallMask,
                             x0, y1, -1, NULL, NULL, NULL, NULL, NULL,
                             &rc, &OverflowFlag, &DX, &DY, &dummy0, &dummy1, "X0Y1") == B_FALSE) {
        Abort = B_TRUE;
        sprintf(cbuf, "Abort : from LSEstimateShiftFFT() in IncrementalTrackPoints()");
        draw_mesg(cbuf);
        RestoreImageSize(zf, width, height, TmpFlag, lFFTWindow,
                         inpixels, outpixels);
        DrawMesgEnabled = TmpFlag;
        VerboseFlag = VbTmp;
        return;
      }
      FieldEstimateDx01 = rint(FieldEstimateDx00) + DX;
      FieldEstimateDy01 = rint(FieldEstimateDy00) + DY;
      if (VerboseFlag)
        printf("IncrementalTrackPoints() : x:%d, y:%d, SX:%.3f, SY:%.3f...\n",
                x0, y1, FieldEstimateDx01, FieldEstimateDy01);
  
      /* find the pixel shift for the point at location 10 of the bounding box */
      if (CalculatePatchFFT(x1, y0, FTKernelWidth, width, ImgPatch, inpixels) == B_FALSE) {
        RestoreImageSize(zf, width, height, TmpFlag, lFFTWindow, inpixels, outpixels);
        DrawMesgEnabled = TmpFlag;
        VerboseFlag = VbTmp;
        return;
      }
      FFTCacheRequests[0].x = x1 - rint(FieldEstimateDx00);
      FFTCacheRequests[0].y = y0 - rint(FieldEstimateDy00);
      if (LSEstimateShiftFFT(outpixels, width, FFTCacheRequests, 0, 0, FTKernelWidth,
                             PointsToTrack[TrackRequests].WeightType, SmallMask,
                             x1, y0, -1, NULL, NULL, NULL, NULL, NULL,
                             &rc, &OverflowFlag, &DX, &DY, &dummy0, &dummy1, "X1Y0") == B_FALSE) {
        Abort = B_TRUE;
        sprintf(cbuf, "Abort : from LSEstimateShiftFFT() in IncrementalTrackPoints()");
        draw_mesg(cbuf);
        RestoreImageSize(zf, width, height, TmpFlag, lFFTWindow,
                         inpixels, outpixels);
        DrawMesgEnabled = TmpFlag;
        VerboseFlag = VbTmp;
        return;
      }
      FieldEstimateDx10 = rint(FieldEstimateDx00) + DX;
      FieldEstimateDy10 = rint(FieldEstimateDy00) + DY;
      if (VerboseFlag)
        printf("IncrementalTrackPoints() : x:%d, y:%d, SX:%.3f, SY:%.3f...\n",
                x1, y0, FieldEstimateDx10, FieldEstimateDy10);
  
      /* find the pixel shift for the point at location 11 of the bounding box */
      if (CalculatePatchFFT(x1, y1, FTKernelWidth, width, ImgPatch, inpixels) == B_FALSE) {
        RestoreImageSize(zf, width, height, TmpFlag, lFFTWindow, inpixels, outpixels);
        DrawMesgEnabled = TmpFlag;
        VerboseFlag = VbTmp;
        return;
      }
      FFTCacheRequests[0].x = x1 - rint(FieldEstimateDx00);
      FFTCacheRequests[0].y = y1 - rint(FieldEstimateDy00);
      if (LSEstimateShiftFFT(outpixels, width, FFTCacheRequests, 0, 0, FTKernelWidth,
                             PointsToTrack[TrackRequests].WeightType, SmallMask,
                             x1, y1, -1, NULL, NULL, NULL, NULL, NULL,
                             &rc, &OverflowFlag, &DX, &DY, &dummy0, &dummy1, "X1Y1") == B_FALSE) {
        Abort = B_TRUE;
        sprintf(cbuf, "Abort : from LSEstimateShiftFFT() in IncrementalTrackPoints()");
        draw_mesg(cbuf);
        RestoreImageSize(zf, width, height, TmpFlag, lFFTWindow,
                         inpixels, outpixels);
        DrawMesgEnabled = TmpFlag;
        VerboseFlag = VbTmp;
        return;
      }
      FieldEstimateDx11 = rint(FieldEstimateDx00) + DX;
      FieldEstimateDy11 = rint(FieldEstimateDy00) + DY;
      if (VerboseFlag)
        printf("IncrementalTrackPoints() : x:%d, y:%d, SX:%.3f, SY:%.3f...\n",
                x1, y1, FieldEstimateDx11, FieldEstimateDy11);
  
      /* calculate interpolated values */
      ETA1 = (PointsToTrack[TrackRequests].x - ((double) x0))/ ((double) (x1 - x0));
      ETA2 = (PointsToTrack[TrackRequests].y - ((double) y0))/ ((double) (y1 - y0));
      PointsToTrack[TrackRequests].FieldEstimateDx = (1.0 - ETA1)*(1.0 - ETA2)*FieldEstimateDx00 +
                                                     ETA1*(1.0 - ETA2)*FieldEstimateDx10 +
                                                     ETA1*ETA2*FieldEstimateDx11 +
                                                     (1.0 - ETA1)*ETA2*FieldEstimateDx01;
      PointsToTrack[TrackRequests].FieldEstimateDy = (1.0 - ETA1)*(1.0 - ETA2)*FieldEstimateDy00 +
                                                     ETA1*(1.0 - ETA2)*FieldEstimateDy10 +
                                                     ETA1*ETA2*FieldEstimateDy11 +
                                                     (1.0 - ETA1)*ETA2*FieldEstimateDy01;

      /* flag this point as done */
	  PointsToTrack[TrackRequests].Done = B_TRUE;
  
      /* fetch total processing time for this iteration */
      ptime = XvgGetTime() - tstart;
      if (VerboseFlag)
        printf("IncrementalTrackPoints() => Point %d at (X:%.3f,Y:%.3f), (Xi:%.2f, Yi:%.2f), SX=%.3f, SY=%.3f, GX:%d, GY:%d, Obj=%.2e, %%VAF=%.1f, t= %.3fs\n",
                PointsToTrack[TrackRequests].index,
                PointsToTrack[TrackRequests].x,
                PointsToTrack[TrackRequests].y,
                ETA1, ETA2,
                PointsToTrack[TrackRequests].FieldEstimateDx,
                PointsToTrack[TrackRequests].FieldEstimateDy,
				PointsToTrack[TrackRequests].GuessXOffset, 
				PointsToTrack[TrackRequests].GuessYOffset, 
                PointsToTrack[TrackRequests].ObjFunc,
                PointsToTrack[TrackRequests].VAF,
  			  ptime);
    }
  }
  
  /* check memory */
  XvgMM_MemoryCheck("IncrementalTrackPoints");
  
  /* information */
  if (VerboseFlag)
    printf("IncrementalTrackPoints() : FFTCacheRequestsNum = %d, FFTCacheHitsNum = %d...\n",
           FFTCacheRequestsNum, FFTCacheHitsNum);
 
  /* restore image size parameters, if required */
  if (ImageUpdateFlag && !(ChildProcess && (nprocessors > 1))) {
    RestoreImageSize(zf, width, height, TmpFlag, lFFTWindow,
		     inpixels, outpixels);
  }
  else {
    ImgWidth  = width;
    ImgHeight = height;
  }

  /* return parameters */
/*  *CacheHitRatio = ((double) FFTCacheHitsNum)/((double) FFTCacheRequestsNum); */
  *CacheHitRatio = ((double) FFTCacheHitsNum)/((double) TrackRequests);
  DrawMesgEnabled = TmpFlag;
  VerboseFlag = VbTmp;
}

static boolean_t LoadPointStereoCoordinates(char *fname, STEREO_POINT *Points,
                                            int height, boolean_t LoadCMFDataFlag)
{
  FILE *fpn;
  int k, valids, index;
  char buf0[CBUF_SIZE];
  double RadiusL, RadiusR, XMult;

  /* check for half width images */
  if (SGIImageHalfWidth)
    XMult = 2.0;
  else
    XMult = 1.0;

  /* open the node file */
  if ((fpn = fopen(fname, "r")) == NULL) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "LoadPointStereotCoordinates() : Could not open file \"%s\"",
	    fname);
    draw_mesg(cbuf);
    return(B_FALSE);
  }

  /* read in the starting point coordinates */
  for (k = 0, valids = 0; fgets(buf0, CBUF_SIZE, fpn); k+=2) {
    /* read in the node index and CMF parameters */
    if (sscanf(buf0, "Node: %d CMF %le %le NULL\n", &index, &RadiusL, &RadiusR) != 3) {
      Abort = B_TRUE;
      sprintf(cbuf,
	      "LoadPointStereoCoordinates() : Error parsing index/CM data in file %s at line %d, index:%d",
	      fname, k, index);
      draw_mesg(cbuf);
      fclose(fpn);
      return(B_FALSE);
    }

    /* check the index range */
    if ((index < 0) || (index >= N_POINTS_MAX)) {
      Abort = B_TRUE;
      sprintf(cbuf,
	      "LoadPointStereoCoordinates() : Point index %d at line %d in %s is out of range",
              index, k, fname);
      draw_mesg(cbuf);
      fclose(fpn);
      return(B_FALSE);
    }

    /* load the node coordinates */
    if (fscanf(fpn, "%le %le %le %le\n",
	       &(Points[index].xl), &(Points[index].yl),
               &(Points[index].xr), &(Points[index].yr)) != 4) {
      Abort = B_TRUE;
      sprintf(cbuf,
	      "LoadPointStereoCoordinates() : Error parsing file \"%s\" at index %d, line %d",
	      fname, index, k);
      draw_mesg(cbuf);
      fclose(fpn);
      return(B_FALSE);
    }

    /* correct for Y axis differences between GL and X, and interlacing */
    Points[index].xl /= XMult;
    Points[index].xr /= XMult;
    Points[index].yl  = (((double) 2*height) - Points[index].yl - 1.0)/2.0;
    Points[index].yr  = (((double) 2*height) - Points[index].yr - 1.0)/2.0;

    /* overwrite the CMF data if required */
    if (LoadCMFDataFlag) {
      Points[index].CMRadiusXL = RadiusL;
      Points[index].CMRadiusYL = RadiusL;
      Points[index].CMRadiusXR = RadiusR;
      Points[index].CMRadiusYR = RadiusR;
    }

    /* flag this point as valid */
    Points[index].valid = 1;
    valids++;
  }

  /* close the file */
  fclose(fpn);
  if (VerboseFlag)
    printf("LoadPointStereoCoordinates() : %d valid points read from file %s...\n",
      valids, fname);

  /* check that the correct number of points were read */
  if (valids != N_POINTS_ACTUAL) {
      Abort = B_TRUE;
      sprintf(cbuf,
	      "LoadPointStereoCoordinates() : Invalid num of points (%d) in file \"%s\"",
	      valids, fname);
      draw_mesg(cbuf);
      fclose(fpn);
      return(B_FALSE);
  }

  /* return success */
  return(B_TRUE);
}

/********************     SequenceTrackPoints()       *************************
*
*   function   : SequenceTrackPoints()
*
*   parameters :
*              *((char *)   ParmPtrs[0])  = File listing the ".rgb" files
*                                           to process.
*              *((char *)   ParmPtrs[1])  = CM Filter parameters.
*              *((char *)   ParmPtrs[2])  = File listing the nodes to be tracked
*                                           (Keyword "all" for all node track)
*              *((double *) ParmPtrs[3])  = extra parameters
*                       parameters[0]     = index of start image in ParmPtrs[0]
*                       parameters[1]     = index of stop  image in ParmPtrs[0]
*                       parameters[2]     = Starting XCorr search radius.
*                       parameters[3]     = XCorr search radius increment.
*                       parameters[4]     = Maximum XCorr search radius.
*                       parameters[5]     = %VAF threshold for XCorr matching.
*                       parameters[6]     = Sampling increment (1 = full resolution).
*
*   IMPROVEMENTS: use a 64 FFT kernel on the final interpoaltion pass of the
*                 bounding box corners in IncrementalTrackPoints()
*
***********************************************************************/
static boolean_t ExtractCMFilterParametersFromString(char *s,
                                                     double *RadiusXL, double *RadiusYL,
						     double *RadiusXR, double *RadiusYR)
{
  if (sscanf(s, "%le %le %le %le\n", RadiusXL, RadiusYL, RadiusXR, RadiusYR) == 4)
    return(B_TRUE);
  else
    return(B_FALSE);
}

static boolean_t WriteOpticalFlowMatlabFiles(double *XLs, double *YLs, double *VAFLs, double *ObjLs,
                                             double *XRs, double *YRs, double *VAFRs, double *ObjRs,
                                             int N, char *fname)
{
  MATFile *DataFp;
  Matrix *a;
  
  /* open the matlab output file */
  if ((DataFp = matOpen(fname, "wb")) == NULL) {
    sprintf(cbuf, "WriteOpticalFlowMatlabFiles : could not open file %s\n",
	    fname);
    ErrorMsg(cbuf);
    return(B_FALSE);
  }

  /* create matrix */
  a = mxCreateFull(N, 1, REAL);

  /* copy left side data */
  memcpy(mxGetPr(a), XLs, N * sizeof(double));
  mxSetName(a, "XL");
  matPutMatrix(DataFp, a);
  memcpy(mxGetPr(a), YLs, N * sizeof(double));
  mxSetName(a, "YL");
  matPutMatrix(DataFp, a);
  if (VAFLs) {
    memcpy(mxGetPr(a), VAFLs, N * sizeof(double));
    mxSetName(a, "VAFL");
    matPutMatrix(DataFp, a);
  }
  if (ObjLs) {
    memcpy(mxGetPr(a), ObjLs, N * sizeof(double));
    mxSetName(a, "ObjL");
    matPutMatrix(DataFp, a);
  }

  /* copy right side data */
  memcpy(mxGetPr(a), XRs, N * sizeof(double));
  mxSetName(a, "XR");
  matPutMatrix(DataFp, a);
  memcpy(mxGetPr(a), YRs, N * sizeof(double));
  mxSetName(a, "YR");
  matPutMatrix(DataFp, a);
  if (VAFRs) {
    memcpy(mxGetPr(a), VAFRs, N * sizeof(double));
    mxSetName(a, "VARR");
    matPutMatrix(DataFp, a);
  }
  if (ObjRs) {
    memcpy(mxGetPr(a), ObjRs, N * sizeof(double));
    mxSetName(a, "ObjR");
    matPutMatrix(DataFp, a);
  }

  /* close file and return */
  mxFreeMatrix(a);
  matClose(DataFp);
  return(B_TRUE);
}

static void ResetPointToTrackItem(POINT_TRACK_REQUEST *PointsToTrack, int NPointsToTrack, double *WeightTypes)
{
  int index;

  /* fetch the point index */  
  index = PointsToTrack[NPointsToTrack].index;

  /* reset guess offsets */
  PointsToTrack[NPointsToTrack].GuessXOffset = 0;
  PointsToTrack[NPointsToTrack].GuessYOffset = 0;

  /* reset CM filter radius and weight type */
  if (PointsToTrack[NPointsToTrack].LorR == -1) {
    PointsToTrack[NPointsToTrack].WeightType = WeightTypes[3*index + 0];
    PointsToTrack[NPointsToTrack].CMRadiusX  = WeightTypes[3*index + 1];
    PointsToTrack[NPointsToTrack].CMRadiusY  = WeightTypes[3*index + 1];
  }
  else {
    PointsToTrack[NPointsToTrack].WeightType = WeightTypes[3*index + 0];
    PointsToTrack[NPointsToTrack].CMRadiusX  = WeightTypes[3*index + 2];
    PointsToTrack[NPointsToTrack].CMRadiusY  = WeightTypes[3*index + 2];
  }
  
  /* reset Done flag */
  PointsToTrack[NPointsToTrack].Done = B_FALSE;
}

static void LoadPointToTrackItem(POINT_TRACK_REQUEST *PointsToTrack, int NPointsToTrack,
                                 int index, double x, double y, double *WeightTypes, int LorR)
{
  PointsToTrack[NPointsToTrack].index = index;
  PointsToTrack[NPointsToTrack].x     = x;
  PointsToTrack[NPointsToTrack].y     = y;
  PointsToTrack[NPointsToTrack].LorR  = LorR;
  ResetPointToTrackItem(PointsToTrack, NPointsToTrack, WeightTypes);
}

void SequenceTrackPoints(double *inpixels, double *outpixels,
		         int height, int width, void *ParmPtrs[])
{
  XImage *pixel_image, *image;
  XColor cells[256];
  extern Widget ProcessedImageDrawingArea;
  POINT_TRACK_REQUEST *PointsToTrack, *PointsToTrackLost;
  STEREO_POINT *Points;
  DOUBLE_ARRAY *parameters;
  FILE *fp, *fpimgs, *fpo, *fpparms;
  void *parms_LoadImageFile[4], *OpticalFlowLoadNodesParms[4];
  boolean_t btmp, rc, StartImgIndexFound, StopImgIndexFound, ErrorRecoveryPass;
  double *CMFilterBuffer, *FFTMask, ObjFunT, WindowRadiusMin, WindowRadiusMax, WindowRadiusMaxBig,
    CacheHitRatio, CoregistrationThreshold, MaxMatchSearchRadiusX, MaxMatchSearchRadiusY, X, Y, X0,  Y0,
	*SmallMask, *BigMask, StereoThreshold, RadiusSqrd, *XLs, *XRs, *YLs, *YRs, *VAFLs, *VAFRs, *ObjLs, stime, etime, XMult,
    *ObjRs, *buf1, *buf2, *buf3, *buf4, *buf5, *buf6, *ImgPatch, CMX, CMY, *WeightTypes, stime0, 
    SumXL, SumYL, SumXR, SumYR, AvgXL, AvgYL, AvgXR, AvgYR, SDXL, SDYL, SDXR, SDYR, BugoutT;
  int StartImgIndex, StopImgIndex, siz, NKeyFrameImages, index, *IndexMap, NCoregistrationErrorsRemaining,
    NImages, FirstIndex, SecondIndex, NPointsToTrack, BufferSize, NFiles, NDisparityErrors, NDisparityErrorsRemaining, 
    *xs, *ys, FTKernelWidth, FFTCacheSize, PointsToTrackLostN, Zero, NCoregistrationErrors,
	NCoregistrationErrorRecoveryPasses, NTotalErrors, x, y, i, j, k;
  char fname[1024], FirstFileName[1024], FileNameStripped[512], LocalCbuf[CBUF_SIZE],
    NodeFileName[512], SecondFileName[1024], CorrectionFile[1024];

  /* initialize internal parameters */
  FFTCacheSize        = 0;
  btmp                = VerboseFlag;

  /* set image width increment */
  if (SGIImageHalfWidth)
    XMult = 2.0;
  else
    XMult = 1.0;

  /* fetch the extra input parameters */
  parameters = (DOUBLE_ARRAY *) ParmPtrs[3];
  if (parameters->n < 7) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort : at least 7 parms required in SequenceTrackPoints()");
    draw_mesg(cbuf);
    return;
  }
  StartImgIndex            = (parameters->data)[0];
  StopImgIndex             = (parameters->data)[1];
  MaxMatchSearchRadiusX    = (parameters->data)[2]; /* suggested value = 12     */
  BugoutT                  = (parameters->data)[3]; /* suggested value = 0.1    */
  ObjFunT                  = (parameters->data)[4]; /* suggested value = 0.0001 */
  StereoThreshold          = (parameters->data)[5]; /* suggested value = 5.0    */
  CoregistrationThreshold  = (parameters->data)[6]; /* suggested value = 1.0    */
  MaxMatchSearchRadiusY    = MaxMatchSearchRadiusX;

  /* set model fitting parameters */
  FTKernelWidth       = 32;
  WindowRadiusMin     = 3.0;
  WindowRadiusMax     = 9.0;
  WindowRadiusMaxBig  = 15.0;	  
  if (VerboseFlag)
    printf("SequenceTrackPoints() => MaxMatchSearchRadius:%.2f, StereoThreshold:%.1f...\n", 
	        MaxMatchSearchRadiusY, StereoThreshold);

  /* allocate fixed size memory */
  PointsToTrack      = (POINT_TRACK_REQUEST *) XvgMM_Alloc((void *) NULL, N_POINTS_MAX*2*sizeof(POINT_TRACK_REQUEST));
  PointsToTrackLost  = (POINT_TRACK_REQUEST *) XvgMM_Alloc((void *) NULL, N_POINTS_MAX*2*sizeof(POINT_TRACK_REQUEST));
  Points             = (STEREO_POINT *)        XvgMM_Alloc((void *) NULL, N_POINTS_MAX*sizeof(STEREO_POINT));
  CMFilterBuffer = (double *) XvgMM_Alloc((void *) NULL, (MAXCMRADIUS*2 + 1)*(MAXCMRADIUS*2 + 1)*sizeof(double));
  xs             =    (int *) XvgMM_Alloc((void *) NULL, (MAXCMRADIUS*2 + 1)*(MAXCMRADIUS*2 + 1)*sizeof(int));
  ys             =    (int *) XvgMM_Alloc((void *) NULL, (MAXCMRADIUS*2 + 1)*(MAXCMRADIUS*2 + 1)*sizeof(int));
  WeightTypes    = (double *) XvgMM_Alloc((void *) NULL, N_POINTS_MAX*3*sizeof(double));
  XLs            = (double *) XvgMM_Alloc((void *) NULL, N_POINTS_MAX*sizeof(double));
  XRs            = (double *) XvgMM_Alloc((void *) NULL, N_POINTS_MAX*sizeof(double));
  YLs            = (double *) XvgMM_Alloc((void *) NULL, N_POINTS_MAX*sizeof(double));
  YRs            = (double *) XvgMM_Alloc((void *) NULL, N_POINTS_MAX*sizeof(double));
  VAFLs          = (double *) XvgMM_Alloc((void *) NULL, N_POINTS_MAX*sizeof(double));
  VAFRs          = (double *) XvgMM_Alloc((void *) NULL, N_POINTS_MAX*sizeof(double));
  ObjLs          = (double *) XvgMM_Alloc((void *) NULL, N_POINTS_MAX*sizeof(double));
  ObjRs          = (double *) XvgMM_Alloc((void *) NULL, N_POINTS_MAX*sizeof(double));
  if ((XLs == NULL) || (XRs == NULL) || (YLs == NULL) || (YRs == NULL) || 
      (VAFLs == NULL) || (VAFRs == NULL) || (ObjLs == NULL) || (ObjRs == NULL) ||
	  (CMFilterBuffer == NULL) || (xs == NULL) || (ys == NULL) || (WeightTypes == NULL) ||
	  (PointsToTrack == NULL) || (Points == NULL) || (PointsToTrackLost == NULL)) {
		  
    Abort = B_TRUE;
    sprintf(cbuf, "Abort : Failed to allocate fixed sized buffers in SequenceTrackPoints()");
    draw_mesg(cbuf);
    return;
  }

  /* create in-memory pixmap, use malloc() because memory freed by X server */
  if (InteractiveFlag) {
    pixel_image    = XCreateImage(UxDisplay, XDefaultVisual(UxDisplay, UxScreen),
			                      8, ZPixmap, 0, ProcessedPixelsX, width, height, 8, 0);
    InMemoryPixmap = XCreatePixmap(UxDisplay, RootWindow(UxDisplay, UxScreen),
				                   width, height, 8);
  }
								 
  /* fill the normal FFT valid pixel mask, use X >= 0 quadrants only... */
  if ((SmallMask = (double *) XvgMM_Alloc((void *) NULL,
				           FTKernelWidth*FTKernelWidth*sizeof(double))) == NULL) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort : Could not allocate for SmallMask in SequenceTrackPoints()");
    draw_mesg(cbuf);
    return;
  }
  for (y = 0, k = 0; y < FTKernelWidth; y++) {
    Y = y - FTKernelWidth/2;
    for (x = 0; x < FTKernelWidth; x++, k++) {
      X = x - FTKernelWidth/2;
      RadiusSqrd = X*X + Y*Y;
      if ((RadiusSqrd < (WindowRadiusMax*WindowRadiusMax))
       && (RadiusSqrd > (WindowRadiusMin*WindowRadiusMin)) && (X >= 0)) {
        SmallMask[k] = 1.0;
      }
      else {
        SmallMask[k] = 0.0;
      }
    }
  }
    
  /* fill the double sized FFT valid pixel mask, use X >= 0 quadrants only... */
  if ((BigMask = (double *) XvgMM_Alloc((void *) NULL,
				         4*FTKernelWidth*FTKernelWidth*sizeof(double))) == NULL) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort : Could not allocate for BigMask in SequenceTrackPoints()");
    draw_mesg(cbuf);
    return;
  }
  for (y = 0, k = 0; y < (FTKernelWidth*2); y++) {
    Y = y - FTKernelWidth;
    for (x = 0; x < (FTKernelWidth*2); x++, k++) {
      X = x - FTKernelWidth;
      RadiusSqrd = X*X + Y*Y;
      if ((RadiusSqrd < (WindowRadiusMaxBig*WindowRadiusMaxBig))
       && (RadiusSqrd > (WindowRadiusMin*WindowRadiusMin)) && (X >= 0)) {
        BigMask[k] = 1.0;
      }
      else {
        BigMask[k] = 0.0;
      }
    }
  }
    
  /* check input parameters */
  if (StopImgIndex <= StartImgIndex) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "SequenceTrackPoints() : Invalid start/stop indicies (%d, %d)",
	    StartImgIndex, StopImgIndex);
    draw_mesg(cbuf);
    return;
  }

  /* initialize the stereo points array */
  for (i = 0; i < N_POINTS_MAX; i++) {
    Points[i].valid = 0;
    Points[i].xl = -1;
    Points[i].yl = -1;
    Points[i].xr = -1;
    Points[i].yr = -1;
    Points[i].CMRadiusXL = 0.0;
    Points[i].CMRadiusYL = 0.0;
    Points[i].CMRadiusXR = 0.0;
    Points[i].CMRadiusYR = 0.0;
  }

  /* open the image listing file */
  draw_mesg("Reading image listing file...");
  if ((fpimgs = fopen(ParmPtrs[0], "r")) == NULL) {
    Abort = B_TRUE;
    sprintf(cbuf,

	    "SequenceTrackPoints() : Could not open file \"%s\"", ParmPtrs[0]);
    draw_mesg(cbuf);
    return;
  }

  /* find the start/stop indicies in the image listing file */
  draw_mesg("Parsing the image listing file...");
  for (StartImgIndexFound = B_FALSE, StopImgIndexFound = B_FALSE,
       NKeyFrameImages = 0;
       (fscanf(fpimgs, "%s\n", fname) == 1) && (StopImgIndexFound == B_FALSE);) {
    /* parse the filename to extract the index */
    sprintf(LocalCbuf, "%s", fname);
    index = ExtractIndex(LocalCbuf);

    /* compare to start index */
    if (index == StartImgIndex) {
      StartImgIndexFound = B_TRUE;
      if (StopImgIndexFound) {
      Abort = B_TRUE;
      sprintf(cbuf, "SequenceTrackPoints() : Misordered filenames in \"%s\"", ParmPtrs[0]);
      draw_mesg(cbuf);
      fclose(fpimgs);
      return;
    }
    if (VerboseFlag)
	  printf("SequenceTrackPoints() : First file to process is %s\n", fname);
    }

    /* compare to stop index */
    if (index == StopImgIndex) {
      StopImgIndexFound = B_TRUE;
      if (VerboseFlag)
	printf("SequenceTrackPoints() : Last file to process is %s\n", fname);
    }

    /* increment the keyframe counter, if require d*/
    if (StartImgIndexFound && !StopImgIndexFound)
      NKeyFrameImages++;
  }

  /* check that the starting image index was found */
  if ((StartImgIndexFound == B_FALSE) || (StopImgIndexFound == B_FALSE)) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "SequenceTrackPoints():Can't find either %d or %d indicies in \"%s\"",
	    StartImgIndex, StopImgIndex, ParmPtrs[0]);
    draw_mesg(cbuf);
    fclose(fpimgs);
    return;
  }

  /* show info */
  NImages = StopImgIndex - StartImgIndex + 1;
  if (VerboseFlag)
    printf("SequenceTrackPoints() : %d images to process (start=%d, stop=%d)...\n",
	   NKeyFrameImages, StartImgIndex, StopImgIndex);
  
  /* scroll back to the starting image filename */
  fseek(fpimgs, 0, 0);
  do {
    fscanf(fpimgs, "%s\n", fname);
    sprintf(LocalCbuf, "%s", fname);
    index = ExtractIndex(LocalCbuf);
  } while (index != StartImgIndex);

  /* extract the mantissa for the first image file */
  sprintf(FirstFileName, "%s", fname);
  for (k = (strlen(FirstFileName) - 1);
       (k >= 0) && (FirstFileName[k] != '.'); k--);
  FirstFileName[k] = '\0';
  for (k = (strlen(FirstFileName) - 1);
       (k >= 0) && (FirstFileName[k] != '/'); k--);
  if (FirstFileName[k] == '/')
    sprintf(FileNameStripped, "%s", &(FirstFileName[k+1]));
  else
    sprintf(FileNameStripped, "%s", FirstFileName);

  /* load the stereo point coordinates from the starting point data file */
  draw_mesg("Loading starting stereo point coordinates...");
  sprintf(NodeFileName, "%s.2d", FileNameStripped);
  if (LoadPointStereoCoordinates(NodeFileName, Points, height, B_FALSE) == B_FALSE) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "SequenceTrackPoints() : Failure in initial LoadPointStereoCoordinates(%s)",
            NodeFileName);
    draw_mesg(cbuf);
    fclose(fpimgs);
    return;
  }

  /* write the starting point coordinates to the output file */
  for (k = 0; k < N_POINTS_MAX; k++) {
    XLs[k]   = Points[k].xl * XMult;
    YLs[k]   = ((double) 2*height) - (Points[k].yl * 2.0) - 1.0;
    XRs[k]   = Points[k].xr * XMult;
    YRs[k]   = ((double) 2*height) - (Points[k].yr * 2.0) - 1.0;
  }
  sprintf(LocalCbuf, "%s_data.mat", FileNameStripped);
  WriteOpticalFlowMatlabFiles(XLs, YLs, VAFLs, ObjLs, XRs, YRs, VAFRs, ObjRs, N_POINTS_MAX, LocalCbuf);

  /* Load in the parameter file (weights) */
  draw_mesg("Reading parameter file...");
  if ((fpparms = fopen(ParmPtrs[1], "r")) == NULL) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "SequenceTrackPoints() : Could not open the file \"%s\"", ParmPtrs[1]);
    draw_mesg(cbuf);
    return;
  }
  for (i = 0; (i < N_POINTS_MAX) && (Abort == B_FALSE); i++) {
    if (fgets(LocalCbuf, CBUF_SIZE, fpparms)) {
      if (sscanf(LocalCbuf, "%d %le %le %le",
	             &index, &(WeightTypes[3*i]), &(WeightTypes[3*i + 1]), &(WeightTypes[3*i + 2])) != 4) {
        Abort = B_TRUE;
        sprintf(cbuf,
	        "SequenceTrackPoints() : Error parsing at line %d in file \"%s\"", i, ParmPtrs[1]);
        draw_mesg(cbuf);
        return;
      }
    }
    /* premature EOF, return error */
    else {
      Abort = B_TRUE;
      sprintf(cbuf,
	      "SequenceTrackPoints() : Premature EOF (%d lines) in file \"%s\"", i, ParmPtrs[1]);
      draw_mesg(cbuf);
      return;
    }
  }
  fclose(fpparms);

  /* load the list of points to track */
  draw_mesg("Parsing the tracking node list file...");
  if ((strcmp(ParmPtrs[2], "all") == 0) || (strcmp(ParmPtrs[2], "ALL") == 0)) {
    /* initialize all point tracking */
    for (i = 0, NPointsToTrack = 0; i < N_POINTS_MAX; i++) {
      if (Points[i].valid == 1) {
        /* left view coordinates of the point */
        LoadPointToTrackItem(PointsToTrack, NPointsToTrack++, i, Points[i].xl, Points[i].yl, WeightTypes, -1);

        /* right view coordinates of the point */
        LoadPointToTrackItem(PointsToTrack, NPointsToTrack++, i, Points[i].xr, Points[i].yr, WeightTypes, +1);
      }
    }
    if (VerboseFlag)
      printf("SequenceTrackPoints() : All point tracking (%d points)...\n",
             NPointsToTrack);
  }
  else {
    /* open the tracking node list file */
    if ((fp = fopen(ParmPtrs[2], "r")) == NULL) {
      Abort = B_TRUE;
      sprintf(cbuf,
	      "SequenceTrackPoints() : Could not open the file \"%s\"", ParmPtrs[2]);
      draw_mesg(cbuf);
      return;
    }

    /* read in the indicies of the nodes to track, one per line */
    for (i = 0, NPointsToTrack = 0; fscanf(fp, "%d\n", &index) == 1; i++) {
      /* check index */
      if ((index < 0) || (index >= N_POINTS_MAX)) {
        Abort = B_TRUE;
        sprintf(cbuf,
	      "SequenceTrackPoints() : Point index %d in %s is out of range",
              index, ParmPtrs[2]);
        draw_mesg(cbuf);
        return;
      }

      /* check that point is defined */
      if (Points[index].valid != 1) {
        Abort = B_TRUE;
        sprintf(cbuf,
	      "SequenceTrackPoints() : Undefined point coordinates at index %d in %s",
              index, ParmPtrs[2]);
        draw_mesg(cbuf);
        return;
      }

      /* left view coordinates of the point */
      LoadPointToTrackItem(PointsToTrack, NPointsToTrack++, index, Points[index].xl, Points[index].yl, WeightTypes, -1);

      /* right view coordinates of the point */
      LoadPointToTrackItem(PointsToTrack, NPointsToTrack++, index, Points[index].xr, Points[index].yr, WeightTypes, +1);
    }

    /* close the file */
    fclose(fp);
  }
  if (VerboseFlag)
    printf("SequenceTrackPoints() : %d points to track...\n", NPointsToTrack);

  /* calculate required cache request buffer size */
  BufferSize = ceil((2.0*MaxMatchSearchRadiusX +3.0)*(2.0*MaxMatchSearchRadiusY +3.0));

  /* initialize the FFT Cache */
  FFTCacheRequests = (FFT_CACHE_REQUEST *) XvgMM_Alloc((void *) FFTCacheRequests,
                                           BufferSize*sizeof(FFT_CACHE_REQUEST));
  if ((InitFFTCache(FFTCacheSize, height, width, 2*FTKernelWidth) == B_FALSE)
      || (FFTCacheRequests == NULL)) {
    Abort = B_TRUE;
    draw_mesg("Abort : InitFFTCache() failed in SequenceTrackPoints()");
  }
    
  /* allocate memory */
  buf1     = (double *) XvgMM_Alloc((void *) NULL, BufferSize*sizeof(double));
  buf2     = (double *) XvgMM_Alloc((void *) NULL, BufferSize*sizeof(double)); 
  buf3     = (double *) XvgMM_Alloc((void *) NULL, BufferSize*sizeof(double));
  buf4     = (double *) XvgMM_Alloc((void *) NULL, BufferSize*sizeof(double));
  buf5     = (double *) XvgMM_Alloc((void *) NULL, BufferSize*sizeof(double));
  buf6     = (double *) XvgMM_Alloc((void *) NULL, BufferSize*sizeof(double));
  IndexMap = (int *)    XvgMM_Alloc((void *) NULL, BufferSize*sizeof(int));
  ImgPatch = (double *) XvgMM_Alloc((void *) NULL,
				    4*FTKernelWidth*FTKernelWidth*sizeof(double));
  if ((buf1 == NULL) || (buf2 == NULL) || (buf3 == NULL) || (buf4 == NULL) ||
      (buf5 == NULL) || (buf6 == NULL) || (ImgPatch == NULL) || (IndexMap == NULL)) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort: can't allocate in SequenceTrackPoints");
    draw_mesg(cbuf);
    return;
  }

  /* verbose */
  printf("SequenceTrackPoints(), Version %d...\n", VERSIONUM);	   

  /* loop to process the images */
  parms_LoadImageFile[0] = fname;
  NFiles = 0;
  do {
    /* start loop timer */
    stime = XvgGetTime();

    /* extract the index and mantissa of the first file */
    sprintf(LocalCbuf, "%s", fname);
    FirstIndex = ExtractIndex(LocalCbuf);
    sprintf(FirstFileName, "%s", fname);
    for (k = (strlen(fname) - 1); (k >= 0) && (fname[k] != '.'); k--);
    FirstFileName[k] = '\0';
    
    /* load the first image */
    LoadImageFile(inpixels, inpixels, height, width, parms_LoadImageFile);
    if (Abort) {
      fclose(fpimgs);
      VerboseFlag = btmp;
      return;
    }
    VerboseFlag = btmp;
    if (VerboseFlag)
      printf("SequenceTrackPoints() : Loaded image file %s\n", fname);
   
    /* Normalize the image, this improves performance slightly */
    for (k = 0; k < height*width; k++)
      inpixels[k] = inpixels[k]/255.0;

    /* clear the stack */
    ImageStackTop = 0;
    
    /* fetch the next filename, skip names preceeded by a "!" */
	do {
      if (fscanf(fpimgs, "%s\n", fname) != 1) {
        sprintf(cbuf,"SequenceTrackPoints() :  failed to read filename from %s",
	            ParmPtrs[0]);
        draw_mesg(cbuf);
        Abort = B_TRUE;
        fclose(fpimgs);
        VerboseFlag = btmp;
        return;
	  }
    } while (fname[0] == '!');
    
    /* extract the index and mantissa (full and stripped) of the second file */
    sprintf(LocalCbuf, "%s", fname);
    SecondIndex = ExtractIndex(LocalCbuf);
    sprintf(SecondFileName, "%s", fname);
    for (k = (strlen(fname) - 1); (k >= 0) && (fname[k] != '.'); k--);
    SecondFileName[k] = '\0';
    for (k = (strlen(SecondFileName) - 1);
	 (k >= 0) && (SecondFileName[k] != '/'); k--);
    if (SecondFileName[k] == '/')
      sprintf(FileNameStripped, "%s", &(SecondFileName[k+1]));
    else
      sprintf(FileNameStripped, "%s", SecondFileName);

    /* load the next image */
    LoadImageFile(outpixels, outpixels, height, width, parms_LoadImageFile);
    if (Abort) {
      fclose(fpimgs);
      VerboseFlag = btmp;
      return;
    }
    VerboseFlag = btmp;
    if (VerboseFlag)
      printf("SequenceTrackPoints() : Loaded image file %s\n", fname);
    
    /* start loop timer */
    stime0 = XvgGetTime();

    /* Normalize the image, this improves performance slightly */
    for (k = 0; k < height*width; k++)
      outpixels[k] = outpixels[k]/255.0;

    /* show progress */
    if (VerboseFlag)
      printf("SequenceTrackPoints() : Processing files %s.* (%d) and %s.* (%d)..\n",
	     FirstFileName, FirstIndex, SecondFileName, SecondIndex);

    /* flag all tracked points to be tracked */
	for (k = 0; k < NPointsToTrack; k++)
	  ResetPointToTrackItem(PointsToTrack, k, WeightTypes);
	    
    /* calculate the required point displacements */
    draw_mesg("Calculating point displacements...");
    IncrementalTrackPoints(inpixels, outpixels, SmallMask, height, width, BugoutT, ObjFunT, 
                           PointsToTrack, NPointsToTrack, FTKernelWidth, 
                           MaxMatchSearchRadiusX, MaxMatchSearchRadiusY,
                           FFTCacheSize, buf1, buf2, buf3, buf4, buf5, buf6,
                           ImgPatch, IndexMap, BufferSize, &CacheHitRatio);
    if (Abort) {
      fclose(fpimgs);
      return;
    }
	NTotalErrors = 0;
	
    /* loop for multipass checking of co-registered nodes */
	for (NCoregistrationErrorRecoveryPasses = 0, ErrorRecoveryPass = B_TRUE;
	     (NCoregistrationErrorRecoveryPasses < N_COREG_ERROR_MAX) && (ErrorRecoveryPass);
	     NCoregistrationErrorRecoveryPasses++) {
	  /* loop to search for co-registration errors */
  	  for (k = 0, ErrorRecoveryPass = B_FALSE, NCoregistrationErrors = 0; k < (NPointsToTrack-1); k++) {
	    X = PointsToTrack[k].x - PointsToTrack[k].FieldEstimateDx;
        Y = PointsToTrack[k].y - PointsToTrack[k].FieldEstimateDy;
	    for (j = k+2; j < NPointsToTrack; j+=2) {
	      X0 = PointsToTrack[j].x - PointsToTrack[j].FieldEstimateDx;
	      Y0 = PointsToTrack[j].y - PointsToTrack[j].FieldEstimateDy;
	      if (sqrt((X - X0)*(X - X0) + (Y - Y0)*(Y - Y0)) <= CoregistrationThreshold) {
		    /* tag both points to be re-checked */
            PointsToTrack[j].Done   = B_FALSE;
            PointsToTrack[k].Done   = B_FALSE;
		    /* verbose */
            if (VerboseFlag)
		      printf("SequenceTrackPoints(PASS %d) : Point %d-%s (%.2f, %.2f) co-incident with %d-%s (%.2f, %.2f) requires correction...\n",
			          NCoregistrationErrorRecoveryPasses, 
			          PointsToTrack[j].index, (PointsToTrack[j].LorR == -1 ? "LEFT" : "RIGHT"), X0, Y0,
			          PointsToTrack[k].index, (PointsToTrack[k].LorR == -1 ? "LEFT" : "RIGHT"), X, Y);
		    ErrorRecoveryPass = B_TRUE;
		    NCoregistrationErrors++;
		  }
		}
	  }
	  
	  /* update total error counter */
	  NTotalErrors += NCoregistrationErrors;

	  /* co-registration error recovery, if required, using double sized FFT */
	  if (ErrorRecoveryPass) {
	    sprintf(cbuf,  "Calculating pass %d co-registration errors (%d points)...",
		        NCoregistrationErrorRecoveryPasses, NCoregistrationErrors);
        draw_mesg(cbuf);
        IncrementalTrackPoints(inpixels, outpixels, BigMask, height, width, BugoutT, ObjFunT*10.0, 
                               PointsToTrack, NPointsToTrack, FTKernelWidth*2, MaxMatchSearchRadiusX, MaxMatchSearchRadiusY,
                               FFTCacheSize, buf1, buf2, buf3, buf4, buf5, buf6,
                               ImgPatch, IndexMap, BufferSize, &CacheHitRatio);
	  }
	}

	/* first pass check for LEFT/RIGHT disparity estimates which exceed a set threshold (second pass error recovery) */
	for (k = 0, ErrorRecoveryPass = B_FALSE, NDisparityErrors = 0; k < NPointsToTrack; k+=2) {
      if ((fabs(PointsToTrack[k].FieldEstimateDx - PointsToTrack[k+1].FieldEstimateDx) > StereoThreshold) ||
	      (fabs(PointsToTrack[k].FieldEstimateDy - PointsToTrack[k+1].FieldEstimateDy) > StereoThreshold)) {
		  /* tag both sides to be redone (risky to compare relative merits of the objective functions.. */
        PointsToTrack[k  ].Done = B_FALSE;
        PointsToTrack[k+1].Done = B_FALSE;
      if (VerboseFlag)
		  printf("SequenceTrackPoints() : Point %d requires disparity error correction...\n",
			      PointsToTrack[k].index);
		ErrorRecoveryPass = B_TRUE;
		NDisparityErrors++;
	  }
	}
	NTotalErrors += NDisparityErrors;
		
	/* disparity estimate error recovery, if required, using double sized FFT and no initial guess */
	if (ErrorRecoveryPass) {
	  sprintf(cbuf,  "Calculating first pass disparity error corrections (%d points)...", NDisparityErrors);
      draw_mesg(cbuf);
      IncrementalTrackPoints(inpixels, outpixels, BigMask, height, width, BugoutT, ObjFunT*10.0, 
                             PointsToTrack, NPointsToTrack, FTKernelWidth*2, MaxMatchSearchRadiusX, MaxMatchSearchRadiusX,
                             FFTCacheSize, buf1, buf2, buf3, buf4, buf5, buf6,
                             ImgPatch, IndexMap, BufferSize, &CacheHitRatio);
	}
	
 
	/* second pass for LEFT/RIGHT disparity estimates which exceed a set threshold, using objective function selection criteria */
	for (k = 0, ErrorRecoveryPass = B_FALSE, NDisparityErrors = 0; k < NPointsToTrack; k+=2) {
      if ((fabs(PointsToTrack[k].FieldEstimateDx - PointsToTrack[k+1].FieldEstimateDx) > StereoThreshold) ||
	      (fabs(PointsToTrack[k].FieldEstimateDy - PointsToTrack[k+1].FieldEstimateDy) > StereoThreshold)) {
		  /* tag the side with the highest objective function to be redone */
		  if (PointsToTrack[k].ObjFunc < PointsToTrack[k+1].ObjFunc) {
		    PointsToTrack[k+1].GuessXOffset = -rint(PointsToTrack[k].FieldEstimateDx);
		    PointsToTrack[k+1].GuessYOffset = -rint(PointsToTrack[k].FieldEstimateDy);
            PointsToTrack[k+1].Done         = B_FALSE;
            if (VerboseFlag)
		      printf("SequenceTrackPoints() : Point %d-%s requires correction (large stereo disparity)...\n",
			          PointsToTrack[k+1].index, (PointsToTrack[k+1].LorR == -1 ? "LEFT" : "RIGHT"));
		  }
		  else {
		    PointsToTrack[k].GuessXOffset = -rint(PointsToTrack[k+1].FieldEstimateDx);
		    PointsToTrack[k].GuessYOffset = -rint(PointsToTrack[k+1].FieldEstimateDy);
            PointsToTrack[k].Done         = B_FALSE;
            if (VerboseFlag)
		      printf("SequenceTrackPoints() : Point %d requires stereo disparity correction...\n",
			          PointsToTrack[k].index);
		  }
		  ErrorRecoveryPass = B_TRUE;
		  NDisparityErrors++;
	  }
	}
	NTotalErrors += NDisparityErrors;
	
	/* disparity estimate error recovery, if required, using objective function selection, double sized FFT and an initial guess */
	if (ErrorRecoveryPass) {
	  sprintf(cbuf,  "Calculating second pass disparity error corrections (%d points)...", NDisparityErrors);
      draw_mesg(cbuf);
      IncrementalTrackPoints(inpixels, outpixels, BigMask, height, width, BugoutT, ObjFunT*10.0, 
                             PointsToTrack, NPointsToTrack, FTKernelWidth*2, StereoThreshold, StereoThreshold,
                             FFTCacheSize, buf1, buf2, buf3, buf4, buf5, buf6,
                             ImgPatch, IndexMap, BufferSize, &CacheHitRatio);
	}
	
	/* reset guesses to 0 */
	for (k = 0; k < NPointsToTrack; k++) {
	  PointsToTrack[k].GuessXOffset = 0;
	  PointsToTrack[k].GuessYOffset = 0;
	}
 
	/* final check for co-registered nodes to see if any errors remain */
	for (k = 0, ErrorRecoveryPass = B_FALSE, NCoregistrationErrorsRemaining = 0; k < (NPointsToTrack-1); k++) {
	  X = PointsToTrack[k].x - PointsToTrack[k].FieldEstimateDx;
      Y = PointsToTrack[k].y - PointsToTrack[k].FieldEstimateDy;
	  for (j = k+1; j < NPointsToTrack; j++) {
	    X0 = PointsToTrack[j].x - PointsToTrack[j].FieldEstimateDx;
	    Y0 = PointsToTrack[j].y - PointsToTrack[j].FieldEstimateDy;
	    if ((fabs(X - X0) < CoregistrationThreshold) && (fabs(Y - Y0) < CoregistrationThreshold)) {
		  printf("SequenceTrackPoints() : Co-registration error correction of points %d-%s and %d-%s was unsuccessfull.\n",
			     PointsToTrack[k].index, (PointsToTrack[k].LorR == -1 ? "LEFT" : "RIGHT"), 
				 PointsToTrack[j].index, (PointsToTrack[j].LorR == -1 ? "LEFT" : "RIGHT"));
		  ErrorRecoveryPass = B_TRUE;
		  NCoregistrationErrorsRemaining++;
		}
	  }
	}

	/* final check for LEFT/RIGHT disparity estimates which exceed a set threshold, to see if any errors remain */
	for (k = 0, ErrorRecoveryPass = B_FALSE, NDisparityErrorsRemaining = 0; k < NPointsToTrack; k+=2) {
      if ((fabs(PointsToTrack[k].FieldEstimateDx - PointsToTrack[k+1].FieldEstimateDx) > StereoThreshold) ||
	      (fabs(PointsToTrack[k].FieldEstimateDy - PointsToTrack[k+1].FieldEstimateDy) > StereoThreshold)) {
		printf("SequenceTrackPoints() : Stereo disparaity error correction of point %d was unsuccessfull.\n",
			   PointsToTrack[k].index);
		ErrorRecoveryPass = B_TRUE;
		NDisparityErrorsRemaining++;
	  }
	}
	  
    /* check abort */
    if (Abort) {
      fclose(fpimgs);
      return;
    }
	
    /* load in the displaced stereo point coordinate data file, if it exists */
    sprintf(NodeFileName, "%s.2d", FileNameStripped);
    if ((fp = fopen(NodeFileName, "r")) != NULL) {
      fclose(fp);
      draw_mesg("Loading point coordinate file...");
      if (LoadPointStereoCoordinates(NodeFileName, Points, height, B_FALSE) == B_FALSE) {
        Abort = B_TRUE;
        sprintf(cbuf,
                "SequenceTrackPoints() : Failure in subsequent LoadPointStereoCoordinates(%s)",
                NodeFileName);
        draw_mesg(cbuf);
        fclose(fpimgs);
        return;
      }
      if (VerboseFlag)
        printf("SequenceTrackPoints() : Loaded previously defined %s file\n",
                NodeFileName);
    }
    else {
      if (VerboseFlag)
        printf("SequenceTrackPoints() : No previously defined %s file\n",
                NodeFileName);
    }

    /* open file to write fit stats */
    draw_mesg("Writing stats, Updated point coordinates...");
    sprintf(LocalCbuf, "%s.stats", FileNameStripped);
    if ((fp = fopen(LocalCbuf, "w")) == NULL) {
      Abort = B_TRUE;
      sprintf(cbuf,
	      "SequenceTrackPoints() : Could not open file \"%s\"", LocalCbuf);
      draw_mesg(cbuf);
      fclose(fpimgs);
      VerboseFlag = btmp;
      return;
    }

    /* print header into stats file */
	fprintf(fp, "Version number         : %d.\n", VERSIONUM);
    fprintf(fp, "CacheHitRatio          : %.2f%%.\n", CacheHitRatio*100.0);
    fprintf(fp, "Elapsed time           : %.3fs (%.6fs/point).\n", etime, etime/((double) NPointsToTrack));
    fprintf(fp, "WindowRadiusMin        : %.2f samples.\n", WindowRadiusMin);
    fprintf(fp, "WindowRadiusMax        : %.2f samples.\n", WindowRadiusMax);
    fprintf(fp, "FTKernelWidth          : %d pixels.\n", FTKernelWidth);
    fprintf(fp, "FFTCacheSize           : %d items.\n", FFTCacheSize);
    fprintf(fp, "MaxMatchSearchRadiusX  : %.2f pixels.\n", MaxMatchSearchRadiusX);
    fprintf(fp, "MaxMatchSearchRadiusY  : %.2f pixels.\n", MaxMatchSearchRadiusY);
	fprintf(fp, "CoregistrationErrors   : %d points.\n",  NCoregistrationErrors);
	fprintf(fp, "DisparityErrors        : %d points.\n",  NDisparityErrors);
	fprintf(fp, "CoregistrationErrors-R : %d points.\n",  NCoregistrationErrorsRemaining);
	fprintf(fp, "DisparityErrors-R      : %d points.\n",  NDisparityErrorsRemaining);
#ifdef _XVG_CM_FILTER
	fprintf(fp, "Center-of-mass filter  : ON.\n");
#else
	fprintf(fp, "Center-of-mass filter  : OFF.\n");
#endif
    fprintf(fp, "\n");

    /* copy the new point coordinates over the existing ones, write stats */
    for (k = 0, SumXL = 0, SumYL = 0, SumXR = 0, SumYR = 0; k < NPointsToTrack; k++) {
      /* fetch index, calculate new point coordinates */
      index = PointsToTrack[k].index;
      X = PointsToTrack[k].x - PointsToTrack[k].FieldEstimateDx;
      Y = PointsToTrack[k].y - PointsToTrack[k].FieldEstimateDy;

#ifdef _XVG_CM_FILTER
      /* center of mass filter */
      if (CenterOfMass(outpixels, height, width, CMFilterBuffer, xs, ys, X, Y,
	                   PointsToTrack[k].CMRadiusX, 0, &CMX, &CMY) == B_FALSE) {
        Abort = B_TRUE;
        sprintf(cbuf,
	        "SequenceTrackPoints() : Error on return from CenterOfMass() in \"%s\"", fname);
        draw_mesg(cbuf);
        fclose(fpimgs);
        VerboseFlag = btmp;
        return;
      }
      if (VerboseFlag)
        printf("SequenceTrackPoints() : CM Filter at %.2f,%.2f (CMX:%.2f, CMY:%.2f, dx:%.3f, dy:%.3f,  Radius:%.1f)\n",
                X, Y, CMX, CMY, X-CMX, Y-CMY, PointsToTrack[k].CMRadiusX);
      X = CMX;
      Y = CMY;
#endif

      /* left view */
      if (PointsToTrack[k].LorR == -1) {
        /* update point coordinates */
        Points[index].xl = X;
        Points[index].yl = Y;

        /* write stats */
        fprintf(fp, "LEFT:  index = %03d, x = %08.1f, y = %08.1f, dxl = %07.3f, dyl = %07.3f, ObjL=%8.3e, %%VAFL=%4.1f, WT:%d\n",
                PointsToTrack[k].index, PointsToTrack[k].x, PointsToTrack[k].y,
                PointsToTrack[k].FieldEstimateDx, PointsToTrack[k].FieldEstimateDy,
                PointsToTrack[k].ObjFunc, PointsToTrack[k].VAF, PointsToTrack[k].WeightType);

        /* update running sums stats */
        SumXL += PointsToTrack[k].FieldEstimateDx;
        SumYL += PointsToTrack[k].FieldEstimateDy;
      }
     
      /* right view */
      else {
        /* update point coordinates */
        Points[index].xr = X;
        Points[index].yr = Y;

        /* write stats */
        fprintf(fp, "RIGHT: index = %03d, x = %08.1f, y = %08.1f, dxr = %07.3f, dyr = %07.3f, ObjR=%8.3e, %%VAFR=%4.1f, WT:%d\n",
                PointsToTrack[k].index, PointsToTrack[k].x, PointsToTrack[k].y,
                PointsToTrack[k].FieldEstimateDx, PointsToTrack[k].FieldEstimateDy,
                PointsToTrack[k].ObjFunc, PointsToTrack[k].VAF, PointsToTrack[k].WeightType);

        /* update running sums stats */
        SumXR += PointsToTrack[k].FieldEstimateDx;
        SumYR += PointsToTrack[k].FieldEstimateDy;
      }

      /* update the PointsToTrack array */
      PointsToTrack[k].x = X;
      PointsToTrack[k].y = Y;
    }
    fclose(fp);

    /* open file to write the new stereo data coordinates */
    draw_mesg("Writing updated point coordinates to output text file...");
    sprintf(NodeFileName, "%s.2d", FileNameStripped);
    if ((fp = fopen(NodeFileName, "w")) == NULL) {
      Abort = B_TRUE;
      sprintf(cbuf,
	      "SequenceTrackPoints() : Could not open file \"%s\"", NodeFileName);
      draw_mesg(cbuf);
      fclose(fpimgs);
      VerboseFlag = btmp;
      return;
    }

    /* loop to write point information to text file and MATLAB arrays */
    for (k = 0; k < N_POINTS_MAX; k++) {
      /* write information to MATLAB arrays */
      XLs[k]   = Points[k].xl * XMult;
      YLs[k]   = ((double) 2*height) - (Points[k].yl * 2.0) - 1.0;
      VAFLs[k] = Points[k].VAFL;
      ObjLs[k] = Points[k].ObjL;
      XRs[k]   = Points[k].xr * XMult;
      YRs[k]   = ((double) 2*height) - (Points[k].yr * 2.0) - 1.0;
      VAFRs[k] = Points[k].VAFR;
      ObjRs[k] = Points[k].ObjR;

      /* write to CMISS .2d file */
      if (Points[k].valid == 1) {
        fprintf(fp, "Node: %03d CMF %.2f %.2f NULL\n", k, Points[k].CMRadiusXL, Points[k].CMRadiusXR);
        fprintf(fp, "%.3f %.3f %.3f %.3f\n",
                Points[k].xl * XMult, ((double) 2*height) - (Points[k].yl * 2.0) - 1.0,
                Points[k].xr * XMult, ((double) 2*height) - (Points[k].yr * 2.0) - 1.0);
      }
    }
    fclose(fp);

    /* calculate data statistics */
    AvgXL = SumXL / ((double) (NPointsToTrack/2));
    AvgYL = SumYL / ((double) (NPointsToTrack/2));
    AvgXR = SumXR / ((double) (NPointsToTrack/2));
    AvgYR = SumYR / ((double) (NPointsToTrack/2));
    for (k = 0, SumXL = 0, SumYL = 0, SumXR = 0, SumYR = 0; k < NPointsToTrack; k++) {
      /* left view */
      if (PointsToTrack[k].LorR == -1) {
        SumXL += ((PointsToTrack[k].FieldEstimateDx - AvgXL)*(PointsToTrack[k].FieldEstimateDx - AvgXL));
        SumYL += ((PointsToTrack[k].FieldEstimateDy - AvgYL)*(PointsToTrack[k].FieldEstimateDy - AvgYL));
      }
      /* right view */
      else {
        SumXR += ((PointsToTrack[k].FieldEstimateDx - AvgXR)*(PointsToTrack[k].FieldEstimateDx - AvgXR));
        SumYR += ((PointsToTrack[k].FieldEstimateDy - AvgYR)*(PointsToTrack[k].FieldEstimateDy - AvgYR));
      }
    }
    SDXL = sqrt(SumXL / ((double) (NPointsToTrack/2)));
    SDYL = sqrt(SumYL / ((double) (NPointsToTrack/2)));
    SDXR = sqrt(SumXR / ((double) (NPointsToTrack/2)));
    SDYR = sqrt(SumYR / ((double) (NPointsToTrack/2)));

    /* write the MATLAB output files */
    draw_mesg("Writing matlab output file...");
    sprintf(LocalCbuf, "%s_data.mat", FileNameStripped);
    WriteOpticalFlowMatlabFiles(XLs, YLs, VAFLs, ObjLs, XRs, YRs, VAFRs, ObjRs, N_POINTS_MAX, LocalCbuf);
    if (VerboseFlag)
      printf("SequenceTrackPoints() : Wrote MATLAB point data files...\n");

    /* display this image, if in interactive mode */
    if (InteractiveFlag) {
      ShowImage(outpixels);
      Zero = 0;
      OpticalFlowLoadNodesParms[0] = (void *) NodeFileName;
      OpticalFlowLoadNodesParms[1] = (void *) &Zero;
      OpticalFlowLoadNodes(outpixels, outpixels, height, width, OpticalFlowLoadNodesParms);
      OpticalFlowNodesOverlay(ProcessedImageDrawingArea);
	
	  /* write the color image with overlay to disk */
      draw_mesg("Writing tiff image file...");
      InMemoryPixmapFlag = B_TRUE;
      RealtoX(outpixels, ProcessedPixelsX, height, width, 1.0, 0.0, MaxScaleVal, MinScaleVal, 1);
      XPutImage(UxDisplay, InMemoryPixmap, gc, pixel_image, 0, 0, 0, 0, width, height);
      OpticalFlowNodesOverlay(ProcessedImageDrawingArea);

      /* write the images to disk */
      image = XGetImage(UxDisplay, InMemoryPixmap, 0, 0, width, height, (unsigned long) 0xffffffff, ZPixmap);
      XmUpdateDisplay(UxTopLevel);
      XSync(UxDisplay, B_FALSE);

      /* write the file */
      for (i = 0; i < 256; i++)
        cells[i].pixel = i;
      XQueryColors(UxDisplay, cmap, cells, 256);
      XSync(UxDisplay, B_FALSE);
      sprintf(NodeFileName, "%s.%04d.tif", FileNameStripped, NFiles);
      if (WriteTiffFileColor(image, 0, 0, height, width, NodeFileName, NULL, cells, 1) == B_FALSE) {
        sprintf(cbuf, "SequenceTrackPoints() : Error writing to file %s", LocalCbuf);
        draw_mesg(cbuf);
        Abort = B_TRUE;
        fclose(fpimgs);
        VerboseFlag = btmp;
        XDestroyImage(image);
        InMemoryPixmapFlag = B_FALSE;
        XFreePixmap(UxDisplay, InMemoryPixmap);
        XDestroyImage(pixel_image);
        return;
      }
      XDestroyImage(image);
      InMemoryPixmapFlag = B_FALSE;
    }

    /* increment file counter */
	NFiles++;

    /* check memory */
    sprintf(cbuf, "SequenceTrackPoints(file #%d, Memory : %d KB)", 
                   FirstIndex, XvgMM_GetMemSize()/1000);	   
    XvgMM_MemoryCheck(cbuf);

    /* stop timer */    
    etime = XvgGetTime() - stime;

    /* show progress */
    printf("SequenceTrackPoints(V%d): File #%d => Errors corrected:%2d, disparity errors:%2d, coregistration errors:%2d, Mem:%4d MB, Time:%.2fs (%.2fs).\n",
	       VERSIONUM, FirstIndex, NTotalErrors, NDisparityErrorsRemaining, NCoregistrationErrorsRemaining, XvgMM_GetMemSize()/1000000,
		   etime,  etime - (stime0 - stime));	   
  } while ((SecondIndex != StopImgIndex) && (Abort == B_FALSE));


  /* verbose */
  if (VerboseFlag)
    printf("SequenceTrackPoints() : Freeing memory...\n");

  /* free storage */
  XvgMM_Free(ImgPatch);
  XvgMM_Free(buf1);
  XvgMM_Free(buf2);
  XvgMM_Free(buf3);
  XvgMM_Free(buf4);
  XvgMM_Free(buf5);
  XvgMM_Free(buf6);
  
  /* clean up */
  fclose(fpimgs);
}

void InterpolateSequence(double *inpixels, double *outpixels,
		         int height, int width, void *ParmPtrs[])
{
  STEREO_POINT Points0[N_POINTS_MAX], Points1[N_POINTS_MAX];
  DOUBLE_ARRAY *parameters;
  FILE *fp, *fpimgs;
  void *parms_LoadImageFile[4];
  boolean_t btmp, rc, StartImgIndexFound, StopImgIndexFound;
  double step, count;
  int index, NKeyFrameImages, nfiles, StartImgIndex, StopImgIndex, NImages, FirstIndex, SecondIndex, k;
  char fname[1024], FirstFileName[1024], FileNameStripped[512], LocalCbuf[CBUF_SIZE],
    NodeFileName[512], SecondFileName[1024];

  /* show version number */
  printf("InterpolateSequence(), Version %d...\n", VERSIONUM);	   

  /* fetch the extra input parameters */
  parameters = (DOUBLE_ARRAY *) ParmPtrs[3];
  if (parameters->n < 2) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort : at least 2 parms required in InterpolateSequence()");
    draw_mesg(cbuf);
    return;
  }
  StartImgIndex = (parameters->data)[0];
  StopImgIndex  = (parameters->data)[1];

  /* check input parameters */
  if (StopImgIndex <= StartImgIndex) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "InterpolateSequence() : Invalid start/stop indicies (%d, %d)",
	    StartImgIndex, StopImgIndex);
    draw_mesg(cbuf);
    return;
  }

  /* open the image listing file */
  draw_mesg("Reading image listing file...");
  if ((fpimgs = fopen(ParmPtrs[0], "r")) == NULL) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "InterpolateSequence() : Could not open file \"%s\"", ParmPtrs[0]);
    draw_mesg(cbuf);
    return;
  }

  /* find the start/stop indicies in the image listing file */
  draw_mesg("Parsing the image listing file...");
  for (StartImgIndexFound = B_FALSE, StopImgIndexFound = B_FALSE,
       NKeyFrameImages = 0;
       (fscanf(fpimgs, "%s\n", fname) == 1) && (StopImgIndexFound == B_FALSE);) {
    /* parse the filename to extract the index */
    sprintf(LocalCbuf, "%s", fname);
    index = ExtractIndex(LocalCbuf);

    /* compare to start index */
    if (index == StartImgIndex) {
      StartImgIndexFound = B_TRUE;
      if (StopImgIndexFound) {
	Abort = B_TRUE;
	sprintf(cbuf,
		"InterpolateSequence() : Misordered filenames in \"%s\"", ParmPtrs[0]);
	draw_mesg(cbuf);
	fclose(fpimgs);
	return;
      }
      if (VerboseFlag)
	printf("InterpolateSequence() : First file to process is %s\n", fname);
    }

    /* compare to stop index */
    if (index == StopImgIndex) {
      StopImgIndexFound = B_TRUE;
      if (VerboseFlag)
	printf("InterpolateSequence() : Last file to process is %s\n", fname);
    }

    /* increment the keyframe counter, if require d*/
    if (StartImgIndexFound && !StopImgIndexFound)
      NKeyFrameImages++;
  }

  /* check that the starting image index was found */
  if ((StartImgIndexFound == B_FALSE) || (StopImgIndexFound == B_FALSE)) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "InterpolateSequence():Can't find either %d or %d indicies in \"%s\"",
	    StartImgIndex, StopImgIndex, ParmPtrs[0]);
    draw_mesg(cbuf);
    fclose(fpimgs);
    return;
  }

  /* show info */
  NImages = StopImgIndex - StartImgIndex + 1;
  if (VerboseFlag)
    printf("InterpolateSequence() : %d images to process (start=%d, stop=%d)...\n",
	   NKeyFrameImages, StartImgIndex, StopImgIndex);
  
  /* scroll back to the starting image filename */
  fseek(fpimgs, 0, 0);
  do {
    fscanf(fpimgs, "%s\n", fname);
    sprintf(LocalCbuf, "%s", fname);
    index = ExtractIndex(LocalCbuf);
  } while (index != StartImgIndex);

  /* loop to process the images */
  parms_LoadImageFile[0] = fname;
  do {
    /* reset valid indicies */
    for (k = 0; k < N_POINTS_MAX; k++)
      Points0[k].valid = -1;

    /* extract the index and mantissa of the first file */
    sprintf(LocalCbuf, "%s", fname);
    FirstIndex = ExtractIndex(LocalCbuf);
    sprintf(FirstFileName, "%s", fname);
    for (k = (strlen(fname) - 1); (k >= 0) && (fname[k] != '.'); k--);
    FirstFileName[k] = '\0';
    for (k = (strlen(FirstFileName) - 1);
        (k >= 0) && (FirstFileName[k] != '/'); k--);
    if (FirstFileName[k] == '/')
      sprintf(FileNameStripped, "%s", &(FirstFileName[k+1]));
    else
      sprintf(FileNameStripped, "%s", FirstFileName);
    
    /* load the stereo point coordinates from the starting point data file */
    draw_mesg("Loading starting stereo point coordinates...");
    sprintf(NodeFileName, "%s.2d", FileNameStripped);
    if (LoadPointStereoCoordinates(NodeFileName, Points0, height, B_TRUE) == B_FALSE) {
      Abort = B_TRUE;
      sprintf(cbuf,
	      "InterpolateSequence() : Failure loading in LoadPointStereoCoordinates(%s)",
              NodeFileName);
      draw_mesg(cbuf);
      fclose(fpimgs);
      return;
    }
    if (VerboseFlag)
      printf("InterpolateSequence() : Loaded file %s\n", NodeFileName);
   
    /* fetch the next filename */ 
    if (fscanf(fpimgs, "%s\n", fname) != 1) {
      sprintf(cbuf,"InterpolateSequence() :  failed to read filename from %s",
	      ParmPtrs[0]);
      draw_mesg(cbuf);
      Abort = B_TRUE;
      fclose(fpimgs);
      VerboseFlag = btmp;
      return;
    }
    
    /* extract the index and mantissa of the second file */
    sprintf(LocalCbuf, "%s", fname);
    SecondIndex = ExtractIndex(LocalCbuf);
    sprintf(SecondFileName, "%s", fname);
    for (k = (strlen(fname) - 1); (k >= 0) && (fname[k] != '.'); k--);
    SecondFileName[k] = '\0';
    for (k = (strlen(SecondFileName) - 1);
	 (k >= 0) && (SecondFileName[k] != '/'); k--);
    if (SecondFileName[k] == '/')
      sprintf(FileNameStripped, "%s", &(SecondFileName[k+1]));
    else
      sprintf(FileNameStripped, "%s", SecondFileName);

    /* load coordinates from the second file */
    sprintf(NodeFileName, "%s.2d", FileNameStripped);
    if (LoadPointStereoCoordinates(NodeFileName, Points1, height, B_TRUE) == B_FALSE) {
      Abort = B_TRUE;
      sprintf(cbuf,
	      "InterpolateSequence() : Failure loading in LoadPointStereoCoordinates(%s)",
              NodeFileName);
      draw_mesg(cbuf);
      fclose(fpimgs);
      return;
    }
    if (VerboseFlag)
      printf("InterpolateSequence() : Loaded file %s\n", NodeFileName);

    /* restore the Y coordinates to the RGB type, double X coordinate if required */
    for (k = 0; k < N_POINTS_MAX; k++) {
      if (Points0[k].valid == 1) {
        if (SGIImageHalfWidth) {
		  Points0[k].xl *= 2.0;
		  Points0[k].xr *= 2.0;
		  Points1[k].xl *= 2.0;
		  Points1[k].xr *= 2.0;
		}
        Points0[k].yl = ((double) 2*height) - (Points0[k].yl * 2.0) - 1.0;
        Points0[k].yr = ((double) 2*height) - (Points0[k].yr * 2.0) - 1.0;
        Points1[k].yl = ((double) 2*height) - (Points1[k].yl * 2.0) - 1.0;
        Points1[k].yr = ((double) 2*height) - (Points1[k].yr * 2.0) - 1.0;
      }
    }

    /* loop between the files to interpolate */
    for (count = 1.0, nfiles = FirstIndex+1, step = SecondIndex - FirstIndex; nfiles < SecondIndex;
         nfiles++, count+=1.0) {
      /* show banner */
      sprintf(cbuf, "Writing point coordinates to output text file %s...", NodeFileName);
      draw_mesg(cbuf);
	  if (VerboseFlag)
	    printf("%s\n",  cbuf);

      /* open file to write the new stereo data coordinates */
      sprintf(NodeFileName, "%s.%05d.2d", ParmPtrs[1], nfiles);
      if ((fp = fopen(NodeFileName, "w")) == NULL) {
        Abort = B_TRUE;
        sprintf(cbuf,
	        "InterpolateSequence() : Could not open file \"%s\"", NodeFileName);
        draw_mesg(cbuf);
        fclose(fpimgs);
        VerboseFlag = btmp;
        return;
      }

      /* loop to write points */
      for (k = 0; k < N_POINTS_MAX; k++) {
        if (Points0[k].valid == 1) {
          fprintf(fp, "Node: %03d CMF %.2f %.2f NULL\n", k, Points0[k].CMRadiusXL, Points0[k].CMRadiusXR);
          fprintf(fp, "%.3f %.3f %.3f %.3f\n",
                  Points0[k].xl + (Points1[k].xl - Points0[k].xl)*(count/step),
                  Points0[k].yl + (Points1[k].yl - Points0[k].yl)*(count/step),
                  Points0[k].xr + (Points1[k].xr - Points0[k].xr)*(count/step),
                  Points0[k].yr + (Points1[k].yr - Points0[k].yr)*(count/step));
        }
      }

      /* close the output file */
      fclose(fp);

      /* verbose */
      printf("InterpolateSequence() : Wrote interpolated output file %s.\n", NodeFileName);
    }

    /* check memory */
    sprintf(cbuf, "InterpolateSequence(file #%d, Memory : %d KB)", 
                   FirstIndex, XvgMM_GetMemSize()/1000);	   
    XvgMM_MemoryCheck(cbuf);
  } while ((SecondIndex != StopImgIndex) && (Abort == B_FALSE));

  /* clean up */
  fclose(fpimgs);

  /* transfer data */
  MoveDoubles(inpixels, outpixels, height*width);
}


/******************     OpticalFlowMakeMovie   *************************
*
*   function : OpticalFlowMakeMovie()
*
*   parameters (loaded):
*              *((char *)    ParmPtrs[0])  = Input list filename
*              *((char *)    ParmPtrs[1])  = Output directory for tiff files
*              *((int  *)    ParmPtrs[2])  = 0/1 (1 : Flip image vertically)
*
***********************************************************************/
void OpticalFlowMakeMovie(double *inpixels, double *outpixels,
			  int height, int width, void *ParmPtrs[])
{
  STEREO_POINT *Points;
  extern Widget ProcessedImageTBG, ProcessedImageShell,
  ProcessedImageDrawingArea;
  XImage *pixel_image, *image;
  XColor cells[256];
  void *parms[4];
  double *lpixels, lmin, lmax, *XLs, *XRs, *YLs, *YRs, XMult;
  FILE *fp, *fpc, *fpl;
  char fname[512], mantissa[512], nodefname[512], outfname[512],
  CorrFName[512], NodeFileNameStripped[512], matfname[512];
  int x, y, i, j, k, lheight, lwidth, vi, NImages, Flip;
  char *PixelsX;

  /* load input parameters */  
  Flip = *((int *) ParmPtrs[2]);

  /* check for half width images */
  if (SGIImageHalfWidth)
    XMult = 2.0;
  else
    XMult = 1.0;

  /* allocate fixed size memory */
  Points = (STEREO_POINT *) XvgMM_Alloc((void *) NULL, N_POINTS_MAX*sizeof(STEREO_POINT));
  XLs    = (double *)       XvgMM_Alloc((void *) NULL, N_POINTS_MAX*sizeof(double));
  XRs    = (double *)       XvgMM_Alloc((void *) NULL, N_POINTS_MAX*sizeof(double));
  YLs    = (double *)       XvgMM_Alloc((void *) NULL, N_POINTS_MAX*sizeof(double));
  YRs    = (double *)       XvgMM_Alloc((void *) NULL, N_POINTS_MAX*sizeof(double));
  if ((XLs == NULL) || (XRs == NULL) || (YLs == NULL) || (YRs == NULL) || (Points == NULL)) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort : Failed to allocate fixed sized buffers in OpticalFlowmakeMovie()");
    draw_mesg(cbuf);
    return;
  }

#ifdef SGI
  /* create in-memory pixmap, use malloc() because memory freed by X server */
  if ((PixelsX = (char *) malloc(height*width)) == NULL) {
    ErrorMsg("OpticalFlowmakeMovie() : Error allocating for X image buffer");
    return;
  }
  pixel_image = XCreateImage(UxDisplay, XDefaultVisual(UxDisplay, UxScreen),
			     8, ZPixmap, 0, PixelsX, width,
			     height, 8, 0);
  InMemoryPixmap = XCreatePixmap(UxDisplay, RootWindow(UxDisplay, UxScreen),
				 width, height, 8);
  InMemoryPixmapFlag = B_TRUE;
#else
  /* check that the processed image window is popped up */
  if (IsTrue(UxGetSet(ProcessedImageTBG)) == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort: Processed image window not popped up in OpticalFlowMakeMovie");
    return;
  }
  XRaiseWindow(UxDisplay, XtWindow(ProcessedImageShell));
  XSync(UxDisplay, B_FALSE);

  /* disable screen saver */
  ScreenSaverOff();
#endif

  /* open the image listing file */
  if ((fp = fopen(ParmPtrs[0], "r")) == NULL) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "OpticalFlowMakeMovie() : Could not open file \"%s\"",
	    ParmPtrs[0]);
    draw_mesg(cbuf);
#ifdef SGI
    InMemoryPixmapFlag = B_FALSE;
    XFreePixmap(UxDisplay, InMemoryPixmap);
    XDestroyImage(pixel_image);
#endif
    return;
  }

  /* open the output image listing file */
  sprintf(fname, "%smovie.doc", ParmPtrs[1]);
  if ((fpl = fopen(fname, "w")) == NULL) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "OpticalFlowMakeMovie() : Could not open file \"%s\"", fname);
    draw_mesg(cbuf);
#ifdef SGI
    InMemoryPixmapFlag = B_FALSE;
    XFreePixmap(UxDisplay, InMemoryPixmap);
    XDestroyImage(pixel_image);
#endif
    return;
  }

  /* open the image listing conversion file */
  sprintf(CorrFName, "%s.conversion", ParmPtrs[0]);
  if ((fpc = fopen(CorrFName, "w")) == NULL) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "OpticalFlowMakeMovie() : Could not open file \"%s\"", CorrFName);
    draw_mesg(cbuf);
#ifdef SGI
    InMemoryPixmapFlag = B_FALSE;
    XFreePixmap(UxDisplay, InMemoryPixmap);
    XDestroyImage(pixel_image);
#endif
    return;
  }

  /* loop for all images in the file */
  NImages = 0;
  while ((fscanf(fp, "%s\n", fname) == 1) && (Abort == B_FALSE)) {
    /* verbose, if required */
    if (VerboseFlag)
      printf("OpticalFlowMakeMovie() : processing file %s...\n", fname);
	  
	/* get rid of "!" character if required */
	if (fname[0] == '!') {
	  sprintf(cbuf, "%s", &(fname[1]));
	  sprintf(fname,  "%s",  cbuf);
	}
    
    /* parse the file name */
    sprintf(mantissa, "%s", fname);
    for (k = (strlen(fname) - 1); (k >= 0) && (fname[k] != '.'); k--);
    mantissa[k] = '\0';
    for (k = (strlen(mantissa) - 1);
	 (k >= 0) && (mantissa[k] != '/'); k--);
    if (mantissa[k] == '/')
      sprintf(NodeFileNameStripped, "%s", &(mantissa[k+1]));
    else
      sprintf(NodeFileNameStripped, "%s", mantissa);

    /* load the input file */
    if (GetFieldFromSGIRGBFile(fname, 1, &lpixels, &lheight, &lwidth,
		               &lmin, &lmax) == B_FALSE) {
      Abort = B_TRUE;
      sprintf(cbuf, "Error reading file %s in OpticalFlowMakeMovie()",
	      fname);
      draw_mesg(cbuf);
      fclose(fpc);
      fclose(fpl);
      fclose(fp);
#ifdef SGI
      InMemoryPixmapFlag = B_FALSE;
      XFreePixmap(UxDisplay, InMemoryPixmap);
      XDestroyImage(pixel_image);
#else
      ScreenSaverOn();
#endif
      XvgMM_Free(lpixels);
      return;
    }

    /* write the data to the processed image window */
    RealtoX(lpixels, PixelsX, height, width,
	    lmax, lmin, MaxScaleVal, MinScaleVal, 1);

#ifdef SGI
    XPutImage(UxDisplay, InMemoryPixmap, gc,
	      pixel_image, 0, 0, 0, 0, width, height);
#else
    XPutImage(UxDisplay, XtWindow(ProcessedImageDrawingArea), gc,
	      ProcessedImageXObj, 0, 0, 0, 0, width, height);
#endif
    /* add the node overlay */
    sprintf(nodefname, "%s.2d", NodeFileNameStripped);
    vi = 0;
    parms[0] = nodefname;
    parms[1] = &vi;
    OpticalFlowLoadNodes(inpixels, outpixels, height, width, parms);
    if (Abort)
      return;
    OpticalFlowNodesOverlay(ProcessedImageDrawingArea);
    XmUpdateDisplay(UxTopLevel);
    XSync(UxDisplay, B_FALSE);

    /* fetch the current color cell definitions */
    for (i = 0; i < 256; i++)
      cells[i].pixel = i;
    XQueryColors(UxDisplay, cmap, cells, 256);
    XSync(UxDisplay, B_FALSE);
    
    /* write the images to disk */
#ifdef SGI
    image = XGetImage(UxDisplay, InMemoryPixmap, 0, 0, width, height,
		      (unsigned long) 0xffffffff,
		      ZPixmap);
#else
    image = XGetImage(UxDisplay, XtWindow(ProcessedImageDrawingArea),
		      ProcessedWinHOffset, ProcessedWinVOffset,
		      ProcessedWinWidth, ProcessedWinHeight,
		      (unsigned long) 0xffffffff,
		      ZPixmap);
#endif
    XSync(UxDisplay, B_FALSE);

    /* write the file */
    sprintf(outfname, "%simg.%04d.tif", ParmPtrs[1], NImages++);
    if (WriteTiffFileColor(image, 0, 0, height, width, outfname, NULL, cells,
			   1) == B_FALSE) {
      sprintf(cbuf, "Error writing file %s in OpticalFlowMakeMovie()",
	      outfname);
      draw_mesg(cbuf);
      Abort = B_TRUE;
      fclose(fpc);
      fclose(fp);
      fclose(fpl);
#ifdef SGI
      XDestroyImage(image);
      InMemoryPixmapFlag = B_FALSE;
      XFreePixmap(UxDisplay, InMemoryPixmap);
      XDestroyImage(pixel_image);
#else
      ScreenSaverOn();
#endif
      XvgMM_Free(lpixels);
      return;
    }
    XDestroyImage(image);

    /* load the stereo point coordinates from the starting point data file */
    draw_mesg("Loading starting stereo point coordinates...");
    if (LoadPointStereoCoordinates(nodefname, Points, height, B_TRUE) == B_FALSE) {
      Abort = B_TRUE;
      sprintf(cbuf, "OpticalFlowMakeMovie() : Failure loading in LoadPointStereoCoordinates(%s)", nodefname);
      draw_mesg(cbuf);
      fclose(fpc);
      fclose(fpl);
      fclose(fp);
      return;
    }
   
    /* loop to write data to MATLAB arrays */
    for (k = 0; k < N_POINTS_MAX; k++) {
      /* write information to MATLAB arrays */
      XLs[k]   = Points[k].xl * XMult;
      YLs[k]   = ((double) 2*height) - (Points[k].yl * 2.0) - 1.0;
      XRs[k]   = Points[k].xr * XMult;
      YRs[k]   = ((double) 2*height) - (Points[k].yr * 2.0) - 1.0;
    }

    /* write the MATLAB output files */
    draw_mesg("Writing matlab output file...");
    sprintf(matfname, "%s_data.mat", NodeFileNameStripped);
    WriteOpticalFlowMatlabFiles(XLs, YLs, NULL, NULL, XRs, YRs, NULL, NULL, N_POINTS_MAX, matfname);
    if (VerboseFlag)
      printf("OpticalFlowMakeMovie() : Wrote MATLAB point data file %s...\n", matfname);

    /* write to the conversion file */
    fprintf(fpc, "%s - %s\n", fname, outfname); 

    /* write to the listing file */
    fprintf(fpl, "%s\n", outfname); 

    /* show stuff */
    sprintf(cbuf, "%s...", fname);
    draw_mesg(cbuf);

    /* check memory */
    sprintf(cbuf, "OpticalFlowMakeMovie(frame %d)", NImages);
    XvgMM_MemoryCheck(cbuf);
  }

  /* close the files */
  fclose(fpc);
  fclose(fpl);
  fclose(fp);

#ifdef SGI
  /* re-enable screen overlay */
  InMemoryPixmapFlag = B_FALSE;
  XFreePixmap(UxDisplay, InMemoryPixmap);
#else
  /* re-enable screen saver */
  ScreenSaverOn();
#endif

  /* clean up */
  XDestroyImage(pixel_image);
  XvgMM_Free(lpixels);
}

/******************     ConvertExNodeFileToMatlab   *************************
*
*   function : ConvertExNodeFileToMatlab()
*
*   parameters (loaded):
*              *((char *)    ParmPtrs[0])  = Input list filename
*              *((char *)    ParmPtrs[1])  = Output directory for tiff files
*              *((int  *)    ParmPtrs[2])  = 0/1 (1 : Flip image vertically)
*
***********************************************************************/
static boolean_t WriteExNodeMatlabFiles(double *Xs, double *Ys, double *Zs, double *Is,
                                        int N, double *Fs, int nfiles, char *fname)
{
  MATFile *DataFp;
  Matrix *a;
  
  /* open the matlab output file */
  if ((DataFp = matOpen(fname, "wb")) == NULL) {
    sprintf(cbuf, "WriteExNodeMatlabFiles : could not open file %s\n",
	    fname);
    ErrorMsg(cbuf);
    return(B_FALSE);
  }

  /* create matrix */
  a = mxCreateFull(N, 1, REAL);

  /* copy left side data */
  memcpy(mxGetPr(a), Xs, N * sizeof(double));
  mxSetName(a, "X");
  matPutMatrix(DataFp, a);

  memcpy(mxGetPr(a), Ys, N * sizeof(double));
  mxSetName(a, "Y");
  matPutMatrix(DataFp, a);

  memcpy(mxGetPr(a), Zs, N * sizeof(double));
  mxSetName(a, "Z");
  matPutMatrix(DataFp, a);

  memcpy(mxGetPr(a), Is, N * sizeof(double));
  mxSetName(a, "I");
  matPutMatrix(DataFp, a);

  mxFreeMatrix(a);

  a = mxCreateFull(nfiles, 1, REAL);
  memcpy(mxGetPr(a), Fs, nfiles * sizeof(double));
  mxSetName(a, "F");
  matPutMatrix(DataFp, a);

  mxFreeMatrix(a);

  /* close file and return */
  matClose(DataFp);
  return(B_TRUE);
}

void ConvertExNodeFileToMatlab(double *inpixels, double *outpixels,
			                   int height, int width, void *ParmPtrs[])
{
  FILE *fpl, *fp;
  int k, nnodes, nfiles, n, SingleSided;
  double x0, x1, x2, *Xs, *Ys, *Zs, *Is, *Fs;
  char fname[512], mantissa[512], matfname[512], NodeFileNameStripped[512];

  /* load input parameters */  
  SingleSided = *((int *) ParmPtrs[1]);

  /* open the image listing file */
  if ((fpl = fopen(ParmPtrs[0], "r")) == NULL) {
    Abort = B_TRUE;
    sprintf(cbuf,
	        "ConvertExNodeFileToMatlab() : Could not open file \"%s\"",
	        ParmPtrs[0]);
    draw_mesg(cbuf);
    return;
  }
  
  /* scan the file to count the number of entries */
  for (nfiles = 0; fgets(cbuf, CBUF_SIZE, fpl); nfiles++);
  fseek(fpl, 0, 0);
  
  /* parse the file name */
  if (fscanf(fpl, "%s\n", fname) != 1) {
    Abort = B_TRUE;
    sprintf(cbuf,
         "ConvertExNodeFileToMatlab() : Could not read first line in file \"%s\"",
		 ParmPtrs[0]);
    draw_mesg(cbuf);
    fclose(fpl);
    return;	
  }
  
  /* open the first input file */
  if ((fp = fopen(fname, "r")) == NULL) {
    Abort = B_TRUE;
    sprintf(cbuf,
         "ConvertExNodeFileToMatlab() : Could not open file \"%s\"", fname);
    draw_mesg(cbuf);
    fclose(fpl);
    return;
  }
	
  /* skip over the header */
  fgets(cbuf, CBUF_SIZE, fp);
  fgets(cbuf, CBUF_SIZE, fp);
  fgets(cbuf, CBUF_SIZE, fp);
  fgets(cbuf, CBUF_SIZE, fp);
  fgets(cbuf, CBUF_SIZE, fp);
  fgets(cbuf, CBUF_SIZE, fp);

  /* loop to write count the number of nodes in the file */
  for (nnodes = 0; fgets(cbuf, CBUF_SIZE, fp); nnodes++);
  if (SingleSided)
    nnodes /= 2;
  else
    nnodes /= 4;
  printf("ConvertExNodeFileToMatlab() : %d nodes in the first exnode file\n", nnodes);

  /* close the file */
  fclose(fp);
	  
  /* allocate fixed size memory */
  Xs = (double *) XvgMM_Alloc((void *) NULL, nnodes*sizeof(double));
  Ys = (double *) XvgMM_Alloc((void *) NULL, nnodes*sizeof(double));
  Zs = (double *) XvgMM_Alloc((void *) NULL, nnodes*sizeof(double));
  Is = (double *) XvgMM_Alloc((void *) NULL, nnodes*sizeof(double));
  Fs = (double *) XvgMM_Alloc((void *) NULL, nfiles*sizeof(double));
  if ((Xs == NULL) || (Ys == NULL) || (Zs == NULL) || (Is == NULL)) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort : Failed to allocate fixed sized buffers in ConvertExNodeFileToMatlab()");
    draw_mesg(cbuf);
    return;
  }

  /* loop for all images in the file */
  fseek(fpl, 0, 0);
  n = 0;
  while ((fscanf(fpl, "%s\n", fname) == 1) && (Abort == B_FALSE)) {
    /* verbose, if required */
    if (VerboseFlag)
      printf("ConvertExNodeFileToMatlab() : processing file %s...\n", fname);
    
    /* parse the file name */
    sprintf(mantissa, "%s", fname);
    for (k = (strlen(fname) - 1); (k >= 0) && (fname[k] != '.'); k--);
    mantissa[k] = '\0';
    for (k = (strlen(mantissa) - 1);
	 (k >= 0) && (mantissa[k] != '/'); k--);
    if (mantissa[k] == '/')
      sprintf(NodeFileNameStripped, "%s", &(mantissa[k+1]));
    else
      sprintf(NodeFileNameStripped, "%s", mantissa);
	
	/* extract the file index */
    for (k = (strlen(mantissa) - 1);
	 (k >= 0) && (mantissa[k] != '.'); k--);
	printf("MANTISSA: %s = %f\n",  &(mantissa[k+1]),  atof(&(mantissa[k+1])));
	Fs[n++] = atof(&(mantissa[k+1]));

    /* open the input file */
    if ((fp = fopen(fname, "r")) == NULL) {
      Abort = B_TRUE;
      sprintf(cbuf,
	          "ConvertExNodeFileToMatlab() : Could not open file \"%s\"", fname);
      draw_mesg(cbuf);
	  fclose(fpl);
      return;
    }
	
	/* skip over the header */
	fgets(cbuf, CBUF_SIZE, fp);
	fgets(cbuf, CBUF_SIZE, fp);
	fgets(cbuf, CBUF_SIZE, fp);
	fgets(cbuf, CBUF_SIZE, fp);
	fgets(cbuf, CBUF_SIZE, fp);
	fgets(cbuf, CBUF_SIZE, fp);

    /* loop to write data to MATLAB arrays */
    for (k = 0; k < nnodes; k++) {
      if (SingleSided) {
  	    /* read in the Node statement */
 	    if (fscanf(fp, "Node: %le\n", &(Is[k])) != 1) {
          Abort = B_TRUE;
          sprintf(cbuf,
	              "ConvertExNodeFileToMatlab() : Could not get Node index from file %s at line %d",
				  fname, k);
          draw_mesg(cbuf);
	      fclose(fpl);
          return;
	    }

 	    /* read in the XYZ information */
	    if (fscanf(fp, "%le %le %le\n", &(Xs[k]), &(Ys[k]), &(Zs[k])) != 3) {
          Abort = B_TRUE;
          sprintf(cbuf,
	              "ConvertExNodeFileToMatlab() : Could not get XYZ info from file %s at line %d",
				  fname, k);
          draw_mesg(cbuf);
	      fclose(fpl);
          return;
	    }
	  }
	  else {
  	    /* read in the Node statement */
 	    if (fscanf(fp, "Node: %le\n", &(Is[k])) != 1) {
          Abort = B_TRUE;
          sprintf(cbuf,
	              "ConvertExNodeFileToMatlab() : Could not get Node index from file %s at line %d",
				  fname, k);
          draw_mesg(cbuf);
	      fclose(fpl);
          return;
	    }
	  
	    /* read in the X information */
	    if (fscanf(fp, "%le %le %le %le\n", &(Xs[k]), &x0, &x1, &x2) != 4) {
          Abort = B_TRUE;
          sprintf(cbuf,
	              "ConvertExNodeFileToMatlab() : Could not get X info from file %s at line %d",
				  fname, k);
          draw_mesg(cbuf);
	      fclose(fpl);
          return;
	    }
	  
	    /* read in the Y information */
	    if (fscanf(fp, "%le %le %le %le\n", &(Ys[k]), &x0, &x1, &x2) != 4) {
          Abort = B_TRUE;
          sprintf(cbuf,
	              "ConvertExNodeFileToMatlab() : Could not get Y info from file %s at line %d",
				  fname, k);
          draw_mesg(cbuf);
	      fclose(fpl);
          return;
	    }
	  
	    /* read in the Z information */
	    if (fscanf(fp, "%le %le %le %le\n", &(Zs[k]), &x0, &x1, &x2) != 4) {
          Abort = B_TRUE;
          sprintf(cbuf,
	              "ConvertExNodeFileToMatlab() : Could not get Z info from file %s at line %d",
				  fname, k);
          draw_mesg(cbuf);
	      fclose(fpl);
          return;
	    }
	  }
    }

	/* close the file */
	fclose(fp);
	  
    /* write the MATLAB output files */
    draw_mesg("Writing matlab output file...");
    sprintf(matfname, "%s.mat", NodeFileNameStripped);
    WriteExNodeMatlabFiles(Xs, Ys, Zs, Is, nnodes, Fs, nfiles, matfname);
    if (VerboseFlag)
      printf("ConvertExNodeFileToMatlab() : Wrote MATLAB point data file %s...\n", matfname);

    /* check memory */
    sprintf(cbuf, "ConvertExNodeFileToMatlab");
    XvgMM_MemoryCheck(cbuf);
  }

  /* close the listing file */
  fclose(fpl);
}


