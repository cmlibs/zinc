/***********************************************************************
*
*  Name:          spacedomIPOPs.c
*
*  Author:        Paul Charette
*
*  Last Modified: 12 August 1993
*
*  Purpose:       Utility routines space domain IP operations
*
*                     AveragingFilter()
*                     Correlate()
*                     Detrend()
*                     Gradient()
*                     Laplacian()
*                     MedianFilter()
*                     MedianFilterByte()
*                     RadiusFilter()
*                     RegionAveragingFilter()
*                     WeightedAveragingFilter()
*                     ZeroEdges()
*
***********************************************************************/
#include <stdio.h>
#include "UxXt.h"
#include <fcntl.h>
#include <memory.h>
#include <search.h>
#include <values.h>
#include "XvgGlobals.h"

/****************************  Heapsort stuff  *****************************/
/*                                                                         */
/*  The heap is a binary tree where any node has a larger value than       */
/*  any of its children, and where all nodes internal nodes (non-leaf)     */
/*  have exactly two children, with the possible execption of the          */
/*  rightmost internal node.                                               */
/*                                                                         */
/*  If the heap is stored in an array, the left child of node i is at      */
/*  "2*i +1" and the right child of i is at "2*i +2".                      */
/*                                                                         */
/*  This code is based on the HEAPSORT algorythm                           */
/*  (See Sara Baase, Computer Algorithms, Addison-Wesley, 1983)            */
/*  Execution time is about 4 times faster than the system provided qsort  */
/*                                                                         */
/***************************************************************************/
/* macros */
#define LEFTCHILD(X)  ((2*X) +1)
#define RIGHTCHILD(X) ((2*X) +2)
#define PARENT(X)     ((X-1) /2)

/* build a heap from scratch by starting at the second to last level */
/* building sub-heaps and percolating upwards to the top (level 0)   */
static void BuildHeap(double *Heap, int HeapLen)
{
  double x, topval;
  int l, n, sn, depth, BiggestChildPos;

  /* compute the heap depth */
  x = log((double) (HeapLen+1)) / log(2.0);
  depth = itrunc(x);
  if (x != ((double) depth))
    depth++;

  /* for each level "l" from second-to-last, arrange all subtrees into heaps */
  for (l = depth-2; l >= 0; l--) {
    /* for each non-leaf node "n" at level "l", build a subheap */
    for (n = (1 << l) -1; n < (1 << (l+1)) -1; n++) {
      /* if node "n" has at least one child proceed, else its a leaf so skip */
      if (LEFTCHILD(n) < HeapLen) {
	topval = Heap[n];
	sn = n;
	/* while subnode "sn" is not a leaf traverse down until either */
	/* the topval node has found its niche or a leaf is reached */
	while (LEFTCHILD(sn) < HeapLen) {
	  /* find biggest child of the node "sn" */
	  if (RIGHTCHILD(sn) < HeapLen) {
	    /* node has two children, so find biggest one */
	    if (Heap[LEFTCHILD(sn)] > Heap[RIGHTCHILD(sn)])
	      BiggestChildPos = LEFTCHILD(sn);
	    else
	      BiggestChildPos = RIGHTCHILD(sn);
	  }
	  else
	    /* else just look at single child */
	    BiggestChildPos = LEFTCHILD(sn);

	  /* is this the niche for topval ? */
	  if (topval < Heap[BiggestChildPos]) {
	    /* no, so swap keys, move down one level and continue... */
	    Heap[sn] = Heap[BiggestChildPos];
	    sn = BiggestChildPos;
	    Heap[sn] = topval;
	  }
	  else
	    /* niche found, yes so exit while loop */
	    break;
	}
      }
      else
	/* node "n" is a leaf, exit for loop */
	break;
    }
  }
}

/* new node at the top of a heap, percolate it downwards until it finds */
/* its niche.                                                           */
static void FixHeap(double *Heap, int NewItemPos, int HeapLen)
{
  double topval;
  int sn, BiggestChildPos;

  sn = NewItemPos;
  topval = Heap[sn];
  /* while subnode "sn" is not a leaf traverse down until either */
  /* the topval node has found its niche or a leaf is reached */
  while (LEFTCHILD(sn) < HeapLen) {
    /* find biggest child of the node "sn" */
    if (RIGHTCHILD(sn) < HeapLen) {
      /* node has two children, so find biggest one */
      if (Heap[LEFTCHILD(sn)] > Heap[RIGHTCHILD(sn)])
	BiggestChildPos = LEFTCHILD(sn);
      else
	BiggestChildPos = RIGHTCHILD(sn);
    }
    else
      /* else just look at single child */
      BiggestChildPos = LEFTCHILD(sn);
    
    /* is this the niche for topval ? */
    if (topval < Heap[BiggestChildPos]) {
      /* no, so swap keys, move down one level and continue... */
      Heap[sn] = Heap[BiggestChildPos];
      sn = BiggestChildPos;
      Heap[sn] = topval;
    }
    else
      /* niche found, yes so exit while loop */
      break;
  }
}

