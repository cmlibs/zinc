/*******************************************************************************
FILE : cell_cmgui_interface.c

LAST MODIFIED : 19 June 2001

DESCRIPTION :
Routines for using the Cell_cmgui_interface objects which allow Cell to interact
with the rest of CMGUI.
==============================================================================*/

#include <stdio.h>
#include <string.h>

#include "cell/cell_graphic.h"
#include "cell/cell_component.h"
#include "cell/cell_cmgui_interface.h"
#include "graphics/scene_viewer.h"
#include "interaction/interactive_toolbar_widget.h"
#include "selection/any_object_selection.h"

/*
Module Objects
--------------
*/
struct Cell_cmgui_interface
/*******************************************************************************
LAST MODIFIED : 01 February 2001

DESCRIPTION :
The main object for Cell to communicate with CMGUI objects.
==============================================================================*/
{
  struct Any_object_selection *any_object_selection;
  struct MANAGER(Interactive_tool) *interactive_tool_manager;
  struct Interactive_tool *interactive_tool,*select_tool,*transform_tool;
  struct Scene_viewer *scene_viewer;
  struct Colour *background_colour;
  struct MANAGER(Light) *light_manager;
  struct Light *default_light;
  struct MANAGER(Light_model) *light_model_manager;
  struct Light_model *default_light_model;
  struct MANAGER(Scene) *scene_manager;
  struct Scene *scene;
  struct MANAGER(Texture) *texture_manager;
  struct LIST(GT_object) *graphics_object_list;
  struct MANAGER(Graphical_material) *graphical_material_manager;
  struct Graphical_material *default_graphical_material;
  struct LIST(GT_object) *glyph_list;
  struct MANAGER(Spectrum) *spectrum_manager;
  struct Spectrum *default_spectrum;
  struct Time_keeper *time_keeper;
  struct User_interface *user_interface;
  struct Cell_window *cell_window;
  struct Cell_interface *cell_interface;
  Widget toolbar_widget;
#if defined (CELL_DISTRIBUTED)
  struct Element_point_ranges_selection *element_point_ranges_selection;
  struct Computed_field_package *computed_field_package;
  struct MANAGER(FE_element) *element_manager;
  struct MANAGER(GROUP(FE_element)) *element_group_manager;
  struct MANAGER(FE_field) *fe_field_manager;
#endif /* defined (CELL_DISTRIBUTED) */
}; /* struct Cell_cmgui_interface */

/*
Module Variables
----------------
*/
/* Used to control the resetting of the variable editing dialog */
static int reset_variable_editing_dialog = 0;

/*
Module Functions
----------------
*/

static void initialise_cell_scene(struct Cell_cmgui_interface *cmgui_interface)
/*******************************************************************************
LAST MODIFIED : 17 November 2000

DESCRIPTION :
Initialise the scene viewer for Cell 3D.
==============================================================================*/
{
  struct Colour *colour;
  
  ENTER(intialise_cell_scene);
  if (cmgui_interface && cmgui_interface->scene_viewer)
  {
    /* set-up the view */
    /* *** */
    /* *** Check graphics/scene_viewer.h for functions *** */
    /* *** */
    /* set the background colour to black */
    colour = create_Colour(0.0,0.0,0.0);
    Scene_viewer_set_background_colour(cmgui_interface->scene_viewer,colour);
    destroy_Colour(&colour);
    /* set the view */
    Scene_viewer_set_lookat_parameters_non_skew(cmgui_interface->scene_viewer,
      0.371067,8.52279,-63.3145,-0.884569,0.135409,0.298719,1,0,0);
    Scene_viewer_set_view_simple(cmgui_interface->scene_viewer,
      0,0,0,15,43.412,100);
    /* turn off the axis */
    Scene_set_axis_visibility(cmgui_interface->scene,g_INVISIBLE);
  }
  else
  {
    display_message(ERROR_MESSAGE,"initialise_cell_scene.  "
      "Invalid argument(s)");
  }
  LEAVE;
} /* intialise_cell_scene() */

static int set_interactive_tool(struct Cell_cmgui_interface *cmgui_interface,
	struct Interactive_tool *interactive_tool)
