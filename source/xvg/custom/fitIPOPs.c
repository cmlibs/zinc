/***********************************************************************
*
*  Name:          fitIPOPs.c
*
*  Author:        Paul Charette
*
*  Last Modified: 26 Nov 1994
*
*  Purpose:       Utility routines for image processing (fitting).
*                     BSplines()
*                     BSplinesAuto()
*                     ChebyshevSurfaceFit()
*                     PolynomialFit()
*                     PolynomialSurfaceFit()
*                     PowerSeriesModelEval()
*
***********************************************************************/
#include <stdio.h>
#include "UxXt.h"
#include <fcntl.h>
#include <search.h>
#include <values.h>
#include "XvgGlobals.h"

/*    speckle auto-correlation model fit globals */
static double SACMF_I, SACMF_VAF, *SACMF_indata, *SACMF_outdata;
static int SACMF_xstart,SACMF_ystart,SACMF_width,SACMF_height,SACMF_xextent; 


/******************************  BSPline  Fit     **************************/
/*                                                                         */
/*  function: fit the data with a BSpline surface representation           */
/*            with predifined knot points.                                 */
/*                                                                         */
/*            Only non-zero points are used, and the arctan radius is      */
/*            used as a weight.                                            */
/*                                                                         */
/*  Note : The NAG routines expect the Y coordinate to vary the fastest    */
/*         and the X coordinate to vary the slowest in 2D matricies !!     */
/*         That is why the X and Y parameters are inverted in the NAG      */
/*         calls, hence there is no need to transpose the matricies.       */
/*                                                                         */
/***************************************************************************/
void BSplines(double *inpixels, double *outpixels,
	      int height, int width, void *ParmPtrs[])
{
  static int OldHeight, OldWidth, NC, NWS, NADRES, *ADRES,*POINT,NPOINT,*IWRK;
  static int LIWRK, LWRK, MX, MY;
  static double *X, *Y, *F, *W, *DL, *WS, *WRK, *XD, *YD;
  double wmax, wmin, wdiv, EPS, SIGMA;
  int x, y, i, j, k, S, M, MP, RANK, NInteriorKnots, WeightType, IFAIL;
  int xmax, xmin, ymax, ymin, *XP, *YP, NWRK1, NWRK2;
  boolean_t NonZero;

  /* load pams */
  NInteriorKnots = *((int *) ParmPtrs[0]);
  EPS = MINDOUBLE;
  
  /* tabulate the non-zero points in the image and their radius values */
  /* scale the x,y values to the range [-1,1] and scale the weights to */
  /* the range [0,1]                                                   */
  draw_mesg("BSplines() : Tabulating non-zero points...");
  if ((height != OldHeight) || (width != OldWidth)) {
    X = (double *) UxRealloc(X, height * width * sizeof(double));
    Y = (double *) UxRealloc(Y, height * width * sizeof(double));
    F = (double *) UxRealloc(F, height * width * sizeof(double));
    W = (double *) UxRealloc(W, height * width * sizeof(double));
  }
  for (y = 0, M = 0, xmax = 0, xmin = width, ymax = 0, ymin = height;
       y < height; y++)
    for (x = 0; x < width; x++)
      if (MaskPixels[y*width + x] > 0) {
	X[M] = x;
	Y[M] = y;
	F[M] = inpixels[y*width + x];
	W[M] = 1.0;
	xmax = (x > xmax ? x : xmax);
	xmin = (x < xmin ? x : xmin);
	ymax = (y > ymax ? y : ymax);
	ymin = (y < ymin ? y : ymin);
	M++;
      }
  
  /* alloc storage for spline the evaluation if required */
  if ((BSplineNKi != (NInteriorKnots + 8))
      || (MX != (xmax - xmin + 1)) || (MY != (ymax - ymin + 1))) {
    MX = xmax - xmin + 1;
    MY = ymax - ymin + 1;
    XD = (double *) UxRealloc(XD, MX * sizeof(double));
    YD = (double *) UxRealloc(YD, MY * sizeof(double));
    NWRK1 = 4*MX + (NInteriorKnots + 8);
    NWRK2 = 4*MY + (NInteriorKnots + 8);
    LWRK = (NWRK1 > NWRK2 ? NWRK1 : NWRK2);
    WRK = (double *) UxRealloc(WRK, LWRK * sizeof(double));
    NWRK1 = MX + (NInteriorKnots + 8) - 4;
    NWRK2 = MY + (NInteriorKnots + 8) - 4;
    LIWRK = (NWRK1 > NWRK2 ? NWRK1 : NWRK2);
    IWRK = (int *) UxRealloc(IWRK, LIWRK * sizeof(int));
  }
  
  /* allocate storage if required for the knots & stuff if required */
  if (BSplineNKi != (NInteriorKnots + 8)) {
    BSplineNKi = BSplineNKj = NInteriorKnots + 8;
    BSplineKi = (double *) UxRealloc(BSplineKi, BSplineNKi * sizeof(double));
    BSplineKj = (double *) UxRealloc(BSplineKj, BSplineNKj * sizeof(double));
    BSplineCij = (double *) UxRealloc(BSplineCij,
				      (BSplineNKi-4) * (BSplineNKj-4)
				      * sizeof(double));
    NC = (BSplineNKi-4)*(BSplineNKj-4);
    DL = (double *) UxRealloc(DL, NC * sizeof(double));
    NWS = (2*NC + 1)*(3*BSplineNKj-6) - 2;
    WS = (double *) UxRealloc(WS, NWS * sizeof(double));
    NADRES = (BSplineNKi-7)*(BSplineNKj-7);
    ADRES = (int *) UxRealloc(ADRES, NADRES * sizeof(int));
  }

  /* load interior knot coordinate arrays */
  for (i = 1; i <= NInteriorKnots; i++) {
    BSplineKi[i + 3] = xmin + (i * MX)/(NInteriorKnots+1);
    BSplineKj[i + 3] = ymin + (i * MY)/(NInteriorKnots+1);
  }
  
  /* allocate storage for the POINT array */
  NPOINT = M + (BSplineNKi-7)*(BSplineNKj-7);
  POINT = (int *) UxRealloc(POINT, NPOINT * sizeof(int));

  /* NAG call to provide the indexing information */
  IFAIL = -1;
  draw_mesg("BSpline() : Computing indexing info...");
  e02zaf(&BSplineNKj, &BSplineNKi, BSplineKj, BSplineKi, &M, Y, X,
	 POINT, &NPOINT, ADRES, &NADRES, &IFAIL);
  if (IFAIL != 0) {
    ErrorMsg("BSplines() : IFAIL != 0 on return from e02zaf()");
    return;
  }
  
  /* NAG call to fit bicubic B-Splines */
  IFAIL = -1;
  draw_mesg("BSpline() : Computing fit...");
  e02daf(&M, &BSplineNKj, &BSplineNKi, Y, X, F, W, BSplineKj, BSplineKi,
	 POINT, &NPOINT, DL, BSplineCij, &NC, WS, &NWS, &EPS, &SIGMA, &RANK,
	 &IFAIL);
  if (IFAIL != 0) {
    ErrorMsg("BSplines() : IFAIL != 0 on return from e02daf()");
    return;
  }

  /* load knot overlay coordinates */
  NKnotPoints = BSplineNKi*BSplineNKj;
  KnotPoints = (XPoint *) UxRealloc(KnotPoints, NKnotPoints * sizeof(XPoint));
  for (j = 0, k = 0; j < BSplineNKj; j++)
    for (i = 0; i < BSplineNKi; i++, k++) {
      KnotPoints[k].x = BSplineKi[i];
      KnotPoints[k].y = BSplineKj[j];
    }

  /* initilize the XD, YD arrays to span the spline domain */
  for (i = BSplineKj[0], k = 0; i <= BSplineKj[BSplineNKj-1]; i++, k++)
    YD[k] = i;
  for (i = BSplineKi[0], k = 0; i <= BSplineKi[BSplineNKi-1]; i++, k++)
    XD[k] = i;
  if (VerboseFlag)
    printf("BSplines() : spline domain is x[%f,%f], y[%f,%f], MX:%d, MY:%d\n",
	   XD[0], XD[MX-1], YD[0], YD[MY-1], MX, MY);

  /* NAG call to evaluate the spline at all image points in bspline domain */
  IFAIL = -1;
  draw_mesg("BSpline() : Evaluating fit...");
  e02dff(&MY, &MX, &BSplineNKj, &BSplineNKi, YD, XD, BSplineKj, BSplineKi,
	 BSplineCij, F, WRK, &LWRK, IWRK, &LIWRK, &IFAIL);
  if (IFAIL != 0) {
    ErrorMsg("BSplines() : IFAIL != 0 on return from e02dff()");
    return;
  }

  /* transfer data to output buffer */
  for (i = 0; i < height*width; i++)
    outpixels[i] = 0;
  for (j = 0, k = 0, NonZero = B_FALSE; j < MY; j++)
    for (i = 0; i < MX; i++, k++)
      outpixels[(int) (YD[j]*width + XD[i])] = F[k];

  /* set flags & exit */
  LoadOverlayFunction(DrawKnots);
  OldHeight = height;
  OldWidth = width;
}

