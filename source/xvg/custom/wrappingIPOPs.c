/***********************************************************************
*
*  Name:          wrappingIPOPs.c
*
*  Author:        Paul Charette
*
*  Last Modified: 27 Nov 1994
*
*  Purpose:       Utility routines wrapping IP operations
*                   FixHook()
*                   NonLinearLSPhaseFit()
*                   RegionBoundaryNormals()
*                   RegionExpansion()
*                   RegionFill()
*                   RegionFillIPOp()
*                   RegionFillSegmentation()
*                   Residues()
*                   Unwrap()
*                   UnwrapManual()
*                   Wrap()
*
***********************************************************************/
#include <stdio.h>
#include "UxXt.h"
#include <fcntl.h>
#include <memory.h>
#include <search.h>
#include <values.h>
#include "XvgGlobals.h"

/****************************  FixHook *************************************/
/*                                                                         */
/*  function: pretty up the fixed hook area so it can be used in fitting   */
/*                                                                         */
/***************************************************************************/
void FixHook(double *inpixels, double *outpixels, int height, int width,
	     void *ParmPtrs[])
{
  int i, x, xc, yc, y, w, h, radius, xmax, xmin;
  double eta, x0, x1;

  /* load parameters */
  xc = *((int *) ParmPtrs[0]);
  yc = *((int *) ParmPtrs[1]);
  radius = *((int *) ParmPtrs[2]);

  /* draw banner */
  draw_mesg("FixHook - Fixing fixed hook neighbourhood...");

  /* copy the data to the output buffer */
  for (i = 0; i < height*width; i++)
    outpixels[i] = inpixels[i];

  /* set pixels inside circle using a linear interp of the x boundary vals */
  for (y = yc - radius, h = -radius; y <= yc + radius; y++, h++) {
    w = sqrt((double) (radius*radius - h*h));
    if (w > 0) {
      xmin = xc - w;
      xmax = xc + w;
      x0 = inpixels[y*width + xmin];
      x1 = inpixels[y*width + xmax];
      for (x = xmin; x <= xmax; x++) {
	eta = ((double) (x - xmin)) / ((double) (xmax - xmin));
	outpixels[y*width + x] = x0*(1.0 - eta) + x1*eta;
      }
    }
  }
}


/** Non linear least squares (Newton-Raphson) phase data fitting ***********/
/*                                                                         */
/*  function: use a Newton method of optimization to solve a               */
/*            non-linear least squares minimization problem.               */
/*            the model is a simple bivariate polynomial, with a MOD op    */
/*            assume the derivative of the MOD operator is 1 everywhere    */
/*                                                                         */
/*            The x,y coordinates are normalized to [-1,1] and the         */
/*            data values over the range [0,1].                            */
/*                                                                         */
/***************************************************************************/
static struct SampleElement {
  double u, v, w;
  double *psums;
} *NonZeroSamples;
static int NNonZeroSamples, Iteration, ModelLen, Order;
static double *Jacobian, *LastX, ModVal;
void NonLinearLSPhaseFit(double *inpixels, double *outpixels,
			 int height, int width, void *ParmPtrs[])
{
  int M, N, LIW, LW, *IW;
  double *X, *W;
  MATFile *DataFp;
  Matrix *a;
  boolean_t GenCoeffs;
  double FSUMSQ, SumZ2, SumZZp2, Z, Zp, val, xdiv, ydiv, zdiv, u, v;
  double *tmpip, *tmprp, tmpr[1], zmin, zmax, Residual;
  int i, j, k, l, x, y, IFAIL, VAF, SamplingPeriod, dx, dy;
  
  /* load parameters */
  SamplingPeriod = *((int *) ParmPtrs[0]);
  Order = *((int *) ParmPtrs[1]);
  ModVal = *((double *) ParmPtrs[2]);

  ModelLen = (Order+1)*(Order+1);
  Iteration = 0;

  /* make sure contour is defined */
  if (DisplayContourComputed == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort : Undefined contour in NonLinearLSPhaseFit()");
    return;
  }

  /* find extrema of non-zero values whithin the contour */
  draw_mesg("NonLinearLSPhaseFit() : Finding data extrema within contour...");
  for (y = 0, zmin = MAXDOUBLE, zmax = -MAXDOUBLE; y < height;
       y += SamplingPeriod)
    for (x = 0; x < width; x += SamplingPeriod)
      if ((MaskPixels[y*width + x] != 0) && (inpixels[y*width + x] != 0)) {
	zmin = (inpixels[y*width + x] < zmin ? inpixels[y*width + x] : zmin);
	zmax = (inpixels[y*width + x] > zmax ? inpixels[y*width + x] : zmax);
      }
  
  /* tabulate the number and location of non-zero points within the contour */
  /* and scale x,y,z to -1..1 range                                         */
  draw_mesg("NonLinearLSPhaseFit(): Tabulating non-zero pts within contour");
  if ((NonZeroSamples = (struct SampleElement *)
       UxMalloc(height * width * sizeof(struct SampleElement))) == NULL) {
    Abort = B_TRUE;
    draw_mesg("Abort: Can't alloc for NonZeroSamples NonLinearLSPhaseFit()");
    return;
  }
  for (y = 0, NNonZeroSamples = 0, xdiv = 1.0/((double) (width-1)),
       ydiv = 1.0/((double) (height-1)), zdiv = 1.0/(zmax - zmin); y < height;
       y += SamplingPeriod)
    for (x = 0; x < width; x += SamplingPeriod)
      if ((MaskPixels[y*width + x] != 0) && (inpixels[y*width + x] != 0)) {
	NonZeroSamples[NNonZeroSamples].u = ((double) (2*x - (width-1)))*xdiv;
	NonZeroSamples[NNonZeroSamples].v = ((double) (2*y - (height-1)))*ydiv;
	NonZeroSamples[NNonZeroSamples].w = inpixels[y*width + x];
/*
	NonZeroSamples[NNonZeroSamples].w
	  = (inpixels[y*width + x] - zmin) * zdiv;
*/
	NonZeroSamples[NNonZeroSamples].psums = NULL;
	NNonZeroSamples++;
      }
  if (VerboseFlag)
    printf("Tabulated %d non-zero data points within contour in NonLinearLSPhaseFit()..\n", NNonZeroSamples);
  
  /* precompute the Jacobian matrix */
  draw_mesg("NonLinearLSPhaseFit() : Computing Jacobian...");
  if ((Jacobian =
       (double *) UxMalloc(NNonZeroSamples * ModelLen * sizeof(double)))
      == NULL) {
    Abort = B_TRUE;
    draw_mesg("Abort: Can't alloc for Jacobian NonLinearLSPhaseFit()");
    UxFree(NonZeroSamples);
    return;
  }
  for (i = 0, l = 0; i <= Order; i++)
    for (j = 0; j <= Order; j++, l++)
      for (k = 0; k < NNonZeroSamples; k++) 
	Jacobian[l*NNonZeroSamples + k]
	  = pow(NonZeroSamples[k].u, (double) i)
	    * pow(NonZeroSamples[k].v, (double) j);
  
  /* allocate storage for NAG call */
  draw_mesg("NonLinearLSPhaseFit() : Allocating storage...");
  M = NNonZeroSamples;
  N = ModelLen;
  LIW = 1;
  IW = (int *) UxMalloc(LIW * sizeof(int));
  LW = 8*N + 2*N*N + 2*M*N + 3*M;
  W = (double *) UxMalloc(LW * sizeof(double));
  LastX = (double *) UxMalloc(ModelLen * sizeof(double));

  /* look for initial guess in file "NLLSCoeffs.mat" */
  draw_mesg("NonLinearLSPhaseFit() : Loading model parameters...");
  GenCoeffs = B_TRUE;
  if ((DataFp = matOpen("NLLSCoeffs.mat", "r")) != NULL)
    if (matGetFull(DataFp, "Order", &dx, &dy, &tmprp, &tmpip) == 0)
      if (tmprp[0] == Order)
	if (matGetFull(DataFp, "Coeffs", &dx, &dy, &X, &tmpip) == 0)
	  if (dx == ModelLen)
	    GenCoeffs = B_FALSE;
  matClose(DataFp);
  
  /* initial guess not found, generate one */
  if (GenCoeffs) {
    UxFree(X);
    X = (double *) UxMalloc(ModelLen * sizeof(double));
    for (i = 0; i < ModelLen; i++)
      X[i] = 1;
  }

  /* fill last guess array */
  for (i = 0; i < ModelLen; i++)
    LastX[i] = X[i];

  /* print out guess values */
  if (VerboseFlag) {
    if (GenCoeffs == B_FALSE)
      printf("Initial guess loaded from NLLSCoeffs.mat...\n");
    else
      printf("Initial guess generated for Newton method fit :\n");
    for (i = 0; i <= Order; i++)
      for (j = 0; j <= Order; j++)
	printf("  a%d%d : %e\n", i, j, X[i*(Order+1) + j]);
  }

  /* call the minimization routine */
  draw_mesg("NonLinearLSPhaseFit() : Minimization...");
  IFAIL = -1;
  e04gef(&M, &N, X, &FSUMSQ, IW, &LIW, W, &LW, &IFAIL);
  
  /* check IFAIL value */
  if ((IFAIL != 0) && (IFAIL != 3) && ((IFAIL < 5) || IFAIL > 8)) {
    Abort = B_TRUE;
    sprintf(cbuf,"Abort : IFAIL = %d on return from e04gef() in NonLinearLSPhaseFit()",
	    IFAIL);
    draw_mesg(cbuf);
    UxFree(NonZeroSamples);
    UxFree(Jacobian);
    UxFree(X);
    UxFree(W);
    UxFree(IW);
    UxFree(LastX);
    return;
  }
  
  /* save order & model parameters in the file NLLSCoeffs.mat */
  draw_mesg("NonLinearLSPhaseFit() : Saving model parameters...");
  if ((DataFp = matOpen("NLLSCoeffsNew.mat", "w")) != NULL) {
    a = mxCreateFull(1, 1, REAL);
    tmpr[0] = Order;
    memcpy(mxGetPr(a), tmpr, sizeof(double));
    mxSetName(a, "Order");
    matPutMatrix(DataFp, a);
    tmpr[0] = PI;
    memcpy(mxGetPr(a), tmpr, sizeof(double));
    mxSetName(a, "zmax");
    matPutMatrix(DataFp, a);
    tmpr[0] = -PI;
    memcpy(mxGetPr(a), tmpr, sizeof(double));
    mxSetName(a, "zmin");
    matPutMatrix(DataFp, a);
    mxFreeMatrix(a);
    a = mxCreateFull(ModelLen, 1, REAL);
    memcpy(mxGetPr(a), X, ModelLen * sizeof(double));
    mxSetName(a, "Coeffs");
    matPutMatrix(DataFp, a);
    mxFreeMatrix(a);
    matClose(DataFp);
  }
  else {
    Abort = B_TRUE;
    draw_mesg("Abort: Can't write to NLLSCoeffs.mat in NonLinearLSPhaseFit()");
    UxFree(NonZeroSamples);
    UxFree(Jacobian);
    UxFree(X);
    UxFree(W);
    UxFree(IW);
    UxFree(LastX);
    return;
  }

  /* print out model parameters */
  if (VerboseFlag)
    for (i = 0, printf("Final model parameters :\n"); i <= Order; i++)
      for (j = 0; j <= Order; j++)
	printf("  a%d%d : %e\n", i, j, X[i*(Order+1) + j]);

  /* compute VAF and model values */
  draw_mesg("NonLinearLSPhaseFit() : Computing model values and VAF...");
  for (y=0, SumZ2=0, SumZZp2=0; y < height; y++) {
    v = ((double) (2*y - (height - 1))) * ydiv;
    for (x=0; x < width; x++)
      if (MaskPixels[y*width + x] != 0) {
	/* load values */
	Z = inpixels[y*width + x];
	u = ((double) (2*x - (width - 1))) * xdiv;
	for (i = 0, k = 0, Zp = 0; i <= Order; i++)
	  for (j = 0; j <= Order; j++, k++)
	    Zp += (X[k] * pow(u, (double) i) * pow(v, (double) j));
/*	Zp = Zp*(zmax - zmin) + zmin; */
	outpixels[y*width +x] = Zp;
	/* compute sums for VAF computation */
	if (ModVal != 0)
	  Residual = fmod(Z - Zp, ModVal);
	else
	  Residual = Z - Zp;
	SumZ2 += (Z * Z);
	SumZZp2 += (Residual * Residual);
      }
      else
	outpixels[y*width +x] = 0;
  }
  VAF = 100.0*(1.0 - SumZZp2/SumZ2);
    
  /* load final value of residual and VAF into message buffer */
  sprintf(Msg, " = IFAIL : %d, VAF : %d%%, FSUMSQ : %e, Iterations : %d",
	  IFAIL, VAF, FSUMSQ, Iteration);
  
  /* free storage */
  UxFree(NonZeroSamples);
  UxFree(Jacobian);
  UxFree(X);
  UxFree(W);
  UxFree(IW);
  UxFree(LastX);
}  

