/*******************************************************************************
FILE : acquisition_work_area.h

LAST MODIFIED : 16 June 1999

DESCRIPTION :
UNIMA_ACQUISITION refers to the acquisition window used with the UNIMA/EMAP
	hardware
==============================================================================*/
#if !defined (ACQUISITION_WORK_AREA_H)
#define ACQUISITION_WORK_AREA_H

#include <stddef.h>
#if defined (MOTIF)
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <Xm/Xm.h>
#endif /* defined (MOTIF) */
#if defined (UNIMA_ACQUISITION)
#include "unemap/acquisition_window.h"
#else /* defined (UNIMA_ACQUISITION) */
#include "unemap/page_window.h"
#endif /* defined (UNIMA_ACQUISITION) */
#include "unemap/mapping_window.h"
#include "unemap/rig.h"
#include "user_interface/user_interface.h"

/*
Global types
============
*/
struct Acquisition_work_area
/*******************************************************************************
LAST MODIFIED : 16 June 1999

DESCRIPTION :
==============================================================================*/
{
#if defined (MOTIF)
	Widget window_shell;
	Widget activation;
#endif /* defined (MOTIF) */
#if defined (UNIMA_ACQUISITION)
	struct Acquisition_window *window;
#else /* defined (UNIMA_ACQUISITION) */
	struct Page_window *window;
#endif /* defined (UNIMA_ACQUISITION) */
	struct Mapping_work_area *mapping_work_area;
	struct Mapping_window *mapping_window;
	struct Rig *rig;
	struct User_interface *user_interface;
}; /* struct Acquisition_work_area */

/*
Global functions
----------------
*/
#if defined (MOTIF)
void close_acquisition_work_area(Widget widget,
	XtPointer acquisition_work_area,XtPointer call_data);
/*******************************************************************************
LAST MODIFIED : 21 June 1997

DESCRIPTION :
Closes the windows associated with the acquisition work area.
==============================================================================*/
#endif /* defined (MOTIF) */

#if defined (MOTIF)
void set_mapping_acquisition_region(Widget widget,
	XtPointer acquisition_work_area,XtPointer call_data);
/*******************************************************************************
LAST MODIFIED : 16 September 1992

DESCRIPTION :
Called when a new rig region is selected from the mapping window in the
acquisition work area.
==============================================================================*/
#endif /* defined (MOTIF) */

int create_acquisition_work_area(struct Acquisition_work_area *acquisition,
#if defined (MOTIF)
	Widget activation,Widget parent,
#endif /* defined (MOTIF) */
	int pointer_sensitivity,
#if defined (MOTIF)
	Pixel identifying_colour,
#endif /* defined (MOTIF) */
	char *signal_file_extension_write,struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 30 April 1999

DESCRIPTION :
Creates the windows associated with the acquisition work area.
???Assign storage for work area ?
==============================================================================*/
#endif /* !defined (ACQUISITION_WORK_AREA_H) */