/*******************  Automatic  BSPline  Fit     **************************/
/*                                                                         */
/*  function: fit the data with a BSpline surface representation           */
/*            where the knot points are found automatically                */
/*                                                                         */
/*  Note : The NAG routines expect the Y coordinate to vary the fastest    */
/*         and the X coordinate to vary the slowest in 2D matricies !!     */
/*         That is why the X and Y parameters are inverted in the NAG      */
/*         calls, hence there is no need to transpose the matricies.       */
/*                                                                         */
/***************************************************************************/
void BSplinesAuto(double *inpixels, double *outpixels,
		  int height, int width, void *ParmPtrs[])
{
  static int MX, MY, M, NXEST, NYEST, LWRK, LIWRK, *IWRK, IFAIL, RANK, OldType;
  static double *F, *W, *WRK, *X, *Y, *XC, *YC;
  double S, FP, *p, tmp;
  int i, j, k, rc, LWRK1, LWRK2, Type;

  /* load pams */
  S = *((double *) ParmPtrs[0]);
  Type = *((int *) ParmPtrs[1]);

  /* allocate storage if required */
  if ((Type == 0) && ((MY != height) || (MX != width)
	  || (NMaskPixels != M) || (OldType == 1))) {
    M = NMaskPixels;
    MX = width;
    MY = height;
    X = (double *) UxRealloc(X, MX * sizeof(double));
    for (i = 0; i < MX; i++)
      X[i] = i;
    Y = (double *) UxRealloc(Y, MY * sizeof(double));
    for (i = 0; i < MY; i++)
      Y[i] = i;
    XC = (double *) UxRealloc(XC, M * sizeof(double));
    YC = (double *) UxRealloc(YC, M * sizeof(double));
    F = (double *) UxRealloc(F, M * sizeof(double));
    W = (double *) UxRealloc(W, M * sizeof(double));
    /* NXEST = NYEST = 4 + sqrt(((double) M)/2.0); */
    NXEST = NYEST = 50;
    BSplineKi = (double *) UxRealloc(BSplineKi, NXEST * sizeof(double));
    BSplineKj = (double *) UxRealloc(BSplineKj, NYEST * sizeof(double));
    BSplineCij = (double *) UxRealloc(BSplineCij,
				      (NXEST-4) * (NYEST-4) * sizeof(double));
    LWRK1 = (7*NXEST*NXEST + 25*NXEST)*(NXEST+1) + 4*(NXEST+M) + 23*NXEST + 56;
    LWRK2 = 4*NXEST*NXEST*NXEST + 2*NXEST*NXEST + 4*NXEST;
    LWRK = LWRK1 + LWRK2;
    WRK = (double *) UxRealloc(WRK, LWRK * sizeof(double));
    LIWRK = M + 2*NXEST*NXEST;
    IWRK = (int *) UxRealloc(IWRK, LIWRK * sizeof(int));
    BSplineComputed = B_FALSE;
    OldType = 0;
  }
  else if ((Type == 1)
	   && ((MY != height) || (MX != width) || (OldType == 0))) {
    MX = width;
    MY = height;
    X = (double *) UxRealloc(IWRK, MX * sizeof(double));
    for (i = 0; i < MX; i++)
      X[i] = i;
    Y = (double *) UxRealloc(Y, MY * sizeof(double));
    for (i = 0; i < MY; i++)
      Y[i] = i;
    NXEST = MX+4;
    BSplineKi = (double *) UxRealloc(BSplineKi, NXEST * sizeof(double));
    NYEST = MY+4;
    BSplineKj = (double *) UxRealloc(BSplineKj, NYEST * sizeof(double));
    BSplineCij = (double *) UxRealloc(BSplineCij, (NXEST-4) * (NYEST-4) * sizeof(double));
    LWRK = 4*(MX+MY) + 11*(NXEST+NYEST) + MX*NYEST
      + (MX > NYEST ? MX : NYEST) + 54;
    WRK = (double *) UxRealloc(WRK, LWRK * sizeof(double));
    LIWRK = 3 + MX + MY + NXEST + NYEST;
    IWRK = (int *) UxRealloc(IWRK, LIWRK * sizeof(int));
    BSplineComputed = B_FALSE;
    OldType = 1;
  }

  /* NAG call to fit bicubic B-Splines */
  IFAIL = -1;
  draw_mesg("BSpline() : Computing fit...");
  if (Type == 0) {
    /* load scattered data points into the array */
    for (j = 0, k = 0; j < height; j++)
      for (i = 0; i < width; i++)
	if ((MaskPixels[j*width + i] != 0) && (inpixels[j*width + i] != 0)) {
	  XC[k] = i;
	  YC[k] = j;
	  W[k] = 1.0;
	  F[k] = inpixels[j*width + i];
	  k++;
	}
    printf("Scattered data B-spline fit for %d points\n", k);
    e02ddf("Cold Start", &k, YC, XC, F, W, &S, &NYEST, &NXEST, 
	   &BSplineNKj, BSplineKj, &BSplineNKi, BSplineKi, BSplineCij,
	   &FP, &RANK, WRK, &LWRK, IWRK, &LIWRK, &IFAIL);
  }
  else {
    /* equally weighted data on rectangular grid */
    if (BSplineComputed)
      e02dcf("Warm Start", &MY, Y, &MX, X, inpixels, &S, &NYEST, &NXEST, 
	     &BSplineNKj, BSplineKj, &BSplineNKi, BSplineKi, BSplineCij,
	     &FP, WRK, &LWRK, IWRK, &LIWRK, &IFAIL);
    else {
      e02dcf("Cold Start", &MY, Y, &MX, X, inpixels, &S, &NYEST, &NXEST, 
	     &BSplineNKj, BSplineKj, &BSplineNKi, BSplineKi, BSplineCij,
	     &FP, WRK, &LWRK, IWRK, &LIWRK, &IFAIL);
    }
  }
  BSplineComputed = B_TRUE;
  
  /* NAG call to evaluate the spline at the grid points */
  IFAIL = -1;
  draw_mesg("BSpline() : Evaluating fit...");
  e02dff(&MY, &MX, &BSplineNKj, &BSplineNKi, Y, X, BSplineKj, BSplineKi,
	 BSplineCij, outpixels, WRK, &LWRK, IWRK, &LIWRK, &IFAIL);

  /* print out results in message banner */
  sprintf(Msg, " = IFAIL:%d, RANK:%d, FP:%e, NKi:%d, NKj:%d ",
	  IFAIL, RANK, FP, BSplineNKi, BSplineNKj);

  /* add knot points display to overlay function array */
  if (NKnotPoints != BSplineNKi*BSplineNKj) {
    NKnotPoints = BSplineNKi*BSplineNKj;
    KnotPoints = (XPoint *) UxRealloc(KnotPoints,NKnotPoints * sizeof(XPoint));
  }
  for (j = 0, k = 0; j < BSplineNKj; j++)
    for (i = 0; i < BSplineNKi; i++, k++) {
      KnotPoints[k].x = BSplineKi[i];
      KnotPoints[k].y = BSplineKj[j];
    }
  LoadOverlayFunction(DrawKnots);
}

