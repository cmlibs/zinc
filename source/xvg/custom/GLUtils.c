/***********************************************************************
*
*  Name: GLUtils.c
*
*  Author:        Paul Charette
*
*  Last Modified: 12 Oct 1993
*
*  Purpose:       Utility routines GL drawing :
*                   BuildMeshSurface()
*                   BuildNurbsObject()
*                   GLBuildNurbs()
*                   GLBuildPoly()
*                   GLDraw()
*                   GLInitLighting()
*                   GLMovieGenerate()
*                   GLMovieSeqGenerate()
*                   GLReadMovie()
*                   GLRecFrame()
*                   GLResetMovie()
*                   GLShowFrame()
*                   GLWriteMovie()
*                   GL_SGI_OpenWindow()
*                   LoadGLField()
*                   LoadGLGeometry()
*                   SaveGLFrame()
*                   WriteGLImageToVCAA()
*
***********************************************************************/
#define NOINTRINSICS
#include <stdio.h>
#include "UxXt.h"
#include "XvgGlobals.h"
#include "GLUtils.h"
#if defined (GL_API)
#include <gl/gl.h>
#include <gl/device.h>
#ifdef SGI
#include <gl/glws.h>
#endif
#endif /* defined (GL_API) */

/* defines */
#ifdef SGI
#define NGLVERTICIES 256
#else
#define NGLVERTICIES 128
#endif

#if defined (GL_API)
static float material[] = {
  SPECULAR, 0.0, 0.0, 0.0,
  DIFFUSE,  0.78, 0.59, 0.49,
  SHININESS, 120.0,
  LMNULL};
static float light_sourceA[] = {
  LCOLOR,    0.9, 0.9, 0.9,
  AMBIENT,   0.2, 0.2, 0.2,
  POSITION,  0.1, 0.1, 0.5, 0.0,
  LMNULL};
static float light_sourceB[] = {
  LCOLOR,    0.5, 0.5, 0.5,
  AMBIENT,   0.2, 0.2, 0.2,
  POSITION,  0.1, 0.15, 0.5, 0.0,
  LMNULL};
static float light_model[] = {
  AMBIENT, 0.7, 0.7, 0.7,
  LMNULL};

static Matrix idmat = { 1.0, 0.0, 0.0, 0.0,
			  0.0, 1.0, 0.0, 0.0,
			  0.0, 0.0, 1.0, 0.0,
			  0.0, 0.0, 0.0, 1.0 } ;
#endif /* defined (GL_API) */

static struct FieldColor {
  unsigned short red;
  unsigned short green;
  unsigned short blue;
} *field;

typedef struct {
  Float64 x, y, z;
} dverticies_type;

#if defined (GL_API)
static Screencoord vwp_left, vwp_right, vwp_bottom, vwp_top;
#endif /* defined (GL_API) */
static Int32 object, VWPHeight, VWPWidth, *GLBuffer, *GLScratch;
static double *GLgeometryX, *GLgeometryY, *GLgeometryZ, *GLGeomMask,
	xmax, xmin, ymax, ymin, zmax, zmin;
static boolean_t DoubleBufferState, RGBModeState, GLGeomMaskDefined,
	ViewingTransformDefined, ObjectDefined, GLInitLightingDone;
static int GLObjectType, FieldWidth, FieldHeight, GeomWidth, GeomHeight,
	mmode_state;

void InitXVGGLGaphics(void)
{
  GLgeometryX = NULL;
  GLgeometryY = NULL;
  GLgeometryZ = NULL;
  GLGeomMask = NULL;
  GLBuffer = NULL;
  GLScratch = NULL;
  field = NULL;
  DoubleBufferState = B_FALSE;
  RGBModeState = B_TRUE;
  GLGeomMaskDefined = B_FALSE;
  GLInitLightingDone = B_FALSE;
  ObjectDefined = B_FALSE;
  ViewingTransformDefined = B_FALSE;
  GLObjectType = SHADEDGEOMETRYOBJ;
  VWPHeight = 0;
  VWPWidth = 0;
  FieldWidth = 0;
  FieldHeight = 0;
  GeomWidth = 0;
  GeomHeight = 0;
  object = -1;
  mmode_state = -1;
  GLMovieNFrames = 0;
  if (VerboseFlag)
    printf("InitXVGGLGaphics() done...\n");
}


static void GLInitLighting(void)
{
  /* check to see if this was done already */
  if (GLInitLightingDone)
    return;

#if defined (GL_API)
  /* define and bind lighting calculation parameters */
  mmode(MVIEWING);
  lmdef(DEFMATERIAL, 1, 11, material);
  lmdef(DEFLMODEL, 1, 5, light_model);
  lmdef(DEFLIGHT, 1, 11, light_sourceA);
  lmbind(MATERIAL, 1);
  lmbind(LIGHT0, 1);
#ifndef SGI
  lmbind(LIGHT1, 2);
#endif
  lmbind(LMODEL, 1);
  mmode(mmode_state);
  if (VerboseFlag) {
    if (mmode_state == MSINGLE)
      printf("GLInitLighting() : mmode(MSINGLE)...\n");
    else if (mmode_state == MVIEWING)
      printf("GLInitLighting() : mmode(MVIEWING)...\n");
    else
      printf("GLInitLighting() : INVALID MMODE...\n");
  }

  /* set internal vars */
  GLInitLightingDone = B_TRUE;
#endif /* defined (GL_API) */
}

