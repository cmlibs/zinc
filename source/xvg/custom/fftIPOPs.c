/***********************************************************************
*
*  Name:          fftIPOPs.c
*
*  Author:        Paul Charette
*
*  Last Modified: 26 Nov 1994
*
*  Purpose:       Utility routines FFT IP operations
*                    ApplyWindow()
*                    AutoCorrelation()
*                    ClearFFTState()
*                    CrossCorrelation()
*                    CustomFilter()
*                    DiskCorrelation()
*                    FFT()
*                    FFTInverse()
*                    HighpassFilter()
*                    InterpolateFFT()
*                    LowpassFilter()
*                    NotchFilter()
*                    ScaleImageFFT()
*                    ShiftImageFFT()
*                    ZoomImageFFT()
*
***********************************************************************/
#include <stdio.h>
#include "UxXt.h"
#include <fcntl.h>
#include <search.h>
#include <values.h>
#include "XvgGlobals.h"

/* DAUB4 wavelet transform coefficients */
#define C0 0.4829629131445341
#define C1 0.8365163037378079
#define C2 0.2241438680420134
#define C3 -0.1294095225512604


/***************************** ClearFFTState *******************************/
void ClearFFTState(void)
{
  FFTTrigm = NULL;
  FFTTrign = NULL;
  FFTWork = NULL;
  FFTMask = NULL;
  FFTRealBuffer = NULL;
  FFTImBuffer = NULL;
  FFTWindowBuffer = NULL;
  FFTPreviousWindow = NOWINDOW;
  FFTWindow = HANNINGWINDOW;
  FFTWidth  = 0;
  FFTHeight = 0;
}

/************************** InterpolateFFT *********************************/
/*                                                                         */
/*  function: build interpolated image using sinc functions in FT domain   */
/*                                                                         */
/***************************************************************************/
void InterpolateFFT(double *inpixels, double *outpixels,
		    int height, int width, void *ParmPtrs[])
{
  double ScaleFactor;
  int n, k, xpad, ypad, x, y, lFFTWindow, NewHeight, NewWidth, IFAIL;

  /* load parameters */
  NewHeight = *((int *) ParmPtrs[1]);
  NewWidth = *((int *) ParmPtrs[2]);

  /* init FFT buffers if required */ 
  if (FFTInit(height, width) == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort : First FFTInit() failed in InterpolateFFT()");
    return;
  }
  if (FFTInit2(NewHeight, NewWidth) == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort : Second FFTInit() failed in InterpolateFFT()");
    return;
  }

  /* set correct window */
  lFFTWindow = FFTWindow;
  FFTWindow = NOWINDOW;

  /* compute FT of the input image */
  draw_mesg("InterpolateFFT() - Calculating input image FFT...");
  FFTApplyWindow(inpixels, FFTRealBuffer, FFTImBuffer);
  IFAIL = 1;
  c06fuf(&width, &height, FFTRealBuffer, FFTImBuffer,
	 "Initial", FFTTrigm, FFTTrign, FFTWork, &IFAIL);
  if (IFAIL != 0) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort:IFAIL=%d, c06fuf() in InterpolateFFT()", IFAIL);
    draw_mesg(cbuf);
    return;
  }
  
  /* flip the quadrants of the decimation image FT */
  FFTShift(FFTRealBuffer, FFTRealBuffer, height, width);
    FFTShift(FFTImBuffer, FFTImBuffer, height, width);
  
  /* clear the interpolated FT */
  for (k = 0; k < NewHeight*NewWidth; k++) {
    FFTRealBuffer2[k] = 0;
    FFTImBuffer2[k] = 0;
  }
  
  /* transfer the data to the interpolated FT buffers and scale */
  ScaleFactor = sqrt(((double)(NewHeight*NewWidth))/((double)(height*width)));
  xpad = (NewWidth - width)/2;
  ypad = (NewHeight - height)/2;
  for (y = ypad, k = 0; y < (NewHeight - ypad); y++) {
    for (x = xpad; x < (NewWidth - xpad); x++, k++) {
      FFTRealBuffer2[y*NewWidth + x] = FFTRealBuffer[k] * ScaleFactor;
      FFTImBuffer2[y*NewWidth + x] = FFTImBuffer[k] * ScaleFactor;
    }
  }
  
  /* un-flip quadrants */
  FFTShift(FFTRealBuffer2, FFTRealBuffer2, NewHeight, NewWidth);
  FFTShift(FFTImBuffer2, FFTImBuffer2, NewHeight, NewWidth);
  
  /* inverse FFT */
  draw_mesg("InterpolateFFT() - Calculating output image FFT...");
  n = NewHeight*NewWidth;
  IFAIL = 1;
  c06gcf(FFTImBuffer2, &n, &IFAIL);
  if (IFAIL != 0) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "Abort : IFAIL = %d, c06gcf() in InterpolateFFT()", IFAIL);
    draw_mesg(cbuf);
    return;
  }
  IFAIL = 1;
  c06fuf(&NewWidth, &NewHeight, FFTRealBuffer2, FFTImBuffer2,
	 "Initial", FFTTrigm2, FFTTrign2, FFTWork2, &IFAIL);
  if (IFAIL != 0) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "Abort:IFAIL = %d, c06fuf(2) in InterpolateFFT() ", IFAIL);
    draw_mesg(cbuf);
    return;
  }
  
  /* restore correct window */
  FFTWindow = lFFTWindow;

  /* debugging */
  WriteMatlabFile(NewHeight, NewWidth, "xvgimage", FFTRealBuffer2,
		  ParmPtrs[0]);
}  

/************************** AutoCorrelation ********************************/
/*                                                                         */
/*  function: autocorrelation in the Fourier domain.                       */
/*                                                                         */
/***************************************************************************/
void AutoCorrelation(double *inpixels, double *outpixels,
		     int height, int width, void *ParmPtrs[])
{
  int i, x, y, w, h, n, padding, IFAIL, lwidth, lheight;

  /* load parameters */
  padding = *((int *) ParmPtrs[0]);
  if ((padding >= height/2) || (padding >= width/2) || (padding < 0)) {
    Abort = B_TRUE;
    draw_mesg("Abort : Invalid padding paramater in AutoCorrelation()");
    return;
  }

  /* extend the image size */
  lwidth = width + (2*padding);
  lheight = height + (2*padding);
  
  /* init FFT buffers if required */ 
  if (FFTInit(lheight, lwidth) == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort : FFTInit() failed in AutoCorrelation()");
    return;
  }

  /* zero padd the current image data and compute the FFT */
  draw_mesg("AutoCorrelation - Computing image FFT ");
  for (i = 0; i < lheight*lwidth; i++) {
    FFTRealBuffer[i] = 0;
    FFTImBuffer[i] = 0;
  }
  for (y = padding, i = 0; y < (lheight - padding); y++)
    for (x = padding; x < (lwidth - padding); x++, i++)
      FFTRealBuffer[y*lwidth + x] = inpixels[i];
  IFAIL = 1;
  c06fuf(&lwidth, &lheight, FFTRealBuffer, FFTImBuffer, "Initial", FFTTrigm,
	 FFTTrign, FFTWork, &IFAIL);
  if (IFAIL != 0) {
    Abort = B_TRUE;
    sprintf(cbuf,"Abort:IFAIL=%d returning from c06fuf() in AutoCorrelation()",
	    IFAIL);
    draw_mesg(cbuf);
    return;
  }

  /* multiply both FT by itself */
  draw_mesg("AutoCorrelation - Multiplying FTs");
  for (i = 0; i < (lheight * lwidth); i++) {
    FFTRealBuffer[i] = FFTRealBuffer[i]*FFTRealBuffer[i]
      + FFTImBuffer[i]*FFTImBuffer[i];
    FFTImBuffer[i] = 0;
  }
  
  /* inverse FFT of result */
  draw_mesg("AutoCorrelation - Computing inverse FFT");
  n = lheight*lwidth;
  IFAIL = 1;
  c06gcf(FFTImBuffer, &n, &IFAIL);
  if (IFAIL != 0) {
    Abort = B_TRUE;
    sprintf(cbuf,"Abort:IFAIL=%d returning from c06gcf() in AutoCorrelation()",
	    IFAIL);
    draw_mesg(cbuf);
    return;
  }
  IFAIL = 1;
  c06fuf(&lwidth, &lheight, FFTRealBuffer, FFTImBuffer,
	 "Subsequent", FFTTrigm, FFTTrign, FFTWork, &IFAIL);
  if (IFAIL != 0) {
    Abort = B_TRUE;
    sprintf(cbuf,"Abort:IFAIL=%d on return from c06fuf() in AutoCorrelation()",
	    IFAIL);
    draw_mesg(cbuf);
    return;
  }

  /* exchange the FFT quadrants */
  draw_mesg("AutoCorrelation - Flipping quadrants");
  FFTShift(FFTRealBuffer, FFTRealBuffer, lheight, lwidth);

  /* crop the data and copy to the output buffer */
  draw_mesg("AutoCorrelation - Cropping");
  for (y = padding, i = 0; y < (lheight - padding); y++)
    for (x = padding; x < (lwidth - padding); x++, i++)
      outpixels[i] = FFTRealBuffer[y*lwidth + x];
}


