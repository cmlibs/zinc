/***********************************************************************
*
*  Name:          fileIPOPs.c
*
*  Author:        Paul Charette
*
*  Last Modified: 12 August 1993
*
*  Purpose:       Utility routines for loading/saving
*                     LoadSinImg()
*                     LoadArctanErrorImg()
*                     LoadArctanRadiusImg()
*                     LoadCosImg()
*                     LoadMask()
*                     LoadMaskFile()
*                     LoadSinImg()
*                     Rotate()
*                     SaveBSpline()
*                     SaveDataSet()
*                     SaveDataSetByte()
*                     SaveDataSetShort()
*                     SaveDataSetReal()
*                     SaveImageFile()
*                     SaveTIFFColorFile()
*                     SaveTIFFGreyLevelFile()
*                     Transpose()
*
***********************************************************************/
#include <stdio.h>
#include "UxXt.h"
#include <fcntl.h>
#include <search.h>
#include <values.h>
#include "XvgGlobals.h"

/**********************   LoadArctanErrorImg    ****************************/
/*                                                                         */
/*  function: load the arctan error data into the window                   */
/*                                                                         */
/***************************************************************************/
void LoadArctanErrorImg(double *inpixels, double *outpixels,
			int height, int width, void *ParmPtrs[])
{
  int i;
  
  /* push current image onto the stack */
  if (XvgImgStack_Push(inpixels, height, width) == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort : Failed pushing current image in LoadArctanErrorImg()");
    return;
  }

  /* load the arctan error data into the current buffer */
  for (i = 0; i < height*width; i++)
    outpixels[i] = ErrorImgData[i];
}

/**********************   LoadArctanRadiusImg   ****************************/
/*                                                                         */
/*  function: load the arctan radius data into the window                  */
/*                                                                         */
/***************************************************************************/
void LoadArctanRadiusImg(double *inpixels, double *outpixels,
			 int height, int width, void *ParmPtrs[])
{
  int i;
  
  /* push current image onto the stack */
  if (XvgImgStack_Push(inpixels, height, width) == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort : Failed pushing current image in LoadArctanRadiusImg()");
    return;
  }

  /* load the arctan radius data into the current buffer */
  for (i = 0; i < height*width; i++)
    outpixels[i] = RadiusImgData[i];
}

/*******************************   LoadCosImg   ****************************/
/*                                                                         */
/*  function: load the 2Icos data into the window                          */
/*                                                                         */
/***************************************************************************/
void LoadCosImg(double *inpixels, double *outpixels,
		int height, int width, void *ParmPtrs[])
{
  int i;
  
  /* push current image onto the stack */
  if (XvgImgStack_Push(inpixels, height, width) == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort : Failed pushing current image in LoadCosImg()");
    return;
  }

  /* load the cos data into the current buffer */
  for (i = 0; i < height*width; i++)
    outpixels[i] = CosImgData[i];
}

/**********************      LoadMask     **********************************/
/*                                                                         */
/*  function: load the mask image                                          */
/*                                                                         */
/***************************************************************************/
void LoadMask(double *inpixels, double *outpixels,
	      int height, int width, void *ParmPtrs[])
{
  int i;

  /* push current image onto the stack */
  if (XvgImgStack_Push(inpixels, height, width) == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort : Failed pushing current image in LoadMask()");
    return;
  }

  /* load the mask data into the current buffer */
  for (i = 0; i < height*width; i++)
    outpixels[i] = MaskPixels[i];
}