static boolean_t XVG_glconfig(boolean_t DoubleBufferFlag,
			      boolean_t RGBModeFlag,
			      int GLMMODE)
{
#if defined (GL_API)
#ifdef SGI
  static Window GLw;
  XVisualInfo *visuals, GLVisual;
  XSetWindowAttributes new_attributes;
  VisualID vid;
  GLXconfig *sdesc_out;
  GLXconfig sdesc_in[] = {
	{GLX_NORMAL, GLX_DOUBLE, B_TRUE},
	{GLX_NORMAL, GLX_RGB, B_TRUE},
	{0 ,0 ,0}};
  Colormap GLcmap;
  XStandardColormap Scmap;
#endif
  int i, go, rc, nitems;

  /* check if there is a change in state */
  if ((DoubleBufferState != DoubleBufferFlag) || (RGBModeState != RGBModeFlag)
    || (GLWindowMapped != B_TRUE)) {

#ifdef SGI
   /* unlink the current window, if required */
    if (GLWindowMapped) {
      GLXunlink(UxDisplay, GLw);
      if (VerboseFlag)
        printf("XVG_glconfig() : Unliked GL drawing window...\n");
    }

    /* set the flag status in the GLX_DOUBLE field */
    if (DoubleBufferFlag) {
      sdesc_in[0].arg = B_TRUE;
      if (VerboseFlag)
        printf("XVG_glconfig() : Doublebuffer mode...\n");
    }
    else {
      sdesc_in[0].arg = B_FALSE;
      if (VerboseFlag)
        printf("XVG_glconfig() : Singlebuffer mode...\n");
    }

    /* set the flag status in the GLX_DOUBLE field */
    if (RGBModeFlag) {
      sdesc_in[1].arg = B_TRUE;
      if (VerboseFlag)
        printf("XVG_glconfig() : RGB mode...\n");
    }
    else {
      sdesc_in[1].arg = B_FALSE;
      if (VerboseFlag)
        printf("XVG_glconfig() : Not RGB mode...\n");
    }

    /* request a GL visual configuration */
    if ((sdesc_out = GLXgetconfig(UxDisplay, UxScreen, sdesc_in)) == 0) {
      ErrorMsg("XVG_glconfig() : Error returning from GLXgetconfig()");
      return(B_FALSE);
    }
    if (VerboseFlag)
      printf("XVG_glconfig() : Got GLXconfig structure...\n");

    /* create the X window with the GL visual, if required */
    if (GLWindowMapped == B_FALSE) {
      /* find the GLX_VISUAL and GLX_COLORMAP fields and fetch the IDs */
      for (i = 0, vid = -1, GLcmap = -1; sdesc_out[i].buffer != 0; i++) {
        if (sdesc_out[i].buffer == GLX_NORMAL) {
          switch (sdesc_out[i].mode) {
            case GLX_VISUAL:
              vid = sdesc_out[i].arg;
              if (VerboseFlag)
                printf("XVG_glconfig() : GL Visual ID : %d...\n", vid);
            break;
            case GLX_COLORMAP:
              GLcmap = sdesc_out[i].arg;
              if (VerboseFlag)
                printf("XVG_glconfig() : GL colormap ID : %d...\n", GLcmap);
            break;
            default:
            break;
          }
        }
      }
      if ((GLcmap == -1) && (vid == -1)) {
        ErrorMsg("XVG_glconfig() : GLX_VISUAL and/or GLX_COLORMAP fields not found");
        return(B_FALSE);
      }

      /* see if this visual is available */
      GLVisual.visualid = vid;
      if ((visuals = XGetVisualInfo(UxDisplay, VisualIDMask, &GLVisual, &nitems))
        == NULL) {
        ErrorMsg("XVG_glconfig() : No matching visuals...");
        return(B_FALSE);
      }
      if (VerboseFlag)
        printf("XVG_glconfig() : %d GL visuals available (depth = %d)...\n",
	  nitems, visuals[0].depth);

      /* create and map the window */
      new_attributes.colormap = GLcmap;
      new_attributes.backing_store = NotUseful;
      new_attributes.border_pixel = 0;
      GLw = XCreateWindow(UxDisplay, XtWindow(GLWindowDAW), 0, 0, 640, 480,
        0, visuals[0].depth, InputOutput, visuals[0].visual,
        (unsigned long) (CWBorderPixel | CWBackingStore | CWColormap), &new_attributes);
      XMapWindow(UxDisplay, GLw);
      if (VerboseFlag)
        printf("XVG_glconfig() : GL X window created and mapped...\n");
    }

    /* find the GLX_WINDOW field and load it with the desired X window ID */
    for (i = 0, go = B_TRUE; (sdesc_out[i].buffer) && (go); i++) {
      if (sdesc_out[i].mode == GLX_WINDOW) {
        go = B_FALSE;
        sdesc_out[i].arg = GLw;
      }
    }
    if (go) {
      ErrorMsg("XVG_glconfig() : GLX_WINDOW field not found");
      return(B_FALSE);
    }
    if (VerboseFlag)
      printf("XVG_glconfig() : GL X window ID loaded into GLXconfig structure...\n");

    /* link GL with the X window for drawing */
    if ((rc = GLXlink(UxDisplay, sdesc_out)) != GLWS_NOERROR) {
      sprintf(cbuf, "XVG_glconfig() : Error returning from GLXlink(rc = %d)", rc);
      ErrorMsg(cbuf);
      return(B_FALSE);
    }
    if (VerboseFlag)
      printf("XVG_glconfig() : GL X window linked with GL...\n");

    /* set this window for drawing */
    if (GLXwinset(UxDisplay, GLw) != GLWS_NOERROR) {
      ErrorMsg("XVG_glconfig() : Error returning from GLXwinset()");
      return(B_FALSE);
    }
    if (VerboseFlag)
      printf("XVG_glconfig() : GL rendering in X window enabled...\n");

#ifdef NOTHIS
    /* check the display mode */
    printf("XVG_glconfig() : double buffer state: %d\n",
	getgconfig((long) GC_DOUBLE));
    printf("XVG_glconfig() : RGB mode state: %d\n",
	getgconfig((long) GC_BITS_CMODE));
    printf("XVG_glconfig() : mmode : %d\n", getmmode());
#endif

#else
    /* map the GL window if required */
    if (GLWindowMapped == B_FALSE) {
      winX(UxDisplay, XtWindow(GLWindowDAW));
      if (VerboseFlag)
	printf("XVG_glconfig() : winX()...\n");
    }
    /* set the buffer and drawing modes */
    if (DoubleBufferFlag) {
      doublebuffer();
      if (VerboseFlag)
	printf("XVG_glconfig() : doublebuffer()...\n");
    }
    else {
      singlebuffer();
      if (VerboseFlag)
	printf("XVG_glconfig() : singlebuffer()...\n");
    }
    RGBmode();
    /* reconfigure GL */
    gconfig();
#endif

    /* reset internal state flags */
    DoubleBufferState = DoubleBufferFlag;
    RGBModeState = RGBModeFlag;
  }

  /* set correct mode */
  if ((DoubleBufferState != DoubleBufferFlag) || (RGBModeState != RGBModeFlag)
    || (GLMMODE != mmode_state) || (GLWindowMapped != B_TRUE)) {
    mmode(GLMMODE);
    if (VerboseFlag) {
      if (GLMMODE == MSINGLE)
        printf("XVG_glconfig() : mmode(MSINGLE)...\n");
      else if (GLMMODE == MVIEWING)
        printf("XVG_glconfig() : mmode(MVIEWING)...\n");
      else
        printf("XVG_glconfig() : INVALID MMODE...\n");
    }
    mmode_state = GLMMODE;
  }

  /* init oighting if required */
  if (GLInitLightingDone == B_FALSE)
    GLInitLighting();

  /* get viewport data */
  getviewport(&vwp_left, &vwp_right, &vwp_bottom, &vwp_top);
  VWPWidth = vwp_right - vwp_left + 1;
  VWPHeight = vwp_top - vwp_bottom +1;
  if (VerboseFlag)
    printf("XVG_glconfig() : viewport is %d,%d,%d,%d\n",
      vwp_left, vwp_right, vwp_bottom, vwp_top); 

  /* alloc memory for internal buffers */
  if ((GLBuffer = (Int32 *) XvgMM_Alloc((void *) GLBuffer,
			     VWPHeight * VWPWidth * sizeof(Int32)))
      == NULL) {
    ErrorMsg("Error allocating for \"GLBuffer\" in XVG_glconfig()");
    return(B_FALSE);
  }
  if ((GLScratch = (Int32 *) XvgMM_Alloc((void *) GLScratch,
			     VWPHeight * VWPWidth * sizeof(Int32)))
      == NULL) {
    ErrorMsg("Error allocating for \"GLScratch\" in XVG_glconfig()");
    return(B_FALSE);
  }

  /* return success */
  GLWindowMapped = B_TRUE;
  return(B_TRUE);
#else /* defined (GL_API) */
  return(B_FALSE);
#endif /* defined (GL_API) */
}

static boolean_t ComputeNormal(dverticies_type *dverticies, int k)
{
  static int N = 3;
#if defined (GL_API)
  Coord norm[3];
#endif /* defined (GL_API) */
  double A[3*3], AA[3*3], B[3], C[3], WKS1[3], WKS2[3], scale;
  int IFAIL;

  /* no need for normal if defining first two verticies */
  if (k < 2)
    return(B_TRUE);
  
  /* compute gradient coefficients */
  A[0] = dverticies[k].x;
  A[1] = dverticies[k-1].x;
  A[2] = dverticies[k-2].x;
  A[3] = dverticies[k].y;
  A[4] = dverticies[k-1].y;
  A[5] = dverticies[k-2].y;
  A[6] = dverticies[k].z;
  A[7] = dverticies[k-1].z;
  A[8] = dverticies[k-2].z;
  B[0] = B[1] = B[2] = 1;
  IFAIL = 1;
  f04atf(&(A[0]), &N, &(B[0]), &N, &(C[0]), &(AA[0]), &N,
	 &(WKS1[0]), &(WKS2[0]), &IFAIL);
  
#if defined (GL_API)
  /* normalize */
  if (IFAIL == 0) {
    scale=1.0 / sqrt(C[0]*C[0] + C[1]*C[1] + C[2]*C[2]);
    if (finite(scale) == 0) {
      sprintf(cbuf,"Arithmetic over/underflow in ComputeNormal() [%e,%e,%e]",
	      C[0], C[1], C[2]);
      ErrorMsg(cbuf);
      return(B_FALSE);
    }
    norm[0] = C[0] * scale;
    norm[1] = C[1] * scale;
    norm[2] = C[2] * scale;
  }
  else {
    norm[0] = 1;
    norm[1] = 1;
    norm[2] = 1;
  }

  /* write normal */
  normal(norm);
  return(B_TRUE);
#else /* defined (GL_API) */
  return(B_FALSE);
#endif /* defined (GL_API) */
}

