/***********************************************************************
*
* Name: grab.c
*
* Author: Paul Charette
*
* Last Modified: 26 Nov 1994
*
* Purpose: Utility routines for grabbing. 
*
*               DisplayOverlay()
*               GalvoOpticalDelay()
*               Grab()
*               OverlayString()
*               VCAWriteData()
*               VCAWriteGLImage()
*
***********************************************************************/
#include <stdio.h>
#include "UxXt.h"
#include <fcntl.h>
#include <search.h>
#include <values.h>
#include "XvgGlobals.h"

/* DA values for selected phase offsets computed with a 4th order chebychev */
/* for two opposing 6mm thick windows                                       */
int PhaseStepDACommands_3[] = {
  -27814,  /*    0 */
  -25043,  /*  120 */
  -21884}; /*  240 */
int PhaseStepDACommands_4[] = {
  -27814,  /*    0 */
  -25767,  /*   90 */
  -23521,  /*  180 */
  -21017}; /*  270 */
int PhaseStepDACommands_5[] = {
  -27814,  /* -180 */
  -25767,  /*  -90 */ 
  -23521,  /*    0 */
  -21017,  /*   90 */
  -18189}; /*  180 */

/**********************    Overlay stuff      *****************************/
void OverlayString(char *s, int x, int y)
{
#ifdef VCAA_CODE
#ifdef NOTTHIS
  extern Widget ScratchDAW;
  struct vca_window window;
  XImage *image;
  int i, j, k, black;
  
  /* generate in-memory image of text string */
  XFillRectangle(UxDisplay, XtWindow(ScratchDAW), gc_BlackWhite,
		 0, 0, UxGetWidth(ScratchDAW), UxGetHeight(ScratchDAW));
  XDrawString(UxDisplay, XtWindow(ScratchDAW), gc_Overlay,
	      0, TEXT_OVERLAY_HEIGHT - 10, s, strlen(s));
  if ((image = XGetImage(UxDisplay, XtWindow(ScratchDAW), 0, 0,
			 TEXT_OVERLAY_WIDTH, TEXT_OVERLAY_HEIGHT,
			 AllPlanes, ZPixmap)) == NULL) 
    ErrorMsg("DisplayOverlay() : Error grabbing overlay image");
  else {
    if (image->data == NULL)
      ErrorMsg("DisplayOverlay() : Image data pointer is NULL");
    else {
      black = BlackPixel(UxDisplay, UxScreen);
      for (i = 0, k = 0; i < TEXT_OVERLAY_HEIGHT; i++)
	for (j = 0; j < TEXT_OVERLAY_WIDTH; j++, k++)
	  TextOverlayBuffer[k] = (image->data[i*image->bytes_per_line + j]
				  == black ? 0 : 0xffff);
      XFillRectangle(UxDisplay, XtWindow(ScratchDAW),
		     gc_BlackWhite, 0, 0, UxGetWidth(ScratchDAW),
		     UxGetHeight(ScratchDAW));
      XDestroyImage(image);
      
      /* draw the image into the overlay plane of the VCAA */
      vca_write_overlay(VCAAFd, x, y, TEXT_OVERLAY_WIDTH, TEXT_OVERLAY_HEIGHT,
			TextOverlayBuffer, 1);
    }
  }
#endif
#endif
}

