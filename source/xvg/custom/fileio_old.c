/***********************************************************************
*
*  Name:          fileio.c
*
*  Author:        Paul Charette
*
*  Last Modified:
*                 13 Sept 1995: added XvgMM XvgStack support, as well
*                               as new version of GetSGIRGBFile() from David.
*
*  Purpose:       Utility routines for file I/O.
*                    CompWriteMatlabImages()
*                    DeCompReadMatlabImages()
*                    GetBSplineFile()
*                    GetMatlabFile()
*                    GetSGIRGBFile()
*                    GetScanImage()
*                    GetTiffFile()
*                    LoadPxData()
*                    LoadXWDFile()
*                    PeekImage()
*                    WriteGLTiffFile()
*                    WriteIGSFile()
*                    WriteMatlabFile()
*                    WritePxFile()
*                    WriteTiffFileColor()
*                    WriteTiffFileGreyLevel()
*
***********************************************************************/
#include <stdio.h>
#ifdef UIL_CODE
#include "UxXt.h"
#else
#include "UxLib.h"
#endif
#include "XvgGlobals.h"
#ifdef CMGUI
#include "general/image_utilities.h"
#endif
#include <math.h>
#include <fcntl.h>
#include <search.h>
#include <values.h>
#include "mat.h"

/*****************************************************************************/
/*                                                                           */
/*    Routine:   Boolean GetScanImage()                                      */
/*                                                                           */
/*    Purpose:   load ASCII laser scan file                                  */
/*               line format : z (mm/10, int) x y z (mm, float)              */
/*               (lines with z == 0 are to be ignored)                       */
/*                                                                           */
/*****************************************************************************/
#define IMSIZ 500.0
Boolean GetScanImage(char *fname, int *heightp, int *widthp,
		     double *min, double *max, double **pixels,
		     Boolean verbose)
{
  MATFile *DataFp;
  Matrix *a;
  struct timeval tv0, tv1;
  struct timezone tz0, tz1;
  FILE *fp;
  int k,i, j, M, nl, zi, NXEST, NYEST, RANK, LWRK, LIWRK, *IWRK, IFAIL, MX, MY;
  int NC, NWS, NADRES, *ADRES, NPOINT, *POINT, NInteriorKnots, NWRK1, NWRK2;
  int dx, dy;
  char cbuf[512];
  double *X, *Y, *Z, *W, x, y, z, xmax, xmin, ymax, ymin, zmax, zmin, S, FP;
  double *WRK, xyratio, inc, *XD, *YD, *DL, *WS, EPS, SIGMA, *ImDataP;
  Boolean AutoFlag, ASCIIFlag;

  /* set the fit type and data limits */
  AutoFlag = TRUE;
  ASCIIFlag = FALSE;

  if (ASCIIFlag) {
    /* open the ".scan" file */
    if ((fp = fopen(fname, "r")) == NULL) {
      sprintf(cbuf, "Could not open the file %s", fname);
      ErrorMsg(cbuf);
      return(FALSE);
    }
    
    /* count number of lines in the file */
    for (nl=0; fgets(cbuf, 512, fp) != NULL; nl++);
    
    /* allocate storage for the data elements */
    X = (double *) UxMalloc(nl * sizeof(double));
    Y = (double *) UxMalloc(nl * sizeof(double));
    Z = (double *) UxMalloc(nl * sizeof(double));
    W = (double *) UxMalloc(nl * sizeof(double));
    
    /* load in non-zero elements */
    for (M=0, k = 0, fseek(fp, 0, 0); fgets(cbuf, 512, fp) != NULL; k++) {
      if (sscanf(cbuf, "%d %lf %lf %lf", &zi, &x, &y, &z) != 4)
	printf("\aError in format conversion at line %d..\n", k);
      if ((zi != 0) && (z != 0)) {
	X[M] = x;
	Y[M] = y;
	Z[M] = z;
	W[M] = 1.0;
	M++;
      }
    }
  }
  else {
    /* get the data from a MATLAB file */
    if ((DataFp=matOpen(fname, "rb")) == NULL) {
      sprintf(cbuf, "GetScanImage() : could not open input file %s\n",
	      fname);
      ErrorMsg(cbuf);
      return(FALSE);
    }
    if (matGetFull(DataFp, "X", &M, &dy, &X, &ImDataP) != 0) {
      sprintf(cbuf, "GetScanImage() : could not get X from input file %s\n",
	      fname);
      ErrorMsg(cbuf);
      return(FALSE);
    }
    if (matGetFull(DataFp, "Y", &M, &dy, &Y, &ImDataP) != 0) {
      sprintf(cbuf, "GetScanImage() : could not get Y from input file %s\n",
	      fname);
      ErrorMsg(cbuf);
      return(FALSE);
    }
    if (matGetFull(DataFp, "Z", &M, &dy, &Z, &ImDataP) != 0) {
      sprintf(cbuf, "GetScanImage() : could not get Z from input file %s\n",
	      fname);
      ErrorMsg(cbuf);
      return(FALSE);
    }
    matClose(DataFp);
    W = (double *) UxMalloc(M * sizeof(double));
    for (i = 0; i < M; i++)
      W[i] = 1.0;
    printf("M:%d\n", M);
  }

  /* find data extrema */
  for (i = 0, xmax = -MINDOUBLE, xmin = MAXDOUBLE,
       ymax = -MINDOUBLE, ymin = MAXDOUBLE,
       zmax = -MINDOUBLE, zmin = MAXDOUBLE; i < M; i++) {
    xmax = (X[i] > xmax ? X[i] : xmax);
    xmin = (X[i] < xmin ? X[i] : xmin);
    ymax = (Y[i] > ymax ? Y[i] : ymax);
    ymin = (Y[i] < ymin ? Y[i] : ymin);
    zmax = (Z[i] > zmax ? Z[i] : zmax);
    zmin = (Z[i] < zmin ? Z[i] : zmin);
  }
  printf("xmax:%f, xmin:%f, ymax:%f, ymin:%f\n", xmax, xmin, ymax, ymin); 

  /* eleminate points outside prescribed xy limits */
  xmin = -50; xmax = 110;
  ymin = -30; ymax = 150;
  for (i = 0, k = 0; i < M; i++)
    if ((X[i] > xmin) && (X[i] < xmax) && (Y[i] > ymin) && (Y[i] < ymax)) {
      X[k] = X[i];
      Y[k] = Y[i];
      Z[k] = -Z[i];
      k++;
    }
  printf("%d/%d points retained\n", k, M);
  M = k;

  /* determine image size, where maximum dimension is 500 */
  xyratio = (ymax - ymin) / (xmax - xmin);
  if (xyratio >= 1.0) {
    MY = IMSIZ;
    MX = IMSIZ / xyratio;
  }
  else {
    MX = IMSIZ;
    MY = IMSIZ * xyratio;
  }

  /* print out info if required */
  if (VerboseFlag) {
    printf("GetScanImage() : %d/%d non-zero elements in file %s\n",
	   M, nl, fname);
    printf("  Data extrema : x[%f,%f], y[%f,%f], z[%f,%f]\n",
	   xmin, xmax, ymin, ymax, zmin, zmax);
    printf("  Image size   : [%d,%d]\n", MX, MY);
  }

  /* allocate storage for the image if required */
  *widthp = MX;
  *heightp = MY;
  if ((*pixels = (double *) XvgMM_Alloc((void *) *pixels,
					*widthp * *heightp * sizeof(double)))
      == NULL) {
    ErrorMsg("GetScanImage() : could not allocate space for image");
    return(FALSE);
  }
  
  /* set parms, allocate storage and exec NAG bicubic spline fit call */
  XD = (double *) UxMalloc(MX * sizeof(double));
  YD = (double *) UxMalloc(MY * sizeof(double));
  if (AutoFlag) {
    S = ((double) M) * ((zmax - zmin)*0.001);
    NXEST = NYEST = sqrt((double) (M/2));
    LWRK = (7*NXEST*NXEST + 25*NXEST)*(NXEST+1) + 4*(NXEST + M) + 23*NXEST +56;
    WRK = (double *) UxMalloc(LWRK * sizeof(double));
    LIWRK = M + 2*NXEST*NXEST;
    IWRK = (int *) UxMalloc(LIWRK * sizeof(int));
    BSplineKi = (double *) UxMalloc(NXEST * sizeof(double));
    BSplineKj = (double *) UxMalloc(NYEST * sizeof(double));
    BSplineCij = (double *) UxMalloc((NXEST-4) * (NYEST-4) * sizeof(double));
    IFAIL = -1;
    gettimeofday(&tv0, &tz0);
    e02ddf("Cold Start", &M, Y, X, Z, W, &S, &NYEST, &NXEST, 
	   &BSplineNKj, BSplineKj, &BSplineNKi, BSplineKi, BSplineCij,
	   &FP, &RANK, WRK, &LWRK, IWRK, &LIWRK, &IFAIL);
    if (IFAIL != 0) {
      ErrorMsg("GetScanImage() : IFAIL != 0 on return from e02ddf()");
      return(FALSE);
    }
  }
  else {
    NInteriorKnots = 50;
    EPS = MINDOUBLE;
    BSplineNKi = BSplineNKj = NInteriorKnots + 8;
    BSplineKi = (double *) UxMalloc(BSplineNKi * sizeof(double));
    BSplineKj = (double *) UxMalloc(BSplineNKj * sizeof(double));
    BSplineCij = (double *) UxMalloc((BSplineNKi-4)
				     * (BSplineNKj-4) * sizeof(double));
    NC = (BSplineNKi-4)*(BSplineNKj-4);
    DL = (double *) UxMalloc(NC * sizeof(double));
    NWS = (2*NC + 1)*(3*BSplineNKj-6) - 2;
    WS = (double *) UxMalloc(NWS * sizeof(double));
    NADRES = (BSplineNKi-7)*(BSplineNKj-7);
    ADRES = (int *) UxMalloc(NADRES * sizeof(int));
    NPOINT = M + (BSplineNKi-7)*(BSplineNKj-7);
    POINT = (int *) UxMalloc(NPOINT * sizeof(int));
    NWRK1 = 4*MX + BSplineNKi;
    NWRK2 = 4*MY + BSplineNKj;
    LWRK = (NWRK1 > NWRK2 ? NWRK1 : NWRK2);
    WRK = (double *) UxMalloc(LWRK * sizeof(double));
    NWRK1 = MX + BSplineNKi - 4;
    NWRK2 = MY + BSplineNKj - 4;
    LIWRK = (NWRK1 > NWRK2 ? NWRK1 : NWRK2);
    IWRK = (int *) UxMalloc(LIWRK * sizeof(int));
    for (i = 1; i <= NInteriorKnots; i++) {
      BSplineKi[i + 3] = xmin +
	(((double) i) * (xmax - xmin)) / ((double) (NInteriorKnots+1));
      BSplineKj[i + 3] = ymin +
	(((double) i) * (ymax - ymin)) / ((double) (NInteriorKnots+1));
    }
    if (BSplineKi[i + NInteriorKnots] > xmax)
      BSplineKi[i + NInteriorKnots] = xmax;
    if (BSplineKj[i + NInteriorKnots] > ymax)
      BSplineKj[i + NInteriorKnots] = ymax;

    gettimeofday(&tv0, &tz0);
    IFAIL = -1;
    e02zaf(&BSplineNKj, &BSplineNKi, BSplineKj, BSplineKi, &M, Y, X,
	   POINT, &NPOINT, ADRES, &NADRES, &IFAIL);
    if (IFAIL != 0) {
      ErrorMsg("GetScanImage() : IFAIL != 0 on return from e02zaf()");
      return(FALSE);
    }
    IFAIL = -1;
    e02daf(&M, &BSplineNKj, &BSplineNKi, Y, X, Z, W, BSplineKj, BSplineKi,
	   POINT, &NPOINT, DL, BSplineCij, &NC, WS, &NWS, &EPS, &SIGMA, &RANK,
	   &IFAIL);
    if (IFAIL != 0) {
      ErrorMsg("GetScanImage() : IFAIL != 0 on return from e02daf()");
      return(FALSE);
    }
  }
  gettimeofday(&tv1, &tz1);

  /* print out fitting result info if required */
  if (VerboseFlag) {
    sprintf(Msg, "");
    printf("  Bspline fit has nodes [%d,%d], FP=%e, RANK=%d, SIGMA=%e %s\n",
	   BSplineNKi, BSplineNKj, FP, RANK, SIGMA,
	   ElapsedTime(tv0, tv1, ""));
  }

  for (i = 0, printf("BSplineKi:\n"); i < BSplineNKi; i++)
    printf("%f ", BSplineKi[i]);
  printf("\n\n");
  for (i = 0, printf("BSplineKj:\n"); i < BSplineNKj; i++)
    printf("%f ", BSplineKj[i]);
  printf("\n\n");

  /* initilize the XD, YD arrays to span the spline domain */
  printf("X:\n");
  inc = (BSplineKi[BSplineNKi-1] - BSplineKi[0])/((double)(MX-1));
  for (i = 0, x = BSplineKi[0]; i < MX; i++, x+=inc) {
    XD[i] = x;
/*    printf("%f ", x); */
  }
  printf("\n\n");
  if (XD[MX - 1] > BSplineKi[BSplineNKi-1])
    XD[MX - 1] = BSplineKi[BSplineNKi-1];
  printf("Y:\n");
  inc = (BSplineKj[BSplineNKj-1] - BSplineKj[0])/((double)(MY-1));
  for (i = 0, y = BSplineKj[0]; i < MY; i++, y+=inc) {
    YD[i] = y;
/*    printf("%f ", y); */
  }
  printf("\n\n");
  if (YD[MY - 1] > BSplineKj[BSplineNKj-1])
    YD[MY - 1] = BSplineKj[BSplineNKj-1];

  /* NAG call to evaluate the spline at all image points in bspline domain */
  IFAIL = -1;
  e02dff(&MY, &MX, &BSplineNKj, &BSplineNKi, YD, XD, BSplineKj, BSplineKi,
	 BSplineCij, *pixels, WRK, &LWRK, IWRK, &LIWRK, &IFAIL);
  if (IFAIL != 0) {
    ErrorMsg("GetScanImage() : IFAIL != 0 on return from e02dff()");
    return(FALSE);
  }

  /* free storage */
  if (AutoFlag == FALSE) {
    UxFree(DL); UxFree(WS); UxFree(ADRES); UxFree(POINT);
  }
  UxFree(X); UxFree(Y); UxFree(Z); UxFree(W);
  UxFree(WRK); UxFree(IWRK); UxFree(XD); UxFree(YD); 

  /* find fit data extrema */
  for (i = 0, *max = -MINDOUBLE, *min = MAXDOUBLE; i < (*heightp * *widthp);
       i++) {
    *max = (*max > (*pixels)[i] ? *max : (*pixels)[i]);
    *min = (*min < (*pixels)[i] ? *min : (*pixels)[i]);
  }
  
  /* print out fitting evaluation result info if required */
  if (VerboseFlag)
    printf("  Fit extrema are [%e,%e]\n", *min, *max);

  /* return */
  return(TRUE);
}


