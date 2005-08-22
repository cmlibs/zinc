/*******************************************************************************
FILE : printer.h

LAST MODIFIED : 31 May 1997

DESCRIPTION :
Structures and function prototypes for handling a printer.
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
