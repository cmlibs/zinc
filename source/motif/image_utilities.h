/*******************************************************************************
FILE : motif/image_utilities.h

LAST MODIFIED : 5 July 2002

DESCRIPTION :
Utilities for handling images with X and Motif.
==============================================================================*/
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is cmgui.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2005
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
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
