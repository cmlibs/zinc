/*******************************************************************************
FILE : unemap_command.h

LAST MODIFIED : 8 May 2003

DESCRIPTION :
Functions and for executing unemap commands.
This should only be included in command/unemap_command.c, command/cmiss.c and
unemap.c.
==============================================================================*/
#if !defined (UNEMAP_COMMAND_H)
#define UNEMAP_COMMAND_H
#include "time/time_keeper.h"
#if defined (NOT_ACQUISITION_ONLY)
#include "unemap/system_window.h"
#else /* defined (NOT_ACQUISITION_ONLY) */
#include "unemap/page_window.h"
#endif /* defined (NOT_ACQUISITION_ONLY) */
#include "user_interface/event_dispatcher.h"
#include "user_interface/user_interface.h"

/*
Global types
------------
*/

struct Unemap_command_data;
/*******************************************************************************
LAST MODIFIED : 12 July 2002

DESCRIPTION :
Data required for Unemap commands.
The contents of this structure are private.
==============================================================================*/

/*
Global functions
----------------
*/

struct Unemap_command_data *CREATE(Unemap_command_data)(
	struct Event_dispatcher *event_dispatcher,
	struct Execute_command *execute_command,
	struct User_interface *user_interface,
#if defined (NOT_ACQUISITION_ONLY)
#if defined (UNEMAP_USE_3D)
#if defined (MOTIF)
	struct Node_tool *node_tool,
	struct Interactive_tool *transform_tool,
#endif /* defined (MOTIF) */
	struct LIST(GT_object) *glyph_list,
	struct Computed_field_package *computed_field_package,
	struct MANAGER(FE_basis) *basis_manager,
	struct Cmiss_region *root_cmiss_region,
	struct Cmiss_region *data_root_cmiss_region,
	struct MANAGER(Graphical_material) *graphical_material_manager,
	struct Graphical_material *default_graphical_material,
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
	struct MANAGER(Light) *light_manager,
	struct Light *default_light,
	struct MANAGER(Light_model) *light_model_manager,
	struct Light_model *default_light_model,
	struct MANAGER(Texture) *texture_manager,
	struct MANAGER(Scene) *scene_manager,
	struct MANAGER(Spectrum) *spectrum_manager,
	struct Element_point_ranges_selection *element_point_ranges_selection,
	struct FE_element_selection *element_selection,
	struct FE_node_selection *data_selection,
	struct FE_node_selection *node_selection,
#endif /* defined (UNEMAP_USE_3D) */
	struct System_window *unemap_system_window,
	struct Time_keeper *default_time_keeper
#else /* defined (NOT_ACQUISITION_ONLY) */
	struct Page_window *page_window
#endif /* defined (NOT_ACQUISITION_ONLY) */
	);
/*******************************************************************************
LAST MODIFIED : 8 May 2003

DESCRIPTION :
Creates a Unemap_command_data structure containing pointers to the passed
objects.
Note only the unemap_system_window is optional. If it is passed into this
structure here then it will not be destroyed when the command data is destroyed.
If the unemap_system_window is created by commands within this structure, it
will be destroyed with it.
==============================================================================*/

int DESTROY(Unemap_command_data)(
	struct Unemap_command_data **unemap_command_data_address);
/*******************************************************************************
LAST MODIFIED : 12 July 2002

DESCRIPTION :
Destroys the Unemap_command_data at <unemap_command_data_address>.
==============================================================================*/

int execute_command_unemap(struct Parse_state *state,
	void *prompt_void, void *unemap_command_data_void);
/*******************************************************************************
LAST MODIFIED : 16 July 2002

DESCRIPTION :
Executes a UNEMAP command.
Pass the command stem string in <prompt_void>, for example "unemap" in Cmgui,
to allow the command prompt to remain.
==============================================================================*/

int unemap_execute_command(char *command_string,
	void *unemap_command_data_void);
/*******************************************************************************
LAST MODIFIED : 15 July 2002

DESCRIPTION:
Execute a <command_string>.
==============================================================================*/

#endif /* !defined (UNEMAP_COMMAND_H) */