void lsfun2(int *Mp, int *Np, double *XC, double *FVECC, double *FJACC,
	    int *LJC)
{
  int i, j, k, m;
  double val, sum, modval;

  /* check abort flag */
  if (Abort == B_TRUE)
    return;

  /* compute the residuals */
  for (m = 0, sum = 0; m < NNonZeroSamples; m++) {
    /* compute sum of monomials */
    for (i = 0, val = 0; i < ModelLen; i++)
      val += (XC[i] * Jacobian[i*NNonZeroSamples + m]);
    /* perform remainder op, if required */
    if (ModVal != 0)
      FVECC[m] = fmod(val - NonZeroSamples[m].w, ModVal);
    else
      FVECC[m] = val - NonZeroSamples[m].w;
    sum += (FVECC[m]*FVECC[m]);
  }
  
  if (VerboseFlag) {
    printf("Iteration :%d, Sum of squared residuals : %e...\n",
	   Iteration, sum);
    for (i = 0, k = 0, printf("Current model parameters :\n"); i <= Order; i++)
      for (j = 0; j <= Order; j++, k++)
	printf("  X%d%d : %e (delta: %e)\n",
	       i, j, XC[k], XC[k] - LastX[k]);
    printf("\n");
    for (i = 0; i < ModelLen; i++)
      LastX[i] = XC[i];
  }

  /* transfer Jacobian data */

  for (i = 0, k = 0; i < ModelLen; i++)
    for (m = 0; m < NNonZeroSamples; m++, k++)
      FJACC[i*(*LJC) + m] = Jacobian[k];

  /* increment the counter */
  Iteration++;

  /* progress resport */
  sprintf(cbuf, "e04gef => iteration : %d, sum of squared residuals : %e...",
	  Iteration, sum);
  draw_mesg(cbuf);
}