void DisplayOverlay(char *s0, char *s1, char *s2, char *s3)
{
#ifdef VCAA_CODE
  /* Check that the frame grabber was initialized */
  if (VCAAConnectionOpen == B_FALSE)
    VCAAInit();

  /* set overlay mode */
  if (VCAAMode != VCA_MODE_OVERLAY) {
    vca_mode(VCAAFd, VCA_MODE_OVERLAY); 
    VCAAMode = VCA_MODE_OVERLAY;
  }

  /* write out the four strings */
  OverlayString(s0, 10, 465 - TEXT_OVERLAY_HEIGHT);
  OverlayString(s1, 620 - TEXT_OVERLAY_WIDTH, 465 - TEXT_OVERLAY_HEIGHT);
  OverlayString(s2, 10, 20);
  OverlayString(s3, 620 - TEXT_OVERLAY_WIDTH, 20);

  /* set VCAA mode to overlay and reset correct window specs */
  vca_window(VCAAFd, VCA_ON, FrameX, FrameY, FrameWidth, FrameHeight);
#endif
} 
/********************** GALVO OPTICAL DELAY STUFF *************************/
/* rotate phase stepping galvo by accelerating until mid-range value      */
/* and then decelerating back down again.                                 */
void GalvoOpticalDelay(int NewVal)
{ 
  int OldVal, i, mid_range_incr, increment;
  double mid_range_val;

  /* find largest number in arithmetic series to reach mid-range value */
  OldVal = DA0CurrentVal;
  mid_range_val = abs(NewVal - OldVal)/2;
  mid_range_incr = (-1.0 + sqrt(1.0 + 8.0*mid_range_val)) / 2.0;

  if (OldVal < NewVal) {
    /* increasing galvo angular velocity */
    for (i = OldVal, increment = 1; increment < mid_range_incr;
	 i += increment++)
      UnimaPSGalvosOut(i);
    /* decreasing galvo angular velocity */
    for (;(i < NewVal) && (increment > 0); i += increment--)
      UnimaPSGalvosOut(i);
    /* slow final zeroing in on target */
    for (;i <= NewVal; i++)
      UnimaPSGalvosOut(i);
  }
  else {
    /* increasing galvo angular velocity */
    for (i = OldVal, increment = 1; increment <= mid_range_incr;
	 i -= increment++)
      UnimaPSGalvosOut(i);
    /* decreasing galvo angular velocity */
    for (;(i > NewVal) && (increment > 0); i -= increment--)
      UnimaPSGalvosOut(i);
    /* slow final zeroing in on target */
    for (;i >= NewVal; i--)
      UnimaPSGalvosOut(i);
  }
}

#ifdef VCAA_CODE
/*************************** VCAWriteGLImage ******************************/
/*                                                                        */
/*  function: write GL image to the VCA                                   */
/*                                                                        */
/**************************************************************************/
void VCAWriteGLImage(int *data, int xextent, int yextent)
{
  struct vca_window window;
  struct vca_controls controls;
  int vca, width, height, xs0, xd0, ys0, yd0, xs, xd, ys, yd, pix;
  unsigned short pixels[480][640];
  
  /* Check that the frame grabber was initialized */
  if (VCAAConnectionOpen == B_FALSE)
    VCAAInit();
  
  /* set memory mode */
  if (VCAAMode != VCA_MODE_MEMORY) {
    vca_mode(VCAAFd, VCA_MODE_MEMORY); 
    VCAAMode = VCA_MODE_MEMORY;
  }
  
  /* set frame grabber window to full screen if required */
  if ((FrameWidth != 640) || (FrameHeight != 480)) {
    window.window_enable = VCA_ON;
    window.window_x = 0;
    window.window_y = 0;
    window.window_width = 640;
    window.window_height = 480;
    vca_set_window(VCAAFd, &window);
  }

  /* check for 565 bit configuration */
  if (VCAAFormat != VCA_FORMAT_565) {
    vca_format(VCAAFd, VCA_FORMAT_565);
    VCAAFormat = VCA_FORMAT_565;
  }

  /* clear the buffer */
  for (yd = 0; yd < 480; yd++)
    for (xd = 0; xd < 640; xd++)
      pixels[yd][xd] = 0;

  /* conpute the source and destination buffer parms */
  if (xextent >= 639) {
    xd0 = 0;
    width = 640;
    xs0 = (xextent - 640)/2;
  }
  else {
    xd0 = (640 - xextent)/2;
    width = xextent + xd0;
    xs0 = 0;
  }
  if (yextent >= 479) {
    yd0 = 0;
    height = 480;
    ys0 = (yextent - 480)/2;
  }
  else {
    yd0 = (480 - yextent)/2;
    height = yextent + yd0;
    ys0 = 0;
  }

  /* convert from 16 bits RGB to 16 bits 565 */
  for (ys = ys0, yd = yd0; yd < height; yd++, ys++)
    for (xs = xs0, xd = xd0; xd < width; xd++, xs++) {
      pix = data[ys*xextent + xs];
      pix = ((pix & 0x000000f8) << 8) | ((pix & 0x0000fc00) >> 5)
	| ((pix & 0x00f80000) >> 19);
      pixels[479-yd][xd] = ((pix & 0xff00) >> 8) | ((pix & 0x00ff) << 8);
    }
  
  /* write the data to the VCA */
  vca_write_image(VCAAFd, 0, 0, 640, 480, pixels);
  
  /* reset frame grabber window to partial screen if required */
  if ((FrameWidth != 640) || (FrameHeight != 480)) {
    window.window_enable = VCA_ON;
    window.window_x = FrameX;
    window.window_y = FrameY;
    window.window_width = FrameWidth;
    window.window_height = FrameHeight;
    vca_set_window(VCAAFd, &window);
  }
}


