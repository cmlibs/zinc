/*******************************************************************************
FILE : graphics_library.h

LAST MODIFIED : 12 September 2002

DESCRIPTION :
Functions and structures for interfacing with the graphics library.
==============================================================================*/
#if !defined (GRAPHICS_LIBRARY_H)
#define GRAPHICS_LIBRARY_H

#if defined (GL_API)
#if defined (MOTIF)
/* needed because X and GL use some of the same names and if X isn't included
	before GL there are errors */
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <Mrm/MrmPublic.h>
#endif /* defined (MOTIF) */
#include <gl/gl.h>
#endif
#if defined (OPENGL_API)
#define GL_GLEXT_PROTOTYPES
#define GLX_GLXEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glu.h>
#if defined (WIN32_SYSTEM)
/* SAB On Win32 I think you have to load all OpenGL 1.2, 1.3 etc functions
	as extensions and keep pointer references to them.  I haven't done this
	yet so we will undefine the version symbols */
#undef GL_NV_vertex_program
#undef GL_NV_register_combiners2
#endif /* defined (WIN32_SYSTEM) */
#endif
#if defined (DECPHIGS_API)
#include <phigs.h>
#endif
#if defined (graPHIGS_API)
#include "phigs.h"
#endif

struct User_interface;

/*
Global types
------------
*/

typedef float gtMatrix[4][4];

enum Texture_storage_type
/*******************************************************************************
LAST MODIFIED : 28 February 2002

DESCRIPTION :
==============================================================================*/
{
	TEXTURE_LUMINANCE,
	TEXTURE_LUMINANCE_ALPHA,
	TEXTURE_RGB,
	TEXTURE_RGBA,
	TEXTURE_ABGR,
	/* The last two types are special and are not user-selectable */
	TEXTURE_DMBUFFER,
	TEXTURE_PBUFFER
}; /* enum Texture_storage_type */

/*
Global variables
----------------
*/
extern int graphics_library_open;

/*
Global functions
----------------
*/

int open_graphics_library(void);
/*******************************************************************************
LAST MODIFIED : 23 March 1993

DESCRIPTION :
Function to open the PHIGS graphics library.
==============================================================================*/

int close_graphics_library(void);
/*******************************************************************************
LAST MODIFIED : 23 March 1993

DESCRIPTION :
Function to close the PHIGS graphics library.
==============================================================================*/

