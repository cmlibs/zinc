/***********************************************************************
*
*  Name:          utils.c
*
*  Author:        Paul Charette
*
*  Last Modified: 3 March 1994
*
*  Purpose:       Utility routines.
*
*                      ClearPPort()
*                      CreateAtanLut()
*                      CreateSpeckleImage()
*                      FlushSPort()
*                      RandomDotStereogram()
*                      RecordFrames()
*                      SGICloseVideo()
*                      SGIGetVideoFBParms()
*                      SGILiveVideoOn()
*                      SGILoadFrame()
*                      sleep_us()
*                      VCRFrameBackward()
*                      VCRFrameForeward()
*                      VCRInitVideo()
*                      VCRReset()
*                      VCRRecord()
*                      XvgImgStack_Alloc()
*                      XvgImgStack_Clear()
*                      XvgImgStack_Init()
*                      XvgImgStack_Pop()
*                      XvgImgStack_Push()
*                      XvgImgStack_Swap()
*
***********************************************************************/
#include <stdio.h>
#include "UxXt.h"
#include <fcntl.h>
#include <memory.h>
#include <search.h>
#include <values.h>
#include <termio.h>
#include <malloc.h>
#ifdef PPORT_CODE
#include <sys/ioacc.h>
#endif
#include "XvgGlobals.h"
#if defined (SGI) && defined (SVIDEO)
#include <svideo.h>
#endif

/***************************************************************************/
/*                                                                         */
/***************************************************************************/
void CreateSpeckleImage(double *inpixels, double *outpixels,
			int height, int width,
			void *ParmPtrs[])
{
  double period_inv, noise_max, noise, noise_norm, v;
  int xperiod, yperiod, i, j, k;
  
  /* load parameters */
  xperiod = *((int *) ParmPtrs[0]);
  yperiod = *((int *) ParmPtrs[1]);
  noise_max = *((double *) ParmPtrs[2]);
  
  /* create the image */
  period_inv = 1.0 / sqrt((double) (xperiod*xperiod + yperiod*yperiod));
  period_inv *= period_inv;
  noise_norm = 2.0 / ((double) MAXINT);
  for (j = 0, k = 0; j < height; j++)
    for (i = 0; i < width; i++, k++) {
      if (noise_max != 0.0)
	noise = ((((double) random()) * noise_norm) - 1.0) * noise_max;
      else
	noise = 0;
      v = (((double) (i*xperiod + j*yperiod))*period_inv*TWOPI) + noise;
      outpixels[k] = v - rint(v/TWOPI)*TWOPI;
    }
}


/***************************************************************************/
/*                                                                         */
/***************************************************************************/
void CreateNoiseImage(double *inpixels, double *outpixels,
			int height, int width,
			void *ParmPtrs[])
{
  double noise_norm;
  int i, j, k;
  
  /* create the image */
  noise_norm = 1.0 / ((double) MAXINT);
  for (j = 0, k = 0; j < height; j++)
    for (i = 0; i < width; i++, k++)
      outpixels[k] = ((double) random()) * noise_norm;
}


/*
IMAGE STACK OPS

NOTE: When images are popped, the storage is not released, so that the
      next Push Operation will be quick if the image size is the same.
      The XvgImgStack_Clear() function will release the storage.
*/
boolean_t XvgImgStack_Alloc(int height, int width)
{
  /* check that stack is not full */
  if (ImageStackTop >= IMAGESTACKSIZE) {
    return(B_FALSE);
  }

  /* allocate storage for new image */
  ImageStack[ImageStackTop].height = height;
  ImageStack[ImageStackTop].width = width;
  if ((ImageStack[ImageStackTop].pixels =
       (double *) XvgMM_Alloc((void *) ImageStack[ImageStackTop].pixels,
			      height*width*sizeof(double))) == NULL) {
    return(B_FALSE);
  }

  /* increment stack pointer */
  ImageStackTop++;
  UpdateStackLabel(ImageStackTop);

  /* return success */
  return(B_TRUE);
}

void XvgImgStack_Init(void)
{
  int i;

  for (i = 0; i < IMAGESTACKSIZE; i++)
    ImageStack[i].pixels = NULL;
}