/************************      Region expansion        *********************/
/*                                                                         */
/*  function: expand regions until they touch and fill the image           */
/*                                                                         */
/*                                                                         */
/***************************************************************************/
#define MAXITERS 200
void RegionExpansion(double *inpixels, double *outpixels, int height,
		     int width, void *ParmPtrs[])
{
  int *TdBuf, *BuBuf, *LrBuf, *RlBuf;
  boolean_t IsUnMarked, go;
  int i, j, k, x, y, nrows, ncols;

  /* allocate storage if required */
  TdBuf = XvgMM_Alloc((void *) NULL, width * sizeof(int));
  BuBuf = XvgMM_Alloc((void *) NULL, width * sizeof(int));
  LrBuf = XvgMM_Alloc((void *) NULL, height * sizeof(int));
  RlBuf = XvgMM_Alloc((void *) NULL, height * sizeof(int));
  if ((TdBuf == NULL) || (BuBuf == NULL)
      || (LrBuf == NULL) || (RlBuf == NULL)) {
    Abort = B_TRUE;
    draw_mesg("Abort : Can't allocate in RegionExpansion()");
    return;
  }

  /* if no mask defined, then define one than spans the image */
  if (DisplayContourComputed == B_FALSE) {
    for (i = 0; i < height*width; i++)
      MaskPixels[i] = 0.0;
    for (j = 1; j < (height-1); j++)
      for (i = 1; i < (width-1); i++)
	MaskPixels[j*width + i] = 1.0;
  }

  /* copy data to outpixels */
  for (i = 0; i < height*width; i++)
    outpixels[i] = inpixels[i];

  /* initialize endpoint buffers */
  for (y = 0, nrows = 0; y < height; y++) {
    for (x = 0; (x < width) && (MaskPixels[y*width+x] == 0.0); x++);
    LrBuf[y] = x;
    for (x = width-1; (x > 0) && (MaskPixels[y*width+x] == 0.0); x--);
    RlBuf[y] = x;
  }
  for (x = 0, ncols = 0; x < width; x++) {
    for (y = 0; (y < height) && (MaskPixels[y*width+x] == 0.0); y++);
    TdBuf[x] = y;
    for (y = height-1; (y > 0) && (MaskPixels[y*width+x] == 0.0); y--);
    BuBuf[x] = y;
  }
  
  /* expand regions untill they touch and fill the contour */
  draw_mesg("Region expansion...");
  k = 0;
  do {
    go = B_FALSE;

    /* top/down expansion */
    for (x = 0; (x < width) && (Abort == B_FALSE); x++) {
      /* loop through columns within image convex boundary */
      for (y = TdBuf[x], IsUnMarked = B_FALSE; y <= BuBuf[x];) {
	/* skip over unmarked pixels */
	for (; (y <= BuBuf[x]) && (outpixels[y*width+x] == 0.0); y++)
	  if (MaskPixels[y*width+x] != 0.0)
	    IsUnMarked = B_TRUE;
	/* find next unmarked pixel */
	for (; (y <= BuBuf[x]) && (outpixels[y*width+x] != 0.0); y++);
	/* mark the pixel if its inside the boundary */
	if ((MaskPixels[y*width+x] != 0.0) && (y <= BuBuf[x])) {
	  outpixels[y*width+x] = outpixels[(y-1)*width+x];
	  y++;
	}
      }
      /* is this row is entirely filled, set boundaries accordingly */
      if (IsUnMarked == B_FALSE) {
	TdBuf[x] = height-1;
	BuBuf[x] = 0;
      }
      else
	go = B_TRUE;
    }

    /* bottom/up expansion */
    for (x = width-1; (x > 0) && (Abort == B_FALSE); x--) {
      /* loop through columns within image convex boundary */
      for (y = BuBuf[x], IsUnMarked = B_FALSE; y >= TdBuf[x];) {
	/* skip over unmarked pixels */
	for (; (y >= TdBuf[x]) && (outpixels[y*width+x] == 0.0); y--)
	  if (MaskPixels[y*width+x] != 0)
	    IsUnMarked = B_TRUE;
	/* find next unmarked pixel */
	for (; (y >= TdBuf[x]) && (outpixels[y*width+x] != 0.0); y--);
	/* mark the pixel if its inside the boundary */
	if ((MaskPixels[y*width+x] != 0.0) && (y >= TdBuf[x])) {
	  outpixels[y*width+x] = outpixels[(y+1)*width+x];
	  y--;
	}
      }
      /* is this row is entirely filled, set boundaries accordingly */
      if (IsUnMarked == B_FALSE) {
	TdBuf[x] = height-1;
	BuBuf[x] = 0;
      }
      else
	go = B_TRUE;
    }

    /* left/right expansion */
    for (y = 0; (y < height) && (Abort == B_FALSE); y++) {
      /* loop through columns within image convex boundary */
      for (x = LrBuf[y], IsUnMarked = B_FALSE; x <= RlBuf[y];) {
	/* skip over unmarked pixels */
	for (; (x <= RlBuf[y]) && (outpixels[y*width+x] == 0.0); x++)
	  if (MaskPixels[y*width+x] != 0.0)
	    IsUnMarked = B_TRUE;
	/* find next unmarked pixel */
	for (; (x <= RlBuf[y]) && (outpixels[y*width+x] != 0.0); x++);
	/* mark the pixel if its inside the boundary */
	if ((MaskPixels[y*width+x] != 0.0) && (x <= RlBuf[y])) {
	  outpixels[y*width+x] = outpixels[y*width+x-1];
	  x++;
	}
      }
      /* is this row is entirely filled, set boundaries accordingly */
      if (IsUnMarked == B_FALSE) {
	LrBuf[y] = width-1;
	RlBuf[y] = 0;
      }
      else
	go = B_TRUE;
    }

    /* right/left expansion */
    for (y = height-1; (y > 0) && (Abort == B_FALSE); y--) {
      /* loop through columns within image convex boundary */
      for (x = RlBuf[y], IsUnMarked = B_FALSE; x >= LrBuf[y];) {
	/* skip over unmarked pixels */
	for (;(x >= LrBuf[y]) && (outpixels[y*width+x] == 0.0); x--)
	  if (MaskPixels[y*width+x] != 0.0)
	    IsUnMarked = B_TRUE;
	/* find next unmarked pixel */
	for (; (x >= LrBuf[y]) && (outpixels[y*width+x] != 0.0); x--);
	/* mark the pixel if its inside the boundary */
	if ((MaskPixels[y*width+x] != 0.0) && (x >= LrBuf[y])) {
	  outpixels[y*width+x] = outpixels[y*width+x+1];
	  IsUnMarked = B_TRUE;
	  x--;
	}
      }
      /* is this row is entirely filled, set boundaries accordingly */
      if (IsUnMarked == B_FALSE) {
	LrBuf[y] = width-1;
	RlBuf[y] = 0;
      }
      else
	go = B_TRUE;
    }

    /* increment loop counter */  
    k++;

    /* update image at each iteration if required */
    if ((IPOpExecVerbose) && (1)) {
      ShowImage(outpixels);
      /* tabulate updated pixels at this pass */
      for (x = 0, ncols = 0; x < width; x++)
	if (BuBuf[x] == 0)
	  ncols++;
      for (y = 0, nrows = 0; y < height; y++)
	if (RlBuf[y] == 0)
	  nrows++;
      sprintf(cbuf, "Region expansion - pass %d (%d rows, %d columns filled)",
	      k, nrows, ncols);
      draw_mesg(cbuf);
      printf("%s\n", cbuf);
      /* print out cols and rows updated if less than 10 */
      if ((nrows > height-10) && (ncols > width-10)) {
	for (x = 0, printf("Columns still changing : "); x < width; x++)
	  if (BuBuf[x] != 0)
	    printf("%d ", x);
	for (y = 0, printf("\nRows still changing    : "); y < height; y++)
	  if (RlBuf[y] != 0)
	    printf("%d ", y);
	printf("\n");
      }
    }
  } while ((go) && (Abort == B_FALSE) && (k < MAXITERS));

  /* check for infinite loop */
  if (k == MAXITERS) {
    Abort = B_TRUE;
    draw_mesg("Abort : Max iterations exceeded in RegionExpansion()");
  }

  /* fill in boundary pixels if required */
  if (DisplayContourComputed == B_FALSE) {
    for (i = 1; i < width-1; i++) {
      outpixels[i] = outpixels[width + i];
      outpixels[(height-1)*width + i] = outpixels[(height-2)*width + i];
    }
    for (i = 0; i < height; i++) {
      outpixels[i*width + 0] = outpixels[i*width + 1];
      outpixels[i*width + width-1] = outpixels[i*width + width-2];
    }
  }

  /* free storage */
  XvgMM_Free(TdBuf);
  XvgMM_Free(BuBuf);
  XvgMM_Free(LrBuf);
  XvgMM_Free(RlBuf);
}


/************************ RegionFill calling interface *********************/
/*                                                                         */
/*  function: seed fill region and return pixel count                      */
/*            generate list of contour points                              */
/*                                                                         */
/***************************************************************************/
void RegionFillIPOp(double *inpixels, double *outpixels, int height,
		    int width, void *ParmPtrs[])
{
  int xs, ys, CentroidX, CentroidY, i;
  double NewVal;

  /* load parameters */
  xs = *((int *) ParmPtrs[0]);
  ys = *((int *) ParmPtrs[1]);
  NewVal = *((double *) ParmPtrs[2]);

  /* transfer data to outpixels buffer */
  for (i = 0; i < height*width; i++)
    outpixels[i] = inpixels[i];

  /* seed feed from the specified starting location */
  RegionFill(outpixels, height, width, ys, xs, outpixels[ys*width + xs],
	     NewVal, NULL, NULL, &CentroidX, &CentroidY);

  /* binarize the image into filled / other pixels */
  for (i = 0; i < height*width; i++)
    outpixels[i] = (outpixels[i] == NewVal ? 0.0 : 1.0);
}