/*********************** Chebyshev Surface Fit *****************************/
/*                                                                         */
/*  function: Fit a surface using Chebyshev polynomials                    */
/*                                                                         */
/***************************************************************************/
void ChebyshevSurfaceFit(double *inpixels, double *outpixels, int height,
			 int width, void *ParmPtrs[])
{
  static int OldHeight, OldWidth, OldOrder, *M, NA, NWORK, INUXP1, INUYP1,K,L;
  static double *X, *Y, *F, *W, *A, *XMIN, *XMAX, *WORK, *FF, YMIN, YMAX;
  int x, y, n, N, MTOT, IFAIL, VAF, order, MFIRST, MLAST, xcnt, index, i;
  double SumZ2=0, SumZZp2, Z;
  boolean_t DataFound;

  /* fetch parameters */
  order = *((int *) ParmPtrs[0]);

  /* make sure contour is defined */
  if (DisplayContourComputed == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort : Undefined contour in ChebyshevSurfaceFit()");
    return;
  }

  /* determine array size and allocate space if required */
  if ((OldHeight != height) || (OldWidth != width) || (OldOrder != order)) {
    draw_mesg("ChebyshevSurfaceFit() : Allocating storage...");
    X = (double *) UxRealloc(X, height*width*sizeof(double));
    F = (double *) UxRealloc(F, height*width*sizeof(double));
    FF = (double *) UxRealloc(FF, width*sizeof(double));
    W = (double *) UxRealloc(W, height*width*sizeof(double));
    Y = (double *) UxRealloc(Y, width*sizeof(double));
    M = (int *) UxRealloc(M, width*sizeof(int));
    XMIN = (double *) UxRealloc(XMIN, width*sizeof(double));
    XMAX = (double *) UxRealloc(XMAX, width*sizeof(double));
    K = order;
    L = order;
    NA = ((L+1) * (K+1));
    A = (double *) UxRealloc(A, NA * sizeof(double));
    NWORK = 2*height*width;
    WORK = (double *) UxRealloc(WORK, NWORK*sizeof(double));
    INUXP1 = 1;
    INUYP1 = 1;
    YMIN = 0;
    YMAX = height-1;
    OldHeight = height;
    OldWidth = width;
    OldOrder = order;
  }
  
  /* load the data arrays */
  draw_mesg("ChebyshevSurfaceFit() : Loading the data arrays...");
  for (y=0, N=0, MTOT=0; y < height; y++) {
    for (x=0, n=0, DataFound=B_FALSE; x < width;
	 x++) {
      if (MaskPixels[y*width +x] != 0) {
	DataFound = B_TRUE;
	X[MTOT] = x;
	W[MTOT] = 1.0;
	F[MTOT++] = inpixels[y*width + x];
	n++;
      }
    }
    if (DataFound) {
      if (K < n) {
	M[N] = n;
	XMIN[N] = 0;
	XMAX[N] = width-1;
	Y[N++] = y;
      }
      else 
	MTOT -= n;
    }
  }
  
  /* check for L order against N */
  if (L >= N) {
    Abort = B_TRUE;
    draw_mesg("Abort : L >= N in ChebyshevSurfaceFit()");
    return;
  }
  
  /* zero the output array */
  for (x = 0; x < height*width; x++)
    outpixels[x] = 0;

  /* compute NAG least squares solution */
  IFAIL = 0;
  draw_mesg("ChebyshevSurfaceFit() : Computing least squares fit...");
  e02caf(M, &N, &K, &L, X, Y, F, W, &MTOT, A, &NA, XMIN, XMAX,
	 X, &INUXP1, X, &INUYP1, WORK, &NWORK, &IFAIL);

  /* if verbose, print out polynomial coefficients */
  if (VerboseFlag) {
    printf("ChebyshevSurfaceFit() (order = %d) Coefficients Aij: ", order);
    for (i = 0; i < NA; i++)
      printf(" %e ", A[i]);
    printf("\n");
  }

  /* compute the VAF for the parametric representation of the data */
  draw_mesg("ChebyshevSurfaceFit() : Computing estimated data and VAF");
  for (y=0, MFIRST = 1, xcnt=0, SumZZp2=0, SumZ2=0; y < N; xcnt+=M[y++]) {
    /* compute the chebyshev fit */
    MLAST =  M[y];
    e02cbf(&MFIRST, &MLAST, &K, &L, &(X[xcnt]), &(XMIN[y]), &(XMAX[y]),
	   &(Y[y]), &YMIN, &YMAX, FF, A, &NA, WORK, &NWORK, &IFAIL);
    /* load the data into the output buffer */
    for (x = 0; x < M[y]; x++) {
      index = Y[y]*width + X[xcnt + x];
      Z = inpixels[index];
      outpixels[index] = FF[x];
      /* compute the VAF sums */
      SumZZp2 += ((Z - FF[x]) * (Z - FF[x]));
      SumZ2 += (Z * Z);
    }
  }
  VAF = 100.0*(1.0 - SumZZp2/SumZ2);

  /* print out VAF */
  sprintf(Msg, " = VAF : %d%%", VAF);
}