/**********************     LoadImageFile    *******************************/
/*                                                                         */
/*  function: load file and push the old one                               */
/*                                                                         */
/***************************************************************************/
void LoadImageFile(double *inpixels, double *outpixels,
		   int height, int width, void *ParmPtrs[])
{
  static double *lpixels;
  double lmin, lmax;
  boolean_t Compressed;
  int i, j, datatype, lheight, lwidth;
  char InFileNameExt[128], InFileNameStripped[128], vname[512], buffer[512];
  char fname[512];

  /* set flags */
  GrabbingOn = B_FALSE;

  /* load parms */
  sprintf(fname, "%s", (char *) (ParmPtrs[0]));

  /* parse file name */
  for (j = (strlen(fname) - 1); (j >= 0) && (fname[j] != '/'); j--);
  for (i = 0, j = (j == 0 ? 0 : j+1); fname[j] != '\0'; i++, j++)
    InFileNameStripped[i] = fname[j];
  InFileNameStripped[i] = '\0';
  for (j = (strlen(fname) - 1); (j >= 0) && (fname[j] != '.'); j--);
  for (i = 0; fname[j] != '\0'; i++, j++)
    InFileNameExt[i] = fname[j];
  InFileNameExt[i] = '\0';

  /* check for compressed file */
  if (strcmp(".Z", InFileNameExt) == 0) {
    draw_mesg("LoadImageFile() : Decompressing file...");
    sprintf(buffer, "uncompress %s", fname);
    system(buffer);
    fname[j-2] = '\0';
    /* get new file extension */
    for (j = (strlen(fname) - 1); (j >= 0) && (fname[j] != '.'); j--);
    for (i = 0; fname[j] != '\0'; i++, j++)
      InFileNameExt[i] = fname[j];
    InFileNameExt[i] = '\0';
    Compressed = B_TRUE;
  }
  else if (strcmp(".gz", InFileNameExt) == 0) {
    draw_mesg("LoadImageFile() : Decompressing file...");
    sprintf(buffer, "gunzip %s", fname);
    system(buffer);
    fname[j-3] = '\0';
    /* get new file extension */
    for (j = (strlen(fname) - 1); (j >= 0) && (fname[j] != '.'); j--);
    for (i = 0; fname[j] != '\0'; i++, j++)
      InFileNameExt[i] = fname[j];
    InFileNameExt[i] = '\0';
    Compressed = B_TRUE;
  }
  else {
    Compressed = B_FALSE;
  }
  
  /* TIFF file */
  sprintf(cbuf, "LoadImageFile() : Loading file %s...", fname);
  draw_mesg(cbuf);
  if ((strcmp(".tiff", InFileNameExt) == 0) || 
      (strcmp(".tif", InFileNameExt) == 0)) {
    if (GetTiffFile(fname, &lpixels, &lheight, &lwidth, &lmin, &lmax,
		    cmap, UserCells, &TrueColorImage, VerboseFlag)
	== B_FALSE) {
      Abort = B_TRUE;
      sprintf(buffer, "Error reading file %s in LoadImageFile()", fname);
      draw_mesg(buffer);
      return;
    }
  }
  
  /* MATLAB file */
  else if (strcmp(".mat", InFileNameExt) == 0) {
    if (GetMatlabFile(&lheight, &lwidth, &lmin, &lmax,
		      &lpixels, fname, VerboseFlag) == B_FALSE) {
      Abort = B_TRUE;
      sprintf(buffer, "Error reading file %s in LoadImageFile()", fname);
      draw_mesg(buffer);
      return;
    }
  }

  /* raw stepping MATLAB file */
  else if (strcmp(".smat", InFileNameExt) == 0) {
    if (DeCompReadMatlabImages(&lheight, &lwidth, &lmin, &lmax,
			       &lpixels, fname, VerboseFlag) == B_FALSE) {
      Abort = B_TRUE;
      sprintf(buffer, "Error reading file %s in LoadImageFile()", fname);
      draw_mesg(buffer);
      return;
    }
  }

  /* b-spline file */
  else if (strcmp(".bspl", InFileNameExt) == 0) {
    if (GetBSplineFile(fname, &lheight, &lwidth, &lmin, &lmax,
		       &lpixels, VerboseFlag) == B_FALSE) {
      Abort = B_TRUE;
      sprintf(buffer, "Error reading file %s in LoadImageFile()", fname);
      draw_mesg(buffer);
      return;
    }
  }

  /* SGI rgb file */
  else if (strcmp(".rgb", InFileNameExt) == 0) {
    if (GetSGIRGBFile(fname, &lpixels, &lheight, &lwidth,
		      &lmin, &lmax) == B_FALSE) {
      Abort = B_TRUE;
      sprintf(buffer, "Error reading file %s in LoadImageFile()", fname);
      draw_mesg(buffer);
      return;
    }
  }
  
  /* SGI rgb file */
  else if (strcmp(".sgi", InFileNameExt) == 0) {
    if (GetFieldFromSGIRGBFile(fname, 1, &lpixels, &lheight, &lwidth,
		               &lmin, &lmax) == B_FALSE) {
      Abort = B_TRUE;
      sprintf(buffer, "Error reading file %s in LoadImageFile()", fname);
      draw_mesg(buffer);
      return;
    }
  }
  
  /* unsupported file type */
  else {
    Abort = B_TRUE;
    sprintf(buffer, "%s has unsupported extension type in LoadImageFile()",
	    fname);
    draw_mesg(buffer);
    return;
  }

  /* check that new image is the correct current size */
  if (((lheight != height) || (lwidth != width)) && (Abort == B_FALSE)) {
    Abort = B_TRUE;
    draw_mesg("Abort : Mismatched image dimensions in LoadImageFile()");
    return;
  }

  /* push current image onto the stack */
  if (XvgImgStack_Push(inpixels, height, width) == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort : Failed pushing current image in LoadImageFile()");
    return;
  }

  /* copy the new image data to the output buffer */
  for (i = 0; i < height*width; i++)
    outpixels[i] = lpixels[i];
    
  /* recompress file if required */
/*
  if (Compressed && Recompress) {
    draw_mesg("LoadImageFile() : Recompressing file...");
    sprintf(buffer, "compress %s", fname);
    system(buffer);
  }
*/
}

