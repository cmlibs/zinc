/*******************************************************************************
FILE : postscript.h

LAST MODIFIED : 22 June 1997

DESCRIPTION :
Prototypes for functions thet create postscript output from the mapping system.
==============================================================================*/
#if !defined (POSTSCRIPT_H)
#define POSTSCRIPT_H

#include <stddef.h>
#include <stdlib.h>
#include "graphics/colour.h"
#include "user_interface/user_interface.h"

#define NO_ALIGNMENT
	/*???DB.  What should be done (carry over from emap) */

/*
Global types
------------
*/
#if !defined (NO_ALIGNMENT)
enum Horizontal_alignment
/*******************************************************************************
LAST MODIFIED : 3 May 1993

DESCRIPTION :
An enumerated type for specifying the horizontal text alignment.
==============================================================================*/
{
	LEFT_ALIGNMENT=0,
	CENTRE_HORIZONTAL_ALIGNMENT=1,
	RIGHT_ALIGNMENT=2
}; /* enum Horizontal_alignment */
#endif

enum Page_orientation
/*******************************************************************************
LAST MODIFIED : 27 April 1993

DESCRIPTION :
An enumerated type for specifying the orientation of a printer page.
==============================================================================*/
{
	PORTRAIT,
	LANDSCAPE
}; /* enum Page_orientation */

#if !defined (NO_ALIGNMENT)
enum Vertical_alignment
/*******************************************************************************
LAST MODIFIED : 3 May 1993

DESCRIPTION :
An enumerated type for specifying the vertical text alignment.
==============================================================================*/
{
	TOP_ALIGNMENT=0,
	CENTRE_VERTICAL_ALIGNMENT=1,
	BOTTOM_ALIGNMENT=2
}; /* enum Vertical_alignment */
#endif

/*
Global variables
----------------
*/
#if !defined (NO_ALIGNMENT)
/*???DB.  Change to module variables ? */
extern enum Horizontal_alignment horizontal_alignment;
extern enum Vertical_alignment vertical_alignment;
#endif

/*
Global macros
-------------
*/
#if !defined (NO_ALIGNMENT)
/*???DB.  Change to module variables ? */
#define SET_HORIZONTAL_ALIGNMENT(alignment) \
	horizontal_alignment=alignment

#define SET_VERTICAL_ALIGNMENT(alignment) \
	vertical_alignment=alignment
#endif

