/***********************************************************************
*
*  Name:          imgproc.c
*
*  Author:        Paul Charette
*
*  Last Modified: 26 Nov 1994
*
*  Purpose:       Utility routines for image processing.
*
*                      Atan()
*                      Atan2()
*                      Binarize()
*                      BandZero()
*                      Center()
*                      Cos()
*                      CharToDouble()
*                      Cos()
*                      Divide()
*                      DoublesToBytesZoom()
*                      DynamicRange()
*                      Exp()
*                      FillEdges()
*                      FreeDynamicRange()
*                      FreezeDynamicRange()
*                      FixEdges()
*                      Histogram()
*                      Log()
*                      MaskOut()
*                      Mix()
*                      MixEm()
*                      MoveDoubles()
*                      Normalize()
*                      Plane()
*                      PlaneFit()
*                      PopImage()
*                      PushImage()
*                      RealtoX()
*                      RegionTraceContour()
*                      RPNRint()
*                      RPNAdd()
*                      RPNAddConstant()
*                      RPNModConstant()
*                      RPNMultiply()
*                      RPNMultiplyConstant()
*                      RPNSubtract()
*                      Second()
*                      Sharpen()
*                      Sin()
*                      SpeckleAtan()
*                      Sqr()
*                      Sqrt()
*                      SetDynamicRange()
*                      Sin()
*                      StackExchangePos()
*                      StackRotateDown()
*                      StackRotateUp()
*                      Stats()
*                      SubtractMean()
*                      SubtractMin()
*                      SubtractPixVal()
*                      SwapXY()
*                      Threshold()
*                      WeightedMean()
*                      ZeroCenter()
*
***********************************************************************/
#include <stdio.h>
#include "UxXt.h"
#include <fcntl.h>
#include <memory.h>
#include <search.h>
#include <values.h>
#include "XvgGlobals.h"

/******************************  SetPixelValue *****************************/
/*                                                                         */
/***************************************************************************/
void SetPixelValue(double *inpixels, double *outpixels, int height, int width,
		   void *parms[])
{
  int i, x, y;
  double v;

  /* load parameters */
  x = *((int *) parms[0]);
  y = *((int *) parms[1]);
  v = *((double *) parms[2]);

  /* check parameters */
  if ((x < 0) || (x >= width) || (y < 0) || (y >= height)) {
    Abort = B_TRUE;
    draw_mesg("Abort : Pixel coordinates out of bounds in SetPixelValue()");
    return;
  }

  for (i = 0; i < height*width; i++)
    outpixels[i] = inpixels[i];
  outpixels[width*y + x] = v;
}


/******************************     Acos()    ******************************/
/*                                                                         */
/***************************************************************************/
void Acos(double *inpixels, double *outpixels, int height, int width,
	  void *parms[])
{
  int i;

  /* compute cos */
  for (i = 0; i < height*width; i++)
    outpixels[i] = acos(inpixels[i]);
}


/******************************     Asin()    ******************************/
/*                                                                         */
/***************************************************************************/
void Asin(double *inpixels, double *outpixels, int height, int width,
	  void *parms[])
{
  int i;

  /* compute cos */
  for (i = 0; i < height*width; i++)
    outpixels[i] = asin(inpixels[i]);
}


/*******************************  Atan  ************************************/
/*                                                                         */
/*  function: atan operation between current and topmost stack image       */
/*                                                                         */
/***************************************************************************/
void Atan(double *inpixels, double *outpixels, int height, int width,
	  void *ParmPtrs[])
{
  int i;

  /* draw message */
  draw_mesg("Computing atan()...");

  /* atan() function */
  for (i = 0; i < (height * width); i++) {
    if (finite(inpixels[i]))
      outpixels[i] = atan(inpixels[i]);
    else
      outpixels[i] = PIOVERTWO;
  }
}

/******************************  Atan2  ************************************/
/*                                                                         */
/*  function: atan2 operation between current and topmost stack image      */
/*                                                                         */
/***************************************************************************/
void Atan2(double *inpixels, double *outpixels, int height, int width,
	   void *ParmPtrs[])
{
  int i;

  /* draw message */
  draw_mesg("Computing atan2()...");

  /* check that stack is not empty, then decrement stack pointer  */
  if (ImageStackTop <= 0) {
    Abort = B_TRUE;
    draw_mesg("Abort : Image stack is empty in Atan2()");
    return;
  }
  ImageStackTop--;
  UpdateStackLabel(ImageStackTop);
  
  /* check that height and width match the current image size */
  if   ((ImageStack[ImageStackTop].height != height)
	|| (ImageStack[ImageStackTop].width != width)) {
    Abort = B_TRUE;
    draw_mesg("Abort : Image in stack is not the current size in Atan2()");
    return;
  }
  
  /* atan2() function */
  for (i = 0; i < (height * width); i++) {
    outpixels[i] = atan2(ImageStack[ImageStackTop].pixels[i], inpixels[i]);
  }
}

/***************************  Binarize  ************************************/
/*                                                                         */
/*  function: convert the image to binary form [0 or 1]                    */
/*                                                                         */
/***************************************************************************/
void Binarize(double *inpixels, double *outpixels, int height, int width,
	      void *ParmPtrs[])
{
  int i;
  double Threshold;

  /* load parameters */
  Threshold = *((double *) ParmPtrs[0]);

  /* draw message */
  draw_mesg("Binarizing image...");

  /* Binarize the data */
  for (i = 0; i < (height * width); i++) {
    if (inpixels[i] <= Threshold)
      outpixels[i] = 0;
    else
      outpixels[i] = 1.0;
  }
#ifdef DEBUG
  printf("Image binarized [%e]\n", Threshold);
#endif
}

/****************************  BracketSD  **********************************/
/*                                                                         */
/*  function: convert the image to binary form [0 or 1]                    */
/*                                                                         */
/***************************************************************************/
void BracketSD(double *inpixels, double *outpixels, int height, int width,
	       void *ParmPtrs[])
{
  int i, n;
  double Threshold, mean, var, scale;

  /* load parameters */
  scale = fabs(*((double *) ParmPtrs[0]));

  /* draw message */
  draw_mesg("Braketing image...");

  if (DisplayContourComputed) {
    /* calculate mean */
    for (i = 0, mean = 0, n = 0; i < width*height; i++) {
      if (MaskPixels[i] == 1.0) {
	mean += inpixels[i];
	n++;
      }
    }
    if (n == 0) {
      Abort = B_TRUE;
      sprintf(cbuf, "Abort : No pixels in BracketSD(%f)", scale);
      draw_mesg(cbuf);
      return;
    }
    mean /= ((double) n);
    
    /* calculate variance */
    for (i = 0, var = 0; i < (height*width); i++) {
      if (MaskPixels[i] == 1.0)
	var += ((inpixels[i] - mean) * (inpixels[i] - mean));
    }
    var /= ((double) n);

    /* set threshold */
    Threshold = sqrt(var) * scale;
    if (VerboseFlag)
      printf("BracketSD(contour) : mean = %f, SD = %f, Threshold = %f\n",
	     mean, sqrt(var), Threshold);

    /* Bracket the data */
    for (i = 0; i < (height * width); i++) {
      if ((inpixels[i] < -Threshold) || (inpixels[i] > Threshold)
	  || (MaskPixels[i] == 0.0))
	outpixels[i] = 0.0;
      else
	outpixels[i] = 1.0;
    }
  }
  else {
    /* calculate mean */
    for (i = 0, mean = 0; i < width*height; i++)
      mean += inpixels[i];
    mean /= ((double) (width*height));
    
    /* calculate variance */
    for (i = 0, var = 0; i < (height*width); i++)
      var += ((inpixels[i] - mean) * (inpixels[i] - mean));
    var /= ((double) (width*height));

    /* set threshold */
    Threshold = sqrt(var) * scale;
    if (VerboseFlag)
      printf("BracketSD(no contour) : mean = %f, SD = %f, Threshold = %f\n",
	     mean, sqrt(var), Threshold);

    /* Bracket the data */
    for (i = 0; i < (height * width); i++) {
      if ((inpixels[i] < -Threshold) || (inpixels[i] > Threshold)) 
	outpixels[i] = 0.0;
      else
	outpixels[i] = 1.0;
    }
  }
}

/***************************  BandZero  ************************************/
/*                                                                         */
/*  function: zero values within a given range of the B_TRUE Zero            */
/*                                                                         */
/***************************************************************************/
void BandZero(double *inpixels, double *outpixels, int height, int width,
	      void *ParmPtrs[])
{
  int i;
  double fraction, max, min, threshold;
  
  /* draw message */
  draw_mesg("Zeroing band...");
  
  /* load & check parameters */
  fraction = *((double *) ParmPtrs[0]);
  if ((fraction < 0) || (fraction > 1)) {
    Abort = B_TRUE;
    draw_mesg("Abort : Invalid fraction paramater in BandZero()");
    return;
  }
  
  /* compute dynamic range of data */
  DynamicRange(inpixels, height*width, &max, &min);
  
  /* Zero the band about the B_TRUE zero */
  for (i = 0, threshold = (max - min)*fraction; i < (height * width); i++)
    if ((inpixels[i] >= -threshold) && (inpixels[i] <= threshold))
      outpixels[i] = 0;
    else
      outpixels[i] = inpixels[i];
}

/***************************  Centre ***************************************/
/*                                                                         */
/*  function: center image on center of mass                               */
/*                                                                         */
/***************************************************************************/
void Center(double *inpixels, double *outpixels, int height, int width,
	    void *ParmPtrs[])
{
  int i, j, is, js, cmx, cmy, cnt;

  /* make sure contour is defined */
  if (DisplayContourComputed == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort : Undefined contour in Center()");
    return;
  }

  /* show banner */
  draw_mesg("Centering image...");

  /* compute coordinates of center of mass */
  for (i = 0, cnt = 0, cmx = 0, cmy = 0; i < height; i++)
    for (j = 0; j < width; j++)
      if (MaskPixels[i*width + j] != 0) {
	cmy += i;
	cmx += j;
	cnt++;
      }
  cmx /= cnt;
  cmy /= cnt;
  CMX = cmx;
  CMY = cmy;
/*  printf("Center of mass at %d,%d\n", CMX, CMY); */

  /* shift image so that center of mass coincides with the image center */
  for (i = 0, cmy -= (height/2), cmx -= (width/2); i < height; i++) {
    /* modulo height */
    is = i - cmy;
    if (is < 0)
      is += height;
    else if (is >= height)
      is -= height;
    /* loop for columns */
    for (j = 0; j < width; j++) {
      /* modulo height */
      js = j - cmx;
      if (js < 0)
	js += width;
      else if (js >= width)
	js -= width;
      /* shift the data */
      outpixels[is*width + js] = inpixels[i*width + j];
    }
  }
}

