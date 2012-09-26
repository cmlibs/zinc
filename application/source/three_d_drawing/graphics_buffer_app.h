


#if 1
#include "configure/cmgui_configure.h"
#endif /* defined (BUILD_WITH_CMAKE) */

#include "general/callback.h"
#include "general/object.h"
#include "user_interface/user_interface.h"

#if defined (GTK_USER_INTERFACE)
/* #define GTK_USE_GTKGLAREA */
#endif /* defined (GTK_USER_INTERFACE) */

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

struct Graphics_buffer_expose_data
{
	int left;
	int bottom;
	int right;
	int top;
};

struct Graphics_buffer_package;

DECLARE_CMISS_CALLBACK_TYPES(Graphics_buffer_callback, \
	struct Graphics_buffer *, void *, void);

DECLARE_CMISS_CALLBACK_TYPES(Graphics_buffer_input_callback, \
	struct Graphics_buffer *, struct Graphics_buffer_input *, void);

/*
Global functions
-
*/


struct Graphics_buffer_package *CREATE(Graphics_buffer_package)(
	struct User_interface *user_interface);



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



struct Graphics_buffer *create_Graphics_buffer_offscreen_from_buffer(
	int width, int height, struct Graphics_buffer *buffer_to_match);
/*******************************************************************************
LAST MODIFIED : 6 May 2004



DESCRIPTION :
==============================================================================*/

#if defined (WIN32_USER_INTERFACE)
struct Graphics_buffer *create_Graphics_buffer_win32(
	struct Graphics_buffer_package *graphics_buffer_package,
	HWND hWnd, HDC hDC,
	enum Graphics_buffer_buffering_mode buffering_mode,
	enum Graphics_buffer_stereo_mode stereo_mode,
	int minimum_colour_buffer_depth, int minimum_depth_buffer_depth,
	int minimum_accumulation_buffer_depth);
/*******************************************************************************
LAST MODIFIED : 1 June 2007

DESCRIPTION :
==============================================================================*/
#endif /* defined (WIN32_USER_INTERFACE) */

#if defined (WIN32_USER_INTERFACE)
int Graphics_buffer_handle_windows_event(struct Graphics_buffer *graphics_buffer,
	UINT event,WPARAM first_message,LPARAM second_message);
/*******************************************************************************
LAST MODIFIED : 31 May 2007

DESCRIPTION:
Passes the supplied windows event on to the graphics buffer.
==============================================================================*/
#endif /* defined (WIN32_USER_INTERFACE) */

#if defined (WIN32_USER_INTERFACE)
int Graphics_buffer_win32_set_window_size(struct Graphics_buffer *buffer,
	int width, int height, int x, int y);
/*******************************************************************************
LAST MODIFIED : 14 September 2007

DESCRIPTION :
Sets the maximum extent of the graphics window within which individual paints
will be requested with handle_windows_event.
==============================================================================*/
#endif /* defined (WIN32_USER_INTERFACE) */

#if defined (CARBON_USER_INTERFACE)
struct Graphics_buffer *create_Graphics_buffer_Carbon(
	struct Graphics_buffer_package *graphics_buffer_package,
	WindowRef windowIn,
	enum Graphics_buffer_buffering_mode buffering_mode,
	enum Graphics_buffer_stereo_mode stereo_mode,
	int minimum_colour_buffer_depth, int minimum_depth_buffer_depth,
	int minimum_accumulation_buffer_depth);
/*******************************************************************************
LAST MODIFIED : 21 November 2006

DESCRIPTION :
==============================================================================*/
#endif /* defined (CARBON_USER_INTERFACE) */

#if defined (WX_USER_INTERFACE) && defined (__cplusplus)
class wxPanel;

struct Graphics_buffer *create_Graphics_buffer_wx(
	struct Graphics_buffer_package *graphics_buffer_package,
	wxPanel *parent,
	enum Graphics_buffer_buffering_mode buffering_mode,
	enum Graphics_buffer_stereo_mode stereo_mode,
	int minimum_colour_buffer_depth, int minimum_depth_buffer_depth,
	int minimum_accumulation_buffer_depth,
	struct Graphics_buffer *buffer_to_match);
/*******************************************************************************
LAST MODIFIED : 7 December 2006

DESCRIPTION :
==============================================================================*/
#endif /* defined (WX_USER_INTERFACE) && defined (__cplusplus) */

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

/*******************************************************************************
LAST MODIFIED : 19 September 2002

DESCRIPTION :
Returns the buffering mode being used by the graphics buffer.
==============================================================================*/

/*******************************************************************************
LAST MODIFIED : 19 September 2002







DESCRIPTION :
Returns the stereo mode being used by the graphics buffer.
==============================================================================*/

int Graphics_buffer_get_colour_buffer_depth(struct Graphics_buffer *buffer,
	int *colour_buffer_depth);


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

int Graphics_buffer_set_width(struct Graphics_buffer *buffer, int width);
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Sets the width of buffer represented by <buffer>.
==============================================================================*/

int Graphics_buffer_set_height(struct Graphics_buffer *buffer, int height);
/*******************************************************************************
LAST MODIFIED : 1 July 2002

DESCRIPTION :
Sets the height of buffer represented by <buffer>.
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

Returns information about the type of buffer that was created.
*/
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

#if defined (WIN32_USER_INTERFACE)
int Graphics_buffer_win32_use_font_bitmaps(struct Graphics_buffer *buffer,
	HFONT font, int first_bitmap, int number_of_bitmaps, int display_list_offset);
/*******************************************************************************
LAST MODIFIED : 17 November 2005

DESCRIPTION :
Function provided for compiling graphics_fonts as a graphics context is
required in the win32 case.
==============================================================================*/
#endif /* defined (WIN32_USER_INTERFACE) */

#if defined (CARBON_USER_INTERFACE)
int Graphics_buffer_carbon_set_window_size(struct Graphics_buffer *graphics_buffer,
	int width, int height, int clip_width, int clip_height);
/*******************************************************************************
LAST MODIFIED : 16 February 2007

DESCRIPTION :
Sets the coordinates within the graphics port which the graphics_buffer should
respect.
==============================================================================*/
#endif /* defined (CARBON_USER_INTERFACE) */


#if defined (OPENGL_API) && defined (USE_MSAA)
void Graphics_buffer_reset_multisample_framebuffer(struct Graphics_buffer *buffer);

void Graphics_buffer_blit_framebuffer(struct Graphics_buffer *buffer);

int Graphics_buffer_set_multisample_framebuffer(struct Graphics_buffer *buffer, int preferred_antialias);
#endif /* defined (USE_MSAA) */