int initialize_graphics_library(struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 14 November 1996

DESCRIPTION :
Sets up the default light, material and light model for the graphics library.
==============================================================================*/

int gtMatrix_is_identity(gtMatrix *matrix);
/*******************************************************************************
LAST MODIFIED : 28 November 2001

DESCRIPTION :
Returns true if <matrix> is the 4x4 identity
==============================================================================*/

int gtMatrix_match(gtMatrix *matrix1, gtMatrix *matrix2);
/*******************************************************************************
LAST MODIFIED : 28 November 2001

DESCRIPTION :
Returns true if <matrix1> and <matrix2> are identical.
==============================================================================*/

int gtMatrix_match_with_tolerance(gtMatrix *matrix1, gtMatrix *matrix2,
	float tolerance);
/*******************************************************************************
LAST MODIFIED : 5 December 2001

DESCRIPTION :
Returns true if <matrix1> and <matrix2> have no components different by
more than <tolerance> times the largest absolute value in either matrix.
==============================================================================*/

int gtMatrix_to_euler(gtMatrix matrix, float *euler_angles);
/*******************************************************************************
LAST MODIFIED : 21 November 2002

DESCRIPTION :
Cleaned this up from view/coord_trans.c
Returns <euler_angles> in radians.
==============================================================================*/

int euler_to_gtMatrix(float *euler_angles, gtMatrix matrix);
/*******************************************************************************
LAST MODIFIED : 21 November 2002

DESCRIPTION :
Cleaned this up from view/coord_trans.c
<euler_angles> are in radians.
==============================================================================*/

#if defined (OPENGL_API)
void wrapperReadMatrix(GLenum matrixName,gtMatrix *theMatrix);
/*******************************************************************************
LAST MODIFIED : 13 February 1995

DESCRIPTION :
==============================================================================*/

void wrapperLoadCurrentMatrix(gtMatrix *theMatrix);
/*******************************************************************************
LAST MODIFIED : 13 February 1995

DESCRIPTION :
==============================================================================*/

void wrapperMultiplyCurrentMatrix(gtMatrix *theMatrix);
/*******************************************************************************
LAST MODIFIED : 13 February 1995

DESCRIPTION :
==============================================================================*/

void wrapperWindow(float left,float right,float bottom,float top,float near,
	float far);
/*******************************************************************************
LAST MODIFIED : 13 February 1995

DESCRIPTION :
==============================================================================*/

void wrapperPolarview(float A,float B,float C,float D);
/*******************************************************************************
LAST MODIFIED : 13 February 1995

DESCRIPTION :
==============================================================================*/

void wrapperPerspective(float A,float B,float C,float D);
/*******************************************************************************
LAST MODIFIED : 13 February 1995

DESCRIPTION :
==============================================================================*/

void wrapperOrtho(float a,float b,float c,float d,float e,float f);
/*******************************************************************************
LAST MODIFIED : 13 February 1995

DESCRIPTION :
==============================================================================*/

void wrapperOrtho2D(float a,float b,float c,float d);
/*******************************************************************************
LAST MODIFIED : 13 February 1995

DESCRIPTION :
==============================================================================*/

void wrapperPrintText(char *theText);
/*******************************************************************************
LAST MODIFIED : 13 February 1995

DESCRIPTION :
==============================================================================*/

void wrapperInitText(struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 14 November 1996

DESCRIPTION :
==============================================================================*/
#endif

#if defined (OPENGL_API)
int query_gl_extension(char *extName);
/*******************************************************************************
LAST MODIFIED : 9 September 1998

DESCRIPTION :
Search for extName in the GL extensions string. Use of strstr() is not sufficient
because extension names can be prefixes of other extension names. Could use
strtok() but the constant string returned by glGetString might be in read-only
memory.
???SAB.  Taken directly from the insight book on OpenGL Extensions
==============================================================================*/
#endif /* defined (OPENGL_API) */

#if defined (OPENGL_API)
int query_gl_version(int major_version, int minor_version);
/*******************************************************************************
LAST MODIFIED : 16 April 2003

DESCRIPTION :
Returns true if the OpenGL version is at least <major_version>.<minor_version>
==============================================================================*/
#endif /* defined (OPENGL_API) */

int Graphics_library_read_pixels(unsigned char *frame_data,
	int width, int height, enum Texture_storage_type storage);
/*******************************************************************************
LAST MODIFIED : 12 September 2002

DESCRIPTION :
Read pixels from the current graphics context into <frame_data> of size <width>
and <height> according to the storage type.  'MakeCurrent' the desired source 
before calling this routine.
==============================================================================*/

int Graphics_library_load_extension(char *extensions);
/*******************************************************************************
LAST MODIFIED : 20 February 2004

DESCRIPTION :
Attempts to load the space separated list of extensions.  Returns true if all
the extensions succeed, false if not.
==============================================================================*/

int Graphics_library_load_extensions(char *extensions);
/*******************************************************************************
LAST MODIFIED : 20 February 2004

DESCRIPTION :
Attempts to load the space separated list of extensions.  Returns true if all
the extensions succeed, false if not.
==============================================================================*/

#if defined (OPENGL_API)
   /* On UNIX systems we just test if we can load the handles but call the functions
	  directly.  On Win32 (and AIX I think) we need to use function ptrs to call each time */
#  if defined (WIN32_SYSTEM) || defined (AIX)
#    define GRAPHICS_LIBRARY_USE_EXTENSION_FUNCTION_HANDLES
#  endif /* defined (WIN32_SYSTEM) || defined (AIX) */

#  define GLEXTENSIONFLAG( extension_name ) extension_name ## _glextension_flag
#  define Graphics_library_check_extension(extension_name) \
	(255 == extension_name ## _glextension_flag ? Graphics_library_load_extension( #extension_name) : extension_name ## _glextension_flag)
#if defined (GRAPHICS_LIBRARY_USE_EXTENSION_FUNCTION_HANDLES)
#  define GLHANDLE( function_name ) function_name ## _handle
#endif /* defined GRAPHICS_LIBRARY_USE_EXTENSION_FUNCTION_HANDLES */

#if defined (GRAPHICS_LIBRARY_C)
/* This is being included from the C file so do the initialisations */
#  define GRAPHICS_LIBRARY_INITIALISE_GLEXTENSIONFLAG(extension_name) \
	unsigned char extension_name ## _glextension_flag = 255
#  define GRAPHICS_LIBRARY_EXTERN
#else /* defined (GRAPHICS_LIBRARY_C) */
#  define GRAPHICS_LIBRARY_INITIALISE_GLEXTENSIONFLAG(extension_name) \
	extern unsigned char extension_name ## _glextension_flag
#  define GRAPHICS_LIBRARY_EXTERN extern
#endif /* defined (GRAPHICS_LIBRARY_C) */

/* Extension flags */
#if defined (GLX_ARB_get_proc_address)
  GRAPHICS_LIBRARY_INITIALISE_GLEXTENSIONFLAG(GLX_ARB_get_proc_address);
#endif /* defined (GLX_ARB_get_proc_address) */
#if defined (GL_VERSION_1_2)
  GRAPHICS_LIBRARY_INITIALISE_GLEXTENSIONFLAG(GL_VERSION_1_2);
#endif /* defined (GL_VERSION_1_2) */
#if defined (GL_VERSION_1_3)
  GRAPHICS_LIBRARY_INITIALISE_GLEXTENSIONFLAG(GL_VERSION_1_3);
#endif /* defined (GL_VERSION_1_3) */
#if defined (GL_VERSION_1_4)
  GRAPHICS_LIBRARY_INITIALISE_GLEXTENSIONFLAG(GL_VERSION_1_4);
#endif /* defined (GL_VERSION_1_4) */
#if defined (GL_EXT_texture3D)
  GRAPHICS_LIBRARY_INITIALISE_GLEXTENSIONFLAG(GL_EXT_texture3D);
#endif /* defined (GL_EXT_texture3D) */
#if defined (GL_EXT_abgr)
  GRAPHICS_LIBRARY_INITIALISE_GLEXTENSIONFLAG(GL_EXT_abgr);
#endif /* defined (GL_EXT_abgr) */
#if defined (GL_ARB_texture_compression)
  GRAPHICS_LIBRARY_INITIALISE_GLEXTENSIONFLAG(GL_ARB_texture_compression);
#endif /* defined (GL_ARB_texture_compression) */
#if defined (GL_ARB_vertex_program)
  GRAPHICS_LIBRARY_INITIALISE_GLEXTENSIONFLAG(GL_ARB_vertex_program);
#endif /* defined (GL_ARB_vertex_program) */
#if defined (GL_ARB_fragment_program)
  GRAPHICS_LIBRARY_INITIALISE_GLEXTENSIONFLAG(GL_ARB_fragment_program);
#endif /* defined (GL_ARB_fragment_program) */

/* Extension function handles */
#if defined (GRAPHICS_LIBRARY_USE_EXTENSION_FUNCTION_HANDLES)
#  if defined (APIENTRY)
   /* Testing to see if we are using Mesa like headers (NVIDIA defined APIENTRY and not APIENTRYP) */

#    if defined (GL_VERSION_1_2) || defined (GL_EXT_texture3D)
       /* Note that while to strictly satisfy the GL_EXT_texture3D 
	    this function would have the EXT delimiter the SGI
	    OpenGL is version 1.1 with GL_EXT_texture3D but supplies
	    all the 1.2 compliant symbols, so the code is simpler with
	    just one version */
       GRAPHICS_LIBRARY_EXTERN PFNGLTEXIMAGE3DPROC GLHANDLE(glTexImage3D);
#      define glTexImage3D (GLHANDLE(glTexImage3D))
#    endif /* defined (GL_VERSION_1_2) || defined (GL_EXT_texture3D) */

#    if defined (GL_VERSION_1_3)
       GRAPHICS_LIBRARY_EXTERN PFNGLACTIVETEXTUREPROC GLHANDLE(glActiveTexture);
#      define glActiveTexture (GLHANDLE(glActiveTexture))
       GRAPHICS_LIBRARY_EXTERN PFNGLMULTITEXCOORD3FVPROC GLHANDLE(glMultiTexCoord3fv);
#      define glMultiTexCoord3fv (GLHANDLE(glMultiTexCoord3fv))
#    endif /* defined (GL_VERSION_1_3) */

#    if defined (GL_VERSION_1_4)
       GRAPHICS_LIBRARY_EXTERN PFNGLBLENDFUNCSEPARATEPROC GLHANDLE(glBlendFuncSeparate);
#      define glBlendFuncSeparate (GLHANDLE(glBlendFuncSeparate))
#    endif /* defined (GL_VERSION_1_4) */

#    if defined (GL_ARB_vertex_program) || defined (GL_ARB_fragment_program)
       GRAPHICS_LIBRARY_EXTERN PFNGLGENPROGRAMSARBPROC GLHANDLE(glGenProgramsARB);
#      define glGenProgramsARB (GLHANDLE(glGenProgramsARB))
       GRAPHICS_LIBRARY_EXTERN PFNGLBINDPROGRAMARBPROC GLHANDLE(glBindProgramARB);
#      define glBindProgramARB (GLHANDLE(glBindProgramARB))
       GRAPHICS_LIBRARY_EXTERN PFNGLPROGRAMSTRINGARBPROC GLHANDLE(glProgramStringARB);
#      define glProgramStringARB (GLHANDLE(glProgramStringARB))
       GRAPHICS_LIBRARY_EXTERN PFNGLDELETEPROGRAMSARBPROC GLHANDLE(glDeleteProgramsARB);
#      define glDeleteProgramsARB (GLHANDLE(glDeleteProgramsARB))
#    endif /* defined (GL_ARB_vertex_program) || defined (GL_ARB_fragment_program) */
#  else  /* defined (APIENTRY) */
     /* We don't have types for the functions so we will not use any extensions */
#    undef GL_VERSION_1_2
#    undef GL_VERSION_1_3
#    undef GL_VERSION_1_4
#    undef GL_EXT_texture3D
#    undef GL_ARB_vertex_program
#    undef GL_ARB_fragment_program
#  endif  /* defined (APIENTRY) */
#endif /* defined GRAPHICS_LIBRARY_USE_EXTENSION_FUNCTION_HANDLES */

#endif /* defined (OPENGL_API) */


#endif