/***************************** CharToDouble ********************************/
/*                                                                         */
/***************************************************************************/
void CharToDouble(char *pin, double *pout, int n)
{
  char *p;
  double *q;
  int j;

  for (j = 0, p = pin, q = pout; j < n; j++)
    *q++ = *p++;
}

/*******************************     Cos()    ******************************/
/*                                                                         */
/***************************************************************************/
void Cos(double *inpixels, double *outpixels, int height, int width,
	 void *parms[])
{
  int i;

  /* compute cos */
  for (i = 0; i < height*width; i++)
    outpixels[i] = cos(inpixels[i]);
}


/***************************  Crop *****************************************/
/*                                                                         */
/*  function: crop image                                                   */
/*                                                                         */
/***************************************************************************/
void Crop(double *inpixels, double *outpixels, int height, int width,
	  void *ParmPtrs[])
{
  extern Widget ProcessedImageShell;
  static double *medians=NULL;
  CONTOUR_ELEMENT contour[CONTOURNMAX];
  int x, y, i, j, k, nm, xmin, xmax, ymin, ymax, offset;
  double v, delta, avg, scale, RadiansToMicrons, disp;

  /* internal parms */
  offset = 10;
  disp = 5.5;
  RadiansToMicrons = 0.152406;

  /* load parameters */
  xmin = *((int *) ParmPtrs[0]);
  xmax = *((int *) ParmPtrs[1]);
  ymin = *((int *) ParmPtrs[2]);
  ymax = *((int *) ParmPtrs[3]);

  /* correct to maximum/minimum parameter size if required */
  if (xmin < 0) {
    xmin = -xmin;
    xmax = width - xmin - 1;
  }
  xmax = (xmax > width-1 ? width-1 : xmax);
  if (ymin < 0) {
    ymin = -ymin;
    ymax = height - ymax - 1;
  }
  ymax = (ymax > height-1 ? height-1 : ymax);

  /* allocate store for median values */
  if ((medians = (double *) XvgMM_Alloc(medians, height * sizeof(double)))
      == NULL) {
    Abort = B_TRUE;
    draw_mesg("Abort : Can't allocate in Crop()");
    return;
  }

  /* draw message  & do it... */
  draw_mesg("Cropping image...");
  scale = ((double) (xmax - xmin + 1))/((double)(xmax - xmin + 1 - 2*offset));
  for (y = 0, i = 0, nm = 0, avg = 0; y < height; y++) {
    /* record the delta for this line, if it's within the cropping area */
    if ((y >= ymin) && (y <= ymax)) {
      /* calculate the displacement along this line */
      v = (inpixels[y*width + xmax - offset]
	   - inpixels[y*width + xmin + offset])*scale;
      avg += v;
      /* insert this value into the sorted array */
      for (k = 0; (k < nm) && (v > medians[k]); k++);
      if (k != nm) {
	for (j = k; j < nm; j++)
	  medians[j+1] = medians[j];
      }
      medians[k] = v;
      nm++;
    }
    /* crop the data on this line */
    for (x = 0; x < width; x++, i++) {
      if ((x >= xmin) && (x <= xmax) && (y >= ymin) && (y <= ymax)) {
	outpixels[i] = inpixels[i];
	MaskPixels[i] = 1.0;
      }
      else {
	outpixels[i] = 0.0;
	MaskPixels[i] = 0.0;
      }
    }
  }
  
  /* Trace contour to order points in counter-clockwise order */
  draw_mesg("Tracing mask contour...");
  for (i = 1; (i < height) && (MaskPixels[i*width +j] != 1.0); i++)
    for (j = 1; (j < width) && (MaskPixels[i*width +j] != 1.0); j++);
  NContourPoints = RegionTraceContour(MaskPixels, DoubleBuffer, height, width,
				      j-1, i-1, 1.0, contour, CONTOURNMAX);
  /* load the contour points overlay array (counter clockwise) */
  if ((ContourPoints =
       (XPoint *) XvgMM_Alloc(ContourPoints,
			      (NContourPoints + 1) * sizeof(XPoint)))
      == NULL) {
    Abort = B_TRUE;
    draw_mesg("Abort : Can't allocate in Crop()");
    return;
  }
  for (i = 0; i < NContourPoints; i++) {
    ContourPoints[i].x = contour[i].x;
    ContourPoints[i].y = contour[i].y;
  }
  ContourPoints[NContourPoints].x = contour[0].x;
  ContourPoints[NContourPoints].y = contour[0].y;
  NContourPoints += 1;

  /* load contour display function into overlay function array */
  DisplayContourComputed = B_TRUE;
  LoadOverlayFunction(ContourOverlay);

  /* resize window */
/*
  if (SlicesProjectionsOn) {
    UxPutHeight(ProcessedImageShell,
		(ymax - ymin + 1)
		*ZoomFactor +SCROLL_BAR_THICKNESS +SLICE_WINDOW_WIDTH);
    UxPutWidth(ProcessedImageShell,
	       (xmax - xmin + 1)
	       *ZoomFactor +SCROLL_BAR_THICKNESS +SLICE_WINDOW_WIDTH);
  }
  else {
    UxPutHeight(ProcessedImageShell,
	(ymax - ymin + 1)*ZoomFactor + SCROLL_BAR_THICKNESS);
    UxPutWidth(ProcessedImageShell,
	(xmax - xmin + 1)*ZoomFactor + SCROLL_BAR_THICKNESS);
  }
*/
#ifdef DEBUG
  printf("Image cropped\n");
#endif
}

/******************************* Divide       ******************************/
/*                                                                         */
/***************************************************************************/
void Divide(double *inpixels, double *outpixels, int height, int width,
	    void *parms[])
{
  double v;
  int i;

  /* check that stack is not empty, then decrement stack pointer  */
  if (ImageStackTop <= 0) {
    Abort = B_TRUE;
    draw_mesg("Abort : Image stack is empty in Divide()");
    return;
  }
  ImageStackTop--;
  UpdateStackLabel(ImageStackTop);
  
  /* check that height and width match the current image size */
  if   ((ImageStack[ImageStackTop].height != height)
	|| (ImageStack[ImageStackTop].width != width)) {
    Abort = B_TRUE;
    draw_mesg("Abort : Image in stack was not the current size in Divide()");
    return;
  }
  
  /* compute division */
  for (i = 0; i < height*width; i++)
    outpixels[i] = ImageStack[ImageStackTop].pixels[i] / inpixels[i];
}


/*************************** DoublesToBytesZoom ****************************/
/*                                                                         */
/***************************************************************************/
void DoublesToBytesZoom(double *inpixels, char *outpixels,
			int height, int width, int ZoomFactor)
{
  double *p;
  char *q, *r;
  int i, j, k;

  if (ZoomFactor == 1)
    for (i = 0; i < height*width; i++)
      outpixels[i] = inpixels[i];
  else {
    for (i = 0, p = inpixels; i < height; i++) {
      for (j = 0, q = &outpixels[(i*ZoomFactor) * (width*ZoomFactor)];
	   j < width; j++, p++)
	for (k = 1; k <= ZoomFactor; k++)
	  *q++ = *p;
      
      /* expand out vertically */
      for (k = 1; k < ZoomFactor; k++)
	for (j = 0, q = &outpixels[(i*ZoomFactor) * (width*ZoomFactor)],
	     r = &outpixels[((i*ZoomFactor) + k) * (width*ZoomFactor)];
	     j < (width*ZoomFactor); j++,q++,r++)
	  *r = *q;
    }
  }
}

/************************** DynamicRange ***********************************/
/*                                                                         */
/*  function: return max/min for an image                                  */
/*                                                                         */
/***************************************************************************/
void DynamicRange(double *inpixels, int n, double *max, double *min)
{
  int i;
  double *p, min_pix, max_pix, pix;

  if (FixDynamicRange) {
    *min = FixedDynamicRangeMin;
    *max = FixedDynamicRangeMax;
  }
  else {
    /* find minimum and maximum values */
    for (i=0, p=inpixels, min_pix=MAXDOUBLE, max_pix=(-MAXDOUBLE); i < n;i++) {
      pix = *p++;
      if (finite(pix)) {
	if (pix < min_pix) {
	  min_pix = pix;
	}
	if (pix > max_pix) {
	  max_pix = pix;
	}
      }
    }
    
    /* return values */
    *min = min_pix;
    *max = max_pix;
  }
    
#ifdef DEBUG
  printf("Dynamic range - Min:%e, Max:%e\n", *min, *max);
#endif
}

/******************************* exp ***************************************/
/*                                                                         */
/*  function: exponential                                                  */
/*                                                                         */
/***************************************************************************/
void Exp(double *inpixels, double *outpixels, int height, int width,
	 void *ParmPtrs[])
{
  int i;

  /* draw message */
  draw_mesg("Exponential...");

  for (i = 0; i < (height*width); i++)
    outpixels[i] = exp(inpixels[i]);
}

/******************************  FillEdges  ********************************/
void FillEdges(double *inpixels, double *outpixels,
	       int height, int width, int len)
{
  int i, j;
  double *p, *q;

  /* draw message */
  draw_mesg("Filling edges...");

  /* left edge */
  for (j = (len/2 * width); j < (height - len/2)*width; j += width)
    for (i = 0; i < len/2; i++)
      outpixels[j+i] = inpixels[i+j];
  /* right edge */
  for (j = (len/2 * width); j < (height - len/2)*width; j += width)
    for (i = (width - len/2); i < width; i++)
      outpixels[j+i] = inpixels[i+j];
  /* top edge */
  for (i = 0; i < len/2; i++)
    for (j = 0, p = &(inpixels[width * i]), q = &(outpixels[width * i]);
	 j < width; j++)
      *q++ = *p++;
  /* bottom edge */
  for (i = height - len/2; i < height; i++)
    for (j = 0, p = &(inpixels[width * i]), q = &(outpixels[width * i]);
	 j < width; j++)
      *q++ = *p++;
}