/**********************     LoadMaskFile    ********************************/
/*                                                                         */
/*  function: load image into the mask                                     */
/*                                                                         */
/***************************************************************************/
void LoadMaskFile(double *inpixels, double *outpixels,
		  int height, int width, void *ParmPtrs[])
{
  static CONTOUR_ELEMENT contour[CONTOURNMAX];
  static double *lpixels;
  int lheight=0, lwidth=0; 
  double lmin, lmax;
  int i, j, datatype;
  char InFileNameExt[128], InFileNameStripped[128], vname[512], buffer[512],*p;

  /* set flags */
  GrabbingOn = B_FALSE;

  /* load parms */
  p = (char *) (ParmPtrs[0]);

  /* parse file name */
  for (j = (strlen(p) - 1); (j >= 0) && (p[j] != '/'); j--);
  for (i = 0, j = (j == 0 ? 0 : j+1); p[j] != '\0'; i++, j++)
    InFileNameStripped[i] = p[j];
  InFileNameStripped[i] = '\0';
  for (j = (strlen(p) - 1); (j >= 0) && (p[j] != '.'); j--);
  for (i = 0; p[j] != '\0'; i++, j++)
    InFileNameExt[i] = p[j];
  InFileNameExt[i] = '\0';

  /* check file type */
  if ((strlen(InFileNameExt) < 3) || (strlen(InFileNameExt) > 5)) {
    sprintf(buffer,
	    "File \"%s\" does not have a \".px\", \".bspl\", \".tiff\" , \".xwd\" or \".mat\" extension!", p);
    Abort = B_TRUE;
    draw_mesg(buffer);
    return;
  }

  /* TIFF file */
  if (strcmp(".tiff", InFileNameExt) == 0) {
    if (GetTiffFile(p, &lpixels, &lheight, &lwidth, &lmin, &lmax,
		    cmap, UserCells, &TrueColorImage, VerboseFlag) == B_FALSE) {
      Abort = B_TRUE;
      sprintf(buffer, "Abort : Error reading file %s in LoadMaskFile()", p);
      draw_mesg(buffer);
      return;
    }
  }

  /* MATLAB file */
  else if (strcmp(".mat", InFileNameExt) == 0) {
    if (GetMatlabFile(&lheight, &lwidth, &lmin, &lmax,
		      &lpixels, p, VerboseFlag) == B_FALSE) {
      Abort = B_TRUE;
      sprintf(buffer, "Abort : Error reading file %s in LoadMaskFile()", p);
      draw_mesg(buffer);
#ifdef DEBUG
      printf("LoadMaskFile() : Returning with error\n");
#endif
      return;
    }
  }

  /* raw stepping MATLAB file */
  else if (strcmp(".smat", InFileNameExt) == 0) {
    if (DeCompReadMatlabImages(&lheight, &lwidth, &lmin, &lmax,
			       &lpixels, p, VerboseFlag) == B_FALSE) {
      Abort = B_TRUE;
      sprintf(buffer, "Abort : Error reading file %s in LoadMaskFile()", p);
      draw_mesg(buffer);
      return;
    }
  }

  /* b-spline file */
  else if (strcmp(".bspl", InFileNameExt) == 0) {
    if (GetBSplineFile(p, &lheight, &lwidth, &lmin, &lmax,
		       &lpixels, VerboseFlag) == B_FALSE) {
      Abort = B_TRUE;
      sprintf(buffer, "Abort : Error reading file %s in LoadMaskFile()", p);
      draw_mesg(buffer);
      return;
    }
  }

  /* unsupported file type */
  else {
    Abort = B_TRUE;
    sprintf(buffer, "Abort : File %s has unsupported type in LoadMaskFile()",
	    p);
    draw_mesg(buffer);
    return;
  }

  /* check that new image is the correct size */
  if ((lheight != height) || (lwidth != width)) {
    Abort = B_TRUE;
    draw_mesg("Abort : Incorrect image dimensions in LoadImageFile()");
    return;
  }
  else
    for (i = 0; i < height*width; i++)
      MaskPixels[i] = lpixels[i];
  
  /* Trace contour to order points in counter-clockwise order */
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
    draw_mesg("Abort : Can't allocate in LoadMaskFile()");
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

  /* transfer the data to the output buffer */
  for (i = 0; i < height*width; i++)
    outpixels[i] = inpixels[i];
}