static void HeapSort(double *Heap, int HeapLen)
{
  int l;
  double TmpVal;
  
  /* first build the heap */
  BuildHeap(Heap, HeapLen);

  /* remove the top nodes of the heap. one by one */
  for (l = HeapLen-1; l > 1; l--) {
    TmpVal = Heap[0];
    Heap[0] = Heap[l];
    Heap[l] = TmpVal;
    FixHeap(Heap, 0, l);
  }
  /* sort the last two entries */
  if (Heap[0] > Heap[1]) {
    TmpVal = Heap[0];
    Heap[0] = Heap[1];
    Heap[1] = TmpVal;
  }
}


/****************************** Averaging filter ***************************/
/*                                                                         */
/*  function: replace central pixel of an kern_w * kern_w neighborhood     */
/*            by the average value in the neighborhood, computed           */
/*            using a kern_w * kern_w size mask.                           */
/*                                                                         */
/***************************************************************************/
void AveragingFilter(double *inpixels, double *outpixels,
		     int height, int width, void *ParmPtrs[])
{
  int kern_w;
  double divisor_inv;

  /* load parameters */
  kern_w = *((int *) ParmPtrs[0]);
  divisor_inv = 1.0 / ((double) (kern_w * kern_w));

  /* check input parameters */
  if ((kern_w < 2) || (kern_w > width)) {
    Abort = B_TRUE;
    draw_mesg("Abort : Invalid kernel size in AveragingFilter()");
    return;
  }

  /* draw message */
/*
  if (VerboseFlag)
    printf("Averaging filter (%d x %d)...\n", kern_w, kern_w);
*/
  draw_mesg("Averaging filter...");

  if (kern_w < 2) {
    int i, j, k, kmin, kmax, l, lmin, lmax;
    double m, n;
    
    /* apply neighborhood averaging filter */
    for (j = 0; (j < height) && (Abort == B_FALSE); j++) {
      /* set bounds on y size of the kernel */
      kmin = (j >= kern_w/2 ? kern_w/2 : j);
      kmax = (j < (height - kern_w/2) ? kern_w/2 : (height - j - 1));
      for (i = 0; (i < width) && (Abort == B_FALSE); i++) {
	/* set bounds on the x size of the kernel */
	lmin = (i >= kern_w/2 ? kern_w/2 : i);
	lmax = (i < (width - kern_w/2) ? kern_w/2 : (width - i - 1));
	/* loop for one kernel */
	for (m=0, k = (j-kmin)*width, n = 0; k <= (j+kmax)*width; k+=width) {
	  for (l = i-lmin; l <= i+lmax; l++, n++)
	    m += inpixels[l+k];
	}
	outpixels[j*width + i] = m/n;
      }
    }
  }
    
  /* alternate sorting algorythm, more efficient for larger kernels */
  else {
    int i, j, k, average_offset;
    double sum, *p, *q;

    /* set vars */
    average_offset = (kern_w >> 1) * (width + 1);

    /* compute total from kernel area around first pixel in the fist line */
    for (i = 0, sum = 0; i < kern_w; i++)
      for (j = 0, p = &inpixels[i * width]; j < kern_w; j++, p++)
	sum += *p;

    /* loop for columns */
    for (i = 0, q = outpixels; (i < (width - kern_w)) && (Abort == B_FALSE);) {
      /* prime run: find first average and store it in outpixels */
      q[i + average_offset] = sum * divisor_inv;

      /* loop for rows, for top to bottom */
      for (j = 0; (j < (height - kern_w)*width) && (Abort == B_FALSE);) {
	/* subtract oldest row */
	for (k = 0, p = &inpixels[i+j]; k < kern_w; k++, p++)
	  sum -= *p;
	/* add new row */
	for (k = 0, p = &inpixels[i+j+(kern_w*width)];k < kern_w; k++,p++)
	  sum += *p;
	/* increment row counter */
	j +=width;
	/* store running average in outpixels */
	q[i + j + average_offset] = sum * divisor_inv;;
      }

      /* subtract oldest column */
      for (k = 0; k < kern_w; k++)
	sum -= inpixels[i+j+(k*width)];
      /* add new column */
      for (k = 0; k < kern_w; k++)
	sum += inpixels[i+j+kern_w+(k*width)];
      /* increment column counter */
      i += 1;
      /* prime run: compute first average and store it in outpixels */
      q[i + j + average_offset] = sum * divisor_inv;;

      /* loop for rows, for bottom to top (check for odd width images) */
      if (i <= (width - kern_w)) {
	for (j = (height - kern_w)*width; (j > 0) && (Abort == B_FALSE);) {
	  /* decrement row counter */
	  j -=width;
	  /* subtract oldest row */
	  for (k = 0, p = &inpixels[i+j+(kern_w*width)];k < kern_w; k++,p++)
	    sum -= *p;
	  /* add new row */
	  for (k = 0, p = &inpixels[i+j]; k < kern_w; k++, p++)
	    sum += *p;
	  /* compute running average and store it in outpixels */
	  q[i + j + average_offset] = sum * divisor_inv;
	}
	/* subtract oldest column */
	for (k = 0, p = &inpixels[i+j]; k < kern_w; k++, p+=width)
	  sum -= inpixels[i+j+(k*width)];
	/* add new column */
	for (k = 0; k < kern_w; k++)
	  sum += inpixels[i+j+kern_w+(k*width)];
	/* increment column counter */
	i += 1;
      }
    }
  }

  /* fill edges with raw data */
  if (Abort == B_FALSE) 
    FillEdges(inpixels, outpixels, height, width, kern_w);
  
#ifdef DEBUG
  printf("Neighborhood average filtering (height:%d, width:%d, kernel: %d)\n",
	 height, width, kern_w);
#endif
}