/*******************************************************************************
LAST MODIFIED : 17 November 2000

DESCRIPTION :
Sets the <interactive_tool> in use in the <cmgui_interface>. Updates the
toolbar to match the selection.
==============================================================================*/
{
	int return_code;

	ENTER(set_interactive_tool);
	if (cmgui_interface)
	{
		if (interactive_toolbar_widget_set_current_interactive_tool(
			cmgui_interface->toolbar_widget,interactive_tool))
		{
			cmgui_interface->interactive_tool = interactive_tool;
			if (interactive_tool == cmgui_interface->transform_tool)
			{
				Scene_viewer_set_input_mode(cmgui_interface->scene_viewer,
					SCENE_VIEWER_TRANSFORM);
			}
			else
			{
				Scene_viewer_set_input_mode(cmgui_interface->scene_viewer,
					SCENE_VIEWER_SELECT);
			}
			Scene_viewer_set_interactive_tool(cmgui_interface->scene_viewer,
				interactive_tool);
			return_code=1;
		}
		else
		{
			display_message(ERROR_MESSAGE,"set_interactive_tool.  "
        "Could not update toolbar");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"set_interactive_tool.  "
      "Invalid argument(s)");
		return_code=0;
	}
	LEAVE;
	return (return_code);
} /* set_interactive_tool() */

static void update_interactive_tool(Widget widget,
	void *cmgui_interface_void,void *interactive_tool_void)
/*******************************************************************************
LAST MODIFIED : 17 November 2000

DESCRIPTION :
Called when a new tool is chosen in the toolbar_widget.
==============================================================================*/
{
	struct Cell_cmgui_interface *cmgui_interface;

	ENTER(update_interactive_tool);
	USE_PARAMETER(widget);
	if (cmgui_interface = (struct Cell_cmgui_interface *)cmgui_interface_void)
	{
		set_interactive_tool(cmgui_interface,
			(struct Interactive_tool *)interactive_tool_void);
	}
	else
	{
		display_message(ERROR_MESSAGE,"update_interactive_tool.  "
      "Invalid argument(s)");
	}
	LEAVE;
} /* update_interactive_tool() */

static int add_component_to_scene(struct Cell_component *component,
  void *scene_void)
/*******************************************************************************
LAST MODIFIED : 20 November 2000

DESCRIPTION :
Iterator function used to add component's to the given <scene> - if the
<component> has a graphic defined.
==============================================================================*/
{
  int return_code = 0;
	struct Scene *scene;
  struct Cell_graphic *graphic;
  struct GT_object *graphics_object;
  struct Scene_object *scene_object;
  gtMatrix transformation;
  char *name;

	ENTER(add_component_to_scene);
	if (component && (scene = (struct Scene *)scene_void))
	{
    if ((graphic = Cell_component_get_graphic(component)) &&
      (graphics_object = Cell_graphic_get_graphics_object(graphic)))
    {
      if (name = Cell_component_get_name(component))
      {
        /* Draw the graphic into the scene */
        Scene_add_graphics_object(scene,graphics_object,0,name,
          /*fast_changing*/0);
        /* Grab the scene object */
        if (scene_object = Scene_get_Scene_object_by_name(scene,name))
        {
          /* Construct the transformation matrix */
          transformation[0][0] = Cell_component_get_axis1(component,0);
          transformation[0][1] = Cell_component_get_axis1(component,1);
          transformation[0][2] = Cell_component_get_axis1(component,2);
          transformation[0][3] = 0.0;
          transformation[1][0] = Cell_component_get_axis2(component,0);
          transformation[1][1] = Cell_component_get_axis2(component,1);
          transformation[1][2] = Cell_component_get_axis2(component,2);
          transformation[1][3] = 0.0;
          transformation[2][0] = Cell_component_get_axis3(component,0);
          transformation[2][1] = Cell_component_get_axis3(component,1);
          transformation[2][2] = Cell_component_get_axis3(component,2);
          transformation[2][3] = 0.0;
          transformation[3][0] = Cell_component_get_point(component,0);
          transformation[3][1] = Cell_component_get_point(component,1);
          transformation[3][2] = Cell_component_get_point(component,2);
          transformation[3][3] = 1.0;
          /* Apply the transformation */
          return_code = Scene_object_set_transformation(scene_object,
            &transformation);
          /* Set the callback data for the any object selection */
          Scene_object_set_represented_object(scene_object,
            CREATE(ANY_OBJECT(Cell_component))(component));
        }
        DEALLOCATE(name);
      }
      else
      {
        display_message(WARNING_MESSAGE,"add_component_to_scene.  "
          "Unable to get the component's name");
        return_code = 1;
      }
    }
    else
    {
      /* No graphic - no error */
      return_code = 1;
    }
	}
	else
	{
		display_message(ERROR_MESSAGE,"add_component_to_scene.  "
      "Invalid argument(s)");
    return_code = 0;
	}
	LEAVE;
  return(return_code);
} /* add_component_to_scene() */

