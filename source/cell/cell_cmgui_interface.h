/*******************************************************************************
FILE : cell_cmgui_interface.h

LAST MODIFIED : 02 February 2001

DESCRIPTION :
Routines for using the Cell_cmgui_interface objects which allow Cell to interact
with the rest of CMGUI.
==============================================================================*/
#if !defined (CELL_CMGUI_INTERFACE_H)
#define CELL_CMGUI_INTERFACE_H

#include "cell/cell_interface.h"
#include "cell/cell_window.h"

/*
Module Objects
--------------
*/
struct Cell_cmgui_interface;
/*******************************************************************************
LAST MODIFIED : 16 January 2001

DESCRIPTION :
The main object for Cell to communicate with CMGUI objects.
==============================================================================*/

/*
Global Functions
----------------
*/
struct Cell_cmgui_interface *CREATE(Cell_cmgui_interface)(
	struct Any_object_selection *any_object_selection,
  struct Colour *background_colour,
  struct Graphical_material *default_graphical_material,
  struct Light *default_light,
  struct Light_model *default_light_model,
  struct Scene *default_scene,
  struct Spectrum *default_spectrum,
  struct Time_keeper *time_keeper,
  struct LIST(GT_object) *graphics_object_list,
  struct LIST(GT_object) *glyph_list,
	struct MANAGER(Interactive_tool) *interactive_tool_manager,
  struct MANAGER(Light) *light_manager,
  struct MANAGER(Light_model) *light_model_manager,
  struct MANAGER(Graphical_material) *graphical_material_manager,
  struct MANAGER(Scene) *scene_manager,
  struct MANAGER(Spectrum) *spectrum_manager,
  struct MANAGER(Texture) *texture_manager,
  struct User_interface *user_interface,
  struct Cell_window *cell_window,struct Cell_interface *cell_interface
#if defined (CELL_DISTRIBUTED)
  ,struct Element_point_ranges_selection *element_point_ranges_selection,
  struct Computed_field_package *computed_field_package,
  struct MANAGER(FE_element) *element_manager,
  struct MANAGER(GROUP(FE_element)) *element_group_manager,
  struct MANAGER(FE_field) *fe_field_manager
#endif /* defined (CELL_DISTRIBUTED) */
  );
/*******************************************************************************
LAST MODIFIED : 25 October 2000

DESCRIPTION :
Creates a Cell_cmgui_interface object, setting the data fields.
==============================================================================*/
int DESTROY(Cell_cmgui_interface)(
  struct Cell_cmgui_interface **cmgui_interface_address);
/*******************************************************************************
LAST MODIFIED : 25 October 2000

DESCRIPTION :
Destroys a Cell_cmgui_interface object.
==============================================================================*/
struct LIST(GT_object) *Cell_cmgui_interface_get_graphics_object_list(
  struct Cell_cmgui_interface *cmgui_interface);
/*******************************************************************************
LAST MODIFIED : 18 November 2000

DESCRIPTION :
Returns the <cmgui_interface>'s graphics object list.
==============================================================================*/
struct Graphical_material *Cell_cmgui_interface_get_default_graphical_material(
	  struct Cell_cmgui_interface *cmgui_interface);
/*******************************************************************************
LAST MODIFIED : 20 November 2002

DESCRIPTION :
Returns the <cmgui_interface>'s default graphical material.
==============================================================================*/
struct MANAGER(Graphical_material)
  *Cell_cmgui_interface_get_graphical_material_manager(
    struct Cell_cmgui_interface *cmgui_interface);
/*******************************************************************************
LAST MODIFIED : 18 November 2000

DESCRIPTION :
Returns the <cmgui_interface>'s graphical material manager.
==============================================================================*/
int Cell_cmgui_interface_draw_component_graphics(
  struct Cell_cmgui_interface *cmgui_interface,
  void *component_list_void);
/*******************************************************************************
LAST MODIFIED : 20 November 2000

DESCRIPTION :
For each of the components in the <component_list> which has a graphic, add the
graphic to the Cell scene and set-up the callback data.
==============================================================================*/
int Cell_cmgui_interface_clear_scene(
  struct Cell_cmgui_interface *cmgui_interface,
  void *graphic_list_void);
/*******************************************************************************
LAST MODIFIED : 20 November 2000

DESCRIPTION :
Clears the graphics objects from the current Cell scene.
==============================================================================*/
struct MANAGER(FE_element) *Cell_cmgui_interface_get_element_manager(
  struct Cell_cmgui_interface *cmgui_interface);
/*******************************************************************************
LAST MODIFIED : 16 January 2001

DESCRIPTION :
Returns the <cmgui_interface>'s FE element manager.
==============================================================================*/
struct MANAGER(GROUP(FE_element))
  *Cell_cmgui_interface_get_element_group_manager(
    struct Cell_cmgui_interface *cmgui_interface);
/*******************************************************************************
LAST MODIFIED : 01 February 2001

DESCRIPTION :
Returns the <cmgui_interface>'s FE element group manager.
==============================================================================*/
struct Element_point_ranges_selection
*Cell_cmgui_interface_get_element_point_ranges_selection(
  struct Cell_cmgui_interface *cmgui_interface);
/*******************************************************************************
LAST MODIFIED : 16 January 2001

DESCRIPTION :
Returns the <cmgui_interface>'s element point ranges selection.
==============================================================================*/
struct MANAGER(Computed_field) *Cell_cmgui_interface_get_computed_field_manager(
  struct Cell_cmgui_interface *cmgui_interface);
/*******************************************************************************
LAST MODIFIED : 16 January 2001

DESCRIPTION :
Returns the <cmgui_interface>'s computed field manager.
==============================================================================*/
struct MANAGER(FE_field) *Cell_cmgui_interface_get_fe_field_manager(
  struct Cell_cmgui_interface *cmgui_interface);
/*******************************************************************************
LAST MODIFIED : 02 February 2001

DESCRIPTION :
Returns the <cmgui_interface>'s FE field manager.
==============================================================================*/

#endif /* !defined (CELL_CMGUI_INTERFACE_H) */