/*****************************   Correlate  ********************************/
/*                                                                         */
/*  function: replace central pixel of an kern_w * kern_w neighborhood     */
/*            by the correlation of the area with a mask.                  */
/*                                                                         */
/***************************************************************************/
void Correlate(double *inpixels, double *outpixels, int height, int width,
	       void *ParmPtrs[])
{
  int kern_w, n;
  double *mask;

  /* load parameters */
  kern_w = *((int *) ParmPtrs[0]);
  mask = ((DOUBLE_ARRAY *) ParmPtrs[1])->data;
  n = ((DOUBLE_ARRAY *) ParmPtrs[1])->n;

  /* check input parameters */
  if ((kern_w < 2) || (kern_w > width)) {
    Abort = B_TRUE;
    draw_mesg("Abort : Invalid kernel size in Correlate()");
    return;
  }

  /* draw message */
  draw_mesg("Correlation...");

  if (kern_w < 4) {
    double *p, *q;
    int i, j, k, l;
    double m;
    double buf[625];
    
    /* apply neighborhood averaging filter */
    for (j = (kern_w/2 * width), p = inpixels, q = outpixels;
	 j < (height - kern_w/2)*width; j += width) {
      for (i= kern_w/2; i < (width - kern_w/2); i++) {
	for (m=0, k = j-(kern_w/2*width); k <= j+(kern_w/2*width); k+=width) {
	  for (l = i - kern_w/2; l <= i + kern_w/2; l++)
	    m += p[l+k];
	}
	q[i+j] = m;
      }
    }
  }
}
    
/*****************************   Detrend  **********************************/
/*                                                                         */
/*                                                                         */
/***************************************************************************/
void Detrend(double *inpixels, double *outpixels, int height, int width,
	     void *ParmPtrs[])
{
  double A[3*3], AA[3*3], B[3], C[3], WKS1[3], WKS2[3],
  SumX, SumX2, SumY, SumY2, SumXY, SumXZ, SumYZ, SumZ, e, Var;
  int x, y, k, n, IFAIL, N;

  /* draw message */
  draw_mesg("Detrending...");

  /* make sure contour is defined */
  if (DisplayContourComputed == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort : Undefined contour in Detrend(), use Crop()...");
    return;
  }

  /* loop to set up normal equations */
  SumX=0; SumX2 = 0; SumY=0; SumY2=0; SumZ=0; SumXY=0; SumXZ=0; SumYZ=0;
  for (y = 0, n = 0, k = 0; y < height; y++) {
    for (x = 0; x < width; x++, k++) {
      /* update running sums */
      if (MaskPixels[k] != 0.0) {
	SumX  += (x);
	SumX2 += (x * x);
	SumY  += (y);
	SumY2 += (y * y);
	SumXY += (x * y);
	SumXZ += (((double) x) * inpixels[k]);
	SumYZ += (((double) y) * inpixels[k]);
	SumZ  += (inpixels[k]);
	n++;
      }
    }
  }

  /* compute plane model coefficients */
  A[0] = SumX2;
  A[1] = SumXY;
  A[2] = SumX;
  A[3] = SumXY;
  A[4] = SumY2;
  A[5] = SumY;
  A[6] = SumX;
  A[7] = SumY;
  A[8] = n;
  B[0] = SumXZ;
  B[1] = SumYZ;
  B[2] = SumZ;
  N = 3;
  IFAIL = 0;
  f04atf(A, &N, B, &N, C, AA, &N, WKS1, WKS2, &IFAIL);
  
  /* loop to detrend */
  for (y = 0, k = 0, Var = 0; y < height; y++) {
    for (x = 0; x < width; x++, k++) {
      if (MaskPixels[k] != 0.0) {
	e = inpixels[k] - (C[0]*((double) x) + C[1]*((double) y) + C[2]);
	outpixels[k] = e;
	Var += (e * e);
      }
      else {
	outpixels[k] = 0.0;
      }
    }
  }
  printf("Detrend() => stddev = %e\n", sqrt(Var / ((double) n)));
}
    
