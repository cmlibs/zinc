/*******************************************************************************
FILE : dm_interface.c

LAST MODIFIED : 28 October 1998

DESCRIPTION :
This provides a Cmgui interface to the Digital Media libraries on the SGI
******************************************************************************/
/* These calls should be available in every system with GLX 1.3 or greater.
	The code should still run on an older GLX even if it is compiled on a GLX 1.3.
	Undefine if you need to compile on an older GLX. */
#define GLX_pbuffer 1
#define GLX_fbconfig 1

#if defined (SGI_DIGITAL_MEDIA)
/*???SAB.  This is defined here because needed in a header file */
	/*???DB.  Which header ?  Must be defined somewhere ? */
#if !defined (_STAMP_T)
#define _STAMP_T
typedef signed long long stamp_t; /* used for USTs and often MSCs */
typedef struct USTMSCpair
{
	stamp_t ust; /* a UST value at which an MSC hit an input or output jack */
	stamp_t msc; /* the MSC which is being described */
} USTMSCpair;
#endif /* !defined (_STAMP_T) */
#if _MIPS_SZLONG == 64
/* SAB  Seems that the 64bit headers are not compiling quite right so I'm
	setting a flag to force it to include a needed declaration */
#define _BSD_TYPES
#endif /* _MIPS_SZLONG == 64 */
#include <X11/Xlib.h>
#define DM_BUFFER_PRIVATE_FUNCTIONS
#include <dmedia/dm_buffer.h>
#include <dmedia/dm_image.h>
#endif /* defined (SGI_DIGITAL_MEDIA) */
#include <GL/glx.h>
#include <GL/gl.h>
#if !defined (SGI_DIGITAL_MEDIA)
/* On the 64Bit version this is defined but the libraries 
	do not exist so I have undefined it so that the file compiles correctly */
#undef GLX_SGIX_dm_pbuffer
#endif /* !defined (SGI_DIGITAL_MEDIA) */
#include <stdio.h>
#include <X11/Xatom.h>
#include <X11/IntrinsicP.h>
#include <X11/StringDefs.h>
#include <X11/Xresource.h>
#include "three_d_drawing/ThreeDDraP.h"
#include "user_interface/user_interface.h"
#include "general/debug.h"
#include "general/object.h"
#include "three_d_drawing/dm_interface.h"
#include "user_interface/message.h"

struct Dm_buffer
{
	struct User_interface *user_interface;
#if defined (GLX_SGIX_dm_pbuffer)
	DMbuffer dmbuffer;
	DMbufferpool dmpool;
#endif /* defined (GLX_SGIX_dm_pbuffer) */
	GLXContext context;
#if defined (GLX_pbuffer)
	GLXPbuffer pbuffer;
#else /* defined (GLX_pbuffer) */
#if defined (GLX_SGIX_dm_pbuffer) || (GLX_SGIX_pbuffer)
	GLXPbufferSGIX pbuffer;
#endif /* defined (GLX_SGIX_dm_pbuffer) || (GLX_SGIX_pbuffer) */
#endif /* defined (GLX_pbuffer) */
#if defined (GLX_fbconfig)
	GLXFBConfig config;
#else /* defined (GLX_fbconfig) */
#if defined (GLX_SGIX_fbconfig)
	GLXFBConfigSGIX config;
#endif /* defined (GLX_SGIX_fbconfig) */
#endif /* defined (GLX_fbconfig) */
	XVisualInfo *visual_info;
	Pixmap pixmap;
	GLXPixmap glx_pixmap;

	int access_count;
};

DECLARE_OBJECT_FUNCTIONS(Dm_buffer)

