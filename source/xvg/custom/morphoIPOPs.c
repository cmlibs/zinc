/***********************************************************************
*
*  Name:          morphoIPOPs.c
*
*  Author:        Paul Charette
*
*  Last Modified: 12 August 1993
*
*  Purpose:       Utility routines for morphological image processing
*                   Contour()
*                   Dilation()
*                   DilationFFT()
*                   Disk()
*                   Erosion()
*                   ErosionFFT()
*
***********************************************************************/
#include <stdio.h>
#ifdef UIL_CODE
#include "UxXt.h"
#else
#include "UxLib.h"
#endif
#include <fcntl.h>
#include <math.h>
#include <memory.h>
#include <search.h>
#include <values.h>
#include "XvgGlobals.h"

#if defined (NAG)
/***************************  Contour  *************************************/
/*                                                                         */
/*  function: trace the outside boundary of an image by morphological      */
/*            closing of all regions and keeping only the largest.         */
/*                                                                         */
/***************************************************************************/
void Contour(double *inpixels, double *outpixels, int height, int width,
	     void *ParmPtrs[])
{
  CONTOUR_ELEMENT contour[CONTOURNMAX];
  int i, j, k, MaxI, MaxJ, cnt, ccnt, CentroidX, CentroidY, cx, cy;
  double *parms[1], Threshold, c, MaxC, offset;
  int CEradius, CDradius, OEradius, ODradius;

  /* fetch parameters */
  CDradius = *((int *) ParmPtrs[0]);
  CEradius = *((int *) ParmPtrs[1]);
  OEradius = *((int *) ParmPtrs[2]);
  ODradius = *((int *) ParmPtrs[3]);

  /* tranfer the original data into the outpixels buffer */
  for (i = 0; i < (height * width); i++)
    outpixels[i] = inpixels[i];

  /* close the image (dilation - erosion) */
  if ((CDradius > 0) && (CEradius > 0)) {
    parms[0] = (double *) (&CDradius);
    Dilation(outpixels, DoubleBuffer, height, width, (void **) parms);
    if (IPOpExecVerbose)
      ShowImage(DoubleBuffer);
    parms[0] = (double *) (&CEradius);
    Erosion(DoubleBuffer, outpixels, height, width, (void **) parms);
    if (IPOpExecVerbose)
      ShowImage(outpixels);
  }
  
  /* open the image (erosion - dilation) */
  if ((ODradius > 0) && (OEradius > 0)) {
    parms[0] = (double *) (&OEradius);
    ErosionFFT(outpixels, DoubleBuffer, height, width, (void **) parms);
    if (IPOpExecVerbose)
      ShowImage(DoubleBuffer);
    parms[0] = (double *) (&ODradius);
    DilationFFT(DoubleBuffer, outpixels, height, width, (void **) parms);
    if (IPOpExecVerbose)
      ShowImage(outpixels);
  }
  
  /* segment the image by closed region and keep only the largest region */
  draw_mesg("Segmentation...");
  for (i = 0, k = 0, c=2.0, NMaskPixels=0; (i < height) && (Abort == FALSE);
       i++)
    for (j = 0; (j < width) && (Abort == FALSE); j++, k++)
      if (outpixels[k] == 1.0) {
	/* seed fill the new region */
	cnt = RegionFill(outpixels, height, width, i, j, 1.0, c, NULL, NULL,
			 &CentroidX, &CentroidY);
	/* if the new region is larger, delete the other, else delete it */
	if (cnt > NMaskPixels) {
	  if (NMaskPixels > 0)
	    RegionFill(outpixels, height, width, MaxI, MaxJ, MaxC, 0.0, NULL,
		       NULL, &CentroidX, &CentroidY);
	  NMaskPixels = cnt;
	  MaxI = i;
	  MaxJ = j;
	  MaxC = c;
	  cx = CentroidX;
	  cy = CentroidY;
	}
	else
	  RegionFill(outpixels, height, width, i, j, c, 0.0, NULL, NULL,
		     &CentroidX, &CentroidY);
	c++;
      }

  /* print info if required */
  if (VerboseFlag)
    printf("Contour() : largest region has %d interior pixels\n", NMaskPixels);
  
  /* check that no illegal conditions were flagged by RegionFill() */
  if (Abort)
    return;

  /* update image if required */
  if (IPOpExecVerbose)
    ShowImage(outpixels);
  
  /* Binarize the contour data */
  Threshold = 0;
  parms[0] = &Threshold;
  Binarize(outpixels, outpixels, height, width, (void **) parms);

  /* update image if required */
  if (IPOpExecVerbose)
    ShowImage(outpixels);
  
  /* Trace contour to order points in counter-clockwise order */
  for (i = 1; (i < height) && (outpixels[i*width +j] != 1.0); i++)
    for (j = 1; (j < width) && (outpixels[i*width +j] != 1.0); j++);
  NContourPoints = RegionTraceContour(outpixels, DoubleBuffer, height, width,
				      j-1, i-1, 1.0, contour, CONTOURNMAX);
  /* print info if required */
  if (VerboseFlag)
    printf("Contour() : largest region has %d contour pixels\n",
	   NContourPoints);

  if (Abort)
    return;

  /* Refill the contour data in order to fill in any islands of 0 value */
  /* by drawing the clipping outline and filling inside it              */
  for (i = 0; i < width*height; i++)
    MaskPixels[i] = 0;
  for (i = 0; i < NContourPoints; i++) 
    MaskPixels[contour[i].y * width + contour[i].x] = -1.0;
  if (IPOpExecVerbose)
    ShowImage(MaskPixels);
  cnt = RegionFill(MaskPixels, height, width, cy, cx, 0.0, 1.0, NULL, NULL,
	     &CentroidX, &CentroidY);
  if (IPOpExecVerbose)
    ShowImage(MaskPixels);
  Threshold = 0;
  parms[0] = &Threshold;
  Binarize(MaskPixels, MaskPixels, height, width, (void **) parms);
  if (IPOpExecVerbose)
    ShowImage(MaskPixels);

  /* print info if required */
  if (VerboseFlag)
    printf("Contour() : Refill count is %d pixels\n", cnt);

  /* load the contour points overlay array (counter clockwise) */
  UxFree(ContourPoints);
  if ((ContourPoints = (XPoint *) UxMalloc((NContourPoints + 1)
					   * sizeof(XPoint))) == NULL) {
    Abort = TRUE;
    draw_mesg("Abort : Can't allocate in Contour()");
    return;
  }
  for (i = 0; i < NContourPoints; i++) {
    ContourPoints[i].x = contour[i].x;
    ContourPoints[i].y = contour[i].y;
  }
  ContourPoints[NContourPoints].x = contour[0].x;
  ContourPoints[NContourPoints].y = contour[0].y;
  NContourPoints += 1;

  /* tranfer the original data into the outpixels buffer */
  for (i = 0; i < (height * width); i++)
    outpixels[i] = inpixels[i];

  /* load contour display function into overlay function array */
  DisplayContourComputed = TRUE;
  LoadOverlayFunction(ContourOverlay);
}
#endif