/*****************************   DetrendXYZ ********************************/
/*                                                                         */
/*                                                                         */
/***************************************************************************/
void DetrendXYZ(double *inpixels, double *outpixels, int height, int width,
	        void *ParmPtrs[])
{
  double A[3*3], AA[3*3], B[3], C[3], WKS1[3], WKS2[3], *X, *Y, 
  SumX, SumX2, SumY, SumY2, SumXY, SumXZ, SumYZ, SumZ, e, m0, m1;
  int i, j, n, samples, IFAIL, N;

  /* make sure contour is defined */
  if (DisplayContourComputed == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort : Undefined contour in DetrendXYZ(), use Crop()...");
    return;
  }

  /* check that stack contains at least 2 images */
  if (ImageStackTop < 2) {
    Abort = B_TRUE;
    draw_mesg("Abort : 2 stack images required in DetrendXYZ()");
    return;
  }
  
  /* apply neighborhood filter */
  X = ImageStack[ImageStackTop-2].pixels;
  Y = ImageStack[ImageStackTop-1].pixels;

  /* loop to set up normal equations */
  draw_mesg("Detrending...");
  SumX=0; SumX2 = 0; SumY=0; SumY2=0; SumZ=0; SumXY=0; SumXZ=0; SumYZ=0;
  for (j = 0, samples = 0, n = 0; j < height; j++) {
    for (i = 0; i < width; i++, n++) {
      /* update running sums */
      if (MaskPixels[n] > 0.0) {
        SumX  += (X[n]);
        SumX2 += (X[n] * X[n]);
        SumY  += (Y[n]);
        SumY2 += (Y[n] * Y[n]);
        SumXY += (X[n] * Y[n]);
        SumXZ += (X[n] * inpixels[n]);
        SumYZ += (Y[n] * inpixels[n]);
        SumZ  += (inpixels[n]);
        samples++;
      }
    }
  }

  /* compute plane model coefficients */
  A[0] = SumX2;
  A[1] = SumXY;
  A[2] = SumX;
  A[3] = SumXY;
  A[4] = SumY2;
  A[5] = SumY;
  A[6] = SumX;
  A[7] = SumY;
  A[8] = samples;
  B[0] = SumXZ;
  B[1] = SumYZ;
  B[2] = SumZ;
  N = 3;
  IFAIL = 0;
  f04atf(A, &N, B, &N, C, AA, &N, WKS1, WKS2, &IFAIL);
  
  /* loop to detrend */
  for (j = 0, n = 0, m0 = 0, m1 = 0; j < height; j++) {
    for (i = 0; i < width; i++, n++) {
      if (MaskPixels[n] > 0.0) {
        e = inpixels[n] - (C[0]*X[n] + C[1]*Y[n] + C[2]);
        m0 += (e);
        m1 += (e * e);
        outpixels[n] = e;
      }
      else {
        outpixels[n] = 0;
      }
    }
  }
  printf("DetrendXYZ() model: \"%.2eX + %.2eY + %.2e\", <e>= %.3e, erms = %.3e\n",
	C[0], C[1], C[2], m0/((double) samples),
	sqrt((m1 - m0*m0)/ ((double) samples)));

  /* pop images off the stack */
  ImageStackTop--;
  ImageStackTop--;
}
    
/***************************** median filter *******************************/
/*                                                                         */
/*  function: replace central pixel of an kern_w * kern_w neighborhood     */
/*            by the median value in the neighborhood.                     */
/*                                                                         */
/***************************************************************************/
static void Swap(double *HeapS, double *HeapL, double *ZapVals,
		 double *inpixels, int kern_w, int inc,
		 int old_arg, int new_arg)
{
  int i, j, k, DataLen;

  /* init vars */
  DataLen = kern_w * kern_w;

  /* remove oldest row or column */
  for (k = 0; k < kern_w; k++)
    ZapVals[k] = inpixels[old_arg + (k*inc)];
  HeapSort(ZapVals, kern_w);
  for (i = 0, j = 0, k = 0; k < kern_w; k++, i++)
    for (; HeapL[i] != ZapVals[k]; i++, j++)
      HeapS[j] = HeapL[i];
  for (; i < DataLen; i++, j++)
    HeapS[j] = HeapL[i];
  
  /* add new row or column */
  for (k = 0; k < kern_w; k++)
    ZapVals[k] = inpixels[new_arg + (k*inc)];
  HeapSort(ZapVals, kern_w);
  for (i=0, j=0, k=0;
       (k < kern_w) && (j < (DataLen-kern_w));
       HeapL[i++] = ZapVals[k++]) {
    for (; (ZapVals[k] > HeapS[j]) && (j < (DataLen-kern_w));)
      HeapL[i++] = HeapS[j++];
  }
  for (; k < kern_w;)
    HeapL[i++] = ZapVals[k++];
  for (; j < (DataLen-kern_w);)
    HeapL[i++] = HeapS[j++];
}

