/*******************************************************************************
FILE : system_window.h

LAST MODIFIED : 8 May 2003

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

struct System_window;
/*******************************************************************************
LAST MODIFIED : 17 July 2002

DESCRIPTION :
Unemap system window object.
The contents of this object are private.
==============================================================================*/

typedef void Unemap_system_window_close_callback_procedure(struct System_window *, void *);

/*
Global functions
----------------
*/

struct System_window *CREATE(System_window)(Widget shell,
	Unemap_system_window_close_callback_procedure *close_callback,
	void *close_callback_data,
#if defined (UNEMAP_USE_3D)
	struct Element_point_ranges_selection *element_point_ranges_selection,
	struct FE_element_selection *element_selection,
	struct FE_node_selection *node_selection,
	struct FE_node_selection *data_selection,
	struct MANAGER(FE_basis) *fe_basis_manager,
	struct Cmiss_region *root_cmiss_region,
	struct Cmiss_region *data_root_cmiss_region,
	struct MANAGER(Texture) *texture_manager,
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
	struct MANAGER(Scene) *scene_manager,
	struct MANAGER(Light_model) *light_model_manager,
	struct MANAGER(Light) *light_manager,
	struct MANAGER(Spectrum) *spectrum_manager,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct LIST(GT_object) *glyph_list,
	struct Graphical_material *graphical_material,
	struct Computed_field_package *computed_field_package,
	struct Light *light,
	struct Light_model *light_model,
#endif /* defined (UNEMAP_USE_3D) */
	struct Time_keeper *time_keeper,
	struct User_interface *user_interface);
/*******************************************************************************
LAST MODIFIED : 8 May 2003

DESCRIPTION :
This function allocates the memory for a system window structure.  It then
retrieves a system window widget with the specified parent/<shell> and assigns
the widget ids to the appropriate fields of the structure.  It returns a
pointer to the created structure if successful and NULL if unsuccessful.
Note the system window is automatically popped-up in this function.
The <close_callback> and associated <close_callback_data> are called when the
user closes the system window. These must be supplied, and are responsible for
the handling the differences in cleaning up Unemap when it is run in Cmgui and
as a standalone application.
==============================================================================*/

int DESTROY(System_window)(struct System_window **system_window_address);
/*******************************************************************************
LAST MODIFIED : 16 July 2002

DESCRIPTION :
Destroys the Unemap system window and all its dependent windows.
==============================================================================*/

int System_window_pop_up(struct System_window *system_window);
/*******************************************************************************
LAST MODIFIED : 17 July 2002

DESCRIPTION :
De-iconifies and brings the unemap system window to the front.
==============================================================================*/

#endif /* !defined (SYSTEM_WINDOW_H) */