/****************************     Second()    ******************************/
/*                                                                         */
/*  function: delay in seconds                                             */
/*                                                                         */
/***************************************************************************/
void Second(double *inpixels, double *outpixels, int height, int width,
	    void *parms[], double max, double min)
{
  int i, delay;

  /* fetch parms */
  delay = *((int *) parms[0]);
  sleep_us(delay*1000000);

  /* transfer data */
  for (i = 0; i < height*width; i++)
    outpixels[i] = inpixels[i];
}

/**************************** set dynamic range ****************************/
/*                                                                         */
/*  function: set fixed dynamic range                                      */
/*                                                                         */
/***************************************************************************/
void SetDynamicRange(double *inpixels, double *outpixels,
		     int height, int width, void *parms[],
		     double max, double min)
{
  int i;

  /* fetch parms */
  FixedDynamicRangeMin = *((double *) parms[0]);
  FixedDynamicRangeMax = *((double *) parms[1]);

  /* check parms */
  if (FixedDynamicRangeMax <= FixedDynamicRangeMin) {
    Abort = B_TRUE;
    draw_mesg("Abort : Invalid parameters in SetDynamicRange()");
    return;
  }
  FixDynamicRange = B_TRUE;

  /* transfer data */
  for (i = 0; i < height*width; i++)
    outpixels[i] = inpixels[i];
}


/************************************** sharpen ****************************/
/*                                                                         */
/*  function: High-boost filter (Gozales & Woods, p.196)                   */
/*                                                                         */
/***************************************************************************/
void Sharpen(double *inpixels, double *outpixels, int height, int width,
	     void *parms[])
{
  double A, v;
  int i, j;

  /* fetch parms */
  A = *((double *) parms[0]);

  /* calculate highpass */
  for (j = 1, v = 1.0/9.0; j < height-1; j++)
    for (i = 1; i < width-1; i++)
      outpixels[j*width + i] = ((8.0*inpixels[j*width + i]
				 - inpixels[(j-1)*width + i-1]
				 - inpixels[(j-1)*width + i]
				 - inpixels[(j-1)*width + i+1]
				 - inpixels[j*width + i-1]
				 - inpixels[j*width + i+1]
				 - inpixels[(j+1)*width + i-1]
				 - inpixels[(j+1)*width + i]
				 - inpixels[(j+1)*width + i+1]) * v)
	+ (A - 1.0) * inpixels[j*width + i];
}


/*******************************     Sin()    ******************************/
/*                                                                         */
/*  function: sin                                                          */
/*                                                                         */
/***************************************************************************/
void Sin(double *inpixels, double *outpixels, int height, int width,
	 void *parms[])
{
  int i;

  /* compute sin */
  for (i = 0; i < height*width; i++)
    outpixels[i] = sin(inpixels[i]);
}


/*******************************     Sqr()    ******************************/
/*                                                                         */
/*  function: square                                                       */
/*                                                                         */
/***************************************************************************/
void Sqr(double *inpixels, double *outpixels, int height, int width,
	 void *parms[])
{
  int i;

  /* compute square */
  for (i = 0; i < height*width; i++)
    outpixels[i] = inpixels[i] * inpixels[i];
}


/*******************************     Sqrt()    *****************************/
/*                                                                         */
/*  function: square root                                                  */
/*                                                                         */
/***************************************************************************/
void Sqrt(double *inpixels, double *outpixels, int height, int width,
	  void *parms[])
{
  int i;

  /* compute square */
  for (i = 0; i < height*width; i++)
    outpixels[i] = sqrt(inpixels[i]);
}


/****************************     Stats       ******************************/
/*                                                                         */
/*  function: mean and standard deviation                                  */
/*                                                                         */
/***************************************************************************/
void Stats(double *inpixels, double *outpixels, int height, int width,
	   void *parms[], double max, double min)
{
  double mean, SD, cnt;
  int i;

  /* compute the mean */
  if (DisplayContourComputed) {
    for (i = 0, cnt = 0, mean = 0; i < height*width; i++)
      if (MaskPixels[i] != 0) {
	mean += inpixels[i];
	cnt++;
      }
    mean /= cnt;

    /* compute SD */
    for (i = 0, SD = 0; i < height*width; i++)
      if (MaskPixels[i] != 0)
	SD += ((inpixels[i] - mean)*(inpixels[i] - mean));
    SD  = sqrt(SD/cnt);
  }
  else {
    for (i = 0, mean = 0; i < height*width; i++)
      mean += inpixels[i];
    mean /= ((double) i);

    /* compute SD */
    for (i = 0, SD = 0; i < height*width; i++)
      SD += ((inpixels[i] - mean)*(inpixels[i] - mean));
    SD  = sqrt(SD/((double) i));
  }

  /* transfer data */
  for (i = 0; i < height*width; i++)
    outpixels[i] = inpixels[i];

  /* print out the stats if required */
  if (!InteractiveFlag && !DisplayOnlyFlag)
    sprintf(Msg, "mean = %.2e, SD:%.2e for file %s ",
	    mean, SD, InFileNameStripped);
  else
    sprintf(Msg, " = mean:%.2e, SD:%.2e ", mean, SD);
  if (VerboseFlag)
    printf("Stats() : mean = %.2e, SD = %.2e\n", mean, SD);
}

/**************************** free dynamic range ***************************/
/*                                                                         */
/*  function: free dynamic range                                           */
/*                                                                         */
/***************************************************************************/
void FreeDynamicRange(double *inpixels, double *outpixels,
		      int height, int width, void *parms[],
		      double max, double min)
{
  int i;

  FixDynamicRange = B_FALSE;

  /* transfer data */
  for (i = 0; i < height*width; i++)
    outpixels[i] = inpixels[i];
}

/**************************** freeze dynamic range *************************/
/*                                                                         */
/*  function: freeze dynamic range to current values                       */
/*                                                                         */
/***************************************************************************/
void FreezeDynamicRange(double *inpixels, double *outpixels,
			int height, int width, void *parms[],
			double max, double min)
{
  int i;

  /* transfer the data and find minimum and maximum values */
  for (i=0, FixedDynamicRangeMin=MAXDOUBLE, FixedDynamicRangeMax=(-MAXDOUBLE);
       i < width*height; i++) {
    outpixels[i] = inpixels[i];
    if (outpixels[i] < FixedDynamicRangeMin)
      FixedDynamicRangeMin = outpixels[i];
    else if (outpixels[i] > FixedDynamicRangeMax)
      FixedDynamicRangeMax = outpixels[i];
  }
  FixDynamicRange = B_TRUE;
}

/******************************  fix edges   *******************************/
/*                                                                         */
/*  function: fill in edges of an image after neighbourhood processing     */
/*                                                                         */
/***************************************************************************/
void FixEdges(double *inpixels, double *outpixels, int height, int width,
	      void *parms[], double max, double min)
{
  int i, j, len;
  double *p, *q;

  /* fetch kernel size */
  len = *((int *) parms[0]);

  /* check input parameters */
  if ((len < 2) || (len > width)) {
    Abort = B_TRUE;
    draw_mesg("Abort : Invalid parms in FixEdges()");
    return;
  }

  /* draw message */
  draw_mesg("Fixing edges...");

  /* left edge */
  for (j = (len/2 * width); j < (height - len/2)*width; j += width)
    for (i = 0; i < len/2; i++)
      outpixels[j+i] = min;
  /* right edge */
  for (j = (len/2 * width); j < (height - len/2)*width; j += width)
    for (i = (width - len/2); i < width; i++)
      outpixels[j+i] = min;
  /* top edge */
  for (i = 0; i < len/2; i++)
    for (j = 0, p = &outpixels[width * (len/2)],
	 q = &outpixels[width * i]; j < width; j++)
      *q++ = min;
  /* bottom edge */
  for (i = height - len/2; i < height; i++)
    for (j = 0, p = &outpixels[width * (height - len/2 - 1)],
	 q = &outpixels[width * i]; j < width; j++)
      *q++ = min;
}