void Build3DConeObject(double *inpixels, double *outpixels,
			int height, int width, void *ParmPtrs[])
{
  Float64 vertex[3];
#if defined (GL_API)
  Coord near, far;
#endif /* defined (GL_API) */
  unsigned short color;
  double divinv, max, min, ObjMaxSiz, ObjDist, ZDx, radius,
  ConeRadius, ConeHeight, angle, x, y, fovy;
  int i, j, k;

  /* check that GL Shell is up */
  if (GLShellCreated == B_FALSE) {
    ErrorMsg("GLShell is not created in Build3DConeObject()");
    return;
  }

  /* load in parameters */
  ConeRadius = *((double *) ParmPtrs[1]);
  angle = *((double *) ParmPtrs[2]);
  ConeHeight = ConeRadius * tan(angle*PI/180.0);
  StereoCameraSeparation  = *((double *) ParmPtrs[3]);
  if (VerboseFlag)
    printf("Build3DConeObject() => ConeHeight:%fm, ConeRadius : %fm\n",
	ConeHeight, ConeRadius);
  
  /* set internal parameters parameters */
  ZDx = 0;
  ObjMaxSiz = 200.0;
  ObjDist   = 600.0;
  StereoMMPerPixel = ObjMaxSiz/((double) (height-1));
  StereoFocalLength = ObjDist;

  /* show internal parameters */
  if (VerboseFlag)
    printf("Build3DConeObject() => MMPerPixel:%fm, CameraSeparation : %fm\n",
	StereoMMPerPixel, StereoCameraSeparation);

  /* map the GL window and reconfigure the GL system */
#if defined (GL_API)
  if (XVG_glconfig(B_FALSE, B_TRUE, MSINGLE) == B_FALSE)
#endif /* defined (GL_API) */
    return;

#if defined (GL_API)
  /* build the speckle plane object */
  draw_mesg("Build3DConeObject() : Building GL object...");
#ifndef SGI
  if (isobj(object)) {
    delobj(object);
    if (VerboseFlag)
      printf("Build3DConeObject() : GL Object deleted...\n");
  }
#endif
  makeobj(object = genobj());
  if (VerboseFlag)
    printf("Build3DConeObject() => Building object %d...\n", object);
  DynamicRange(inpixels, width*height, &max, &min);
  if (VerboseFlag)
    printf("Build3DConeObject() => dynamic range is (%f, %f)\n",
        min, max);
  divinv = 255.0/(max - min);
  for (j = 0; j < (height-1); j++) {
    if (j % 25 == 0) {
      sprintf(cbuf, "Build3DConeObject() : Building GL object at line %d...",
	      j);
      draw_mesg(cbuf);
    }
    for (i = 0; i < width;) {
      bgntmesh();
      for (k = 0; (k < NGLVERTICIES) && (i < width); i++, k++) {
        x = ((double) (i - width/2)) * StereoMMPerPixel;
        y = ((double) (j - height/2)) * StereoMMPerPixel;
        radius = sqrt(x*x + y*y);
	vertex[0] = x;
	vertex[1] = y;
        if (radius <= ConeRadius)
          vertex[2] = ConeHeight*(1.0 - radius/ConeRadius);
        else
          vertex[2] = 0;
	color = (inpixels[j*width + i] - min)*divinv;
	RGBcolor(color, color, color);
	v3d(vertex);
	
        x = ((double) (i - width/2)) * StereoMMPerPixel;
        y = ((double) (j + 1 - height/2)) * StereoMMPerPixel;
        radius = sqrt(x*x + y*y);
	vertex[0] = x;
	vertex[1] = y;
        if (radius <= ConeRadius)
          vertex[2] = ConeHeight*(1.0 - radius/ConeRadius);
        else
          vertex[2] = 0;
	color = (inpixels[(j+1)*width + i] - min)*divinv;
	RGBcolor(color, color, color);
	v3d(vertex);
      }
      endtmesh();
      if (i < width)
	i--;
    }
  }
  closeobj();
  
  /* set up the viewing transformation */
#if defined (SGI) && defined (SGI32)
 loadmatrix((const float (*)[4]) idmat);
#else
  loadmatrix(idmat);
#endif
  lookat((Coord) 0.0, (Coord) 0.0, (Coord) 1.0,
	 (Coord) 0.0, (Coord) 0.0, (Coord) 0.0,
	 (Angle) 0.0);
  near = ObjDist - (ConeHeight + 1.0e-7);
  far  = ObjDist + 1.0e-7;
  fovy = ceil(atan(ObjMaxSiz/(ObjDist))/PI*180.0*10.0);
  perspective((Angle) fovy,
	      ((Float32) VWPWidth) / ((Float32) VWPHeight), near, far);

  /* draw the left image and capture to disk */
  draw_mesg("Build3DConeObject() : Rendering left image...");
  RGBcolor(0,0,0);
  clear();
  pushmatrix();
  translate((Coord) StereoCameraSeparation/2.0, (Coord) 0.0,
	(Coord) (-ObjDist));
  callobj(object);
  popmatrix();
  draw_mesg("Build3DConeObject() : Saving left image...");
  sprintf(cbuf, "%s_left.tif", ParmPtrs[0]);
  lrectread(0, 0, VWPWidth-1, VWPHeight-1, GLBuffer);
  WriteGLGrayLevelTiffFile(cbuf, VWPWidth, VWPHeight, (int *)GLBuffer,
			   (char *)GLScratch);

  /* draw the left image and capture to disk */
  draw_mesg("Build3DConeObject() : Rendering right image...");
  RGBcolor(0,0,0);
  clear();
  pushmatrix();
  translate((Coord) -StereoCameraSeparation/2.0, (Coord) 0.0,
	(Coord) (-ObjDist - ZDx));
  callobj(object);
  popmatrix();
  draw_mesg("Build3DConeObject() : Saving right image...");
  sprintf(cbuf, "%s_right.tif", ParmPtrs[0]);
  lrectread(0, 0, VWPWidth-1, VWPHeight-1, GLBuffer);
  WriteGLGrayLevelTiffFile(cbuf, VWPWidth, VWPHeight, (int *)GLBuffer,
			   (char *)GLScratch);

  /* set up the stereo image parameters correctly stereo reconstruction */
  StereoFocalLength = ObjDist;
  StereoMMPerPixel = 2.0*StereoFocalLength*tan(fovy*PI/(2.0 *180.0 *10.0))
	/((double) VWPHeight);
  if (VerboseFlag)
    printf("Build3DPlaneObject() : %.3fmm per pixel at f = %.3fmm\n",
	StereoMMPerPixel, StereoFocalLength);

  /* copy the data to the output buffer */
  for (i = 0; i < height*width; i++)
    outpixels[i] = inpixels[i];
#endif /* defined (GL_API) */
}


void Build3DHemisphere(double *inpixels, double *outpixels,
		       int height, int width, void *ParmPtrs[])
{
  Float64 vertex[3];
  Float32 colors[3];
#if defined (GL_API)
  Coord near, far;
#endif /* defined (GL_API) */
  unsigned short color;
  double ObjMaxSiz, ObjDist, ZDx, Radius, angle, x, y, fovy;
  int i, j, k;

  /* check that GL Shell is up */
  if (GLShellCreated == B_FALSE) {
    ErrorMsg("GLShell is not created in Build3DHemisphere()");
    return;
  }

  /* load in parameters */
  Radius = *((double *) ParmPtrs[1]);
  StereoCameraSeparation  = *((double *) ParmPtrs[2]);
  Normalize(inpixels, outpixels, height, width, NULL);
  if (VerboseFlag)
    printf("Build3DHemisphere() => Radius : %fm\n", Radius);
  
  /* set internal parameters parameters */
  ZDx = 0;
  ObjMaxSiz = 2.5 * Radius;
  ObjDist   = 600.0;
  StereoMMPerPixel = ObjMaxSiz/((double) (height-1));
  StereoFocalLength = ObjDist;

  /* map the GL window and reconfigure the GL system */
#if defined (GL_API)
  if (XVG_glconfig(B_FALSE, B_TRUE, MSINGLE) == B_FALSE)
#endif /* defined (GL_API) */
    return;

#if defined (GL_API)
  /* build the speckle plane object */
  draw_mesg("Build3DHemisphere() : Building GL object...");
#ifndef SGI
  if (isobj(object)) {
    delobj(object);
    if (VerboseFlag)
      printf("Build3DHemisphere() : GL Object deleted...\n");
  }
#endif
  makeobj(object = genobj());
  if (VerboseFlag)
    printf("Build3DHemisphere() => Building object %d...\n", object);
  for (j = 0; j < (height-1); j++) {
    if (j % 25 == 0) {
      sprintf(cbuf, "Build3DHemisphere() : Building GL object at line %d...",
	      j);
      draw_mesg(cbuf);
    }
    for (i = 0; i < width;) {
      bgntmesh();
      for (k = 0; (k < NGLVERTICIES) && (i < width); i++, k++) {
	/* x & y world coordinates */
        x = ((double) (i - width/2)) * StereoMMPerPixel;
        y = ((double) (j - height/2)) * StereoMMPerPixel;

	/* vertex 1 */
	vertex[0] = x;
	vertex[1] = y;
        if (sqrt(x*x + y*y) <= Radius)
          vertex[2] = sqrt(Radius*Radius - x*x - y*y);
        else
          vertex[2] = 0;
	colors[0] = colors[1] = colors[2] = outpixels[j*width + i];
	c3f(colors);
	v3d(vertex);
	
	/* vertex 2 */
        y += StereoMMPerPixel;
	vertex[0] = x;
	vertex[1] = y;
        if (sqrt(x*x + y*y) <= Radius)
          vertex[2] = sqrt(Radius*Radius - x*x - y*y);
        else
          vertex[2] = 0;
	colors[0] = colors[1] = colors[2] = outpixels[(j+1)*width + i];
	c3f(colors);
	v3d(vertex);
      }
      endtmesh();
      if (i < width)
	i--;
    }
  }
  closeobj();
  if (VerboseFlag)
    printf("Build3DHemisphere() => Object defined...\n");
  
  /* set up the viewing transformation */
#if defined (SGI) && defined (SGI32)
  loadmatrix((const float (*)[4]) idmat);
#else
  loadmatrix(idmat);
#endif
  lookat((Coord) 0.0, (Coord) 0.0, (Coord) 1.0,
	 (Coord) 0.0, (Coord) 0.0, (Coord) 0.0,
	 (Angle) 0.0);
  near = ObjDist - (Radius + 1.0e-7);
  far  = ObjDist + 1.0e-7;
  fovy = ceil(atan(ObjMaxSiz/(ObjDist))/PI*180.0*10.0);
  perspective((Angle) fovy,
	      ((Float32) VWPWidth) / ((Float32) VWPHeight), near, far);
  if (VerboseFlag)
    printf("Build3DHemisphere() => Viewing matrix defined...\n");

  /* draw the left image and capture to disk */
  draw_mesg("Build3DHemisphere() : Rendering left image...");
  RGBcolor(0,0,0);
  clear();
  pushmatrix();
  translate((Coord) StereoCameraSeparation/2.0, (Coord) 0.0,
	(Coord) (-ObjDist));
  callobj(object);
  if (VerboseFlag)
    printf("Build3DHemisphere() => Left image of object drawn...\n");
  popmatrix();
  draw_mesg("Build3DHemisphere() : Saving left image...");
  sprintf(cbuf, "%s_left.tif", ParmPtrs[0]);
  lrectread(0, 0, VWPWidth-1, VWPHeight-1, GLBuffer);
  if (VerboseFlag)
    printf("Build3DHemisphere() => Left image of object read...\n");
  WriteGLGrayLevelTiffFile(cbuf, VWPWidth, VWPHeight, (int *)GLBuffer,
			   (char *)GLScratch);
  if (VerboseFlag)
    printf("Build3DHemisphere() => Left image of object saved...\n");

  /* draw the left image and capture to disk */
  draw_mesg("Build3DHemisphere() : Rendering right image...");
  RGBcolor(0,0,0);
  clear();
  pushmatrix();
  translate((Coord) -StereoCameraSeparation/2.0, (Coord) 0.0,
	(Coord) (-ObjDist - ZDx));
  callobj(object);
  if (VerboseFlag)
    printf("Build3DHemisphere() => Right image of object drawn...\n");
  popmatrix();
  draw_mesg("Build3DHemisphere() : Saving right image...");
  sprintf(cbuf, "%s_right.tif", ParmPtrs[0]);
  lrectread(0, 0, VWPWidth-1, VWPHeight-1, GLBuffer);
  WriteGLGrayLevelTiffFile(cbuf, VWPWidth, VWPHeight, (int *)GLBuffer,
			   (char *)GLScratch);

  /* set up the stereo image parameters correctly stereo reconstruction */
  StereoFocalLength = ObjDist;
  StereoMMPerPixel = 2.0*StereoFocalLength*tan(fovy*PI/(2.0 *180.0 *10.0))
	/((double) VWPHeight);
  if (VerboseFlag)
    printf("Build3DPlaneObject() : %.3fmm per pixel at f = %.3fmm\n",
	StereoMMPerPixel, StereoFocalLength);

  /* copy the data to the output buffer */
  for (i = 0; i < height*width; i++)
    outpixels[i] = inpixels[i];
#endif /* defined (GL_API) */
}


