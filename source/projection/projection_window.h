/*******************************************************************************
FILE : projection_window.h

LAST MODIFIED : 29 January 1999

DESCRIPTION :
???DB.  Started as mapping_window.h in emap
==============================================================================*/
#if !defined (PROJECTION_WINDOW_H)
#define PROJECTION_WINDOW_H

#include <stddef.h>
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <Xm/Xm.h>
#include "command/parser.h"
#include "finite_element/finite_element.h"
#include "projection/projection.h"

/*
Global types
------------
*/
struct Projection_print_menu
/*******************************************************************************
LAST MODIFIED : 23 July 1998

DESCRIPTION :
The menu associated with the print button.
==============================================================================*/
{
	Widget postscript_button;
	Widget rgb_button;
	Widget tiff_button;
}; /* struct Projection_print_menu */

struct Projection_window
/*******************************************************************************
LAST MODIFIED : 24 May 1997

DESCRIPTION :
The projection window object.
==============================================================================*/
{
	/* creater is the widget which was selected to create the window */
	Widget creator,shell,window;
	int open;
	Widget configure_button;
	struct Projection_dialog *configure_dialog;
	Widget animate_button;
	Widget print_button;
	struct Projection_print_menu print_menu;
	Widget close_button;
	struct Projection *projection;
	Widget projection_drawing_area;
	struct Drawing_2d *projection_drawing;
#if defined (OLD_CODE)
/*???DB.  Replaced by unemap/drawing_2d.h */
	struct Projection_drawing *projection_drawing;
#endif /* defined (OLD_CODE) */
	Widget spectrum_drawing_area;
	struct Drawing_2d *spectrum_drawing;
#if defined (OLD_CODE)
/*???DB.  Replaced by unemap/drawing_2d.h */
	struct Projection_drawing *spectrum_drawing;
#endif /* defined (OLD_CODE) */
	int animation_front;
	struct Projection_window **address;
	struct User_interface *user_interface;
}; /* struct Projection_window */

struct Open_projection_window_data
/*******************************************************************************
LAST MODIFIED : 29 January 1999

DESCRIPTION :
Data required for opening a projection window.
==============================================================================*/
{
	struct MANAGER(FE_field) *fe_field_manager;
	struct MANAGER(FE_element) *element_manager;
	struct MANAGER(GROUP(FE_element)) *element_group_manager;
	struct MANAGER(Spectrum) *spectrum_manager;
	struct Spectrum *default_spectrum;
	struct User_interface *user_interface;
}; /* struct Open_projection_window_data */

/*
Global functions
----------------
*/
int open_projection_window(struct Parse_state *state,
	void *projection_window_address_void,void *open_projection_window_data_void);
/*******************************************************************************
LAST MODIFIED : 8 October 1996

DESCRIPTION :
If the projection window does not exist it is created.  The projection window is
then opened with the specified <modifications>.
==============================================================================*/

int update_projection_drawing_area(struct Projection_window *window,
	int recalculate);
/*******************************************************************************
LAST MODIFIED : 24 May 1997

DESCRIPTION :
This function for redrawing the <mapping> drawing area.  If <recalculate> is >0
then the colours for the pixels are recalculated.  If <recalculate> is >1 then
the interpolation functions are also recalculated.
==============================================================================*/

int update_projection_spectrum(struct Projection_window *window);
/*******************************************************************************
LAST MODIFIED : 3 December 1992

DESCRIPTION :
The callback for redrawing the colour bar or auxiliary devices drawing area.
==============================================================================*/
#endif /* !defined (PROJECTION_WINDOW_H) */