/************************** histogram **************************************/
/*                                                                         */
/*  function: build histogram of BW pixel intensities                      */
/*            (ignore black pixels)                                        */
/*                                                                         */
/***************************************************************************/
void Histogram(double *pixels, int xs, int ys, int w, int h,
	       int xextent, double max, double min, int *hist,
	       XRectangle *XRects, double *meanp, double *SDp)
{
  int i, j, indx, xstart, ystart, width, height;
  double maxval, divisor, *p, mean, SD, cnt;

  /* scale parameters by zoom factor */
  xstart = xs / ZoomFactor;
  ystart = ys / ZoomFactor;
  width = w / ZoomFactor;
  height = h / ZoomFactor;
#ifdef DEBUG
  printf("Histogram() => h:%d, w:%d, ZoomFactor:%d, xstart:%d, ystart:%d, width:%d, height:%d, max:%e, min:%e\n",
	 h, w, ZoomFactor, xstart, ystart, width, height, max, min);
#endif

  /* check for undefined data */
  if (pixels == NULL) {
    if (VerboseFlag)
      printf("Histogram() : NULL data pointer...\n");
    return;
  }

  /* check for input errors */
  if (((ystart+height-1)*xextent + xstart+width-1) > (ImgHeight*ImgWidth - 1)){
    if (VerboseFlag) {
      printf("Histogram(): invalid call parameters...\n");
      printf("    xstart:     %d\n", xstart);
      printf("    ystart:     %d\n", ystart);
      printf("    width:      %d\n", width);
      printf("    height:     %d\n", height);
      printf("    xextent:    %d\n", xextent);
    }
    return;
  }
  
  /* zero out histogram entries */
  for (i = 0; i < HISTOGRAM_BINS; i++)
    hist[i] = 0;

  /* build histogram */
  if (max != min) {
    if (DisplayContourComputed) {
      for (i = ystart, divisor = 1.0/(max-min)*((double) (HISTOGRAM_BINS -1)),
	   mean = 0, cnt = 0;
	   i < (ystart+height); i++)
	for (j=xstart, p=&(pixels[i*xextent + j]); j < (xstart + width);
	     j++, p++)
	  if ((MaskPixels[i*xextent + j] != 0) && finite(*p)) {
	    indx = (*p - min)*divisor;
	    if ((indx >= 0) && (indx < HISTOGRAM_BINS)) {
	      hist[indx] += 1;
	      mean += *p;
	      cnt++;
	    }
	    else {
	      printf("Internal error in Histogram(): index out of range!\a\n");
	      printf("    index:      %d\n", indx);
	      printf("    data value: %e\n", *p);
	      printf("    x:          %d\n", j);
	      printf("    y:          %d\n", i);
	      printf("    min:        %e\n", min);
	      printf("    max:        %e\n", max);
	      printf("    divisor:    %e\n", divisor);
	      ErrorMsg("Internal error in Histogram()!");
	      return;
	    }
	  }
      mean /= cnt;
      /* compute SD */
      for (i = ystart, SD = 0; i < (ystart+height); i++)
	for (j=xstart, p=&pixels[i*xextent + j]; j < (xstart + width);
	     j++, p++)
	  if ((MaskPixels[i*xextent + j] != 0) && finite(*p))
	    SD += ((*p - mean)*(*p - mean));
      SD  = sqrt(SD/cnt);
    }
    else {
      for (i = ystart, divisor = 1.0/(max-min)*((double) (HISTOGRAM_BINS -1)),
	   mean = 0, cnt = 0; i < (ystart+height); i++)
	for (j=xstart,p=&pixels[i*xextent + j]; j < (xstart + width);
	     j++,p++) {
	  if (finite(*p)) {
	    indx = (*p - min)*divisor;
	    if ((indx >= 0) && (indx < HISTOGRAM_BINS)) {
	      hist[indx] += 1;
	      mean += *p;
	      cnt++;
	    }
	    else {
	      printf("Internal error in Histogram(): index out of range!\a\n");
	      printf("    index:      %d\n", indx);
	      printf("    data value: %e\n", *p);
	      printf("    x:          %d\n", j);
	      printf("    y:          %d\n", i);
	      printf("    min:        %e\n", min);
	      printf("    max:        %e\n", max);
	      printf("    divisor:    %e\n", divisor);
	      ErrorMsg("Internal error in Histogram()!");
	      return;
	    }
	  }
	}
      mean /= cnt;
      /* compute SD */
      for (i = ystart, SD = 0; i < (ystart+height); i++)
	for (j=xstart, p=&pixels[i*xextent + j]; j < (xstart + width); j++,p++)
	  if (finite(*p))
	    SD += ((*p - mean)*(*p - mean));
      SD  = sqrt(SD/cnt);
    }
  }
  else {
    mean = min;
    SD = 0;
  }
#ifdef DEBUG
  printf("Histogram() => mean:%e, stdev:%e\n", mean, SD);
#endif
  
  /* find histogram maximum (forget about min & max values) */
  for (i = 1, maxval = 0; i < (HISTOGRAM_BINS-1); i++)
    if (hist[i] > maxval)
      maxval = hist[i];
#ifdef DEBUG
  printf("Histogram() => maxval:%e\n", maxval);
#endif
  
  /* create scaled array of histogram rectangles */
#ifdef DEBUG
  printf("Histogram values: ");
#endif
  if (maxval != 0) 
    for (i = 0; i < HISTOGRAM_BINS; i++) {
      XRects[i].width = 2;
      XRects[i].height = (int)(((double) (hist[i]*HISTOGRAM_HEIGHT))/maxval);
      XRects[i].x = 2*i;
      XRects[i].y = HISTOGRAM_HEIGHT - XRects[i].height;
#ifdef DEBUG
  printf("%d ", XRects[i].height);
#endif
    }
  else
    for (i = 0; i < HISTOGRAM_BINS; i++) {
      XRects[i].width = 2;
      XRects[i].height = 0;
      XRects[i].x = 2*i;
      XRects[i].y = HISTOGRAM_HEIGHT - XRects[i].height;
#ifdef DEBUG
  printf("%d ", XRects[i].height);
#endif
    }
#ifdef DEBUG
  printf("\n");
#endif

  /* return mean and SD values */
  *meanp = mean;
  *SDp = SD;
}

/**********************       log            *******************************/
/*                                                                         */
/*  function: natural logarythm                                            */
/*                                                                         */
/***************************************************************************/
void Log(double *inpixels, double *outpixels, int height, int width,
	 void *ParmPtrs[])
{
  int i;
  double max, min, offset;
  boolean_t tmp;

  /* draw message */
  draw_mesg("Logarithm...");

  /* flip negative values */
  for (i = 0; i < (height*width); i++)
    outpixels[i] = fabs(inpixels[i]);

  /* check for zero values */
  tmp = FixDynamicRange;
  FixDynamicRange = B_FALSE;
  DynamicRange(outpixels, height*width, &max, &min);
  FixDynamicRange = tmp;
  if (min == 0)
    for (i = 0, offset = max*0.0001; i < (height*width); i++)
      outpixels[i] = log(outpixels[i] + offset);
  else
    for (i = 0; i < (height*width); i++)
      outpixels[i] = log(outpixels[i]);
}

/*************************** MaskOut ***************************************/
/*                                                                         */
/*  function: multiply image by the binary mask                            */
/*                                                                         */
/***************************************************************************/
void MaskOut(double *inpixels, double *outpixels, int height, int width,
	     void *ParmPtrs[])
{
  int i;

  /* make sure contour mask is defined */
  if (DisplayContourComputed == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort : Undefined contour in MaskOut()");
    return;
  }

  /* mask pixels */
  for (i = 0; i < height*width; i++)
    outpixels[i] = inpixels[i] * MaskPixels[i];
}


/*************************** Mix *******************************************/
/*                                                                         */
/*  function: mix two images I = (1-x)I0 + (x)I1, where 0<= x <= 1         */
/*                                                                         */
/***************************************************************************/
void MixEm(double *inpixels1, double *inpixels2, double *outpixels,
	   int height, int width, double s)
{
  int i;

  for (i = 0; i < height*width; i++)
    outpixels[i] = ((1.0 - s) * inpixels2[i]) + (s * inpixels1[i]);
}

void Mix(double *inpixels, double *outpixels, int height, int width,
	 void *ParmPtrs[])
{
  int i;
  double s;

  /* draw message */
  draw_mesg("Mixing images...");
  
  /* load & check parameters */
  s = *((double *) ParmPtrs[0]);
  if ((s < 0) || (s > 1)) {
    Abort = B_TRUE;
    draw_mesg("Abort : Paramater outside [0..1] range in Mix()");
    return;
  }

  /* check that stack is not empty, then decrement stack pointer  */
  if (ImageStackTop <= 0) {
    Abort = B_TRUE;
    draw_mesg("Abort : Image stack is empty in Mix()");
    return;
  }
  ImageStackTop--;
  UpdateStackLabel(ImageStackTop);
  
  /* check that height and width match the current image size */
  if   ((ImageStack[ImageStackTop].height != height)
	|| (ImageStack[ImageStackTop].width != width)) {
    Abort = B_TRUE;
    draw_mesg("Abort : Image in stack is not the current size in Mix()");
    return;
  }
  
  /* mix pixels */
  MixEm(inpixels, ImageStack[ImageStackTop].pixels, outpixels,
	height, width, s);
}


/***************************** MoveDoubles *******************************/
/*                                                                         */
/***************************************************************************/
void MoveDoubles(double *pin, double *pout, int n)
{
  double *p, *q;
  int j;

  for (j = 0, p = pin, q = pout; j < n; j++)
    *q++ = *p++;
}

/***************************** Normalize *** *******************************/
/*                                                                         */
/* Normalize the data to the 0..1 range                                    */
/*                                                                         */
/***************************************************************************/
void Normalize(double *inpixels, double *outpixels, int height, int width,
	       void *ParmPtrs[])
{
  double max, min, scale;
  int i;

  /* fetch dynamic range */
  DynamicRange(inpixels, height*width, &max, &min);
  scale = 1.0 / (max - min);

  /* normalize to 0..1 */
  draw_mesg("Normalizing...");
  for (i = 0; i < height*width; i++)
    outpixels[i] = (inpixels[i] - min) * scale;
}

/***************************     Plane     *********************************/
/*                                                                         */
/*  function: Draw a plane modeled as z = Ux + Vy                          */
/*                                                                         */
/***************************************************************************/
void Plane(double *inpixels, double *outpixels, int height, int width,
	      void *ParmPtrs[])
{
  double U, V;
  int i, j, k;

  /* load parameters */
  U = *((double *) ParmPtrs[0]);
  V = *((double *) ParmPtrs[1]);

  /* draw plane */
  for (j = 0; j < height; j++)
    for (i = 0; i < width; i++, k++)
      outpixels[k] = ((double) (i - width/2))*U + ((double) (j - height/2))*V; 
}