/*************************** CrossCorrelation *******************************/
void CrossCorrelation(double *inpixels, double *outpixels,
		      int height, int width, void *ParmPtrs[])
{
  int i, j, k, x, y, w, h, n, padding, lwidth, lheight, IFAIL;
  double sr, si, *LocalBuf, m;

  /* load parameters */
  padding = *((int *) ParmPtrs[0]);
  if ((padding >= height/2) || (padding >= width/2) || (padding < 0)) {
    Abort = B_TRUE;
    draw_mesg("Abort : Invalid padding paramater in CrossCorrelation()");
    return;
  }

  /* check that stack is not empty, then decrement stack pointer  */
  if (ImageStackTop <= 0) {
    Abort = B_TRUE;
    draw_mesg("Abort : Image stack is empty in Correlation()");
    return;
  }
  
  /* check that height and width match the current image size */
  if   ((ImageStack[ImageStackTop-1].height != height)
	|| (ImageStack[ImageStackTop-1].width != width)) {
    Abort = B_TRUE;
    draw_mesg("Abort:Image on stack is not the current size in Correlation()");
    return;
  }
  ImageStackTop--;
  UpdateStackLabel(ImageStackTop);
  
  /* extend the image size parameters */
  lwidth = width + (2*padding);
  lheight = height + (2*padding);

  /* allocate temporary storage */
  LocalBuf = NULL;
  if ((LocalBuf = (double *) XvgMM_Alloc(NULL, lheight*lwidth*sizeof(double)))
      == NULL) {
    Abort = B_TRUE;
    draw_mesg("Abort : Can't allocate in Correlation()");
    return;
  }

  /* init FFT buffers if required */ 
  if (FFTInit(lheight, lwidth) == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort : FFTInit() failed in Correlation()");
    return;
  }

  /* clear the FT buffers */
  for (i = 0; i < lheight*lwidth; i++) {
    FFTRealBuffer[i] = 0;
    FFTMask[i] = 0;
  }

  /* zero padd the stack image data */
  draw_mesg("CrossCorrelation - Zero padding IMG1");
  for (y = padding, i = 0; y < (lheight - padding); y++)
    for (x = padding; x < (lwidth - padding); x++, i++)
      FFTMask[y*lwidth + x] = ImageStack[ImageStackTop].pixels[i];

  /* window the data */
  draw_mesg("CrossCorrelation - Windowing IMG1");
  FFTApplyWindow(FFTMask, FFTMask, LocalBuf);

  /* compute the FFT of the stack image */
  draw_mesg("CrossCorrelation - Computing the IMG1 FFT ");
  IFAIL = 1;
  c06fuf(&lwidth, &lheight, FFTMask, LocalBuf, "Initial",
	 FFTTrigm, FFTTrign, FFTWork, &IFAIL);
  if (IFAIL != 0) {
    Abort = B_TRUE;
    sprintf(cbuf,"Abort:IFAIL=%d returning in c06fuf() in CrossCorrelation()",
	    IFAIL);
    draw_mesg(cbuf);
    return;
  }

  /* zero padd the current image data and compute the FFT */
  draw_mesg("CrossCorrelation - Zero padding IMG2");
  for (y = padding, i = 0; y < (lheight - padding); y++)
    for (x = padding; x < (lwidth - padding); x++, i++)
      FFTRealBuffer[y*lwidth + x] = inpixels[i];

  /* window the data */
  draw_mesg("CrossCorrelation - Windowing IMG2");
  FFTApplyWindow(FFTRealBuffer, FFTRealBuffer, FFTImBuffer);

  /* compute the FFT of the second image */
  draw_mesg("CrossCorrelation - Computing the IMG2 FFT ");
  IFAIL = 1;
  c06fuf(&lwidth, &lheight, FFTRealBuffer, FFTImBuffer, "Initial", FFTTrigm,
	 FFTTrign, FFTWork, &IFAIL);
  if (IFAIL != 0) {
    Abort = B_TRUE;
    sprintf(cbuf,"Abort:IFAIL=%d after c06fuf() in CrossCorrelation()",
	    IFAIL);
    draw_mesg(cbuf);
    return;
  }

  /* multiply both FTs together */
  draw_mesg("CrossCorrelation - Multiplying FTs");
  for (i = 0; i < (lheight * lwidth); i++) {
    sr = FFTRealBuffer[i]*FFTMask[i] + FFTImBuffer[i]*LocalBuf[i];
    si = FFTImBuffer[i]*FFTMask[i] - FFTRealBuffer[i]*LocalBuf[i];
    FFTRealBuffer[i] = sr;
    FFTImBuffer[i] = si;
  }
  
  /* inverse FFT of result */
  draw_mesg("CrossCorrelation - Computing inverse FFT");
  n = lheight*lwidth;
  IFAIL = 1;
  c06gcf(FFTImBuffer, &n, &IFAIL);
  if (IFAIL != 0) {
    Abort = B_TRUE;
    sprintf(cbuf,"Abort:IFAIL=%d on ret from c06gcf() in CrossCorrelation()",
	    IFAIL);
    draw_mesg(cbuf);
    return;
  }
  IFAIL = 1;
  c06fuf(&lwidth, &lheight, FFTRealBuffer, FFTImBuffer,
	 "Subsequent", FFTTrigm, FFTTrign, FFTWork, &IFAIL);
  if (IFAIL != 0) {
    Abort = B_TRUE;
    sprintf(cbuf,"Abort:IFAIL=%d on ret from c06fuf() in CrossCorrelation()",
	    IFAIL);
    draw_mesg(cbuf);
    return;
  }

  /* exchange the FFT quadrants */
  draw_mesg("CrossCorrelation - Flipping quadrants");
  FFTShift(FFTRealBuffer, FFTRealBuffer, lheight, lwidth);

  /* crop the data and transfer to the output buffer */
  draw_mesg("CrossCorrelation - Cropping");
  for (y = padding, i = 0; y < (lheight - padding); y++)
    for (x = padding; x < (lwidth - padding); x++, i++)
      outpixels[i] = FFTRealBuffer[y*lwidth + x];

  /* find shift of the peak */
  for (j = 0, k = 0, m = -MAXDOUBLE; j < height; j++)
    for (i = 0; i < width; i++, k++)
      if (outpixels[k] > m) {
	m = outpixels[k];
	x = i;
	y = j;
      }
  if (VerboseFlag)
    printf("CrossCorrelation : max at (%d,%d) which is (%d,%d) from center\n",
	   x, y, x - width/2, y - height/2);
  
  /* free local storage */
  XvgMM_Free(LocalBuf);
}