void Build3DPlaneObject(double *inpixels, double *outpixels,
			int height, int width, void *ParmPtrs[])
{
  Float64 vertex[3];
#if defined (GL_API)
  Coord near, far;
#endif /* defined (GL_API) */
  unsigned short color;
  double divinv, max, min, ObjMaxSiz, ObjDist, ZDx, XRot, YRot, fovy;
  int i, j, k;

  /* check that GL Shell is up */
  if (GLShellCreated == B_FALSE) {
    ErrorMsg("GLShell is not created in Build3DPlaneObject()");
    return;
  }

  /* load in parameters */
  XRot = *((double *) ParmPtrs[1]);
  YRot = *((double *) ParmPtrs[2]);
  StereoCameraSeparation  = *((double *) ParmPtrs[3]);
/*
  StereoCameraSeparation = 50.0;
*/

  /* set internal parameters parameters */
  ObjMaxSiz = 200.0;
  ObjDist   = 600.0;
  StereoMMPerPixel = ObjMaxSiz/((double) (height-1));
  ZDx = 0;
/*
  ZDx = *((double *) ParmPtrs[3]);
*/

  /* map the GL window and reconfigure the GL system */
#if defined (GL_API)
  if (XVG_glconfig(B_FALSE, B_TRUE, MSINGLE) == B_FALSE)
#endif /* defined (GL_API) */
    return;

#if defined (GL_API)
  /* build the speckle plane object */
  draw_mesg("Build3DPlaneObject() : Building GL object...");
#ifndef SGI
  if (isobj(object)) {
    delobj(object);
    if (VerboseFlag)
      printf("Build3DPlaneObject() : GL Object deleted...\n");
  }
#endif
  makeobj(object = genobj());
  if (VerboseFlag)
    printf("Build3DPlaneObject() => Building object %d...\n", object);
  DynamicRange(inpixels, width*height, &max, &min);
  if (VerboseFlag)
    printf("Build3DPlaneObject() => dynamic range is (%f, %f)\n",
        min, max);
  divinv = 255.0/(max - min);
  vertex[2] = 0;
  for (j = 0; j < (height-1); j++) {
    if (j % 25 == 0) {
      sprintf(cbuf, "Build3DPlaneObject() : Building GL object at line %d...",
	      j);
      draw_mesg(cbuf);
    }
    for (i = 0; i < width;) {
      bgntmesh();
      for (k = 0; (k < NGLVERTICIES) && (i < width); i++, k++) {
	vertex[0] = ((double) (i - width/2)) * StereoMMPerPixel;
	vertex[1] = ((double) (j - height/2)) * StereoMMPerPixel;
	color = (inpixels[j*width + i] - min)*divinv;
	RGBcolor(color, color, color);
	v3d(vertex);
	
	vertex[0] = ((double) (i - width/2))  * StereoMMPerPixel;
	vertex[1] = ((double) (j+1 - height/2)) * StereoMMPerPixel;
	color = (inpixels[(j+1)*width + i] - min)*divinv;
	RGBcolor(color, color, color);
	v3d(vertex);
      }
      endtmesh();
      if (i < width)
	i--;
    }
  }
  closeobj();
  
  /* set up the viewing transformation */
#if defined (SGI) && defined (SGI32)
  loadmatrix((const float (*)[4]) idmat);
#else
  loadmatrix(idmat);
#endif
  lookat((Coord) 0.0, (Coord) 0.0, (Coord) 1.0,
	 (Coord) 0.0, (Coord) 0.0, (Coord) 0.0,
	 (Angle) 0.0);
  if ((ZDx == 0.0) && (XRot == 0.0) && (YRot == 0.0)) {
    near = ObjDist - 1.0e-8;
    far  = ObjDist + 1.0e-8; 
  }
  else {
    near = 0.5*ObjDist;
    far  = 1.5*ObjDist;
  }
  fovy = ceil(atan(ObjMaxSiz/(ObjDist))/PI*180.0*10.0);
  perspective((Angle) fovy,
	      ((Float32) VWPWidth) / ((Float32) VWPHeight), near, far);

  /* show internal parameters */
  if (VerboseFlag)
    printf("Build3DPlaneObject() => FovY: %.3f degrees, MMPerPixel:%.3fmm, CameraSeparation : %.3fmm\n",
	fovy/10.0, StereoMMPerPixel, StereoCameraSeparation);

  /* draw the left image and capture to disk */
  draw_mesg("Build3DPlaneObject() : Rendering left image...");
  RGBcolor(0,0,0);
  clear();
  pushmatrix();
  translate((Coord) StereoCameraSeparation/2.0, (Coord) 0.0, (Coord) (-ObjDist));
  rot((Float32) XRot, 'X');
  rot((Float32) YRot, 'Y');
  callobj(object);
  popmatrix();
  draw_mesg("Build3DPlaneObject() : Saving left image...");
  sprintf(cbuf, "%s_left.tif", ParmPtrs[0]);
  lrectread(0, 0, VWPWidth-1, VWPHeight-1, GLBuffer);
  WriteGLGrayLevelTiffFile(cbuf, VWPWidth, VWPHeight, (int *)GLBuffer,
			   (char *)GLScratch);

  /* draw the left image and capture to disk */
  draw_mesg("Build3DPlaneObject() : Rendering right image...");
  RGBcolor(0,0,0);
  clear();
  pushmatrix();
  translate((Coord) -StereoCameraSeparation/2.0, (Coord) 0.0,
	(Coord) (-ObjDist - ZDx));
  rot((Float32) XRot, 'X');
  rot((Float32) YRot, 'Y');
  callobj(object);
  popmatrix();
  draw_mesg("Build3DPlaneObject() : Saving right image...");
  sprintf(cbuf, "%s_right.tif", ParmPtrs[0]);
  lrectread(0, 0, VWPWidth-1, VWPHeight-1, GLBuffer);
  WriteGLGrayLevelTiffFile(cbuf, VWPWidth, VWPHeight, (int *)GLBuffer,
			   (char *)GLScratch);

  /* copy the data to the output buffer */
  for (i = 0; i < height*width; i++)
    outpixels[i] = inpixels[i];

  /* set up the stereo image parameters correctly stereo reconstruction */
  StereoFocalLength = ObjDist;
  StereoMMPerPixel = 2.0*StereoFocalLength*tan(fovy*PI/(2.0 *180.0 *10.0))
	/((double) VWPHeight);
  if (VerboseFlag)
    printf("Build3DPlaneObject() : %.3fmm per pixel at f = %.3fmm\n",
	StereoMMPerPixel, StereoFocalLength);
#endif /* defined (GL_API) */
}