/*****************************************************************************/
/*                                                                           */
/*    Routine:   Boolean GetBSplineFile()                                    */
/*                                                                           */
/*    Purpose:   load BSpline definition and evaluate it using NAG           */
/*               to generate an image.                                       */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
Boolean GetBSplineFile(char *fname, int *heightp, int *widthp,
		       double *min, double *max, double **pixels,
		       Boolean verbose)
{
  MATFile *DataFp;
  double *WRK, *X, *Y, *ImDataP, *hp, *wp, inc, x, y;
  int i, j, k, dx, dy, MX, MY, LWRK, LIWRK, *IWRK, NWRK1, NWRK2, IFAIL;
  char cbuf[512];
  
  /* flag image data only */
  ComplexDataSet = FALSE;
  
  /* load the BSpline definition */
  if ((DataFp=matOpen(fname, "rb")) == NULL) {
    sprintf(cbuf, "GetBSplineFile() : could not open input file %s\n", fname);
    ErrorMsg(cbuf);
    return(FALSE);
  }
  if (matGetFull(DataFp, "Cij", &dx, &dy, &BSplineCij, &ImDataP) != 0) {
    sprintf(cbuf, "GetBSplineFile() : could not get Cij from input file %s\n",
	    fname);
    ErrorMsg(cbuf);
    return(FALSE);
  }
  if (matGetFull(DataFp, "Ki", &BSplineNKi, &dy, &BSplineKi, &ImDataP) != 0) {
    sprintf(cbuf, "GetBSplineFile() : could not get Ki from input file %s\n",
	    fname);
    ErrorMsg(cbuf);
    return(FALSE);
  }
  if (matGetFull(DataFp, "Kj", &BSplineNKj, &dy, &BSplineKj, &ImDataP) != 0) {
    sprintf(cbuf, "GetBSplineFile() : could not get Kj from input file %s\n",
	    fname);
    ErrorMsg(cbuf);
    return(FALSE);
  }
  if (matGetFull(DataFp, "width", &dx, &dy, &wp, &ImDataP) != 0) {
    sprintf(cbuf, "GetBSplineFile() : couldn't get width from input file %s\n",
	    fname);
    ErrorMsg(cbuf);
    return(FALSE);
  }
  if (matGetFull(DataFp, "height", &dx, &dy, &hp, &ImDataP) != 0) {
    sprintf(cbuf, "GetBSplineFile(): couldn't get height from input file %s\n",
	    fname);
    ErrorMsg(cbuf);
    return(FALSE);
  }
  matClose(DataFp);
  
  /* allocate storage for the image if required */
  *widthp = *wp;
  *heightp = *hp;
  if ((*pixels = (double *) XvgMM_Alloc((void *) *pixels,
					*widthp * *heightp * sizeof(double)))
      == NULL) {
    ErrorMsg("GetBSplineFile() : could not allocate space for image");
    return(FALSE);
  }
  
  /* allocate storage for domain arrays */
  MX = *wp;
  MY = *hp;
  X = (double *) UxMalloc(MX * sizeof(double));
  Y = (double *) UxMalloc(MY * sizeof(double));

  /* initilize the X, Y arrays to span the spline domain */
  inc = (BSplineKi[BSplineNKi-1] - BSplineKi[0])/((double)(MX-1));
  for (i = 0, x = BSplineKi[0]; i < MX; i++, x+=inc)
    X[i] = x;
  if (X[MX - 1] > BSplineKi[BSplineNKi-1])
    X[MX - 1] = BSplineKi[BSplineNKi-1];
  inc = (BSplineKj[BSplineNKj-1] - BSplineKj[0])/((double)(MY-1));
  for (i = 0, y = BSplineKj[0]; i < MY; i++, y+=inc)
    Y[i] = y;
  if (Y[MY - 1] > BSplineKj[BSplineNKj-1])
    Y[MY - 1] = BSplineKj[BSplineNKj-1];
  
  /* allocate storage for the spline evaluation */
  NWRK1 = 4*MX + BSplineNKi;
  NWRK2 = 4*MY + BSplineNKj;
  LWRK = (NWRK1 > NWRK2 ? NWRK1 : NWRK2);
  WRK = (double *) UxMalloc(LWRK * sizeof(double));
  NWRK1 = MX + BSplineNKi - 4;
  NWRK2 = MY + BSplineNKj - 4;
  LIWRK = (NWRK1 > NWRK2 ? NWRK1 : NWRK2);
  IWRK = (int *) UxMalloc(LIWRK * sizeof(int));
  
  /* evaluate fit and generate image */
  IFAIL = -1;
  e02dff(&MY, &MX, &BSplineNKj, &BSplineNKi, Y, X, BSplineKj, BSplineKi,
	 BSplineCij, *pixels, WRK, &LWRK, IWRK, &LIWRK, &IFAIL);
  
  /* find fit data extrema */
  for (i = 0, *max = -MINDOUBLE, *min = MAXDOUBLE; i < (*heightp * *widthp);
       i++) {
    *max = (*max > (*pixels)[i] ? *max : (*pixels)[i]);
    *min = (*min < (*pixels)[i] ? *min : (*pixels)[i]);
  }
  
  /* print out info if required */
  if (verbose)
    printf("GetBSplineFile() : model loaded from file %s (w:%f, h:%f, MX:%d, MY:%d, min:%e, max:%e, BSplineNKi:%d, BSplineNKj:%d)\n",
	   fname, *wp, *hp, MX, MY, *min, *max, BSplineNKi, BSplineNKj);
  
  /* free storage */
  UxFree(WRK); UxFree(IWRK); UxFree(X); UxFree(Y);
}

/*****************************************************************************/
/*                                                                           */
/*    Routine:   Boolean LoadXWDFile()                                       */
/*                                                                           */
/*    Purpose:   image  creation from ".px" 8 bit/pixel format files         */
/*               located in /usr/X11/bitmaps                         */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/
Boolean LoadXWDFile(char *fname, double **pixels, int *heightp, int *widthp,
		    Colormap cmap, XColor *Cells, Boolean verbose)
{
  FILE *fp;
  XWDFileHeader header; 
  XWDColor_Def *LocalCmap;
  char s[512], *inpixels;
  int rc, n, i, j, k, x, y;
  double *dp;
  
  /* flag image data only */
  ComplexDataSet = FALSE;

  /* free any allocated color cells if required */
  if (PagedCmap == FALSE) {
    XFreeColors(UxDisplay, cmap, LutCells, AllocatedCmapCells, 0xffffffff);
    AllocatedCmapCells = 0;
  }

  /* open the ".xwd" file */
  if ((fp = fopen(fname, "rb")) == NULL) {
    sprintf(s, "Could not open the file %s", fname);
    ErrorMsg(s);
    return(FALSE);
  }
  
  /* read file header */
  rc = fread(&header, sizeof(XWDFileHeader), 1, fp);
  if (rc == 0) {
    sprintf(s, "Failed reading header from file %s", fname);
    ErrorMsg(s);
    return(FALSE);
  }
  *widthp = header.pixmap_width;
  *heightp = header.pixmap_height;
  
  /* allocate space if required */
  if ((*pixels = (double *) XvgMM_Alloc((void *) *pixels,
					*widthp * *heightp * sizeof(double)))
      == NULL) {
    sprintf(s, "Could not allocate storage in LoadXWDFile()");
    ErrorMsg(s);
    return(FALSE);
  }
  
  /* read dummy pad bytes */
  rc = fread(s, 1, header.header_size - sizeof(XWDFileHeader), fp);
  if (rc != (header.header_size - sizeof(XWDFileHeader))) {
    sprintf(s, "Failed reading pad bytes from file %s", fname);
    ErrorMsg(s);
    return(FALSE);
  }
  
  /* load colormap */
  if ((LocalCmap = (XWDColor_Def *) UxRealloc(LocalCmap,
					      header.colormap_entries
					      * sizeof(XWDColor_Def)))
      == NULL) {
    sprintf(s, "Failed allocation for colormap data for file %s", fname);
    ErrorMsg(s);
    return(FALSE);
  }
  rc = fread(LocalCmap, sizeof(XWDColor_Def), header.colormap_entries, fp);
  if (rc != header.colormap_entries) {
    sprintf(s, "Failed reading colormap data from file %s", fname);
    ErrorMsg(s);
    return(FALSE);
  }
  
  /* load data */
  n = header.bytes_per_line * header.window_height;
  if ((inpixels = (char *) UxRealloc(inpixels, n)) == NULL) {
    sprintf(s, "Failed allocation for pixel data for file %s", fname);
    ErrorMsg(s);
    return(FALSE);
  }
  rc = fread(inpixels, 1, n, fp);
  if (rc != n) {
    sprintf(s, "Failed reading image data from file %s", fname);
    ErrorMsg(s);
    UxFree(inpixels);
    return(FALSE);
  }

  /* mark the colormap entries as unused */
  for (i = 0; i < header.colormap_entries; i++)
    LocalCmap[i].flags = 0;
  
  /* convert data to double precision and correct lut indicies */
  for (y = 0, k = 0, dp = *pixels; y < header.pixmap_height; y++)
    for (x = 0; x < header.pixmap_width; x++) {
      /* find correct color cell */
      for (j = 0; (j < header.colormap_entries)
	   && (inpixels[y*header.bytes_per_line + x] != LocalCmap[j].pixel);
	   j++);
      /* if the color is new, load it into the Cells array */
      if (LocalCmap[j].flags == 0) {
	if ((k > NPAGEDLUTCELLS)   && (PagedCmap)) {
	  ErrorMsg("Maximum number of colors exceeded!");
	  UxFree(inpixels);
	  return(FALSE);
	}
	Cells[k].red = LocalCmap[j].red;
	Cells[k].green = LocalCmap[j].green;
	Cells[k].blue = LocalCmap[j].blue;
	if (PagedCmap == FALSE) {
	  XAllocColor(UxDisplay, cmap, &(Cells[k]));
	  LutCells[AllocatedCmapCells++] = Cells[k].pixel;
	  LocalCmap[j].flags = Cells[k].pixel;
	  k++;
	}
	else
	  LocalCmap[j].flags = k++;
      }
      *dp++ = Cells[LocalCmap[j].flags].pixel;
    }
  
  /* store colors */
  if (PagedCmap)
    XStoreColors(UxDisplay, cmap, Cells, NPAGEDLUTCELLS);

  /* print out info if required */
  if (VerboseFlag)
    printf("LoadXWDFile() : loaded image (%dx%d) with %d color entries\n",
	   *widthp, *heightp, header.colormap_entries);
  UxFree(inpixels);
  return(TRUE);
}