/*************************** VCAWriteData *********************************/
/*                                                                        */
/*  function: write image to the VCA                                      */
/*                                                                        */
/**************************************************************************/
boolean_t VCAWriteData(XImage *image, XColor *Cells)
{
  struct vca_window window;
  struct vca_controls controls;
  XColor colors[256];
  int width, height, i, xs0, xd0, ys0, yd0, xs, xd, ys, yd, bytes_per_line;
  char *inpixels;
  unsigned short pixels[480][640], pix;
  
  /* Check that the frame grabber was initialized */
  if (VCAAConnectionOpen == B_FALSE)
    VCAAInit();

  /* set memory mode */
  if (VCAAMode != VCA_MODE_MEMORY) {
    vca_mode(VCAAFd, VCA_MODE_MEMORY); 
    VCAAMode = VCA_MODE_MEMORY;
  }

  /* set frame grabber window to full screen if required */
  if ((FrameWidth != 640) || (FrameHeight != 480))
    vca_window(VCAAFd, VCA_ON, 0, 0, 640, 480);

  /* set image parms */
  bytes_per_line = image->bytes_per_line;
  inpixels = image->data;
  
  /* clear the output buffer */
  for (yd = 0; yd < 480; yd++)
    for (xd = 0; xd < 640; xd++)
      pixels[yd][xd] = 0;
  
  /* conpute the source and destination buffer parms */
  if (image->width >= 639) {
    xd0 = 0;
    width = 640;
    xs0 = (image->width - 640)/2;
  }
  else {
    xd0 = (640 - image->width)/2;
    width = image->width + xd0;
    xs0 = 0;
  }
  if (image->height >= 479) {
    yd0 = 0;
    height = 480;
    ys0 = (image->height - 480)/2;
  }
  else {
    yd0 = (480 - image->height)/2;
    height = image->height + yd0;
    ys0 = 0;
  }
  
  /* build color conversion table from 8 bit indicies to 24bit RGB */
  for (i = 0; i < 256; i++)
    colors[Cells[i].pixel] = Cells[i];
  
  /* convert from 24 bits RGB to 16 bits 565 */
  ioctl(VCAAFd,VCA_IOC_GET_CONTROLS, &controls);
  if (controls.format == VCA_FORMAT_565) {
    for (ys = ys0, yd = yd0; yd < height; yd++, ys++)
      for (xs = xs0, xd = xd0; xd < width; xd++, xs++) {
	pix = (colors[inpixels[ys*bytes_per_line + xs]].red & 0xf800)
	  | ((colors[inpixels[ys*bytes_per_line + xs]].green & 0xfc00) >> 5)
	    | ((colors[inpixels[ys*bytes_per_line + xs]].blue & 0xf800) >> 11);
	pixels[yd][xd] = ((pix & 0xff00) >> 8) | ((pix & 0x00ff) << 8);
      }
  }
  else {
    for (ys = ys0, yd = yd0; yd < height; yd++, ys++)
      for (xs = xs0, xd = xd0; xd < width; xd++, xs++) {
	pix = (colors[inpixels[ys*bytes_per_line + xs]].red & 0xfc00)
	  | ((colors[inpixels[ys*bytes_per_line + xs]].green & 0xfc00) >> 6)
	    | ((colors[inpixels[ys*bytes_per_line + xs]].blue & 0xf000) >> 12);
	pixels[yd][xd] = ((pix & 0xff00) >> 8) | ((pix & 0x00ff) << 8);
      }
  }
  
  /* write the data to the VCA */
  vca_write_image(VCAAFd, 0, 0, 640, 480, pixels);

  /* reset frame grabber window to partial screen if required */
  if ((FrameWidth != 640) || (FrameHeight != 480))
    vca_window(VCAAFd, VCA_ON, FrameX, FrameY, FrameWidth, FrameHeight);
}
#else
#if defined (SGI) && defined (SVIDEO)
boolean_t VCAWriteData(XImage *image, XColor *Cells)
{
  XColor colors[256];
  int width, height, i, xs0, xd0, ys0, yd0, xs, xd, ys, yd, bytes_per_line;
  char *inpixels;

  /* initialize the SGI frame garbber memory if required */
  if (SGIFBInitialized == B_FALSE)
    if (SGIGetVideoFBParms() == B_FALSE) {
      ErrorMsg("VCAWriteData() : Failed initializing the SGI frame grabber");
      return(B_FALSE);
    }

  /* set image parms */
  bytes_per_line = image->bytes_per_line;
  inpixels = image->data;
  
  /* clear the output buffer */
  for (i = 0; i < SGIFBHeight*SGIFBWidth; i++)
    SGIFrameBuffer[i] = 0x00;

  /* conpute the source and destination buffer parms */
  if (image->width >= (SGIFBWidth-1)) {
    xd0 = 0;
    width = SGIFBWidth;
    xs0 = (image->width - SGIFBWidth)/2;
  }
  else {
    xd0 = (SGIFBWidth - image->width)/2;
    width = image->width + xd0;
    xs0 = 0;
  }
  if (image->height >= (SGIFBHeight-1)) {
    yd0 = 0;
    height = SGIFBHeight;
    ys0 = (image->height - SGIFBHeight)/2;
  }
  else {
    yd0 = (SGIFBHeight - image->height)/2;
    height = image->height + yd0;
    ys0 = 0;
  }
  
  /* build color conversion table from 8 bit indicies to 24bit RGB */
  for (i = 0; i < 256; i++)
    colors[Cells[i].pixel] = Cells[i];
  
  /* convert to 24 bit RGB */
  for (ys = ys0, yd = yd0; yd < height; yd++, ys++)
    for (xs = xs0, xd = xd0; xd < width; xd++, xs++)
      SGIFrameBuffer[(SGIFBHeight - yd - 1)*SGIFBWidth + xd] =
	((colors[inpixels[ys*bytes_per_line + xs]].red & 0xff00) >> 8)
	  | (colors[inpixels[ys*bytes_per_line + xs]].green & 0xff00)
	    | ((colors[inpixels[ys*bytes_per_line + xs]].blue & 0xff00) << 8);
  
  /* write the data to the SGI frame buffer */
  if (SGILoadFrame() == B_FALSE) {
    ErrorMsg("VCAWriteData(): Failed writing to the SGI frame grabber memory");
    return(B_FALSE);
  }
#ifdef DEBUG
  printf("VCAWriteData() : Frame buffer loaded (%d,%d)\n", height, width);
#endif
}
void VCAWriteGLImage(int *data, int xextent, int yextent){}
#else
void VCAWriteGLImage(int *data, int xextent, int yextent)
{
  ErrorMsg("VCAWriteGLImage() : Video output not enabled");
}
boolean_t VCAWriteData(XImage *image, XColor *Cells)
{
  ErrorMsg("VCAWriteData() : Video output not enabled");
  return(B_FALSE);
}
#endif
#endif