/*********************** Newton-Raphson root finding  **********************
*
*  function: find the root of a one-dimensional function
*
*  arguments:
*            func: user supplied routine that evaluates the function
*                  value and first derivative at a point
*                   parm0 : point at which evaluation is required
*                   parm1 : return address of function value
*                   parm2 : return address of function derivative value
*                   parm3 : user supplied data (integers)
*                   parm4 : user supplied data (double)
*            tol:   solution accuracy
*            x1:    solution lower bound
*            x2:    solution upper bound
*            fdata: user supplied data to be passed to func()
*            root:  pointer to variable to contain the solution
*            vaf:   pointer to variable to contain the %VAF for final solution
*
*  returns:
*             0 : success
*            -1 : Invalid bounds : either none or multiple roots
*                 exist within the specified interval.
*            -2 : maximum number of allowed iterations exceeded.
*            -3 : overflow/underflow error in function calculation.
*            -4 : derivative is zero.
*
***************************************************************************/
#define MAXITERS 25
static int NewtonRaphson(double (*func)(double, double *, double *,
					double *, double *),
			 double tol, double x1, double x2,
			 double *user1, double *user2,
			 double *root, double *VAF)
{
  double df, dx, dxold, f, fh, fl, xh, xl, rts, vaf1, vaf2, vaf;
  int i;

  /* evaluate the objective function at the specified initial guess */
  /* and terminate immediately if the objective function is 0 */
  vaf = (*func)(*root, &f, &df, user1, user2);
  if (f == 0.0)
    {
      *VAF = vaf;
      return(0);
    }

  /* evaluate the objective function at the specified interval bounds */
  vaf1 = (*func)(x1, &fl, &df, user1, user2);
  vaf2 = (*func)(x2, &fh, &df, user1, user2);
  sprintf(cbuf, "f(%.2e) = %.2e and f(%.2e) = %.2e", x1, fl, x2, fh);
  printf("%s\n", cbuf);

  /* terminate immediately if one of the bounds is a root */
  if (fl == 0.0)
    {
      *root = x1;
      *VAF = vaf1;
      return(0);
    }
  if (fh == 0.0)
    {
      *root = x2;
      *VAF = vaf2;
      return(0);
    }
    
  /* check if either none or more than one root in the specified interval */
  if (((fl > 0.0) && (fh > 0.0)) || ((fl < 0.0) && (fh < 0.0)))
    {
      sprintf(cbuf, "f(%.2e) = %.2e and f(%.2e) = %.2e", x1, fl, x2, fh);
      draw_mesg(cbuf);
      printf("%s\n", cbuf);
      return(-1);
    }

  /* reorder bounds if necessary according to the value of the function at x1*/
  if (fl < 0.0)
    {
      xl = x1;
      xh = x2;
    }
  else
    {
      xl = x2;
      xh = x1;
    }

  /* If the supplied initial guess is outside bounds, bisect the interval */
  if ((*root < xl) || (*root > xh))
    rts = 0.5 * (xl + xh);
  else
    rts = *root;
  
  /* initialize the search loop vars and calculate function value   */
  /* at the initial guess */
  dxold = fabs(xl + xh);
  dx = dxold;
  vaf = (*func)(rts, &f, &df, user1, user2);

  printf("Newton-Raphson initial vals : xl=%.2e, xh=%.2e, rts=%.2e, f=%.2e\n",
	 xl, xh, rts, f);

  /* solution search loop */
  for (i = 0; i < MAXITERS; i++)
    {
      /* show iteration */
      printf("Iter %d: f=%.2e, df=%.2e, rts=%.2e, dx=%.2e, xl=%.2e, xh=%.2e\n",
	     i, f, df, rts, dx, xl, xh);

      /* check that first derivative is non-zero */
      if (df == 0.0)
	return(-4);
      
      /* if the Newton step is out of range or too small, bisect */
      if (((((rts - xh)*df - f) * ((rts - xl)*df -f)) >= 0.0)
	   || (fabs(2.0 * f) > fabs(dxold * df)))
	{
	  dxold = dx;
	  dx = 0.5 * (xh - xl);
	  rts = xl + dx;
	  printf("Bisection step\n");
	}
      /* else take the newton step and update vars */
      else
	{
	  dxold = dx;
	  dx = f / df;
	  rts -= dx;
	}

      /* check for convergence */
      if (fabs(dx) <= tol)
	{
	  *root = rts;
	  *VAF = vaf;
	  return(0);
	}

      /* evaluate function at updated point */
      vaf = (*func)(rts, &f, &df, user1, user2);

      /* update bounds */
      if (f < 0.0)
	xl = rts;
      else
	xh = rts;
    }

  /* maximum number of iterations exceeded */
  return(-2);
}