static void BuildMeshSurface(int decimation, boolean_t MakeObjectFlag)
{
  static dverticies_type dverticies[256];
  int i, j, k, xinc, yinc;

  /* geometry data is loaded */
  if ((GLgeometryX == NULL) || (GLgeometryY == NULL)
      || (GLgeometryZ == NULL)) {
    ErrorMsg("BuildMeshSurface() : No geometry defined!");
    return;
  }

  /* if in field overlay mode, make sure an appropriate field is loaded */
  if (GLObjectType == OVERLAYOBJ) {
    if (field == NULL) {
      ErrorMsg("BuildMeshSurface() : No field defined!");
      return;
    }
    if ((FieldWidth != GeomWidth) || (FieldHeight != GeomHeight)) {
      ErrorMsg("BuildMeshSurface() : Field and geometry dimensions mismatch!");
      return;
    }
  }

#if defined (GL_API)
  if (MakeObjectFlag) {
    makeobj(object = genobj());
    if (VerboseFlag) {
      if (GLObjectType == OVERLAYOBJ)
        printf("BuildMeshSurface() => Building overlay object %d...\n",
	       object);
      else
        printf("BuildMeshSurface() => Building shaded object %d...\n", object);
    }
  }
  for (xinc = yinc = decimation, j=0, k=0; j < GeomHeight-yinc; j+=yinc) {
    if (j % 25 == 0) {
      sprintf(cbuf, "BuildMeshSurface() : Building GL object at line %d...",
	      j);
      draw_mesg(cbuf);
    }
    for (i = 0; i < GeomWidth-xinc;) {
      bgntmesh();
      for (k = 0; (k < NGLVERTICIES) && (i < GeomWidth-xinc); i+=xinc) {
	if (((GLGeomMaskDefined) && (GLGeomMask[j*GeomWidth + i] != 0)
	     && (GLGeomMask[(j+yinc)*GeomWidth + i] != 0))
	    || (GLGeomMaskDefined == B_FALSE)) {
	  dverticies[k].x = GLgeometryX[j*GeomWidth + i];
	  dverticies[k].y = GLgeometryY[j*GeomWidth + i];
	  dverticies[k].z = GLgeometryZ[j*GeomWidth + i];
	  if (GLObjectType == OVERLAYOBJ)
	    RGBcolor(field[j*GeomWidth + i].red,
		     field[j*GeomWidth + i].green,
		     field[j*GeomWidth + i].blue);
	  else if (ComputeNormal(dverticies, k) == B_FALSE)
	    return;
	  v3d((Float64 *) &(dverticies[k++]));
	  
	  dverticies[k].x = GLgeometryX[(j+yinc)*GeomWidth + i];
	  dverticies[k].y = GLgeometryY[(j+yinc)*GeomWidth + i];
	  dverticies[k].z = GLgeometryZ[(j+yinc)*GeomWidth + i];
	  if (GLObjectType == OVERLAYOBJ)
	    RGBcolor(field[(j+yinc)*GeomWidth + i].red,
		     field[(j+yinc)*GeomWidth + i].green,
		     field[(j+yinc)*GeomWidth + i].blue);
	  else if (ComputeNormal(dverticies, k) == B_FALSE)
	    return;
	  v3d((Float64 *) &(dverticies[k++]));
	}
      }
      endtmesh();
      if (i < (GeomWidth-xinc))
	i -= xinc;
    }
  }
  if (MakeObjectFlag) {
    closeobj();
    ObjectDefined = B_TRUE;
  }
#endif /* defined (GL_API) */
}

void GLSetObjectType(int t)
{
#if defined (GL_API)
  if (GLWindowMapped == B_FALSE)
    XVG_glconfig(B_TRUE, B_TRUE, MSINGLE);
  
  if (t == OVERLAYOBJ) {
    GLObjectType = OVERLAYOBJ;
    mmode_state = MSINGLE;
    if (VerboseFlag)
      printf("GLSetObjectType() : mmode(MSINGLE)...\n");
  }
  else {
    GLObjectType = SHADEDGEOMETRYOBJ;
    mmode_state = MVIEWING;
    if (VerboseFlag)
      printf("GLSetObjectType() : mmode(MVIEWING)...\n");
  }
  mmode(mmode_state);
#endif /* defined (GL_API) */
 }

void LoadGLGeometry(double PixelsPerMM)
{
  double MMPerPixel, zx, zy, xy;
  int i, j, k;
  
  /* initialize the data pointer */
  if (ProcessedPixels == NULL)
    return;

  /* reset gemetry buffer size values */
  GeomWidth = ImgWidth;
  GeomHeight = ImgHeight;
  if (VerboseFlag)
    printf("LoadGLGeometry() GeomHeight:%d, GeomWidth:%d, PixelsPerMM:%f..\n",
	GeomHeight, GeomWidth, PixelsPerMM);

  /* remove the mean of the Z data */
  SubtractMean(ProcessedPixels, DoubleBuffer, GeomHeight, GeomWidth, NULL);

  /* allocate storage for the X, Y and Z coordinates */
  if ((GLgeometryX = (double *) XvgMM_Alloc((void *) GLgeometryX,
        GeomWidth * GeomHeight * sizeof(double))) == NULL) {
    ErrorMsg("Error allocating for \"geometryX\" in LoadGLGeometry()");
    return;
  }
  if ((GLgeometryY = (double *) XvgMM_Alloc((void *) GLgeometryY,
        GeomWidth * GeomHeight * sizeof(double))) == NULL) {
    ErrorMsg("Error allocating for \"geometryY\" in LoadGLGeometry()");
    return;
  }
  if ((GLgeometryZ = (double *) XvgMM_Alloc((void *) GLgeometryZ,
        GeomWidth * GeomHeight * sizeof(double))) == NULL) {
    ErrorMsg("Error allocating for \"geometryz\" in LoadGLGeometry()");
    return;
  }

  /* if there are two images on the stack, assume that they contain X and Y */
  if (ImageStackTop == 2) {
    /* transfer the data to the X and Y buffers */
    for (i = 0; i < GeomWidth*GeomHeight; i++) {
      GLgeometryX[i] = ImageStack[ImageStackTop-2].pixels[i];
      GLgeometryY[i] = ImageStack[ImageStackTop-1].pixels[i];
    }
    InfoMsg("Three dimensional geometry loaded...");
  }
  else {
    /* transfer the data to the X and Y buffers */
    for (j = 0, k = 0, MMPerPixel = 1.0/PixelsPerMM; j < GeomHeight; j++) {
      for (i = 0; i < GeomWidth; i++, k++) {
        GLgeometryX[k] = ((double) (i - GeomWidth/2)) * MMPerPixel;
        GLgeometryY[k] = ((double) (j - GeomHeight/2)) * MMPerPixel;
      }
    }
  }

  /* copy the data to the Z geometry buffer */
  for (i = 0; i < GeomWidth*GeomHeight; i++)
    GLgeometryZ[i] = DoubleBuffer[i];

  /* save a local copy of the contour and mask if required */
  if (DisplayContourComputed) {
    /* set local flag */
    GLGeomMaskDefined = B_TRUE;

    /* allocate storage for the mask if required */
    if ((GLGeomMask = (double *) XvgMM_Alloc((void *) GLGeomMask,
		GeomWidth * GeomHeight * sizeof(double))) == NULL) {
      ErrorMsg("Error allocating for \"GLGeomMask\" in LoadGLGeometry()");
      return;
    }

    /* copy the mask data to the mask buffer */
    for (i = 0; i < GeomWidth*GeomHeight; i++)
      GLGeomMask[i] = MaskPixels[i];

    /* be verbose if required */
    if (VerboseFlag)
      printf("LoadGLGeometry() : Mask image defined...\n");
  }
  else {
    GLGeomMaskDefined = B_FALSE;
    if (VerboseFlag)
      printf("LoadGLGeometry() : No mask image defined...\n");
  }

  /* find xyz data extrema to define the viewing fustrum */
  xmax = -MAXDOUBLE; xmin = MAXDOUBLE;
  ymax = -MAXDOUBLE; ymin = MAXDOUBLE;
  zmax = -MAXDOUBLE; zmin = MAXDOUBLE;
  for (k = 0; k < GeomHeight*GeomWidth; k++)
    if (((GLGeomMaskDefined) && (GLGeomMask[k] != 0))
	|| (GLGeomMaskDefined == B_FALSE)) {
      xmax = (GLgeometryX[k] > xmax ? GLgeometryX[k] : xmax);
      xmin = (GLgeometryX[k] < xmin ? GLgeometryX[k] : xmin);
      ymax = (GLgeometryY[k] > ymax ? GLgeometryY[k] : ymax);
      ymin = (GLgeometryY[k] < ymin ? GLgeometryY[k] : ymin);
      zmax = (GLgeometryZ[k] > zmax ? GLgeometryZ[k] : zmax);
      zmin = (GLgeometryZ[k] < zmin ? GLgeometryZ[k] : zmin);
    }

  /* check to make sure that data volume is at least somewhat cubic... */
  zx = fabs(zmax - zmin)/fabs(xmax - xmin);
  zy = fabs(zmax - zmin)/fabs(ymax - ymin);
  xy = fabs(xmax - xmin)/fabs(ymax - ymin);
  if ((zx > 10.0) || (zx < 0.1)
      || (zy > 10.0) || (zy < 0.1)
      || (xy > 10.0) || (xy < 0.1))
    InfoMsg("LoadGLGeometry() WARNING : The data volume is not cubic, clipping problems may occur!");

  /* show results if required */
  if (VerboseFlag) {
    printf("LoadGLGeometry() : object dimensions:\n");
    printf("   xmax:%f, xmin:%f\n", xmax, xmin);
    printf("   ymax:%f, ymin:%f\n", ymax, ymin);
    printf("   zmax:%f, zmin:%f\n", zmax, zmin);
  }
}