boolean_t XvgImgStack_Pop(void)
{
  int i;

  /* check that stack is not empty */
  if (ImageStackTop <= 0) {
    return(B_FALSE);
  }
  
  /* transfer data and put up image */
  ImageStackTop--;
  UpdateStackLabel(ImageStackTop);
  ImgHeight = ImageStack[ImageStackTop].height;
  ImgWidth = ImageStack[ImageStackTop].width;
  InitProcBuffers();
  for (i = 0; i < ImgHeight*ImgWidth; i++)
    ProcessedPixels[i] = ImageStack[ImageStackTop].pixels[i];
  DynamicRange(ProcessedPixels, ImgHeight*ImgWidth, &ProcMax, &ProcMin);
  CreateProcImage();
  
  /* free up the storage */
/*
  XvgMM_Free(ImageStack[ImageStackTop].pixels);
  ImageStack[ImageStackTop].pixels = NULL;
*/

  /* return success */
  return(B_TRUE);
}

boolean_t XvgImgStack_Push(double *pixels, int height, int width)
{
  int i;

  /* check that stack is not full */
  if (ImageStackTop >= IMAGESTACKSIZE) {
    return(B_FALSE);
  }

  /* allocate storage for new image */
  ImageStack[ImageStackTop].height = height;
  ImageStack[ImageStackTop].width = width;
  if ((ImageStack[ImageStackTop].pixels =
       (double *) XvgMM_Alloc((void *) ImageStack[ImageStackTop].pixels,
			      height*width*sizeof(double))) == NULL) {
    return(B_FALSE);
  }

  /* transfer data */
  for (i = 0; i < height*width; i++)
    ImageStack[ImageStackTop].pixels[i] = pixels[i];

  /* increment stack pointer */
  ImageStackTop++;
  UpdateStackLabel(ImageStackTop);

  /* return success */
  return(B_TRUE);
}

boolean_t XvgImgStack_Swap(double *pixels)
{
  int i;
  double tmp;

  /* check that stack is not empty */
  if (ImageStackTop <= 0) {
    return(B_FALSE);;
  }

  /* check that image sizes match */
  if ((ImgHeight != ImageStack[ImageStackTop-1].height)
      || (ImgWidth != ImageStack[ImageStackTop-1].width)) {
    return(B_FALSE);
  }

  /* swap data */
  for (i = 0; i < ImgHeight*ImgWidth; i++) {
    tmp = pixels[i];
    pixels[i] = ImageStack[ImageStackTop-1].pixels[i];
    ImageStack[ImageStackTop-1].pixels[i] = tmp;
  }

  /* return success */
  return(B_TRUE);
}

void XvgImgStack_Clear(void)
{
  int i;

  for (i = IMAGESTACKSIZE-1; i >= 0; i--) {
    XvgMM_Free(ImageStack[i].pixels);
    ImageStack[i].pixels = NULL;
  }
  ImageStackTop = 0;
  UpdateStackLabel(ImageStackTop);
}


