/***********************************************************************
*
*  Name:          binaryIPOPs.c
*
*  Author:        Paul Charette
*
*  Last Modified: 26 Nov 1994
*
*  Purpose:       Utility routines binary IP operations
*                   OR()
*                   Skeleton()
***********************************************************************/
#include <stdio.h>
#include "UxXt.h"
#include <fcntl.h>
#include <search.h>
#include <values.h>
#include "XvgGlobals.h"

/********************************* OR **************************************/
/*                                                                         */
/*  function: Logical OR with the image on the stack                       */
/*                                                                         */
/***************************************************************************/
void OR(double *inpixels, double *outpixels,
	int height, int width, void *ParmPtrs[])
{
  int i;

  /* check that stack is not empty, then decrement stack pointer  */
  if (ImageStackTop <= 0) {
    Abort = B_TRUE;
    draw_mesg("Abort : Image stack is empty in OR()");
    return;
  }
  ImageStackTop--;
  UpdateStackLabel(ImageStackTop);
  
  /* check that height and width match the current image size */
  if   ((ImageStack[ImageStackTop].height != height)
	|| (ImageStack[ImageStackTop].width != width)) {
    Abort = B_TRUE;
    draw_mesg("Abort: Image on stack is not the current size in OR()");
    return;
  }
  
  /* transfer data and OR */
  for (i = 0; i < height*width; i++)
    if ((inpixels[i] != 0) || (ImageStack[ImageStackTop].pixels[i] != 0))
      outpixels[i] = 1.0;
    else
      outpixels[i] = 0.0;
}