void LoadGLField(void)
{
  double max, min, divinv, *pixels;
  int i, k, color;

  /* initialize the data pointer */
  if (ProcessedPixels == NULL)
    pixels = RawPixels;
  else
    pixels = ProcessedPixels;

  /* allocate storage for the field if required */
  if ((FieldWidth != ImgWidth)||(FieldHeight != ImgHeight)||(field == NULL)) {
    FieldWidth = ImgWidth;
    FieldHeight = ImgHeight;
    if ((field =
	 (struct FieldColor *) XvgMM_Alloc((void *) field,
					   FieldWidth * FieldHeight
					   * sizeof(struct FieldColor)))
	== NULL) {
      ErrorMsg("Error allocating for \"field\" in LoadGLField()");
      return;
    }
  }

  /* find the dynamic range of the field data and scale to color range */
  DynamicRange(pixels, FieldWidth*FieldHeight, &max, &min);
  if (CurrentCells == BRYGMBVMCells)
    for (i = 0, k = 0, divinv = 16777215.0/(max - min);
	 i < FieldHeight*FieldWidth; i++, k++) {
      color = (pixels[i] - min)*divinv;
      field[k].red =   (color & 0x00ff0000) >> 16;
      field[k].green = (color & 0x0000ff00) >> 8;
      field[k].blue =   color & 0x000000ff;
    }
  else
    for (i = 0, k = 0, divinv = 255.0/(max - min); i < FieldHeight*FieldWidth;
	 i++, k++) {
      color = (pixels[i] - min)*divinv;
      field[k].red = color;
      field[k].green = color;
      field[k].blue = color;
    }
}


void GLBuildPoly(int decimation,
		 int X, int Y, int Z, int Zoom,
		 char *startvals, char *incvals,
		 int frames, 
		 char *SpecularS, char *DiffuseS, char *ShininessS,
		 double PixelsPerMM)
{
  /* set up double buffering and correct mmode */
#if defined (GL_API)
  if (XVG_glconfig(B_TRUE, B_TRUE,
		   (GLObjectType == OVERLAYOBJ? MSINGLE : MVIEWING)) == B_FALSE)
#endif /* defined (GL_API) */
    return;

  /* build object */
  BuildMeshSurface(decimation, B_TRUE);
}


static void InitFustrum(int Zoom)
{
#if defined (GL_API)
  Coord left, right, bottom, top, near, far;
#endif /* defined (GL_API) */
  double xclip, yclip;

#if defined (GL_API)
  /* initialize the perspective transformation to look at the center of */
  /* the object, down the Z axis                                        */
#if defined (SGI) && defined (SGI32)
  loadmatrix((const float (*)[4]) idmat);
#else
  loadmatrix(idmat);
#endif
  lookat((Coord) 0.0, (Coord) 0.0, (Coord) 1.0,
	 (Coord) 0.0, (Coord) 0.0, (Coord) 0.0,
	 (Angle) 0.0);

  /* determines size of XYZ box that preserves proportions in the viewport */
  xclip = (fabs(xmax) > fabs(xmin) ? fabs(xmax) : fabs(xmin))
	/ (((double) Zoom) / 10.0); 
  yclip = (fabs(ymax) > fabs(ymin) ? fabs(ymax) : fabs(ymin))
	/ (((double) Zoom) / 10.0);
  if (((yclip/xclip)*((double) VWPWidth)) <= ((double) VWPHeight)) {
    left = -xclip;
    right = xclip;
    bottom = -xclip*((double) VWPHeight)/((double) VWPWidth);
    top = xclip*((double) VWPHeight)/((double) VWPWidth);
  }
  else {
    left = -yclip*((double) VWPWidth)/((double) VWPHeight);
    right = yclip*((double) VWPWidth)/((double) VWPHeight);
    bottom = -yclip;
    top = yclip;
  }
  near = -3.0/2.0*(zmax - zmin);
  far  = (zmax - zmin)/2.0;

  /* setup the ortho transformation */ 
  ortho(left, right, bottom, top, near, far);
  if (VerboseFlag)
    printf("InitFustrum() => ortho(%f, %f, %f, %f, %f, %f)...\n",
      left, right, bottom, top, -near, -far);
#endif /* defined (GL_API) */
}


void GLDraw(int decimation,
	    int X, int Y, int Z, int Zoom,
	    char *startvals, char *incvals,
	    int frames, 
	    char *SpecularS, char *DiffuseS, char *ShininessS,
	    double PixelsPerMM)
{
  /* check that user has defined object */
  if (ObjectDefined == B_FALSE) {
    ErrorMsg("No object defined! (Use Build buttons)");
    return;
  }

#if defined (GL_API)
  /* compute viewing fustrum */
  InitFustrum(Zoom);

  /* parse the material property arguments if required */
  if (GLObjectType == SHADEDGEOMETRYOBJ) {
    sscanf(SpecularS, "%f,%f,%f",
	   &(material[1]), &(material[2]), &(material[3]));
    sscanf(DiffuseS, "%f,%f,%f",
	   &(material[5]), &(material[6]), &(material[7]));
    sscanf(ShininessS, "%f",
	   &(material[9]));
    lmdef(DEFMATERIAL, 1, 11, material);
  }

  pushmatrix();
  RGBcolor(0,0,0);
  clear();

  rotate((Angle) (X*10.0) ,'X');
  rotate((Angle) (Y*10.0) ,'Y');
  rotate((Angle) (Z*10.0) ,'Z');
  callobj(object);

  swapbuffers();
  popmatrix();
#endif /* defined (GL_API) */
}


void GLResetMovie(int decimation,
	     int X, int Y, int Z,
	     char *startvals, char *incvals,
	     int frames, 
	     char *SpecularS, char *DiffuseS, char *ShininessS,
	     double PixelsPerMM)
{
  GLMovieNFrames = 0;
}

boolean_t GLRecFrame(int decimation,
		   int X, int Y, int Z, int Zoom,
		   char *startvals, char *incvals,
		   int frames, 
		   char *SpecularS, char *DiffuseS, char *ShininessS,
		   double PixelsPerMM)
{
  /* check input parms */
  if (GLMovieNFrames >= GLMOVIEFRAMESNMAX) {
    sprintf(cbuf, "Maximum allowed number of frames (%d) exceeded",
	    GLMOVIEFRAMESNMAX);
    ErrorMsg(cbuf);
    return(B_FALSE);
  }
  MovieType = GLMOVIE;
  
#if defined (GL_API)
  /* set required GL parameters & record frame */
  readsource(SRC_FRONT);
  if ((GLMovieFrames[GLMovieNFrames] =
       (int *) XvgMM_Alloc((void *) GLMovieFrames[GLMovieNFrames],
			   VWPHeight * VWPWidth * sizeof(int)))
      == NULL) {
    sprintf(cbuf, "Could not allocate for frame %d in GLRecFrame()",
	    GLMovieNFrames);
    ErrorMsg(cbuf);
    return(B_FALSE);
  }
  lrectread(vwp_left, vwp_bottom, vwp_right, vwp_top,
	    (Int32 *) GLMovieFrames[GLMovieNFrames++]);
#endif /* defined (GL_API) */
  return(B_TRUE);
}

void GLMovieGenerate(int decimation,
		     int X, int Y, int Z, int Zoom,
		     char *startvals, char *incvals,
		     int frames, 
		     char *SpecularS, char *DiffuseS, char *ShininessS,
		     double PixelsPerMM, char *fname)
{
  boolean_t rc;
  int i;
  
  /* check that user has defined viewing transformation & rotations */
  if (ViewingTransformDefined == B_FALSE) {
    ErrorMsg("Viewing transformation is not defined! (Use Build buttons)");
    return;
  }
  
  /* set required GL parameters */
  MovieType = GLMOVIE;
  
#if defined (GL_API)
  /* parse the material property arguments if required */
  if (GLObjectType == SHADEDGEOMETRYOBJ) {
    sscanf(SpecularS, "%f,%f,%f",
	   &(material[1]), &(material[2]), &(material[3]));
    sscanf(DiffuseS, "%f,%f,%f",
	   &(material[5]), &(material[6]), &(material[7]));
    sscanf(ShininessS, "%f",
	   &(material[9]));
    lmdef(DEFMATERIAL, 1, 11, material);
  }

  /* compute viewing fustrum */
  InitFustrum(Zoom);

  /* load in list of filenames, loop */
  if (LoadMovie(fname)) {
    ScreenSaverOff();
    for (i = 0, rc = B_TRUE, GLMovieNFrames = 0; (i < MovieFilesN) && rc; i++) {
      sprintf(cbuf, "Processing file %s...", MovieFileNames[i]); 
      LoadFile(MovieFileNames[i]);
      ExecuteIPOpList();
      pushmatrix();
      RGBcolor(0,0,0);
      clear();
      rotate((Angle) (X*10.0) ,'X');
      rotate((Angle) (Y*10.0) ,'Y');
      rotate((Angle) (Z*10.0) ,'Z');
      LoadGLGeometry(PixelsPerMM);
      BuildMeshSurface(decimation, B_FALSE);
      swapbuffers();
      popmatrix();
      rc = GLRecFrame(decimation, X, Y, Z, Zoom, startvals, incvals, frames, 
		      SpecularS, DiffuseS, ShininessS, PixelsPerMM);
    }
    ScreenSaverOn();
  }
#endif /* defined (GL_API) */
}