/************************** create_atan_lut ********************************/
/*                                                                         */
/*  function: create VCAA_RED_DRx2+1 x VCAA_RED_DRx2+1 lut for converting  */
/*            phase stepping data to phase angle in radians by unwrapping  */
/*            the arctan function.                                         */
/*            The lookup table is addressed as:                            */
/*                      atan_lut[numerator + VCAA_RED_DR,                  */
/*                               denominator + VCAA_RED_DR]                */
/*                                                                         */
/*            where : numerator   = I3 - I1                                */
/*                    denominator = I0 - I2                                */
/***************************************************************************/
void CreateAtanLut(double radius_threshold)
{
  register int i, j;
  register double pi, RadiusX2, alpha, beta, sin_angle, cos_angle;

  for (i = -VCAA_RED_DR; i <= VCAA_RED_DR; i++)
    for (j = -VCAA_RED_DR; j <= VCAA_RED_DR; j++) {
      beta = (double) i;
      alpha = (double) j;
      RadiusX2 = sqrt(alpha*alpha + beta*beta);
      atan_lut[i+VCAA_RED_DR][j+VCAA_RED_DR].radius = RadiusX2 * 0.5;
      if (atan_lut[i+VCAA_RED_DR][j+VCAA_RED_DR].radius > radius_threshold) {
	if ((i >= 0) && (j >= 0)) {                 /* upper right quadrant */
	  sin_angle = asin(beta / RadiusX2);
	  cos_angle = acos(alpha / RadiusX2);
	}
	else if ((i >= 0) && (j < 0)) {              /* upper left quadrant */
	  sin_angle = PI - asin(beta / RadiusX2);
	  cos_angle = acos(alpha / RadiusX2);
	}
	else if ((i < 0) && (j < 0)) {               /* lower left quadrant */
	  sin_angle = -(PI + asin(beta / RadiusX2));
	  cos_angle = - acos(alpha / RadiusX2);
	}
	else {                                      /* lower right quadrant */
	  sin_angle = asin(beta / RadiusX2);
	  cos_angle = -acos(alpha / RadiusX2);
	}
	/* weighted average of both asin and acos angles */
	atan_lut[i+VCAA_RED_DR][j+VCAA_RED_DR].angle =
	  (fabs(alpha)*sin_angle + fabs(beta)*cos_angle)
	    /(fabs(alpha) + fabs(beta));
	/* delta theta in weighted sum */
	atan_lut[i+VCAA_RED_DR][j+VCAA_RED_DR].error =
	  fabs(sin_angle - cos_angle);
	}
      else {
	atan_lut[i+VCAA_RED_DR][j+VCAA_RED_DR].angle = 0;
	atan_lut[i+VCAA_RED_DR][j+VCAA_RED_DR].error = 0;
      }
    }
  AtanLutCreated = B_TRUE;
}

/*
VCAA frame grabber stuff
*/
void VCAAInit(void)
{
#ifdef VCAA_CODE
  unsigned short *overlay_bits;
  int i;

  /* open connect to the frame grabber if required */
  if (VCAAConnectionOpen == B_FALSE) {
    if ((VCAAFd = open("/dev/vca0", O_RDWR)) == -1) {
      ErrorMsg("Cannot open connection to /dev/vca0");
      return;
    }

    /* flag open connection */
    VCAAConnectionOpen = B_TRUE;
  }

  /* initialize VCAA structure */
  VCAAMode = VCA_MODE_OVERLAY;
  VcaControls.source = VCA_COMPOSITE;
  VcaControls.format = VCA_FORMAT_664;
  VcaControls.sync_on_green = VCA_IGNORE;
  VcaControls.display_enable = VCA_IGNORE;
  VcaControls.gen_lock = VCA_IGNORE;
  VcaControls.sync_lock = VCA_IGNORE;
  VcaControls.capture_position = VCA_IGNORE;
  VcaControls.plane = VCA_IGNORE;
  
  /* set general controls */
  vca_set_controls(VCAAFd, &VcaControls);
  VCAAFormat = VcaControls.format;
  VCAASource = VcaControls.source;

  /* clear the overlay plane bits */
  vca_window(VCAAFd, VCA_ON, 0, 0, 640, 480);
  vca_plane(VCAAFd, VCA_OVERLAY_PLANE);
  lseek(VCAAFd, 0, SEEK_SET);
  if ((overlay_bits =
       (unsigned short *) XvgMM_Alloc((void *) overlay_bits,
				      640 * sizeof(unsigned short))) == NULL) {
    ErrorMsg("Cannot open malloc in VCAAInit()");
    return;
  }
  for (i = 0; i < 640; i++)
    overlay_bits[i] = 0x0;
  for (i = 0; i < 480; i++)
    write(VCAAFd, overlay_bits, 640 * sizeof(ushort));
  XvgMM_Free(overlay_bits);

  /* set mode & window specs */
  vca_plane(VCAAFd, VCA_IMAGE_PLANE);
  vca_window(VCAAFd, VCA_ON, FrameX, FrameY, FrameWidth, FrameHeight);
  vca_mode(VCAAFd, VCAAMode);

  if (VerboseFlag)
    printf("VCAA initialized.\n");
#endif
}

/*************************** ClearPPort ***********************************/
/*                                                                        */
/*    Clear the parallel port bits                                        */
/*                                                                        */
/**************************************************************************/
void ClearPPort(void)
{
  volatile unsigned char *data_p;
  volatile unsigned long busaddr;

#ifdef PPORT_CODE
  PPortOutVal = 0;
  busaddr = busio_att();
  data_p = busaddr + PPORT_DATA;
  BUSIO_PUTC(data_p, 0);
  busio_det(busaddr);
#endif
}

