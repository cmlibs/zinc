/*******************************************************************************
FILE : dm_interface.h

LAST MODIFIED : 14 September 1998

DESCRIPTION :
==============================================================================*/
#if !defined (DM_INTERFACE_H)
#define DM_INTERFACE_H

#include "general/object.h"
#include "user_interface/user_interface.h"

enum Dm_buffer_type
{
	DM_BUFFER_INVALID_TYPE,
	DM_BUFFER_DM_PBUFFER, /* O2 only at the moment,
									 is a superset of a GLX_PBUFFER*/
	DM_BUFFER_GLX_PBUFFER
};

struct Dm_buffer;

PROTOTYPE_OBJECT_FUNCTIONS(Dm_buffer);

struct Dm_buffer *CREATE(Dm_buffer)(int width, int height, int depth_buffer,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 23 July 1999

DESCRIPTION :
The <depth_buffer> flag specifies whether the visual selected is requested to
have a depth buffer or not.
==============================================================================*/

int Dm_buffer_glx_make_current(struct Dm_buffer *buffer);
/*******************************************************************************
LAST MODIFIED : 10 September 1998
DESCRIPTION :
==============================================================================*/

int Dm_buffer_glx_make_read_current(struct Dm_buffer *buffer);
/*******************************************************************************
LAST MODIFIED : 10 September 1998
DESCRIPTION :
Sets this buffer to be the GLX source and the current ThreeDWindow (the one last
made current) to be the GLX destination.
==============================================================================*/

enum Dm_buffer_type Dm_buffer_get_type(struct Dm_buffer *buffer);
/*******************************************************************************
LAST MODIFIED : 14 September 1998
DESCRIPTION :
Returns information about the type of buffer that was created.  (Only the O2
currently supports Dm_pbuffer so to operate on the Octane a different 
mechanism needs to be supported.
==============================================================================*/	  

#if defined (DM_BUFFER_PRIVATE_FUNCTIONS)
Display *Dm_buffer_get_display(struct Dm_buffer *buffer);
/*******************************************************************************
LAST MODIFIED : 11 September 1998
DESCRIPTION :
==============================================================================*/

Screen *Dm_buffer_get_screen(struct Dm_buffer *buffer);
/*******************************************************************************
LAST MODIFIED : 11 September 1998
DESCRIPTION :
==============================================================================*/

#if defined (SGI_DIITAL_MEDIA)
GLXPbufferSGIX Dm_buffer_get_pbuffer(struct Dm_buffer *buffer);
/*******************************************************************************
LAST MODIFIED : 14 September 1998
DESCRIPTION :
==============================================================================*/
#endif /* defined (SGI_DIITAL_MEDIA) */

GLXContext Dm_buffer_get_glxcontext(struct Dm_buffer *buffer);
/*******************************************************************************
LAST MODIFIED : 11 September 1998
DESCRIPTION :
==============================================================================*/

XVisualInfo *Dm_buffer_get_visual_info(struct Dm_buffer *buffer);
/*******************************************************************************
LAST MODIFIED : 11 September 1998
DESCRIPTION :
==============================================================================*/
#endif /* defined (DM_BUFFER_PRIVATE_FUNCTIONS) */

int DESTROY(Dm_buffer)(struct Dm_buffer **buffer);
/*******************************************************************************
LAST MODIFIED : 10 September 1998

DESCRIPTION :
Closes a Digital Media buffer instance
==============================================================================*/

#endif /* !defined (DM_INTERFACE_H) */

