/*******************************************************************************
FILE : graphics_buffer.h

LAST MODIFIED : 1 July 2002

DESCRIPTION :
This provides a Cmgui interface to the OpenGL contexts of many types.
Should be merged with dm_interface.c
==============================================================================*/
#if !defined (GRAPHICS_BUFFER_H)
#define GRAPHICS_BUFFER_H

#include "general/callback.h"
#include "general/object.h"
#if defined (MOTIF)
#include "three_d_drawing/ThreeDDraw.h"
#endif /* defined (MOTIF) */

/*
Global types
------------
*/

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

enum Graphics_buffer_type
{
	GRAPHICS_BUFFER_INVALID_TYPE,
	GRAPHICS_BUFFER_X3D_TYPE,
	GRAPHICS_BUFFER_GTKGLAREA_TYPE
};

struct Graphics_buffer;

DECLARE_CMISS_CALLBACK_TYPES(Graphics_buffer_callback, \
	struct Graphics_buffer *, void *);

DECLARE_CMISS_CALLBACK_TYPES(Graphics_buffer_input_callback, \
	struct Graphics_buffer *, struct Graphics_buffer_input *);

/*
Global functions
----------------
*/

PROTOTYPE_OBJECT_FUNCTIONS(Graphics_buffer);

#if defined (MOTIF)
struct Graphics_buffer *create_Graphics_buffer_X3d(Widget parent,
	X3dBufferColourMode colour_mode, X3dBufferingMode buffer_mode,
	int specified_visual_id);
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
If <specified_visual_id> is not zero then this visual is required.
==============================================================================*/
#endif /* defined (MOTIF) */

#if defined (GTK_USER_INTERFACE)
struct Graphics_buffer *create_Graphics_buffer_gtkglarea(GtkContainer *parent);
/*******************************************************************************
LAST MODIFIED : 10 July 2002

DESCRIPTION :
==============================================================================*/
#endif /* defined (GTK_USER_INTERFACE) */

int Graphics_buffer_make_current(struct Graphics_buffer *buffer);
/*******************************************************************************
LAST MODIFIED : 1 July 2002
DESCRIPTION :
==============================================================================*/

int Graphics_buffer_get_visual_id(struct Graphics_buffer *buffer);
/*******************************************************************************
LAST MODIFIED : 9 August 2002

DESCRIPTION :
Returns the visual id used by the graphics buffer.
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

enum Graphics_buffer_type Graphics_buffer_get_type(struct Graphics_buffer *buffer);
/*******************************************************************************
LAST MODIFIED : 1 July 2002
DESCRIPTION :
Returns information about the type of buffer that was created.  (Only the O2
currently supports Dm_pbuffer so to operate on the Octane a different 
mechanism needs to be supported.
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

#endif /* !defined (GRAPHICS_BUFFER_H) */