/*****************************************************************************/
/*                                                                           */
/*    Routine:   char *LoadPxData()                                          */
/*                                                                           */
/*    Purpose:   image  creation from ".px" 8 bit/pixel format files         */
/*               located in /usr/X11/bitmaps/*                       */
/*                                                                           */
/*    Parameters(INPUT):                                                     */
/*               - fname      : image filename                               */
/*               - cmap       : X colormap                                   */
/*               - background : cmap index for desired background color      */
/*               - AllocFlag  : TRUE if colors are to be allocated in cmap   */
/*                              FALSE if colors are to be stored in Cells    */
/*               - Cells      : array of XColor previously allocated         */
/*              (OUTPUT):                                                    */
/*               - width      : image width                                  */
/*               - height     : image height                                 */
/*              (RETURN):                                                    */
/*               - pointer to pixels, else NULL if an error occured          */
/*                                                                           */
/*    Sample Call:                                                           */
/*      char *pixels;                                                        */
/*      int width, height, ccnt;                                             */
/*      pixels = LoadPxData("/usr/X11/bitmaps/xdt_p_misc/mona.px",   */
/*                  DefaultColormap(UxDisplay,UxScreen),                     */
/*                  BlackPixel(UxDisplay,UxScreen), 128, &width, &height)    */
/*                                                                           */
/*    Notes:      1)Files in the above directory that are NOT readable by    */
/*                  this program (such as ".xbm" format files, or 1 bit      */
/*                  deep ".px" image files, ie: X Bitmaps)                   */
/*                  are often directly readible by the AIC pixmap editor     */
/*                                                                           */
/*                 2)The ErrorMsg() routine just prints out an error         */
/*                   message string                                          */
/*                                                                           */
/*****************************************************************************/
#define BUFLEN 512
char *LoadPxData(char *fname, Colormap cmap, int background, int AllocFlag,
		 XColor Cells[], int *width, int *height)
{
  char *pixels;
  XColor color, RGBColor;
  FILE *fp;
  int i, j, k, ConvItems, ccnt, entries;
  char red[5], green[5], blue[5], *buffer;
  char c, d[BUFLEN], s[BUFLEN], CTable[256];

  /* flag image data only */
  ComplexDataSet = FALSE;

  /* open the ".px" file */
  if ((fp = fopen(fname, "r")) == NULL) {
    sprintf(s, "Could not open the file %s", fname);
    ErrorMsg(s);
    return(NULL);
  }

  /* locate "static" keyword */
  do {
    if (fgets(d, BUFLEN, fp) == NULL) {
      sprintf(s, "Could not find \"static\" keywork in the file %s", fname);
      ErrorMsg(s);
      return(NULL);
    }
    sscanf(d, "%s", s);
  } while (strcmp(s, "static") != 0);

  /* fetch image size & stuff */
  if ((fgets(d, BUFLEN, fp) == NULL) ||
      (sscanf(d, "\"%d %d %d", width, height, &ConvItems) != 3)) {
    sprintf(s, "Unsupported image header in file %s", fname);
    ErrorMsg(s);
    return(NULL);
  }
#ifdef DEBUG
  printf("Image file %s: Width=%d, Height=%d, Conversion entries=%d\n",
	 fname, *width, *height, ConvItems);
#endif

  /* Fetch logo size and allocate data storage */
  if ((buffer = (char *) XvgMM_Alloc((void *) buffer, *width + 16)) == NULL) {
    sprintf(s, "Could not allocate storage in LoadPxData()");
    ErrorMsg(s);
    return(NULL);
  }
  if ((pixels = (char *) XvgMM_Alloc((void *) pixels, *width * *height))
      == NULL) {
    sprintf(s, "Could not allocate storage in LoadPxData()");
    ErrorMsg(s);
    return(NULL);
  }
  
  /* Parse the first lines to compute LUT entries for conversion */
  for (i = 0, ccnt = 0, entries=0; i < ConvItems; i++) {
    if (fgets(buffer, BUFLEN, fp) != NULL) {
      if ((sscanf(buffer, "\"%c c %s", &c, s) == 2)
	  || (sscanf(buffer, "\"%c m %s c %s", &c, d, s) == 3)) {

	/* check for tailing double quote and strip it */
	for (j = 0; (s[j] != '"') && (s[j] != '\0'); j++);
	if (s[j] == '"')
	  s[j] = '\0';

	if (strcmp("#Transparent", s) == 0) { 	             /* Transparent */
	  CTable[c] = background;
	  ccnt++;
#ifdef DEBUG
	  printf("Converted \"%c\" (Transparent) to background color\n", c);
#endif
	}
	else {                                   /* hex spec or named color */
	  XParseColor(UxDisplay, cmap, s, &RGBColor);
	  if (AllocFlag)
	    XAllocColor(UxDisplay, cmap, &RGBColor);
	  else {
	    Cells[entries].red = RGBColor.red;
	    Cells[entries].green = RGBColor.green;
	    Cells[entries].blue = RGBColor.blue;
	    RGBColor.pixel = Cells[entries++].pixel;
	  }
	  CTable[c] = RGBColor.pixel;
	  ccnt++;
#ifdef DEBUG
	  printf("Converted \"%c\" to %d (%s)\n", c, RGBColor.pixel, s);
#endif
	}
      }
      /* unsupported */
      else {
	sprintf(s, "Invalid color conversion entry \"%s\" in file %s",
		buffer, fname);
	ErrorMsg(s);
	return(NULL);
      }
    }
    else {
      sprintf(s, "Premature end of file %s", fname);
      ErrorMsg(s);
      return(NULL);
    }
  }
  
  /* check to make sure all color conversions done */
  if (ccnt != ConvItems) {
    sprintf(s, "Only %d of %d lut entries successfully converted in file %s",
	    ccnt, ConvItems, fname);
    ErrorMsg(s);
    return(NULL);
  }

  /* Scan the image data to convert from char to LUT indices */
#ifdef DEBUG
  printf("Converting image to X LUT indicies...\n");
#endif
  for (i = 0; i < *height; i++) {
    if (fgets(buffer, (*width + 16), fp) == NULL) {
      sprintf(s, "Premature end of file %s", fname);
      ErrorMsg(s);
      return(NULL);
    }
    if (strlen(buffer) < (*width +2)) {
      sprintf(s, "Premature end of line in file %s", fname);
      ErrorMsg(s);
      return(NULL);
    }
    for (j = 0; j < *width; j++)
      pixels[i*(*width) + j] = CTable[buffer[j+1]];
  }
#ifdef DEBUG
  printf("Done\n");
#endif

  /* clean up */
  XvgMM_Free((void **)&(buffer));
  fclose(fp);

  /* return the pixmap */
  return(pixels);
}

void PeekImage(char *fname, int *height, int *width, int *ConvItems)
{
  FILE *fp;
  char buffer[BUFLEN], s[BUFLEN];
  
  /* open the ".px" file */
  if ((fp = fopen(fname, "r")) == NULL) {
    printf("Could not open the file %s\n", fname);
    exit(0);
  }
  
  /* locate "static" keyword */
  do {
    if (fgets(buffer, BUFLEN, fp) == NULL) {
      printf("Could not find \"static\" keywork in the file %s\n", fname);
      exit(0);
    }
    sscanf(buffer, "%s", s);
  } while (strcmp(s, "static") != 0);
  
  /* fetch image size & stuff */
  if ((fgets(buffer, BUFLEN, fp) == NULL) ||
      (sscanf(buffer, "\"%d %d %d", width, height, &ConvItems) != 3)) {
    printf("Could not fetch  image stats from the file %s\n", fname);
    exit(0);
  }
  
  /* close the file */
  fclose(fp);
}

/****************************** WritePXFile  *******************************/
/*                                                                         */
/*  function: write image & assorted stuff in px format                    */
/*                                                                         */
/***************************************************************************/
Boolean WritePxFile(int height, int width, double *pixels,
		    char *fname, int ncolors)
{
  FILE *fp;
  char fnameStripped[512];
  char symbols[]="abdefghijklmnopqrstuvwxyzABDEFGHIJKLMNOPQRSTUVWXYZ1234567890@*.:";
  char s[256];
  int i, j, k;
  double min, max, divisor;

  /* open the ".px" file */
  if ((fp = fopen(fname, "w")) == NULL) {
    sprintf(s, "Could not open the file %s", fname);
    ErrorMsg(s);
    return(FALSE);
  }

  /* strip the directory path from the file name */
  for (j = (strlen(fname) - 1); (j >= 0) && (fname[j] != '/'); j--);
  if (j == 0)
    sprintf(fnameStripped, "%s", fname);
  else
    sprintf(fnameStripped, "%s", &(fname[j+1]));

  /* write out the px header */
  fprintf(fp, "/* XPM2 C */\n");
  fprintf(fp, "static char ** %s = {\n", fnameStripped);
  fprintf(fp, "\"%d %d %d 1\",\n", width, height, ncolors+2);

  /* generate conversion table and write it out */
  fprintf(fp, "\"  c #Transparent\",\n");
  fprintf(fp, "\"a c #%04x%04x%04x\",\n", 
	  CurrentCells[0].red,
	  CurrentCells[0].green,
	  CurrentCells[0].blue);
  for (i = 1; i <= ncolors; i++)
    fprintf(fp, "\"%c c #%04x%04x%04x\",\n", symbols[i],
	    CurrentCells[(i * 128/ncolors) - 1].red,
	    CurrentCells[(i * 128/ncolors) - 1].green,
	    CurrentCells[(i * 128/ncolors) - 1].blue);

  /* check for NULL images */
  DynamicRange(pixels, height*width, &max, &min);
  if ((max - min) <= (MINDOUBLE*1000.0)) {
    ErrorMsg("WritePxFile() : NULL image");
    return(FALSE);
  }
    
  /* covert pixels to ASCII characters and save to the file */
  if (DisplayContourComputed)
    for (j = 0, k = 0, divisor = ((double) ncolors) / (max - min); j < height; j++) {
      fprintf(fp, "\"");
      for (i = 0; i < width; i++, k++)
	if (MaskPixels[k] == 0)
	  fprintf(fp, " ");
	else
	  fprintf(fp, "%c", symbols[(int) nearest((pixels[k] - min)*divisor)]);
      if (j < height-1)
	fprintf(fp, "\",\n");
      else
	fprintf(fp, "\"};\n");
    }
  else
    for (j = 0, k = 0, divisor = ((double) ncolors) / (max - min); j < height; j++) {
      fprintf(fp, "\"");
      for (i = 0; i < width; i++, k++)
	fprintf(fp, "%c", symbols[(int) nearest((pixels[k] - min)*divisor)]);
      if (j < height-1)
	fprintf(fp, "\",\n");
      else
	fprintf(fp, "\"};\n");
    }

  /* close the file */
  fclose(fp);
}

/***************************** WriteIGSFile  *******************************/
/*                                                                         */
/*  function: write image & assorted stuff in IGS format                   */
/*                                                                         */
/***************************************************************************/
Boolean WriteIGSFile(int height, int width, double *pixels, char *fname)
{
  FILE *fp;
  char s[256], spaces[]="                                                                        ";
  char fnameStripped[512];
  int i, j, k;
  double x, y;

  /* open the ".igs" file */
  if ((fp = fopen(fname, "w")) == NULL) {
    sprintf(s, "Could not open the file %s", fname);
    ErrorMsg(s);
    return(FALSE);
  }

  /* strip the directory path from the file name */
  for (j = (strlen(fname) - 1); (j >= 0) && (fname[j] != '/'); j--);
  if (j == 0)
    sprintf(fnameStripped, "%s", fname);
  else
    sprintf(fnameStripped, "%s", &(fname[j+1]));

  /* write out the IGS header */
  sprintf(s, "1H,,1H;,4HIGES,%dH%s,22HI-DEAS Master Series 1,",
	  strlen(fnameStripped), fnameStripped);
  fprintf(fp, "                                                                        S0000001\n");
  fprintf(fp, "Maximum GME point tolerance:  0.0                                       S0000002\n");
  fprintf(fp, "Minimum GME point tolerance:  10.0                                      S0000003\n");
  fprintf(fp, "                                                                        S0000004\n");
  fprintf(fp, "%s%sG0000001\n", s, &(spaces[strlen(s)]));
  fprintf(fp, "8HIGES 5.1,32,37,8,308,16,22HI-DEAS Master Series 1,1.00000000,2,2HMM,  G0000002\n");
  fprintf(fp, "1,1.00000000,13H931111.202811,0.01000000,1.00000005,4HNONE,4HNONE,8,0,; G0000003\n");

  /* loop to write the point header lines */
  for (i = 1, j = 1; i < (height*width + 1);) {
    fprintf(fp, "     116%8d       0       1       0       0       0       000000000D%7d\n",
	    i++, j++);
    fprintf(fp, "     116       0       5       1       0                   POINT       0D%7d\n",
	    j++);
  }

  /* loop to write the point data */
  for (y = 0, i = 1, j = 1, k = 0; y < height; y++)
    for (x = 0; x < width; x++, i++, j+=2, k++) {
      sprintf(s, "116,%.8f,%.8f,%.8f;", x, y, pixels[k]);
      fprintf(fp, "%s%s%12dP%7d\n", s, &(spaces[strlen(s) + 12]), j, i);
    }
  
  /* write the last line */
  fprintf(fp, "S0000004G0000003D%07dP%07d                                        T0000001\n", j-1, i-1);

  /* close the file */
  fclose(fp);
}