void GLMovieSeqGenerate(int decimation,
			int X, int Y, int Z, int Zoom,
			char *startvals, char *incvals,
			int frames, 
			char *SpecularS, char *DiffuseS, char *ShininessS,
			double PixelsPerMM)
{
  double xs, ys, zs, xinc, yinc, zinc;
  int xrot, yrot, zrot, items, i;

  /* check that user has defined object */
  if (ObjectDefined == B_FALSE) {
    ErrorMsg("GLMovieSeqGenerate() : No object defined!");
    return;
  }

  /* check that user has defined viewing transformation & rotations */
  if (ViewingTransformDefined == B_FALSE) {
    ErrorMsg("GLMovieSeqGenerate() : Viewing transformation is not defined!");
    return;
  }

  /* convert parms */
  sscanf(startvals, "%le,%le,%le", &xs, &ys, &zs);
  sscanf(incvals, "%le,%le,%le", &xinc, &yinc, &zinc);

  /* check input parms */
  if (frames >= GLMOVIEFRAMESNMAX) {
    sprintf(cbuf, "GLMovieSeqGenerate() : Max number of frames (%d) exceeded",
	    GLMOVIEFRAMESNMAX);
    ErrorMsg(cbuf);
    return;
  }
  if ((xinc == 0) && (yinc == 0) && (zinc == 0)) {
    ErrorMsg("GLMovieSeqGenerate() : No rotation angle increment specified");
    return;
  }
  GLMovieNFrames = frames*2;
  MovieType = GLMOVIE;

#if defined (GL_API)
  /* set required GL parameters */
  readsource(SRC_FRONT);

  /* parse the material property arguments if required */
  if (GLObjectType == SHADEDGEOMETRYOBJ) {
    sscanf(SpecularS, "%f,%f,%f",
	   &(material[1]), &(material[2]), &(material[3]));
    sscanf(DiffuseS, "%f,%f,%f",
	   &(material[5]), &(material[6]), &(material[7]));
    sscanf(ShininessS, "%f",
	   &(material[9]));
    lmdef(DEFMATERIAL, 1, 11, material);
  }

  /* compute viewing fustrum */
  InitFustrum(Zoom);

  /* loop to generate frames in increasing angle order */
  ScreenSaverOff();
  for (i=0, xrot=xs, yrot=ys, zrot=zs; i < frames;
       i++, xrot+=xinc, yrot+=yinc, zrot+=zinc) {
    /* draw object */
    pushmatrix();
    RGBcolor(0,0,0);
    clear();
    rotate((Angle) (xrot * 10.0) ,'X');
    rotate((Angle) (yrot * 10.0) ,'Y');
    rotate((Angle) (zrot * 10.0) ,'Z');
    callobj(object);
    popmatrix();
    swapbuffers();
    
    /* record frame */
    if ((GLMovieFrames[i] =
	 (int *) XvgMM_Alloc((void *) GLMovieFrames[i],
			     VWPHeight * VWPWidth * sizeof(int)))
	== NULL) {
      sprintf(cbuf, "GLMovieSeqGenerate() : Could not allocate for frame %d in GLMovieGenerate()", i);
      ErrorMsg(cbuf);
      return;
    }
    items = lrectread(vwp_left, vwp_bottom, vwp_right, vwp_top,
		      (Int32 *) GLMovieFrames[i]);
  }

  /* loop to generate frames in decreasing angle order */
  for (; i < (frames*2); i++, xrot-=xinc, yrot-=yinc, zrot-=zinc) {
    /* draw object */
    pushmatrix();
    RGBcolor(0,0,0);
    clear();
    rotate((Angle) (xrot * 10.0) ,'X');
    rotate((Angle) (yrot * 10.0) ,'Y');
    rotate((Angle) (zrot * 10.0) ,'Z');
    callobj(object);
    popmatrix();
    swapbuffers();
    
    /* record frame */
    if ((GLMovieFrames[i] =
	 (int *) XvgMM_Alloc((void *) GLMovieFrames[i],
			     VWPHeight * VWPWidth * sizeof(int)))
	== NULL) {
      sprintf(cbuf, "GLMovieSeqGenerate() : Could not allocate for frame %d in GLMovieGenerate()", i);
      ErrorMsg(cbuf);
      return;
    }
    items = lrectread(vwp_left, vwp_bottom, vwp_right, vwp_top, (Int32 *) GLMovieFrames[i]);
  }
  ScreenSaverOn();
#endif /* defined (GL_API) */
}

void GLShowFrame(int n)
{
#if defined (GL_API)
  /* write the data from memory into the GL window */
  lrectwrite(vwp_left, vwp_bottom, vwp_right, vwp_top, (Int32 *) GLMovieFrames[n]);
  swapbuffers();

  /* write the data to the VCAA or VCR if required */
  if (VCAWriteOn) {
    /* dump the image to the VCAA */
    VCAWriteGLImage(GLMovieFrames[n], VWPWidth, VWPHeight);
    /* record the frame if required */
    if (VCRRecordOn)
      VCRRecord(VCRFramesPerImage);
  }
#endif /* defined (GL_API) */
}

void SaveGLFrame(char *fname)
{
  /* check that the window is up */
  if (GLShellCreated == B_FALSE) {
    ErrorMsg("GL window not initialized!");
    return;
  }
  
#if defined (GL_API)
  /* load the GL image */
  readsource(SRC_FRONT);
  lrectread(vwp_left, vwp_bottom, vwp_right, vwp_top, GLBuffer);
  WriteGLTiffFile(fname, (int) VWPWidth, (int) VWPHeight,
		  (int *) GLBuffer, (int *) GLScratch);
#endif /* defined (GL_API) */
}

void WriteGLImageToVCAA(void)
{
#if defined (GL_API)
  readsource(SRC_FRONT);
  lrectread(vwp_left, vwp_bottom, vwp_right, vwp_top, GLBuffer);
  VCAWriteGLImage((int *) GLBuffer, (int) VWPWidth, (int) VWPHeight);
#endif /* defined (GL_API) */
}

void GLWriteMovie(char *fname)
{
  FILE *fp;
  char s[256];
  int i, wc;

  /* open the movie file */
  if ((fp = fopen(fname, "w")) == NULL) {
    sprintf(s, "Could not open the file %s", fname);
    ErrorMsg(s);
    return;
  }
  
  /* write the frames */
  for (i = 0; i < GLMovieNFrames; i++) {
    wc = fwrite(GLMovieFrames[i], sizeof(int), VWPHeight*VWPWidth, fp);
    if (wc != (VWPHeight*VWPWidth)) {
      sprintf(s, "Failed writing GL data to %s (%d items written)",
	      fname, VWPHeight*VWPWidth);
      ErrorMsg(s);
      return;
    }
  }
  
  /* close the file */
  fclose(fp);
}


void GLReadMovie(char *fname)
{
  FILE *fp;
  char s[256];
  int i, rc;

  /* open the movie file */
  if ((fp = fopen(fname, "r")) == NULL) {
    sprintf(s, "Could not open the file %s", fname);
    ErrorMsg(s);
    return;
  }
  
  /* read the frames */
  i = 0;
  do {
    if ((GLMovieFrames[i] =
	 (int *) XvgMM_Alloc((void *) GLMovieFrames[i],
			     VWPHeight * VWPWidth * sizeof(int)))
	== NULL) {
      sprintf(s, "Could not allocate memory for frame %d in GLReadMovie()", i);
      ErrorMsg(s);
      return;
    }
    rc = fread(GLMovieFrames[i++], sizeof(int), VWPHeight*VWPWidth, fp);
  } while (rc == VWPHeight*VWPWidth);
  GLMovieNFrames = i-1;  
  
  /* close the file */
  fclose(fp);
}