/*********************** Objective function      ***************************
*
*  function: Evaluate the function and its derivative at
*            a paticular value. The function here is the derivative of an
*            objective function that we seek to minimize, w/r to its one
*            free parameter ("s", the first zero of the auto-correlation
*            function). The objective function itself is the sum of the
*            squared residuals between the measured data points
*            and the auto-correlation model, in an mxn neighborhood
*            about one point. Note that m & n must be odd.
*  
*            Auto-correlation model:
*            R(x,y) = <I>{1 + sinc(PIx/v)sinc(PIx/v)sinc(PIy/v)sinc(PIy/v)}
*                         where <I> is the average intensity in the 
*                                   mxn neighborhood
*                               v   the first zero of the function
*                                   (speckle size)
*
* parameters:
*           - *v:     adress of current "v" parameter value
*           - *fc:    return address of the value of the function at "v"
*           - *dfc:   return address of the value of the derivative of
*                     the function at "v"
*
***************************************************************************/
static void CrossCorrObjFuncEval(double *v, double *fc, double *dfc)
{
  double sincax, sinc2ax, sincay, sinc2ay, acorr, I2, residual, ax, ay;
  double dsinc2ax, dsinc2ay, sinax, sinay, sumZ2, sumZZp2, val, VAF;
  int x, y, half_width, half_height, index;

  /* init vars */
  half_width = SACMF_width/2;
  half_height = SACMF_height/2;
  I2 = SACMF_I * SACMF_I;
  *fc = 0;
  *dfc = 0;
  sumZ2 = 0;
  sumZZp2 = 0;

  /* loop for all rows in the neighborhood */
  for (y = -half_height; y <= half_height; y++)
    {
      /* pre compute y coordinate sinc function and derivative */
      ay = (PI * ((double) y)) / *v;
      sinay = sin(ay);
      if (ay == 0.0)
	sincay = 1.0; /* l'Hopital's rule */
      else
	sincay = sinay / ay;
      sinc2ay = sincay * sincay;
      if (ay == 0.0)
	dsinc2ay = 0.0; /* l'Hopital's rule */
      else
	dsinc2ay = sinay*(sincay - cos(ay)) / (PI*y);
      
      /* loop for all rows in the neighborhood */
      for (x = -half_width; x <= half_width; x++)
	{
	  /* compute x coordinate sinc function & derivative */
	  ax = (PI * ((double) x)) / *v;
	  sinax = sin(ax);
	  if (ax == 0.0)
	    sincax = 1.0;
	  else
	    sincax = sinax / ax;
	  sinc2ax = sincax * sincax;
	  if (ax == 0.0)
	    dsinc2ax = 0.0; /* l'Hopital's rule */
	  else
	    dsinc2ax = sinax*(sincax - cos(ax)) / (PI*x);
	  
	  /* compute auto-correlation model prediction at x,y */
	  acorr = I2*(1.0 + sinc2ax*sinc2ay);

	  /* compute array index of data value at x,y */
	  index = (SACMF_ystart + y + half_height)*SACMF_xextent
	    + SACMF_xstart + x + half_width + 1;

	  /* compute residual between model and data value at x,y */
	  val = SACMF_indata[index];
	  residual =  val - acorr;

	  /* load the model prediction in the data array (debugging) */
	  if (SACMF_outdata)
	    SACMF_outdata[index] = acorr;

	  /* add residual to running sum of the sum of squred residuals */
	  *fc += (residual * residual);

	  /* add this derivative to the running sum */
	  *dfc -= (4.0*residual * I2*(sinc2ax*dsinc2ay + dsinc2ax*sinc2ay));

	  /* update VAF running sums */
	  sumZ2 = (val * val);
	  sumZZp2 += (residual * residual);
	}
    }

  /* %VAF */
  if (sumZ2 == 0.0)
    SACMF_VAF = -100.0;
  else
    SACMF_VAF = 100.0*(1.0 - sumZZp2/sumZ2);
  if (SACMF_VAF < 0)
    SACMF_VAF = 0;

  /* print out results */
/*
  printf("CrossCorrObjFuncEval() : xc = %f, fc = %.2e, dfc = %.2e\n",
	 *v, *fc, *dfc);
*/
}  