/***************************** WriteMatlabFile  ****************************/
/*                                                                         */
/*  function: write image & assorted stuff in regular MATLAB format        */
/*                                                                         */
/***************************************************************************/
Boolean WriteMatlabFile(int height, int width, char *vname,
			double *pixels, char *fname)
{
  MATFile *DataFp;
  Matrix *a;
  char cbuf[512];
  double *p;
  int i;
  
  /* open the matlab output file */
  if ((DataFp=matOpen(fname, "wb")) == NULL) {
    sprintf(cbuf, "WriteMatlabFile : could not open output file %s\n", fname);
    ErrorMsg(cbuf);
    return(FALSE);
  }

  /* save image data */
  a = mxCreateFull(width, height, REAL);
  memcpy(mxGetPr(a), pixels, height * width * sizeof(double));
  mxSetName(a, "xvgimage");
  matPutMatrix(DataFp, a);
  
  /* save radius image data if required */
/*
  if (ComplexDataSet) {
    memcpy(mxGetPr(a), RadiusImgData, height * width * sizeof(double));
    mxSetName(a, "magnitude");
    matPutMatrix(DataFp, a);
  }
*/
  mxFreeMatrix(a);

  /* if there's a contour defined, then save it */
  if (DisplayContourComputed) {
    a = mxCreateFull(NContourPoints, 1, REAL);
    for (i = 0, p = mxGetPr(a); i < NContourPoints; i++)
      p[i] = ContourPoints[i].x;
    mxSetName(a, "contourx");
    matPutMatrix(DataFp, a);
    for (i = 0, p = mxGetPr(a); i < NContourPoints; i++)
      p[i] = ContourPoints[i].y;
    mxSetName(a, "contoury");
    matPutMatrix(DataFp, a);
    mxFreeMatrix(a);
  }
  
  /* close file and return */
  matClose(DataFp);
  return(TRUE);
}

/*********************** CompWriteMatlabImage  *****************************/
/*                                                                         */
/*  function: write the phase stepping data in char data type MATLAB file  */
/*                                                                         */
/***************************************************************************/
Boolean CompWriteMatlabImages(char *fname_mant,
			      unsigned short *sbufs, char *suffix)
{
  static char buf[640*480*5], fname[256];
  Fmatrix x;
  FILE *fpo;
  int i, nimages;
  char cbuf[512], vname[512], *stype;
  Boolean go;

  /* open matlab output file */
  sprintf(fname, "%s_%s.smat", fname_mant, suffix);
  fpo = fopen(fname, "wb");
  if (fpo == NULL) {
    sprintf(cbuf, "Could not open MATLAB input file: %s", fname);
    ErrorMsg(cbuf);
    return(FALSE);
  }

  /* init grabbing type and nimages */
  switch (GrabType) {
  case THREESTEPGRAB:
    stype = "PStepData3Step";
    nimages = 3;
    break;
  case FOURSTEPGRAB:
    stype = "PStepData4Step";
    nimages = 4;
    break;
  case FOUR_ASTEPGRAB:
    stype = "PStepData4AStep";
    nimages = 4;
    break;
  case FIVESTEPGRAB:
    stype = "PStepData5Step";
    nimages = 5;
    break;
  default:
    stype = "Other";
    nimages = 1;
    break;
  }

  /* compress data to byte form, keeping RED pixel data only */
  for (i = 0; i < (FrameHeight*FrameWidth*nimages); i++)
    buf[i] = sbufs[i] & 0x00FC;
  
  /* machine ID = 1, row-wise oriented matrix of unsigned char elements  */
  x.type = (1 * 1000) + (1 * 100) + (5 * 10) + (0);
  x.mrows = FrameHeight;
  x.ncols = FrameWidth;
  x.imagf = 0;
  
  /* loop for images */
  for (i = 0, go = TRUE; (i < nimages) && go; i++) {
    /* init vname */
    sprintf(vname, "%s%s%d", stype, suffix, i);
    
    /* store header */
    x.namlen = strlen(vname) + 1;
    fwrite(&x, sizeof(Fmatrix), 1, fpo);
    fwrite(vname, sizeof(char), (int)(x.namlen), fpo);
    
    /* write image data to the output file */
    if (fwrite(&(buf[i*FrameWidth*FrameHeight]), sizeof(char),
	       FrameWidth*FrameHeight, fpo) != FrameWidth*FrameHeight)
      go = FALSE;
  }
  
  /* close file and check return code */
  fclose(fpo);
  if (go)
    return(TRUE);
  else {
    ErrorMsg("Failed writing to MATLAB output file");
    return(FALSE);
  }
}


/*********************** DeCompReadMatlabImage  ****************************/
/*                                                                         */
/*  function: read the phase stepping data in char data type MATLAB file   */
/*            and convert the data to phase using the ATAN lookup table    */
/*                                                                         */
/***************************************************************************/
Boolean DeCompReadMatlabImages(int *height, int *width,
			       double *min, double *max,
			       double **pixels,
			       char *fname, Boolean verbose)
{
  static char *buf=NULL;
  Fmatrix x;
  FILE *fpo;
  int i, k, rc, lt, lw, lh, length;
  unsigned short *p;
  double *b;
  char cbuf[512], mbuf[256], vname[256];

#ifdef CMGUI
  ErrorMsg("function DeCompReadMatlabImages() disabled in this version");
  return(FALSE);
#endif

  /* open matlab input file */
  fpo = fopen(fname, "rb");
  if (fpo == NULL) {
    sprintf(cbuf, "Could not open MATLAB input file: %s", fname);
    ErrorMsg(cbuf);
    return(FALSE);
  }

  /* init local buffer if required */
  if ((buf = (char *) XvgMM_Alloc((void *) buf, 640*480)) == NULL) {
    sprintf(cbuf,
	    "DeCompReadMatlabImages(): Error allocating %d bytes",
	    640*480);
    ErrorMsg(cbuf);
    return(FALSE);
  }
  
  /* loop to read images */
  for (k = 0, rc = fread((char *)(&x), sizeof(Fmatrix), 1, fpo);
       rc != 0; rc = fread((char *)(&x), sizeof(Fmatrix), 1, fpo)) {
    /* read variable name */
    if (fread(vname, sizeof(char), x.namlen, fpo) == 0) {
      sprintf(cbuf, "Failed reading variable name from %s", fname);
      ErrorMsg(cbuf);
      return(FALSE);
    }
    
    /* check that the variable is an image variable name */
    sprintf(cbuf, "%s", vname);
    cbuf[5] = '\0';
    if (strcmp(cbuf, "PStep") == 0) {
      /* save the image data variable name */
      sprintf(mbuf, "%s", vname);
      /* save image height and width */
      *height = x.mrows;
      *width = x.ncols;
      length = *height * *width;
      /* read image data */
      if (fread(buf, sizeof(char), x.mrows * x.ncols, fpo)
	  != (x.mrows * x.ncols)) {
	sprintf(cbuf, "Failed reading image data from %s", fname);
	ErrorMsg(cbuf);
	return(FALSE);
      }
      /* convert from byte to short integer data */
      for (i = 0, p = &(RawPsBufsX[x.mrows * x.ncols * k]); i < length; i++)
	p[i] = buf[i];
      /* increment image counter */
      k++;
    }
    else
      rc = 0;
  }
  fclose(fpo);

  /* check that space was allocated for the phase image */
  if ((*pixels = (double *) XvgMM_Alloc((void *) *pixels,
					length * sizeof(double))) == NULL) {
    sprintf(cbuf,
	    "DeCompReadMatlabImages(): Error allocating %d bytes", length);
    ErrorMsg(cbuf);
    return(FALSE);
  }

  /* check that space was allocated for the radius image */
  if ((RadiusImgData = (double *) XvgMM_Alloc((void *) RadiusImgData,
					      length * sizeof(double)))
      == NULL) {
    sprintf(cbuf,
	    "DeCompReadMatlabImages(): Error allocating %d bytes", length);
    ErrorMsg(cbuf);
    return(FALSE);
  }

  /* check that space was allocated fot the sin image */
  if ((SinImgData = (double *) XvgMM_Alloc((void *) SinImgData,
					   length * sizeof(double)))
      == NULL) {
    sprintf(cbuf,
	    "DeCompReadMatlabImages(): Error allocating %d bytes", length);
    ErrorMsg(cbuf);
    return(FALSE);
  }

  /* check that space was allocated fot the cos image */
  if ((CosImgData = (double *) XvgMM_Alloc((void *) CosImgData,
					   length * sizeof(double)))
      == NULL) {
    sprintf(cbuf,
	    "DeCompReadMatlabImages(): Error allocating %d bytes", length);
    ErrorMsg(cbuf);
    return(FALSE);
  }

  /* acertain the grabbing type */
  lt = GrabType;
  mbuf[11] = '\0';
  if (strcmp(mbuf, "PStepData3S") == 0)
    GrabType = THREESTEPGRAB;
  else if (strcmp(mbuf, "PStepData4S") == 0)
    GrabType = FOURSTEPGRAB;
  else if (strcmp(mbuf, "PStepData4A") == 0)
    GrabType = FOUR_ASTEPGRAB;
  else if (strcmp(mbuf, "PStepData5S") == 0)
    GrabType = FIVESTEPGRAB;
  else
    GrabType = GRAYLEVELGRAB;

  /* make atan lut is created */
  if (AtanLutCreated == FALSE)
    CreateAtanLut(AtanRadiusThreshold);

  /* print out info */
  if (verbose)
    printf("DeCompReadMatlabImages() : %d raw stepping images read (w:%d, h:%d, type:%d) from file \"%s\"\n", k, *width, *height, GrabType, fname);

  /* convert the raw step images to angle/radius data using the ATAN lut */
  lh = FrameHeight;
  lw = FrameWidth;
  FrameHeight = *height;
  FrameWidth = *width;
#ifndef CMGUI
  Grab(*pixels, RawPsBufsX, FALSE, TRUE);
#endif
  FrameHeight = lh;
  FrameWidth = lw;
  GrabType = lt;

  /* find data extrema */
  for (i = 0, *min = MAXDOUBLE, *max = MINDOUBLE, b = *pixels;
       i < (*height * *width); i++, b++) {
    *min = ((*b < *min) ? *b : *min);
    *max = ((*b > *max) ? *b : *max);
  }

  /* return */
  return(TRUE);
}