/*******************************   LoadSinImg   ****************************/
/*                                                                         */
/*  function: load the 2Isin data into the window                          */
/*                                                                         */
/***************************************************************************/
void LoadSinImg(double *inpixels, double *outpixels,
		int height, int width, void *ParmPtrs[])
{
  int i;
  
  /* push current image onto the stack */
  if (XvgImgStack_Push(inpixels, height, width) == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort : Failed pushing current image in LoadSinImg()");
    return;
  }

  /* load the Sin data into the current buffer */
  for (i = 0; i < height*width; i++)
    outpixels[i] = SinImgData[i];
}

/*************************     Rotate     **********************************/
/*                                                                         */
/*  function: rotate the image                                             */
/*                                                                         */
/***************************************************************************/
void Rotate(double *pixels, int height, int width)
{
  int i, j, k;

  /* check that "DoubleBuffer" is defined */
  if ((DoubleBuffer
       = (double *) XvgMM_Alloc((void *) DoubleBuffer,
				height*width*sizeof(double)))
      == NULL) {
    sprintf(cbuf, "Transpose() : Could not allocate memory (%dx%d)",
	    width, height);
    ErrorMsg(cbuf);
    return;
  }
  
  /* transfer input data to buffer */
  for (k = 0; k < height*width; k++)
    DoubleBuffer[k] = pixels[k];
  
  /* transpose data */
  for (j = 0, k = 0; j < height; j++)
    for (i = 0; i < width; i++, k++)
      pixels[i*height + height - j - 1] = DoubleBuffer[j*width + i];
}

/**********************     SaveDataSet   **********************************/
/*                                                                         */
/*  function: save double data in MATLAB file                              */
/*                                                                         */
/***************************************************************************/
void SaveDataSet(double *inpixels, double *outpixels,
		 int height, int width, void *ParmPtrs[])
{
  int i;

  WriteMatlabFile(height, width, "xvgimage", inpixels, ParmPtrs[0]);

  for (i = 0; i < height*width; i++)
    outpixels[i] = inpixels[i];
}