/******************************** RegionFill *******************************/
/*                                                                         */
/*  function: seed fill region and return pixel count                      */
/*            generate list of contour points                              */
/*                                                                         */
/***************************************************************************/
#define FILLSTACKSIZE 1024
int RegionFill(double *pixels, int height, int width, int ys, int xs,
	       double MarkVal, double NewVal, CONTOUR_ELEMENT *contour,
	       int *pcnt, int *CentroidX, int *CentroidY)
{
  struct {
    int x;
    int y;
  } FillStack[FILLSTACKSIZE];
  int i, j, ls, StackTop, xc, yc, UpRun, DownRun, pcount, maxcnt, cnt, LeftB;
  int MaxWidth;
  double *l, *prev, *next;

  /* show information */
/*
  if (VerboseFlag)
    printf("RegionFill() : xs=%d, ys=%d, MarkVal=%.2f, NewVal=%.2f\n",
	   xs, ys, MarkVal, NewVal);
*/

  /* check input parms */
  if ((ys > height-1) || (xs > width-1)) {
    Abort = B_TRUE;
    draw_mesg("Abort : invalid seed point in RegionFill()");
    return(0);
  }

  /* init contour buffer parms if required */
  if (contour != NULL) {
    maxcnt = *pcnt;
    cnt=0;
  }

  /* loop until empty stack */
  for (FillStack[0].x = xs, FillStack[0].y = ys, StackTop = 1, pcount=0,
       MaxWidth=0; StackTop > 0;) {
    /* pop topmost seed point */
    StackTop--;
    xc = FillStack[StackTop].x;
    yc = FillStack[StackTop].y;

    /* find left boundary */
    for (i = xc, l = &(pixels[yc*width]); (i > 0) && (l[i] == MarkVal); i--);
    if (i > 0)
      i++;
    LeftB = i;

    /* save the left boudary point for this line, if required */
    if ((contour != NULL) && (i > 0) && (l[i]==MarkVal)) {
      if (cnt > maxcnt) {
	Abort = B_TRUE;
	draw_mesg("Abort(1) : contour array size exceeded in RegionFill()");
	sprintf(cbuf, "RegionFill(): Array exceeded Max:%d, pcnt:%d, cnt:%d\n",
		maxcnt, pcount, cnt);
	ErrorMsg(cbuf);
	return(pcount);
      }
      contour[cnt].x = i;
      contour[cnt].y = yc;
      contour[cnt].outval = l[i - 1];
      if ((contour[cnt].outval == MarkVal) || (contour[cnt].outval == NewVal)){
	sprintf(cbuf, "Left neighbor same value at ",
		contour[cnt].x, contour[cnt].y);
	draw_mesg(cbuf);
      }
      cnt++;
    }
    
    /* fill to right boudary, pushing upper & lower seed points if any */
    prev = (yc > 0 ? &(pixels[(yc-1)*width]) : NULL);
    next = (yc < (height-1) ? &(pixels[(yc+1)*width]) : NULL);
    for (UpRun = B_FALSE, DownRun=B_FALSE; (i < width) && (l[i] == MarkVal);
	 i++) {
      l[i] = NewVal;
      pcount++;
      if (prev != NULL) {
	/* push point above if unmarked start of a new run */
	if ((UpRun == B_FALSE) && (prev[i] == MarkVal)) {
	  FillStack[StackTop].x = i;
	  FillStack[StackTop].y = yc-1;
	  if (++StackTop >= FILLSTACKSIZE) {
	    Abort = B_TRUE;
	    draw_mesg("Abort(2) : stack size exceeded in RegionFill()");
	    return(pcount);
	  }
	  UpRun = B_TRUE;
	}
	else if ((UpRun == B_TRUE) && (prev[i] != MarkVal))
	  UpRun = B_FALSE;
	/* save the top boudary point for this pixel, if required */
	if (contour != NULL)
	  if ((prev[i] != MarkVal) && (prev[i] != NewVal)) {
	    if (cnt > maxcnt) {
	      Abort = B_TRUE;
	      draw_mesg("Abort(3):contour array size exceeded, RegionFill()");
	      sprintf(cbuf,
		      "RegionFill():Array exceeded Max:%d, pcnt:%d, cnt:%d\n",
		      maxcnt, pcount, cnt);
	      ErrorMsg(cbuf);
	      return(pcount);
	    }
	    contour[cnt].x = i;
	    contour[cnt].y = yc;
	    contour[cnt].outval = prev[i];
	    if ((contour[cnt].outval == MarkVal)
		|| (contour[cnt].outval == NewVal)) {
	      sprintf(cbuf, "Top neighbor same value at (%d,%d)",
		      contour[cnt].x, contour[cnt].y);
	      draw_mesg(cbuf);
	    }
	    cnt++;
	  }
      }
      if (next != NULL) {
	/* push point below if unmarked start of a new run */
	if ((DownRun == B_FALSE) && (next[i] == MarkVal)) {
	  FillStack[StackTop].x = i;
	  FillStack[StackTop].y = yc+1;
	  if (++StackTop >= FILLSTACKSIZE) {
	    Abort = B_TRUE;
	    draw_mesg("Abort(4) : stack size exceeded in RegionFill()");
	    return(pcount);
	  }
	  DownRun = B_TRUE;
	}
	else if ((DownRun == B_TRUE) && (next[i] != MarkVal))
	  DownRun = B_FALSE;
	/* save the bottom boudary point for this pixel, if required */
	if (contour != NULL)
	  if ((next[i] != MarkVal) && (next[i] != NewVal)) {
	    if (cnt > maxcnt) {
	      Abort = B_TRUE;
	      draw_mesg("Abort(5) : contour array size exceeded in RegionFill()");
	      sprintf(cbuf, "RegionFill() : Array size exceeded Max:%d, pcnt:%d, cnt:%d\n", maxcnt, pcount, cnt);
	      ErrorMsg(cbuf);
	      return(pcount);
	    }
	    contour[cnt].x = i;
	    contour[cnt].y = yc;
	    contour[cnt].outval = next[i];
	    if ((contour[cnt].outval == MarkVal)
		|| (contour[cnt].outval == NewVal)) {
	      sprintf(cbuf, "Bottom neighbor same value at (%d,%d)",
		      contour[cnt].x, contour[cnt].y);
	      draw_mesg(cbuf);
	    }
	    cnt++;
	  }
      }
    }
    /* compare this line width to the max, and replace if its bigger */
    if ((i - LeftB) > MaxWidth) {
      MaxWidth = i - LeftB;
      *CentroidX = (i + LeftB)/2;
      *CentroidY = yc;
    }
    
    /* save the right boudary point for this line, if required */
    if ((contour != NULL) && (i < width) && (i != LeftB)) {
      if (cnt > maxcnt) {
	Abort = B_TRUE;
	draw_mesg("Abort(6) : contour array size exceeded in RegionFill()");
	sprintf(cbuf, "RegionFill() : Array size exceeded Max:%d, pcnt:%d, cnt:%d\n", maxcnt, pcount, cnt);
	ErrorMsg(cbuf);
	return(pcount);
      }
      contour[cnt].x = i-1;
      contour[cnt].y = yc;
      contour[cnt].outval = l[i];
      if ((contour[cnt].outval == MarkVal) || (contour[cnt].outval == NewVal)){
	sprintf(cbuf, "Right neighbor same value at (%d,%d)",
		contour[cnt].x, contour[cnt].y);
	draw_mesg(cbuf);
      }
      cnt++;
    }
  }

  /* find the Y component of the "centroid" of the region */
  if ((*CentroidY >= 0) && (*CentroidY < height)
      && (*CentroidX >= 0) && (*CentroidX < width)) {
    for (i = *CentroidY;
	 (i > 0) && (pixels[i*width + *CentroidX] == NewVal); i--);
    for (j = *CentroidY;
	 (j < height) && (pixels[j*width + *CentroidX] == NewVal); j++);
    *CentroidY = (i+j)/2;
  }
  
  /* return parameters */
  if (pcnt != NULL)
    *pcnt = cnt;
  return(pcount);
}