/****************************  Dilation ************************************/
/*                                                                         */
/*  function: morphological dilation by a circle structuring element       */
/*                                                                         */
/***************************************************************************/
void Dilation(double *inpixels, double *outpixels, int height, int width,
	      void *ParmPtrs[])
{
  char cbuf[256];
  int i, j, k, x, xm, ym, y, n, go, radius, radius2, xmax, xmin, ymax, ymin;
  struct CircleArrayElement {
    int y;
    int xmin;
    int xmax;
  } *CircleArray;

  /* load parameters */
  radius = *((int *) ParmPtrs[0]);
  radius2 = radius*radius;

  /* build the array of circle boundary coordinates */
  draw_mesg("Dilation...");
  if ((CircleArray = (struct CircleArrayElement *)
       UxMalloc((2*radius +1) * sizeof(struct CircleArrayElement))) == NULL) {
    Abort = B_TRUE;
    draw_mesg("Abort : Can't allocate in Dilation()");
    return;
  }
  for (y = -radius, n=0; y <= radius; y++, n++) {
    CircleArray[n].y = y;
    CircleArray[n].xmax = sqrt(radius2 - y*y);
    CircleArray[n].xmin = -CircleArray[n].xmax;
  }

  /* find smallest box which encloses non-zero pixel values, clear outpixels */
  for (j = 0, k = 0, xmin = width, xmax = 0, ymin = height, ymax = 0;
       j < height; j++)
    for (i = 0; i < width; i++, k++) {
      outpixels[k] = 0;
      if (inpixels[k] != 0) {
	xmin = (i < xmin ? i : xmin);
	xmax = (i > xmax ? i : xmax);
	ymin = (j < ymin ? j : ymin);
	ymax = (j > ymax ? j : ymax);
      }
    }
  xmin = (xmin > radius ? (xmin - radius) : 0);
  xmax = (xmax < (width - radius) ? (xmax + radius) : width);
  ymin = (ymin > radius ? (ymin - radius) : 0);
  ymax = (ymax < (height - radius) ? (ymax + radius) : height);
  
  /* loop for all pixels within bounding box */
  for (i = ymin; (i < ymax) && (Abort == FALSE); i++) {
    if (IPOpExecVerbose) {
      sprintf(cbuf, "Dilation - Row %d", i);
      draw_mesg(cbuf);
    }
    for (j = xmin; j < xmax; j++) {
      if (inpixels[i*width +j] != 0.0)
	outpixels[i*width +j] = 1.0;
      else {
	y = ((i - radius) >= 0 ? -radius : -i);
	ym =  (i < (height - radius) ? radius : (height-i-1));
	for (n = radius+y, go=TRUE; (y <= ym) && (go); y++, n++) {
	  x = ((j + CircleArray[n].xmin) < 0 ? 0 : CircleArray[n].xmin);
	  xm = ((j + CircleArray[n].xmax) < width ?
		CircleArray[n].xmax : (width-j-1));
	  for (; (x <= xm) && (go); x++)
	    if (inpixels[(i+y)*width + (j+x)] != 0.0)
	      go = FALSE;
	}
	outpixels[i*width +j] = (go ? 0.0 : 1.0);
      }
    }
  }

  /* free storage */
  UxFree(CircleArray);
}