struct Dm_buffer *CREATE(Dm_buffer)(int width, int height, int depth_buffer,
	int shared_display_buffer, struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 23 June 2000

DESCRIPTION :
The <depth_buffer> flag specifies whether the visual selected is requested to
have a depth buffer or not.
The <shared_display_buffer> flag specifies whether the created buffer must
share display_lists with the main_display or not.  If not then a pixel buffer
supported on displays other than SGI will do.
==============================================================================*/
{
	struct Dm_buffer *buffer;
#if defined (GLX_SGIX_dm_pbuffer)
	int count = 1;
#endif /* defined (GLX_SGIX_dm_pbuffer) */
#if defined (GLX_SGIX_dm_pbuffer)
	DMparams *imageFormat, *poolSpec;
	DMboolean cacheable = DM_FALSE;
	DMboolean mapped = DM_FALSE;
	int dm_pbuffer_attribs [] = 
	{
		GLX_DIGITAL_MEDIA_PBUFFER_SGIX, True,
		GLX_PRESERVED_CONTENTS_SGIX, True,
		(int) None
	};
#endif /* defined (GLX_SGIX_dm_pbuffer) */
#if defined (GLX_pbuffer)
	int pbuffer_attribs [] = 
	{
		GLX_PBUFFER_WIDTH, 0, /* Note that these 0 values are explictly overwritten below */
		GLX_PBUFFER_HEIGHT, 0,
		GLX_PRESERVED_CONTENTS, True,
		(int) None
	};
	int glx_major_version, glx_minor_version, nelements;
#else /* defined (GLX_pbuffer) */
#if defined (GLX_SGIX_dm_pbuffer) || (GLX_SGIX_pbuffer)
	int pbuffer_attribs [] = 
	{
		GLX_PRESERVED_CONTENTS_SGIX, True,
		(int) None
	};
#endif /* defined (GLX_SGIX_dm_pbuffer) || (GLX_SGIX_pbuffer) */
#endif /* defined (GLX_pbuffer) */
#if defined (GLX_fbconfig)
	GLXFBConfig *config_list;
	static int fbvisattrsRGB_with_depth[] =
	{
		GLX_RED_SIZE, 5,
		GLX_GREEN_SIZE, 5,
		GLX_BLUE_SIZE, 5,
		GLX_DEPTH_SIZE, 5,
		None
	};
	static int fbvisattrsRGB_no_depth[] =
	{
		GLX_RED_SIZE, 5,
		GLX_GREEN_SIZE, 5,
		GLX_BLUE_SIZE, 5,
		None
	};
	int *fbvisattrs;
#endif /* defined (GLX_fbconfig) */
	static int visattrsRGB_with_depth[] =
	{
		GLX_RGBA,
		GLX_RED_SIZE, 5,
		GLX_GREEN_SIZE, 5,
		GLX_BLUE_SIZE, 5,
		GLX_ALPHA_SIZE, 1,
		GLX_DEPTH_SIZE, 5,
		None
	};
	static int visattrsRGB_no_depth[] =
	{
		GLX_RGBA,
		GLX_RED_SIZE, 5,
		GLX_GREEN_SIZE, 5,
		GLX_BLUE_SIZE, 5,
		GLX_ALPHA_SIZE, 1,
		None
	};
	int *visattrs;

	ENTER(CREATE(Dm_buffer));
#if defined (GLX_pbuffer)
	USE_PARAMETER(shared_display_buffer);
#endif /* defined (GLX_pbuffer) */

	if(user_interface)
	{
		if (depth_buffer)
		{
			visattrs = visattrsRGB_with_depth;
#if defined (GLX_fbconfig)
			fbvisattrs = fbvisattrsRGB_with_depth;
#endif /* defined (GLX_fbconfig) */
		}
		else
		{
			visattrs = visattrsRGB_no_depth;
#if defined (GLX_fbconfig)
			fbvisattrs = fbvisattrsRGB_no_depth;
#endif /* defined (GLX_fbconfig) */
		}
		if (ALLOCATE(buffer, struct Dm_buffer, 1))
		{
			buffer->user_interface = user_interface;
#if defined (GLX_SGIX_dm_pbuffer)
			buffer->dmbuffer = (DMbuffer)NULL;
			buffer->dmpool = (DMbufferpool)NULL;
#endif /* defined (GLX_SGIX_dm_pbuffer) */
			buffer->context = (GLXContext)NULL;
#if defined (GLX_pbuffer)
			buffer->pbuffer = (GLXPbuffer)NULL;
			/* On an SGI it is invalid to use the function parameters in
				the structure initialisation */
			pbuffer_attribs[1] = width;
			pbuffer_attribs[3] = height;
#else /* defined (GLX_pbuffer) */
#if defined (GLX_SGIX_dm_pbuffer) || (GLX_SGIX_pbuffer)
			buffer->pbuffer = (GLXPbufferSGIX)NULL;
#endif /* defined (GLX_SGIX_dm_pbuffer) || (GLX_SGIX_pbuffer) */
#endif /* defined (GLX_pbuffer) */
#if defined (GLX_SGIX_fbconfig)
			buffer->config = (GLXFBConfigSGIX)NULL;;
#endif /* defined (GLX_SGIX_fbconfig) */
#if defined (GLX_fbconfig)
			buffer->config = (GLXFBConfig)NULL;;
#endif /* defined (GLX_fbconfig) */
			buffer->visual_info = (XVisualInfo *)NULL;
			buffer->pixmap = (Pixmap)NULL;
			buffer->glx_pixmap = (GLXPixmap)NULL;
			buffer->access_count = 0;

#if defined (GLX_SGIX_dm_pbuffer)
			if(query_glx_extension("GLX_SGIX_dm_pbuffer", user_interface->display,
				DefaultScreen(user_interface->display)))
			{
				if((DM_SUCCESS==dmParamsCreate( &imageFormat ))
					&& (DM_SUCCESS==dmSetImageDefaults( imageFormat,
						width, height, DM_IMAGE_PACKING_RGBA ))
					/*				&& (DM_SUCCESS==dmParamsSetEnum( imageFormat,
									DM_IMAGE_ORIENTATION, DM_TOP_TO_BOTTOM ))*/
					&& (DM_SUCCESS==dmParamsSetEnum( imageFormat,
						DM_IMAGE_LAYOUT, DM_IMAGE_LAYOUT_GRAPHICS )))
				{
					if((DM_SUCCESS==dmParamsCreate(&poolSpec))
						&& (DM_SUCCESS==dmBufferSetPoolDefaults( poolSpec,
							count, 0, cacheable, mapped ))
						&& (DM_SUCCESS==dmBufferGetGLPoolParams( imageFormat,
							poolSpec )))
					{
						if ((DM_SUCCESS==dmBufferCreatePool(poolSpec, &(buffer->dmpool)))
							&&(DM_SUCCESS==dmBufferAllocate(buffer->dmpool,
								&(buffer->dmbuffer))))
						{
							if((buffer->visual_info = glXChooseVisual(user_interface->display,
								DefaultScreen(user_interface->display), visattrs))
								&& (buffer->config  = glXGetFBConfigFromVisualSGIX(
									user_interface->display, buffer->visual_info)))
							{
								printf("CREATE(Dm_buffer). openGL visual id = %d\n",
									(int)buffer->visual_info->visualid);
								if (buffer->pbuffer = glXCreateGLXPbufferSGIX(user_interface->display,
									buffer->config, width, height, dm_pbuffer_attribs))
								{
									if (buffer->context = glXCreateContextWithConfigSGIX(
										user_interface->display, buffer->config, GLX_RGBA_TYPE_SGIX, 
										ThreeDDrawing_get_shareable_context(), GL_TRUE))
									{
										if (glXAssociateDMPbufferSGIX(user_interface->display,
											buffer->pbuffer, imageFormat, buffer->dmbuffer))
										{
											/* Finished I think, hooray! */
#if defined (DEBUG)
											{
												/* debug */
												char *pixels, *pixel_pointer;
												int bytes_per_pixel = 4, i;
												char red = 50, green = 50, blue = 0;
												if (glXMakeCurrent(user_interface->display, buffer->pbuffer, buffer->context))
												{
													if (pixels = (char *)malloc(width * height * bytes_per_pixel))
													{
														glClearColor((double)red/255.0, (double)green/255.0,
															(double)blue/255.0, 1.0);
														glClearDepth(1.0);
														glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
														glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE,
															pixels);
														pixel_pointer = pixels;
														for (i = 0 ; i < width * height ; i++)
														{
															if (!(i % 1000))
															{
																printf ("%d    %d %d %d\n", i, pixel_pointer[0],
																	pixel_pointer[1], pixel_pointer[2]);
															}
															if ((pixel_pointer[0] != red) || (pixel_pointer[1] != green) ||
																(pixel_pointer[2] != blue))
															{
																printf ("ERROR %d   %d %d %d\n", i, pixel_pointer[0],
																	pixel_pointer[1], pixel_pointer[2]); 
															}
															pixel_pointer += bytes_per_pixel;
														}
														free(pixels);
													}
												}
											}
#endif /* defined (DEBUG) */
										}
										else
										{
											display_message(ERROR_MESSAGE,"CREATE(Dm_buffer). Unable to associate pbuffer with dmbuffer");
											DEALLOCATE(buffer);
											buffer = (struct Dm_buffer *)NULL;
										}
									}
									else
									{
										display_message(ERROR_MESSAGE,"CREATE(Dm_buffer). Cannot get GLX context");
										DEALLOCATE(buffer);
										buffer = (struct Dm_buffer *)NULL;
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,"CREATE(Dm_buffer). Cannot create pbuffer");
									DEALLOCATE(buffer);
									buffer = (struct Dm_buffer *)NULL;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,"CREATE(Dm_buffer). Cannot get Frame Buffer Configuration");
								DEALLOCATE(buffer);
								buffer = (struct Dm_buffer *)NULL;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"CREATE(Dm_buffer). Cannot allocate Dmbuffer");
							DEALLOCATE(buffer);
							buffer = (struct Dm_buffer *)NULL;
						}
						dmParamsDestroy(poolSpec);
					}
					else
					{
						display_message(ERROR_MESSAGE,"CREATE(Dm_buffer). Cannot create DM Pool Parameters");
						DEALLOCATE(buffer);
						buffer = (struct Dm_buffer *)NULL;
					}
					dmParamsDestroy(imageFormat);
				}
				else
				{
					display_message(ERROR_MESSAGE,"CREATE(Dm_buffer). Cannot create image parameters");
					DEALLOCATE(buffer);
					buffer = (struct Dm_buffer *)NULL;
				}
			}
			else
			{
#endif /* defined (GLX_SGIX_dm_pbuffer) */
#if defined (GLX_pbuffer)
				/* SAB 12 December 2001.  This should be only be a test on the version number 
					of GLX (>=1.3) but the NVIDIA card drivers are not reporting the correct version
					and so I am assuming that if this extension is available then the GLX 1.3 stuff
					is also in.  Hopefully this won't last long */
				if ((ThreeDDrawing_get_glx_version(&glx_major_version, &glx_minor_version) && 
					((glx_major_version > 1) || (glx_minor_version > 2)))
					|| query_glx_extension("GLX_SGIX_pbuffer", user_interface->display,
					DefaultScreen(user_interface->display)))
				{
					if ((config_list = glXChooseFBConfig(user_interface->display, 
						DefaultScreen(user_interface->display), fbvisattrs, &nelements)) && 
						(buffer->config = *config_list) &&
						(buffer->visual_info = glXGetVisualFromFBConfig(
						user_interface->display, buffer->config)))
					{
						if (buffer->pbuffer = glXCreatePbuffer(user_interface->display,
							buffer->config, pbuffer_attribs))
						{
							if (buffer->context = glXCreateNewContext(
								user_interface->display, buffer->config, GLX_RGBA_TYPE, 
								ThreeDDrawing_get_shareable_context(), GL_TRUE))
							{
								/* Finished I think, hooray! */
							}
							else
							{
								display_message(ERROR_MESSAGE,"CREATE(Dm_buffer). Cannot get GLX context");
								DEALLOCATE(buffer);
								buffer = (struct Dm_buffer *)NULL;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"CREATE(Dm_buffer). Cannot create pbuffer");
							DEALLOCATE(buffer);
							buffer = (struct Dm_buffer *)NULL;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"CREATE(Dm_buffer). Cannot get Frame Buffer Configuration");
						DEALLOCATE(buffer);
						buffer = (struct Dm_buffer *)NULL;
					}
				}
				else
				{
#else /* defined (GLX_pbuffer) */
				/* Superseded by the GLX 1.3 code above */
#if defined (GLX_SGIX_pbuffer)
				if(query_glx_extension("GLX_SGIX_pbuffer", user_interface->display,
					DefaultScreen(user_interface->display)))
				{
#if defined (DEBUG)
					/* This message causes many problems as people wonder if something is wrong. */
					display_message(INFORMATION_MESSAGE,"CREATE(Dm_buffer). DM_PBuffer Unavailable, using plain Pbuffer\n");
#endif /* defined (DEBUG) */
					if((buffer->visual_info = glXChooseVisual(user_interface->display,
						DefaultScreen(user_interface->display), visattrs))
						&& (buffer->config = glXGetFBConfigFromVisualSGIX(
							user_interface->display, buffer->visual_info)))
					{
						if ( buffer->pbuffer = glXCreateGLXPbufferSGIX(user_interface->display,
							buffer->config, width, height, pbuffer_attribs))
						{
							if (buffer->context = glXCreateContextWithConfigSGIX(
								user_interface->display, buffer->config, GLX_RGBA_TYPE_SGIX, 
								ThreeDDrawing_get_shareable_context(), GL_TRUE))
							{
								/* Finished I think, hooray! */
							}
							else
							{
								display_message(ERROR_MESSAGE,"CREATE(Dm_buffer). Cannot get GLX context");
								DEALLOCATE(buffer);
								buffer = (struct Dm_buffer *)NULL;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"CREATE(Dm_buffer). Cannot create pbuffer");
							DEALLOCATE(buffer);
							buffer = (struct Dm_buffer *)NULL;
						}
					}
					else
					{
						display_message(ERROR_MESSAGE,"CREATE(Dm_buffer). Cannot get Frame Buffer Configuration");
						DEALLOCATE(buffer);
						buffer = (struct Dm_buffer *)NULL;
					}
				}
				else
				{
#endif /* defined (GLX_SGIX_pbuffer) */
#endif /* defined (GLX_pbuffer) */
					if (shared_display_buffer)
					{
#if defined (DEBUG)
							/* This message causes many problems as people wonder if something is wrong. */
						display_message(ERROR_MESSAGE,"CREATE(Dm_buffer).  Unable to create shared display offscreen buffer.");
#endif /* defined (DEBUG) */
						DEALLOCATE(buffer);
						buffer = (struct Dm_buffer *)NULL;						
					}
					else
					{
						if (buffer->visual_info = glXChooseVisual(user_interface->display,
								 DefaultScreen(user_interface->display), visattrs))
						{
							printf("CREATE(Dm_buffer). openGL visual id = %d\n",
								(int)buffer->visual_info->visualid);

							/* Try a pixmap buffer */
							if (buffer->pixmap = XCreatePixmap(user_interface->display,
									 DefaultRootWindow(user_interface->display), width, height, 
									 buffer->visual_info->depth))
							{

								if (buffer->glx_pixmap = glXCreateGLXPixmap(user_interface->display,
										 buffer->visual_info, buffer->pixmap))
								{
									if (buffer->context = glXCreateContext(
											 user_interface->display, buffer->visual_info,
											 (GLXContext)NULL , GL_TRUE))
									{
										/* Finished I think, hooray! */
									}
									else
									{
										display_message(ERROR_MESSAGE,"CREATE(Dm_buffer). Unable to get config.");
										DEALLOCATE(buffer);
										buffer = (struct Dm_buffer *)NULL;
									}
								}
								else
								{
									display_message(ERROR_MESSAGE,"CREATE(Dm_buffer). Unable to create GLX pixmap.");
									DEALLOCATE(buffer);
									buffer = (struct Dm_buffer *)NULL;
								}
							}
							else
							{
								display_message(ERROR_MESSAGE,"CREATE(Dm_buffer). Unable to create pixmap.");
								DEALLOCATE(buffer);
								buffer = (struct Dm_buffer *)NULL;
							}
						}
						else
						{
							display_message(ERROR_MESSAGE,"CREATE(Dm_buffer). Unable to get appropriate visual.");
							DEALLOCATE(buffer);
							buffer = (struct Dm_buffer *)NULL;
						}
					}
#if defined (GLX_pbuffer) || defined (GLX_SGIX_pbuffer)
				}
#endif /* defined (GLX_SGIX_pbuffer) || defined (GLX_SGIX_pbuffer) */
#if defined (GLX_SGIX_dm_pbuffer)
			}
#endif /* defined (GLX_SGIX_dm_pbuffer) */
		}
		else
		{
			display_message(ERROR_MESSAGE,"CREATE(Dm_buffer). Unable to allocate buffer structure");
			buffer = (struct Dm_buffer *)NULL;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"CREATE(Dm_buffer). Invalid arguments");
		buffer = (struct Dm_buffer *)NULL;
	}

	LEAVE;

	return (buffer);
} /* CREATE(Dm_buffer) */

int Dm_buffer_glx_make_current(struct Dm_buffer *buffer)
/*******************************************************************************
LAST MODIFIED : 23 June 2000
DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Dm_buffer_glx_make_current);
	if (buffer && buffer->context)
	{
#if defined (GLX_SGIX_dm_pbuffer) || (GLX_SGIX_pbuffer) || (GLX_pbuffer)
		if (buffer->pbuffer)
		{
			if (glXMakeCurrent(buffer->user_interface->display,
				buffer->pbuffer, buffer->context))
			{
				return_code=1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Dm_buffer_glx_make_current.  Unable to make pbuffer current.");
				return_code=0;
			}
		}
		else
		{
#endif /* defined (GLX_SGIX_dm_pbuffer) || (GLX_SGIX_pbuffer) || (GLX_pbuffer) */
			if (buffer->glx_pixmap)
			{
				if (glXMakeCurrent(buffer->user_interface->display,
					buffer->glx_pixmap, buffer->context))
				{
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"Dm_buffer_glx_make_current.  Unable to make pixmap current.");
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Dm_buffer_glx_make_current.  No drawable in buffer.");
				return_code=0;
			}
#if defined (GLX_SGIX_dm_pbuffer) || (GLX_SGIX_pbuffer) || (GLX_pbuffer)
		}