/***************************** Highpass filter *****************************/
void CustomFilter(double *inpixels, double *outpixels,
		  int height, int width, void *ParmPtrs[])
{
  int i, IFAIL, n;

  /* check that stack is not empty, then decrement stack pointer  */
  if (ImageStackTop <= 0) {
    Abort = B_TRUE;
    draw_mesg("Abort : Image stack is empty in CustomFilter()");
    return;
  }
  
  /* check that height and width match the current image size */
  if   ((ImageStack[ImageStackTop-1].height != height)
	|| (ImageStack[ImageStackTop-1].width != width)) {
    Abort = B_TRUE;
    draw_mesg("Abort:Image on stack isn't the current size in CustomFilter()");
    return;
  }
  ImageStackTop--;
  UpdateStackLabel(ImageStackTop);
  
  /* init buffers if required */
  if (FFTInit(height, width) == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort : FFTInit() failed in CustomFilter()");
    return;
  }
  
  /* window the data */
  draw_mesg("Custom filter - Windowing");
  FFTApplyWindow(inpixels, FFTRealBuffer, FFTImBuffer);

  /* perform FFT */
  draw_mesg("Custom filter - Computing FFT");
  IFAIL = 1;
  c06fuf(&width, &height, FFTRealBuffer, FFTImBuffer,
	 "Initial", FFTTrigm, FFTTrign, FFTWork, &IFAIL);
  if (IFAIL != 0) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "Abort : IFAIL = %d on return from c06fuf() in CustomFilter() ",
	    IFAIL);
    draw_mesg(cbuf);
    return;
  }

  /* multiply by the filter image */
  draw_mesg("Custom filter - Multiplying by filter mask");
  FFTShift(FFTRealBuffer, FFTRealBuffer, height, width);
  FFTShift(FFTImBuffer, FFTImBuffer, height, width);
  for (i = 0; i < (height * width); i++) {
    FFTRealBuffer[i] *= ImageStack[ImageStackTop].pixels[i];
    FFTImBuffer[i] *= ImageStack[ImageStackTop].pixels[i];
  }
  FFTShift(FFTRealBuffer, FFTRealBuffer, height, width);
  FFTShift(FFTImBuffer, FFTImBuffer, height, width);

  /* inverse FFT */
  draw_mesg("Custom filter - Computing inverse FFT");
  n = height * width;
  IFAIL = 1;
  c06gcf(FFTImBuffer, &n, &IFAIL);
  if (IFAIL != 0) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "Abort : IFAIL = %d on return from c06gcf() in CustomFilter() ",
	    IFAIL);
    draw_mesg(cbuf);
    return;
  }
  IFAIL = 1;
  c06fuf(&width, &height, FFTRealBuffer, FFTImBuffer,
	 "Subsequent", FFTTrigm, FFTTrign, FFTWork, &IFAIL);
  if (IFAIL != 0) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "Abort : IFAIL = %d on return from c06fuf() in CustomFilter() ",
	    IFAIL);
    draw_mesg(cbuf);
    return;
  }
  IFAIL = 1;
  c06gcf(FFTImBuffer, &n, &IFAIL);
  if (IFAIL != 0) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "Abort : IFAIL = %d on return from c06gcf() in CustomFilter() ",
	    IFAIL);
    draw_mesg(cbuf);
    return;
  }

  /* inverse window the data and copy to the output buffer */
  draw_mesg("Custom filter - Inverse windowing");
  FFTApplyInverseWindow(FFTRealBuffer, outpixels);
}

/************************** DiskCorrelation ********************************/
/*                                                                         */
/*  function: correlation with a disk in the Fourier domain. The result    */
/*            is binarized using a specified fraction of the maximum       */
/*            value as a threshold. Used for large disk area morphology.   */
/*                                                                         */
/***************************************************************************/
void DiskCorrelation(double *inpixels, double *outpixels,
		     int height, int width, void *ParmPtrs[])
{
  int i, x, y, w, h, n, radius, lheight, lwidth, IFAIL;
  double sr, si, max, scale, *LocalBuf;
  
  /* load parameters */
  radius = *((int *) ParmPtrs[0]);
  if ((radius >= height/2) || (radius >= width/2) || (radius < 1)) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort : Invalid radius paramater (%d) in DiskCorrelation()",
	    radius);
    draw_mesg(cbuf);
    return;
  }
  scale = *((double *) ParmPtrs[1]);
  if ((scale < 0) || (scale > 1)) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort : Invalid scale paramater (%f) in DiskCorrelation()",
	    scale);
    draw_mesg(cbuf);
    return;
  }

  /* extend the image size parameters */
  lwidth = width + (2*radius);
  lheight = height + (2*radius);

  /* allocate temporary storage */
  LocalBuf = NULL;
  if ((LocalBuf = (double *) XvgMM_Alloc(NULL, lheight*lwidth*sizeof(double)))
      == NULL) {
    Abort = B_TRUE;
    draw_mesg("Abort : Can't allocate in DiskCorrelation()");
    return;
  }

  /* init buffers, create the disk image and FT */
  draw_mesg("DiskCorrelation - Allocating storage");
  if (FFTInit(lheight, lwidth) == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort : FFTInit() failed in DiskCorrelation()");
    return;
  }
  
  /* create the disk image */
  draw_mesg("DiskCorrelation - Creating disk image");
  for (i = 0; i < lheight*lwidth; i++) {
    FFTMask[i] = 0;
    LocalBuf[i] = 0;
  }
  for (y = lheight/2 - radius, h = -radius; y <= lheight/2 + radius; y++,h++) {
    w = sqrt((double) (radius*radius - h*h));
    for (x = lwidth/2 - w; x <= lwidth/2 + w; x++) 
      FFTMask[y*lwidth + x] = 1.0;
  }
  
  /* compute the disk FFT */
  draw_mesg("DiskCorrelation - Computing disk FFT ");
  IFAIL = 1;
  c06fuf(&lwidth, &lheight, FFTMask, LocalBuf, "Initial",
	 FFTTrigm, FFTTrign, FFTWork, &IFAIL);
  if (IFAIL != 0) {
    Abort = B_TRUE;
    sprintf(cbuf,"Abort:IFAIL=%d returning in c06fuf() in DiskCorrelation()",
	    IFAIL);
    draw_mesg(cbuf);
    return;
  }

  /* zero padd the current image data by the disk radius and compute the FFT */
  draw_mesg("DiskCorrelation - Computing image FFT ");
  for (i = 0; i < lheight*lwidth; i++) {
    FFTRealBuffer[i] = 0;
    FFTImBuffer[i] = 0;
  }
  for (y = radius, i = 0; y < (lheight - radius); y++)
    for (x = radius; x < (lwidth - radius); x++, i++)
      FFTRealBuffer[y*lwidth + x] = inpixels[i];
  IFAIL = 1;
  c06fuf(&lwidth, &lheight, FFTRealBuffer, FFTImBuffer, "Initial", FFTTrigm,
	 FFTTrign, FFTWork, &IFAIL);
  if (IFAIL != 0) {
    Abort = B_TRUE;
    sprintf(cbuf,"Abort:IFAIL=%d returning from c06fuf() in DiskCorrelation()",
	    IFAIL);
    draw_mesg(cbuf);
    return;
  }

  /* multiply both FTs together */
  draw_mesg("DiskCorrelation - Multiplying FTs");
  for (i = 0; i < (lheight * lwidth); i++) {
    sr= FFTRealBuffer[i]*FFTMask[i] + FFTImBuffer[i]*LocalBuf[i];
    si= FFTImBuffer[i]*FFTMask[i] - FFTRealBuffer[i]*LocalBuf[i];
    FFTRealBuffer[i] = sr;
    FFTImBuffer[i] = si;
  }
  
  /* inverse FFT of result */
  draw_mesg("DiskCorrelation - Computing inverse FFT");
  n = lheight*lwidth;
  IFAIL = 1;
  c06gcf(FFTImBuffer, &n, &IFAIL);
  if (IFAIL != 0) {
    Abort = B_TRUE;
    sprintf(cbuf,"Abort:IFAIL=%d returning from c06gcf() in DiskCorrelation()",
	    IFAIL);
    draw_mesg(cbuf);
    return;
  }
  IFAIL = 1;
  c06fuf(&lwidth, &lheight, FFTRealBuffer, FFTImBuffer,
	 "Subsequent", FFTTrigm, FFTTrign, FFTWork, &IFAIL);
  if (IFAIL != 0) {
    Abort = B_TRUE;
    sprintf(cbuf,"Abort:IFAIL=%d on return from c06fuf() in DiskCorrelation()",
	    IFAIL);
    draw_mesg(cbuf);
    return;
  }

  /* exchange the FFT quadrants */
  draw_mesg("DiskCorrelation - Flipping quadrants");
  FFTShift(FFTRealBuffer, FFTRealBuffer, lheight, lwidth);

  /* crop the data and find data maximum */
  draw_mesg("DiskCorrelation - Cropping");
  for (y = radius, i = 0, max = -MAXDOUBLE; y < (lheight - radius); y++)
    for (x = radius; x < (lwidth - radius); x++, i++) {
      outpixels[i] = FFTRealBuffer[y*lwidth + x];
      max = (outpixels[i] > max ? outpixels[i] : max);
    }
  
  /* binarize the resulting image */
  draw_mesg("DiskCorrelation - Binarizing");
  for (i = 0, max *= scale; i < height*width; i++)
    outpixels[i] = (outpixels[i] >= max ? 1.0 : 0.0);

  /* free local storage */
  XvgMM_Free(LocalBuf);
}