/************************ SaveDataSetByte  *****************************/
/*                                                                         */
/*  function: write the image data as a compressed byte MATLAB file        */
/*                                                                         */
/***************************************************************************/
void SaveDataSetByte(double *inpixels, double *outpixels,
		     int height, int width, void *ParmPtrs[])
{
  Fmatrix x;
  MATFile *DataFp;
  Matrix *a;
  double max, min, dbuf, ScaleInv;
  int i, j, k;
  char *p;
  
  /* open the matlab output file */
  if ((DataFp=matOpen(ParmPtrs[0], "w")) == NULL) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort : Could not open file %s in SaveDataSetByte()\n",
	    ParmPtrs[0]);
    draw_mesg(cbuf);
    return;
  }

  /* compress data to byte form and transpose */
  DynamicRange(inpixels, height*width, &max, &min);
  for (i = 0, ScaleInv = 255.0/(max - min), p = (char *) DoubleBuffer;
       i < height*width; i++)
    *p++ = (inpixels[i] - min)*ScaleInv;
  
  /* write the header for the pixel data                                 */
  /* machine ID = 1, row-wise oriented matrix of unsigned char elements  */
  x.type = (1 * 1000) + (0 * 100) + (5 * 10) + (0);
  x.mrows = width;
  x.ncols = height;
  x.imagf = 0;
  x.namlen = strlen("xvgimage") + 1;
  fwrite(&x, sizeof(Fmatrix), 1, matGetFp(DataFp));
  fwrite("xvgimage", sizeof(char), (int)(x.namlen), matGetFp(DataFp));
  
  /* write image data to the output file */
  if (fwrite(DoubleBuffer, sizeof(char), width*height, matGetFp(DataFp))
      != width*height) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort : Could not write to file %s in SaveDataSetByte()\n",
	    ParmPtrs[0]);
    draw_mesg(cbuf);
    return;
  }

  /* write the scaling information to the output file */
  a = mxCreateFull(1, 1, REAL);
  dbuf = max;
  memcpy(mxGetPr(a), &dbuf, sizeof(double));
  mxSetName(a, "Max");
  matPutMatrix(DataFp, a);
  dbuf = min;
  memcpy(mxGetPr(a), &dbuf, sizeof(double));
  mxSetName(a, "Min");
  matPutMatrix(DataFp, a);
  dbuf = 255.0;
  memcpy(mxGetPr(a), &dbuf, sizeof(double));
  mxSetName(a, "DynamicRange");
  matPutMatrix(DataFp, a);
  mxFreeMatrix(a);

  /* close file */
  matClose(DataFp);

  /* copy the data to the output buffer */
  for (i = 0; i < width*height; i++)
      outpixels[i] = inpixels[i];
}

/************************ SaveDataSetShort *********************************/
/*                                                                         */
/*  function: write the image data as a compressed byte MATLAB file        */
/*                                                                         */
/***************************************************************************/
void SaveDataSetShort(double *inpixels, double *outpixels,
		      int height, int width, void *ParmPtrs[])
{
  Fmatrix x;
  MATFile *DataFp;
  Matrix *a;
  double max, min, dbuf, ScaleInv;
  int i, j, k;
  unsigned short *p; 
  
  /* open the matlab output file */
  if ((DataFp=matOpen(ParmPtrs[0], "w")) == NULL) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort : Could not open file %s in SaveDataSetShort()\n",
	    ParmPtrs[0]);
    draw_mesg(cbuf);
    return;
  }

  /* compress data to unsigned short form and transpose */
  DynamicRange(inpixels, height*width, &max, &min);
  for (i = 0, ScaleInv = 65535.0/(max - min), p = (unsigned short *) DoubleBuffer;
       i < height*width; i++)
    *p++ = (inpixels[i] - min)*ScaleInv;
  
  /* write the header for the pixel data                                 */
  /* machine ID = 1, row-wise oriented matrix of unsigned unsigned short elements  */
  x.type = (1 * 1000) + (0 * 100) + (4 * 10) + (0);
  x.mrows = width;
  x.ncols = height;
  x.imagf = 0;
  x.namlen = strlen("xvgimage") + 1;
  fwrite(&x, sizeof(Fmatrix), 1, matGetFp(DataFp));
  fwrite("xvgimage", sizeof(char), (int)(x.namlen), matGetFp(DataFp));
  
  /* write image data to the output file */
  if (fwrite(DoubleBuffer, sizeof(unsigned short), width*height, matGetFp(DataFp))
      != width*height) {
    Abort = B_TRUE;
    sprintf(cbuf, "Abort : Could not write to file %s in SaveDataSetShort()\n",
	    ParmPtrs[0]);
    draw_mesg(cbuf);
    return;
  }

  /* write the scaling information to the output file */
  a = mxCreateFull(1, 1, REAL);
  dbuf = max;
  memcpy(mxGetPr(a), &dbuf, sizeof(double));
  mxSetName(a, "Max");
  matPutMatrix(DataFp, a);
  dbuf = min;
  memcpy(mxGetPr(a), &dbuf, sizeof(double));
  mxSetName(a, "Min");
  matPutMatrix(DataFp, a);
  dbuf = 65535.0;
  memcpy(mxGetPr(a), &dbuf, sizeof(double));
  mxSetName(a, "DynamicRange");
  matPutMatrix(DataFp, a);
  mxFreeMatrix(a);

  /* close file */
  matClose(DataFp);

  /* copy the data to the output buffer */
  for (i = 0; i < width*height; i++)
      outpixels[i] = inpixels[i];
}