/*************************** LightSourceSelect() **************************/
/*                                                                        */
/*    Toggle betwwen laset & lamp via the 110VAC digital switch           */
/*                                                                        */
/**************************************************************************/
void LightSourceSelect(int v)
{
  volatile unsigned char *data_p;
  volatile unsigned long busaddr;

#ifdef PPORT_CODE
  /* set/reset the 110VAC digital switch bit in the parallel port */
  if (v == 0)
    PPortOutVal &= ~0x04;
  else
    PPortOutVal |= 0x04;

  /* map the Micro Channel address space and define register pointers */
  busaddr = busio_att();
  data_p = busaddr + PPORT_DATA;
  BUSIO_PUTC(data_p, PPortOutVal | 0x02);
  busio_det(busaddr);
#endif
}

/******************************* sleep_us *********************************/
/*                                                                        */
/*    Accurate timing routine                                             */
/*                                                                        */
/**************************************************************************/
void sleep_us(int delay)
{
  double t;
  for (t = second() + ((double) delay)*0.000001; t > second(););
}


/********************* SGI frame grabber support code  *********************/
/*                                                                         */
/*  Support code for SGI frame buffer access & control                     */
/*                                                                         */
/***************************************************************************/
#if defined (SGI) && defined (SVIDEO)
static SVhandle SGIVideo;
static char *svErrorMsgs[] = {
  "DUMMY",
  "SV_BAD_HANDLE (bad pointer)",
  "SV_BADOPEN (unable to open video device)",
  "SV_BADSTAT (bad stat of video device)",
  "SV_NODEV (video device doesn't exist)",
  "SV_BAD_INFO (bad info call on video driver)",
  "SV_BAD_ATTACH (unable to attach to video device)",
  "SV_NO_MEM (no memory available)",
  "SV_NO_GL (no GL support)",
  "SV_BAD_VALUE (Bad value of argument to routine)",
  "SV_NO_WINDOW (svBindWindow not done yet)",
  "SV_NO_INIT_CAP (svInitCapture not done yet)",
  "SV_INIT_CAP (cannot call after svInitCapture)",
  "SV_EXCLUSIVE (board already in exclusive mode)",
  "SV_NO_X (no X server with video available)"
};

boolean_t SGIGetVideoFBParms(void)
{
  long int pvbuf[2];

  /* open the connection to the frame grabber memory */
  if ((SGIVideo = svOpenVideo()) == 0) {
    sprintf(cbuf, "SGIGetVideoFBParms() : Error in svOpenVideo() call - %s",
	    svErrorMsgs[svideo_errno]);
    ErrorMsg(cbuf);
    return(B_FALSE);
  }
  
  /* fetch the BROADCAST video parameters */
  pvbuf[0] = SV_BROADCAST;
  if (svGetParam(SGIVideo, pvbuf, 2) != 0) {
    sprintf(cbuf, "SGIGetVideoFBParms() : Error in svGetParam() call - %s",
	    svErrorMsgs[svideo_errno]);
    ErrorMsg(cbuf);
    svCloseVideo(SGIVideo);
    return(B_FALSE);
  }
  /* get size parameters */
  if (SV_PAL==pvbuf[1]) {
    SGIFBWidth = SV_PAL_XMAX;
    SGIFBHeight = SV_PAL_YMAX;
#ifdef DEBUG
    printf("SGIGetVideoFBParms() : PAL (%dx%d)\n", SGIFBWidth, SGIFBHeight);
#endif
  }
  else {
    SGIFBWidth = SV_NTSC_XMAX;
    SGIFBHeight = SV_NTSC_YMAX;
#ifdef DEBUG
    printf("SGIGetVideoFBParms() : NTSC (%dx%d)\n", SGIFBWidth, SGIFBHeight);
#endif
  }

  /* allocate storage for the frame buffer */
  UxFree(SGIFrameBuffer);
  if ((SGIFrameBuffer =
       (unsigned long *)UxMalloc(SGIFBWidth*SGIFBHeight*sizeof(unsigned long)))
      == NULL) {
    ErrorMsg("SGIGetVideoFBParms() : Error allocating memory");
    svCloseVideo(SGIVideo);
    return(B_FALSE);
  }

  /* return */
  SGIFBInitialized = B_TRUE;
  return(B_TRUE);
}

