/*******************************************************************************
FILE : graphics_buffer.h

LAST MODIFIED : 4 June 2004

DESCRIPTION :
This provides a Cmgui interface to the OpenGL contexts of many types.
==============================================================================*/
#if !defined (GRAPHICS_BUFFER_H)
#define GRAPHICS_BUFFER_H

#include "general/callback.h"
#include "general/object.h"
#include "user_interface/user_interface.h"

#if defined (GTK_USER_INTERFACE)
/* #define GTK_USE_GTKGLAREA */
#endif /* defined (GTK_USER_INTERFACE) */

#if defined (OPENGL_API)
#  if defined (MOTIF) || defined (GTK_USER_INTERFACE)
#     define GRAPHICS_BUFFER_USE_BUFFERS
#     define GRAPHICS_BUFFER_USE_OFFSCREEN_BUFFERS
#  endif /* defined (MOTIF) || defined (GTK_USER_INTERFACE) */
#endif /* defined (OPENGL_API) */

/*
Global types
------------
*/

enum Graphics_buffer_type
{
	GRAPHICS_BUFFER_INVALID_TYPE,
	GRAPHICS_BUFFER_GLX_X3D_TYPE,
	GRAPHICS_BUFFER_GLX_DM_PBUFFER_TYPE, /* Special type available only on O2's */
	GRAPHICS_BUFFER_GLX_PBUFFER_TYPE, /* Accelerated offscreen rendering */
	GRAPHICS_BUFFER_GLX_PIXMAP_TYPE, /* Non shared offscreen, no good for our display lists but
											  can be used for find_xi_special buffer */
	GRAPHICS_BUFFER_GTKGLAREA_TYPE,
	GRAPHICS_BUFFER_GTKGLEXT_TYPE
};

enum Graphics_buffer_buffering_mode
{
	GRAPHICS_BUFFER_ANY_BUFFERING_MODE,
	GRAPHICS_BUFFER_SINGLE_BUFFERING,
	GRAPHICS_BUFFER_DOUBLE_BUFFERING
};

enum Graphics_buffer_stereo_mode
{
	GRAPHICS_BUFFER_ANY_STEREO_MODE,
	GRAPHICS_BUFFER_MONO,
	GRAPHICS_BUFFER_STEREO
};

enum Graphics_buffer_input_modifier
{
	GRAPHICS_BUFFER_INPUT_MODIFIER_SHIFT = 1,
	GRAPHICS_BUFFER_INPUT_MODIFIER_CONTROL = 2,
	GRAPHICS_BUFFER_INPUT_MODIFIER_ALT = 4,
	GRAPHICS_BUFFER_INPUT_MODIFIER_BUTTON1 = 8
};

enum Graphics_buffer_input_event_type
{
	GRAPHICS_BUFFER_INVALID_INPUT,
	GRAPHICS_BUFFER_MOTION_NOTIFY,
	GRAPHICS_BUFFER_BUTTON_PRESS,
	GRAPHICS_BUFFER_BUTTON_RELEASE,
	GRAPHICS_BUFFER_KEY_PRESS,
	GRAPHICS_BUFFER_KEY_RELEASE
};

struct Graphics_buffer_input
{
	enum Graphics_buffer_input_event_type type;
	int button_number;
	int key_code;
	int position_x;
	int position_y;
	/* flags indicating the state of the shift, control and alt keys - use
		 logical OR with GRAPHICS_BUFFER_INPUT_MODIFIER_SHIFT etc. */
	enum Graphics_buffer_input_modifier input_modifier;
};

struct Graphics_buffer_package;

struct Graphics_buffer;

DECLARE_CMISS_CALLBACK_TYPES(Graphics_buffer_callback, \
	struct Graphics_buffer *, void *);

DECLARE_CMISS_CALLBACK_TYPES(Graphics_buffer_input_callback, \
	struct Graphics_buffer *, struct Graphics_buffer_input *);

/*
Global functions
----------------
*/