/******************************* GetMatlabFile  ****************************/
/*                                                                         */
/*  function: read image & assorted stuff in regular MATLAB format         */
/*                                                                         */
/***************************************************************************/
Boolean GetMatlabFile(int *heightp, int *widthp, double *min, double *max,
		      double **pixels, char *fname, Boolean verbose)
{
  MATFile *DataFp;
  Matrix *a;
  double *p;
  int i, nfiles, lwidth, lheight, dx, dy;
  double *lmax, *lmin, *dr, ScaleFactor;
  char cbuf[512], **dir, *vname;

#ifdef DEBUG 
  printf("GetMatlabFile(\"%s\")\n", fname);
#endif

  /* open the matlab output file */
  if ((DataFp=matOpen(fname, "rb")) == NULL) {
    sprintf(cbuf, "GetMatlabFile : could not open input file %s\n", fname);
    ErrorMsg(cbuf);
    return(FALSE);
  }

  /* look for the first entry in the directory */
  if ((dir = matGetDir(DataFp, &nfiles)) == NULL) {
    sprintf(cbuf, "GetMatlabFile : could not get directory from file %s\n",
	    fname);
    ErrorMsg(cbuf);
    return(FALSE);
  }
  vname = dir[0];
  if (VerboseFlag)
    printf("GetMatlabFile() : First directory entry is \"%s\"\n", vname);
  
  /* read in image data */
  if (matGetFull(DataFp, vname, widthp, heightp, pixels, NULL) != 0) {
    sprintf(cbuf, "GetMatlabFile : could not get matrix \"%s\" in file %s\n",
	    vname, fname);
    ErrorMsg(cbuf);
    matFreeDir(dir, nfiles);
    return(FALSE);
  }

  /* check for any scaling factor and scale if required */
  if ((matGetFull(DataFp, "Max", &dx, &dy, &lmax, NULL) == 0)
      && (matGetFull(DataFp,"Min", &dx, &dy, &lmin, NULL) == 0)
      && (matGetFull(DataFp, "DynamicRange", &dx, &dy, &dr, NULL) == 0)) {
    if (VerboseFlag)
      printf("GetMatlabFile() : Image scaled (max:%e, min:%e, dr:%e)\n",
	     *lmax, *lmin, *dr);
    if ((*lmax == *lmin) || (*dr == 0) || isnan((*lmax - *lmin)/(*dr)))
      ScaleFactor = 1.0;
    else
      ScaleFactor = (*lmax - *lmin)/(*dr);
    for (i = 0; i < (*heightp * *widthp); i++)
      (*pixels)[i] = ((*pixels)[i] * ScaleFactor) + *lmin;
    *min = *lmin;
    *max = *lmax;
  }
  else 
    /* find image data extrema */
    for (i = 0, *min = MAXDOUBLE, *max = MINDOUBLE, p = *pixels;
	 i < (*heightp * *widthp); i++, p++) {
      *min = ((*p < *min) ? *p : *min);
      *max = ((*p > *max) ? *p : *max);
    }
  
  /* print out info if required */
  if (VerboseFlag)
    printf("GetMatlabFile() : Loaded \"%s\" from %s (%dx%d), max=%e, min=%e\n",
	   vname, fname, *widthp, *heightp, *max, *min);

  /* read in magnitude portion of an image if there is any */
  if (matGetFull(DataFp, "magnitude", &lwidth, &lheight,
		 &RadiusImgData, NULL) == 0) {
    if ((lwidth != *widthp) || (lheight != *heightp))
      InfoMsg("Warning : Image data and magnitude have different sizes!");
    ComplexDataSet = TRUE;
    if (VerboseFlag)
      printf("GetMatlabFile() : Loaded \"magnitude\" from %s...\n",
	     fname, *widthp, *heightp);
  }
  else
    ComplexDataSet = FALSE;

  /* free storage and close the file */
  XvgMM_Free((void **)&(lmax));
  XvgMM_Free((void **)&(lmin));
  XvgMM_Free((void **)&(dr));
  matClose(DataFp);
  matFreeDir(dir, nfiles);
  return(TRUE);
}


/************************ SGI rgb image file programs  *********************/
/*                                                                         */
/*  function: load an SGI rgb file                                         */
/*                                                                         */
/***************************************************************************/
Boolean GetSGIRGBFile(char *fname, double **pixels, int *heightp, int *widthp,
		      double *min, double *max)
{
  Boolean return_code;
  char cbuf[256];
  long int number_of_components, i, height, width;
	unsigned char *image;
  
  /* set return code */
  return_code = FALSE;
  
#ifdef CMGUI
  /* check for null pointers */
  if ((heightp) && (widthp) && (pixels) && (min) && (max))
    {
      /* read in rgb image data */
      if (read_rgb_image_file(fname, &number_of_components,
				  &height, &width,
				  (long unsigned **) (&image)))
			{
				/* allocate storage for intensity data data */
				if ((height != *heightp) || (width != *widthp))
				{
					*heightp = height;
					*widthp = width;
					if (*pixels != NULL)
					mxFree(*pixels);
					*pixels = (double *) mxCalloc(height*width, sizeof(double));
				}
				if (*pixels)
				{
					switch (number_of_components)
					{
						case 1:
							/* I */
						{
							/* convert I data to intensity */
							for (i = 0; i < (height * width); i++)
							{
								(*pixels)[i] = image[i];
							}
						} break;
						case 2:
							/* IA */
						{
							/* convert IA data to intensity */
							for (i = 0; i < (height * width); i++)
							{
								(*pixels)[i] = image[2*i];
							}
						} break;
						case 3:
							/* RGB */
						{
							/* convert RGB data to intensity */
							for (i = 0; i < (height * width); i++)
							{
								(*pixels)[i] = image[3*i]+image[3*i+1]+image[3*i+2];
							}
						} break;
						case 4:
							/* RGBA */
						{
							/* convert RGBA data to intensity */
							for (i = 0; i < (height * width); i++)
							{
								(*pixels)[i] = image[4*i]+image[4*i+1]+image[4*i+2];
							}
						} break;
					}
					/* free rgb buffer storage */
					DEALLOCATE(image);
					
					/* get dynamic range */
					DynamicRange(*pixels, height*width, max, min);
					
					/* set return code */
					return_code = TRUE;
				}
				else
				{
					sprintf(cbuf, "GetSGIRGBFile. Can't alloc storage for %dx%d",
						width, height);
					write_error(CM_ERROR_MESSAGE, cbuf);
				}
			}
      else 
			{
				write_error(CM_ERROR_MESSAGE, "GetSGIRGBFile. read failed");
			}
    }
  else
    {
      write_error(CM_ERROR_MESSAGE, "GetSGIRGBFile. Null pointer arguments");
    }
#endif
  
  /* return success or failure */
  return(return_code);
}


/************************ TIFF image file programs  ************************/
/*                                                                         */
/*  function: generate a TIFF format image file and decodes format         */
/*                                                                         */
/***************************************************************************/
#define LH 0x4949    /* low byte at low address */
#define HL 0x4D4D    /* opposite */
#define ROWSPERSTRIP 8

struct tag_entry {
  unsigned short num;
  char *name;
} tag_table[] = {
  254, "NEW SUBPROFILE TYPE",
  256, "WIDTH",
  257, "HEIGHT",
  258, "BITS PER SAMPLE",
  259, "COMPRESSION",
  262, "PHOTOMETRIC INTERPRETATION",
  269, "DOCUMENT NAME",
  270, "IMAGE DESCRIPTION",
  271, "MAKE",
  272, "MODEL",
  273, "STRIP OFFSETS",
  277, "SAMPLES PER PIXELS",
  278, "ROWS PER STRIP",
  279, "STRIP BYTE COUNTS",
  282, "X RESOLUTION",
  283, "Y RESOLUTION",
  284, "PLANAR CONFIGURATION",
  285, "PAGE NAME",
  286, "X POSITION",
  287, "Y POSITION",
  290, "GRAY RESPONSE UNIT",
  291, "GRAY RESPONSE CURVE",
  292, "GROUP 3 OPTIONS (FACSIMILE OPTION)",
  293, "GROUP 4 OPTIONS (FACSIMILE OPTION)",
  296, "RESOLUTION UNIT",
  297, "PAGE NUMBER",
  301, "COLOR RESPONSE CURVES",
  305, "SOFTWARE",
  306, "DATE TIME",
  315, "ARTIST",
  316, "HOST COMPUTER",
  317, "PREDICTOR",
  320, "COLORMAP",
  65535, NULL,
};

struct IFD_entry {
  unsigned short tag;
  unsigned short type;
  unsigned long length;
  unsigned long value_offset;
};

struct tiff_header_struct {
  unsigned short byte_order;
  unsigned short version;
  unsigned long IFD_offset;
  unsigned short IFD_entry_count;
};

struct IFD_struct {
  struct IFD_entry image_width;
  struct IFD_entry image_length;
  struct IFD_entry bits_per_sample;
  struct IFD_entry photometric_int;
  struct IFD_entry image_description;
  struct IFD_entry strip_offsets;
  struct IFD_entry samples_per_pixel;
  struct IFD_entry rows_per_strip;
  struct IFD_entry strip_byte_counts;
  struct IFD_entry colormap;
  unsigned long next_IFD_offset;
};


/************************** display tag contents *******************/
static void echo_tag(struct IFD_entry *IFD, char *inbuf)
{
  int j, k;

  for (j = 0;
       (tag_table[j].name != NULL) && (tag_table[j].num != IFD->tag); j++);
  
  if (tag_table[j].name != NULL)
    printf("  *%s tag: ", tag_table[j].name);
  else
    printf("\a**** WARNING: %d UNKNOWN TAG!!  ", IFD->tag);
  
  switch (IFD->type) {
  case 1:	                                          /* byte data type */
    for (k = 0; (k < IFD->length) && (k < 10); k++)
      printf("%d ", ((char *) inbuf)[k]);
    break;
  case 2:	                                         /* ASCII data type */
    k = 0;
    printf("%s", inbuf);
    break;
  case 3:                                          /* short data type */
    for (k = 0; (k < IFD->length) && (k < 10); k++)
      printf("%d ", ((unsigned short *) inbuf)[k]);
    break;
  case 4:                                           /* long data type */
    for (k = 0; (k < IFD->length) && (k < 10); k++)
      printf("%d ", ((unsigned long *) inbuf)[k]);
    break;
  case 5:                                       /* rational data type */
    for (k = 0; (k < (2*IFD->length)) && (k < 10); k+=2)
      printf("%f ", (float)(((unsigned long *) inbuf)[k])
	     / (float)(((unsigned long *) inbuf)[k+1]));
    break;
  default:
    printf("\aWARNING: %d IS AN UNSUPPORTED DATA TYPE!\n", IFD->type);
    break;
  }
  if ((k == 10) || (k == 256))
    printf(" ...");
  printf("\n");
}  

/******************** get pointer to data in IFD block *********************/
static Boolean fetch_ptr(struct IFD_entry *IFD, FILE *fpo,
			 char *inbuf, int blab)
{
  int rc, i;
  char cbuf[512];

  if (IFD->length == 0)
    return(TRUE);

  switch (IFD->type) {
  case 1:	                                          /* byte data type */
#ifdef DEBUG
    printf("fetch_ptr() : Byte array of length %d\n", IFD->length);
#endif
  case 2:	                                          /* char data type */
#ifdef DEBUG
    printf("fetch_ptr() : Char array of length %d\n", IFD->length);
#endif
    if (IFD->length <= 4)
      for (i = 0; i < IFD->length; i++)
	inbuf[i] = ((char *) &(IFD->value_offset))[i];
    else {
      fseek(fpo, IFD->value_offset, 0);
      rc = fread(inbuf, IFD->length, sizeof(unsigned char), fpo);
      if (rc == 0) {
	sprintf(cbuf, "Failed reading IFD data from data file");
	ErrorMsg(cbuf);
	return(FALSE);
      }
    }
    break;
  case 3:                                                /* short data type */
#ifdef DEBUG
    printf("fetch_ptr() : Short array of length %d\n", IFD->length);
#endif
    if (IFD->length <= 2)
      for (i = 0; i < IFD->length; i++)
	((unsigned short *) inbuf)[i] = ((unsigned short *) &(IFD->value_offset))[i];
    else {
      fseek(fpo, IFD->value_offset, 0);
      rc = fread(inbuf, IFD->length, sizeof(unsigned short), fpo);
      if (rc == 0) {
	sprintf(cbuf, "Failed reading IFD data from data file");
	ErrorMsg(cbuf);
	return(FALSE);
      }
    }
    break;
  case 4:                                                 /* long data type */
#ifdef DEBUG
    printf("fetch_ptr() : Long array of length %d\n", IFD->length);
#endif
    if (IFD->length <= 1)
      for (i = 0; i < IFD->length; i++)
	((unsigned long *) inbuf)[i] = ((unsigned long *) &(IFD->value_offset))[i];
    else {
      fseek(fpo, IFD->value_offset, 0);
      rc = fread(inbuf, IFD->length, sizeof(unsigned long), fpo);
      if (rc == 0) {
	sprintf(cbuf, "Failed reading IFD data from data file");
	ErrorMsg(cbuf);
	return(FALSE);
      }
    }
    break;
  case 5:                                             /* rational data type */
#ifdef DEBUG
    printf("fetch_ptr() : Rational array of length %d\n", IFD->length);
#endif
    fseek(fpo, IFD->value_offset, 0);
    rc = fread(inbuf, IFD->length * 2, sizeof(unsigned long), fpo);
    if (rc == 0) {
      sprintf(cbuf, "Failed reading IFD data from data file");
      ErrorMsg(cbuf);
      return(FALSE);
    }
    break;
  default:
    sprintf(cbuf, "Unsupported data type (%d)", IFD->type);
    ErrorMsg(cbuf);
    return(FALSE);
    break;
  }

  if (blab)
    echo_tag(IFD, inbuf);

  return(TRUE);
}