#if defined (NAG)
/*************************  DilationFFT  ***********************************/
/*                                                                         */
/*  function: morphological dilation by a circle structuring element       */
/*            where the correlation is done in the Fourier domain.         */
/*                                                                         */
/***************************************************************************/
void DilationFFT(double *inpixels, double *outpixels, int height, int width,
		 void *ParmPtrs[])
{
  double *parms[2], scale;
  int radius;

  radius = *((int *) ParmPtrs[0]);
  scale = 0.01;
  parms[0] = (double *) (&radius);
  parms[1] = (double *) (&scale);
  DiskCorrelation(inpixels, outpixels, height, width, (void **) parms);
}
#endif


/****************************  Disk ****************************************/
/*                                                                         */
/*  function: create a disk image                                          */
/*                                                                         */
/***************************************************************************/
void Disk(double *inpixels, double *outpixels, int height, int width,
	  void *ParmPtrs[])
{
  char cbuf[256];
  int i, x, xc, yc, y, w, h, radius;

  /* load parameters */
  xc = *((int *) ParmPtrs[0]);
  yc = *((int *) ParmPtrs[1]);
  radius = *((int *) ParmPtrs[2]);

  /* clear the output buffer */
  for (i = 0; i < height*width; i++)
    outpixels[i] = 0;

  /* set the pixels inside the circle to 1.0 */
  for (y = yc - radius, h = -radius; y <= yc + radius; y++, h++) {
    w = sqrt((double) (radius*radius - h*h));
    for (x = xc - w; x <= xc + w; x++) 
      outpixels[y*width + x] = 1.0;
  }
}