static int remove_graphic_from_scene(struct Cell_graphic *graphic,
  void *scene_void)
/*******************************************************************************
LAST MODIFIED : 20 November 2000

DESCRIPTION :
Iterator function used to remove graphics from the <scene>
==============================================================================*/
{
  int return_code = 0;
	struct Scene *scene;
  struct GT_object *graphics_object;

	ENTER(remove_graphic_from_scene);
	if (graphic && (scene = (struct Scene *)scene_void))
	{
    if (graphics_object = Cell_graphic_get_graphics_object(graphic))
    {
      Scene_remove_graphics_object(scene,graphics_object);
      return_code = 1;
    }
    else
    {
      /* No graphic - no error */
      return_code = 1;
    }
	}
	else
	{
		display_message(ERROR_MESSAGE,"remove_graphic_from_scene.  "
      "Invalid argument(s)");
    return_code = 0;
	}
	LEAVE;
  return(return_code);
} /* remove_graphic_from_scene() */

int edit_variables_in_component(struct Cell_component *component,
  void *cell_interface_void)
/*******************************************************************************
LAST MODIFIED : 20 November 2000

DESCRIPTION :
Iterator function for adding selected components to the variable editing dialog
==============================================================================*/
{
  int return_code = 0;
  struct Cell_interface *cell_interface;
  char *name;

  ENTER(edit_variables_in_component);
  if (component &&
    (cell_interface = (struct Cell_interface *)cell_interface_void))
  {
    if (name = Cell_component_get_name(component))
    {
      Cell_interface_edit_component_variables(cell_interface,name,
        reset_variable_editing_dialog);
      reset_variable_editing_dialog = 0;
      return_code = 1;
      DEALLOCATE(name);
    }
    else
    {
      display_message(WARNING_MESSAGE,"edit_variables_in_component.  "
        "Unable to get the components name");
      return_code = 1;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"edit_variables_in_component.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* edit_variables_in_component() */

static void cell_any_object_selection_change(
  struct Any_object_selection *any_object_selection,
	struct Any_object_selection_changes *changes,
	void *cell_cmgui_interface_void)
/*******************************************************************************
LAST MODIFIED : 20 November 2000

DESCRIPTION :
Callback for change in the global selection of Any_objects - from which
Cell_component selections are interpreted.
==============================================================================*/
{
  struct Cell_cmgui_interface *cmgui_interface;
  struct LIST(Any_object) *selection_list;

  ENTER(cell_any_object_selection_change);
  USE_PARAMETER(changes);
  if (any_object_selection && (selection_list =
    Any_object_selection_get_any_object_list(any_object_selection)) &&
    (cmgui_interface =
      (struct Cell_cmgui_interface *)cell_cmgui_interface_void))
  {
    /* Make sure that the dialog is reset for the first component added */
    reset_variable_editing_dialog = 1;
    FOR_EACH_OBJECT_IN_LIST(ANY_OBJECT(Cell_component))(
			edit_variables_in_component,(void *)(cmgui_interface->cell_interface),
			selection_list);
  }
  else
  {
    display_message(ERROR_MESSAGE,"cell_any_object_selection_change.  "
      "Invalid argument(s)");
  }
  LEAVE;
} /* cell_any_object_selection_change() */

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
  )
/*******************************************************************************
LAST MODIFIED : 01 February 2001

DESCRIPTION :
Creates a Cell_cmgui_interface object, setting the data fields.
==============================================================================*/
{
  struct Cell_cmgui_interface *cmgui_interface;
  struct Callback_data callback;
  Widget form;

  ENTER(CREATE(Cell_cmgui_interface));
  if (any_object_selection && background_colour &&
    default_graphical_material && default_light && default_light_model &&
    default_scene && default_spectrum && time_keeper &&
    graphics_object_list && glyph_list && interactive_tool_manager &&
    light_manager && light_model_manager && graphical_material_manager &&
    scene_manager && spectrum_manager && texture_manager && user_interface
#if defined (CELL_DISTRIBUTED)
    && element_point_ranges_selection && computed_field_package &&
    element_manager && element_group_manager && fe_field_manager
#endif /* defined (CELL_DISTRIBUTED) */
    )
  {
    if (ALLOCATE(cmgui_interface,struct Cell_cmgui_interface,1))
    {
      /* Initialise the fields */
      cmgui_interface->toolbar_widget = (Widget)NULL;
      cmgui_interface->cell_interface = cell_interface;
      cmgui_interface->cell_window = cell_window;
      cmgui_interface->any_object_selection = any_object_selection;
      cmgui_interface->background_colour = background_colour;
      cmgui_interface->default_graphical_material = default_graphical_material;
      cmgui_interface->default_light = default_light;
      cmgui_interface->default_light_model = default_light_model;
      cmgui_interface->scene = (struct Scene *)NULL;
      cmgui_interface->default_spectrum = default_spectrum;
      cmgui_interface->time_keeper = time_keeper;
      cmgui_interface->graphics_object_list = graphics_object_list;
      cmgui_interface->glyph_list = glyph_list;
      cmgui_interface->interactive_tool_manager = interactive_tool_manager;
      cmgui_interface->light_manager = light_manager;
      cmgui_interface->light_model_manager = light_model_manager;
      cmgui_interface->graphical_material_manager = graphical_material_manager;
      cmgui_interface->scene_manager = scene_manager;
      cmgui_interface->spectrum_manager = spectrum_manager;
      cmgui_interface->texture_manager = texture_manager;
      cmgui_interface->user_interface = user_interface;
      cmgui_interface->transform_tool = ACCESS(Interactive_tool)(
        FIND_BY_IDENTIFIER_IN_MANAGER(Interactive_tool,name)(
          "transform_tool",cmgui_interface->interactive_tool_manager));
      cmgui_interface->select_tool = ACCESS(Interactive_tool)(
        FIND_BY_IDENTIFIER_IN_MANAGER(Interactive_tool,name)(
          "select_tool",cmgui_interface->interactive_tool_manager));
      cmgui_interface->interactive_tool = cmgui_interface->select_tool;
#if defined (CELL_DISTRIBUTED)
      cmgui_interface->element_point_ranges_selection =
        element_point_ranges_selection;
      cmgui_interface->computed_field_package = computed_field_package;
      cmgui_interface->element_manager = element_manager;
      cmgui_interface->element_group_manager = element_group_manager;
      cmgui_interface->fe_field_manager = fe_field_manager;
#endif /* defined (CELL_DISTRIBUTED) */
      if (cmgui_interface->scene = CREATE(Scene)("Cell 3D"))
      {
        Scene_enable_graphics(cmgui_interface->scene,
          cmgui_interface->glyph_list,
          cmgui_interface->graphical_material_manager,
          cmgui_interface->default_graphical_material,
          cmgui_interface->light_manager,
          cmgui_interface->spectrum_manager,
          cmgui_interface->default_spectrum,
          cmgui_interface->texture_manager);
        Scene_disable_time_behaviour(cmgui_interface->scene);
        if (!ADD_OBJECT_TO_MANAGER(Scene)(cmgui_interface->scene,
          cmgui_interface->scene_manager))
        {
          display_message(WARNING_MESSAGE,"create_Cell_window. "
            "Unable to add scene to manager, using default");
          DESTROY(Scene)(&cmgui_interface->scene);
          cmgui_interface->scene = default_scene;
        }
      }
      else
      {
        display_message(WARNING_MESSAGE,"create_Cell_window. "
          "Unable to create a new scene, using default");
        cmgui_interface->scene = default_scene;
      }
      form = Cell_window_get_scene_form(cmgui_interface->cell_window);
      if (cmgui_interface->scene_viewer =
        CREATE(Scene_viewer)(form,cmgui_interface->background_colour,
          SCENE_VIEWER_DOUBLE_BUFFER,cmgui_interface->light_manager,
          cmgui_interface->default_light,cmgui_interface->light_model_manager,
          cmgui_interface->default_light_model,cmgui_interface->scene_manager,
          cmgui_interface->scene,cmgui_interface->texture_manager,
          cmgui_interface->user_interface))
      {
        /* Set-up the scene */
        initialise_cell_scene(cmgui_interface);
        /* Create the Toolbar */
        form = Cell_window_get_toolbar_form(cmgui_interface->cell_window);
        if (cmgui_interface->toolbar_widget =
          create_interactive_toolbar_widget(form,
            cmgui_interface->interactive_tool_manager,
            INTERACTIVE_TOOLBAR_HORIZONTAL))
        {
          if (cmgui_interface->transform_tool)
          {
            add_interactive_tool_to_interactive_toolbar_widget(
              cmgui_interface->transform_tool,
              (void *)(cmgui_interface->toolbar_widget));
          }
          if (cmgui_interface->select_tool)
          {
            add_interactive_tool_to_interactive_toolbar_widget(
              cmgui_interface->select_tool,
              (void *)(cmgui_interface->toolbar_widget));
          }
          /* Make sure that the select tool is initially set */
          set_interactive_tool(cmgui_interface,cmgui_interface->select_tool);
          /* Set the callback for interactive toolbar */
          callback.data = (void *)cmgui_interface;
          callback.procedure = update_interactive_tool;
          interactive_toolbar_widget_set_callback(
            cmgui_interface->toolbar_widget,&callback);
          /* Set the callback for any object selection */
          Any_object_selection_add_callback(any_object_selection,
            cell_any_object_selection_change,(void *)cmgui_interface);
        }
        else
        {
          display_message(ERROR_MESSAGE,"CREATE(Cell_cmgui_interface).  "
            "Unable to create the toolbar widget");
          DESTROY(Cell_cmgui_interface)(&cmgui_interface);
          cmgui_interface = (struct Cell_cmgui_interface *)NULL;
        }
      }
      else
      {
        display_message(ERROR_MESSAGE,"CREATE(Cell_cmgui_interface).  "
          "Unable to create the scene viewer");
        DESTROY(Cell_cmgui_interface)(&cmgui_interface);
        cmgui_interface = (struct Cell_cmgui_interface *)NULL;
      }
    }
    else
    {
      display_message(ERROR_MESSAGE,"CREATE(Cell_cmgui_interface).  "
        "Unable to allocate memory for the CMGUI interface");
      cmgui_interface = (struct Cell_cmgui_interface *)NULL;
    }
  }
  else
  {
    display_message(ERROR_MESSAGE,"CREATE(Cell_cmgui_interface).  "
      "Invalid argument(s)");
    cmgui_interface = (struct Cell_cmgui_interface *)NULL;
  }
  LEAVE;
  return(cmgui_interface);
} /* CREATE(Cell_cmgui_interface)() */

int DESTROY(Cell_cmgui_interface)(
  struct Cell_cmgui_interface **cmgui_interface_address)
/*******************************************************************************
LAST MODIFIED : 25 October 2000

DESCRIPTION :
Destroys a Cell_cmgui_interface object.
==============================================================================*/
{
  int return_code = 0;
  struct Cell_cmgui_interface *cmgui_interface;

  ENTER(DESTROY(Cell_cmgui_interface));
  if (cmgui_interface_address && (cmgui_interface = *cmgui_interface_address))
  {
    /* Deaccess the interactive tools */
    if (cmgui_interface->transform_tool)
    {
      DEACCESS(Interactive_tool)(&cmgui_interface->transform_tool);
    }
    if (cmgui_interface->select_tool)
    {
      DEACCESS(Interactive_tool)(&cmgui_interface->select_tool);
    }
    if (cmgui_interface->toolbar_widget)
    {
      /* By destroying the widget we free up the associated memory in the
         widget's destroy callback */
      XtDestroyWidget(cmgui_interface->toolbar_widget);
    }
    if (cmgui_interface->scene_viewer)
    {
      DESTROY(Scene_viewer)(&cmgui_interface->scene_viewer);
    }
    DEALLOCATE(*cmgui_interface_address);
    *cmgui_interface_address = (struct Cell_cmgui_interface *)NULL;
  }
  else
  {
    display_message(ERROR_MESSAGE,"DESTROY(Cell_cmgui_interface). "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* DESTROY(Cell_cmgui_interface)() */

struct LIST(GT_object) *Cell_cmgui_interface_get_graphics_object_list(
  struct Cell_cmgui_interface *cmgui_interface)
/*******************************************************************************
LAST MODIFIED : 18 November 2000

DESCRIPTION :
Returns the <cmgui_interface>'s graphics object list.
==============================================================================*/
{
  struct LIST(GT_object) *list;

  ENTER(Cell_cmgui_interface_get_graphics_object_list);
  if (cmgui_interface)
  {
    list = cmgui_interface->graphics_object_list;
  }
  else
  {
    display_message(ERROR_MESSAGE,
      "Cell_cmgui_interface_get_graphics_object_list.  "
      "Invalid argument(s)");
    list = (struct LIST(GT_object) *)NULL;
  }
  LEAVE;
  return(list);
} /* Cell_cmgui_interface_get_graphics_object_list() */

struct MANAGER(Graphical_material)
  *Cell_cmgui_interface_get_graphical_material_manager(
    struct Cell_cmgui_interface *cmgui_interface)
/*******************************************************************************
LAST MODIFIED : 18 November 2000

DESCRIPTION :
Returns the <cmgui_interface>'s graphical material manager.
==============================================================================*/
{
  struct MANAGER(Graphical_material) *manager;

  ENTER(Cell_cmgui_interface_get_graphical_material_manager);
  if (cmgui_interface)
  {
    manager = cmgui_interface->graphical_material_manager;
  }
  else
  {
    display_message(ERROR_MESSAGE,
      "Cell_cmgui_interface_get_graphical_material_manager.  "
      "Invalid argument(s)");
    manager = (struct MANAGER(Graphical_material) *)NULL;
  }
  LEAVE;
  return(manager);
} /* Cell_cmgui_interface_get_graphical_material_manager() */

int Cell_cmgui_interface_draw_component_graphics(
  struct Cell_cmgui_interface *cmgui_interface,
  void *component_list_void)
/*******************************************************************************
LAST MODIFIED : 20 November 2000

DESCRIPTION :
For each of the components in the <component_list> which has a graphic, add the
graphic to the Cell scene and set-up the callback data.
==============================================================================*/
{
  int return_code = 0;
  struct LIST(Cell_component) *component_list;
  
  ENTER(Cell_cmgui_interface_draw_component_graphics);
  if (cmgui_interface && (component_list =
    (struct LIST(Cell_component) *)component_list_void))
  {
    /* Loop through all the components adding any graphics */
    return_code = FOR_EACH_OBJECT_IN_LIST(Cell_component)(
      add_component_to_scene,(void *)(cmgui_interface->scene),component_list);
  }
  else
  {
    display_message(ERROR_MESSAGE,
      "Cell_cmgui_interface_draw_component_graphics.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_cmgui_interface_draw_component_graphics() */

int Cell_cmgui_interface_clear_scene(
  struct Cell_cmgui_interface *cmgui_interface,
  void *graphic_list_void)
/*******************************************************************************
LAST MODIFIED : 20 November 2000

DESCRIPTION :
Clears the graphics objects from the current Cell scene.
==============================================================================*/
{
  int return_code = 0;
  struct LIST(Cell_graphic) *graphic_list;
  
  ENTER(Cell_cmgui_interface_clear_scene);
  if (cmgui_interface && cmgui_interface->scene &&
    (graphic_list = (struct LIST(Cell_graphic) *)graphic_list_void))
  {
    return_code = FOR_EACH_OBJECT_IN_LIST(Cell_graphic)(
      remove_graphic_from_scene,(void *)(cmgui_interface->scene),graphic_list);
  }
  else
  {
    display_message(ERROR_MESSAGE,
      "Cell_cmgui_interface_clear_scene.  "
      "Invalid argument(s)");
    return_code = 0;
  }
  LEAVE;
  return(return_code);
} /* Cell_cmgui_interface_clear_scene() */

#if defined (CELL_DISTRIBUTED)
struct MANAGER(FE_element) *Cell_cmgui_interface_get_element_manager(
  struct Cell_cmgui_interface *cmgui_interface)
/*******************************************************************************
LAST MODIFIED : 16 January 2001

DESCRIPTION :
Returns the <cmgui_interface>'s FE element manager.
==============================================================================*/
{
  struct MANAGER(FE_element) *manager;

  ENTER(Cell_cmgui_interface_get_element_manager);
  if (cmgui_interface)
  {
    manager = cmgui_interface->element_manager;
  }
  else
  {
    display_message(ERROR_MESSAGE,
      "Cell_cmgui_interface_get_element_manager.  "
      "Invalid argument(s)");
    manager = (struct MANAGER(FE_element) *)NULL;
  }
  LEAVE;
  return(manager);
} /* Cell_cmgui_interface_get_element_manager() */
#endif /* defined (CELL_DISTRIBUTED) */

#if defined (CELL_DISTRIBUTED)
struct MANAGER(GROUP(FE_element))
  *Cell_cmgui_interface_get_element_group_manager(
    struct Cell_cmgui_interface *cmgui_interface)
/*******************************************************************************
LAST MODIFIED : 01 February 2001

DESCRIPTION :
Returns the <cmgui_interface>'s FE element group manager.
==============================================================================*/
{
  struct MANAGER(GROUP(FE_element)) *manager;

  ENTER(Cell_cmgui_interface_get_element_group_manager);
  if (cmgui_interface)
  {
    manager = cmgui_interface->element_group_manager;
  }
  else
  {
    display_message(ERROR_MESSAGE,
      "Cell_cmgui_interface_get_element_group_manager.  "
      "Invalid argument(s)");
    manager = (struct MANAGER(GROUP(FE_element)) *)NULL;
  }
  LEAVE;
  return(manager);
} /* Cell_cmgui_interface_get_element_group_manager() */
#endif /* defined (CELL_DISTRIBUTED) */

#if defined (CELL_DISTRIBUTED)
struct Element_point_ranges_selection
*Cell_cmgui_interface_get_element_point_ranges_selection(
  struct Cell_cmgui_interface *cmgui_interface)
/*******************************************************************************
LAST MODIFIED : 16 January 2001

DESCRIPTION :
Returns the <cmgui_interface>'s element point ranges selection.
==============================================================================*/
{
  struct Element_point_ranges_selection *element_point_ranges_selection;

  ENTER(Cell_cmgui_interface_get_element_point_ranges_selection);
  if (cmgui_interface)
  {
    element_point_ranges_selection =
      cmgui_interface->element_point_ranges_selection;
  }
  else
  {
    display_message(ERROR_MESSAGE,
      "Cell_cmgui_interface_get_element_point_ranges_selection.  "
      "Invalid argument(s)");
    element_point_ranges_selection =
      (struct Element_point_ranges_selection *)NULL;
  }
  LEAVE;
  return(element_point_ranges_selection);
} /* Cell_cmgui_interface_get_element_point_ranges_selection() */
#endif /* defined (CELL_DISTRIBUTED) */

#if defined (CELL_DISTRIBUTED)
struct MANAGER(Computed_field) *Cell_cmgui_interface_get_computed_field_manager(
  struct Cell_cmgui_interface *cmgui_interface)
/*******************************************************************************
LAST MODIFIED : 16 January 2001

DESCRIPTION :
Returns the <cmgui_interface>'s computed field manager.
==============================================================================*/
{
  struct MANAGER(Computed_field) *manager;

  ENTER(Cell_cmgui_interface_get_computed_field_manager);
  if (cmgui_interface)
  {
    manager = Computed_field_package_get_computed_field_manager(
      cmgui_interface->computed_field_package);
  }
  else
  {
    display_message(ERROR_MESSAGE,
      "Cell_cmgui_interface_get_computed_field_manager.  "
      "Invalid argument(s)");
    manager = (struct MANAGER(Computed_field) *)NULL;
  }
  LEAVE;
  return(manager);
} /* Cell_cmgui_interface_get_computed_field_manager() */
#endif /* defined (CELL_DISTRIBUTED) */

#if defined (CELL_DISTRIBUTED)
struct MANAGER(FE_field) *Cell_cmgui_interface_get_fe_field_manager(
  struct Cell_cmgui_interface *cmgui_interface)
/*******************************************************************************
LAST MODIFIED : 02 February 2001

DESCRIPTION :
Returns the <cmgui_interface>'s FE field manager.
==============================================================================*/
{
  struct MANAGER(FE_field) *manager;

  ENTER(Cell_cmgui_interface_get_fe_field_manager);
  if (cmgui_interface)
  {
    manager = cmgui_interface->fe_field_manager;
  }
  else
  {
    display_message(ERROR_MESSAGE,
      "Cell_cmgui_interface_get_fe_field_manager.  "
      "Invalid argument(s)");
    manager = (struct MANAGER(FE_field) *)NULL;
  }
  LEAVE;
  return(manager);
} /* Cell_cmgui_interface_get_fe_field_manager() */
#endif /* defined (CELL_DISTRIBUTED) */
