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
#include <GL/glx.h>
#include <GL/gl.h>
#endif /* defined (SGI_DIGITAL_MEDIA) */
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
#if defined (SGI_DIGITAL_MEDIA)
	DMbuffer dmbuffer;
	DMbufferpool dmpool;
	GLXContext context;
	GLXPbufferSGIX pbuffer;
	struct User_interface *user_interface;
	XVisualInfo *visual_info;
#endif /* defined (SGI_DIGITAL_MEDIA) */

	int access_count;
};

DECLARE_OBJECT_FUNCTIONS(Dm_buffer)

struct Dm_buffer *CREATE(Dm_buffer)(int width, int height, int depth_buffer,
	struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 23 July 1999

DESCRIPTION :
The <depth_buffer> flag specifies whether the visual selected is requested to
have a depth buffer or not.
==============================================================================*/
{
	struct Dm_buffer *buffer;
#if defined (SGI_DIGITAL_MEDIA)
	DMparams *imageFormat, *poolSpec;
	int count = 1;
	DMboolean cacheable = DM_FALSE;
	DMboolean mapped = DM_FALSE;
	GLXFBConfigSGIX config;
	int dm_pbuffer_attribs [] = 
	{
		GLX_DIGITAL_MEDIA_PBUFFER_SGIX, True,
		GLX_PRESERVED_CONTENTS_SGIX, True,
		(int) None
	},
		pbuffer_attribs [] = 
	{
		GLX_PRESERVED_CONTENTS_SGIX, True,
		(int) None
	};
	static int visattrsRGB_with_depth[] =
	{
		GLX_RGBA,
		GLX_RED_SIZE, 8,
		GLX_GREEN_SIZE, 8,
		GLX_BLUE_SIZE, 8,
		GLX_ALPHA_SIZE, 8,
		GLX_DEPTH_SIZE, 8,
		None
	};
	static int visattrsRGB_no_depth[] =
	{
		GLX_RGBA,
		GLX_RED_SIZE, 8,
		GLX_GREEN_SIZE, 8,
		GLX_BLUE_SIZE, 8,
		GLX_ALPHA_SIZE, 8,
		None
	};
	int *visattrs;
#endif /* defined (SGI_DIGITAL_MEDIA) */

	ENTER(CREATE(Dm_buffer));
#if defined (SGI_DIGITAL_MEDIA)
	/* checking arguments */
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
			buffer->dmbuffer = (DMbuffer)NULL;
			buffer->dmpool = (DMbufferpool)NULL;
			buffer->access_count = 0;
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
								&& (config = glXGetFBConfigFromVisualSGIX(
									user_interface->display, buffer->visual_info)))
							{
								printf("CREATE(Dm_buffer). openGL visual id = %d\n",
									buffer->visual_info->visualid);
								if ( buffer->pbuffer = glXCreateGLXPbufferSGIX(user_interface->display,
									config, width, height, dm_pbuffer_attribs))
								{
									if (buffer->context = glXCreateContextWithConfigSGIX(
										user_interface->display, config, GLX_RGBA_TYPE_SGIX, 
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
								XFree(config);
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
				if(query_glx_extension("GLX_SGIX_pbuffer", user_interface->display,
					DefaultScreen(user_interface->display)))
				{
					display_message(INFORMATION_MESSAGE,"CREATE(Dm_buffer). DM_PBuffer Unavailable, using plain Pbuffer\n");
					if((buffer->visual_info = glXChooseVisual(user_interface->display,
						DefaultScreen(user_interface->display), visattrs))
						&& (config = glXGetFBConfigFromVisualSGIX(
							user_interface->display, buffer->visual_info)))
					{
						if ( buffer->pbuffer = glXCreateGLXPbufferSGIX(user_interface->display,
							config, width, height, pbuffer_attribs))
						{
							if (buffer->context = glXCreateContextWithConfigSGIX(
								user_interface->display, config, GLX_RGBA_TYPE_SGIX, 
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
						XFree(config);
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
					display_message(ERROR_MESSAGE,"CREATE(Dm_buffer). GLX Pbuffer extensions not available");
					DEALLOCATE(buffer);
					buffer = (struct Dm_buffer *)NULL;
				}
			}
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
#else /* defined (SGI_DIGITAL_MEDIA) */
	display_message(ERROR_MESSAGE,
		"SGI Digital Media not available");
	buffer = (struct Dm_buffer *)NULL;
#endif /* defined (SGI_DIGITAL_MEDIA) */
	LEAVE;

	return (buffer);
} /* CREATE(Dm_buffer) */

int Dm_buffer_glx_make_current(struct Dm_buffer *buffer)
/*******************************************************************************
LAST MODIFIED : 10 September 1998
DESCRIPTION :
==============================================================================*/
{
	int return_code;

	ENTER(Dm_buffer_glx_make_current);
#if defined (SGI_DIGITAL_MEDIA)
	/* checking arguments */
	if (buffer && buffer->context && buffer->pbuffer)
	{
		if (glXMakeCurrent(buffer->user_interface->display,
			buffer->pbuffer, buffer->context))
		{
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"Dm_buffer_glx_make_current.  Unable to make current");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Dm_buffer_glx_make_current.  Invalid buffer");
		return_code=0;
	}
#else /* defined (SGI_DIGITAL_MEDIA) */
	display_message(ERROR_MESSAGE,
		"SGI Digital Media not available");
		return_code=0;
#endif /* defined (SGI_DIGITAL_MEDIA) */
	LEAVE;

	return (return_code);
} /* Dm_buffer_glx_make_current */

int Dm_buffer_glx_make_read_current(struct Dm_buffer *buffer)
/*******************************************************************************
LAST MODIFIED : 10 September 1998
DESCRIPTION :
Sets this buffer to be the GLX source and the current ThreeDWindow (the one last
made current) to be the GLX destination.
==============================================================================*/
{
	int return_code;

	ENTER(Dm_buffer_glx_make_current);
#if defined (SGI_DIGITAL_MEDIA)
	/* checking arguments */
	if (buffer && buffer->context && buffer->pbuffer)
	{
		X3dThreeDDrawingAddReadContext(buffer->pbuffer);
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Dm_buffer_glx_make_read_current.  Invalid buffer");
		return_code=0;
	}
#else /* defined (SGI_DIGITAL_MEDIA) */
	display_message(ERROR_MESSAGE,
		"SGI Digital Media not available");
		return_code=0;
#endif /* defined (SGI_DIGITAL_MEDIA) */
	LEAVE;

	return (return_code);
} /* Dm_buffer_glx_make_read_current */

enum Dm_buffer_type Dm_buffer_get_type(struct Dm_buffer *buffer)
/*******************************************************************************
LAST MODIFIED : 14 September 1998
DESCRIPTION :
Returns information about the type of buffer that was created.  (Only the O2
currently supports Dm_pbuffer so to operate on the Octane a different 
mechanism needs to be supported.
==============================================================================*/	{
	enum Dm_buffer_type return_code;

	ENTER(Dm_buffer_get_type);
#if defined (SGI_DIGITAL_MEDIA)
	/* checking arguments */
	if (buffer)
	{
		return_code = DM_BUFFER_INVALID_TYPE;
		if (buffer->pbuffer)
		{
			if (buffer->dmbuffer)
			{
				return_code = DM_BUFFER_DM_PBUFFER;
			}
			else
			{
				return_code = DM_BUFFER_GLX_PBUFFER;
			}
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"Dm_buffer_get_display.  Invalid buffer");
		return_code=DM_BUFFER_INVALID_TYPE;
	}
#else /* defined (SGI_DIGITAL_MEDIA) */
	display_message(ERROR_MESSAGE,
		"SGI Digital Media not available");
		return_code=DM_BUFFER_INVALID_TYPE;
#endif /* defined (SGI_DIGITAL_MEDIA) */
	LEAVE;

	return (return_code);
} /* Dm_buffer_get_type */

Display *Dm_buffer_get_display(struct Dm_buffer *buffer)
/*******************************************************************************
LAST MODIFIED : 11 September 1998
DESCRIPTION :
==============================================================================*/
{
	Display *return_code;

	ENTER(Dm_buffer_get_display);
#if defined (SGI_DIGITAL_MEDIA)
	/* checking arguments */
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
#else /* defined (SGI_DIGITAL_MEDIA) */
	display_message(ERROR_MESSAGE,
		"SGI Digital Media not available");
		return_code=0;
#endif /* defined (SGI_DIGITAL_MEDIA) */
	LEAVE;

	return (return_code);
} /* Dm_buffer_get_display */

Screen *Dm_buffer_get_screen(struct Dm_buffer *buffer)
/*******************************************************************************
LAST MODIFIED : 11 September 1998
DESCRIPTION :
==============================================================================*/
{
	Screen *return_code;

	ENTER(Dm_buffer_);
#if defined (SGI_DIGITAL_MEDIA)
	/* checking arguments */
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
#else /* defined (SGI_DIGITAL_MEDIA) */
	display_message(ERROR_MESSAGE,
		"SGI Digital Media not available");
		return_code=0;
#endif /* defined (SGI_DIGITAL_MEDIA) */
	LEAVE;

	return (return_code);
} /* Dm_buffer_get_screen */

GLXContext Dm_buffer_get_glxcontext(struct Dm_buffer *buffer)
/*******************************************************************************
LAST MODIFIED : 11 September 1998
DESCRIPTION :
==============================================================================*/
{
	GLXContext return_code;

	ENTER(Dm_buffer_);
#if defined (SGI_DIGITAL_MEDIA)
	/* checking arguments */
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
#else /* defined (SGI_DIGITAL_MEDIA) */
	display_message(ERROR_MESSAGE,
		"SGI Digital Media not available");
		return_code=0;
#endif /* defined (SGI_DIGITAL_MEDIA) */
	LEAVE;

	return (return_code);
} /* Dm_buffer_get_glxcontext */

XVisualInfo *Dm_buffer_get_visual_info(struct Dm_buffer *buffer)
/*******************************************************************************
LAST MODIFIED : 11 September 1998
DESCRIPTION :
==============================================================================*/
{
	XVisualInfo *return_code;

	ENTER(Dm_buffer_);
#if defined (SGI_DIGITAL_MEDIA)
	/* checking arguments */
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
#else /* defined (SGI_DIGITAL_MEDIA) */
	display_message(ERROR_MESSAGE,
		"SGI Digital Media not available");
		return_code=0;
#endif /* defined (SGI_DIGITAL_MEDIA) */
	LEAVE;

	return (return_code);
} /* Dm_buffer_get_visual_info */

#if defined (SGI_DIGITAL_MEDIA)
GLXPbufferSGIX Dm_buffer_get_pbuffer(struct Dm_buffer *buffer)
/*******************************************************************************
LAST MODIFIED : 11 September 1998

DESCRIPTION :
==============================================================================*/
{
	GLXPbufferSGIX return_code;

	ENTER(Dm_buffer_);
#if defined (SGI_DIGITAL_MEDIA)
	/* checking arguments */
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
#else /* defined (SGI_DIGITAL_MEDIA) */
	display_message(ERROR_MESSAGE,
		"SGI Digital Media not available");
		return_code=0;
#endif /* defined (SGI_DIGITAL_MEDIA) */
	LEAVE;

	return (return_code);
} /* Dm_buffer_get_pbuffer */
#endif /* defined (SGI_DIGITAL_MEDIA) */

int DESTROY(Dm_buffer)(struct Dm_buffer **buffer)
/*******************************************************************************
LAST MODIFIED : 10 September 1998

DESCRIPTION :
Closes a Digital Media buffer instance
x==============================================================================*/
{
	int return_code;

	ENTER(DESTROY(Dm_buffer));
#if defined (SGI_DIGITAL_MEDIA)
	/* checking arguments */
	if (buffer && *buffer)
	{
		return_code=1;

		X3dThreeDDrawingRemakeCurrent();
		if((*buffer)->visual_info)
		{
			XFree((*buffer)->visual_info);
		}
		if((*buffer)->context)
		{
			glXDestroyContext((*buffer)->user_interface->display,
				(*buffer)->context);
		}
		if((*buffer)->pbuffer)
		{
			glXDestroyGLXPbufferSGIX((*buffer)->user_interface->display,
				(*buffer)->pbuffer );
		}
		if((*buffer)->dmbuffer)
		{
			dmBufferFree((*buffer)->dmbuffer);
		}
		if((*buffer)->dmpool)
		{
			dmBufferDestroyPool((*buffer)->dmpool);
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
#else /* defined (SGI_DIGITAL_MEDIA) */
	display_message(ERROR_MESSAGE,
		"DESTROY(Dm_buffer). SGI Digital Media not available");
	return_code=0;
#endif /* defined (SGI_DIGITAL_MEDIA) */
	LEAVE;

	return (return_code);
} /* DESTROY(Dm_buffer) */