boolean_t SGILiveVideoOn(void)
{
  long svParms[2];
  
  /* freeze video input */
  svParms[0] = SV_OUTPUTMODE;
  svParms[1] = SV_LIVE_OUTPUT;
  if (svSetParam(SGIVideo, svParms, 2) != 0) {
    sprintf(cbuf, "SGILiveVideoOn() : Error in svSetParam() call - %s",
	    svErrorMsgs[svideo_errno]);
    ErrorMsg(cbuf);
    return(B_TRUE);
  }
#ifdef DEBUG
  printf("SGILiveVideoOn() : Live video restored\n");
#endif
  
  /* return */
  return(B_TRUE);
}

boolean_t SGILoadFrame(void)
{
  char cbuf[1024];

  /* write the image to the frame brabber video memory */
  if (svPutFrame(SGIVideo, (char *) SGIFrameBuffer) != 0) {
    sprintf(cbuf, "SGILoadFrame() : Error in svPutFrame() call - %s",
	    svErrorMsgs[svideo_errno]);
    ErrorMsg(cbuf);
    return(B_TRUE);
  }
  
  /* return */
  return(B_TRUE);
}

boolean_t SGICloseVideo(void)
{
  svCloseVideo(SGIVideo);
  return(B_TRUE);
}
#endif


/********************* VIDEO RECORDER SUPPORT CODE *************************/
/*                                                                         */
/*  Support code for the Sony Hi-8 VCR recorder                            */
/*                                                                         */
/***************************************************************************/
#ifdef VCR_CODE
extern Widget InfoDialog;
int		RS232;
struct termio	Setup;

/* initialization bytes */
int n_initbytes = 10;
char initbytes[] = {
  0xdf,0xc6,                    /* CONTROLLER ENABLE */
  0xde,0x90,                    /* IN ENTRY */
  0xdf,0xc0,0x32,0x34,0x30,0x40 /* EDIT PRESET(VIDEO/AFM INSERT EDIT), ENTER */
  };
/* frame recording bytes */
int n_recordbytes = 7;
char recordbytes[] = {	0xdf,0x92,
			0x30,0x30,0x30,0x31,    /* 0001 frames/recording */
			0x40 };
/* Set current point bytes (IN-ENTRY) */
int n_in_entry_bytes = 2;
char in_entry_bytes[] = {0xde,0x90};

/* other globals */
char ReplyBytes[1024];

void FlushSPort(void)
{
  int c;
  for (; read(RS232,&c,1) != 0;);
}

static boolean_t VCRSendBytes(char *buffer, int n)
{
  static char c;
  int i, j;

  for (j = 0, FlushSPort(); j < n; j++) {
    /* send byte */
    write(RS232, &(buffer[j]), 1);
    /* read the return code, check for timeout */
    if (read(RS232, &c, 1) != 1) {
      ErrorMsg("Hi8 recorder timeout : Check that it's in REMOTE mode");
      return(B_FALSE);
    }
    /* check for NAK */
    if (c != 0x0a) {
      /* fetch the error code */
      sprintf(ReplyBytes, "0x%02x", c);
      while (read(RS232, &c, 1) == 1)
	sprintf(ReplyBytes, "%s, 0x%02x", ReplyBytes, c);
      sprintf(cbuf, "NAK reported by the Hi8 Recorder : (Check that it's in PLAY/PAUSE mode and that the tape is writable), Reply=[%s] after byte %d of ", ReplyBytes, j);
      for (i = 0; i < j; i++)
	sprintf(cbuf, "%s0x%02x, ", cbuf, buffer[i]);
      sprintf(cbuf, "%s0x%02x", cbuf, buffer[j]);
      ErrorMsg(cbuf);
      return(B_FALSE);
    }
  }
  return(B_TRUE);
}

boolean_t VCRReset(void)
{
#ifndef CMGUI
  extern Widget errorDialog;
#endif
  boolean_t rc;
  int i;

  for (i = 0, VCRFrameForeward(80), rc = B_FALSE;
       (i < 10) && ((rc = VCRSendBytes(initbytes,n_initbytes)) == B_FALSE);
       i++, VCRFrameForeward(1));
  if (rc) {
    if (i > 0) {
#ifndef CMGUI
      UxPopdownInterface(errorDialog);
#endif
      XSync(UxDisplay, B_FALSE);
    }
    return(B_TRUE);
  }
  else
    return(B_FALSE);
}

