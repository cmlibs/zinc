/*******************************************************************************
FILE : drawing_2d.h

LAST MODIFIED : 23 July 1998

DESCRIPTION :
==============================================================================*/
#if !defined (DRAWING_2D_H)
#define DRAWING_2D_H

#include <stddef.h>
#if defined (MOTIF)
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#endif /* defined (MOTIF) */

/*
Global types
------------
*/
enum Create_drawing_2d_image
/*******************************************************************************
LAST MODIFIED : 16 December 1996

DESCRIPTION :
Logical for specifying whether or not an image is created for a drawing.
==============================================================================*/
{
	DRAWING_IMAGE,
	NO_DRAWING_IMAGE
}; /* enum Create_drawing_2d_image */

struct Drawing_2d
/*******************************************************************************
LAST MODIFIED : 25 March 1997

DESCRIPTION :
==============================================================================*/
{
#if defined (MOTIF)
	Widget widget;
#endif /* defined (MOTIF) */
	int depth,height,width;
#if defined (MOTIF)
	Pixmap pixel_map;
		/* stored by the X server */
	XImage *image;
		/* stored by the X client (application) machine */
#endif /* defined (MOTIF) */
	struct User_interface *user_interface;
}; /* struct Drawing_2d */

/*
Global macros
-------------
*/
#define SCALE_FACTOR(unscaled_range,scaled_range) \
	((float)(scaled_range)/(float)(unscaled_range))

#define SCALE_X(unscaled,unscaled_first,left,scale_factor) \
	((left)+(int)((float)((unscaled)-(unscaled_first))*scale_factor+0.5))

#define SCALE_Y(unscaled,unscaled_last,top,scale_factor) \
	((top)+(int)((float)((unscaled_last)-(unscaled))*scale_factor+0.5))

/*
Global functions
----------------
*/
struct Drawing_2d *create_Drawing_2d(
#if defined (MOTIF)
	Widget widget,
#endif /* defined (MOTIF) */
	int width,int height,enum Create_drawing_2d_image create_image,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 25 March 1997

DESCRIPTION :
This function allocates memory for a drawing and initializes the fields to the
specified values.  It returns a pointer to the created drawing if successful
and NULL if unsuccessful.
==============================================================================*/

int destroy_Drawing_2d(struct Drawing_2d **drawing);
/*******************************************************************************
LAST MODIFIED : 16 December 1996

DESCRIPTION :
This function frees the memory associated with the fields of <**drawing>, frees
the memory for <**drawing> and changes <*drawing> to NULL.
==============================================================================*/

unsigned long *get_Drawing_2d_image(struct Drawing_2d *drawing);
/*******************************************************************************
LAST MODIFIED : 23 July 1998

DESCRIPTION :
Allocates an image, suitable for passing to the image_utilities, and fills it
with the current contents of the <drawing>.
==============================================================================*/
#endif /* !defined (DRAWING_2D_H) */