/************************ FFTApplyInverseWindow *****************************/
void FFTApplyInverseWindow(double *RealBuffer, double *outpixels)
{
  int i, j, k;
  double WidthInv, HeightInv, WidthMult, HeightMult, s, xm, ym;
  
  switch (FFTWindow) {
  case HANNINGWINDOW:
    /* apply the inverse hanning window */
    for (i = 0, k = 0; i < FFTHeight; i++) {
      for (j = 0; j < FFTWidth; j++, k++) {
	if (FFTWindowBuffer[k] < 0.01)
	  outpixels[k] = RealBuffer[k]/0.01;
	else
	  outpixels[k] = RealBuffer[k]/FFTWindowBuffer[k];
      }
    }
    break;

  case TRIANGULARWINDOW:
    HeightMult = ((double) FFTHeight) / 2.0;
    HeightInv = 2.0 / ((double) FFTHeight);
    WidthMult = ((double) FFTWidth) / 2.0;
    WidthInv = 2.0 / ((double) FFTWidth);
    for (i = 0, k = 0; i < FFTHeight; i++)
      for (j = 0; j < FFTWidth; j++, k++) {
	s = (1 - abs((i - HeightMult)*HeightInv))
	  * (1 - abs((j - WidthMult)*WidthInv));
	if (s < 0.01)
	  s = 0.01;
	outpixels[k] = RealBuffer[k]/s;
      }
    break;
  case HALFSIZETRIANGULARWINDOW:
    HeightMult = ((double) FFTHeight) / 2.0;
    HeightInv = 1.0 / HeightMult;
    WidthMult = ((double) FFTWidth) / 2.0;
    WidthInv = 1.0 / WidthMult;
    for (j = 0, k = 0; j < FFTHeight; j++) {
      ym = 1.0 - fabs(((double) j) - HeightMult)*HeightInv*2.0;
      for (i = 0; i < FFTWidth; i++, k++) {
	xm = 1.0 - fabs(((double) i) - WidthMult)*WidthInv*2.0;
	if ((xm > 0.0) && (ym > 0.0))
	  outpixels[k] = RealBuffer[k] / (xm * ym);
	else
	  outpixels[k] = 0;
      }
    }
    break;
  case NOWINDOW:
  default:
    for (i = 0; i < (FFTHeight * FFTWidth); i++)
      outpixels[i] = RealBuffer[i];
    break;
  }
}


/****************************** FFTWindow ***********************************/
void FFTApplyWindow(double *inpixels, double *RealBuffer,
		    double *ImBuffer)
{
  int i, j, k;
  double WidthInv, HeightInv, WidthMult, HeightMult, xm, ym;

  switch (FFTWindow) {
  case HANNINGWINDOW:
    /* define the Hanning window if required */
    if (FFTPreviousWindow != FFTWindow) {
      WidthInv = TWOPI / ((double) FFTWidth);
      HeightInv = TWOPI / ((double) FFTHeight);
      for (i = 0, k = 0; i < FFTHeight; i++) {
	for (j = 0; j < FFTWidth; j++, k++) {
	  FFTWindowBuffer[k] = 0.25
	    * (1.0 - cos(((double) i)*HeightInv))
	      * (1.0 - cos(((double) j)*WidthInv));
	}
      }
      if (VerboseFlag)
	printf("FFTApplyWindow() : Window defined...\n");
    }
    
    /* apply the Hanning window */
    for (i = 0, k = 0; i < FFTHeight; i++) {
      for (j = 0; j < FFTWidth; j++, k++) {
	RealBuffer[k] = inpixels[k] * FFTWindowBuffer[k];
	ImBuffer[k]   = 0.0;
      }
    }
    
    /* flag this buffer definition */
    FFTPreviousWindow = HANNINGWINDOW;
    break;

  case TRIANGULARWINDOW:
    HeightMult = ((double) FFTHeight) / 2.0;
    HeightInv = 2.0 / ((double) FFTHeight);
    WidthMult = ((double) FFTWidth) / 2.0;
    WidthInv = 2.0 / ((double) FFTWidth);
    for (i = 0, k = 0; i < FFTHeight; i++)
      for (j = 0; j < FFTWidth; j++, k++) {
	RealBuffer[k] = inpixels[k]
	  * (1.0 - fabs((((double) i) - HeightMult)*HeightInv))
	    * (1.0 - fabs((((double) j) - WidthMult)*WidthInv));
	ImBuffer[k] = 0;
      }
    break;
  case HALFSIZETRIANGULARWINDOW:
    HeightMult = ((double) FFTHeight) / 2.0;
    HeightInv = 1.0 / HeightMult;
    WidthMult = ((double) FFTWidth) / 2.0;
    WidthInv = 1.0 / WidthMult;
    for (j = 0, k = 0; j < FFTHeight; j++) {
      ym = 1.0 - fabs(((double) j) - HeightMult)*HeightInv*2.0;
      for (i = 0; i < FFTWidth; i++, k++) {
	xm = 1.0 - fabs(((double) i) - WidthMult)*WidthInv*2.0;
	if ((xm > 0.0) && (ym > 0.0))
	  RealBuffer[k] = inpixels[k] * xm * ym;
	else
	  RealBuffer[k] = 0;
	ImBuffer[k] = 0;
      }
    }
    break;
  case NOWINDOW:
  default:
    for (i = 0; i < (FFTHeight * FFTWidth); i++) {
      RealBuffer[i] = inpixels[i];
      ImBuffer[i] = 0;
    }
    break;
  }
}


/****************************       FFT    *********************************/
void FFT(double *inpixels, double *outpixels,
	 int height, int width, void *ParmPtrs[])
{
  double *im;
  int IFAIL;

  /* make room on the stack for the imaginary part of the transform */
  PushImage(inpixels, inpixels, height, width, NULL);
  if (Abort) {
    draw_mesg("FFT() : Image stack is full");
    return;
  }
  im = ImageStack[ImageStackTop-1].pixels;

  /* initialization */
  if (FFTInit(height, width) == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort : FFTInit() failed in FFT()");
    return;
  }
  
  /* window the data and zero the imaginary part */
  draw_mesg("FFT - Applying window");
  FFTApplyWindow(inpixels, FFTRealBuffer, FFTImBuffer);
  
  /* compute FFT */
  draw_mesg("FFT - Computing FFT");
  IFAIL = 1;
  c06fuf(&width, &height, FFTRealBuffer, FFTImBuffer,
	 "Initial", FFTTrigm, FFTTrign, FFTWork, &IFAIL);
  if (IFAIL != 0) {
    Abort = B_TRUE;
    sprintf(cbuf,"Abort : IFAIL = %d on return from c06fuf() in FFT() ",IFAIL);
    draw_mesg(cbuf);
    return;
  }
  
  /* FFT shift real data */
  draw_mesg("FFT - Flipping quadrants");
  FFTShift(FFTRealBuffer, outpixels, height, width);
  FFTShift(FFTImBuffer, im, height, width);
}