/************** speckle auto-correlation model fit  ************************
*
*  function: perform neighborhood auto-correlation computations
*            and fit with the analytic speckle model (see above)
*            The autocorrelations are performed about each pixel
*            in the image at half the neighborhood width in lags
*            in both the positive and negative x/y directions.
*            +/- kernw lags, where 
*
***************************************************************************/

void SpeckleAutoCorrelationTest(double *inpixels, double *outpixels,
				int height, int width, void *ParmPtrs[])
{
  double v, f, df, root, LB, UB, E1, E2;
  int i, w, MAXCAL, IFAIL;

  /* load neighborhood width and check that it is odd */
  E2 = *((double *) (ParmPtrs[0]));
  v = *((double *) (ParmPtrs[1]));
  LB = (*((double *) (ParmPtrs[2])));
  UB = (*((double *) (ParmPtrs[3])));
  w = 200;
  SACMF_I = 1;
  E1 = 0;
  MAXCAL = 15;

  for (i = 0; i < height*width; i++)
    outpixels[i] = 0;

  SACMF_xstart = 0;
  SACMF_ystart = 0;
  SACMF_width = w;
  SACMF_height = w;
  SACMF_xextent = width;
  SACMF_indata = SACMF_outdata = outpixels;
  CrossCorrObjFuncEval(&v, &f, &df);

  SACMF_indata = outpixels;
  SACMF_outdata = NULL;
  CrossCorrObjFuncEval(&v, &f, &df);
  printf("Synthetic data set created\n\n");

  /* find minimum */
  IFAIL = -1;
  e04bbf((void *) CrossCorrObjFuncEval, &E1, &E2, &LB, &UB, &MAXCAL,
	 &root, &f, &df, &IFAIL);
  printf("Root = %.2e, VAF = %.2f, f = %.2e, df = %.2e\n",
	 root, SACMF_VAF, f, df);
}