void GLBuildNurbs(int PixelTolerance,
		  int X, int Y, int Z, char *startvals, char *incvals,
		  int frames, 
		  char *SpecularS, char *DiffuseS, char *ShininessS,
		  double PixelsPerMM)
{
  int i;
  double xmax, xmin, ymax, ymin, zmax, zmin, max, min;

  /* make sure NURBS object defined */
  if (BSplineComputed == B_FALSE) {
    ErrorMsg("No NURBS definition computed from data set!");
    return;
  }

  /* make sure that ProcessedPixels array is loaded with the data */
  if (ProcessedPixels == NULL) {
    ProcMax = RawMax;
    ProcMin = RawMin;
    InitProcBuffers();
    MoveDoubles(RawPixels, ProcessedPixels, ImgHeight*ImgWidth);
  }

#if defined (GL_API)
  /* set NURBS properties */
  setnurbsproperty(N_ERRORCHECKING, 1);
  setnurbsproperty(N_PIXEL_TOLERANCE, (Float32) PixelTolerance);

  /* download NURBS display list */
  BuildNurbsObject(1.0 / PixelsPerMM);

  /* find xyz data extrema to define a cubic viewing fustrum */
  if (DisplayContourComputed) {
    for (i = 0, xmax = -MAXDOUBLE, xmin = MAXDOUBLE,
	 ymax = -MAXDOUBLE, ymin = MAXDOUBLE; i < NContourPoints; i++) {
      xmax = ((ContourPoints[i].x - ImgWidth/2) > xmax ?
	      (ContourPoints[i].x - ImgWidth/2) : xmax);
      ymax = ((ContourPoints[i].y - ImgHeight/2) > ymax ?
	      (ContourPoints[i].y - ImgHeight/2) : ymax);
      xmin = ((ContourPoints[i].x - ImgWidth/2) < xmin ?
	      (ContourPoints[i].x - ImgWidth/2) : xmin);
      ymin = ((ContourPoints[i].y - ImgHeight/2) < ymin ?
	      (ContourPoints[i].y - ImgHeight/2) : ymin);
    }
  }
  else {
    for (i = 0, xmax = -MAXDOUBLE, xmin = MAXDOUBLE; i < BSplineNKi; i++) {
      xmax = ((BSplineKi[i] - ImgWidth/2) > xmax ?
	      (BSplineKi[i] - ImgWidth/2) : xmax);
      xmin = ((BSplineKi[i] - ImgWidth/2) < xmin ?
	      (BSplineKi[i] - ImgWidth/2) : xmin);
    }
    for (i = 0, ymax = -MAXDOUBLE, ymin = MAXDOUBLE; i < BSplineNKj; i++) {
      ymax = ((BSplineKj[i] - ImgHeight/2) > ymax ?
	      (BSplineKj[i] - ImgHeight/2) : ymax);
      ymin = ((BSplineKj[i] - ImgHeight/2) < ymin ?
	      (BSplineKj[i] - ImgHeight/2) : ymin);
    }
  }
  xmax /= PixelsPerMM;
  xmin /= PixelsPerMM;
  ymax /= PixelsPerMM;
  ymin /= PixelsPerMM;
  for (i = 0, zmax = -MAXDOUBLE, zmin = MAXDOUBLE;
       i < (ImgHeight * ImgWidth); i++) {
    zmax = (ProcessedPixels[i] > zmax ? ProcessedPixels[i] : zmax);
    zmin = (ProcessedPixels[i] < zmin ? ProcessedPixels[i] : zmin);
  }
  max = (xmax > ymax ? xmax : ymax);
  min = (xmin < ymin ? xmin : ymin);
  max = (zmax > max ? zmax : max);
  min = (zmin < min ? zmin : min);
  max = (max > 0 ? max*1.2 : max*0.8);
  min = (min > 0 ? min*0.8 : min*1.2);
  ViewingTransformDefined = B_TRUE;

  if (VerboseFlag) {
    printf("xmax:%e, xmin:%e\n", xmax, xmin);
    printf("ymax:%e, ymin:%e\n", ymax, ymin);
    printf("zmax:%e, zmin:%e\n", zmax, zmin);
    printf(" max:%e,  min:%e\n", max, min);
    printf("Pixels/MM:%f, tolerance:%f\n", PixelsPerMM, PixelTolerance);
  }
#endif /* defined (GL_API) */
}

void BuildNurbsObject(double MMPerPixel)
{
  static struct VERTEX {
    Float64 x;
    Float64 y;
    Float64 z;
    Float64 w;
  } *ctlarray;

  static struct CONTOUR {
    Float64 s;
    Float64 t;
    Float64 w;
  } *contour;

  static Float64 *s_knots, *t_knots;
  static Int32 s_knot_count, t_knot_count, s_stride, t_stride;

  int i, j, k, N, JOB, IFAIL;
  double *D, *E, *PX, *PY;

  /* allocate storage for the NAG band matrix solving calls */
  JOB = 0;
  D = (double *) UxMalloc(((BSplineNKi > BSplineNKj ? BSplineNKi : BSplineNKj)
			 - 4) * sizeof(double));
  E = (double *) UxMalloc(((BSplineNKi > BSplineNKj ? BSplineNKi : BSplineNKj)
			 - 4) * sizeof(double));
  PX = (double *) UxMalloc(((BSplineNKi > BSplineNKj ? BSplineNKi : BSplineNKj)
			  - 4) * sizeof(double));
  PY = (double *) UxMalloc(((BSplineNKi > BSplineNKj ? BSplineNKi : BSplineNKj)
			  - 4) * sizeof(double));
  
  /* solve the BAND matrix to generate the X coords of the control polygon */
  N = BSplineNKi - 4;
  for (i = 0; i < N; i++)
    D[i] = 4.0/6.0;
  for (i = 0; i < N; i++)
    E[i] = 1.0/6.0;
  for (i = 0; i < N; i++)
    PX[i] = (BSplineKi[i+2] - ((double) ImgWidth / 2)) * MMPerPixel;
  IFAIL = 0;
  f04faf(&JOB, &N, D, E, PX, &IFAIL);
  
  /* solve the BAND matrix to generate the Y coords of the control polygon */
  N = BSplineNKj - 4;
  for (i = 0; i < N; i++)
    D[i] = 4.0/6.0;
  for (i = 0; i < N; i++)
    E[i] = 1.0/6.0;
  for (i = 0; i < N; i++)
    PY[i] = (BSplineKj[i+2] - ((double) ImgHeight / 2)) * MMPerPixel;
  IFAIL = 0;
  f04faf(&JOB, &N, D, E, PY, &IFAIL);
  
  /* initialize NURBS object */
  s_knot_count = BSplineNKi;
  t_knot_count = BSplineNKj;
  UxFree(s_knots);
  s_knots = (Float64 *) UxMalloc(BSplineNKi * sizeof(Float64));
  UxFree(t_knots);
  t_knots = (Float64 *) UxMalloc(BSplineNKj * sizeof(Float64));
  s_stride = sizeof(struct VERTEX);
  t_stride = s_stride * (BSplineNKi-4);

  /* create dummy control point arrays in s,t space (pixel space) */
  for (i = 0; i < BSplineNKi; i++)
    s_knots[i] = BSplineKi[i];
  for (i = 0; i < BSplineNKj; i++)
    t_knots[i] = BSplineKj[i];
  
  /* generate the control polygon verticies from the NAG functional form */
  UxFree(ctlarray);
  ctlarray = (struct VERTEX *) UxMalloc((BSplineNKi-4) * (BSplineNKj-4)
				      * sizeof(struct VERTEX));

  /* memory order check */
  ctlarray[0].x = 0;
  ctlarray[0].y = 1;
  ctlarray[0].z = 2;
  ctlarray[0].w = 3;
  if ((((double *) ctlarray)[0] != 0) || (((double *) ctlarray)[1] != 1)
      || (((double *) ctlarray)[2] != 2) || (((double *) ctlarray)[3] != 3)) {
    ErrorMsg("Memory mapping order is incorrect!");
    return;
  }

  /* fill Xij elements with the NAG result, row by row */
  for (j = 0; j < BSplineNKj-4; j++)
    for (i = 0; i < BSplineNKi-4; i++)
      ctlarray[j*(BSplineNKi-4) + i].x = PX[i]; 
/*
      ctlarray[j*(BSplineNKi-4) + i].x =
	(BSplineKi[i+2] - ((double) ImgWidth / 2)) * MMPerPixel; 
*/

  /* fill Yij elements with the NAG result, column by column */
  for (i = 0; i < BSplineNKi-4; i++)
    for (j = 0; j < BSplineNKj-4; j++)
      ctlarray[j*(BSplineNKi-4) + i].y = PY[j];
/*
      ctlarray[j*(BSplineNKi-4) + i].y =
	(BSplineKj[j+2] - ((double) ImgHeight / 2)) * MMPerPixel;
*/

  /* z coordinate & w coordinate */
  for (j = 0, k = 0; j < BSplineNKj-4; j++)
    for (i = 0; i < BSplineNKi-4; i++, k++) {
      ctlarray[k].z = BSplineCij[k];
/*
      ctlarray[k].z = BSplineCij[k] * ImgWidth * ImgHeight
	/((BSplineKj[j+4] - BSplineKj[j]) * (BSplineKi[i+4] - BSplineKi[i]));
*/
      ctlarray[k].w = 1.0;
    }

  /* define contour array if required */
  if (DisplayContourComputed) {
    UxFree(contour);
    contour = (struct CONTOUR *) UxMalloc(NContourPoints
					* sizeof(struct CONTOUR));
    for (i = 0; i < NContourPoints; i++) {
      contour[i].s = ContourPoints[i].x;
      contour[i].t = ContourPoints[i].y;
      contour[i].w = 1.0;
    }
  }
  else {
    UxFree(contour);
    contour = NULL;
  }

#if defined (GL_API)
  makeobj(object = genobj());
  bgnsurface();
  nurbssurface((Int32) s_knot_count, (Float64 *) s_knots, (Int32) t_knot_count, (Float64 *) t_knots, (Int32) s_stride,
	       (Int32) t_stride, (Float64 *) ctlarray, (Int32) 4, (Int32) 4, (Int32) N_STW);
  if (contour != NULL) {
    bgntrim();
    pwlcurve((Int32) NContourPoints, (Float64 *) contour, (Int32) sizeof(struct CONTOUR), (Int32) N_STW);
    endtrim();
  }
  endsurface();
  closeobj();

  UxFree(D); UxFree(E); UxFree(PX); UxFree(PY);
  ObjectDefined = B_TRUE;
#endif /* defined (GL_API) */
}