/**********************     SaveDataSetReal ********************************/
/*                                                                         */
/*  function: save double data in MATLAB file (real data only)             */
/*                                                                         */
/***************************************************************************/
void SaveDataSetReal(double *inpixels, double *outpixels,
		     int height, int width, void *ParmPtrs[])
{
  boolean_t tmp;
  int i;

  tmp = ComplexDataSet;
  ComplexDataSet = B_FALSE;
  WriteMatlabFile(height, width, "xvgimage", inpixels, ParmPtrs[0]);
  ComplexDataSet = tmp;

  for (i = 0; i < height*width; i++)
    outpixels[i] = inpixels[i];
}

/********************** SaveTIFFGreyLevelFile  *****************************/
/*                                                                         */
/*  function: save 8 bit pixel data in TIFF file                           */
/*                                                                         */
/***************************************************************************/
void SaveTIFFGreyLevelFile(double *inpixels, double *outpixels,
			   int height, int width, void *ParmPtrs[])
{
  int i;

  if (WriteTiffFileGreyLevel(inpixels, 0, 0, height, width,
			     ParmPtrs[0], NULL, B_FALSE) == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort: error on return from WriteTiffFileGreyLevel() in SaveTIFFGreyLevelFile()");
    return;
  }
  
  /* transfer the data to the output buffer */
  for (i = 0; i < height*width; i++)
    outpixels[i] = inpixels[i];
}


/********************** SaveTIFFGreyLevelFile  *****************************/
/*                                                                         */
/*  function: save 8 bit pixel data in TIFF file                           */
/*                                                                         */
/***************************************************************************/
void SaveTIFFGreyLevelFixedByteDRFile(double *inpixels, double *outpixels,
				      int height, int width, void *ParmPtrs[])
{
  int i;

  if (WriteTiffFileGreyLevel(inpixels, 0, 0, height, width,
			     ParmPtrs[0], NULL, B_TRUE) == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort: error on return from WriteTiffFileGreyLevel() in SaveTIFFGreyLevelFile()");
    return;
  }
  
  /* transfer the data to the output buffer */
  for (i = 0; i < height*width; i++)
    outpixels[i] = inpixels[i];
}