/***************************** plane fitting *******************************/
/*                                                                         */
/*  function : neighborhood plane fitting (n = m x m size kernel)          */
/*                                                                         */
/*             Plane equation => z = ux + vy + w                           */
/*                                                                         */
/*   Least squares fit => Sum(z)  = u*Sum(x)   + v*Sum(y)   + w*n          */
/*                        Sum(xz) = u*Sum(x^2) + v*Sum(xy)  + w*Sum(x)     */
/*                        Sum(yz) = u*Sum(xy)  + v*Sum(y^2) + w*Sum(y)     */
/*                                                                         */
/*                        Get 3 equations and 3 unknowns in the form:      */
/*                                au + bv + cw = d                         */
/*                                eu + fv + gw = h                         */
/*                                iu + jv + kw = l                         */
/*                                                                         */
/*                                where a = g = Sum(x)  = 0                */
/*                                      b = k = Sum(y)  = 0                */
/*                                      c =     n                          */
/*                                      d =     Sum(z)                     */
/*                                      e = j = Sum(x^2) = Sum(y^2)        */
/*                                      f = i = Sum(xy) = 0                */
/*                                      h =     Sum(xz)                    */
/*                                      l =     Sum(yz)                    */
/*                                                                         */
/*                                for a 2D square domain centered on the   */
/*                                origin, equally space in x and y.        */
/*                                                                         */
/*                        Solution :                                       */
/*                               u = h/e = Sum(xz) / Sum(x^2)              */
/*                               v = l/j = Sum(yz) / Sum(y^2)              */
/*                               w = d/c = Sum(z)  / n                     */
/*                                                                         */
/*                        VAF =  100*( 1 - Sum((Z - Zp)^2) / Sum(Z^2))     */
/*                                                                         */
/* NOTE: two images are created on the stack, containing the U and V       */
/*       components of the gradient at each point in the image             */
/*                                                                         */
/*                                                                         */
/***************************************************************************/
void PlaneFit(double *inpixels, double *outpixels, int height, int width,
	      void *ParmPtrs[])
{
  int kern_w, i, j, x, y, save_gradients;
  double *p, SumX2, SumX2Inv, NInv, SumXZ, SumYZ, SumZ, SumZ2, SumZZp2, val;
  double ZZp, VAF, u, v, w, *up, *vp, *wp;

  /* load parameters */
  kern_w = *((int *) ParmPtrs[0]);
  save_gradients = *((int *) ParmPtrs[1]);
  if (VerboseFlag)
    printf("PlaneFit() : kernel:%d (half size: %d)\n", kern_w, kern_w/2);

  /* check input parameters */
  if ((kern_w < 2) || (kern_w > width)) {
    Abort = B_TRUE;
    draw_mesg("Abort : Invalid parms in PlaneFit()");
    return;
  }

  /* allocate spacd for gradient images if required */
  if (save_gradients) {
    if (XvgImgStack_Alloc(height, width) == B_FALSE) {
      Abort = B_TRUE;
      draw_mesg("Abort : Could not allocate for W in PlaneFit()");
      return;
    }
    wp = ImageStack[ImageStackTop-1].pixels;

    if (XvgImgStack_Alloc(height, width) == B_FALSE) {
      Abort = B_TRUE;
      draw_mesg("Abort : Could not allocate for V in PlaneFit()");
      return;
    }
    vp = ImageStack[ImageStackTop-1].pixels;
    
    if (XvgImgStack_Alloc(height, width) == B_FALSE) {
      Abort = B_TRUE;
      draw_mesg("Abort : Could not allocate for U in PlaneFit()");
      return;
    }
    up = ImageStack[ImageStackTop-1].pixels;
  }

  /* compute Sum(x^2) for a kernel area of kern_w * kern_w */
  for (x = 1, SumX2=0 ; x <= kern_w/2; x++)
    SumX2 += (x * x);
  SumX2 *= (kern_w * 2.0);
  SumX2Inv = 1.0 / SumX2;
  NInv = 1.0 / ((double) (kern_w * kern_w));

  /* compute the least squares fit plane equation at every point */
  draw_mesg("Neighborhood plane fit...");
  for (i = kern_w/2; (i < (height - kern_w/2)) && (Abort == B_FALSE);
       i++) {
    for (j = kern_w/2; j < (width - kern_w/2); j++) {
      /* compute Sum(xz), Sum(yz) and Sum(z) for the kernel */
      for (y = -kern_w/2, SumXZ=0, SumYZ=0, SumZ2=0,SumZ=0; y <= kern_w/2; y++)
	for (x = -kern_w/2, p = &(inpixels[(i+y)*width + j - kern_w/2]);
	     x <= kern_w/2; x++, p++) {
	  SumXZ += (x * (*p));
	  SumYZ += (y * (*p));
	  SumZ2 += ((*p) * (*p));
	  SumZ  += (*p);
	}
      /* compute the least squares fit for the plane equation */
      u = (SumXZ * SumX2Inv);
      v = (SumYZ * SumX2Inv);
      w = (SumZ * NInv);

      /* store the gradient components if required */
      if (save_gradients) {
	up[i*width + j] = u;
	vp[i*width + j] = v;
	wp[i*width + j] = w;
      }

      /* compute Sum(z - zp) for the kernel */
      for (y = -kern_w/2, SumZZp2=0; y <= kern_w/2; y++)
	for (x = -kern_w/2, p = &(inpixels[(i+y)*width + j - kern_w/2]);
	     x <= kern_w/2; x++, p++) {
	  ZZp = (*p - (u*x + v*y + w));
	  SumZZp2 +=  (ZZp * ZZp);
	}
      /* compute the VAF and store (check for underflow) */
      val = 100.0*(1.0 - SumZZp2 / SumZ2);
      outpixels[i*width + j] = (finite(val) != 0 ? val : 0);
    }
  }
  
  /* zero left edge */
  for (j = (kern_w/2 * width); j < (height - kern_w/2)*width; j += width)
    for (i = 0; i < kern_w/2; i++) {
      outpixels[j+i] = 0;
      if (save_gradients) {
	up[j+i] = 0;
	vp[j+i] = 0;
	wp[j+i] = 0;
      }
    }
  /* zero right edge */
  for (j = (kern_w/2 * width); j < (height - kern_w/2)*width; j += width)
    for (i = (width - kern_w/2); i < width; i++) {
      outpixels[j+i] = 0;
      if (save_gradients) {
	up[j+i] = 0;
	vp[j+i] = 0;
	wp[j+i] = 0;
      }
    }
  /* zero top edge */
  for (i = 0; i < kern_w/2; i++)
    for (j = 0; j < width; j++) {
      outpixels[width*i + j] = 0;
      if (save_gradients) {
	up[width*i + j] = 0;
	vp[width*i + j] = 0;
	wp[width*i + j] = 0;
      }
    }
  /* zero bottom edge */
  for (i = height - kern_w/2; i < height; i++)
    for (j = 0; j < width; j++) {
      outpixels[width*i + j] = 0;
      if (save_gradients) {
	up[width*i + j] = 0;
	vp[width*i + j] = 0;
	wp[width*i + j] = 0;
      }
    }
}


/************************     PopImage     *********************************/
/*                                                                         */
/*  function: fetch image from buffer                                      */
/*                                                                         */
/***************************************************************************/
void PopImage(double *inpixels, double *outpixels, int height, int width,
	      void *ParmPtrs[])
{
  int i;

  /* check that stack is not empty, then decrement stack pointer  */
  if (ImageStackTop <= 0) {
    Abort = B_TRUE;
    draw_mesg("Abort : Image stack is empty in PopImage()");
    return;
  }
  ImageStackTop--;
  UpdateStackLabel(ImageStackTop);
  
  /* check that height and width match the current image size */
  if   ((ImageStack[ImageStackTop].height != height)
	|| (ImageStack[ImageStackTop].width != width)) {
    Abort = B_TRUE;
    draw_mesg("Abort : Image in stack is not the current size in PopImage()");
    return;
  }
  
  /* transfer data */
  draw_mesg("Popping image...");
  for (i = 0; i < height*width; i++)
    outpixels[i] = ImageStack[ImageStackTop].pixels[i];
}


/************************     PushImage     ********************************/
/*                                                                         */
/*  function: save image in buffer                                         */
/*                                                                         */
/***************************************************************************/
void PushImage(double *inpixels, double *outpixels, int height, int width,
	       void *ParmPtrs[])
{
  int i;

  /* pop the image */
  if (XvgImgStack_Push(inpixels, height, width) == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort : popping error in PushImage()");
    return;
  }

  /* transfer data */
  for (i = 0; i < height*width; i++)
    outpixels[i] = inpixels[i];
}