/******************************** FFT init *********************************/
boolean_t FFTInit(int height, int width)
{
  if ((FFTWidth != width) || (FFTHeight != height)) {
    /* set size */
    FFTWidth = width;
    FFTHeight = height;
    
    /* flag new window mask definition */
    FFTPreviousWindow = NOWINDOW;
    
    /* allocate storage */
    FFTMask = (double *) XvgMM_Alloc((void *) FFTMask,
				     height*width*sizeof(double));
    FFTWindowBuffer = (double *) XvgMM_Alloc((void *) FFTWindowBuffer,
					     height*width*sizeof(double));
    FFTRealBuffer = (double *) XvgMM_Alloc((void *) FFTRealBuffer,
					   height*width*sizeof(double));
    FFTImBuffer = (double *) XvgMM_Alloc((void *) FFTImBuffer,
					 height*width*sizeof(double));
    FFTTrigm = (double *)
      XvgMM_Alloc((void *) FFTTrigm,
		  2*(height > width ? height:width)*sizeof(double));
    FFTTrign = (double *)
      XvgMM_Alloc((void *) FFTTrign,
		  2*(height > width ? height:width)*sizeof(double));
    FFTWork = (double *) XvgMM_Alloc((void *) FFTWork,
				     2*width*height*sizeof(double));
    if ((FFTMask == NULL) || (FFTRealBuffer == NULL) || (FFTImBuffer == NULL)
	|| (FFTTrigm == NULL) || (FFTTrigm == NULL) || (FFTWork == NULL)
	|| (FFTWindowBuffer == NULL))
      return(B_FALSE);
    
    if (VerboseFlag)
      printf("FFTInit(%dx%d)\n", width, height);
  }
  return(B_TRUE);
}

/******************************** FFT init Extra ***************************/
boolean_t FFTInit2(int height, int width)
{
  /* allocate storage */
  FFTRealBuffer2 = (double *) XvgMM_Alloc((void *) FFTRealBuffer2,
					  height*width*sizeof(double));
  FFTImBuffer2 = (double *) XvgMM_Alloc((void *) FFTImBuffer2,
					height*width*sizeof(double));
  FFTTrigm2 = (double *) XvgMM_Alloc((void *) FFTTrigm2,
				     2*(height > width ? height:width)
				     * sizeof(double));
  FFTTrign2 = (double *) XvgMM_Alloc((void *) FFTTrign2,
				     2*(height > width ? height:width)
				     * sizeof(double));
  FFTWork2 = (double *) XvgMM_Alloc((void *) FFTWork2,
				    2*width*height*sizeof(double));
  if (VerboseFlag)
    printf("FFTInit2(%dx%d)\n", width, height);
  if ((FFTRealBuffer2 == NULL) || (FFTImBuffer2 == NULL)
      || (FFTTrigm2 == NULL) || (FFTTrigm2 == NULL) || (FFTWork2 == NULL))
    return(B_FALSE);
  else
    return(B_TRUE);
}

/**************************** FFTInverse    *********************************/
void FFTInverse(double *inpixels, double *outpixels,
		int height, int width, void *ParmPtrs[])
{
  int i, j, type, n, IFAIL;
  double *p1, *p2, *p3, *p4, tmp;

  /* check that an FFT has been computed */
  if ((FFTRealBuffer == NULL) || (FFTImBuffer == NULL)) {
    Abort = B_TRUE;
    draw_mesg("Abort : No defined FT in FFTInverse()");
    return;
  }

  /* inverse FFT */
  draw_mesg("FFTInverse - Computing inverse FFT");
  n = height * width;
  IFAIL = 1;
  c06gcf(FFTImBuffer, &n, &IFAIL);
  if (IFAIL != 0) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "Abort : IFAIL = %d on return from c06gcf() in FFTInverse() ",
	    IFAIL);
    draw_mesg(cbuf);
    return;
  }
  IFAIL = 1;
  c06fuf(&width, &height, FFTRealBuffer, FFTImBuffer,
	 "Subsequent", FFTTrigm, FFTTrign, FFTWork, &IFAIL);
  if (IFAIL != 0) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "Abort : IFAIL = %d on return from c06fuf() in FFTInverse() ",
	    IFAIL);
    draw_mesg(cbuf);
    return;
  }
  IFAIL = 1;
  c06gcf(FFTImBuffer, &n, &IFAIL);
  if (IFAIL != 0) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "Abort : IFAIL = %d on return from c06gcf() in FFTInverse()",
	    IFAIL);
    draw_mesg(cbuf);
    return;
  }

  /* Copy data to the output buffer */
  draw_mesg("FFTInverse - Generating output");
  for (i = 0; i < height*width; i++)
    outpixels[i] = FFTRealBuffer[i];
}


/***************************** FFT Shift *****************************/
void FFTShift(double *inpixels, double *outpixels,
	      int height, int width)
{
  double *ip1, *ip2, *ip3, *ip4, *op1, *op2, *op3, *op4, tmp;
  int i, j;

  for (i = 0; i < height/2; i++) {
    ip1=&(inpixels[i*width]);
    ip2=&(inpixels[i*width + width/2]);
    ip3=&(inpixels[(i+height/2)*width]);
    ip4=&(inpixels[(i+height/2)*width + width/2]);
    op1=&(outpixels[i*width]);
    op2=&(outpixels[i*width + width/2]);
    op3=&(outpixels[(i+height/2)*width]);
    op4=&(outpixels[(i+height/2)*width + width/2]);
    for (j = 0; j < width/2; j++) {
      /* swap quadrants 1 & 4 */
      tmp = ip1[j];
      op1[j] = ip4[j];
      op4[j] = tmp;
      /* swap quadrants 2 & 3 */
      tmp = ip2[j];
      op2[j] = ip3[j];
      op3[j] = tmp;
    }
  }
}

/***************************** Highpass filter *****************************/
void HighpassFilter(double *inpixels, double *outpixels,
		    int height, int width, void *ParmPtrs[])
{
  int i, j, k, IFAIL, n;
  double  Cutoff, LowAsymp, UpAsymp, *ib, *rb, *m, *p, ratio;

  /* load parameters */
  LowAsymp = *((double *) ParmPtrs[0]);
  UpAsymp = *((double *) ParmPtrs[1]);
  Cutoff = *((double *) ParmPtrs[2]);

  /* init buffers if required */
  if (FFTInit(height, width) == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort : FFTInit() failed in HighpassFilter()");
    return;
  }
  
  /* create mask */
  draw_mesg("Highpass filter - Computing mask");
  
  /* compute first quadrant */
  for (i = 0, FFTMask; i < height/2;i++)
    for (j = 0; j < width/2; j++) {
      ratio = Cutoff / sqrt(i*i + j*j);
      FFTMask[i*width + j] = LowAsymp +
	(UpAsymp - LowAsymp) / (1.0 + (0.41421356*ratio*ratio*ratio*ratio));
    }
  /* copy first quadrant to second quadrant */
  for (i = 0; i < height/2; i++)
    for (j = 0, k = width-1; j < width/2; j++, k--)
      FFTMask[i*width + k] = FFTMask[i*width + j];
  
  /* copy quadrants 1&2 to quadrants 3&4 */
  for (i = 0, k = height-1; i < height/2; i++, k--)
    for (j = 0; j < width; j++)
      FFTMask[k*width + j] = FFTMask[i*width + j];
  
  /* window the data and zero the imaginary part */
  draw_mesg("Highpass filter - Windowing");
  FFTApplyWindow(inpixels, FFTRealBuffer, FFTImBuffer);

  /* perform FFT */
  draw_mesg("Highpass filter - Computing FFT");
  IFAIL = 1;
  c06fuf(&width, &height, FFTRealBuffer, FFTImBuffer,
	 "Initial", FFTTrigm, FFTTrign, FFTWork, &IFAIL);
  if (IFAIL != 0) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "Abort : IFAIL = %d on return from c06fuf() in HighpassFilter() ",
	    IFAIL);
    draw_mesg(cbuf);
    return;
  }

  /* multiply by the mask */
  draw_mesg("Highpass filter - Multiplying by filter mask");
  for (i = 0, rb=FFTRealBuffer, ib=FFTImBuffer, m=FFTMask;
       i < (height * width); i++, rb++, ib++, m++) {
    *rb = *rb * *m;
    *ib = *ib * *m;
  }

  /* inverse FFT */
  draw_mesg("Highpass filter - Computing inverse FFT");
  n = height * width;
  IFAIL = 1;
  c06gcf(FFTImBuffer, &n, &IFAIL);
  if (IFAIL != 0) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "Abort : IFAIL = %d on return from c06gcf() in HighpassFilter() ",
	    IFAIL);
    draw_mesg(cbuf);
    return;
  }
  IFAIL = 1;
  c06fuf(&width, &height, FFTRealBuffer, FFTImBuffer,
	 "Subsequent", FFTTrigm, FFTTrign, FFTWork, &IFAIL);
  if (IFAIL != 0) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "Abort : IFAIL = %d on return from c06fuf() in HighpassFilter() ",
	    IFAIL);
    draw_mesg(cbuf);
    return;
  }
  IFAIL = 1;
  c06gcf(FFTImBuffer, &n, &IFAIL);
  if (IFAIL != 0) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "Abort : IFAIL = %d on return from c06gcf() in HighpassFilter() ",
	    IFAIL);
    draw_mesg(cbuf);
    return;
  }

  /* inverse window the data and copy to the output buffer */
  draw_mesg("Highpass filter - Inverse windowing");
  FFTApplyInverseWindow(FFTRealBuffer, outpixels);
}