#if defined (MOTIF) || defined (GTK_USER_INTERFACE)
struct Graphics_buffer_package *CREATE(Graphics_buffer_package)(
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 6 May 2004

DESCRIPTION :
Creates a Graphics_buffer_package which enables Graphics_buffers created from
it to share graphics contexts.
==============================================================================*/
#endif /* defined (MOTIF) || defined (GTK_USER_INTERFACE) */

#if defined (MOTIF) || defined (GTK_USER_INTERFACE)
int DESTROY(Graphics_buffer_package)(struct Graphics_buffer_package **package_ptr);
/*******************************************************************************
LAST MODIFIED : 5 May 2004

DESCRIPTION :
Closes the Graphics buffer package
==============================================================================*/
#endif /* defined (MOTIF) || defined (GTK_USER_INTERFACE) */

#if defined (MOTIF) || defined (GTK_USER_INTERFACE)
int Graphics_buffer_package_set_override_visual_id(
	struct Graphics_buffer_package *graphics_buffer_package,
	int override_visual_id);
/*******************************************************************************
LAST MODIFIED : 21 May 2004

DESCRIPTION :
Sets a particular visual to be used by all graphics buffers.
==============================================================================*/
#endif /* defined (MOTIF) || defined (GTK_USER_INTERFACE) */

PROTOTYPE_OBJECT_FUNCTIONS(Graphics_buffer);

#if defined (MOTIF) || defined (GTK_USER_INTERFACE)
struct Graphics_buffer *create_Graphics_buffer_offscreen(
	struct Graphics_buffer_package *graphics_buffer_package,
	int width, int height,
	enum Graphics_buffer_buffering_mode buffering_mode,
	enum Graphics_buffer_stereo_mode stereo_mode,
	int minimum_colour_buffer_depth, int minimum_depth_buffer_depth,
	int minimum_accumulation_buffer_depth);
/*******************************************************************************
LAST MODIFIED : 6 May 2004

DESCRIPTION :
==============================================================================*/
#endif /* defined (MOTIF) || defined (GTK_USER_INTERFACE) */

#if defined (MOTIF) || defined (GTK_USER_INTERFACE)
struct Graphics_buffer *create_Graphics_buffer_shared_offscreen(
	struct Graphics_buffer_package *graphics_buffer_package,
	int width, int height,
	enum Graphics_buffer_buffering_mode buffering_mode,
	enum Graphics_buffer_stereo_mode stereo_mode,
	int minimum_colour_buffer_depth, int minimum_depth_buffer_depth,
	int minimum_accumulation_buffer_depth);
/*******************************************************************************
LAST MODIFIED : 6 May 2004

DESCRIPTION :
==============================================================================*/
#endif /* defined (MOTIF) || defined (GTK_USER_INTERFACE) */

#if defined (MOTIF) || defined (GTK_USER_INTERFACE)
struct Graphics_buffer *create_Graphics_buffer_offscreen_from_buffer(
	int width, int height, struct Graphics_buffer *buffer_to_match);
/*******************************************************************************
LAST MODIFIED : 6 May 2004

DESCRIPTION :
==============================================================================*/
#endif /* defined (MOTIF) || defined (GTK_USER_INTERFACE) */

#if defined (MOTIF)
struct Graphics_buffer *create_Graphics_buffer_X3d(
	struct Graphics_buffer_package *graphics_buffer_package,
	Widget parent, int width, int height,
	enum Graphics_buffer_buffering_mode buffering_mode,
	enum Graphics_buffer_stereo_mode stereo_mode,
	int minimum_colour_buffer_depth, int minimum_depth_buffer_depth,
	int minimum_accumulation_buffer_depth);
/*******************************************************************************
LAST MODIFIED : 5 May 2004

DESCRIPTION :
==============================================================================*/
#endif /* defined (MOTIF) */

#if defined (MOTIF)
struct Graphics_buffer *create_Graphics_buffer_X3d_from_buffer(
	Widget parent, int width, int height, 
	struct Graphics_buffer *buffer_to_match);
/*******************************************************************************
LAST MODIFIED : 6 May 2004

DESCRIPTION :
==============================================================================*/
#endif /* defined (MOTIF) */

#if defined (GTK_USER_INTERFACE)
struct Graphics_buffer *create_Graphics_buffer_gtkgl(
	struct Graphics_buffer_package *graphics_buffer_package,
	GtkContainer *parent,
	enum Graphics_buffer_buffering_mode buffering_mode,
	enum Graphics_buffer_stereo_mode stereo_mode,
	int minimum_colour_buffer_depth, int minimum_depth_buffer_depth, 
	int minimum_accumulation_buffer_depth);
/*******************************************************************************
LAST MODIFIED : 2 June 2004

DESCRIPTION :
==============================================================================*/
#endif /* defined (GTK_USER_INTERFACE) */

int Graphics_buffer_make_current(struct Graphics_buffer *buffer);
/*******************************************************************************
LAST MODIFIED : 1 July 2002
DESCRIPTION :
==============================================================================*/

int Graphics_buffer_get_visual_id(struct Graphics_buffer *buffer, int *visual_id);
/*******************************************************************************
LAST MODIFIED : 19 September 2002

DESCRIPTION :
Returns the visual id used by the graphics buffer.
==============================================================================*/

int Graphics_buffer_get_buffering_mode(struct Graphics_buffer *buffer,
	enum Graphics_buffer_buffering_mode *buffering_mode);
/*******************************************************************************
LAST MODIFIED : 19 September 2002

DESCRIPTION :
Returns the buffering mode being used by the graphics buffer.
==============================================================================*/

int Graphics_buffer_get_stereo_mode(struct Graphics_buffer *buffer,
	enum Graphics_buffer_stereo_mode *stereo_mode);
/*******************************************************************************
LAST MODIFIED : 19 September 2002

DESCRIPTION :
Returns the stereo mode being used by the graphics buffer.
==============================================================================*/

int Graphics_buffer_get_colour_buffer_depth(struct Graphics_buffer *buffer,
	int *colour_buffer_depth);
/*******************************************************************************
LAST MODIFIED : 19 September 2002

DESCRIPTION :
Returns the depth of the colour buffer used by the graphics buffer.
==============================================================================*/

int Graphics_buffer_get_depth_buffer_depth(struct Graphics_buffer *buffer,
	int *depth_buffer_depth);
/*******************************************************************************
LAST MODIFIED : 19 September 2002

DESCRIPTION :
Returns the depth of the depth buffer used by the graphics buffer.
==============================================================================*/

int Graphics_buffer_get_accumulation_buffer_depth(struct Graphics_buffer *buffer,
	int *accumulation_buffer_depth);
/*******************************************************************************
LAST MODIFIED : 19 September 2002

DESCRIPTION :
Returns the depth of the accumulation buffer used by the graphics buffer.
==============================================================================*/

int Graphics_buffer_swap_buffers(struct Graphics_buffer *buffer);
/*******************************************************************************
LAST MODIFIED : 2 July 2002

DESCRIPTION :
==============================================================================*/

int Graphics_buffer_make_read_current(struct Graphics_buffer *buffer);
/*******************************************************************************
LAST MODIFIED : 1 July 2002
DESCRIPTION :
Sets this buffer to be the source and the current ThreeDWindow (the one last
made current) to be the destination.
==============================================================================*/

int Graphics_buffer_get_width(struct Graphics_buffer *buffer);
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Returns the width of buffer represented by <buffer>.
==============================================================================*/

int Graphics_buffer_set_width(struct Graphics_buffer *buffer, int width);
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Sets the width of buffer represented by <buffer>.
==============================================================================*/

int Graphics_buffer_get_height(struct Graphics_buffer *buffer);
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Gets the height of buffer represented by <buffer>.
==============================================================================*/

int Graphics_buffer_set_height(struct Graphics_buffer *buffer, int height);
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Sets the height of buffer represented by <buffer>.
==============================================================================*/

int Graphics_buffer_get_border_width(struct Graphics_buffer *buffer);
/*******************************************************************************
LAST MODIFIED : 2 July 2002

DESCRIPTION :
Returns the border width of buffer represented by <buffer>.
==============================================================================*/

int Graphics_buffer_set_border_width(struct Graphics_buffer *buffer,
	int border_width);
/*******************************************************************************
LAST MODIFIED : 2 July 2002

DESCRIPTION :
Sets the border width of buffer represented by <buffer>.
==============================================================================*/

int Graphics_buffer_is_visible(struct Graphics_buffer *buffer);
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Returns 1 if the <buffer> is visible.  If the scene viewer gets zero from this
routine it will not bother rendering into it, allowing us to avoid rendering
into unmanaged or invisible widgets.
==============================================================================*/

int Graphics_buffer_awaken(struct Graphics_buffer *buffer);
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Activates the graphics <buffer>.
==============================================================================*/

enum Graphics_buffer_type Graphics_buffer_get_type(struct Graphics_buffer *buffer);
/*******************************************************************************
LAST MODIFIED : 27 May 2004

DESCRIPTION :
Returns information about the type of buffer that was created.
==============================================================================*/

int Graphics_buffer_add_initialise_callback(struct Graphics_buffer *buffer,
	CMISS_CALLBACK_FUNCTION(Graphics_buffer_callback) initialise_callback, void *user_data);
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Adds an initialise callback to the graphics <buffer>.
==============================================================================*/

int Graphics_buffer_add_resize_callback(struct Graphics_buffer *buffer,
	CMISS_CALLBACK_FUNCTION(Graphics_buffer_callback) resize_callback, void *user_data);
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Adds an resize callback to the graphics <buffer>.
==============================================================================*/

int Graphics_buffer_add_expose_callback(struct Graphics_buffer *buffer,
	CMISS_CALLBACK_FUNCTION(Graphics_buffer_callback) expose_callback, void *user_data);
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Adds an expose callback to the graphics <buffer>.
==============================================================================*/

int Graphics_buffer_add_input_callback(struct Graphics_buffer *buffer,
	CMISS_CALLBACK_FUNCTION(Graphics_buffer_input_callback) input_callback, void *user_data);
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Adds an input callback to the graphics <buffer>.
==============================================================================*/

int DESTROY(Graphics_buffer)(struct Graphics_buffer **buffer);
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Closes a Graphics buffer instance
==============================================================================*/

#if defined (OPENGL_API) && defined (MOTIF)
int query_glx_extension(char *extName, Display *display, int screen);
/*******************************************************************************
LAST MODIFIED : 4 May 2004

DESCRIPTION :
Search for extName in the GLX extensions string. Use of strstr() is not sufficient
because extension names can be prefixes of other extension names. Could use
strtok() but the constant string returned by glGetString might be in read-only
memory.
???SAB.  Taken directly from above
==============================================================================*/
#endif /* defined (OPENGL_API) && defined (MOTIF) */
#endif /* !defined (GRAPHICS_BUFFER_H) */

