/*******************************************************************************
FILE : printer.c

LAST MODIFIED : 21 June 1997

DESCRIPTION :
Functions for handling a printer.
==============================================================================*/

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