/***************************** Lowpass filter *****************************/
void LowpassFilter(double *inpixels, double *outpixels,
		   int height, int width, void *ParmPtrs[])
{
  int i, j, k, n, IFAIL;
  double  Cutoff, *ib, *rb, *m, *p, ratio;

  /* load parameters */
  Cutoff = *((double *) ParmPtrs[0]);

  /* init buffers if required */
  if (FFTInit(height, width) == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort : FFTInit() failed in LowpassFilter()");
    return;
  }

  /* create new mask */
  draw_mesg("Lowpass filter - Computing new mask");
  
  /* compute first quadrant */
  for (i = 0, FFTMask; i < height/2;i++)
    for (j = 0; j < width/2; j++) {
      ratio = sqrt(i*i + j*j) / Cutoff;
      FFTMask[i*width + j] =
	1.0 / (1.0 + (0.41421356*ratio*ratio*ratio*ratio));
    }
  /* copy first quadrant to second quadrant */
  for (i = 0; i < height/2; i++)
    for (j = 0, k = width-1; j < width/2; j++, k--)
      FFTMask[i*width + k] = FFTMask[i*width + j];
  
  /* copy quadrants 1&2 to quadrants 3&4 */
  for (i = 0, k = height-1; i < height/2; i++, k--)
    for (j = 0; j < width; j++)
      FFTMask[k*width + j] = FFTMask[i*width + j];
  
  if (IPOpExecVerbose)
    ShowImage(FFTMask);
  
  /* window the data and zero the imaginary part */
  draw_mesg("Lowpass filter - Windowing");
  FFTApplyWindow(inpixels, FFTRealBuffer, FFTImBuffer);
  if (IPOpExecVerbose)
    ShowImage(FFTRealBuffer);

  /* perform FFT */
  draw_mesg("Lowpass filter - Computing FFT");
  IFAIL = 1;
  c06fuf(&width, &height, FFTRealBuffer, FFTImBuffer,
	 "Initial", FFTTrigm, FFTTrign, FFTWork, &IFAIL);
  if (IFAIL != 0) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "Abort : IFAIL = %d on return from c06fuf() in LowpassFilter() ",
	    IFAIL);
    draw_mesg(cbuf);
    return;
  }
  if (IPOpExecVerbose)
    ShowImage(FFTRealBuffer);

  /* multiply by the mask */
  draw_mesg("Lowpass filter - Multiplying by filter mask");
  for (i = 0, rb=FFTRealBuffer, ib=FFTImBuffer, m=FFTMask;
       i < (height * width); i++, rb++, ib++, m++) {
    *rb = *rb * *m;
    *ib = *ib * *m;
  }
  if (IPOpExecVerbose)
    ShowImage(FFTRealBuffer);

  /* inverse FFT */
  draw_mesg("Lowpass filter - Computing inverse FFT");
  n = height*width;
  IFAIL = 1;
  c06gcf(FFTImBuffer, &n, &IFAIL);
  if (IFAIL != 0) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "Abort : IFAIL = %d on return from c06gcf() in LowpassFilter() ",
	    IFAIL);
    draw_mesg(cbuf);
    return;
  }
  IFAIL = 1;
  c06fuf(&width, &height, FFTRealBuffer, FFTImBuffer,
	 "Subsequent", FFTTrigm, FFTTrign, FFTWork, &IFAIL);
  if (IFAIL != 0) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "Abort : IFAIL = %d on return from c06fuf() in LowpassFilter() ",
	    IFAIL);
    draw_mesg(cbuf);
    return;
  }
  IFAIL = 1;
  c06gcf(FFTImBuffer, &n, &IFAIL);
  if (IFAIL != 0) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "Abort : IFAIL = %d on return from c06gcf() in LowpassFilter() ",
	    IFAIL);
    draw_mesg(cbuf);
    return;
  }

  /* inverse window the data and copy to the output buffer */
  draw_mesg("Lowpass filter - Inverse windowing");
  FFTApplyInverseWindow(FFTRealBuffer, outpixels);
}

  
/***************************** Notch filter *****************************/
void NotchFilter(double *inpixels, double *outpixels,
		 int height, int width, void *ParmPtrs[])
{
  int cx, cy, i, j, k, dx, dy, radius, IFAIL, n;

  /* load parameters */
  dx = *((int *) ParmPtrs[0]);
  dy = *((int *) ParmPtrs[1]);
  cx = width/2;
  cy = height/2;

  /* init buffers if required */
  if (FFTInit(height, width) == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort : FFTInit() failed in NotchFilter()");
    return;
  }
  
  /* perform FFT */
  draw_mesg("Notch filter - Computing FFT");
  FFTApplyWindow(inpixels, FFTRealBuffer, FFTImBuffer);
  IFAIL = 1;
  c06fuf(&width, &height, FFTRealBuffer, FFTImBuffer,
	 "Initial", FFTTrigm, FFTTrign, FFTWork, &IFAIL);
  if (IFAIL != 0) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "Abort : IFAIL = %d on return from c06fuf() in NotchFilter() ",
	    IFAIL);
    draw_mesg(cbuf);
    return;
  }
  FFTShift(FFTRealBuffer, FFTRealBuffer, height, width);
  FFTShift(FFTImBuffer, FFTImBuffer, height, width);

  /* implement the notch */
  draw_mesg("Notch filter - Multiplying by filter mask");
  FFTRealBuffer[(cy + dy)*width + dx + cx] = 0;
  FFTImBuffer[(cy + dy)*width + dx + cx] = 0;
  FFTRealBuffer[(cy + dy - 1)*width + dx + cx] *= 0.5;
  FFTImBuffer[(cy + dy - 1)*width + dx + cx] *= 0.5;
  FFTRealBuffer[(cy + dy + 1)*width + dx + cx] *= 0.5;
  FFTImBuffer[(cy + dy + 1)*width + dx + cx] *= 0.5;
  FFTRealBuffer[(cy + dy)*width + dx + cx - 1] *= 0.5;
  FFTImBuffer[(cy + dy)*width + dx + cx - 1] *= 0.5;
  FFTRealBuffer[(cy + dy)*width + dx + cx + 1] *= 0.5;
  FFTImBuffer[(cy + dy)*width + dx + cx + 1] *= 0.5;
  FFTRealBuffer[(cy - dy)*width - dx + cx] = 0;
  FFTImBuffer[(cy - dy)*width - dx + cx] = 0;
  FFTRealBuffer[(cy - dy - 1)*width - dx + cx] *= 0.5;
  FFTImBuffer[(cy - dy - 1)*width - dx + cx] *= 0.5;
  FFTRealBuffer[(cy - dy + 1)*width - dx + cx] *= 0.5;
  FFTImBuffer[(cy - dy + 1)*width - dx + cx] *= 0.5;
  FFTRealBuffer[(cy - dy)*width - dx + cx - 1] *= 0.5;
  FFTImBuffer[(cy - dy)*width - dx + cx - 1] *= 0.5;
  FFTRealBuffer[(cy - dy)*width - dx + cx + 1] *= 0.5;
  FFTImBuffer[(cy - dy)*width - dx + cx + 1] *= 0.5;
  FFTShift(FFTRealBuffer, FFTRealBuffer, height, width);
  FFTShift(FFTImBuffer, FFTImBuffer, height, width);

  /* inverse FFT */
  draw_mesg("Notch filter - Computing inverse FFT");
  n = height * width;
  IFAIL = 1;
  c06gcf(FFTImBuffer, &n, &IFAIL);
  if (IFAIL != 0) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "Abort : IFAIL = %d on return from c06gcf() in NotchFilter() ",
	    IFAIL);
    draw_mesg(cbuf);
    return;
  }
  IFAIL = 1;
  c06fuf(&width, &height, FFTRealBuffer, FFTImBuffer,
	 "Subsequent", FFTTrigm, FFTTrign, FFTWork, &IFAIL);
  if (IFAIL != 0) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "Abort : IFAIL = %d on return from c06fuf() in NotchFilter() ",
	    IFAIL);
    draw_mesg(cbuf);
    return;
  }
  IFAIL = 1;
  c06gcf(FFTImBuffer, &n, &IFAIL);
  if (IFAIL != 0) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "Abort : IFAIL = %d on return from c06gcf() in NotchFilter() ",
	    IFAIL);
    draw_mesg(cbuf);
    return;
  }

  /* inverse window the data and copy to the output buffer */
  draw_mesg("Notch filter - Inverse windowing");
  FFTApplyInverseWindow(FFTRealBuffer, outpixels);
}