/*
Global functions
----------------
*/
int open_postscript(char *file_name,enum Page_orientation page_orientation,
	Colormap colour_map,Pixel *spectrum_pixels,int number_of_spectrum_colours,
	Pixel background_drawing_colour,struct Colour *background_printer_colour,
	Pixel contour_colour,XFontStruct *font,float printer_page_width_mm,
	float printer_page_height_mm,float printer_page_left_margin_mm,
	float printer_page_right_margin_mm,float printer_page_top_margin_mm,
	float printer_page_bottom_margin_mm,struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 31 May 1997

DESCRIPTION :
Opens a file with the specified <file_name> for writing.  Writes the PostScript
file heading and some standard routines to the file.  If the <page_orientation>
is <PORTRAIT> then the short axis is x and the long axis is y.  If the
<page_orientation> is <LANDSCAPE> then the long axis is x and the short axis is
y.  The axes are scaled so that the units are mm.
==============================================================================*/

int close_postscript(void);
/*******************************************************************************
LAST MODIFIED : 8 December 1992

DESCRIPTION :
Write the terminating postscript commands and closes the <postscript> file.
==============================================================================*/

int get_postscript_page_size(float *width,float *height);
/*******************************************************************************
LAST MODIFIED : 23 June 1996

DESCRIPTION :
Gets the <width> and <height> of the postscript page.  Used in conjunction with
set_postscript_display_transfor .
==============================================================================*/

int new_postscript_page(void);
/*******************************************************************************
LAST MODIFIED : 29 April 1993

DESCRIPTION :
Write the postscript commands for printing the current drawing (page) and then
clearing the drawing (page).  NB  This resets the display transformation to the
default display transformation.
==============================================================================*/

int set_postscript_display_transfor(float page_window_left,
	float page_window_bottom,float page_window_width,float page_window_height,
	float world_left,float world_top,float world_width,float world_height);
/*******************************************************************************
LAST MODIFIED : 29 April 1993

DESCRIPTION :
Sets the display transformation so that the contents of the world rectangle are
displayed in the page rectangle.
==============================================================================*/

int get_postscript_display_transfor(float *page_window_left,
	float *page_window_bottom,float *page_window_width,float *page_window_height,
	float *world_left,float *world_top,float *world_width,float *world_height);
/*******************************************************************************
LAST MODIFIED : 22 June 1997

DESCRIPTION :
Gets the display transformation.
==============================================================================*/

float get_pixel_aspect_ratio(Display *display);
/*******************************************************************************
LAST MODIFIED : 8 February 1997

DESCRIPTION :
Returns the aspect ratio (height/width) of a pixel in display space.
==============================================================================*/

void XPSDrawLine(Display *display,Drawable drawable,GC gc,int x1,int y1,int x2,
	int y2);
/*******************************************************************************
LAST MODIFIED : 13 December 1992

DESCRIPTION :
If the <postscript> file is open then PostScript for drawing the line is
written to the file, otherwise XDrawLine is called.
???In the future the idea is for it to switch based on either the <display> or
the <drawable> (not sure which)
==============================================================================*/

void XPSDrawLineFloat(Display *display,Drawable drawable,GC gc,float x1,
	float y1,float x2,float y2);
/*******************************************************************************
LAST MODIFIED : 14 August 1994

DESCRIPTION :
If the <postscript> file is open then PostScript for drawing the line is
written to the file, otherwise XDrawLine is called.
???In the future the idea is for it to switch based on either the <display> or
the <drawable> (not sure which)
???At present only retrieves the foreground colour from the graphics context.
???DB.  Quick fix so that I can get smoother constant width contours for emap.
???It is a duplicate of XPSDrawLine except that it takes floats for the
???coordinates.
???X.  Round coordinates to integers.
???Postscript.  Use floats.
==============================================================================*/

void XPSDrawLines(Display *display,Drawable drawable,GC gc,XPoint *points,
	int num_points,int mode);
/*******************************************************************************
LAST MODIFIED : 23 February 1992

DESCRIPTION :
If the <postscript> file is open then PostScript for drawing the polyline
specified by <points> is written to the file, otherwise XDrawLines is called.
???In the future the idea is for it to switch based on either the <display> or
the <drawable> (not sure which)
???At present only retrieves the foreground colour from the graphics context.
==============================================================================*/

void XPSDrawString(Display *display,Drawable drawable,GC gc,int x,int y,
	char *string,int length);
/*******************************************************************************
LAST MODIFIED : 13 December 1992

DESCRIPTION :
If the <postscript> file is open then PostScript for drawing the string is
written to the file, otherwise XDrawString is called.
???In the future the idea is for it to switch based on either the <display> or
the <drawable> (not sure which)
==============================================================================*/

void XPSFillRectangle(Display *display,Drawable drawable,GC gc,int x,int y,
	unsigned int width,unsigned int height);
/*******************************************************************************
LAST MODIFIED : 9 December 1992

DESCRIPTION :
If the <postscript> file is open then PostScript for filling the rectangle is
written to the file, otherwise XFillRectangle is called.
???In the future the idea is for it to switch based on either the <display> or
the <drawable> (not sure which)
==============================================================================*/

void XPSFillArc(Display *display,Drawable drawable,GC gc,int x,int y,
	unsigned int width,unsigned int height,int angle1,int angle2);
/*******************************************************************************
LAST MODIFIED : 24 July 1998

DESCRIPTION :
If the <postscript> file is open then PostScript for filling the arc is written
to the file, otherwise XFillArc is called.  Fills a single elliptical arc using
the bounding rectangle and the specified angle.  <angle1> and <angle2> are
specified in units of degrees*64.  <angle1> indicates the starting position of
the arc, and <angle2> indicates the ending position.  If <angle1> is positive,
the direction is counterclockwise from the three o'clock position.  Negative
values indicate clockwise motion.  Positive values for <angle2> indicate
counterclockwise motion relative to the starting position, and negative values
indicate clockwise motion.  If the value of <angle2> is greater than 360*64, it
is truncated to 360*64.
???In the future the idea is for it to switch based on either the <display> or
the <drawable> (not sure which)
==============================================================================*/

void XPSPutImage(Display *display,Drawable drawable,GC gc,XImage *image,
	int src_x,int src_y,int dest_x,int dest_y,unsigned int width,
	unsigned int height);
/*******************************************************************************
LAST MODIFIED : 13 December 1992

DESCRIPTION :
If the <postscript> file is open then PostScript for displaying the <image> is
written to the file, otherwise XPutImage is called.
???In the future the idea is for it to switch based on either the <display> or
the <drawable> (not sure which)
==============================================================================*/

void XPSSetClipRectangles(Display *display,GC gc,int x,int y,
	XRectangle *rectangles,int num_rectangles,int order);
/*******************************************************************************
LAST MODIFIED : 10 May 1993

DESCRIPTION :
If the <postscript> file is open then PostScript for setting the clipping
rectangles is written to the file, otherwise XSetClipRectangles is called.
???Initially, for postscript, only one rectangle will be used and the clipping
will be for all GCs.
==============================================================================*/

void XPSSetClipMask(Display *display,GC gc,Pixmap pixmap);
/*******************************************************************************
LAST MODIFIED : 10 May 1993

DESCRIPTION :
If the <postscript> file is open then PostScript for setting the clip mask
attribute of the <gc> is written to the file, otherwise XSetClipMask is called.
???Initially, for postscript, this will turn off clipping.
==============================================================================*/
#endif
