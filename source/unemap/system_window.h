/*******************************************************************************
FILE : system_window.h

LAST MODIFIED : 30 April 1999

DESCRIPTION :
==============================================================================*/
#if !defined (SYSTEM_WINDOW_H)
#define SYSTEM_WINDOW_H

#include <stddef.h>
#if defined (MOTIF)
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <Xm/Xm.h>
#endif /* defined (MOTIF) */
#include "unemap/acquisition_work_area.h"
#include "unemap/analysis_work_area.h"
#include "unemap/mapping_work_area.h"
#include "user_interface/user_interface.h"

/*
Global types
------------
*/
struct Time_keeper;  /* Either this or #include "time/time.h" */

struct System_window
/*******************************************************************************
LAST MODIFIED : 30 April 1999

DESCRIPTION :
The system window object.
==============================================================================*/
{
	Widget window,window_shell;
	Widget acquisition_button;
	struct Acquisition_work_area acquisition;
	Widget analysis_button;
	struct Analysis_work_area analysis;
	Widget mapping_button;
	struct Mapping_work_area mapping;
	Widget close_button;
	struct User_interface *user_interface;
	struct Map_drawing_information *map_drawing_information;
	/* user settings */
	char *configuration_directory,*configuration_file_extension,
		*postscript_file_extension,*signal_file_extension_read,
		*signal_file_extension_write;
	int pointer_sensitivity;
	Pixel acquisition_colour,analysis_colour;
	struct Time_keeper *time_keeper;
	struct Unemap_package *unemap_package;	
}; /* struct System_window */

/*
Global functions
----------------
*/
void close_emap(Widget widget,XtPointer system_window,XtPointer call_data);
/*******************************************************************************
LAST MODIFIED : 14 February 1998

DESCRIPTION :
Close emap environment.
==============================================================================*/

struct System_window *create_System_window(Widget shell,
	XtCallbackProc close_button_callback,struct Time_keeper *time_keeper,
	struct User_interface *user_interface,struct Unemap_package *unemap_package
#if defined (UNEMAP_USE_NODES) 
	,struct Element_point_ranges_selection *element_point_ranges_selection,
	struct FE_element_selection *element_selection,
	struct FE_node_selection *node_selection,
	struct FE_node_selection *data_selection,
	struct MANAGER(Texture) *texture_manager,
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
	struct MANAGER(Scene) *scene_manager,
	struct MANAGER(Light_model) *light_model_manager,
	struct MANAGER(Light) *light_manager,
	struct MANAGER(Spectrum) *spectrum_manager,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct MANAGER(FE_node) *data_manager,
	struct LIST(GT_object) *glyph_list,
	struct Graphical_material *graphical_material,
	struct Computed_field_package *computed_field_package,
	struct Light *light,
	struct Light_model *light_model
#endif /* defined (UNEMAP_USE_NODES) */
	);
/*******************************************************************************
LAST MODIFIED : 27 April 1999

DESCRIPTION :
This function allocates the memory for a system window structure.  It then
retrieves a system window widget with the specified parent/<shell> and assigns
the widget ids to the appropriate fields of the structure.  It returns a
pointer to the created structure if successful and NULL if unsuccessful.
==============================================================================*/
#endif /* !defined (SYSTEM_WINDOW_H) */
