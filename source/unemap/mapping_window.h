/*******************************************************************************
FILE : mapping_window.h

LAST MODIFIED : 1 February 2000

DESCRIPTION :
==============================================================================*/
#if !defined (MAPPING_WINDOW_H)
#define MAPPING_WINDOW_H

#include <stddef.h>
#if defined (MOTIF)
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <Xm/Xm.h>
#endif /* defined (MOTIF) */
#include "unemap/map_dialog.h"
#include "unemap/mapping.h"

/*
Global types
------------
*/
enum Mapping_associate
/*******************************************************************************
LAST MODIFIED : 28 May 1992

DESCRIPTION :
The work area that the mapping window is currently associated with.
==============================================================================*/
{
	ACQUISITION_ASSOCIATE,
	ANALYSIS_ASSOCIATE
}; /* enum Mapping_associate */

#if defined (MOTIF)
struct Mapping_file_menu
/*******************************************************************************
LAST MODIFIED : 25 March 1997

DESCRIPTION :
The menu associated with the file button.
==============================================================================*/
{
	Widget save_configuration_button;
	Widget read_configuration_button;
	Widget read_bard_electrode_button;
	Widget set_default_configuration_button;
}; /* struct Mapping_file_menu */
#endif /* defined (MOTIF) */

#if defined (MOTIF)
struct Mapping_print_menu
/*******************************************************************************
LAST MODIFIED : 23 July 1998

DESCRIPTION :
The menu associated with the print button.
==============================================================================*/
{
	Widget postscript_button;
	Widget rgb_button;
	Widget tiff_button;
	Widget animate_rgb_button;
	Widget animate_tiff_button;
}; /* struct Mapping_print_menu */
#endif /* defined (MOTIF) */

struct Mapping_window
/*******************************************************************************
LAST MODIFIED : 1 February 2000

DESCRIPTION :
The mapping window object.
==============================================================================*/
{
#if defined (MOTIF)
	struct Scene_viewer *scene_viewer;
	Widget activation,window;
	Widget menu;
	Widget map_button;
#endif /* defined (MOTIF) */
	struct Map_dialog *map_dialog;
#if defined (MOTIF)
	Widget animate_button;
	Widget setup_button;
#endif /* defined (MOTIF) */
	struct Setup_dialog *setup_dialog;
#if defined (MOTIF)
	Widget modify_button;
	Widget page_button;
	Widget file_button;
	struct Mapping_file_menu file_menu;
	Widget projection_choice; /* the menu*/
	Widget projection_cylinder; /* choice in the above menu */	
	Widget projection_hammer; /* choice in the above menu */	
	Widget projection_polar; /* choice in the above menu */	
	Widget projection_patch; /* choice in the above menu */	
	Widget projection_3d; /* choice in the above menu */
	Widget region_choice;
	Widget region_pull_down_menu;
#endif /* defined (MOTIF) */
	int number_of_regions;
#if defined (MOTIF)
	WidgetList regions;
	Widget current_region;
	Widget print_button;
	struct Mapping_print_menu print_menu;
	Widget close_button;
#endif /* defined (MOTIF) */
	struct Map *map;
#if defined (MOTIF)
	Widget map_drawing_area_2d;
	Widget mapping_area;
	Widget mapping_area_2d,mapping_area_3d;
#endif /* defined (MOTIF) */
	struct Drawing_2d *map_drawing;
#if defined (MOTIF)
	Widget colour_or_auxiliary_drawing_area;
#endif /* defined (MOTIF) */
	struct Drawing_2d *colour_or_auxiliary_drawing;
#if defined (MOTIF)
	Widget colour_or_auxiliary_scroll_bar;
#endif /* defined (MOTIF) */
	struct Mapping_window **address,**current_mapping_window_address;
	char *open;
	struct Time_object *potential_time_object;
#if defined (MOTIF)
	Widget time_editor_dialog;
#endif /* defined (MOTIF) */
	struct User_interface *user_interface;
}; /* struct Mapping_window */

/*
Global variables
----------------
*/
#if defined (MAPPING_WINDOW)
#if defined (MOTIF)
Widget mapping_file_read_select_shel=(Widget)NULL,
	mapping_file_read_select=(Widget)NULL;
#endif /* defined (MOTIF) */
#else
#if defined (MOTIF)
/*??? do these all need to be visible ? */
extern Widget mapping_file_read_select_shel,mapping_file_read_select;
#endif /* defined (MOTIF) */
#endif