/*
WARNING: ARRAY LENGHTS MUST BE POWERS OF 2 ONLY!!!!!!
*/

static void DAUB4(double *a, double *wksp, unsigned long n, int isign)
{
  unsigned long halfn, nh1, i, j;

  /* check input parms */
  if (n < 4)
    return;
  
  /* init parameters */
  halfn = n >> 1;
  nh1   = halfn + 1;

  /* forward wavelet transform */
  if (isign >= 0) {
    for (i=1, j=1; j <= n-3; j+=2,i++) {
      wksp[i]       = C0*a[j] + C1*a[j+1] + C2*a[j+2] + C3*a[j+3];
      wksp[i+halfn] = C3*a[j] - C2*a[j+1] + C1*a[j+2] - C0*a[j+3];
    }
    wksp[i]       = C0*a[n-1] + C1*a[n] + C2*a[1] + C3*a[2];
    wksp[i+halfn] = C3*a[n-1] - C2*a[n] + C1*a[1] - C0*a[2];
  }

  /* inverse wavelet transform */
  else {
    wksp[1] = C2*a[halfn] + C1*a[n] + C0*a[1] + C3*a[nh1];
    wksp[2] = C3*a[halfn] - C0*a[n] + C1*a[1] - C2*a[nh1];
    for (i=1, j=3; i < halfn; i++) {
      wksp[j++] = C2*a[i] + C1*a[i+halfn] + C0*a[i+1] + C3*a[i+nh1];
      wksp[j++] = C3*a[i] - C0*a[i+halfn] + C1*a[i+1] - C2*a[i+nh1];
    }
  }

  /* load results into output array */
  for (i=1; i <= n; i++)
    a[i] = wksp[i];
}

void WaveletTransform(double *inpixels, double *outpixels, int height, int width,
		      void *ParmPtrs[])
{
  double *buf, *wksp;
  int isign, nmax;
  unsigned long i, j, k, n;
  
  /* load params */
  isign = *((int *) ParmPtrs[0]);

  /* allocate storage */
  nmax = (width > height ? width : height) + 1;
  if ((buf = (double *) XvgMM_Alloc(NULL, nmax*sizeof(double))) == NULL) {
    ErrorMsg("Could not allocate storage in wt1()");
    return;
  }
  if ((wksp = (double *) XvgMM_Alloc(NULL, nmax*sizeof(double))) == NULL) {
    ErrorMsg("Could not allocate storage in wt1()");
    return;
  }
  
  /* transform along the X axis */
  if (width > 4) {
    for (j = 0; j < height; j++) {
      /* load input image line */
      for (i = j*width, k=1; k <= width; k++, i++)
	wksp[k] = inpixels[i];
      /* forward transform */
      if (isign >= 0) {
	for(n = width; n >= 4; n /= 2)
	  DAUB4(wksp, buf, n, isign);
      }
      /* inverse transform */
      else {
	for(n = 4; n <= width; n *= 2)
	  DAUB4(wksp, buf, n, isign);
      }
      /* copy transformed line */
      for (i = j*width, k=1; k <= width; k++, i++)
	outpixels[i] = wksp[k];
    }
  }
  else {
    for (i = 0; i < width*height; i++)
      outpixels[i] = inpixels[i];
  }

  /* transform along the Y axis */
  if (height > 4) {
    for (j = 0; j < width; j++) {
      /* load input image line */
      for (i = j, k=1; k <= height; k++, i+=width)
	wksp[k] = outpixels[i];
      /* forward transform */
      if (isign >= 0) {
	for(n = height; n >= 4; n /= 2)
	  DAUB4(wksp, buf, n, isign);
      }
      /* inverse transform */
      else {
	for(n = 4; n <= height; n *= 2)
	  DAUB4(wksp, buf, n, isign);
      }
      /* copy transformed line */
      for (i = j, k=1; k <= height; k++, i+=width)
	outpixels[i] = wksp[k];
    }
  }

  /* free storage */
  XvgMM_Free(buf);
  XvgMM_Free(wksp);
}

/***************************** Window ************************************/
void ApplyWindow(double *inpixels, double *outpixels,
		 int height, int width, void *ParmPtrs[])
{
  draw_mesg("Windowing...");
  FFTApplyWindow(inpixels, outpixels, inpixels);
}

/**************************** ShiftImage   *********************************/
void ShiftImageFFT(double *inpixels, double *outpixels,
		   int height, int width, void *ParmPtrs[])
{
  double m, p, dx, dy, min, max;
  int i, j, k, n, lFFTWindow, IFAIL;

  /* load parameters */
  dx = *((double *) ParmPtrs[0]);
  dy = *((double *) ParmPtrs[1]);

  /* get input image dynamic range */
  DynamicRange(inpixels, height*width, &max, &min);

  /* initialization */
  if (FFTInit(height, width) == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort : FFTInit() failed in ShiftImage()");
    return;
  }

  /* disable windowing temporarily */
  lFFTWindow = FFTWindow;
  FFTWindow = NOWINDOW;

  /* window the data and zero the imaginary part */
  draw_mesg("ShiftImage() - Applying window");
  FFTApplyWindow(inpixels, FFTRealBuffer, FFTImBuffer);
  
  /* compute FFT */
  draw_mesg("ShiftImage - Computing FFT...");
  IFAIL = 1;
  c06fuf(&width, &height, FFTRealBuffer, FFTImBuffer,
	 "Initial", FFTTrigm, FFTTrign, FFTWork, &IFAIL);
  if (IFAIL != 0) {
    Abort = B_TRUE;
    sprintf(cbuf,"Abort : IFAIL = %d on return from c06fuf() in ShiftImage() ",
	    IFAIL);
    draw_mesg(cbuf);
    FFTWindow = lFFTWindow;
    return;
  }
  
  /* FFT shift */
  draw_mesg("ShiftImage - Flipping quadrants...");
  FFTShift(FFTRealBuffer, FFTRealBuffer, height, width);
  FFTShift(FFTImBuffer, FFTImBuffer, height, width);

  /* calculate phase and magnitude, add delay, and back-convert */
  draw_mesg("ShiftImage - Adding phase shift...");
  for (j = 0, k = 0; j < height; j++) {
    for (i = 0; i < width; i++, k++) {
      /* calculate phase and magnitude */
      m = sqrt(FFTRealBuffer[k]*FFTRealBuffer[k]
	       + FFTImBuffer[k]*FFTImBuffer[k]);
      p = atan2(FFTImBuffer[k], FFTRealBuffer[k]);
      /* add phase delay */
      p += (TWOPI * ((((double) (i - width/2))*dx/((double) width))
		     + (((double) (j - height/2))*dy/((double) height))));
      /* recalculate real and imaginary part of FT */
      FFTRealBuffer[k] = m * cos(p);
      FFTImBuffer[k] = m * sin(p);
    }
  }

  /* un-flip quadrants */
  draw_mesg("ShiftImage - UnFlipping quadrants...");
  FFTShift(FFTRealBuffer, FFTRealBuffer, height, width);
  FFTShift(FFTImBuffer, FFTImBuffer, height, width);

  /* inverse FFT */
  draw_mesg("ShiftImage - Computing inverse FFT...");
  n = height*width;
  IFAIL = 1;
  c06gcf(FFTImBuffer, &n, &IFAIL);
  if (IFAIL != 0) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "Abort : IFAIL = %d on return from c06gcf() in ShiftImage() ",
	    IFAIL);
    draw_mesg(cbuf);
    FFTWindow = lFFTWindow;
    return;
  }
  IFAIL = 1;
  c06fuf(&width, &height, FFTRealBuffer, FFTImBuffer,
	 "Subsequent", FFTTrigm, FFTTrign, FFTWork, &IFAIL);
  if (IFAIL != 0) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "Abort : IFAIL = %d on return from c06fuf() in ShiftImage() ",
	    IFAIL);
    draw_mesg(cbuf);
    FFTWindow = lFFTWindow;
    return;
  }
  IFAIL = 1;
  c06gcf(FFTImBuffer, &n, &IFAIL);
  if (IFAIL != 0) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "Abort : IFAIL = %d on return from c06gcf() in ShiftImage() ",
	    IFAIL);
    draw_mesg(cbuf);
    FFTWindow = lFFTWindow;
    return;
  }

  /* inverse window the data and copy to the output buffer */
  draw_mesg("ShiftImage - Inverse windowing...");
  FFTApplyInverseWindow(FFTRealBuffer, outpixels);

  /* constrain dynamic range to that of the input image */
  draw_mesg("ShiftImage - Fixing dynamic range...");
  for (k = 0; k < height*width; k++) {
    if (outpixels[k] < min)
      outpixels[k] = min;
    if (outpixels[k] > max)
      outpixels[k] = max;
  }

  /* restore window type */
  FFTWindow = lFFTWindow;
}