boolean_t VCRInitVideo(void)
{
  static boolean_t RS232Open = B_FALSE;

  if (RS232Open == B_FALSE) {
#ifdef DEBUG
    printf("VCRInitVideo() => Opening RS232 connection...\n");
#endif
    /*--- open tty for read-write ---*/
    if ((RS232 = open(VCRtty, O_RDWR | O_NOCTTY, 0666)) == -1) {
      sprintf(cbuf, "open %s", VCRtty);
      perror(cbuf);
      return(B_FALSE);
    }
    
    /*--- this call is need for probably for some suspect 
      line-disciplin settings ---*/
    if (ioctl(RS232, TCGETA, &Setup) == -1) {
      sprintf(cbuf, "get setup %s", VCRtty);
      perror(cbuf);
      return(B_FALSE);
    }
    
    Setup.c_iflag = IGNBRK | IXOFF | IXON;
    Setup.c_oflag = OPOST | ONLCR;
    Setup.c_cflag = B9600 | CS8 | CREAD | CLOCAL;
    Setup.c_lflag = 0;
    Setup.c_cc[VINTR] = 0;
    Setup.c_cc[VQUIT] = 0;
    Setup.c_cc[VERASE] = 0;
    Setup.c_cc[VKILL] = 0;
    Setup.c_cc[VEOF] = 0;
    Setup.c_cc[VMIN] = 0;
    Setup.c_cc[VEOL] = 0;
    Setup.c_cc[VTIME] = 5;	/* timeout: 0.5 sec */
    Setup.c_cc[VEOL2] = 0;
#ifdef SGI
    Setup.c_line=LDISC0;
#endif
    if (ioctl(RS232, TCSETA, &Setup) == -1) {
      sprintf(cbuf, "set setup %s", VCRtty);
      perror(cbuf);
      return(B_FALSE);
    }
    RS232Open = B_TRUE;
  }
  
  /* send initialization bytes */
  HGCursor(B_TRUE);
  if (VCRSendBytes(initbytes, n_initbytes) == B_TRUE) {
    VCRInitDone = B_TRUE;
    HGCursor(B_FALSE);
    return(B_TRUE);
  }
  else {
    HGCursor(B_FALSE);
    return(B_FALSE);
  }    
}


boolean_t VCRFrameBackward(int nframes)
{
  static char c, F1F = 0x2C;
  int j;

  /* check that VCR is initialized */
  if (VCRInitDone == B_FALSE) {
    ErrorMsg("Initialize the Hi8 recorder with InitVCR button");
    return(B_FALSE);
  }
  
  /* loop to send frame reverse commands */
  HGCursor(B_TRUE);
  for (j = 0, cbuf[0] = '\0'; j < nframes; j++) {
    write(RS232, &F1F, 1);
    read(RS232, &(ReplyBytes[j]), 1);
    sprintf(cbuf, "%s 0x%02x", cbuf, ReplyBytes[j]);
  }
  HGCursor(B_FALSE);

#ifdef DEBUG
  printf("VCRFrameBackward() => %d frames reversed\n", nframes);
#endif
  return(B_TRUE);
}

boolean_t VCRFrameForeward(int nframes)
{
  static char c, F1F = 0x2B;
  int j;

  /* check that VCR is initialized */
  if (VCRInitDone == B_FALSE) {
    ErrorMsg("Initialize the Hi8 recorder with InitVCR button");
    return(B_FALSE);
  }
  
  /* loop to send frame advance commands */
  HGCursor(B_TRUE);
  for (j = 0, cbuf[0] = '\0'; j < nframes; j++) {
    write(RS232, &F1F, 1);
    read(RS232, &(ReplyBytes[j]), 1);
    sprintf(cbuf, "%s 0x%02x", cbuf, ReplyBytes[j]);
  }
  HGCursor(B_FALSE);

#ifdef DEBUG
  printf("VCRFrameForward() => %d frames advanced\n", nframes);
#endif
  return(B_TRUE);
}