/************************ RegionFillSegmentation ***************************/
/*                                                                         */
/*  function: seed fill disconnected regions with distinct values          */
/*            and grow regions within the contour until they touch and     */
/*            all available space is filled.                               */
/*            If a region has fewer than MinSize pixels, delete it         */
/*                                                                         */
/*            Also, the binarized image of the regions before expansion    */
/*            is pushed on the stack.                                      */
/*                                                                         */
/***************************************************************************/
void RegionFillSegmentation(double *inpixels, double *outpixels, int height,
			    int width, void *ParmPtrs[])
{
  int MaxCnt, ExpandFlag;
  CONTOUR_ELEMENT *contour;
  int e, i, j, k, n, s, cnt, CentroidX, CentroidY;
  int elements, NeighbourCnt, NeighbourElementCnt[MAXNEIGHBOURS], MinSize,
  MaxSize;
  double *parms[1], Threshold, c, neighbours[MAXNEIGHBOURS];

  /* load parameters */
  MinSize = *((int *) ParmPtrs[0]);
  ExpandFlag = *((int *) ParmPtrs[1]);

  /* show banner */
  draw_mesg("Region segmentation...");
  
  /* copy data to outpixels */
  for (i = 0; i < height*width; i++)
    outpixels[i] = inpixels[i];

  /* determine the size of the largest region if required, and set the */
  /* region size threshold to 1/4 of this number                       */
  if (MinSize < 0) {
    for (i = 0, k = 0, c=2.0, MaxSize = 0; (i < height) && (Abort == B_FALSE);
	 i++) {
      for (j = 0; (j < width) && (Abort == B_FALSE); j++, k++)
	if (outpixels[k] == 1.0) {
	  s = RegionFill(outpixels, height, width, i, j, 1.0, c++, NULL, NULL,
			 &CentroidX, &CentroidY); 
	  if (s > MaxSize)
	    MaxSize = s;
	}
    }
    MinSize = MaxSize/4;
    
    /* restore the input image buffer */
    for (i = 0; i < height*width; i++)
      outpixels[i] = inpixels[i];
  }
  if (VerboseFlag)
    printf("RegionFillSegmentation() : MinSize = %d\n", MinSize);

  /* seed fill disjointed regions to segment image */
  for (i = 0, k = 0, c=2.0; (i < height) && (Abort == B_FALSE); i++) {
    for (j = 0; (j < width) && (Abort == B_FALSE); j++, k++) {
      if (outpixels[k] == 1.0) {
	s = RegionFill(outpixels, height, width, i, j, 1.0, c, NULL, NULL,
		       &CentroidX, &CentroidY); 
	if (s < MinSize) {
	  RegionFill(outpixels, height, width, i, j, c, 0.0, NULL, NULL,
		     &CentroidX, &CentroidY);
	}
	else {
	  c++;
	}
      }
    }
  }
  if (VerboseFlag)
    printf("RegionFillSegmentation() : Seed filled disjointed regions...\n");
  
  /* Expand regions till they touch, if required */
  if (ExpandFlag) {
    if (VerboseFlag)
      printf("RegionFillSegmentation() : Expanding regions...\n");
    /* contour allocate buffer */
    MaxCnt = 4*(height+width);
    if ((contour = (CONTOUR_ELEMENT *)
	 XvgMM_Alloc((void *) NULL, MaxCnt*sizeof(CONTOUR_ELEMENT))) == NULL) {
      Abort = B_TRUE;
      draw_mesg("Abort : Can't alloc for contour in RegionFillSegmentation()");
      return;
    }

    RegionExpansion(outpixels, outpixels, height, width, NULL);
    if (Abort) {
      XvgMM_Free(contour);
      return;
    }
    
    /* build regions list with contour coordinates */
    draw_mesg("Building region data structure");
    for (i = 1, NRegions=0; (i < height) && (Abort == B_FALSE); i++)
      for (j = 1; (j < width) && (Abort == B_FALSE); j++)
	if (outpixels[i*width + j] > 0) {
	  /* seed fill region to get data */
	  cnt = MaxCnt;
	  n = RegionFill(outpixels, height, width, i, j, outpixels[i*width+j],
			 -outpixels[i*width+j], contour, &cnt,
			 &CentroidX, &CentroidY);
	  if (IPOpExecVerbose)
	    ShowImage(outpixels);
	  /* add region contour list */
	  RegionList[NRegions].index = fabs(outpixels[i*width + j]);
	  RegionList[NRegions].NPixels = n;
	  RegionList[NRegions].CentroidX = CentroidX;
	  RegionList[NRegions].CentroidY = CentroidY;
	  RegionList[NRegions].OffsetSet = B_FALSE;
	  
	  /* tabulate neighbours */
	  for (elements = 0, NeighbourCnt = 0; elements < cnt; elements++) {
	    /* scan existing tabulated neighbours for index match */
	    for (n = 0; (n < NeighbourCnt)
		 && (fabs(contour[elements].outval) != neighbours[n]); n++);
	    /* if exhausted list, add this neighbour to the list */
	    if (n == NeighbourCnt)
	      neighbours[NeighbourCnt++] = fabs(contour[elements].outval);
	  }
	  RegionList[NRegions].NNeighbours = NeighbourCnt;
	  
	  /* load list of neighbour indicies */
	  XvgMM_Free(RegionList[NRegions].Neighbours);
	  if ((RegionList[NRegions].Neighbours = (double *)
	       XvgMM_Alloc((void *) NULL, NeighbourCnt * sizeof(double)))
	      == NULL) {
	    Abort = B_TRUE;
	    draw_mesg("Abort : Can't alloc RegionFillSegmentation()");
	    XvgMM_Free(contour);
	    return;
	  }
	  for (n = 0; n < NeighbourCnt; n++)
	    RegionList[NRegions].Neighbours[n] = neighbours[n];
	  
	  /* allocate storage for the indexing neighbour info */
	  XvgMM_Free(RegionList[NRegions].NeighbourIndicies);
	  if ((RegionList[NRegions].NeighbourIndicies = (int *)
	       XvgMM_Alloc((void *) NULL,
			   NeighbourCnt * sizeof(int))) == NULL) {
	    Abort = B_TRUE;
	    draw_mesg("Abort : Can't alloc RegionFillSegmentation()");
	    XvgMM_Free(contour);
	    return;
	  }
	  
	  /* trace contour */
	  cnt = RegionTraceContour(outpixels, DoubleBuffer, height, width,
				   j-1, i-1,
				   outpixels[i*width + j], contour, MaxCnt);
	  
	  /* print info if required */
	  if (VerboseFlag) {
	    printf("RegionFillSegmentation() : Region %3.0f (%d pixels, %d contour elms) has neighbours :",
		   RegionList[NRegions].index,
		   RegionList[NRegions].NPixels,  cnt);
	    for (n = 0; n < RegionList[NRegions].NNeighbours; n++)
	      printf(" %3.0f", RegionList[NRegions].Neighbours[n]);
	    printf("\n");
	  }
	  
	  /* store the contour elements */
	  RegionList[NRegions].NContourElements = cnt;
	  XvgMM_Free(RegionList[NRegions].ContourElements);
	  if ((RegionList[NRegions].ContourElements = (CONTOUR_ELEMENT *)
	       XvgMM_Alloc((void *) NULL,
			   cnt * sizeof(CONTOUR_ELEMENT))) == NULL) {
	    Abort = B_TRUE;
	    draw_mesg("Abort : Can't alloc RegionFillSegmentation()");
	    XvgMM_Free(contour);
	    return;
	  }
	  for (elements = 0; elements < cnt; elements++)
	    RegionList[NRegions].ContourElements[elements] = contour[elements];
	  
	  /* check for max array size */
	  if (NRegions++ >= REGIONLISTSIZE) {
	    printf("\aRegion list max size exceeded!\n");
	    XvgMM_Free(contour);
	    return;
	  }
	}
    
    /* complete the neighbour info by filling in RegionList indicies that */
    /* match region values                                                */
    for (i = 0; i < NRegions; i++) {
      /* scan all neighbours of this region */
      for (j = 0; j < RegionList[i].NNeighbours; j++)
	/* look for the index of this neighbour if its not the background */
	if (RegionList[i].Neighbours[j] != 0) {
	  for (k = 0; RegionList[k].index != RegionList[i].Neighbours[j]; k++);
	  /* store it in the structure */
	  RegionList[i].NeighbourIndicies[j] = k;
	}
	else
	  RegionList[i].NeighbourIndicies[j] = -1;
    }

    /* convert data to positive values */
    for (i = 0; i < width*height; i++)
      outpixels[i] = -outpixels[i];

    /* free storage */
    XvgMM_Free(contour);

    /* add region label display to overlay function array */
    LoadOverlayFunction(DrawRegionLabels);
  }
  
  /* else, just renumber the regions starting from 0 */
  else {
    for (i = 0; i < width*height; i++) {
      if (outpixels[i])
	outpixels[i] -= 2;
      else
	outpixels[i] -= 1;
    }
  }

  /* check memory */
  XvgMM_MemoryCheck("RegionFillSegmentation()");
}