/************************* RealtX ******************************************/
/*                                                                         */
/*  function: convert double data values to X pixel values                 */
/*                                                                         */
/***************************************************************************/
void RealtoX(double *inpixels, char *outpixels, int height, int width,
	     double max, double min, int maxscale, int minscale,
	     int ZoomFactor)
{
  int i, j, k, indx, non_finite_points;
  char *q, *r, val;
  double *p, divisor, localmin, localmax;

  /* check ZoomFactor parameter */
  if ((ZoomFactor != 1) && (ZoomFactor != 2) && (ZoomFactor != 4)
      && (ZoomFactor != 8)) {
    sprintf(cbuf, "RealtoX() : Zoomfactor = %d", ZoomFactor);
    ErrorMsg(cbuf);
    printf("%s\n", cbuf);
    return;
  }
  if (VerboseFlag)
    printf("RealtoX() : Zoomfactor = %d\n", ZoomFactor);

  /* set flags */
  non_finite_points = 0;

  /* check for NULL images */
  if ((max - min) <= (MINDOUBLE*1000.0)) {
    for (i = 0, q = outpixels; i <  (height*width); i++)
      *q++ = CurrentCells[0].pixel;
    return;
  }

  /* compute mapping dynamic range */
  localmin = min + ((max - min)
		    * ((double) (minscale - HIST_SCALE_MIN))
		    / ((double) (HIST_SCALE_MAX - HIST_SCALE_MIN)));
  localmax = min + ((max - min)
		    * ((double) (maxscale - HIST_SCALE_MIN))
		    / ((double) (HIST_SCALE_MAX - HIST_SCALE_MIN)));
      
  if (ZoomFactor == 1) {
    if ((minscale == HIST_SCALE_MIN) && (maxscale == HIST_SCALE_MAX)) {
      for (i = 0, p = inpixels, q = outpixels, divisor = 1.0/(max - min)*127.0;
	   i < (height*width); i++, p++, q++) {
	/* make sure that data point is finite */
	if ((finite(*p) != 0) && (isnan(*p) == 0)) {
	  /* error check */
	  indx = (*p - min)*divisor;
	  if ((indx < 0) || (indx > 127)) {
	    printf("Internal error in RealToX() : X index out of range!\a\n");
	    printf("    index:      %d\n", i);
	    printf("    data value: %e\n", *p);
	    printf("    min:        %e\n", min);
	    printf("    max:        %e\n", max);
	    printf("    minscale:   %d\n", minscale);
	    printf("    maxscale:   %d\n", maxscale);
	    printf("    ZoomFactor: %d\n", ZoomFactor);
	    printf("    divisor:    %e\n", divisor);
	    printf("    X index:    %d\n", indx);
	    printf("    height:     %d\n", height);
	    printf("    width:      %d\n", width);
	    printf("    DRFreeze:   %d\n", FixDynamicRange);
	    ErrorMsg("Internal error in RealToX()!");
	    return;
	  }
	  else {
	    *q = CurrentCells[indx].pixel;
	  }
	}
	else {
	  *p = min;
	  *q = black;
	  non_finite_points++;
	}
      }
    }
    else {
      for (i = 0, p = inpixels, q = outpixels,
	   divisor = 1.0/(localmax - localmin) * 127.0;
	   i < (height*width); i++, p++, q++) {
	if (*p < localmin)
	  *q = CurrentCells[0].pixel;
	else if (*p > localmax)
	  *q = CurrentCells[127].pixel;
	else
	  *q = CurrentCells[(int)((*p - localmin)*divisor)].pixel;
      }
    }
  }
  else {
    if ((minscale == HIST_SCALE_MIN) && (maxscale == HIST_SCALE_MAX)) {
      for (i=0, p=inpixels, divisor=1.0/(max - min)*127.0; i < height; i++) {
	for (j=0, q=&outpixels[(i*ZoomFactor) * (width*ZoomFactor)];
	     j < width; j++, p++) {
	  val=CurrentCells[(int)((*p - min)*divisor)].pixel;
	  for (k=0; k < ZoomFactor; k++)
	    *q++ = val;
	}
	/* expand out vertically */
	for (k = 1; k < ZoomFactor; k++)
	  for (j = 0, q=&outpixels[(i*ZoomFactor) * (width*ZoomFactor)],
	       r = &outpixels[((i*ZoomFactor) + k) * (width*ZoomFactor)];
	       j < (width*ZoomFactor); j++,q++,r++)
	    *r = *q;
      }
    }
    else {
      for (i=0, p=inpixels, divisor=1.0/(max - min)*127.0; i < height; i++) {
	for (j=0, q=&outpixels[(i*ZoomFactor) * (width*ZoomFactor)];
	     j < width; j++, p++) {
	  if (*p < localmin)
	    val = CurrentCells[0].pixel;
	  else if (*p > localmax)
	    val = CurrentCells[127].pixel;
	  else
	    val = CurrentCells[(int)((*p - localmin)*divisor)].pixel;
	  for (k=0; k < ZoomFactor; k++)
	    *q++ = val;
	}
	/* expand out vertically */
	for (k = 1; k < ZoomFactor; k++)
	  for (j = 0, q=&outpixels[(i*ZoomFactor) * (width*ZoomFactor)],
	       r = &outpixels[((i*ZoomFactor) + k) * (width*ZoomFactor)];
	       j < (width*ZoomFactor); j++,q++,r++)
	    *r = *q;
      }
    }
  }

  /* if non-finite points detected, signal the error */
  if (non_finite_points) {
    sprintf(cbuf, "RealtoX() : %d non-finite points in the image buffer",
	    non_finite_points);
    ErrorMsg(cbuf);
  }
#ifdef DEBUG
  printf("Real to X pixels (Lut Scale Max:%d & Min:%d, Zoom:%d)\n",
	 maxscale, minscale, ZoomFactor);
#endif
}

/************************ RegionTraceContour *******************************/
/*                                                                         */
/*  function: starting with a seed point, trace the outside contour        */
/*            in the counter-clockwise direction until                     */
/*            return to seed point or dead end.                            */
/*                                                                         */
/*            8-connected neighbours are as follows:                       */
/*                 0   1   2                                               */
/*                 7   x   3                                               */
/*                 6   5   4                                               */
/*                                                                         */
/*                                                                         */
/***************************************************************************/
#define PIXEL0 0x01
#define PIXEL1 0x02
#define PIXEL2 0x04
#define PIXEL3 0x08
#define PIXEL4 0x10
#define PIXEL5 0x20
#define PIXEL6 0x40
#define PIXEL7 0x80
static   double *N8_0, *N8_1, *N8_2, *N8_3, *N8_4, *N8_5, *N8_6, *N8_7,
         *DB_0, *DB_1, *DB_2, *DB_3, *DB_4, *DB_5, *DB_6, *DB_7;
static boolean_t NextPixel(int *x, int *y, int flag, int rc,
			 CONTOUR_ELEMENT *contour)
{
  /* pixel 0 */
  if ((*N8_7 == rc) && (*N8_0 != rc) && (*DB_0 != 1) && (flag & PIXEL0)) {
    *x = (*x) -1;
    *y = (*y) -1;
  }
  /* pixel 1 */
  else if ((*N8_0 == rc) && (*N8_1 != rc) && (*DB_1 != 1) && (flag & PIXEL1)) {
    *y = (*y) -1;
  }
  /* pixel 2 */
  else if ((*N8_1 == rc) && (*N8_2 != rc) && (*DB_2 != 1) && (flag & PIXEL2)) {
    *x = (*x) +1;
    *y = (*y) -1;
  }
  /* pixel 3 */
  else if ((*N8_2 == rc) && (*N8_3 != rc) && (*DB_3 != 1) && (flag & PIXEL3)) {
    *x = (*x) +1;
  }
  /* pixel 4 */
  else if ((*N8_3 == rc) && (*N8_4 != rc) && (*DB_4 != 1) && (flag & PIXEL4)) {
    *x = (*x) +1;
    *y = (*y) +1;
  }
  /* pixel 5 */
  else if ((*N8_4 == rc) && (*N8_5 != rc) && (*DB_5 != 1) && (flag & PIXEL5)) {
    *y = (*y) +1;
  }
  /* pixel 6 */
  else if ((*N8_5 == rc) && (*N8_6 != rc) && (*DB_6 != 1) && (flag & PIXEL6)) {
    *x = (*x) -1;
    *y = (*y) +1;
  }
  /* pixel 7 */
  else if ((*N8_6 == rc) && (*N8_7 != rc) && (*DB_7 != 1) && (flag & PIXEL7)) {
    *x = (*x) -1;
  }
  else {
    return(B_FALSE);
  }
  return(B_TRUE);
}

int RegionTraceContour(double *pixels, double *buffer, int height, int width,
		       int xs, int ys, double rc, CONTOUR_ELEMENT *contour,
		       int nmax)
{
  boolean_t BackTrack;
  int i, x, y, xc, yc, cnt;
  
  /* print out banner */
  draw_mesg("Tracing region contour...");

  /* check input parms */
  if ((pixels[ys*width + xs] == rc) && (xs != 0) && (ys != 0)) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort:Invalid start [%d,%d] %e==%e in RegionTraceContour()",
	    xs, ys, pixels[ys*width + xs], rc);
    draw_mesg(cbuf);
    return(0);
  }

  /* clear dirty pixel buffer */
  for (i = 0; i < height*width; i++)
    buffer[i] = 0;
  
  /* loop till contour traced */
  cnt = 0; x = xs; y = ys;
  do {
    /* reset flags */
    BackTrack = B_FALSE;

    /* check for array size exceeded */
    if (cnt >= nmax) {
      Abort = B_TRUE;
      draw_mesg("Abort : Contour array size exceeded in RegionTraceContour()");
      return(cnt);
    }

    /* mark this pixel as dirty */
    if (cnt > 1)
      buffer[y*width + x] = 1;
    
    /* address pixel data 8-neighbors */
    N8_0 = &(pixels[(y-1)*width + x-1]);
    N8_1 = &(pixels[(y-1)*width + x]);
    N8_2 = &(pixels[(y-1)*width + x+1]);
    N8_3 = &(pixels[y*width + x+1]);
    N8_4 = &(pixels[(y+1)*width + x+1]);
    N8_5 = &(pixels[(y+1)*width + x]);
    N8_6 = &(pixels[(y+1)*width + x-1]);
    N8_7 = &(pixels[y*width + x-1]);
    
    /* address dirty buffer 8-neighbors */
    DB_0 = &(buffer[(y-1)*width + x-1]);
    DB_1 = &(buffer[(y-1)*width + x]);
    DB_2 = &(buffer[(y-1)*width + x+1]);
    DB_3 = &(buffer[y*width + x+1]);
    DB_4 = &(buffer[(y+1)*width + x+1]);
    DB_5 = &(buffer[(y+1)*width + x]);
    DB_6 = &(buffer[(y+1)*width + x-1]);
    DB_7 = &(buffer[y*width + x-1]);
    
    /* pixels that are not on the outer edge */
    if ((x > 0) && (x < (width-1)) && (y > 0) && (y < (height-1))) {
      if (NextPixel(&x, &y, PIXEL0 | PIXEL1 | PIXEL2 | PIXEL3
		    | PIXEL4 | PIXEL5 | PIXEL6 | PIXEL7,
		    rc, contour) == B_FALSE) {
	/* backtrack */
	if (cnt > 1) {
	  cnt--;
	  x = contour[cnt-1].x;
	  y = contour[cnt-1].y;
	  BackTrack = B_TRUE;
	}
	else {
	  sprintf(cbuf, "RegionTraceContour() : Dead end at cnt=%d (%d,%d) 8-neighbours are [%f,%f,%f,%f,%f,%f,%f,%f]",
		  cnt, x, y, *N8_0, *N8_1, *N8_2, *N8_3, *N8_4,
		  *N8_5, *N8_6, *N8_7);
	  ErrorMsg(cbuf);
	  return(cnt);
	}
      }
    }
    /* points on outer edge of image */
    else {
      /* top edge */
      if ((y == 0) && (x > 0) && (x < width)) {
	if (NextPixel(&x, &y, PIXEL3 | PIXEL4 | PIXEL5 | PIXEL6 | PIXEL7,
		      rc, contour) == B_FALSE) {
	  x--;
	}
      }
      /* left edge */
      else if ((x == 0) && (y < (height-1)) && (y >= 0)) {
	if (NextPixel(&x, &y, PIXEL1 | PIXEL2 | PIXEL3 | PIXEL4 | PIXEL5,
		      rc, contour) == B_FALSE) {
	  y++;
	}
      }
      /* bottom edge */
      else if ((y == (height-1)) && (x < (width-1)) && (x >= 0)) {
	if (NextPixel(&x, &y, PIXEL0 | PIXEL1 | PIXEL2 | PIXEL3 | PIXEL7,
		      rc, contour) == B_FALSE) {
	  x++;
	}
      }
      /* right edge */
      else if (x == (width-1) && (y > 0) && (y < height)) {
	if (NextPixel(&x, &y, PIXEL0 | PIXEL1 | PIXEL5 | PIXEL6 | PIXEL7,
		      rc, contour) == B_FALSE) {
	  y--;
	}
      }
      /* error condition */
      else {
	sprintf(cbuf, "Abort: Error at [%d,%d] in RegionTraceContour()",
		x, y);
	draw_mesg(cbuf);
	return(cnt);
      }
    }
    
    /* save the start point if required */
    if (cnt == 0) {
      xc = x;
      yc = y;
    }

    /* save this contour point if not backtracking */
    if (BackTrack == B_FALSE) {
      contour[cnt].x = x;
      contour[cnt].y = y;
      cnt++;
    }

  } while (!((cnt > 1) && (xc == x) && (yc == y)));
  return(cnt);
}