void SpeckleAutoCorrelationFit(double *inpixels, double *outpixels, int height,
			       int width, void *ParmPtrs[])
{
  double *acorr, ac, LB, UB, E1, E2, root, I, f, df, tol, A, B;
  int i, j, x, y, lx, ly, k, kernw, fitflag, MAXCAL, IFAIL;

  /* load neighborhood width and check that it is odd */
  kernw = *((int *) (ParmPtrs[0]));
  if ((kernw & 0x01) == 0)
    {
      Abort = B_TRUE;
      draw_mesg("Neighborhood size must be odd");
      return;
    }

  /* load speckle size upper bound check that it is greater than the LB */
  LB = 0.5;
  UB = *((double *) (ParmPtrs[1]));
  if (UB <= LB)
    {
      Abort = B_TRUE;
      draw_mesg("Upper bound must be greater then lower bound");
      return;
    }

  /* load solution tolerance and check that it is >= 0 */
  tol = *((double *) (ParmPtrs[2]));
  if (tol < 0)
    {
      Abort = B_TRUE;
      draw_mesg("Tolerance must be >= 0");
      return;
    }

  /* load model fit flag */
  fitflag = *((int *) (ParmPtrs[3]));

  /* check that stack is not empty, then decrement stack pointer  */
  if (ImageStackTop < 1) {
    Abort = B_TRUE;
    draw_mesg("Abort : No image on the stack for %VAF");
    return;
  }
  
  /* allocate storage for the auto-correlation buffer */
  if ((acorr = UxMalloc(kernw*kernw*sizeof(double))) == NULL)
    {
      Abort = B_TRUE;
      sprintf(cbuf, "Could not allocate %d doubles for buffer", kernw*kernw);
      draw_mesg(cbuf);
      return;
    }

  /* initialize objective function calculation parms */
  SACMF_xstart = 0;
  SACMF_ystart = 0;
  SACMF_width = kernw;
  SACMF_height = kernw;
  SACMF_xextent = kernw;
  SACMF_indata = acorr;
  SACMF_outdata = NULL;

  /* loop for rows in the image */
  for (y = kernw, root=0; (y < (height - kernw)) && (Abort == B_FALSE); y++)
    {
      /* show line number being processed */
      sprintf(cbuf, "Auto-correlation model fit: line %d", y);
      draw_mesg(cbuf);

      /* loop for columns in the image */
      for (x = kernw; (x < (width - kernw)) && (Abort == B_FALSE); x++)
	{
	  /* loop for +/- kernw/2 lags at "y" */
	  for (j = y - kernw/2, k = 0, I = 0; j <= y + kernw/2; j++)
	    /* loop for +/- kernw/2 lags at "x" */
	    for (i = x - kernw/2; i <= x + kernw/2; i++)
	      {
		/* loop for rows in neighborhood for one pt in the acorr */
		for (ly = - kernw/2, ac = 0; ly <= kernw/2; ly++)
		  /* loop for columns in neighborhood for one pt in the acorr*/
		  for (lx = kernw/2; lx <= kernw/2; lx++)
		    ac += (inpixels[(j + ly)*width + i + lx]
			   * inpixels[(y + ly)*width + x + lx]);
		
		/* store the auto-correlation point in the buffer */
		acorr[k++] = ac;
		
		/* add intensity at (i,j) to average running sum */
		I += inpixels[j*width + i];
	      }

	  /* if required, fit the speckle auto-correlation model */
	  if (fitflag)
	    {
	      /* calculate the speckle auto-correlation fit if average */
	      /* intensity is non-zero (ie: the are some valid data    */
	      if (I != 0.0)
		{
		  /* non-linear fit to the speckle acorr model */
		  SACMF_I = I;
		  E1 = 0;
		  E2 = tol;
		  A = LB;
		  B = UB;
		  MAXCAL = 15;
		  IFAIL = -1;
		  e04bbf((void *) CrossCorrObjFuncEval, &E1, &E2, &A, &B,
			 &MAXCAL, &root, &f, &df, &IFAIL);
		  
		  /* check if successfull return from minimization */
		  if (IFAIL == 0)
		    {
		      outpixels[y*width + x] = root;
		    }
		  /* else flag this error situation with negative values */
		  else
		    {
		      outpixels[y*width + x] = -1.0;
		      SACMF_VAF = -99.0;
		    }
		}
	      /* no valid data, flag this with negative values */
	      else
		{
		  outpixels[y*width + x] = -1.0;
		  SACMF_VAF = -99.0;
		}
	      
	      /* store the %VAF in the stack image */
	      ImageStack[ImageStackTop-1].pixels[y*width + x] = SACMF_VAF;
	    }
	  
	  /* if not fitting, just store the peak value of the auto-corr */
	  else
	    {
	      outpixels[y*width + x] = acorr[kernw*kernw/2 + kernw/2];
	    }
	}
    }
  
  /* free storage */
  UxFree(acorr);
}


/*********************** Power Series Model Eval ***************************/
/*                                                                         */
/*  function: Evaluate a power series polynomial model                     */
/*                                                                         */
/***************************************************************************/
void PowerSeriesModelEval(double *inpixels, double *outpixels, int height,
		     int width, void *ParmPtrs[])
{
  MATFile *DataFp;
  boolean_t LoadSuccess;
  double *tmpr, *tmpi, *X, val, u, v, zmax, zmin, xdiv, ydiv;
  int i, j, k, x, y, dx, dy, Order, ModelLen;
  char *p;

  /* load parms */
  p = (char *) (ParmPtrs[0]);

  /* load model coefficients */
  LoadSuccess = B_FALSE;
  if ((DataFp = matOpen(p, "r")) != NULL)
    if (matGetFull(DataFp, "Order", &dx, &dy, &tmpr, &tmpi) == 0) {
      Order = tmpr[0];
      if (matGetFull(DataFp, "zmax", &dx, &dy, &tmpr, &tmpi) == 0) {
	zmax = tmpr[0];
	if (matGetFull(DataFp, "zmin", &dx, &dy, &tmpr, &tmpi) == 0) {
	  zmin = tmpr[0];
	  if (matGetFull(DataFp, "Coeffs", &ModelLen, &dy, &X, &tmpi) == 0)
	    LoadSuccess = B_TRUE;
	}
      }
    }
  matClose(DataFp);
  if (LoadSuccess == B_FALSE) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort : Error loading model parameters from file %s", p);
    draw_mesg(cbuf);
    return;
  }

  /* print out loaded coefficients if required */
  if (VerboseFlag)
    for (i = 0, printf("Model parameters (zmin:%e, zmax:%e):\n", zmin, zmax);
	 i <= Order; i++)
      for (j = 0; j <= Order; j++)
	printf("  a%d%d : %e\n", i, j, X[i*(Order+1) + j]);
    
  /* evaluate field */
  for (y = 0, k = 0, xdiv = 1.0/((double) (width-1)),
       ydiv = 1.0/((double) (height-1));
       (y < height) && (Abort == B_FALSE); y++) {
    v = ((double) (2*y - (height-1))) * ydiv;
    for (x = 0; x < width; x++, k++) {
      u = ((double) (2*x - (width-1))) * xdiv;
      /* evaluate monomial series */
      for (i = 0, val = 0; i <= Order; i++)
	for (j = 0; j <= Order; j++)
	  val += (X[i*(Order+1) + j]
		  * pow(u, (double) i)
		  * pow(v, (double) j));
      /* store the result */
/*      outpixels[k] = (val*(zmax - zmin) + zmax + zmin)*0.5; */
      outpixels[k] = val;
    }
  }
}