/************************** RegionBoundaryNormals **************************/
/*                                                                         */
/*  function: compute normals at region boudaries                          */
/*                                                                         */
/***************************************************************************/
void RegionBoundaryNormals(double *inpixels, double *outpixels, int height,
			   int width, void *ParmPtrs[])
{
  NORMAL_ELEMENT *normals;
  int x, y, x0, x1, y0, y1, i, j, k, n, m, windowsize, samplingperiod;
  int NNormalElementsTmp, NeighbourIndx, MaxNorms;
  double slope, SumX, SumXY, SumY, SumX2, HalfDx, b, w, offset, NeighboursTmp;
  boolean_t Swap;
  
  /* load parameters */
  samplingperiod = *((int *) ParmPtrs[0]);
  windowsize = *((int *) ParmPtrs[1]);
  w = (windowsize/2)*2 + 1;
  
  /* allocate storage if required */
  MaxNorms = 4*(height+width);
  if ((normals = (NORMAL_ELEMENT *)
       XvgMM_Alloc((void *) NULL,
		   MaxNorms*sizeof(NORMAL_ELEMENT))) == NULL) {
    Abort = B_TRUE;
    draw_mesg("Abort : Can't alloc for normals in RegionBoundaryNormals()");
    return;
  }

  /* loop for all regions */
  for (i = 0; i < NRegions; i++) {

    /* Null contour element counts */
    XvgMM_Free(RegionList[i].NNormalElements);
    if ((RegionList[i].NNormalElements =
	 (int *) XvgMM_Alloc((void *) NULL,
			     RegionList[i].NNeighbours * sizeof(int)))
	== NULL) {
      Abort = B_TRUE;
      draw_mesg("Abort : Can't alloc in RegionBoundaryNormals()");
      XvgMM_Free(normals);
      return;
    }
    for (j = 0; j < RegionList[i].NNeighbours; j++)
      RegionList[i].NNormalElements[j] = 0;

    /* loop for all contour elements */
    for (k = windowsize/2, n = 0;
	 k < RegionList[i].NContourElements - windowsize/2; k++) {
      
      /* look at contour elements at "samplingperiod" intervals */ 
      if ((k/samplingperiod)*samplingperiod == k) {
	x = RegionList[i].ContourElements[k].x;
	y = RegionList[i].ContourElements[k].y;
	
	/* compute tangent line slope using least squares line fit */
	for (SumX=0, SumY=0, SumXY=0, SumX2=0, j = k - windowsize/2;
	     j <= k + windowsize/2; j++) {
	  SumX += RegionList[i].ContourElements[j].x;
	  SumX2 += (RegionList[i].ContourElements[j].x
		    * RegionList[i].ContourElements[j].x);
	  SumY += RegionList[i].ContourElements[j].y;
	  SumXY += (RegionList[i].ContourElements[j].x
		    * RegionList[i].ContourElements[j].y);
	}
	slope = (SumX*SumY - w*SumXY) / (SumX*SumX - w*SumX2);
	
	/* check for arithmetic overflow, if so use horizontal line */
	if (isnan(slope)) {
	  x0 = x + windowsize/2;
	  x1 = x - windowsize/2;
	  y0 = y;
	  y1 = y;
	}
	/* check for arithmetic underflow, if so use vertical line */
	else if (fabs(slope) < 0.01) {
	  x0 = x;
	  x1 = x;
	  y0 = y + windowsize/2;
	  y1 = y - windowsize/2;
	}
	/* regular case */
	else {
	  slope = -1.0 / slope;
	  b = ((double) y) - slope*((double) x);
	  HalfDx = ((double) windowsize) / sqrt(slope*slope + 1.0) / 2.0;
	  x0 = ((double) x) - HalfDx;
	  x1 = ((double) x) + HalfDx;
	  y0 = slope*(((double) x) - HalfDx) + b;
	  y1 = slope*(((double) x) + HalfDx) + b;
	}

	/* load normal contour intersection point */
	normals[n].x = x;
	normals[n].y = y;
	/* adjust order of endpoints so that the line points outwards */
	if (inpixels[y0*width + x0] == RegionList[i].index) {
	  normals[n].LineX0 = x0;
	  normals[n].LineY0 = y0;
	  normals[n].LineX1 = x1;
	  normals[n].LineY1 = y1;
	  normals[n].outval = inpixels[y1*width + x1];
	}
	else if (inpixels[y1*width + x1] == RegionList[i].index) {
	  normals[n].LineX0 = x1;
	  normals[n].LineY0 = y1;
	  normals[n].LineX1 = x0;
	  normals[n].LineY1 = y0;
	  normals[n].outval = inpixels[y0*width + x0];
	}

	/* scan the list of this region's neighbours to make sure that   */
	/* the normal outer endpoint is in one of them. If so, inc count */
	for (j = 0; (j < RegionList[i].NNeighbours)
	     && (normals[n].outval != RegionList[i].Neighbours[j]); j++);
	if ((j != RegionList[i].NNeighbours) && (normals[n].outval != 0)
	    && (inpixels[(normals[n].LineY0)*width + normals[n].LineX0]
		== RegionList[i].index)) {
	  RegionList[i].NNormalElements[j] += 1;
	  n++;
	}
	if (n >= MaxNorms) {
	  Abort = B_TRUE;
	  draw_mesg("Abort : Internal overflow in RegionBoundaryNormals()");
	  XvgMM_Free(normals);
	  return;
	}
      }
    }

    /* reorder the list of neighbours in order of decreasing NNormalElements */
    do {
      for (j = 0, Swap = B_FALSE; j < (RegionList[i].NNeighbours - 1); j++)
	if ((RegionList[i].NNormalElements[j] <
	     RegionList[i].NNormalElements[j+1])
	    || (RegionList[i].Neighbours[j] == 0)) {
	  /* swap Neighbours */
	  NeighboursTmp = RegionList[i].Neighbours[j+1];
	  RegionList[i].Neighbours[j+1] = RegionList[i].Neighbours[j];
	  RegionList[i].Neighbours[j] = NeighboursTmp;
	  /* swap NeighbourIndicies */
	  NeighbourIndx = RegionList[i].NeighbourIndicies[j+1];
	  RegionList[i].NeighbourIndicies[j+1] =
	    RegionList[i].NeighbourIndicies[j];
	  RegionList[i].NeighbourIndicies[j] = NeighbourIndx;
	  /* swap NNormalElements */
	  NNormalElementsTmp = RegionList[i].NNormalElements[j+1];
	  RegionList[i].NNormalElements[j+1] =RegionList[i].NNormalElements[j];
	  RegionList[i].NNormalElements[j] = NNormalElementsTmp;
	  /* set swap flag */
	  Swap = B_TRUE;
	}
    } while (Swap);

    /* allocate storage for lists of normals */
    XvgMM_Free(RegionList[i].NormalElements);
    if ((RegionList[i].NormalElements = (NORMAL_ELEMENT **)
	 XvgMM_Alloc((void *) NULL,
		     RegionList[i].NNeighbours * sizeof(NORMAL_ELEMENT **)))
	== NULL) {
      Abort = B_TRUE;
      draw_mesg("Abort : Can't alloc in RegionBoundaryNormals()");
      XvgMM_Free(normals);
      return;
    }
    for (j = 0; j < RegionList[i].NNeighbours; j++)
      if (RegionList[i].NNormalElements[j] > 0) {
	if ((RegionList[i].NormalElements[j] = (NORMAL_ELEMENT *) 
	     XvgMM_Alloc((void *) NULL,
			 RegionList[i].NNormalElements[j]
			 * sizeof(NORMAL_ELEMENT))) == NULL) {
	  Abort = B_TRUE;
	  draw_mesg("Abort : Can't alloc in RegionBoundaryNormals()");
	  XvgMM_Free(normals);
	  return;
	}
      }

    /* transfer normal element data into the lists */
    for (j = 0; j < RegionList[i].NNeighbours; j++) {
      for (k = 0, m = 0; (k < RegionList[i].NNormalElements[j]) && (m<n);m++)
	if (normals[m].outval == RegionList[i].Neighbours[j])
	  RegionList[i].NormalElements[j][k++] = normals[m];
      /* consistency check */
      if ((m == n) && (k !=  RegionList[i].NNormalElements[j])) {
	Abort = B_TRUE;
	draw_mesg("Abort : Internal error in RegionBoundaryNormals()");
	XvgMM_Free(normals);
	return;
      }
    }

    /* print info if required */
    if (VerboseFlag) {
      printf("RegionBoundaryNormals() : Region %.0f has neighbours :",
	     RegionList[i].index);
      for (j = 0; j < RegionList[i].NNeighbours; j++)
	printf(" %.0f (indx:%d, %d normals)", RegionList[i].Neighbours[j],
	       RegionList[i].NeighbourIndicies[j],
	       RegionList[i].NNormalElements[j]);
      printf("\n");
    }
  }
    
  /* copy the data to the outpixels buffer */
  for (i = 0; i < height*width; i++)
    outpixels[i] = inpixels[i];

  /* add region label display to overlay function array */
  LoadOverlayFunction(DrawRegionNormals);

  /* free storage */
  XvgMM_Free(normals);
}