/**************************** Skeletonisation ******************************/
/*                                                                         */
/*  function: Skeletonisation of a BINARY image [0,1]                      */
/*            "Digital Image Processing", Gonzalez & Woods, 1992, p491     */
/*                                                                         */
/***************************************************************************/
void Skeleton(double *inpixels, double *outpixels,
	      int height, int width, void *ParmPtrs[])
{
  int *marks;
  int MaxGap, MinLen, i, j, k=1, mnum, MnumTotal, mnum1, mnum2, Sp;
  double p1, p2, p3, p4, p5, p6, p7, p8, p9, Np;

  /* load parameters */
  MinLen = *((int *) ParmPtrs[0]);
  MaxGap = *((int *) ParmPtrs[1]);

  /* allocate storage if required */
  if ((marks = (int *) UxRealloc(NULL, height*width*sizeof(int))) == NULL) {
    Abort = B_TRUE;
    draw_mesg("Abort : Could not allocate in Skeleton()");
    return;
  }

  /* transfer data to outpixels buffer */
  for (i = 0; i < (height*width) ; i++)
    outpixels[i] = inpixels[i];

  do {
    /* reset total marked pixel counter to 0 */
    MnumTotal = 0;

    /* mark points for deletion from the top */
    sprintf(cbuf, "Skeletonisation - Top/down thinning iteration %d", k);
    draw_mesg(cbuf);
    for (j = 1, mnum=0; j < (width-1); j++)
      for (i = 1; (i < (height-1)) && (Abort == B_FALSE); i++)
	if (outpixels[i*width + j] == 1) {
	  p8 = outpixels[(i-1)*width + j-1];
	  p1 = outpixels[(i-1)*width + j];
	  p2 = outpixels[(i-1)*width + j+1];
	  p7 = outpixels[i*width + j-1];
	  p3 = outpixels[i*width + j+1];
	  p6 = outpixels[(i+1)*width + j-1];
	  p5 = outpixels[(i+1)*width + j];
	  p4 = outpixels[(i+1)*width + j+1];
	  Np = p1+p2+p3+p4+p5+p6+p7+p8;
	  Sp = ((((p3 == 1)&&(p4 == 0))||((p2 == 0)&&(p4 == 1))) ? Sp+1 : 0);
	  Sp = ((((p5 == 1)&&(p6 == 0))||((p4 == 0)&&(p6 == 1))) ? Sp+1 : Sp);
	  Sp = ((((p7 == 1)&&(p8 == 0))||((p6 == 0)&&(p8 == 1))) ? Sp+1 : Sp);
	  Sp = ((((p1 == 1)&&(p2 == 0))||((p8 == 0)&&(p2 == 1))) ? Sp+1 : Sp);
	  if ((Np > 1) && (Sp == 1))
	    marks[mnum++] = i*width + j;
	  for (; (i < (height-1)) && (outpixels[i*width + j] == 1); i++);
	}
    /* delete points marked for deletion */
    MnumTotal += mnum;
    for (i = 0; i < mnum ; i++)
      outpixels[marks[i]] = 0;
    ShowImage(outpixels);
    
    /* mark points for deletion from the bottom */
    sprintf(cbuf, "Skeletonisation - Bottom/up thinning iteration %d", k);
    draw_mesg(cbuf);
    for (j = 1, mnum=0; j < (width-1); j++)
      for (i = height-1; (i > 1) && (Abort == B_FALSE); i--)
	if (outpixels[i*width + j] == 1) {
	  p8 = outpixels[(i-1)*width + j-1];
	  p1 = outpixels[(i-1)*width + j];
	  p2 = outpixels[(i-1)*width + j+1];
	  p7 = outpixels[i*width + j-1];
	  p3 = outpixels[i*width + j+1];
	  p6 = outpixels[(i+1)*width + j-1];
	  p5 = outpixels[(i+1)*width + j];
	  p4 = outpixels[(i+1)*width + j+1];
	  Np = p1+p2+p3+p4+p5+p6+p7+p8;
	  Sp = ((((p3 == 1)&&(p4 == 0))||((p2 == 0)&&(p4 == 1))) ? Sp+1 : 0);
	  Sp = ((((p5 == 1)&&(p6 == 0))||((p4 == 0)&&(p6 == 1))) ? Sp+1 : Sp);
	  Sp = ((((p7 == 1)&&(p8 == 0))||((p6 == 0)&&(p8 == 1))) ? Sp+1 : Sp);
	  Sp = ((((p1 == 1)&&(p2 == 0))||((p8 == 0)&&(p2 == 1))) ? Sp+1 : Sp);
	  if ((Np > 1) && (Sp == 1))
	    marks[mnum++] = i*width + j;
	  for (; (i > 1) && (outpixels[i*width + j] == 1); i--);
	}
    /* delete points marked for deletion */
    MnumTotal += mnum;
    for (i = 0; i < mnum ; i++)
      outpixels[marks[i]] = 0;
    ShowImage(outpixels);
    
    /* mark points for deletion from the left */
    sprintf(cbuf, "Skeletonisation - Left/right thinning iteration %d", k);
    draw_mesg(cbuf);
    for (i = 1, mnum=0; i < (height-1); i++)
      for (j = 1; (j < (width-1)) && (Abort == B_FALSE); j++)
	if (outpixels[i*width + j] == 1) {
	  p8 = outpixels[(i-1)*width + j-1];
	  p1 = outpixels[(i-1)*width + j];
	  p2 = outpixels[(i-1)*width + j+1];
	  p7 = outpixels[i*width + j-1];
	  p3 = outpixels[i*width + j+1];
	  p6 = outpixels[(i+1)*width + j-1];
	  p5 = outpixels[(i+1)*width + j];
	  p4 = outpixels[(i+1)*width + j+1];
	  Np = p1+p2+p3+p4+p5+p6+p7+p8;
	  Sp = ((((p3 == 1)&&(p4 == 0))||((p2 == 0)&&(p4 == 1))) ? Sp+1 : 0);
	  Sp = ((((p5 == 1)&&(p6 == 0))||((p4 == 0)&&(p6 == 1))) ? Sp+1 : Sp);
	  Sp = ((((p7 == 1)&&(p8 == 0))||((p6 == 0)&&(p8 == 1))) ? Sp+1 : Sp);
	  Sp = ((((p1 == 1)&&(p2 == 0))||((p8 == 0)&&(p2 == 1))) ? Sp+1 : Sp);
	  if ((Np > 1) && (Sp == 1))
	    marks[mnum++] = i*width + j;
	  for (; (j < (width-1)) && (outpixels[i*width + j] == 1); j++);
	}
    /* delete points marked for deletion */
    MnumTotal += mnum;
    for (i = 0; i < mnum ; i++)
      outpixels[marks[i]] = 0;
    ShowImage(outpixels);
    
    /* mark points for deletion from the right */
    sprintf(cbuf, "Skeletonisation - Right/left thinning iteration %d", k);
    draw_mesg(cbuf);
    for (i = 1, mnum=0; i < (height-1); i++)
      for (j = width-1; (j > 1) && (Abort == B_FALSE); j--)
	if (outpixels[i*width + j] == 1) {
	  p8 = outpixels[(i-1)*width + j-1];
	  p1 = outpixels[(i-1)*width + j];
	  p2 = outpixels[(i-1)*width + j+1];
	  p7 = outpixels[i*width + j-1];
	  p3 = outpixels[i*width + j+1];
	  p6 = outpixels[(i+1)*width + j-1];
	  p5 = outpixels[(i+1)*width + j];
	  p4 = outpixels[(i+1)*width + j+1];
	  Np = p1+p2+p3+p4+p5+p6+p7+p8;
	  Sp = ((((p3 == 1)&&(p4 == 0))||((p2 == 0)&&(p4 == 1))) ? Sp+1 : 0);
	  Sp = ((((p5 == 1)&&(p6 == 0))||((p4 == 0)&&(p6 == 1))) ? Sp+1 : Sp);
	  Sp = ((((p7 == 1)&&(p8 == 0))||((p6 == 0)&&(p8 == 1))) ? Sp+1 : Sp);
	  Sp = ((((p1 == 1)&&(p2 == 0))||((p8 == 0)&&(p2 == 1))) ? Sp+1 : Sp);
	  if ((Np > 1) && (Sp == 1))
	    marks[mnum++] = i*width + j;
	  for (; (j > 1) && (outpixels[i*width + j] == 1); j--);
	}
    /* delete points marked for deletion */
    MnumTotal += mnum;
    for (i = 0; i < mnum ; i++)
      outpixels[marks[i]] = 0;
    ShowImage(outpixels);
    
    /* increment loop variable */
    printf("Mnumtotal:%d\n", MnumTotal);
    k++;
  } while (MnumTotal > 0);

  UxFree(marks);
  return;

#ifdef NOTTHIS
  for (k = 1, mnum1=1; (mnum1 != 0) || (mnum2 != 0); k++) {
    /* mark points for deletion in pass #1 */
    sprintf(cbuf, "Skeletonisation - Thinning iteration %d pass 1", k);

    draw_mesg(cbuf);
    for (i = 1, mnum1=0; i < (height-1); i++)
      for (j = 1; j < (width-1); j++) {
	if (outpixels[i*width + j] == 1) {
	  p9 = outpixels[(i-1)*width + j-1];
	  p2 = outpixels[(i-1)*width + j];
	  p3 = outpixels[(i-1)*width + j+1];
	  p8 = outpixels[i*width + j-1];
	  p4 = outpixels[i*width + j+1];
	  p7 = outpixels[(i+1)*width + j-1];
	  p6 = outpixels[(i+1)*width + j];
	  p5 = outpixels[(i+1)*width + j+1];
	  Np = p2+p3+p4+p5+p6+p7+p8+p9;
	  Sp = (((p2 == 0) && (p3 == 1)) ? Sp+1 : 0);
	  Sp = (((p3 == 0) && (p4 == 1)) ? Sp+1 : Sp);
	  Sp = (((p4 == 0) && (p5 == 1)) ? Sp+1 : Sp);
	  Sp = (((p5 == 0) && (p6 == 1)) ? Sp+1 : Sp);
	  Sp = (((p6 == 0) && (p7 == 1)) ? Sp+1 : Sp);
	  Sp = (((p7 == 0) && (p8 == 1)) ? Sp+1 : Sp);
	  Sp = (((p8 == 0) && (p9 == 1)) ? Sp+1 : Sp);
	  if ((Np >= 2) && (Np <= 6) && (Sp == 1)
	      && ((p2*p4*p6) == 0) && ((p4*p6*p8) == 0))
	    marks[mnum1++] = i*width + j;
	}
      }
    /* delete points marked for deletion in pass #1 */
    for (i = 0; i < mnum1 ; i++)
	  outpixels[marks[i]] = 0;
      
    /* mark points for deletion in pass #2 */
    sprintf(cbuf, "Skeletonisation - Thinning iteration %d pass 2", k);
    draw_mesg(cbuf);
    for (i = 1, mnum2=0; i < (height-1); i++)
      for (j = 1; j < (width-1); j++) {
	if (outpixels[i*width + j] == 1) {
	  p9 = outpixels[(i-1)*width + j-1];
	  p2 = outpixels[(i-1)*width + j];
	  p3 = outpixels[(i-1)*width + j+1];
	  p8 = outpixels[i*width + j-1];
	  p4 = outpixels[i*width + j+1];
	  p7 = outpixels[(i+1)*width + j-1];
	  p6 = outpixels[(i+1)*width + j];
	  p5 = outpixels[(i+1)*width + j+1];
	  Np = p2+p3+p4+p5+p6+p7+p8+p9;
	  Sp = (((p2 == 0) && (p3 == 1)) ? Sp+1 : 0);
	  Sp = (((p3 == 0) && (p4 == 1)) ? Sp+1 : Sp);
	  Sp = (((p4 == 0) && (p5 == 1)) ? Sp+1 : Sp);
	  Sp = (((p5 == 0) && (p6 == 1)) ? Sp+1 : Sp);
	  Sp = (((p6 == 0) && (p7 == 1)) ? Sp+1 : Sp);
	  Sp = (((p7 == 0) && (p8 == 1)) ? Sp+1 : Sp);
	  Sp = (((p8 == 0) && (p9 == 1)) ? Sp+1 : Sp);
	  if ((Np >= 2) && (Np <= 6) && (Sp == 1)
	      && ((p2*p4*p6) == 0) && ((p4*p6*p8) == 0))
	    marks[mnum2++] = i*width + j;
	}
      }
    /* delete points marked for deletion in pass #2 */
    for (i = 0; i < mnum2 ; i++)
	  outpixels[marks[i]] = 0;
  }

  UxFree(marks);
#endif
}