#define UP   0
#define DOWN 1  
void MedianFilter(double *inpixels, double *outpixels, int height, int width,
		  void *ParmPtrs[])
{
  double *HeapS, *HeapL, *ZapVals;
  int i, rows, cols, DataLen, median_offset, old_row_inc, new_row_inc,
  row_cnt_inc, direction, kern_w;
  
  /* load parameters */
  kern_w = *((int *) ParmPtrs[0]);

  /* check input parameters */
  if ((kern_w < 2) || (kern_w > width)) {
    Abort = B_TRUE;
    draw_mesg("Abort : Invalid parms in MedianFilter()");
    return;
  }

  /* simple bubble sort if kernel <= 3 */
  if (kern_w <= 3) {
    double *buffer;
    int i, j, k, l, m, n, o, swap, kern_len, kern_med;
    double temp;

    /* calc internal parms */
    kern_w = 3;
    kern_len = kern_w * kern_w;
    kern_med = kern_len/2 + 1;

    /* allocate storage */
    if ((buffer = (double *) XvgMM_Alloc((void *) NULL,
					 kern_len*sizeof(double)))
	== NULL) {
      Abort = B_TRUE;
      sprintf(cbuf, "Abort: can't alloc buffer (%d bytes) in MedianFilter()",
	      kern_len*sizeof(double));
      draw_mesg(cbuf);
      return;
    }

    /* loop to sort */
    for (j = (kern_w/2 * width); j < (height - kern_w/2)*width; j+=width) {
      for (i = kern_w/2; i < (width - kern_w/2); i++) {
	/* fill sorting buffer */
	for (m=0, k = j-(kern_w/2*width); k <= j+(kern_w/2*width); k+=width) {
	  for (l = i - kern_w/2; l <= i + kern_w/2; l++, m++)
	    buffer[m] = inpixels[l+k];
	}
	/* bubble sort */
	for (swap = kern_len - 1; swap > 0;) {
	  n = swap;
	  for (o = 0, swap = 0; o < n; o++) {
	    if (buffer[o] > buffer[o+1]) {
	      temp = buffer[o];
	      buffer[o] = buffer[o+1];
	      buffer[o+1] = temp;
	      swap = o;
	    }
	  }
	}
	/* replace original pixel value with median value */
	outpixels[i+j] = buffer[kern_med];
      }
    }
    
    /* free storage */
    XvgMM_Free(buffer);
  }
	
  /* alternate sorting algorythm, more efficient for larger kernels */
  else {
    /* BUGGY on SGI !!!! */
    Abort = B_TRUE;
    sprintf(cbuf, "Abort: MedianFilter() buggy for kernels > 3!");
    draw_mesg(cbuf);
    return;
    
#ifdef NOTHIS
    /* init variables and allocate required storage */
    median_offset = (kern_w >> 1) * (width + 1);
    DataLen = kern_w * kern_w;
    if ((HeapS = (double *) XvgMM_Alloc((void *) NULL,
					DataLen*sizeof(double)))
	== NULL) {
      Abort = B_TRUE;
      sprintf(cbuf, "Abort: can't alloc HeapS %d bytes in MedianFilter()",
	      DataLen*sizeof(double));
      draw_mesg(cbuf);
      return;
    }
    if ((HeapL = (double *) XvgMM_Alloc((void *) NULL,
					DataLen*sizeof(double)))
	== NULL) {
      Abort = B_TRUE;
      sprintf(cbuf, "Abort: can't alloc HeapL %d bytes in MedianFilter()",
	      DataLen*sizeof(double));
      draw_mesg(cbuf);
      return;
    }
    if ((ZapVals = (double *) XvgMM_Alloc((void *) NULL,
					  kern_w*sizeof(double)))
	== NULL) {
      Abort = B_TRUE;
      sprintf(cbuf, "Abort: can't alloc ZapVals %d bytes in MedianFilter()",
	      kern_w*sizeof(double));
      draw_mesg(cbuf);
      return;
    }
    
    /* generate the first kernel and do an initial sort */
    for (rows = 0; rows < kern_w; rows++)
      for (cols = 0; cols < kern_w; cols++)
	HeapL[rows*kern_w + cols] = inpixels[rows*width + cols];
    HeapSort(HeapL, DataLen);
    
    /* store first median in outpixels */
    outpixels[median_offset] = HeapL[DataLen/2];
    
    /* loop for columns */
    for (cols = 0, direction = DOWN;
	 (cols <= (width - kern_w)) && (Abort == B_FALSE);) {
      /* set looping parms according to direction of scan (up/down) */
      if (direction == DOWN) {
	rows = 0;
	old_row_inc = 0;
	new_row_inc = kern_w*width;
	row_cnt_inc = width;
	direction = UP;
      }
      else {
	rows = (height - kern_w)*width;
	old_row_inc = (kern_w-1)*width;
	new_row_inc = -width;
	row_cnt_inc = -width;
	direction = DOWN;
      }
      
      /* loop for rows */
      for (i = 0; (i < (height - kern_w)) && (Abort == B_FALSE); i++) {
	/* remove oldest row, add new one */
	Swap(HeapS, HeapL, ZapVals, inpixels,
	     kern_w, 1, cols+rows+old_row_inc, cols+rows+new_row_inc);
	/* increment row counter */
	rows += row_cnt_inc;
	/* find median and store it in outpixels */
	outpixels[cols + rows + median_offset] = HeapL[DataLen/2];
      }
      
      /* remove oldest column, add new one */
      Swap(HeapS, HeapL, ZapVals, inpixels,
	   kern_w, width, cols+rows, cols+rows+kern_w);
      /* store median in outpixels */
      outpixels[(++cols) + rows + median_offset] = HeapL[DataLen/2];
      
      /* print out the column number */
      if (IPOpExecVerbose) {
	sprintf(cbuf, "Median - col: %d/%d", cols + kern_w/2, width);
	draw_mesg(cbuf);
      }
    }
    
    /* free storage */
    XvgMM_Free(HeapS);
    XvgMM_Free(HeapL);
    XvgMM_Free(ZapVals);
#endif
  }

  /* fill edges with raw data */
  if (Abort == B_FALSE)
    FillEdges(inpixels, outpixels, height, width, kern_w);
}