/****************************** Residues ***********************************/
/*                                                                         */
/*  function: Computation of phase residues (preliminary step for          */
/*            phase unwrapping                                             */
/*                                                                         */
/***************************************************************************/
void Residues(double *inpixels, double *outpixels, int height, int width,
	      void *ParmPtrs[])
{
  int i, j;
  double p1, p2, p3, p4, OneOverTwoPi, res, max, min;

  /* zero residue array */
  draw_mesg("Residues - Initializing mask");
  for (i = 0; i < (height*width); i++)
    residues[i] = 0.0;

  /* allocate storage if required */
  if ((residues = (double *) UxMalloc(ImgHeight*ImgWidth*sizeof(double)))
      == NULL) {
    Abort = B_TRUE;
    draw_mesg("Abort : can't allocate in Residues()");
    return;
  }

  /* compute residue mask */
  draw_mesg("Residues - Computing mask");
  DynamicRange(inpixels, height*width, &max, &min);
  OneOverTwoPi = 1.0/(max - min);
  if (max == min) {
    Abort = B_TRUE;
    draw_mesg("Abort : Max = min in Residues()");
    UxFree(residues);
    return;
  }
  for (i = 0; i < (height-1)*width; i+=width)
    for (j = 0; j < (width-1); j++) {
      /* compure residue in a square path */
      p1 = inpixels[i + j];
      p2 = inpixels[i + j + 1];
      p3 = inpixels[i+width + j + 1];
      p4 = inpixels[i+width + j];
      res = rint((p2-p1)*OneOverTwoPi) + rint((p3-p2)*OneOverTwoPi)
	+ rint((p4-p3)*OneOverTwoPi) + rint((p1-p4)*OneOverTwoPi);
      
      /* mark this box if the residue is non-zero */
      if (res > 0) {
	residues[i + j] = 1;
	residues[i + j + 1] = 1;
	residues[i+width + j + 1] = 1;
	residues[i+width + j] = 1;
      }
      else if (res < 0) {
	residues[i + j] = -1;
	residues[i + j + 1] = -1;
	residues[i+width + j + 1] = -1;
	residues[i+width + j] = -1;
      }
    }

  /* overlay residue mask onto the image */
  draw_mesg("Residues - Overlaying mask");
  for (i = 0; i < (height*width); i++) {
    if (residues[i] != 0.0)
      outpixels[i] = max;
    else
      outpixels[i] = inpixels[i];
  }

  /* free storage */
  UxFree(residues);
}