boolean_t VCRRecord(int nframes)
{
  static char c, TCode;
  int j, num0, num1, num2;

  /* check that VCR is initialized */
  if (VCRInitDone == B_FALSE) {
    ErrorMsg("Initialize the Hi8 recorder with InitVCR button (Note that this will set the recording position to the current frame)");
    return(B_FALSE);
  }
  
  /* set the "number of frame" bytes */
  num2 = nframes / 100;
  num1 = (nframes - num2*100) / 10;
  num0 = (nframes - num1*10 - num2*100);
  recordbytes[3] = 0x30 + num2;
  recordbytes[4] = 0x30 + num1;
  recordbytes[5] = 0x30 + num0;

  /* send record commands & wait for termination code */
  if (VCRSendBytes(recordbytes, n_recordbytes) == B_FALSE)
    return(B_FALSE);
  for (; read(RS232, &TCode, 1) != 1;);
  
  /* check for termination code */
  if (TCode != 1) {
    sprintf(ReplyBytes, "0x%02x", TCode);
    while (read(RS232, &c, 1) == 1)
      sprintf(ReplyBytes, "%s, 0x%02x", ReplyBytes, c);
    sprintf(cbuf, "Wrong code reported by the Hi8 recorder, reply=[%s]",
	    ReplyBytes);
    ErrorMsg(cbuf);
    return(B_FALSE);
  }

#if defined (SGI) && defined (SVIDEO)
  /* restore SGI live video */
  SGILiveVideoOn();
#endif

  /* return */
#ifdef DEBUG
      printf("VCRRecord() => %d frames recorded\n", nframes);
#endif
  return(B_TRUE);
} 

/* IPOP to record n frames */
void RecordFrames(double *inpixels, double *outpixels, int height, int width,
		  void *ParmPtrs[])
{
  boolean_t BoolTmp1, BoolTmp2;
  int i, n, IntTmp;
  
  /* load parameters */
  n = *((int *) ParmPtrs[0]);
  if ((n < 1) || (n > 499)) {
    ErrorMsg("Abort : NFrames out of range [1..499] in RecordFrames()");
    Abort = B_TRUE;
    return;
  } 
  
  /* save state variables locally and reset them as required */
  BoolTmp1 = VCRRecordOn;
  BoolTmp2 = VCAWriteOn;
  IntTmp = VCRFramesPerImage;
  VCRRecordOn = B_TRUE;
  VCAWriteOn = B_TRUE;
  VCRFramesPerImage = n;
  
  /* disable screen saver */
  ScreenSaverOff();

  /* record frames */
  draw_mesg("Recording frames...");
  if (DumpImage() == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort : Error executing RecordFrames()");
  }
  
  /* reenable screen saver */
  ScreenSaverOn();

  /* restore system state variables */
  VCRRecordOn = BoolTmp1;
  VCAWriteOn = BoolTmp2;
  VCRFramesPerImage = IntTmp;
  
  /* copy data to the output buffer */
  for (i = 0; i < (height * width); i++)
    outpixels[i] = inpixels[i];
}

#else
boolean_t VCRFrameBackward(int nframes) {return(B_FALSE);}
boolean_t VCRFrameForeward(int nframes) {return(B_FALSE);}
boolean_t VCRInitVideo(void) {return(B_FALSE);}
boolean_t VCRRecord(int nframes) {return(B_FALSE);}
boolean_t VCRReset(void) {return(B_FALSE);}
void RecordFrames(double *inpixels, double *outpixels,
		  int height,int width, void *ParmPtrs[]) {}
#endif

/* 
   Algorithm for drawing an autostereogram
   based on Thimbleby, H.W., Inglis, S. and Witten, I.H.
   COMPUTER, October 1994, 38-48.
*/

#define separation(Z) ((int) rint((1.0 - mu*Z)*E/(2.0 - mu*Z)))
                            /* Stereo separation corresponding to position Z */
#define far separation(0)         /* ....and corresponding to far plane, Z=0 */
#define SQRSIZ 5