/************************ median filter (byte) *****************************/
/*                                                                         */
/*  function: replace central pixel of an kern_w * kern_w neighborhood     */
/*            by the median value in the neighborhood.                     */
/*            Special high efficiency version when dynamic range           */
/*            is converted to 0..255.                                      */
/*                                                                         */
/***************************************************************************/
void MedianFilterByte(double *inpixels, double *outpixels,
		      int height, int width, void *ParmPtrs[])
{
  int kern_w, HalfKernW, i, nvals, *tparms[1];
  double min, max, scale;

  /* load parameters */
  kern_w = *((int *) ParmPtrs[0]);

  /* check input parameters */
  if ((kern_w < 2) || (kern_w > width)) {
    Abort = B_TRUE;
    draw_mesg("Abort : Invalid parms in MedianFilterByte()");
    return;
  }

  /* draw message */
  draw_mesg("Median filtering...");

  /* find data extrema and map to 0..255 */
  for (i = 0, max = -MAXDOUBLE, min = MAXDOUBLE; i < height*width; i++) {
    min = (inpixels[i] < min ? inpixels[i] : min);
    max = (inpixels[i] > max ? inpixels[i] : max);
  }
  for (i = 0, scale = 255.0/(max - min); i < height*width; i++)
    DoubleBuffer[i] = (inpixels[i] - min)*scale;

  /* loop to filter */
  if (kern_w < 4) {
    double buffer[625];
    int i, j, k, l, m, n, o, swap, kern_len, kern_med;
    double *p, *q, *b, temp;

    kern_len = kern_w * kern_w;
    kern_med = kern_len/2 + 1;
    p = DoubleBuffer;
    q = outpixels;
    b = buffer;

    for (j = (kern_w/2 * width); j < (height - kern_w/2)*width; j += width) {
      for (i= kern_w/2; i < (width - kern_w/2); i++) {
	for (m =0, k =j-(kern_w/2*width); k <= j+(kern_w/2*width); k+=width) {
	  for (l = i - kern_w/2; l <= i + kern_w/2; l++)
	    b[m++] = p[l+k];
	}
	/* bubble sort */
	for (swap = kern_len - 1; swap > 0;)  
	  for (o = 0, n = swap, swap = 0; o < n; o++) {
	    if (b[o] > b[o+1]) {
	      temp = b[o];
	      b[o] = b[o+1];
	      b[o+1] = temp;
	      swap = o;
	    }
	  }
	/* replace original pixel value with median value */
	q[i+j] = b[kern_med];
      }
    }
  }
	
  /* alternate sorting algorythm, more efficient for larger kernels */
  else {
    int i, j, k, tmp, median_offset;
    double *p;
    int buckets[256];

    /* set vars */
    median_offset = (kern_w >> 1) * (width + 1);

    /* null bucket contents */
    for (i = 0; i < 256; i++)
      buckets[i] = 0;

    /* fill buckets with kernel area around first pixel in the fist line */
    nvals = 0;
    for (i = 0; i < kern_w; i++)
      for (j = 0, p = &DoubleBuffer[i * width]; j < kern_w; j++, p++)
	if (finite(*p)) {
	  buckets[((int) *p) & 0xff] += 1;
	  nvals++;
	}
    
    /* loop for columns */
    for (i = 0; (i < (width - kern_w)) && (Abort == B_FALSE);) {
      /* prime run: find first median and store it in outpixels */
      for (k = 0, tmp = 0; tmp < (nvals >> 1); k++)
	tmp += buckets[k];
      outpixels[i + median_offset] = k - 1;
#ifdef DEBUG
      if (k > 255) {
	printf("Median filter (byte) - K>255! (0) k=%d, i=%d, j=%d\n",
	       k, i, j);
	return;
      }
#endif

      /* loop for rows, for top to bottom */
      for (j = 0; (j < (height - kern_w)*width) && (Abort == B_FALSE);) {
	/* subtract oldest row */
	for (k = 0, p = &DoubleBuffer[i+j]; k < kern_w; k++, p++)
	  if (finite(*p)) {
	    buckets[((int) *p) & 0xff] -= 1;
	    nvals--;
	  }
	/* add new row */
	for (k = 0, p = &DoubleBuffer[i+j+(kern_w*width)];k < kern_w; k++, p++)
	  if (finite(*p)) {
	    buckets[((int) *p) & 0xff] += 1;
	    nvals++;
	  }
	/* increment row counter */
	j +=width;
	/* scan buckets to find median index and store it in outpixels */
	for (k = 0, tmp = 0; tmp < (nvals >> 1); k++)
	  tmp += buckets[k];
	outpixels[i + j + median_offset] = k - 1;
#ifdef DEBUG
	if (k > 255) {
	  printf("Median filter (byte) - K>255! (1) k=%d, i=%d, j=%d\n",
		 k, i, j);
	  return;
	}
#endif
      }

      /* subtract oldest column */
      for (k = 0, p = &DoubleBuffer[i+j]; k < kern_w; k++)
	if (finite(p[k*width])) {
	  buckets[((int) p[k*width]) & 0xff] -= 1;
	  nvals--;
	}
      /* add new column */
      for (k = 0, p = &DoubleBuffer[i+j+kern_w]; k < kern_w; k++)
	if (finite(p[k*width])) {
	  buckets[((int) p[k*width]) & 0xff] += 1;
	  nvals++;
	}
      /* increment column counter */
      i += 1;
      /* prime run: find first median and store it in outpixels */
      for (k = 0, tmp = 0; tmp < (nvals >> 1); k++)
	tmp += buckets[k];
      outpixels[i + j + median_offset] = k - 1;
#ifdef DEBUG
      if (k > 255) {
	printf("Median filter (byte) - K>255! (2) k=%d, i=%d, j=%d\n",
	       k, i, j);
	return;
      }
#endif

      /* loop for rows, for bottom to top (check for odd width images) */
      if (i <= (width - kern_w)) {
	for (j = (height - kern_w)*width; (j > 0) && (Abort == B_FALSE);) {
	  /* decrement row counter */
	  j -=width;
	  /* subtract oldest row */
	  for (k = 0, p = &DoubleBuffer[i+j+(kern_w*width)];k < kern_w;
	       k++, p++)
	    if (finite(*p)) {
	      buckets[((int) *p) & 0xff] -= 1;
	      nvals--;
	    }
	  /* add new row */
	  for (k = 0, p = &DoubleBuffer[i+j]; k < kern_w; k++, p++)
	    if (finite(*p)) {
	      buckets[((int) *p) & 0xff] += 1;
	      nvals++;
	    }
	  /* scan buckets to find median index and store it in outpixels */
	  for (k = 0, tmp = 0; tmp < (nvals >> 1); k++)
	    tmp += buckets[k];
	  outpixels[i + j + median_offset] = k - 1;
#ifdef DEBUG
	  if (k > 255) {
	    printf("Median filter (byte) - K>255! (3) k=%d, i=%d, j=%d\n",
		   k, i, j);
	    return;
	  }
#endif
	}
	/* subtract oldest column */
	for (k = 0, p = &DoubleBuffer[i+j]; k < kern_w; k++)
	  if (finite(p[k*width])) {
	    buckets[((int) p[k*width]) & 0xff] -= 1;
	    nvals--;
	  }
	/* add new column */
	for (k = 0, p = &DoubleBuffer[i+j+kern_w]; k < kern_w; k++)
	  if (finite(p[k*width])) {
	    buckets[((int) p[k*width]) & 0xff] += 1;
	    nvals++;
	  }
	/* increment column counter */
	i += 1;
      }
    }
  }

  /* remap to original dynamic range */
  for (i = 0, scale = (max - min)/255.0; i < height*width; i++)
    outpixels[i] = outpixels[i]*scale + min;

  /* null out edges */
/*
  HalfKernW = kern_w/2  + 1;
  tparms[0] = (void *) &HalfKernW;
  ZeroEdges(outpixels, outpixels, height, width, (void **) tparms);
*/

  /* fill edges with raw data */
  if (Abort == B_FALSE)
    FillEdges(inpixels, outpixels, height, width, kern_w);
}