/*********************** PolynomialSurfaceFit ******************************/
/*                                                                         */
/*  function: Fit a surface with a bi-variate power series model           */
/*            using least squares.                                         */
/*                                                                         */
/***************************************************************************/
int PolynomialSurfaceFit(double *inpixels, double *outpixels,
			 int height, int width, int order, double *p)
{
  static int OldHeight, OldWidth, OldOrder, lwork, n;
  static double *A, *B, *work, tolerance;
  double sigma, X, Y, Z, Zp, SumZ2, SumZZp2;
  int d, i, j, k, x, y, m, rank, IFAIL, VAF;

  /* determine array size and allocate space if required */
  if ((OldHeight != height) || (OldWidth != width) || (OldOrder != order)) {
    draw_mesg("PolynomialSurfaceFit() : Allocating storage...");
    n = (order+1) * (order+1);
    tolerance = 1e-9;
    lwork = 4 * n;
    A = (double *) UxRealloc(A, height * width * n * sizeof(double));
    B = (double *) UxRealloc(B, height * width * sizeof(double));
    work = (double *) UxRealloc(work, lwork*sizeof(double));
    OldHeight = height;
    OldWidth = width;
    OldOrder = order;
  }
  
  /* find number of points within contour, ie: "m" */
  for (y=0, m=0; y < height; y++)
    for (x = 0; x < width; x++)
      if (MaskPixels[y*width +x] != 0)
	m++;

  /* load m x n data array */
  draw_mesg("PolynomialSurfaceFit() : Loading data matrix...");
  for (y=0, d=0; y < height; y++)
    for (x=0; x < width; x++)
      if (MaskPixels[y*width +x] != 0) {
	for (i = 0, k = 0; i <= order; i++)
	  for (j = 0; j <= order; j++)
	    A[(k++)*m + d] = pow((double) x, (double) i)
	      * pow((double) y, (double) j);
	B[d++] = inpixels[y*width +x];
      }
  
  /* compute NAG least squares solution if parms are OK */
  if ((n < 1) || (m < n)) {
    Abort = B_TRUE;
    draw_mesg("Abort : Invalid NAG call parameters in PolynomialFit()");
    return(0);
  }
  IFAIL = 0;
  draw_mesg("PolynomialSurfaceFit() : Computing least squares fit...");
  f04jaf(&m, &n, A, &m, B, &tolerance, &sigma, &rank, work, &lwork, &IFAIL);

  /* if verbose, print out polynomial coefficients */
  if (VerboseFlag) {
    printf("PolynomialSurfaceFit() (order = %d) Coefficients Aij: ", order);
    for (i = 0; i < n; i++)
      printf(" %e ", B[i]);
    printf("\n");
  }

  /* compute VAF for the parametric representation of the data */
  draw_mesg("PolynomialSurfaceFit() : Computing VAF...");
  for (y=0, SumZ2=0, SumZZp2=0; y < height; y++)
    for (x=0; x < width; x++)
      if (MaskPixels[y*width +x] != 0) {
	/* load values */
	X = x;
	Y = y;
	Z = inpixels[y*width +x];
	/* compute estimated z */
	for (i=0, k=0, Zp=0; i <= order; i++)
	  for (j=0; j <= order; j++)
	    Zp += B[k++] * pow(X, (double) i) * pow(Y, (double) j);
	outpixels[y*width +x] = Zp;
	/* compute sums for VAF computation */
	SumZ2 += (Z * Z);
	SumZZp2 += ((Z - Zp) * (Z - Zp));
      }
      else
	outpixels[y*width +x] = 0;
  VAF = 100.0*(1.0 - SumZZp2/SumZ2);

  /* print out VAF */
  sprintf(cbuf, "PolynomialSurfaceFit() VAF : %d%%", VAF);
  draw_mesg(cbuf);

  /* return parameter array */
  for (i = 0; i < n; i++)
    p[i] = B[i];
  return(VAF);
}

void PolynomialFit(double *inpixels, double *outpixels, int height, int width,
		   void *ParmPtrs[])
{
  static double *u;
  static int OldOrder, n;
  int i, j, k, x, y, order, VAF;
  double X, Y, Zp;

  /* load parameter */
  order = *((int *) ParmPtrs[0]);

  /* make sure contour is defined */
  if (DisplayContourComputed == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort : Undefined contour in QuadraticFit()");
    return;
  }

  /* allocate storage if required */
  if (OldOrder != order) {
    n = (order+1)*(order+1);
    u = (double *) UxRealloc(u, n * sizeof(double));
    OldOrder = order;
  }

  /* fill the output buffer with the parametric fit data */
  VAF = PolynomialSurfaceFit(inpixels, outpixels, height, width, order, u);

  /* load "msg" array with VAF value */
  sprintf(Msg, " = VAF : %d%% ", VAF);
}