/****************************** Unwrap *************************************/
/*                                                                         */
/*  function: compute relative offsets between regions and unwrap          */
/*            using template in current image and                          */
/*            and first wrapped data on the stack to compute relative      */
/*            offsets (filtered image) and second image of raw data.       */
/*                                                                         */
/*  Parameters:                                                            */
/*            DRThreshold : fraction of dynamic range used as threshold    */
/*                          for determining the relative offsets between   */
/*                          regions, depending on the average offset       */
/*                          computed over the normals common to two regions*/
/*                                                                         */
/*  Note that the filtered data is unwrapped (not the raw) because least   */
/*  squares is later used to fit a model to the data. This process would   */
/*  be unduly affected by the spike-type noise of the raw speckle data.    */
/*                                                                         */
/***************************************************************************/
#define STACKMAX 100
void Unwrap(double *inpixels, double *outpixels, int height,
	    int width, void *ParmPtrs[])
{
  int i, j, k, x, y, nmax, StackTop, Stack[STACKMAX], region, neighbour, indx;
  int MaxNormals;
  double offset, MinOffset, pix, DynRange, ThresholdScale, index;
  boolean_t Done;

  /* load parms */
  ThresholdScale = *((double *) ParmPtrs[0]);

  /* check that twp images are on the stack, then decrement stack pointer  */
  if (ImageStackTop < 2) {
    Abort = B_TRUE;
    draw_mesg("Abort : Two images required on the stack for Unwrap()");
    return;
  }
  ImageStackTop--;
  UpdateStackLabel(ImageStackTop);

  /* check that the image on the stack has data in the range -PI,PI */
  for (i = 0; (i < height*width) && (Abort == B_FALSE); i++) {
    if ((ImageStack[ImageStackTop].pixels[i] < -PI)
	|| (ImageStack[ImageStackTop].pixels[i] > PI))
      Abort = B_TRUE;
  }
  if (Abort) {
    ImageStackTop++;
    UpdateStackLabel(ImageStackTop);
    sprintf(cbuf, "Abort : First stack image data (%f) dynamic range exceeds [-pi,pi] in Unwrap()", ImageStack[ImageStackTop-1].pixels[i-1]);
    draw_mesg(cbuf);
    return;
  }
    
  /* check that height and width match the current image size */
  if   ((ImageStack[ImageStackTop].height != height)
	|| (ImageStack[ImageStackTop].width != width)) {
    Abort = B_TRUE;
    draw_mesg("Abort : Image on stack is not the current size in Unwrap()");
    return;
  }
  
  /* compute the offsets at every normal between regions */
  draw_mesg("Unwrap : computing relative offsets between regions...");
  for (i = 0; i < NRegions; i++)
    /* loop for the neighbours of each region */
    for (j = 0; j < RegionList[i].NNeighbours; j++) {
      /* loop for the normals for each neighbours */
      for (k = 0; (k < RegionList[i].NNormalElements[j])
	   && (RegionList[i].Neighbours[j] != 0); k++) {
	
	/* compute average value in 3x3 neighborhood inside region */
	x =  RegionList[i].NormalElements[j][k].LineX0;
	y =  RegionList[i].NormalElements[j][k].LineY0;
	RegionList[i].NormalElements[j][k].offset
	  = (ImageStack[ImageStackTop].pixels[(y-1)*width + x+1]
	     + ImageStack[ImageStackTop].pixels[(y-1)*width + x]
	     + ImageStack[ImageStackTop].pixels[(y-1)*width + x-1]
	     + ImageStack[ImageStackTop].pixels[y*width + x+1]
	     + ImageStack[ImageStackTop].pixels[y*width + x]
	     + ImageStack[ImageStackTop].pixels[y*width + x-1]
	     + ImageStack[ImageStackTop].pixels[(y+1)*width + x-1]
	     + ImageStack[ImageStackTop].pixels[(y+1)*width + x]
	     + ImageStack[ImageStackTop].pixels[(y+1)*width + x+1]);
	
	/* compute average value in 3x3 neighborhood outside region */
	x =  RegionList[i].NormalElements[j][k].LineX1;
	y =  RegionList[i].NormalElements[j][k].LineY1;
	RegionList[i].NormalElements[j][k].offset
	  -= (ImageStack[ImageStackTop].pixels[(y-1)*width + x+1]
	      + ImageStack[ImageStackTop].pixels[(y-1)*width + x]
	      + ImageStack[ImageStackTop].pixels[(y-1)*width + x-1]
	      + ImageStack[ImageStackTop].pixels[y*width + x+1]
	      + ImageStack[ImageStackTop].pixels[y*width + x]
	      + ImageStack[ImageStackTop].pixels[y*width + x-1]
	      + ImageStack[ImageStackTop].pixels[(y+1)*width + x-1]
	      + ImageStack[ImageStackTop].pixels[(y+1)*width + x]
	      + ImageStack[ImageStackTop].pixels[(y+1)*width + x+1]);
      }
    }
  
  /* get dynamic range within contour */
  DynRange = TWOPI;

  /* find the biggest region */
  for (i = 0, nmax = 0, region = 0; i < NRegions; i++)
    if (RegionList[i].NPixels > nmax) {
      nmax = RegionList[i].NPixels;
      region = i;
    }
  
  /* loop while stack not empty, start off with biggest region */
  draw_mesg("Unwrapping...");
  for (RegionList[region].offset = 0, RegionList[region].OffsetSet = B_TRUE,
       Stack[0] = region, StackTop = 1, Done = B_FALSE, MinOffset = 0;
       Done == B_FALSE;) {
    
    /* scan the stack to find the region which shares the largest    */
    /* number of normals with an unmarked region (except background) */
    for (i = 0, MaxNormals = 0; i < StackTop; i++)
      for (j = 0; j < RegionList[Stack[i]].NNeighbours; j++)
	if (RegionList[Stack[i]].Neighbours[j] != 0)
	  if ((RegionList[RegionList[Stack[i]].NeighbourIndicies[j]].OffsetSet
	       == B_FALSE)
	      && (RegionList[Stack[i]].NNormalElements[j] > MaxNormals)) {
	    MaxNormals = RegionList[Stack[i]].NNormalElements[j];
	    region = Stack[i];
	    neighbour = RegionList[Stack[i]].NeighbourIndicies[j];
	    indx = j;
	  }
    
    /* if unmarked neighbour found, exit the loop */
    if (MaxNormals == 0)
      Done = B_TRUE;
      
    /* else calculate offset of the neighbour and push it on the stack   */
    /* if its not already there                                          */
    else {
      /* scan the stack to see if the neighbour is there, push if it ain't */
      for (i = 0; (i < StackTop) && (neighbour != Stack[i]); i++);
      if (i == StackTop)
	Stack[StackTop++] = neighbour;
	
      /* compute the relative offset using this region's list if it exists */
      if (RegionList[region].NNormalElements[indx] > 0) {
	for (i=0, offset=0; i < RegionList[region].NNormalElements[indx]; i++)
	  offset += RegionList[region].NormalElements[indx][i].offset;
	offset /= (((double) RegionList[region].NNormalElements[indx]) * 9.0);
      }
      /* else compute offset using list of normals in the neighbour */
      else {
	/* search neighbour normal list to find matching required list */
	for (j = 0; (j < RegionList[neighbour].NNeighbours) &&
	     (RegionList[region].index != RegionList[neighbour].Neighbours[j]);
	     j++);
	if (j == RegionList[neighbour].NNeighbours) {
	  Abort = B_TRUE;
	  draw_mesg("Abort : Internal error (No matching list) in Unwrap()");
	  return;
	}
	if (RegionList[neighbour].NNormalElements[j] == 0) {
	  Abort = B_TRUE;
	  draw_mesg("Abort : Internal error (No Normals) in Unwrap()");
	  return;
	}
	for (i=0, offset=0; i < RegionList[neighbour].NNormalElements[j]; i++)
	  offset -= RegionList[neighbour].NormalElements[j][i].offset;
	offset /= (((double) RegionList[neighbour].NNormalElements[j]) * 9.0);
      }
      
      /* compute and store the absolute offset */
      if (isnan(offset))
	RegionList[neighbour].offset = RegionList[region].offset;
      else if (offset < - (DynRange*ThresholdScale))
	RegionList[neighbour].offset = RegionList[region].offset - DynRange;
      else if (offset > (DynRange*ThresholdScale))
	RegionList[neighbour].offset = RegionList[region].offset + DynRange;
      else
	RegionList[neighbour].offset = RegionList[region].offset;
      RegionList[neighbour].OffsetSet = B_TRUE;

      /* find the smallest offset */
      MinOffset = (RegionList[neighbour].offset < MinOffset ?
		   RegionList[neighbour].offset : MinOffset);
      
      /* print out info if required */
      if (VerboseFlag)
	printf("UnWrap() : Region %2.0f has RelOffset %6.1f (computed over %d normals) w/r to region %2.0f (GlobOffset:%6.1f) => GlobOffset set to %6.1f\n",
	       RegionList[neighbour].index, offset, i,RegionList[region].index,
	       RegionList[region].offset, RegionList[neighbour].offset);
    }
  }    
  
  /* pop raw data image from the stack (second buffer) */
  ImageStackTop--;
  
  /* check that the image on the stack has data in the range -PI,PI */
  for (i = 0; (i < height*width) && (Abort == B_FALSE); i++) {
    if ((ImageStack[ImageStackTop].pixels[i] < -PI)
	|| (ImageStack[ImageStackTop].pixels[i] > PI))
      Abort = B_TRUE;
  }
  if (Abort) {
    ImageStackTop++;
    UpdateStackLabel(ImageStackTop);
    sprintf(cbuf, "Abort : Second stack iamge data (%f) dynamic range exceeds [-pi,pi] in Unwrap()", ImageStack[ImageStackTop-1].pixels[i-1]);
    draw_mesg(cbuf);
    return;
  }
    
  /* check that height and width match the current image size */
  if   ((ImageStack[ImageStackTop].height != height)
	|| (ImageStack[ImageStackTop].width != width)) {
    Abort = B_TRUE;
    draw_mesg("Abort : Image on stack is not the current size in Unwrap()");
    return;
  }
  
  /* transfer data, adding offsets inside contour */
  for (i = 0, index = MINDOUBLE; i < height*width; i++)
    if (MaskPixels[i] != 0) {
      if (inpixels[i] != index) {
	for (j = 0; (j < NRegions)&&(RegionList[j].index != inpixels[i]);j++);
	if (j == NRegions) {
	  Abort = B_TRUE;
	  sprintf(cbuf, "Abort : Invalid index (%f) at pixel %d in Unwrap()",
		 inpixels[i], i);
	  draw_mesg(cbuf);
	  return;
	}
	offset = RegionList[j].offset;
	index = RegionList[j].index;
      }
      outpixels[i] = offset + ImageStack[ImageStackTop].pixels[i];
      ImageStack[ImageStackTop].pixels[i] = offset;
    }

  /* set contour outside to zero - and offset the inside from 0 */
/*
  for (i = 0; i < height*width; i++)
    if (MaskPixels[i] != 0) {
      outpixels[i] -= (MinOffset - DynRange);
      ImageStack[ImageStackTop].pixels[i] -= (MinOffset - DynRange);
    }
    else {
      outpixels[i] = 0;
      ImageStack[ImageStackTop].pixels[i] = 0;
    }
*/
  
  /* reset stack pointer */
  ImageStackTop++;
  UpdateStackLabel(ImageStackTop);
}


/**************************** Unwrap manual ********************************/
/*                                                                         */
/*  function: unwrap data in RawPixels using step data in                  */
/*            ProcessedPixels                                              */
/*                                                                         */
/***************************************************************************/
void UnwrapManual(void)
{
  int i;

  for (i = 0; i < ImgHeight*ImgWidth; i++)
    ProcessedPixels[i] = (DoubleBuffer[i] + ProcessedPixels[i]*TWOPI)
      * MaskPixels[i];
}


/****************************** Wrap ***************************************/
/*                                                                         */
/*  function: modulo operation to wrap the data                            */
/*            from -oo, +oo to the range -PI..PI                           */
/*                                                                         */
/***************************************************************************/
void Wrap(double *inpixels, double *outpixels, int height,
	  int width, void *ParmPtrs[])
{
  int i;

  draw_mesg("Wrapping..");
  for (i = 0; i < height*width; i++)
    outpixels[i] = inpixels[i] - rint(inpixels[i]/TWOPI)*TWOPI;
}