Boolean GetTiffFile(char *fname, double **pixels, int *heightp, int *widthp,
		    double *min, double *max, Colormap cmap, XColor *Cells,
		    Boolean *TrueColorFlag, Boolean verbose)
{
  struct tiff_header_struct tiff_header;
  struct IFD_entry IFD;
  XColor *color, LocalCells[256];
  FILE *fpo;
  unsigned short pm_int;
  unsigned long *strips_buf, *byte_cnt_buf, rows_per_strip, rows_in_strip;
  long il, strips_l, byte_cnt_l, cnt;
  char *inbuf, byte, cbuf[512], black_pixel, white_pixel;
  int gs, bs, offset, i, j, k, m,planes,samples_per_pixel,rc,tiff_header_size,
  height, width;
  double *dp;
  Boolean color_map_defined, ColorNotDefined, FlagError;

  /* flag image data only */
  ComplexDataSet = FALSE;

  pm_int = 1;
  planes = 8;
  samples_per_pixel = 1;
  color_map_defined = FALSE;
  FlagError = FALSE;
  *TrueColorFlag = FALSE;

  /* KLUDGE: fool compiler to adjust structure size */
  tiff_header_size = sizeof(struct tiff_header_struct) - 2;

  /* open tiff input file */
  if ((fpo = fopen(fname, "rb")) == NULL) {
    sprintf(cbuf, "Could not open input file: %s", fname);
    ErrorMsg(cbuf);
    return(FALSE);
  }

  /* read tiff file header from disk */
  rc = fread(&tiff_header, 1, tiff_header_size, fpo);
  if (rc == 0) {
    sprintf(cbuf, "Failed reading tiff header from %s", fname);
    ErrorMsg(cbuf);
    return(FALSE);
  }

  /* read IFD count and blocks from disk */  
  fseek(fpo, tiff_header.IFD_offset, 0);
  fread(&tiff_header.IFD_entry_count, 1, sizeof(tiff_header.IFD_entry_count),
	fpo);

  /* print header contents */
  if (verbose) {
    printf("TIFF FILE HEADER:\n");
    printf("  byte order:    0x%x\n", tiff_header.byte_order);
    printf("  version:         %d\n", tiff_header.version);
    printf("  IFD offset:      %ld\n", tiff_header.IFD_offset);
    printf("  IFD entry count: %ld\n\n", tiff_header.IFD_entry_count);
  }

  /* loop to read IFD blocks */
  for (i = 0; i < tiff_header.IFD_entry_count; i++) {
    /* read IFD */
    fseek(fpo, tiff_header.IFD_offset + 2 + i*sizeof(struct IFD_entry), 0);
    rc = fread(&IFD, 1, sizeof(struct IFD_entry), fpo);
    if (rc == 0) {
      sprintf(cbuf,"Failed reading tiff IFD block %d from %s",i+1, fname);
      ErrorMsg(cbuf);
      return(FALSE);
    }
    
    if (IFD.length > 0) {
      /* allocate storage */
      if ((inbuf = (char *) XvgMM_Alloc((void *) inbuf,
					IFD.length *2 *sizeof(double)))
	  == NULL) {
	sprintf(cbuf, "GetTiffFile(): Can't allocate for inbuf");
	ErrorMsg(cbuf);
	return(FALSE);
      }
      
      /* decode IFD information */
      switch (IFD.tag) {
      case 256:                                              /* image width */
	if (fetch_ptr(&IFD, fpo, inbuf, verbose) == FALSE)
	  return(FALSE);
	if (IFD.type == 3)
	  width = *(unsigned short *) inbuf;
	else
	  width = *(long *) inbuf;
#ifdef DEBUG
	printf("GetTiffFile() => Image width tag : %d\n", width);
#endif
	break;
      case 257:                                             /* image height */
	if (fetch_ptr(&IFD, fpo, inbuf, verbose) == FALSE)
	  return(FALSE);
	if (IFD.type == 3)
	  height = *(unsigned short *) inbuf;
	else
	  height = *(long *) inbuf;
#ifdef DEBUG
	printf("GetTiffFile() => Image height tag : %d\n", height);
#endif
	break;
      case 258:                                          /* bits per sample */
	if (fetch_ptr(&IFD, fpo, inbuf, verbose) == FALSE)
	  return(FALSE);
	if (IFD.length == 1) {
	  if ((*(unsigned short *) inbuf) == 1)
	    planes = 1;
	  else if ((*(unsigned short *) inbuf) == 8)
	    planes = 8;
	  else {
	    sprintf(cbuf, "Unsupported number of bits per sample (%d) in %s",
		    *(unsigned short *) inbuf, fname);
	    ErrorMsg(cbuf);
	    return(FALSE);
	  }
	}
	else if (IFD.length == 3) {
	  if ((((unsigned short *) inbuf)[0] == 8)
	      && (((unsigned short *) inbuf)[1] == 8)
	      && (((unsigned short *) inbuf)[2] == 8))
	    planes = 8;
	  else {
	    sprintf(cbuf, "Unsupported bits/sample IFD field in %s", fname);
	    ErrorMsg(cbuf);
	    return(FALSE);
	  }
	}
	else {
	  sprintf(cbuf, "Unsupported length of bits/sample (%d) in %s",
		  IFD.length, fname);
	  ErrorMsg(cbuf);
	  return(FALSE);
	}
#ifdef DEBUG
	printf("GetTiffFile() => Bits per sample tag : %d\n", planes);
#endif
	break;
      case 259:                                              /* compression */
	if (fetch_ptr(&IFD, fpo, inbuf, verbose) == FALSE)
	  return(FALSE);
	if (*((unsigned short *) inbuf) != 1) {
	  sprintf(cbuf, "Unsupported Compression (%d) in %s",
		  *((unsigned short *) inbuf), fname);
	  ErrorMsg(cbuf);
	  return(FALSE);
	}
#ifdef DEBUG
	printf("GetTiffFile() => Compression tag\n");
#endif
	break;
      case 262:                               /* photometric interpretation */
	if (fetch_ptr(&IFD, fpo, inbuf, verbose) == FALSE)
	  return(FALSE);
	pm_int = *((unsigned short *) inbuf);
	if (pm_int >= 4) {
	  sprintf(cbuf, "Unsupported Photometric Interpretation (%d) in %s\n",
		  *((unsigned short *) inbuf), fname);
	  ErrorMsg(cbuf);
	  return(FALSE);
	}
#ifdef DEBUG
	printf("GetTiffFile() => Photometric interpretation tag : %d\n", 
	       pm_int);
#endif
	break;
      case 270:                                   /* image information flag */
	if (fetch_ptr(&IFD, fpo, inbuf, verbose) == FALSE)
	  return(FALSE);
#ifdef DEBUG
	printf("GetTiffFile() => Image information tag\n");
#endif
	break;
      case 273:                                            /* strip offsets */
	strips_l = IFD.length;
	if ((strips_buf
	     = (unsigned long *) XvgMM_Alloc((void *) strips_buf,
					     strips_l * sizeof(unsigned long)))
	    == NULL) {
	  sprintf(cbuf, "GetTiffFile(): Can't not allocate for strips_buf");
	  ErrorMsg(cbuf);
	  return(FALSE);
	}
	if (IFD.type == 4) {
	  if (fetch_ptr(&IFD, fpo, (char *) strips_buf, verbose) == FALSE)
	    return(FALSE);
	}
	else {
	  if (fetch_ptr(&IFD, fpo, inbuf, verbose) == FALSE)
	    return(FALSE);
	  for (j = 0; j < IFD.length; j++)
	    strips_buf[j] = ((unsigned short *) inbuf)[j];
	}
#ifdef DEBUG
	if (strips_l == 1)
	  printf("GetTiffFile() => Strip offsets tag : val = %d\n",
		 strips_buf[0]);
	else
	  printf("GetTiffFile() => Strip offsets tag : length = %d\n",
		 strips_l);
#endif
	break;
      case 277:                                        /* samples per pixel */
	if (fetch_ptr(&IFD, fpo, inbuf, verbose) == FALSE)
	  return(FALSE);
	samples_per_pixel = *(unsigned short *) inbuf;
#ifdef DEBUG
	printf("GetTiffFile() => Samples per pixel tag : %d\n",
	       samples_per_pixel);
#endif
	break;
      case 278:                                           /* rows per strip */
	if (fetch_ptr(&IFD, fpo, inbuf, verbose) == FALSE)
	  return(FALSE);
	if (IFD.type == 3)
	  rows_per_strip = *(unsigned short *) inbuf;
	else
	  rows_per_strip = *(long *) inbuf;
#ifdef DEBUG
	printf("GetTiffFile() => Rows per strip tag : %d\n", rows_per_strip);
#endif
	break;
      case 279:                                        /* strip byte counts */
	byte_cnt_l = IFD.length;
	if ((byte_cnt_buf =
	     (unsigned long *) XvgMM_Alloc((void *) byte_cnt_buf,
					   byte_cnt_l * sizeof(unsigned long)))
	    == NULL) {
	  sprintf(cbuf, "GetTiffFile(): Can't allocate for byte_cnt_buf");
	  ErrorMsg(cbuf);
	  return(FALSE);
	}
	else {
	  if (IFD.type == 4) {
	    if (fetch_ptr(&IFD, fpo, (char *) byte_cnt_buf, verbose) == FALSE)
	      return(FALSE);
	  }
	  else {
	    if (fetch_ptr(&IFD, fpo, inbuf, verbose) == FALSE)
	      return(FALSE);
	    for (j = 0; j < IFD.length; j++)
	      byte_cnt_buf[j] = ((unsigned short *) inbuf)[j];
	  }
#ifdef DEBUG
	  if (IFD.length == 1)
	    printf("GetTiffFile() => Strip byte counts tag, val=%d\n",
		   byte_cnt_buf[0]);
	  else
	    printf("GetTiffFile() => Strip byte counts tag, length=%d\n",
		   IFD.length);
#endif
	}
	break;
      case 284:                                     /* planar configuration */
	if (fetch_ptr(&IFD, fpo, inbuf, verbose) == FALSE)
	  return(FALSE);
#ifdef DEBUG
	printf("GetTiffFile() => Planar configuration tag\n");
#endif
	break;
      case 320:                                                /* color map */
	if (fetch_ptr(&IFD, fpo, inbuf, verbose) == FALSE)
	  return(FALSE);
	/* fetch colors */
	for (j=0; j < IFD.length/3; j++) {
	  LocalCells[j].flags = 0;
	  LocalCells[j].red = ((unsigned short *) inbuf)[j];
	  LocalCells[j].green = ((unsigned short *) inbuf)[j + 256];
	  LocalCells[j].blue = ((unsigned short *) inbuf)[j + 512];
	}
	color_map_defined = TRUE;
	*TrueColorFlag = TRUE;
#ifdef DEBUG
	printf("GetTiffFile() => Colormap tag, length = %d\n", IFD.length);
#endif
	break;
      default:                         /* unsupported TAG, display contents */
	if (verbose)
	  printf("   - UNSUPPORTED - ");
	if (fetch_ptr(&IFD, fpo, inbuf, verbose) == FALSE)
	  return(FALSE);
	break;
      }
      if (inbuf != NULL)
	XvgMM_Free((void **)&(inbuf));
    }
  }
  
  /* allocate storage if required */
  *heightp = height;
  *widthp = width;
  if ((*pixels = (double *) XvgMM_Alloc((void *) *pixels,
					*widthp * *heightp * sizeof(double)))
      == NULL) {
    sprintf(cbuf, "GetTiffFile(): Can't not allocate storage for pixels");
    ErrorMsg(cbuf);
    return(FALSE);
#ifdef DEBUG
    printf("GetTiffFile() : %d doubles of storage allocated\n",
	   (*heightp) * (*widthp));
#endif
  }
  
  /* load image data from strips into buffer */
  switch (pm_int) {
  case 1:                                           /* bilevel or grayscale */
    /* check parms */
    if (samples_per_pixel != 1) {
      ErrorMsg("PM_INT=1 and SAMPLES_PER_PIXEL!=1 TIFF images not supported!");
      return(FALSE);
    }
    if (planes == 1) {                                   /* 1 bit per pixel */
#ifdef DEBUG
      printf("GetTiffFile() : 1 bit/pixel decoding\n");
#endif
      /* set black and white pixel values */
      if (pm_int == 0) {
	white_pixel = 0x00;
	black_pixel = 0xff;
      }
      else {
	white_pixel = 0xff;
	black_pixel = 0x00;
      }
      
      /* loop for strips */
      for (il = 0, cnt = 0; il < strips_l; il++) {
	/* input strip */
	fseek(fpo, strips_buf[il], 0);
	if ((inbuf = (char *) XvgMM_Alloc((void *) inbuf,
					  byte_cnt_buf[il])) == NULL) {
	  sprintf(cbuf, "GetTiffFile(): can't allocate for inbuf");
	  ErrorMsg(cbuf);
	  return(FALSE);
	}
	rc = fread(inbuf, 1, byte_cnt_buf[il], fpo);
	if (rc == 0) {
	  sprintf(cbuf, "Failed reading strip %dl from %s", il, fname);
	  ErrorMsg(cbuf);
	  return(FALSE);
	}
	
	/* check for possible shorter last strip */
	if (il != (strips_l - 1))
	  rows_in_strip = rows_per_strip;
	else 
	  rows_in_strip = *heightp - (strips_l - 1)*rows_per_strip;
	
	/* convert strip from 1 bit to 8 bits per pixel and store */
	for (j = 0, m = 0; m < rows_in_strip; m++)
	  for (i = 0; i < *widthp;) 
	    for (k=0, byte = inbuf[j++]; (k < 8) && (i < *widthp); i++, k++) {
	      if (byte & 0x80)
		(*pixels)[cnt++] = white_pixel;
	      else
		(*pixels)[cnt++] = black_pixel;
	      byte = byte << 1;
	    }
	XvgMM_Free((void **)&(inbuf));
      }
    }
    else {                                              /* 8 bits per pixel */
#ifdef DEBUG
      printf("GetTiffFile() : 8 bits/pixel grayscale decoding\n");
#endif
      for (il = 0, cnt = 0, dp = *pixels; il < strips_l; il++) {
	fseek(fpo, strips_buf[il], 0);
#ifdef DEBUG
      printf("GetTiffFile() : Allocating stip buffer (%d bytes)\n",
	     byte_cnt_buf[il]);
#endif
	if ((inbuf = (char *) XvgMM_Alloc((void *) inbuf,
					  byte_cnt_buf[il])) == NULL) {
	  sprintf(cbuf, "GetTiffFile(): Can't allocate storage for inbuf");
	  ErrorMsg(cbuf);
	  return(FALSE);
	}
#ifdef DEBUG
      printf("GetTiffFile() : Loading strip data\n");
#endif
	if ((rc = fread(inbuf, 1, byte_cnt_buf[il], fpo)) == 0) {
	  sprintf(cbuf, "Failed reading strip %dl from %s", il, fname);
	  ErrorMsg(cbuf);
	  return(FALSE);
	}
#ifdef DEBUG
      printf("GetTiffFile() : Converting byte data to double (%d)\n",
	     byte_cnt_buf[il]);
#endif
	for (j = 0; j < byte_cnt_buf[il]; j++)
	  *dp++ = inbuf[j];

	/* update counter */
	cnt += byte_cnt_buf[il];
	XvgMM_Free((void **)&(inbuf));
      }
    }
    /* install current colors */
    if (PagedCmap)
      XStoreColors(UxDisplay, cmap, CurrentCells, NPAGEDLUTCELLS);
    break;
  case 2:                                                       /* RGB image */
    /* check parms */
    if (samples_per_pixel != 3) {
      ErrorMsg("PM_INT=2 and SAMPLES_PER_PIXEL!=3 TIFF images not supported!");
      return(FALSE);
    }
#ifdef DEBUG
      printf("GetTiffFile() : 24 bits/pixel decoding (%d strips)\n", strips_l);
#endif
    /* fetch strip data */
    for (il = 0, k = 0, dp = *pixels; il < strips_l; il++) {
      fseek(fpo, strips_buf[il], 0);
      if ((inbuf = (char *) XvgMM_Alloc((void *) inbuf,
					byte_cnt_buf[il])) == NULL) {
	sprintf(cbuf, "GetTiffFile(): Can't allocate storage for inbuf");
	ErrorMsg(cbuf);
	return(FALSE);
      }
      rc = fread(inbuf, 1, byte_cnt_buf[il], fpo);
      if (rc == 0) {
	sprintf(cbuf, "Failed reading strip %dl from %s", il, fname);
	ErrorMsg(cbuf);
	return(FALSE);
      }
      /* convert to single color index, double precision */
      for (j = 0; j < byte_cnt_buf[il]; j+=3) {
	/* look up this RGB color to see if its defined */
	for (i = 0, ColorNotDefined=TRUE; (i < k) && (ColorNotDefined);)
	  if (((Cells[i].red >> 8) == inbuf[j])
	      && ((Cells[i].green >> 8) == inbuf[j+1])
	      && ((Cells[i].blue >> 8) == inbuf[j+2]))
	    ColorNotDefined = FALSE;
	  else
	    i++;
	/* if color has not been defined, then define it */
	if ((ColorNotDefined) && (k < NPAGEDLUTCELLS)) {
	  Cells[k].red = inbuf[j] << 8;
	  Cells[k].green = inbuf[j+1] << 8;
	  Cells[k].blue = inbuf[j+2] << 8;
#ifdef DEBUG
	  printf("GetTiffFile() : color 0x%04x%04x%04x loaded at index %d\n",
		 Cells[k].red, Cells[k].green, Cells[k].blue, Cells[k].pixel);
#endif
	  k++;
	}
	/* load color index into the image buffer */
	*dp++ = Cells[i].pixel;
      }
      XvgMM_Free((void **)&(inbuf));
    }
    /* if user lut is currently active, then load it */
    if (PagedCmap) {
      XStoreColors(UxDisplay, cmap, Cells, NPAGEDLUTCELLS);
#ifdef DEBUG
      printf("GetTiffFile() : Installed loaded lookup table\n");
#endif
    }
    
    /* print banner if number of colors exceeded */
    if (k >= NPAGEDLUTCELLS)
      InfoMsg("GetTiffFile() : Maximum number of colors exceeded");
#ifdef DEBUG
    printf("GetTiffFile() : %d color RGB file decoded\n", k);
#endif

    /* set color image flag */
    *TrueColorFlag = TRUE;
    break;
  case 3:                                             /* palette color image */
    /* check parms */
    if (samples_per_pixel != 1) {
      ErrorMsg("PM_INT=3 and SAMPLES_PER_PIXEL!=1 TIFF images not supported!");
      return(FALSE);
    }
    if (color_map_defined == FALSE) {
      ErrorMsg("Undefined colormap with PM_INT=3 TIFF images not supported!");
      return(FALSE);
    }
#ifdef DEBUG
      printf("GetTiffFile() : 8 bits/pixel color decoding\n");
#endif
    /* fetch strip data */
    for (il = 0, cnt = 0, dp = *pixels, k = 0; il < strips_l; il++) {
      fseek(fpo, strips_buf[il], 0);
      if ((inbuf = (char *) XvgMM_Alloc((void *) inbuf,
					byte_cnt_buf[il])) == NULL) {
	sprintf(cbuf, "GetTiffFile(): Can't allocate storage for inbuf");
	ErrorMsg(cbuf);
	return(FALSE);
      }
      rc = fread(inbuf, 1, byte_cnt_buf[il], fpo);
      if (rc == 0) {
	sprintf(cbuf, "Failed reading strip %dl from %s", il, fname);
	ErrorMsg(cbuf);
	return(FALSE);
      }
      /* convert data to double, mark used colors and load them into the lut */
      for (j = 0; j < byte_cnt_buf[il]; j++) {
	/* if the color is new, load it into the Cells array */
	if (LocalCells[inbuf[j]].flags == 0) {
	  if (k < NPAGEDLUTCELLS) {
	    Cells[k].red = LocalCells[inbuf[j]].red;
	    Cells[k].green = LocalCells[inbuf[j]].green;
	    Cells[k].blue = LocalCells[inbuf[j]].blue;
	    LocalCells[inbuf[j]].flags = k;
	  }
	  else
	    LocalCells[inbuf[j]].flags = NPAGEDLUTCELLS-1;
	  k++;
	}
	*dp++ = Cells[LocalCells[inbuf[j]].flags].pixel;
      }
      /* update counter */
      cnt += byte_cnt_buf[il];
      XvgMM_Free((void **)&(inbuf));
    }
    /* print banner if number of colors exceeded */
    if (k > NPAGEDLUTCELLS) {
      sprintf(cbuf, "GetTiffFile() : Max number of colors exceeded (%d / %d)",
	      k, NPAGEDLUTCELLS);
      InfoMsg(cbuf);
    }
#ifdef DEBUG
    printf("GetTiffFile() : %d color 8 bit/pixel file decoded\n", k);
#endif

    /* store colors */
    if (PagedCmap)
      XStoreColors(UxDisplay, cmap, Cells, NPAGEDLUTCELLS);
    break;
  }
  
  /* find min & max */
  DynamicRange(*pixels, (*heightp) * (*widthp), max, min);
  
  /* close input file and deallocate memory */
  XvgMM_Free((void **)&(strips_buf));
  XvgMM_Free((void **)&(byte_cnt_buf));

  fclose(fpo);
  return(TRUE);
}