/*
Global functions
----------------
*/
#if defined (OLD_CODE)
struct Mapping_window *create_Mapping_window(
	struct Mapping_window **address,Widget activation,Widget parent,
	struct Map *map,struct Rig **rig_pointer,Pixel identifying_colour,
	int screen_width,int screen_height,char *configuration_file_extension,
	char *postscript_file_extension,
	struct Map_drawing_information *map_drawing_information,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 29 December 1996

DESCRIPTION :
This function allocates the memory for a mapping_window and sets the fields to
the specified values (<address>, <map>).  It then retrieves a mapping window
widget with the specified parent/<shell> and assigns the widget ids to the
appropriate fields of the structure.  If successful it returns a pointer to the
created mapping window and, if <address> is not NULL, makes <*address> point to
the created mapping window.  If unsuccessful, NULL is returned.
==============================================================================*/

Widget create_mapping_window_shell(Widget *address,Widget parent,
	int screen_width,int screen_height);
/*******************************************************************************
LAST MODIFIED : 27 April 1999

DESCRIPTION :
Creates a popup shell widget for a mapping window.  If <address> is not NULL,
<*address> is set to the id of the created widget and on destruction <*address>
is set to NULL.  The id of the created widget is returned.
==============================================================================*/
#endif /* defined (OLD_CODE) */

int open_mapping_window(struct Mapping_window **mapping_address,
#if defined (MOTIF)
	Widget activation,Widget parent,Widget *shell,Widget *outer_form,
#endif /* defined (MOTIF) */
	struct Mapping_window **current_mapping_window_address,char *open,
	enum Mapping_associate *current_associate,enum Map_type *map_type,
	enum Colour_option colour_option,enum Contours_option contours_option,
	enum Electrodes_option electrodes_option,enum Fibres_option fibres_option,
	enum Landmarks_option landmarks_option,enum Extrema_option extrema_option,
	int maintain_aspect_ratio,int print_spectrum,
	enum Projection_type projection_type,enum Contour_thickness contour_thickness,
	struct Rig **rig_address,int *event_number_address,
	int *potential_time_address,int *datum_address,int *start_search_interval,
	int *end_search_interval,
#if defined (MOTIF)
	Pixel identifying_colour,
#endif /* defined (MOTIF) */
	enum Mapping_associate associate,
#if defined (MOTIF)
	XtPointer set_mapping_region,XtPointer select_map_drawing_area,
	XtPointer select_colour_or_auxiliary_draw,XtPointer work_area,
#endif /* defined (MOTIF) */
	int screen_width,int screen_height,
	char *configuration_file_extension,char *postscript_file_extension,
	struct Map_drawing_information *map_drawing_information,
	struct User_interface *user_interface,struct Unemap_package *unemap_package);
/*******************************************************************************
LAST MODIFIED : 1 February 2000

DESCRIPTION :
If the mapping window does not exist then it is created with the specified
properties.  Then the mapping window is opened.
==============================================================================*/

int update_mapping_drawing_area(struct Mapping_window *mapping,int recalculate);
/*******************************************************************************
LAST MODIFIED : 30 May 2000

DESCRIPTION :
Calls draw_map_3d or update_mapping_drawing_area_2d depending upon
<mapping> ->map->projection_type
==============================================================================*/

int update_mapping_colour_or_auxili(struct Mapping_window *mapping);
/*******************************************************************************
LAST MODIFIED : 10 June 1992

DESCRIPTION :
The callback for redrawing the colour bar or auxiliary devices drawing area.
==============================================================================*/

int update_mapping_window_menu(struct Mapping_window *mapping);
/*******************************************************************************
LAST MODIFIED : 7 August 1992

DESCRIPTION :
Updates the mapping region menu to be consistent with the current rig.
==============================================================================*/

int highlight_electrode_or_auxiliar(struct Device *device,int electrode_number,
	int auxiliary_number,
#if defined (OLD_CODE)
	int start_data,int end_data,
#endif
	struct Map *map,
	struct Mapping_window *mapping);
/*******************************************************************************
LAST MODIFIED : 7 May 1993

DESCRIPTION :
Highlights/dehighlights an electrode or an auxiliary device in the <mapping>
window.
==============================================================================*/

int Mapping_window_set_potential_time_object(struct Mapping_window *mapping,
	struct Time_object *potential_time_object);
/*******************************************************************************
LAST MODIFIED : 15 October 1998
DESCRIPTION :
==============================================================================*/
#endif /* !defined (MAPPING_WINDOW_H) */