/****************************** Shift Image ********************************/
/*                                                                         */
/*  function: Shift Image in plane                                         */
/*                                                                         */
/***************************************************************************/
void ShiftImage(double *inpixels, double *outpixels, int height,
		int width, void *ParmPtrs[])
{
  int i, j, k, dx, dy;

  /* load parms */
  dx = *((int *) ParmPtrs[0]);
  dy = *((int *) ParmPtrs[1]);

  for (j = 0, k = 0; j < height; j++)
    for (i = 0; i < width; i++, k++)
      outpixels[((j+dy) < height ? j+dy : j+dy-height)*width
		+ ((i+dx) < width ? i+dx : i+dx-width)] = inpixels[k];
}

/********************************* RPNAdd **********************************/
/*                                                                         */
/*  function: Add image to the one on the stack                            */
/*                                                                         */
/***************************************************************************/
void RPNAdd(double *inpixels, double *outpixels, int height, int width,
	    void *ParmPtrs[])
{
  int i;

  /* check that stack is not empty, then decrement stack pointer  */
  if (ImageStackTop <= 0) {
    Abort = B_TRUE;
    draw_mesg("Abort : Image stack is empty in RPNAdd()");
    return;
  }
  ImageStackTop--;
  UpdateStackLabel(ImageStackTop);
  
  /* check that height and width match the current image size */
  if   ((ImageStack[ImageStackTop].height != height)
	|| (ImageStack[ImageStackTop].width != width)) {
    Abort = B_TRUE;
    draw_mesg("Abort: Image on stack is not the current size in RPNAdd()");
    return;
  }
  
  /* transfer data and add */
  for (i = 0; i < height*width; i++)
    outpixels[i] = inpixels[i] + ImageStack[ImageStackTop].pixels[i];
}

/**************************** RPNAddConstant *******************************/
/*                                                                         */
/*  function: Add constant to an image                                     */
/*                                                                         */
/***************************************************************************/
void RPNAddConstant(double *inpixels, double *outpixels, int height, int width,
		    void *ParmPtrs[])
{
  int i;
  double val;

  /* load parms */
  val = *((double *) ParmPtrs[0]);

  /* transfer data and add the constant */
  for (i = 0; i < height*width; i++)
    outpixels[i] = inpixels[i] + val;
}

/******************************* RPNXInverse *******************************/
/*                                                                         */
/*  function: Multiplicative inverse                                       */
/*                                                                         */
/***************************************************************************/
void RPNXInverse(double *inpixels, double *outpixels, int height, int width,
		 void *ParmPtrs[])
{
  int i;

  /* ivert and transfer data */
  for (i = 0; i < height*width; i++)
    outpixels[i] = 1.0/inpixels[i];
}

/*********************** RPNModConstant ************************************/
/*                                                                         */
/*  function: modulo operation                                             */
/*                                                                         */
/***************************************************************************/
void RPNModConstant(double *inpixels, double *outpixels, int height,
		    int width, void *ParmPtrs[])
{
  double val;
  int i;

  /* load parms */
  val = *((double *) ParmPtrs[0]);

  for (i = 0; i < height*width; i++) {
    outpixels[i] = fmod(outpixels[i], val);
    if (outpixels[i] < 0)
      outpixels[i] += val;
  }
}

/**************************** RPNMULTIPLY **********************************/
/*                                                                         */
/*  function: Multiply by image on the top of the stack and pop ...        */
/*                                                                         */
/***************************************************************************/
void RPNMultiply(double *inpixels, double *outpixels, int height, int width,
		 void *ParmPtrs[])
{
  int i;

  /* check that stack is not empty, then decrement stack pointer  */
  if (ImageStackTop <= 0) {
    Abort = B_TRUE;
    draw_mesg("Abort : Image stack is empty in RPNMultiply()");
    return;
  }
  ImageStackTop--;
  UpdateStackLabel(ImageStackTop);
  
  /* check that height and width match the current image size */
  if   ((ImageStack[ImageStackTop].height != height)
	|| (ImageStack[ImageStackTop].width != width)) {
    Abort = B_TRUE;
    draw_mesg("Abort:Image on stack is not the current size in RPNMultiply()");
    return;
  }
  
  /* transfer data and multiply */
  for (i = 0; i < height*width; i++)
    outpixels[i] = inpixels[i] * ImageStack[ImageStackTop].pixels[i];
}

/********************** RPNMULTIPLYConstant ********************************/
/*                                                                         */
/*  function: Multiply by image by a constant                              */
/*                                                                         */
/***************************************************************************/
void RPNMultiplyConstant(double *inpixels, double *outpixels, int height,
			 int width, void *ParmPtrs[])
{
  double val;
  int i;

  /* load parms */
  val = *((double *) ParmPtrs[0]);

  /* transfer data and multiply by the constant */
  for (i = 0; i < height*width; i++)
    outpixels[i] = inpixels[i] * val;
}


/********************************* RPNRint *********************************/
/*                                                                         */
/*  function: Round to the nearest integer                                 */
/*                                                                         */
/***************************************************************************/
void RPNRint(double *inpixels, double *outpixels, int height, int width,
	     void *ParmPtrs[])
{
  int i;

  /* round and transfer data and add */
  for (i = 0; i < height*width; i++)
    outpixels[i] = rint(inpixels[i]);
}

/**************************** RPNSUBTRACT **********************************/
/*                                                                         */
/*  function: subtract an image on the top of the stack and pop ...        */
/*                                                                         */
/***************************************************************************/
void RPNSubtract(double *inpixels, double *outpixels, int height, int width,
		 void *ParmPtrs[])
{
  int i;

  /* check that stack is not empty, then decrement stack pointer  */
  if (ImageStackTop <= 0) {
    Abort = B_TRUE;
    draw_mesg("Abort : Image stack is empty in RPNSubtract()");
    return;
  }
  ImageStackTop--;
  UpdateStackLabel(ImageStackTop);
  
  /* check that height and width match the current image size */
  if   ((ImageStack[ImageStackTop].height != height)
	|| (ImageStack[ImageStackTop].width != width)) {
    Abort = B_TRUE;
    draw_mesg("Abort:Image on stack is not the current size in RPNSubtract()");
    return;
  }
  
  /* transfer data and subtract */
  for (i = 0; i < height*width; i++)
    outpixels[i] = ImageStack[ImageStackTop].pixels[i] - inpixels[i];
}

/******************************  SpeckleAtan  ******************************/
/*                                                                         */
/*  function: atan operation between current and topmost stack image       */
/*            which uses a weighted average of sin and cos to compute      */
/*            the angle.                                                   */
/*            The stack image is assumend to be of the form:               */
/*               I * Cos(THETA + APLHA/2) * Sin(ALPHA/2)                   */
/*            The current image is assumend to be of the form:             */
/*               I * Cos(THETA + APLHA/2) * Cos(ALPHA/2)                   */
/*                                                                         */
/*            Because THETA is unknown, ALPHA/2 can only be recovered      */
/*            to within a "quadrant flip". However, by multiplying         */
/*            the atan result by 2 to obtain ALPHA and wrapping in         */
/*            the interval -PI,PI, the result is in the correct quadrant.  */
/*                                                                         */
/***************************************************************************/
void SpeckleAtan(double *inpixels, double *outpixels, int height, int width,
		 void *ParmPtrs[])
{
  double sin_angle, cos_angle, cos_val, sin_val, total_weights_inv,
    fabs_sin_val, fabs_cos_val, scale_inv, v;
  int k, flag;

  /* load flag */
  flag = *((int *) ParmPtrs[0]);

  /* draw message */
  draw_mesg("Computing atanw()...");

  /* check that stack is not empty, then decrement stack pointer  */
  if (ImageStackTop <= 0) {
    Abort = B_TRUE;
    draw_mesg("Abort : Image stack is empty in Atanw()");
    return;
  }
  ImageStackTop--;
  UpdateStackLabel(ImageStackTop);
  
  /* check that height and width match the current image size */
  if   ((ImageStack[ImageStackTop].height != height)
	|| (ImageStack[ImageStackTop].width != width)) {
    Abort = B_TRUE;
    draw_mesg("Abort : Image in stack is not the current size in Atanw()");
    return;
  }
  
  /* atan2() function */
  for (k = 0; k < (height * width); k++) {
    /* load sin and cos values */
    sin_val = ImageStack[ImageStackTop].pixels[k];
    cos_val = inpixels[k];

    /* choose between atan and atan2 options */
    if (flag) {
      v = sin_val/cos_val;
      if (finite(v))
	outpixels[k] = atan(v) * 2.0;
      else
	outpixels[k] = 0;
    }
    
    else {
      /* calculate the weights */
      fabs_sin_val = fabs(sin_val);
      fabs_cos_val = fabs(cos_val);
      total_weights_inv = 1.0 / (fabs_sin_val + fabs_cos_val);
      
      /* calculate the inverse value of the I*Cos(THETA + ALPHA/2) term */
      scale_inv = 1.0 / sqrt(sin_val*sin_val + cos_val*cos_val);
      
      /* check for finite weight inverse and scale inverse */
      if (finite(total_weights_inv) && finite(scale_inv)) {
	
	/* calculate angle */
	if ((sin_val >= 0) && (cos_val >= 0)) {     /* upper right quadrant */
	  sin_angle = asin(sin_val * scale_inv);
	  cos_angle = acos(cos_val * scale_inv);
	}
	else if ((sin_val >= 0) && (cos_val < 0)) { /* upper left quadrant */
	  sin_angle = PI - asin(sin_val * scale_inv);
	  cos_angle = acos(cos_val * scale_inv);
	}
	else if ((sin_val < 0) && (cos_val < 0)) {  /* lower left quadrant */
	  sin_angle = -(PI + asin(sin_val * scale_inv));
	  cos_angle = - acos(cos_val * scale_inv);
	}
	else {                                      /* lower right quadrant */
	  sin_angle = asin(sin_val * scale_inv);
	  cos_angle = -acos(cos_val * scale_inv);
	}
	
	/* weighted average of both asin and acos angles */
	outpixels[k] = (fabs_cos_val*sin_angle + fabs_sin_val*cos_angle)
	  *total_weights_inv;
	
	/* multiply by 2 and wrap to obtain desired angle */
	outpixels[k] = fmod(2.0*outpixels[k], TWOPI);
	if (outpixels[k] > PI)
	  outpixels[k] -= TWOPI;
	else if (outpixels[k] <= -PI)
	  outpixels[k] += TWOPI;
      }
      /* else arbitrarily set the angle to 0 */
      else {
	outpixels[k] = 0;
      }
    }
  }
}

