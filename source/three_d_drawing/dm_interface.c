/*******************************************************************************
FILE : dm_interface.c

LAST MODIFIED : 28 October 1998

DESCRIPTION :
This provides a Cmgui interface to the Digital Media libraries on the SGI
******************************************************************************/
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
#if defined (GLX_SGIX_dm_pbuffer) || (GLX_SGIX_pbuffer)
	GLXPbufferSGIX pbuffer;
#endif /* defined (GLX_SGIX_dm_pbuffer) || (GLX_SGIX_pbuffer) */
#if defined (GLX_SGIX_fbconfig)
	GLXFBConfigSGIX config;
#endif /* defined (GLX_SGIX_fbconfig) */
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
	int count = 1;
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
#if defined (GLX_SGIX_dm_pbuffer) || (GLX_SGIX_pbuffer)
	int pbuffer_attribs [] = 
	{
		GLX_PRESERVED_CONTENTS_SGIX, True,
		(int) None
	};
#endif /* defined (GLX_SGIX_dm_pbuffer) || (GLX_SGIX_pbuffer) */
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

	if(user_interface)
	{
		if (depth_buffer)
		{
			visattrs = visattrsRGB_with_depth;
		}
		else
		{
			visattrs = visattrsRGB_no_depth;
		}
		if (ALLOCATE(buffer, struct Dm_buffer, 1))
		{
			buffer->user_interface = user_interface;
#if defined (GLX_SGIX_dm_pbuffer)
			buffer->dmbuffer = (DMbuffer)NULL;
			buffer->dmpool = (DMbufferpool)NULL;
#endif /* defined (GLX_SGIX_dm_pbuffer) */
			buffer->context = (GLXContext)NULL;
#if defined (GLX_SGIX_dm_pbuffer) || (GLX_SGIX_pbuffer)
			buffer->pbuffer = (GLXPbufferSGIX)NULL;
#endif /* defined (GLX_SGIX_dm_pbuffer) || (GLX_SGIX_pbuffer) */
#if defined (GLX_SGIX_fbconfig)
			buffer->config = (GLXFBConfigSGIX)NULL;;
#endif /* defined (GLX_SGIX_fbconfig) */
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
						if(buffer->visual_info = glXChooseVisual(user_interface->display,
							DefaultScreen(user_interface->display), visattrs))
						{
							printf("CREATE(Dm_buffer). openGL visual id = %d\n",
								(int)buffer->visual_info->visualid);

							/* Try a pixmap buffer */
							if(buffer->pixmap = XCreatePixmap(user_interface->display,
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
#if defined (GLX_SGIX_pbuffer)
				}
#endif /* defined (GLX_SGIX_pbuffer) */
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
#if defined (GLX_SGIX_dm_pbuffer) || (GLX_SGIX_pbuffer)
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
#endif /* defined (GLX_SGIX_dm_pbuffer) || (GLX_SGIX_pbuffer) */
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
#if defined (GLX_SGIX_dm_pbuffer) || (GLX_SGIX_pbuffer)
		}
#endif /* defined (GLX_SGIX_dm_pbuffer) || (GLX_SGIX_pbuffer) */
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
#if defined (GLX_SGIX_dm_pbuffer) || (GLX_SGIX_pbuffer)
		if (buffer->pbuffer)
		{
			X3dThreeDDrawingAddReadContext(buffer->pbuffer);
			return_code = 1;
		}
		else
		{
#endif /* defined (GLX_SGIX_dm_pbuffer) || (GLX_SGIX_pbuffer) */
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
#if defined (GLX_SGIX_dm_pbuffer) || (GLX_SGIX_pbuffer)
		}
#endif /* defined (GLX_SGIX_dm_pbuffer) || (GLX_SGIX_pbuffer) */
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
#if defined (GLX_SGIX_dm_pbuffer) || (GLX_SGIX_pbuffer)
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
#endif /* defined (GLX_SGIX_dm_pbuffer) || (GLX_SGIX_pbuffer) */
				if (buffer->glx_pixmap)
				{
					return_code = DM_BUFFER_GLX_PIXMAP;
				}
#if defined (GLX_SGIX_dm_pbuffer) || (GLX_SGIX_pbuffer)
			}
#if defined (GLX_SGIX_dm_pbuffer)
		}
#endif /* defined (GLX_SGIX_dm_pbuffer) */
#endif /* defined (GLX_SGIX_dm_pbuffer) || (GLX_SGIX_pbuffer) */
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
#if defined (GLX_SGIX_fbconfig)
		if((*buffer)->config)
		{		
			XFree((*buffer)->config);
		}
#endif /* defined (GLX_SGIX_fbconfig) */
		if((*buffer)->visual_info)
		{
			XFree((*buffer)->visual_info);
		}
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

