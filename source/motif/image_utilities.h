/*******************************************************************************
FILE : motif/image_utilities.h

LAST MODIFIED : 5 July 2002

DESCRIPTION :
Utilities for handling images with X and Motif.
==============================================================================*/
#if !defined (MOTIF_IMAGE_UTILITIES_H)
#define MOTIF_IMAGE_UTILITIES_H
#include <Xm/Xm.h>
#include "general/image_utilities.h"
#include "graphics/colour.h"

/*
Global functions
----------------
*/

struct Cmgui_image *create_Cmgui_image_from_Pixmap(Display *display,
	Pixmap pixmap);
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Creates a single Cmgui_image which stores the data from the X <pixmap>.
==============================================================================*/

Pixmap create_Pixmap_from_Cmgui_image(Display *display,
	struct Cmgui_image *cmgui_image, int depth);
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Generates an X Pixmap which represents the <cmgui_image>.
==============================================================================*/

int convert_Colour_to_Pixel(Display *display, struct Colour *colour,
	Pixel *pixel);
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Finds an X pixel value that represents the <colour>.
==============================================================================*/

int convert_Pixel_to_Colour(Display *display, Pixel pixel,
	struct Colour *colour);
/*******************************************************************************
LAST MODIFIED : 5 July 2002

DESCRIPTION :
Fills in the <colour> based on the pixel.
==============================================================================*/
#endif /* !defined (MOTIF_IMAGE_UTILITIES_H) */