/********************************* Grab ************************************/
/*                                                                         */
/*  function: grab a frame to compose an image                             */
/*                                                                         */
/***************************************************************************/
#define NFRMAVGS 4
void Grab(double *pixels, unsigned short *sbufs,
	  boolean_t NewFrame, boolean_t AtanConvert)
{
  static double OneThird = 1.0/3.0, LastGrabType;
  int i, j, pix, RedMask, img0_offset, img1_offset, img2_offset, img3_offset,
  offset;
#ifdef VCAA_CODE
  struct vca_controls controls;
#endif
  double seconds, x, y;
  ATAN_LUT_ELEMENT ae;

  /* set correct mode */
#ifdef VCAA_CODE
  /* Check that the frame grabber was initialized */
  if (VCAAConnectionOpen == B_FALSE)
    VCAAInit();

  if ((VCAAMode != VCA_MODE_CAPTURE) && (NewFrame)) {
    vca_mode(VCAAFd, VCA_MODE_MEMORY);  /* required for some sort of reset */
    vca_mode(VCAAFd, VCA_MODE_CAPTURE); 
    VCAAMode = VCA_MODE_CAPTURE;
    for (seconds = second(); second() < (seconds + 0.1);); 
    ioctl(VCAAFd, VCA_IOC_CAPTURE);
  }
#endif

  switch (GrabType) {
  case THREESTEPGRAB:
#ifdef VCAA_CODE
    if (NewFrame) {
      /* check that VCAA word format is ok */
      if (VCAAFormat != VCA_FORMAT_664) {
	vca_format(VCAAFd, VCA_FORMAT_664);
	VCAAFormat = VCA_FORMAT_664;
      }
      /* check for correct initial setting */
      if (LastGrabType != THREESTEPGRAB) {
	LastGrabType = THREESTEPGRAB;
	GalvoOpticalDelay(PhaseStepDACommands_3[0]);
      }
      /* grab frames */
      for (i = 0; i < 3; i++) {
	/* capture */
	sync();
	ioctl(VCAAFd, VCA_IOC_CAPTURE);
	/* advance galvo to next position */
	if (i == 2)
	  GalvoOpticalDelay(PhaseStepDACommands_3[0]);
	else
	  GalvoOpticalDelay(PhaseStepDACommands_3[i+1]);
	/* read in frame */
	lseek(VCAAFd, (long) 0, 0);
	read(VCAAFd, &(sbufs[i*FrameHeight*FrameWidth]),
	     FrameHeight*FrameWidth * sizeof(unsigned short));
      }
    }
#endif
    /* compute arctan function */
    for (i = 0, RedMask = 0x00FC; (i < FrameHeight*FrameWidth) && AtanConvert;
	 i++) {
      x = 1.732050808*((double) ((sbufs[1*FrameHeight*FrameWidth + i]
				  & RedMask)
				 - (sbufs[2*FrameHeight*FrameWidth + i]
				    & RedMask)));
      y = ((double) (2*(sbufs[0*FrameHeight*FrameWidth + i] & RedMask)
		     - (sbufs[1*FrameHeight*FrameWidth + i] & RedMask)
		     - (sbufs[2*FrameHeight*FrameWidth + i] & RedMask)));
      pixels[i] = atan2(x, y);
      RadiusImgData[i] = sqrt(x*x + y*y);
      ErrorImgData[i] = 0;
    }
    ComplexDataSet = B_TRUE;
    break;
  case FOURSTEPGRAB:
#ifdef VCAA_CODE
    if (NewFrame) {
      /* check that VCAA word format is ok */
      if (VCAAFormat != VCA_FORMAT_664) {
	vca_format(VCAAFd, VCA_FORMAT_664);
	VCAAFormat = VCA_FORMAT_664;
      }
      /* check for correct initial setting */
      if (LastGrabType != FOURSTEPGRAB) {
	LastGrabType = FOURSTEPGRAB;
	GalvoOpticalDelay(PhaseStepDACommands_4[0]);
      }
      /* grab frames */
      for (i = 0; i < 4; i++) {
	/* capture */
	ioctl(VCAAFd, VCA_IOC_CAPTURE);
	/* advance galvo to next position */
	if (i == 3)
	  GalvoOpticalDelay(PhaseStepDACommands_4[0]);
	else
	  GalvoOpticalDelay(PhaseStepDACommands_4[i+1]);
	/* read in frame */
	lseek(VCAAFd, (long) 0, 0);
	read(VCAAFd, &(sbufs[i*FrameHeight*FrameWidth]),
	     FrameHeight*FrameWidth * sizeof(unsigned short));
      }
      sync();
    }
#endif
    /* pass through LUT to compute arctan function */
    img0_offset = 0*FrameHeight*FrameWidth;
    img1_offset = 1*FrameHeight*FrameWidth;
    img2_offset = 2*FrameHeight*FrameWidth;
    img3_offset = 3*FrameHeight*FrameWidth;
    for (i = 0, RedMask = 0x00FC; (i < FrameHeight*FrameWidth) && AtanConvert;
	 i++) {
      /* calculate 2Isin, 2Icos images */
      SinImgData[i] = (sbufs[img3_offset + i] & RedMask)
	- (sbufs[img1_offset + i] & RedMask);
      CosImgData[i] = (sbufs[img0_offset + i] & RedMask)
	- (sbufs[img2_offset + i] & RedMask);

      /* calculate radius image but check for saturation */
      if (((sbufs[img0_offset + i] & RedMask) == RedMask)
	  || ((sbufs[img1_offset + i] & RedMask) == RedMask)
	  || ((sbufs[img2_offset + i] & RedMask) == RedMask)
	  || ((sbufs[img3_offset + i] & RedMask) == RedMask))
	RadiusImgData[i] = 0;
      else
	RadiusImgData[i] = sqrt(SinImgData[i]*SinImgData[i]
				+ CosImgData[i]*CosImgData[i]) * 0.5;
      
      /* fetch other quantities from the LUT */
      ae = atan_lut[((int) SinImgData[i]) + VCAA_RED_DR]
		    [((int) CosImgData[i]) + VCAA_RED_DR];
      pixels[i] = ae.angle;
      ErrorImgData[i] = ae.error;
    }
    ComplexDataSet = B_TRUE;
    if (VerboseFlag)
      printf("Grab(): decoded image\n");
    break;
  case FOUR_ASTEPGRAB:
#ifdef VCAA_CODE
    if (NewFrame) {
      /* check that VCAA word format is ok */
      if (VCAAFormat != VCA_FORMAT_664) {
	vca_format(VCAAFd, VCA_FORMAT_664);
	VCAAFormat = VCA_FORMAT_664;
      }
      /* check for correct initial setting */
      if (LastGrabType != FOUR_ASTEPGRAB) {
	LastGrabType = FOUR_ASTEPGRAB;
	GalvoOpticalDelay(PhaseStepDACommands_4[0]);
      }
      /* grab frames */
      for (i = 0; i < 4; i++) {
	/* capture */
	sync();
	ioctl(VCAAFd, VCA_IOC_CAPTURE);
	/* advance galvo to next position */
	if (i == 3)
	  GalvoOpticalDelay(PhaseStepDACommands_4[0]);
	else
	  GalvoOpticalDelay(PhaseStepDACommands_4[i+1]);
	/* read in frame */
	lseek(VCAAFd, (long) 0, 0);
	read(VCAAFd, &(sbufs[i*FrameHeight*FrameWidth]),
	     FrameHeight*FrameWidth * sizeof(unsigned short));
      }
    }
#endif
    /* pass through LUT to compute arctan function */
    for (i = 0, RedMask = 0x00FC; (i < FrameHeight*FrameWidth) && AtanConvert;
	 i++) {
      ae = atan_lut[(int)
		    (((double)(sbufs[1*FrameHeight*FrameWidth + i]
			       & RedMask))
		     - OneThird*((double)((sbufs[0*FrameHeight*FrameWidth + i]
					   & RedMask)
					  + (sbufs[2*FrameHeight*FrameWidth +i]
					     & RedMask)
					  + (sbufs[3*FrameHeight*FrameWidth +i]
					     & RedMask))))
		    + VCAA_RED_DR]
		      [(int)
		       (OneThird*((double)((sbufs[0*FrameHeight*FrameWidth + i]
					    & RedMask)
					   + (sbufs[1*FrameHeight*FrameWidth+i]
					      & RedMask)
					   + (sbufs[3*FrameHeight*FrameWidth+i]
					      & RedMask)))
			- ((double)(sbufs[2*FrameHeight*FrameWidth + i]
				    & RedMask)))
		       + VCAA_RED_DR];
      pixels[i] = ae.angle;
      RadiusImgData[i] = ae.radius;
      ErrorImgData[i] = ae.error;
    }
    ComplexDataSet = B_TRUE;
    break;
  case FIVESTEPGRAB:
#ifdef VCAA_CODE
    if (NewFrame) {
      /* check that VCAA word format is ok */
      if (VCAAFormat != VCA_FORMAT_664) {
	vca_format(VCAAFd, VCA_FORMAT_664);
	VCAAFormat = VCA_FORMAT_664;
      }
      /* check for correct initial setting */
      if (LastGrabType != FIVESTEPGRAB) {
	LastGrabType = FIVESTEPGRAB;
	GalvoOpticalDelay(PhaseStepDACommands_5[0]);
      }
      /* grab frames */
      for (i = 0; i < 5; i++) {
	/* capture */
	sync();
	ioctl(VCAAFd, VCA_IOC_CAPTURE);
	/* advance galvo to next position */
	if (i == 4)
	  GalvoOpticalDelay(PhaseStepDACommands_5[0]);
	else
	  GalvoOpticalDelay(PhaseStepDACommands_5[i+1]);
	/* read in frame */
	lseek(VCAAFd, (long) 0, 0);
	read(VCAAFd, &(sbufs[i*FrameHeight*FrameWidth]),
	     FrameHeight*FrameWidth * sizeof(unsigned short));
      }
    }
#endif
    /* pass through LUT to compute arctan function */
    for (i = 0, RedMask = 0x00FC; (i < FrameHeight*FrameWidth) && AtanConvert;
	 i++) {
      ae = atan_lut[(sbufs[1*FrameHeight*FrameWidth + i] & RedMask)
		    - (sbufs[3*FrameHeight*FrameWidth + i] & RedMask)
		    + VCAA_RED_DR]
		      [(int)(((double)(2*(sbufs[2*FrameHeight*FrameWidth + i]
					  & RedMask)
				       - (sbufs[4*FrameHeight*FrameWidth + i]
					  & RedMask)
				       - (sbufs[0*FrameHeight*FrameWidth + i]
					  & RedMask)))*0.5)
		       + VCAA_RED_DR];
      pixels[i] = ae.angle;
      RadiusImgData[i] = ae.radius;
      ErrorImgData[i] = ae.error;
    }
    ComplexDataSet = B_TRUE;
    break;
  case REDGRAB:
#ifdef VCAA_CODE
    /* capture the frame & read the data */
    if (VCAAFormat == VCA_FORMAT_565)
      RedMask = 0xf8;
    else
      RedMask = 0xfc;
    ioctl(VCAAFd, VCA_IOC_CAPTURE);
    lseek(VCAAFd, (long) 0, 0);
    read(VCAAFd, sbufs, FrameHeight*FrameWidth * sizeof(unsigned short));
    for (i = 0; i < FrameHeight*FrameWidth; i++)
      pixels[i] = sbufs[i] & RedMask;
    ComplexDataSet = B_FALSE;
    LastGrabType = REDGRAB;
#endif
    break;
  case GREENGRAB:
#ifdef VCAA_CODE
    /* capture the frame & read the data */
    ioctl(VCAAFd,VCA_IOC_GET_CONTROLS, &controls);
    ioctl(VCAAFd, VCA_IOC_CAPTURE);
    lseek(VCAAFd, (long) 0, 0);
    read(VCAAFd, sbufs, FrameHeight*FrameWidth * sizeof(unsigned short));
    if (VCAAFormat == VCA_FORMAT_664)
      for (i = 0; i < FrameHeight*FrameWidth; i++)
	pixels[i] = ((sbufs[i] & 0xf000) >> 10) | ((sbufs[i] & 0x0003) << 6);
    else
      for (i = 0; i < FrameHeight*FrameWidth; i++)
	pixels[i] = ((sbufs[i] & 0xe000) >> 11) | ((sbufs[i] & 0x0007) << 5);
    ComplexDataSet = B_FALSE;
    LastGrabType = GREENGRAB;
#endif
    break;
  default:
  case GRAYLEVELGRAB:
#ifdef VCAA_CODE
    if (NewFrame) {
      if (VerboseFlag) {
	printf("Grab() : Grabbing 4 frames...");
	fflush(stdout);
      }
      /* capture 4 frames & read the data */
      for (i = 0; i < NFRMAVGS; i++) {
	sync();
	/* capture */
	ioctl(VCAAFd, VCA_IOC_CAPTURE);
	/* read in frame */
	lseek(VCAAFd, (long) 0, 0);
	read(VCAAFd, &(sbufs[i*FrameHeight*FrameWidth]),
	     FrameHeight*FrameWidth * sizeof(unsigned short));
      }
      if (VerboseFlag)
	printf("Done.\n");

      /* check for the format and convert to 8 bits */
      if (VCAAFormat == VCA_FORMAT_565)
	for (i = 0; i < FrameHeight*FrameWidth; i++) {
	  pixels[i] = 0;
	  for (j = 0; j < NFRMAVGS; j++) {
	    offset = j*FrameHeight*FrameWidth;
	    pix = ((sbufs[offset + i] & 0xff00) >> 8)
	      | ((sbufs[offset + i] & 0x00ff) << 8);
	    pixels[i] += (((pix & 0xf800) >> 8) + ((pix & 0x07e0) >> 3)
			  + ((pix & 0x001f) << 3));
	  }
	  pixels[i] *= 0.0834;
	}
      else
	for (i = 0; i < FrameHeight*FrameWidth; i++) {
	  pixels[i] = 0;
	  for (j = 0; j < NFRMAVGS; j++) {
	    offset = j*FrameHeight*FrameWidth;
	    pix = ((sbufs[offset + i] & 0xff00) >> 8)
	      | ((sbufs[offset + i] & 0x00ff) << 8);
	    pixels[i] += (((pix & 0xfc00) >> 8) + ((pix & 0x03f0) >> 2)
			  + ((pix & 0x000f) << 4));
	  }
	  pixels[i] *= 0.0834;
	}
    }
    if (VerboseFlag)
      printf("Grab() : Data averaged and stored.\n");
    ComplexDataSet = B_FALSE;
    LastGrabType = GRAYLEVELGRAB;
#endif
    break;
  }
}



