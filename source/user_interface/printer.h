/*******************************************************************************
FILE : printer.h

LAST MODIFIED : 31 May 1997

DESCRIPTION :
Structures and function prototypes for handling a printer.
==============================================================================*/
#if !defined (PRINTER_H)
#define PRINTER_H

#include "graphics/colour.h"
#include "user_interface/user_interface.h"

/*
Global types
------------
*/
struct Printer
/*******************************************************************************
LAST MODIFIED : 31 May 1997

DESCRIPTION :
Printer information.
==============================================================================*/
{
	int page_bottom_margin_mm,page_height_mm,page_left_margin_mm,
		page_right_margin_mm,page_top_margin_mm,page_width_mm;
	struct Colour background_colour,foreground_colour;
#if defined (MOTIF)
	Pixel background_colour_pixel,foreground_colour_pixel;
#endif /* defined (MOTIF) */
}; /* struct Printer */

/*
Global functions
----------------
*/
int open_printer(struct Printer *printer,struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 12 December 1996

DESCRIPTION :
Open the <printer>.
==============================================================================*/

int close_printer(struct Printer *printer);
/*******************************************************************************
LAST MODIFIED : 12 December 1996

DESCRIPTION :
Close the <printer>.
==============================================================================*/
#endif /* !defined (PRINTER_H) */