Boolean WriteTiffFileGreyLevel(double *pixels, int xstart, int ystart,
			       int height, int width, char *fname, char *info)
{
  FILE *fpo;
  struct tiff_header_struct tiff_header;
  struct IFD_struct IFD;
  double max, min, ScaleInv, val;
  char cbuf[512], *p;
  int tiff_header_size, info_len, i, j, k;
  
  /* write out info if required */
  if (VerboseFlag)
    printf("WriteTiffFileGreyLevel() : writing to file %s (height:%d, width:%d, xstart:%d, ystart:%d)\n", fname, height, width, xstart, ystart);
  
  /* find length of information string */
  if (info != NULL) {
    for (info_len = 0; info[info_len] != '\0'; info_len++);
    info_len++;
  }
  else
    info_len = 0;
  
  /* KLUDGE: fool compiler to adjust structure size */
  tiff_header_size = sizeof(struct tiff_header_struct) - 2;
  
  /* open tiff output file */
  if ((fpo = fopen(fname, "wb")) == NULL) {
    sprintf(cbuf, "WriteTiffFileGreyLevel() : Could not open output file: %s",
	    fname);
    ErrorMsg(cbuf);
    return(FALSE);
  }
  
  /* fill tiff file header fields and write to the file */
  tiff_header.byte_order = HL;
  tiff_header.version = 42;
  tiff_header.IFD_offset = 8;
  tiff_header.IFD_entry_count = 9;
  if (fwrite(&tiff_header, tiff_header_size, 1, fpo) != 1) {
    ErrorMsg("WriteTiffFileGreyLevel() : Failed writing tiff header");
    return(FALSE);
  }
  
  /* fill IFD entries and write to file */
  IFD.image_width.tag = 256;
  IFD.image_width.type = 3;
  IFD.image_width.length = 1;
  IFD.image_width.value_offset = width << 16;
  
  IFD.image_length.tag = 257;
  IFD.image_length.type = 3;
  IFD.image_length.length = 1;
  IFD.image_length.value_offset = height << 16;
  
  IFD.bits_per_sample.tag = 258;
  IFD.bits_per_sample.type = 3;
  IFD.bits_per_sample.length = 1;
  IFD.bits_per_sample.value_offset = 8 << 16;
  
  IFD.photometric_int.tag = 262;
  IFD.photometric_int.type = 3;
  IFD.photometric_int.length = 1;
  IFD.photometric_int.value_offset = 1 << 16;
  
  IFD.image_description.tag = 270;
  IFD.image_description.type = 2;
  IFD.image_description.length = info_len;
  IFD.image_description.value_offset = tiff_header_size
    +sizeof(struct IFD_struct);
  
  IFD.strip_offsets.tag = 273;
  IFD.strip_offsets.type = 4;
  IFD.strip_offsets.length = 1;
  IFD.strip_offsets.value_offset = tiff_header_size
    +sizeof(struct IFD_struct)+info_len;
  
  IFD.samples_per_pixel.tag = 277;
  IFD.samples_per_pixel.type = 3;
  IFD.samples_per_pixel.length = 1;
  IFD.samples_per_pixel.value_offset = 1 << 16;
  
  IFD.rows_per_strip.tag = 278;
  IFD.rows_per_strip.type = 4;
  IFD.rows_per_strip.length = 1;
  IFD.rows_per_strip.value_offset = height;
  
  IFD.strip_byte_counts.tag = 279;
  IFD.strip_byte_counts.type = 4;
  IFD.strip_byte_counts.length = 1;
  IFD.strip_byte_counts.value_offset = height*width;

  IFD.next_IFD_offset = 0;

  /* write IFD blocks */
  if (fwrite(&IFD, sizeof(struct IFD_struct), 1, fpo) != 1) {
    ErrorMsg("WriteTiffFileGreyLevel() : Failed writing tiff IFD");
    return(FALSE);
  }
  
  /* write image information data to the output file */
  if (info != NULL) {
    if (fwrite(info, 1, info_len, fpo) != info_len) {
      ErrorMsg("WriteTiffFileGreyLevel() : Failed writing info");
      return(FALSE);
    }
  }

  /* check that "DoubleBuffer" is defined */
  if ((DoubleBuffer
       = (double *) XvgMM_Alloc((void *) DoubleBuffer,
				height*width*sizeof(double)))
      == NULL) {
    sprintf(cbuf, "WriteTiffFileGreyLevel() : Couldn't alloc memory (%dx%d)",
	    width, height);
    ErrorMsg(cbuf);
    return(FALSE);
  }

  /* crop if required */
  for (j = ystart, k = 0; j < (ystart + height); j++)
    for (i = xstart; i < (xstart + width); i++, k++)
      DoubleBuffer[k] = pixels[j*ImgWidth + i];

  /* convert the data to byte form */
  DynamicRange(DoubleBuffer, height*width, &max, &min);
  if (CurrentCells == GammaGreyLevelCells)
    for (i = 0, ScaleInv = 255.0/(max - min), p = (char *) DoubleBuffer;
	 i < height*width; i++) {
      val = (DoubleBuffer[i] - min)*ScaleInv;
      p[i] = sqrt(val/255.0)*255.0*GcFactor + val*(1.0-GcFactor);
    }
  else
    for (i = 0, ScaleInv = 255.0/(max - min), p = (char *) DoubleBuffer;
	 i < height*width; i++)
      p[i] = (DoubleBuffer[i] - min)*ScaleInv;

  
  /* write image data to the output file */
  if (fwrite(DoubleBuffer, 1, width*height, fpo) != (height*width)) {
    ErrorMsg("WriteTiffFileGreyLevel() : Failed writing image data");
    return(FALSE);
  }

  /* close and return */
  fclose(fpo);
  return(TRUE);
}