#endif /* defined (GLX_SGIX_dm_pbuffer) || (GLX_SGIX_pbuffer) || (GLX_pbuffer) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Dm_buffer_glx_make_current.  Invalid buffer");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Dm_buffer_glx_make_current */

int Dm_buffer_glx_make_read_current(struct Dm_buffer *buffer)
/*******************************************************************************
LAST MODIFIED : 23 June 2000
DESCRIPTION :
Sets this buffer to be the GLX source and the current ThreeDWindow (the one last
made current) to be the GLX destination.
==============================================================================*/
{
	int return_code;

	ENTER(Dm_buffer_glx_make_current);
	if (buffer && buffer->context)
	{
#if defined (GLX_SGIX_dm_pbuffer) || (GLX_SGIX_pbuffer) || (GLX_pbuffer)
		if (buffer->pbuffer)
		{
			X3dThreeDDrawingAddReadContext(buffer->pbuffer);
			return_code = 1;
		}
		else
		{
#endif /* defined (GLX_SGIX_dm_pbuffer) || (GLX_SGIX_pbuffer) || (GLX_pbuffer) */
			if (buffer->glx_pixmap)
			{
				X3dThreeDDrawingAddReadContext(buffer->glx_pixmap);
				return_code = 1;
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"Dm_buffer_glx_make_read_current.  No drawable in buffer.");
				return_code = 0;
			}
#if defined (GLX_SGIX_dm_pbuffer) || (GLX_SGIX_pbuffer) || (GLX_pbuffer)
		}
#endif /* defined (GLX_SGIX_dm_pbuffer) || (GLX_SGIX_pbuffer) || (GLX_pbuffer) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Dm_buffer_glx_make_read_current.  Invalid buffer");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Dm_buffer_glx_make_read_current */

enum Dm_buffer_type Dm_buffer_get_type(struct Dm_buffer *buffer)
/*******************************************************************************
LAST MODIFIED : 23 June 2000
DESCRIPTION :
Returns information about the type of buffer that was created.  (Only the O2
currently supports Dm_pbuffer so to operate on the Octane a different 
mechanism needs to be supported.
==============================================================================*/	{
	enum Dm_buffer_type return_code;

	ENTER(Dm_buffer_get_type);
	if (buffer)
	{
		return_code = DM_BUFFER_INVALID_TYPE;
#if defined (GLX_SGIX_dm_pbuffer) || (GLX_SGIX_pbuffer) || (GLX_pbuffer)
#if defined (GLX_SGIX_dm_pbuffer)
		if (buffer->pbuffer && buffer->dmbuffer)
		{
			return_code = DM_BUFFER_DM_PBUFFER;
		}
		else
		{
#endif /* defined (GLX_SGIX_dm_pbuffer) */
			if (buffer->pbuffer)
			{
				return_code = DM_BUFFER_GLX_PBUFFER;
			}
			else
			{
#endif /* defined (GLX_SGIX_dm_pbuffer) || (GLX_SGIX_pbuffer) || (GLX_pbuffer) */
				if (buffer->glx_pixmap)
				{
					return_code = DM_BUFFER_GLX_PIXMAP;
				}
#if defined (GLX_SGIX_dm_pbuffer) || (GLX_SGIX_pbuffer) || (GLX_pbuffer)
			}
#if defined (GLX_SGIX_dm_pbuffer)
		}
#endif /* defined (GLX_SGIX_dm_pbuffer) */
#endif /* defined (GLX_SGIX_dm_pbuffer) || (GLX_SGIX_pbuffer) || (GLX_pbuffer) */
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Dm_buffer_get_display.  Invalid buffer");
		return_code=DM_BUFFER_INVALID_TYPE;
	}
	LEAVE;

	return (return_code);
} /* Dm_buffer_get_type */

Display *Dm_buffer_get_display(struct Dm_buffer *buffer)
/*******************************************************************************
LAST MODIFIED : 23 June 2000
DESCRIPTION :
==============================================================================*/
{
	Display *return_code;

	ENTER(Dm_buffer_get_display);
	if (buffer && buffer->user_interface)
	{
		return_code = buffer->user_interface->display;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Dm_buffer_get_display.  Invalid buffer");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Dm_buffer_get_display */

Screen *Dm_buffer_get_screen(struct Dm_buffer *buffer)
/*******************************************************************************
LAST MODIFIED : 23 June 2000
DESCRIPTION :
==============================================================================*/
{
	Screen *return_code;

	ENTER(Dm_buffer_);
	if (buffer && buffer->user_interface)
	{
		return_code = DefaultScreenOfDisplay(buffer->user_interface->display);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Dm_buffer_glx_make_read_current.  Invalid buffer");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Dm_buffer_get_screen */

GLXContext Dm_buffer_get_glxcontext(struct Dm_buffer *buffer)
/*******************************************************************************
LAST MODIFIED : 23 June 2000
DESCRIPTION :
==============================================================================*/
{
	GLXContext return_code;

	ENTER(Dm_buffer_);
	if (buffer && buffer->context)
	{
		return_code = buffer->context;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Dm_buffer_glx_make_read_current.  Invalid buffer");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Dm_buffer_get_glxcontext */

XVisualInfo *Dm_buffer_get_visual_info(struct Dm_buffer *buffer)
/*******************************************************************************
LAST MODIFIED : 23 June 2000
DESCRIPTION :
==============================================================================*/
{
	XVisualInfo *return_code;

	ENTER(Dm_buffer_);

	if (buffer && buffer->visual_info)
	{
		return_code = buffer->visual_info;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Dm_buffer_glx_make_read_current.  Invalid buffer");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Dm_buffer_get_visual_info */

#if defined (GLX_pbuffer)
GLXPbuffer Dm_buffer_get_pbuffer(struct Dm_buffer *buffer)
/*******************************************************************************
LAST MODIFIED : 23 June 2000

DESCRIPTION :
==============================================================================*/
{
	GLXPbuffer return_code;

	ENTER(Dm_buffer_get_pbuffer);
	if (buffer && buffer->pbuffer)
	{
		return_code = buffer->pbuffer;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Dm_buffer_glx_make_read_current.  Invalid buffer");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Dm_buffer_get_pbuffer */
#else /* defined (GLX_pbuffer) */
#if defined (GLX_SGIX_pbuffer)
GLXPbufferSGIX Dm_buffer_get_pbuffer(struct Dm_buffer *buffer)
/*******************************************************************************
LAST MODIFIED : 23 June 2000

DESCRIPTION :
==============================================================================*/
{
	GLXPbufferSGIX return_code;

	ENTER(Dm_buffer_);
	if (buffer && buffer->pbuffer)
	{
		return_code = buffer->pbuffer;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Dm_buffer_glx_make_read_current.  Invalid buffer");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* Dm_buffer_get_pbuffer */
#endif /* defined (GLX_SGIX_pbuffer) */
#endif /* defined (GLX_pbuffer) */

int DESTROY(Dm_buffer)(struct Dm_buffer **buffer)
/*******************************************************************************
LAST MODIFIED : 23 June 2000

DESCRIPTION :
Closes a Digital Media buffer instance
x==============================================================================*/
{
	int return_code;

	ENTER(DESTROY(Dm_buffer));
	if (buffer && *buffer)
	{
		return_code=1;

		X3dThreeDDrawingRemakeCurrent();
#if defined (GLX_SGIX_dm_pbuffer)
		if((*buffer)->dmbuffer)
		{
			dmBufferFree((*buffer)->dmbuffer);
		}
		if((*buffer)->dmpool)
		{
			dmBufferDestroyPool((*buffer)->dmpool);
		}
#endif /* defined (GLX_SGIX_dm_pbuffer) */
		if((*buffer)->context)
		{
			glXDestroyContext((*buffer)->user_interface->display,
				(*buffer)->context);
		}
#if defined (GLX_SGIX_dm_pbuffer) || (GLX_SGIX_pbuffer)
		if((*buffer)->pbuffer)
		{
			glXDestroyGLXPbufferSGIX((*buffer)->user_interface->display,
				(*buffer)->pbuffer );
		}
#endif /* defined (GLX_SGIX_dm_pbuffer) || (GLX_SGIX_pbuffer) */
#if defined (GLX_pbuffer)
		if((*buffer)->pbuffer)
		{
			glXDestroyPbuffer((*buffer)->user_interface->display,
				(*buffer)->pbuffer );
		}
#endif /* defined (GLX_pbuffer) */
#if defined (GLX_SGIX_fbconfig) || (GLX_fbconfig)
		if((*buffer)->config)
		{		
			XFree((*buffer)->config);
		}
#endif /* defined (GLX_SGIX_fbconfig) || (GLX_fbconfig) */
#if defined (DEBUG)
		if((*buffer)->visual_info)
		{
			XFree((*buffer)->visual_info);
		}
#endif /* defined (DEBUG) */
		if((*buffer)->glx_pixmap)
		{
			glXDestroyGLXPixmap((*buffer)->user_interface->display, (*buffer)->glx_pixmap);
		}
		if((*buffer)->pixmap)
		{
			XFreePixmap((*buffer)->user_interface->display, (*buffer)->pixmap);
		}

		DEALLOCATE(*buffer);
		*buffer = (struct Dm_buffer *)NULL;
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"DESTROY(Dm_buffer).  Missing buffer");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* DESTROY(Dm_buffer) */