void RandomDotStereogram(double *inpixels, double *outpixels,
			 int height, int width, void *ParmPtrs[])
{
  double max, min, scale, E, MU_E_INV, mu, DPI;
  int i, j, x, y, mask, minsep=MAXINT, maxsep=0;
  int *pix;                                           /* Color of this pixel */
  int *same;                           /* Points to a pixel to the right ... */
                                  /*... that is constrained to be this color */

  /* load input parameters */
  mu = *((double *) ParmPtrs[1]);
  DPI = *((double *) ParmPtrs[2]);
  switch (*((int *) ParmPtrs[0]))
    {
    default:
    case 1: /* binary */
      mask = 0x01;
      break;
    case 2: /* 4 colors */
      mask = 0x03;
      break;
    case 3: /* 8 colors */
      mask = 0x07;
      break;
    case 4: /* 16 colors */
      mask = 0x0f;
      break;
    case 5: /* 32 colors */
      mask = 0x1f;
      break;
    case 6: /* 64 colors */
      mask = 0x3f;
      break;
    }

  /* normalize the object data to [0..1] */
  DynamicRange(inpixels, height*width, &max, &min);
  for (i = 0, scale = 1.0 / (max - min); i < height*width; i++)
    outpixels[i] = (inpixels[i] - min)*scale;

  /* set computation constants */
  E = rint(2.5*DPI);        /* Eye separation is assumed to be 2.5 inches */
  MU_E_INV = 1.0/(mu*E);

  /* allocate dynamic storage */
  pix = UxMalloc(width * sizeof(int));
  same = UxMalloc(width * sizeof(int)); 
  
  /* Loop for all scanlines, converting each scan line independently */
  for(y = 0; y < height; y++)
    {
      int s;                        /* Stereo separation at this (x,y) point */
      int left, right;      /* x values corresponding to left and right eyes */
      
      /* Loop to set each pixel initially linked with itself */
      for( x = 0; x < width; x++ )
	same[x] = x;
      
      /* loop for all points in the object */
      for( x = 0; x < width; x++ )
	{
	  /* stereo separation of two image points at the screen */
	  s = separation(outpixels[y*width + x]);
	  left = x - (s + (s & y & 1)) / 2;
	  right = left + s;

	  /* update maximum and minimum separation */
	  minsep = (s < minsep ? s : minsep);
	  maxsep = (s > maxsep ? s : maxsep);

	  /* draw points if both images are on the screen */
	  if((0 <= left) && (right < width))
	    {
	      int visible;          /* First, perform hidden-surface removal */
	      int t = 1;     /* We will check the points (x-t,y) and (x+t,y) */
	      float zt;                /* Z-coord of ray at these two points */

	      /* loop in neighboring points for hidden surface removal */
	      do
		{ 
		  zt = outpixels[y*width + x]
		    + 2.0*(2.0 - mu*outpixels[y*width +x])
		      *((double) t)*MU_E_INV;
		  visible = (outpixels[y*width + x - t] < zt)
		    && (outpixels[y*width + x + t] <zt); /* B_FALSE if obscured*/
		  t++;
		} while( visible && zt < 1 );  /* Done hidden-surface removal*/

	      /* if the point is not obscured, record the constraints */
	      if (visible)
		{
		  int k;
		  for(k = same[left]; (k != left) && (k != right);
		      k = same[left])
		    {
		      if(k < right)   /* But first, juggle the pointers ... */
			left = k;       /* ... until either same[left]=left */
		      else                       /* ... or same[right]=left */
			{
			  left = right;
			  right = k;
			}
		    }
		  same[left] = right; /* This is where we actually record it */
		}
	    }
	}

      /* loop to set pixels to actual colors */
      for(x = width-1; x >= 0; x--)  /* Now set the pixels on this scan line */
	{
	  if( same[x] == x )
	    {
	      pix[x] = random() & mask;       /* Free choice, do it randomly */
	    }
	  else
	    {
	      pix[x] = pix[same[x]];  /* Constrained choice; obey constraint */
	    }
	  outpixels[y*width + x] = pix[x];
	}
    }

  /* draw convergence squares near bottom */
  y = height - 4*SQRSIZ;
  x = width/2 - far/2;
  for (j = y - SQRSIZ; j < (y + SQRSIZ); j++)
    for (i = x - SQRSIZ; i < (x + SQRSIZ); i++)
      {
	outpixels[j*width + i] = 0;
	outpixels[j*width + i + far] = 0;
      }

  /* free storage */
  UxFree(pix);
  UxFree(same);

  /* show stats */
  sprintf(Msg, " Separation range: [%d, %d] ", minsep, maxsep);
}