/****************************  Erosion *************************************/
/*                                                                         */
/*  function: morphological erosion by a circle structuring element        */
/*                                                                         */
/***************************************************************************/
void Erosion(double *inpixels, double *outpixels, int height, int width,
	     void *ParmPtrs[])
{
  char cbuf[256];
  int i, j, x, y, n, go, radius, radius2;
  struct CircleArrayElement{
    int y;
    int xmin;
    int xmax;
  } *CircleArray;

  /* load parameters */
  radius = *((int *) ParmPtrs[0]);
  radius2 = radius*radius;

  /* build the array of circle boundary coordinates */
  draw_mesg("Erosion...");
  if ((CircleArray = (struct CircleArrayElement *)
      UxMalloc((2*radius +1) * sizeof(struct CircleArrayElement))) == NULL) {
    Abort = B_TRUE;
    draw_mesg("Abort : Can't allocate in Erosion()");
    return;
  }
  for (y = -radius, n=0; y <= radius; y++, n++) {
    CircleArray[n].y = y;
    CircleArray[n].xmax = sqrt(radius2 - y*y);
    CircleArray[n].xmin = -CircleArray[n].xmax;
  }
  
  /* normalized cross-correlation */
  for (i=radius; (i < (height-radius)) && (Abort == FALSE); i++) {
    if (IPOpExecVerbose) {
      sprintf(cbuf, "Erosion - Cross correlation row %d", i);
      draw_mesg(cbuf);
    }
    for (j = radius; j < (width-radius); j++) {
      for (y = -radius, n=0, go=TRUE; (y <= radius) && (go); y++, n++)
	for (x = CircleArray[n].xmin; (x <= CircleArray[n].xmax) && (go); x++)
	  if (inpixels[(i+y)*width + (j+x)] == 0.0)
	    go = FALSE;
      outpixels[i*width +j] = (go ? 1.0 : 0.0);
    }
  }

  /* zero left edge */
  if (IPOpExecVerbose)
    draw_mesg("Erosion - Zeroing left edge");
  for (j = (radius * width); j < (height - radius)*width; j += width)
    for (i = 0; i < radius; i++)
      outpixels[j+i] = 0;
  /* zero right edge */
  if (IPOpExecVerbose)
    draw_mesg("Erosion - Zeroing right edge");
  for (j = (radius * width); j < (height - radius)*width; j += width)
    for (i = (width - radius); i < width; i++)
      outpixels[j+i] = 0;
  /* zero top edge */
  if (IPOpExecVerbose)
    draw_mesg("Erosion - Zeroing top edge");
  for (i = 0; i < (radius*width); i+=width)
    for (j = 0; j < width; j++)
      outpixels[i+j] = 0;
  /* zero bottom edge */
  if (IPOpExecVerbose)
    draw_mesg("Erosion - Zeroing bottom edge");
  for (i = (height-radius)*width; i < (height*width); i+=width)
    for (j = 0; j < width; j++)
      outpixels[i+j] = 0;


  /* free storage */
  UxFree(CircleArray);
}


#if defined (NAG)
/*************************  ErosionFFT  ************************************/
/*                                                                         */
/*  function: morphological erosion by a circle structuring element        */
/*            where the correlation is done in the Fourier domain.         */
/*                                                                         */
/***************************************************************************/
void ErosionFFT(double *inpixels, double *outpixels, int height, int width,
		void *ParmPtrs[])
{
  double *parms[2], scale;
  int radius;

  radius = *((int *) ParmPtrs[0]);
  scale = 0.99;
  parms[0] = (double *) (&radius);
  parms[1] = (double *) (&scale);
  DiskCorrelation(inpixels, outpixels, height, width, (void **) parms);
}
#endif