Boolean WriteTiffFileColor(XImage *image, int xstart, int ystart,
			   int height, int width, char *fname,
			   char *info, XColor *Cells, int dfactor)
{
  XColor colors[256];
  FILE *fpo;
  struct tiff_header_struct tiff_header;
  struct IFD_struct IFD;
  char cbuf[512], table[256], *p, *pixels;
  int tiff_header_size, info_len, i, j, k, dx, dy, pix, bytes_per_line;


#ifdef DEBUG
  printf("WriteTiffFileColor() : Height = %d, Width = %d, dfactor = %d\n",
	 height, width, dfactor);
  printf("   iwidth:%d, iheight:%d, ixoffset:%d, ibytesperline:%d\n",
	 image->width, image->height, image->xoffset, image->bytes_per_line);
#endif

  /* set local variables */
  bytes_per_line = image->bytes_per_line;
  pixels = image->data;

  /* find length of information string */
  if (info != NULL) {
    for (info_len = 0; info[info_len] != '\0'; info_len++);
    info_len++;
  }
  else
    info_len = 0;

  /* KLUDGE: fool compiler to adjust structure size */
  tiff_header_size = sizeof(struct tiff_header_struct) - 2;

  /* open tiff output file */
  fpo = fopen(fname, "wb");
  if (fpo == NULL) {
    sprintf(cbuf, "WriteTiffFileColor() : Could not open output file: %s",
	    fname);
    ErrorMsg(cbuf);
    return(FALSE);
  }

  /* fill tiff file header fields and write to the file */
  tiff_header.byte_order = HL;
  tiff_header.version = 42;
  tiff_header.IFD_offset = 8;
  tiff_header.IFD_entry_count = 9;
  if (fwrite(&tiff_header, tiff_header_size, 1, fpo) != 1) {
    ErrorMsg("WriteTiffFileColor() : Failed writing tiff header");
    return(FALSE);
  }
  
  /* fill IFD entries and write to file */
  IFD.image_width.tag = 256;
  IFD.image_width.type = 3;
  IFD.image_width.length = 1;
  IFD.image_width.value_offset = (width/dfactor) << 16;
  
  IFD.image_length.tag = 257;
  IFD.image_length.type = 3;
  IFD.image_length.length = 1;
  IFD.image_length.value_offset = (height/dfactor) << 16;
  
  IFD.bits_per_sample.tag = 258;
  IFD.bits_per_sample.type = 3;
  IFD.bits_per_sample.length = 1;
  IFD.bits_per_sample.value_offset = 8 << 16;
  
  IFD.photometric_int.tag = 262;
  IFD.photometric_int.type = 3;
  IFD.photometric_int.length = 1;
  IFD.photometric_int.value_offset = 2 << 16;
  
  IFD.image_description.tag = 270;
  IFD.image_description.type = 2;
  IFD.image_description.length = info_len;
  IFD.image_description.value_offset = tiff_header_size
    +sizeof(struct IFD_struct);
  
  IFD.strip_offsets.tag = 273;
  IFD.strip_offsets.type = 4;
  IFD.strip_offsets.length = 1;
  IFD.strip_offsets.value_offset = tiff_header_size
    +sizeof(struct IFD_struct)+info_len;
  
  IFD.samples_per_pixel.tag = 277;
  IFD.samples_per_pixel.type = 3;
  IFD.samples_per_pixel.length = 1;
  IFD.samples_per_pixel.value_offset = 3 << 16;
  
  IFD.rows_per_strip.tag = 278;
  IFD.rows_per_strip.type = 4;
  IFD.rows_per_strip.length = 1;
  IFD.rows_per_strip.value_offset = (height/dfactor);
  
  IFD.strip_byte_counts.tag = 279;
  IFD.strip_byte_counts.type = 4;
  IFD.strip_byte_counts.length = 1;
  IFD.strip_byte_counts.value_offset = height/dfactor * width/dfactor * 3;

  IFD.next_IFD_offset = 0;

  /* write IFD blocks */
  if (fwrite(&IFD, sizeof(struct IFD_struct), 1, fpo) != 1) {
    ErrorMsg("WriteTiffFileColor(): Failed writing tiff IFD");
    return(FALSE);
  }
  
  /* write image information data to the output file */
  if (info != NULL) {
    if (fwrite(info, 1, info_len, fpo) != info_len) {
      ErrorMsg("WriteTiffFileColor() : Failed writing information data");
      return(FALSE);
    }
  }

  /* build color conversion table and flag non-"gray level" colors */
  for (i = 0; i < 256; i++) {
    colors[Cells[i].pixel] = Cells[i];
    if ((Cells[i].red ==  Cells[i].green) && (Cells[i].red ==  Cells[i].blue))
      colors[Cells[i].pixel].flags = FALSE;
    else
      colors[Cells[i].pixel].flags = TRUE;
  }

  /* convert the data to 24bit color */
  if (dfactor == 1) {
    /* no decimation */
    if ((p = (char *) XvgMM_Alloc((void *) p, height*width*3)) == NULL) {
      ErrorMsg("WriteTiffFileColor(): Failed allocating for color conversion");
      return(FALSE);
    }
    for (i = ystart, k = 0; i < (height+ystart); i++)
      for (j = xstart; j < (width+xstart); j++) {
	p[k++] = colors[pixels[i*bytes_per_line + j]].red >> 8;
	p[k++] = colors[pixels[i*bytes_per_line + j]].green >> 8;
	p[k++] = colors[pixels[i*bytes_per_line + j]].blue >> 8; 
      }
  }
  else {
    /* decimation, priority to non-"gray level" pixels */
    if ((p = (char *) XvgMM_Alloc((void *) p,
				  (height/dfactor)*(width/dfactor)*3))
	== NULL) {
      ErrorMsg("WriteTiffFileColor(): Failed allocating for color conversion");
      return(FALSE);
    }
    for (i = ystart, k = 0; i < (height+ystart); i+=dfactor)
      for (j = xstart; j < (width+xstart); j +=dfactor) {
	for (dy = 0, pix = pixels[i*bytes_per_line + j];
	     dy < dfactor; dy++) {
	  for (dx = 0; dx < dfactor; dx++)
	    if (colors[pixels[(i+dy)*bytes_per_line + j+dx]].flags)
	      pix = pixels[(i+dy)*bytes_per_line + j+dx];
	}
	p[k++] = colors[pix].red >> 8;
	p[k++] = colors[pix].green >> 8;
	p[k++] = colors[pix].blue >> 8; 
      }
  }

  /* write image data to the output file */
  for (i = 0; i < (height/dfactor); i++) {
    if (fwrite(&(p[i*(width/dfactor)*3]), 1, (width/dfactor) * 3, fpo)
	!= ((width/dfactor)*3)) {
      sprintf(cbuf, "WriteTiffFileColor() : Failed writing data at line %d",
	      i);
      ErrorMsg(cbuf);
      fclose(fpo);
      XvgMM_Free((void **)&(p));
      return(FALSE);
    }
  }
  
  fclose(fpo);
  XvgMM_Free((void **)&(p));
  return(TRUE);
}

Boolean WriteGLTiffFile(char *fname, int width, int height,
			int *pixels, int *buffer)
{
  FILE *fpo;
  struct tiff_header_struct tiff_header;
  struct IFD_struct IFD;
  char cbuf[512], *p;
  int tiff_header_size, info_len, i, j, k, *q;

  /* set local variables */
  info_len = 0;

  /* KLUDGE: fool compiler to adjust structure size */
  tiff_header_size = sizeof(struct tiff_header_struct) - 2;

  /* open tiff output file */
  fpo = fopen(fname, "wb");
  if (fpo == NULL) {
    sprintf(cbuf, "WriteGLTiffFile() : could not open output file: %s", fname);
    ErrorMsg(cbuf);
    return(FALSE);
  }

  /* fill tiff file header fields and write to the file */
  tiff_header.byte_order = HL;
  tiff_header.version = 42;
  tiff_header.IFD_offset = 8;
  tiff_header.IFD_entry_count = 9;
  if (fwrite(&tiff_header, tiff_header_size, 1, fpo) == 0) {
    sprintf(cbuf, "WriteGLTiffFile() : failed writing tiff header to file %s",
	    fname);
    return(FALSE);
  }
  
  /* fill IFD entries and write to file */
  IFD.image_width.tag = 256;
  IFD.image_width.type = 3;
  IFD.image_width.length = 1;
  IFD.image_width.value_offset = width << 16;
  
  IFD.image_length.tag = 257;
  IFD.image_length.type = 3;
  IFD.image_length.length = 1;
  IFD.image_length.value_offset = height << 16;
  
  IFD.bits_per_sample.tag = 258;
  IFD.bits_per_sample.type = 3;
  IFD.bits_per_sample.length = 1;
  IFD.bits_per_sample.value_offset = 8 << 16;
  
  IFD.photometric_int.tag = 262;
  IFD.photometric_int.type = 3;
  IFD.photometric_int.length = 1;
  IFD.photometric_int.value_offset = 2 << 16;
  
  IFD.image_description.tag = 270;
  IFD.image_description.type = 2;
  IFD.image_description.length = info_len;
  IFD.image_description.value_offset = tiff_header_size
    +sizeof(struct IFD_struct);
  
  IFD.strip_offsets.tag = 273;
  IFD.strip_offsets.type = 4;
  IFD.strip_offsets.length = 1;
  IFD.strip_offsets.value_offset = tiff_header_size
    +sizeof(struct IFD_struct)+info_len;
  
  IFD.samples_per_pixel.tag = 277;
  IFD.samples_per_pixel.type = 3;
  IFD.samples_per_pixel.length = 1;
  IFD.samples_per_pixel.value_offset = 3 << 16;
  
  IFD.rows_per_strip.tag = 278;
  IFD.rows_per_strip.type = 4;
  IFD.rows_per_strip.length = 1;
  IFD.rows_per_strip.value_offset = height;
  
  IFD.strip_byte_counts.tag = 279;
  IFD.strip_byte_counts.type = 4;
  IFD.strip_byte_counts.length = 1;
  IFD.strip_byte_counts.value_offset = height * width * 3;

  IFD.next_IFD_offset = 0;

  /* write IFD blocks */
  if (fwrite(&IFD, sizeof(struct IFD_struct), 1, fpo) == 0) {
    sprintf(cbuf, "WriteGLTiffFile() : failed writing tiff IFD to file %s",
	    fname);
    ErrorMsg(cbuf);
    return(FALSE);
  }

  /* pack the data from 32 bits to 24 bits, flip the image in y */
  for (j = height-1, p = (char *) buffer; j >= 0; j--)
    for (i = 0, q = &(pixels[j*width]); i < width; i++, q++) {
      *p++ = (*q & 0x000000ff);
      *p++ = (*q & 0x0000ff00) >> 8;
      *p++ = (*q & 0x00ff0000) >> 16;
    }
  
  /* write image data to the output file */
  if (fwrite(buffer, 3, width*height, fpo) != (width*height)) {
    sprintf(cbuf, "WriteGLTiffFile() : failed writing image data to file %s",
	    fname);
    ErrorMsg(cbuf);
    return(FALSE);
  }
  
  /* close file, exit */
  fclose(fpo);
  return(FALSE);
}