/************************     StackExchangePos    **************************/
/*                                                                         */
/*  function: exchange images on the stack                                 */
/*                                                                         */
/***************************************************************************/
void StackExchangePos(double *inpixels, double *outpixels,
		      int height, int width, void *ParmPtrs[])
{
  double *tmp;
  int i, p1, p2;
  
  /* load parms */
  p1 = *((int *) ParmPtrs[0]);
  p2 = *((int *) ParmPtrs[1]);

  /* check that the stack contains these positions */
  if ((ImageStackTop <= p1) || (ImageStackTop <= p2)) {
    Abort = B_TRUE;
    draw_mesg("Abort : Image stack too small in StackExchangePos()");
    return;
  }

  /* check that the images are the same size */
  if ((ImageStack[p1].height != ImageStack[p2].height) ||
      (ImageStack[p1].width != ImageStack[p2].width)) {
    Abort = B_TRUE;
    draw_mesg("Abort : Image sizes don't match in StackExchangePos()");
    return;
  }
  
  /* exchange data from stack both positions */
  tmp = ImageStack[p1].pixels;
  ImageStack[p1].pixels = ImageStack[p2].pixels;
  ImageStack[p2].pixels = tmp;

  /* copy the data from the input to the output buffer */
  for (i = 0; i < height*width; i++)
    outpixels[i] = inpixels[i];
}


/************************     StackClear ***********************************/
/*                                                                         */
/*  function: clear the stack                                              */
/*                                                                         */
/***************************************************************************/
void StackClear(double *inpixels, double *outpixels,
		      int height, int width, void *ParmPtrs[])
{
  int i;

  /* clear the stack */
  XvgImgStack_Clear();
  UpdateStackLabel(ImageStackTop);

  /* copy the data from the input to the output buffer */
  for (i = 0; i < height*width; i++)
    outpixels[i] = inpixels[i];
}


/************************     StackRotateDown   ****************************/
/*                                                                         */
/*  function: rotate the stack downward                                    */
/*                                                                         */
/***************************************************************************/
void StackRotateDown(double *inpixels, double *outpixels,
		     int height, int width, void *ParmPtrs[])
{
  int i;

  /* check that stack is not empty */
  if (ImageStackTop == 0) {
    Abort = B_TRUE;
    draw_mesg("Abort : Image stack is empty in StackRotateDown()");
    return;
  }

  /* transfer data from the bottommost stack position to the current image */
  for (i = 0; i < height*width; i++)
    outpixels[i] = ImageStack[0].pixels[i];

  /* rotate the stack */
  for (i = 0; i < ImageStackTop-1; i++)
    ImageStack[i] = ImageStack[i+1];

  /* transfer data from the current image to the topmost stack position */
  if ((ImageStack[ImageStackTop-1].pixels
       = (double *) XvgMM_Alloc((void *) ImageStack[ImageStackTop-1].pixels,
				height*width*sizeof(double))) == NULL) {
    Abort = B_TRUE;
    draw_mesg("Abort : Could not reallocate stack space in StackRotateDown()");
    return;
  }
  ImageStack[ImageStackTop-1].height = height;
  ImageStack[ImageStackTop-1].width = width;
  for (i = 0; i < height*width; i++)
    ImageStack[ImageStackTop-1].pixels[i] = inpixels[i];
}


/************************     StackRotateUp   ******************************/
/*                                                                         */
/*  function: rotate the stack upward                                      */
/*                                                                         */
/***************************************************************************/
void StackRotateUp(double *inpixels, double *outpixels, int height, int width,
		   void *ParmPtrs[])
{
  int i;

  /* check that stack is not empty */
  if (ImageStackTop == 0) {
    Abort = B_TRUE;
    draw_mesg("Abort : Image stack is empty in StackRotateUp()");
    return;
  }

  /* transfer data from the topmost stack position to the current image */
  for (i = 0; i < height*width; i++)
    outpixels[i] = ImageStack[ImageStackTop-1].pixels[i];

  /* rotate the stack */
  for (i = ImageStackTop-1; i > 1; i--)
    ImageStack[i] = ImageStack[i-1];

  /* transfer data from the current image to the topmost stack position */
  if ((ImageStack[0].pixels
       = (double *) XvgMM_Alloc((void *) ImageStack[0].pixels,
				height*width*sizeof(double))) == NULL) {
    Abort = B_TRUE;
    draw_mesg("Abort : Could not reallocate stack space in StackRotateUp()");
    return;
  }
  ImageStack[0].height = height;
  ImageStack[0].width = width;
  for (i = 0; i < height*width; i++)
    ImageStack[0].pixels[i] = inpixels[i];
}


/**************************** Subtract Mean ********************************/
/*                                                                         */
/*  function: Subtract mean from the image                                 */
/*                                                                         */
/***************************************************************************/
void SubtractMean(double *inpixels, double *outpixels, int height, int width,
		  void *ParmPtrs[])
{
  int i;
  double meanval;

  /* compute mean */
  draw_mesg("Subtract Mean - Computing mean...");
  for (i = 0, meanval = 0; i < (height*width); i++)
    meanval += inpixels[i];
  meanval /= (height*width);

  /* subtract mean */
  draw_mesg("Subtract Mean - Subtracting...");
  for (i = 0; i < (height*width); i++)
    outpixels[i] = inpixels[i] - meanval;
}


/**************************** Subtract Min *********************************/
/*                                                                         */
/*  function: Subtract minimum from the image                              */
/*                                                                         */
/***************************************************************************/
void SubtractMin(double *inpixels, double *outpixels, int height, int width,
		 void *ParmPtrs[])
{
  int i;
  double min;
  
  /* find minimum */
  draw_mesg("Subtract Min - Find minimum...");
  for (i=0, min=MAXDOUBLE; i < width*height; i++)
    if (inpixels[i] < min)
      min = inpixels[i];
  
  /* subtract min */
  draw_mesg("Subtract Min - Subtracting...");
  for (i = 0; i < (height*width); i++)
    outpixels[i] = inpixels[i] - min;
}


/**************************** SubtractPixVal *******************************/
/*                                                                         */
/*  function: Subtract value from a specific pixel                         */
/*                                                                         */
/***************************************************************************/
void SubtractPixVal(double *inpixels, double *outpixels, int height, int width,
		    void *ParmPtrs[])
{
  int i, x, y;
  double val;

  /* get pixel coordinates */
  x = *((int *) ParmPtrs[0]);
  y = *((int *) ParmPtrs[1]);

  /* find minimum */
  draw_mesg("SubtractPixVal - Subtracting...");
  for (i = 0, val = inpixels[y*width + x]; i < (height*width); i++)
    outpixels[i] = inpixels[i] - val;
}


/************************    Swap XY       *********************************/
/*                                                                         */
/*  function: swap the current image with the one on the stack             */
/*                                                                         */
/***************************************************************************/
void SwapXY(double *inpixels, double *outpixels, int height, int width,
	    void *ParmPtrs[])
{
  int i;

  /* swap */
  if (XvgImgStack_Swap(inpixels) == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort : Swapping error in SwapXY()");
    return;
  }
  
  /* transfer data */
  for (i = 0; i < height*width; i++)
    outpixels[i] = inpixels[i];
}


/************************  threshold  ************************************/
/*                                                                         */
/*  function: thresholding of BW pixel values to 0 or 255                  */
/*                                                                         */
/***************************************************************************/
void Threshold(double *inpixels, double *outpixels, int height, int width,
	       void *ParmPtrs[])
{
  int i;
  double *p, *q, LowerThreshold, UpperThreshold, LowerRepVal, UpperRepVal;

  /* load parameters */
  LowerThreshold = *((double *) ParmPtrs[0]);
  UpperThreshold = *((double *) ParmPtrs[1]);
  LowerRepVal = *((double *) ParmPtrs[2]);
  UpperRepVal = *((double *) ParmPtrs[3]);

  /* threshold the data */
  draw_mesg("Thresholding...");
  for (i = 0, p = inpixels, q = outpixels;
       i < (height * width); i++, p++, q++) {
    if (*p <= LowerThreshold)
      *q = LowerRepVal;
    else if (*p >= UpperThreshold)
      *q = UpperRepVal;
    else
      *q = *p;
  }
#ifdef DEBUG
  printf("Image thresholded [%e,%e]\n", LowerThreshold, UpperThreshold);
#endif
}


/**************************** ZeroCenter ***********************************/
/*                                                                         */
/*  function: set absolute point to zero at the center and position        */
/*            the rest of the image relative to the old value.             */
/*                                                                         */
/***************************************************************************/
void ZeroCenter(double *inpixels, double *outpixels, int height,
		int width, void *ParmPtrs[])
{
  int i;
  double val;

  /* find center value */
  val = inpixels[(height>>1)*width + (width>>1)];

  /* shift data */
  draw_mesg("Zero centering...");
  for (i = 0; i < height*width; i++)
    outpixels[i] = inpixels[i] - val;
}