/*****************************  Laplacian ***********************************/
/*                                                                         */
/* function: compute Laplacian                                              */
/*                                                                         */
/***************************************************************************/
void Laplacian(double *inpixels, double *outpixels, int height, int width,
	       void *ParmPtrs[])
{
  int i, j, k;
  
  /* Laplacian */
  draw_mesg("Computing Laplacian...");
  for (j = 0, k = 0; j < height; j++)
    for (i = 0; i < width; i++, k++) {
      if ((i > 0) && (i < width-1) && (j > 0) && (j < height-1)) {
        outpixels[k] =
	   inpixels[(j - 1)*width + i - 1] * -1.0 +
	   inpixels[(j - 1)*width + i] * -1.0 +
	   inpixels[(j - 1)*width + i + 1] * -1.0 +
	   inpixels[(j + 1)*width + i - 1] * -1.0 +
	   inpixels[(j + 1)*width + i] * -1.0 +
	   inpixels[(j + 1)*width + i + 1] * -1.0 +
	   inpixels[j*width + i - 1] * -1.0 +
	   inpixels[j*width + i]*8.0 +
	   inpixels[j*width + i + 1] * -1.0;
       }
       else {
         outpixels[k] = 0.0;
       }
    }
}

/*****************************  Gradient ***********************************/
/*                                                                         */
/* function: compute gradient                                              */
/*                                                                         */
/***************************************************************************/
void Gradient(double *inpixels, double *outpixels, int height, int width,
	      void *ParmPtrs[])
{
  static double Gx_multipliers[9] = {-0.125, 0.0, 0.125,
				       -0.25, 0.0, 0.25,
				       -0.125, 0.0, 0.125};
  static double Gy_multipliers[9] = {-0.125, -0.25, -0.125,
				       0.0, 0.0, 0.0,
				       0.125, 0.25, 0.125};
  double *p, *q, x, y;
  int i, j;
  
  /* draw message */
  draw_mesg("Gradient...");

  for (p = inpixels, q = outpixels, j = width;
       j < (height - 1)*width; j += width)
    for (i = 1; i < (width - 1); i++) {
      x = (inpixels[i-1+j-width]*Gx_multipliers[0]
	   + inpixels[i+1+j-width]*Gx_multipliers[2]
	   + inpixels[i-1+j]*Gx_multipliers[3]
	   + inpixels[i+1+j]*Gx_multipliers[5]
	   + inpixels[i-1+j+width]*Gx_multipliers[6]
	   + inpixels[i+1+j+width]*Gx_multipliers[8]);
      y = (inpixels[i-1+j-width]*Gy_multipliers[0]
	   + inpixels[i+j-width]*Gy_multipliers[1]
	   + inpixels[i+1+j-width]*Gy_multipliers[2]
	   + inpixels[i-1+j+width]*Gy_multipliers[6]
	   + inpixels[i+j+width]*Gy_multipliers[7]
	   + inpixels[i+1+j+width]*Gy_multipliers[8]);
      outpixels[i+j] = sqrt(x*x + y*y);
    }
}

