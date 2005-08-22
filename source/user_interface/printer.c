/*******************************************************************************
FILE : printer.c

LAST MODIFIED : 21 June 1997

DESCRIPTION :
Functions for handling a printer.
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

#include "general/debug.h"
#include "user_interface/message.h"
#include "user_interface/printer.h"
#include "user_interface/user_interface.h"

/*
Module types
------------
*/
typedef struct Printer User_settings;

/*
Global functions
----------------
*/
int open_printer(struct Printer *printer,struct User_interface *user_interface)
/*******************************************************************************
LAST MODIFIED : 31 May 1997

DESCRIPTION :
Open the <printer>.
==============================================================================*/
{
	int return_code;
#if defined (MOTIF)
#define XmNprinterBackgroundColour "printerBackgroundColour"
#define XmCPrinterBackgroundColour "PrinterBackgroundColour"
#define XmNprinterForegroundColour "printerForegroundColour"
#define XmCPrinterForegroundColour "PrinterForegroundColour"
#define XmNprinterPageBottomMarginMm "printerPageBottomMarginMm"
#define XmCPrinterPageBottomMarginMm "PrinterPageBottomMarginMm"
#define XmNprinterPageHeightMm "printerPageHeightMm"
#define XmCPrinterPageHeightMm "PrinterPageHeightMm"
#define XmNprinterPageLeftMarginMm "printerPageLeftMarginMm"
#define XmCPrinterPageLeftMarginMm "PrinterPageLeftMarginMm"
#define XmNprinterPageRightMarginMm "printerPageRightMarginMm"
#define XmCPrinterPageRightMarginMm "PrinterPageRightMarginMm"
#define XmNprinterPageTopMarginMm "printerPageTopMarginMm"
#define XmCPrinterPageTopMarginMm "PrinterPageTopMarginMm"
#define XmNprinterPageWidthMm "printerPageWidthMm"
#define XmCPrinterPageWidthMm "PrinterPageWidthMm"
	static XtResource resources[]=
	{
		{
			XmNprinterForegroundColour,
			XmCPrinterForegroundColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(User_settings,foreground_colour_pixel),
			XmRString,
			"black"
		},
		{
			XmNprinterBackgroundColour,
			XmCPrinterBackgroundColour,
			XmRPixel,
			sizeof(Pixel),
			XtOffsetOf(User_settings,background_colour_pixel),
			XmRString,
			"white"
		},
		{
			XmNprinterPageBottomMarginMm,
			XmCPrinterPageBottomMarginMm,
			XmRInt,
			sizeof(int),
			XtOffsetOf(User_settings,page_bottom_margin_mm),
			XmRString,
			"25"
		},
		{
			XmNprinterPageHeightMm,
			XmCPrinterPageHeightMm,
			XmRInt,
			sizeof(int),
			XtOffsetOf(User_settings,page_height_mm),
			XmRString,
			"297"
		},
		{
			XmNprinterPageLeftMarginMm,
			XmCPrinterPageLeftMarginMm,
			XmRInt,
			sizeof(int),
			XtOffsetOf(User_settings,page_left_margin_mm),
			XmRString,
			"10"
		},
		{
			XmNprinterPageRightMarginMm,
			XmCPrinterPageRightMarginMm,
			XmRInt,
			sizeof(int),
			XtOffsetOf(User_settings,page_right_margin_mm),
			XmRString,
			"10"
		},
		{
			XmNprinterPageTopMarginMm,
			XmCPrinterPageTopMarginMm,
			XmRInt,
			sizeof(int),
			XtOffsetOf(User_settings,page_top_margin_mm),
			XmRString,
			"25"
		},
		{
			XmNprinterPageWidthMm,
			XmCPrinterPageWidthMm,
			XmRInt,
			sizeof(int),
			XtOffsetOf(User_settings,page_width_mm),
			XmRString,
			"210"
		}
	};
	XColor colour;
#endif /* defined (MOTIF) */

	ENTER(open_printer);
	if (printer&&user_interface)
	{
		return_code=0;
#if defined (MOTIF)
		XtVaGetApplicationResources(User_interface_get_application_shell(user_interface),
			printer,resources,XtNumber(resources),NULL);
		colour.pixel=printer->background_colour_pixel;
		XQueryColor(User_interface_get_display(user_interface),
			XDefaultColormap(User_interface_get_display(user_interface),
			DefaultScreen(User_interface_get_display(user_interface))),&colour);
		printer->background_colour.red=(float)(colour.red)/65535.;
		printer->background_colour.green=(float)(colour.green)/65535.;
		printer->background_colour.blue=(float)(colour.blue)/65535.;
		colour.pixel=printer->foreground_colour_pixel;
		XQueryColor(User_interface_get_display(user_interface),
			XDefaultColormap(User_interface_get_display(user_interface),
			DefaultScreen(User_interface_get_display(user_interface))),&colour);
		printer->foreground_colour.red=(float)(colour.red)/65535.;
		printer->foreground_colour.green=(float)(colour.green)/65535.;
		printer->foreground_colour.blue=(float)(colour.blue)/65535.;
		return_code=1;
#endif /* defined (MOTIF) */
	}
	else
	{
		display_message(ERROR_MESSAGE,"open_printer.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* open_printer */

int close_printer(struct Printer *printer)
/*******************************************************************************
LAST MODIFIED : 12 December 1996

DESCRIPTION :
Close the <printer>.
==============================================================================*/
{
	int return_code;

	ENTER(close_printer);
	if (printer)
	{
		return_code=1;
	}
	else
	{
		display_message(ERROR_MESSAGE,"close_printer.  Missing printer");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* close_printer */
