/***********************************************************************
*
*  Name:          grabIPOPs.c
*
*  Author:        Paul Charette
*
*  Last Modified: 12 August 1993
*
*  Purpose:       Utility routines garbbing operations
*                   GrabFrame()
*                   GrabStep()
*                   StepMinus()
*                   StepPlus()
*
***********************************************************************/
#include <stdio.h>
#include "UxXt.h"
#include <fcntl.h>
#include <memory.h>
#include <search.h>
#include <values.h>
#ifdef PPORT_CODE
#include <sys/ioacc.h>
#endif
#include "XvgGlobals.h"

/**********************   Double grab frame     ****************************/
/*                                                                         */
/*  function: grabe two frames within a fixed delay and subtract           */
/*                                                                         */
/***************************************************************************/
void DoubleGrabSubtract(double *inpixels, double *outpixels,
			int height, int width, void *ParmPtrs[])
{
  int i, delay;
  
  /* load input parms */
  delay = *((int *) ParmPtrs[0]);

  /* init grabbing if required */
  if (GrabbingOn == B_FALSE)
    GrabbingState(B_TRUE);

  /* grab frames */
  Grab(inpixels, RawPsBufsX, B_TRUE, B_TRUE);
  sleep_us(delay*1000000);
  Grab(outpixels, RawPsBufsX, B_TRUE, B_TRUE);

  /* subtract */
  for (i = 0; i < height*width; i++)
    outpixels[i] = outpixels[i] - inpixels[i];
}


/**********************      Grab frame         ****************************/
/*                                                                         */
/*  function: grabe frame and load it                                      */
/*                                                                         */
/***************************************************************************/
void GrabFrame(double *inpixels, double *outpixels,
	       int height, int width, void *ParmPtrs[])
{
  int i;
  
  /* push current image onto the stack */
  if (XvgImgStack_Push(inpixels, height, width) == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort : Failed pushing current image in GrabFrame()");
    return;
  }

  /* init grabbing if required */
  if (GrabbingOn == B_FALSE)
    GrabbingState(B_TRUE);
  else
    /* grab frame */
    Grab(outpixels, (unsigned short *) inpixels, B_TRUE, B_TRUE);
}


/**********************      StepPlus     **********************************/
/*                                                                         */
/*  function: step membrane radius by one increment                        */
/*                                                                         */
/***************************************************************************/
void StepPlus(double *inpixels, double *outpixels,
	      int height, int width, void *ParmPtrs[])
{
/*
  int i, tmp;

  tmp = GrabbingControl;
  GrabbingControl = 0;
  StepPlusCB();
  GrabbingControl = tmp;
  HGCursor(B_TRUE);

  for (i = 0; i < (height * width); i++)
    outpixels[i] = inpixels[i];
*/
}

/**********************      StepMinus    **********************************/
/*                                                                         */
/*  function: step membrane radius by minus one increment                  */
/*                                                                         */
/***************************************************************************/
void StepMinus(double *inpixels, double *outpixels,
	       int height, int width, void *ParmPtrs[])
{
/*
  int i, tmp;

  tmp = GrabbingControl;
  GrabbingControl = 0;
  StepMinusCB();
  GrabbingControl = tmp;
  HGCursor(B_TRUE);

  for (i = 0; i < (height * width); i++)
    outpixels[i] = inpixels[i];
*/
}

/**************************** Grab & step **********************************/
/*                                                                         */
/*  function: grab two frames, step in between (debugging)                 */
/*                                                                         */
/***************************************************************************/
void GrabStep(double *inpixels, double *outpixels,
	      int height, int width, void *ParmPtrs[])
{
  volatile unsigned char *data_p;
  volatile unsigned long busaddr;
  int i, nsteps, delay;

  /* load parameters */
  nsteps = *((int *) ParmPtrs[0]);
  delay = *((int *) ParmPtrs[1]);

  /* grab frames */
  Grab(inpixels, RawPsBufsX, B_TRUE, B_TRUE);
  
#ifdef PPORT_CODE
  /* map the Micro Channel address space and define register pointers */
  busaddr = busio_att();
  data_p = busaddr + PPORT_DATA;
  
  /* loop for steps */
  for (i = 0; i < nsteps; i++) {
    BUSIO_PUTC(data_p, PPortOutVal | 0x02);
    usleep(25);
    BUSIO_PUTC(data_p, PPortOutVal & ~0x02);
    usleep(75);
  }
  
  /* detach the Micro Channel address space   */
  busio_det(busaddr);
#endif
  
  /* grab second frame */
  sleep_us(delay*1000000);
  Grab(outpixels, RawPsBufsX, B_TRUE, B_TRUE);

  /* subtract & wrap */
  for (i = 0; i < height*width; i++) {
    outpixels[i] = fmod((outpixels[i] - inpixels[i]), TWOPI);
    if (outpixels[i] > PI)
      outpixels[i] -= TWOPI;
    else if (outpixels[i] <= -PI)
      outpixels[i] += TWOPI;
  }
}