/**************************** ScaleImage   *********************************/
void ScaleImageFFT(double *inpixels, double *outpixels,
		   int height, int width, void *ParmPtrs[])
{
  double sf, min, max, ScaleFactor;
  int n, xpad, ypad, i, j, k, x, y, lheight, lwidth, lFFTWindow, IFAIL;
  
  /* load parameters */
  sf = *((double *) ParmPtrs[0]);
  if (sf <= 0) {
    Abort = B_TRUE;
    draw_mesg("Abort : invalid input parameter in ScaleImageFFT()");
    return;
  }
  xpad = fabs(((double) width) * (sf - 1.0)) / 2.0;
  ypad = fabs(((double) height) * (sf - 1.0)) / 2.0;
  if (sf > 1.0) {
    lwidth = width + 2*xpad;
    lheight = height + 2*ypad;
  }
  else {
    lwidth = width - 2*xpad;
    lheight = height - 2*ypad;
  }
  if (VerboseFlag)
    printf("ScaleImageFFT() : lh:%d, lw:%d, xp:%d, yp:%d\n",
	   lheight, lwidth, xpad, ypad);
  
  /* get input image dynamic range */
  DynamicRange(inpixels, height*width, &max, &min);
  
  /* initialization */
  if (FFTInit(height, width) == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort : FFTInit() failed in ScaleImageFFT()");
    return;
  }

  /* disable windowing temporarily */
  lFFTWindow = FFTWindow;
  FFTWindow = NOWINDOW;

  /* window the data and zero the imaginary part */
  draw_mesg("ScaleImageFFT() - Applying window");
  FFTApplyWindow(inpixels, FFTRealBuffer, FFTImBuffer);
  
  /* compute FFT */
  draw_mesg("ScaleImageFFT - Computing FFT...");
  IFAIL = 1;
  c06fuf(&width, &height, FFTRealBuffer, FFTImBuffer,
	 "Initial", FFTTrigm, FFTTrign, FFTWork, &IFAIL);
  if (IFAIL != 0) {
    Abort = B_TRUE;
    sprintf(cbuf,"Abort : IFAIL=%d on return from c06fuf() in ScaleImageFFT()",
	    IFAIL);
    draw_mesg(cbuf);
    FFTWindow = lFFTWindow;
    return;
  }
  
  /* FFT shift */
  draw_mesg("ScaleImageFFT - Flipping quadrants...");
  FFTShift(FFTRealBuffer, FFTRealBuffer, height, width);
  FFTShift(FFTImBuffer, FFTImBuffer, height, width);

  /* init FFT2 buffers if required */ 
  if (FFTInit2(lheight, lwidth) == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort : FFTInit() failed in ScaleImageFFT()");
    FFTWindow = lFFTWindow;
    return;
  }

  /* zero padd or crop the current FFT data */
  draw_mesg("ScaleImageFFT - Cropping/scaling image...");
  ScaleFactor = sqrt(((double)(lheight*lwidth))/((double)(height*width)));
  if (sf > 1.0) {
    /* init the buffers */
    for (k = 0; k < lheight*lwidth; k++) {
      FFTRealBuffer2[k] = 0;
      FFTImBuffer2[k] = 0;
    }
    /* transfer the data */
    for (y = ypad, k = 0; y < (lheight - ypad); y++) {
      for (x = xpad; x < (lwidth - xpad); x++, k++) {
	FFTRealBuffer2[y*lwidth + x] = FFTRealBuffer[k]*ScaleFactor;
	FFTImBuffer2[y*lwidth + x] = FFTImBuffer[k]*ScaleFactor;
      }
    }
  }
  else {
    /* transfer the data */
    for (y = ypad, k = 0; y < (height - ypad); y++)
      for (x = xpad; x < (width - xpad); x++, k++) {
	FFTRealBuffer2[k] = FFTRealBuffer[y*width + x]*ScaleFactor;
	FFTImBuffer2[k] = FFTImBuffer[y*width + x]*ScaleFactor;
      }
  }

  /* un-flip quadrants */
  draw_mesg("ScaleImageFFT - UnFlipping quadrants...");
  FFTShift(FFTRealBuffer2, FFTRealBuffer2, lheight, lwidth);
  FFTShift(FFTImBuffer2, FFTImBuffer2, lheight, lwidth);

  /* inverse FFT */
  draw_mesg("ScaleImageFFT - Computing inverse FFT...");
  n = lheight*lwidth;
  IFAIL = 1;
  c06gcf(FFTImBuffer2, &n, &IFAIL);
  if (IFAIL != 0) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "Abort : IFAIL = %d on return from c06gcf() in ScaleImageFFT() ",
	    IFAIL);
    draw_mesg(cbuf);
    FFTWindow = lFFTWindow;
    return;
  }
  IFAIL = 1;
  c06fuf(&lwidth, &lheight, FFTRealBuffer2, FFTImBuffer2,
	 "Initial", FFTTrigm2, FFTTrign2, FFTWork2, &IFAIL);
  if (IFAIL != 0) {
    Abort = B_TRUE;
    sprintf(cbuf,
	    "Abort : IFAIL = %d on return from c06fuf(2) in ScaleImageFFT() ",
	    IFAIL);
    draw_mesg(cbuf);
    FFTWindow = lFFTWindow;
    return;
  }

  /* zero padd or crop the current FFT data */
  draw_mesg("ScaleImageFFT - Cropping/scaling image...");
  if (sf > 1.0) {
    /* transfer the data */
    for (y = ypad, k = 0; y < (lheight - ypad); y++) {
      for (x = xpad; x < (lwidth - xpad); x++, k++) {
	outpixels[k] = FFTRealBuffer2[y*lwidth + x];
      }
    }
  }
  else {
    /* init the buffer */
    for (i = 0; i < height*width; i++) {
      outpixels[i] = 0;
    }
    /* transfer the data */
    for (y = ypad, k = 0; y < (height - ypad); y++)
      for (x = xpad; x < (width - xpad); x++, k++) {
	outpixels[y*width + x] = FFTRealBuffer2[k];
      }
  }

  /* constrain dynamic range to that of the input image */
  draw_mesg("ScaleImageFFT - Fixing dynamic range...");
  for (k = 0; k < height*width; k++) {
    if (outpixels[k] < min)
      outpixels[k] = min;
    if (outpixels[k] > max)
      outpixels[k] = max;
  }

  /* restore window type */
  FFTWindow = lFFTWindow;
}