/**********************  SaveTIFFColorFile  ********************************/
/*                                                                         */
/*  function: save 24 bit color pixel data in TIFF file                    */
/*                                                                         */
/***************************************************************************/
void SaveTIFFColorFile(double *inpixels, double *outpixels,
		       int height, int width, void *ParmPtrs[])
{
  extern Widget ProcessedImageDrawingArea, ProcessedImageTBG;
  XImage *image;
  XColor cells[256];
  int i;

  /* check that processed image window is popped up */
  if (IsTrue(UxGetSet(ProcessedImageTBG)) == B_FALSE) {
    Abort = B_TRUE;
    draw_mesg("Abort: Processed image window must be popped up for saving");
    return;
  }

  /* make sure the image has been updated on the screen */
  ShowImage(inpixels);

  /* execute the overlay functions if any */
  for (i = 0; (i < NOverlayFunctions) && (OverlayLevel > 0); i++)
    (OverlayFunctions[i])(ProcessedImageDrawingArea);
  
  /* display colorbar */
  if (ColorBarOverlay)
    DrawColorBar(ProcessedImageDrawingArea, B_TRUE);
  
  /* fetch the current color cell definitions */
  for (i = 0; i < 256; i++)
    cells[i].pixel = i;
  XQueryColors(UxDisplay, cmap, cells, 256);

  /* fetch drawing area contents */	
  XSync(UxDisplay, B_FALSE);
  image = XGetImage(UxDisplay,XtWindow(ProcessedImageDrawingArea),
		    0, 0, ImgWidth*ZoomFactor, ImgHeight*ZoomFactor,
		    (unsigned long) 0xffffffff, ZPixmap);
  
  /* write the data */
  WriteTiffFileColor(image, ProcessedWinHOffset, ProcessedWinVOffset,
		     ProcessedWinHeight, ProcessedWinWidth,
		     ParmPtrs[0], NULL, cells, DecimationFactor);
  
  /* destroy the image */
  XDestroyImage(image);

  /* transfer the data to the output buffer */
  for (i = 0; i < height*width; i++)
    outpixels[i] = inpixels[i];
}


/**********************     SaveBSpline   **********************************/
/*                                                                         */
/*  function: save the BSpline in MATLAB file                              */
/*                                                                         */
/***************************************************************************/
void SaveBSpline(double *inpixels, double *outpixels,
		 int height, int width, void *ParmPtrs[])
{
  MATFile *DataFp;
  Matrix *a;
  int i;
  double d;
  
  /* save spline data in a MATLAB file */
  if ((DataFp=matOpen(ParmPtrs[0], "w")) == NULL) {
    sprintf(cbuf, "SaveBSpline() : Could not open output file %s",
	    ParmPtrs[0]);
    ErrorMsg(cbuf);
    return;
  }

  a = mxCreateFull((BSplineNKi-4), (BSplineNKj-4), REAL);
  memcpy(mxGetPr(a), BSplineCij,
	 (BSplineNKi-4) * (BSplineNKj-4) * sizeof(double));
  mxSetName(a, "Cij");
  matPutMatrix(DataFp, a);
  mxFreeMatrix(a);
  
  a = mxCreateFull(BSplineNKi, 1, REAL);
  memcpy(mxGetPr(a), BSplineKi, BSplineNKi * sizeof(double));
  mxSetName(a, "Ki");
  matPutMatrix(DataFp, a);
  mxFreeMatrix(a);
  
  a = mxCreateFull(BSplineNKj, 1, REAL);
  memcpy(mxGetPr(a), BSplineKj, BSplineNKj * sizeof(double));
  mxSetName(a, "Kj");
  matPutMatrix(DataFp, a);
  mxFreeMatrix(a);
  
  a = mxCreateFull(1, 1, REAL);
  d = width;
  memcpy(mxGetPr(a), &d, sizeof(double));
  mxSetName(a, "width");
  matPutMatrix(DataFp, a);
  d = height;
  memcpy(mxGetPr(a), &d, sizeof(double));
  mxSetName(a, "height");
  matPutMatrix(DataFp, a);
  mxFreeMatrix(a);
  
  matClose(DataFp);

  for (i = 0; i < height*width; i++)
    outpixels[i] = inpixels[i];
}

/**********************     Transpose     **********************************/
/*                                                                         */
/*  function: transpose the image                                          */
/*                                                                         */
/***************************************************************************/
void Transpose(double *pixels, int height, int width)
{
  int i, j, k;

  /* check that "DoubleBuffer" is defined */
  if ((DoubleBuffer
       = (double *) XvgMM_Alloc((void *) DoubleBuffer,
				height*width*sizeof(double)))
      == NULL) {
    sprintf(cbuf, "Transpose() : Could not allocate memory (%dx%d)",
	    width, height);
    ErrorMsg(cbuf);
    return;
  }
  
  /* transfer input data to buffer */
  for (k = 0; k < height*width; k++)
    DoubleBuffer[k] = pixels[k];
  
  /* transpose data */
  for (j = 0, k = 0; j < height; j++)
    for (i = 0; i < width; i++, k++)
      pixels[i*height + j] = DoubleBuffer[k];
}