/*********************** Region averaging filter ***************************/
/*                                                                         */
/*  function: replace central pixel of an kern_w * kern_w neighborhood     */
/*            by the average value in the neighborhood, computed           */
/*            using a kern_w * kern_w size mask, within the countour.      */
/*                                                                         */
/***************************************************************************/
void RegionAveragingFilter(double *inpixels, double *outpixels,
			   int height, int width, void *ParmPtrs[])
{
  double divisor_inv, *p, *q, m;
  int i, j, k, l, kern_w;
  double buf[625];

  /* load parameters */
  kern_w = *((int *) ParmPtrs[0]);

  /* check input parameters */
  if ((kern_w < 2) || (kern_w > width)) {
    Abort = B_TRUE;
    draw_mesg("Abort : Invalid kernel size in RegionAveragingFilter()");
    return;
  }

  /* make sure contour is defined */
  if (DisplayContourComputed == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort : Undefined contour in RegionAveragingFilter()");
    return;
  }

  /* draw message */
  draw_mesg("Region averaging filter...");

  /* transfer data to output buffer */
  for (i = 0; i < height*width; i++)
    outpixels[i] = inpixels[i];

  /* apply neighborhood averaging filter */
  for (j = (kern_w/2 * width), p = inpixels, q = outpixels,
       divisor_inv = 1.0 / ((double) (kern_w * kern_w));
       (j < (height - kern_w/2)*width) && (Abort == B_FALSE); j += width) {
    for (i= kern_w/2; (i < (width - kern_w/2)) && (Abort == B_FALSE); i++)
      if (MaskPixels[i+j] != 0) {
	for (m=0, k = j-(kern_w/2*width); k <= j+(kern_w/2*width); k+=width) {
	  for (l = i - kern_w/2; l <= i + kern_w/2; l++)
	    m += p[l+k];
	}
	q[i+j] = m * divisor_inv;
      }
  }
}
    
/*************************** WeightedAveragingFilter ***********************/
/*                                                                         */
/*  function: replace central pixel of an kern_w * kern_w neighborhood     */
/*            by the weighted average of its neighbours.                   */
/*            The stack image values are used as weights.                  */
/*                                                                         */
/***************************************************************************/
void WeightedAveragingFilter(double *inpixels, double *outpixels,
			     int height, int width, void *ParmPtrs[])
{
  int i, j, k, l, kern_w, HalfKernW;
  double x, cnt, *w, *tparms[1];

  /* load parameters */
  kern_w = *((int *) ParmPtrs[0]);

  /* check input parameters */
  if ((kern_w < 2) || (kern_w > width)) {
    Abort = B_TRUE;
    draw_mesg("Abort : Invalid parms in WeightedAveragingFilter()");
    return;
  }

  /* check that stack is not empty */
  if (ImageStackTop <= 0) {
    Abort = B_TRUE;
    draw_mesg("Abort : One stack image required in WeightedAveragingFilter()");
    return;
  }
  
  /* apply neighborhood filter */
  w = ImageStack[ImageStackTop-1].pixels;
  for (j = (kern_w/2 * width); j < (height - kern_w/2)*width; j += width) {
    for (i= kern_w/2; i < (width - kern_w/2); i++) {
      for (k = j-(kern_w/2*width), cnt=0, x=0; k <= j+(kern_w/2*width);
	   k+=width) {
	for (l = i - kern_w/2; l <= i + kern_w/2; l++) {
	  x += (w[l+k] * inpixels[l+k]);
	  cnt += w[l+k];
	}
      }
      if (cnt > 0.0)
	outpixels[i+j] = x / cnt;
      else
	outpixels[i+j] = 0;
    }
  }

  /* pop the stack */
  ImageStackTop--;
  UpdateStackLabel(ImageStackTop);
}

/****************************** zero edges   *******************************/
/*                                                                         */
/*  function: zero in edges of an image                                    */
/*                                                                         */
/***************************************************************************/
void ZeroEdges(double *inpixels, double *outpixels, int height, int width,
	       void *parms[])
{
  int i, j, len;
  double *q;

  /* fetch kernel size */
  len = *((int *) parms[0]);

  /* draw message */
  draw_mesg("Zeroing edges...");

  /* transfer data to output buffer */
  for (i = 0; i < height*width; i++)
    outpixels[i] = inpixels[i];

  /* left edge */
  for (j = (len * width); j < (height - len)*width; j += width)
    for (i = 0; i < len; i++)
      outpixels[j+i] = 0;
  /* right edge */
  for (j = (len * width); j < (height - len)*width; j += width)
    for (i = (width - len); i < width; i++)
      outpixels[j+i] = 0;
  /* top edge */
  for (i = 0; i < (width*len); i++)
    outpixels[i] = 0;
  /* bottom edge */
  for (i = 0, q = &outpixels[(height-len-1)*width]; i < (width*len); i++)
      *q++ = 0;
}

